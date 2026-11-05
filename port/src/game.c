#include "platform.h"
#include "assets.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static uint8_t g_fb[SCREEN_W * SCREEN_H];
static rgb_t g_palette[256];

void game_init(void) {
    for (int i = 0; i < 256; i++) {
        g_palette[i].r = i;
        g_palette[i].g = i;
        g_palette[i].b = i;
    }
    memset(g_fb, 0, sizeof(g_fb));
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
}

void game_frame(void) {
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
