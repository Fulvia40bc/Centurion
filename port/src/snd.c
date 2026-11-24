#include "snd.h"

/* port glue (snd.h): forwards to the attached backend, else the old no-op stubs' results */

static const snd_ops *g_ops;

void snd_attach(const snd_ops *ops) { g_ops = ops; }
char snd_music_set(void) { return g_ops != NULL ? g_ops->music_set : 0; }
char snd_music_choice(void) { return g_ops != NULL ? g_ops->music_choice() : 's'; }
char snd_switch_music(char set) { return g_ops != NULL ? g_ops->music_switch(set) : set; }
void snd_start_sequence_paused(uint32_t h) { if (g_ops != NULL) g_ops->start_sequence_paused((int32_t)h); }

void snd_startup(void)  { if (g_ops != NULL) g_ops->startup(); }
void snd_shutdown(void) { if (g_ops != NULL) g_ops->shutdown(); }

int32_t snd_allocate_sample(void)   { return g_ops != NULL ? g_ops->allocate_sample() : 1; }
int32_t snd_allocate_sequence(void) { return g_ops != NULL ? g_ops->allocate_sequence() : 1; }

int snd_set_sample_file(uint32_t h, const uint8_t *img, size_t len)
{
    if (g_ops != NULL)
        return g_ops->set_sample_file((int32_t)h, img, len);
    return img != NULL ? 1 : 0;                       /* G: AIL fails without a file image */
}

void snd_set_sample_volume(uint32_t h, int v)      { if (g_ops != NULL) g_ops->set_sample_volume((int32_t)h, v); }
void snd_set_sample_loop_count(uint32_t h, int n)  { if (g_ops != NULL) g_ops->set_sample_loop_count((int32_t)h, n); }
void snd_start_sample(uint32_t h)                  { if (g_ops != NULL) g_ops->start_sample((int32_t)h); }
void snd_end_sample(uint32_t h)                    { if (g_ops != NULL) g_ops->end_sample((int32_t)h); }
int32_t snd_sample_status(uint32_t h)              { return g_ops != NULL ? g_ops->sample_status((int32_t)h) : 2; }

int32_t snd_active_sample_count(uint32_t dig)
{
    (void)dig;                                        /* the one digital driver */
    return g_ops != NULL ? g_ops->active_sample_count() : 0;
}

int snd_init_sequence(uint32_t h, const uint8_t *img, size_t len, int n)
{
    return g_ops != NULL ? g_ops->init_sequence((int32_t)h, img, len, n) : 1;
}

void snd_start_sequence(uint32_t h)                { if (g_ops != NULL) g_ops->start_sequence((int32_t)h); }
void snd_stop_sequence(uint32_t h)                 { if (g_ops != NULL) g_ops->stop_sequence((int32_t)h); }
void snd_resume_sequence(uint32_t h)               { if (g_ops != NULL) g_ops->resume_sequence((int32_t)h); }
void snd_end_sequence(uint32_t h)                  { if (g_ops != NULL) g_ops->end_sequence((int32_t)h); }
int32_t snd_sequence_status(uint32_t h)            { return g_ops != NULL ? g_ops->sequence_status((int32_t)h) : 2; }

void snd_set_sequence_volume(uint32_t h, int v, int ms)
{
    (void)ms;                                         /* always 0 in the game (no ramp) */
    if (g_ops != NULL)
        g_ops->set_sequence_volume((int32_t)h, v);
}

void snd_set_sequence_loop_count(uint32_t h, int n) { if (g_ops != NULL) g_ops->set_sequence_loop_count((int32_t)h, n); }
const char *snd_last_error(void) { return g_ops != NULL ? g_ops->last_error() : ""; }

const uint8_t *snd_music_image(const char *name, size_t *len)
{
    *len = 0;
    return g_ops != NULL ? g_ops->music_image(name, len) : NULL;
}
