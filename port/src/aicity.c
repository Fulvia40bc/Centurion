/* Module AICITY of docs/M6_PLAN.md (M7 batch 1): the computer players' city planning and token gathering. Literal
   translations of nine routines (12,768 bytes); the address of each instruction a line comes from is in its comment.
   D = read in the code, G = inference. Prototypes: aicity.h.

     0x337C8  f337C8  how many new cities a computer aims for (412 bytes)
     0x33964  f33964  plan city sites and gather tokens on them, with trials under the World State Stack (1,384)
     0x33ECC  f33ECC  candidate city sites with priorities (1,136)
     0x3433C  f3433C  tokens still needed to build at t (1,396)
     0x348B0  f348B0  threat estimate around t: the enemies moving later, by land, road and sea (2,652)
     0x353C4  f353C4  predict the moves of the later nations with ships, into the shadow tables (680)
     0x3566C  f3566C  one nation's predicted state back from the shadow tables (416)
     0x3580C  f3580C  move tokens to a site in up to eight stages (4,364)
     0x79128  f79128  a land territory between two others usable by a nation (328)

   Calls into code that is not ported yet (all of this batch; each a static stub named as the original that stops the
   port with not_ported(address), or with -DAICITY_TEST the test's recorder; the real signatures, read at the call
   sites, are what the stubs declare):
     AIQ    0x48028  void f48028(int32_t *cnt, int32_t *list, int t, int n, int filter)        (ret 4: filter)
            0x4892C  void f4892C(int t, int32_t *cnt, int32_t *out, int count, int32_t *list, int n, int filter)
                                                                                                (ret 0xC: list, n, filter)
            0x48D44  void f48D44(int32_t *cnt, int32_t *list, int n, int t, int flags)        (ret 4: flags)
            0x7C838  int  f7C838(int t, int n, int k, int32_t *left)
            0x495C0  int  f495C0(int t, int n, int mode, int32_t *enemy)
            0x49790  int  f49790(int r, int t, int n, int enemy)
            0x49934  int  f49934(int t, int n, int mode)
            0x7701C  void f7701C(int len, uint8_t *buf, int k)
            0x160EC  void f160EC(int n)
            0x7BD5C  void f7BD5C(int n)
     AISEA  0x36918  void f36918(int n, int t, int32_t *need, int code)   (code: movsx of the mode byte, D 0x36343)
            0x374E0  int  f374E0(int j, int t, int n, int max, int a)    (ret 4: a)
            0x648C8  void f648C8(int n, int flags)
     AIPLAN 0x49A1C  int  f49A1C(int t, int n, int k, int exclude)
            0x46BB8  void f46BB8(int n)
   Ported callees: stack.h (f7A940 push, f7AC60 pop of the World State Stack), mstack.h (f7AA1C, f7AE14, f7AD24,
   f7AD7C, f7AF28, f7AFB8, f7A688 / f7A700, f7A7E8 / f7A85C), rules.h (f78D08, f7C7C4, f57CA4, f460C0, f46158),
   route.h (f5843C, f58888), helpers2.h (the comparator f3530C), wqsort.h (Watcom's qsort 0x8C331).

   Watcom runtime -> standard C: 0x7D5D0 memset -> memset (in g_ds for data, else on the local); the rep movsd /
   movsw / movsb record copies of 0x353C4 / 0x3566C (199 bytes) and the movsd x4 swaps of 0x348B0 (16 bytes) ->
   memcpy in g_ds. __CHK dropped. No game_rand (f78AE4), no clock (0x8A080), no input wait, no trace string and no
   message in these routines (D). The AI's rolls are the pre-rolled table int16 0x112A94[30][10] (rows of 20 bytes,
   column n - 1; made by f7BCD4 each TAXATION, saved), read as the original does (dword [2k + base] sar 16).

   Data (D, read in the code at the addresses given; offsets as the code adds them):
     nation n = 0xF14CC + 199n: +0x26 Metalworking (G: inventory), +0x27 Agriculture, +0x28 Roadbuilding, +0x31
       Architecture, +0x3E int8 treasury, +0x40 uint8 ships, +0x41 + j uint8 ship j's movement points, +0x45 + 2j int16
       ship j's territory + 1, +0x51 int8 cities, +0x5C int8 the AST step, +0x5D int8 the tokens it may still move
       to a site (G), +0x60 int8 count and +0x61 + i uint8 the areas (territory + 1) it holds, +0x98 int8 tokens on
       the board, +0x9A 0 out / 2 computer / 3 human
     area record r = 0xE31B8 + 69r (territory r - 1): +1 int8 nations there, +1 + m int8 the marker of nation m (m
       when present), +0x0E + m int8 m's tokens, +0x24 + m int8 m's tokens that can still move, int16 +0x37 the
       tokens of the planned site
     territory record t = 0xE1638 + 35t: +0 bit 0x02 coastal, +1 bit 0x40 a city site, bits 3..5 the population
       limit, +2 bit 0x01 (f33ECC mode 1), bit 0x04 (the guarded sweeps of f3580C skip it)
     territory state t = 0xE2C18 + 3t: +0 the owner, +2 bit 0x01 a city, bit 0x02 a site planned by the AI
     0xEE16C[t] in play; 0xEFA04[160n + t] int8 the site priorities (saved; row n, so row 0 is never read here);
     byte 0xF06C0[n] the order of play (rank; the planner looks at the nations with a higher rank, those moving after
     n); byte 0xF488C[n] (G: a cities-in-reserve factor); int32 0xF52E0[10][10] the relations; int8 0xF8558[9][160]
     the failure cache of f33964 (0x63 = impossible); int16 0xF3580[9][157] the AI's territory flags (low byte bit
     0x80); int32 [0xF4898] the stock size; int32 [0xF5908] (saved, cleared and restored around simulations); byte
     0xBCCDC[9][17][5] the AST table (column 0: the cities wanted at a step); int16 [0xBB3F0] (sea transport allowed,
     G); flags word 0xE1624: bits 10..11 the AI level, bit 5 (2*7 instead of 2*9 treasury kept for Architecture)
     move list 1: 4-byte records at 0x107694 + 4i {from, to, count, code}, int32 count [0x1079B4]; list 2's count
       int32 [0x107D3C] (saved and restored by f33964); byte [0x107ED8] (12+ tokens on a planned site); int32
       [0x107EDC] (Architecture used for a site), [0x107EE0] (Architecture reserved for a non-site)
     the ship search: 0x108820 16-byte records, int32 count [0x109E00] (route.h), +8 the tokens collected (f374E0);
       0xF8418 / 0xF84B8 (160 bytes) and 0xF8194 (0x280) caches of f374E0, set to -1 before each route list
     the shadow tables: nations 0xF09E8 + 199k, areas 0xEB3F0 + 69r (columns +0x02, +0x0F, +0x1C, +0x25, +0x2E,
       +0x3C, nine bytes each); not in the save (M6_PLAN risk 9)
     the land lists [0xF8E7C + 4t]: port handles to int16 count, int16 territories (game.h)
   Move codes written to byte +3 (G: the reason, read by the playback): 0x20 the release of a conflict area, 0 / 0x1B
   adjacent surplus, 1 / 0x1C and 2 / 0x1D the two sweeps, 3 / 0x1E the guarded sweeps, 0x12 / 0x1F the two steps
   of a road move (count bytes 0x33 and 0x4C, D), 0xC8 a planned site {0xC8, t, 0, +3 not written}, 0xC9 a mode-2 goal
   {0xC9, owner of t, 0, +3 not written}; the first of each pair for mode 0, the second for modes 1 / 2.

   Reads of stack garbage (M6_PLAN risk 7), fixed in both modes (a port cannot reproduce them):
     1. 0x33964: the counter [ebp-0x30] is set only after the candidate list is built (0x33A59); the two early exits
        (no sites left to plan: goal <= *count, 0x33A28; or cities + planned sites >= 9, 0x33A3D) go to the
        Architecture retry and to *count += counter (0x33EC3) with whatever the stack held. Reachable (G): a nation
        with 8 cities and one site already planned (bit 0x02 with 3+ tokens) and goal 1 from f337C8. Fixed: the
        counter starts at 0. Marked BUG(1995); the original's value cannot be reproduced, so --original changes nothing
        here (check in DOSBox-X: BP 0x33CA8 with [ebp-0x30] != 0 there).
     2. 0x3580C, 0x348B0 and f33964's list: the locals the original leaves uninitialised but that the callees fill
        before any read (the count [ebp-0x40] by f48028, [ebp-0x48] by f7C838, the candidate buffer read one entry
        past the count by f33ECC's tie scan at 0x3428C, whose result does not matter there) are zeroed here; no
        behaviour depends on it (D). f3580C's [ebp-0x28] is one slot for the whole call (x here), handed to f495C0
        and then to f49790 in all four sweeps: f495C0 writes *enemy = 0 before anything else (D 0x495E1), so the
        slot is never read uninitialised (not a garbage read; the inventory's "may leave it unwritten" is wrong).
   Deterministic out-of-range reads (risk 8, kept: they go through the data object): f3433C with enemy 0 (from f7C7C4)
   reads relation row 0, nation record 0 and the AI table one column left (0x112A92 + 20r, the previous row's last
   column); with enemy 10 (the Pirates) the relation 0xF52E0 + 400 + 4n lies past the 10x10 table (G: [0xF5470..],
   M6_PLAN risk 13). f3580C's mode-1 end with f7C7C4 = 0 reads area +0x0E (nation 0's column). A road move whose
   middle territory f79128 cannot find (-1) would move the tokens through area record 0 (G: unreachable, the middle
   comes from f4892C's own list).

   1995 bugs: none fixed beyond the garbage read above. Kept as the original (D): the choices of the planner, odd as
   they are (the Thrace / Illyria special cases of f33ECC, f353C4 simulating only the nations that own ships, f348B0's
   human repeat that keeps the first pass's sums and caps at 1 when the enemy has Metalworking and n has not, else 2,
   the dead constant 0 subtracted in f3433C's formulas [ebp-0x3c], the Architecture retry of f33964 that ignores the
   number still wanted). The inventory's BUG(1995) candidate for 0x3580C's mode-2 record (an "unscaled index" at
   0x36887 / 0x368D8) is not a bug: eax is the count shifted left by 2 at 0x3687D, so the record lands at 0x107694 +
   4 * count like every other (D); its +1 is the owner of t and its +3 is not written. M6_PLAN risk 11 should drop it.

   Easter eggs: none (no strings in these routines). Oddities (D): f33ECC hard-codes two nations: Thrace (n == 4) may
   plan SCYTHIA (territory 1) like a limit-1 territory, and Illyria / Thrace (n == 3 / 4) also plan limit-2 territories
   (0x340E8..0x34126); Crete (n == 5) aims at 2 tokens fewer on the board (0x337FF). The sibling headers (aiq.h,
   aisea.h, aiplan.h) declare the fifteen callees with the signatures above (f4892C's list there is const int32_t *):
   linking is #include of those headers in place of the stubs. */

#include <stdint.h>
#include <string.h>

#include "aicity.h"
#include "game.h"
#include "helpers2.h"
#include "mstack.h"
#include "route.h"
#include "rules.h"
#include "stack.h"
#include "state.h"
#include "wqsort.h"

/* ---- calls into code that is not ported yet (all of M7 batch 1; see the file comment) -------------------------- */

#ifdef AICITY_TEST
void aicity_test_f48028(int32_t *cnt, int32_t *list, int t, int n, int filter);
void aicity_test_f4892C(int t, int32_t *cnt, int32_t *out, int count, int32_t *list, int n, int filter);
void aicity_test_f48D44(int32_t *cnt, int32_t *list, int n, int t, int flags);
int  aicity_test_f7C838(int t, int n, int k, int32_t *left);
int  aicity_test_f495C0(int t, int n, int mode, int32_t *enemy);
int  aicity_test_f49790(int r, int t, int n, int enemy);
int  aicity_test_f49934(int t, int n, int mode);
void aicity_test_f7701C(int len, uint8_t *buf, int k);
void aicity_test_f160EC(int n);
void aicity_test_f7BD5C(int n);
void aicity_test_f36918(int n, int t, int32_t *need, int code);
int  aicity_test_f374E0(int j, int t, int n, int max, int a);
void aicity_test_f648C8(int n, int flags);
int  aicity_test_f49A1C(int t, int n, int k, int exclude);
void aicity_test_f46BB8(int n);
#define f48028(c, l, t, n, f)          aicity_test_f48028(c, l, t, n, f)
#define f4892C(t, c, o, k, l, n, f)    aicity_test_f4892C(t, c, o, k, l, n, f)
#define f48D44(c, l, n, t, f)          aicity_test_f48D44(c, l, n, t, f)
#define f7C838(t, n, k, p)             aicity_test_f7C838(t, n, k, p)
#define f495C0(t, n, m, p)             aicity_test_f495C0(t, n, m, p)
#define f49790(r, t, n, e)             aicity_test_f49790(r, t, n, e)
#define f49934(t, n, m)                aicity_test_f49934(t, n, m)
#define f7701C(l, b, k)                aicity_test_f7701C(l, b, k)
#define f160EC(n)                      aicity_test_f160EC(n)
#define f7BD5C(n)                      aicity_test_f7BD5C(n)
#define f36918(n, t, p, c)             aicity_test_f36918(n, t, p, c)
#define f374E0(j, t, n, m, a)          aicity_test_f374E0(j, t, n, m, a)
#define f648C8(n, f)                   aicity_test_f648C8(n, f)
#define f49A1C(t, n, k, e)             aicity_test_f49A1C(t, n, k, e)
#define f46BB8(n)                      aicity_test_f46BB8(n)
#else
/* M7 batch 1 linked (Claude, 21 Nov): the other planner modules' real routines (they were not_ported stubs
   while the five modules were translated in parallel) */
#include "aiq.h"
#include "aisea.h"
#include "aimove.h"
#include "aiplan.h"
#endif

/* ---- records ---------------------------------------------------------------------------------------------------- */

#define NAT(n)     (0xF14CCu + 199u * (uint32_t)(n))
#define AREA(r)    (0xE31B8u + 69u * (uint32_t)(r))
#define TERR(t)    (0xE1638u + 35u * (uint32_t)(t))
#define TST(t)     (0xE2C18u + 3u * (uint32_t)(t))
#define RANK(n)    (0xF06C0u + (uint32_t)(n))
#define CACHE(n1, t)  (0xF8558u + 160u * (uint32_t)(n1) + (uint32_t)(t))
#define AITAB(row, k) (0x112A94u + 20u * (uint32_t)(row) + 2u * (uint32_t)((k) - 1))   /* int16, column k - 1 */
#define REL(a, b)  (0xF52E0u + 40u * (uint32_t)(a) + 4u * (uint32_t)(b))

#define FLAGS       0xE1624u
#define STOCK_SIZE  0xF4898u
#define RESERVE     0xF488Cu
#define SAVED_5908  0xF5908u
#define ML1         0x107694u
#define ML1_COUNT   0x1079B4u
#define ML2_COUNT   0x107D3Cu
#define BIG_SITE    0x107ED8u   /* byte */
#define ARCH_SITE   0x107EDCu   /* int32 */
#define ARCH_NONSITE 0x107EE0u  /* int32 */
#define SEA_OK      0xBB3F0u    /* int16 */
#define AST_TAB     0xBCCDCu
#define LANDADJ     0xF8E7Cu
#define SHADOW_NAT  0xF09E8u
#define SHADOW_AREA 0xEB3F0u

static int ai_level(void)   /* mov ax, [0xE1624]; shl eax, 4; shr ax, 14: bits 10..11 */
{
    return (D16(FLAGS) >> 10) & 3;
}

static int flag_bit5(void)  /* shl eax, 10; shr ax, 15: bit 5 */
{
    return (D16(FLAGS) >> 5) & 1;
}

static int limit_of(int t)  /* territory +1: shl al, 2; shr al, 5 */
{
    return (D8(TERR(t) + 1u) >> 3) & 7;
}

static int32_t hword(uint32_t h)   /* int16 in a land list (a port handle) */
{
    const uint8_t *p = load_heap_ptr(h, 2);
    return p != NULL ? (int32_t)(int16_t)(p[0] | p[1] << 8) : 0;   /* port guard: an unresolved handle reads 0 */
}

/* a move record {from, to, b2, code} appended to move list 1 (the bytes the original writes, in any order) */
static void ml_add(int from, int to, int b2, int code)
{
    int32_t c = S32(ML1_COUNT);
    setD8(ML1 + 4u * (uint32_t)c, (uint8_t)from);
    setD8(ML1 + 4u * (uint32_t)c + 1u, (uint8_t)to);
    setD8(ML1 + 4u * (uint32_t)c + 3u, (uint8_t)code);
    setD32(ML1_COUNT, (uint32_t)(c + 1));
    setD8(ML1 + 4u * (uint32_t)c + 2u, (uint8_t)b2);
}

/* ---- 0x337C8: the number of new cities to aim for --------------------------------------------------------------- */

int f337C8(int n)
{
    uint32_t rec = NAT(n);                                            /* 0x337DD [ebp-4] */
    int32_t lim, tok, k, c, e, i, shared;

    lim = S32(STOCK_SIZE) / 2;                                        /* 0x337EE..0x337FA sar edx, 1fh; sub; sar */
    if (n == 5)                                                       /* 0x337FF Crete */
        lim -= 2;
    tok = S8(rec + 0x98u);                                            /* 0x3380C dword [+0x95] sar 24 */
    if (S16(AITAB(4, n)) / 300 < (ai_level() + 1) * 10)               /* 0x3381B [2n + 0x112AE0] sar 16; 0x33853 */
        lim -= 5;
    if (tok < lim) {                                                  /* 0x3385E */
        shared = 0;
        for (i = 0; i < S8(rec + 0x60u); i++)                         /* 0x33873 dword [+0x5D] sar 24 */
            if (S8(AREA(D8(rec + 0x61u + (uint32_t)i)) + 1u) > 1) {   /* 0x33897 [a + 68a + 0xE31B6] sar 24 */
                shared = 1;
                break;
            }
        return (tok > 21 || shared) ? 1 : 0;                          /* 0x338AD */
    }
    c = S8(rec + 0x51u);                                              /* 0x338CD [ebp-0x24] */
    for (k = c; 2 * k <= tok && tok > 18; k++)                        /* 0x338DC..0x338F6 */
        tok -= 6;
    e = k - c < 3 ? k - c : 3;                                        /* 0x33903 */
    if (e == 3)                                                       /* 0x33916 */
        e -= S16(AITAB(5, n)) / 300 < 25;                             /* 0x3391F [2n + 0x112AF4] sar 16; setl */
    if (e + c > 9)                                                    /* 0x33949 */
        e = 9 - c;
    return e;
}

/* ---- 0x33ECC: candidate sites ------------------------------------------------------------------------------------ */

void f33ECC(int32_t *cnt, uint8_t *list, int n, int mode)
{
    int32_t t, i, j, k, v;
    uint16_t w;
    uint8_t tmp[160];                                                 /* [ebp-0xCC] */

    *cnt = 0;                                                         /* 0x33EF7 */
    for (t = 0; t < 157; t++) {                                       /* 0x33FD2 */
        if (D8(0xEE16Cu + (uint32_t)t) == 0)                          /* 0x33FE2 */
            continue;
        if (D8(TST(t) + 2u) & 1)                                      /* 0x33FF8 */
            continue;
        if (D8(TST(t) + 2u) & 2)                                      /* 0x3400F */
            continue;
        if (D8(TERR(t) + 1u) & 0x40) {                                /* 0x34029 a city site */
            if (S8(AREA(t + 1) + 0x0Eu + (uint32_t)n) > 0) {          /* 0x3404B [.. + n-1 + 0xE31C4] sar 24 */
                if (mode == 0 || mode == 2)                           /* 0x3405F */
                    goto add_site;                                    /* 0x3406D */
                if (mode != 1)                                        /* 0x33F07 */
                    continue;
                if (!(D8(TERR(t) + 2u) & 1))                          /* 0x33F24 */
                    continue;
            } else {
                if (mode != 0)                                        /* 0x33F70 */
                    continue;
                if (f49934(t, n, 0) == 0)                             /* 0x33F7F */
                    continue;
            }
        add_site:
            list[2 * *cnt] = (uint8_t)t;                              /* 0x3407C */
            list[2 * *cnt + 1] = (uint8_t)(D8(0xEFA04u + 160u * (uint32_t)n + (uint32_t)t) + 10);   /* 0x34090 */
            (*cnt)++;                                                 /* 0x340A9 */
            continue;
        }
        if (mode != 0)                                                /* 0x340B6 */
            continue;
        v = limit_of(t);                                              /* 0x340D2 */
        if (!(v == 1 || (n == 4 && t == 1))) {                        /* 0x340E3..0x340F2 */
            if (n != 4 && n != 3)                                     /* 0x340F4 Thrace, Illyria */
                continue;
            if (limit_of(t) != 2)                                     /* 0x34115 */
                continue;
        }
        if (S8(AREA(t + 1) + 1u) != 1)                                /* 0x3413A */
            continue;
        if (S8(AREA(t + 1) + 1u + (uint32_t)n) != n)                  /* 0x34161 [.. + n-1 + 0xE31B7] sar 24 */
            continue;
        list[2 * *cnt] = (uint8_t)t;                                  /* 0x34182 */
        list[2 * *cnt + 1] = (uint8_t)(9 - limit_of(t));              /* 0x34195..0x341C4 */
        (*cnt)++;
    }

    /* bubble sort, priorities descending (signed bytes), the 2-byte entries swapped as words */
    for (i = *cnt - 2; i >= 0; i--)                                   /* 0x341CD..0x341F3 */
        for (j = 0; j <= i; j++)                                      /* 0x34202 */
            if ((int8_t)list[2 * j + 3] > (int8_t)list[2 * j + 1]) {  /* 0x3421C cmp dl, ch; jle */
                w = (uint16_t)(list[2 * j + 2] | list[2 * j + 3] << 8);   /* 0x3422E */
                list[2 * j + 2] = list[2 * j];                        /* 0x34248 */
                list[2 * j + 3] = list[2 * j + 1];
                list[2 * j] = (uint8_t)w;                             /* 0x34259 */
                list[2 * j + 1] = (uint8_t)(w >> 8);
            }

    /* each run of equal priority reordered by f7701C */
    i = 0;                                                            /* 0x3425E */
    while (i < *cnt) {                                                /* 0x34263 */
        j = i;                                                        /* 0x34274 */
        while (list[2 * i + 1] == list[2 * j + 3] && j + 1 < *cnt)    /* 0x34289..0x3429A (reads entry j+1 first) */
            j++;
        if (i == j) {                                                 /* 0x342AA */
            i++;
            continue;
        }
        for (k = i; k <= j; k++)                                      /* 0x342C2 */
            tmp[k - i] = list[2 * k];                                 /* 0x342DA */
        f7701C(j - i + 1, tmp, n);                                    /* 0x342F3 */
        for (k = i; k <= j; k++)                                      /* 0x34306 */
            list[2 * k] = tmp[k - i];                                 /* 0x34323 */
        i = j + 1;                                                    /* 0x3432B */
    }
}

/* ---- 0x33964: plan sites and gather tokens ---------------------------------------------------------------------- */

int f33964(int goal, int32_t *count, int n, int mode)
{
    uint32_t rec;
    int32_t n1, rem, planned, i, cnt = 0, done, j, t, s, lim, x, r, budget, save1, save2;
    uint8_t list[0x198];                                              /* [ebp-0x198] */

    if (goal <= 0)                                                    /* 0x33985 */
        return goal;
    memset(list, 0, sizeof list);                                     /* port: see the file comment (garbage 2) */
    rec = NAT(n);                                                     /* 0x3399D [ebp-0xC] */
    n1 = n - 1;                                                       /* 0x339A4 [ebp-0x48] */
    rem = goal - *count;                                              /* 0x339B1 [ebp-0x28] */
    planned = 0;                                                      /* 0x339B6 [ebp-0x24] */
    for (i = 0; i < S8(rec + 0x60u); i++) {                           /* 0x339C4 */
        uint32_t a = D8(rec + 0x61u + (uint32_t)i);                   /* 0x339D8 */
        if (!(D8(TST(a - 1u) + 2u) & 2))                              /* 0x339EB [3(a-1) + 0xE2C1A] */
            continue;
        if (S8(AREA(a) + 0x0Eu + (uint32_t)n) >= 3)                   /* 0x33A0E [69a + n + 0xE31C3] sar 24 */
            planned++;
    }
    /* BUG(1995): the counter [ebp-0x30] is first set at 0x33A59, after the two early exits below (stack garbage;
       see the file comment). It starts at 0 here in both modes. */
    done = 0;
    if (rem > 0 && S8(rec + 0x51u) + planned < 9) {                   /* 0x33A24, 0x33A2E..0x33A3D */
        f33ECC(&cnt, list, n, mode);                                  /* 0x33A52 */
        done = 0;                                                     /* 0x33A59 [ebp-0x30] */
        j = 0;                                                        /*         [ebp-0x2c] */
        while (done < rem && j < cnt && S8(rec + 0x51u) + done + planned < 9) {   /* 0x33A5F..0x33A90 */
            t = list[2 * j];                                          /* 0x33A9B [ebp-0x20] */
            if (D8(CACHE(n1, t)) != 0) {                              /* 0x33AB7 */
                j++;                                                  /* 0x33D2D */
                continue;
            }
            s = f3433C(t, n);                                         /* 0x33ACE [ebp-0x1c] */
            if (s < 0) {                                              /* 0x33AD9 */
                setD8(CACHE(n1, t), 0x63);                            /* 0x33AEF */
                j++;
                continue;
            }
            f7A940();                                                 /* 0x33AFB World State push */
            save1 = S32(ML1_COUNT);                                   /* 0x33B00 [ebp-0x38] */
            save2 = S32(ML2_COUNT);                                   /* 0x33B08 [ebp-0x34] */
            lim = f78D08(t, n);                                       /* 0x33B16 [ebp-0x14] */
            x = S8(AREA(t + 1) + 0x0Eu + (uint32_t)n);                /* 0x33B31 [.. + n-1 + 0xE31C4] sar 24 */
            x = x > lim ? lim : x;                                    /* 0x33B3D */
            setD8(rec + 0x5Du, (uint8_t)(D8(rec + 0x5Du) - (uint8_t)x));   /* 0x33B6B sub byte [+0x5D], al */
            r = f3580C(s, t, n, 0, s);                                /* 0x33B7D */
            if (r == 0 || (r == 1 && D8(RESERVE + (uint32_t)n) != 0 && (D8(TERR(t) + 1u) & 0x40) &&
                           S8(AREA(t + 1) + 0x0Eu + (uint32_t)n) >= 6)) {   /* 0x33B85..0x33BE2 */
                if (r == 1) {                                         /* 0x33BEB */
                    int32_t c = S32(ML1_COUNT);                       /* 0x33BF0 */
                    setD8(ML1 + 4u * (uint32_t)c, 0xC8);              /* 0x33BFD [4(c+1) + 0x107690] */
                    setD32(ML1_COUNT, (uint32_t)(c + 1));             /* 0x33C05 */
                    setD8(ML1 + 4u * (uint32_t)c + 1u, (uint8_t)t);   /* 0x33C0D; +2 / +3 not written (D) */
                    setD8(TST(t) + 2u, (uint8_t)(D8(TST(t) + 2u) | 2));   /* 0x33C1F */
                    setD8(TST(t), (uint8_t)n);                        /* 0x33C37 */
                    setD16(AREA(t + 1) + 0x37u,                       /* 0x33C63 word [69(t+1) + 0xE31EF] */
                           (uint16_t)(int16_t)S8(AREA(t + 1) + 0x0Eu + (uint32_t)n));   /* 0x33C50 movsx bx */
                }
                f7AD24();                                             /* 0x33C6B World State drop */
                done++;                                               /* 0x33C73 */
                j++;                                                  /* 0x33C79 */
                if (S8(AREA(t + 1) + 0x0Eu + (uint32_t)n) >= 12) {    /* 0x33C8F [69(t+1) + n + 0xE31C3] */
                    setD8(BIG_SITE, 1);                               /* 0x33CA1 */
                    break;
                }
                continue;
            }
            f7AC60();                                                 /* 0x33CF0 World State pop */
            j++;                                                      /* 0x33CF8 */
            setD32(ML1_COUNT, (uint32_t)save1);                       /* 0x33CFE */
            setD32(ML2_COUNT, (uint32_t)save2);                       /* 0x33D06 */
            setD8(CACHE(n1, t), (uint8_t)r);                          /* 0x33D22 */
        }
    }
    /* 0x33CA8: short of the goal: the Architecture retry */
    if (done < rem && S32(ARCH_NONSITE) == 0 && D8(rec + 0x31u) != 0) {   /* 0x33CA8..0x33CCF */
        budget = S8(rec + 0x3Eu) - 2 * (flag_bit5() ? 7 : 9);         /* 0x33CD5..0x33D51 [ebp-0x3c] */
        if (budget > 3)                                               /* 0x33D54 */
            budget = 3;
        else if (budget < 0) {                                        /* 0x33D5D */
            budget = S8(rec + 0x3Eu);                                 /* 0x33D68 */
            if (budget > 3)                                           /* 0x33D77 */
                budget = 3;
        }
        if (budget > 0)                                               /* 0x33D83 */
            for (t = 0; t < 157; t++) {                               /* 0x33E26 [ebp-0x4c] */
                if (D8(CACHE(n1, t)) == 0)                            /* 0x33E45 test eax, eax; jle (a byte) */
                    continue;
                if (D8(CACHE(n1, t)) > budget)                        /* 0x33E71 */
                    continue;
                s = f3433C(t, n);                                     /* 0x33E7C */
                if (s < 0) {                                          /* 0x33E87 */
                    setD8(CACHE(n1, t), 0x63);                        /* 0x33EA1 */
                    continue;
                }
                f7A940();                                             /* 0x33D97 */
                save1 = S32(ML1_COUNT);                               /* 0x33D9C */
                save2 = S32(ML2_COUNT);                               /* 0x33DA4 */
                r = f3580C(s - D8(CACHE(n1, t)), t, n, 0, s);         /* 0x33DB8..0x33DDC */
                if (r == 0) {                                         /* 0x33DE4 */
                    f7AD24();                                         /* 0x33EAD */
                    done++;                                           /* 0x33EB5 */
                    continue;
                }
                f7AC60();                                             /* 0x33DEE */
                setD32(ML1_COUNT, (uint32_t)save1);                   /* 0x33DF6 */
                setD32(ML2_COUNT, (uint32_t)save2);                   /* 0x33DFE */
                setD8(CACHE(n1, t), (uint8_t)r);                      /* 0x33E1A */
            }
    }
    *count += done;                                                   /* 0x33EC3 */
    return done;                                                      /* eax = [ebp-0x30] */
}

/* ---- 0x3433C: tokens still needed at t -------------------------------------------------------------------------- */

int f3433C(int t, int n)
{
    int32_t ret = -1;                                                 /* 0x34356 [ebp-0x2c] */
    int32_t enemy = 0;                                                /* 0x34362 [ebp-0x28] */
    int32_t k;                                                        /*         [ebp-0x24] */
    int32_t a = t + 1;                                                /* 0x34369 [ebp-0x40] */
    int32_t dead = 0;                                                 /* 0x34373 [ebp-0x3c]: never written again (D) */
    int32_t ab = 0;                                                   /* 0x34376 [ebp-0x38] */
    int32_t maxb = 0;                                                 /* [ebp-0x30]: always written by f348B0 (D) */
    int32_t x, e, metal, mb, hum, lvl, q, thr, r, tok;

    if (D8(NAT(n) + 0x31u) != 0 && S32(ARCH_NONSITE) == 0) {          /* 0x34380, 0x34389 */
        ab = S8(NAT(n) + 0x3Eu) - 2 * (flag_bit5() ? 7 : 9);          /* 0x34392..0x343C9 */
        if (ab > 6)                                                   /* 0x343CE */
            ab = 6;
        else if (ab < 0)                                              /* 0x343E0 */
            ab = 0;
    }
    if (S8(AREA(a) + 1u) >= 2) {                                      /* 0x343F3 */
        if (S8(AREA(a) + 1u) != 2)                                    /* 0x34416 */
            return ret;
        if (S8(AREA(a) + 0x0Eu + (uint32_t)n) == 0)                   /* 0x34431 [.. + n-1 + 0xE31C4] sar 24 */
            return ret;
    }
    x = f348B0(t, n, &enemy, &maxb);                                  /* 0x3444E [ebp-0x20] */
    e = 0;                                                            /* 0x34458 [ebp-0x1c] */
    if ((S8(AREA(a) + 1u) == 1 && S8(AREA(a) + 1u + (uint32_t)n) != n) ||   /* 0x3446F, 0x34486 */
        (S8(AREA(a) + 1u) == 2 && S8(AREA(a) + 1u + (uint32_t)n) == n)) {   /* 0x344A8, 0x344BF */
        k = f7C7C4(t, n);                                             /* 0x344D3 */
        if (k != enemy) {                                             /* 0x344E3 */
            x = 0;                                                    /* 0x344E7 */
            enemy = k;                                                /* 0x344ED */
        }
        e = S8(AREA(a) + 0x0Eu + (uint32_t)enemy);                    /* 0x34502 [69a + enemy + 0xE31C3] sar 24 */
    }
    if (x + e <= 0 || enemy == 11)                                    /* 0x34514, 0x3451F */
        goto no_threat;
    if (S32(REL(enemy, n)) < 0) {                                     /* 0x3453A [40 enemy + 4n + 0xF52E0] */
        hum = S8(NAT(enemy) + 0x9Au) == 3;                            /* 0x3454F [+0x97] sar 24; sete */
        lvl = ai_level();                                             /* 0x34566..0x34575 */
        q = 200 / ((hum + 1) * (5 - lvl));                            /* 0x3457D..0x3458E idiv */
        thr = S32(REL(n, enemy)) - q;                                 /* 0x345A4..0x345B0 */
        r = t % 4 + 9;                                                /* 0x345B2..0x345BD idiv 4; edx + 9 */
        if (S16(0x112A92u + 20u * (uint32_t)r + 2u * (uint32_t)enemy) / 300 > thr) {   /* 0x345CE..0x345E6 */
            /* 0x34643 */
            if (S16(0x112A92u + 20u * (uint32_t)r + 2u * (uint32_t)n) / 300 >   /* 0x34665..0x34679 */
                S32(REL(n, enemy)) / (5 - lvl))                       /* 0x34694..0x3469F idiv ecx */
                goto site;                                            /* 0x346A3 jle 0x34601, else on */
        } else if (S8(NAT(enemy) + 0x9Au) == 2)                       /* 0x345EF a computer that will not fight */
            goto no_threat;                                           /* 0x345FB */
        /* 0x34601: a fight is expected */
        metal = 0;
        if (D8(NAT(enemy) + 0x26u) != 0 && D8(NAT(n) + 0x26u) == 0)   /* 0x3460D, 0x3461D */
            metal = 1;
        if (metal + 2 < x)                                            /* 0x34633 */
            x = metal + 2;                                            /* 0x3463E */
    }
site:                                                                 /* 0x346A9 */
    if (!(D8(TERR(t) + 1u) & 0x40))                                   /* 0x346BA */
        return -999;                                                  /* 0x347F1 */
    mb = 0;                                                           /* 0x346C9 [ebp-0x18] */
    if (D8(NAT(n) + 0x26u) == 0 && D8(NAT(enemy) + 0x26u) != 0)       /* 0x346D3, 0x346E4 */
        mb = 1;
    tok = S8(AREA(a) + 0x0Eu + (uint32_t)n);                          /* [69a + n + 0xE31C3] sar 24 */
    if (x + e > 3 * S8(RESERVE + (uint32_t)n) + 4 - mb)               /* 0x346F7 [n + 0xF4889] sar 24; 0x3471B */
        ret = -(x + e);                                               /* 0x3471F */
    else
        ret = (6 - dead) + (x + e - 1 + mb) - tok;                    /* 0x34727..0x34755 */
    if (S32(ARCH_NONSITE) == 0 && S32(ARCH_SITE) == 0 && D8(NAT(n) + 0x31u) != 0 && ab > 2 &&
        tok + ret > 7) {                                              /* 0x34758..0x347B1 */
        ret -= tok + ret - 7;                                         /* 0x347C9..0x347D8 */
        setD32(ARCH_SITE, 1);                                         /* 0x347DB */
    }
    return ret;

no_threat:                                                            /* 0x34804 */
    if (D8(TERR(t) + 1u) & 0x40) {                                    /* 0x34815 */
        ret = (e == 0 ? 0 : e - 1) + 6 - dead - S8(AREA(a) + 0x0Eu + (uint32_t)n);   /* 0x3481E..0x3484D */
        return ret;
    }
    if (maxb == 0 && e == 0) {                                        /* 0x3485E, 0x34865 */
        ret = 12 - ab - S8(AREA(a) + 0x0Eu + (uint32_t)n);            /* 0x34875..0x3488E */
        if (ab > 0)                                                   /* 0x34896 */
            setD32(ARCH_NONSITE, 1);                                  /* 0x3489A */
    }
    return ret;                                                       /* 0x348A4 */
}

/* ---- 0x348B0: threat estimate ------------------------------------------------------------------------------------ */

/* the ships of nation k that can reach t: the best tokens-collected value per ship into d[] (0x34D00..0x34DB0 and the
   twin 0x3505F..0x35111; identical code). Ships are at most 4 (the int16 fields +0x45..+0x4B, D) */
static void ship_reach(int t, int k, int32_t d[4])
{
    int32_t j, m, i, from, pts, cnt;
    uint8_t tmp[16];                                                  /* [ebp-0x7c] */

    for (j = 0; j < D8(NAT(k) + 0x40u); j++) {                        /* 0x34D00 [ebp-0x4c] */
        from = S16(NAT(k) + 0x45u + 2u * (uint32_t)j) - 1;            /* 0x34D25 [199k + 2j + 0xF150F] sar 16 */
        pts = D8(NAT(k) + 0x41u + (uint32_t)j);                       /* 0x34D3F */
        if (f5843C(from, t, k, pts) != 1)                             /* 0x34D54 */
            continue;
        f58888(from, t, k, pts);                                      /* 0x34D6A */
        memset(DP(0xF8418u), 0xFF, 0xA0);                             /* 0x34D7E */
        memset(DP(0xF84B8u), 0xFF, 0xA0);                             /* 0x34D92 */
        memset(DP(0xF8194u), 0xFF, 0x280);                            /* 0x34DA8 */
        for (m = 0; m < S32(ROUTE_COUNT); m++) {                      /* 0x34BE4 */
            int32_t save = S32(SAVED_5908), v;                        /* 0x34BF1 [ebp-0x38] */
            setD32(SAVED_5908, 0);                                    /* 0x34BFB */
            v = f374E0(m, t, k, 999, 1);                              /* 0x34C11 */
            setD32(SAVED_5908, (uint32_t)save);                       /* 0x34C1C */
            setD8(ROUTE_BUF + 16u * (uint32_t)m + 8u, (uint8_t)v);    /* 0x34C2A */
        }
        cnt = S32(ROUTE_COUNT);                                       /* edx */
        if (cnt < 20) {                                               /* 0x34C32 */
            for (i = cnt - 2; i >= 0; i--)                            /* 0x34C3B..0x34C5C */
                for (m = 0; m <= i; m++)                              /* 0x34C6F */
                    if (S8(ROUTE_BUF + 16u * (uint32_t)m + 0x18u) > S8(ROUTE_BUF + 16u * (uint32_t)m + 8u)) {   /* 0x34C8F */
                        memcpy(tmp, DP(ROUTE_BUF + 16u * (uint32_t)m + 16u), 16);                       /* 0x34CA2 */
                        memcpy(DP(ROUTE_BUF + 16u * (uint32_t)m + 16u), DP(ROUTE_BUF + 16u * (uint32_t)m), 16);   /* 0x34CBE */
                        memcpy(DP(ROUTE_BUF + 16u * (uint32_t)m), tmp, 16);                              /* 0x34CD1 */
                    }
        } else
            wc_qsort(DP(ROUTE_BUF), (size_t)cnt, 16, f3530C);         /* 0x34CE6 call 0x8C331, comparator 0x3530C */
        d[j] = S8(ROUTE_BUF + 8u);                                    /* 0x34CEB [0x108825] sar 24 */
    }
}

/* C[k-1] from the ships of k (0x34DB5..0x34E5F and 0x35116..0x351C0) */
static void ship_threat(uint32_t rec, int n1, int k, int32_t *c, const int32_t d[4])
{
    int32_t i, kk = k - 1;                                            /* [ebp-0x2c] */
    for (i = 0; i < D8(NAT(k) + 0x40u); i++)                          /* 0x34DC2 */
        if (d[i] > c[kk])                                             /* 0x34DEA */
            c[kk] = d[i];
    if (S8(AST_TAB + 5u * (uint32_t)S8(rec + 0x5Cu) + 85u * (uint32_t)n1) > S8(rec + 0x51u))   /* 0x34E04..0x34E33 */
        c[kk] /= 4 - ai_level();                                      /* 0x34E38..0x34E5D idiv */
}

int f348B0(int t, int n, int32_t *enemy, int32_t *maxb)
{
    int32_t A[9], B[9], C[9];                                         /* [ebp-0xe8], [ebp-0xa0], [ebp-0xc4] */
    int32_t l1[160], l2[160], d[4];                                   /* [ebp-0x5e8], [ebp-0x368], [ebp-0x6c] */
    int32_t c1 = 0, c2 = 0, n1, rank, k, i, j, a, v, maxa, cap;
    uint32_t rec;

    memset(A, 0, sizeof A);                                           /* 0x348DE */
    memset(B, 0, sizeof B);                                           /* 0x348F0 */
    memset(C, 0, sizeof C);                                           /* 0x34902 */
    memset(l1, 0, sizeof l1);                                         /* port: filled by f48028 */
    memset(l2, 0, sizeof l2);                                         /* port: filled by f4892C */
    n1 = n - 1;                                                       /* 0x3490B [ebp-0x30] */
    rec = NAT(n);                                                     /* 0x3491C [ebp-0xc] */
    rank = S8(RANK(n));                                               /* 0x34922 [n + 0xF06BD] sar 24 */
    f7A940();                                                         /* 0x3492E World State push */
    f7AA1C();                                                         /* 0x34938 Move List push */
    for (k = 1; k <= 9; k++)                                          /* 0x34948 (ebx = 1 kept across 0x7AA1C) */
        if (S8(RANK(k)) > rank)                                       /* 0x3495D */
            f3566C(k);                                                /* 0x34964 */
    f48028(&c1, l1, t, n, 11);                                        /* 0x3497C */
    f4892C(t, &c2, l2, c1, l1, n, 11);                                /* 0x3499D */

    for (i = 0; i < c1; i++) {                                        /* 0x349AF */
        a = l1[i] + 1;                                                /* 0x349C6 [ebp-0x48] */
        if (S8(AREA(a) + 1u) == 0)                                    /* 0x349DD */
            continue;
        for (j = 0; j < 9; j++) {                                     /* 0x349EE [ebp-0x58] */
            if (j == n1)                                              /* 0x349FC */
                continue;
            if (!(S8(RANK(j + 1)) > rank) && j + 1 != 11)             /* 0x34A01 [j + 0xF06BE] sar 24; 0x34A15 */
                continue;
            v = S8(AREA(a) + 0x24u + (uint32_t)(j + 1));              /* 0x34A2C [69a + j + 0xE31DA] sar 24 */
            if (v == 0)                                               /* 0x34A3B */
                continue;
            B[j] += v;                                                /* 0x34A45 */
            A[j] += v;                                                /* 0x34A52 */
        }
    }
    for (i = 0; i < c2; i++) {                                        /* 0x34A68 */
        a = l2[i] + 1;                                                /* 0x34A7F */
        if (S8(AREA(a) + 1u) == 0)                                    /* 0x34A96 */
            continue;
        for (j = 0; j < 9; j++) {                                     /* 0x34AA7 */
            if (j == n1)                                              /* 0x34AB5 */
                continue;
            if (!(S8(RANK(j + 1)) > rank) && j + 1 != 11)             /* 0x34ABA, 0x34ACE */
                continue;
            if (D8(NAT(j + 1) + 0x28u) == 0)                          /* 0x34ADD Roadbuilding */
                continue;
            v = S8(AREA(a) + 0x24u + (uint32_t)(j + 1));              /* 0x34AF8 */
            if (v == 0)                                               /* 0x34B07 */
                continue;
            B[j] += v;                                                /* 0x34B11 */
            A[j] += v - 1;                                            /* 0x34B1F */
        }
    }
    maxa = 0;                                                         /* 0x34B2D [ebp-0x3c] */
    for (j = 0; j < 9; j++)                                           /* 0x34B3B */
        if (A[j] > maxa) {                                            /* 0x34B4E */
            maxa = A[j];                                              /* 0x34B5C */
            *enemy = j + 1;                                           /* 0x34B66 */
        }
    *maxb = 0;                                                        /* 0x34B6D */
    for (j = 0; j < 9; j++)                                           /* 0x34B80 */
        if (B[j] > *maxb)                                             /* 0x34B95 */
            *maxb = B[j];                                             /* 0x34BA6 */

    if (maxa < 5 && (D8(TERR(t)) & 2)) {                              /* 0x34BAA, 0x34BC5 coastal */
        for (k = 1; k <= 9; k++) {                                    /* 0x34E6C [ebp-0x40] */
            if (!(S8(RANK(k)) > rank))                                /* 0x34E81 */
                continue;
            if (D8(NAT(k) + 0x40u) == 0)                              /* 0x34E97 */
                continue;
            memset(d, 0, sizeof d);                                   /* 0x34EAE (16 bytes) */
            ship_reach(t, k, d);
            ship_threat(rec, n1, k, C, d);
        }
        maxa = 0;                                                     /* 0x34EBD */
        for (j = 0; j < 9; j++)                                       /* 0x34ECB */
            if (A[j] + C[j] > maxa) {                                 /* 0x34EE8 */
                maxa = A[j] + C[j];                                   /* 0x34F02 */
                *enemy = j + 1;                                       /* 0x34F0C */
            }
    }
    f7AC60();                                                         /* 0x34F10 World State pop */
    f7AE14();                                                         /* 0x34F15 Move List pop */
    if (maxa != 0)                                                    /* 0x34F1A */
        return maxa;

    /* nothing found: the human nations after n with ships, their +0x5D at 99 */
    f7A940();                                                         /* 0x34F24 */
    f7AA1C();                                                         /* 0x34F29 */
    for (k = 1; k <= 9; k++) {                                        /* 0x351CD */
        if (S8(NAT(k) + 0x9Au) != 3)                                  /* 0x351E3 */
            continue;
        if (!(S8(RANK(k)) > rank))                                    /* 0x351F7 */
            continue;
        if (D8(NAT(k) + 0x40u) == 0)                                  /* 0x3520D */
            continue;
        setD8(NAT(k) + 0x5Du, 0x63);                                  /* 0x35226 */
        memset(d, 0, sizeof d);                                       /* 0x3522F */
        ship_reach(t, k, d);
        ship_threat(rec, n1, k, C, d);
    }
    maxa = 0;                                                         /* 0x3523E */
    for (j = 0; j < 9; j++)                                           /* 0x3524E: A and C as the first pass left them */
        if (A[j] + C[j] > maxa) {                                     /* 0x3526B */
            maxa = A[j] + C[j];                                       /* 0x35285 */
            *enemy = j + 1;                                           /* 0x3528F */
        }
    cap = (D8(NAT(*enemy) + 0x26u) != 0 && D8(NAT(n) + 0x26u) == 0) ? 1 : 2;   /* 0x35293..0x352BC */
    if (cap < maxa)                                                   /* 0x352C1 */
        maxa = cap;                                                   /* 0x352C6..0x352F4 (the same test again) */
    f7AC60();                                                         /* 0x352F7 */
    f7AE14();                                                         /* 0x352FC */
    return maxa;                                                      /* 0x35301 */
}

/* ---- 0x353C4 / 0x3566C: the shadow tables ----------------------------------------------------------------------- */

static const uint8_t k_cols[6] = { 0x02, 0x0F, 0x1C, 0x2E, 0x25, 0x3C };   /* the order of the copies (D) */

void f353C4(int n)
{
    int32_t rank, k, r, j, c, goal, done, save;

    f7A940();                                                         /* 0x353DC World State push */
    f7AA1C();                                                         /* 0x353E1 Move List push */
    rank = S8(RANK(n));                                               /* 0x353E9 [ebp-0x18] */
    for (k = 1; k <= 9; k++) {                                        /* 0x35404 [ebp-0x34] */
        if (!(S8(RANK(k)) > rank))                                    /* 0x3541D */
            continue;
        if (D8(NAT(k) + 0x40u) == 0)                                  /* 0x35433 */
            continue;
        f460C0(k);                                                    /* 0x35441 */
        f46158(k);                                                    /* 0x35449 */
        setD8(NAT(k) + 0x5Du, 0x63);                                  /* 0x35455 */
        save = S32(SAVED_5908);                                       /* 0x3545C [ebp-0x2c] */
        setD32(SAVED_5908, 0);                                        /* 0x35466 */
        f160EC(k);                                                    /* 0x3546F */
        f7BD5C(k);                                                    /* 0x35477 */
        memset(DP(CACHE(k - 1, 0)), 0, 0xA0);                         /* 0x35497 */
        goal = f337C8(k);                                             /* 0x3549F [ebp-0x1c] */
        done = 0;                                                     /* 0x354A9 [ebp-0x20] */
        f33964(goal, &done, k, 2);                                    /* 0x354BA */
        f46BB8(k);                                                    /* 0x354C2 */
        setD32(SAVED_5908, (uint32_t)save);                           /* 0x354CA */
        memcpy(DP(SHADOW_NAT + 199u * (uint32_t)k), DP(NAT(k)), 199); /* 0x354F3 rep movsd x0x31, movsw, movsb */
        for (r = 1; r <= 160; r++)                                    /* 0x35503 [ebp-0x28] */
            for (j = 0; j < 9; j++)                                   /* 0x3551D [ebp-0x24] */
                for (c = 0; c < 6; c++)                               /* 0x3554B..0x3564D */
                    setD8(SHADOW_AREA + 69u * (uint32_t)r + k_cols[c] + (uint32_t)j,
                          D8(AREA(r) + k_cols[c] + (uint32_t)j));
    }
    f7AC60();                                                         /* 0x35658 World State pop */
    f7AE14();                                                         /* 0x3565D Move List pop */
}

void f3566C(int k)
{
    int32_t r, c, j = k - 1;                                          /* 0x35688 [ebp-4] */

    memcpy(DP(NAT(k)), DP(SHADOW_NAT + 199u * (uint32_t)k), 199);     /* 0x356AA */
    for (r = 1; r <= 160; r++)                                        /* 0x356BE [ebp-8] */
        for (c = 0; c < 6; c++)                                       /* 0x356F3..0x357F5 */
            setD8(AREA(r) + k_cols[c] + (uint32_t)j, D8(SHADOW_AREA + 69u * (uint32_t)r + k_cols[c] + (uint32_t)j));
}

/* ---- 0x3580C: move tokens to a site ------------------------------------------------------------------------------ */

/* one visit of the two plain sweeps (0x35B23..0x35C3B and 0x35CE3..0x35DFC): a token from a if n keeps more there than
   the defenders f49790 asks for (at least 1). 1 = moved. */
static int sweep_plain(int a, int t, int n, int mode, int code0, int code1, int32_t *x)
{
    uint32_t rec = NAT(n);
    int32_t keep;                                                     /* [ebp-0x50] */

    keep = f495C0(a, n, 1, x);                                        /* 0x35B5A */
    keep = f49790(keep, a, n, *x);                                    /* 0x35B6E */
    if (keep == 0)                                                    /* 0x35B79 */
        keep = 1;
    if (S8(AREA(a + 1) + 0x24u + (uint32_t)n) > 0 &&                  /* 0x35B96 [.. + n-1 + 0xE31DA] */
        S8(AREA(a + 1) + 0x0Eu + (uint32_t)n) > keep) {               /* 0x35BB9 */
        f57CA4(a, t, n, 1, 1);                                        /* 0x35BD7 */
        ml_add(a, t, 1, mode == 0 ? code0 : code1);                   /* 0x35BDC..0x35C20 */
        setD8(rec + 0x5Du, (uint8_t)(D8(rec + 0x5Du) - 1));           /* 0x35C2E sub byte [+0x5D], bl */
        return 1;
    }
    return 0;
}

/* one visit of the two guarded sweeps (0x35E8D..0x3602A and 0x363C1..0x36542); second = 0 / 1 */
static int sweep_guarded(int a, int t, int n, int mode, int second, int32_t *x)
{
    uint32_t rec = NAT(n);
    int32_t keep, save, ok = 0;

    keep = f495C0(a, n, 1, x);                                        /* 0x35EC4 / 0x363F8 */
    keep = f49790(keep, a, n, *x);                                    /* 0x35ED8 / 0x3640C (no minimum of 1, D) */
    save = S32(SAVED_5908);                                           /* 0x35EE0 [ebp-0x4c] */
    setD32(SAVED_5908, 0);                                            /* 0x35EEA */
    if (S8(AREA(a + 1) + 0x24u + (uint32_t)n) > 0 &&                  /* 0x35F02 / 0x36436 */
        S8(AREA(a + 1) + 0x0Eu + (uint32_t)n) > keep &&               /* 0x35F2E / 0x36462 */
        (second || !(D8(TERR(a) + 2u) & 4)) &&                        /* 0x35F48: the first sweep only (D) */
        ((mode == 0 && keep == 0) || mode == 1 || mode == 2)) {       /* 0x35F55..0x35F6B / 0x3646B..0x36481 */
        if (f49A1C(a, n, 1, t) != 0)                                  /* 0x35F7F / 0x36495 */
            ok = 1;
        else if (S8(rec + 0x5Du) > 0 &&                               /* 0x35F8E / 0x364A4 */
                 (D8(0xF3580u + 314u * (uint32_t)(n - 1) + 2u * (uint32_t)a) & 0x80))   /* 0x35FA3 / 0x364B5 */
            ok = 1;
        else if (second && mode == 2)                                 /* 0x364BF: the second sweep only (D) */
            ok = 1;
    }
    if (ok) {
        f57CA4(a, t, n, 1, 1);                                        /* 0x35FBD / 0x364D5 */
        ml_add(a, t, 1, mode == 0 ? 3 : 0x1E);                        /* 0x35FC2..0x36007 / 0x364DA..0x3651D */
        setD8(rec + 0x5Du, (uint8_t)(D8(rec + 0x5Du) - 1));           /* 0x36015 / 0x3652D */
    }
    setD32(SAVED_5908, (uint32_t)save);                               /* 0x36028 / 0x36540 */
    return ok;
}

/* a guarded sweep with its look-ahead (0x35E21..0x3615D and 0x36355..0x36675) */
static void guarded_stage(int32_t *need, int t, int n, int mode, const int32_t *list, int32_t cnt, uint8_t *used,
                          int second, int32_t *x)
{
    int32_t flag = 0, keep_need, i, tries, k, rank, save;            /* [ebp-0x20], [ebp-0x24] */

    memset(used, 0, 0xA0);                                            /* 0x35E30 / 0x36362 */
    if (!(*need > 0 && cnt != 0 && S8(NAT(n) + 0x5Du) > 0))           /* 0x35E38..0x35E57 / 0x3636C..0x3638B */
        return;
    f7A940();                                                         /* 0x35E5D World State push */
    f7AA1C();                                                         /* 0x35E62 Move List push */
    keep_need = *need;                                                /* 0x35E6A */
again:                                                                /* 0x35E6D / 0x363A1 */
    i = 0;
    tries = 10;
    while (*need > 0 && tries > 0) {                                  /* 0x35E79, 0x35E83 */
        if (used[i] == 0) {                                           /* 0x35E90 [i + ebp-0x117] sar 24 */
            if (sweep_guarded(list[i], t, n, mode, second, x))
                (*need)--;                                            /* 0x3600F */
            else
                used[i] = 1;                                          /* 0x3601D */
        }
        i++;                                                          /* 0x36030 */
        if (i == cnt) {                                               /* 0x36036 */
            i = 0;
            tries--;                                                  /* 0x36047 */
        }
    }
    f7A688();                                                         /* 0x3604F world snapshot */
    f7A7E8();                                                         /* 0x36054 move-list snapshot */
    if (flag == 0) {                                                  /* 0x36059 */
        rank = S8(RANK(n));                                           /* 0x3606F [ebp-0x18] */
        memset(used, 0, 0xA0);                                        /* 0x36084 */
        for (k = 1; k <= 9; k++) {                                    /* 0x36094 [ebp-0x1c] */
            if (!(S8(RANK(k)) > rank))                                /* 0x360AD */
                continue;
            f460C0(k);                                                /* 0x360B4 */
            f46158(k);                                                /* 0x360BC */
            save = S32(SAVED_5908);                                   /* 0x360C1 */
            setD32(SAVED_5908, 0);                                    /* 0x360CB */
            f648C8(k, 0);                                             /* 0x360D6 */
            setD32(SAVED_5908, (uint32_t)save);                       /* 0x360DE */
            for (i = 0; i < cnt; i++)                                 /* 0x360EE (ecx = 0 kept across 0x648C8) */
                if (S8(AREA(list[i] + 1) + 0x0Eu + (uint32_t)k) != 0) {   /* 0x36112 [69(a+1) + k + 0xE31C3] */
                    used[i] = 1;                                      /* 0x36122 */
                    flag = 1;                                         /* 0x3612A */
                }
        }
        if (flag != 0) {                                              /* 0x36133 */
            f7AF28();                                                 /* 0x36139 World State peek */
            f7AFB8();                                                 /* 0x3613E Move List peek */
            *need = keep_need;                                        /* 0x36146 */
            goto again;                                               /* 0x36149 */
        }
        f7A700();                                                     /* 0x3614E back from the world snapshot */
        f7A85C();                                                     /* 0x36153 back from the move-list snapshot */
    }
    f7AD24();                                                         /* 0x36158 World State drop */
    f7AD7C();                                                         /* 0x3615D Move List drop */
}

int f3580C(int need, int t, int n, int mode, int s)
{
    uint32_t rec = NAT(n);                                            /* 0x3583B [ebp-0xc] */
    int32_t agri = S8(rec + 0x27u);                                   /* 0x35844 [ebp-0x68] */
    int32_t road = S8(rec + 0x28u);                                   /* 0x35850 [ebp-0x64] */
    int32_t start = need;                                             /* 0x35860 [ebp-0x2c] */
    int32_t tok_t = S8(AREA(t + 1) + 0x0Eu + (uint32_t)n);            /* 0x35876 [ebp-0x3c] */
    int32_t list[160], l2[160];                                       /* [ebp-0x394], [ebp-0x614] */
    uint8_t used[160];                                                /* [ebp-0x114] */
    int32_t cnt = 0, c2 = 0, left = 0, x = 0, i, a, keep, tries, mid, k, ek, c;   /* x: [ebp-0x28], f495C0's enemy (written by it first, D 0x495E1) */

    memset(list, 0, sizeof list);                                     /* port: filled by the callees */
    memset(l2, 0, sizeof l2);
    memset(used, 0, sizeof used);

    /* 1. (mode 0) the areas a conflict would release, code 0x20 */
    if (mode == 0) {                                                  /* 0x35882 */
        f48028(&cnt, list, t, n, 0x15);                               /* 0x3589D */
        if (cnt != 0) {                                               /* 0x358A5 */
            f48D44(&cnt, list, n, t, 1);                              /* 0x358C0 */
            for (i = 0; i < cnt; i++) {                               /* 0x358D0 [ebp-0x6c] */
                f7C838(list[i], n, 0, &left);                         /* 0x358EE */
                if (left == 0)                                        /* 0x358F6 */
                    continue;
                while (need > 0 &&                                    /* 0x358FA */
                       S8(AREA(list[i] + 1) + 0x24u + (uint32_t)n) > 0) {   /* 0x3591C */
                    f57CA4(list[i], t, n, 1, 1);                      /* 0x35940 */
                    ml_add(list[i], t, 1, 0x20);                      /* 0x35945..0x35978 */
                    need--;                                           /* 0x35982 */
                }
            }
        }
    }

    /* 2. the adjacent areas' surplus over Agriculture + the limit, code 0 / 0x1B */
    f48028(&cnt, list, t, n, 9);                                      /* 0x3599B */
    if (cnt != 0) {                                                   /* 0x359A0 */
        f48D44(&cnt, list, n, t, 1);                                  /* 0x359BB */
        for (i = 0; i < cnt && need > 0; i++) {                       /* 0x359C5, 0x359D1 [ebp-0x5c] */
            a = list[i];                                              /* 0x359E5 [ebp-0x14] */
            keep = agri + limit_of(a);                                /* 0x35A00..0x35A14 [ebp-0x60] */
            while (need > 0 && S8(AREA(a + 1) + 0x24u + (uint32_t)n) > 0 &&   /* 0x35A19, 0x35A35 */
                   S8(AREA(a + 1) + 0x0Eu + (uint32_t)n) > keep) {   /* 0x35A58 */
                f57CA4(a, t, n, 1, 1);                                /* 0x35A76 */
                ml_add(a, t, 1, mode == 0 ? 0 : 0x1B);                /* 0x35A7B..0x35AB6 */
                need--;                                               /* 0x35AC5 */
            }
        }
    }

    /* 3. the first plain sweep (10 rounds), code 1 / 0x1C */
    if (cnt > 0 && S8(rec + 0x5Du) > 0) {                             /* 0x35AD8, 0x35AE5 [+0x5A] sar 24 */
        i = 0;
        tries = 10;                                                   /* 0x35AF8 */
        memset(used, 0, 0xA0);                                        /* 0x35B0A */
        while (need > 0 && tries > 0) {                               /* 0x35B0F, 0x35B19 */
            if (used[i] == 0) {                                       /* 0x35B26 */
                if (sweep_plain(list[i], t, n, mode, 1, 0x1C, &x))
                    need--;                                           /* 0x35C28 */
                else
                    used[i] = 1;                                      /* 0x35C36 */
            }
            i++;                                                      /* 0x35C41 */
            if (i == cnt) {                                           /* 0x35C47 */
                i = 0;
                tries--;
            }
        }
    }

    /* 4. the second plain sweep, code 2 / 0x1D (the list re-read first, sorted only when the sweep runs) */
    f48028(&cnt, list, t, n, 9);                                      /* 0x35C71 */
    if (need > 0 && cnt != 0 && S8(rec + 0x5Du) > 0) {                /* 0x35C76..0x35C95 */
        f48D44(&cnt, list, n, t, 1);                                  /* 0x35CAC */
        i = 0;
        tries = 10;                                                   /* 0x35CB6 */
        memset(used, 0, 0xA0);                                        /* 0x35CCA */
        while (need > 0 && tries > 0) {                               /* 0x35CCF */
            if (used[i] == 0) {                                       /* 0x35CE6 */
                if (sweep_plain(list[i], t, n, mode, 2, 0x1D, &x))
                    need--;                                           /* 0x35DE9 */
                else
                    used[i] = 1;                                      /* 0x35DF7 */
            }
            i++;                                                      /* 0x35E02 */
            if (i == cnt) {                                           /* 0x35E08 */
                i = 0;
                tries--;
            }
        }
    }

    /* 5. the first guarded sweep with the look-ahead, code 3 / 0x1E */
    guarded_stage(&need, t, n, mode, list, cnt, used, 0, &x);            /* 0x35E21..0x3615D */

    /* 6. Roadbuilding: two steps through a middle territory, codes 0x12 / 0x1F, count bytes 0x33 and 0x4C */
    if (cnt > 0 && road != 0) {                                       /* 0x36162, 0x3616C */
        f4892C(t, &c2, l2, cnt, list, n, 9);                          /* 0x36192 */
        if (c2 > 0) {                                                 /* 0x3619A */
            f48D44(&c2, l2, n, t, 1);                                 /* 0x361B3 */
            for (i = 0; i < c2 && need > 0; i++) {                    /* 0x361BD, 0x361C9 */
                a = l2[i];                                            /* 0x361DD [ebp-0x14] */
                while (need > 0 && S8(AREA(a + 1) + 0x24u + (uint32_t)n) > 0 &&   /* 0x361E7, 0x36203 */
                       S8(AREA(a + 1) + 0x0Eu + (uint32_t)n) > 1) {  /* 0x3622F */
                    mid = f79128(a, t, n);                            /* 0x36241 [ebp-0x54] */
                    f57CA4(a, mid, n, 1, 1);                          /* 0x36259 */
                    ml_add(a, mid, 0x33, mode == 0 ? 0x12 : 0x1F);    /* 0x3625E..0x362A3 */
                    f57CA4(mid, t, n, 0, 1);                          /* 0x362B5 */
                    ml_add(mid, t, 0x4C, mode == 0 ? 0x12 : 0x1F);    /* 0x362BA..0x362FE */
                    need--;                                           /* 0x36306 */
                }
            }
        }
    }

    /* 7. by sea, when t is coastal and int16 [0xBB3F0] is set */
    if (need > 0 && (D8(TERR(t)) & 2) && S16(SEA_OK) != 0)            /* 0x36319..0x36341 */
        f36918(n, t, &need, (int)(int8_t)mode);                       /* 0x36350 movsx ecx, byte [ebp-0x38] */

    /* 8. the second guarded sweep (no territory-flag test; mode 2 also moves without f49A1C or the flag) */
    guarded_stage(&need, t, n, mode, list, cnt, used, 1, &x);            /* 0x36355..0x36675 */

    /* the end, by mode */
    if (mode == 0) {                                                  /* 0x3667A */
        if (S8(AREA(t + 1) + 0x0Eu + (uint32_t)n) >= 12 && S8(BIG_SITE) == 1)   /* 0x36697, 0x366A5 [0x107ED5] sar 24 */
            need = -1;                                                /* 0x366B2 */
        if (need == 0) {                                              /* 0x366B9 */
            c = S32(ML1_COUNT);                                       /* 0x366C3 */
            setD8(ML1 + 4u * (uint32_t)c, 0xC8);                      /* 0x366D1 */
            setD32(ML1_COUNT, (uint32_t)(c + 1));                     /* 0x366DB */
            setD8(ML1 + 4u * (uint32_t)c + 2u, 0);                    /* 0x366E0 [4(c+1) + 0x107692] */
            setD8(ML1 + 4u * (uint32_t)c + 1u, (uint8_t)t);           /* 0x366EA; +3 not written (D) */
            setD8(TST(t) + 2u, (uint8_t)(D8(TST(t) + 2u) | 2));       /* 0x366FC */
            setD8(TST(t), (uint8_t)n);                                /* 0x36714 */
            setD16(AREA(t + 1) + 0x37u, (uint16_t)(int16_t)S8(AREA(t + 1) + 0x0Eu + (uint32_t)n));   /* 0x3672D, 0x36740 */
            return 0;
        }
        setD8(TST(t) + 2u, (uint8_t)(D8(TST(t) + 2u) & 0xFD));        /* 0x3675F and byte, -3 */
        setD8(TST(t), 0);                                             /* 0x36774 */
        setD16(AREA(t + 1) + 0x37u, 0);                               /* 0x36787 */
        return need;
    }
    if (mode == 1) {                                                  /* 0x3679E */
        if (need == 0)                                                /* 0x367AA */
            return 0;
        if (D8(TST(t) + 2u) & 1)                                      /* 0x367C5 a city */
            return need;
        k = f7C7C4(t, n);                                             /* 0x367E7 */
        ek = S8(AREA(t + 1) + 0x0Eu + (uint32_t)k);                   /* 0x367EC [69(t+1) + k + 0xE31C3] sar 24 */
        if (ek > limit_of(t))                                         /* 0x36818 */
            return need;
        if (S8(AREA(t + 1) + 1u) != 1)                                /* 0x3682C..0x36838 */
            return need;
        if (start - need >= 2)                                        /* 0x36844 */
            return 0;
        if (start - need < 1)                                         /* 0x3684F cmp eax, ecx (1) */
            return need;
        if (tok_t <= 0)                                               /* 0x36857 */
            return need;
        return 0;                                                     /* 0x36861 */
    }
    if (mode == 2) {                                                  /* 0x3686F */
        if (need == 0 || start - need >= s) {                         /* 0x36883, 0x368D0 (jl: need) */
            c = S32(ML1_COUNT);                                       /* 0x36878 shl eax, 2: 4 * count (D) */
            setD8(ML1 + 4u * (uint32_t)c, 0xC9);                      /* 0x36887 / 0x368D8 */
            setD8(ML1 + 4u * (uint32_t)c + 2u, 0);                    /* 0x36890 / 0x368E1 */
            setD8(ML1 + 4u * (uint32_t)c + 1u, D8(TST(t)));           /* 0x368AF / 0x36900 [4(c+1) + 0x107691] */
            setD32(ML1_COUNT, (uint32_t)(c + 1));                     /* 0x368B6 / 0x36907; +3 not written (D) */
            return 0;
        }
    }
    return need;                                                      /* 0x3678F */
}

/* ---- 0x79128: a land territory between t1 and t2 ---------------------------------------------------------------- */

int f79128(int t1, int t2, int n)
{
    uint32_t l1 = D32(LANDADJ + 4u * (uint32_t)t1);                   /* 0x79145 [ebp-8] */
    uint32_t l2 = D32(LANDADJ + 4u * (uint32_t)t2);                   /* 0x79152 [ebp-4] */
    int32_t i, j, v, a;

    for (i = 1; hword(l1) >= i; i++) {                                /* 0x7916B movsx; cmp; jl */
        v = hword(l1 + 2u * (uint32_t)i);                             /* 0x79182 [ebp-0x10] */
        for (j = 1; hword(l2) >= j; j++) {                            /* 0x79197 */
            if (hword(l2 + 2u * (uint32_t)j) != v)                    /* 0x791AA */
                continue;
            a = v + 1;                                                /* 0x791B8 [ebp-0xc] */
            if ((D8(TST(v) + 2u) & 1) && D8(TST(v)) == (uint32_t)n)   /* 0x791C7, 0x791DE */
                return v;
            if (S8(AREA(a) + 1u) == 0 && !(D8(TST(v) + 2u) & 1))      /* 0x791F8, 0x79212 */
                return v;
            if (S8(AREA(a) + 1u) == 1 && S8(AREA(a) + 1u + (uint32_t)n) == n)   /* 0x79225, 0x7924A */
                return v;
        }
    }
    return -1;                                                        /* 0x79264 */
}
