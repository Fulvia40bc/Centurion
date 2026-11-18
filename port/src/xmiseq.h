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
void     xmi_seq_free(xmi_seq *s);
uint32_t xmi_seq_loops(const xmi_seq *s);              /* loop jumps performed so far (for test output) */

#endif
