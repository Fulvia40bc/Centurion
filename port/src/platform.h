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
    int key_down[256];
    int key_special[8];   /* KEY_LEFT / KEY_RIGHT etc. */
    int mouse_x, mouse_y;
    uint8_t mouse_buttons;
    int quit_requested;
} input_t;

int  platform_init(void);
void platform_shutdown(void);
void platform_present(const uint8_t *framebuffer, const rgb_t palette[256]);
void platform_poll_input(input_t *in);
void platform_sleep_ms(uint32_t ms);
uint32_t platform_ticks_ms(void);
void audio_play_square(uint32_t freq_hz, uint32_t duration_ms);

#endif
