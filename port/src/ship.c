/* Module SHIP of docs/M6_PLAN.md: the ship phases, human side, and their shared helpers. Literal translations; the
   address of each instruction a line comes from is in its comment. D = read in the code, G = inference. Prototypes and
   the records: ship.h.

     0x60AE0  f60AE0  the ship-phase order (both phases)
     0x60BD8  f60BD8  SHIP MAINTENANCE (phase 3)
     0x6106C  f6106C  a computer maintains or scraps one ship (its only caller is SHIPAI's 0x61254)
     0x61940  f61940  SHIP CONSTRUCTION (phase 4), "DoShipContruction" by its alloc tags
     0x62488  f62488  a computer builds one ship
     0x62658  f62658  the candidate sites
     0x62774  f62774  a human builds one ship
     0x7A598  f7A598  the world snapshot before / after the housekeeping
     0x7A610  f7A610  back from it

   Not ported here (module SHIPAI and AIMOVE of docs/M6_PLAN.md, M7): each call site stops the port with
   not_ported(address); the statements after it are kept as in the original (as loop.c does with PBEM):
     0x61254  a computer's ship maintenance        f60BD8 0x60ED9 (+0x9A == 2)
     0x62840  the computers' construction search   f61940 0x62197 (computers, and humans with 4 ships, D 0x61AF1..
                                                   0x61B05)
     0x461BC  the AI planner (AIMOVE)              f60BD8 0x60F6A, f61940 0x62109: the "housekeeping" after every
                                                   nation but the last one of 0xF06B0 when int8 [0xE1621] > 0, so in a
                                                   game with computers it runs after a human's turn too (D)
   So a phase with computers stops at the first computer with ships (maintenance) or with a site (construction), or
   at the first housekeeping; an all-human game (0 computers) runs both phases to the end. With SHIP_TEST
   (port/test/ship_test.c) the three go to the test's recorders, which return, so the code after them can be checked.

   Play by e-mail (dropped by Erik's ruling): the bit-0x80 tests of byte [0xE1624] are kept as data tests (they only
   choose whether the nation int8 [0xE1622], the PBEM local player, gets the human screens); no PBEM routine is called
   from these functions.

   Watcom runtime -> standard C:
     0x7DB3D sprintf -> snprintf bounded to the destination: 0xF5500 (500 bytes, as loop.c), the stack buffers 100
             bytes (0x60BD8 [ebp-0x9C], 0x6106C [ebp-0xB0], 0x61940 [ebp-0xBC], 0x62488 [ebp-0xA4]); the inlined
             strcpy of the payment word into 32 bytes ([ebp-0x4C] / [ebp-0x40])
     0x822B5 malloc(size, tag "TempTerr" / "Tempnation", "DoShipContruction") -> load_heap_alloc (port handles in
             [0xEED74] / [0xEED78], as stack.c's dcase_50 / dcase_51 free them); 0x877E6 free -> load_heap_free
     0x8A080 clock: read twice per housekeeping from round 7 (start, and the busy wait until start + 250 / 10
             hundredths, 0x60F86 / 0x62125, unsigned ja). Port (G): ui_kit2.c's clock_8A080 is static, so the wait
             is f78A20(250), which equals the original when f461BC spends no clock time (open: export the clock).
             (f461BC is ported since M7 batch 1, 21 Nov)
     rep movsd / movsb -> memcpy inside g_ds (and into the heap blocks); 0x7D4AC (__CHK) dropped; 0x78B24 (the alloc
             trace, called with the size or 0 after each malloc / free) is a no-op (game.h).
   game_rand f78AE4: none drawn here (D); the phase's random numbers are all in SHIPAI (0x61254, 0x62840) and f461BC.

   Data (D, read in the code at the addresses given; G for meanings not stated by the code):
     0xBB3FA..0xBB400 int16 message area x, y, w, h (169, 340, 300, 35 in both phases); 0xEE168 int8 the background
       number f3A924 / f3A990 reload (3); [0xF52C8] back buffer, [0xF52CC] background buffer; 0xF596C the panel dialog;
       [0xF5940] int32 the first player-button record of 0xF596C (f783B8 / f7844C)
     0xF40F8 int32 the nation now acting; 0xF40FC int16 the round; 0xE1621 int8 computers; 0xE1622 int8 the PBEM local
       player; 0xE1624 bit 0x80 PBEM; 0xF097C bit 0x08 Restart Confirmations; 0xF097F bit 0x40 / 0x80 CONTINUE after a
       computer's maintenance / construction message (G: the AI pause toggles); 0xF40BA int16 the self-running demo
     0xC768C names, 0xC76C4 adjectives, 0xC77D8 territory names (uint32 link addresses)
     0x108804 int8 1 land mode / 2 sea mode (2 during construction, so the map draws the ship boxes, G); 0x10F2C0 /
       0x10F2C4 int32 the AI think-time counters (zeroed); 0xF5470 int32 (no writer found, M6_PLAN risk 13; the extra
       ships need it >= 3)
     0x10F6B8 + 18r: SHIPAI's options (D from the reads at 0x621BA..0x62223): +2 int8 ships, +3 + j uint8 site of ship
       j, +7 + j int8 treasury, +11 + j int8 levy
     0x112CEC / 0x1137D0 / 0x1139B0 the snapshot of f7A598 (nations 1..11, territory state, areas 1..160)

   1995 bugs (fixed unless g_original, each marked BUG(1995) below):
     A. f60BD8, MAINTAIN BY LEVY (0x60DDF): the token is taken off the canonical territory c = 0xC803C[t] (0x60DB9), but
        f581C4 gets the ship's own t. For a ship on a second coast (t 157..159) f581C4 then looks at the alias record,
        where n has no marker, so when that was n's last token at c, c keeps n's marker and stays in n's territory list
        with 0 tokens. Construction does it right (f581C4(c, n), 0x61EDD). Fixed: f581C4(c, n). D (code), G (reach:
        a ship of n on 157..159 and its last token there paid as levy).
     B. f61940, the computers' extra ships (0x62346..0x623E6): the bubble sort of the coastal candidates by n's tokens
        reads area record cand (territory cand - 1, [cand*69 + n - 1 + 0xE31C7]) instead of cand + 1. Fixed: record
        cand + 1. D (code), G (effect: the order of the computer's extra ships).
     C. f61940, the same loop: with no candidate (count 0) it builds at cand[0] (0x623EE), which no store wrote: stack
        garbage on the first nation, the previous nation's first candidate later. Fixed: the extra ships stop (the loop
        goes to the housekeeping). Under g_original the port keeps the array across nations as the original's frame does
        and starts it at 0 (the garbage itself cannot be reproduced). D (code), G (reach: a computer with treasury >= 2,
        stock < 4, cities, [0xF5470] >= 3 and every coastal territory of its list holding one of its ships).
     B and C are latent (G): the loop needs int32 [0xF5470] >= 3, and nothing writes 0xF5470..0xF5473: the only two
     references in the code are reads (fixups 0x46971 and 0x62259, both cmp dword [0xF5470], 3), the save has the int16
     at 0xF5474 but not these bytes, and obj2 holds 0 there. So the extra ships never happen unless a block write not
     found yet covers them (M6_PLAN risk 13).
   Kept as the original (D unless marked):
     - f60BD8: any click code other than -5 / -6 / -8 / -100 ends the ship's screen with no payment and no scrap (the
       ship is kept for free). G: which buttons of 0xF596C are enabled then and what the dispatcher returns for them
       is not settled; the three buttons of 0xC6E40 return only -5 / -6 / -8 (dispatch.c).
     - f62488 / f62774 do not reset the new slot's moves +0x41 + k or tokens aboard +0x4D + k (f78D98 does not shift
       them either, rules.c); G: harmless if the movement phase sets both before use.
     - f62658 pre-fills 157 entries with -1 but scans 160 territories (and lists the second coasts 157..159 as sites of
       their own when their territory byte has bit 0x02).
     - f62774's slot search has no bound (a byte; it relies on fewer than 4 ships).
     - f6106C and f62488 pass t itself to f57CE8 (no 0xC803C); SHIPAI chooses t (G: canonical there).
     - The two allocation tags spell "DoShipContruction" (OVERVIEW §6, egg 10; never shown). No text slips in the
       shown strings.
   Oddities (D): the empty "nothing happened" messages 0xB61A6 / 0xB6311 (a pause and the home view of the first
   nation, M6_PLAN egg 15); the developer housekeeping messages from round 7 (egg 12); f6106C shows its message twice
   (once without a wait, then timed); RESTART in construction restores n's record and all 161 area records from the
   heap blocks, but not the territory state or the other nations. */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "fg.h"
#include "game.h"
#include "kit.h"
#include "map.h"
#include "mapdata.h"
#include "move.h"
#include "panel.h"
#include "rules.h"
#include "ship.h"
#include "stack.h"
#include "start.h"
#include "state.h"
#include "ui.h"

/* The computers' routines (SHIPAI / AIMOVE, M7). With SHIP_TEST they go to the test's recorders, which return. */
#ifdef SHIP_TEST
void ship_test_f61254(int n);
int  ship_test_f62840(int n);
void ship_test_f461BC(int n);
#define CALL_F61254(n) ship_test_f61254(n)
#define CALL_F62840(n) ship_test_f62840(n)
#define CALL_F461BC(n) ship_test_f461BC(n)
#else
#include "aimove.h"
#include "aiship.h"
#define CALL_F61254(n) f61254(n)                    /* aiship.c (M7 batch 2) */
#define CALL_F62840(n) f62840(n)                    /* aiship.c (M7 batch 2) */
#define CALL_F461BC(n) f461BC(n)                    /* aimove.c (M7 batch 1) */
#endif

#define FLAGS        0xE1624u
#define PBEM_BIT     0x80u
#define COMPUTERS    0xE1621u   /* int8 */
#define PBEM_LOCAL   0xE1622u   /* int8 */
#define ROUND        0xF40FCu   /* int16 */
#define CUR_NATION   0xF40F8u   /* int32 */
#define PANEL        0xF596Cu   /* the map screen's panel dialog */
#define PLAYER_BASE  0xF5940u   /* int32 */
#define ORDER_COUNT  0xF06AEu   /* int16 */
#define ORDER        0xF06B0u   /* int16[] */
#define CENSUS_COUNT 0xF0698u   /* int16 */
#define CENSUS       0xF069Au   /* int16[] */
#define SITE_COUNT   0xF7C9Cu   /* int32 */
#define SITES        0xF7CA0u   /* int32[] */
#define MSG_BUF      0xF5500u   /* 500 bytes */
#define DLG_MAINT    0xC6E40u
#define DLG_BUILD    0xC722Cu
#define TEMP_TERR    0xEED74u   /* heap handle: area records 0..160 */
#define TEMP_NATION  0xEED78u   /* heap handle: one nation record */
#define SEA_MODE     0x108804u  /* int8 */
#define AREA0        0xE31B8u
#define AREA_SIZE    0x2B65u    /* 161 * 69 */
#define NAT_SIZE     0xC7u

#define NATION(n)    (0xF14CCu + 199u * (uint32_t)(n))
#define AREA(t)      (AREA0 + 69u * (uint32_t)((t) + 1))   /* the area record of territory t */
#define NAME(n)      DSTR(D32(0xC768Cu + 4u * (uint32_t)(n)))
#define ADJ(n)       DSTR(D32(0xC76C4u + 4u * (uint32_t)(n)))
#define TNAME(t)     DSTR(D32(0xC77D8u + 4u * (uint32_t)(t)))
#define CANON(t)     ((int32_t)D8(0xC803Cu + (uint32_t)(t)))
#define TERRFLAG(t)  D8(0xE1638u + 35u * (uint32_t)(t))

/* the background 3 (plainbck, G) into the back buffer: f3CD6C(3), byte [0xEE168] = 3, fg_vbcopy of the whole screen
   from [0xF52CC] to [0xF52C8] (0x60C36..0x60C66, 0x60E08..0x60E3D, 0x619FA..0x61A2E, 0x61B3A..0x61B66; in each the
   byte is stored after the call of f3CD6C and both handles are read after it) */
static void background3(void)
{
    f3CD6C(3);
    setD8(0xEE168u, 3);
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(0xF52CCu), S32(0xF52C8u));
}

/* message area (169, 340, 300, 35): 0x60C01..0x60C2F, 0x619AF..0x619DA */
static void message_area(void)
{
    setD16(0xBB3FAu, 0xA9);
    setD16(0xBB3FCu, 0x154);
    setD16(0xBB3FEu, 0x12C);
    setD16(0xBB400u, 0x23);
}

/* the housekeeping after nation n (0x60EDE..0x60FA2 / 0x6207D..0x62141): with computers and n not the last of the
   order. text is "Maintenance housekeeping for %s" (0xB6186) or "Construction housekeeping for %s" (0xB62B8). */
static void housekeeping(int32_t n, const char *fmt)
{
    if (S8(COMPUTERS) <= 0)                                             /* 0x60EDE dword [0xE161E] sar 24; jle */
        return;
    /* 0x60EEE: int16 [0xF06B0 + 2 * (int16 [0xF06AE] - 1)] (dword [2 * count + 0xF06AC] sar 16), the last nation */
    if ((int32_t)S16(ORDER + 2u * (uint32_t)((int32_t)S16(ORDER_COUNT) - 1)) == n)   /* 0x60F03 cmp; je */
        return;
    if ((int32_t)S16(ROUND) >= 7) {                                     /* 0x60F0B dword [0xF40FA] sar 16; cmp 7; jl */
        /* 0x60F18 [ebp-0x24] = 250; 0x60F1F start = clock() (port: see the file comment) */
        snprintf((char *)DP(MSG_BUF), 0x1F4, fmt, NAME(n));            /* 0x60F3C */
        f79270(-550, n, -250, DSTR(MSG_BUF), 0);                        /* 0x60F58 eax -0x226, ebx -0xFA */
    }
    f7A940();                                                           /* 0x60F5D push the world */
    f7A610();                                                           /* 0x60F62 the snapshot back */
    CALL_F461BC(n);                                                     /* 0x60F6A f461BC(n) [AIMOVE] */
    f7A598();                                                           /* 0x60F6F a new snapshot */
    f7AC60();                                                           /* 0x60F74 pop the world */
    if ((int32_t)S16(ROUND) >= 7)                                       /* 0x60F79 */
        f78A20(250);                                                    /* 0x60F86..0x60FA2: until start + 250 / 10
                                                                           (port: see the file comment) */
}

/* ---- 0x60AE0: the ship-phase order -------------------------------------------------------------------------------- */

void f60AE0(void)
{
    int32_t i;                                                          /* [ebp-0x14] */
    int32_t c;

    setD16(ORDER_COUNT, 0);                                             /* 0x60AF6 */
    for (i = 0; (int32_t)S16(CENSUS_COUNT) > i; i++) {                  /* 0x60B0A dword [0xF0696] sar 16; jle */
        /* 0x60B1B dword [2i + 0xF0698] sar 16: int16 0xF069A[i]; imul 0xC7; byte [.. + 0xF1501]: +0x35 */
        if (D8(NATION(S16(CENSUS + 2u * (uint32_t)i)) + 0x35u) != 0)   /* 0x60B31 test ah, ah; jne */
            continue;
        c = (int32_t)S16(ORDER_COUNT);                                  /* 0x60B38 movsx */
        setD16(ORDER_COUNT, (uint16_t)(c + 1));                         /* 0x60B51 */
        setD16(ORDER + 2u * (uint32_t)c, D16(CENSUS + 2u * (uint32_t)i));   /* 0x60B57 */
    }
    for (i = 0; (int32_t)S16(CENSUS_COUNT) > i; i++) {                  /* 0x60B6D */
        if (D8(NATION(S16(CENSUS + 2u * (uint32_t)i)) + 0x35u) == 0)   /* 0x60B94 test bl, bl; je */
            continue;
        c = (int32_t)S16(ORDER_COUNT);                                  /* 0x60BA9 */
        setD16(ORDER + 2u * (uint32_t)c, D16(CENSUS + 2u * (uint32_t)i));   /* 0x60BAD */
        setD16(ORDER_COUNT, (uint16_t)(c + 1));                         /* 0x60BB5 */
    }
    f783B8(PANEL, S32(PLAYER_BASE));                                    /* 0x60BC8 */
}

/* ---- 0x60BD8: SHIP MAINTENANCE --------------------------------------------------------------------------------- */

void f60BD8(void)
{
    int32_t nobody;                                                     /* [ebp-0x34] */
    int32_t i;                                                          /* [ebp-0x38] */
    int32_t n;                                                          /* [ebp-0x1C] */
    uint32_t rec;                                                       /* [ebp-0x18] */
    int32_t k;                                                          /* [ebp-0x2C] */
    int32_t t;                                                          /* [ebp-0x30] */
    int32_t c;
    int32_t code;                                                       /* [ebp-0x28] */
    char buf[100];                                                      /* [ebp-0x9C] */

    if (S8(COMPUTERS) > 0)                                              /* 0x60BF0 dword [0xE161E] sar 24; jle */
        f7A598();                                                       /* 0x60BFC */
    message_area();                                                     /* 0x60C01..0x60C2F */
    background3();                                                      /* 0x60C36..0x60C66 */
    nobody = 1;                                                         /* 0x60C73 */
    f7A8D0();                                                           /* 0x60C78 */
    for (i = 0; (int32_t)S16(ORDER_COUNT) > i; i++) {                   /* 0x60FAA dword [0xF06AC] sar 16; jle 0x61038 */
        n = (int32_t)S16(ORDER + 2u * (uint32_t)i);                     /* 0x60FBE dword [2i + 0xF06AE] sar 16 */
        setD32(CUR_NATION, (uint32_t)n);                                /* 0x60FCE */
        rec = NATION(n);                                                /* 0x60FE1 */
        /* 0x60FE4..0x61013: in play and not PBEM, or PBEM and n is the local player */
        if (!(((D8(FLAGS) & PBEM_BIT) == 0 && D8(rec + 0x9Au) != 0)
              || ((D8(FLAGS) & PBEM_BIT) != 0 && (int32_t)S8(PBEM_LOCAL) == n))) {
            housekeeping(n, "Maintenance housekeeping for %s");        /* -> 0x60EDE */
            continue;
        }
        if ((int32_t)S8(rec + 0x9Au) == 3) {                            /* 0x61019 dword [rec + 0x97] sar 24; cmp 3 */
            for (k = 0; (int32_t)D8(rec + 0x40u) > k; k++) {            /* 0x6102E k = 0; 0x60DF4 cmp; jle 0x60EDE */
                background3();                                          /* 0x60E08..0x60E3D */
                f7876C();                                               /* 0x60E45 the phase name */
                f7853C(DLG_MAINT);                                      /* 0x60E4F */
                f3B05C(PANEL);                                          /* 0x60E59 */
                t = (int32_t)S16(rec + 0x45u + 2u * (uint32_t)k) - 1;   /* 0x60E66 dword [rec + 2k + 0x43] sar 16; dec */
                f3F670();                                               /* 0x60E70 */
                f3F624(t, 0x0F);                                        /* 0x60E7D */
                f3DA60(t);                                              /* 0x60E85 */
                if (S8(rec + 0x3Eu) > 0)                                /* 0x60E8D dword [rec + 0x3B] sar 24; jle 0x60C85 */
                    f3C314(PANEL, 0x34);                                /* 0x60EA5 MAINTAIN FROM TREASURY */
                else
                    f3C3D0(PANEL, 0x34);                                /* 0x60C8F */
                c = CANON(t);                                           /* 0x60C97 */
                if (S8(AREA(c) + 0x0Eu + (uint32_t)n) > 0)              /* 0x60CB4 dword [.. + 0xE31C3] sar 24; jle */
                    f3C314(PANEL, 0x35);                                /* 0x60CCB MAINTAIN BY LEVY */
                else
                    f3C3D0(PANEL, 0x35);                                /* 0x60CDC */
                f459E4(n);                                              /* 0x60CE4 */
                f3A924();                                               /* 0x60CE9 */
                snprintf(buf, sizeof buf, "%s, select maintenance options for\nShip #%d in %s",   /* 0x60D15 (0xB6154) */
                         NAME(n), k + 1, TNAME(t));
                f3AB7C(buf);                                            /* 0x60D23 */
                f7844C(PANEL, S32(PLAYER_BASE));                        /* 0x60D35 */
                f3CBBC();                                               /* 0x60D3A */
                nobody = 0;                                             /* 0x60D3F ecx = 0 (0x60D33, kept across) */
                do
                    code = f78BA4(PANEL, 0);                            /* 0x60D49 */
                while (code == -100);                                   /* 0x60D54 */
                /* 0x60D59..0x60D6C, 0x60EAF (unsigned compares): -6, -5, -8; any other code: nothing */
                if (code == -6) {                                       /* 0x60D99 MAINTAIN BY LEVY */
                    c = CANON(t);                                       /* 0x60D9C */
                    setD8(AREA(c) + 0x0Eu + (uint32_t)n,
                          (uint8_t)(D8(AREA(c) + 0x0Eu + (uint32_t)n) - 1u));   /* 0x60DB9 dec byte [.. + 0xE31C6] */
                    setD8(rec + 0x99u, (uint8_t)(D8(rec + 0x99u) + 1u));       /* 0x60DC6 inc byte [.. + 0xF1565] */
                    setD8(rec + 0x98u, (uint8_t)(D8(rec + 0x98u) - 1u));       /* 0x60DD3 dec byte [.. + 0xF1564] */
                    if (g_original)
                        /* BUG(1995) A: the ship's own t, an alias on a second coast (file comment) */
                        f581C4(t, n);                                   /* 0x60DDF */
                    else
                        f581C4(c, n);
                } else if (code == -5) {                                /* 0x60D84 MAINTAIN FROM TREASURY */
                    setD8(rec + 0x3Eu, (uint8_t)(D8(rec + 0x3Eu) - 1u));       /* 0x60D87 */
                    setD8(rec + 0x99u, (uint8_t)(D8(rec + 0x99u) + 1u));       /* 0x60D91 */
                } else if (code == -8) {                                /* 0x60D71 DON'T MAINTAIN */
                    f78D98(k, n);                                       /* 0x60D77 */
                    k--;                                                /* 0x60D7F */
                }
                f78664(DLG_MAINT);                                      /* 0x60DE9 */
            }                                                           /* 0x60DF1 k++ */
        } else if ((int32_t)S8(rec + 0x9Au) == 2) {                     /* 0x60EBD cmp eax, 2; jne 0x60EDE */
            if (D8(rec + 0x40u) > 0)                                    /* 0x60EC2 */
                nobody = 0;                                             /* 0x60ED3 */
            CALL_F61254(n);                                             /* 0x60ED9 f61254(n) [SHIPAI] */
        }
        housekeeping(n, "Maintenance housekeeping for %s");            /* 0x60EDE (0xB6186) */
    }                                                                   /* 0x60FA7 i++ */
    f7ABD0();                                                           /* 0x61038 */
    if (nobody != 0)                                                    /* 0x6103D */
        f79270(250, (int32_t)S16(ORDER), -200, "", 0);                  /* 0x6105D (0xB61A6, empty); edx = the first
                                                                           nation, dword [0xF06AE] sar 16 */
}

/* ---- 0x6106C: a computer maintains or scraps ship k -------------------------------------------------------------- */

void f6106C(int k, int scrap, int tp1, int useToken, int n)
{
    int32_t t;                                                          /* [ebp-0x20] */
    uint32_t rec;                                                       /* [ebp-0x14] */
    int8_t tok, tre;                                                    /* [ebp-0xC], [ebp-0x10] */
    int32_t maintain;                                                   /* [ebp-0x1C] */
    char src[32];                                                       /* [ebp-0x4C] */
    char buf[100];                                                      /* [ebp-0xB0] */

    t = tp1 - 1;                                                        /* 0x61091 */
    rec = NATION(n);                                                    /* 0x610A2 n = [ebp+8] */
    if (useToken != 0) {                                                /* 0x610A5 */
        tok = 1;                                                        /* 0x610AB */
        tre = 0;
    } else {
        tok = 0;                                                        /* 0x610B8 */
        tre = 1;
    }
    maintain = scrap == 0;                                              /* 0x610BF sete */
    snprintf(src, sizeof src, "%s", tre != 0 ? "Treasury" : "Levy");    /* 0x610CE..0x610FC (0xB61A7 / 0xB61B0) */
    if (maintain != 0)                                                  /* 0x61109 */
        snprintf(buf, sizeof buf, "%s maintains a ship in %s using %s.", NAME(n), TNAME(t), src);   /* 0x6112D (0xB61B5) */
    else
        snprintf(buf, sizeof buf, "%s doesn't maintain a ship in %s.", NAME(n), TNAME(t));           /* 0x61151 (0xB61D9) */
    f79270(-550, n, t, buf, 0);                                         /* 0x6116C */
    if (maintain == 0)                                                  /* 0x61171 */
        f78A20(500);                                                    /* 0x6117C */
    if (maintain != 0) {                                                /* 0x61181 */
        setD8(rec + 0x3Eu, (uint8_t)(D8(rec + 0x3Eu) - (uint8_t)tre)); /* 0x6118D sub byte [rec + 0x3E], al */
        setD8(rec + 0x99u, (uint8_t)(D8(rec + 0x99u) + (uint8_t)tre)); /* 0x61196 */
        setD8(rec + 0x99u, (uint8_t)(D8(rec + 0x99u) + (uint8_t)tok)); /* 0x611A2 */
        setD8(rec + 0x98u, (uint8_t)(D8(rec + 0x98u) - (uint8_t)tok)); /* 0x611AE */
        f57CE8(tp1 - 1, n, 0, tok);                                     /* 0x611C1 movsx ecx, byte tok */
    } else
        f78D98(k, n);                                                   /* 0x611CE */
    if ((D8(0xF097Fu) & 0x40u) == 0) {                                  /* 0x611D3 */
        f3CBBC();                                                       /* 0x611DC */
        if (f7BA64() == 0)                                              /* 0x611E1 */
            f79270(1000, n, t, buf, 0);                                 /* 0x6121F (eax pushed: 0) */
        else
            f79270(D16(0xF40BAu) != 0 ? 1000 : 500, n, t, buf, 0);      /* 0x61205 word [0xF40BA]; 0x6121F */
    } else {
        f79270(-500, n, t, buf, 0);                                     /* 0x61239 */
        f3A990();                                                       /* 0x6123E */
        f3CBBC();                                                       /* 0x61243 */
    }
}

/* ---- 0x61940: SHIP CONSTRUCTION ("DoShipContruction") --------------------------------------------------------- */

/* the heap block behind handle field a (NULL if none: port guard; the original writes through the pointer) */
static uint8_t *temp_block(uint32_t field, uint32_t size)
{
    return D32(field) != 0 ? load_heap_ptr(D32(field), size) : NULL;
}

void f61940(void)
{
    int32_t nobody;                                                     /* [ebp-0x2C] */
    int32_t i;                                                          /* [ebp-0x54] */
    int32_t n;                                                          /* [ebp-0x50] */
    int32_t nm1;                                                        /* [ebp-0x28] */
    uint32_t rec;                                                       /* [ebp-0x20] */
    int32_t j;                                                          /* [ebp-0x4C] */
    int32_t sel;                                                        /* [ebp-0x38] */
    int32_t redraw;                                                     /* [ebp-0x34] */
    int32_t code;                                                       /* [ebp-0x58] */
    int32_t r;                                                          /* [ebp-0x48] */
    int32_t cnt;                                                        /* [ebp-0x24] */
    int32_t s, tmp;                                                     /* [ebp-0x18], [ebp-0x3C] */
    int32_t tp1, t, c;                                                  /* [ebp-0x40], [ebp-0x44] */
    int32_t cand[160];                                                  /* [ebp-0x33C] */
    char buf[100];                                                      /* [ebp-0xBC] */
    uint8_t *blk;

    memset(cand, 0, sizeof cand);   /* port: the original's stack garbage (BUG(1995) C) cannot be reproduced */
    setD32(TEMP_TERR, load_heap_alloc(AREA_SIZE));                      /* 0x61967 malloc(0x2B65) "TempTerr" */
    /* 0x61976 f78B24(0x2B65): the alloc trace, no-op */
    setD32(TEMP_NATION, load_heap_alloc(NAT_SIZE));                     /* 0x6198A malloc(199) "Tempnation" */
    /* 0x61999 f78B24(199) */
    if (S8(COMPUTERS) > 0)                                              /* 0x6199E */
        f7A598();                                                       /* 0x619AA */
    message_area();                                                     /* 0x619AF..0x619DA */
    setD8(SEA_MODE, 2);                                                 /* 0x619E1 */
    setD32(0x10F2C0u, 0);                                               /* 0x619EE */
    setD32(0x10F2C4u, 0);                                               /* 0x619F4 */
    background3();                                                      /* 0x619FA..0x61A2E */
    nobody = 1;                                                         /* 0x61A36 */
    f7A8D0();                                                           /* 0x61A39 */
    for (i = 0; (int32_t)S16(ORDER_COUNT) > i; i++) {                   /* 0x62149; jle 0x62405 */
        n = (int32_t)S16(ORDER + 2u * (uint32_t)i);                     /* 0x6215D */
        setD32(CUR_NATION, (uint32_t)n);                                /* 0x6216D */
        nm1 = n - 1;                                                    /* 0x62176 */
        rec = NATION(n);                                                /* 0x62187 */
        blk = temp_block(TEMP_TERR, AREA_SIZE);
        for (j = 0; j < 0xA1; j++)                                      /* 0x6218C, 0x61A4E cmp 0xA1; jge */
            if (blk != NULL)
                memcpy(blk + 69u * (uint32_t)j, DP(AREA0 + 69u * (uint32_t)j), 69);   /* 0x61A88 rep movsd x17, movsb */
        blk = temp_block(TEMP_NATION, NAT_SIZE);
        if (blk != NULL)
            memcpy(blk, DP(rec), NAT_SIZE);                             /* 0x61AA5 rep movsd x49, movsw, movsb */
        /* 0x61AAD dword [rec + 0x95] sar 24: int8 +0x98, the tokens on the board (not the stock +0x99); 0x61ABE
           dword [rec + 0x4E] sar 24: +0x51 cities */
        if (!(S8(rec + 0x98u) > 2 || S8(rec + 0x51u) > 0))
            goto housekeeping_;
        f62658(n);                                                      /* 0x61ACF */
        if (D32(SITE_COUNT) == 0)                                       /* 0x61AD4 */
            goto housekeeping_;
        if ((int32_t)S8(rec + 0x9Au) != 3 || D8(rec + 0x40u) >= 4)      /* 0x61AE2..0x61B05 */
            goto computer;
        if ((D8(FLAGS) & PBEM_BIT) != 0 && (int32_t)S8(PBEM_LOCAL) != n)   /* 0x61B0B..0x61B2A (PBEM) */
            goto housekeeping_;

        /* 0x61B30: the human's screen */
        nobody = 0;                                                     /* 0x61B32 */
        background3();                                                  /* 0x61B35..0x61B66 */
        fg_vbopen(S32(0xF52C8u));                                       /* 0x61B75 */
        f3AF8C(0xF51ACu, 0xFB, 0x181);                                  /* 0x61B8C the left arrow */
        f3AF8C(0xF51B4u, 0x178, 0x181);                                 /* 0x61BA0 the right arrow */
        fg_vbclose();                                                   /* 0x61BA5 */
        fg_setcolor(5);                                                 /* 0x61BAC */
        f7876C();                                                       /* 0x61BB4 */
        f7853C(DLG_BUILD);                                              /* 0x61BBE */
        f3B05C(PANEL);                                                  /* 0x61BC8 */
        fg_setcolor(5);                                                 /* 0x61BCF */
        f3A924();                                                       /* 0x61BD7 */
        f3A9F4("CANDIDATE SITES FOR SHIP CONSTRUCTION");                /* 0x61BE1 (0xB6274) */
        sel = 0;                                                        /* 0x61BE6 */
        redraw = 1;                                                     /* 0x61BE9 */
        f7844C(PANEL, S32(PLAYER_BASE));                                /* 0x61BFB */
        for (;;) {                                                      /* 0x61C00 */
            if (D8(rec + 0x40u) >= 4 || S32(SITE_COUNT) <= 0)           /* 0x61C00..0x61C1B */
                break;
            fg_vbopen(S32(0xF52C8u));                                   /* 0x61C28 */
            f3C9D0(0x109, 0x181, 0x6E, 0x13, 5);                        /* 0x61C46 the site's box */
            fg_setcolor(5);                                             /* 0x61C4D */
            f3A0B0(0x109, 0x185, 0x6E, TNAME(S32(SITES + 4u * (uint32_t)sel)));   /* 0x61C75 its name */
            fg_vbclose();                                               /* 0x61C7A */
            f3C3D0(PANEL, 0x36);                                        /* 0x61C89 FROM TREASURY */
            f3C3D0(PANEL, 0x37);                                        /* 0x61C98 BY LEVY */
            f3C3D0(PANEL, 0x38);                                        /* 0x61CA7 BY BOTH */
            c = CANON(S32(SITES + 4u * (uint32_t)sel));                 /* 0x61CAF */
            if (S8(AREA(c) + 0x0Eu + (uint32_t)n) >= 2)                 /* 0x61CD3 dword [.. + 0xE31C3] sar 24; cmp 2; jl */
                f3C314(PANEL, 0x37);                                    /* 0x61CEB */
            if (S8(rec + 0x3Eu) >= 2)                                   /* 0x61CF0 */
                f3C314(PANEL, 0x36);                                    /* 0x61D08 */
            if (S8(rec + 0x3Eu) >= 1) {                                 /* 0x61D0D */
                c = CANON(S32(SITES + 4u * (uint32_t)sel));             /* 0x61D1E */
                if (S8(AREA(c) + 0x0Eu + (uint32_t)n) >= 1)             /* 0x61D42 */
                    f3C314(PANEL, 0x38);                                /* 0x61D5A */
            }
            f3F670();                                                   /* 0x61D5F */
            f3F624(S32(SITES + 4u * (uint32_t)sel), 0x0F);              /* 0x61D73 */
            if (redraw != 0) {                                          /* 0x61D78 */
                f459E4(n);                                              /* 0x61D81 */
                f3DA60(S32(SITES + 4u * (uint32_t)sel));                /* 0x61D92 */
                redraw = 0;                                             /* 0x61D97 */
            }
            f3CBBC();                                                   /* 0x61D9A */
            code = f78BA4(PANEL, 1);                                    /* 0x61DA9 map clicks on: a site's index */
            if (code == -100)                                           /* 0x61DB1 */
                continue;
            if (code >= 0) {                                            /* 0x61DBE */
                sel = code;                                             /* 0x61DC5 */
                redraw = 1;                                             /* 0x61DC8 */
                continue;
            }
            if (code == -1)                                             /* 0x61DD7 FINISHED */
                break;
            /* 0x61DE0..0x61E02, 0x62048, 0x62056 (unsigned compares): -9, -10, -5, -6, -7, -99; others loop */
            if (code == -9) {                                           /* 0x61E07 the left arrow */
                if (sel > 0) {                                          /* 0x61E07 cmp 0; jle */
                    sel--;                                              /* 0x61E14 */
                    redraw = 1;                                         /* 0x61E17 */
                }
            } else if (code == -10) {                                   /* 0x61E23 the right arrow */
                if (S32(SITE_COUNT) - 1 > sel) {                        /* 0x61E28 dec eax; cmp; jle */
                    sel++;                                              /* 0x61E35 */
                    redraw = 1;                                         /* 0x61E38 */
                }
            } else if (code == -5) {                                    /* 0x61E44 FROM TREASURY: 2 from the treasury */
                setD8(rec + 0x3Eu, (uint8_t)(D8(rec + 0x3Eu) - 2u));   /* 0x61E47 */
                setD8(rec + 0x99u, (uint8_t)(D8(rec + 0x99u) + 2u));   /* 0x61E52 add byte [.. + 0xF1565], 2 */
                f62774(S32(SITES + 4u * (uint32_t)sel), n, &sel);       /* 0x61E6E */
                redraw = 1;                                             /* 0x61E73 */
            } else if (code == -6) {                                    /* 0x61E7B BY LEVY: 2 tokens there */
                c = CANON(S32(SITES + 4u * (uint32_t)sel));             /* 0x61E85 */
                setD8(AREA(c) + 0x0Eu + (uint32_t)n,
                      (uint8_t)(D8(AREA(c) + 0x0Eu + (uint32_t)n) - 2u));       /* 0x61EA2 sub byte [.. + 0xE31C6], 2 */
                setD8(rec + 0x98u, (uint8_t)(D8(rec + 0x98u) - 2u));   /* 0x61EB0 */
                setD8(rec + 0x99u, (uint8_t)(D8(rec + 0x99u) + 2u));   /* 0x61EBE */
                f581C4(CANON(S32(SITES + 4u * (uint32_t)sel)), n);      /* 0x61EDD */
                f62774(S32(SITES + 4u * (uint32_t)sel), n, &sel);       /* 0x61EF7 */
                redraw = 1;                                             /* 0x61EFC */
            } else if (code == -7) {                                    /* 0x61F04 BY BOTH: 1 + 1 */
                setD8(rec + 0x3Eu, (uint8_t)(D8(rec + 0x3Eu) - 1u));   /* 0x61F07 */
                c = CANON(S32(SITES + 4u * (uint32_t)sel));             /* 0x61F16 */
                setD8(AREA(c) + 0x0Eu + (uint32_t)n,
                      (uint8_t)(D8(AREA(c) + 0x0Eu + (uint32_t)n) - 1u));       /* 0x61F2C */
                setD8(rec + 0x98u, (uint8_t)(D8(rec + 0x98u) - 1u));   /* 0x61F39 */
                setD8(rec + 0x99u, (uint8_t)(D8(rec + 0x99u) + 2u));   /* 0x61F46 */
                f581C4(CANON(S32(SITES + 4u * (uint32_t)sel)), n);      /* 0x61F65 */
                f62774(S32(SITES + 4u * (uint32_t)sel), n, &sel);       /* 0x61F7F */
                redraw = 1;                                             /* 0x61F84 */
            } else if (code == -99) {                                   /* 0x61F8C RESTART */
                snprintf(buf, sizeof buf, "Restart %s Ship Construction?", ADJ(n));   /* 0x61FA3 (0xB629A) */
                if ((D8(0xF097Cu) & 0x08u) != 0 && f795A8(buf) != -0x61)   /* 0x61FB1; 0x61FBC; cmp -0x61 (YES) */
                    continue;
                blk = temp_block(TEMP_TERR, AREA_SIZE);
                for (j = 0; j < 0xA1; j++)                              /* 0x61FCA; 0x61FD7 cmp 0xA1 */
                    if (blk != NULL)
                        memcpy(DP(AREA0 + 69u * (uint32_t)j), blk + 69u * (uint32_t)j, 69);   /* 0x6200D */
                blk = temp_block(TEMP_NATION, NAT_SIZE);
                if (blk != NULL)
                    memcpy(DP(rec), blk, NAT_SIZE);                     /* 0x6202A */
                f62658(n);                                              /* 0x62034 */
                sel = 0;                                                /* 0x62039 ecx = 0 */
                redraw = 1;                                             /* 0x6203C */
            }
        }
        f78664(DLG_BUILD);                                              /* 0x62073..0x62078 */
        goto housekeeping_;

    computer:                                                           /* 0x62194 */
        r = CALL_F62840(n);                                             /* 0x62197 f62840(n) [SHIPAI] */
        if (r >= 0) {                                                   /* 0x621A2 */
            nobody = 0;                                                 /* 0x621AC */
            /* 0x621BA: int8 [0x10F6BA + 18r] (dword [18r + 0x10F6B7] sar 24) ships */
            for (j = 0; (int32_t)S8(0x10F6BAu + 18u * (uint32_t)r) > j; j++)
                f62488((int32_t)D8(0x10F6BBu + 18u * (uint32_t)r + (uint32_t)j),     /* 0x6221D movzx: the site */
                       (int32_t)S8(0x10F6C3u + 18u * (uint32_t)r + (uint32_t)j),     /* 0x62203 sar 24: the levy */
                       (int32_t)S8(0x10F6BFu + 18u * (uint32_t)r + (uint32_t)j),     /* 0x621E9 sar 24: treasury */
                       n);                                              /* 0x6222A */
        }
        /* 0x62231: the extra ships from the treasury */
        if (S8(rec + 0x99u) >= 4 || S8(rec + 0x51u) <= 0 || S32(0xF5470u) < 3 || D8(rec + 0x40u) >= 4)   /* 0x62231..
                                                                                                         0x62272 */
            goto housekeeping_;
        for (;;) {                                                      /* 0x62278 */
            if (S8(rec + 0x99u) >= 4 || D8(rec + 0x40u) >= 4 || S8(rec + 0x3Eu) < 2)   /* 0x62278..0x622AD */
                break;
            cnt = 0;                                                    /* 0x622B5 */
            for (j = 0; (int32_t)S8(rec + 0x60u) > j; j++) {            /* 0x622C3 dword [rec + 0x5D] sar 24 */
                tp1 = (int32_t)D8(rec + 0x61u + (uint32_t)j);           /* 0x622D9 */
                t = tp1 - 1;                                            /* 0x622E3 */
                if ((TERRFLAG(t) & 0x02u) == 0)                         /* 0x622F7 byte [35t + 0xE1638]; test 2 */
                    continue;
                /* 0x62314 dword [69 tp1 + n - 1 + 0xE31D1] sar 24: area tp1 +0x1B + n, n's ships */
                if (S8(0xE31D4u + 69u * (uint32_t)tp1 + (uint32_t)nm1) != 0)
                    continue;
                cand[cnt++] = t;                                        /* 0x6232A */
            }
            if (cnt == 0 && !g_original)
                break;      /* BUG(1995) C fixed: no candidate; the original builds at cand[0] (file comment) */
            /* 0x62333..0x623E6: bubble sort, the most tokens of n first (signed byte compare, a swap only when
               greater) */
            for (j = cnt - 2; j >= 0; j--)                              /* 0x62339; 0x6234D dec; jl 0x623EB */
                for (s = 0; s <= j; s++) {                              /* 0x6236A cmp s, j; jg */
                    int32_t a1, a0;
                    if (g_original) {
                        /* BUG(1995) B: area record cand, territory cand - 1 (0x623A8 [69 cand + n - 1 + 0xE31C7]) */
                        a1 = (int32_t)S8(0xE31C7u + 69u * (uint32_t)cand[s + 1] + (uint32_t)nm1);
                        a0 = (int32_t)S8(0xE31C7u + 69u * (uint32_t)cand[s] + (uint32_t)nm1);
                    } else {
                        a1 = (int32_t)S8(AREA(cand[s + 1]) + 0x0Eu + (uint32_t)n);
                        a0 = (int32_t)S8(AREA(cand[s]) + 0x0Eu + (uint32_t)n);
                    }
                    if (a1 <= a0)                                       /* 0x623B4 cmp al, bh; jle */
                        continue;
                    tmp = cand[s + 1];                                  /* 0x623BB */
                    cand[s + 1] = cand[s];                              /* 0x623D2 */
                    cand[s] = tmp;                                      /* 0x623DF */
                }
            f62488(cand[0], 0, 2, n);                                   /* 0x623FB */
        }

    housekeeping_:                                                      /* 0x6207D */
        housekeeping(n, "Construction housekeeping for %s");           /* (0xB62B8) */
    }                                                                   /* 0x62146 i++ */
    setD8(SEA_MODE, 1);                                                 /* 0x6240C */
    f7ABD0();                                                           /* 0x62412 */
    load_heap_free(D32(TEMP_TERR));                                     /* 0x6241E free */
    setD32(TEMP_TERR, 0);                                               /* 0x62423 */
    /* 0x62432 f78B24(0) */
    load_heap_free(D32(TEMP_NATION));                                   /* 0x62446 */
    setD32(TEMP_NATION, 0);                                             /* 0x6244D */
    /* 0x62453 f78B24(0) */
    if (nobody != 0)                                                    /* 0x62458 */
        f79270(250, (int32_t)S16(ORDER), -200, "", 0);                  /* 0x62477 (0xB6311, empty) */
}

/* ---- 0x62488: a computer builds a ship ----------------------------------------------------------------------- */

void f62488(int t, int levy, int treasury, int n)
{
    uint32_t rec;                                                       /* [ebp-0xC] */
    int32_t k;                                                          /* [ebp-0x10] */
    const char *how;
    char src[32];                                                       /* [ebp-0x40] */
    char buf[100];                                                      /* [ebp-0xA4] */

    rec = NATION(n);                                                    /* 0x624B7 */
    k = (int32_t)D8(rec + 0x40u);                                       /* 0x624BF */
    if (treasury != 0 && levy != 0)                                     /* 0x624C5, 0x624CB */
        how = "Treasury and Levy";                                      /* 0xB6312 */
    else if (treasury != 0)                                             /* 0x624DB */
        how = "Treasury";                                               /* 0xB6324 */
    else
        how = "Levy";                                                   /* 0xB632D */
    snprintf(src, sizeof src, "%s", how);                               /* 0x624EB..0x62507 */
    snprintf(buf, sizeof buf, "%s constructed a ship in %s using %s.", NAME(n), TNAME(t), src);   /* 0x6252E (0xB6332) */
    f79270(-550, n, t, buf, 0);                                         /* 0x62549 */
    f78A20(500);                                                        /* 0x62553 */
    setD16(rec + 0x45u + 2u * (uint32_t)k, (uint16_t)(t + 1));          /* 0x62564 */
    setD8(rec + 0x40u, (uint8_t)(D8(rec + 0x40u) + 1u));                /* 0x6256B */
    setD8(rec + 0x3Eu, (uint8_t)(D8(rec + 0x3Eu) - (uint8_t)treasury)); /* 0x62574 sub byte, al */
    setD8(rec + 0x99u, (uint8_t)(D8(rec + 0x99u) + (uint8_t)treasury)); /* 0x6257D */
    setD8(rec + 0x99u, (uint8_t)(D8(rec + 0x99u) + (uint8_t)levy));     /* 0x62589 */
    setD8(rec + 0x98u, (uint8_t)(D8(rec + 0x98u) - (uint8_t)levy));     /* 0x62595 */
    if (levy > 0)                                                       /* 0x6259B */
        f57CE8(t, n, 0, levy);                                          /* 0x625AC */
    setD8(AREA(t) + 0x1Bu + (uint32_t)n, (uint8_t)(D8(AREA(t) + 0x1Bu + (uint32_t)n) + 1u));   /* 0x625C4 inc byte
                                                                           [.. + 0xE31D3] */
    setD8(AREA(t) + 0x2Du + (uint32_t)n, (uint8_t)k);                   /* 0x625E2 [.. + 0xE31E5] = k */
    if ((D8(0xF097Fu) & 0x80u) == 0) {                                  /* 0x625E8 */
        f3CBBC();                                                       /* 0x625F1 */
        if (f7BA64() == 0)                                              /* 0x625F6 */
            f79270(1000, n, t, buf, 0);                                 /* 0x62649 (eax pushed: 0) */
        else
            f79270(D16(0xF40BAu) != 0 ? 1000 : 500, n, t, buf, 0);      /* 0x6261A; 0x62649 */
    } else
        f79270(-500, n, t, buf, 0);                                     /* 0x62644; 0x62649 */
}

/* ---- 0x62658: the candidate sites ---------------------------------------------------------------------------- */

void f62658(int n)
{
    uint32_t rec;                                                       /* [ebp-0x4] */
    int32_t i;                                                          /* [ebp-0xC] */
    int32_t t;                                                          /* [ebp-0x8] */
    int32_t tok;

    rec = NATION(n);                                                    /* 0x6267B */
    setD32(SITE_COUNT, 0);                                              /* 0x62680 */
    for (i = 0; i < 0x9D; i++)                                          /* 0x62691 cmp 0x9D; jge */
        setD32(SITES + 4u * (uint32_t)i, 0xFFFFFFFFu);                  /* 0x6269D */
    if (!(S8(rec + 0x98u) > 2 || S8(rec + 0x51u) > 0))                  /* 0x626AD dword [rec + 0x95] sar 24: board
                                                                           +0x98; 0x626BE [rec + 0x4E]: cities +0x51 */
        return;
    for (t = 0; t < 0xA0; t++) {                                        /* 0x626D9 cmp 0xA0; jge */
        if ((TERRFLAG(t) & 0x02u) == 0)                                 /* 0x626F7 test dl, 2 */
            continue;
        tok = (int32_t)S8(AREA(CANON(t)) + 0x0Eu + (uint32_t)n);        /* 0x6271D dword [n + .. + 0xE31C3] sar 24 */
        if (tok < 2) {                                                  /* 0x62727 cmp 2; jge 0x6274B */
            if (S8(rec + 0x3Eu) < 1)                                    /* 0x62732 */
                continue;
            if (tok < 1)                                                /* 0x6273D (the same byte, read again) */
                continue;
        }
        setD32(SITES + 4u * D32(SITE_COUNT), (uint32_t)t);              /* 0x6274B..0x62760 */
        setD32(SITE_COUNT, D32(SITE_COUNT) + 1u);                       /* 0x62758 */
    }
}

/* ---- 0x62774: a human's new ship ----------------------------------------------------------------------------- */

void f62774(int t, int n, int32_t *sel)
{
    uint32_t rec;                                                       /* [ebp-0x8] */
    uint8_t k;                                                          /* [ebp-0x4] */

    rec = NATION(n);                                                    /* 0x6279B */
    setD8(AREA(t) + 0x1Bu + (uint32_t)n, (uint8_t)(D8(AREA(t) + 0x1Bu + (uint32_t)n) + 1u));   /* 0x627B1 */
    k = 0;                                                              /* 0x627B9 */
    while (S16(rec + 0x45u + 2u * (uint32_t)k) != 0)                    /* 0x627C6 dword [rec + 2k + 0x43] sar 16 */
        k++;                                                            /* 0x627D3 */
    setD16(rec + 0x45u + 2u * (uint32_t)k, (uint16_t)(t + 1));          /* 0x627E4 */
    setD8(AREA(t) + 0x2Du + (uint32_t)n, k);                            /* 0x62800 */
    setD8(rec + 0x40u, (uint8_t)(D8(rec + 0x40u) + 1u));                /* 0x62809 */
    f3DA60(t);                                                          /* 0x6280F */
    f459E4(n);                                                          /* 0x62817 */
    f3CBBC();                                                           /* 0x6281C */
    f3D52C(0, 0);                                                       /* 0x62825 */
    f62658(n);                                                          /* 0x6282D */
    f79A9C(sel);                                                        /* 0x62835 */
}

/* ---- 0x7A598 / 0x7A610: the world snapshot ------------------------------------------------------------------- */

void f7A598(void)
{
    memcpy(DP(0x1139B0u), DP(0xE31FDu), 0x2B20);                        /* 0x7A5A8..0x7A5C4 areas 1..160 */
    memcpy(DP(0x1137D0u), DP(0xE2C18u), 0x1E0);                         /* 0x7A5C7..0x7A5E3 territory state */
    memcpy(DP(0x112CECu), DP(0xF1593u), 0x88D);                         /* 0x7A5E6..0x7A602 nations 1..11 */
}

void f7A610(void)
{
    memcpy(DP(0xE31FDu), DP(0x1139B0u), 0x2B20);                        /* 0x7A620..0x7A63C */
    memcpy(DP(0xE2C18u), DP(0x1137D0u), 0x1E0);                         /* 0x7A63F..0x7A65B */
    memcpy(DP(0xF1593u), DP(0x112CECu), 0x88D);                         /* 0x7A65E..0x7A67A */
}
