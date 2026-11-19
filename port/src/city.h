#ifndef CITY_H
#define CITY_H

/* Module CITY of docs/M6_PLAN.md (city.c): CITY CONSTRUCTION 0x32214 (phase 8 of the game loop 0x54824, call at
   0x54B8A) with the human's screen (site arrows, Architecture's treasury arrows, BUILD CITY) and its candidate list
   0x334F4; REMOVE SURPLUS 0x5C238 (phase 9, f5C238(1) at 0x54BDD; the AI simulates it with show = 0) and its token
   removal 0x5C528; CITY SUPPORT 0x5C62C (phases 10 and 17, calls at 0x54C11 and 0x550CC), the support check of one
   nation 0x5C750 and the human's city chooser 0x5C880 (REDUCE CITY). The computers' routines 0x333E8 (a computer
   builds a city) and 0x5DE5C (a computer reduces a city) are module M7's (AI): their call sites stop the port through
   not_ported(address).

   Conventions: those of ui.h, game.h and kit.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax,
   edx, ebx, ecx, then the stack; void unless a caller reads eax). D = read in the code, G = inference.

   Globals (D): int16 [0xF06AE] / 0xF06B0[] the player list (nations in play, ascending); int32 [0xF40F8] the current
   nation; the choice list int32 [0xF7C9C] / 0xF7CA0[157] (the candidate sites) with int32 0xF7F20[] the tokens each
   needs; int16 [0xF408E] the PBEM interaction flag; uint8 0xF5934[12] the surplus tokens per nation (saved); int16
   [0x109E20] 1 = a city support reduction happened (the loop clears it); int16 [0xF7AFE] / [0xF7AFC] the Slave Revolt
   victim and its token loss (set by CAL1's 0x235E4); int16 [0xF40B2] the dead developer switch (cities cost 1 / 2
   tokens instead of 6 / 12, M6_PLAN egg 11; never written, D). Records: rules.h (nation, territory state) and move.h
   (area record); territory record 0xE1638 + 35t, byte +1: bit 0x40 a city site, bits 3..5 the population limit. */

#include <stdint.h>

/* 0x32214 CITY CONSTRUCTION: the player list, f783B8; need_screen (any nation with Architecture +0x31, or no PBEM:
   always 1 without PBEM) and special (area record 114, FAYUM, holds one nation with >= 6 tokens and Agriculture); for
   n 1..9 in play with < 9 cities ([0xF40F8] = n): f334F4(Architecture, n); a human gets the screen (dialog 0xC154C:
   site arrows 0x39 / 0x3A -> -9 / -10, treasury arrows 0x3B / 0x3C -> -11 / -12 with Architecture, BUILD CITY 0x3D ->
   -13, FINISHED 0x2A -> -1, map clicks pick a site) or, only in PBEM without need_screen / special, every candidate at
   once ("%s builds a city in %s"); a computer reorders its candidates when it has more than its free slots, builds
   every site it has the tokens for (0x333E8, M7) and one more with Architecture, then waits until 2 s after its start.
   If no nation had a candidate: f79270(100, first player, -200, "", 0). BUG(1995) fixed unless g_original: the
   computer's reorder reads the tokens of territory t - 1 (city.c) */
void f32214(void);

/* 0x334F4: the candidate sites of nation n into 0xF7CA0[] / 0xF7F20[] (count [0xF7C9C]; entries 0..156 first -1):
   cost 6 / 12 (1 / 2 when int16 [0xF40B2] != 0). For t < 157 without a city: with arch and n alone in the area (area
   +1 == 1): a site with tokens >= 3 and tokens + treasury >= 6 needs 6, a non-site with tokens >= 6 and tokens +
   treasury >= 12 needs 12 (the Architecture rule ignores [0xF40B2]); otherwise a site with tokens >= cost6 needs
   cost6, a non-site with tokens >= cost12 needs cost12. arch = eax, n = edx; eax unused by both call sites */
void f334F4(int arch, int n);

/* 0x5C238 REMOVE SURPLUS (show = 1 from the loop; 0 from the AI's simulations, without any drawing or message):
   show: memset(0xF5934, 0, 12), the player list, f783B8, the map screen, f3C720(0xF596C). For n 1..9 and t 0..156:
   show == 0 and territory state bit 0x02 (an AI-marked site): all n's tokens there removed (f5C528, also k = 0); a
   city: all n's tokens there removed and added to 0xF5934[n]; else the tokens over the population limit (bits 3..5)
   plus 1 with Agriculture (+0x27). show: f3C6A0(0xF596C) and, if nothing was removed, the empty message 0xB5D54 */
void f5C238(int show);

/* 0x5C528: k tokens of n removed from t (area +0x0E+n, f581C4, board - k, stock + k, bytes); show: "%s - %d token%s
   removed from %s" with f79270(1000 ms, or 500 / 1500 unattended (int16 [0xF40BA]), n, t, text, 0). Args eax k, edx
   t, ebx n, ecx show */
void f5C528(int k, int t, int n, int show);

/* 0x5C62C CITY SUPPORT: the player list, f783B8; for n 1..9 ([0xF40F8] = n): "City Support Check for %s." (or "...
   with Slave revolt losses for %s." for the victim int16 [0xF7AFE]) and f5C750(n, the victim's loss int16 [0xF7AFC]
   or 0, text); then, if int16 [0x109E20] is still 0, the empty message 0xB5DC7 */
void f5C62C(void);

/* 0x5C750: nation n (in play) supports its cities: while board (+0x98) - loss < 2 * cities (+0x51): [0x109E20] = 1;
   a human: f7844C, "<msg> You must reduce your cities.", f5C880(&n, 1, text, 1); a computer: board - loss during
   0x5DE5C(n, msg) (M7). BUG(1995) fixed unless g_original: with no city left the loop could never end (city.c).
   Callers: 0x5C6DF (f5C62C), CAL1's Slave Revolt 0x2378F; eax unused */
void f5C750(int n, int loss, const char *msg);

/* 0x5C880 REDUCE CITY: the choice list of the cities of the cnt nations in list (int32, the caller's stack; mode 1:
   only those built this round, int16 [0xF40FC], when there are any, with the prefix "%s First using cities built this
   round."), dialog 0xC5DD8 (arrows 0x39 / 0x3A -> -9 / -10, REDUCE CITY 0x3E -> -14; no FINISHED) on the map screen;
   clicks until REDUCE (f78FEC on the city, one only) or -1; returns the owner of the city shown last. BUG(1995)
   fixed unless g_original: an empty list reads territory -1 (city.c; fixed: REDUCE then only leaves). Callers read
   eax (G for the calamities) */
int  f5C880(const int32_t *list, int cnt, const char *msg, int mode);

#endif
