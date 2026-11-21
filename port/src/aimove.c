/* Module AIMOVE of docs/M6_PLAN.md, the top level (9,792 bytes): the computer's movement planner 0x461BC (1,840) and
   its passes 0x37CF8 (2,768, attacks on enemy territories), 0x387C8 (1,676, the attack test of one territory), 0x38E54
   (532, expansion into city sites), 0x39068 (1,684, an attack on an enemy city) and 0x396FC (1,292, relief of besieged
   own cities). Literal translations; the address of each instruction a line comes from is in its comment. D = read in
   the code, G = inference. Prototypes and records: aimove.h.

     0x461BC  f461BC  the whole plan of computer n (callers: MOVEMENT 0x55A5F, the ship phases' housekeeping 0x60F6A /
                      0x62109, SHIPAI 0x6132A / 0x61598 / 0x62989 / 0x62C73, POP 0x682A0)
     0x37CF8  f37CF8  attacks on territories held by one other nation (f461BC 0x463F5 x = 1, 0x46686 x = 0)
     0x387C8  f387C8  the attack test of one territory (f37CF8 0x37EFE)
     0x38E54  f38E54  tokens into neighbouring city sites (f461BC 0x46611)
     0x39068  f39068  an attack on an enemy city (f461BC 0x465C7, 0x465FC)
     0x396FC  f396FC  relief of own cities with foreign tokens in them (f461BC 0x463FD)

   Not ported here (each call site stops the port with not_ported(address); the static stubs below carry the names, so
   a later translation that exports one of these names collides with its stub at compile time instead of being shadowed
   by it). All belong to the other modules of M7's batch 1; real signatures (Watcom registers eax, edx, ebx, ecx, then
   the stack; pointers are the caller's stack int32 locals, lea):
     AIQ (aiq.c):     void f7BD5C(int n); void f46998(int n); void f160EC(int n); int f468EC(void);
                      void f48028(int32_t *cnt, int32_t *list, int t, int n, int filter) (ret 4);
                      void f48D44(int32_t *cnt, int32_t *list, int n, int t, int flags) (ret 4);
                      int f4CC5C(int t, int n, int mode); int f39C08(int n); int f49934(int t, int n, int mode);
                      int f4CEE4(int t, int n, int32_t *out); int f4D110(int t, int32_t *cnt, int32_t *list, int m,
                      int k) (ret 4)
     AICITY (aicity.c): int f337C8(int n); int f33964(int goal, int32_t *cnt, int n, int mode) (its eax is read by
                      none of the three call sites here, so the stub is void);
                      int f3580C(int need, int t, int n, int mode, int s) (ret 4; 0 = done)
     AISEA (aisea.c): int f4D29C(int t, int m); void f36918(int n, int t, int32_t *need, int code) (code = cl);
                      void f643C8(int n); void f648C8(int n, int flags); void f65CB0(int n)
     AIPLAN (aiplan.c): void f4A088(int n, int a, int b); void f4C074(int n); void f4C9B8(int n, int x);
                      void f46BB8(int n); void f4C6E4(int n); void f478D8(int c, int32_t *cnt, int n, int32_t *flag);
                      void f47A44(int c, int32_t *cnt, int n, int32_t *flag); void f49E04(int n); void f4BBDC(int n);
                      int f49A1C(int t, int n, int k, int exclude)
   (void = no call site here reads eax; the owning module's translation settles its other callers.) With -DAIMOVE_TEST
   they go to the test's recorders (port/test/aimove_test.c) instead, which return.
   Ported callees: f7C760, f7C7C4, f78D08, f57CA4, f46158 (rules.c); f46A4C, f46B34 (formulas.c); f3F6E4, f3F758
   (kit.c); f3F698 (mapdata.c); f5AAA4 (conflict.c, show 0); f5C238 (city.c, show 0); f7A940, f7AC60 (stack.c); f7A688,
   f7A700, f7A7E8, f7A85C, f7AA1C, f7AD24, f7AD7C, f7AE14, f7AF28, f7AFB8 (mstack.c); f4D518 (movephase.c); f78A20.

   Watcom runtime -> standard C: 0x7D5D0 memset -> memset in g_ds and on the locals; rep movsd record copies -> struct
   assignment; 0x7D4AC (__CHK) dropped. No 0x78B0C / 0x78B24 trace call and no trace string in these six (D).
   0x8A080 clock: f461BC takes start = clock() when [0xF5908] != 0 (0x461DD, [ebp-0x30] = 500) and, after its last
   pop, busy-waits while (unsigned) start + (500 / [0xBB3F4]) / 10 > clock() (0x46899..0x468C2). The port's clock
   (ui_kit2.c) advances only inside f78A20, so the translation calls f78A20(500) there, as pop.c / city.c do (G, port
   design): the same wait when nothing between waits; f78A20 returns at once headless. G: no callee between the two
   (AIQ/AICITY/AISEA/AIPLAN) is known to wait; settle when they land.
   game_rand f78AE4: none drawn here (D). The random choices read the pre-rolled table int16 0x112A94 (made by 0x7BCD4
   each TAXATION, saved) as the original: rows 7, 8, 9, 9 + t % 4 (f37CF8), 11 + k % 9 (f38E54), 24 (f39068).

   Stack discipline (D, kept exactly): f461BC pushes the World State at 0x462FD (popped at 0x4688B, the end: every trial
   move is undone, the plan stays in the move lists), pushes / pops around the reserve measures (0x4630D / 0x46371,
   0x46426 / 0x46471), peeks the first level at the start of every pass of its loop (0x463DF), and pushes both stacks
   at 0x466FA / 0x466FF for the f49E04 variants (snapshots 0x7A688 / 0x7A7E8, peeks, restore 0x7A700 / 0x7A85C when
   the second variant is not better), dropped at 0x4676A / 0x4676F. f37CF8 / f396FC / f39068 push both stacks for each
   attempt: a failure pops both (undone), a success drops both levels (the moves kept, one level less).

   Globals written that the stacks do not restore (D; for the DOSBox-X comparison): the nation stack int32 [0x107D44]
   (+1 at entry, -1 at the end) / int32 0x107D48[depth] = n; int32 [0x108194], [0x107EE0], [0x107EDC], byte [0x107ED8],
   int32 0x108164[n] (zeroed each pass), memset(0xF8558 + 160(n - 1), 0, 160) (the city-site failure cache of
   0x33964), int32 [0x107D40] = 0, byte [0xC28AC] (0 then 1), int32 [0xC28B0] (3 then 4); the token box colours
   0xFFC00 (f3F6E4, then 15 where n has tokens) and ship box colours 0xFFCA0 (f3F758); [0xF5908] (saved, cleared and
   restored around each try of pass C). The area clears of 0x4625D..0x462F1 (+0x3B, int16 +0x37, +0x24 + n for areas
   1..160) happen before the first push, so they stay after the final pop; nation +0x5D is written only inside the
   pushed level and is back to its entry value at the end.

   1995 bugs: none fixed here. Checked (D code, G reach):
     - f461BC's [ebp-0x24] (the most disliked nation) and f39068's [ebp-0x24] are set only when an attitude beats the
       start value (-99999 / -9999); both loops include n itself, whose entry 0xF52E0[n][n] is 0 (0x7C960 writes
       s_n - s_n for every nation in play), so they are always set (G: n is in play). The port starts them at 0.
     - f39068's [ebp-0x48] (b) is read uninitialised only if 0x4CEE4 did not write *out; its record writes it on both
       paths (AIQ). The port starts it at 0.
     - f461BC's start [ebp-0x1C] and [ebp-0x30] = 500 are written only when [0xF5908] != 0 at entry (0x461D4), but the
       wait at 0x46890 tests [0xF5908] again: were it 0 at entry and set at the end, the busy wait would read stack
       garbage. Every write of [0xF5908] inside the planner (0x34BFB / 0x34C1C .. 0x4D2D5 / 0x4D2EB, pass C here) is a
       save, clear and restore pair, and only MOVEMENT (0x5598C / 0x55A6B) sets it around the call, so the flag is the
       same at both tests (G: unreachable). The port's f78A20(500) reads no garbage either way.
     - The land move list 0x107694 has room for 200 records with no bound check (as movephase.c notes); each token
       moves at most once per plan (f57CA4 takes it off the movable count +0x24 + n), so the kept records stay far
       below (G). The record writes keep the original's byte order; a write outside the data object is skipped (port
       guard, unreachable).
   Kept as the original (AI heuristics, not bugs; D code):
     - f37CF8 lets a negative attitude F52E0[n][victim] gate the extra roll but computes its threshold from the
       transposed entry F52E0[victim][n] (0x385B1: [40 victim + 4n + 0xF52E0]); the inventory read it as the same entry.
     - f461BC's "nothing changed" test after f46BB8 compares the territory count +0x60 (0x46616 dword +0x5D sar 24),
       not the reserve +0x5D as the inventory says.
     - f39068 marks the conquered city's area byte +0x3B (0x39685: 69(t + 1) + 0xE31F3, no + n) with n's tokens there,
       the byte f461BC clears per area, while f38E54 clears +0x3B + n.
     - f396FC compares Engineering (+0x2A) with == 1 / == 0, not != 0; o = 0 (only n's own tokens in its city) reads
       nation record 0 and area byte +0x0E (in the data object) and pushes / pops for nothing.
     - f37CF8's 9-byte marking of crowded territories (0x38153..0x38255) is never read.
     - Deterministic reads past the tables (risk 8), kept through the data object: a victim 10 / 11 (f7C760 returns
       1..11) makes f37CF8 read F52E0[victim][n] past the int32 [10][10] matrix (0xF52E0 + 400 + 4n = [0xF5470 + 4n],
       0xF5498 + 4n for 11); f39068 with a city owner 11..13 reads F52E0[n][o] in the next row and 0xBCCE0 past its 9
       rows (85 (o - 1)); a victim 0 (only factions 12 / 13 there) reads nation record 0 and area byte +0x0E.
   Easter eggs: none in these six (no strings). Oddities above. */

#include <stdint.h>
#include <string.h>

#include "aimove.h"
#include "city.h"
#include "conflict.h"
#include "formulas.h"
#include "game.h"
#include "kit.h"
#include "mapdata.h"
#include "movephase.h"
#include "mstack.h"
#include "rules.h"
#include "stack.h"
#include "state.h"

/* ---- calls into code that is not ported yet (see the file comment) ----------------------------------------------- */

#ifdef AIMOVE_TEST
void aimove_test_f7BD5C(int n);
void aimove_test_f46998(int n);
void aimove_test_f160EC(int n);
int  aimove_test_f468EC(void);
void aimove_test_f48028(int32_t *cnt, int32_t *list, int t, int n, int filter);
void aimove_test_f48D44(int32_t *cnt, int32_t *list, int n, int t, int flags);
int  aimove_test_f4CC5C(int t, int n, int mode);
int  aimove_test_f39C08(int n);
int  aimove_test_f49934(int t, int n, int mode);
int  aimove_test_f4CEE4(int t, int n, int32_t *out);
int  aimove_test_f4D110(int t, int32_t *cnt, int32_t *list, int m, int k);
int  aimove_test_f337C8(int n);
void aimove_test_f33964(int goal, int32_t *cnt, int n, int mode);
int  aimove_test_f3580C(int need, int t, int n, int mode, int s);
int  aimove_test_f4D29C(int t, int m);
void aimove_test_f36918(int n, int t, int32_t *need, int code);
void aimove_test_f643C8(int n);
void aimove_test_f648C8(int n, int flags);
void aimove_test_f65CB0(int n);
void aimove_test_f4A088(int n, int a, int b);
void aimove_test_f4C074(int n);
void aimove_test_f4C9B8(int n, int x);
void aimove_test_f46BB8(int n);
void aimove_test_f4C6E4(int n);
void aimove_test_f478D8(int c, int32_t *cnt, int n, int32_t *flag);
void aimove_test_f47A44(int c, int32_t *cnt, int n, int32_t *flag);
void aimove_test_f49E04(int n);
void aimove_test_f4BBDC(int n);
int  aimove_test_f49A1C(int t, int n, int k, int exclude);
#define f7BD5C(n)                aimove_test_f7BD5C(n)
#define f46998(n)                aimove_test_f46998(n)
#define f160EC(n)                aimove_test_f160EC(n)
#define f468EC()                 aimove_test_f468EC()
#define f48028(c, l, t, n, f)    aimove_test_f48028(c, l, t, n, f)
#define f48D44(c, l, n, t, f)    aimove_test_f48D44(c, l, n, t, f)
#define f4CC5C(t, n, m)          aimove_test_f4CC5C(t, n, m)
#define f39C08(n)                aimove_test_f39C08(n)
#define f49934(t, n, m)          aimove_test_f49934(t, n, m)
#define f4CEE4(t, n, o)          aimove_test_f4CEE4(t, n, o)
#define f4D110(t, c, l, m, k)    aimove_test_f4D110(t, c, l, m, k)
#define f337C8(n)                aimove_test_f337C8(n)
#define f33964(g, c, n, m)       aimove_test_f33964(g, c, n, m)
#define f3580C(k, t, n, m, s)    aimove_test_f3580C(k, t, n, m, s)
#define f4D29C(t, m)             aimove_test_f4D29C(t, m)
#define f36918(n, t, k, c)       aimove_test_f36918(n, t, k, c)
#define f643C8(n)                aimove_test_f643C8(n)
#define f648C8(n, f)             aimove_test_f648C8(n, f)
#define f65CB0(n)                aimove_test_f65CB0(n)
#define f4A088(n, a, b)          aimove_test_f4A088(n, a, b)
#define f4C074(n)                aimove_test_f4C074(n)
#define f4C9B8(n, x)             aimove_test_f4C9B8(n, x)
#define f46BB8(n)                aimove_test_f46BB8(n)
#define f4C6E4(n)                aimove_test_f4C6E4(n)
#define f478D8(c, k, n, f)       aimove_test_f478D8(c, k, n, f)
#define f47A44(c, k, n, f)       aimove_test_f47A44(c, k, n, f)
#define f49E04(n)                aimove_test_f49E04(n)
#define f4BBDC(n)                aimove_test_f4BBDC(n)
#define f49A1C(t, n, k, e)       aimove_test_f49A1C(t, n, k, e)
#else
/* M7 batch 1 linked (Claude, 21 Nov): the other planner modules' real routines (they were not_ported stubs
   while the five modules were translated in parallel) */
#include "aiq.h"
#include "aicity.h"
#include "aisea.h"
#include "aiplan.h"
#endif

/* ---- data (D, the addresses as the code reads them) -------------------------------------------------------------- */

#define FLAGS        0xE1624u   /* word: bits 10..11 the AI level */
#define SHOWN        0xF5908u   /* int32: the computer's moves are shown */
#define INPLAY_T     0xEE16Cu   /* uint8[160] territory in play */
#define LANDADJ      0xF8E7Cu   /* uint32[160]: handles to int16 count, int16 territories (game.h GAME_LANDADJ_PTR) */
#define ORDER        0xF06C0u   /* int8[] turn order position of nation n */
#define ERA          0xBCCE0u   /* int8 [9][17][5]: byte 0 of record 5 * AST square of row n - 1 */
#define ERAW         0xC2864u   /* int32 [6][3]: weight per era and AI level */
#define AITAB        0x112A94u  /* int16 [30][10] the pre-rolled AI table */
#define L1           0x107694u  /* land move list, 4-byte records */
#define L1N          0x1079B4u  /* int32 its count */
#define L2N          0x107D3Cu  /* int32 the ship move list's count */
#define NSTACK_D     0x107D44u  /* int32 depth of the nation stack */
#define NSTACK       0x107D48u  /* int32[] */
#define TOKENS_SUP   0xF4898u   /* int32 the token supply */
#define SEA_ON       0xBB3F0u   /* int16: the sea moves are on */

static uint32_t nat(int32_t n)  { return 0xF14CCu + 199u * (uint32_t)n; }
static uint32_t area(int32_t r) { return 0xE31B8u + 69u * (uint32_t)r; }    /* area record r = territory r - 1 */
static uint32_t tst(int32_t t)  { return 0xE2C18u + 3u * (uint32_t)t; }
static uint32_t terr(int32_t t) { return 0xE1638u + 35u * (uint32_t)t; }

static int32_t att(int32_t n, int32_t m)   /* dword [40n + 4m + 0xF52E0] */
{
    return S32(0xF52E0u + 40u * (uint32_t)n + 4u * (uint32_t)m);
}

static int32_t aitab(int32_t row, int32_t n)   /* dword [2(n - 1) + 20 row + 0x112A92] sar 16 */
{
    return S16(AITAB + 20u * (uint32_t)row + 2u * (uint32_t)(n - 1));
}

static int32_t ai_level(void)   /* mov ax, word [0xE1624]; shl eax, 4; shr ax, 0xE */
{
    return (int32_t)((((uint32_t)D16(FLAGS) << 4) & 0xFFFFu) >> 14);
}

static int32_t mul32(int32_t a, int32_t b)   /* imul r32, r32: the low 32 bits */
{
    return (int32_t)((uint32_t)a * (uint32_t)b);
}

static int32_t hword(uint32_t h)   /* movsx word [handle] (port guard: an unresolved handle reads 0, as mono.c) */
{
    const uint8_t *p = load_heap_ptr(h, 2);
    return p != NULL ? (int32_t)(int16_t)(p[0] | p[1] << 8) : 0;
}

static void put8(uint32_t a, int32_t v)   /* mov byte [..]: port guard, a store outside the data object is skipped */
{
    if (ds_valid(a, 1))
        setD8(a, (uint8_t)v);
}

/* One land move record {from, to, k, type} at [0x1079B4], in the original's store order: from, to, type, then either
   the count byte before the new count (k_first) or the new count before the count byte (G: only an overfull list,
   where the record overlaps the count itself, could tell them apart). */
static void ml_add(int32_t from, int32_t to, int32_t k, int32_t type, int k_first)
{
    uint32_t i = D32(L1N);

    put8(L1 + 4u * i, from);
    put8(L1 + 4u * i + 1u, to);
    put8(L1 + 4u * i + 3u, type);
    if (k_first) {
        put8(L1 + 4u * i + 2u, k);
        setD32(L1N, i + 1u);
    } else {
        setD32(L1N, i + 1u);
        put8(L1 + 4u * i + 2u, k);
    }
}

/* ---- the gathering shared by 0x37CF8 and 0x396FC ----------------------------------------------------------------- */

/* The tokens of n gathered into t from the source list (identical code in both: 0x38691..0x384E4 in f37CF8, codes
   0x0C / 0x0D / 0x0E / 0x0F; 0x397FA..0x39B53 in f396FC, codes 0x24 / 0x25 / 0x26 / 0x27). need is the caller's stack
   int32 (also passed to f36918). Pass C's record stores the count byte before the new count in f37CF8 (0x38469)
   and after it in f396FC (0x39ADF): c_kfirst. */
static void gather(int n, int32_t t, int32_t *need, int32_t lcnt, const int32_t *list, int32_t ca, int32_t cb,
                   int32_t cc, int c_kfirst, int32_t cship)
{
    int32_t j, s, r, lim, sweeps, saved;

    /* pass A: the tokens above the population limit, one at a time */
    for (j = 0; j < lcnt && *need > 0; j++) {                       /* 0x38696 / 0x397FF */
        s = list[j];                                                /* 0x386AF / 0x39818 */
        r = s + 1;
        lim = f78D08(s, n);                                         /* 0x386C6 / 0x3982F */
        while (*need > 0                                            /* 0x386CE / 0x39837 */
               && S8(area(r) + 0x24u + (uint32_t)n) > 0             /* 0x386EA dword [69r + n - 1 + 0xE31DA] sar 24 */
               && S8(area(r) + 0x0Eu + (uint32_t)n) > lim) {        /* 0x3870D dword [.. + 0xE31C4] sar 24 */
            f57CA4(s, t, n, 1, 1);                                  /* 0x3872F / 0x39890 */
            ml_add(s, t, 1, ca, 0);                                 /* 0x38734..0x38761 / 0x39895..0x398C2 */
            (*need)--;                                              /* 0x3876B / 0x398CC */
        }
    }
    /* pass B: up to 10 sweeps, one token from each source with more than one */
    if (*need > 0 && lcnt != 0) {                                   /* 0x3826F / 0x398DF */
        j = 0;
        sweeps = 10;                                                /* 0x38288 / 0x398F8 */
        while (*need > 0 && sweeps > 0) {                           /* 0x3828F / 0x398FF */
            s = list[j];
            r = s + 1;
            if (S8(area(r) + 0x24u + (uint32_t)n) > 0               /* 0x382C9 / 0x39939 */
                && S8(area(r) + 0x0Eu + (uint32_t)n) > 1) {         /* 0x382E8 / 0x39958 */
                f57CA4(s, t, n, 1, 1);                              /* 0x38306 / 0x39976 */
                ml_add(s, t, 1, cb, 0);                             /* 0x3830B..0x38337 / 0x3997B..0x399A8 */
                (*need)--;
            }
            j++;                                                    /* 0x38347 / 0x399B8 */
            if (j == lcnt) {
                j = 0;
                sweeps--;                                           /* 0x3835E / 0x399CF */
            }
        }
    }
    /* pass C: up to 10 sweeps with the shown flag cleared: a token only where f49A1C says the source can spare it, or
       for a target that is no city site whose other nation lacks Architecture */
    if (*need > 0 && lcnt != 0) {                                   /* 0x38366 / 0x399D7 */
        j = 0;
        sweeps = 10;
        while (*need > 0 && sweeps > 0) {                           /* 0x38386 / 0x399F7 */
            s = list[j];
            r = s + 1;
            saved = S32(SHOWN);                                     /* 0x383AE / 0x39A1F */
            setD32(SHOWN, 0);                                       /* 0x383B8 / 0x39A29 */
            if (S8(area(r) + 0x24u + (uint32_t)n) > 0               /* 0x383D0 / 0x39A41 */
                && (f49A1C(s, n, 1, t) != 0                         /* 0x383EF / 0x39A60 */
                    || ((D8(terr(t) + 1u) & 0x40) == 0              /* 0x38409 / 0x39A7A */
                        && D8(nat(f7C7C4(t, n)) + 0x31u) == 0))) {  /* 0x38418, 0x38423 / 0x39A89, 0x39A94 */
                f57CA4(s, t, n, 1, 1);                              /* 0x3843C / 0x39AAD */
                ml_add(s, t, 1, cc, c_kfirst);                      /* 0x38441..0x38470 / 0x39AB2..0x39ADF */
                (*need)--;
            }
            setD32(SHOWN, (uint32_t)saved);                         /* 0x3847E / 0x39AEF */
            j++;
            if (j == lcnt) {
                j = 0;
                sweeps--;
            }
        }
    }
    /* by ship, to a coastal target, when the sea moves are on */
    if (*need > 0                                                   /* 0x384A7 / 0x39B16 */
        && (D8(terr(t)) & 0x02) != 0                                /* 0x384BE / 0x39B2D */
        && D16(SEA_ON) != 0)                                        /* 0x384C7 / 0x39B36 cmp word */
        f36918(n, t, need, cship);                                  /* 0x384DF / 0x39B4E */
}

/* ---- 0x387C8: the attack test of one territory ------------------------------------------------------------------- */

int f387C8(int t, int n, int mode, int32_t *need, int32_t *victim, int x)
{
    int32_t r = t + 1;                                              /* 0x387ED [ebp-0x30] */
    int32_t metn = S8(nat(n) + 0x26u);                              /* 0x38804 [ebp-0x24] */
    int32_t o, meto, bonus, e, s, v;
    uint32_t tok;                                                   /* the victim's tokens: 69r + o - 1 + 0xE31C4, sar 24 */
    int32_t lcnt;                                                   /* [ebp-0x14] */
    int32_t list[160];                                              /* [ebp-0x2C0] */

    if (S8(area(r) + 1u) != 1)                                      /* 0x3881E */
        return 0;
    if ((D8(tst(t) + 2u) & 0x02) != 0                               /* 0x38833 */
        && S8(area(r) + 1u + (uint32_t)n) == n)                     /* 0x3885A */
        return 0;
    o = f7C760(t);                                                  /* 0x38868 [ebp-0x2C] */
    meto = S8(nat(o) + 0x26u);                                      /* 0x38884 [ebp-0x1C] */
    *victim = o;                                                    /* 0x38892 */
    bonus = 0;                                                      /* 0x38894 [ebp-0x18] */
    if (metn != 0 && meto == 0)                                     /* 0x38897, 0x3889D */
        bonus = 1;
    if (S8(nat(o) + 0x51u) >= 9)                                    /* 0x388BA */
        return 0;
    tok = area(r) + 0x0Fu + (uint32_t)(o - 1);

    if ((D8(terr(t) + 1u) & 0x40) != 0 && x == 0                    /* 0x388DA, 0x388E3 a city site, the first pass */
        && (D8(tst(t) + 2u) & 0x01) == 0) {                         /* 0x388F9 no city */
        if (D8(nat(o) + 0x31u) == 0) {                              /* 0x38927 the victim lacks Architecture */
            if (!(S8(tok) >= 6                                      /* 0x38938 */
                  && S8(ORDER + (uint32_t)n) > S8(ORDER + (uint32_t)o))) {   /* 0x38949 cmp byte; jg */
                if (S8(tok) < 4)                                    /* 0x38970 */
                    return 0;
                if (att(n, o) <= 300)                               /* 0x3898B */
                    return 0;
                e = S8(ERA + 85u * (uint32_t)(n - 1) + 5u * (uint32_t)S8(nat(n) + 0x5Cu));   /* 0x389C8 */
                if (e != 3) {                                       /* 0x389D2 */
                    e = S8(ERA + 85u * (uint32_t)(n - 1) + 5u * (uint32_t)S8(nat(n) + 0x5Cu));   /* 0x38A03 */
                    if (e != 4)                                     /* 0x38A0D */
                        return 0;
                }
                if (S8(tok) < f78D08(t, o))                         /* 0x38A37, 0x38A3C */
                    return 0;
                if (S8(ORDER + (uint32_t)n) >= S8(ORDER + (uint32_t)o))   /* 0x38A50 jge */
                    return 0;
                if (f337C8(o) <= 0)                                 /* 0x38A5F, 0x38A66 */
                    return 0;
            }
            /* 0x38A6C */
            if (S8(tok) > mode + 6 + bonus)                         /* 0x38A90 */
                return 0;
            if (S8(tok) >= 4                                        /* 0x38AB3 */
                && S8(tok) >= f78D08(t, o)                          /* 0x38ADD, 0x38AE2 */
                && S8(ORDER + (uint32_t)n) < S8(ORDER + (uint32_t)o)) {   /* 0x38AF6 jge */
                f48028(&lcnt, list, t, o, 0x0C);                    /* 0x38B13 */
                s = f4D110(t, &lcnt, list, o, 1);                   /* 0x38B29 [ebp-0x10] */
                v = s + S8(tok);                                    /* 0x38B51 [ebp-0xC] */
                if (v > 9)                                          /* 0x38B57 */
                    v = 9;
                *need = (v - 6 > 0 ? v - 6 : 0) + 2 - bonus;        /* 0x38B63..0x38B82 */
                return 1;
            }
            *need = S8(tok) - 4 - bonus;                            /* 0x38BAD..0x38BB8 */
            return 1;
        }
        /* 0x38BC8: the victim has Architecture */
        if (S8(tok) <= f78D08(t, o))                                /* 0x38BD7, 0x38BDC */
            return 0;
        if (S8(ORDER + (uint32_t)n) <= S8(ORDER + (uint32_t)o))     /* 0x38BF0 jle */
            return 0;
        if (S8(tok) < 3)                                            /* 0x38C17 */
            return 0;
        if (S8(tok) > mode + 3 + bonus)                             /* 0x38C44 */
            return 0;
        if (S8(nat(o) + 0x3Eu) + S8(tok) < 6)                       /* 0x38C79 treasury + tokens */
            return 0;
        *need = S8(tok) - 1 - bonus;                                /* 0x38C9D..0x38CA6 */
        return 1;
    }

    /* 0x38CB6: no city site, a city, or the second pass */
    if (D8(nat(o) + 0x31u) == 0) {                                  /* 0x38CD7 */
        if (S8(tok) < 12)                                           /* 0x38CE8 */
            return 0;
        if (S8(ORDER + (uint32_t)n) <= S8(ORDER + (uint32_t)o))     /* 0x38CFD jle */
            return 0;
        if (S8(tok) > mode + 12 + bonus)                            /* 0x38D2D */
            return 0;
        *need = S8(tok) - 10 - bonus;                               /* 0x38D50..0x38D5B */
        return 1;
    }
    /* 0x38D6B */
    if (x != 0)                                                     /* 0x38D6E */
        return 0;
    if (S8(tok) < 6)                                                /* 0x38D7F */
        return 0;
    if (S8(nat(o) + 0x51u) >= 9)                                    /* 0x38D98 */
        return 0;
    if (S8(ORDER + (uint32_t)n) <= S8(ORDER + (uint32_t)o))         /* 0x38DAD jle */
        return 0;
    if (S8(tok) > mode + 6 + bonus)                                 /* 0x38DDD */
        return 0;
    if (S8(tok) + S8(nat(o) + 0x3Eu) < 12)                          /* 0x38E0E */
        return 0;
    *need = S8(tok) - 4 - bonus;                                    /* 0x38E2E..0x38E39 */
    return 1;
}

/* ---- 0x37CF8: attacks on territories held by one other nation ---------------------------------------------------- */

typedef struct {   /* 12 bytes, [ebp-0x1224] / [ebp-0xAA4] */
    int32_t t;        /* +0 */
    int32_t need;     /* +4 */
    int32_t victim;   /* +8 */
} attack_t;

void f37CF8(int n, int x)
{
    uint32_t rec = nat(n);                                          /* 0x37D2A [ebp-0x30] */
    int32_t mode = 1;                                               /* 0x37D2D [ebp-0x78] */
    int32_t L, cnt, cnt2, t, i, j, k, u, r, found, done, sum;
    int32_t need = 0, victim = 0;                                   /* [ebp-0x80], [ebp-0x74] */
    int32_t lcnt;                                                   /* [ebp-0x14] */
    int32_t list[160];                                              /* [ebp-0x324] */
    attack_t c1[160], c2[160], tmp;                                 /* [ebp-0x1224], [ebp-0xAA4], [ebp-0xA4] */
    uint8_t mark[9], cur[9];                                        /* [ebp-0x8C], [ebp-0x98] */
    uint32_t h;

    L = ai_level();                                                 /* 0x37D4F..0x37D63 cx */
    if (aitab(7, n) / 300 < 80 + 10 * (2 - L)) {                    /* 0x37D37..0x37D78 */
        mode = 2;                                                   /* 0x37D7A */
        if (aitab(8, n) / 300 < 3 * L)                              /* 0x37D84..0x37DA7 */
            mode = 3;                                               /* 0x37DA9 */
    }

    cnt = 0;                                                        /* 0x37DB2 [ebp-0x20] */
    for (t = 0; t < 0x9D; t++) {                                    /* 0x37DB5 [ebp-0x70], 0x37ED4 */
        if (D8(INPLAY_T + (uint32_t)t) == 0)                        /* 0x37EE0 */
            continue;
        if (f387C8(t, n, mode, &need, &victim, x) == 0)             /* 0x37EFE */
            continue;
        found = 1;                                                  /* 0x37F0C [ebp-0x50] */
        h = D32(LANDADJ + 4u * (uint32_t)t);                        /* 0x37F12 */
        for (j = 1; hword(h) >= j; j++) {                           /* 0x37F1C, 0x37DC3 movsx; cmp; jl */
            u = hword(h + 2u * (uint32_t)j);                        /* 0x37DDA */
            r = u + 1;                                              /* 0x37DDD */
            if (S8(area(r) + 0x0Eu + (uint32_t)n) > 0) {            /* 0x37DF3 n has tokens in a land neighbour */
                c1[cnt].need = need;                                /* 0x37E16 */
                c1[cnt].victim = victim;                            /* 0x37E33 */
                c1[cnt].t = t;                                      /* 0x37E53 */
                cnt++;
                found = 0;                                          /* 0x37E5C */
                break;
            }
        }
        if (found != 0 && f4D29C(t, n) != 0) {                      /* 0x37E5F, 0x37E6B reachable by sea */
            c1[cnt].need = need;                                    /* 0x37E8A */
            c1[cnt].victim = victim;                                /* 0x37EA7 */
            c1[cnt].t = t;                                          /* 0x37EC7 */
            cnt++;
        }
    }
    if (cnt > 0 && aitab(9, n) / 300 < 2 * L + 1)                   /* 0x37F24..0x37F5C */
        return;

    /* 0x37F62: bubble sort by n's attitude towards the victim, descending */
    for (i = cnt - 2; i >= 0; i--)                                  /* 0x37F68, 0x37F7F */
        for (j = 0; j <= i; j++)                                    /* 0x37F77, 0x37F96 */
            if (att(n, c1[j + 1].victim) > att(n, c1[j].victim)) {  /* 0x37FF1..0x37FFF */
                tmp = c1[j + 1];                                    /* 0x3801B */
                c1[j + 1] = c1[j];                                  /* 0x38048 */
                c1[j] = tmp;                                        /* 0x38064 */
            }
    /* 0x3806C: territories that are no city site first, then the sites */
    cnt2 = 0;                                                       /* 0x3806E [ebp-0x1C] */
    for (i = 0; i < cnt; i++)                                       /* 0x38071, 0x3807C */
        if ((D8(terr(c1[i].t) + 1u) & 0x40) == 0)                   /* 0x380A5 */
            c2[cnt2++] = c1[i];                                     /* 0x380DC */
    for (i = 0; i < cnt; i++)                                       /* 0x380E3, 0x380EE */
        if ((D8(terr(c1[i].t) + 1u) & 0x40) != 0)                   /* 0x38117 */
            c2[cnt2++] = c1[i];                                     /* 0x3814E */

    /* 0x38153: a marking of n's crowded territories (nations' tokens over the population limit) that nothing reads */
    memset(mark, 0, 9);                                             /* 0x38160 */
    for (i = 0; i < S8(rec + 0x60u); i++) {                         /* 0x38167, 0x38172 dword +0x5D sar 24 */
        r = D8(rec + 0x61u + (uint32_t)i);                          /* 0x3818C */
        if (S8(area(r) + 1u) <= 1)                                  /* 0x3819C */
            continue;
        sum = 0;                                                    /* 0x381AD */
        memset(cur, 0, 9);                                          /* 0x381BD */
        for (k = 0; k < 9; k++) {                                   /* 0x381C2, 0x381CD */
            sum += S8(area(r) + 0x0Fu + (uint32_t)k);               /* 0x381E5 */
            cur[k] = 1;                                             /* 0x381F4 */
        }
        if ((int32_t)((uint8_t)(D8(terr(r - 1) + 1u) << 2) >> 5) < sum)   /* 0x38210..0x38221 */
            for (k = 0; k < 9; k++)                                 /* 0x3822C */
                mark[k] = cur[k];                                   /* 0x3824E */
    }
    (void)mark;

    /* 0x38257: the attempts, until one succeeds */
    done = 0;                                                       /* 0x38259 [ebp-0x4C] */
    for (i = 0; i < cnt2 && done == 0; i++) {                       /* 0x3825C [ebp-0x6C], 0x384FE, 0x3850A */
        if (att(n, c2[i].victim) < 0) {                             /* 0x38536 */
            int32_t d = ((S8(nat(c2[i].victim) + 0x9Au) == 3) + 1) * (5 - L);   /* 0x3855C..0x3858C */
            int32_t q = (att(c2[i].victim, n) - 200) / d;           /* 0x385B1..0x385C3 [40 victim + 4n + 0xF52E0] */
            if (aitab(9 + c2[i].t % 4, n) / 300 <= q)               /* 0x385DF..0x38617 */
                continue;
        }
        /* 0x3861D: [ebp-0x40] = 1, always taken */
        f7A940();                                                   /* 0x38630 */
        f7AA1C();                                                   /* 0x38635 */
        t = c2[i].t;                                                /* 0x3864D [ebp-0x70] */
        need = c2[i].need;                                          /* 0x38663 [ebp-0x5C] */
        f48028(&lcnt, list, t, n, 9);                               /* 0x38677 */
        f48D44(&lcnt, list, n, t, 1);                               /* 0x3868C */
        gather(n, t, &need, lcnt, list, 0x0C, 0x0D, 0x0E, 1, 0x0F); /* 0x38691..0x384DF */
        if (need != 0) {                                            /* 0x384E4 */
            f7AC60();                                               /* 0x384EE */
            f7AE14();                                               /* 0x384F3 */
        } else {
            /* 0x38773: the marker record {0xC9, the nation there, 0, -} (byte +3 not written) */
            uint32_t m = D32(L1N);                                  /* 0x38773 */
            int32_t o;
            put8(L1 + 4u * m, 0xC9);                                /* 0x38778 -0x37 */
            put8(L1 + 4u * m + 2u, 0);                              /* 0x38782 */
            o = f7C7C4(t, n);                                       /* 0x3878F */
            m = D32(L1N) + 1u;                                      /* 0x38796, 0x3879B */
            put8(L1 + 4u * m - 3u, o);                              /* 0x387A1 [4 eax + 0x107691] */
            setD32(L1N, m);                                         /* 0x387A8 */
            done = 1;                                               /* 0x387AD */
            f7AD24();                                               /* 0x387B0 */
            f7AD7C();                                               /* 0x387B5 */
        }
    }
}

/* ---- 0x38E54: tokens into neighbouring city sites ---------------------------------------------------------------- */

void f38E54(int n)
{
    uint32_t rec = nat(n);                                          /* 0x38E7B [ebp-0x10] */
    int32_t ctr = 0;                                                /* 0x38E87 [ebp-0x1C] */
    int32_t i, j, r, s, u, k, row;
    int32_t lcnt;                                                   /* [ebp-0x14] */
    int32_t list[160];                                              /* [ebp-0x2B8] */

    if (S8(rec + 0x98u) < 14)                                       /* 0x38E91..0x38E9D dword [199n + 0xF1561] sar 24 */
        return;
    if (f39C08(n) >= 9)                                             /* 0x38EA6, 0x38EAB */
        return;
    for (i = 0; i < S8(rec + 0x60u); i++) {                         /* 0x38EB4 [ebp-0x34], 0x38EBF (live count) */
        r = D8(rec + 0x61u + (uint32_t)i);                          /* 0x38ED9 [ebp-0x28] */
        s = r - 1;                                                  /* 0x38EE3 [ebp-0x2C] */
        if ((D8(tst(s) + 2u) & 0x02) != 0)                          /* 0x38EF2 */
            continue;
        f48028(&lcnt, list, s, n, 0x14);                            /* 0x38F0E */
        f48D44(&lcnt, list, n, s, 0x201);                           /* 0x38F27 */
        for (j = 0; j < lcnt; j++) {                                /* 0x38F2E [ebp-0x38], 0x38F39 */
            u = list[j];                                            /* 0x38F48 [ebp-0x24] */
            k = f4CC5C(u, n, 3);                                    /* 0x38F5D [ebp-0x20] */
            if (k <= 0)                                             /* 0x38F68 */
                continue;
            if (S8(area(r) + 0x24u + (uint32_t)n) < k)              /* 0x38F7E..0x38F8A movable at s */
                continue;
            row = ctr % 9 + 11;                                     /* 0x38F8C..0x38F9E (the old value) */
            ctr++;                                                  /* 0x38F8F */
            if (aitab(row, n) / 300 >= 95 - 15 * ai_level())        /* 0x38FB0..0x38FEA */
                continue;
            f57CA4(s, u, n, 1, k);                                  /* 0x39002 */
            put8(area(u + 1) + 0x3Bu + (uint32_t)n, 0);             /* 0x3901C [69(u + 1) + n - 1 + 0xE31F4] */
            ml_add(s, u, k, 0x19, 0);                               /* 0x39022..0x39053 */
        }
    }
}

/* ---- 0x39068: an attack on an enemy city ------------------------------------------------------------------------- */

typedef struct {   /* 16 bytes, [ebp-0xA70] */
    int32_t t;        /* +0 */
    int32_t value;    /* +4: attitude * era weight / 100 */
    int32_t a;        /* +8: tokens needed (0x4CEE4), 99999 = not asked yet */
    int32_t b;        /* +12: its *out */
} cityatt_t;

void f39068(int n, int x)
{
    uint32_t rec = nat(n);                                          /* 0x39093 [ebp-0x14] */
    int32_t L = ai_level();
    int32_t score, ex, tr, max, who = 0, maxast, thr, cnt, t, o, m, i, j, era, a, done;
    int32_t b = 0;                                                  /* [ebp-0x48] */
    cityatt_t c[160], tmp;                                          /* [ebp-0xA70], [ebp-0x70] */

    if (S8(rec + 0x51u) >= 9)                                       /* 0x390A0 */
        return;
    score = 0;                                                      /* 0x390B1 [ebp-0x40] */
    if (D8(rec + 0x2Au) != 0)                                       /* 0x390B7 Engineering */
        score += 10;
    score += 15 - 5 * L;                                            /* 0x390C2..0x390E4 */
    ex = S8(rec + 0x98u) - 6 * x - S32(TOKENS_SUP) / 2;             /* 0x390E7..0x39119 [ebp-0x34] */
    if (ex > 0)                                                     /* 0x3911C */
        score += 5 * ex;
    tr = S8(rec + 0x3Eu);                                           /* 0x39131 dword +0x3B sar 24 */
    if (tr >= 15 && tr < 18)                                        /* 0x3913A, 0x3913F */
        score += 20;
    max = -9999;                                                    /* 0x39148 [ebp-0x30] */
    for (m = 1; m <= 9; m++)                                        /* 0x3914F [ebp-0x1C] */
        if (D8(nat(m) + 0x9Au) != 0 && att(n, m) > max) {          /* 0x3916B, 0x39191 */
            max = att(n, m);                                        /* 0x391AE */
            who = m;                                                /* 0x391B4 [ebp-0x24] */
        }
    if (max > 0)                                                    /* 0x391B9 */
        score += 10 * (max / 50 + 1);                               /* 0x391BF..0x391DD */
    maxast = 0;                                                     /* 0x391E2 [ebp-0x2C] */
    for (m = 1; m <= 9; m++)                                        /* 0x391E5 */
        if (D8(nat(m) + 0x9Au) != 0 && S8(nat(m) + 0x5Cu) > maxast) /* 0x39201, 0x39218 dword +0x59 sar 24 */
            maxast = S8(nat(m) + 0x5Cu);
    if (maxast > 12)                                                /* 0x39237 */
        maxast = 12;
    thr = mul32(score, maxast) / 13;                                /* 0x39244..0x39257 [ebp-0x3C] */
    if (aitab(24, n) / 300 >= thr)                                  /* 0x3925A..0x39276 */
        return;

    cnt = 0;                                                        /* 0x3927E [ebp-0x20] */
    for (t = 0; t < 0x9D; t++) {                                    /* 0x39281 [ebp-0x1C], 0x392B1 */
        if ((D8(tst(t) + 2u) & 0x01) == 0)                          /* 0x392CA a city */
            continue;
        o = D8(tst(t));                                             /* 0x392E5 [ebp-0x28] */
        if (S8(area(t + 1) + 1u) != 0) {                            /* 0x392F9 */
            if (S8(area(t + 1) + 1u) != 1)                          /* 0x3931C */
                continue;
            if (S8(area(t + 1) + 1u + (uint32_t)o) != o)            /* 0x3933E only the owner there */
                continue;
        }
        if (o == n)                                                 /* 0x3935E */
            continue;
        if (att(n, o) < -200)                                       /* 0x39379 */
            continue;
        if (o == 10)                                                /* 0x393A1 */
            continue;
        if (f49934(t, n, 0) == 0)                                   /* 0x393B2 */
            continue;
        c[cnt].t = t;                                               /* 0x393C8 */
        c[cnt].value = 0;                                           /* 0x393D7 */
        era = S8(ERA + 85u * (uint32_t)(o - 1) + 5u * (uint32_t)S8(nat(o) + 0x5Cu));   /* 0x3940B */
        c[cnt].value = mul32(att(n, o), S32(ERAW + 4u * (uint32_t)L + 12u * (uint32_t)era)) / 100;   /* 0x39450..0x39472 */
        if (o != who) {                                             /* 0x3947F */
            c[cnt].a = 99999;                                       /* 0x39291 */
            c[cnt].b = 99999;                                       /* 0x3929E */
        } else {
            c[cnt].a = f4CEE4(t, n, &b);                            /* 0x39490, 0x3949B */
            c[cnt].b = b;                                           /* 0x394AB */
        }
        cnt++;                                                      /* 0x392A8 */
    }
    /* 0x394B7: by a ascending (the bubble from the end) */
    for (i = 1; i < cnt; i++)                                       /* 0x394B7, 0x394C6 */
        for (j = cnt - 1; j >= i; j--)                              /* 0x394D2, 0x394E1 */
            if (c[j - 1].a > c[j].a) {                              /* 0x394F5..0x39505 */
                tmp = c[j - 1];                                     /* 0x39517 */
                c[j - 1] = c[j];                                    /* 0x39535 */
                c[j] = tmp;                                         /* 0x39549 */
            }
    /* 0x3954F: then by value descending */
    for (i = cnt - 2; i >= 0; i--)                                  /* 0x39552, 0x3956C */
        for (j = 0; j <= i; j++)                                    /* 0x39564, 0x39586 */
            if (c[j + 1].value > c[j].value) {                      /* 0x3959A..0x395AA */
                tmp = c[j];                                         /* 0x395BC */
                c[j] = c[j + 1];                                    /* 0x395DA */
                c[j + 1] = tmp;                                     /* 0x395EE */
            }

    done = 0;                                                       /* 0x395F6 [ebp-0x4C] */
    for (i = 0; i < cnt && done == 0; i++) {                        /* 0x395F9 [ebp-0x1C], 0x39698, 0x396A0 */
        t = c[i].t;                                                 /* 0x396B3 [ebp-0x54] */
        if (c[i].a == 99999) {                                      /* 0x396C3 */
            a = f4CEE4(t, n, &b);                                   /* 0x396D8 [ebp-0x50] */
        } else {
            a = c[i].a;                                             /* 0x3960E */
            b = c[i].b;                                             /* 0x3961E */
        }
        if (a == -1)                                                /* 0x39621 */
            continue;
        f7A940();                                                   /* 0x39627 */
        f7AA1C();                                                   /* 0x3962C */
        if (f3580C(a, t, n, 2, b) != 0) {                           /* 0x39643, 0x39648: 0 = done */
            f7AC60();                                               /* 0x396E5 */
            f7AE14();                                               /* 0x396EA */
        } else {
            f7AD24();                                               /* 0x39650 */
            f7AD7C();                                               /* 0x39655 */
            put8(area(t + 1) + 0x3Bu, D8(area(t + 1) + 0x0Eu + (uint32_t)n));   /* 0x3967F, 0x39685 (+0x3B, no + n) */
            done = 1;                                               /* 0x3968B */
        }
    }
}

/* ---- 0x396FC: relief of own cities with foreign tokens in them --------------------------------------------------- */

void f396FC(int n)
{
    uint32_t rec = nat(n);                                          /* 0x39724 [ebp-0x14] */
    int32_t k, r, t, o, base, bonus, need;
    int32_t need2;                                                  /* [ebp-0x34] */
    int32_t lcnt;                                                   /* [ebp-0x18] */
    int32_t list[160];                                              /* [ebp-0x2DC] */

    for (k = 0; k < S8(rec + 0x51u); k++) {                         /* 0x3972E [ebp-0x44], 0x39B6D */
        r = D8(rec + 0x52u + (uint32_t)k);                          /* 0x39B87 [ebp-0x54] */
        t = r - 1;                                                  /* 0x39B90 [ebp-0x58] */
        if (S8(area(r) + 1u) == 0)                                  /* 0x39B9E */
            continue;
        o = f7C7C4(t, n);                                           /* 0x39BB2 [ebp-0x50] */
        if (S8(rec + 0x2Au) == 1 && S8(nat(o) + 0x2Au) == 0)        /* 0x39BBD, 0x39BD3 */
            base = 8;                                               /* 0x39BE4 [ebp-0x40] */
        else if (S8(rec + 0x2Au) == 0 && S8(nat(o) + 0x2Au) == 1)   /* 0x39738, 0x39745 */
            base = 6;                                               /* 0x3975A */
        else
            base = 7;                                               /* 0x39763 */
        bonus = 0;                                                  /* 0x3976C [ebp-0x3C] */
        if (D8(rec + 0x26u) != 0 && D8(nat(o) + 0x26u) == 0)        /* 0x39772, 0x3977F */
            bonus = 1;
        need = S8(area(r) + 0x0Eu + (uint32_t)o) - base + 2 - bonus;   /* 0x397A1..0x397B5 [ebp-0x30] */
        f7A940();                                                   /* 0x397B8 */
        f7AA1C();                                                   /* 0x397BD */
        need2 = need;                                               /* 0x397CB */
        f48028(&lcnt, list, t, n, 9);                               /* 0x397DF */
        f48D44(&lcnt, list, n, t, 1);                               /* 0x397F5 */
        gather(n, t, &need2, lcnt, list, 0x24, 0x25, 0x26, 0, 0x27);   /* 0x397FA..0x39B4E */
        if (need2 != 0) {                                           /* 0x39B53 */
            f7AC60();                                               /* 0x39B5D */
            f7AE14();                                               /* 0x39B62 */
        } else {
            f7AD24();                                               /* 0x39BF0 */
            f7AD7C();                                               /* 0x39BF5 */
        }
    }
}

/* ---- 0x461BC: the whole movement plan of computer n -------------------------------------------------------------- */

void f461BC(int n)
{
    uint32_t rec = nat(n);                                          /* 0x46213 [ebp-0x18] */
    int32_t nm1 = n - 1;                                            /* 0x4621A [ebp-0x44] */
    int32_t r, m, i, k, old, v;
    int32_t reserve;                                                /* [ebp-0x48] */
    int32_t c;                                                      /* [ebp-0x58] the cities to aim for */
    int32_t half;                                                   /* [ebp-0x4C] */
    int32_t again;                                                  /* [ebp-0x54] */
    int32_t cnt;                                                    /* [ebp-0x40] the cities planned */
    int32_t flag;                                                   /* [ebp-0x50] */
    int32_t max, anyneg, who = 0, era;                              /* [ebp-0x28], [ebp-0x2C], [ebp-0x24], [ebp-0x20] */
    uint32_t d;

    /* 0x461D4: if [0xF5908], start = clock() ([ebp-0x1C]) and [ebp-0x30] = 500: see the wait at the end */
    d = D32(NSTACK_D);                                              /* 0x461EC */
    setD32(NSTACK + 4u * d, (uint32_t)n);                           /* 0x461F8 */
    setD32(NSTACK_D, d + 1u);                                       /* 0x461FF */
    f7BD5C(n);                                                      /* 0x46220 */
    f46998(n);                                                      /* 0x46228 */
    f160EC(n);                                                      /* 0x46230 */
    f3F6E4();                                                       /* 0x46235 */
    f3F758();                                                       /* 0x4623A */
    f46158(n);                                                      /* 0x46247 */
    for (r = 1; r <= 0xA0; r++) {                                   /* 0x4624C [ebp-0x5C], 0x4625D */
        setD8(area(r) + 0x3Bu, 0);                                  /* 0x46276 [69r + 0xE31F3] */
        setD16(area(r) + 0x37u, 0);                                 /* 0x46289 word [69r + 0xE31EF] */
        if (D8(area(r) + 1u + (uint32_t)n) == 0) {                  /* 0x462B9 [69r + n - 1 + 0xE31BA] */
            setD8(area(r) + 0x24u + (uint32_t)n, 0);                /* 0x46251 [.. + 0xE31DD] */
        } else {
            setD8(area(r) + 0x24u + (uint32_t)n, D8(area(r) + 0x0Eu + (uint32_t)n));   /* 0x462D7, 0x462DD */
            f3F698(r - 1, 15);                                      /* 0x462EC */
        }
    }
    (void)nm1;

    reserve = 99;                                                   /* 0x462F6 */
    f7A940();                                                       /* 0x462FD the level popped at the end */
    c = f337C8(n);                                                  /* 0x46305 */
    f7A940();                                                       /* 0x4630D */
    f5AAA4(0);                                                      /* 0x46314 */
    f5C238(0);                                                      /* 0x4631B */
    half = (S8(rec + 0x98u) - 1) / 2;                               /* 0x46320..0x46336 */
    if (S8(rec + 0x51u) + c >= half)                                /* 0x46339..0x4634A */
        reserve = 0;                                                /* 0x4634E */
    else
        reserve = S8(rec + 0x98u) - 2 - 2 * (S8(rec + 0x51u) + c);  /* 0x46353..0x4636E */
    f7AC60();                                                       /* 0x46371 */

    again = 1;                                                      /* 0x46376 */
    while (again != 0) {                                            /* 0x4637D */
        setD32(L1N, 0);                                             /* 0x46389 */
        setD32(L2N, 0);                                             /* 0x4638F */
        cnt = 0;                                                    /* 0x46395 */
        setD32(0x108194u, 0);                                       /* 0x46398 */
        setD32(0x107EE0u, 0);                                       /* 0x4639E */
        setD32(0x107EDCu, 0);                                       /* 0x463A6 */
        setD8(0x107ED8u, 0);                                        /* 0x463AC */
        flag = 0;                                                   /* 0x463B2 */
        setD32(0x108164u + 4u * (uint32_t)n, 0);                    /* 0x463B8 */
        memset(DP(0xF8558u + 160u * (uint32_t)(n - 1)), 0, 0xA0);   /* 0x463DA */
        f7AF28();                                                   /* 0x463DF peek the first level */
        setD8(rec + 0x5Du, (uint8_t)reserve);                       /* 0x463EA */
        f37CF8(n, 1);                                               /* 0x463F5 */
        f396FC(n);                                                  /* 0x463FD */
        if (S8(rec + 0x5Du) == 0) {                                 /* 0x46405..0x4640D dword +0x5A sar 24 */
            f4A088(n, 0, 1);                                        /* 0x46419 */
            f4C074(n);                                              /* 0x46421 */
            f7A940();                                               /* 0x46426 */
            f5AAA4(0);                                              /* 0x4642D */
            f5C238(0);                                              /* 0x46434 */
            reserve = S8(rec + 0x98u) - 2 - 2 * (S8(rec + 0x51u) + c);   /* 0x46439..0x4645D */
            if (reserve < 0)                                        /* 0x46463 */
                reserve = 0;
            f7AC60();                                               /* 0x46471 (edx = 1 kept across it) */
            flag = 1;                                               /* 0x46476 */
        }
        setD8(rec + 0x5Du, (uint8_t)reserve);                       /* 0x46484 */

        /* the most disliked nation and whether any is disliked at all */
        max = -99999;                                               /* 0x46487 0xFFFE7961 */
        anyneg = 0;                                                 /* 0x4648C */
        for (m = 1; m <= 9; m++) {                                  /* 0x4648F, 0x4649E */
            if (att(n, m) > max) {                                  /* 0x464C0 */
                max = att(n, m);                                    /* 0x464DD */
                who = m;                                            /* 0x464E3 */
            }
            if (att(n, m) < 0)                                      /* 0x464F8 */
                anyneg = 1;                                         /* 0x46502 */
        }
        era = S8(ERA + 85u * (uint32_t)(n - 1) + 5u * (uint32_t)S8(nat(who) + 0x5Cu));   /* 0x4650B..0x46542 */
        if (S8(nat(who) + 0x9Au) == 3) {                            /* 0x4654C..0x46558 a human */
            max += 200;                                             /* 0x4655A */
            era++;                                                  /* 0x46564 */
        }
        if ((anyneg != 0                                            /* 0x46567 */
             && mul32(max, S32(ERAW + 4u * (uint32_t)ai_level() + 12u * (uint32_t)era)) / 100 > 500)   /* 0x4656D..0x465AE */
            || f468EC() != 0) {                                     /* 0x465B0 */
            setD32(0x107D40u, 0);                                   /* 0x465BB */
            f39068(n, cnt);                                         /* 0x465C7 */
            f33964(c, &cnt, n, 0);                                  /* 0x465D7 */
        } else {
            f33964(c, &cnt, n, 0);                                  /* 0x465EB */
            setD32(0x107D40u, 0);                                   /* 0x465F0 */
            f39068(n, cnt);                                         /* 0x465FC */
        }
        f4C9B8(n, 1);                                               /* 0x46609 */
        f38E54(n);                                                  /* 0x46611 */
        old = S8(rec + 0x60u);                                      /* 0x46616..0x4661F dword +0x5D sar 24 */
        f46BB8(n);                                                  /* 0x46625 */
        if (S8(rec + 0x60u) == old                                  /* 0x46633 */
            && D8(rec + 0x40u) == 0                                 /* 0x46643 no ships */
            && S8(rec + 0x51u) == 0                                 /* 0x46650 no cities */
            && S8(rec + 0x98u) >= 12)                               /* 0x4665E */
            f33964(c + 1, &cnt, n, 0);                              /* 0x4666F */
        f4C9B8(n, 2);                                               /* 0x4667C */
        f37CF8(n, 0);                                               /* 0x46686 */
        f4C6E4(n);                                                  /* 0x46690 */
        setD8(0x107ED8u, 0);                                        /* 0x46695 dh */
        f478D8(c, &cnt, n, &flag);                                  /* 0x466A7 */
        if (cnt < 4)                                                /* 0x466AC */
            f47A44(c, &cnt, n, &flag);                              /* 0x466BE */
        if (f46B34(n) == 0) {                                       /* 0x466C6, 0x466CB */
            if (ai_level() != 2)                                    /* 0x466CF..0x466E4 */
                f643C8(n);                                          /* 0x466E9 */
            else
                f648C8(n, 0);                                       /* 0x466F5 */
        }

        /* 0x466FA: two variants of the spreading, the second kept only if it leaves fewer tokens over the limits */
        f7A940();                                                   /* 0x466FA */
        f7AA1C();                                                   /* 0x466FF */
        f4A088(n, 0, 0);                                            /* 0x4670B */
        f49E04(n);                                                  /* 0x46713 */
        v = f46A4C(n);                                              /* 0x4671B [ebp-0x38] */
        if (v > 0) {                                                /* 0x46723 */
            f7A688();                                               /* 0x46729 */
            f7A7E8();                                               /* 0x4672E */
            f7AF28();                                               /* 0x46733 */
            f7AFB8();                                               /* 0x46738 */
            f49E04(n);                                              /* 0x46740 */
            f4A088(n, 0, 0);                                        /* 0x4674C */
            if (f46A4C(n) >= v) {                                   /* 0x46754, 0x4675C */
                f7A700();                                           /* 0x46760 */
                f7A85C();                                           /* 0x46765 */
            }
        }
        f7AD24();                                                   /* 0x4676A */
        f7AD7C();                                                   /* 0x4676F */

        f4C074(n);                                                  /* 0x46779 */
        setD8(0xC28ACu, 0);                                         /* 0x4677E */
        f46BB8(n);                                                  /* 0x46787 */
        setD8(0xC28ACu, 1);                                         /* 0x46793 */
        setD32(0xC28B0u, 3);                                        /* 0x46799 */
        f4A088(n, 0, 1);                                            /* 0x467A9 */
        setD32(0xC28B0u, 4);                                        /* 0x467AE */
        f648C8(n, 0);                                               /* 0x467BD */
        f4BBDC(n);                                                  /* 0x467C5 */
        if (S32(SHOWN) != 0)                                        /* 0x467CA */
            f65CB0(n);                                              /* 0x467D6 */

        if (cnt > 0) {                                              /* 0x467DB */
            /* 0x467E5: own territories with the AI's site mark */
            k = 0;                                                  /* [ebp-0x3C] */
            for (i = 0; i < S8(rec + 0x60u); i++)                   /* 0x467EA, 0x467F5 dword +0x5D sar 24 */
                if ((D8(tst((int32_t)D8(rec + 0x61u + (uint32_t)i) - 1) + 2u) & 0x02) != 0)   /* 0x46809..0x4681D */
                    k++;
            f5AAA4(0);                                              /* 0x46830 */
            f5C238(0);                                              /* 0x46837 */
            if (S8(nat(n) + 0x98u) - 2 < 2 * S8(nat(n) + 0x51u) + 2 * k)   /* 0x4683C..0x4686A */
                c--;                                                /* 0x4686F: plan one city fewer, again */
            else
                again = 0;                                          /* 0x46879 */
        } else {
            again = 0;                                              /* 0x46883 */
        }
    }
    f7AC60();                                                       /* 0x4688B */
    if (S32(SHOWN) != 0)                                            /* 0x46890 */
        f78A20(500);                                                /* 0x46899..0x468C2 the busy wait (file comment) */
    f4D518(n, S32(SHOWN) != 0);                                     /* 0x468C4..0x468D6 */
    setD32(NSTACK_D, D32(NSTACK_D) - 1u);                           /* 0x468DB */
}
