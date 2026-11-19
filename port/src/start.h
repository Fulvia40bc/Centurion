#ifndef START_H
#define START_H

/* Module START of docs/MAP_PLAN.md (start.c): main 0x11854..0x11884, the computers' coin flips 0x6B9AC, the starting-
   territory screen 0x454D0 ("%s place your token in a starting territory." / "%s is selecting its starting
   territory."), the map-screen click 0x3D434, the computer's pick 0x45F88, the message-area restore 0x3A924 and the
   .bpm opening-map loader 0x15FE0. The .bpm name itself comes from 0x14C6C (aistart.h, the other half).

   Conventions: those of ui.h and game.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx, ebx,
   ecx; dialogs as uint32_t link addresses; void unless a caller reads eax: every call site checked, D). Grades: D =
   read in the code, G = inference. */

#include <stdint.h>

void f6B9AC(void);                     /* D: for n 1..9 with int8 nation +0x9A == 2 (computer): byte +0xBC = (game_rand * 100 >> 15) < 50 (signed setl). One caller (main 0x11854), no argument, eax unused */
void f454D0(void);                     /* D: the starting-territory screen (layout in start.c): message area (169, 332, 300, 123), the player list 0xF06AE/0xF06B0, f783B8, phase byte [0xF4102] = 0x15, SAVE/RESTORE (0xC1A5C) and panel buttons 7, 6, 3, 4 disabled; per nation 1..9 in play: f14C6C + f15FE0 (the .bpm), f3A924, then a human clicks one of its own territories (0x3D434 on 0xF596C) or a computer picks (f45F88); token, area and nation records updated; buttons enabled again at the end. One caller (main 0x11868) */
int  f3D434(int x, int y, uint32_t dlg);   /* D: a click at (x, y) on the map screen: the first enabled record of dlg (ebx) with x <= x < x + w, y <= y < y + h -> f3C5D4(dlg, id), return f3FB94(id); otherwise, if y <= 0x145 (signed), return f3D2A4(x, y) (the territory, or -100); else -100. 3 callers (0x45737, 0x5602F, 0x56FC5), dlg always 0xF596C, all read eax */
int  f45F88(int n);                    /* D: the computer's starting territory for nation n (al, kept as a signed byte): byte +0x9B = (game_rand * 5 >> 15 == 1); then from best = 0x7F downwards the territories t < 157 with int8 0xEED84[160n + t] + 5 * bit 2 of 0xF0044[160n + t] == best go to the list 0xF7C9C/0xF7CA0, and the first non-empty list gives t = list[game_rand * count >> 15]. Returns t + 1 (the area record index). One caller (0x45843) */
void f3A924(void);                     /* D: restore the message area from the background: f3CD6C(int8 [0xEE168]), fg_vbcopy(x, x + w - 1, y, y + h - 1, x, y + h - 1, [0xF52CC], [0xF52C8]) with the int16 area 0xBB3FA (x), 0xBB3FC (y), 0xBB3FE (w), 0xBB400 (h). 27 call sites, eax unused (D: none reads it; the twin of f3A990, which uses the fixed rows 0x14C..0x1C6) */
void f15FE0(int n, const char *name);  /* D: load the .bpm opening map name (edx: a data string 0xF6064 + 13n or the caller's buffer from f14C6C) for nation n (eax) from ACIV.CT3 [0xF40DC] into [0xF522C] (cat_size, cat_load; not in the catalogue (-2): the loose file, 0x7D334), then 160 bytes each: +0x2B -> 0xEED84 + 160n, +0xCB -> 0xF0044 + 160n, +0x16B -> 0xEFA04 + 160n. 3 callers (0x14A38, 0x14B07, 0x45650), eax unused */

void start_slice(void);                /* port: main 0x11854..0x11884 after the nation choice: f6B9AC, f454D0, memcpy 0x640 bytes 0xEED84 -> 0xEF3C4, byte [0xF4102] = 0 (the PBEM branch 0x1187F, f12C38, is left out as dropped). Main goes on at 0x1188C */

#endif
