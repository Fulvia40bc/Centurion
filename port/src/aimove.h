#ifndef AIMOVE_H
#define AIMOVE_H

/* Module AIMOVE of docs/M6_PLAN.md, the top level (aimove.c): the computer's movement planner 0x461BC and its five
   passes 0x37CF8 (attacks on enemy territories), 0x387C8 (the attack test of one territory), 0x38E54 (expansion into
   city sites), 0x39068 (an attack on an enemy city) and 0x396FC (relief of besieged own cities). The rest of the
   planner is modules AIQ (aiq.c), AICITY (aicity.c), AISEA (aisea.c) and AIPLAN (aiplan.c); until they are linked,
   aimove.c's calls into them stop the port through not_ported(address) (its file comment lists them).

   Conventions: those of ui.h, game.h, move.h and rules.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments
   eax, edx, ebx, ecx, then the stack (callee-popped); void unless a caller reads eax: every call site checked, D).
   Pointer arguments are the addresses of the caller's stack int32 locals (lea), hence C pointers. D = read in the
   code, G = inference.

   Records and globals (D, offsets as the code adds them; meanings G unless said):
     nation n (0xF14CC + 199n): +0x26 Metalworking, +0x2A Engineering, +0x31 Architecture, +0x3E int8 treasury, +0x40
       uint8 ships, +0x51 int8 cities, +0x52 + k uint8 territory + 1 of city k, +0x5C int8 AST square, +0x5D int8 the
       planner's token reserve, +0x60 int8 / +0x61.. the territory list (territory + 1), +0x98 int8 tokens on the
       board, +0x9A 0 out / 2 computer / 3 human
     area record r = territory r - 1 (0xE31B8 + 69r): +0x01 int8 nations there, +0x01 + m marker, +0x0E + m int8 tokens,
       +0x24 + m int8 movable tokens, +0x37 int16 (cleared by 0x461BC), +0x3B uint8 (0x461BC clears it, 0x39068 writes
       the tokens of n there), +0x3B + m uint8 nation m's target mark (0x38E54 clears it)
     territory state t (0xE2C18 + 3t): +0 owner of the city, +2 bit 0x01 a city, bit 0x02 the AI's site mark
     territory record t (0xE1638 + 35t): +0 bit 0x02 coastal, +1 bit 0x40 a city site, bits 3..5 the population limit
     int32 0xF52E0 + 40n + 4m the score difference matrix of 0x7C960 (the AI's "attitude" of n towards m)
     int16 0x112A94 + 20row + 2(n - 1) the pre-rolled AI table (0x7BCD4, saved), read / 300 as a percentage
     AI level: bits 10..11 of word [0xE1624] (0 Stronger, 1 Normal, 2 Weaker)
     int8 0xF06C0[n] the turn order position of n; uint8 0xEE16C[t] territory t in play
     land move list 0x107694 (4-byte records {from, to, int8 count, type}) / int32 [0x1079B4]; ship move list
       0x1079B8 / int32 [0x107D3C] (movephase.c's file comment)
     int32 [0xF5908] 1 = the computer's moves are shown (MOVEMENT sets it around the call) */

#include <stdint.h>

void f461BC(int n);   /* D: the whole movement plan of computer nation n, played at the end by f4D518. Callers (eax n, none
                         reads eax): MOVEMENT 0x55A5F; SHIP MAINTENANCE's housekeeping 0x60F6A; SHIPAI 0x6132A, 0x61598
                         (0x61254), 0x62989, 0x62C73 (0x62840); SHIP CONSTRUCTION's housekeeping 0x62109; POP 0x682A0
                         (0x680DC) */
void f37CF8(int n, int x);   /* D: attacks on territories held by one other nation: candidates by f387C8, sorted by attitude
                         (and non-sites first), the first one the tokens can be gathered for is planned (World State and
                         Move List levels dropped), the others undone. x = 1 the first pass, 0 the second. Callers: f461BC
                         0x463F5, 0x46686 */
int  f387C8(int t, int n, int mode, int32_t *need, int32_t *victim, int x);
                      /* D: 1 if n should attack territory t (one nation o there, *victim = o, written before the tests),
                         *need the tokens to bring; else 0. mode 1..3 the aggressiveness of f37CF8; need and victim the
                         caller's stack int32 (ecx, [ebp+8]); x the stack argument [ebp+0xC] (ret 8). Caller: f37CF8
                         0x37EFE */
void f38E54(int n);   /* D: with 14+ tokens on the board and f39C08(n) < 9: tokens from n's territories into neighbouring
                         city sites (records type 0x19), each move gated by the AI table rows 11..19. Caller: f461BC
                         0x46611 */
void f39068(int n, int x);   /* D: an attack on an enemy city (via AICITY's 0x3580C) when the score passes the AI table row
                         24; x = the cities planned so far. Callers: f461BC 0x465C7, 0x465FC */
void f396FC(int n);   /* D: for each own city with foreign tokens in it: the tokens to outnumber them gathered (records
                         0x24..0x27), kept only if enough. Caller: f461BC 0x463FD */

#endif
