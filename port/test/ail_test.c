/* Tests for TASK 22 (ail.c, musplay.c, the mixer and xmiseq additions) on synthetic data only: no GAMEDIR, no
   SoundFont, no SDL. Run: ail_test   (build with the port's rules: C11 -Wall -Wextra -Werror -pedantic)
   Covers: mix_start/mix_active/mix_halt/mix_volume/mix_loops (ids, generations, stale ids, the loop rule at
   run-out, busy voices, empty samples, the 0-frame voice fix), the xmiseq additions (controller shadows, CC7
   scaling and resend, pause/resume/rewind, the FF 2F loop counts and the count-0 quirk, NEXT below 64 ignored),
   musplay (probe/load walk, statuses, the set-up messages, mus_render), and the ail facade (handles, statuses,
   the live/not-live rule, error texts, preferences, the lock balance). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/ail.h"
#include "../src/mixer.h"
#include "../src/musplay.h"
#include "../src/wav.h"
#include "../src/xmi.h"
#include "../src/xmiseq.h"

static int fails = 0;

static void check(int cond, const char *msg)
{
    if (!cond) {
        fails++;
        printf("FAIL %s\n", msg);
    }
}

/* ---------- little builders ---------- */
static void put16le(uint8_t *p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static void put32le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24);
}

static uint8_t *make_wav(uint32_t rate, uint16_t ch, uint16_t bits, uint32_t frames, int fill, size_t *len_out)
{
    uint32_t dbytes = frames * ch * (bits / 8u);
    size_t total = 12 + 8 + 16 + 8 + dbytes;
    uint8_t *b = calloc(1, total);
    size_t p = 0;
    uint32_t i;

    memcpy(b + p, "RIFF", 4); p += 4;
    put32le(b + p, (uint32_t)(total - 8)); p += 4;
    memcpy(b + p, "WAVE", 4); p += 4;
    memcpy(b + p, "fmt ", 4); p += 4;
    put32le(b + p, 16); p += 4;
    put16le(b + p, 1); p += 2;
    put16le(b + p, ch); p += 2;
    put32le(b + p, rate); p += 4;
    put32le(b + p, rate * ch * (bits / 8u)); p += 4;
    put16le(b + p, (uint16_t)(ch * (bits / 8u))); p += 2;
    put16le(b + p, bits); p += 2;
    memcpy(b + p, "data", 4); p += 4;
    put32le(b + p, dbytes); p += 4;
    for (i = 0; i < frames * ch; i++) {
        if (bits == 16) {
            put16le(b + p, (uint16_t)(int16_t)(fill + (int)i));
            p += 2;
        } else {
            b[p++] = (uint8_t)(fill + (int)i);
        }
    }
    *len_out = total;
    return b;
}

typedef struct { uint8_t *b; size_t n, cap; } bb;

static void bb_need(bb *x, size_t k)
{
    if (x->n + k > x->cap) {
        size_t nc = x->cap ? x->cap * 2 : 256;
        while (nc < x->n + k)
            nc *= 2;
        x->b = realloc(x->b, nc);
        x->cap = nc;
    }
}

static void bb_u8(bb *x, int v) { bb_need(x, 1); x->b[x->n++] = (uint8_t)v; }
static void bb_bytes(bb *x, const void *p, size_t k) { bb_need(x, k); memcpy(x->b + x->n, p, k); x->n += k; }
static void bb_u32be(bb *x, uint32_t v)
{
    bb_u8(x, (int)(v >> 24)); bb_u8(x, (int)(v >> 16)); bb_u8(x, (int)(v >> 8)); bb_u8(x, (int)v);
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

static void bb_plain(bb *x, const char *id, const uint8_t *body, size_t blen)   /* a plain chunk: id + size + data */
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

static void ev_cc(bb *e, uint32_t delta, int ch, int num, int val)
{
    bb_vlq(e, delta);
    bb_u8(e, 0xB0 | ch);
    bb_u8(e, num);
    bb_u8(e, val);
}

static void ev_note(bb *e, uint32_t delta, int ch, int key, int vel, uint32_t dur)
{
    bb_vlq(e, delta);
    bb_u8(e, 0x90 | ch);
    bb_u8(e, key);
    bb_u8(e, vel);
    bb_vlq(e, dur);
}

static void ev_eot(bb *e, uint32_t delta)
{
    bb_vlq(e, delta);
    bb_u8(e, 0xFF);
    bb_u8(e, 0x2F);
    bb_u8(e, 0);
}

static const uint8_t TIMB_BODY[4] = { 1, 0, 0, 0 };   /* one timbre: patch 0, bank 0 */

/* a FORM XMID sequence holding TIMB + EVNT */
static uint8_t *make_form_xmid(const uint8_t *evnt, size_t elen, int with_evnt, size_t *out_len)
{
    bb body = { 0, 0, 0 };
    bb top = { 0, 0, 0 };

    bb_plain(&body, "TIMB", TIMB_BODY, sizeof TIMB_BODY);
    if (with_evnt)
        bb_plain(&body, "EVNT", evnt, elen);
    bb_chunk(&top, "FORM", "XMID", body.b, body.n);
    free(body.b);
    *out_len = top.n;
    return top.b;
}

/* a file of top chunks: optionally FORM XDIR, then CAT XMID with nseq FORM XMID sequences */
static uint8_t *make_xmi(const uint8_t *const *evnts, const size_t *elens, int nseq, int with_xdir, size_t *out_len)
{
    bb catbody = { 0, 0, 0 };
    bb top = { 0, 0, 0 };
    int i;

    for (i = 0; i < nseq; i++) {
        size_t sl;
        uint8_t *sf = make_form_xmid(evnts[i], elens[i], 1, &sl);

        bb_chunk(&catbody, "FORM", "XMID", sf, sl);
        free(sf);
    }
    if (with_xdir) {
        bb xdir = { 0, 0, 0 };
        uint8_t info[1] = { 'x' };

        bb_plain(&xdir, "INFO", info, 1);
        bb_chunk(&top, "FORM", "XDIR", xdir.b, xdir.n);
        free(xdir.b);
    }
    bb_chunk(&top, "CAT ", "XMID", catbody.b, catbody.n);
    free(catbody.b);
    *out_len = top.n;
    return top.b;
}

/* ---------- a recording backend / out-callback ---------- */
typedef struct {
    uint8_t msg[8192][3];
    int len[8192];
    int n;
    int resets;
    int renders;
    int render_frames;
} rec_t;

static void rec_midi(void *u, const uint8_t *msg, int len)
{
    rec_t *r = u;

    if (r->n < 8192 && len <= 3) {
        memcpy(r->msg[r->n], msg, (size_t)len);
        r->len[r->n] = len;
        r->n++;
    }
}

static void rec_render(void *u, int16_t *out, int frames)
{
    rec_t *r = u;
    int i;

    r->renders++;
    r->render_frames += frames;
    for (i = 0; i < frames * 2; i++)
        out[i] = 1234;
}

static void rec_reset(void *u) { ((rec_t *)u)->resets++; }

static int rec_count(const rec_t *r, int b0, int d1)   /* messages with msg[0]==b0, msg[1]==d1 */
{
    int i, n = 0;

    for (i = 0; i < r->n; i++)
        if (r->msg[i][0] == b0 && (d1 < 0 || r->msg[i][1] == d1))
            n++;
    return n;
}

static int rec_find_at(const rec_t *r, int idx, int b0, int d1, int d2)
{
    return idx < r->n && r->msg[idx][0] == b0 && r->msg[idx][1] == d1 && r->msg[idx][2] == d2;
}

static int rec_count3(const rec_t *r, int b0, int d1, int d2)   /* messages equal to b0 d1 d2 */
{
    int i, n = 0;

    for (i = 0; i < r->n; i++)
        if (r->msg[i][0] == b0 && r->msg[i][1] == d1 && r->msg[i][2] == d2)
            n++;
    return n;
}

/* ---------- the lock counter ---------- */
static int g_lock_balance = 0;

static void test_lock(int on) { g_lock_balance += on ? 1 : -1; }

/* ---------- mixer ---------- */
static void test_mixer(void)
{
    mixer *m = mix_new();
    wav_sound w, w0;
    uint8_t *wb, *wb0;
    size_t wl, wl0;
    static int16_t out[256];
    int id, id2, v, i, ids[8];

    wb = make_wav(44100, 1, 16, 5, 100, &wl);
    check(wav_decode(wb, wl, &w) == 0 && w.frames == 5 && w.samples != NULL, "wav: 5 frames decode");
    wb0 = make_wav(44100, 1, 16, 0, 0, &wl0);
    check(wav_decode(wb0, wl0, &w0) == 0 && w0.frames == 0 && w0.samples == NULL, "wav: 0 frames decode");

    id = mix_start(m, &w, 127, 1);
    check(id >= 0, "mix_start returns an id");
    check(mix_active(m, id) == 1, "active before the data runs out");
    mix_render(m, out, 5);
    check(mix_active(m, id) == 0, "loops 1: not active once the data ran out");

    id = mix_start(m, &w, 127, 0);
    mix_render(m, out, 5);
    check(mix_active(m, id) == 1, "loops 0: still active at run-out");
    mix_render(m, out, 100);
    check(mix_active(m, id) == 1, "loops 0: still active after 100 more frames");
    mix_halt(m, id);
    check(mix_active(m, id) == 0, "mix_halt ends it");

    id2 = mix_start(m, &w, 100, 1);
    check(id2 >= 0 && id2 != id, "a restart gets a new id");
    check(mix_active(m, id) == 0, "the old id is stale after the voice was reused");
    mix_volume(m, id, 5);
    mix_loops(m, id, 9);
    mix_halt(m, id);                                   /* stale id: no crash, no effect */
    check(mix_active(m, id2) == 1, "the new id still active");
    mix_halt(m, id2);

    id = mix_start(m, &w, 127, 3);
    mix_render(m, out, 5);
    check(mix_active(m, id) == 1, "loops 3: pass 1 done, restarts");
    mix_render(m, out, 5);
    check(mix_active(m, id) == 1, "loops 3: pass 2 done, restarts");
    mix_render(m, out, 5);
    check(mix_active(m, id) == 1, "loops 3: pass 3 still playing (the run-out is read at the next frame)");
    mix_render(m, out, 5);
    check(mix_active(m, id) == 0, "loops 3: the third pass ends it");

    id = mix_start(m, &w, 127, 0);
    mix_render(m, out, 5);
    mix_loops(m, id, 1);                               /* set while playing: read at the next run-out */
    check(mix_active(m, id) == 0, "loops set to 1 while playing ends at run-out");
    mix_halt(m, id);                                   /* free the voice for the 8-voice test */

    for (i = 0; i < 8; i++)
        ids[i] = mix_start(m, &w, 127, 0);
    for (i = 0; i < 8; i++)
        check(ids[i] >= 0, "8 voices start");
    check(mix_start(m, &w, 127, 0) == -1, "the 9th start fails (all busy)");
    for (i = 0; i < 8; i++)
        mix_halt(m, ids[i]);

    check(mix_start(m, &w0, 127, 1) == -1, "an empty sample is not started");
    v = mix_play(m, &w0, 127, 64);                     /* mix_play takes it, but render clears the 0-frame voice */
    check(v >= 0, "mix_play on an empty sample takes a voice");
    mix_render(m, out, 1);
    check(mix_playing(m, v) == 0, "the 0-frame voice is cleared by render");
    id = mix_start(m, &w, 127, 1);
    check((id & 7) == v, "the cleared voice is reused");
    mix_halt(m, id);

    mix_music_volume(m, 64);                           /* no re-allocation, just the gain */
    v = mix_play(m, &w, 127, 64);
    check(v >= 0 && mix_playing(m, v) == 1, "mix_play still works");
    mix_stop(m, v);
    check(mix_playing(m, v) == 0, "mix_stop still works");

    mix_free(m);
    wav_free(&w);
    wav_free(&w0);
    free(wb);
    free(wb0);
}

/* ---------- xmiseq ---------- */
static void test_xmiseq(void)
{
    bb e;
    uint8_t *xmi;
    size_t xl;
    xmi_song song;
    xmi_seq *s;
    rec_t r;

    /* song: FOR 0 @0, NEXT 10 @1 (ignored), NEXT 127 @2 (jumps), note @3, EOT @20 */
    memset(&e, 0, sizeof e);
    ev_cc(&e, 0, 0, 116, 0);
    ev_cc(&e, 1, 0, 117, 10);
    ev_cc(&e, 1, 0, 117, 127);
    ev_note(&e, 1, 0, 60, 90, 5);
    ev_eot(&e, 17);
    xmi = make_form_xmid(e.b, e.n, 1, &xl);
    free(e.b);
    check(xmi_parse(xmi, xl, &song) == 0, "parse the NEXT-test song");
    memset(&r, 0, sizeof r);
    s = xmi_seq_new(&song, rec_midi, NULL, &r);
    xmi_seq_advance(s, 10);
    check(xmi_seq_loops(s) >= 1, "NEXT below 64 is ignored: the FOR 0 still jumps at NEXT 127");
    xmi_seq_free(s);
    xmi_free(&song);
    free(xmi);

    /* song: CC7 ch0 100 @0, CC7 ch0 60 @1, note ch2 @2, EOT @5  (volume scaling) */
    memset(&e, 0, sizeof e);
    ev_cc(&e, 0, 0, 7, 100);
    ev_cc(&e, 1, 0, 7, 60);
    ev_note(&e, 1, 2, 70, 80, 3);
    ev_eot(&e, 3);
    xmi = make_form_xmid(e.b, e.n, 1, &xl);
    free(e.b);
    check(xmi_parse(xmi, xl, &song) == 0, "parse the CC7 song");
    memset(&r, 0, sizeof r);
    s = xmi_seq_new(&song, rec_midi, NULL, &r);
    xmi_seq_advance(s, 0);
    check(rec_find_at(&r, 0, 0xB0, 7, 100), "CC7 100 goes out raw at the default volume");
    xmi_seq_set_volume(s, 64);
    check(rec_find_at(&r, r.n - 1, 0xB0, 7, (100 * 64) / 127), "volume change resends the scaled CC7 (shadow raw)");
    xmi_seq_advance(s, 1);
    check(rec_count(&r, 0xB2, 7) == 0, "a channel that never got a CC7 is not touched");
    {
        int i, scaled = -1;

        for (i = 0; i < r.n; i++)
            if (r.msg[i][0] == 0xB0 && r.msg[i][1] == 7 && r.msg[i][2] == (60 * 64) / 127)
                scaled = i;
        check(scaled >= 0, "the song's own CC7 60 goes out as 60*64/127");
    }
    xmi_seq_set_volume(s, 127);
    check(rec_find_at(&r, r.n - 1, 0xB0, 7, 60), "back to 127 resends the raw shadow value");
    /* rewind clears the shadows: a volume change afterwards resends nothing */
    xmi_seq_set_volume(s, 127);
    {
        int n_before = r.n;

        xmi_seq_rewind(s);
        xmi_seq_set_volume(s, 64);
        check(r.n == n_before, "rewind clears the shadows: no CC7 resend");
        xmi_seq_resume(s);                            /* rewind leaves the sequence paused */
        xmi_seq_advance(s, 0);
        check(rec_find_at(&r, r.n - 1, 0xB0, 7, (100 * 64) / 127), "after rewind the song's CC7 100 replays, scaled by the volume");
    }
    xmi_seq_free(s);
    xmi_free(&song);
    free(xmi);

    /* song: CC64 ch1 127 @0, CC64 ch0 10 @0, note ch1 64 @0 (dur 100), EOT @10  (pause) */
    memset(&e, 0, sizeof e);
    ev_cc(&e, 0, 1, 64, 127);
    ev_cc(&e, 0, 0, 64, 10);
    ev_note(&e, 0, 1, 64, 100, 100);
    ev_eot(&e, 10);
    xmi = make_form_xmid(e.b, e.n, 1, &xl);
    free(e.b);
    check(xmi_parse(xmi, xl, &song) == 0, "parse the pause song");
    memset(&r, 0, sizeof r);
    s = xmi_seq_new(&song, rec_midi, NULL, &r);
    xmi_seq_advance(s, 1);
    check(rec_count(&r, 0x91, 64) == 1, "the note is on");
    xmi_seq_pause(s);
    check(rec_count3(&r, 0x81, 64, 0) == 1, "pause sends note-off for the sounding note");
    check(rec_count3(&r, 0xB1, 64, 0) == 1 && r.msg[r.n - 1][0] == 0xB1 && r.msg[r.n - 1][1] == 64 && r.msg[r.n - 1][2] == 0,
          "pause sends CC64 0 where sustain was down");
    check(rec_count3(&r, 0xB0, 64, 0) == 0, "no CC64 0 where the shadow is below 64");
    {
        int n_at_pause = r.n;

        xmi_seq_advance(s, 10);                     /* past the EOT at tick 10: still nothing while paused */
        check(r.n == n_at_pause, "advance emits nothing while paused");
        check(xmi_seq_done(s) == 0, "no event is processed while paused (the EOT is not reached)");
        xmi_seq_resume(s);
        xmi_seq_advance(s, 5);
        check(rec_count(&r, 0x91, 64) == 1, "resume does not re-sound the note");
    }
    /* FF 2F with the default loop count 1: pause the notes and done */
    xmi_seq_advance(s, 10);
    check(xmi_seq_done(s) == 1, "EOT with loop count 1 ends the sequence");
    xmi_seq_free(s);
    xmi_free(&song);
    free(xmi);

    /* EOT loop counts: 0 = forever, n = n passes, and the count-0 quirk */
    memset(&e, 0, sizeof e);
    ev_note(&e, 0, 0, 60, 90, 2);
    ev_eot(&e, 10);
    xmi = make_form_xmid(e.b, e.n, 1, &xl);
    free(e.b);
    xmi_parse(xmi, xl, &song);
    memset(&r, 0, sizeof r);
    s = xmi_seq_new(&song, rec_midi, NULL, &r);
    xmi_seq_set_loops(s, 0);
    xmi_seq_advance(s, 40);
    check(!xmi_seq_done(s) && xmi_seq_loops(s) >= 1, "loop count 0 rewinds at EOT forever");
    xmi_seq_free(s);
    s = xmi_seq_new(&song, rec_midi, NULL, &r);
    xmi_seq_set_loops(s, 3);
    xmi_seq_advance(s, 100);
    check(xmi_seq_done(s) == 1 && xmi_seq_loops(s) == 2, "loop count 3: two rewinds, then done");
    xmi_seq_free(s);
    /* the quirk: ended with count 0 -> a rewind without a new count loops forever */
    s = xmi_seq_new(&song, rec_midi, NULL, &r);
    xmi_seq_advance(s, 20);                            /* default 1: ends, count now 0 */
    check(xmi_seq_done(s) == 1, "default count 1 ends at EOT");
    xmi_seq_rewind(s);
    check(!xmi_seq_done(s), "rewind clears done");
    xmi_seq_resume(s);
    xmi_seq_advance(s, 100);
    check(!xmi_seq_done(s) && xmi_seq_loops(s) >= 1, "the count-0 quirk: rewind without a new count loops forever");
    xmi_seq_free(s);
    xmi_free(&song);
    free(xmi);
}

/* ---------- musplay ---------- */
static void test_musplay(void)
{
    bb e;
    uint8_t *xmi2, *lone, *noevnt;
    size_t l2, ll, ln;
    uint8_t *evs[2];
    size_t els[2];
    rec_t r;
    mus_backend b;
    musplay *p, *q;
    static int16_t out[8820];
    xmi_song song;

    /* two sequences in a CAT XMID behind a FORM XDIR; seq 0: note@0 (dur 5), CC11@3, EOT@5 */
    memset(&e, 0, sizeof e);
    ev_note(&e, 0, 0, 60, 90, 5);
    ev_cc(&e, 3, 0, 11, 42);
    ev_eot(&e, 2);
    evs[0] = e.b;
    els[0] = e.n;
    memset(&e, 0, sizeof e);
    ev_note(&e, 0, 1, 62, 90, 5);
    ev_eot(&e, 5);
    evs[1] = e.b;
    els[1] = e.n;
    xmi2 = make_xmi((const uint8_t *const *)evs, els, 2, 1, &l2);
    free(evs[0]);
    free(evs[1]);
    {
        bb e2;

        memset(&e2, 0, sizeof e2);
        ev_note(&e2, 0, 0, 60, 90, 5);
        ev_eot(&e2, 5);
        lone = make_form_xmid(e2.b, e2.n, 1, &ll);
        free(e2.b);
        noevnt = make_form_xmid(NULL, 0, 0, &ln);
    }

    check(mus_probe(xmi2, l2, 0, &song) == 0 && song.count > 0, "probe n=0 of the CAT XMID");
    xmi_free(&song);
    check(mus_probe(xmi2, l2, 1, &song) == 0, "probe n=1");
    xmi_free(&song);
    check(mus_probe(xmi2, l2, 2, &song) != 0, "probe n=2 fails");
    check(mus_probe(lone, ll, 0, &song) == 0, "a lone FORM XMID: n=0");
    xmi_free(&song);
    check(mus_probe(lone, ll, 1, &song) != 0, "a lone FORM XMID: n=1 fails");
    check(mus_probe(noevnt, ln, 0, &song) != 0, "no EVNT fails");

    memset(&r, 0, sizeof r);
    b.midi = rec_midi;
    b.render = rec_render;
    b.reset = rec_reset;
    b.u = &r;
    p = mus_new(&b, 44100);
    check(mus_status(p) == MUS_FREE, "a new player is free");
    check(r.n == 16 * 15, "the channel set-up goes out once at mus_new (16 ch x 15 messages)");
    check(r.resets == 0, "no reset at mus_new");

    check(mus_load(p, xmi2, l2, 0) == 1, "load sequence 0");
    check(mus_status(p) == MUS_DONE, "loaded: done, at the start");
    check(r.resets == 1, "reset between songs");
    {
        int n0 = r.n;

        mus_start(p);
        check(mus_status(p) == MUS_PLAYING, "start: playing");
        check(r.n == n0 + 16 * 15 + 1, "start resends the set-up, then the song's tick-0 note-on");
        mus_render(p, out, 919);                      /* two full ticks (367+368) + a 184-frame piece of tick 2 */
        check(r.renders == 3 && r.render_frames == 919, "mus_render renders per 1/120 s interval");
        check(rec_count(&r, 0x90, 60) == 1, "the note-on reached the backend");
        check(rec_count3(&r, 0xB0, 11, 42) == 0, "a partial 1/120 s piece does not advance the sequencer");
        check(mus_status(p) == MUS_PLAYING, "still playing before the EOT at tick 5");
        mus_render(p, out, 184);                      /* tick 2 complete: three full ticks now */
        check(rec_count3(&r, 0xB0, 11, 42) == 1, "the tick-3 event goes out when its 1/120 s passed");
        mus_render(p, out, 4410);                     /* past the EOT at tick 5 */
        check(mus_status(p) == MUS_DONE, "the song ended through its loop count: status done");
        mus_pause(p);
        check(mus_status(p) == MUS_DONE, "pause after done does nothing");
        mus_end(p);
        check(mus_status(p) == MUS_DONE, "end: done");
    }
    {
        mus_load(p, xmi2, l2, 1);
        mus_start(p);
        check(mus_status(p) == MUS_PLAYING, "second song plays");
        mus_pause(p);
        check(mus_status(p) == MUS_PAUSED, "pause: paused");
        mus_resume(p);
        check(mus_status(p) == MUS_PLAYING, "resume: playing");
        mus_volume(p, 64);
        mus_loops(p, 0);
        mus_render(p, out, 441);
        check(mus_status(p) == MUS_PLAYING, "loop count 0 keeps it playing");
        mus_end(p);
        mus_free(p);
    }

    q = mus_new(NULL, 44100);                          /* no backend: silent, same state machine */
    check(mus_load(q, lone, ll, 0) == 1, "null backend: load ok");
    mus_start(q);
    check(mus_status(q) == MUS_PLAYING, "null backend: playing");
    memset(out, 0x55, sizeof out);
    mus_render(q, out, 441);
    check(out[0] == 0 && out[441 * 2 - 1] == 0, "null backend renders silence");
    check(mus_status(q) == MUS_DONE || mus_status(q) == MUS_PLAYING, "status readable");
    mus_free(q);

    free(xmi2);
    free(lone);
    free(noevnt);
}

/* ---------- ail ---------- */
static void test_ail(void)
{
    int32_t h[16];
    int32_t sh;
    int i;
    uint8_t *wavbuf, *voc, *junk, *badwav;
    size_t wl, vocl, jl, bl, xl;
    uint8_t *xmi2;
    uint8_t *evs[2];
    size_t els[2];
    bb e;
    static int16_t out[88200];

    memset(&e, 0, sizeof e);
    ev_note(&e, 0, 0, 60, 90, 5);
    ev_eot(&e, 5);
    evs[0] = e.b;
    els[0] = e.n;
    memset(&e, 0, sizeof e);
    ev_note(&e, 0, 1, 62, 90, 5);
    ev_eot(&e, 5);
    evs[1] = e.b;
    els[1] = e.n;
    xmi2 = make_xmi((const uint8_t *const *)evs, els, 2, 1, &xl);
    free(evs[0]);
    free(evs[1]);

    wavbuf = make_wav(22050, 1, 16, 2205, 7, &wl);      /* 0.1 s at 22050 Hz */
    voc = malloc(26);
    memcpy(voc, "Creative Voice File\x1A\x00\x00", 20);
    vocl = 26;
    junk = malloc(16);
    memcpy(junk, "RIFF\x08\x00\x00\x00XXXX", 12);
    jl = 12;
    badwav = make_wav(22050, 1, 16, 100, 1, &bl);
    badwav[20] = 0xFF;                                 /* break the fmt chunk: wav_decode refuses */
    bl = 30;                                           /* and cut it short */

    ail_init(test_lock, NULL);
    check(ail_set_preference(7, 1) == 0, "preference 7 defaults to 0");
    check(ail_set_preference(7, 1) == 1, "preference 7 returns the old value");
    check(ail_set_preference(1, 0) == 1, "preference 1 defaults to 1");

    for (i = 0; i < 16; i++)
        h[i] = ail_allocate_sample();
    for (i = 0; i < 16; i++)
        check(h[i] == i + 1, "sample handles 1..16 in allocation order");
    check(ail_allocate_sample() == 0, "the 17th sample handle is 0");
    check(strcmp(ail_last_error(), "Out of sample handles\n") == 0, "the handle error text");

    ail_init_sample(h[0]);
    check(ail_sample_status(h[0]) == AIL_DONE, "init_sample: status done");
    ail_start_sample(h[0]);
    check(ail_sample_status(h[0]) == AIL_DONE, "start with no file does nothing");

    check(ail_set_sample_file(h[0], wavbuf, wl) == 1, "a WAV file is accepted");
    check(ail_set_sample_file(h[0], voc, vocl) == 0, "a VOC is refused");
    check(strcmp(ail_last_error(), "Unrecognized digital audio file type\n") == 0, "the VOC error text");
    check(ail_set_sample_file(h[0], junk, jl) == 0, "a non-WAV is refused");
    check(ail_set_sample_file(h[0], badwav, bl) == 0, "a broken WAV is refused");
    check(ail_sample_status(h[0]) == AIL_DONE, "a refused file does not change the status");

    /* not live (the default): loop 1 is done at once, nothing mixed; loop 0 stays playing until ended */
    ail_set_sample_loop_count(h[0], 1);
    ail_start_sample(h[0]);
    check(ail_sample_status(h[0]) == AIL_DONE, "not live, loop 1: done at once");
    check(ail_active_sample_count() == 0, "not live: nothing active");
    ail_set_sample_loop_count(h[0], 0);
    ail_start_sample(h[0]);
    check(ail_sample_status(h[0]) == AIL_PLAYING, "not live, loop 0: stays playing");
    check(ail_active_sample_count() == 1, "not live: one active");
    memset(out, 0x55, sizeof out);
    ail_render(out, 256);
    check(out[0] == 0 && out[511] == 0, "not live: nothing is mixed");
    check(ail_sample_status(h[0]) == AIL_PLAYING, "not live: still playing after render");
    ail_end_sample(h[0]);
    check(ail_sample_status(h[0]) == AIL_DONE, "end: done");
    ail_end_sample(h[0]);
    check(ail_sample_status(h[0]) == AIL_DONE, "end on a done handle stays done");

    /* live */
    ail_set_live(1);
    ail_set_sample_loop_count(h[0], 1);
    ail_set_sample_volume(h[0], 127);
    ail_start_sample(h[0]);
    check(ail_sample_status(h[0]) == AIL_PLAYING, "live: start plays");
    check(ail_active_sample_count() == 1, "live: one active");
    ail_render(out, 4410);                             /* 0.1 s at 44100 = the whole 0.1 s sample */
    check(ail_sample_status(h[0]) == AIL_DONE, "live, loop 1: done once the data ran out");
    check(ail_active_sample_count() == 0, "live: none active after the end");
    ail_set_sample_loop_count(h[0], 0);
    ail_start_sample(h[0]);
    check(ail_sample_status(h[0]) == AIL_PLAYING, "live, loop 0: playing");
    ail_render(out, 4410);
    check(ail_sample_status(h[0]) == AIL_PLAYING, "live, loop 0: still playing after the data");
    ail_start_sample(h[0]);                            /* start restarts, even while playing */
    check(ail_sample_status(h[0]) == AIL_PLAYING, "start while playing restarts");
    ail_end_sample(h[0]);
    check(ail_sample_status(h[0]) == AIL_DONE, "end after restart: done");
    check(ail_sample_status(99) == 0, "an unknown handle has status 0");
    check(ail_sample_status(0) == 0, "handle 0 is none");

    /* the sequence */
    sh = ail_allocate_sequence();
    check(sh == 1, "sequence handle 1");
    check(ail_sequence_status(sh) == AIL_DONE, "a new sequence handle is done");
    check(ail_init_sequence(sh, xmi2, xl, 2) == 0, "init_sequence n=2 fails");
    check(strcmp(ail_last_error(), "Invalid XMIDI sequence\n") == 0, "the sequence error text");
    check(ail_init_sequence(sh, xmi2, xl, 0) == 1, "init_sequence n=0 ok");
    ail_start_sequence(sh);
    check(ail_sequence_status(sh) == AIL_PLAYING, "sequence plays");
    ail_stop_sequence(sh);
    check(ail_sequence_status(sh) == AIL_PAUSED, "stop: paused");
    ail_resume_sequence(sh);
    check(ail_sequence_status(sh) == AIL_PLAYING, "resume: playing");
    ail_set_sequence_volume(sh, 112, 0);
    ail_set_sequence_loop_count(sh, 0);
    ail_render(out, 4410);
    check(ail_sequence_status(sh) == AIL_PLAYING, "loop count 0: still playing after 0.1 s");
    ail_end_sequence(sh);
    check(ail_sequence_status(sh) == AIL_DONE, "end: done");
    ail_set_sequence_loop_count(sh, 1);
    ail_start_sequence(sh);
    check(ail_sequence_status(sh) == AIL_PLAYING, "restart after end");
    ail_render(out, 44100);                            /* 1 s: past the song's EOT at tick 5 */
    check(ail_sequence_status(sh) == AIL_DONE, "the song ended through its loop count: status done");
    check(ail_sequence_status(9) == 0, "an unknown sequence handle: 0");
    ail_start_sequence(sh);
    check(ail_sequence_status(sh) == AIL_PLAYING, "playing again");
    check(ail_init_sequence(sh, xmi2, xl, 2) == 0, "a failed init_sequence still returns 0");
    check(ail_sequence_status(sh) == AIL_DONE, "init_sequence sets status done first, even when it fails");

    check(g_lock_balance == 0, "every lock(1) has its lock(0)");
    ail_shutdown();
    free(wavbuf);
    free(voc);
    free(junk);
    free(badwav);
    free(xmi2);
}

int main(void)
{
    test_mixer();
    test_xmiseq();
    test_musplay();
    test_ail();
    if (fails == 0)
        printf("ail_test: all checks pass\n");
    else
        printf("ail_test: %d FAILURES\n", fails);
    return fails ? 1 : 0;
}
