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
typedef struct { int stop_at, resume_at, rewind_at; } script;

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
        size_t hi, xi, n;
        int ok = 1;

        xr.group = step;                               /* tag this batch's emissions */
        xmi_host_tick(h);                              /* host call #step+1 (its recs carry tick step+1) */
        xmi_seq_advance(s, flush ? 0u : 1u);           /* xmi batch #step (advance(0) first: the tick-0 flush) */
        flush = 0;
        hi = xmi_host_count(h);
        n = hi - h0;
        if (n != xr.n - x0)
            ok = 0;
        else {
            int32_t htick = xmi_host_tickno(h);      /* every rec of this group carries this tick */

            for (xi = 0; xi < n; xi++) {
                const xmi_host_rec *a = &xmi_host_recs(h)[h0 + xi];
                const xmi_host_rec *b = &xr.v[x0 + xi];

                if (a->len != b->len || memcmp(a->msg, b->msg, (size_t)a->len) != 0 || a->tick != htick)
                    ok = 0;
            }
        }
        if (!ok) {
            fails++;
            printf("FAIL %s step %d (host tick %d): %zu host msgs vs %zu xmi msgs\n", name, step, step + 1,
                   n, xr.n - x0);
            for (xi = 0; xi < n && xi < 8; xi++)
                printf("    host[%zu] tick %d len %d %02X %02X %02X | xmi %02X %02X %02X\n", xi,
                       xmi_host_recs(h)[h0 + xi].tick, xmi_host_recs(h)[h0 + xi].len,
                       xmi_host_recs(h)[h0 + xi].msg[0], xmi_host_recs(h)[h0 + xi].msg[1],
                       xmi_host_recs(h)[h0 + xi].msg[2],
                       xr.v[x0 + xi].msg[0], xr.v[x0 + xi].msg[1], xr.v[x0 + xi].msg[2]);
        }
        if (sc != NULL) {
            if (step == sc->stop_at) {
                size_t h1 = xmi_host_count(h), x1 = xr.n;

                xmi_host_stop(h);
                xmi_seq_pause(s);
                for (xi = x1; xi < xr.n; xi++)
                    xr.v[xi].tick = step;
                check(xmi_host_count(h) - h1 == xr.n - x1, "stop emission counts equal");
                for (xi = 0; xi < xr.n - x1 && xi < xmi_host_count(h) - h1; xi++)
                    check(xmi_host_recs(h)[h1 + xi].len == xr.v[x1 + xi].len &&
                          memcmp(xmi_host_recs(h)[h1 + xi].msg, xr.v[x1 + xi].msg,
                                 (size_t)xr.v[x1 + xi].len) == 0, "stop emission bytes equal");
            }
            if (step == sc->resume_at) {
                xmi_host_start(h);                     /* AIL_resume_sequence: status 8 -> 4 */
                xmi_seq_resume(s);
            }
            if (step == sc->rewind_at) {
                size_t h1 = xmi_host_count(h), x1 = xr.n;

                xmi_host_stop(h);                      /* AIL_start_sequence: stop if playing, reset, play */
                xmi_host_reset(h);
                xmi_host_start(h);
                xmi_seq_rewind(s);
                xmi_seq_resume(s);
                flush = 1;
                for (xi = x1; xi < xr.n; xi++)
                    xr.v[xi].tick = step;
                check(xmi_host_count(h) - h1 == xr.n - x1, "rewind emission counts equal");
                for (xi = 0; xi < xr.n - x1 && xi < xmi_host_count(h) - h1; xi++)
                    check(xmi_host_recs(h)[h1 + xi].len == xr.v[x1 + xi].len &&
                          memcmp(xmi_host_recs(h)[h1 + xi].msg, xr.v[x1 + xi].msg,
                                 (size_t)xr.v[x1 + xi].len) == 0, "rewind emission bytes equal");
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
    /* S6: stop (pause), resume, rewind mid-song on the S1 shape */
    {
        bb e = { 0, 0, 0 };
        script sc = { 6, 10, 14 };

        ev_cc(&e, 0, 0, 64, 127);          /* sustain down: the stop must send CC64 0 */
        ev_cc(&e, 0, 3, 112, 100);         /* and CC112 0 */
        ev_note(&e, 0, 0, 60, 90, 8);
        ev_note(&e, 2, 1, 62, 90, 20);
        ev_eot(&e, 30);
        run_case("S6 stop/resume/rewind", e.b, e.n, 0, 100, 30, &sc);
        free(e.b);
    }

    if (fails == 0)
        printf("xmiseq_cmp_test: all checks pass\n");
    else
        printf("xmiseq_cmp_test: %d FAILURES\n", fails);
    return fails ? 1 : 0;
}
