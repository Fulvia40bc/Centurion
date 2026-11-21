#ifndef AITRADE_H
#define AITRADE_H

/* Module AITRADE of M7 batch 2 (docs/M6_PLAN.md, module TRADEAI): the computer players' side of the three trade steps
   (aitrade.c). Conventions of game.h / trade.h: fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx,
   ebx, ecx, then the stack; void unless a caller reads eax. D = read in the code, G = inference.

   Called from trade.c (D, the call sites): f68730 at 0x6F32B (DECLARE OFFERS), f698A4 at 0x724E8 (COUNTER OFFERS),
   f6B0EC at 0x73887 (ACCEPT TRADES). The others are reached only from these three; exported for
   port/test/aitrade_test.c. Tables and globals: aitrade.c's file comment. */

#include <stdint.h>

void f68730(int n);            /* D: n declares its offer: f68FDC, f68788, f68B44; with a want count f68C90, else +0x9C = 0 */
void f68788(int n);            /* D: n's ranked list of held commodities 0x1101AC (16-byte entries, count [0x1103EC]), sorted by value */
void f68A64(int n, int unused);   /* D: 0x1105A0[s] = 1 when another nation holds a card of stack >= s (edx stored, never read) */
void f68B44(int n);            /* D: how many cards n asks for ([0x1105AC + 4n], [0x11059C]) and which (+0xA1..+0xA5) */
void f68C90(int n);            /* D: n's declared offer +0x9C..+0xA8 (game_rand once or twice, f6937C, f76F70) */
void f68FDC(int n);            /* D: n's most harmful calamity: [0x110600] its score (<= 0), [0x110604] its id */
void f6937C(int n, int32_t *truth, int count, int k, int flag);   /* D: the stated (bluff) third card(s); game_rand per card; recursive; ret 4 */
void f698A4(int n);            /* D: n's counter offers to the declared offers (rows 0x111118, count int16 [0x112548]) */
void f6A5FC(int n, int k, int32_t *out, int c);   /* D: one counter-offer row for candidate k (game_rand once, f6937C) */
void f6AD70(int n);            /* D: the cards no partner can give cleared from the copied offers 0x110608 */
void f6AF34(int n);            /* D: the same test on n's response list 0x111498, clearing in 0x110608 (kept, file comment) */
void f6B0EC(int n);            /* D: n accepts at most one response to its offer (f75074) */

#endif
