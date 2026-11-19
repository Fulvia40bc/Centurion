#ifndef POP_H
#define POP_H

/* Module POP of docs/M6_PLAN.md (pop.c): the POPULATION EXPANSION phase 0x673A8 (phase 1 of the game loop, called at
   0x54952) and its human screen ManualPopExpansion 0x67758. The computer's choice 0x680DC (module AI, M7) and the
   choice-list builder 0x28CFC (CAL1) are not ported yet: their call sites stop with not_ported(address).

   Conventions: those of ui.h and game.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx, ebx,
   ecx, then the stack; void unless a caller reads eax). D = read in the code, G = inference. Layouts and quirks:
   pop.c's file comment. */

void f673A8(void);   /* D: POPULATION EXPANSION: message area (0xA9, 0x154, 0x12C, 0x23), the player list int16 [0xF06AE] /
                        0xF06B0[] of the nations in play, the map screen; for n 1..9 in play ([0xF40F8] = n, byte
                        [0xF488C + n] = 0): need = sum over n's areas (+0x60, +0x61..) of 2 (tokens >= 2) or 1 (one
                        token); stock (+0x99) >= need: tokens 1 -> 2, 2 -> 4 (marker written), > 2 -> + 2, board +0x98
                        and stock moved; else f7844C and "%s, you have no stock for expansion." (stock <= 0, f79270),
                        f67758(n) (human, +0x9A == 3) or 0x680DC([0xF40F8], need) (computer, not ported). Then a wait to
                        1 s after the start. One caller (0x54952); eax unused */
void f67758(int n);  /* D: ManualPopExpansion of human nation n short of stock: snapshots of areas / territory states /
                        nations ([0xEED74] / [0xEED7C] / [0xEED78], port handles), allowance area +0x24+n (1 for one token,
                        2 for more), dialog 0xC8954 (arrows, DISTRIBUTE ALL 0xDB, ADD ONE 0xDC, ADD MAX 0xDD, RESTART 0x41,
                        FINISHED 0x2A) on 0xF596C, the list 0x28CFC(n, 3, 0) (not ported), click loop on f78BA4(0xF596C,
                        1) until FINISHED (-1). BUG(1995) fixed unless g_original: DISTRIBUTE ALL stops when a pass adds
                        nothing (pop.c). One caller (0x6764D); eax unused */

#endif
