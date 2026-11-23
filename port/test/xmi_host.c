/* The host XMIDI interpreter, copied from port/test/fm_dro_cmp.c (which is proven against a DOSBox-X capture)
   with host_queue/host_flush replaced by a recorder. Do not "fix" anything here: it is the reference.
   How this copy differs from fm_dro_cmp.c's host: (1) its tick does not advance while the sequence is not
   playing (fm_dro_cmp's tick is the driver's global one); (2) a sysex is recorded truncated to 256 bytes;
   (3) the stop's CC64 0 / CC112 0 go to the queue directly, not through ail_msg, so the shadow keeps its value
   (the same fix as fm_dro_cmp.c's commit 22d9743); (4) program change and pitch bend are stored per channel
   (0x99440) and xmi_host_resume replays the stored state (0x9B6F0 / 0x99C20), which fm_dro_cmp.c does not model
   (it has no resume; the replay order and storage slots are from the task's address listing). */

#include "xmi_host.h"

#include <stdlib.h>
#include <string.h>

struct xmi_host {
    const uint8_t *evnt;
    uint32_t evnt_len;
    uint32_t ptr;
    int32_t status;            /* 2 done, 4 playing, 8 stopped */
    int32_t loop_count;
    int32_t countdown;
    int32_t tick_count;
    int32_t volume;
    int32_t tempo_pct;
    int32_t acc;
    int32_t for_ptr[4];
    int32_t for_count[4];      /* -1 = free */
    int32_t shadow[16][128];
    int32_t prog[16];          /* 0x99440: [S+0xD4+4*ch], -1 = never sent */
    int32_t bend[16][2];       /* 0x99440: d1 [S+0x114+4*ch], d2 [S+0x154+4*ch], -1 = never sent */
    int32_t note_count;
    int32_t nq_ch[32];
    int32_t nq_key[32];
    int32_t nq_rem[32];
    int end_hit;
    unsigned long rewinds, next_jumps, sysex;
    /* the recorder (was host->q / fmdrv_xmit) */
    xmi_host_rec *rec;
    size_t nrec, caprec;
    int32_t tick;              /* the current 120 Hz tick (host->tick) */
};

static uint8_t ail_byte(const xmi_host *s, uint32_t p)
{
    return p < s->evnt_len ? s->evnt[p] : 0;
}

static uint32_t ail_vlq(const xmi_host *s, uint32_t *p)   /* 0x99110: up to 4 bytes */
{
    uint32_t v = 0;
    int n = 4;

    for (;;) {
        uint8_t b = ail_byte(s, *p);

        (*p)++;
        v = (v << 7) | (b & 0x7Fu);
        if (!(b & 0x80u) || --n == 0)
            return v;
    }
}

static void host_queue(xmi_host *s, int st, int d1, int d2)   /* 0x98F30, recorded instead of sent */
{
    int len = ((st & 0xF0) == 0xC0 || (st & 0xF0) == 0xD0) ? 2 : 3;
    xmi_host_rec *r;

    if (s->nrec == s->caprec) {
        size_t ncap = s->caprec ? s->caprec * 2 : 1024;
        xmi_host_rec *nr = realloc(s->rec, ncap * sizeof *nr);

        if (nr == NULL)
            return;
        s->rec = nr;
        s->caprec = ncap;
    }
    r = &s->rec[s->nrec++];
    r->msg[0] = (uint8_t)st;
    r->msg[1] = (uint8_t)d1;
    if (len == 3)
        r->msg[2] = (uint8_t)d2;
    r->len = len;
    r->tick = s->tick;
}

static void ail_msg(xmi_host *s, int st, int d1, int d2)   /* 0x99610 */
{
    int ch = st & 0x0F, kind = st & 0xF0;

    if (kind == 0xC0)                                 /* 0x99440 stores the program change */
        s->prog[ch] = d1;
    else if (kind == 0xE0) {                          /* ... and the pitch bend (both data bytes) */
        s->bend[ch][0] = d1;
        s->bend[ch][1] = d2;
    }
    if (kind == 0xB0) {
        s->shadow[ch][d1 & 0x7F] = d2;
        switch (d1) {
        case 6:
            ail_msg(s, 0xB0 | ch, 100, 0);
            ail_msg(s, 0xB0 | ch, 101, 0);
            ail_msg(s, 0xB0 | ch, 38, 0);
            break;
        case 7: {
            long v = (long)d2 * s->volume * 127 / 0x3F01;

            if (v > 127)
                v = 127;
            if (v < 0)
                v = 0;
            d2 = (int)v;
            break;
        }
        case 109:
        case 110:
        case 111:
        case 118:
        case 119:
            return;
        case 116: {
            int i;

            for (i = 0; i < 4 && s->for_count[i] != -1; i++)
                ;
            if (i == 4)
                return;
            s->for_count[i] = d2;
            s->for_ptr[i] = (int32_t)s->ptr;
            return;
        }
        case 117: {
            int i;

            if (d2 < 64)
                return;
            for (i = 3; i >= 0 && s->for_count[i] == -1; i--)
                ;
            if (i < 0)
                return;
            if (s->for_count[i] == 0) {
                s->ptr = (uint32_t)s->for_ptr[i];
                s->next_jumps++;
                return;
            }
            if (--s->for_count[i] != 0) {
                s->ptr = (uint32_t)s->for_ptr[i];
                s->next_jumps++;
                return;
            }
            s->for_count[i] = -1;
            return;
        }
        default:
            break;
        }
    }
    if (kind == 0x90 && s->shadow[ch][107] >= 64)         /* 0x99A2C: mute */
        return;
    host_queue(s, st, d1, d2);                         /* 0x99A83 */
}

static void ail_stop(xmi_host *s)                          /* AIL_stop_sequence 0x85974 -> 0x9B620 / 0x99AA0 */
{
    int i, ch;

    if (s->status != 4)
        return;
    s->status = 8;
    for (i = 0; i < 32; i++)
        if (s->nq_ch[i] != -1) {
            ail_msg(s, 0x80 | s->nq_ch[i], s->nq_key[i], 0);
            s->nq_ch[i] = -1;
        }
    s->note_count = 0;
    for (ch = 0; ch < 16; ch++) {                     /* ailsem 7: CC64 0 / CC112 0 where the shadow >= 64, */
        if (s->shadow[ch][64] >= 64)                  /* queued through 0x98F30 directly (0x9B67B, 0x9B6A7), */
            host_queue(s, 0xB0 | ch, 64, 0);          /* not 0x99610: the shadow keeps its value, so a later */
        if (s->shadow[ch][112] >= 64)                 /* stop sends them again and the resume replay (0x99C20) */
            host_queue(s, 0xB0 | ch, 112, 0);         /* puts the pedal down again */
    }
}

int xmi_host_tick(xmi_host *s)                             /* 0x99FCE..0x9A6A9 */
{
    if (s->status != 4)
        return 1;
    s->tick_count++;
    s->tick++;
    s->acc += s->tempo_pct;
    s->end_hit = 0;
    while (s->acc >= 100) {
        int i;

        s->acc -= 100;
        if (s->note_count > 0)
            for (i = 0; i < 32; i++) {
                if (s->nq_ch[i] == -1)
                    continue;
                s->nq_rem[i] += -1;
                if (0 < s->nq_rem[i])
                    continue;
                ail_msg(s, 0x80 | s->nq_ch[i], s->nq_key[i], 0);
                s->nq_ch[i] = -1;
                s->note_count += -1;
                if (s->note_count == 0)
                    break;
            }
        if (--s->countdown > 0)
            continue;
        for (;;) {
            uint32_t p = s->ptr;
            int b = ail_byte(s, p);

            if (b < 0x80 || s->end_hit) {
                if (!s->end_hit) {
                    s->ptr = p + 1;
                    s->countdown = b;
                }
                break;
            }
            if (b == 0xFF) {
                int type = ail_byte(s, p + 1);
                uint32_t len;

                p += 2;
                len = ail_vlq(s, &p);
                s->ptr = p;
                if (type == 0x2F) {
                    s->end_hit = 1;
                    if (s->loop_count == 0 || --s->loop_count != 0) {
                        s->ptr = 0;
                        s->rewinds++;
                    } else {
                        ail_stop(s);
                        s->status = 2;
                    }
                }
                s->ptr += len;
                continue;
            }
            if (b == 0xF0 || b == 0xF7) {                 /* sysex: flushed, goes out alone */
                uint32_t q = p + 1, total;

                total = ail_vlq(s, &q);
                total += q - p;
                if (p + (total > 0x200 ? 0x200 : total) <= s->evnt_len) {
                    /* recorded as one message */
                    size_t k, n = total > 0x200 ? 0x200 : total;

                    if (s->nrec == s->caprec) {
                        size_t ncap = s->caprec ? s->caprec * 2 : 1024;
                        xmi_host_rec *nr = realloc(s->rec, ncap * sizeof *nr);

                        if (nr == NULL)
                            return 1;
                        s->rec = nr;
                        s->caprec = ncap;
                    }
                    {
                        xmi_host_rec *r = &s->rec[s->nrec++];

                        for (k = 0; k < n && k < sizeof r->msg; k++)
                            r->msg[k] = s->evnt[p + k];
                        r->len = (int)(n < sizeof r->msg ? n : sizeof r->msg);
                        r->tick = s->tick;
                    }
                    s->sysex++;
                }
                s->ptr = p + total;
                continue;
            }
            if (b > 0xF0) {
                s->ptr = p + 1;
                continue;
            }
            ail_msg(s, b, ail_byte(s, p + 1), ail_byte(s, p + 2));
            if ((b & 0xF0) == 0x90) {
                for (i = 0; i < 32 && s->nq_ch[i] != -1; i++)
                    ;
                if (i == 32) {
                    ail_stop(s);
                    s->status = 2;
                    return 0;
                }
                s->note_count++;
                s->nq_ch[i] = b & 0x0F;
                s->nq_key[i] = ail_byte(s, s->ptr + 1);
                s->ptr += 3;
                s->nq_rem[i] = (int32_t)ail_vlq(s, &s->ptr);
                continue;
            }
            {
                int k = ail_byte(s, s->ptr) & 0xF0;

                s->ptr += (k == 0xC0 || k == 0xD0) ? 2 : (k >= 0x80 && k <= 0xE0) ? 3 : 0;
            }
        }
    }
    return 1;
}

xmi_host *xmi_host_new(const uint8_t *evnt, size_t evnt_len)
{
    xmi_host *s = calloc(1, sizeof *s);

    if (s == NULL)
        return NULL;
    s->evnt = evnt;
    s->evnt_len = (uint32_t)evnt_len;
    s->loop_count = 1;
    s->tempo_pct = 100;
    s->volume = 127;
    s->tick = 0;
    xmi_host_reset(s);
    s->status = 2;
    return s;
}

void xmi_host_free(xmi_host *s)
{
    if (s == NULL)
        return;
    free(s->rec);
    free(s);
}

void xmi_host_reset(xmi_host *s)                           /* 0x99E60 */
{
    int i, ch;

    s->ptr = 0;
    memset(s->shadow, 0xFF, sizeof s->shadow);
    for (ch = 0; ch < 16; ch++) {
        s->prog[ch] = -1;
        s->bend[ch][0] = -1;
        s->bend[ch][1] = -1;
    }
    for (i = 0; i < 4; i++)
        s->for_count[i] = -1;
    for (i = 0; i < 32; i++)
        s->nq_ch[i] = -1;
    s->note_count = 0;
    s->countdown = 0;
    s->tick_count = 0;
}

void xmi_host_start(xmi_host *s) { s->status = 4; }
void xmi_host_stop(xmi_host *s) { ail_stop(s); }

/* AIL_resume_sequence 0x9B6F0, only when the status is 8: the channel locks (0x9B720..0x9B757, CC110 shadow
   >= 64 re-locks a channel via 0x87235) are NOT modelled (CC110 is consumed; no shipped song sends it);
   0x9B759..0x9B76E: 0x99C20(S, ch) for ch = 0..15, then status 4. 0x99C20 replays the channel's stored state
   through 0x99610 in the code's order (0x99C34..0x99E4B), each only where it was sent (-1 = never): CC114
   [S+0x294], program change [S+0xD4], pitch bend (d1 [S+0x114], d2 [S+0x154], gated on the stored d2),
   CC107 [S+0x214], CC111 [S+0x1D4], CC112 [S+0x254], CC1 [S+0x354], CC7 [S+0x394], CC10 [S+0x3D4],
   CC11 [S+0x414], CC64 [S+0x454], CC91 [S+0x494], CC93 [S+0x4D4], CC6 [S+0x514] (+4*ch each). */
void xmi_host_resume(xmi_host *s)
{
    static const int ccs[] = { 107, 111, 112, 1, 7, 10, 11, 64, 91, 93, 6 };
    int ch, i;

    if (s->status != 8)
        return;
    for (ch = 0; ch < 16; ch++) {
        if (s->shadow[ch][114] >= 0)
            ail_msg(s, 0xB0 | ch, 114, s->shadow[ch][114]);
        if (s->prog[ch] >= 0)
            ail_msg(s, 0xC0 | ch, s->prog[ch], 0);
        if (s->bend[ch][1] >= 0)
            ail_msg(s, 0xE0 | ch, s->bend[ch][0], s->bend[ch][1]);
        for (i = 0; i < (int)(sizeof ccs / sizeof ccs[0]); i++)
            if (s->shadow[ch][ccs[i]] >= 0)
                ail_msg(s, 0xB0 | ch, ccs[i], s->shadow[ch][ccs[i]]);
    }
    s->status = 4;
}

void xmi_host_set_loops(xmi_host *s, int n) { s->loop_count = n; }
void xmi_host_set_volume(xmi_host *s, int v) { s->volume = v; }
int32_t xmi_host_status(const xmi_host *s) { return s->status; }

size_t xmi_host_count(const xmi_host *s) { return s->nrec; }
const xmi_host_rec *xmi_host_recs(const xmi_host *s) { return s->rec; }
void xmi_host_clear_recs(xmi_host *s) { s->nrec = 0; }

int32_t xmi_host_tickno(const xmi_host *h) { return h->tick; }
