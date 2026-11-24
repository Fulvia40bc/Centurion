#ifndef PLATFORM_H
#define PLATFORM_H
#include <stdint.h>

#define SCREEN_W 320
#define SCREEN_H 200
#define FPS 60

typedef struct {
    uint8_t r, g, b;
} rgb_t;

#define KEY_LEFT  1
#define KEY_RIGHT 2

typedef struct {
    int key_down[256];    /* 1 = pressed; 2 = from the keypad: the BIOS's code as it is (no Shift table) */
    int key_special[8];   /* KEY_LEFT / KEY_RIGHT etc. */
    int mouse_x, mouse_y;
    uint8_t mouse_buttons;
    int quit_requested;
} input_t;

int  platform_init(void);
void platform_shutdown(void);
void platform_present(const uint8_t *framebuffer, const rgb_t palette[256]);
void platform_poll_input(input_t *in);
void platform_console(void);     /* Windows: started from a terminal, write the messages there (the exe has no console) */
void platform_warp(int x, int y);/* put the pointer on game pixel (x, y), if it is over the window */
int  platform_pointer(int on);   /* show (1) / hide (0) the window's system pointer; returns the previous state */
void platform_sleep_ms(uint32_t ms);
uint32_t platform_ticks_ms(void);
void platform_pump(void);        /* read the window's events into the queue without taking any (SDL_PumpEvents) */
/* the music's audio device (music.c): 49716 Hz mono 16-bit, fill() called on the audio thread; 0 = running */
int  platform_audio_open(void (*fill)(int16_t *buf, int n));
void platform_audio_lock(int on);   /* 1: hold the audio callback off (the driver's cli), 0: let it run again */

#endif
