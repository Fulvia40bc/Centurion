/* Module START of docs/MAP_PLAN.md: what main runs after the nation choice (0x11854..0x11884): the computers' coin
   flips 0x6B9AC, then the starting-territory screen 0x454D0 ("%s place your token in a starting territory." for a
   human, "%s is selecting its starting territory." for a computer), with the routines only it needs: the map-screen
   click 0x3D434, the computer's pick 0x45F88, the message-area restore 0x3A924 and the .bpm opening-map loader 0x15FE0.
   The .bpm name of each nation comes from 0x14C6C (aistart.h / aistart.c, the other half of the module). Literal
   translations; the address of each instruction a line comes from is in its comment. D = read in the code, G =
   inference. Prototypes: start.h.

   Watcom runtime -> standard C:
     0x7DB3D sprintf -> snprintf bounded to the original's stack buffer (100 bytes at [ebp-0x94] in 0x454D0)
     0x7D555 printf (D: it passes its format and the stack's va_list to 0x9480D with the stdout FILE 0xD5D56; ui.h's
             f3A780 uses it the same way) -> fputs of the format, which has no conversion: the string 0xB46F1 is
             "\a", the BEL the original rings for a click outside the player's own territories
     rep movsd / movsb (0x15FE0, main 0x1186D) -> memcpy inside g_ds (the ranges do not overlap)
     0x7D4AC (__CHK, every prologue) dropped; 0x7D4BF is __STK, called only from __CHK (0x7D4B0): not used here.
   Catalog calls through fg.h (cat_size 0xA24A7, cat_load 0xA25B3) in the original argument order. game_rand is f78AE4
   (game.h), called in the original order and number: 0x6B9AC one call per computer nation (n ascending); 0x45F88 two
   per computer nation (the +0x9B flip, then exactly one draw from the first non-empty list); 0x454D0 none of its own;
   0x14C6C makes no calls at all (D: its only call is __CHK). So START draws 3 numbers per computer nation.

   Data (D, read in the code at the addresses given):
     nation n at 0xF14CC + 199n (game.h): +0x60 uint8 count of the area list, +0x61 uint8 the first area (territory +
       1), +0x98 tokens on the map, +0x99 tokens in stock, +0x9A 0 not in play / 2 computer / 3 human, +0x9B (written by
       0x45F88: 1 with chance 1/5, G: an AI trait), +0xBC (written by 0x6B9AC for computers: 1 with chance 1/2; 0x45D30
       set it to 1 for every nation, G: an AI trait)
     territory record t at 0xE1638 + 35t (game.h): byte +0 >> 4 = the nation whose starting territory t is (0 none;
       0x128F0 edits these nibbles by player count, newgame.c). 0x454D0 highlights t < 157 with that nibble == n
       (border colour 15) and accepts a human click only on such a t.
     area record a at 0xE31B8 + 69a (record t + 1 = territory t): +1 uint8 incremented per start token (G: the number of
       nations there), +1+n = n, +0x0E+n = 1 (G: tokens of nation n; the same fields the PBEM start 0x12C38 writes)
     0xF06AE int16 the count, 0xF06B0 int16[] the nations in play in order (the player buttons, f783B8)
     0xF40F8 int32 the current nation (set for every n 1..9 the loop visits, in play or not)
     0xF4102 uint8 the phase: 0x15 during the screen (f7876C's name table index), 0 after it (main 0x11886)
     0xBB3FA.. int16 the message area: (169, 332, 300, 123) here
     0xC1A5C the SYSTEM menu dialog (SAVE GAME 0x4F, RESTORE GAME 0x50, ...); 0xF596C the map screen's panel dialog
       (SYSTEM 5, AST 3, CENSUS 4, TOOLS 6, GOODS 7, mini-map 0x23, TOGGLE UNITS 0x24, ZOOM 0x25, replay "0" 0x101, the
       scroll strips 0x1C (7,0,625,6), 0x1E (0,6,8,285), 0x1F (632,6,8,285), 0x21 (7,326,625,6), and the player
       buttons 9.. at y 460, obj2 0xBB414 + 0x7784C). The screen disables SAVE, RESTORE, GOODS, TOOLS, AST, CENSUS
       and enables them again at its end; SYSTEM, the map buttons and the player buttons stay live.
     The .bpm opening maps (ACIV.CT3 [0xF40DC], 523 bytes each, D from the files and 0x15FE0):
       +0x00 uint8 1 (G: a format version; 1 in all 66), +0x01 char[41] the editor's label, NUL-terminated (the bytes
       after the NUL are leftovers of earlier labels), +0x2A uint8 the player count (= the digit in the name in all
       66), then three tables of 160 bytes indexed by territory:
         +0x2B  -> 0xEED84 + 160n  int8 priority (0xFF = -1, never wanted); 0x45F88 picks the highest
         +0xCB  -> 0xF0044 + 160n  flags; bit 2 adds 5 to the priority in 0x45F88 (meaning of the other bits G)
         +0x16B -> 0xEFA04 + 160n  (not read here; G: a second AI table)
       main then copies 0xEED84[10][160] to 0xEF3C4 (0x640 bytes, 0x1186D; G: the original priorities, kept while the
       AI changes the working copy).
     0xF60E8 int32[10] / 0xF6064 char[10][13]: a per-nation override name for the .bpm (used by 0x14C6C, 0x454D0,
       0x14A00, 0x14AA4). main 0x10172 clears 0xF60E8 and nothing writes either array (D: no other reference), so the
       override is a developer leftover that never fires (G: set by a debugger or a removed switch).

   Heap pointers in data fields: none written. [0xF522C] is read (the .bpm buffer): ui_screen.c's ui_heap_F522C,
   0xFF00 bytes in the port (title.c), as in choose.c. [0xF52C8] / [0xF52CC] are Fastgraph buffer handles.

   Where the screen waits (for scripted runs): see f454D0's comment.

   1995 bugs: none found that change the game (no g_original branches). Oddities, kept (D unless marked):
     - 0x454D0 calls 0x14C6C and 0x15FE0 for human nations too (their tables are loaded; G: so the AI can advise or
       take over). The .bpm name buffer is one local of 0x454D0 for the whole loop: when 0x14C6C leaves it untouched
       (it does for a player count it has no map for, aistart.h), the nation gets the previous nation's file again
       (and the first nation: stack garbage; the port's buffer starts as zeros).
     - For a nation whose +0x9A is neither 3 nor 2 (never written so, G) 0x454D0 would place a token on the area index
       left in its local from the previous nation (uninitialised for the first); the port starts that local at 0.
     - A click is any button (left or right): the wait tests [0xE1630] != 0.
     - The highlights of the last nation (border colour 15) are not cleared when the screen ends (f3F670 runs only at
       the start of each nation's turn).
     - 0x45F88 does not look at 0xEE16C (in play) or at other nations' tokens: the .bpm tables alone decide. Its list
       0xF7CA0 takes up to 157 int32 here (0xF7CA0..0xF7F13; the next variable referenced is 0xF7F1C), more than the
       nine entries the nation choice uses.
     - A priority + 5 above 127 could never be picked (best starts at 0x7F and only goes down); no .bpm has one (all
       66 checked). */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "aistart.h"
#include "fg.h"
#include "game.h"
#include "map.h"
#include "mapdata.h"
#include "panel.h"
#include "start.h"
#include "state.h"
#include "ui.h"

/* Defined in ui_screen.c, not yet in ui.h (title.c and choose.c declare it the same way): the port's home of
   [0xF522C]. */
extern uint8_t *ui_heap_F522C;

#define BACKBUF    0xF52C8u   /* int32 virtual-buffer handle of the back buffer */
#define BGBUF      0xF52CCu   /* int32 handle of the background-picture buffer */
#define PANEL_DLG  0xF596Cu   /* the map screen's panel dialog */
#define SYSTEM_DLG 0xC1A5Cu   /* the SYSTEM menu dialog (SAVE GAME 0x4F, RESTORE GAME 0x50, ...) */
#define MOUSE_X    0xE1628u   /* int32 mouse x from fg_mousepos */
#define MOUSE_Y    0xE162Cu   /* int32 mouse y */
#define MOUSE_B    0xE1630u   /* int32 mouse buttons */
#define CUR_DLG    0xF8E78u   /* uint32 the dialog f3B05C drew last (ui.h) */
#define BPM_PRIO   0xEED84u   /* int8[10][160] the .bpm priorities (+0x2B) */
#define BPM_FLAGS  0xF0044u   /* uint8[10][160] the .bpm flags (+0xCB) */
#define BPM_THIRD  0xEFA04u   /* uint8[10][160] the .bpm third table (+0x16B) */
#define LIST_COUNT 0xF7C9Cu   /* int32 count of the choice list */
#define LIST       0xF7CA0u   /* int32[] the choice list */

/* nation n's record field */
static uint32_t nat(int32_t n, uint32_t off)
{
    return (uint32_t)((int32_t)GAME_NATION + 199 * n) + off;
}

/* territory t's byte +0 >> 4: the nation whose starting territory t is (mov al,[35t + 0xE1638]; shr al,4; and 0xFF) */
static int32_t start_nation(int32_t t)
{
    return (int32_t)(D8((uint32_t)((int32_t)GAME_TERR + GAME_TERR_SIZE * t)) >> 4);
}

/* ---- 0x6B9AC: the computers' coin flips -------------------------------------------------------------------------- */

void f6B9AC(void)
{
    int32_t n;                                                  /* [ebp-0x8] */
    uint32_t r;

    for (n = 1; n <= 9; n++) {                                  /* 0x6B9BD n = 1; 0x6B9CC cmp 9; jg */
        if (S8(nat(n, 0x9A)) != 2)                              /* 0x6B9D2 dword [199n + 0xF1563] sar 24; cmp 2; jne */
            continue;
        r = (uint32_t)f78AE4();                                 /* 0x6B9E7 game_rand */
        r = (r * 100u) >> 15;                                   /* 0x6B9EC..0x6B9FB ((3r)*8 + r)*4 = 100r; shr 0xF */
        setD8(nat(n, 0xBC), (uint8_t)((int32_t)r < 0x32));      /* 0x6BA05 cmp eax,0x32; setl; 0x6BA0B [199n + 0xF1588] */
    }
}

/* ---- 0x3A924: restore the message area ------------------------------------------------------------------------- */

void f3A924(void)
{
    int32_t x, y, w, h;

    f3CD6C(S8(0xEE168u));                                       /* 0x3A934 dword [0xEE165] sar 24; 0x3A93C */
    h = S16(0xBB400u);                                          /* 0x3A947 dword [0xBB3FE] sar 16 */
    y = S16(0xBB3FCu);                                          /* 0x3A94D dword [0xBB3FA] sar 16 */
    x = S16(0xBB3FAu);                                          /* 0x3A965 dword [0xBB3F8] sar 16 */
    w = S16(0xBB3FEu);                                          /* 0x3A971 dword [0xBB3FC] sar 16 */
    fg_vbcopy(x, x + w - 1, y, y + h - 1, x, y + h - 1,         /* 0x3A97F: lower-left (x, y + h - 1) */
              S32(BGBUF), S32(BACKBUF));
}

/* ---- 0x3D434: a click on the map screen -------------------------------------------------------------------------- */

int f3D434(int x, int y, uint32_t dlg)
{
    int32_t code;                                               /* [ebp-0x18] */
    int32_t onmap;                                              /* [ebp-0x14] */
    int32_t i;                                                  /* [ebp-0x10] */
    uint32_t rec;                                               /* [ebp-0xC] */

    code = -100;                                                /* 0x3D44F mov [ebp-0x18], 0xFFFFFF9C */
    onmap = 1;                                                  /* 0x3D456 */
    for (i = 0; S16(dlg + DLG_COUNT) > i; i++) {                /* 0x3D45D; 0x3D46D movsx word [dlg]; 0x3D473 jle */
        rec = dlg + DLG_REC(i);                                 /* 0x3D479..0x3D490: dlg + 2 + 20i */
        if (S8(rec + BTN_ENABLED) != 1)                         /* 0x3D496 dword [rec+0xD] sar 24; cmp 1; jne */
            continue;
        /* 0x3D4A1..0x3D4D9, signed compares: x <= x < x + w, y <= y < y + h */
        if (S16(rec + BTN_X) > x)                               /* 0x3D4A4 movsx word [rec]; 0x3D4AC jg */
            continue;
        if (S16(rec + BTN_X) + S16(rec + BTN_W) <= x)           /* 0x3D4B1 [rec+2] sar 16; 0x3D4BC jle */
            continue;
        if (S16(rec + BTN_Y) > y)                               /* 0x3D4C1 [rec] sar 16; 0x3D4C9 jg */
            continue;
        if (S16(rec + BTN_Y) + S16(rec + BTN_H) <= y)           /* 0x3D4CE [rec+4] sar 16; 0x3D4D9 jle */
            continue;
        f3C5D4(dlg, S16(rec + BTN_ID));                         /* 0x3D4DE [rec+0xC] sar 16; 0x3D4E7 */
        code = f3FB94(S16(rec + BTN_ID));                       /* 0x3D4EF..0x3D4FA */
        onmap = 0;                                              /* 0x3D4FD */
        break;                                                  /* falls into 0x3D502: out of the loop */
    }

    if (onmap != 0 && y <= 0x145)                               /* 0x3D502 je; 0x3D508 cmp y,0x145; jg (signed) */
        code = f3D2A4(x, y);                                    /* 0x3D517; 0x3D51C */
    return code;                                                /* 0x3D51F */
}

/* ---- 0x15FE0: load a .bpm opening map ---------------------------------------------------------------------------- */

void f15FE0(int n, const char *name)
{
    if (cat_size(name, S32(0xF40DCu)) != -2)                    /* 0x16005 cat_size(name, [0xF40DC] = CT3); cmp -2; je */
        (void)cat_load(name, ui_heap_F522C, S32(0xF40DCu));     /* 0x16024 cat_load(name, [0xF522C], [0xF40DC]) */
    else
        not_ported(0x7D334u);                                   /* 0x16039 0x7D334(name, [0xF522C]): the loose file.
                                                                   newgame.c's translation is static (see the report) */

    memcpy(DP(BPM_PRIO + 160u * (uint32_t)n), ui_heap_F522C + 0x2B, 0xA0);    /* 0x16041..0x16072 */
    memcpy(DP(BPM_FLAGS + 160u * (uint32_t)n), ui_heap_F522C + 0xCB, 0xA0);   /* 0x16075..0x160A9 */
    memcpy(DP(BPM_THIRD + 160u * (uint32_t)n), ui_heap_F522C + 0x16B, 0xA0);  /* 0x160AC..0x160E0 */
}

/* ---- 0x45F88: the computer's starting territory ------------------------------------------------------------------ */

int f45F88(int n_arg)
{
    int8_t n = (int8_t)n_arg;                                   /* 0x45F9B mov [ebp-0x10], al (read back with movsx) */
    int32_t best;                                               /* [ebp-0x18] */
    int32_t pick;                                               /* [ebp-0x14] */
    int32_t t;                                                  /* [ebp-0x1C] */
    int32_t k;
    uint32_t r;

    r = (uint32_t)f78AE4();                                     /* 0x45F9E game_rand */
    if (((r * 5u) >> 15) == 1)                                  /* 0x45FA3..0x45FAD lea 5r; shr 0xF; cmp 1; jne */
        setD8(nat(n, 0x9B), 1);                                 /* 0x45FBC [199n + 0xF1567] = 1 */
    else
        setD8(nat(n, 0x9B), 0);                                 /* 0x45FD1 = dh = 0 */

    best = 0x7F;                                                /* 0x45FD7 */
    pick = -1;                                                  /* 0x45FDE */
    while (pick == -1) {                                        /* 0x45FE5 cmp -1; jne 0x460AB */
        setD32(LIST_COUNT, 0);                                  /* 0x45FF1 */
        for (t = 0; t < 0x9D; t++) {                            /* 0x45FF7; 0x46002 cmp 0x9D; jge */
            uint32_t idx = (uint32_t)(160 * (int32_t)n + t);    /* movsx n; lea 4n + n; shl 5; + t */
            uint8_t b = D8(BPM_FLAGS + idx);                    /* 0x4601E mov al,[.. + 0xF0044] */
            int32_t v = (int32_t)S8(BPM_PRIO + idx)             /* 0x4604A dword [.. + 0xEED81] sar 24 */
                        + 5 * (int32_t)((uint8_t)(b << 5) >> 7);    /* 0x46024 shl al,5; shr al,7: bit 2; lea 5x */
            if (v == best) {                                    /* 0x46058 cmp; jne */
                k = S32(LIST_COUNT);                            /* 0x4605C */
                setD32(LIST_COUNT, (uint32_t)(k + 1));          /* 0x46069 */
                setD32(LIST + 4u * (uint32_t)k, (uint32_t)t);   /* 0x46071 [4k + 0xF7CA0] = t */
            }
        }
        if (S32(LIST_COUNT) > 0) {                              /* 0x46079 cmp 0; jle (signed) */
            r = (uint32_t)f78AE4();                             /* 0x46082 game_rand */
            r = (r * (uint32_t)S32(LIST_COUNT)) >> 15;          /* 0x46087 imul [0xF7C9C]; 0x4608E shr 0xF */
            pick = S32(LIST + 4u * r);                          /* 0x46091 [4r + 0xF7CA0] */
        } else {
            best--;                                             /* 0x460A3 */
        }
    }
    return pick + 1;                                            /* 0x460AB inc eax */
}

/* ---- 0x454D0: the starting territories ----------------------------------------------------------------------------

   Where it waits (D): for a human nation (+0x9A == 3), in its own inline loop at 0x45700..0x4578A, not through the
   generic click wait 0x78BA4: f775F0 (until no button is down), then fg_mousepos into 0xE1628/0xE162C/0xE1630 until
   [0xE1630] != 0 (any button), then f3D434([0xE1628], [0xE162C], 0xF596C):
     - an enabled record of the panel dialog 0xF596C under the point takes the click first (press feedback f3C5D4, then
       the dispatcher f3FB94: scroll strips, mini-map, TOGGLE UNITS, ZOOM, SYSTEM, the player buttons; all of them
       return a negative code here, so the loop simply waits again);
     - else, with y <= 0x145, the map hit test f3D2A4(x, y): the territory t in play (0xEE16C) whose mapdataz spans
       contain (x + [0xF4884] * 78 - 8, y + scroll y - 6), scroll y = (row / 2) * 71 + (row % 2) * 35 of [0xF4888],
       aliases 0x9D..0x9F folded to 0x39/0x0D/0x87 unless [0x108804] == 2 (sea mode; phase 0x15 is not 4);
     - t outside 0..0x9F: wait again; byte terr[t] +0 >> 4 == n: the choice (area index t + 1); else printf("\a")
       (a BEL) and wait again.
   The scroll is nation n's home view, set by f3D9FC(n) just before the wait (column 0xC19E4[n], row 0xC1A0C[n],
   clamped by f3D52C to [0xF6110]..[0xF6112] / 0..7; entry 0 for n == 2 when bit 0x02 of [0xE1624] is set), unless the
   player scrolls. So a script clicks at screen (mx - [0xF4884] * 78 + 8, my - scroll y + 6) for a map-picture pixel
   (mx, my) inside one of n's highlighted territories, with the screen y <= 325 and outside the panel's records (the
   scroll strips cover x 0..7, x 632..639, y 0..5 and y 326..331). Computer nations do not wait for input: f45F88
   picks, then f78A20(750). Every nation then waits f78A20(500) after its token is drawn. */

void f454D0(void)
{
    int32_t n;                                                  /* [ebp-0x20] */
    int32_t t = 0;                                              /* [ebp-0x1C]: uninitialised in the original (port) */
    int32_t i;                                                  /* [ebp-0x18] */
    int32_t k;
    char name[16] = { 0 };                                      /* [ebp-0x30]..[ebp-0x21]: the .bpm name (port: zeros
                                                                   instead of stack garbage) */
    char text[100];                                             /* [ebp-0x94]..[ebp-0x31] */

    setD16(0xBB3FAu, 0xA9);                                     /* 0x454FE message area x 169 */
    setD16(0xBB3FCu, 0x14C);                                    /* 0x45505 y 332 */
    setD16(0xBB3FEu, 0x12C);                                    /* 0x4550C w 300 */
    setD16(0xBB400u, 0x7B);                                     /* 0x45513 h 123 */
    setD16(0xF06AEu, 0);                                        /* 0x4551A */
    for (i = 1; i <= 9; i++) {                                  /* 0x45521; 0x45530 cmp 9; jg */
        if (D8(nat(i, 0x9A)) == 0)                              /* 0x4553D mov dl,[199i + 0xF1566]; test; je */
            continue;
        k = S16(0xF06AEu);                                      /* 0x45547 mov ax,[0xF06AE]; movsx edx,ax */
        setD16(0xF06AEu, (uint16_t)(k + 1));                    /* 0x45550 inc eax; 0x45551 mov [0xF06AE],ax */
        setD16((uint32_t)(0xF06B0 + 2 * k), (uint16_t)i);       /* 0x4555A mov [2*edx + 0xF06B0], ax */
    }
    f783B8(PANEL_DLG, S32(0xF5940u));                           /* 0x4556F the player button labels */
    setD8(0xF4102u, 0x15);                                      /* 0x4557D mov [0xF4102], dh (dh = 0x15) */
    f3BFBC(SYSTEM_DLG, 0x4F, 0);                                /* 0x45588 SAVE GAME off */
    f3BFBC(SYSTEM_DLG, 0x50, 0);                                /* 0x45599 RESTORE GAME off */
    f3BFBC(PANEL_DLG, 7, 0);                                    /* 0x455AA GOODS off */
    f3BFBC(PANEL_DLG, 6, 0);                                    /* 0x455BB TOOLS off */
    f3BFBC(PANEL_DLG, 3, 0);                                    /* 0x455CC AST off */
    f3BFBC(PANEL_DLG, 4, 0);                                    /* 0x455DD CENSUS off */
    f3CD6C(3);                                                  /* 0x455E7 background 3 into [0xF52CC] */
    setD8(0xEE168u, 3);                                         /* 0x45610 */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BGBUF), S32(BACKBUF));   /* 0x45616 background -> back buffer */
    f3B05C(PANEL_DLG);                                          /* 0x45628 the panel buttons into the back buffer */

    for (n = 1; n <= 9; n++) {                                  /* 0x4562D [ebp-0x20] = edx = 1 (kept by f3B05C);
                                                                   0x45918 cmp 9; jg 0x4595F */
        setD32(0xF40F8u, (uint32_t)n);                          /* 0x45921 the current nation */
        if (D8(nat(n, 0x9A)) == 0)                              /* 0x4592D mov bh,[199n + 0xF1566]; test; je 0x45912 */
            continue;
        f14C6C(name, n);                                        /* 0x4593D f14C6C(&local, n): the .bpm name */
        if (D32((uint32_t)(0xF60E8 + 4 * n)) != 0)              /* 0x45945 [4n + 0xF60E8]; test; jne 0x45635 */
            f15FE0(n, DSTR((uint32_t)(0xF6064 + 13 * n)));      /* 0x45635..0x45650 the override name 0xF6064 + 13n */
        else
            f15FE0(n, name);                                    /* 0x45954..0x45650 */
        f3A924();                                               /* 0x45655 clear the message area */

        if (S8(nat(n, 0x9A)) == 3) {                            /* 0x4565A dword [199n + 0xF1563] sar 24; cmp 3; jne */
            /* a human */
            snprintf(text, sizeof text, DSTR(0xB46C4u),         /* 0x4568A "%s place your token in a starting territory." */
                     DSTR(D32((uint32_t)(0xC768C + 4 * n))));
            f3A9F4(text);                                       /* 0x4569A */
            f3F670();                                           /* 0x4569F clear the border highlights */
            for (i = 0; i < 0x9D; i++)                          /* 0x456A4 i = ecx = 0; 0x456AF cmp 0x9D; jge */
                if (start_nation(i) == n)                       /* 0x456C9..0x456DA */
                    f3F624(i, 0xF);                             /* 0x456E4 */
            f3D9FC(n);                                          /* 0x456EE home view (redraws the map) */
            f459E4(n);                                          /* 0x456F6 status panel */
            f3CBBC();                                           /* 0x456FB panel rows to the screen */
            for (;;) {
                f775F0();                                       /* 0x45700 wait until no button is down */
                while (S32(MOUSE_B) == 0) {                     /* 0x45705 cmp dword [0xE1630],0; jne */
                    int mx, my, mb;
                    fg_mousepos(&mx, &my, &mb);                 /* 0x4571D fg_mousepos(0xE1628, 0xE162C, 0xE1630) */
                    setD32(MOUSE_X, (uint32_t)mx);
                    setD32(MOUSE_Y, (uint32_t)my);
                    setD32(MOUSE_B, (uint32_t)mb);
                }
                t = f3D434(S32(MOUSE_X), S32(MOUSE_Y), PANEL_DLG);   /* 0x45737; 0x4573C */
                if (t < 0 || t >= 0xA0)                         /* 0x4573F jl; 0x45745 cmp 0xA0; jge -> 0x45700 */
                    continue;
                if (start_nation(t) == n) {                     /* 0x4574E..0x45770 */
                    t++;                                        /* 0x45775 the area index */
                    break;                                      /* 0x45778 jmp 0x45855 */
                }
                fputs(DSTR(0xB46F1u), stdout);                  /* 0x45782 printf("\a") */
                fflush(stdout);                                 /* port: ring now (G: DOS printed at once) */
            }
        } else if (S8(nat(n, 0x9A)) == 2) {                     /* 0x4578F dword [199n + 0xF1563] sar 24; cmp 2; jne
                                                                   0x45855 (other values: t as it was, see above) */
            /* a computer */
            snprintf(text, sizeof text, DSTR(0xB46F3u),         /* 0x457BF "%s is selecting its starting territory." */
                     DSTR(D32((uint32_t)(0xC768C + 4 * n))));
            f3A9F4(text);                                       /* 0x457CD */
            f3C828(D32(CUR_DLG));                               /* 0x457D9 the buttons off, saved (back buffer) */
            f3F670();                                           /* 0x457DE */
            for (i = 0; i < 0x9D; i++)                          /* 0x457E3 i = ecx = 0; 0x457EE cmp 0x9D; jge */
                if (start_nation(i) == n)                       /* 0x45808..0x45819 */
                    f3F624(i, 0xF);                             /* 0x45823 */
            f3D9FC(n);                                          /* 0x4582D */
            f459E4(n);                                          /* 0x45835 */
            f3CBBC();                                           /* 0x4583A */
            t = f45F88((int8_t)n);                              /* 0x4583F movsx eax, byte [ebp-0x20]; 0x45843 */
            f78A20(0x2EE);                                      /* 0x4584B wait 750 ms */
        }

        /* 0x45855: the token goes to area t (territory t - 1) */
        setD8(nat(n, 0x99), (uint8_t)(D8(nat(n, 0x99)) - 1));   /* 0x4585C dec byte [199n + 0xF1565]: in stock */
        setD8(nat(n, 0x98), (uint8_t)(D8(nat(n, 0x98)) + 1));   /* 0x45869 inc [199n + 0xF1564]: on the map */
        setD8(nat(n, 0x60), (uint8_t)(D8(nat(n, 0x60)) + 1));   /* 0x45876 inc [199n + 0xF152C]: area list count */
        setD8(nat(n, 0x61), (uint8_t)t);                        /* 0x45886 [199n + 0xF152D] = t */
        setD8((uint32_t)((int32_t)GAME_AREA + GAME_AREA_SIZE * t + 1),          /* 0x45896 inc [69t + 0xE31B9] */
              (uint8_t)(D8((uint32_t)((int32_t)GAME_AREA + GAME_AREA_SIZE * t + 1)) + 1));
        setD8((uint32_t)((int32_t)GAME_AREA + GAME_AREA_SIZE * t + 1 + n), (uint8_t)n);   /* 0x458B4 [69t + n + 0xE31B9] */
        setD8((uint32_t)((int32_t)GAME_AREA + GAME_AREA_SIZE * t + 0x0E + n), 1);         /* 0x458CC [69t + n + 0xE31C6] */
        f459E4(n);                                              /* 0x458D6 */
        f3D52C(0, 0);                                           /* 0x458DF redraw the map */
        f78A20(0x1F4);                                          /* 0x458E9 wait 500 ms */
        if (S8(nat(n, 0x9A)) == 2) {                            /* 0x458EE dword [199n + 0xF1563] sar 24; cmp 2; jne */
            f3C7A0(D32(CUR_DLG));                               /* 0x45908 the buttons back (back buffer) */
            f3CBBC();                                           /* 0x4590D */
        }
    }

    f3BFBC(SYSTEM_DLG, 0x4F, 1);                                /* 0x4596E SAVE GAME on */
    f3BFBC(SYSTEM_DLG, 0x50, 1);                                /* 0x45982 RESTORE GAME on */
    f3BFBC(PANEL_DLG, 7, 1);                                    /* 0x45996 */
    f3BFBC(PANEL_DLG, 6, 1);                                    /* 0x459AA */
    f3BFBC(PANEL_DLG, 3, 1);                                    /* 0x459BE */
    f3BFBC(PANEL_DLG, 4, 1);                                    /* 0x459D2 */
}

/* ---- main 0x11854..0x11884 ------------------------------------------------------------------------------------- */

void start_slice(void)
{
    f6B9AC();                                                   /* 0x11854 */
    f454D0();                                                   /* 0x11868 (ecx, esi, edi for the copy set at
                                                                   0x11859..0x11863, kept across the call) */
    memcpy(DP(0xEF3C4u), DP(BPM_PRIO), 0x640);                  /* 0x1186D..0x1187C rep movsd 0x190, movsb 0 */
    /* 0x1187D jmp 0x11884 over 0x1187F (f12C38, the PBEM start: dropped) */
    setD8(0xF4102u, 0);                                         /* 0x11884 xor bl,bl; 0x11886 mov [0xF4102], bl */
}
