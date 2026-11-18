#ifndef FG_H
#define FG_H

/* The Fastgraph 4.04 / Fastgraph/Image 2.04 / Fastgraph/Fonts 2.04 subset the game calls, reimplemented for one mode:
   640x480x256 (Fastgraph mode 25, fg_setmode at 0x109F3). Behaviour per docs/LIBRARY_CALLS.md (each entry was
   identified and checked against the code); same names and argument orders as the original calls, so a translated
   call site reads like the disassembly.

   Surfaces: the screen (640x480, what the window shows) and up to 32 virtual buffers (handles 0..31, Fastgraph's
   table at 0xCB196). Drawing goes to the active virtual buffer after fg_vbopen(h), to the screen after fg_vbclose().
   Coordinates: x to the right, y down, inclusive bounds, as Fastgraph. The graphics cursor and the clipping region
   belong to the active surface as in Fastgraph (fg_vbopen saves the screen's and loads the buffer's); the colour is
   global (D: fg_vbopen 0x87C31 saves and loads no colour).

   The port's addition: the screen is shown in the window (platform.h) whenever the game reads input or waits
   (fg_mousepos, fg_intkey, fg_waitfor), so the original's direct video-memory writes appear without explicit calls. */

#include <stdint.h>

void fg_init(void);                       /* screen cleared, palette black, no buffers; call once after plat_init */
uint8_t *fg_screen(void);                 /* the 640x480 screen bytes (palette indices) */
const uint8_t (*fg_dac(void))[3];         /* the 256-entry 6-bit palette as last set by fg_setdacs */
void fg_present(void);                    /* show the screen now (also done by the input calls) */

/* virtual buffers: the game allocates the memory itself (0x82398) and defines the buffer */
int  fg_vbdefine(uint8_t *buffer, int width, int height);   /* returns the handle (0..31) or -1 */
int  fg_vbopen(int handle);               /* 0x87C31: -1 bad handle, -2 not defined, 0 ok */
void fg_vbclose(void);                    /* 0x87D73 */
int  fg_vbcopy(int minx, int maxx, int miny, int maxy, int newx, int newy, int source, int dest);  /* 0x87A1A */
int  fg_vbpaste(int minx, int maxx, int miny, int maxy, int x, int y);   /* 0x8A085: active buffer -> screen */
int  fg_vbcut(int minx, int maxx, int miny, int maxy, int newx, int newy); /* 0x90940: screen -> active buffer */
int  fg_vbtccopy(int minx, int maxx, int miny, int maxy, int newx, int newy, int source, int dest); /* 0x8DF22 */
int  fg_settrans(int number, int flag);   /* 0x8DEE1 */

/* drawing on the active surface */
int  fg_setcolor(int color);              /* 0x87AC2 */
int  fg_getcolor(void);
void fg_move(int x, int y);               /* 0x87DF7 */
int  fg_getxpos(void);                    /* 0x9E010 */
int  fg_getypos(void);                    /* 0x9E016 */
int  fg_point(int x, int y);              /* 0x90FC7 */
int  fg_getpixel(int x, int y);           /* 0x914BC */
int  fg_draw(int x, int y);               /* 0x8A833: line from the cursor, clipped; moves the cursor */
int  fg_rect(int minx, int maxx, int miny, int maxy);    /* 0x8C8B2 */
int  fg_box(int minx, int maxx, int miny, int maxy);     /* 0x8C76E (fg_boxdepth thickness) */
int  fg_boxdepth(int xdepth, int ydepth); /* 0x912BE */
int  fg_setclip(int minx, int maxx, int miny, int maxy); /* 0x9125D */
int  fg_drawmap(const uint8_t *map, int width_bytes, int rows);           /* 0x9D830 (1-bit, lower-left at cursor) */
int  fg_getimage(uint8_t *map, int width, int height);   /* 0x8E444 */
int  fg_clpimage(const uint8_t *map, int width, int height);              /* 0x8EC48 */
int  fg_flpimage(const uint8_t *map, int width, int height);              /* 0x8FA30 */
int  fg_dash(int x, int y, int pattern);  /* 0x91AB3 */
int  fg_dashrel(int ix, int iy, int pattern);             /* 0x8C803 */

/* palette (6-bit DAC values, as the game keeps them) */
int  fg_setdacs(int start, int count, const uint8_t *values);   /* 0x879DC */
int  fg_getdacs(int start, int count, uint8_t *values);         /* 0x90F84 */
int  fg_gifpal(const char *filename, uint8_t *palette);         /* 0x878F7: reads a GIF file's table >> 2 */

/* images and catalogs (Fastgraph/Image; the game's catalog layer 0xA22C3..0xA25B3 on top of catalog.c) */
int  fg_showgif(const char *filename, int flags);               /* 0x88F98: at the cursor (upper-left), flags per doc */
int  cat_open_file(const char *path);     /* 0xA22C3: returns a catalog handle (>0) or 0 */
int  cat_show(const char *name, int flags, int handle);          /* 0xA22FF: GIF (or FLC) from a catalog at the cursor */
long cat_size(const char *name, int handle);                    /* 0xA24A7: entry length, or <0 */
int  cat_load(const char *name, uint8_t *buffer, int handle);   /* 0xA25B3: 0 ok */

/* fonts (fgfont.c) as the game's calls use them */
int  fgf_define(const uint8_t *data);     /* 0x8DFEE: returns the font number (1..32) or 0 */
int  fgf_select(int font);                /* 0x8E188 */
int  fgf_height(const char *s, int n);    /* 0x8E1EB: the tallest glyph of the string */
int  fgf_baseline(void);                  /* 0x8E23E */
int  fgf_spacing(int pixels);             /* 0x8E244 */
int  fgf_print(const char *s, int n);     /* 0x8E253: at the cursor, current colour; moves the cursor */
int  fgf_width(const char *s, int n);     /* 0x8A7DD */

/* mouse and keyboard (platform.h underneath) */
int  fg_mouseini(void);                   /* returns the number of buttons (2) */
int  fg_mousevis(int state);              /* 0x91E9E: 1 show, 0 hide (Fastgraph's own pointer, drawn by the port) */
void fg_mousepos(int *x, int *y, int *buttons);                 /* 0x8C22C: buttons bit 0 left, bit 1 right */
void fg_mousefin(void);                   /* 0x87E43 */
void fg_intkey(unsigned char *key, unsigned char *aux);         /* 0x87E7C: 0,0 when no key waits */
void fg_waitfor(int ticks);               /* 18.2 Hz ticks */

/* port additions (fg.c): no original counterpart unless an address is given */
void fg_set_headless(int on);             /* 1: fg_setdacs does not pace, fg_waitfor does not sleep (tests) */
void fg_set_gamedir(const char *dir);     /* relative file names (fg_showgif, fg_gifpal, cat_open_file) are tried here first */
void fg_set_quit(void (*handler)(void));  /* run when the window is closed (PLAT_QUIT); default plat_shutdown + exit(0) */
void fg_set_input_hook(void (*hook)(void));  /* called each time the game polls input (tests: save the frame, quit) */
void fg_frame_with_pointer(uint8_t *out, int x, int y);   /* tests: the screen + Fastgraph's pointer at (x, y) if shown */
void fg_test_mouse(int x, int y, int buttons);            /* tests (headless): set the mouse as the game will read it */
int  fg_imagebuf(uint8_t *buffer, uint32_t size);   /* 0x8240F: the buffer fg_showgif(.., flags & 4) reads; size 0 =
                                                       Fastgraph's internal 4096 bytes (0xD6694); returns 0 */
int  cat_read(int handle, uint8_t *buffer, unsigned count, unsigned *bytes);  /* _dos_read (0x92420) on a catalog
                                          handle, from where cat_open/cat_size/cat_load/cat_show left it; 0 ok, 6 bad handle */

#endif
