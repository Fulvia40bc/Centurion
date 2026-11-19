/* Module TAX of docs/M6_PLAN.md: the TAXATION phase 0x66050 (1,080 bytes), the human's Coinage rate screen 0x66488
   (808), the tax revolt 0x6685C (760) and the human's city removal 0x66D8C (1,564, ChooseMultipleCitiesToRemove).
   Literal translations; the address of each instruction a line comes from is in its comment. D = read in the code,
   G = inference. Prototypes and globals: tax.h.

   Not ported here (each call site stops the port with not_ported(address); the static stubs below carry the names,
   so the call sites read as in the original and a later translation that exports one of these names collides with
   its stub at compile time instead of being shadowed by it):
     0x667B0  the computer's tax rate (M7, AI; the only caller is 0x661DA)
     0x66B54  a computer annexes revolting cities (M7, AI; called at 0x66A74 for computers and PBEM humans)
     0x66C2C  a computer removes its own cities nobody could take (M7, AI; called at 0x66AFA)
     0x2468C  ChooseCitiesToLoseToPirates in mode 2: a human picks the cities it receives (CAL1; called at 0x66A64)
     0x28CFC  the choice list 0xF7CA0 / [0xF7C9C] of kind 2, the nation's cities (CAL1; 0x66FA2, 0x672E0, 0x67357)
   So today a TAXATION in which a computer owns Coinage stops at 0x667B0, and a tax revolt stops at its first absorber
   (0x2468C / 0x66B54) or, with no absorber, at 0x28CFC (human) / 0x66C2C (computer). Built with -DTAX_TEST
   (port/test/tax_test.c) the five go to the test's recorders instead, which return.

   Play by e-mail (dropped by Erik's ruling): no PBEM routine is called here. The bit-0x80 tests of byte [0xE1624]
   and the local-player byte [0xE1622] (int8, read as dword [0xE161F] sar 24) are kept literally; they only choose
   between the human and computer paths.

   Watcom runtime -> standard C:
     0x7DB3D sprintf -> snprintf bounded to the stack buffer: 100 bytes for 0x66050 ([ebp-0xA4]..[ebp-0x40]) and
             0x6685C ([ebp-0xCC]..[ebp-0x68]), 152 for 0x66488 ([ebp-0xC0]..[ebp-0x28]), 200 for 0x66D8C
             ([ebp-0xF8]..[ebp-0x30])
     0x8C897 itoa(v, buf, 10) -> snprintf "%d"
     0x822B5 malloc -> load_heap_alloc (port handles in the 32-bit fields, game.h); 0x877E6 free -> load_heap_free
     rep movsd / movsw / movsb record copies -> memcpy between g_ds and the block (load_heap_ptr)
     0x8A080 clock: see the final wait of 0x66050 below
   __CHK dropped; 0x78B0C (debug trace, "ChooseMultipleCitiesToRemove") is a no-op and dropped.

   The final wait of 0x66050 (0x6642D..0x66457, port design, G): the original takes start = clock() before the
   nations' loop (0x661B7) and at the end busy-waits until clock() >= start + (500 / [0xBB3F4]) / 10 hundredths. The
   port's clock (ui_kit2.c, static there) advances only inside f78A20, so the port waits f78A20(500) when no path that
   takes at least 500 ms ran in between (a computer's rate message 2000 ms, a human's rate screen, a Democracy message
   3000 ms, a revolt with its CONTINUE box), and not at all otherwise. Only a human who closes the rate screen within
   half a second waits less than in the original (screen pacing only; no game data depend on it).

   Data (D, read in the code at the addresses given):
     int16 0xBB3FA / 0xBB3FC / 0xBB3FE / 0xBB400 the message area (x, y, w, h) = 0xA9, 0x154, 0x12C, 0x23 (0x66083)
     int16 [0xF06AE] the number of players, int16 0xF06B0[] their nations (read as dword [0xF06AE] sar 16 for the first)
     int32 0x11017C[n] the revolt flags; int32 [0xF40F8] the current nation; int32 [0xFFD40] the help topic
     int32 [0xF5940] the first player button's record index in 0xF596C (f783B8 / f7844C)
     [0xF52C8] the back buffer, [0xF52CC] the background buffer; int8 [0xEE168] the background number (3)
     uint8 [0xE1624] bit 0x80 PBEM; int8 [0xE1622] the local PBEM player; uint8 [0xF097C] bit 0x08 restart confirmations
     dialogs: 0xF596C the map screen's panel, 0xC817C the rate arrows (codes -11 / -12), 0xC8568 the removal screen
       (arrows 0x39 / 0x3A -> -9 / -10, RESTART 0x41 -> -0x63, REMOVE 0x3F -> -0x65, FINISHED 0x2A -> -1)
     sprites 0xF51AC / 0xF51B4 (the two arrow pictures, G); names 0xC768C[n], adjectives 0xC76C4[n], territory names
       0xC77D8[t] (0xC77D4 = entry -1 is "PIRACY", the last calamity name)
     the choice list int32 0xF7CA0[] / int32 [0xF7C9C] (built by 0x28CFC: count 0, entries 0..156 = -1, then for
       kind 2 (0x28FD8) each territory 0..156 in ascending order with a city (state +2 bit 0x01) of owner +0 == n; D)
     heap fields [0xEED74] (0x2B65 bytes: the area records 0..160), [0xEED7C] (0x1E0: the territory states 0..159),
       [0xEED78] (0x954: nations 1..11 at offset 199n); port handles, freed and zeroed at the end of 0x66D8C

   1995 bugs, fixed unless g_original (each marked BUG(1995) in the code; grades D code / G effect):
     1. Text: "Democrary spares %s from a Tax Revolt." (0xB8904) -> "Democracy" (Erik's ruling of 19 Nov: misspellings).
     2. 0x66408: the revolt calls f7844C (the player-button highlight of [0xF40F8]) without setting [0xF40F8] = n:
        it still holds 9, the last value of the tax loop, so the buttons show nation 9 (or nobody) as the current
        player during n's revolt. Fixed: [0xF40F8] = n first (as 0x66488 does before its f7844C, 0x664C7). Side
        effects of the fix: the map's current-player stack (f3DAD8, through f3D52C) is n's during the revolt, and
        [0xF40F8] is n, not 9, after it; [0xF40F8] is not in the save (savewrite.c), so the autosave is unaffected.
     3. 0x66FF8 / 0x67076: when the human has removed its last city (k equal to its cities, nobody to absorb them),
        the list is empty and entry 0xF7CA0[0] is 0x28CFC's filler -1: the box shows territory -1's name, which is
        "PIRACY" (0xC77D4), and f3F624(-1, 15) writes 15 into byte 0xFFB5F, the top byte of the heap pointer
        [0xFFB5C] (seaadj.dat's block; in the port a handle, which that write invalidates). Fixed: with an empty list
        the box stays empty and no highlight is set.
   Kept as the original (G unless marked):
     - 0x663F8 divides stock by the rate: rate 0 would fault, but a nation with rate 0 owes 0 and cannot revolt
       unless its stock is negative (D code, G unreachable).
     - 0x6685C's absorbers are ranked by 5 * (9 - cities) + stock, not by any rule of the board game (design).
     - 0x66D8C's first shift of the list after REMOVE (0x672FA..0x67331) is overwritten at once by 0x28CFC's rebuild.
   Easter eggs / oddities: the debug name "ChooseMultipleCitiesToRemove" (0xB8A34) passed to the no-op logger
   0x78B0C; the empty "nothing to tax" message 0xB892B (a 100 ms home view of the first player); the tax revolt
   borrows the Pirates' city chooser 0x2468C in mode 2 (M6_PLAN egg 9). */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "fg.h"
#include "game.h"
#include "kit.h"
#include "map.h"
#include "mapdata.h"
#include "panel.h"
#include "rules.h"
#include "start.h"
#include "state.h"
#include "tax.h"
#include "ui.h"

#define NATION(n)   (0xF14CCu + 199u * (uint32_t)(n))
#define NAME(n)     DSTR(D32(0xC768Cu + 4u * (uint32_t)(n)))
#define ADJ(n)      DSTR(D32(0xC76C4u + 4u * (uint32_t)(n)))
#define TERR_NAME(t) DSTR(D32(0xC77D8u + 4u * (uint32_t)(t)))
#define REVOLT(n)   (0x11017Cu + 4u * (uint32_t)(n))
#define FLAGS       0xE1624u
#define PBEM_BIT    0x80u
#define LOCAL_PBEM  0xE1622u   /* int8: dword [0xE161F] sar 24 */
#define CUR_NATION  0xF40F8u
#define HELP_TOPIC  0xFFD40u
#define PANEL_DLG   0xF596Cu
#define PLAYER_BASE 0xF5940u
#define PLAYERS     0xF06AEu   /* int16 count */
#define PLAYER_LIST 0xF06B0u   /* int16[] */
#define BACKBUF     0xF52C8u
#define BACKGROUND  0xF52CCu
#define RATE_DLG    0xC817Cu
#define REMOVE_DLG  0xC8568u
#define CHOICE      0xF7CA0u   /* int32[] */
#define CHOICE_N    0xF7C9Cu   /* int32 */
#define SNAP_AREA   0xEED74u
#define SNAP_NATION 0xEED78u
#define SNAP_TSTATE 0xEED7Cu
#define AREA        0xE31B8u   /* 161 x 69 */
#define TSTATE      0xE2C18u   /* 160 x 3 */

/* ---- calls into code that is not ported yet (see the file comment) ----------------------------------------------- */

#ifdef TAX_TEST
int  tax_test_f667B0(int n);
void tax_test_f66B54(int m, int n, int k);
void tax_test_f66C2C(int n, int k);
void tax_test_f2468C(int c, int v, int k, int mode);
void tax_test_f28CFC(int n, int kind, int x);
#define f667B0(n)             tax_test_f667B0(n)
#define f66B54(m, n, k)       tax_test_f66B54(m, n, k)
#define f66C2C(n, k)          tax_test_f66C2C(n, k)
#define f2468C(c, v, k, mode) tax_test_f2468C(c, v, k, mode)
#define f28CFC(n, kind, x)    tax_test_f28CFC(n, kind, x)
#else
#include "formulas.h"                           /* f667B0, the computer's tax rate (Mistral, Task 15, verified) */

static void f66B54(int m, int n, int k)         /* M7 (AI): computer m annexes k revolting cities of n */
{
    (void)m; (void)n; (void)k;
    not_ported(0x66B54u);
}

static void f66C2C(int n, int k)                /* M7 (AI): computer n removes k of its own cities */
{
    (void)n; (void)k;
    not_ported(0x66C2Cu);
}

static void f2468C(int c, int v, int k, int mode)   /* CAL1: ChooseCitiesToLoseToPirates (human chooser c, victim v) */
{
    (void)c; (void)v; (void)k; (void)mode;
    not_ported(0x2468Cu);
}

static void f28CFC(int n, int kind, int x)      /* CAL1: the choice list 0xF7CA0 / [0xF7C9C] for n by kind */
{
    (void)n; (void)kind; (void)x;
    not_ported(0x28CFCu);
}
#endif

/* ---- 0x66050 TAXATION -------------------------------------------------------------------------------------------- */

void f66050(void)
{
    char     buf[100];     /* [ebp-0xA4] */
    int      any_revolt;   /* [ebp-0x30] */
    int      n;            /* [ebp-0x2C] in the first loop, [ebp-0x28] after */
    int      cities;       /* [ebp-0x3C] */
    int      nobody;       /* [ebp-0x40] 1 until the map screen is drawn */
    uint32_t rec;          /* [ebp-0x1C] */
    int      tax;          /* [ebp-0x20] cities, then cities * rate */
    int      old;          /* [ebp-0x34] */
    int      rate;         /* [ebp-0x38] */
    int      kept;         /* [ebp-0x24] the cities the stock still pays for */
    int      waited = 0;   /* port: a wait of 500 ms or more ran since 0x661B7 (the file comment) */

    any_revolt = 0;                                                   /* 0x66068 */
    setD16(0xBB3FAu, 0xA9);                                           /* 0x66083 the message area */
    setD16(0xBB3FCu, 0x154);                                          /* 0x6608A */
    setD16(0xBB3FEu, 0x12C);                                          /* 0x66091 */
    setD16(0xBB400u, 0x23);                                           /* 0x66098 */
    setD16(PLAYERS, 0);                                               /* 0x6609F */
    for (n = 1; n <= 9; n++) {                                        /* 0x660A6..0x660B9 */
        setD32(REVOLT(n), 0);                                         /* 0x660C0 */
        if (D8(NATION(n) + 0x9A) != 0) {                              /* 0x660CE byte [199n + 0xF1566] */
            int16_t p = S16(PLAYERS);                                 /* 0x660D8 movsx edx, ax */
            setD16(PLAYERS, (uint16_t)(p + 1));                       /* 0x660E2 */
            setD16(PLAYER_LIST + 2u * (uint32_t)(int32_t)p, (uint16_t)n);   /* 0x660EB */
        }
    }
    f783B8(PANEL_DLG, S32(PLAYER_BASE));                              /* 0x66102 */
    cities = 0;                                                       /* 0x66107 */
    nobody = 1;                                                       /* 0x6610F */
    for (n = 1; n <= 9; n++) {                                        /* 0x66112..0x66121 */
        cities = S8(NATION(n) + 0x51);                                /* 0x6612A dword [199n + 0xF151A] sar 24 */
        if (cities > 0)                                               /* 0x6613B jle: the first nation with cities */
            break;
    }
    if (cities > 0) {                                                 /* 0x6613D jle 0x66459 */
        f3CD6C(3);                                                    /* 0x6614C */
        setD8(0xEE168u, 3);                                           /* 0x66176 */
        fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BACKGROUND), S32(BACKBUF));   /* 0x6617C background -> back */
        f7876C();                                                     /* 0x66184 the phase name */
        f3B05C(PANEL_DLG);                                            /* 0x6618E */
        f459E4(S16(PLAYER_LIST));                                     /* 0x6619B dword [0xF06AE] sar 16 */
        nobody = 0;                                                   /* 0x661A2 */
        f3CBBC();                                                     /* 0x661A5 */
        f3D9FC(S16(PLAYER_LIST));                                     /* 0x661B2 */
        /* 0x661B7 start = clock(): see the final wait */
        for (n = 1; n <= 9; n++) {                                    /* 0x661BF..0x66298 */
            setD32(CUR_NATION, (uint32_t)n);                          /* 0x662A1 */
            rec = NATION(n);                                          /* 0x662B4 */
            if (D8(rec + 0x9A) == 0)                                  /* 0x662BA */
                continue;
            if (D8(rec + 0x51) == 0)                                  /* 0x662C7 */
                continue;
            tax = S8(rec + 0x51);                                     /* 0x662D0 dword [rec+0x4E] sar 24 */
            if (D8(rec + 0x2C) != 0) {                                /* 0x662DC Coinage */
                if ((S8(rec + 0x9A) == 3 && (D8(FLAGS) & PBEM_BIT) == 0)            /* 0x662F3, 0x662F8 */
                    || ((D8(FLAGS) & PBEM_BIT) != 0 && S8(LOCAL_PBEM) == n)) {      /* 0x66301, 0x66316 */
                    f66488(n);                                        /* 0x66322 */
                    waited = 1;
                } else {
                    old = S8(rec + 0x3F);                             /* 0x661CE dword [rec+0x3C] sar 24 */
                    rate = f667B0(n);                                 /* 0x661DA [M7] */
                    setD8(rec + 0x3F, (uint8_t)rate);                 /* 0x661E8 */
                    if (rate != old && (D8(FLAGS) & PBEM_BIT) == 0) { /* 0x661F3, 0x661F5 */
                        f3A990();                                     /* 0x661FE */
                        snprintf(buf, sizeof buf, "%s sets its Tax Rate to %d.",
                                 NAME(S32(CUR_NATION)), rate);        /* 0x66220 (0xB88E8) */
                        f3A9F4(buf);                                  /* 0x6622E */
                        f459E4(n);                                    /* 0x66236 */
                        f3CBBC();                                     /* 0x6623B */
                        f3D9FC(n);                                    /* 0x66243 */
                        f78A20(2000);                                 /* 0x6624D */
                        waited = 1;
                    }
                }
            }
            /* 0x66252 */
            tax = tax * S8(rec + 0x3F);                               /* 0x6625E imul (rate: dword [rec+0x3C] sar 24) */
            if (S8(rec + 0x99) < tax) {                               /* 0x66270 dword [rec+0x96] sar 24; jl */
                setD32(REVOLT(n), 1);                                 /* 0x66334 */
                any_revolt = 1;                                       /* 0x6633B */
            } else {
                setD8(rec + 0x3E, (uint8_t)(D8(rec + 0x3E) + (uint8_t)tax));   /* 0x6627F treasury */
                setD8(rec + 0x99, (uint8_t)(D8(rec + 0x99) - (uint8_t)tax));   /* 0x66288 stock */
            }
        }
        if (any_revolt != 0) {                                        /* 0x66343 */
            for (n = 1; n <= 9; n++) {                                /* 0x6634D..0x663AE */
                if (S32(REVOLT(n)) == 0)                              /* 0x663BE */
                    continue;
                if (D8(NATION(n) + 0x34) != 0) {                      /* 0x663C9 Democracy: byte [199n + 0xF1500] */
                    f3A924();                                         /* 0x66356 */
                    if (g_original)   /* BUG(1995): the misspelling "Democrary" (0xB8904) */
                        snprintf(buf, sizeof buf, "Democrary spares %s from a Tax Revolt.", NAME(n));   /* 0x66372 */
                    else
                        snprintf(buf, sizeof buf, "Democracy spares %s from a Tax Revolt.", NAME(n));
                    f3A9F4(buf);                                      /* 0x66380 */
                    f459E4(n);                                        /* 0x66388 */
                    f3CBBC();                                         /* 0x6638D */
                    f3D9FC(n);                                        /* 0x66395 */
                    f78A20(3000);                                     /* 0x6639F */
                    waited = 1;
                } else {
                    kept = S8(NATION(n) + 0x99) / S8(NATION(n) + 0x3F);   /* 0x663F8 idiv: [199n + 0xF1562] sar 24 /
                                                                         [199n + 0xF1508] sar 24 (stock / rate) */
                    if (!g_original)   /* BUG(1995): f7844C highlights [0xF40F8], still 9 from the tax loop */
                        setD32(CUR_NATION, (uint32_t)n);
                    f7844C(PANEL_DLG, S32(PLAYER_BASE));              /* 0x66408 */
                    f6685C(n, S8(NATION(n) + 0x51) - kept);           /* 0x66423 cities - kept */
                    waited = 1;
                }
            }
        }
        /* 0x6642D..0x66457: while (unsigned) start + (500 / [0xBB3F4]) / 10 > clock() (port: the file comment) */
        if (!waited)
            f78A20(500);
    }
    if (nobody != 0)                                                  /* 0x66459 */
        f79270(100, S16(PLAYER_LIST), -200, "", 0);                   /* 0x66479 ("" 0xB892B) */
}

/* ---- 0x66488 the human's tax rate (Coinage) ---------------------------------------------------------------------- */

void f66488(int n)
{
    char     buf[152];     /* [ebp-0xC0] */
    uint32_t rec;          /* [ebp-0x1C] */
    int      max;          /* [ebp-0x20] */
    int      v;            /* [ebp-0x24] */
    int      r;            /* [ebp-0x28] */

    setD32(HELP_TOPIC, (uint32_t)-50);                                /* 0x664A3 */
    rec = NATION(n);                                                  /* 0x664BB */
    setD32(CUR_NATION, (uint32_t)n);                                  /* 0x664C7 */
    f7844C(PANEL_DLG, S32(PLAYER_BASE));                              /* 0x664D1 */
    if ((D8(FLAGS) & PBEM_BIT) != 0 && S8(LOCAL_PBEM) != n)           /* 0x664DC..0x664F5 (the bit tested twice) */
        return;                                                       /* 0x667A4 */
    f3CD6C(3);                                                        /* 0x66500 */
    setD8(0xEE168u, 3);                                               /* 0x6652A */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BACKGROUND), S32(BACKBUF));   /* 0x66530 */
    f7876C();                                                         /* 0x66538 */
    max = S8(rec + 0x99) / S8(rec + 0x51);                            /* 0x66557 idiv: stock / cities (the caller
                                                                         guarantees cities != 0) */
    if (max > 3)                                                      /* 0x6655F jle */
        max = 3;                                                      /* 0x66564 */
    if (max == 0) {                                                   /* 0x6656B */
        f3A924();                                                     /* 0x66571 */
        snprintf(buf, sizeof buf, "%s, a tax revolt is unavoidable.\nThe tax rate has been set to 1 to minimize "
                 "the effect.", NAME(n));                             /* 0x6658D (0xB892C) */
        f3AB7C(buf);                                                  /* 0x6659B */
        f3F670();                                                     /* 0x665A0 */
        f459E4(n);                                                    /* 0x665A8 */
        f3D9FC(n);                                                    /* 0x665B0 */
        f3B05C(PANEL_DLG);                                            /* 0x665BA */
        f7844C(PANEL_DLG, S32(PLAYER_BASE));                          /* 0x665CA */
        f3CBBC();                                                     /* 0x665CF */
        setD8(rec + 0x3F, 1);                                         /* 0x665D7 */
        f78A20(4000);                                                 /* 0x665E0 */
        return;
    }
    f7853C(RATE_DLG);                                                 /* 0x665EF */
    fg_vbopen(S32(BACKBUF));                                          /* 0x665FB */
    f3AF8C(0xF51ACu, 0x128, 0x181);                                   /* 0x66612 */
    f3AF8C(0xF51B4u, 0x14A, 0x181);                                   /* 0x66626 */
    fg_setcolor(5);                                                   /* 0x6662D */
    f3A3A0(0x125, 0x185, DSTR(0xB8984u));                             /* 0x66644 "TAX" */
    f3A044(0x15A, 0x185, DSTR(0xB8988u));                             /* 0x66658 "RATE" */
    fg_vbclose();                                                     /* 0x6665D */
    snprintf(buf, sizeof buf, "%s, set your tax rate.", NAME(n));     /* 0x66679 (0xB898D) */
    f3A9F4(buf);                                                      /* 0x66687 */
    f3B05C(PANEL_DLG);                                                /* 0x66691 */
    v = S8(rec + 0x3F);                                               /* 0x6669C dword [rec+0x3C] sar 24 */
    if (v > max)                                                      /* 0x666A8 jle */
        v = max;                                                      /* 0x666AD */
    f3F670();                                                         /* 0x666B0 */
    f459E4(n);                                                        /* 0x666B8 */
    f3D9FC(n);                                                        /* 0x666C0 */
    for (;;) {
        fg_vbopen(S32(BACKBUF));                                      /* 0x666CC */
        f3C9D0(0x136, 0x181, 0x13, 0x13, 5);                          /* 0x666E7 */
        fg_setcolor(5);                                               /* 0x666EE */
        snprintf(buf, sizeof buf, "%d", v);                           /* 0x6670A itoa(v, buf, 10) */
        f3A0B0(0x136, 0x185, 0x13, buf);                              /* 0x6671E */
        fg_vbclose();                                                 /* 0x66723 */
        f3CBBC();                                                     /* 0x66728 */
        r = f78BA4(PANEL_DLG, 0);                                     /* 0x66734 */
        if (r == -100)                                                /* 0x6673F */
            continue;
        if (r == -1) {                                                /* 0x66747 */
            setD8(rec + 0x3F, (uint8_t)v);                            /* 0x66752 */
            break;
        }
        if ((uint32_t)r < (uint32_t)-12)                              /* 0x6675A jb (unsigned) */
            continue;
        if (r == -12) {                                               /* 0x66763 jbe 0x66783 */
            if (v < max)                                              /* 0x66789 jge */
                v++;                                                  /* 0x66792 */
        } else if (r == -11) {                                        /* 0x66765 */
            if (v > 1)                                                /* 0x66772 jle */
                v--;                                                  /* 0x6677B */
        }
    }
    f78664(RATE_DLG);                                                 /* 0x6679F */
}

/* ---- 0x6685C the tax revolt -------------------------------------------------------------------------------------- */

void f6685C(int n, int k)
{
    char    buf[100];       /* [ebp-0xCC] */
    int16_t list[9][2];     /* [ebp-0x68]: {nation, score} pairs, one dword each */
    int     count;          /* [ebp-0x18] */
    int     i, j;           /* [ebp-0x30], [ebp-0x1C] */
    int     m;              /* [ebp-0x30] in the first loop, [ebp-0x24] after */
    int     idx;            /* [ebp-0x28] */
    int     take;           /* [ebp-0x20] */
    int     x;              /* [ebp-0x2C] */
    int16_t swap[2];        /* [ebp-0x14] */

    setD32(HELP_TOPIC, 2);                                            /* 0x66880 */
    count = 0;                                                        /* 0x66886 */
    for (m = 1; m <= 9; m++) {                                        /* 0x66889..0x6689C */
        if (D8(NATION(m) + 0x9A) == 0)                                /* 0x668A5 byte [199m + 0xF1566] */
            continue;
        if (m == n)                                                   /* 0x668B2 */
            continue;
        if (S32(REVOLT(m)) != 0)                                      /* 0x668BA */
            continue;
        list[count][0] = (int16_t)m;                                  /* 0x668CA */
        /* 0x668D6..0x668FF: 5 * (9 - movsx16 +0x51) + movsx16 +0x99, stored as a word (the upper half of eax is 0:
           199m < 65536) */
        list[count][1] = (int16_t)(uint16_t)(5 * (9 - S8(NATION(m) + 0x51)) + S8(NATION(m) + 0x99));
        count++;                                                      /* 0x668FC */
    }
    for (i = count - 2; i >= 0; i--)                                  /* 0x66906..0x66926 */
        for (j = 0; j <= i; j++)                                      /* 0x66915..0x6693B */
            if (list[j + 1][1] > list[j][1]) {                        /* 0x6694D cmp ax, bx; jle (int16) */
                memcpy(swap, list[j + 1], sizeof swap);               /* 0x66955 */
                memcpy(list[j + 1], list[j], sizeof swap);            /* 0x66966 */
                memcpy(list[j], swap, sizeof swap);                   /* 0x66970 */
            }
    idx = 0;                                                          /* 0x66978 */
    snprintf(buf, sizeof buf, "%s suffers a Tax Revolt!", NAME(n));   /* 0x66992 (0xB89A4) */
    f79270(-500, n, -200, buf, 0);                                    /* 0x669AE */
    f3A990();                                                         /* 0x669B3 */
    f3CBBC();                                                         /* 0x669B8 */
    for (; k > 0 && idx < count; idx++) {                             /* 0x669BD..0x669CD, 0x66A82 */
        m = list[idx][0];                                             /* 0x669D6 dword [ebp+4idx-0x6A] sar 16 */
        if (S8(NATION(m) + 0x51) >= 9)                                /* 0x669F0 jge */
            continue;
        take = 9 - S8(NATION(m) + 0x51);                              /* 0x66A0E */
        if (take > k)                                                 /* 0x66A13 jle */
            take = k;                                                 /* 0x66A17 */
        if (S8(NATION(m) + 0x9A) == 3 && (D8(FLAGS) & PBEM_BIT) == 0) /* 0x66A48, 0x66A4D */
            f2468C(m, n, take, 2);                                    /* 0x66A64 [CAL1] */
        else
            f66B54(m, n, take);                                       /* 0x66A74 [M7] */
        k -= take;                                                    /* 0x66A7C */
    }
    if (k > 0) {                                                      /* 0x66A8A */
        snprintf(buf, sizeof buf, "%s's revolting cities can no longer be absorbed by the other nations.",
                 NAME(n));                                            /* 0x66AA7 (0xB89BD) */
        f79270(-500, n, -200, buf, 0);                                /* 0x66AC4 */
        if (S8(NATION(n) + 0x9A) == 3 && (D8(FLAGS) & PBEM_BIT) == 0) /* 0x66AD9, 0x66ADE */
            f66D8C(n, k);                                             /* 0x66AED */
        else
            f66C2C(n, k);                                             /* 0x66AFA [M7] */
    }
    x = S8(NATION(n) + 0x51) * S8(NATION(n) + 0x3F);                  /* 0x66B06..0x66B25 cities * rate */
    setD8(NATION(n) + 0x3E, (uint8_t)(D8(NATION(n) + 0x3E) + (uint8_t)x));   /* 0x66B35 treasury */
    setD8(NATION(n) + 0x99, (uint8_t)(D8(NATION(n) + 0x99) - (uint8_t)x));   /* 0x66B45 stock */
}

/* ---- 0x66D8C ChooseMultipleCitiesToRemove ------------------------------------------------------------------------ */

/* The record copies of 0x66DA9..0x66E9E (save) and 0x67208..0x672CC (restore): to_block 1 copies g_ds into the
   blocks, 0 back. Each record as the original's rep movs: areas 0..160 (69 bytes at 69r), territory states 0..159
   (3 bytes at 3t), nations 1..11 (199 bytes at 199n of the block, which leaves its first 199 bytes unused). Port
   guard: a block that is not there (out of memory) is skipped; the original would write through a NULL pointer. */
static void tax_snapshot(int to_block)
{
    uint8_t *area = load_heap_ptr(D32(SNAP_AREA), 0x2B65u);
    uint8_t *ts = load_heap_ptr(D32(SNAP_TSTATE), 0x1E0u);
    uint8_t *nat = load_heap_ptr(D32(SNAP_NATION), 0x954u);
    uint32_t i;

    if (area != NULL) {
        for (i = 0; i < 0xA1u; i++) {                                 /* 0x66DE3 / 0x67215 */
            if (to_block)
                memcpy(area + 69u * i, DP(AREA + 69u * i), 69);       /* 0x66E1D rep movsd (0x11) + movsb */
            else
                memcpy(DP(AREA + 69u * i), area + 69u * i, 69);       /* 0x6724B */
        }
    }
    if (to_block) {
        if (ts != NULL)
            for (i = 0; i < 0xA0u; i++)                               /* 0x66E2F */
                memcpy(ts + 3u * i, DP(TSTATE + 3u * i), 3);          /* 0x66E5E movsw + movsb */
        if (nat != NULL)
            for (i = 1; i <= 0xBu; i++)                               /* 0x66E72 */
                memcpy(nat + 199u * i, DP(NATION(i)), 199);           /* 0x66E99 rep movsd (0x31) + movsw + movsb */
    } else {
        /* the restore copies the nations before the territory states (0x67250, then 0x6728D) */
        if (nat != NULL)
            for (i = 1; i <= 0xBu; i++)                               /* 0x6725F */
                memcpy(DP(NATION(i)), nat + 199u * i, 199);           /* 0x67286 */
        if (ts != NULL)
            for (i = 0; i < 0xA0u; i++)                               /* 0x6729A */
                memcpy(DP(TSTATE + 3u * i), ts + 3u * i, 3);          /* 0x672C9 */
    }
}

void f66D8C(int n, int k)
{
    char buf[200];       /* [ebp-0xF8] */
    int  left;           /* [ebp-0x18] */
    int  removed;        /* [ebp-0x1C] */
    int32_t sel;         /* [ebp-0x20] (its address goes to f79A9C) */
    int  redraw;         /* [ebp-0x24] */
    int  r;              /* [ebp-0x28] */
    int  i;              /* [ebp-0x2C] */

    setD32(SNAP_AREA, load_heap_alloc(0x2B65u));                      /* 0x66DAE malloc, 0x66DB3 */
    setD32(SNAP_TSTATE, load_heap_alloc(0x1E0u));                     /* 0x66DBD, 0x66DC2 */
    setD32(SNAP_NATION, load_heap_alloc(0x954u));                     /* 0x66DCE, 0x66DD3 */
    tax_snapshot(1);                                                  /* 0x66DD8..0x66E9E */
    /* 0x66EA5 0x78B0C("ChooseMultipleCitiesToRemove"): the no-op debug trace, dropped */
    f3CD6C(3);                                                        /* 0x66EAF */
    setD8(0xEE168u, 3);                                               /* 0x66ED9 */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BACKGROUND), S32(BACKBUF));   /* 0x66EDF */
    fg_vbopen(S32(BACKBUF));                                          /* 0x66EEE */
    f3AF8C(0xF51ACu, 0xFB, 0x195);                                    /* 0x66F05 */
    f3AF8C(0xF51B4u, 0x178, 0x195);                                   /* 0x66F19 */
    fg_setcolor(5);                                                   /* 0x66F20 */
    f3A3A0(0x12F, 0x185, DSTR(0xB8A51u));                             /* 0x66F37 "LEFT TO" */
    f3A044(0x150, 0x185, DSTR(0xB8A59u));                             /* 0x66F4B "REMOVE" */
    fg_vbclose();                                                     /* 0x66F50 */
    snprintf(buf, sizeof buf, "%s, select cities to be removed", NAME(n));   /* 0x66F6C (0xB8A60) */
    f3AB7C(buf);                                                      /* 0x66F7A */
    f7876C();                                                         /* 0x66F7F */
    f7853C(REMOVE_DLG);                                               /* 0x66F89 */
    f3B05C(PANEL_DLG);                                                /* 0x66F93 */
    f28CFC(n, 2, 0);                                                  /* 0x66FA2 [CAL1] n's cities */
    redraw = 1;                                                       /* 0x66FA7 */
    sel = 0;                                                          /* 0x66FB0 */
    removed = 0;                                                      /* 0x66FB3 */
    left = k;                                                         /* 0x66FB9 */
    for (;;) {
        /* 0x66FBC */
        fg_vbopen(S32(BACKBUF));                                      /* 0x66FC3 */
        f3C9D0(0x109, 0x195, 0x6E, 0x13, 5);                          /* 0x66FE1 */
        fg_setcolor(5);                                               /* 0x66FE8 */
        if (g_original || S32(CHOICE_N) > 0)   /* BUG(1995): an empty list shows entry -1, "PIRACY" (0xC77D4) */
            f3A0B0(0x109, 0x199, 0x6E, TERR_NAME(S32(CHOICE + 4u * (uint32_t)sel)));   /* 0x67010 */
        f3C9D0(0x136, 0x181, 0x13, 0x13, 5);                          /* 0x67028 */
        fg_setcolor(5);                                               /* 0x6702F */
        snprintf(buf, sizeof buf, "%d", left);                        /* 0x6704B itoa(left, buf, 10) */
        f3A0B0(0x136, 0x185, 0x13, buf);                              /* 0x6705F */
        fg_vbclose();                                                 /* 0x67064 */
        f3F670();                                                     /* 0x67069 */
        if (g_original || S32(CHOICE_N) > 0)   /* BUG(1995): an empty list writes 15 into byte 0xFFB5F (f3F624(-1)) */
            f3F624(S32(CHOICE + 4u * (uint32_t)sel), 15);             /* 0x6707D */
        if (left > 0) {                                               /* 0x67085 jle */
            f3C3D0(PANEL_DLG, 0x2A);                                  /* 0x67093 FINISHED off */
            f3C314(PANEL_DLG, 0x3F);                                  /* 0x670A2 REMOVE on */
        } else {
            f3C314(PANEL_DLG, 0x2A);                                  /* 0x670B3 FINISHED on */
            f3C3D0(PANEL_DLG, 0x3F);                                  /* 0x670C2 REMOVE off */
        }
        if (redraw != 0) {                                            /* 0x670C7 */
            f459E4(n);                                                /* 0x670D0 */
            if (S32(CHOICE_N) > 0)                                    /* 0x670DA jle */
                f3DA60(S32(CHOICE + 4u * (uint32_t)sel));             /* 0x670E8 */
            else
                f3D52C(0, 0);                                         /* 0x670F3 */
            redraw = 0;                                               /* 0x670FA */
        }
        f3CBBC();                                                     /* 0x670FD */
        r = f78BA4(PANEL_DLG, 1);                                     /* 0x6710C */
        if (r == -100)                                                /* 0x67114 */
            continue;
        if (r >= 0) {                                                 /* 0x67123 jl */
            sel = r;                                                  /* 0x67128 a map click on a listed territory */
            redraw = 1;                                               /* 0x6712B */
            continue;
        }
        if (r == -1)                                                  /* 0x6713A */
            break;
        if ((uint32_t)r < (uint32_t)-0x63) {                          /* 0x67149 jb 0x672F1 */
            if (r == -0x65) {                                         /* 0x672F4 REMOVE */
                f78F04(S32(CHOICE + 4u * (uint32_t)sel), n);          /* 0x671AF */
                left--;                                               /* 0x671B7 */
                removed = 1;                                          /* 0x671BF */
                redraw = 1;                                           /* 0x671C2 */
            }
        } else if (r == -0x63) {                                      /* 0x6714F jbe 0x671CA RESTART */
            snprintf(buf, sizeof buf, "Restart %s Tax Revolt?", ADJ(n));   /* 0x671E1 (0xB8A80) */
            if ((D8(0xF097Cu) & 0x08) == 0 || f795A8(buf) == -0x61) { /* 0x671EF, 0x671FA, 0x671FF */
                tax_snapshot(0);                                      /* 0x67208..0x672CC */
                left = k;                                             /* 0x672D1 */
                f28CFC(n, 2, 0);                                      /* 0x672E0 [CAL1] */
                sel = 0;                                              /* 0x672E5 */
                redraw = 1;                                           /* 0x672E8 */
            }
        } else if (r == -0xA) {                                       /* 0x67154 jb, jbe 0x67181 */
            if (S32(CHOICE_N) - 1 > sel) {                            /* 0x67187 jle */
                sel++;                                                /* 0x67193 */
                redraw = 1;                                           /* 0x67196 */
            }
        } else if (r == -9) {                                         /* 0x6715C */
            if (sel > 0) {                                            /* 0x67169 jle */
                sel--;                                                /* 0x67172 */
                redraw = 1;                                           /* 0x67175 */
            }
        }
        /* 0x672FA */
        if (removed != 0) {
            for (i = sel; S32(CHOICE_N) - 1 > i; i++)                 /* 0x67304..0x6731B */
                setD32(CHOICE + 4u * (uint32_t)i, D32(CHOICE + 4u * (uint32_t)(i + 1)));   /* 0x6732A */
            setD32(CHOICE_N, (uint32_t)(S32(CHOICE_N) - 1));          /* 0x6733F */
            f79A9C(&sel);                                             /* 0x67345 */
            removed = 0;                                              /* 0x6734A */
            f28CFC(n, 2, 0);                                          /* 0x67357 [CAL1] */
        }
    }
    f78664(REMOVE_DLG);                                               /* 0x67366 */
    load_heap_free(D32(SNAP_AREA));                                   /* 0x67372 */
    setD32(SNAP_AREA, 0);                                             /* 0x6737C */
    load_heap_free(D32(SNAP_TSTATE));                                 /* 0x67382 */
    setD32(SNAP_TSTATE, 0);                                           /* 0x6738C */
    load_heap_free(D32(SNAP_NATION));                                 /* 0x67392 */
    setD32(SNAP_NATION, 0);                                           /* 0x67397 */
}
