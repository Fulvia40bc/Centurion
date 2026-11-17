#include "platform.h"
#include "assets.h"
#include "ui.h"
#include "compat.h"
#include "state.h"
#include "screen.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* The image viewer (no arguments), the headless dumps (--dump-screen / --dump-frame / --end-turn / --save-game) and the
   province screens' dump frames. The game itself (--play / --load) is main.c play_game -> map.c map_loop. */
static uint8_t g_fb[SCREEN_W * SCREEN_H];
void map_startup(void);              /* map.c */
static const char *g_save_path;      /* --save of --dump-frame */
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

/* the OVL0 file, for the menu record of the game_menu dump */
static const uint8_t *g_ovl0;

/* load a DS menu record from OVL0 into a menu_t */
static void load_menu(menu_t *m, uint16_t rec_ds) {
    ui_menu_load(m, g_ovl0, rec_ds);
}



/* ---- save load/write + End Turn economy (roadmap items 2-3) ----
   Save = 0x38 header + state blocks. End Turn (VERIFIED diff fresh_275BC -> turn2_274BC,
   42 bytes): turn counter CSV 0x5FE 0->1; every province record +0x19 0->1; Rome record
   manpower +0x04 5000->5175, +0x08 0->32, +0x0E 10->5, +0x10 0->1; CSV 0x3A3 75->77,
   0x5F9 1->6, 0x5FA 20->28, 0x6AA 0->108. (Single observation - formulas UNVERIFIED.) */


/* one End Turn: apply the observed fresh->turn2 byte transforms */
/* headless: centurion --end-turn in.csv out.csv  (load, one End Turn, write) */
int game_end_turn(const char *in_path, const char *out_path) {
    /* real End Turn (EXE 0x09BA, endturn.c) on the DS image; replaces the earlier placeholder */
    /* in_path may also be a 640K memory snapshot (.bin) taken at End Turn entry: it carries the dice state */
    size_t len = strlen(in_path);
    int mem = len > 4 && strcasecmp(in_path + len - 4, ".bin") == 0;
    if (state_init("OVL0.OVL") != 0) { fprintf(stderr, "cannot read OVL0.OVL\n"); return 1; }
    if ((mem ? state_load_mem(in_path) : state_load(in_path)) != 0) { fprintf(stderr, "cannot read %s\n", in_path); return 1; }
    end_turn();
    if (g_enemy_unported) fprintf(stderr, "end turn: an unported path was reached; the result is not exact\n");
    size_t olen = strlen(out_path);
    if (olen > 4 && strcasecmp(out_path + olen - 4, ".bin") == 0) {   /* the whole DS image, for comparing with a snapshot */
        FILE *f = fopen(out_path, "wb");
        if (!f || fwrite(g_ds, 1, sizeof(g_ds), f) != sizeof(g_ds)) { if (f) fclose(f); fprintf(stderr, "cannot write %s\n", out_path); return 1; }
        fclose(f);
    } else if (state_save(out_path) != 0) { fprintf(stderr, "cannot write %s\n", out_path); return 1; }
    printf("end turn: %s -> %s\n", in_path, out_path);
    return 0;
}

/* headless: centurion --save-game in.csv out.csv  (load + write unchanged: round-trip check) */
int game_save_game(const char *in_path, const char *out_path) {
    /* load into the DS image and write it back through the save block table (state.c) */
    if (state_init("OVL0.OVL") != 0) { fprintf(stderr, "cannot read OVL0.OVL\n"); return 1; }
    if (state_load(in_path) != 0) { fprintf(stderr, "cannot read save %s\n", in_path); return 1; }
    if (state_save(out_path) != 0) { fprintf(stderr, "cannot write %s\n", out_path); return 1; }
    printf("saved: %s -> %s\n", in_path, out_path);
    return 0;
}

/* The map screen from a save, as the game draws it: static DS (state_init) + the save (state_load; without --save the
   first of 3.CSV / 0.CSV in the game folder that exists, else the static DS; the checks pass --save), the start-up writes
   (map_startup), then map_build(FFh) (0x494A); the screen goes to g_work for the dump. Headless. */
static int dump_map_ds(const char *save_path) {
    if (state_init("OVL0.OVL") != 0) return 1;
    if (save_path) { if (state_load(save_path) != 0) return 1; }
    else if (state_load("3.CSV") != 0) state_load("0.CSV");
    if (screen_open(1) != 0) return 1;
    map_startup();
    map_build(0xFF);
    memcpy(g_work, g_scr, sizeof(g_work));
    return 0;
}

int game_dump_frame(const char *outpath, const char *screen, const char *save_path) {
    g_save_path = save_path;  /* used by the map fleet loader */
    if (screen && strncasecmp(screen, "province", 8) == 0) {
        /* the game's own screen (province.c 0x7366 / 0x6CB8 up to their first menu wait) on the DS of the save (a .bin =
           a 640K snapshot or 64K DS image, dice included): "province_rome" = zone 0, "province_narbonensis" = 24,
           "provinceN" = N. (Until 2026-11-17 this drew a hand-built copy of the screen: docs/FRAME_DIFF_NOTES.md.) */
        int z = strcasecmp(screen, "province_narbonensis") == 0 ? 24 : atoi(screen + 8);
        size_t n = save_path ? strlen(save_path) : 0;
        if (state_init("OVL0.OVL") != 0) return 1;
        if (n > 4 && strcasecmp(save_path + n - 4, ".bin") == 0) { if (state_load_mem(save_path) != 0) return 1; }
        else if (save_path && state_load(save_path) != 0) return 1;
        if (screen_open(1) != 0) return 1;
        map_startup();
        province_dump(z);
        memcpy(g_work, g_scr, sizeof(g_work));
    } else if (screen && strcasecmp(screen, "map") == 0) {
        /* the map screen as the game builds it (map.c map_build 0x494A) from the DS of the save */
        if (dump_map_ds(g_save_path) != 0) return 1;
    } else if (screen && strcasecmp(screen, "game_menu") == 0) {
        /* the map, then the Game Control Menu record DS:05A6 drawn over it (0x5614 mode 2) */
        if (dump_map_ds(g_save_path) != 0) return 1;
        {
            FILE *f = fopen("OVL0.OVL", "rb");
            static uint8_t ovl0[0x20000];
            if (!f) return 1;
            if (fread(ovl0, 1, sizeof(ovl0), f) == 0) { fclose(f); return 1; }
            fclose(f);
            g_ovl0 = ovl0;
            menu_t m;
            load_menu(&m, 0x5A6);   /* Game Control Menu record */
            ui_menu_draw(g_work, &m, -1);
        }
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


/* --play used to run a prototype loop here; the game is now main.c play_game (map_startup, then map_loop: map.c,
   0x07EF..0x09B9) */

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
