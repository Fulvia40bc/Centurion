#include "platform.h"
#include <SDL2/SDL.h>
#include <string.h>

static SDL_Window *g_win;
static SDL_Renderer *g_ren;
static SDL_Texture *g_tex;
static SDL_AudioDeviceID g_audio;
static uint32_t g_audio_freq;
static volatile int g_audio_on;
static volatile uint32_t g_audio_until;

static void audio_callback(void *ud, uint8_t *stream, int len) {
    (void)ud;
    uint32_t now = platform_ticks_ms();
    int16_t *out = (int16_t *)stream;
    int frames = len / 2;
    if (!g_audio_on || now >= g_audio_until) {
        memset(stream, 0, len);
        return;
    }
    static uint32_t phase = 0;
    for (int i = 0; i < frames; i++) {
        int16_t v = (phase < g_audio_freq / 2) ? 8000 : -8000;
        out[i] = v;
        phase = (phase + 1) % g_audio_freq;
    }
}

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
    g_tex = SDL_CreateTexture(g_ren, SDL_PIXELFORMAT_RGB24,
                              SDL_TEXTUREACCESS_STREAMING, SCREEN_W, SCREEN_H);
    if (!g_tex) return -1;
    SDL_AudioSpec want = {0};
    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = audio_callback;
    g_audio = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
    if (g_audio) SDL_PauseAudioDevice(g_audio, 0);
    return 0;
}

void platform_shutdown(void) {
    if (g_audio) SDL_CloseAudioDevice(g_audio);
    if (g_tex) SDL_DestroyTexture(g_tex);
    if (g_ren) SDL_DestroyRenderer(g_ren);
    if (g_win) SDL_DestroyWindow(g_win);
    SDL_Quit();
}

void platform_present(const uint8_t *fb, const rgb_t palette[256]) {
    uint32_t pixels[SCREEN_W * SCREEN_H];
    for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
        rgb_t c = palette[fb[i]];
        pixels[i] = (c.r << 16) | (c.g << 8) | c.b;
    }
    SDL_UpdateTexture(g_tex, NULL, pixels, SCREEN_W * 3);
    SDL_RenderClear(g_ren);
    SDL_RenderCopy(g_ren, g_tex, NULL, NULL);
    SDL_RenderPresent(g_ren);
}

void platform_poll_input(input_t *in) {
    memset(in, 0, sizeof(*in));
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) in->quit_requested = 1;
        else if (e.type == SDL_KEYDOWN && e.key.keysym.sym < 256)
            in->key_down[e.key.keysym.sym] = 1;
        else if (e.type == SDL_MOUSEMOTION) {
            in->mouse_x = e.motion.x / 2;
            in->mouse_y = e.motion.y / 2;
        } else if (e.type == SDL_MOUSEBUTTONDOWN)
            in->mouse_buttons |= 1 << (e.button.button - 1);
    }
}

void platform_sleep_ms(uint32_t ms) { SDL_Delay(ms); }
uint32_t platform_ticks_ms(void) { return SDL_GetTicks(); }

void audio_play_square(uint32_t freq_hz, uint32_t duration_ms) {
    if (!g_audio || freq_hz == 0) return;
    g_audio_freq = freq_hz;
    g_audio_on = 1;
    g_audio_until = SDL_GetTicks() + duration_ms;
}
