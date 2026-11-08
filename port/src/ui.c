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

int ui_text_width(const char *text) {
    int w = 0;
    for (const char *p = text; *p; p++) {
        int gw, yo;
        if (font_glyph((unsigned char)*p, &gw, &yo)) w += gw + 1;
    }
    return w ? w - 1 : 0;
}

void ui_text_centered(uint8_t *fb, int cx, int y, const char *text, uint8_t color) {
    ui_text(fb, cx - ui_text_width(text) / 2, y, text, color);
}

/* ---- menu engine (EXE 0x5614/0x54c8): menus are DS records in OVL0 ---- */
static const char *ds_str(const uint8_t *ovl0, uint16_t ptr, char *buf, int bufsz) {
    const uint8_t *p = ovl0 + 0xB400 + ptr;
    int i = 0;
    while (p[i] && i < bufsz - 1) { buf[i] = p[i]; i++; }
    buf[i] = 0;
    return buf;
}

void ui_menu_load(menu_t *m, const uint8_t *ovl0, uint16_t rec_ds) {
    const uint8_t *r = ovl0 + 0xB400 + rec_ds;
    uint16_t x0 = r[0] | (r[1] << 8), y0 = r[2] | (r[3] << 8);
    uint16_t x1 = r[4] | (r[5] << 8), y1 = r[6] | (r[7] << 8);
    uint16_t items = r[10] | (r[11] << 8), title = r[12] | (r[13] << 8);
    int count = r[14];
    m->x = x0; m->y = y0; m->w = x1 - x0 + 1; m->h = y1 - y0 + 1;
    m->count = count > 16 ? 16 : count;
    ds_str(ovl0, title, m->title, sizeof(m->title));
    for (int i = 0; i < m->count; i++) {
        const uint8_t *it = ovl0 + 0xB400 + items + 12 * i;
        uint16_t tp = it[0] | (it[1] << 8);
        uint16_t hk = it[2] | (it[3] << 8);
        ds_str(ovl0, tp, m->items[i].text, sizeof(m->items[i].text));
        m->items[i].hotkey = (char)(hk & 0xFF);
        m->items[i].x0 = (int16_t)(it[4] | (it[5] << 8));
        m->items[i].y0 = (int16_t)(it[6] | (it[7] << 8));
        m->items[i].x1 = (int16_t)(it[8] | (it[9] << 8));
        m->items[i].y1 = (int16_t)(it[10] | (it[11] << 8));
    }
}

static void ui_frame(uint8_t *fb, int x, int y, int w, int h) {
    ui_rect(fb, x, y, w, h, 26);
    ui_rect(fb, x, y, w, 2, 15);
    ui_rect(fb, x, y + 2, 2, h - 4, 8);
    ui_rect(fb, x + w - 2, y + 2, 2, h - 4, 8);
    ui_rect(fb, x, y + h - 2, w, 2, 0);
    ui_rect(fb, x + 3, y + 3, w - 6, 1, 8);
    ui_rect(fb, x + 3, y + h - 3, w - 6, 1, 8);
    ui_rect(fb, x + 3, y + 4, 1, h - 7, 8);
    ui_rect(fb, x + w - 4, y + 4, 1, h - 7, 8);
}

void ui_menu_draw(uint8_t *fb, const menu_t *m, int preselect) {
    ui_frame(fb, m->x, m->y, m->w, m->h);
    ui_text_centered(fb, m->x + m->w / 2, m->y + 4, m->title, 15);
    for (int i = 0; i < m->count; i++) {
        int ix = m->x + m->items[i].x0, iy = m->y + m->items[i].y0;
        ui_text(fb, ix, iy, m->items[i].text, 0);
        if (i == preselect)
            for (int y = iy; y < iy + 9; y++)
                for (int x = ix; x < m->x + m->items[i].x1; x++) {
                    uint8_t *p = &fb[y * SCREEN_W + x];
                    if (*p == 0) *p = 26; else if (*p == 26) *p = 0;
                }
    }
}

int ui_menu_run(const menu_t *m, const input_t *in, int *last_click) {
    int click = in->mouse_buttons & 1;
    if (click && !*last_click) {
        if (in->mouse_x < m->x || in->mouse_x > m->x + m->w ||
            in->mouse_y < m->y || in->mouse_y > m->y + m->h)
            return -1;
        for (int i = 0; i < m->count; i++) {
            int x0 = m->x + m->items[i].x0, y0 = m->y + m->items[i].y0;
            int x1 = m->x + m->items[i].x1, y1 = m->y + m->items[i].y1;
            if (in->mouse_x >= x0 && in->mouse_x <= x1 && in->mouse_y >= y0 && in->mouse_y <= y1)
                return i;
        }
    }
    for (int i = 0; i < m->count; i++) {
        unsigned char hk = (unsigned char)m->items[i].hotkey;
        if (hk > 0 && hk < 128 && in->key_down[hk])
            return i;
    }
    if (in->key_down[27])
        return -1;
    return -2;
}
