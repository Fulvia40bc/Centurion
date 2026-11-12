/* The gladiator show's pictures (OVL0 section 3, disasm/OVL_GLAD.asm), Claude 2026-11-10: section 3's drawing routines,
 * translated literally after docs/GLAD_SCREEN_NOTES.md and GLAD_DATA_NOTES.md. The logic, the timing and the dice are in
 * glad.c. The EXE helpers are naval_gfx.c's (targets 67E4 / 680F with the descriptor DS:C0D0, 7E74 fill, 7ECA hline,
 * 8BE4 text with the font block DS:9B56, 8E62 sprites from 163B sheets kept as small handles in the DS words C2B4..C2BA,
 * 6973 copy work buffer -> screen); here only what section 3 needs on top of them:
 *   - 13A8 / 8790: ARENABTM.CBM loaded once into "picture slot 4" (the bytes expanded through its colour table, as 13A8
 *     does) and blitted opaque at the point DS:7BC8 = (0, 126);
 *   - 1273 + 8E62 + 86A0: COLOSEUM.CBM drawn as a sprite (hotspot, transparent nibble, colour table of its header);
 *   - 3233: the bevelled box without the inner groove (EXE 3233..32DF, decoded here: fill c unless -1; colour 8 verticals
 *     x0 and x1 (y0..y1), x0+1 and x1-1 (y0+1..y1-1); colour 0 hlines y1 (x0..x1), y1-1 (x0+1..x1-1); colour 15 hlines y0
 *     (x0..x1), y0+1 (x0+1..x1-1); [9AFA] is left at 15);
 *   - 6738 roman_numeral into DS:C072 (battle.c's roman_numeral).
 * The port is VGA only ([9B26] == 2: the grey of the health bars is 0x36, nv_vga()).
 * 1990 slip fixed here (--original keeps it): C149, the crowd meter's rest fill ends at x 221 (0xDD) although the bar
 * reaches 82 + 145 = 227, so the white bar stopped at x 220 (crowd 29 and 30 looked the same, 139 px at most); fixed: the
 * rest fill ends at 227, so exactly w = 145 * v / 30 white pixels show (as the health bars show w red ones).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "state.h"
#include "screen.h"
#include "assets.h"
#include "naval_int.h"
#include "glad.h"

static const char *dstr(uint16_t a) { return (const char *)&g_ds[a]; }

/* ---------------------------------------------------------------- the CBM pictures */
static uint8_t *g_btm;                    /* picture slot 4 (ARENABTM.CBM), colour bytes */
static int g_btm_w, g_btm_h;

/* a CBM member of the open archive: nibbles, size, hotspot, transparent nibble, colour table */
static uint8_t *cbm_member(const char *lower, int *w, int *h, int *xo, int *yo, int *trans, int *lut) {
    char up[16]; size_t i;
    for (i = 0; lower[i] && i < sizeof(up) - 1; i++) up[i] = (char)(lower[i] >= 'a' && lower[i] <= 'z' ? lower[i] - 32 : lower[i]);
    up[i] = 0;
    int k = assets_find(up);
    if (k < 0) return NULL;
    size_t rl = 0;
    uint8_t *raw = assets_decode(k, &rl);
    uint8_t *px = assets_decode_cbm(k, w, h, lut);
    if (!raw || !px || rl < 10) { free(raw); free(px); return NULL; }
    *xo = (int16_t)(raw[2] | raw[3] << 8); *yo = (int16_t)(raw[4] | raw[5] << 8); *trans = raw[8];
    free(raw);
    return px;
}
/* 13A8(4, &arch): picture slot 4 = DS:0250 + 8 * 4 -> name DS:01F0 "arenabtm.cbm", descriptor C0BE */
int gl_picture_13A8(void) {
    int w, h, xo, yo, tr, lut;
    free(g_btm); g_btm = NULL;
    uint8_t *px = cbm_member(dstr(W(0x0250 + 8 * 4)), &w, &h, &xo, &yo, &tr, &lut);
    if (!px) { fprintf(stderr, "glad: %s missing\n", dstr(W(0x0250 + 8 * 4))); return 0; }
    const uint8_t *t = assets_lut(lut);
    for (int i = 0; i < w * h; i++) px[i] = t ? t[px[i] & 15] : px[i];
    g_btm = px; g_btm_w = w; g_btm_h = h;
    return 1;
}
/* 8790(C0BE, DS:7BC8, C0D0): the picture opaque at the point, into the current target (clipped to the screen) */
static void blit_8790(void) {
    if (!g_btm) return;
    int x = SW(0x7BC8), y = SW(0x7BCA);
    for (int r = 0; r < g_btm_h; r++) {
        int Y = y + r;
        if (Y < 0 || Y >= SCREEN_H) continue;
        for (int c = 0; c < g_btm_w; c++) {
            int X = x + c;
            if (X >= 0 && X < SCREEN_W) nv_fb[Y * SCREEN_W + X] = g_btm[r * g_btm_w + c];
        }
    }
}
/* 1273("coloseum.cbm" DS:7B89, &arch, &desc): only tested here; gl_title_draw decodes it again (the port keeps nothing) */
int gl_title_1273(void) { return assets_find("COLOSEUM.CBM") >= 0; }
/* B11B..B154: 67E4; [9AFA] = 0, fill the screen; 8E62(&coloseum, 0, 0); 680F; 86A0 (free); 6973(0, 0, 319, 199) */
void gl_title_draw(void) {
    nv_target_work_67E4();
    setW(0x9AFA, 0);
    nv_fill_7E74(0, 0, 0x13F, 0xC7);
    int w, h, xo, yo, tr, lut;
    uint8_t *px = cbm_member(dstr(0x7B89), &w, &h, &xo, &yo, &tr, &lut);
    if (px) {
        const uint8_t *t = assets_lut(lut);
        for (int r = 0; r < h; r++)
            for (int c = 0; c < w; c++) {
                int X = c - xo, Y = r - yo, n = px[r * w + c];
                if (n == tr || X < SW(0xC0DA) || X > SW(0xC0DE) || Y < SW(0xC0DC) || Y > SW(0xC0E0)) continue;
                if (X >= 0 && X < SCREEN_W && Y >= 0 && Y < SCREEN_H) nv_fb[Y * SCREEN_W + X] = t ? t[n] : (uint8_t)n;
            }
        free(px);
    }
    nv_target_screen_680F();
    nv_restore_6973(0, 0, 0x13F, 0xC7);
}
void gl_pictures_free(void) { free(g_btm); g_btm = NULL; }

/* ---------------------------------------------------------------- the screens */
/* B29B..B2DC: 67E4; [9AFA] = 0, fill; arena frame 0 at (0, 0); 8790 (ARENABTM at (0, 126)); 680F; 6973(0, 0, 319, 199) */
void gl_arena_draw(void) {
    nv_target_work_67E4();
    setW(0x9AFA, 0);
    nv_fill_7E74(0, 0, 0x13F, 0xC7);
    nv_sprite_8E62(W(SH_ARENA), 0, 0, 0);
    blit_8790();
    nv_target_screen_680F();
    nv_restore_6973(0, 0, 0x13F, 0xC7);
}

/* B67C: the empty card grid in the work buffer: dark blue (1), the 16 hlines DS:7BDA {colour w, y b, pad}, the 14 pieces
   DS:7C1A {x w, y w, frame b, pad} of sheet "arena" (columns, plinths); 680F; 6973(0, 0, 319, 199) */
void gl_grid_B67C(void) {
    nv_target_work_67E4();
    setW(0x9AFA, 1);
    nv_fill_7E74(0, 0, 0x13F, 0xC7);
    for (uint16_t si = 0x7BDA; si < 0x7C1A; si += 4) {               /* b695 */
        setW(0x9AFA, W(si));
        nv_hline_7ECA(0, B(si + 2), 0x13F);
    }
    for (uint16_t si = 0x7C1E; si < 0x7C72; si += 6)                 /* b6b8 */
        nv_sprite_8E62(W(SH_ARENA), B(si), SW(si - 4), SW(si - 2));
    nv_target_screen_680F();
    nv_restore_6973(0, 0, 0x13F, 0xC7);
}

/* 6738(n) -> DS:C072 */
static uint16_t numeral_6738(unsigned n) {
    char b[32];
    roman_numeral(n, b);
    size_t l = strlen(b);
    memcpy(&g_ds[0xC072], b, l + 1);
    return 0xC072;
}
/* B6EC(pos [bp+8], rec [bp+6], facing [bp+4]): a card on the current target (the screen). cx = 56 + 106 (pos % 3),
   yb = 87 + 100 (pos / 3); the name (+ the generation's numeral, men only, generation != 1) at (cx, yb - 80); with wins:
   "Bouts Won" at yb - 70 and the numeral at yb - 60; the portrait of sheet "show": facing != 0 -> frame body at cx - 10,
   facing 0 -> frame body + 2 at cx + 10 (+ 15 for a beast), at y yb */
void gl_card_B6EC(int pos, uint8_t rec, int facing) {
    char name[0x30 + 2];                                             /* [bp-30h] */
    int yb = 0x57 + 0x64 * ((unsigned)pos / 3);                      /* [bp-4] */
    int cx = 0x38 + 0x6A * ((unsigned)pos % 3);                      /* [bp-2] */
    snprintf(name, sizeof(name), "%s", dstr(W(REC(rec, R_NAME))));    /* 9E32 */
    if (B(REC(rec, R_STY)) != 4 && B(REC(rec, R_GEN)) != 1) {
        size_t l = strlen(name);
        snprintf(name + l, sizeof(name) - l, "%s", dstr(numeral_6738(B(REC(rec, R_GEN)))));   /* 9DF2 */
    }
    nv_text_8BE4(name, cx, yb - 0x50);
    if (B(REC(rec, R_WON)) != 0) {
        nv_text_8BE4(dstr(0x7DAD), cx, yb - 0x46);                   /* "Bouts Won" */
        nv_text_8BE4(dstr(numeral_6738(B(REC(rec, R_WON)))), cx, yb - 0x3C);
    }
    int frame, x;                                                    /* [bp-8], [bp-6] */
    if (facing) { frame = B(REC(rec, R_BODY)); x = cx - 10; }
    else {
        frame = B(REC(rec, R_BODY)) + 2; x = cx + 10;
        if (B(REC(rec, R_STY)) == 4) x += 0x0F;
    }
    nv_sprite_8E62(W(SH_SHOW), frame, x, yb);
}

/* 3233(x0, y0, x1, y1, c), see the header */
void gl_box_3233(int x0, int y0, int x1, int y1, int c) {
    if (c != -1) { setW(0x9AFA, (uint16_t)c); nv_fill_7E74(x0, y0, x1, y1); }
    setW(0x9AFA, 8);
    nv_fill_7E74(x0, y0, x0, y1);                                    /* 925C verticals */
    nv_fill_7E74(x0 + 1, y0 + 1, x0 + 1, y1 - 1);
    nv_fill_7E74(x1, y0, x1, y1);
    nv_fill_7E74(x1 - 1, y0 + 1, x1 - 1, y1 - 1);
    setW(0x9AFA, 0);
    nv_hline_7ECA(x0, y1, x1);
    nv_hline_7ECA(x0 + 1, y1 - 1, x1 - 1);
    setW(0x9AFA, 0x0F);
    nv_hline_7ECA(x0, y0, x1);
    nv_hline_7ECA(x0 + 1, y0 + 1, x1 - 1);
}

/* C149(v): w = 145 v / 30 (16-bit imul / idiv) clamped 0..145; white (82, 13)-(82 + w, 17), then [6346] from 82 + w to
   x 221 (1990, --original) or 227 (fixed, see the header) */
void gl_crowd_C149(int v) {
    int16_t si = (int16_t)((int16_t)(0x91 * (int16_t)v) / 0x1E);
    if (si < 0) si = 0;
    else if (si > 0x91) si = 0x91;
    setW(0x9AFA, 0x0F);
    nv_fill_7E74(0x52, 0x0D, si + 0x52, 0x11);
    setW(0x9AFA, W(0x6346));
    nv_fill_7E74(si + 0x52, 0x0D, g_original_bugs ? 0xDD : 0x52 + 0x91, 0x11);
}

/* C1A4: 67E4; 8790 (ARENABTM at (0, 126)); the frames of both states; the opponent first and the player on top, except
   when the opponent lies in his dying state (then the player first, the body on top); 680F; 6973(0, 126, 319, 185) */
void gl_fighters_C1A4(void) {
    nv_target_work_67E4();
    blit_8790();
    uint8_t fp = gl_mt(GL_P, 2), fo = gl_mt(GL_O, 2);                /* [bp-1], [bp-2] */
    if (B(F_ST(GL_O)) != B(F_DIE(GL_O))) {
        nv_sprite_8E62(W(SH_OPP), fo, SW(F_X(GL_O)), SW(F_Y(GL_O)));
        nv_sprite_8E62(W(SH_PLAYER), fp, SW(F_X(GL_P)), SW(F_Y(GL_P)));
    } else {
        nv_sprite_8E62(W(SH_PLAYER), fp, SW(F_X(GL_P)), SW(F_Y(GL_P)));
        nv_sprite_8E62(W(SH_OPP), fo, SW(F_X(GL_O)), SW(F_Y(GL_O)));
    }
    nv_target_screen_680F();
    nv_restore_6973(0, 0x7E, 0x13F, 0xB9);
}

/* C302(health, side): w = max(health, 0) * 128 / 50; red (4) from the left edge for side 0 (the opponent), from the right
   edge for side 1 (the player), the rest grey (0x36 on VGA, else 8) up to x 128 / from x 191, rows 188..193 */
static void bar_C302(int health, int side) {
    int si = nv_vga() ? 0x36 : 8;
    if (health < 0) health = 0;
    int di = (int16_t)((int16_t)(health << 7) / 0x32);
    setW(0x9AFA, 4);
    if (side == 0) {
        nv_fill_7E74(0, 0xBC, di, 0xC1);
        setW(0x9AFA, (uint16_t)si);
        nv_fill_7E74(di, 0xBC, 0x80, 0xC1);
    } else {
        di = 0x13F - di;
        nv_fill_7E74(0x13F, 0xBC, di, 0xC1);
        setW(0x9AFA, (uint16_t)si);
        nv_fill_7E74(di, 0xBC, 0xBF, 0xC1);
    }
}
/* C25F(side): [9AFA] = 8 (overwritten by C302); C302(side ? P.health : O.health, side) */
void gl_health_C25F(int side) {
    setW(0x9AFA, 8);
    bar_C302(side == 0 ? SB(F_HP(GL_O)) : SB(F_HP(GL_P)), side);
}

/* BFD8: the thumbs box, arena frame 3 at (79, 20), on the screen */
void gl_thumbs_box(void) { nv_sprite_8E62(W(SH_ARENA), 3, 0x4F, 0x14); }
