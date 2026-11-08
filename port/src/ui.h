#ifndef UI_H
#define UI_H
#include <stdint.h>
#include "platform.h"

/* UI primitives mirroring the original EXE (see docs/MAINLOOP.md):
   all draw into the offscreen work buffer; the frame loop blits it to screen. */
void ui_rect(uint8_t *fb, int x, int y, int w, int h, uint8_t color);   /* 0x7E74 */
void ui_window(uint8_t *fb, int x, int y, int w, int h, const char *title); /* 0x8E62 */
void ui_text(uint8_t *fb, int x, int y, const char *text, uint8_t color);   /* 0x8BE4 + font */
int ui_text_width(const char *text);
void ui_text_centered(uint8_t *fb, int cx, int y, const char *text, uint8_t color);
void ui_blit(uint8_t *dst, const uint8_t *src);                              /* 0x0494 */

/* menu engine (EXE 0x5614/0x54c8) */
typedef struct { uint16_t count, x, y, w, h, pad, items; } menu_hdr_t;
typedef struct { const char *text; char hotkey; int16_t x0, y0, x1, y1; } menu_item_t;
void ui_menu_draw(uint8_t *fb, const menu_hdr_t *m, const menu_item_t *items, const char *title, int preselect);
int ui_menu_run(const menu_hdr_t *m, const menu_item_t *items, const input_t *in, int *last_click);

#endif
