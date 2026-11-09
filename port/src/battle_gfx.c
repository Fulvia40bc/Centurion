/* Land battle graphics: background (BB97/BB7E) and unit sprites (BA4F sort + BA72 draw). Claude, 2026-11-09.
 * VERIFIED pixel-exact (0 differing pixels) against the game's own frame buffer in two DOSBox snapshots of a real battle
 * (tools/dosbox/results/battle/b11_orders*_open.bin + frames, see tools/cmp_battle_frame.py).
 *
 * Background: DESERT.CBM if prov[zone]+9 == 1, else BATTLENW.CBM with WATER.CBM over it at its hotspot (x = 220,
 * nibble 0 transparent); each with the LUT in its own header (23 / 25 / 24).
 * Sprites: seven sheets <name>HDR.LZW/<name>DAT.LZW, table DS:727E (10 bytes: first sprite id, -, name, remap arg):
 * ee 382+ (elephants), enemy rg 308+ / rc 236+ / ri 191+, Roman rg 117+ / rc 45+ / ri 0+. Barbarian enemies (home 11-14,
 * 23-26) use bg/bi. Enemy sheets are loaded through the nibble remap DS:7252, whose entry 4 = DS:7262[enemy home]
 * (the enemy's colour). HDR = 14-byte entries <HhhBBBBI w, xhot, yhot, bpr, rows, transparent nibble, LUT, offset;
 * the offset counts from 2 bytes before the unpacked DAT. Drawn at screen (x - xhot, y - yhot).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "state.h"
#include "assets.h"
#include "platform.h"

typedef struct { int w, xhot, yhot, bpr, rows, trans, lut; uint32_t off; } bspr_t;
typedef struct { bspr_t *e; int n; uint8_t *dat; size_t ndat; } bsheet_t;

static bsheet_t g_sheets[7];
static uint8_t g_bg[SCREEN_W * SCREEN_H];
static int g_loaded;

static uint8_t *member(const char *name, size_t *len) {
    int k = assets_find(name);
    return k < 0 ? NULL : assets_decode(k, len);
}
static void free_sheets(void) {
    for (int k = 0; k < 7; k++) { free(g_sheets[k].e); free(g_sheets[k].dat); memset(&g_sheets[k], 0, sizeof(bsheet_t)); }
}
static int load_sheet(bsheet_t *s, const char *name, const uint8_t *remap) {
    char hn[16], dn[16];
    snprintf(hn, sizeof(hn), "%sHDR.LZW", name); snprintf(dn, sizeof(dn), "%sDAT.LZW", name);
    for (char *c = hn; *c; c++) if (*c >= 'a' && *c <= 'z') *c -= 32;
    for (char *c = dn; *c; c++) if (*c >= 'a' && *c <= 'z') *c -= 32;
    size_t hl = 0;
    uint8_t *h = member(hn, &hl);
    s->dat = member(dn, &s->ndat);
    if (!h || !s->dat) { free(h); fprintf(stderr, "battle gfx: %s missing\n", name); return -1; }
    s->n = (int)(hl / 14);
    s->e = calloc((size_t)s->n + 1, sizeof(bspr_t));
    for (int i = 0; i < s->n; i++) {
        const uint8_t *p = h + 14 * i;
        s->e[i].w = p[0] | p[1] << 8;
        s->e[i].xhot = (int16_t)(p[2] | p[3] << 8);
        s->e[i].yhot = (int16_t)(p[4] | p[5] << 8);
        s->e[i].bpr = p[6]; s->e[i].rows = p[7]; s->e[i].trans = p[8]; s->e[i].lut = p[9];
        s->e[i].off = (uint32_t)(p[10] | p[11] << 8 | p[12] << 16 | (uint32_t)p[13] << 24);
    }
    free(h);
    if (remap)
        for (size_t i = 0; i < s->ndat; i++)
            s->dat[i] = (uint8_t)(remap[s->dat[i] >> 4] << 4 | remap[s->dat[i] & 15]);
    return 0;
}
/* a CBM member drawn into g_bg at its hotspot; trans < 0 = opaque */
static int cbm_into_bg(const char *name, int trans) {
    int k = assets_find(name);
    if (k < 0) return -1;
    size_t rl = 0;
    uint8_t *raw = assets_decode(k, &rl);
    int w, h, lut;
    uint8_t *px = assets_decode_cbm(k, &w, &h, &lut);
    if (!raw || !px) { free(raw); free(px); return -1; }
    int xo = (int16_t)(raw[2] | raw[3] << 8), yo = (int16_t)(raw[4] | raw[5] << 8);
    const uint8_t *table = assets_lut(lut);
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            int X = x - xo, Y = y - yo, n = px[y * w + x];
            if (n == trans || X < 0 || X >= SCREEN_W || Y < 0 || Y >= SCREEN_H) continue;
            g_bg[Y * SCREEN_W + X] = table ? table[n] : (uint8_t)n;
        }
    free(raw); free(px);
    return 0;
}

/* BB97: load the field for zone Z and the sprite sheets for the enemy (DS:C29A must hold the enemy home).
   Needs assets_load("BATL.DIR","BATL.DAT") and assets_load_luts done. */
int battle_gfx_load(int Z) {
    if (g_loaded) free_sheets();
    g_loaded = 0;
    memset(g_bg, 0, sizeof(g_bg));
    if (B((Z << 5) + 0x5279) == 1) { if (cbm_into_bg("DESERT.CBM", -1)) return -1; }
    else {
        /* BATTLENW's nibble 0 (the sea) is transparent and WATER covers it, all but one pixel at (218,8), which keeps
           the previous screen's pixel: colour 105 in both captured battles (the map). */
        memset(g_bg, 105, sizeof(g_bg));
        if (cbm_into_bg("BATTLENW.CBM", 0) || cbm_into_bg("WATER.CBM", 0)) return -1;
    }
    uint16_t home = W(0xC29A);
    int barb = (home >= 0xB && home <= 0xE) || (home >= 0x17 && home <= 0x1A);
    uint8_t remap[16];
    memcpy(remap, &g_ds[0x7252], 16);
    remap[4] = B(0x7262 + home);
    static const char *roman[7] = {"ee", "rg", "rc", "ri", "rg", "rc", "ri"};
    for (int k = 0; k < 7; k++) {
        const char *n = roman[k];
        if (barb && k == 1) n = "bg";
        if (barb && k == 3) n = "bi";
        if (load_sheet(&g_sheets[k], n, (k >= 1 && k <= 3) ? remap : NULL)) return -1;
    }
    g_loaded = 1;
    return 0;
}

/* BD67: the original's quicksort of the sprite index list by the record's position word (y << 8 | x, signed) */
static int16_t key(uint16_t idx) { return (int16_t)W(0xA92A + 6 * idx); }
static void qsort_sprites(uint16_t arr, int lo, int hi) {
    int i = lo, j = hi;
    int16_t pivot = key(W(arr + 2 * ((lo + hi) >> 1)));
    for (;;) {
        while (key(W(arr + 2 * i)) < pivot && i < hi) i++;
        while (key(W(arr + 2 * j)) > pivot && j > lo) j--;
        if (i <= j) {
            uint16_t t = W(arr + 2 * i); setW(arr + 2 * i, W(arr + 2 * j)); setW(arr + 2 * j, t);
            i++; j--;
        }
        if (j < i) break;
    }
    if (lo < j) qsort_sprites(arr, lo, j);
    if (i < hi) qsort_sprites(arr, i, hi);
}
void battle_sort_sprites(void) {                                 /* BA4F */
    uint16_t n = W(0xA928);
    for (uint16_t k = 0; k < n; k++) setW(0xAA02 + 2 * k, k);
    if (n) qsort_sprites(0xAA02, 0, n - 1);
}

static void blit(uint8_t *fb, const bsheet_t *s, int id, int x, int y) {
    if (id < 0 || id >= s->n) return;
    const bspr_t *e = &s->e[id];
    const uint8_t *lut = assets_lut(e->lut);
    x -= e->xhot; y -= e->yhot;
    for (int r = 0; r < e->rows; r++) {
        int Y = y + r;
        if (Y < 0 || Y >= SCREEN_H) continue;
        for (int c = 0; c < e->w; c++) {
            int X = x + c;
            if (X < 0 || X >= SCREEN_W) continue;
            size_t k = e->off - 2 + (size_t)r * e->bpr + c / 2;
            if (k >= s->ndat) continue;
            uint8_t b = s->dat[k], n = (c & 1) ? (b & 15) : (b >> 4);
            if (n == e->trans) continue;
            fb[Y * SCREEN_W + X] = lut ? lut[n] : n;
        }
    }
}
/* grid -> screen, C7B0 */
void battle_grid_to_screen(int x, int y, int *sx, int *sy) { *sx = (x - y) * 8 + 0xA4; *sy = (x + y) * 4 - 0x4A; }

/* BB7E + BA72: the field and every sprite in the sorted list (call battle_sort_sprites first) */
void battle_draw(uint8_t *fb) {
    memcpy(fb, g_bg, sizeof(g_bg));
    if (!g_loaded) return;
    uint16_t n = W(0xA928);
    for (uint16_t k = 0; k < n; k++) {
        uint16_t r = (uint16_t)(0xA92A + 6 * W(0xAA02 + 2 * k));
        int sx, sy; battle_grid_to_screen(SB(r), SB(r + 1), &sx, &sy);
        uint8_t frame = B(r + 4), anim = B(r + 5);
        uint16_t t = W(0x719C + 2 * anim);
        int dx = SB(t + 2 * frame), dy = SB(t + 2 * frame + 1);
        uint16_t id = W(r + 2);
        int f = 0;
        while (f < 7 && W(0x727E + 10 * f) > id) f++;
        if (f == 7) continue;
        blit(fb, &g_sheets[f], id - W(0x727E + 10 * f), sx + dx, sy + dy - 4);
    }
}
