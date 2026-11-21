#ifndef AISHIP_H
#define AISHIP_H

/* Module AISHIP of M7 batch 2 (aiship.c): the computer players' ship maintenance 0x61254 (with its subset generator
   0x617C0) and ship construction search 0x62840 (with its combination generator 0x63D10 and the clock budget
   0x6332C), the coastal-target list 0x639E8, and the small AI parts of three phases: TAXATION's annexing 0x66B54 and
   city removal 0x66C2C, POPULATION EXPANSION's choice for a computer short of stock 0x680DC, and CONFLICT's plunder
   transfer 0x5BF30.

   Conventions: those of ui.h, game.h, move.h and rules.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments
   eax, edx, ebx, ecx, then the stack (callee-popped); void unless a caller reads eax). D = read in the code, G =
   inference. n is a nation 1..9 (record 0xF14CC + 199n), t a territory 0..159 (area record t + 1 at 0xE31B8 + 69r).

   Records and globals (D, offsets as the code adds them):
     nation +0x25 Clothmaking, +0x3E int8 treasury, +0x40 ships, +0x45 int16[4] ship territory + 1, +0x51 cities,
       +0x52.. the city list (territory + 1), +0x5C int8 AST square, +0x60 / +0x61.. the area list (territory + 1),
       +0x98 tokens on the board, +0x99 stock
     area +0x01 nations there, +0x0E + n tokens, +0x1B + n ships of n (G), +0x2D + n the last ship index put there (G),
       +0x3B + n set to 1 where n lost a city in a tax revolt (G)
     the ship move list 0x1079B8 + 6i (count int32 [0x107D3C]): +0 from, +1 to, +2 count, +3 ship k, +4 type (3 embark,
       4 unload, 5 disembark, 0x0A sail), +5 a reason code; the land move list 0x107694 + 4i (count int32 [0x1079B4]):
       +0 from, +1 to, +2 count, +3 type (aisea.h, aicity.c)
     maintenance options 0x10F590 [16] x 18 bytes: int32 keep[4], int16 score +0x10; count int32 [0x10F6B0], the
       top mask int32 [0x10F6B4] (15)
     construction options 0x10F6B8 [150] x 18 bytes: int16 score +0, int8 ships +2, uint8 site[3] +3, treasury[3] +7,
       levy[3] +0x0B, +0x0F cleared, int8 capacity +0x11; count / current int32 [0x110144]
     the construction search's globals 0x110148..0x110178 (budget counters, stage, indexes, the combination), the
       site list 0x10F30C / 0x10F310 (helpers2.c's f63950), the excluded sites uint8 0x10F220[160], the time and
       countdown words 0x10F2C0..0x10F2F0 and the thresholds int32 0x10F2F4[6] (title.c's f60A80)
   The routines' results are their records and globals; the eax each caller reads is noted. */

#include <stdint.h>

void f61254(int n);           /* D: computer n's ship maintenance: a plan with all ships (f353C4, f461BC under a World
                                 State push); a ship without a plan record (or a negative reason) is scrapped; treasury
                                 0: all scrapped; fewer treasury than used ships: each subset of 'treasury' ships
                                 (f617C0) scored by a trial plan (f78D98 the others, f461BC, f7B044), the best by a
                                 bubble sort and one game_rand among the ties; then a busy wait to 0.75 s after the
                                 start and f6106C for each ship, last first. Caller SHIP MAINTENANCE 0x60ED9
                                 (ship.c's CALL_F61254) */
int  f617C0(int n, int budget, const int32_t *tokenUse, const int32_t *scrap, int32_t *tried);
                              /* D: World State peek (f7AF28); the next untried mask m | tokenUse bits & ~scrap bits
                                 (m from [0x10F6B4] down) with exactly 'budget' bits of 0..3 becomes option [0x10F6B0]'s
                                 keep[k] (k < ships); 1, or 0 when none is left. tried is int32[16] (stack argument,
                                 ret 4). Caller f61254 (0x614F0) */
int  f62840(int n);           /* D: the construction search of nation n (computers, and humans with 4 ships): -1 when
                                 off (int16 [0xBB3F0] == 0) or nothing is better than the plan without new ships, else
                                 the chosen option's index in 0x10F6B8 (sorted). Up to 150 combinations of f63D10 tried
                                 under the World State Stack, each scored by a trial plan (f461BC, f7B044); one
                                 game_rand when options survive; a busy wait to 0.75 s after the start when it searched.
                                 Caller SHIP CONSTRUCTION 0x62197 (ship.c's CALL_F62840) */
void f6332C(int total);       /* D: the search's time budget (int16 [0xBB3E8] * 9500 / 100 hundredths): from the trial
                                 times 0x10F2CC / 0x10F2DC and the clock since [0x10F2C8], too slow a search excludes
                                 the sites of the options past a limit; else the stage-2 start index [0x10F2E0]. total
                                 = the ship budget a + b + c. Caller f63D10 (0x64361, on entering stage 1) */
void f639E8(int n, int32_t *count, int32_t *list);
                              /* D: *count = 0; every coastal city site t < 157 (terr +1 bit 0x40, +0 bit 0x02) held by
                                 one nation only, appended once for each nation k + 1 (k 0..8) with tokens there whose
                                 int8 0xF06C1[k] beats n's 0xF06C0[n]. list holds 160 entries (port guard: no write past
                                 them). Callers f61254 (0x6130D), f62840 (0x629B3); neither reads the results */
int  f63D10(int n);           /* D: the next ship combination into 0x110178 (count), 0x110174 (sites), 0x11016C
                                 (treasury), 0x110170 (levy): 1, or 0 when all stages are done. Stage 0 one ship, 1
                                 pairs, 2 triples; the stage changes re-admit sites (stage 1, with byte 0xF488C[n]), call
                                 f6332C (stage 1) and exclude the first sites of options [0x10F2E0] .. [0x10F2F0]
                                 (stage 2). Caller f62840 (0x62A50) */
void f66B54(int m, int n, int k);   /* D: computer m annexes k of n's revolting cities: each t = f20C70(m, n) - 1 (CAL1,
                                 not ported here), highlighted and centred, 500 ms, area +0x3B + n = 1, f78F04(t, n),
                                 f78E74(t, m), "%s annexes a city in %s." for 2000 ms. Caller TAXATION 0x66A74 (tax.c) */
void f66C2C(int n, int k);    /* D: computer n removes k of its own cities, the ones outside its four start territories
                                 0xC0618[n] first, each from the end of its list: highlight, 500 ms, f78F04, "%s removes
                                 a city in %s" for 2500 ms. Caller TAXATION 0x66AFA (tax.c) */
void f680DC(int n, int need); /* D: computer n with stock < need places its tokens: stock 0 (or under 80% of need) sets
                                 byte 0xF488C[n] = 1; a full expansion simulated under a World State push, f461BC and
                                 the silent f5AAA4(0) on it, the excess cut where the plan or flags say, then the
                                 placement (f57E28) of what is left; f7ABD0 frees the stack; a busy wait to 1 s after
                                 the start (not scaled by /C). Caller POPULATION EXPANSION 0x67661 (pop.c) */
void f5BF30(int n, int amount, int show);   /* D: computer n's stock -> treasury transfer after a plunder of amount
                                 (<= 3): all of it when the AST table's value for record n - 1's square is >= 2, else
                                 the ship count clamped to 0..amount; with show the UNITS TO TRANSFER box, the message
                                 and a 2500 ms wait. n is a byte (al). Caller CONFLICT 0x5BECB (conflict.c) */

#endif
