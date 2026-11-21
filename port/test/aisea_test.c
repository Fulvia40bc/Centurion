/* aisea_test [GAMEDIR [PLAYDIR [DUMP]]]: unit tests for module AISEA (aisea.c) that need no game run.

   It loads the data object from GAMEDIR/AC_MAIN.EXE as the port does (state_init; GAMEDIR defaults to the run copy
   C:\work\advciv_run, PLAYDIR to C:\work\advciv_play, both only read), the territory records (f12394, terrflag.dat:
   the population limits are not in the saves), mapdataz.dat (f3EA7C: the record handles at 0xFF64C that f64BB8's
   out-of-range rows read), then a save through saveload.c's f5F05C, the map variant's in-play table (f12B00) and the
   adjacency, the land matrix and <players>.RL (gfxload.c's f3EED8), and allocates the World State Stack (f7A8D0).
   aisea.c is compiled with -DAISEA_TEST, so its calls into the unported AIQ / AIPLAN routines (f495C0, f49790, f48028,
   f48D44, f7B044, f7C838, f49A1C, f46BB8) and the two ported silent phases f5AAA4(0) / f5C238(0) go to recorders here:
   each logs its call (name and arguments) and answers from the current state, deterministically, so the order and
   the state at each call are checked. The primitives aisea.c calls (route.c, rules.c, move.c, stack.c, mstack.c,
   wqsort.c, helpers2.c) are the real, separately tested ones. game_rand f78AE4 counts its calls (none expected).
     A. random differential runs of all nine routines against a second implementation written here from the listing
        (a transcription that follows the original's labels and frame offsets, not aisea.c), each on its own copy of
        the same starting state: the two real saves (SAVGAMEA.DAT of PLAYDIR, the R4 autosave with seven computers;
        savgame0.dat of GAMEDIR, the R4 game at the start of MOVEMENT) with random edits through the primitives (ships
        and their points, positions on the second coasts 157..159 included, tokens, movable counts, marked sites and
        their +0x39 needs, caches, scores, the pre-rolled table, Clothmaking / Agriculture / Astronomy, both values of
        g_original). After each run the whole data object, the call log, the result and the out parameter must equal
        the model's, and nothing may draw a random number or quit.
     B. a mutation check: deliberately broken copies of the model (one change each) must each be caught by part A's
        scenarios.
     C. data facts: the code bytes 0xC1938[0..4] = 04 23 28 0F 27 and the out-of-table bytes 0xC1947 / 0xC195F
        (f36918, risk 8: both 0); with DUMP (default C:\DOSBox-X\advciv\M7_R4MV_0.BIN, a DOSBox-X dump of the data
        object during an R4 movement, read if present) the --original rebuild of the bytes 0xFF645..0xFF81E that
        f64BB8 reads past the land matrix must equal the original's; the computers' ships of the R4 saves (for the
        DOSBox-X check plan).
   Prints each failure (the first few), the counts per part, and exits 0 when all pass.

   Build (from the repo root; artefacts outside the repo):
     clang -std=c11 -Wall -Wextra -DAISEA_TEST -Iport/src port/test/aisea_test.c port/src/aisea.c port/src/route.c
       port/src/rules.c port/src/move.c port/src/stack.c port/src/mstack.c port/src/wqsort.c port/src/helpers2.c
       port/src/saveload.c port/src/load.c port/src/gfxload.c port/src/state.c port/src/le_image.c -o aisea_test.exe */

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
#include "../src/aisea.h"
#include "../src/move.h"
#include "../src/mstack.h"
#include "../src/route.h"
#include "../src/rules.h"
#include "../src/saveload.h"
#include "../src/stack.h"
#include "../src/wqsort.h"

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

#define TRSIZE (1 << 18)
static char g_tr_port[TRSIZE], g_tr_model[TRSIZE];
static char *g_tr = g_tr_port;
static size_t g_trlen;

static void tr(const char *fmt, ...)
{
    va_list ap;
    if (g_trlen >= TRSIZE - 200)
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
void f3CBBC(void) { unexpected("3CBBC"); }
void f3CC68(void) { unexpected("3CC68"); }
void f3CCBC(void) { unexpected("3CCBC"); }
void f3D52C(int dx, int dy) { (void)dx; (void)dy; unexpected("3D52C"); }
void f3DA60(int t) { (void)t; unexpected("3DA60"); }
int  f3E89C(int t) { (void)t; unexpected("3E89C"); return 0; }
int  f3E8FC(int t) { (void)t; unexpected("3E8FC"); return 0; }
void f3F558(void) { unexpected("3F558"); }
void f3F624(int t, int c) { (void)t; (void)c; unexpected("3F624"); }
void f3F670(void) { unexpected("3F670"); }
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
int  fg_setcolor(int color) { (void)color; return 0; }
void fg_vbclose(void) { }
int  fg_vbopen(int handle) { (void)handle; return 0; }
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
    return (int)((s >> 16) & 0x7FFF);
}

/* ---- data helpers (independent of aisea.c) ----------------------------------------------------------------------- */

static int32_t rb(uint32_t a) { return D8(a); }                              /* movzx byte */
static int32_t sb(uint32_t a) { return (int32_t)(D32(a - 3u)) >> 24; }       /* dword [a - 3] sar 24 */
static int32_t sw(uint32_t a) { return (int32_t)(D32(a - 2u)) >> 16; }       /* dword [a - 2] sar 16 */
static int32_t rd(uint32_t a) { return (int32_t)D32(a); }
static void wb(uint32_t a, int32_t v) { setD8(a, (uint8_t)v); }
static void ww(uint32_t a, int32_t v) { setD16(a, (uint16_t)v); }
static void wd(uint32_t a, int32_t v) { setD32(a, (uint32_t)v); }
static void incb(uint32_t a) { wb(a, rb(a) + 1); }
static void decb(uint32_t a) { wb(a, rb(a) - 1); }
static uint32_t ar(int32_t r) { return 0xE31B8u + (uint32_t)(r * 69); }      /* area record r */
static int32_t al(int32_t u) { return rb(0xC803Cu + (uint32_t)u); }

/* ---- the recorders (shared by the port's and the model's runs) ---------------------------------------------------- */

int aisea_test_f495C0(int t, int n, int mode, int32_t *enemy)
{
    tr("495C0(%d,%d,%d) ", t, n, mode);
    *enemy = (t * 7 + n) % 9 + 1;
    return (sb(ar(t + 1) + 0x0Eu + (uint32_t)n) + t + mode) & 3;
}

int aisea_test_f49790(int r, int t, int n, int enemy)
{
    tr("49790(%d,%d,%d,%d) ", r, t, n, enemy);
    return (r + t + enemy + n) % 3;
}

int aisea_test_f49A1C(int t, int n, int k, int exclude)
{
    tr("49A1C(%d,%d,%d,%d) ", t, n, k, exclude);
    return (t + n + rb(ar(t + 1) + 1u)) & 1;
}

void aisea_test_f48028(int32_t *cnt, int32_t *list, int t, int n, int filter)
{
    int u;
    tr("48028(%d,%d,%d) ", t, n, filter);
    *cnt = 0;
    for (u = 0; u < 157; u++)
        if (rb(GAME_LAND_MATRIX + (uint32_t)(157 * t + u)) && rb(GAME_INPLAY + (uint32_t)u))
            list[(*cnt)++] = u;
}

void aisea_test_f48D44(int32_t *cnt, int32_t *list, int n, int t, int flags)
{
    int i, j, x;
    tr("48D44(%d,%d,%d,%d) ", *cnt, n, t, flags);
    for (i = 1; i < *cnt; i++) {   /* stable, by n's tokens, most first */
        x = list[i];
        for (j = i; j > 0 && sb(ar(list[j - 1] + 1) + 0x0Eu + (uint32_t)n) < sb(ar(x + 1) + 0x0Eu + (uint32_t)n); j--)
            list[j] = list[j - 1];
        list[j] = x;
    }
}

int aisea_test_f7B044(int n)
{
    int32_t s = 0;
    int c;
    tr("7B044(%d) ", n);
    for (c = 0; c < 157; c++)
        s += sb(ar(c + 1) + 0x0Eu + (uint32_t)n) * ((c * 13) % 17);
    s += 3 * rd(0x107D3Cu) - 120;
    if (s > 0x7FFF)
        s = 0x7FFF;
    return s;
}

int aisea_test_f7C838(int t, int n, int k, int32_t *left)
{
    tr("7C838(%d,%d,%d) ", t, n, k);
    *left = 2;
    return sb(ar(t + 1) + 0x0Eu + (uint32_t)n) % 3;
}

void aisea_test_f46BB8(int n)
{
    uint32_t nat = 0xF14CCu + 199u * (uint32_t)n;
    int t;
    tr("46BB8(%d) ", n);
    wd(0x108164u + 4u * (uint32_t)n, 1);
    if (rb(nat + 0x60u) > 0) {   /* the first area of n's list: movable = tokens */
        t = rb(nat + 0x61u) - 1;
        wb(ar(t + 1) + 0x24u + (uint32_t)n, rb(ar(t + 1) + 0x0Eu + (uint32_t)n));
    }
}

void aisea_test_f5AAA4(int show)
{
    tr("5AAA4(%d) ", show);
    wd(0x109E24u, rd(0x109E24u) + 1);
    wb(ar(12) + 0x25u, 0);   /* nation 1's movable tokens in territory 11 (in the world state: undone by the pop) */
}

void aisea_test_f5C238(int show)
{
    tr("5C238(%d) ", show);
    wb(0xF5934u, rb(0xF5934u) + 1);
}

/* ======================================================================================================================
   The model: the nine routines transcribed from the listing (labels = the original's addresses, locals = its frame
   offsets). Callees: the recorders above and the real primitives.
   ==================================================================================================================== */

/* ---- 0x374E0 ---- */

static int32_t m374E0(int32_t j, int32_t t, int32_t n, int32_t max, int32_t a)
{
    uint8_t v110[199];
    uint8_t v66c[10 * 73];
    int32_t v390[160];
    int32_t v0c = 0, v14 = n - 1, v18 = 0xF14CC + n * 199, v3c = 0, v38 = 0, v1c, v2c, v28, v34 = 0, v30 = 0, v24, v20;
    int32_t eax;

    memcpy(v110, DP((uint32_t)v18), 199);
    v1c = 0x108820 + j * 16;
    v2c = al(rb((uint32_t)v1c));
    v28 = v2c + 1;
    memset(DP(0x108820u + (uint32_t)(j * 16) + 8u), 0, 6);
    memset(v390, 0, sizeof v390);
#define TAKE()                                                                                     \
    do {                                                                                           \
        if (rd(0xF8194u + 4u * (uint32_t)v2c) == -1) {                                             \
            wd(0xF8194u + 4u * (uint32_t)v2c, v0c);                                                \
            memcpy(v66c + 73 * v0c + 4, DP(ar(v28)), 69);                                          \
            memcpy(v66c + 73 * v0c, &v2c, 4);                                                      \
            v0c++;                                                                                 \
        }                                                                                          \
        v3c++;                                                                                     \
        decb((uint32_t)v18 + 0x5Du);                                                               \
        incb(0x108828u + (uint32_t)(j * 16 + v38));                                                \
        f57CE8(v2c, n, 1, 1);                                                                      \
        wd(0xF8414u, 1);                                                                           \
    } while (0)
L37598:
    if (rb((uint32_t)v1c) == 0xFF || v38 > 5 || v3c >= 5 || v3c >= max)
        goto L37980;
    eax = sb(0xF84B8u + (uint32_t)v2c);
    if (eax != -1)
        goto L378FE;
    if (sb(ar(v28) + 0x24u + (uint32_t)n) <= 0 || sb((uint32_t)v18 + 0x5Du) <= 0 || v2c == t)
        goto L37956;
    if (rb(0xE2C1Au + (uint32_t)(3 * v2c)) & 2)
        goto L37956;
    if (v38 == 0 && v2c == t)
        goto L37956;
    if (sb(0xF95FCu + (uint32_t)(157 * v2c + t)) != 0)
        goto L37956;
    if (sb(0xF8418u + (uint32_t)v2c) == -1) {
        v34 = aisea_test_f495C0(v2c, n, a, &v30);
        wb(0xF8418u + (uint32_t)v2c, aisea_test_f49790(v34, v2c, n, v30));
    }
    v34 = sb(0xF8418u + (uint32_t)v2c);
    if ((int8_t)rb(ar(v28) + 0x24u + (uint32_t)n) >= (int8_t)rb(ar(v28) + 0x0Eu + (uint32_t)n))
        goto L377E6;
L376F0:
    if (sb(ar(v28) + 0x24u + (uint32_t)n) <= v34 || sb((uint32_t)v18 + 0x5Du) <= 0 || v3c >= 5 || v3c >= max)
        goto L37956;
    TAKE();
    goto L376F0;
L377E6:
    if (v34 == 0 && !(rb(0xF3580u + (uint32_t)(314 * v14 + 2 * v2c)) & 0x80))
        v34 = MUT(1) ? 2 : 1;
L37808:
    if (sb(ar(v28) + 0x0Eu + (uint32_t)n) <= v34 || sb((uint32_t)v18 + 0x5Du) <= 0 || v3c >= 5 || v3c >= max)
        goto L37956;
    TAKE();
    goto L37808;
L378FE:
    if (v390[v2c] != 0)
        goto L37956;
    v24 = sb(0xF84B8u + (uint32_t)v2c);
    while (v24 > 0 && v3c < 5) {
        v3c++;
        incb(0x108828u + (uint32_t)(j * 16 + v38));
        v24--;
    }
    v390[v2c] = 1;
L37956:
    v1c++;
    v2c = al(rb((uint32_t)v1c));
    v28 = v2c + 1;
    v38++;
    goto L37598;
L37980:
    if (v3c >= max || v3c >= 5 || MUT(2))
        goto L37BD6;
    v38 = 0;
    v1c = 0x108820 + j * 16;
    v2c = al(rb((uint32_t)v1c));
    v28 = v2c + 1;
    if (sb(0xF84B8u + (uint32_t)v2c) != -1)
        goto L37BD6;
L379D9:
    if (rb((uint32_t)v1c) == 0xFF || v38 > 5 || v3c >= 5 || v3c >= max)
        goto L37BD6;
    if (sb(ar(v28) + 0x24u + (uint32_t)n) <= 0 || v2c == t || sb((uint32_t)v18 + 0x5Du) <= 0 || v2c == t)
        goto L37BAC;
    if (rb(0xE2C1Au + (uint32_t)(3 * v2c)) & 2)
        goto L37BAC;
    if (v38 == 0 && v2c == t)
        goto L37BAC;
    if (sb(0xF95FCu + (uint32_t)(157 * v2c + t)) != 0)
        goto L37BAC;
    if (sb(ar(v28) + 0x0Eu + (uint32_t)n) != 1)
        goto L37BAC;
    if (aisea_test_f49A1C(v2c, n, 1, -1) != 0)
        goto L37B06;
    if (sb((uint32_t)v18 + 0x5Du) <= 0)
        goto L37BAC;
    if (!(rb(0xF3580u + (uint32_t)(314 * v14 + 2 * v2c)) & 0x80))
        goto L37BAC;
L37B06:
    TAKE();
L37BAC:
    v1c++;
    v2c = al(rb((uint32_t)v1c));
    v28 = v2c + 1;
    v38++;
    goto L379D9;
L37BD6:
    v38 = 0;
    v1c = 0x108820 + j * 16;
    while (rb((uint32_t)v1c) != 0xFF && v38 <= 5) {
        eax = al(rb((uint32_t)v1c));
        if (sb(0xF84B8u + (uint32_t)eax) == -1)
            wb(0xF84B8u + (uint32_t)eax, rb(0x108828u + (uint32_t)(j * 16 + v38)));
        v1c++;
        v38++;
    }
    if (v3c > 5)
        v3c = 5;
    memcpy(DP((uint32_t)v18), v110, 199);
    for (v20 = 0; v20 < v0c; v20++) {
        memcpy(&eax, v66c + 73 * v20, 4);
        memcpy(DP(ar(eax + 1)), v66c + 73 * v20 + 4, 69);
        wd(0xF8194u + 4u * (uint32_t)eax, -1);
    }
#undef TAKE
    return v3c;
}

/* ---- the comparators (independent of helpers2.c) ---- */

static int mc7(const void *a, const void *b)   { return (int)(int8_t)((const uint8_t *)b)[7] - (int)(int8_t)((const uint8_t *)a)[7]; }
static int mc8(const void *a, const void *b)   { return (int)(int8_t)((const uint8_t *)b)[8] - (int)(int8_t)((const uint8_t *)a)[8]; }
static int mc0E(const void *a, const void *b)
{
    const uint8_t *x = a, *y = b;
    return (int)(int16_t)(y[14] | y[15] << 8) - (int)(int16_t)(x[14] | x[15] << 8);
}

static void swap16(uint32_t a, uint32_t b)
{
    uint8_t t[16];
    memcpy(t, DP(b), 16);
    memcpy(DP(b), DP(a), 16);
    memcpy(DP(a), t, 16);
}

/* ---- 0x36918 ---- */

static void m36918(int32_t n, int32_t t, int32_t *v38p, int32_t code)
{
    uint8_t F[0x100];   /* the frame: byte [ebp - o] = F[0x100 - o]; zeroed (the port's fix of the stack garbage) */
#define FB(o) F[0x100 - (o)]
#define FD(o) (*(int32_t *)(void *)&F[0x100 - (o)])
    int8_t v0c = (int8_t)code;
    int32_t v30 = 0xF14CC + n * 199, v10 = 0, v28 = *v38p, v48, v2c, v20, v24, v4c, v44, v50, v3c, v18, v40, v14, v34,
            v1c, ebx, x, i;
    uint32_t e;

    memset(F, 0, sizeof F);
    f7A940();
    for (v48 = 0; rb((uint32_t)v30 + 0x40u) > v48; v48++) {
        if (v28 <= 0)
            continue;
        v2c = sw((uint32_t)v30 + 0x45u + 2u * (uint32_t)v48) - 1;
        v20 = al(v2c);
        if (f5843C(v2c, t, n, rb((uint32_t)v30 + 0x41u + (uint32_t)v48)) != 1)
            continue;
        f7AA1C();
        if (*v38p + 1 > sb(ar(v20 + 1) + 0x24u + (uint32_t)n))
            goto L36A6E;
        if (rb(0xF95FCu + (uint32_t)(157 * v20 + t)) != 0 || v2c == t || (rb(0xE2C1Au + (uint32_t)(3 * v20)) & 2))
            goto L36A6E;
        v24 = *v38p <= 5 ? *v38p : 5;
        f57CA4(v20, t, n, 1, v24);
        FD(0xEC - 28 * v10) = 1;
        FB(0xDE - 28 * v10) = (uint8_t)v2c;
        FB(0xE4 - 28 * v10) = (uint8_t)v24;
        FD(0xE8 - 28 * v10) = v24;
        FD(0xF0 - 28 * v10) = v48;
        f58888(v2c, t, n, rb((uint32_t)v30 + 0x41u + (uint32_t)v48));
        memcpy(&FB(0xDE - 28 * v10), DP(0x108820u), 6);
        v10++;
        goto L36F1C;
    L36A6E:
        f58888(v2c, t, n, rb((uint32_t)v30 + 0x41u + (uint32_t)v48));
        memset(DP(0xF8418u), 0xFF, 0xA0);
        memset(DP(0xF84B8u), 0xFF, 0xA0);
        memset(DP(0xF8194u), 0xFF, 0x280);
        for (v4c = 0; v4c < rd(0x109E00u); v4c++) {
            v44 = m374E0(v4c, t, n, v28, 0);
            wb(0x108827u + (uint32_t)(v4c * 16), v44 < v28 ? v44 : v28);
        }
        x = rd(0x109E00u);
        if (x < 20) {
            for (v50 = x - 2; v50 >= 0; v50--)
                for (v4c = 0; v4c <= v50; v4c++)
                    if ((int8_t)rb(0x108837u + (uint32_t)(16 * v4c)) > (int8_t)rb(0x108827u + (uint32_t)(16 * v4c)))
                        swap16(0x108820u + (uint32_t)(16 * v4c), 0x108830u + (uint32_t)(16 * v4c));
        } else {
            wc_qsort(DP(0x108820u), (size_t)x, 16, mc7);
        }
        v3c = 1;
        while (ds_valid(0x108826u + (uint32_t)(16 * v3c), 1) && rb(0x108826u) == rb(0x108826u + (uint32_t)(16 * v3c)))
            v3c++;
        ebx = 30000 / (v3c > 30000 ? 30000 : v3c);
        x = sw(0x112B0Au + 2u * (uint32_t)(MUT(3) ? n - 1 : n));
        if (x / ebx < v3c)
            v18 = x / ebx;
        else
            v18 = v3c - 1;
        if (sb(0x108827u + (uint32_t)(16 * v18)) == 0)
            goto L36F1C;
        memcpy(&FB(0xDE - 28 * v10), DP(0x108820u + (uint32_t)(16 * v18)), 6);
        memcpy(&FB(0xE4 - 28 * v10), DP(0x108828u + (uint32_t)(16 * v18)), 6);
        FD(0xE8 - 28 * v10) = 0;
        for (v4c = 0; v4c <= 5; v4c++)
            FD(0xE8 - 28 * v10) += sb(0x108828u + (uint32_t)(16 * v18 + v4c));
        FD(0xEC - 28 * v10) = 0;
        for (v4c = 0; v4c <= 5; v4c++)
            if (FB(0xDE - 28 * v10 - v4c) != 0xFF)
                FD(0xEC - 28 * v10)++;
        v28 -= FD(0xE8 - 28 * v10);
        FD(0xF0 - 28 * v10) = v48;
        v10++;
        v40 = v10 - 1;
        v4c = 0;
        v14 = FD(0xE8 - 28 * v40);
        if (v14 == 0)
            goto L36F1C;
        while (v4c <= 5 && v14 > 0) {
            v1c = (int8_t)FB(0xE4 - 28 * v40 - v4c);
            v14 -= v1c;
            f57CA4(al(FB(0xDE - 28 * v40 - v4c)), t, n, 1, v1c);
            decb((uint32_t)v30 + 0x41u + (uint32_t)FD(0xF0 - 28 * v40));
            v4c++;
        }
        while (v4c <= 5 && FB(0xDE - 28 * v40 - v4c) != t) {
            decb((uint32_t)v30 + 0x41u + (uint32_t)FD(0xF0 - 28 * v40));
            v4c++;
        }
        ww((uint32_t)v30 + 0x45u + 2u * (uint32_t)FD(0xF0 - 28 * v40), FB(0xDE - 28 * v40 - v4c) + 1);
    L36F1C:
        f7AE14();
    }
    f7AC60();
    for (v50 = 1; v50 < v10; v50++)
        for (v4c = v10 - 1; v4c >= v50; v4c--)
            if (MUT(4) ? FD(0xEC - 28 * (v4c - 1)) < FD(0xEC - 28 * v4c) : FD(0xEC - 28 * (v4c - 1)) > FD(0xEC - 28 * v4c)) {
                memcpy(&FB(0x84), &FB(0xF4 - 28 * (v4c - 1)), 28);
                memcpy(&FB(0xF4 - 28 * (v4c - 1)), &FB(0xF4 - 28 * v4c), 28);
                memcpy(&FB(0xF4 - 28 * v4c), &FB(0x84), 28);
            }
    for (v40 = 0; v40 < v10 && *v38p != 0 && rb((uint32_t)v30 + 0x40u) > v40; v40++) {
        v4c = 0;
        v14 = FD(0xE8 - 28 * v40);
        if (v14 == 0)
            continue;
        v34 = v14;
        while (v4c <= 5 && v14 > 0) {
            v1c = (int8_t)FB(0xE4 - 28 * v40 - v4c);
            e = 0x1079B8u + 6u * (uint32_t)rd(0x107D3Cu);
            wb(e + 2u, v1c);
            v14 -= v1c;
            *v38p -= v1c;
            f57CA4(al(FB(0xDE - 28 * v40 - v4c)), t, n, 1, v1c);
            e = 0x1079B8u + 6u * (uint32_t)rd(0x107D3Cu);
            wb(e + 4u, 3);
            wb(e + 3u, FD(0xF0 - 28 * v40));
            wb(e + 0u, FB(0xDE - 28 * v40 - v4c));
            wb(e + 5u, rb(0xC1938u + (uint32_t)(int32_t)(MUT(5) ? v0c + 1 : v0c)));
            if (sb(e + 2u) != 0)
                wd(0x107D3Cu, rd(0x107D3Cu) + 1);
            e = 0x1079B8u + 6u * (uint32_t)rd(0x107D3Cu);
            wb(e + 5u, rb(0xC1938u + (uint32_t)(int32_t)v0c));
            wb(e + 4u, 0x0A);
            wb(e + 3u, FD(0xF0 - 28 * v40));
            decb((uint32_t)v30 + 0x41u + (uint32_t)FD(0xF0 - 28 * v40));
            wb(e + 0u, FB(0xDE - 28 * v40 - v4c));
            i = rd(0x107D3Cu);
            wd(0x107D3Cu, i + 1);
            wb(0x1079B9u + 6u * (uint32_t)i, FB(0xDD - 28 * v40 - v4c));
            v4c++;
        }
        while ((0xDE - 28 * v40 - v4c) > 0x69 && FB(0xDE - 28 * v40 - v4c) != t) {
            e = 0x1079B8u + 6u * (uint32_t)rd(0x107D3Cu);
            wb(e + 5u, rb(0xC1938u + (uint32_t)(int32_t)v0c));
            wb(e + 4u, 0x0A);
            wb(e + 3u, FD(0xF0 - 28 * v40));
            decb((uint32_t)v30 + 0x41u + (uint32_t)FD(0xF0 - 28 * v40));
            wb(e + 0u, FB(0xDE - 28 * v40 - v4c));
            i = rd(0x107D3Cu);
            wd(0x107D3Cu, i + 1);
            wb(0x1079B9u + 6u * (uint32_t)i, FB(0xDD - 28 * v40 - v4c));
            v4c++;
        }
        ww((uint32_t)v30 + 0x45u + 2u * (uint32_t)FD(0xF0 - 28 * v40), FB(0xDE - 28 * v40 - v4c) + 1);
        e = 0x1079B8u + 6u * (uint32_t)rd(0x107D3Cu);
        wb(e + 5u, rb(0xC1938u + (uint32_t)(int32_t)v0c));
        wb(e + 2u, v34);
        wb(e + 4u, 5);
        wb(e + 3u, FD(0xF0 - 28 * v40));
        i = rd(0x107D3Cu);
        wd(0x107D3Cu, i + 1);
        wb(0x1079B9u + 6u * (uint32_t)i, t);
    }
#undef FB
#undef FD
}

/* ---- 0x4D29C ---- */

static int32_t m4D29C(int32_t v34, int32_t v14)
{
    int32_t v58[256], v18 = 0, v30, v24, v28, v1c, v20, v38, x;
    uint32_t nat = 0xF14CCu + 199u * (uint32_t)v14;

    f7A940();
    f7AA1C();
    f460C0(v14);
    v30 = rd(0xF5908u);
    wd(0xF5908u, 0);
    aisea_test_f46BB8(v14);
    wd(0xF5908u, v30);
    memset(v58, 0, sizeof v58);
    for (v24 = 0; (int32_t)rb(nat + 0x40u) > v24; v24++) {
        v28 = sw(nat + 0x45u + 2u * (uint32_t)v24) - 1;
        v1c = rb(nat + 0x41u + (uint32_t)v24);
        if (f5843C(v28, v34, v14, v1c) != 1)
            continue;
        f58888(v28, v34, v14, v1c);
        memset(DP(0xF8418u), 0xFF, 0xA0);
        memset(DP(0xF84B8u), 0xFF, 0xA0);
        memset(DP(0xF8194u), 0xFF, 0x280);
        for (v20 = 0; v20 < rd(0x109E00u); v20++)
            wb(0x108828u + (uint32_t)(16 * v20), m374E0(v20, v34, v14, 999, 1));
        x = rd(0x109E00u);
        if (x < 20) {
            for (v38 = x - 2; v38 >= 0; v38--)
                for (v20 = 0; v20 <= v38; v20++)
                    if ((int8_t)rb(0x108838u + (uint32_t)(16 * v20)) > (int8_t)rb(0x108828u + (uint32_t)(16 * v20)))
                        swap16(0x108820u + (uint32_t)(16 * v20), 0x108830u + (uint32_t)(16 * v20));
        } else {
            wc_qsort(DP(0x108820u), (size_t)x, 16, mc8);
        }
        v58[v24] = sb(0x108828u);
    }
    f7AC60();
    f7AE14();
    for (v38 = 0; (int32_t)rb(nat + 0x40u) > v38; v38++)
        if (v58[v38] > v18)
            v18 = v58[v38];
    return v18;
}

/* ---- 0x4AC50 ---- */

static int32_t m4AC50(int32_t v6c, int32_t v64, int32_t *v74, int32_t v80)
{
    uint8_t F[0x90 + 1];   /* [ebp-0x90] .. [ebp-0x80): leg 1, padding, leg 2, padding (0xFF: the port's fix) */
#define L1(i) F[(i)]
#define L2(i) F[8 + (i)]
    int32_t v3b0[160];
    int8_t v130[160];
    uint32_t v0c = 0xF14CCu + 199u * (uint32_t)v64;
    int32_t v50 = 0, v78 = 0, v2c = 0, v70 = 0, v18, v48, v40, v44, v68, v60, v5c, v24, v28, v10, v34, v54, v1c,
            v20, v58, v14, v4c, v38, v3c, x;

    memset(F, 0xFF, sizeof F);
#define CNT(c) sb(ar((c) + 1) + 1u)
#define MRK(c) sb(ar((c) + 1) + 1u + (uint32_t)v64)
#define TOK(c) sb(ar((c) + 1) + 0x0Eu + (uint32_t)v64)
#define MOV(c) sb(ar((c) + 1) + 0x24u + (uint32_t)v64)
    if (v80 > 1) {
        v78 = *v74;
        f57E28(v6c, v64, v78);
    }
    for (v18 = 0; (int32_t)rb(v0c + 0x40u) > v18 && *v74 > 0; v18++) {
        v48 = sw(v0c + 0x45u + 2u * (uint32_t)v18) - 1;
        if (f5843C(v48, v6c, v64, rb(v0c + 0x41u + (uint32_t)v18)) != 1)
            continue;
        if (v48 == v6c) {
            v40 = 0;
        } else {
            f58888(v48, v6c, v64, rb(v0c + 0x41u + (uint32_t)v18));
            v40 = 1;
            for (v60 = MUT(16) ? 1 : 2; v60 < 6 && rb(0x108820u + (uint32_t)v60) != 0xFF; v60++)
                v40++;
        }
        v44 = (int32_t)rb(v0c + 0x41u + (uint32_t)v18) - v40;
        if (v44 <= 0)
            continue;
        f5861C(v48, v64, v44);
        wd(0x107EE4u + 4u * (uint32_t)v48, 0);
        v68 = 0;
        for (v60 = 0; v60 < 0xA0; v60++) {
            if (rd(0x107EE4u + 4u * (uint32_t)v60) != 1)
                continue;
            v34 = al(v60);
            if (CNT(v34) == 1) {
                if (MRK(v34) != v64)
                    goto L4AE4C;
                if (TOK(v34) < f78D08(v34, v64))
                    goto L4AE91;
            }
        L4AE4C:
            if (CNT(v34) != 0 || (rb(0xE2C1Au + (uint32_t)(3 * v34)) & 1) || rb(0xF488Cu + (uint32_t)v64) != 0)
                continue;
        L4AE91:
            v3b0[v68++] = v60;
        }
        for (v60 = v68 - 2; v60 >= 0; v60--)
            for (v5c = 0; v5c <= v60; v5c++)
                if ((int8_t)rb(0xEED84u + (uint32_t)(160 * v64 + v3b0[v5c + 1]))
                    > (int8_t)rb(0xEED84u + (uint32_t)(160 * v64 + v3b0[v5c]))) {
                    x = v3b0[v5c + 1];
                    v3b0[v5c + 1] = v3b0[v5c];
                    v3b0[v5c] = x;
                }
        if (v68 <= 0)
            continue;
        if (v48 != v6c && v80 != 1) {
            f58888(v48, v6c, v64, v40);
            v24 = 0;
            for (v60 = 0; v60 < rd(0x109E00u); v60++) {
                v28 = 0;
                v10 = 0x108820 + 16 * v60;
                for (v5c = 0; v5c < 6 && rb((uint32_t)(v10 + v5c)) != 0xFF; v5c++) {
                    v34 = al(rb((uint32_t)(v10 + v5c)));
                    v54 = f78D08(v34, v64);
                    if (rb(0xE2C1Au + (uint32_t)(3 * v34)) & 2)
                        continue;
                    if (MRK(v34) != v64)
                        continue;
                    if (CNT(v34) == 1 && TOK(v34) > v54)
                        goto L4B120;
                    if (CNT(v34) > 1 && v80 == 1) {
                        x = TOK(v34);
                        v2c = aisea_test_f7C838(v34, v64, 0, &v70);
                        if (x == v2c)
                            goto L4B120;
                    }
                    if (CNT(v34) <= 1 || MRK(v34) != v64 || v80 < 1)
                        continue;
                L4B120:
                    if (MOV(v34) <= 0)
                        continue;
                    if (CNT(v34) == 1 && MRK(v34) == v64)
                        v28 += TOK(v34) - v54;
                    else if (v80 == 1)
                        v28 += v2c;
                    else
                        v28 = *v74;
                }
                if (v28 > v24) {
                    v24 = v28;
                    memcpy(&L1(0), DP((uint32_t)(0x108820 + 16 * v60)), 6);
                }
            }
        } else if (v48 != v6c && v80 == 1) {
            f58888(v48, v6c, v64, v40);
            memcpy(&L1(0), DP(0x108820u), 6);
            v24 = CNT(v6c) == 1 ? TOK(v6c) - f78D08(v6c, v64) : *v74;
        } else {
            v24 = CNT(v6c) == 1 ? TOK(v6c) - f78D08(v6c, v64) : *v74;
            L1(0) = (uint8_t)v6c;
            L1(1) = 0xFF;
        }
        if (v24 <= 0)
            continue;
        v1c = 0;
        for (v60 = 0; v60 < v68; v60++) {
            memset(v130, 0, sizeof v130);
            f58888(v6c, v3b0[v60], v64, v44);
            for (v5c = 0; v5c < rd(0x109E00u); v5c++) {
                v20 = 0;
                v10 = 0x108820 + 16 * v5c;
                for (v58 = 0; v58 < 6 && rb((uint32_t)(v10 + v58)) != 0xFF; v58++) {
                    v34 = al(rb((uint32_t)(v10 + v58)));
                    if (v130[v34] != 0)
                        continue;
                    if (CNT(v34) == 1 && MRK(v34) == v64 && TOK(v34) < f78D08(v34, v64))
                        goto L4B48B;
                    if (CNT(v34) != 0 || (rb(0xE2C1Au + (uint32_t)(3 * v34)) & 1) || rb(0xF488Cu + (uint32_t)v64) != 0)
                        continue;
                L4B48B:
                    v20 += CNT(v34) == 1 ? f78D08(v34, v64) - TOK(v34) : f78D08(v34, v64);
                    v130[v34] = 1;
                }
                if (v20 > v1c) {
                    v1c = v20;
                    memcpy(&L2(0), DP((uint32_t)(0x108820 + 16 * v5c)), 6);
                }
            }
        }
        if (v1c == 0)
            continue;
        if (v1c > *v74)
            v1c = *v74;
        v14 = 0;
        for (v60 = 0; v60 < 6 && L1(v60) != 0xFF; v60++) {
            v4c = 0;
            v34 = al(L1(v60));
            if (v14 >= v1c)
                goto L4B5B7;
            v54 = f78D08(v34, v64);
            if (rb(0xE2C1Au + (uint32_t)(3 * v34)) & 2)
                goto L4B5B7;
            if (MRK(v34) != v64)
                goto L4B5B7;
            if (CNT(v34) == 1 && TOK(v34) > v54)
                goto L4B84B;
            if (CNT(v34) > 1 && (!MUT(18) || v80 == 1)) {   /* 0x4B7C8: no mode test here (MUT(18) adds one) */
                x = TOK(v34);
                v2c = aisea_test_f7C838(v34, v64, 0, &v70);
                if (x == v2c)
                    goto L4B84B;
            }
            if (CNT(v34) <= 1 || MRK(v34) != v64 || v80 < 1)
                goto L4B5B7;
        L4B84B:
            if (MOV(v34) <= 0)
                goto L4B5B7;
            if (CNT(v34) == 1 && MRK(v34) == v64)
                v38 = TOK(v34) - v54;
            else if (TOK(v34) == v2c)
                v38 = v2c;
            else
                goto L4B5B7;
            while (v38 > 0 && v14 < v1c && v4c < 5) {
                v4c++;
                v14++;
                v38--;
            }
        L4B5B7:
            if (v4c > 0) {
                uint32_t e;
                int32_t i;
                f57CE8(v34, v64, 1, v4c);
                wb(v0c + 0x4Du + (uint32_t)v18, rb(v0c + 0x4Du + (uint32_t)v18) + v4c);
                e = 0x1079B8u + 6u * (uint32_t)rd(0x107D3Cu);
                wb(e + 2u, v4c);
                wb(e + 4u, 3);
                wb(e + 3u, v18);
                wb(e + 0u, L1(v60));
                i = rd(0x107D3Cu);
                wb(0x1079BDu + 6u * (uint32_t)i, MUT(7) ? 0x2C : 0x2B);
                wd(0x107D3Cu, i + 1);
            }
            if (L1(v60 + 1) != 0xFF) {
                uint32_t e = 0x1079B8u + 6u * (uint32_t)rd(0x107D3Cu);
                int32_t i;
                wb(e + 5u, 0x2B);
                wb(e + 4u, 0x0A);
                wb(e + 3u, v18);
                wb(e + 0u, L1(v60));
                i = rd(0x107D3Cu);
                wd(0x107D3Cu, i + 1);
                wb(0x1079B9u + 6u * (uint32_t)i, L1(v60 + 1));
                decb(v0c + 0x41u + (uint32_t)v18);
                ww(v0c + 0x45u + 2u * (uint32_t)v18, L1(v60 + 1) + 1);
            }
        }
        if (v14 <= 0)
            continue;
        for (v60 = 0; v60 < 6 && v14 > 0 && L2(v60) != 0xFF; v60++) {
            v34 = al(L2(v60));
            v3c = 0;
            v54 = f78D08(v34, v64);
            if (CNT(v34) == 1 && MRK(v34) == v64 && TOK(v34) < f78D08(v34, v64))
                goto L4BB72;
            if (CNT(v34) != 0 || (rb(0xE2C1Au + (uint32_t)(3 * v34)) & 1))
                goto L4B936;
        L4BB72:
            v3c = CNT(v34) == 1 ? f78D08(v34, v64) - TOK(v34) : f78D08(v34, v64);
            if (v3c > v14 && !MUT(6))
                v3c = v14;
        L4B936:
            if (v3c > 0) {
                uint32_t e;
                int32_t i;
                f57E28(v34, v64, v3c);
                wb(v0c + 0x4Du + (uint32_t)v18, rb(v0c + 0x4Du + (uint32_t)v18) - v3c);
                e = 0x1079B8u + 6u * (uint32_t)rd(0x107D3Cu);
                wb(e + 5u, 0x2C);
                wb(e + 2u, v3c);
                wb(e + 4u, 4);
                wb(e + 3u, v18);
                i = rd(0x107D3Cu);
                wd(0x107D3Cu, i + 1);
                wb(0x1079B9u + 6u * (uint32_t)i, L2(v60));
                v14 -= v3c;
                *v74 -= v3c;
                v50 += v3c;
            }
            if (L2(v60 + 1) != 0xFF && sb(v0c + 0x4Du + (uint32_t)v18) > 0) {
                uint32_t e = 0x1079B8u + 6u * (uint32_t)rd(0x107D3Cu);
                int32_t i;
                wb(e + 5u, 0x2C);
                wb(e + 4u, 0x0A);
                wb(e + 3u, v18);
                wb(e + 0u, L2(v60));
                i = rd(0x107D3Cu);
                wd(0x107D3Cu, i + 1);
                wb(0x1079B9u + 6u * (uint32_t)i, L2(v60 + 1));
                decb(v0c + 0x41u + (uint32_t)v18);
                ww(v0c + 0x45u + 2u * (uint32_t)v18, L2(v60 + 1) + 1);
            }
        }
    }
    if (v80 > 1)
        f57CE8(v6c, v64, 0, v78);
    (void)v54;
#undef L1
#undef L2
#undef CNT
#undef MRK
#undef TOK
#undef MOV
    return v50;
}

/* ---- 0x648C8, 0x64BB8, 0x65010 ---- */

static int32_t m65010(int32_t n, int32_t flags);
static int32_t m64BB8(int32_t k, int32_t mp, int32_t n);

static void m648C8(int32_t v3c, int32_t v38)
{
    uint32_t v18 = 0xF14CCu + 199u * (uint32_t)v3c, v14, e;
    int32_t v2c, v30, v28, v24, v1c, v34, v20, d, i;

    if (sb(0xF488Cu + (uint32_t)v3c) == 1)
        return;
    v2c = (rb(v18 + 0x25u) != 0) + 4;
    for (v30 = 0; (int32_t)rb(v18 + 0x40u) > v30; v30++) {
        if ((int32_t)rb(v18 + 0x41u + (uint32_t)v30) != v2c)
            continue;
        v28 = sw(v18 + 0x45u + 2u * (uint32_t)v30) - 1;
        v24 = m64BB8(v30, v2c, v3c);
        if (v24 > 0) {
            f58888(v28, v24, v3c, v2c);
            v1c = m65010(v3c, v38);
        } else {
            v1c = -1;
        }
        if (v1c == -1)
            continue;
        v14 = 0x108820u + 16u * (uint32_t)v1c;
        for (v34 = 0; v34 <= 5; v34++) {
            v20 = rb(v14 + (uint32_t)v34);
            if (v20 == 0xFF)
                continue;
            d = sb(v14 + 8u + (uint32_t)v34);
            if (d < 0) {
                f57CE8(al(v20), v3c, 1, -d);
                e = 0x1079B8u + 6u * (uint32_t)rd(0x107D3Cu);
                wb(e + 5u, 6);
                wb(e + 4u, 3);
                wb(e + 2u, -(int32_t)rb(v14 + 8u + (uint32_t)v34));
                wb(e + 3u, v30);
                i = rd(0x107D3Cu);
                wd(0x107D3Cu, i + 1);
                wb(0x1079B8u + 6u * (uint32_t)i, v20);
            } else if (d > 0) {
                f57E28(al(v20), v3c, d);
                e = 0x1079B8u + 6u * (uint32_t)rd(0x107D3Cu);
                wb(e + 5u, 6);
                wb(e + 2u, rb(v14 + 8u + (uint32_t)v34));
                wb(e + 4u, 4);
                wb(e + 3u, v30);
                i = rd(0x107D3Cu);
                wd(0x107D3Cu, i + 1);
                wb(0x1079B9u + 6u * (uint32_t)i, v20);
            }
            if ((v34 != 5 || MUT(10)) && rb(v14 + (uint32_t)v34 + 1u) != 0xFF) {
                e = 0x1079B8u + 6u * (uint32_t)rd(0x107D3Cu);
                wb(e + 4u, 0x0A);
                wb(e + 3u, v30);
                wb(e + 0u, v20);
                wb(e + 5u, 6);
                i = rd(0x107D3Cu);
                wd(0x107D3Cu, i + 1);
                wb(0x1079B9u + 6u * (uint32_t)i, rb(v14 + (uint32_t)v34 + 1u));
                decb(v18 + 0x41u + (uint32_t)v30);
                ww(v18 + 0x45u + 2u * (uint32_t)v30, rb(v14 + (uint32_t)v34 + 1u) + 1);
            }
        }
    }
}

/* the land matrix byte f64BB8 reads (0x64C49), both modes */
static uint32_t g_dos_base = 0x378F40u;
static int32_t mland(int32_t t, int32_t u)
{
    uint32_t a, f, v;
    if (t > 156 && !g_original)
        t = al(t);
    a = 0xF95FCu + (uint32_t)(157 * t + u);
    if (a <= 0xFF647u)
        return rb(a);
    if (MUT(15))
        return rb(a);                   /* the broken copy: the port's handle bytes */
    f = (a - 0xFF648u) / 4u;            /* field f of the mapdataz pointers: the DOS address */
    v = D32(0xFF648u + 4u * f);
    if (v != 0)
        v = v - D32(0xFF648u) + g_dos_base;
    return (int32_t)((v >> (8u * ((a - 0xFF648u) % 4u))) & 0xFF);
}

static int32_t m64BB8(int32_t v14, int32_t v50, int32_t v5c)
{
    int32_t v420[160];
    uint8_t v100[160], v1a0[160];
    int32_t v18 = 0, v48 = v5c - 1, v20, v40 = 0, v60, v58, v54, v38, v2c, v1c, v10, v4c, v34, v28, v3c, v44, v30, v24,
            ebx;

    v20 = sw(0xF14CCu + 199u * (uint32_t)v5c + 0x45u + 2u * (uint32_t)v14) - 1;
    f5861C(v20, v5c, v50);
    wd(0x107EE4u + 4u * (uint32_t)v20, 0);
    for (v60 = 0; v60 < 0xA0; v60++) {
        if (rd(0x107EE4u + 4u * (uint32_t)v60) != 1)
            continue;
        if (mland(v20, v60) != 0)
            continue;
        v58 = al(v60);
        if (sb(ar(v58 + 1) + 1u) != 0)
            continue;
        if (rb(0xE2C1Au + (uint32_t)(3 * v58)) & 1)
            continue;
        v420[v40++] = v60;
    }
    if (v40 == 0)
        return -1;
    v38 = 0;
    v2c = 0;
    v1c = -1;
    for (v10 = 0; (int16_t)v10 < v40; v10++) {
        v4c = v420[(int16_t)v10];
        f58888(v20, v4c, v5c, v50);
        v34 = 0;
        v28 = 0;
        for (v3c = 0; v3c < rd(0x109E00u); v3c++) {
            memset(v100, 0, sizeof v100);
            memset(v1a0, 0, sizeof v1a0);
            for (v44 = 0; v44 <= v50; v44++) {
                v58 = rb(0x108820u + (uint32_t)(16 * v3c + v44));
                v54 = al(v58);
                if (v58 == 0xFF)
                    break;
                if (rb(0xE2C1Au + (uint32_t)(3 * v54)) & 2)
                    continue;
                v1a0[v58] = (uint8_t)rb(ar(v58 + 1) + 0x0Eu + (uint32_t)v5c);
            }
            v30 = 0;
            v24 = 0;
            for (v44 = 0; v44 <= v50; v44++) {
                v58 = rb(0x108820u + (uint32_t)(16 * v3c + v44));
                v54 = al(v58);
                if (v58 == 0xFF)
                    break;
                if (v100[v54] != 0)
                    continue;
                if (sb(ar(v54 + 1) + 1u) != 0 || (rb(0xE2C1Au + (uint32_t)(3 * v54)) & 1)) {
                    if ((int8_t)v1a0[v54] > 0) {
                        v24 += (int8_t)v1a0[v54];
                        v1a0[v54] = 0;
                    }
                    continue;
                }
                if (!(rb(0xE163Au + (uint32_t)(35 * v54)) & 1) && rb(0xF4234u + (uint32_t)(160 * v48 + v54)) == 0)
                    v30++;
                v100[v54] = 1;
            }
            if (v30 > v24)
                v30 = v24;
            if (v30 > v34) {
                v34 = v30;
                v28 = v24;
            } else if (v30 == v34 && v24 > v28) {
                v28 = v24;
            }
        }
        if (v34 > v38 && sb(0xEED84u + (uint32_t)(160 * v5c + v4c)) > 0) {
            v38 = v34;
            v2c = v28;
            v1c = v4c;
        } else if (v34 == v38) {
            if (v28 > v2c) {
                v2c = v28;
                v1c = v4c;
            } else if (v28 == v2c && v1c != -1
                       && (int8_t)rb(0xEED84u + (uint32_t)(160 * v5c + v4c)) >= (int8_t)rb(0xEED84u + (uint32_t)(160 * v5c + v1c))) {
                ebx = v18++;
                if (sw(0x112A94u + (uint32_t)(20 * (ebx % 8 + 12) + 2 * v5c)) / 300 < (MUT(11) ? 51 : 50))
                    v1c = v4c;
            }
        }
    }
    if (v38 == 0)
        v1c = -1;
    return v1c;
}

static int32_t m65010(int32_t v1c, int32_t v18)
{
    int32_t vc0[6], va8[6], v90[6], vd8[6], v358[160];
    uint8_t v634[10 * 73];
    uint32_t v20 = 0xF14CCu + 199u * (uint32_t)v1c;
    int32_t v14 = 0, v54 = v1c - 1, v2c, v64, v3c, v38, v58, v60, v4c, v48, v40, v68 = 0, v50, v34, v30, v44, v28, v24, ebx,
            x, any;

    v2c = (rb(v20 + 0x25u) != 0) + 4;
    if (rd(0x109E00u) > rd(0x10818Cu))
        wd(0x10818Cu, rd(0x109E00u));
    memset(DP(0xF8418u), 0xFF, 0xA0);
    f7A940();
    f7AA1C();
#define BACKUP()                                                                                   \
    do {                                                                                           \
        if (rd(0xF8194u + 4u * (uint32_t)v4c) == -1) {                                             \
            wd(0xF8194u + 4u * (uint32_t)v4c, v14);                                                \
            memcpy(v634 + 73 * v14 + 4, DP(ar(v48)), 69);                                          \
            x = v48 - 1;                                                                           \
            memcpy(v634 + 73 * v14, &x, 4);                                                        \
            v14++;                                                                                 \
        }                                                                                          \
    } while (0)
#define LIM(c) (((rb(0xE1639u + (uint32_t)(35 * (c))) << 2 & 0xFF) >> 5) + sb(v20 + 0x27u))
    for (v64 = 0; v64 < rd(0x109E00u); v64++) {
        v14 = 0;
        memset(vc0, 0, sizeof vc0);
        memset(va8, 0, sizeof va8);
        memset(v90, 0, sizeof v90);
        memset(vd8, 0, sizeof vd8);
        memset(v358, 0, sizeof v358);
        v3c = 0;
        v38 = 0;
        for (v58 = 0; v58 <= v2c; v58++) {
            v60 = rb(0x108820u + (uint32_t)(16 * v64 + v58));
            if (v60 == 0xFF)
                break;
            v4c = al(v60);
            if (v358[v4c] != 0)
                continue;
            v48 = v4c + 1;
            if (sb(0xF8418u + (uint32_t)v4c) == -1) {
                v40 = aisea_test_f495C0(v4c, v1c, 0, &v68);
                v40 = aisea_test_f49790(v40, v4c, v1c, v68);
                wb(0xF8418u + (uint32_t)v4c, v40);
            } else {
                v40 = sb(0xF8418u + (uint32_t)v4c);
            }
            if (v40 == 0) {
                if (!(rb(0xF3580u + (uint32_t)(314 * v54 + 2 * v4c)) & 0x80))
                    v40 = 1;
                else if (rb(0xE163Au + (uint32_t)(35 * v4c)) & 4)
                    v40 = 1;
            }
            if (v18 == 0x10 && (rb(0xE2C1Au + (uint32_t)(3 * v4c)) & 2) && rb(0xE2C18u + (uint32_t)(3 * v4c)) == v1c) {
                v40 = 0;
                v90[v58] = 1;
            }
            if (!(rb(0xE2C1Au + (uint32_t)(3 * v4c)) & 2) && sb(ar(v48) + 0x0Eu + (uint32_t)v1c) > LIM(v4c)
                && sb(ar(v48) + 1u) > 1)
                v90[v58] = 1;
            if (v18 != 0 || (rb(0xE2C1Au + (uint32_t)(3 * v4c)) & 2)) {
                if (v18 != 0x10)
                    goto L653DB;
            }
            if (sb(ar(v48) + 0x24u + (uint32_t)v1c) <= v40)
                goto L653DB;
            if (v18 == 0x10 && (rb(0xE2C1Au + (uint32_t)(3 * v4c)) & 2) && rb(0xE2C18u + (uint32_t)(3 * v4c)) == v1c)
                v50 = sw(ar(v48) + 0x39u);
            else
                v50 = sb(ar(v48) + 0x24u + (uint32_t)v1c) - v40;
            v3c += v50;
            vd8[v58] = (int8_t)v50;
            v358[v4c] = 1;
            if (sb(ar(v48) + 1u) > 1)
                vc0[v58] = 1;
            continue;
        L653DB:
            if (rb(0xE163Au + (uint32_t)(35 * v4c)) & 2)
                continue;
            if (rb(0xE2C1Au + (uint32_t)(3 * v4c)) & 1)
                continue;
            if (sb(ar(v48) + 1u) != 0 || va8[v58] != 0 || v3c <= 0)
                continue;
            va8[v58] = 1;
            v358[v4c] = 1;
            v38 += LIM(v4c);
        }
        if (v3c > 5)
            v3c = 5;
        v34 = v3c > v38 ? v38 : v3c;
        v30 = v34;
        memset(DP(0x108828u + (uint32_t)(16 * v64)), 0, 6);
        if (v3c <= 0) {
            ww(0x10882Eu + (uint32_t)(16 * v64), MUT(13) ? -299 : -300);
            goto L65A28;
        }
        for (ebx = 0; ebx < 3; ebx++) {   /* MUT(12): the over-limit stops before the contested ones */
            int pass = MUT(12) ? (ebx == 0 ? 1 : ebx == 1 ? 0 : 2) : ebx;
            for (v44 = 0; v44 <= v2c && v34 > 0; v44++) {
                v60 = rb(0x108820u + (uint32_t)(16 * v64 + v44));
                if (pass == 0 && vc0[v44] == 0)
                    continue;
                if (pass == 1 && v90[v44] == 0)
                    continue;
                if (pass == 2 && (v60 == 0xFF || vd8[v44] <= 0))
                    continue;
                v4c = al(v60);
                v48 = v4c + 1;
                while (v34 > 0 && vd8[v44] > 0) {
                    if (pass == 1 && !((v18 == 0 && sb(ar(v48) + 0x24u + (uint32_t)v1c) > 0)
                                       || (v18 == 0x10 && sw(ar(v48) + 0x39u) > 0)))
                        break;
                    BACKUP();
                    decb(0x108828u + (uint32_t)(16 * v64 + v44));
                    v34--;
                    vd8[v44]--;
                    f57CE8(v48 - 1, v1c, 1, 1);
                    if (pass == 1 && v18 == 0x10) {
                        uint8_t *w = load_heap_ptr(D32(0xF5240u + 4u * D32(0xCAC54u)) + (uint32_t)(69 * (v48 - 1)) + 0x39u, 2);
                        uint16_t q = (uint16_t)(w[0] | w[1] << 8);
                        q--;
                        w[0] = (uint8_t)q;
                        w[1] = (uint8_t)(q >> 8);
                    }
                }
            }
        }
        while (v30 > 0) {
            any = 0;
            for (v44 = v2c; v44 >= 0 && v30 > 0; v44--) {
                if (va8[v44] == 0)
                    continue;
                v60 = rb(0x108820u + (uint32_t)(16 * v64 + v44));
                v4c = al(v60);
                v48 = v4c + 1;
                if (v60 == 0xFF)
                    continue;
                if (sb(ar(v48) + 0x0Eu + (uint32_t)v1c) >= LIM(v4c))
                    continue;
                BACKUP();
                v30--;
                incb(0x108828u + (uint32_t)(16 * v64 + v44));
                f57E28(v4c, v1c, 1);
                any = 1;
            }
            if (!any && !g_original)
                break;
        }
        ww(0x10882Eu + (uint32_t)(16 * v64), aisea_test_f7B044(v1c));
    L65A28:
        for (v44 = 0; v44 < v14; v44++) {
            memcpy(&x, v634 + 73 * v44, 4);
            memcpy(DP(ar(x + 1)), v634 + 73 * v44 + 4, 69);
            wd(0xF8194u + 4u * (uint32_t)x, -1);
        }
    }
#undef BACKUP
#undef LIM
    f7AC60();
    f7AE14();
    x = rd(0x109E00u);
    if (x < 20) {
        for (v44 = x - 2; v44 >= 0; v44--)
            for (v58 = 0; v58 <= v44; v58++)
                if (sw(0x10883Eu + (uint32_t)(16 * v58)) > sw(0x10882Eu + (uint32_t)(16 * v58)))
                    swap16(0x108820u + (uint32_t)(16 * v58), 0x108830u + (uint32_t)(16 * v58));
    } else {
        wc_qsort(DP(0x108820u), (size_t)x, 16, mc0E);
    }
    v28 = 0;
    while (rb(0x108826u) == rb(0x108826u + (uint32_t)(16 * v28)) && sw(0x10882Eu) == sw(0x10882Eu + (uint32_t)(16 * v28))
           && v28 < rd(0x109E00u))
        v28++;
    if (v28 == 0)
        return -1;
    ebx = 30000 / (v28 > 30000 ? 30000 : v28);
    v24 = sw(0x112AA6u + 2u * (uint32_t)v1c) / ebx < v28 ? sw(0x112AA6u + 2u * (uint32_t)v1c) / ebx : v28 - 1;
    return sw(0x10882Eu + (uint32_t)(16 * v24)) < 0 ? -1 : v24;
}

/* ---- 0x643C8 ---- */

static void m643C8(int32_t v4c)
{
    int32_t v2cc[160], v18, v7cc[160][2], v48 = 0, v44, v1c, v40, v3c, v28, v24, v38, v30, v2c, i;

    if (sb(0xF488Cu + (uint32_t)v4c) == 1)
        return;
    f7A940();
    f7AA1C();
    for (v44 = 1; v44 < 0x9D; v44++) {
        if (rb(0xEE16Cu + (uint32_t)v44) == 0 || !(rb(0xE1638u + (uint32_t)(35 * v44)) & 2)
            || !(rb(0xE2C1Au + (uint32_t)(3 * v44)) & 2) || rb(0xE2C18u + (uint32_t)(3 * v44)) != v4c)
            continue;
        v1c = v44 + 1;
        if (sb(ar(v1c) + 0x24u + (uint32_t)v4c) <= 0)
            continue;
        v40 = 0;
        v3c = sb(ar(v1c) + 0x24u + (uint32_t)v4c);
        aisea_test_f48028(&v18, v2cc, v44, v4c, 0x0F);
        if (v18 > 0) {
            aisea_test_f48D44(&v18, v2cc, v4c, v44, 1);
            v28 = 0;
            v24 = MUT(9) ? 9 : 10;
            while (v3c > 0 && v24 > 0) {
                v38 = v2cc[v28];
                if (sb(ar(v38 + 1) + 0x24u + (uint32_t)v4c) > 0 && sb(ar(v38 + 1) + 0x0Eu + (uint32_t)v4c) > 1) {
                    f57CA4(v38, v44, v4c, 1, 1);
                    i = rd(0x1079B4u);
                    wb(0x107694u + 4u * (uint32_t)i, v38);
                    wb(0x107695u + 4u * (uint32_t)i, v44);
                    wb(0x107697u + 4u * (uint32_t)i, 0x10);
                    wb(0x107696u + 4u * (uint32_t)i, 1);
                    wd(0x1079B4u, i + 1);
                    v3c--;
                    v40++;
                }
                v28++;
                if (v28 == v18) {
                    v28 = 0;
                    v24--;
                }
            }
        }
        if (v40 != 0) {
            v7cc[v48][0] = v44;
            v7cc[v48][1] = 0;
            v48++;
            ww(ar(v1c) + 0x39u, v40);
        }
    }
    if (v48 == 0) {
        f7AC60();
        f7AE14();
        m648C8(v4c, 0);
        return;
    }
    f7A940();
    f7AA1C();
    m648C8(v4c, 0x10);
    v30 = 0;
    for (v28 = 0; v28 < v48; v28++) {
        v1c = v7cc[v28][0] + 1;
        if (sw(ar(v1c) + 0x39u) != 0) {
            uint8_t *w = load_heap_ptr(D32(0xF5240u + 4u * D32(0xCAC54u)) + (uint32_t)(69 * (v1c - 1)) + 0x39u, 2);
            v7cc[v28][1] = (int16_t)(w[0] | w[1] << 8) - sw(ar(v1c) + 0x39u);
            v30 = 1;
        }
    }
    if (v30 == 0) {
        f7AD24();
        f7AD7C();
        f7AD24();
        f7AD7C();
        return;
    }
    f7AC60();
    f7AE14();
    f7AF28();
    f7AFB8();
    for (v2c = 0; v2c < v48; v2c++) {
        v1c = v7cc[v2c][0] + 1;
        v44 = v7cc[v2c][0];
        v3c = v7cc[v2c][1];
        ww(ar(v1c) + 0x39u, v7cc[v2c][1]);
        aisea_test_f48028(&v18, v2cc, v44, v4c, 0x0F);
        if (v18 <= 0)
            continue;
        aisea_test_f48D44(&v18, v2cc, v4c, MUT(17) ? v44 : 0, 1);   /* 0x6479E xor ecx, ecx */
        v28 = 0;
        v24 = 10;
        while (v3c > 0 && v24 > 0) {
            v38 = v2cc[v28];
            if (sb(ar(v38 + 1) + 0x24u + (uint32_t)v4c) > 0 && sb(ar(v38 + 1) + 0x0Eu + (uint32_t)v4c) > 1) {
                f57CA4(v38, v44, v4c, 1, 1);
                i = rd(0x1079B4u);
                wb(0x107694u + 4u * (uint32_t)i, v38);
                wb(0x107695u + 4u * (uint32_t)i, v44);
                wb(0x107697u + 4u * (uint32_t)i, 0x10);
                wd(0x1079B4u, i + 1);
                wb(0x107692u + 4u * (uint32_t)(i + 1), 1);
                v3c--;
            }
            v28++;
            if (v28 == v18) {
                v28 = 0;
                v24--;
            }
        }
    }
    m648C8(v4c, 0x10);
    f7AD24();
    f7AD7C();
}

/* ---- 0x65CB0 ---- */

static void m65CB0(int32_t v2c)
{
    int32_t v2c8[160], v7c8[160][2], tmp[2], v38, v34, v40, v20, v24, v28, v3c, i;
    uint32_t v1c, e;

    f7A940();
    aisea_test_f5AAA4(0);
    aisea_test_f5C238(0);
    v1c = 0xF14CCu + 199u * (uint32_t)v2c;
    memset(v2c8, 0, sizeof v2c8);
    for (v38 = 0; (int32_t)rb(v1c + 0x40u) > v38; v38++) {
        i = sw(v1c + 0x45u + 2u * (uint32_t)v38);
        if (i >= 1 && i <= 160)
            v2c8[i - 1] = 1;
    }
    for (v38 = 0; (int32_t)rb(v1c + 0x40u) > v38; v38++) {
        if (rb(v1c + 0x41u + (uint32_t)v38) == 0)
            continue;
        v34 = sw(v1c + 0x45u + 2u * (uint32_t)v38) - 1;
        f5861C(v34, v2c, rb(v1c + 0x41u + (uint32_t)v38));
        wd(0x107EE4u + 4u * (uint32_t)v34, 1);
        for (v40 = 0; v40 < 0xA0; v40++)
            if (v2c8[v40] != 0)
                wd(0x107EE4u + 4u * (uint32_t)v40, 0);
        v20 = 0;
        for (v40 = 0; v40 < 0xA0; v40++) {
            if (rd(0x107EE4u + 4u * (uint32_t)v40) != 1)
                continue;
            v24 = al(v40);
            if (sb(ar(v24 + 1) + 1u + (uint32_t)v2c) != v2c || (rb(0xE2C1Au + (uint32_t)(3 * v24)) & 2))
                continue;
            v7c8[v20][0] = v40;
            v7c8[v20][1] = sb(ar(v24 + 1) + 0x0Eu + (uint32_t)v2c);
            v20++;
        }
        if (v20 <= 0)
            continue;
        for (v40 = v20 - 2; v40 >= 0; v40--)
            for (v28 = 0; v28 <= v40; v28++)
                if (MUT(14) ? v7c8[v28 + 1][1] < v7c8[v28][1] : v7c8[v28 + 1][1] > v7c8[v28][1]) {
                    memcpy(tmp, v7c8[v28 + 1], 8);
                    memcpy(v7c8[v28 + 1], v7c8[v28], 8);
                    memcpy(v7c8[v28], tmp, 8);
                }
        v3c = v7c8[0][0];
        v2c8[v3c] = 1;
        if (v3c == v34)
            continue;
        f58888(v34, v3c, v2c, rb(v1c + 0x41u + (uint32_t)v38));
        for (v40 = 0; v40 <= 5; v40++) {
            v24 = rb(0x108820u + (uint32_t)v40);
            if (v24 == 0xFF || v40 == 5 || rb(0x108821u + (uint32_t)v40) == 0xFF)
                continue;
            e = 0x1079B8u + 6u * (uint32_t)rd(0x107D3Cu);
            wb(e + 4u, 0x0A);
            wb(e + 3u, v38);
            wb(e + 0u, v24);
            wb(e + 5u, 0x2A);
            i = rd(0x107D3Cu);
            wd(0x107D3Cu, i + 1);
            wb(0x1079B9u + 6u * (uint32_t)i, rb(0x108821u + (uint32_t)v40));
            decb(v1c + 0x41u + (uint32_t)v38);
            ww(v1c + 0x45u + 2u * (uint32_t)v38, rb(0x108821u + (uint32_t)v40) + 1);
        }
    }
    f7AC60();
}

/* ======================================================================================================================
   Part A: scenarios
   ==================================================================================================================== */

static uint32_t g_seed = 12345;
static int rnd(int n) { g_seed = g_seed * 1103515245u + 12345u; return n > 0 ? (int)((g_seed >> 8) % (uint32_t)n) : 0; }

static uint8_t *g_ds_live;             /* the image the loaders built (g_ds at start) */
static uint8_t *g_base, *g_port, *g_model;
static int g_ship_terr[160], g_nship_terr;   /* territories that are ends of N.RL records (ships can be there) */

enum { F374E0, F36918, F4D29C, F4AC50, F643C8, F648C8, F64BB8, F65010, F65CB0, NFN };
static const char *const FNAME[NFN] = { "f374E0", "f36918", "f4D29C", "f4AC50", "f643C8", "f648C8", "f64BB8", "f65010", "f65CB0" };

typedef struct {
    int fn, a[4], orig;
} Op;

typedef struct {
    int32_t ret, out;
    int quits;
    long rands;
} Res;

static void nat_setup(int n)
{
    uint32_t nat = 0xF14CCu + 199u * (uint32_t)n;
    int k, ships = 1 + rnd(4), c, i, t, w;

    wb(nat + 0x25u, rnd(2));
    wb(nat + 0x27u, rnd(2));
    wb(nat + 0x2Bu, rnd(3) == 0);
    wb(nat + 0x5Du, rnd(9));
    wb(nat + 0x40u, ships);
    for (k = 0; k < ships; k++) {
        if (rnd(6) == 0)
            t = 157 + rnd(3);
        else
            t = g_ship_terr[rnd(g_nship_terr)];
        ww(nat + 0x45u + 2u * (uint32_t)k, t + 1);
        w = rnd(4);
        wb(nat + 0x41u + (uint32_t)k, w == 0 ? rnd(6) : 4 + (rb(nat + 0x25u) != 0));
        wb(nat + 0x4Du + (uint32_t)k, rnd(3) == 0 ? rnd(4) : 0);
    }
    for (i = 0; i < 14; i++) {   /* tokens of n near the ships and elsewhere */
        c = rnd(3) ? al(g_ship_terr[rnd(g_nship_terr)]) : rnd(157);
        f57E28(c, n, 1 + rnd(5));
    }
    for (i = 0; i < 4; i++) {   /* other nations */
        c = rnd(157);
        f57E28(c, 1 + rnd(9), 1 + rnd(4));
    }
    if (rnd(3))
        f460C0(n);
    else
        for (c = 0; c < 157; c++)
            if (sb(ar(c + 1) + 0x0Eu + (uint32_t)n) > 0)
                wb(ar(c + 1) + 0x24u + (uint32_t)n, rnd(sb(ar(c + 1) + 0x0Eu + (uint32_t)n) + 1));
    for (i = 0; i < 6; i++) {   /* marked sites of n (with a need), cities */
        c = al(g_ship_terr[rnd(g_nship_terr)]);
        if (rnd(2)) {
            wb(0xE2C1Au + (uint32_t)(3 * c), rb(0xE2C1Au + (uint32_t)(3 * c)) | 2);
            wb(0xE2C18u + (uint32_t)(3 * c), rnd(4) ? n : 1 + rnd(9));
            ww(ar(c + 1) + 0x39u, rnd(5));
        } else if (rnd(3) == 0) {
            wb(0xE2C1Au + (uint32_t)(3 * c), rb(0xE2C1Au + (uint32_t)(3 * c)) | 1);
        }
    }
    for (c = 0; c < 160; c++)
        wb(0xEED84u + (uint32_t)(160 * n + c), rnd(60) - 20);
    if (rnd(2))   /* the pre-rolled table, its top values included (they reach the "idx - 1" picks) */
        for (i = 0; i < 300; i++)
            ww(0x112A94u + 2u * (uint32_t)i, rnd(4) ? rnd(30000) : 29990 + rnd(10));
    wb(0xF488Cu + (uint32_t)n, rnd(8) == 0);
    /* the caches: mostly as the callers leave them (-1), sometimes stale or a fresh process's zeros */
    i = rnd(4);
    memset(DP(0xF8418u), 0xFF, 0xA0);
    memset(DP(0xF84B8u), 0xFF, 0xA0);
    memset(DP(0xF8194u), i == 0 ? 0 : 0xFF, 0x280);
    if (i == 1)
        for (c = 0; c < 20; c++) {
            wb(0xF8418u + (uint32_t)rnd(160), rnd(4));
            wb(0xF84B8u + (uint32_t)rnd(160), rnd(3));
        }
    wd(0x107D3Cu, rnd(10));
    wd(0x1079B4u, rnd(10));
    wd(0xF5908u, rnd(3));
}

static void run_one(Op *op, int model, Res *res)
{
    int32_t out = op->a[2];
    volatile int fn = op->fn;

    g_tr = model ? g_tr_model : g_tr_port;
    g_trlen = 0;
    g_tr[0] = 0;
    g_quits = 0;
    g_rand = 0;
    g_ds = model ? g_model : g_port;
    memcpy(g_ds, g_base, DS_SIZE);
    g_original = op->orig;
    res->ret = 0;
    if (setjmp(g_jb) == 0) {
        switch (fn) {
        case F374E0: res->ret = model ? m374E0(op->a[0], op->a[1], op->a[3], out, op->a[2] & 1) : f374E0(op->a[0], op->a[1], op->a[3], out, op->a[2] & 1); break;
        case F36918: if (model) m36918(op->a[3], op->a[1], &out, op->a[0]); else f36918(op->a[3], op->a[1], &out, op->a[0]); break;
        case F4D29C: res->ret = model ? m4D29C(op->a[1], op->a[3]) : f4D29C(op->a[1], op->a[3]); break;
        case F4AC50: res->ret = model ? m4AC50(op->a[1], op->a[3], &out, op->a[0]) : f4AC50(op->a[1], op->a[3], &out, op->a[0]); break;
        case F643C8: if (model) m643C8(op->a[3]); else f643C8(op->a[3]); break;
        case F648C8: if (model) m648C8(op->a[3], op->a[0]); else f648C8(op->a[3], op->a[0]); break;
        case F64BB8: res->ret = model ? m64BB8(op->a[0], op->a[1], op->a[3]) : f64BB8(op->a[0], op->a[1], op->a[3]); break;
        case F65010: res->ret = model ? m65010(op->a[3], op->a[0]) : f65010(op->a[3], op->a[0]); break;
        case F65CB0: if (model) m65CB0(op->a[3]); else f65CB0(op->a[3]); break;
        }
    }
    res->out = out;
    res->quits = g_quits;
    res->rands = g_rand;
    g_original = 0;
}

/* One scenario: random edits on a copy of the save, a call, port and model on their own copies. Returns 1 if they
   agree. The prepared state is left in g_base. */
static int scenario(const uint8_t *save, Op *op, int verbose)
{
    Res rp, rm;
    char what[200];
    int n, k, ok, i;
    uint32_t a;

    g_ds = g_base;
    memcpy(g_base, save, DS_SIZE);
    n = 1 + rnd(9);
    nat_setup(n);
    op->a[3] = n;
    op->orig = rnd(3) == 0;
    k = sw(0xF14CCu + 199u * (uint32_t)n + 0x45u) - 1;   /* ship 0's territory */
    switch (op->fn) {
    case F374E0:   /* a route table first: a ship's routes to a land target */
        op->a[1] = rnd(157);
        f58888(k, op->a[1], n, 1 + rnd(5));
        if (rd(0x109E00u) <= 0)
            return -1;
        op->a[0] = rnd(rd(0x109E00u));
        op->a[2] = rnd(4) ? 1 + rnd(8) : 999;
        break;
    case F36918:
        op->a[1] = rnd(2) ? al(g_ship_terr[rnd(g_nship_terr)]) : rnd(157);
        op->a[2] = 1 + rnd(12);
        { static const int codes[7] = { 0, 1, 2, 3, 4, 0x0F, 0x27 }; op->a[0] = codes[rnd(7)]; }
        break;
    case F4D29C:
        op->a[1] = rnd(2) ? al(g_ship_terr[rnd(g_nship_terr)]) : rnd(157);
        break;
    case F4AC50:
        op->a[1] = rnd(3) ? k : al(g_ship_terr[rnd(g_nship_terr)]);
        if (op->a[1] > 156)
            op->a[1] = al(op->a[1]);
        op->a[2] = 1 + rnd(8);
        op->a[0] = rnd(4);
        break;
    case F643C8:   /* marked coastal sites of n with tokens there and around (up to 12: the ten rounds) */
        for (i = 0; i < 3; i++) {
            int t, u, c;
            for (c = 0; c < 50; c++) {
                t = 1 + rnd(156);
                if (rb(GAME_INPLAY + (uint32_t)t) && (rb(0xE1638u + (uint32_t)(35 * t)) & 2))
                    break;
            }
            wb(0xE2C1Au + (uint32_t)(3 * t), rb(0xE2C1Au + (uint32_t)(3 * t)) | 2);
            wb(0xE2C18u + (uint32_t)(3 * t), n);
            f57E28(t, n, 1 + rnd(3));
            wb(ar(t + 1) + 0x24u + (uint32_t)n, rnd(3) ? sb(ar(t + 1) + 0x0Eu + (uint32_t)n) : 8 + rnd(5));
            for (u = 0; u < 157; u++)
                if (rb(GAME_LAND_MATRIX + (uint32_t)(157 * t + u)) && rnd(2)) {
                    f57E28(u, n, rnd(12));
                    wb(ar(u + 1) + 0x24u + (uint32_t)n, sb(ar(u + 1) + 0x0Eu + (uint32_t)n));
                }
        }
        break;
    case F648C8:
        op->a[0] = rnd(2) ? 0 : 0x10;
        break;
    case F64BB8:
        op->a[0] = rnd(rb(0xF14CCu + 199u * (uint32_t)n + 0x40u));
        op->a[1] = 4 + rnd(2);
        break;
    case F65010:   /* routes from ship 0 to a target, as f648C8 leaves them */
        op->a[0] = rnd(2) ? 0 : 0x10;
        f58888(k, g_ship_terr[rnd(g_nship_terr)], n, 4 + (rb(0xF14CCu + 199u * (uint32_t)n + 0x25u) != 0));
        if (rd(0x109E00u) <= 0)
            return -1;
        if (op->a[0] == 0x10) {   /* f643C8's level under f65010's own (its slot holds the +0x39 needs) */
            f7A940();
            f7AA1C();
        }
        break;
    case F65CB0:
        break;
    }
    run_one(op, 0, &rp);
    run_one(op, 1, &rm);
    if (op->fn == F65010 && op->a[0] == 0x10) {   /* the two levels pushed for it come off again (blocks freed once) */
        g_ds = g_base;
        f7AD24();
        f7AD7C();
    }
    ok = 1;
    for (a = 0; a < DS_SIZE && g_port[a] == g_model[a]; a++)
        ;
    snprintf(what, sizeof what, "A %s(%d,%d,%d,n %d) orig %d", FNAME[op->fn], op->a[0], op->a[1], op->a[2], n, op->orig);
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
            printf("  %s: call logs differ at %d: port \"%.60s\" model \"%.60s\"\n", what, i, g_tr_port + i, g_tr_model + i);
        }
    }
    if (rp.ret != rm.ret || rp.out != rm.out || rp.quits != rm.quits) {
        ok = 0;
        if (verbose && g_fail < 30)
            printf("  %s: ret %d/%d out %d/%d quits %d/%d\n", what, rp.ret, rm.ret, rp.out, rm.out, rp.quits, rm.quits);
    }
    if (verbose) {
        check(ok, what);
        check(rp.quits == 0 && rp.rands == 0, "A no quit, no game_rand");
    }
    g_ds = g_base;
    return ok;
}

/* coverage counters: how many scenarios reached the interesting paths (port run) */
static long g_cov_ships157, g_cov_moves, g_cov_landmoves;

static void part_a(uint8_t *const *saves, int nsaves, int per)
{
    Op op;
    int s, f, i, r, skipped = 0;
    long before = g_fail, runs = 0;

    for (s = 0; s < nsaves; s++)
        for (f = 0; f < NFN; f++)
            for (i = 0; i < per; i++) {
                op.fn = f;
                memset(op.a, 0, sizeof op.a);
                r = scenario(saves[s], &op, 1);
                if (r < 0) {
                    skipped++;
                    continue;
                }
                runs++;
                g_ds = g_port;
                if (memcmp(g_port + (0x107D3Cu - DS_BASE), g_base + (0x107D3Cu - DS_BASE), 4) != 0)
                    g_cov_moves++;
                if (memcmp(g_port + (0x1079B4u - DS_BASE), g_base + (0x1079B4u - DS_BASE), 4) != 0)
                    g_cov_landmoves++;
                {
                    int k, n = op.a[3];
                    for (k = 0; k < rb(0xF14CCu + 199u * (uint32_t)n + 0x40u); k++)
                        if (sw(0xF14CCu + 199u * (uint32_t)n + 0x45u + 2u * (uint32_t)k) > 157)
                            g_cov_ships157++;
                }
                g_ds = g_base;
            }
    printf("A. %ld differential runs (%d skipped: no route), %ld failed; %ld added ship moves, %ld added land moves, "
           "%ld ships on 157..159\n", runs, skipped, g_fail - before, g_cov_moves, g_cov_landmoves, g_cov_ships157);
}

/* ---- Part B: the broken models must be caught ---- */

static void part_b(uint8_t *const *saves, int nsaves)
{
    static const struct { int mut, fn; const char *what; } M[] = {
        { 1, F374E0, "f374E0 keep 2 instead of 1" },
        { 2, F374E0, "f374E0 without pass 2" },
        { 3, F36918, "f36918 table entry of nation n - 1" },
        { 4, F36918, "f36918 plans sorted the other way" },
        { 5, F36918, "f36918 code byte of the next entry" },
        { 6, F4AC50, "f4AC50 unload not capped by the load" },
        { 7, F4AC50, "f4AC50 load record code 0x2C" },
        { 9, F643C8, "f643C8 nine rounds" },
        { 10, F648C8, "f648C8 a hop after stop 5" },
        { 11, F64BB8, "f64BB8 tie threshold 51" },
        { 12, F65010, "f65010 over-limit stops first" },
        { 13, F65010, "f65010 empty score -299" },
        { 14, F65CB0, "f65CB0 smallest stack first" },
        { 15, F64BB8, "f64BB8 --original reads the handle bytes" },
        { 16, F4AC50, "f4AC50 leg-1 steps counted from byte 1" },
        { 17, F643C8, "f643C8 redo passes t to f48D44" },
        { 18, F4AC50, "f4AC50 load pass f7C838 in mode 1 only" },
    };
    Op op;
    unsigned m;
    int i, caught, r;
    char what[160];

    for (m = 0; m < sizeof M / sizeof M[0]; m++) {
        caught = 0;
        for (i = 0; i < 20000 && !caught; i++) {
            op.fn = M[m].fn;
            memset(op.a, 0, sizeof op.a);
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

/* ---- Part C: data facts ---- */

static void part_c(const char *dump)
{
    static const uint8_t codes[5] = { 0x04, 0x23, 0x28, 0x0F, 0x27 };
    FILE *fp;
    uint8_t *d;
    uint32_t a, base;
    int i, bad = 0, n;
    char what[120];

    for (i = 0; i < 5; i++) {
        snprintf(what, sizeof what, "C 0xC1938[%d] = %02X", i, codes[i]);
        check(D8(0xC1938u + (uint32_t)i) == codes[i], what);
    }
    check(D8(0xC1947u) == 0 && D8(0xC195Fu) == 0, "C 0xC1947 / 0xC195F (codes 0x0F / 0x27) are 0");
    printf("C. code bytes 0xC1938..: %02X %02X %02X %02X %02X; [0xC1947] = %02X, [0xC195F] = %02X\n", D8(0xC1938u),
           D8(0xC1939u), D8(0xC193Au), D8(0xC193Bu), D8(0xC193Cu), D8(0xC1947u), D8(0xC195Fu));
    fp = fopen(dump, "rb");
    if (fp == NULL) {
        printf("C. %s not found: the DOS-address rebuild not checked\n", dump);
        return;
    }
    d = malloc(DS_SIZE);
    n = d != NULL && fread(d, 1, DS_SIZE, fp) == DS_SIZE;
    fclose(fp);
    if (!n) {
        free(d);
        printf("C. %s unreadable\n", dump);
        return;
    }
    base = (uint32_t)d[0xFF648u - DS_BASE] | (uint32_t)d[0xFF649u - DS_BASE] << 8 | (uint32_t)d[0xFF64Au - DS_BASE] << 16
           | (uint32_t)d[0xFF64Bu - DS_BASE] << 24;
    g_original = 1;
    for (i = 157; i <= 159; i++) {
        int u;
        for (u = 0; u < 160; u++) {
            a = 0xF95FCu + 157u * (uint32_t)i + (uint32_t)u;
            if (mland(i, u) != d[a - DS_BASE])
                bad++;
        }
    }
    g_original = 0;
    snprintf(what, sizeof what, "C the --original rows 157..159 equal the dump's bytes (base 0x%X)", (unsigned)base);
    check(bad == 0 && base == g_dos_base, what);
    printf("C. %s: [0xFF648] = 0x%08X; rows 157..159 of the land matrix rebuilt: %d of 480 bytes differ\n", dump,
           (unsigned)base, bad);
    free(d);
}

static void ships_report(const char *name)
{
    int n, k, w, t;
    printf("C. %s: phase %d, round %d; computers' ships (+0x9A == 2):", name, D8(0xF4102u), S16(0xF40FCu));
    for (n = 1; n <= 9; n++) {
        uint32_t nat = 0xF14CCu + 199u * (uint32_t)n;
        if (D8(nat + 0x9Au) != 2)
            continue;
        printf(" n%d[", n);
        for (k = 0; k < D8(nat + 0x40u); k++) {
            w = S16(nat + 0x45u + 2u * (uint32_t)k);
            t = w - 1;
            printf("%s%d:%d%s", k ? " " : "", t, D8(nat + 0x41u + (uint32_t)k), t >= 157 ? "*" : "");
        }
        printf("]");
    }
    printf("  (territory:points, * second coast)\n");
}

/* ---- loading ---- */

static int load_state(const char *gamedir, const char *path, uint8_t *out)
{
    int16_t ver;
    char desc[64];
    FILE *fp;
    int v;

    memcpy(g_ds, g_ds_live, DS_SIZE);
    fp = fopen(path, "rb");
    if (fp == NULL) {
        printf("cannot open %s\n", path);
        return -1;
    }
    f5F05C(&ver, desc, fp);
    fclose(fp);
    /* the map variant and its tables as after a restore (main 0x1164A..0x1169B, f12B00, f3EED8) */
    v = 7 * ((D16(0xE1624u) & 2) != 0) + S32(0xF40F0u) - 2;
    setD16(GAME_MAP_VARIANT, (uint16_t)v);
    f12B00();
    snprintf((char *)DP(0xF5820u), 0x64, "%s\\", gamedir);
    f3EED8();
    setD32(0xCAC54u, 0xFFFFFFFFu);   /* both stacks empty */
    setD32(0xCAC58u, 0xFFFFFFFFu);
    memcpy(out, g_ds, DS_SIZE);
    printf("loaded %s: \"%s\" version 0x%X, players %d, variant %d\n", path, desc, ver, S32(0xF40F0u), v);
    ships_report(path);
    return 0;
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:\\work\\advciv_run";
    const char *playdir = argc > 2 ? argv[2] : "C:\\work\\advciv_play";
    const char *dump = argc > 3 ? argv[3] : "C:\\DOSBox-X\\advciv\\M7_R4MV_0.BIN";
    char path[600];
    uint8_t *saves[2];
    int nsaves = 0, t, e;

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
    saves[0] = malloc(DS_SIZE);
    saves[1] = malloc(DS_SIZE);
    if (!g_ds_live || !g_base || !g_port || !g_model || !saves[0] || !saves[1])
        return 2;
    memcpy(g_ds_live, g_ds, DS_SIZE);
    part_c(dump);

    snprintf(path, sizeof path, "%s\\SAVGAMEA.DAT", playdir);
    if (load_state(playdir, path, saves[nsaves]) == 0)
        nsaves++;
    snprintf(path, sizeof path, "%s\\savgame0.dat", gamedir);
    if (load_state(gamedir, path, saves[nsaves]) == 0)
        nsaves++;
    if (nsaves == 0)
        return 2;
    /* the territories ships can be in: the ends of the N.RL records (table 1) */
    g_ds = saves[0];
    g_nship_terr = 0;
    for (t = 0; t < 160; t++)
        for (e = 0; e < S32(0xFFD44u); e++)
            if (D8(0xFFD48u + 8u * (uint32_t)e) == t || D8(0xFFD49u + 8u * (uint32_t)e) == t) {
                g_ship_terr[g_nship_terr++] = t;
                break;
            }
    printf("%d territories are ends of N.RL routes\n", g_nship_terr);

    part_a(saves, nsaves, argc > 4 ? atoi(argv[4]) : 400);
    part_b(saves, nsaves);

    printf("%ld checks, %ld failed\n", g_tests, g_fail);
    return g_fail == 0 ? 0 : 1;
}
