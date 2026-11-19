#ifndef KIT_H
#define KIT_H

/* Module KIT of docs/M6_PLAN.md (kit.c): the game-screen message 0x79270, the YES/NO question 0x795A8, the CONTINUE
   message box 0x7983C, the player-button highlight 0x7844C, the button-by-id helpers 0x3B728 (redraw in the back
   buffer), 0x3B914 (pressed), 0x3BA30 / 0x3BABC / 0x3BBD8 / 0x3BCF4 (w / h / x / y), 0x3BEA0 (label colour), 0x3C1F4
   (no-flash), 0x3C48C / 0x3C530 (enable / disable on the active surface), the whole-dialog enable / disable 0x3C6A0 /
   0x3C720, the token / ship box clears 0x3F6E4 / 0x3F758 and the rectangle recolour 0x448EC. Watcom's qsort 0x8C331 is
   wqsort.h.

   Conventions: those of ui.h and game.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx, ebx,
   ecx, then the stack; dialogs as uint32_t link addresses; texts as const char *: the callers pass both data strings
   and stack buffers; void unless a caller reads eax). Grades: D = read in the code, G = inference. */

#include <stdint.h>

/* 0x79270: the common game-screen message (114 call sites). wait (eax): -560 no redraw of the screen and no panel
   (the strip erased with f3A990, no view change), -550 no wait, -500 wait for CONTINUE (0xBB800 appended to the panel
   dialog 0xF596C at (0x109, 0x1AC); unattended: f78A20(250, or 1000 with int16 [0xF40BA])), else f78A20(wait) ms.
   n (edx): the nation of the status panel (f459E4, not with t == -300) and of the home view. t (ebx): -200 home view
   of n, -300 / -250 the map as it is, t >= 0 centre on territory t with only t highlighted, other t < 0 centre on -t
   and add its highlight. text (ecx): printed with f3A9F4 unless empty. anim (stack, ret 4): 0 none; with animations
   on (bit 0x04 of [0xF097C]) -298 Zeus (f79ADC), -299 the BRPC (f79C70), any other the calamity [0xF783C] (f7A0A0);
   with animations off only -299 (f79C70). The three are module ANIM's (anim.c).
   The message area's y / h (int16 0xBB3FC / 0xBB400) are 0x14C / 0x7B during the call and restored after (D). */
void f79270(int wait, int n, int t, const char *text, int anim);
/* 0x795A8: modal YES/NO question over the screen (dialog 0xBBFD8: YES id 0xCA code -0x61, NO id 0xCB code -0x62,
   NO above YES); the current dialog [0xF8E78] disabled meanwhile; returns -0x61 or -0x62 (D) */
int  f795A8(const char *text);
/* 0x7983C: modal message with CONTINUE (dialog 0xBBBEC, id 0x2A) over the screen; unattended runs go on after
   f78A20(250, or 1000 with int16 [0xF40BA]) (D) */
void f7983C(const char *text);
/* 0x7844C: for p < [0xF40EC]: record base + p of dlg gets label colour (+0x0C) 15 if nation int16 0xF06B0[p] is the
   current [0xF40F8], else 0, redrawn (f3B4E0) in the back buffer (D) */
void f7844C(uint32_t dlg, int base);

/* By button id (16-bit compare of the id, the first match wins; none: "Button not found" through f3A7B0) */
void f3B728(uint32_t dlg, int id);              /* D: redraw the button as it is (f3B4E0) in the back buffer [0xF52C8] */
void f3B914(uint32_t dlg, int id, int pressed); /* D: pressed byte rec+0x12 = (int8)pressed, no redraw */
int  f3BA30(uint32_t dlg, int id);              /* D: the width, int16 rec+0x04 (the original loads ax only: callers movsx the word); 0 if none */
int  f3BABC(uint32_t dlg, int id);              /* D: the height, int16 rec+0x06; 0 if none */
int  f3BBD8(uint32_t dlg, int id);              /* D: x, int16 rec+0x00; 0 if none */
int  f3BCF4(uint32_t dlg, int id);              /* D: y, int16 rec+0x02; 0 if none */
void f3BEA0(uint32_t dlg, int id, int color);   /* D: label colour byte rec+0x0C = (int8)color, no redraw */
void f3C1F4(uint32_t dlg, int id, int noflash); /* D: no-flash byte rec+0x0D = (int8)noflash, no redraw */
void f3C48C(uint32_t dlg, int id);              /* D: enabled rec+0x10 = 1, redrawn (f3B4E0) on the ACTIVE surface */
void f3C530(uint32_t dlg, int id);              /* D: enabled rec+0x10 = 0, redrawn on the active surface */

/* Whole dialog, in the back buffer [0xF52C8], no state saved (unlike f3C828 / f3C940) */
void f3C6A0(uint32_t dlg);                      /* D: every record enabled (+0x10 = 1) and redrawn */
void f3C720(uint32_t dlg);                      /* D: every record disabled (+0x10 = 0) and redrawn */

void f3F6E4(void);                              /* D: memset(0xFFC00, 0, 0xA0): every token box colour cleared */
void f3F758(void);                              /* D: memset(0xFFCA0, 0, 0xA0): every ship box colour cleared */
void f448EC(int x, int y, int w, int h);        /* D: each pixel of the rectangle on the active surface through the table 0xC2720 (fg_getpixel, fg_setcolor, fg_point), column by column */

#endif
