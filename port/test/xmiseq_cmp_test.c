/* xmiseq_cmp_test: port/src/xmiseq.c must emit exactly the channel-message stream of the game's own XMIDI
   interpreter (the host model in xmi_host.c, copied from fm_dro_cmp.c): same message bytes, same order, same
   120 Hz tick. The host is driven one ail_tick per step; xmiseq one advance per step (the first advance is
   advance(0), the tick-0 flush, which pairs with the host's first tick). Each step's emissions are compared.
   Synthetic EVNT songs hit the queue slot order, re-strikes, duration 0, the FF 2F rewind, FOR/NEXT, the CC6
   prefix, CC7 scaling, the CC107 mute, the consumed controllers, the queue overflow, and stop/resume/rewind.
   Run: xmiseq_cmp_test   (no arguments, no files) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/xmi.h"
#include "../src/xmiseq.h"
#include "xmi_host.h"

static int fails = 0;

static void check(int cond, const char *msg)
{
    if (!cond) {
        fails++;
        printf("FAIL %s\n", msg);
    }
}

/* ---------- byte builders ---------- */
typedef struct { uint8_t *b; size_t n, cap; } bb;

static void bb_u8(bb *x, int v)
{
    if (x->n == x->cap) {
        x->cap = x->cap ? x->cap * 2 : 256;
        x->b = realloc(x->b, x->cap);
    }
    x->b[x->n++] = (uint8_t)v;
}

static void bb_vlq(bb *x, uint32_t v)
{
    uint8_t t[8];
    int n = 0, i;

    do {
        t[n++] = (uint8_t)(v & 0x7Fu);
        v >>= 7;
    } while (v != 0);
    for (i = n - 1; i >= 0; i--)
        bb_u8(x, t[i] | (i > 0 ? 0x80 : 0));
}

static void bb_u32be(bb *x, uint32_t v)
{
    bb_u8(x, (int)(v >> 24)); bb_u8(x, (int)(v >> 16)); bb_u8(x, (int)(v >> 8)); bb_u8(x, (int)v);
}

static void bb_bytes(bb *x, const void *p, size_t k)
{
    size_t i;

    for (i = 0; i < k; i++)
        bb_u8(x, ((const uint8_t *)p)[i]);
}

static void bb_plain(bb *x, const char *id, const uint8_t *body, size_t blen)
{
    bb_bytes(x, id, 4);
    bb_u32be(x, (uint32_t)blen);
    bb_bytes(x, body, blen);
    if (blen & 1u)
        bb_u8(x, 0);
}

static void bb_chunk(bb *x, const char *id, const char *type, const uint8_t *body, size_t blen)
{
    bb_bytes(x, id, 4);
    bb_u32be(x, (uint32_t)blen + 4);
    bb_bytes(x, type, 4);
    bb_bytes(x, body, blen);
    if ((blen + 4) & 1u)
        bb_u8(x, 0);
}

/* a FORM XMID holding TIMB + EVNT, for xmi_parse */
static uint8_t *make_xmi(const uint8_t *evnt, size_t elen, size_t *out_len)
{
    bb body = { 0, 0, 0 }, top = { 0, 0, 0 };
    static const uint8_t timb[4] = { 1, 0, 0, 0 };

    bb_plain(&body, "TIMB", timb, sizeof timb);
    bb_plain(&body, "EVNT", evnt, elen);
    bb_chunk(&top, "FORM", "XMID", body.b, body.n);
    free(body.b);
    *out_len = top.n;
    return top.b;
}

static void ev_note(bb *e, int delta, int ch, int key, int vel, uint32_t dur)
{
    bb_vlq(e, (uint32_t)delta);
    bb_u8(e, 0x90 | ch);
    bb_u8(e, key);
    bb_u8(e, vel);
    bb_vlq(e, dur);
}

static void ev_cc(bb *e, int delta, int ch, int num, int val)
{
    bb_vlq(e, (uint32_t)delta);
    bb_u8(e, 0xB0 | ch);
    bb_u8(e, num);
    bb_u8(e, val);
}

static void ev_other(bb *e, int delta, int st, int d1, int d2)
{
    bb_vlq(e, (uint32_t)delta);
    bb_u8(e, st);
    bb_u8(e, d1);
    bb_u8(e, d2);
}

static void ev_eot(bb *e, int delta)
{
    bb_vlq(e, (uint32_t)delta);
    bb_u8(e, 0xFF);
    bb_u8(e, 0x2F);
    bb_u8(e, 0);
}

/* ---------- the xmiseq recorder ---------- */
typedef struct { xmi_host_rec *v; size_t n, cap; int group; } xrec;

static void xrec_out(void *user, const uint8_t *msg, int len)
{
    xrec *r = user;

    if (r->n == r->cap) {
        r->cap = r->cap ? r->cap * 2 : 1024;
        r->v = realloc(r->v, r->cap * sizeof *r->v);
    }
    memcpy(r->v[r->n].msg, msg, (size_t)len);
    r->v[r->n].len = len;
    r->v[r->n].tick = r->group;
    r->n++;
}

/* ---------- one differential case ---------- */
typedef struct { int stop_at, resume_at, stop2_at, rewind_at, end_at; } script;   /* -1 = never */

/* compare the emissions of one script action (stop / resume / rewind) between host and xmiseq */
static void cmp_emissions(const char *what, const xmi_host *h, size_t h1, const xrec *xr, size_t x1)
{
    size_t xi, hn = xmi_host_count(h) - h1, xn = xr->n - x1;
    char msg[128];

    snprintf(msg, sizeof msg, "%s emission counts equal (%zu vs %zu)", what, hn, xn);
    check(hn == xn, msg);
    for (xi = 0; xi < hn && xi < xn; xi++) {
        const xmi_host_rec *a = &xmi_host_recs(h)[h1 + xi];
        const xmi_host_rec *b = &xr->v[x1 + xi];

        snprintf(msg, sizeof msg, "%s emission %zu bytes equal", what, xi);
        check(a->len == b->len && memcmp(a->msg, b->msg, (size_t)a->len) == 0, msg);
    }
}

static void run_case(const char *name, const uint8_t *evnt, size_t elen, int loops, int volume, int steps,
                     const script *sc)
{
    xmi_host *h = xmi_host_new(evnt, elen);
    uint8_t *xmi_img;
    size_t xmi_len;
    xmi_song song;
    xmi_seq *s;
    xrec xr;
    int flush = 1;
    int32_t playing = 0;                   /* host calls made while playing: the host ticks once per such call */
    int step;

    memset(&xr, 0, sizeof xr);
    xmi_img = make_xmi(evnt, elen, &xmi_len);
    check(xmi_parse(xmi_img, xmi_len, &song) == 0, "parse the synthetic song");
    s = xmi_seq_new(&song, xrec_out, NULL, &xr);
    check(s != NULL, "xmi_seq_new");

    xmi_host_set_loops(h, loops);
    xmi_host_set_volume(h, volume);
    xmi_host_reset(h);
    xmi_host_start(h);
    xmi_seq_set_loops(s, loops);
    xmi_seq_set_volume(s, volume);

    for (step = 0; step < steps; step++) {
        size_t h0 = xmi_host_count(h), x0 = xr.n;
        size_t hi, xi, n, xn;
        int ok = 1;

        if (xmi_host_status(h) == 4)
            playing++;
        xr.group = step;                               /* tag this batch's emissions */
        xmi_host_tick(h);                              /* host call #step+1 */
        xmi_seq_advance(s, flush ? 0u : 1u);           /* xmi batch #step (advance(0) first: the tick-0 flush) */
        flush = 0;
        hi = xmi_host_count(h);
        n = hi - h0;
        xn = xr.n - x0;
        /* the tick claim: the batch pairing itself (step k <-> host call k+1) is what the per-batch count and
           byte compare below proves; the host recs' own tick tags come from the same host counter, so they
           are not compared against xmiseq here */
        if (n != xn)
            ok = 0;
        else
            for (xi = 0; xi < n; xi++) {
                const xmi_host_rec *a = &xmi_host_recs(h)[h0 + xi];
                const xmi_host_rec *b = &xr.v[x0 + xi];

                if (a->len != b->len || memcmp(a->msg, b->msg, (size_t)a->len) != 0)
                    ok = 0;
            }
        check(xmi_host_tickno(h) == playing, "the host ticks once per playing call (tick pairing)");
        if (!ok) {
            fails++;
            printf("FAIL %s step %d (host tick %d): %zu host msgs vs %zu xmi msgs\n", name, step,
                   xmi_host_tickno(h), n, xn);
            for (xi = 0; xi < n && xi < xn && xi < 8; xi++)   /* bound by BOTH counts */
                printf("    host[%zu] tick %d len %d %02X %02X %02X | xmi %02X %02X %02X\n", xi,
                       xmi_host_recs(h)[h0 + xi].tick, xmi_host_recs(h)[h0 + xi].len,
                       xmi_host_recs(h)[h0 + xi].msg[0], xmi_host_recs(h)[h0 + xi].msg[1],
                       xmi_host_recs(h)[h0 + xi].msg[2],
                       xr.v[x0 + xi].msg[0], xr.v[x0 + xi].msg[1], xr.v[x0 + xi].msg[2]);
            if (n > xn)
                printf("    ... %zu more host msgs than xmi sent\n", n - xn);
        }
        if (sc != NULL) {
            if (step == sc->stop_at || step == sc->stop2_at) {
                size_t h1 = xmi_host_count(h), x1 = xr.n;

                xmi_host_stop(h);
                xmi_seq_pause(s);
                cmp_emissions(step == sc->stop2_at ? "second stop" : "stop", h, h1, &xr, x1);
            }
            if (step == sc->resume_at) {
                size_t h1 = xmi_host_count(h), x1 = xr.n;

                xmi_host_resume(h);                    /* AIL_resume_sequence: the 0x99C20 replay, 8 -> 4 */
                xmi_seq_resume(s);
                cmp_emissions("resume replay", h, h1, &xr, x1);
            }
            if (step == sc->rewind_at) {
                size_t h1 = xmi_host_count(h), x1 = xr.n;

                xmi_host_stop(h);                      /* AIL_start_sequence: stop if playing, reset, play */
                xmi_host_reset(h);
                xmi_host_start(h);
                xmi_seq_rewind(s);
                xmi_seq_resume(s);
                flush = 1;
                cmp_emissions("rewind", h, h1, &xr, x1);
            }
            if (step == sc->end_at) {
                size_t h1 = xmi_host_count(h), x1 = xr.n;

                xmi_host_stop(h);                      /* AIL_end_sequence: the gated stop (0x9B632) */
                xmi_seq_stop(s);
                cmp_emissions("end", h, h1, &xr, x1);
            }
        }
    }
    check(xmi_host_count(h) == xr.n, "total message counts equal");

    xmi_seq_free(s);
    xmi_free(&song);
    free(xmi_img);
    xmi_host_free(h);
    free(xr.v);
}

int main(void)
{
    /* S1: queue slot order, re-strike (second slot, no note-off), duration 0 (off at the next interval) */
    {
        bb e = { 0, 0, 0 };

        ev_note(&e, 0, 5, 60, 90, 3);
        ev_note(&e, 0, 0, 90, 90, 3);
        ev_note(&e, 0, 0, 60, 90, 10);
        ev_note(&e, 1, 0, 60, 90, 2);      /* re-strike of (0,60) */
        ev_note(&e, 1, 2, 64, 90, 0);      /* duration 0 */
        ev_eot(&e, 17);
        run_case("S1 slots/restrike/dur0", e.b, e.n, 1, 127, 25, NULL);
        free(e.b);
    }
    /* S2: the FF 2F rewind restarts at the next interval (lead-in delay counts again) */
    {
        bb e = { 0, 0, 0 };

        ev_note(&e, 2, 0, 60, 90, 5);
        ev_eot(&e, 3);
        run_case("S2 eot rewind", e.b, e.n, 0, 127, 30, NULL);
        free(e.b);
    }
    /* S3: FOR 0 / NEXT 127 (forever) and FOR 2 / NEXT 127 (two passes) */
    {
        bb e = { 0, 0, 0 };

        ev_cc(&e, 0, 0, 116, 0);
        ev_note(&e, 0, 0, 60, 90, 4);
        ev_cc(&e, 2, 0, 117, 127);
        ev_eot(&e, 10);
        run_case("S3a FOR0/NEXT127", e.b, e.n, 1, 127, 25, NULL);
        free(e.b);
    }
    {
        bb e = { 0, 0, 0 };

        ev_cc(&e, 0, 0, 116, 2);
        ev_note(&e, 0, 0, 61, 90, 2);
        ev_cc(&e, 1, 0, 117, 127);
        ev_eot(&e, 5);
        run_case("S3b FOR2/NEXT127", e.b, e.n, 1, 127, 15, NULL);
        free(e.b);
    }
    /* S4: CC6's CC100/101/38 prefix, CC7 scaling at volume 112, the CC107 mute, consumed controllers,
       forwarded 112/120, program change and pitch bend lengths */
    {
        bb e = { 0, 0, 0 };

        ev_cc(&e, 0, 0, 7, 100);
        ev_cc(&e, 0, 0, 6, 5);
        ev_cc(&e, 0, 0, 107, 127);
        ev_note(&e, 0, 0, 60, 90, 3);      /* muted: no message, still queued */
        ev_cc(&e, 0, 0, 107, 0);
        ev_note(&e, 0, 0, 62, 90, 3);
        ev_cc(&e, 0, 0, 109, 1);           /* consumed */
        ev_cc(&e, 0, 0, 112, 70);          /* forwarded, and the stop pedal shadow */
        ev_cc(&e, 0, 0, 120, 9);           /* forwarded */
        ev_other(&e, 0, 0xC0, 5, 0);       /* program change: 2 bytes */
        ev_other(&e, 0, 0xE0, 0x00, 0x40);  /* pitch bend */
        ev_eot(&e, 5);
        run_case("S4 controllers", e.b, e.n, 1, 112, 15, NULL);
        free(e.b);
    }
    /* S5: the 32-slot queue overflow stops the sequence: 33 one-tick-apart notes of duration 100, so at the
       33rd note-on all 32 slots are still sounding (the note-on is sent, then the stop emission) */
    {
        bb e = { 0, 0, 0 };
        int i;

        for (i = 0; i < 33; i++)
            ev_note(&e, i == 0 ? 0 : 1, i & 15, 40 + (i & 15), 90, 100);
        ev_eot(&e, 5);
        run_case("S5 queue overflow", e.b, e.n, 1, 127, 45, NULL);
        free(e.b);
    }
    /* S6: stop (pause), resume, rewind mid-song on the S1 shape; a note-on is due at tick 9, while paused
       (steps 7..9): time must not pass while paused, and the rewind at 14 happens with a note and the
       CC64/CC112 shadows still down */
    {
        bb e = { 0, 0, 0 };
        script sc = { 6, 10, -1, 14, -1 };

        ev_cc(&e, 0, 0, 64, 127);          /* sustain down: the stop must send CC64 0 */
        ev_cc(&e, 0, 3, 112, 100);         /* and CC112 0 */
        ev_note(&e, 0, 0, 60, 90, 8);
        ev_note(&e, 2, 1, 62, 90, 20);
        ev_note(&e, 6, 2, 65, 90, 3);      /* due at tick 9: after the pause, before the resume */
        ev_eot(&e, 24);
        run_case("S6 stop/resume/rewind", e.b, e.n, 0, 100, 30, &sc);
        free(e.b);
    }
    /* S7: raw EVNT bytes (the builders always write a delay byte; these do not). S7a: two notes in ONE
       interval whose offs fall due together, with slot order (ch2,key 0x40) != (channel,key) order (ch0,0x3C):
       92 40 5A 03 90 3C 5A 03 05 FF 2F 00. S7b: an event at tick 0 with no leading delay: the advance(0)
       tick-0 flush must send it in batch 0. S7c: a real 0x8n note-off in the stream, forwarded where it stands:
       90 3C 5A 05 80 3C 00 03 FF 2F 00 */
    {
        static const uint8_t s7a[] = { 0x92, 0x40, 0x5A, 0x03, 0x90, 0x3C, 0x5A, 0x03, 0x05, 0xFF, 0x2F, 0x00 };
        static const uint8_t s7b[] = { 0x90, 0x3C, 0x5A, 0x05, 0xFF, 0x2F, 0x00 };
        static const uint8_t s7c[] = { 0x90, 0x3C, 0x5A, 0x05, 0x80, 0x3C, 0x00, 0x03, 0xFF, 0x2F, 0x00 };

        run_case("S7a raw slot order", s7a, sizeof s7a, 1, 127, 12, NULL);
        run_case("S7b raw tick-0 flush", s7b, sizeof s7b, 1, 127, 10, NULL);
        run_case("S7c raw note-off forwarded", s7c, sizeof s7c, 1, 127, 10, NULL);
    }
    /* S8: the resume replay (item 1) and the second stop (item 2). Raw EVNT, every event in ONE interval
       (no delay bytes), so the stop at step 4 catches all shadows set: program change 5, pitch bend, CC7,
       CC6, CC64 >= 64, CC112 >= 64, a note; stop, resume, stop again (the shadows kept their values, so the
       second stop sends CC64 0 / CC112 0 again) */
    {
        static const uint8_t s8[] = {
            0xC0, 0x05,                         /* program change 5 on ch0 */
            0xE0, 0x00, 0x40,                   /* pitch bend */
            0xB0, 0x07, 0x64,                   /* CC7 100 (scaled by the volume 112) */
            0xB0, 0x06, 0x05,                   /* CC6 5 (with its CC100/101/38 prefix) */
            0xB0, 0x40, 0x7F,                   /* CC64 127: sustain down */
            0xB0, 0x70, 0x64,                   /* CC112 100 */
            0x90, 0x3C, 0x5A, 0x08,             /* note ch0 key 60 vel 90 dur 8 */
            0x20, 0xFF, 0x2F, 0x00              /* EOT at tick 32 */
        };
        script sc = { 4, 8, 12, -1, -1 };

        run_case("S8 resume replay/second stop", s8, sizeof s8, 0, 112, 20, &sc);
    }
    /* S9a: the stop's status gate (0x9B632): pause (stop, status 8) then END (AIL_end_sequence) - the end
       must send nothing (the host's stop acts only while playing), even though the pedal shadows are kept */
    {
        static const uint8_t s9a[] = {
            0xB0, 0x40, 0x7F,                   /* CC64 127 */
            0xB0, 0x70, 0x64,                   /* CC112 100 */
            0x90, 0x3C, 0x5A, 0x64,             /* note dur 100 */
            0x20, 0xFF, 0x2F, 0x00              /* EOT at tick 32 (loops 0: never reached) */
        };
        script sc = { 4, -1, -1, -1, 8 };       /* stop at 4, end at 8 */

        run_case("S9a end after pause sends nothing", s9a, sizeof s9a, 0, 127, 12, &sc);
    }
    /* S9b: play to the natural end (EOT, loop count 1: the end emission fires once), then rewind - the
       rewind must send nothing more (the host's stop is gated on status 4; the sequence is done, status 2) */
    {
        bb e = { 0, 0, 0 };
        script sc = { -1, -1, -1, 8, -1 };

        ev_cc(&e, 0, 0, 64, 127);
        ev_cc(&e, 0, 0, 112, 100);
        ev_note(&e, 0, 0, 60, 90, 2);
        ev_eot(&e, 3);
        run_case("S9b rewind after the natural end", e.b, e.n, 1, 127, 16, &sc);
        free(e.b);
    }
    /* S9c: the natural end, then the music toggle (stop, resume): both send nothing. AIL_stop_sequence and
       AIL_resume_sequence are gated on status 4 and 8 (0x9B632, 0x9B6F0); the done sequence is status 2, so
       the stored program and controllers are not replayed. (The game's 12 songs loop forever on CC116/117
       and never reach their end, so this is pinned here only.) */
    {
        bb e = { 0, 0, 0 };
        script sc = { 8, 10, -1, -1, -1 };

        ev_cc(&e, 0, 0, 7, 100);
        ev_cc(&e, 0, 0, 10, 40);
        ev_cc(&e, 0, 0, 64, 127);
        ev_note(&e, 0, 0, 60, 90, 2);
        ev_eot(&e, 3);
        run_case("S9c stop and resume after the natural end", e.b, e.n, 1, 127, 16, &sc);
        free(e.b);
    }
    /* S10: pin the WHOLE 0x99C20 replay set and order: every replayed controller on TWO channels (CC114,
       CC107 < 64 so nothing is muted, CC111 consumed, CC112, CC1, CC7, CC10, CC11, CC64, CC91, CC93, CC6,
       program, bend), all in one interval; stop@4, resume@8 replays all of both channels in order;
       rewind@12 then resume must replay NOTHING (0x99E60's -1 fill cleared the stored state) */
    {
        static const uint8_t s10[] = {
            0xB0, 0x72, 0x05,  0xB0, 0x6B, 0x3C,  0xB0, 0x6F, 0x07,  0xB0, 0x70, 0x64,
            0xB0, 0x01, 0x08,  0xB0, 0x07, 0x64,  0xB0, 0x0A, 0x40,  0xB0, 0x0B, 0x7F,
            0xB0, 0x40, 0x7F,  0xB0, 0x5B, 0x28,  0xB0, 0x5D, 0x09,  0xB0, 0x06, 0x05,
            0xC0, 0x05,  0xE0, 0x00, 0x40,  0x90, 0x3C, 0x5A, 0x08,
            0xB1, 0x72, 0x06,  0xB1, 0x6B, 0x3C,  0xB1, 0x6F, 0x07,  0xB1, 0x70, 0x64,
            0xB1, 0x01, 0x08,  0xB1, 0x07, 0x64,  0xB1, 0x0A, 0x40,  0xB1, 0x0B, 0x7F,
            0xB1, 0x40, 0x7F,  0xB1, 0x5B, 0x28,  0xB1, 0x5D, 0x09,  0xB1, 0x06, 0x05,
            0xC1, 0x06,  0xE1, 0x10, 0x50,  0x91, 0x3E, 0x5A, 0x08,
            0x20, 0xFF, 0x2F, 0x00              /* EOT at tick 32 */
        };
        script sc = { 4, 8, -1, 12, -1 };

        run_case("S10 whole replay set, two channels", s10, sizeof s10, 0, 112, 20, &sc);
    }

    if (fails == 0)
        printf("xmiseq_cmp_test: all checks pass\n");
    else
        printf("xmiseq_cmp_test: %d FAILURES\n", fails);
    return fails ? 1 : 0;
}
