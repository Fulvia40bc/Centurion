#include "mixer.h"

#include <stdlib.h>
#include <string.h>

#define MIX_OUT_RATE   44100u
#define MIX_VOICES     8
#define MIX_MUSIC_MAX  4096

typedef struct {
    const wav_sound *s;   /* not owned; the caller keeps it alive while the voice plays */
    uint64_t p;          /* position, 32 fraction bits */
    uint64_t step;       /* (rate << 32) / 44100 */
    int volume;
    int gl, gr;
    int active;
    int loops;           /* read when the data runs out (mix_start); mix_play uses 1 */
    uint32_t gen;        /* bumped by every mix_start on this voice, for the id */
} mix_voice;

struct mixer {
    mix_voice v[MIX_VOICES];
    void (*music_render)(void *user, int16_t *out, int frames);
    void *music_user;
    int music_volume;
    int16_t *mbuf;       /* MIX_MUSIC_MAX stereo frames */
    uint32_t clipped;
};

/* arithmetic shift right by 16, without an implementation-defined shift of a negative value */
static int32_t asr16(int32_t x)
{
    return x >= 0 ? x >> 16 : -(((-x) + 0xFFFF) >> 16);
}

/* int32 multiplication with two's-complement wraparound, well defined in C */
static int32_t mul32(int32_t a, int32_t b)
{
    uint32_t p = (uint32_t)a * (uint32_t)b;
    int32_t r;

    memcpy(&r, &p, sizeof r);
    return r;
}

static int clamp16(int32_t *v, uint32_t *clipped)
{
    if (*v > 32767) {
        *v = 32767;
        (*clipped)++;
        return *v;
    }
    if (*v < -32768) {
        *v = -32768;
        (*clipped)++;
    }
    return *v;
}

mixer *mix_new(void)
{
    return calloc(1, sizeof(mixer));
}

void mix_free(mixer *m)
{
    if (m == NULL)
        return;
    free(m->mbuf);
    free(m);
}

int mix_play(mixer *m, const wav_sound *s, int volume, int pan)
{
    int vi;

    if (m == NULL || s == NULL)
        return -1;
    if (volume < 0)
        volume = 0;
    if (volume > 127)
        volume = 127;
    if (pan < 0)
        pan = 0;
    if (pan > 127)
        pan = 127;
    for (vi = 0; vi < MIX_VOICES; vi++) {
        if (!m->v[vi].active) {
            mix_voice *vc = &m->v[vi];
            int gl, gr;

            if (s->channels == 2) {
                gl = 128;               /* stereo: pan ignored, channels map to L/R */
                gr = 128;
            } else {
                gl = pan <= 64 ? 128 : ((127 - pan) * 128) / 63;
                gr = pan >= 64 ? 128 : (pan * 128) / 64;
            }
            vc->s = s;
            vc->p = 0;
            vc->step = ((uint64_t)s->rate << 32) / MIX_OUT_RATE;
            vc->volume = volume;
            vc->gl = gl;
            vc->gr = gr;
            vc->loops = 1;
            vc->active = 1;
            return vi;
        }
    }
    return -1;
}

static int mix_id_of(const mix_voice *vc, int vi)
{
    uint32_t id32 = ((uint32_t)vi + 8u * vc->gen) & 0x7FFFFFFFu;   /* wraps inside int range, never negative */
    int id;

    memcpy(&id, &id32, sizeof id);
    return id;
}

static mix_voice *mix_find_id(mixer *m, int id, int *vi_out)
{
    int vi;

    if (m == NULL || id < 0)
        return NULL;
    vi = id & 7;
    if (vi >= MIX_VOICES)
        return NULL;
    if (!m->v[vi].active || mix_id_of(&m->v[vi], vi) != id)
        return NULL;                        /* stale: ended, stopped, or the voice was reused */
    if (vi_out != NULL)
        *vi_out = vi;
    return &m->v[vi];
}

int mix_start(mixer *m, const wav_sound *s, int volume, int loops)
{
    int vi;

    if (m == NULL || s == NULL)
        return -1;
    if (s->samples == NULL || s->frames == 0)
        return -1;                          /* AIL does not start an empty sample */
    if (volume < 0)
        volume = 0;
    if (volume > 127)
        volume = 127;
    for (vi = 0; vi < MIX_VOICES; vi++) {
        if (!m->v[vi].active) {
            mix_voice *vc = &m->v[vi];

            vc->gen++;                      /* the previous play on this voice, if any, is now stale */
            vc->s = s;
            vc->p = 0;
            vc->step = ((uint64_t)s->rate << 32) / MIX_OUT_RATE;
            vc->volume = volume;
            vc->gl = 128;                   /* pan 64 */
            vc->gr = 128;
            vc->loops = loops;
            vc->active = 1;
            return mix_id_of(vc, vi);
        }
    }
    return -1;
}

int mix_active(const mixer *m, int id)
{
    const mix_voice *vc;
    int vi;

    if (m == NULL || id < 0)
        return 0;
    vi = id & 7;
    if (vi >= MIX_VOICES)
        return 0;
    vc = &m->v[vi];
    if (!vc->active || mix_id_of(vc, vi) != id)
        return 0;
    if (vc->s == NULL || vc->s->samples == NULL)
        return 0;
    if ((uint32_t)(vc->p >> 32) >= vc->s->frames && vc->loops == 1)
        return 0;                           /* the data has run out and the count says end */
    return 1;
}

void mix_halt(mixer *m, int id)
{
    mix_voice *vc = mix_find_id(m, id, NULL);

    if (vc != NULL)
        vc->active = 0;
}

void mix_volume(mixer *m, int id, int volume)
{
    mix_voice *vc = mix_find_id(m, id, NULL);

    if (vc == NULL)
        return;
    if (volume < 0)
        volume = 0;
    if (volume > 127)
        volume = 127;
    vc->volume = volume;
}

void mix_loops(mixer *m, int id, int loops)
{
    mix_voice *vc = mix_find_id(m, id, NULL);

    if (vc != NULL)
        vc->loops = loops;
}

void mix_music_volume(mixer *m, int volume)
{
    if (m == NULL)
        return;
    if (volume < 0)
        volume = 0;
    if (volume > 127)
        volume = 127;
    m->music_volume = volume;
}

void mix_stop(mixer *m, int voice)
{
    if (m == NULL || voice < 0 || voice >= MIX_VOICES)
        return;
    m->v[voice].active = 0;
}

int mix_playing(const mixer *m, int voice)
{
    const mix_voice *vc;

    if (m == NULL || voice < 0 || voice >= MIX_VOICES)
        return 0;
    vc = &m->v[voice];
    return vc->active && vc->s != NULL && vc->s->samples != NULL &&
           (uint32_t)(vc->p >> 32) < vc->s->frames;
}

void mix_set_music(mixer *m, void (*render)(void *user, int16_t *out, int frames), void *user, int volume)
{
    if (m == NULL)
        return;
    if (volume < 0)
        volume = 0;
    if (volume > 127)
        volume = 127;
    if (render != NULL && m->mbuf == NULL) {
        m->mbuf = malloc((size_t)MIX_MUSIC_MAX * 2u * sizeof(int16_t));
        if (m->mbuf == NULL) {
            m->music_render = NULL;
            m->music_user = NULL;
            m->music_volume = 0;
            return;
        }
    } else if (render == NULL && m->mbuf != NULL) {
        free(m->mbuf);
        m->mbuf = NULL;
    }
    m->music_render = render;
    m->music_user = user;
    m->music_volume = volume;
}

void mix_render(mixer *m, int16_t *out, int frames)
{
    int f = 0;

    if (m == NULL || out == NULL || frames <= 0)
        return;
    while (f < frames) {
        int piece = frames - f;
        int i;

        if (piece > MIX_MUSIC_MAX)
            piece = MIX_MUSIC_MAX;
        if (m->music_render != NULL && m->mbuf != NULL)
            m->music_render(m->music_user, m->mbuf, piece);
        for (i = 0; i < piece; i++) {
            int32_t sumL = 0, sumR = 0;
            int vi;

            for (vi = 0; vi < MIX_VOICES; vi++) {
                mix_voice *vc = &m->v[vi];
                uint64_t pos;
                uint32_t fi;
                uint32_t frac;

                if (!vc->active)
                    continue;
                if (vc->s == NULL || vc->s->samples == NULL) {
                    vc->active = 0;             /* a 0-frame voice does not stay active forever */
                    continue;
                }
                pos = vc->p;
                fi = (uint32_t)(pos >> 32);
                if (fi >= vc->s->frames) {      /* the data ran out: the loop count is read now */
                    if (vc->loops == 0) {
                        vc->p &= 0xFFFFFFFFull; /* again from frame 0, keeping the resampling fraction */
                        continue;
                    }
                    if (vc->loops > 1) {
                        vc->loops--;
                        vc->p &= 0xFFFFFFFFull;
                        continue;
                    }
                    vc->active = 0;             /* loops == 1: the voice ends */
                    continue;
                }
                frac = (uint32_t)((pos >> 16) & 0xFFFFu);
                if (vc->s->channels == 2) {
                    int32_t l0 = vc->s->samples[2u * fi];
                    int32_t l1 = (fi + 1u < vc->s->frames) ? vc->s->samples[2u * (fi + 1u)] : l0;
                    int32_t r0 = vc->s->samples[2u * fi + 1u];
                    int32_t r1 = (fi + 1u < vc->s->frames) ? vc->s->samples[2u * (fi + 1u) + 1u] : r0;
                    int32_t lv = l0 + asr16(mul32(l1 - l0, (int32_t)frac));
                    int32_t rv = r0 + asr16(mul32(r1 - r0, (int32_t)frac));
                    sumL += (int32_t)(((int64_t)lv * vc->volume * 128) / (127 * 128));
                    sumR += (int32_t)(((int64_t)rv * vc->volume * 128) / (127 * 128));
                } else {
                    int32_t s0 = vc->s->samples[fi];
                    int32_t s1 = (fi + 1u < vc->s->frames) ? vc->s->samples[fi + 1u] : s0;
                    int32_t sv = s0 + asr16(mul32(s1 - s0, (int32_t)frac));
                    sumL += (int32_t)(((int64_t)sv * vc->volume * vc->gl) / (127 * 128));
                    sumR += (int32_t)(((int64_t)sv * vc->volume * vc->gr) / (127 * 128));
                }
                vc->p += vc->step;
            }
            if (m->music_render != NULL && m->mbuf != NULL) {
                sumL += (int32_t)(((int64_t)m->mbuf[2 * i] * m->music_volume) / 127);
                sumR += (int32_t)(((int64_t)m->mbuf[2 * i + 1] * m->music_volume) / 127);
            }
            out[2 * (f + i)] = (int16_t)clamp16(&sumL, &m->clipped);
            out[2 * (f + i) + 1] = (int16_t)clamp16(&sumR, &m->clipped);
        }
        f += piece;
    }
}

uint32_t mix_clipped(const mixer *m)
{
    return m == NULL ? 0 : m->clipped;
}
