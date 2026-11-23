#include "ail.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gm_synth.h"
#include "mixer.h"
#include "musplay.h"
#include "wav.h"

enum { AIL_RATE = 44100, PREF_MAX = 32 };

typedef struct {
    int used;
    int status;          /* AIL_FREE / AIL_DONE / AIL_PLAYING */
    int loops;
    int volume;
    wav_sound wav;       /* owned when has_data */
    int has_data;
    int id;              /* the mixer id while playing, -1 when not */
} ail_sample;

typedef struct {
    int used;
    int status;          /* AIL_FREE / AIL_DONE / AIL_PLAYING / AIL_PAUSED */
} ail_seq;

static void (*g_lock)(int on);
static mixer *g_mix;
static musplay *g_mus;
static gm_synth *g_gm;
static int g_live;
static ail_sample g_smp[AIL_MAX_SAMPLES + 1];    /* index 1..16 */
static ail_seq g_seq[AIL_MAX_SEQUENCES + 1];      /* index 1..8 */
static char g_err[128];
static int32_t g_pref[PREF_MAX];
static int g_pref_seen[PREF_MAX];

static void lock_on(void) { if (g_lock != NULL) g_lock(1); }
static void lock_off(void) { if (g_lock != NULL) g_lock(0); }

static void set_err(const char *msg)
{
    snprintf(g_err, sizeof g_err, "%s", msg);
}

/* gm_synth as a mus_backend */
static void gm_midi_cb(void *u, const uint8_t *msg, int len) { gm_synth_midi(u, msg, len); }
static void gm_render_cb(void *u, int16_t *out, int frames) { gm_synth_render((gm_synth *)u, out, frames); }
static void gm_reset_cb(void *u) { gm_synth_reset((gm_synth *)u); }

void ail_init(void (*lock)(int on), const char *sf2_path)
{
    memset(g_smp, 0, sizeof g_smp);
    memset(g_seq, 0, sizeof g_seq);
    g_err[0] = '\0';
    memset(g_pref, 0, sizeof g_pref);
    memset(g_pref_seen, 0, sizeof g_pref_seen);
    g_lock = lock;
    g_live = 0;
    g_gm = NULL;
    g_mix = mix_new();
    g_mus = NULL;
    if (g_mix != NULL) {
        if (sf2_path != NULL)
            g_gm = gm_synth_new(sf2_path, AIL_RATE);
        if (g_gm != NULL) {
            mus_backend b;

            b.midi = gm_midi_cb;
            b.render = gm_render_cb;
            b.reset = gm_reset_cb;
            b.u = g_gm;
            g_mus = mus_new(&b, AIL_RATE);
        } else {
            g_mus = mus_new(NULL, AIL_RATE);    /* no SoundFont / no FluidSynth: silent, same state machine */
        }
        if (g_mus != NULL)
            mix_set_music(g_mix, mus_render, g_mus, 127);
    }
}

void ail_shutdown(void)
{
    if (g_mus != NULL) {
        mus_free(g_mus);
        g_mus = NULL;
    }
    if (g_gm != NULL) {
        gm_synth_free(g_gm);
        g_gm = NULL;
    }
    if (g_mix != NULL) {
        mix_free(g_mix);
        g_mix = NULL;
    }
    {
        int i;

        for (i = 1; i <= AIL_MAX_SAMPLES; i++)
            if (g_smp[i].has_data)
                wav_free(&g_smp[i].wav);
    }
    memset(g_smp, 0, sizeof g_smp);
    memset(g_seq, 0, sizeof g_seq);
    g_live = 0;
}

void ail_render(int16_t *out, int frames)
{
    if (g_mix != NULL)
        mix_render(g_mix, out, frames);
}

void ail_set_live(int on)
{
    int i;

    on = on ? 1 : 0;
    lock_on();
    if (g_live && !on) {                          /* live -> not live: convert the running voices now */
        for (i = 1; i <= AIL_MAX_SAMPLES; i++) {
            ail_sample *s = &g_smp[i];

            if (s->used && s->status == AIL_PLAYING && s->id >= 0) {
                mix_halt(g_mix, s->id);
                s->id = -1;
                if (s->loops != 0)
                    s->status = AIL_DONE;         /* count 1 or n > 1: done; count 0 stays playing, no voice */
            }
        }
    }
    g_live = on;
    lock_off();
}

int32_t ail_set_preference(int n, int32_t v)
{
    int32_t old;

    if (n < 0 || n >= PREF_MAX)
        return 0;
    lock_on();
    if (!g_pref_seen[n]) {
        g_pref[n] = (n == 1) ? 1 : 0;            /* defaults: 1 for n = 1, 0 for n = 7 (and the rest) */
        g_pref_seen[n] = 1;
    }
    old = g_pref[n];
    g_pref[n] = v;
    lock_off();
    return old;
}

static ail_sample *smp_of(int32_t h)
{
    if (h < 1 || h > AIL_MAX_SAMPLES)
        return NULL;
    return &g_smp[h];
}

static ail_seq *seq_of(int32_t h)
{
    if (h < 1 || h > AIL_MAX_SEQUENCES)
        return NULL;
    return &g_seq[h];
}

int32_t ail_allocate_sample(void)
{
    int i;

    lock_on();
    for (i = 1; i <= AIL_MAX_SAMPLES; i++) {
        if (!g_smp[i].used) {
            g_smp[i].used = 1;
            g_smp[i].status = AIL_DONE;
            g_smp[i].loops = 1;
            g_smp[i].volume = 100;
            g_smp[i].has_data = 0;
            g_smp[i].id = -1;
            lock_off();
            return (int32_t)i;
        }
    }
    set_err("Out of sample handles\n");
    lock_off();
    return 0;
}

void ail_init_sample(int32_t h)
{
    ail_sample *s = smp_of(h);

    if (s == NULL || !s->used)
        return;
    lock_on();
    if (s->id >= 0) {
        mix_halt(g_mix, s->id);
        s->id = -1;
    }
    if (s->has_data) {                            /* AIL sets start, length and position to 0 */
        wav_free(&s->wav);
        s->has_data = 0;
    }
    s->status = AIL_DONE;
    s->loops = 1;
    s->volume = 100;
    lock_off();
}

int ail_set_sample_file(int32_t h, const uint8_t *img, size_t len)
{
    ail_sample *s = smp_of(h);
    int rc = 0;

    if (s == NULL || !s->used)
        return 0;
    lock_on();
    if (img != NULL && len >= 20 && memcmp(img, "Creative", 8) == 0) {
        set_err("Unrecognized digital audio file type\n");        /* VOC: the game ships none */
    } else if (img != NULL && len >= 12 && memcmp(img + 8, "WAVE", 4) == 0) {
        wav_sound w;

        if (wav_decode(img, len, &w) == 0) {
            if (s->id >= 0) {                                     /* a playing sound is halted first */
                mix_halt(g_mix, s->id);
                s->id = -1;
            }
            /* the status does not change (AIL only sets the address, ailsem 5) */
            if (s->has_data)
                wav_free(&s->wav);
            if (w.samples != NULL && w.frames > 0) {
                s->wav = w;
                s->has_data = 1;                                  /* a 0-frame WAV counts as no data */
            } else {
                wav_free(&w);
                s->has_data = 0;
            }
            rc = 1;
        } else {
            set_err("Unrecognized digital audio file type\n");
        }
    } else {
        set_err("Unrecognized digital audio file type\n");
    }
    lock_off();
    return rc;
}

void ail_set_sample_volume(int32_t h, int v)
{
    ail_sample *s = smp_of(h);

    if (s == NULL || !s->used)
        return;
    if (v < 0)
        v = 0;
    if (v > 127)
        v = 127;
    lock_on();
    s->volume = v;
    if (s->id >= 0)
        mix_volume(g_mix, s->id, v);
    lock_off();
}

void ail_set_sample_loop_count(int32_t h, int n)
{
    ail_sample *s = smp_of(h);

    if (s == NULL || !s->used)
        return;
    lock_on();
    s->loops = n;
    if (s->id >= 0)
        mix_loops(g_mix, s->id, n);
    lock_off();
}

void ail_start_sample(int32_t h)
{
    ail_sample *s = smp_of(h);

    if (s == NULL || !s->used || !s->has_data)
        return;                                               /* nothing if the handle has no data */
    lock_on();
    if (s->id >= 0) {
        mix_halt(g_mix, s->id);
        s->id = -1;
    }
    if (g_live) {
        s->id = mix_start(g_mix, &s->wav, s->volume, s->loops);
        s->status = (s->id >= 0) ? AIL_PLAYING : AIL_DONE;
    } else {
        /* not live (headless): a loop count of 1 is done at once, nothing mixed; 0 stays playing until ended;
           n > 1 also ends at once here (the game uses 0 and 1) */
        s->id = -1;
        s->status = (s->loops == 0) ? AIL_PLAYING : AIL_DONE;
    }
    lock_off();
}

void ail_end_sample(int32_t h)
{
    ail_sample *s = smp_of(h);

    if (s == NULL || !s->used)
        return;
    lock_on();
    if (s->status == AIL_PLAYING) {
        if (s->id >= 0)
            mix_halt(g_mix, s->id);
        s->id = -1;
        s->status = AIL_DONE;                                 /* end on a done handle stays done */
    }
    lock_off();
}

int ail_sample_status(int32_t h)
{
    ail_sample *s = smp_of(h);
    int st;

    if (s == NULL)
        return 0;                                             /* outside 1..16 */
    if (!s->used)
        return AIL_FREE;                                      /* unallocated inside the range: free */
    lock_on();
    st = s->status;
    if (st == AIL_PLAYING && s->id >= 0 && !mix_active(g_mix, s->id)) {
        s->id = -1;                                           /* the data ran out with count 1: done, and the
                                                             id is dropped so nothing revives the voice */
        st = s->status = AIL_DONE;
    }
    lock_off();
    return st;
}

int ail_active_sample_count(void)
{
    int i, n = 0;

    lock_on();
    for (i = 1; i <= AIL_MAX_SAMPLES; i++) {
        if (g_smp[i].used && g_smp[i].status == AIL_PLAYING) {
            if (g_smp[i].id >= 0 && !mix_active(g_mix, g_smp[i].id)) {
                g_smp[i].id = -1;
                g_smp[i].status = AIL_DONE;                   /* not-live plays have no voice: they stay 4 */
            } else {
                n++;
            }
        }
    }
    lock_off();
    return n;
}

int32_t ail_allocate_sequence(void)
{
    int i;

    lock_on();
    for (i = 1; i <= AIL_MAX_SEQUENCES; i++) {
        if (!g_seq[i].used) {
            g_seq[i].used = 1;
            g_seq[i].status = AIL_DONE;
            lock_off();
            return (int32_t)i;
        }
    }
    set_err("Out of sequence handles\n");
    lock_off();
    return 0;
}

/* refresh the cached status from the player (a song ended by its loop count reads DONE) */
static void seq_refresh(ail_seq *q)
{
    if (q->status == AIL_PLAYING && mus_status(g_mus) == MUS_DONE)
        q->status = AIL_DONE;
}

int ail_init_sequence(int32_t h, const uint8_t *img, size_t len, int n)
{
    ail_seq *q = seq_of(h);
    xmi_song song;

    if (q == NULL || !q->used)
        return 0;
    lock_on();
    if (q->status == AIL_PLAYING || q->status == AIL_PAUSED)
        mus_end(g_mus);                                       /* status 2 stops the timer servicing the old
                                                             song: end it, so stop/end can reach it */
    q->status = AIL_DONE;                                     /* status 2 first */
    lock_off();
    if (mus_probe(img, len, n, &song) != 0) {                 /* the parse is done outside the lock */
        set_err("Invalid XMIDI sequence\n");
        return 0;
    }
    lock_on();
    mus_install(g_mus, &song);                                /* the previous song is ended first, under the lock */
    lock_off();
    return 1;
}

void ail_start_sequence(int32_t h)
{
    ail_seq *q = seq_of(h);

    if (q == NULL || !q->used)
        return;                                               /* nothing on a free handle */
    lock_on();
    mus_start(g_mus);                                         /* rewind and play; loop count and volume kept */
    q->status = AIL_PLAYING;
    lock_off();
}

void ail_stop_sequence(int32_t h)
{
    ail_seq *q = seq_of(h);

    if (q == NULL || !q->used)
        return;
    lock_on();
    seq_refresh(q);
    if (q->status == AIL_PLAYING) {                           /* only when 4 */
        mus_pause(g_mus);
        q->status = AIL_PAUSED;
    }
    lock_off();
}

void ail_resume_sequence(int32_t h)
{
    ail_seq *q = seq_of(h);

    if (q == NULL || !q->used)
        return;
    lock_on();
    seq_refresh(q);
    if (q->status == AIL_PAUSED) {                            /* only when 8 */
        mus_resume(g_mus);
        q->status = AIL_PLAYING;
    }
    lock_off();
}

void ail_end_sequence(int32_t h)
{
    ail_seq *q = seq_of(h);

    if (q == NULL || !q->used)
        return;
    lock_on();
    seq_refresh(q);
    if (q->status == AIL_PLAYING || q->status == AIL_PAUSED) {
        mus_end(g_mus);                                       /* stop the notes, CC64 0 where sustain is down */
        q->status = AIL_DONE;
    }
    lock_off();
}

int ail_sequence_status(int32_t h)
{
    ail_seq *q = seq_of(h);
    int st;

    if (q == NULL)
        return 0;                                             /* outside 1..8 */
    if (!q->used)
        return AIL_FREE;                                      /* unallocated inside the range: free */
    lock_on();
    seq_refresh(q);
    st = q->status;
    lock_off();
    return st;
}

void ail_set_sequence_volume(int32_t h, int v, int ms)
{
    ail_seq *q = seq_of(h);

    (void)ms;                                             /* always 0 in the game; any ms is treated as 0 */
    if (q == NULL || !q->used)
        return;
    lock_on();
    mus_volume(g_mus, v);
    lock_off();
}

void ail_set_sequence_loop_count(int32_t h, int n)
{
    ail_seq *q = seq_of(h);

    if (q == NULL || !q->used)
        return;
    lock_on();
    mus_loops(g_mus, n);
    lock_off();
}

const char *ail_last_error(void)
{
    return g_err;
}
