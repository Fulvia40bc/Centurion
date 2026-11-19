/* route_test [GAMEDIR]: unit vectors for module ROUTE (route.c) that need no game run.

   It loads the data object from GAMEDIR/AC_MAIN.EXE as the port does (state_init: le_image.c; GAMEDIR defaults to the
   run copy C:\work\advciv_run, only read). Expectations are written here from the disassembly's semantics,
   independently of route.c:
     A. hand vectors on a small synthetic N.RL written into the tables (0xFFD44, 0x101DC8, the index arrays 0x107410 /
        0x107550 and a heap block at [0x10740C]): every result worked out by hand from the listing, quirks included
        (the blocked territory left out of the slot, the uncounted route that ends the scan, the redo rounds and their
        order, the reversal and shift of a swapped pair, the +6 byte, k == 0 leaving the buffer as it was, record -1,
        f58A74 and f58DCC called directly, and the latent stale bytes of an unsorted record, plain and swapped);
        the whole data object must equal the snapshot taken before plus exactly the bytes listed;
     B. a table over the real N.RL files 3.RL..8.RL of GAMEDIR, each loaded into the data object the way f3EED8 does
        (0x2084 bytes to 0xFFD44, 0x5644 to 0x101DC8, the rest to a heap block, the first-record indexes) and read a
        second time from the file into plain arrays for a second implementation of the seven routines (written from
        the listing, not from route.c). For every file, both tables (Astronomy off / on: nation record +0x2B), three
        token layouts (none, every territory, random for all nine nations), every ordered pair (a, b) of 0..159 and
        k = -1..6: f58888's whole buffer 0x108820 (0x15E0 bytes) and count [0x109E00] against the model, f5843C's
        result, and f5861C's reach table 0x107EE4 (per t); a sample of the calls also checks that nothing else in the
        data object changed. Semantic checks besides (independent of the byte emulation): each kept record runs from
        the caller's a to its b with fewer than k intermediates, and +6 = k - steps.
     C. data facts the file comment of route.c relies on: routes sorted by length in every record, at most 323 routes
        per record, every route byte < 160 or 0xFF, every route inside its block.
   Prints each failure (the first few), the counts per part, and exits 0 when all pass.

   Link: route_test.c route.c load.c state.c le_image.c (load.c: the heap handles; it brings its own f128BC). f44CAC
   (route.c's port guard) is a stub that reports and exits 2 (no call expected). */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/state.h"
#include "../src/game.h"
#include "../src/ui.h"
#include "../src/route.h"

/* ---- stubs ------------------------------------------------------------------------------------------------------ */

_Noreturn void f44CAC(void) { fprintf(stderr, "route_test: quit: %s\n", DSTR(0xF5500u)); exit(2); }
void f3A780(const char *msg) { fprintf(stderr, "route_test: load.c message: %s\n", msg); exit(2); }   /* load.c's */

/* ---- bookkeeping ------------------------------------------------------------------------------------------------ */

static long g_tests, g_fail;
static uint8_t *g_before, *g_want;

static void fail(const char *fmt, const char *what, long a, long b, long c, long d)
{
    g_fail++;
    if (g_fail <= 40) {
        printf("FAIL ");
        printf(fmt, what, a, b, c, d);
        printf("\n");
    }
}

static void snap(void)
{
    memcpy(g_before, g_ds, DS_SIZE);
    memcpy(g_want, g_ds, DS_SIZE);
}

static void W8(uint32_t a, uint8_t v) { g_want[a - DS_BASE] = v; }
static void W32(uint32_t a, uint32_t v)
{
    g_want[a - DS_BASE] = (uint8_t)v; g_want[a - DS_BASE + 1] = (uint8_t)(v >> 8);
    g_want[a - DS_BASE + 2] = (uint8_t)(v >> 16); g_want[a - DS_BASE + 3] = (uint8_t)(v >> 24);
}

/* the whole data object must equal g_want */
static void verify_ds(const char *what, long r, long want_r)
{
    uint32_t i;

    g_tests++;
    if (r != want_r) {
        fail("%s: returned %ld, want %ld%ld%ld", what, r, want_r, 0, 0);
        return;
    }
    for (i = 0; i < DS_SIZE; i++)
        if (g_ds[i] != g_want[i]) {
            fail("%s: byte 0x%lX = 0x%02lX, want 0x%02lX%ld", what, (long)(i + DS_BASE), g_ds[i], g_want[i], 0);
            return;
        }
}

/* ---- area / nation fields --------------------------------------------------------------------------------------- */

#define TOK(u, n)  (0xE31B8u + 69u * (uint32_t)((u) + 1) + 0x0Eu + (uint32_t)(n))
#define ASTRO(n)   (0xF14CCu + 199u * (uint32_t)(n) + 0x2Bu)

static void clear_tokens(void)
{
    int u, n;
    for (u = 0; u < 160; u++)
        for (n = 1; n <= 9; n++)
            setD8(TOK(u, n), 0);
}

static void set_astro(int n, int on)
{
    int m;
    for (m = 0; m <= 13; m++)
        setD8(ASTRO(m), (uint8_t)(on ? 0 : 1));     /* every other nation the opposite */
    setD8(ASTRO(n), (uint8_t)(on ? 1 : 0));
}

/* ---- A. hand vectors on a synthetic N.RL ------------------------------------------------------------------------ */

static uint32_t g_block;   /* the heap handle in [0x10740C] */

static void put_rec(uint32_t tab, int e, int from, int to, int cnt, uint32_t idx)
{
    uint32_t r = tab + 4u + 8u * (uint32_t)e;
    setD8(r, (uint8_t)from); setD8(r + 1, (uint8_t)to); setD16(r + 2, (uint16_t)cnt); setD32(r + 4, idx);
}

static void synth(void)
{
    static const uint8_t routes[6][4] = {
        { 20, 0xFF, 0xFF, 0xFF }, { 20, 21, 0xFF, 0xFF }, { 30, 31, 32, 0xFF },   /* table 1: 3 -> 9 */
        { 40, 41, 42, 43 },                                                        /* table 1: 3 -> 12 */
        { 17, 0xFF, 0xFF, 0xFF },                                                  /* table 1: 5 -> 5 */
        { 50, 0xFF, 0xFF, 0xFF },                                                  /* table 2: 3 -> 9 */
    };
    int t;
    uint8_t *p;

    load_heap_free(D32(GAME_RL_BLOCK));
    g_block = load_heap_alloc(sizeof routes);
    p = load_heap_ptr(g_block, sizeof routes);
    memcpy(p, routes, sizeof routes);
    setD32(GAME_RL_BLOCK, g_block);
    memset(DP(GAME_RL_TABLE1), 0, 0x2084);
    memset(DP(GAME_RL_TABLE2), 0, 0x5644);
    setD32(GAME_RL_TABLE1, 3);
    put_rec(GAME_RL_TABLE1, 0, 3, 9, 3, 0);
    put_rec(GAME_RL_TABLE1, 1, 3, 12, 1, 3);
    put_rec(GAME_RL_TABLE1, 2, 5, 5, 1, 4);
    setD32(GAME_RL_TABLE2, 1);
    put_rec(GAME_RL_TABLE2, 0, 3, 9, 1, 5);
    for (t = 0; t < 160; t++) {
        setD16(GAME_RL_FIRST1 + 2u * (uint32_t)t, 0xFFFF);
        setD16(GAME_RL_FIRST2 + 2u * (uint32_t)t, 0xFFFF);
    }
    setD16(GAME_RL_FIRST1 + 2u * 3, 0); setD16(GAME_RL_FIRST1 + 2u * 5, 2);
    setD16(GAME_RL_FIRST2 + 2u * 3, 0);
    setD32(0xFFD40u, 0);
}

/* the buffer as the hand vector says: all 0xFF except the listed slots (6..16 bytes each, -1 = leave 0xFF) */
static void want_buf(int nslots, const int slots[][8])
{
    int s, i;
    memset(g_want + (ROUTE_BUF - DS_BASE), 0xFF, 0x15E0);
    for (s = 0; s < nslots; s++)
        for (i = 0; i < 7; i++)
            if (slots[s][i] >= 0)
                W8(ROUTE_BUF + 16u * (uint32_t)slots[s][7] + (uint32_t)i, (uint8_t)slots[s][i]);
}

static void reach_want(const int *ts, int n)
{
    int i;
    memset(g_want + (ROUTE_REACH - DS_BASE), 0, 0x280);
    for (i = 0; i < n; i++)
        W32(ROUTE_REACH + 4u * (uint32_t)ts[i], 1);
}

static void part_a(void)
{
    synth();
    clear_tokens();
    set_astro(2, 0);
    memset(DP(ROUTE_BUF), 0x5A, 0x15E0);
    setD32(ROUTE_COUNT, 77);

    /* f58888(9, 3, 2, 3), no tokens: five redo rounds clear 21, 30, 31, 32, then 20; two routes kept, the third
       written to slot 2 uncounted; swapped: reversed and shifted; +6 = 1 and 0 */
    {
        static const int s[3][8] = {
            { 9, 20, 3, 0xFF, 0xFF, 0xFF, 1, 0 }, { 9, 21, 20, 3, 0xFF, 0xFF, 0, 1 }, { 3, 30, 31, 32, 9, 0xFF, -1, 2 } };
        snap(); want_buf(3, s); W32(ROUTE_COUNT, 2);
        f58888(9, 3, 2, 3);
        verify_ds("A1 f58888(9,3,2,3) no tokens", 0, 0);
    }
    /* tokens of nation 2 in 20: route 0 kept at once; route 1 (21 blocked) leaves [3,20,FF,9]; route 2 ends the scan
       in slot 1 (30..32 blocked, not copied): [3,20,FF,9,9,FF] */
    {
        static const int s[2][8] = { { 3, 20, 9, 0xFF, 0xFF, 0xFF, 1, 0 }, { 3, 20, 0xFF, 9, 9, 0xFF, -1, 1 } };
        setD8(TOK(20, 2), 1);
        snap(); want_buf(2, s); W32(ROUTE_COUNT, 1);
        f58888(3, 9, 2, 3);
        verify_ds("A2 f58888(3,9,2,3) tokens in 20", 0, 0);
        setD8(TOK(20, 3), 1); setD8(TOK(20, 2), 0);   /* another nation's tokens do not count */
        {
            static const int s2[3][8] = {
                { 3, 20, 9, 0xFF, 0xFF, 0xFF, 1, 0 }, { 3, 20, 21, 9, 0xFF, 0xFF, 0, 1 }, { 3, 30, 31, 32, 9, 0xFF, -1, 2 } };
            snap(); want_buf(3, s2); W32(ROUTE_COUNT, 2);
            f58888(3, 9, 2, 3);
            verify_ds("A2b f58888(3,9,2,3) tokens of nation 3 only", 0, 0);
        }
        setD8(TOK(20, 3), 0);
    }
    /* k == 0: count 0, the buffer as it was */
    memset(DP(ROUTE_BUF), 0x33, 0x15E0);
    setD32(ROUTE_COUNT, 5);
    snap(); W32(ROUTE_COUNT, 0);
    f58888(9, 3, 2, 0);
    verify_ds("A3 f58888 k = 0", 0, 0);
    /* k == 1: the first route (1 intermediate) ends every pass; 20 cleared once; slot 0 = [3,20,9]; count 0 */
    {
        static const int s[1][8] = { { 3, 20, 9, 0xFF, 0xFF, 0xFF, -1, 0 } };
        snap(); want_buf(1, s); W32(ROUTE_COUNT, 0);
        f58888(3, 9, 2, 1);
        verify_ds("A4 f58888(3,9,2,1)", 0, 0);
    }
    /* Astronomy: table 2's route [50]: blocked, cleared, kept; swapped -> [9,50,3], +6 = 2 - 2 = 0 */
    {
        static const int s[1][8] = { { 9, 50, 3, 0xFF, 0xFF, 0xFF, 0, 0 } };
        set_astro(2, 1);
        snap(); want_buf(1, s); W32(ROUTE_COUNT, 1);
        f58888(9, 3, 2, 2);
        verify_ds("A5 f58888(9,3,2,2) Astronomy", 0, 0);
        set_astro(2, 0);
    }
    /* 4 intermediates, all with tokens: k = 5 keeps it (+6 = 0, no 0xFF in bytes 2..5); k = 4 ends the scan */
    {
        static const int s[1][8] = { { 3, 40, 41, 42, 43, 12, 0, 0 } };
        static const int s4[1][8] = { { 3, 40, 41, 42, 43, 12, -1, 0 } };
        setD8(TOK(40, 2), 2); setD8(TOK(41, 2), 0x80); setD8(TOK(42, 2), 1); setD8(TOK(43, 2), 0xFF);
        snap(); want_buf(1, s); W32(ROUTE_COUNT, 1);
        f58888(3, 12, 2, 5);
        verify_ds("A6 f58888(3,12,2,5)", 0, 0);
        snap(); want_buf(1, s4); W32(ROUTE_COUNT, 0);
        f58888(3, 12, 2, 4);
        verify_ds("A6b f58888(3,12,2,4)", 0, 0);
        /* the swapped 4-intermediate route: [12,43,42,41,40,3], +6 = 0; k = 300 gives (uint8)(300 - 5) = 0x27 */
        {
            static const int s5[1][8] = { { 12, 43, 42, 41, 40, 3, 0x27, 0 } };
            snap(); want_buf(1, s5); W32(ROUTE_COUNT, 1);
            f58888(12, 3, 2, 300);
            verify_ds("A6c f58888(12,3,2,300)", 0, 0);
        }
        clear_tokens();
    }
    /* sea territory 17 (type 0) never blocks; the self route 5 -> 5 */
    {
        static const int s[1][8] = { { 5, 17, 5, 0xFF, 0xFF, 0xFF, 1, 0 } };
        snap(); want_buf(1, s); W32(ROUTE_COUNT, 1);
        f58888(5, 5, 2, 3);
        verify_ds("A7 f58888(5,5,2,3) sea", 0, 0);
    }
    /* first index -1: record -1 (0xFFD40: from 0, to 0, 0 routes), then the whole table: nothing */
    snap(); want_buf(0, NULL); W32(ROUTE_COUNT, 0);
    f58888(4, 9, 2, 3);
    verify_ds("A8 f58888(4,9,2,3) no record", 0, 0);
    snap(); want_buf(0, NULL); W32(ROUTE_COUNT, 0);
    f58888(0, 0, 2, 3);
    verify_ds("A8b f58888(0,0,2,3) record -1 matches, 0 routes", 0, 0);

    /* f5843C */
    snap(); verify_ds("A9 f5843C(9,3,2,2)", f5843C(9, 3, 2, 2), 1);
    snap(); verify_ds("A9 f5843C(9,3,2,1)", f5843C(9, 3, 2, 1), 0);
    snap(); verify_ds("A9 f5843C(9,3,2,0)", f5843C(9, 3, 2, 0), 0);
    snap(); verify_ds("A9 f5843C(3,12,2,4)", f5843C(3, 12, 2, 4), 0);
    snap(); verify_ds("A9 f5843C(12,3,2,5)", f5843C(12, 3, 2, 5), 1);
    snap(); verify_ds("A9 f5843C(4,9,2,5)", f5843C(4, 9, 2, 5), 0);
    snap(); verify_ds("A9 f5843C(5,5,2,2)", f5843C(5, 5, 2, 2), 1);
    snap(); verify_ds("A9 f5843C(3,9,2,-1)", f5843C(3, 9, 2, -1), 0);
    set_astro(2, 1);
    snap(); verify_ds("A9 f5843C(3,9,2,1) Astronomy", f5843C(3, 9, 2, 1), 0);
    snap(); verify_ds("A9 f5843C(9,3,2,2) Astronomy", f5843C(9, 3, 2, 2), 1);
    snap(); verify_ds("A9 f5843C(3,12,2,6) Astronomy", f5843C(3, 12, 2, 6), 0);
    set_astro(2, 0);
    /* f584B4 / f58568 called directly with a > b: no swap, no record */
    snap(); verify_ds("A9 f584B4(9,3,2)", f584B4(9, 3, 2), 0);
    snap(); verify_ds("A9 f58568(3,9,2)", f58568(3, 9, 2), 1);

    /* f5861C */
    {
        static const int r1[1] = { 9 }, r2[2] = { 9, 12 }, r3[1] = { 3 }, r4[1] = { 5 };
        memset(DP(ROUTE_REACH), 0x11, 0x280);
        snap(); reach_want(r1, 1); f5861C(3, 2, 2); verify_ds("A10 f5861C(3,2,2)", 0, 0);
        snap(); reach_want(r2, 2); f5861C(3, 2, 5); verify_ds("A10 f5861C(3,2,5)", 0, 0);
        snap(); reach_want(r3, 1); f5861C(9, 2, 2); verify_ds("A10 f5861C(9,2,2)", 0, 0);
        snap(); reach_want(r4, 1); f5861C(5, 2, 2); verify_ds("A10 f5861C(5,2,2)", 0, 0);
        snap(); reach_want(NULL, 0); f5861C(3, 2, 1); verify_ds("A10 f5861C(3,2,1)", 0, 0);
        set_astro(2, 1);
        snap(); reach_want(r1, 1); f5861C(3, 2, 2); verify_ds("A10 f5861C(3,2,2) Astronomy", 0, 0);
        snap(); reach_want(NULL, 0); f5861C(12, 2, 6); verify_ds("A10 f5861C(12,2,6) Astronomy", 0, 0);
        set_astro(2, 0);
    }
    /* f58A74 / f58DCC directly: the buffer is not cleared by them (only by redo rounds) */
    {
        memset(DP(ROUTE_BUF), 0x44, 0x15E0);
        setD8(TOK(20, 2), 1);
        snap();
        W8(ROUTE_BUF + 0, 3); W8(ROUTE_BUF + 1, 20); W8(ROUTE_BUF + 2, 9); W32(ROUTE_COUNT, 1);
        f58A74(3, 9, 2, 2);   /* route 0 kept; route 1 (2 intermediates) ends the scan in slot 1: [3,20,(21 blocked),9] */
        W8(ROUTE_BUF + 16, 3); W8(ROUTE_BUF + 17, 20); W8(ROUTE_BUF + 19, 9);
        verify_ds("A11 f58A74(3,9,2,2)", 0, 0);
        clear_tokens();
    }
    /* f58DCC directly (table 2, index 0x107550): [50] blocked (no tokens), counter cleared, redo memsets the buffer and
       keeps [3,50,9]; +6 is not written by it */
    {
        static const int s[1][8] = { { 3, 50, 9, 0xFF, 0xFF, 0xFF, -1, 0 } };
        memset(DP(ROUTE_BUF), 0x44, 0x15E0);
        snap(); want_buf(1, s); W32(ROUTE_COUNT, 1);
        f58DCC(3, 9, 2, 2);
        verify_ds("A12 f58DCC(3,9,2,2)", 0, 0);
    }
    /* The latent stale-bytes oddity (the shipped files never have it: part C): a record 6 -> 7 whose routes are not
       sorted by length, [40,41,42] then [44]; tokens of nation 2 in 40, 42, 44, none in 41 (type 8). Route 0: 41
       blocked, slot 0 = [6,40,FF,42,7], not counted; route 1 overwrites only bytes 0..2: [6,44,7,42,7,FF], counted.
       +6 = 4 - (5 - 1) = 0 (the intended 4 - 2 = 2). Swapped: reversed [FF,7,42,7,44,6], shifted [7,42,7,44,6,FF]. */
    {
        static const uint8_t routes[2][4] = { { 40, 41, 42, 0xFF }, { 44, 0xFF, 0xFF, 0xFF } };
        static const int s[1][8] = { { 6, 44, 7, 42, 7, 0xFF, 0, 0 } };
        static const int s2[1][8] = { { 7, 42, 7, 44, 6, 0xFF, 0, 0 } };
        uint32_t h;
        int t;

        load_heap_free(D32(GAME_RL_BLOCK));
        h = load_heap_alloc(sizeof routes);
        memcpy(load_heap_ptr(h, sizeof routes), routes, sizeof routes);
        setD32(GAME_RL_BLOCK, h);
        memset(DP(GAME_RL_TABLE1), 0, 0x2084);
        setD32(GAME_RL_TABLE1, 1);
        put_rec(GAME_RL_TABLE1, 0, 6, 7, 2, 0);
        for (t = 0; t < 160; t++)
            setD16(GAME_RL_FIRST1 + 2u * (uint32_t)t, 0xFFFF);
        setD16(GAME_RL_FIRST1 + 2u * 6, 0);
        setD8(TOK(40, 2), 1); setD8(TOK(42, 2), 1); setD8(TOK(44, 2), 1);
        snap(); want_buf(1, s); W32(ROUTE_COUNT, 1);
        f58888(6, 7, 2, 4);
        verify_ds("A13 f58888(6,7,2,4) unsorted routes: stale bytes", 0, 0);
        snap(); want_buf(1, s2); W32(ROUTE_COUNT, 1);
        f58888(7, 6, 2, 4);
        verify_ds("A13b f58888(7,6,2,4) unsorted routes, swapped", 0, 0);
        clear_tokens();
    }
}

/* ---- B. the model: a second implementation on plain arrays ------------------------------------------------------- */

typedef struct {
    uint8_t t1[0x2084], t2[0x5644];  /* the two table images as in the file */
    uint8_t *blk;                    /* the route block */
    uint32_t m;
    int first[2][160];
} RL;

static RL g_rl;
static uint8_t g_type[160];
static int8_t g_tok[10][160];
static uint32_t g_ffd40;

static int32_t rd32(const uint8_t *p) { return (int32_t)((uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24); }
static int16_t rd16(const uint8_t *p) { return (int16_t)(p[0] | p[1] << 8); }

/* the 8 bytes of record e of table s (0 = table 1, 1 = table 2); e = -1 reads the 4 bytes in front of the count (for
   table 1 the dword 0xFFD40, for table 2 the last dword of table 1's image) and the count itself */
static void m_rec(int s, int e, uint8_t out[8])
{
    const uint8_t *tab = s ? g_rl.t2 : g_rl.t1;
    if (e >= 0) {
        memcpy(out, tab + 4 + 8 * e, 8);
        return;
    }
    if (s == 0) {
        out[0] = (uint8_t)g_ffd40; out[1] = (uint8_t)(g_ffd40 >> 8);
        out[2] = (uint8_t)(g_ffd40 >> 16); out[3] = (uint8_t)(g_ffd40 >> 24);
    } else
        memcpy(out, g_rl.t1 + 0x2080, 4);
    memcpy(out + 4, tab, 4);
}

static int m_count(int s) { return rd32(s ? g_rl.t2 : g_rl.t1); }

/* the leading intermediates of route r (stop at 0xFF, at most 4) */
static int m_route(uint32_t r, uint8_t u[4])
{
    int n = 0;
    while (n < 4 && g_rl.blk[4 * r + n] != 0xFF) {
        u[n] = g_rl.blk[4 * r + n];
        n++;
    }
    return n;
}

static int m_first_short(int s, int a, int b, int k)
{
    int e;
    uint8_t r[8], u[4];
    for (e = 0; e < m_count(s); e++) {
        m_rec(s, e, r);
        if (r[0] == a && r[1] == b)
            return m_route((uint32_t)rd32(r + 4), u) < k;
    }
    return 0;
}

static int m_5843C(int a, int b, int astro, int k)
{
    if (k == 0)
        return 0;
    return a <= b ? m_first_short(astro, a, b, k) : m_first_short(astro, b, a, k);
}

static void m_5861C(int t, int astro, int k, int32_t reach[160])
{
    int e;
    uint8_t r[8], u[4];
    memset(reach, 0, 160 * sizeof reach[0]);
    for (e = 0; e < m_count(astro); e++) {
        m_rec(astro, e, r);
        if (r[0] != t && r[1] != t)
            continue;
        if (m_route((uint32_t)rd32(r + 4), u) < k)
            reach[r[0] == t ? r[1] : r[0]] = 1;
    }
}

/* one pass over the routes of record rr; mode 0 blocks on (no tokens of nation n, type 8) and counts, mode 1 on the
   counters; buf/cnt as the data object's 0x108820 / [0x109E00] */
static void m_pass(int mode, const uint8_t rr[8], int a, int b, int n, int k, int16_t cntr[160], uint8_t *buf,
                   int32_t *cnt)
{
    int j, x, len, blocked;
    uint8_t u[4];
    for (j = 0; j < rd16(rr + 2); j++) {
        uint8_t *slot = buf + 16 * *cnt;
        len = m_route((uint32_t)rd32(rr + 4) + (uint32_t)j, u);
        slot[0] = (uint8_t)a;
        blocked = 0;
        for (x = 0; x < len; x++) {
            int bl = mode == 0 ? (g_tok[n][u[x]] == 0 && g_type[u[x]] == 8) : cntr[u[x]] != 0;
            if (bl) {
                if (mode == 0)
                    cntr[u[x]]++;
                blocked = 1;
            } else
                slot[1 + x] = u[x];
        }
        slot[1 + len] = (uint8_t)b;
        if (len >= k)
            return;
        if (!blocked)
            (*cnt)++;
    }
}

static void m_paths(int s, int a, int b, int n, int k, uint8_t *buf, int32_t *cnt)
{
    int e, u, bu;
    uint8_t rr[8];
    int16_t cntr[160];

    *cnt = 0;
    memset(cntr, 0, sizeof cntr);
    for (e = g_rl.first[s][a]; e < m_count(s); e++) {
        m_rec(s, e, rr);
        if (rr[0] == a && rr[1] == b)
            break;
    }
    if (e >= m_count(s))
        return;
    m_pass(0, rr, a, b, n, k, cntr, buf, cnt);
    while (*cnt == 0) {
        bu = -1;
        for (u = 0; u < 160; u++)
            if (cntr[u] != 0 && (bu < 0 || cntr[u] < cntr[bu]))
                bu = u;
        if (bu < 0)
            return;
        cntr[bu] = 0;
        memset(buf, 0xFF, 0x15E0);
        m_pass(1, rr, a, b, n, k, cntr, buf, cnt);
    }
}

static void m_58888(int a, int b, int n, int astro, int k, uint8_t *buf, int32_t *cnt)
{
    int sw = 0, r, i, t;
    if (k == 0) {
        *cnt = 0;
        return;
    }
    if (a > b) {
        t = a; a = b; b = t; sw = 1;
    }
    memset(buf, 0xFF, 0x15E0);
    m_paths(astro, a, b, n, k, buf, cnt);
    for (r = 0; r < *cnt; r++) {
        uint8_t *p = buf + 16 * r, q[6];
        if (sw) {
            int lead = 0;
            for (i = 0; i < 6; i++)
                q[i] = p[5 - i];
            while (lead < 6 && q[lead] == 0xFF)
                lead++;
            for (i = 0; i < 6; i++)
                p[i] = i + lead < 6 ? q[i + lead] : 0xFF;
        }
        for (i = 2; i < 6 && p[i] != 0xFF; i++)
            ;
        p[6] = (uint8_t)(k - (i - 1));
    }
}

/* ---- B. loading a real N.RL ------------------------------------------------------------------------------------- */

static int load_rl(const char *gamedir, int players)
{
    char name[512];
    FILE *fp;
    uint8_t hdr[4];
    uint8_t *p;
    int s, t, e;

    snprintf(name, sizeof name, "%s/%d.RL", gamedir, players);
    fp = fopen(name, "rb");
    if (fp == NULL) {
        printf("cannot open %s\n", name);
        return -1;
    }
    free(g_rl.blk);
    if (fread(hdr, 4, 1, fp) != 1 || fread(g_rl.t1, 0x2084, 1, fp) != 1 || fread(g_rl.t2, 0x5644, 1, fp) != 1) {
        fclose(fp);
        return -1;
    }
    g_rl.m = (uint32_t)rd32(hdr);
    g_rl.blk = malloc(g_rl.m);
    if (g_rl.blk == NULL || fread(g_rl.blk, 1, g_rl.m, fp) != g_rl.m) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    for (s = 0; s < 2; s++)
        for (t = 0; t < 160; t++) {
            g_rl.first[s][t] = -1;
            for (e = 0; e < m_count(s); e++)
                if ((s ? g_rl.t2 : g_rl.t1)[4 + 8 * e] == t) {
                    g_rl.first[s][t] = e;
                    break;
                }
        }

    /* into the data object as f3EED8 does (0x3F436..0x3F546) */
    load_heap_free(D32(GAME_RL_BLOCK));
    setD32(GAME_RL_BLOCK, load_heap_alloc(g_rl.m));
    memcpy(DP(GAME_RL_TABLE1), g_rl.t1, 0x2084);
    memcpy(DP(GAME_RL_TABLE2), g_rl.t2, 0x5644);
    p = load_heap_ptr(D32(GAME_RL_BLOCK), g_rl.m);
    if (p == NULL)
        return -1;
    memcpy(p, g_rl.blk, g_rl.m);
    for (t = 0; t < 160; t++) {
        setD16(GAME_RL_FIRST1 + 2u * (uint32_t)t, 0xFFFF);
        setD16(GAME_RL_FIRST2 + 2u * (uint32_t)t, 0xFFFF);
        for (e = 0; e < S32(GAME_RL_TABLE1); e++)
            if (D8(0xFFD48u + 8u * (uint32_t)e) == t) {
                setD16(GAME_RL_FIRST1 + 2u * (uint32_t)t, (uint16_t)e);
                break;
            }
        for (e = 0; e < S32(GAME_RL_TABLE2); e++)
            if (D8(0x101DCCu + 8u * (uint32_t)e) == t) {
                setD16(GAME_RL_FIRST2 + 2u * (uint32_t)t, (uint16_t)e);
                break;
            }
    }
    return 0;
}

/* C. the data facts */
static long g_c_tests, g_c_fail;
static void data_facts(int players)
{
    int s, e, j, i, prev;
    uint8_t r[8], u[4];
    for (s = 0; s < 2; s++)
        for (e = 0; e < m_count(s); e++) {
            m_rec(s, e, r);
            g_c_tests++;
            if (rd16(r + 2) > 323 || rd16(r + 2) < 0 || (uint32_t)(rd32(r + 4) + rd16(r + 2)) * 4u > g_rl.m) {
                g_c_fail++;
                printf("FAIL C %d.RL table %d record %d: %d routes from %d\n", players, s + 1, e, rd16(r + 2), rd32(r + 4));
                continue;
            }
            prev = 0;
            for (j = 0; j < rd16(r + 2); j++) {
                int len = m_route((uint32_t)rd32(r + 4) + (uint32_t)j, u);
                g_c_tests++;
                for (i = 0; i < len; i++)
                    if (u[i] >= 160)
                        break;
                if (len < prev || i < len) {
                    g_c_fail++;
                    printf("FAIL C %d.RL table %d record %d route %d: length %d after %d\n", players, s + 1, e, j, len,
                           prev);
                }
                prev = len;
            }
        }
}

/* ---- B. the table ----------------------------------------------------------------------------------------------- */

static uint32_t g_rng = 12345;
static uint32_t rnd(void) { g_rng = g_rng * 1103515245u + 12345u; return g_rng >> 16; }

static long g_b_tests, g_b_fail, g_sem_tests, g_sem_fail, g_ds_checks;

static void bfail(const char *what, int players, int a, int b, int astro, int lay, int k)
{
    g_b_fail++;
    if (g_b_fail <= 30)
        printf("FAIL B %s: %d.RL a %d b %d astro %d layout %d k %d\n", what, players, a, b, astro, lay, k);
}

/* the data object must equal g_before except [lo, hi) */
static int ds_same_except(uint32_t lo, uint32_t hi)
{
    uint32_t i;
    for (i = 0; i < DS_SIZE; i++)
        if (g_ds[i] != g_before[i] && !(i + DS_BASE >= lo && i + DS_BASE < hi))
            return 0;
    return 1;
}

static void part_b(const char *gamedir)
{
    static uint8_t mbuf[0x15E0];
    int players, lay, astro, a, b, k, n, u, nn, t;
    long calls = 0;
    static const uint32_t ffd40[3] = { 0, 2, 0xFFFFFFCEu };

    for (players = 3; players <= 8; players++) {
        if (load_rl(gamedir, players) != 0) {
            g_b_fail++;
            continue;
        }
        data_facts(players);
        g_ffd40 = ffd40[players % 3];
        setD32(0xFFD40u, g_ffd40);
        for (lay = 0; lay < 3; lay++) {
            for (nn = 1; nn <= 9; nn++)
                for (u = 0; u < 160; u++) {
                    int8_t v = lay == 0 ? 0 : lay == 1 ? 1 : (rnd() % 3 == 0 ? (int8_t)(rnd() % 5 + (rnd() & 1 ? 0x7E : 0)) : 0);
                    g_tok[nn][u] = v;
                    setD8(TOK(u, nn), (uint8_t)v);
                }
            for (astro = 0; astro < 2; astro++) {
                for (a = 0; a < 160; a++) {
                    for (b = 0; b < 160; b++) {
                        n = 1 + (a * 7 + b) % 9;
                        set_astro(n, astro);
                        for (k = -1; k <= 6; k++) {
                            int32_t mcnt, r;
                            uint8_t fill = (uint8_t)rnd();
                            int sample = (++calls & 4095) == 0;

                            /* f58888 */
                            memset(DP(ROUTE_BUF), fill, 0x15E0);
                            memset(mbuf, fill, 0x15E0);
                            setD32(ROUTE_COUNT, 0x12345678u);
                            mcnt = 0x12345678;
                            if (sample)
                                memcpy(g_before, g_ds, DS_SIZE);
                            f58888(a, b, n, k);
                            m_58888(a, b, n, astro, k, mbuf, &mcnt);
                            g_b_tests++;
                            if (S32(ROUTE_COUNT) != mcnt || memcmp(DP(ROUTE_BUF), mbuf, 0x15E0) != 0)
                                bfail("f58888", players, a, b, astro, lay, k);
                            if (sample) {
                                g_ds_checks++;
                                if (!ds_same_except(ROUTE_BUF, ROUTE_COUNT + 4))
                                    bfail("f58888 wrote outside 0x108820..0x109E03", players, a, b, astro, lay, k);
                            }
                            /* semantics of the kept records */
                            for (r = 0; r < S32(ROUTE_COUNT) && r < 350; r++) {
                                const uint8_t *p = DP(ROUTE_BUF + 16u * (uint32_t)r);
                                int len = 0, ok;
                                while (len < 6 && p[len] != 0xFF)
                                    len++;
                                ok = len >= 2 && p[0] == a && p[len - 1] == b && p[6] == (uint8_t)(k - (len - 1));
                                g_sem_tests++;
                                if (!ok || len - 2 >= k) {
                                    g_sem_fail++;
                                    if (g_sem_fail <= 10)
                                        printf("FAIL semantic %d.RL a %d b %d k %d record %d\n", players, a, b, k, r);
                                }
                            }

                            /* f5843C */
                            if (sample)
                                memcpy(g_before, g_ds, DS_SIZE);
                            g_b_tests++;
                            if (f5843C(a, b, n, k) != m_5843C(a, b, astro, k))
                                bfail("f5843C", players, a, b, astro, lay, k);
                            if (sample) {
                                g_ds_checks++;
                                if (!ds_same_except(0, 0))
                                    bfail("f5843C wrote to the data object", players, a, b, astro, lay, k);
                            }
                        }
                    }
                }
                /* f5861C, every t */
                for (t = 0; t < 160; t++)
                    for (k = -1; k <= 6; k++) {
                        int32_t reach[160];
                        int i, same = 1, whole = (t * 8 + k) % 61 == 0;
                        n = 1 + t % 9;
                        set_astro(n, astro);
                        memset(DP(ROUTE_REACH), 0x77, 0x280);
                        if (whole)
                            memcpy(g_before, g_ds, DS_SIZE);
                        f5861C(t, n, k);
                        m_5861C(t, astro, k, reach);
                        for (i = 0; i < 160; i++)
                            if (S32(ROUTE_REACH + 4u * (uint32_t)i) != reach[i])
                                same = 0;
                        g_b_tests++;
                        if (!same)
                            bfail("f5861C", players, t, -1, astro, lay, k);
                        if (whole)
                            g_ds_checks++;
                        if (whole && !ds_same_except(ROUTE_REACH, ROUTE_REACH + 0x280))
                            bfail("f5861C wrote outside 0x107EE4", players, t, -1, astro, lay, k);
                    }
            }
        }
        printf("  %d.RL done (%ld checks so far, %ld fail)\n", players, g_b_tests, g_b_fail);
    }
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:/work/advciv_run";
    int u;

    if (state_init(gamedir) != 0)
        return 1;
    g_before = malloc(DS_SIZE);
    g_want = malloc(DS_SIZE);
    if (g_before == NULL || g_want == NULL)
        return 1;
    for (u = 0; u < 160; u++)
        g_type[u] = D8(0xC80DCu + (uint32_t)u);

    part_a();
    printf("A. hand vectors: %ld of %ld pass\n", g_tests - g_fail, g_tests);

    part_b(gamedir);
    printf("B. table over 3.RL..8.RL: %ld of %ld checks agree with the model (%ld whole-object checks); semantic "
           "checks of kept records: %ld of %ld pass\n",
           g_b_tests - g_b_fail, g_b_tests, g_ds_checks, g_sem_tests - g_sem_fail, g_sem_tests);
    printf("C. data facts: %ld of %ld pass\n", g_c_tests - g_c_fail, g_c_tests);

    return g_fail == 0 && g_b_fail == 0 && g_sem_fail == 0 && g_c_fail == 0 ? 0 : 1;
}
