/* The naval battle's pictures (OVL0 section 5, disasm/OVL_NAVAL.asm), Claude 2026-11-09: the EXE drawing helpers the
 * section calls and the section's own drawing routines, translated literally (docs/NAVAL_SCREEN_NOTES.md,
 * NAVAL_DATA_NOTES.md). The logic, the timing and the dice are in naval.c.
 *
 * EXE helpers (segment 0823): 67B6 / 67E4 / 680F / 6837 / 6887 (targets and the page flip, VGA path), 691B / 6973
 * (save / restore a rect screen <-> work buffer), 7E74 (filled rect), 7ECA (hline), 8E62 (sprite), 899C / 8BE4 / 8C58
 * (text, centred, width) and 8A9E / 8AC8 / 8AF2 / 8B2C (numbers), 817C / 81EA / 8238 (rects), 0F7B / 0F0C / 11C4 /
 * 163B / 19A0 (archive, picture, member, sprite sheets), 01B6 (DAC).
 *   - The current target is the 9-word descriptor DS:C0D0, copied from DS:9AB6 (the screen, g_scr) or DS:C1DA (the work
 *     buffer, g_wrk); its words +A..+10 (C0DA x0, C0DC y0, C0DE x1, C0E0 y1) clip 7E74 (when [9B24] != 0), 7ECA, 8E62 and
 *     the glyphs of 899C (drawn by the sprite blitter). The naval code sets the clip bottom of all three descriptors at
 *     once: 185 while the sea is drawn, 199 for the panel and the boxes.
 *   - Text: the font is the 10-byte block DS:9B56 (widths, yoffs, glyphs, height, gap, spacing), copied from DS:A05C (the
 *     normal font) or DS:A3B4 (the small 6-pixel panel font, NAVAL_SCREEN_NOTES 2.1). The port reads the block's tables
 *     from the DS, so both fonts are drawn by the same code (glyph c = height bytes at glyphs + height * (c - 0x20),
 *     bit 7 left; ink [9B69] & 15 through the colour table DS:60C0). The message boxes go through screen.c (the port's
 *     normal font), as everywhere else.
 *   - Sprite sheets: the original keeps far pointers to 14-byte descriptors and computes desc = handle + 14 * frame. The
 *     port keeps the sheets here and stores a small handle (1..16) in the DS words (C3A2, C3A6, ...), so the original's
 *     "handle == 0" tests still work; a frame is (handle, frame number). OARS2's "- 0x310" is frame - 56.
 *   - The port is VGA only: [9B26] is 2 in every DOSBox image but 0 in a DS built from OVL0.OVL (no video detection in the
 *     port), so the VGA paths are taken whatever it holds (nv_vga()).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "state.h"
#include "screen.h"
#include "assets.h"
#include "platform.h"
#include "naval_int.h"

uint8_t *nv_fb = g_scr;

int nv_vga(void) { return 1; }
static const char *dstr(uint16_t a) { return (const char *)&g_ds[a]; }
static void present(void) { if (!screen_headless()) screen_present(); }

/* ---------------------------------------------------------------- targets, pages (EXE, VGA path) */
static void desc_copy(uint16_t from) { memmove(&g_ds[0xC0D0], &g_ds[from], 18); }
void nv_target_work_67E4(void) { desc_copy(0xC1DA); nv_fb = g_wrk; }
void nv_target_screen_680F(void) { desc_copy(0x9AB6); nv_fb = g_scr; }
/* 6887: [4F96] ^= 1; target = work buffer; the whole work buffer to the screen (0494) */
void nv_flip_6887(void) {
    B(0x4F96) ^= 1;
    desc_copy(0xC1DA); nv_fb = g_wrk;
    memcpy(g_scr, g_wrk, sizeof(g_scr));
    present();
}
/* 6837: [4F96] == 0 -> 6887; [4F96] = 0; target = the screen */
void nv_page_6837(void) {
    if (W(0x4F96) == 0) nv_flip_6887();
    setW(0x4F96, 0);
    desc_copy(0x9AB6); nv_fb = g_scr;
}
static ui_box_t box_of(int x0, int y0, int x1, int y1) { ui_box_t r = { x0, y0, x1, y1 }; return r; }
void nv_save_691B(int x0, int y0, int x1, int y1) { ui_copy_rect(g_wrk, g_scr, box_of(x0, y0, x1, y1)); }
void nv_restore_6973(int x0, int y0, int x1, int y1) { ui_copy_rect(g_scr, g_wrk, box_of(x0, y0, x1, y1)); present(); }
/* 67B6 (VGA, or [4F96] == 1): 691B(0, 0, 319, 199), the screen into the work buffer */
void nv_save_67B6(void) { nv_save_691B(0, 0, 0x13F, 0xC7); }

void nv_clip(uint16_t y1) { setW(0xC0E0, y1); setW(0xC1EA, y1); setW(0x9AC6, y1); }
static int cx0(void) { return SW(0xC0DA); }
static int cy0(void) { return SW(0xC0DC); }
static int cx1(void) { return SW(0xC0DE); }
static int cy1(void) { return SW(0xC0E0); }

/* 7E74(x0, y0, x1, y1) in [9AFA]: corners sorted, x clipped to C0DA..C0DE, y to C0DC..C0E0 (when [9B24] != 0) */
void nv_fill_7E74(int x0, int y0, int x1, int y1) {
    if (x1 < x0) { int t = x0; x0 = x1; x1 = t; }
    if (y1 < y0) { int t = y0; y0 = y1; y1 = t; }
    if (x0 < cx0()) x0 = cx0();
    if (x1 > cx1()) x1 = cx1();
    if (W(0x9B24)) { if (y0 < cy0()) y0 = cy0(); if (y1 > cy1()) y1 = cy1(); }
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > SCREEN_W - 1) x1 = SCREEN_W - 1;
    if (y1 > SCREEN_H - 1) y1 = SCREEN_H - 1;
    for (int y = y0; y <= y1 && x0 <= x1; y++) memset(nv_fb + y * SCREEN_W + x0, (uint8_t)W(0x9AFA), (size_t)(x1 - x0 + 1));
}
/* 7ECA(x0, y, x1): a horizontal line in [9AFA], nothing when y is outside the clip */
void nv_hline_7ECA(int x0, int y, int x1) {
    if (y < cy0() || y > cy1()) return;
    nv_fill_7E74(x0, y, x1, y);
}

/* 817C(out, a, b): intersection into out (NULL -> DS:9AE8); -1 when not empty */
int nv_rect_817C(int16_t *out, const int16_t *a, const int16_t *b) {
    int16_t r[4];
    r[0] = a[0] >= b[0] ? a[0] : b[0];
    r[1] = a[1] >= b[1] ? a[1] : b[1];
    r[2] = a[2] <= b[2] ? a[2] : b[2];
    r[3] = a[3] <= b[3] ? a[3] : b[3];
    if (out) memcpy(out, r, sizeof(r));
    else for (int i = 0; i < 4; i++) setW((uint16_t)(0x9AE8 + 2 * i), (uint16_t)r[i]);
    return (r[3] >= r[1] && r[2] >= r[0]) ? -1 : 0;
}
/* 81EA(out, a, b): union (min x0 / y0, max x1 / y1) */
void nv_rect_81EA(int16_t *out, const int16_t *a, const int16_t *b) {
    out[0] = a[0] <= b[0] ? a[0] : b[0];
    out[1] = a[1] <= b[1] ? a[1] : b[1];
    out[2] = a[2] >= b[2] ? a[2] : b[2];
    out[3] = a[3] >= b[3] ? a[3] : b[3];
}
/* 8238(x, y, rect): inclusive, signed */
int nv_inrect_8238(int x, int y, const int16_t *r) {
    return (x >= r[0] && x <= r[2] && y >= r[1] && y <= r[3]) ? -1 : 0;
}

/* ---------------------------------------------------------------- text (899C with the font block DS:9B56) */
void nv_font(uint16_t block) { memmove(&g_ds[0x9B56], &g_ds[block], 10); }
static int adv_8C58(const char *s) {
    int t = 0;
    for (; s && *s && *s != '\n'; s++)
        if ((signed char)*s >= 0x20) t += B(W(0x9B56) + (uint8_t)*s - 0x20) + B(0x9B5E);
    return t;
}
static void glyph(int c, int x, int y, uint8_t ink) {
    int h = B(0x9B5C);
    uint16_t g = (uint16_t)(W(0x9B5A) + h * (c - 0x20));
    y += B(W(0x9B58) + c - 0x20);
    for (int r = 0; r < h; r++) {
        int Y = y + r;
        if (Y < cy0() || Y > cy1() || Y < 0 || Y >= SCREEN_H) continue;
        uint8_t bits = B(g + r);
        for (int i = 0; i < 8; i++) {
            int X = x + i;
            if ((bits & (0x80 >> i)) && X >= cx0() && X <= cx1() && X >= 0 && X < SCREEN_W) nv_fb[Y * SCREEN_W + X] = ink;
        }
    }
}
/* 899C(s, x, y) in [9B69] (transparent; the band bit is never set here) */
static void text_899C(const char *s, int x, int y) {
    uint16_t a = W(0x9B69);
    uint8_t ink = B(0x60C0 + 16 * ((a >> 8) & 0x3F) + (a & 0x0F));
    int sx = x;
    for (; *s; s++) {
        if (*s == '\n') { sx = x; y += B(0x9B5C) + B(0x9B5D); continue; }
        if ((signed char)*s < 0x20) continue;
        glyph((uint8_t)*s, sx, y, ink);
        sx += B(W(0x9B56) + (uint8_t)*s - 0x20) + B(0x9B5E);
    }
}
/* 8BE4(s, cx, y): each line at cx - (8C58(line) >> 1), lines height + gap apart */
void nv_text_8BE4(const char *s, int cx, int y) {
    char line[256];
    while (s) {
        const char *nl = strchr(s, '\n');
        size_t n = nl ? (size_t)(nl - s) : strlen(s);
        if (n >= sizeof(line)) n = sizeof(line) - 1;
        memcpy(line, s, n); line[n] = 0;
        text_899C(line, cx - (int)((unsigned)adv_8C58(line) >> 1), y);
        y += B(0x9B5C) + B(0x9B5D);
        s = nl ? nl + 1 : NULL;
    }
}
/* 8A9E(long v, x, y) / 8AC8 (right-aligned at x: 899C(s, x - 8C58(s), y)) */
void nv_num_8A9E(uint32_t v, int x, int y) { char b[16]; snprintf(b, sizeof(b), "%lu", (unsigned long)v); text_899C(b, x, y); }
void nv_num_8AC8(uint32_t v, int x, int y) { char b[16]; snprintf(b, sizeof(b), "%lu", (unsigned long)v); text_899C(b, x - adv_8C58(b), y); }
/* 8AF2(v, x, y, w): 7E74(x, y + Y0, x + w, y + Y0 + [9B5C]) then 8A9E; Y0 = yoff of ' '. 8B2C: the rect left of x, 8AC8 */
void nv_num_8AF2(uint32_t v, int x, int y, int w) {
    int y0 = B(W(0x9B58));
    nv_fill_7E74(x, y + y0, x + w, y + y0 + B(0x9B5C));
    nv_num_8A9E(v, x, y);
}
void nv_num_8B2C(uint32_t v, int x, int y, int w) {
    int y0 = B(W(0x9B58));
    nv_fill_7E74(x - w, y + y0, x, y + y0 + B(0x9B5C));
    nv_num_8AC8(v, x, y);
}

/* ---------------------------------------------------------------- archives and sprite sheets */
typedef struct { int w, xhot, yhot, bpr, rows, trans, lut; uint32_t off; } nspr_t;
typedef struct { nspr_t *e; int n; uint8_t *dat; size_t ndat; int used; } nsheet_t;
static nsheet_t g_sh[16];

/* 0F7B(n, &arch): 8 = MNAVAL, 9 = NAVL (NAVAL_DATA_NOTES 1); one archive is open at a time in the port (assets.c) */
int nv_archive_0F7B(int n) {
    if (n == 8) return assets_load("MNAVAL.DIR", "MNAVAL.DAT") == 0;
    if (n == 9) return assets_load("NAVL.DIR", "NAVL.DAT") == 0;
    return 0;
}
static uint8_t *member(const char *name, size_t *len) {
    int k = assets_find(name);
    return k < 0 ? NULL : assets_decode(k, len);
}
/* 0F0C("galleys.lzw", ...): the raw 320x200 picture into the work buffer */
int nv_galleys_0F0C(void) {
    size_t n = 0;
    uint8_t *p = member(dstr(0x97CC), &n);
    if (!p) return 0;
    memcpy(g_wrk, p, n < sizeof(g_wrk) ? n : sizeof(g_wrk));
    free(p);
    return 1;
}
/* 11C4("collide.lzw"): the member's bytes */
int nv_collide_11C4(int8_t *dst, int n) {
    size_t len = 0;
    uint8_t *p = member(dstr(0x97FB), &len);
    if (!p) return 0;
    memset(dst, 0, (size_t)n);
    memcpy(dst, p, len < (size_t)n ? len : (size_t)n);
    free(p);
    return 1;
}
/* 163B(name, &arch, 0): <name>HDR.LZW (14-byte descriptors) + <name>DAT.LZW (4-bit pixels; offsets count from 2 bytes
   before the data), no nibble remap */
uint16_t nv_sheet_163B(const char *name) {
    int k = 0;
    while (k < 16 && g_sh[k].used) k++;
    if (k == 16) { fprintf(stderr, "naval: no free sheet slot for %s\n", name); return 0; }
    char hn[24], dn[24];
    snprintf(hn, sizeof(hn), "%s%s", name, dstr(0x0284));            /* "HDR.LZW" */
    snprintf(dn, sizeof(dn), "%s%s", name, dstr(0x028C));            /* "DAT.LZW" */
    size_t hl = 0;
    nsheet_t *s = &g_sh[k];
    uint8_t *h = member(hn, &hl);
    s->dat = member(dn, &s->ndat);
    if (!h || !s->dat || hl < 14) { free(h); free(s->dat); memset(s, 0, sizeof(*s)); fprintf(stderr, "naval: sheet %s missing\n", name); return 0; }
    s->n = (int)(hl / 14);
    s->e = calloc((size_t)s->n, sizeof(nspr_t));
    for (int i = 0; i < s->n; i++) {
        const uint8_t *p = h + 14 * i;
        s->e[i].w = p[0] | p[1] << 8;
        s->e[i].xhot = (int16_t)(p[2] | p[3] << 8);
        s->e[i].yhot = (int16_t)(p[4] | p[5] << 8);
        s->e[i].bpr = p[6]; s->e[i].rows = p[7]; s->e[i].trans = p[8]; s->e[i].lut = p[9];
        s->e[i].off = (uint32_t)(p[10] | p[11] << 8 | p[12] << 16 | (uint32_t)p[13] << 24);
    }
    free(h);
    s->used = 1;
    return (uint16_t)(k + 1);
}
void nv_sheet_19A0(uint16_t h) {
    if (h < 1 || h > 16) return;
    nsheet_t *s = &g_sh[h - 1];
    free(s->e); free(s->dat);
    memset(s, 0, sizeof(*s));
}
static const nspr_t *spr(uint16_t h, int frame) {
    if (h < 1 || h > 16 || !g_sh[h - 1].used || frame < 0 || frame >= g_sh[h - 1].n) return NULL;
    return &g_sh[h - 1].e[frame];
}
int nv_desc(uint16_t h, int frame, int *w, int *xhot, int *yhot, int *rows) {
    const nspr_t *e = spr(h, frame);
    if (!e) { *w = *xhot = *yhot = *rows = 0; return 0; }
    *w = e->w; *xhot = e->xhot; *yhot = e->yhot; *rows = e->rows;
    return 1;
}
/* 8E62(desc, x, y): at (x - xhot, y - yhot), clipped to the target's rect; nibble -> DS:60C0 + 16 * lut */
void nv_sprite_8E62(uint16_t h, int frame, int x, int y) {
    const nspr_t *e = spr(h, frame);
    if (!e || e->off < 2) return;
    const nsheet_t *s = &g_sh[h - 1];
    x -= e->xhot; y -= e->yhot;
    for (int r = 0; r < e->rows; r++) {
        int Y = y + r;
        if (Y < cy0() || Y > cy1() || Y < 0 || Y >= SCREEN_H) continue;
        for (int c = 0; c < e->w; c++) {
            int X = x + c;
            size_t i = e->off - 2 + (size_t)r * e->bpr + c / 2;
            if (X < cx0() || X > cx1() || X < 0 || X >= SCREEN_W || i >= s->ndat) continue;
            uint8_t n = (c & 1) ? (s->dat[i] & 15) : (s->dat[i] >> 4);
            if (n != e->trans) nv_fb[Y * SCREEN_W + X] = B(0x60C0 + 16 * e->lut + n);
        }
    }
}

/* ---------------------------------------------------------------- the ships */
/* oars of side s: frame 93E6[h] + C390[s] of OARS1 (h < 8) or OARS2 (h >= 8, desc - 0x310 = frame - 56) */
static void oars(int s, int x, int y) {
    int h = SB(HEAD(s));
    int k = SB(OARF(s)) + SB(0x93E6 + h);
    if (h < 8) nv_sprite_8E62(W(H_OARS1), k, x, y);
    else nv_sprite_8E62(W(H_OARS2), k - 56, x, y);
}
/* B8E3(s): one ship: oars, hull frame = heading, flames (CD4B) while burning */
void nv_ship_B8E3(int s) {
    int x = SW(PX(s)), y = SW(PY(s));
    oars(s, x, y);
    nv_sprite_8E62(W(SHEET(s)), SB(HEAD(s)), x, y);
    if (B(FIRE(s))) nv_fire_CD4B(s);
}
/* B96A: the ship behind (C384 ^ 1) and the one in front: oars(b), oars(f), hull(b), flames(b), hull(f), flames(f) */
void nv_ships_B96A(void) {
    int f = W(RAMMER), b = f ^ 1;
    oars(b, SW(PX(b)), SW(PY(b)));
    oars(f, SW(PX(f)), SW(PY(f)));
    nv_sprite_8E62(W(SHEET(b)), SB(HEAD(b)), SW(PX(b)), SW(PY(b)));
    if (B(FIRE(b))) nv_fire_CD4B(b);
    nv_sprite_8E62(W(SHEET(f)), SB(HEAD(f)), SW(PX(f)), SW(PY(f)));
    if (B(FIRE(f))) nv_fire_CD4B(f);
}
/* CD4B(s): the burn counter runs down; every 16th frame the hull loses 1; flame frame 0x30 + (0..2) at the ship.
   1990 bug 9.1 (NAVAL_SCREEN_NOTES): the sinking animation D3F2 draws the wreck with B8E3 after the duel, and this
   turned a sunk hull 0 into -1 (the fleet battle tests == 0 and was skipped: "You retreat to safety" after sinking the
   enemy flagship), or sank a burning survivor after the fact. Fixed: the flames burn only while the duel runs. */
void nv_fire_CD4B(int s) {
    B(FIRE(s))--;
    if ((B(FIRE(s)) & 0x0F) == 0 && (g_original_bugs || nv_burn_on)) B(HULL(s))--;
    B(FIREANIM(s))++;
    if (SB(FIREANIM(s)) >= 3) B(FIREANIM(s)) = 0;
    nv_sprite_8E62(W(H_BOMBS), (int8_t)(B(FIREANIM(s)) + 0x30), SW(PX(s)), SW(PY(s)));
}
/* BA7A(s): the grappling hooks / corvus while C3AA[s] runs down; thrown broadside only; the tip in the other ship ->
   95D2[s] (boarding). Only the Roman side ever has hooks (B6A0's BA7A(0) finds C3AA[0] == 0). */
uint16_t nv_hooks_BA7A(int s) {
    int f = -1, ox = 0, oy = 0, hit = 0;                             /* [bp-0Bh], [bp-8], [bp-4], [bp-2] */
    if (B(HOOKS(s)) == 0) return 0;
    B(HOOKS(s))--;
    int h = SB(HEAD(s));
    int k = h + 16 * B(TYPE(s));                                     /* [bp-0Ah] */
    int rel = (uint8_t)(h - B(BEAR(s))) & 0x0F;
    if (rel > 0 && rel < 8) { f = (uint8_t)(B(0x95D4 + h) + 0x3A); ox = SB(0x95F4 + k); oy = SB(0x9624 + k); }
    if (rel > 8) { f = (uint8_t)(B(0x95E4 + h) + 0x3A); ox = SB(0x9654 + k); oy = SB(0x9684 + k); }
    if (f == -1 || f == 0xFF) { B(HOOKS(s)) = 0; return 0; }
    ox += SW(PX(s)); oy += SW(PY(s));
    nv_sprite_8E62(W(H_BOMBS), f, ox, oy);
    hit = nv_d070(ox + SB(0x96B4 + f - 0x3A), oy + SB(0x96BC + f - 0x3A), s ^ 1);
    return hit ? B(0x95D2 + s) : 0;
}
/* CCDE(s): the fire pot (in flight 98BE + direction, splash 98BE) and the arrow volley (0x20 + direction) */
void nv_shots_CCDE(int s) {
    if (B(POT(s))) {
        uint8_t f = B(POTANIM(s));
        if (B(POT(s)) != 1) f = (uint8_t)(f + B(PDIR(s)));
        nv_sprite_8E62(W(H_BOMBS), (int8_t)f, SW(POTX(s)), SW(POTY(s)));
    }
    if (B(ARROW(s))) nv_sprite_8E62(W(H_BOMBS), (int8_t)(B(ADIR(s)) + 0x20), SW(ARRX(s)), SW(ARRY(s)));
}

/* ---------------------------------------------------------------- the panel */
/* C71B: clip 199; a hull <= 0 becomes 0 with its men; strength bars (enemy red from the left, Rome from the right,
   115 px = 60); "MEN" with both crews, or the Roman speed word for C37C frames; clip 185 */
void nv_panel_C71B(void) {
    nv_clip(0xC7);
    if (SB(HULL(0)) <= 0) { B(HULL(0)) = 0; setW(CREW(0), 0); }
    setW(0x9AFA, 4);
    int w = (int16_t)(0x73 * SB(HULL(0))) / 60;
    nv_fill_7E74(0, 0xBE, w, 0xC4);
    setW(0x9AFA, W(0x6346));
    nv_fill_7E74(w, 0xBE, 0x73, 0xC4);
    if (SB(HULL(1)) <= 0) { B(HULL(1)) = 0; setW(CREW(1), 0); }
    setW(0x9AFA, 4);
    w = (int16_t)(0x73 * SB(HULL(1))) / 60;
    nv_fill_7E74(0x13F, 0xBE, 0x13F - w, 0xC4);
    setW(0x9AFA, W(0x6346));
    nv_fill_7E74(0x13F - w, 0xBE, 0xCC, 0xC4);
    nv_fill_7E74(0x77, 0xC1, 0xC7, 0xC7);
    setW(0x9B69, 0);
    if (B(SPEEDMSG) == 0) {
        nv_text_8BE4(dstr(0x98BA), 0x9F, 0xC1);                      /* "MEN" */
        nv_num_8A9E(W(CREW(1)), 0xAE, 0xC1);
        nv_num_8AC8(W(CREW(0)), 0x90, 0xC1);
    } else {
        B(SPEEDMSG)--;
        nv_text_8BE4(dstr(W(0x9870 + 2 * SB(OARS(1)))), 0x9F, 0xC1);
    }
    nv_clip(0xB9);
}
/* DC8E(e, r): C404 = the Roman ships, C406 = the enemy's (all 3 types); printed at (174, 187) and right-aligned at 144 */
void nv_totals_DC8E(uint8_t e, uint8_t r) {
    setW(0xC406, 0); setW(0xC404, 0);
    for (int i = 0; i < 3; i++) {
        setW(0xC404, (uint16_t)(W(0xC404) + W(0x110E + 12 * r + 2 * i)));
        setW(0xC406, (uint16_t)(W(0xC406) + W(0x110E + 12 * e + 2 * i)));
    }
    nv_clip(0xC7);
    setW(0x9AFA, W(0x6346));
    setW(0x9B69, 0);
    nv_num_8AF2(W(0xC404), 0xAE, 0xBB, 0x19);
    nv_num_8B2C(W(0xC406), 0x90, 0xBB, 0x19);
    nv_clip(0xB9);
}
/* BEAE(e, r): all timer slots = now; sea, black line, panel, "SHIPS" (small font), C71B, DC8E, both ships (on the
   screen, the target the caller left), the screen into the work buffer, flip. On re-engage the clip is already 185, so
   the line, the panel fill and "SHIPS" are clipped away (invisible, the old panel stays) */
void nv_screen_BEAE(uint8_t e, uint8_t r) {
    nv_t_all_00FF();
    setW(0x9AFA, 9);
    nv_fill_7E74(0, 0, 0x13F, 0xB9);
    setW(0x9AFA, 0);
    nv_hline_7ECA(0, 0xBA, 0x13F);
    setW(0x9AFA, W(0x6346));
    nv_fill_7E74(0, 0xBB, 0x13F, 0xC7);
    setW(0x9B69, 0);
    nv_font(0xA3B4);
    nv_text_8BE4(dstr(0x97BA), 0x9F, 0xBB);                          /* "SHIPS" */
    nv_panel_C71B();
    nv_totals_DC8E(e, r);
    nv_ship_B8E3(1);
    nv_ship_B8E3(0);
    nv_save_691B(0, 0, 0x13F, 0xC7);
    nv_flip_6887();
}

/* ---------------------------------------------------------------- the intro */
/* C1BB: when more than 4 ticks have passed since DS:9812, the water colours rotate one step (VGA DAC, 01B6) */
static int palette_C1BB(void) {                                     /* port: 1 when the colours moved */
    uint32_t last = (uint32_t)W(0x9812) | (uint32_t)W(0x9814) << 16;
    uint32_t now = (uint32_t)W(0x42) | (uint32_t)W(0x44) << 16;
    if (!(last + 4 < now)) return 0;
    if (++B(0x9816) > 5) B(0x9816) = 0;
    if (++B(0x9817) > 4) B(0x9817) = 0;
    /* 84BB: wait for the retrace (nothing in the port) */
    for (int i = 0; i < 6; i++) {
        uint16_t c = W(0x9762 + 2 * ((B(0x9816) + i) % 6)), t = (uint16_t)(0x9778 + 6 * i);
        screen_dac(c, W(t), W(t + 2), W(t + 4));
    }
    for (int i = 0; i < 5; i++) {
        uint16_t c = W(0x976E + 2 * ((B(0x9817) + i) % 5)), t = (uint16_t)(0x979C + 6 * i);
        screen_dac(c, W(t), W(t + 2), W(t + 4));
    }
    setW(0x9812, W(0x42)); setW(0x9814, W(0x44));
    return 1;
}
/* The 11 cycled colours (DS:9762, 976E) back to CMAP.BIN. The original's restore (C18C) has no caller, so they stayed
   rotated after the battle (NAVAL_SCREEN_NOTES 4.3); the port restores them at the end unless --original. */
void nv_palette_restore(void) {
    for (int i = 0; i < 11; i++) screen_dac_restore(W(0x9762 + 2 * i));
}
/* C105: the GALLEYS picture with the waving pennant (FLAG frame 1, 2, 3, 0, ... at (38, 5) every 4 ticks over the sky
   colour 0x80) and the water colours cycling, until a key or a held mouse button; then the screen into the work buffer,
   the screen as the target, the FLAG sheet freed. The port: one poll per tick (the input ending also ends it); the
   window is shown only when the pennant or the colours change (the per-tick poll reads the events without presenting:
   a present waits for the display's vsync, which on a 60 Hz display would stretch every 72.8 Hz tick). */
void nv_intro_C105(void) {
    int si = 0;
    uint16_t di = nv_vga() ? 0x80 : 0x0B;
    nv_t_reset_00F8();
    nv_flip_6887();                                                  /* c16f */
    for (;;) {
        mouse_poll_input();                                          /* c172: 0644 */
        if (key_read()) break;                                       /* 03AA */
        if (W(0xB2)) break;                                          /* c11d */
        if (screen_quit()) break;
        int dac = nv_vga() && palette_C1BB();                        /* the DAC changes at once in the original */
        if (nv_t_since0_01AE() > 3) {
            nv_t_reset_00F8();
            setW(0x9AFA, di);
            nv_fill_7E74(0x26, 2, 0x44, 0x0F);
            if (++si > 3) si = 0;
            nv_sprite_8E62(W(H_FLAG), si, 0x26, 5);
            nv_flip_6887();
        } else if (dac) present();                                   /* port: show the new colours */
        nv_tick();
    }
    nv_save_67B6();
    nv_page_6837();
    nv_sheet_19A0(W(H_FLAG));
}

/* ---------------------------------------------------------------- the sinking */
/* D919(s): the sinking area = position + DS:9932[8 * type] (x0, y0, x1, y1), y1 at most 185 */
static void d919(int s, int16_t *r) {
    uint16_t t = (uint16_t)(0x9932 + 8 * B(TYPE(s)));
    for (int i = 0; i < 4; i++) r[i] = (int16_t)SW(t + 2 * i);
    r[0] = (int16_t)(r[0] + SW(PX(s))); r[1] = (int16_t)(r[1] + SW(PY(s)));
    r[2] = (int16_t)(r[2] + SW(PX(s))); r[3] = (int16_t)(r[3] + SW(PY(s)));
    if (r[3] > 0xB9) r[3] = 0xB9;
}
/* D97C(s): union of the hull frame's rect and the resting oars' (frame 93E6[h] + 6) rect, each (x - xhot, y - yhot) to
   (+ w, + rows) */
static void d97c(int s, int16_t *out) {
    int w, xh, yh, rows;
    int16_t a[4], b[4];
    int h = SB(HEAD(s)), x = SW(PX(s)), y = SW(PY(s));
    nv_desc(W(SHEET(s)), h, &w, &xh, &yh, &rows);
    a[0] = (int16_t)(x - xh); a[2] = (int16_t)(w + a[0]); a[1] = (int16_t)(y - yh); a[3] = (int16_t)(rows + a[1]);
    if (h < 8) nv_desc(W(H_OARS1), SB(0x93E6 + h) + 6, &w, &xh, &yh, &rows);
    else nv_desc(W(H_OARS2), SB(0x93E6 + h) + 6 - 56, &w, &xh, &yh, &rows);
    b[0] = (int16_t)(x - xh); b[2] = (int16_t)(w + b[0]); b[1] = (int16_t)(y - yh); b[3] = (int16_t)(rows + b[1]);
    nv_rect_81EA(out, b, a);
}
/* D3F2(bits): bit 0 = the Roman flagship sank, bit 1 = the enemy's; both: an empty sea in the work buffer. One: the
   survivor backs water (oar frames 3, 2, 1, 0, 6, 3, ...) while the rects overlap, the wreck turns the short way to
   heading 4, smoke frame rng(3) at the midpoint (DICE every frame), a frame every 14 ticks; then the work buffer holds
   the sea and the survivor. */
void nv_sink_D3F2(uint16_t bits) {
    int done = 0, s, o;                                              /* [bp-0Eh], [bp-8], [bp-6] */
    int16_t Rs[4], Ro[4];
    if (!bits) return;
    setW(0x9AFA, 9);
    if (bits == 3) { nv_target_work_67E4(); nv_fill_7E74(0, 0, 0x13F, 0xB9); return; }
    if (bits & 1) { s = 1; d919(1, Rs); o = 0; }
    else { s = 0; d919(0, Rs); o = 1; }
    B(OARF(o)) = 6;
    B(OARF(s)) = 3;
    d97c(o, Ro);
    int8_t dir = 1;                                                  /* [bp-1] */
    if (SB(HEAD(s)) > 4 && SB(HEAD(s)) < 12) dir = -1;
    int16_t cx = (int16_t)SW(PX(s)), cy = (int16_t)SW(PY(s));        /* [bp-0Ch], [bp-0Ah] */
    if (nv_rect_817C(NULL, Rs, Ro)) {
        cx = (int16_t)((int16_t)(cx + SW(PX(o))) / 2);
        cy = (int16_t)((int16_t)(cy + SW(PY(o))) / 2);
    }
    while (!done) {
        nv_t_reset_00F8();
        nv_fill_7E74(0, 0, 0x13F, 0xB9);
        int ov = nv_rect_817C(NULL, Rs, Ro);                         /* [bp-4] */
        if (ov) {
            int h = SB(HEAD(o));
            setW(PX(o), (uint16_t)(SW(PX(o)) + SB(0x9958 + h)));
            setW(PY(o), (uint16_t)(SW(PY(o)) + SB(0x9968 + h)));
            B(OARF(o))--;
            if (SB(OARF(o)) < 0) B(OARF(o)) = 6;
            if (B(OARF(o)) == 5) B(OARF(o)) = 3;
            d97c(o, Ro);
        }
        if (B(HEAD(s)) == 4) { if (!ov) done = -1; }
        else B(HEAD(s)) = (uint8_t)((B(HEAD(s)) + dir) & 0x0F);
        nv_ship_B8E3(o);
        nv_ship_B8E3(s);
        int k = nv_rng(3);
        nv_sprite_8E62(W(H_SMOK), k, cx, cy);
        nv_flip_6887();
        while (nv_t_since0_01AE() < 0x0E) nv_tick();
    }
    B(OARF(s)) = 6;
    nv_save_67B6();
    nv_page_6837();
    nv_target_work_67E4();
    nv_fill_7E74(0, 0, 0x13F, 0xB9);
    nv_ship_B8E3(o);
}
/* D89B(k): the shark: erase 10x6, one pixel right, frame AAC6[k]; at frame 5 rng(10) != 0 (9 in 10) goes back to 2
   (circling), else on to 5..7 (it dives) and 0, 1 (it surfaces) */
static void shark_D89B(int k) {
    if (!W(0x9956)) return;
    int x = SW(SHARKX(k)), y = SW(SHARKY(k));
    nv_fill_7E74(x - 5, y - 3, x + 4, y + 2);
    int f = B(SHARKF(k));
    setW(SHARKX(k), (uint16_t)(x + 1));
    nv_sprite_8E62(W(0x9956), f, x + 1, y);
    B(SHARKF(k))++;
    if (B(SHARKF(k)) == 5) { if (nv_rng(10)) B(SHARKF(k)) = 2; }
    else if (B(SHARKF(k)) == 8) B(SHARKF(k)) = 0;
}
static void shark_start(int k, const int16_t *r) {                   /* D6A7..D6C3 / D714..D730 */
    setW(SHARKX(k), (uint16_t)r[0]);
    setW(SHARKY(k), (uint16_t)(r[3] + 4));
    if (SW(SHARKY(k)) >= 0xB9) setW(SHARKY(k), (uint16_t)(r[1] - 2));
    B(SHARKF(k)) = (uint8_t)k;
}
/* D60C(bits): the sinking frames ("xsink" -> ssink / msink / lsink by type; 22 / 27 / 24 frames) of each wreck, one
   every 14 ticks, with a shark per wreck (a shark inside the OTHER wreck's rect stays still); MNAVAL is opened here.
   1990 bug (NAVAL_DATA_NOTES 8): when both flagships sank and they are the same type, the enemy wreck shares the Roman
   sheet and D714..D730 are skipped: its shark started where the last battle's stopped ((0, 0) the first time).
   Fixed: it starts below its own wreck; --original keeps the stale place. */
void nv_sink_D60C(uint16_t bits) {
    uint16_t SR = 0, SE = 0;                                         /* [bp-8], [bp-6] */
    int doneR = 0, doneE = 0, ownR = 0, ownE = 0, n = 0;             /* [bp-2], [bp-4], [bp-0Ch], [bp-0Ah], [bp-0Eh] */
    int16_t RR[4], RE[4];
    if (!bits || !nv_archive_0F7B(8)) return;
    d919(1, RR);
    d919(0, RE);
    if (bits & 1) {
        g_ds[W(0x9950)] = B(0x9952 + B(TYPE(1)));
        SR = nv_sheet_163B(dstr(W(0x9950)));
        if (!SR) { message_box(dstr(0x9978)); return; }             /* "Failed to Load Human Sinking" */
        ownR = -1;
        shark_start(0, RR);
    }
    if (bits & 2) {
        if (SR && B(TYPE(1)) == B(TYPE(0))) {
            SE = SR;
            if (!g_original_bugs) shark_start(1, RE);
        } else {
            g_ds[W(0x9950)] = B(0x9952 + B(TYPE(0)));
            SE = nv_sheet_163B(dstr(W(0x9950)));
            if (!SE) { message_box(dstr(0x9995)); return; }         /* "Failed to Load Computer Sinking" */
            ownE = -1;
            shark_start(1, RE);
        }
    }
    setW(0x9956, nv_sheet_163B(dstr(0x99B5)));                       /* "shark" */
    /* 11A9 */
    nv_target_work_67E4();
    if (SR) nv_sprite_8E62(SR, 0, SW(PX(1)), SW(PY(1)));
    if (SE) nv_sprite_8E62(SE, 0, SW(PX(0)), SW(PY(0)));
    nv_target_screen_680F();
    nv_restore_6973(0, 0, 0x13F, 0xC7);
    nv_target_work_67E4();
    setW(0x9AFA, 9);
    while (!doneR || !doneE) {
        nv_t_reset_00F8();
        if (SR) {
            if (!nv_inrect_8238(SW(SHARKX(0)), SW(SHARKY(0)), RE)) shark_D89B(0);
            if (SW(0x994A + 2 * B(TYPE(1))) > n) { nv_fill_7E74(RR[0], RR[1], RR[2], RR[3]); nv_sprite_8E62(SR, n, SW(PX(1)), SW(PY(1))); }
            else doneR = -1;
        } else doneR = -1;
        if (SE) {
            if (!nv_inrect_8238(SW(SHARKX(1)), SW(SHARKY(1)), RR)) shark_D89B(1);
            if (SW(0x994A + 2 * B(TYPE(0))) > n) { nv_fill_7E74(RE[0], RE[1], RE[2], RE[3]); nv_sprite_8E62(SE, n, SW(PX(0)), SW(PY(0))); }
            else doneE = -1;
        } else doneE = -1;
        nv_flip_6887();
        n++;
        while (nv_t_since0_01AE() < 0x0E) nv_tick();
    }
    if (ownR) nv_sheet_19A0(SR);
    if (ownE) nv_sheet_19A0(SE);
    if (W(0x9956)) nv_sheet_19A0(W(0x9956));
}
