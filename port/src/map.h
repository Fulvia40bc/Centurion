#ifndef MAP_H
#define MAP_H

/* The map window of AC_MAIN.EXE, drawing half of module MAP1 (docs/MAP_PLAN.md): scroll and redraw 0x3D52C, the
   territory border 0x3D18C, the land tokens 0x3DAD8, the ships 0x3DE40, the number sprites 0x3E98C / 0x3E9DC /
   0x3EA2C, the lower panel back buffer -> screen 0x3CBBC, and the two panel labels 0x787F8 (territory name) and
   0x7876C (phase name). Translations in map.c; the other half of MAP1 (scroll helpers, setters, hit test, home view,
   free) is mapdata.h / mapdata.c.

   Conventions: those of ui.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx, ebx, ecx; void
   unless a caller uses eax: all call sites checked, D). Grades: D = read in the code, G = inference. */

#include <stdint.h>

void f3D52C(int dx, int dy);           /* D: scroll by (dx, dy): column [0xF4884] + dx clamped to int16 [0xF6110]..[0xF6112], row [0xF4888] + dy clamped to 0..7; the map picture [0xF52C4] (624 x 319 at the scroll) -> back buffer [0xF52C8] at (8, 6); clip (8..0x277, 6..0x144): borders (0xFFB60), markers (0xF4D2C), cities (0xF497C), tokens and ships (f3DAD8 / f3DE40, order by phase 5 and [0x108804]); rows 0..0x144 to the screen; the mini-map 0xF520C at (0x1E1, 0x170) with its colour-15 viewport box, pasted (0x1E0..0x27B, 0x16F..0x1A3). 50 call sites, eax unused */
void f3D18C(int t, int color);         /* D: territory t's border (0x9D/0x9E/0x9F -> 0x39/0x0D/0x87) in color with fg_point, record [0xFF8D4 + 4t], points moved by the scroll (f449B8, f449E0) and kept inside the map window (8..0x277, 6..0x144). One caller (0x3D65B) */
void f3DAD8(int sx, int sy);           /* D: the land tokens (int8 [0xC1A34] >= 2), t < 157: nations k 0..12 except the current one ([0xF40F8] - 1), count area +0x0F+k in 1..29 -> sprite 0xF489C + 8(k+1) at point 0 - scroll + 7i and f3E98C; then the current player's stack with a box in colour 0xFFC00[t] (2 px when int8 [0x108804] == 1 and the colour != 0) and, in phase 5, f3E9DC(area +0x24+P) at (x+18, y). Two callers, eax unused */
void f3DE40(int sx, int sy);           /* D: the ships (int8 [0xC1A34] >= 3), t < 160: nations k 0..8 except the current one, count area +0x1C+k in 1..4 -> sprite 0xF490C + 8(k+1) at point 1 - scroll + 7i and f3E98C; then the current player's ships with a 28x18 box in colour 0xFFCA0[t] (2 px when int8 [0x108804] == 2 and the colour != 0); in phase 5 the selected ship s = area +0x2D+P: int8 nation +0x4D+s > 0 -> marker sprite, token 0xF4E3C + 8P and the value in font 0; uint8 +0x41+s in 1..5 -> f3EA2C at (x+28, y). Two callers, eax unused */
void f3E98C(int n, int x, int y);      /* D: number n as sprite 0xF4EAC + 8n, top-left (x - int32 0xC2820[n/10], y - 4) */
void f3E9DC(int n, int x, int y);      /* D: the same with the sprites 0xF4F9C + 8n (offsets 0xC2820) */
void f3EA2C(int n, int x, int y);      /* D: the same with the sprites 0xF508C + 8n (offsets 0xC282C) */
void f3CBBC(void);                     /* D: mouse hidden (f78884), fg_vbopen([0xF52C8]), fg_vbpaste(0, 0x27F, 0x145, 0x1DF, 0, 0x1DF) (the panel under the map window), fg_vbclose, f788AC. 114 call sites, eax unused */
void f787F8(int t);                    /* D: in the back buffer [0xF52C8]: font 1, fg_rect(0x1F9, 0x264, 0x1AE, 0x1BE) in colour 0x3C, the name [0xC77D8 + 4t] centred in colour 5 at (0x1F9, 0x1B1) w 0x6C (f3A0B0). One caller (0x3DA60) */
void f7876C(void);                     /* D: in the back buffer [0xF52C8]: font 1, fg_rect(0x1F3, 0x268, 0x14F, 0x15E) in colour 0x3C, the phase name [0xCA9F4 + 4 * int8 [0xF4102]] centred in colour 5 at (0x1E0, 0x152) w 0x9B (f3A0B0). 35 call sites, eax unused */

#endif
