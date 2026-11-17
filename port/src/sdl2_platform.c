#include "platform.h"
#include "music.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>

static SDL_Window *g_win;
static SDL_Renderer *g_ren;
static SDL_Texture *g_tex;
static SDL_AudioDeviceID g_audio;            /* the music's device (music.c), opened by platform_audio_open only */
static void (*g_fill)(int16_t *buf, int n);

static void audio_callback(void *ud, uint8_t *stream, int len) {
    (void)ud;
    g_fill((int16_t *)stream, len / 2);
}

/* 49716 Hz (the OPL2's own rate), mono, 16-bit: SDL converts to the hardware's format. The callback runs on SDL's audio
   thread with the device locked; platform_audio_lock takes the same lock from the game thread. */
int platform_audio_open(void (*fill)(int16_t *buf, int n)) {
    SDL_AudioSpec want;
    memset(&want, 0, sizeof want);
    want.freq = 49716;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = audio_callback;
    g_fill = fill;
    g_audio = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
    if (!g_audio) { fprintf(stderr, "audio: no device (%s): no music\n", SDL_GetError()); return -1; }
    SDL_PauseAudioDevice(g_audio, 0);
    return 0;
}
void platform_audio_lock(int on) {
    if (!g_audio) return;
    if (on) SDL_LockAudioDevice(g_audio); else SDL_UnlockAudioDevice(g_audio);
}
void platform_pump(void) { SDL_PumpEvents(); }

int platform_init(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0)
        return -1;
    g_win = SDL_CreateWindow("Centurion: Defender of Rome (port)",
                             SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                             SCREEN_W * 2, SCREEN_H * 2, SDL_WINDOW_SHOWN);
    if (!g_win) return -1;
    g_ren = SDL_CreateRenderer(g_win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_ren) g_ren = SDL_CreateRenderer(g_win, -1, SDL_RENDERER_SOFTWARE);
    if (!g_ren) return -1;
    g_tex = SDL_CreateTexture(g_ren, SDL_PIXELFORMAT_RGB888,
                              SDL_TEXTUREACCESS_STREAMING, SCREEN_W, SCREEN_H);
    if (!g_tex) return -1;
    return 0;
}

void platform_shutdown(void) {
    if (g_audio) SDL_CloseAudioDevice(g_audio);
    g_audio = 0;
    if (g_tex) SDL_DestroyTexture(g_tex);
    if (g_ren) SDL_DestroyRenderer(g_ren);
    if (g_win) SDL_DestroyWindow(g_win);
    SDL_Quit();
}

void platform_present(const uint8_t *fb, const rgb_t palette[256]) {
    static uint32_t pixels[SCREEN_W * SCREEN_H];   /* static: 256 KB, too big for stack */
    for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
        rgb_t c = palette[fb[i]];
        pixels[i] = 0xFF000000u | ((uint32_t)c.r << 16) | ((uint32_t)c.g << 8) | c.b;
    }
    SDL_UpdateTexture(g_tex, NULL, pixels, SCREEN_W * 4);
    SDL_RenderClear(g_ren);
    SDL_RenderCopy(g_ren, g_tex, NULL, NULL);
    SDL_RenderPresent(g_ren);
}

/* The keypad keys (keysyms >= 256) as the BIOS gives them to INT 16h (AL): + - * / always (Shift or not), the digits and
   '.' only with Num Lock xor Shift (else they are the cursor keys, AL = 0, which 0x03AA drops); 0 = no ASCII code. The
   caller marks them 2 in key_down, so screen.c does not shift them again (G: the AT BIOS's keypad, not run in a window) */
static int kp_ascii(SDL_Keycode k, Uint16 mod) {
    switch (k) {
    case SDLK_KP_PLUS: return '+';
    case SDLK_KP_MINUS: return '-';
    case SDLK_KP_MULTIPLY: return '*';
    case SDLK_KP_DIVIDE: return '/';
    default: break;
    }
    if (!(mod & KMOD_NUM) == !(mod & KMOD_SHIFT)) return 0;
    if (k == SDLK_KP_0) return '0';
    if (k >= SDLK_KP_1 && k <= SDLK_KP_9) return '1' + (int)(k - SDLK_KP_1);
    return k == SDLK_KP_PERIOD ? '.' : 0;
}

int platform_pointer(int on) {
    int prev = SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE;
    SDL_ShowCursor(on ? SDL_ENABLE : SDL_DISABLE);
    return prev;
}

void platform_poll_input(input_t *in) {
    memset(in, 0, sizeof(*in));
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) in->quit_requested = 1;
        if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_S && (e.key.keysym.mod & KMOD_CTRL)) {
            /* by the physical key, as 03FF tests make code 1Fh (any keyboard layout) */
            music_ctrl_s_03FF();             /* the INT 9 hook 03FF: Ctrl+S mutes / unmutes the music; 03AA drops the */
            continue;                        /* key (13h), so no reader sees it */
        }
        else if (e.type == SDL_KEYDOWN && e.key.keysym.sym < 256) {
            in->key_down[e.key.keysym.sym] = 1;
            if (e.key.keysym.sym == '=' && (e.key.keysym.mod & KMOD_SHIFT)) in->key_down['+'] = 1;   /* US '+' */
        }
        else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_INSERT)
            in->key_down[0x10] = 1;          /* the EXE's key reader 03AA turns Insert (scan 52h) into 10h */
        else if (e.type == SDL_KEYDOWN && kp_ascii(e.key.keysym.sym, e.key.keysym.mod))
            in->key_down[kp_ascii(e.key.keysym.sym, e.key.keysym.mod)] = 2;
        else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_KP_0)
            in->key_down[0x10] = 1;          /* keypad 0 without Num Lock is Insert (scan 52h) */
        else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_KP_ENTER)
            in->key_down[13] = 1;
        else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RIGHT)
            in->key_special[KEY_RIGHT] = 1;
        else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_LEFT)
            in->key_special[KEY_LEFT] = 1;
        else if (e.type == SDL_MOUSEBUTTONDOWN)
            in->mouse_buttons |= 1 << (e.button.button - 1);   /* a press shorter than a frame still counts */
    }
    /* position and held buttons every frame (drags need them), scaled from the window to 320x200 */
    int mx, my, ww = SCREEN_W * 2, wh = SCREEN_H * 2;
    uint32_t b = SDL_GetMouseState(&mx, &my);
    if (g_win) SDL_GetWindowSize(g_win, &ww, &wh);
    in->mouse_x = ww > 0 ? mx * SCREEN_W / ww : 0;
    in->mouse_y = wh > 0 ? my * SCREEN_H / wh : 0;
    if (b & SDL_BUTTON_LMASK) in->mouse_buttons |= 1;
    if (b & SDL_BUTTON_RMASK) in->mouse_buttons |= 4;
}

void platform_sleep_ms(uint32_t ms) { SDL_Delay(ms); }
uint32_t platform_ticks_ms(void) { return SDL_GetTicks(); }
