/* A YM3812 (OPL2), the chip of the AdLib, for the music (music.c). Claude, 2026-11-17.
 *
 * Written from scratch in C11 from what is known of the chip (Yamaha's YM3812 manual: the register map, the rate and
 * level tables; and the chip's documented inner workings: log-sine / exp tables, the envelope counter); no third-party
 * code. Every number here is G: a chip fact as I know it, not checked against a real AdLib or a DOSBox capture
 * (docs/MUSIC_NOTES.md 10). One chip, one sample per call at the chip's own rate 49715.9 Hz.
 *
 *   - 9 channels of 2 operators (the modulator and the carrier); connection FM (C0 bit 0 = 0: the modulator's output
 *     moves the carrier's phase) or additive (1: both are heard); feedback of the modulator (C0 bits 1-3: its last two
 *     outputs, summed, >> (9 - FB), move its own phase; FB 7 = 4 pi)
 *   - the operator: a 20-bit phase (one cycle = 1 << 20) that steps (fnum << block) x MULT each sample (MULT from the
 *     table 1/2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 12, 12, 15, 15; fnum 0x158 block 4 = 261 Hz); the wave is read at
 *     phase >> 10 (1024 per cycle) through a 256-entry log-sine quarter table (attenuation in 1/256 octave) and a
 *     256-entry exp table: a 13-bit signed output, at most 4084 (measured)
 *   - waveforms (E0 bits 0-1, when register 01 bit 5 enables them, else 0): 0 sine, 1 half sine (the negative half
 *     silent), 2 absolute sine, 3 quarter sine (the rising quarters only); the driver uses 0, 1 and 2
 *   - envelope: 9-bit attenuation, 0.1875 dB steps (511 = silent); attack (exponential: each step takes 1/8 of the
 *     distance to 0, instant at rate 60 and up), decay (linear in dB) down to SL (SL x 3 dB; SL 15 = 93 dB), then
 *     sustain held until key-off (EG type 1, register 20 bit 5) or decaying on at the release rate (EG type 0), release
 *     at RR. Rate = 4 x R + the key scale (KSR = 1: block x 2 + fnum bit 9 (bit 8 when NTS, register 08 bit 6); KSR = 0:
 *     that >> 2), at most 63; R = 0 never moves. The chip's global counter: rate 4 x e + q steps on one sample in
 *     2^(12 - e) (e <= 12) by an 8-step pattern of 0 / 1 with mean (4 + q) / 8, or every sample by 1..2 << (e - 13):
 *     each R step halves the time (R 1: 0 -> 96 dB in about 42 s at 49716 Hz; the manual's table, made at 3.6 MHz,
 *     says 39.3 s)
 *   - the level: envelope + TL x 0.75 dB + key-scale level (KSL 1 / 2 / 3 = 3 / 1.5 / 6 dB per octave: by fnum's top 4
 *     bits and the block, 42 / 21 / 10.5 dB at the top of block 7 for KSL 3 / 1 / 2; the manual's 16-entry table is the
 *     3 dB / octave column, half the 6 dB one) + tremolo (AM, register 20 bit 7: a 3.7 Hz triangle, depth 1.1 dB, or
 *     4.9 dB with BD bit 7). 2026-11-17, review: KSL was half of this (the 3 dB column used as the 6 dB one); the
 *     values now agree with what is known of Nuked OPL3's and DOSBox DBOPL's tables (G: neither run here)
 *   - vibrato (VIB, register 20 bit 6): fnum moves by up to (fnum >> 7) / 2 at 6.1 Hz (8 steps of 1024 samples): about
 *     7 cents, or 14 cents with BD bit 6
 *   - key-on (B0 bit 5) 0 -> 1: both operators' phases to 0, attack from the present level; 1 -> 0: release
 *   - the output: the 9 channels' carriers (both operators when additive) summed and clipped to 16 bits
 * Not done, as the AdLib driver never uses them (MUSIC_NOTES 7): rhythm mode (BD bits 0-5), CSM (08 bit 7), the timers
 * (02-04) and the status register.
 */
#include <math.h>
#include <string.h>
#include "opl2.h"

enum { EG_ATTACK, EG_DECAY, EG_SUSTAIN, EG_RELEASE };
typedef struct {
    uint32_t phase;               /* 20 bits */
    int env, state;               /* 0..511, EG_* */
    int out[2];                   /* its last two outputs (a modulator's feedback) */
    uint8_t r20, r40, r60, r80, rE0;
} op_t;
typedef struct { op_t op[2]; uint8_t a0, b0, c0; } ch_t;   /* op[0] the modulator, op[1] the carrier */

static ch_t g_ch[9];
static uint8_t g_01, g_08, g_bd;
static uint32_t g_n;              /* samples since the reset: the envelope counter, tremolo, vibrato */
static uint16_t g_logsin[256];    /* -log2(sin((i + 0.5) pi / 512)) x 256: the first quarter wave, 1/256 octave */
static uint16_t g_exp[256];       /* (2^(i / 256) - 1) x 1024 */

static const uint8_t g_mult[16] = { 1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 24, 24, 30, 30 };   /* MULT x 2 */
/* the key-scale level by fnum >> 6, in 0.75 dB, one octave above block 7 (0 .. 48 dB); minus 6 dB per octave below that:
   at block 7 0, 18, 24, 27.75 ... 42 dB (KSL 3), the manual's 3 dB / octave column (... 21 dB) is half of it (KSL 1) */
static const uint8_t g_ksl[16] = { 0, 32, 40, 45, 48, 51, 53, 55, 56, 58, 59, 60, 61, 62, 63, 64 };

static int asr(int v, int n) { return v >= 0 ? v >> n : -((-v - 1) >> n) - 1; }   /* floor(v / 2^n) */

void opl2_reset(void) {
    if (!g_exp[255])
        for (int i = 0; i < 256; i++) {
            g_logsin[i] = (uint16_t)lround(-log2(sin((i + 0.5) * 3.14159265358979323846 / 512)) * 256);
            g_exp[i] = (uint16_t)lround((exp2(i / 256.0) - 1) * 1024);
        }
    memset(g_ch, 0, sizeof g_ch);
    for (int c = 0; c < 9; c++)
        for (int k = 0; k < 2; k++) { g_ch[c].op[k].env = 511; g_ch[c].op[k].state = EG_RELEASE; }
    g_01 = g_08 = g_bd = 0;
    g_n = 0;
}

/* the operator of registers 20 / 40 / 60 / 80 / E0 + offset 00-05, 08-0D, 10-15 (channel 3g + k % 3; k >= 3 carrier) */
static op_t *slot(uint8_t reg) {
    int o = reg & 0x1F, k = o & 7, g = o >> 3;
    if (k > 5 || g > 2) return NULL;
    return &g_ch[g * 3 + k % 3].op[k / 3];
}

void opl2_write(uint8_t reg, uint8_t v) {
    if (reg == 0x01) { g_01 = v; return; }
    if (reg == 0x08) { g_08 = v; return; }
    if (reg == 0xBD) { g_bd = v; return; }
    if ((reg >= 0x20 && reg < 0xA0) || reg >= 0xE0) {
        op_t *o = slot(reg);
        if (!o) return;
        switch (reg & 0xE0) {
        case 0x20: o->r20 = v; break;
        case 0x40: o->r40 = v; break;
        case 0x60: o->r60 = v; break;
        case 0x80: o->r80 = v; break;
        default: o->rE0 = v; break;
        }
        return;
    }
    int c = reg & 0x0F;
    if (c > 8) return;
    ch_t *ch = &g_ch[c];
    switch (reg & 0xF0) {
    case 0xA0: ch->a0 = v; break;
    case 0xB0:
        if (!(ch->b0 & 0x20) && (v & 0x20))
            for (int k = 0; k < 2; k++) { ch->op[k].phase = 0; ch->op[k].state = EG_ATTACK; }
        else if ((ch->b0 & 0x20) && !(v & 0x20))
            for (int k = 0; k < 2; k++) ch->op[k].state = EG_RELEASE;
        ch->b0 = v;
        break;
    case 0xC0: ch->c0 = v; break;
    default: break;
    }
}

/* the effective rate of R (0..15) with the key scale rof */
static int rate(int r, int ksr, int rof) {
    if (!r) return 0;
    r = 4 * r + (ksr ? rof : rof >> 2);
    return r > 63 ? 63 : r;
}
/* the envelope's step at rate r on this sample (0 = no step now) */
static int eg_step(int r) {
    static const uint8_t lo[4] = { 0xAA, 0xBA, 0xEE, 0xFE };   /* 0/1 patterns, mean 4/8 5/8 6/8 7/8 */
    static const uint8_t hi[4] = { 0x00, 0x88, 0xAA, 0xEE };   /* the +1 of 1..2 patterns, mean 0 1/4 2/4 3/4 */
    int e = r >> 2, q = r & 3;
    if (e == 0) return 0;
    if (e <= 12) {
        int sh = 12 - e;
        if (g_n & ((1u << sh) - 1)) return 0;
        return (lo[q] >> ((g_n >> sh) & 7)) & 1;
    }
    return (1 + ((hi[q] >> (g_n & 7)) & 1)) << (e - 13);
}
static void envelope(op_t *o, int rof) {
    int ksr = (o->r20 & 0x10) != 0;
    if (o->state == EG_ATTACK) {
        int r = rate(o->r60 >> 4, ksr, rof), k;
        if (r >= 60) o->env = 0;
        else if ((k = eg_step(r)) != 0) o->env -= ((o->env + 1) * k + 7) >> 3;
        if (o->env <= 0) { o->env = 0; o->state = EG_DECAY; }
    } else if (o->state == EG_DECAY) {
        int sl = o->r80 >> 4;
        o->env += eg_step(rate(o->r60 & 15, ksr, rof));
        if (o->env >= (sl == 15 ? 31 : sl) << 4) o->state = EG_SUSTAIN;
    } else if (o->state == EG_RELEASE || !(o->r20 & 0x20)) {   /* release; or sustain with EG type 0: on at RR */
        o->env += eg_step(rate(o->r80 & 15, ksr, rof));
    }
    if (o->env > 511) o->env = 511;
}

/* the wave ws at index idx (10 bits) through attenuation att (1/256 octave): 13-bit signed */
static int wave(int ws, unsigned idx, int att) {
    unsigned q = idx & 0xFF;
    if (idx & 0x100) q ^= 0xFF;                    /* the second and fourth quarters run backwards */
    int neg = (idx & 0x200) != 0;
    if (ws == 1 && neg) return 0;
    if (ws == 3 && (idx & 0x100)) return 0;
    if (ws >= 2) neg = 0;
    int a = g_logsin[q] + att;
    if (a >= 13 << 8) return 0;
    int v = ((g_exp[(a & 0xFF) ^ 0xFF] | 0x400) << 1) >> (a >> 8);
    return neg ? -v : v;
}

/* one sample of operator o; mod = the phase offset (the modulator's output or the feedback) */
static int op_calc(op_t *o, int fnum, int block, int rof, int ksl, int am, int mod) {
    static const int8_t ksh[4] = { 0, 1, 2, 0 };
    int tot = o->env + ((o->r40 & 0x3F) << 2) + ((o->r40 >> 6) ? ksl >> ksh[o->r40 >> 6] : 0);
    if (o->r20 & 0x80) tot += am;
    if (tot > 511) tot = 511;
    int v = wave((g_01 & 0x20) ? o->rE0 & 3 : 0, ((o->phase >> 10) + (unsigned)mod) & 0x3FF, tot << 3);
    int f = fnum;
    if (o->r20 & 0x40) {
        static const int8_t vib[8] = { 0, 1, 2, 1, 0, -1, -2, -1 };
        int d = ((fnum >> 7) & 7) * vib[(g_n >> 10) & 7];
        f += (g_bd & 0x40) ? d / 2 : d / 4;
    }
    o->phase = (o->phase + ((((uint32_t)f << block) * g_mult[o->r20 & 15]) >> 1)) & 0xFFFFF;
    envelope(o, rof);
    o->out[1] = o->out[0];
    o->out[0] = v;
    return v;
}

int16_t opl2_sample(void) {
    int tri = (int)((g_n >> 6) % 210);             /* tremolo: 210 steps of 64 samples */
    if (tri > 104) tri = 209 - tri;
    int am = (g_bd & 0x80) ? tri >> 2 : tri >> 4;  /* 0..26 (4.9 dB) or 0..6 (1.1 dB) */
    int32_t mix = 0;
    for (int c = 0; c < 9; c++) {
        ch_t *ch = &g_ch[c];
        int fnum = ch->a0 | (ch->b0 & 3) << 8, block = (ch->b0 >> 2) & 7;
        int rof = block << 1 | ((fnum >> ((g_08 & 0x40) ? 8 : 9)) & 1);
        int ksl = 4 * g_ksl[fnum >> 6] - 32 * (8 - block);   /* in 0.1875 dB steps: 6 dB (32) per octave */
        if (ksl < 0) ksl = 0;
        int fb = (ch->c0 >> 1) & 7;
        op_t *m = &ch->op[0];
        int mv = op_calc(m, fnum, block, rof, ksl, am, fb ? asr(m->out[0] + m->out[1], 9 - fb) : 0);
        if (ch->c0 & 1) mix += mv + op_calc(&ch->op[1], fnum, block, rof, ksl, am, 0);
        else mix += op_calc(&ch->op[1], fnum, block, rof, ksl, am, mv);
    }
    g_n++;
    return (int16_t)(mix > 32767 ? 32767 : mix < -32768 ? -32768 : mix);
}
