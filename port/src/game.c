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
static void box(int x, int y, int w, int h) {
    /* measured from the capture: title band keeps the 116 background,
       body (y+18 .. y+h-6) is filled 26, with the game's 4px borders */
    ui_rect(g_work, x + 4, y + 18, w - 8, h - 24, 26);
    ui_rect(g_work, x, y, w, 1, 0);
    ui_rect(g_work, x, y + 1, w, 1, 8);
    ui_rect(g_work, x, y, 1, h, 25);
    ui_rect(g_work, x + 1, y, 3, h, 30);
    ui_rect(g_work, x + w - 6, y, 3, h, 28);
    ui_rect(g_work, x + w - 3, y, 1, h, 0);
    ui_rect(g_work, x + w - 2, y, 1, h, 28);
    ui_rect(g_work, x + w - 1, y, 1, h, 27);
    ui_rect(g_work, x, y + h - 5, w, 3, 28);
    ui_rect(g_work, x, y + h - 2, w, 1, 0);
    ui_rect(g_work, x, y + h - 1, w, 1, 28);
}

static void draw_province_rome(void) {
    memset(g_work, 116, sizeof(g_work));
    /* city strip ROME.CBM across the top */
    {
        img_entry_t imgs[256];
        int n = assets_scan_images(".", imgs, 256);
        for (int i = 0; i < n; i++)
            if (strcasecmp(imgs[i].name, "ROME.CBM") == 0) { blit_cbm_at(&imgs[i], 0, 0); break; }
    }
    ui_text(g_work, 140, 45, "ROME", 0);
    /* command menu box, 7 items */
    box(6, 63, 151, 135);
    ui_text(g_work, 10, 66, "What is your command?", 0);
    const char *items[] = {"Legions", "Tribute", "Hold Games", "Build Fleet",
                           "Tribute Policy", "See Province List", "Continue"};
    for (int i = 0; i < 7; i++)
        ui_text(g_work, 23, 104 + i * 10, items[i], 0);
    /* status panel */
    box(164, 63, 151, 135);
    ui_text(g_work, 168, 65, "The people are Rebellious", 0);
    ui_text(g_work, 168, 86, "Status: Citizens", 0);
    ui_text(g_work, 168, 106, "Tribute: 8", 0);
    ui_text(g_work, 168, 117, "Manpower: 5000", 0);
    ui_text(g_work, 168, 128, "Courage: Fierce", 0);
    ui_text(g_work, 168, 139, "Rank: Officer", 0);
    ui_text(g_work, 168, 156, "Talents: 20", 0);
    ui_text(g_work, 168, 167, "Galley Slave level", 0);
}

/* Headless: render the current frame (work buffer) to a raw index file. */
int game_dump_frame(const char *outpath, const char *screen) {
    if (screen && strcasecmp(screen, "province_rome") == 0) {
        if (assets_load_luts("OVL0.OVL") != 0) return 1;
        draw_province_rome();
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
