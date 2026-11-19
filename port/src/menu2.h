#ifndef MENU2_H
#define MENU2_H

/* Module MENU2 of docs/MAP_PLAN.md (menu2.c): the AUDIO/VISUALS preferences box 0x43E88, the "Scoring History" graph
   0x44D5C, the nation's map view behind panel button 8 (0x44A34), the WAV loader 0x7B504 (sample slots; the Miles AIL
   calls are port no-ops), and the bodies of the dispatcher's cases 8, 0x56 and 0xE2 as dcase_ functions for
   dispatch.c to call.

   Conventions: those of ui.h and game.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx, ebx,
   ecx; void unless a caller reads eax: every call site checked, D). Grades: D = read in the code, G = inference. */

#include <stdint.h>

void f43E88(void);                     /* D: AUDIO/VISUALS: box (0xD2, 0x50, 0xDC, 0x64) in the back buffer, rows "Sound effects" / "Volume", "Music" / "Volume", "Animations" with the tick sprites 0xF51DC (on) / 0xF51E4 (off) and the volumes 0..9 in sunken boxes; dialog 0xC2234 until CONTINUE (-1). Codes: -0x7B effects (bit 0x01 of 0xF097C), -0x84 / -0x83 effects volume -1 / +1 (bits 7..13 of word 0xF097E; test sample f7B504(1, [0xF549C])), -0x7C music (bit 0x02), -0x86 / -0x85 music volume -1 / +1 (bits 0..6), -0x7D animations (bit 0x04); effects and animations only with word [0xF40AE] == 0 (the low-XMS switch). On exit: the test sample's block freed and its name 0xF6158 cleared; aciv.cfg (6 bytes from 0xF097C) written if anything changed. One caller (dispatcher case 0xE2, 0x3FD58), which keeps its own -1 in edx */
void f44D5C(void);                     /* D: Scoring History: the current round's scores into int16 0xEE9F0[n][r] (BUG(1995) fixed unless g_original: the column is not clamped to 44 there), the graph (100-rounded scale, rounds capped at 44, legend sorted by score), CONTINUE (0xBB800 moved to (265, 455)) until -1, then the background buffer back on the screen. One caller (case 0x56, 0x3FD11), which keeps its own -1 in edi */
void f44A34(void);                     /* D: panel button 8: background 3 (egypt5.gif) into the back buffer with [0xEE168] = 3, the screen saved ("junk": kept in memory in the port), the phase title, CONTINUE 0xBB800 appended to the panel 0xF596C at (265, 425), panel buttons 5, 3, 7, 6, 4, 0x101 disabled, f783B8, the status panel and the home view of nation [0xF40F8]; f78BA4(0xF596C, 0) until -1; then all undone and the saved screen shown again. One caller (case 8, 0x3FC4B), eax unused */
void f7B504(int s, const char *name);  /* D: load the WAV name (a data string or a caller's stack buffer, hence a C pointer) into sample slot s (0..2) if digital sound is on ([0xF625C]): the slot's old block [0xF6260 + 4s] freed when its name 0xF6118 + 64s is set; the name copied there; the file from ACIV.CT5 [0xF40E4] (or the loose file when cat_size gives -2) into a new block (port: a load_heap handle); AIL_set_sample_file (port no-op: fails only without a block, then the "%s\nEffects file:%s,(%d) \n" quit), volume, loop count 1. Does not start the sample. 8 call sites, none reads eax */

/* AIL's file helpers, exported for loop.c's sample-slot free (0x548B1 / 0x548C0); newgame.c and saveload.c keep
   static copies bound to their own blocks (they do not include this header) */
long f7D2E8(const char *name);         /* D: the file's size, or -1 ([0xCAC68] = 3 no file, 5 no length) */
void f7CB9D(uint32_t handle, long size);   /* D: free a locked block (port: a load_heap handle; 0 = nothing) */

/* Dispatcher cases (bodies of 0x3FB94's jump-table targets; code = the dispatcher's local [ebp-0x3C], -100 on entry;
   each returns what the case returns) */
int dcase_8(int code);                 /* D: 0x3FC4B: f44A34, returns code (-100) */
int dcase_56(int code);                /* D: 0x3FD07: f3CC68, f44D5C, returns -1 (SCORING) */
int dcase_E2(int code);                /* D: 0x3FD49: f3CC68, f3CCBC, f43E88, returns -1 (AUDIO/VISUALS) */

#endif
