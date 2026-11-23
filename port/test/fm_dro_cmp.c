/* fm_dro_cmp: plays one XMIDI song through the port's SBPRO2.MDI translation (fmdrv.h) the way the original's AIL
   host drives the MDI, records every OPL register write with the 120 Hz tick it happened in, and compares that
   write sequence with a DOSBox-X DRO capture of the original.

   usage: fm_dro_cmp [-raw] [-vol N] [-dump N] GAMEDIR SONG.XMI DRO [SECONDS]
     GAMEDIR   holds ACIV.CT5 (the song), EHGM2FM2.OPL (the GTL bank) and ACIV.CFG (the music volume; 9 if absent)
     SONG.XMI  a catalog name in ACIV.CT5, e.g. SMAP__1C.XMI
     DRO       a DOSBox-X DRO v2.0 capture of that song played by SBPRO2.MDI
     SECONDS   how long the port plays (default: one pass of the song + the DRO's length + 5 s)
     -raw      compare the port's writes unfiltered (default: pass them through DOSBox-X's capture rules first)
     -vol N    the music volume 0..9 instead of ACIV.CFG's
     -dump N   the length of the DRO's t = 0 state dump (default: found, see dro_dump_len)
   Exit 0 when every DRO write after the dump matches the port's sequence in order from the alignment, else 1
   (2 on a usage or file error).

   The host side, from C:\work\advciv_cmp\sound\fm\structure.md (AC_MAIN addresses; driver offsets drv 0x...):
     install (6.4, AIL_install_MDI_driver 0x9A860):
       0x300 DRV_INIT, 0x301 GET_INFO: no register writes, nothing to model;
       0x304 VERIFY_IO: the timer probe of 7.2 writes regs 4/2 at IO+8; not part of fmdrv.h, and DOSBox-X never
             logs timer registers, so it is left out;
       0x305 INIT_DEV: fmdrv_new (chip reset 0x2C47, the 492 writes, and the state init 0x2FE1);
       0x501 INIT_INS_MGR: the GTL file (fmdrv_set_bank);
       the channel set-up (step 9, 0x9AD90..0x9AE96): 15 messages per channel queued through 0x98F30, one 0x502
             per channel (CX = 15);
     per song (f7B854, newgame.c 0x7B854): AIL_init_sequence (8.3: flush, then 0x503 with the TIMB chunk),
       AIL_set_sequence_volume(music * 127 / 9), AIL_start_sequence, AIL_set_sequence_loop_count(0);
     per 120 Hz tick (6.6): the interpreter queues the tick's channel messages through 0x98F30 (a flush first when
       the 0x200-byte MIDI_data would overflow) and flushes them with one 0x502 at the end of the tick
       (0x9A6D0..0x9A712).
   The XMIDI interpreter is the host's own, translated here from AC_MAIN (disasm/AC_MAIN.linear.asm) rather than
   taken from port/src/xmiseq.c, because the order of the driver's writes depends on it: the timer 0x99F80 walks
   the EVNT bytes with a delay countdown, keeps a 32-slot note queue (first free slot at a note-on, note-offs in
   slot order at the start of each interval; a re-struck key takes a second slot and gets no note-off first), and
   sends every channel message through 0x99610 (controller shadow, CC100/101/38 0 before CC6, CC7 scaled, FOR/NEXT
   and the other AIL controllers consumed). */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/catalog.h"
#include "../src/dro.h"
#include "../src/fmdrv.h"
#include "../src/xmi.h"

#define CTX_LINES 20           /* writes of context on each side of a mismatch */
#define KGRAM 8                /* the seed length of the alignment search */

typedef struct { uint16_t reg; uint8_t val; int32_t tick; } pwrite;   /* tick -1 = written at install */

typedef struct {
    fmdrv *drv;
    uint8_t q[0x200];          /* DST+0x100, MIDI_data */
    int qbytes;                /* mdi+0x1AC */
    int qcount;                /* mdi+0x1A8 */
    pwrite *w;
    size_t nw, capw;
    int32_t tick;
    int oom;
    unsigned long xmits;
    unsigned long msgs;
} host;

/* ---- files ------------------------------------------------------------------------------------------------- */

static uint8_t *load_file(const char *path, size_t *size_out)
{
    FILE *f = fopen(path, "rb");
    long n;
    uint8_t *buf;

    *size_out = 0;
    if (f == NULL)
        return NULL;
    if (fseek(f, 0, SEEK_END) != 0 || (n = ftell(f)) < 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }
    buf = malloc(n > 0 ? (size_t)n : 1);
    if (buf == NULL || (n > 0 && fread(buf, 1, (size_t)n, f) != (size_t)n)) {
        fclose(f);
        free(buf);
        return NULL;
    }
    fclose(f);
    *size_out = (size_t)n;
    return buf;
}

static uint8_t *load_in_dir(const char *dir, const char *name, size_t *size_out)
{
    size_t dl = strlen(dir), nl = strlen(name);
    char *path = malloc(dl + nl + 2);
    uint8_t *data;

    if (path == NULL)
        return NULL;
    memcpy(path, dir, dl);
    path[dl] = '/';
    memcpy(path + dl + 1, name, nl + 1);
    data = load_file(path, size_out);
    free(path);
    return data;
}

static uint32_t rd_u32be(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

/* the first chunk `id` (TIMB, EVNT) inside a FORM XMID, walked as xmi.c walks it (FORM/CAT/LIST nest, odd sizes
   padded); *body = the chunk body (TIMB: u16 count, then patch, bank pairs; EVNT: the event bytes) */
static int find_chunk(const uint8_t *d, size_t start, size_t end, int in_xmid, const char *id, const uint8_t **body,
                      uint32_t *size)
{
    size_t pos = start;

    while (pos + 8 <= end) {
        uint32_t sz = rd_u32be(d + pos + 4);

        if (sz > end - pos - 8)
            sz = (uint32_t)(end - pos - 8);
        if (memcmp(d + pos, "FORM", 4) == 0 || memcmp(d + pos, "CAT ", 4) == 0 || memcmp(d + pos, "LIST", 4) == 0) {
            if (sz >= 4) {
                int x = in_xmid || memcmp(d + pos + 8, "XMID", 4) == 0;

                if (find_chunk(d, pos + 12, pos + 8 + sz, x, id, body, size))
                    return 1;
            }
        } else if (in_xmid && memcmp(d + pos, id, 4) == 0) {
            *body = d + pos + 8;
            *size = sz;
            return 1;
        }
        pos += 8 + (size_t)sz + (sz & 1u);
    }
    return 0;
}

/* ---- the host: the write sink, the MIDI queue 0x98F30 and the 0x502 flush --------------------------------- */

static void on_write(void *user, int reg, int val)
{
    host *h = user;

    if (h->nw == h->capw) {
        size_t ncap = h->capw == 0 ? 65536 : h->capw * 2;
        pwrite *nv = realloc(h->w, ncap * sizeof *nv);

        if (nv == NULL) {
            h->oom = 1;
            return;
        }
        h->w = nv;
        h->capw = ncap;
    }
    h->w[h->nw].reg = (uint16_t)(reg & 0x1FF);
    h->w[h->nw].val = (uint8_t)val;
    h->w[h->nw].tick = h->tick;
    h->nw++;
}

static void host_flush(host *h)                       /* 0x502 MIDI_XMIT with CX = the queued count */
{
    if (h->qcount > 0) {
        fmdrv_xmit(h->drv, h->q, h->qbytes);
        h->xmits++;
    }
    h->qcount = 0;
    h->qbytes = 0;
}

static void host_queue(host *h, int st, int d1, int d2)   /* 0x98F30 */
{
    int len = ((st & 0xF0) == 0xC0 || (st & 0xF0) == 0xD0) ? 2 : 3;

    if (h->qbytes + len > 0x200 && h->qcount > 0)     /* the buffer would overflow: flush first */
        host_flush(h);
    h->q[h->qbytes] = (uint8_t)st;
    h->q[h->qbytes + 1] = (uint8_t)d1;
    if (len == 3)
        h->q[h->qbytes + 2] = (uint8_t)d2;
    h->qbytes += len;
    h->qcount++;
    h->msgs++;
}

/* ---- the host's XMIDI interpreter (AC_MAIN; ailsem.md 7..8; sequence fields as S+offset) ------------------ */

typedef struct {
    host *h;
    const uint8_t *evnt;       /* S+0x10 + 8: the EVNT chunk body, the rewind target */
    uint32_t evnt_len;
    uint32_t ptr;              /* S+0x14: the next event byte, as an offset into evnt */
    int32_t status;            /* S+0x04: 2 done, 4 playing, 8 stopped */
    int32_t loop_count;        /* S+0x2C */
    int32_t countdown;         /* S+0x30: the delay countdown */
    int32_t tick_count;        /* S+0x34 */
    int32_t volume;            /* S+0x38: the sequence volume */
    int32_t tempo_pct;         /* S+0x48 */
    int32_t acc;               /* S+0x58: the tempo accumulator */
    int32_t for_ptr[4];        /* S+0x74: the FOR event's position */
    int32_t for_count[4];      /* S+0x84: -1 = free */
    int32_t shadow[16][128];   /* S+0xD4..: the controller shadow, -1 = never sent (raw values) */
    int32_t note_count;        /* S+0x554 */
    int32_t nq_ch[32];         /* S+0x558: -1 = free */
    int32_t nq_key[32];        /* S+0x5D8 */
    int32_t nq_rem[32];        /* S+0x658: intervals left */
    int end_hit;               /* [0x11B174]: FF 2F seen this tick, parsing stops */
    unsigned long rewinds, next_jumps, sysex;
} ail_seq;

static uint8_t ail_byte(const ail_seq *s, uint32_t p)
{
    return p < s->evnt_len ? s->evnt[p] : 0;
}

static uint32_t ail_vlq(const ail_seq *s, uint32_t *p)   /* 0x99110: up to 4 bytes */
{
    uint32_t v = 0;
    int n = 4;

    for (;;) {
        uint8_t b = ail_byte(s, *p);                      /* 0x99121 */

        (*p)++;                                           /* 0x99128 */
        v = (v << 7) | (b & 0x7Fu);                       /* 0x99123..0x9912E */
        if (!(b & 0x80u) || --n == 0)                     /* 0x99130..0x99136 */
            return v;
    }
}

/* 0x99610(S, status, d1, d2, flag): one channel message from the sequence to the driver queue. Channel mapping
   (S+0x94) and locks (mdi+0x20) are the identity / unlocked in the game; the callbacks (S+0x1C, S+0x20, S+0x24,
   mdi+0x1A0) are not set by the game. */
static void ail_msg(ail_seq *s, int st, int d1, int d2)
{
    int ch = st & 0x0F, kind = st & 0xF0;

    if (kind == 0xB0) {
        s->shadow[ch][d1 & 0x7F] = d2;                    /* 0x99676: the shadow (0x99440) keeps the raw value */
        switch (d1) {
        case 6:                                           /* 0x99782: CC100 0, CC101 0, CC38 0, then the CC6 */
            ail_msg(s, 0xB0 | ch, 100, 0);
            ail_msg(s, 0xB0 | ch, 101, 0);
            ail_msg(s, 0xB0 | ch, 38, 0);
            break;
        case 7:                                           /* 0x99810: raw * volume * master(127) / 16129, clamped */
            d2 = (int)((long)d2 * s->volume * 127 / 0x3F01);
            if (d2 > 127)
                d2 = 127;
            if (d2 < 0)
                d2 = 0;
            break;
        case 109:                                         /* 0x99943: branch (AIL_branch_index): consumed */
        case 110:                                         /* 0x9997F: channel lock: consumed */
        case 111:                                         /* 0x99954: channel protect: consumed */
        case 118:                                         /* 0x9983D: clear beat count: consumed */
        case 119:                                         /* 0x9987B: callback trigger: consumed */
            return;
        case 116: {                                       /* 0x99899: FOR, the first free of 4 slots */
            int i;

            for (i = 0; i < 4 && s->for_count[i] != -1; i++)
                ;
            if (i == 4)
                return;
            s->for_count[i] = d2;                         /* 0x998B8 */
            s->for_ptr[i] = (int32_t)s->ptr;              /* 0x998C2: the FOR event itself */
            return;
        }
        case 117: {                                       /* 0x998CD: NEXT, the innermost open FOR */
            int i;

            if (d2 < 64)
                return;
            for (i = 3; i >= 0 && s->for_count[i] == -1; i--)
                ;
            if (i < 0)
                return;
            if (s->for_count[i] == 0) {                   /* 0x99909: forever */
                s->ptr = (uint32_t)s->for_ptr[i];
                s->next_jumps++;
                return;
            }
            if (--s->for_count[i] != 0) {                 /* 0x99918..0x99928 */
                s->ptr = (uint32_t)s->for_ptr[i];
                s->next_jumps++;
                return;
            }
            s->for_count[i] = -1;                         /* 0x99932 */
            return;
        }
        default:                                          /* 107, 108, 112..115, 120.. and the rest: sent */
            break;
        }
    }
    if (kind == 0x90 && s->shadow[ch][107] >= 64)         /* 0x99A2C..0x99A40: mute */
        return;
    host_queue(s->h, st, d1, d2);                         /* 0x99A83 */
}

static void ail_stop(ail_seq *s)                          /* AIL_stop_sequence 0x85974 -> 0x9B620 */
{
    int i, ch;

    if (s->status != 4)
        return;
    s->status = 8;
    for (i = 0; i < 32; i++)                              /* 0x99AA0: note-off for each queued note */
        if (s->nq_ch[i] != -1) {
            ail_msg(s, 0x80 | s->nq_ch[i], s->nq_key[i], 0);
            s->nq_ch[i] = -1;
        }
    s->note_count = 0;                                    /* 0x99AFE */
    host_flush(s->h);                                     /* 0x99B08..0x99B3E */
    for (ch = 0; ch < 16; ch++) {                         /* ailsem 7: CC64 0 / CC112 0 where the shadow >= 64 */
        if (s->shadow[ch][64] >= 64)
            ail_msg(s, 0xB0 | ch, 64, 0);
        if (s->shadow[ch][112] >= 64)
            ail_msg(s, 0xB0 | ch, 112, 0);
    }
}

/* one 120 Hz timer call for this sequence (0x99FCE..0x9A6A9); 0 = the interpreter returned at once (0x9A4C4:
   note queue overflow, no end-of-tick flush) */
static int ail_tick(ail_seq *s)
{
    if (s->status != 4)                                   /* 0x99FD3 */
        return 1;
    s->tick_count++;                                      /* 0x99FE8 */
    s->acc += s->tempo_pct;                               /* 0x99FE9 */
    s->end_hit = 0;                                       /* 0x99FEE */
    while (s->acc >= 100) {                               /* 0x99FFF */
        int i;

        s->acc -= 100;                                    /* 0x9A008 */
        if (s->note_count > 0)                            /* 0x9A014: note durations first, in slot order */
            for (i = 0; i < 32; i++) {
                if (s->nq_ch[i] == -1)                    /* 0x9A03F */
                    continue;
                s->nq_rem[i] += -1;                       /* 0x9A047 */
                if (0 < s->nq_rem[i])                     /* 0x9A055 */
                    continue;
                ail_msg(s, 0x80 | s->nq_ch[i], s->nq_key[i], 0);   /* 0x9A059..0x9A073 */
                s->nq_ch[i] = -1;                         /* 0x9A083 */
                s->note_count += -1;                      /* 0x9A08A */
                if (s->note_count == 0)                   /* 0x9A09B */
                    break;
            }
        if (--s->countdown > 0)                           /* 0x9A0B5..0x9A0C3 */
            continue;
        for (;;) {                                        /* 0x9A0C9: parse up to a delay byte */
            uint32_t p = s->ptr;
            int b = ail_byte(s, p);

            if (b < 0x80 || s->end_hit) {                 /* 0x9A0DB / 0x9A0E6 -> 0x9A532 */
                if (!s->end_hit) {
                    s->ptr = p + 1;                       /* 0x9A543 */
                    s->countdown = b;                     /* 0x9A551 */
                }
                break;
            }
            if (b == 0xFF) {                              /* 0x9A121: meta */
                int type = ail_byte(s, p + 1);
                uint32_t len;

                p += 2;
                len = ail_vlq(s, &p);                     /* 0x9A134 */
                s->ptr = p;
                if (type == 0x2F) {                       /* 0x9A168: end of track */
                    s->end_hit = 1;
                    if (s->loop_count == 0 || --s->loop_count != 0) {   /* 0x9A17B..0x9A186 */
                        s->ptr = 0;                       /* 0x9A19B..0x9A1AB: back to EVNT+8 */
                        s->rewinds++;
                    } else {
                        ail_stop(s);                      /* 0x9A1C6 */
                        s->status = 2;                    /* 0x9A1D6 */
                    }
                }
                s->ptr += len;                            /* 0x9A1F9 / 0x9A23C / 0x9A2F2 */
                continue;
            }
            if (b == 0xF0 || b == 0xF7) {                 /* 0x9A2FA: sysex, copied whole to the driver */
                uint32_t q = p + 1, total;

                total = ail_vlq(s, &q);                   /* 0x9A30D */
                total += q - p;                           /* 0x9A32A..0x9A330 */
                host_flush(s->h);                         /* 0x9A33B */
                if (p + (total > 0x200 ? 0x200 : total) <= s->evnt_len) {   /* 0x9A35E..0x9A371: one message */
                    fmdrv_xmit(s->h->drv, s->evnt + p, (int)(total > 0x200 ? 0x200 : total));
                    s->h->xmits++;
                }
                s->sysex++;
                s->ptr = p + total;                       /* 0x9A389 */
                continue;
            }
            if (b > 0xF0) {                               /* 0x9A10C -> 0x9A391, length 0: never in the game */
                s->ptr = p + 1;
                continue;
            }
            ail_msg(s, b, ail_byte(s, p + 1), ail_byte(s, p + 2));   /* 0x9A3B6..0x9A3CE */
            if ((b & 0xF0) == 0x90) {                     /* 0x9A452: queue the note in the first free slot */
                for (i = 0; i < 32 && s->nq_ch[i] != -1; i++)
                    ;
                if (i == 32) {                            /* 0x9A48E: "Internal note queue overflow" */
                    ail_stop(s);
                    s->status = 2;
                    return 0;
                }
                s->note_count++;                          /* 0x9A4DE */
                s->nq_ch[i] = b & 0x0F;                   /* 0x9A4EB */
                s->nq_key[i] = ail_byte(s, s->ptr + 1);   /* 0x9A4FF */
                s->ptr += 3;                              /* 0x9A50C */
                s->nq_rem[i] = (int32_t)ail_vlq(s, &s->ptr);   /* 0x9A512..0x9A526 */
                continue;
            }
            {                                             /* 0x9A3E8..0x9A44A: the length of the byte at ptr now */
                int k = ail_byte(s, s->ptr) & 0xF0;

                s->ptr += (k == 0xC0 || k == 0xD0) ? 2 : (k >= 0x80 && k <= 0xE0) ? 3 : 0;
            }
        }
    }
    return 1;
}

/* AIL_init_sequence's reset 0x99E60 and AIL_start_sequence (ailsem 7); the loop count, volume, tempo are kept */
static void ail_reset(ail_seq *s)
{
    int i;

    s->ptr = 0;
    memset(s->shadow, 0xFF, sizeof s->shadow);
    for (i = 0; i < 4; i++)
        s->for_count[i] = -1;
    for (i = 0; i < 32; i++)
        s->nq_ch[i] = -1;
    s->note_count = 0;
    s->countdown = 0;
    s->tick_count = 0;
}

/* step 9 of AIL_install_MDI_driver (0x9AD90..0x9AE96): per channel 15 messages, flushed with 0x502 after each
   channel; AIL_delay(3) after channels 0, 4, 8 and 12 is time only */
static void host_channel_setup(host *h)
{
    static const uint8_t ccs[12][2] = {
        { 112, 0 }, { 1, 0 }, { 7, 127 }, { 10, 64 }, { 11, 127 }, { 64, 0 },
        { 91, 40 }, { 93, 0 }, { 100, 0 }, { 101, 0 }, { 38, 0 }, { 6, 2 }   /* CC7 = pref13, CC6 = pref16 */
    };
    int ch, i;

    for (ch = 0; ch < 16; ch++) {
        host_queue(h, 0xB0 | ch, 114, 0);
        host_queue(h, 0xC0 | ch, 0, 0);
        host_queue(h, 0xE0 | ch, 0x00, 0x40);
        for (i = 0; i < 12; i++)
            host_queue(h, 0xB0 | ch, ccs[i][0], ccs[i][1]);
        host_flush(h);
    }
}

/* ---- DOSBox-X's capture rules (G: inferred from the capture, see main) ------------------------------------- */

/* registers DOSBox-X's DRO writer has a code for: 01, 04, 05, 08, BD, the operator ranges 20/40/60/80/E0 + 0..15h
   (offsets 6, 7 of each 8 left out), A0-A8, B0-B8, C0-C8; bank 0 regs 02..04 are the timer chip's, never logged */
static int dbx_logged(int reg)
{
    int r = reg & 0xFF, bank = reg >> 8;
    static const int bases[5] = { 0x20, 0x40, 0x60, 0x80, 0xE0 };
    int i;

    if (bank == 0 && r >= 0x02 && r <= 0x04)
        return 0;
    if (r == 0x01 || r == 0x04 || r == 0x05 || r == 0x08 || r == 0xBD)
        return 1;
    for (i = 0; i < 5; i++)
        if (r >= bases[i] && r < bases[i] + 0x18 && ((r - bases[i]) & 7) < 6)
            return 1;
    if ((r >= 0xA0 && r <= 0xA8) || (r >= 0xB0 && r <= 0xB8) || (r >= 0xC0 && r <= 0xC8))
        return 1;
    return 0;
}

/* ---- the comparison ---------------------------------------------------------------------------------------- */

typedef struct { uint64_t h; uint32_t j; } kgram;

static int kgram_cmp(const void *a, const void *b)
{
    const kgram *x = a, *y = b;

    if (x->h != y->h)
        return x->h < y->h ? -1 : 1;
    return x->j < y->j ? -1 : (x->j > y->j);
}

static uint64_t khash(const uint32_t *k, int n)
{
    uint64_t h = 1469598103934665603ull;
    int i;

    for (i = 0; i < n; i++) {
        h ^= k[i];
        h *= 1099511628211ull;
    }
    return h;
}

/* the longest run of identical writes between D (m) and P (n): seeds of KGRAM writes, each maximal run extended
   once from its start. Returns its length; *bi, *bj its start in D and P. */
static size_t longest_run(const uint32_t *D, size_t m, const uint32_t *P, size_t n, size_t *bi, size_t *bj)
{
    int k = KGRAM;
    size_t nk, i, best = 0;
    kgram *tab;

    *bi = *bj = 0;
    if (m == 0 || n == 0)
        return 0;
    if ((size_t)k > m)
        k = (int)m;
    if ((size_t)k > n)
        k = (int)n;
    nk = n - (size_t)k + 1;
    tab = malloc(nk * sizeof *tab);
    if (tab == NULL)
        return 0;
    for (i = 0; i < nk; i++) {
        tab[i].h = khash(P + i, k);
        tab[i].j = (uint32_t)i;
    }
    qsort(tab, nk, sizeof *tab, kgram_cmp);
    for (i = 0; i + (size_t)k <= m; i++) {
        uint64_t h = khash(D + i, k);
        size_t lo = 0, hi = nk;

        while (lo < hi) {                             /* the first entry with hash >= h */
            size_t mid = lo + (hi - lo) / 2;

            if (tab[mid].h < h)
                lo = mid + 1;
            else
                hi = mid;
        }
        for (; lo < nk && tab[lo].h == h; lo++) {
            size_t j = tab[lo].j, L = 0;

            if (i > 0 && j > 0 && D[i - 1] == P[j - 1])
                continue;                             /* not the start of its run */
            while (i + L < m && j + L < n && D[i + L] == P[j + L])
                L++;
            if (L > best) {
                best = L;
                *bi = i;
                *bj = j;
            }
        }
    }
    free(tab);
    return best;
}

static void put_group(int reg, int *label)
{
    int r = reg & 0xFF;

    *label = (r < 0x20 || r == 0xBD) ? r : (r & 0xF0);
}

static void show_context(const char *title, long f, long off,
                         const uint16_t *dreg, const uint8_t *dval, const uint32_t *dms, size_t m,
                         const pwrite *const *pw, size_t n)
{
    long i;

    printf("\n%s (DRO write %ld = port write %ld):\n", title, f, f + off);
    printf("    DRO#   reg=val       ms |   port#   reg=val   tick        s\n");
    for (i = f - CTX_LINES; i <= f + CTX_LINES; i++) {
        long j = i + off;
        int hd = (i >= 0 && (size_t)i < m), hp = (j >= 0 && (size_t)j < n);
        int same = hd && hp && dreg[i] == pw[j]->reg && dval[i] == pw[j]->val;

        if (!hd && !hp)
            continue;
        printf("%c ", i == f ? '>' : (hd && hp && !same ? '*' : ' '));
        if (hd)
            printf("%6ld   %03X=%02X %8lu |", i, (unsigned)dreg[i], (unsigned)dval[i], (unsigned long)dms[i]);
        else
            printf("%6s   %6s %8s |", "", "", "");
        if (hp) {
            if (pw[j]->tick < 0)
                printf(" %7ld   %03X=%02X   inst\n", j, (unsigned)pw[j]->reg, (unsigned)pw[j]->val);
            else
                printf(" %7ld   %03X=%02X %6ld %8.3f\n", j, (unsigned)pw[j]->reg, (unsigned)pw[j]->val,
                       (long)pw[j]->tick, (double)pw[j]->tick / 120.0);
        } else {
            printf("\n");
        }
    }
}

/* the length of DOSBox-X's state dump: the t = 0 writes up to the first register written a second time (the dump
   writes each cached register once; a repeat at t = 0 is a real write that fell in the capture's first ms) */
static uint32_t dro_dump_len(const dro_file *df)
{
    uint8_t seen[0x200];
    uint32_t i;

    memset(seen, 0, sizeof seen);
    for (i = 0; i < df->count; i++) {
        int reg = (df->ev[i].bank << 8) | df->ev[i].reg;

        if (df->ev[i].delay_ms != 0)
            break;
        if (seen[reg & 0x1FF])
            break;
        seen[reg & 0x1FF] = 1;
    }
    return i;
}

static void usage(void)
{
    fprintf(stderr, "usage: fm_dro_cmp [-raw] [-vol N] [-dump N] GAMEDIR SONG.XMI DRO [SECONDS]\n");
}

int main(int argc, char **argv)
{
    const char *pos[4];
    int npos = 0, raw = 0, vol_cfg = -1, a, rc = 1;
    long dump_opt = -1;
    double seconds = 0.0;
    uint8_t *ct5 = NULL, *bank = NULL, *cfg = NULL, *drodata = NULL, *mdi = NULL;
    size_t ct5_len, bank_len, cfg_len, dro_len, mdi_len;
    catalog *cat = NULL;
    const cat_entry *ce;
    const uint8_t *xmi;
    xmi_song song;
    int have_song = 0;
    const uint8_t *timb = NULL;
    uint32_t timb_len = 0;
    const uint8_t *evnt = NULL;
    uint32_t evnt_len = 0;
    ail_seq *seq = NULL;
    host h;
    dro_file df;
    int have_dro = 0;
    int music, seqvol;
    uint32_t song_ticks, total_ticks, t;
    pwrite **pw = NULL;
    size_t n = 0, i, n_install = 0;
    uint8_t cache[0x200];
    uint32_t dump, m = 0;
    uint16_t *dreg = NULL;
    uint8_t *dval = NULL;
    uint32_t *dms = NULL, *dkey = NULL, *pkey = NULL;
    size_t L, bi, bj;
    long off, first_bad = -1;
    size_t inorder = 0, ondiag = 0;
    static unsigned long gd[256][2], gp[256][2];

    memset(&h, 0, sizeof h);
    memset(&song, 0, sizeof song);
    memset(&df, 0, sizeof df);
    for (a = 1; a < argc; a++) {
        if (strcmp(argv[a], "-raw") == 0) {
            raw = 1;
        } else if (strcmp(argv[a], "-vol") == 0 && a + 1 < argc) {
            vol_cfg = atoi(argv[++a]);
        } else if (strcmp(argv[a], "-dump") == 0 && a + 1 < argc) {
            dump_opt = atol(argv[++a]);
        } else if (npos < 4) {
            pos[npos++] = argv[a];
        } else {
            usage();
            return 2;
        }
    }
    if (npos < 3) {
        usage();
        return 2;
    }
    if (npos == 4)
        seconds = atof(pos[3]);

    /* ---- the data ---- */
    ct5 = load_in_dir(pos[0], "ACIV.CT5", &ct5_len);
    bank = load_in_dir(pos[0], "EHGM2FM2.OPL", &bank_len);
    mdi = load_in_dir(pos[0], "SBPRO2.MDI", &mdi_len);   /* the driver's tables (fmdrv_new) */
    if (ct5 == NULL || bank == NULL || mdi == NULL) {
        fprintf(stderr, "fm_dro_cmp: cannot read ACIV.CT5, EHGM2FM2.OPL or SBPRO2.MDI in %s\n", pos[0]);
        rc = 2;
        goto done;
    }
    cat = malloc(sizeof *cat);
    if (cat == NULL || cat_open(cat, ct5, ct5_len) != 0 || (ce = cat_find(cat, pos[1])) == NULL
        || (xmi = cat_data(cat, ce)) == NULL) {
        fprintf(stderr, "fm_dro_cmp: %s not found in ACIV.CT5\n", pos[1]);
        rc = 2;
        goto done;
    }
    if (xmi_parse(xmi, ce->length, &song) != 0 || song.count == 0) {
        fprintf(stderr, "fm_dro_cmp: %s does not parse as XMIDI\n", pos[1]);
        rc = 2;
        goto done;
    }
    have_song = 1;
    if (!find_chunk(xmi, 0, ce->length, 0, "EVNT", &evnt, &evnt_len) || evnt_len == 0) {
        fprintf(stderr, "fm_dro_cmp: %s has no EVNT chunk\n", pos[1]);
        rc = 2;
        goto done;
    }
    if (!find_chunk(xmi, 0, ce->length, 0, "TIMB", &timb, &timb_len) || timb_len < 2) {
        fprintf(stderr, "fm_dro_cmp: %s has no TIMB chunk\n", pos[1]);
        rc = 2;
        goto done;
    }
    music = 9;                                        /* ACIV.CFG +2 = [0xF097E]: music volume in bits 0..6 */
    cfg = load_in_dir(pos[0], "ACIV.CFG", &cfg_len);
    if (cfg != NULL && cfg_len >= 4)
        music = cfg[2] & 0x7F;
    if (vol_cfg >= 0)
        music = vol_cfg;
    seqvol = music * 127 / 9;                         /* 0x7B9FB..0x7BA27 */
    drodata = load_file(pos[2], &dro_len);
    if (drodata == NULL || dro_parse(drodata, dro_len, &df) != 0) {
        fprintf(stderr, "fm_dro_cmp: cannot read the DRO %s\n", pos[2]);
        rc = 2;
        goto done;
    }
    have_dro = 1;

    song_ticks = song.ev[song.count - 1].tick;
    if (seconds <= 0.0)
        seconds = (double)song_ticks / 120.0 + (double)df.length_ms / 1000.0 + 5.0;
    total_ticks = (uint32_t)(seconds * 120.0 + 0.5);

    /* ---- install (structure.md 6.4) ---- */
    h.tick = -1;
    h.drv = fmdrv_new(on_write, &h, mdi, mdi_len);    /* 0x305 INIT_DEV: chip reset 0x2C47 + state 0x2FE1 */
    if (h.drv == NULL) {
        fprintf(stderr, "fm_dro_cmp: SBPRO2.MDI is not the shipped driver, or no memory\n");
        rc = 2;
        goto done;
    }
    if (!fmdrv_set_bank(h.drv, bank, bank_len)) {      /* 0x501 INIT_INS_MGR: the GTL file */
        fprintf(stderr, "fm_dro_cmp: EHGM2FM2.OPL rejected by fmdrv_set_bank\n");
        rc = 2;
        goto done;
    }
    host_channel_setup(&h);                           /* step 9: 16 x (15 messages, 0x502) */

    /* ---- the song (f7B854 0x7B9B6..0x7BA47) ---- */
    host_flush(&h);                                   /* AIL_init_sequence: flush queued MIDI (0x9B512) */
    if (!fmdrv_install_timbres(h.drv, timb, timb_len)) {   /* 0x503 with the TIMB chunk */
        int bt = fmdrv_failed_timbre(h.drv);

        printf("Driver could not install timbre bank %u, patch %u\n", (unsigned)(bt >> 8) & 0xFF, (unsigned)bt & 0xFF);
        rc = 2;
        goto done;
    }
    seq = calloc(1, sizeof *seq);
    if (seq == NULL) {
        fprintf(stderr, "fm_dro_cmp: no memory\n");
        rc = 2;
        goto done;
    }
    seq->h = &h;
    seq->evnt = evnt;
    seq->evnt_len = evnt_len;
    seq->loop_count = 1;                              /* AIL_allocate_sequence_handle's defaults */
    seq->tempo_pct = 100;
    seq->volume = 127;
    ail_reset(seq);                                   /* AIL_init_sequence */
    seq->status = 2;
    seq->volume = seqvol;                             /* AIL_set_sequence_volume(v, 0): no CC7 sent yet to rescale */
    ail_reset(seq);                                   /* AIL_start_sequence: rewind and play */
    seq->status = 4;
    seq->loop_count = 0;                              /* AIL_set_sequence_loop_count(0): forever */

    /* ---- the 120 Hz timer 0x99F80: one interval per tick at 100 %, then one 0x502 ---- */
    for (t = 1; t <= total_ticks && !h.oom; t++) {
        h.tick = (int32_t)t;
        if (ail_tick(seq))
            host_flush(&h);                           /* 0x9A6D0..0x9A712 */
    }
    if (h.oom) {
        fprintf(stderr, "fm_dro_cmp: no memory for the write log\n");
        rc = 2;
        goto done;
    }

    /* ---- the port's sequence as DOSBox-X would log it ---- */
    pw = malloc((h.nw > 0 ? h.nw : 1) * sizeof *pw);
    if (pw == NULL) {
        rc = 2;
        goto done;
    }
    memset(cache, 0, sizeof cache);                   /* DOSBox-X's register cache, zero at power-on */
    for (i = 0; i < h.nw; i++) {
        int reg = h.w[i].reg;

        if (!raw) {
            int timer = (reg >> 8) == 0 && (reg & 0xFF) >= 2 && (reg & 0xFF) <= 4;
            int keep = dbx_logged(reg) && cache[reg] != h.w[i].val;

            if (!timer)
                cache[reg] = h.w[i].val;
            if (!keep)
                continue;
        }
        pw[n++] = &h.w[i];
    }

    /* ---- the DRO after its state dump ---- */
    dump = dro_dump_len(&df);
    if (dump_opt >= 0)
        dump = dump_opt > (long)df.count ? df.count : (uint32_t)dump_opt;
    m = df.count - dump;
    dreg = malloc((m > 0 ? m : 1) * sizeof *dreg);
    dval = malloc((m > 0 ? m : 1) * sizeof *dval);
    dms = malloc((m > 0 ? m : 1) * sizeof *dms);
    dkey = malloc((m > 0 ? m : 1) * sizeof *dkey);
    pkey = malloc((n > 0 ? n : 1) * sizeof *pkey);
    if (dreg == NULL || dval == NULL || dms == NULL || dkey == NULL || pkey == NULL) {
        rc = 2;
        goto done;
    }
    {
        uint64_t ms = 0;
        uint8_t dc[0x200];
        uint8_t known[0x200];
        unsigned long self_dup = 0, self_dup_b0 = 0, unlogged = 0;

        memset(dc, 0, sizeof dc);
        memset(known, 0, sizeof known);
        for (i = 0; i < df.count; i++) {
            int reg = (df.ev[i].bank << 8) | df.ev[i].reg;

            ms += df.ev[i].delay_ms;
            if (i < dump) {
                dc[reg & 0x1FF] = df.ev[i].value;
                known[reg & 0x1FF] = 1;
                continue;
            }
            dreg[i - dump] = (uint16_t)(reg & 0x1FF);
            dval[i - dump] = df.ev[i].value;
            dms[i - dump] = (uint32_t)ms;
            dkey[i - dump] = ((uint32_t)(reg & 0x1FF) << 8) | df.ev[i].value;
            if (!dbx_logged(reg))
                unlogged++;
            if (known[reg & 0x1FF] && dc[reg & 0x1FF] == df.ev[i].value) {
                if ((reg & 0xFF) >= 0xB0 && (reg & 0xFF) <= 0xB8 && known[reg & 0x1FF] == 1)
                    self_dup_b0++;                    /* the first B0 write after the dump */
                else
                    self_dup++;
            }
            dc[reg & 0x1FF] = df.ev[i].value;
            known[reg & 0x1FF] = 2;
        }
        printf("DRO  %s: %lu writes, %lu ms, hardware %u\n", pos[2], (unsigned long)df.count,
               (unsigned long)df.length_ms, (unsigned)df.hardware);
        printf("     state dump: %lu writes at t = 0%s; compared: %lu writes from %lu ms to %lu ms\n",
               (unsigned long)dump, dump_opt >= 0 ? " (-dump)" : " (t = 0 run up to the first repeated register)",
               (unsigned long)m, m ? (unsigned long)dms[0] : 0ul, m ? (unsigned long)dms[m - 1] : 0ul);
        if (dump_opt < 0 && dump < df.count && df.ev[dump].delay_ms == 0)
            printf("     note: write %lu (%X%02X=%02X) is also at t = 0 but repeats a dumped register: a real write\n",
                   (unsigned long)dump, (unsigned)df.ev[dump].bank, (unsigned)df.ev[dump].reg,
                   (unsigned)df.ev[dump].value);
        printf("     capture-rule check on the DRO itself: %lu writes to unlogged registers, %lu writes equal to the\n"
               "     register's last value (+%lu first B0-B8 writes equal to the dump's value)\n",
               unlogged, self_dup, self_dup_b0);
    }
    for (i = 0; i < n; i++)
        pkey[i] = ((uint32_t)pw[i]->reg << 8) | pw[i]->val;
    while (n_install < h.nw && h.w[n_install].tick < 0)
        n_install++;

    printf("PORT %s at music volume %d (sequence volume %d): %.1f s = %lu ticks, %lu 0x502 calls, %lu messages\n",
           pos[1], music, seqvol, seconds, (unsigned long)total_ticks, h.xmits, h.msgs);
    printf("     %lu register writes (%lu at install), %lu after DOSBox-X's capture rules%s; song pass %lu ticks, "
           "%lu FF 2F rewinds, %lu NEXT jumps, %lu sysex\n", (unsigned long)h.nw,
           (unsigned long)n_install,
           (unsigned long)n, raw ? " (-raw: none applied)" : "", (unsigned long)song_ticks,
           seq->rewinds, seq->next_jumps, seq->sysex);

    /* ---- alignment ---- */
    L = longest_run(dkey, m, pkey, n, &bi, &bj);
    if (L == 0) {
        printf("\nno alignment: no write of the DRO occurs in the port's sequence\n");
        rc = 1;
        goto groups;
    }
    off = (long)bj - (long)bi;                        /* DRO write i <-> port write i + off */
    printf("\nALIGNMENT: longest identical run %lu writes, DRO %lu..%lu = port %lu..%lu\n", (unsigned long)L,
           (unsigned long)bi, (unsigned long)(bi + L - 1), (unsigned long)bj, (unsigned long)(bj + L - 1));
    printf("           offset: DRO write 0 = port write %ld", off);
    if (off >= 0 && (size_t)off < n) {
        if (pw[off]->tick >= 0)
            printf(" (tick %ld, %.3f s into the song)", (long)pw[off]->tick, (double)pw[off]->tick / 120.0);
        else
            printf(" (at install)");
    }
    printf("\n");
    for (i = 0; i < m; i++) {
        long j = (long)i + off;
        int same = j >= 0 && (size_t)j < n && dkey[i] == pkey[j];

        if (same) {
            ondiag++;
            if (first_bad < 0)
                inorder++;
        } else if (first_bad < 0) {
            first_bad = (long)i;
        }
    }
    printf("MATCH:     %lu of %lu DRO writes match in order from the alignment; %lu equal on that diagonal\n",
           (unsigned long)inorder, (unsigned long)m, (unsigned long)ondiag);
    if (first_bad < 0) {
        printf("           every DRO write after the dump matches\n");
        rc = 0;
    } else {
        long j = first_bad + off;

        if (j >= 0 && (size_t)j < n)
            printf("FIRST MISMATCH at DRO write %ld (%lu ms): DRO %03X=%02X, port %03X=%02X (tick %ld)\n", first_bad,
                   (unsigned long)dms[first_bad], (unsigned)dreg[first_bad], (unsigned)dval[first_bad],
                   (unsigned)pw[j]->reg, (unsigned)pw[j]->val, (long)pw[j]->tick);
        else
            printf("FIRST MISMATCH at DRO write %ld (%lu ms): the port's sequence %s there\n", first_bad,
                   (unsigned long)dms[first_bad], j < 0 ? "has not started" : "has ended (raise SECONDS)");
        show_context("first mismatch", first_bad, off, dreg, dval, dms, m, (const pwrite *const *)pw, n);
        if ((size_t)first_bad < bi && bi + L < m)
            show_context("end of the longest run", (long)(bi + L), off, dreg, dval, dms, m,
                         (const pwrite *const *)pw, n);
        rc = 1;
    }

groups:
    /* ---- per register group: the DRO after the dump, and the port over the same span from the alignment ---- */
    for (i = 0; i < m; i++) {
        int g;

        put_group(dreg[i], &g);
        gd[g][dreg[i] >> 8]++;
    }
    {
        long lo = L > 0 ? (long)bj - (long)bi : 0, hi = lo + (long)m, j;

        if (lo < 0)
            lo = 0;
        if (hi > (long)n)
            hi = (long)n;
        for (j = lo; j < hi; j++) {
            int g;

            put_group(pw[j]->reg, &g);
            gp[g][pw[j]->reg >> 8]++;
        }
        printf("\nREGISTER GROUPS: DRO writes %lu..%lu vs port writes %ld..%ld\n", 0ul,
               m ? (unsigned long)m - 1 : 0ul, lo, hi - 1);
        printf("  group      DRO bank0 bank1  total |  port bank0 bank1  total\n");
        for (a = 0; a < 256; a++) {
            unsigned long td = gd[a][0] + gd[a][1], tp = gp[a][0] + gp[a][1];
            char lab[8];

            if (td == 0 && tp == 0)
                continue;
            if (a < 0x20 || a == 0xBD)
                snprintf(lab, sizeof lab, "%02X", a);
            else
                snprintf(lab, sizeof lab, "%02X-%02X", a, a + 0x0F);
            printf("  %-6s %c      %5lu %5lu %6lu |       %5lu %5lu %6lu\n", lab, td != tp ? '*' : ' ',
                   gd[a][0], gd[a][1], td, gp[a][0], gp[a][1], tp);
        }
    }

done:
    free(seq);
    if (h.drv != NULL)
        fmdrv_free(h.drv);
    free(h.w);
    free(pw);
    free(dreg);
    free(dval);
    free(dms);
    free(dkey);
    free(pkey);
    if (have_dro)
        dro_free(&df);
    if (have_song)
        xmi_free(&song);
    free(cat);
    free(ct5);
    free(bank);
    free(mdi);
    free(cfg);
    free(drodata);
    return rc;
}
