#ifndef PALETTE_H
#define PALETTE_H

/* The VGA palette as the game sets it: 6 bits per component (the DAC), from a GIF's 8-bit colour table by a plain
   shift (Fastgraph/Image fg_gifpal, 0x878F7: lodsw; shr ax,2; and al,3Fh). The game's palette is PAL.PAL's table
   (start-up 0x11177 and the fade-in 0x3CFE4). For the screen the 6-bit values are widened back to 8 bits the way a
   VGA DAC's output is usually rendered: v << 2 | v >> 4 (G: DOSBox-X does the same). */

#include <stddef.h>
#include <stdint.h>

void pal_gif_to_dac(const uint8_t gif[][3], int ncolors, uint8_t dac[256][3]);   /* the rest of the 256 become 0 */
void pal_dac_to_rgb(const uint8_t dac[256][3], uint8_t rgb[256][3]);

#endif
