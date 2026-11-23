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

/* AIL-style plays: mix_start names the play with an id (voice + 8 * generation), so a reused voice's old id
   goes stale (mix_play bumps the generation too). Loops are read when the data runs out: 0 = start again at
   frame 0 forever, 1 = end, n > 1 = decrement and start again. A restart rewinds by the length, keeping the
   overshoot and the resampling fraction, and the run-out frame is mixed from the new position (no silent
   frame); mix_find_id/mix_active apply the run-out rule, so a count-1 play that ran out is stale everywhere. */
int    mix_start(mixer *m, const wav_sound *s, int volume, int loops);  /* -> id >= 0, or -1 if all 8 busy / no frames */
int    mix_active(const mixer *m, int id);              /* 1 while that play still has data; 0 ended/stopped/stale */
void   mix_halt(mixer *m, int id);                       /* silent from the next rendered frame; stale id: nothing */
void   mix_volume(mixer *m, int id, int volume);         /* 0..127 clamped, from the next rendered frame */
void   mix_loops(mixer *m, int id, int loops);           /* read when the data runs out */

void   mix_set_music(mixer *m, void (*render)(void *user, int16_t *out, int frames), void *user, int volume);
void   mix_music_volume(mixer *m, int volume);           /* the music input's gain, no re-allocation */
void   mix_render(mixer *m, int16_t *out, int frames);    /* out: frames*2 int16, interleaved L R */
uint32_t mix_clipped(const mixer *m);                    /* samples clamped since mix_new (for test output) */

#endif
