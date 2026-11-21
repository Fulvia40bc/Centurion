#ifndef AIQ_H
#define AIQ_H

/* Module AIQ of docs/M6_PLAN.md (aiq.c): the computer player's queries, filters and scores, used by the movement
   planner 0x461BC (module AIMOVE) and its helpers in AICITY, AISEA and AIPLAN (milestone M7, batch 1). Nineteen
   routines; the three small formulas 0x7CAB4, 0x46A4C and 0x46B34 of the same module are formulas.c's.

   Conventions: those of ui.h, game.h, move.h and rules.h (fXXXXX = the original at 0xXXXXX, Watcom register
   arguments eax, edx, ebx, ecx, then the stack; void unless a caller reads eax). Territory lists are the callers'
   stack arrays of int32 (lea of a local at every call site, D), so they are C pointers here, as are the int32 counts
   and result cells the callers pass by address. D = read in the code, G = inference.

   Records and tables (D, offsets as the code adds them; more in aiq.c's file comment):
     area record r = territory r - 1: 0xE31B8 + 69r: +1 int8 nations there, +1 + n marker, +0x0E + n int8 tokens,
       +0x24 + n int8 tokens that may still move, +0x3B + n uint8 the AI's target mark (f46998)
     territory record t: 0xE1638 + 35t: +0 bit 0x02, +1 bit 0x40 a city site, bits 3..5 the population limit, +2 bit
       0x01; territory state t: 0xE2C18 + 3t: +0 the city's owner, +2 bit 0x01 a city, bit 0x02 the AI's site mark
     int16 0xF3580[9][157]  the AI's territory flags (row n - 1), written by f7BD5C (low byte 0x01..0x80, high byte
       0x01, 0x02, 0x04, 0x08)
     int8 0xEED84[10][160]  the opening-map scores (row n; f160EC refreshes them from the base copy 0xEF3C4)
     uint8 0xF0044[10][160] the .BPM flags (row n): 0x02, 0x04 (OVERVIEW egg 12)
     int8 0xF06C0[]         the movement order (index n)
     int32 0xF52E0[10][10]  the purchase / attitude matrix of 0x7C960 (row n, column m)
     int16 0x112A94[30][10] the pre-rolled AI table (f7BCD4; column n - 1)
     uint8 0xF4234[9][160]  the per-nation territory bytes of f12394 (row n - 1)
     AI level = bits 10..11 of word [0xE1624] (0 = Stronger) */

#include <stdint.h>

/* ---- neighbour and territory lists ---------------------------------------------------------------------------- */

void f48028(int32_t *cnt, int32_t *list, int t, int n, int filter);
                         /* D: *cnt = 0, then the land neighbours u of t (list [0xF8E7C + 4t]) that pass filter (jump
                            table 0x47FC4, 25 entries; > 0x18: none): 0 empty, no city; 1 n alone, tokens < limit (+1
                            with +0x27); 2 movable > 0; 7 n alone, no city site; 8 n alone; 9 n alone, no state bit
                            0x02; 0x0A movable > 0, no state bit 0x02; 0x0B all; 0x0C n has tokens; 0x0F n alone,
                            territory +0 bit 0x02 clear, no state bit 0x02; 0x11 n alone, tokens > limit; 0x12 flag lo
                            0x80; 0x13 empty, no city, 0xF4234[n-1][u]; 0x14 city site, flag lo 0x10 or hi 0x02, n
                            moves after the other nation there; 0x15 two or more nations, n among them; 0x17 as 0x14
                            with one nation there and n absent; 0x18 one nation there, n absent; 3..6, 0x0D, 0x0E,
                            0x10, 0x16 none. Stack argument filter (ret 4) */
void f4876C(int32_t *cnt, int32_t *list, int t, int n, int filter);
                         /* D: filter 0x16 only: appends (no reset of *cnt) the land neighbours u of t held by one
                            other nation o (f7C7C4) with .BPM flag 0x02 for n, when 0xF52E0[n][o] >= 0 or table row
                            25 + u%3 / 300 > 0xF52E0[o][n] - 200 / ((1 + o human) (5 - level)) */
void f4892C(int start, int32_t *cnt, int32_t *out, int count, const int32_t *list, int n, int filter);
                         /* D: *cnt = 0; if [0xF09E4] != 0: for each t of list[0..count) that is n's city, empty
                            without a city or held by n alone, its land neighbours u (not start, not in list, not yet
                            in out) passing filter 0 / 9 / 0x0B / 0x14 / 0x17 (as f48028) are appended to out. Three
                            stack arguments list, n, filter (ret 0xC) */
void f48D44(int32_t *cnt, int32_t *list, int n, int t, int flags);
                         /* D: reorders / filters a territory list in place, steps in this order: 0x80 city sites
                            first; always a bubble sort by score 0xEED84[n][u] descending; 0x10 keep the first
                            (count of scores >= 0); 0x20 drop u with 0xF4234[n-1][u] (byte [0x108190] = 1 for each);
                            0x200 drop cities; 0x02 scores <= score(t) first; 0x04 .BPM 0x02 first; 0x400 keep scores
                            > 105; 0x100 cities first; 0x40 flag hi 0x04 first. Nothing when *cnt <= 0. ret 4 */
void f47BA0(int32_t *cnt, int32_t *list, int n, int mode, int flags);
                         /* D: n's territories t < 157 in play: mode 3 no state bit 0x02, movable > 0, one nation there
                            or more with .BPM 0x02; mode 0x0E no state bit 0x02, city site, territory +2 bit 0x01, n
                            alone, tokens > limit + int8 +0x27; other modes none. flags 0x01 bubble sort by score
                            ascending, 0x08 those over f78D08 first. ret 4. Callers f46BB8 (3, 1), f478D8 (0x0E, 0) */

/* ---- strength and reach ---------------------------------------------------------------------------------------- */

int  f495C0(int t, int n, int mode, int32_t *enemy);
                         /* D: the largest stack of a nation 1..9 other than n in a land neighbour of t (mode 1: only
                            nations that move after n), *enemy = that nation (0 none); a neighbour with two or more
                            nations is first resolved as a trial (area saved, f7A778, [0xBB3F8] = 0, f5B588, [0xBB3F8] =
                            1, f7A7B0; the area put back after its scan). Callers 0x3580C (4), f374E0, f46CBC (2),
                            f471CC, 0x65010 */
int  f49790(int r, int t, int n, int enemy);
                         /* D: tokens to leave in t: 0 if r == 0; 1 if r < max'(limit - 1) + 1 + Metalworking(n) -
                            Metalworking(enemy) (max' turns 0 into 1 only); else min(tokens of n, limit) when table
                            row 12 + t%8 / 300 < 95 - 10 level, else 1 */
int  f49934(int t, int n, int mode);
                         /* D: 1 if n has tokens in a land neighbour of t; else (mode != 1) 1 if a ship k of n can
                            reach t (f5843C(ship territory, t, n, points) == 1); else 0. Callers 0x33ECC, f39068 */
int  f4CC5C(int t, int n, int mode);
                         /* D: tokens n needs to take t, or -1 (see aiq.c). Callers f38E54, f474A0 (2), f4BBDC, f4C9B8 */
int  f4CEE4(int t, int n, int32_t *out);
                         /* D: tokens n needs to take the city at t (see aiq.c); *out gets e + 7 + E or 7 + E. Callers
                            f39068 (2), f4C9B8 */
int  f4D110(int t, int32_t *cnt, const int32_t *list, int m, int k);
                         /* D: the tokens m can bring from list[0..*cnt) and its second ring (f4892C(t, .., *cnt, list,
                            m, 0x0B)): sum of tokens - k over areas with nations, where positive and (no city site or
                            <= 4). Stack argument k (ret 4). Callers f387C8, f4CC5C, f4CEE4 */

/* ---- map analysis, trials, scores ------------------------------------------------------------------------------ */

void f7BD5C(int n);      /* D: the AI's territory flags of n: memset(0xF3580 + 314(n-1), 0, 314), then for t = 0..156 the
                            flags of aiq.c's file comment. Callers f353C4 (0x35477), f461BC (0x46220) */
int  f7C838(int t, int n, int k, int32_t *left);
                         /* D: conflict trial: area t + 1 and the nations saved, k tokens of n added (marker and count
                            if n was not there), memset(0xF547C, 0, 10), [0xBB3F8] = 0, f5B588(t + 1), [0xBB3F8] = 1,
                            *left = nations left there, nations and area restored; returns int8 [0xF547C + n] (n's
                            losses). 0xF547C keeps the trial's losses. Callers f3580C, f474A0 (2), f4A088, f4AC50 (2),
                            f4BBDC, f4C074, f4C6E4 (2) */
int  f7B044(int n);      /* D: the score of n's position (at most 0x7FFF): cities, marked sites, areas, values, the
                            move list's attacks 0xC9; see aiq.c. Callers f61254, f62840 (2), f65010 */
int  f468EC(void);       /* D: 1 when the AI level is 0 and a human nation 1..9 has stock < 4, cities > 0, ships < 4 and
                            [0xF5470] >= 3; else 0. Caller f461BC (0x465B0) */
void f46998(int n);      /* D: for t = 0..157 (157 included, Risk 8): flag hi 0x08, score 0xEED84[n][t] > 0 and area
                            t + 1's +0x3B + n != 1: that byte = 1. Caller f461BC (0x46228) */
int  f39C08(int n);      /* D: the city sites n holds or will keep (see aiq.c; the simulated conflict is tested after
                            the area is restored, so a contested site never counts: kept). Caller f38E54 (0x38EA6) */
void f160EC(int n);      /* D: opening-map priorities: 0xF62A8[n] = 0, [0xF62B4] = 0; each t 0..159 with .BPM bit 0x04:
                            0xF62A8[n] = 1, t appended to 0xF62B8 + 5(n-1) (no bound), score 0xEED84[n][t] = base
                            0xEF3C4[n][t] + 5 if n has tokens in the area of 0xC803C[t], else the base. Callers
                            f353C4 (0x3546F), f461BC (0x46230), f46BB8 (0x46CAC) */
void f7701C(int count, uint8_t *buf, int k);
                         /* D: shuffle buf[0..count) with the pre-rolled table, column k - 1 (int16 0x112A92 + 20 row +
                            2k); buf is the caller's stack buffer. Caller f33ECC (0x342F3), count >= 2 there */

#endif
