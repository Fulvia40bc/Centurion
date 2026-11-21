#ifndef AICAL1_H
#define AICAL1_H

/* Module AICAL1 of M7 batch 2 (docs/M6_PLAN.md, the computer parts of CAL1, CAL3 and MONO): which city a computer
   loses or takes in the city calamities, which volcano erupts and which city an Earthquake hits, and the computers'
   Monotheism. Literal translations (aical1.c); D = read in the code, G = inference.

   Conventions: those of ui.h / game.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx, ebx,
   ecx; void unless a caller uses eax). The human sides are cal1.c (Treachery 0x20564, Superstition 0x23574, Slave
   Revolt / Civil Disorder / Iconoclasm 0x237A8 / 0x23ACC, Piracy 0x241D8), cal2.c (Flood 0x2D4F0), cal3.c (Volcano /
   Earthquake 0x1E7A0, 0x1F524) and mono.c (0x21914); their static stubs of these names are to be replaced by an
   #include "aical1.h" when the modules are linked. Other callers: CAL2's 0x260C4 (0x213C0), CAL4's 0x2D10C and
   TAX's 0x66B54 (0x20C70).

   Records: rules.h (nation, territory state), conflict.h (area record +0x3B + n marks). The choice list is int32
   0xF7CA0[] with its count int32 [0xF7C9C]. No game_rand, no stack pushes in these routines (D). */

#include <stdint.h>

/* CAL1 */
int  f20C70(int p, int v);          /* D: the city of v that p takes by Treachery, territory + 1. p == v: f213C0(byte
                                       v +0x3D). p's city count other than 9: v's list +0x52 + i from i = 9 down to 0
                                       (index 9 = byte +0x5B, one past the list): the first city with a land neighbour
                                       whose area marker +1 + (p's +0x3D) is set, else the same over the coastal
                                       neighbours, else the first coastal city (territory +0 bit 0x02), else the list's
                                       last entry (+0x51 + count). p with 9 cities: if v's AST square +0x5C > p's
                                       (signed byte) the first city ascending whose territory (area byte +0 - 1) lacks
                                       +1 bit 0x40, else the last one descending that has it; else the last entry.
                                       Callers 0x206E6, 0x2D13A (CAL4), 0x66B80 (TAX) */
int  f213C0(int n);                 /* D: computer n's (signed byte) own city to reduce, territory + 1: the cities and
                                       their population limits (bits 3..5 of territory +1) bubble-sorted by limit
                                       descending (stable); city 0 if its limit is above city 1's and its territory
                                       has +1 bit 0x40; else cities 1 / 2 the same way when city 0 lacks the bit and
                                       city 1 has it; one city: city 0; else the pair's int8 sums of n's tokens in the
                                       land neighbours plus the "coastal" sum (the list read past its end, aical1.c),
                                       the first of the pair only if its sum is larger. Callers 0x20FA0, 0x2358C,
                                       0x23A3C, 0x23DA8, 0x2414A, 0x264A7 (CAL2) */
void f24EB8(int c, int v, int k);   /* D: computer c gives v's coastal cities to the Pirates (nation 10): 0xF7AC4[0..11]
                                       = 0, [v] = 1, f28CFC(v, 0x1E, 0); v's entries: 1 or 2 taken in list order, more
                                       than 2 two picked by f2E1D0(v, 0x1E); each f78F04(t, v), f78E74(t, 10), area
                                       t+1 +0x3B + v = 2 and its +0x3C.. marks 1 -> 2, "%s is overrun by the Pirates."
                                       through f79270(2500, or -500 with bit 0x04 of byte [0xF0980], v, t, text, 1).
                                       BUG(1995) fixed unless g_original: k (the room left) is ignored (aical1.c). One
                                       caller (0x243FC) */
void f252BC(int c, int v, int k);   /* D: computer v picks up to k secondary victims' coastal cities: the nations 1..9 in
                                       play with cities other than v and c, bubble-sorted by int32 0xF52E0[10v + m]
                                       descending; for each while k > 0: t = f25548(m); t >= 0: f78F04(t, m), f78E74(t,
                                       10), area marks for the old owner, "Pirates overrun %s as a secondary victim."
                                       through f79270 as above, k - 1. One caller (0x2462B) */
int  f25548(int n);                 /* D: the coastal city of n the Pirates take: n's coastal cities (territory +0 bit
                                       0x02) founded this round (state +1 == int16 [0xF40FC]) as {t, n's tokens in its
                                       land neighbours}, bubble-sorted by limit ascending (unsigned bytes), then by sum
                                       ascending; the first t; if there is none the same over the other coastal cities;
                                       -1 if n has no coastal city. One caller (0x25421) */
int  f2E1D0(int n, int c);          /* D: computer n's city in the list 0xF7CA0 for calamity c: 0x19 (Flood): from n's
                                       last city down, the first list entry that is the city and not one of n's four
                                       preferred sites (0xC0618 + 16n) unless it is the most threatened entry (n's
                                       tokens in the land neighbours, the first strict maximum above 0); else the entry
                                       past the count if it matches a city; 0x1E (Piracy): the first of n's cities in
                                       the list; otherwise / none: list[0]. Callers 0x2529F (Piracy), 0x2D8C7 (CAL2,
                                       Flood) */

/* CAL3 */
int  f1EF28(int n, int32_t *scores);   /* D: computer n's volcano among tied ones (scores = int32[3] of the caller):
                                       0, 1 or 2. An int8 sum over the volcano words 0xBDE18..0xBDE1F (v = 1, 2) of v
                                       per nation k 1..11 that has tokens in area record t (t, not t + 1) other than n,
                                       or per nation when territory t - 1 holds a city of an owner other than n and 10;
                                       1 -> 0, 2 -> 1; else by the scores and Crete (n == 5), the tie of 0 and 1 by the
                                       tokens in area records 34, 35, 54 against 76. One caller (0x1EAAB) */
void f20068(int n);                 /* D: the Earthquake pairs 0xF7300 (8-byte records, [0xF7C9C] of them) bubble-sorted
                                       descending by int16 +6, then (stable) by int16 +4, swapped through the global
                                       temp 0xF77D8; nothing with fewer than 2 (n unused). One caller (0x1FD0E) */
int  f201B8(int n);                 /* D: computer n's city an Earthquake hits when no pair exists: int16 scores of n's
                                       tokens in the land list words 0..count-1 of each list entry (the count word
                                       included, the last neighbour left out); no equal scores among the first count-1:
                                       the last entry scoring 0, else the first highest (above 0; none: the word
                                       [0xF7C9C] itself); with equal scores the first highest, else entry 0. Returns the
                                       territory. One caller (0x1FDB3) */

/* MONO */
void f21E8C(int n);                 /* D: computer n converts by Monotheism (candidates 0xF7CA0 from f227B0): with fewer
                                       than 9 cities and board tokens / 2 >= cities + 1, the candidate cities sorted by
                                       int8 0xEED84[160n + t], then int32 0xF52E0[10n + owner] (descending, bubble):
                                       the first city changes hands (area +0x3B + owner = 1, f78F04, f78E74); otherwise
                                       the other candidates with their largest stack of nations 1..9 (looked at only
                                       when nation "candidate index + 1" is in play: the slip in aical1.c), sorted by
                                       0xEED84, 0xF52E0[10n + nation], the stack: every nation's tokens in the first
                                       territory become n's (marks +0x3B + k = 1, f57CE8 / f57E28, board and stock).
                                       "%s converts %s by Monotheism!" (mono_converts_fmt) through f79270(-550), f78A20
                                       (300), then f79270(-500) after the change. BUG(1995) fixed unless g_original:
                                       no record (aical1.c). One caller (0x219C8) */

#endif
