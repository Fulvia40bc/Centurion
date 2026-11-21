/* Module AISEA of docs/M6_PLAN.md (16,908 bytes): the computer players' ship transport and voyages, called by the
   movement planner 0x461BC and its helpers (M7, batch 1). Literal translations; the address of each instruction a line
   comes from is in its comment. D = read in the code, G = inference. Prototypes and results: aisea.h.

     0x36918  f36918  tokens to a land target by ship (trial per ship, then the replay)       3,016 bytes
     0x374E0  f374E0  the tokens one route collects (trial and undo)                          2,072
     0x4AC50  f4AC50  ship tokens away from t to free land (two legs)                         3,980
     0x4D29C  f4D29C  how many tokens m's ships could bring to t                                636
     0x643C8  f643C8  gather tokens at marked coastal city sites, then the voyages            1,280
     0x648C8  f648C8  the voyages of the ships with full movement points                       752
     0x64BB8  f64BB8  a ship's destination                                                   1,112
     0x65010  f65010  score and choose a route                                               3,132
     0x65CB0  f65CB0  idle ships toward the largest stacks                                     928
   The qsort comparators 0x3530C, 0x35368, 0x65C4C are helpers2.c's; Watcom's qsort 0x8C331 is wqsort.c's wc_qsort.

   Not ported here (each call site stops the port with not_ported(address); the static stubs below carry the names, so
   a later translation that exports one of these names collides with its stub at compile time instead of being shadowed
   by it). Real signatures (Watcom registers eax, edx, ebx, ecx, then the stack; pointers are this module's int32 stack
   locals, lea):
     AIQ (aiq.c):     int f495C0(int t, int n, int mode, int32_t *enemy); int f49790(int r, int t, int n, int enemy);
                      void f48028(int32_t *cnt, int32_t *list, int t, int n, int filter) (ret 4);
                      void f48D44(int32_t *cnt, int32_t *list, int n, int t, int flags) (ret 4);
                      int f7B044(int n); int f7C838(int t, int n, int k, int32_t *left)
     AIPLAN (aiplan.c): int f49A1C(int t, int n, int k, int exclude); void f46BB8(int n)
   (void = no call site here reads eax.) With -DAISEA_TEST they go to the test's recorders (port/test/aisea_test.c),
   and so do the two ported routines f65CB0 runs in their silent mode, f5AAA4(0) (conflict.c) and f5C238(0) (city.c),
   so the unit test links no screen code.
   Ported callees: f5843C, f58888, f5861C (route.c); f57CA4, f460C0, f78D08 (rules.c); f57CE8, f57E28 (move.c); f7A940,
   f7AC60 (stack.c); f7AA1C, f7AD24, f7AD7C, f7AE14, f7AF28, f7AFB8 (mstack.c); wc_qsort (wqsort.c) with f3530C,
   f35368, f65C4C (helpers2.c).

   Watcom runtime -> standard C: 0x7D5D0 memset -> memset in g_ds and on the locals; rep movs record copies -> memcpy;
   0x8C331 qsort -> wc_qsort on DP(0x108820) (the original's alignment: 0x108820 % 4 == 0, size 16); 0x7D4AC (__CHK)
   dropped. No game_rand f78AE4 call, no clock() 0x8A080, no 0x78B0C / 0x78B24 trace call and no string in these nine
   (D: the call scan). The random choices read the pre-rolled table int16 0x112A94 [30][10] (0x7BCD4, saved) as the
   original: f36918 the word 0x112B0A + 2n, f65010 0x112AA6 + 2n, f64BB8 0x112A94 + 20 * (12 + i % 8) + 2n.

   Data (D, offsets as the code adds them; more in move.h, rules.h, route.h):
     area record r = 0xE31B8 + 69r (territory r - 1): +1 nations there, +1+n marker, +0x0E+n tokens, +0x24+n movable,
       int16 +0x39 the tokens gathered for / still wanted at a marked city site (f643C8, f65010)
     territory state 0xE2C18 + 3t: +0 owner of the site, +2 bit 0 city, bit 1 a site the computer marked (G)
     territory record 0xE1638 + 35t: +0 bit 1 (f643C8: coastal, G), +1 bits 3..5 population limit, +2 bit 0
       (f64BB8), bit 1 (f65010: not a landing), bit 2 (f65010: keep 1)
     0xF95FC land matrix uint8[157][157] (game.h); 0xC803C uint8[160] the alias of each territory (157..159 -> 57,
       13, 135); 0xEED84 int8 [10][160] the computers' site scores; 0xF488C int8[10]; 0xF4234 uint8 [9][160];
       0xF3580 int16 [9][157] (bit 0x80 of the low byte); 0xEE16C uint8[160] in play
     caches (not in the save, G: process state, risk 9): 0xF8418 int8[160] the tokens to keep (f495C0 + f49790),
       0xF84B8 int8[160] the pickups of a stop, 0xF8194 int32[160] the slot of a saved area record or -1, int32
       [0xF8414] = 1 once a pickup was tried, int32 [0x10818C] the largest route count seen (f65010; printed by /C)
     the World State Stack's top area block [0xF5240 + 4 * [0xCAC54]] (record r at 69(r - 1)): f643C8 reads its
       +0x39 words, f65010 decrements them (a port handle, read and written through load_heap_ptr)

   Stack discipline (D, kept exactly):
     f36918: World push; per ship that can reach t: Move List push ... Move List pop; World pop; then the replay.
     f4D29C: World push, Move List push, ..., World pop, Move List pop.
     f643C8: World + Move List push; none gathered: World pop, Move List pop, f648C8(n, 0). Else World + Move List push,
       f648C8(n, 0x10); nothing left wanted: World drop, Move List drop, twice. Else World pop, Move List pop, World peek,
       Move List peek (the state before the gathering), the gathering redone with the reduced needs, f648C8(n, 0x10),
       World drop, Move List drop.
     f65010: World push, Move List push, ..., World pop, Move List pop (the stacked +0x39 decrements come back with
       the pop: f643C8 reads them).
     f65CB0: World push ... World pop (the move list keeps the voyages).
     f374E0 and f4AC50 use no stack: f374E0 undoes by its own copies, f4AC50 changes the state for good.

   Globals written that the stacks do not restore (D; for the DOSBox-X comparison): the route table 0x108820 and
   [0x109E00] (f58888, and +7, +8..+13, +0x0E here), the reach table 0x107EE4 (f5861C, and the single entries set
   here), the caches 0xF8418 / 0xF84B8 / 0xF8194, [0xF8414], [0x10818C], the move lists 0x1079B8 / [0x107D3C] and
   0x107694 / [0x1079B4] outside the Move List Stack levels, [0xF5908] (saved and restored by f4D29C), the stacks' high
   water marks, and in f65CB0 whatever f5AAA4(0) / f5C238(0) change outside the world state (conflict.h, city.h).

   1995 bugs and port guards (each marked in the code; "--original" = g_original; D code / G effect):
     1. f64BB8 (risk 8, out-of-range read): the land matrix is indexed 157t + u with the ship's own territory t, which
        is 157..159 for a ship on a second coast: rows 157..159 lie past the 157x157 matrix, in the mapdataz pointers
        0xFF648..0xFF81E (D). The original reads bytes of DOS heap addresses there; the port keeps handles in those
        fields. Fixed by default: the row of the alias 0xC803C[t]. --original: the original's bytes, rebuilt from the
        handles as DOS address 0x378F40 + offset in the block (D: 0x378F40 in [0xFF648] of every DOSBox-X dump of the R4
        and 3-player runs; G: the new-game dumps N1 / N2 / NEWGAME hold 0x377F3C, so the base depends on the start-up
        path; the offsets equal the port's, checked on M7_R4MV_0.BIN). Reachable (G): a computer ship with full points
        on 157..159.
     2. (none: f4AC50's test "tokens == [ebp-0x2c]" at 0x4B567 is reached only with more than one nation at the stop,
        and the loading pass calls f7C838 for such a stop in every mode (0x4B7C8 tests no mode, unlike the scoring
        pass's 0x4B09C), so [ebp-0x2c] always holds this stop's result: no garbage read. Reviewer, D.)
     3. f4AC50 (risk 7): the hop after stop 5 reads the byte after a 6-byte leg, stack padding ([ebp-0x8A] after leg 1,
        [ebp-0x82] after leg 2). Reachable (G: a ship with 5 points, Clothmaking, on a leg of 4 intermediates). Both
        modes read 0xFF (the leg ends).
     4. f36918 (risk 7, unreachable, G): the replay reads stops[m + 1] at m = 5 and the scans for t read past stops[5]
        when t is not among the stops; the route ends with t (f58888 a -> b), so neither happens. The 28-byte plan
        records are zeroed here and the scan stops at the end of the local (port guard).
     5. f36918 (port guard): the scan for routes tied with route 0 on +6 has no bound; f58888 leaves 0xFF after the
        last record, so it stops at the count (G). Kept, bounded by the data object.
     6. f65010 (hang, unreachable G): the unload sweep repeats while tokens are aboard; every landing takes up to its
        limit and the load is at most the sum of the limits, so it ends. Fixed by default (a sweep that unloads nothing
        ends it); --original loops.
     7. f4D29C / f65CB0 (port guards, unreachable D: the nation record has 4 ship slots): best[] has 256 entries, and a
        ship territory outside 0..159 is not marked in f65CB0's 160-entry table.
     8. f65010 (crash, unreachable G): with no route the tie count is 0 and 30000 / 0 traps (0x65BCB idiv). f648C8
        calls f65010 only after f64BB8 chose the target from the same routes (f58888 with the same arguments), so the
        count is at least 1. Both modes return -1 there (a C division by 0 has no defined result).
     Kept literally (they change only the computer's choices, or nothing): f36918's code byte 0xC1938[code] for the
     codes 0x0F / 0x27 that f37CF8 / f396FC pass (risk 8: 0xC1947 / 0xC195F, both 0 in the data object; 0xC1938 lies in
     the data object, obj2.bin offset 0x11938, so D8 reads exactly what the original reads, run-time writes included;
     f3A748 writes only the byte 0xC1944); f64BB8's pickup bytes stored by territory and read by alias; f65010's
     missing reset of 0xF8194 (a fresh process has 0 there, D: the dumps, so f65010 then backs nothing up and the trial
     loads of one route stay for the next until the World pop) and its decrements of the stacked copy; the dead
     f7C838 branch of f4AC50's leg-1 score (mode 1 takes the other branch; the loading pass's f7C838 call is live in
     every mode); the unwritten +0 / +2 bytes of some move
     records (stale bytes stay). */

#include <stdio.h>
#include <string.h>

#include "aisea.h"
#include "city.h"
#include "conflict.h"
#include "game.h"
#include "helpers2.h"
#include "move.h"
#include "mstack.h"
#include "route.h"
#include "rules.h"
#include "stack.h"
#include "state.h"
#include "ui.h"
#include "wqsort.h"

/* ---- calls into code that is not ported yet (see the file comment) ----------------------------------------------- */

#ifdef AISEA_TEST
int  aisea_test_f495C0(int t, int n, int mode, int32_t *enemy);
int  aisea_test_f49790(int r, int t, int n, int enemy);
void aisea_test_f48028(int32_t *cnt, int32_t *list, int t, int n, int filter);
void aisea_test_f48D44(int32_t *cnt, int32_t *list, int n, int t, int flags);
int  aisea_test_f7B044(int n);
int  aisea_test_f7C838(int t, int n, int k, int32_t *left);
int  aisea_test_f49A1C(int t, int n, int k, int exclude);
void aisea_test_f46BB8(int n);
void aisea_test_f5AAA4(int show);
void aisea_test_f5C238(int show);
#define f495C0(t, n, m, e)       aisea_test_f495C0(t, n, m, e)
#define f49790(r, t, n, e)       aisea_test_f49790(r, t, n, e)
#define f48028(c, l, t, n, f)    aisea_test_f48028(c, l, t, n, f)
#define f48D44(c, l, n, t, f)    aisea_test_f48D44(c, l, n, t, f)
#define f7B044(n)                aisea_test_f7B044(n)
#define f7C838(t, n, k, l)       aisea_test_f7C838(t, n, k, l)
#define f49A1C(t, n, k, e)       aisea_test_f49A1C(t, n, k, e)
#define f46BB8(n)                aisea_test_f46BB8(n)
#define f5AAA4(s)                aisea_test_f5AAA4(s)
#define f5C238(s)                aisea_test_f5C238(s)
#else
/* M7 batch 1 linked (Claude, 21 Nov): the other planner modules' real routines (they were not_ported stubs
   while the five modules were translated in parallel) */
#include "aiq.h"
#include "aicity.h"
#include "aimove.h"
#include "aiplan.h"
#endif

/* ---- data ----------------------------------------------------------------------------------------------------- */

#define NAT(n)        (0xF14CCu + 199u * (uint32_t)(n))    /* nation record */
#define AREA(r)       (0xE31B8u + 69u * (uint32_t)(r))     /* area record r (territory r - 1) */
#define TST(t)        (0xE2C18u + 3u * (uint32_t)(t))      /* territory state */
#define TERR(t)       (0xE1638u + 35u * (uint32_t)(t))     /* territory record */
#define ROUTE(r)      (ROUTE_BUF + 16u * (uint32_t)(r))    /* route record r (route.h) */
#define ALIAS(u)      ((int32_t)D8(0xC803Cu + (uint32_t)(u)))
#define LAND_MATRIX   0xF95FCu                              /* uint8 [157][157] */
#define SCORES        0xEED84u                              /* int8 [10][160] */
#define F3580         0xF3580u                              /* int16 [9][157] */
#define KEEP          0xF8418u                              /* int8 [160] */
#define PICK          0xF84B8u                              /* int8 [160] */
#define SLOT          0xF8194u                              /* int32 [160] */
#define PICKED        0xF8414u                              /* int32 */
#define ML1           0x107694u                             /* land moves, 4 bytes */
#define ML1_COUNT     0x1079B4u
#define ML2           0x1079B8u                             /* ship moves, 6 bytes */
#define ML2_COUNT     0x107D3Cu
#define WS_AREA       0xF5240u                              /* the World State Stack's area blocks [10] */
#define WS_DEPTH      0xCAC54u
#define MAPDATA_PTRS  0xFF648u                              /* the mapdataz block, then its record pointers */
#define DOS_MAPDATA   0x378F40u                             /* the original's [0xFF648] (bug 1 above) */

static int tok(int32_t c, int n)  { return S8(AREA(c + 1) + 0x0Eu + (uint32_t)n); }   /* n's tokens in c */
static int mov(int32_t c, int n)  { return S8(AREA(c + 1) + 0x24u + (uint32_t)n); }   /* n's movable tokens in c */
static int cnt(int32_t c)         { return S8(AREA(c + 1) + 1u); }                    /* nations in c */
static int mark(int32_t c, int n) { return S8(AREA(c + 1) + 1u + (uint32_t)n); }      /* n's marker in c */
static int popbits(int32_t c)     { return (uint8_t)(D8(TERR(c) + 1u) << 2) >> 5; }  /* shl al, 2; shr al, 5 */

static void add8(uint32_t a, int v) { setD8(a, (uint8_t)(D8(a) + (uint8_t)v)); }     /* byte add / sub / inc / dec */

static uint32_t ml2(void)            /* the ship move record at the count */
{
    return ML2 + 6u * D32(ML2_COUNT);
}

static uint32_t ml2_next(void)       /* the ship move record at the count, count + 1 (inc then a store at the old index) */
{
    uint32_t i = D32(ML2_COUNT);

    setD32(ML2_COUNT, i + 1u);
    return ML2 + 6u * i;
}

static int32_t rd32(const uint8_t *p)
{
    return (int32_t)((uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24);
}
static void wr32(uint8_t *p, int32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)((uint32_t)v >> 8);
    p[2] = (uint8_t)((uint32_t)v >> 16);
    p[3] = (uint8_t)((uint32_t)v >> 24);
}

/* The bytes at off in the World State Stack's top area block (pushed just before by every caller here). */
static uint8_t *ws_top(uint32_t off, uint32_t n)
{
    uint8_t *p = load_heap_ptr(D32(WS_AREA + 4u * D32(WS_DEPTH)) + off, n);

    if (p == NULL) {   /* port guard: never (a block exists for every pushed slot) */
        snprintf((char *)DP(0xF5500u), 0x1F4, "World State Stack slot %d has no block", (int)S32(WS_DEPTH));
        f44CAC();
    }
    return p;
}

/* The land matrix byte 0xF95FC + 157t + u (f64BB8; bug 1 of the file comment). */
static int land_adj(int32_t t, int32_t u)
{
    uint32_t a, off, h;

    if (t >= 157 && !g_original)
        t = ALIAS(t);   /* BUG(1995) fixed: rows 157..159 lie past the matrix; the second coast's own territory */
    a = LAND_MATRIX + 157u * (uint32_t)t + (uint32_t)u;
    if (a < MAPDATA_PTRS)
        return D8(a);
    /* --original: the DOS address the original keeps in the field, rebuilt from the port's handle */
    off = a - MAPDATA_PTRS;
    h = D32(MAPDATA_PTRS + (off & ~3u));
    if (h != 0)
        h = DOS_MAPDATA + (h - D32(MAPDATA_PTRS));
    return (uint8_t)(h >> (8u * (off & 3u)));
}

/* ---- 0x374E0: the tokens route j collects ------------------------------------------------------------------------ */

/* A saved area record (0xF8194[c] = its slot): +0 int32 c, +4 the 69 bytes (73-byte entries at [ebp-0x66C]). */
#define SAVE_SIZE 73

static void take374(uint8_t save[][SAVE_SIZE], int32_t *nslot, int32_t *taken, uint32_t nat, int j, int32_t m,
                    int32_t c, int n)
{
    if (S32(SLOT + 4u * (uint32_t)c) == -1) {                     /* 0x3773E cmp ebx, -1 */
        setD32(SLOT + 4u * (uint32_t)c, (uint32_t)*nslot);         /* 0x37750 */
        memcpy(save[*nslot] + 4, DP(AREA(c + 1)), 69);             /* 0x37789 rep movsd (17) + movsb */
        wr32(save[*nslot], c);                                     /* 0x377A3 */
        (*nslot)++;                                                /* 0x3778F */
    }
    (*taken)++;                                                   /* 0x377AD */
    add8(nat + 0x5Du, -1);                                        /* 0x377B3 dec byte [nat + 0x5D] */
    add8(ROUTE((uint32_t)j) + 8u + (uint32_t)m, 1);               /* 0x377BF inc byte [16j + m + 0x108828] */
    f57CE8(c, n, 1, 1);                                           /* 0x377D2 */
    setD32(PICKED, 1);                                            /* 0x377D7 */
}

int f374E0(int j, int t, int n, int max, int a)
{
    uint8_t natcopy[199];          /* [ebp-0x110] */
    uint8_t save[10][SAVE_SIZE];   /* [ebp-0x66C]: at most 6 used (one per stop) */
    int32_t visited[160];          /* [ebp-0x390] */
    int32_t nslot = 0;             /* [ebp-0xC] */
    int32_t n1 = n - 1;            /* [ebp-0x14] */
    uint32_t nat = NAT(n);         /* [ebp-0x18] */
    int32_t taken = 0;             /* [ebp-0x3C] */
    int32_t m = 0;                 /* [ebp-0x38] the stop index */
    uint32_t p;                    /* [ebp-0x1C] */
    int32_t c;                     /* [ebp-0x2C] (and [ebp-0x28] = c + 1) */
    int32_t keep;                  /* [ebp-0x34] */
    int32_t e;                     /* [ebp-0x30] */
    int32_t k;                     /* [ebp-0x24] */
    int32_t i;                     /* [ebp-0x20] */

    memcpy(natcopy, DP(nat), 199);                                /* 0x3751E rep movsd (0x31) + movsw + movsb */
    p = ROUTE((uint32_t)j);                                       /* 0x37550 */
    c = ALIAS(D8(p));                                             /* 0x3755C */
    memset(DP(ROUTE((uint32_t)j) + 8u), 0, 6);                    /* 0x37581 */
    memset(visited, 0, sizeof visited);                           /* 0x37593 */

    /* pass 1 (0x37598) */
    while (D8(p) != 0xFF && m <= 5 && taken < 5 && taken < max) {
        if (S8(PICK + (uint32_t)c) == -1) {                       /* 0x375CD dword [c + 0xF84B5] sar 24 */
            if (mov(c, n) > 0 && S8(nat + 0x5Du) > 0 && c != t && !(D8(TST(c) + 2u) & 2)
                && (m != 0 || c != t) && S8(LAND_MATRIX + 157u * (uint32_t)c + (uint32_t)t) == 0) {   /* 0x375E2..0x37662 */
                if (S8(KEEP + (uint32_t)c) == -1) {               /* 0x37668 */
                    k = f495C0(c, n, a, &e);                      /* 0x37685 */
                    setD8(KEEP + (uint32_t)c, (uint8_t)f49790(k, c, n, e));   /* 0x37699, 0x376A1 mov byte, al */
                }
                keep = S8(KEEP + (uint32_t)c);                    /* 0x376A7 */
                if (mov(c, n) < tok(c, n)) {                      /* 0x376DE cmp al, byte; jge (signed) */
                    while (mov(c, n) > keep && S8(nat + 0x5Du) > 0 && taken < 5 && taken < max)   /* 0x376F0 */
                        take374(save, &nslot, &taken, nat, j, m, c, n);
                } else {
                    if (keep == 0 && !(D8(F3580 + 314u * (uint32_t)n1 + 2u * (uint32_t)c) & 0x80))   /* 0x377E6 */
                        keep = 1;                                 /* 0x37801 */
                    while (tok(c, n) > keep && S8(nat + 0x5Du) > 0 && taken < 5 && taken < max)      /* 0x37808 */
                        take374(save, &nslot, &taken, nat, j, m, c, n);
                }
            }
        } else if (visited[c] == 0) {                             /* 0x378FE, 0x37906 */
            for (k = S8(PICK + (uint32_t)c); k > 0 && taken < 5; k--) {   /* 0x37910, 0x3791F */
                taken++;                                          /* 0x3792E */
                add8(ROUTE((uint32_t)j) + 8u + (uint32_t)m, 1);   /* 0x3793A */
            }
            visited[c] = 1;                                       /* 0x3794B */
        }
        p++;                                                      /* 0x37959 */
        c = ALIAS(D8(p));                                         /* 0x37965 */
        m++;                                                      /* 0x37978 */
    }

    /* pass 2 (0x37980): one more from a stop holding a single token, when the first stop has no cached pickups */
    if (taken < max && taken < 5) {
        m = 0;                                                    /* 0x37998 */
        p = ROUTE((uint32_t)j);
        c = ALIAS(D8(p));                                         /* 0x379B4 */
        if (S8(PICK + (uint32_t)c) == -1) {                       /* 0x379C4 */
            while (D8(p) != 0xFF && m <= 5 && taken < 5 && taken < max) {   /* 0x379D9 */
                if (mov(c, n) > 0 && c != t && S8(nat + 0x5Du) > 0 && c != t && !(D8(TST(c) + 2u) & 2)
                    && (m != 0 || c != t) && S8(LAND_MATRIX + 157u * (uint32_t)c + (uint32_t)t) == 0
                    && tok(c, n) == 1) {                          /* 0x37A0E..0x37ABE */
                    if (f49A1C(c, n, 1, -1) != 0                  /* 0x37AD4 */
                        || (S8(nat + 0x5Du) > 0 && (D8(F3580 + 314u * (uint32_t)n1 + 2u * (uint32_t)c) & 0x80)))
                        take374(save, &nslot, &taken, nat, j, m, c, n);   /* 0x37B06 */
                }
                p++;                                              /* 0x37BAF */
                c = ALIAS(D8(p));
                m++;                                              /* 0x37BCE */
            }
        }
    }

    /* pass 3 (0x37BD6): the pickups into the cache of the stops not cached yet */
    m = 0;
    p = ROUTE((uint32_t)j);
    while (D8(p) != 0xFF && m <= 5) {                             /* 0x37BEB */
        c = ALIAS(D8(p));                                         /* 0x37C02 */
        if (S8(PICK + (uint32_t)c) == -1)                         /* 0x37C0D */
            setD8(PICK + (uint32_t)c, D8(ROUTE((uint32_t)j) + 8u + (uint32_t)m));   /* 0x37C2A */
        p++;
        m++;
    }
    if (taken > 5)                                                /* 0x37C3E */
        taken = 5;
    memcpy(DP(nat), natcopy, 199);                                /* 0x37C63 */
    for (i = 0; i < nslot; i++) {                                 /* 0x37C75 */
        c = rd32(save[i]);
        memcpy(DP(AREA(c + 1)), save[i] + 4, 69);                 /* 0x37CC3 */
        setD32(SLOT + 4u * (uint32_t)c, 0xFFFFFFFFu);             /* 0x37CDE */
    }
    return taken;                                                 /* 0x37CEB */
}

/* ---- 0x36918: tokens to t by ship -------------------------------------------------------------------------------- */

/* The plan records: 4 x 28 bytes at [ebp-0xF4], then the swap record at [ebp-0x84]: +4 int32 ship k, +8 int32 stops,
   +0xC int32 tokens, +0x10 int8 pickups[6], +0x16 uint8 stops[6] (+0 never written). */
#define PLAN 28

static int32_t pl32(const uint8_t *pl, int32_t i, int off) { return rd32(pl + PLAN * i + off); }
static void    pl32w(uint8_t *pl, int32_t i, int off, int32_t v) { wr32(pl + PLAN * i + off, v); }

void f36918(int n, int t, int32_t *need, int code)
{
    uint8_t pl[5 * PLAN];          /* [ebp-0xF4]; zeroed here (bug 4) */
    uint8_t tmp[PLAN];
    int8_t cd = (int8_t)code;      /* [ebp-0xC] (movsx byte at each use) */
    uint32_t nat = NAT(n);         /* [ebp-0x30] */
    int32_t np = 0;                /* [ebp-0x10] the plans */
    int32_t rem = *need;           /* [ebp-0x28] */
    int32_t k;                     /* [ebp-0x48] */
    int32_t s;                     /* [ebp-0x2C] */
    int32_t c;                     /* [ebp-0x20] */
    int32_t v, j, x, idx, d, r, pi, m, q, q0, p8, cntr;
    uint32_t rec;

    memset(pl, 0, sizeof pl);   /* BUG(1995) bug 4 (unreachable): zeroed, not the stack's garbage */
    f7A940();                                                     /* 0x36957 */
    for (k = 0; (int)D8(nat + 0x40u) > k; k++) {                  /* 0x36F27 */
        if (rem <= 0)                                             /* 0x36F3E */
            continue;
        s = S16(nat + 0x45u + 2u * (uint32_t)k) - 1;              /* 0x36F4A dword [+0x43 + 2k] sar 16 */
        c = ALIAS(s);                                             /* 0x36F59 */
        if (f5843C(s, t, n, D8(nat + 0x41u + (uint32_t)k)) != 1)   /* 0x36F7A */
            continue;
        f7AA1C();                                                 /* 0x36F84 */
        if (*need + 1 <= mov(c, n)                                /* 0x36FAB cmp eax, edx; jg */
            && D8(LAND_MATRIX + 157u * (uint32_t)c + (uint32_t)t) == 0 && s != t
            && !(D8(TST(c) + 2u) & 2)) {                          /* 0x36FBD..0x36FE9 */
            /* (A) direct: from c straight to t (0x36964) */
            cntr = *need <= 5 ? *need : 5;                        /* 0x36FF2 */
            f57CA4(c, t, n, 1, cntr);                             /* 0x3697E */
            pl32w(pl, np, 8, 1);                                  /* 0x3698F */
            pl[PLAN * np + 0x16] = (uint8_t)s;                    /* 0x369B0 */
            pl[PLAN * np + 0x10] = (uint8_t)cntr;                 /* 0x369CD */
            pl32w(pl, np, 0xC, cntr);                             /* 0x369EA */
            pl32w(pl, np, 4, k);                                  /* 0x36A07 */
            f58888(s, t, n, D8(nat + 0x41u + (uint32_t)k));       /* 0x36A26 */
            memcpy(pl + PLAN * np + 0x16, DP(ROUTE_BUF), 6);      /* 0x36A59 route 0 */
            np++;                                                 /* 0x36A66 */
        } else {
            /* (B) by the routes that collect on the way (0x36A6E) */
            f58888(s, t, n, D8(nat + 0x41u + (uint32_t)k));       /* 0x36A86 */
            memset(DP(KEEP), 0xFF, 0xA0);                         /* 0x36A9A */
            memset(DP(PICK), 0xFF, 0xA0);                         /* 0x36AAE */
            memset(DP(SLOT), 0xFF, 0x280);                        /* 0x36AC2 */
            for (j = 0; j < S32(ROUTE_COUNT); j++) {              /* 0x36AE3 */
                v = f374E0(j, t, n, rem, 0);                      /* 0x36AFE */
                setD8(ROUTE((uint32_t)j) + 7u, (uint8_t)(v < rem ? v : rem));   /* 0x36B09, 0x36AD7 */
            }
            cntr = S32(ROUTE_COUNT);                              /* ecx: the last count read */
            if (cntr < 20) {                                      /* 0x36B13 */
                for (x = cntr - 2; x >= 0; x--)                   /* 0x36B1C */
                    for (j = 0; j <= x; j++)                      /* 0x36B50 */
                        if (S8(ROUTE((uint32_t)j) + 0x17u) > S8(ROUTE((uint32_t)j) + 7u)) {   /* 0x36B70 cmp dl, bl; jle */
                            memcpy(tmp, DP(ROUTE((uint32_t)j) + 16u), 16);                 /* 0x36B83 */
                            memcpy(DP(ROUTE((uint32_t)j) + 16u), DP(ROUTE((uint32_t)j)), 16);   /* 0x36B9F */
                            memcpy(DP(ROUTE((uint32_t)j)), tmp, 16);                       /* 0x36BB2 */
                        }
            } else {
                wc_qsort(DP(ROUTE_BUF), (size_t)cntr, 16, f35368);   /* 0x36BC9 */
            }
            /* the routes tied with route 0 on +6 (0x36BCE; bug 5: bounded by the data object) */
            for (idx = 1; ds_valid(ROUTE((uint32_t)idx) + 6u, 1)
                          && D8(ROUTE_BUF + 6u) == D8(ROUTE((uint32_t)idx) + 6u); idx++)
                ;
            d = 30000 / (idx <= 30000 ? idx : 30000);             /* 0x36C0E idiv */
            v = S16(0x112B0Au + 2u * (uint32_t)n);                /* 0x36C15 dword [2n + 0x112B08] sar 16 */
            r = v / d < idx ? v / d : idx - 1;                    /* 0x36C24..0x36C65 */
            if (S8(ROUTE((uint32_t)r) + 7u) != 0) {               /* 0x36C6F */
                memcpy(pl + PLAN * np + 0x16, DP(ROUTE((uint32_t)r)), 6);        /* 0x36CB1 */
                memcpy(pl + PLAN * np + 0x10, DP(ROUTE((uint32_t)r) + 8u), 6);   /* 0x36CEE */
                pl32w(pl, np, 0xC, 0);                            /* 0x36D06 */
                for (m = 0; m <= 5; m++)                          /* 0x36D18 */
                    pl32w(pl, np, 0xC, pl32(pl, np, 0xC) + S8(ROUTE((uint32_t)r) + 8u + (uint32_t)m));   /* 0x36D3F */
                pl32w(pl, np, 8, 0);                              /* 0x36D4A */
                for (m = 0; m <= 5; m++)                          /* 0x36D5C */
                    if (pl[PLAN * np + 0x16 + m] != 0xFF)
                        pl32w(pl, np, 8, pl32(pl, np, 8) + 1);    /* 0x36D93 */
                rem -= pl32(pl, np, 0xC);                         /* 0x36DA3 */
                pl32w(pl, np, 4, k);                              /* 0x36DBF */
                np++;                                             /* 0x36DA9 */
                /* the trial moves of this plan (undone by the World pop below) */
                pi = np - 1;                                      /* 0x36DCA */
                m = 0;
                q = pl32(pl, pi, 0xC);                            /* 0x36DE5 */
                if (q != 0) {                                     /* 0x36DE8 */
                    while (m <= 5 && q > 0) {                     /* 0x36DF8 */
                        p8 = (int8_t)pl[PLAN * pi + 0x10 + m];    /* 0x36E1E dword [.. - 0xE7] sar 24 */
                        q -= p8;                                  /* 0x36E2E */
                        f57CA4(ALIAS(pl[PLAN * pi + 0x16 + m]), t, n, 1, p8);   /* 0x36E6B */
                        add8(nat + 0x41u + (uint32_t)pl32(pl, pi, 4), -1);      /* 0x36E89 */
                        m++;
                    }
                    while (m <= 5 && pl[PLAN * pi + 0x16 + m] != t) {           /* 0x36E97 */
                        add8(nat + 0x41u + (uint32_t)pl32(pl, pi, 4), -1);      /* 0x36ED9 */
                        m++;
                    }
                    setD16(nat + 0x45u + 2u * (uint32_t)pl32(pl, pi, 4),
                           (uint16_t)(pl[PLAN * pi + 0x16 + m] + 1));           /* 0x36F18 (m 6: bug 4) */
                }
            }
        }
        f7AE14();                                                 /* 0x36F1C */
    }
    f7AC60();                                                     /* 0x37005: every trial move undone */

    /* the plans by their stop counts, fewest first (0x3700A) */
    for (x = 1; x < np; x++)                                      /* 0x37019 */
        for (j = np - 1; j >= x; j--)                             /* 0x37034 */
            if (pl32(pl, j - 1, 8) > pl32(pl, j, 8)) {            /* 0x3706D cmp; jle */
                memcpy(pl + PLAN * 4, pl + PLAN * (j - 1), PLAN); /* 0x37090 to [ebp-0x84] */
                memcpy(pl + PLAN * (j - 1), pl + PLAN * j, PLAN); /* 0x370C1 */
                memcpy(pl + PLAN * j, pl + PLAN * 4, PLAN);       /* 0x370E1 */
            }

    /* the replay (0x370E8) */
    for (pi = 0; pi < np && *need != 0 && (int)D8(nat + 0x40u) > pi; pi++) {   /* 0x370ED */
        m = 0;
        q = pl32(pl, pi, 0xC);                                    /* 0x3712A */
        if (q == 0)                                               /* 0x37137 */
            continue;
        q0 = q;                                                   /* [ebp-0x34] */
        k = pl32(pl, pi, 4);
        while (m <= 5 && q > 0) {                                 /* 0x37145 */
            p8 = (int8_t)pl[PLAN * pi + 0x10 + m];                /* 0x3716B */
            setD8(ml2() + 2u, (uint8_t)p8);                       /* 0x3718D */
            q -= p8;                                              /* 0x37196 */
            *need -= p8;                                          /* 0x3719F */
            f57CA4(ALIAS(pl[PLAN * pi + 0x16 + m]), t, n, 1, p8); /* 0x371D9 */
            rec = ml2();                                          /* 0x371DE */
            setD8(rec + 4u, 3);                                   /* 0x371F0 embark */
            setD8(rec + 3u, (uint8_t)k);                          /* 0x3720A */
            setD8(rec + 0u, pl[PLAN * pi + 0x16 + m]);            /* 0x37229 */
            setD8(rec + 5u, D8(0xC1938u + (uint32_t)(int32_t)cd));   /* 0x37239 (risk 8: kept) */
            if (S8(rec + 2u) != 0)                                /* 0x3723F dword [.. + 0x1079B7] sar 24 */
                setD32(ML2_COUNT, D32(ML2_COUNT) + 1u);           /* 0x3724C */
            rec = ml2();                                          /* 0x37256 */
            setD8(rec + 5u, D8(0xC1938u + (uint32_t)(int32_t)cd));   /* 0x3726E */
            setD8(rec + 4u, 0x0A);                                /* 0x37274 sail */
            setD8(rec + 3u, (uint8_t)k);                          /* 0x3728E */
            add8(nat + 0x41u + (uint32_t)k, -1);                  /* 0x372AD */
            setD8(rec + 0u, pl[PLAN * pi + 0x16 + m]);            /* 0x372DD */
            setD8(ml2_next() + 1u, pl[PLAN * pi + 0x16 + m + 1]); /* 0x3730A, 0x37317 (m 5: bug 4) */
            m++;
        }
        /* the hops on to t (0x37328; bug 4: no bound in the original, here the end of the local) */
        while (PLAN * pi + 0x16 + m + 1 < (int)sizeof pl && pl[PLAN * pi + 0x16 + m] != t) {
            rec = ml2();
            setD8(rec + 5u, D8(0xC1938u + (uint32_t)(int32_t)cd));   /* 0x3736D */
            setD8(rec + 4u, 0x0A);                                /* 0x37373 */
            setD8(rec + 3u, (uint8_t)k);                          /* 0x3738C */
            add8(nat + 0x41u + (uint32_t)k, -1);                  /* 0x373AB */
            setD8(rec + 0u, pl[PLAN * pi + 0x16 + m]);            /* 0x373DB */
            setD8(ml2_next() + 1u, pl[PLAN * pi + 0x16 + m + 1]); /* 0x37405, 0x37412 */
            m++;
        }
        setD16(nat + 0x45u + 2u * (uint32_t)k, (uint16_t)(pl[PLAN * pi + 0x16 + m] + 1));   /* 0x3745A */
        rec = ml2();
        setD8(rec + 5u, D8(0xC1938u + (uint32_t)(int32_t)cd));   /* 0x3747A */
        setD8(rec + 2u, (uint8_t)q0);                             /* 0x37485 */
        setD8(rec + 4u, 5);                                       /* 0x3748B disembark */
        setD8(rec + 3u, (uint8_t)k);                              /* 0x374AA */
        setD8(ml2_next() + 1u, (uint8_t)t);                       /* 0x374BD, 0x374C6 */
    }
}

/* ---- 0x4D29C: the most tokens m's ships can bring to t ----------------------------------------------------------- */

int f4D29C(int t, int m)
{
    int32_t best[256];             /* [ebp-0x58] int32[4] (bug 7) */
    int32_t res = 0;               /* [ebp-0x18] */
    uint32_t nat = NAT(m);
    int32_t saved, k, s, pts, i, x, cntr;
    uint8_t tmp[16];

    f7A940();                                                     /* 0x4D2BB */
    f7AA1C();                                                     /* 0x4D2C0 */
    f460C0(m);                                                    /* 0x4D2C8 */
    saved = S32(0xF5908u);                                        /* 0x4D2CD */
    setD32(0xF5908u, 0);                                          /* 0x4D2D5 edx = 0 */
    f46BB8(m);                                                    /* 0x4D2DE */
    setD32(0xF5908u, (uint32_t)saved);                            /* 0x4D2EB */
    memset(best, 0, sizeof best);                                 /* 0x4D2F5 memset(best, 0, 0x10) */
    for (k = 0; (int)D8(nat + 0x40u) > k; k++) {                  /* 0x4D40D */
        s = S16(nat + 0x45u + 2u * (uint32_t)k) - 1;              /* 0x4D432 */
        pts = D8(nat + 0x41u + (uint32_t)k);                      /* 0x4D44E */
        if (f5843C(s, t, m, pts) != 1)                            /* 0x4D463 */
            continue;
        f58888(s, t, m, pts);                                     /* 0x4D479 */
        memset(DP(KEEP), 0xFF, 0xA0);                             /* 0x4D48D */
        memset(DP(PICK), 0xFF, 0xA0);                             /* 0x4D4A1 */
        memset(DP(SLOT), 0xFF, 0x280);                            /* 0x4D4B7 */
        for (i = 0; i < S32(ROUTE_COUNT); i++)                    /* 0x4D308 */
            setD8(ROUTE((uint32_t)i) + 8u, (uint8_t)f374E0(i, t, m, 999, 1));   /* 0x4D325, 0x4D336 */
        cntr = S32(ROUTE_COUNT);                                  /* edx: the last count read */
        if (cntr < 20) {                                          /* 0x4D33E */
            for (x = cntr - 2; x >= 0; x--)                       /* 0x4D347 */
                for (i = 0; i <= x; i++)                          /* 0x4D37B */
                    if (S8(ROUTE((uint32_t)i) + 0x18u) > S8(ROUTE((uint32_t)i) + 8u)) {   /* 0x4D39B cmp al, ah; jle */
                        memcpy(tmp, DP(ROUTE((uint32_t)i) + 16u), 16);                 /* 0x4D3AE */
                        memcpy(DP(ROUTE((uint32_t)i) + 16u), DP(ROUTE((uint32_t)i)), 16);   /* 0x4D3CA */
                        memcpy(DP(ROUTE((uint32_t)i)), tmp, 16);                       /* 0x4D3DD */
                    }
        } else {
            wc_qsort(DP(ROUTE_BUF), (size_t)cntr, 16, f3530C);    /* 0x4D3F2 */
        }
        best[k] = S8(ROUTE_BUF + 8u);                             /* 0x4D3F7 dword [0x108825] sar 24 */
    }
    f7AC60();                                                     /* 0x4D4C4 */
    f7AE14();                                                     /* 0x4D4CB */
    for (k = 0; (int)D8(nat + 0x40u) > k; k++)                    /* 0x4D4DB */
        if (best[k] > res)                                        /* 0x4D4FC */
            res = best[k];
    return res;
}

/* ---- 0x4AC50: ship tokens away from t ---------------------------------------------------------------------------- */

int f4AC50(int t, int n, int32_t *pcnt, int mode)
{
    uint8_t legs[16];              /* [ebp-0x90] leg 1 (6), 2 padding bytes, [ebp-0x88] leg 2 (6), 2 padding (bug 3) */
    int32_t cand[160];             /* [ebp-0x3B0] */
    int8_t vis[160];               /* [ebp-0x130] */
    uint32_t nat = NAT(n);         /* [ebp-0xC] */
    int32_t landed = 0;            /* [ebp-0x50] */
    int32_t orig = 0;              /* [ebp-0x78] */
    int32_t trial = 0;             /* [ebp-0x2C]: f7C838's result; read only after a call at the same stop */
    int32_t left;                  /* [ebp-0x70] */
    int32_t k;                     /* [ebp-0x18] */
    int32_t s;                     /* [ebp-0x48] */
    int32_t steps;                 /* [ebp-0x40] */
    int32_t avail;                 /* [ebp-0x44] */
    int32_t nc;                    /* [ebp-0x68] */
    int32_t best1;                 /* [ebp-0x24] */
    int32_t best2;                 /* [ebp-0x1C] (then the tokens to carry) */
    int32_t score;                 /* [ebp-0x28] / [ebp-0x20] */
    int32_t loaded;                /* [ebp-0x14] */
    int32_t take;                  /* [ebp-0x4C] (load), [ebp-0x3C] (unload) */
    int32_t c, L, x, i, j, m, sw;
    uint32_t p, rec;

    memset(legs, 0xFF, sizeof legs);   /* BUG(1995) bug 3: the padding read as 0xFF in both modes */
    if (mode > 1) {                                               /* 0x4AC8E */
        orig = *pcnt;                                             /* 0x4AC99 */
        f57E28(t, n, orig);                                       /* 0x4ACA5 */
    }
    for (k = 0; (int)D8(nat + 0x40u) > k && *pcnt > 0; k++) {    /* 0x4ACB7 */
        s = S16(nat + 0x45u + 2u * (uint32_t)k) - 1;              /* 0x4ACDF */
        if (f5843C(s, t, n, D8(nat + 0x41u + (uint32_t)k)) != 1)   /* 0x4AD01 */
            continue;
        if (s != t) {                                             /* 0x4AD11 */
            f58888(s, t, n, D8(nat + 0x41u + (uint32_t)k));       /* 0x4AD2D */
            steps = 1;                                            /* 0x4AD32 */
            for (i = 2; i < 6 && D8(ROUTE_BUF + (uint32_t)i) != 0xFF; i++)   /* 0x4AD48 */
                steps++;
        } else {
            steps = 0;                                            /* 0x4AD6B */
        }
        avail = (int)D8(nat + 0x41u + (uint32_t)k) - steps;       /* 0x4AD81 */
        if (avail <= 0)                                           /* 0x4AD86 */
            continue;
        f5861C(s, n, avail);                                      /* 0x4AD99 */
        setD32(ROUTE_REACH + 4u * (uint32_t)s, 0);                /* 0x4ADA3 */

        /* the free landings in reach (0x4ADB8) */
        nc = 0;
        for (i = 0; i < 0xA0; i++) {
            if (S32(ROUTE_REACH + 4u * (uint32_t)i) != 1)         /* 0x4ADCF */
                continue;
            c = ALIAS(i);                                         /* 0x4ADD9 */
            if ((cnt(c) == 1 && mark(c, n) == n && tok(c, n) < f78D08(c, n))   /* 0x4ADFE..0x4AE4A */
                || (cnt(c) == 0 && !(D8(TST(c) + 2u) & 1) && D8(0xF488Cu + (uint32_t)n) == 0))   /* 0x4AE4C..0x4AE8B */
                cand[nc++] = i;                                   /* 0x4AEA1 */
        }
        for (x = nc - 2; x >= 0; x--)                             /* 0x4AEAD */
            for (j = 0; j <= x; j++)                              /* 0x4AEE4 */
                if (S8(SCORES + 160u * (uint32_t)n + (uint32_t)cand[j + 1])
                    > S8(SCORES + 160u * (uint32_t)n + (uint32_t)cand[j])) {   /* 0x4AF2E cmp al, bl; jle */
                    sw = cand[j + 1];                             /* 0x4AF3C [ebp-0x38] */
                    cand[j + 1] = cand[j];                        /* 0x4AF4C */
                    cand[j] = sw;                                 /* 0x4AF59 */
                }
        if (nc <= 0)                                              /* 0x4AF65 */
            continue;

        /* leg 1: s -> t */
        if (s != t && mode != 1) {                                /* 0x4AF75, 0x4AF7D */
            f58888(s, t, n, steps);                               /* 0x4AF95 */
            best1 = 0;                                            /* 0x4AF9A */
            for (i = 0; i < S32(ROUTE_COUNT); i++) {              /* 0x4B1F2 */
                score = 0;
                p = ROUTE((uint32_t)i);                           /* 0x4B213 */
                for (j = 0; j < 6 && D8(p + (uint32_t)j) != 0xFF; j++) {   /* 0x4AFBD */
                    c = ALIAS(D8(p + (uint32_t)j));               /* 0x4AFEF */
                    L = f78D08(c, n);                             /* 0x4B005 */
                    if (D8(TST(c) + 2u) & 2)                      /* 0x4B019 */
                        continue;
                    if (mark(c, n) != n)                          /* 0x4B03D */
                        continue;
                    if (cnt(c) == 1 && tok(c, n) > L)             /* 0x4B05A, 0x4B07A */
                        goto take1;
                    if (cnt(c) > 1 && mode == 1) {                /* 0x4B097, 0x4B09C (never here: mode != 1) */
                        x = tok(c, n);                            /* 0x4B0B4 */
                        trial = f7C838(c, n, 0, &left);           /* 0x4B0C8 */
                        if (x == trial)                           /* 0x4B0D0 */
                            goto take1;
                    }
                    if (cnt(c) <= 1)                              /* 0x4B0E9 */
                        continue;
                    if (mark(c, n) != n)                          /* 0x4B10D */
                        continue;
                    if (mode < 1)                                 /* 0x4B116 */
                        continue;
                take1:
                    if (mov(c, n) <= 0)                           /* 0x4B13B */
                        continue;
                    if (cnt(c) == 1 && mark(c, n) == n)           /* 0x4B157, 0x4B17B */
                        score += tok(c, n) - L;                   /* 0x4B1A2 */
                    else if (mode == 1)                           /* 0x4AFA8 (never here) */
                        score += trial;                           /* 0x4AFB4 */
                    else
                        score = *pcnt;                            /* 0x4B1AF */
                }
                if (score > best1) {                              /* 0x4B1BA */
                    best1 = score;
                    memcpy(legs, DP(ROUTE((uint32_t)i)), 6);      /* 0x4B1E2 */
                }
            }
        } else if (s != t && mode == 1) {                         /* 0x4B224, 0x4B22C */
            f58888(s, t, n, steps);                               /* 0x4B242 */
            memcpy(legs, DP(ROUTE_BUF), 6);                       /* 0x4B25D */
            if (cnt(t) == 1)                                      /* 0x4B27C */
                best1 = tok(t, n) - f78D08(t, n);                 /* 0x4B2A9 */
            else
                best1 = *pcnt;                                    /* 0x4B2B5 */
        } else {
            if (cnt(t) == 1)                                      /* 0x4B2CF */
                best1 = tok(t, n) - f78D08(t, n);                 /* 0x4B2FC */
            else
                best1 = *pcnt;                                    /* 0x4B308 */
            legs[0] = (uint8_t)t;                                 /* 0x4B30E */
            legs[1] = 0xFF;                                       /* 0x4B314 */
        }
        if (best1 <= 0)                                           /* 0x4B31B */
            continue;

        /* leg 2: t -> each candidate, the route with the most room */
        best2 = 0;                                                /* 0x4B327 */
        for (i = 0; i < nc; i++) {                                /* 0x4B335 */
            memset(vis, 0, sizeof vis);                           /* 0x4B34E */
            f58888(t, cand[i], n, avail);                         /* 0x4B366 */
            for (j = 0; j < S32(ROUTE_COUNT); j++) {              /* 0x4B510 */
                score = 0;
                p = ROUTE((uint32_t)j);                           /* 0x4B531 */
                for (m = 0; m < 6 && D8(p + (uint32_t)m) != 0xFF; m++) {   /* 0x4B394 */
                    c = ALIAS(D8(p + (uint32_t)m));               /* 0x4B3C2 */
                    if (vis[c] != 0)                              /* 0x4B3D5 */
                        continue;
                    if ((cnt(c) == 1 && mark(c, n) == n && tok(c, n) < f78D08(c, n))   /* 0x4B3E3..0x4B444 */
                        || (cnt(c) == 0 && !(D8(TST(c) + 2u) & 1) && D8(0xF488Cu + (uint32_t)n) == 0)) {   /* 0x4B446.. */
                        if (cnt(c) == 1)                          /* 0x4B48B */
                            score += f78D08(c, n) - tok(c, n);    /* 0x4B4C9, 0x4B4CE sub eax, ebx */
                        else
                            score += f78D08(c, n);                /* 0x4B37B */
                        vis[c] = 1;                               /* 0x4B386 */
                    }
                }
                if (score > best2) {                              /* 0x4B4D5 */
                    best2 = score;
                    memcpy(legs + 8, DP(ROUTE((uint32_t)j)), 6);  /* 0x4B500 */
                }
            }
        }
        if (best2 == 0)                                           /* 0x4B53C */
            continue;
        if (best2 > *pcnt)                                        /* 0x4B54E */
            best2 = *pcnt;

        /* leg 1 for real: load on the way, sail to t */
        loaded = 0;                                               /* 0x4B55C */
        for (i = 0; i < 6 && legs[i] != 0xFF; i++) {              /* 0x4B6DF */
            take = 0;                                             /* 0x4B707 */
            c = ALIAS(legs[i]);                                   /* 0x4B70C */
            if (loaded >= best2)                                  /* 0x4B722 */
                goto loaded1;
            L = f78D08(c, n);                                     /* 0x4B730 */
            if (D8(TST(c) + 2u) & 2)                              /* 0x4B74A */
                goto loaded1;
            if (mark(c, n) != n)                                  /* 0x4B76E */
                goto loaded1;
            if (cnt(c) == 1 && tok(c, n) > L)                     /* 0x4B78B, 0x4B7AB */
                goto take2;
            if (cnt(c) > 1) {                                     /* 0x4B7C8 (no mode test here, unlike 0x4B09C) */
                x = tok(c, n);                                    /* 0x4B7DF */
                trial = f7C838(c, n, 0, &left);                   /* 0x4B7F3 */
                if (x == trial)                                   /* 0x4B7FB */
                    goto take2;
            }
            if (cnt(c) <= 1)                                      /* 0x4B814 */
                goto loaded1;
            if (mark(c, n) != n)                                  /* 0x4B838 */
                goto loaded1;
            if (mode < 1)                                         /* 0x4B841 */
                goto loaded1;
        take2:
            if (mov(c, n) <= 0)                                   /* 0x4B866 */
                goto loaded1;
            if (cnt(c) == 1 && mark(c, n) == n) {                 /* 0x4B882, 0x4B8A6 */
                for (x = tok(c, n) - L; x > 0 && loaded < best2 && take < 5; x--) {   /* 0x4B8CF, 0x4B8D2 */
                    take++;
                    loaded++;
                }
            } else if (tok(c, n) == trial) {                      /* 0x4B567 (cnt > 1 here: trial is this stop's) */
                for (x = trial; x > 0 && loaded < best2 && take < 5; x--) {   /* 0x4B58C, 0x4B58F */
                    take++;
                    loaded++;
                }
            }
        loaded1:
            if (take > 0) {                                       /* 0x4B5B7 */
                f57CE8(c, n, 1, take);                            /* 0x4B5CB */
                add8(nat + 0x4Du + (uint32_t)k, take);            /* 0x4B5D9 */
                rec = ml2();                                      /* 0x4B5DC */
                setD8(rec + 2u, (uint8_t)take);                   /* 0x4B5F3 */
                setD8(rec + 4u, 3);                               /* 0x4B5F9 load */
                setD8(rec + 3u, (uint8_t)k);                      /* 0x4B602 */
                setD8(rec + 0u, legs[i]);                         /* 0x4B612 */
                setD8(ml2_next() + 5u, 0x2B);                     /* 0x4B62A, 0x4B631 */
            }
            if (legs[i + 1] != 0xFF) {                            /* 0x4B637 */
                rec = ml2();                                      /* 0x4B651 */
                setD8(rec + 5u, 0x2B);                            /* 0x4B667 */
                setD8(rec + 4u, 0x0A);                            /* 0x4B66D sail */
                setD8(rec + 3u, (uint8_t)k);                      /* 0x4B676 */
                setD8(rec + 0u, legs[i]);                         /* 0x4B68C */
                setD8(ml2_next() + 1u, legs[i + 1]);              /* 0x4B69F, 0x4B6AF */
                add8(nat + 0x41u + (uint32_t)k, -1);              /* 0x4B6BB */
                setD16(nat + 0x45u + 2u * (uint32_t)k, (uint16_t)(legs[i + 1] + 1));   /* 0x4B6D5 */
            }
        }
        if (loaded <= 0)                                          /* 0x4B906 */
            continue;

        /* leg 2 for real: unload where there is room, sail on while tokens are aboard */
        for (i = 0; i < 6 && loaded > 0 && legs[8 + i] != 0xFF; i++) {   /* 0x4BA85 */
            c = ALIAS(legs[8 + i]);                               /* 0x4BAB7 */
            take = 0;                                             /* 0x4BAC9 */
            L = f78D08(c, n);                                     /* 0x4BAD2 (unused) */
            (void)L;
            if ((cnt(c) == 1 && mark(c, n) == n && tok(c, n) < f78D08(c, n))   /* 0x4BAEE..0x4BB3B */
                || (cnt(c) == 0 && !(D8(TST(c) + 2u) & 1))) {    /* 0x4BB3D..0x4BB6C */
                if (cnt(c) == 1)                                  /* 0x4BB86 */
                    take = f78D08(c, n) - tok(c, n);              /* 0x4BBB0, 0x4BBB5 */
                else
                    take = f78D08(c, n);                          /* 0x4B920 */
                if (take > loaded)                                /* 0x4B92B */
                    take = loaded;
            }
            if (take > 0) {                                       /* 0x4B936 */
                f57E28(c, n, take);                               /* 0x4B949 */
                add8(nat + 0x4Du + (uint32_t)k, -take);           /* 0x4B957 sub byte */
                rec = ml2();                                      /* 0x4B95A */
                setD8(rec + 5u, 0x2C);                            /* 0x4B96C */
                setD8(rec + 2u, (uint8_t)take);                   /* 0x4B978 */
                setD8(rec + 4u, 4);                               /* 0x4B97E unload */
                setD8(rec + 3u, (uint8_t)k);                      /* 0x4B987 */
                setD8(ml2_next() + 1u, legs[8 + i]);              /* 0x4B99F, 0x4B9AF */
                loaded -= take;                                   /* 0x4B9B8 */
                *pcnt -= take;                                    /* 0x4B9C1 */
                landed += take;                                   /* 0x4B9C6 */
            }
            if (legs[8 + i + 1] != 0xFF && S8(nat + 0x4Du + (uint32_t)k) > 0) {   /* 0x4B9C9, 0x4B9E3 */
                rec = ml2();                                      /* 0x4B9F7 */
                setD8(rec + 5u, 0x2C);                            /* 0x4BA0D */
                setD8(rec + 4u, 0x0A);                            /* 0x4BA13 */
                setD8(rec + 3u, (uint8_t)k);                      /* 0x4BA1C */
                setD8(rec + 0u, legs[8 + i]);                     /* 0x4BA32 */
                setD8(ml2_next() + 1u, legs[8 + i + 1]);          /* 0x4BA45, 0x4BA55 */
                add8(nat + 0x41u + (uint32_t)k, -1);              /* 0x4BA61 */
                setD16(nat + 0x45u + 2u * (uint32_t)k, (uint16_t)(legs[8 + i + 1] + 1));   /* 0x4BA7B */
            }
        }
    }
    if (mode > 1)                                                 /* 0x4BBBC */
        f57CE8(t, n, 0, orig);                                    /* 0x4BBCD */
    return landed;                                                /* 0x4BBD2 */
}

/* ---- 0x643C8: gather tokens at the marked coastal city sites ----------------------------------------------------- */

/* One gathering pass over list for site t (0x6454F / 0x647B1): up to need tokens, one at a time, from listed
   territories with movable tokens and more than one token, ten rounds over the list. Returns the tokens moved. */
static int32_t gather(int32_t *list, int32_t lcnt, int32_t t, int n, int32_t need)
{
    int32_t i = 0;                 /* [ebp-0x28] */
    int32_t tries = 10;            /* [ebp-0x24] */
    int32_t moved = 0;
    int32_t s, x;

    while (need > 0 && tries > 0) {                               /* 0x6454F */
        s = list[i];                                              /* 0x64566 */
        if (mov(s, n) > 0 && tok(s, n) > 1) {                     /* 0x64592, 0x645B1 */
            f57CA4(s, t, n, 1, 1);                                /* 0x645C6 */
            x = S32(ML1_COUNT);                                   /* 0x645CB */
            setD8(ML1 + 4u * (uint32_t)x + 0u, (uint8_t)s);       /* 0x645D4 */
            setD8(ML1 + 4u * (uint32_t)x + 1u, (uint8_t)t);       /* 0x645E0 */
            setD8(ML1 + 4u * (uint32_t)x + 3u, 0x10);             /* 0x645E9 */
            setD8(ML1 + 4u * (uint32_t)x + 2u, 1);                /* 0x645F3 */
            setD32(ML1_COUNT, (uint32_t)(x + 1));                 /* 0x645FA */
            need--;                                               /* 0x64602 */
            moved++;                                              /* 0x64608 */
        }
        i++;                                                      /* 0x6460E */
        if (i == lcnt) {                                          /* 0x64614 */
            i = 0;
            tries--;                                              /* 0x64625 */
        }
    }
    return moved;
}

void f643C8(int n)
{
    int32_t list[160];             /* [ebp-0x2CC] */
    int32_t lcnt;                  /* [ebp-0x18] */
    int32_t sites[160][2];         /* [ebp-0x7CC]: the territory, the tokens still wanted */
    int32_t ns = 0;                /* [ebp-0x48] */
    int32_t t;                     /* [ebp-0x44] */
    int32_t r;                     /* [ebp-0x1C] = t + 1 */
    int32_t moved;                 /* [ebp-0x40] */
    int32_t need;                  /* [ebp-0x3C] */
    int32_t i, changed;
    uint8_t *w;

    if (S8(0xF488Cu + (uint32_t)n) == 1)                          /* 0x643E6 dword [n + 0xF4889] sar 24 */
        return;
    f7A940();                                                     /* 0x643FF */
    f7AA1C();                                                     /* 0x64406 */
    for (t = 1; t < 0x9D; t++) {                                  /* 0x64458 */
        if (D8(0xEE16Cu + (uint32_t)t) == 0)                      /* 0x6446E */
            continue;
        if (!(D8(TERR(t)) & 2))                                   /* 0x64489 */
            continue;
        if (!(D8(TST(t) + 2u) & 2))                               /* 0x6449A */
            continue;
        if ((int)D8(TST(t)) != n)                                 /* 0x644BA */
            continue;
        r = t + 1;                                                /* 0x644C3 */
        if (mov(t, n) <= 0)                                       /* 0x644E1 */
            continue;
        moved = 0;                                                /* 0x644EB */
        need = mov(t, n);                                         /* 0x64509 [ebp-0x3C] */
        f48028(&lcnt, list, t, n, 0x0F);                          /* 0x6451D */
        if (lcnt > 0) {                                           /* 0x64525 */
            f48D44(&lcnt, list, n, t, 1);                         /* 0x64540 */
            moved = gather(list, lcnt, t, n, need);               /* 0x6454F */
        }
        if (moved != 0) {                                         /* 0x64417 */
            sites[ns][0] = t;                                     /* 0x64423 */
            sites[ns][1] = 0;                                     /* 0x64432 */
            ns++;
            setD16(AREA(r) + 0x39u, (uint16_t)moved);             /* 0x6444B */
        }
    }
    if (ns == 0) {                                                /* 0x6462D */
        f7AC60();                                                 /* 0x64633 */
        f7AE14();                                                 /* 0x64638 */
        f648C8(n, 0);                                             /* 0x64642 */
        return;
    }
    f7A940();                                                     /* 0x6464C */
    f7AA1C();                                                     /* 0x64651 */
    f648C8(n, 0x10);                                              /* 0x64660 */
    changed = 0;                                                  /* 0x64665 */
    for (i = 0; i < ns; i++) {                                    /* 0x64673 */
        r = sites[i][0] + 1;                                      /* 0x64686 */
        if (S16(AREA(r) + 0x39u) != 0) {                          /* 0x6469D */
            w = ws_top(69u * (uint32_t)(r - 1) + 0x39u, 2);       /* 0x646BE dword [.. + 0x37] sar 16 */
            sites[i][1] = (int16_t)(w[0] | w[1] << 8) - S16(AREA(r) + 0x39u);   /* 0x646D9 */
            changed = 1;                                          /* 0x646E5 */
        }
    }
    if (changed == 0) {                                           /* 0x646F1 */
        f7AD24();                                                 /* 0x648AA */
        f7AD7C();
        f7AD24();
        f7AD7C();
        return;
    }
    f7AC60();                                                     /* 0x646FB */
    f7AE14();                                                     /* 0x64700 */
    f7AF28();                                                     /* 0x64705 */
    f7AFB8();                                                     /* 0x6470C */
    for (i = 0; i < ns; i++) {                                    /* 0x6471C */
        r = sites[i][0] + 1;                                      /* 0x64733 */
        t = sites[i][0];                                          /* 0x6473D */
        setD16(AREA(r) + 0x39u, (uint16_t)sites[i][1]);           /* 0x6476C */
        f48028(&lcnt, list, t, n, 0x0F);                          /* 0x64784 */
        if (lcnt <= 0)                                            /* 0x6478C */
            continue;
        f48D44(&lcnt, list, n, 0, 1);                             /* 0x647A2 (ecx = 0, D 0x6479E: not t) */
        (void)gather(list, lcnt, t, n, sites[i][1]);              /* 0x647B1 ([ebp-0x40] = 0, not counted) */
    }
    f648C8(n, 0x10);                                              /* 0x64891 */
    f7AD24();                                                     /* 0x64896 */
    f7AD7C();                                                     /* 0x6489B */
}

/* ---- 0x648C8: the voyages -------------------------------------------------------------------------------------- */

void f648C8(int n, int flags)
{
    uint32_t nat = NAT(n);         /* [ebp-0x18] */
    int32_t full;                  /* [ebp-0x2C] */
    int32_t k;                     /* [ebp-0x30] */
    int32_t t;                     /* [ebp-0x28] */
    int32_t tgt;                   /* [ebp-0x24] */
    int32_t plan;                  /* [ebp-0x1C] */
    int32_t m;                     /* [ebp-0x34] */
    int32_t tm;                    /* [ebp-0x20] */
    int32_t d;
    uint32_t p, rec;

    if (S8(0xF488Cu + (uint32_t)n) == 1)                          /* 0x648E5 */
        return;
    full = (D8(nat + 0x25u) != 0) + 4;                            /* 0x6490B setne; add 4 */
    for (k = 0; (int)D8(nat + 0x40u) > k; k++) {                  /* 0x6492A */
        if ((int)D8(nat + 0x41u + (uint32_t)k) != full)           /* 0x6494C */
            continue;
        t = S16(nat + 0x45u + 2u * (uint32_t)k) - 1;              /* 0x64959 */
        tgt = f64BB8(k, full, n);                                 /* 0x6496C */
        if (tgt > 0) {                                            /* 0x64977 */
            f58888(t, tgt, n, full);                              /* 0x64987 */
            plan = f65010(n, flags);                              /* 0x64992 */
        } else {
            plan = -1;                                            /* 0x6499C */
        }
        if (plan == -1)                                           /* 0x649A3 */
            continue;
        p = ROUTE((uint32_t)plan);                                /* 0x649BA */
        for (m = 0; m <= 5; m++) {                                /* 0x64AEC */
            tm = D8(p + (uint32_t)m);                             /* 0x64B00 */
            if (tm == 0xFF)                                       /* 0x64B06 */
                continue;
            d = S8(p + 8u + (uint32_t)m);                         /* 0x64B13 dword [p + m + 5] sar 24 */
            if (d < 0) {                                          /* 0x64B21 */
                f57CE8(ALIAS(tm), n, 1, -d);                      /* 0x64B45 */
                rec = ml2();                                      /* 0x64B4A */
                setD8(rec + 5u, 6);                               /* 0x64B5C */
                setD8(rec + 4u, 3);                               /* 0x64B70 embark */
                setD8(rec + 2u, (uint8_t)-D8(p + 8u + (uint32_t)m));   /* 0x64B6E neg al, 0x64B76 */
                setD8(rec + 3u, (uint8_t)k);                      /* 0x64B85 */
                setD8(ml2_next() + 0u, (uint8_t)tm);              /* 0x64B98, 0x64BA1 */
            } else if (d > 0) {                                   /* 0x649CD */
                f57E28(ALIAS(tm), n, d);                          /* 0x649EE */
                rec = ml2();                                      /* 0x649F3 */
                setD8(rec + 5u, 6);                               /* 0x64A05 */
                setD8(rec + 2u, D8(p + 8u + (uint32_t)m));        /* 0x64A17 */
                setD8(rec + 4u, 4);                               /* 0x64A1D disembark */
                setD8(rec + 3u, (uint8_t)k);                      /* 0x64A26 */
                setD8(ml2_next() + 1u, (uint8_t)tm);              /* 0x64A3F, 0x64A48 */
            }
            if (m != 5 && D8(p + (uint32_t)m + 1u) != 0xFF) {     /* 0x64A4E, 0x64A66 */
                rec = ml2();                                      /* 0x64A6D */
                setD8(rec + 4u, 0x0A);                            /* 0x64A7F sail */
                setD8(rec + 3u, (uint8_t)k);                      /* 0x64A89 */
                setD8(rec + 0u, (uint8_t)tm);                     /* 0x64A92 */
                setD8(rec + 5u, 6);                               /* 0x64A98 */
                setD8(ml2_next() + 1u, D8(p + (uint32_t)m + 1u)); /* 0x64AB5, 0x64ABE */
                add8(nat + 0x41u + (uint32_t)k, -1);              /* 0x64ACB */
                setD16(nat + 0x45u + 2u * (uint32_t)k, (uint16_t)(D8(p + (uint32_t)m + 1u) + 1));   /* 0x64AE2 */
            }
        }
    }
}

/* ---- 0x64BB8: a ship's destination ------------------------------------------------------------------------------- */

int f64BB8(int k, int mp, int n)
{
    int32_t cand[160];             /* [ebp-0x420] */
    uint8_t seen[160];             /* [ebp-0x100] the landings counted */
    uint8_t load[160];             /* [ebp-0x1A0] the tokens at each stop (stored by territory, read by alias) */
    int32_t tie = 0;               /* [ebp-0x18] */
    int32_t n1 = n - 1;            /* [ebp-0x48] */
    int32_t t;                     /* [ebp-0x20] */
    int32_t nc = 0;                /* [ebp-0x40] */
    int32_t best1 = 0;             /* [ebp-0x38] */
    int32_t best2 = 0;             /* [ebp-0x2C] */
    int32_t res = -1;              /* [ebp-0x1C] */
    int32_t ci;                    /* [ebp-0x10] (read as a word, movsx) */
    int32_t u;                     /* [ebp-0x4C] */
    int32_t r;                     /* [ebp-0x3C] */
    int32_t s1, s2;                /* [ebp-0x34], [ebp-0x28] */
    int32_t lands, loads;          /* [ebp-0x30], [ebp-0x24] */
    int32_t m;                     /* [ebp-0x44] */
    int32_t x, c, v;               /* [ebp-0x58], [ebp-0x54] */

    t = S16(NAT(n) + 0x45u + 2u * (uint32_t)k) - 1;               /* 0x64BED */
    f5861C(t, n, mp);                                             /* 0x64C04 */
    setD32(ROUTE_REACH + 4u * (uint32_t)t, 0);                    /* 0x64C0E */
    for (u = 0; u < 0xA0; u++) {                                  /* 0x64C23 */
        if (S32(ROUTE_REACH + 4u * (uint32_t)u) != 1)             /* 0x64C3A */
            continue;
        if (land_adj(t, u) != 0)                                  /* 0x64C49 (bug 1) */
            continue;
        c = ALIAS(u);                                             /* 0x64C58 */
        if (cnt(c) != 0)                                          /* 0x64C77 */
            continue;
        if (D8(TST(c) + 2u) & 1)                                  /* 0x64C87 */
            continue;
        cand[nc++] = u;                                           /* 0x64C93, 0x64CA0 */
    }
    if (nc == 0)                                                  /* 0x64CAC */
        return -1;
    for (ci = 0; (int16_t)ci < nc; ci++) {                        /* 0x64FBE movsx word [ebp-0x10] */
        u = cand[(int16_t)ci];                                    /* 0x64FCB */
        f58888(t, u, n, mp);                                      /* 0x64FE3 */
        s1 = 0;
        s2 = 0;
        for (r = 0; r < S32(ROUTE_COUNT); r++) {                  /* 0x64E8F */
            memset(seen, 0, sizeof seen);                         /* 0x64EA7 */
            memset(load, 0, sizeof load);                         /* 0x64EBB */
            for (m = 0; m <= mp; m++) {                           /* 0x64CDB */
                x = D8(ROUTE((uint32_t)r) + (uint32_t)m);         /* 0x64CF0 */
                c = ALIAS(x);                                     /* 0x64CFE (read before the 0xFF test) */
                if (x == 0xFF)                                    /* 0x64D0A */
                    break;
                if (D8(TST(c) + 2u) & 2)                          /* 0x64D24 */
                    continue;
                load[x] = (uint8_t)tok(x, n);                     /* 0x64D3C area x + 1, 0x64D45 by territory */
            }
            lands = 0;                                            /* 0x64D50 */
            loads = 0;                                            /* 0x64D53 */
            for (m = 0; m <= mp; m++) {                           /* 0x64D81 */
                x = D8(ROUTE((uint32_t)r) + (uint32_t)m);         /* 0x64D9A */
                c = ALIAS(x);                                     /* 0x64DA8 */
                if (x == 0xFF)                                    /* 0x64DB4 */
                    break;
                if (seen[c] != 0)                                 /* 0x64DCA */
                    continue;
                if (cnt(c) != 0 || (D8(TST(c) + 2u) & 1)) {       /* 0x64DE5, 0x64DFE */
                    v = (int8_t)load[c];                          /* 0x64D5E by alias */
                    if (v > 0) {                                  /* 0x64D6A */
                        loads += v;                               /* 0x64D6C */
                        load[c] = 0;                              /* 0x64D74 */
                    }
                    continue;
                }
                if (!(D8(TERR(c) + 2u) & 1)                       /* 0x64E1B */
                    && D8(0xF4234u + 160u * (uint32_t)n1 + (uint32_t)c) == 0)   /* 0x64E32 */
                    lands++;                                      /* 0x64E3E */
                seen[c] = 1;                                      /* 0x64E44 */
            }
            if (lands > loads)                                    /* 0x64E54 */
                lands = loads;
            if (lands > s1) {                                     /* 0x64E62 */
                s1 = lands;
                s2 = loads;                                       /* 0x64E83 */
            } else if (lands == s1 && loads > s2) {               /* 0x64E77, 0x64E7F */
                s2 = loads;
            }
        }
        if (s1 > best1 && S8(SCORES + 160u * (uint32_t)n + (uint32_t)u) > 0) {   /* 0x64ECB, 0x64EE2 */
            best1 = s1;                                           /* 0x64EF2 */
            best2 = s2;                                           /* 0x64EF8 */
            res = u;                                              /* 0x64FB5 */
        } else if (s1 == best1) {                                 /* 0x64F06 */
            if (s2 > best2) {                                     /* 0x64F14 */
                best2 = s2;
                res = u;
            } else if (s2 == best2 && res != -1                   /* 0x64F29, 0x64F34 */
                       && S8(SCORES + 160u * (uint32_t)n + (uint32_t)u)
                          >= S8(SCORES + 160u * (uint32_t)n + (uint32_t)res)) {   /* 0x64F6B jl */
                x = tie++;                                        /* 0x64F76 */
                v = S16(0x112A94u + 20u * (uint32_t)(x % 8 + 12) + 2u * (uint32_t)n);   /* 0x64F83 idiv 8, 0x64F97 */
                if (v / 300 < 50)                                 /* 0x64FAB idiv 0x12C, 0x64FAD */
                    res = u;
            }
        }
    }
    if (best1 == 0)                                               /* 0x64FF6 */
        res = -1;
    return res;
}

/* ---- 0x65010: score and choose a route --------------------------------------------------------------------------- */

static void back65(uint8_t bk[][SAVE_SIZE], int32_t *nbk, int32_t c)   /* 0x6554F, 0x656AD, 0x657F7, 0x65956 */
{
    if (S32(SLOT + 4u * (uint32_t)c) != -1)
        return;
    setD32(SLOT + 4u * (uint32_t)c, (uint32_t)*nbk);
    memcpy(bk[*nbk] + 4, DP(AREA(c + 1)), 69);
    wr32(bk[*nbk], c);
    (*nbk)++;
}

int f65010(int n, int flags)
{
    int32_t contested[6];          /* [ebp-0xC0] */
    int32_t landed[6];             /* [ebp-0xA8] */
    int32_t over[6];               /* [ebp-0x90] */
    int32_t avail[6];              /* [ebp-0xD8] */
    int32_t seen[160];             /* [ebp-0x358] */
    uint8_t bk[10][SAVE_SIZE];     /* [ebp-0x634]: at most full + 1 used */
    int32_t nbk = 0;               /* [ebp-0x14] */
    uint32_t nat = NAT(n);         /* [ebp-0x20] */
    int32_t n1 = n - 1;            /* [ebp-0x54] */
    int32_t full;                  /* [ebp-0x2C] */
    int32_t r;                     /* [ebp-0x64] */
    int32_t m;                     /* [ebp-0x58] / [ebp-0x44] */
    int32_t u;                     /* [ebp-0x60] */
    int32_t c;                     /* [ebp-0x4C] (and [ebp-0x48] = c + 1) */
    int32_t keep;                  /* [ebp-0x40] */
    int32_t e;                     /* [ebp-0x68] */
    int32_t a;                     /* [ebp-0x50] */
    int32_t tot;                   /* [ebp-0x3C] */
    int32_t room;                  /* [ebp-0x38] */
    int32_t cap, cap2;             /* [ebp-0x34], [ebp-0x30] */
    int32_t idx, d, v, pick, x, i, unloaded;
    uint8_t tmp[16];
    uint8_t *w;

    full = (D8(nat + 0x25u) != 0) + 4;                            /* 0x6504D setne; add 4 */
    if (S32(ROUTE_COUNT) > S32(0x10818Cu))                        /* 0x65064 cmp; jle */
        setD32(0x10818Cu, D32(ROUTE_COUNT));                      /* 0x6506C */
    memset(DP(KEEP), 0xFF, 0xA0);                                 /* 0x65080 */
    f7A940();                                                     /* 0x65085 */
    f7AA1C();                                                     /* 0x6508A */
    for (r = 0; r < S32(ROUTE_COUNT); r++) {                      /* 0x6509C */
        nbk = 0;                                                  /* 0x650AD */
        memset(contested, 0, sizeof contested);                   /* 0x650BD */
        memset(landed, 0, sizeof landed);                         /* 0x650CF */
        memset(over, 0, sizeof over);                             /* 0x650E1 */
        memset(avail, 0, sizeof avail);                           /* 0x650F3 */
        memset(seen, 0, sizeof seen);                             /* 0x65105 */
        tot = 0;                                                  /* 0x6510A */
        room = 0;                                                 /* 0x6510D */

        /* the stops: what to load where, where to land (0x6533A) */
        for (m = 0; m <= full; m++) {
            u = D8(ROUTE((uint32_t)r) + (uint32_t)m);             /* 0x65353 */
            if (u == 0xFF)                                        /* 0x6535F */
                break;
            c = ALIAS(u);                                         /* 0x65370 */
            if (seen[c] != 0)                                     /* 0x65383 */
                continue;
            if (S8(KEEP + (uint32_t)c) == -1) {                   /* 0x6539A */
                keep = f495C0(c, n, 0, &e);                       /* 0x653AE */
                keep = f49790(keep, c, n, e);                     /* 0x653C2 */
                setD8(KEEP + (uint32_t)c, (uint8_t)keep);         /* 0x653D0 */
            } else {
                keep = S8(KEEP + (uint32_t)c);                    /* 0x65118 */
            }
            if (keep == 0 && (!(D8(F3580 + 314u * (uint32_t)n1 + 2u * (uint32_t)c) & 0x80)   /* 0x65137 */
                              || (D8(TERR(c) + 2u) & 4)))         /* 0x65152 */
                keep = 1;                                         /* 0x6515B */
            if (flags == 0x10 && (D8(TST(c) + 2u) & 2) && (int)D8(TST(c)) == n) {   /* 0x65162..0x65197 */
                keep = 0;                                         /* 0x6519B */
                over[m] = 1;                                      /* 0x651A1 */
            }
            if (!(D8(TST(c) + 2u) & 2)                            /* 0x651B8 */
                && tok(c, n) > popbits(c) + S8(nat + 0x27u)       /* 0x65209 */
                && cnt(c) > 1)                                    /* 0x65221 */
                over[m] = 1;                                      /* 0x65229 */
            if (((flags == 0 && !(D8(TST(c) + 2u) & 2)) || flags == 0x10)   /* 0x65234, 0x6524F */
                && mov(c, n) > keep) {                            /* 0x65274 */
                if (flags == 0x10 && (D8(TST(c) + 2u) & 2) && (int)D8(TST(c)) == n)   /* 0x65280..0x652B4 */
                    a = S16(AREA(c + 1) + 0x39u);                 /* 0x652C0 dword [.. + 0xE31EF] sar 16 */
                else
                    a = mov(c, n) - keep;                         /* 0x652EA */
                tot += a;                                         /* 0x652F2 */
                avail[m] = (int8_t)a;                             /* 0x652F5 movsx byte */
                seen[c] = 1;                                      /* 0x6530B */
                if (cnt(c) > 1)                                   /* 0x65326 */
                    contested[m] = 1;                             /* 0x6532D */
                continue;
            }
            /* a landing (0x653DB) */
            if (D8(TERR(c) + 2u) & 2)                             /* 0x653F2 */
                continue;
            if (D8(TST(c) + 2u) & 1)                              /* 0x65407 */
                continue;
            if (cnt(c) != 0)                                      /* 0x65428 */
                continue;
            if (landed[m] != 0)                                   /* 0x6543A */
                continue;
            if (tot <= 0)                                         /* 0x65442 */
                continue;
            landed[m] = 1;                                        /* 0x65454 */
            seen[c] = 1;                                          /* 0x6545E */
            room += popbits(c) + S8(nat + 0x27u);                 /* 0x65491 */
        }
        if (tot > 5)                                              /* 0x65499 */
            tot = 5;
        cap = tot > room ? room : tot;                            /* 0x654A9 */
        cap2 = cap;                                               /* 0x654BC */
        memset(DP(ROUTE((uint32_t)r) + 8u), 0, 6);                /* 0x654D4 */
        if (tot <= 0) {                                           /* 0x654D9 */
            setD16(ROUTE((uint32_t)r) + 0x0Eu, 0xFED4);           /* 0x65A12 -300 */
        } else {
            /* embark: contested stops first (0x654F0) */
            for (m = 0; m <= full && cap > 0; m++) {
                if (contested[m] == 0)                            /* 0x65510 */
                    continue;
                c = ALIAS(D8(ROUTE((uint32_t)r) + (uint32_t)m)); /* 0x65528 */
                while (cap > 0 && avail[m] > 0) {                 /* 0x6553C */
                    back65(bk, &nbk, c);
                    add8(ROUTE((uint32_t)r) + 8u + (uint32_t)m, -1);   /* 0x655C8 */
                    cap--;                                        /* 0x655D1 */
                    avail[m]--;                                   /* 0x655D7 */
                    f57CE8(c, n, 1, 1);                           /* 0x655EC */
                }
            }
            /* then the stops over their limit or marked (0x65603) */
            for (m = 0; m <= full && cap > 0; m++) {
                if (over[m] == 0)                                 /* 0x65625 */
                    continue;
                c = ALIAS(D8(ROUTE((uint32_t)r) + (uint32_t)m)); /* 0x6563B */
                while (cap > 0 && avail[m] > 0) {                 /* 0x6564F */
                    if (!((flags == 0 && mov(c, n) > 0)           /* 0x65662, 0x65683 */
                          || (flags == 0x10 && S16(AREA(c + 1) + 0x39u) > 0)))   /* 0x65687, 0x656A5 */
                        break;
                    back65(bk, &nbk, c);
                    add8(ROUTE((uint32_t)r) + 8u + (uint32_t)m, -1);   /* 0x65726 */
                    cap--;                                        /* 0x6572F */
                    avail[m]--;                                   /* 0x65735 */
                    f57CE8(c, n, 1, 1);                           /* 0x6574A */
                    if (flags == 0x10) {                          /* 0x6574F */
                        w = ws_top(69u * (uint32_t)c + 0x39u, 2); /* 0x65776 dec word [top + 69c + 0x39] */
                        v = (w[0] | w[1] << 8) - 1;
                        w[0] = (uint8_t)v;
                        w[1] = (uint8_t)(v >> 8);
                    }
                }
            }
            /* then any stop (0x6578D) */
            for (m = 0; m <= full && cap > 0; m++) {
                u = D8(ROUTE((uint32_t)r) + (uint32_t)m);         /* 0x657AE */
                c = ALIAS(u);                                     /* 0x657BC */
                if (u == 0xFF)                                    /* 0x657CF */
                    continue;
                if (avail[m] <= 0)                                /* 0x657DA */
                    continue;
                while (cap > 0 && avail[m] > 0) {                 /* 0x657E4 */
                    back65(bk, &nbk, c);
                    add8(ROUTE((uint32_t)r) + 8u + (uint32_t)m, -1);   /* 0x65870 */
                    cap--;                                        /* 0x65879 */
                    avail[m]--;                                   /* 0x6587F */
                    f57CE8(c, n, 1, 1);                           /* 0x65894 */
                }
            }
            /* unload from the last landing back, one token per landing per sweep (0x6589E; bug 6) */
            while (cap2 > 0) {
                unloaded = 0;
                for (m = full; m >= 0 && cap2 > 0; m--) {         /* 0x658B6 */
                    if (landed[m] == 0)                           /* 0x658CC */
                        continue;
                    u = D8(ROUTE((uint32_t)r) + (uint32_t)m);     /* 0x658DD */
                    c = ALIAS(u);                                 /* 0x658EB */
                    if (u == 0xFF)                                /* 0x658FE */
                        continue;
                    if (tok(c, n) >= popbits(c) + S8(nat + 0x27u))   /* 0x6594E cmp; jge */
                        continue;
                    back65(bk, &nbk, c);
                    cap2--;                                       /* 0x659C9 */
                    add8(ROUTE((uint32_t)r) + 8u + (uint32_t)m, 1);   /* 0x659D5 */
                    f57E28(c, n, 1);                              /* 0x659E6 */
                    unloaded = 1;
                }
                if (unloaded == 0 && !g_original)                 /* BUG(1995) fixed: a sweep with no room ends */
                    break;
            }
            setD16(ROUTE((uint32_t)r) + 0x0Eu, (uint16_t)f7B044(n));   /* 0x659F3, 0x659FE */
        }
        for (i = 0; i < nbk; i++) {                               /* 0x65A28 */
            c = rd32(bk[i]);
            memcpy(DP(AREA(c + 1)), bk[i] + 4, 69);               /* 0x65A78 */
            setD32(SLOT + 4u * (uint32_t)c, 0xFFFFFFFFu);         /* 0x65A93 */
        }
    }
    f7AC60();                                                     /* 0x65AA0 */
    f7AE14();                                                     /* 0x65AA5 */

    /* best score first (0x65AAA) */
    x = S32(ROUTE_COUNT);
    if (x < 20) {                                                 /* 0x65AAF */
        for (d = x - 2; d >= 0; d--)                              /* 0x65AB8 */
            for (i = 0; i <= d; i++)                              /* 0x65AEC */
                if (S16(ROUTE((uint32_t)i) + 0x1Eu) > S16(ROUTE((uint32_t)i) + 0x0Eu)) {   /* 0x65B0E cmp dx, si; jle */
                    memcpy(tmp, DP(ROUTE((uint32_t)i) + 16u), 16);                     /* 0x65B22 */
                    memcpy(DP(ROUTE((uint32_t)i) + 16u), DP(ROUTE((uint32_t)i)), 16);  /* 0x65B3E */
                    memcpy(DP(ROUTE((uint32_t)i)), tmp, 16);                           /* 0x65B51 */
                }
    } else {
        wc_qsort(DP(ROUTE_BUF), (size_t)x, 16, f65C4C);           /* 0x65B68 */
    }
    /* the routes tied with route 0 on +6 and the score (0x65B72) */
    for (idx = 0; D8(ROUTE_BUF + 6u) == D8(ROUTE((uint32_t)idx) + 6u)
                  && S16(ROUTE_BUF + 0x0Eu) == S16(ROUTE((uint32_t)idx) + 0x0Eu) && idx < S32(ROUTE_COUNT); idx++)
        ;
    if (idx == 0)   /* BUG(1995) bug 8: no route, 30000 / 0 traps; never (G: f64BB8 chose t from these routes) */
        return -1;
    d = 30000 / (idx <= 30000 ? idx : 30000);                     /* 0x65BCB idiv */
    v = S16(0x112AA6u + 2u * (uint32_t)n);                        /* 0x65BD2 dword [2n + 0x112AA4] sar 16 */
    pick = v / d < idx ? v / d : idx - 1;                         /* 0x65BE1..0x65C22 */
    return S16(ROUTE((uint32_t)pick) + 0x0Eu) >= 0 ? pick : -1;   /* 0x65C2C dword [16p + 0x10882C] sar 16; jl */
}

/* ---- 0x65CB0: idle ships toward the largest stacks ---------------------------------------------------------------- */

void f65CB0(int n)
{
    int32_t occ[160];              /* [ebp-0x2C8] the ships' territories and the targets taken */
    int32_t cand[160][2];          /* [ebp-0x7C8]: territory, n's tokens there */
    int32_t tmp[2];                /* [ebp-0x48] */
    uint32_t nat;                  /* [ebp-0x1C] */
    int32_t k;                     /* [ebp-0x38] */
    int32_t s;                     /* [ebp-0x34] */
    int32_t nc;                    /* [ebp-0x20] */
    int32_t tgt;                   /* [ebp-0x3C] */
    int32_t u, c, x, i, m;
    uint32_t rec;

    f7A940();                                                     /* 0x65CCB */
    f5AAA4(0);                                                    /* 0x65CD2 the conflicts, silently */
    f5C238(0);                                                    /* 0x65CD9 the surplus, silently */
    nat = NAT(n);                                                 /* 0x65CEC */
    memset(occ, 0, sizeof occ);                                   /* 0x65D03 */
    for (k = 0; (int)D8(nat + 0x40u) > k; k++) {                  /* 0x65D15 */
        s = S16(nat + 0x45u + 2u * (uint32_t)k) - 1;              /* 0x65D2D */
        if (s >= 0 && s < 160)                                    /* bug 7: port guard */
            occ[s] = 1;                                           /* 0x65D33 [ebp + 4*word - 0x2CC] */
    }
    for (k = 0; (int)D8(nat + 0x40u) > k; k++) {                  /* 0x65D4D */
        if (D8(nat + 0x41u + (uint32_t)k) == 0)                   /* 0x65D6F */
            continue;
        s = S16(nat + 0x45u + 2u * (uint32_t)k) - 1;              /* 0x65D80 */
        f5861C(s, n, D8(nat + 0x41u + (uint32_t)k));              /* 0x65D98 */
        setD32(ROUTE_REACH + 4u * (uint32_t)s, 1);                /* 0x65DA7 */
        for (u = 0; u < 0xA0; u++)                                /* 0x65DB9 */
            if (occ[u] != 0)                                      /* 0x65DCC */
                setD32(ROUTE_REACH + 4u * (uint32_t)u, 0);        /* 0x65DD5 */
        nc = 0;                                                   /* 0x65DE0 */
        for (u = 0; u < 0xA0; u++) {                              /* 0x65DEE */
            if (S32(ROUTE_REACH + 4u * (uint32_t)u) != 1)         /* 0x65E08 */
                continue;
            c = ALIAS(u);                                         /* 0x65E0F */
            if (mark(c, n) != n)                                  /* 0x65E3A */
                continue;
            if (D8(TST(c) + 2u) & 2)                              /* 0x65E4F */
                continue;
            cand[nc][0] = u;                                      /* 0x65E57 */
            cand[nc][1] = tok(c, n);                              /* 0x65E81 */
            nc++;                                                 /* 0x65E7E */
        }
        if (nc <= 0)                                              /* 0x65E8D */
            continue;
        for (x = nc - 2; x >= 0; x--)                             /* 0x65E97 */
            for (i = 0; i <= x; i++)                              /* 0x65ECA */
                if (cand[i + 1][1] > cand[i][1]) {                /* 0x65EE6 cmp edx, ecx; jle */
                    tmp[0] = cand[i + 1][0];                      /* 0x65EF7 */
                    tmp[1] = cand[i + 1][1];
                    cand[i + 1][0] = cand[i][0];                  /* 0x65F0D */
                    cand[i + 1][1] = cand[i][1];
                    cand[i][0] = tmp[0];                          /* 0x65F1C */
                    cand[i][1] = tmp[1];
                }
        tgt = cand[0][0];                                         /* 0x65F20 */
        occ[tgt] = 1;                                             /* 0x65F2C */
        if (tgt == s)                                             /* 0x65F3A */
            continue;
        f58888(s, tgt, n, D8(nat + 0x41u + (uint32_t)k));         /* 0x65F5B */
        for (m = 0; m <= 5; m++) {                                /* 0x65F74 route 0 */
            u = D8(ROUTE_BUF + (uint32_t)m);                      /* 0x65F86 */
            if (u == 0xFF)                                        /* 0x65F8E */
                continue;
            if (m == 5)                                           /* 0x65F98 */
                continue;
            if (D8(ROUTE_BUF + (uint32_t)m + 1u) == 0xFF)         /* 0x65FAB */
                continue;
            rec = ml2();                                          /* 0x65FB2 */
            setD8(rec + 4u, 0x0A);                                /* 0x65FC4 sail */
            setD8(rec + 3u, (uint8_t)k);                          /* 0x65FCE */
            setD8(rec + 0u, (uint8_t)u);                          /* 0x65FD9 */
            setD8(rec + 5u, 0x2A);                                /* 0x65FDF */
            setD8(ml2_next() + 1u, D8(ROUTE_BUF + (uint32_t)m + 1u));   /* 0x65FFD, 0x66006 */
            add8(nat + 0x41u + (uint32_t)k, -1);                  /* 0x66013 */
            setD16(nat + 0x45u + 2u * (uint32_t)k, (uint16_t)(D8(ROUTE_BUF + (uint32_t)m + 1u) + 1));   /* 0x6602A */
        }
    }
    f7AC60();                                                     /* 0x66033: the state back, the move list kept */
}
