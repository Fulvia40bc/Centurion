#include "fgfont.h"

int fgf_define(const uint8_t *data, size_t len, fg_font *f)
{
    int i, a, abpr, zero = 0;
    const uint8_t *c;

    if (data == NULL || len < 194)
        return -1;
    f->bpr = data[0];
    f->height = data[1];
    f->cell = f->bpr * f->height;
    if (f->cell == 0 || len != (size_t)194 + (size_t)96 * (size_t)f->cell)
        return -1;
    for (i = 0; i < 96; i++) {
        f->width[i] = data[2 + i];
        f->rows[i] = data[98 + i];
        if (((f->width[i] + 7) >> 3) * f->rows[i] > f->cell)
            return -1;
    }
    f->cells = data + 194;
    /* 0x8DFEE flips every glyph to bottom row first, then counts the zero bytes at the start of 'A' (code 65) and
       divides by A's bytes per row: the blank rows at the bottom of 'A' */
    a = 'A' - 32;
    abpr = (f->width[a] + 7) >> 3;
    c = f->cells + a * f->cell;
    if (abpr == 0) {
        f->baseline = 0;
        return 0;
    }
    for (i = f->rows[a] - 1; i >= 0; i--) {
        int k, blank = 1;
        for (k = 0; k < abpr; k++)
            if (c[i * abpr + k] != 0)
                blank = 0;
        if (!blank)
            break;
        zero += abpr;
    }
    f->baseline = zero / abpr;
    return 0;
}

static void put(uint8_t *fb, int fb_w, int fb_h, int x, int y, uint8_t color)
{
    if (x >= 0 && y >= 0 && x < fb_w && y < fb_h)
        fb[y * fb_w + x] = color;
}

void fgf_print(uint8_t *fb, int fb_w, int fb_h, const fg_font *f, int *x, int y, const char *s, int n, int spacing,
               uint8_t *color)
{
    int i, ul = 0, ul_x = 0, bottom = y + f->baseline;   /* the glyphs' bottom row (0x8E29A: y += baseline) */

    for (i = 0; i < n; i++) {
        uint8_t ch = (uint8_t)s[i];
        if (ch == 0x7F) {
            if (++i >= n)
                break;
            if ((uint8_t)s[i] == '_') {
                if (ul) {            /* fg_rect from the start to the last column of the previous character */
                    int xx;
                    for (xx = ul_x; xx <= *x - spacing - 1; xx++)
                        put(fb, fb_w, fb_h, xx, y + 2, *color);
                    ul = 0;
                } else {
                    ul_x = *x;
                    ul = 1;
                }
            } else {
                *color = (uint8_t)s[i];
            }
            continue;
        }
        if (ch < 32 || ch > 127)
            continue;
        {
            int c = ch - 32, w = f->width[c], rows = f->rows[c], cb = (w + 7) >> 3, r, b;
            const uint8_t *g = f->cells + c * f->cell;
            for (r = 0; r < rows; r++)         /* row r of the file is rows-1-r rows above the bottom */
                for (b = 0; b < cb * 8; b++)
                    if (g[r * cb + (b >> 3)] & (0x80 >> (b & 7)))
                        put(fb, fb_w, fb_h, *x + b, bottom - (rows - 1 - r), *color);
            *x += w + spacing;
        }
    }
}

int fgf_width(const fg_font *f, const char *s, int n, int spacing)
{
    int i, w = 0;

    for (i = 0; i < n; i++) {
        uint8_t ch = (uint8_t)s[i];
        if (ch == 0x7F) {
            i++;
            continue;
        }
        if (ch >= 32 && ch <= 127)
            w += f->width[ch - 32] + spacing;
    }
    return w;
}

int fgf_string_height(const fg_font *f, const char *s, int n)
{
    int i, h = 0;

    for (i = 0; i < n; i++) {
        uint8_t ch = (uint8_t)s[i];
        if (ch == 0x7F) {
            i++;
            continue;
        }
        if (ch >= 32 && ch <= 127 && f->rows[ch - 32] > h)
            h = f->rows[ch - 32];
    }
    return h;
}
