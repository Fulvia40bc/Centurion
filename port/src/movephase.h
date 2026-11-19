#ifndef MOVEPHASE_H
#define MOVEPHASE_H

/* Module MOVE of docs/M6_PLAN.md, the phase (movephase.c): the MOVEMENT phase 0x55574 (phase 5 of the game loop
   0x54824, call at 0x54A51) with its human click loop and its helpers, the computers' movement playback (0x4D518 and
   its list sort 0x4D82C and ship playback 0x4DACC, called by AIMOVE's planner 0x461BC) and the four animated moves
   0x5A084 / 0x5A2C4 / 0x5A4BC / 0x5A750 (also called by the PBEM viewer 0x59298, dropped). The movement primitives and
   the dispatcher's movement buttons are move.h's (move.c); the ship panel and the count buttons 0x572CC shippanel.h.

   Conventions: those of ui.h, game.h, kit.h and move.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments
   eax, edx, ebx, ecx, then the stack (callee-popped); void unless a caller reads eax). D = read in the code, G =
   inference. Records: move.h (area record t + 1 at 0xE31B8 + 69(t + 1); nation n at 0xF14CC + 199n); the movement
   globals: move.c's file comment and movephase.c's. */

/* ---- the phase ---------------------------------------------------------------------------------------------------- */

/* 0x55574 MOVEMENT: message area (0xA9, 0x14E, 0x12C, 0x22); territory states' bit 0x02 cleared, cityless ones zeroed;
   TempTerr / Tempnation [0xEED74] / [0xEED78]; the movement order int16 0xF06B0[] / [0xF06AE] (the census order, nations
   without Military +0x35 first); per nation in play: snapshot for RESTART, then the computer's turn (0x353C4 / 0x461BC,
   AIMOVE, not ported) with REPLAY / CONTINUE, or the human's click loop (wait 0x55FFD). Callers: 0x54A51 (loop.c) */
void f55574(void);

/* ---- its helpers -------------------------------------------------------------------------------------------------- */
int  f57160(int n);           /* D: RESTART: "Restart %s Movement?" (YES/NO only with bit 0x08 of [0xF097C]); records 0..160 and
                                 nation n back from TempTerr / Tempnation; f3F6E4, f3F758, f3A924; 1 (0 = NO). 2 callers */
int  f57254(int n);           /* D: "Are you sure that you are Finished movement for %s?": PBEM always asks, else only with bit
                                 0x10 of [0xF097C]; 1 = finished. One caller (0x560C8) */
void f574B0(void);            /* D: count buttons 0xC4..0xC8 enabled up to int16 [0xF09E0], that one drawn pressed, the others
                                 up (back buffer). One caller (0x56777) */
int  f577B0(int n);           /* D: the first territory r - 1 (r 1..157) where n has tokens that may move (+0x24 + n), else -1 */
int  f57810(int n);           /* D: FIND UNMOVED: the next territory with movable tokens ([0x108804] == 1, cursor [0x109E18])
                                 or the next ship with points ([0x109E1C]); scans again from the start. BUG(1995) fixed
                                 unless g_original: -1 after a whole empty scan instead of an endless loop */
int  f5790C(int n);           /* D: f57940(n) || f5799C(n) */
int  f57940(int n);           /* D: 1 if any record 1..157 has movable tokens of n */
int  f5799C(int n);           /* D: 1 if a ship k < +0x40 of n has points +0x41 + k > 0 or tokens aboard +0x4D + k != 0 */
int  f57A90(int t, int n);    /* D: 1 if a ship k < +0x40 of n has the whole int16 word +0x45 + 2k == t + 1 */
int  f57AFC(int n);           /* D: 1 if a ship k < +0x40 of n has tokens aboard */
int  f57BD8(int t, int n);    /* D: a first click on t valid in the current mode: tokens ([0x108804] == 1): count +1 != 0 and
                                 one of the markers +2..+0x0A == n; ships (== 2): one of the four ship words' low byte == t+1 */
int  f582CC(int t, int u);    /* D: 1 if u is in t's land list [0xF8E7C + 4t], else -2 */
int  f58330(int t, int u, int n);   /* D: the selected ship of n at t may sail to u: points > 1: u in the coast list
                                 (or the sea list with Astronomy +0x2B); last point: the coast list only; 1, else -2
                                 (-3 with Astronomy on the last point) */

/* ---- the computers' movement playback (called by AIMOVE's 0x461BC at 0x468D6) -------------------------------------- */
void f4D518(int n, int show); /* D: plays and applies nation n's land move list 0x107694 / int32 [0x1079B4] (sorted by f4D82C
                                 when show != 0, whatever the Show AI Movement toggle), then its ship move list
                                 through f4DACC */
void f4D82C(void);            /* D: the land move list sorted for display: counts < 50 first, two bubble passes (to, from),
                                 runs with the same from / to merged (the later ones' from = 0xFF, the first's count += run - 1) */
void f4DACC(int n, int show); /* D: plays nation n's ship move list 0x1079B8 (6-byte records) / int32 [0x107D3C]: types 3
                                 EMBARK, 4 DISEMBARK, 5 DISEMBARK ALL, 10 a ship move (jump table 0x4DAAC) */

/* ---- one animated move each (al = the phase byte to show during the move, edx = the nation) ----------------------- */
void f5A084(int phase, int n, int hl, int flag, int from, int cfrom, int to, int cto, int k, int show);
                              /* D: k land tokens of n from -> to (f57CE8 with flag == 1, f57E28); with show and (PBEM or
                                 Show AI Movement, bit 0x40 of [0xF097C]) the source blinks, then the boxes cfrom / cto */
void f5A2C4(int phase, int n, int hl, int from, int color, int to, int show);
                              /* D: n's selected ship from -> to (f580CC), one point less, f576F8(from); shown as f5A084 */
void f5A4BC(int phase, int n, int i, int k, int show);
                              /* D: EMBARK k: k tokens off 0xC803C[i] onto the ship selected at f3E8FC(i) */
void f5A750(int phase, int n, int i, int k, int show);
                              /* D: DISEMBARK k: k tokens off the ship selected at f3E8FC(i) into its territory */

#endif
