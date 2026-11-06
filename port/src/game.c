#include "platform.h"
#include "assets.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static uint8_t g_fb[SCREEN_W * SCREEN_H];
static rgb_t g_palette[256];
static int g_static_image;

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
            g_palette[i].r = (uint8_t)((cmap[i * 3] & 0x3F) << 2);
            g_palette[i].g = (uint8_t)((cmap[i * 3 + 1] & 0x3F) << 2);
            g_palette[i].b = (uint8_t)((cmap[i * 3 + 2] & 0x3F) << 2);
        }
        printf("palette: %s (250 x 6-bit RGB)\n", path);
    } else {
        printf("palette %s not found, using grayscale\n", path);
    }
}

void game_init(void) {
    memset(g_fb, 0, sizeof(g_fb));
    load_palette("cmap.bin");
    if (assets_load("CENT.DIR", "CENT.DAT") == 0) {
        int idx = assets_find("newgame.lzw");
        if (idx >= 0) {
            size_t n;
            uint8_t *d = assets_decode(idx, &n);
            printf("asset pipeline OK: newgame.lzw -> %zu bytes\n", n);
            free(d);
        }
    } else {
        printf("assets not found (run from the game data directory)\n");
    }
    /* First visible milestone: show the captured map screen (320x200, mode 13h) */
    if (load_file("screen_map.bin", g_fb, sizeof(g_fb)) == 0) {
        g_static_image = 1;
        printf("loaded screen_map.bin (320x200 framebuffer)\n");
    } else {
        g_static_image = 0;
        printf("screen_map.bin not found, using test pattern\n");
    }
}

void game_frame(void) {
    if (g_static_image) return; /* static map frame */
    static int x = 0;
    memset(g_fb, 0, sizeof(g_fb));
    for (int i = 0; i < 40; i++)
        g_fb[(80 + i) * SCREEN_W + x] = 0x0f;
    x = (x + 1) % SCREEN_W;
}

void game_run(void) {
    game_init();
    input_t in;
    const uint32_t frame_ms = 1000 / FPS;
    uint32_t next = platform_ticks_ms();
    while (1) {
        platform_poll_input(&in);
        if (in.quit_requested) break;
        game_frame();
        platform_present(g_fb, g_palette);
        next += frame_ms;
        uint32_t now = platform_ticks_ms();
        if (next > now) platform_sleep_ms(next - now);
        else next = now;
    }
}
