#ifndef TAX_H
#define TAX_H

/* Module TAX of docs/M6_PLAN.md (tax.c): the TAXATION phase 0x66050 (phase 0 of the game loop 0x54824, call at
   0x5492A) and its human paths: the Coinage rate screen 0x66488, the tax revolt 0x6685C and the human's own city
   removal 0x66D8C (ChooseMultipleCitiesToRemove). The computers' routines 0x667B0 (tax rate), 0x66B54 (annexing)
   and 0x66C2C (city removal) are module M7's (AI), and CAL1's 0x2468C (ChooseCitiesToLoseToPirates, mode 2) and
   0x28CFC (the choice list) are not ported yet: their call sites stop the port through not_ported(address).

   Conventions: those of ui.h, game.h and kit.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax,
   edx, ebx, ecx, then the stack; void unless a caller reads eax). D = read in the code, G = inference.

   Globals (D): int32 0x11017C[10] per nation 1 = it could not pay its tax this round (written by 0x66050, read by
   0x6685C); int32 [0xFFD40] the help topic (-50 the rate screen, 2 the tax revolt: menu1.h's f42CEC); int32
   [0xF40F8] the current nation; int16 [0xF06AE] / 0xF06B0[] the player list (nations in play, ascending); nation
   record 0xF14CC + 199n: +0x2C Coinage, +0x34 Democracy, +0x3E int8 treasury, +0x3F int8 tax rate, +0x51 int8
   cities, +0x52 + j city j's territory + 1, +0x99 int8 tokens in stock, +0x9A 0 out / 2 computer / 3 human. */

/* 0x66050 TAXATION: the message area (0xA9, 0x154, 0x12C, 0x23), 0x11017C[n] = 0 and the player list rebuilt
   (nations 1..9 with +0x9A != 0), f783B8. Nobody with cities: f79270(100, first player, -200, "", 0). Else the map
   screen, then for n 1..9 in play with cities ([0xF40F8] = n): Coinage: the human's f66488(n), the computer's
   f667B0(n) (M7; "%s sets its Tax Rate to %d." 2000 ms when it changed); tax = cities * rate; stock >= tax: treasury
   += tax, stock -= tax (bytes); else 0x11017C[n] = 1. Then each revolt: Democracy "Democracy spares %s from a Tax
   Revolt." 3000 ms (the original's "Democrary" with --original); else f7844C and f6685C(n, cities - stock / rate).
   Last a wait until 500 ms after the start of the nations' loop (see tax.c) */
void f66050(void);

/* 0x66488: human nation n sets its tax rate (Coinage): [0xFFD40] = -50, [0xF40F8] = n, f7844C; PBEM and n not the
   local player: nothing more. max = min(stock / cities, 3); max == 0: the forced message, rate 1, 4000 ms. Else
   dialog 0xC817C with "TAX RATE", v = min(rate, max), clicks: -12 up to max, -11 down to 1, -1 rate = v */
void f66488(int n);

/* 0x6685C: tax revolt of nation n losing k cities: [0xFFD40] = 2; the other nations in play without a revolt sorted
   by 5 * (9 - cities) + stock (int16, descending, bubble sort: ties keep the nation order); "%s suffers a Tax
   Revolt!"; each takes min(9 - its cities, k) while k > 0 (human: 0x2468C(m, n, take, 2), computer 0x66B54(m, n,
   take)); cities left over: the message, then the human's f66D8C(n, k) or the computer's 0x66C2C(n, k); last
   treasury += cities * rate, stock -= the same (bytes) */
void f6685C(int n, int k);

/* 0x66D8C ChooseMultipleCitiesToRemove: human nation n removes k of its own cities on the map screen (dialog
   0xC8568: arrows -9 / -10, RESTART -0x63, REMOVE -0x65, FINISHED -1; map clicks select). The area records, the
   territory states and nations 1..11 are kept in heap blocks [0xEED74] / [0xEED7C] / [0xEED78] (port handles) for
   RESTART and freed at the end */
void f66D8C(int n, int k);

#endif
