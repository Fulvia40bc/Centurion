/* The computer players' parts of modules CARDS and CITY of docs/M6_PLAN.md, M7 batch 2. Literal translations; the
   address of each instruction a line comes from is in its comment. D = read in the code, G = inference. Prototypes
   and results: aicards.h.

     0x6DBAC  f6DBAC  a computer acquires its cards face down (CARDS)                         492 bytes
     0x6DE04  f6DE04  a computer's buy decision and purchase (CARDS)                          852
     0x6EDA8  f6EDA8  a computer discards down to 8 (CARDS)                                   720
     0x6F078  f6F078  the ranking of the goods held (CARDS)                                   504
     0x333E8  f333E8  a computer builds a city (CITY)                                         268
     0x5DE5C  f5DE5C  a computer reduces a city (CITY)                                      2,392
   Callers: cards.c's f6BD98 (0x6C5E1 f6DBAC, 0x6C6B2 f6DE04) and f6E870 (0x6EC61 f6EDA8); city.c's f32214 (0x330B8,
   0x33241 f333E8) and f5C750 (0x5C862 f5DE5C). Those files carry static not_ported stubs of these names today with
   the same signatures (Claude links them). 0x6F078 is called only by 0x6EDA8 (D). Ported callees: f17EDC (adv.c),
   f194E4 / f76F14 (info.c), f76E50 / f771F8 / f78E74 / f78FEC (rules.c), f57CE8 (move.c), f79270 (kit.c), f7BA64 /
   f78BA4 / f78A20 and the UI kit (game.h, ui.h, kit.h), fg.c. No call into another batch-2 module. No PBEM branch in
   these six (D: no test of [0xE1624] & 0x80; 0x6DE04 tests only the variant bit 0x20): PBEM_DROPPED: none.

   No game_rand (D: the call scan; the cards come off the stacks in order, f76E50), no clock(): the only waits are
   f79270's and f78A20's (ported, virtual clock) and f78BA4's click wait.

   Watcom runtime -> standard C:
     0x7DB3D sprintf -> snprintf bounded to the stack buffer: 52 bytes in 0x6DBAC ([ebp-0x5C]..[ebp-0x28]), 100 in
             0x6EDA8 ([ebp-0xA0]..), 0x333E8 ([ebp-0x7C]..) and 0x5DE5C ([ebp-0xAC]..)
     0x8C897 itoa([0x110AC8], buf, 10) (0x6DD4F, the arguments riding in edx / ebx through f39F90) -> snprintf "%d"
     0x7D5D0 memset(0x1101AC, 0, 0x120) -> memset on g_ds; the 16-byte movsd swaps of 0x6F078 and the 8-byte movsd
             copies of 0x5DE5C's pairs -> memcpy / struct copies
   __CHK dropped; no debug stub and no trace or progress string in these functions (D).

   Data (D, read in the code at the addresses given):
     nation n = 0xF14CC + 199n: +0x01..+0x12 int8 commodity cards (read as dword [rec + k - 3] sar 24 or movsx), +0x13
       int8 calamities (in f76F14's count), +0x29 Mining, +0x3E int8 treasury, +0x51 int8 cities, +0x52 + j uint8 the
       territory + 1 of city j, +0x98 int8 tokens on the board, +0x99 int8 stock; record 0 (0xF14CC) cards 1..18 are
       0x6EDA8's discard pile (cleared first, emptied at the end)
     card stacks 0xF06D0 + 25s (int16 count; 0x6DE04 reads it as dword [25(s-1) + 0xF06CE] sar 16); card sprites
       0xF4D14 (a back), 0xF4C1C + 8c (tiny card c); the spacing int32 [0x11254C]
     int32 [0x110AC4] cards per city, [0x110AC8] the cards drawn (cards.c's counters), [0xF590C + 4n] the stack bought
       from; byte [0xF488C + n] (G: a cities-in-reserve factor, as aicity.c); byte [0xE1624] bit 0x20 the variant
     the ranking table 0x1101AC (18 x 16 bytes: +0 id, +4 value, +8 count, +0xC 0), count int32 [0x1103EC]; the
       Mining goods 0xBDD3C int8[6] = {0, 4, 12, 11, 16, 18} (obj2)
     territory state 0xE2C18 + 3t (+1 the turn byte, +2 bit 0x02 cleared by 0x333E8), territory record 0xE1638 + 35t
       (+1 bits 3..5 the population limit, bit 0x40 a flag), the names 0xC77D8 + 4t, int16 [0xF40FC] the turn (read as
       dword [0xF40FA] sar 16), the land lists [0xF8E7C + 4t] (port heap handles), the area tokens int8 0xE31B8 + 69r +
       0x0E + n (dword [69r + (n-1) + 0xE31C4] sar 24), the AI flags 0xF3580 int16[9][157] (low byte bit 0x80)
     the toggles byte [0xF0980]: bit 0x01 the city-construction AI pause, bit 0x10 the city-support AI pause; int16
       [0xF40BA] the self-running demo's players (unattended waits)

   Stack words read before they are written: none (D: every local is set on each path before its first read; 0x5DE5C
   reads only the pairs it filled).
   Deterministic out-of-range reads (M6_PLAN risk 8), through the data object (D code):
     - 0x6DE04 with no stack chosen (s == 0, variant path) tests the "stack -1" count int16 [0xF06B7] (0x6E016); either
       answer leaves s == 0.
     - 0x6F078 with nothing held returns int32 [0x11019C] (the dword before its table: bytes 0x11019C..0x11019F of
       TRADEAI's byte array, D 0x68E47 / 0x6A8BD), as 0x6EDA8's item 1 below.
     - 0x5DE5C's second pass reads the AI flags of row n (0xF3580 + 314n + 2t) where the first pass reads row n - 1
       (M6_PLAN risk 11, for Erik): for n == 9 the row lies past the [9][157] table (0xF408A + 2t..), read there. Kept
       as the original in both modes (an AI choice, not a crash).
   Port guards (no change in play): writes at a garbage card id (item 1 with --original) are skipped outside the data
   object; the land lists read 0 for a handle that is neither a heap handle nor a data-object address.

   1995 bugs, fixed unless g_original (BUG(1995) in the code; grades D code / G effect):
     1. 0x6EDA8 (M6_PLAN risk 6, as cards.c's human loop): the discards go on while the hand (f76F14: commodities and
        calamities) is over 8; with no commodity left 0x6F078 ranks nothing and returns the dword before its table
        (int32 [0x11019C], TRADEAI's bytes): <= 0 retries for ever (a hang), > 0 is taken as a card id and writes
        record 0 and record n at that offset (memory corruption for a large value). Fixed: the loop ends when 0x6F078
        found no commodity (its count [0x1103EC] is 0); its table writes stay as the original's. --original keeps the
        loop (it hangs as the original does). D code, G reach (calamities are resolved in phase 15, before DISCARD
        EXCESS in phase 19).
   Kept as the original (AI choices or harmless, D):
     - 0x6DE04 calls f17EDC(n, -1, 0) twice with the same arguments (0x6E084, 0x6E097; it writes the states, credits
       and the labels of 0xF634C each time); the second result decides between the hand size and the Gold / Ivory
       test.
     - 0x6DE04 without the variant ignores the "no Gold or no Ivory" case for cities < 9 (no purchase) while it buys
       for a full set of both; an AI heuristic.
     - 0x6EDA8 leaves record 0's cards 1..18 at 0 (their earlier content is lost); its sprites go to the active
       surface without hiding the mouse (as the original).
     - 0x5DE5C rechecks 2 * cities > tokens before its second pass; its msg argument (edx) is never read.
   Texts: "%s acquires...", "Stack %d Empty", "Discard Excess Goods", "%s discards...", "%s builds a city in %s.",
   "%s builds a city in %s using Architecture.", "%s reduces %s" (two copies 0xB5E7F / 0xB5E8D, M6_PLAN egg 28's kind;
   no period, as its twin "%s removes a city in %s"): no obvious error (Erik's ruling: style stays).
   Easter eggs / oddities: none new. The computer's acquired cards are shown as card backs (no peeking, D); "%s reduces
   %s" exists twice in the string pool (known kind, egg 28). */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "adv.h"
#include "aicards.h"
#include "fg.h"
#include "game.h"
#include "info.h"
#include "kit.h"
#include "move.h"
#include "rules.h"
#include "state.h"
#include "ui.h"

#define NATION(n)    (0xF14CCu + 199u * (uint32_t)(n))
#define REC0         0xF14CCu
#define NAME(n)      DSTR(D32(0xC768Cu + 4u * (uint32_t)(n)))
#define TNAME(t)     DSTR(D32(0xC77D8u + 4u * (uint32_t)(t)))
#define TS(t)        (0xE2C18u + 3u * (uint32_t)(t))
#define TERR(t)      (0xE1638u + 35u * (uint32_t)(t))
#define AREA(r)      (0xE31B8u + 69u * (uint32_t)(r))
#define STACK(s)     (0xF06D0u + 25u * (uint32_t)(int32_t)(s))
#define LANDADJ      0xF8E7Cu
#define F3580(row, t) (0xF3580u + 314u * (uint32_t)(int32_t)(row) + 2u * (uint32_t)(t))
#define TURN         0xF40FCu    /* int16: dword [0xF40FA] sar 16 */
#define PER_CITY     0x110AC4u   /* int32 */
#define ACQUIRED     0x110AC8u   /* int32 */
#define BOUGHT_FROM  0xF590Cu    /* int32[] per nation */
#define RESERVE      0xF488Cu    /* int8[] per nation */
#define FLAGS        0xE1624u
#define VARIANT_ANY  0x20u       /* "Allow purchase from any stack" */
#define CARD_BACK    0xF4D14u
#define CARDTINY(c)  (0xF4C1Cu + 8u * (uint32_t)(int32_t)(c))
#define SPACING1     0x11254Cu
#define PICTURE      0xEE168u    /* int8: dword [0xEE165] sar 24 */
#define BACKBUF      0xF52C8u
#define BACKGROUND   0xF52CCu
#define DISC_DLG     0x110ADCu
#define RANK         0x1101ACu   /* 18 x 16 bytes */
#define RANKCNT      0x1103ECu   /* int32 */
#define TOGGLES      0xF0980u
#define SRD          0xF40BAu    /* int16 */
#define ROW(p)       (49 * (p))  /* lea eax,[4p]; sub eax,p; shl eax,4; add eax,p */
#define COL(s)       (39 * (s))  /* lea eax,[4s]; add eax,s; shl eax,3; sub eax,s */

static void add8(uint32_t a, int v) { setD8(a, (uint8_t)(D8(a) + (uint8_t)v)); }   /* byte inc / dec / add / sub */

static void add8g(uint32_t a, int v)   /* the same at a garbage card id (port guard: skipped outside the data object) */
{
    if (ds_valid(a, 1))
        add8(a, v);
}

static int32_t land_word(uint32_t h)   /* movsx word [h] of a land list (as aiplan.c) */
{
    const uint8_t *p = load_heap_ptr(h, 2);
    if (p != NULL)
        return (int16_t)(p[0] | p[1] << 8);
    if (ds_valid(h, 2))
        return S16(h);
    return 0;   /* port guard */
}

/* ==== 0x6DBAC: a computer acquires its cards =========================================================================== */

void f6DBAC(int p, int n)
{
    char buf[52];                                                     /* [ebp-0x5C] */
    int32_t maxw;                                                     /* [ebp-0x14] */
    int32_t cities = S8(NATION(n) + 0x51u);                           /* [ebp-0x20] 0x6DBCC dword [199n + 0xF151A] sar 24 */
    int32_t s, j;                                                     /* [ebp-0x1C], [ebp-0x18] */
    int8_t card;                                                      /* [ebp-0x10] */

    snprintf(buf, sizeof buf, DSTR(0xB8CE1u), NAME(n));               /* 0x6DBEC "%s acquires..." */
    fg_setcolor(5);                                                   /* 0x6DBF6 */
    f3A044(0xF6, 0x62, buf);                                          /* 0x6DC0D */
    setD32(ACQUIRED, 0);                                              /* 0x6DC12 (ecx = 0 from 0x6DC0B) */
    for (s = 0; s < cities; s++) {                                    /* 0x6DC1D, 0x6DC29 jge */
        for (j = 0; j < S32(PER_CITY); j++) {                         /* 0x6DC9D, 0x6DCAC jge */
            card = (int8_t)f76E50(s, n);                              /* 0x6DCB8; byte [ebp-0x10] */
            if (card == 0) {                                          /* 0x6DCC6 */
                snprintf(buf, sizeof buf, DSTR(0xB8CF0u), s + 1);     /* 0x6DC44 "Stack %d Empty" */
                f78884();                                             /* 0x6DC4C */
                f3A42C(0xEC + COL(s), 50 * j + 0x73, 0x22, 2, buf, &maxw, 1);   /* 0x6DC93 */
                f788AC();                                             /* 0x6DC98 */
            } else {
                f78884();                                             /* 0x6DCCC */
                f3AF8C(CARD_BACK, 0xEC + COL(s), 50 * j + 0x6E);      /* 0x6DD03 face down */
                f788AC();                                             /* 0x6DD08 */
                setD32(ACQUIRED, D32(ACQUIRED) + 1u);                 /* 0x6DD0D */
            }
        }
    }
    f3AF8C(CARD_BACK, 0x5F, ROW(p) + 0x24);                           /* 0x6DD33 */
    f39F90(2);                                                        /* 0x6DD45 */
    snprintf(buf, sizeof buf, "%d", S32(ACQUIRED));                   /* 0x6DD4F itoa(.., buf, 10) */
    fg_setcolor(5);                                                   /* 0x6DD56 */
    f3A0B0(0x60, ROW(p) + 0x32, 0x1E, buf);                           /* 0x6DD7F */
    f39F90(1);                                                        /* 0x6DD89 */
}

/* ==== 0x6DE04: a computer's purchase =================================================================================== */

/* "dword [rec + k - 3] sar 24 >= 2" or "movsx byte [rec + k] >= 2": n holds a pair of card k */
static int pair(uint32_t rec, uint32_t k)
{
    return S8(rec + k) >= 2;
}

int f6DE04(int n)
{
    int32_t card = 0;                                                 /* [ebp-0x14] 0x6DE1B */
    int32_t s = 0;                                                    /* [ebp-0x10] 0x6DE1E */
    int32_t result = 0;                                               /* [ebp-0x18] 0x6DE21 */
    uint32_t rec = NATION(n);                                         /* [ebp-0xC] 0x6DE32 */

    if ((D8(FLAGS) & VARIANT_ANY) != 0) {                             /* 0x6DE35 */
        if (S8(rec + 0x51u) <= 3 && S8(rec + 0x3Eu) >= 6) {           /* 0x6DE4B jg, 0x6DE5D jl */
            if (pair(rec, 6) || pair(rec, 5))                         /* 0x6DE66 [rec+3], 0x6DE74 [rec+2] */
                s = 3;                                                /* 0x6DE82 */
            else if (pair(rec, 4) || pair(rec, 3))                    /* 0x6DE8E [rec+1], 0x6DE9C [rec] */
                s = 2;                                                /* 0x6DEA9 */
            else if (pair(rec, 2) || pair(rec, 1))                    /* 0x6DEB5 movsx +2, 0x6DEC1 movsx +1 */
                s = 1;                                                /* 0x6DED1 */
        } else if (S8(rec + 0x51u) <= 5 && S8(rec + 0x3Eu) >= 0xA) {  /* 0x6DEE6 jg, 0x6DEF4 jl */
            if (pair(rec, 10) || pair(rec, 9))                        /* 0x6DEF9 [rec+7], 0x6DF07 [rec+6] */
                s = 5;                                                /* 0x6DF15 */
            else if (pair(rec, 8) || pair(rec, 7))                    /* 0x6DF21 [rec+5], 0x6DF2F [rec+4] */
                s = 4;                                                /* 0x6DF41 */
        } else if (S8(rec + 0x3Eu) >= 0x12 && S8(rec + 0x99u) < 0x12) {   /* 0x6DF56 jl, 0x6DF6B jge */
            if (pair(rec, 12) || pair(rec, 11))                       /* 0x6DF74 [rec+9], 0x6DF82 [rec+8] */
                s = 6;                                                /* 0x6DF90 */
            else if (pair(rec, 14) || pair(rec, 13))                  /* 0x6DF9C [rec+0xB], 0x6DFAA [rec+0xA] */
                s = 7;                                                /* 0x6DFB8 */
            else if (pair(rec, 16) || pair(rec, 15))                  /* 0x6DFC1 [rec+0xD], 0x6DFCF [rec+0xC] */
                s = 8;                                                /* 0x6DFDD */
            else if (pair(rec, 18) || pair(rec, 17))                  /* 0x6DFE6 [rec+0xF], 0x6DFF4 [rec+0xE] */
                s = 9;                                                /* 0x6E002 */
        }
        if (S16(STACK(s - 1)) == 0)                                   /* 0x6E009..0x6E022 [25(s-1) + 0xF06CE] sar 16
                                                                         (s == 0: the "stack -1" count, risk 8) */
            s = 0;                                                    /* 0x6E028 */
    } else if (S8(rec + 0x3Eu) >= 0x12) {                             /* 0x6E030..0x6E03C jl */
        f194E4(n);                                                    /* 0x6E045 */
        if (S8(rec + 0x99u) < 2 * S8(rec + 0x51u))                    /* 0x6E04A..0x6E068 jl */
            goto L_6E0E8;
        if (S8(RESERVE + (uint32_t)n) != 0) {                         /* 0x6E071 byte [n + 0xF488C] */
            if (f17EDC(n, -1, 0) == 0)                                /* 0x6E084; 0x6E089 */
                goto L_6E0E8;
            if (f17EDC(n, -1, 0) != 0 && f76F14(n) <= 8)              /* 0x6E097; 0x6E09C je; 0x6E0A3; 0x6E0AB jle */
                goto L_6E0E8;
        }
        /* 0x6E0AD */
        if (S8(rec + 0x12u) >= 1 && S8(rec + 0x11u) >= 1)             /* 0x6E0B0 [rec+0xF] Gold, 0x6E0BE [rec+0xE] Ivory */
            goto L_6E0E8;
        if (S8(rec + 0x51u) != 9)                                     /* 0x6E0CC..0x6E0D5 jne */
            goto L_6E0EF;
        if (S8(rec + 0x99u) >= 0x19)                                  /* 0x6E0DA..0x6E0E6 jge */
            goto L_6E0EF;
L_6E0E8:
        s = 9;                                                        /* 0x6E0E8 */
    }
L_6E0EF:
    if (s > 0 && S8(rec + 0x3Eu) >= s + s)                            /* 0x6E0EF jle, 0x6E0F5..0x6E105 jl */
        card = f76E50(s - 1, n);                                      /* 0x6E10E; 0x6E113 */
    if (card != 0) {                                                  /* 0x6E116 */
        setD32(BOUGHT_FROM + 4u * (uint32_t)n, (uint32_t)(s - 1));    /* 0x6E123 */
        add8(rec + 0x3Eu, -(int)(uint8_t)(s + s));                    /* 0x6E12A..0x6E132 sub byte, dl */
        add8(rec + 0x99u, (int)(uint8_t)(s + s));                     /* 0x6E135..0x6E13D add byte, dl */
        result = (s << 8) + 1;                                        /* 0x6E143..0x6E14A */
    }
    return result;
}

/* ==== 0x6F078: the ranking of the goods held ============================================================================ */

int f6F078(int n)
{
    uint32_t rec = NATION(n);                                         /* [ebp-0x14] 0x6F09D */
    int32_t cities;                                                   /* [ebp-0x20] */
    int32_t i, j;                                                     /* [ebp-0x28], [ebp-0x18] */
    int32_t flag, c, v;                                               /* [ebp-0x1C] */
    uint32_t a;
    uint8_t tmp[16];                                                  /* [ebp-0x38] */

    memset(DP(RANK), 0, 0x120);                                       /* 0x6F0AC */
    setD32(RANKCNT, 0);                                               /* 0x6F0B3 */
    cities = S8(rec + 0x51u);                                         /* 0x6F0BC [rec + 0x4E] sar 24 */
    for (i = 0x12; i >= 1; i--) {                                     /* 0x6F0C5, 0x6F14F, 0x6F155 jl */
        if (D8(rec + (uint32_t)i) == 0)                               /* 0x6F161 test dl, dl */
            continue;
        a = RANK + 16u * D32(RANKCNT);                                /* 0x6F16D ebx = count << 4 */
        setD32(a, (uint32_t)i);                                       /* 0x6F173 */
        setD32(a + 8u, (uint32_t)(int32_t)S8(rec + (uint32_t)i));     /* 0x6F182 movsx */
        if ((i + 1) / 2 <= cities)                                    /* 0x6F188..0x6F19A jg */
            setD32(a + 8u, D32(a + 8u) + 1u);                         /* 0x6F19C */
        flag = 0;                                                     /* 0x6F1A4 */
        for (j = 0; j < 6; j++)                                       /* 0x6F0D1, 0x6F0D7 jge */
            if ((int32_t)S8(0xBDD3Cu + (uint32_t)j) == i) {           /* 0x6F0E0 [j + 0xBDD39] sar 24 */
                flag = 1;                                             /* 0x6F0F0 */
                break;
            }
        c = S8(rec + (uint32_t)i);
        if (D8(rec + 0x29u) != 0 && flag != 0)                        /* 0x6F0FA, 0x6F100 */
            v = (c + 1) * (c + 1);                                    /* 0x6F106..0x6F119 */
        else
            v = c * c;                                                /* 0x6F11C..0x6F12B */
        v = ((i + 1) >> 1) * v;                                       /* 0x6F12E..0x6F137 */
        setD32(RANK + 16u * D32(RANKCNT) + 4u, (uint32_t)v);          /* 0x6F143 */
        setD32(RANKCNT, D32(RANKCNT) + 1u);                           /* 0x6F149 */
    }
    for (i = S32(RANKCNT) - 2; i >= 0; i--)                           /* 0x6F1AF, 0x6F1CB bubble sort, descending */
        for (j = 0; j <= i; j++)                                      /* 0x6F1E2, 0x6F1E8 jg */
            if (S32(RANK + 16u * (uint32_t)(j + 1) + 4u) > S32(RANK + 16u * (uint32_t)j + 4u)) {   /* 0x6F1FC..0x6F20A jle */
                memcpy(tmp, DP(RANK + 16u * (uint32_t)(j + 1)), 16);  /* 0x6F21B movsd x4 */
                memcpy(DP(RANK + 16u * (uint32_t)(j + 1)), DP(RANK + 16u * (uint32_t)j), 16);   /* 0x6F237 */
                memcpy(DP(RANK + 16u * (uint32_t)j), tmp, 16);        /* 0x6F24A */
            }
    return S32(0x11019Cu + 16u * D32(RANKCNT));                       /* 0x6F258 [16 count + 0x11019C] */
}

/* ==== 0x6EDA8: a computer discards ======================================================================================= */

void f6EDA8(int n)
{
    char buf[100];                                                    /* [ebp-0xA0] */
    int32_t x0 = 0x4B;                                                /* [ebp-0x20] 0x6EDC3 */
    uint32_t rec = NATION(n);                                         /* [ebp-0x18] 0x6EDD8 */
    int32_t x;                                                        /* [ebp-0x28] */
    int32_t i, j, id, lim, swapped, c;                                /* [ebp-0x1C], [ebp-0x38], [ebp-0x2C], [ebp-0x34], [ebp-0x30] */
    int code;                                                         /* [ebp-0x24] */

    f3CD6C(S8(PICTURE));                                              /* 0x6EDE3 */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BACKGROUND), S32(BACKBUF));   /* 0x6EE0B */
    fg_vbopen(S32(BACKBUF));                                          /* 0x6EE1A */
    f3A18C(0, 8, 0x280, 0xB0, DSTR(0xB8E60u));                        /* 0x6EE38 "Discard Excess Goods" */
    f3A18C(0, 0x1E, 0x280, 0x23, NAME(n));                            /* 0x6EE59 */
    fg_setcolor(0xD);                                                 /* 0x6EE60 */
    snprintf(buf, sizeof buf, DSTR(0xB8E75u), NAME(n));               /* 0x6EE7F "%s discards..." */
    f3A044(x0 + 0xF, 0xBA, buf);                                      /* 0x6EE98 */
    f3C0D8(DISC_DLG, 0x2A, 1);                                        /* 0x6EEAC FINISHED style 1 */
    f3BFBC(DISC_DLG, 0x2A, 1);                                        /* 0x6EEC0 enabled */
    f3BFBC(DISC_DLG, 0x8D, 0);                                        /* 0x6EED1 disabled */
    f3B05C(DISC_DLG);                                                 /* 0x6EEDB */
    fg_vbclose();                                                     /* 0x6EEE0 */
    f3CC14();                                                         /* 0x6EEE5 */
    x = x0 + 5;                                                       /* 0x6EEF0 */
    for (i = 1; i <= 0x12; i++)                                       /* 0x6EEFC */
        setD8(REC0 + (uint32_t)i, 0);                                 /* 0x6EF0D */

    while (f76F14(n) > 8) {                                           /* 0x6EF15..0x6EF20 jle */
        id = f6F078(n);                                               /* 0x6EF29 [ebp-0x2C] */
        /* BUG(1995) 1 (file comment): with no commodity held 0x6F078 returns the dword before its table, and the
           loop retries for ever (<= 0) or writes at a garbage card id (> 0). Fixed: the loop ends. */
        if (!g_original && S32(RANKCNT) == 0)
            break;
        if (id <= 0)                                                  /* 0x6EF34 jle */
            continue;
        swapped = 0;                                                  /* 0x6EF3A */
        lim = (int32_t)((uint32_t)((int32_t)((uint32_t)id + 1u) / 2) * 2u + 1u);   /* 0x6EF4C..0x6EF5B */
        for (j = 0x12; lim <= j; j--) {                               /* 0x6EF3D, 0x6EF46, 0x6EF5C jg */
            if (S8(REC0 + (uint32_t)j) == 0)                          /* 0x6EF64 [j + 0xF14C9] sar 24 */
                continue;
            add8g(REC0 + (uint32_t)id, 1);                            /* 0x6EF74 */
            add8g(rec + (uint32_t)id, -1);                            /* 0x6EF80 */
            add8(REC0 + (uint32_t)j, -1);                             /* 0x6EF85 */
            add8(rec + (uint32_t)j, 1);                               /* 0x6EF91..0x6EF9A */
            swapped = 1;                                              /* 0x6EF9C */
            break;
        }
        if (swapped != 0)                                             /* 0x6EF9F */
            continue;
        add8g(REC0 + (uint32_t)id, 1);                                /* 0x6EFAC */
        add8g(rec + (uint32_t)id, -1);                                /* 0x6EFB8 */
    }

    for (i = 1; i <= 0x12; i++) {                                     /* 0x6EFC8, 0x6EFCE jg */
        c = S8(REC0 + (uint32_t)i);                                   /* 0x6EFD7 [i + 0xF14C9] sar 24: [ebp-0x30] */
        for (j = 0; j < c; j++) {                                     /* 0x6EFEA, 0x6EFF0 jge */
            f771F8(i);                                                /* 0x6EFFB */
            add8(REC0 + (uint32_t)i, -1);                             /* 0x6F003 */
            f3AF8C(CARDTINY(i), x, 0xC8);                             /* 0x6F01C */
            x += S32(SPACING1);                                       /* 0x6F026 */
        }
    }
    do {
        if (f7BA64() == 0)                                            /* 0x6F02B */
            code = f78BA4(DISC_DLG, 0);                               /* 0x6F03B */
        else {
            f78A20(S16(SRD) == 0 ? 0xFA : 0x3E8);                     /* 0x6F045 cmp word; 0x6F05B */
            code = -1;                                                /* 0x6F060 */
        }
    } while (code != -1);                                             /* 0x6F067 */
}

/* ==== 0x333E8: a computer builds a city ================================================================================== */

void f333E8(int t, int n, int k, int arch)
{
    char buf[100];                                                    /* [ebp-0x7C] */

    f78E74(t, n);                                                     /* 0x3340C */
    f57CE8(t, n, 0, k);                                               /* 0x3341C */
    setD8(TS(t) + 2u, (uint8_t)(D8(TS(t) + 2u) & 0xFDu));             /* 0x3342D and byte, -3 */
    setD8(TS(t) + 2u, (uint8_t)(D8(TS(t) + 2u) & 0x03u));             /* 0x33440 and byte, 3 */
    if (arch == 0)                                                    /* 0x33453 */
        snprintf(buf, sizeof buf, DSTR(0xB3ACBu), NAME(n), TNAME(t)); /* 0x33485 "%s builds a city in %s." */
    else
        snprintf(buf, sizeof buf, DSTR(0xB3AE3u), NAME(n), TNAME(t)); /* "%s builds a city in %s using Architecture." */
    if ((D8(TOGGLES) & 0x01u) != 0)                                   /* 0x3348D */
        f79270(-500, n, t, buf, 0);                                   /* 0x334D5 -> 0x334E5 */
    else if (f7BA64() == 0)                                           /* 0x33496 */
        f79270(0x7D0, n, t, buf, 0);                                  /* 0x334A2 push eax (0) */
    else
        f79270(S16(SRD) == 0 ? 0x1F4 : 0x5DC, n, t, buf, 0);          /* 0x334B9 test si, si */
}

/* ==== 0x5DE5C: a computer reduces a city ================================================================================== */

typedef struct { int32_t t, score; } Pair;   /* 8 bytes: [ebp + 8i - 0xAAC] / [ebp + 8i - 0xAA8] (B at - 0x5AC) */

/* the candidates of one pass: n's cities built this turn (this_turn) or not, each scored by n's tokens in its land
   neighbours (0x5DF19..0x5DF8E / 0x5E3A8..0x5E41D, the j loop 0x5DEC0 / 0x5E34F) */
static int32_t candidates(int n, int this_turn, Pair *a)
{
    uint32_t rec = NATION(n);
    int32_t cnt = 0;                                                  /* [ebp-0x30] */
    int32_t i, j, t, h, sc;                                           /* [ebp-0x34], [ebp-0x14], [ebp-0x18], [ebp-0x2C] */

    for (i = 0; S8(rec + 0x51u) > i; i++) {                           /* 0x5DF19 [rec + 0x4E] sar 24; jle */
        t = (int32_t)D8(rec + 0x52u + (uint32_t)i) - 1;               /* 0x5DF2D */
        if (((int32_t)D8(TS(t) + 1u) == S16(TURN)) != this_turn)      /* 0x5DF43 byte [3t + 0xE2C19]; 0x5DF51 */
            continue;
        a[cnt].t = t;                                                 /* 0x5DF67 / 0x5E3F6 */
        h = (int32_t)D32(LANDADJ + 4u * (uint32_t)t);                 /* 0x5DF78 [4t + 0xF8E7C] */
        sc = 0;                                                       /* 0x5DF84 */
        for (j = 1; land_word((uint32_t)h) >= j; j++)                 /* 0x5DEC3 movsx word; jl */
            sc += S8(AREA(land_word((uint32_t)h + 2u * (uint32_t)j) + 1) + 0x0Eu + (uint32_t)n);   /* 0x5DED3..0x5DEFB */
        a[cnt].score = sc;                                            /* 0x5DF06 */
        cnt++;                                                        /* 0x5DF10 */
    }
    return cnt;
}

static int32_t poplimit(int32_t t)   /* mov al, [35t + 0xE1639]; shl al, 2; shr al, 5 (unsigned byte) */
{
    return (int32_t)((uint8_t)(D8(TERR(t) + 1u) << 2) >> 5);
}

/* the sorts and the stable partitions of one pass (0x5DF9D..0x5E296 / 0x5E42C..0x5E725); row = the AI-flag row */
static void order(Pair *a, Pair *b, int32_t cnt, int32_t row)
{
    int32_t i, j, k;
    Pair tmp;                                                         /* [ebp-0x48] */

    for (i = cnt - 2; i >= 0; i--)                                   /* 0x5DF9D by population limit, descending */
        for (j = 0; j <= i; j++)
            if (poplimit(a[j + 1].t) > poplimit(a[j].t)) {            /* 0x5E026 cmp bl, al; jbe */
                tmp = a[j + 1];
                a[j + 1] = a[j];
                a[j] = tmp;
            }
    for (i = cnt - 2; i >= 0; i--)                                    /* 0x5E063 by score, descending */
        for (j = 0; j <= i; j++)
            if (a[j + 1].score > a[j].score) {                        /* 0x5E0B2 jle */
                tmp = a[j + 1];
                a[j + 1] = a[j];
                a[j] = tmp;
            }
    for (i = 0; i < cnt; i++)                                         /* 0x5E0F3 */
        b[i] = a[i];
    k = 0;                                                            /* 0x5E121 */
    for (i = 0; i < cnt; i++)                                         /* 0x5E129 AI flag bit 0x80 first */
        if ((D8(F3580(row, b[i].t)) & 0x80u) != 0)                    /* 0x5E148 [314 row + 2t + 0xF3580] */
            a[k++] = b[i];
    for (i = 0; i < cnt; i++)                                         /* 0x5E176 */
        if ((D8(F3580(row, b[i].t)) & 0x80u) == 0)
            a[k++] = b[i];
    for (i = 0; i < k; i++)                                           /* 0x5E1C3 */
        b[i] = a[i];
    cnt = k;                                                          /* 0x5E1EC [ebp-0x38] */
    k = 0;
    for (i = 0; i < cnt; i++)                                         /* 0x5E1F9 territory flag 0x40 first */
        if ((D8(TERR(b[i].t) + 1u) & 0x40u) != 0)                     /* 0x5E21F [35t + 0xE1639] */
            a[k++] = b[i];
    for (i = 0; i < cnt; i++)                                         /* 0x5E24C */
        if ((D8(TERR(b[i].t) + 1u) & 0x40u) == 0)
            a[k++] = b[i];
}

static void reduce(int n, int32_t t, uint32_t fmt)   /* 0x5E298..0x5E30D / 0x5E727..0x5E7A5 */
{
    char buf[100];                                                    /* [ebp-0xAC] */

    f78FEC(t, n);                                                     /* 0x5E2A1 / 0x5E730 */
    snprintf(buf, sizeof buf, DSTR(fmt), NAME(n), TNAME(t));          /* 0x5E2CB / 0x5E75A "%s reduces %s" */
    if ((D8(TOGGLES) & 0x10u) == 0)                                   /* 0x5E2D8 / 0x5E767 */
        f79270(0x9C4, n, t, buf, 0);                                  /* 0x5E7A5 / 0x5E781 */
    else
        f79270(-500, n, t, buf, 0);                                   /* 0x5E30D / 0x5E7A5 */
}

void f5DE5C(int n, const char *msg)
{
    uint32_t rec = NATION(n);                                         /* [ebp-0x1C] 0x5DE87 */
    Pair a[160], b[160];                                              /* [ebp-0xAAC], [ebp-0x5AC] */
    int32_t cnt;

    (void)msg;                                                        /* [ebp-0x40] never read */
    if (2 * S8(rec + 0x51u) > S8(rec + 0x98u)) {                      /* 0x5DE94..0x5DEAA jle */
        cnt = candidates(n, 1, a);                                    /* pass 1: built this turn */
        if (cnt != 0) {                                               /* 0x5DF93 */
            order(a, b, cnt, n - 1);                                  /* row n - 1 ([ebp-0x28], 0x5E141) */
            reduce(n, a[0].t, 0xB5E7Fu);
            return;
        }
    }
    if (2 * S8(rec + 0x51u) <= S8(rec + 0x98u))                       /* 0x5E31B..0x5E339 jle */
        return;
    cnt = candidates(n, 0, a);                                        /* pass 2: the others */
    if (cnt == 0)                                                     /* 0x5E422 */
        return;
    order(a, b, cnt, n);                                              /* row n ([ebp-0x3C], 0x5E5D0: risk 8 / 11) */
    reduce(n, a[0].t, 0xB5E8Du);
}
