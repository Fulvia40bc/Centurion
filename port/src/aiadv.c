/* Module ADVAI of docs/M6_PLAN.md: the computer players' side of ACQUIRE CIVILIZATION (phase 18), M7 batch 2.
   Literal translations; the address of each instruction a line comes from is in its comment. D = read in the code,
   G = inference. Prototypes and results: aiadv.h.

     0x177EC  f177EC  which advance to buy (one game_rand, and f1B45C's)                    1,776 bytes
     0x19700  f19700  the box "%s has acquired ..."                                          1,296
     0x1A888  f1A888  the desirability of each advance                                       3,028
     0x1B45C  f1B45C  buy anyway (one game_rand when the score beats the cost)                 120
     0x1B59C  f1B59C  the cards that pay for the advances (the 18-digit odometer)            2,156
     0x1BE08  f1BE08  one hand scored for f1B59C                                             1,236
   Callers: adv.c's computer_turn (0x16D98..0x175DF of 0x162D0) calls f1A888, f177EC, f1B59C and f19700 (it carries
   static not_ported stubs of these four names today; Claude links them). Ported callees: f78AE4 game_rand, f17760
   (game.h), f76F14 (info.c), f1B538 / f1B4D4 / f1C2DC / f19448 (formulas.c), the UI kit (ui.h, kit.h, game.h), fg.c,
   junk.c. No call into another batch-2 module. No PBEM branch in these six (D: no test of [0xE1624] & 0x80):
   PBEM_DROPPED: none.

   Random numbers (M6_PLAN risk 2): f177EC draws once (0x17DC1) on its weighted path, also when the candidate count is
   1 (the second jump table sends count 1 to 0x17C2C after the draw) and when it is 0 (both tables skipped); the
   special case of the human leader (0x17C2C reached from 0x17C34) draws nothing. Then the final test calls f1B45C only
   when f76F14(n) <= 8, f1B538(n) == 0 and both goal flags are clear (short-circuit, D 0x17E8A..0x17EB2), and f1B45C
   draws only when the cost is below the score (0x1B48D). No other draw (D: the call scan).

   Watcom runtime -> standard C:
     0x7D5D0 memset (0xF62EC, 0x60 bytes; f1B59C's 0x48-byte flag block) -> memset
     0x7DB3D sprintf("%s has acquired \n") and the inline strcat loops (repne scasb + copy) of 0x19700 -> snprintf /
             cat() into the 300 bytes from [ebp-0x16C] up to the locals at [ebp-0x40]
     rep movsd / movsw / movsb record n -> record 0 (0x1B5DC) and the 32-byte list copies (0x1B896) -> memmove on g_ds
     0x8CE50 fg_makegif / 0x88F98 fg_showgif "junk" -> junk_makegif / junk_showgif (junk.h: kept in memory)
   __CHK dropped; no debug stub 0x78B0C / 0x78B24 and no trace or progress string in these functions (D).

   Data (D, read in the code at the addresses given):
     nation n = 0xF14CC + 199n: +0x01..+0x12 int8 commodity cards, +0x24 + i uint8 advance i held (+0x26 Metalworking,
       +0x28 Roadbuilding, +0x29 Mining, +0x2B Astronomy, +0x2C Coinage, +0x35 Military, +0x37..+0x3B the religious
       and philosophical ones: offsets as the code adds them, names G from adv.c's table), +0x3E int8 treasury, +0x51
       int8 cities, +0x5C int8 the AST square, +0x60 int8 / +0x61.. the area list (area index = territory + 1),
       +0x98 int8 tokens on the board, +0x99 int8 stock, +0x9A 0 out / 2 computer / 3 human
     area record r = 0xE31B8 + 69r (territory r - 1): +1 int8 the nations present (dword [0xE31B6 + 69r] sar 24)
     territory record t = 0xE1638 + 35t: +1 flags (bit 0x40 read by 0x1A888's Architecture case, 0x1AFE9)
     the land lists [0xF8E7C + 4t] (port heap handles; int16 count, int16 territories), read by 0x1A888 as
       [0xF8E78 + 4r] for area r (= territory r - 1)
     0xF52E0 int32[10][10] the attitudes, [0xF52D4] the leading nation, [0xF40EC] / [0xF40F0] the players, [0xF4898]
       the token supply, byte 0xF5934 + n (the save's "surplus removed" bytes), bytes 0x109E24 + 9n + j (the 9 x 9
       "met" matrix of the save, written by 0x5C1BA with 0-based rows; risk-8 item below), the credits 0xF6764[24]
       (read at 0xF678C [10], 0xF67A4 [16], 0xF67AC [18]), the states int8 0xF67C4[24]
     tables (obj2): int16 0xBD480 costs, int16 0xBD510 group masks (bit 0 crafts .. 4 religion), the AI table 0xBCCE2 +
       85(n-1) + 5 * square (+0 the groups goal, +2 the points goal / 100), int32 0xBC800[k] = 1 << k
     f1BE08's list 0xF68CC, 18 entries of 32 bytes: +0x00 the value used, +0x04 count, +0x08 commodity id, +0x0C 1 =
       counted with Mining, +0x10 Mining-eligible (ids 4, 12, 11, 16, 18), +0x14 face (id+1) >> 1, +0x18 c*c*face,
       +0x1C (c+1)^2*face; count [0xF6D4C]; the best copy 0xF6B0C, count [0xF6D50]
     the Mining list of f194E4: int32 [0xF68B0] the choice, [0xF68B4] the count, int8 0xF68B8[] the commodities

   Stack words read before they are written (batch 1's rule: a stale word of a sibling frame is modelled; only what
   cannot be predicted is fixed, in both modes). Words 1 and 2 are not bugs (noted after the list); fixed:
     3. 0x1BE08's [ebp-0x20], the last entry of the prefix that reaches need, is written in the prefix loop only; with
        no commodity in record 0 (count 0: n holds no commodity card and pays from its treasury alone) the loop does
        not run and the word is stale: it is [ebp-0x114] of 0x162D0's callees (0x1B59C's ebp; 0x1BE08's frame sits
        below 0x1B59C's 0xDC bytes and its two stack arguments). No routine computer_turn calls before 0x1B59C
        reaches that depth (D, frames checked: 0x17EDC with 0x195D8 / 0x3C284 / 0x3BEA0, 0x1A888 with 0x1C2DC,
        0x177EC, 0x176BC, 0x194E4, 0x3B914, and 0x1B59C's own 0x1C2DC all end above it). So it holds what the
        previous player's turn left there: the previous 0x1BE08's e, then possibly bytes 88..91 of 0x19700's text
        buffer [ebp-0x16C], or words of the drawing routines' fg-library frames (0x1818C, 0x19170, the human
        screen; G: deep enough), which the port's fg.c does not reproduce: it cannot be modelled. Its use: the masks (1 << (e+1)) - 1, which select no entry with count 0, and, on the
        excess path (min(shortfall, treasury) > need), the walk back over entries e-1..0, which would read stale list
        entries. BUG(1995), fixed in both modes: e = 0 (no walk; the excess is min(shortfall, treasury) - need).
   Noted, not bugs (stale words 1 and 2, as the code's comments number them; nothing to fix):
     (1) 0x1A888's [ebp-0x30], the nation of the highest attitude m (written only for a nation in play whose
        0xF52E0[n][k] beats -5000, 0x1A956), is read by the Military case (0x1B176). n itself is in play and
        0x7C960 writes 0xF52E0[n][n] = 0 (D, aimove.c), so the word is always written (G: unreachable). Its stale value
        would be the return address of 0x17EDC's last call (0x17EDC, called from the same frame just before, ends
        its frame at [ebp-0x2C]: a relocated code address, times 199 as a nation record), which a port cannot model:
        the port's local starts at 0 (both modes; no effect while the word is written).
     (2) 0x1B59C's out-words [ebp-0x34] / [ebp-0x30] (*excess, *mask) are written by 0x1BE08 only when the hand reaches
        need (0x1C0C4 / 0x1C0D5). A later call that does not reach it leaves the previous call's words, which are >=
        the best already kept, so nothing happens (the port keeps them as f1B59C locals across the calls, as the
        original). The first call always reaches need (G from D: computer_turn buys only advances 0x17EDC found
        affordable with hand + treasury + credits, and need = the costs less the credits used; 0x1BE08's full hand
        with its best Mining choice is f195D8's value): the port starts *excess at INT32_MAX, which is never read.
   Deterministic out-of-range reads (M6_PLAN risk 8), kept byte-exact through the data object (D code):
     - 0x1A888's three slips: (1) the "another nation present" test computes the address 0xE31B8 + 69r + 2 and tests
       it for zero (0x1AAF5, 0x1AB59): always true, so the test is dropped (no read is made); (2) the attitude
       average a takes the last value read (mov at 0x1A914), not a sum, divided by [0xF40EC]; (3) the Metalworking
       loop reads nation records 0..8 (0x1AA6E, record 0 the scratch, never nation 9) where its next loops use k + 1.
     - 0x1A888 reads the matrix 0x109E24 as row n (dword [9n + j + 0x109E21] sar 24, j 0..8: Metalworking 0x1AA61,
       Roadbuilding 0x1AD36, Astronomy 0x1ADD8) where 0x5C1BA writes 0-based rows (G: row n - 1 meant): for n == 9
       the row runs past the 81 saved bytes into 0x109E75..0x109E7D, which hold the round label the loop prints at
       0x109E7C ("%d" of the round, D loop.h): nation 9 sees "met" at j == 7. Read through the data object, as is.
     - 0x177EC with no candidate (G: unreachable, computer_turn calls it only when 0x17EDC found one) passes -1 to
       0x1B45C, which reads cost int16 [0xBD47E] and score [0xF62E8].
     - 0x19700 saves the enabled byte of every record of the current dialog into int8 0xF8E44[i] without a bound (52
       records would reach [0xF8E78] itself; G: the dialog 0xF634C has fewer).
   Port guards (no change in play): divisions by [0xF40EC] / [0xF4898] give 0 for a zero divisor (the original
   faults; G: both set for any game in progress); reads at addresses computed from data values ([0xF52D4], the
   area lists) give 0 outside the data object; the land lists read 0 for a handle that is neither a heap handle nor a
   data-object address (as aiplan.c).

   1995 bugs (graded code / effect). In the planner an odd weight is not a bug (the three slips above, the Military
   case's +100 for the first human only, the fallback flags of 0x177EC that make a cut-back goal a reason to buy):
   kept. Fixed unless g_original:
     1. 0x19700 (text slip, M6_PLAN "Text slips"): with three or more advances the separator after the LAST name is
        " and \n" and the one before it ", \n" (the test at 0x197E3 is count - 1 > i, meant count - 2 > i):
        "Pottery, \nLaw, \nMusic and \n.". adv.c's BUG(1995) 4 lets a computer buy three, so the default passes 3
        here: fixed, ", \n" up to the last but one, " and \n" before the last, nothing after it. D code, G reach
        (with --original adv.c never passes more than 2).
   Also fixed: item 3 of the stale words above, in both modes (so it is listed there, not here).
   Text: "%s has acquired \n" (the space before the line break) and "CONTINUE" stay (Erik's ruling: style).
   Easter eggs / oddities (none is a joke): the box saves the screen to the "junk" GIF (M6_PLAN egg 22, known); it
   relabels the shared CONTINUE of 0xBBBEC "CONTINUE" again, which undoes the "NOT REVEAL GDS" that REVEAL GOODS
   leaves (egg 17's sticky label: new, D 0x19AD2); 0x177EC's special case (new, D 0x17BE9): a computer facing a human
   leader who holds Military and whose attitude to it exceeds 500 stops weighing and takes its top-scored advance;
   0x1A888's national flavour (new, D): Egypt +50 and Babylon +35 for Engineering (case 6), Architecture favoured by
   nations 1 and 4, Clothmaking by the seafaring 1 and 5 and only with exactly 3 players for nation 2. */

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "aiadv.h"
#include "fg.h"
#include "formulas.h"
#include "game.h"
#include "info.h"
#include "junk.h"
#include "kit.h"
#include "state.h"
#include "ui.h"

#define NATION(n)    (0xF14CCu + 199u * (uint32_t)(n))
#define REC0         0xF14CCu
#define NAME(n)      DSTR(D32(0xC768Cu + 4u * (uint32_t)(n)))
#define ADV_NAME(i)  DSTR(D32(0xC7DC4u + 4u * (uint32_t)(int32_t)(i)))
#define COST(i)      ((int32_t)S16(0xBD480u + 2u * (uint32_t)(int32_t)(i)))   /* dword [2i + 0xBD47E] sar 16 */
#define GROUP(i)     ((int32_t)S16(0xBD510u + 2u * (uint32_t)(int32_t)(i)))   /* dword [2i + 0xBD50E] sar 16 */
#define SC(i)        (0xF62ECu + 4u * (uint32_t)(int32_t)(i))                 /* int32 the desirability */
#define STATE(i)     ((int32_t)S8(0xF67C4u + (uint32_t)(int32_t)(i)))         /* dword [i + 0xF67C1] sar 24 */
#define CNT          0xF7C9Cu    /* int32 the candidate count */
#define LIST(i)      (0xF7CA0u + 4u * (uint32_t)(int32_t)(i))                 /* int32 the candidates */
#define REL(a, b)    (0xF52E0u + 40u * (uint32_t)(a) + 4u * (uint32_t)(b))
#define LEADER       0xF52D4u
#define PLAYERS_A    0xF40ECu
#define PLAYERS_B    0xF40F0u
#define SUPPLY       0xF4898u
#define NEIGH(n, j)  (0x109E24u + 9u * (uint32_t)(n) + (uint32_t)(j))      /* dword [9n + j + 0x109E21] sar 24 */
#define AREA(r)      (0xE31B8u + 69u * (uint32_t)(r))
#define TERR(t)      (0xE1638u + 35u * (uint32_t)(t))
#define BIT(k)       D32(0xBC800u + 4u * (uint32_t)(int32_t)(k))             /* int32 1 << k */
#define SHORT_TREAS  0xF6748u    /* int32 min(shortfall, treasury) */
#define TREASURY     0xF674Cu    /* int32 */
#define PAY_TREAS    0xF6750u    /* int32 */
#define MINING_CHOICE 0xF68B0u   /* int32 */
#define MINING_COUNT 0xF68B4u    /* int32 */
#define MINING_LIST  0xF68B8u    /* int8[]: dword [j + 0xF68B5] sar 24 */
#define ENT(k)       (0xF68CCu + 32u * (uint32_t)(int32_t)(k))   /* f1BE08's list */
#define ENTCNT       0xF6D4Cu
#define BEST(k)      (0xF6B0Cu + 32u * (uint32_t)(int32_t)(k))   /* the best copy */
#define BESTCNT      0xF6D50u
#define CUR_DIALOG   0xF8E78u
#define SAVED_EN     0xF8E44u
#define BOX_DLG      0xBBBECu
#define BACKBUF      0xF52C8u
#define PICTURE_NOW  0xF52D0u    /* int8 */
#define SRD          0xF40BAu    /* int16 the self-running demo's players */

/* ---- small helpers ---------------------------------------------------------------------------------------------- */

static int32_t div32(int32_t a, int32_t b)   /* idiv (port guard: 0 for b == 0, where the original faults) */
{
    if (b == 0)
        return 0;
    if (b == -1)
        return (int32_t)(0u - (uint32_t)a);
    return a / b;
}

static int32_t safe_s8(uint32_t a)   /* a byte at an address computed from data values (port guard: 0 outside) */
{
    return ds_valid(a, 1) ? (int32_t)S8(a) : 0;
}

static int32_t safe_s32(uint32_t a)
{
    return ds_valid(a, 4) ? S32(a) : 0;
}

/* one int16 of a land list (movsx word [h]): a heap block (port handle) or, for an out-of-range territory, the data
   object (as aiplan.c's land_word) */
static int32_t land_word(uint32_t h)
{
    const uint8_t *p = load_heap_ptr(h, 2);
    if (p != NULL)
        return (int16_t)(p[0] | p[1] << 8);
    if (ds_valid(h, 2))
        return S16(h);
    return 0;   /* port guard */
}

static void add8(uint32_t a, int v) { setD8(a, (uint8_t)(D8(a) + (uint8_t)v)); }   /* byte inc / dec / add / sub */

/* ==== 0x1B45C: buy anyway ========================================================================================== */

int f1B45C(int i)
{
    int32_t r = 0;                                                      /* [ebp-0xC] 0x1B473 */
    int32_t d;
    uint32_t x;

    if (COST(i) < S32(SC(i))) {                                         /* 0x1B479..0x1B48D */
        d = S32(SC(i)) - COST(i);                                       /* 0x1B492..0x1B4AB ebx - edx */
        x = (uint32_t)f78AE4();                                         /* 0x1B4A6 game_rand */
        x = (x * 100u) >> 15;                                           /* 0x1B4AD..0x1B4BC (25 * 4, shr) */
        if ((int32_t)x < d)                                             /* 0x1B4BF jge */
            r = 1;                                                      /* 0x1B4C3 */
    }
    return r;
}

/* ==== 0x177EC: which advance to buy ================================================================================= */

/* the candidate list: every advance in state 2 (0x17829 / 0x179BD / 0x17B0E) */
static void all_candidates(void)
{
    int32_t i, c;
    setD32(CNT, 0);                                                     /* 0x1781E / 0x179B3 / 0x17B03 */
    for (i = 0; i <= 0x17; i++) {
        if (STATE(i) != 2)
            continue;
        c = S32(CNT);
        setD32(CNT, (uint32_t)(c + 1));
        setD32(LIST(c), (uint32_t)i);
    }
}

/* the entries set to -1 squeezed out in place (0x17954 / 0x17AAF) */
static void compact(void)
{
    int32_t i, k = 0;                                                   /* [ebp-0x58], [ebp-0x54] */
    for (i = 0; i < S32(CNT); i++) {
        if (S32(LIST(i)) == -1)
            continue;
        setD32(LIST(k), D32(LIST(i)));
        k++;
    }
    setD32(CNT, (uint32_t)k);                                           /* 0x1799D / 0x17AF2 */
}

int f177EC(int n)
{
    uint32_t rec = NATION(n);                                           /* [ebp-0x18] 0x17819 */
    int32_t choice = -1;                                                /* [ebp-0x24] 0x17804 */
    int32_t i, j;                                                       /* [ebp-0x58], [ebp-0x54] */
    int32_t mask, mask0, groups;                                        /* [ebp-0x50], [ebp-0x4C], [ebp-0x2C] */
    int32_t cut1, fb1, points, need, cut2, fb2;                         /* [ebp-0x44], -0x40, -0x34, -0x30, -0x3C, -0x38 */
    int32_t s[4] = { 0, 0, 0, 0 };                                      /* [ebp-0x68..-0x5C] (read only up to the count) */
    int32_t total, r, t, sum01;                                         /* [ebp-0x1C], [ebp-0x28], [ebp-0x20], edx */
    int32_t pos = S8(rec + 0x5Cu);                                      /* dword [rec + 0x59] sar 24 */
    uint32_t row = (uint32_t)(5 * pos + 85 * (n - 1));                  /* 5 (pos + 1) + 85 (n - 1) - 5 */

    all_candidates();                                                   /* 0x1781E..0x17861 */

    mask = 0;                                                           /* 0x17865 */
    for (i = 0; i <= 0x17; i++)                                         /* 0x1786D */
        if (D8(rec + 0x24u + (uint32_t)i) != 0)                         /* 0x1787F test dl, dl */
            mask |= GROUP(i);                                           /* 0x17893 */
    mask0 = mask;                                                       /* 0x1789B */
    groups = 0;                                                         /* 0x178A0 */
    for (i = 0; i < 5; i++) {                                           /* 0x178A8 */
        if ((mask & 1) != 0)                                            /* 0x178B7 */
            groups++;
        mask >>= 1;                                                     /* 0x178C1 sar (arithmetic) */
    }
    mask = mask0;                                                       /* 0x178C9 */

    cut1 = 0;                                                           /* 0x178CE */
    if ((int32_t)S8(0xBCCE2u + row) > groups) {                         /* 0x178FA [ebx + eax + 0xBCCDA] sar 24; jle */
        cut1 = 1;                                                       /* 0x17909 */
        for (i = 0; i < S32(CNT); i++)                                  /* 0x17915 */
            if ((GROUP(S32(LIST(i))) | mask) == mask)                   /* 0x17930..0x17942: no new group */
                setD32(LIST(i), 0xFFFFFFFFu);                           /* 0x17947 */
    }
    compact();                                                          /* 0x17954..0x179A0 */
    fb1 = 0;                                                            /* 0x179A7 */
    if (S32(CNT) == 0)                                                  /* 0x179AA */
        all_candidates();                                               /* 0x179B3..0x179F5 */
    else if (cut1 != 0)                                                 /* 0x179F7 */
        fb1 = 1;                                                        /* 0x179FE */

    points = f17760(n);                                                 /* 0x17A08 */
    need = (int32_t)S8(0xBCCE4u + row) * 100;                           /* 0x17A39 [edx + eax + 0xBCCDC] sar 24; *100 */
    cut2 = 0;                                                           /* 0x17A59 */
    if (need > points) {                                                /* 0x17A5F jle */
        cut2 = 1;                                                       /* 0x17A64 */
        for (i = 0; i < S32(CNT); i++)                                  /* 0x17A70 */
            if (COST(S32(LIST(i))) + points < need)                     /* 0x17A8B..0x17A9D jge */
                setD32(LIST(i), 0xFFFFFFFFu);                           /* 0x17AA2 */
    }
    compact();                                                          /* 0x17AAF..0x17AF5 */
    fb2 = 0;                                                            /* 0x17AFC */
    if (S32(CNT) == 0)                                                  /* 0x17AFF */
        all_candidates();                                               /* 0x17B03..0x17B46 */
    else if (cut2 != 0)                                                 /* 0x17B48 */
        fb2 = 1;                                                        /* 0x17B4F */

    for (i = S32(CNT) - 2; i >= 0; i--)                                 /* 0x17B56 bubble sort, descending */
        for (j = 0; j <= i; j++)                                        /* 0x17B85 */
            if (S32(SC(S32(LIST(j + 1)))) > S32(SC(S32(LIST(j))))) {    /* 0x17BA7..0x17BB5 jle */
                t = S32(LIST(j + 1));                                   /* 0x17BBC */
                setD32(LIST(j + 1), D32(LIST(j)));                      /* 0x17BD3 */
                setD32(LIST(j), (uint32_t)t);                           /* 0x17BE0 */
            }

    {
        int32_t lead = S32(LEADER);                                     /* 0x17BE9 ebx */
        uint32_t lrec = NATION(lead);
        if (safe_s8(lrec + 0x9Au) == 3                                  /* 0x17BF5 dword [eax + 0xF1563] sar 24 */
            && safe_s8(lrec + 0x35u) != 0                               /* 0x17C03 byte [eax + 0xF1501] */
            && safe_s32(REL(n, lead)) > 500) {                          /* 0x17C1F [4 lead + 8 * 5n + 0xF52E0]; jle */
            choice = S32(LIST(0));                                      /* 0x17C2C */
            goto L_17E8A;
        }
    }

    if (S32(CNT) > 4)                                                   /* 0x17C39 */
        setD32(CNT, 4);                                                 /* 0x17C44 */
    switch ((uint32_t)(S32(CNT) - 1)) {                                 /* 0x17C4E..0x17C78 table 0x177CC */
    case 0:                                                             /* 0x17C80 */
        s[0] = S32(SC(S32(LIST(0))));
        break;
    case 1:                                                             /* 0x17C9C */
        s[0] = S32(SC(S32(LIST(0)))) * 2;
        s[1] = S32(SC(S32(LIST(1))));
        break;
    case 2:                                                             /* 0x17CBD: + 66 % and + 33 % */
        t = S32(SC(S32(LIST(0))));
        s[0] = t + div32(t * 66, 100);                                  /* 0x17CBD..0x17CE0 */
        t = S32(SC(S32(LIST(1))));
        s[1] = t + div32(t * 33, 100);                                  /* 0x17CE3..0x17CFF */
        s[2] = S32(SC(S32(LIST(2))));                                   /* 0x17D02 */
        break;
    case 3:                                                             /* 0x17D18: + 50 %, + 30 %, + 20 % */
        t = S32(SC(S32(LIST(0))));
        s[0] = t + div32(t * 50, 100);                                  /* 0x17D18..0x17D42 */
        t = S32(SC(S32(LIST(1))));
        s[1] = t + div32(t * 30, 100);                                  /* 0x17D45..0x17D64 */
        t = S32(SC(S32(LIST(2))));
        s[2] = t + div32(t * 20, 100);                                  /* 0x17D67..0x17D88 */
        s[3] = S32(SC(S32(LIST(3))));                                   /* 0x17D8B */
        break;
    default:                                                            /* 0x17D9A */
        break;
    }
    total = 0;                                                          /* 0x17D9C */
    for (i = 0; i < S32(CNT); i++)                                      /* 0x17DA4 */
        total += s[i];                                                  /* 0x17DBC */
    r = (int32_t)(((uint32_t)f78AE4() * (uint32_t)total) >> 15);        /* 0x17DC1 game_rand; imul; shr 0xF */

    sum01 = s[0] + s[1];                                                /* 0x17DDF edx (s[1] unread for count 1) */
    switch ((uint32_t)(S32(CNT) - 1)) {                                 /* 0x17DD0..0x17DE5 table 0x177DC */
    case 0:                                                             /* 0x17C2C */
        choice = S32(LIST(0));
        break;
    case 1:                                                             /* 0x17DED */
        choice = r < s[0] ? S32(LIST(0)) : S32(LIST(1));
        break;
    case 2:                                                             /* 0x17E09 */
        if (r < s[0])
            choice = S32(LIST(0));                                      /* 0x17E11 */
        else if (r >= s[0] && sum01 > r)                                /* 0x17E1E jl, 0x17E22 jle */
            choice = S32(LIST(1));                                      /* 0x17E27 */
        else
            choice = S32(LIST(2));                                      /* 0x17E31 */
        break;
    case 3:                                                             /* 0x17E3B */
        if (r < s[0])
            choice = S32(LIST(0));                                      /* 0x17E43 */
        else if (r >= s[0] && sum01 > r)                                /* 0x17E50 jl, 0x17E54 jle */
            choice = S32(LIST(1));                                      /* 0x17E59 */
        else if (s[0] + s[1] <= r && s[0] + s[1] + s[2] > r)            /* 0x17E69 jg, 0x17E79 jle */
            choice = S32(LIST(2));                                      /* 0x17E7B */
        else
            choice = S32(LIST(3));                                      /* 0x17E82 */
        break;
    default:                                                            /* -> 0x17E8A, choice -1 */
        break;
    }

L_17E8A:
    if (f76F14(n) > 8                                                   /* 0x17E8D; 0x17E92 jg */
        || f1B538(n) != 0                                               /* 0x17E9A */
        || fb1 != 0                                                     /* 0x17EA3 */
        || fb2 != 0                                                     /* 0x17EA9 */
        || f1B45C(choice) != 0                                          /* 0x17EB2 */
        || f1B4D4(n) != 0)                                              /* 0x17EBE */
        return choice;                                                  /* 0x17EC7 */
    return -1;                                                          /* 0x17ECC */
}

/* ==== 0x1A888: the desirability of each advance ===================================================================== */

/* nation j's advance byte at +off ("cmp byte [199j + 0xF14CC + off], 0") */
static int held(int32_t j, uint32_t off)
{
    return D8(NATION(j) + off) != 0;
}

static int in_play(int32_t j)   /* byte [199j + 0xF1566] (+0x9A) != 0 */
{
    return D8(NATION(j) + 0x9Au) != 0;
}

void f1A888(int n)
{
    uint32_t rec = NATION(n);                                           /* [ebp-0x18] 0x1A8AE */
    int32_t nm1 = n - 1;                                                /* [ebp-0x4C] 0x1A8B5 */
    int32_t m = -5000;                                                  /* [ebp-0x34] 0x1A8C9 */
    int32_t a = 0;                                                      /* [ebp-0x28] 0x1A8D2 */
    int32_t mk = 0;                                                     /* [ebp-0x30]: stale-word item 1 (port: 0) */
    int32_t i;                                                          /* [ebp-0x54] the advance */
    int32_t j, k, l, v, t, u, c, h, x;                                  /* [ebp-0x60], [ebp-0x5C], [ebp-0x58], [ebp-0x50] */
    int32_t mx, who, mil, seen;                                         /* [ebp-0x44], -0x40, -0x3C; [ebp-0x2C] */

    memset(DP(0xF62ECu), 0, 0x60);                                      /* 0x1A8C4 memset(0xF62EC, 0, 0x60) */
    for (j = 1; j <= 9; j++) {                                          /* 0x1A8DE */
        if (!in_play(j))                                                /* 0x1A8F1 */
            continue;
        a = S32(REL(n, j));                                             /* 0x1A90D slip (2): the last value read */
        if (S32(REL(n, j)) > m) {                                       /* 0x1A929..0x1A935 jle */
            m = S32(REL(n, j));                                         /* 0x1A950 */
            mk = j;                                                     /* 0x1A956 */
        }
    }
    a = div32(a, S32(PLAYERS_A));                                       /* 0x1A95B..0x1A96C idiv [0xF40EC] */

    for (i = 0; i <= 0x17; i++) {                                       /* 0x1A99C, 0x1A9A2 jg */
        if (D8(rec + 0x24u + (uint32_t)i) != 0)                         /* 0x1A9B6 byte [199n + i + 0xF14F0] */
            continue;
        if (STATE(i) != 2)                                              /* 0x1A9C0..0x1A9CF */
            continue;
        switch (i) {                                                    /* 0x1A9D9 table 0x1A828 */
        case 0:                                                         /* 0x1A976 */
            v = 0x2D;
            if (S32(LEADER) == n && m < -200)                           /* 0x1A982, 0x1A987 jge */
                v += 0x32;
            setD32(0xF62ECu, (uint32_t)v);                              /* 0x1A997 */
            break;
        case 1:                                                         /* 0x1A9E1 */
            v = 0x2D;
            switch ((uint32_t)(n - 1)) {                                /* 0x1A9EC table 0x1A7BC */
            case 0: v += 0x1E; break;                                   /* 0x1A9F9 */
            case 1: if (S32(PLAYERS_B) == 3) v += 0xA; break;           /* 0x1A9FF */
            case 2: case 3: v -= 0xA; break;                            /* 0x1AA0E */
            case 4: v += 0x32; break;                                   /* 0x1AA14 */
            case 7: case 8: v -= 0x14; break;                           /* 0x1AA1A */
            default: break;                                             /* 0x1AA1E (5, 6 and out of range) */
            }
            if (held(n, 0x2Bu))                                         /* 0x1AA21 */
                v += 0xA;
            setD32(0xF62F0u, (uint32_t)v);                              /* 0x1AA2E */
            break;
        case 2:                                                         /* 0x1AA38 */
            v = 0x50;
            for (j = 0; j < 9; j++)                                     /* 0x1AA46; slip (3): record j, not j + 1 */
                if (S8(NEIGH(n, j)) != 0 && held(j, 0x26u)) {           /* 0x1AA61, 0x1AA75 */
                    v += 0x50;                                          /* 0x1AA7F */
                    break;
                }
            for (j = 0; S8(rec + 0x60u) > j; j++) {                     /* 0x1AA8A, 0x1AA90 [rec + 0x5D] sar 24 */
                t = D8(rec + 0x61u + (uint32_t)j);                      /* 0x1AAAA [ebp-0x48] */
                if (S8(AREA(t) + 1u) > 1) {                             /* 0x1AABA [t + 68t + 0xE31B6] sar 24; jle */
                    for (k = 0; k < 9; k++) {                           /* 0x1AAD0 */
                        if (k == nm1)                                   /* 0x1AAE2 */
                            continue;
                        /* 0x1AAE6..0x1AAFD slip (1): tests the address 0xE31B8 + 69t + 2 for zero: never */
                        if (held(k + 1, 0x26u)) {                       /* 0x1AB09 */
                            v += 0x28;                                  /* 0x1AB13 */
                            j = 0x64;                                   /* 0x1AB17 */
                            break;
                        }
                    }
                }
                h = (int32_t)D32(0xF8E78u + 4u * (uint32_t)t);          /* 0x1AB21 [4t + 0xF8E78] (= territory t - 1) */
                for (l = 1; land_word((uint32_t)h) >= l; l++) {         /* 0x1AB2B, 0x1AB8C jl */
                    u = land_word((uint32_t)h + 2u * (uint32_t)l) + 1;  /* 0x1ABA3 movsx; inc: [ebp-0x1C] */
                    if (S8(AREA(u) + 1u) <= 1)                          /* 0x1ABB4..0x1ABC1 */
                        continue;
                    for (k = 0; k < 9; k++) {                           /* 0x1AB34 */
                        if (k == nm1)                                   /* 0x1AB46 */
                            continue;
                        /* 0x1AB4A..0x1AB61 slip (1) again: never zero */
                        if (held(k + 1, 0x26u)) {                       /* 0x1AB6D */
                            v += 0x28;                                  /* 0x1AB77 */
                            j = 0x64;                                   /* 0x1AB80 */
                            l = 0x64;                                   /* 0x1AB83 */
                            break;
                        }
                    }
                }
            }
            if (held(n, 0x35u))                                         /* 0x1ABD0 */
                for (j = 1; j <= 9; j++)                                /* 0x1ABDF */
                    if (!held(j, 0x35u))                                /* 0x1ABF2 (in play or not) */
                        v += 0x14;
            if (held(n, 0x2Bu))                                         /* 0x1AC05 */
                v += 0x1E;
            if (held(n, 0x28u))                                         /* 0x1AC12 */
                v += 0xA;
            setD32(0xF62F4u, (uint32_t)v);                              /* 0x1AC1F */
            break;
        case 3:                                                         /* 0x1AC29 */
            v = 0x6E;
            switch ((uint32_t)(n - 1)) {                                /* 0x1AC34 table 0x1A7E0 */
            case 0: case 4: v += 0x32; break;                           /* 0x1AC41 */
            case 7: case 8: v -= 0x14; break;                           /* 0x1AC47 */
            default: break;                                             /* 0x1AC4B */
            }
            if (S8(0xF5934u + (uint32_t)n) > 0)                         /* 0x1AC4E dword [n + 0xF5931] sar 24; jle */
                v += 0x1E;
            setD32(0xF62F8u, (uint32_t)v);                              /* 0x1AC62 */
            break;
        case 4:                                                         /* 0x1AC6C */
            v = 0x8C;
            if (n == 1)                                                 /* 0x1AC7B jbe */
                v -= 0x14;                                              /* 0x1AD09 */
            else if (n == 5)                                            /* 0x1AC84 */
                v -= 0x1E;                                              /* 0x1AD12 */
            if (a > 0xC8) {                                             /* 0x1AC8A jle */
                x = div32((a - 0xC8) * 40, 0x12C) + 0x3C;               /* 0x1AC93..0x1ACB7 [ebp-0x24] */
                if (x > 0x64)                                           /* 0x1ACBD */
                    x = 0x64;
                v += x;                                                 /* 0x1ACCC */
            }
            if (div32((int32_t)S8(rec + 0x98u) * 100, S32(SUPPLY)) > 0x4B)   /* 0x1ACCF..0x1ACF9 jle */
                v += 0x64;                                              /* 0x1ACFE */
            for (j = 0; j < 9; j++)                                     /* 0x1AD1B */
                if (S8(NEIGH(n, j)) != 0) {                             /* 0x1AD36 */
                    v += 0x19;                                          /* 0x1AD43 */
                    break;
                }
            setD32(0xF62FCu, (uint32_t)v);                              /* 0x1AD4A */
            break;
        case 5:                                                         /* 0x1AD54 */
            v = 0xB4;
            if (held(n, 0x2Cu))                                         /* 0x1AD5E */
                v += 0x28;
            setD32(0xF6300u, (uint32_t)v);                              /* 0x1AD6B */
            break;
        case 6:                                                         /* 0x1AD75 */
            v = 0x8C;
            if (n == 8)                                                 /* 0x1AD84 jbe */
                v += 0x23;                                              /* 0x1AD9E */
            else if (n == 9)                                            /* 0x1AD89 */
                v += 0x32;                                              /* 0x1AD98 */
            setD32(0xF6304u, (uint32_t)v);                              /* 0x1AD8E / 0x1ADA5 */
            break;
        case 7:                                                         /* 0x1ADAF */
            v = 0x50;
            for (j = 0; j < 9; j++)                                     /* 0x1ADBD */
                if (S8(NEIGH(n, j)) != 0) {                             /* 0x1ADD8 */
                    v += 0x14;                                          /* 0x1ADE5 */
                    break;
                }
            if (n == 1)                                                 /* 0x1ADF1 jbe */
                v += 0x28;                                              /* 0x1AE12 */
            else if (n == 5)                                            /* 0x1ADF6 */
                v += 0x32;                                              /* 0x1AE18 */
            if (held(n, 0x35u))                                         /* 0x1ADFB */
                v += 0x3C;
            setD32(0xF6308u, (uint32_t)v);                              /* 0x1AE08 */
            break;
        case 8:                                                         /* 0x1AE1E */
            v = 0x6E;
            if (held(n, 0x29u))                                         /* 0x1AE28 */
                v += 0x28;
            if (f1C2DC(n) > 0)                                          /* 0x1AE35; jle */
                v += 0x28;
            setD32(0xF630Cu, (uint32_t)v);                              /* 0x1AE45 */
            break;
        case 9:                                                         /* 0x1AE4F */
            v = 0x8C;
            if (held(n, 0x28u))                                         /* 0x1AE59 */
                v += 0x1E;
            if (m <= 0x64)                                              /* 0x1AE63 jg */
                v += 0x14;
            if (m >= 0x12C)                                             /* 0x1AE6D jl */
                v -= 0x14;
            if (S32(LEADER) == n)                                       /* 0x1AE7F */
                v += 0x32;
            setD32(0xF6310u, (uint32_t)v);                              /* 0x1AE8B */
            break;
        case 10:                                                        /* 0x1AE95 */
            v = 0xB4;
            if (S32(0xF678Cu) >= 0x64)                                  /* 0x1AE9C the credit 0xF6764[10]; jl */
                v += 0x32;
            setD32(0xF6314u, (uint32_t)v);                              /* 0x1AEAC */
            break;
        case 11:
        case 12:                                                        /* 0x1AEB6 */
            v = 0x3C;
            mx = 0;                                                     /* 0x1AEBF */
            who = 0;                                                    /* 0x1AEC2 */
            mil = 0;                                                    /* 0x1AEC5 */
            for (j = 1; j <= 9; j++) {                                  /* 0x1AED1 */
                if (!in_play(j))                                        /* 0x1AEE4 */
                    continue;
                if (S8(NATION(j) + 0x99u) > mx) {                       /* 0x1AEF5 [199j + 0xF1562] sar 24; jle */
                    mx = S8(NATION(j) + 0x99u);                         /* 0x1AF15 */
                    who = j;                                            /* 0x1AF1B */
                }
                if (held(j, 0x35u))                                     /* 0x1AF25 */
                    mil = 1;                                            /* 0x1AF2E */
            }
            if (who == n)                                               /* 0x1AF3A */
                v -= 0x1E;
            c = (int32_t)S8(rec + 0x51u) * 5 + S8(rec + 0x98u);         /* 0x1AF46..0x1AF61 */
            if (c > 0x23)                                               /* 0x1AF63 jle */
                v += c - 0x23;                                          /* 0x1AF68 */
            if (mil != 0)                                               /* 0x1AF6E */
                v += 0xA;
            if (held(n, 0x35u))                                         /* 0x1AF7B */
                v += 0x14;
            if (held(n, 0x28u))                                         /* 0x1AF88 */
                v += 0x14;
            setD32(SC(i), (uint32_t)v);                                 /* 0x1AF98 [4i + 0xF62EC] */
            break;
        case 13:                                                        /* 0x1AFA4 */
            v = 0x78;
            c = 0;                                                      /* [ebp-0x38] 0x1AFAD */
            for (j = 0; S8(rec + 0x60u) > j; j++) {                     /* 0x1AFB5, 0x1AFBB */
                t = D8(rec + 0x61u + (uint32_t)j);                      /* 0x1AFD1 [ebp-0x48] */
                if ((D8(TERR(t - 1) + 1u) & 0x40u) != 0)                /* 0x1AFE9 [35 (t-1) + 0xE1639]; test dl, 0x40 */
                    c++;
            }
            if (c < 9)                                                  /* 0x1AFFC jge */
                v += (9 - c) * 25;                                      /* 0x1B002..0x1B018 */
            if (n == 1)                                                 /* 0x1B023 jbe */
                v += 0x14;                                              /* 0x1B037 */
            else if (n == 4)                                            /* 0x1B028 */
                v += 0x1E;                                              /* 0x1B03D */
            setD32(0xF6320u, (uint32_t)v);                              /* 0x1B02D / 0x1B044 */
            break;
        case 14:                                                        /* 0x1B04E */
            setD32(0xF6324u, 0x46);
            break;
        case 15:                                                        /* 0x1B05D */
            v = 0xAA;
            v -= 0x1E;                                                  /* 0x1B064 */
            if (m <= 0x64)                                              /* 0x1B068 jg */
                v += 0x1E;
            if (held(n, 0x35u))                                         /* 0x1B075 */
                v += 0x14;
            if (held(n, 0x28u))                                         /* 0x1B082 */
                v += 0x28;
            if (held(n, 0x3Au))                                         /* 0x1B08F */
                v += 0x14;
            setD32(0xF6328u, (uint32_t)v);                              /* 0x1B09C */
            break;
        case 16:                                                        /* 0x1B0A6 */
            v = 0xC8;
            v -= 0x28;                                                  /* 0x1B0AD */
            if (m <= 0x64)                                              /* 0x1B0B1 jg */
                v += 0x28;
            if (S32(0xF67A4u) >= 0x64)                                  /* 0x1B0BB the credit 0xF6764[16]; jl */
                v += 0x28;
            if (held(n, 0x35u))                                         /* 0x1B0CB */
                v += 0x14;
            if (held(n, 0x28u))                                         /* 0x1B0D8 */
                v += 0x14;
            setD32(0xF632Cu, (uint32_t)v);                              /* 0x1B0E5 */
            break;
        case 17:                                                        /* 0x1B0EF */
            v = 0xB4;
            seen = 0;                                                   /* 0x1B0F8 */
            for (j = 1; j <= 9; j++) {                                  /* 0x1B142, 0x1B148 */
                if (!in_play(j))                                        /* 0x1B15B */
                    continue;
                if (!held(j, 0x35u))                                    /* 0x1B16C */
                    v += 0x14;                                          /* 0x1B104 */
                else
                    v -= 0x14;                                          /* 0x1B170 */
                if (held(j, 0x26u))                                     /* 0x1B10F */
                    v += 0xA;
                if (S8(NATION(j) + 0x9Au) == 3 && seen == 0) {          /* 0x1B12C [199j + 0xF1563] sar 24; 0x1B131 */
                    seen = 1;                                           /* 0x1B137 */
                    v += 0x64;                                          /* 0x1B13E */
                }
            }
            if (mk != n && held(mk, 0x35u))                             /* 0x1B179, 0x1B185 */
                v += 0x32;
            if (held(n, 0x26u))                                         /* 0x1B195 */
                v += 0x1E;
            if (held(n, 0x2Bu))                                         /* 0x1B1A2 */
                v += 0x1E;
            if (held(n, 0x28u))                                         /* 0x1B1AF */
                v += 0x14;
            switch ((uint32_t)(n - 1)) {                                /* 0x1B1BD table 0x1A804 */
            case 0: case 4: v += 0x14; break;                           /* 0x1B1CA */
            case 7: case 8: v -= 0x14; break;                           /* 0x1B1D0 */
            default: break;                                             /* 0x1B1D4 */
            }
            setD32(0xF6330u, (uint32_t)v);                              /* 0x1B1D7 */
            break;
        case 18:                                                        /* 0x1B1E1 */
            v = 0xA0;
            if (S32(0xF67ACu) >= 0x78)                                  /* 0x1B1E8 the credit 0xF6764[18]; jl */
                v += 0x50;
            if (held(n, 0x28u))                                         /* 0x1B1F8 */
                v += 0x14;
            if (held(n, 0x3Au))                                         /* 0x1B205 */
                v += 0x14;
            setD32(0xF6334u, (uint32_t)v);                              /* 0x1B212 */
            break;
        case 19:                                                        /* 0x1B21C */
            v = 0x32;
            if (held(n, 0x38u))                                         /* 0x1B226 */
                v -= 0xA;
            if (held(n, 0x39u))                                         /* 0x1B233 */
                v -= 0x1E;
            setD32(0xF6338u, (uint32_t)v);                              /* 0x1B240 */
            break;
        case 20:                                                        /* 0x1B24A */
            v = 0x50;
            if (held(n, 0x37u))                                         /* 0x1B254 */
                v -= 0xA;
            if (held(n, 0x39u))                                         /* 0x1B261 */
                v -= 0x1E;
            setD32(0xF633Cu, (uint32_t)v);                              /* 0x1B26E */
            break;
        case 21:                                                        /* 0x1B278 */
            v = 0x96;
            if (held(n, 0x37u))                                         /* 0x1B282 */
                v -= 0xA;
            if (held(n, 0x38u))                                         /* 0x1B28F */
                v -= 0x1E;
            if (held(n, 0x35u))                                         /* 0x1B29C */
                v += 0x14;
            for (j = 1; j <= 9; j++) {                                  /* 0x1B2AF */
                if (!in_play(j))                                        /* 0x1B2C8 */
                    continue;
                if (held(j, 0x39u))                                     /* 0x1B2D9 byte [199j + 0xF1505] */
                    v += 0x1E;
                if (held(j, 0x3Au))                                     /* 0x1B2E8 byte [199j + 0xF1506] */
                    v += 0x28;
            }
            setD32(0xF6340u, (uint32_t)v);                              /* 0x1B2FA */
            break;
        case 22:                                                        /* 0x1B304 */
            v = 0xDC;
            if (held(n, 0x28u))                                         /* 0x1B30E */
                v += 0xA;
            if (held(n, 0x2Bu))                                         /* 0x1B31B */
                v += 0x14;
            if (held(n, 0x35u))                                         /* 0x1B328 */
                for (j = 1; j <= 9; j++) {                              /* 0x1B337 */
                    if (!in_play(j))                                    /* 0x1B350 */
                        continue;
                    if (!held(j, 0x35u))                                /* 0x1B361 */
                        v += 0xA;
                }
            for (j = 1; j <= 9; j++) {                                  /* 0x1B374 */
                if (!in_play(j))                                        /* 0x1B38D */
                    continue;
                if (!held(j, 0x39u))                                    /* 0x1B39E */
                    v -= 0x14;
                if (!held(j, 0x3Au))                                    /* 0x1B3AD */
                    v += 0x28;
                if (!held(j, 0x3Bu))                                    /* 0x1B3C1 byte [199j + 0xF1507] */
                    v -= 0x28;
            }
            setD32(0xF6344u, (uint32_t)v);                              /* 0x1B3D3 */
            break;
        case 23:                                                        /* 0x1B3DD */
            v = 0xFA;
            if (held(n, 0x28u))                                         /* 0x1B3E7 */
                v += 0x14;
            if (held(n, 0x3Au))                                         /* 0x1B3F4 */
                v -= 0x64;
            if (!held(n, 0x3Au))                                        /* 0x1B401 */
                for (j = 1; j <= 9; j++) {                              /* 0x1B410 */
                    if (!in_play(j))                                    /* 0x1B429 */
                        continue;
                    if (held(j, 0x3Au))                                 /* 0x1B43A */
                        v += 0x32;
                }
            setD32(0xF6348u, (uint32_t)v);                              /* 0x1B447 */
            break;
        default:
            break;
        }
    }
}

/* ==== 0x1BE08: one hand scored ======================================================================================== */

static int32_t ev(int32_t k, uint32_t off) { return S32(ENT(k) + off); }   /* [32k + 0xF68CC + off] */

static uint32_t shl1(int32_t c) { return 1u << ((uint32_t)c & 31u); }        /* shl reg, cl (count & 31) */

int f1BE08(int n, int need, int32_t *excess, int32_t *mask, int32_t *exact, int first)
{
    const uint32_t rec0 = REC0;                                         /* [ebp-0x4] 0x1BE26 */
    int32_t i;                                                          /* [ebp-0x30] */
    int32_t mine;                                                       /* [ebp-0x8] the Mining entry */
    int32_t j;                                                          /* [ebp-0x2C] */
    int32_t sum, sum2;                                                  /* [ebp-0x28], [ebp-0x24] */
    int32_t e = 0;                                                      /* [ebp-0x20]: stale-word item 3 (BUG(1995)) */
    int32_t found, m, lo, d;                                            /* [ebp-0x10], [ebp-0xC], [ebp-0x18] */
    uint32_t cur;                                                       /* [ebp-0x14] */
    int32_t c, k;
    uint32_t a;

    (void)n;                                                            /* [ebp-0x40] never read */
    (void)first;                                                        /* [ebp+0x14] never read */
    setD32(ENTCNT, 0);                                                  /* 0x1BE34 */
    for (i = 1; i <= 0x12; i++) {                                       /* 0x1BEBE, 0x1BEC4 jg */
        if ((int32_t)S8(rec0 + (uint32_t)i) <= 0)                       /* 0x1BED4 movsx; jle */
            continue;
        a = ENT(S32(ENTCNT));                                           /* 0x1BEE9 [0xF6D4C] << 5 */
        setD32(a + 0x04u, (uint32_t)(int32_t)S8(rec0 + (uint32_t)i));   /* 0x1BEEF */
        setD32(a + 0x08u, (uint32_t)i);                                 /* 0x1BEF8 */
        if (i == 4 || i == 0xC || i == 0xB || i == 0x10 || i == 0x12)   /* 0x1BF01..0x1BF1C */
            setD32(a + 0x10u, 1);                                       /* 0x1BF2A */
        else
            setD32(a + 0x10u, 0);                                       /* 0x1BE44 */
        setD32(a + 0x14u, (uint32_t)((i + 1) >> 1));                    /* 0x1BE58 inc; sar */
        c = S8(rec0 + (uint32_t)i);
        setD32(a + 0x18u, (uint32_t)(((i + 1) >> 1) * (c * c)));        /* 0x1BE64..0x1BE7C */
        setD32(a + 0x00u, D32(a + 0x18u));                              /* 0x1BE88 */
        if (D32(a + 0x10u) != 0)                                        /* 0x1BE8E */
            setD32(a + 0x1Cu, (uint32_t)(((i + 1) >> 1) * ((c + 1) * (c + 1))));   /* 0x1BE92..0x1BEB2 */
        setD32(ENTCNT, D32(ENTCNT) + 1u);                               /* 0x1BEB8 */
    }

    mine = -1;                                                          /* 0x1BF39 */
L_1BF40:
    if (D8(rec0 + 0x29u) != 0) {                                        /* 0x1BF43 Mining */
        for (i = mine + 1; i < S32(ENTCNT); i++) {                      /* 0x1BF4D, 0x1BF5C */
            mine = -1;                                                  /* 0x1BF6B */
            if (ev(i, 0x10u) == 0)                                      /* 0x1BF7E */
                continue;
            mine = i;                                                   /* 0x1BF85 */
            setD32(ENT(i) + 0x0Cu, 1);                                  /* 0x1BF8E */
            setD32(ENT(i), D32(ENT(i) + 0x1Cu));                        /* 0x1BFA4..0x1BFAF */
            for (j = 1; j < S32(MINING_COUNT); j++)                     /* 0x1BFB5, 0x1BFC0 jge */
                if ((int32_t)S8(MINING_LIST + (uint32_t)j) == ev(i, 0x08u))   /* 0x1BFD4 [j + 0xF68B5] sar 24 */
                    setD32(MINING_CHOICE, (uint32_t)j);                 /* 0x1BFEA */
            break;                                                      /* 0x1BFC9 -> 0x1BFF1 */
        }
    }

    sum = 0;                                                            /* 0x1BFF3 */
    for (i = 0; i < S32(ENTCNT); i++) {                                 /* 0x1BFFB, 0x1C001 */
        sum += ev(i, 0);                                                /* 0x1C018 */
        if (sum + S32(TREASURY) >= need) {                              /* 0x1C01B..0x1C029 jl */
            e = i;                                                      /* 0x1C02E */
            break;
        }
    }
    if (sum + S32(TREASURY) < need) {                                   /* 0x1C031..0x1C03D jge */
        if (mine == -1)                                                 /* 0x1C042 */
            return 0;
        if (S32(ENTCNT) - 1 <= mine)                                    /* 0x1C04B..0x1C054 jle */
            return 0;
        setD32(ENT(mine) + 0x0Cu, 0);                                   /* 0x1C062 */
        setD32(ENT(mine), D32(ENT(mine) + 0x18u));                      /* 0x1C074..0x1C07A */
        goto L_1BF40;                                                   /* 0x1C080 */
    }

    if (sum + S32(SHORT_TREAS) <= need) {                               /* 0x1C085..0x1C095 jg */
        *exact = (int32_t)(shl1(e + 1) - 1u);                           /* 0x1C097..0x1C0A6 */
        return 1;
    }
    found = 0;                                                          /* 0x1C0B4 */
    *excess = sum + S32(SHORT_TREAS) - need;                            /* 0x1C0B7..0x1C0C4 */
    *mask = (int32_t)(shl1(e + 1) - 1u);                                /* 0x1C0C6..0x1C0D5 */
    for (i = e - 1; i >= 0; i--) {                                      /* 0x1C0D7, 0x1C0E0, 0x1C0E6 jl */
        sum -= ev(i, 0);                                                /* 0x1C0FC */
        if (sum + S32(TREASURY) > need && sum + S32(SHORT_TREAS) > need) {   /* 0x1C10D jle, 0x1C11D jle */
            d = sum + S32(SHORT_TREAS) - need;                          /* 0x1C11F..0x1C129 */
            if (d < *excess) {                                          /* 0x1C130 jge */
                *excess = d;                                            /* 0x1C141 */
                *mask = (int32_t)(shl1(e) | (shl1(i) - 1u));            /* 0x1C143..0x1C161 */
            }
            continue;
        }
        if (sum + S32(TREASURY) >= need && sum + S32(SHORT_TREAS) <= need) {   /* 0x1C176 jl, 0x1C184 jg */
            *exact = (int32_t)(shl1(e) | (shl1(i) - 1u));               /* 0x1C186..0x1C1A4 */
            return 1;
        }
        sum += ev(i, 0);                                                /* 0x1C1BF */
        m = (int32_t)(shl1(e) | (shl1(i + 1) - 1u));                    /* 0x1C1C2..0x1C1DA [ebp-0x1C] */
        lo = (int32_t)shl1(e);                                          /* 0x1C1DD..0x1C1E7 [ebp-0x18] */
        for (; m >= lo; m--) {                                          /* 0x1C247, 0x1C24D jl */
            sum2 = sum;                                                 /* 0x1C258 */
            cur = (uint32_t)m;                                          /* 0x1C25E */
            for (k = 0; S32(ENTCNT) - 1 > k; k++)                       /* 0x1C1F2, 0x1C1F8 jle */
                if ((cur & BIT(k)) == 0)                                /* 0x1C210 */
                    sum2 -= ev(k, 0);                                   /* 0x1C220 */
            if (sum2 + S32(TREASURY) >= need && sum2 + S32(SHORT_TREAS) <= need) {   /* 0x1C231 jl, 0x1C23F jg */
                found = m;                                              /* 0x1C244 */
                continue;
            }
            if (sum2 + S32(TREASURY) < need)                            /* 0x1C276 jl */
                continue;
            if (sum2 + S32(SHORT_TREAS) < need)                         /* 0x1C286 jl */
                continue;
            d = sum2 + S32(SHORT_TREAS) - need;                         /* 0x1C288..0x1C292 */
            if (d < *excess) {                                          /* 0x1C29B jge */
                *excess = d;                                            /* 0x1C2AA */
                *mask = m;                                              /* 0x1C2B2 */
            }
        }
        if (found != 0) {                                               /* 0x1C2B6 */
            *exact = found;                                             /* 0x1C2C2 */
            return 1;
        }
        return 0;                                                       /* 0x1C2BA -> 0x1C2D1 */
    }
    return 0;                                                           /* 0x1C2D1 */
}

/* ==== 0x1B59C: the cards that pay ===================================================================================== */

/* the kept best (or the exact fit) written to record 0: the cards cleared, then the masked entries' counts */
static void apply_mask(uint32_t mask, uint32_t list, uint32_t cnt)
{
    int32_t i;
    for (i = 1; i <= 0x12; i++)                                         /* 0x1B6EF / 0x1B7AD / 0x1B8B5 */
        setD8(REC0 + (uint32_t)i, 0);
    for (i = 0; i < S32(cnt); i++) {                                    /* 0x1B713 / 0x1B7D1 / 0x1B8D9 */
        if ((mask & BIT(i)) == 0)                                       /* 0x1B731 test eax, ebx */
            continue;
        setD8(REC0 + D32(list + 32u * (uint32_t)i + 8u), D8(list + 32u * (uint32_t)i + 4u));   /* 0x1B744..0x1B750 */
    }
}

static int paid(int need)   /* [0xF6750] = max(0, need - f19448()); return 1 (0x1B75C / 0x1B812 / 0x1B922) */
{
    int32_t d = need - f19448();
    setD32(PAY_TREAS, (uint32_t)d);
    if (d < 0)
        setD32(PAY_TREAS, 0);
    return 1;
}

int f1B59C(int n, int need)
{
    uint32_t recn = NATION(n);                                          /* [ebp-0x18] 0x1B5EF */
    int32_t best = 0x3E7;                                               /* [ebp-0x20] 0x1B5B9 */
    uint32_t bestmask = 0;                                              /* [ebp-0x38] 0x1B5C2 */
    int32_t choice;                                                     /* [ebp-0x28] */
    int32_t tol = 3;                                                    /* [ebp-0x1C] 0x1B627 */
    int32_t first;                                                      /* [ebp-0x40] */
    int32_t excess = INT32_MAX;                                         /* [ebp-0x34]: stale-word item 2 (never read) */
    int32_t mask = 0;                                                   /* [ebp-0x30] */
    int32_t exact = 0;                                                  /* [ebp-0x2C] */
    int32_t grown[18];                                                  /* [ebp-0x94] the 0x48-byte flag block */
    int32_t k, i;

    memmove(DP(REC0), DP(recn), 199);                                   /* 0x1B5DC rep movsd 0x31; movsw; movsb */
    choice = S32(MINING_CHOICE);                                        /* 0x1B5F9 */
    setD32(SHORT_TREAS, (uint32_t)f1C2DC(n));                           /* 0x1B604; 0x1B609 */
    setD32(TREASURY, (uint32_t)(int32_t)S8(recn + 0x3Eu));              /* 0x1B611 [rec + 0x3B] sar 24 */
    if (S32(SHORT_TREAS) > S32(TREASURY))                               /* 0x1B61D jle */
        setD32(SHORT_TREAS, D32(TREASURY));                             /* 0x1B621 */
    memset(grown, 0, sizeof grown);                                     /* 0x1B640 memset([ebp-0x94], 0, 0x48) */
    first = 1;                                                          /* 0x1B645 ecx = 1 */
    /* the 18 carry flags [ebp-0xDC + 4(k-1)] are cleared at 0x1B650..0x1B6D0 on the way down: a digit that does not
       carry jumps back to the label clearing the ones below it, so every flag below the moving digit is 0 at each
       evaluation and the flags need no variables here (D) */

    for (;;) {
        /* 0x1B6D8 */
        if (best < tol) {                                               /* 0x1B6DE jge */
            apply_mask(bestmask, 0xF6B0Cu, BESTCNT);                    /* 0x1B6E6..0x1B752 */
            setD32(MINING_CHOICE, (uint32_t)choice);                    /* 0x1B757 */
            return paid(need);
        }
        if (f1BE08(n, need, &excess, &mask, &exact, first) == 1) {      /* 0x1B786..0x1B7A2 */
            apply_mask((uint32_t)exact, 0xF68CCu, ENTCNT);              /* 0x1B7A8..0x1B810 */
            return paid(need);
        }
        first = 0;                                                      /* 0x1B83E */
        if (excess < best) {                                            /* 0x1B847 jge */
            best = excess;                                              /* 0x1B84E */
            bestmask = (uint32_t)mask;                                  /* 0x1B854 */
            choice = S32(MINING_CHOICE);                                /* 0x1B85C */
            for (i = 0; i < S32(ENTCNT); i++)                           /* 0x1B866 */
                memmove(DP(BEST(i)), DP(ENT(i)), 32);                   /* 0x1B896 rep movsd (8) */
            setD32(BESTCNT, D32(ENTCNT));                               /* 0x1B89A esi = [0xF6D4C] */
        }
        if (best < tol) {                                               /* 0x1B8A6 jge */
            apply_mask(bestmask, 0xF6B0Cu, BESTCNT);                    /* 0x1B8AC..0x1B918 */
            setD32(MINING_CHOICE, (uint32_t)choice);                    /* 0x1B91D */
            return paid(need);
        }
        /* 0x1B94C..0x1BDF5: the odometer step, commodity 1 first; digit k counts its byte down to 1 and then reloads
           it from record n (carry); the tolerance grows the first time digits 2, 4, ..., 16 carry (flags at
           [ebp-0x8C], -0x84, -0x7C, -0x74, -0x6C, -0x64, -0x5C, -0x54); digit 18's carry ends the search */
        for (k = 1; k <= 0x12; k++) {
            if (S8(REC0 + (uint32_t)k) > 1) {                           /* movsx / dword [rec0 + k - 3] sar 24; jg */
                add8(REC0 + (uint32_t)k, -1);                           /* dec byte [rec0 + k] */
                break;                                                  /* no carry: evaluate */
            }
            setD8(REC0 + (uint32_t)k, D8(recn + (uint32_t)k));          /* the full count reloaded, carry */
            if (k == 0x12)
                return 0;                                               /* 0x1BDFB */
            if ((k & 1) == 0 && k <= 0x10 && grown[k] == 0) {           /* 0x1B9BD, 0x1BA48, ... 0x1BD74 */
                grown[k] = 1;
                tol += 3;
            }
        }
    }
}

/* ==== 0x19700: "%s has acquired ..." ================================================================================== */

static void cat(char *buf, size_t size, const char *s)   /* the inline strcat (repne scasb; copy two bytes a turn) */
{
    size_t l = strlen(buf);
    if (l + 1 < size)
        snprintf(buf + l, size - l, "%s", s);
}

void f19700(int n, int count, uint32_t list)
{
    char buf[300];                                                      /* [ebp-0x16C] (up to the locals at -0x40) */
    int32_t i;                                                          /* [ebp-0x10] */
    uint32_t saved;                                                     /* [ebp-0x14] */
    uint32_t dlg, r;
    int32_t w, bw, h, x, y, bx;                                         /* [ebp-0x28], [ebp-0x24], [ebp-0x18], [ebp-0x34], [ebp-0x30], [ebp-0x2C] */
    int32_t maxw;                                                       /* [ebp-0x1C] */
    int code;                                                           /* [ebp-0x20] */
    int last2 = g_original ? count - 1 : count - 2;                     /* BUG(1995) 1 (file comment): the ", " test */

    snprintf(buf, sizeof buf, DSTR(0xB15E6u), NAME(n));                 /* 0x19738 "%s has acquired \n" */
    for (i = 0; i < count; i++) {                                       /* 0x19788, 0x1978E jge */
        cat(buf, sizeof buf, ADV_NAME(S16(list + 2u * (uint32_t)i)));   /* 0x1979E movsx; [4a + 0xC7DC4]; 0x197B7 */
        if (count > 2) {                                                /* 0x197D3 jle */
            if (last2 > i)                                              /* 0x197E3 jle (count - 1 > i, D) */
                cat(buf, sizeof buf, DSTR(0xB15F8u));                   /* 0x197EB ", \n" */
            else if (g_original || i == count - 2)                      /* 0x19745 (the original: every later name) */
                cat(buf, sizeof buf, DSTR(0xB15FCu));                   /* " and \n" */
        } else if (count == 2 && i == 0) {                              /* 0x1974F, 0x19757 */
            cat(buf, sizeof buf, DSTR(0xB1603u));                       /* 0x1975B " and \n" */
        }
    }
    cat(buf, sizeof buf, DSTR(0xB160Au));                               /* 0x197F5 "." */

    saved = D32(CUR_DIALOG);                                            /* 0x19827 */
    f78884();                                                           /* 0x1982C */
    for (i = 0; ; i++) {                                                /* 0x19836 */
        dlg = D32(CUR_DIALOG);                                          /* 0x1983C */
        if (S16(dlg) <= i)                                              /* 0x19842 movsx; jle */
            break;
        setD8(SAVED_EN + (uint32_t)i, D8(dlg + 20u * (uint32_t)i + 0x12u));   /* 0x1985A..0x19861 the enabled byte */
        {
            int16_t id = S16(dlg + 20u * (uint32_t)i + 0x10u);          /* 0x19873 */
            if (id >= 0x8E && id <= 0xA5)                               /* 0x19878 jl, 0x1988B jle (signed) */
                continue;
        }
        f3B4E0(D32(CUR_DIALOG) + 2u + 20u * (uint32_t)i);               /* 0x198AE */
        setD8(D32(CUR_DIALOG) + 20u * (uint32_t)i + 0x12u, 0);          /* 0x198C5 */
    }
    f788AC();                                                           /* 0x198D5 */

    w = fgf_width(buf, (int)strlen(buf)) + 0xA;                         /* 0x198EC; 0x198F4 */
    bw = w > 0xB4 ? w : 0xB4;                                           /* 0x198FA..0x1990D */
    if (bw - 0x14 > 0x15E)                                              /* 0x19913..0x1991B jle */
        bw = 0x15E;                                                     /* 0x1991D */
    h = f3A42C(0, 0, bw - 0x14, 0, buf, &maxw, 0) + 0x2D;               /* 0x1993D measure; 0x19945 */
    x = (0x280 - bw) / 2;                                               /* 0x19949..0x1995C */
    y = (0x1E0 - h) / 2;                                                /* 0x1995F..0x19972 */
    bx = x + (bw - 0x6E) / 2;                                           /* 0x19975..0x19989 */
    f78884();                                                           /* 0x1998C */
    junk_makegif(x, x + bw - 1, y, y + h - 1, DSTR(0xB160Cu));          /* 0x199AE fg_makegif(.., "junk") (port: junk.c) */
    f788AC();                                                           /* 0x199BB */
    f3C530(D32(CUR_DIALOG), 5);                                         /* 0x199C5 (edx = 5 from 0x199B6) */
    f3C530(D32(CUR_DIALOG), 3);                                         /* 0x199D4 */
    f3C530(D32(CUR_DIALOG), 4);                                         /* 0x199E3 */
    f3C530(D32(CUR_DIALOG), 6);                                         /* 0x199F2 */
    f3C530(D32(CUR_DIALOG), 7);                                         /* 0x19A01 */
    f3C530(D32(CUR_DIALOG), 8);                                         /* 0x19A10 */
    f3C530(D32(CUR_DIALOG), 0x2A);                                      /* 0x19A1F */
    f3C530(D32(CUR_DIALOG), 0x41);                                      /* 0x19A2E */
    f3C530(D32(CUR_DIALOG), 0xA6);                                      /* 0x19A3D */
    fg_vbopen(S32(BACKBUF));                                            /* 0x19A49 */
    f3C9D0(x, y, bw, h, 3);                                             /* 0x19A5F */
    fg_setcolor(5);                                                     /* 0x19A66 */
    f3A42C(x + 0xA, y + 0xA, bw - 0x14, 2, buf, &maxw, 1);              /* 0x19A92 */
    f3BB48(BOX_DLG, 0x2A, bx);                                          /* 0x19AA4 */
    f3BC64(BOX_DLG, 0x2A, y + h - 0x1D);                                /* 0x19ABE */
    f3C284(BOX_DLG, 0x2A, 0xB1611u);                                    /* 0x19AD2 "CONTINUE" */
    f3B05C(BOX_DLG);                                                    /* 0x19ADC */
    fg_vbclose();                                                       /* 0x19AE1 */
    f78884();                                                           /* 0x19AE6 */
    fg_vbpaste(x, x + bw - 1, y, y + h - 1, x, y + h - 1);              /* 0x19B0F */
    f788AC();                                                           /* 0x19B17 */
    do {
        if (f7BA64() == 0)                                              /* 0x19B1C */
            code = f78BA4(BOX_DLG, 0);                                  /* 0x19B2C */
        else {
            f78A20(S16(SRD) == 0 ? 0xFA : 0x3E8);                       /* 0x19B36 cmp word; 0x19B51 */
            code = -1;                                                  /* 0x19B56 */
        }
    } while (code == -100 || code != -1);                               /* 0x19B59, 0x19B62 */

    setD32(CUR_DIALOG, saved);                                          /* 0x19B6A */
    f78884();                                                           /* 0x19B71 */
    for (i = 0; ; i++) {                                                /* 0x19B7B */
        r = D32(CUR_DIALOG);                                            /* 0x19B81 */
        if (S16(r) <= i)                                                /* 0x19B87 movsx; jle */
            break;
        setD8(r + 20u * (uint32_t)i + 0x12u, D8(SAVED_EN + (uint32_t)i));   /* 0x19BA3..0x19BA9 */
        f3B4E0(D32(CUR_DIALOG) + 2u + 20u * (uint32_t)i);               /* 0x19BC6 */
    }
    f788AC();                                                           /* 0x19BCD */
    setD8(PICTURE_NOW, 0);                                              /* 0x19BD4 */
    f78884();                                                           /* 0x19BDA */
    fg_move(x, y);                                                      /* 0x19BE7 */
    junk_showgif(DSTR(0xB161Au), 3);                                    /* 0x19BF6 fg_showgif("junk", 3) (port: junk.c) */
    f3CCBC();                                                           /* 0x19BFE */
    f788AC();                                                           /* 0x19C03 */
}
