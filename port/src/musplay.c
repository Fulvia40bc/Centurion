#include "musplay.h"

#include <stdlib.h>
#include <string.h>

#include "gm_synth.h"      /* gm_tick_frames only; the backend itself is the caller's */

enum { MUS_RATE_DEFAULT = 44100 };

struct musplay {
    mus_backend backend;
    int has_backend;
    int rate;
    xmi_song song;          /* owned */
    int has_song;
    xmi_seq *seq;
    int status;
    int volume;             /* 0..127, default 127 */
    int loops;              /* the FF 2F count, default 1 */
    uint64_t tick;          /* output ticks (1/120 s each) started so far */
    int tick_off;           /* frames already rendered of the current output tick */
};

static uint32_t rd_u32be(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static void seq_out(void *user, const uint8_t *msg, int len)
{
    musplay *p = user;

    if (p->has_backend && p->backend.midi != NULL)
        p->backend.midi(p->backend.u, msg, len);
}

/* the channel set-up AIL sends at driver install, on all 16 channels: CC114 0, program 0, pitch bend 0x2000,
   CC112 0, CC1 0, CC7 127, CC10 64, CC11 127, CC64 0, CC91 40, CC93 0, CC100 0, CC101 0, CC38 0, CC6 2 */
static void mus_send_setup(musplay *p)
{
    static const uint8_t ccs[12][2] = {
        { 112, 0 }, { 1, 0 }, { 7, 127 }, { 10, 64 }, { 11, 127 }, { 64, 0 },
        { 91, 40 }, { 93, 0 }, { 100, 0 }, { 101, 0 }, { 38, 0 }, { 6, 2 }
    };
    int ch, i;

    if (!p->has_backend || p->backend.midi == NULL)
        return;
    for (ch = 0; ch < 16; ch++) {
        uint8_t msg[3];

        msg[0] = (uint8_t)(0xB0u | (unsigned)ch); msg[1] = 114; msg[2] = 0;
        p->backend.midi(p->backend.u, msg, 3);
        msg[0] = (uint8_t)(0xC0u | (unsigned)ch); msg[1] = 0;
        p->backend.midi(p->backend.u, msg, 2);                       /* program 0 */
        msg[0] = (uint8_t)(0xE0u | (unsigned)ch); msg[1] = 0x00; msg[2] = 0x40;
        p->backend.midi(p->backend.u, msg, 3);                       /* pitch bend 0x2000 */
        for (i = 0; i < 12; i++) {
            msg[0] = (uint8_t)(0xB0u | (unsigned)ch);
            msg[1] = ccs[i][0];
            msg[2] = ccs[i][1];
            p->backend.midi(p->backend.u, msg, 3);
        }
    }
}

/* Decide as 0x99180 does (ailsem 7 step 3): walk the top chunks ("FORM"/"CAT " only, ids case-sensitive,
   big-endian sizes, else fail); a chunk whose type at +8 is not "XMID" (FORM XDIR) is skipped; the FIRST
   FORM/CAT of type XMID decides and the walk never goes on: a lone FORM XMID is the one sequence (n must be
   0), a CAT XMID gives its n-th sub-chunk of type XMID (0-based), else fail. No pad byte is added after an
   odd size. */
int mus_probe(const uint8_t *xmi, size_t len, int n, xmi_song *out)
{
    size_t pos = 0;

    if (out == NULL)
        return -1;
    memset(out, 0, sizeof *out);
    if (xmi == NULL || len < 8 || n < 0)
        return -1;
    while (pos + 8 <= len) {
        const uint8_t *tag = xmi + pos;
        uint32_t size = rd_u32be(xmi + pos + 4);
        size_t body = pos + 8;

        if ((size_t)size > len - body)
            size = (uint32_t)(len - body);
        if (memcmp(tag, "FORM", 4) != 0 && memcmp(tag, "CAT ", 4) != 0)
            return -1;                            /* top chunks are FORM/CAT only, else fail */
        if (body + 4 <= len && memcmp(xmi + body, "XMID", 4) == 0) {
            if (memcmp(tag, "FORM", 4) == 0) {    /* a lone FORM XMID: the one sequence, n must be 0 */
                if (n != 0)
                    return -1;
                return xmi_parse(xmi + pos, 8 + (size_t)size, out);   /* fails when there is no EVNT */
            } else {                              /* CAT XMID: the n-th XMID sub-chunk, 0-based */
                size_t p2 = body + 4;
                size_t end2 = body + size;
                int idx = 0;

                if (end2 > len)
                    end2 = len;
                while (p2 + 8 <= end2) {
                    const uint8_t *t2 = xmi + p2;
                    uint32_t s2 = rd_u32be(xmi + p2 + 4);

                    if ((size_t)s2 > len - (p2 + 8))
                        s2 = (uint32_t)(len - (p2 + 8));
                    if (memcmp(t2, "FORM", 4) == 0 && p2 + 12 <= len &&
                        memcmp(xmi + p2 + 8, "XMID", 4) == 0) {
                        if (idx == n)
                            return xmi_parse(xmi + p2, 8 + (size_t)s2, out);
                        idx++;
                    }
                    p2 += 8 + s2;                 /* no pad byte after an odd size */
                }
                return -1;
            }
        }
        pos = body + size;                        /* no pad byte after an odd size */
    }
    return -1;
}

musplay *mus_new(const mus_backend *b, int rate)
{
    musplay *p = calloc(1, sizeof *p);

    if (p == NULL)
        return NULL;
    if (b != NULL) {
        p->backend = *b;
        p->has_backend = 1;
    }
    if (rate <= 0)
        rate = MUS_RATE_DEFAULT;
    p->rate = rate;
    p->status = MUS_FREE;
    p->volume = 127;
    p->loops = 1;
    mus_send_setup(p);                            /* once, at mus_new */
    return p;
}

void mus_free(musplay *p)
{
    if (p == NULL)
        return;
    if (p->seq != NULL)
        xmi_seq_free(p->seq);
    if (p->has_song)
        xmi_free(&p->song);
    free(p);
}

int mus_install(musplay *p, xmi_song *song)
{
    if (song == NULL)
        return 0;
    if (p == NULL) {                              /* no player (no memory at ail_init): the song is still ours */
        xmi_free(song);
        return 0;
    }
    if (p->seq != NULL) {                         /* the previous song is ended first */
        xmi_seq_stop(p->seq);
        xmi_seq_free(p->seq);
        p->seq = NULL;
    }
    if (p->has_song) {
        xmi_free(&p->song);
        p->has_song = 0;
    }
    if (p->has_backend && p->backend.reset != NULL)
        p->backend.reset(p->backend.u);          /* between songs */
    if (p->has_backend && p->backend.timbres != NULL && song->ntimb > 0) {
        uint8_t body[2 + 2 * 64];                /* AIL_init_sequence's 0x503 (structure.md 8.3): the TIMB chunk's
                                                    u16 LE count and pairs (xmi.c keeps up to 64; the songs have
                                                    at most 31). Port guard: no TIMB or a count of 0 installs
                                                    nothing (the driver would walk 65,536 entries, 0x2DCB) */
        int i;

        body[0] = (uint8_t)(song->ntimb & 0xFF);
        body[1] = (uint8_t)(song->ntimb >> 8);
        for (i = 0; i < song->ntimb && i < 64; i++) {
            body[2 + 2 * i] = song->timb[i][0];
            body[3 + 2 * i] = song->timb[i][1];
        }
        if (!p->backend.timbres(p->backend.u, body, 2 + 2 * (size_t)i)) {
            xmi_free(song);                      /* "Driver could not install timbre bank ..": the caller fails */
            p->status = MUS_DONE;
            return 0;
        }
    }
    p->song = *song;
    p->has_song = 1;
    p->seq = xmi_seq_new(&p->song, seq_out, NULL, p);
    if (p->seq == NULL) {
        xmi_free(&p->song);
        p->has_song = 0;
        return 0;
    }
    p->loops = 1;
    p->volume = 127;
    xmi_seq_set_loops(p->seq, 1);
    xmi_seq_set_volume(p->seq, 127);
    p->status = MUS_DONE;                         /* position at the start, not playing */
    p->tick = 0;
    p->tick_off = 0;
    return 1;
}

int mus_load(musplay *p, const uint8_t *xmi, size_t len, int n)
{
    xmi_song song;

    if (p == NULL)
        return 0;
    if (mus_probe(xmi, len, n, &song) != 0)
        return 0;
    return mus_install(p, &song);
}

void mus_start(musplay *p)
{
    if (p == NULL || !p->has_song || p->seq == NULL)
        return;
    if (p->backend.reset != NULL)                 /* the channel set-up first, then the song's events: only for a */
        mus_send_setup(p);                        /* backend reset between songs (GM). An FM driver keeps its state
                                                     as the original's does, which sends the set-up once, at the
                                                     driver's install (0x9AD90..0x9AE96: mus_new) */
    xmi_seq_rewind(p->seq);
    xmi_seq_resume(p->seq);
    xmi_seq_advance(p->seq, 0);                   /* flush the events at tick 0 */
    p->status = MUS_PLAYING;
}

void mus_start_paused(musplay *p)
{
    if (p == NULL || !p->has_song || p->seq == NULL)
        return;
    if (p->backend.reset != NULL)
        mus_send_setup(p);
    xmi_seq_rewind(p->seq);                       /* rewound and paused: nothing of the song is sent until
                                                     mus_resume, whose first tick takes the tick-0 events */
    p->status = MUS_PAUSED;
}

void mus_pause(musplay *p)
{
    if (p == NULL || p->status != MUS_PLAYING)
        return;
    xmi_seq_pause(p->seq);
    p->status = MUS_PAUSED;
}

void mus_resume(musplay *p)
{
    if (p == NULL || p->status != MUS_PAUSED)
        return;
    xmi_seq_resume(p->seq);
    p->status = MUS_PLAYING;
}

void mus_end(musplay *p)
{
    if (p == NULL)
        return;
    if (p->seq != NULL)
        xmi_seq_stop(p->seq);                      /* stop the notes */
    p->status = MUS_DONE;
}

void mus_volume(musplay *p, int v)
{
    if (p == NULL)
        return;
    if (v < 0)
        v = 0;
    if (v > 127)
        v = 127;
    p->volume = v;
    if (p->seq != NULL)
        xmi_seq_set_volume(p->seq, v);
}

void mus_loops(musplay *p, int n)
{
    if (p == NULL)
        return;
    p->loops = n;
    if (p->seq != NULL)
        xmi_seq_set_loops(p->seq, n);
}

int mus_status(musplay *p)
{
    if (p == NULL)
        return MUS_FREE;
    if (p->status == MUS_PLAYING && p->seq != NULL && xmi_seq_done(p->seq))
        p->status = MUS_DONE;                     /* ended through its loop count */
    return p->status;
}

void mus_render(void *pp, int16_t *out, int frames)
{
    musplay *p = pp;
    int off = 0, left = frames;

    if (p == NULL || out == NULL || frames <= 0)
        return;
    while (left > 0) {
        int full = gm_tick_frames(p->tick, p->rate) - p->tick_off;   /* samples left in 120 Hz tick p->tick */
        int n = full > left ? left : full;

        if (p->has_backend && p->backend.render != NULL)
            p->backend.render(p->backend.u, out + 2 * (size_t)off, n);
        else
            memset(out + 2 * (size_t)off, 0, (size_t)n * 2u * sizeof(int16_t));
        off += n;
        left -= n;
        p->tick_off += n;
        if (p->tick_off == gm_tick_frames(p->tick, p->rate)) {   /* a whole 1/120 s of output passed */
            p->tick++;
            p->tick_off = 0;
            if (p->status == MUS_PLAYING && p->seq != NULL) {
                xmi_seq_advance(p->seq, 1);
                if (xmi_seq_done(p->seq))
                    p->status = MUS_DONE;
            }
        }
    }
}
