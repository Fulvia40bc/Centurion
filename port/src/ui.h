#ifndef UI_H
#define UI_H
#include <stdint.h>
#include "platform.h"

/* UI primitives mirroring the original EXE (see docs/MAINLOOP.md):
   all draw into the offscreen work buffer; the frame loop blits it to screen. */
void ui_rect(uint8_t *fb, int x, int y, int w, int h, uint8_t color);   /* 0x7E74 */
void ui_window(uint8_t *fb, int x, int y, int w, int h, const char *title); /* 0x8E62 */
void ui_text(uint8_t *fb, int x, int y, const char *text, uint8_t color);   /* 0x8BE4 + font */
void ui_blit(uint8_t *dst, const uint8_t *src);                              /* 0x0494 */

#endif
