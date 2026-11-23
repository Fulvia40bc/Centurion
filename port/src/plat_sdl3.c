/* SDL3 backend of platform.h (OVERVIEW §7.3).
   The window shows the 640x480 picture at 4:3, letterboxed at any window size, scaled with SDL's pixel-art sampling
   (sharp pixels at in-between sizes, e.g. a 200% Windows display). Alt+Enter or F11 toggles full screen; the game
   never sees those keys. Pen and touch reach the game as the mouse (SDL's default). */
#include "platform.h"
#include "png_write.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

static int g_headless;
static SDL_Window *g_win;
static SDL_Renderer *g_ren;
static SDL_Texture *g_tex;
static int g_have_frame;
static uint8_t g_fb[SCREEN_W * SCREEN_H];   /* the last presented frame, for plat_save_png and redraws */
static uint8_t g_pal[256][3];
static SDL_AudioStream *g_audio;
static void (*g_fill)(void *user, int16_t *out, int frames);
static void *g_fill_user;

void plat_console(void)
{
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_ERROR_HANDLE);
    if ((h == NULL || h == INVALID_HANDLE_VALUE) && AttachConsole(ATTACH_PARENT_PROCESS)) {
        if (!freopen("CONOUT$", "w", stdout)) return;
        if (!freopen("CONOUT$", "w", stderr)) return;
    }
#endif
}

static void redraw(void)
{
    SDL_SetRenderDrawColor(g_ren, 0, 0, 0, 255);
    SDL_RenderClear(g_ren);
    if (g_have_frame)
        SDL_RenderTexture(g_ren, g_tex, NULL, NULL);
    SDL_RenderPresent(g_ren);
}

int plat_init(const char *title, int flags)
{
    SDL_DisplayID d;
    SDL_Rect ub;
    int w = SCREEN_W, h = SCREEN_H;

    g_headless = (flags & PLAT_HEADLESS) != 0;
    if (g_headless)
        return 0;
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        fprintf(stderr, "platform: SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }
    /* the window: the largest whole multiple of 640x480 (in Windows' scaled units) within 85% of the usable desktop */
    d = SDL_GetPrimaryDisplay();
    if (d != 0 && SDL_GetDisplayUsableBounds(d, &ub)) {
        int s = 1;
        while ((s + 1) * SCREEN_W <= ub.w * 85 / 100 && (s + 1) * SCREEN_H <= ub.h * 85 / 100)
            s++;
        w = SCREEN_W * s;
        h = SCREEN_H * s;
    }
    g_win = SDL_CreateWindow(title, w, h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (g_win == NULL) {
        fprintf(stderr, "platform: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetWindowMinimumSize(g_win, SCREEN_W / 2, SCREEN_H / 2);
    g_ren = SDL_CreateRenderer(g_win, NULL);
    if (g_ren == NULL) {
        fprintf(stderr, "platform: SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetRenderVSync(g_ren, 1);
    SDL_SetRenderLogicalPresentation(g_ren, SCREEN_W, SCREEN_H, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    g_tex = SDL_CreateTexture(g_ren, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_W, SCREEN_H);
    if (g_tex == NULL) {
        fprintf(stderr, "platform: SDL_CreateTexture failed: %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetTextureScaleMode(g_tex, SDL_SCALEMODE_PIXELART);
    redraw();
    return 0;
}

void plat_shutdown(void)
{
    if (g_audio != NULL)
        SDL_DestroyAudioStream(g_audio);
    g_audio = NULL;
    if (g_headless)
        return;
    if (g_tex != NULL) SDL_DestroyTexture(g_tex);
    if (g_ren != NULL) SDL_DestroyRenderer(g_ren);
    if (g_win != NULL) SDL_DestroyWindow(g_win);
    g_tex = NULL; g_ren = NULL; g_win = NULL;
    SDL_Quit();
}

void plat_present(const uint8_t *fb, const uint8_t pal[256][3])
{
    static uint32_t px[SCREEN_W * SCREEN_H];
    int i;

    memcpy(g_fb, fb, sizeof g_fb);
    memcpy(g_pal, pal, sizeof g_pal);
    g_have_frame = 1;
    if (g_headless)
        return;
    for (i = 0; i < SCREEN_W * SCREEN_H; i++) {
        const uint8_t *c = pal[fb[i]];
        px[i] = 0xFF000000u | (uint32_t)c[0] << 16 | (uint32_t)c[1] << 8 | c[2];
    }
    SDL_UpdateTexture(g_tex, NULL, px, SCREEN_W * 4);
    redraw();
}

int plat_save_png(const char *path)
{
    if (!g_have_frame)
        return -1;
    return png_write_indexed(path, g_fb, SCREEN_W, SCREEN_H, (const uint8_t (*)[3])g_pal);
}

static void to_game(float wx, float wy, int *x, int *y)
{
    float lx = wx, ly = wy;

    if (g_ren != NULL)
        SDL_RenderCoordinatesFromWindow(g_ren, wx, wy, &lx, &ly);
    *x = lx < 0 ? 0 : lx >= SCREEN_W ? SCREEN_W - 1 : (int)lx;
    *y = ly < 0 ? 0 : ly >= SCREEN_H ? SCREEN_H - 1 : (int)ly;
}

/* SDL's physical keys -> PC set-1 scan codes (G: the codes INT 16h returned on a US AT keyboard) */
static uint8_t scan_code(SDL_Scancode s)
{
    static const uint8_t letters[26] = { 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32,
                                         0x31, 0x18, 0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C };
    if (s >= SDL_SCANCODE_A && s <= SDL_SCANCODE_Z) return letters[s - SDL_SCANCODE_A];
    if (s >= SDL_SCANCODE_1 && s <= SDL_SCANCODE_0) return (uint8_t)(0x02 + (s - SDL_SCANCODE_1));
    if (s >= SDL_SCANCODE_F1 && s <= SDL_SCANCODE_F10) return (uint8_t)(0x3B + (s - SDL_SCANCODE_F1));
    switch (s) {
    case SDL_SCANCODE_ESCAPE: return 0x01;       case SDL_SCANCODE_MINUS: return 0x0C;
    case SDL_SCANCODE_EQUALS: return 0x0D;       case SDL_SCANCODE_BACKSPACE: return 0x0E;
    case SDL_SCANCODE_TAB: return 0x0F;          case SDL_SCANCODE_LEFTBRACKET: return 0x1A;
    case SDL_SCANCODE_RIGHTBRACKET: return 0x1B; case SDL_SCANCODE_RETURN: return 0x1C;
    case SDL_SCANCODE_KP_ENTER: return 0x1C;     case SDL_SCANCODE_SEMICOLON: return 0x27;
    case SDL_SCANCODE_APOSTROPHE: return 0x28;   case SDL_SCANCODE_GRAVE: return 0x29;
    case SDL_SCANCODE_BACKSLASH: return 0x2B;    case SDL_SCANCODE_COMMA: return 0x33;
    case SDL_SCANCODE_PERIOD: return 0x34;       case SDL_SCANCODE_SLASH: return 0x35;
    case SDL_SCANCODE_KP_MULTIPLY: return 0x37;  case SDL_SCANCODE_SPACE: return 0x39;
    case SDL_SCANCODE_HOME: return 0x47;         case SDL_SCANCODE_UP: return 0x48;
    case SDL_SCANCODE_PAGEUP: return 0x49;       case SDL_SCANCODE_KP_MINUS: return 0x4A;
    case SDL_SCANCODE_LEFT: return 0x4B;         case SDL_SCANCODE_RIGHT: return 0x4D;
    case SDL_SCANCODE_KP_PLUS: return 0x4E;      case SDL_SCANCODE_END: return 0x4F;
    case SDL_SCANCODE_DOWN: return 0x50;         case SDL_SCANCODE_PAGEDOWN: return 0x51;
    case SDL_SCANCODE_INSERT: return 0x52;       case SDL_SCANCODE_DELETE: return 0x53;
    case SDL_SCANCODE_F11: return 0x57;          case SDL_SCANCODE_F12: return 0x58;
    case SDL_SCANCODE_KP_7: return 0x47;         case SDL_SCANCODE_KP_8: return 0x48;
    case SDL_SCANCODE_KP_9: return 0x49;         case SDL_SCANCODE_KP_4: return 0x4B;
    case SDL_SCANCODE_KP_5: return 0x4C;         case SDL_SCANCODE_KP_6: return 0x4D;
    case SDL_SCANCODE_KP_1: return 0x4F;         case SDL_SCANCODE_KP_2: return 0x50;
    case SDL_SCANCODE_KP_3: return 0x51;         case SDL_SCANCODE_KP_0: return 0x52;
    case SDL_SCANCODE_KP_PERIOD: return 0x53;    case SDL_SCANCODE_KP_DIVIDE: return 0x35;
    default: return 0;
    }
}

/* The ASCII code a US keyboard's BIOS gave: Shift and Caps Lock for letters, Shift for the rest, Ctrl+letter = 1..26,
   Alt+anything = 0 (the game then reads the scan code). */
static uint8_t ascii_code(SDL_Keycode k, SDL_Keymod m)
{
    static const char plain[] = "`1234567890-=[]\\;',./";
    static const char shifted[] = "~!@#$%^&*()_+{}|:\"<>?";
    int shift = (m & SDL_KMOD_SHIFT) != 0;
    const char *p;

    if (m & SDL_KMOD_ALT)
        return 0;
    if (k >= SDLK_A && k <= SDLK_Z) {
        if (m & SDL_KMOD_CTRL)
            return (uint8_t)(k - SDLK_A + 1);
        return (uint8_t)((shift != ((m & SDL_KMOD_CAPS) != 0)) ? k - SDLK_A + 'A' : k);
    }
    if (m & SDL_KMOD_CTRL)
        return 0;
    if (k < 128 && k != 0 && (p = strchr(plain, (int)k)) != NULL)
        return (uint8_t)(shift ? shifted[p - plain] : *p);
    switch (k) {
    case SDLK_SPACE: return ' ';
    case SDLK_RETURN: case SDLK_KP_ENTER: return 13;
    case SDLK_ESCAPE: return 27;
    case SDLK_BACKSPACE: return 8;
    case SDLK_TAB: return shift ? 0 : 9;
    case SDLK_KP_PLUS: return '+';
    case SDLK_KP_MINUS: return '-';
    case SDLK_KP_MULTIPLY: return '*';
    case SDLK_KP_DIVIDE: return '/';
    default: break;
    }
    if (m & SDL_KMOD_NUM) {   /* the keypad digits only with Num Lock (else they are the cursor keys, ASCII 0) */
        if (k >= SDLK_KP_1 && k <= SDLK_KP_9) return (uint8_t)('1' + (k - SDLK_KP_1));
        if (k == SDLK_KP_0) return '0';
        if (k == SDLK_KP_PERIOD) return '.';
    }
    return 0;
}

static int button_bit(Uint8 b)
{
    return b == SDL_BUTTON_LEFT ? PLAT_BUTTON_LEFT : b == SDL_BUTTON_RIGHT ? PLAT_BUTTON_RIGHT
         : b == SDL_BUTTON_MIDDLE ? PLAT_BUTTON_MIDDLE : 0;
}

int plat_poll(plat_event *ev)
{
    SDL_Event e;

    if (g_headless)
        return 0;
    while (SDL_PollEvent(&e)) {
        memset(ev, 0, sizeof *ev);
        switch (e.type) {
        case SDL_EVENT_QUIT:
            ev->type = PLAT_QUIT;
            return 1;
        case SDL_EVENT_WINDOW_EXPOSED:
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            redraw();   /* the game may be waiting for input: show its last frame at the new size */
            break;
        case SDL_EVENT_KEY_DOWN:
            if (e.key.key == SDLK_F11 || (e.key.key == SDLK_RETURN && (e.key.mod & SDL_KMOD_ALT))) {
                if (!e.key.repeat)
                    SDL_SetWindowFullscreen(g_win, (SDL_GetWindowFlags(g_win) & SDL_WINDOW_FULLSCREEN) == 0);
                break;
            }
            ev->type = PLAT_KEY;
            ev->scan = scan_code(e.key.scancode);
            ev->ascii = ascii_code(e.key.key, e.key.mod);
            ev->mods = (uint8_t)(((e.key.mod & SDL_KMOD_SHIFT) ? PLAT_MOD_SHIFT : 0) |
                                 ((e.key.mod & SDL_KMOD_CTRL) ? PLAT_MOD_CTRL : 0) |
                                 ((e.key.mod & SDL_KMOD_ALT) ? PLAT_MOD_ALT : 0));
            if (ev->scan == 0 && ev->ascii == 0)
                break;   /* a key the PC keyboard did not have (or a lone modifier) */
            return 1;
        case SDL_EVENT_MOUSE_MOTION:
            ev->type = PLAT_MOUSE_MOVE;
            to_game(e.motion.x, e.motion.y, &ev->x, &ev->y);
            return 1;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            ev->button = button_bit(e.button.button);
            if (ev->button == 0)
                break;
            ev->type = e.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? PLAT_MOUSE_DOWN : PLAT_MOUSE_UP;
            to_game(e.button.x, e.button.y, &ev->x, &ev->y);
            return 1;
        default:
            break;
        }
    }
    return 0;
}

void plat_mouse(int *x, int *y, int *buttons)
{
    float wx = 0, wy = 0;
    SDL_MouseButtonFlags b;

    if (g_headless) {
        *x = *y = *buttons = 0;
        return;
    }
    b = SDL_GetMouseState(&wx, &wy);
    to_game(wx, wy, x, y);
    *buttons = ((b & SDL_BUTTON_LMASK) ? PLAT_BUTTON_LEFT : 0) | ((b & SDL_BUTTON_RMASK) ? PLAT_BUTTON_RIGHT : 0) |
               ((b & SDL_BUTTON_MMASK) ? PLAT_BUTTON_MIDDLE : 0);
}

int plat_pointer(int on)
{
    int prev;

    if (g_headless)
        return 0;
    prev = SDL_CursorVisible() ? 1 : 0;
    if (on)
        SDL_ShowCursor();
    else
        SDL_HideCursor();
    return prev;
}

int plat_chdir(const char *dir)
{
#ifdef _WIN32
    return _chdir(dir);
#else
    return chdir(dir);
#endif
}

int plat_getcwd(char *buf, int size)
{
#ifdef _WIN32
    return _getcwd(buf, size) != NULL ? 0 : -1;
#else
    return getcwd(buf, (size_t)size) != NULL ? 0 : -1;
#endif
}

uint64_t plat_ticks_ms(void) { return SDL_GetTicks(); }
void plat_sleep_ms(uint32_t ms) { SDL_Delay(ms); }

/* SDL asks for `additional` more bytes; fill() makes them in pieces of up to 1024 frames */
static void SDLCALL audio_cb(void *ud, SDL_AudioStream *s, int additional, int total)
{
    int16_t buf[1024 * 2];

    (void)ud;
    (void)total;
    while (additional > 0) {
        int frames = additional / 4 < 1024 ? additional / 4 : 1024;
        if (frames <= 0)
            frames = 1;
        g_fill(g_fill_user, buf, frames);
        SDL_PutAudioStreamData(s, buf, frames * 4);
        additional -= frames * 4;
    }
}

int plat_audio_open(void (*fill)(void *user, int16_t *out, int frames), void *user)
{
    SDL_AudioSpec spec;

    if (g_headless)
        return 0;
    spec.format = SDL_AUDIO_S16;
    spec.channels = 2;
    spec.freq = PLAT_RATE;
    g_fill = fill;
    g_fill_user = user;
    g_audio = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, audio_cb, NULL);
    if (g_audio == NULL) {
        fprintf(stderr, "platform: no audio device (%s): no sound\n", SDL_GetError());
        return -1;
    }
    SDL_ResumeAudioStreamDevice(g_audio);
    return 0;
}

void plat_audio_lock(int on)
{
    if (g_audio == NULL)
        return;
    if (on)
        SDL_LockAudioStream(g_audio);
    else
        SDL_UnlockAudioStream(g_audio);
}
