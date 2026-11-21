#ifndef AISEA_H
#define AISEA_H

/* Module AISEA of docs/M6_PLAN.md (aisea.c): the computer players' ship transport and voyages, part of the movement
   planner 0x461BC (M7, batch 1). Literal translations; aisea.c's file comment has the address map, the data layouts,
   the stack discipline and the 1995 fixes.

   Conventions: those of ui.h and game.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx, ebx,
   ecx, then the stack; void unless a caller reads eax; pointers are the callers' int32 stack locals, passed with lea).
   D = read in the code, G = inference. n is a computer nation 1..9 (record 0xF14CC + 199n), t a territory (0..159;
   157..159 the second coasts, 0xC803C maps them to 57 / 13 / 135), k a ship index 0..3.

   Results the callers read (D):
     route records 0x108820 (route.h): +7 the tokens a route carries (f36918), +8..+13 the per-stop pickups (f374E0;
       negative = embark, positive = disembark after f65010), int16 +0x0E the route's score (f65010)
     the ship move list 0x1079B8 + 6i, count int32 [0x107D3C]: +0 from, +1 to, +2 count, +3 ship k, +4 type (3 embark /
       load, 4 unload, 5 disembark, 0x0A sail), +5 a reason code (f36918: byte 0xC1938[code]; f4AC50: 0x2B leg 1, 0x2C
       leg 2; f648C8: 6; f65CB0: 0x2A)
     the land move list 0x107694 + 4i, count int32 [0x1079B4]: {from, to, 1, 0x10} (f643C8) */

#include <stdint.h>

void f36918(int n, int t, int32_t *need, int code);   /* D: carry up to *need tokens of n to land territory t by
                                                 ship: a trial per ship under the World State Stack (one Move List
                                                 Stack level per ship), then the plans replayed for real in order of
                                                 their stop counts; *need goes down by the tokens landed. code
                                                 (movsx cl) indexes the byte table 0xC1938 for the move records' +5.
                                                 Callers AICITY f3580C (0x36350), AIMOVE f37CF8 (0x384DF, code 0x0F),
                                                 f396FC (0x39B4E, code 0x27) */
int  f374E0(int j, int t, int n, int max, int a);   /* D: the tokens (<= 5, <= max) route j of 0x108820 can collect
                                                 along its stops for a trip to t; writes the per-stop pickups to +8..
                                                 +13 and the caches 0xF8418 / 0xF84B8, [0xF8414] = 1 when it takes;
                                                 every trial edit undone (nation n and the area records). a (stack,
                                                 ret 4) goes to f495C0. Callers AICITY f348B0 (2), f36918, f4D29C */
int  f4AC50(int t, int n, int32_t *cnt, int mode);  /* D: ship n's tokens from t to free land: loads on the way to
                                                 t (leg 1), unloads on the way on (leg 2); returns the tokens landed,
                                                 *cnt lowered by them. mode > 1 puts *cnt tokens on t first and takes
                                                 them off at the end. Caller AIPLAN f4A2E0 (0x4A59B) */
int  f4D29C(int t, int m);                     /* D: the most tokens one of m's ships can bring to t (a trial under
                                                 both stacks; nothing kept but the caches and the route table).
                                                 Callers AIMOVE f37CF8 (0x37E6B), AIQ f4CEE4 (0x4CFC9) */
void f643C8(int n);                            /* D: gather n's tokens next to its marked coastal city sites, then
                                                 f648C8 with flag 0x10 (redone with the reduced needs when the ships
                                                 cover part of it). Caller AIMOVE f461BC (0x466E9) */
void f648C8(int n, int flags);                 /* D: for each ship of n with full movement points: a destination
                                                 (f64BB8), its routes (f58888), the best (f65010); the route's
                                                 embarks, disembarks and hops done and listed. flags 0 or 0x10.
                                                 Callers AICITY f3580C (0x360D6, 0x365F0), AIMOVE f461BC (0x466F5,
                                                 0x467BD), f643C8 (3) */
int  f64BB8(int k, int mp, int n);             /* D: ship k's destination (a free coastal territory) or -1. Caller
                                                 f648C8 */
int  f65010(int n, int flags);                 /* D: score the routes of 0x108820 (+8.. pickups, +0x0E score,
                                                 sorted best first) and pick one (pre-rolled table) or -1. Caller
                                                 f648C8 */
void f65CB0(int n);                            /* D: idle ships with points left sail toward n's largest stacks
                                                 (the moves listed, the state restored). Caller AIMOVE f461BC
                                                 (0x467D6) */

#endif
