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

/* menu engine (EXE 0x5614/0x54c8): menus are DS records in OVL0 (file 0xB400+DS).
   record: u16 x0,y0,x1,y1, u16 0, u16 items_ptr, u16 title_ptr, u8 count, u8 0xFF, u16 0xFFFF
   item:   u16 text_ptr, u16 hotkey, s16 x0,y0,x1,y1 (relative to box) */
typedef struct {
    int x, y, w, h;
    char title[64];
    int count;
    struct { char text[64]; char hotkey; int x0, y0, x1, y1; } items[16];
} menu_t;

void ui_frame(uint8_t *fb, int x, int y, int w, int h);              /* bevelled box (menus, messages) */
void ui_menu_load(menu_t *m, const uint8_t *ovl0, uint16_t rec_ds);   /* read record from OVL0 */
void ui_menu_draw(uint8_t *fb, const menu_t *m, int preselect);
int  ui_menu_run(const menu_t *m, const input_t *in, int *last_click);  /* index, -1 cancel, -2 none */

#endif
