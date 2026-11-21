#ifndef AICITY_H
#define AICITY_H

/* Module AICITY of docs/M6_PLAN.md (aicity.c): the computer players' city planning and token gathering, nine
   routines of the movement planner f461BC (M7 batch 1): 0x337C8, 0x33964, 0x33ECC, 0x3433C, 0x348B0, 0x353C4,
   0x3566C, 0x3580C (4,364 bytes) and 0x79128.

   Conventions: those of ui.h and game.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx, ebx,
   ecx, then the stack; void unless a caller reads eax). A pointer argument points to a caller's stack local (every
   call site passes lea [ebp-..], D); n is a computer nation 1..9 (record 0xF14CC + 199n), t a territory 0..156 (area
   record t + 1). D = read in the code, G = inference. The data each routine reads and writes is listed in aicity.c. */

#include <stdint.h>

int  f337C8(int n);                    /* D: how many new cities n aims for: 0..3 (cities + it <= 9); below its token
                                          target (stock / 2, -2 for Crete, -5 more by the AI-table roll 0x112AE2[n]):
                                          1 if tokens > 21 or one of n's areas holds 2+ nations, else 0. Callers
                                          0x3549F, 0x38A5F, 0x46305 (all read eax) */
int  f33964(int goal, int32_t *count, int n, int mode);
                                       /* D: plans up to goal - *count new city sites for n: candidates f33ECC(mode),
                                          per site f3433C then a trial f3580C(need, t, n, 0, need) under a World State
                                          push (kept: f7AD24; failed: f7AC60, move-list counts restored, failure
                                          cache 0xF8558[n-1][t] = result); then the Architecture retry. *count +=
                                          the number planned; returns it (goal when goal <= 0). Callers 0x354BA,
                                          0x465D7, 0x465EB, 0x4666F (mode 0), 0x47A23 (mode 1), 0x47B94 (mode 0) */
void f33ECC(int32_t *cnt, uint8_t *list, int n, int mode);
                                       /* D: candidate sites {t, priority} (2 bytes each) for n, sorted by priority
                                          descending, each run of equal priority shuffled by f7701C. Caller 0x33A52 */
int  f3433C(int t, int n);             /* D: tokens n still needs at t to build there (>= 0), or < 0 when not feasible
                                          (-1, -999, or -(threat + enemy tokens)); may set [0x107EDC] / [0x107EE0]
                                          (Architecture). Callers 0x33ACE, 0x33E7C */
int  f348B0(int t, int n, int32_t *enemy, int32_t *maxb);
                                       /* D: the enemy units that can reach t before n builds (nations moving after
                                          n, their predicted positions f3566C, by land, two steps with Roadbuilding,
                                          by ship); *enemy = the strongest nation (written only when one counts),
                                          *maxb = the largest raw count (always written). Caller 0x3444E */
void f353C4(int n);                    /* D: predicts the moves of the nations after n that own ships (f460C0, f46158,
                                          f160EC, f7BD5C, f337C8, f33964 mode 2, f46BB8 under a World State and Move
                                          List push) and keeps them in the shadow tables: nation k -> 0xF09E8 + 199k,
                                          the area columns 0..8 -> 0xEB3F0 + 69r. Callers 0x55A52 (movephase.c),
                                          0x61322, 0x62981 (SHIPAI) */
void f3566C(int k);                    /* D: nation k's predicted record and area column k-1 back from the shadow
                                          tables. Caller 0x34964 */
int  f3580C(int need, int t, int n, int mode, int s);
                                       /* D: moves n's tokens toward t until need is met (move records into 0x107694
                                          / [0x1079B4]); returns the need left (0 = done; mode 0 can return -1); s is
                                          the stack argument (ret 4), read only by mode 2. Callers 0x33B7D, 0x33DDC
                                          (mode 0), 0x39643 (mode 2), 0x4CB4F (mode 1) */
int  f79128(int t1, int t2, int n);    /* D: the first land neighbour of t1 that is also one of t2 and usable by n (its
                                          own city, empty land, or n alone there), or -1. Caller 0x36241 */

#endif
