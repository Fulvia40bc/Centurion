#ifndef XMI_H
#define XMI_H

#include <stddef.h>
#include <stdint.h>

typedef struct { uint32_t tick; uint8_t status, d1, d2; uint8_t len; /* 1..3 bytes of MIDI used */ } xmi_event;
typedef struct { xmi_event *ev; uint32_t count; uint8_t timb[64][2]; int ntimb; /* TIMB: (patch, bank) pairs */ } xmi_song;

int  xmi_parse(const uint8_t *data, size_t len, xmi_song *out);   /* 0 ok, <0 error */
void xmi_free(xmi_song *s);

#endif
