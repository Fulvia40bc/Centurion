/* Module AISHIP of M7 batch 2 (docs/M6_PLAN.md, module SHIPAI and the AI parts of TAX, POP and CONFLICT): the
   computer players' ship maintenance and construction search and three small decisions of other phases. Literal
   translations; the address of each instruction a line comes from is in its comment. D = read in the code, G =
   inference. Prototypes and records: aiship.h.

     0x61254  f61254  a computer's ship maintenance (ship.c's CALL_F61254)                    1,388 bytes
     0x617C0  f617C0  the next maintenance subset                                               384
     0x62840  f62840  the construction search, -1 or the option (ship.c's CALL_F62840)        2,796
     0x6332C  f6332C  the search's clock budget                                               1,572
     0x639E8  f639E8  coastal city sites of leading nations (results unused by both callers)    264
     0x63D10  f63D10  the next ship combination                                               1,720
     0x66B54  f66B54  a computer annexes revolting cities (tax.c's stub)                        216
     0x66C2C  f66C2C  a computer removes its own cities (tax.c's stub)                          352
     0x680DC  f680DC  a computer short of stock places its tokens (pop.c's stub)              1,620
     0x5BF30  f5BF30  a computer's stock -> treasury transfer after plunder (conflict.c's stub)  480
   Already ported and called here: f63950, f63AF0, f63B7C, f63C6C (helpers2.c), f461BC (aimove.c), f353C4
   (aicity.c), f7B044, f48D44 (aiq.c), f6106C, f7A610 (ship.c), f5AAA4 (conflict.c, silent with 0), f78D98, f78F04,
   f78E74, f78D08 (rules.c), f57E28 (move.c), the World State Stack f7A8D0 / f7A940 / f7AC60 / f7ABD0 (stack.c) and
   f7AF28 / f7AD24 (mstack.c), the screen kit (f79270 kit.c, f78A20 / f3A9F4 ui_kit2.c, f3F670 / f3F624 / f3DA60 /
   f3D9FC mapdata.c, f7876C / f3CBBC map.c, f459E4 panel.c, f3CD6C / f3A3A0 / f3A044 / f3A0B0 / f3C9D0 / f3B05C).

   Not ported here (a call into another module of batch 2; the static stub carries the name, so a later translation
   that exports it collides with the stub at compile time instead of being shadowed by it):
     CAL1 (aical1.c): int f20C70(int p, int v)   the city of v that computer p takes (territory + 1; Treachery's
                      choice, also used by the tax revolt). Caller here: f66B54 (0x66B80).
   With -DAISHIP_TEST it goes to the test's recorder (port/test/aiship_test.c).
   PBEM_DROPPED: none; no routine here tests the PBEM bit 0x80 of 0xE1624 (D).

   Watcom runtime -> standard C: 0x7DB3D sprintf -> snprintf bounded to the destination (0xF5500: 500 bytes, as
   loop.c; the stack buffers: 100 bytes each, the room between the buffer and the next local); 0x8C897 itoa(v, buf, 10)
   -> "%d"; 0x7D5D0 memset -> memset on the locals and in g_ds; rep movsd/movsw record copies -> memcpy in g_ds;
   0x7D4AC (__CHK) dropped. No 0x78B0C / 0x78B24 trace call in these ten (D).

   0x8A080 clock (game.h's wat_clock, ui_kit2.c's virtual clock, which advances only inside f78A20's waits):
     - The busy waits keep the original's start: f61254 reads start = clock() at 0x612B0 and waits at the end until
       clock() >= start + 750 / 10 (0x6174F..0x6176B); f62840 until [0x10F2C8] + [ebp-0x40] / 10 (0x632FC..0x6331B,
       [ebp-0x40] = 750 only when it searched, else 0: no wait); f680DC until start (0x6814F) + 1000 / 10
       (0x686FF..0x6871B). None is scaled by /C ([0xBB3F4]), as in the original (M6_PLAN risk 5). busy_wait() below
       does the same wait with one f78A20 of the remaining time times 10 * [0xBB3F4] (f78A20 divides by both), so its
       loop "while target > clock()" is the original's and a headless run does not spin.
     - The time budget (M6_PLAN risk 3). f62840 sets [0x10F2C8] = clock() (0x6296C) after its pushes, then times each
       trial plan: [0x10F2D0] = clock() before f461BC, the difference after it added to [0x10F2CC] (and [0x10F2C0]),
       [0x10F2DC] (and [0x10F2C4]) counting them. f6332C (called once per search, when f63D10 enters stage 1) takes
       avg = [0x10F2CC] / [0x10F2DC] (unsigned) and elapsed = clock() - [0x10F2C8] and compares avg * C + elapsed with
       the budget int16 [0xBB3E8] * 9500 / 100 = 9500 hundredths (95 s), C = the triangular table int32 0xC7618[avail]
       (0, 1, 3, 6, ... 153 for avail = 0..18; avail = the sites not excluded). The budget bites (prunes the sites of
       the options past a limit, and shifts the stage-2 start [0x10F2E0]) when
         (a) any trial took at least one DOS clock step (avg >= 1) and avail >= 19: 0xC7618[19..] are not triangular
             numbers but the string pointers that follow the table (0xB6434..., about 746,548: risk 8, read through
             the data object), so avg * C is far over the budget; or
         (b) avg * C(avail) + elapsed > 9,500 with avail <= 18, e.g. avg > 62 hundredths (0.62 s per trial plan) with
             18 sites, or the search already 95 s old.
       In the port the trials (f461BC with [0xF5908] = 0, f79270 -560) do not wait, so the virtual clock does not move
       between [0x10F2C8] and f6332C: elapsed = 0, every trial time is 0, avg = 0 ([0x10F2DC] >= 1): the budget never
       bites, whatever avail. A DOSBox-X comparison must therefore use positions where the original stays inside it:
       check the dump at the f6332C call (BP 0x64361): [0x10F2D8] * [0x10F2E8] + [0x10F2D4] <= 9500 and, for (a),
       [0x10F2D8] == 0 or avail <= 18 (check_plan). The time words 0x10F2C0..0x10F2D8 themselves differ by design.

   game_rand f78AE4: one draw in f61254 (0x616EF, only when the treasury is above 0 and below the used ships), one in
   f62840 (0x632E3, only when options better than the plan without new ships survive). None elsewhere here (D).

   Stack discipline (D, kept exactly; the World State Stack only):
     f61254: push (0x61291), f7A610 (the phase's snapshot back), push (0x61312), plan, pop (0x6143C); treasury 0 or
       enough: pop (0x6174A); else push (0x614D5), each subset peeked by f617C0 (0x617DE), pop (0x615D2), pop
       (0x6174A). So 2 / 2 or 3 / 3: the final pop restores the state at entry.
     f62840: push (0x6294E), f7A610, push (0x62958), the baseline plan, peek (0x629A2); a search: push (0x62A48), each
       combination applied, planned and peeked (0x62FAC), pop (0x62FCA); then drop (0x632F2) and pop (0x632F7).
     f680DC: f7A8D0 (allocate once), push (0x681C8), the simulated expansion, f461BC, f5AAA4(0), pop (0x685C7), and
       f7ABD0 at the end (0x686FA: frees the stack's blocks, depth -1), as the original.
   The Move List Stack is not used here (the move lists keep the last trial's plan, as in the original).
   Comparison points (port_checkpoint, game.h; --stop-at ADDR N): 0x6174A (f61254's decision, before its last pop),
   0x632F2 (f62840's sorted options and pick, before the drop), 0x64366 (f6332C's budget applied), 0x686FA (f680DC's
   placement done, before f7ABD0): the original's instructions at those addresses, for DOSBox-X breakpoints.

   Globals written that the save does not hold (D; for the DOSBox-X comparison): the maintenance options 0x10F590..
   0x10F6B7; the construction options 0x10F6B8..0x110143 and 0x110144..0x110178; the excluded sites 0x10F220[160];
   the time and countdown words 0x10F2C0..0x10F2F0 (the time values differ by design); byte [0x108190] = 0
   (0x62892); 0xF7C9C / 0xF7CA0 (f680DC's round robin); the move lists 0x107694 / 0x1079B8 and their counts (the last
   trial plan); byte 0xF488C[n] (f680DC, saved) and everything the silent f461BC / f5AAA4(0) leave outside the World
   State Stack (aimove.c, conflict.c); the area bytes +0x3B + n (f66B54).

   1995 bugs (graded code / effect). In the computers' code an odd heuristic is not a bug (kept); only garbage reads,
   hangs and crashes are fixed:
     1. BUG(1995), fixed in both modes: f61254 passes its scrap[4] array to f617C0, which tests scrap[k] == 1 for all
        k < 4 (0x61859..0x61875), but f61254 writes only scrap[0 .. ships-1] (0x613FF / 0x61432). For a nation with
        fewer than 4 ships the other words are what the stack held at [ebp-0x80 + 4k] (D code; reachable whenever the
        treasury is above 0 and below the used ships, G). Sibling-frame analysis (D): f61254 is called by 0x60BD8 with
        no stack argument, at the depth of the housekeeping's f461BC (0x60F6A) of the nation before in the order, so
        both frames start at the same ebp E. f461BC's frame ends at E-0x60; its callees run at ebp E-0x68. f7AC60
        (0x4688B) pushes ecx, edx, esi, edi at E-0x6C..E-0x78 and its copy temporaries (edi) at E-0x7C, the last one
        0xF1593. Then f461BC's very last callee, f4D518 at 0x468D6 (show = [0xF5908] = 0 outside MOVEMENT), pushes
        ebx, ecx, esi, edi at E-0x6C..E-0x78 (0x4D525) and writes its [ebp-0x14] = E-0x7C only when show != 0
        (0x4D630); its callees run below E-0x10C, and 0x60BD8's f7A598 / f7AC60 after it stay above E-0x30. So
        scrap[1] = 0xF1593, scrap[2] = f461BC's edi = [0xF5908] = 0 (0x468C4) and scrap[3] = f461BC's final esi: 0
        (0x4634C / 0x465B9), or the last positive value of f46A4C (0x46759), which can be 1 (or the esi of 0x60BD8
        when neither ran). That last word is not observable from here (a register inside aimove.c's f461BC; modelling
        it needs aimove.c to keep it, G), and for the first nation of the order
        (preceded by 0x60BD8's screen set-up: f3CD6C, cat_show, malloc) and from round 7 (the housekeeping's busy
        wait calls clock() at that depth while the timer interrupt runs) the words come from library code and
        interrupt frames: they cannot be modelled. Fixed: the port zeroes scrap[ships..3], which is the original's
        behaviour whenever those words are not 1 (always for scrap[1] and scrap[2] in the case above, and for scrap[3]
        unless f461BC's final esi was 1). --original cannot reproduce the garbage and does the same. A DOSBox-X run
        that differs here: BP 0x614F0 (the first f617C0 call), dword [ebp-0x80 + 4k] for k >= ships equal to 1.
     2. f63D10's stage 1 / 2 type loops (0x63E33, 0x63FCF) never end when a + b + c is below 2 / 3; the stage is
        entered only when stage + 1 <= a + b + c (0x643AB) and the three counters do not change during a search (D),
        so it is unreachable; the port's loop stops instead of spinning (port guard, no change in play).
     3. f680DC's round robin (0x68637..0x68687) would spin with excess > 0 and nothing left to take; unreachable (D:
        the listed allocations minus the excess never drop below the stock, which is above 0 there); the port stops a
        pass that takes nothing (port guard).
   Kept as the original (AI choices, deterministic; D code unless marked):
     - f617C0 counts the mask's bits 0..3 even for ships that do not exist, so with 1..3 ships an option can keep
       fewer real ships than the treasury pays for (with scrap[ships..3] = 0, see 1.).
     - f61254's tie count (0x616BF) compares option[c] before testing c < count: it reads one option past the count
       (zeroed by the memset, or for 16 options the word 0x10F6C0 inside the construction options: through the data
       object, risk 8). With no option at all it still draws one game_rand (pick 0, all ships scrapped).
     - f61254 takes a ship's "use" from the first plan record's reason byte +5 and scraps a ship whose reason is
       negative; f6106C always gets tokenUse 0 (the array is only zeroed): no computer ever pays upkeep with a token.
     - f62840's levy check compares the move's ship index with j (0x62DF0), not ships + j (G: the new ship's index);
       a pair (stage 1) may use the same site twice on its first row (0x63E88: j starts at [0x110160] + 1 = 0 = i).
     - f62840 drops every option whose score is not above the plan without new ships; ties need the same ship count.
     - f6332C's stage-2 start [0x10F2E0] is a count of sites (min(avail, 4)) used as an option index by f63D10
       (0x6436F); the over-budget path's early return leaves its limit unused.
     - f639E8's results are read by neither caller (both pass stack arrays and ignore them).
     - f680DC: a cut of 1 per crowded area (setg, 0x68347, not tokens - limit); the plan's land moves index the
       allocation by their byte +0 up to 0xA0 inclusive (0x683D6 jg): 160 is one past the int32[160] array, the first
       word of the message buffer [ebp-0xB0] that follows it on the stack (the first four letters of the nation's name,
       as the sprintf at 0x6812D left them; risk 8, deterministic): kept, alloc[160] below. The writes there only
       change that dead buffer.
     - f5BF30 reads the AST square of nation record n - 1 (0x5BF50 imul (n - 1) * 199 + 0xF1525) with the table row
       n - 1 (G: record n meant); for n = 1 that is record 0's byte 0xF1528, read through the data object. The
       transfer is then the whole amount (value >= 2) or the ship count clamped to 0..amount.
     - f66B54 does not check f20C70's result: 0 would give t = -1 (G: unreachable, the victim has at least take cities).
   Texts (Erik's ruling of 19 Nov: obvious errors only; none changed here): "%s removes a city in %s" (0xB8A1C) has no
   final period while its twin "%s annexes a city in %s." has one, and "%s, choosing number units to transfer from
   stock to treasury." (0xB5D16) lacks "of" (TEXT_SWEEP.md); a missing period / word is not on the ruling's list
   (as cal2.c and conflict.c decided): for Erik.
   Easter eggs / developer leftovers (M6_PLAN egg 12, OVERVIEW §6; none new): the counters shown to the player, "%s
   considers building ships. (%d,?,?)" (two copies of the string, 0xB6358 / 0xB637E) counting the sites down, then
   "(0,%d,%d)" with the pair and triple estimates (for 19+ sites the pair count is a string pointer, about 746,548,
   from the table overrun above); "%s considers ship maintenance. %s" with the | / - \ spinner (0xBB404). */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "aiship.h"
#include "aicity.h"
#include "aimove.h"
#include "aiq.h"
#include "conflict.h"
#include "fg.h"
#include "game.h"
#include "helpers2.h"
#include "kit.h"
#include "map.h"
#include "mapdata.h"
#include "move.h"
#include "mstack.h"
#include "panel.h"
#include "rules.h"
#include "ship.h"
#include "stack.h"
#include "state.h"
#include "ui.h"

#define NAT(n)       (0xF14CCu + 199u * (uint32_t)(n))
#define AREA(r)      (0xE31B8u + 69u * (uint32_t)(r))            /* area record r = territory r - 1 */
#define TS(t)        (0xE2C18u + 3u * (uint32_t)(t))              /* territory state t */
#define TERR(t)      (0xE1638u + 35u * (uint32_t)(t))             /* territory record t */
#define NAME(n)      DSTR(D32(0xC768Cu + 4u * (uint32_t)(n)))
#define TNAME4(x)    DSTR(D32(0xC77D4u + 4u * (uint32_t)(x)))     /* the name table read at 0xC77D4 + 4x (x = t + 1) */
#define MSG          0xF5500u    /* char [500] */
#define PHASE        0xF4102u    /* uint8 */
#define CDIV         0xBB3F4u    /* int32: 1, 10 with /C */
#define SHORT_FLAG   0xF488Cu    /* uint8 [10] */
#define SMOVES       0x1079B8u   /* ship moves, 6 bytes */
#define SMOVECNT     0x107D3Cu
#define LMOVES       0x107694u   /* land moves, 4 bytes */
#define LMOVECNT     0x1079B4u
#define CHOICE_CNT   0xF7C9Cu
#define CHOICE       0xF7CA0u
#define BITS         0xBC800u    /* int32 [4] 1, 2, 4, 8 */
#define SPIN         0xBB404u    /* char *[4] "|", "/", "-", "\" */
#define TRI          0xC7618u    /* int32 triangular numbers (19 entries, then other data) */
#define TPLUS        0x10F2F4u   /* int32 [6] their running sums (f60A80) */
/* maintenance options */
#define MOPT(i)      (0x10F590u + 18u * (uint32_t)(i))            /* int32 keep[4], int16 score +0x10 */
#define MOPT_CNT     0x10F6B0u
#define MOPT_TOP     0x10F6B4u
/* construction search */
#define T_SUM        0x10F2C0u
#define T_RUNS       0x10F2C4u
#define T_START      0x10F2C8u
#define T_TOTAL      0x10F2CCu
#define T_ONE        0x10F2D0u
#define T_ELAPSED    0x10F2D4u
#define T_AVG        0x10F2D8u
#define T_COUNT      0x10F2DCu
#define STAGE2_FROM  0x10F2E0u
#define CD_SITES     0x10F2E4u
#define CD_PAIRS     0x10F2E8u
#define CD_TRIPLES   0x10F2ECu
#define STAGE2_TO    0x10F2F0u
#define EXCL         0x10F220u   /* uint8 [160] */
#define SITES_N      0x10F30Cu
#define SITES        0x10F310u   /* int32 [] */
#define COPT(i)      (0x10F6B8u + 18u * (uint32_t)(i))
#define COPT_CNT     0x110144u
#define SLOTS        0x110148u
#define N_TREAS      0x11014Cu
#define N_ONE        0x110150u
#define N_LEVY       0x110154u
#define STAGE        0x110158u
#define IDX0         0x11015Cu
#define IDX1         0x110160u
#define IDX2         0x110164u
#define X168         0x110168u
#define C_TREAS      0x11016Cu   /* uint8 [3] */
#define C_LEVY       0x110170u   /* uint8 [3] */
#define C_SITE       0x110174u   /* uint8 [3] */
#define C_COUNT      0x110178u

/* ---- calls into code that is not ported yet (see the file comment) ----------------------------------------------- */

#ifdef AISHIP_TEST
int aiship_test_f20C70(int p, int v);
#define f20C70(p, v) aiship_test_f20C70(p, v)
#else
#include "aical1.h"                             /* f20C70 (M7 batch 2, linked) */

#endif

/* ---- small helpers ----------------------------------------------------------------------------------------------- */

static void add8(uint32_t a, int v) { setD8(a, (uint8_t)(D8(a) + (uint8_t)v)); }   /* byte add / sub / inc / dec */

/* a busy wait "while (unsigned) target > clock()" (0x6174F, 0x632FC, 0x686FF): one f78A20 that waits until the same
   target (f78A20 divides its argument by [0xBB3F4] and 10); none when the clock is already there */
static void busy_wait(uint32_t target)
{
    uint32_t now = wat_clock();

    if (target > now)
        f78A20((int)((target - now) * 10u * D32(CDIV)));
}

/* the 18-byte record swaps of the bubble sorts (rep movsd x4, movsw through a stack temporary) */
static void swap18(uint32_t a, uint32_t b)
{
    uint8_t tmp[18];

    memcpy(tmp, DP(b), 18);
    memcpy(DP(b), DP(a), 18);
    memcpy(DP(a), tmp, 18);
}

/* the construction options by capacity +0x11 descending (int8): f62840 0x6309D..0x6316C, f6332C 0x634A9..0x6357E and
   0x636B3..0x63787 (three copies of the same loops) */
static void sort_capacity(void)
{
    int32_t i, j;

    for (i = S32(COPT_CNT) - 2; i >= 0; i--)
        for (j = 0; j <= i; j++)
            if (S8(COPT(j + 1) + 0x11u) > S8(COPT(j) + 0x11u))          /* cmp dl, cl; jle (signed) */
                swap18(COPT(j + 1), COPT(j));
}

/* the construction options by score +0 descending (int16): f62840 0x63171..0x63248, f6332C 0x63583..0x6365B and
   0x6378C..0x63864 */
static void sort_score(void)
{
    int32_t i, j;

    for (i = S32(COPT_CNT) - 2; i >= 0; i--)
        for (j = 0; j <= i; j++)
            if (S16(COPT(j + 1)) > S16(COPT(j)))                        /* cmp ax, bx; jle (signed) */
                swap18(COPT(j + 1), COPT(j));
}

/* ---- 0x639E8: coastal city sites of the nations ahead of n ------------------------------------------------------- */

void f639E8(int n, int32_t *count, int32_t *list)
{
    int32_t th;                                                         /* [ebp-0x4] */
    int32_t t, r, k;                                                    /* [ebp-0xC], [ebp-0x8], [ebp-0x10] */

    th = S8(0xF06C0u + (uint32_t)n);                                    /* 0x63A05 dword [n + 0xF06BD] sar 24 */
    *count = 0;                                                         /* 0x63A14 */
    for (t = 0; t < 0x9D; t++) {                                        /* 0x63A27 cmp 0x9D; jge */
        r = t + 1;                                                      /* 0x63A38 */
        if ((D8(TERR(t) + 1u) & 0x40u) == 0)                            /* 0x63A52 test dl, 0x40 */
            continue;
        if ((D8(TERR(t)) & 0x02u) == 0)                                 /* 0x63A68 test byte [.. + 0xE1638], 2 */
            continue;
        if (S8(AREA(r) + 1u) != 1)                                      /* 0x63A7B dword [.. + 0xE31B6] sar 24; cmp 1 */
            continue;
        for (k = 0; k < 9; k++) {                                       /* 0x63A97 */
            if (S8(0xF06C1u + (uint32_t)k) <= th)                       /* 0x63AA0 dword [k + 0xF06BE] sar 24; jle */
                continue;
            if (S8(AREA(r) + 0x0Fu + (uint32_t)k) == 0)                 /* 0x63AC2 dword [.. + 0xE31C4] sar 24 */
                continue;
            if (*count < 160)                                           /* port guard (the callers' 160 entries) */
                list[*count] = t;                                       /* 0x63AE7 */
            (*count)++;                                                 /* 0x63ADF */
        }
    }
}

/* ---- 0x617C0: the next maintenance subset ------------------------------------------------------------------------ */

int f617C0(int n, int budget, const int32_t *tokenUse, const int32_t *scrap, int32_t *tried)
{
    uint32_t nat;                                                       /* [ebp-0xC] */
    int32_t m, mask, k, bits, pc;                                       /* [ebp-0x20], [ebp-0x18], [ebp-0x1C], [ebp-0x14], [ebp-0x10] */

    f7AF28();                                                           /* 0x617DE the trial state back */
    nat = NAT(n);                                                       /* 0x617F1 */
    for (m = S32(MOPT_TOP); m >= 0; m--) {                              /* 0x617F4, 0x61804 jl */
        mask = m;                                                       /* 0x61811 */
        for (k = 0; k < 4; k++)                                         /* 0x61821 */
            if (tokenUse[k] == 1)                                       /* 0x61832 cmp ebx, 1 */
                mask |= S32(BITS + 4u * (uint32_t)k);                   /* 0x61841 */
        for (k = 0; k < 4; k++)                                         /* 0x61853 */
            if (scrap[k] == 1)                                          /* 0x61864 */
                mask &= ~S32(BITS + 4u * (uint32_t)k);                  /* 0x61873 not; and */
        if ((uint32_t)mask >= 16)                                       /* port guard: tried has 16 entries (mask <= 15
                                                                           with the data's bits, G) */
            continue;
        if (tried[mask] != 0)                                           /* 0x61885 */
            continue;
        tried[mask] = 1;                                                /* 0x61896 */
        pc = 0;                                                         /* 0x6189C */
        bits = mask;                                                    /* 0x618A2 */
        for (k = 0; k < 4; k++) {                                       /* 0x618B0 */
            pc += bits & 1;                                             /* 0x618BC */
            bits >>= 1;                                                 /* 0x618BF sar */
        }
        if (pc != budget)                                               /* 0x618C7 */
            continue;
        for (k = 0; (int32_t)D8(nat + 0x40u) > k; k++)                  /* 0x618E8 the ships, re-read */
            setD32(MOPT(S32(MOPT_CNT)) + 4u * (uint32_t)k,              /* 0x6191D */
                   (mask & S32(BITS + 4u * (uint32_t)k)) != 0);         /* 0x61913 test; setne */
        return 1;                                                       /* 0x61925 */
    }
    return 0;                                                           /* 0x61933 */
}

/* ---- 0x61254: a computer's ship maintenance ---------------------------------------------------------------------- */

void f61254(int n)
{
    uint32_t nat;                                                       /* [ebp-0x20] */
    uint32_t start;                                                     /* [ebp-0x24] */
    int32_t wait;                                                       /* [ebp-0x2C] */
    int32_t cnt639, list639[160];                                       /* [ebp-0x50], [ebp-0x364] (unused results) */
    uint8_t phase;                                                      /* [ebp-0x18] */
    int32_t use[4];                                                     /* [ebp-0x70] */
    int32_t scrap[4];                                                   /* [ebp-0x80] */
    int32_t tokenUse[4];                                                /* [ebp-0x60] */
    int32_t spare[4];                                                   /* [ebp-0x90], zeroed, never read */
    int32_t tried[16];                                                  /* [ebp-0xE4] */
    int32_t used, budget, spin, i, j, k, c, pick, cnt;                  /* [ebp-0x44], [ebp-0x3C], [ebp-0x28], [ebp-0x48], [ebp-0x4C], .., [ebp-0x34], [ebp-0x1C] */

    nat = NAT(n);                                                       /* 0x6127D */
    if (D8(nat + 0x40u) == 0)                                           /* 0x6128B no ships */
        return;
    f7A940();                                                           /* 0x61291 */
    f7A610();                                                           /* 0x61296 the phase's snapshot back */
    /* 0x6129B [ebp-0x30] = 4 + (Clothmaking +0x25 != 0): the movement points, never read */
    start = wat_clock();                                                /* 0x612B0 */
    snprintf((char *)DP(MSG), 0x1F4, DSTR(0xB61FBu), NAME(n));          /* 0x612CD "%s considers ship maintenance." */
    f79270(-550, n, (int32_t)S16(nat + 0x45u) - 1, DSTR(MSG), 0);       /* 0x612F5 ebx = ship 0's territory */
    wait = 750;                                                         /* 0x612FA */
    f639E8(n, &cnt639, list639);                                        /* 0x6130D */
    f7A940();                                                           /* 0x61312 */
    phase = D8(PHASE);                                                  /* 0x61317 */
    f353C4(n);                                                          /* 0x61322 */
    f461BC(n);                                                          /* 0x6132A the plan with every ship */
    setD8(PHASE, phase);                                                /* 0x61332 */
    for (i = 0; i < 4; i++)                                             /* 0x61344 */
        use[i] = -1;                                                    /* 0x6134D */
    memset(spare, 0, sizeof spare);                                     /* 0x61364 */
    memset(tokenUse, 0, sizeof tokenUse);                               /* 0x61373 */
    memset(DP(MOPT(0)), 0, 0x120);                                      /* 0x61386 */
    /* BUG(1995) 1 (file comment): f617C0 reads scrap[ships..3], which the original never writes (stack words of
       sibling frames and interrupts, not reproducible): zeroed here in both modes */
    memset(scrap, 0, sizeof scrap);
    for (i = 0; i < S32(SMOVECNT); i++) {                               /* 0x61396 */
        k = S8(SMOVES + 6u * (uint32_t)i + 3u);                         /* 0x613AD dword [6i + 0x1079B8] sar 24 */
        if (k < 0 || k > 3)                                             /* port guard: a ship index (G: < ships <= 4) */
            continue;
        if (use[k] == -1)                                               /* 0x613BB */
            use[k] = S8(SMOVES + 6u * (uint32_t)i + 5u);                /* 0x613CC dword [6i + 0x1079BA] sar 24 */
    }
    used = 0;                                                           /* 0x613F4 [ebp-0x44] ([ebp-0x40] = 0 too) */
    for (i = 0; (int32_t)D8(nat + 0x40u) > i; i++) {                    /* 0x61411 */
        if (use[i] >= 0) {                                              /* 0x6142E test; jge 0x613FF */
            scrap[i] = 0;                                               /* 0x61401 */
            used++;                                                     /* 0x61408 */
        } else
            scrap[i] = 1;                                               /* 0x61432 */
    }
    f7AC60();                                                           /* 0x6143C */
    if (S8(nat + 0x3Eu) == 0) {                                         /* 0x61444 dword [+0x3B] sar 24; jne */
        for (i = 0; (int32_t)D8(nat + 0x40u) > i; i++)                  /* 0x61459 */
            if (scrap[i] == 0 && tokenUse[i] == 0)                      /* 0x61474, 0x6147B */
                scrap[i] = 1;                                           /* 0x61485 */
    } else if (S8(nat + 0x3Eu) < used) {                                /* 0x61492 cmp eax, edx; jge 0x6174A */
        budget = 0 + S8(nat + 0x3Eu);                                   /* 0x614AD [ebp-0x40] (0) + treasury */
        setD32(MOPT_TOP, 15);                                           /* 0x614C4 */
        memset(tried, 0, sizeof tried);                                 /* 0x614CA 0x40 bytes */
        setD32(MOPT_CNT, 0);                                            /* 0x614CF */
        f7A940();                                                       /* 0x614D5 */
        spin = 0;                                                       /* 0x614DA */
        while (f617C0(n, budget, tokenUse, scrap, tried) != 0) {        /* 0x614F0 */
            k = spin++;                                                 /* 0x61500 */
            snprintf((char *)DP(MSG), 0x1F4, DSTR(0xB621Au), NAME(n),   /* 0x61523 "%s considers ship maintenance. %s" */
                     DSTR(D32(SPIN + 4u * (uint32_t)(k & 3))));
            f79270(-560, n, -200, DSTR(MSG), 0);                        /* 0x6153F */
            for (i = (int32_t)D8(nat + 0x40u) - 1; i >= 0; i--)         /* 0x6154F */
                if (S32(MOPT(S32(MOPT_CNT)) + 4u * (uint32_t)i) == 0)   /* 0x61575 */
                    f78D98(i, n);                                       /* 0x61586 the ship is scrapped in the trial */
            phase = D8(PHASE);                                          /* 0x6158D */
            f461BC(n);                                                  /* 0x61598 */
            setD8(PHASE, phase);                                        /* 0x615A0 */
            c = f7B044(n);                                              /* 0x615A8 */
            cnt = S32(MOPT_CNT);                                        /* 0x615AD */
            setD16(MOPT(cnt) + 0x10u, (uint16_t)c);                     /* 0x615BF */
            setD32(MOPT_CNT, (uint32_t)(cnt + 1));                      /* 0x615C7 */
        }
        f7AC60();                                                       /* 0x615D2 */
        for (i = S32(MOPT_CNT) - 2; i >= 0; i--)                        /* 0x615D7, 0x615F3 */
            for (j = 0; j <= i; j++)                                    /* 0x61610 jg */
                if (S16(MOPT(j + 1) + 0x10u) > S16(MOPT(j) + 0x10u))    /* 0x61643 cmp ax, bx; jle */
                    swap18(MOPT(j + 1), MOPT(j));                       /* 0x61648..0x616B3 */
        c = 0;                                                          /* 0x616BC */
        while (S16(MOPT(0) + 0x10u) == S16(MOPT(c) + 0x10u)             /* 0x616D2 (option c read first: c may be the count) */
               && c < S32(MOPT_CNT))                                    /* 0x616DF */
            c++;                                                        /* 0x616EA */
        pick = (int32_t)((uint32_t)(f78AE4() * c) >> 15);               /* 0x616EF imul; shr 0xF */
        for (i = 0; (int32_t)D8(nat + 0x40u) > i; i++)                  /* 0x6170B */
            scrap[i] = S32(MOPT(pick) + 4u * (uint32_t)i) == 0;         /* 0x6173C sete */
    }
    port_checkpoint(0x6174Au);                                          /* port (M7): the decision made (comparison point) */
    f7AC60();                                                           /* 0x6174A */
    busy_wait(start + (uint32_t)(wait / 10));                           /* 0x6174F..0x6176B (not scaled by /C) */
    for (i = (int32_t)D8(nat + 0x40u) - 1; i >= 0; i--)                 /* 0x61779, 0x61784 jl */
        f6106C(i, scrap[i], (int32_t)S16(nat + 0x45u + 2u * (uint32_t)i), tokenUse[i], n);   /* 0x617AD */
}

/* ---- 0x6332C: the search's clock budget -------------------------------------------------------------------------- */

void f6332C(int total)
{
    int32_t budget;                                                     /* [ebp-0x20] */
    int32_t avail;                                                      /* [ebp-0x18] */
    int32_t i;                                                          /* [ebp-0x34] */
    int32_t limit;                                                      /* [ebp-0x30] */
    uint32_t rem;                                                       /* [ebp-0x2C] / [ebp-0x28] */
    uint32_t y;                                                         /* [ebp-0x1C] */
    int32_t x;                                                          /* [ebp-0x24] */

    budget = (int32_t)S16(0xBB3E8u) * 0x251C / 100;                    /* 0x6334D imul 9500; idiv 100 */
    setD32(STAGE2_TO, D32(COPT_CNT));                                   /* 0x63369 */
    avail = 0;                                                          /* 0x6336E */
    for (i = 0; i < S32(SITES_N); i++)                                  /* 0x6337C */
        if (D8(EXCL + D32(SITES + 4u * (uint32_t)i)) == 0)              /* 0x63391 */
            avail++;                                                    /* 0x6339E */
    setD32(CD_PAIRS, D32(TRI + 4u * (uint32_t)avail));                  /* 0x633AD (avail >= 19 reads past the table) */
    if (S32(T_COUNT) == 0)                                              /* 0x633B2 */
        setD32(T_COUNT, 1);                                             /* 0x633BB */
    setD32(T_AVG, D32(T_TOTAL) / D32(T_COUNT));                         /* 0x633D2 div (unsigned) */
    setD32(T_ELAPSED, wat_clock() - D32(T_START));                      /* 0x633D9 clock() - [0x10F2C8] */
    if (D32(T_AVG) * D32(CD_PAIRS) + D32(T_ELAPSED) > (uint32_t)budget) {   /* 0x63401 cmp; jbe 0x636B3 (unsigned) */
        if (avail <= 3)                                                 /* 0x6340D cmp 3; jle (return) */
            return;
        if (D32(T_ELAPSED) > (uint32_t)budget)                          /* 0x63419 cmp esi, edx; jbe */
            limit = 3;                                                  /* 0x6341D */
        else {
            rem = ((uint32_t)budget - D32(T_ELAPSED)) / D32(T_AVG);     /* 0x63433 div */
            limit = 15;                                                 /* 0x63438 */
            for (i = 0; i < 15; i++) {                                  /* 0x6344C */
                if ((int32_t)rem == S32(TRI + 4u * (uint32_t)i))        /* 0x6345F je 0x63483 */
                    break;
                if (S32(TRI + 4u * (uint32_t)i) >= (int32_t)rem)        /* 0x63470 cmp eax, esi; jge (signed) */
                    continue;
                if ((int32_t)rem >= S32(TRI + 4u * (uint32_t)(i + 1)))  /* 0x6347A cmp; jge (signed) */
                    continue;
                break;                                                  /* 0x63483 */
            }
            if (i < 15)
                limit = i;                                              /* 0x63486 */
        }
        if (limit < 3)                                                  /* 0x63489 */
            limit = 3;                                                  /* 0x6348F */
        if (limit > avail) {                                            /* 0x63499 cmp; jle */
            limit = avail;                                              /* 0x634A1 (never read: the early return) */
            (void)limit;
            return;                                                     /* 0x634A4 */
        }
        sort_capacity();                                                /* 0x634A9 */
        sort_score();                                                   /* 0x63583 */
        for (i = limit; i < S32(COPT_CNT); i++)                         /* 0x63660 */
            setD8(EXCL + D8(COPT(i) + 3u), 1);                          /* 0x63691 the option's first site */
        setD32(CD_PAIRS, D32(TRI + 4u * (uint32_t)limit));              /* 0x636A4 */
        return;
    }
    sort_capacity();                                                    /* 0x636B3 */
    sort_score();                                                       /* 0x6378C */
    if (total > 2)                                                      /* 0x63869 cmp [ebp-0x3C], 2; jle */
        setD32(STAGE2_FROM, (uint32_t)(avail > 4 ? 4 : avail));         /* 0x6386F..0x6387F */
    else
        setD32(STAGE2_FROM, 0);                                         /* 0x63888 */
    x = S32(TPLUS + 4u * D32(STAGE2_FROM));                             /* 0x63893 */
    y = D32(T_AVG) * D32(CD_PAIRS) + D32(T_ELAPSED);                    /* 0x638A2..0x638B1 */
    if (D32(T_AVG) * (uint32_t)x + y > (uint32_t)budget) {              /* 0x638B9..0x638C3 jbe (unsigned) */
        rem = ((uint32_t)budget - y) / D32(T_AVG);                      /* 0x638D9 div */
        setD32(STAGE2_FROM, 4);                                         /* 0x638E5 */
        for (i = 0; i <= 4; i++) {                                      /* 0x638F5 cmp 4; jg */
            if ((int32_t)rem == S32(TPLUS + 4u * (uint32_t)i)) {        /* 0x63908 je */
                setD32(STAGE2_FROM, (uint32_t)i);                       /* 0x6392F */
                break;
            }
            if (S32(TPLUS + 4u * (uint32_t)i) >= (int32_t)rem)          /* 0x63919 jge (signed) */
                continue;
            if ((int32_t)rem >= S32(TPLUS + 4u * (uint32_t)(i + 1)))    /* 0x63923 jge (i = 4 reads 0x10F308) */
                continue;
            setD32(STAGE2_FROM, (uint32_t)i);                           /* 0x6392F */
            break;
        }
    }
    setD32(CD_TRIPLES, D32(TPLUS + 4u * D32(STAGE2_FROM)));             /* 0x63940 */
}

/* ---- 0x63D10: the next ship combination -------------------------------------------------------------------------- */

/* n's tokens at site s: byte [69(s + 1) + (n - 1) + 0xE31C7], compared as int8 with the type (cmp al, bl; jle) */
#define SITE_TOK(s)  S8(0xE31C7u + 69u * (uint32_t)((s) + 1) + (uint32_t)nm1)
#define SITE(i)      S32(SITES + 4u * (uint32_t)(i))

int f63D10(int n)
{
    int32_t nm1;                                                        /* [ebp-0x30] */
    int32_t a, b, c, total;                                             /* [ebp-0x24], [ebp-0x20], [ebp-0x1C], [ebp-0x18] */
    int8_t ty[3] = { 0, 0, 0 };                                         /* [ebp-0x28], [ebp-0x27], [ebp-0x26] */
    int32_t i = 0, j = 0, k = 0, q;                                     /* [ebp-0x40], [ebp-0x3C], [ebp-0x38], [ebp-0x2C] */
    int32_t s;
    uint32_t stage;

    nm1 = n - 1;                                                        /* 0x63D2C */
    a = S32(N_TREAS);                                                   /* 0x63D34 */
    b = S32(N_ONE);                                                     /* 0x63D3C */
    c = S32(N_LEVY);                                                    /* 0x63D44 */
    total = a + b + c;                                                  /* 0x63D52 */

L_63D55:
    stage = D32(STAGE);                                                 /* 0x63D55 (unsigned compares) */
    if (stage < 1)
        goto L_63D77;                                                   /* 0x63D5D jb 0x641F8; test; je 0x63D77 */
    if (stage == 1)
        goto L_63E2E;                                                   /* 0x63D63 jbe */
    if (stage == 2)
        goto L_63FCA;                                                   /* 0x63D6C */
    goto L_64200;                                                       /* 0x63D72 */

L_63D77:   /* stage 0: one ship, type 0 if treasury ships remain, else 1 if one-and-one, else 2 */
    if (S32(N_TREAS) > 0)                                               /* 0x63D77 cmp [0x11014C], 0; jle */
        ty[0] = 0;                                                      /* 0x63D82 */
    else
        ty[0] = (int8_t)((S32(N_ONE) <= 0) + 1);                        /* 0x63D8F setle; inc */
    for (i = S32(IDX0) + 1; i < S32(SITES_N); i++) {                    /* 0x63D9D, 0x63DB7 jge 0x64200 */
        if (SITE_TOK(SITE(i)) > ty[0]) {                                /* 0x63DE0 cmp al, bl; jle 0x63DA2 */
            setD32(C_COUNT, 1);                                         /* 0x63DE4 */
            setD8(C_SITE, (uint8_t)SITE(i));                            /* 0x63DF8 */
            setD8(C_TREAS, (uint8_t)(2 - ty[0]));                       /* 0x63E04 */
            setD8(C_LEVY, (uint8_t)ty[0]);                              /* 0x63E12 */
            setD32(IDX0, D32(IDX0) + 1u);                               /* 0x63E1E */
            return 1;
        }
        setD32(IDX0, D32(IDX0) + 1u);                                   /* 0x63DA2 */
    }
    goto L_64200;

L_63E2E:   /* stage 1: pairs */
    for (q = 0; q < 2; ) {                                              /* 0x63E33 */
        if (a > 0) {                                                    /* 0x63E3C */
            a--;
            ty[q++] = 0;                                                /* 0x63E4E */
        } else if (b > 0) {                                             /* 0x63E57 */
            b--;
            ty[q++] = 1;                                                /* 0x63E67 */
        } else if (c > 0) {                                             /* 0x63E71 */
            c--;
            ty[q++] = 2;                                                /* 0x63E81 */
        } else
            break;   /* port guard: the original loops for ever here (BUG(1995) 2, unreachable) */
    }
    i = S32(IDX0) + 1;                                                  /* 0x63E8E */
    goto L_63F4F;
L_63E96:
    setD32(IDX1, D32(IDX1) + 1u);                                       /* 0x63E96 */
    j++;                                                                /* 0x63E9F */
L_63EA2:
    if (j >= S32(SITES_N))                                              /* 0x63EA5 */
        goto L_63F36;
    s = SITE(j);
    if (D8(EXCL + (uint32_t)s) != 0)                                    /* 0x63EC1 */
        goto L_63E96;
    if (SITE_TOK(s) <= ty[1])                                           /* 0x63EE7 cmp al, [ebp-0x27]; jle */
        goto L_63E96;
    setD8(C_SITE + 1u, (uint8_t)s);                                     /* 0x63EF6 */
    setD8(C_TREAS + 1u, (uint8_t)(2 - ty[1]));                          /* 0x63F02 */
    setD8(C_LEVY + 1u, (uint8_t)ty[1]);                                 /* 0x63F0A */
    setD32(C_COUNT, 2);                                                 /* 0x63F1A */
    setD32(IDX1, D32(IDX1) + 1u);                                       /* 0x63F26 */
    return 1;
L_63F36:
    setD32(IDX0, D32(IDX0) + 1u);                                       /* 0x63F3D */
    setD32(IDX1, D32(IDX0));                                            /* 0x63F43 */
    i++;                                                                /* 0x63F4C */
L_63F4F:
    if (i >= S32(SITES_N))                                              /* 0x63F52 */
        goto L_64200;
    s = SITE(i);
    if (D8(EXCL + (uint32_t)s) != 0)                                    /* 0x63F6E */
        goto L_63F36;
    if (SITE_TOK(s) <= ty[0])                                           /* 0x63F94 */
        goto L_63F36;
    setD8(C_SITE, (uint8_t)s);                                          /* 0x63FA3 */
    setD8(C_TREAS, (uint8_t)(2 - ty[0]));                               /* 0x63FAF */
    setD8(C_LEVY, (uint8_t)ty[0]);                                      /* 0x63FB7 */
    j = S32(IDX1) + 1;                                                  /* 0x63FC2 (the first row starts at j = i = 0) */
    goto L_63EA2;

L_63FCA:   /* stage 2: triples */
    for (q = 0; q < 3; ) {                                              /* 0x63FCF */
        if (a > 0) {                                                    /* 0x63FD8 */
            a--;
            ty[q++] = 0;                                                /* 0x63FEA */
        } else if (b > 0) {                                             /* 0x63FF3 */
            b--;
            ty[q++] = 1;                                                /* 0x64003 */
        } else if (c > 0) {                                             /* 0x6400D */
            c--;
            ty[q++] = 2;                                                /* 0x6401D */
        } else
            break;   /* port guard (BUG(1995) 2, unreachable) */
    }
    i = S32(IDX0) + 1;                                                  /* 0x6402A */
    goto L_64181;
L_64032:
    setD32(IDX2, D32(IDX2) + 1u);                                       /* 0x64032 */
    k++;                                                                /* 0x6403B */
L_6403E:
    if (k >= S32(SITES_N))                                              /* 0x64041 */
        goto L_640D2;
    s = SITE(k);
    if (D8(EXCL + (uint32_t)s) != 0)                                    /* 0x6405D */
        goto L_64032;
    if (SITE_TOK(s) <= ty[2])                                           /* 0x64083 cmp al, [ebp-0x26] */
        goto L_64032;
    setD8(C_SITE + 2u, (uint8_t)s);                                     /* 0x64092 */
    setD8(C_TREAS + 2u, (uint8_t)(2 - ty[2]));                          /* 0x6409E */
    setD8(C_LEVY + 2u, (uint8_t)ty[2]);                                 /* 0x640A6 */
    setD32(C_COUNT, 3);                                                 /* 0x640B6 */
    setD32(IDX2, D32(IDX2) + 1u);                                       /* 0x640C2 */
    return 1;
L_640D2:
    setD32(IDX1, D32(IDX1) + 1u);                                       /* 0x640D9 */
    setD32(IDX2, D32(IDX1));                                            /* 0x640DF */
    j++;                                                                /* 0x640E8 */
L_640EB:
    if (j >= S32(SITES_N))                                              /* 0x640EE */
        goto L_64162;
    s = SITE(j);
    if (D8(EXCL + (uint32_t)s) != 0)                                    /* 0x64106 */
        goto L_640D2;
    if (SITE_TOK(s) <= ty[1])                                           /* 0x6412C */
        goto L_640D2;
    setD8(C_SITE + 1u, (uint8_t)s);                                     /* 0x6413B */
    setD8(C_TREAS + 1u, (uint8_t)(2 - ty[1]));                          /* 0x64147 */
    setD8(C_LEVY + 1u, (uint8_t)ty[1]);                                 /* 0x6414F */
    k = S32(IDX2) + 1;                                                  /* 0x6415A */
    goto L_6403E;
L_64162:
    setD32(IDX0, D32(IDX0) + 1u);                                       /* 0x64169 */
    setD32(IDX1, D32(IDX0));                                            /* 0x6416F */
    setD32(IDX2, D32(IDX0));                                            /* 0x64175 */
    i++;                                                                /* 0x6417E */
L_64181:
    if (i >= S32(SITES_N))                                              /* 0x64184 */
        goto L_64200;
    s = SITE(i);
    if (D8(EXCL + (uint32_t)s) != 0)                                    /* 0x6419C */
        goto L_64162;
    if (SITE_TOK(s) <= ty[0])                                           /* 0x641C2 */
        goto L_64162;
    setD8(C_SITE, (uint8_t)s);                                          /* 0x641D1 */
    setD8(C_TREAS, (uint8_t)(2 - ty[0]));                               /* 0x641DD */
    setD8(C_LEVY, (uint8_t)ty[0]);                                      /* 0x641E5 */
    j = S32(IDX1) + 1;                                                  /* 0x641F0 */
    goto L_640EB;

L_64200:   /* the next stage */
    setD32(IDX0, 0xFFFFFFFFu);                                          /* 0x64215 */
    setD32(IDX1, 0xFFFFFFFFu);                                          /* 0x6421B */
    setD32(IDX2, 0xFFFFFFFFu);                                          /* 0x64221 */
    setD32(X168, 2);                                                    /* 0x64228 */
    setD32(STAGE, D32(STAGE) + 1u);                                     /* 0x6422E */
    a = S32(N_TREAS);                                                   /* 0x64234 */
    b = S32(N_ONE);                                                     /* 0x6423C */
    c = S32(N_LEVY);                                                    /* 0x64244 */
    total = a + b + c;                                                  /* 0x64252 */
    if (S32(STAGE) > 2)                                                 /* 0x64255 cmp ecx, 2; jg 0x643BA */
        return 0;
    if (S32(STAGE) == 1) {                                              /* 0x6425D */
        if (D8(SHORT_FLAG + (uint32_t)n) != 0) {                        /* 0x6426F: excluded sites back in */
            for (i = 0; i < S32(SITES_N); i++) {                        /* 0x642CF */
                s = SITE(i);
                if (D8(EXCL + (uint32_t)s) == 0)                        /* 0x642EE */
                    continue;
                if (S8(AREA(s + 1) + 0x0Eu + (uint32_t)n) <= 1)         /* 0x6430C dword [.. + n + 0xE31C3] sar 24 */
                    continue;
                if (S8(0xEED84u + 160u * (uint32_t)n + (uint32_t)s) < 0)   /* 0x64335 dword [.. + 0xEED81] sar 24 */
                    continue;
                setD8(EXCL + (uint32_t)s, 0);                           /* 0x6434E */
                for (j = 0; j < S32(COPT_CNT); j++)                     /* 0x64284 */
                    if ((int32_t)D8(COPT(j) + 3u) == SITE(i)) {         /* 0x642AA */
                        setD16(COPT(j), 0);                             /* 0x642C1 its score */
                        break;                                          /* 0x642C9: the first match only (D; stage 0
                                                                           made one option per site, so there is
                                                                           only one, G) */
                    }
            }
        }
        port_checkpoint(0x64361u);                                      /* port (M7): the budget's inputs (comparison point) */
        f6332C(total);                                                  /* 0x64361 */
        port_checkpoint(0x64366u);                                      /* port (M7): the budget applied (comparison point) */
    }
    if (S32(STAGE) == 2)                                                /* 0x64366 */
        for (i = S32(STAGE2_FROM); i < S32(STAGE2_TO); i++)             /* 0x6436F, 0x64382 */
            setD8(EXCL + D8(COPT(i) + 3u), 1);                          /* 0x643A2 */
    if (S32(STAGE) + 1 <= total)                                        /* 0x643B1 cmp; jle 0x63D55 */
        goto L_63D55;
    return 0;                                                           /* 0x643BA */
}

#undef SITE_TOK
#undef SITE

/* ---- 0x62840: the construction search ---------------------------------------------------------------------------- */

/* the options by ship count +2 ascending (int8): 0x62FCF..0x63098 */
static void sort_count(void)
{
    int32_t i, j;

    for (i = 1; i < S32(COPT_CNT); i++)                                 /* 0x62FDE */
        for (j = S32(COPT_CNT) - 1; j >= i; j--)                        /* 0x62FEF, 0x63003 jl */
            if (S8(COPT(j - 1) + 2u) > S8(COPT(j) + 2u))                /* 0x6302D cmp dl, cl; jle */
                swap18(COPT(j), COPT(j - 1));                           /* 0x63031..0x63096 */
}

int f62840(int n)
{
    uint32_t nat;                                                       /* [ebp-0x28] */
    int32_t nm1;                                                        /* [ebp-0x50] */
    int32_t pick = -1;                                                  /* [ebp-0x58] */
    int32_t base;                                                       /* [ebp-0x4C] */
    int32_t wait;                                                       /* [ebp-0x40] */
    int32_t lsum;                                                       /* [ebp-0x3C] */
    int32_t flag;                                                       /* [ebp-0x48] */
    uint32_t dt;                                                        /* [ebp-0x2C] */
    int32_t ties;                                                       /* [ebp-0x24] */
    int32_t i, j, k, cur, s;                                            /* [ebp-0x5C], [ebp-0x34] */
    int32_t cnt639, list639[160];                                       /* [ebp-0x38], [ebp-0x2F4] (unused results) */
    uint8_t phase;                                                      /* [ebp-0x1C] */
    int8_t oldships;                                                    /* [ebp-0x18] */
    uint8_t cap;                                                        /* [ebp-0x20] */
    uint8_t used[4];                                                    /* [ebp-0x30] */
    uint8_t bl;
    uint32_t o, rec;

    if (S16(0xBB3F0u) == 0)                                             /* 0x62862 cmp word [0xBB3F0], 0; je 0x6331D */
        return pick;
    nat = NAT(n);                                                       /* 0x6287E */
    nm1 = n - 1;                                                        /* 0x62885 */
    f63950(n);                                                          /* 0x6288B the sites: 0x10F30C / 0x10F310 */
    setD8(0x108190u, 0);                                                /* 0x62892 */
    setD32(SLOTS, (uint32_t)(4 - (int32_t)D8(nat + 0x40u)));            /* 0x628AA */
    setD32(N_TREAS, (uint32_t)f63AF0(n));                               /* 0x628B8 */
    setD32(N_ONE, (uint32_t)f63B7C(n));                                 /* 0x628C5 */
    setD32(N_LEVY, (uint32_t)f63C6C(n));                                /* 0x628DA */
    setD32(COPT_CNT, 0);                                                /* 0x628E6 */
    setD32(CD_SITES, D32(SITES_N));                                     /* 0x628EC */
    setD32(CD_PAIRS, 0);                                                /* 0x628F6 */
    setD32(CD_TRIPLES, 0);                                              /* 0x628FE */
    if (S32(N_LEVY) + (S32(N_TREAS) + S32(N_ONE)) != 0 && S32(SITES_N) != 0) {   /* 0x62904 je; 0x62908 */
        snprintf((char *)DP(MSG), 0x1F4, DSTR(0xB6358u), NAME(n), S32(CD_SITES));   /* 0x6292D "(%d,?,?)" */
        f79270(-550, n, -200, DSTR(MSG), 0);                            /* 0x62949 */
    }
    f7A940();                                                           /* 0x6294E */
    f7A610();                                                           /* 0x62953 the phase's snapshot back */
    f7A940();                                                           /* 0x62958 */
    wait = 0;                                                           /* 0x6295F */
    setD32(T_TOTAL, 0);                                                 /* 0x62962 */
    setD32(T_COUNT, 0);                                                 /* 0x62967 */
    setD32(T_START, wat_clock());                                       /* 0x6296C */
    phase = D8(PHASE);                                                  /* 0x62976 */
    f353C4(n);                                                          /* 0x62981 */
    f461BC(n);                                                          /* 0x62989 the plan without new ships */
    setD8(PHASE, phase);                                                /* 0x62991 */
    base = (int16_t)f7B044(n);                                          /* 0x62999 cwde */
    f7AF28();                                                           /* 0x629A2 */
    f639E8(n, &cnt639, list639);                                        /* 0x629B3 */
    setD32(N_ONE, (uint32_t)f63B7C(n));                                 /* 0x629C0 recounted */
    setD32(N_LEVY, (uint32_t)f63C6C(n));                                /* 0x629CD */
    /* 0x629D2 [ebp-0x54] = 4 + (Clothmaking != 0): never read */
    setD32(STAGE, 0);                                                   /* 0x629F4 */
    setD32(X168, 0xFFFFFFFFu);                                          /* 0x629FA */
    setD32(IDX2, 0xFFFFFFFFu);                                          /* 0x629FF */
    setD32(IDX1, 0xFFFFFFFFu);                                          /* 0x62A04 */
    setD32(IDX0, 0xFFFFFFFFu);                                          /* 0x62A09 */
    if (S32(N_TREAS) + S32(N_ONE) + S32(N_LEVY) != 0 && S32(SITES_N) != 0) {   /* 0x62A1D je; 0x62A23 je 0x632F2 */
        wait = 750;                                                     /* 0x62A30 */
        memset(DP(EXCL), 0, 0xA0);                                      /* 0x62A43 */
        f7A940();                                                       /* 0x62A48 */
        do {
            if (f63D10(n) == 0)                                         /* 0x62A50 */
                break;                                                  /* 0x62A57 je 0x62FCA */
            oldships = (int8_t)D8(nat + 0x40u);                         /* 0x62A63 */
            lsum = 0;                                                   /* 0x62A68 */
            /* the combination applied to the trial state (0x62A76..0x62BC2) */
            for (i = 0; i < S32(C_COUNT); i++) {                        /* 0x62A79 */
                k = D8(nat + 0x40u);                                    /* 0x62A8D [ebp-0x44] */
                s = D8(C_SITE + (uint32_t)i);                           /* 0x62A95 */
                add8(AREA(s + 1) + 0x1Cu + (uint32_t)nm1, 1);           /* 0x62ABF inc byte [.. + 0xE31D4]: +0x1B + n */
                setD16(nat + 0x45u + 2u * (uint32_t)k, (uint16_t)(s + 1));   /* 0x62ACD */
                bl = D8(nat + 0x40u);                                   /* 0x62AD4 */
                setD8(nat + 0x40u, (uint8_t)(bl + 1u));                 /* 0x62ADE */
                setD8(AREA(s + 1) + 0x2Eu + (uint32_t)nm1, bl);         /* 0x62AFE byte [.. + 0xE31E6]: +0x2D + n */
                add8(nat + 0x3Eu, -(int)D8(C_TREAS + (uint32_t)i));     /* 0x62B10 sub byte [+0x3E] */
                add8(AREA(s + 1) + 0x0Fu + (uint32_t)nm1, -(int)D8(C_LEVY + (uint32_t)i));   /* 0x62B37 */
                add8(nat + 0x99u, (int)(uint8_t)(D8(C_TREAS + (uint32_t)i) + D8(C_LEVY + (uint32_t)i)));   /* 0x62B52 */
                add8(nat + 0x98u, -(int)D8(C_LEVY + (uint32_t)i));      /* 0x62B64 */
                o = COPT(S32(COPT_CNT));                                /* 0x62B6A */
                setD8(o + 3u + (uint32_t)i, D8(C_SITE + (uint32_t)i));  /* 0x62B87 */
                setD8(o + 7u + (uint32_t)i, D8(C_TREAS + (uint32_t)i)); /* 0x62B9A */
                setD8(o + 0x0Bu + (uint32_t)i, D8(C_LEVY + (uint32_t)i));   /* 0x62BAD */
                lsum += S8(C_LEVY + (uint32_t)i);                       /* 0x62BBF dword [i + 0x11016D] sar 24 */
            }
            if (S32(CD_SITES) != 0) {                                   /* 0x62BD3 */
                snprintf((char *)DP(MSG), 0x1F4, DSTR(0xB637Eu), NAME(n), S32(CD_SITES));   /* 0x62BE9 "(%d,?,?)" */
                setD32(CD_SITES, D32(CD_SITES) - 1u);                   /* 0x62BF8 */
            } else {
                snprintf((char *)DP(MSG), 0x1F4, DSTR(0xB63A4u), NAME(n),   /* 0x62C1F "(0,%d,%d)" */
                         S32(CD_PAIRS), S32(CD_TRIPLES));
                if (S32(C_COUNT) == 2)                                  /* 0x62C2D */
                    setD32(CD_PAIRS, D32(CD_PAIRS) - 1u);               /* 0x62C32 */
                else if (S32(C_COUNT) == 3)                             /* 0x62C3A */
                    setD32(CD_TRIPLES, D32(CD_TRIPLES) - 1u);           /* 0x62C3F */
            }
            f79270(-560, n, -200, DSTR(MSG), 0);                        /* 0x62C59 */
            setD32(T_ONE, wat_clock());                                 /* 0x62C5E */
            phase = D8(PHASE);                                          /* 0x62C68 */
            f461BC(n);                                                  /* 0x62C73 the trial plan */
            setD8(PHASE, phase);                                        /* 0x62C7B */
            dt = wat_clock() - D32(T_ONE);                              /* 0x62C80..0x62C8D */
            setD32(T_TOTAL, D32(T_TOTAL) + dt);                         /* 0x62C93 */
            setD32(T_SUM, D32(T_SUM) + dt);                             /* 0x62CBC */
            setD8(COPT(S32(COPT_CNT)) + 0x0Fu, 0);                      /* 0x62CC2 */
            setD32(T_COUNT, D32(T_COUNT) + 1u);                         /* 0x62CD6 */
            setD32(T_RUNS, D32(T_RUNS) + 1u);                           /* 0x62CDE */
            memset(used, 0, sizeof used);                               /* 0x62CE4 */
            cap = (uint8_t)(D8(nat + 0x40u) * ((D8(nat + 0x25u) != 0) + 4));   /* 0x62D06 imul; [ebp-0x20] = al */
            for (j = 0; j < S32(SMOVECNT); j++) {                       /* 0x62D19 */
                rec = SMOVES + 6u * (uint32_t)j;
                k = S8(rec + 3u);                                       /* 0x62D37 the ship */
                if (k >= 0 && k < 4)                                    /* port guard (G: k < ships <= 4) */
                    used[k] = 1;                                        /* 0x62D3C */
                if (S8(rec + 4u) == 10)                                 /* 0x62D56 a sail move */
                    cap--;                                              /* 0x62D5E */
            }
            setD8(COPT(S32(COPT_CNT)) + 0x11u, cap);                    /* 0x62D79 */
            flag = 0;                                                   /* 0x62D7F */
            if (lsum > 0) {                                             /* 0x62D82 */
                flag = 1;                                               /* 0x62D8C: an unused levy until a landing */
                for (i = 0; i < S32(C_COUNT); i++) {                    /* 0x62D9E */
                    if (S8(C_LEVY + (uint32_t)i) <= 0)                  /* 0x62DBB */
                        continue;
                    for (j = 0; j < S32(SMOVECNT); j++) {               /* 0x62DCC */
                        rec = SMOVES + 6u * (uint32_t)j;
                        if (S8(rec + 3u) != i)                          /* 0x62DF0 the ship index against i */
                            continue;
                        if (!(S8(rec + 5u) == 4                         /* 0x62E0A */
                              || (S8(rec + 5u) == 6 && (S8(rec + 4u) == 4 || S8(rec + 4u) == 5))))   /* 0x62E25.. */
                            continue;
                        if ((D8(TERR(D8(rec + 1u)) + 2u) & 0x01u) == 0) /* 0x62E8D test dl, 1; jne */
                            flag = 0;                                   /* 0x62E98 */
                    }
                }
                if (S32(C_COUNT) == 1 && flag != 0) {                   /* 0x62EA0, 0x62EA5 */
                    if (oldships >= 0 && oldships < 4)                  /* port guard */
                        used[oldships] = 0;                             /* 0x62EB1 */
                }
            }
            cur = S32(COPT_CNT);
            if (flag == 0)                                              /* 0x62EB5 */
                setD16(COPT(cur), (uint16_t)f7B044(n));                 /* 0x62EBE, 0x62ED4 */
            else
                setD16(COPT(cur), 0xFF9C);                              /* 0x62EED -100 */
            setD8(COPT(S32(COPT_CNT)) + 2u, D8(C_COUNT));               /* 0x62F0C */
            for (i = 0; i < S32(C_COUNT); i++) {                        /* 0x62F20 */
                k = oldships + i;
                if (k < 0 || k > 3 || used[k] == 0) {                   /* 0x62F36 dword [ebp + k - 0x33] sar 24 (port
                                                                           guard: k < 4, G: ships + count <= 4) */
                    setD16(COPT(S32(COPT_CNT)), 0xFF9C);                /* 0x62F5B a new ship without a move */
                } else {
                    if (S16(COPT(S32(COPT_CNT))) > base)                /* 0x62F84 cmp edx, ebx; jg (signed) */
                        continue;
                    setD16(COPT(S32(COPT_CNT)), 0xFF9C);                /* 0x62F93 not better than without */
                }
                if (S32(STAGE) == 0)                                    /* 0x62F63 / 0x62F9A test; jne 0x62FAC */
                    setD8(EXCL + D8(C_SITE), 1);                        /* 0x62FA5 the site is not tried again */
                break;
            }
            f7AF28();                                                   /* 0x62FAC */
            setD32(COPT_CNT, D32(COPT_CNT) + 1u);                       /* 0x62FB8 */
        } while (S32(COPT_CNT) < 0x96);                                 /* 0x62FBE cmp 150; jl 0x62A4D */
        f7AC60();                                                       /* 0x62FCA */
        sort_count();                                                   /* 0x62FCF */
        sort_capacity();                                                /* 0x6309D */
        sort_score();                                                   /* 0x63171 */
        for (i = 0; i < S32(COPT_CNT); i++)                             /* 0x6325A */
            if (S16(COPT(i)) <= base) {                                 /* 0x63280 cmp eax, edx; jg (signed) */
                setD32(COPT_CNT, (uint32_t)i);                          /* 0x63285 */
                break;
            }
        if (S32(COPT_CNT) > 0) {                                        /* 0x6328A cmp; jle 0x632F2 */
            for (ties = 0; ties < S32(COPT_CNT)                         /* 0x6329B */
                           && S16(COPT(0)) == S16(COPT(ties))           /* 0x632B6 */
                           && D8(COPT(0) + 2u) == D8(COPT(ties) + 2u);  /* 0x632D2 */
                 ties++)
                ;
            pick = (int32_t)((uint32_t)(f78AE4() * ties) >> 15);        /* 0x632E3 imul; shr 0xF */
        }
    }
    port_checkpoint(0x632F2u);                                          /* port (M7): the options sorted, the pick drawn */
    f7AD24();                                                           /* 0x632F2 drop the snapshot level */
    f7AC60();                                                           /* 0x632F7 the state at entry */
    busy_wait(D32(T_START) + (uint32_t)(wait / 10));                    /* 0x632FC..0x6331B (not scaled by /C) */
    return pick;                                                        /* 0x6331D */
}

/* ---- 0x66B54: computer m annexes k revolting cities of n --------------------------------------------------------- */

void f66B54(int m, int n, int k)
{
    int32_t t;                                                          /* [ebp-0x14] */
    char buf[100];                                                      /* [ebp-0x80] */

    while (k > 0) {                                                     /* 0x66B70 cmp; jle */
        t = f20C70(m, n) - 1;                                           /* 0x66B80 (CAL1, not ported) */
        f3F670();                                                       /* 0x66B89 */
        f3F624(t, 0x0F);                                                /* 0x66B96 */
        f3DA60(t);                                                      /* 0x66B9E */
        f78A20(500);                                                    /* 0x66BA8 */
        setD8(AREA(t + 1) + 0x3Bu + (uint32_t)n, 1);                    /* 0x66BC0 byte [.. + 0xE31F3] */
        f78F04(t, n);                                                   /* 0x66BCD n's city removed */
        f78E74(t, m);                                                   /* 0x66BD8 m's city built */
        k--;                                                            /* 0x66BE0 */
        snprintf(buf, sizeof buf, DSTR(0xB8A03u), NAME(m), TNAME4(t + 1));   /* 0x66C02 "%s annexes a city in %s." */
        f79270(2000, n, t, buf, 0);                                     /* 0x66C1A */
    }
}

/* ---- 0x66C2C: computer n removes k of its own cities ------------------------------------------------------------- */

void f66C2C(int n, int k)
{
    int32_t j, i, q;                                                    /* [ebp-0x20], [ebp-0x28], [ebp-0x24] */
    int32_t t1;                                                         /* [ebp-0x1C] */
    uint8_t pass, found;                                                /* [ebp-0x10], [ebp-0x14] */
    char buf[100];                                                      /* [ebp-0x90] */

    for (j = 0; j < 2 && k > 0; j++) {                                  /* 0x66C55, 0x66C5F */
        pass = (uint8_t)(j != 0);                                       /* 0x66C6E setne */
        for (i = S8(NAT(n) + 0x51u) - 1; i >= 0 && k > 0; i--) {        /* 0x66C7B dword [+0x4E] sar 24; 0x66D60 */
            found = 0;                                                  /* 0x66D76 */
            for (q = 0; q < 4; q++)                                     /* 0x66C9D */
                if ((int32_t)D8(NAT(n) + 0x52u + (uint32_t)i) - 1       /* 0x66CB9 cmp edx, [16n + 4q + 0xC0618] */
                    == S32(0xC0618u + 16u * (uint32_t)n + 4u * (uint32_t)q))
                    found = 1;                                          /* 0x66CC2 */
            if (found != pass)                                          /* 0x66CCB */
                continue;
            t1 = D8(NAT(n) + 0x52u + (uint32_t)i);                      /* 0x66CD6 */
            f3F670();                                                   /* 0x66CDF */
            f3F624(t1 - 1, 0x0F);                                       /* 0x66CED */
            f3DA60(t1 - 1);                                             /* 0x66CF6 */
            f78A20(500);                                                /* 0x66D00 */
            f78F04(t1 - 1, n);                                          /* 0x66D0C */
            k--;                                                        /* 0x66D14 */
            snprintf(buf, sizeof buf, DSTR(0xB8A1Cu), NAME(n), TNAME4(t1));   /* 0x66D39 "%s removes a city in %s" */
            f79270(2500, n, t1 - 1, buf, 0);                            /* 0x66D55 */
        }
    }
}

/* ---- 0x680DC: a computer short of stock places its tokens -------------------------------------------------------- */

void f680DC(int n, int need)
{
    uint32_t nat;                                                       /* [ebp-0x18] */
    int32_t nm1;                                                        /* [ebp-0x44] */
    uint32_t start;                                                     /* [ebp-0x1C] */
    int32_t wait;                                                       /* [ebp-0x34] */
    int32_t before;                                                     /* [ebp-0x24] */
    int32_t excess;                                                     /* [ebp-0x30] */
    int32_t cut, lim, tok, r, t, i, took;                               /* [ebp-0x2C], [ebp-0x28], [ebp-0x40], [ebp-0x38], [ebp-0x20], [ebp-0x3C] / [ebp-0x48] */
    int32_t alloc[161];                                                 /* [ebp-0x330]; [160] = the word at [ebp-0xB0] */
    char buf[100];                                                      /* [ebp-0xB0] */
    uint32_t m;
    int32_t ccnt, clist[160];

    nm1 = n - 1;                                                        /* 0x680FD */
    nat = NAT(n);                                                       /* 0x6810E */
    f7A8D0();                                                           /* 0x68111 */
    snprintf(buf, sizeof buf, DSTR(0xB8B55u), NAME(n));                 /* 0x6812D "%s cannot fully populate and is
                                                                           considering where to place its tokens." */
    f79270(-550, n, -200, buf, 0);                                      /* 0x6814A */
    start = wat_clock();                                                /* 0x6814F */
    wait = 1000;                                                        /* 0x68157 */
    if (S8(nat + 0x99u) == 0) {                                         /* 0x68161 dword [+0x96] sar 24 */
        setD8(SHORT_FLAG + (uint32_t)n, 1);                             /* 0x68171 */
        goto L_686FA;
    }
    if (100 * S8(nat + 0x99u) / need < 80)                              /* 0x68193 idiv (signed); cmp 0x50; jge */
        setD8(SHORT_FLAG + (uint32_t)n, 1);                             /* 0x6819E */
    memset(alloc, 0, 0x280);                                            /* 0x681B2 */
    /* risk 8: alloc[160] is the first word of the message buffer above it on the stack */
    alloc[160] = (int32_t)((uint32_t)(uint8_t)buf[0] | (uint32_t)(uint8_t)buf[1] << 8
                           | (uint32_t)(uint8_t)buf[2] << 16 | (uint32_t)(uint8_t)buf[3] << 24);
    before = S8(nat + 0x98u);                                           /* 0x681C3 dword [+0x95] sar 24 */
    f7A940();                                                           /* 0x681C8 */
    for (i = 0; S8(nat + 0x60u) > i; i++) {                             /* 0x6820E dword [+0x5D] sar 24 */
        r = D8(nat + 0x61u + (uint32_t)i);                              /* 0x68224 */
        tok = S8(0xE31C7u + 69u * (uint32_t)r + (uint32_t)nm1);         /* 0x6823C dword [.. + 0xE31C4] sar 24 */
        if (r == 0)                                                     /* port guard: alloc[-1] (G: area lists hold
                                                                           territory + 1 >= 1) */
            continue;
        if (tok == 1) {                                                 /* 0x6824B */
            add8(nat + 0x98u, 1);                                       /* 0x68253 */
            add8(0xE31C7u + 69u * (uint32_t)r + (uint32_t)nm1, 1);      /* 0x6826B */
            alloc[r - 1] += tok;                                        /* 0x68274 [ebp + 4r - 0x334] */
        } else if (tok >= 2) {                                          /* 0x681D5 cmp 2; jl */
            add8(0xE31C7u + 69u * (uint32_t)r + (uint32_t)nm1, 2);      /* 0x681EC */
            add8(nat + 0x98u, 2);                                       /* 0x681F6 */
            alloc[r - 1] += 2;                                          /* 0x68200 */
        }
    }
    excess = S8(nat + 0x98u) - before - S8(nat + 0x99u);                /* 0x6829A */
    f461BC(n);                                                          /* 0x682A0 the plan on the full board */
    f5AAA4(0);                                                          /* 0x682A7 its conflicts, silent */
    for (i = 0; S8(nat + 0x60u) > i; i++) {                             /* 0x682B9 */
        r = D8(nat + 0x61u + (uint32_t)i);                              /* 0x682D3 */
        t = r - 1;                                                      /* 0x682DD */
        lim = f78D08(t, n);                                             /* 0x682E6 */
        if ((D8(TS(t) + 2u) & 0x02u) != 0)                              /* 0x68300 test dl, 2 (a planned site) */
            continue;
        tok = S8(0xE31C7u + 69u * (uint32_t)r + (uint32_t)nm1);         /* 0x68317 */
        if (tok <= lim)                                                 /* 0x68325 jle */
            continue;
        cut = tok > lim;                                                /* 0x68347 setg: 1, not tok - lim */
        if (t < 0)                                                      /* port guard (r = 0, see above) */
            continue;
        while (cut > 0 && excess > 0 && alloc[t] > 0) {                 /* 0x68352, 0x6835C, 0x68369 */
            excess--;                                                   /* 0x6837A */
            cut--;                                                      /* 0x68380 */
            alloc[t]--;                                                 /* 0x68386 */
        }
    }
    if (excess > 0)                                                     /* 0x6838F */
        for (i = 0; i < S32(LMOVECNT); i++) {                           /* 0x683A6 */
            m = LMOVES + 4u * (uint32_t)i;
            if (S8(m + 3u) != 7)                                        /* 0x683C2 dword [4i + 0x107694] sar 24 */
                continue;
            if (D8(m) > 0xA0)                                           /* 0x683D6 cmp 0xA0; jg (160 reads alloc[160]) */
                continue;
            cut = S8(m + 2u);                                           /* 0x683E7 dword [4i + 0x107693] sar 24 */
            while (cut > 0 && excess > 0 && alloc[D8(m)] > 0) {         /* 0x683ED, 0x683F3, 0x68408 */
                excess--;
                cut--;
                alloc[D8(m)]--;                                         /* 0x6842D */
            }
        }
    if (excess > 0)                                                     /* 0x68436 */
        for (i = 0; i < S32(LMOVECNT); i++) {                           /* 0x6844D */
            m = LMOVES + 4u * (uint32_t)i;
            if (S8(m + 3u) != 0x11)                                     /* 0x68469 */
                continue;
            if (D8(m) > 0xA0)                                           /* 0x6847D */
                continue;
            cut = S8(m + 2u);                                           /* 0x6848E */
            while (cut > 0 && excess > 0 && alloc[D8(m)] > 0) {         /* 0x68494.. */
                excess--;
                cut--;
                alloc[D8(m)]--;                                         /* 0x684D4 */
            }
        }
    if (excess > 0)                                                     /* 0x684DD */
        for (i = 0; S8(nat + 0x60u) > i; i++) {                         /* 0x684F4 */
            t = (int32_t)D8(nat + 0x61u + (uint32_t)i) - 1;             /* 0x68514 */
            if ((D8(0xF3581u + 314u * (uint32_t)nm1 + 2u * (uint32_t)t) & 0x01u) == 0)   /* 0x68528 bit 8 */
                continue;
            if (t < 0)                                                  /* port guard */
                continue;
            while (excess > 0 && alloc[t] > 0) {                        /* 0x6852D, 0x68536 */
                excess--;
                alloc[t]--;
            }
        }
    if (excess > 0)                                                     /* 0x68552 */
        for (i = 0; S8(nat + 0x60u) > i; i++) {                         /* 0x68569 */
            t = (int32_t)D8(nat + 0x61u + (uint32_t)i) - 1;             /* 0x68589 */
            if ((D8(0xF3580u + 314u * (uint32_t)nm1 + 2u * (uint32_t)t) & 0x80u) == 0)   /* 0x6859D bit 7 */
                continue;
            if (t < 0)                                                  /* port guard */
                continue;
            while (excess > 0 && alloc[t] > 0) {                        /* 0x685A2, 0x685AB */
                excess--;
                alloc[t]--;
            }
        }
    f7AC60();                                                           /* 0x685C7 the board back */
    if (excess > 0) {                                                   /* 0x685CC */
        setD32(CHOICE_CNT, 0);                                          /* 0x685D8 */
        for (i = 0; S8(nat + 0x60u) > i; i++) {                         /* 0x685E9 */
            int32_t c = S32(CHOICE_CNT);                                /* 0x68600 */
            setD32(CHOICE + 4u * (uint32_t)c, (uint32_t)((int32_t)D8(nat + 0x61u + (uint32_t)i) - 1));   /* 0x68614 */
            setD32(CHOICE_CNT, (uint32_t)(c + 1));                      /* 0x6861A */
        }
        /* 0x68632 f48D44(0xF7C9C, 0xF7CA0, n, 0, 1): the global count and list, passed through locals and back */
        ccnt = S32(CHOICE_CNT);
        for (i = 0; i < ccnt && i < 160; i++)
            clist[i] = S32(CHOICE + 4u * (uint32_t)i);
        f48D44(&ccnt, clist, n, 0, 1);
        setD32(CHOICE_CNT, (uint32_t)ccnt);
        for (i = 0; i < ccnt && i < 160; i++)
            setD32(CHOICE + 4u * (uint32_t)i, (uint32_t)clist[i]);
        while (excess > 0) {                                            /* 0x68637 */
            took = 0;
            for (i = 0; i < S32(CHOICE_CNT) && excess > 0; i++) {       /* 0x6864D, 0x68655 */
                t = S32(CHOICE + 4u * (uint32_t)i);                     /* 0x6865E */
                if (t < 0 || t > 160)                                   /* port guard (G: t = territory 0..159) */
                    continue;
                if (alloc[t] > 0) {                                     /* 0x6866C */
                    alloc[t]--;                                         /* 0x6867A */
                    excess--;                                           /* 0x68684 */
                    took = 1;
                }
            }
            if (!took)                                                  /* port guard: BUG(1995) 3 (unreachable) */
                break;
        }
    }
    for (t = 0; t < 0xA0; t++) {                                        /* 0x68696 */
        if (alloc[t] <= 0)                                              /* 0x686A9 */
            continue;
        if (S8(nat + 0x99u) <= 0)                                       /* 0x686B9 the stock, re-read */
            continue;
        f57E28(t, n, alloc[t]);                                         /* 0x686CD */
        add8(nat + 0x98u, (int)(uint8_t)alloc[t]);                      /* 0x686DF add byte [+0x98], dl */
        add8(nat + 0x99u, -(int)(uint8_t)alloc[t]);                     /* 0x686F2 sub byte [+0x99], dl */
    }
L_686FA:
    port_checkpoint(0x686FAu);                                          /* port (M7): the tokens placed (comparison point) */
    f7ABD0();                                                           /* 0x686FA the World State Stack freed */
    busy_wait(start + (uint32_t)(wait / 10));                           /* 0x686FF..0x6871B (not scaled by /C) */
}

/* ---- 0x5BF30: a computer's stock -> treasury transfer after plunder ---------------------------------------------- */

void f5BF30(int n, int amount, int show)
{
    int8_t nb;                                                          /* [ebp-0xC] al */
    int32_t s, v, x;                                                    /* [ebp-0x10] (word), [ebp-0x18] */
    char buf[100];                                                      /* [ebp-0x80] */

    nb = (int8_t)n;                                                     /* 0x5BF42 */
    s = S8(0xF1528u + 199u * (uint32_t)(nb - 1));                       /* 0x5BF56 dword [199(n-1) + 0xF1525] sar 24:
                                                                           record n - 1's +0x5C (file comment) */
    v = S16(0xBCFDAu + 4u * (uint32_t)s + 68u * (uint32_t)(nb - 1));    /* 0x5BF74 word [4s + 68(n-1) + 0xBCFDA] */
    if (v >= 2)                                                         /* 0x5BF83 cmp 2; jge */
        x = amount;                                                     /* 0x5BFB9 */
    else {
        x = D8(NAT(nb) + 0x40u);                                        /* 0x5BF94 the ships */
        if (x > amount)                                                 /* 0x5BFA3 jg */
            x = amount;
        else if (x < 1)                                                 /* 0x5BFAA cmp 1; jge */
            x = 0;                                                      /* 0x5BFB1 */
    }
    add8(NAT(nb) + 0x99u, -(int)(uint8_t)x);                            /* 0x5BFC9 sub byte [+0x99] */
    add8(NAT(nb) + 0x3Eu, (int)(uint8_t)x);                             /* 0x5BFDC add byte [+0x3E] */
    if (show == 0)                                                      /* 0x5BFE2 */
        return;
    f3CD6C(3);                                                          /* 0x5BFF1 */
    setD8(0xEE168u, 3);                                                 /* 0x5C01A (after the call) */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(0xF52CCu), S32(0xF52C8u));   /* 0x5C020 */
    f7876C();                                                           /* 0x5C028 */
    fg_vbopen(S32(0xF52C8u));                                           /* 0x5C034 */
    f3A3A0(0x125, 0x185, DSTR(0xB5D04u));                               /* 0x5C04B "UNITS TO" */
    f3A044(0x15A, 0x185, DSTR(0xB5D0Du));                               /* 0x5C064 "TRANSFER" */
    f3C9D0(0x136, 0x181, 0x13, 0x13, 5);                                /* 0x5C077 */
    fg_setcolor(5);                                                     /* 0x5C07E */
    snprintf(buf, sizeof buf, "%d", (int)x);                            /* 0x5C094 itoa(x, buf, 10) */
    f3A0B0(0x136, 0x185, 0x13, buf);                                    /* 0x5C0A8 */
    fg_vbclose();                                                       /* 0x5C0AD */
    snprintf(buf, sizeof buf, DSTR(0xB5D16u), NAME(nb));                /* 0x5C0C7 "%s, choosing number units to
                                                                           transfer from stock to treasury." */
    f3A9F4(buf);                                                        /* 0x5C0D2 */
    f459E4(nb);                                                         /* 0x5C0DB */
    f3B05C(0xF596Cu);                                                   /* 0x5C0E5 */
    f3F670();                                                           /* 0x5C0EA */
    f3D9FC(nb);                                                         /* 0x5C0F3 */
    f3CBBC();                                                           /* 0x5C0F8 */
    f78A20(2500);                                                       /* 0x5C102 */
}
