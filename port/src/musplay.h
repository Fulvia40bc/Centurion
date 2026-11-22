#ifndef MUSPLAY_H
#define MUSPLAY_H

/* The port's music player (TASK 22): one XMIDI song at a time through a backend (GM now: gm_synth.c; FM later).
   The caller drives time: mus_render() advances the sequencer one interval per 1/120 s of output and renders the
   backend in between. With a NULL backend everything still works, silently. */

#include <stddef.h>
#include <stdint.h>

#include "xmi.h"
#include "xmiseq.h"

typedef struct {
    void (*midi)(void *u, const uint8_t *msg, int len);     /* one channel message */
    void (*render)(void *u, int16_t *out, int frames);      /* interleaved stereo */
    void (*reset)(void *u);                                 /* all sound off (between songs) */
    void *u;
} mus_backend;

typedef struct musplay musplay;

enum { MUS_FREE = 1, MUS_DONE = 2, MUS_PLAYING = 4, MUS_PAUSED = 8 };

musplay *mus_new(const mus_backend *b, int rate);           /* NULL backend = silent */
void     mus_free(musplay *p);

/* sequence n of an XMI image (the 4.6 walk: top chunks FORM/CAT only, case-sensitive ids, big-endian sizes, else
   fail; a chunk whose type at +8 is not "XMID" (FORM XDIR) is skipped; a lone FORM XMID is the one sequence (n = 0);
   in CAT XMID the n-th sub-chunk of type XMID, 0-based). 0 ok, <0 not found / no EVNT / malformed. */
int      mus_probe(const uint8_t *xmi, size_t len, int n, xmi_song *out);   /* out is filled on success */
int      mus_install(musplay *p, xmi_song *song);           /* takes ownership of *song; status MUS_DONE */
int      mus_load(musplay *p, const uint8_t *xmi, size_t len, int n);      /* probe + install; 1 ok, 0 fail */

void     mus_start(musplay *p);      /* channel set-up, rewind, play: status MUS_PLAYING */
void     mus_pause(musplay *p);      /* only when playing: status MUS_PAUSED */
void     mus_resume(musplay *p);     /* only when paused: status MUS_PLAYING */
void     mus_end(musplay *p);        /* stop the notes: status MUS_DONE */
void     mus_volume(musplay *p, int v);   /* 0..127, default 127 */
void     mus_loops(musplay *p, int n);    /* the FF 2F count, default 1 */
int      mus_status(musplay *p);          /* MUS_FREE / MUS_DONE / MUS_PLAYING / MUS_PAUSED */

void     mus_render(void *p, int16_t *out, int frames);    /* the mixer's music callback */

#endif
