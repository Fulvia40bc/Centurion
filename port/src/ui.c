#include "ui.h"
#include "assets.h"
#include <string.h>

void ui_rect(uint8_t *fb, int x, int y, int w, int h, uint8_t color) {
    for (int j = 0; j < h; j++) {
        int dy = y + j;
        if (dy < 0 || dy >= SCREEN_H) continue;
        for (int i = 0; i < w; i++) {
            int dx = x + i;
            if (dx >= 0 && dx < SCREEN_W)
                fb[dy * SCREEN_W + dx] = color;
        }
    }
}

void ui_text(uint8_t *fb, int x, int y, const char *text, uint8_t color) {
    for (const char *p = text; *p; p++) {
        int w, yoff;
        const uint8_t *g = font_glyph((unsigned char)*p, &w, &yoff);
        if (!g) continue;
        for (int r = 0; r < 8; r++) {
            uint8_t bits = g[r];
            int dy = y + yoff + r;
            if (dy < 0 || dy >= SCREEN_H) continue;
            for (int i = 0; i < 8; i++) {
                if (bits & (0x80 >> i)) {
                    int dx = x + i;
                    if (dx >= 0 && dx < SCREEN_W)
                        fb[dy * SCREEN_W + dx] = color;
                }
            }
        }
        x += w + 1;
    }
}

void ui_window(uint8_t *fb, int x, int y, int w, int h, const char *title) {
    ui_rect(fb, x, y, w, h, 0x0F);          /* frame/fill */
    ui_rect(fb, x + 1, y + 1, w - 2, h - 2, 0x00);  /* inner */
    if (title)
        ui_text(fb, x + 4, y + 2, title, 0x0F);
}

void ui_blit(uint8_t *dst, const uint8_t *src) {
    memcpy(dst, src, (size_t)SCREEN_W * SCREEN_H);
}
