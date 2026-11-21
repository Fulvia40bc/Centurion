#ifndef AIPLAN_H
#define AIPLAN_H

/* Module AIPLAN of M7 batch 1 (aiplan.c): the movement passes of the computer players' planner 0x461BC (module
   AIMOVE): spreading tokens, the attack and reinforcement passes, the excess-token relay, the retreat, the city
   plans' token measure and the expansion targets. Called by the planner 0x461BC and by AICITY / AISEA (0x353C4,
   0x3580C, 0x374E0, 0x37CF8, 0x396FC, 0x4D29C).

   Conventions: those of ui.h, game.h, move.h and rules.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments
   eax, edx, ebx, ecx, then the stack (callee-popped); void unless a caller reads eax). Territory lists are the
   callers' int32 stack arrays (160 entries), passed by pointer. D = read in the code, G = inference.

   Records and globals (D, offsets as the code adds them; area record r = territory r - 1 at 0xE31B8 + 69r, nation n
   at 0xF14CC + 199n, territory record t at 0xE1638 + 35t, territory state t at 0xE2C18 + 3t: move.h, rules.h,
   conflict.h):
     area +0x01 int8 nations there, +0x01+n marker, +0x0E+n tokens, +0x24+n movable tokens, +0x37 int16 (written by
       AICITY's 0x33964: the tokens a planned city keeps, G), +0x3B uint8 (written by 0x4C9B8: n's tokens at a reached
       target), +0x3B+n int8 n's AI mark of the territory (0 cleared after a move into it)
     territory +0x00 bit 0x02 coastal (G), +0x01 bits 3..5 the population limit, bit 0x40 a city site
     territory state +0x00 owner, +0x02 bit 0x01 a city, bit 0x02 a site the AI plans a city on
     nation +0x26 Metalworking, +0x51 cities, +0x5D int8 the tokens spare for cities (written by 0x478D8 / 0x47A44),
       +0x60 / +0x61.. the territory list (territory + 1), +0x98 tokens on the board, +0x9B int8 the nation's
       aggression (G: added 10x to the attack threshold of 0x4C074)
     0xF0044 + 160n + t uint8 the .BPM flags of n (bit 0x01 hold, bit 0x02 a goal, G)
     0xEED84 + 160n + t int8 the territory scores of n (refreshed by AIQ's 0x160EC)
     0xF3580 + 314(n - 1) + 2t uint8 pair of AI flag bytes (lo: 0x04, 0x10, 0x20, 0x40, 0x80; hi 0xF3581: 0x02, 0x04)
     0x112A94 int16 [30][10] the pre-rolled AI table (f7BCD4 in TAXATION; saved): row r, nation n at 20r + 2(n - 1),
       read as value / 300 (0..99)
     AI level: bits 10..11 of the word 0xE1624 ((w << 4) as 16 bits, >> 14)
     0xF547C uint8 [14] the losses of the last conflict (written by AIQ's trial 0x7C838)
     0xF06C0 int8 [14] the nations' places in the movement order (G)
     the land move list: 0x107694 + 4i (+0 from, +1 to, +2 count, +3 type), count int32 [0x1079B4] (200 records of
       room, no bound check in the original); types written here: 5, 7..10, 0x11, 0x13, 0x15, 0x17, 0x18, 0x1A, 0x22,
       0x29
     0x10819C uint8 [160] / 0x10823C uint8 [160] per-territory done flags of the spread pass; int32 0x108164 + 4n "n
       spread something"; int32 [0xC2860] = 0x107EE4: a pointer to the int32 [160] mark array (shared with the ship
       routines)
     uint8 [0xC28AC] (1 at start; 0 during 0x461BC's first spread, 1 after), int32 [0xC28B0] (the relay depth 3 / 4),
       int32 [0xC28B4] (1; 0 during 0x49A1C's trial), uint8 [0x1082DC] (0x49A1C's recursion guard), int32 [0x108194]
       (a city was planned), int32 [0x107D40] (0x4C9B8's once-per-round city attack), int32 [0x108198] (0x474A0's
       roll counter: incremented per roll, never reset, not saved: a process-lifetime static, as the original)
     0xF7C9C / 0xF7CA0 int32 the shared choice list (0x49E04 and 0x4C074 copy the territory list there) */

#include <stdint.h>

void f46BB8(int n);                       /* D: spread n's tokens: unless f46B34(n): clears 0x10819C, entry 0 of the mark
                                             array (4 bytes) and 0x10823C; passes of f47BA0(.., n, 3, 1) + f46CBC per
                                             territory until none moves (first pass reordered by f471CC with [0xC28AC]);
                                             then f160EC(n). Callers 0x353C4, 0x461BC (2), 0x4D29C */
int  f46CBC(int s, int n);                /* D: spread from s: moves n's spare tokens to the targets of f48028 / f4876C /
                                             f48D44; 1 if s had spare tokens or f474A0 moved. Callers f46BB8, f49A1C */
void f471CC(int32_t *cnt, int32_t *list, int n);   /* D: marks (int32 1 in the mark array) the sources whose spare
                                             tokens face a goal or at most one target, then reorders list: marked first
                                             (stable). Caller f46BB8 */
void f474A0(int s, int n, int32_t *avail, int32_t *ret, int cnt, const int32_t *list);
                                          /* D: attacks weak neighbours of s (pass 1 hi flag 0x02, type 0x13; pass 2 lo
                                             flag 0x10, type 0x15) when the roll table[10 + c % 10][n - 1] / 300 <
                                             90 - 20L, c = [0x108198]++. Caller f46CBC (ret 8) */
void f478D8(int c, int32_t *cnt, int n, int32_t *flag);   /* D: plan more cities (0x33964(c + 1, cnt, n, 1)) when
                                             the simulated spare (tokens - 4 - 2(cities + *cnt)) is positive.
                                             Caller 0x461BC */
void f47A44(int c, int32_t *cnt, int n, int32_t *flag);   /* D: the same for a crowded nation (tokens >= 20 and
                                             f46A4C(n)): 0x33964(c + [0x108194] + 1, cnt, n, 0). Caller 0x461BC */
int  f49A1C(int t, int n, int k, int exclude);   /* D: would t be refilled after emptying it (trial under the World
                                             State Stack, guard [0x1082DC]); 0 when the guard is set. Callers 0x3580C (2),
                                             0x374E0, 0x37CF8, 0x396FC, f46CBC, f471CC */
int  f49C88(int s, int n);                /* D: s's surplus over its limit, one token each to neighbours one below their
                                             limit (type 0x17); returns the tokens moved. Caller f46CBC */
void f49E04(int n);                       /* D: fill marked empty lands from n's territories (type 0x18), only the first
                                             target of each source. Caller 0x461BC (2) */
void f4A088(int n, int mode, int conquer);   /* D: excess tokens of each of n's territories relayed by f4A2E0(excess, t,
                                             n, 0, {t}, 1, mode, conquer). Callers 0x461BC (4), f478D8, f47A44 */
int  f4A2E0(int k, int s, int n, int depth, const int32_t *visited, int nvisited, int mode, int conquer);
                                          /* D: relocate k tokens out of s (recursive, depth < [0xC28B0]); returns the
                                             tokens moved. Callers f4A088, itself (ret 0x10) */
void f4BBDC(int n);                       /* D: retreat from contested territories n would lose (type 0x29). Caller
                                             0x461BC */
void f4C074(int n);                       /* D: attacks on neighbours held by one other nation (type 0x11), through the
                                             shared list 0xF7CA0 (see aiplan.c). Callers 0x461BC (2), f478D8, f47A44 */
void f4C6E4(int n);                       /* D: one token into each threatened territory t < 157 (type 0x1A). Caller
                                             0x461BC */
void f4C9B8(int n, int x);                /* D: n goes for the territories it marked x (area +0x3B+n): 0x3580C trials
                                             under both stacks. Callers 0x461BC (2) */

#endif
