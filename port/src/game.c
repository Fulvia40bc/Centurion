#include "platform.h"
#include "assets.h"
#include "ui.h"
#include "compat.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static uint8_t g_fb[SCREEN_W * SCREEN_H];      /* what SDL shows */
static uint8_t g_work[SCREEN_W * SCREEN_H];    /* off-screen work buffer, like the original */
static rgb_t g_palette[256];

static img_entry_t g_imgs[256];
static int g_nimg;
static int g_cur;
static int g_viewer;

static int load_file(const char *path, uint8_t *buf, size_t want) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    size_t n = fread(buf, 1, want, f);
    fclose(f);
    return n == want ? 0 : -1;
}

static void load_palette(const char *path) {
    uint8_t cmap[750];
    for (int i = 0; i < 256; i++)
        g_palette[i].r = g_palette[i].g = g_palette[i].b = (uint8_t)i;
    if (load_file(path, cmap, sizeof(cmap)) == 0) {
        /* CMAP.BIN: 750 bytes = 250 entries x 3 bytes of 6-bit VGA DAC values */
        for (int i = 0; i < 250; i++) {
            g_palette[i].r = (uint8_t)(cmap[i * 3] << 2);
            g_palette[i].g = (uint8_t)(cmap[i * 3 + 1] << 2);
            g_palette[i].b = (uint8_t)(cmap[i * 3 + 2] << 2);
        }
        printf("palette: %s (250 x 6-bit RGB)\n", path);
    } else {
        printf("palette %s not found, using grayscale\n", path);
    }
}

static void blit_cbm_at(const img_entry_t *e, int px0, int py0) {
    int w, h, xo, yo, trans, lut = 0;
    uint8_t *px = assets_decode_cbm_file(e->dat, e, &w, &h, &xo, &yo, &trans, &lut);
    if (!px) return;
    const uint8_t *table = assets_lut(lut ? lut : 0);
    if (!table) table = assets_lut(0);
    for (int y = 0; y < h; y++) {
        int dy = py0 + y;
        if (dy < 0 || dy >= SCREEN_H) continue;
        for (int x = 0; x < w; x++) {
            int dx = px0 + x;
            if (dx < 0 || dx >= SCREEN_W) continue;
            g_work[dy * SCREEN_W + dx] = table[px[y * w + x] & 0xF];
        }
    }
    free(px);
}

static void blit_cbm_at_trans(const img_entry_t *e, int px0, int py0, int trans_nibble) {
    int w, h, xo, yo, trans, lut = 0;
    uint8_t *px = assets_decode_cbm_file(e->dat, e, &w, &h, &xo, &yo, &trans, &lut);
    if (!px) return;
    const uint8_t *table = assets_lut(lut ? lut : 0);
    if (!table) table = assets_lut(0);
    for (int y = 0; y < h; y++) {
        int dy = py0 + y;
        if (dy < 0 || dy >= SCREEN_H) continue;
        for (int x = 0; x < w; x++) {
            int dx = px0 + x;
            if (dx < 0 || dx >= SCREEN_W) continue;
            uint8_t nib = px[y * w + x] & 0xF;
            if (nib == (uint8_t)trans_nibble) continue;   /* byte8 IS transparent for overlays (VERIFIED) */
            g_work[dy * SCREEN_W + dx] = table[nib];
        }
    }
    free(px);
}

static void blit_cbm(const img_entry_t *e) {
    int w, h, xo, yo, trans, lut = 0;
    uint8_t *px = assets_decode_cbm_file(e->dat, e, &w, &h, &xo, &yo, &trans, &lut);
    if (!px) return;
    (void)lut;
    /* BB1/BB2 are the EGA versions of BATTLENW/DESERT (same layout, dithered for 16 colours); a VGA game
       never shows them, so the viewer draws them with the identity table = the standard EGA colours. */
    int ega = strcasecmp(e->name, "BB1.CBM") == 0 || strcasecmp(e->name, "BB2.CBM") == 0;
    const uint8_t *table = assets_lut(ega ? 0 : assets_effective_lut(g_imgs, (int)(e - g_imgs)));
    if (!table) table = assets_lut(0);
    memset(g_work, 0, sizeof(g_work));
    /* Header x/y are hotspot offsets relative to where the game draws the image (WATER: -220,0;
       NEGBG*: 0,-19; TITLE: -4,-10), not screen positions. The viewer ignores them: full-screen
       images go at 0,0 (under the label bar); smaller ones are centred below it. */
    int px0 = w < SCREEN_W ? (SCREEN_W - w) / 2 : 0;
    int py0 = h <= SCREEN_H - 12 ? 12 + (SCREEN_H - 12 - h) / 2 : 0;
    for (int y = 0; y < h; y++) {
        int dy = py0 + y;
        if (dy < 0 || dy >= SCREEN_H) continue;
        for (int x = 0; x < w; x++) {
            int dx = px0 + x;
            if (dx < 0 || dx >= SCREEN_W) continue;
            uint8_t nib = px[y * w + x];          /* raw nibble */
            /* Header byte 8 is NOT a transparent nibble for full-screen images: MAP.CBM has 8=0x06
               and the game draws its 6172 nibble-6 pixels (VERIFIED vs RAM). Draw opaque until its
               meaning is known. (void)trans keeps the value for later. */
            (void)trans;
            g_work[dy * SCREEN_W + dx] = table[nib];   /* map via LUT at draw time */
        }
    }
    free(px);
    printf("showing %s (%dx%d at %d,%d, trans %d)\n", e->name, w, h, xo, yo, trans);
    ui_rect(g_work, 0, 0, SCREEN_W, 12, 0x1F);          /* top bar */
    char label[64];
    snprintf(label, sizeof(label), "%s  %dx%d%s", e->name, w, h, ega ? "  (EGA version)" : "");
    ui_text(g_work, 4, 2, label, 0x0F);                 /* image name label */
}

void game_init(void) {
    memset(g_fb, 0, sizeof(g_fb));
    load_palette("cmap.bin");
    if (assets_load_luts("OVL0.OVL") == 0)
        printf("CBM LUTs loaded from OVL0.OVL\n");
    if (assets_load_font("OVL0.OVL") == 0)
        printf("font loaded from OVL0.OVL (96 glyphs)\n");
    g_nimg = assets_scan_images(".", g_imgs, 256);
    printf("found %d CBM images\n", g_nimg);
    g_cur = 0;
    g_viewer = g_nimg > 0;
    if (g_viewer) blit_cbm(&g_imgs[g_cur]);
}

void game_frame(void) {
    /* viewer mode: static image, input handled in game_run */
}

/* Headless: render one CBM to a raw 320x200 palette-index file and exit
   (for screen-diffing against the real game; see HANDOFF.md). */
int game_dump_screen(const char *name, const char *outpath) {
    if (assets_load_luts("OVL0.OVL") != 0) return 1;
    img_entry_t imgs[256];
    int n = assets_scan_images(".", imgs, 256);
    for (int i = 0; i < n; i++) {
        if (strcasecmp(imgs[i].name, name) == 0) {
            int w, h, xo, yo, tr, lut = 0;
            uint8_t *px = assets_decode_cbm_file(imgs[i].dat, &imgs[i], &w, &h, &xo, &yo, &tr, &lut);
            if (!px) return 1;
            const uint8_t *t = assets_lut(assets_effective_lut(imgs, i)); if (!t) t = assets_lut(0);
            uint8_t fb[SCREEN_W * SCREEN_H];
            memset(fb, 0, sizeof(fb));
            for (int y = 0; y < h && yo + y < SCREEN_H; y++)
                for (int x = 0; x < w && xo + x < SCREEN_W; x++) {
                    uint8_t nib = px[y * w + x];
                    (void)tr;   /* opaque: header byte 8 is not transparency (see blit_cbm) */
                    if (yo + y >= 0 && xo + x >= 0)
                        fb[(yo + y) * SCREEN_W + (xo + x)] = t[nib];
                }
            FILE *f = fopen(outpath, "wb");
            if (!f) return 1;
            fwrite(fb, 1, sizeof(fb), f);
            fclose(f);
            free(px);
            printf("dumped %s -> %s (%dx%d lut %d)\n", name, outpath, w, h, lut);
            return 0;
        }
    }
    printf("image not found: %s\n", name);
    return 1;
}

/* Rome province screen (target: tools/dosbox/results/scr_rome_province.bin).
   Composed from the mapped primitives; coordinates from the verified capture. */
static void box(int x, int y, int w, int h, int sep_rows) {
    ui_rect(g_work, 1, y, 319, 1, 0);
    ui_rect(g_work, 2, y + 1, 318, 1, 8);
    ui_rect(g_work, x, y + 16, w, sep_rows, 27);
    ui_rect(g_work, x + 4, y + 16 + sep_rows, w - 8, h - 21 - sep_rows, 26);
    ui_rect(g_work, x, y + 16, 1, h - 16, 25);
    ui_rect(g_work, x + 1, y + 16, 3, h - 16, 30);
    ui_rect(g_work, x + w - 6, y + 16, 3, h - 16, 28);
    ui_rect(g_work, x + w - 3, y + 16, 1, h - 16, 0);
    ui_rect(g_work, x + w - 2, y + 16, 1, h - 16, 28);
    ui_rect(g_work, x + w - 1, y + 16, 1, h - 16, 27);
    ui_rect(g_work, x, y + h - 5, w, 3, 28);
    ui_rect(g_work, x, y + h - 2, w, 1, 0);
    ui_rect(g_work, x, y + h - 1, w, 1, 28);
    ui_rect(g_work, x, y + h, w, 1, 26);
}

/* ---- province screen, driven by game data (EXE func_0x6f0c + menu DS:573C) ---- */
static const uint8_t *g_ovl0;

static const char *ds_str(uint16_t ptr) {
    static char buf[128];
    const uint8_t *p = g_ovl0 + 0xB400 + ptr;
    int i = 0;
    while (p[i] && i < 127) { buf[i] = p[i]; i++; }
    buf[i] = 0;
    return buf;
}

typedef struct {
    const char *name;        /* province title */
    const char *strip;       /* CBM city strip (cities) or NULL */
    const char *bg, *fg;     /* layered strip: NEGBG?/NEGFG? (non-city) */
    int sky;                 /* sky fill color for layered strips */
    const char *const *items; /* NULL = read the 7 DS:573C records; else override list */
    const char *status, *courage, *rank;
    int tribute, manpower, talents, mood;  /* mood = func_0x6c26 index 0..3 */
} province_t;

static void draw_province(const province_t *p) {
    memset(g_work, 116, sizeof(g_work));
    /* strip: full city CBM, or layered sky+NEGBG+NEGFG (VERIFIED for Narbonensis) */
    {
        img_entry_t imgs[256];
        int n = assets_scan_images(".", imgs, 256);
        if (p->strip) {
            for (int i = 0; i < n; i++)
                if (strcasecmp(imgs[i].name, p->strip) == 0) { blit_cbm_at(&imgs[i], 0, 0); break; }
        } else {
            ui_rect(g_work, 0, 0, SCREEN_W, 31, p->sky);
            for (int i = 0; i < n; i++) {
                if (p->bg && strcasecmp(imgs[i].name, p->bg) == 0)
                    blit_cbm_at_trans(&imgs[i], 0, 19, 8);
                if (p->fg && strcasecmp(imgs[i].name, p->fg) == 0)
                    blit_cbm_at_trans(&imgs[i], 0, 31, 8);
            }
        }
    }
    ui_text_centered(g_work, 160, 66, p->name, 0);
    /* command menu: box + items from DS:573C records (12-byte {text,hk,x0,y0,x1,y1}) */
    box(6, 63, 151, 134, 2);
    ui_text(g_work, 27, 85, ds_str(0x5702), 15);   /* "What is your command?" */
    {
        char items[7][64];
        for (int i = 0; i < 7; i++) {
            const uint8_t *rec = g_ovl0 + 0xB400 + 0x573C + 12 * i;
            uint16_t text = rec[0] | (rec[1] << 8);
            if (p->items) {
                snprintf(items[i], sizeof(items[i]), "%s", p->items[i]);
            } else {
                snprintf(items[i], sizeof(items[i]), "%s", ds_str(text));
            }
            /* rect y0 is window-relative; window body starts at y=63+16+2=81, items at 81+23-... :
               measured item0 glyphs at y102 -> draw y = 102 - (23 - 23) ... use rect y0 mapped: y = 63 + 16 + 2 + y0 - 23 + 23 */
            int y0 = (int16_t)(rec[6] | (rec[7] << 8));
            ui_text(g_work, 6 + 17, 63 + 39 + (y0 - 23), items[i], 0);
        }
    }
    for (int y = 100; y < 172; y++)
        for (int x = 14; x <= 18; x += 2)
            g_work[y * SCREEN_W + x] = 8;
    /* status panel (func_0x6f0c): mood line centred (239,86), rest left x=171 */
    box(164, 63, 151, 134, 3);
    {
        static const char *moods[4] = {"Rebellious", "Angry", "Restless", "Content"};  /* DS:5734 */
        char line[128];
        snprintf(line, sizeof(line), "The people are %s", moods[p->mood & 3]);
        ui_text_centered(g_work, 239, 86, line, 0);
        snprintf(line, sizeof(line), "Status: %s", p->status);   ui_text(g_work, 171, 106, line, 0);
        snprintf(line, sizeof(line), "Tribute: %d", p->tribute); ui_text(g_work, 171, 117, line, 0);
        snprintf(line, sizeof(line), "Manpower: %d", p->manpower); ui_text(g_work, 171, 128, line, 0);
        snprintf(line, sizeof(line), "Courage: %s", p->courage); ui_text(g_work, 171, 139, line, 0);
        snprintf(line, sizeof(line), "Rank: %s", p->rank);       ui_text(g_work, 171, 156, line, 0);
        snprintf(line, sizeof(line), "Talents: %d", p->talents);  ui_text(g_work, 171, 167, line, 0);
        ui_text_centered(g_work, 239, 180, "Galley Slave level", 0);
    }
    /* crowd sprite by mood (INIT2 sheet entry = mood, VERIFIED sprite 0 = Rebellious) */
    if (assets_load("MINIT.DIR", "MINIT.DAT") == 0) {
        sprite_ent_t ents[16];
        int n = 0;
        uint8_t *pix = NULL;
        size_t npix = 0;
        if (assets_load_sheet("INIT2HDR.LZW", "INIT2DAT.LZW", ents, &n, &pix, &npix) == 0 && n > (p->mood & 3)) {
            const uint8_t *t = assets_lut(ents[p->mood & 3].lut ? ents[p->mood & 3].lut : 22);
            if (!t) t = assets_lut(22);
            assets_blit_sprite(pix, &ents[p->mood & 3], t, g_work, SCREEN_W, SCREEN_H, 255, 104);
            free(pix);
        }
    }
}

static void draw_province_rome(void) {
    static const char *const rome_items[7] = {"Legions", "Tribute", "Hold Games", "Build Fleet",
                                               "Tribute Policy", "See Province List", "Continue"};
    static const province_t rome = {"ROME", "ROME.CBM", NULL, NULL, 0, rome_items,
        "Citizens", "Fierce", "Officer", 8, 5000, 20, 0};
    draw_province(&rome);
}

/* Headless: render the current frame (work buffer) to a raw index file. */
int game_dump_frame(const char *outpath, const char *screen) {
    if (screen && strncasecmp(screen, "province", 8) == 0) {
        if (assets_load_luts("OVL0.OVL") != 0) return 1;
        if (assets_load_font("OVL0.OVL") != 0) return 1;
        FILE *f = fopen("OVL0.OVL", "rb");
        static uint8_t ovl0[0x20000];
        if (!f) return 1;
        fread(ovl0, 1, sizeof(ovl0), f);
        fclose(f);
        g_ovl0 = ovl0;
        if (strcasecmp(screen, "province_narbonensis") == 0) {
            static const province_t narb = {"Narbonensis", NULL, "NEGBGA.CBM", "NEGFGX.CBM", 23, NULL,
                "Occupied", "Good", "Officer", 0, 5279, 20, 0};
            draw_province(&narb);
        } else {
            draw_province_rome();
        }
    } else if (screen && strcasecmp(screen, "game_menu") == 0) {
        if (assets_load_font("OVL0.OVL") != 0) return 1;
        memset(g_work, 116, sizeof(g_work));
        box(6, 63, 151, 134, 2);
        ui_text(g_work, 27, 85, "Game Control Menu", 15);
    } else {
        game_init();
        game_frame();
    }
    FILE *f = fopen(outpath, "wb");
    if (!f) return 1;
    fwrite(g_work, 1, sizeof(g_work), f);
    fclose(f);
    printf("dumped frame -> %s\n", outpath);
    return 0;
}

void game_run(void) {
    game_init();
    input_t in;
    const uint32_t frame_ms = 1000 / FPS;
    uint32_t next = platform_ticks_ms();
    int last_key = -1;
    while (1) {
        platform_poll_input(&in);
        if (in.quit_requested) break;
        if (g_viewer && in.key_special[KEY_RIGHT] && last_key != KEY_RIGHT) {
            g_cur = (g_cur + 1) % g_nimg;
            blit_cbm(&g_imgs[g_cur]);
        }
        if (g_viewer && in.key_special[KEY_LEFT] && last_key != KEY_LEFT) {
            g_cur = (g_cur - 1 + g_nimg) % g_nimg;
            blit_cbm(&g_imgs[g_cur]);
        }
        last_key = in.key_special[KEY_RIGHT] ? KEY_RIGHT : (in.key_special[KEY_LEFT] ? KEY_LEFT : -1);
        game_frame();
        memcpy(g_fb, g_work, sizeof(g_fb));   /* blit work buffer -> screen */
        platform_present(g_fb, g_palette);
        next += frame_ms;
        uint32_t now = platform_ticks_ms();
        if (next > now) platform_sleep_ms(next - now);
        else next = now;
    }
}
