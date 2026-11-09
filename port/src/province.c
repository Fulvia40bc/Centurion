/* The province side of the map: the provincial screen 0x6CB8, the Rome screen 0x7366 and their commands, the lists
 * and the foreign zone message. Claude, 2026-11-09. Literal translation of EXE segment 0823 (disasm/EXE_RUNTIME.bin,
 * llvm-objdump per function; docs/GAMELOOP_NOTES.md section 4 and its verifier corrections). Every function carries
 * its EXE address. Works on the DS image (state.h) with the original addresses and widths.
 *
 * Drawing: the original composes a screen in the work buffer (0x67E4 selects it as the target) and copies it to the
 * screen (0x680F selects the screen again, 0x6973 copies); g_fb is that target here (g_wrk or g_scr, screen.h).
 * Menus, boxes and texts after the copy go straight to the screen, as in the original.
 * Sprites (0x8E62(y, x, desc): at (x - xhot, y - yhot), nibble = header byte 8 transparent, the entry's colour table)
 * come from the sheets the start-up (OVL0 section 1 B18A) loads: "mmenu" (CENT, [C0F6]: the panel frame of 0x30E6),
 * "init" (MINIT, 14 descriptors whose owners are listed at DS:6350: C0E2 C1D6 C108 C10C C10A C110 C0F0 C0F2 C0F4 C0E6
 * C0E8 C0EA C0EC C0EE), "init2" (MINIT, [C28A]: the crowd by mood). The province strips are CBMs of archive 7 (MNEGO)
 * loaded by 0x1273 and drawn with 0x8E62 too.
 *
 * Not ported here (each says so on screen and leaves the DS as it is): the chariot race (section 4 D08A) and the
 * gladiator show (section 3 B0E0) of Rome's Hold Games; the show's amphitheatre purchase (0x34CB, 50 talents) is
 * skipped with it, and as neither module writes its once-per-turn word (154E / 154C) they may be chosen again.
 * Build Fleet has no naval part: it is ported whole, with the auto-repeat of a held button on +/- (0x2900).
 * The legions.h UI hooks are installed by the caller (main.c, legions_ui_install), not by these entry points.
 * The mouse cursor calls (0x1D55 / 0x1D8A / 0x1CCE) and the disk prompt 0x1E21 do nothing in the port.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "screen.h"
#include "platform.h"
#include "state.h"
#include "assets.h"
#include "legions.h"
#include "fleet.h"

#define P(z)   ((uint16_t)((uint16_t)(uint8_t)(z) << 5))      /* + 0x5270..: province record 5270 + 32z */
#define AC60   0xAC60u

static uint8_t *g_fb = g_scr;                                  /* drawing target: 0x67E4 work buffer / 0x680F screen */

static const char *dstr(uint16_t a) { return (const char *)&g_ds[a]; }
static void ds_strcpy(uint16_t d, const char *s) { size_t n = strlen(s); memmove(&g_ds[d], s, n + 1); }     /* 9E32 */
static void ds_strcat(uint16_t d, const char *s) { ds_strcpy((uint16_t)(d + strlen(dstr(d))), s); }        /* 9DF2 */
static void ds_strcat_num(uint16_t d, long v) { char b[24]; snprintf(b, sizeof(b), "%ld", v); ds_strcat(d, b); }  /* A022/9FDC + 9DF2 */

/* ---------------------------------------------------------------- drawing primitives of segment 0823 */
/* 0x7E74(y1, x1, y0, x0): filled rect in [9AFA], inclusive, corners swapped if reversed, clipped to the screen */
static void fill7e74(int x0, int y0, int x1, int y1) {
    if (x1 < x0) { int t = x0; x0 = x1; x1 = t; }
    if (y1 < y0) { int t = y0; y0 = y1; y1 = t; }
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > SCREEN_W - 1) x1 = SCREEN_W - 1;
    if (y1 > SCREEN_H - 1) y1 = SCREEN_H - 1;
    uint8_t c = (uint8_t)W(0x9AFA);
    for (int y = y0; y <= y1; y++) memset(g_fb + y * SCREEN_W + x0, c, x1 >= x0 ? (size_t)(x1 - x0 + 1) : 0);
}
static void hline7eca(int x0, int y, int x1) { fill7e74(x0, y, x1, y); }                 /* 0x7ECA(x1, y, x0) */
static void pixel82de(int x, int y) {                                                     /* 0x82DE(y, x) */
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) g_fb[y * SCREEN_W + x] = (uint8_t)W(0x9AFA);
}
static void text899c(int x, int y, const char *s) { ui_text_attr(g_fb, x, y, s, W(0x9B69)); }          /* 0x899C(y, x, s) */
static void text8be4(int cx, int y, const char *s) { ui_text_block_attr(g_fb, cx, y, s, W(0x9B69)); }  /* 0x8BE4(y, x, s) */
static void text8c3a(int x, int y, const char *s) { text899c(x - ui_text_advance(s), y, s); }          /* 0x8C3A: right-aligned */
static void num8a9e(int x, int y, long v) { char b[24]; snprintf(b, sizeof(b), "%ld", v); text899c(x, y, b); }  /* 0x8A9E */
static void num8ac8(int x, int y, long v) { char b[24]; snprintf(b, sizeof(b), "%ld", v); text8c3a(x, y, b); }  /* 0x8AC8 */
/* 0x8AF2(w, y, x, v): fill (x, y + yoff[' '], x + w, y + yoff[' '] + [9B5C]) in [9AFA], then 0x8A9E */
static void num8af2(int x, int y, int w, long v) {
    int yo = B(W(0x9B58));
    fill7e74(x, y + yo, x + w, y + yo + B(0x9B5C));
    num8a9e(x, y, v);
}
static ui_box_t box_of(int x0, int y0, int x1, int y1) { ui_box_t r = { x0, y0, x1, y1 }; return r; }
static void save691b(int x0, int y0, int x1, int y1) { ui_copy_rect(g_wrk, g_scr, box_of(x0, y0, x1, y1)); }
static void restore6973(int x0, int y0, int x1, int y1) { ui_copy_rect(g_scr, g_wrk, box_of(x0, y0, x1, y1)); }

/* ---------------------------------------------------------------- sprites and CBMs (0x8E62) */
typedef struct { int w, xhot, yhot, bpr, rows, trans, lut; uint32_t off; } spr_t;
typedef struct { spr_t *e; int n; uint8_t *dat; size_t ndat; int tried; } sheet_t;
static sheet_t g_mmenu, g_init, g_init2;

static uint8_t *member(const char *name, size_t *len) {
    int k = assets_find(name);
    return k < 0 ? NULL : assets_decode(k, len);
}
/* xxxHDR.LZW = 14-byte descriptors <HhhBBBBI (w, xhot, yhot, bytes/row, rows, transparent nibble, colour table, offset);
   xxxDAT.LZW = 4-bit pixels, the offset counting from 2 bytes before the unpacked data (as battle_gfx.c, VERIFIED there) */
static int sheet_load(sheet_t *s, const char *dir, const char *dat, const char *name) {
    if (s->tried) return s->e ? 0 : -1;
    s->tried = 1;
    char hn[24], dn[24];
    snprintf(hn, sizeof(hn), "%sHDR.LZW", name); snprintf(dn, sizeof(dn), "%sDAT.LZW", name);
    size_t hl = 0;
    uint8_t *h = NULL;
    if (assets_load(dir, dat) == 0) { h = member(hn, &hl); s->dat = member(dn, &s->ndat); }
    if (!h || !s->dat) { free(h); free(s->dat); s->dat = NULL; fprintf(stderr, "province: sprite sheet %s missing\n", name); return -1; }
    s->n = (int)(hl / 14);
    s->e = calloc((size_t)s->n + 1, sizeof(spr_t));
    for (int i = 0; i < s->n; i++) {
        const uint8_t *p = h + 14 * i;
        s->e[i].w = p[0] | p[1] << 8;
        s->e[i].xhot = (int16_t)(p[2] | p[3] << 8);
        s->e[i].yhot = (int16_t)(p[4] | p[5] << 8);
        s->e[i].bpr = p[6]; s->e[i].rows = p[7]; s->e[i].trans = p[8]; s->e[i].lut = p[9];
        s->e[i].off = (uint32_t)(p[10] | p[11] << 8 | p[12] << 16 | (uint32_t)p[13] << 24);
    }
    free(h);
    return 0;
}
static void blit_nibbles(const uint8_t *px, size_t npx, size_t base, int bpr, int w, int rows, int trans, int lut, int x, int y) {
    const uint8_t *t = assets_lut(lut);
    for (int r = 0; r < rows; r++) {
        int Y = y + r;
        if (Y < 0 || Y >= SCREEN_H) continue;
        for (int c = 0; c < w; c++) {
            int X = x + c;
            if (X < 0 || X >= SCREEN_W) continue;
            size_t k = base + (size_t)r * bpr + c / 2;
            if (k >= npx) continue;
            uint8_t n = (c & 1) ? (px[k] & 15) : (px[k] >> 4);
            if (n == trans) continue;
            g_fb[Y * SCREEN_W + X] = t ? t[n] : n;
        }
    }
}
/* 0x8E62(y, x, desc) for sheet sprite id */
static void sprite8e62(sheet_t *s, int id, int x, int y) {
    if (!s->e || id < 0 || id >= s->n) return;
    const spr_t *e = &s->e[id];
    if (e->off < 2) return;
    blit_nibbles(s->dat, s->ndat, e->off - 2, e->bpr, e->w, e->rows, e->trans, e->lut, x - e->xhot, y - e->yhot);
}
static sheet_t *mmenu(void) { sheet_load(&g_mmenu, "CENT.DIR", "CENT.DAT", "MMENU"); return &g_mmenu; }   /* [636C] = "mmenu" (VGA) */
static sheet_t *init1(void) { sheet_load(&g_init, "MINIT.DIR", "MINIT.DAT", "INIT"); return &g_init; }
static sheet_t *init2(void) { sheet_load(&g_init2, "MINIT.DIR", "MINIT.DAT", "INIT2"); return &g_init2; }

/* 0x1273 (load a CBM member of the open archive into a descriptor), 0x8E62(y, x, desc), 0x86A0 (free).
   The archive must be loaded (assets_load). Returns 0 when the member was there. */
static int cbm_draw(const char *name, int x, int y) {
    char up[16]; size_t i;
    for (i = 0; name[i] && i < sizeof(up) - 1; i++) up[i] = (char)(name[i] >= 'a' && name[i] <= 'z' ? name[i] - 32 : name[i]);
    up[i] = 0;
    int k = assets_find(up);
    if (k < 0) return -1;
    size_t rl = 0;
    uint8_t *raw = assets_decode(k, &rl);
    int w, h, lut;
    uint8_t *px = assets_decode_cbm(k, &w, &h, &lut);
    if (!raw || !px || rl < 10) { free(raw); free(px); return -1; }
    int xo = (int16_t)(raw[2] | raw[3] << 8), yo = (int16_t)(raw[4] | raw[5] << 8), trans = raw[8];
    const uint8_t *t = assets_lut(lut);
    for (int r = 0; r < h; r++)
        for (int c = 0; c < w; c++) {
            int X = x - xo + c, Y = y - yo + r, n = px[r * w + c];
            if (n == trans || X < 0 || X >= SCREEN_W || Y < 0 || Y >= SCREEN_H) continue;
            g_fb[Y * SCREEN_W + X] = t ? t[n] : (uint8_t)n;
        }
    free(raw); free(px);
    return 0;
}

/* ---------------------------------------------------------------- the menu engine around ui.c / screen.c */
/* 0x30E6(y, x): the panel frame: [9AFA] = [6346], rect(x+4, y+3, x+0x92, y+0x71), then sprites [C0F6] + 0, 14, 28, 42
   (mmenu 0..3) at (x, y) */
static void frame30e6(int x, int y) {
    setW(0x9AFA, W(0x6346));
    fill7e74(x + 4, y + 3, x + 0x92, y + 0x71);
    for (int k = 0; k < 4; k++) sprite8e62(mmenu(), k, x, y);
}
/* 0x5614 menu_draw(rec, mode): mode 1 = the frame 0x30E6 then the items, ornament and title (ui.c mode 0 draws exactly
   those); mode 2 = box 3134 + the rest (ui.c); 0 = items only */
static void menu_draw5614(uint16_t rec, int mode) {
    menu_t m;
    menu_load_ds(&m, rec);
    if (mode == 1) { frame30e6(m.x, m.y); ui_menu_draw_mode(g_fb, &m, 0, -1); }
    else ui_menu_draw_mode(g_fb, &m, mode, -1);
}
/* 0x54C8 menu_run(rec, pre) with a mouse: the item, 0xFF = Esc (or the end of the input) */
static uint16_t menu_run54c8(uint16_t rec) {
    menu_t m;
    menu_load_ds(&m, rec);
    int r = menu_choose(&m, -1, -1);
    return r < 0 ? 0xFF : (uint16_t)r;
}
/* 0x585B(rec, item, on) */
static void menu_mark585b(uint16_t rec, int item, int on) {
    menu_t m;
    menu_load_ds(&m, rec);
    ui_menu_highlight(g_fb, &m, item, on);
}
/* 0x55EA menu_on(rec): [rec+0x10] = 0xFFFF, 0x56B6 (ornament, title) */
static void menu_on55ea(uint16_t rec) {
    menu_t m;
    setW((uint16_t)(rec + 0x10), 0xFFFF);
    menu_load_ds(&m, rec);
    ui_menu_enable(g_fb, &m);
}
/* 0x55FF menu_off(rec): [rec+0x10] = 0, 0x57F7 (ornament erased, title in colour 6) */
static void menu_off55ff(uint16_t rec) {
    menu_t m;
    setW((uint16_t)(rec + 0x10), 0);
    menu_load_ds(&m, rec);
    ui_menu_disable(g_fb, &m);
}
static void msgbox59d4(uint16_t s) { message_box(dstr(s)); }
static void not_ported(const char *what) {
    char b[160];
    snprintf(b, sizeof(b), "%s\nis not ported yet", what);
    fprintf(stderr, "province: %s NOT PORTED (DS left as it is)\n", what);
    message_box(b);
}

/* ---------------------------------------------------------------- province screen pieces */
/* 0x6C26 mood(z): ((+0x10) - (+0x0E) + (+0x0D)) / 25, signed bytes, idiv; 0..3 */
static int mood6c26(uint8_t z) {
    uint16_t b = P(z);
    int v = (SB(b + 0x5280) - SB(b + 0x527E) + SB(b + 0x527D)) / 25;
    return v < 0 ? 0 : v > 3 ? 3 : v;
}
/* 0x2A07 coastal(z): the first neighbour B(50EC + 8z) when it is a sea (27..0xFE), else 0xFF */
static uint8_t coastal2a07(uint8_t z) {
    uint8_t a = B(0x50EC + ((uint16_t)z << 3));
    return (a >= 0x1B && a != 0xFF) ? a : 0xFF;
}
/* 0x71BB: the lines under the strip */
static void strip_lines71bb(void) {
    setW(0x9AFA, 0); hline7eca(1, 0x3F, 0x13F);
    setW(0x9AFA, 8); hline7eca(2, 0x40, 0x13F);
    setW(0x9AFA, W(0x6344)); pixel82de(0, 0x3F); pixel82de(0, 0x40); pixel82de(1, 0x40);
}
/* 0x70BA(archive, z): rows 0..63 of the screen. Rome: rome.cbm (DS:5881). Else: the sky fill (colour 0x0B, 0x17 when
   [9B26] = 2 = VGA) on rows 0..31, a cloud picture "skyN.cbm" (DS:58DA, N = roll(4)) when roll(100) >= [58A6 + z],
   "negbg?.cbm" (DS:58CE, ? = 'A' + [588A + z]) and "negfg?.cbm" (DS:58C2, ? = '@' + z) at y 31; then 0x71BB.
   The letters are written into the DS strings, as in the original. */
static void strips_draw70ba(uint8_t z) {
    if (z == 0) {
        cbm_draw(dstr(0x5881), 0, 0);
    } else {
        uint16_t si = rng_roll(100);
        setW(0x9AFA, 0x0B);
        if (W(0x9B26) == 2) setW(0x9AFA, 0x17);
        fill7e74(0, 0, 0x13F, 0x1F);
        if (B(0x58A6 + z) <= si) {
            B(0x58DD) = (uint8_t)(rng_roll(4) + '0');
            cbm_draw(dstr(0x58DA), 0, 0);
        }
        B(0x58D3) = (uint8_t)(B(0x588A + z) + 'A');
        cbm_draw(dstr(0x58CE), 0, 0);
        B(0x58C7) = (uint8_t)(z + '@');
        cbm_draw(dstr(0x58C2), 0, 0x1F);
    }
    strip_lines71bb();
}
/* 0x708A strips_load(z): open archive 7 (MNEGO), select the work buffer (0x67E4), 0x70BA, close */
static void strips_load708a(uint8_t z) {
    if (assets_load("MNEGO.DIR", "MNEGO.DAT") != 0) return;
    g_fb = g_wrk;
    strips_draw70ba(z);
}
/* For nego.c (0x6570, with archive 7 already loaded by its caller): 0x70BA, 0x71BB and 0x1273 + 0x8E62 + 0x86A0 drawn
   on the given target (the target this file keeps is restored after) */
void province_strips70ba(uint8_t *fb, uint8_t z) { uint8_t *s = g_fb; g_fb = fb; strips_draw70ba(z); g_fb = s; }
void province_lines71bb(uint8_t *fb) { uint8_t *s = g_fb; g_fb = fb; strip_lines71bb(); g_fb = s; }
int  province_cbm(uint8_t *fb, const char *name, int x, int y) { uint8_t *s = g_fb; g_fb = fb; int r = cbm_draw(name, x, y); g_fb = s; return r; }
/* 0x3512 panel_frame: 0x30E6(79, 164), then "Rank: " + rank name + "\nTalents: " + treasury at (171, 156) */
static void panel_frame3512(void) {
    frame30e6(0xA4, 0x4F);
    setW(0x9B69, 0);
    ds_strcpy(AC60, dstr(0x1692));
    ds_strcat(AC60, dstr(W(0x5972 + 2 * B(0x5971))));
    ds_strcat(AC60, dstr(0x1699));
    ds_strcat_num(AC60, W(0x634A));
    text899c(0xAB, 0x9C, dstr(AC60));
}
/* 0x6F0C province_panel(z) */
static void province_panel6f0c(uint8_t z) {
    int si = mood6c26(z);
    uint16_t di = P(z);
    ds_strcpy(AC60, dstr(0x59CC));                                    /* "The people are " */
    ds_strcat(AC60, dstr(W(0x5734 + 2 * si)));
    setW(0x9B69, 0);
    text8be4(0xEF, 0x56, dstr(AC60));
    ds_strcpy(AC60, dstr(0x584F));                                    /* "Status: " */
    ds_strcat(AC60, dstr(W(0x57B2 + 2 * B(di + 0x5285))));
    ds_strcat(AC60, dstr(0x5858));                                    /* "\nTribute: " */
    ds_strcat_num(AC60, B(di + 0x5281));
    ds_strcat(AC60, dstr(0x5863));                                    /* "\nManpower: " */
    ds_strcat_num(AC60, W(di + 0x5274));
    ds_strcat(AC60, dstr(0x586F));                                    /* "\nCourage: " */
    ds_strcat(AC60, dstr(W(0x7486 + 2 * (B(di + 0x527A) / 25))));
    text899c(0xAB, 0x6A, dstr(AC60));
    sprite8e62(init2(), si, 0x118, 0x83);                             /* [C28A] + 14 * mood at (280, 131) */
    ds_strcpy(AC60, dstr(W(0x484 + 2 * B(0x48E))));                   /* difficulty name */
    for (int i = 1; i < 4; i++)
        if (B(0x48E + i) != B(0x48E)) ds_strcpy(AC60, dstr(W(0x48C)));   /* mixed: "Fine Tuning" */
    ds_strcat(AC60, dstr(0x587A));                                    /* " level" */
    text8be4(0xEF, 0xB4, dstr(AC60));
}
/* 0x680F + 0x6973(0, 0, 319, 199): the screen is the target again and gets the composed picture */
static void present_composed(void) {
    g_fb = g_scr;
    restore6973(0, 0, 0x13F, 0xC7);
    screen_present();
}

/* ---------------------------------------------------------------- commands */
/* 0x7CE0 tribute_menu(z): the level of z (+0x12) through menu DS:6054 */
static void tribute_menu7ce0(uint8_t z) {
    uint16_t a = (uint16_t)(P(z) + 0x5282);
    uint16_t si = B(a);
    wait_release();                                                   /* 35DB, 1D55(0) */
    menu_draw5614(0x6054, 1);
    if (W(0xBA)) menu_mark585b(0x6054, si, 1);
    si = menu_run54c8(0x6054);
    if (si < 5) B(a) = (uint8_t)si;
}
/* 0x7D42 tribute_policy(): DS:5FC0 through menu DS:60AE, then +0x12 of every province with status != 0 */
static void tribute_policy7d42(void) {
    uint16_t di = B(0x5FC0);
    wait_release();
    menu_draw5614(0x60AE, 1);
    setW(0x9B69, 0);
    if (W(0xBA)) menu_mark585b(0x60AE, di, 1);
    di = menu_run54c8(0x60AE);
    if (di < 5) {
        B(0x5FC0) = (uint8_t)di;
        for (uint16_t si = 0x5285; si < 0x55E5; si += 0x20)
            if (B(si) != 0) B(si - 3) = B(0x5FC0);
    }
}
/* 0x3342 provincial_games(z): costs DS:15D4 {15, 30, 40, 50}, menu DS:15B0; one show per command */
static void provincial_games3342(uint8_t z) {
    uint16_t di = 0;                                                  /* the menu choice */
    uint16_t done = 0;                                                /* [bp-2] */
    if (B(0x15D4) > W(0x634A)) { msgbox59d4(W(0x15D8)); return; }   /* "You don't have enough Talents..." */
    uint16_t a = (uint16_t)(P(z) + 0x527E);                           /* [bp-4]: +0x0E (unrest) */
    int16_t si = SB(a);
    while (!done) {
        menu_draw5614(0x15B0, 1);
        di = menu_run54c8(0x15B0);
        if (di > 3) return;
        menu_mark585b(0x15B0, di, 0);
        if (B(0x15D4 + di) > W(0x634A)) { msgbox59d4(W(0x15D8)); return; }
        switch (di) {
        case 0: si = (int16_t)(si - 10); break;
        case 1: si = (int16_t)(si * 3); si = (int16_t)(si / 4); break;   /* imul 3, idiv 4 */
        case 2: si = (int16_t)(si / 2); break;
        case 3: si = (int16_t)(si / 4); break;
        }
        done = 0xFFFF;
        if (di < 4) {
            setW(0x634A, (uint16_t)(W(0x634A) - B(0x15D4 + di)));
            msgbox59d4(W(0x15DA));                                    /* "The Roman governor stages a spectacle..." */
            B(a) = (uint8_t)(si >= 1 ? si : 1);
            setW(0xC100, (uint16_t)(W(0xC100) + 1));
            if (SB(0xED5) > 10) B(0xED5) = 10;
            if (z == B(0xED6)) { setW(0xEDA, 0); B(0xED6) = 0; }
        }
        panel_frame3512();
        province_panel6f0c(z);                                        /* 0x342D: back to the loop test, done is set */
    }
}
/* 0x3457 rome_games(): menu DS:15C2; returns -1 when a module ran (the caller redraws the Rome screen) */
static int rome_games3457(void) {
    int16_t si = 0;
    menu_draw5614(0x15C2, 1);                                         /* after 1D55(0) */
    uint16_t di = menu_run54c8(0x15C2);
    if (di == 0) {                                                    /* Racing Chariots */
        if (W(0x154E) == W(0x634C)) msgbox59d4(0x15DC);               /* "Only One Race Per Turn" */
        else if (W(0x634A) >= 5) {                                    /* 1E21 disk check, B0AA = section 4 D08A */
            not_ported("The chariot race");
            si = -1;
        } else msgbox59d4(0x15F3);                                    /* "You cannot afford to\nenter a chariot race." */
    } else if (di == 1) {                                             /* Gladiator Show */
        if (W(0x154A) == 0) {                                         /* no amphitheatre */
            if (W(0x634A) < 0x32) { msgbox59d4(0x161E); goto out; }
            menu_off55ff(0x15C2);
            if (!yes_no(dstr(0x164E))) { menu_on55ea(0x15C2); goto out; }   /* "...Spend 50 Talents?" */
            /* 0x34CB-0x34D6: [634A] -= 50 and [154A] = FFFF (the amphitheatre) pay for the show module only (154A is
               read nowhere else in the EXE); as the show is not ported they are NOT done: the treasury stays as it is.
               [15D2] = FFFF is the menu record's own flag (15C2 + 0x10, cleared by 55FF above) and is kept. */
            setW(0x15D2, 0xFFFF);
        } else if (W(0x154C) == W(0x634C)) { msgbox59d4(0x167A); goto out; }  /* "Only One Show Per Turn" */
        not_ported("The gladiator show");                             /* 1E21, B0B6 = section 3 B0E0 */
        si = -1;
    }
out:
    if (!si) menu_mark585b(0x15C2, di, 0);
    return si;                                                        /* 1D8A */
}

/* ---- Build Fleet 0x2458 and its parts ---- */
/* 0x2687 enemy_fleet(z): an enemy fleet (records 3..5) in the sea B(50EC + 8z) */
static int enemy_fleet2687(uint8_t z) {
    int si = 0;
    for (uint8_t f = 3; f < 6; f++) {
        uint16_t b = (uint16_t)(12 * f);
        if (B(0x50EC + ((uint16_t)z << 3)) == B(b + 0x1114) && B(b + 0x1116) != 0xFF) si = -1;
    }
    return si;
}
/* 0x2A32 fleet_screen(f): the item texts of DS:11DC get "N ships" (written into the DS strings), menu 11DC in mode 1,
   the three ship pictures (init 9..11 = [C0E6], [C0E8], [C0EA]) and the fleet name centred at (109, 183) */
static void fleet_screen2a32(uint16_t f) {
    wait_release();                                                   /* 1D55(0), 35DB */
    uint16_t di = (uint16_t)(0x110E + 12 * f);
    for (uint16_t si = 0x116A; si < 0x118E; si += 0x0C, di += 2) {
        char b[16];
        snprintf(b, sizeof(b), "%d", (int16_t)W(di));                /* 9FDC(W(di), AC60, 10) */
        ds_strcpy(AC60, b);
        char *p = strchr((char *)&g_ds[W(si)], ' ');                  /* 9FF8 */
        if (p) {
            p[1] = 0;
            ds_strcat(W(si), dstr(AC60));
            ds_strcat(W(si), dstr(0x1162));                           /* " ships" */
        }
    }
    menu_draw5614(0x11DC, 1);
    sprite8e62(init1(), 9, 0x5A, 0x60);
    sprite8e62(init1(), 10, 0x52, 0x79);
    sprite8e62(init1(), 11, 0x41, 0x96);
    setW(0x9B69, 0);
    text8be4(0x6D, 0xB7, dstr(W(0x1100 + 2 * f)));
}
/* 0x2852 buy_draw(type): box DS:11CA (OK / Cancel; its count is 2 with a mouse, 0 without, while it is drawn, then 4 for
   the +/- arrows), the arrows (init 12, 13), the ship type, "Cost:" and the price, "Available talents:" */
static void buy_draw2852(uint16_t type) {
    B(0x11D8) = (uint8_t)(W(0xBA) >= 1 ? 2 : 0);
    menu_draw5614(0x11CA, 2);
    B(0x11D8) = 4;
    sprite8e62(init1(), 12, 0x5A, 0x51);
    sprite8e62(init1(), 13, 0x5A, 0x5B);
    setW(0x9B69, 0);
    uint16_t si = (uint16_t)(14 * type);
    text899c(0x7C, 0x55, dstr((uint16_t)(si + 0x11EE)));
    text899c(0xBE, 0x55, dstr(0x12DA));                               /* "Cost:" */
    num8a9e(0xD6, 0x55, B(si + 0x11FB));
    text899c(0x72, 0x74, dstr(0x12E0));                               /* "Available talents:" */
    if (W(0xBA) == 0) text899c(0x5A, 0x69, dstr(0x12F3));            /* "Enter to Buy      ESC to Cancel" */
}
/* 0x00F8 / 0x01AE: timer slot 0 restarted / the ticks of the 72.8 Hz timer since then (0xFFFF when more than 16 bits).
   Only 0x2900 uses slot 0 in province.c; the slot's value from before (other callers) is taken as long ago. */
static uint32_t g_slot0;
static int g_slot0_set;
static void ticks_reset00f8(void) { g_slot0 = platform_ticks_ms(); g_slot0_set = 1; }
static uint16_t ticks01ae(void) {
    if (!g_slot0_set) return 0xFFFF;
    uint32_t t = (uint32_t)(((uint64_t)(platform_ticks_ms() - g_slot0) * 728u) / 10000u);
    return t > 0xFFFF ? 0xFFFF : (uint16_t)t;
}
/* 0x2900 buy_poll(rec) with a mouse ([BA] != 0; its 1D55(1) cursor-on is not drawn in the port). The loop 0x2917..0x296D,
   while 0x552B / 0x5593 give 0x100 (nothing):
     0x0644 poll (+ 0x1BE6 cursor); no button down -> [12D2] = FFFF;
     [12D2] == 0 (still down since the last hit) and 0x01AE < 10 -> poll again: the auto-repeat delay (10 ticks);
     button down -> [12D2] = 0, 0x552B(rec, [B6]/[B8]) (any button); else 0x5593(rec, 0x03AA key); 0x00F8.
   There is no 0x35DB: a held button on +/- steps again every 10 ticks. 0xFF = Esc.
   The port polls with wait_input (0x359B), which returns at once while a button is held and blocks (the original spins)
   while none is. A release while it blocks is not reported, so it is seen by the time: a held-button return takes no
   sleep, a return after a block at least one 5 ms poll (screen.c) -> [12D2] = FFFF as the original's up poll sets it.
   A script click is a press and a release ("click" lines never hold), so in scripted runs [12D2] = FFFF before each poll. */
static uint16_t buy_poll2900(uint16_t rec) {
    menu_t m;
    menu_load_ds(&m, rec);
    int r = -2;                                                       /* si = 0x100 */
    while (r == -2) {
        if (screen_scripted()) setW(0x12D2, 0xFFFF);                  /* the release between two script clicks */
        uint32_t t = platform_ticks_ms();
        int b = wait_input();                                         /* 0x0644 */
        if (b == 0 && screen_quit()) return 0xFF;
        if (b == 0 || platform_ticks_ms() - t >= 4) setW(0x12D2, 0xFFFF);   /* 0x2925: the button was up */
        if (W(0x12D2) == 0 && ticks01ae() < 10) {                     /* 0x2932 */
            if (!screen_headless()) platform_sleep_ms(1);
            continue;
        }
        if (b) { setW(0x12D2, 0); r = ui_menu_hit(&m, SW(0xB6), SW(0xB8)); }   /* 0x2948 */
        else r = ui_menu_hotkey(&m, B(0xC07F));                       /* 0x295A */
        ticks_reset00f8();                                            /* 0x2966 */
    }
    return r == -1 ? 0xFF : (uint16_t)r;                              /* 1D8A */
}
/* 0x26D0 buy(min, type, f): ships of one type for fleet f; min = the count before this Build Fleet command */
static void buy26d0(uint16_t min, uint16_t type, uint16_t f) {
    int16_t si = 0;                                                   /* ships added in this box */
    uint16_t done = 0;
    save691b(0x54, 0x4B, 0xEA, 0x83);
    buy_draw2852(type);
    uint16_t F = (uint16_t)(0x110E + 2 * (6 * f + type));             /* [bp-8] */
    uint16_t cur = W(F);                                              /* [bp-4] */
    uint16_t money = W(0x634A);                                       /* [bp-2] */
    uint16_t built = (uint16_t)(0x12D4 + 2 * type);                   /* [bp-0xA] */
    wait_release();
    while (!done) {
        setW(0x9AFA, 0xF);
        fill7e74(0x65, 0x54, 0x77, 0x5F);
        setW(0x9B69, 0);
        num8ac8(0x77, 0x55, (int16_t)(W(built) + si));
        setW(0x9AFA, W(0x6346));
        fill7e74(0xC2, 0x74, 0xE5, 0x7E);
        setW(0x9B69, 0);
        num8a9e(0xC2, 0x74, money);
        uint16_t r = buy_poll2900(0x11CA);
        if (r == 0xFF) { done = 0xFFFF; continue; }
        if (r > 0xFF) continue;
        uint8_t cost = B(0x11FB + 14 * type);
        switch (r & 0xFF) {
        case 0:                                                       /* OK */
            if (si > 0) { setW(built, (uint16_t)(W(built) + si)); setW(F, (uint16_t)(W(F) + si)); }
            else if (si < 0) {
                si = (int16_t)-si;
                setW(built, (uint16_t)(W(built) - si)); setW(F, (uint16_t)(W(F) - si));
            }
            setW(0x634A, money);
            done = 0xFFFF;
            break;
        case 1: done = 0xFFFF; break;                                 /* Cancel */
        case 2:                                                       /* + */
            if (cur < 0x3E7 && cost <= money) { cur++; si++; money = (uint16_t)(money - cost); }
            break;
        case 3:                                                       /* - */
            if (cur > min) { cur--; si--; money = (uint16_t)(money + cost); }
            break;
        }
    }
    restore6973(0x54, 0x4B, 0xEA, 0x83);
    setW(0x9AFA, W(0x6346)); setW(0x9B69, 0);                         /* 0x44E5(treasury) */
    num8af2(0xD3, 0xA7, 0x37, W(0x634A));
    fleet_screen2a32(f);
}
/* 0x2458 build_fleet(z) */
static void build_fleet2458(uint8_t z) {
    uint8_t here = 0xFF;                                              /* [bp-2]: a Roman fleet in z's sea */
    uint8_t nb = 0;                                                   /* [bp-3]: fleets in service */
    uint8_t f;                                                        /* [bp-4] */
    wait_release();
    for (uint8_t i = 0; i < 3; i++) if (B(12 * i + 0x1116) != 0xFF) nb++;
    for (f = 0; f < 3; f++) {
        uint16_t b = (uint16_t)(12 * f);
        if (B(0x50EC + ((uint16_t)z << 3)) == B(b + 0x1114) && B(b + 0x1116) != 0xFF) break;
    }
    if (f < 3) here = f;
    if (here == 0xFF) for (f = 0; f < 3 && B(12 * f + 0x1116) != 0xFF; f++) ;   /* the first free record */
    if (coastal2a07(z) == 0xFF) { msgbox59d4(0x1274); return; }                 /* "There is no ocean here" */
    if (B(0x5968 + B(0x5971)) == 0) { msgbox59d4(0x1218); return; }            /* "You cannot command any\nfleets..." */
    if (enemy_fleet2687(z)) { msgbox59d4(0x128C); return; }                     /* "The enemy fleet\nprevents..." */
    if (nb == 3 && here == 0xFF) { msgbox59d4(0x12B6); return; }                /* "All fleets have been built" */
    if (B(0x5968 + B(0x5971)) <= nb && here == 0xFF) { msgbox59d4(0x1244); return; }   /* "...any more\nfleets..." */
    uint16_t fw = f;                                                  /* [bp-0x14] */
    fleet_screen2a32(fw);                                             /* after 1D55(0) */
    uint16_t b12 = (uint16_t)(12 * fw);                               /* [bp-0x12] */
    uint16_t a = W(b12 + 0x110E), b = W(b12 + 0x1110), c = W(b12 + 0x1112);
    for (int k = 0; k < 3; k++) setW((uint16_t)(0x12D4 + 2 * k), 0);
    uint16_t di = 0, si = 0;
    while (!si) {
        di = menu_run54c8(0x11DC);
        menu_off55ff(0x11DC);
        if (di == 0xFF) si = 0xFFFF;
        else if (di < 0xFF) switch (di & 0xFF) {
            case 0: buy26d0(a, di, fw); break;
            case 1: buy26d0(b, di, fw); break;
            case 2: buy26d0(c, di, fw); break;
            case 3: si = 0xFFFF; break;
            }
        menu_mark585b(0x11DC, di, 0);
        menu_on55ea(0x11DC);
    }
    uint16_t n = 0;
    for (int k = 0; k < 3; k++) n = (uint16_t)(n + W(0x110E + 2 * (6 * fw + k)));
    if (here == 0xFF) {
        if (n) {
            B(b12 + 0x1116) = 2;
            B(b12 + 0x1117) = 0xFF;
            B(b12 + 0x1115) = z;
            B(b12 + 0x1114) = coastal2a07(z);
        }
    } else if ((uint16_t)(c + b + a) < n) B(b12 + 0x1116) = 2;
    /* 1D8A */
}

/* ---- See Province List 0x69CC ---- */
/* 0x6BA2 row(y, z): name at 30, status at 96, mood at 142, manpower right-aligned at 226, tribute right-aligned at 281 */
static void plist_row6ba2(int y, uint8_t z) {
    int md = mood6c26(z);
    text899c(0x1E, y, dstr(W(0x5222 + 2 * z)));
    uint16_t si = P(z);
    text899c(0x60, y, dstr(W(0x57B2 + 2 * B(si + 0x5285))));
    text899c(0x8E, y, dstr(W(0x5734 + 2 * md)));
    num8ac8(0xE2, y, W(si + 0x5274));
    num8ac8(0x119, y, B(si + 0x5281));
}
void province_list(void) {                                            /* 0x69CC */
    uint8_t n = 0;                                                    /* [bp-1] */
    int h = B(0x9B5C);                                                /* [bp-0x14] */
    int lp = h + B(0x9B5D);                                           /* [bp-0xC] */
    int scroll = 0;                                                   /* [bp-0x12] */
    for (uint16_t si = 0x5285; si < 0x55E5; si += 0x20) if (B(si) >= 1) n++;
    if (n == 0) return;
    uint8_t rows = n;                                                 /* [bp-9] */
    if (n > 14) { rows = 14; scroll = -1; }
    int16_t tot = (int16_t)(rows * h + (lp + 4) * 3);
    int16_t di = (int16_t)(0x5A - tot / 2);                           /* top */
    int16_t bottom = (int16_t)(tot + di);                             /* [bp-0x10] */
    int16_t top = di;                                                 /* [bp-0xE] */
    save691b(0x18, di, 0x127, bottom);                                /* after 1D55(0) */
    ui_box(g_fb, 0x18, di, 0x127, bottom, W(0x6346));
    setW(0x9B69, 0xF);
    text8be4(0xA0, di + 6, dstr(0x56EF));                             /* "Province List" */
    bottom = (int16_t)(bottom - (lp + 2));
    text8be4(0xA0, bottom, dstr(W(0x3E40)));                          /* "Click to continue" */
    setW(0x9B69, 0);
    for (uint16_t p = 0x57A2; p < 0x57B2; p += 4) text899c(W(p + 2), lp + di + 6, dstr(W(p)));   /* the column heads */
    int16_t y0 = (int16_t)((lp + 3) * 2 + di);                        /* [bp-0x16] */
    int16_t y = y0;
    uint16_t cnt = 0, k;
    for (k = 0; k < 0x1B; k++) {
        uint8_t z = B(0x5718 + k);                                    /* display order */
        if (B(P(z) + 0x5285) < 1) continue;
        plist_row6ba2(y, z);
        y = (int16_t)(y + h);
        if (rows <= ++cnt) break;
    }
    wait_click();                                                     /* 35F9 */
    if (scroll) {
        uint16_t si = (uint16_t)(k + 1);
        di = y0;
        setW(0x9AFA, W(0x6346));
        fill7e74(0x1E, di, 0x121, bottom);
        for (k = si; k < 0x1B; k++) {
            uint8_t z = B(0x5718 + k);
            if (B(P(z) + 0x5285) < 1) continue;
            plist_row6ba2(di, z);
            di = (int16_t)(di + h);
        }
        wait_click();
    }
    restore6973(0x18, top, 0x127, bottom + lp + 2);
    screen_present();                                                 /* 1D8A */
}

/* ---- zone_info 0x720B ---- */
void zone_info(int zi) {
    uint8_t z = (uint8_t)zi;
    B(AC60) = 0;
    int found = 0;
    uint16_t di = 0;
    for (uint16_t si = 0xC11A; si < 0xC1B4; si += 0x0E, di++) {       /* army records C112 + 14i: +8 active, +1 zone */
        if (W(si) == 0 || B(si - 7) != z) continue;
        ds_strcpy(AC60, dstr(0x58E3));                                /* "Marauding Army\nof " */
        ds_strcat(AC60, dstr(W(0x5222 + 2 * W(0xC114 + 14 * di))));   /* +2 home */
        ds_strcat(AC60, dstr(0x58F6));                                /* "\nin " */
        found = -1;
        break;
    }
    if (!found && z > 0x1A) {
        di = 3;
        for (uint16_t si = 0x113A; si < 0x115E; si += 0x0C, di++) {   /* enemy fleets 3..5: +2 status, +0 zone */
            if (B(si) == 0xFF || B(si - 2) != z) continue;
            ds_strcpy(AC60, dstr(W(0x1100 + 2 * di)));
            ds_strcat(AC60, dstr(0x58FB));                            /* "\nin " */
            break;
        }
    }
    ds_strcat(AC60, dstr(W(0x5222 + 2 * z)));
    message_box(dstr(AC60));
}

/* ---- Complete Fleet List 0x2FD6 ---- */
/* 0x2D60 fleets_in(z): A69C = free fleets in z, A6A1 = fleets in service, A69E[] = the fleets in z; returns their count */
uint8_t fleets_in2d60(uint8_t z) {                                    /* also fleet.c (2C6F, 2DB2) */
    uint8_t n = 0;
    B(0xA69C) = 0; B(0xA6A1) = 0;
    uint8_t dl = 0;
    for (uint16_t si = 0x1116; si < 0x113A; si += 0x0C, dl++) {
        if (B(si) == 0xFF) continue;
        B(0xA6A1)++;
        if (B(si - 2) != z) continue;
        B(0xA69E + n) = dl;
        n++;
        if (B(si) == 0) B(0xA69C)++;
    }
    return n;
}
/* 0x2E96 fleet_row(y, f): name at 25, the three ship counts right-aligned at 104 / 162 / 216, the legion aboard (numeral
   via 0x6738 into DS:C072, or "None") centred at 256, the status 1B54[] centred at 295 */
static void fleet_row2e96(int y, uint8_t f) {
    setW(0x9B69, 0);
    text899c(0x19, y, dstr(W(0x1100 + 2 * f)));
    uint16_t b = (uint16_t)(12 * f);
    num8ac8(0x68, y, W(b + 0x110E));
    num8ac8(0xA2, y, W(b + 0x1110));
    num8ac8(0xD8, y, W(b + 0x1112));
    char buf[16];
    uint8_t l = B(b + 0x1117);
    if (l == 0xFF) snprintf(buf, sizeof(buf), "%s", dstr(0x139E));    /* "None" */
    else {
        char r[32];
        roman_numeral(B(20 * l + 0x1922), r);
        ds_strcpy(0xC072, r);                                         /* 6738 */
        snprintf(buf, sizeof(buf), "%s", dstr(0xC072));
    }
    text8be4(0x100, y, buf);
    text8be4(0x127, y, dstr(W(0x1B54 + 2 * B(b + 0x1116))));
}
/* 0x2E96 for fleet.c (2DB2, 2F5A): drawn on fb */
void province_fleet_row(uint8_t *fb, int y, uint8_t f) { uint8_t *s = g_fb; g_fb = fb; fleet_row2e96(y, f); g_fb = s; }
void fleet_list(void) {                                               /* 0x2FD6 */
    fleets_in2d60(B(0xC080));                                         /* after 1D55(0) */
    if (B(0xA6A1) != 0) {
        uint16_t h = (uint8_t)((uint8_t)(B(0xA6A1) + 3) * B(0x9B5C) + B(0x9B5D) + 0x0C);   /* byte arithmetic */
        int16_t di = (int16_t)((int16_t)(0xC8 - h) / 2);
        int16_t bottom = (int16_t)(h + di);
        int16_t si = (int16_t)(B(0x9B5D) + di + 5);
        save691b(2, di, 0x13D, bottom);
        ui_box(g_fb, 2, di, 0x13D, bottom, W(0x6346));
        char title[96];
        snprintf(title, sizeof(title), "%s%s", dstr(0x13A3), dstr(W(0x3E40)));   /* "Complete Fleet List -- " */
        setW(0x9B69, 0xF);
        text8be4(0xA0, di + 5, title);
        setW(0x9B69, 0);
        text899c(0x50, (B(0x9B5C) + 3) * 2 + di, dstr(0x1314));      /* "Triremes  Quinqueremes  Galleons   Legion   Status" */
        si = (int16_t)(si + 2 * B(0x9B5C));
        for (uint8_t f = 0; f < 3; f++) {
            if (B(12 * f + 0x1116) == 0xFF) continue;
            si = (int16_t)(si + B(0x9B5C));
            fleet_row2e96(si, f);
        }
        wait_click();
        restore6973(2, di, 0x13D, bottom);
    }
    screen_present();                                                 /* 1D8A */
}

/* ---- Complete Legion List 0x4280 (legions.c) ---- */
void legion_list(void) {
    legions_list();
}

/* ---------------------------------------------------------------- the two screens */
/* 0x6CB8 province_screen(z), z = 1..26 (Roman) */
void province_screen(int zi) {
    uint8_t z = (uint8_t)zi;
    uint16_t redraw = 0xFFFF;                                         /* [bp-4] */
    uint16_t leave = 0;                                               /* [bp-6] */
    uint16_t asked = 0;                                               /* [bp-8]: zeroed once per visit (verifier corr. 3) */
    uint16_t sel = 0;                                                 /* [bp-0xA] */
    wait_release();                                                   /* 35DB, 1D55(0) */
    strips_load708a(z);
    setW(0x9AFA, W(0x6344));
    fill7e74(0, 0x41, 0x13F, 0xC7);
    setW(0x9B69, 0);
    text8be4(0xA0, 0x41, dstr(W(0x5222 + 2 * z)));
    panel_frame3512();
    province_panel6f0c(z);
    present_composed();
    while (!leave) {
        if (redraw) {                                                 /* 0x6D32 */
            B(W(0x579A) + 0x33) = coastal2a07(z) == 0xFF ? 0xFF : 0;  /* item 4 "Build Fleet" disabled byte: grey */
            menu_draw5614(0x5790, 1);
            redraw = 0;
        }
        sel = menu_run54c8(0x5790);
        if (sel == 0xFF) { leave = 0xFFFF; continue; }
        if (sel > 0xFF) continue;
        switch (sel & 0xFF) {
        case 0:                                                       /* Legions */
            leave = (uint16_t)legions_cmd();
            redraw = 0xFFFF;
            break;
        case 1: {                                                     /* Plunder, inline 0x6DB2 */
            uint16_t si;
            for (si = 0x1925; si < 0x1A15; si += 0x14)
                if (B(si) != 0xFF && B(si - 1) == z) break;
            if (si < 0x1A15) {
                menu_off55ff(0x5790);
                if (yes_no(dstr(0x57BE))) {                           /* "Are you sure?" */
                    uint16_t bx = P(z);
                    if (B(bx + 0x5285) == 1) {                        /* Occupied */
                        B(0x634E) >>= 1;
                        uint8_t al = B(bx + 0x527E);
                        if ((int8_t)al > 0x32) al = 0x32;
                        B(bx + 0x527E) = (uint8_t)(al << 1);
                    } else {
                        B(0x634E) >>= 2;
                        uint8_t al = B(bx + 0x527E);
                        if ((int8_t)al > 0x19) al = 0x19;
                        B(bx + 0x527E) = (uint8_t)(al << 2);
                    }
                    uint8_t t = B(bx + 0x5284);
                    setW(0x634A, (uint16_t)(W(0x634A) + t));
                    uint32_t tot = (uint32_t)W(0xC09A) | (uint32_t)W(0xC09C) << 16;
                    tot += t;
                    setW(0xC09A, (uint16_t)tot); setW(0xC09C, (uint16_t)(tot >> 16));
                    if (t == 0) msgbox59d4(0x5834);                   /* "There's nothing\nto plunder" */
                    else {
                        B(bx + 0x527F) >>= 1;
                        B(bx + 0x5284) = 0;
                        ds_strcpy(AC60, dstr(0x57F4));                /* "You plunder the province for " */
                        ds_strcat_num(AC60, t);
                        ds_strcat(AC60, dstr(0x5812));                /* "\ntalents.  The natives are upset." */
                        msgbox59d4(AC60);
                    }
                    panel_frame3512();
                    province_panel6f0c(z);
                }
                menu_on55ea(0x5790);
                asked = 0xFFFF;
            }
            if (asked == 0) msgbox59d4(0x57CC);                       /* "You must have a legion\nthere to plunder" */
            menu_mark585b(0x5790, 1, 0);
            break;
        }
        case 2: tribute_menu7ce0(z); redraw = 0xFFFF; break;
        case 3: provincial_games3342(z); redraw = 0xFFFF; break;
        case 4: build_fleet2458(z); redraw = 0xFFFF; break;
        case 5: province_list(); menu_mark585b(0x5790, 5, 0); break;
        case 6: leave = 0xFFFF; break;
        default: break;
        }
    }
    map_build(z);                                                     /* 494A(z), 1D8A */
}

/* 0x7411: draw the Rome screen (composed in the work buffer, then copied) */
static void rome_draw7411(void) {
    strips_load708a(0);
    setW(0x9AFA, W(0x6344));
    fill7e74(0, 0x41, 0x13F, 0xC7);
    strip_lines71bb();
    setW(0x9B69, 0);
    text8be4(0xA0, 0x41, dstr(0x5A42));                               /* "ROME" */
    panel_frame3512();
    province_panel6f0c(0);
    present_composed();
}
/* 0x7366 rome_screen() */
void rome_screen(void) {
    uint16_t leave = 0;                                               /* di */
    uint16_t first = 0xFFFF;                                          /* [bp-2] */
    uint16_t sel = 0;                                                 /* [bp-4] */
    wait_release();                                                   /* 1D55(0), 35DB */
    rome_draw7411();
    while (!leave) {
        if (first) menu_draw5614(0x5A30, 1);
        first = 0xFFFF;                                               /* 0x7396: redrawn after every command */
        sel = menu_run54c8(0x5A30);
        if (sel == 0xFF) { leave = 0xFFFF; first = 0; continue; }
        if (sel > 0xFF) continue;
        switch (sel & 0xFF) {
        case 0: leave = (uint16_t)legions_cmd(); break;
        case 1: tribute_menu7ce0(0); break;
        case 2: if (rome_games3457()) rome_draw7411(); break;
        case 3: build_fleet2458(0); break;
        case 4: tribute_policy7d42(); break;
        case 5: province_list(); break;
        case 6: leave = 0xFFFF; first = 0; break;
        default: break;
        }
    }
    map_build(0);                                                     /* 494A(0), 1D8A */
}

/* ---------------------------------------------------------------- the Legions UI (legions.h hooks) */
static int hook_menu(uint16_t menu, int pre) { (void)pre; return menu_run54c8(menu); }
/* 382A: 359B, then 6C67([B6], [B8]) for a button; Esc / another key */
static int hook_click(void) {
    int b = wait_input();
    if (b == 0) {
        if (screen_quit()) return LEGIONS_ESC;
        return B(0xC07F) == 0x1B ? LEGIONS_ESC : LEGIONS_KEY;
    }
    return zone_at(SW(0xB6), SW(0xB8));
}
/* 3DB7 legion_row: all colour 0 (legions.c wrote [9B69] = 0 before; the words are drawn with that colour here) */
static void draw_row3db7(const legions_row_t *r) {
    char b[24];
    ui_text_attr(g_fb, r->num_x - ui_text_advance(r->numeral), r->y, r->numeral, 0);
    ui_text_attr(g_fb, r->name_x, r->y, r->name, 0);
    ui_text_attr(g_fb, 0x4D, r->y, r->type, 0);
    snprintf(b, sizeof(b), "%u", r->inf); ui_text_attr(g_fb, 0x90 - ui_text_advance(b), r->y, b, 0);
    snprintf(b, sizeof(b), "%u", r->cav); ui_text_attr(g_fb, 0xBC - ui_text_advance(b), r->y, b, 0);
    ui_text_block_attr(g_fb, 0xEC, r->y, r->col4, 0);
    ui_text_block_attr(g_fb, 0x124, r->y, r->status, 0);
}
static void hook_ui(int ev, int a, int b, const void *data) {
    g_fb = g_scr;
    switch (ev) {
    case LEV_WAIT_RELEASE: wait_release(); break;
    case LEV_WAIT_CLICK: wait_click(); break;
    case LEV_MENU_DRAW: menu_draw5614((uint16_t)a, b); break;
    case LEV_MENU_ON: { menu_t m; menu_load_ds(&m, (uint16_t)a); ui_menu_enable(g_fb, &m); break; }
    case LEV_MENU_OFF: { menu_t m; menu_load_ds(&m, (uint16_t)a); ui_menu_disable(g_fb, &m); break; }
    case LEV_MENU_MARK: menu_mark585b((uint16_t)a, b, 0); break;
    case LEV_RECT_RESTORE: { const int *r = data; restore6973(r[0], r[1], r[2], r[3]); screen_present(); break; }
    case LEV_LEGION_BOX: {                                            /* 3CB0 */
        const legions_box_t *x = data;
        if (x->full) {
            save691b(x->rect[0], x->rect[1], x->rect[2], x->rect[3]);
            menu_draw5614(0x1B7E, 2);
            ui_text_attr(g_fb, 0x73, x->header_y, x->header, 0);
            for (int i = 0; i < x->n; i++) draw_row3db7(&x->rows[i]);
        }
        if (x->title) {                                               /* [9AFA] = [6346] (legions.c) */
            fill7e74(7, 5, 0x138, 0xF);
            ui_text_block_attr(g_fb, 0xA0, 5, x->title, 0xF);
        }
        break;
    }
    case LEV_MOVE_HEADER: {                                           /* 3EA3 */
        const legions_box_t *x = data;
        save691b(x->rect[0], x->rect[1], x->rect[2], x->rect[3]);
        ui_box(g_fb, x->rect[0], x->rect[1], x->rect[2], x->rect[3], W(0x6346));
        ui_text_attr(g_fb, 0x73, x->header_y, x->header, 0);
        draw_row3db7(&x->rows[0]);
        break;
    }
    case LEV_LEGION_LIST: {                                           /* 4280 */
        const legions_list_t *x = data;
        save691b(x->rect[0], x->rect[1], x->rect[2], x->rect[3]);
        ui_box(g_fb, x->rect[0], x->rect[1], x->rect[2], x->rect[3], W(0x6346));
        ui_text_block_attr(g_fb, 0xA0, x->title_y, x->title, 0xF);
        ui_text_attr(g_fb, 0x73, x->header_y, x->header, 0);
        for (int i = 0; i < x->n; i++) draw_row3db7(&x->rows[i]);
        break;
    }
    case LEV_MAP_REDRAW: map_build(a); break;
    case LEV_HIGHLIGHT: {                                             /* 8E62([C10C], anchor) */
        int x = SW(0x1AB8 + 4 * a), y = SW(0x1ABA + 4 * a);
        if (b) { x -= 3; y -= 4; }
        sprite8e62(init1(), 3, x, y);
        break;
    }
    case LEV_TREASURY: num8af2(0xD3, 0xA7, 0x37, (uint16_t)a); break;   /* 44E5 */
    case LEV_RECRUITS: num8af2(0xE1, 0x80, 0x18, (uint16_t)a); break;   /* 4510 */
    default: break;                                                   /* cursor, music, disk: nothing in the port */
    }
}
void legions_ui_install(void) {
    legions_message = message_box;
    legions_yesno = yes_no;
    legions_menu = hook_menu;
    legions_click = hook_click;
    legions_ui = hook_ui;
}
