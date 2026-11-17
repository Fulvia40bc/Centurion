#ifndef FLC_H
#define FLC_H

#include <stddef.h>
#include <stdint.h>

typedef struct { uint16_t w, h, frames; uint32_t speed_ms; uint8_t pal[256][3]; uint8_t *pixels; /* w*h */
                 const uint8_t *data; size_t len; size_t pos; uint32_t index; } flc_anim;

int  flc_open(flc_anim *a, const uint8_t *data, size_t len);  /* reads the header, allocates pixels (zeroed), 0 ok */
int  flc_next(flc_anim *a);   /* decodes the next frame into pixels/pal; after the last frame it plays the ring frame
                                 and wraps to frame 1 (index counts 0..frames-1); 0 ok, <0 error */
void flc_close(flc_anim *a);

#endif
