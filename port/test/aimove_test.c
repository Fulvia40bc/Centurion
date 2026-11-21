/* aimove_test [GAMEDIR [SAVEDIR]]: unit tests for module AIMOVE's top level (aimove.c) that need no game run.

   It loads the data object from GAMEDIR/AC_MAIN.EXE as the port does (state_init; GAMEDIR defaults to the run copy
   C:\work\advciv_run, only read), the territory records from GAMEDIR's terrflag.dat (f12394) and the land lists from
   GAMEDIR's LANDADJ.DAT into one load.c heap block (a handle per territory at 0xF8E7C + 4t, as the game keeps them),
   and the saves with saveload.c's f5F05C: GAMEDIR/savgame0.dat (the R4 game at the start of MOVEMENT) and
   SAVEDIR/SAVGAMEA.DAT (the R4 autosave, seven computers; SAVEDIR defaults to C:\work\advciv_play). All read-only.

   aimove.c is compiled with -DAIMOVE_TEST, so its calls into the other modules of M7's batch 1 (AIQ, AICITY, AISEA,
   AIPLAN: not ported yet) go to the recorders here. Each recorder logs its call and arguments in a trace and answers
   from a seeded stream (return values, *cnt / *need / *out / *flag, lists of real land neighbours), and many make
   small changes to the world (a token moved with f57CA4, move-list records, +0x5D, target marks, ship records) so the
   World State and Move List stacks' pushes, pops, peeks and drops have something to undo or keep. f5AAA4(0) and
   f5C238(0) (tested in their own modules) are recorders of the same kind (token removals). The real ported helpers
   run on both sides: rules.c (f57CA4, f7C760, f7C7C4, f78D08, f46158), move.c, formulas.c (f46A4C, f46B34),
   mapdata.c (f3F698), stack.c and mstack.c (the two stacks). The linker's --wrap puts a tracing wrapper around the
   stack operations and those helpers, so the trace holds every push / pop / peek / drop / snapshot in order.

     A. random differential runs of f387C8, f37CF8, f396FC, f38E54, f39068 and f461BC against a second
        implementation written here from the listing (not from aimove.c), run on a copy of the data object (g_ds
        swapped) with the same recorders and the same stream seed. States: the two saves with random tokens of nations
        1..11 added through the primitives, cities (f78E74), AI site marks, attitudes 0xF52E0, the AI table 0x112A94,
        the AI level, the turn order 0xF06C0, advances (+0x26, +0x2A, +0x31), treasuries, AST squares, human flags,
        movable counts, the token supply, [0xF5908], word [0xBB3F0]. After each run: the whole data object, the
        trace (every recorder and wrapped call with its arguments, in order), the stream position and the stack depths
        must equal the model's. g_original is run too (nothing in this module depends on it).
     B. hand vectors: f387C8's branches with expected *need values worked out by hand; f461BC's entry (the area clears,
        +0x24 + n, the token box colours, the nation stack, the order of its first calls) and its balanced stacks.
     C. the saves: f461BC for each computer in play on both saves, port against the model.
     M. mutation check: deliberately broken copies of the model (15 mutations, one per rule) must each be caught by
        part A's comparison.
   Prints each failure, the counts per part, and exits 0 when all pass.

   Build (from the repo root; artefacts outside the repo):
     clang -std=c11 -Wall -Wextra -DAIMOVE_TEST -Iport/src port/test/aimove_test.c port/src/aimove.c port/src/rules.c
       port/src/move.c port/src/formulas.c port/src/stack.c port/src/mstack.c port/src/mapdata.c port/src/load.c
       port/src/state.c port/src/le_image.c port/src/saveload.c -Wl,--wrap=f7A940,--wrap=f7AC60,--wrap=f7AA1C,
       --wrap=f7AE14,--wrap=f7AD24,--wrap=f7AD7C,--wrap=f7AF28,--wrap=f7AFB8,--wrap=f7A688,--wrap=f7A700,
       --wrap=f7A7E8,--wrap=f7A85C,--wrap=f57CA4,--wrap=f3F698,--wrap=f46158,--wrap=f78D08,--wrap=f7C760,
       --wrap=f7C7C4,--wrap=f46A4C,--wrap=f46B34 -o aimove_test.exe */

#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <direct.h>
#define chdir _chdir
#define getcwd _getcwd

#include "../src/state.h"
#include "../src/game.h"
#include "../src/aimove.h"
#include "../src/city.h"
#include "../src/conflict.h"
#include "../src/fg.h"
#include "../src/junk.h"
#include "../src/kit.h"
#include "../src/map.h"
#include "../src/mapdata.h"
#include "../src/move.h"
#include "../src/movephase.h"
#include "../src/mstack.h"
#include "../src/panel.h"
#include "../src/rules.h"
#include "../src/save.h"
#include "../src/saveload.h"
#include "../src/stack.h"
#include "../src/ui.h"

/* ---- checks -------------------------------------------------------------------------------------------------------- */

static int g_tests, g_fail, g_quiet_fail;

static int check(int ok, const char *what)
{
    g_tests++;
    if (!ok) {
        g_fail++;
        if (!g_quiet_fail)
            printf("FAIL %s\n", what);
    }
    return ok;
}

/* ---- the two sides: trace, stream ------------------------------------------------------------------------------ */

typedef struct {
    char *buf;
    size_t len, cap;
    uint32_t s;      /* stream state */
    int used;        /* stream draws */
} side_t;

static side_t g_sd[2];
static int g_side;            /* 0 = the port, 1 = the model */
static uint8_t *g_port_ds, *m_ds;
static int g_trace_on = 1;

static void tr(const char *fmt, ...)
{
    side_t *sd = &g_sd[g_side];
    char tmp[256];
    int k;
    va_list ap;

    if (!g_trace_on)
        return;
    va_start(ap, fmt);
    k = vsnprintf(tmp, sizeof tmp, fmt, ap);
    va_end(ap);
    if (k < 0)
        return;
    if (k >= (int)sizeof tmp)
        k = (int)sizeof tmp - 1;
    if (sd->len + (size_t)k + 2 > sd->cap) {
        size_t nc = sd->cap ? sd->cap * 2 : 1 << 16;
        char *nb;
        while (nc < sd->len + (size_t)k + 2)
            nc *= 2;
        nb = realloc(sd->buf, nc);
        if (nb == NULL)
            return;
        sd->buf = nb;
        sd->cap = nc;
    }
    memcpy(sd->buf + sd->len, tmp, (size_t)k);
    sd->len += (size_t)k;
    sd->buf[sd->len++] = ' ';
    sd->buf[sd->len] = 0;
}

static int rnd(int n)   /* the side's stream: 0..n-1 */
{
    side_t *sd = &g_sd[g_side];
    sd->s = sd->s * 1103515245u + 12345u;
    sd->used++;
    return n <= 1 ? 0 : (int)((sd->s >> 16) % (uint32_t)n);
}

/* the test's own generator for the states (not a side's stream) */
static uint32_t g_rs = 20261121u;
static int grnd(int n)
{
    g_rs = g_rs * 1664525u + 1013904223u;
    return n <= 1 ? 0 : (int)((g_rs >> 8) % (uint32_t)n);
}

/* ---- addresses (written out here from the listing) --------------------------------------------------------------- */

#define A_(r, off)   (0xE31B8u + 69u * (uint32_t)(r) + (uint32_t)(off))
#define NA_(m, off)  (0xF14CCu + 199u * (uint32_t)(m) + (uint32_t)(off))
#define TS_(t, off)  (0xE2C18u + 3u * (uint32_t)(t) + (uint32_t)(off))
#define TR_(t, off)  (0xE1638u + 35u * (uint32_t)(t) + (uint32_t)(off))
#define LIST1        0x107694u
#define LIST1N       0x1079B4u
#define LIST2        0x1079B8u
#define LIST2N       0x107D3Cu
#define WSD          0xCAC54u
#define MLD          0xCAC58u

static int32_t ATT_(int32_t a, int32_t b) { return S32(0xF52E0u + 40u * (uint32_t)a + 4u * (uint32_t)b); }
static int32_t TAB_(int32_t row, int32_t n) { return S16(0x112A94u + 20u * (uint32_t)row + 2u * (uint32_t)(n - 1)); }
static int32_t LEVEL_(void) { return (D16(0xE1624u) >> 10) & 3; }
static int32_t ORD_(int32_t m) { return S8(0xF06C0u + (uint32_t)m); }
static int32_t ERA_(int32_t row_nation, int32_t ast)
{
    return S8(0xBCCE0u + (uint32_t)(85 * (row_nation - 1)) + (uint32_t)(5 * ast));
}
static int32_t ERAW_(int32_t era, int32_t lvl) { return S32(0xC2864u + (uint32_t)(12 * era) + (uint32_t)(4 * lvl)); }

/* the land list of t (the game's heap block through the handle) */
static int land_n(int t)
{
    const uint8_t *p = load_heap_ptr(D32(0xF8E7Cu + 4u * (uint32_t)t), 2);
    return p ? (int16_t)(p[0] | p[1] << 8) : 0;
}
static int land_at(int t, int j)   /* j = 1.. */
{
    const uint8_t *p = load_heap_ptr(D32(0xF8E7Cu + 4u * (uint32_t)t) + 2u * (uint32_t)j, 2);
    return p ? (int16_t)(p[0] | p[1] << 8) : 0;
}

/* ---- wrapped helpers (both sides; the trace keeps their order) --------------------------------------------------- */

void __real_f7A940(void); void __real_f7AC60(void); void __real_f7AA1C(void); void __real_f7AE14(void);
void __real_f7AD24(void); void __real_f7AD7C(void); void __real_f7AF28(void); void __real_f7AFB8(void);
void __real_f7A688(void); void __real_f7A700(void); void __real_f7A7E8(void); void __real_f7A85C(void);
void __real_f57CA4(int a, int b, int n, int flag, int k); void __real_f3F698(int t, int c); void __real_f46158(int n);
int __real_f78D08(int t, int n); int __real_f7C760(int t); int __real_f7C7C4(int t, int n);
int __real_f46A4C(int n); int __real_f46B34(int n);
void __wrap_f7A940(void); void __wrap_f7AC60(void); void __wrap_f7AA1C(void); void __wrap_f7AE14(void);
void __wrap_f7AD24(void); void __wrap_f7AD7C(void); void __wrap_f7AF28(void); void __wrap_f7AFB8(void);
void __wrap_f7A688(void); void __wrap_f7A700(void); void __wrap_f7A7E8(void); void __wrap_f7A85C(void);
void __wrap_f57CA4(int a, int b, int n, int flag, int k); void __wrap_f3F698(int t, int c); void __wrap_f46158(int n);
int __wrap_f78D08(int t, int n); int __wrap_f7C760(int t); int __wrap_f7C7C4(int t, int n);
int __wrap_f46A4C(int n); int __wrap_f46B34(int n);

void __wrap_f7A940(void) { tr("WSpush"); __real_f7A940(); }
void __wrap_f7AC60(void) { tr("WSpop"); __real_f7AC60(); }
void __wrap_f7AA1C(void) { tr("MLpush"); __real_f7AA1C(); }
void __wrap_f7AE14(void) { tr("MLpop"); __real_f7AE14(); }
void __wrap_f7AD24(void) { tr("WSdrop"); __real_f7AD24(); }
void __wrap_f7AD7C(void) { tr("MLdrop"); __real_f7AD7C(); }
void __wrap_f7AF28(void) { tr("WSpeek"); __real_f7AF28(); }
void __wrap_f7AFB8(void) { tr("MLpeek"); __real_f7AFB8(); }
void __wrap_f7A688(void) { tr("WSsnap"); __real_f7A688(); }
void __wrap_f7A700(void) { tr("WSback"); __real_f7A700(); }
void __wrap_f7A7E8(void) { tr("MLsnap"); __real_f7A7E8(); }
void __wrap_f7A85C(void) { tr("MLback"); __real_f7A85C(); }
void __wrap_f57CA4(int a, int b, int n, int flag, int k) { tr("57CA4(%d,%d,%d,%d,%d)", a, b, n, flag, k); __real_f57CA4(a, b, n, flag, k); }
void __wrap_f3F698(int t, int c) { tr("3F698(%d,%d)", t, c); __real_f3F698(t, c); }
void __wrap_f46158(int n) { tr("46158(%d)", n); __real_f46158(n); }
int __wrap_f78D08(int t, int n) { int r = __real_f78D08(t, n); tr("78D08(%d,%d)=%d", t, n, r); return r; }
int __wrap_f7C760(int t) { int r = __real_f7C760(t); tr("7C760(%d)=%d", t, r); return r; }
int __wrap_f7C7C4(int t, int n) { int r = __real_f7C7C4(t, n); tr("7C7C4(%d,%d)=%d", t, n, r); return r; }
int __wrap_f46A4C(int n) { int r = __real_f46A4C(n); tr("46A4C(%d)=%d", n, r); return r; }
int __wrap_f46B34(int n) { int r = __real_f46B34(n); tr("46B34(%d)=%d", n, r); return r; }

/* ---- world changes the recorders make (same stream on both sides) ------------------------------------------------- */

/* a token of n from one of its territories with movable tokens to a land neighbour, with a land record */
static void wob_move(int n)
{
    int cnt = S8(NA_(n, 0x60)), i, r, t, u, k;
    if (cnt <= 0)
        return;
    i = rnd(cnt);
    r = D8(NA_(n, 0x61 + i));
    t = r - 1;
    if (t < 0 || t >= 157 || S8(A_(r, 0x24 + n)) <= 0 || land_n(t) <= 0)
        return;
    k = land_n(t);
    u = land_at(t, 1 + rnd(k));
    if (u < 0 || u >= 157)
        return;
    __wrap_f57CA4(t, u, n, 1, 1);
    {
        uint32_t c = D32(LIST1N);
        if (c < 190) {
            setD8(LIST1 + 4 * c, (uint8_t)t);
            setD8(LIST1 + 4 * c + 1, (uint8_t)u);
            setD8(LIST1 + 4 * c + 2, 1);
            setD8(LIST1 + 4 * c + 3, 0x77);
            setD32(LIST1N, c + 1);
        }
    }
}

static void wobble(int n)
{
    int w = rnd(6);
    if (n < 1 || n > 9)
        return;
    switch (w) {
    case 0:
        wob_move(n);
        break;
    case 1: {
        uint32_t c = D32(LIST2N);
        if (c < 100) {
            setD8(LIST2 + 6 * c, (uint8_t)rnd(157));
            setD8(LIST2 + 6 * c + 1, (uint8_t)rnd(157));
            setD8(LIST2 + 6 * c + 4, 10);
            setD32(LIST2N, c + 1);
        }
        break;
    }
    case 2:
        setD8(NA_(n, 0x5D), (uint8_t)(D8(NA_(n, 0x5D)) + (rnd(2) ? 1 : 0xFF)));
        break;
    case 3:
        setD8(A_(1 + rnd(157), 0x3B + n), (uint8_t)rnd(3));
        break;
    default:
        break;
    }
}

/* ---- the recorders: aimove.c's calls into AIQ, AICITY, AISEA, AIPLAN -------------------------------------------- */

void aimove_test_f7BD5C(int n) { tr("7BD5C(%d)", n); }
void aimove_test_f46998(int n)
{
    tr("46998(%d)", n);
    if (rnd(2))
        setD8(A_(1 + rnd(157), 0x3B + n), 1);
}
void aimove_test_f160EC(int n) { tr("160EC(%d)", n); }
int aimove_test_f468EC(void) { int r = rnd(4) == 0; tr("468EC()=%d", r); return r; }
void aimove_test_f48028(int32_t *cnt, int32_t *list, int t, int n, int filter)
{
    int k = 0, j, u, nn = t >= 0 && t < 160 ? land_n(t) : 0;
    if (rnd(8) != 0)
        for (j = 1; j <= nn && k < 40; j++) {
            u = land_at(t, j);
            if (u < 0 || u >= 157)
                continue;
            if ((n >= 1 && n <= 13 && S8(A_(u + 1, 0x0E + n)) > 0) || rnd(4) == 0)
                list[k++] = u;
        }
    *cnt = k;
    tr("48028(%d,%d,%d)=%d:%d", t, n, filter, k, k > 0 ? list[0] : -1);
}
void aimove_test_f48D44(int32_t *cnt, int32_t *list, int n, int t, int flags)
{
    int k = *cnt, i, x;
    if (rnd(2))
        for (i = 0; i < k / 2; i++) {
            x = list[i];
            list[i] = list[k - 1 - i];
            list[k - 1 - i] = x;
        }
    if (k > 0 && rnd(4) == 0)
        k--;
    *cnt = k;
    tr("48D44(%d,%d,%d,%d)=%d", *cnt, n, t, flags, k > 0 ? list[0] : -1);
}
int aimove_test_f4CC5C(int t, int n, int mode) { int r = rnd(7) - 1; tr("4CC5C(%d,%d,%d)=%d", t, n, mode, r); return r; }
int aimove_test_f39C08(int n) { int r = rnd(11); tr("39C08(%d)=%d", n, r); return r; }
int aimove_test_f49934(int t, int n, int mode) { int r = rnd(3) != 0; tr("49934(%d,%d,%d)=%d", t, n, mode, r); return r; }
int aimove_test_f4CEE4(int t, int n, int32_t *out)
{
    int r = rnd(5) == 0 ? -1 : rnd(12);
    *out = rnd(10);
    tr("4CEE4(%d,%d)=%d,%d", t, n, r, *out);
    return r;
}
int aimove_test_f4D110(int t, int32_t *cnt, int32_t *list, int m, int k)
{
    int r = rnd(6);
    tr("4D110(%d,%d,%d,%d,%d)=%d", t, *cnt, *cnt > 0 ? list[0] : -1, m, k, r);
    return r;
}
int aimove_test_f337C8(int n) { int r = rnd(4); tr("337C8(%d)=%d", n, r); return r; }
void aimove_test_f33964(int goal, int32_t *cnt, int n, int mode)
{
    int add = rnd(3);
    tr("33964(%d,%d,%d,%d)+%d", goal, *cnt, n, mode, add);
    *cnt += add;
    wobble(n);
}
int aimove_test_f3580C(int need, int t, int n, int mode, int s)
{
    int r;
    wobble(n);
    wobble(n);
    r = rnd(3);
    tr("3580C(%d,%d,%d,%d,%d)=%d", need, t, n, mode, s, r);
    return r;
}
int aimove_test_f4D29C(int t, int m) { int r = rnd(3) == 0 ? 1 + rnd(3) : 0; tr("4D29C(%d,%d)=%d", t, m, r); return r; }
void aimove_test_f36918(int n, int t, int32_t *need, int code)
{
    int w = rnd(4);
    tr("36918(%d,%d,%d,%d)/%d", n, t, *need, code, w);
    if (w == 0)
        *need = 0;
    else if (w == 1)
        *need -= 1;
    wobble(n);
}
void aimove_test_f643C8(int n) { tr("643C8(%d)", n); wobble(n); }
void aimove_test_f648C8(int n, int flags) { tr("648C8(%d,%d)", n, flags); wobble(n); }
void aimove_test_f65CB0(int n) { tr("65CB0(%d)", n); wobble(n); }
void aimove_test_f4A088(int n, int a, int b) { tr("4A088(%d,%d,%d)", n, a, b); wobble(n); wobble(n); }
void aimove_test_f4C074(int n) { tr("4C074(%d)", n); wobble(n); }
void aimove_test_f4C9B8(int n, int x) { tr("4C9B8(%d,%d)", n, x); wobble(n); }
void aimove_test_f46BB8(int n) { tr("46BB8(%d)", n); wobble(n); if (rnd(3) == 0) wob_move(n); }
void aimove_test_f4C6E4(int n) { tr("4C6E4(%d)", n); wobble(n); }
void aimove_test_f478D8(int c, int32_t *cnt, int n, int32_t *flag)
{
    int a = rnd(2), f = rnd(3) == 0;
    tr("478D8(%d,%d,%d,%d)+%d,%d", c, *cnt, n, *flag, a, f);
    *cnt += a;
    if (f)
        *flag = 1;
    wobble(n);
}
void aimove_test_f47A44(int c, int32_t *cnt, int n, int32_t *flag)
{
    int a = rnd(2);
    tr("47A44(%d,%d,%d,%d)+%d", c, *cnt, n, *flag, a);
    *cnt += a;
    wobble(n);
}
void aimove_test_f49E04(int n) { tr("49E04(%d)", n); wobble(n); wob_move(n); }
void aimove_test_f4BBDC(int n) { tr("4BBDC(%d)", n); wobble(n); }
int aimove_test_f49A1C(int t, int n, int k, int exclude)
{
    int r = rnd(3) == 0;
    tr("49A1C(%d,%d,%d,%d)=%d", t, n, k, exclude, r);
    return r;
}

/* ---- ported routines not linked: recorders (tested in their own modules) ------------------------------------------ */

static void remove_token(int r, int m)
{
    f57CE8(r - 1, m, 0, 1);
    setD8(NA_(m, 0x98), (uint8_t)(S8(NA_(m, 0x98)) - 1));
    setD8(NA_(m, 0x99), (uint8_t)(S8(NA_(m, 0x99)) + 1));
}

void f5AAA4(int show)
{
    int i, r, m, tries;
    tr("5AAA4(%d)", show);
    for (i = rnd(3); i > 0; i--)
        for (tries = 0; tries < 20; tries++) {
            r = 1 + rnd(157);
            m = 1 + rnd(11);
            if (S8(A_(r, 1)) >= 2 && S8(A_(r, 0x0E + m)) > 0) {
                remove_token(r, m);
                break;
            }
        }
}
void f5C238(int show)
{
    int r, m, tries;
    tr("5C238(%d)", show);
    if (rnd(2))
        for (tries = 0; tries < 20; tries++) {
            r = 1 + rnd(157);
            m = 1 + rnd(9);
            if (S8(A_(r, 0x0E + m)) > 1) {
                remove_token(r, m);
                break;
            }
        }
}
void f4D518(int n, int show) { tr("4D518(%d,%d) L1=%u L2=%u", n, show, (unsigned)D32(LIST1N), (unsigned)D32(LIST2N)); }
void f78A20(int t) { tr("78A20(%d)", t); }
void f3F6E4(void) { tr("3F6E4"); memset(DP(0xFFC00u), 0, 0xA0); }
void f3F758(void) { tr("3F758"); memset(DP(0xFFCA0u), 0, 0xA0); }

/* ---- stubs the linked files need (none may run) ------------------------------------------------------------------ */

static jmp_buf g_jb;
static int g_quits, g_np;
_Noreturn void f44CAC(void) { g_quits++; tr("QUIT[%s]", DSTR(0xF5500u)); longjmp(g_jb, 1); }
_Noreturn void not_ported(uint32_t addr) { g_np++; tr("NP(%X)", (unsigned)addr); longjmp(g_jb, 2); }
_Noreturn void port_longjmp(uint32_t jmpbuf) { tr("LJ(%X)", (unsigned)jmpbuf); longjmp(g_jb, 3); }
void f3A780(const char *msg) { tr("3A780[%s]", msg); }
void f3A990(void) { tr("3A990"); }
void f3AB7C(const char *text) { tr("3AB7C[%s]", text); }
void f3AF54(uint32_t spr) { (void)spr; tr("3AF54"); }
void f3B7CC(uint32_t dlg, int id) { (void)dlg; (void)id; tr("3B7CC"); }
void f3B870(uint32_t dlg, int id) { (void)dlg; (void)id; tr("3B870"); }
int f3B9A4(uint32_t dlg, int id) { (void)dlg; (void)id; tr("3B9A4"); return 0; }
void f3BFBC(uint32_t dlg, int id, int enabled) { (void)dlg; (void)id; (void)enabled; tr("3BFBC"); }
void f3C284(uint32_t dlg, int id, uint32_t label) { (void)dlg; (void)id; (void)label; tr("3C284"); }
void f3C314(uint32_t dlg, int id) { (void)dlg; (void)id; tr("3C314"); }
void f3C3D0(uint32_t dlg, int id) { (void)dlg; (void)id; tr("3C3D0"); }
void f3C7A0(uint32_t dlg) { (void)dlg; tr("3C7A0"); }
void f3C828(uint32_t dlg) { (void)dlg; tr("3C828"); }
void f3CBBC(void) { tr("3CBBC"); }
void f3CC68(void) { tr("3CC68"); }
void f3CCBC(void) { tr("3CCBC"); }
void f3D52C(int dx, int dy) { tr("3D52C(%d,%d)", dx, dy); }
void f426D0(int i) { tr("426D0(%d)", i); }
void f572CC(int t) { tr("572CC(%d)", t); }
void f5E7D0(void) { tr("5E7D0"); }
int f5FF28(void) { tr("5FF28"); return -15; }
void f76F70(int n, uint8_t *cards) { (void)n; (void)cards; tr("76F70"); }
void f787F8(int t) { tr("787F8(%d)", t); }
void f78884(void) { tr("78884"); }
void f788AC(void) { tr("788AC"); }
int f78AE4(void) { tr("RAND"); return 0; }
int f78BA4(uint32_t dlg, int map) { (void)dlg; (void)map; tr("78BA4"); return -1; }
void fg_move(int x, int y) { (void)x; (void)y; tr("fg_move"); }
void fg_vbclose(void) { tr("vbclose"); }
int fg_vbopen(int handle) { (void)handle; tr("vbopen"); return 0; }
int junk_makegif(int minx, int maxx, int miny, int maxy, const char *name)
{
    (void)minx; (void)maxx; (void)miny; (void)maxy; (void)name;
    tr("junk_makegif");
    return 0;
}
int junk_showgif(const char *name, int flags) { (void)name; (void)flags; tr("junk_showgif"); return 0; }

/* ================================================================================================================== */
/* ---- the model: the six routines written again from the listing ---------------------------------------------------- */
/* ================================================================================================================== */

static int g_mut;   /* 0 = faithful; 1..15 = one deliberate break (part M) */

static void m_rec(int from, int to, int k, int type)
{
    uint32_t i = D32(LIST1N);
    setD8(LIST1 + 4 * i + 0, (uint8_t)from);
    setD8(LIST1 + 4 * i + 1, (uint8_t)to);
    setD8(LIST1 + 4 * i + 2, (uint8_t)k);
    setD8(LIST1 + 4 * i + 3, (uint8_t)type);
    setD32(LIST1N, i + 1);
}

/* 0x387C8 */
static int m_387C8(int t, int n, int mode, int32_t *need, int32_t *victim, int x)
{
    int r = t + 1, o, bonus, T, arch, after, before;
    int32_t lc, ls[160];

    if (S8(A_(r, 1)) != 1)
        return 0;
    if ((D8(TS_(t, 2)) & 2) && S8(A_(r, 1 + n)) == n)
        return 0;
    o = __wrap_f7C760(t);
    *victim = o;
    bonus = (S8(NA_(n, 0x26)) != 0 && S8(NA_(o, 0x26)) == 0) ? 1 : 0;
    if (g_mut == 15)
        bonus = 0;
    if (S8(NA_(o, 0x51)) >= 9)
        return 0;
    T = S8(A_(r, 0x0E + o));
    arch = D8(NA_(o, 0x31)) != 0;
    after = ORD_(n) > ORD_(o);
    before = ORD_(n) < ORD_(o);
    if ((D8(TR_(t, 1)) & 0x40) && x == 0 && !(D8(TS_(t, 2)) & 1)) {
        if (!arch) {
            if (!(T >= 6 && after)) {
                if (T < 4 || ATT_(n, o) <= 300)
                    return 0;
                {
                    int e = ERA_(n, S8(NA_(n, 0x5C)));
                    if (e != 3 && e != 4)
                        return 0;
                }
                if (T < __wrap_f78D08(t, o) || !before || aimove_test_f337C8(o) <= 0)
                    return 0;
            }
            if (T > mode + 6 + bonus)
                return 0;
            if (T >= 4 && T >= __wrap_f78D08(t, o) && before) {
                int s, v;
                aimove_test_f48028(&lc, ls, t, o, 12);
                s = aimove_test_f4D110(t, &lc, ls, o, 1);
                v = s + T;
                if (v > 9)
                    v = 9;
                *need = (v > 6 ? v - 6 : 0) + 2 - bonus;
            } else {
                *need = T - 4 - bonus;
            }
            return 1;
        }
        if (T <= __wrap_f78D08(t, o) || !after || T < 3 || T > mode + (g_mut == 1 ? 4 : 3) + bonus
            || T + S8(NA_(o, 0x3E)) < 6)
            return 0;
        *need = T - 1 - bonus;
        return 1;
    }
    if (!arch) {
        if (T < 12 || !after || T > mode + 12 + bonus)
            return 0;
        *need = T - 10 - bonus;
        return 1;
    }
    if (x != 0 || T < 6 || S8(NA_(o, 0x51)) >= 9 || !after || T > mode + 6 + bonus || T + S8(NA_(o, 0x3E)) < 12)
        return 0;
    *need = T - 4 - bonus;
    return 1;
}

/* the gathering of 0x37CF8 / 0x396FC */
static void m_gather(int n, int t, int32_t *need, int lc, const int32_t *ls, int code0)
{
    int j, s, lim, sweep;

    for (j = 0; j < lc && *need > 0; j++) {
        s = ls[j];
        lim = __wrap_f78D08(s, n);
        while (*need > 0 && S8(A_(s + 1, 0x24 + n)) > 0 && S8(A_(s + 1, 0x0E + n)) > lim) {
            __wrap_f57CA4(s, t, n, 1, 1);
            m_rec(s, t, 1, code0);
            --*need;
        }
    }
    if (lc != 0)
        for (sweep = 0, j = 0; *need > 0 && sweep < 10;) {
            s = ls[j];
            if (S8(A_(s + 1, 0x24 + n)) > 0 && S8(A_(s + 1, 0x0E + n)) > (g_mut == 4 ? 0 : 1)) {
                __wrap_f57CA4(s, t, n, 1, 1);
                m_rec(s, t, 1, code0 + 1);
                --*need;
            }
            if (++j == lc) {
                j = 0;
                sweep++;
            }
        }
    if (lc != 0)
        for (sweep = 0, j = 0; *need > 0 && sweep < 10;) {
            int32_t keep = S32(0xF5908u), ok = 0;
            s = ls[j];
            setD32(0xF5908u, 0);
            if (S8(A_(s + 1, 0x24 + n)) > 0) {
                if (aimove_test_f49A1C(s, n, 1, t))
                    ok = 1;
                else if (!(D8(TR_(t, 1)) & 0x40) && D8(NA_(__wrap_f7C7C4(t, n), 0x31)) == 0)
                    ok = 1;
            }
            if (ok) {
                __wrap_f57CA4(s, t, n, 1, 1);
                m_rec(s, t, 1, code0 + 2);
                --*need;
            }
            setD32(0xF5908u, (uint32_t)keep);
            if (++j == lc) {
                j = 0;
                sweep++;
            }
        }
    if (*need > 0 && (D8(TR_(t, 0)) & 2) && S16(0xBB3F0u) != 0)
        aimove_test_f36918(n, t, need, code0 + 3);
}

typedef struct { int t, need, victim; } m_cand;

/* 0x37CF8 */
static void m_37CF8(int n, int x)
{
    int L = LEVEL_(), mode = 1, t, j, i, cnt = 0, k;
    int32_t need, victim, lc, ls[160];
    m_cand cs[160], sorted[160], tmp;

    if (TAB_(7, n) / 300 < 80 + 10 * (2 - L)) {
        mode = 2;
        if (TAB_(8, n) / 300 < 3 * L)
            mode = 3;
    }
    for (t = 0; t < 157; t++) {
        int ok = 0;
        if (!D8(0xEE16Cu + (uint32_t)t) || !m_387C8(t, n, mode, &need, &victim, x))
            continue;
        for (j = 1; j <= land_n(t); j++)
            if (S8(A_(land_at(t, j) + 1, 0x0E + n)) > 0) {
                ok = 1;
                break;
            }
        if (!ok && aimove_test_f4D29C(t, n))
            ok = 1;
        if (ok) {
            cs[cnt].t = t;
            cs[cnt].need = need;
            cs[cnt].victim = victim;
            cnt++;
        }
    }
    if (cnt > 0 && TAB_(9, n) / 300 < 2 * L + 1)
        return;
    /* stable insertion sort, attitude descending (the bubble with > is stable) */
    for (i = 1; i < cnt; i++) {
        tmp = cs[i];
        for (j = i; j > 0; j--) {
            int32_t a = ATT_(n, tmp.victim), b = ATT_(n, cs[j - 1].victim);
            if (g_mut == 2 ? a < b : a > b)
                cs[j] = cs[j - 1];
            else
                break;
        }
        cs[j] = tmp;
    }
    k = 0;
    for (i = 0; i < cnt; i++)
        if (!(D8(TR_(cs[i].t, 1)) & 0x40))
            sorted[k++] = cs[i];
    for (i = 0; i < cnt; i++)
        if (D8(TR_(cs[i].t, 1)) & 0x40)
            sorted[k++] = cs[i];
    for (i = 0; i < k; i++) {
        int v = sorted[i].victim;
        t = sorted[i].t;
        if (ATT_(n, v) < 0) {
            int d = (5 - L) * (S8(NA_(v, 0x9A)) == 3 ? 2 : 1);
            int q = ((g_mut == 3 ? ATT_(n, v) : ATT_(v, n)) - 200) / d;
            if (!(TAB_(9 + t % 4, n) / 300 > q))
                continue;
        }
        __wrap_f7A940();
        __wrap_f7AA1C();
        need = sorted[i].need;
        aimove_test_f48028(&lc, ls, t, n, 9);
        aimove_test_f48D44(&lc, ls, n, t, 1);
        m_gather(n, t, &need, lc, ls, 0x0C);
        if (need != 0) {
            __wrap_f7AC60();
            __wrap_f7AE14();
            continue;
        }
        {
            uint32_t c = D32(LIST1N);
            int o = __wrap_f7C7C4(t, n);
            setD8(LIST1 + 4 * c, 0xC9);
            setD8(LIST1 + 4 * c + 1, (uint8_t)o);
            setD8(LIST1 + 4 * c + 2, 0);
            setD32(LIST1N, c + 1);
        }
        if (g_mut == 5) {
            __wrap_f7AC60();
            __wrap_f7AE14();
        } else {
            __wrap_f7AD24();
            __wrap_f7AD7C();
        }
        break;
    }
}

/* 0x38E54 */
static void m_38E54(int n)
{
    int calls = 0, i, j, s, u, k;
    int32_t lc, ls[160];

    if (S8(NA_(n, 0x98)) < 14 || aimove_test_f39C08(n) >= 9)
        return;
    for (i = 0; i < S8(NA_(n, 0x60)); i++) {
        s = D8(NA_(n, 0x61 + i)) - 1;
        if (D8(TS_(s, 2)) & 2)
            continue;
        aimove_test_f48028(&lc, ls, s, n, 0x14);
        aimove_test_f48D44(&lc, ls, n, s, 0x201);
        for (j = 0; j < lc; j++) {
            int row;
            u = ls[j];
            k = aimove_test_f4CC5C(u, n, 3);
            if (k <= 0 || S8(A_(s + 1, 0x24 + n)) < k)
                continue;
            row = 11 + calls % 9 + (g_mut == 6 ? 1 : 0);
            calls++;
            if (TAB_(row, n) / 300 < 95 - 15 * LEVEL_()) {
                __wrap_f57CA4(s, u, n, 1, k);
                setD8(A_(u + 1, 0x3B + n), 0);
                m_rec(s, u, k, 0x19);
            }
        }
    }
}

typedef struct { int t, value, a, b, idx; } m_city;

/* 0x39068 */
static void m_39068(int n, int x)
{
    int L = LEVEL_(), score = 0, m, best = -9999, who = 0, top = 0, cnt = 0, t, i, j;
    int32_t b = 0;
    m_city cs[160], tmp;

    if (S8(NA_(n, 0x51)) >= 9)
        return;
    if (D8(NA_(n, 0x2A)))
        score += 10;
    score += 15 - 5 * L;
    {
        int ex = S8(NA_(n, 0x98)) - 6 * x - S32(0xF4898u) / 2;
        if (ex > 0)
            score += 5 * ex;
    }
    if (S8(NA_(n, 0x3E)) >= 15 && S8(NA_(n, 0x3E)) <= 17)
        score += 20;
    for (m = 1; m <= 9; m++)
        if (D8(NA_(m, 0x9A)) && ATT_(n, m) > best) {
            best = ATT_(n, m);
            who = m;
        }
    if (best > 0)
        score += 10 * (best / 50 + 1);
    for (m = 1; m <= 9; m++)
        if (D8(NA_(m, 0x9A)) && S8(NA_(m, 0x5C)) > top)
            top = S8(NA_(m, 0x5C));
    if (top > 12)
        top = 12;
    if (TAB_(24, n) / 300 >= score * top / 13)
        return;
    for (t = 0; t < 157; t++) {
        int o, here;
        if (!(D8(TS_(t, 2)) & 1))
            continue;
        o = D8(TS_(t, 0));
        here = S8(A_(t + 1, 1));
        if (here != 0 && (here != 1 || S8(A_(t + 1, 1 + o)) != o))
            continue;
        if (o == n || ATT_(n, o) < -200 || o == 10 || !aimove_test_f49934(t, n, 0))
            continue;
        cs[cnt].t = t;
        cs[cnt].value = ATT_(n, o) * ERAW_(ERA_(o, S8(NA_(o, 0x5C))), L) / 100;
        if (o == who) {
            cs[cnt].a = aimove_test_f4CEE4(t, n, &b);
            cs[cnt].b = b;
        } else {
            cs[cnt].a = cs[cnt].b = 99999;
        }
        cs[cnt].idx = cnt;
        cnt++;
    }
    /* the two stable bubbles = one stable sort: value descending, then a ascending, then the original order */
    for (i = 1; i < cnt; i++) {
        tmp = cs[i];
        for (j = i; j > 0; j--) {
            const m_city *p = &cs[j - 1];
            int before;
            if (g_mut == 8)
                before = tmp.a < p->a || (tmp.a == p->a && tmp.idx < p->idx);
            else
                before = tmp.value > p->value || (tmp.value == p->value && (tmp.a < p->a
                         || (tmp.a == p->a && tmp.idx < p->idx)));
            if (!before)
                break;
            cs[j] = cs[j - 1];
        }
        cs[j] = tmp;
    }
    for (i = 0; i < cnt; i++) {
        int a;
        t = cs[i].t;
        if (cs[i].a == 99999)
            a = aimove_test_f4CEE4(t, n, &b);
        else {
            a = cs[i].a;
            b = cs[i].b;
        }
        if (a == -1)
            continue;
        __wrap_f7A940();
        __wrap_f7AA1C();
        if (aimove_test_f3580C(a, t, n, 2, b) != 0) {
            __wrap_f7AC60();
            __wrap_f7AE14();
            continue;
        }
        __wrap_f7AD24();
        __wrap_f7AD7C();
        setD8(A_(t + 1, 0x3B + (g_mut == 7 ? n : 0)), D8(A_(t + 1, 0x0E + n)));
        break;
    }
}

/* 0x396FC */
static void m_396FC(int n)
{
    int k, t, o, base, bonus;
    int32_t need, lc, ls[160];

    for (k = 0; k < S8(NA_(n, 0x51)); k++) {
        t = D8(NA_(n, 0x52 + k)) - 1;
        if (S8(A_(t + 1, 1)) == 0)
            continue;
        o = __wrap_f7C7C4(t, n);
        base = 7;
        if (S8(NA_(n, 0x2A)) == 1 && S8(NA_(o, 0x2A)) == 0)
            base = g_mut == 9 ? 6 : 8;
        else if (S8(NA_(n, 0x2A)) == 0 && S8(NA_(o, 0x2A)) == 1)
            base = g_mut == 9 ? 8 : 6;
        bonus = D8(NA_(n, 0x26)) && !D8(NA_(o, 0x26));
        need = S8(A_(t + 1, 0x0E + o)) - base + 2 - bonus;
        __wrap_f7A940();
        __wrap_f7AA1C();
        aimove_test_f48028(&lc, ls, t, n, 9);
        aimove_test_f48D44(&lc, ls, n, t, 1);
        m_gather(n, t, &need, lc, ls, 0x24);
        if (need) {
            __wrap_f7AC60();
            __wrap_f7AE14();
        } else {
            __wrap_f7AD24();
            __wrap_f7AD7C();
        }
    }
}

/* 0x461BC */
static void m_461BC(int n)
{
    int r, m, goal, reserve, cnt_on, again = 1;
    int32_t cnt, flag;
    uint32_t depth = D32(0x107D44u);

    setD32(0x107D48u + 4 * depth, (uint32_t)n);
    setD32(0x107D44u, depth + 1);
    aimove_test_f7BD5C(n);
    aimove_test_f46998(n);
    aimove_test_f160EC(n);
    f3F6E4();
    f3F758();
    __wrap_f46158(n);
    for (r = 1; r <= 160; r++) {
        setD8(A_(r, 0x3B), 0);
        if (g_mut != 14) {
            setD8(A_(r, 0x37), 0);
            setD8(A_(r, 0x38), 0);
        }
        if (S8(A_(r, 1 + n)) != 0) {
            setD8(A_(r, 0x24 + n), D8(A_(r, 0x0E + n)));
            __wrap_f3F698(r - 1, 15);
        } else {
            setD8(A_(r, 0x24 + n), 0);
        }
    }
    __wrap_f7A940();
    goal = aimove_test_f337C8(n);
    __wrap_f7A940();
    f5AAA4(0);
    f5C238(0);
    {
        int tok = S8(NA_(n, 0x98)), cit = S8(NA_(n, 0x51));
        reserve = cit + goal >= (tok - 1) / 2 ? 0 : tok - 2 - 2 * (cit + goal);
    }
    __wrap_f7AC60();
    while (again) {
        int worst = -99999, any = 0, era, lvl, before_cnt, v;
        setD32(LIST1N, 0);
        setD32(LIST2N, 0);
        cnt = 0;
        flag = 0;
        setD32(0x108194u, 0);
        setD32(0x107EE0u, 0);
        setD32(0x107EDCu, 0);
        setD8(0x107ED8u, 0);
        setD32(0x108164u + 4u * (uint32_t)n, 0);
        memset(DP(0xF8558u + 160u * (uint32_t)(n - 1)), 0, 160);
        __wrap_f7AF28();
        setD8(NA_(n, 0x5D), (uint8_t)reserve);
        m_37CF8(n, 1);
        m_396FC(n);
        if (S8(NA_(n, 0x5D)) == 0) {
            aimove_test_f4A088(n, 0, 1);
            aimove_test_f4C074(n);
            __wrap_f7A940();
            f5AAA4(0);
            f5C238(0);
            reserve = S8(NA_(n, 0x98)) - 2 - 2 * (S8(NA_(n, 0x51)) + goal);
            if (reserve < 0)
                reserve = 0;
            __wrap_f7AC60();
            flag = 1;
        }
        setD8(NA_(n, 0x5D), (uint8_t)reserve);
        m = 0;
        {
            int k;
            for (k = 1; k <= 9; k++) {
                if (ATT_(n, k) > worst) {
                    worst = ATT_(n, k);
                    m = k;
                }
                if (ATT_(n, k) < 0)
                    any = 1;
            }
        }
        era = ERA_(n, S8(NA_(m, 0x5C)));
        if (S8(NA_(m, 0x9A)) == 3) {
            worst += 200;
            era++;
        }
        lvl = LEVEL_();
        cnt_on = any && worst * ERAW_(era, lvl) / 100 > 500;
        if (!cnt_on)
            cnt_on = aimove_test_f468EC();
        if (cnt_on || g_mut == 12) {
            setD32(0x107D40u, 0);
            m_39068(n, cnt);
            aimove_test_f33964(goal, &cnt, n, 0);
        } else {
            aimove_test_f33964(goal, &cnt, n, 0);
            setD32(0x107D40u, 0);
            m_39068(n, cnt);
        }
        aimove_test_f4C9B8(n, 1);
        m_38E54(n);
        before_cnt = S8(NA_(n, g_mut == 11 ? 0x5D : 0x60));
        aimove_test_f46BB8(n);
        if (S8(NA_(n, g_mut == 11 ? 0x5D : 0x60)) == before_cnt && D8(NA_(n, 0x40)) == 0 && S8(NA_(n, 0x51)) == 0
            && S8(NA_(n, 0x98)) >= 12)
            aimove_test_f33964(goal + 1, &cnt, n, 0);
        aimove_test_f4C9B8(n, 2);
        m_37CF8(n, 0);
        aimove_test_f4C6E4(n);
        setD8(0x107ED8u, 0);
        aimove_test_f478D8(goal, &cnt, n, &flag);
        if (cnt < 4)
            aimove_test_f47A44(goal, &cnt, n, &flag);
        if (!__wrap_f46B34(n)) {
            if (LEVEL_() == 2)
                aimove_test_f648C8(n, 0);
            else
                aimove_test_f643C8(n);
        }
        __wrap_f7A940();
        __wrap_f7AA1C();
        aimove_test_f4A088(n, 0, 0);
        aimove_test_f49E04(n);
        v = __wrap_f46A4C(n);
        if (v > 0) {
            int v2;
            __wrap_f7A688();
            __wrap_f7A7E8();
            __wrap_f7AF28();
            __wrap_f7AFB8();
            aimove_test_f49E04(n);
            aimove_test_f4A088(n, 0, 0);
            v2 = __wrap_f46A4C(n);
            if (g_mut == 10 ? v2 < v : v2 >= v) {
                __wrap_f7A700();
                __wrap_f7A85C();
            }
        }
        __wrap_f7AD24();
        __wrap_f7AD7C();
        aimove_test_f4C074(n);
        setD8(0xC28ACu, 0);
        aimove_test_f46BB8(n);
        setD8(0xC28ACu, 1);
        setD32(0xC28B0u, 3);
        aimove_test_f4A088(n, 0, 1);
        setD32(0xC28B0u, 4);
        aimove_test_f648C8(n, 0);
        aimove_test_f4BBDC(n);
        if (S32(0xF5908u))
            aimove_test_f65CB0(n);
        again = 0;
        if (cnt > 0) {
            int marked = 0, i;
            for (i = 0; i < S8(NA_(n, 0x60)); i++)
                if (D8(TS_(D8(NA_(n, 0x61 + i)) - 1, 2)) & (g_mut == 13 ? 1 : 2))
                    marked++;
            f5AAA4(0);
            f5C238(0);
            if (S8(NA_(n, 0x98)) - 2 < 2 * (S8(NA_(n, 0x51)) + marked)) {
                goal--;
                again = 1;
            }
        }
    }
    __wrap_f7AC60();
    if (S32(0xF5908u))
        f78A20(500);
    f4D518(n, S32(0xF5908u) != 0);
    setD32(0x107D44u, D32(0x107D44u) - 1);
}

/* ================================================================================================================== */
/* ---- states -------------------------------------------------------------------------------------------------------- */
/* ================================================================================================================== */

static uint8_t *g_base[2];
static int g_nbase;

static void add_tokens(int t, int m, int k)
{
    f57E28(t, m, k);
    setD8(NA_(m, 0x98), (uint8_t)(S8(NA_(m, 0x98)) + k));
}

static int in_play_nation(int m) { return m >= 1 && m <= 9 && D8(NA_(m, 0x9A)) != 0; }

static int g_att_t[16], g_natt;   /* the territories setup_attacks prepared */

static void clear_area(int r)
{
    int m;
    for (m = 1; m <= 13; m++)
        if (S8(A_(r, 0x0E + m)) > 0) {
            setD8(NA_(m, 0x98), (uint8_t)(S8(NA_(m, 0x98)) - S8(A_(r, 0x0E + m))));
            f57CE8(r - 1, m, 0, S8(A_(r, 0x0E + m)));
        }
}

static void setup_attacks(int n)
{
    int k, i, cntn, s, u, o, T, tries;

    g_natt = 0;
    for (k = 1 + grnd(5); k > 0; k--)
        for (tries = 0; tries < 10; tries++) {
            cntn = S8(NA_(n, 0x60));
            if (cntn <= 0)
                return;
            s = D8(NA_(n, 0x61 + grnd(cntn))) - 1;
            if (s < 0 || s >= 157 || land_n(s) <= 0)
                continue;
            u = land_at(s, 1 + grnd(land_n(s)));
            if (u < 0 || u >= 157 || !D8(0xEE16Cu + (uint32_t)u) || S8(A_(u + 1, 0x0E + n)) > 0)
                continue;
            clear_area(u + 1);
            do
                o = grnd(6) == 0 ? 10 + grnd(2) : 1 + grnd(9);
            while (o == n);
            T = grnd(2) ? 3 + grnd(6) : 10 + grnd(7);
            add_tokens(u, o, T);
            if (grnd(3) == 0 && S8(NA_(o, 0x51)) < 9 && !(D8(TS_(u, 2)) & 1))
                f78E74(u, o);
            if (grnd(2))
                setD8(0xF06C0u + (uint32_t)n, (uint8_t)(D8(0xF06C0u + (uint32_t)o) + 1));
            if (grnd(2))
                setD32(0xF52E0u + 40u * (uint32_t)n + 4u * (uint32_t)o,
                       (uint32_t)(grnd(3) == 0 ? -grnd(400) : 301 + grnd(500)));
            if (g_natt < 16)
                g_att_t[g_natt++] = u;
            break;
        }
    /* foreign tokens in n's own cities */
    for (i = 0; i < S8(NA_(n, 0x51)) && i < 9; i++)
        if (grnd(2)) {
            int t = D8(NA_(n, 0x52 + i)) - 1;
            if (t < 0 || t >= 157)
                continue;
            do
                o = 1 + grnd(11);
            while (o == n);
            add_tokens(t, o, 1 + grnd(9));
        }
}

/* a random state from base b; returns the nation to plan for */
static int make_state(int b)
{
    int i, m, t, n, k;

    memcpy(g_ds, g_base[b], DS_SIZE);
    for (k = 0, n = 0; k < 50 && n == 0; k++) {
        m = 1 + grnd(9);
        if (in_play_nation(m))
            n = m;
    }
    if (n == 0)
        n = 1;
    /* the AI table, the attitudes, the AI level, the turn order */
    for (i = 0; i < 300; i++)
        setD16(0x112A94u + 2u * (uint32_t)i, (uint16_t)grnd(30000));
    for (i = 1; i <= 9; i++)
        for (m = 1; m <= 9; m++)
            setD32(0xF52E0u + 40u * (uint32_t)i + 4u * (uint32_t)m,
                   (uint32_t)(i == m ? 0 : grnd(5) == 0 ? -(grnd(900)) : grnd(1300) - 300));
    if (grnd(6) == 0)
        for (m = 1; m <= 9; m++)
            if (m != n)
                setD32(0xF52E0u + 40u * (uint32_t)n + 4u * (uint32_t)m, (uint32_t)-(1 + grnd(800)));
    setD16(0xE1624u, (uint16_t)((D16(0xE1624u) & ~0x0C00u) | (uint16_t)((grnd(7) == 0 ? 3 : grnd(3)) << 10)));
    for (m = 0; m <= 13; m++)
        setD8(0xF06C0u + (uint32_t)m, (uint8_t)grnd(10));
    setD32(0xF4898u, (uint32_t)(20 + grnd(36)));
    setD32(0xF5908u, (uint32_t)grnd(2));
    setD16(0xBB3F0u, (uint16_t)grnd(2));
    for (m = 1; m <= 11; m++) {
        setD8(NA_(m, 0x26), (uint8_t)(grnd(3) == 0));
        setD8(NA_(m, 0x2A), (uint8_t)(grnd(5) == 0 ? 2 : grnd(2)));
        setD8(NA_(m, 0x31), (uint8_t)(grnd(3) == 0));
        setD8(NA_(m, 0x3E), (uint8_t)grnd(21));
        setD8(NA_(m, 0x5C), (uint8_t)grnd(17));
        if (in_play_nation(m) && m != n && grnd(3) == 0)
            setD8(NA_(m, 0x9A), 3);
        if (grnd(8) == 0 && m != n)
            setD8(NA_(m, 0x51), 9);
    }
    setD8(NA_(n, 0x9A), 2);
    setD8(NA_(n, 0x40), (uint8_t)(grnd(3) == 0 ? 0 : grnd(5)));
    /* tokens */
    for (i = grnd(60); i > 0; i--) {
        t = grnd(157);
        m = grnd(3) == 0 ? n : 1 + grnd(11);
        add_tokens(t, m, 1 + grnd(grnd(4) == 0 ? 14 : 5));
    }
    /* n near its own territories' neighbours, so the gathering has sources */
    for (i = grnd(12); i > 0; i--) {
        int cntn = S8(NA_(n, 0x60));
        if (cntn <= 0)
            break;
        t = D8(NA_(n, 0x61 + grnd(cntn))) - 1;
        if (t >= 0 && t < 157 && land_n(t) > 0) {
            int u = land_at(t, 1 + grnd(land_n(t)));
            if (u >= 0 && u < 157)
                add_tokens(u, 1 + grnd(11), 1 + grnd(10));
        }
    }
    /* territories next to n's held by one other nation (the attack candidates), foreign tokens in n's cities */
    setup_attacks(n);
    /* cities and site marks */
    for (i = grnd(8); i > 0; i--) {
        t = grnd(157);
        if (!(D8(TS_(t, 2)) & 1)) {
            m = grnd(4) == 0 ? n : 1 + grnd(11);
            if (S8(NA_(m, 0x51)) < 9)
                f78E74(t, m);
        }
    }
    for (i = grnd(10); i > 0; i--) {
        t = grnd(157);
        setD8(TS_(t, 2), (uint8_t)(D8(TS_(t, 2)) ^ 2));
    }
    for (i = 0; i < S8(NA_(n, 0x60)); i++)   /* AI site marks on n's own territories */
        if (grnd(3) == 0) {
            t = D8(NA_(n, 0x61 + i)) - 1;
            if (t >= 0 && t < 157)
                setD8(TS_(t, 2), (uint8_t)(D8(TS_(t, 2)) | 2));
        }
    for (i = 1; i <= 160; i++)   /* stale bytes the planner clears or writes */
        if (grnd(4) == 0) {
            setD8(A_(i, 0x3B), (uint8_t)grnd(256));
            setD8(A_(i, 0x37), (uint8_t)grnd(256));
            setD8(A_(i, 0x38), (uint8_t)grnd(256));
        }
    if (grnd(3) == 0)
        setD8(NA_(n, 0x98), (uint8_t)(10 + grnd(30)));
    /* movable counts: all, then random cuts */
    for (m = 1; m <= 9; m++)
        f460C0(m);
    for (i = grnd(20); i > 0; i--) {
        int r = 1 + grnd(157);
        if (S8(A_(r, 0x24 + n)) > 0)
            setD8(A_(r, 0x24 + n), (uint8_t)grnd(S8(A_(r, 0x24 + n)) + 1));
    }
    /* stacks empty, move lists empty */
    setD32(WSD, 0xFFFFFFFFu);
    setD32(MLD, 0xFFFFFFFFu);
    setD32(LIST1N, 0);
    setD32(LIST2N, 0);
    setD32(0x107D44u, (uint32_t)grnd(3));
    return n;
}

/* ---- running a routine on both sides ------------------------------------------------------------------------------ */

enum { R_387C8, R_37CF8, R_396FC, R_38E54, R_39068, R_461BC, R_N };
static const char *const k_rname[R_N] = { "f387C8", "f37CF8", "f396FC", "f38E54", "f39068", "f461BC" };

typedef struct { int which, n, t, mode, x; } job_t;
static int32_t g_out[2][3];   /* f387C8's return, *need, *victim */

static void run_one(const job_t *j)
{
    int32_t need = -777, victim = -777, ret = -777;
    volatile int quit = 0;

    if (setjmp(g_jb) != 0) {
        quit = 1;
    } else if (g_side == 0) {
        switch (j->which) {
        case R_387C8: ret = f387C8(j->t, j->n, j->mode, &need, &victim, j->x); break;
        case R_37CF8: f37CF8(j->n, j->x); break;
        case R_396FC: f396FC(j->n); break;
        case R_38E54: f38E54(j->n); break;
        case R_39068: f39068(j->n, j->x); break;
        default: f461BC(j->n); break;
        }
    } else {
        switch (j->which) {
        case R_387C8: ret = m_387C8(j->t, j->n, j->mode, &need, &victim, j->x); break;
        case R_37CF8: m_37CF8(j->n, j->x); break;
        case R_396FC: m_396FC(j->n); break;
        case R_38E54: m_38E54(j->n); break;
        case R_39068: m_39068(j->n, j->x); break;
        default: m_461BC(j->n); break;
        }
    }
    if (quit)
        tr("QUITTED");
    g_out[g_side][0] = ret;
    g_out[g_side][1] = need;
    g_out[g_side][2] = victim;
}

static void reset_side(int s, uint32_t seed)
{
    g_sd[s].len = 0;
    if (g_sd[s].buf)
        g_sd[s].buf[0] = 0;
    g_sd[s].s = seed;
    g_sd[s].used = 0;
}

static int g_show_diff = 1;

/* state in g_ds (the port's); runs the port, then the model on a copy; 1 if they agree */
static int compare_job(const job_t *j, uint32_t seed, const char *what)
{
    int ok = 1;
    size_t i;
    char buf[200];

    memcpy(m_ds, g_ds, DS_SIZE);
    reset_side(0, seed);
    reset_side(1, seed);
    g_side = 0;
    run_one(j);
    g_port_ds = g_ds;
    g_ds = m_ds;
    g_side = 1;
    run_one(j);
    g_ds = g_port_ds;
    g_side = 0;

    if (memcmp(g_ds, m_ds, DS_SIZE) != 0) {
        ok = 0;
        if (g_show_diff && !g_quiet_fail) {
            for (i = 0; i < DS_SIZE && g_ds[i] == m_ds[i]; i++)
                ;
            printf("   %s: data differ first at 0x%X (port %02X, model %02X)\n", what, (unsigned)(i + DS_BASE),
                   g_ds[i], m_ds[i]);
        }
    }
    if (g_sd[0].len != g_sd[1].len || (g_sd[0].len && memcmp(g_sd[0].buf, g_sd[1].buf, g_sd[0].len) != 0)) {
        ok = 0;
        if (g_show_diff && !g_quiet_fail) {
            size_t a = 0;
            while (a < g_sd[0].len && a < g_sd[1].len && g_sd[0].buf[a] == g_sd[1].buf[a])
                a++;
            printf("   %s: traces differ at %u:\n     port  ...%.160s\n     model ...%.160s\n", what, (unsigned)a,
                   g_sd[0].buf + (a > 60 ? a - 60 : 0), g_sd[1].buf + (a > 60 ? a - 60 : 0));
        }
    }
    if (g_sd[0].used != g_sd[1].used)
        ok = 0;
    if (j->which == R_387C8 && (g_out[0][0] != g_out[1][0] || g_out[0][1] != g_out[1][1] || g_out[0][2] != g_out[1][2]))
        ok = 0;
    snprintf(buf, sizeof buf, "%s %s", what, k_rname[j->which]);
    (void)buf;
    return ok;
}

/* ---- A. random differential runs ----------------------------------------------------------------------------------- */

static int g_stat_rec, g_stat_c9, g_stat_drop, g_stat_np, g_stat_ret1;

static int part_a(int runs, int quiet)
{
    int i, bad = 0;
    char what[120];

    for (i = 0; i < runs; i++) {
        job_t j;
        int b = grnd(g_nbase);
        j.n = make_state(b);
        j.which = grnd(10) < 3 ? R_387C8 : grnd(R_N - 1) + 1;
        if (j.which == R_461BC && grnd(2))
            j.which = 1 + grnd(4);
        j.t = g_natt > 0 && grnd(4) != 0 ? g_att_t[grnd(g_natt)] : grnd(157);
        j.mode = 1 + grnd(3);
        j.x = j.which == R_39068 ? grnd(5) : grnd(2);
        snprintf(what, sizeof what, "A run %d (%s, base %d, n %d)", i, k_rname[j.which], b, j.n);
        if (!compare_job(&j, 777u + (uint32_t)i * 7919u, what)) {
            bad++;
            if (!quiet)
                check(0, what);
        } else if (!quiet) {
            check(1, what);
            if (strstr(g_sd[0].buf ? g_sd[0].buf : "", "57CA4"))
                g_stat_rec++;
            {
                uint32_t q, c = D32(LIST1N);
                for (q = 0; q < c && q < 200; q++)
                    if (D8(LIST1 + 4 * q) == 0xC9) {
                        g_stat_c9++;
                        break;
                    }
            }
            if (strstr(g_sd[0].buf ? g_sd[0].buf : "", "WSdrop"))
                g_stat_drop++;
            if (strstr(g_sd[0].buf ? g_sd[0].buf : "", "QUIT") || strstr(g_sd[0].buf ? g_sd[0].buf : "", "NP("))
                g_stat_np++;
            if (j.which == R_387C8 && g_out[0][0] == 1)
                g_stat_ret1++;
        }
    }
    return bad;
}

/* ---- B. hand vectors ------------------------------------------------------------------------------------------------ */

static void clear_area_of(int r)
{
    int m;
    for (m = 1; m <= 13; m++)
        if (S8(A_(r, 0x0E + m)) > 0)
            f57CE8(r - 1, m, 0, S8(A_(r, 0x0E + m)));
}

/* a territory t < 157 in play, with or without the city-site bit, without a city */
static int find_terr(int site)
{
    int t;
    for (t = 3; t < 157; t++)
        if (D8(0xEE16Cu + (uint32_t)t) && (((D8(TR_(t, 1)) & 0x40) != 0) == site) && !(D8(TS_(t, 2)) & 3)
            && land_n(t) > 0)
            return t;
    return -1;
}

static void part_b(void)
{
    int n = 1, o = 2, t, r, ret, lim;
    int32_t need, victim;
    char buf[200];

    /* B1: f387C8, territory without a city site */
    memcpy(g_ds, g_base[0], DS_SIZE);
    t = find_terr(0);
    check(t >= 0, "B1 a non-site territory in play");
    if (t < 0)
        return;
    r = t + 1;
    clear_area_of(r);
    setD8(NA_(n, 0x26), 0);
    setD8(NA_(o, 0x26), 0);
    setD8(NA_(o, 0x51), 2);
    setD8(NA_(o, 0x31), 0);
    setD8(0xF06C0u + 1, 5);   /* n moves after o */
    setD8(0xF06C0u + 2, 3);
    add_tokens(t, o, 13);
    g_trace_on = 0;
    ret = f387C8(t, n, 1, &need, &victim, 0);
    snprintf(buf, sizeof buf, "B1 no site, no Architecture, 13 tokens, mode 1: ret %d need %d victim %d (1, 3, 2)", ret,
             need, victim);
    check(ret == 1 && need == 3 && victim == 2, buf);
    add_tokens(t, o, 1);   /* 14 > 1 + 12 */
    ret = f387C8(t, n, 1, &need, &victim, 0);
    check(ret == 0, "B1 14 tokens > mode + 12: no attack");
    ret = f387C8(t, n, 2, &need, &victim, 0);
    check(ret == 1 && need == 4, "B1 14 tokens, mode 2: need 4");
    setD8(NA_(n, 0x26), 1);   /* Metalworking against none: bonus 1 */
    ret = f387C8(t, n, 1, &need, &victim, 0);
    check(ret == 1 && need == 3, "B1 Metalworking bonus: 14 <= 14, need 14 - 10 - 1 = 3");
    setD8(0xF06C0u + 1, 2);   /* n moves before o */
    ret = f387C8(t, n, 2, &need, &victim, 0);
    check(ret == 0, "B1 n moves earlier: no attack");
    /* Architecture, second pass and first */
    setD8(0xF06C0u + 1, 5);
    setD8(NA_(n, 0x26), 0);
    setD8(NA_(o, 0x31), 1);
    f57CE8(t, o, 0, 7);   /* 7 tokens */
    setD8(NA_(o, 0x3E), 5);
    ret = f387C8(t, n, 1, &need, &victim, 0);
    check(ret == 1 && need == 3, "B1 Architecture: 7 tokens + 5 treasury >= 12, need 3");
    ret = f387C8(t, n, 1, &need, &victim, 1);
    check(ret == 0, "B1 Architecture, x = 1: no attack");
    setD8(NA_(o, 0x3E), 4);
    ret = f387C8(t, n, 1, &need, &victim, 0);
    check(ret == 0, "B1 Architecture: 7 + 4 < 12");
    setD8(NA_(o, 0x51), 9);
    ret = f387C8(t, n, 1, &need, &victim, 0);
    check(ret == 0 && victim == 2, "B1 victim with 9 cities: 0, *victim already written");
    /* two nations there */
    setD8(NA_(o, 0x51), 2);
    add_tokens(t, 3, 1);
    ret = f387C8(t, n, 1, &need, &victim, 0);
    check(ret == 0, "B1 two nations: no attack");

    /* B2: a city site without a city, first pass, no Architecture */
    memcpy(g_ds, g_base[0], DS_SIZE);
    t = find_terr(1);
    check(t >= 0, "B2 a site territory in play");
    if (t >= 0) {
        r = t + 1;
        clear_area_of(r);
        lim = (D8(TR_(t, 1)) >> 3) & 7;
        setD8(NA_(n, 0x26), 0);
        setD8(NA_(o, 0x26), 0);
        setD8(NA_(o, 0x51), 2);
        setD8(NA_(o, 0x31), 0);
        setD8(0xF06C0u + 1, 5);
        setD8(0xF06C0u + 2, 3);
        add_tokens(t, o, 7);   /* >= 6 and n later: straight to 0x38A6C; 7 > 1 + 6? no: 7 <= 7 */
        ret = f387C8(t, n, 1, &need, &victim, 0);
        snprintf(buf, sizeof buf, "B2 site, 7 tokens, n later, mode 1: ret %d need %d (1, 3)", ret, need);
        check(ret == 1 && need == 3, buf);   /* n later: not "before", so need = 7 - 4 */
        ret = f387C8(t, n, 1, &need, &victim, 1);
        check(ret == 0, "B2 the same with x = 1 (the city rule): 7 < 12");
        /* n earlier, 5 tokens, attitude > 300, era 3 or 4 of n's AST square */
        f57CE8(t, o, 0, 2);
        setD8(0xF06C0u + 1, 1);
        setD32(0xF52E0u + 40u * 1 + 4u * 2, 400);
        setD8(NA_(n, 0x5C), 12);   /* row 0, square 12: byte 60 of 0xBCCE0 = 3 (obj2) */
        check(ERA_(1, 12) == 3, "B2 era of Africa's square 12 is 3");
        g_trace_on = 1;
        reset_side(0, 5);   /* f337C8 = rnd(4) with seed 5 */
        g_side = 0;
        ret = f387C8(t, n, 1, &need, &victim, 0);
        {
            uint32_t s = 5u * 1103515245u + 12345u;
            int r337 = (int)((s >> 16) % 4u);
            int expect;
            lim = __real_f78D08(t, o);
            if (5 < lim || r337 <= 0)
                expect = 0;
            else
                expect = 1;
            snprintf(buf, sizeof buf, "B2 n earlier, 5 tokens (limit %d, f337C8 %d): ret %d (%d)", lim, r337, ret,
                     expect);
            check(ret == expect, buf);
            if (ret == 1) {
                /* 5 >= 4, >= limit, n earlier: f48028 / f4D110 path; f4D110 = the next draw */
                check(strstr(g_sd[0].buf, "48028(") != NULL && strstr(g_sd[0].buf, "4D110(") != NULL,
                      "B2 the reinforcement path asks f48028 and f4D110");
            }
        }
        g_trace_on = 0;
    }

    /* B3: f461BC's entry and its balance */
    {
        job_t j;
        int bad = 0, badm = 0, badbox = 0, rr;
        memcpy(g_ds, g_base[0], DS_SIZE);
        n = 0;
        for (rr = 1; rr <= 9 && n == 0; rr++)
            if (D8(NA_(rr, 0x9A)) == 2)
                n = rr;
        if (n == 0)
            n = 1;
        setD32(WSD, 0xFFFFFFFFu);
        setD32(MLD, 0xFFFFFFFFu);
        setD32(0x107D44u, 1);
        setD32(0xF5908u, 1);
        for (rr = 1; rr <= 160; rr++) {
            setD8(A_(rr, 0x3B), 0x5A);
            setD8(A_(rr, 0x37), 0x5A);
            setD8(A_(rr, 0x38), 0x5A);
        }
        memset(DP(0xFFC00u), 7, 0xA0);
        g_trace_on = 1;
        reset_side(0, 99);
        g_side = 0;
        j.which = R_461BC;
        j.n = n;
        j.t = j.mode = j.x = 0;
        run_one(&j);
        /* the final pop puts back the state of the first push, taken after the area loop */
        for (rr = 1; rr <= 160; rr++) {
            int here = S8(A_(rr, 1 + n)) != 0;
            if (D8(A_(rr, 0x3B)) != 0 || D8(A_(rr, 0x37)) != 0 || D8(A_(rr, 0x38)) != 0)
                bad++;
            if (D8(A_(rr, 0x24 + n)) != (here ? D8(A_(rr, 0x0E + n)) : 0))
                badm++;
            if (D8(0xFFC00u + (uint32_t)(rr - 1)) != (here ? 15 : 0))
                badbox++;
        }
        check(bad == 0, "B3 f461BC: +0x3B and int16 +0x37 cleared in every area 1..160");
        check(badm == 0, "B3 f461BC: +0x24 + n = n's tokens where n is, else 0");
        check(badbox == 0, "B3 f461BC: token box colour 15 exactly where n has tokens");
        check(D32(0x107D44u) == 1 && D32(0x107D48u + 4u) == (uint32_t)n, "B3 the nation stack: pushed n, depth back");
        check(S32(WSD) == -1 && S32(MLD) == -1, "B3 both stacks balanced");
        {
            const char *s = g_sd[0].buf;
            const char *p1 = strstr(s, "7BD5C("), *p2 = strstr(s, "46998("), *p3 = strstr(s, "160EC("),
                       *p4 = strstr(s, "3F6E4"), *p5 = strstr(s, "3F758"), *p6 = strstr(s, "46158("),
                       *p7 = strstr(s, "WSpush 337C8("), *p8 = strstr(s, "WSpush 5AAA4(0) "),
                       *p9 = strstr(s, "WSpeek"), *pa = strstr(s, "78A20(500)"), *pb = strstr(s, "4D518(");
            check(p1 && p2 && p3 && p4 && p5 && p6 && p7 && p8 && p9 && p1 < p2 && p2 < p3 && p3 < p4 && p4 < p5
                  && p5 < p6 && p6 < p7 && p7 < p8 && p8 < p9, "B3 f461BC's first calls in the listing's order");
            check(pa && pb && pa < pb && strstr(pb, "4D518(") == pb && strstr(s, "4D518(") == pb,
                  "B3 shown: the 500 ms wait, then f4D518(n, 1) once");
            {
                const char *q = strstr(s, "4D518(");
                char want[32];
                snprintf(want, sizeof want, "4D518(%d,1)", n);
                check(q && strncmp(q, want, strlen(want)) == 0, "B3 f4D518 gets show = 1");
            }
        }
        /* every area where n has tokens got the 15 box, in ascending order, from f3F698 */
        {
            int prev = -1, okord = 1, cntbox = 0;
            const char *p = g_sd[0].buf, *end = strstr(g_sd[0].buf, "WSpush");
            while ((p = strstr(p, "3F698(")) != NULL && (end == NULL || p < end)) {
                int tt = atoi(p + 6);
                if (tt <= prev || strncmp(strchr(p, ',') ? strchr(p, ',') : p, ",15)", 4) != 0)
                    okord = 0;
                prev = tt;
                cntbox++;
                p += 6;
            }
            check(okord && cntbox > 0, "B3 the token boxes (15) of n's areas in ascending order, before the first push");
        }
        g_trace_on = 1;
    }
}

/* ---- C. the saves ---------------------------------------------------------------------------------------------------- */

static int load_save(const char *path, char *desc)
{
    int16_t ver;
    FILE *fp = fopen(path, "rb");
    if (fp == NULL)
        return -1;
    f5F05C(&ver, desc, fp);
    fclose(fp);
    return ver;
}

static void part_c(int b, const char *path)
{
    int m;
    char what[200];
    for (m = 1; m <= 9; m++) {
        job_t j;
        if (D8(NA_(m, 0x9A)) != 2)
            continue;
        memcpy(g_ds, g_base[b], DS_SIZE);
        setD32(WSD, 0xFFFFFFFFu);
        setD32(MLD, 0xFFFFFFFFu);
        j.which = R_461BC;
        j.n = m;
        j.t = j.mode = j.x = 0;
        snprintf(what, sizeof what, "C %s: f461BC(%d) port = model", path, m);
        check(compare_job(&j, 4242u + (uint32_t)m, what), what);
        printf("   %s f461BC(%d): %u land records, %u ship records, trace %u bytes, %d draws\n", path, m,
               (unsigned)D32(LIST1N), (unsigned)D32(LIST2N), (unsigned)g_sd[0].len, g_sd[0].used);
    }
}

/* ---- the land lists ----------------------------------------------------------------------------------------------- */

static uint32_t g_handles[160];
static int g_lists_built;

/* LANDADJ.DAT into one heap block, the neighbours not in play dropped from every list (as 0x3EED8 does, by the
   in-play bytes 0xEE16C already set) */
static int load_landadj(const char *gamedir)
{
    char path[600];
    FILE *fp;
    long size;
    uint8_t *buf;
    uint32_t h, off = 0;
    uint8_t *blk;
    int t;

    snprintf(path, sizeof path, "%s/LANDADJ.DAT", gamedir);
    fp = fopen(path, "rb");
    if (fp == NULL)
        return 0;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    buf = malloc((size_t)size);
    if (buf == NULL || fread(buf, 1, (size_t)size, fp) != (size_t)size) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    /* the file: int32 size, then per territory int16 count and the territories (as cal4_test reads it) */
    h = load_heap_alloc((uint32_t)size);
    blk = load_heap_ptr(h, (uint32_t)size - 4u);
    if (blk == NULL)
        return 0;
    memcpy(blk, buf + 4, (size_t)size - 4u);
    for (t = 0; t < 160; t++) {
        int16_t c;
        int i, kept = 0;
        g_handles[t] = h + off;
        setD32(0xF8E7Cu + 4u * (uint32_t)t, h + off);
        if (off + 2u > (uint32_t)size - 4u)
            return 0;
        c = (int16_t)(blk[off] | blk[off + 1] << 8);
        for (i = 0; i < c; i++) {
            int u = (int16_t)(blk[off + 2 + 2 * i] | blk[off + 3 + 2 * i] << 8);
            if (u >= 0 && u < 160 && D8(0xEE16Cu + (uint32_t)u) != 0) {
                blk[off + 2 + 2 * kept] = (uint8_t)u;
                blk[off + 3 + 2 * kept] = (uint8_t)(u >> 8);
                kept++;
            }
        }
        blk[off] = (uint8_t)kept;
        blk[off + 1] = (uint8_t)(kept >> 8);
        off += 2u + 2u * (uint32_t)(c > 0 ? c : 0);
    }
    free(buf);
    return 1;
}

/* ---- main ------------------------------------------------------------------------------------------------------------ */

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:/work/advciv_run";
    const char *savedir = argc > 2 ? argv[2] : "C:/work/advciv_play";
    char cwd[400], path[2][400], desc[64];
    int i, k, bad;

    if (state_init(gamedir) != 0)
        return 1;
    if (getcwd(cwd, sizeof cwd) == NULL)
        cwd[0] = 0;
    if (chdir(gamedir) == 0) {
        f12394();
        (void)chdir(cwd);
    }
    f7A8D0();   /* the World State Stack's blocks (handles the same on both sides) */
    m_ds = malloc(DS_SIZE);
    g_base[0] = malloc(DS_SIZE);
    g_base[1] = malloc(DS_SIZE);
    if (m_ds == NULL || g_base[0] == NULL || g_base[1] == NULL)
        return 1;

    snprintf(path[0], sizeof path[0], "%s/savgame0.dat", gamedir);
    snprintf(path[1], sizeof path[1], "%s/SAVGAMEA.DAT", savedir);
    g_nbase = 0;
    for (i = 0; i < 2; i++) {
        int v = load_save(path[i], desc);
        if (v < 0) {
            printf("save %s not found: skipped\n", path[i]);
            continue;
        }
        printf("save %s: \"%s\", version %d, phase %d, round %d, humans %d, computers %d\n", path[i], desc, v,
               D8(0xF4102u), S16(0xF40FCu), S8(0xE1620u), S8(0xE1621u));
        /* main 0x1164A..0x11670 and 0x116DC: the map variant and the territories in play (not in the save) */
        setD16(0xF6114u, (uint16_t)(7u * ((D16(0xE1624u) >> 1) & 1u) + D32(0xF40F0u) - 2u));
        f12B00();
        {
            int t, c = 0;
            for (t = 0; t < 160; t++)
                c += D8(0xEE16Cu + (uint32_t)t) != 0;
            printf("   map variant %d, %d territories in play\n", S16(0xF6114u), c);
        }
        if (!g_lists_built) {
            if (!load_landadj(gamedir)) {
                fprintf(stderr, "cannot read %s/LANDADJ.DAT\n", gamedir);
                return 1;
            }
            g_lists_built = 1;
        }
        for (k = 0; k < 160; k++)
            setD32(0xF8E7Cu + 4u * (uint32_t)k, g_handles[k]);
        memcpy(g_base[g_nbase++], g_ds, DS_SIZE);
    }
    check(g_nbase == 2, "both saves read");
    if (g_nbase == 0)
        return 1;

    /* C */
    for (i = 0; i < g_nbase; i++)
        part_c(i, path[i]);
    printf("C. %d checks, %d fail so far\n", g_tests, g_fail);

    /* B */
    part_b();
    printf("B. %d checks, %d fail so far\n", g_tests, g_fail);

    /* A */
    part_a(6000, 0);
    g_original = 1;
    part_a(1500, 0);
    g_original = 0;
    printf("A. %d checks, %d fail so far; runs with moves %d, with a 0xC9 record last %d, with a drop %d, f387C8 = 1: %d,"
           " quits / not_ported %d\n", g_tests, g_fail, g_stat_rec, g_stat_c9, g_stat_drop, g_stat_ret1, g_stat_np);
    check(g_stat_np == 0 && g_quits == 0 && g_np == 0, "A. no quit and no not_ported in any run");
    check(g_stat_c9 > 20 && g_stat_ret1 > 20 && g_stat_drop > 50, "A. the runs reach the success paths");

    /* M: each broken model must be caught */
    g_show_diff = 0;
    g_quiet_fail = 1;
    for (g_mut = 1; g_mut <= 15; g_mut++) {
        char what[80];
        uint32_t rs = g_rs;
        bad = part_a(3000, 1);
        g_rs = rs;
        snprintf(what, sizeof what, "M. mutation %d caught (%d of 3000 runs differ)", g_mut, bad);
        g_quiet_fail = 0;
        check(bad > 0, what);
        g_quiet_fail = 1;
        printf("   mutation %2d: %d of 3000 runs differ\n", g_mut, bad);
    }
    g_mut = 0;
    g_quiet_fail = 0;

    printf("%d checks, %d fail\n", g_tests, g_fail);
    return g_fail == 0 ? 0 : 1;
}
