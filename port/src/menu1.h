#ifndef MENU1_H
#define MENU1_H

/* Module MENU1 of docs/MAP_PLAN.md (menu1.c): the in-game options menu 0x42754 (the panel's SYSTEM button, id 5), the
   HELP screen 0x4290C with its file/title picker 0x42CEC, and the TOGGLES screen 0x436FC; plus the bodies of the
   dispatcher 0x3FB94's cases of this module as dcase_<id>(code), for dispatch.c to call (code = the dispatcher's
   local, -100 on entry; the return value is what the case returns).

   Conventions: those of ui.h and game.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx, ebx,
   ecx; dialogs as uint32_t link addresses; void unless a caller reads eax). Grades: D = read in the code, G = inference. */

#include <stdint.h>

void f42754(void);                     /* D: the options menu: f3C940([0xF8E78]), f3CD10, [0xF52D0] = 0; frame (0xBE, 0x50, 0x104, 0x8C) in the back buffer; dialog 0xC1A5C (SAVE GAME 0x4F, RESTORE GAME 0x50, START NEW GAME 0x51, SCORING 0x56, AUDIO/VISUALS 0xE2, TOGGLES 0x52, HELP 0x53, QUIT TO DOS 0x57, CANCEL 0x2A, SHOW PBEM 0x107) set up by the PBEM flag 0xE1624 & 0x80; pasted (0xBE..0x1C1, 0x50..0xDB); f78BA4(0xC1A5C, 0) until -1; then f3CC68, f3C8C4([0xF8E78]), f3CCBC. Its only caller (dispatcher id 5, 0x3FC12) ignores eax */
void f4290C(void);                     /* D: HELP for the current phase: f42CEC names the file and title; the file from ACIV.CT3 [0xF40DC] (or "Help file '%s' not found."); box width by trial (0x11D, +30 ..), centred; the screen under it saved ("junk" GIF in the original, memory in the port); frame, title centred with an underline, text, CONTINUE (dialog 0xBBBEC) at the bottom centre; f78BA4(0xBBBEC, 0) until -1; the region restored into the back buffer and shown (f3CC14). One caller (dispatcher id 0x53, 0x3FD7C), eax unused */
void f42CEC(char *name, char *title);  /* D: strcpy the help file name (eax, 36-byte buffer) and title (edx, 40-byte buffer) for phase byte [0xF4102] (0..21) and, in phase 15, calamity [0xF783C] - 0x12 (0..12); buffers untouched for phases 2, 9, > 21, phase 0 with [0xFFD40] not 2 or -50, and calamities out of range. BUG(1995) fixed unless g_original: phases 10 and 17 (CITY SUPPORT) get citysup.hlp instead of shipcnst.hlp (menu1.c). One caller (0x4290C) */
void f436FC(void);                     /* D: TOGGLES: frame (0xD2, 0x50, 0xDC, 0xF0) with ten labels and tick sprites (0xF51DC set, 0xF51E4 clear) for the bits of the aciv.cfg image 0xF097C..0xF0981; dialog 0xC1E48; each code from f78BA4 (-0x63, -0x7F, -0x80, -0x87..-0x8C, -0xAC) flips one bit and redraws its box on the screen; on -1 writes aciv.cfg (6 bytes from 0xF097C, "wb") if any row was clicked. One caller (dispatcher id 0x52, 0x3FD30), eax unused */

/* The dispatcher cases (jump table 0x3F780, entry id - 3; targets D). dcase_57 does not return (quit). */
int  dcase_5(int code);                /* 0x3FC0D: f42754; returns -100 (ebx = -100 kept across the call) */
int  dcase_52(int code);               /* 0x3FD26: f3CC68, f3CCBC, f436FC; falls into 0x3FD35: -1 */
int  dcase_53(int code);               /* 0x3FD6D: f3CC68, f3CCBC, f4290C; -1 (ebx = -1 kept across the call) */
int  dcase_57(int code);               /* 0x3FD91: byte [0xF5500] = 0, f44CAC (quit to DOS); never returns */
int  dcase_41(int code);               /* 0x41997: -0x63 (toggle: Restart Confirmations) */
int  dcase_6D(int code);               /* 0x41A23: -0x7F (toggle: Movement Finished Confirmation) */
int  dcase_6E(int code);               /* 0x41A37: -0x80 (toggle: Show AI Movement) */
int  dcase_E3(int code);               /* 0x4248A: -0x87 (toggle: Speedup AI Movement) */
int  dcase_E4(int code);               /* 0x4249E: -0x88 (toggle: Ship Maintenance AI Pause) */
int  dcase_E5(int code);               /* 0x424B2: -0x89 (toggle: Ship Construction AI Pause) */
int  dcase_E6(int code);               /* 0x424C6: -0x8A (toggle: City Construction AI Pause) */
int  dcase_E7(int code);               /* 0x424DA: -0x8B (toggle: Calamity AI Pause) */
int  dcase_E8(int code);               /* 0x424EE: -0x8C (toggle: City Support AI Pause) */
int  dcase_E9(int code);               /* 0x42502: -0xAC (toggle: Alternate Tool Acquisition Colors) */

#endif
