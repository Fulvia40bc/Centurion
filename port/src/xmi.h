#ifndef XMI_H
#define XMI_H

#include <stddef.h>
#include <stdint.h>

typedef struct { uint32_t tick; uint8_t status, d1, d2; uint8_t len; /* 1..3 bytes of MIDI used */
                 uint32_t dur; /* note duration in ticks on note-on events, 0 on all others */
                 uint8_t own;  /* 1 = the parser's own note-off (from a note-on's dur); a real 0x8n in the
                                  EVNT stream has own = 0 and is forwarded by the sequencer where it stands */ } xmi_event;
typedef struct { xmi_event *ev; uint32_t count; uint8_t timb[64][2]; int ntimb; /* TIMB: (patch, bank) pairs */ } xmi_song;

int  xmi_parse(const uint8_t *data, size_t len, xmi_song *out);   /* 0 ok, <0 error */
void xmi_free(xmi_song *s);

#endif
