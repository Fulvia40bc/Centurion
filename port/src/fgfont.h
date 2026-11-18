#ifndef FGFONT_H
#define FGFONT_H

/* The game's three bitmap fonts (BLOCK05.FGF, COGNAC10.FGF, COGNAC19.FGF in ACIV.CT1, second directory block) and
   the Fastgraph/Fonts 2.04 calls the game uses: define (0x8DFEE), print (0x8E253), spacing (0x8E244), height of a
   string (0x8E1EB), baseline (0x8E23E). D = read from that code, see OVERVIEW §4.
   File: u8 bytes_per_row, u8 height; u8 width[96]; u8 rows[96]; 96 cells of bytes_per_row*height bytes, characters
   32..127. A glyph is rows[c] rows of (width[c]+7)/8 bytes, top row first, most significant bit leftmost. */

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t bpr, height;
    uint8_t width[96], rows[96];
    const uint8_t *cells;   /* points into the caller's data, which must outlive the font */
    int cell;               /* bytes per cell = bpr * height */
    int baseline;           /* blank rows under 'A' (0x8DFEE): the text sits this far above the glyphs' bottom */
} fg_font;

int  fgfont_load(const uint8_t *data, size_t len, fg_font *f);   /* 0 ok, -1 bad size */

/* Draw n bytes of s at the graphics cursor (*x, y = the text's baseline) into an 8-bit frame, as 0x8E253:
   characters below 32 or above 127 draw nothing; 0x7F + c sets the colour to c, 0x7F + '_' starts / ends an
   underline 2 rows under the baseline; each character advances x by its width + spacing. Pixels outside the frame are
   clipped. On return *x is the cursor after the text and *color the colour after any 0x7F changes. */
void fgfont_print(uint8_t *fb, int fb_w, int fb_h, const fg_font *f, int *x, int y, const char *s, int n, int spacing,
               uint8_t *color);
int  fgfont_width(const fg_font *f, const char *s, int n, int spacing);   /* the advance fgfont_print would make */
int  fgfont_height(const fg_font *f, const char *s, int n);      /* the tallest glyph's rows (0x8E1EB) */

#endif
