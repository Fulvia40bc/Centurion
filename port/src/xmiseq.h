#ifndef XMISEQ_H
#define XMISEQ_H

#include <stddef.h>
#include <stdint.h>

#include "xmi.h"

typedef void (*xmi_out_fn)(void *user, const uint8_t *msg, int len);     /* one MIDI channel message, 2 or 3 bytes */
typedef void (*xmi_trigger_fn)(void *user, int channel, int value);      /* controller 119 reached */

typedef struct xmi_seq xmi_seq;

xmi_seq *xmi_seq_new(const xmi_song *song, xmi_out_fn out, xmi_trigger_fn trig, void *user);  /* NULL = no memory */
void     xmi_seq_advance(xmi_seq *s, uint32_t ticks);  /* move on by `ticks` (120 per second), emitting what falls due */
int      xmi_seq_done(const xmi_seq *s);               /* 1 after FF 2F and the last pending note-off */
void     xmi_seq_stop(xmi_seq *s);                     /* note-off for every sounding note now; then done */

/* AIL sequence controls (TASK 22). pause: note-off every sounding note, forget them, CC64 0 where the sustain
   shadow is down; time does not advance while paused. rewind: pause if playing, then back to tick 0 (FOR slots
   and note queue empty, controller shadows cleared); the loop count and the volume are kept. */
void     xmi_seq_pause(xmi_seq *s);
void     xmi_seq_resume(xmi_seq *s);
void     xmi_seq_rewind(xmi_seq *s);
void     xmi_seq_set_loops(xmi_seq *s, int n);         /* FF 2F: 0 = rewind forever, n >= 1 = decrement, 0 -> done */
void     xmi_seq_set_volume(xmi_seq *s, int v);        /* 0..127, default 127: CC7 out as raw * v / 127, resent on change */

void     xmi_seq_free(xmi_seq *s);
uint32_t xmi_seq_loops(const xmi_seq *s);              /* loop jumps performed so far (for test output) */

#endif
