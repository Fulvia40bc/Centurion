#ifndef CONFLICT_H
#define CONFLICT_H

/* Module CONFLICT of docs/M6_PLAN.md (conflict.c): the CONFLICT phase 0x5AAA4 (phase 7 of the game loop 0x54824, call
   f5AAA4(1) at 0x54B56) with the shared rules it and the calamities use: the city attack 0x5B224, the token battle
   0x5B588, the plunder 0x5BA0C, the conflict matrix 0x5C110, the pre-movement owner 0x5C1CC and the losses box
   0x5CDE8. The computer's plunder transfer 0x5BF30 (M7, AI) and the calamity animation 0x7A4C4 (module ANIM) are not
   ported yet: their call sites stop with not_ported(address).

   Conventions: those of ui.h, game.h, move.h and rules.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments
   eax, edx, ebx, ecx, then the stack; void unless a caller reads eax; records as uint32_t link addresses). D = read in
   the code, G = inference. Records and globals (D, offsets as the code adds them):
     area record r (= territory r + 1): 0xE31B8 + 69r; +0x00 uint8 r itself, +0x01 int8 nations there, +0x01 + n
       marker (n when nation n has tokens there), +0x0E + n int8 tokens of n (n = 1..11; +0x18 the Pirates), +0x3B + n
       int8 a "lost territory" mark of nation n (G: the meaning; written only by 0x5AAA4 here)
     pre-movement copies taken by the loop at 0x549FF: areas 0xE8888 (161 x 69), territory states 0xE2FD8 (160 x 3)
     territory state t: 0xE2C18 + 3t: +0 owner of the city (or, bit 0x02, of the site the AI marks), +1 turn byte,
       +2 bit 0x01 a city stands there, bit 0x02 the AI's site mark (written by 0x5B224)
     territory record t: 0xE1638 + 35t, byte +1 bits 3..5 the population limit
     nation n: 0xF14CC + 199n: +0x01..+0x12 commodity cards, +0x13 int8 calamities held, +0x14.. their ids, +0x26
       Metalworking, +0x2A Engineering, +0x31 Architecture, +0x35 Military (PBEM rule only), +0x3E int8 treasury, +0x98
       int8 tokens on the board, +0x99 int8 stock, +0x9A 0 out / 2 computer / 3 human, +0xA9 + j int8 hostility of n
       towards nation j + 1 (G: the meaning; 0..5)
     0x109E24 uint8 [9][9] the conflict matrix (row i: nation i + 1 met nation j + 1; in the save)
     0xF547C / 0xF548C uint8 [14] tokens / cities lost in the current conflict (the losses box); 0xF5486 = Pirates' tokens
     plunder pending: uint8 0x109E08 flag, 0x109E09 attacker, 0x109E0A city owner, int16 0x109E0C area record r
     uint8 [0xBB3F8] 1 = a real resolution (lost tokens go back to stock), 0 = an AI trial (G, writers 0x39D79 / 0x4975E)
     0x112584 uint8 [32] / int32 [0x1125A4] the owner's card list of 0x5BA0C */

#include <stdint.h>

void f5AAA4(int show);   /* D: CONFLICT. show = 1 from the game loop (0x54B56), 0 from the AI's trials (M7; then no screen,
                            no player list, no matrix clear, no hostility update, no plunder transfer for a human).
                            Lists the contested area records 1..157 (two or more nations over the population limit, or a
                            city whose owner has no tokens there with tokens of others over the limit); two passes, areas
                            without a city first, then cities: per entry the matrix row (f5C110), the highlight, the
                            city attack f5B224 or the battle f5B588; with show the "lost territory" marks, the losses box
                            f5CDE8 and the plunder f5BA0C. After both passes (show) the hostility update; none contested
                            (show): the empty message, 100 ms home view of the first player. Always f3C314(0xF596C, 7) */
void f5B224(uint32_t rec);   /* D: attack on the city of area record rec (address 0xE31B8 + 69r): battle first if two
                            or more nations are there; the first nation 1..11 present is the attacker (the owner: no
                            attack; none, i.e. only the factions 12 / 13: a guard, no attack, see conflict.c); needs 8 / 6 / 7 tokens (owner with / attacker with / else Engineering), the city
                            replaced by 7 / 5 / 6 of the owner's stock (all for the Pirates), then a battle again; a
                            computer winner with 6+ tokens (3+ with Architecture) marks the site (state +2 |= 2); a
                            winner with tokens left (not the Barbarians) sets the plunder pending. Callers: 0x5AEDB, the
                            barbarians 0x1D102, 0x1D5DD, 0x1D79C, 0x1DD18 */
void f5B588(int r);      /* D: token battle in area record r (territory r - 1): nations 1..11 with tokens ranked by
                            tokens (+50 with Metalworking), the smallest group first, one token each per turn round the
                            groups until one nation is left (a city) or the tokens fit the population limit; with byte
                            [0xBB3F8] the tokens go back to stock; then every Pirate token there is removed. 10 callers */
void f5BA0C(int show);   /* D: plunder of the pending city (0x109E09 attacker a, 0x109E0A owner o, int16 0x109E0C r).
                            show and o <= 9 with commodity cards: one random card of o (game_rand, the calamities in the
                            draw too) moves to a, "%s gets a random trade good from %s as plunder." (2500 ms). Then a
                            human a (show only) moves up to min(stock, 3) tokens from stock to treasury (arrows -12 / -11,
                            FINISHED -1); a computer goes to 0x5BF30 (M7, not ported); no stock: the message (show). One
                            caller (0x5B0E9) */
void f5C110(uint32_t rec);   /* D: conflict matrix row: for i 0..8, nation i + 1 present in rec (or owning the city
                            there, see the BUG(1995) in conflict.c): 0x109E24[9i + j] = 1 for each other nation j + 1
                            present. One caller (0x5AE8C) */
int  f5C1CC(int t);      /* D: the first nation n 1..9 with tokens in territory t before the movement (marker n in the
                            copy 0xE8888), -1 if none. One caller (0x5AF47) */
int  f5CDE8(int anim, const char *title);
                         /* D: the losses box: title in the message area, per nation 1..13 in play with losses (0xF547C /
                            0xF548C) a column with its name (the short name when too wide), token and city sprites and
                            the counts; anim != 0: the calamity animation 0x7A4C4 (module ANIM, not ported). Waits for
                            FINISHED (-1, dialog 0xC5214 id 0x2A), or 250 / 1000 ms when unattended; the faction buttons
                            -0x50 / -0x51 (dialog 0xC0214 of 0x2D3E0) return 12 / 13, FINISHED 0. With no nation to show
                            (the original divides by 0) no column is drawn. title is a data string at every call site.
                            Callers: 0x5B0C2 (eax 0), calamities 0x1D671, 0x1D82B, 0x1F4FF, 0x2D4CE */

#endif
