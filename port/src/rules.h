#ifndef RULES_H
#define RULES_H

/* Module RULES of docs/M6_PLAN.md (rules.c): the shared rule helpers of the game loop's phases, pure record edits and
   queries on the territory, territory-state, area, nation and card-stack records.

   Conventions: those of ui.h, game.h and move.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx,
   ebx, ecx, then the stack; void unless a caller reads eax; records as uint32_t link addresses). D = read in the code,
   G = inference. Records (D, offsets as the code adds them; meanings G unless said; move.h has the area record and
   the ship and territory-list fields of the nation record):
     territory record t: 0xE1638 + 35t (game.h)
       +0x01        uint8  bits 3..5 (shl al, 2; shr al, 5): the population limit
     territory state t: 0xE2C18 + 3t (game.h)
       +0x00        uint8  the nation whose city stands there (0 none)
       +0x01        uint8  the turn byte [0xF40FC] when it was built (0xFF none)
       +0x02        uint8  bit 0: a city stands there
     nation record n: 0xF14CC + 199n
       +c           uint8  commodity cards of id c (c = 1..18: +0x01..+0x12; f76E50 inc [199n + c + 0xF14CC])
       +0x13        int8   calamity cards held
       +0x14 + j    uint8  calamity id j (0x13..0x1E), j < +0x13 (0 when empty)
       +0x1C + j    uint8  the nation that gave calamity j (0xFF when empty or drawn from a stack)
       +0x25        uint8  Clothmaking (ships move 5)        +0x27  int8  advance 3 (G: Agriculture)
       +0x3D        uint8  the nation's own number n
       +0x51        int8   cities;  +0x52 + j  uint8  territory + 1 of city j
       +0x98        uint8  tokens on the board;  +0x99  int8  tokens in stock
     card stacks: 0xF06D0 + 25s (game.h: int16 count, the cards from +6; the top card is the last)
     discard piles: 0xF07B4 + 25s, the same shape (int16 count, cards from +6)
     0xCA9C1 + c  int8  the pile of calamity c (0x13..0x1E: 1, 1, 2, 2, 3, 3, 4, 4, 5, 6, 7, 8; D, the bytes after
                        "Select Nation To Show") */

#include <stdint.h>

/* territories and cities */
int  f78D08(int t, int n);           /* D: population limit of t for n: bits 3..5 of territory +1, plus 1 when n holds
                                        +0x27 (!= 0) and n is alone in t (area +1 == 1 and marker +1+n == n). 39 call
                                        sites read eax */
void f78E74(int t, int n);           /* D: city of n built in t: state +2 |= 1, +0 = n, +1 = [0xF40FC]; t + 1 appended
                                        to n's city list (+0x52 + count), count +0x51 + 1 */
void f78F04(int t, int n);           /* D: city of n in t removed: state +2 &= ~1, +0 = 0, +1 = 0xFF; t + 1 taken out of
                                        n's city list (shift down, count - 1, the old last slot 0). BUG(1995) in rules.c */
void f78FEC(int t, int n);           /* D: city of n in t reduced to tokens: f78F04(t, n); limit = bits 3..5 of
                                        territory +1, plus the byte +0x27 when area t+1 is empty or has one nation and
                                        n has tokens there; min(stock +0x99, limit) tokens placed one by one
                                        (f57E28(t, n, 1)); stock - k, board +0x98 + k */
int  f7C760(int t);                  /* D: the first nation m (1..11) whose marker +1+m == m in t, 0 if none */
int  f7C7C4(int t, int n);           /* D: as f7C760, skipping m == n */
int  f789EC(int n);                  /* D: 0 for n == 4, 5, 8 (Thracian, Cretan, Babylonian), else 1 (10 Pirate and
                                        11 Barbarian too). 0x21021 maps 1 to "An", 0 to "A"; 0x2110B maps them the
                                        other way round, 1 to "a", 0 to "an" (CAL1's text, see rules.c) */

/* ships and movement */
void f78D98(int k, int n);           /* D: ship k of n removed: its words +0x45.. shifted down from k (to index 2), ship
                                        count +0x40 - 1, ships +0x1B+n of the area record word(k) - 1, word 3 = 0; then
                                        for every ship left [0xF40F8] = n and f576F8(word - 1). The bytes +0x41.. and
                                        +0x4D.. are not shifted (see rules.c) */
void f57CA4(int a, int b, int n, int flag, int k);   /* D: f57CE8(a, n, flag, k) then f57E28(b, n, k); k on the
                                        stack (ret 4) */
void f460C0(int n);                  /* D: for area records 1..160: +0x24+n = +0x0E+n when the marker +1+n != 0, else 0 */
void f46158(int n);                  /* D: every ship k < +0x40 of n: +0x41+k = 4, or 5 with +0x25 != 0 */

/* cards */
int  f76E50(int s, int n);           /* D: the top card of stack s to n: count != 0: card = the byte at +5 + count (the
                                        last card, cards[count - 1] with the cards from +6), count - 1;
                                        a calamity (> 18) appended at +0x14 (count +0x13 + 1; the giver byte not
                                        written), a commodity +c + 1. Returns the card (int8), 0 for an empty stack */
void f771F8(int c);                  /* D: card c onto its discard pile: (c - 1) / 2 for c <= 18, else int8 [0xCA9C1+c];
                                        appended at +6 + count, int16 count + 1 */
void f6E460(int n);                  /* D: n's calamities sorted ascending (int8 compare), the givers along (bubble) */
void f75CA8(int c, uint32_t rec);    /* D: card c (int8; 0 ignored) taken from the nation record at rec: c <= 18:
                                        +c - 1; a calamity: found in +0x14.., both lists shifted down, count - 1, the
                                        old last slots 0 / 0xFF. BUG(1995) in rules.c */
void f75D84(int c, uint32_t rec, uint32_t giver);    /* D: card c (int8; 0 ignored) to the record at rec: c <= 18:
                                        +c + 1; a calamity appended at +0x14 + count with the byte +0x3D of the record
                                        at giver at +0x1C + count, count + 1 */

#endif
