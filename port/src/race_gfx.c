/* The chariot race's pictures (OVL0 section 4, disasm/OVL_RACE.asm), Claude 2026-11-12: section 4's drawing routines,
 * translated literally after docs/RACE_SCREEN_NOTES.md and RACE_DATA_NOTES.md. The logic, the timing and the dice are in
 * race.c. The EXE helpers are naval_gfx.c's (targets 67E4 / 680F with the descriptor DS:C0D0, 6887 / 6837 pages, 691B /
 * 6973 rects, 7E74 fill, 8BE4 text with the font block DS:9B56, 163B / 8E62 sprite sheets); here only what section 4 needs
 * on top of them:
 *   - the sheet values: naval_gfx.c's small handle << 12, so that "sheet + 14 * frame" works as in the original (race.h);
 *   - 13A8 / 8790: RACEEST.CBM, "picture slot 3" (DS:0250 + 8 * 3), expanded through its colour table and blitted opaque
 *     at the point DS:840E = (0, 130);
 *   - 1273 + 8E62 + 86A0: GALLERY1.CBM (the descriptor DS:C356) and CIRCUS.CBM (C2DE's local descriptor) drawn as sprites
 *     (hotspot, transparent nibble, colour table of the header, the target's clip);
 *   - 82DE putpixel, 979C line (through 82DE), 899C left-aligned text (8BE4's per-line rule with the line's own width);
 *   - DE64, the hand-written speckle plotter (VGA path DEC4: the word 5652h per set bit, MSB first, one row per bit);
 *   - the race's own drawing: CDCE racer card, D392 lap dolphins, D3C2 / D400 / D453 / D49A / D4EC gauges, C23D parade
 *     chariot, C299 brazier; the path buffer of CBE6 (calloc(368, 4) -> W(C308)).
 * The port is VGA only (nv_vga(): [9B26] is 0 in a DS built from OVL0.OVL).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "state.h"
#include "screen.h"
#include "assets.h"
#include "naval_int.h"
#define RACE_INTERNAL
#include "race.h"

static const char *dstr(uint16_t a) { return (const char *)&g_ds[a]; }
static int cx0(void) { return SW(0xC0DA); }
static int cy0(void) { return SW(0xC0DC); }
static int cx1(void) { return SW(0xC0DE); }
static int cy1(void) { return SW(0xC0E0); }

/* ---------------------------------------------------------------- archive, sheets, pictures */
/* 0F7B(0, &arch): archive 0 = "mrace" (DS:029E[0]) = MRACE.DIR / MRACE.DAT; 1 = open */
int rc_archive_0F7B(void) { return assets_load("MRACE.DIR", "MRACE.DAT") == 0; }

/* 163B(name, &arch, 0): naval_gfx.c's loader; the value kept in the DS is handle << 12 (0 = failure) */
uint16_t rc_sheet_163B(const char *name) {
    uint16_t h = nv_sheet_163B(name);
    if (h >= 16) { nv_sheet_19A0(h); fprintf(stderr, "race: no sheet slot below 16 for %s\n", name); return 0; }
    return (uint16_t)(h << 12);
}
void rc_sheet_19A0(uint16_t v) { if (v) nv_sheet_19A0(RC_SHEET(v)); }

typedef struct { uint8_t *px; int w, h, xo, yo, trans, lut; } pic_t;   /* nibbles of a CBM member */
static pic_t g_raceest, g_gallery1, g_circus;

static void pic_free(pic_t *p) { free(p->px); memset(p, 0, sizeof(*p)); }
/* a CBM member of the open archive (the name in lower case, as in the DS) */
static int pic_load(pic_t *p, const char *lower) {
    char up[16]; size_t i;
    pic_free(p);
    for (i = 0; lower[i] && i < sizeof(up) - 1; i++) up[i] = (char)(lower[i] >= 'a' && lower[i] <= 'z' ? lower[i] - 32 : lower[i]);
    up[i] = 0;
    int k = assets_find(up);
    if (k < 0) { fprintf(stderr, "race: %s missing\n", up); return 0; }
    size_t rl = 0;
    uint8_t *raw = assets_decode(k, &rl);
    uint8_t *px = assets_decode_cbm(k, &p->w, &p->h, &p->lut);
    if (!raw || !px || rl < 10) { free(raw); free(px); memset(p, 0, sizeof(*p)); fprintf(stderr, "race: %s unreadable\n", up); return 0; }
    p->xo = (int16_t)(raw[2] | raw[3] << 8); p->yo = (int16_t)(raw[4] | raw[5] << 8); p->trans = raw[8];
    free(raw);
    p->px = px;
    return 1;
}
/* 8E62 of a CBM descriptor: at (x - xhot, y - yhot), transparent nibble skipped, the colour table DS:60C0 + 16 * lut,
   clipped to the target's rect */
static void pic_draw(const pic_t *p, int x, int y) {
    if (!p->px) return;
    x -= p->xo; y -= p->yo;
    for (int r = 0; r < p->h; r++) {
        int Y = y + r;
        if (Y < cy0() || Y > cy1() || Y < 0 || Y >= SCREEN_H) continue;
        for (int c = 0; c < p->w; c++) {
            int X = x + c, n = p->px[r * p->w + c] & 15;
            if (n == p->trans || X < cx0() || X > cx1() || X < 0 || X >= SCREEN_W) continue;
            nv_fb[Y * SCREEN_W + X] = B(0x60C0 + 16 * p->lut + n);
        }
    }
}
/* 13A8(&arch, 3): picture slot 3 = DS:0250 + 8 * 3 -> "raceest.cbm" (descriptor C0B6); 1 = loaded */
int rc_picture_13A8(void) { return pic_load(&g_raceest, dstr(W(0x0250 + 8 * 3))); }
/* 1273("gallery1.cbm" DS:83FA, &arch, C356) */
int rc_gallery1_1273(void) { return pic_load(&g_gallery1, dstr(0x83FA)); }
void rc_gallery1_free_86A0(void) { pic_free(&g_gallery1); }
/* 1273("circus.cbm" DS:87AF, &arch, &local) */
int rc_circus_load(void) { return pic_load(&g_circus, dstr(0x87AF)); }
void rc_circus_draw(void) { pic_draw(&g_circus, 0, 0); }
void rc_circus_free(void) { pic_free(&g_circus); }

/* 8790(C0B6, DS:840E, C0D0): the slot-3 picture opaque at the point, into the current target (screen bounds only) */
void rc_blit_8790(void) {
    const pic_t *p = &g_raceest;
    if (!p->px) return;
    int x = SW(0x840E), y = SW(0x8410);
    for (int r = 0; r < p->h; r++) {
        int Y = y + r;
        if (Y < 0 || Y >= SCREEN_H) continue;
        for (int c = 0; c < p->w; c++) {
            int X = x + c;
            if (X >= 0 && X < SCREEN_W) nv_fb[Y * SCREEN_W + X] = B(0x60C0 + 16 * p->lut + (p->px[r * p->w + c] & 15));
        }
    }
}

/* 8E62(desc, x, y): a sheet frame (race.h's encoding) or the gallery1 picture */
void rc_sprite_8E62(uint16_t desc, int x, int y) {
    if (desc == RC_GALLERY1) { pic_draw(&g_gallery1, x, y); return; }
    if (RC_SHEET(desc) == 0) return;
    nv_sprite_8E62(RC_SHEET(desc), RC_FRAME(desc), x, y);
}

/* ---------------------------------------------------------------- pixels, lines, text */
/* 82DE(x, y): one pixel in [9AFA], clipped to the target's rect */
void rc_pixel_82DE(int x, int y) {
    if (x < cx0() || x > cx1() || y < cy0() || y > cy1() || x < 0 || y < 0 || x >= SCREEN_W || y >= SCREEN_H) return;
    nv_fb[y * SCREEN_W + x] = (uint8_t)W(0x9AFA);
}
/* 979C(x0, y0, x1, y1): a line of 82DE pixels (section 4 draws vertical ones only) */
void rc_line_979C(int x0, int y0, int x1, int y1) {
    int dx = x1 > x0 ? x1 - x0 : x0 - x1, dy = y1 > y0 ? y1 - y0 : y0 - y1;
    int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1, err = dx - dy;
    for (;;) {
        rc_pixel_82DE(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}
/* 899C(s, x, y), left-aligned, in [9B69] with the font block DS:9B56: 8BE4 places each line at cx - (8C58(line) >> 1), so
   a one-line string goes through it with cx = x + (width >> 1) */
void rc_text_899C(const char *s, int x, int y) {
    int t = 0;
    for (const char *p = s; *p && *p != '\n'; p++)
        if ((signed char)*p >= 0x20) t += B(W(0x9B56) + (uint8_t)*p - 0x20) + B(0x9B5E);
    nv_text_8BE4(s, x + (t >> 1), y);
}

/* DE64(pattern, y, x), the VGA path DEC4: rows from y down; a pattern starting above the clip top C0DC is shifted (16+ rows
   above: nothing), one ending past the clip bottom C0E0 masked by DS:8BB8 (a row at or below C0E0: nothing); then for every
   set bit, MSB first, the word 5652h at (x, row): pixel x = 52h, pixel x + 1 = 56h */
void rc_plot_DE64(uint16_t pat, int16_t y, int16_t x) {
    int16_t dy = y;
    int16_t c = (int16_t)(SW(0xC0DC) - dy);
    if (c >= 0) {                                                    /* de7b: js */
        if (c >= 0x10) return;
        pat = (uint16_t)(pat << c);
        y = (int16_t)(y + c);
    }
    int16_t b = (int16_t)(SW(0xC0E0) - dy);
    if (b <= 0) return;                                              /* de8e: jle */
    if (b < 0x10) pat &= W(0x8BB8 + 2 * b);
    int row = y;
    while (pat) {                                                    /* deca */
        int bit = pat & 0x8000;
        pat = (uint16_t)(pat << 1);
        if (bit && row >= 0 && row < SCREEN_H && x >= 0 && x + 1 < SCREEN_W) {
            nv_fb[row * SCREEN_W + x] = 0x52;
            nv_fb[row * SCREEN_W + x + 1] = 0x56;
        }
        row++;
    }
}

/* ---------------------------------------------------------------- the race's own drawing */
/* CDCE(i): racer card i at DS:8A8A[i]: box 3134 (100 x 80, fill [6346]); in the colour DS:87DE[i]: the home province
   (DS:5222), the strategy (DS:8804, not for the player), the chariot type (DS:87FE); the chariot seen from above (race
   sheet frame DS:87F6[type]) at (x + 33, y + 36) */
void rc_card_CDCE(int i) {
    int x = SW(0x8A8A + 4 * i), y = SW(0x8A8C + 4 * i);
    ui_box(nv_fb, x, y, x + 0x64, y + 0x50, SW(0x6346));
    int cx = x + 0x32;                                               /* di */
    int lh = B(0x9B5C) + B(0x9B5D) + 6;                              /* [bp-2] */
    setW(0x9B69, W(0x87DE + 2 * i));
    nv_text_8BE4(dstr(W(0x5222 + 2 * B(REC(i, R_PROV)))), cx, y + 6);
    if (i != 3) nv_text_8BE4(dstr(W(0x8804 + 2 * B(0xC368 + i))), cx, y + lh);
    nv_text_8BE4(dstr(W(0x87FE + 2 * B(REC(i, R_TYPE)))), cx, y - lh + 0x50);
    rc_sprite_8E62((uint16_t)(14 * B(0x87F6 + B(REC(i, R_TYPE))) + W(0xC2BC)), x + 0x21, y + 0x24);
}

/* D392: laps frame max(C350, 0) at (134, 185) */
void rc_laps_D392(void) {
    int8_t f = 0;
    if (SB(0xC350) > 0) f = SB(0xC350);
    if (!g_original_bugs && f > 3) f = 3;                            /* race.c bug 13: the sheet has frames 0..3 */
    rc_sprite_8E62((uint16_t)(14 * f + W(0xC306)), 0x86, 0xB9);
}
/* D453: the red needle at the safe cornering speed of the player's lane */
static void needle_D453(void) {
    if (B(0xC301) == 5) return;
    setW(0x9AFA, 4);
    int16_t x = SW(0xC340);
    int a = x < 0 ? -x : x;
    a >>= 4;
    if (x < 0) a = -a;
    if (a > 0xF) a = 0xF;
    uint8_t l = (uint8_t)((uint8_t)(a + 0xF) << 2);
    int16_t n = (int16_t)((int16_t)(l * 0x6E) / 0x82 + 0xD);
    rc_line_979C(n, 0xB7, n, 0xBA);
}
/* D400: the speed bar 110 * speed / 130 in colour 2, the rest in 7, then the needle */
void rc_speed_D400(void) {
    if (B(0xC301) == 5) return;
    setW(0x9AFA, 2);
    int16_t s = (int16_t)((int16_t)(0x6E * B(0xC2FE)) / 0x82);
    if (s < 0) s = 0;
    nv_fill_7E74(0xE, 0xB7, s + 0xE, 0xBA);
    setW(0x9AFA, 7);
    nv_fill_7E74(s + 0xE, 0xB7, 0x7C, 0xBA);
    needle_D453();
}
/* D49A: the horses bar 110 * horses / 60 in colour 4 */
void rc_horses_D49A(void) {
    if (B(0xC301) == 5) return;
    setW(0x9AFA, 4);
    int16_t h = (int16_t)((int16_t)(0x6E * SB(0xC304)) / 0x3C);
    if (h < 0) h = 0;
    nv_fill_7E74(0xC7, 0xB3, h + 0xC7, 0xB5);
    setW(0x9AFA, 7);
    nv_fill_7E74(h + 0xC7, 0xB3, 0x135, 0xB5);
}
/* D4EC: the chariot bar 110 * strength / 20 in colour 1 */
void rc_body_D4EC(void) {
    if (B(0xC301) == 5) return;
    setW(0x9AFA, 1);
    int16_t c = (int16_t)((int16_t)(0x6E * SB(0xC303)) / 0x14);
    if (c < 0) c = 0;
    nv_fill_7E74(0xC7, 0xBC, c + 0xC7, 0xBE);
    setW(0x9AFA, 7);
    nv_fill_7E74(c + 0xC7, 0xBC, 0x135, 0xBE);
}
/* D3C2(all): speed always, horses / chariot / laps when flagged; all != 0: the flags cleared (C32E too, never read) */
void rc_gauges_D3C2(int all) {
    rc_speed_D400();
    if (W(0xC330)) rc_horses_D49A();
    if (W(0xC36C)) rc_body_D4EC();
    if (W(0xC332)) rc_laps_D392();
    if (all) { setW(0xC36C, 0); setW(0xC330, 0); setW(0xC32E, 0); setW(0xC332, 0); }
}

/* C23D(type, x, f): race2 frame 5 + type at (x + 50, 142), frame 8 + f at (x, 130), frame 14 + (f & 1) at (x + 67, 158) */
void rc_chariot_C23D(int type, int x, int f) {
    rc_sprite_8E62((uint16_t)(0xE * type + W(0xC354) + 0x46), x + 0x32, 0x8E);
    rc_sprite_8E62((uint16_t)(0xE * f + W(0xC354) + 0x70), x, 0x82);
    rc_sprite_8E62((uint16_t)((uint8_t)(f & 1) * 0xE + W(0xC354) + 0xC4), x + 0x43, 0x9E);
}
/* C299: the brazier, race2 frame W(8418) at (292, 35); W(8418) = (W(8418) + 1) mod 5 */
void rc_brazier_C299(void) {
    rc_sprite_8E62((uint16_t)(0xE * W(0x8418) + W(0xC354)), 0x124, 0x23);
    setW(0x8418, (uint16_t)(W(0x8418) + 1));
    if (SW(0x8418) >= 5) setW(0x8418, 0);
}

/* ---------------------------------------------------------------- the path buffer */
/* CBE6: calloc(368, 4) -> W(C308) (a DS-near pointer in 1990; here a static buffer and a non-zero marker) */
static int16_t g_path[0x170][2];
void rc_path_alloc(void) { memset(g_path, 0, sizeof(g_path)); setW(0xC308, 0x7488); }
void rc_path_free(void) { }                    /* 74B7 free(W(C308)): the word itself keeps its value, as in 1990 */
void rc_path_set(uint16_t p, int16_t x, int16_t y) {
    if (p >= 0x170) { fprintf(stderr, "race: path point %u past the buffer\n", (unsigned)p); return; }
    g_path[p][0] = x; g_path[p][1] = y;
}
void rc_path_point(uint16_t p, int *x, int *y) {
    if (p >= 0x170) { *x = *y = -1; return; }
    *x = g_path[p][0]; *y = g_path[p][1];
}
