#ifndef PLATFORM_H
#define PLATFORM_H

/* The port's platform layer (M1): one 640x480 8-bit picture, keyboard and mouse (pen and touch arrive as the mouse),
   one stereo audio stream, a millisecond clock. The SDL3 backend is plat_sdl3.c (OVERVIEW §7.3). With PLAT_HEADLESS
   nothing opens: plat_present() only keeps the frame, so a test can save it with plat_save_png(). */

#include <stdint.h>

#define SCREEN_W 640
#define SCREEN_H 480

enum { PLAT_HEADLESS = 1 };

/* Input events, in the order they happened. Keys carry what the PC BIOS gave the game (INT 16h): the set-1 scan code
   and the ASCII code for a US keyboard (0 for keys without one: arrows, F-keys, Alt+letter), plus the modifiers.
   Mouse positions are game pixels (0..639, 0..479); the letterbox bars clamp to the edge. */
enum { PLAT_KEY = 1, PLAT_MOUSE_MOVE, PLAT_MOUSE_DOWN, PLAT_MOUSE_UP, PLAT_QUIT };
enum { PLAT_MOD_SHIFT = 1, PLAT_MOD_CTRL = 2, PLAT_MOD_ALT = 4 };
enum { PLAT_BUTTON_LEFT = 1, PLAT_BUTTON_RIGHT = 2, PLAT_BUTTON_MIDDLE = 4 };

typedef struct {
    int type;
    uint8_t scan, ascii, mods;   /* PLAT_KEY */
    int x, y;                    /* mouse events: position in game pixels */
    int button;                  /* PLAT_MOUSE_DOWN / UP: one PLAT_BUTTON_* */
} plat_event;

int  plat_init(const char *title, int flags);   /* 0 ok; the message is on stderr otherwise */
void plat_shutdown(void);
void plat_console(void);   /* Windows: a GUI exe started from a terminal writes its messages there */

/* fb: SCREEN_W*SCREEN_H palette indices; pal: 256 RGB triples, 8 bits each (VGA's 6-bit values: v << 2 | v >> 4) */
void plat_present(const uint8_t *fb, const uint8_t pal[256][3]);
int  plat_save_png(const char *path);           /* the last presented frame, as an 8-bit indexed PNG; 0 ok */

int  plat_poll(plat_event *ev);                 /* 1 = one event taken, 0 = none waiting */
void plat_mouse(int *x, int *y, int *buttons);  /* where the mouse is now, and the PLAT_BUTTON_* bits held */
int  plat_pointer(int on);                      /* show / hide the system pointer; returns the previous state */

int  plat_chdir(const char *dir);              /* the original runs in the game folder and opens files relative to it */
const char *plat_exe_dir(void);                 /* the program's folder, ending in a separator ("" if unknown) */
int  plat_first_match(const char *dir, const char *pattern, char *out, int size);   /* dir/<first match in name
                                                   order, case-insensitive pattern> into out: 0, or -1 for none */

uint64_t plat_ticks_ms(void);
void plat_sleep_ms(uint32_t ms);

/* Audio: 44100 Hz, 16-bit, interleaved stereo. fill() runs on the audio thread; plat_audio_lock(1) holds it off while
   the game thread changes what it plays. Headless: nothing is opened and fill() is never called. */
enum { PLAT_RATE = 44100 };
int  plat_audio_open(void (*fill)(void *user, int16_t *out, int frames), void *user);
void plat_audio_lock(int on);

#endif
