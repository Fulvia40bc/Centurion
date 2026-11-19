#ifndef MONO_H
#define MONO_H

/* Module MONO of docs/M6_PLAN.md (mono.c): the MONOTHEISM phase 0x21914 (phase 16 of the game loop 0x54824, call at
   0x55098), the human's conversion screen 0x21A70 and the candidate list 0x227B0. The computers' conversion 0x21E8C
   is module M7's (AI): its call site stops the port through not_ported(0x21E8C).

   Conventions: those of ui.h, game.h and kit.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax,
   edx, ebx, ecx, then the stack; void unless a caller reads eax). D = read in the code, G = inference.

   Globals (D): int16 0xBB3FA / 0xBB3FC / 0xBB3FE / 0xBB400 the message area (x, y, w, h); int16 [0xF06AE] / 0xF06B0[]
   the player list (nations in play, ascending); int32 [0xF40F8] the current nation; the choice list int32 [0xF7C9C] /
   0xF7CA0[157] (the candidate territories); the land lists [0xF8E7C + 4t] (port handles into landadj.dat's heap
   block, game.h). Records: nation 0xF14CC + 199n (+0x3A Monotheism, +0x3B Theology, +0x51 int8 cities, +0x98 int8
   tokens on the board, +0x99 int8 stock, +0x9A 0 out / 2 computer / 3 human); area record 0xE31B8 + 69r (territory
   r - 1; move.h, conflict.h: +0x0E + n tokens of n, +0x3B + n the "lost territory" mark); territory state 0xE2C18 +
   3t (rules.h). */

#include <stdint.h>

/* 0x21914 MONOTHEISM: message area (0xA9, 0x154, 0x12C, 0x23); the player list rebuilt, f783B8; for n 1..9 in play
   holding Monotheism (+0x3A): f227B0(n); with candidates: f553C0 (the phase music), then a human (+0x9A == 3, no
   PBEM) f21A70(n), anyone else 0x21E8C(n) (M7). If no nation had a candidate: f79270(100, first player, -200, "", 0) */
void f21914(void);

/* 0x21A70: human nation n converts one territory of the list (dialog 0xBE9F4: arrows 0x39 / 0x3A -> -9 / -10,
   CONVERT TERRITORY 0x58 -> -16, FINISHED 0x2A -> -1; map clicks on listed territories select). CONVERT on a city:
   area +0x3B + owner = 1, f78F04(t, owner), f78E74(t, n); else every nation k 1..9 with tokens x there: area +0x3B +
   k = 1, f57CE8(t, k, 0, x), k's board - x and stock + x, f57E28(t, n, x), n's stock - x and board + x (bytes); then
   f459E4(n), f3D52C(0, 0), f3CBBC, f78A20(1000) and the screen ends (one conversion). One caller (0x21A37) */
void f21A70(int n);

/* 0x227B0: the candidate list of nation n into 0xF7CA0[] / [0xF7C9C] (entries 0..156 first -1): the territories
   land-adjacent ([0xF8E7C + 4t]) to one where n has a city or tokens, not themselves such, in ascending order: a city
   whose owner is not 10 (the Pirates) and holds neither Monotheism nor Theology, when n's cities (int8 +0x51) != 9;
   else tokens of nations 1..9 other than n totalling 1..n's stock (int8 +0x99), none of those nations holding
   Monotheism or Theology (a city territory only by the city rule, unless g_original: BUG(1995) in mono.c). One caller
   (0x219FD) */
void f227B0(int n);

/* Text for the translation of 0x21E8C (M7): its two sprintf formats 0xB1EEA (0x221AC, the city branch) and 0xB1F07
   (0x226A0, the token branch), "%s converts %s by Montheism!". Returns the data string under g_original, else the
   corrected "%s converts %s by Monotheism!" (Erik's ruling of 19 Nov: misspellings). addr is 0xB1EEA or 0xB1F07 */
const char *mono_converts_fmt(uint32_t addr);

#endif
