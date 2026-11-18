#ifndef PANEL_H
#define PANEL_H

/* Module PANEL of docs/MAP_PLAN.md, first half (panel.c): the nation status panel 0x459E4, the whole-board overview
   0x3E374 (ZOOM), the message-strip eraser 0x3A990, the mirrored sprite 0x3AFF4, the player button labels 0x783B8 and
   the append / remove of a dialog's records to / from the map screen's panel dialog 0xF596C (0x7853C / 0x78664).
   The other half (the ship panel 0x5D14C and its helpers) is shippanel.h.

   Conventions: those of ui.h and game.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx, ebx,
   ecx; dialogs and sprite records as uint32_t link addresses; void unless a caller reads eax: the call sites were
   scanned, a few tail positions are G). Grades: D = read in the code, G = inference. */

#include <stdint.h>

void f459E4(int n);                    /* D: nation status panel for nation n: f3CD6C(int8 [0xEE168]), the panel (0x3C..0x9A, 0x162..0x1C5) from [0xF52CC] into the back buffer [0xF52C8]; for n 1..9 the name box (0x3A..0x9C, 0x14F..0x15E, colour 0x3C, name 0xC768C[n] in colour 5), IN STOCK / ON MAP / TREASURY in font 0 colour 0x9B, sprites and numbers in font 1 colour 5 (tokens +0x99, cities 9 - +0x51, ships 4 - +0x40; tokens +0x98, cities +0x51; treasury +0x3E); the panel rows pasted to the screen. 51 call sites, eax unused (G for tail positions) */
void f3E374(int P);                    /* D: the whole-board overview for nation P in the back buffer [0xF52C8]: the zoom picture 0xC1A38[[0xF40F0]] from CT1 [0xF40D4] at (8, 6); if int8 [0xC1A34] > 1: cities, ships, token stacks with counts (font 0, then font 1); fg_vbclose, f3CC14. BUG(1995) fixed unless g_original: the ship loops order by nation P-1 instead of P (panel.c) */
void f3A990(void);                     /* D: erase the message strip: f3CD6C(int8 [0xEE168]), fg_vbcopy(x, x + w - 1, 0x14C, 0x1C6, x, 0x1C6, [0xF52CC], [0xF52C8]) with x = int16 [0xBB3FA], w = int16 [0xBB3FE]. 13 call sites, eax unused */
void f3AFF4(uint32_t spr, int x, int y);   /* D: draw sprite spr mirrored left-right, top-left at (x, y): fg_move(x, y + h - 1), fg_flpimage(data, w, h) on the active surface. One caller (0x5D14C) */
void f783B8(uint32_t dlg, int base);   /* D: for p < [0xF40EC]: n = int16 [0xF06B0 + 2p]; record base + p of dlg gets label 0xC768C[n] (+0x08) and label colour 0 (+0x0C); byte [0xF06C0 + n] = p. 20 call sites, eax unused */
void f7853C(uint32_t dlg);             /* D: append dlg's records (20 bytes each) after the records of 0xF596C, then int16 [0xF596C] += int16 [dlg]. The twin of f785D4 with the destination fixed. 33 call sites, eax unused */
void f78664(uint32_t dlg);             /* D: remove dlg's records from 0xF596C: the first record of 0xF596C whose id (+0x0E) equals the id of dlg's first record; the records after the int16 [dlg] removed ones move down; int16 [0xF596C] -= int16 [dlg]; nothing if the id is not found. 31 call sites, eax unused */

#endif
