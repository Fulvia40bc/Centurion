/* aical2_test [GAMEDIR [PLAYDIR]]: unit checks for module AICAL2 (aical2.c, the computers' side of Famine, Epidemic,
   Flood and Civil War) that need no game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init; default C:\work\advciv_run, only read), TERRFLAG.DAT (f12394:
   the population limits and flood-plain bits are not in the save) and the R4 saves savgame0.dat (start of MOVEMENT),
   savgame3.dat..savgame9.dat (R4 at TRADE / calamities, edited); PLAYDIR holds SAVGAMEA.DAT (the R4 autosave: seven
   computers; default C:\work\advciv_play, only read). The saves are read with saveload.c's f5F05C as the game does.

   aical2.c is built with -DAICAL2_TEST: its two calls into AICAL1 (0x213C0, 0x20C70) come to recorders here, which
   trace the call and answer one of the asked nation's cities, chosen by the check's seed (so a call out of order or
   with other arguments changes everything after it). The real primitives are linked: rules.c (f78F04, f78E74, f78FEC)
   and move.c (f57CE8, f57E28). Everything else aical2.c calls is a stub below that appends one line to a trace (the
   drawing included); those with data effects apply them: cal1.c's choice list 0x28CFC (its rules written out again
   here), the border bytes 0xFFB60 (f3F670 / f3F624), the current dialog (f3B05C). f7BA64 (unattended?) answers from
   the seed. cal2.c's cal2_river_text is written out again here (the "TIGRUS" fix).

   The model is a second implementation of the nine routines, written here from the listing (not from aical2.c), in
   the listing's own terms: fields are read at the listing's displacements (dword [a] sar 24 is the byte a + 3, word
   reads sar 16), the texts are the binary's own strings at their addresses (snprintf with the data object's format),
   and the BUG(1995) fixes are implemented separately (0x260C4's per-territory bookkeeping is an array indexed by
   territory + 1, not a search of the plans). Each check runs the port on a copy of the start state, then the model
   on another copy, and compares the whole data object byte for byte, the traces line for line, the return value and
   the text buffer 0x2C894 writes into. A run that makes more than 30,000 stub calls is cut off ("CUT") identically on
   both sides.
     A. hand vectors with the expected values written out: 0x27518's ranking and shares; 0x2E45C's own rivers (Iberia
        by the player count, the text fixes and --original); 0x27850's hand test on the advance bytes (+0x24 + i) and
        its three outcomes; 0x260C4's over-planning example of the file comment (--original: a count of -2; fixed: 0)
        and the hang with only cities left (fixed: it ends); 0x2C298's Iberia rows; 0x2C894's faction counts (fixed /
        --original) and the empty list; 0x2D1C0 with a stale count (fixed: it ends).
     B. random differential checks: each routine called with random arguments on random states built from the real
        saves (tokens and cities moved about with the real primitives, factions 12 / 13 filled, AI tables, marks,
        flags, the plain units, ranking tables), with and without --original. --original runs avoid the states where
        the original hangs (0x260C4 with no token left to plan, 0x2D1C0 with k over o's tokens): the port keeps those
        hangs literally there. Default runs check, besides the model, that 0x260C4 never leaves a negative count.
     C. the real saves: every routine for every nation in play from the save's own state (the edited savgame3..9 are
        all-human; the routines do not test +0x9A) (Famine 10 and the
        secondary 8, Epidemic 16, Flood on each plain with the plain's units as Flood computes them; the secondary
        assignment as Famine sets it up; the river choice over all five plains; a Civil War faction and its annexing),
        port against model, with a few results printed for the DOSBox-X check plan.
     D. mutations: seventeen deliberately broken copies of the model must each be caught by part B's checks.
   Prints each failure (the first differing trace line or byte), the counts per part, and exits 0 when all pass.

   Build (from the repo root; artefacts outside the repo):
     clang -std=c11 -Wall -Wextra -DAICAL2_TEST -Iport/src port/test/aical2_test.c port/src/aical2.c port/src/rules.c
       port/src/move.c port/src/saveload.c port/src/load.c port/src/state.c port/src/le_image.c -o aical2_test.exe */

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <direct.h>
#define chdir _chdir
#define getcwd _getcwd

#include "aical2.h"
#include "cal1.h"
#include "cal2.h"
#include "cal4.h"
#include "fg.h"
#include "game.h"
#include "kit.h"
#include "map.h"
#include "mapdata.h"
#include "move.h"
#include "panel.h"
#include "rules.h"
#include "saveload.h"
#include "state.h"
#include "ui.h"

void f12394(void);   /* load.c: TERRFLAG.DAT into the territory records */
int  aical2_test_f213C0(int n);
int  aical2_test_f20C70(int p, int v);

/* ---- byte access on the current data object (g_ds: the port's or the model's copy) ---------------------------- */

static int      rb(uint32_t a) { return g_ds[a - DS_BASE]; }
static int      sb(uint32_t a) { return (int8_t)g_ds[a - DS_BASE]; }
static int      rw(uint32_t a) { return g_ds[a - DS_BASE] | g_ds[a + 1 - DS_BASE] << 8; }
static int32_t  d32(uint32_t a)
{
    return (int32_t)((uint32_t)g_ds[a - DS_BASE] | (uint32_t)g_ds[a + 1 - DS_BASE] << 8 |
                     (uint32_t)g_ds[a + 2 - DS_BASE] << 16 | (uint32_t)g_ds[a + 3 - DS_BASE] << 24);
}
static int32_t  sar24(uint32_t a) { return (int8_t)g_ds[a + 3 - DS_BASE]; }       /* dword [a] sar 24 */
static int32_t  sar16(uint32_t a) { return (int16_t)rw(a + 2); }                   /* dword [a] sar 16 */
static void w8(uint32_t a, int v) { g_ds[a - DS_BASE] = (uint8_t)v; }
static void w16(uint32_t a, int v) { g_ds[a - DS_BASE] = (uint8_t)v; g_ds[a + 1 - DS_BASE] = (uint8_t)(v >> 8); }
static void w32(uint32_t a, int32_t v)
{
    uint32_t u = (uint32_t)v;
    g_ds[a - DS_BASE] = (uint8_t)u; g_ds[a + 1 - DS_BASE] = (uint8_t)(u >> 8);
    g_ds[a + 2 - DS_BASE] = (uint8_t)(u >> 16); g_ds[a + 3 - DS_BASE] = (uint8_t)(u >> 24);
}
static const char *str(uint32_t a) { return (const char *)(g_ds + (a - DS_BASE)); }
static const char *nm(int n) { return str((uint32_t)d32(0xC768Cu + 4u * (uint32_t)n)); }
static const char *tn(int t) { return str((uint32_t)d32(0xC77D8u + 4u * (uint32_t)t)); }

#define NAT(n)   (0xF14CCu + 199u * (uint32_t)(n))
#define AR(r)    (0xE31B8u + 69u * (uint32_t)(r))
#define TS(t)    (0xE2C18u + 3u * (uint32_t)(t))
#define TR(t)    (0xE1638u + 35u * (uint32_t)(t))
#define LIST     0xF7CA0u
#define LIST_N   0xF7C9Cu
#define BORDERS  0xFFB60u
#define LIM(t)   (((rb(TR(t) + 1u) << 2) & 0xFF) >> 5)

/* ---- traces, checks ------------------------------------------------------------------------------------------ */

#define LOGSZ (1u << 21)
static char *g_logs[2], *g_log;
static size_t g_lens[2], *g_len;
static int g_tests, g_fail, g_verbose = 1, g_quiet;
static int g_mut;              /* part D: the model's mutation (0 = none) */
static long g_cov[40];

static void tr(const char *fmt, ...)
{
    va_list ap;
    int k;
    if (*g_len + 700 >= LOGSZ)
        return;
    va_start(ap, fmt);
    k = vsnprintf(g_log + *g_len, LOGSZ - *g_len, fmt, ap);
    va_end(ap);
    if (k > 0)
        *g_len += (size_t)k;
    g_log[(*g_len)++] = '\n';
    g_log[*g_len] = 0;
}

static int check(int ok, const char *what)
{
    g_tests++;
    if (!ok) {
        g_fail++;
        if (g_verbose && !g_quiet)
            printf("FAIL %s\n", what);
    }
    return ok;
}

/* ---- the seeded script, the cut --------------------------------------------------------------------------------- */

static uint32_t g_srng, g_rng = 0x1234567u;
static int g_steps, g_cuts, g_np;
static jmp_buf g_jb;
static int g_ba = -1;          /* f7BA64's answer (-1: the script) */

static uint32_t srnd(uint32_t n)   /* the check's script (same seed for the port and the model) */
{
    g_srng ^= g_srng << 13;
    g_srng ^= g_srng >> 17;
    g_srng ^= g_srng << 5;
    return n ? g_srng % n : 0;
}

static uint32_t rnd(uint32_t n)    /* the generator of states and arguments */
{
    g_rng ^= g_rng << 13;
    g_rng ^= g_rng >> 17;
    g_rng ^= g_rng << 5;
    return n ? g_rng % n : 0;
}

static void step(void)
{
    if (++g_steps > 30000) {
        tr("CUT");
        g_cuts++;
        longjmp(g_jb, 1);
    }
}

/* ---- stubs (both sides) --------------------------------------------------------------------------------------- */

_Noreturn void not_ported(uint32_t addr)
{
    tr("NOT PORTED %X", (unsigned)addr);
    g_np++;
    longjmp(g_jb, 1);
}

int f78AE4(void)
{
    tr("UNEXPECTED rand");
    return 0;
}

static int city_of(int t, int n) { return (rb(TS(t) + 2u) & 1) != 0 && rb(TS(t)) == n; }

static void choice_add(int t)
{
    int c = d32(LIST_N);
    w32(LIST + 4u * (uint32_t)c, t);
    w32(LIST_N, c + 1);
}

void f28CFC(int n, int kind, int s)   /* cal1.c's rules, written out again */
{
    int t, i;
    uint32_t a;
    step();
    w32(LIST_N, 0);
    for (i = 0; i < 0x9D; i++)
        w32(LIST + 4u * (uint32_t)i, -1);
    if (kind == 0x15 || kind == 4 || kind == 9) {
        for (t = 0; t < 0x9D; t++)
            if (city_of(t, n) || rb(AR(t + 1) + 0x0Eu + (uint32_t)n) != 0)
                choice_add(t);
    } else if (kind == 0x1B) {
        for (t = 0; t < 0x9D; t++)
            if (city_of(t, n) || sb(AR(t + 1) + 0x0Eu + (uint32_t)n) > 1)
                choice_add(t);
    } else if (kind == 7 || kind == 8) {
        for (t = 0; t < 0x9D; t++)
            if (rb((kind == 7 ? 0xF7984u : 0xF7A24u) + (uint32_t)t) != 0)
                choice_add(t);
    } else if (kind == 6) {
        for (t = 0; t < 0x9D; t++)
            if (rb(AR(t + 1) + 0x0Eu + (uint32_t)n) != 0)
                choice_add(t);
    } else if (kind == 2 || kind == 5) {
        for (t = 0; t < 0x9D; t++)
            if (city_of(t, n))
                choice_add(t);
    } else if (kind == 0x19) {
        a = 0xBEE08u + 22u * (uint32_t)s;
        for (i = 0; (int16_t)rw(a + 2u) > i; i++) {
            t = (int16_t)rw(a + 8u + 2u * (uint32_t)i);
            if ((city_of(t, n) && (rb(TR(t) + 1u) & 0x80) != 0) || rb(AR(t + 1) + 0x0Eu + (uint32_t)n) != 0)
                choice_add(t);
        }
    }
    tr("list %d %d %d -> %d", n, kind, s, d32(LIST_N));
}

static int pick_city(int n)
{
    int c = sb(NAT(n) + 0x51u);
    if (c <= 0 || c > 9)
        return 0;
    return rb(NAT(n) + 0x52u + srnd((uint32_t)c));
}

int aical2_test_f213C0(int n) { int r; step(); r = pick_city(n); tr("213C0 %d -> %d", n, r); return r; }
int aical2_test_f20C70(int p, int v) { int r; step(); r = pick_city(v); tr("20C70 %d %d -> %d", p, v, r); return r; }

void f79270(int wait, int n, int t, const char *text, int anim) { step(); tr("msg %d %d %d %s %d", wait, n, t, text, anim); }
void f78A20(int t) { step(); tr("wait %d", t); }
void f3CD6C(int8_t pic) { step(); tr("bg %d", pic); }
int  fg_vbcopy(int a, int b, int c, int d, int e, int f, int s, int dd) { step(); tr("vbcopy %d %d %d %d %d %d %d %d", a, b, c, d, e, f, s, dd); return 0; }
int  fg_vbopen(int h) { step(); tr("vbopen %d", h); return 0; }
void fg_vbclose(void) { step(); tr("vbclose"); }
int  fg_setcolor(int c) { step(); tr("color %d", c); return 0; }
void f3A3A0(int x, int y, const char *s) { step(); tr("right %d %d %s", x, y, s); }
void f3A044(int x, int y, const char *s) { step(); tr("print %d %d %s", x, y, s); }
void f3A0B0(int x, int y, int w, const char *s) { step(); tr("text %d %d %d %s", x, y, w, s); }
void f3A9F4(const char *s) { step(); tr("strip %s", s); }
void f3AB7C(const char *s) { step(); tr("box %s", s); }
void f7876C(void) { step(); tr("phase"); }
void f3CBBC(void) { step(); tr("paste"); }
void f3B05C(uint32_t dlg) { step(); tr("dialog %X", (unsigned)dlg); w32(0xF8E78u, (int32_t)dlg); }
void f459E4(int n) { step(); tr("panel %d", n); }
void f3D9FC(int n) { step(); tr("home %d", n); }
void f3DA60(int t) { step(); tr("centre %d", t); }
void f3D52C(int dx, int dy) { step(); tr("scroll %d %d", dx, dy); }
void f3F670(void) { step(); tr("clrb"); memset(g_ds + (BORDERS - DS_BASE), 0, 0xA0); }
void f3F624(int t, int c) { step(); tr("border %d %d", t, c); w8(BORDERS + (uint32_t)t, c); }
void f3C9D0(int x, int y, int w, int h, int f) { step(); tr("frame %d %d %d %d %d", x, y, w, h, f); }
void f3C940(uint32_t dlg) { step(); tr("disable %X", (unsigned)dlg); }
void f3C8C4(uint32_t dlg) { step(); tr("enable %X", (unsigned)dlg); }
void f2EE6C(int n, int t, int left) { step(); tr("auto %d %d %d", n, t, left); }
void f291D0(int a, int b, const char *text)
{
    int m;
    char row[80];
    size_t k = 0;
    step();
    for (m = 0; m < 14; m++)
        k += (size_t)snprintf(row + k, sizeof row - k, " %d", rb(0xF547Cu + (uint32_t)m));
    tr("shown %d %d %s:%s", a, b, text, row);
}
int f7BA64(void)
{
    int r;
    step();
    r = g_ba >= 0 ? g_ba : (int)srnd(2);
    tr("unattended %d", r);
    return r;
}
const char *cal2_river_text(const char *r)   /* cal2.c's, written out again */
{
    if (!g_original && strcmp(r, "TIGRUS-EUPHRATES") == 0)
        return "TIGRIS-EUPHRATES";
    return r;
}

/* move.c's other callees (its dispatcher cases; never reached from the primitives) */
static _Noreturn void unexpected(const char *what)
{
    printf("unexpected call of %s\n", what);
    exit(3);
}
void f3D18C(int t, int c) { (void)t; (void)c; unexpected("f3D18C"); }
void f5D14C(int n, int i) { (void)n; (void)i; unexpected("f5D14C"); }
void f3A780(const char *msg) { (void)msg; unexpected("f3A780"); }
void f3B7CC(uint32_t dlg, int id) { (void)dlg; (void)id; unexpected("f3B7CC"); }
void f3B870(uint32_t dlg, int id) { (void)dlg; (void)id; unexpected("f3B870"); }
int  f3B9A4(uint32_t dlg, int id) { (void)dlg; (void)id; unexpected("f3B9A4"); }
void f3BFBC(uint32_t dlg, int id, int en) { (void)dlg; (void)id; (void)en; unexpected("f3BFBC"); }
void f3C3D0(uint32_t dlg, int id) { (void)dlg; (void)id; unexpected("f3C3D0"); }
int  f3E89C(int t) { (void)t; unexpected("f3E89C"); }
int  f3E8FC(int t) { (void)t; unexpected("f3E8FC"); }
void f3F698(int t, int c) { (void)t; (void)c; unexpected("f3F698"); }
void f3F70C(int t, int c) { (void)t; (void)c; unexpected("f3F70C"); }
void f426D0(int i) { (void)i; unexpected("f426D0"); }
_Noreturn void f44CAC(void) { unexpected("f44CAC"); }
void f572CC(int t) { (void)t; unexpected("f572CC"); }
void f76F70(int n, uint8_t *cards) { (void)n; (void)cards; unexpected("f76F70"); }

/* ---- the model: the nine routines from the listing -------------------------------------------------------------- */

static int m_wait1(void)          /* 0x26662..0x266C9 and its twins */
{
    if (rb(0xF0980u) & 4)
        return -500;
    if (f7BA64() == 0)
        return 0x7D0;
    return (int16_t)rw(0xF40BAu) == 0 ? 0x1F4 : 0x5DC;
}

static int m_wait2(void)          /* 0x2C9FE..0x2CA1D */
{
    if (f7BA64() == 0)
        return 0x5DC;
    if (rw(0xF40BAu) != 0)
        return 0x5DC;
    return 0x1F4;
}

#define MPL 400
static int16_t mT[MPL], mA[MPL];
static int32_t m_c2, m_pl[400];

static void m_add(int t1, int a)
{
    if (m_c2 >= MPL)
        return;
    mT[m_c2] = (int16_t)t1;
    mA[m_c2] = (int16_t)a;
    m_c2++;
    if (t1 >= 0 && t1 < 400)
        m_pl[t1] += a;
    if (m_c2 > 160)
        g_cov[39]++;   /* never expected (the port stores at most 160) */
}

static void m260C4(int n, int L, int kind, int s)
{
    char cal[28], buf[100];
    uint32_t nr = 199u * (uint32_t)n, ts = 22u * (uint32_t)s;
    int32_t era, cities, tokens, terr, R, per, tmp, q, r, t, i, j, c, cap, lim, tk, a, d, round, e, room;
    int fix = !g_original;

    m_c2 = 0;
    memset(m_pl, 0, sizeof m_pl);
    f28CFC(n, kind, s);
    cal[0] = 0;
    if (kind == 0x15)
        snprintf(cal, sizeof cal, "%s", str(0xB2AA4u));
    else if (kind == 0x1B)
        snprintf(cal, sizeof cal, "%s", str(0xB2AABu));
    else if (kind == 0x19)
        snprintf(cal, sizeof cal, "%s", str(0xB2AB4u));
    snprintf(buf, sizeof buf, str(0xB2ABAu), cal, nm(n));
    f79270(0x5DC, n, -200, buf, 1);
    era = (int16_t)rw(0xBCFDAu + 68u * (uint32_t)(n - 1) + 4u * (uint32_t)sar24(0xF1525u + nr));
    cities = (int8_t)rb(0xF151Du + nr);
    tokens = (int8_t)rb(0xF1564u + nr);
    terr = (int8_t)rb(0xF152Cu + nr);
    for (i = 0; i < MPL; i++)
        mT[i] = mA[i] = 0;
    R = 0;
    if (kind == 0x15) {
        if (L != 10)
            goto L_262A8;
        if (era <= 1 && cities >= (g_mut == 1 ? 4 : 3)) { R = 1; goto L_263CC; }
        if (era == 2 && cities >= 4) { R = 1; goto L_263CC; }
        if (era == 3 && cities >= 5) { R = 1; goto L_263CC; }
        if (era >= 4 && cities >= 6) { R = 1; goto L_263CC; }
        R = tokens - 10 < cities + cities;
        goto L_263CC;
    L_262A8:
        R = (tokens - terr < L && cities > 0) ? 1 : 0;
        goto L_263CC;
    }
    if (kind == 0x1B) {
        if (L < 10)
            goto L_263A5;
        if (era <= 1 && cities >= 4) { R = 2; goto L_263CC; }
        if (era == 2 && cities >= 5) { R = 2; goto L_263CC; }
        if (era == 3 && cities >= 6) { R = 2; goto L_263CC; }
        if (era >= 4 && cities >= 7) { R = 2; goto L_263CC; }
        if (tokens - 16 < cities + cities) { R = 1; goto L_263CC; }
        if (tokens - 16 < cities + cities - 2) { R = 2; goto L_263CC; }
        R = 0;
        goto L_263CC;
    L_263A5:
        R = (tokens - terr < L && cities > 0) ? 1 : 0;
    }
L_263CC:
    per = kind == 0x15 ? 5 : kind == 0x1B ? (g_mut == 2 ? 5 : 4) : 1;
    tmp = (int16_t)(uint16_t)((uint32_t)(R * per) + (uint32_t)(tokens - terr));
    if (tmp < L) {
        q = (L - tmp) / per;
        r = (L - tmp) % per;
        R += q + (r > 0 ? 1 : 0);
        if ((int16_t)R > (int16_t)cities)
            R = cities;
    }
    if (cities <= 0 || (int16_t)R <= 0 || kind == 0x19)
        goto L_26767;
L_26498:
    if ((int16_t)R == 0)
        goto L_26C10;
    g_cov[0]++;
    t = aical2_test_f213C0((int8_t)n) - 1;
    snprintf(buf, sizeof buf, str(0xB2AE3u), cal, nm(n), tn(t));
    f79270(-550, n, t, buf, 1);
    f78A20(500);
    if (kind == 0x15) {
        f78FEC(t, n);
        L -= 5 - sar24(0xE31C3u + 69u * (uint32_t)(t + 1) + (uint32_t)n);
        while (sar24(0xE31C3u + 69u * (uint32_t)(t + 1) + (uint32_t)n) > 0 && L > 0) {
            f57CE8(t, n, 0, 1);
            L--;
            w8(0xF1564u + nr, rb(0xF1564u + nr) - 1);
            w8(0xF1565u + nr, rb(0xF1565u + nr) + 1);
        }
    } else if (kind == 0x1B) {
        f78FEC(t, n);
        L -= 5 - sar24(0xE31C3u + 69u * (uint32_t)(t + 1) + (uint32_t)n);
        while (sar24(0xE31C3u + 69u * (uint32_t)(t + 1) + (uint32_t)n) > 1 && L > 0) {
            f57CE8(t, n, 0, 1);
            L--;
            w8(0xF1564u + nr, rb(0xF1564u + nr) - 1);
            w8(0xF1565u + nr, rb(0xF1565u + nr) + 1);
        }
    }
    snprintf(buf, sizeof buf, str(0xB2B01u), cal, nm(n), tn(t));
    f79270(m_wait1(), n, t, buf, 1);
    for (j = 0; j < d32(LIST_N); j++) {
        e = d32(LIST + 4u * (uint32_t)j);
        if (e == t) {
            w32(LIST + 4u * (uint32_t)j, d32(0xF7CA4u + 4u * (uint32_t)j));
            w32(0xF7CA4u + 4u * (uint32_t)j, -1);
        } else if (g_mut != 3 && e == -1 && d32(0xF7CA4u + 4u * (uint32_t)j) > 0) {
            w32(LIST + 4u * (uint32_t)j, d32(0xF7CA4u + 4u * (uint32_t)j));
            w32(0xF7CA4u + 4u * (uint32_t)j, e);
        }
    }
    w32(LIST_N, d32(LIST_N) - 1);
    R--;
    goto L_26498;

L_26767:
    if (L <= 0 || kind != 0x19)
        goto L_26C10;
    g_cov[1]++;
    c = 0;
    for (i = 0; sar16(0xBEE08u + ts) > i; i++) {
        e = sar16(0xBEE0Eu + ts + 2u * (uint32_t)i);
        if ((rb(0xE2C1Au + 3u * (uint32_t)e) & 1) == 0)
            continue;
        if (rb(0xE2C18u + 3u * (uint32_t)e) != n)
            continue;
        if ((rb(0xE1639u + 35u * (uint32_t)e) & 0x80) == 0)
            continue;
        c++;
    }
    cap = (int16_t)(uint16_t)((uint32_t)rw((g_mut == 4 ? 0xBEE0Au : 0xBEE08u) + ts) - 5u * (uint32_t)c);
    for (i = 0; sar16(0xBEE08u + ts) > i; i++) {
        e = sar16(0xBEE0Eu + ts + 2u * (uint32_t)i);
        if ((rb(0xE2C1Au + 3u * (uint32_t)e) & 1) == 0 || rb(0xE2C18u + 3u * (uint32_t)e) != n
            || (rb(0xE1639u + 35u * (uint32_t)e) & 0x80) == 0)
            continue;
        g_cov[2]++;
        L -= 5;
        f3F670();
        f3F624(e, 15);
        f3DA60(e);
        f78A20(500);
        f78F04(e, n);
        snprintf(buf, sizeof buf, str(0xB2B1Fu), cal, nm(n), tn(e));
        f79270(m_wait1(), n, e, buf, 1);
        if (L < 1)
            break;
        if ((int16_t)cap < L)
            continue;
        break;
    }
    if (L <= 0)
        goto L_26C10;
    for (i = 0; sar16(0xBEE08u + ts) > i; i++) {
        e = sar16(0xBEE0Eu + ts + 2u * (uint32_t)i);
        L -= sar24(0xE31C3u + 69u * (uint32_t)(e + 1) + (uint32_t)n);
        m_add((int16_t)(rw(0xBEE10u + ts + 2u * (uint32_t)i) + 1), (int8_t)rb(0xE31C6u + 69u * (uint32_t)(e + 1) + (uint32_t)n));
        if (L >= 1)
            continue;
        break;
    }

L_26C10:
    if (kind == 0x15) {
        if (d32(0xF4204u + 4u * (uint32_t)n) != 0)
            goto L_26F95;
        g_cov[3]++;
    L_26C2E:
        if (L <= 0)
            goto L_271B2;
        round = m_c2;
        for (i = 0; i < d32(LIST_N); i++) {
            t = d32(LIST + 4u * (uint32_t)i);
            lim = LIM(t);
            tk = (int8_t)rb(0xE31C6u + 69u * (uint32_t)(t + 1) + (uint32_t)n);
            f3F624(t, 15);
            if (lim <= 2 || tk < lim)
                continue;
            if (tk < L) {
                a = lim - (g_mut == 5 ? 1 : 2);
                if (fix) {
                    room = tk - m_pl[t + 1];
                    if (a > room) a = room;
                    if (a <= 0) continue;
                }
                m_add(t + 1, a);
                L -= a;
                continue;
            }
            a = L;
            if (fix) {
                room = tk - m_pl[t + 1];
                if (a > room) a = room;
                if (a <= 0) continue;
            }
            m_add(t + 1, a);
            L -= a;
            if (L == 0)
                break;
        }
        if (L <= 0)
            goto L_26EC9;
        for (i = 0; i < d32(LIST_N); i++) {
            t = d32(LIST + 4u * (uint32_t)i);
            lim = LIM(t);
            tk = (int8_t)rb(0xE31C6u + 69u * (uint32_t)(t + 1) + (uint32_t)n);
            if (lim != 2 || tk < lim)
                continue;
            d = tk - lim;
            if (d < L && d != 0)
                a = d;
            else if (d >= L)
                a = L;
            else if (tk < 1)
                continue;
            else
                a = 1;
            if (fix) {
                room = tk - m_pl[t + 1];
                if (a > room) a = room;
                if (a <= 0) continue;
            }
            m_add(t + 1, a);
            L -= a;
            if (L == 0)
                break;
        }
    L_26EC9:
        if (L <= 0)
            goto L_271B2;
        for (i = 0; i < d32(LIST_N); i++) {
            t = d32(LIST + 4u * (uint32_t)i);
            tk = (int8_t)rb(0xE31C6u + 69u * (uint32_t)(t + 1) + (uint32_t)n);
            if (tk <= 0)
                continue;
            if (fix && tk - m_pl[t + 1] < 1)
                continue;
            m_add(t + 1, 1);
            L--;
            if (L != 0)
                continue;
            goto L_271B2;
        }
        if (fix && m_c2 == round) {
            g_cov[4]++;
            goto L_271B2;
        }
        if (m_c2 > round + 0 && round > 0)
            g_cov[5]++;   /* a second round */
        goto L_26C2E;
    L_26F95:
        g_cov[6]++;
        for (i = 1; i < d32(LIST_N); i++)
            for (j = d32(LIST_N) - 1; j >= i; j--) {
                int va = (int8_t)rb(0xEED84u + 160u * (uint32_t)n + (uint32_t)d32(0xF7C9Cu + 4u * (uint32_t)j));
                int vb = (int8_t)rb(0xEED84u + 160u * (uint32_t)n + (uint32_t)d32(LIST + 4u * (uint32_t)j));
                if (g_mut == 6 ? va < vb : va > vb) {
                    e = d32(0xF7C9Cu + 4u * (uint32_t)j);
                    w32(0xF7C9Cu + 4u * (uint32_t)j, d32(LIST + 4u * (uint32_t)j));
                    w32(LIST + 4u * (uint32_t)j, e);
                }
            }
        for (;;) {
            if (L <= 0)
                goto L_271B2;
            round = m_c2;
            for (i = 0; i < d32(LIST_N); i++) {
                t = d32(LIST + 4u * (uint32_t)i);
                tk = (int8_t)rb(0xE31C6u + 69u * (uint32_t)(t + 1) + (uint32_t)n);
                if (fix)
                    tk -= m_pl[t + 1];
                while ((int16_t)tk > 0 && L > 0) {
                    m_add(t + 1, 1);
                    L--;
                    tk--;
                }
            }
            if (fix && m_c2 == round) {
                g_cov[7]++;
                goto L_271B2;
            }
        }
    }
    if (kind == 0x1B) {
        if (d32(0xF4204u + 4u * (uint32_t)n) != 0)
            goto L_271BC;
        g_cov[8]++;
        if (L <= 0)
            goto L_271B2;
        for (i = sar24(0xF1529u + nr) - 1; i >= 0; i--) {
            e = rb(0xF152Du + nr + (uint32_t)i);
            tk = (int8_t)rb(0xE31C6u + 69u * (uint32_t)e + (uint32_t)n);
            if (tk <= 1)
                continue;
            a = tk - 1 > L ? L : tk - 1;
            m_add(e, a);
            L -= a;
            if (L != 0)
                continue;
            break;
        }
        goto L_271B2;
    L_271BC:
        g_cov[9]++;
        for (i = 1; i < d32(LIST_N); i++)
            for (j = d32(LIST_N) - 1; j >= i; j--) {
                int va = (int8_t)rb(0xEED84u + 160u * (uint32_t)n + (uint32_t)d32(0xF7C9Cu + 4u * (uint32_t)j));
                int vb = (int8_t)rb(0xEED84u + 160u * (uint32_t)n + (uint32_t)d32(LIST + 4u * (uint32_t)j));
                if (va > vb) {
                    e = d32(0xF7C9Cu + 4u * (uint32_t)j);
                    w32(0xF7C9Cu + 4u * (uint32_t)j, d32(LIST + 4u * (uint32_t)j));
                    w32(LIST + 4u * (uint32_t)j, e);
                }
            }
        for (;;) {
            if (L <= 0)
                goto L_271B2;
            round = m_c2;
            for (i = 0; i < d32(LIST_N); i++) {
                t = d32(LIST + 4u * (uint32_t)i);
                tk = (int8_t)rb(0xE31C6u + 69u * (uint32_t)(t + 1) + (uint32_t)n);
                if (fix)
                    tk -= m_pl[t + 1];
                while ((int16_t)tk > 1 && L > 0) {
                    m_add(t + 1, 1);
                    L--;
                    tk -= 1;
                }
            }
            if (fix && m_c2 == round) {
                g_cov[10]++;
                goto L_271B2;
            }
        }
    }

L_271B2:
    for (i = 0; i < m_c2; i++) {
        if (mT[i] == -1)
            continue;
        for (j = i + 1; j < m_c2; j++)
            if (mT[j] == mT[i]) {
                mA[i] = (int16_t)(mA[i] + mA[j]);
                mA[j] = 0;
                mT[j] = -1;
            }
    }
    for (i = 0; i < m_c2; i++) {
        if (mA[i] <= 0)
            continue;
        snprintf(buf, sizeof buf, str(0xB2B3Du), cal, nm(n), (int)mA[i], str((uint32_t)d32(0xC77D4u + 4u * (uint32_t)(int32_t)mT[i])));
        f79270(-550, n, mT[i] - 1, buf, 1);
        f78A20(500);
        f57CE8(mT[i] - 1, n, 0, mA[i]);
        w8(0xF1564u + nr, (uint8_t)(rb(0xF1564u + nr) - (uint8_t)mA[i]));
        if (g_mut != 7)
            w8(0xF1565u + nr, (uint8_t)(rb(0xF1565u + nr) + (uint8_t)mA[i]));
        f79270(m_wait1(), n, mT[i] - 1, buf, 1);
    }
}

static void m27518(int n, int trader, int total, int kind)
{
    struct { int32_t id, sc; } rec[10], tmp;
    int32_t list[10], cnt, i, j, share, give, c;
    char buf[100];

    memset(list, 0, sizeof list);
    for (i = 0; i <= 9; i++) {
        rec[i].id = i;
        if (rb(0xF1566u + 199u * (uint32_t)i) == 0)
            rec[i].sc = (int32_t)0xFFFE7961;
        else
            rec[i].sc = (int32_t)((uint32_t)d32(0xF52E0u + 40u * (uint32_t)n + 4u * (uint32_t)i)
                                  + 100u * (uint32_t)sar24(0x109E20u + 9u * (uint32_t)(n - 1) + (uint32_t)i));
    }
    for (i = 8; i >= 0; i--)
        for (j = 0; j <= i; j++)
            if (g_mut == 9 ? rec[j + 1].sc >= rec[j].sc : rec[j + 1].sc > rec[j].sc) {
                tmp = rec[j + 1];
                rec[j + 1] = rec[j];
                rec[j] = tmp;
            }
    cnt = 0;
    for (i = 0; i <= 9; i++)
        if (sar16(0xF796Au + 2u * (uint32_t)rec[i].id) > 0)
            list[cnt++] = rec[i].id;
    memset(g_ds + (0xF547Cu - DS_BASE), 0, 14);
    w8(0xF547Cu + (uint32_t)list[0], rb(0xF796Cu + 2u * (uint32_t)list[0]));
    total -= sar16(0xF796Au + 2u * (uint32_t)list[0]);
    if (total > 0) {
        share = cnt > 2 ? (g_mut == 8 ? total / 2 : (total + 1) / 2) : total;
        c = sar16(0xF796Au + 2u * (uint32_t)list[1]);
        give = c > share ? share : c;
        w8(0xF547Cu + (uint32_t)list[1], give);
        total -= give;
        if (total > 0 && cnt >= 3) {
            if (share > total)
                share = total;
            c = sar16(0xF796Au + 2u * (uint32_t)list[2]);
            give = c > share ? share : c;
            w8(0xF547Cu + (uint32_t)list[2], give);
            total -= give;
            if (total > 0 && cnt >= 4) {
                for (i = 3; i < cnt && total > 0; i++) {
                    c = sar16(0xF796Au + 2u * (uint32_t)list[i]);
                    w8(0xF547Cu + (uint32_t)list[i], c > total ? total : c);
                    total -= c;
                }
            }
        }
    }
    snprintf(buf, sizeof buf, str(0xB2B6Cu), str((uint32_t)d32(0xC76E0u + 4u * (uint32_t)kind)));
    f291D0(n, trader, buf);
}

static void m_grain_boxes(int n, char *buf)
{
    int x;
    fg_vbopen(d32(0xF52C8u));
    f3C9D0(0x136, 0x181, 0x13, 0x13, 5);
    fg_setcolor(5);
    snprintf(buf, 100, "%d", sar16(0xF4098u + 2u * (uint32_t)n));
    f3A0B0(0x136, 0x185, 0x13, buf);
    f3C9D0(0x136, 0x195, 0x13, 0x13, 5);
    fg_setcolor(5);
    x = sar24(0xF5479u + (uint32_t)n);
    if (x < 0)
        x = 0;
    snprintf(buf, 100, "%d", x);
    f3A0B0(0x136, 0x199, 0x13, buf);
    fg_vbclose();
    f3CBBC();
}

static void m27850(int n)
{
    char buf[100];
    int32_t v = 0, i, c, Lc, g, cc, base = g_mut == 10 ? 0 : 0x24;
    uint32_t nr = 199u * (uint32_t)n;

    f3CD6C(3);
    w8(0xEE168u, 3);
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, d32(0xF52CCu), d32(0xF52C8u));
    fg_vbopen(d32(0xF52C8u));
    fg_setcolor(5);
    f3A3A0(0x125, 0x185, str(0xB2B90u));
    f3A044(0x15A, 0x185, str(0xB2B96u));
    f3A3A0(0x12F, 0x199, str(0xB2B9Bu));
    f3A044(0x150, 0x199, str(0xB2BA2u));
    fg_vbclose();
    f7876C();
    snprintf(buf, sizeof buf, str(0xB2BADu), nm(n));
    f3A9F4(buf);
    f3B05C(0xF596Cu);
    w16(0xF409Au + 2u * (uint32_t)n, 0);
    f459E4(n);
    f3F670();
    f3D9FC(n);
    m_grain_boxes(n, buf);
    f78A20(0x3E8);
    g = sar24(0xF14D1u + nr);
    if (g > 2) {
        g_cov[11]++;
        if (sar24(0xF1561u + nr) >= 2 * sar24(0xF151Au + nr) + 4)
            w16(0xF409Au + 2u * (uint32_t)n, 0);
        else {
            w16(0xF409Au + 2u * (uint32_t)n, rw(0xF409Au + 2u * (uint32_t)n) + 1);
            w8(0xF547Cu + (uint32_t)n, rb(0xF547Cu + (uint32_t)n) - 4);
        }
        goto L_27C06;
    }
    for (i = 1; i <= 0x12; i++) {
        c = (int8_t)rb(0xF14CCu + (uint32_t)base + nr + (uint32_t)i);   /* [0xF14ED + 199n + i] sar 24 */
        v += c * c * ((i + 1) >> 1);
    }
    cc = (int8_t)rb(0xF14CCu + (uint32_t)(base + 8) + nr);            /* [0xF14F5 + 199n] sar 24 */
    if (v >= 60 && v - 4 * cc * cc < 60) {
        g_cov[12]++;
        goto L_27C06;
    }
    g_cov[13]++;
    Lc = sar24(0xF5479u + (uint32_t)n);
    if (Lc > 8)
        Lc = 8;
    if (4 * g > sar24(0xF5479u + (uint32_t)n)) {
        w16(0xF409Au + 2u * (uint32_t)n, (int16_t)rw(0xF409Au + 2u * (uint32_t)n) + Lc / 5 + 1);
        w8(0xF547Cu + (uint32_t)n, (int8_t)rb(0xF547Cu + (uint32_t)n) - (4 * (Lc / 5) + 4));
    } else {
        w16(0xF409Au + 2u * (uint32_t)n, (int16_t)rw(0xF409Au + 2u * (uint32_t)n) + g);
        w8(0xF547Cu + (uint32_t)n, (int8_t)rb(0xF547Cu + (uint32_t)n) - 4 * g);
    }
L_27C06:
    m_grain_boxes(n, buf);
    f78A20(0x9C4);
}

static int m2E45C(int n)
{
    char buf[100];
    int32_t i, j, p;
    uint32_t tp;

    w32(0xBEDE8u, g_mut == 11 ? d32(0xF40F0u) >= 3 : d32(0xF40F0u) == 3);
    f3F670();
    for (i = 0; i < d32(LIST_N); i++) {
        if (d32(LIST + 4u * (uint32_t)i) == d32(0xBEDE0u + 4u * (uint32_t)n))
            continue;
        p = d32(LIST + 4u * (uint32_t)i);
        tp = 22u * (uint32_t)p;
        snprintf(buf, sizeof buf, g_original ? str(0xB35A7u) : "%s chooses the %s to flood.", nm(n),
                 cal2_river_text(str((uint32_t)d32(0xBEE0Cu + tp))));
        for (j = 0; sar16(0xBEE08u + tp) > j; j++)
            f3F624(sar16(0xBEE0Eu + tp + 2u * (uint32_t)j), 15);
        f79270(-500, n, -200, buf, 0);
        break;
    }
    return d32(LIST + 4u * (uint32_t)i);
}

static void m2C298(int x, int count, int src, int target, int flag)
{
    int32_t taken = 0, left, T = 0, need, retry, i, j, sel, tk, k, e;
    int keep;
    int8_t pass;

    f3C940((uint32_t)d32(0xF8E78u));
    left = count;
    f2EE6C(x, -1, left);
    if (d32(0xF40F0u) > 3) {
        static const int32_t a[4] = { 0x95, 0x97, 0x8F, 0x90 }, b[5] = { 0x96, 0x87, 0x83, 0x8E, 0x8D };
        for (i = 0; i < 4; i++) w32(0xC0638u + 4u * (uint32_t)i, a[i]);
        for (i = 0; i < 5; i++) w32(0xC06CCu + 4u * (uint32_t)i, b[i]);
    } else if (g_mut != 13) {
        static const int32_t a[4] = { 0x21, 0x7F, 0x35, 0x24 }, b[5] = { 0x13, 0x22, 0x34, 0x12, 0x07 };
        for (i = 0; i < 4; i++) w32(0xC0638u + 4u * (uint32_t)i, a[i]);
        for (i = 0; i < 5; i++) w32(0xC06CCu + 4u * (uint32_t)i, b[i]);
    }
    keep = count >= 20 && (int8_t)flag == 0;
    for (i = 0; i < d32(LIST_N); i++)
        T += sar24(0xE31C3u + 69u * (uint32_t)(d32(LIST + 4u * (uint32_t)i) + 1) + (uint32_t)x);
    need = count - (T < 10 ? T : 10);
    need = need / 5 + (need % 5 > 0);
    retry = 0;
    i = 0;
L_2C577:
    if (i >= d32(LIST_N))
        goto L_2C5C8;
    if ((rb(0xE2C1Au + 3u * (uint32_t)d32(LIST + 4u * (uint32_t)i)) & 1) == 0 || taken >= need || left < 5)
        goto L_2C54E;
    sel = -1;
    if (retry) {
        sel = d32(LIST + 4u * (uint32_t)i);
        goto L_2C4BD;
    }
    for (j = 0; j < 4; j++) {
        e = d32(0xC0618u + 16u * (uint32_t)x + 4u * (uint32_t)j);
        if (d32(LIST + 4u * (uint32_t)i) == e) {
            if (keep) { sel = e; break; }
            continue;
        }
        if (keep)
            continue;
        sel = d32(LIST + 4u * (uint32_t)i);
        break;
    }
L_2C4BD:
    if (sel < 0)
        goto L_2C54E;
    g_cov[14]++;
    f78F04(sel, src);
    f78E74(sel, target);
    left -= 5;
    taken++;
    if (taken == 4)
        taken = 9;
    for (j = i; j < d32(LIST_N); j++)
        w32(LIST + 4u * (uint32_t)j, d32(0xF7CA4u + 4u * (uint32_t)j));
    w32(LIST_N, d32(LIST_N) - 1);
    i--;
    f2EE6C(x, sel, left);
    if (left < 1)
        goto L_2C5C8;
L_2C54E:
    if (retry == 0 && d32(LIST_N) - 1 == i && taken < need) {
        g_cov[15]++;
        retry++;
        i = g_mut == 12 ? -1 : 0;
    }
    i++;
    goto L_2C577;
L_2C5C8:
    if (left <= 0)
        goto L_2C765;
    pass = 2;
L_2C5D6:
    pass--;
    for (i = 0; i < d32(LIST_N); i++) {
        sel = -1;
        if (pass != 0) {
            for (j = 0; j < 5; j++) {
                e = d32(0xC06B8u + 20u * (uint32_t)(x - 1) + 4u * (uint32_t)j);
                if (d32(LIST + 4u * (uint32_t)i) == e) {
                    if (keep) { sel = e; break; }
                    continue;
                }
                if (keep)
                    continue;
                sel = d32(LIST + 4u * (uint32_t)i);
                break;
            }
        } else {
            sel = d32(LIST + 4u * (uint32_t)i);
        }
        if (sel < 0)
            continue;
        tk = sar24(0xE31C3u + 69u * (uint32_t)(sel + 1) + (uint32_t)x);
        if (tk > 0) {
            g_cov[16]++;
            for (k = 0; k < tk; k++) {
                left--;
                f57CE8(sel, src, 0, 1);
                w8(0xF1564u + 199u * (uint32_t)src, rb(0xF1564u + 199u * (uint32_t)src) - 1);
                w8(0xF1565u + 199u * (uint32_t)src, rb(0xF1565u + 199u * (uint32_t)src) + 1);
                f57E28(sel, target, 1);
                w8(0xF1564u + 199u * (uint32_t)target, rb(0xF1564u + 199u * (uint32_t)target) + 1);
                w8(0xF1565u + 199u * (uint32_t)target, rb(0xF1565u + 199u * (uint32_t)target) - 1);
                if (left == 0)
                    break;
            }
            f2EE6C(x, sel, left);
        }
        if (left == 0) {
            pass = 0;
            break;
        }
    }
    if (pass > 0)
        goto L_2C5D6;
L_2C765:
    f3C8C4((uint32_t)d32(0xF8E78u));
}

static void m2C190(int x, int count, const char *text, int src, int target, int mode, int flag)
{
    w16(0xBB400u, 0x15);
    f3CD6C(3);
    w8(0xEE168u, 3);
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, d32(0xF52CCu), d32(0xF52C8u));
    fg_vbopen(d32(0xF52C8u));
    fg_setcolor(5);
    f3A3A0(0x12F, 0x176, str(0xB335Fu));
    if (mode == 4 || mode == 5 || mode == 6)
        f3A044(0x150, 0x176, str(mode == 4 ? 0xB3367u : mode == 5 ? 0xB336Eu : 0xB3374u));
    fg_vbclose();
    f3AB7C(text);
    f7876C();
    f28CFC(src, mode, 0);
    m2C298(x, count, src, target, (int8_t)flag);
}

static void m_show(char *text, int name_t, int y, int w, int left)   /* 0x2C894's box: name (t + 1 index of 0xC77D4) */
{
    fg_vbopen(d32(0xF52C8u));
    f3C9D0(0x109, 0x186, 0x6E, 0x13, 5);
    fg_setcolor(5);
    if (name_t != -1000)
        f3A0B0(0x109, 0x18A, 0x6E, str((uint32_t)d32(0xC77D4u + 4u * (uint32_t)name_t)));
    f3C9D0(0x136, 0x172, 0x13, 0x13, 5);
    fg_setcolor(5);
    snprintf(text, 12, "%d", left);
    f3A0B0(0x136, y, w, text);
    fg_vbclose();
}

static void m2C894(int n, int count, char *text, int ov, int flag, int mode)
{
    int32_t left, i, j, t, b, tk, changed;
    uint8_t exact, core, want;
    uint32_t nr = 199u * (uint32_t)n;

    f3CD6C(3);
    w8(0xEE168u, 3);
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, d32(0xF52CCu), d32(0xF52C8u));
    fg_vbopen(d32(0xF52C8u));
    fg_setcolor(5);
    f3A3A0(0x12F, 0x176, str(0xB337Au));
    f3A044(0x150, 0x176, str(0xB3382u));
    fg_vbclose();
    f3AB7C(text);
    f7876C();
    f3B05C(0xF596Cu);
    f3C940((uint32_t)d32(0xF8E78u));
    f28CFC(n, mode, 0);
    left = count;
    fg_vbopen(d32(0xF52C8u));
    f3C9D0(0x109, 0x186, 0x6E, 0x13, 5);
    fg_setcolor(5);
    f3C9D0(0x136, 0x172, 0x13, 0x13, 5);
    fg_setcolor(5);
    snprintf(text, 12, "%d", left);
    f3A0B0(0x136, 0x177, 0x13, text);
    fg_vbclose();
    f3F670();
    f3D52C(0, 0);
    f3CBBC();
    f78A20(m_wait2());
    exact = (uint8_t)((g_mut == 14 ? ov : (ov < 0 ? -ov : ov)) + left == 5);
    for (i = 0; i < d32(LIST_N); i++) {
        if (left <= ov)
            break;
        t = d32(LIST + 4u * (uint32_t)i);
        if (rb(0xE2C18u + 3u * (uint32_t)t) != n && rb(0xE31C6u + 69u * (uint32_t)(t + 1) + (uint32_t)n) == 0)
            continue;
        want = (int8_t)flag != 0;
        changed = 0;
        if ((rb(0xE2C1Au + 3u * (uint32_t)t) & 1) && rb(0xE2C18u + 3u * (uint32_t)t) == n && exact) {
            core = 0;
            for (j = 0; j < 4; j++)
                if (t != d32(0xC0618u + 16u * (uint32_t)n + 4u * (uint32_t)j)) {
                    core = 1;
                    break;
                }
            if (core == want) {
                g_cov[17]++;
                f78F04(t, n);
                left -= 5;
                changed = 1;
            }
        } else if (rb(0xE31C6u + 69u * (uint32_t)(t + 1) + (uint32_t)n) != 0 && !exact) {
            core = 0;
            for (j = 0; j < 5; j++)
                if (t == d32(0xC06B8u + 20u * (uint32_t)(n - 1) + 4u * (uint32_t)j)) {
                    core = 1;
                    break;
                }
            if (core == want) {
                tk = sar24(0xE31C3u + 69u * (uint32_t)(t + 1) + (uint32_t)n);
                while (tk > 0 && left > ov) {
                    g_cov[18]++;
                    f57CE8(t, n, 0, 1);
                    if (!g_original && g_mut != 15) {
                        w8(0xF1564u + nr, rb(0xF1564u + nr) - 1);
                        w8(0xF1565u + nr, rb(0xF1565u + nr) + 1);
                    }
                    tk--;
                    left--;
                    changed = 1;
                }
            }
        }
        if (!changed)
            continue;
        b = rb(0xE31B8u + 69u * (uint32_t)(t + 1));
        m_show(text, b, 0x176, 0x14, left);
        f3F670();
        f3F624(b - 1, 15);
        f3DA60(b - 1);
        f3CBBC();
        f78A20(m_wait2());
    }
    /* pass 2 */
    m_show(text, (g_original || d32(LIST_N) > 0) ? d32(LIST) + 1 : -1000, 0x176, 0x13, left);
    f3F670();
    if (g_original || d32(LIST_N) > 0) {
        f3F624(d32(LIST), 15);
        f3DA60(d32(LIST));
    } else {
        g_cov[19]++;
        f3D52C(0, 0);
    }
    f3CBBC();
    if (left <= ov)
        goto L_2D0F6;
    f78A20(m_wait2());
    if (left <= ov)
        goto L_2D0F6;
    for (i = 0; i < d32(LIST_N); i++) {
        if (left <= ov)
            break;
        changed = 0;
        t = d32(LIST + 4u * (uint32_t)i);
        if ((rb(0xE2C1Au + 3u * (uint32_t)t) & 1) && rb(0xE2C18u + 3u * (uint32_t)t) == n && exact) {
            g_cov[20]++;
            f78F04(t, n);
            left -= 5;
            changed = 1;
        } else if (rb(0xE31C6u + 69u * (uint32_t)(t + 1) + (uint32_t)n) != 0 && !exact) {
            while (sar24(0xE31C3u + 69u * (uint32_t)(t + 1) + (uint32_t)n) > 0 && left > ov) {
                g_cov[21]++;
                f57CE8(t, n, 0, 1);
                if (!g_original && g_mut != 15) {
                    w8(0xF1564u + nr, rb(0xF1564u + nr) - 1);
                    w8(0xF1565u + nr, rb(0xF1565u + nr) + 1);
                }
                left--;
                changed = 1;
            }
        }
        if (!changed)
            continue;
        m_show(text, t + 1, 0x176, 0x14, left);
        f3F670();
        f3F624(t, 15);
        f3DA60(t);
        f3CBBC();
        f78A20(m_wait2());
    }
L_2D0F6:
    f3C8C4((uint32_t)d32(0xF8E78u));
}

static void m2D10C(int v, int k, int o, int b)
{
    char buf[100];
    int r, t;
    while (k != 0) {
        r = aical2_test_f20C70(b, o);
        t = r - 1;
        if (g_mut != 17)
            w8(0xE31F3u + 69u * (uint32_t)r + (uint32_t)v, 1);
        f78F04(t, o);
        f78E74(t, b);
        k--;
        snprintf(buf, sizeof buf, str(0xB3389u), nm(b), tn(t));
        f79270(0x5DC, b, t, buf, 1);
    }
}

static void m2D1C0(int v, int k, int o, int b)
{
    char buf[100];
    int32_t lst[200], cnt = 0, i, j, flag, match, tk, t = 0, moved;

    for (i = 0; sar24(0xF1529u + 199u * (uint32_t)o) > i; i++)
        lst[cnt++] = rb(0xF152Du + 199u * (uint32_t)o + (uint32_t)i);
    flag = 1;
    while (k != 0) {
        match = 0;
        moved = 0;
        for (i = 0; i < cnt && k > 0; i++) {
            if (g_mut == 16)
                match = 0;
            for (j = 0; j < 5; j++)
                if (lst[i] == d32(0xC06B8u + 20u * (uint32_t)(v - 1) + 4u * (uint32_t)j)) {
                    match = 1;
                    break;
                }
            if (match != flag)
                continue;
            tk = sar24(0xE31C3u + 69u * (uint32_t)lst[i] + (uint32_t)o);
            for (j = 0; j < tk && k > 0; j++) {
                w8(0xE31F3u + 69u * (uint32_t)lst[i] + (uint32_t)v, 1);
                t = lst[i] - 1;
                f57CE8(t, o, 0, 1);
                k--;
                w8(0xF1564u + 199u * (uint32_t)o, rb(0xF1564u + 199u * (uint32_t)o) - 1);
                w8(0xF1565u + 199u * (uint32_t)o, rb(0xF1565u + 199u * (uint32_t)o) + 1);
                f57E28(t, b, 1);
                w8(0xF1564u + 199u * (uint32_t)b, rb(0xF1564u + 199u * (uint32_t)b) + 1);
                w8(0xF1565u + 199u * (uint32_t)b, rb(0xF1565u + 199u * (uint32_t)b) - 1);
                moved++;
            }
            if (tk > 0) {
                snprintf(buf, sizeof buf, str(0xB33A3u), nm(b), tn(t));
                f79270(0x5DC, b, t, buf, 1);
            }
        }
        if (!g_original && flag == 0 && moved == 0) {
            g_cov[22]++;
            break;
        }
        flag = 0;
    }
}

/* ---- one check: the port, then the model, from the same start ---------------------------------------------------- */

enum { F260C4, F27518, F27850, F2E45C, F2C190, F2C298, F2C894, F2D10C, F2D1C0, NFN };
static const char *const fn_name[NFN] = { "260C4", "27518", "27850", "2E45C", "2C190", "2C298", "2C894", "2D10C",
                                          "2D1C0" };

typedef struct {
    int fn;
    int a[7];
    char text[100];
    uint32_t seed;
} Call;

static uint8_t *P, *Mm, *g_start, *g_base[12], *g_ds0;
static int g_nbase;
static int32_t g_retp, g_retm;
static char g_textp[100], g_textm[100];
static long g_runs[NFN], g_fails[NFN];

static int32_t call_port(const Call *c, char *text)
{
    const int *a = c->a;
    switch (c->fn) {
    case F260C4: f260C4(a[0], a[1], a[2], a[3]); return 0;
    case F27518: f27518(a[0], a[1], a[2], a[3]); return 0;
    case F27850: f27850(a[0]); return 0;
    case F2E45C: return f2E45C(a[0]);
    case F2C190: f2C190(a[0], a[1], text, a[2], a[3], a[4], a[5]); return 0;
    case F2C298: f2C298(a[0], a[1], a[2], a[3], a[4]); return 0;
    case F2C894: f2C894(a[0], a[1], text, a[2], a[3], a[4]); return 0;
    case F2D10C: f2D10C(a[0], a[1], a[2], a[3]); return 0;
    default:     f2D1C0(a[0], a[1], a[2], a[3]); return 0;
    }
}

static int32_t call_model(const Call *c, char *text)
{
    const int *a = c->a;
    switch (c->fn) {
    case F260C4: m260C4(a[0], a[1], a[2], a[3]); return 0;
    case F27518: m27518(a[0], a[1], a[2], a[3]); return 0;
    case F27850: m27850(a[0]); return 0;
    case F2E45C: return m2E45C(a[0]);
    case F2C190: m2C190(a[0], a[1], text, a[2], a[3], a[4], a[5]); return 0;
    case F2C298: m2C298(a[0], a[1], a[2], a[3], a[4]); return 0;
    case F2C894: m2C894(a[0], a[1], text, a[2], a[3], a[4]); return 0;
    case F2D10C: m2D10C(a[0], a[1], a[2], a[3]); return 0;
    default:     m2D1C0(a[0], a[1], a[2], a[3]); return 0;
    }
}

static void first_diff(const char *a, const char *b, char *out, size_t size)
{
    int line = 1;
    const char *la = a, *lb = b;
    while (*a && *a == *b) {
        if (*a == '\n') {
            line++;
            la = a + 1;
            lb = b + 1;
        }
        a++;
        b++;
    }
    snprintf(out, size, "line %d: port \"%.90s\" / model \"%.90s\"", line, la, lb);
    {
        char *p = strchr(out, '\n');
        while (p != NULL) { *p = '|'; p = strchr(p, '\n'); }
    }
}

/* runs c from start (the port's result stays in P); 1 when port and model agree */
static int run(const Call *c, const uint8_t *start, const char *what)
{
    uint32_t i;
    char msg[400];
    int ok = 1, cutp, cutm;

    g_runs[c->fn]++;
    memcpy(P, start, DS_SIZE);
    g_ds = P;
    g_log = g_logs[0]; g_len = &g_lens[0]; g_lens[0] = 0; g_logs[0][0] = 0;
    g_srng = c->seed | 1u; g_steps = 0;
    memcpy(g_textp, c->text, sizeof g_textp);
    g_retp = 0;
    cutp = setjmp(g_jb);
    if (cutp == 0)
        g_retp = call_port(c, g_textp);

    memcpy(Mm, start, DS_SIZE);
    g_ds = Mm;
    g_log = g_logs[1]; g_len = &g_lens[1]; g_lens[1] = 0; g_logs[1][0] = 0;
    g_srng = c->seed | 1u; g_steps = 0;
    memcpy(g_textm, c->text, sizeof g_textm);
    g_retm = 0;
    cutm = setjmp(g_jb);
    if (cutm == 0)
        g_retm = call_model(c, g_textm);
    g_ds = P;

    if (strcmp(g_logs[0], g_logs[1]) != 0) {
        first_diff(g_logs[0], g_logs[1], msg, sizeof msg);
        ok = 0;
    } else if (memcmp(P, Mm, DS_SIZE) != 0) {
        for (i = 0; i < DS_SIZE && P[i] == Mm[i]; i++)
            ;
        snprintf(msg, sizeof msg, "byte 0x%X: port %02X model %02X", (unsigned)(i + DS_BASE), P[i], Mm[i]);
        ok = 0;
    } else if (g_retp != g_retm) {
        snprintf(msg, sizeof msg, "return port %d model %d", (int)g_retp, (int)g_retm);
        ok = 0;
    } else if (memcmp(g_textp, g_textm, sizeof g_textp) != 0) {
        snprintf(msg, sizeof msg, "text buffer port \"%.40s\" model \"%.40s\"", g_textp, g_textm);
        ok = 0;
    }
    if (!ok) {
        g_fails[c->fn]++;
        if (!g_quiet)
            printf("   %s %s (%d,%d,%d,%d,%d,%d) seed %u%s: %s\n", what, fn_name[c->fn], c->a[0], c->a[1], c->a[2],
                   c->a[3], c->a[4], c->a[5], (unsigned)c->seed, g_original ? " --original" : "", msg);
    }
    return check(ok, what);
}

/* ---- state builders (on g_ds) -------------------------------------------------------------------------------- */

static void tok_on(int t, int n, int k)     /* k tokens of n onto t with the board and stock counts */
{
    if (k <= 0)
        return;
    f57E28(t, n, k);
    w8(NAT(n) + 0x98u, rb(NAT(n) + 0x98u) + k);
    w8(NAT(n) + 0x99u, rb(NAT(n) + 0x99u) - k);
}

static void tok_off(int t, int n, int k)
{
    if (k <= 0)
        return;
    f57CE8(t, n, 0, k);
    w8(NAT(n) + 0x98u, rb(NAT(n) + 0x98u) - k);
    w8(NAT(n) + 0x99u, rb(NAT(n) + 0x99u) + k);
}

static int tokens_of(int t, int n) { return sb(AR(t + 1) + 0x0Eu + (uint32_t)n); }

static int free_site(int t) { return (rb(TS(t) + 2u) & 1) == 0; }

static void city_to(int t, int from, int to)
{
    if (from > 0)
        f78F04(t, from);
    f78E74(t, to);
}

/* a random state from a base: n gets some tokens and cities, the tables random */
static void randomize_nation(int n, int heavy)
{
    int i, t;
    for (i = 0; i < (heavy ? 8 : 3); i++) {
        t = (int)rnd(157);
        if (rnd(3) == 0 && tokens_of(t, n) > 0)
            tok_off(t, n, 1 + (int)rnd((uint32_t)tokens_of(t, n)));
        else
            tok_on(t, n, 1 + (int)rnd(5));
    }
    if (rnd(3) == 0 && sb(NAT(n) + 0x51u) < 9) {
        t = (int)rnd(157);
        if (free_site(t))
            city_to(t, 0, n);
    }
    w8(NAT(n) + 0x99u, (int)rnd(30));
    w8(NAT(n) + 0x5Cu, (int)rnd(16));
}

static void random_globals(void)
{
    int i;
    w8(0xF0980u, (rb(0xF0980u) & ~4) | (rnd(3) == 0 ? 4 : 0));
    w16(0xF40BAu, rnd(3) == 0 ? (int)rnd(4) : 0);
    w32(0xF40F0u, 3 + (int)rnd(6));
    for (i = 0; i < 10; i++)
        w32(0xF4204u + 4u * (uint32_t)i, rnd(2) ? 1 : 0);
}

/* the original hangs in 0x260C4's passes when no listed territory without a city of n keeps a token to plan: an
   --original run needs one with at least 2 tokens of n (D: the city loop only touches the cities' territories) */
static int no_hang_260C4(int n, int kind, int s)
{
    int i, t;
    if (kind == 0x19)
        return 1;
    if (kind == 0x1B && d32(0xF4204u + 4u * (uint32_t)n) == 0)
        return 1;
    f28CFC(n, kind, s);
    for (i = 0; i < d32(LIST_N); i++) {
        t = d32(LIST + 4u * (uint32_t)i);
        if (!city_of(t, n) && tokens_of(t, n) >= 2)
            return 1;
    }
    return 0;
}

static int list_tokens(int o)
{
    int i, s = 0;
    for (i = 0; i < sb(NAT(o) + 0x60u); i++)
        s += tokens_of(rb(NAT(o) + 0x61u + (uint32_t)i) - 1, o);
    return s;
}

/* faction f (12 / 13) gets some of v's units */
static void make_faction(int v, int f, int toks, int cities)
{
    int t, k;
    for (t = 0; t < 157 && toks > 0; t++) {
        k = tokens_of(t, v);
        if (k <= 0 || rnd(3) == 0)
            continue;
        if (k > toks)
            k = toks;
        tok_off(t, v, k);
        tok_on(t, f, k);
        toks -= k;
    }
    for (t = 0; t < 157 && cities > 0; t++)
        if (city_of(t, v) && rnd(2) == 0) {
            city_to(t, v, f);
            cities--;
        }
}

static int count_negative(void)
{
    int r, n, bad = 0;
    for (r = 1; r <= 160; r++)
        for (n = 1; n <= 13; n++)
            if (sb(AR(r) + 0x0Eu + (uint32_t)n) < 0)
                bad++;
    return bad;
}

/* ---- part B: random checks ---------------------------------------------------------------------------------------- */

static int build_random(Call *c, int fn)
{
    int n, k, i, t, s, v, o, b, sum;

    memcpy(g_ds, g_base[rnd((uint32_t)g_nbase)], DS_SIZE);
    random_globals();
    memset(c, 0, sizeof *c);
    c->fn = fn;
    c->seed = (uint32_t)rnd(0x7FFFFFFF) + 1u;
    snprintf(c->text, sizeof c->text, "check text %u", (unsigned)rnd(1000));
    n = 1 + (int)rnd(9);
    switch (fn) {
    case F260C4: {
        int kind = rnd(3) == 0 ? 0x19 : rnd(2) ? 0x15 : 0x1B;
        if (kind == 0x19 && rnd(6) == 0)
            n = 10;
        randomize_nation(n, 1);
        for (i = 0; i < 160; i++)
            if (rnd(4) == 0)
                w8(0xEED84u + 160u * (uint32_t)n + (uint32_t)i, (int)rnd(256));
        s = (int)rnd(5);
        if (kind == 0x19) {
            int cnt = (int16_t)rw(0xBEE0Au + 22u * (uint32_t)s);
            for (i = 0; i < cnt; i++) {
                t = (int16_t)rw(0xBEE10u + 22u * (uint32_t)s + 2u * (uint32_t)i);
                if (rnd(3) == 0)
                    tok_on(t, n, 1 + (int)rnd(4));
                if (rnd(4) == 0 && free_site(t) && sb(NAT(n) + 0x51u) < 9)
                    city_to(t, 0, n);
                if (rnd(4) == 0)
                    w8(TR(t) + 1u, rb(TR(t) + 1u) ^ 0x80);
            }
            w16(0xBEE08u + 22u * (uint32_t)s, (int)rnd(30));
            k = 1 + (int)rnd(17);
        } else if (kind == 0x15) {
            k = rnd(3) == 0 ? 10 : 1 + (int)rnd(10);
        } else {
            k = 1 + (int)rnd(21);
        }
        if (rnd(4) == 0) {   /* few tokens: the hang / over-planning cases */
            for (t = 0; t < 157; t++)
                if (tokens_of(t, n) > 0 && rnd(3))
                    tok_off(t, n, tokens_of(t, n));
        }
        c->a[0] = n; c->a[1] = k; c->a[2] = kind; c->a[3] = kind == 0x19 ? s : -1;
        if (g_original && !no_hang_260C4(n, kind, c->a[3]))
            return 0;
        return 1;
    }
    case F27518: {
        int kind = 0x13 + (int)rnd(12);
        sum = 0;
        for (i = 0; i <= 10; i++) {
            v = rnd(3) ? (int)rnd(11) : 0;
            if (i == 0 && rnd(4))
                v = 0;
            w16(0xF796Cu + 2u * (uint32_t)i, v);
        }
        w16(0xF796Cu + 2u * (uint32_t)n, 0);
        for (i = 0; i <= 9; i++) {
            if (rnd(5) == 0)
                w8(NAT(i) + 0x9Au, rnd(2) ? 0 : 2);
            w32(0xF52E0u + 40u * (uint32_t)n + 4u * (uint32_t)i, (int32_t)rnd(2000) - 1000);
            w8(0x109E23u + 9u * (uint32_t)(n - 1) + (uint32_t)i, (int)rnd(256));
        }
        for (i = 0; i <= 9; i++) {
            /* the nations the ranking lists: in play or not, as long as their count is > 0 */
            v = (int16_t)rw(0xF796Cu + 2u * (uint32_t)i);
            if (v > 0)
                sum += v;
        }
        if (sum < 2)
            return 0;
        c->a[0] = n; c->a[1] = rnd(3) ? -1 : (int)rnd(10); c->a[2] = 1 + (int)rnd((uint32_t)(sum - 1)); c->a[3] = kind;
        return 1;
    }
    case F27850:
        for (i = 0x25; i <= 0x36; i++)
            w8(NAT(n) + (uint32_t)i, rnd(2) ? 1 : 0);
        w8(NAT(n) + 0x08u, (int)rnd(6));
        w8(0xF547Cu + (uint32_t)n, (int)rnd(16) - 5);
        w8(NAT(n) + 0x98u, (int)rnd(40));
        w8(NAT(n) + 0x51u, (int)rnd(10));
        c->a[0] = n;
        return 1;
    case F2E45C: {
        int pl[5] = { 0, 1, 2, 3, 4 }, cnt = 2 + (int)rnd(4), tmp;
        for (i = 4; i > 0; i--) {
            k = (int)rnd((uint32_t)i + 1u);
            tmp = pl[i]; pl[i] = pl[k]; pl[k] = tmp;
        }
        w32(LIST_N, cnt);
        for (i = 0; i < 157; i++)
            w32(LIST + 4u * (uint32_t)i, i < cnt ? pl[i] : -1);
        c->a[0] = n;
        return 1;
    }
    case F2C190: case F2C298: {
        int mode = rnd(3) == 0 ? (rnd(2) ? 5 : 6) : 4;
        randomize_nation(n, 1);
        v = n; o = 12;
        if (mode != 4) {   /* annexing: src = a faction, target = a beneficiary */
            o = rnd(2) ? 12 : 13;
            make_faction(n, o, 5 + (int)rnd(20), (int)rnd(4));
            v = o;
            b = 1 + (int)rnd(9);
        } else {
            b = 12;
        }
        c->a[0] = rnd(8) ? n : 1 + (int)rnd(9);   /* x: the panel's nation (n at both call sites) */
        c->a[1] = 1 + (int)rnd(35);
        if (fn == F2C190) {
            c->a[2] = v; c->a[3] = b; c->a[4] = mode; c->a[5] = rnd(2) ? 1 : (rnd(5) ? 0 : 0x100 | (int)rnd(256));
        } else {
            f28CFC(v, mode, 0);
            c->a[2] = v; c->a[3] = b; c->a[4] = (int8_t)(rnd(2) ? 1 : 0);
        }
        return 1;
    }
    case F2C894: {
        int f = rnd(2) ? 12 : 13, mode = 7 + (int)rnd(3);
        randomize_nation(n, 1);
        make_faction(n, f, 3 + (int)rnd(25), (int)rnd(4));
        for (t = 0; t < 157; t++) {
            w8(0xF7984u + (uint32_t)t, rnd(4) == 0 && (tokens_of(t, 12) > 0 || city_of(t, 12)) ? 1 : (rnd(30) == 0));
            w8(0xF7A24u + (uint32_t)t, rnd(4) == 0 && (tokens_of(t, 13) > 0 || city_of(t, 13)) ? 1 : (rnd(30) == 0));
        }
        if (rnd(10) == 0)   /* an empty list */
            for (t = 0; t < 157; t++)
                w8((mode == 7 ? 0xF7984u : 0xF7A24u) + (uint32_t)t, 0);
        k = 1 + (int)rnd(6);
        c->a[0] = f; c->a[1] = k; c->a[2] = rnd(4) == 0 ? -(int)rnd(3) : (int)rnd((uint32_t)k + 1u);
        c->a[3] = rnd(2) ? 1 : 0; c->a[4] = mode;
        return 1;
    }
    case F2D10C: {
        o = rnd(2) ? 12 : 13;
        make_faction(n, o, (int)rnd(5), 1 + (int)rnd(5));
        b = 1 + (int)rnd(9);
        k = sb(NAT(o) + 0x51u);
        if (k <= 0)
            return 0;
        c->a[0] = n; c->a[1] = (int)rnd((uint32_t)k + 1u); c->a[2] = o; c->a[3] = b;
        return 1;
    }
    default: {
        o = rnd(2) ? 12 : 13;
        randomize_nation(n, 1);
        make_faction(n, o, 2 + (int)rnd(25), 0);
        b = 1 + (int)rnd(9);
        sum = list_tokens(o);
        k = (int)rnd((uint32_t)sum + 6u);
        if (g_original && k > sum)
            return 0;
        c->a[0] = n; c->a[1] = k; c->a[2] = o; c->a[3] = b;
        return 1;
    }
    }
}

static int part_b(int per_fn, const char *what)
{
    int fn, i, made, fail0 = g_fail;
    Call c;
    char label[64];
    for (fn = 0; fn < NFN; fn++) {
        for (i = 0, made = 0; made < per_fn && i < per_fn * 20; i++) {
            g_ds = g_start;
            if (!build_random(&c, fn))
                continue;
            made++;
            snprintf(label, sizeof label, "%s %d", what, made);
            run(&c, g_start, label);
            if (fn == F260C4 && !g_original && !g_mut)
                check(count_negative() == 0, "260C4 left a negative count (fixed)");
        }
    }
    return g_fail - fail0;
}

/* ---- part A: hand vectors --------------------------------------------------------------------------------------- */

static void clear_nation_units(int n)
{
    int t;
    for (t = 0; t < 157; t++) {
        if (tokens_of(t, n) > 0)
            tok_off(t, n, tokens_of(t, n));
        if (city_of(t, n))
            f78F04(t, n);
    }
}

static int find_site(int lim, int avoid)
{
    int t;
    for (t = 0; t < 157; t++)
        if (t != avoid && LIM(t) == lim && free_site(t) && rb(AR(t + 1) + 1u) == 0)
            return t;
    return -1;
}

static void part_a(void)
{
    Call c;
    char what[160];
    int x, y, n, i, t;
    int fail0 = g_fail;

    /* A1. 0x27518: ranks and shares. n = 3; nations 1..6 in play, 7..9 out; counts 1:4 2:6 4:3 5:5 6:2; scores
       1:10, 2:500, 4:30, 5:30 (tie: the lower stays first), 6:-50; total 12 over four nations: the first (2) all 6,
       then share (6 + 1) / 2 = 3: 4 gets 3 (of 3)... order by score: 2 (500), 4 (30), 5 (30), 1 (10), 6 (-50) */
    g_ds = g_start;
    memcpy(g_ds, g_base[0], DS_SIZE);
    for (i = 0; i <= 9; i++) {
        w8(NAT(i) + 0x9Au, i >= 1 && i <= 6 ? 2 : 0);
        w32(0xF52E0u + 40u * 3u + 4u * (uint32_t)i, 0);
        w8(0x109E23u + 9u * 2u + (uint32_t)i, 0);
        w16(0xF796Cu + 2u * (uint32_t)i, 0);
    }
    w32(0xF52E0u + 120u + 4u, 10); w32(0xF52E0u + 120u + 8u, 500); w32(0xF52E0u + 120u + 16u, 30);
    w32(0xF52E0u + 120u + 20u, 30); w32(0xF52E0u + 120u + 24u, -50);
    w16(0xF796Cu + 2u, 4); w16(0xF796Cu + 4u, 6); w16(0xF796Cu + 8u, 3); w16(0xF796Cu + 10u, 5); w16(0xF796Cu + 12u, 2);
    memset(&c, 0, sizeof c);
    c.fn = F27518; c.a[0] = 3; c.a[1] = -1; c.a[2] = 12; c.a[3] = 0x15; c.seed = 7;
    run(&c, g_start, "A1 27518");
    g_ds = P;
    /* 12 - 6 = 6 left; share (6+1)/2 = 3 (5 nations listed); 4: min(3, 3) = 3, left 3; 5: share min(3, 3) -> 3, left 0 */
    check(rb(0xF547Cu + 2u) == 6 && rb(0xF547Cu + 4u) == 3 && rb(0xF547Cu + 5u) == 3 && rb(0xF547Cu + 1u) == 0
          && rb(0xF547Cu + 6u) == 0, "A1 27518 shares 2:6 4:3 5:3");
    check(strstr(g_logs[0], "Assignments for Famine secondary losses") != NULL, "A1 27518 text (the name 0xC76E0[0x15])");

    /* A2. 0x2E45C: Iberia (2) with plains {1, 0}: 3 players -> own river 1 (Po), picks 0 (Rhone); 4 players -> own 0,
       picks 1 (Po). Egypt (9) with {4, 3}: picks 3, "TIGRIS" fixed, "chooses"; --original "TIGRUS", "choses" */
    for (x = 0; x < 2; x++) {
        g_ds = g_start;
        memcpy(g_ds, g_base[0], DS_SIZE);
        w32(0xF40F0u, x == 0 ? 3 : 4);
        w32(LIST_N, 2); w32(LIST, 1); w32(LIST + 4u, 0);
        memset(&c, 0, sizeof c);
        c.fn = F2E45C; c.a[0] = 2; c.seed = 3;
        run(&c, g_start, "A2 2E45C Iberia");
        g_ds = P;
        check(g_retp == (x == 0 ? 0 : 1) && d32(0xBEDE8u) == (x == 0 ? 1 : 0), "A2 2E45C Iberia's own river by players");
    }
    for (x = 0; x < 2; x++) {
        g_original = x;
        g_ds = g_start;
        memcpy(g_ds, g_base[0], DS_SIZE);
        w32(LIST_N, 2); w32(LIST, 4); w32(LIST + 4u, 3);
        memset(&c, 0, sizeof c);
        c.fn = F2E45C; c.a[0] = 9; c.seed = 3;
        run(&c, g_start, "A2 2E45C Egypt");
        snprintf(what, sizeof what, "%s the %s to flood.", x ? "choses" : "chooses", x ? "TIGRUS-EUPHRATES" : "TIGRIS-EUPHRATES");
        check(g_retp == 3 && strstr(g_logs[0], what) != NULL, "A2 2E45C Egypt never floods the Nile; the text");
        g_original = 0;
    }

    /* A3. 0x27850: g = 1, no advances: v = 0 -> used; loss 10 > 4: used 1, loss 6. g = 2, loss 6: 8 > 6 -> used 6/5+1 = 2,
       loss 6 - 8 = -2. g = 3, tokens 5 < 2 * 2 + 4: used 1, loss - 4; tokens 9: used 0. The advance bytes: +0x2C (i = 8)
       and i 7, 11, 13..18 set: v = 62, 62 - 4 = 58 < 60 -> not used (the Grain count +0x08 = 1 is not what is read) */
    {
        static const int gv[4] = { 1, 2, 3, 3 }, lv[4] = { 10, 6, 7, 7 }, tv[4] = { 0, 0, 5, 9 };
        static const int used[4] = { 1, 2, 1, 0 }, left[4] = { 6, -2, 3, 7 };
        for (x = 0; x < 5; x++) {
            g_ds = g_start;
            memcpy(g_ds, g_base[0], DS_SIZE);
            n = 4;
            for (i = 0x25; i <= 0x36; i++)
                w8(NAT(n) + (uint32_t)i, 0);
            if (x < 4) {
                w8(NAT(n) + 0x08u, gv[x]); w8(0xF547Cu + (uint32_t)n, lv[x]);
                w8(NAT(n) + 0x98u, tv[x]); w8(NAT(n) + 0x51u, 2);
            } else {
                static const int set[9] = { 7, 8, 11, 13, 14, 15, 16, 17, 18 };
                w8(NAT(n) + 0x08u, 1); w8(0xF547Cu + (uint32_t)n, 10);
                for (i = 0; i < 9; i++)
                    w8(NAT(n) + 0x24u + (uint32_t)set[i], 1);
            }
            memset(&c, 0, sizeof c);
            c.fn = F27850; c.a[0] = n; c.seed = 5;
            run(&c, g_start, "A3 27850");
            g_ds = P;
            if (x < 4)
                check((int16_t)rw(0xF409Au + 8u) == used[x] && sb(0xF547Cu + 4u) == left[x], "A3 27850 used / losses");
            else
                check((int16_t)rw(0xF409Au + 8u) == 0 && sb(0xF547Cu + 4u) == 10, "A3 27850 advance bytes: not used");
        }
    }

    /* A4. 0x260C4, the file comment's example: n = 6 without cities, 3 tokens in X (limit 3), 3 in Y (limit 1),
       Famine L = 6, 0xF4204[n] = 0. --original: X -2 (0xFE), Y 2, board - 6; fixed: X 0, Y 0 */
    for (x = 0; x < 2; x++) {
        g_original = x;
        g_ds = g_start;
        memcpy(g_ds, g_base[0], DS_SIZE);
        n = 6;
        clear_nation_units(n);
        t = find_site(3, -1);
        y = find_site(1, t);
        if (t < 0 || y < 0) {
            check(0, "A4 no territories of limit 3 and 1");
            break;
        }
        tok_on(t, n, 3);
        tok_on(y, n, 3);
        w32(0xF4204u + 4u * (uint32_t)n, 0);
        memset(&c, 0, sizeof c);
        c.fn = F260C4; c.a[0] = n; c.a[1] = 6; c.a[2] = 0x15; c.a[3] = -1; c.seed = 11;
        run(&c, g_start, x ? "A4 260C4 over-planned (--original)" : "A4 260C4 over-planned (fixed)");
        g_ds = P;
        if (x)
            check(tokens_of(t, n) == -2 && tokens_of(y, n) == 2, "A4 260C4 --original: X -2, Y 2");
        else
            check(tokens_of(t, n) == 0 && tokens_of(y, n) == 0, "A4 260C4 fixed: X 0, Y 0");
        printf("   A4 %s: X (t %d, limit 3) %d, Y (t %d, limit 1) %d, board %d\n", x ? "--original" : "fixed     ", t,
               tokens_of(t, n), y, tokens_of(y, n), sb(NAT(n) + 0x98u));
        g_original = 0;
    }
    /* the hang: one city, no tokens, no stock, Famine L = 8: the city goes (L 3), nothing left to plan; fixed: it ends */
    g_ds = g_start;
    memcpy(g_ds, g_base[0], DS_SIZE);
    n = 6;
    clear_nation_units(n);
    t = find_site(2, -1);
    if (t < 0)
        t = find_site(3, -1);
    city_to(t, 0, n);
    w8(NAT(n) + 0x99u, 0);
    w32(0xF4204u + 4u * (uint32_t)n, 0);
    memset(&c, 0, sizeof c);
    c.fn = F260C4; c.a[0] = n; c.a[1] = 8; c.a[2] = 0x15; c.a[3] = -1; c.seed = 13;
    run(&c, g_start, "A4 260C4 nothing left to plan (fixed)");
    g_ds = P;
    check(strstr(g_logs[0], "CUT") == NULL && !city_of(t, n), "A4 260C4 the fixed passes end with nothing left");
    memcpy(g_start, g_base[0], DS_SIZE);   /* the priority passes too */
    g_ds = g_start;
    clear_nation_units(n);
    city_to(t, 0, n);
    w8(NAT(n) + 0x99u, 0);
    w32(0xF4204u + 4u * (uint32_t)n, 1);
    run(&c, g_start, "A4 260C4 nothing left (priority, fixed)");
    c.a[2] = 0x1B;
    run(&c, g_start, "A4 260C4 nothing left (Epidemic priority, fixed)");

    /* A5. 0x2C298's Iberia rows by the player count (state outside the save) */
    for (x = 0; x < 2; x++) {
        g_ds = g_start;
        memcpy(g_ds, g_base[0], DS_SIZE);
        w32(0xF40F0u, x ? 5 : 3);
        f28CFC(4, 4, 0);
        memcpy(g_start, g_ds, DS_SIZE);
        memset(&c, 0, sizeof c);
        c.fn = F2C298; c.a[0] = 4; c.a[1] = 15; c.a[2] = 4; c.a[3] = 12; c.a[4] = 0; c.seed = 17;
        run(&c, g_start, "A5 2C298");
        g_ds = P;
        check(d32(0xC0638u) == (x ? 149 : 33) && d32(0xC0644u) == (x ? 144 : 36) && d32(0xC06CCu) == (x ? 150 : 19)
              && d32(0xC06DCu) == (x ? 141 : 7), "A5 2C298 Iberia's rows");
    }

    /* A6. 0x2C894: faction 12 with tokens only (no city: exact never matters), count 3, ov 0, mode 9: the board loses 3;
       fixed: +0x98 - 3, +0x99 + 3; --original: both unchanged */
    for (x = 0; x < 2; x++) {
        int b98, b99;
        g_original = x;
        g_ds = g_start;
        memcpy(g_ds, g_base[0], DS_SIZE);
        clear_nation_units(12);
        n = 0;
        for (t = 0; t < 157 && n < 6; t++)
            if (tokens_of(t, 3) >= 2) {
                tok_off(t, 3, 2);
                tok_on(t, 12, 2);
                n += 2;
            }
        b98 = sb(NAT(12) + 0x98u);
        b99 = sb(NAT(12) + 0x99u);
        memset(&c, 0, sizeof c);
        c.fn = F2C894; c.a[0] = 12; c.a[1] = 3; c.a[2] = 0; c.a[3] = 0; c.a[4] = 9; c.seed = 19;
        snprintf(c.text, sizeof c.text, "%s", "Somebody choosing MILITARY losses for First Faction.");
        run(&c, g_start, "A6 2C894");
        g_ds = P;
        if (x)
            check(sb(NAT(12) + 0x98u) == b98 && sb(NAT(12) + 0x99u) == b99, "A6 2C894 --original: counts stale");
        else
            check(sb(NAT(12) + 0x98u) == b98 - 3 && sb(NAT(12) + 0x99u) == b99 + 3, "A6 2C894 fixed: counts follow");
        check(strcmp(g_textp, "0") == 0, "A6 2C894 the caller's text ends as the count");
        g_original = 0;
    }
    /* the empty list (mode 7, nothing marked): fixed no f3F624(-1); --original writes 15 into 0xFFB5F */
    for (x = 0; x < 2; x++) {
        g_original = x;
        g_ds = g_start;
        memcpy(g_ds, g_base[0], DS_SIZE);
        memset(g_ds + (0xF7984u - DS_BASE), 0, 157);
        w8(0xFFB5Fu, 0);
        memset(&c, 0, sizeof c);
        c.fn = F2C894; c.a[0] = 12; c.a[1] = 2; c.a[2] = 0; c.a[3] = 0; c.a[4] = 7; c.seed = 23;
        run(&c, g_start, "A6 2C894 empty list");
        g_ds = P;
        check(rb(0xFFB5Fu) == (x ? 15 : 0), "A6 2C894 empty list: 0xFFB5F");
        g_original = 0;
    }

    /* A7. 0x2D1C0 with a stale count: o = 13 holds 2 tokens, k = 5: fixed it moves 2 and ends */
    g_ds = g_start;
    memcpy(g_ds, g_base[0], DS_SIZE);
    clear_nation_units(13);
    for (t = 0; t < 157; t++)
        if (tokens_of(t, 5) >= 2) {
            tok_off(t, 5, 2);
            tok_on(t, 13, 2);
            break;
        }
    if (t == 157) {
        t = find_site(2, -1);
        tok_on(t, 13, 2);
    }
    memset(&c, 0, sizeof c);
    c.fn = F2D1C0; c.a[0] = 5; c.a[1] = 5; c.a[2] = 13; c.a[3] = 7; c.seed = 29;
    run(&c, g_start, "A7 2D1C0 stale count");
    g_ds = P;
    check(strstr(g_logs[0], "CUT") == NULL && tokens_of(t, 13) == 0 && tokens_of(t, 7) >= 2, "A7 2D1C0 ends (fixed)");

    printf("A. hand vectors: %d fail\n", g_fail - fail0);
}

/* ---- part C: the real saves -------------------------------------------------------------------------------------- */

static void part_c(int slot, const char *name)
{
    Call c;
    int n, s, i, t, k, fail0 = g_fail, runs = 0, units, sum;
    char what[96];

    for (n = 1; n <= 9; n++) {
        g_ds = g_base[slot];
        if (rb(NAT(n) + 0x9Au) == 0)
            continue;
        /* Famine 10 and 8, Epidemic 16 */
        for (i = 0; i < 3; i++) {
            memcpy(g_start, g_base[slot], DS_SIZE);
            g_ds = g_start;
            units = 5 * sb(NAT(n) + 0x51u) + sb(NAT(n) + 0x98u);
            memset(&c, 0, sizeof c);
            c.fn = F260C4; c.a[0] = n; c.a[2] = i < 2 ? 0x15 : 0x1B; c.a[3] = -1; c.seed = 100u + (uint32_t)(n * 7 + i);
            c.a[1] = i == 0 ? (units < 10 ? units : 10) : i == 1 ? (units < 8 ? units : 8) : (units < 16 ? units : 16);
            if (c.a[1] <= 0 || (g_original && !no_hang_260C4(n, c.a[2], -1)))
                continue;
            memcpy(g_start, g_base[slot], DS_SIZE);
            snprintf(what, sizeof what, "C %s n %d", name, n);
            run(&c, g_start, what);
            runs++;
            if (slot == 0 && !g_original && i == 0) {
                g_ds = P;
                printf("   %s %s Famine %d: cities %d -> %d, board %d -> %d\n", name, nm(n), c.a[1],
                       (int)(int8_t)g_base[slot][NAT(n) + 0x51u - DS_BASE], sb(NAT(n) + 0x51u),
                       (int)(int8_t)g_base[slot][NAT(n) + 0x98u - DS_BASE], sb(NAT(n) + 0x98u));
            }
        }
        /* Flood on each plain, the units as 0x2D4F0 counts them */
        for (s = 0; s < 5; s++) {
            memcpy(g_start, g_base[slot], DS_SIZE);
            g_ds = g_start;
            units = 0;
            for (i = 0; i < (int16_t)rw(0xBEE0Au + 22u * (uint32_t)s); i++) {
                t = (int16_t)rw(0xBEE10u + 22u * (uint32_t)s + 2u * (uint32_t)i);
                if (city_of(t, n) && (rb(TR(t) + 1u) & 0x80))
                    units += 5;
                units += tokens_of(t, n);
            }
            if (units == 0)
                continue;
            w16(0xBEE08u + 22u * (uint32_t)s, units);
            memset(&c, 0, sizeof c);
            c.fn = F260C4; c.a[0] = n; c.a[1] = units < 17 ? units : 17; c.a[2] = 0x19; c.a[3] = s;
            c.seed = 200u + (uint32_t)(n * 5 + s);
            snprintf(what, sizeof what, "C %s n %d Flood %d", name, n, s);
            run(&c, g_start, what);
            runs++;
        }
        /* the secondary assignment as Famine sets it up, Grain */
        memcpy(g_start, g_base[slot], DS_SIZE);
        g_ds = g_start;
        sum = 0;
        for (i = 0; i <= 9; i++) {
            k = 0;
            if (i >= 1 && i != n && rb(NAT(i) + 0x9Au) != 0) {
                k = 5 * sb(NAT(i) + 0x51u) + sb(NAT(i) + 0x98u);
                if (k > 8)
                    k = 8;
            }
            w16(0xF796Cu + 2u * (uint32_t)i, k);
            sum += k;
        }
        if (sum > 20) {
            memset(&c, 0, sizeof c);
            c.fn = F27518; c.a[0] = n; c.a[1] = -1; c.a[2] = 20; c.a[3] = 0x15; c.seed = 300u + (uint32_t)n;
            snprintf(what, sizeof what, "C %s n %d secondary", name, n);
            run(&c, g_start, what);
            runs++;
        }
        memcpy(g_start, g_base[slot], DS_SIZE);
        g_ds = g_start;
        w8(0xF547Cu + (uint32_t)n, 10);
        memset(&c, 0, sizeof c);
        c.fn = F27850; c.a[0] = n; c.seed = 400u + (uint32_t)n;
        snprintf(what, sizeof what, "C %s n %d Grain", name, n);
        run(&c, g_start, what);
        runs++;
        /* the river among all five plains */
        memcpy(g_start, g_base[slot], DS_SIZE);
        g_ds = g_start;
        w32(LIST_N, 5);
        for (i = 0; i < 5; i++)
            w32(LIST + 4u * (uint32_t)i, d32(0xBEDE0u + 4u * (uint32_t)n) == 4 ? 4 - i : i);
        memset(&c, 0, sizeof c);
        c.fn = F2E45C; c.a[0] = n; c.seed = 500u + (uint32_t)n;
        run(&c, g_start, "C river");
        runs++;
        if (slot == 0 && !g_original)
            printf("   %s %s: own river %d, picks %d of the five\n", name, nm(n), d32(0xBEDE0u + 4u * (uint32_t)n), g_retp);
        /* Civil War: the victim's first faction, then the annexing of what it holds */
        for (i = 0; i < 2; i++) {
            memcpy(g_start, g_base[slot], DS_SIZE);
            g_ds = g_start;
            memset(&c, 0, sizeof c);
            c.fn = F2C190; c.a[0] = n; c.a[1] = i ? 20 : 15; c.a[2] = n; c.a[3] = 12; c.a[4] = 4; c.a[5] = i;
            snprintf(c.text, sizeof c.text, "%s is the victim of Civil War. Choosing units for the first faction.", nm(n));
            c.seed = 600u + (uint32_t)(n * 2 + i);
            snprintf(what, sizeof what, "C %s n %d faction", name, n);
            run(&c, g_start, what);
            runs++;
            /* faction 12 as the port left it: its Military losses, then the annexing by nation 1 + n % 9 */
            memcpy(g_start, P, DS_SIZE);
            g_ds = g_start;
            for (t = 0; t < 157; t++)
                w8(0xF7984u + (uint32_t)t, tokens_of(t, 12) > 0 || city_of(t, 12));
            memset(&c, 0, sizeof c);
            c.fn = F2C894; c.a[0] = 12; c.a[1] = 5; c.a[2] = 2; c.a[3] = i; c.a[4] = i ? 7 : 9; c.seed = 700u + (uint32_t)n;
            snprintf(c.text, sizeof c.text, "%s choosing MILITARY losses for First Faction.", nm(n));
            run(&c, g_start, "C military");
            runs++;
            memcpy(g_start, P, DS_SIZE);
            g_ds = g_start;
            k = sb(NAT(12) + 0x51u);
            memset(&c, 0, sizeof c);
            c.fn = F2D10C; c.a[0] = n; c.a[1] = k > 1 ? k - 1 : k; c.a[2] = 12; c.a[3] = 1 + n % 9; c.seed = 800u + (uint32_t)n;
            run(&c, g_start, "C annex cities");
            runs++;
            memcpy(g_start, P, DS_SIZE);
            g_ds = g_start;
            sum = list_tokens(12);
            memset(&c, 0, sizeof c);
            c.fn = F2D1C0; c.a[0] = n; c.a[1] = sum > 1 ? sum - 1 : sum; c.a[2] = 12; c.a[3] = 1 + n % 9; c.seed = 900u + (uint32_t)n;
            run(&c, g_start, "C annex tokens");
            runs++;
        }
    }
    printf("C. %s%s: %d runs, %d fail\n", name, g_original ? " --original" : "", runs, g_fail - fail0);
}

/* ---- part D: mutations ------------------------------------------------------------------------------------------- */

static void part_d(void)
{
    static const struct { int mut, fn; const char *what; } muts[] = {
        { 1, F260C4, "Famine era threshold 3 -> 4" },        { 2, F260C4, "Epidemic per 4 -> 5" },
        { 3, F260C4, "compaction without the hole move" },   { 4, F260C4, "Flood capacity from the count word" },
        { 5, F260C4, "pass 1 takes lim - 1" },               { 6, F260C4, "priority sort descending" },
        { 7, F260C4, "stock not raised" },                   { 8, F27518, "share total / 2" },
        { 9, F27518, "ties swapped" },                       { 10, F27850, "the cards read where meant" },
        { 11, F2E45C, "Iberia flag >= 3" },                  { 12, F2C298, "retry from candidate 0" },
        { 13, F2C298, "Iberia rows kept in 3-player games" },{ 14, F2C894, "exact with ov, not |ov|" },
        { 15, F2C894, "faction counts not updated (fixed)" },{ 16, F2D1C0, "match reset per area" },
        { 17, F2D10C, "mark not written" },
    };
    int i, j, caught = 0, made, f;
    Call c;
    int keep_fail = g_fail, keep_tests = g_tests;

    g_quiet = 1;
    for (i = 0; i < (int)(sizeof muts / sizeof muts[0]); i++) {
        g_mut = muts[i].mut;
        f = g_fail;
        for (j = 0, made = 0; made < 1500 && j < 30000 && g_fail == f; j++) {
            g_ds = g_start;
            if (!build_random(&c, muts[i].fn))
                continue;
            made++;
            run(&c, g_start, "D");
        }
        if (muts[i].fn == F2C298 && g_fail == f)   /* through 0x2C190 too */
            for (j = 0, made = 0; made < 1500 && j < 30000 && g_fail == f; j++) {
                g_ds = g_start;
                if (!build_random(&c, F2C190))
                    continue;
                made++;
                run(&c, g_start, "D");
            }
        printf("   D mutation %2d (%s): %s\n", muts[i].mut, muts[i].what, g_fail > f ? "caught" : "NOT CAUGHT");
        if (g_fail > f)
            caught++;
    }
    g_mut = 0;
    g_quiet = 0;
    g_fail = keep_fail;
    g_tests = keep_tests;
    check(caught == (int)(sizeof muts / sizeof muts[0]), "D every mutation caught");
    printf("D. %d of %d mutations caught\n", caught, (int)(sizeof muts / sizeof muts[0]));
}

/* ---- main ---------------------------------------------------------------------------------------------------------- */

static int load_save(const char *path, int slot)
{
    FILE *fp = fopen(path, "rb");
    int16_t ver = 0;
    char desc[64];
    if (fp == NULL) {
        printf("   cannot open %s (skipped)\n", path);
        return 0;
    }
    memcpy(g_ds, g_ds0, DS_SIZE);
    f5F05C(&ver, desc, fp);
    fclose(fp);
    memcpy(g_base[slot], g_ds, DS_SIZE);
    printf("   %s: version %d \"%.40s\"\n", path, ver, desc);
    return 1;
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:/work/advciv_run";
    const char *playdir = argc > 2 ? argv[2] : "C:/work/advciv_play";
    char cwd[400], path[400];
    int i, k, n;
    uint8_t *own;

    if (state_init(gamedir) != 0)
        return 1;
    if (getcwd(cwd, sizeof cwd) == NULL)
        cwd[0] = 0;
    if (chdir(gamedir) == 0) {
        f12394();
        (void)chdir(cwd);
    }
    own = g_ds;
    g_logs[0] = malloc(LOGSZ);
    g_logs[1] = malloc(LOGSZ);
    g_ds0 = malloc(DS_SIZE);
    P = malloc(DS_SIZE);
    Mm = malloc(DS_SIZE);
    g_start = malloc(DS_SIZE);
    for (i = 0; i < 12; i++)
        g_base[i] = malloc(DS_SIZE);
    if (g_logs[0] == NULL || g_logs[1] == NULL || g_ds0 == NULL || P == NULL || Mm == NULL || g_start == NULL)
        return 1;
    g_log = g_logs[0]; g_len = &g_lens[0];
    memcpy(g_ds0, g_ds, DS_SIZE);

    printf("   the data object's defaults: Iberia's rows 0xC0638.. %d %d %d %d / 0xC06CC.. %d %d %d %d %d; 0xBEDE0:",
           d32(0xC0638u), d32(0xC063Cu), d32(0xC0640u), d32(0xC0644u), d32(0xC06CCu), d32(0xC06D0u), d32(0xC06D4u),
           d32(0xC06D8u), d32(0xC06DCu));
    for (n = 0; n < 10; n++)
        printf(" %d", d32(0xBEDE0u + 4u * (uint32_t)n));
    printf("\n   factions' rows read by 0x2C894: 0xC06D8.. %d %d %d %d, 0xC06E8.. %d %d %d %d; 0xC0794.. %d %d %d %d %d, "
           "0xC07A8.. %d %d %d %d %d\n", d32(0xC06D8u), d32(0xC06DCu), d32(0xC06E0u), d32(0xC06E4u), d32(0xC06E8u),
           d32(0xC06ECu), d32(0xC06F0u), d32(0xC06F4u), d32(0xC0794u), d32(0xC0798u), d32(0xC079Cu), d32(0xC07A0u),
           d32(0xC07A4u), d32(0xC07A8u), d32(0xC07ACu), d32(0xC07B0u), d32(0xC07B4u), d32(0xC07B8u));

    g_nbase = 0;
    snprintf(path, sizeof path, "%s/SAVGAMEA.DAT", playdir);
    if (load_save(path, g_nbase))
        g_nbase++;
    snprintf(path, sizeof path, "%s/savgame0.dat", gamedir);
    if (load_save(path, g_nbase))
        g_nbase++;
    for (k = 3; k <= 9; k++) {
        snprintf(path, sizeof path, "%s/savgame%d.dat", gamedir, k);
        if (load_save(path, g_nbase))
            g_nbase++;
    }
    if (g_nbase == 0)
        return 1;

    part_a();

    for (k = 0; k < g_nbase; k++) {
        snprintf(path, sizeof path, "slot %d%s", k, k == 0 ? " (SAVGAMEA.DAT)" : k == 1 ? " (savgame0.dat)" : "");
        part_c(k, path);
    }
    g_original = 1;
    for (k = 0; k < g_nbase; k++) {
        snprintf(path, sizeof path, "slot %d", k);
        part_c(k, path);
    }
    g_original = 0;

    printf("B. default: %d fail\n", part_b(3000, "B"));
    g_original = 1;
    printf("B. --original: %d fail\n", part_b(1500, "B-orig"));
    g_original = 0;
    for (k = 0; k < NFN; k++)
        printf("   %s: %ld runs, %ld fail\n", fn_name[k], g_runs[k], g_fails[k]);
    printf("   coverage (model):");
    for (k = 0; k < 23; k++)
        printf(" %d:%ld", k, g_cov[k]);
    printf("\n   cuts %d, not_ported %d, >160 plans %ld\n", g_cuts, g_np, g_cov[39]);

    part_d();

    g_ds = own;
    printf("%d checks, %d fail\n", g_tests, g_fail);
    return g_fail == 0 ? 0 : 1;
}
