#include "platform.h"
#include "assets.h"
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

static void draw_text(uint8_t *fb, int x, int y, const char *text, uint8_t color) {
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

static void blit_cbm(const img_entry_t *e) {
    int w, h, xo, yo, trans;
    uint8_t *px = assets_decode_cbm_file(e->dat, e, &w, &h, &xo, &yo, &trans);
    if (!px) return;
    memset(g_work, 0, sizeof(g_work));
    for (int y = 0; y < h; y++) {
        int dy = yo + y;
        if (dy < 0 || dy >= SCREEN_H) continue;
        for (int x = 0; x < w; x++) {
            int dx = xo + x;
            if (dx < 0 || dx >= SCREEN_W) continue;
            uint8_t p = px[y * w + x];
            if (p == (uint8_t)trans) continue;   /* transparent nibble */
            g_work[dy * SCREEN_W + dx] = p;
        }
    }
    free(px);
    printf("showing %s (%dx%d at %d,%d, trans %d)\n", e->name, w, h, xo, yo, trans);
    draw_text(g_fb, 4, 4, e->name, 0x0F);   /* label: white text on the image */
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
