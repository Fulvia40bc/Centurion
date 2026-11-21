#ifndef AICARDS_H
#define AICARDS_H

/* The computer players' parts of modules CARDS and CITY of docs/M6_PLAN.md (aicards.c), M7 batch 2: trade-card
   acquisition and the buy decision (ACQUIRE GOODS, phase 11), the discards down to 8 (DISCARD EXCESS, phase 19) with
   the ranking of the goods, a computer's city build (CITY CONSTRUCTION) and its city reduction (CITY SUPPORT). The
   human sides are cards.c and city.c. Literal translations; aicards.c's file comment has the address map, the data
   layouts and the 1995 fixes.

   Conventions: those of ui.h and game.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx, ebx,
   ecx, then the stack; void unless a caller reads eax). D = read in the code, G = inference. n is a computer nation
   1..9 (record 0xF14CC + 199n), t a territory 0..156, p a row of the acquire screen, s a card stack 0..8. */

#include <stdint.h>

void f6DBAC(int p, int n);           /* D: "%s acquires..." at (0xF6, 0x62); [0x110AC8] = the cards drawn: for each
                                        stack s < cities, [0x110AC4] times f76E50(s, n), a card back at (0xEC + 39s,
                                        0x6E + 50j), or "Stack %d Empty"; no wait; then the row's counter (card back at
                                        (0x5F, 0x24 + 49p), the count in font 2). Caller: cards.c's f6BD98 (0x6C5E1) */
int  f6DE04(int n);                  /* D: n's purchase from stack s - 1 for 2s: with the variant "purchase from any
                                        stack" (byte [0xE1624] & 0x20) by cities, treasury and pairs in hand (s 1..9,
                                        0 if that stack is empty); without it stack 8 (s 9) when the treasury holds 18,
                                        by stock, [0xF488C + n], two f17EDC(n, -1, 0) calls, the hand size and Gold /
                                        Ivory. Bought: [0xF590C + 4n] = s - 1, treasury - 2s, stock + 2s, returns
                                        (s << 8) + 1; else 0. Caller: f6BD98 (0x6C6B2) */
void f6EDA8(int n);                  /* D: the discard screen of a computer (titles, "%s discards...", dialog 0x110ADC
                                        with FINISHED on and 0x8D off); record 0's cards 1..18 cleared and used as the
                                        discard pile: while n holds more than 8 cards the lowest-ranked commodity
                                        (f6F078) moves to record 0, or swaps with a higher one already there; then each
                                        discarded card to its pile (f771F8) and drawn from x 0x50; wait for -1
                                        (unattended: one f78A20 of 250 / 1000). BUG(1995) fixed unless g_original: no
                                        end with no commodity left (aicards.c). Caller: cards.c's f6E870 (0x6EC61) */
int  f6F078(int n);                  /* D: n's held commodities ranked in the table 0x1101AC (16-byte entries {id,
                                        value, count (+1 when the face <= cities), 0}, count [0x1103EC]) by value
                                        c*c*face ((c+1)^2*face for the Mining goods 0xBDD3C with +0x29), bubble-sorted
                                        descending; returns the last entry's id, int32 [0x11019C + 16 * count] (the
                                        dword before the table when nothing is held). Caller: f6EDA8 (0x6EF29) */
void f333E8(int t, int n, int k, int arch);
                                     /* D: n builds a city in t with k tokens: f78E74(t, n), f57CE8(t, n, 0, k), the
                                        city state's bit 0x02 cleared (and the byte masked to 3), "%s builds a city in
                                        %s." (arch == 0) or "... using Architecture.", f79270(-500 with the AI pause
                                        toggle bit 0x01 of [0xF0980], else 2000, unattended 500 / 1500, n, t, text, 0).
                                        Callers: city.c's f32214 (0x330B8 arch 0, 0x33241 arch 0x63) */
void f5DE5C(int n, const char *msg); /* D: when 2 * cities > tokens on the board, n reduces one city: its cities built
                                        this turn (else the others), scored by n's tokens in their land neighbours,
                                        bubble-sorted by population limit then score (descending), the AI-flag bit
                                        0x80 of 0xF3580 and the territory flag 0x40 first (stable); f78FEC on the
                                        first, "%s reduces %s" through f79270(-500 with bit 0x10 of [0xF0980], else
                                        2500, n, t, text, 0). msg (edx) is not read. Caller: city.c's f5C750
                                        (0x5C862) */

#endif
