#ifndef SHIPPANEL_H
#define SHIPPANEL_H

/* The ship panel of AC_MAIN.EXE, half of module PANEL (docs/MAP_PLAN.md): the ship panel 0x5D14C, its two coast labels
   0x5DDF4, the one-territory refresh after a movement action 0x426D0 and the count buttons 0x572CC. Translations in
   shippanel.c; the other half of PANEL (status panel, overview, record lists, message strip, mirrored sprite) is
   panel.h / panel.c.

   Conventions: those of ui.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx, ebx, ecx; void
   unless a caller uses eax: every call site checked, D). Grades: D = read in the code, G = inference.

   The panel rectangle these draw in the back buffer [0xF52C8]: x 278..357 (0x116..0x165), y 370..411 (0x172..0x19B),
   restored first from the background buffer [0xF52CC]. Buttons: dialog 0xF596C (the map screen's panel), ids 0x2B..0x32
   (ship actions) and 0xC4..0xC8 (count 1..5). */

#include <stdint.h>

void f5D14C(int n, int i);    /* D: ship panel of nation n at territory index i (t = 0xC803C[i], twin = f3E89C(t)): counts n's
                                 ships (nation word +0x45+2k, low byte - 1) at t and at the twin; int16 [0x109E14] = 0; in
                                 phase 5 sets up buttons 0x2B..0x32 (one group: long labels, w 110; two groups: short
                                 labels, w 55, the second group on 0x2F..0x32) and enables them; then in every phase
                                 restores the panel rectangle and draws the ship sprite(s), the selected ship's number and
                                 its tokens, and with two groups the coast names (f5DDF4). 18 call sites, eax unused */
void f5DDF4(const char *left, const char *right);   /* D: font 0, colour 5, left centred at (0x118, 0x18E) w 0x21, right at
                                 (0x143, 0x18E) w 0x21 (f3A0B0); font 1. One caller (0x5D14C), data strings */
void f426D0(int i);           /* D: t = 0xC803C[i]; f3F698(t, area[t+1] +0x24+[0xF40F8] == 0 ? 5 : 0x0F); f5D14C([0xF40F8], i);
                                 f3D52C(0, 0); f3CBBC(). 11 call sites, eax unused */
void f572CC(int t);           /* D: int16 [0xF09E0] = 1; buttons 0xC4..0xC8 of 0xF596C enabled (f3BFBC) when area[t+1]
                                 +0x24+[0xF40F8] >= 1..5; in the back buffer 0xC4 drawn pressed (f3B7CC), 0xC5..0xC8 up
                                 (f3B870). 5 call sites, eax unused */

#endif
