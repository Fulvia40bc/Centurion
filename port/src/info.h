#ifndef INFO_H
#define INFO_H

/* Module INFO of docs/MAP_PLAN.md (info.c): the information screens behind the panel buttons CENSUS (id 4), TOOLS (6),
   GOODS (7) and the trade screens' REVIEW (0x103), with their helpers:
     0x13E50 census, 0x19C10 Civilization Tools, 0x1A124 the advance description pop-up, 0x76A9C Goods Held,
     0x76798 "Select Nation To Show", 0x71F2C / 0x71FB0 a hand face down / face up, 0x76F14 cards held,
     0x194E4 the Mining bonus list, 0x195D8 the value of a hand, 0x763FC / 0x764A8 Trading Wrapup,
   and the bodies of the dispatcher cases 4, 6, 7 and 0x103 as dcase_<id> (dispatch.c wires them in).

   Conventions: those of ui.h and game.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx, ebx,
   ecx; dialogs and sprite records as uint32_t link addresses; void unless a caller reads eax). D = read in the code,
   G = inference. The data layouts are in info.c's file comment. */

#include <stdint.h>

void f13E50(void);                     /* D: the census screen (census.gif, background 5, drawn in the background buffer [0xF52CC]): each nation in play as its 24x23 token in the cell of its tokens on the map (+0x98), several in one cell offset diagonally, a dashed box for +0x35 != 0; the PBEM line if byte [0xF40FE] == 1; CONTINUE of 0xBB800 moved to (264, 398) and sized 111 x 23, f3CC68, f78BA4 until -1, size put back to 110 x 19. One caller (dispatcher id 4) */
void f19C10(void);                     /* D: Civilization Tools (background 20): the screen saved ("junk"), each nation in play with token, name, the cost of its advances and the 6 x 4 grid of the 24 advance names; dialog 0xBD940 (rows 0xB2..0xB9 -> f1A124 on the clicked advance, then a full redraw; CONTINUE ends); the screen put back into the back buffer and shown, [0xF8E78] restored. One caller (dispatcher id 6) */
void f1A124(int idx);                  /* D: pop-up box with advance idx's name and description (0xC7DC4, 0xC7E9C) over the screen, CONTINUE of 0xBBBEC; the area saved and put back ("junk2"), the current dialog disabled and re-enabled (f3C940 / f3C8C4). One caller (0x1A0CD) */
void f76A9C(void);                     /* D: Goods Held (background 19): whose hand is face up (one human: that nation; PBEM: byte [0xE1622]; more humans: f76798; none: [0xF40F8]); one row per nation in play with token, name, f194E4, the hand face up with POINTS (f195D8) or face down; CONTINUE of 0xBB800 at (265, 450). One caller (dispatcher id 7) */
int  f76798(void);                     /* D: "Select Nation To Show": a box with one button (dialog 0xBC3C4, ids 0x12..) per nation with +0x9A == 3; returns the nation clicked. The area saved and put back ("junk"). One caller (0x76B84) */
void f71F2C(int x, int y, int w, int n);   /* D: nation n's hand face down: f76F14(n) card backs (sprite 0xF4D14) from (x, y), spacing [0x11254C] = 40 reduced by 2 until it fits w. 4 call sites, eax unused */
void f71FB0(int x, int y, int w, int n);   /* D: nation n's hand face up: copies of cardtiny sprite i (0xF4C1C + 8i) per commodity i 1..18, then the calamity cards (+0x13 of them, ids at +0x14..), steps [0x11254C] (40) and [0x112550] (10) reduced until the width fits w. 10 call sites, eax unused */
int  f76F14(int n);                    /* D: cards held by nation n: the signed bytes +0x01..+0x12 and +0x13 summed. 17 call sites */
void f194E4(int n);                    /* D: the Mining bonus list of nation n: 0xF68B8[6] cleared; with advance 5 (+0x29) and [0xF6740] == [0xF6744] == 0: [0xF6754] = 1, count [0xF68B4] from 1, the held commodities of 0xBDD3C {4, 12, 11, 16, 18} in 0xF68B8[count] and their index in 0xF68C0[count]; else [0xF6754] = 0. [0xF68B0] = 0. 6 call sites, eax unused */
int  f195D8(int n);                    /* D: value of nation n's hand: sum over i 1..18 of c*c*((i+1) >> 1), c = signed byte +i; with the Mining flag [0xF6754] the best of the totals with one held commodity of the list 0xF68B8[0..count) counted as c + 1. 9 call sites */
void f763FC(void);                     /* D: Trading Wrapup, if byte [0x110F74] (trades) != 0: scroll int16 [0x112544] = 0, then f764A8 and wait (dialog 0xCA5A8, or 250 / 1000 ms when f7BA64 says unattended) until -1; -0x2C / -0x2B scroll by +6 / -6. 2 callers, eax unused */
void f764A8(void);                     /* D: draw Trading Wrapup (background 16, [0xEE168] = 0x10) in the back buffer: up to 6 trades from the scroll row, 60 px apart from y 50, each "<-->", nation A (token, name, up to 3 cards) and nation B (token, name, up to 5 cards); dialog 0xCA5A8 drawn (f3B05C), f3CC14. One caller */

/* Dispatcher cases (0x3FB94's jump table 0x3F780, entry = id - 3): code is the dispatcher's local ([ebp-0x3C], -100 on
   entry); each returns what the original returns from that case. */
int  dcase_4(int code);                /* -> 0x3FBF1: f3CCBC, f13E50, f3CC14; returns code (-100) */
int  dcase_6(int code);                /* -> 0x3FC27: f19C10; returns code (-100) */
int  dcase_7(int code);                /* -> 0x3FC39: f76A9C; returns code (-100) */
int  dcase_103(int code);              /* -> 0x3FC5D: the screen saved ("junk"), [0xEE168] kept, f763FC, the screen put back into the background buffer, f3CC68, f3CCBC, [0xF52D0] = 0, [0xEE168] restored; returns code (-100) */

#endif
