/* The computer nations of AC_MAIN.EXE (module N3 of docs/MAP_PLAN.md): 0x302C4 (give the nations the humans left to
   the computer, by player count) and 0x30174 (give one nation to the computer and show it). Literal translations; the
   address of each instruction a line comes from is in its comment. D = read in the code, G = inference. Prototypes:
   game.h (section "Nation choice, human players", which also has the layouts used here).

   Caller: the nation choice 0x2EFA0 (choose.c) at 0x2F9DE, when [0xE1621] (computer players) > 0, between
   f3C940(0xC08D0) and f3C7A0(0xC08D0). 0x302C4 is its only caller of 0x30174 (D: every call 0x30174 lies in
   0x302C4).

   Watcom runtime: none. __CHK dropped. game_rand is f78AE4 (game.h), called in the original order and number: every
   call is unconditional on its path, and the paths depend only on [0xF40F0] and which nations are human (byte +0x9A ==
   3), never on the count rem, so the BUG(1995) fixes below do not change the random sequence (D).

   Data (D):
     [0xF40F0]                int32 players (humans + computers, START GAME 0x4E281)
     0xF1566 + 0xC7n          nation n's byte +0x9A (record 0xF14CC + 199n): 0 free, 2 computer (0x30191), 3 human
                              (0x2F7B6). Free is tested as a byte (cmp byte [..], 0); human as dword [.. - 3] sar 24
                              == 3. n: 1 AFRICA 0xF162D, 2 IBERIA 0xF16F4, 3 ILLYRIA 0xF17BB, 4 THRACE 0xF1882, 5 CRETE
                              0xF1949, 6 ASIA 0xF1A10, 7 ASSYRIA 0xF1AD7, 8 BABYLON 0xF1B9E, 9 EGYPT 0xF1C65 (names
                              0xC768C)
     0xC076C, 0xC0780         int16[10] the x, y of nation n's 35x35 token; sprite 0xF4A5C + 8n
   Locals of 0x302C4: [ebp-0x1C] rem, [ebp-0x20] the counting loop's n, [ebp-0x18] the mask (all int32).

   What 0x302C4 does (D): one block per player count, each starting with rem = [0xF40F0] - (nations 1..9 that are
   human) = the computer players asked for. Then a fixed order of nations, each normally taken only if rem > 0 and it
   is free; a pair "X|Y" is one game_rand*2 >> 15 draw (0 X, 1 Y), X taken only if Y is not human and vice versa; a
   triple (A,B,C) is a switch through a jump table on mask = 4*(A human) + 2*(B human) + (C human):
     mask 0: game_rand*3 >> 15 picks a pair (r 0, 1, 2), the first taken unguarded, the second if rem > 0;
     mask 1 (C human): a rand*2 draw, A or B, unguarded; mask 2 or 4 (one of A, B human): the remaining one of the pair
     (C, or for 6,7,8 the nation 8), unguarded; masks 3, 5, 6, 7 (two or three human): none.
   Tables (read from obj1.bin with the fixups, D; entries 0..7):
     0x30224 (4 players, 3,4,5): 30521 304F7 305BD 30412 305BD 30412 30412 30412
     0x30244 (5 players, 3,4,5): 306EF 306C5 3072C 30412 3072C 30412 30412 30412
     0x30264 (7 players, 6,7,8): 30A0D 30940 30A76 30965 30A76 30965 30965 30965
     0x30284 (7 players, 3,4,5): 30AD7 30A94 30B65 30AB9 30B65 30AB9 30AB9 30AB9
     0x302A4 (8 players, 6,7,8): 30CE4 30BFB 30D14 30C20 30D14 30C20 30C20 30C20
   The orders:
     3: Africa, Illyria|Iberia, Thrace|Crete
     4: Africa, Iberia, triple (3,4,5) with pairs r0 {3,4}, r1 {5,3}, r2 {4,5}
     5: Africa, Iberia, Asia, triple (3,4,5) as 4 (its r0 and r1 jump into the 4-player code, 0x30545 / 0x30574)
     6: Africa, Iberia, Crete, Egypt, Asia|Assyria, Illyria|Thrace
     7: triple (6,7,8) with pairs r0 {7,8}, r1 {8,6}, r2 {7,8}; Africa, Egypt, Iberia; triple (3,4,5) as 4
     8: triple (6,7,8) with pairs r0 {7,8}, r1 {6,8}, r2 {7,8}, the second unguarded too; Africa, Egypt, Iberia,
        Illyria, Thrace, Crete
   The blocks for 3, 4 and 5 are one if / else chain on edx = [0xF40F0]; 6, 7 and 8 each re-read [0xF40F0] (0x30412,
   0x308B1, 0x30AB9).

   1995 bugs (fixed by default, g_original keeps them):
     BUG(1995) A: after taking a nation the code does rem-- (dec dword [ebp-0x1C]) everywhere except at nine places,
       where it does rem++ (inc): Africa in all six blocks (0x30339, 0x3048A, 0x30639, 0x30793, 0x30981, 0x30C3C),
       Iberia with 4 and 5 players (0x304A9, 0x30658) and Asia with 5 (0x30677). So those nations do not use up the
       count, and when the humans' choices leave fewer free slots than the fixed order has, the computer takes one or
       two nations more than the setup asked for: more nations in play than [0xF40F0] players. The fix: rem-- there
       too.
     BUG(1995) B (7 players only): the switch on (3,4,5) runs after Africa, Egypt and Iberia, but its unguarded cases
       (mask 1 at 0x30AAE, mask 2/4 at 0x30B65) take a nation even with rem <= 0. This happens when the humans hold all
       three of Asia, Assyria and Babylon (a triple with two computer slots) and exactly one of Illyria, Thrace, Crete:
       one computer nation too many even with fix A. The fix: those two takes also need rem > 0 (the game_rand call of
       mask 1 still happens, so the random sequence is the same). In the other switches the unguarded takes cannot meet
       rem <= 0 once fix A is in (checked by enumerating every setup and human choice).
     With both fixes the computer gets exactly [0xE1621] nations in every setup the setup screen allows (3..8 players,
     0..players-1 humans, every choice of the humans among the candidates of 0x2EFA0); in the original, with the
     1995 code, it gets one or two more for some choices (never fewer). With one human (the setup most played) the
     count is always right.
   Noted, not changed (G: by design): in the triples (6,7,8) of 7 and 8 players, r 0 and r 2 lead to the same pair
   (0x30A8B -> 0x30A5C, 0x30D4E -> 0x30D29), so with no human among them Babylon is always taken, with Assyria 2/3 of
   the time and Asia 1/3; the triples (3,4,5) have three distinct pairs. */

#include <stdint.h>
#include "game.h"
#include "fg.h"

/* game_rand*2 >> 15: 0 or 1 (add eax,eax; shr eax,0xF; unsigned) */
static uint32_t rand2(void) { return (uint32_t)f78AE4() * 2u >> 15; }
/* game_rand*3 >> 15: 0, 1 or 2 (mov edx,eax; shl eax,2; sub eax,edx; shr eax,0xF) */
static uint32_t rand3(void) { return (uint32_t)f78AE4() * 3u >> 15; }

/* rem after a take at one of the nine rem++ places (BUG(1995) A, see above): the original increments */
static int32_t rem_after_bug_a(int32_t rem)
{
    if (g_original)
        return rem + 1;     /* BUG(1995): A, inc dword [ebp-0x1C] */
    return rem - 1;
}

/* ---- 0x30174: one nation to the computer ------------------------------------------------------------------------ */

/* 0x30174: nation n becomes a computer nation (+0x9A = 2) and is shown: with the mouse hidden, its token on the active
   surface, the sunken name box with its name in colour 5, then a 750 ms wait. */
void f30174(int n)
{
    int32_t nn = n;                                             /* [ebp-0x10] */

    setD8((uint32_t)(nn * 0xC7 + 0xF1566), 2);                  /* 0x3018A imul 0xC7; 0x30191 byte [eax+0xF1566] = 2 */
    f78884();                                                   /* 0x30198 */
    f3AF8C((uint32_t)(nn * 8 + 0xF4A5C),                        /* 0x301BA shl 3; add 0xF4A5C; 0x301C7 */
           S16((uint32_t)(0xC076C + 2 * nn)),                   /* 0x301AD dword [2n + 0xC076A] sar 16 */
           S16((uint32_t)(0xC0780 + 2 * nn)));                  /* 0x301A0 dword [2n + 0xC077E] sar 16 */
    f3C9D0(0x109, 0x185, 0x6E, 0x13, 5);                        /* 0x301DD; h = ecx 0x13 from 0x301C2 (f3AF8C keeps ecx) */
    fg_setcolor(5);                                             /* 0x301E4 */
    f3A0B0(0x109, 0x189, 0x6E, DSTR(D32((uint32_t)(0xC768C + 4 * nn))));   /* 0x301FE [4n + 0xC768C]; 0x30205 */
    f788AC();                                                   /* 0x3020A */
    f78A20(0x2EE);                                              /* 0x30214 wait 750 ms */
}

/* ---- 0x302C4: the computer nations ------------------------------------------------------------------------------ */

/* 0x302C4: the computer takes the nations the humans left, by player count (orders and bugs in the file comment) */
void f302C4(void)
{
    int32_t players;            /* edx at 0x302D9; edi, ecx, ebx at the later re-reads */
    int32_t rem;                /* [ebp-0x1C] */
    int32_t n;                  /* [ebp-0x20] */
    int32_t mask;               /* [ebp-0x18] */
    uint32_t r;

    players = S32(0xF40F0u);                                    /* 0x302D9 mov edx, [0xF40F0] */
    if (players == 3) {                                         /* 0x302DF cmp edx,3; jne 0x30430 */
        rem = players;                                          /* 0x302E8 */
        for (n = 1; n <= 9; n++)                                /* 0x302EB, 0x302FA cmp 9; jg */
            if (S8((uint32_t)(n * 0xC7 + 0xF1566)) == 3)        /* 0x30300 imul; 0x30307 dword [+0xF1563] sar 24 */
                rem--;                                          /* 0x30318 */
        if (rem > 0 && D8(0xF162Du) == 0) {                     /* 0x3031D jle; 0x30323 cmp byte, 0; jne */
            f30174(1);                                          /* 0x30331 Africa */
            rem = rem_after_bug_a(rem);                         /* 0x30339 inc: BUG(1995) A */
        }
        if (rand2() == 0) {                                     /* 0x3033C game_rand; 0x30346 test; jne 0x3036E */
            if (rem > 0 && D8(0xF17BBu) == 0 && S8(0xF16F4u) != 3) {   /* 0x3034D, 0x30351, 0x3035A [0xF16F1] sar 24 */
                f30174(3);                                      /* 0x30367, 0x30390 Illyria */
                rem--;                                          /* 0x30398 */
            }
        } else {
            if (rem > 0 && D8(0xF16F4u) == 0 && S8(0xF17BBu) != 3) {   /* 0x30371, 0x30375, 0x3037E [0xF17B8] sar 24 */
                f30174(2);                                      /* 0x3038B, 0x30390 Iberia */
                rem--;                                          /* 0x30398 */
            }
        }
        if (rand2() == 0) {                                     /* 0x3039B game_rand; 0x303A5 test; jne 0x303D9 */
            if (rem > 0 && D8(0xF1882u) == 0 && S8(0xF1949u) != 3) {   /* 0x303AC, 0x303B4, 0x303C1 [0xF1946] sar 24 */
                f30174(4);                                      /* 0x303D2, 0x30407 Thrace */
                rem--;                                          /* 0x3040F */
            }
        } else {
            if (rem > 0 && D8(0xF1949u) == 0 && S8(0xF1882u) != 3) {   /* 0x303DC, 0x303E4, 0x303F1 [0xF187F] sar 24 */
                f30174(5);                                      /* 0x30402, 0x30407 Crete */
                rem--;                                          /* 0x3040F */
            }
        }
    } else if (players == 4) {                                  /* 0x30430 cmp edx,4; jne 0x305DF */
        rem = players;                                          /* 0x30439 */
        for (n = 1; n <= 9; n++)                                /* 0x3043C, 0x3044B */
            if (S8((uint32_t)(n * 0xC7 + 0xF1566)) == 3)        /* 0x30451, 0x30458 */
                rem--;                                          /* 0x30469 */
        if (rem > 0 && D8(0xF162Du) == 0) {                     /* 0x3046E, 0x30474 */
            f30174(1);                                          /* 0x30482 Africa */
            rem = rem_after_bug_a(rem);                         /* 0x3048A inc: BUG(1995) A */
        }
        if (rem > 0 && D8(0xF16F4u) == 0) {                     /* 0x3048D, 0x30493 */
            f30174(2);                                          /* 0x304A1 Iberia */
            rem = rem_after_bug_a(rem);                         /* 0x304A9 inc: BUG(1995) A */
        }
        mask = 0;                                               /* 0x304AC */
        if (S8(0xF17BBu) == 3)                                  /* 0x304B1 [0xF17B8] sar 24 */
            mask += 4;                                          /* 0x304BE */
        if (S8(0xF1882u) == 3)                                  /* 0x304C2 [0xF187F] sar 24 */
            mask += 2;                                          /* 0x304CF */
        if (S8(0xF1949u) == 3)                                  /* 0x304D3 [0xF1946] sar 24 */
            mask++;                                             /* 0x304E0 */
        if ((uint32_t)mask <= 7) {                              /* 0x304E6 cmp 7; ja 0x30412 */
            switch (mask) {                                     /* 0x304EF jmp cs:[4*eax + 0x30224] */
            case 0:                                             /* 0x30521 */
                r = rand3();                                    /* 0x30521..0x3052D */
                if (r == 0) {                                   /* 0x30530 jb 0x305D2; test; je 0x30545 */
                    f30174(3);                                  /* 0x3054A */
                    rem--;                                      /* 0x30552 */
                    if (rem > 0) {                              /* 0x30555 jle 0x30412 */
                        f30174(4);                              /* 0x30564 */
                        rem--;                                  /* 0x3056C */
                    }
                } else if (r == 1) {                            /* 0x30539 jbe 0x30574 */
                    f30174(5);                                  /* 0x30579 */
                    rem--;                                      /* 0x30581 */
                    if (rem > 0) {                              /* 0x30584 */
                        f30174(3);                              /* 0x30593 */
                        rem--;                                  /* 0x3059B */
                    }
                } else if (r == 2) {                            /* 0x3053B cmp 2; je 0x305A3 */
                    f30174(4);                                  /* 0x305A8 */
                    rem--;                                      /* 0x305B0 */
                    if (rem > 0) {                              /* 0x305B3 jle 0x30412 */
                        f30174(5);                              /* 0x305C2 (0x305BD) */
                        rem--;                                  /* 0x305CA */
                    }
                }                                               /* 0x30540 jmp 0x30412 (r > 2: none) */
                break;
            case 1:                                             /* 0x304F7: Crete human */
                f30174(rand2() == 0 ? 3 : 4);                   /* 0x304F7..0x30511 */
                rem--;                                          /* 0x30519 */
                break;
            case 2:                                             /* 0x305BD: Thrace human */
            case 4:                                             /* 0x305BD: Illyria human */
                f30174(5);                                      /* 0x305C2 */
                rem--;                                          /* 0x305CA */
                break;
            default:                                            /* 0x30412: two or three human */
                break;
            }
        }
    } else if (players == 5) {                                  /* 0x305DF cmp edx,5; jne 0x30412 */
        rem = players;                                          /* 0x305E8 */
        for (n = 1; n <= 9; n++)                                /* 0x305EB, 0x305FA */
            if (S8((uint32_t)(n * 0xC7 + 0xF1566)) == 3)        /* 0x30600, 0x30607 */
                rem--;                                          /* 0x30618 */
        if (rem > 0 && D8(0xF162Du) == 0) {                     /* 0x3061D, 0x30623 */
            f30174(1);                                          /* 0x30631 Africa */
            rem = rem_after_bug_a(rem);                         /* 0x30639 inc: BUG(1995) A */
        }
        if (rem > 0 && D8(0xF16F4u) == 0) {                     /* 0x3063C, 0x30642 */
            f30174(2);                                          /* 0x30650 Iberia */
            rem = rem_after_bug_a(rem);                         /* 0x30658 inc: BUG(1995) A */
        }
        if (rem > 0 && D8(0xF1A10u) == 0) {                     /* 0x3065B, 0x30661 */
            f30174(6);                                          /* 0x3066F Asia */
            rem = rem_after_bug_a(rem);                         /* 0x30677 inc: BUG(1995) A */
        }
        mask = 0;                                               /* 0x3067A */
        if (S8(0xF17BBu) == 3)                                  /* 0x3067F */
            mask += 4;                                          /* 0x3068C */
        if (S8(0xF1882u) == 3)                                  /* 0x30690 */
            mask += 2;                                          /* 0x3069D */
        if (S8(0xF1949u) == 3)                                  /* 0x306A1 */
            mask++;                                             /* 0x306AE */
        if ((uint32_t)mask <= 7) {                              /* 0x306B4 cmp 7; ja 0x30412 */
            switch (mask) {                                     /* 0x306BD jmp cs:[4*eax + 0x30244] */
            case 0:                                             /* 0x306EF */
                r = rand3();                                    /* 0x306EF..0x306FB */
                if (r == 0) {                                   /* 0x306FE jb 0x30741; test; je 0x30545 (4 players' code) */
                    f30174(3);                                  /* 0x3054A */
                    rem--;                                      /* 0x30552 */
                    if (rem > 0) {                              /* 0x30555 */
                        f30174(4);                              /* 0x30564 */
                        rem--;                                  /* 0x3056C */
                    }
                } else if (r == 1) {                            /* 0x30703 jbe 0x30574 (4 players' code) */
                    f30174(5);                                  /* 0x30579 */
                    rem--;                                      /* 0x30581 */
                    if (rem > 0) {                              /* 0x30584 */
                        f30174(3);                              /* 0x30593 */
                        rem--;                                  /* 0x3059B */
                    }
                } else if (r == 2) {                            /* 0x30709 cmp 2; jne 0x30412 */
                    f30174(4);                                  /* 0x30717 */
                    rem--;                                      /* 0x3071F */
                    if (rem > 0) {                              /* 0x30722 jle 0x30412 */
                        f30174(5);                              /* 0x30731 (0x3072C) */
                        rem--;                                  /* 0x30739 */
                    }
                }
                break;
            case 1:                                             /* 0x306C5 */
                f30174(rand2() == 0 ? 3 : 4);                   /* 0x306C5..0x306DF */
                rem--;                                          /* 0x306E7 */
                break;
            case 2:                                             /* 0x3072C */
            case 4:
                f30174(5);                                      /* 0x30731 */
                rem--;                                          /* 0x30739 */
                break;
            default:                                            /* 0x30412 */
                break;
            }
        }
    }

    players = S32(0xF40F0u);                                    /* 0x30412 mov edi, [0xF40F0] */
    if (players == 6) {                                         /* 0x30418 cmp edi,6; jne 0x308B1 */
        rem = players;                                          /* 0x30421 */
        for (n = 1; n <= 9; n++)                                /* 0x30424, 0x30754 */
            if (S8((uint32_t)(n * 0xC7 + 0xF1566)) == 3)        /* 0x3075A, 0x30761 */
                rem--;                                          /* 0x30772 */
        if (rem > 0 && D8(0xF162Du) == 0) {                     /* 0x30777, 0x3077D */
            f30174(1);                                          /* 0x3078B Africa */
            rem = rem_after_bug_a(rem);                         /* 0x30793 inc: BUG(1995) A */
        }
        if (rem > 0 && D8(0xF16F4u) == 0) {                     /* 0x30796, 0x3079C */
            f30174(2);                                          /* 0x307AA Iberia */
            rem--;                                              /* 0x307B2 */
        }
        if (rem > 0 && D8(0xF1949u) == 0) {                     /* 0x307B5, 0x307BB */
            f30174(5);                                          /* 0x307C9 Crete */
            rem--;                                              /* 0x307D1 */
        }
        if (rem > 0 && D8(0xF1C65u) == 0) {                     /* 0x307D4, 0x307DA */
            f30174(9);                                          /* 0x307E8 Egypt */
            rem--;                                              /* 0x307F0 */
        }
        if (rand2() == 0) {                                     /* 0x307F3 game_rand; 0x307FD test; jne 0x30825 */
            if (rem > 0 && D8(0xF1A10u) == 0 && S8(0xF1AD7u) != 3) {   /* 0x30801, 0x30808, 0x30811 [0xF1AD4] sar 24 */
                f30174(6);                                      /* 0x3081E, 0x30847 Asia */
                rem--;                                          /* 0x3084F */
            }
        } else {
            if (rem > 0 && D8(0xF1AD7u) == 0 && S8(0xF1A10u) != 3) {   /* 0x30825, 0x3082C, 0x30835 [0xF1A0D] sar 24 */
                f30174(7);                                      /* 0x30842, 0x30847 Assyria */
                rem--;                                          /* 0x3084F */
            }
        }
        if (rand2() == 0) {                                     /* 0x30852 game_rand; 0x3085C test; jne 0x30884 */
            if (rem > 0 && D8(0xF17BBu) == 0 && S8(0xF1882u) != 3) {   /* 0x30860, 0x30867, 0x30870 [0xF187F] sar 24 */
                f30174(3);                                      /* 0x3087D, 0x308A6 Illyria */
                rem--;                                          /* 0x308AE */
            }
        } else {
            if (rem > 0 && D8(0xF1882u) == 0 && S8(0xF17BBu) != 3) {   /* 0x30884, 0x3088B, 0x30894 [0xF17B8] sar 24 */
                f30174(4);                                      /* 0x308A1, 0x308A6 Thrace */
                rem--;                                          /* 0x308AE */
            }
        }
    }

    players = S32(0xF40F0u);                                    /* 0x308B1 mov ecx, [0xF40F0] */
    if (players == 7) {                                         /* 0x308B7 cmp ecx,7; jne 0x30AB9 */
        rem = players;                                          /* 0x308C0 */
        for (n = 1; n <= 9; n++)                                /* 0x308C3, 0x308D2 */
            if (S8((uint32_t)(n * 0xC7 + 0xF1566)) == 3)        /* 0x308D8, 0x308DF */
                rem--;                                          /* 0x308F0 */
        mask = 0;                                               /* 0x308F5 */
        if (S8(0xF1A10u) == 3)                                  /* 0x308FA [0xF1A0D] sar 24: Asia */
            mask += 4;                                          /* 0x30907 */
        if (S8(0xF1AD7u) == 3)                                  /* 0x3090B [0xF1AD4] sar 24: Assyria */
            mask += 2;                                          /* 0x30918 */
        if (S8(0xF1B9Eu) == 3)                                  /* 0x3091C [0xF1B9B] sar 24: Babylon */
            mask++;                                             /* 0x30929 */
        if ((uint32_t)mask <= 7) {                              /* 0x3092F cmp 7; ja 0x30965 */
            switch (mask) {                                     /* 0x30938 jmp cs:[4*eax + 0x30264] */
            case 0:                                             /* 0x30A0D */
                r = rand3();                                    /* 0x30A0D..0x30A19 */
                if (r == 0) {                                   /* 0x30A1C jb 0x30A8B; test; je 0x30A5C (as r == 2) */
                    f30174(7);                                  /* 0x30A61 */
                    rem--;                                      /* 0x30A69 */
                    if (rem > 0) {                              /* 0x30A6C jle 0x30965 */
                        f30174(8);                              /* 0x30A7B (0x30A76) */
                        rem--;                                  /* 0x30A83 */
                    }
                } else if (r == 1) {                            /* 0x30A21 jbe 0x30A2D */
                    f30174(8);                                  /* 0x30A32 */
                    rem--;                                      /* 0x30A3A */
                    if (rem > 0) {                              /* 0x30A3D jle 0x30965 */
                        f30174(6);                              /* 0x30A4C */
                        rem--;                                  /* 0x30A54 */
                    }
                } else if (r == 2) {                            /* 0x30A23 cmp 2; je 0x30A5C */
                    f30174(7);                                  /* 0x30A61 */
                    rem--;                                      /* 0x30A69 */
                    if (rem > 0) {                              /* 0x30A6C */
                        f30174(8);                              /* 0x30A7B (0x30A76) */
                        rem--;                                  /* 0x30A83 */
                    }
                }                                               /* 0x30A28 jmp 0x30965 */
                break;
            case 1:                                             /* 0x30940: Babylon human */
                f30174(rand2() == 0 ? 6 : 7);                   /* 0x30940..0x3095A */
                rem--;                                          /* 0x30962, falls to 0x30965 */
                break;
            case 2:                                             /* 0x30A76: Assyria human */
            case 4:                                             /* 0x30A76: Asia human */
                f30174(8);                                      /* 0x30A7B */
                rem--;                                          /* 0x30A83 */
                break;
            default:                                            /* 0x30965 */
                break;
            }
        }
        if (rem > 0 && D8(0xF162Du) == 0) {                     /* 0x30965, 0x3096B */
            f30174(1);                                          /* 0x30979 Africa */
            rem = rem_after_bug_a(rem);                         /* 0x30981 inc: BUG(1995) A */
        }
        if (rem > 0 && D8(0xF1C65u) == 0) {                     /* 0x30984, 0x3098A */
            f30174(9);                                          /* 0x30998 Egypt */
            rem--;                                              /* 0x309A0 */
        }
        if (rem > 0 && D8(0xF16F4u) == 0) {                     /* 0x309A3, 0x309A9 */
            f30174(2);                                          /* 0x309B7 Iberia */
            rem--;                                              /* 0x309BF */
        }
        mask = 0;                                               /* 0x309C2 */
        if (S8(0xF17BBu) == 3)                                  /* 0x309C7 */
            mask += 4;                                          /* 0x309D4 */
        if (S8(0xF1882u) == 3)                                  /* 0x309D8 */
            mask += 2;                                          /* 0x309E5 */
        if (S8(0xF1949u) == 3)                                  /* 0x309E9 */
            mask++;                                             /* 0x309F6 */
        if ((uint32_t)mask <= 7) {                              /* 0x309FC cmp 7; ja 0x30AB9 */
            switch (mask) {                                     /* 0x30A05 jmp cs:[4*eax + 0x30284] */
            case 0:                                             /* 0x30AD7 */
                r = rand3();                                    /* 0x30AD7..0x30AE3 */
                if (r == 0) {                                   /* 0x30AE6 jb 0x30B7A; test; je 0x30AF8 */
                    f30174(3);                                  /* 0x30AFD */
                    rem--;                                      /* 0x30B05 */
                    if (rem > 0) {                              /* 0x30B08 jle 0x30AB9 */
                        f30174(4);                              /* 0x30B13 */
                        rem--;                                  /* 0x30B1B */
                    }
                } else if (r == 1) {                            /* 0x30AEF jbe 0x30B20 */
                    f30174(5);                                  /* 0x30B25 */
                    rem--;                                      /* 0x30B2D */
                    if (rem > 0) {                              /* 0x30B30 */
                        f30174(3);                              /* 0x30B3B */
                        rem--;                                  /* 0x30B43 */
                    }
                } else if (r == 2) {                            /* 0x30AF1 cmp 2; je 0x30B4B */
                    f30174(4);                                  /* 0x30B50 */
                    rem--;                                      /* 0x30B58 */
                    if (rem > 0) {                              /* 0x30B5B jle 0x30AB9 */
                        f30174(5);                              /* 0x30B6A (0x30B65) */
                        rem--;                                  /* 0x30B72 */
                    }
                }                                               /* 0x30AF6 jmp 0x30AB9 */
                break;
            case 1:                                             /* 0x30A94: Crete human */
                r = rand2();                                    /* 0x30A94..0x30A9E */
                /* BUG(1995): B, the take is unguarded; with rem <= 0 it is one computer nation too many */
                if (g_original || rem > 0) {
                    f30174(r == 0 ? 3 : 4);                     /* 0x30AA2 / 0x30AA9, 0x30AAE */
                    rem--;                                      /* 0x30AB6 */
                }
                break;
            case 2:                                             /* 0x30B65: Thrace human */
            case 4:                                             /* 0x30B65: Illyria human */
                /* BUG(1995): B, unguarded, as case 1 */
                if (g_original || rem > 0) {
                    f30174(5);                                  /* 0x30B6A */
                    rem--;                                      /* 0x30B72 */
                }
                break;
            default:                                            /* 0x30AB9 */
                break;
            }
        }
    }

    players = S32(0xF40F0u);                                    /* 0x30AB9 mov ebx, [0xF40F0] */
    if (players == 8) {                                         /* 0x30ABF cmp ebx,8; jne 0x30CDA */
        rem = players;                                          /* 0x30AC8 */
        for (n = 1; n <= 9; n++)                                /* 0x30ACB, 0x30B8D */
            if (S8((uint32_t)(n * 0xC7 + 0xF1566)) == 3)        /* 0x30B93, 0x30B9A */
                rem--;                                          /* 0x30BAB */
        mask = 0;                                               /* 0x30BB0 */
        if (S8(0xF1A10u) == 3)                                  /* 0x30BB5 */
            mask += 4;                                          /* 0x30BC2 */
        if (S8(0xF1AD7u) == 3)                                  /* 0x30BC6 */
            mask += 2;                                          /* 0x30BD3 */
        if (S8(0xF1B9Eu) == 3)                                  /* 0x30BD7 */
            mask++;                                             /* 0x30BE4 */
        if ((uint32_t)mask <= 7) {                              /* 0x30BEA cmp 7; ja 0x30C20 */
            switch (mask) {                                     /* 0x30BF3 jmp cs:[4*eax + 0x302A4] */
            case 0:                                             /* 0x30CE4 */
                r = rand3();                                    /* 0x30CE4..0x30CF0 */
                if (r == 0) {                                   /* 0x30CF3 jb 0x30D4E; test; je 0x30D29 (as r == 2) */
                    f30174(7);                                  /* 0x30D2E */
                    rem--;                                      /* 0x30D36 */
                    f30174(8);                                  /* 0x30D3E (no rem test) */
                    rem--;                                      /* 0x30D46 */
                } else if (r == 1) {                            /* 0x30CF8 jbe 0x30D04 */
                    f30174(6);                                  /* 0x30D09 */
                    rem--;                                      /* 0x30D11 */
                    f30174(8);                                  /* 0x30D19 (0x30D14, no rem test) */
                    rem--;                                      /* 0x30D21 */
                } else if (r == 2) {                            /* 0x30CFA cmp 2; je 0x30D29 */
                    f30174(7);                                  /* 0x30D2E */
                    rem--;                                      /* 0x30D36 */
                    f30174(8);                                  /* 0x30D3E */
                    rem--;                                      /* 0x30D46 */
                }                                               /* 0x30CFF jmp 0x30C20 */
                break;
            case 1:                                             /* 0x30BFB: Babylon human */
                f30174(rand2() == 0 ? 6 : 7);                   /* 0x30BFB..0x30C15 */
                rem--;                                          /* 0x30C1D, falls to 0x30C20 */
                break;
            case 2:                                             /* 0x30D14: Assyria human */
            case 4:                                             /* 0x30D14: Asia human */
                f30174(8);                                      /* 0x30D19 */
                rem--;                                          /* 0x30D21 */
                break;
            default:                                            /* 0x30C20 */
                break;
            }
        }
        if (rem > 0 && D8(0xF162Du) == 0) {                     /* 0x30C20, 0x30C26 */
            f30174(1);                                          /* 0x30C34 Africa */
            rem = rem_after_bug_a(rem);                         /* 0x30C3C inc: BUG(1995) A */
        }
        if (rem > 0 && D8(0xF1C65u) == 0) {                     /* 0x30C3F, 0x30C45 */
            f30174(9);                                          /* 0x30C53 Egypt */
            rem--;                                              /* 0x30C5B */
        }
        if (rem > 0 && D8(0xF16F4u) == 0) {                     /* 0x30C5E, 0x30C64 */
            f30174(2);                                          /* 0x30C72 Iberia */
            rem--;                                              /* 0x30C7A */
        }
        if (rem > 0 && D8(0xF17BBu) == 0) {                     /* 0x30C7D, 0x30C83 */
            f30174(3);                                          /* 0x30C91 Illyria */
            rem--;                                              /* 0x30C99 */
        }
        if (rem > 0 && D8(0xF1882u) == 0) {                     /* 0x30C9C, 0x30CA2 */
            f30174(4);                                          /* 0x30CB0 Thrace */
            rem--;                                              /* 0x30CB8 */
        }
        if (rem > 0 && D8(0xF1949u) == 0) {                     /* 0x30CBB, 0x30CC1 */
            f30174(5);                                          /* 0x30CCF Crete */
            rem--;                                              /* 0x30CD7 */
        }
    }
}                                                               /* 0x30CDA */
