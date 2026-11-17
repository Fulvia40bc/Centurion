#ifndef GIF_H
#define GIF_H

#include <stddef.h>
#include <stdint.h>

typedef struct { uint16_t w, h; uint8_t pal[256][3]; uint16_t ncolors; uint8_t *pixels; /* w*h, row-major */
                 uint16_t left, top;        /* image descriptor position */
                 uint16_t screen_w, screen_h; /* logical screen size (header bytes 6..9) */ } gif_image;

int  gif_decode(const uint8_t *data, size_t len, gif_image *out);  /* 0 ok, <0 error code */
void gif_free(gif_image *img);

#endif
