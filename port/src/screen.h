#ifndef SCREEN_IFACE_H
#define SCREEN_IFACE_H
/* The screen, input and the shared boxes of the EXE (segment 0823), as one interface for the game loop, the province
 * screens and Game Control (screen.c; docs/MENU_NOTES.md, docs/TEXTBOX_NOTES.md, docs/GAMELOOP_NOTES.md).
 *
 * Two 320x200 pictures, as in the original: g_scr = the VGA screen (A000), g_wrk = the work buffer (2687:0000) that
 * 0x691B / 0x6973 use as the save area under boxes. Input goes through the original's DS words: W(0xB2) buttons
 * (bit 0 left, bit 1 right), W(0xB6) / W(0xB8) mouse x / y (0x0644), B(0xC07F) the last key (0x359B).
 * The port always has a mouse (W(0x00BA) = FFFF), so only the mouse paths are ported (no keyboard cursor 0x3603 / 0x5901). */
#include "ui.h"

extern uint8_t g_scr[64000];     /* the screen (A000) */
extern uint8_t g_wrk[64000];     /* the work buffer (2687:0000) */

int  screen_open(int headless);  /* palette CMAP.BIN (CENT.DAT), LUTs/font (OVL0.OVL), OVL0 bytes; headless = no window */
const uint8_t *screen_ovl0(void);/* the whole OVL0.OVL file */
void screen_close(void);
void screen_present(void);       /* show g_scr (window) or nothing (headless); polls input */
int  screen_quit(void);          /* window closed / script ended */
int  wait_input(void);           /* 0x359B: blocks; 1 = left, 2 = right, 0 = key (B(0xC07F)); sets W(0xB6)/W(0xB8) mouse x/y, W(0xB2) */
void wait_release(void);         /* 0x35DB */
int  key_poll(void);             /* 0x03AA semantics: ASCII or 0, Insert -> 0x10, never blocks */
void message_box(const char *s); /* 0x59D4: 691B save, draw (ui_message_draw), 35F9 wait, 6973 restore */
int  yes_no(const char *s);      /* 0x5C20 with the Yes/No record DS:3E5A: 1 yes / 0 no */
int  menu_choose(menu_t *m, int mode, int pre);  /* 5614 draw + 54C8 until chosen (585B highlight + release wait); -1 Esc */
void screen_shot(const char *name);              /* headless: write g_scr to <shotdir>/<name>.bin */
/* scripted input (headless tests): screen_script(path, shotdir) loads lines "click X Y", "rclick X Y", "key C" (C = a char,
   or esc/enter/space/ins), "shot NAME", "battle F T", "orders T:ops;...", "end"; wait_input/key_poll consume them in order;
   when they run out, screen_quit() becomes true, wait_input returns 0 with B(0xC07F) = 0, and every loop must then exit. */
void screen_script(const char *path, const char *shotdir);
/* battle_ui_attach(1) makes battle_auto() fight on screen through battle_ui.c (menus, ticks, orders, report) instead of
   needing --choices; in scripted runs battles take their choices from the script lines "battle F T" and orders lines. */
void battle_ui_attach(int on);

/* Entry points owned by the parallel agents (defined in map.c, province.c, gamectl.c; NOT in screen.c) */
void map_loop(void);                    /* map.c: 0x07EF..0x09B9 on g_ds; returns when quitting */
void map_build(int z);                  /* 0x494A: the map screen from g_ds (+ pointer warp to DS:1AB8[z]) */
void music_play_5D29(uint8_t m);        /* map.c: 0x5D29 (with 0x5D73), the DS part: slot table DS:A6B0 and [3EEE] */
int  zone_at(int x, int y);             /* 0x6C67 incl. the Asterix village message; 0..36 or 0xFF */
void province_screen(int z);            /* province.c: 0x6CB8 (z 1..26) */
void rome_screen(void);                 /* province.c: 0x7366 */
void zone_info(int z);                  /* province.c: 0x720B */
void province_list(void);               /* province.c: 0x69CC (key P / menu) */
void legion_list(void);                 /* province.c: 0x4280 (key L), uses legions.c */
void fleet_list(void);                  /* province.c: 0x2FD6 (key F) */
void legions_ui_install(void);          /* province.c: fills the legions.h UI hooks with screen.c calls */
void province_strips70ba(uint8_t *fb, uint8_t z);   /* province.c: 0x70BA on fb (archive 7 MNEGO loaded by the caller) */
void province_lines71bb(uint8_t *fb);               /* province.c: 0x71BB on fb */
int  province_cbm(uint8_t *fb, const char *name, int x, int y);   /* province.c: 0x1273 + 0x8E62 + 0x86A0 on fb; 0 = drawn */
int  game_control_menu(void);           /* gamectl.c: 0x1A8B; returns -1 = quit the game, else 0 */
int  new_game(void);                    /* gamectl.c: 0x752E (NEWGAME.LZW) into g_ds */

/* ---- notes on the interface above ----
   menu_choose: mode 2 = box, 1 = province panel fill, 0 = items only (ui_menu_draw_mode), mode < 0 = not drawn (the caller
     drew it); pre >= 0 = the caller's 0x585B(pre, ON) before 0x54C8 (it stays white, as in the original). Returns -1 also when
     the input ends (screen_quit()).
   Script: a click is taken only by wait_input (0x359B), never by wait_release (0x35DB); one poll takes either a click or a
     key, so lines are consumed strictly in order. key_poll takes only "key" lines: a loop that polls keys while a "click"
     line is next must also call wait_input (or it spins). "shot" lines are written when the input reaches them (the screen
     as it waits) and, at the end, by screen_close. Mouse x / y are clamped to the driver range [BC..C0] x [BE..C2].
   Window: Insert -> 0x10, Shift / Caps Lock give upper case and US shifted symbols, Ctrl+letter its control code; arrows,
     F-keys, Delete and Ctrl-S are swallowed (0x03AA). The mouse cursor sprite (0x1BE6) is not drawn: the system pointer is. */

/* ---- extras in screen.c (beyond the shared interface; optional for callers) ---- */
extern int g_battle_ui_attached;        /* set by battle_ui_attach(); the battle wiring (battle.c / battle_ui.c) reads it */
/* Script battle lines, for that wiring. screen_script_choices has the signature of battle_choices (state.h): it takes the
   first unused "battle F T" line of the script (wherever it is: input reading skips battle / orders lines) and returns 1,
   or 0 when there is none. The "orders ..." lines right after that battle line are joined with ';' and returned by
   screen_script_orders() (in main.c's --orders format "T:ops;T:ops"), NULL if none. */
int  screen_script_choices(uint8_t *formation, uint8_t *tactic);
const char *screen_script_orders(void);
int  screen_scripted(void);             /* a script is loaded (input comes from it, not from the window) */
int  screen_headless(void);
const rgb_t *screen_palette(void);      /* the CMAP.BIN palette (250 x 6-bit RGB, x4) */
void menu_load_ds(menu_t *m, uint16_t rec);   /* ui_menu_load from the live g_ds (records patched at run time, e.g. 3E5A) */
void mouse_warp(int x, int y);          /* 0x066A: W(0xB6)/W(0xB8) = x, y and INT 33h fn 4 (the window pointer / script mouse) */
void wait_click(void);                  /* 0x35F9: 35DB, 359B, 35DB */
int  mouse_poll(void);                  /* 0x0644 for the loops that poll: W(0xB2) after it. In a script it takes the next line
                                           when that is a click (as 0x359B's poll); in the window it reads the window. */

#endif
