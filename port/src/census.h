#ifndef CENSUS_H
#define CENSUS_H

/* Module CENSUS of docs/M6_PLAN.md (census.c): the census phase 0x13D44 (phase 2 of the game loop 0x54824, call at
   0x5497D) and its qsort comparator 0x13DE8 (reached only by address, D: functions.txt "p").

   Conventions: those of ui.h, game.h and kit.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx,
   ebx, ecx, then the stack; void unless a caller reads eax). D = read in the code, G = inference.

   Globals (D): int16 [0xF0698] the number of nations in the census order, int16 0xF069A[10] the order (nations 1..9
   with byte +0x9A != 0, sorted); both saved (saveload.c 0x5F1E0 / 0x5F1F7). Nation record 0xF14CC + 199n: +0x98 the
   tokens on the board (compared as int8), +0x9A the player type (0 = not in play). */

/* 0x13D44 CENSUS: f79270(100, int16 [0xF06B0], -200, "", 0) (a 100 ms home view of the first player's nation), then
   int16 [0xF0698] = the count and 0xF069A[] = the nations 1..9 in play in ascending order, then Watcom's qsort
   (wc_qsort) of the count int16 entries with f13DE8: tokens on the board descending, ties by nation ascending (D) */
void f13D44(void);

/* 0x13DE8: qsort comparator on two int16 nation numbers a (eax), b (edx): -1 if int8 +0x98 of a > that of b; 1 if
   less; on equal tokens -1 if a <= b (int16), else 1. Never 0, even for a == b (D) */
int  f13DE8(const void *a, const void *b);

#endif
