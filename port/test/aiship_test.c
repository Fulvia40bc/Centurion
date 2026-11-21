/* aiship_test [GAMEDIR [PLAYDIR [RUNS]]]: unit tests for module AISHIP (aiship.c) that need no game run.

   It loads the data object from GAMEDIR/AC_MAIN.EXE as the port does (state_init; GAMEDIR defaults to the run copy
   C:\work\advciv_run, PLAYDIR to C:\work\advciv_play, both only read), the territory records (f12394, terrflag.dat),
   mapdataz.dat (f3EA7C), then saves through saveload.c's f5F05C (PLAYDIR\SAVGAMEA.DAT, the R4 autosave with seven
   computers; GAMEDIR\savgame0.dat, R4 at MOVEMENT; GAMEDIR\savgame3.dat and savgame7.dat, R4 at TRADE / calamities,
   edited), the map variant's tables (f12B00, f3EED8), the thresholds 0x10F2F4 as title.c's initializer f60A80 leaves
   them, and allocates the World State Stack (f7A8D0).
   aiship.c is compiled with -DAISHIP_TEST, so its call into the unported CAL1 routine f20C70 goes to a recorder here;
   the ported heavy callees it calls through their headers (the planner f461BC, f353C4, the score f7B044, f48D44, the
   silent f5AAA4(0), f6106C, the message f79270, the snapshot f7A610 and the screen kit) are recorders defined here
   too: each logs its call (name and arguments, the clock) and acts on the state deterministically: f461BC writes a
   ship and a land move list derived from a hash of the state, perturbs the board inside the World State Stack's
   ranges, changes the phase byte and spends a scenario-chosen time on the test clock; f7B044 scores the state;
   f5AAA4 removes a token; f48D44 reverses the list. The clock is the test's: wat_clock() reads it, f78A20 advances it
   by DOS clock steps as ui_kit2.c's does. The primitives aiship.c calls (rules.c, move.c, stack.c, mstack.c,
   helpers2.c) are the real, separately tested ones. game_rand f78AE4 counts its calls.
     A. random differential runs of the ten routines against a second implementation written here from the listing
        (a transcription that follows the original's frame offsets and labels, not aiship.c), each on its own copy of
        the same starting state: the saves with random edits (ships and their territories, treasuries, stocks,
        tokens on coastal sites, area lists, cities, start territories, the AI flags, the short-of-stock byte, the
        budget word 0xBB3E8, the switch 0xBB3F0, /C's divisor, the trial times). After each run the whole data
        object, the call log, the result, the test clock and the random-number count must equal the model's, and
        nothing may quit.
     B. a mutation check: deliberately broken copies of the model (one change each) must each be caught by part A's
        scenarios.
     C. data facts: the triangular table 0xC7618 (19 entries, then the string pointers f6332C reads for 19+ sites),
        the bit table 0xBC800, the spinner 0xBB404, the budget word 0xBB3E8 = 100, the switch 0xBB3F0 = 1, the strings;
        hand vectors of f6332C's budget (fits, over by time, over by the table overrun) and of f62840 / f61254's busy
        waits with /C.
   Prints each failure (the first few), the counts per part, and exits 0 when all pass.

   Build (from the repo root; artefacts outside the repo):
     clang -std=c11 -Wall -Wextra -DAISHIP_TEST -Iport/src port/test/aiship_test.c port/src/aiship.c port/src/rules.c
       port/src/move.c port/src/stack.c port/src/mstack.c port/src/helpers2.c port/src/route.c port/src/wqsort.c
       port/src/saveload.c port/src/load.c port/src/gfxload.c port/src/state.c port/src/le_image.c -o aiship_test.exe */

#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <direct.h>
#define chdir _chdir

#include "../src/state.h"
#include "../src/game.h"
#include "../src/fg.h"
#include "../src/ui.h"
#include "../src/aiship.h"
#include "../src/aimove.h"
#include "../src/aicity.h"
#include "../src/aiq.h"
#include "../src/conflict.h"
#include "../src/helpers2.h"
#include "../src/kit.h"
#include "../src/map.h"
#include "../src/mapdata.h"
#include "../src/move.h"
#include "../src/mstack.h"
#include "../src/panel.h"
#include "../src/rules.h"
#include "../src/saveload.h"
#include "../src/ship.h"
#include "../src/stack.h"

/* ---- bookkeeping ------------------------------------------------------------------------------------------------ */

static long g_tests, g_fail;
static int g_mut;                  /* the model's mutation, 0 = none (part B) */
#define MUT(k) (g_mut == (k))

static int check(int ok, const char *what)
{
    g_tests++;
    if (!ok) {
        g_fail++;
        if (g_fail <= 30)
            printf("FAIL %s\n", what);
    }
    return ok;
}

/* ---- the call log ----------------------------------------------------------------------------------------------- */

#define TRSIZE (1 << 20)
static char g_tr_port[TRSIZE], g_tr_model[TRSIZE];
static char *g_tr = g_tr_port;
static size_t g_trlen;

static void tr(const char *fmt, ...)
{
    va_list ap;
    if (g_trlen >= TRSIZE - 300)
        return;
    va_start(ap, fmt);
    g_trlen += (size_t)vsnprintf(g_tr + g_trlen, TRSIZE - g_trlen, fmt, ap);
    va_end(ap);
}

/* ---- stubs: nothing here may be reached by the routines tested (the loaders' screen calls are not run) ---------- */

static jmp_buf g_jb;
static int g_quits;
static long g_rand;

_Noreturn void f44CAC(void)
{
    g_quits++;
    tr("QUIT(%s) ", DSTR(0xF5500u));
    longjmp(g_jb, 1);
}

_Noreturn void not_ported(uint32_t addr)
{
    g_quits++;
    tr("NOT_PORTED(%X) ", (unsigned)addr);
    longjmp(g_jb, 3);
}

static void unexpected(const char *name) { tr("UNEXPECTED %s ", name); g_quits++; }

void f3A780(const char *msg) { printf("load message: %s\n", msg); }
void f3A990(void) { unexpected("3A990"); }
void f3AB7C(const char *text) { (void)text; unexpected("3AB7C"); }
void f3AE90(uint32_t spr, int x, int y, int w, int h) { (void)spr; (void)x; (void)y; (void)w; (void)h; unexpected("3AE90"); }
void f3AF54(uint32_t spr) { (void)spr; unexpected("3AF54"); }
void f3B7CC(uint32_t dlg, int id) { (void)dlg; (void)id; unexpected("3B7CC"); }
void f3B870(uint32_t dlg, int id) { (void)dlg; (void)id; unexpected("3B870"); }
int  f3B9A4(uint32_t dlg, int id) { (void)dlg; (void)id; unexpected("3B9A4"); return 0; }
void f3BFBC(uint32_t dlg, int id, int enabled) { (void)dlg; (void)id; (void)enabled; unexpected("3BFBC"); }
void f3C284(uint32_t dlg, int id, uint32_t label) { (void)dlg; (void)id; (void)label; unexpected("3C284"); }
void f3C314(uint32_t dlg, int id) { (void)dlg; (void)id; unexpected("3C314"); }
void f3C3D0(uint32_t dlg, int id) { (void)dlg; (void)id; unexpected("3C3D0"); }
void f3C7A0(uint32_t dlg) { (void)dlg; unexpected("3C7A0"); }
void f3C828(uint32_t dlg) { (void)dlg; unexpected("3C828"); }
void f3CC68(void) { unexpected("3CC68"); }
void f3CCBC(void) { unexpected("3CCBC"); }
void f3D52C(int dx, int dy) { (void)dx; (void)dy; unexpected("3D52C"); }
int  f3E89C(int t) { (void)t; unexpected("3E89C"); return 0; }
int  f3E8FC(int t) { (void)t; unexpected("3E8FC"); return 0; }
void f3F558(void) { unexpected("3F558"); }
void f3F698(int t, int c) { (void)t; (void)c; unexpected("3F698"); }
void f3F70C(int t, int c) { (void)t; (void)c; unexpected("3F70C"); }
void f426D0(int i) { (void)i; unexpected("426D0"); }
void f572CC(int t) { (void)t; unexpected("572CC"); }
void f5E7D0(void) { unexpected("5E7D0"); }
int  f5FF28(void) { unexpected("5FF28"); return -15; }
void f76F70(int n, uint8_t *cards) { (void)n; (void)cards; unexpected("76F70"); }
void f78884(void) { unexpected("78884"); }
void f788AC(void) { unexpected("788AC"); }
int  f78BA4(uint32_t dlg, int map) { (void)dlg; (void)map; unexpected("78BA4"); return -1; }
int  cat_show(const char *name, int flags, int handle) { (void)name; (void)flags; (void)handle; unexpected("cat_show"); return 0; }
int  fg_getpixel(int x, int y) { (void)x; (void)y; return 0; }
void fg_move(int x, int y) { (void)x; (void)y; }
int  fg_point(int x, int y) { (void)x; (void)y; return 0; }
int  junk_makegif(int minx, int maxx, int miny, int maxy, const char *name)
{
    (void)minx; (void)maxx; (void)miny; (void)maxy; (void)name;
    unexpected("junk_makegif");
    return 0;
}
int  junk_showgif(const char *name, int flags) { (void)name; (void)flags; unexpected("junk_showgif"); return 0; }
_Noreturn void port_longjmp(uint32_t jmpbuf) { (void)jmpbuf; unexpected("longjmp"); longjmp(g_jb, 2); }

int f78AE4(void)   /* game_rand: the LCG of game.h, counted */
{
    uint32_t s = D32(GAME_SEED) * 0x15A4E35u + 1u;
    setD32(GAME_SEED, s);
    g_rand++;
    tr("RAND ");
    return (int)((s >> 16) & 0x7FFF);
}

/* ---- the test clock ---------------------------------------------------------------------------------------------- */

static uint32_t g_clk, g_ticks;
static uint32_t g_cost[4];         /* the time each f461BC spends (by call number & 3) */

uint32_t wat_clock(void) { return g_clk; }

static void tick(void)   /* one DOS clock step: hundredths = ticks * 1000 / 182 */
{
    uint32_t a = g_ticks * 1000u / 182u;
    g_ticks++;
    g_clk += g_ticks * 1000u / 182u - a;
}

void f78A20(int t)   /* as ui_kit2.c's: start = clock(); while start + (t / [0xBB3F4]) / 10 > clock(): a step */
{
    uint32_t start = g_clk;
    while (start + (uint32_t)((int32_t)t / S32(0xBB3F4u) / 10) > g_clk)
        tick();
}

/* ---- data helpers (independent of aiship.c) ---------------------------------------------------------------------- */

static int32_t rb(uint32_t a) { return D8(a); }                              /* movzx byte */
static int32_t sb(uint32_t a) { return (int32_t)(D32(a - 3u)) >> 24; }       /* dword [a - 3] sar 24 */
static int32_t sw(uint32_t a) { return (int32_t)(D32(a - 2u)) >> 16; }       /* dword [a - 2] sar 16 */
static int32_t rd(uint32_t a) { return (int32_t)D32(a); }
static void wb(uint32_t a, int32_t v) { setD8(a, (uint8_t)v); }
static void ww(uint32_t a, int32_t v) { setD16(a, (uint16_t)v); }
static void wd(uint32_t a, int32_t v) { setD32(a, (uint32_t)v); }
static uint32_t nat(int32_t n) { return 0xF14CCu + 199u * (uint32_t)n; }
static uint32_t ar(int32_t r) { return 0xE31B8u + (uint32_t)(r * 69); }      /* area record r */

static uint32_t g_seed = 12345u;
static int rnd(int n) { g_seed = g_seed * 1103515245u + 12345u; return n > 0 ? (int)((g_seed >> 8) % (uint32_t)n) : 0; }

/* ---- recorders ------------------------------------------------------------------------------------------------- */

static long g_calls;     /* f461BC calls in this run */
static int g_bias;       /* f7B044's offset */
static int g_n;          /* the scenario's nation (f5AAA4 acts on it) */

static uint32_t fnv(uint32_t h, uint32_t a, uint32_t len)
{
    uint32_t i;
    for (i = 0; i < len; i++)
        h = (h ^ D8(a + i)) * 16777619u;
    return h;
}

static uint32_t hash_state(int n)
{
    uint32_t h = 2166136261u ^ (uint32_t)g_calls;
    int k;
    h = fnv(h, nat(n), 199);
    for (k = 0; k < 4; k++) {
        int t = sw(nat(n) + 0x45u + 2u * (uint32_t)k) - 1;
        if (t >= 0 && t < 160)
            h = fnv(h, ar(t + 1), 69);
    }
    return h;
}

void f461BC(int n)
{
    uint32_t h, m, nt = nat(n);
    int i, c, ships, cnt;

    g_calls++;
    tr("461BC(%d)@%u ", n, (unsigned)g_clk);
    g_clk += g_cost[g_calls & 3];
    h = hash_state(n);
    wb(0xF4102u, 0x5A);                                    /* the callers save and restore the phase byte */
    ships = rb(nt + 0x40u);
    c = (int)(h % 7u);
    wd(0x107D3Cu, c);
    for (i = 0; i < c; i++) {
        static const uint8_t types[5] = { 3, 4, 5, 10, 10 };
        static const uint8_t why[6] = { 4, 6, 0xFF, 0x2A, 6, 4 };
        uint32_t a = 0x1079B8u + 6u * (uint32_t)i;
        h = h * 1103515245u + 12345u;
        wb(a, (int)((h >> 4) % 160u));
        wb(a + 1u, (int)((h >> 12) % 160u));
        wb(a + 2u, 1 + (int)((h >> 20) % 3u));
        wb(a + 3u, (h >> 9) % 8u == 0 ? (int)((h >> 14) % 4u) : (ships > 0 ? (int)((h >> 14) % (uint32_t)ships) : 0));
        wb(a + 4u, types[(h >> 22) % 5u]);
        wb(a + 5u, why[(h >> 25) % 6u]);
    }
    cnt = sb(nt + 0x60u);
    c = (int)((h >> 7) % 8u);
    wd(0x1079B4u, c);
    for (i = 0; i < c; i++) {
        static const uint8_t ltypes[4] = { 7, 0x11, 3, 7 };
        uint32_t a = 0x107694u + 4u * (uint32_t)i;
        h = h * 1103515245u + 12345u;
        m = (h >> 5) % 10u;
        if (m == 0)
            wb(a, 160);
        else if (m < 7 && cnt > 0)
            wb(a, rb(nt + 0x61u + (h >> 9) % (uint32_t)cnt) - 1);
        else
            wb(a, (int)((h >> 9) % 170u));
        wb(a + 1u, (int)((h >> 17) % 160u));
        wb(a + 2u, (h >> 3) % 9u == 0 ? 0 : 1 + (int)((h >> 23) % 3u));
        wb(a + 3u, ltypes[(h >> 27) % 4u]);
    }
    /* a trial change on the board (areas 1..160 are in the World State Stack) and a stray token */
    m = 1u + (h >> 11) % 160u;
    wb(ar((int32_t)m) + 0x0Eu + (uint32_t)n, rb(ar((int32_t)m) + 0x0Eu + (uint32_t)n) + 1);
}

void f353C4(int n)
{
    tr("353C4(%d) ", n);
    wb(0xF09E8u + 199u * (uint32_t)n + 0x3Eu, rb(0xF09E8u + 199u * (uint32_t)n + 0x3Eu) + 1);
}

int f7B044(int n)
{
    uint32_t nt = nat(n);
    int r, tok = 0, s;
    for (r = 1; r <= 160; r++)
        tok += sb(ar(r) + 0x0Eu + (uint32_t)n);
    s = 3 * rb(nt + 0x40u) + (sb(nt + 0x3Eu) & 3) + (tok & 7) / 3 + rd(0x107D3Cu) % 3 + g_bias;
    tr("7B044(%d)=%d ", n, s);
    return s;
}

void f48D44(int32_t *cnt, int32_t *list, int n, int t, int flags)
{
    int i, c = *cnt;
    tr("48D44(%d,n%d,t%d,f%d) ", c, n, t, flags);
    for (i = 0; i < c / 2; i++) {
        int32_t x = list[i];
        list[i] = list[c - 1 - i];
        list[c - 1 - i] = x;
    }
}

void f5AAA4(int show)
{
    uint32_t nt = nat(g_n);
    tr("5AAA4(%d) ", show);
    if (sb(nt + 0x60u) > 0) {
        uint32_t a = ar(rb(nt + 0x61u)) + 0x0Eu + (uint32_t)g_n;
        if (sb(a) > 0)
            wb(a, rb(a) - 1);
    }
}

int aiship_test_f20C70(int p, int v)
{
    uint32_t h = hash_state(v);
    int c = sb(nat(v) + 0x51u), r;
    r = c > 0 ? rb(nat(v) + 0x52u + h % (uint32_t)c) : 1 + (int)(h % 150u);
    tr("20C70(%d,%d)=%d ", p, v, r);
    return r;
}

void f6106C(int k, int scrap, int tp1, int useToken, int n) { tr("6106C(%d,%d,%d,%d,%d)@%u ", k, scrap, tp1, useToken, n, (unsigned)g_clk); }
void f79270(int wait, int n, int t, const char *text, int anim) { tr("79270(%d,%d,%d,\"%s\",%d) ", wait, n, t, text, anim); }
void f7A610(void)   /* as ship.c's: the snapshot back */
{
    tr("7A610 ");
    memcpy(DP(0xE31FDu), DP(0x1139B0u), 0x2B20);
    memcpy(DP(0xE2C18u), DP(0x1137D0u), 0x1E0);
    memcpy(DP(0xF1593u), DP(0x112CECu), 0x88D);
}
static void snapshot(void)   /* as ship.c's f7A598 */
{
    memcpy(DP(0x1139B0u), DP(0xE31FDu), 0x2B20);
    memcpy(DP(0x1137D0u), DP(0xE2C18u), 0x1E0);
    memcpy(DP(0x112CECu), DP(0xF1593u), 0x88D);
}
void f3F670(void) { tr("3F670 "); }
void f3F624(int t, int c) { tr("3F624(%d,%d) ", t, c); }
void f3DA60(int t) { tr("3DA60(%d) ", t); }
void f3D9FC(int n) { tr("3D9FC(%d) ", n); }
void f3CD6C(int8_t pic) { tr("3CD6C(%d) ", pic); }
int  fg_vbcopy(int a, int b, int c, int d, int e, int f, int s, int t) { tr("vbcopy(%d,%d,%d,%d,%d,%d,%d,%d) ", a, b, c, d, e, f, s, t); return 0; }
void f7876C(void) { tr("7876C "); }
int  fg_vbopen(int handle) { tr("vbopen(%d) ", handle); return 0; }
void fg_vbclose(void) { tr("vbclose "); }
int  fg_setcolor(int color) { tr("setcolor(%d) ", color); return 0; }
void f3A3A0(int x, int y, const char *s) { tr("3A3A0(%d,%d,%s) ", x, y, s); }
void f3A044(int x, int y, const char *s) { tr("3A044(%d,%d,%s) ", x, y, s); }
void f3A0B0(int x, int y, int w, const char *s) { tr("3A0B0(%d,%d,%d,%s) ", x, y, w, s); }
void f3C9D0(int x, int y, int w, int h, int flags) { tr("3C9D0(%d,%d,%d,%d,%d) ", x, y, w, h, flags); }
void f3A9F4(const char *text) { tr("3A9F4(\"%s\") ", text); }
void f459E4(int n) { tr("459E4(%d) ", n); }
void f3B05C(uint32_t dlg) { tr("3B05C(%X) ", (unsigned)dlg); }
void f3CBBC(void) { tr("3CBBC "); }
void port_checkpoint(uint32_t addr) { (void)addr; }

/* =================================================================================================================
   The model: the ten routines transcribed again from disasm/AC_MAIN.asm, frame offsets as names (vNN = [ebp-0xNN]).
   ================================================================================================================= */

static void m_busy(uint32_t target)   /* the loops 0x6174F / 0x632FC / 0x686FF as they stand: one step per pass */
{
    while (target > wat_clock())
        f78A20(10 * rd(0xBB3F4u));
}

static void m639E8(int32_t v1c, int32_t *v18, int32_t *v14)
{
    int32_t v4, vC, v8, v10;
    v4 = sb(v1c + 0xF06C0u);
    *v18 = 0;
    for (vC = 0; vC < 0x9D; vC++) {
        v8 = vC + 1;
        if (!(rb(0xE1639u + 35u * (uint32_t)vC) & 0x40))
            continue;
        if (!(rb(0xE1638u + 35u * (uint32_t)vC) & 0x02))
            continue;
        if (sb(0xE31B9u + 69u * (uint32_t)v8) != 1)
            continue;
        for (v10 = 0; v10 < 9; v10++) {
            if (sb(0xF06C1u + (uint32_t)v10) <= v4)
                continue;
            if (sb(0xE31C7u + 69u * (uint32_t)v8 + (uint32_t)v10) == 0)
                continue;
            if (*v18 < 160)
                v14[*v18] = vC;
            (*v18)++;
        }
    }
}

static int32_t m617C0(int32_t v30, int32_t v2c, const int32_t *v28, const int32_t *v24, int32_t *a8)
{
    uint32_t vC;
    int32_t v20, v18, v1c, v10, v14;
    f7AF28();
    vC = nat(v30);
    for (v20 = rd(0x10F6B4u); v20 >= 0; v20--) {
        v18 = v20;
        for (v1c = 0; v1c < 4; v1c++)
            if (v28[v1c] == 1)
                v18 |= rd(0xBC800u + 4u * (uint32_t)v1c);
        for (v1c = 0; v1c < 4; v1c++)
            if (v24[v1c] == 1)
                v18 &= ~rd(0xBC800u + 4u * (uint32_t)v1c);
        if (v18 < 0 || v18 > 15)
            continue;
        if (a8[v18] != 0)
            continue;
        a8[v18] = 1;
        v10 = 0;
        v14 = v18;
        for (v1c = 0; v1c < (MUT(4) ? 3 : 4); v1c++) {
            v10 += v14 & 1;
            v14 >>= 1;
        }
        if (v10 != v2c)
            continue;
        for (v1c = 0; rb(vC + 0x40u) > v1c; v1c++)
            wd(0x10F590u + 18u * (uint32_t)rd(0x10F6B0u) + 4u * (uint32_t)v1c,
               (v18 & rd(0xBC800u + 4u * (uint32_t)v1c)) != 0);
        return 1;
    }
    return 0;
}

static void mswap(uint32_t p, uint32_t q)   /* the three rep movs of 18 bytes through [ebp-0xA4] / [ebp-0x74] / [ebp-0x50] */
{
    uint8_t t[18];
    memcpy(t, DP(q), 18);
    memcpy(DP(q), DP(p), 18);
    memcpy(DP(p), t, 18);
}

static void m61254(int32_t v38)
{
    uint32_t v20, v24;
    int32_t v2c, v48, v44, v3c, v28, v4c, v34, v1c, v50, eax, cnt;
    int32_t v70[4], v80[4], v60[4], v90[4], vE4[16], v364[160];
    uint8_t v18;

    v20 = nat(v38);
    if (rb(v20 + 0x40u) == 0)
        return;
    f7A940();
    if (!MUT(19))
        f7A610();
    v24 = wat_clock();
    snprintf((char *)DP(0xF5500u), 500, DSTR(0xB61FBu), DSTR(rd(0xC768Cu + 4u * (uint32_t)v38)));
    f79270(-550, v38, sw(0xF1511u + 199u * (uint32_t)v38) - 1, DSTR(0xF5500u), 0);
    v2c = 0x2EE;
    m639E8(v38, &v50, v364);
    f7A940();
    v18 = (uint8_t)rb(0xF4102u);
    f353C4(v38);
    f461BC(v38);
    wb(0xF4102u, v18);
    for (v48 = 0; v48 < 4; v48++)
        v70[v48] = -1;
    memset(v90, 0, 16);
    memset(v60, 0, 16);
    memset(DP(0x10F590u), 0, 0x120);
    memset(v80, 0, 16);   /* BUG(1995) 1: the words past the ships (stack garbage in the original) */
    for (v48 = 0; v48 < rd(0x107D3Cu); v48++) {
        eax = sb(0x1079BBu + 6u * (uint32_t)v48);
        if (eax < 0 || eax > 3)
            continue;
        if (v70[eax] != -1)
            continue;
        v70[eax] = sb(0x1079BDu + 6u * (uint32_t)v48);
    }
    v44 = 0;
    for (v48 = 0; rb(v20 + 0x40u) > v48; v48++) {
        if (v70[v48] >= 0) {
            v80[v48] = 0;
            v44++;
        } else
            v80[v48] = 1;
    }
    f7AC60();
    eax = sb(v20 + 0x3Eu);
    if (eax == 0) {
        for (v48 = 0; rb(v20 + 0x40u) > v48; v48++)
            if (v80[v48] == 0 && v60[v48] == 0)
                v80[v48] = 1;
        goto L_6174A;
    }
    if (MUT(3) ? eax > v44 : eax >= v44)
        goto L_6174A;
    v3c = 0 + eax;
    wd(0x10F6B4u, 15);
    memset(vE4, 0, 0x40);
    wd(0x10F6B0u, 0);
    f7A940();
    v28 = 0;
    while (m617C0(v38, v3c, v60, v80, vE4) != 0) {
        eax = v28++;
        snprintf((char *)DP(0xF5500u), 500, DSTR(0xB621Au), DSTR(rd(0xC768Cu + 4u * (uint32_t)v38)),
                 DSTR(rd(0xBB404u + 4u * (uint32_t)(eax & 3))));
        f79270(-560, v38, -200, DSTR(0xF5500u), 0);
        for (v48 = rb(v20 + 0x40u) - 1; v48 >= 0; v48--)
            if (rd(0x10F590u + 18u * (uint32_t)rd(0x10F6B0u) + 4u * (uint32_t)v48) == 0)
                f78D98(v48, v38);
        v18 = (uint8_t)rb(0xF4102u);
        f461BC(v38);
        wb(0xF4102u, v18);
        eax = f7B044(v38);
        cnt = rd(0x10F6B0u);
        ww(0x10F5A0u + 18u * (uint32_t)cnt, eax);
        wd(0x10F6B0u, cnt + 1);
    }
    f7AC60();
    for (v48 = rd(0x10F6B0u) - 2; v48 >= 0; v48--)
        for (v4c = 0; v4c <= v48; v4c++)
            if (MUT(1) ? sw(0x10F5A0u + 18u * (uint32_t)(v4c + 1)) < sw(0x10F5A0u + 18u * (uint32_t)v4c)
                       : sw(0x10F5A0u + 18u * (uint32_t)(v4c + 1)) > sw(0x10F5A0u + 18u * (uint32_t)v4c))
                mswap(0x10F590u + 18u * (uint32_t)(v4c + 1), 0x10F590u + 18u * (uint32_t)v4c);
    v34 = 0;
    for (;;) {
        if (sw(0x10F5A0u) != sw(0x10F5A0u + 18u * (uint32_t)v34))
            break;
        if (v34 >= rd(0x10F6B0u))
            break;
        v34++;
    }
    v1c = (int32_t)((uint32_t)(f78AE4() * (MUT(2) && v34 > 1 ? v34 - 1 : v34)) >> 15);
    for (v48 = 0; rb(v20 + 0x40u) > v48; v48++)
        v80[v48] = rd(0x10F590u + 18u * (uint32_t)v1c + 4u * (uint32_t)v48) == 0;
L_6174A:
    f7AC60();
    m_busy(v24 + (uint32_t)(v2c / 10));
    for (v48 = rb(v20 + 0x40u) - 1; v48 >= 0; v48--)
        f6106C(v48, v80[v48], sw(v20 + 0x45u + 2u * (uint32_t)v48), v60[v48], v38);
}

#define OPT(i) (0x10F6B8u + 18u * (uint32_t)(i))

static void m_sortcap(void)
{
    int32_t v34, v38;
    for (v34 = rd(0x110144u) - 2; v34 >= 0; v34--)
        for (v38 = 0; v38 <= v34; v38++)
            if ((int8_t)rb(OPT(v38 + 1) + 0x11u) > (int8_t)rb(OPT(v38) + 0x11u))
                mswap(OPT(v38 + 1), OPT(v38));
}

static void m_sortscore(void)
{
    int32_t v34, v38;
    for (v34 = rd(0x110144u) - 2; v34 >= 0; v34--)
        for (v38 = 0; v38 <= v34; v38++)
            if ((int16_t)D16(OPT(v38 + 1)) > (int16_t)D16(OPT(v38)))
                mswap(OPT(v38 + 1), OPT(v38));
}

static void m6332C(int32_t v3c)
{
    int32_t v20, v18, v34, v30, v24;
    uint32_t v2c, v1c, esi, eax;

    v20 = sw(0xBB3E8u) * (MUT(10) ? 9600 : 9500) / 100;
    wd(0x10F2F0u, rd(0x110144u));
    v18 = 0;
    for (v34 = 0; v34 < rd(0x10F30Cu); v34++)
        if (rb(0x10F220u + (uint32_t)rd(0x10F310u + 4u * (uint32_t)v34)) == 0)
            v18++;
    wd(0x10F2E8u, rd(0xC7618u + 4u * (uint32_t)v18));
    if (rd(0x10F2DCu) == 0)
        wd(0x10F2DCu, 1);
    wd(0x10F2D8u, (int32_t)((uint32_t)rd(0x10F2CCu) / (uint32_t)rd(0x10F2DCu)));
    eax = wat_clock() - (uint32_t)rd(0x10F2C8u);
    wd(0x10F2D4u, (int32_t)eax);
    esi = (uint32_t)rd(0x10F2D4u);
    if ((uint32_t)rd(0x10F2D8u) * (uint32_t)rd(0x10F2E8u) + esi <= (uint32_t)v20)
        goto L_636B3;
    if (v18 <= 3)
        return;
    if (esi > (uint32_t)v20) {
        v30 = 3;
    } else {
        v2c = ((uint32_t)v20 - esi) / (uint32_t)rd(0x10F2D8u);
        v30 = 0xF;
        for (v34 = 0; v34 < 0xF; v34++) {
            int32_t c0 = rd(0xC7618u + 4u * (uint32_t)v34), c1 = rd(0xC761Cu + 4u * (uint32_t)v34);
            if ((int32_t)v2c == c0 || (c0 < (int32_t)v2c && (int32_t)v2c < c1)) {
                v30 = v34;
                break;
            }
        }
    }
    if (v30 < 3)
        v30 = 3;
    if (v30 > v18)
        return;
    m_sortcap();
    m_sortscore();
    for (v34 = v30; v34 < rd(0x110144u); v34++)
        wb(0x10F220u + (uint32_t)rb(OPT(v34) + 3u), 1);
    wd(0x10F2E8u, rd(0xC7618u + 4u * (uint32_t)v30));
    return;
L_636B3:
    m_sortcap();
    m_sortscore();
    if (v3c > 2)
        wd(0x10F2E0u, v18 > (MUT(11) ? 3 : 4) ? (MUT(11) ? 3 : 4) : v18);
    else
        wd(0x10F2E0u, 0);
    v24 = rd(0x10F2F4u + 4u * (uint32_t)rd(0x10F2E0u));
    v1c = (uint32_t)rd(0x10F2D8u) * (uint32_t)rd(0x10F2E8u) + (uint32_t)rd(0x10F2D4u);
    if ((uint32_t)rd(0x10F2D8u) * (uint32_t)v24 + v1c > (uint32_t)v20) {
        uint32_t v28 = ((uint32_t)v20 - v1c) / (uint32_t)rd(0x10F2D8u);
        wd(0x10F2E0u, 4);
        for (v34 = 0; v34 <= 4; v34++) {
            int32_t c0 = rd(0x10F2F4u + 4u * (uint32_t)v34), c1 = rd(0x10F2F8u + 4u * (uint32_t)v34);
            if ((int32_t)v28 == c0 || (c0 < (int32_t)v28 && (int32_t)v28 < c1)) {
                wd(0x10F2E0u, v34);
                break;
            }
        }
    }
    wd(0x10F2ECu, rd(0x10F2F4u + 4u * (uint32_t)rd(0x10F2E0u)));
}

/* the token byte of n at site s as 0x63D10 reads it: byte [69(s + 1) + v30 + 0xE31C7], int8 */
#define TOKS(s) ((int8_t)rb(0xE31C7u + 69u * (uint32_t)((s) + 1) + (uint32_t)v30))
#define STE(i)  rd(0x10F310u + 4u * (uint32_t)(i))
#define XC(s)   rb(0x10F220u + (uint32_t)(s))

static int32_t m63D10(int32_t v44)
{
    int32_t v30 = v44 - 1, v24, v20, v1c, v18, v2c, v40 = 0, v3c = 0, v38 = 0, s;
    int8_t v28[3] = { 0, 0, 0 };
    uint32_t eax;

    v24 = rd(0x11014Cu);
    v20 = rd(0x110150u);
    v1c = rd(0x110154u);
    v18 = v24 + v20 + v1c;
L_63D55:
    eax = (uint32_t)rd(0x110158u);
    if (eax == 0) {
        if (rd(0x11014Cu) > 0)
            v28[0] = 0;
        else
            v28[0] = (int8_t)((rd(0x110150u) <= 0) + 1);
        for (v40 = rd(0x11015Cu) + 1; v40 < rd(0x10F30Cu); v40++) {
            s = STE(v40);
            if (TOKS(s) > v28[0]) {
                wd(0x110178u, 1);
                wb(0x110174u, s);
                wb(0x11016Cu, 2 - v28[0]);
                wb(0x110170u, v28[0]);
                wd(0x11015Cu, rd(0x11015Cu) + 1);
                return 1;
            }
            wd(0x11015Cu, rd(0x11015Cu) + 1);
        }
        goto L_64200;
    }
    if (eax == 1) {
        for (v2c = 0; v2c < 2;) {
            if (v24 > 0) { v24--; v28[v2c++] = 0; }
            else if (v20 > 0) { v20--; v28[v2c++] = 1; }
            else if (v1c > 0) { v1c--; v28[v2c++] = 2; }
            else break;
        }
        for (v40 = rd(0x11015Cu) + 1; v40 < rd(0x10F30Cu); v40++) {
            s = STE(v40);
            if (!XC(s) && TOKS(s) > v28[0]) {
                wb(0x110174u, s);
                wb(0x11016Cu, 2 - v28[0]);
                wb(0x110170u, v28[0]);
                for (v3c = (MUT(9) ? (rd(0x110160u) + 1 > v40 ? rd(0x110160u) + 1 : v40 + 1) : rd(0x110160u) + 1);
                     v3c < rd(0x10F30Cu); v3c++) {
                    s = STE(v3c);
                    if (!XC(s) && TOKS(s) > v28[1]) {
                        wb(0x110175u, s);
                        wb(0x11016Du, 2 - v28[1]);
                        wb(0x110171u, v28[1]);
                        wd(0x110178u, 2);
                        wd(0x110160u, rd(0x110160u) + 1);
                        return 1;
                    }
                    wd(0x110160u, rd(0x110160u) + 1);
                }
            }
            wd(0x11015Cu, rd(0x11015Cu) + 1);
            wd(0x110160u, rd(0x11015Cu));
        }
        goto L_64200;
    }
    if (eax == 2) {
        for (v2c = 0; v2c < 3;) {
            if (v24 > 0) { v24--; v28[v2c++] = 0; }
            else if (v20 > 0) { v20--; v28[v2c++] = 1; }
            else if (v1c > 0) { v1c--; v28[v2c++] = 2; }
            else break;
        }
        for (v40 = rd(0x11015Cu) + 1; v40 < rd(0x10F30Cu); v40++) {
            s = STE(v40);
            if (!XC(s) && TOKS(s) > v28[0]) {
                wb(0x110174u, s);
                wb(0x11016Cu, 2 - v28[0]);
                wb(0x110170u, v28[0]);
                for (v3c = rd(0x110160u) + 1; v3c < rd(0x10F30Cu); v3c++) {
                    s = STE(v3c);
                    if (!XC(s) && TOKS(s) > v28[1]) {
                        wb(0x110175u, s);
                        wb(0x11016Du, 2 - v28[1]);
                        wb(0x110171u, v28[1]);
                        for (v38 = rd(0x110164u) + 1; v38 < rd(0x10F30Cu); v38++) {
                            s = STE(v38);
                            if (!XC(s) && TOKS(s) > v28[2]) {
                                wb(0x110176u, s);
                                wb(0x11016Eu, 2 - v28[2]);
                                wb(0x110172u, v28[2]);
                                wd(0x110178u, 3);
                                wd(0x110164u, rd(0x110164u) + 1);
                                return 1;
                            }
                            wd(0x110164u, rd(0x110164u) + 1);
                        }
                    }
                    wd(0x110160u, rd(0x110160u) + 1);
                    wd(0x110164u, rd(0x110160u));
                }
            }
            wd(0x11015Cu, rd(0x11015Cu) + 1);
            wd(0x110160u, rd(0x11015Cu));
            wd(0x110164u, rd(0x11015Cu));
        }
        goto L_64200;
    }
L_64200:
    wd(0x11015Cu, -1);
    wd(0x110160u, -1);
    wd(0x110164u, -1);
    wd(0x110168u, 2);
    wd(0x110158u, rd(0x110158u) + 1);
    v24 = rd(0x11014Cu);
    v20 = rd(0x110150u);
    v1c = rd(0x110154u);
    v18 = v24 + v20 + v1c;
    if (rd(0x110158u) > 2)
        return 0;
    if (rd(0x110158u) == 1) {
        if (rb(0xF488Cu + (uint32_t)v44) != 0) {
            for (v40 = 0; v40 < rd(0x10F30Cu); v40++) {
                s = STE(v40);
                if (!XC(s))
                    continue;
                if (sb(0xE31C6u + 69u * (uint32_t)(s + 1) + (uint32_t)v44) <= 1)
                    continue;
                if (!MUT(8) && sb(0xEED84u + 160u * (uint32_t)v44 + (uint32_t)s) < 0)
                    continue;
                wb(0x10F220u + (uint32_t)s, 0);
                for (v3c = 0; v3c < rd(0x110144u); v3c++)
                    if (rb(OPT(v3c) + 3u) == STE(v40)) {
                        ww(OPT(v3c), 0);
                        break;          /* 0x642C1 falls through to 0x642C9 (the next site): the first match only */
                    }
            }
        }
        m6332C(v18);
    }
    if (rd(0x110158u) == 2)
        for (v40 = rd(0x10F2E0u); v40 < rd(0x10F2F0u); v40++)
            wb(0x10F220u + (uint32_t)rb(OPT(v40) + 3u), 1);
    if (rd(0x110158u) + 1 <= v18)
        goto L_63D55;
    return 0;
}

#undef TOKS
#undef STE
#undef XC

static int32_t m62840(int32_t v60)
{
    uint32_t v28, eax, edx, ebx;
    int32_t v58 = -1, v50, v4c, v40, v3c, v48, v5c, v34, v38, v24, k;
    int32_t v2f4[160];
    uint8_t v1c, v20, v30[4];
    int8_t v18;

    if (sw(0xBB3F0u) == 0)
        return v58;
    v28 = nat(v60);
    v50 = v60 - 1;
    f63950(v60);
    wb(0x108190u, 0);
    wd(0x110148u, 4 - rb(v28 + 0x40u));
    wd(0x11014Cu, f63AF0(v60));
    wd(0x110150u, f63B7C(v60));
    eax = (uint32_t)f63C6C(v60);
    ebx = (uint32_t)rd(0x110150u);
    edx = eax;
    wd(0x110154u, (int32_t)eax);
    wd(0x110144u, 0);
    wd(0x10F2E4u, rd(0x10F30Cu));
    wd(0x10F2E8u, 0);
    eax = (uint32_t)rd(0x11014Cu) + ebx;
    wd(0x10F2ECu, 0);
    edx += eax;
    if (edx != 0 && rd(0x10F30Cu) != 0) {
        snprintf((char *)DP(0xF5500u), 500, DSTR(0xB6358u), DSTR(rd(0xC768Cu + 4u * (uint32_t)v60)), rd(0x10F2E4u));
        f79270(-550, v60, -200, DSTR(0xF5500u), 0);
    }
    f7A940();
    f7A610();
    f7A940();
    v40 = 0;
    wd(0x10F2CCu, 0);
    wd(0x10F2DCu, 0);
    wd(0x10F2C8u, (int32_t)wat_clock());
    v1c = (uint8_t)rb(0xF4102u);
    f353C4(v60);
    f461BC(v60);
    wb(0xF4102u, v1c);
    v4c = (int16_t)f7B044(v60);
    f7AF28();
    m639E8(v60, &v38, v2f4);
    wd(0x110150u, f63B7C(v60));
    wd(0x110154u, f63C6C(v60));
    wd(0x110158u, 0);
    wd(0x110168u, -1);
    wd(0x110164u, -1);
    wd(0x110160u, -1);
    wd(0x11015Cu, -1);
    if (rd(0x11014Cu) + rd(0x110150u) + rd(0x110154u) == 0 || rd(0x10F30Cu) == 0)
        goto L_632F2;
    v40 = 0x2EE;
    memset(DP(0x10F220u), 0, 0xA0);
    f7A940();
L_62A4D:
    if (m63D10(v60) == 0)
        goto L_62FCA;
    v18 = (int8_t)rb(v28 + 0x40u);
    v3c = 0;
    for (v5c = 0; v5c < rd(0x110178u); v5c++) {
        int32_t v44 = rb(v28 + 0x40u), r = rb(0x110174u + (uint32_t)v5c) + 1;
        uint8_t bl;
        wb(0xE31D4u + 69u * (uint32_t)r + (uint32_t)v50, rb(0xE31D4u + 69u * (uint32_t)r + (uint32_t)v50) + 1);
        ww(v28 + 0x45u + 2u * (uint32_t)v44, r);
        bl = (uint8_t)rb(v28 + 0x40u);
        wb(v28 + 0x40u, bl + 1);
        wb(0xE31E6u + 69u * (uint32_t)r + (uint32_t)v50, bl);
        wb(v28 + 0x3Eu, rb(v28 + 0x3Eu) - rb(0x11016Cu + (uint32_t)v5c));
        wb(0xE31C7u + 69u * (uint32_t)r + (uint32_t)v50, rb(0xE31C7u + 69u * (uint32_t)r + (uint32_t)v50) - rb(0x110170u + (uint32_t)v5c));
        wb(v28 + 0x99u, rb(v28 + 0x99u) + (uint8_t)(rb(0x11016Cu + (uint32_t)v5c) + rb(0x110170u + (uint32_t)v5c)));
        wb(v28 + 0x98u, rb(v28 + 0x98u) - rb(0x110170u + (uint32_t)v5c));
        wb(0x10F6BBu + (uint32_t)v5c + 18u * (uint32_t)rd(0x110144u), rb(0x110174u + (uint32_t)v5c));
        wb(0x10F6BFu + (uint32_t)v5c + 18u * (uint32_t)rd(0x110144u), rb(0x11016Cu + (uint32_t)v5c));
        wb(0x10F6C3u + (uint32_t)v5c + 18u * (uint32_t)rd(0x110144u), rb(0x110170u + (uint32_t)v5c));
        v3c += sb(0x110170u + (uint32_t)v5c);
    }
    if (rd(0x10F2E4u) != 0) {
        snprintf((char *)DP(0xF5500u), 500, DSTR(0xB637Eu), DSTR(rd(0xC768Cu + 4u * (uint32_t)v60)), rd(0x10F2E4u));
        wd(0x10F2E4u, rd(0x10F2E4u) - 1);
    } else {
        snprintf((char *)DP(0xF5500u), 500, DSTR(0xB63A4u), DSTR(rd(0xC768Cu + 4u * (uint32_t)v60)), rd(0x10F2E8u),
                 rd(0x10F2ECu));
        if (rd(0x110178u) == 2)
            wd(0x10F2E8u, rd(0x10F2E8u) - 1);
        else if (rd(0x110178u) == 3)
            wd(0x10F2ECu, rd(0x10F2ECu) - 1);
    }
    f79270(-560, v60, -200, DSTR(0xF5500u), 0);
    wd(0x10F2D0u, (int32_t)wat_clock());
    v1c = (uint8_t)rb(0xF4102u);
    f461BC(v60);
    wb(0xF4102u, v1c);
    eax = wat_clock() - (uint32_t)rd(0x10F2D0u);
    wd(0x10F2CCu, (int32_t)((uint32_t)rd(0x10F2CCu) + eax));
    wd(0x10F2C0u, (int32_t)((uint32_t)rd(0x10F2C0u) + eax));
    wb(OPT(rd(0x110144u)) + 0x0Fu, 0);
    wd(0x10F2DCu, rd(0x10F2DCu) + 1);
    wd(0x10F2C4u, rd(0x10F2C4u) + 1);
    memset(v30, 0, 4);
    v20 = (uint8_t)(rb(v28 + 0x40u) * ((rb(v28 + 0x25u) != 0) + 4));
    for (v34 = 0; v34 < rd(0x107D3Cu); v34++) {
        k = sb(0x1079BBu + 6u * (uint32_t)v34);
        if (k >= 0 && k < 4)
            v30[k] = 1;
        if (sb(0x1079BCu + 6u * (uint32_t)v34) == (MUT(17) ? 3 : 10))
            v20--;
    }
    wb(OPT(rd(0x110144u)) + 0x11u, v20);
    v48 = 0;
    if (v3c > 0) {
        v48 = 1;
        for (v5c = 0; v5c < rd(0x110178u); v5c++) {
            if (sb(0x110170u + (uint32_t)v5c) <= 0)
                continue;
            for (v34 = 0; v34 < rd(0x107D3Cu); v34++) {
                uint32_t a = 0x1079B8u + 6u * (uint32_t)v34;
                if (sb(a + 3u) != (MUT(5) ? v18 + v5c : v5c))
                    continue;
                if (sb(a + 5u) != 4) {
                    if (sb(a + 5u) != 6)
                        continue;
                    if (sb(a + 4u) != 4 && sb(a + 4u) != 5)
                        continue;
                }
                if (rb(0xE163Au + 35u * (uint32_t)rb(a + 1u)) & 1)
                    continue;
                v48 = 0;
            }
        }
        if (rd(0x110178u) == 1 && v48 != 0 && v18 >= 0 && v18 < 4)
            v30[v18] = 0;
    }
    if (v48 == 0)
        ww(OPT(rd(0x110144u)), f7B044(v60));
    else
        ww(OPT(rd(0x110144u)), -100);
    wb(OPT(rd(0x110144u)) + 2u, rb(0x110178u));
    for (v5c = 0; v5c < rd(0x110178u); v5c++) {
        k = v18 + v5c;
        if (k >= 0 && k < 4 && v30[k] != 0) {
            if ((int16_t)D16(OPT(rd(0x110144u))) > v4c)
                continue;
        }
        ww(OPT(rd(0x110144u)), -100);
        if (rd(0x110158u) == 0)
            wb(0x10F220u + (uint32_t)rb(0x110174u), 1);
        break;
    }
    if (!MUT(20))
        f7AF28();
    wd(0x110144u, rd(0x110144u) + 1);
    if (rd(0x110144u) < 0x96)
        goto L_62A4D;
L_62FCA:
    f7AC60();
    for (v5c = 1; v5c < rd(0x110144u); v5c++)
        for (v34 = rd(0x110144u) - 1; v34 >= v5c; v34--)
            if ((int8_t)rb(OPT(v34 - 1) + 2u) > (int8_t)rb(OPT(v34) + 2u))
                mswap(OPT(v34), OPT(v34 - 1));
    m_sortcap();
    m_sortscore();
    for (v5c = 0; v5c < rd(0x110144u); v5c++)
        if (MUT(6) ? (int16_t)D16(OPT(v5c)) < v4c : (int16_t)D16(OPT(v5c)) <= v4c) {
            wd(0x110144u, v5c);
            break;
        }
    if (rd(0x110144u) > 0) {
        v24 = 0;
        while (v24 < rd(0x110144u) && D16(OPT(0)) == D16(OPT(v24)) && (MUT(7) || rb(OPT(0) + 2u) == rb(OPT(v24) + 2u)))
            v24++;
        v58 = (int32_t)((uint32_t)(f78AE4() * v24) >> 15);
    }
L_632F2:
    f7AD24();
    f7AC60();
    m_busy((uint32_t)rd(0x10F2C8u) + (uint32_t)(v40 / 10));
    return v58;
}

static void m66B54(int32_t v10, int32_t v1c, int32_t v18)
{
    int32_t v14;
    char v80[100];
    while (v18 > 0) {
        v14 = aiship_test_f20C70(v10, v1c) - 1;
        f3F670();
        f3F624(v14, 0xF);
        f3DA60(v14);
        f78A20(0x1F4);
        wb(0xE31F3u + 69u * (uint32_t)(v14 + 1) + (uint32_t)(MUT(16) ? v10 : v1c), 1);
        f78F04(v14, v1c);
        f78E74(v14, v10);
        v18--;
        snprintf(v80, sizeof v80, DSTR(0xB8A03u), DSTR(rd(0xC768Cu + 4u * (uint32_t)v10)), DSTR(rd(0xC77D8u + 4u * (uint32_t)v14)));
        f79270(0x7D0, v1c, v14, v80, 0);
    }
}

static void m66C2C(int32_t v18, int32_t v2c)
{
    int32_t v20, v28, v24, v1c;
    uint8_t v10, v14;
    char v90[100];
    for (v20 = 0; v20 < (MUT(14) ? 1 : 2) && v2c > 0; v20++) {
        v10 = v20 != 0;
        for (v28 = sb(0xF151Du + 199u * (uint32_t)v18) - 1; v28 >= 0 && v2c > 0; v28--) {
            v14 = 0;
            for (v24 = 0; v24 < 4; v24++)
                if (rb(0xF151Eu + 199u * (uint32_t)v18 + (uint32_t)v28) - 1 == rd(0xC0618u + 16u * (uint32_t)v18 + 4u * (uint32_t)v24))
                    v14 = 1;
            if (v14 != v10)
                continue;
            v1c = rb(0xF151Eu + 199u * (uint32_t)v18 + (uint32_t)v28);
            f3F670();
            f3F624(v1c - 1, 0xF);
            f3DA60(v1c - 1);
            f78A20(0x1F4);
            f78F04(v1c - 1, v18);
            v2c--;
            snprintf(v90, sizeof v90, DSTR(0xB8A1Cu), DSTR(rd(0xC768Cu + 4u * (uint32_t)v18)), DSTR(rd(0xC77D4u + 4u * (uint32_t)v1c)));
            f79270(0x9C4, v18, v1c - 1, v90, 0);
        }
    }
}

/* the frame of 0x680DC from [ebp-0x330] up: int32 alloc[160] at 0, the message buffer at 0x280 */
static uint8_t g_fr[0x2E4];
static int32_t FA(int32_t i) { int32_t v; memcpy(&v, g_fr + 4 * i, 4); return v; }
static void FAset(int32_t i, int32_t v) { memcpy(g_fr + 4 * i, &v, 4); }

static void m680DC(int32_t v4c, int32_t v14)
{
    uint32_t v18, v1c;
    int32_t v44, v34, v24, v3c, v38, v40, v30, v20, v28, v2c, v48, t, took;
    int32_t cnt, list[160];

    v44 = v4c - 1;
    v18 = nat(v4c);
    f7A8D0();
    memset(g_fr, 0xEE, sizeof g_fr);
    snprintf((char *)g_fr + 0x280, 100, DSTR(0xB8B55u), DSTR(rd(0xC768Cu + 4u * (uint32_t)v4c)));
    f79270(-550, v4c, -200, (char *)g_fr + 0x280, 0);
    v1c = wat_clock();
    v34 = 0x3E8;
    if (sb(v18 + 0x99u) == 0) {
        wb(0xF488Cu + (uint32_t)v4c, 1);
        goto L_686FA;
    }
    if (sb(v18 + 0x99u) * 100 / v14 < 0x50)
        wb(0xF488Cu + (uint32_t)v4c, 1);
    memset(g_fr, 0, 0x280);
    if (MUT(13))
        memset(g_fr + 0x280, 0, 4);
    v24 = sb(v18 + 0x98u);
    f7A940();
    for (v3c = 0; sb(v18 + 0x60u) > v3c; v3c++) {
        v38 = rb(v18 + 0x61u + (uint32_t)v3c);
        v40 = sb(0xE31C7u + 69u * (uint32_t)v38 + (uint32_t)v44);
        if (v38 == 0)
            continue;
        if (v40 == 1) {
            wb(v18 + 0x98u, rb(v18 + 0x98u) + 1);
            wb(0xE31C7u + 69u * (uint32_t)v38 + (uint32_t)v44, rb(0xE31C7u + 69u * (uint32_t)v38 + (uint32_t)v44) + 1);
            FAset(v38 - 1, FA(v38 - 1) + v40);
        } else if (v40 >= 2) {
            wb(0xE31C7u + 69u * (uint32_t)v38 + (uint32_t)v44, rb(0xE31C7u + 69u * (uint32_t)v38 + (uint32_t)v44) + 2);
            wb(v18 + 0x98u, rb(v18 + 0x98u) + 2);
            FAset(v38 - 1, FA(v38 - 1) + 2);
        }
    }
    v30 = sb(v18 + 0x98u) - v24 - sb(v18 + 0x99u);
    f461BC(v4c);
    f5AAA4(0);
    for (v3c = 0; sb(v18 + 0x60u) > v3c; v3c++) {
        v38 = rb(v18 + 0x61u + (uint32_t)v3c);
        v20 = v38 - 1;
        v28 = f78D08(v20, v4c);
        if (rb(0xE2C1Au + 3u * (uint32_t)v20) & 2)
            continue;
        if (sb(0xE31C7u + 69u * (uint32_t)v38 + (uint32_t)v44) <= v28)
            continue;
        v2c = MUT(12) ? sb(0xE31C7u + 69u * (uint32_t)v38 + (uint32_t)v44) - v28 : sb(0xE31C7u + 69u * (uint32_t)v38 + (uint32_t)v44) > v28;
        if (v20 < 0)
            continue;
        while (v2c > 0 && v30 > 0 && FA(v20) > 0) {
            v30--;
            v2c--;
            FAset(v20, FA(v20) - 1);
        }
    }
    if (v30 > 0)
        for (v48 = 0; v48 < rd(0x1079B4u); v48++) {
            uint32_t a = 0x107694u + 4u * (uint32_t)v48;
            if (sb(a + 3u) != 7 || rb(a) > 0xA0)
                continue;
            for (v2c = sb(a + 2u); v2c > 0 && v30 > 0 && FA(rb(a)) > 0; v2c--) {
                v30--;
                FAset(rb(a), FA(rb(a)) - 1);
            }
        }
    if (v30 > 0)
        for (v48 = 0; v48 < rd(0x1079B4u); v48++) {
            uint32_t a = 0x107694u + 4u * (uint32_t)v48;
            if (sb(a + 3u) != 0x11 || rb(a) > 0xA0)
                continue;
            for (v2c = sb(a + 2u); v2c > 0 && v30 > 0 && FA(rb(a)) > 0; v2c--) {
                v30--;
                FAset(rb(a), FA(rb(a)) - 1);
            }
        }
    if (v30 > 0)
        for (v3c = 0; sb(v18 + 0x60u) > v3c; v3c++) {
            v20 = rb(v18 + 0x61u + (uint32_t)v3c) - 1;
            if (!(rb(0xF3581u + 314u * (uint32_t)v44 + 2u * (uint32_t)v20) & 1) || v20 < 0)
                continue;
            while (v30 > 0 && FA(v20) > 0) {
                v30--;
                FAset(v20, FA(v20) - 1);
            }
        }
    if (v30 > 0)
        for (v3c = 0; sb(v18 + 0x60u) > v3c; v3c++) {
            v20 = rb(v18 + 0x61u + (uint32_t)v3c) - 1;
            if (!(rb(0xF3580u + 314u * (uint32_t)v44 + 2u * (uint32_t)v20) & 0x80) || v20 < 0)
                continue;
            while (v30 > 0 && FA(v20) > 0) {
                v30--;
                FAset(v20, FA(v20) - 1);
            }
        }
    f7AC60();
    if (v30 > 0) {
        wd(0xF7C9Cu, 0);
        for (v48 = 0; sb(v18 + 0x60u) > v48; v48++) {
            wd(0xF7CA0u + 4u * (uint32_t)rd(0xF7C9Cu), rb(v18 + 0x61u + (uint32_t)v48) - 1);
            wd(0xF7C9Cu, rd(0xF7C9Cu) + 1);
        }
        cnt = rd(0xF7C9Cu);
        for (v48 = 0; v48 < cnt && v48 < 160; v48++)
            list[v48] = rd(0xF7CA0u + 4u * (uint32_t)v48);
        f48D44(&cnt, list, v4c, 0, 1);
        wd(0xF7C9Cu, cnt);
        for (v48 = 0; v48 < cnt && v48 < 160; v48++)
            wd(0xF7CA0u + 4u * (uint32_t)v48, list[v48]);
        while (v30 > 0) {
            took = 0;
            for (v48 = 0; v48 < rd(0xF7C9Cu) && v30 > 0; v48++) {
                t = rd(0xF7CA0u + 4u * (uint32_t)v48);
                if (t < 0 || t > 160)
                    continue;
                if (FA(t) > 0) {
                    FAset(t, FA(t) - 1);
                    v30--;
                    took = 1;
                }
            }
            if (!took)
                break;
        }
    }
    for (v48 = 0; v48 < 0xA0; v48++) {
        if (FA(v48) <= 0 || sb(v18 + 0x99u) <= 0)
            continue;
        f57E28(v48, v4c, FA(v48));
        wb(v18 + 0x98u, rb(v18 + 0x98u) + (uint8_t)FA(v48));
        wb(v18 + 0x99u, rb(v18 + 0x99u) - (uint8_t)FA(v48));
    }
L_686FA:
    f7ABD0();
    m_busy(v1c + (uint32_t)(v34 / (MUT(18) ? rd(0xBB3F4u) : 1) / 10));
}

static void m5BF30(int32_t al, int32_t v14, int32_t v1c)
{
    int8_t vC = (int8_t)al;
    int32_t v10, v18, s;
    char v80[100];

    s = sb(0xF1528u + 199u * (uint32_t)(MUT(15) ? vC : vC - 1));
    v10 = sw(0xBCFDAu + 4u * (uint32_t)s + 68u * (uint32_t)(vC - 1));
    if (v10 >= 2)
        v18 = v14;
    else {
        v18 = rb(0xF150Cu + 199u * (uint32_t)vC);
        if (v18 > v14)
            v18 = v14;
        else if (v18 < 1)
            v18 = 0;
    }
    wb(0xF1565u + 199u * (uint32_t)vC, rb(0xF1565u + 199u * (uint32_t)vC) - (uint8_t)v18);
    wb(0xF150Au + 199u * (uint32_t)vC, rb(0xF150Au + 199u * (uint32_t)vC) + (uint8_t)v18);
    if (v1c == 0)
        return;
    f3CD6C(3);
    wb(0xEE168u, 3);
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, rd(0xF52CCu), rd(0xF52C8u));
    f7876C();
    fg_vbopen(rd(0xF52C8u));
    f3A3A0(0x125, 0x185, DSTR(0xB5D04u));
    f3A044(0x15A, 0x185, DSTR(0xB5D0Du));
    f3C9D0(0x136, 0x181, 0x13, 0x13, 5);
    fg_setcolor(5);
    snprintf(v80, sizeof v80, "%d", (int)v18);
    f3A0B0(0x136, 0x185, 0x13, v80);
    fg_vbclose();
    snprintf(v80, sizeof v80, DSTR(0xB5D16u), DSTR(rd(0xC768Cu + 4u * (uint32_t)vC)));
    f3A9F4(v80);
    f459E4(vC);
    f3B05C(0xF596Cu);
    f3F670();
    f3D9FC(vC);
    f3CBBC();
    f78A20(0x9C4);
}

/* =================================================================================================================
   Part A: differential runs
   ================================================================================================================= */

enum { F639E8, F61254, F62840, F66B54, F66C2C, F680DC, F5BF30, NFN };
static const char *const FNAME[NFN] = { "f639E8", "f61254", "f62840", "f66B54", "f66C2C", "f680DC", "f5BF30" };

static uint8_t *g_ds_live, *g_base, *g_port, *g_model;
static uint32_t g_coastal[160];
static int g_ncoastal;

typedef struct {
    int fn, a[4], orig;
    uint32_t clk0, cost[4];
    int bias;
} Op;

typedef struct {
    int32_t ret;
    uint32_t hash, clk;
    int quits;
    long rands;
} Res;

static void put_city(int n, int t)
{
    if (sb(nat(n) + 0x51u) < 9 && rb(0xE2C18u + 3u * (uint32_t)t) == 0 && !(rb(0xE2C1Au + 3u * (uint32_t)t) & 1))
        f78E74(t, n);
}

static void nat_setup(int n, int fn)
{
    uint32_t nt = nat(n);
    int k, ships, i, c, t, m;

    ships = fn == F61254 ? 1 + rnd(4) : rnd(5);
    if (rnd(10) == 0)
        ships = 0;
    wb(nt + 0x40u, ships);
    for (k = 0; k < 4; k++)
        ww(nt + 0x45u + 2u * (uint32_t)k, k < ships ? (int)g_coastal[rnd(g_ncoastal)] + 1 : 0);
    wb(nt + 0x3Eu, fn == F61254 ? rnd(4) : rnd(4) == 0 ? rnd(25) : rnd(7));
    wb(nt + 0x25u, rnd(2));
    wb(nt + 0x99u, rnd(25));
    wb(0xF488Cu + (uint32_t)n, rnd(2));
    /* tokens on coastal sites (the search's candidates): up to 30 */
    m = fn == F62840 ? (rnd(3) == 0 ? 25 + rnd(30) : rnd(12)) : rnd(6);
    for (i = 0; i < m; i++) {
        t = (int)g_coastal[rnd(g_ncoastal)];
        c = sb(ar(t + 1) + 0x0Eu + (uint32_t)n);
        if (c < 2 + rnd(3))
            f57E28(t, n, 1 + rnd(3));
        wb(0xEED84u + 160u * (uint32_t)n + (uint32_t)t, rnd(11) - 4);
        if (rnd(4) == 0)
            wb(0xE2C1Au + 3u * (uint32_t)t, rb(0xE2C1Au + 3u * (uint32_t)t) | 2);
    }
    for (i = 0; i < 10; i++)
        wb(0xF06C0u + (uint32_t)i, rnd(12) - 2);
}

static int scenario_setup(Op *op)
{
    int n, m, k, i, t;
    uint32_t nt;

    n = 1 + rnd(9);
    g_n = n;
    op->a[3] = n;
    nt = nat(n);
    nat_setup(n, op->fn);
    wd(0xBB3F4u, rnd(4) == 0 ? 10 : 1);
    ww(0xBB3E8u, rnd(3) == 0 ? (int)(1 + rnd(100)) : 100);
    ww(0xBB3F0u, rnd(12) == 0 ? 0 : 1);
    wd(0x10F2C0u, rnd(1000));
    wd(0x10F2C4u, rnd(50));
    switch (op->fn) {
    case F639E8:
        for (i = 0; i < 40; i++) {   /* one-nation coastal city sites */
            t = (int)g_coastal[rnd(g_ncoastal)];
            if (t >= 157 || !(rb(0xE1639u + 35u * (uint32_t)t) & 0x40))
                continue;
            k = 1 + rnd(9);
            wb(ar(t + 1) + 1u, rnd(5) ? 1 : 2);
            wb(ar(t + 1) + 0x0Eu + (uint32_t)k, 1 + rnd(4));
        }
        break;
    case F61254:
    case F62840:
        break;
    case F66B54:
        m = 1 + rnd(9);
        op->a[0] = m;
        for (i = 0; i < 1 + rnd(5); i++)
            put_city(n, rnd(157));
        for (i = 0; i < rnd(5); i++)
            put_city(m, rnd(157));
        op->a[1] = 1 + rnd(3);
        break;
    case F66C2C:
        for (i = 0; i < 1 + rnd(8); i++)
            put_city(n, rnd(157));
        for (k = 0; k < 4; k++)   /* some cities in the start territories */
            if (rnd(2) && sb(nt + 0x51u) > 0)
                wd(0xC0618u + 16u * (uint32_t)n + 4u * (uint32_t)k, rb(nt + 0x52u + (uint32_t)rnd(sb(nt + 0x51u))) - 1);
        op->a[1] = 1 + rnd(9);
        break;
    case F680DC: {
        int need = 0, tok;
        m = 1 + rnd(12);
        for (i = 0; i < m; i++)
            f57E28(rnd(157), n, 1 + rnd(3));
        for (i = 0; i < sb(nt + 0x60u); i++) {
            t = rb(nt + 0x61u + (uint32_t)i) - 1;
            tok = sb(ar(t + 1) + 0x0Eu + (uint32_t)n);
            need += tok >= 2 ? 2 : tok == 1;
            if (rnd(3) == 0)
                wb(0xF3580u + 314u * (uint32_t)(n - 1) + 2u * (uint32_t)t, rnd(256));
            if (rnd(3) == 0)
                wb(0xF3581u + 314u * (uint32_t)(n - 1) + 2u * (uint32_t)t, rnd(256));
            if (rnd(5) == 0)
                wb(0xE2C1Au + 3u * (uint32_t)t, rb(0xE2C1Au + 3u * (uint32_t)t) | 2);
        }
        if (need < 2)
            need = 2;
        wb(nt + 0x99u, rnd(6) == 0 ? 0 : rnd(need));
        op->a[1] = need;
        break;
    }
    case F5BF30:
        op->a[1] = rnd(4);
        op->a[2] = rnd(2);
        wb(0xF1528u + 199u * (uint32_t)(n - 1), rnd(22) - 2);
        break;
    }
    /* the phase's snapshot, then a few changes after it (f7A610 brings it back) */
    snapshot();
    if (rnd(2))
        wb(nt + 0x3Eu, rb(nt + 0x3Eu) + 1);
    if (rnd(2))
        f57E28((int)g_coastal[rnd(g_ncoastal)], n, 1);
    op->orig = rnd(3) == 0;
    op->clk0 = (uint32_t)rnd(100000);
    for (i = 0; i < 4; i++) {
        static const uint32_t costs[8] = { 0, 0, 1, 6, 30, 200, 900, 5000 };
        op->cost[i] = costs[rnd(8)];
    }
    if (rnd(2))
        op->cost[1] = op->cost[2] = op->cost[3] = op->cost[0];
    op->bias = rnd(9) - 4;
    return 0;
}

static void run_one(const Op *op, int model, Res *res)
{
    volatile int fn = op->fn;
    int32_t cnt = 0, list[160];
    int i;

    g_tr = model ? g_tr_model : g_tr_port;
    g_trlen = 0;
    g_tr[0] = 0;
    g_quits = 0;
    g_rand = 0;
    g_calls = 0;
    g_ds = model ? g_model : g_port;
    memcpy(g_ds, g_base, DS_SIZE);
    if (load_heap_ptr(D32(0xF5240u), 0x2B20) == NULL) {   /* the World State Stack freed by a run before: again */
        for (i = 0; i < 30; i++)
            setD32(0xF5240u + 4u * (uint32_t)i, 0);
        f7A8D0();
        for (i = 0; i < 30; i++)
            if (D32(0xF5240u + 4u * (uint32_t)i) != *(uint32_t *)(void *)(g_base + 0xF5240u - DS_BASE + 4u * (uint32_t)i))
                tr("STACK HANDLES MOVED ");
    }
    g_original = op->orig;
    g_clk = op->clk0;
    g_ticks = 0;
    memcpy(g_cost, op->cost, sizeof g_cost);
    g_bias = op->bias;
    g_n = op->a[3];
    res->ret = 0;
    memset(list, 0, sizeof list);
    if (setjmp(g_jb) == 0) {
        switch (fn) {
        case F639E8: if (model) m639E8(op->a[3], &cnt, list); else f639E8(op->a[3], &cnt, list); break;
        case F61254: if (model) m61254(op->a[3]); else f61254(op->a[3]); break;
        case F62840: res->ret = model ? m62840(op->a[3]) : f62840(op->a[3]); break;
        case F66B54: if (model) m66B54(op->a[0], op->a[3], op->a[1]); else f66B54(op->a[0], op->a[3], op->a[1]); break;
        case F66C2C: if (model) m66C2C(op->a[3], op->a[1]); else f66C2C(op->a[3], op->a[1]); break;
        case F680DC: if (model) m680DC(op->a[3], op->a[1]); else f680DC(op->a[3], op->a[1]); break;
        case F5BF30: if (model) m5BF30(op->a[3], op->a[1], op->a[2]); else f5BF30(op->a[3], op->a[1], op->a[2]); break;
        }
    }
    if (fn == F639E8) {
        res->ret = cnt;
        res->hash = 2166136261u;
        for (i = 0; i < cnt && i < 160; i++)
            res->hash = (res->hash ^ (uint32_t)list[i]) * 16777619u;
    } else
        res->hash = 0;
    res->clk = g_clk;
    res->quits = g_quits;
    res->rands = g_rand;
    g_original = 0;
}

/* coverage (port runs) */
static long g_cov[16];
enum { C_SCRAP_ALL, C_OPTIONS, C_PICK, C_SEARCH, C_PICKED, C_NONE, C_BUDGET_TIME, C_BUDGET_OVERRUN, C_STAGE2, C_150,
       C_ALLOC160, C_ROBIN, C_SHORT0, C_WAITED, C_OFF };

static int scenario(const uint8_t *save, Op *op, int verbose)
{
    Res rp, rm;
    char what[200];
    int ok, i;
    uint32_t a;

    g_ds = g_base;
    memcpy(g_base, save, DS_SIZE);
    memset(op->a, 0, sizeof op->a);
    if (scenario_setup(op) < 0)
        return -1;
    run_one(op, 0, &rp);
    run_one(op, 1, &rm);
    ok = 1;
    for (a = 0; a < DS_SIZE && g_port[a] == g_model[a]; a++)
        ;
    snprintf(what, sizeof what, "A %s(%d,%d,%d,n %d) orig %d", FNAME[op->fn], op->a[0], op->a[1], op->a[2], op->a[3], op->orig);
    if (a < DS_SIZE) {
        ok = 0;
        if (verbose && g_fail < 30)
            printf("  %s: data object differs first at 0x%X (port %02X model %02X)\n", what, (unsigned)(a + DS_BASE),
                   g_port[a], g_model[a]);
    }
    if (strcmp(g_tr_port, g_tr_model) != 0) {
        ok = 0;
        if (verbose && g_fail < 30) {
            for (i = 0; g_tr_port[i] && g_tr_port[i] == g_tr_model[i]; i++)
                ;
            printf("  %s: call logs differ at %d: port \"%.80s\" model \"%.80s\"\n", what, i, g_tr_port + i, g_tr_model + i);
        }
    }
    if (rp.ret != rm.ret || rp.hash != rm.hash || rp.clk != rm.clk || rp.quits != rm.quits || rp.rands != rm.rands) {
        ok = 0;
        if (verbose && g_fail < 30)
            printf("  %s: ret %d/%d hash %08X/%08X clk %u/%u quits %d/%d rands %ld/%ld\n", what, rp.ret, rm.ret,
                   (unsigned)rp.hash, (unsigned)rm.hash, (unsigned)rp.clk, (unsigned)rm.clk, rp.quits, rm.quits,
                   rp.rands, rm.rands);
    }
    if (verbose) {
        check(ok, what);
        check(rp.quits == 0, "A no quit");
        /* coverage, from the port's run */
        g_ds = g_port;
        if (op->fn == F61254) {
            if (strstr(g_tr_port, "RAND"))
                g_cov[C_PICK]++;
            if (strstr(g_tr_port, "considers ship maintenance. "))
                g_cov[C_OPTIONS]++;
        }
        if (op->fn == F62840) {
            if (rp.ret == -1 && sw(0xBB3F0u) == 0)
                g_cov[C_OFF]++;
            else if (rd(0x110158u) > 0 || rd(0x110144u) > 0)
                g_cov[C_SEARCH]++;
            if (rp.ret >= 0)
                g_cov[C_PICKED]++;
            if ((uint32_t)rd(0x10F2D8u) * (uint32_t)rd(0x10F2E8u) > 9500u || rd(0x10F2D4u) > 9500)
                g_cov[C_BUDGET_TIME]++;
            if (rd(0x110158u) >= 2)
                g_cov[C_STAGE2]++;
            {
                const char *p = g_tr_port;
                int calls = 0;
                while ((p = strstr(p, "461BC(")) != NULL) {
                    calls++;
                    p++;
                }
                if (calls > 150)
                    g_cov[C_150]++;
            }
            if (rd(0x10F30Cu) >= 19 && rd(0x110158u) >= 1)
                g_cov[C_BUDGET_OVERRUN]++;
        }
        if (op->fn == F680DC) {
            if (strstr(g_tr_port, "48D44"))
                g_cov[C_ROBIN]++;
            if (sb(nat(op->a[3]) + 0x99u) == 0)
                g_cov[C_SHORT0]++;
        }
        if (rp.clk != op->clk0)
            g_cov[C_WAITED]++;
    }
    g_ds = g_base;
    return ok;
}

static void part_a(uint8_t *const *saves, int nsaves, int per)
{
    Op op;
    int s, f, i, r, skipped = 0;
    long before = g_fail, runs = 0;

    for (s = 0; s < nsaves; s++)
        for (f = 0; f < NFN; f++)
            for (i = 0; i < per; i++) {
                op.fn = f;
                r = scenario(saves[s], &op, 1);
                if (r < 0) {
                    skipped++;
                    continue;
                }
                runs++;
            }
    printf("A. %ld differential runs (%d skipped), %ld failed\n", runs, skipped, g_fail - before);
    printf("   coverage: f61254 options %ld, draws %ld; f62840 searches %ld, picks %ld, off %ld, over budget %ld "
           "(19+ sites at stage 1: %ld), stage 2 %ld, 150 options %ld; f680DC round robin %ld, stock 0 %ld; waits %ld\n",
           g_cov[C_OPTIONS], g_cov[C_PICK], g_cov[C_SEARCH], g_cov[C_PICKED], g_cov[C_OFF], g_cov[C_BUDGET_TIME],
           g_cov[C_BUDGET_OVERRUN], g_cov[C_STAGE2], g_cov[C_150], g_cov[C_ROBIN], g_cov[C_SHORT0], g_cov[C_WAITED]);
}

/* ---- Part B: the broken models must be caught ---- */

static void part_b(uint8_t *const *saves, int nsaves)
{
    static const struct { int mut, fn; const char *what; } M[] = {
        { 1, F61254, "f61254 options sorted ascending" },
        { 2, F61254, "f61254 one tie fewer in the draw" },
        { 3, F61254, "f61254 treasury == used ships searches" },
        { 4, F61254, "f617C0 counts bits 0..2 only" },
        { 5, F62840, "f62840 levy check with ships + j" },
        { 6, F62840, "f62840 keeps options equal to the base" },
        { 7, F62840, "f62840 ties ignore the ship count" },
        { 8, F62840, "f63D10 re-admits sites without the score test" },
        { 9, F62840, "f63D10 first pair row starts at i + 1" },
        { 10, F62840, "f6332C budget 9600" },
        { 11, F62840, "f6332C stage-2 start min(avail, 3)" },
        { 12, F680DC, "f680DC cuts tokens - limit" },
        { 13, F680DC, "f680DC alloc[160] = 0" },
        { 14, F66C2C, "f66C2C one pass only" },
        { 15, F5BF30, "f5BF30 reads record n" },
        { 16, F66B54, "f66B54 marks area +0x3B + m" },
        { 17, F62840, "f62840 capacity counts embarks" },
        { 18, F680DC, "f680DC's wait scaled by /C" },
        { 19, F61254, "f61254 without the snapshot f7A610" },
        { 20, F62840, "f62840 without the peek per option" },
    };
    Op op;
    unsigned m;
    int i, caught, r;
    char what[160];

    for (m = 0; m < sizeof M / sizeof M[0]; m++) {
        caught = 0;
        for (i = 0; i < 20000 && !caught; i++) {
            op.fn = M[m].fn;
            g_mut = M[m].mut;
            r = scenario(saves[i % nsaves], &op, 0);
            g_mut = 0;
            if (r == 0)
                caught = 1;
        }
        snprintf(what, sizeof what, "B mutant %d (%s) caught after %d scenarios", M[m].mut, M[m].what, i);
        check(caught, what);
        printf("B. mutant %2d %-45s %s (%d scenarios)\n", M[m].mut, M[m].what, caught ? "caught" : "NOT CAUGHT", i);
    }
}

/* ---- Part C: data facts and hand vectors ---- */

static void part_c(const uint8_t *save)
{
    static const int32_t tri[19] = { 0, 1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 66, 78, 91, 105, 120, 136, 153, 171 };
    int i, ok = 1, n;
    char what[160];

    for (i = 0; i < 19; i++)
        ok &= rd(0xC7618u + 4u * (uint32_t)i) == tri[i];
    check(ok, "C 0xC7618[0..18] are the triangular numbers 0..171");
    snprintf(what, sizeof what, "C 0xC7618[19] = 0x%X (a string pointer, read for 19+ sites)", (unsigned)rd(0xC7618u + 76u));
    check(rd(0xC7618u + 76u) > 9500, what);
    printf("C. %s\n", what);
    check(rd(0xBC800u) == 1 && rd(0xBC804u) == 2 && rd(0xBC808u) == 4 && rd(0xBC80Cu) == 8, "C 0xBC800 = 1, 2, 4, 8");
    check(strcmp(DSTR(rd(0xBB404u)), "|") == 0 && strcmp(DSTR(rd(0xBB408u)), "/") == 0
          && strcmp(DSTR(rd(0xBB40Cu)), "-") == 0 && strcmp(DSTR(rd(0xBB410u)), "\\") == 0, "C the spinner | / - \\");
    check(sw(0xBB3E8u) == 100 && sw(0xBB3F0u) == 1 && rd(0xBB3F4u) == 1, "C [0xBB3E8] = 100, [0xBB3F0] = 1, [0xBB3F4] = 1");
    check(strcmp(DSTR(0xB6358u), "%s considers building ships. (%d,?,?)") == 0
          && strcmp(DSTR(0xB637Eu), DSTR(0xB6358u)) == 0
          && strcmp(DSTR(0xB63A4u), "%s considers building ships. (0,%d,%d)") == 0
          && strcmp(DSTR(0xB621Au), "%s considers ship maintenance. %s") == 0
          && strcmp(DSTR(0xB8A1Cu), "%s removes a city in %s") == 0
          && strcmp(DSTR(0xB5D16u), "%s, choosing number units to transfer from stock to treasury.") == 0,
          "C the strings");
    check(rd(0x10F2F4u) == 0 && rd(0x10F2F8u) == 1 && rd(0x10F2FCu) == 4 && rd(0x10F300u) == 10 && rd(0x10F304u) == 20
          && rd(0x10F308u) == 35, "C the thresholds 0x10F2F4 = 0, 1, 4, 10, 20, 35 (f60A80)");

    /* f6332C hand vectors: 10 options with first sites 0..9, 20 sites (none excluded) */
    for (i = 0; i < 4; i++) {
        static const struct { uint32_t total, count, elapsed; int32_t sites, from, e8, ec, excl; } V[4] = {
            { 0, 0, 0, 12, 4, 78, 20, 0 },          /* fits: avg 0, [0x10F2E8] = C[12], E0 = min(12, 4), [0x10F2EC] = T[4] */
            { 200, 1, 0, 12, -1, 45, -1, 1 },       /* avg 200 * C[12] = 15600 > 9500: rem 47 -> limit 9 (45 < 47 < 55) */
            { 1, 1, 0, 20, -1, 120, -1, 0 },        /* 20 sites: C[20] is past the table: over; rem 9500 -> limit 15 */
            { 0, 1, 9600, 12, -1, 6, -1, 7 },       /* over by time alone: limit 3, the options 3..9 excluded */
        };
        int t, k;
        g_ds = g_port;
        memcpy(g_ds, save, DS_SIZE);
        g_clk = 1000 + V[i].elapsed;
        wd(0x10F2C8u, 1000);
        wd(0x10F2CCu, (int32_t)V[i].total);
        wd(0x10F2DCu, (int32_t)V[i].count);
        wd(0x10F30Cu, V[i].sites);
        for (t = 0; t < V[i].sites; t++)
            wd(0x10F310u + 4u * (uint32_t)t, 10 + t);
        memset(DP(0x10F220u), 0, 160);
        wd(0x110144u, 10);
        for (k = 0; k < 10; k++) {
            memset(DP(0x10F6B8u + 18u * (uint32_t)k), 0, 18);
            ww(0x10F6B8u + 18u * (uint32_t)k, 10 - k);
            wb(0x10F6BBu + 18u * (uint32_t)k, 10 + k);
        }
        f6332C(3);
        k = 0;
        for (t = 0; t < 160; t++)
            k += rb(0x10F220u + (uint32_t)t);
        snprintf(what, sizeof what, "C f6332C vector %d: E0 %d E8 %d EC %d excluded %d", i, rd(0x10F2E0u), rd(0x10F2E8u),
                 rd(0x10F2ECu), k);
        if (V[i].from >= 0)
            check(rd(0x10F2E0u) == V[i].from && rd(0x10F2E8u) == V[i].e8 && rd(0x10F2ECu) == V[i].ec && k == V[i].excl, what);
        else
            check(rd(0x10F2E8u) == V[i].e8 && k == V[i].excl, what);
        printf("%s\n", what);
    }
    /* the busy waits keep the original's start and ignore /C: f5BF30 is shown with 2500 ms (scaled), the others not */
    for (n = 0; n < 2; n++) {
        Op op;
        Res r;
        memset(&op, 0, sizeof op);
        g_ds = g_base;
        memcpy(g_base, save, DS_SIZE);
        op.fn = F680DC;
        op.a[3] = 3;
        op.a[1] = 50;
        wd(0xBB3F4u, n ? 10 : 1);
        op.clk0 = 500;
        run_one(&op, 0, &r);
        snprintf(what, sizeof what, "C f680DC waits 1 s from its start with [0xBB3F4] = %d: clock %u -> %u", n ? 10 : 1,
                 500u, (unsigned)r.clk);
        check(r.clk >= 600 && r.clk < 606, what);
        printf("%s\n", what);
    }
    g_ds = g_base;
}

/* ---- loading ---- */

static int load_state(const char *gamedir, const char *path, uint8_t *out)
{
    int16_t ver;
    char desc[64];
    FILE *fp;
    int v;
    uint32_t a;

    memcpy(g_ds, g_ds_live, DS_SIZE);
    fp = fopen(path, "rb");
    if (fp == NULL) {
        printf("cannot open %s\n", path);
        return -1;
    }
    f5F05C(&ver, desc, fp);
    fclose(fp);
    v = 7 * ((D16(0xE1624u) & 2) != 0) + S32(0xF40F0u) - 2;
    setD16(GAME_MAP_VARIANT, (uint16_t)v);
    f12B00();
    snprintf((char *)DP(0xF5820u), 0x64, "%s\\", gamedir);
    f3EED8();
    setD32(0xCAC54u, 0xFFFFFFFFu);   /* both stacks empty */
    setD32(0xCAC58u, 0xFFFFFFFFu);
    /* title.c's f60A80 (run before main): the running sums of 0xC7618[0..5] */
    a = D32(0xC7618u);
    setD32(0x10F2F4u, a);
    a += D32(0xC761Cu);
    setD32(0x10F2F8u, a);
    a += D32(0xC7620u);
    setD32(0x10F2FCu, a);
    a += D32(0xC7624u);
    setD32(0x10F300u, a);
    a += D32(0xC7628u);
    setD32(0x10F304u, a);
    a += D32(0xC762Cu);
    setD32(0x10F308u, a);
    memcpy(out, g_ds, DS_SIZE);
    printf("loaded %s: \"%s\" version 0x%X, players %d, variant %d, phase %d, round %d\n", path, desc, ver, S32(0xF40F0u),
           v, D8(0xF4102u), S16(0xF40FCu));
    return 0;
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:\\work\\advciv_run";
    const char *playdir = argc > 2 ? argv[2] : "C:\\work\\advciv_play";
    int per = argc > 3 ? atoi(argv[3]) : 300;
    char path[600];
    uint8_t *saves[4];
    int nsaves = 0, t, i;
    static const char *const runsaves[3] = { "savgame0.dat", "savgame3.dat", "savgame7.dat" };

    setvbuf(stdout, NULL, _IONBF, 0);
    if (state_init(gamedir) != 0) {
        printf("cannot load the data object from %s\n", gamedir);
        return 2;
    }
    if (chdir(gamedir) != 0) {
        printf("cannot enter %s\n", gamedir);
        return 2;
    }
    f12394();
    f3EA7C();
    f7A8D0();
    g_ds_live = malloc(DS_SIZE);
    g_base = malloc(DS_SIZE);
    g_port = malloc(DS_SIZE);
    g_model = malloc(DS_SIZE);
    for (i = 0; i < 4; i++)
        saves[i] = malloc(DS_SIZE);
    if (!g_ds_live || !g_base || !g_port || !g_model || !saves[0] || !saves[1] || !saves[2] || !saves[3])
        return 2;
    memcpy(g_ds_live, g_ds, DS_SIZE);

    snprintf(path, sizeof path, "%s\\SAVGAMEA.DAT", playdir);
    if (load_state(playdir, path, saves[nsaves]) == 0)
        nsaves++;
    for (i = 0; i < 3; i++) {
        snprintf(path, sizeof path, "%s\\%s", gamedir, runsaves[i]);
        if (load_state(gamedir, path, saves[nsaves]) == 0)
            nsaves++;
    }
    if (nsaves == 0)
        return 2;
    g_ds = saves[0];
    g_ncoastal = 0;
    for (t = 0; t < 160; t++)
        if (D8(0xE1638u + 35u * (uint32_t)t) & 0x02)
            g_coastal[g_ncoastal++] = (uint32_t)t;
    printf("%d coastal territories\n", g_ncoastal);

    memcpy(g_base, saves[0], DS_SIZE);
    g_ds = g_base;
    part_c(saves[0]);
    part_a(saves, nsaves, per);
    part_b(saves, nsaves);

    printf("%ld checks, %ld failed\n", g_tests, g_fail);
    return g_fail == 0 ? 0 : 1;
}
