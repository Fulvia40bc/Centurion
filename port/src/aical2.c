/* Module AICAL2 of M7 batch 2: the computers' side of the unit-loss calamities (CAL2) and of Civil War (CAL4).
   Literal translations; the address of each instruction a line comes from is in its comment. D = read in the code,
   G = inference. Prototypes: aical2.h.

     0x260C4  f260C4  a computer loses L unit points (Famine, Epidemic, Flood)                5,204 bytes
     0x27518  f27518  a computer victim assigns the secondary losses                             824
     0x27850  f27850  a computer's Grain against Famine                                         1,180
     0x2E45C  f2E45C  a computer's river among the tied flood plains                              320
     0x2C190  f2C190  the computer's faction choice screen (Civil War)                            264
     0x2C298  f2C298  the computer chooses a faction's units (Iberia's AI rows rewritten)        1,248
     0x2C894  f2C894  the computer takes a faction's Military losses                            2,168
     0x2D10C  f2D10C  a computer beneficiary annexes cities                                       180
     0x2D1C0  f2D1C0  a computer beneficiary annexes tokens                                       544
   Their callers (cal1.c, cal2.c, cal4.c) reach them through static not_ported stubs of the same names today; the
   signatures here are the ones those call sites use (aical2.h).

   Ported routines called here: cal1.c's f28CFC (the choice list), cal2.c's f291D0 (the secondary losses shown) and
   cal2_river_text (the "TIGRUS" fix), cal4.c's f2EE6C (one automatic step shown), rules.c's f78F04 / f78E74 / f78FEC,
   move.c's f57CE8 / f57E28, kit.c's f79270, and the UI / map helpers of ui.h, game.h, map.h, mapdata.h, panel.h, fg.h.

   Not ported here (other modules of M7 batch 2): AICAL1's 0x213C0 (int f213C0(int n): computer n's own city to lose,
   territory + 1; called by 0x260C4 at 0x264A7 with (int8)n) and 0x20C70 (int f20C70(int p, int v): the city of v that
   computer p takes, territory + 1; called by 0x2D10C at 0x2D13A). Each is a static stub below of the same name that
   calls not_ported(address); built with -DAICAL2_TEST (port/test/aical2_test.c) they go to the test's recorders.

   game_rand (f78AE4): no call in these nine routines (D: no call of 0x78AE4 or 0x76F70 in them). No World State /
   Move List Stack operation either (D). No clock(): every wait is f78A20 (headless it returns at once).

   PBEM_DROPPED: nothing. No PBEM routine is called here and no branch tests byte [0xE1624] bit 0x80 (D).

   Watcom runtime -> standard C:
     0x7DB3D sprintf -> snprintf bounded to the stack buffer (100 bytes in each routine: the locals' layout; the
             calamity name of 0x260C4 28 bytes); the formats are written out with their addresses in the comments
     0x8C897 itoa(v, buf, 10) -> snprintf "%d" (0x2C894 writes into its caller's text buffer: 12 bytes are enough)
     0x7D5D0 memset -> memset on the data object
   __CHK dropped. No debug stub (0x78B0C / 0x78B24) is called here.

   Data (D, read in the code at the addresses given):
     nation record n: 0xF14CC + 199n (rules.h, move.h): +0x08 Grain cards, +0x25..+0x36 advances (+0x2C Coinage),
       +0x51 int8 cities, +0x5C int8 AST position, +0x60 int8 token areas with +0x61.. their territory + 1, +0x98 int8
       tokens on the board, +0x99 int8 stock, +0x9A 0 out / 2 computer / 3 human
     area record r = t + 1: 0xE31B8 + 69r: +0 a byte read as territory + 1 by 0x2C894 (the name index of 0xC77D4),
       +0x0E + n tokens, +0x3B + v the "lost territory" mark of the Civil War victim v
     territory record t: 0xE1638 + 35t: +1 bits 3..5 the population limit, +1 bit 0x80 a flood-plain city site
     territory state t: 0xE2C18 + 3t: +0 owner, +2 bit 0x01 a city
     AST table int16 0xBCFDA + 68(n-1) + 4p: the era of AST position p (0x260C4)
     flood plains 0xBEE08 + 22s: +0 int16 the primary victim's units on the plain (written by cal2.c's 0x2D4F0), +2
       int16 count, +4 char *river, +8 int16 territories
     int32 0xBEDE0[n] the river (plain) a computer never floods (Africa -1, Iberia 0 / 1, Illyria 2, Thrace 2, Crete
       -1, Asia -1, Assyria 3, Babylon 3, Egypt 4; 0xBEDE8 = Iberia's, rewritten by 0x2E45C)
     AI tables int32 0xC0618 + 16n + 4j (j < 4: preferred city sites) and 0xC06B8 + 20(n-1) + 4j (j < 5: token
       areas); Iberia's rows 0xC0638.. / 0xC06CC.. rewritten by 0x2C298
     int8 0xEED84 + 160n + t the AI's value of territory t (sort key of the priority passes)
     int32 0xF4204[n] 1 = more than 75% of n's tokens on the board (cal1.c's 0x1E0A0)
     int32 0xF52E0 + 40n + 4k and int8 0x109E23 + 9(n-1) + k: 0x27518's ranking (G: n's history with k)
     int16 0xF796C[k] secondary units of k; uint8 0xF547C[k] the losses assigned; int16 0xF409A[n] Grain used
     the choice list int32 [0xF7C9C] / 0xF7CA0[157] (cal1.c's f28CFC)
     calamity names 0xC76E0 + 4 * card id; nation names 0xC768C[n]; territory names 0xC77D8[t] (0xC77D4[t + 1])
     byte [0xF0980] bit 0x04: the computers' messages wait for CONTINUE; int16 [0xF40BA] the self-running demo;
     int32 [0xF40F0] the player count; [0xF8E78] the current dialog; 0xF596C the panel dialog

   Locals read before they are written (aiplan.c's file comment, items 1./2.):
     - 0x260C4's R ([ebp-0x30]) is not set for Flood (the kind chain 0x261F9 / 0x262D7 skips it), but 0x263F1..0x26473
       still computes with it. Its result is dead: the city loop is skipped for Flood (0x2648E), and neither
       [ebp-0x30] nor [ebp-0x24] is read on the Flood path; idiv by 1 cannot fault. Port: R = 0 for Flood (no frame
       to model, nothing observable).
     - 0x260C4's calamity name [ebp-0x74] is set only for 0x15 / 0x1B / 0x19, the only kinds its callers pass (D).
       Port: "" otherwise.
     - 0x27518 reads list[0] / list[1] ([ebp-0x5C], [ebp-0x58]) without testing that the list holds them: list[0]
       with no nation listed, list[1] when the first nation's count already exceeds the total. Neither is reachable:
       every caller passes a total smaller than the sum of 0xF796C over the nations listed (cal1.c's 0x23ACC: total >
       2, cal2.c's: > 20 / 10 / 25), so a second nation exists whenever anything is left after the first (D from the
       callers). Port: the list starts zeroed.

   1995 bugs, fixed unless g_original (each marked BUG(1995) in the code; grades code / effect):
     1. 0x260C4's token plans (D / G, reachable in ordinary play): the Famine passes (0x26C2E..0x26F90) and both
        priority passes (0x27044.., 0x2726B..) repeat until L is 0 without counting what an earlier round already
        planned, and the plans are applied as they are (0x2749D f57CE8(t, n, 0, A), board - A, stock + A). So a
        territory can be planned for more tokens than it holds: e.g. a computer without cities holding 3 tokens in a
        territory X of limit 3 and 3 in Y of limit 1, Famine L = 6: round 1 plans 1 in X (pass 1: limit - 2), then 1
        in X and 1 in Y (pass 3); round 2's pass 1 finds X still holding 3 >= L = 3 on the board and plans all 3 there
        again: X is planned 5 and its count becomes -2 (int8 0xFE), Y keeps 2, while the board total +0x98 drops by 6
        (port/test/aical2_test.c, part A4).
        With no token left to plan (only cities, or every token already reduced) a round plans nothing and the loop
        never ends (M6_PLAN risk 6). Fixed: no plan takes more than the territory holds minus what is planned there
        already (Epidemic's priority pass keeps 1, as its own test does), and a round that plans nothing ends the
        passes. The first round is never affected (no territory can be over-planned in it, D), so the fix changes
        nothing where the original keeps the board consistent. --original: literal, including the hang.
     2. 0x2C894 takes the faction's tokens with f57CE8 only (0x2CCBB, 0x2CF49): the faction's +0x98 is not lowered and
        +0x99 not raised (D / D), unlike NationLosesUnits 0x27FCC and 0x29E34's own removals. The faction's counts then
        disagree with the board: 0x29E34's "no longer exists" and annexing tests read them, and 0x2D1C0 is given a k
        the board cannot supply (bug 3). Fixed: board - 1, stock + 1 for each token, as 0x27FCC does.
     3. 0x2D1C0 repeats its pass 0 while k != 0 (0x2D230): when o has fewer tokens on the board than k (its +0x98
        stale, bug 2) a pass moves nothing and the loop never ends (D / G; M6_PLAN risk 6). Fixed: a pass 0 that moves
        nothing ends the loop (the next one would be identical). --original: literal, including the hang.
     4. 0x2C894's second pass shows entry 0 of the list without testing the count (0x2CDF6..0x2CE8C): with an empty list
        that is -1, the name "PIRACY" (0xC77D4), f3F624(-1, 15) writes 15 into byte 0xFFB5F, the top byte of the heap
        pointer [0xFFB5C] (in the port a handle, which that write breaks), and f3DA60(-1) centres on a table byte
        before 0xC803C (D / G: not reachable, the faction always has units in its list when called). Fixed as cal1.c's
        bug 6: no name, no highlight, the map as it is (f3D52C(0, 0)).
     5. Text (Erik's ruling of 19 Nov, TEXT_SWEEP.md): "%s choses the %s to flood." (0xB35A7) -> "chooses"; the river
        name "TIGRUS-EUPHRATES" through cal2.c's cal2_river_text (the same fix as Flood's and CAL3's messages).
   Kept as the original (an AI choice, or a slip that is deterministic; D code, G effect unless marked):
     - 0x27850's hand test reads the bytes +0x25..+0x36 (0xF14ED + 199n + i, sar 24) and +0x2C (0xF14F5) where the
       commodity cards +0x01..+0x12 and the Grain count +0x08 are meant: the whole test sits 0x24 bytes too high, in
       the advances (M6_PLAN risk 11 saw only the +0x2C half). It only steers how many Grain cards the computer spends.
       The cards are not taken here (only counted in 0xF409A; ADV's 0x1818C spends them).
     - 0x260C4: Famine / Epidemic eliminate R cities through AICAL1's 0x213C0 and credit 5 - (tokens placed) whatever
       0x78FEC placed; Flood's city loop stops on L < 1 or when the "capacity" int16 [0xBEE08 + 22s] - 5 * cities
       (the primary victim's units on the plain, written by 0x2D4F0: for a secondary victim or the Pirates another
       nation's count; M6_PLAN risk 8, read byte-exact) is not below L; the list compaction after each city moves a
       -1 hole only past entries > 0 and reads one entry past the count; the Flood token plans include territories
       where n has no tokens (A = 0, skipped when applied).
     - 0x27518 ranks nation 0 and n itself too; the first nation gets its whole count, unclamped; the rest loop
       subtracts the count, not the amount given (the same result, D).
     - 0x2E45C returns list[count] when every listed plain is n's own river (G: not reachable, Flood calls it only
       with two or more tied plains, and no nation has two own rivers).
     - 0x2C298: the retry pass starts at candidate 1 (index reset to 0, then incremented); the fourth city sets taken =
       9; without keep any city is taken (the first table entry that differs); tokens are counted and tested in x's
       column but moved from src's (x == src at both call sites, D).
     - 0x2C894 for factions 12 / 13 reads rows 12 / 13 of 0xC0618 (0xC06D8.., 0xC06E8.., inside 0xC06B8, Iberia's and
       Illyria's rows) and rows 11 / 12 of 0xC06B8 (0xC0794.., 0xC07A8.., past the table): M6_PLAN risk 8, read
       byte-exact through the data object. The city test sets "core" at the first entry that differs from t (so
       practically always); the per-removal display names the territory through the area record's byte +0.
     - 0x2D1C0 compares territory + 1 with the 0xC06B8 entries (territory numbers elsewhere), and its "match" is reset
       only per pass, not per area: pass 1 takes every area from the first match on, pass 0 the areas before it.
     - 0x2D10C does not test that 0x20C70 found a city (r = 0 would mark area 0 and remove city -1; G: the caller asks
       for fewer cities than the faction holds).
   Easter eggs / oddities: M6_PLAN egg 6, "the computer never floods its own river" (0x2E45C, table 0xBEDE0, Iberia's
   entry rewritten to the Po in 3-player games: known); new here: "%s forces %s to eliminate %s." is three separate
   copies in the string pool (0xB2AE3, 0xB2B01, 0xB2B1F), and "GRAIN / USED / LOSSES / REDUCED TO", "LEFT TO" and
   "ANNEX" have copies of their own for the computers' screens; the computer's Grain test scores its advances where
   its cards are meant (above); the computer's annexing screen 0x2C190 labels the box "ANNEX" with a second copy of the
   word for mode 6. */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "aical2.h"
#include "cal1.h"
#include "cal2.h"
#include "cal4.h"
#include "fg.h"
#include "game.h"
#include "kit.h"
#include "map.h"
#include "mapdata.h"
#include "move.h"
#include "panel.h"
#include "rules.h"
#include "state.h"
#include "ui.h"

#define NATION(n)    (0xF14CCu + 199u * (uint32_t)(n))
#define NAME(n)      DSTR(D32(0xC768Cu + 4u * (uint32_t)(n)))
#define TERR_NAME(t) DSTR(D32(0xC77D8u + 4u * (uint32_t)(t)))
#define TERR(t)      (0xE1638u + 35u * (uint32_t)(t))
#define TSTATE(t)    (0xE2C18u + 3u * (uint32_t)(t))
#define AREA(r)      (0xE31B8u + 69u * (uint32_t)(r))
#define PLAIN(s)     (0xBEE08u + 22u * (uint32_t)(s))   /* +0 int16 units, +2 int16 count, +4 char *, +8 int16[7] */
#define CHOICE       0xF7CA0u   /* int32[157] */
#define CHOICE_N     0xF7C9Cu   /* int32 */
#define SEC_CNT      0xF796Cu   /* int16[11] */
#define SEC_LOSS     0xF547Cu   /* uint8[14] */
#define GRAIN_USED   0xF409Au   /* int16[10] */
#define OWN_RIVER    0xBEDE0u   /* int32[10] */
#define AI_CITIES    0xC0618u   /* int32 [n][4] */
#define AI_TOKENS    0xC06B8u   /* int32 [n - 1][5] */
#define AI_VALUE     0xEED84u   /* int8 [n][160] */
#define MOSTLY_OUT   0xF4204u   /* int32[10] */
#define PLAYERS_ALL  0xF40F0u   /* int32 */
#define CUR_DLG      0xF8E78u
#define PANEL_DLG    0xF596Cu
#define BACKBUF      0xF52C8u
#define BACKGROUND   0xF52CCu
#define BGNUM        0xEE168u
#define AI_PAUSE     0xF0980u   /* byte, bit 0x04 */
#define DEMO         0xF40BAu   /* int16 */

#define LIST(i)      S32(CHOICE + 4u * (uint32_t)(i))

/* ---- calls into code that is not ported yet (see the file comment) ----------------------------------------------- */

#ifdef AICAL2_TEST
int  aical2_test_f213C0(int n);
int  aical2_test_f20C70(int p, int v);
#define f213C0(n)    aical2_test_f213C0(n)
#define f20C70(p, v) aical2_test_f20C70(p, v)
#else
#include "aical1.h"                             /* f213C0, f20C70 (M7 batch 2, linked) */

#endif

/* ---- small shared pieces ------------------------------------------------------------------------------------------ */

static void inc8(uint32_t a) { setD8(a, (uint8_t)(D8(a) + 1u)); }
static void dec8(uint32_t a) { setD8(a, (uint8_t)(D8(a) - 1u)); }

/* n's tokens in territory t: byte [69(t+1) + n + 0xE31C6] (read as dword [.. + 0xE31C3] sar 24 or movsx byte) */
static uint32_t tok(int32_t t, int32_t n)
{
    return AREA(t + 1) + 0x0Eu + (uint32_t)n;
}

/* the background 3 into the back buffer: f3CD6C(3), byte [0xEE168] = 3 (stored after the call), fg_vbcopy of the
   whole screen from [0xF52CC] to [0xF52C8] (0x2786D..0x2789F, 0x2C1B2..0x2C1EE, 0x2C8B2..0x2C8E7) */
static void background3(void)
{
    f3CD6C(3);
    setD8(BGNUM, 3);
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BACKGROUND), S32(BACKBUF));
}

/* the wait of 0x260C4's second message after each removal (0x26662..0x266C9, 0x26A36..0x26AFE, 0x274D0..0x2750A):
   CONTINUE (-500) when bit 0x04 of [0xF0980] is set (f7BA64 not called), else 2000 ms, or unattended (f7BA64 != 0)
   500 ms, 1500 with the self-running demo */
static int loss_wait(void)
{
    if (D8(AI_PAUSE) & 4)
        return -500;
    if (f7BA64() == 0)
        return 0x7D0;
    return S16(DEMO) == 0 ? 0x1F4 : 0x5DC;
}

/* 0x2C894's step wait (0x2C9FE..0x2CA1D and its three twins): 500 ms when unattended without the demo, else 1500 */
static int step_wait(void)
{
    if (f7BA64() != 0 && S16(DEMO) == 0)
        return 0x1F4;
    return 0x5DC;
}

/* 0x260C4's plan tables (int16 T[160] / A[160] on its stack) with the plan count c2 */
#define PLANS 0xA0

/* one plan appended: T[c2] = t1 (territory + 1, the low word), A[c2] = a, c2 + 1. Port guard: a 161st plan is not
   stored (the original would write past A into T; G: unreachable, every plan takes at least 1 of L <= 21, Flood's at
   most the plain's 7 territories) */
static void plan(int16_t *T, int16_t *A, int32_t *c2, int32_t t1, int32_t a)
{
    if (*c2 < PLANS) {
        T[*c2] = (int16_t)t1;
        A[*c2] = (int16_t)a;
    }
    (*c2)++;
}

/* BUG(1995) fix 1 (file comment): the tokens the plans already take in territory t1 - 1 */
static int32_t planned(const int16_t *T, const int16_t *A, int32_t c2, int32_t t1)
{
    int32_t j, sum = 0;

    for (j = 0; j < c2 && j < PLANS; j++)
        if (T[j] == (int16_t)t1)
            sum += A[j];
    return sum;
}

/* BUG(1995) fix 1: a plan of a tokens of n in t clamped to what is left there (tk less what is planned); 0 = none */
static int32_t clamp_plan(const int16_t *T, const int16_t *A, int32_t c2, int32_t t, int32_t tk, int32_t a)
{
    int32_t room = tk - planned(T, A, c2, t + 1);

    return a > room ? (room > 0 ? room : 0) : a;
}

/* ==== 0x260C4 a computer loses L unit points ======================================================================== */

void f260C4(int n, int L, int kind, int s)
{
    char cal[28];        /* [ebp-0x74]..[ebp-0x59] the calamity's name */
    char buf[100];       /* [ebp-0xD8]..[ebp-0x75] */
    int16_t T[160];      /* [ebp-0x218] the plans: territory + 1 */
    int16_t A[160];      /* [ebp-0x358] the plans: tokens */
    int32_t era;         /* [ebp-0xC] (word) */
    int32_t cities;      /* [ebp-0x34] (word) */
    int32_t tokens;      /* [ebp-0x10] (word) */
    int32_t terr;        /* [ebp-0x20] (word) */
    int32_t R;           /* [ebp-0x30] the cities to eliminate (compared as a word) */
    int32_t per;         /* [ebp-0x18] (word) */
    int32_t tmp;         /* [ebp-0x24] (word) */
    int32_t c2;          /* [ebp-0x28] the plan count (indexed as a word) */
    int32_t t;           /* [ebp-0x54] */
    int32_t i;           /* [ebp-0x68] / [ebp-0x60] / [ebp-0x64] */
    int32_t j;           /* [ebp-0x60] */
    int32_t c;           /* [ebp-0x14] n's flood-plain cities */
    int32_t cap;         /* [ebp-0x38] (word) */
    int32_t lim;         /* [ebp-0x1C] (word) */
    int32_t tk;          /* [ebp-0x2C] (word) */
    int32_t a, e, round, q, rm;
    uint32_t rec = NATION(n);

    f28CFC(n, kind, s);                                             /* 0x260EE */
    cal[0] = '\0';   /* port: the original leaves it uninitialised for other kinds (file comment) */
    if (kind == 0x15)                                               /* 0x260F3 */
        snprintf(cal, sizeof cal, "%s", DSTR(0xB2AA4u));            /* 0x26120 sprintf(cal, "Famine") */
    else if (kind == 0x1B)                                          /* 0x26103 */
        snprintf(cal, sizeof cal, "%s", DSTR(0xB2AABu));            /* "Epidemic" */
    else if (kind == 0x19)                                          /* 0x26112 */
        snprintf(cal, sizeof cal, "%s", DSTR(0xB2AB4u));            /* "Flood" */
    snprintf(buf, sizeof buf, "%s ravages %s.\nChoosing sites of losses.", cal, NAME(n));   /* 0x26143 (0xB2ABA) */
    f79270(0x5DC, n, -200, buf, 1);                                 /* 0x26160 */
    era = S16(0xBCFDAu + 68u * (uint32_t)(n - 1) + 4u * (uint32_t)S8(rec + 0x5Cu));   /* 0x26165..0x26183 */
    cities = (int16_t)S8(rec + 0x51u);                              /* 0x26195 movsx ax */
    tokens = (int16_t)S8(rec + 0x98u);                              /* 0x261A7 */
    terr = (int16_t)S8(rec + 0x60u);                                /* 0x261B9 */
    c2 = 0;                                                         /* 0x261C6 */
    for (i = 0; i < 0xA0; i++) {                                    /* 0x261CB..0x261F7 */
        T[i] = 0;
        A[i] = 0;
    }

    /* the cities to eliminate */
    R = 0;   /* port: not set for Flood in the original, and dead there (file comment) */
    if (kind == 0x15) {                                             /* 0x261F9 */
        if (L == 0xA) {                                             /* 0x26206 */
            if (era <= 1 && cities >= 3)                            /* 0x26213 jg, 0x2621F jl */
                R = 1;
            else if (era == 2 && cities >= 4)                       /* 0x26234, 0x2623D */
                R = 1;
            else if (era == 3 && cities >= 5)                       /* 0x26252, 0x2625B */
                R = 1;
            else if (era >= 4 && cities >= 6)                       /* 0x26270, 0x26279 */
                R = 1;
            else
                R = tokens - 0xA < 2 * cities;                      /* 0x26287..0x2629E setl */
        } else {
            R = (tokens - terr < L && cities > 0) ? 1 : 0;          /* 0x262A8..0x262CF */
        }
    } else if (kind == 0x1B) {                                      /* 0x262D7 */
        if (L >= 0xA) {                                             /* 0x262E6 jl 0x263A5 */
            if (era <= 1 && cities >= 4)                            /* 0x262F3, 0x262FF */
                R = 2;
            else if (era == 2 && cities >= 5)                       /* 0x26314, 0x2631D */
                R = 2;
            else if (era == 3 && cities >= 6)                       /* 0x26332, 0x2633B */
                R = 2;
            else if (era >= 4 && cities >= 7)                       /* 0x26350, 0x26359 */
                R = 2;
            else if (tokens - 0x10 < 2 * cities)                    /* 0x26367..0x26376 jge */
                R = 1;
            else if (tokens - 0x10 < 2 * cities - 2)                /* 0x26381..0x26393 jge */
                R = 2;
            else
                R = 0;
        } else {
            R = (tokens - terr < L && cities > 0) ? 1 : 0;          /* 0x263A5..0x263C9 */
        }
    }
    per = 1;                                                        /* 0x263CC */
    if (kind == 0x15)
        per = 5;                                                    /* 0x263D9 */
    else if (kind == 0x1B)
        per = 4;                                                    /* 0x263EA */
    tmp = (int16_t)(uint16_t)((uint32_t)R * (uint32_t)per           /* 0x263F1..0x2641A: the low word of */
                              + (uint32_t)(S8(rec + 0x98u) - S8(rec + 0x60u)));   /* R * per + tokens - areas */
    if (tmp < L) {                                                  /* 0x2641D movsx eax, word; jge */
        q = (L - tmp) / per;                                        /* 0x26438 idiv */
        rm = (L - tmp) % per;                                       /* 0x2644F idiv (edx) */
        R += q + (rm > 0);                                          /* 0x26451 setg, 0x26464 */
        if ((int16_t)R > (int16_t)cities)                           /* 0x2646A cmp ax, word; jle */
            R = cities;                                             /* 0x26473 */
    }
    if (cities > 0 && (int16_t)R > 0 && kind != 0x19) {             /* 0x26476, 0x26482, 0x2648E */
        while ((int16_t)R != 0) {                                   /* 0x26498 cmp word, 0; je 0x26C10 */
            t = f213C0((int8_t)n) - 1;                              /* 0x264A7 [AICAL1] movsx eax, byte; 0x264AC */
            snprintf(buf, sizeof buf, "%s forces %s to eliminate %s.", cal, NAME(n), TERR_NAME(t));   /* 0x264D6 (0xB2AE3) */
            f79270(-0x226, n, t, buf, 1);                           /* 0x264F1 */
            f78A20(0x1F4);                                          /* 0x264FB */
            if (kind == 0x15) {                                     /* 0x26503 */
                f78FEC(t, n);                                       /* 0x26512 */
                L -= 5 - S8(tok(t, n));                             /* 0x26517..0x2653A */
                while (S8(tok(t, n)) > 0 && L > 0) {                /* 0x2653D..0x26565 */
                    f57CE8(t, n, 0, 1);                             /* 0x26578 */
                    L--;                                            /* 0x26580 */
                    dec8(rec + 0x98u);                              /* 0x2658A */
                    inc8(rec + 0x99u);                              /* 0x26597 */
                }
            } else if (kind == 0x1B) {                              /* 0x265A2 */
                f78FEC(t, n);                                       /* 0x265B1 */
                L -= 5 - S8(tok(t, n));                             /* 0x265B6..0x265D9 */
                while (S8(tok(t, n)) > 1 && L > 0) {                /* 0x265DC..0x26601: one token stays */
                    f57CE8(t, n, 0, 1);                             /* 0x26610 */
                    L--;                                            /* 0x26618 */
                    dec8(rec + 0x98u);                              /* 0x26622 */
                    inc8(rec + 0x99u);                              /* 0x2662F */
                }
            }
            snprintf(buf, sizeof buf, "%s forces %s to eliminate %s.", cal, NAME(n), TERR_NAME(t));   /* 0x2665D (0xB2B01) */
            f79270(loss_wait(), n, t, buf, 1);                      /* 0x266C9 */
            /* t taken out of the list: its slot gets the next entry; a -1 hole moves up past an entry > 0 */
            for (j = 0; j < S32(CHOICE_N); j++) {                   /* 0x266CE..0x2670D */
                if (LIST(j) == t) {                                 /* 0x26733 */
                    setD32(CHOICE + 4u * (uint32_t)j, D32(CHOICE + 4u * (uint32_t)(j + 1)));   /* 0x2673D */
                    setD32(CHOICE + 4u * (uint32_t)(j + 1), 0xFFFFFFFFu);   /* 0x26746 */
                } else if (LIST(j) == -1 && LIST(j + 1) > 0) {      /* 0x266DB, 0x266E0 jle */
                    setD32(CHOICE + 4u * (uint32_t)j, D32(CHOICE + 4u * (uint32_t)(j + 1)));   /* 0x266F6 */
                    setD32(CHOICE + 4u * (uint32_t)(j + 1), 0xFFFFFFFFu);   /* 0x26700 (edi = -1) */
                }
            }
            setD32(CHOICE_N, (uint32_t)(S32(CHOICE_N) - 1));        /* 0x26753 */
            R--;                                                    /* 0x2675F */
        }                                                           /* (R == 0: 0x26C10) */
    } else if (L > 0 && kind == 0x19) {                             /* 0x26767 jle 0x26C10, 0x26772 jne 0x26C10 */
        /* Flood: n's cities on the plain, then every token there */
        c = 0;                                                      /* 0x2677E */
        cap = 0;                                                    /* 0x26781 */
        for (i = 0; S16(PLAIN(s) + 2u) > i; i++) {                  /* 0x26784..0x267C2 dword [0xBEE08 + 22s] sar 16 */
            e = S16(PLAIN(s) + 8u + 2u * (uint32_t)i);              /* 0x267CB dword [.. + 0xBEE0E] sar 16 */
            if ((D8(TSTATE(e) + 2u) & 1) != 0                       /* 0x267DE test ch, 1 */
                && (int32_t)D8(TSTATE(e)) == n                      /* 0x26813..0x26821 */
                && (D8(TERR(e) + 1u) & 0x80) != 0)                  /* 0x26856 test byte, 0x80 */
                c++;                                                /* 0x26866 */
        }
        cap = (int16_t)(uint16_t)(D16(PLAIN(s)) - 5u * (uint32_t)c);   /* 0x2686E..0x2687B: risk 8, the units word */
        for (i = 0; S16(PLAIN(s) + 2u) > i; i++) {                  /* 0x2687E..0x268A9 */
            e = S16(PLAIN(s) + 8u + 2u * (uint32_t)i);
            if ((D8(TSTATE(e) + 2u) & 1) == 0                       /* 0x268DF */
                || (int32_t)D8(TSTATE(e)) != n                      /* 0x26919 */
                || (D8(TERR(e) + 1u) & 0x80) == 0)                  /* 0x26951 */
                continue;
            L -= 5;                                                 /* 0x2695E */
            f3F670();                                               /* 0x26962 */
            f3F624(e, 15);                                          /* 0x2698D */
            f3DA60(e);                                              /* 0x269B3 */
            f78A20(0x1F4);                                          /* 0x269BD */
            f78F04(e, n);                                           /* 0x269E8 */
            snprintf(buf, sizeof buf, "%s forces %s to eliminate %s.", cal, NAME(n), TERR_NAME(e));   /* 0x26A31 (0xB2B1F) */
            f79270(loss_wait(), n, e, buf, 1);                      /* 0x26AFE */
            if (L < 1)                                              /* 0x26B03 jl 0x26B16 */
                break;
            if (!((int16_t)cap < L))                                /* 0x26B09..0x26B10 jl 0x26885 */
                break;
        }
        if (L > 0) {                                                /* 0x26B16 jle 0x26C10 */
            for (i = 0; S16(PLAIN(s) + 2u) > i; i++) {              /* 0x26B20..0x26B4B */
                e = S16(PLAIN(s) + 8u + 2u * (uint32_t)i);
                L -= S8(tok(e, n));                                 /* 0x26B71..0x26B8C */
                plan(T, A, &c2, e + 1, S8(tok(e, n)));              /* 0x26BA5..0x26BFC: word + 1, movsx dx, byte */
                if (L < 1)                                          /* 0x26C07 jge 0x26B27 */
                    break;
            }
        }
    }

    /* 0x26C10: the token plans */
    if (kind == 0x15) {                                             /* 0x26C16 */
        if (S32(MOSTLY_OUT + 4u * (uint32_t)n) == 0) {              /* 0x26C20 */
            /* three passes, repeated until L is 0 */
            for (;;) {
                if (L <= 0)                                         /* 0x26C2E jle 0x271B2 */
                    break;
                round = c2;
                for (i = 0; i < S32(CHOICE_N); i++) {               /* pass 1, 0x26C38..0x26C4E: limit over 2 */
                    t = LIST(i);
                    lim = (D8(TERR(t) + 1u) >> 3) & 7;              /* 0x26C6C shl al, 2; shr al, 5 */
                    tk = (int16_t)S8(tok(t, n));                    /* 0x26C99 movsx ax, byte */
                    f3F624(t, 15);                                  /* 0x26CB3 */
                    if (lim <= 2)                                   /* 0x26CBC jle */
                        continue;
                    if ((int16_t)tk < (int16_t)lim)                 /* 0x26CC8 cmp ax, word; jl */
                        continue;
                    if (tk < L)                                     /* 0x26CDF jge 0x26D22 */
                        a = lim - 2;                                /* 0x26CF7..0x26CFA: all but 2 of the limit */
                    else
                        a = L;                                      /* 0x26D44: all that is left, then L = 0 (0x26D51) */
                    if (!g_original) {   /* BUG(1995): never more than the territory holds (file comment, 1) */
                        a = clamp_plan(T, A, c2, t, tk, a);
                        if (a <= 0)
                            continue;
                    }
                    plan(T, A, &c2, t + 1, a);                      /* 0x26CE3..0x26D04 / 0x26D22..0x26D47 */
                    L -= a;                                         /* 0x26D10..0x26D1A dword [.. - 0x35C] sar 16 */
                    if (L == 0)                                     /* the 0x26D22 branch falls into 0x26D54 (the */
                        break;                                      /* other one cannot reach 0: lim - 2 < tk < L) */
                }
                if (L > 0) {                                        /* 0x26D54 jle 0x26EC9 */
                    for (i = 0; i < S32(CHOICE_N); i++) {           /* pass 2, 0x26D5E..0x26D74: limit 2 */
                        t = LIST(i);
                        lim = (D8(TERR(t) + 1u) >> 3) & 7;          /* 0x26D92 */
                        tk = (int16_t)S8(tok(t, n));                /* 0x26DBF */
                        if (lim != 2)                               /* 0x26DCE jne */
                            continue;
                        if ((int16_t)tk < (int16_t)lim)             /* 0x26DD6 jl */
                            continue;
                        e = tk - lim;
                        if (e < L && e != 0)                        /* 0x26DE9 jge 0x26E3E, 0x26DF7 je 0x26E3E */
                            a = e;                                  /* 0x26E11..0x26E14: the excess over the limit */
                        else if (e >= L)                            /* 0x26E4D jl 0x26E82 */
                            a = L;                                  /* 0x26E70, then L = 0 (0x26E7D) */
                        else if (tk < 1)                            /* 0x26E89 jl */
                            continue;
                        else
                            a = 1;                                  /* 0x26EAE */
                        if (!g_original) {   /* BUG(1995): never more than the territory holds (file comment, 1) */
                            a = clamp_plan(T, A, c2, t, tk, a);
                            if (a <= 0)
                                continue;
                        }
                        plan(T, A, &c2, t + 1, a);                  /* 0x26DFC..0x26E20 / 0x26E4F..0x26E73 / 0x26E8F.. */
                        L -= a;                                     /* 0x26E36 / 0x26E7D / 0x26EBB */
                        if (L == 0)                                 /* 0x26E80 jmp 0x26EC9 / 0x26EC3 jne 0x26D65 */
                            break;
                    }
                }
                if (L > 0) {                                        /* 0x26EC9 jle 0x26C2E */
                    for (i = 0; i < S32(CHOICE_N); i++) {           /* pass 3, 0x26ED3..0x26EE9: one token each */
                        t = LIST(i);
                        tk = (int16_t)S8(tok(t, n));                /* 0x26F34 */
                        if (tk <= 0)                                /* 0x26F45 jle */
                            continue;
                        if (!g_original && clamp_plan(T, A, c2, t, tk, 1) <= 0)   /* BUG(1995): (file comment, 1) */
                            continue;
                        plan(T, A, &c2, t + 1, 1);                  /* 0x26F47..0x26F66 */
                        L--;                                        /* 0x26F73 */
                        if (L == 0)                                 /* 0x26F7B jne 0x26EDA; 0x26F85 jle 0x271B2 */
                            break;
                    }
                }
                if (!g_original && c2 == round)   /* BUG(1995): a round that plans nothing ends (file comment, 1) */
                    break;
            }
        } else {
            /* 0x26F95: by the AI's value of each territory, ascending (bubble from the end), then one token at a time */
            for (i = 1; i < S32(CHOICE_N); i++)                     /* 0x26F95..0x26FAF */
                for (j = S32(CHOICE_N) - 1; j >= i; j--)            /* 0x26FB5..0x26FC9 */
                    if (S8(AI_VALUE + 160u * (uint32_t)n + (uint32_t)LIST(j - 1))      /* 0x27001 cmp al, cl; jle */
                        > S8(AI_VALUE + 160u * (uint32_t)n + (uint32_t)LIST(j))) {
                        e = LIST(j - 1);                            /* 0x27014 [4j + 0xF7C9C] */
                        setD32(CHOICE + 4u * (uint32_t)(j - 1), (uint32_t)LIST(j));   /* 0x2702B */
                        setD32(CHOICE + 4u * (uint32_t)j, (uint32_t)e);   /* 0x27038 */
                    }
            for (;;) {
                if (L <= 0)                                         /* 0x27044 jle 0x271B2 */
                    break;
                round = c2;
                for (i = 0; i < S32(CHOICE_N); i++) {               /* 0x2704E..0x27064 */
                    t = LIST(i);
                    tk = (int16_t)S8(tok(t, n));                    /* 0x27082 */
                    if (!g_original)   /* BUG(1995): what earlier rounds planned there is gone (file comment, 1) */
                        tk -= planned(T, A, c2, t + 1);
                    while ((int16_t)tk > 0 && L > 0) {              /* 0x2708D..0x27099 */
                        plan(T, A, &c2, t + 1, 1);                  /* 0x2709E..0x270BA */
                        L--;                                        /* 0x270C7 */
                        tk--;                                       /* 0x270CD */
                    }
                }
                if (!g_original && c2 == round)   /* BUG(1995): a round that plans nothing ends (file comment, 1) */
                    break;
            }
        }
    } else if (kind == 0x1B) {                                      /* 0x270D5 */
        if (S32(MOSTLY_OUT + 4u * (uint32_t)n) == 0) {              /* 0x270DE */
            /* n's own token areas, the last first: all but one token each (no repeat) */
            if (L > 0) {                                            /* 0x270EF jle 0x271B2 */
                for (i = S8(rec + 0x60u) - 1; i >= 0; i--) {        /* 0x270F7..0x27108, 0x27113 jl 0x271B2 */
                    e = D8(rec + 0x61u + (uint32_t)i);              /* 0x27129 territory + 1 */
                    tk = (int16_t)S8(AREA(e) + 0x0Eu + (uint32_t)n);   /* 0x2713E */
                    if (tk <= 1)                                    /* 0x27150 jle */
                        continue;
                    a = tk - 1 > L ? L : tk - 1;                    /* 0x27170..0x27186 */
                    plan(T, A, &c2, e, a);                          /* 0x27168, 0x2718C, 0x27197 */
                    L -= a;                                         /* 0x2719B..0x271A5 */
                    if (L == 0)                                     /* 0x271AC jne 0x2710D */
                        break;
                }
            }
        } else {
            /* 0x271BC: by the AI's value, then one token at a time, one staying in each territory */
            for (i = 1; i < S32(CHOICE_N); i++)                     /* 0x271BC..0x271D6 */
                for (j = S32(CHOICE_N) - 1; j >= i; j--)            /* 0x271DC..0x271F0 */
                    if (S8(AI_VALUE + 160u * (uint32_t)n + (uint32_t)LIST(j - 1))      /* 0x27234 cmp al, ch; jle */
                        > S8(AI_VALUE + 160u * (uint32_t)n + (uint32_t)LIST(j))) {
                        e = LIST(j - 1);                            /* 0x2723B */
                        setD32(CHOICE + 4u * (uint32_t)(j - 1), (uint32_t)LIST(j));   /* 0x27252 */
                        setD32(CHOICE + 4u * (uint32_t)j, (uint32_t)e);   /* 0x2725F */
                    }
            for (;;) {
                if (L <= 0)                                         /* 0x2726B jle 0x271B2 */
                    break;
                round = c2;
                for (i = 0; i < S32(CHOICE_N); i++) {               /* 0x27275..0x2728B */
                    t = LIST(i);
                    tk = (int16_t)S8(tok(t, n));                    /* 0x272A9 */
                    if (!g_original)   /* BUG(1995): what earlier rounds planned there is gone (file comment, 1) */
                        tk -= planned(T, A, c2, t + 1);
                    while ((int16_t)tk > 1 && L > 0) {              /* 0x272B4..0x272C1 */
                        plan(T, A, &c2, t + 1, 1);                  /* 0x272C3..0x272E7 */
                        L--;                                        /* 0x272F2 */
                        tk -= 1;                                    /* 0x272F8 */
                    }
                }
                if (!g_original && c2 == round)   /* BUG(1995): a round that plans nothing ends (file comment, 1) */
                    break;
            }
        }
    }
    if (c2 > PLANS)
        c2 = PLANS;   /* port guard (see plan) */

    /* 0x271B2: the plans merged per territory */
    for (i = 0; i < c2; i++) {                                      /* 0x271B2, 0x27303 movsx eax, word; jle */
        if (T[i] == -1)                                             /* 0x2731D */
            continue;
        for (j = i + 1; j < c2; j++) {                              /* 0x27322..0x27338 */
            if (T[j] != T[i])                                       /* 0x27350 cmp dx, di; jne */
                continue;
            A[i] = (int16_t)(A[i] + A[j]);                          /* 0x27363 add word */
            A[j] = 0;                                               /* 0x27370 */
            T[j] = -1;                                              /* 0x2737B */
        }
    }
    /* 0x27387: and applied */
    for (i = 0; i < c2; i++) {                                      /* 0x27387, 0x273EC */
        if (A[i] <= 0)                                              /* 0x27406 jle */
            continue;
        snprintf(buf, sizeof buf, "%s forces %s to reduce\npopulation by %d in %s.", cal, NAME(n), (int)A[i],
                 DSTR(D32(0xC77D4u + 4u * (uint32_t)(int32_t)T[i])));   /* 0x27448 (0xB2B3D) */
        f79270(-0x226, n, T[i] - 1, buf, 1);                        /* 0x2746E */
        f78A20(0x1F4);                                              /* 0x27478 */
        f57CE8(T[i] - 1, n, 0, A[i]);                               /* 0x2749D */
        setD8(rec + 0x98u, (uint8_t)(D8(rec + 0x98u) - (uint8_t)A[i]));   /* 0x274B3 sub byte, dl */
        setD8(rec + 0x99u, (uint8_t)(D8(rec + 0x99u) + (uint8_t)A[i]));   /* 0x274CA add byte, dl */
        f79270(loss_wait(), n, T[i] - 1, buf, 1);                   /* 0x274D0..0x2750A, 0x273E1 */
    }
}

/* ==== 0x27518 a computer victim assigns the secondary losses ======================================================== */

void f27518(int n, int trader, int total, int kind)
{
    struct {
        int32_t k;       /* +0 the nation */
        int32_t score;   /* +4 */
    } rk[10], tmp;       /* [ebp-0xAC] + 8k; tmp [ebp-0x34] */
    int32_t list[10];    /* [ebp-0x5C] + 4i */
    char buf[100];       /* [ebp-0x110]..[ebp-0xAD] */
    int32_t cnt;         /* [ebp-0x14] */
    int32_t i;           /* [ebp-0x28] */
    int32_t j;           /* [ebp-0x10] */
    int32_t share;       /* [ebp-0x1C] */
    int32_t give;        /* [ebp-0xC] */
    int32_t c;

    memset(list, 0, sizeof list);   /* port: list[0] / list[1] are read unchecked (file comment, G unreachable) */
    for (i = 0; i <= 9; i++) {                                      /* 0x2753B..0x27562 */
        rk[i].k = i;                                                /* 0x27543 / 0x2757E */
        if (D8(NATION(i) + 0x9Au) == 0)                             /* 0x27571 byte [199i + 0xF1566] */
            rk[i].score = -99999;                                   /* 0x2754D 0xFFFE7961 */
        else
            rk[i].score = (int32_t)(D32(0xF52E0u + 40u * (uint32_t)n + 4u * (uint32_t)i)   /* 0x275C1 */
                                    + 100u * (uint32_t)S8(0x109E23u + 9u * (uint32_t)(n - 1) + (uint32_t)i));   /* 0x27595 dword [.. + 0x109E20] sar 24 */
    }
    for (i = 8; i >= 0; i--)                                        /* 0x275D6..0x275F8 */
        for (j = 0; j <= i; j++)                                    /* 0x275FA..0x2760D */
            if (rk[j + 1].score > rk[j].score) {                    /* 0x27615..0x27625 jle */
                tmp = rk[j + 1];                                    /* 0x27634 movsd x2 */
                rk[j + 1] = rk[j];                                  /* 0x2764A */
                rk[j] = tmp;                                        /* 0x27659 */
            }
    cnt = 0;                                                        /* 0x2765F */
    for (i = 0; i <= 9; i++)                                        /* 0x27662..0x27671 */
        if (S16(SEC_CNT + 2u * (uint32_t)rk[i].k) > 0)              /* 0x2767D dword [2k + 0xF796A] sar 16 */
            list[cnt++] = rk[i].k;                                  /* 0x276A2 */
    memset(DP(SEC_LOSS), 0, 0xE);                                   /* 0x276B4 */
    setD8(SEC_LOSS + (uint32_t)list[0], D8(SEC_CNT + 2u * (uint32_t)list[0]));   /* 0x276BC the low byte */
    total -= S16(SEC_CNT + 2u * (uint32_t)list[0]);                 /* 0x276D9 */
    if (total > 0) {                                                /* 0x276DC jle 0x27819 */
        share = cnt > 2 ? (total + 1) / 2 : total;                  /* 0x276E9..0x27700 */
        c = S16(SEC_CNT + 2u * (uint32_t)list[1]);                  /* 0x27706 */
        give = c > share ? share : c;                               /* 0x27710..0x27727 */
        setD8(SEC_LOSS + (uint32_t)list[1], (uint8_t)give);         /* 0x27730 */
        total -= give;                                              /* 0x27739 */
        if (total > 0 && cnt >= 3) {                                /* 0x2773C, 0x27746 */
            if (share > total)                                      /* 0x27756 jle */
                share = total;                                      /* 0x2775D */
            c = S16(SEC_CNT + 2u * (uint32_t)list[2]);              /* 0x27763 */
            give = c > share ? share : c;                           /* 0x2776D..0x27784 */
            setD8(SEC_LOSS + (uint32_t)list[2], (uint8_t)give);     /* 0x2778D */
            total -= give;                                          /* 0x27796 */
            if (total > 0 && cnt >= 4)                              /* 0x27799, 0x277A3 */
                for (i = 3; i < cnt && total > 0; i++) {            /* 0x277AD..0x277EE */
                    c = S16(SEC_CNT + 2u * (uint32_t)list[i]);      /* 0x277F7 */
                    setD8(SEC_LOSS + (uint32_t)list[i], (uint8_t)(c > total ? total : c));   /* 0x27801..0x277C2 */
                    total -= c;                                     /* 0x277D9: the count, not the amount given */
                }
        }
    }
    snprintf(buf, sizeof buf, "Assignments for %s secondary losses",
             DSTR(D32(0xC76E0u + 4u * (uint32_t)kind)));            /* 0x27830 (0xB2B6C) */
    f291D0(n, trader, buf);                                         /* 0x27844 */
}

/* ==== 0x27850 a computer's Grain against Famine ===================================================================== */

/* the two number boxes: Grain used, and the losses (at least 0) (0x27970..0x27A3A, 0x27C06..0x27CD1) */
static void grain_boxes(int n, char *buf, size_t size)
{
    int32_t loss;

    fg_vbopen(S32(BACKBUF));                                        /* 0x27976 / 0x27C0D */
    f3C9D0(0x136, 0x181, 0x13, 0x13, 5);                            /* 0x27991 / 0x27C28 */
    fg_setcolor(5);                                                 /* 0x27998 / 0x27C2F */
    snprintf(buf, size, "%d", (int)S16(GRAIN_USED + 2u * (uint32_t)n));   /* 0x279BE itoa, dword [0xF4098 + 2n] sar 16 */
    f3A0B0(0x136, 0x185, 0x13, buf);                                /* 0x279D2 / 0x27C69 */
    f3C9D0(0x136, 0x195, 0x13, 0x13, 5);                            /* 0x279EA / 0x27C81 */
    fg_setcolor(5);                                                 /* 0x279F1 / 0x27C88 */
    loss = S8(SEC_LOSS + (uint32_t)n);                              /* 0x27A07 dword [n + 0xF5479] sar 24 */
    if (loss < 0)                                                   /* 0x27A12 jge */
        loss = 0;
    snprintf(buf, size, "%d", loss);                                /* 0x27A16 itoa */
    f3A0B0(0x136, 0x199, 0x13, buf);                                /* 0x27A30 / 0x27CC7 */
    fg_vbclose();                                                   /* 0x27A35 / 0x27CCC */
    f3CBBC();                                                       /* 0x27A3A / 0x27CD1 */
}

void f27850(int n)
{
    char buf[100];       /* [ebp-0x88]..[ebp-0x25] */
    int32_t v;           /* [ebp-0x1C] the hand's value */
    int32_t i;           /* [ebp-0x20] */
    int32_t c;
    int32_t Lc;          /* [ebp-0x18] */
    int32_t g;
    uint32_t rec = NATION(n);

    v = 0;                                                          /* 0x2786A */
    background3();                                                  /* 0x2786D..0x2789F */
    fg_vbopen(S32(BACKBUF));                                        /* 0x278AE */
    fg_setcolor(5);                                                 /* 0x278B8 */
    f3A3A0(0x125, 0x185, DSTR(0xB2B90u));                           /* 0x278CF "GRAIN" */
    f3A044(0x15A, 0x185, DSTR(0xB2B96u));                           /* 0x278E3 "USED" */
    f3A3A0(0x12F, 0x199, DSTR(0xB2B9Bu));                           /* 0x278F7 "LOSSES" */
    f3A044(0x150, 0x199, DSTR(0xB2BA2u));                           /* 0x2790B "REDUCED TO" */
    fg_vbclose();                                                   /* 0x27910 */
    f7876C();                                                       /* 0x27915 */
    snprintf(buf, sizeof buf, "%s - choosing Grain reduction in Famine losses.", NAME(n));   /* 0x27931 (0xB2BAD) */
    f3A9F4(buf);                                                    /* 0x2793F */
    f3B05C(PANEL_DLG);                                              /* 0x27949 */
    setD16(GRAIN_USED + 2u * (uint32_t)n, 0);                       /* 0x27953 */
    f459E4(n);                                                      /* 0x2795E */
    f3F670();                                                       /* 0x27963 */
    f3D9FC(n);                                                      /* 0x2796B */
    grain_boxes(n, buf, sizeof buf);                                /* 0x27970..0x27A3A */
    f78A20(0x3E8);                                                  /* 0x27A44 */
    g = S8(rec + 0x08u);                                            /* 0x27A50 dword [0xF14D1 + 199n] sar 24: Grain */
    if (g <= 2) {                                                   /* 0x27A59 jg 0x27BBC */
        /* the hand's value: sum of c * c * ((i + 1) >> 1); D: c is the byte +0x24 + i (0xF14ED + 199n + i, sar 24),
           the advances, not the cards +i (file comment: kept) */
        for (i = 1; i <= 0x12; i++) {                               /* 0x27A62..0x27A7C */
            c = S8(rec + 0x24u + (uint32_t)i);                      /* 0x27A81 / 0x27A94 */
            v += c * c * ((i + 1) >> 1);                            /* 0x27A9D..0x27AA9 */
        }
        /* used when the value is under 60, or still at least 60 after 4 * (byte +0x2C)^2 (D: Coinage, kept) */
        if (v < 0x3C                                                /* 0x27AAE jl 0x27AE1 */
            || v - 4 * S8(rec + 0x2Cu) * S8(rec + 0x2Cu) >= 0x3C) { /* 0x27AB4..0x27ADB jl 0x27C06 */
            Lc = S8(SEC_LOSS + (uint32_t)n);                        /* 0x27AE4 */
            if (Lc > 8)                                             /* 0x27AED jle */
                Lc = 8;
            if (4 * g > S8(SEC_LOSS + (uint32_t)n))                 /* 0x27B27 jg 0x27B3D */
                setD16(GRAIN_USED + 2u * (uint32_t)n,
                       (uint16_t)(D16(GRAIN_USED + 2u * (uint32_t)n) + (uint32_t)(Lc / 5 + 1)));   /* 0x27B4B..0x27B55 */
            else
                setD16(GRAIN_USED + 2u * (uint32_t)n,
                       (uint16_t)(D16(GRAIN_USED + 2u * (uint32_t)n) + (uint32_t)g));   /* 0x27B38, 0x27B50..0x27B55 */
            if (4 * g > S8(SEC_LOSS + (uint32_t)n))                 /* 0x27B84 jg 0x27B9D */
                setD8(SEC_LOSS + (uint32_t)n,
                      (uint8_t)(S8(SEC_LOSS + (uint32_t)n) - (4 * (Lc / 5) + 4)));   /* 0x27BAB..0x27BB8 */
            else
                setD8(SEC_LOSS + (uint32_t)n, (uint8_t)(S8(SEC_LOSS + (uint32_t)n) - 4 * g));   /* 0x27B98, 0x27BB3 */
        }
    } else if (S8(rec + 0x98u) < 2 * S8(rec + 0x51u) + 4) {         /* 0x27BBC..0x27BE8 jge 0x27BFD */
        setD16(GRAIN_USED + 2u * (uint32_t)n, (uint16_t)(D16(GRAIN_USED + 2u * (uint32_t)n) + 1u));   /* 0x27BEA inc word */
        setD8(SEC_LOSS + (uint32_t)n, (uint8_t)(D8(SEC_LOSS + (uint32_t)n) - 4u));   /* 0x27BF4 sub byte, 4 */
    } else {
        setD16(GRAIN_USED + 2u * (uint32_t)n, 0);                   /* 0x27BFF */
    }
    grain_boxes(n, buf, sizeof buf);                                /* 0x27C06..0x27CD1 */
    f78A20(0x9C4);                                                  /* 0x27CDB */
}

/* ==== 0x2E45C a computer's river among the tied flood plains ======================================================= */

int f2E45C(int n)
{
    char buf[100];       /* [ebp-0x84]..[ebp-0x21] */
    int32_t i;           /* [ebp-0x18] */
    int32_t j;           /* [ebp-0x1C] */
    int32_t p;

    setD32(0xBEDE8u, S32(PLAYERS_ALL) == 3 ? 1u : 0u);              /* 0x2E479..0x2E488: Iberia's river, Po in 3-player games */
    f3F670();                                                       /* 0x2E48D */
    for (i = 0; i < S32(CHOICE_N); i++) {                           /* 0x2E492..0x2E4A6 */
        p = LIST(i);
        if (p == S32(OWN_RIVER + 4u * (uint32_t)n))                 /* 0x2E4B9 cmp; je 0x2E497 */
            continue;
        if (g_original)   /* BUG(1995): "choses" (0xB35A7; text) */
            snprintf(buf, sizeof buf, "%s choses the %s to flood.", NAME(n),
                     cal2_river_text(DSTR(D32(PLAIN(p) + 4u))));    /* 0x2E4FD */
        else
            snprintf(buf, sizeof buf, "%s chooses the %s to flood.", NAME(n),
                     cal2_river_text(DSTR(D32(PLAIN(p) + 4u))));   /* "TIGRUS" fixed there (cal2.c) */
        for (j = 0; S16(PLAIN(p) + 2u) > j; j++)                    /* 0x2E505..0x2E535 */
            f3F624(S16(PLAIN(p) + 8u + 2u * (uint32_t)j), 15);      /* 0x2E564 */
        f79270(-500, n, -200, buf, 0);                              /* 0x2E580 */
        break;                                                      /* falls into 0x2E585 */
    }
    return LIST(i);                                                 /* 0x2E588: list[count] when none qualifies */
}

/* ==== 0x2C190 the computer's faction choice screen ================================================================== */

void f2C190(int x, int count, const char *text, int src, int target, int mode, int flag)
{
    setD16(0xBB400u, 0x15);                                         /* 0x2C1B7 */
    background3();                                                  /* 0x2C1BE..0x2C1EE */
    fg_vbopen(S32(BACKBUF));                                        /* 0x2C1FD */
    fg_setcolor(5);                                                 /* 0x2C207 */
    f3A3A0(0x12F, 0x176, DSTR(0xB335Fu));                           /* 0x2C21E "LEFT TO" */
    if (mode == 4)                                                  /* 0x2C223 */
        f3A044(0x150, 0x176, DSTR(0xB3367u));                       /* 0x2C256 "SELECT" */
    else if (mode == 5)                                             /* 0x2C233 */
        f3A044(0x150, 0x176, DSTR(0xB336Eu));                       /* "ANNEX" */
    else if (mode == 6)                                             /* 0x2C242 */
        f3A044(0x150, 0x176, DSTR(0xB3374u));                       /* "ANNEX" (a second copy) */
    fg_vbclose();                                                   /* 0x2C25B */
    f3AB7C(text);                                                   /* 0x2C263 */
    f7876C();                                                       /* 0x2C268 */
    f28CFC(src, mode, 0);                                           /* 0x2C275 */
    f2C298(x, count, src, target, (int8_t)flag);                    /* 0x2C27A movsx eax, byte; 0x2C28B */
}

/* ==== 0x2C298 the computer chooses a faction's units ================================================================ */

void f2C298(int x, int count, int src, int target, int flag)
{
    int32_t taken;       /* [ebp-0x30] cities taken */
    int32_t left;        /* [ebp-0x34] */
    uint8_t keep;        /* [ebp-0xC] */
    int32_t T;           /* [ebp-0x18] x's tokens over the list */
    int32_t need;        /* [ebp-0x2C] cities needed */
    int32_t retry;       /* [ebp-0x20] */
    int32_t i;           /* [ebp-0x28] */
    int32_t j;           /* [ebp-0x24] */
    int32_t sel;         /* [ebp-0x14] */
    int8_t pass;         /* [ebp-0x10] */
    int32_t tk;          /* [ebp-0x1C] */
    int32_t k;           /* [ebp-0x20] */
    int32_t e, cnt;

    taken = 0;                                                      /* 0x2C2B8 */
    f3C940(D32(CUR_DLG));                                           /* 0x2C2C0 */
    left = count;                                                   /* 0x2C2C8 */
    f2EE6C(x, -1, left);                                            /* 0x2C2D6 */
    if (S32(PLAYERS_ALL) > 3) {                                     /* 0x2C2DB jle 0x2C348: Iberia's rows */
        setD32(0xC0638u, 0x95); setD32(0xC063Cu, 0x97); setD32(0xC0640u, 0x8F); setD32(0xC0644u, 0x90);   /* 0x2C302.. */
        setD32(0xC06CCu, 0x96); setD32(0xC06D0u, 0x87); setD32(0xC06D4u, 0x83); setD32(0xC06D8u, 0x8E);
        setD32(0xC06DCu, 0x8D);                                     /* ..0x2C340 */
    } else {
        setD32(0xC0638u, 0x21); setD32(0xC063Cu, 0x7F); setD32(0xC0640u, 0x35); setD32(0xC0644u, 0x24);   /* 0x2C366.. */
        setD32(0xC06CCu, 0x13); setD32(0xC06D0u, 0x22); setD32(0xC06D4u, 0x34); setD32(0xC06D8u, 0x12);
        setD32(0xC06DCu, 0x07);                                     /* ..0x2C3A3 */
    }
    keep = (uint8_t)(count >= 0x14 && (int8_t)flag == 0);           /* 0x2C3A9..0x2C3BF */
    T = 0;                                                          /* 0x2C3C4 */
    for (i = 0; i < S32(CHOICE_N); i++)                             /* 0x2C3C7..0x2C3DB */
        T += S8(tok(LIST(i), x));                                   /* 0x2C3F9 [.. + 0xE31C3] sar 24 (x's column) */
    need = count - (T >= 0xA ? 0xA : T);                            /* 0x2C407..0x2C41C */
    need = need / 5 + (need % 5 > 0 ? 1 : 0);                       /* 0x2C42D / 0x2C43F idiv, 0x2C441..0x2C445 */
    retry = 0;                                                      /* 0x2C44B */

    /* cities first */
    for (i = 0; i < S32(CHOICE_N); i++) {                           /* 0x2C44E, 0x2C577..0x2C580 */
        if ((D8(TSTATE(LIST(i)) + 2u) & 1) != 0                     /* 0x2C595 test dh, 1 */
            && taken < need                                         /* 0x2C5A6 jge */
            && left >= 5) {                                         /* 0x2C5AC jl */
            sel = -1;                                               /* 0x2C5AE */
            if (retry != 0)                                         /* 0x2C5BA jne 0x2C4B0 */
                sel = LIST(i);
            else
                for (j = 0; j < 4; j++) {                           /* 0x2C5C0, 0x2C45C jge 0x2C4BD */
                    e = S32(AI_CITIES + 16u * (uint32_t)x + 4u * (uint32_t)j);   /* 0x2C47A [16x + 4j + 0xC0618] */
                    if (LIST(i) == e && keep) {                     /* 0x2C482, 0x2C488 */
                        sel = LIST(i);                              /* 0x2C4B0 */
                        break;
                    }
                    if (LIST(i) != e && !keep) {                    /* 0x2C4A8, 0x2C4AE */
                        sel = LIST(i);
                        break;
                    }
                }
            if (sel >= 0) {                                         /* 0x2C4BD jl 0x2C54E */
                f78F04(sel, src);                                   /* 0x2C4CD */
                f78E74(sel, target);                                /* 0x2C4D8 */
                left -= 5;                                          /* 0x2C4DD */
                taken++;                                            /* 0x2C4E4 */
                if (taken == 4)                                     /* 0x2C4EA */
                    taken = 9;                                      /* 0x2C4EF */
                cnt = S32(CHOICE_N);
                for (j = i; j < cnt; j++)                           /* 0x2C4F6..0x2C525: reads one past the count */
                    setD32(CHOICE + 4u * (uint32_t)j, D32(CHOICE + 4u * (uint32_t)(j + 1)));
                setD32(CHOICE_N, (uint32_t)(cnt - 1));              /* 0x2C527 */
                i--;                                                /* 0x2C533 */
                f2EE6C(x, sel, left);                               /* 0x2C53F */
                if (left < 1)                                       /* 0x2C548 jl 0x2C5C8 */
                    break;
            }
        }
        /* 0x2C54E: short of cities at the last candidate: once more, from candidate 1 */
        if (retry == 0 && S32(CHOICE_N) - 1 == i && taken < need) { /* 0x2C553, 0x2C55E, 0x2C566 */
            retry++;                                                /* 0x2C56B */
            i = 0;                                                  /* 0x2C56E (then i++, 0x2C574) */
        }
    }

    /* 0x2C5C8: then tokens, first by row x - 1 of 0xC06B8, then any */
    if (left > 0) {                                                 /* 0x2C5CC jle 0x2C765 */
        pass = 2;                                                   /* 0x2C5D2 */
        do {
            pass--;                                                 /* 0x2C5D9 */
            for (i = 0; i < S32(CHOICE_N); i++) {                   /* 0x2C5DE..0x2C5F2 */
                sel = -1;                                           /* 0x2C5F8 */
                if (pass != 0) {                                    /* 0x2C604 je 0x2C683 */
                    for (j = 0; j < 5; j++) {                       /* 0x2C60C, 0x2C617 jge 0x2C690 */
                        e = S32(AI_TOKENS + 20u * (uint32_t)(x - 1) + 4u * (uint32_t)j);   /* 0x2C643 */
                        if (LIST(i) == e && keep) {                 /* 0x2C649, 0x2C64D */
                            sel = LIST(i);                          /* 0x2C683 */
                            break;
                        }
                        if (LIST(i) != e && !keep) {                /* 0x2C67B, 0x2C67D */
                            sel = LIST(i);
                            break;
                        }
                    }
                } else {
                    sel = LIST(i);                                  /* 0x2C683 */
                }
                if (sel < 0)                                        /* 0x2C690 jl 0x2C5E3 */
                    continue;
                tk = S8(tok(sel, x));                               /* 0x2C6AD [.. + 0xE31C3] sar 24 (x's column) */
                if (tk > 0) {                                       /* 0x2C6BE jle 0x2C74A */
                    for (k = 0; k < tk; k++) {                      /* 0x2C6C6..0x2C6D7 */
                        left--;                                     /* 0x2C6DC */
                        f57CE8(sel, src, 0, 1);                     /* 0x2C6EC */
                        dec8(NATION(src) + 0x98u);                  /* 0x2C6F8 */
                        inc8(NATION(src) + 0x99u);                  /* 0x2C705 */
                        f57E28(sel, target, 1);                     /* 0x2C716 */
                        inc8(NATION(target) + 0x98u);               /* 0x2C722 */
                        dec8(NATION(target) + 0x99u);               /* 0x2C72F */
                        if (left == 0)                              /* 0x2C73A jne 0x2C6CB */
                            break;
                    }
                    f2EE6C(x, sel, left);                           /* 0x2C745 */
                }
                if (left == 0) {                                    /* 0x2C74E jne 0x2C5E3 */
                    pass = 0;                                       /* 0x2C756 */
                    break;
                }
            }
        } while (pass > 0);                                         /* 0x2C759 movsx eax, byte; jg 0x2C5D6 */
    }
    f3C8C4(D32(CUR_DLG));                                           /* 0x2C76A */
}

/* ==== 0x2C894 the computer takes a faction's Military losses ======================================================== */

/* one token of faction n off territory t (0x2CCBB, 0x2CF49): f57CE8(t, n, 0, 1); BUG(1995) fix 2 (file comment): the
   faction's board - 1 and stock + 1 as 0x27FCC's (the original leaves both) */
static void faction_token_off(int32_t t, int n)
{
    f57CE8(t, n, 0, 1);
    if (!g_original) {   /* BUG(1995): the faction's counts were not updated (file comment, 2) */
        dec8(NATION(n) + 0x98u);
        inc8(NATION(n) + 0x99u);
    }
}

void f2C894(int n, int count, char *text, int ov, int flag, int mode)
{
    int32_t left;        /* [ebp-0x28] */
    uint8_t exact;       /* [ebp-0xC] a city is exactly what is left */
    int32_t i;           /* [ebp-0x2C] */
    int32_t j;           /* [ebp-0x20] */
    uint8_t core;        /* [ebp-0x10] */
    uint8_t want;        /* [ebp-0x14] */
    int32_t changed;     /* [ebp-0x18] */
    int32_t tk;          /* [ebp-0x24] */
    int32_t t, b;

    background3();                                                  /* 0x2C8B2..0x2C8EC */
    fg_vbopen(S32(BACKBUF));                                        /* 0x2C8F6 */
    fg_setcolor(5);                                                 /* 0x2C900 */
    f3A3A0(0x12F, 0x176, DSTR(0xB337Au));                           /* 0x2C917 "LEFT TO" */
    f3A044(0x150, 0x176, DSTR(0xB3382u));                           /* 0x2C92B "REMOVE" */
    fg_vbclose();                                                   /* 0x2C930 */
    f3AB7C(text);                                                   /* 0x2C938 */
    f7876C();                                                       /* 0x2C93D */
    f3B05C(PANEL_DLG);                                              /* 0x2C947 */
    f3C940(D32(CUR_DLG));                                           /* 0x2C951 */
    f28CFC(n, mode, 0);                                             /* 0x2C95E */
    left = count;                                                   /* 0x2C966 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x2C970 */
    f3C9D0(0x109, 0x186, 0x6E, 0x13, 5);                            /* 0x2C98E */
    fg_setcolor(5);                                                 /* 0x2C995 */
    f3C9D0(0x136, 0x172, 0x13, 0x13, 5);                            /* 0x2C9B0 */
    fg_setcolor(5);                                                 /* 0x2C9B7 */
    snprintf(text, 12, "%d", left);                                 /* 0x2C9CA itoa(left, text, 10): the caller's buffer */
    f3A0B0(0x136, 0x177, 0x13, text);                               /* 0x2C9E1 */
    fg_vbclose();                                                   /* 0x2C9E6 */
    f3F670();                                                       /* 0x2C9EB */
    f3D52C(0, 0);                                                   /* 0x2C9F4 */
    f3CBBC();                                                       /* 0x2C9F9 */
    f78A20(step_wait());                                            /* 0x2C9FE..0x2CA1D */
    exact = (uint8_t)((ov < 0 ? -ov : ov) + left == 5);             /* 0x2CA22..0x2CA38 cdq; xor; sub; sete */

    /* pass 1: by the AI rows (for n 12 / 13 past the 10-nation tables: risk 8, byte-exact) */
    for (i = 0; i < S32(CHOICE_N) && left > ov; i++) {              /* 0x2CA3D, 0x2CCE5..0x2CCFA */
        t = LIST(i);
        if ((int32_t)D8(TSTATE(t)) != n && D8(tok(t, n)) == 0)      /* 0x2CD1E je 0x2CD48; 0x2CD3F je 0x2CCDF */
            continue;
        want = (uint8_t)((int8_t)flag != 0);                        /* 0x2CD48..0x2CD56 */
        changed = 0;                                                /* 0x2CD5B */
        if ((D8(TSTATE(t) + 2u) & 1) != 0                           /* 0x2CD71 */
            && (int32_t)D8(TSTATE(t)) == n                          /* 0x2CD9C */
            && exact != 0) {                                        /* 0x2CDA5 */
            core = 0;                                               /* 0x2CDB1 */
            for (j = 0; j < 4; j++)                                 /* 0x2CDB4, 0x2CA4B jge 0x2CA77 */
                if (t != S32(AI_CITIES + 16u * (uint32_t)n + 4u * (uint32_t)j)) {   /* 0x2CA69 cmp; je 0x2CA45 */
                    core = 1;                                       /* 0x2CA73 */
                    break;
                }
            if (core == want) {                                     /* 0x2CA7A jne 0x2CAA0 */
                f78F04(t, n);                                       /* 0x2CA90 */
                left -= 5;                                          /* 0x2CA95 */
                changed = 1;                                        /* 0x2CA99 */
            }
        } else if (D8(tok(t, n)) != 0 && exact == 0) {              /* 0x2CBFE..0x2CC11 */
            core = 0;                                               /* 0x2CC17 (dh = 0) */
            for (j = 0; j < 5; j++)                                 /* 0x2CC1A..0x2CC2B */
                if (t == S32(AI_TOKENS + 20u * (uint32_t)(n - 1) + 4u * (uint32_t)j)) {   /* 0x2CC55 jne 0x2CC21 */
                    core = 1;                                       /* 0x2CC59 */
                    break;
                }
            if (core == want) {                                     /* 0x2CC60 jne 0x2CAA0 */
                tk = S8(tok(t, n));                                 /* 0x2CC8E */
                while (tk > 0 && left > ov) {                       /* 0x2CC91..0x2CCA1 */
                    faction_token_off(t, n);                        /* 0x2CCBB */
                    tk--;                                           /* 0x2CCC3 */
                    left--;                                         /* 0x2CCC9 */
                    changed = 1;                                    /* 0x2CCCC */
                }
            }
        }
        if (changed == 0)                                           /* 0x2CAA0 je 0x2CCDF */
            continue;
        b = D8(AREA(t + 1));                                        /* 0x2CAF9 the area record's byte +0 */
        fg_vbopen(S32(BACKBUF));                                    /* 0x2CAB1 */
        f3C9D0(0x109, 0x186, 0x6E, 0x13, 5);                        /* 0x2CACF */
        fg_setcolor(5);                                             /* 0x2CAD6 */
        f3A0B0(0x109, 0x18A, 0x6E, DSTR(D32(0xC77D4u + 4u * (uint32_t)b)));   /* 0x2CB11 */
        f3C9D0(0x136, 0x172, 0x13, 0x13, 5);                        /* 0x2CB29 */
        fg_setcolor(5);                                             /* 0x2CB30 */
        snprintf(text, 12, "%d", left);                             /* 0x2CB43 itoa */
        f3A0B0(0x136, 0x176, 0x14, text);                           /* 0x2CB5A */
        fg_vbclose();                                               /* 0x2CB5F */
        f3F670();                                                   /* 0x2CB64 */
        f3F624(b - 1, 15);                                          /* 0x2CB8F */
        f3DA60(b - 1);                                              /* 0x2CBB3 */
        f3CBBC();                                                   /* 0x2CBB8 */
        f78A20(step_wait());                                        /* 0x2CBBD..0x2CCDA */
    }

    /* 0x2CDBE pass 2: anything */
    fg_vbopen(S32(BACKBUF));                                        /* 0x2CDC9 */
    f3C9D0(0x109, 0x186, 0x6E, 0x13, 5);                            /* 0x2CDE7 */
    fg_setcolor(5);                                                 /* 0x2CDEE */
    if (g_original || S32(CHOICE_N) > 0)   /* BUG(1995): an empty list shows entry -1, "PIRACY" (file comment, 4) */
        f3A0B0(0x109, 0x18A, 0x6E, TERR_NAME(LIST(0)));             /* 0x2CE16 */
    f3C9D0(0x136, 0x172, 0x13, 0x13, 5);                            /* 0x2CE2E */
    fg_setcolor(5);                                                 /* 0x2CE35 */
    snprintf(text, 12, "%d", left);                                 /* 0x2CE48 itoa */
    f3A0B0(0x136, 0x176, 0x13, text);                               /* 0x2CE5F */
    fg_vbclose();                                                   /* 0x2CE64 */
    f3F670();                                                       /* 0x2CE69 */
    if (g_original || S32(CHOICE_N) > 0) {   /* BUG(1995): f3F624(-1, 15) and f3DA60(-1) on an empty list (4) */
        f3F624(LIST(0), 15);                                        /* 0x2CE7D */
        f3DA60(LIST(0));                                            /* 0x2CE8C */
    } else {
        f3D52C(0, 0);   /* the map as it is (cal1.c's bug 6) */
    }
    f3CBBC();                                                       /* 0x2CE91 */
    if (left > ov) {                                                /* 0x2CE99 jle 0x2D0F6 */
        f78A20(step_wait());                                        /* 0x2CEA2..0x2CEC1 */
        if (left > ov) {                                            /* 0x2CECC jle 0x2D0F6 */
            for (i = 0; i < S32(CHOICE_N) && left > ov; i++) {      /* 0x2CED2, 0x2D065..0x2D07A */
                changed = 0;                                        /* 0x2D07E */
                t = LIST(i);
                if ((D8(TSTATE(t) + 2u) & 1) != 0                   /* 0x2D09A je 0x2CEDC */
                    && (int32_t)D8(TSTATE(t)) == n                  /* 0x2D0C4 */
                    && exact != 0) {                                /* 0x2D0CE */
                    f78F04(t, n);                                   /* 0x2D0E1 */
                    left -= 5;                                      /* 0x2D0E6 */
                    changed = 1;                                    /* 0x2D0EA */
                } else if (D8(tok(t, n)) != 0 && exact == 0) {      /* 0x2CEFC, 0x2CF04 */
                    while (S8(tok(t, n)) > 0 && left > ov) {        /* 0x2CF06..0x2CF33 (the board re-read) */
                        faction_token_off(t, n);                    /* 0x2CF49 */
                        left--;                                     /* 0x2CF51 */
                        changed = 1;                                /* 0x2CF54 */
                    }
                }
                if (changed == 0)                                   /* 0x2CF5D je 0x2D05F */
                    continue;
                fg_vbopen(S32(BACKBUF));                            /* 0x2CF6E */
                f3C9D0(0x109, 0x186, 0x6E, 0x13, 5);                /* 0x2CF8C */
                fg_setcolor(5);                                     /* 0x2CF93 */
                f3A0B0(0x109, 0x18A, 0x6E, TERR_NAME(t));           /* 0x2CFBB */
                f3C9D0(0x136, 0x172, 0x13, 0x13, 5);                /* 0x2CFD3 */
                fg_setcolor(5);                                     /* 0x2CFDA */
                snprintf(text, 12, "%d", left);                     /* 0x2CFED itoa */
                f3A0B0(0x136, 0x176, 0x14, text);                   /* 0x2D004 */
                fg_vbclose();                                       /* 0x2D009 */
                f3F670();                                           /* 0x2D00E */
                f3F624(t, 15);                                      /* 0x2D022 */
                f3DA60(t);                                          /* 0x2D031 */
                f3CBBC();                                           /* 0x2D036 */
                f78A20(step_wait());                                /* 0x2D03B..0x2D05A */
            }
        }
    }
    f3C8C4(D32(CUR_DLG));                                           /* 0x2D0FB */
}

/* ==== 0x2D10C a computer beneficiary annexes cities ================================================================ */

void f2D10C(int v, int k, int o, int b)
{
    char buf[100];       /* [ebp-0x80]..[ebp-0x1D] */
    int32_t r;
    int32_t t;           /* [ebp-0x10] */

    while (k != 0) {                                                /* 0x2D12A je 0x2D1B8 */
        r = f20C70(b, o);                                           /* 0x2D13A [AICAL1] */
        t = r - 1;                                                  /* 0x2D13F */
        setD8(AREA(r) + 0x3Bu + (uint32_t)v, 1);                    /* 0x2D154 [69r + v + 0xE31F3] */
        f78F04(t, o);                                               /* 0x2D161 */
        f78E74(t, b);                                               /* 0x2D16C */
        k--;                                                        /* 0x2D174 */
        snprintf(buf, sizeof buf, "%s annexes the city in %s", NAME(b), TERR_NAME(t));   /* 0x2D196 (0xB3389) */
        f79270(0x5DC, b, t, buf, 1);                                /* 0x2D1AE */
    }
}

/* ==== 0x2D1C0 a computer beneficiary annexes tokens ================================================================ */

void f2D1C0(int v, int k, int o, int b)
{
    char buf[100];       /* [ebp-0x94]..[ebp-0x31] */
    int32_t lst[0xA0];   /* [ebp-0x314]..[ebp-0x95] (160 entries, up to buf): o's areas, territory + 1 */
    int32_t cnt;         /* [ebp-0xC] */
    int32_t i;           /* [ebp-0x28] (the copy: [ebp-0x24]) */
    int32_t j;           /* [ebp-0x24] */
    int32_t flag;        /* [ebp-0x18] 1: the areas from the first table match on, 0: those before it */
    int32_t match;       /* [ebp-0x14] reset per pass only */
    int32_t tk;          /* [ebp-0x1C] */
    int32_t t = 0;       /* [ebp-0x20] */
    int32_t moved;

    cnt = 0;                                                        /* 0x2D1E2 */
    for (i = 0; S8(NATION(o) + 0x60u) > i && cnt < 0xA0; i++)      /* 0x2D1E5..0x2D203 (port bound; int8 +0x60 <= 127) */
        lst[cnt++] = D8(NATION(o) + 0x61u + (uint32_t)i);           /* 0x2D20F..0x2D220 */
    flag = 1;                                                       /* 0x2D229 */
    while (k != 0) {                                                /* 0x2D230 je 0x2D3D9 */
        match = 0;                                                  /* 0x2D23C */
        moved = 0;
        for (i = 0; i < cnt && k > 0; i++) {                        /* 0x2D23F, 0x2D3B7..0x2D3C3 */
            for (j = 0; j < 5; j++)                                 /* 0x2D3C7, 0x2D24D jge 0x2D286 */
                if (lst[i] == S32(AI_TOKENS + 20u * (uint32_t)(v - 1) + 4u * (uint32_t)j)) {   /* 0x2D27B jne 0x2D247 */
                    match = 1;                                      /* 0x2D27F */
                    break;
                }
            if (match != flag)                                      /* 0x2D289 jne 0x2D3B1 */
                continue;
            tk = S8(AREA(lst[i]) + 0x0Eu + (uint32_t)o);            /* 0x2D2AB [69r + o + 0xE31C3] sar 24 */
            for (j = 0; j < tk && k > 0; j++) {                     /* 0x2D2B7..0x2D2D4 */
                setD8(AREA(lst[i]) + 0x3Bu + (uint32_t)v, 1);       /* 0x2D2F3 */
                t = lst[i] - 1;                                     /* 0x2D305 */
                f57CE8(t, o, 0, 1);                                 /* 0x2D315 */
                k--;                                                /* 0x2D31D */
                dec8(NATION(o) + 0x98u);                            /* 0x2D327 */
                inc8(NATION(o) + 0x99u);                            /* 0x2D334 */
                f57E28(t, b, 1);                                    /* 0x2D345 */
                inc8(NATION(b) + 0x98u);                            /* 0x2D351 */
                dec8(NATION(b) + 0x99u);                            /* 0x2D35E */
                moved++;
            }
            if (tk > 0) {                                           /* 0x2D369 jle 0x2D3B1 */
                snprintf(buf, sizeof buf, "%s annexes tokens in %s", NAME(b), TERR_NAME(t));   /* 0x2D391 (0xB33A3) */
                f79270(0x5DC, b, t, buf, 1);                        /* 0x2D3AC */
            }
        }
        if (!g_original && flag == 0 && moved == 0)   /* BUG(1995): a pass 0 that moves nothing never ends (3) */
            break;
        flag = 0;                                                   /* 0x2D3CF */
    }
}
