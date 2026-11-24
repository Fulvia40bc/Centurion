#include "platform.h"
#include "music.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

/* The window: 320x200 shown at 4:3 (a 1990 VGA monitor stretched the 200 lines to the 4:3 tube), letterboxed in black at
   any window size; Alt+Enter or F11 toggles full screen (the desktop's resolution). LOGICAL_H = the picture's height in
   the 4:3 logical space the renderer scales. */
#define LOGICAL_H 240
static SDL_Window *g_win;
static SDL_Renderer *g_ren;
static SDL_Texture *g_tex;
static int g_have_frame;                     /* the texture holds a frame (redrawn after a resize / full-screen switch) */

/* The exe is a GUI program, so a double-click opens no console window. Started from a terminal without redirection, it
   attaches to that terminal so the error messages still show (redirected output needs nothing). */
void platform_console(void) {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_ERROR_HANDLE);
    if ((h == NULL || h == INVALID_HANDLE_VALUE) && AttachConsole(ATTACH_PARENT_PROCESS)) {
        if (!freopen("CONOUT$", "w", stdout)) return;
        if (!freopen("CONOUT$", "w", stderr)) return;
    }
#endif
}

static void present_texture(void) {
    SDL_SetRenderDrawColor(g_ren, 0, 0, 0, 255);
    SDL_RenderClear(g_ren);
    if (g_have_frame) SDL_RenderCopy(g_ren, g_tex, NULL, NULL);
    SDL_RenderPresent(g_ren);
}

static void toggle_fullscreen(void) {
    int fs = (SDL_GetWindowFlags(g_win) & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP;
    SDL_SetWindowFullscreen(g_win, fs ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
    present_texture();
}

/* 320x200 game coordinates <-> window coordinates, through the renderer's letterboxed 4:3 logical space */
static void window_to_game(int wx, int wy, int *x, int *y) {
    float lx, ly;
    SDL_RenderWindowToLogical(g_ren, wx, wy, &lx, &ly);
    int gx = (int)lx, gy = (int)(ly * SCREEN_H / LOGICAL_H);
    *x = gx < 0 ? 0 : gx >= SCREEN_W ? SCREEN_W - 1 : gx;
    *y = gy < 0 ? 0 : gy >= SCREEN_H ? SCREEN_H - 1 : gy;
}
void platform_warp(int x, int y) {
    if (!g_win || SDL_GetMouseFocus() != g_win) return;   /* only while the pointer is over the game's window */
    int wx, wy;
    SDL_RenderLogicalToWindow(g_ren, x + 0.5f, (y + 0.5f) * LOGICAL_H / SCREEN_H, &wx, &wy);
    SDL_WarpMouseInWindow(g_win, wx, wy);
}
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
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");   /* sharp pixels on a scaled (high-DPI) screen */
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0)
        return -1;
    /* the largest whole multiple of 320x240 that takes at most 3/4 of the usable desktop, at least 2x */
    int s = 2;
    SDL_Rect ub;
    if (SDL_GetDisplayUsableBounds(0, &ub) == 0)
        while ((s + 1) * SCREEN_W <= ub.w * 3 / 4 && (s + 1) * LOGICAL_H <= ub.h * 3 / 4) s++;
    g_win = SDL_CreateWindow("Centurion: Defender of Rome", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                             SCREEN_W * s, LOGICAL_H * s, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!g_win) return -1;
    SDL_SetWindowMinimumSize(g_win, SCREEN_W, LOGICAL_H);
    g_ren = SDL_CreateRenderer(g_win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_ren) g_ren = SDL_CreateRenderer(g_win, -1, SDL_RENDERER_SOFTWARE);
    if (!g_ren) return -1;
    SDL_RenderSetLogicalSize(g_ren, SCREEN_W, LOGICAL_H);
    g_tex = SDL_CreateTexture(g_ren, SDL_PIXELFORMAT_RGB888,
                              SDL_TEXTUREACCESS_STREAMING, SCREEN_W, SCREEN_H);
    if (!g_tex) return -1;
    present_texture();
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
    g_have_frame = 1;
    present_texture();
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
        if (e.type == SDL_WINDOWEVENT && (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
                                          e.window.event == SDL_WINDOWEVENT_EXPOSED)) {
            present_texture();               /* the game may be waiting for input: show its last frame at the new size */
            continue;
        }
        if (e.type == SDL_KEYDOWN && (e.key.keysym.sym == SDLK_F11 ||
                                      (e.key.keysym.sym == SDLK_RETURN && (e.key.keysym.mod & KMOD_ALT)))) {
            if (!e.key.repeat) toggle_fullscreen();   /* the port's own keys: the game never sees them */
            continue;
        }
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
    /* position and held buttons every frame (drags need them), from the window to 320x200 (the black bars clamp) */
    int mx, my;
    uint32_t b = SDL_GetMouseState(&mx, &my);
    if (g_ren) window_to_game(mx, my, &in->mouse_x, &in->mouse_y);
    if (b & SDL_BUTTON_LMASK) in->mouse_buttons |= 1;
    if (b & SDL_BUTTON_RMASK) in->mouse_buttons |= 4;
}

void platform_sleep_ms(uint32_t ms) { SDL_Delay(ms); }
uint32_t platform_ticks_ms(void) { return SDL_GetTicks(); }
