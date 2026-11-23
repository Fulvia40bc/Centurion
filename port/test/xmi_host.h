#ifndef XMI_HOST_H
#define XMI_HOST_H

/* A copy of the game's own XMIDI interpreter (the host model of port/test/fm_dro_cmp.c, translated from AC_MAIN
   0x99F80..0x9A6A9 / 0x99610 / 0x99AA0), with the driver queue replaced by a recorder: every channel message is
   logged with the 120 Hz tick it went out in. Used by xmiseq_cmp_test.c to check that port/src/xmiseq.c emits
   exactly the same stream (bytes, order, tick). fm_dro_cmp.c keeps its own copy and is unchanged. */

#include <stddef.h>
#include <stdint.h>

typedef struct { uint8_t msg[256]; int len; int32_t tick; } xmi_host_rec;

typedef struct xmi_host xmi_host;

xmi_host *xmi_host_new(const uint8_t *evnt, size_t evnt_len);
void      xmi_host_free(xmi_host *h);

void      xmi_host_reset(xmi_host *h);          /* AIL_init_sequence / AIL_start_sequence's reset (0x99E60) */
void      xmi_host_start(xmi_host *h);          /* status 4 (playing) */
void      xmi_host_stop(xmi_host *h);           /* AIL_stop_sequence (0x99AA0): only when playing */
void      xmi_host_set_loops(xmi_host *h, int n);
void      xmi_host_set_volume(xmi_host *h, int v);
int       xmi_host_tick(xmi_host *h);           /* one 120 Hz interval; 0 = note queue overflow (no flush) */

size_t              xmi_host_count(const xmi_host *h);
const xmi_host_rec *xmi_host_recs(const xmi_host *h);
void                xmi_host_clear_recs(xmi_host *h);   /* drop the recording, keep playing */

#endif
int32_t xmi_host_tickno(const xmi_host *h);   /* the current 120 Hz tick (increments only while playing) */
