#ifndef LEGIONS_H
#define LEGIONS_H
/* The province command "Legions" (EXE 0x4065) and the routines it calls, as game logic on the DS image (state.h).
 * See legions.c and docs/LEGIONS_NOTES.md. The UI is not here: every message, question, menu, click and drawing call
 * of the original goes through the function pointers below, in the original's order. Each has a headless default. */
#include <stdint.h>

/* ---- input hooks (the player's choices) ---- */
#define LEGIONS_ESC  (-1)   /* legions_click: Esc pressed ([C07F] == 0x1B after 359B returned 0) */
#define LEGIONS_KEY  (-2)   /* legions_click: another key (ignored by 382A) */
extern void (*legions_message)(const char *text);           /* 59D4 message box */
extern int  (*legions_yesno)(const char *text);             /* 5C20: 1 = Yes, 0 = No */
extern int  (*legions_menu)(uint16_t menu, int preselect);  /* 54C8 menu run on the DS record `menu` (1C0A, 1C1C, 1B7E):
                                                                item index, or 0xFF = Esc */
extern int  (*legions_click)(void);                         /* 382A's 359B + 6C67: zone under the mouse click (6C67's value,
                                                                0xFF = none), or LEGIONS_ESC / LEGIONS_KEY for a key */

/* Headless defaults take their answers in order from this list (menu: item, yes/no: 0/1, click: zone or LEGIONS_ESC).
   When the list is used up: menu 0xFF (Esc), yes/no 0, click LEGIONS_ESC, so every loop of the original ends. */
void legions_script(const int *answers, int n);

/* ---- drawing / output-only calls (UI) ---- */
enum legions_ev {
    LEV_CURSOR_PUSH,    /* 1D55(a) */
    LEV_CURSOR_POP,     /* 1D8A */
    LEV_CURSOR_SHOW,    /* 1CCE */
    LEV_WAIT_RELEASE,   /* 35DB wait for the mouse button release */
    LEV_WAIT_CLICK,     /* 35F9 release, click or key, release (Legion List) */
    LEV_MENU_DRAW,      /* 5614(a = menu, b = mode) */
    LEV_MENU_ON,        /* 55EA(a = menu): [menu+0x10] = 0xFFFF is written here first; the event is 56B6 (marks, title 15) */
    LEV_MENU_OFF,       /* 55FF(a = menu): [menu+0x10] = 0 is written here first; the event is 57F7 (marks erased, title 6) */
    LEV_MENU_MARK,      /* 585B(a = menu, b = item, 0) */
    LEV_RECT_RESTORE,   /* 6973(data = const int[4] x0, y0, x1, y1) */
    LEV_LEGION_BOX,     /* 3CB0's drawing: data = const legions_box_t * */
    LEV_MOVE_HEADER,    /* 3EA3's drawing: data = const legions_box_t * (one row) */
    LEV_LEGION_LIST,    /* 4280's drawing: data = const legions_list_t * (then LEV_WAIT_CLICK, LEV_RECT_RESTORE) */
    LEV_MAP_REDRAW,     /* 494A(a = zone the mouse goes to, 0xFF = none) */
    LEV_HIGHLIGHT,      /* 8E62 sprite [C10C] at zone a's anchor (1AB8/1ABA); b = 1: at anchor - (3, 4), a sea with a free fleet */
    LEV_TREASURY,       /* 44E5(a): treasury at (211,167) */
    LEV_RECRUITS,       /* 4510(a): recruits at (225,128) */
    LEV_MUSIC_STOP,     /* 5D20: [3EEE] = 0xFF (current music) is written here first; the event is the driver call 074B */
    LEV_DISK_SWAP       /* 1E21 disk swap prompt (before a battle) */
};
extern void (*legions_ui)(int ev, int a, int b, const void *data);   /* default: nothing */

/* One row of 3DB7 legion_row(L, y, select), all colour 0 */
typedef struct {
    uint8_t     L;
    int         y;
    char        numeral[16];   /* 6738 (DS:C072), right-aligned at num_x */
    int         num_x, name_x; /* 35 / 37 (select) or 25 / 27 (list) */
    const char *name;          /* record +0x00, left at name_x */
    const char *type;          /* 1C64[type], left at 77 */
    uint16_t    inf, cav;      /* right-aligned at 144 / 188 */
    const char *col4;          /* centred at 236: select = courage 7486[c/25]; list = fleet 1100[f] if aboard else zone 5222[z] */
    const char *status;        /* 1B54[status], centred at 292 */
} legions_row_t;

/* 3CB0 / 3EA3 box at the top of the screen */
typedef struct {
    int full;                  /* 1: box drawn (save rect, box 3134 colour [6346], header, rows); 0: only the title */
    int rect[4];               /* 2, 0, 317, y1 */
    const char *title;         /* NULL = none; else fill (7,5)-(312,15) colour [6346], text colour 15 centred at (160, 5) */
    const char *header;        /* 1EFA, colour 0, left at (115, header_y) */
    int header_y;
    int n;
    legions_row_t rows[3];
} legions_box_t;

/* 4280 Complete Legion List */
typedef struct {
    int rect[4];               /* 2, top, 317, bottom: save 691B + box 3134 colour [6346] */
    char title[80];            /* "Complete Legion List -- " + [3E40], colour 15 centred at (160, title_y) */
    int title_y;
    const char *header;        /* 1ECC, colour 0, left at (115, header_y) */
    int header_y;
    int n;
    legions_row_t rows[12];
} legions_list_t;

/* ---- the command and its sub-routines (EXE addresses) ---- */
int      legions_cmd(void);                         /* 4065: on province [C080]; 0 = back to the menu, 0xFFFF = a move ran */
uint8_t  legions_scan_zone(uint8_t z);               /* 3F44: fills A6A2[], A6AE, A6AF */
uint8_t  legions_select(uint8_t z, uint16_t any);    /* 3BAF: legion index, 0xFF = none */
uint16_t legions_raise(uint8_t z, uint16_t type);    /* 4392: 0xFFFF raised (or charged), 0 not */
void     legions_strengthen(uint8_t L, uint8_t z);   /* 453B */
void     legions_upgrade(uint8_t L);                 /* 3F97 */
void     legions_move(uint8_t L);                    /* 382A (clicks from legions_click) */
uint16_t legions_fleet_fit(uint8_t f, uint16_t L, uint16_t *cav, uint16_t *inf);   /* 3A32: men left behind */
uint16_t legions_board(uint8_t z0, uint8_t L, uint8_t f);   /* 3AE4: 0xFFFF boarded, 0 refused */
uint16_t legions_fight_armies(uint16_t L, uint16_t z);      /* 4B3F: 0xFFFF no battle lost, 0 a battle lost */
uint16_t legions_enter_foreign(uint8_t L, uint8_t z);       /* 5F82: nonzero = province taken or allied (rebels' battle
                                                                here, the negotiation in nego.c) */
void     legions_list(void);                         /* 4280 */
void     legions_list_data(legions_list_t *out);     /* 4280's layout and rows, without the DS writes or drawing */
void     legions_row_data(uint8_t L, int y, uint16_t select, legions_row_t *r);   /* 3DB7 row as data (no DS writes) */
void     legions_close_box(void);                    /* 3F23 */
void     legions_move_header(uint8_t L);             /* 3EA3: [1B52] = (h + 6) * 2 + gap, one-row box (LEV_MOVE_HEADER).
                                                        Callers: 4065 at 421B, and the map loop at 0x08F5..0x0919:
                                                        L = legions_select(z, 0); legions_close_box();
                                                        if (L < 12) { legions_move_header(L); legions_move(L); } */

int      legions_selftest(const char *ds_path);      /* runs sub-commands on a DS image (NULL = endturn/t10_in.bin) */

#endif
