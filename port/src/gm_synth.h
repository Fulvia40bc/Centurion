#ifndef GM_SYNTH_H
#define GM_SYNTH_H

/* General MIDI music through FluidSynth + a SoundFont (OVERVIEW §7.2): the `g*` songs.
   The caller drives time: one xmi_seq_advance(seq, 1) per 120 Hz tick, then gm_synth_render() for that tick's samples.
   FluidSynth is LGPL-2.1 and is linked as a DLL (libfluidsynth-3.dll). */

#include <stdint.h>

#include "xmiseq.h"

typedef struct gm_synth gm_synth;

gm_synth *gm_synth_new(const char *sf2_path, int rate);  /* NULL on failure (message on stderr) */
void      gm_synth_midi(void *synth, const uint8_t *msg, int len);  /* an xmi_out_fn: one channel message */
void      gm_synth_render(gm_synth *g, int16_t *out, int frames);   /* interleaved stereo */
void      gm_synth_reset(gm_synth *g);   /* all sound off, controllers and programs reset (between songs) */
void      gm_synth_free(gm_synth *g);

/* Samples in 120 Hz tick n at `rate`: floor((n+1)*rate/120) - floor(n*rate/120), so no drift over a song. */
int       gm_tick_frames(uint64_t tick, int rate);

#endif
