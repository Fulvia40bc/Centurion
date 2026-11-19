#ifndef ADV_H
#define ADV_H

/* Module ADV of docs/M6_PLAN.md (adv.c), the human side: ACQUIRE CIVILIZATION 0x162D0 (phase 18 of the game loop
   0x54824, call at 0x55113; debug tag "Acquire Civ") with its own click loop, the affordability pass 0x17EDC (also
   called by the computers' card buying 0x6DE04, CARDS / M7), the purchase screen 0x1818C, the still-needed sum
   0x18AEC, the hand strip 0x18C68, the number boxes 0x18DD8, the goods strip 0x19170, and Tool Acquisition Wrapup
   0x1A3A0 (phase 18 at 0x55153, and phase 20 at 0x551F8 in PBEM). 0x176BC and 0x193AC are formulas.h's. The
   computers' routines 0x1A888, 0x177EC, 0x1B59C and 0x19700 (ADVAI, M7) and the CARDS helpers 0x6FBDC / 0x6D0C4 stop
   the port through not_ported(address) (adv.c's static stubs).

   Conventions: those of ui.h, game.h and kit.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax,
   edx, ebx, ecx, then the stack; void unless a caller reads eax). D = read in the code, G = inference.

   Globals (D): dialog 0xF634C (int16 count, 20-byte records from 0xF634E; built at a new game by newgame.c from
   0xBD554): advances 0x8E..0xA5 (code -0x3E, the dispatcher stores id - 0x8E in int32 [0xF6758]), ACQUIRE 0xA6
   (-0x3F), treasury arrows 0xA7 / 0xA8 (-0x40 / -0x41), the goods strip 0xA9 (-0x42), Mining arrows 0xAA / 0xAB
   (-0x43 / -0x44), the hand strip 0x75 (-0x1D), RESTART 0x41 (-0x63), FINISHED 0x2A (-1); int32 [0xF6758] / [0xF675C]
   the advance selected / before; int32 [0xF6760] the hand value; int32 0xF6764[24] the credits; int8 0xF67C4[24] the
   states (0 not affordable, 1 owned, 2 affordable); int8 0xF67E4[19] the goods toward purchase; int32 [0xF67DC]
   still needed (< 0: excess), [0xF67E0] treasury used, [0xF68C8] carry forward, [0xF6740] Mining bought, [0xF6744]
   Mining used, [0xF6750] the computer's treasury, int8 [0xF6754] the Mining flag, int32 [0xF68B0] / [0xF68B4] the
   Mining choice / count with int8 0xF68B8[] / 0xF68C0[] (info.h f194E4), [0xF68AC] the width of "Still needed";
   int8 0xF6D54 + 19n + i the cards n spent; int16 0xF6E12 + 10n [5] the advances n bought, int16 0xF6E76 + 2n their
   count; int16 0xF409A[n] the Grain set aside against Famine (CAL2); [0xEED78] the heap handle of the record backup;
   int16 [0xF4092] 1 = something was affordable; the toggle bit 0x01 of [0xF0981] (Alternate Tool Acquisition
   Colors). Tables: 0xBD480 int16 costs, 0xBD4B0 int16 prerequisites, 0xBD240 int8[24][24] credits (row = advance
   held), 0xBD4E0 int16 frame colours, 0xC7DC4 / 0xC7E24 / 0xC7E9C names / upper-case names / descriptions, 0xBDD44
   the metal names. */

#include <stdint.h>

/* 0x162D0 ACQUIRE CIVILIZATION: the player list, f783B8(0xF634C, [0xF5968]), the record backup [0xEED78] = malloc(199),
   [0xF68AC], RESTART disabled, background 0x12; for n 1..9: the record backed up, the discard piles copied, the spent
   cards cleared, [0xF40F8] = n, carry / Mining flags 0; in play: the Grain set aside subtracted from +0x08, a human gets
   the screen and its click loop (nothing affordable: skipped), a computer buys up to three advances (0x1A888 / 0x177EC
   / 0x1B59C / 0x19700, M7); the Grain added back; for every n +0x5E = f17760(n). Then the backup freed and, if nobody
   could buy, f79270(100, first player, -200, "", 0). BUG(1995) fixes unless g_original: four (adv.c) */
void f162D0(void);

/* 0x17EDC: the states of nation n's 24 advances (0xF67C4) and their labels on 0xF634C (colours only with draw == 1).
   budget == -1: [0xF6760] = f195D8(n), the credits 0xF6764 from the owned advances, budget = carry + hand + treasury
   + the advance's credit; else the budget as given and no credit. Returns 1 (and int16 [0xF4092] = 1) when one is
   affordable with its prerequisite held. Callers 0x162D0 (5 sites), CARDS's 0x6DE04 (AI) */
int  f17EDC(int n, int budget, int draw);

/* 0x1818C: the Acquire Civilization Tools screen of n in the back buffer, then shown: mode 0 the hand face down, 1
   face up with POINTS; the frames of the advance buttons; with a selection [0xF6758] >= 0 its name, description and,
   if affordable, record n copied to record 0, the goods cleared and the purchase boxes; the Famine Grain strip; the
   buttons enabled for an affordable selection; ACQUIRE / FINISHED by [0xF67DC] >= 0. 4 call sites in 0x162D0 */
void f1818C(int n, int mode);

/* 0x18AEC: still needed for the selection = cost - min(credit, cost) - goods - treasury used - carry; an excess with
   treasury in use gives treasury back (not below 0) and redraws its box; the label "Still needed" / "Excess"; the
   treasury + arrow 0xA8 enabled while need > 0. Returns need. BUG(1995) fixed unless g_original (adv.c) */
int  f18AEC(void);

void f18C68(int n);   /* 0x18C68: n's hand strip face up and POINTS redrawn from the background and pasted (caller passes 0, record 0) */
void f18DD8(int n);   /* 0x18DD8: the Goods, From Treasury, Still needed / Excess, Mining and treasury-left boxes of n redrawn and pasted; ACQUIRE / FINISHED by [0xF67DC] > 0 */
void f19170(void);    /* 0x19170: the goods toward purchase 0xF67E4 as tiny cards from (0x10A, 0x62), spacing [0xF6738] / overlap [0xF673C] fitted to 0x150 px, pasted */

/* 0x1A3A0 Tool Acquisition Wrapup: only if a nation in play bought something: record 0 cleared, background 0x19, the
   title, one 51-px row per nation in play (token, name, the advances bought, the cards spent face up), dialog 0xBD940
   (eight invisible row buttons 0xB2..0xB9 and CONTINUE), f3CC14, wait until -1 (unattended: one f78A20 of 250 / 1000).
   Callers: the loop 0x55153 (phase 18) and 0x551F8 (phase 20, PBEM) */
void f1A3A0(void);

#endif
