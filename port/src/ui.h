#ifndef UI_H
#define UI_H
#include <stdint.h>
#include "platform.h"

/* UI primitives mirroring the original EXE (segment 0823; docs/MENU_NOTES.md, docs/TEXTBOX_NOTES.md):
   all draw into the offscreen work buffer; the frame loop blits it to screen. The game runs with a mouse
   (word DS:00BA = FFFF in every snapshot), so only the mouse paths are ported. */
typedef struct { int x0, y0, x1, y1; } ui_box_t;          /* inclusive screen rect */

void ui_rect(uint8_t *fb, int x, int y, int w, int h, uint8_t color);   /* fill (0x7E74 takes corners: x, y, x+w-1, y+h-1) */
void ui_window(uint8_t *fb, int x, int y, int w, int h, const char *title); /* (prototype helper, not from the EXE) */
void ui_text(uint8_t *fb, int x, int y, const char *text, uint8_t color);   /* 0x899C, transparent: '\n' -> x back, y += 11 */
void ui_text_attr(uint8_t *fb, int x, int y, const char *text, uint16_t attr); /* 0x899C with the word DS:9B69 (bit 15 = opaque) */
int  ui_text_width(const char *text);       /* ink width = 0x8C58 - 1 (0 for ""); counts every line (no '\n' stop) */
int  ui_text_advance(const char *text);     /* 0x8C58: sum of (glyph width + 1) up to NUL or '\n' */
void ui_text_block(uint8_t *fb, int cx, int y, const char *text, uint8_t color);    /* 0x8BE4: each line at cx - (adv >> 1), 11 px */
void ui_text_block_attr(uint8_t *fb, int cx, int y, const char *text, uint16_t attr); /* 0x8BE4 with the word DS:9B69 */
void ui_text_centered(uint8_t *fb, int cx, int y, const char *text, uint8_t color); /* legacy port rule cx - ui_text_width/2,
                                                    NOT 8BE4 (x is 1 px further right when the 8C58 width is even): see ui.c */
void ui_blit(uint8_t *dst, const uint8_t *src);                              /* 0x0494 */

/* boxes and text boxes */
void     ui_box(uint8_t *fb, int x0, int y0, int x1, int y1, int fill);      /* 0x3134: bevelled box, fill -1 = no fill */
void     ui_frame(uint8_t *fb, int x, int y, int w, int h);                  /* 0x3134 with fill [6346] = 26 */
ui_box_t ui_text_rect(const char *s, int minw, int lines0, int cx, int cy);  /* 0x8C9F */
ui_box_t ui_text_box(uint8_t *fb, const char *s, int minw, int lines0);      /* 8C9F(s,minw,lines0,159,99) + 7, 3134, 8BE4 black
                                                                                (the body of 0x59D4 and of the battle report E48B) */
void     ui_mouse_init(void);                                                /* OVL0 start-up: [00BA] != 0 -> [3E40] = 3E2E (idempotent) */
const char *ui_click_text(void);                                             /* the string at DS:[3E40] after ui_mouse_init (3E2E "Click to continue") */
ui_box_t ui_message_draw(uint8_t *fb, const char *text);                     /* 0x59D4 drawing (no wait, no restore) */
void     ui_copy_rect(uint8_t *dst, const uint8_t *src, ui_box_t r);         /* 0x691B / 0x6973: same coords in both buffers */

/* menu engine (EXE 0x5614/0x54C8): menus are DS records in OVL0 (file 0xB400+DS).
   record (18 bytes): s16 X0,Y0,X1,Y1, u16 colour, u16 items_ptr, u16 title_ptr, u8 count, u8 single, u16 enabled
   item (12 bytes):   u16 text_ptr, u8 hotkey, u8 disabled, s16 x0,y0,x1,y1 (inclusive, relative to X0,Y0) */
typedef struct {
    int x, y, w, h;            /* X0, Y0, X1 - X0 + 1, Y1 - Y0 + 1 */
    char title[64];            /* "" when the title pointer is 0 */
    int count;
    uint16_t colour;           /* +08: word for DS:9B69 (items; title = colour ^ 0x0F) */
    uint8_t single;            /* +0F: one ornament for the column (else one per item) */
    uint16_t enabled;          /* +10: FFFF = ornament + title drawn; 0 = greyed (0x55FF) */
    struct { char text[64]; char hotkey; uint8_t disabled; int x0, y0, x1, y1; } items[16];
} menu_t;

void ui_menu_load(menu_t *m, const uint8_t *ovl0, uint16_t rec_ds);   /* read record from OVL0 */
void ui_menu_draw_mode(uint8_t *fb, const menu_t *m, int mode, int preselect);  /* 0x5614; mode 2 = box, 0 = none,
                                                                       1 = fill of 0x30E6 only (frame sprites not ported);
                                                                       preselect >= 0: 0x585B(m, preselect, ON) */
void ui_menu_draw(uint8_t *fb, const menu_t *m, int preselect);       /* = ui_menu_draw_mode(fb, m, 2, preselect); the original
                                                                       has no hover highlight: pass -1 unless porting a 585B ON */
void ui_menu_highlight(uint8_t *fb, const menu_t *m, int index, int on);   /* 0x585B: item text in 15 (on) / colour (off) */
void ui_menu_enable(uint8_t *fb, menu_t *m);                          /* 0x55EA */
void ui_menu_disable(uint8_t *fb, menu_t *m);                         /* 0x55FF + 0x57F7 */
int  ui_menu_hit(const menu_t *m, int x, int y);                      /* 0x552B: index, or -2 */
int  ui_menu_hotkey(const menu_t *m, int key);                        /* 0x5593: index, -1 for Esc, -2 none */
/* 0x54C8 polled once per frame: index, -1 = Esc, -2 = nothing chosen yet. *last_click != 0 = a button is still held
   from before (0x35DB): buttons and keys are ignored until all are released. Keys are read only when no button is down. After that a held left button hit-tests on every
   poll (drag-select). A choice sets *last_click to the held state; the caller then calls ui_menu_highlight(fb, m, r, 1)
   as 0x54C8 does (for r >= 0). */
int  ui_menu_run(const menu_t *m, const input_t *in, int *last_click);

#endif
