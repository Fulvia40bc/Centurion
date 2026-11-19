#ifndef CARDS_H
#define CARDS_H

/* Module CARDS of docs/M6_PLAN.md (cards.c): ACQUIRE GOODS 0x6BD98 (DrawTradingCards, phase 11 of the game loop
   0x54824, call at 0x54C6D) with its screen 0x6CB8C (DrawInitialDrawingScreen), the left column 0x6CDA0, the treasury
   box 0x6CF50, the hand strip face down / face up 0x6D010 / 0x6D0C4, the human's ACQUIRE 0x6D178 and BUY 0x6D564;
   DISCARD EXCESS 0x6E770 (phase 19, call at 0x55186) with the human's discard loop 0x6E870 and its screen 0x6EC84;
   the reshuffle of the discard piles into the stacks 0x77278 (game_rand through f76F70) with 0x77510; and the trade
   screens' shared helpers REVEAL GOODS 0x6FBDC, the POINTS box 0x6FE68 / 0x6FFC4 and the rectangle copies 0x71D30 /
   0x71E34. 0x6DB00 (can n buy a card) is formulas.h's. The computers' routines 0x6DBAC, 0x6DE04, 0x6EDA8 (and 0x6F078
   behind it) are module M7's (AI): their call sites stop the port through not_ported(address).

   Conventions: those of ui.h, game.h and kit.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax,
   edx, ebx, ecx, then the stack; void unless a caller reads eax). D = read in the code, G = inference.

   Globals (D): int16 [0xF06AE] / 0xF06B0[] the player list (0x6BD98: nations in play by cities ascending, ties by
   number; 0x6E770: ascending); int8 0x110AD0[p] the hand size of list position p; int32 [0x110AC4] cards per stack
   and city (1, 2 with the variant bit 0x40 of [0xE1624]); int32 [0x110AC8] / [0x110ACC] the row's "Acqrd" / "Buy"
   counters; int32 0xF590C[n] the stack n bought from this turn (-1 none); int8 [0xEE168] the background (10, 11 with
   8 players; 0x11 for the discard screen); int16 0x111524[14][4] the restore rectangles {x, y, w, h} (slot 13 =
   0x11158C, the hand strip); int32 [0x11254C] / [0x112550] the card spacing of f71F2C / f71FB0; int16 [0xF4096] 1 =
   a discard happened (the loop's PBEM test); dialogs 0x1106D8 (acquire: 0x8A ACQUIRE -0x32, 0x8B BUY -0x33, 0x8C the
   stack choice -0x34, 0x2A FINISHED -1) and 0x110ADC (discard: 0x8D the hand -0x35, 0x2A FINISHED) built by 0x7784C;
   card stacks 0xF06D0 + 25s and discard piles 0xF07B4 + 25s (game.h, rules.h). */

#include <stdint.h>

/* ---- the two phase routines ------------------------------------------------------------------------------------ */

/* 0x6BD98 ACQUIRE GOODS (DrawTradingCards): nobody with cities: f79270(100, first of the old list, -200, "", 0).
   Else f553C0, the player list by cities (ascending, ties by nation), f783B8(0x1106D8, [0xF5960]), the hand sizes,
   [0x110AC4], [0xEE168], f6CB8C; per list position p ([0xF40F8] = n, +0x3C = 0, 0xF590C[n] = -1): no cities: the row
   "0 0 hand" and 500 ms; a human: the treasury, REVEAL GOODS, the hand, f78BA4(0x1106D8) until FINISHED (ACQUIRE
   f6D178, BUY f6D564), the row's counters; a computer: 0x6DBAC (M7), 1500 ms, buys through 0x6DE04 (M7) while it
   buys and has 6 in the treasury, the row's counters. Last f6CDA0(-1). BUG(1995) fixed unless g_original: the
   computer's "Buy" counter (cards.c) */
void f6BD98(void);

/* 0x6E770 DISCARD EXCESS: the player list (nations in play, ascending), f783B8(0x110ADC, [0xF5964]); a nation in play
   holds more than 8 cards: f6E870, else f79270(100, first player, -200, "", 0); then, without PBEM, f77278 (the
   reshuffle, game_rand), with PBEM int16 [0xF4098] = 1 */
void f6E770(void);

/* ---- ACQUIRE GOODS ------------------------------------------------------------------------------------------------ */

void f6CB8C(void);                     /* D: DrawInitialDrawingScreen: background int8 [0xEE168], the rectangles (slot 13 x 7 / 0xD1 with 8 players, w 0x1F4 / 0x172; slots 0..3), "Acquire Goods", the four column heads, dialog 0x1106D8, f3CC14. One caller */
void f6CDA0(int p);                    /* D: the left column, one row per list entry in play (token, name colour 15 for entry p else 5, card back, hand size 0x110AD0[i]); p = -1 none. 9 call sites in f6BD98 */
void f6CF50(int n);                    /* D: the treasury box of n: token 0xF49EC + 8n at (0x203, 0x168) in the back buffer, f71E34(1), the treasury +0x3E at (0x222, 0x170) on the screen, the box pasted (0x1FC..0x269, 0x162..0x184). Callers 0x6C26E, 0x6D9BA */
void f6D010(int x, int y, int w, int n);   /* D: n's hand face down in the strip (x..x+w-1, y..y+0x2D): background into the back buffer, f71F2C, pasted with the mouse hidden */
void f6D0C4(int x, int y, int w, int n);   /* D: the same face up (f71FB0). 9 call sites (also 0x162D0, ADV) */
void f6D178(int p, int n);             /* D: ACQUIRE (-0x32) for the human n at row p: "You acquired...", [0x110AC4] cards per city from stacks 0..cities-1 (f76E50, 50 ms each, "Stack %d Empty"), f6E460, the hand face up, POINTS, the "Acqrd" counter; ACQUIRE disabled; the PBEM pre-declared buy (int16 [0xF0684 + 2n] == -0x61) or BUY enabled per f6DB00; FINISHED enabled */
void f6D564(int p, int n);             /* D: BUY (-0x33) for the human n at row p: without the variant bit 0x20 of [0xE1624] the top card of stack 8 for 18; with it "Choose stack to buy from..." and a click on a stack column (-0x34, 2(s+1)) or CANCEL (-1). BUG(1995) fixed unless g_original: four (cards.c) */

/* ---- DISCARD EXCESS ------------------------------------------------------------------------------------------------ */

void f6E870(void);                     /* D: for n 1..9 in play with more than 8 cards: a human: f6EC84, the hand, REVEAL GOODS, the hand face up, POINTS, then clicks on the hand (-0x35, hit test on x only) discard one commodity each (f771F8) until 8 are left; a computer: 0x6EDA8 (M7); others: int16 [0xF4096] = 1. BUG(1995) fixed unless g_original: no end with no commodity left (cards.c) */
void f6EC84(int n);                    /* D: the discard screen of n: titles, the two texts, FINISHED style 2 and disabled, 0x8D enabled, dialog 0x110ADC drawn, f3CC14 */

/* ---- the reshuffle ------------------------------------------------------------------------------------------------ */

void f77278(void);                     /* D: for s 0..8 with a non-empty discard pile: s 1..4 first take its non-tradable calamity out (f77510); the pile shuffled (f76F70: one game_rand per card) and put in front of the stack's cards (f77184), count + pile; the calamity, if found, in front of all, count + 1; the pile emptied. Callers 0x6E854, 0x52F33 (PBEM) */
int  f77510(int s);                    /* D: the first card of discard pile s that is one of 0xCA9E0[1..4] (0x13, 0x15, 0x17, 0x19) taken out (the gap closed, count - 1); 1 if one was found, else 0 */

/* ---- the trade screens' shared helpers ---------------------------------------------------------------------------- */

void f6FBDC(int n, int disable);       /* D: REVEAL GOODS: only without PBEM, with cards in n's hand and more than one human: the box area saved ("junk", in memory), with disable == 1 the current dialog disabled, a box with n's name, CONTINUE (0x2A) of 0xBBBEC moved to (0x109, 0xFA) and labelled "REVEAL GOODS", wait for -1; the dialog restored, the label "NOT REVEAL GDS" left, the area put back, f3CCBC. 7 callers */
void f6FE68(int x, int y, int n, int show, int color);   /* D: the POINTS box at (x, y): f194E4(n), the background restored (x..x+0x31, y-10..y+0x27), "POINTS" and f195D8(n) in color, with n != 0 the token and the treasury; show == 1 pastes it. color is the stack argument (ret 4) */
void f6FFC4(int x, int y, int n, int show);   /* D: clears the POINTS box at (x, y) from the background; show == 1 pastes it (after an empty fg_vbopen / fg_vbclose of the back buffer). n unused */
void f71D30(int s);                    /* D: restore rectangle s (0x111524 + 8s; s == -1 the hand strip 0x11158C) from the background into the back buffer. Callers in TRADE */
void f71E34(int s);                    /* D: rectangle s (as f71D30) from the background buffer to the screen (fg_vbopen of the background, fg_vbpaste, fg_vbclose). 18 call sites */

#endif
