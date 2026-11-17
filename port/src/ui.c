/* UI primitives, text boxes and the menu engine of the EXE (segment 0823).
 * docs/MENU_NOTES.md (0x5614 / 0x56B6 / 0x585B / 0x57F7 / 0x54C8) and docs/TEXTBOX_NOTES.md (0x899C / 0x8C58 / 0x8BE4 /
 * 0x8C9F / 0x3134 / 0x59D4), with their verifier corrections. Font globals at run time: height [9B5C] = 8, gap
 * [9B5D] = 3 (line pitch 11), spacing [9B5E] = 1. The game always has a mouse (DS:00BA = FFFF), so the keyboard-only
 * paths (bullet sprite [C1D6], 0x5901) are not ported. */
#include "ui.h"
#include "assets.h"
#include "state.h"
#include <string.h>

#define LINE_H   11   /* [9B5C] + [9B5D] */
#define SPACING  1    /* [9B5E] */
#define BOX_FILL 26   /* [6346] at run time (7 in the static OVL0 image) */

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

/* 0x7E74 rect(x0, y0, x1, y1) in [9AFA]: inclusive, corners swapped if reversed */
static void rect7e74(uint8_t *fb, int x0, int y0, int x1, int y1, uint8_t c) {
    if (x1 < x0) { int t = x0; x0 = x1; x1 = t; }
    if (y1 < y0) { int t = y0; y0 = y1; y1 = t; }
    ui_rect(fb, x0, y0, x1 - x0 + 1, y1 - y0 + 1, c);
}

/* 0x925C line(x0, y0, x1, y1) in [9AFA], inclusive. Every call ported here is vertical or horizontal. */
static void line925c(uint8_t *fb, int x0, int y0, int x1, int y1, uint8_t c) {
    rect7e74(fb, x0, y0, x1, y1, c);
}

/* 0x8D56 glyph(ch, x, y): ink pixels only, colour [9B69] & 0x0F; returns the advance (width + [9B5E]) */
static int glyph8d56(uint8_t *fb, int ch, int x, int y, uint8_t color) {
    int w, yoff;
    const uint8_t *g = font_glyph(ch, &w, &yoff);   /* NULL below 0x20 and for 0x80..0xFF (signed test in 0x8A5D) */
    if (!g) return -1;
    for (int r = 0; r < 8; r++) {
        uint8_t bits = g[r];
        int dy = y + yoff + r;
        if (dy < 0 || dy >= SCREEN_H) continue;
        for (int i = 0; i < 8; i++)
            if (bits & (0x80 >> i)) {
                int dx = x + i;
                if (dx >= 0 && dx < SCREEN_W) fb[dy * SCREEN_W + dx] = color;
            }
    }
    return w + SPACING;
}

/* 0x8C58 width(str): sum of (width + [9B5E]) over chars >= 0x20 (signed) up to NUL or '\n'; NULL -> 0 */
int ui_text_advance(const char *text) {
    int t = 0;
    if (!text) return 0;
    for (const char *p = text; *p && *p != '\n'; p++) {
        int w, yo;
        if ((signed char)*p >= 0x20 && font_glyph((unsigned char)*p, &w, &yo)) t += w + SPACING;
    }
    return t;
}

/* 0x899C text(str, x, y) with the word [9B69] = attr: ink attr & 0x0F; bit 15 (byte 9B6A bit 7) = first paint each
   line's band rect(x - 1, yy + yoff[' '] - 2, x + 8C58(line), yy + 11 - 1) in colour (attr & 0xF0) >> 4 */
void ui_text_attr(uint8_t *fb, int x, int y, const char *text, uint16_t attr) {
    if (!text) return;
    if ((attr & 0x8000) && *text) {
        int sw, yoff0;
        if (!font_glyph(' ', &sw, &yoff0)) yoff0 = 0;   /* [[9B58]] = yoff of ' ' (1) */
        int yy = y;
        for (const char *p = text; p; p = strchr(p, '\n')) {
            if (*p == '\n') p++;
            int wl = ui_text_advance(p);
            rect7e74(fb, x - 1, yy + yoff0 - 2, wl + x, yy + LINE_H - 1, (uint8_t)((attr & 0xF0) >> 4));
            yy += LINE_H;
        }
    }
    int sx = x;
    for (const char *p = text; *p; p++) {
        if (*p == '\n') { sx = x; y += LINE_H; continue; }
        if ((signed char)*p < 0x20) continue;
        int adv = glyph8d56(fb, (unsigned char)*p, sx, y, (uint8_t)(attr & 0x0F));
        if (adv > 0) sx += adv;
    }
}

/* 0x899C, transparent (menus and boxes always write a word with high byte 0). The colour is used as given. */
void ui_text(uint8_t *fb, int x, int y, const char *text, uint8_t color) {
    if (!text) return;
    int sx = x;
    for (const char *p = text; *p; p++) {
        if (*p == '\n') { sx = x; y += LINE_H; continue; }
        if ((signed char)*p < 0x20) continue;
        int adv = glyph8d56(fb, (unsigned char)*p, sx, y, color);
        if (adv > 0) sx += adv;
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

/* 0x8BE4 centred_text(str, cx, y) with the word [9B69] = attr: each '\n' line drawn by 899C at cx - (8C58(line) >> 1)
   (shr: unsigned), y += [9B5C] + [9B5D] = 11; str NULL draws nothing. 899C gets the same word for every line, so
   bit 15 paints each line's band. */
void ui_text_block_attr(uint8_t *fb, int cx, int y, const char *text, uint16_t attr) {
    char line[256];
    for (const char *s = text; s; ) {
        const char *nl = strchr(s, '\n');
        size_t n = nl ? (size_t)(nl - s) : strlen(s);
        if (n >= sizeof(line)) n = sizeof(line) - 1;
        memcpy(line, s, n); line[n] = 0;                     /* 8BE4 writes a 0 over the '\n' while it draws */
        ui_text_attr(fb, cx - (int)((unsigned)ui_text_advance(line) >> 1), y, line, attr);
        y += LINE_H;
        s = nl ? nl + 1 : NULL;
    }
}

/* 0x8BE4 with [9B69] = color (high byte 0: transparent) */
void ui_text_block(uint8_t *fb, int cx, int y, const char *text, uint8_t color) {
    ui_text_block_attr(fb, cx, y, text, color);
}

/* 0x8BE4 for one line: x = cx - (8C58 advance >> 1) (= ui_text_block) */
void ui_text_centered(uint8_t *fb, int cx, int y, const char *text, uint8_t color) {
    ui_text_block(fb, cx, y, text, color);
}

/* ---- boxes ---- */

/* 0x3134 box(x0, y0, x1, y1, fill): all inclusive; the order decides the corners ((x0,y0+1) and (x1,y0+1) stay 8,
   (x0,y1-1) and (x1,y1-1) stay 8) */
void ui_box(uint8_t *fb, int x0, int y0, int x1, int y1, int fill) {
    if (fill != -1) rect7e74(fb, x0, y0, x1, y1, (uint8_t)fill);
    line925c(fb, x0, y0, x0, y1, 8);
    line925c(fb, x0 + 1, y0 + 1, x0 + 1, y1 - 1, 8);
    line925c(fb, x1, y0, x1, y1, 8);
    line925c(fb, x1 - 1, y0 + 1, x1 - 1, y1 - 1, 8);
    line925c(fb, x0 + 3, y0 + 3, x0 + 3, y1 - 3, 8);
    line925c(fb, x0 + 3, y1 - 3, x1 - 3, y1 - 3, 8);
    line925c(fb, x1 - 3, y0 + 3, x1 - 3, y1 - 3, 8);
    line925c(fb, x0 + 3, y0 + 3, x1 - 3, y0 + 3, 8);
    line925c(fb, x0, y1, x1, y1, 0);
    line925c(fb, x0 + 1, y1 - 1, x1 - 1, y1 - 1, 0);
    line925c(fb, x0, y0, x1, y0, 15);
    line925c(fb, x0 + 1, y0 + 1, x1 - 1, y0 + 1, 15);
}

void ui_frame(uint8_t *fb, int x, int y, int w, int h) {   /* the game's grey bevelled box: 0x3134 with fill [6346] */
    ui_box(fb, x, y, x + w - 1, y + h - 1, BOX_FILL);
}

static int cdiv2(int v) { return v / 2; }   /* cwd; sub ax, dx; sar ax, 1 = C division (toward 0) */

/* 0x8C9F text_rect(str, minw, lines0, cx, cy): n = lines0 + line count (a leading '\n' skipped, a trailing one counts
   an empty line), mw = max(minw, 8C58 of each line); returns (cx - mw/2, cy - 11n/2, cx + mw/2, cy + 11n/2) */
ui_box_t ui_text_rect(const char *s, int minw, int lines0, int cx, int cy) {
    int n = lines0, mw = minw;
    if (s && strlen(s)) {
        const char *p = s;
        do {
            if (*p == '\n') p++;
            int w = ui_text_advance(p);
            if (w > mw) mw = w;
            n++;
            p = strchr(p, '\n');
        } while (p);
    }
    int hw = cdiv2(mw), h = cdiv2(n * LINE_H);
    ui_box_t r = { cx - hw, cy - h, cx + hw, cy + h };
    return r;
}

/* The common body of 0x59D4 and of the battle report (OVL_BATTLE E48B): r = 8C9F(s, minw, lines0, 159, 99) grown by
   7, 3134(r, [6346]), [9B69] = 0, 8BE4(s, (X0 + X1) / 2, Y0 + 7). Returns the box. */
ui_box_t ui_text_box(uint8_t *fb, const char *s, int minw, int lines0) {
    ui_box_t r = ui_text_rect(s, minw, lines0, 0x9F, 0x63);
    r.x0 -= 7; r.y0 -= 7; r.x1 += 7; r.y1 += 7;
    ui_box(fb, r.x0, r.y0, r.x1, r.y1, BOX_FILL);
    ui_text_block(fb, cdiv2(r.x1 + r.x0), r.y0 + 7, s, 0);
    return r;
}

/* Resident overlay start-up, OVL0.OVL file 0x0105..0x0121 (bytes 83 3E BA 00 00 74 12 E8 .. 0B C0 75 05 A3 BA 00 EB 06
   C7 06 40 3E 2E 3E): if ([00BA]) { if (mouse_init()) [3E40] = 0x3E2E; else [00BA] = 0; }. The port always has a
   mouse, so mouse_init() succeeds. Idempotent: running it again on a DS image that already went through it changes
   nothing (nothing in the EXE writes [3E40]). */
void ui_mouse_init(void) {
    if (W(0x00BA)) setW(0x3E40, 0x3E2E);
}

/* The string at DS:[3E40] (pushed by 0x59D4 at 0x59EE / 0x5A6B). The static OVL0 image holds 3E16 "Hit any key to
   continue"; the start-up above sets 3E2E "Click to continue" (t10_in.bin). The start-up runs first, as in the
   original, so a DS image straight from state_init / state_load reads the same as a run-time one. */
const char *ui_click_text(void) {
    ui_mouse_init();
    return (const char *)&g_ds[W(0x3E40)];
}

/* 0x59D4 message_box(str), the drawing part: 8C9F(s, 8C58([3E40]), 1, 159, 99) grown by 7, box, s in black from
   Y0 + 7, then the click line in white centred on 159 at Y1 - 15. The original then saves the screen under the box
   first (691B), waits (35F9) and restores it (6973): the caller does that with the returned box (ui_copy_rect).
   NULL draws nothing (returns an empty box with x1 < x0); "" still shows a box. */
ui_box_t ui_message_draw(uint8_t *fb, const char *text) {
    ui_box_t none = { 0, 0, -1, -1 };
    if (!text) return none;
    const char *click = ui_click_text();
    ui_box_t r = ui_text_box(fb, text, ui_text_advance(click), 1);
    ui_text_block(fb, 0x9F, r.y1 - 0xF, click, 15);
    return r;
}

/* 0x691B save / 0x6973 restore (950E): copy the inclusive rect between two 320x200 buffers at the same coordinates */
void ui_copy_rect(uint8_t *dst, const uint8_t *src, ui_box_t r) {
    for (int y = r.y0; y <= r.y1; y++) {
        if (y < 0 || y >= SCREEN_H) continue;
        for (int x = r.x0; x <= r.x1; x++)
            if (x >= 0 && x < SCREEN_W) dst[y * SCREEN_W + x] = src[y * SCREEN_W + x];
    }
}

/* ---- menu engine (EXE 0x5614/0x54c8): menus are DS records in OVL0 ---- */
static const char *ds_str(const uint8_t *ovl0, uint16_t ptr, char *buf, int bufsz) {
    const uint8_t *p = ovl0 + 0xB400 + ptr;
    int i = 0;
    while (ptr && p[i] && i < bufsz - 1) { buf[i] = p[i]; i++; }   /* pointer 0 = no string (8BE4 draws nothing) */
    buf[i] = 0;
    return buf;
}

void ui_menu_load(menu_t *m, const uint8_t *ovl0, uint16_t rec_ds) {
    const uint8_t *r = ovl0 + 0xB400 + rec_ds;
    int16_t x0 = (int16_t)(r[0] | (r[1] << 8)), y0 = (int16_t)(r[2] | (r[3] << 8));
    int16_t x1 = (int16_t)(r[4] | (r[5] << 8)), y1 = (int16_t)(r[6] | (r[7] << 8));
    uint16_t items = r[10] | (r[11] << 8), title = r[12] | (r[13] << 8);
    int count = r[14];
    m->x = x0; m->y = y0; m->w = x1 - x0 + 1; m->h = y1 - y0 + 1;
    m->colour = (uint16_t)(r[8] | (r[9] << 8));
    m->single = r[15];
    m->enabled = (uint16_t)(r[16] | (r[17] << 8));
    m->count = count > 16 ? 16 : count;
    ds_str(ovl0, title, m->title, sizeof(m->title));
    for (int i = 0; i < m->count; i++) {
        const uint8_t *it = ovl0 + 0xB400 + items + 12 * i;
        uint16_t tp = it[0] | (it[1] << 8);
        ds_str(ovl0, tp, m->items[i].text, sizeof(m->items[i].text));
        m->items[i].hotkey = (char)it[2];
        m->items[i].disabled = it[3];
        m->items[i].x0 = (int16_t)(it[4] | (it[5] << 8));
        m->items[i].y0 = (int16_t)(it[6] | (it[7] << 8));
        m->items[i].x1 = (int16_t)(it[8] | (it[9] << 8));
        m->items[i].y1 = (int16_t)(it[10] | (it[11] << 8));
    }
}

static int menu_x1(const menu_t *m) { return m->x + m->w - 1; }
static int menu_y1(const menu_t *m) { return m->y + m->h - 1; }

/* 0x56B6 ornament and title (mouse mode): colour 8; with single = 0 one ornament per item (cap and base rows at the
   item's top y0 and bottom y1 - 1, lines at sx+2/4/6), else one for the column (first item's top to the last item's
   bottom); then the title in colour ^ 0x0F centred (8BE4) on (X0 + X1) / 2 at Y0 + 6 */
static void menu_ornament_title(uint8_t *fb, const menu_t *m) {
    for (int i = 0; i < m->count; i++) {
        int sx = m->items[i].x0 + m->x, yt = m->items[i].y0 + m->y, yb = m->items[i].y1 + m->y - 1;
        if (!m->single) {
            for (int k = 1; k < 4; k++) line925c(fb, sx + 2 * k, yt, sx + 2 * k, yb, 8);
            line925c(fb, sx + 1, yt, sx + 7, yt, 8);
            line925c(fb, sx + 1, yb, sx + 7, yb, 8);
        }
    }
    if (m->single && m->count > 0) {
        int sx = m->items[0].x0 + m->x, yt = m->items[0].y0 + m->y, yb = m->items[m->count - 1].y1 + m->y - 1;
        for (int k = 0; k < 3; k++) { sx += 2; line925c(fb, sx, yt, sx, yb, 8); }
        line925c(fb, sx - 5, yt, sx + 1, yt, 8);
        line925c(fb, sx - 5, yb, sx + 1, yb, 8);
    }
    ui_text_block_attr(fb, cdiv2(menu_x1(m) + m->x), m->y + 6, m->title, (uint16_t)(m->colour ^ 0x0F));   /* 57CF: xor al, 0Fh */
}

/* 0x5614 menu_draw(rec, mode): mode 2 = box 3134(X0, Y0, X1, Y1, [6346]); mode 1 = the province panel frame 0x30E6
   (only its fill rect(x0+4, y0+3, x0+0x92, y0+0x71) in [6346] is ported, not its four sprites [C0F6]); mode 0 = nothing.
   Items at (X0 + x0 + 11, Y0 + y0) with [9B69] = the whole record colour word (0x5673; the word 6 if disabled,
   0x566B); ornament and title only when enabled.
   preselect >= 0: the caller's 0x585B(rec, preselect, ON) (e.g. 0x1A31, 0x1B63, 0x7D21, 0x7D76). */
void ui_menu_draw_mode(uint8_t *fb, const menu_t *m, int mode, int preselect) {
    if (mode == 2) ui_box(fb, m->x, m->y, menu_x1(m), menu_y1(m), BOX_FILL);
    else if (mode == 1) rect7e74(fb, m->x + 4, m->y + 3, m->x + 0x92, m->y + 0x71, BOX_FILL);
    for (int i = 0; i < m->count; i++) {
        uint16_t c = m->items[i].disabled ? 6 : m->colour;
        ui_text_attr(fb, m->items[i].x0 + m->x + 0xB, m->items[i].y0 + m->y, m->items[i].text, c);
    }
    if (m->enabled) menu_ornament_title(fb, m);
    if (preselect >= 0) ui_menu_highlight(fb, m, preselect, 1);
}

void ui_menu_draw(uint8_t *fb, const menu_t *m, int preselect) {
    ui_menu_draw_mode(fb, m, 2, preselect);
}

/* 0x585B highlight(rec, index, on): index 0xFF = nothing; the item text again at (X0 + x0 + 11, Y0 + y0) with [9B69]
   = the word 0x000F (on, 0x58AA), else the record colour word (0x5898) / the word 6 if disabled (0x5890). With a mouse
   nothing else changes (the 5x3 marker rect at 0x58D8 is keyboard only). */
void ui_menu_highlight(uint8_t *fb, const menu_t *m, int index, int on) {
    if (index < 0 || index == 0xFF || index >= m->count) return;
    uint16_t c = m->items[index].disabled ? 6 : m->colour;
    if (on) c = 0x000F;
    ui_text_attr(fb, m->items[index].x0 + m->x + 0xB, m->items[index].y0 + m->y, m->items[index].text, c);
}

/* 0x55EA enable(rec): enabled = FFFF, redraw ornament and title (0x56B6) */
void ui_menu_enable(uint8_t *fb, menu_t *m) {
    m->enabled = 0xFFFF;
    menu_ornament_title(fb, m);
}

/* 0x55FF disable(rec): enabled = 0, then 0x57F7: if single, erase the ornament with rect(sx, yt, sx + 8, yb) in
   [6346]; the title again in colour 6. Items are not redrawn. */
void ui_menu_disable(uint8_t *fb, menu_t *m) {
    m->enabled = 0;
    if (m->single && m->count > 0) {
        int sx = m->items[0].x0 + m->x;
        rect7e74(fb, sx, m->items[0].y0 + m->y, sx + 8, m->items[m->count - 1].y1 + m->y - 1, BOX_FILL);
    }
    ui_text_block(fb, cdiv2(menu_x1(m) + m->x), m->y + 6, m->title, 6);
}

/* 0x8238 in_rect: signed, inclusive */
static int in_rect8238(int x, int y, int x0, int y0, int x1, int y1) {
    return x >= x0 && x <= x1 && y >= y0 && y <= y1;
}

/* 0x552B hit(xy, rec): outside the box -> 0x100 (-2); else the first item whose rect holds the box-relative point */
int ui_menu_hit(const menu_t *m, int x, int y) {
    if (!in_rect8238(x, y, m->x, m->y, menu_x1(m), menu_y1(m))) return -2;
    x -= m->x; y -= m->y;
    for (int i = 0; i < m->count; i++)
        if (in_rect8238(x, y, m->items[i].x0, m->items[i].y0, m->items[i].x1, m->items[i].y1)) return i;
    return -2;
}

/* 0x5593 hotkey(rec, key): toupper (9DDC), first item with that hotkey (disabled items too); else Esc -> 0xFF (-1),
   anything else 0x100 (-2) */
int ui_menu_hotkey(const menu_t *m, int key) {
    unsigned char k = (unsigned char)key;
    if (k >= 'a' && k <= 'z') k -= 0x20;
    for (int i = 0; i < m->count; i++)
        if ((unsigned char)m->items[i].hotkey == k) return i;
    return k == 0x1B ? -1 : -2;
}

/* 0x54C8 menu_run(rec, preselect) with a mouse, one poll of its loop: 0x35DB (wait until no button is down), then
   0x359B: any button down ([B2] != 0, 0x359E) -> [C07F] = 0 and no key is read: bit 0 (left, held or new) -> 0x552B
   at the mouse position; else 0x5593(rec, 0) (so items with hotkey 0 answer a right click). Only with no button down
   does 0x359B read a key (0x3603) -> 0x5593(rec, key). So no key is read while 0x35DB waits or while a button is held;
   in the original such a key waits in the BIOS buffer until the release, here it is read only if it is still down
   in the first poll without a button. Arrow keys never reach it (key_down only holds keys below 256). Preselect is
   ignored with a mouse. */
int ui_menu_run(const menu_t *m, const input_t *in, int *last_click) {
    int r = -2;
    if (*last_click) {                               /* 0x35DB */
        if (in->mouse_buttons) return -2;
        *last_click = 0;
    }
    if (in->mouse_buttons & 1) {                     /* 0x35C4 */
        r = ui_menu_hit(m, in->mouse_x, in->mouse_y);
    } else if (in->mouse_buttons) {                  /* 0x35CE / 0x35D8: [C07F] = 0 */
        r = ui_menu_hotkey(m, 0);
    } else {                                         /* 0x35A5..0x35BA */
        for (int k = 1; k < 256 && r == -2; k++)
            if (in->key_down[k]) r = ui_menu_hotkey(m, k);
    }
    if (r != -2) *last_click = in->mouse_buttons != 0;   /* the 0x35DB after the choice (0x5510) */
    return r;
}
