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

static void mus_fill_cb(void *u, int16_t *out, int frames)
{
    int i;

    (void)u;
    for (i = 0; i < frames * 2; i++)
        out[i] = 1000;
}

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
    check(mix_active(m, id) == 0, "loops 3: the third pass ends it (15 frames, no silent frame)");

    id = mix_start(m, &w, 127, 0);
    mix_render(m, out, 5);
    mix_loops(m, id, 1);                               /* set while playing: read at the next run-out */
    check(mix_active(m, id) == 0, "loops set to 1 while playing ends at run-out");
    mix_halt(m, id);                                   /* free the voice for the 8-voice test */

    /* a restart rewinds by the length and mixes the run-out frame: the 5-frame sample repeats every 5
       frames, not every 6 (no silent frame per pass) */
    id = mix_start(m, &w, 127, 0);
    {
        int k, ok = 1;

        for (k = 0; k < 12; k++) {
            mix_render(m, out, 1);
            if (out[0] != 100 + (k % 5) || out[1] != out[0])
                ok = 0;
        }
        check(ok, "loops 0: the 5-frame sample repeats every 5 frames (the run-out frame is mixed)");
    }
    mix_halt(m, id);

    /* a count-1 play that ran out is stale everywhere: mix_loops/mix_volume cannot revive it */
    id = mix_start(m, &w, 127, 1);
    mix_render(m, out, 5);
    check(mix_active(m, id) == 0, "loops 1: ran out");
    mix_loops(m, id, 0);
    mix_volume(m, id, 99);
    check(mix_active(m, id) == 0, "a run-out count-1 play is stale: mix_loops/mix_volume do not revive it");
    mix_halt(m, id);

    /* mix_halt does NOT apply the run-out rule: it stops a voice that ran out at the last frame of a render
       (the voice is still active until the next frame), so the voice is reused by the next start */
    id = mix_start(m, &w, 127, 1);
    mix_render(m, out, 5);                        /* the last source frame is the last frame of the call */
    check(mix_active(m, id) == 0, "loops 1: ran out at the last frame of the render");
    mix_halt(m, id);
    v = mix_start(m, &w, 127, 1);
    check(v >= 0 && (v & 7) == (id & 7) && v != id,
          "mix_halt stopped the run-out voice: the voice is reused with a new generation");
    mix_halt(m, v);

    /* mix_play bumps the generation: an old mix_start id on the reused voice goes stale */
    id = mix_start(m, &w, 127, 0);
    mix_halt(m, id);
    v = mix_play(m, &w, 127, 64);
    check(v >= 0 && mix_active(m, id) == 0, "mix_play reuses the voice and stales the old mix_start id");
    mix_stop(m, v);

    /* mix_volume on a playing voice, observed in the output */
    id = mix_start(m, &w, 127, 0);
    mix_render(m, out, 1);
    check(out[0] == 100, "volume 127: the raw sample value goes out");
    mix_volume(m, id, 0);
    mix_render(m, out, 1);
    check(out[0] == 0, "volume 0 silences from the next rendered frame");
    mix_volume(m, id, 64);
    mix_render(m, out, 1);
    check(out[0] == (102 * 64) / 127, "volume 64: scaled from the next rendered frame");
    mix_halt(m, id);

    /* mix_music_volume with a music input, without re-allocating */
    mix_set_music(m, mus_fill_cb, NULL, 127);
    mix_render(m, out, 1);
    check(out[0] == 1000, "the music input at gain 127");
    mix_music_volume(m, 64);
    mix_render(m, out, 1);
    check(out[0] == (1000 * 64) / 127, "mix_music_volume changes the music gain");
    mix_set_music(m, NULL, NULL, 0);

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
    check(rec_count(&r, 0x90, 60) == 0, "NEXT below 64 is not a break either: the note behind the jump never sounds");
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
    xmi_seq_advance(s, 1);                            /* the delta-0 event is at tick 1 (a 0 delta costs one interval) */
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
        xmi_seq_advance(s, 1);
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
    xmi_seq_advance(s, 3);                            /* the three delta-0 events are at ticks 1, 2, 3 */
    check(rec_count(&r, 0x91, 64) == 1, "the note is on");
    xmi_seq_pause(s);
    check(rec_count3(&r, 0x81, 64, 0) == 1, "pause sends note-off for the sounding note");
    check(rec_count3(&r, 0xB1, 64, 0) == 1 && r.msg[r.n - 1][0] == 0xB1 && r.msg[r.n - 1][1] == 64 && r.msg[r.n - 1][2] == 0,
          "pause sends CC64 0 where sustain was down");
    check(rec_count3(&r, 0xB0, 64, 0) == 0, "no CC64 0 where the shadow is below 64");
    {
        int n_at_pause = r.n;

        xmi_seq_advance(s, 10);                     /* past the EOT at tick 13: still nothing while paused */
        check(r.n == n_at_pause, "advance emits nothing while paused");
        check(xmi_seq_done(s) == 0, "no event is processed while paused (the EOT is not reached)");
        xmi_seq_resume(s);
        xmi_seq_advance(s, 5);
        check(rec_count(&r, 0x91, 64) == 1, "resume does not re-sound the note");
        /* the resume replay (0x99C20): every stored controller goes out again, in channel order; the stop's
           CC64 0 kept the shadow, so the pedal is put down again */
        check(rec_count3(&r, 0xB0, 64, 10) == 2, "the resume replays every stored controller (CC64 10 on ch0 too)");
        check(rec_count3(&r, 0xB1, 64, 127) == 2, "the resume replay resends the stored CC64 127 (pedal down again)");
        check(rec_count3(&r, 0xB1, 64, 0) == 1, "the stop's CC64 0 went out once (the shadow kept its value)");
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

    /* stop releases the pedal: note-off for the sounding note, CC64 0 where the sustain shadow is down */
    memset(&e, 0, sizeof e);
    ev_cc(&e, 0, 0, 64, 127);
    ev_note(&e, 0, 0, 60, 100, 100);
    ev_eot(&e, 10);
    xmi = make_form_xmid(e.b, e.n, 1, &xl);
    free(e.b);
    check(xmi_parse(xmi, xl, &song) == 0, "parse the stop song");
    memset(&r, 0, sizeof r);
    s = xmi_seq_new(&song, rec_midi, NULL, &r);
    xmi_seq_advance(s, 2);                            /* CC64 at tick 1, the note at tick 2 */
    check(rec_count(&r, 0x90, 60) == 1, "the note sounds");
    xmi_seq_stop(s);
    check(rec_count3(&r, 0x80, 60, 0) == 1, "stop sends note-off for the sounding note");
    check(rec_count3(&r, 0xB0, 64, 0) == 1, "stop sends CC64 0 where sustain was down (the pedal is released)");
    xmi_seq_free(s);
    /* ending at the EOT (count 1) stops the notes and releases the pedal too */
    memset(&r, 0, sizeof r);
    s = xmi_seq_new(&song, rec_midi, NULL, &r);
    xmi_seq_advance(s, 12);                           /* the EOT is at tick 12 */
    check(xmi_seq_done(s) == 1, "EOT with count 1: done");
    check(rec_count3(&r, 0x80, 60, 0) == 1, "ending at the EOT stops the sounding note");
    check(rec_count3(&r, 0xB0, 64, 0) == 1, "ending at the EOT sends CC64 0 where sustain was down");
    xmi_seq_free(s);
    xmi_free(&song);
    free(xmi);

    /* the FF 2F rewind restarts at the NEXT interval: shift = t + 1, the lead-in delay counts again */
    memset(&e, 0, sizeof e);
    ev_cc(&e, 3, 0, 7, 100);                       /* first event at tick 3 */
    ev_eot(&e, 10);                                /* EOT at tick 13 */
    xmi = make_form_xmid(e.b, e.n, 1, &xl);
    free(e.b);
    check(xmi_parse(xmi, xl, &song) == 0, "parse the lead-in song");
    memset(&r, 0, sizeof r);
    s = xmi_seq_new(&song, rec_midi, NULL, &r);
    xmi_seq_set_loops(s, 0);
    xmi_seq_advance(s, 14);
    check(xmi_seq_loops(s) == 1, "the EOT rewound once");
    check(rec_count3(&r, 0xB0, 7, 100) == 1, "shift = t+1: the lead-in event replays at 14+3=17, not yet at 14");
    xmi_seq_advance(s, 2);                        /* now 16 */
    check(rec_count3(&r, 0xB0, 7, 100) == 1, "still not at 16 (the host restarts at the next interval)");
    xmi_seq_advance(s, 1);                        /* now 17 */
    check(rec_count3(&r, 0xB0, 7, 100) == 2, "the lead-in delay counts again after the rewind");
    xmi_seq_free(s);
    xmi_free(&song);
    free(xmi);

    /* rewind while a note sounds and inside a FOR loop: note-off, FOR slots and shadows cleared */
    memset(&e, 0, sizeof e);
    ev_cc(&e, 0, 0, 116, 0);
    ev_cc(&e, 0, 0, 7, 90);
    ev_note(&e, 0, 0, 60, 100, 100);
    ev_eot(&e, 100);
    xmi = make_form_xmid(e.b, e.n, 1, &xl);
    free(e.b);
    check(xmi_parse(xmi, xl, &song) == 0, "parse the rewind song");
    memset(&r, 0, sizeof r);
    s = xmi_seq_new(&song, rec_midi, NULL, &r);
    xmi_seq_advance(s, 3);                            /* FOR at 1, CC7 at 2, the note at 3 */
    check(rec_count(&r, 0x90, 60) == 1, "the note sounds inside the FOR loop");
    xmi_seq_rewind(s);
    check(rec_count3(&r, 0x80, 60, 0) == 1, "rewind sends note-off for the sounding note");
    xmi_seq_resume(s);
    xmi_seq_advance(s, 3);
    check(rec_count(&r, 0x90, 60) == 2, "after rewind the FOR and the note replay");
    check(rec_count3(&r, 0xB0, 7, 90) == 2, "after rewind the song resends its CC7 (shadows were cleared)");
    xmi_seq_free(s);
    xmi_free(&song);
    free(xmi);

    /* the FF 2F count-0 rewind keeps the controller shadows */
    memset(&e, 0, sizeof e);
    ev_cc(&e, 0, 0, 7, 100);
    ev_eot(&e, 10);
    xmi = make_form_xmid(e.b, e.n, 1, &xl);
    free(e.b);
    check(xmi_parse(xmi, xl, &song) == 0, "parse the shadow song");
    memset(&r, 0, sizeof r);
    s = xmi_seq_new(&song, rec_midi, NULL, &r);
    xmi_seq_set_loops(s, 0);
    xmi_seq_advance(s, 11);                           /* CC7 at 1, the EOT at 11 */
    check(xmi_seq_loops(s) == 1, "count 0 rewound at the EOT");
    check(rec_count3(&r, 0xB0, 7, 100) == 1, "the rewind is shift t+1: the CC7 replays at 13, not yet at 11");
    xmi_seq_advance(s, 2);                            /* now 13 */
    check(rec_count3(&r, 0xB0, 7, 100) == 2, "the CC7 replayed at the rewind");
    xmi_seq_set_volume(s, 64);
    check(rec_find_at(&r, r.n - 1, 0xB0, 7, (100 * 64) / 127), "the count-0 rewind kept the shadow: volume change resends");
    xmi_seq_free(s);
    xmi_free(&song);
    free(xmi);

    /* no time passes while paused: the remaining delay is kept for after the resume */
    memset(&e, 0, sizeof e);
    ev_note(&e, 0, 0, 60, 90, 100);
    ev_cc(&e, 8, 0, 11, 42);                         /* marker event at tick 8 */
    ev_eot(&e, 12);
    xmi = make_form_xmid(e.b, e.n, 1, &xl);
    free(e.b);
    check(xmi_parse(xmi, xl, &song) == 0, "parse the delay song");
    memset(&r, 0, sizeof r);
    s = xmi_seq_new(&song, rec_midi, NULL, &r);
    xmi_seq_advance(s, 5);
    xmi_seq_pause(s);                                /* sends the note-off, forgets the note */
    check(rec_count3(&r, 0x80, 60, 0) == 1, "pause sends the note-off");
    {
        int n_at_pause = r.n;

        xmi_seq_advance(s, 5);                       /* paused: nothing, time does not advance */
        check(r.n == n_at_pause, "nothing is emitted while paused");
        check(rec_count3(&r, 0xB0, 11, 42) == 0, "the tick-9 event has not fired while paused");
        xmi_seq_resume(s);
        xmi_seq_advance(s, 2);                       /* now 7 */
        check(rec_count3(&r, 0xB0, 11, 42) == 0, "the remaining delay is kept (now 7 of 9)");
        xmi_seq_advance(s, 2);                       /* now 9 */
        check(rec_count3(&r, 0xB0, 11, 42) == 1, "the event fires at tick 9, after the resume");
    }
    xmi_seq_free(s);
    xmi_free(&song);
    free(xmi);

    /* the stop's status gate (0x9B632): AIL_stop_sequence acts only while playing (status 4). The pedal
       shadows are kept (22d), so an ungated stop would send CC64 0 / CC112 0 again on a paused or ended
       sequence - it must not */
    memset(&e, 0, sizeof e);
    ev_cc(&e, 0, 0, 64, 127);
    ev_cc(&e, 0, 0, 112, 100);
    ev_note(&e, 0, 0, 60, 90, 100);
    ev_eot(&e, 10);
    xmi = make_form_xmid(e.b, e.n, 1, &xl);
    free(e.b);
    check(xmi_parse(xmi, xl, &song) == 0, "parse the gate song");
    memset(&r, 0, sizeof r);
    s = xmi_seq_new(&song, rec_midi, NULL, &r);
    xmi_seq_advance(s, 2);                            /* CC64/CC112 at tick 1, the note at tick 2 */
    xmi_seq_pause(s);
    check(rec_count3(&r, 0xB0, 64, 0) == 1 && rec_count3(&r, 0xB0, 112, 0) == 1, "pause releases the pedal once");
    {
        int n1 = r.n;

        xmi_seq_stop(s);                              /* end on a PAUSED handle: status 8, not 4 */
        check(r.n == n1, "stop on a paused handle emits nothing (the 0x9B632 gate)");
        check(rec_count3(&r, 0xB0, 64, 0) == 1, "no second CC64 0");
    }
    xmi_seq_free(s);
    /* play to the natural end (EOT, count 1: the end emission fires there), then rewind: nothing more */
    memset(&r, 0, sizeof r);
    s = xmi_seq_new(&song, rec_midi, NULL, &r);
    xmi_seq_advance(s, 13);                           /* the EOT is at tick 13 (delta-0 events at 1, 2, 3) */
    check(xmi_seq_done(s) == 1, "the song ended at its EOT");
    check(rec_count3(&r, 0xB0, 64, 0) == 1 && rec_count3(&r, 0xB0, 112, 0) == 1, "the end released the pedal once");
    {
        int n1 = r.n;

        xmi_seq_rewind(s);                            /* after the end: status 2, not 4 */
        check(r.n == n1, "rewind after the natural end emits nothing (the 0x9B632 gate)");
        xmi_seq_resume(s);
        xmi_seq_advance(s, 2);
        check(rec_count(&r, 0x90, 60) == 1, "after the rewind the song replays");
    }
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
    check(mus_probe(xmi2, l2, 1, &song) == 0 && song.ev[0].d1 == 62, "probe n=1 returns the second sequence (key 62)");
    xmi_free(&song);
    {
        bb e1;
        uint8_t *ev1[1];
        size_t el1[1], l1;
        uint8_t *x1;

        memset(&e1, 0, sizeof e1);
        ev_note(&e1, 0, 0, 60, 90, 5);
        ev_eot(&e1, 5);
        ev1[0] = e1.b;
        el1[0] = e1.n;
        x1 = make_xmi((const uint8_t *const *)ev1, el1, 1, 1, &l1);   /* CAT XMID with one FORM XMID */
        check(mus_probe(x1, l1, 0, &song) == 0, "a CAT XMID with one FORM XMID: n=0");
        xmi_free(&song);
        check(mus_probe(x1, l1, 1, &song) != 0, "a CAT XMID with one FORM XMID: n=1 fails");
        {
            bb top = { 0, 0, 0 };

            bb_plain(&top, "JUNK", (const uint8_t *)"xx", 2);
            bb_bytes(&top, x1, l1);
            check(mus_probe(top.b, top.n, 0, &song) != 0, "a non-FORM/CAT top chunk fails");
            free(top.b);
        }
        {
            bb top = { 0, 0, 0 };

            bb_bytes(&top, x1, l1);
            bb_plain(&top, "JUNK", (const uint8_t *)"xx", 2);
            check(mus_probe(top.b, top.n, 0, &song) == 0, "a junk chunk after the hit is never walked: the hit stands");
            xmi_free(&song);
            free(top.b);
        }
        {
            bb body = { 0, 0, 0 }, top = { 0, 0, 0 };

            bb_plain(&body, "TIMB", TIMB_BODY, sizeof TIMB_BODY);
            bb_plain(&body, "EVNT", e1.b, e1.n);
            bb_chunk(&top, "form", "xmid", body.b, body.n);            /* lower-case ids */
            check(mus_probe(top.b, top.n, 0, &song) != 0, "lower-case ids do not match (case-sensitive)");
            free(body.b);
            free(top.b);
        }
        {
            /* an odd-sized, unpadded FORM XDIR before the CAT: the walk adds no pad byte after an odd size */
            bb top = { 0, 0, 0 };

            bb_bytes(&top, "FORM", 4); bb_u32be(&top, 13); bb_bytes(&top, "XDIR", 4);
            bb_bytes(&top, "INFO", 4); bb_u32be(&top, 1); bb_u8(&top, 'x');   /* XDIR body: 13 bytes, odd, no pad */
            bb_bytes(&top, x1 + 22, l1 - 22);                                 /* x1's CAT XMID chunk (x1: 22-byte XDIR) */
            check(mus_probe(top.b, top.n, 0, &song) == 0, "no pad byte after an odd size: the CAT is still found");
            xmi_free(&song);
            free(top.b);
        }
        free(x1);
        free(e1.b);
    }

    memset(&r, 0, sizeof r);
    memset(&b, 0, sizeof b);                       /* the fields not set here (timbres) are NULL */
    b.midi = rec_midi;
    b.render = rec_render;
    b.reset = rec_reset;
    b.u = &r;
    p = mus_new(&b, 44100);
    check(mus_status(p) == MUS_FREE, "a new player is free");
    check(r.n == 16 * 15, "the channel set-up goes out once at mus_new (16 ch x 15 messages)");
    check(r.resets == 0, "no reset at mus_new");
    {
        static const int ccs[12][2] = {
            { 112, 0 }, { 1, 0 }, { 7, 127 }, { 10, 64 }, { 11, 127 }, { 64, 0 },
            { 91, 40 }, { 93, 0 }, { 100, 0 }, { 101, 0 }, { 38, 0 }, { 6, 2 }
        };
        int ch, i, ok = 1;

        for (ch = 0; ch < 16 && ok; ch++) {
            int base = ch * 15;

            if (!rec_find_at(&r, base + 0, 0xB0 | ch, 114, 0)) ok = 0;         /* CC114 0 */
            if (!rec_find_at(&r, base + 1, 0xC0 | ch, 0, 0)) ok = 0;            /* program 0 */
            if (!rec_find_at(&r, base + 2, 0xE0 | ch, 0x00, 0x40)) ok = 0;      /* pitch bend 0x2000 */
            for (i = 0; i < 12 && ok; i++)
                if (!rec_find_at(&r, base + 3 + i, 0xB0 | ch, ccs[i][0], ccs[i][1]))
                    ok = 0;
        }
        check(ok, "the set-up messages match the spec list in value and order (16 ch x 15)");
    }

    check(mus_load(p, xmi2, l2, 0) == 1, "load sequence 0");
    check(mus_status(p) == MUS_DONE, "loaded: done, at the start");
    check(r.resets == 1, "reset between songs");
    {
        int n0 = r.n;

        mus_start(p);
        check(mus_status(p) == MUS_PLAYING, "start: playing");
        check(r.n == n0 + 16 * 15, "start resends the set-up (the delta-0 events fire at the first 1/120 s, as the host's countdown does)");
        mus_render(p, out, 919);                      /* two full ticks (367+368) + a 184-frame piece of tick 2 */
        check(r.renders == 3 && r.render_frames == 919, "mus_render renders per 1/120 s interval");
        check(rec_count(&r, 0x90, 60) == 1, "the note-on (tick 1) reached the backend");
        check(rec_count3(&r, 0xB0, 11, 42) == 0, "a partial 1/120 s piece does not advance the sequencer");
        check(mus_status(p) == MUS_PLAYING, "still playing before the EOT at tick 6");
        mus_render(p, out, 735);                      /* tick 2 and tick 3 complete: four full ticks now */
        check(rec_count3(&r, 0xB0, 11, 42) == 1, "the tick-4 event goes out when its 1/120 s passed");
        mus_render(p, out, 4410);                     /* past the EOT at tick 6 */
        check(mus_status(p) == MUS_DONE, "the song ended through its loop count: status done");
        mus_pause(p);
        check(mus_status(p) == MUS_DONE, "pause after done does nothing");
        mus_end(p);
        check(mus_status(p) == MUS_DONE, "end: done");
    }
    {
        /* a song whose first event has NO delay byte (tick 0, raw EVNT): mus_start's advance(0) must send it
           at once, right after the 16*15 set-up, before any render */
        static const uint8_t raw[] = { 0x90, 0x3C, 0x5A, 0x05, 0x03, 0xFF, 0x2F, 0x00 };
        uint8_t *xraw;
        size_t xrl;
        int n0;

        xraw = make_form_xmid(raw, sizeof raw, 1, &xrl);
        check(mus_load(p, xraw, xrl, 0) == 1, "load the raw tick-0 song");
        n0 = r.n;
        mus_start(p);
        check(r.n == n0 + 16 * 15 + 1, "start sends the set-up AND the tick-0 note-on (the advance(0) flush)");
        check(rec_find_at(&r, n0 + 16 * 15, 0x90, 0x3C, 0x5A), "the tick-0 note-on is the first song message");
        mus_end(p);
        free(xraw);
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
        mus_render(p, out, 4410);                     /* 12 ticks: past the EOT at tick 5 */
        check(mus_status(p) == MUS_PLAYING, "loop count 0 really reaches its EOT and keeps playing");
        check(rec_count(&r, 0x91, 62) >= 2, "the song's note replayed after the EOT rewind");
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
    check(mus_status(q) == MUS_PLAYING, "null backend: one named status (playing after one tick)");
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
    uint8_t *wb5, *wb11, *wb8, *wb0, *wb100, *wb1000;
    size_t wl, vocl, jl, bl, xl, wl5, wl11, wl8, wl0, wl100, wl1000;
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
    check(ail_sample_status(3) == AIL_FREE, "an unallocated sample handle inside 1..16 reports free");
    check(ail_sequence_status(2) == AIL_FREE, "an unallocated sequence handle inside 1..8 reports free");
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

    /* WAVs of other shapes through the facade (back to not-live for the headless rules) */
    ail_set_live(0);
    wb5 = make_wav(44100, 1, 16, 5, 100, &wl5);        /* samples 100..104 at the output rate */
    wb11 = make_wav(11025, 1, 16, 110, 7, &wl11);
    wb8 = make_wav(22050, 1, 8, 100, 3, &wl8);
    wb0 = make_wav(22050, 1, 16, 0, 0, &wl0);
    check(ail_set_sample_file(h[1], wb11, wl11) == 1, "an 11,025 Hz WAV is accepted");
    check(ail_set_sample_file(h[2], wb8, wl8) == 1, "an 8-bit WAV is accepted");
    check(ail_set_sample_file(h[3], wb0, wl0) == 1, "a 0-frame WAV is accepted");
    ail_set_sample_loop_count(h[4], 0);
    ail_start_sample(h[4]);                            /* an allocated handle with no file */
    check(ail_sample_status(h[4]) == AIL_DONE && ail_active_sample_count() == 0,
          "start on an empty handle with loop count 0 stays done, nothing active");
    ail_set_sample_loop_count(h[3], 0);
    ail_start_sample(h[3]);
    check(ail_sample_status(h[3]) == AIL_DONE && ail_active_sample_count() == 0,
          "a 0-frame WAV counts as no data: start does nothing even with loop count 0");

    /* init_sample resets loops/volume and clears the data */
    ail_set_sample_loop_count(h[1], 0);
    ail_set_sample_volume(h[1], 55);
    ail_init_sample(h[1]);
    check(ail_sample_status(h[1]) == AIL_DONE, "init_sample: done");
    ail_set_sample_loop_count(h[1], 0);
    ail_start_sample(h[1]);
    check(ail_sample_status(h[1]) == AIL_DONE && ail_active_sample_count() == 0,
          "init_sample cleared the data: start with loop count 0 does nothing");
    ail_set_sample_loop_count(h[1], 1);
    ail_start_sample(h[1]);
    check(ail_sample_status(h[1]) == AIL_DONE, "init_sample cleared the data: start does nothing");
    check(ail_set_sample_file(h[1], wb5, wl5) == 1, "set_sample_file after init");
    ail_start_sample(h[1]);                            /* not live; loops reset to 1: done at once */
    check(ail_sample_status(h[1]) == AIL_DONE, "init_sample reset the loop count to 1 (not-live: done at once)");

    /* live: exact rendered values, restart position, volume, loop counts through the facade */
    ail_set_live(1);
    ail_set_sample_loop_count(h[1], 0);
    ail_set_sample_volume(h[1], 127);
    ail_start_sample(h[1]);
    check(ail_sample_status(h[1]) == AIL_PLAYING, "live: playing");
    ail_render(out, 2);
    check(out[0] == 100 && out[2] == 101, "the rendered values are the sample's (100, 101)");
    ail_start_sample(h[1]);                            /* start restarts even while playing */
    ail_render(out, 3);
    check(out[0] == 100 && out[2] == 101 && out[4] == 102, "start restarts the position at frame 0");
    ail_set_sample_volume(h[1], 0);
    ail_render(out, 1);
    check(out[0] == 0, "volume 0 silences the playing voice");
    ail_set_sample_volume(h[1], 127);
    ail_render(out, 1);
    check(out[0] == 104, "volume 127 again (frame 4 = 104)");
    ail_set_sample_loop_count(h[1], 1);                /* set while playing: ends at the run-out */
    ail_render(out, 2);
    check(ail_sample_status(h[1]) == AIL_DONE, "loop count set to 1 while playing ends it at the run-out");
    ail_set_sample_loop_count(h[1], 0);                /* a done sample must not come back */
    ail_set_sample_volume(h[1], 99);
    check(ail_sample_status(h[1]) == AIL_DONE, "a done sample does not come back (loop count / volume inert)");
    ail_end_sample(h[1]);
    check(ail_sample_status(h[1]) == AIL_DONE, "end on a done handle stays done");

    /* loop count 3 through the facade: 15 frames of the 5-frame sample, then done */
    ail_set_sample_loop_count(h[1], 3);
    ail_start_sample(h[1]);
    ail_render(out, 10);
    check(ail_sample_status(h[1]) == AIL_PLAYING, "loops 3: playing after 10 frames");
    ail_render(out, 5);
    check(ail_sample_status(h[1]) == AIL_DONE, "loops 3: done after 15 frames");

    /* a file on a PLAYING handle keeps status 4 (accepted or refused) */
    ail_set_sample_loop_count(h[1], 0);
    ail_start_sample(h[1]);
    check(ail_sample_status(h[1]) == AIL_PLAYING, "playing again");
    check(ail_set_sample_file(h[1], wb11, wl11) == 1, "a new file on a playing handle is accepted");
    check(ail_sample_status(h[1]) == AIL_PLAYING, "a successful set_sample_file keeps status 4");
    check(ail_set_sample_file(h[1], voc, vocl) == 0, "a refused file on a playing handle");
    check(ail_sample_status(h[1]) == AIL_PLAYING, "a refused file keeps status 4");
    ail_end_sample(h[1]);
    check(ail_sample_status(h[1]) == AIL_DONE, "end after the file swap: done");

    /* live -> not live converts the running voices under the lock */
    check(ail_set_sample_file(h[1], wb5, wl5) == 1, "load the 5-frame WAV");
    ail_set_sample_loop_count(h[1], 1);
    ail_start_sample(h[1]);
    check(ail_sample_status(h[1]) == AIL_PLAYING, "live: playing");
    ail_set_live(0);
    check(ail_sample_status(h[1]) == AIL_DONE && ail_active_sample_count() == 0,
          "set_live(0) converts a running count-1 voice to done");
    ail_set_sample_loop_count(h[1], 0);
    ail_set_live(1);
    ail_start_sample(h[1]);
    check(ail_sample_status(h[1]) == AIL_PLAYING, "live again: playing");
    ail_set_live(0);
    check(ail_sample_status(h[1]) == AIL_PLAYING && ail_active_sample_count() == 1,
          "set_live(0) with loop count 0: stays playing (no voice, still counted active)");
    ail_set_live(1);
    ail_end_sample(h[1]);

    /* a run-out voice that a new WAV in its slot cannot bring back: the play-once sample's last source frame
       is the last frame of one ail_render call, so the voice is still active when the status reads 2; the
       status call must retire the voice before dropping the id, so the next set_sample_file cannot revive it */
    wb100 = make_wav(44100, 1, 16, 100, 100, &wl100);    /* 100 frames at the output rate */
    wb1000 = make_wav(44100, 1, 16, 1000, 100, &wl1000); /* a longer WAV for the same slot */
    ail_set_live(1);
    check(ail_set_sample_file(h[2], wb100, wl100) == 1, "load the 100-frame WAV");
    ail_set_sample_loop_count(h[2], 1);
    ail_set_sample_volume(h[2], 127);
    ail_start_sample(h[2]);
    check(ail_sample_status(h[2]) == AIL_PLAYING, "run-out case: playing");
    memset(out, 0x55, sizeof out);
    ail_render(out, 100);                            /* 100 output frames = the whole 100-frame sample */
    check(ail_sample_status(h[2]) == AIL_DONE, "the last frame was the last of the call: status reads 2");
    check(ail_active_sample_count() == 0, "run-out case: none active");
    check(ail_set_sample_file(h[2], wb1000, wl1000) == 1, "the longer WAV loads into the slot");
    memset(out, 0x55, sizeof out);
    ail_render(out, 100);
    {
        int k, silent = 1;

        for (k = 0; k < 200; k++)
            if (out[k] != 0) {
                silent = 0;
                break;
            }
        check(silent, "the next render is silent: the run-out voice was retired before its data was replaced");
    }

    /* the same through ail_active_sample_count, the other place the id is dropped */
    check(ail_set_sample_file(h[5], wb100, wl100) == 1, "h5: the 100-frame WAV");
    ail_set_sample_loop_count(h[5], 1);
    ail_set_sample_volume(h[5], 127);
    ail_start_sample(h[5]);
    check(ail_active_sample_count() == 1, "h5: one active");
    memset(out, 0x55, sizeof out);
    ail_render(out, 100);
    check(ail_active_sample_count() == 0, "h5: the run-out voice is not counted");
    check(ail_set_sample_file(h[5], wb1000, wl1000) == 1, "h5: the longer WAV loads");
    memset(out, 0x55, sizeof out);
    ail_render(out, 100);
    {
        int k, silent = 1;

        for (k = 0; k < 200; k++)
            if (out[k] != 0) {
                silent = 0;
                break;
            }
        check(silent, "h5: the next render is silent too (the count path retires the voice)");
    }

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
    /* stop/resume/end act on the refreshed status: a song ended by its loop count reads 2, and they do nothing */
    ail_stop_sequence(sh);
    check(ail_sequence_status(sh) == AIL_DONE, "stop when not 4 does nothing");
    ail_resume_sequence(sh);
    check(ail_sequence_status(sh) == AIL_DONE, "resume when not 8 does nothing");
    ail_start_sequence(sh);
    check(ail_sequence_status(sh) == AIL_PLAYING, "playing again");
    ail_resume_sequence(sh);
    check(ail_sequence_status(sh) == AIL_PLAYING, "resume while playing does nothing");
    ail_stop_sequence(sh);
    check(ail_sequence_status(sh) == AIL_PAUSED, "stop: paused");
    ail_stop_sequence(sh);
    check(ail_sequence_status(sh) == AIL_PAUSED, "stop while paused does nothing");
    ail_resume_sequence(sh);
    check(ail_sequence_status(sh) == AIL_PLAYING, "resume: playing");
    /* init_sequence resets loop count 1 and volume 127 (not masked by earlier sets) */
    ail_set_sequence_loop_count(sh, 0);
    ail_set_sequence_volume(sh, 64, 0);
    check(ail_init_sequence(sh, xmi2, xl, 0) == 1, "re-init ok");
    check(ail_sequence_status(sh) == AIL_DONE, "init: done");
    ail_start_sequence(sh);
    ail_render(out, 44100);
    check(ail_sequence_status(sh) == AIL_DONE, "init reset the loop count to 1 (ended, not looping forever)");
    /* a failed init on a playing handle: status 2, the old song ended */
    ail_start_sequence(sh);
    check(ail_sequence_status(sh) == AIL_PLAYING, "playing before the failed init");
    check(ail_init_sequence(sh, xmi2, xl, 2) == 0, "a failed init_sequence still returns 0");
    check(ail_sequence_status(sh) == AIL_DONE, "init_sequence sets status done first, even when it fails");
    ail_stop_sequence(sh);
    check(ail_sequence_status(sh) == AIL_DONE, "stop after a failed init does nothing");

    check(g_lock_balance == 0, "every lock(1) has its lock(0)");
    ail_shutdown();
    free(wavbuf);
    free(voc);
    free(junk);
    free(badwav);
    free(wb5);
    free(wb11);
    free(wb8);
    free(wb0);
    free(wb100);
    free(wb1000);
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
