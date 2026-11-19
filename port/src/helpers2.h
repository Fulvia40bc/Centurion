#ifndef HELPERS2_H
#define HELPERS2_H

/* The small helpers of the game loop (docs/M6_PLAN.md "Mistral candidates" items 5-7, TASK 16): trade
   bookkeeping, ship counts, the qsort comparators and two calamity helpers. None draws, waits for input or calls
   game_rand; they read and write only data-object bytes. Two call ported helpers from rules.h (f75CA8, f75D84,
   f78D08). Prototypes in the original's register-argument order (eax, edx); see helpers2.c for each address. */

#include <stdint.h>

int  f75BD4(int c);                          /* 0x75BD4: 1 if calamity card c is tradable (c > 0x12: table 0xC95C5+c) */
void f75C08(uint32_t a, uint32_t b);         /* 0x75C08: swap the 5+5 offered cards between records a and b */
void f75DF0(int n);                          /* 0x75DF0: clear n's own offer and delete its counter-offer rows */
void f75EB4(int a, int b);                   /* 0x75EB4: f75DF0 for a, then for b, when each has an offer */
void f75F08(int a, int b);                   /* 0x75F08: delete the counter-offer rows made by a or b */
void f63950(int n);                          /* 0x63950: list n's coastal territories with 2+ tokens at 0x10F310 */
int  f63AF0(int n);                          /* 0x63AF0: how many ships n can still buy (2 treasury each) */
int  f63B7C(int n);                          /* 0x63B7C: can n buy one more ship with the tokens it holds */
int  f63C6C(int n);                          /* 0x63C6C: can n buy a ship needing 3+ tokens in one area */
int  f2EC38(int n, int L);                   /* 0x2EC38: the Epidemic helper: losses n can take, cities first */
int  f2C778(int n);                          /* 0x2C778: the computer's Civil War faction choice, 12 or 13 */

int  f3530C(const void *a, const void *b);   /* 0x3530C: qsort comparator: tokens collected (+8), descending */
int  f35368(const void *a, const void *b);   /* 0x35368: qsort comparator: tokens carried (+7), descending */
int  f65C4C(const void *a, const void *b);   /* 0x65C4C: qsort comparator: route score int16 (+0x0E), descending */

#endif
