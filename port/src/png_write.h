#ifndef PNG_WRITE_H
#define PNG_WRITE_H

#include <stddef.h>
#include <stdint.h>

/* An 8-bit indexed PNG (colour type 3) with a 256-entry palette: the palette indices survive exactly, so a saved frame
   can be compared pixel for pixel with a capture of the original. Stored (uncompressed) deflate blocks: no zlib. */
int png_write_indexed(const char *path, const uint8_t *pixels, int w, int h, const uint8_t pal[256][3]);

#endif
