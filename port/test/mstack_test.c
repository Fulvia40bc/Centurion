/* mstack_test [GAMEDIR]: unit vectors for module STK (mstack.c) that need no game run.

   It loads the data object from GAMEDIR/AC_MAIN.EXE as the port does (state_init: le_image.c; GAMEDIR defaults to the
   run copy C:\work\advciv_run, only read). stack.c is linked for the World State Stack's allocate, push, free and pop
   (0x7A8D0, 0x7A940, 0x7ABD0, 0x7AC60), so the round trips run through both files. Expectations are written here from
   the disassembly's semantics, independently of mstack.c:
     A. hand vectors: every byte range each routine should change is listed (address, size, source) and the whole data
        object must equal the image taken before plus exactly those bytes; the slot fields, depths and high-water
        marks by value; the odd indexing of 0x7AD7C and the unchecked peeks at depth -1 with sentinels around them;
        the overflow and underflow messages (both texts: fixed and g_original) with f44CAC stubbed to record the call
        and longjmp back; push/pop and snapshot/restore round trips.
     B. a random differential run: a second implementation of both stacks and the snapshots on a plain copy of the
        data object and plain slot arrays (written from the listing, not from mstack.c or stack.c) and the port apply
        the same random operations (and random edits of the copied ranges, their neighbours and the counts); after
        every operation the two images must agree byte for byte outside the 44 heap-handle fields, each handle field
        must be 0 exactly when the model's is, every live block must hold exactly the model's bytes (exact size), the
        number of live heap blocks must equal the number of non-zero handle fields (no leak, no double free), and the
        quits must match in number and message.
   The UI routines stack.c calls are stubs that record a call trace; none may be called by the routines tested.
   Prints each failure, the counts per part, and exits 0 when all pass.

   Link: mstack_test.c mstack.c stack.c load.c state.c le_image.c. */

#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/state.h"
#include "../src/game.h"
#include "../src/fg.h"
#include "../src/map.h"
#include "../src/mapdata.h"
#include "../src/panel.h"
#include "../src/save.h"
#include "../src/stack.h"
#include "../src/ui.h"
#include "../src/mstack.h"

/* ---- call trace, the quit stub and the UI stubs ---------------------------------------------------------------- */

static char g_trace[4096];
static jmp_buf g_jb;
static int g_quits;
static char g_quitmsg[600];

static void tr(const char *fmt, ...)
{
    size_t len = strlen(g_trace);
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(g_trace + len, sizeof g_trace - len, fmt, ap);
    va_end(ap);
}

_Noreturn void f44CAC(void)
{
    g_quits++;
    snprintf(g_quitmsg, sizeof g_quitmsg, "%s", DSTR(0xF5500u));
    tr("44CAC ");
    longjmp(g_jb, 1);
}

void f3A990(void) { tr("3A990 "); }
void f3AB7C(const char *text) { tr("3AB7C(%s) ", text); }
void f3AF54(uint32_t spr) { tr("3AF54(%X) ", (unsigned)spr); }
void f3C284(uint32_t dlg, int id, uint32_t label) { tr("3C284(%X,%X,%X) ", (unsigned)dlg, id, (unsigned)label); }
void f3C314(uint32_t dlg, int id) { tr("3C314(%X,%X) ", (unsigned)dlg, id); }
void f3C7A0(uint32_t dlg) { tr("3C7A0(%X) ", (unsigned)dlg); }
void f3C828(uint32_t dlg) { tr("3C828(%X) ", (unsigned)dlg); }
void f3CBBC(void) { tr("3CBBC "); }
void f3CC68(void) { tr("3CC68 "); }
void f3CCBC(void) { tr("3CCBC "); }
void f3D52C(int dx, int dy) { tr("3D52C(%d,%d) ", dx, dy); }
void f3F558(void) { tr("3F558 "); }
void f5E7D0(void) { tr("5E7D0 "); }
void f3A780(const char *msg) { tr("3A780(%s) ", msg); }
int f5FF28(void) { tr("5FF28 "); return -15; }
void f78884(void) { tr("78884 "); }
void f788AC(void) { tr("788AC "); }
int f78BA4(uint32_t dlg, int map) { tr("78BA4(%X,%d) ", (unsigned)dlg, map); return -1; }
int fg_vbopen(int handle) { tr("vbopen(%d) ", handle); return 0; }
void fg_vbclose(void) { tr("vbclose "); }
int fg_getpixel(int x, int y) { tr("getpixel(%d,%d) ", x, y); return 0; }
int fg_point(int x, int y) { tr("point(%d,%d) ", x, y); return 0; }
int fg_setcolor(int color) { tr("setcolor(%d) ", color); return 0; }
int fg_getcolor(void) { tr("getcolor "); return 0; }
int fg_getxpos(void) { tr("getxpos "); return 0; }
int fg_getypos(void) { tr("getypos "); return 0; }
void fg_move(int x, int y) { tr("move(%d,%d) ", x, y); }
_Noreturn void port_longjmp(uint32_t jmpbuf) { tr("longjmp(%X) ", (unsigned)jmpbuf); longjmp(g_jb, 2); }

/* run fn; 1 if it quit through f44CAC (the stub jumps back here) */
static int run(void (*fn)(void))
{
    volatile int quit = 0;

    if (setjmp(g_jb) == 0)
        fn();
    else
        quit = 1;
    return quit;
}

/* ---- addresses, read in the listing (written out here, not taken from mstack.c) ------------------------------- */

#define WS_DEPTH 0xCAC54u
#define WS_HIGH  0x112A8Cu
#define WS_AREA  0xF5240u
#define WS_NAT   0xF5268u
#define WS_TST   0xF5290u
#define ML_DEPTH 0xCAC58u
#define ML_HIGH  0x112A90u
#define ML_P1    0x112A1Cu
#define ML_P2    0x112A38u
#define ML_C1    0x112A54u
#define ML_C2    0x112A70u
#define L1       0x107694u
#define L1N      0x1079B4u
#define L2       0x1079B8u
#define L2N      0x107D3Cu
#define AREAS    0xE31FDu
#define TST      0xE2C18u
#define NATS     0xF1593u
#define N_AREAS  0x2B20u
#define N_TST    0x1E0u
#define N_NATS   0x88Du
#define MSGBUF   0xF5500u

#define TXT_WS_BLOWN  "World State Stack has blown its max size. > 10"
#define TXT_ML_BLOWN  "Move List Stack has blown its max size. > 7"
#define TXT_WS_POP    "World State Stack has popped once too often."
#define TXT_WS_POP_O  "World State Stack has popped once to often."
#define TXT_ML_POP    "Move Stack has popped once too often."
#define TXT_ML_POP_O  "Move Stack has popped once to often."

static int g_tests, g_fail;

static void check(int ok, const char *what)
{
    g_tests++;
    if (!ok) {
        g_fail++;
        printf("FAIL: %s\n", what);
    }
}

static int is_handle_field(uint32_t a)
{
    return (a >= WS_AREA && a < WS_TST + 40u) || (a >= ML_P1 && a < ML_C1);
}

static uint32_t live_blocks(void)
{
    uint32_t i, n = 0;

    for (i = 1; i <= 1024; i++)
        if (load_heap_ptr(0x80000000u | i << 20, 1) != NULL)
            n++;
    return n;
}

/* ---- A. hand vectors ------------------------------------------------------------------------------------------- */

static uint8_t *g_want;   /* the expected data object */

static uint8_t *W(uint32_t a) { return g_want + (a - DS_BASE); }
static void wset32(uint32_t a, uint32_t v) { uint8_t *p = W(a); p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }
static void wcopy(uint32_t dst, uint32_t src, uint32_t n) { memmove(W(dst), W(src), n); }
static void wmsg(const char *t) { memcpy(W(MSGBUF), t, strlen(t) + 1); }

static void want_now(void) { memcpy(g_want, g_ds, DS_SIZE); }

/* the whole data object equals g_want, the handle fields compared only where mask says so */
static int image_ok(int handles_too)
{
    uint32_t a;

    if (handles_too)
        return memcmp(g_ds, g_want, DS_SIZE) == 0;
    for (a = DS_BASE; a < DS_BASE + DS_SIZE; a++)
        if (!is_handle_field(a & ~3u) && g_ds[a - DS_BASE] != g_want[a - DS_BASE])
            return 0;
    return 1;
}

static void fill(uint32_t a, uint32_t n, uint32_t seed)
{
    uint32_t i;

    for (i = 0; i < n; i++) {
        seed = seed * 1103515245u + 12345u;
        setD8(a + i, (uint8_t)(seed >> 16));
    }
}

/* the neighbourhood of every range the routines touch, refilled */
static void fill_all(uint32_t seed)
{
    fill(AREAS - 8, N_AREAS + 16, seed + 1);
    fill(TST - 8, N_TST + 16, seed + 2);
    fill(NATS - 8, N_NATS + 16, seed + 3);
    fill(L1 - 8, 800, seed + 4);          /* up to the count */
    fill(L2 - 4, 904, seed + 5);
    fill(0x117CC0u - 8, N_AREAS + 16, seed + 6);
    fill(0x117AE0u - 8, N_TST + 8, seed + 7);   /* 0x117AE0 + 0x1E0 = 0x117CC0 */
    fill(0x116518u - 8, N_NATS + 16, seed + 8);
    fill(0x116FFCu - 8, N_NATS + 16, seed + 9);
    fill(0x11A828u - 8, 808, seed + 10);
    fill(0x11AB4Cu, 900, seed + 11);
}

static void part_a(void)
{
    static uint8_t buf[N_AREAS], ws_a[10][N_AREAS], ws_t[10][N_TST], ws_n[10][N_NATS];
    uint32_t h, h1, h2, i, k, live0;
    int q;

    g_original = 0;
    live0 = live_blocks();

    /* A1. the fixed snapshots: exactly their ranges */
    fill_all(100);
    want_now();
    wcopy(0x117CC0u, AREAS, N_AREAS);
    wcopy(0x117AE0u, TST, N_TST);
    wcopy(0x116518u, NATS, N_NATS);
    q = run(f7A688);
    check(!q && image_ok(1), "A1 0x7A688 world snapshot ranges");

    fill_all(200);
    want_now();
    wcopy(AREAS, 0x117CC0u, N_AREAS);
    wcopy(TST, 0x117AE0u, N_TST);
    wcopy(NATS, 0x116518u, N_NATS);
    q = run(f7A700);
    check(!q && image_ok(1), "A1 0x7A700 world restore ranges");

    fill_all(300);
    want_now();
    wcopy(0x116FFCu, NATS, N_NATS);
    q = run(f7A778);
    check(!q && image_ok(1), "A1 0x7A778 nations snapshot range");

    fill_all(400);
    want_now();
    wcopy(NATS, 0x116FFCu, N_NATS);
    q = run(f7A7B0);
    check(!q && image_ok(1), "A1 0x7A7B0 nations restore range");

    /* A2. the move-list snapshot with counts 0, 1, full, and odd sizes */
    {
        static const int32_t c1s[] = { 0, 1, 200, 37, 5 }, c2s[] = { 0, 150, 1, 3, 149 };
        for (k = 0; k < 5; k++) {
            fill_all(500 + k);
            setD32(L1N, (uint32_t)c1s[k]);
            setD32(L2N, (uint32_t)c2s[k]);
            want_now();
            wset32(0x11AB48u, (uint32_t)c1s[k]);
            wcopy(0x11A828u, L1, 4u * (uint32_t)c1s[k]);
            wset32(0x11AED0u, (uint32_t)c2s[k]);
            wcopy(0x11AB4Cu, L2, 6u * (uint32_t)c2s[k]);
            q = run(f7A7E8);
            check(!q && image_ok(1), "A2 0x7A7E8 move-list snapshot");

            fill_all(600 + k);
            setD32(0x11AB48u, (uint32_t)c1s[k]);
            setD32(0x11AED0u, (uint32_t)c2s[k]);
            want_now();
            wset32(L1N, (uint32_t)c1s[k]);
            wcopy(L1, 0x11A828u, 4u * (uint32_t)c1s[k]);
            wset32(L2N, (uint32_t)c2s[k]);
            wcopy(L2, 0x11AB4Cu, 6u * (uint32_t)c2s[k]);
            q = run(f7A85C);
            check(!q && image_ok(1), "A2 0x7A85C move-list restore");
        }
    }
    /* A2'. a negative count: stored, then the port guard quits (the original would copy 4 GB) */
    fill_all(700);
    setD32(L1N, 0xFFFFFFFFu);
    want_now();
    wset32(0x11AB48u, 0xFFFFFFFFu);
    wmsg("Stack copy of 0xFFFFFFFC bytes at 0x107694 is outside the data object");
    g_quits = 0;
    q = run(f7A7E8);
    check(q && g_quits == 1 && image_ok(1), "A2' 0x7A7E8 count -1: count stored, guard quit");

    /* A3. Move List Stack push: depth, high-water mark, slot fields, exact blocks */
    setD32(ML_DEPTH, 0xFFFFFFFFu);
    setD32(ML_HIGH, 0);
    for (i = 0; i < 7; i++) {
        setD32(ML_P1 + 4 * i, 0);
        setD32(ML_P2 + 4 * i, 0);
        setD32(ML_C1 + 4 * i, 0);
        setD32(ML_C2 + 4 * i, 0);
    }
    fill_all(800);
    setD32(L1N, 3);
    setD32(L2N, 2);
    want_now();
    q = run(f7AA1C);
    h1 = D32(ML_P1);
    h2 = D32(ML_P2);
    wset32(ML_DEPTH, 0);
    wset32(ML_C1, 3);
    wset32(ML_C2, 2);
    wset32(ML_P1, h1);
    wset32(ML_P2, h2);
    check(!q && image_ok(1), "A3 push 1: depth 0, high-water mark stays 0 (0 > 0 false), counts, handles");
    check(load_heap_ptr(h1, 12) != NULL && load_heap_ptr(h1, 13) == NULL && memcmp(load_heap_ptr(h1, 12), DP(L1), 12) == 0,
          "A3 push 1: list 1 block exactly 12 bytes, list 1's bytes");
    check(load_heap_ptr(h2, 12) != NULL && load_heap_ptr(h2, 13) == NULL && memcmp(load_heap_ptr(h2, 12), DP(L2), 12) == 0,
          "A3 push 1: list 2 block exactly 12 bytes (6 * 2), list 2's bytes");

    /* push with count 1 = 0 and count 2 = -5: no blocks, handle fields not written (sentinels stay) */
    setD32(ML_P1 + 4, 0x12345678u);
    setD32(ML_P2 + 4, 0x9ABCDEF0u);
    setD32(L1N, 0);
    setD32(L2N, 0xFFFFFFFBu);
    want_now();
    wset32(ML_DEPTH, 1);
    wset32(ML_HIGH, 1);
    wset32(ML_C1 + 4, 0);
    wset32(ML_C2 + 4, 0xFFFFFFFBu);
    q = run(f7AA1C);
    check(!q && image_ok(1), "A3 push 2: counts 0 / -5 stored, handle fields untouched, high-water 1");
    setD32(ML_P1 + 4, 0);
    setD32(ML_P2 + 4, 0);

    /* pop of slot 1: counts back, nothing copied, depth 0; slot counts stay */
    fill(L1, 16, 801);
    want_now();
    wset32(L1N, 0);
    wset32(L2N, 0xFFFFFFFBu);
    wset32(ML_DEPTH, 0);
    q = run(f7AE14);
    check(!q && image_ok(1), "A3 pop of a slot with counts 0 / -5");

    /* peek of slot 0, then pop: list bytes back; the pop frees and zeroes the handles, keeps the counts */
    fill(L1, 800, 802);
    fill(L2, 900, 803);
    setD32(L1N, 77);
    setD32(L2N, 66);
    want_now();
    wset32(L1N, 3);
    wset32(L2N, 2);
    memcpy(W(L1), load_heap_ptr(h1, 12), 12);
    memcpy(W(L2), load_heap_ptr(h2, 12), 12);
    q = run(f7AFB8);
    check(!q && image_ok(1), "A3 peek of slot 0: 12 + 12 bytes and the counts, nothing else");
    check(load_heap_ptr(h1, 12) != NULL && load_heap_ptr(h2, 12) != NULL, "A3 peek keeps the blocks");
    fill(L1, 800, 804);
    fill(L2, 900, 805);
    want_now();
    wset32(L1N, 3);
    wset32(L2N, 2);
    memcpy(W(L1), load_heap_ptr(h1, 12), 12);
    memcpy(W(L2), load_heap_ptr(h2, 12), 12);
    wset32(ML_P1, 0);
    wset32(ML_P2, 0);
    wset32(ML_DEPTH, 0xFFFFFFFFu);
    q = run(f7AE14);
    check(!q && image_ok(1), "A3 pop of slot 0: lists back, handles 0, counts kept in the slot, depth -1");
    check(load_heap_ptr(h1, 1) == NULL && load_heap_ptr(h2, 1) == NULL && live_blocks() == live0, "A3 pop frees both blocks");

    /* A4. overflow at depth 6: message, nothing else */
    for (q = 0; q < 2; q++) {
        g_original = q;
        setD32(ML_DEPTH, 6);
        want_now();
        wmsg(TXT_ML_BLOWN);
        g_quits = 0;
        check(run(f7AA1C) == 1 && g_quits == 1 && strcmp(g_quitmsg, TXT_ML_BLOWN) == 0 && image_ok(1),
              "A4 Move List Stack push at depth 6: '> 7' and quit, depth and high-water unchanged");
    }
    /* push at depth 5 is the last that fits (slot 6) */
    g_original = 0;
    setD32(ML_DEPTH, 5);
    setD32(ML_HIGH, 2);
    setD32(L1N, 1);
    setD32(L2N, 0);
    q = run(f7AA1C);
    h = D32(ML_P1 + 24);
    check(!q && S32(ML_DEPTH) == 6 && S32(ML_HIGH) == 6 && D32(ML_C1 + 24) == 1 && load_heap_ptr(h, 4) != NULL,
          "A4 push at depth 5 fills slot 6, high-water 6");
    /* drop of slot 6: both handles and counts zeroed (0x112A38[6] and 0x112A70[6] through the d-1 indexing) */
    setD32(ML_P2 + 24, 0);
    setD32(ML_C2 + 24, 0x55u);
    setD32(0x112A8Cu, 0x11111111u);     /* next field after 0x112A70[6] */
    want_now();
    wset32(ML_P1 + 24, 0);
    wset32(ML_C1 + 24, 0);
    wset32(ML_P2 + 24, 0);
    wset32(ML_C2 + 24, 0);
    wset32(ML_DEPTH, 5);
    q = run(f7AD7C);
    check(!q && image_ok(1) && load_heap_ptr(h, 1) == NULL, "A4 drop of slot 6: four fields zeroed, block freed, depth 5");
    setD32(0x112A8Cu, 0);

    /* drop of slot 0 with sentinels around: 0x112A38[0] / 0x112A70[0] zeroed via [4*(-1) + 0x112A3C] / [+ 0x112A74] */
    setD32(ML_DEPTH, 0);
    h1 = load_heap_alloc(8);
    h2 = load_heap_alloc(6);
    setD32(ML_P1, h1);
    setD32(ML_P2, h2);
    setD32(ML_C1, 2);
    setD32(ML_C2, 1);
    setD32(ML_P1 + 24, 0xAAAAAAAAu);     /* 0x112A34, just below 0x112A38 */
    setD32(ML_C1 + 24, 0xBBBBBBBBu);     /* 0x112A6C, just below 0x112A70 */
    want_now();
    wset32(ML_P1, 0);
    wset32(ML_P2, 0);
    wset32(ML_C1, 0);
    wset32(ML_C2, 0);
    wset32(ML_DEPTH, 0xFFFFFFFFu);
    q = run(f7AD7C);
    check(!q && image_ok(1) && load_heap_ptr(h1, 1) == NULL && load_heap_ptr(h2, 1) == NULL,
          "A4 drop of slot 0: 0x112A1C/38/54/70 zeroed, 0x112A34 and 0x112A6C untouched, blocks freed");

    /* A5. Move List Stack peek at depth -1: count 1 from 0x112A50, count 2 from 0x112A6C, list 2 from 0x112A34's block */
    h = load_heap_alloc(12);
    for (i = 0; i < 12; i++)
        load_heap_ptr(h, 12)[i] = (uint8_t)(0xC0 + i);
    setD32(0x112A50u, 0);                /* 0x112A38[6] read as count 1 */
    setD32(0x112A6Cu, 2);                /* 0x112A54[6] read as count 2 */
    setD32(0x112A34u, h);                /* 0x112A1C[6] read as list 2's handle */
    setD32(ML_DEPTH, 0xFFFFFFFFu);
    fill(L2, 900, 900);
    want_now();
    wset32(L1N, 0);
    wset32(L2N, 2);
    memcpy(W(L2), load_heap_ptr(h, 12), 12);
    q = run(f7AFB8);
    check(!q && image_ok(1), "A5 Move List Stack peek at depth -1 reads the fields below each array");
    load_heap_free(h);
    setD32(0x112A34u, 0);
    setD32(0x112A6Cu, 0);

    /* A5'. (reviewer) a slot count > 0 with no block behind it: the count is stored first (0x7AE40 / 0x7AFD6), then
       the port guard quits on the handle field; depth, handles and lists unchanged */
    for (k = 0; k < 2; k++) {
        setD32(ML_DEPTH, 0);
        setD32(ML_P1, 0);
        setD32(ML_P2, 0);
        setD32(ML_C1, 2);
        setD32(ML_C2, 0);
        setD32(L1N, 9);
        fill(L1, 800, 950 + k);
        want_now();
        wset32(L1N, 2);
        wmsg("Stack slot 0x112A1C has no block of 0x8 bytes");
        g_quits = 0;
        q = run(k ? f7AFB8 : f7AE14);
        check(q && g_quits == 1 && image_ok(1), k ? "A5' 0x7AFB8 count 2 without a block: count stored, guard quit"
                                                  : "A5' 0x7AE14 count 2 without a block: count stored, guard quit");
    }
    /* list 2 alone: count 1 is 0, count 2 is 1 with no block: L1N 0, L2N 1, then the guard on 0x112A38 */
    setD32(ML_C1, 0);
    setD32(ML_C2, 1);
    want_now();
    wset32(L1N, 0);
    wset32(L2N, 1);
    wmsg("Stack slot 0x112A38 has no block of 0x6 bytes");
    g_quits = 0;
    q = run(f7AE14);
    check(q && g_quits == 1 && image_ok(1), "A5' 0x7AE14 list 2 count 1 without a block: guard quit on 0x112A38");
    setD32(ML_C2, 0);
    /* the high-water mark is never lowered: depth -1, mark 5, push -> depth 0, mark 5 */
    setD32(ML_DEPTH, 0xFFFFFFFFu);
    setD32(ML_HIGH, 5);
    setD32(L1N, 0);
    setD32(L2N, 1);
    fill(L2, 900, 960);
    want_now();
    q = run(f7AA1C);
    h = D32(ML_P2);
    wset32(ML_DEPTH, 0);
    wset32(ML_C1, 0);
    wset32(ML_C2, 1);
    wset32(ML_P2, h);
    check(!q && image_ok(1) && load_heap_ptr(h, 6) != NULL && load_heap_ptr(h, 7) == NULL
              && memcmp(load_heap_ptr(h, 6), DP(L2), 6) == 0,
          "A5' push below the high-water mark: mark stays 5, a 6-byte list 2 block only");
    /* its drop: handle and counts zeroed, the block freed */
    want_now();
    wset32(ML_P2, 0);
    wset32(ML_C2, 0);
    wset32(ML_DEPTH, 0xFFFFFFFFu);
    q = run(f7AD7C);
    check(!q && image_ok(1) && load_heap_ptr(h, 1) == NULL && live_blocks() == live0, "A5' drop of that slot");
    setD32(ML_HIGH, 0);

    /* A6. underflow messages, both texts, depth unchanged */
    for (k = 0; k < 2; k++) {
        static void (*const fns[3])(void) = { f7AD24, f7AD7C, f7AE14 };
        static const char *const fixed[3] = { TXT_WS_POP, TXT_ML_POP, TXT_ML_POP };
        static const char *const orig[3] = { TXT_WS_POP_O, TXT_ML_POP_O, TXT_ML_POP_O };
        g_original = (int)k;
        for (i = 0; i < 3; i++) {
            const char *t = k ? orig[i] : fixed[i];
            setD32(i == 0 ? WS_DEPTH : ML_DEPTH, 0xFFFFFFFEu);    /* -2 */
            memset(DP(MSGBUF), 'x', 0x1F4);
            want_now();
            wmsg(t);
            g_quits = 0;
            g_quitmsg[0] = 0;
            q = run(fns[i]);
            check(q && g_quits == 1 && strcmp(g_quitmsg, t) == 0 && image_ok(1),
                  i == 0 ? "A6 0x7AD24 underflow" : i == 1 ? "A6 0x7AD7C underflow" : "A6 0x7AE14 underflow");
        }
    }
    g_original = 0;

    /* A7. World State Stack drop and peek, round trips with stack.c's allocate / push / pop */
    setD32(WS_DEPTH, 0xFFFFFFFFu);
    setD32(WS_HIGH, 0);
    q = run(f7A8D0);
    check(!q && D32(WS_AREA) != 0 && live_blocks() == live0 + 30, "A7 allocate (stack.c): 30 blocks");
    for (k = 0; k < 10; k++) {
        fill_all(1000 + k);
        memcpy(ws_a[k], DP(AREAS), N_AREAS);
        memcpy(ws_t[k], DP(TST), N_TST);
        memcpy(ws_n[k], DP(NATS), N_NATS);
        q = run(f7A940);
        check(!q && S32(WS_DEPTH) == (int32_t)k, "A7 push (stack.c)");
    }
    want_now();
    wmsg(TXT_WS_BLOWN);
    g_quits = 0;
    q = run(f7A940);
    check(q && g_quits == 1 && strcmp(g_quitmsg, TXT_WS_BLOWN) == 0 && image_ok(1), "A7 the 11th push: '> 10' and quit");
    for (k = 10; k-- > 0;) {
        fill_all(2000 + k);
        want_now();
        memcpy(W(AREAS), ws_a[k], N_AREAS);
        memcpy(W(TST), ws_t[k], N_TST);
        memcpy(W(NATS), ws_n[k], N_NATS);
        q = run(f7AF28);
        check(!q && image_ok(1), "A7 peek of slot k: the three ranges of push k, nothing else, depth unchanged");
        fill_all(3000 + k);
        want_now();
        if (k % 2) {
            wset32(WS_DEPTH, k - 1);
            q = run(f7AD24);
            check(!q && image_ok(1), "A7 drop: depth - 1 only");
        } else {
            memcpy(W(AREAS), ws_a[k], N_AREAS);
            memcpy(W(TST), ws_t[k], N_TST);
            memcpy(W(NATS), ws_n[k], N_NATS);
            wset32(WS_DEPTH, k - 1);
            q = run(f7AC60);
            check(!q && image_ok(1), "A7 pop (stack.c): the ranges of push k, depth - 1");
        }
    }
    /* peek at depth -1: [0xF523C] (0 in the data object) does not resolve: the guard quits */
    want_now();
    wmsg("Stack slot 0xF523C has no block of 0x2B20 bytes");
    q = run(f7AF28);
    check(q && image_ok(1), "A7 peek at depth -1 with [0xF523C] = 0: guard quit");
    /* with a block there: areas from it, territory state from 0xF528C (nation slot 9), nations from 0xF5264 (area slot 9) */
    h = load_heap_alloc(N_AREAS);
    for (i = 0; i < N_AREAS; i++)
        load_heap_ptr(h, N_AREAS)[i] = (uint8_t)(i * 13 + 5);
    setD32(0xF523Cu, h);
    want_now();
    memcpy(W(AREAS), load_heap_ptr(h, N_AREAS), N_AREAS);
    memcpy(W(TST), load_heap_ptr(D32(WS_NAT + 36), N_TST), N_TST);
    memcpy(W(NATS), load_heap_ptr(D32(WS_AREA + 36), N_NATS), N_NATS);
    q = run(f7AF28);
    check(!q && image_ok(1), "A7 peek at depth -1 reads 0xF523C, 0xF528C, 0xF5264");
    load_heap_free(h);
    setD32(0xF523Cu, 0);
    q = run(f7ABD0);
    check(!q && live_blocks() == live0 && S32(WS_DEPTH) == -1, "A7 free (stack.c): no block left");

    /* A8. Move List Stack round trip: seven pushes of different lists, seven pops in reverse */
    {
        static uint8_t keep1[7][800], keep2[7][900];
        static int32_t kc1[7], kc2[7];
        setD32(ML_DEPTH, 0xFFFFFFFFu);
        setD32(ML_HIGH, 0);
        for (i = 0; i < 7; i++) {
            fill(L1, 800, 4000 + i);
            fill(L2, 900, 5000 + i);
            kc1[i] = (int32_t)((i * 37) % 201);
            kc2[i] = (int32_t)((i * 53 + 1) % 151);
            setD32(L1N, (uint32_t)kc1[i]);
            setD32(L2N, (uint32_t)kc2[i]);
            memcpy(keep1[i], DP(L1), 800);
            memcpy(keep2[i], DP(L2), 900);
            q = run(f7AA1C);
            check(!q, "A8 push");
        }
        check(S32(ML_DEPTH) == 6 && S32(ML_HIGH) == 6, "A8 depth 6, high-water 6 after seven pushes");
        check(live_blocks() == live0 + 13u, "A8 thirteen blocks (slot 0's count 1 is 0)");
        for (i = 7; i-- > 0;) {
            fill(L1, 800, 6000 + i);
            fill(L2, 900, 7000 + i);
            want_now();
            wset32(L1N, (uint32_t)kc1[i]);
            wset32(L2N, (uint32_t)kc2[i]);
            memcpy(W(L1), keep1[i], 4u * (uint32_t)kc1[i]);
            memcpy(W(L2), keep2[i], 6u * (uint32_t)kc2[i]);
            wset32(ML_P1 + 4 * i, 0);
            wset32(ML_P2 + 4 * i, 0);
            wset32(ML_DEPTH, i - 1);
            q = run(f7AE14);
            check(!q && image_ok(1), "A8 pop i: list i's counted bytes back, the rest of the lists as they were");
        }
        check(live_blocks() == live0, "A8 every block freed");
    }

    /* A9. snapshot / restore round trips */
    fill_all(8000);
    memcpy(buf, DP(AREAS), N_AREAS);
    {
        static uint8_t t0[N_TST], n0[N_NATS], l1[800], l2[900];
        int32_t c1 = 123, c2 = 77;
        memcpy(t0, DP(TST), N_TST);
        memcpy(n0, DP(NATS), N_NATS);
        setD32(L1N, (uint32_t)c1);
        setD32(L2N, (uint32_t)c2);
        memcpy(l1, DP(L1), 800);
        memcpy(l2, DP(L2), 900);
        run(f7A688);
        run(f7A7E8);
        fill(AREAS, N_AREAS, 8001);
        fill(TST, N_TST, 8002);
        fill(NATS, N_NATS, 8003);
        fill(L1, 800, 8004);
        fill(L2, 900, 8005);
        setD32(L1N, 9);
        setD32(L2N, 9);
        run(f7A700);
        run(f7A85C);
        check(memcmp(DP(AREAS), buf, N_AREAS) == 0 && memcmp(DP(TST), t0, N_TST) == 0 && memcmp(DP(NATS), n0, N_NATS) == 0,
              "A9 world snapshot round trip");
        check(S32(L1N) == c1 && S32(L2N) == c2 && memcmp(DP(L1), l1, 4u * (uint32_t)c1) == 0
                  && memcmp(DP(L2), l2, 6u * (uint32_t)c2) == 0,
              "A9 move-list snapshot round trip (the counted entries)");
        run(f7A778);
        fill(NATS, N_NATS, 8006);
        run(f7A7B0);
        check(memcmp(DP(NATS), n0, N_NATS) == 0, "A9 nations snapshot round trip");
    }

    {
        const char *t = g_trace;
        while (strncmp(t, "44CAC ", 6) == 0)
            t += 6;
        check(*t == 0, "A no UI routine called (the trace holds only 44CAC)");
    }
}

/* ---- B. random differential run against a model on plain arrays ----------------------------------------------- */

static uint8_t *M;                         /* the model's data object */
static uint8_t mw_a[10][N_AREAS], mw_t[10][N_TST], mw_n[10][N_NATS];
static uint8_t mm1[7][800], mm2[7][900];
static uint32_t mm1n[7], mm2n[7];          /* block sizes (0 = no block) */
static char m_quit[600];
static int m_quits;

static uint8_t *MI(uint32_t a) { return M + (a - DS_BASE); }
static uint32_t m32(uint32_t a) { const uint8_t *p = MI(a); return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }
static void mset32(uint32_t a, uint32_t v) { uint8_t *p = MI(a); p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }
static void mquit(const char *t) { memcpy(MI(MSGBUF), t, strlen(t) + 1); snprintf(m_quit, sizeof m_quit, "%s", t); m_quits++; }
static int in_ds(uint32_t a, uint32_t n) { return (uint64_t)a >= DS_BASE && (uint64_t)a + n <= (uint64_t)DS_BASE + DS_SIZE; }

/* model of a count-driven copy inside the data object; 0 if the port's guard quits */
static int mcopy(uint32_t dst, uint32_t src, uint32_t n)
{
    char t[100];
    if (!in_ds(src, n) || !in_ds(dst, n)) {
        snprintf(t, sizeof t, "Stack copy of 0x%X bytes at 0x%X is outside the data object", (unsigned)n,
                 (unsigned)(in_ds(src, n) ? dst : src));
        mquit(t);
        return 0;
    }
    memmove(MI(dst), MI(src), n);
    return 1;
}

static void m_ws_alloc(void)
{
    int i;
    if (m32(WS_AREA) != 0)
        return;
    for (i = 0; i < 10; i++) {
        mset32(WS_AREA + 4u * (uint32_t)i, 1);
        mset32(WS_NAT + 4u * (uint32_t)i, 1);
        mset32(WS_TST + 4u * (uint32_t)i, 1);
        memset(mw_a[i], 0, N_AREAS);
        memset(mw_t[i], 0, N_TST);
        memset(mw_n[i], 0, N_NATS);
    }
}

static void m_ws_free(void)
{
    if (m32(WS_AREA) == 0)
        return;
    memset(MI(WS_AREA), 0, 120);
    mset32(WS_DEPTH, 0xFFFFFFFFu);
}

static void m_ws_push(void)
{
    int32_t d = (int32_t)m32(WS_DEPTH);
    if (d >= 9) { mquit(TXT_WS_BLOWN); return; }
    d++;
    mset32(WS_DEPTH, (uint32_t)d);
    if (d > (int32_t)m32(WS_HIGH))
        mset32(WS_HIGH, (uint32_t)d);
    memcpy(mw_a[d], MI(AREAS), N_AREAS);
    memcpy(mw_t[d], MI(TST), N_TST);
    memcpy(mw_n[d], MI(NATS), N_NATS);
}

static void m_ws_back(int32_t d)
{
    memcpy(MI(AREAS), mw_a[d], N_AREAS);
    memcpy(MI(TST), mw_t[d], N_TST);
    memcpy(MI(NATS), mw_n[d], N_NATS);
}

static void m_ws_pop(void)
{
    int32_t d = (int32_t)m32(WS_DEPTH);
    if (d < 0) { mquit(g_original ? TXT_WS_POP_O : TXT_WS_POP); return; }
    m_ws_back(d);
    mset32(WS_DEPTH, (uint32_t)(d - 1));
}

static void m_ws_peek(void) { m_ws_back((int32_t)m32(WS_DEPTH)); }

static void m_ws_drop(void)
{
    int32_t d = (int32_t)m32(WS_DEPTH);
    if (d < 0) { mquit(g_original ? TXT_WS_POP_O : TXT_WS_POP); return; }
    mset32(WS_DEPTH, (uint32_t)(d - 1));
}

static void m_ml_push(void)
{
    int32_t d = (int32_t)m32(ML_DEPTH), c;
    if (d >= 6) { mquit(TXT_ML_BLOWN); return; }
    d++;
    mset32(ML_DEPTH, (uint32_t)d);
    if (d > (int32_t)m32(ML_HIGH))
        mset32(ML_HIGH, (uint32_t)d);
    c = (int32_t)m32(L1N);
    mset32(ML_C1 + 4u * (uint32_t)d, (uint32_t)c);
    if (c > 0) {
        mm1n[d] = 4u * (uint32_t)c;
        memcpy(mm1[d], MI(L1), mm1n[d]);
        mset32(ML_P1 + 4u * (uint32_t)d, 1);
    }
    c = (int32_t)m32(L2N);
    mset32(ML_C2 + 4u * (uint32_t)d, (uint32_t)c);
    if (c > 0) {
        mm2n[d] = 6u * (uint32_t)c;
        memcpy(mm2[d], MI(L2), mm2n[d]);
        mset32(ML_P2 + 4u * (uint32_t)d, 1);
    }
}

static void m_ml_back(int32_t d, int and_free)
{
    int32_t c = (int32_t)m32(ML_C1 + 4u * (uint32_t)d);
    mset32(L1N, (uint32_t)c);
    if (c > 0) {
        memcpy(MI(L1), mm1[d], 4u * (uint32_t)c);
        if (and_free) {
            mm1n[d] = 0;
            mset32(ML_P1 + 4u * (uint32_t)d, 0);
        }
    }
    c = (int32_t)m32(ML_C2 + 4u * (uint32_t)d);
    mset32(L2N, (uint32_t)c);
    if (c > 0) {
        memcpy(MI(L2), mm2[d], 6u * (uint32_t)c);
        if (and_free) {
            mm2n[d] = 0;
            mset32(ML_P2 + 4u * (uint32_t)d, 0);
        }
    }
}

static void m_ml_pop(void)
{
    int32_t d = (int32_t)m32(ML_DEPTH);
    if (d < 0) { mquit(g_original ? TXT_ML_POP_O : TXT_ML_POP); return; }
    m_ml_back(d, 1);
    mset32(ML_DEPTH, (uint32_t)(d - 1));
}

static void m_ml_drop(void)
{
    int32_t d = (int32_t)m32(ML_DEPTH);
    if (d < 0) { mquit(g_original ? TXT_ML_POP_O : TXT_ML_POP); return; }
    mm1n[d] = mm2n[d] = 0;
    mset32(ML_P1 + 4u * (uint32_t)d, 0);
    mset32(ML_C1 + 4u * (uint32_t)d, 0);
    mset32(ML_P2 + 4u * (uint32_t)d, 0);
    mset32(ML_C2 + 4u * (uint32_t)d, 0);
    mset32(ML_DEPTH, (uint32_t)(d - 1));
}

static void m_snap_lists(void)
{
    uint32_t c = m32(L1N);
    mset32(0x11AB48u, c);
    if (!mcopy(0x11A828u, L1, 4u * c))
        return;
    c = m32(L2N);
    mset32(0x11AED0u, c);
    mcopy(0x11AB4Cu, L2, 6u * c);
}

static void m_back_lists(void)
{
    uint32_t c = m32(0x11AB48u);
    mset32(L1N, c);
    if (!mcopy(L1, 0x11A828u, 4u * c))
        return;
    c = m32(0x11AED0u);
    mset32(L2N, c);
    mcopy(L2, 0x11AB4Cu, 6u * c);
}

static uint32_t g_rng = 12345u;
static uint32_t rnd(uint32_t n) { g_rng = g_rng * 1664525u + 1013904223u; return (g_rng >> 8) % n; }

/* the same random edit in both images */
static void edit_both(void)
{
    /* the copied ranges with 4 bytes before and after, except where a count field lies next to them: list 1 is
       followed by its count [0x1079B4], list 2 (0x1079B8 = 0x1079B4 + 4) by [0x107D3C], the snapshot buffers by
       [0x11AB48] and [0x11AED0]; the counts are edited alone, with values a list can hold (or a negative one) */
    static const uint32_t lo[] = { AREAS - 4, TST - 4, NATS - 4, L1 - 4, L2, 0x11A828u - 4, 0x11AB4Cu,
                                   0x117CC0u - 4, 0x117AE0u - 4, 0x116518u - 4, 0x116FFCu - 4 };
    static const uint32_t hi[] = { AREAS + N_AREAS + 4, TST + N_TST + 4, NATS + N_NATS + 4, L1 + 800, L2 + 900,
                                   0x11A828u + 800, 0x11AB4Cu + 900, 0x117CC0u + N_AREAS + 4, 0x117AE0u + N_TST,
                                   0x116518u + N_NATS + 4, 0x116FFCu + N_NATS + 4 };
    static const uint32_t cnt[] = { L1N, L2N, 0x11AB48u, 0x11AED0u }, cap[] = { 200, 150, 200, 150 };
    uint32_t k = rnd(15), i, n, a;

    if (k < 11) {
        n = 1 + rnd(64);
        a = lo[k] + rnd(hi[k] - lo[k] - n + 1);
        for (i = 0; i < n; i++) {
            uint8_t v = (uint8_t)rnd(256);
            setD8(a + i, v);
            *MI(a + i) = v;
        }
    } else {
        int32_t c = rnd(8) == 0 ? -(int32_t)rnd(3) : (int32_t)rnd(cap[k - 11] + 1);
        setD32(cnt[k - 11], (uint32_t)c);
        mset32(cnt[k - 11], (uint32_t)c);
    }
}

/* compare the port's state with the model's; returns the number of differences found */
static int compare(void)
{
    int bad = 0;
    uint32_t a, i, nz = 0;

    if (memcmp(g_ds, M, WS_AREA - DS_BASE) != 0
        || memcmp(g_ds + (WS_TST + 40u - DS_BASE), M + (WS_TST + 40u - DS_BASE), ML_P1 - (WS_TST + 40u)) != 0
        || memcmp(g_ds + (ML_C1 - DS_BASE), M + (ML_C1 - DS_BASE), DS_BASE + DS_SIZE - ML_C1) != 0) {
        for (a = DS_BASE; a < DS_BASE + DS_SIZE; a++)
            if (!is_handle_field(a & ~3u) && g_ds[a - DS_BASE] != M[a - DS_BASE]) {
                printf("  image differs first at 0x%X: port %02X model %02X\n", (unsigned)a, g_ds[a - DS_BASE],
                       M[a - DS_BASE]);
                break;
            }
        bad++;
    }
    for (i = 0; i < 10; i++) {
        static const uint32_t f[3] = { WS_AREA, WS_NAT, WS_TST }, sz[3] = { N_AREAS, N_NATS, N_TST };
        uint32_t j;
        for (j = 0; j < 3; j++) {
            uint32_t h = D32(f[j] + 4 * i);
            const uint8_t *want = j == 0 ? mw_a[i] : j == 1 ? mw_n[i] : mw_t[i];
            if ((h != 0) != (m32(f[j] + 4 * i) != 0)) { printf("  WS field 0x%X\n", (unsigned)(f[j] + 4 * i)); bad++; continue; }
            if (h == 0)
                continue;
            nz++;
            if (load_heap_ptr(h, sz[j]) == NULL || load_heap_ptr(h, sz[j] + 1) != NULL
                || memcmp(load_heap_ptr(h, sz[j]), want, sz[j]) != 0) {
                printf("  WS slot %u kind %u block differs\n", (unsigned)i, (unsigned)j);
                bad++;
            }
        }
    }
    for (i = 0; i < 7; i++) {
        uint32_t h1 = D32(ML_P1 + 4 * i), h2 = D32(ML_P2 + 4 * i);
        if ((h1 != 0) != (m32(ML_P1 + 4 * i) != 0) || (h2 != 0) != (m32(ML_P2 + 4 * i) != 0)) {
            printf("  ML slot %u handle zero-ness differs\n", (unsigned)i);
            bad++;
            continue;
        }
        if (h1 != 0) {
            nz++;
            if (load_heap_ptr(h1, mm1n[i]) == NULL || load_heap_ptr(h1, mm1n[i] + 1) != NULL
                || memcmp(load_heap_ptr(h1, mm1n[i]), mm1[i], mm1n[i]) != 0) { printf("  ML slot %u list 1 block\n", (unsigned)i); bad++; }
        }
        if (h2 != 0) {
            nz++;
            if (load_heap_ptr(h2, mm2n[i]) == NULL || load_heap_ptr(h2, mm2n[i] + 1) != NULL
                || memcmp(load_heap_ptr(h2, mm2n[i]), mm2[i], mm2n[i]) != 0) { printf("  ML slot %u list 2 block\n", (unsigned)i); bad++; }
        }
    }
    if (live_blocks() != nz) {
        printf("  %u live blocks, %u non-zero handle fields\n", (unsigned)live_blocks(), (unsigned)nz);
        bad++;
    }
    return bad;
}

static long g_opn[17], g_opq[17];
static const char *const names_b[] = { "edit", "7A8D0", "7A940", "7AC60", "7AF28", "7AD24", "7ABD0", "7AA1C", "7AE14",
                                       "7AFB8", "7AD7C", "7A688", "7A700", "7A778", "7A7B0", "7A7E8", "7A85C" };

static void part_b(long ops, long *done, long *bad, long *quits)
{
    const char *const *names = names_b;
    long n;
    int i;

    /* a clean start in both (the trace is checked after every operation, then cleared) */
    g_trace[0] = 0;
    run(f7ABD0);
    for (i = 0; i < 7; i++) {
        load_heap_free(D32(ML_P1 + 4u * (uint32_t)i));
        load_heap_free(D32(ML_P2 + 4u * (uint32_t)i));
    }
    memset(DP(ML_P1), 0, 0x112A94u - ML_P1);
    setD32(ML_DEPTH, 0xFFFFFFFFu);
    setD32(WS_DEPTH, 0xFFFFFFFFu);
    setD32(WS_HIGH, 0);
    memcpy(M, g_ds, DS_SIZE);
    memset(mm1n, 0, sizeof mm1n);
    memset(mm2n, 0, sizeof mm2n);
    if (compare() != 0)
        printf("FAIL: B start state\n");

    for (n = 0; n < ops; n++) {
        uint32_t op = rnd(17);
        int q, mq0 = m_quits, wsd = S32(WS_DEPTH), mld = S32(ML_DEPTH), alloc = D32(WS_AREA) != 0;
        void (*fn)(void) = NULL;

        g_original = rnd(4) == 0;
        if (op == 0 || (op <= 6 && !alloc && op != 1 && op != 5 && op != 6) || (op == 4 && wsd < 0)
            || (op == 9 && mld < 0)) {
            edit_both();
            continue;
        }
        switch (op) {
        case 1: fn = f7A8D0; m_ws_alloc(); break;
        case 2: fn = f7A940; m_ws_push(); break;
        case 3: fn = f7AC60; m_ws_pop(); break;
        case 4: fn = f7AF28; m_ws_peek(); break;
        case 5: fn = f7AD24; m_ws_drop(); break;
        case 6: if (rnd(20) != 0) { edit_both(); continue; } fn = f7ABD0; m_ws_free(); break;
        case 7: fn = f7AA1C; m_ml_push(); break;
        case 8: fn = f7AE14; m_ml_pop(); break;
        case 9: fn = f7AFB8; m_ml_back((int32_t)m32(ML_DEPTH), 0); break;
        case 10: fn = f7AD7C; m_ml_drop(); break;
        case 11: fn = f7A688; memcpy(MI(0x117CC0u), MI(AREAS), N_AREAS); memcpy(MI(0x117AE0u), MI(TST), N_TST);
                 memcpy(MI(0x116518u), MI(NATS), N_NATS); break;
        case 12: fn = f7A700; memcpy(MI(AREAS), MI(0x117CC0u), N_AREAS); memcpy(MI(TST), MI(0x117AE0u), N_TST);
                 memcpy(MI(NATS), MI(0x116518u), N_NATS); break;
        case 13: fn = f7A778; memcpy(MI(0x116FFCu), MI(NATS), N_NATS); break;
        case 14: fn = f7A7B0; memcpy(MI(NATS), MI(0x116FFCu), N_NATS); break;
        case 15: fn = f7A7E8; m_snap_lists(); break;
        default: fn = f7A85C; m_back_lists(); break;
        }
        g_quits = 0;
        q = run(fn);
        (*done)++;
        if (q != (m_quits != mq0) || (q && strcmp(g_quitmsg, m_quit) != 0)) {
            printf("FAIL: B op %ld %s: quit %d (%s), model %d (%s)\n", n, names[op], q, q ? g_quitmsg : "",
                   m_quits != mq0, m_quits != mq0 ? m_quit : "");
            (*bad)++;
        }
        if (strcmp(g_trace, q ? "44CAC " : "") != 0) {
            printf("FAIL: B op %ld %s: call trace %s\n", n, names[op], g_trace);
            (*bad)++;
        }
        g_trace[0] = 0;
        g_opn[op]++;
        if (q) {
            (*quits)++;
            g_opq[op]++;
        }
        if (compare() != 0) {
            printf("FAIL: B op %ld %s (g_original %d)\n", n, names[op], g_original);
            (*bad)++;
            memcpy(M, g_ds, DS_SIZE);              /* resynchronise the images (the blocks stay as they are) */
            if (*bad > 20)
                return;
        }
    }
    g_original = 0;
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:/work/advciv_run";
    long done = 0, bad = 0, quits = 0;
    int a_tests, a_fail, i;

    setvbuf(stdout, NULL, _IONBF, 0);
    if (state_init(gamedir) != 0)
        return 1;
    g_want = malloc(DS_SIZE);
    M = malloc(DS_SIZE);
    if (g_want == NULL || M == NULL)
        return 1;

    part_a();
    a_tests = g_tests;
    a_fail = g_fail;
    printf("A. hand vectors: %d of %d pass\n", a_tests - a_fail, a_tests);

    part_b(300000, &done, &bad, &quits);
    printf("B. random differential run: %ld operations (%ld quits), %ld disagree\n", done, quits, bad);
    for (i = 1; i < 17; i++)
        printf("  %s %ld (%ld quit)%s", names_b[i], g_opn[i], g_opq[i], i % 4 == 0 ? "\n" : ";");
    printf("\n");
    {
        const char *t = g_trace;
        while (strncmp(t, "44CAC ", 6) == 0)
            t += 6;
        printf("call trace: %s\n", *t == 0 ? "only f44CAC, once per quit" : t);
        if (*t != 0)
            bad++;
    }

    return g_fail == 0 && bad == 0 ? 0 : 1;
}
