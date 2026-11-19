#ifndef SHIP_H
#define SHIP_H

/* Module SHIP of docs/M6_PLAN.md (ship.c): the two ship phases, human side, and their shared helpers. SHIP MAINTENANCE
   (phase 3: the loop calls f60AE0 then f60BD8) and SHIP CONSTRUCTION (phase 4: f60AE0 then f61940, "DoShipContruction"
   by its alloc tags); the ship-phase nation order 0x60AE0; the candidate sites 0x62658; a human's ship 0x62774; a
   computer's ship 0x62488; the computer's maintain-or-scrap message 0x6106C (called only by SHIPAI's 0x61254); the
   world-state snapshot 0x7A598 / 0x7A610 around the housekeeping.

   The computers' parts (module SHIPAI, M7) stop the port with not_ported(their address) at their call sites in ship.c:
   0x61254 (a computer's maintenance), 0x62840 (the construction search; also every nation with 4 ships), and the AI
   planner 0x461BC that the "housekeeping" runs after every nation but the last when computers play (AIMOVE).

   Conventions: those of ui.h, game.h and move.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx,
   ebx, ecx, then the stack; void unless a caller reads eax: all call sites checked, D). D = read in the code, G =
   inference. Records (D; move.h has the area record, rules.h the nation record):
     nation n (0xF14CC + 199n): +0x35 int8 Military (G: the advance; the order test only), +0x3E int8 treasury, +0x40
       uint8 ships, +0x45 + 2k int16 ship k's territory + 1, +0x51 int8 cities, +0x60 / +0x61.. its territory list,
       +0x98 int8 tokens on the board, +0x99 int8 stock, +0x9A 0 out / 2 computer / 3 human
     area record of territory t: 0xE31B8 + 69(t + 1): +0x0E + n tokens, +0x1B + n ships, +0x2D + n selected ship
     0xC803C uint8[160] the canonical territory (157..159 are the second coasts of 57, 13, 135)
     territory record t: 0xE1638 + 35t, byte +0: bit 0x02 a coastal territory (G: the meaning; the ship tests use it)
     0xF06AE int16 count, 0xF06B0 int16[] the ship-phase order; 0xF0698 int16 count, 0xF069A int16[] the census order
     0xF7C9C int32 count, 0xF7CA0 int32[] the candidate sites (the shared choice list f78BA4 / f79A9C read) */

#include <stdint.h>

void f60AE0(void);   /* D: the ship-phase order: int16 [0xF06AE] = 0; the census order int16 0xF069A[0..int16 [0xF0698]) with
                        int8 +0x35 == 0 first, then those with +0x35 != 0, into int16 0xF06B0[]; f783B8(0xF596C,
                        [0xF5940]). Callers: the game loop 0x549A6 (phase 3), 0x549D5 (phase 4) */
void f60BD8(void);   /* D: SHIP MAINTENANCE (phase 3). With computers (int8 [0xE1621] > 0) f7A598. Message area (0xA9, 0x154,
                        0x12C, 0x23), background 3 into the back buffer, f7A8D0. Per nation n of 0xF06B0 ([0xF40F8] = n), in
                        play: a human (+0x9A == 3) gets one screen per ship k ("%s, select maintenance options for\nShip #%d
                        in %s", dialog 0xC6E40: MAINTAIN FROM TREASURY 0x34 -> -5, MAINTAIN BY LEVY 0x35 -> -6, DON'T
                        MAINTAIN 0x33 -> -8); a computer (+0x9A == 2) goes to SHIPAI's f61254 (not ported). After each
                        nation but the last, with computers: the housekeeping (f7A940, f7A610, AIMOVE's f461BC (not ported),
                        f7A598, f7AC60; from round 7 a message and a 250 ms floor). End: f7ABD0; the empty message (0xB61A6)
                        if no human saw a ship and no computer had one. BUG(1995) fixed unless g_original: a token paid
                        at a second coast (157..159) is taken off with f581C4 on the alias, see ship.c. Caller: the game
                        loop 0x549AB */
void f6106C(int k, int scrap, int tp1, int useToken, int n);
                     /* D: a computer maintains (scrap == 0) or scraps ship k at territory tp1 - 1, paying 1 token
                        (useToken != 0) or 1 from the treasury: "%s maintains a ship in %s using %s." / "%s doesn't
                        maintain a ship in %s." shown with f79270(-550), the record edits (f57CE8 for the token, f78D98
                        for the scrap; scrapping waits 500 ms first), then the message again: timed (1000 ms, or 500 ms
                        when f7BA64 says unattended and int16 [0xF40BA] == 0) or with CONTINUE when bit 0x40 of [0xF097F]
                        is set (then f3A990, f3CBBC). n is the stack argument (ret 4). One caller: SHIPAI's 0x61254
                        (0x617AD) */
void f61940(void);   /* D: SHIP CONSTRUCTION (phase 4, "DoShipContruction"). Heap blocks [0xEED74] (0x2B65: area records
                        0..160) and [0xEED78] (199: nation n) for RESTART; with computers f7A598; message area as f60BD8;
                        byte [0x108804] = 2; int32 [0x10F2C0] = [0x10F2C4] = 0; background 3; f7A8D0. Per nation n of 0xF06B0
                        with more than 2 tokens on the board (int8 +0x98) or cities and at least one site (f62658): a
                        human with fewer than 4 ships gets the screen 'CANDIDATE SITES FOR SHIP CONSTRUCTION' (dialog 0xC722C: arrows 0x39 -9 / 0x3A -10, FROM
                        TREASURY 0x36 -5, BY LEVY 0x37 -6, BY BOTH 0x38 -7, RESTART 0x41 -99, FINISHED 0x2A -1; a map click
                        on a site returns its index); every other nation goes to SHIPAI's f62840 (not ported), then f62488
                        for each ship of its choice and the extra ships from treasury. Housekeeping as f60BD8 ("Construction
                        housekeeping for %s"). End: [0x108804] = 1, f7ABD0, the blocks freed, the empty message (0xB6311)
                        if nobody built. BUG(1995) fixed unless g_original: the extra ships' sort and the empty-list case,
                        see ship.c. Caller: the game loop 0x549DA */
void f62488(int t, int levy, int treasury, int n);
                     /* D: a computer builds a ship at t for treasury + levy: "%s constructed a ship in %s using %s."
                        (f79270(-550), 500 ms), ship k = +0x40 gets word +0x45 + 2k = t + 1, ships + 1, treasury -
                        treasury, stock + treasury + levy, board - levy, f57CE8(t, n, 0, levy) if levy > 0, area t
                        +0x1B + n + 1 and +0x2D + n = k; then the message again, timed as f6106C or with CONTINUE when bit
                        0x80 of [0xF097F] is set. Callers: 0x6222A, 0x623FB (f61940) */
void f62658(int n);  /* D: the candidate sites of nation n: int32 [0xF7C9C] = 0, 0xF7CA0[0..156] = -1; if int8 +0x98 > 2 or
                        int8 +0x51 > 0: every t < 160 with bit 0x02 of territory byte +0 where n's tokens at 0xC803C[t]
                        are >= 2, or >= 1 with treasury >= 1, appended (t itself, aliases included). Callers: 0x61ACF,
                        0x62034 (f61940), 0x6282D (f62774) */
void f62774(int t, int n, int32_t *sel);
                     /* D: a human's new ship at t: area t +0x1B + n + 1; the first slot k (uint8) with word +0x45 + 2k == 0
                        gets t + 1; area t +0x2D + n = k; ships + 1; f3DA60(t), f459E4(n), f3CBBC, f3D52C(0, 0),
                        f62658(n), f79A9C(sel). sel is the caller's stack int32 (lea, D 0x61E5F..). Callers: 0x61E6E,
                        0x61EF7, 0x61F7F (f61940) */
void f7A598(void);   /* D: the world snapshot: areas 0xE31FD (0x2B20) -> 0x1139B0, territory state 0xE2C18 (0x1E0) ->
                        0x1137D0, nations 0xF1593 (0x88D) -> 0x112CEC. Callers: f60BD8 (0x60BFC, 0x60F6F), f61940 (0x619AA,
                        0x6210E) */
void f7A610(void);   /* D: back from the snapshot of f7A598 (the same three copies the other way). Callers: f60BD8 (0x60F62),
                        f61940 (0x62101), SHIPAI's f61254 (0x61296) and f62840 (0x62953) */

#endif
