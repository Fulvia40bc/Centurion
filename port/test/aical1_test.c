/* aical1_test [GAMEDIR [PLAYDIR [RUNS]]]: unit checks for module AICAL1 (aical1.c) that need no game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init; default C:\work\advciv_run, only read), terrflag.dat /
   mapdataz.dat / the adjacency files / <players>.RL (f12394, f3EA7C, f3EED8 after each restore) and the saves
   savgame0.dat (R4 at MOVEMENT) and savgame3.dat..savgame9.dat (R4 at TRADE / calamities, edited); PLAYDIR holds
   SAVGAMEA.DAT (the R4 autosave, seven computers; default C:\work\advciv_play). All are read with saveload.c's
   f5F05C as the game does, then the map variant (f12B00) and the adjacency lists (gfxload.c's f3EED8).

   aical1.c is compiled on its own with -Df78E74=rec_f78E74 -Df78F04=rec_f78F04 -Df57CE8=rec_f57CE8
   -Df57E28=rec_f57E28, so its calls of the real rules.c / move.c primitives pass through recorders here that write a
   trace line and then call the real routine. Its other callees are recorders only: f79270 (the message: wait, n, t,
   text, anim), f78A20, mono_converts_fmt (mono.c's rule: the data string under g_original, else "Monotheism") and
   f28CFC (CAL1's list builder, kind 0x1E written here from the listing 0x29045..0x290D7; any other kind is
   "UNEXPECTED"). game_rand f78AE4 counts calls (none expected).

   The model is a second implementation of the ten routines written from the disassembly (docs: the listing, not
   aical1.c): a transcription that keeps the original's labels as gotos and its frame offsets as variable names, with
   a byte frame for 0x213C0 (whose bubble sort indexes [ebp + 4j + 0x16] for lim[j - 1]) and 0x21E8C (12-byte records
   moved with memcpy through the [ebp-0x40] temp); the stale word of 0x2E1D0 is its own stand-in (a stack address)
   and the 1995 fixes are written from their spec in aical1.c's file comment. Each check runs the port, keeps its data
   object, trace and result, restores the start state, runs the model and compares all three (the whole data object
   byte for byte).
     A. hand vectors with expected values written out (0x20068, 0x201B8, 0x1EF28, 0x2E1D0, 0x25548, 0x24EB8 and its
        fix, 0x21E8C's empty token branch both modes, 0x20C70, 0x213C0).
     B. random differential runs: each routine on random states made from the real saves (cities placed and removed
        consistently in the nation lists and territory states, tokens and markers, marks, AST squares, stocks, players
        in play, the round and founding rounds, the 0xF52E0 / 0xEED84 tables, the Earthquake pairs, the choice list
        and the words past it, the 0x40 / limit bits of the territory records, the option byte 0xF0980), both values
        of g_original.
     C. a mutation check: deliberately broken copies of the model (one change each) must each be caught by part B's
        scenarios.
   Prints each failure (the first few), the counts per part, and exits 0 when all pass.

   Build (from the repo root; artefacts outside the repo):
     clang -std=c11 -c port/src/aical1.c -Iport/src -Df78E74=rec_f78E74 -Df78F04=rec_f78F04 -Df57CE8=rec_f57CE8
       -Df57E28=rec_f57E28 -o aical1.o
     clang -std=c11 -Iport/src port/test/aical1_test.c aical1.o port/src/rules.c port/src/move.c port/src/saveload.c
       port/src/load.c port/src/gfxload.c port/src/state.c port/src/le_image.c -o aical1_test.exe */

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <direct.h>
#define chdir _chdir

#include "../src/state.h"
#include "../src/game.h"
#include "../src/fg.h"
#include "../src/ui.h"
#include "../src/aical1.h"
#include "../src/move.h"
#include "../src/rules.h"
#include "../src/saveload.h"

/* ---- bookkeeping ------------------------------------------------------------------------------------------------ */

static long g_tests, g_fail;
static int g_mut;                  /* the model's mutation, 0 = none (part C) */
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

#define TRSIZE (1 << 16)
static char g_tr[TRSIZE];
static size_t g_trlen;
static long g_rand;

/* path coverage of the model (counted only in part B's unmutated runs) */
enum { CV_C70_LAND, CV_C70_COAST, CV_C70_FLAG2, CV_C70_ASC, CV_C70_DESC, CV_C70_LAST, CV_C70_SELF,
       CV_3C0_FIRST, CV_3C0_SECOND, CV_3C0_ONE, CV_3C0_SUMS, CV_3C0_PAST,
       CV_EB8_SHORT, CV_EB8_SECOND, CV_EB8_LOOP, CV_EB8_CUT,
       CV_2BC_TAKEN, CV_548_P1, CV_548_P2, CV_548_NONE,
       CV_1D0_FLOOD, CV_1D0_BEST, CV_1D0_PAST, CV_1D0_PIR, CV_1D0_FIRST,
       CV_F28_S1, CV_F28_S2, CV_F28_ALL, CV_F28_T01, CV_F28_T02, CV_F28_NOT,
       CV_068_SWAP, CV_1B8_DUPS, CV_1B8_ZERO, CV_1B8_MAX, CV_1B8_COUNT,
       CV_E8C_CITY, CV_E8C_UNIT, CV_E8C_EMPTY, CV_N };
static const char *const g_cvname[CV_N] = {
    "20C70 land", "20C70 coast", "20C70 flag 2", "20C70 9 cities asc", "20C70 9 cities desc", "20C70 last",
    "20C70 p == v",
    "213C0 first", "213C0 second", "213C0 one city", "213C0 sums", "213C0 past the coastal record",
    "24EB8 1-2 cities", "24EB8 second", "24EB8 picks", "24EB8 cut by k (fixed)",
    "252BC taken", "25548 this round", "25548 others", "25548 none",
    "2E1D0 flood", "2E1D0 flood best", "2E1D0 flood list[count]", "2E1D0 piracy", "2E1D0 list[0]",
    "1EF28 sum 1", "1EF28 sum 2", "1EF28 all equal", "1EF28 tie 0/1", "1EF28 tie 0/2", "1EF28 no tie",
    "20068 swaps", "201B8 duplicates", "201B8 a zero", "201B8 the maximum", "201B8 [0xF7C9C]",
    "21E8C city", "21E8C tokens", "21E8C no record" };
static long g_cv[CV_N];
static int g_cvon;
#define CV(k) (g_cvon ? (void)g_cv[k]++ : (void)0)

static void tr(const char *fmt, ...)
{
    va_list ap;
    if (g_trlen >= TRSIZE - 300)
        return;
    va_start(ap, fmt);
    g_trlen += (size_t)vsnprintf(g_tr + g_trlen, TRSIZE - g_trlen, fmt, ap);
    va_end(ap);
}

/* ---- recorders and stubs ---------------------------------------------------------------------------------------- */

static jmp_buf g_jb;

_Noreturn void f44CAC(void) { tr("QUIT "); longjmp(g_jb, 1); }
static void unexpected(const char *name) { tr("UNEXPECTED %s ", name); }

void f79270(int wait, int n, int t, const char *text, int anim)
{
    tr("79270(%d,%d,%d,\"%s\",%d) ", wait, n, t, text, anim);
}
void f78A20(int t) { tr("78A20(%d) ", t); }
const char *mono_converts_fmt(uint32_t addr)
{
    tr("fmt(%X) ", (unsigned)addr);
    return g_original ? DSTR(addr) : "%s converts %s by Monotheism!";
}
void rec_f78E74(int t, int n) { tr("78E74(%d,%d) ", t, n); f78E74(t, n); }
void rec_f78F04(int t, int n) { tr("78F04(%d,%d) ", t, n); f78F04(t, n); }
void rec_f57CE8(int t, int n, int flag, int k) { tr("57CE8(%d,%d,%d,%d) ", t, n, flag, k); f57CE8(t, n, flag, k); }
void rec_f57E28(int t, int n, int k) { tr("57E28(%d,%d,%d) ", t, n, k); f57E28(t, n, k); }

/* 0x28CFC kind 0x1E (0x28D1A..0x28D3F, 0x29045..0x290D7): the list cleared to -1, then every t < 0x9D with a city
   (state +2 bit 1) whose owner is marked in 0xF7AC4 and whose territory byte +0 has bit 0x02 */
void f28CFC(int n, int kind, int s)
{
    int32_t t, i;
    tr("28CFC(%d,%d,%d) ", n, kind, s);
    setD32(0xF7C9Cu, 0);
    for (i = 0; i < 0x9D; i++)
        setD32(0xF7CA0u + 4u * (uint32_t)i, 0xFFFFFFFFu);
    if (kind != 0x1E) {
        unexpected("28CFC kind");
        return;
    }
    for (t = 0; t < 0x9D; t++) {
        if ((D8(0xE2C1Au + 3u * (uint32_t)t) & 1) == 0)
            continue;
        if (D8(0xF7AC4u + D8(0xE2C18u + 3u * (uint32_t)t)) == 0)
            continue;
        if ((D8(0xE1638u + 35u * (uint32_t)t) & 2) == 0)
            continue;
        setD32(0xF7CA0u + 4u * D32(0xF7C9Cu), (uint32_t)t);
        setD32(0xF7C9Cu, D32(0xF7C9Cu) + 1u);
    }
}

int f78AE4(void) { g_rand++; return 0; }

/* the loaders' and move.c's screen calls: never reached by the routines tested */
void f3A780(const char *msg) { printf("load message: %s\n", msg); }
void f3B7CC(uint32_t dlg, int id) { (void)dlg; (void)id; unexpected("3B7CC"); }
void f3B870(uint32_t dlg, int id) { (void)dlg; (void)id; unexpected("3B870"); }
int  f3B9A4(uint32_t dlg, int id) { (void)dlg; (void)id; unexpected("3B9A4"); return 0; }
void f3BFBC(uint32_t dlg, int id, int enabled) { (void)dlg; (void)id; (void)enabled; unexpected("3BFBC"); }
void f3C3D0(uint32_t dlg, int id) { (void)dlg; (void)id; unexpected("3C3D0"); }
void f3CBBC(void) { unexpected("3CBBC"); }
void f3D52C(int dx, int dy) { (void)dx; (void)dy; unexpected("3D52C"); }
void f3DA60(int t) { (void)t; unexpected("3DA60"); }
int  f3E89C(int t) { (void)t; unexpected("3E89C"); return 0; }
int  f3E8FC(int t) { (void)t; unexpected("3E8FC"); return 0; }
void f3F624(int t, int c) { (void)t; (void)c; unexpected("3F624"); }
void f3F670(void) { unexpected("3F670"); }
void f3F698(int t, int c) { (void)t; (void)c; unexpected("3F698"); }
void f3F70C(int t, int c) { (void)t; (void)c; unexpected("3F70C"); }
void f426D0(int i) { (void)i; unexpected("426D0"); }
void f572CC(int t) { (void)t; unexpected("572CC"); }
void f76F70(int n, uint8_t *cards) { (void)n; (void)cards; unexpected("76F70"); }
void fg_move(int x, int y) { (void)x; (void)y; }
void f3AE90(uint32_t spr, int x, int y, int w, int h) { (void)spr; (void)x; (void)y; (void)w; (void)h; unexpected("3AE90"); }
int  cat_show(const char *name, int flags, int handle) { (void)name; (void)flags; (void)handle; unexpected("cat_show"); return 0; }
int  fg_getpixel(int x, int y) { (void)x; (void)y; return 0; }
int  fg_point(int x, int y) { (void)x; (void)y; return 0; }
int  fg_setcolor(int color) { (void)color; return 0; }
void fg_vbclose(void) { }
int  fg_vbopen(int handle) { (void)handle; return 0; }

/* ---- model data helpers (independent of aical1.c) ---------------------------------------------------------------- */

static uint32_t rb(uint32_t a) { return ds_valid(a, 1) ? D8(a) : 0u; }                 /* movzx byte */
static int32_t rd(uint32_t a) { return ds_valid(a, 4) ? (int32_t)D32(a) : 0; }
static int32_t sar24(uint32_t a) { return ds_valid(a + 3u, 1) ? (int32_t)(int8_t)D8(a + 3u) : 0; }   /* dword [a] sar 24 */
static int32_t sar16(uint32_t a) { return ds_valid(a + 2u, 2) ? (int32_t)(int16_t)D16(a + 2u) : 0; } /* dword [a] sar 16 */
static void wbm(uint32_t a, uint32_t v) { if (ds_valid(a, 1)) setD8(a, (uint8_t)v); }

/* movsx word [p]: p a heap handle + offset (the adjacency blocks) or a data-object address */
static int32_t mw(uint32_t p)
{
    if (p & 0x80000000u) {
        const uint8_t *q = load_heap_ptr(p, 2);
        return q ? (int16_t)(q[0] | q[1] << 8) : 0;
    }
    return ds_valid(p, 2) ? (int16_t)D16(p) : 0;
}
static uint32_t area(int32_t r) { return 0xE31B8u + 69u * (uint32_t)r; }   /* shl 4, add, shl 2, add */
static uint32_t terr(int32_t t) { return 0xE1638u + 35u * (uint32_t)t; }   /* lea 9t; shl 2; sub t */
static uint32_t nat(int32_t n) { return 0xF14CCu + 199u * (uint32_t)n; }
static int32_t lst(int32_t i) { return rd(0xF7CA0u + 4u * (uint32_t)i); }
static const char *tname(int32_t t) { return DSTR(D32(0xC77D8u + 4u * (uint32_t)t)); }
static const char *nname(int32_t n) { return DSTR(D32(0xC768Cu + 4u * (uint32_t)n)); }

/* ---- the model ---------------------------------------------------------------------------------------------------- */

/* 0x213C0: frame ebp = esp_after_pushes - 0x82; locals at ebp - 0xA .. ebp + 0x7E */
static uint8_t F2[0x8C];
#define FX(x) ((x) + 0x0A)
static int32_t fd(int x) { uint8_t *p = F2 + FX(x); return (int32_t)((uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24); }
static void fdw(int x, int32_t v) { uint8_t *p = F2 + FX(x); p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }
static int32_t fb(int x) { return (int8_t)F2[FX(x)]; }
static void fbw(int x, int32_t v) { F2[FX(x)] = (uint8_t)v; }

static int32_t m213C0(int32_t eax)
{
    uint32_t h;
    int32_t i, j, e;

    memset(F2, 0xCC, sizeof F2);
    fbw(0x76, eax);
    fdw(0x5E, (int32_t)nat(fb(0x76)));
    fbw(0x6E, 0);
    goto L21414;
L213FC:
    fdw(4 * fb(0x6E) - 0xA, 0);
    fdw(4 * fb(0x6E) + 0x1A, 0);
L2140E:
    fbw(0x6E, fb(0x6E) + 1);
L21414:
    if (fb(0x6E) >= 9)
        goto L21479;
    i = fb(0x6E);
    if (rb((uint32_t)fd(0x5E) + (uint32_t)i + 0x52u) == 0)
        goto L213FC;
    fdw(4 * i - 0xA, (int32_t)rb((uint32_t)fd(0x5E) + (uint32_t)i + 0x52u));
    e = (int32_t)rb((uint32_t)fd(0x5E) + (uint32_t)i + 0x52u) - 1;
    fdw(4 * i + 0x1A, (int32_t)(((rb(terr(e) + 1u) << 2) & 0xFFu) >> (MUT(1) ? 4 : 5)));
    goto L2140E;
L21479:
    fbw(0x6E, 0);
    goto L21486;
L21480:
    fbw(0x6E, fb(0x6E) + 1);
L21486:
    if (fb(0x6E) >= 9)
        goto L2153C;
    fbw(0x7A, 1);
    goto L2149F;
L21499:
    fbw(0x7A, fb(0x7A) + 1);
L2149F:
    if (fb(0x7A) >= 9)
        goto L21480;
    j = fb(0x7A);
    fdw(0x46, fd(4 * j + 0x16));
    fdw(0x4A, fd(4 * j + 0x1A));
    if (MUT(2) ? fd(0x4A) < fd(0x46) : fd(0x4A) <= fd(0x46))
        goto L21499;
    fdw(0x56, fd(4 * j - 0xE));
    fdw(0x5A, fd(4 * j + 0x16));
    fdw(4 * j - 0xE, fd(4 * j - 0xA));
    fdw(4 * j + 0x16, fd(4 * j + 0x1A));
    fdw(4 * j - 0xA, fd(0x56));
    fdw(4 * j + 0x1A, fd(0x5A));
    goto L21499;
L2153C:
    fdw(0x46, fd(0x1A));
    fdw(0x4A, fd(0x1E));
    fdw(0x3E, 0);
    fdw(0x42, 1);
    if (fd(0x46) > fd(0x4A) && (rb(terr(fd(-0xA) - 1) + 1u) & 0x40u) != 0) {
        CV(CV_3C0_FIRST);
        return fd(-0xA);
    }
    if ((rb(terr(fd(-0xA) - 1) + 1u) & 0x40u) != 0)
        goto L21613;
    if (fd(-0x6) == 0)
        goto L21613;
    if ((rb(terr(fd(-0x6) - 1) + 1u) & 0x40u) == 0)
        goto L21613;
    fdw(0x46, fd(0x1E));
    fdw(0x4A, fd(0x22));
    fdw(0x3E, 1);
    fdw(0x42, 2);
    if (fd(0x46) > fd(0x4A) && (rb(terr(fd(-0x6) - 1) + 1u) & 0x40u) != 0) {
        CV(CV_3C0_SECOND);
        return fd(-0x6);
    }
L21613:
    if (fd(-0x6) == 0) {
        CV(CV_3C0_ONE);
        return fd(-0xA);
    }
    CV(CV_3C0_SUMS);
    fbw(0x72, 0);
    fbw(0x7E, 0);
    fbw(0x6E, 0);
    goto L21742;
L2163D:
    e = fd(4 * fd(0x42) - 0xA);
L21641:
    fdw(0x62, (int32_t)area(e));
    fbw(0x6A, 0);
    h = (uint32_t)rd(4u * rb((uint32_t)fd(0x62)) + 0xF8E78u);
    fdw(0x4E, (int32_t)h);
    fbw(0x7A, 1);
    goto L2167C;
L21676:
    fbw(0x7A, fb(0x7A) + 1);
L2167C:
    if (fb(0x7A) >= mw((uint32_t)fd(0x4E)) + 1)
        goto L216C2;
    fdw(0x66, (int32_t)area(mw((uint32_t)fd(0x4E) + 2u * (uint32_t)fb(0x7A)) + 1));
    fbw(0x6A, fb(0x6A) + (int8_t)rb((uint32_t)sar24((uint32_t)fd(0x5E) + 0x3Au) + (uint32_t)fd(0x66) + 0x0Eu));
    goto L21676;
L216C2:
    fdw(0x52, rd(4u * rb((uint32_t)fd(0x62)) + 0xF90F8u));
    fbw(0x7A, MUT(3) ? 1 : 0);
    goto L216E3;
L216DD:
    fbw(0x7A, fb(0x7A) + 1);
L216E3:
    if (fb(0x7A) >= mw((uint32_t)fd(0x52) + 2u * (uint32_t)fb(0x7A)))
        goto L21730;
    if (fb(0x7A) > mw((uint32_t)fd(0x52)))
        CV(CV_3C0_PAST);
    fdw(0x66, (int32_t)area(mw((uint32_t)fd(0x52) + 2u * (uint32_t)fb(0x7A)) + 1));
    fbw(0x6A, fb(0x6A) + (int8_t)rb((uint32_t)sar24((uint32_t)fd(0x5E) + 0x3Au) + (uint32_t)fd(0x66) + 0x0Eu));
    goto L216DD;
L21730:
    if (fb(0x6E) != 0)
        goto L2176B;
    fbw(0x7E, fb(0x6A));
L2173C:
    fbw(0x6E, fb(0x6E) + 1);
L21742:
    if (fb(0x6E) >= 2)
        goto L21773;
    if (fb(0x6E) != 0)
        goto L2163D;
    e = fd(4 * fd(0x3E) - 0xA);
    goto L21641;
L2176B:
    fbw(0x72, fb(0x6A));
    goto L2173C;
L21773:
    if (fb(0x7E) > fb(0x72))
        return fd(4 * fd(0x3E) - 0xA);
    return fd(4 * fd(0x42) - 0xA);
}

static int32_t m20C70(int32_t eax, int32_t edx)
{
    int32_t v2c = eax, v28 = edx, v24, v20, v1c;
    uint32_t vc = nat(v2c), v8 = nat(v28), v18, v14, v10;

    if (v2c == v28)
        goto L20F97;
    if (9 - sar24(vc + 0x4Eu) == 0)
        goto L20E84;
    v24 = 9;
    goto L20CDA;
L20CD4:
    v24--;
L20CDA:
    if (v24 < 0)
        goto L20D74;
    if ((int32_t)rb(v8 + (uint32_t)v24 + 0x52u) <= 0)
        goto L20CD4;
    v18 = (uint32_t)rd(4u * rb(v8 + (uint32_t)v24 + 0x52u) + 0xF8E78u);
    v20 = 1;
    goto L20D1D;
L20D17:
    v20++;
L20D1D:
    if (mw(v18) + 1 <= v20)
        goto L20CD4;
    v10 = area(mw(v18 + 2u * (uint32_t)v20) + 1);
    if (rb((uint32_t)sar24(vc + 0x3Au) + v10 + 1u) == 0)
        goto L20D17;
    CV(CV_C70_LAND);
    return (int32_t)rb(v8 + (uint32_t)v24 + 0x52u);
L20D74:
    v24 = 9;
    goto L20D83;
L20D7D:
    v24--;
L20D83:
    if (v24 < 0)
        goto L20E1D;
    if ((int32_t)rb(v8 + (uint32_t)v24 + 0x52u) <= 0)
        goto L20D7D;
    v14 = (uint32_t)rd(4u * rb(v8 + (uint32_t)v24 + 0x52u) + (MUT(4) ? 0xF8E78u : 0xF90F8u));
    v20 = 1;
    goto L20DC6;
L20DC0:
    v20++;
L20DC6:
    if (mw(v14) + 1 <= v20)
        goto L20D7D;
    v10 = area(mw(v14 + 2u * (uint32_t)v20) + 1);
    if (rb((uint32_t)sar24(vc + 0x3Au) + v10 + 1u) == 0)
        goto L20DC0;
    CV(CV_C70_COAST);
    return (int32_t)rb(v8 + (uint32_t)v24 + 0x52u);
L20E1D:
    v24 = 9;
    goto L20E2C;
L20E26:
    v24--;
L20E2C:
    if (v24 < 0)
        goto L20FA7;
    if ((int32_t)rb(v8 + (uint32_t)v24 + 0x52u) <= 0)
        goto L20E26;
    if ((rb(terr((int32_t)rb(v8 + (uint32_t)v24 + 0x52u) - 1)) & 2u) == 0)
        goto L20E26;
    CV(CV_C70_FLAG2);
    return (int32_t)rb(v8 + (uint32_t)v24 + 0x52u);
L20E84:
    v1c = (int8_t)rb(v8 + 0x5Cu) > (int8_t)rb(vc + 0x5Cu);
    if (MUT(5))
        v1c = !v1c;
    if (v1c == 0)
        goto L20F23;
    v24 = 0;
    goto L20EB4;
L20EAE:
    v24++;
L20EB4:
    if (sar24(v8 + 0x4Eu) <= v24)
        goto L20FA7;
    v10 = area((int32_t)rb(v8 + (uint32_t)v24 + 0x52u));
    if ((rb(terr((int32_t)rb(v10) - 1) + 1u) & 0x40u) != 0)
        goto L20EAE;
    CV(CV_C70_ASC);
    return (int32_t)rb(v8 + (uint32_t)v24 + 0x52u);
L20F23:
    v24 = sar24(v8 + 0x4Eu) - 1;
    goto L20F38;
L20F32:
    v24--;
L20F38:
    if (v24 < 0)
        goto L20FA7;
    v10 = area((int32_t)rb(v8 + (uint32_t)v24 + 0x52u));
    if ((rb(terr((int32_t)rb(v10) - 1) + 1u) & 0x40u) == 0)
        goto L20F32;
    CV(CV_C70_DESC);
    return (int32_t)rb(v8 + (uint32_t)v24 + 0x52u);
L20F97:
    CV(CV_C70_SELF);
    return m213C0(sar24(v8 + 0x3Au));
L20FA7:
    CV(CV_C70_LAST);
    return (int32_t)rb((uint32_t)sar24(v8 + 0x4Eu) + v8 + 0x51u);
}

#define MBI_STALE 0x003FF000   /* the model's stand-in for 0x2D4F0's text address (any value >= 157 alike) */

static int32_t m2E1D0(int32_t eax, int32_t edx)
{
    int32_t v1c = eax, v24 = edx, vc = 0, v20, v10 = 0, v14 = MBI_STALE, v18;
    int32_t v48[9];
    uint8_t v4;
    uint32_t v8;

    for (v20 = 0; v20 < 9; v20++)
        v48[v20] = 0;
    v20 = 0;
    goto L2E217;
L2E211:
    v20++;
L2E217:
    if (v20 >= rd(0xF7C9Cu))
        goto L2E280;
    v8 = (uint32_t)rd(4u * (uint32_t)lst(v20) + 0xF8E7Cu);
    v10 = 1;
    goto L2E245;
L2E23F:
    v10++;
L2E245:
    if (mw(v8) + 1 <= v10)
        goto L2E211;
    if (v20 < 9)
        v48[v20] += sar24(area(mw(v8 + 2u * (uint32_t)v10) + 1) + (uint32_t)v1c + 0x0Bu);   /* [+ n + 0xE31C3] */
    goto L2E23F;
L2E280:
    for (v20 = 0; v20 < rd(0xF7C9Cu); v20++)
        if (v20 < 9 && (MUT(6) ? v48[v20] >= vc : v48[v20] > vc)) {
            vc = v48[v20];
            v14 = v20;
        }
    if (v24 != 0x19)
        goto L2E3D6;
    v20 = sar24(nat(v1c) + 0x4Eu) - 1;
    goto L2E2DE;
L2E2D8:
    v20--;
L2E2DE:
    if (v20 < 0)
        goto L2E37E;
    v4 = 0;
    v10 = 0;
    goto L2E2FA;
L2E2F4:
    v10++;
L2E2FA:
    if (v10 >= rd(0xF7C9Cu))
        goto L2E2D8;
    for (v18 = 0; v18 < 4; v18++)
        if (rd(0xC0618u + 16u * (uint32_t)v1c + 4u * (uint32_t)v18) == lst(v10)) {
            v4 = 1;
            break;
        }
    if ((int32_t)rb(nat(v1c) + (uint32_t)v20 + 0x52u) != lst(v10) + 1)
        goto L2E2F4;
    if (v4 != 0 && v14 != v10 && !MUT(7))
        goto L2E2F4;
    CV(v4 != 0 ? CV_1D0_BEST : CV_1D0_FLOOD);
    return lst(v10);
L2E37E:
    v20 = sar24(nat(v1c) + 0x4Eu) - 1;
    goto L2E39A;
L2E394:
    v20--;
L2E39A:
    if (v20 < 0)
        goto L2E44F;
    if ((int32_t)rb(nat(v1c) + (uint32_t)v20 + 0x52u) != lst(MUT(8) ? 0 : v10) + 1)
        goto L2E394;
    CV(CV_1D0_PAST);
    return lst(MUT(8) ? 0 : v10);
L2E3D6:
    if (v24 != 0x1E)
        goto L2E44F;
    for (v20 = 0; sar24(nat(v1c) + 0x4Eu) > v20; v20++)
        for (v10 = 0; v10 < rd(0xF7C9Cu); v10++)
            if ((int32_t)rb(nat(v1c) + (uint32_t)v20 + 0x52u) - 1 == lst(v10)) {
                CV(CV_1D0_PIR);
                return lst(v10);
            }
L2E44F:
    CV(CV_1D0_FIRST);
    return rd(0xF7CA0u);
}

static void m_marks(int32_t t, int32_t owner)
{
    int32_t v24;
    wbm(area(t + 1) + (uint32_t)owner + 0x3Bu, 2);
    for (v24 = 0; v24 < 9; v24++)
        if (sar24(area(t + 1) + (uint32_t)v24 + 0x39u) == (MUT(9) ? 2 : 1))
            wbm(area(t + 1) + (uint32_t)v24 + 0x3Cu, 2);
}

static int m_wait(void) { return (rb(0xF0980u) & 4u) ? (int)0xFFFFFE0C : 0x9C4; }

static void m24EB8(int32_t eax, int32_t edx, int32_t ebx)
{
    int32_t v2c = eax, v28 = edx, v30 = ebx, v10, v24, v14, v1c, v18, v20 = 0;
    int32_t v54[160];
    char vb8[100];

    (void)v2c;
    memset(v54, 0, sizeof v54);
    v10 = 0;
    for (v24 = 0; v24 < 12; v24++)
        wbm(0xF7AC4u + (uint32_t)v24, 0);
    wbm(0xF7AC4u + (uint32_t)v28, 1);
    f28CFC(v28, 0x1E, 0);
    for (v14 = 0; v14 < rd(0xF7C9Cu); v14++)
        if ((int32_t)rb(3u * (uint32_t)lst(v14) + 0xE2C18u) == v28)
            v54[v10++] = lst(v14);
    v1c = v10 > 2 ? 2 : v10;
    if (!g_original && v1c > v30 && !MUT(10)) {   /* the fix (aical1.c bug 1) */
        CV(CV_EB8_CUT);
        v1c = v30;
    }
    if (v10 < 1 || v10 > 2)
        goto L25078;
    if (!g_original && v1c <= 0)   /* the fix: k <= 0 takes nothing */
        goto L25078;
    CV(CV_EB8_SHORT);
    v18 = v54[0];
    rec_f78F04(v18, v28);
    rec_f78E74(v54[0], 0xA);
    m_marks(v54[0], v28);
    snprintf(vb8, sizeof vb8, "%s is overrun by the Pirates.", tname(v18));
    f79270(m_wait(), v28, v18, vb8, 1);
    v1c--;
L25078:
    if (rd(0xF7C9Cu) != 2)
        goto L25175;
    if (!g_original && v1c <= 0)
        goto L25175;
    CV(CV_EB8_SECOND);
    v18 = v54[1];
    rec_f78F04(v18, v28);
    rec_f78E74(rd(0xF7CA4u), 0xA);
    m_marks(v54[1], v28);
    snprintf(vb8, sizeof vb8, "%s is overrun by the Pirates.", tname(v18));
    f79270(m_wait(), v28, v18, vb8, 1);
    v1c--;
    return;
L25175:
    for (v14 = 0; v14 < v1c; v14++) {
        CV(CV_EB8_LOOP);
        v18 = m2E1D0(v28, 0x1E);
        rec_f78F04(v18, v28);
        for (v24 = 0; v24 < rd(0xF7C9Cu); v24++)
            if (v54[v24] == v18) {
                v20 = v24;
                break;
            }
        rec_f78E74(v54[v20], 0xA);
        m_marks(v54[v20], v28);
        snprintf(vb8, sizeof vb8, "%s is overrun by the Pirates.", tname(v18));
        f79270(m_wait(), v28, v18, vb8, 1);
    }
}

static int32_t m25548(int32_t eax)
{
    int32_t v28 = eax, v14 = eax - 1, v24, v20, v1c, v18;
    int32_t r[2 * 160];   /* [ebp-0x530 + 8i]: t, sum */
    int32_t tmp0, tmp1;
    uint32_t v8 = nat(v28), v4;
    int pass;

    memset(r, 0, sizeof r);
    for (pass = 0; pass < 2; pass++) {
        v24 = 0;
        for (v20 = 0; sar24(v8 + 0x4Eu) > v20; v20++) {
            if ((rb(terr((int32_t)rb(v8 + (uint32_t)v20 + 0x52u) - 1)) & 2u) == 0)
                continue;
            if (pass == 0 ? rb(3u * (uint32_t)((int32_t)rb(v8 + (uint32_t)v20 + 0x52u) - 1) + 0xE2C19u) != (uint32_t)sar16(0xF40FAu)
                          : rb(3u * (uint32_t)((int32_t)rb(v8 + (uint32_t)v20 + 0x52u) - 1) + 0xE2C19u) == (uint32_t)sar16(0xF40FAu))
                continue;
            if (v24 >= 160)
                break;
            r[2 * v24] = (int32_t)rb(v8 + (uint32_t)v20 + 0x52u) - 1;
            v4 = (uint32_t)rd(4u * (uint32_t)r[2 * v24] + 0xF8E7Cu);
            v18 = 0;
            for (v1c = 1; mw(v4) >= v1c; v1c++)
                v18 += sar24(area(mw(v4 + 2u * (uint32_t)v1c) + 1) + (uint32_t)v14 + 0x0Cu);
            r[2 * v24 + 1] = v18;
            v24++;
        }
        if (v24 == 0)
            continue;
        CV(pass == 0 ? CV_548_P1 : CV_548_P2);
        for (v20 = 1; v20 < v24; v20++)
            for (v1c = v24 - 1; v1c >= v20; v1c--) {
                uint32_t bl = ((rb(terr(r[2 * (v1c - 1)]) + 1u) << 2) & 0xFFu) >> 5;
                uint32_t al = ((rb(terr(r[2 * v1c]) + 1u) << 2) & 0xFFu) >> 5;
                if (MUT(11) ? bl >= al : bl <= al)
                    continue;
                tmp0 = r[2 * (v1c - 1)]; tmp1 = r[2 * (v1c - 1) + 1];
                r[2 * (v1c - 1)] = r[2 * v1c]; r[2 * (v1c - 1) + 1] = r[2 * v1c + 1];
                r[2 * v1c] = tmp0; r[2 * v1c + 1] = tmp1;
            }
        for (v20 = 1; v20 < v24; v20++)
            for (v1c = v24 - 1; v1c >= v20; v1c--) {
                if (r[2 * (v1c - 1) + 1] <= r[2 * v1c + 1])
                    continue;
                tmp0 = r[2 * (v1c - 1)]; tmp1 = r[2 * (v1c - 1) + 1];
                r[2 * (v1c - 1)] = r[2 * v1c]; r[2 * (v1c - 1) + 1] = r[2 * v1c + 1];
                r[2 * v1c] = tmp0; r[2 * v1c + 1] = tmp1;
            }
        return r[0];
    }
    CV(CV_548_NONE);
    return -1;
}

static void m252BC(int32_t eax, int32_t edx, int32_t ebx)
{
    int32_t v34 = eax, v38 = edx, v2c = ebx, v1c, v28, v30, v24, v14, v18, v20, v10;
    int32_t v60[10];
    char vc4[100];

    memset(v60, 0, sizeof v60);
    v1c = 0;
    for (v28 = 1; v28 <= 9; v28++) {
        if (rb(199u * (uint32_t)v28 + 0xF1566u) == 0)
            continue;
        if (sar24(199u * (uint32_t)v28 + 0xF151Au) <= 0)
            continue;
        if (v28 == v38 || (!MUT(12) && v28 == v34))
            continue;
        v60[v1c++] = v28;
    }
    for (v28 = v1c - 2; v28 >= 0; v28--)
        for (v30 = 0; v30 <= v28; v30++)
            if (rd(0xF52E0u + 40u * (uint32_t)v38 + 4u * (uint32_t)v60[v30 + 1])
                > rd(0xF52E0u + 40u * (uint32_t)v38 + 4u * (uint32_t)v60[v30])) {
                v24 = v60[v30 + 1];
                v60[v30 + 1] = v60[v30];
                v60[v30] = v24;
            }
    v14 = v2c;
    for (v18 = 0; v14 > 0 && v18 < v1c; v18++) {
        v20 = m25548(v60[v18]);
        if (v20 < 0)
            continue;
        CV(CV_2BC_TAKEN);
        v10 = (int32_t)rb(3u * (uint32_t)v20 + 0xE2C18u);
        rec_f78F04(v20, v60[v18]);
        rec_f78E74(v20, 0xA);
        m_marks(v20, v10);
        snprintf(vc4, sizeof vc4, "Pirates overrun %s as a secondary victim.", tname(v20));
        f79270(m_wait(), v38, v20, vc4, 1);
        v14--;
    }
}

static int32_t m1EF28(int32_t eax, int32_t *edx)
{
    int32_t v1c = eax, t;
    int8_t vc = 0, v14 = 0, v10, v4 = 0, v8;

    for (v10 = 1; v10 < 3; v10++)
        for (v4 = 0; v4 < 2; v4++)
            for (v8 = 0; v8 < 11; v8++) {
                t = sar16(0xBDE12u + 4u * (uint32_t)v10 + 2u * (uint32_t)v4);
                if (sar24(area(t) + (uint32_t)v8 + 0x0Cu) > 0 && v8 + 1 != v1c)
                    goto L1F084;
                if ((rb(3u * (uint32_t)(t - 1) + 0xE2C1Au) & 1u) != 1u)
                    continue;
                if ((int32_t)rb(3u * (uint32_t)(t - 1) + 0xE2C18u) == v1c)
                    continue;
                if (rb(3u * (uint32_t)(t - 1) + 0xE2C18u) == (MUT(13) ? 11u : 10u))
                    continue;
L1F084:
                v14 = (int8_t)(v14 + v10);
            }
    if (v14 == 1) {
        CV(CV_F28_S1);
        return 0;
    }
    if (v14 == 2) {
        CV(CV_F28_S2);
        return 1;
    }
    if (edx[0] == edx[1] && edx[0] == edx[2] && (CV(CV_F28_ALL), 1))
        return v1c != 5 ? 2 : (v1c == 1 ? 0 : 1);
    if (edx[0] == edx[1]) {
        CV(CV_F28_T01);
        v14 = 0;
        vc = 0;
        for (v10 = 0; v10 < 9; v10++)
            if (sar24(0xE3AEEu + (uint32_t)v10) > 0 && v4 + 1 != v1c)
                v14 = (int8_t)(v14 + (int8_t)rb(0xE3AF1u + (uint32_t)v10));
        for (v10 = 0; v10 < 9; v10++)
            if (sar24(0xE3B33u + (uint32_t)v10) > 0 && v4 + 1 != v1c)
                v14 = (int8_t)(v14 + (int8_t)rb(0xE3B36u + (uint32_t)v10));
        for (v10 = 0; v10 < 9; v10++)
            if (sar24(0xE4052u + (uint32_t)v10) > 0 && v4 + 1 != v1c)
                v14 = (int8_t)(v14 + (int8_t)rb(0xE4055u + (uint32_t)v10));
        for (v10 = 0; v10 < 9; v10++)
            if (sar24(0xE4640u + (uint32_t)v10) > 0 && v4 + 1 != v1c) {
                if (MUT(14))
                    vc = (int8_t)(vc + (int8_t)rb(0xE4643u + (uint32_t)v10));
                else
                    vc = (int8_t)rb(0xE4643u + (uint32_t)v10);
            }
        return v14 <= vc ? 1 : 0;
    }
    if (edx[0] == edx[2]) {
        CV(CV_F28_T02);
        return v1c != 5 ? 2 : 0;
    }
    CV(CV_F28_NOT);
    return v1c == 5 ? 1 : 2;
}

static void m20068(void)
{
    int32_t v8, v4;
    uint8_t tmp[8];

    if (rd(0xF7C9Cu) <= 1)
        return;
    for (v8 = rd(0xF7C9Cu) - 2; v8 >= 0; v8--)
        for (v4 = 0; v4 <= v8; v4++)
            if ((int16_t)D16(0xF730Eu + 8u * (uint32_t)v4) > (int16_t)D16(0xF7306u + 8u * (uint32_t)v4)) {
                CV(CV_068_SWAP);
                memcpy(DP(0xF77D8u), DP(0xF7308u + 8u * (uint32_t)v4), 8);
                memcpy(DP(0xF7308u + 8u * (uint32_t)v4), DP(0xF7300u + 8u * (uint32_t)v4), 8);
                memcpy(DP(0xF7300u + 8u * (uint32_t)v4), DP(0xF77D8u), 8);
            }
    if (MUT(15))
        return;
    for (v8 = rd(0xF7C9Cu) - 2; v8 >= 0; v8--)
        for (v4 = 0; v4 <= v8; v4++)
            if ((int16_t)D16(0xF730Cu + 8u * (uint32_t)v4) > (int16_t)D16(0xF7304u + 8u * (uint32_t)v4)) {
                if (MUT(16)) {
                    memcpy(tmp, DP(0xF7308u + 8u * (uint32_t)v4), 8);
                    memcpy(DP(0xF7308u + 8u * (uint32_t)v4), DP(0xF7300u + 8u * (uint32_t)v4), 8);
                    memcpy(DP(0xF7300u + 8u * (uint32_t)v4), tmp, 8);
                    continue;
                }
                memcpy(DP(0xF77D8u), DP(0xF7308u + 8u * (uint32_t)v4), 8);
                memcpy(DP(0xF7308u + 8u * (uint32_t)v4), DP(0xF7300u + 8u * (uint32_t)v4), 8);
                memcpy(DP(0xF7300u + 8u * (uint32_t)v4), DP(0xF77D8u), 8);
            }
}

static int32_t m201B8(int32_t eax)
{
    int32_t v1c = eax, v14 = -1, v8 = 0, v4 = 0, v10, v18;
    int16_t vd0[160];
    uint32_t vc;

    memset(vd0, 0, sizeof vd0);
    for (v10 = 0; v10 < rd(0xF7C9Cu) && v10 < 157; v10++) {
        vc = (uint32_t)rd(4u * (uint32_t)lst(v10) + 0xF8E7Cu);
        for (v18 = MUT(17) ? 1 : 0; mw(vc) > v18; v18++)
            vd0[v10] = (int16_t)(vd0[v10] + (int16_t)(int8_t)rb(area(mw(vc + 2u * (uint32_t)v18) + 1) + (uint32_t)v1c + 0x0Eu));
    }
    for (v10 = 0; rd(0xF7C9Cu) - 1 > v10 && v10 < 157; v10++)
        for (v18 = 0; rd(0xF7C9Cu) - 1 > v18 && v18 < 157; v18++)
            if (vd0[v10] == vd0[v18] && v10 != v18)
                v8++;
    if ((int16_t)v8 != 0) {
        CV(CV_1B8_DUPS);
        goto L20364;
    }
    for (v10 = 0; v10 < rd(0xF7C9Cu) && v10 < 157; v10++)
        if (vd0[v10] == 0)
            v14 = MUT(18) && v14 >= 0 ? v14 : v10;
    if (v14 >= 0) {
        CV(CV_1B8_ZERO);
        goto L20314;
    }
    CV(CV_1B8_MAX);
    for (v10 = 0; v10 < rd(0xF7C9Cu) && v10 < 157; v10++)
        if (vd0[v10] > (int16_t)v4) {
            v4 = (uint16_t)vd0[v10];
            v14 = v10;
        }
L20314:
    if (v14 < 0)
        CV(CV_1B8_COUNT);
    return rd(4u * (uint32_t)v14 + 0xF7CA0u);
L20364:
    v14 = 0;
    for (v10 = 0; v10 < rd(0xF7C9Cu) && v10 < 157; v10++)
        if (vd0[v10] > (int16_t)v4) {
            v4 = (uint16_t)vd0[v10];
            v14 = v10;
        }
    goto L20314;
}

/* 0x21E8C: records of 12 bytes at [ebp-0x858 + 12i], temp [ebp-0x40] */
static uint8_t FM[0x858];
#define REC(i) (FM + 12 * (i))
static int32_t rget(int i, int f) { int32_t v; memcpy(&v, REC(i) + 4 * f, 4); return v; }
static void rset(int i, int f, int32_t v) { memcpy(REC(i) + 4 * f, &v, 4); }
static void rswap(int j)
{
    uint8_t v40[12];
    memcpy(v40, REC(j + 1), 12);
    memcpy(REC(j + 1), REC(j), 12);
    memcpy(REC(j), v40, 12);
}

static void m21E8C(int32_t eax)
{
    int32_t v18 = eax, v20, v28, v24, v34, v30, v2c;
    uint32_t v1c = nat(v18);
    char vd8[152];

    memset(FM, 0, sizeof FM);
    if (sar24(v1c + 0x4Eu) >= 9)
        goto L2225A;
    if (sar24(v1c + 0x95u) / 2 < sar24(v1c + 0x4Eu) + 1)
        goto L2225A;
    v20 = 0;
    for (v28 = 0; v28 < rd(0xF7C9Cu); v28++)
        if ((rb(3u * (uint32_t)lst(v28) + 0xE2C1Au) & 1u) != 0 && v20 < 160)
            rset(v20++, 0, lst(v28));
    if (v20 <= 0)
        goto L2225A;
    for (v28 = v20 - 2; v28 >= 0; v28--)
        for (v24 = 0; v24 <= v28; v24++)
            if ((int8_t)rb(0xEED84u + 160u * (uint32_t)v18 + (uint32_t)rget(v24 + 1, 0))
                > (int8_t)rb(0xEED84u + 160u * (uint32_t)v18 + (uint32_t)rget(v24, 0)))
                rswap(v24);
    if (!MUT(19))
        for (v28 = v20 - 2; v28 >= 0; v28--)
            for (v24 = 0; v24 <= v28; v24++)
                if (rd(0xF52E0u + 40u * (uint32_t)v18 + 4u * rb(3u * (uint32_t)rget(v24 + 1, 0) + 0xE2C18u))
                    > rd(0xF52E0u + 40u * (uint32_t)v18 + 4u * rb(3u * (uint32_t)rget(v24, 0) + 0xE2C18u)))
                    rswap(v24);
    CV(CV_E8C_CITY);
    v34 = rget(0, 0);
    snprintf(vd8, sizeof vd8, mono_converts_fmt(0xB1EEAu), nname(v18), tname(v34));
    f79270((int)0xFFFFFDDA, v18, v34, vd8, 0);
    f78A20(0x12C);
    wbm(area(v34 + 1) + rb(3u * (uint32_t)v34 + 0xE2C18u) + 0x3Bu, 1);
    rec_f78F04(v34, (int)rb(3u * (uint32_t)v34 + 0xE2C18u));
    rec_f78E74(v34, v18);
    goto L22238;
L2225A:
    v20 = 0;
    for (v28 = 0; v28 < rd(0xF7C9Cu); v28++) {
        v30 = lst(v28);
        if ((rb(3u * (uint32_t)lst(v28) + 0xE2C1Au) & 1u) != 0)
            continue;
        if (v20 >= 160)
            break;
        rset(v20, 0, lst(v28));
        rset(v20, 1, 0);
        rset(v20, 2, 0);
        for (v24 = 0; v24 < 9; v24++) {
            if (rb(199u * (uint32_t)((MUT(20) ? v24 : v28) + 1) + 0xF1566u) == 0)
                continue;
            if (sar24(area(v30 + 1) + (uint32_t)v24 + 0x0Cu) <= rget(v20, 2))
                continue;
            rset(v20, 2, sar24(area(v30 + 1) + (uint32_t)v24 + 0x0Cu));
            rset(v20, 1, v24 + 1);
        }
        v20++;
    }
    CV(v20 == 0 ? CV_E8C_EMPTY : CV_E8C_UNIT);
    if (v20 == 0 && !g_original)   /* the fix (aical1.c bug 2) */
        return;
    for (v28 = v20 - 2; v28 >= 0; v28--)
        for (v24 = 0; v24 <= v28; v24++)
            if ((int8_t)rb(0xEED84u + 160u * (uint32_t)v18 + (uint32_t)rget(v24 + 1, 0))
                > (int8_t)rb(0xEED84u + 160u * (uint32_t)v18 + (uint32_t)rget(v24, 0)))
                rswap(v24);
    for (v28 = v20 - 2; v28 >= 0; v28--)
        for (v24 = 0; v24 <= v28; v24++)
            if (rd(0xF52E0u + 40u * (uint32_t)v18 + 4u * (uint32_t)rget(v24 + 1, 1))
                > rd(0xF52E0u + 40u * (uint32_t)v18 + 4u * (uint32_t)rget(v24, 1)))
                rswap(v24);
    for (v28 = v20 - 2; v28 >= 0; v28--)
        for (v24 = 0; v24 <= v28; v24++)
            if (MUT(21) ? rget(v24 + 1, 2) < rget(v24, 2) : rget(v24 + 1, 2) > rget(v24, 2))
                rswap(v24);
    v34 = rget(0, 0);
    snprintf(vd8, sizeof vd8, mono_converts_fmt(0xB1F07u), nname(v18), tname(v34));
    f79270((int)0xFFFFFDDA, v18, v34, vd8, 0);
    f78A20(0x12C);
    for (v28 = 0; v28 < 9; v28++) {
        if (rb(area(v34 + 1) + (uint32_t)v28 + 0x0Fu) == 0)
            continue;
        wbm(area(v34 + 1) + (uint32_t)v28 + 0x3Cu, 1);
        v2c = sar24(area(v34 + 1) + (uint32_t)v28 + 0x0Cu);
        rec_f57CE8(v34, v28 + 1, 0, v2c);
        wbm(199u * (uint32_t)(v28 + 1) + 0xF1564u, rb(199u * (uint32_t)(v28 + 1) + 0xF1564u) - (uint32_t)v2c);
        wbm(199u * (uint32_t)(v28 + 1) + 0xF1565u, rb(199u * (uint32_t)(v28 + 1) + 0xF1565u) + (uint32_t)v2c);
        rec_f57E28(v34, v18, v2c);
        wbm(199u * (uint32_t)v18 + 0xF1565u, rb(199u * (uint32_t)v18 + 0xF1565u) - (uint32_t)v2c);
        wbm(199u * (uint32_t)v18 + 0xF1564u, rb(199u * (uint32_t)v18 + 0xF1564u) + (uint32_t)v2c);
    }
L22238:
    f79270((int)0xFFFFFE0C, v18, v34, vd8, 0);
}

/* ---- running one operation on both sides ------------------------------------------------------------------------ */

enum { OP_20C70, OP_213C0, OP_24EB8, OP_252BC, OP_25548, OP_2E1D0, OP_1EF28, OP_20068, OP_201B8, OP_21E8C, OP_N };
static const char *const g_opname[OP_N] = { "20C70", "213C0", "24EB8", "252BC", "25548", "2E1D0", "1EF28", "20068",
                                            "201B8", "21E8C" };

typedef struct { int op, a, b, c; int32_t sc[3]; } Op;

static int32_t run(const Op *o, int model)
{
    int32_t r = 0, sc[3];
    memcpy(sc, o->sc, sizeof sc);
    g_trlen = 0;
    g_tr[0] = 0;
    if (setjmp(g_jb) != 0)
        return -9999;
    switch (o->op) {
    case OP_20C70: r = model ? m20C70(o->a, o->b) : f20C70(o->a, o->b); break;
    case OP_213C0: r = model ? m213C0(o->a) : f213C0(o->a); break;
    case OP_24EB8: if (model) m24EB8(o->a, o->b, o->c); else f24EB8(o->a, o->b, o->c); break;
    case OP_252BC: if (model) m252BC(o->a, o->b, o->c); else f252BC(o->a, o->b, o->c); break;
    case OP_25548: r = model ? m25548(o->a) : f25548(o->a); break;
    case OP_2E1D0: r = model ? m2E1D0(o->a, o->b) : f2E1D0(o->a, o->b); break;
    case OP_1EF28: r = model ? m1EF28(o->a, sc) : f1EF28(o->a, sc); break;
    case OP_20068: if (model) m20068(); else f20068(o->a); break;
    case OP_201B8: r = model ? m201B8(o->a) : f201B8(o->a); break;
    case OP_21E8C: if (model) m21E8C(o->a); else f21E8C(o->a); break;
    default: break;
    }
    return r;
}

static uint8_t *g_start, *g_port;
static char g_trport[TRSIZE];
static long g_opcount[OP_N];

/* runs o on the current state with both sides; 1 if they agree */
static int compare(const Op *o, const char *tag)
{
    int32_t rp, rm;
    char what[200];
    size_t i;
    int ok;

    memcpy(g_start, g_ds, DS_SIZE);
    rp = run(o, 0);
    memcpy(g_port, g_ds, DS_SIZE);
    memcpy(g_trport, g_tr, g_trlen + 1);
    memcpy(g_ds, g_start, DS_SIZE);
    rm = run(o, 1);
    ok = rp == rm && strcmp(g_trport, g_tr) == 0 && memcmp(g_port, g_ds, DS_SIZE) == 0 && g_rand == 0;
    if (!ok && g_mut == 0 && g_fail < 30) {
        snprintf(what, sizeof what, "%s %s(%d,%d,%d) orig %d: result %d / %d", tag, g_opname[o->op], o->a, o->b, o->c,
                 g_original, rp, rm);
        printf("  %s\n", what);
        if (strcmp(g_trport, g_tr) != 0)
            printf("    trace port : %.400s\n    trace model: %.400s\n", g_trport, g_tr);
        for (i = 0; i < DS_SIZE; i++)
            if (g_port[i] != g_ds[i]) {
                printf("    first data difference at 0x%X: port %02X model %02X\n", (unsigned)(i + DS_BASE), g_port[i],
                       g_ds[i]);
                break;
            }
    }
    return ok;
}

/* ---- random states ------------------------------------------------------------------------------------------------ */

static uint32_t g_rng = 12345u;
static uint32_t rnd(void) { g_rng = g_rng * 1103515245u + 12345u; return (g_rng >> 8) & 0xFFFFFFu; }
static int rr(int n) { return n > 0 ? (int)(rnd() % (uint32_t)n) : 0; }

static int is_city(int t) { return (D8(0xE2C1Au + 3u * (uint32_t)t) & 1) != 0; }

static void city_remove_all(int n)
{
    int i, c;
    for (i = 0; i < 10; i++) {
        c = D8(nat(n) + 0x52u + (uint32_t)i);
        if (i < S8(nat(n) + 0x51u) && c > 0 && c <= 157) {
            setD8(0xE2C1Au + 3u * (uint32_t)(c - 1), (uint8_t)(D8(0xE2C1Au + 3u * (uint32_t)(c - 1)) & ~1u));
            setD8(0xE2C18u + 3u * (uint32_t)(c - 1), 0);
            setD8(0xE2C19u + 3u * (uint32_t)(c - 1), 0xFF);
        }
    }
    for (i = 0; i < 9; i++)
        setD8(nat(n) + 0x52u + (uint32_t)i, 0);
    setD8(nat(n) + 0x51u, 0);
}

static int city_add(int n, int t)
{
    int cnt = S8(nat(n) + 0x51u);
    if (cnt >= 9 || is_city(t))
        return 0;
    setD8(0xE2C1Au + 3u * (uint32_t)t, (uint8_t)(D8(0xE2C1Au + 3u * (uint32_t)t) | 1u));
    setD8(0xE2C18u + 3u * (uint32_t)t, (uint8_t)n);
    setD8(0xE2C19u + 3u * (uint32_t)t, (uint8_t)(S16(0xF40FCu) - rr(3)));
    setD8(nat(n) + 0x52u + (uint32_t)cnt, (uint8_t)(t + 1));
    setD8(nat(n) + 0x51u, (uint8_t)(cnt + 1));
    return 1;
}

static void clear_tokens(int n)
{
    int r;
    for (r = 0; r < 161; r++)
        setD8(area(r) + 0x0Eu + (uint32_t)n, 0);
}

static int coastal(int t) { return (D8(0xE1638u + 35u * (uint32_t)t) & 2) != 0; }

static void random_state(const uint8_t *save)
{
    int i, n, t, k;

    memcpy(g_ds, save, DS_SIZE);
    setD16(0xF40FCu, (uint16_t)(4 + rr(4)));
    for (n = 1; n <= 11; n++) {
        if (rr(3) == 0)
            continue;
        city_remove_all(n);
        k = rr(4) == 0 ? 9 : rr(10);
        if (n == 10)
            k = rr(10);
        for (i = 0; i < 40 && S8(nat(n) + 0x51u) < k; i++) {
            t = rr(157);
            if (rr(2) && !coastal(t))
                continue;
            city_add(n, t);
        }
    }
    for (i = 0; i < 400; i++) {
        t = rr(161);
        n = 1 + rr(11);
        k = rr(8) == 0 ? 60 + rr(68) : rr(9);
        if (rr(3) == 0)
            k = 0;
        setD8(area(t) + 0x0Eu + (uint32_t)n, (uint8_t)k);
        if (rr(2))
            setD8(area(t) + 1u + (uint32_t)n, (uint8_t)(k ? n : 0));
        if (rr(4) == 0)
            setD8(area(t) + 0x3Bu + (uint32_t)rr(12), (uint8_t)rr(3));
    }
    for (n = 1; n <= 13; n++) {
        if (rr(2))
            setD8(nat(n) + 0x5Cu, (uint8_t)rr(16));
        if (rr(2))
            setD8(nat(n) + 0x98u, (uint8_t)rr(56));
        if (rr(2))
            setD8(nat(n) + 0x99u, (uint8_t)rr(56));
        if (n <= 9 && rr(6) == 0)
            setD8(nat(n) + 0x9Au, (uint8_t)(rr(3) == 0 ? 0 : 2 + rr(2)));
    }
    for (i = 0; i < 60; i++)
        setD32(0xF52E0u + 4u * (uint32_t)rr(140), (uint32_t)(rr(9) - 3));
    for (i = 0; i < 300; i++)
        setD8(0xEED84u + (uint32_t)rr(160 * 10), (uint8_t)(rr(7) - 2));
    for (i = 0; i < 20; i++) {
        t = rr(157);
        setD8(0xE1639u + 35u * (uint32_t)t, (uint8_t)(D8(0xE1639u + 35u * (uint32_t)t) ^ (1u << rr(8))));
    }
    if (rr(2))
        setD8(0xF0980u, (uint8_t)(D8(0xF0980u) ^ 4u));
    for (i = 0; i < 40; i++) {
        setD16(0xF7300u + 8u * (uint32_t)i, (uint16_t)rnd());
        setD16(0xF7302u + 8u * (uint32_t)i, (uint16_t)rnd());
        setD16(0xF7304u + 8u * (uint32_t)i, (uint16_t)(rr(4) - 1));
        setD16(0xF7306u + 8u * (uint32_t)i, (uint16_t)(rr(4) - 1));
    }
    for (i = 0; i < 157; i++)   /* the list and the words past it */
        setD32(0xF7CA0u + 4u * (uint32_t)i, rr(3) == 0 ? 0xFFFFFFFFu : (uint32_t)rr(157));
}

/* the choice list: n's cities (some), other territories (some) */
static void random_list(int n, int maxc, int others)
{
    int cnt = 0, i, t;
    int cities = S8(nat(n) + 0x51u);
    for (i = 0; i < cities && cnt < maxc; i++)
        if (rr(4))
            setD32(0xF7CA0u + 4u * (uint32_t)cnt++, (uint32_t)(D8(nat(n) + 0x52u + (uint32_t)i) - 1));
    for (i = 0; i < others && cnt < maxc; i++) {
        t = rr(157);
        setD32(0xF7CA0u + 4u * (uint32_t)cnt++, (uint32_t)t);
    }
    for (i = cnt - 1; i > 0; i--) {   /* shuffle */
        int j = rr(i + 1);
        uint32_t a = D32(0xF7CA0u + 4u * (uint32_t)i), b = D32(0xF7CA0u + 4u * (uint32_t)j);
        setD32(0xF7CA0u + 4u * (uint32_t)i, b);
        setD32(0xF7CA0u + 4u * (uint32_t)j, a);
    }
    setD32(0xF7C9Cu, (uint32_t)cnt);
}

static Op random_op(int op)
{
    Op o;
    memset(&o, 0, sizeof o);
    o.op = op;
    switch (op) {
    case OP_20C70:
        o.a = 1 + rr(9);
        o.b = rr(5) == 0 ? o.a : 1 + rr(9);
        if (rr(3) == 0) {   /* p with 9 cities */
            int t, i;
            for (i = 0; i < 200 && S8(nat(o.a) + 0x51u) < 9; i++) {
                t = rr(157);
                city_add(o.a, t);
            }
        }
        break;
    case OP_213C0: o.a = 1 + rr(9); break;
    case OP_24EB8:
        o.a = 1 + rr(9);
        o.b = 1 + rr(9);
        o.c = rr(6) - 1;   /* -1..4: -1 is the room of a save whose Pirates hold 10 cities (--original) */
        break;
    case OP_252BC: o.a = 1 + rr(9); o.b = 1 + rr(9); o.c = rr(4); break;
    case OP_25548: o.a = 1 + rr(9); break;
    case OP_2E1D0:
        o.a = 1 + rr(9);
        o.b = rr(3) == 0 ? 0x19 : rr(2) ? 0x1E : rr(0x20);
        random_list(o.a, rr(10), rr(3));
        if (o.b == 0x19 && rr(2)) {   /* n's cities on its preferred sites, listed; the word past the list a city */
            int q, t, cnt = 0, r;
            city_remove_all(o.a);
            for (q = 0; q < 4; q++) {
                t = (int)S32(0xC0618u + 16u * (uint32_t)o.a + 4u * (uint32_t)q);
                if (t >= 0 && t < 157 && city_add(o.a, t))
                    setD32(0xF7CA0u + 4u * (uint32_t)cnt++, (uint32_t)t);
            }
            for (r = 0; r < 50; r++) {
                t = rr(157);
                if (city_add(o.a, t))
                    break;
            }
            setD32(0xF7C9Cu, (uint32_t)cnt);
            setD32(0xF7CA0u + 4u * (uint32_t)cnt, (uint32_t)(D8(nat(o.a) + 0x51u + (uint32_t)S8(nat(o.a) + 0x51u)) - 1));
            if (rr(2))
                clear_tokens(o.a);
        }
        break;
    case OP_1EF28:
        o.a = 1 + rr(9);
        o.sc[0] = rr(3); o.sc[1] = rr(3); o.sc[2] = rr(3);
        if (rr(2)) {
            int i;
            for (i = 0; i < 9; i++) {
                setD8(0xE3AF1u + (uint32_t)i, (uint8_t)(rr(3) ? 0 : rr(6)));
                setD8(0xE3B36u + (uint32_t)i, (uint8_t)(rr(3) ? 0 : rr(6)));
                setD8(0xE4055u + (uint32_t)i, (uint8_t)(rr(3) ? 0 : rr(6)));
                setD8(0xE4643u + (uint32_t)i, (uint8_t)(rr(3) ? 0 : rr(9)));
            }
            for (i = 0; i < 4; i++) {    /* cities at the four looked-at territory states (owners n, 10, 11, other) */
                static const int ts[4] = { 51, 126, 75, 77 };
                static const int own[4] = { 0, 10, 11, 3 };
                if (rr(2)) {
                    setD8(0xE2C1Au + 3u * (uint32_t)ts[i], (uint8_t)(D8(0xE2C1Au + 3u * (uint32_t)ts[i]) | 1u));
                    setD8(0xE2C18u + 3u * (uint32_t)ts[i], (uint8_t)(rr(4) == 0 ? o.a : own[rr(4)]));
                } else
                    setD8(0xE2C1Au + 3u * (uint32_t)ts[i], (uint8_t)(D8(0xE2C1Au + 3u * (uint32_t)ts[i]) & ~1u));
            }
            for (i = 0; i < 12; i++) {   /* the four looked-at area records */
                static const int ar[4] = { 52, 127, 76, 78 };
                setD8(area(ar[rr(4)]) + 0x0Fu + (uint32_t)rr(11), (uint8_t)(rr(2) ? 0 : rr(4)));
            }
            if (rr(2)) {                 /* sparse: 0..2 stacks in all four, so the sums 1 and 2 occur */
                static const int ar[4] = { 52, 127, 76, 78 };
                int a2, k2;
                for (a2 = 0; a2 < 4; a2++)
                    for (k2 = 0; k2 < 11; k2++)
                        setD8(area(ar[a2]) + 0x0Fu + (uint32_t)k2, 0);
                for (k2 = rr(3); k2 > 0; k2--)
                    setD8(area(ar[rr(4)]) + 0x0Fu + (uint32_t)rr(11), (uint8_t)(1 + rr(3)));
            }
        }
        break;
    case OP_20068: o.a = rr(10); setD32(0xF7C9Cu, (uint32_t)(rr(8) == 0 ? rr(3) : rr(14))); break;
    case OP_201B8:
        o.a = 1 + rr(9);
        random_list(o.a, rr(10), rr(4) == 0 ? rr(3) : 0);
        break;
    case OP_21E8C:
        o.a = 1 + rr(9);
        {
            int cnt = rr(4) == 0 ? 0 : rr(20), i, t;
            for (i = 0; i < cnt; i++) {
                t = rr(157);
                setD32(0xF7CA0u + 4u * (uint32_t)i, (uint32_t)t);
            }
            setD32(0xF7C9Cu, (uint32_t)cnt);
            if (rr(3) == 0) {   /* only cities */
                int c = 0;
                for (t = 0; t < 157 && c < 6; t++)
                    if (is_city(t) && D8(0xE2C18u + 3u * (uint32_t)t) != o.a && rr(3) == 0)
                        setD32(0xF7CA0u + 4u * (uint32_t)c++, (uint32_t)t);
                setD32(0xF7C9Cu, (uint32_t)c);
            }
        }
        break;
    default: break;
    }
    return o;
}

/* ---- part A: hand vectors --------------------------------------------------------------------------------------- */

static void set_pair(int i, int16_t id, int16_t w4, int16_t w6)
{
    setD16(0xF7300u + 8u * (uint32_t)i, (uint16_t)id);
    setD16(0xF7302u + 8u * (uint32_t)i, 0);
    setD16(0xF7304u + 8u * (uint32_t)i, (uint16_t)w4);
    setD16(0xF7306u + 8u * (uint32_t)i, (uint16_t)w6);
}


static void part_a(uint8_t *const *saves)
{
    Op o;
    int32_t r, sc[3];
    int i, t, n, coast[4], nc;
    char what[160];

    /* 0x20068: (id, +4, +6): r0 (1,0) r1 (2,5) r2 (2,3) r3 (0,9) -> r1 r2 r0 r3, the temp = r0 */
    memcpy(g_ds, saves[0], DS_SIZE);
    set_pair(0, 10, 1, 0); set_pair(1, 11, 2, 5); set_pair(2, 12, 2, 3); set_pair(3, 13, 0, 9);
    setD32(0xF7C9Cu, 4);
    f20068(0);
    check(S16(0xF7300u) == 11 && S16(0xF7308u) == 12 && S16(0xF7310u) == 10 && S16(0xF7318u) == 13
          && S16(0xF77D8u) == 10, "A 20068 order and temp");
    memcpy(g_ds, saves[0], DS_SIZE);
    set_pair(0, 10, 1, 0); set_pair(1, 11, 2, 5);
    setD16(0xF77D8u, 0x7777);
    setD32(0xF7C9Cu, 1);
    f20068(0);
    check(S16(0xF7300u) == 10 && S16(0xF77D8u) == 0x7777, "A 20068 one record untouched");

    /* 0x201B8 with no tokens of n anywhere: count 2 -> list[1] (no duplicates among the first 1), count 3 -> list[0],
       count 0 -> the word [0xF7C9C] (0) */
    memcpy(g_ds, saves[0], DS_SIZE);
    clear_tokens(3);
    setD32(0xF7CA0u, 40); setD32(0xF7CA4u, 41); setD32(0xF7CA8u, 42);
    setD32(0xF7C9Cu, 2);
    check(f201B8(3) == 41, "A 201B8 count 2");
    setD32(0xF7C9Cu, 3);
    check(f201B8(3) == 40, "A 201B8 count 3");
    setD32(0xF7C9Cu, 0);
    check(f201B8(3) == 0, "A 201B8 count 0 reads the count");

    /* 0x1EF28: no tokens / cities at the looked-at places */
    memcpy(g_ds, saves[0], DS_SIZE);
    for (n = 1; n <= 11; n++) {
        static const int ar[4] = { 52, 127, 76, 78 };
        for (i = 0; i < 4; i++)
            setD8(area(ar[i]) + 0x0Eu + (uint32_t)n, 0);
    }
    for (i = 0; i < 4; i++) {
        static const int ts[4] = { 51, 126, 75, 77 };
        setD8(0xE2C1Au + 3u * (uint32_t)ts[i], 0);
    }
    for (i = 0; i < 9; i++) {
        setD8(0xE3AF1u + (uint32_t)i, 0); setD8(0xE3B36u + (uint32_t)i, 0);
        setD8(0xE4055u + (uint32_t)i, 0); setD8(0xE4643u + (uint32_t)i, 0);
    }
    sc[0] = 5; sc[1] = 5; sc[2] = 5;
    check(f1EF28(5, sc) == 1 && f1EF28(4, sc) == 2, "A 1EF28 all equal");
    sc[2] = 3;
    check(f1EF28(4, sc) == 1, "A 1EF28 tie 0/1, nothing around");
    setD8(0xE3AF1u, 3);
    check(f1EF28(4, sc) == 0 && f1EF28(3, sc) == 1, "A 1EF28 tie 0/1, Rome's tokens (n == 3 skips all)");
    /* area record 76 (PALERMO) is also the first loop's record for the MILAZZO word: a stack of another nation there
       already makes the sum 2. n's own stack is skipped there and counted by the tie test (j left at 2) */
    setD8(0xE4643u + 3u, 5);
    check(f1EF28(4, sc) == 1, "A 1EF28 tie 0/1, Palermo's stack (n's own)");
    setD8(0xE4643u + 3u, 0);
    setD8(0xE4643u, 5);
    check(f1EF28(4, sc) == 1 && f1EF28(1, sc) == 1, "A 1EF28 Palermo's foreign stack: sum 2 first; n 1: tie test");
    setD8(0xE4643u, 0);
    sc[1] = 3; sc[2] = 5;
    check(f1EF28(5, sc) == 0 && f1EF28(4, sc) == 2, "A 1EF28 tie 0/2");
    sc[0] = 1; sc[1] = 2; sc[2] = 3;
    check(f1EF28(5, sc) == 1 && f1EF28(2, sc) == 2, "A 1EF28 no tie");
    setD8(area(52) + 0x0Eu + 2u, 1);   /* nation 2 in area record 52 (W. MEDITERRANEAN): sum 1 */
    check(f1EF28(4, sc) == 0 && f1EF28(2, sc) == 2, "A 1EF28 sum 1 (nation 2), own tokens skipped");
    setD8(area(52) + 0x0Eu + 2u, 0);
    setD8(area(76) + 0x0Eu + 1u, 1);   /* nation 1 in PALERMO: sum 2 */
    check(f1EF28(4, sc) == 1, "A 1EF28 sum 2");

    /* 0x2E1D0: Piracy picks the first of n's cities (list order) that is in the list; others list[0] */
    memcpy(g_ds, saves[0], DS_SIZE);
    for (n = 1; n <= 11; n++)
        city_remove_all(n);
    city_add(4, 20); city_add(4, 30); city_add(4, 40);
    setD32(0xF7CA0u, 40); setD32(0xF7CA4u, 30); setD32(0xF7C9Cu, 2);
    check(f2E1D0(4, 0x1E) == 30 && f2E1D0(4, 0x15) == 40, "A 2E1D0 Piracy / other");

    /* 0x25548: two coastal cities founded this round, no tokens of n: the lower limit first; then neither of this
       round: the other pass */
    memcpy(g_ds, saves[0], DS_SIZE);
    for (n = 1; n <= 11; n++)
        city_remove_all(n);
    clear_tokens(6);
    nc = 0;
    for (t = 0; t < 157 && nc < 4; t++)
        if (coastal(t))
            coast[nc++] = t;
    setD16(0xF40FCu, 5);
    setD8(0xE1639u + 35u * (uint32_t)coast[0], (uint8_t)((D8(0xE1639u + 35u * (uint32_t)coast[0]) & ~0x38u) | (3u << 3)));
    setD8(0xE1639u + 35u * (uint32_t)coast[1], (uint8_t)((D8(0xE1639u + 35u * (uint32_t)coast[1]) & ~0x38u) | (1u << 3)));
    city_add(6, coast[0]); city_add(6, coast[1]);
    setD8(0xE2C19u + 3u * (uint32_t)coast[0], 5); setD8(0xE2C19u + 3u * (uint32_t)coast[1], 5);
    snprintf(what, sizeof what, "A 25548 lower limit first (%d, %d)", coast[0], coast[1]);
    check(f25548(6) == coast[1], what);
    setD8(0xE2C19u + 3u * (uint32_t)coast[0], 4); setD8(0xE2C19u + 3u * (uint32_t)coast[1], 4);
    check(f25548(6) == coast[1], "A 25548 second pass");
    city_remove_all(6);
    check(f25548(6) == -1, "A 25548 none");

    /* 0x24EB8: the Pirates hold 8, v three coastal cities, room k = 1: fixed 9 Pirate cities, original 10 (+0x5B) */
    for (i = 0; i < 2; i++) {
        memcpy(g_ds, saves[0], DS_SIZE);
        for (n = 1; n <= 11; n++)
            city_remove_all(n);
        for (t = 0, nc = 0; t < 157 && S8(nat(10) + 0x51u) < 8; t++)
            if (!coastal(t))
                city_add(10, t);
        for (t = 0, nc = 0; t < 157 && nc < 3; t++)
            if (coastal(t) && city_add(7, t))
                nc++;
        g_original = i;
        g_trlen = 0;
        f24EB8(2, 7, 1);
        snprintf(what, sizeof what, "A 24EB8 room 1, g_original %d: Pirate cities %d", i, S8(nat(10) + 0x51u));
        check(S8(nat(10) + 0x51u) == (i ? 10 : 9) && S8(nat(7) + 0x51u) == (i ? 1 : 2), what);
        /* two cities, room 1 */
        memcpy(g_ds, saves[0], DS_SIZE);
        for (n = 1; n <= 11; n++)
            city_remove_all(n);
        for (t = 0; t < 157 && S8(nat(10) + 0x51u) < 8; t++)
            if (!coastal(t))
                city_add(10, t);
        for (t = 0, nc = 0; t < 157 && nc < 2; t++)
            if (coastal(t) && city_add(7, t))
                nc++;
        f24EB8(2, 7, 1);
        snprintf(what, sizeof what, "A 24EB8 two cities room 1, g_original %d", i);
        check(S8(nat(10) + 0x51u) == (i ? 10 : 9), what);
        g_original = 0;
    }

    /* 0x21E8C: every candidate a city, n with 9 cities: fixed nothing happens; --original converts territory 0 */
    for (i = 0; i < 2; i++) {
        memcpy(g_ds, saves[0], DS_SIZE);
        for (n = 1; n <= 11; n++)
            city_remove_all(n);
        for (t = 0; t < 157 && S8(nat(3) + 0x51u) < 9; t++)
            city_add(3, t);
        city_add(4, 100); city_add(4, 101);
        setD32(0xF7CA0u, 100); setD32(0xF7CA4u, 101); setD32(0xF7C9Cu, 2);
        g_original = i;
        memcpy(g_start, g_ds, DS_SIZE);
        g_trlen = 0;
        g_tr[0] = 0;
        f21E8C(3);
        snprintf(what, sizeof what, "A 21E8C no record, g_original %d", i);
        if (i == 0)
            check(memcmp(g_start, g_ds, DS_SIZE) == 0 && g_trlen == 0, what);
        else
            check(strstr(g_tr, "79270(-550,3,0,") != NULL, what);
        g_original = 0;
    }

    /* 0x20C70 / 0x213C0 against the model on hand-placed cities (the expected value is the model's: the adjacency
       makes a written-out value depend on the shipped lists) and the eliminate branch written out */
    memcpy(g_ds, saves[0], DS_SIZE);
    for (n = 1; n <= 11; n++)
        city_remove_all(n);
    for (t = 0; t < 157 && S8(nat(2) + 0x51u) < 9; t++)
        if (!coastal(t))
            city_add(2, t);
    city_add(5, 120); city_add(5, 121); city_add(5, 122);
    setD8(0xE1639u + 35u * 120u, (uint8_t)(D8(0xE1639u + 35u * 120u) | 0x40u));
    setD8(0xE1639u + 35u * 121u, (uint8_t)(D8(0xE1639u + 35u * 121u) & ~0x40u));
    setD8(0xE1639u + 35u * 122u, (uint8_t)(D8(0xE1639u + 35u * 122u) | 0x40u));
    setD8(nat(5) + 0x5Cu, 7); setD8(nat(2) + 0x5Cu, 3);
    check(f20C70(2, 5) == 122, "A 20C70 eliminate, ahead: first without 0x40");
    setD8(nat(5) + 0x5Cu, 2);
    check(f20C70(2, 5) == 123, "A 20C70 eliminate, behind: last with 0x40");
    memset(&o, 0, sizeof o);
    o.op = OP_213C0; o.a = 5;
    check(compare(&o, "A"), "A 213C0 hand state");
    r = f213C0(5);
    snprintf(what, sizeof what, "A 213C0 returns one of the cities (%d)", r);
    check(r == 121 || r == 122 || r == 123, what);
}

/* ---- part B: random differential runs ------------------------------------------------------------------------------ */

static long part_b(uint8_t *const *saves, int nsaves, int per, int quiet)
{
    int s, k, op, mode;
    long bad = 0;
    Op o;
    char what[120];

    g_rng = 777u;
    g_cvon = !quiet;
    for (op = 0; op < OP_N; op++)
        for (k = 0; k < per; k++)
            for (s = 0; s < nsaves; s++)
                for (mode = 0; mode < 2; mode++) {
                    random_state(saves[s]);
                    o = random_op(op);
                    g_original = mode;
                    if (!compare(&o, "B")) {
                        bad++;
                        if (!quiet) {
                            snprintf(what, sizeof what, "B %s save %d run %d mode %d", g_opname[op], s, k, mode);
                            check(0, what);
                        }
                    } else if (!quiet) {
                        g_tests++;
                        g_opcount[op]++;
                    }
                    g_original = 0;
                }
    g_cvon = 0;
    return bad;
}

#define NMUT 21

static void part_c(uint8_t *const *saves, int nsaves)
{
    int m;
    long bad;
    char what[80];

    for (m = 1; m <= NMUT; m++) {
        g_mut = m;
        bad = part_b(saves, nsaves, 30, 1);
        snprintf(what, sizeof what, "C mutation %d caught (%ld runs differ)", m, bad);
        check(bad > 0, what);
        if (bad == 0)
            printf("  mutation %d not caught\n", m);
    }
    g_mut = 0;
}

/* ---- loading ------------------------------------------------------------------------------------------------------ */

static uint8_t *g_live;

static int load_state(const char *gamedir, const char *path, uint8_t *out)
{
    int16_t ver;
    char desc[64];
    FILE *fp;
    int v;

    memcpy(g_ds, g_live, DS_SIZE);
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
    memcpy(out, g_ds, DS_SIZE);
    printf("loaded %s: \"%s\" version 0x%X, players %d, phase %d\n", path, desc, ver, S32(0xF40F0u), S8(0xF4102u));
    /* for the DOSBox-X check plan: who is a computer (+0x9A 2) or human (3), cities, Monotheism (+0x3A), the
       calamities held (+0x13 count, +0x14.. ids, +0x1C.. givers) */
    for (v = 1; v <= 9; v++) {
        int j, cnt = S8(nat(v) + 0x13u);
        if (D8(nat(v) + 0x9Au) == 0)
            continue;
        printf("   %d %-9s type %d cities %d mono %d stock %d board %d calamities", v, nname(v), D8(nat(v) + 0x9Au),
               S8(nat(v) + 0x51u), D8(nat(v) + 0x3Au), D8(nat(v) + 0x99u), D8(nat(v) + 0x98u));
        for (j = 0; j < cnt && j < 8; j++)
            printf(" %02X/%d", D8(nat(v) + 0x14u + (uint32_t)j), S8(nat(v) + 0x1Cu + (uint32_t)j));
        printf("\n");
    }
    return 0;
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:\\work\\advciv_run";
    const char *playdir = argc > 2 ? argv[2] : "C:\\work\\advciv_play";
    int per = argc > 3 ? atoi(argv[3]) : 150;
    static const char *const names[] = { "savgame0.dat", "savgame3.dat", "savgame4.dat", "savgame5.dat",
                                         "savgame6.dat", "savgame7.dat", "savgame8.dat", "savgame9.dat" };
    char path[600];
    uint8_t *saves[12];
    int nsaves = 0, i;
    long before;

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
    g_live = malloc(DS_SIZE);
    g_start = malloc(DS_SIZE);
    g_port = malloc(DS_SIZE);
    if (!g_live || !g_start || !g_port)
        return 2;
    memcpy(g_live, g_ds, DS_SIZE);
    snprintf(path, sizeof path, "%s\\SAVGAMEA.DAT", playdir);
    saves[nsaves] = malloc(DS_SIZE);
    if (saves[nsaves] && load_state(playdir, path, saves[nsaves]) == 0)
        nsaves++;
    for (i = 0; i < (int)(sizeof names / sizeof names[0]); i++) {
        snprintf(path, sizeof path, "%s\\%s", gamedir, names[i]);
        saves[nsaves] = malloc(DS_SIZE);
        if (saves[nsaves] && load_state(gamedir, path, saves[nsaves]) == 0)
            nsaves++;
    }
    if (nsaves == 0)
        return 2;

    before = g_fail;
    part_a(saves);
    printf("part A: %ld failed\n", g_fail - before);
    before = g_fail;
    part_b(saves, nsaves, per, 0);
    printf("part B: %ld failed; runs per routine:", g_fail - before);
    for (i = 0; i < OP_N; i++)
        printf(" %s %ld", g_opname[i], g_opcount[i]);
    printf("\n");
    for (i = 0; i < CV_N; i++) {
        printf("  %-30s %6ld%s", g_cvname[i], g_cv[i], i % 3 == 2 ? "\n" : "");
        check(g_cv[i] > 0, g_cvname[i]);
    }
    printf("\n");
    before = g_fail;
    part_c(saves, nsaves);
    printf("part C: %ld of %d mutations not caught\n", g_fail - before, NMUT);
    printf("%ld checks, %ld failed, game_rand calls %ld\n", g_tests, g_fail, g_rand);
    return g_fail == 0 ? 0 : 1;
}
