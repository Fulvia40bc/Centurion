#ifndef AIADV_H
#define AIADV_H

/* Module ADVAI of docs/M6_PLAN.md (aiadv.c): the computer players' side of ACQUIRE CIVILIZATION (phase 18), M7 batch
   2. The human side, the computer's turn (adv.c's computer_turn, part of 0x162D0) and the small formulas 0x19448,
   0x1B4D4, 0x1B538, 0x1C2DC (formulas.c) are ported already. Literal translations; aiadv.c's file comment has the
   address map, the data layouts and the 1995 fixes.

   Conventions: those of ui.h and game.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx, ebx,
   ecx, then the stack; void unless a caller reads eax; out-pointers are the callers' int32 stack locals, passed with
   lea). D = read in the code, G = inference. n is a computer nation 1..9 (record 0xF14CC + 199n), i an advance 0..23.

   Results the callers read (D):
     int32 0xF62EC[24]   the desirability of each advance for n (f1A888; 0 for the owned and the unaffordable)
     int32 [0xF7C9C] / 0xF7CA0[]  the candidate list f177EC leaves (the shared list of the AI, also AIQ's)
     nation record 0 (0xF14CC)    the cards f1B59C pays with (+0x01..+0x12), copied from n first
     int32 [0xF6748] / [0xF674C] / [0xF6750]  min(token shortfall, treasury) / treasury / the treasury f1B59C pays
     int32 [0xF68B0]              the Mining choice (an index of 0xF68B8) that goes with record 0's cards
     0xF68CC / 0xF6B0C, [0xF6D4C] / [0xF6D50]  f1BE08's 32-byte list of the held commodities and its best copy */

#include <stdint.h>

void f1A888(int n);                  /* D: the desirability 0xF62EC[24] of the advances in state 2 (0xF67C4) that n
                                        does not hold: a base value per advance, adjusted by n, its advances, the
                                        others' advances, the attitude row 0xF52E0 + 40n, the leading nation
                                        [0xF52D4], credits 0xF6764. No call but memset and f1C2DC. Callers: adv.c's
                                        computer_turn (0x16E57, 0x16F1F, 0x17014) */
int  f177EC(int n);                  /* D: the advance n buys next, or -1: the state-2 advances (0xF7CA0, count
                                        [0xF7C9C]) cut by the AI table's group and points goals (0xBCCE2 / 0xBCCE4 +
                                        85(n-1) + 5 * the AST square), bubble-sorted by 0xF62EC; the first when a
                                        human leader with Military threatens (0xF52E0 > 500), else one of the first
                                        four by weight (one game_rand); kept only if n has more than 8 cards, a full
                                        set, a goal was cut back, f1B45C (a second game_rand when the score beats the
                                        cost) or a stock shortfall. Callers: computer_turn (0x16E5F, 0x16F27,
                                        0x1701C) */
int  f1B45C(int i);                  /* D: buy anyway: if cost(i) < 0xF62EC[i], 1 when game_rand * 100 >> 15 < the
                                        difference (one draw), else 0 (no draw). Caller f177EC (0x17EB2) */
int  f1B59C(int n, int need);        /* D: the cards n pays need points with: record n copied to record 0, then an
                                        18-digit odometer over record 0's commodity counts (each from the held count
                                        down to 1, commodity 1 fastest; the tolerance 3 grows by 3 the first time
                                        commodities 3, 5, ..., 17 move), each hand scored by f1BE08; an exact fit, or
                                        the smallest excess once below the tolerance, is written to record 0 (+
                                        [0xF68B0]) with [0xF6750] = max(0, need - f19448()) and 1 returned; 0 when
                                        the odometer runs out. Caller: computer_turn (0x1718B) */
int  f1BE08(int n, int need, int32_t *excess, int32_t *mask, int32_t *exact, int first);
                                     /* D: record 0's hand for need points: the list 0xF68CC of held commodities
                                        (value c*c*face, with Mining (+0x29) one eligible entry counted as c+1, tried
                                        in turn), the prefix that reaches need with the treasury [0xF674C]; an exact
                                        fit (within [0xF6748]) returns 1 with its entry mask in *exact; else the
                                        smallest excess and its mask in *excess / *mask (written only when the hand
                                        reaches need) and 0. n and first (the second stack argument; ret 8) are not
                                        read. Caller f1B59C (0x1B79A) */
void f19700(int n, int count, uint32_t list);
                                     /* D: "%s has acquired \n" + the names of the count advances in the int16 list
                                        at link address list (", \n" / " and \n" between, "." at the end) in a box
                                        over the screen (saved to "junk"), the current dialog's other buttons off
                                        meanwhile, CONTINUE of 0xBBBEC; wait for -1 (unattended: one f78A20 of 250 /
                                        1000). BUG(1995) text slip fixed unless g_original (aiadv.c). Caller:
                                        computer_turn (0x175DA) */

#endif
