#ifndef MIXER_H
#define MIXER_H

#include <stddef.h>
#include <stdint.h>

#include "wav.h"

typedef struct mixer mixer;

mixer *mix_new(void);                                    /* output fixed at 44100 Hz stereo; NULL = no memory */
void   mix_free(mixer *m);
int    mix_play(mixer *m, const wav_sound *s, int volume, int pan);   /* -> voice 0..7, or -1 if all 8 are busy */
void   mix_stop(mixer *m, int voice);                    /* silent from the next rendered frame */
int    mix_playing(const mixer *m, int voice);           /* 1 while the voice still has samples to play */
void   mix_set_music(mixer *m, void (*render)(void *user, int16_t *out, int frames), void *user, int volume);
void   mix_render(mixer *m, int16_t *out, int frames);    /* out: frames*2 int16, interleaved L R */
uint32_t mix_clipped(const mixer *m);                    /* samples clamped since mix_new (for test output) */

#endif
