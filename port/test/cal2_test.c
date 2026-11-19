/* cal2_test [GAMEDIR [SAVEDIR]]: unit checks for module CAL2 (cal2.c) that need no game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init; default C:\work\advciv_run, only read) and the saves
   savgame0.dat (start of MOVEMENT R4) and savgame1.dat (R4, all human); SAVEDIR holds SAVGAMEA.DAT and savgameb.dat
   (default C:\work\advciv_play, only read). All four are read with saveload.c's f5F05C as the game does.

   cal2.c is built with -DCAL2_TEST: its calls into code that is not ported yet (the computers' 0x27850, 0x260C4,
   0x27518, 0x2E45C, 0x2E1D0; ANIM's 0x7A4C4; CAL3's 0x1EAEC) come to recorders here, which trace the call, act out a
   plausible effect where the caller depends on one (0x27518 fills the losses 0xF547C; 0x2E45C / 0x2E1D0 / 0x1EAEC
   return a listed entry) and return.

   Every other routine cal2.c calls is a stub below that appends one line to a trace (drawing included); those with
   data effects apply them on the data object g_ds: rules.c's city removal / reduction (f78F04, f78FEC), move.c's
   token removal (f57CE8 with f581C4's marker and list clean-up), the choice list (cal1.c's f28CFC for the kinds used
   here), the borders, the clamp f79A9C, and the dialog record edits (append / remove on the panel 0xF596C, enable,
   disable, x, y, style), so the screens' button states are real data. Clicks (f78BA4, and the mouse of 0x29550's own
   loop) come from a script: a seeded random choice among what the screen offers (only ENABLED buttons, as the real
   click wait returns; map clicks on listed entries; the arrows; codes that do nothing), the same seed for the port and
   the model; a run that polls more than 4000 times is cut off ("CUT") identically on both sides.

   The model is a second implementation of the eight routines, written from the disassembly (not from cal2.c): it
   runs on a copy of the data object (g_ds pointed at the copy), calls the same stubs and recorders, and so writes the
   trace it expects. After each run the two traces must be equal and the whole data object must equal the model's.
     A. hand vectors with the expected values written out (Famine's losses, Grain, secondary totals and both screens;
        the Grain arrows' limits; NationLosesUnits' REMOVE ONE / ALL / REDUCE / RESTART arithmetic per kind, the
        TARGET box, FINISHED only at the target, the two BUG(1995) fixes for an empty list; 0x291D0's columns and
        early return; 0x29550's +/- limits, auto-repeat and CONTINUE; Flood's plain choice, ties, the coastal city,
        the "unaffected" message, the <= 10 and > 10 branches, the BUG(1995) human flag, the Pirates, TIGRIS;
        Epidemic's losses, the f2EC38 target, Medicine / Roadbuilding on the secondary victims, MEDICINE's message);
     B. 0x27FCC alone on 6,000 random states (kinds 0x15, 0x19, 0x1B, 7, 8, 9; targets; restarts);
     C. 0x291D0 / 0x29550 / 0x27CEC on 3,000 random states;
     D. Famine, Flood and Epidemic on 3,000 random states each (humans, computers, PBEM bit, Grain, advances, plains,
        Pirates, the trader);
     E. the three calamities on the four real saves, every victim in play, 100 random runs per save and calamity.
   Every random part runs with and without --original.
   Prints each failure (the first differing trace line or byte), the counts per part, and exits 0 when all pass.

   Build (see the module report): clang -std=c11 -DCAL2_TEST port/test/cal2_test.c port/src/cal2.c
          port/src/saveload.c port/src/state.c port/src/le_image.c -Iport/src -o cal2_test.exe */

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cal1.h"
#include "cal2.h"
#include "fg.h"
#include "game.h"
#include "helpers2.h"
#include "kit.h"
#include "map.h"
#include "mapdata.h"
#include "move.h"
#include "panel.h"
#include "rules.h"
#include "saveload.h"
#include "start.h"
#include "state.h"
#include "ui.h"

/* ---- addresses (from the listing, written out here) ---------------------------------------------------------- */

#define NAT(n)      (0xF14CCu + 199u * (uint32_t)(n))
#define AR(r)       (0xE31B8u + 69u * (uint32_t)(r))
#define TS(t)       (0xE2C18u + 3u * (uint32_t)(t))
#define TR0(t)      (0xE1638u + 35u * (uint32_t)(t))
#define TR1(t)      (0xE1639u + 35u * (uint32_t)(t))
#define LIST        0xF7CA0u
#define LIST_N      0xF7C9Cu
#define BORDERS     0xFFB60u
#define CREC(k)     (0xF77E4u + 3u * (uint32_t)(k))
#define SECC        0xF796Cu
#define SECL        0xF547Cu
#define GRAIN       0xF409Au
#define PANEL       0xF596Cu
#define PBASE       0xF5944u
#define PL(s)       (0xBEE08u + 22u * (uint32_t)(s))
#define MX          0xE1628u
#define MY          0xE162Cu
#define MB          0xE1630u

/* ---- byte access on the current data object (g_ds: the port's or the model's copy) ---------------------------- */

static uint8_t *R;          /* the port's data object (state_init's) */
static uint8_t *M;          /* the model's copy */
static uint8_t *g_pristine;

static int      rb(uint32_t a) { return g_ds[a - DS_BASE]; }
static int      sb(uint32_t a) { return (int8_t)g_ds[a - DS_BASE]; }
static int      u16(uint32_t a) { return g_ds[a - DS_BASE] | g_ds[a + 1 - DS_BASE] << 8; }
static int      s16(uint32_t a) { return (int16_t)u16(a); }
static int32_t  s32(uint32_t a)
{
    return (int32_t)((uint32_t)g_ds[a - DS_BASE] | (uint32_t)g_ds[a + 1 - DS_BASE] << 8 |
                     (uint32_t)g_ds[a + 2 - DS_BASE] << 16 | (uint32_t)g_ds[a + 3 - DS_BASE] << 24);
}
static void w8(uint32_t a, int v) { g_ds[a - DS_BASE] = (uint8_t)v; }
static void w16(uint32_t a, int v) { g_ds[a - DS_BASE] = (uint8_t)v; g_ds[a + 1 - DS_BASE] = (uint8_t)(v >> 8); }
static void w32(uint32_t a, int32_t v)
{
    uint32_t u = (uint32_t)v;
    g_ds[a - DS_BASE] = (uint8_t)u; g_ds[a + 1 - DS_BASE] = (uint8_t)(u >> 8);
    g_ds[a + 2 - DS_BASE] = (uint8_t)(u >> 16); g_ds[a + 3 - DS_BASE] = (uint8_t)(u >> 24);
}
static const char *str(uint32_t a) { return (const char *)(g_ds + (a - DS_BASE)); }
static const char *nm(int n) { return str((uint32_t)s32(0xC768Cu + 4u * (uint32_t)n)); }
static const char *snm(int n) { return str((uint32_t)s32(0xC7664u + 4u * (uint32_t)n)); }
static const char *tn(int t) { return str((uint32_t)s32(0xC77D8u + 4u * (uint32_t)t)); }

/* ---- traces --------------------------------------------------------------------------------------------------- */

#define LOGSZ (1u << 22)
static char *g_real, *g_model, *g_log;
static size_t g_real_len, g_model_len, *g_len;

static void tr(const char *fmt, ...)
{
    va_list ap;
    int k;
    if (*g_len + 600 >= LOGSZ)
        return;
    va_start(ap, fmt);
    k = vsnprintf(g_log + *g_len, LOGSZ - *g_len, fmt, ap);
    va_end(ap);
    if (k > 0)
        *g_len += (size_t)k;
    g_log[(*g_len)++] = '\n';
    g_log[*g_len] = 0;
}

/* ---- the click script, the cut -------------------------------------------------------------------------------- */

static uint32_t g_srng;
static int g_steps, g_np, g_cuts, g_unatt;
static uint32_t g_screen;
static jmp_buf g_jb;
static int g_forced[64], g_nforced, g_iforced;   /* hand vectors: these clicks first, then the script */
static int g_fmouse[64][2], g_nfm, g_ifm;        /* hand vectors: 0x29550's clicks as button ids (or -1: nowhere) */
static int g_yes = -1;                           /* hand vectors: the YES/NO answer (-1 = the script) */
static long g_cov[12];  /* port side: REMOVE ONE, REMOVE ALL, REDUCE, RESTART, FINISHED, 29550 +, 29550 -,
                           27CEC arrows, 291D0 shown, recorders 260C4, fixes 2 / 3 hit, cut */

static uint32_t srnd(uint32_t n)
{
    g_srng ^= g_srng << 13;
    g_srng ^= g_srng >> 17;
    g_srng ^= g_srng << 5;
    return n ? g_srng % n : 0;
}

static void cut_check(void)
{
    if (++g_steps > 4000) {
        tr("CUT");
        g_cuts++;
        longjmp(g_jb, 1);
    }
}

/* the first record of the panel 0xF596C with this id (0 if none) */
static uint32_t prec(int id)
{
    int i, c = s16(PANEL);
    for (i = 0; i < c && i < 50; i++)
        if (s16(PANEL + 2u + 20u * (uint32_t)i + 0x0Eu) == (int16_t)id)
            return PANEL + 2u + 20u * (uint32_t)i;
    return 0;
}

static int enabled(int id)
{
    uint32_t r = prec(id);
    return r != 0 && sb(r + 0x10) == 1;
}

/* whether the real click wait could return this code on the current screen (the button is there and enabled) */
static int offered(int code)
{
    int cnt = s32(LIST_N);
    switch (code) {
    case -100: return 1;
    case -1: return enabled(0x2A);
    case -9: return enabled(0x39);
    case -10: return enabled(0x3A);
    case -0x65: return enabled(0x3F);
    case -0x66: return enabled(0x40);
    case -0xE: return enabled(0x3E);
    case -0x63: return enabled(0x41);
    default:
        if (code >= 0)
            return g_screen == 0xBF650u && code < cnt;
        return 1;   /* codes of other panel buttons (they do nothing on these screens) */
    }
}

static int pick(void)
{
    int cnt = s32(LIST_N), r;
    if (enabled(0x2A) && srnd(3) != 0)
        return -1;
    for (;;) {
        switch (srnd(16)) {
        case 0: r = -100; break;
        case 1: case 2: r = cnt > 0 ? (int)srnd((uint32_t)cnt) : -100; break;
        case 3: r = -9; break;
        case 4: r = -10; break;
        case 5: r = srnd(5) == 0 ? -0x63 : -100; break;
        case 6: case 7: r = -0x65; break;
        case 8: r = -0x66; break;
        case 9: r = -0xE; break;
        case 10: case 11: case 12: r = -1; break;
        case 13: r = -5; break;
        case 14: r = -0x64; break;
        default: r = -0x20; break;
        }
        if (offered(r))
            return r;
    }
}

/* ---- shared effects (on g_ds) --------------------------------------------------------------------------------- */

static int okt(int t) { return t >= 0 && t < 160; }
static int okn(int n) { return n >= 1 && n <= 13; }

static void eff_78F04(int t, int n)   /* rules.c's f78F04 (fixed: a city not in the list leaves the list) */
{
    uint32_t rec = NAT(n);
    int c, j, k;
    if (!okt(t) || !okn(n))
        return;
    w8(TS(t) + 2, rb(TS(t) + 2) & ~1);
    w8(TS(t), 0);
    w8(TS(t) + 1, 0xFF);
    c = sb(rec + 0x51);
    for (j = 0; j < c; j++)
        if (rb(rec + 0x52u + (uint32_t)j) == t + 1)
            break;
    if (j < c) {
        for (k = j; k < c - 1; k++)
            w8(rec + 0x52u + (uint32_t)k, rb(rec + 0x53u + (uint32_t)k));
        w8(rec + 0x51, c - 1);
        w8(rec + 0x52u + (uint32_t)(c - 1), 0);
    }
}

static void eff_list_add(int t, int n)   /* f57E28's list part: t + 1 into n's area list +0x60 / +0x61.. */
{
    uint32_t rec = NAT(n);
    int c = sb(rec + 0x60), j;
    for (j = 0; j < c; j++)
        if (rb(rec + 0x61u + (uint32_t)j) == t + 1)
            return;
    if (c >= 0 && c < 55) {
        w8(rec + 0x61u + (uint32_t)c, t + 1);
        w8(rec + 0x60, c + 1);
    }
}

static void eff_57E28(int t, int n, int k)
{
    uint32_t a;
    if (!okt(t) || !okn(n) || k <= 0)
        return;
    a = AR(t + 1);
    if (sb(a + 1u + (uint32_t)n) != n) {
        w8(a + 1, rb(a + 1) + 1);
        w8(a + 1u + (uint32_t)n, n);
    }
    w8(a + 0x0Eu + (uint32_t)n, rb(a + 0x0Eu + (uint32_t)n) + k);
    eff_list_add(t, n);
}

static void eff_581C4(int t, int n)   /* marker cleared and the list entry removed when no token is left */
{
    uint32_t a = AR(t + 1), rec = NAT(n);
    int c, j, k;
    if (sb(a + 1u + (uint32_t)n) == 0 || sb(a + 0x0Eu + (uint32_t)n) != 0)
        return;
    w8(a + 1, rb(a + 1) - 1);
    w8(a + 1u + (uint32_t)n, 0);
    c = sb(rec + 0x60);
    for (j = 0; j < c; j++)
        if (rb(rec + 0x61u + (uint32_t)j) == t + 1)
            break;
    if (j < c) {
        for (k = j; k < c - 1; k++)
            w8(rec + 0x61u + (uint32_t)k, rb(rec + 0x62u + (uint32_t)k));
        w8(rec + 0x61u + (uint32_t)(c - 1), 0xFF);
        w8(rec + 0x60, c - 1);
    }
}

static void eff_57CE8(int t, int n, int flag, int k)
{
    uint32_t a;
    if (!okt(t) || !okn(n))
        return;
    a = AR(t + 1);
    if (k > 0) {
        w8(a + 0x0Eu + (uint32_t)n, rb(a + 0x0Eu + (uint32_t)n) - k);
        if (flag == 1)
            w8(a + 0x24u + (uint32_t)n, rb(a + 0x24u + (uint32_t)n) - k);
    }
    eff_581C4(t, n);
}

static void eff_78FEC(int t, int n)
{
    uint32_t rec = NAT(n);
    int lim, k, i;
    if (!okt(t) || !okn(n))
        return;
    eff_78F04(t, n);
    lim = (rb(TR1(t)) >> 3) & 7;
    if (sb(AR(t + 1) + 1) <= 1)
        lim += sb(rec + 0x27);
    k = sb(rec + 0x99) < lim ? sb(rec + 0x99) : lim;
    for (i = 0; i < k; i++)
        eff_57E28(t, n, 1);
    if (k > 0) {
        w8(rec + 0x99, sb(rec + 0x99) - k);
        w8(rec + 0x98, sb(rec + 0x98) + k);
    }
}

/* cal1.c's 0x28CFC for the kinds this module passes (cal1.h): 0x15, 4, 9 cities or tokens; 0x1B cities or more
   than one token; 7 / 8 the byte tables; 0x19 the plain's territories with a flood-plain city or tokens */
static void eff_28CFC(int n, int kind, int s)
{
    int t, j, c;
    w32(LIST_N, 0);
    for (j = 0; j < 157; j++)
        w32(LIST + 4u * (uint32_t)j, -1);
    if (kind == 0x19) {
        if (s < 0 || s > 4)
            return;
        for (j = 0; j < s16(PL(s) + 2); j++) {
            t = s16(PL(s) + 8u + 2u * (uint32_t)j);
            if (((rb(TS(t) + 2) & 1) && rb(TS(t)) == n && (rb(TR1(t)) & 0x80)) || rb(AR(t + 1) + 0x0Eu + (uint32_t)n)) {
                c = s32(LIST_N);
                w32(LIST + 4u * (uint32_t)c, t);
                w32(LIST_N, c + 1);
            }
        }
        return;
    }
    for (t = 0; t < 157; t++) {
        int city = (rb(TS(t) + 2) & 1) && rb(TS(t)) == n;
        int tok = sb(AR(t + 1) + 0x0Eu + (uint32_t)n);
        int ok;
        if (kind == 0x15 || kind == 4 || kind == 9)
            ok = city || tok > 0;
        else if (kind == 0x1B)
            ok = city || tok > 1;
        else if (kind == 7)
            ok = rb(0xF7984u + (uint32_t)t) != 0;
        else if (kind == 8)
            ok = rb(0xF7A24u + (uint32_t)t) != 0;
        else
            ok = 0;
        if (ok) {
            c = s32(LIST_N);
            w32(LIST + 4u * (uint32_t)c, t);
            w32(LIST_N, c + 1);
        }
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
    uint32_t s = (uint32_t)s32(0xCAC5Cu) * 0x15A4E35u + 1u;
    w32(0xCAC5Cu, (int32_t)s);
    tr("rand %d", (int)((s >> 16) & 0x7FFF));
    return (int)((s >> 16) & 0x7FFF);
}

void f76F70(int n, uint8_t *cards) { (void)n; (void)cards; }   /* saveload.c's 0x726DC only (not reached here) */

#define TAG 0x80000000u
static uint8_t *s_blk[32];
static uint32_t s_len[32];

uint32_t load_heap_alloc(uint32_t size)
{
    uint32_t i;
    if (size == 0 || size > 0xFFFFFu)
        return 0;
    for (i = 0; i < 32; i++)
        if (s_blk[i] == NULL) {
            s_blk[i] = calloc(1, size);
            s_len[i] = size;
            tr("malloc %X", (unsigned)size);
            return TAG | (i + 1u) << 20;
        }
    return 0;
}

uint8_t *load_heap_ptr(uint32_t h, uint32_t n)
{
    uint32_t slot = (h & ~TAG) >> 20, off = h & 0xFFFFFu;
    if ((h & TAG) == 0 || slot < 1 || slot > 32 || s_blk[slot - 1] == NULL || off > s_len[slot - 1] ||
        n > s_len[slot - 1] - off)
        return NULL;
    return s_blk[slot - 1] + off;
}

void load_heap_free(uint32_t h)
{
    uint32_t slot = (h & ~TAG) >> 20;
    tr("free %X", (unsigned)h);
    if ((h & TAG) == 0 || slot < 1 || slot > 32)
        return;
    free(s_blk[slot - 1]);
    s_blk[slot - 1] = NULL;
}

static void heap_reset(void)
{
    int i;
    for (i = 0; i < 32; i++) {
        free(s_blk[i]);
        s_blk[i] = NULL;
    }
}

int fg_vbcopy(int minx, int maxx, int miny, int maxy, int newx, int newy, int source, int dest)
{
    tr("vbcopy %d %d %d %d %d %d %d %d", minx, maxx, miny, maxy, newx, newy, source, dest);
    return 0;
}
int fg_vbopen(int handle) { tr("vbopen %d", handle); return 0; }
void fg_vbclose(void) { tr("vbclose"); }
int fg_setcolor(int color) { tr("color %d", color); return 0; }
int fgf_width(const char *s, int n) { tr("width %s %d", s, n); return 8 * n; }
void f3CD6C(int8_t pic) { tr("bg %d", pic); }
void f7876C(void) { tr("phase"); }
void f3B05C(uint32_t dlg) { tr("dialog %X", (unsigned)dlg); }
void f459E4(int n) { tr("panel %d", n); }
void f3CBBC(void) { tr("paste"); }
void f3A924(void) { tr("area"); }
void f3A990(void) { tr("strip"); }
void f3A9F4(const char *text) { tr("left %s", text); }
void f3AB7C(const char *text) { tr("box %s", text); }
void f78A20(int t) { tr("wait %d", t); }
void f79270(int wait, int n, int t, const char *text, int anim) { tr("msg %d %d %d %s %d", wait, n, t, text, anim); }
void f3F670(void) { tr("clrb"); memset(g_ds + (BORDERS - DS_BASE), 0, 0xA0); }
void f3F624(int t, int c) { tr("border %d %d", t, c); w8(BORDERS + (uint32_t)t, c); }
void f3AF8C(uint32_t spr, int x, int y) { tr("sprite %X %d %d", (unsigned)spr, x, y); }
void f3A3A0(int x, int y, const char *s) { tr("right %d %d %s", x, y, s); }
void f3A044(int x, int y, const char *s) { tr("print %d %d %s", x, y, s); }
void f3A0B0(int x, int y, int w, const char *s) { tr("text %d %d %d %s", x, y, w, s); }
void f3C9D0(int x, int y, int w, int h, int flags) { tr("frame %d %d %d %d %d", x, y, w, h, flags); }
void f3DA60(int t) { tr("centre %d", t); }
void f3D52C(int dx, int dy) { tr("scroll %d %d", dx, dy); }
void f3D9FC(int n) { tr("home %d", n); }
void f1E69C(int k) { tr("resolved %d", k); if (g_ds == R) g_cov[11]++; }
void f78F04(int t, int n) { tr("78F04 %d %d", t, n); eff_78F04(t, n); }
void f78FEC(int t, int n) { tr("78FEC %d %d", t, n); eff_78FEC(t, n); }
void f57CE8(int t, int n, int flag, int k) { tr("57CE8 %d %d %d %d", t, n, flag, k); eff_57CE8(t, n, flag, k); }
void f28CFC(int n, int kind, int s) { tr("list %d %X %d", n, (unsigned)kind, s); eff_28CFC(n, kind, s); }
int f2EC38(int n, int L)   /* helpers2.c's (tested there): a plausible value from the record, traced */
{
    int r = L > 4 ? (L + sb(NAT(n) + 0x51)) % 4 : 0;
    tr("2EC38 %d %d -> %d", n, L, r);
    return r;
}
int f207B4(int p, int v, int kind)   /* cal1.c's chooser: v's first coastal city */
{
    int t;
    tr("207B4 %d %d %X", p, v, (unsigned)kind);
    cut_check();
    for (t = 0; t < 157; t++)
        if ((rb(TS(t) + 2) & 1) && rb(TS(t)) == v && (rb(TR0(t)) & 2))
            return t;
    return 0;
}
void f79A9C(int32_t *p)
{
    if (s32(LIST_N) <= *p)
        *p = s32(LIST_N) - 1;
    if (*p < 0)
        *p = 0;
    tr("clamp %d", (int)*p);
}
int f795A8(const char *text)
{
    int r = g_yes != -1 ? g_yes : (srnd(3) == 0 ? -0x62 : -0x61);
    tr("yesno %s %d", text, r);
    return r;
}
int f7BA64(void)
{
    int r = g_unatt ? (int)srnd(2) : 0;
    tr("unattended %d", r);
    return r;
}

/* the dialog records (panel.c / ui_dialog.c's edits, on the data) */
static uint32_t drec(uint32_t dlg, int id)
{
    int i, c = s16(dlg);
    for (i = 0; i < c && i < 50; i++)
        if (s16(dlg + 2u + 20u * (uint32_t)i + 0x0Eu) == (int16_t)id)
            return dlg + 2u + 20u * (uint32_t)i;
    return 0;
}
void f7853C(uint32_t dlg)
{
    int c = s16(PANEL), k = s16(dlg);
    tr("append %X", (unsigned)dlg);
    g_screen = dlg;
    if (c >= 0 && k > 0 && c + k <= 50) {
        memcpy(g_ds + (PANEL + 2u + 20u * (uint32_t)c - DS_BASE), g_ds + (dlg + 2u - DS_BASE), 20u * (uint32_t)k);
        w16(PANEL, c + k);
    }
}
void f78664(uint32_t dlg)
{
    int c = s16(PANEL), k = s16(dlg), id = s16(dlg + 2u + 0x0Eu), i;
    tr("remove %X", (unsigned)dlg);
    g_screen = 0;
    for (i = 0; i < c && i < 50; i++)
        if (s16(PANEL + 2u + 20u * (uint32_t)i + 0x0Eu) == id)
            break;
    if (i >= c || i >= 50)
        return;
    if (i + k < c)
        memmove(g_ds + (PANEL + 2u + 20u * (uint32_t)i - DS_BASE),
                g_ds + (PANEL + 2u + 20u * (uint32_t)(i + k) - DS_BASE), 20u * (uint32_t)(c - i - k));
    w16(PANEL, c - k);
}
void f3C314(uint32_t dlg, int id) { uint32_t r = drec(dlg, id); tr("on %X %X", (unsigned)dlg, (unsigned)id); if (r) w8(r + 0x10, 1); }
void f3C3D0(uint32_t dlg, int id) { uint32_t r = drec(dlg, id); tr("off %X %X", (unsigned)dlg, (unsigned)id); if (r) w8(r + 0x10, 0); }
int  f3BF30(uint32_t dlg, int id)
{
    uint32_t r = drec(dlg, id);
    int e = r ? sb(r + 0x10) : 0;
    tr("isen %X %X %d", (unsigned)dlg, (unsigned)id, e);
    return e;
}
void f3BFBC(uint32_t dlg, int id, int en) { uint32_t r = drec(dlg, id); tr("seten %X %X %d", (unsigned)dlg, (unsigned)id, en); if (r) w8(r + 0x10, en); }
void f3BB48(uint32_t dlg, int id, int x) { uint32_t r = drec(dlg, id); tr("setx %X %X %d", (unsigned)dlg, (unsigned)id, x); if (r) w16(r, x); }
void f3BC64(uint32_t dlg, int id, int y) { uint32_t r = drec(dlg, id); tr("sety %X %X %d", (unsigned)dlg, (unsigned)id, y); if (r) w16(r + 2, y); }
void f3C0D8(uint32_t dlg, int id, int st) { uint32_t r = drec(dlg, id); tr("style %X %X %d", (unsigned)dlg, (unsigned)id, st); if (r) w8(r + 0x11, st); }
void f3C5D4(uint32_t dlg, int id) { tr("press %X %X", (unsigned)dlg, (unsigned)id); }
int  f3FB94(int id)
{
    int r = -100;
    if (id >= 0xCC && id <= 0xD2) r = -0x67 - (id - 0xCC);
    else if (id >= 0xD3 && id <= 0xD9) r = -0x6E - (id - 0xD3);
    else if (id == 0x2A) r = -1;
    tr("dispatch %X -> %d", (unsigned)id, r);
    if (g_ds == R && r <= -0x67 && r >= -0x6D) g_cov[5]++;
    if (g_ds == R && r <= -0x6E && r >= -0x74) g_cov[6]++;
    return r;
}
void f775F0(void) { tr("release"); w32(MB, 0); }

/* 0x29550's mouse: a click on an enabled +/- or CONTINUE (scripted), or somewhere else */
void fg_mousepos(int *x, int *y, int *b)
{
    uint32_t r = 0;
    int i, c = s16(PANEL), want = -2, id, pickn, cand[50], nc = 0;
    cut_check();
    if (g_ifm < g_nfm) {
        want = g_fmouse[g_ifm][0];
        g_ifm++;
        r = want >= 0 ? prec(want) : 0;
        *b = 1;
    } else {
        *b = srnd(8) == 0 ? 0 : 1;
        pickn = (int)srnd(20);
        for (i = 0; i < c && i < 50; i++) {
            id = s16(PANEL + 2u + 20u * (uint32_t)i + 0x0Eu);
            if (pickn < 10 ? (id >= 0xCC && id <= 0xD2) : pickn < 15 ? (id >= 0xD3 && id <= 0xD9) : pickn < 18 ? id == 0x2A : 0)
                cand[nc++] = i;
        }
        if (nc > 0)
            r = PANEL + 2u + 20u * (uint32_t)cand[srnd((uint32_t)nc)];
    }
    if (r != 0) {
        *x = s16(r) + s16(r + 4) / 2;
        *y = s16(r + 2) + s16(r + 6) / 2;
    } else {
        *x = 600;
        *y = 470;
    }
    tr("mouse %d %d %d", *x, *y, *b);
}

int f78BA4(uint32_t dlg, int map)
{
    int r;
    cut_check();
    if (g_iforced < g_nforced) {
        r = g_forced[g_iforced++];
        if (!offered(r)) {
            tr("click %X %d %d not offered", (unsigned)dlg, map, r);
            r = -100;
        }
    } else if (g_screen == 0xBB800u)
        r = srnd(4) == 0 ? -100 : -1;
    else if (g_screen == 0xBF264u) {
        int c = (int)srnd(8);
        r = c < 3 ? -10 : c < 5 ? -9 : c < 7 ? -1 : -5;
    } else
        r = pick();
    tr("click %X %d %d", (unsigned)dlg, map, r);
    if (g_ds == R) {
        if (g_screen == 0xBF650u) {
            if (r == -0x65) g_cov[0]++;
            if (r == -0x66) g_cov[1]++;
            if (r == -0xE) g_cov[2]++;
            if (r == -0x63) g_cov[3]++;
            if (r == -1) g_cov[4]++;
        }
        if (g_screen == 0xBF264u && (r == -9 || r == -10)) g_cov[7]++;
        if (g_screen == 0xBB800u) g_cov[8]++;
    }
    return r;
}

/* the recorders of the calls that are not ported (cal2.c with -DCAL2_TEST) */
void cal2_test_f27850(int n) { tr("27850 %d", n); }
void cal2_test_f260C4(int n, int L, int kind, int s)
{
    tr("260C4 %d %d %X %d", n, L, (unsigned)kind, s);
    if (g_ds == R)
        g_cov[9]++;
}
void cal2_test_f27518(int n, int trader, int total, int kind)
{
    int m, rest = total, a;
    tr("27518 %d %d %d %X", n, trader, total, kind);
    memset(g_ds + (SECL - DS_BASE), 0, 14);
    for (m = 1; m <= 9; m++) {
        a = s16(SECC + 2u * (uint32_t)m);
        if (a > rest) a = rest;
        w8(SECL + (uint32_t)m, a);
        rest -= a;
    }
}
int cal2_test_f2E45C(int n)
{
    int r = s32(LIST_N) > 0 ? s32(LIST + 4u * (uint32_t)(s32(LIST_N) - 1)) : 0;
    tr("2E45C %d -> %d", n, r);
    return r;
}
int cal2_test_f2E1D0(int n, int c)
{
    int r = s32(LIST_N) > 0 ? s32(LIST) : 0;
    tr("2E1D0 %d %X -> %d", n, (unsigned)c, r);
    return r;
}
void cal2_test_f7A4C4(int anim) { tr("7A4C4 %d", anim); }
int cal2_test_f1EAEC(int n, int kind)
{
    int r = s32(LIST_N) > 0 ? s32(LIST + 4u * (srnd((uint32_t)s32(LIST_N)))) : 0;
    tr("1EAEC %d %X -> %d", n, (unsigned)kind, r);
    cut_check();
    return r;
}

/* ==== the model (g_ds == M), written from the disassembly ======================================================= */

static int m_hum(int n) { return (rb(0xE1624u) & 0x80) == 0 && sb(NAT(n) + 0x9A) == 3; }

static void m_bg3(void)
{
    f3CD6C(3);
    w8(0xEE168u, 3);
    fg_vbcopy(0, 639, 0, 479, 0, 479, s32(0xF52CCu), s32(0xF52C8u));
}

static void m_itoa(char *b, int v) { snprintf(b, 32, "%d", v); }

/* 0x27CEC */
static void m27CEC(int n)
{
    char b[100];
    int r;
    uint32_t used = GRAIN + 2u * (uint32_t)n, ls = SECL + (uint32_t)n;

    f7853C(0xBF264u);
    m_bg3();
    fg_vbopen(s32(0xF52C8u));
    f3AF8C(0xF51ACu, 296, 385);
    f3AF8C(0xF51B4u, 330, 385);
    fg_setcolor(5);
    f3A3A0(293, 389, "GRAIN");
    f3A044(346, 389, "USED");
    f3A3A0(303, 409, "LOSSES");
    f3A044(336, 409, "REDUCED TO");
    fg_vbclose();
    f7876C();
    snprintf(b, sizeof b, "%s - choose Grain reduction of Famine losses.", nm(n));
    f3A9F4(b);
    f3B05C(PANEL);
    w16(used, 0);
    f459E4(n);
    f3F670();
    f3D9FC(n);
top:
    fg_vbopen(s32(0xF52C8u));
    f3C9D0(310, 385, 19, 19, 5);
    fg_setcolor(5);
    m_itoa(b, s16(used));
    f3A0B0(310, 389, 19, b);
    f3C9D0(310, 405, 19, 19, 5);
    fg_setcolor(5);
    m_itoa(b, sb(ls) < 0 ? 0 : sb(ls));
    f3A0B0(310, 409, 19, b);
    fg_vbclose();
    f3CBBC();
    r = f78BA4(PANEL, 0);
    if (r == -1) {
        f78664(0xBF264u);
        return;
    }
    if (r == -9 && s16(used) > 0) {
        w16(used, s16(used) - 1);
        w8(ls, rb(ls) + 4);
    } else if (r == -10 && sb(ls) > 0 && s16(used) < sb(NAT(n) + 0x08)) {
        w16(used, s16(used) + 1);
        w8(ls, rb(ls) - 4);
    }
    goto top;
}

/* 0x27FCC */
static void m_snap(int save)
{
    uint8_t *a = load_heap_ptr((uint32_t)s32(0xEED74u), 0x2B65u);
    uint8_t *t = load_heap_ptr((uint32_t)s32(0xEED7Cu), 0x1E0u);
    uint8_t *v = load_heap_ptr((uint32_t)s32(0xEED78u), 0xAE2u);
    uint8_t *d = g_ds - DS_BASE;
    if (save) {
        if (a) memcpy(a, d + AR(0), 161 * 69);
        if (t) memcpy(t, d + TS(0), 160 * 3);
        if (v) memcpy(v + 199, d + NAT(1), 13 * 199);
    } else {
        if (a) memcpy(d + AR(0), a, 161 * 69);
        if (v) memcpy(d + NAT(1), v + 199, 13 * 199);
        if (t) memcpy(d + TS(0), t, 160 * 3);
    }
}

static int m_lim_agr(int t, int n) { return ((rb(TR1(t)) >> 3) & 7) + sb(NAT(n) + 0x27); }
static int m_tok(int t, int n) { return sb(AR(t + 1) + 0x0Eu + (uint32_t)n); }
static int m_cityat(int t) { return rb(TS(t) + 2) & 1; }

static void m_off1(int t, int n)
{
    f57CE8(t, n, 0, 1);
    w8(NAT(n) + 0x98, rb(NAT(n) + 0x98) - 1);
    w8(NAT(n) + 0x99, rb(NAT(n) + 0x99) + 1);
}

static void m27FCC(int n, int L, const char *text, int kind, int sub, int target)
{
    char b[100];
    int left, redraw, removed, r, i, t, emp, cut;
    int32_t sel;
    int civil = kind == 7 || kind == 8 || kind == 9;

    w32(0xEED74u, (int32_t)load_heap_alloc(0x2B65));
    w32(0xEED7Cu, (int32_t)load_heap_alloc(0x1E0));
    w32(0xEED78u, (int32_t)load_heap_alloc(0xAE2));
    m_snap(1);
    m_bg3();
    fg_vbopen(s32(0xF52C8u));
    f3AF8C(0xF51ACu, 251, 390);
    f3AF8C(0xF51B4u, 376, 390);
    fg_setcolor(5);
    f3A3A0(303, 374, "LEFT TO");
    f3A044(336, 374, "REMOVE");
    fg_vbclose();
    f3AB7C(text);
    f7876C();
    f3BB48(0xBF650u, 0x3F, civil ? 219 : 169);
    f3BB48(0xBF650u, 0x40, civil ? 319 : 269);
    f3C0D8(0xBF650u, 0x3E, civil ? 2 : 1);
    f3BFBC(0xBF650u, 0x3E, civil ? 0 : 1);
    f7853C(0xBF650u);
    f3B05C(PANEL);
    left = L;
    f28CFC(n, kind, sub);
    if (target != 0) {
        fg_vbopen(s32(0xF52C8u));
        fg_setcolor(5);
        f3A3A0(439, 374, "TARGET");
        f3C9D0(444, 370, 19, 19, 5);
        fg_setcolor(5);
        m_itoa(b, target);
        f3A0B0(444, 374, 19, b);
        fg_vbclose();
    }
    redraw = 1;
    sel = 0;
    removed = 0;
    for (;;) {
        emp = s32(LIST_N) <= 0;
        cut = !g_original && emp;
        t = s32(LIST + 4u * (uint32_t)sel);
        fg_vbopen(s32(0xF52C8u));
        f3C9D0(265, 390, 110, 19, 5);
        fg_setcolor(5);
        if (!cut)
            f3A0B0(265, 394, 110, tn(t));
        else if (g_ds == M)
            g_cov[10]++;
        f3C9D0(310, 370, 19, 19, 5);
        fg_setcolor(5);
        m_itoa(b, left);
        f3A0B0(310, 374, 19, b);
        fg_vbclose();
        f3F670();
        if (!cut)
            f3F624(t, 15);
        if (left == target) f3C314(PANEL, 0x2A); else f3C3D0(PANEL, 0x2A);
        {
            int fin = f3BF30(PANEL, 0x2A);
            int on = 0;
            if (fin == 0 && left > 0) {
                if (kind != 0x1B)
                    on = 1;
                else if (m_tok(t, n) > 1)
                    on = 1;
                else if (m_cityat(t))
                    on = 1;
            }
            if (on && cut)
                on = 0;
            if (on) { f3C314(PANEL, 0x3F); f3C314(PANEL, 0x40); }
            else { f3C3D0(PANEL, 0x3F); f3C3D0(PANEL, 0x40); }
        }
        if (kind != 7 && kind != 8) {
            if (f3BF30(PANEL, 0x2A) == 0 && m_cityat(t) && !cut)
                f3C314(PANEL, 0x3E);
            else
                f3C3D0(PANEL, 0x3E);
        }
        if (redraw) {
            f459E4(n);
            if (s32(LIST_N) > 0) f3DA60(t); else f3D52C(0, 0);
        }
        if (left < target) {
            f3C3D0(PANEL, 0x3F);
            f3C3D0(PANEL, 0x40);
            f3C3D0(PANEL, 0x3E);
            f3C3D0(PANEL, 0x2A);
        }
        f3CBBC();
        if (redraw) {
            cal2_test_f7A4C4(-600);
            redraw = 0;
        }
        r = f78BA4(PANEL, 1);
        if (r == -100)
            continue;
        if (r >= 0) {
            sel = r;
            redraw = 1;
            continue;
        }
        if (r == -1)
            break;
        switch (r) {
        case -9:
            if (sel > 0) { sel--; redraw = 1; }
            break;
        case -10:
            if (sel < s32(LIST_N) - 1) { sel++; redraw = 1; }
            break;
        case -0xE:
            f78FEC(t, n);
            left -= 5 - m_lim_agr(t, n);
            redraw = 1;
            break;
        case -0x63:
            snprintf(b, sizeof b, "Are you sure you want to Restart?");
            if ((rb(0xF097Cu) & 8) && f795A8(b) != -0x61)
                break;
            m_snap(0);
            left = L;
            f28CFC(n, kind, sub);
            sel = 0;
            redraw = 1;
            break;
        case -0x65:   /* REMOVE ONE */
            if (m_cityat(t)) {
                if (kind == 0x1B) {
                    f78FEC(t, n);
                    left -= 5 - m_lim_agr(t, n);
                    while (m_tok(t, n) > 1 && left != 0) { left--; m_off1(t, n); }
                } else {
                    f78F04(t, n);
                    left -= 5;
                    removed = 1;
                }
            } else if (kind != 0x1B || m_tok(t, n) > 1) {
                left--;
                m_off1(t, n);
                if (m_tok(t, n) == 0)
                    removed = 1;
            }
            redraw = 1;
            break;
        case -0x66:   /* REMOVE ALL */
            if (m_cityat(t)) {
                f78FEC(t, n);
                left -= 5 - m_lim_agr(t, n);
                if (kind == 0x1B) {
                    while (m_tok(t, n) > 1 && left != 0) { left--; m_off1(t, n); }
                } else {
                    while (m_tok(t, n) > 0 && left != 0) { left--; m_off1(t, n); }
                    removed = 1;
                }
            } else {
                for (;;) {
                    if (left == 0) break;
                    if (kind == 0x1B ? m_tok(t, n) <= 1 : rb(AR(t + 1) + 0x0Eu + (uint32_t)n) == 0) break;
                    left--;
                    m_off1(t, n);
                }
                if (m_tok(t, n) == 0)
                    removed = 1;
            }
            redraw = 1;
            break;
        default:
            break;
        }
        if (removed) {
            for (i = sel; i < s32(LIST_N) - 1; i++)
                w32(LIST + 4u * (uint32_t)i, s32(LIST + 4u * (uint32_t)(i + 1)));
            w32(LIST_N, s32(LIST_N) - 1);
            f79A9C(&sel);
            removed = 0;
        }
    }
    f78664(0xBF650u);
    load_heap_free((uint32_t)s32(0xEED74u));
    w32(0xEED74u, 0);
    load_heap_free((uint32_t)s32(0xEED7Cu));
    w32(0xEED7Cu, 0);
    load_heap_free((uint32_t)s32(0xEED78u));
    w32(0xEED78u, 0);
}

/* 0x291D0 / 0x29550 / 0x29C94 */
static int m_cols(int a, int b, int *lst)
{
    int m, c = 0;
    for (m = 1; m <= 9; m++)
        if (rb(NAT(m) + 0x9A) && m != a && m != b && s16(SECC + 2u * (uint32_t)m)) {
            if (lst) lst[c] = m;
            c++;
        }
    return c;
}

static void m_head(int m, int x, int w)
{
    const char *s = nm(m);
    fg_setcolor(5);
    if (fgf_width(s, (int)strlen(s)) >= w)
        s = snm(m);
    f3A0B0(x, 365, w, s);
    f3AF8C(0xF489Cu + 8u * (uint32_t)m, x + (w - u16(0xF48A0u + 8u * (uint32_t)m)) / 2, 380);
}

static void m291D0(int a, int b, const char *text)
{
    int lst[9], c, w, x0, j, r;
    char s[32];
    m_bg3();
    c = m_cols(a, b, lst);
    if (c == 0)
        return;
    w = (c == 2 ? 200 : 300) / c;
    x0 = c == 2 ? 220 : 170;
    f3A924();
    f3A9F4(text);
    f7876C();
    fg_vbopen(s32(0xF52C8u));
    fg_vbcopy(170, 469, 380, 418, 170, 418, s32(0xF52CCu), s32(0xF52C8u));
    fg_setcolor(5);
    for (j = 0; j < c; j++) {
        m_head(lst[j], x0 + w * j, w);
        f3C9D0(x0 + w * j + (w - 16) / 2, 408, 16, 14, 5);
        fg_setcolor(5);
        m_itoa(s, sb(SECL + (uint32_t)lst[j]));
        f3A0B0(x0 + w * j, 410, w, s);
    }
    fg_vbclose();
    f3BB48(0xBB800u, 0x2A, 265);
    f3BC64(0xBB800u, 0x2A, 430);
    f7853C(0xBB800u);
    f3B05C(PANEL);
    f3D52C(0, 0);
    f3CBBC();
    do {
        if (f7BA64()) {
            f78A20(s16(0xF40BAu) ? 1000 : 250);
            r = -1;
        } else
            r = f78BA4(PANEL, 0);
    } while (r != -1);
    f78664(0xBB800u);
}

static void m29C94(int a, int b, const int *lst, int w, int left, int x0)
{
    char s[32];
    int j, c = m_cols(a, b, NULL);
    fg_vbopen(s32(0xF52C8u));
    f3C9D0(239, 430, 18, 17, 5);
    m_itoa(s, left);
    fg_setcolor(5);
    f3A0B0(239, 433, 18, s);
    for (j = 0; j < c; j++) {
        f3C9D0(x0 + w * j + (w - 16) / 2, 408, 16, 14, 5);
        fg_setcolor(5);
        m_itoa(s, sb(SECL + (uint32_t)lst[j]));
        f3A0B0(x0 + w * j, 410, w, s);
    }
    fg_vbclose();
    if (m_hum(a)) {
        if (left > 0) f3C3D0(PANEL, 0x2A); else f3C314(PANEL, 0x2A);
        f3CBBC();
    }
}

static void m29550(int a, int b, int total, const char *text)
{
    int lst[9] = {0}, c, w, x0, j, m, left, sum, r, i, mx, my, mb, col, base;
    uint32_t q;
    m_bg3();
    c = m_cols(a, b, lst);
    w = (c == 2 ? 200 : 300) / c;
    x0 = c == 2 ? 220 : 170;
    memset(g_ds + (SECL - DS_BASE), 0, 14);
    f3A924();
    f3A9F4(text);
    f7876C();
    fg_vbopen(s32(0xF52C8u));
    fg_vbcopy(170, 469, 380, 418, 170, 418, s32(0xF52CCu), s32(0xF52C8u));
    fg_setcolor(5);
    w16(PANEL, s32(PBASE));
    base = s32(PBASE);
    for (j = 0; j < c; j++) {
        int x = x0 + w * j + (w - 16) / 2;
        m_head(lst[j], x0 + w * j, w);
        f3AF8C(0xF51BCu, x - 7, 410);
        f3AF8C(0xF51C4u, x + 18, 410);
        q = PANEL + 2u + 20u * (uint32_t)(base + 2 * j);
        memcpy(g_ds + (q - DS_BASE), g_ds + (0xC0614u - DS_BASE), 20);
        w16(q, x - 8);
        w16(q + 2, 410);
        w16(q + 14, u16(q + 14) + j);
        w8(q + 16, 1);
        w16(PANEL, u16(PANEL) + 1);
        q += 20;
        memcpy(g_ds + (q - DS_BASE), g_ds + (0xC0600u - DS_BASE), 20);
        w16(q, x + 18);
        w16(q + 2, 410);
        w16(q + 14, u16(q + 14) + j);
        w8(q + 16, 1);
        w16(PANEL, u16(PANEL) + 1);
    }
    for (sum = 0, m = 1; m <= 9; m++)
        sum += s16(SECC + 2u * (uint32_t)m);
    left = total < sum ? total : sum;
    f3A3A0(232, 434, "LEFT TO");
    f3A044(263, 434, "ASSIGN");
    fg_vbclose();
    f7853C(0xBFA3Cu);
    f3B05C(PANEL);
    f3D52C(0, 0);
redraw:
    m29C94(a, b, lst, w, left, x0);
    f775F0();
again:
    while (s32(MB) == 0) {
        fg_mousepos(&mx, &my, &mb);
        w32(MX, mx);
        w32(MY, my);
        w32(MB, mb);
    }
    r = -100;
    for (i = 0; i < s16(PANEL); i++) {
        q = PANEL + 2u + 20u * (uint32_t)i;
        if (sb(q + 16) == 1 && s16(q) <= s32(MX) && s32(MX) < s16(q) + s16(q + 4) && s16(q + 2) <= s32(MY) &&
            s32(MY) < s16(q + 2) + s16(q + 6)) {
            f3C5D4(PANEL, s16(q + 14));
            r = f3FB94(s16(q + 14));
            break;
        }
    }
    if (r == -1) {
        w16(PANEL, s32(PBASE));
        return;
    }
    if (r >= -0x6D && r <= -0x67) {
        if (left <= 0)
            goto redraw;
        col = -0x67 - r;
        if (sb(SECL + (uint32_t)lst[col]) < s16(SECC + 2u * (uint32_t)lst[col])) {
            w8(SECL + (uint32_t)lst[col], rb(SECL + (uint32_t)lst[col]) + 1);
            left--;
        }
        m29C94(a, b, lst, w, left, x0);
        f78A20(150);
        w32(MB, 0);
        goto again;
    }
    if (r >= -0x74 && r <= -0x6E) {
        col = -0x6E - r;
        if (sb(SECL + (uint32_t)lst[col]) > 0) {
            w8(SECL + (uint32_t)lst[col], rb(SECL + (uint32_t)lst[col]) - 1);
            left++;
        }
        m29C94(a, b, lst, w, left, x0);
        f78A20(150);
        w32(MB, 0);
        goto again;
    }
    goto redraw;
}

/* 0x25A38 Famine */
static int m_fam_units(int m) { return 5 * sb(NAT(m) + 0x51) + sb(NAT(m) + 0x98); }

static void m25A38(int k)
{
    char b[100];
    int n, v, loss, m, h;

    w16(0xBB3FAu, 169); w16(0xBB3FCu, 340); w16(0xBB3FEu, 300); w16(0xBB400u, 21);
    n = sb(CREC(k) + 2);
    w32(0xF40F8u, n);
    h = m_hum(n);
    snprintf(b, sizeof b, "Famine ravages %s.", nm(n));
    f79270(-500, n, -200, b, 1);
    if (h)
        snprintf(b, sizeof b, "%s, choose sites of Famine losses.", nm(n));
    else {
        f3A990();
        f3CBBC();
    }
    v = m_fam_units(n);
    w8(SECL + (uint32_t)n, v > 10 ? 10 : v);
    if (rb(NAT(n) + 0x24) && sb(NAT(n) + 0x08)) {
        if (h) m27CEC(n); else cal2_test_f27850(n);
    }
    loss = sb(SECL + (uint32_t)n) < 0 ? 0 : sb(SECL + (uint32_t)n);
    if (loss > 0) {
        if (h) m27FCC(n, loss, b, 0x15, 0, 0); else cal2_test_f260C4(n, loss, 0x15, -1);
    } else {
        snprintf(b, sizeof b, "Your usage of Grain has prevented Famine in %s.", nm(n));
        f79270(-500, n, -200, b, 1);
    }
    for (m = 1; m <= 9; m++) {
        w16(SECC + 2u * (uint32_t)m, 0);
        if (rb(NAT(m) + 0x9A) == 0 || m == n)
            continue;
        v = m_fam_units(m);
        w16(SECC + 2u * (uint32_t)m, v > 8 ? 8 : v);
    }
    for (v = 0, m = 1; m <= 9; m++)
        v += s16(SECC + 2u * (uint32_t)m);
    if (v <= 20) {
        for (m = 1; m <= 9; m++)
            w8(SECL + (uint32_t)m, rb(SECC + 2u * (uint32_t)m));
        m291D0(n, -1, "Assignments for Famine secondary losses");
    } else if (h) {
        snprintf(b, sizeof b, "%s, assign secondary losses for Famine.", nm(n));
        m29550(n, -1, 20, b);
    } else
        cal2_test_f27518(n, -1, 20, 0x15);
    for (m = 1; m <= 9; m++) {
        if (rb(SECL + (uint32_t)m) && rb(NAT(m) + 0x24) && sb(NAT(m) + 0x08)) {
            if (m_hum(m))
                m27CEC(m);
            else {
                cal2_test_f27850(m);
                f78A20(2000);
            }
        }
        if (sb(SECL + (uint32_t)m) > 0) {
            if (m_hum(m)) {
                snprintf(b, sizeof b, "Famine also ravages %s.\nChoose sites of losses.", nm(m));
                m27FCC(m, sb(SECL + (uint32_t)m), b, 0x15, 0, 0);
            } else
                cal2_test_f260C4(m, sb(SECL + (uint32_t)m), 0x15, -1);
        }
    }
    f1E69C(k);
    w16(0xBB400u, 35);
}

/* 0x2E59C Epidemic */
static int m_epi_units(int m) { return sb(NAT(m) + 0x98) - sb(NAT(m) + 0x60) + 4 * sb(NAT(m) + 0x51); }

static void m2E59C(int k)
{
    char b[100];
    int n, e, L, v, m, x, pos, h;

    w16(0xBB3FAu, 169); w16(0xBB3FCu, 340); w16(0xBB3FEu, 300); w16(0xBB400u, 21);
    n = sb(CREC(k) + 2);
    e = sb(CREC(k) + 1);
    w32(0xF40F8u, n);
    L = 16 - (rb(NAT(n) + 0x2D) ? 8 : 0) + (rb(NAT(n) + 0x28) ? 5 : 0);
    h = m_hum(n);
    if (h) {
        snprintf(b, sizeof b, "Epidemic ravages %s.\nChoose sites of losses.", nm(n));
        if (m_epi_units(n) < L)
            L = m_epi_units(n);
        x = sb(NAT(n) + 0x98) == sb(NAT(n) + 0x60) ? f2EC38(n, L) : 0;
        m27FCC(n, L, b, 0x1B, 0, x);
    } else {
        if (m_epi_units(n) < L)
            L = m_epi_units(n);
        cal2_test_f260C4(n, L, 0x1B, -1);
    }
    for (m = 1; m <= 9; m++) {
        w16(SECC + 2u * (uint32_t)m, 0);
        if (rb(NAT(m) + 0x9A) == 0 || m == n || m == e)
            continue;
        v = m_epi_units(m);
        w16(SECC + 2u * (uint32_t)m, v > 10 ? 10 : v);
    }
    for (v = 0, m = 1; m <= 9; m++)
        v += s16(SECC + 2u * (uint32_t)m);
    if (v <= 25) {
        for (m = 1; m <= 9; m++)
            w8(SECL + (uint32_t)m, rb(SECC + 2u * (uint32_t)m));
        m291D0(n, -1, h ? "Assignments for Epidemic secondary losses." : "Assignments for Epidemic secondary losses");
    } else if (h) {
        snprintf(b, sizeof b, "%s, assign secondary losses for Epidemic.", nm(n));
        m29550(n, e, 25, b);
    } else
        cal2_test_f27518(n, e, 25, 0x1B);
    for (m = 1; m <= 9; m++) {
        if (rb(SECL + (uint32_t)m) == 0)
            continue;
        pos = sb(SECL + (uint32_t)m) > 0;
        if (rb(NAT(m) + 0x2D)) {
            w8(SECL + (uint32_t)m, rb(SECL + (uint32_t)m) - 5);
            if (sb(SECL + (uint32_t)m) < 0)
                w8(SECL + (uint32_t)m, 0);
        }
        if (rb(NAT(m) + 0x28))
            w8(SECL + (uint32_t)m, rb(SECL + (uint32_t)m) + 5);
        if (sb(SECL + (uint32_t)m) > m_epi_units(m))
            w8(SECL + (uint32_t)m, m_epi_units(m));
        if (sb(SECL + (uint32_t)m) > 0) {
            if (m_hum(m)) {
                snprintf(b, sizeof b, "Epidemic also ravages %s.\nChoose sites of losses.", nm(m));
                x = sb(NAT(m) + 0x98) == sb(NAT(m) + 0x60) ? f2EC38(m, sb(SECL + (uint32_t)m)) : 0;
                m27FCC(m, sb(SECL + (uint32_t)m), b, 0x1B, 0, x);
            } else
                cal2_test_f260C4(m, sb(SECL + (uint32_t)m), 0x1B, -1);
        } else if (pos) {
            snprintf(b, sizeof b, "%s saved from Epidemic by MEDICINE.", nm(m));
            f79270(-500, n, -200, b, 1);
        }
    }
    f1E69C(k);
    w16(0xBB400u, 35);
}

/* 0x2D4F0 Flood */
static const char *m_river(int s)
{
    const char *r = str((uint32_t)s32(PL(s) + 4));
    if (s == 3 && !g_original)
        return "TIGRIS-EUPHRATES";
    return r;
}

static int m_units(int s, int m, int *cnt)   /* 22-byte plain s, nation m: 5 per flood-plain city, else tokens */
{
    int j, t, u = 0;
    for (j = 0; j < s16(PL(s) + 2); j++) {
        t = s16(PL(s) + 8u + 2u * (uint32_t)j);
        if ((rb(TS(t) + 2) & 1) && rb(TS(t)) == m && (rb(TR1(t)) & 0x80)) {
            u += 5;
            (*cnt)++;
        } else if (rb(AR(t + 1) + 0x0Eu + (uint32_t)m)) {
            u += sb(AR(t + 1) + 0x0Eu + (uint32_t)m);
            (*cnt)++;
        }
    }
    return u;
}

static void m2D4F0(int k)
{
    char b[100];
    int u[11], n, e, any = 0, s, t, best, ties, last, plain, L, m, tot, nn, dummy, c, wt;

    w16(0xBB400u, 21);
    memset(g_ds + (SECL - DS_BASE), 0, 14);
    memset(g_ds + (0xF548Cu - DS_BASE), 0, 14);
    n = sb(CREC(k) + 2);
    e = sb(CREC(k) + 1);
    w32(0xF40F8u, n);
    for (s = 0; s < 5; s++)
        w16(PL(s), m_units(s, n, &any));
    if (any == 0) {
        w32(LIST_N, 0);
        for (t = 0; t < 157; t++)
            if ((rb(TS(t) + 2) & 1) && rb(TS(t)) == n && (rb(TR0(t)) & 2)) {
                c = s32(LIST_N);
                w32(LIST + 4u * (uint32_t)c, t);
                w32(LIST_N, c + 1);
            }
        if (s32(LIST_N) == 0) {
            snprintf(b, sizeof b, "%s has no units on any flood plain or coastal cities, so is unaffected by Flood.", nm(n));
            f79270(-500, n, -200, b, 1);
            f1E69C(k);
            return;
        }
        snprintf(b, sizeof b, "%s has no units on any flood plain so a coastal city must be %s.", nm(n),
                 rb(NAT(n) + 0x2A) ? "reduced" : "eliminated");
        f79270(-500, n, -200, b, 1);
        t = m_hum(n) ? f207B4(n, n, 0x19) : cal2_test_f2E1D0(n, 0x19);
        if (rb(NAT(n) + 0x2A)) f78FEC(t, n); else f78F04(t, n);
        if (m_hum(n)) {
            f3D52C(0, 0);
            f78A20(2000);
        } else {
            snprintf(b, sizeof b, "%s %s a coastal city in %s.", nm(n), rb(NAT(n) + 0x2A) ? "reduces" : "eliminates", tn(t));
            if (rb(0xF0980u) & 4) wt = -500;
            else if (f7BA64() == 0) wt = 2000;
            else wt = s16(0xF40BAu) ? 1500 : 500;
            f79270(wt, n, t, b, 1);
        }
        f1E69C(k);
        return;
    }
    best = 0; ties = 0; last = 0;
    for (s = 0; s < 5; s++) {
        int v = s16(PL(s));
        if (v > best) { ties = 0; best = v; last = s; }
        else if (v == best) { ties++; last = s; }
    }
    plain = last;
    if (ties) {
        w32(LIST_N, 0);
        for (s = 0; s < 5; s++)
            if (s16(PL(s)) == best) {
                c = s32(LIST_N);
                w32(LIST + 4u * (uint32_t)c, s);
                w32(LIST_N, c + 1);
            }
        if (m_hum(n))
            plain = cal2_test_f1EAEC(n, 0x19);
        else if (s32(LIST_N) > 1)
            plain = cal2_test_f2E45C(n);
    }
    L = s16(PL(plain));
    if (L > (rb(NAT(n) + 0x2A) ? 7 : 17))
        L = rb(NAT(n) + 0x2A) ? 7 : 17;
    if (m_hum(n)) {
        snprintf(b, sizeof b, "The %s river Floods.\n%s, choose sites of losses.", m_river(plain), nm(n));
        m27FCC(n, L, b, 0x19, plain, 0);
    } else
        cal2_test_f260C4(n, L, 0x19, plain);
    for (m = 1; m <= 10; m++) {
        u[m] = 0;
        w16(SECC + 2u * (uint32_t)m, 0);
        if (m == n || rb(NAT(m) + 0x9A) == 0)
            continue;
        dummy = 0;
        u[m] = m_units(plain, m, &dummy);
        w16(SECC + 2u * (uint32_t)m, u[m]);
        if (s16(SECC + 2u * (uint32_t)m) > 10)
            w16(SECC + 2u * (uint32_t)m, 10);
        if (rb(NAT(m) + 0x2A) && s16(SECC + 2u * (uint32_t)m) > 7)
            w16(SECC + 2u * (uint32_t)m, 7);
    }
    for (tot = 0, nn = 0, m = 1; m <= 9; m++) {
        tot += s16(SECC + 2u * (uint32_t)m);
        nn += s16(SECC + 2u * (uint32_t)m) != 0;
    }
    if (tot > 0 && tot <= 10) {
        snprintf(b, sizeof b, "The %s river flooding eliminates all secondary units on the flood plain.", m_river(plain));
        f79270(-500, n, s16(PL(plain) + 8), b, 1);
        for (m = 1; m <= 9; m++) {
            int sc = s16(SECC + 2u * (uint32_t)m);
            if (rb(NAT(m) + 0x9A) == 0 || m == n || m == e)
                continue;
            if (sc < u[m]) {
                if ((rb(0xE1624u) & 0x80) == 0 && sb(NAT(m) + 0x9A) == 3) {
                    snprintf(b, sizeof b, "Flood also ravages %s.\nChoose sites of losses.", nm(m));
                    m27FCC(m, sc, b, 0x19, plain, 0);
                } else
                    cal2_test_f260C4(m, sc, 0x19, plain);
            } else if (sc > 0)
                cal2_test_f260C4(m, sc, 0x19, plain);
        }
    } else if (tot > 10) {
        if (m_hum(n)) {
            if (nn > 1) {
                snprintf(b, sizeof b, "%s, assign secondary losses for Flood.", nm(n));
                m29550(n, e, 10, b);
            } else
                for (m = 1; m <= 9; m++)
                    if (s16(SECC + 2u * (uint32_t)m))
                        w8(SECL + (uint32_t)m, s16(SECC + 2u * (uint32_t)m) > 10 ? 10 : s16(SECC + 2u * (uint32_t)m));
        } else
            cal2_test_f27518(n, -1, 10, 0x19);
        for (m = 1; m <= 9; m++) {
            if (sb(SECL + (uint32_t)m) <= 0)
                continue;
            if (m_hum(g_original ? n : m)) {
                snprintf(b, sizeof b, "Flood also ravages %s.\nChoose sites of losses.", nm(m));
                m27FCC(m, sb(SECL + (uint32_t)m), b, 0x19, plain, 0);
            } else
                cal2_test_f260C4(m, sb(SECL + (uint32_t)m), 0x19, plain);
        }
    }
    if (u[10])
        cal2_test_f260C4(10, u[10], 0x19, plain);
    f1E69C(k);
}

/* ==== the runner ================================================================================================= */

enum { R_25A38, R_2D4F0, R_2E59C, R_27CEC, R_27FCC, R_291D0, R_29550 };

static int g_checks, g_fail;
static uint32_t g_rng = 12345u;
static int g_a[6];
static char g_text[100] = "Test text.";

static uint32_t rnd(uint32_t n)
{
    g_rng = g_rng * 1103515245u + 12345u;
    return n ? (g_rng >> 8) % n : 0;
}

static void expect(int ok, const char *what)
{
    g_checks++;
    if (!ok) {
        g_fail++;
        printf("FAIL %s\n", what);
    }
}

static void call(int which, int model)
{
    switch (which) {
    case R_25A38: if (model) m25A38(g_a[0]); else f25A38(g_a[0]); break;
    case R_2D4F0: if (model) m2D4F0(g_a[0]); else f2D4F0(g_a[0]); break;
    case R_2E59C: if (model) m2E59C(g_a[0]); else f2E59C(g_a[0]); break;
    case R_27CEC: if (model) m27CEC(g_a[0]); else f27CEC(g_a[0]); break;
    case R_27FCC:
        if (model) m27FCC(g_a[0], g_a[1], g_text, g_a[2], g_a[3], g_a[4]);
        else f27FCC(g_a[0], g_a[1], g_text, g_a[2], g_a[3], g_a[4]);
        break;
    case R_291D0: if (model) m291D0(g_a[0], g_a[1], g_text); else f291D0(g_a[0], g_a[1], g_text); break;
    case R_29550: if (model) m29550(g_a[0], g_a[1], g_a[2], g_text); else f29550(g_a[0], g_a[1], g_a[2], g_text); break;
    }
}

static void side(int model, int which, uint32_t seed)
{
    g_log = model ? g_model : g_real;
    g_len = model ? &g_model_len : &g_real_len;
    *g_len = 0;
    g_log[0] = 0;
    g_ds = model ? M : R;
    g_srng = seed * 2654435761u + 1u;
    g_steps = 0;
    g_screen = 0;
    g_iforced = 0;
    g_ifm = 0;
    heap_reset();
    if (setjmp(g_jb) == 0)
        call(which, model);
    g_ds = R;
}

/* runs the port and the model from the same state; returns 1 when both agree. The port's state is left in R. */
static int run_both(int which, uint32_t seed, const char *what)
{
    size_t i;
    const char *a, *b;
    int line = 1;

    memcpy(M, R, DS_SIZE);
    side(0, which, seed);
    side(1, which, seed);
    g_checks++;
    if (strcmp(g_real, g_model) != 0) {
        a = g_real;
        b = g_model;
        while (*a && *a == *b) {
            if (*a == '\n')
                line++;
            a++;
            b++;
        }
        while (a > g_real && a[-1] != '\n') { a--; b--; }
        printf("FAIL %s: trace line %d\n  port : %.150s\n  model: %.150s\n", what, line, a, b);
        g_fail++;
        return 0;
    }
    for (i = 0; i < DS_SIZE; i++)
        if (R[i] != M[i]) {
            printf("FAIL %s: byte 0x%X port %02X model %02X\n", what, (unsigned)(i + DS_BASE), R[i], M[i]);
            g_fail++;
            return 0;
        }
    return 1;
}

static int has(const char *s) { return strstr(g_real, s) != NULL; }
static int count_before(const char *s, const char *marker)
{
    int c = 0;
    const char *p = g_real, *end = strstr(g_real, marker);
    if (end == NULL)
        end = g_real + strlen(g_real);
    while ((p = strstr(p, s)) != NULL && p < end) { c++; p += strlen(s); }
    return c;
}
static int count_of(const char *s)
{
    int c = 0;
    const char *p = g_real;
    while ((p = strstr(p, s)) != NULL) { c++; p += strlen(s); }
    return c;
}

/* ---- states ---------------------------------------------------------------------------------------------------- */

static void put_tokens(int t, int n, int k)
{
    uint32_t a = AR(t + 1), rec = NAT(n);
    if (k <= 0)
        return;
    if (sb(a + 1u + (uint32_t)n) != n) {
        w8(a + 1, rb(a + 1) + 1);
        w8(a + 1u + (uint32_t)n, n);
    }
    w8(a + 0x0Eu + (uint32_t)n, sb(a + 0x0Eu + (uint32_t)n) + k);
    eff_list_add(t, n);
    w8(rec + 0x98, sb(rec + 0x98) + k);
}

static int put_city(int t, int n, int round)
{
    uint32_t rec = NAT(n);
    int c = sb(rec + 0x51);
    if (c >= 9 || (rb(TS(t) + 2) & 1) || sb(AR(t + 1) + 1) != 0)
        return 0;
    w8(TS(t) + 2, rb(TS(t) + 2) | 1);
    w8(TS(t), n);
    w8(TS(t) + 1, round);
    w8(rec + 0x52u + (uint32_t)c, t + 1);
    w8(rec + 0x51, c + 1);
    return 1;
}

static void clear_board(void)
{
    int n, t;
    for (t = 0; t < 160; t++) {
        uint32_t a = AR(t + 1);
        memset(g_ds + (a - DS_BASE) + 1, 0, 68);
        w8(TS(t), 0);
        w8(TS(t) + 1, 0xFF);
        w8(TS(t) + 2, 0);
    }
    for (n = 1; n <= 13; n++) {
        uint32_t rec = NAT(n);
        w8(rec + 0x51, 0);
        memset(g_ds + (rec + 0x52 - DS_BASE), 0, 9);
        w8(rec + 0x60, 0);
        memset(g_ds + (rec + 0x61 - DS_BASE), 0xFF, 55);
        w8(rec + 0x98, 0);
    }
}

static void base_clean(void)
{
    int n, a;
    memcpy(R, g_pristine, DS_SIZE);
    g_ds = R;
    clear_board();
    for (n = 1; n <= 13; n++) {
        w8(NAT(n) + 0x9A, n <= 9 ? 3 : 3);
        for (a = 0x01; a <= 0x12; a++)
            w8(NAT(n) + (uint32_t)a, 0);
        for (a = 0x24; a <= 0x3B; a++)
            w8(NAT(n) + (uint32_t)a, 0);
        w8(NAT(n) + 0x99, 20);
        w8(NAT(n) + 0x3D, n);
    }
    for (n = 0; n < 157; n++) {
        w8(TR0(n), rb(TR0(n)) & ~2);
        w8(TR1(n), (rb(TR1(n)) & 0x47) | 2 << 3);
    }
    memcpy(g_ds + (PANEL - DS_BASE), g_ds + (0xBB414u - DS_BASE), 1002);   /* the map screen's panel, 13 records */
    w32(PBASE, s16(PANEL));
    w16(0xF40FCu, 3);
    w8(0xE1624u, rb(0xE1624u) & 0x6F);
    w8(0xF0980u, rb(0xF0980u) & ~4);
    w8(0xF097Cu, rb(0xF097Cu) & ~8);
    w8(0xF4102u, 15);
    w16(0xF40BAu, 0);
    memset(g_ds + (GRAIN - DS_BASE), 0, 20);
    g_nforced = 0;
    g_nfm = 0;
    g_yes = -1;
    g_unatt = 0;
    g_original = 0;
}

static const int g_plain_t[] = { 138, 143, 18, 19, 9, 126, 16, 48, 47, 46, 68, 71, 69, 70, 111, 115, 112, 113, 114 };

/* a random game state: players, advances, Grain, the board (with bias to the flood plains), cities, Pirates */
static void random_world(int keep_board)
{
    int n, t, k, c, a, inplay = 0;
    g_ds = R;
    if (!keep_board) {
        clear_board();
        for (t = 0; t < 157; t++) {
            w8(TR0(t), (rb(TR0(t)) & ~2) | (rnd(2) ? 2 : 0));
            w8(TR1(t), (rb(TR1(t)) & 0x47) | (int)rnd(6) << 3 | (rnd(3) == 0 ? 0x80 : 0));
        }
    }
    for (n = 1; n <= 9; n++) {
        if (!keep_board)
            w8(NAT(n) + 0x9A, rnd(6) == 0 ? 0 : rnd(5) == 0 ? 2 : 3);
        else if (rb(NAT(n) + 0x9A) != 0)
            w8(NAT(n) + 0x9A, rnd(5) == 0 ? 2 : 3);
        inplay += rb(NAT(n) + 0x9A) != 0;
        for (a = 0x24; a <= 0x3B; a++)
            if (rnd(4) == 0)
                w8(NAT(n) + (uint32_t)a, rnd(5) != 0);
        w8(NAT(n) + 0x08, rnd(3) == 0 ? (int)rnd(5) : 0);
        w8(NAT(n) + 0x99, (int)rnd(30));
        w8(NAT(n) + 0x3D, n);
    }
    if (inplay < 2) {
        w8(NAT(1) + 0x9A, 3);
        w8(NAT(2) + 0x9A, 3);
    }
    if (!keep_board) {
        for (n = 1; n <= 10; n++) {
            if (n <= 9 && rb(NAT(n) + 0x9A) == 0)
                continue;
            c = (int)rnd(6);
            for (k = 0; k < c * 3 && sb(NAT(n) + 0x51) < c; k++)
                put_city(rnd(2) ? g_plain_t[rnd(19)] : (int)rnd(157), n, (int)rnd(4));
        }
        for (k = 0; k < 90; k++) {
            n = 1 + (int)rnd(10);
            t = rnd(3) == 0 ? g_plain_t[rnd(19)] : (int)rnd(157);
            if (rb(NAT(n) + 0x9A) && !(rb(TS(t) + 2) & 1) && (n <= 9 || rnd(4) == 0))
                put_tokens(t, n, 1 + (int)rnd(4));
        }
    }
    w8(0xE1624u, (rb(0xE1624u) & 0x6F) | (rnd(25) == 0 ? 0x80 : 0));
    w8(0xF0980u, (rb(0xF0980u) & ~4) | (rnd(2) ? 4 : 0));
    w8(0xF097Cu, (rb(0xF097Cu) & ~8) | (rnd(2) ? 8 : 0));
    w16(0xF40BAu, rnd(4) == 0 ? 3 : 0);
    g_unatt = rnd(4) == 0;
}

static int pick_victim(void)
{
    int n;
    do n = 1 + (int)rnd(9); while (rb(NAT(n) + 0x9A) == 0);
    return n;
}

static void set_rec(int k, int id, int giver, int v)
{
    w8(CREC(k), id);
    w8(CREC(k) + 1, giver);
    w8(CREC(k) + 2, v);
}

static int load_save(const char *path)
{
    FILE *fp;
    int16_t version = 0;
    char desc[0x34];

    memcpy(R, g_pristine, DS_SIZE);
    g_ds = R;
    fp = fopen(path, "rb");
    if (fp == NULL)
        return 0;
    memset(desc, 0, sizeof desc);
    f5F05C(&version, desc, fp);
    fclose(fp);
    return 1;
}

/* ---- A. hand vectors ------------------------------------------------------------------------------------------- */

static void forced(int n, ...)
{
    va_list ap;
    int i;
    va_start(ap, n);
    for (i = 0; i < n && i < 64; i++)
        g_forced[i] = va_arg(ap, int);
    va_end(ap);
    g_nforced = n;
}

static void fmouse(int n, ...)
{
    va_list ap;
    int i;
    va_start(ap, n);
    for (i = 0; i < n && i < 64; i++)
        g_fmouse[i][0] = va_arg(ap, int);
    va_end(ap);
    g_nfm = n;
}

static void only(int n)   /* nations 1..n in play (human), the rest out */
{
    int m;
    for (m = 1; m <= 9; m++)
        w8(NAT(m) + 0x9A, m <= n ? 3 : 0);
}

static void part_a(void)
{
    int f0 = g_fail, c0 = g_checks;
    char s[160];

    /* --- Famine, human victim 1: 2 cities + 3 tokens -> loss 10; no secondary units -> 0x291D0 returns at once --- */
    base_clean(); only(3);
    put_city(0, 1, 1); put_city(1, 1, 1); put_tokens(5, 1, 3);
    set_rec(0, 0x15, -1, 1);
    forced(3, -0x65, -0x65, -1);
    g_a[0] = 0;
    run_both(R_25A38, 1, "A famine 1");
    snprintf(s, sizeof s, "msg -500 1 -200 Famine ravages %s. 1", nm(1));
    expect(has(s), "A famine message");
    snprintf(s, sizeof s, "box %s, choose sites of Famine losses.", nm(1));
    expect(has(s), "A famine NationLosesUnits text");
    expect(has("text 310 374 19 10\n"), "A famine left 10");
    expect(has("list 1 15 0"), "A famine list kind 0x15");
    expect(has("7A4C4 -600"), "A famine animation hook");
    expect(!has("Assignments"), "A famine no secondary columns");
    expect(has("resolved 0") && s16(0xBB400u) == 0x23, "A famine resolved, height 0x23");

    /* --- Famine: Pottery + 2 Grain; the arrows: +, +, + (limit 2), -, + ; loss 10 - 8 = 2 --- */
    base_clean(); only(3);
    put_city(0, 1, 1); put_city(1, 1, 1); put_tokens(5, 1, 3);
    w8(NAT(1) + 0x24, 1); w8(NAT(1) + 0x08, 2);
    set_rec(0, 0x15, -1, 1);
    forced(7, -10, -10, -10, -9, -10, -1, -100);
    g_a[0] = 0;
    run_both(R_25A38, 2, "A famine grain");
    expect(s16(GRAIN + 2) == 2, "A grain used 2");
    expect(has("text 310 374 19 2\n"), "A grain: NationLosesUnits with 2");
    snprintf(s, sizeof s, "left %s - choose Grain reduction of Famine losses.", nm(1));
    expect(has(s), "A grain text");

    /* --- Famine: Grain covers everything: 0 cities, 2 tokens, 1 card -> loss -2 -> prevented --- */
    base_clean(); only(3);
    put_tokens(5, 1, 2);
    w8(NAT(1) + 0x24, 1); w8(NAT(1) + 0x08, 1);
    set_rec(0, 0x15, -1, 1);
    forced(3, -10, -10, -1);
    g_a[0] = 0;
    run_both(R_25A38, 3, "A famine prevented");
    snprintf(s, sizeof s, "msg -500 1 -200 Your usage of Grain has prevented Famine in %s. 1", nm(1));
    expect(has(s) && s16(GRAIN + 2) == 1, "A grain prevents famine");
    expect(!has("append BF650"), "A no NationLosesUnits");

    /* --- Famine secondary <= 20: nations 2, 3 with 1 city + 4 tokens (cap 8) and 2 tokens -> 8 + 2 shown --- */
    base_clean(); only(3);
    put_tokens(5, 1, 1);
    put_city(10, 2, 1); put_tokens(11, 2, 4);
    put_tokens(20, 3, 2);
    set_rec(0, 0x15, -1, 1);
    g_a[0] = 0;
    forced(2, -0x65, -1);
    run_both(R_25A38, 4, "A famine secondary 10");
    expect(has("left Assignments for Famine secondary losses\n"), "A secondary title");
    snprintf(s, sizeof s, "text 220 365 100 %s", nm(2));
    expect(has(s), "A two columns at 220, w 100");
    expect(has("text 220 410 100 8\n") && has("text 320 410 100 2\n"), "A secondary numbers 8, 2");
    snprintf(s, sizeof s, "box Famine also ravages %s.\nChoose sites of losses.", nm(2));
    expect(has(s), "A secondary victim 2 screen");
    expect(has("setx BB800 2A 265") && has("sety BB800 2A 430"), "A CONTINUE moved");

    /* --- Famine secondary > 20: four nations x 8 -> 0x29550, 20 assigned --- */
    base_clean(); only(5);
    put_tokens(5, 1, 1);
    put_tokens(10, 2, 9); put_tokens(11, 3, 9); put_tokens(12, 4, 9); put_tokens(13, 5, 9);
    set_rec(0, 0x15, -1, 1);
    g_a[0] = 0;
    forced(2, -0x65, -1);
    run_both(R_25A38, 5, "A famine assign");
    snprintf(s, sizeof s, "left %s, assign secondary losses for Famine.", nm(1));
    expect(has(s), "A assign text");
    expect(has("text 239 433 18 20\n") && has("text 239 433 18 0\n"), "A left 20 .. 0");
    expect(has("on F596C 2A"), "A CONTINUE enabled at 0");
    expect(rb(SECL + 2) + rb(SECL + 3) + rb(SECL + 4) + rb(SECL + 5) == 20 || has("CUT"), "A 20 assigned");
    expect(s16(PANEL) == 13 || has("CUT"), "A panel count back to the base");

    /* --- 0x29550 alone: a plus at the cap does nothing; auto-repeat (no release between); a minus at 0 nothing --- */
    base_clean(); only(3);
    w16(SECC + 4, 2); w16(SECC + 6, 1);
    fmouse(8, 0xD3, 0xCC, 0xCC, 0xCC, 0xCD, 0xD4, 0xCD, 0x2A);
    g_a[0] = 1; g_a[1] = -1; g_a[2] = 3;
    run_both(R_29550, 6, "A 29550");
    expect(rb(SECL + 2) == 2 && rb(SECL + 3) == 1, "A 29550 caps 2, 1");
    expect(count_of("release") == 1, "A 29550 auto-repeat: one release wait");
    expect(count_of("wait 150") == 7, "A 29550 seven +/- waits");
    expect(has("dispatch 2A -> -1"), "A 29550 CONTINUE");
    expect(prec(0xCC) == 0, "A 29550 records cut off");

    /* --- 0x291D0: no column -> nothing but the background --- */
    base_clean(); only(3);
    g_a[0] = 1; g_a[1] = 2;
    w16(SECC + 4, 3);
    run_both(R_291D0, 7, "A 291D0 none");
    expect(!has("area") && has("vbcopy 0 639"), "A 291D0 returns at once");

    /* --- 0x291D0 with 5 columns: w 60, x0 170; the short name when the long one is too wide (8 px per char) --- */
    base_clean(); only(6);
    w16(SECC + 4, 1); w16(SECC + 6, 2); w16(SECC + 8, 3); w16(SECC + 10, 4); w16(SECC + 12, 5);
    g_a[0] = 1; g_a[1] = -1;
    g_unatt = 1;
    run_both(R_291D0, 8, "A 291D0 five");
    snprintf(s, sizeof s, "text 170 365 60 %s", (int)strlen(nm(2)) * 8 >= 60 ? snm(2) : nm(2));
    expect(has(s), "A 291D0 column 0 name");
    expect(has("frame 192 408 16 14 5"), "A 291D0 box of column 0");

    /* --- 0x27FCC Famine: city REMOVE ONE (eliminated, -5), token REMOVE ALL, FINISHED only at 0 --- */
    base_clean(); only(3);
    put_city(30, 1, 1); put_tokens(31, 1, 3);
    g_a[0] = 1; g_a[1] = 8; g_a[2] = 0x15; g_a[3] = 0; g_a[4] = 0;
    forced(5, -1, -0x65, -10, -0x66, -1);
    run_both(R_27FCC, 9, "A 27FCC famine");
    expect(has("78F04 30 1") && has("text 310 374 19 3\n") && has("text 310 374 19 0\n"), "A 27FCC 8 -> 3 -> 0");
    expect(has("not offered"), "A 27FCC FINISHED refused before 0");
    expect(sb(AR(32) + 0x0F) == 0 && sb(NAT(1) + 0x98) == 0, "A 27FCC tokens gone");
    expect(has("free"), "A 27FCC blocks freed");

    /* --- 0x27FCC Epidemic: REMOVE ONE on a city reduces it (limit 2, no Agriculture: -3) and keeps one token --- */
    base_clean(); only(3);
    put_city(30, 1, 1); put_tokens(31, 1, 3);
    w8(NAT(1) + 0x99, 10);
    g_a[0] = 1; g_a[1] = 5; g_a[2] = 0x1B; g_a[3] = 0; g_a[4] = 0;
    forced(4, 0, -0x65, -10, -0x65);
    run_both(R_27FCC, 10, "A 27FCC epidemic");
    expect(has("78FEC 30 1") && has("text 310 374 19 1\n"), "A epidemic city: 5 - (5 - 2) = 2, then a token: 1");
    expect(sb(AR(31) + 0x0F) == 1, "A epidemic keeps one token in the reduced city");

    /* --- 0x27FCC target box, REDUCE CITY, RESTART restores --- */
    base_clean(); only(3);
    put_city(30, 1, 1);
    g_a[0] = 1; g_a[1] = 4; g_a[2] = 0x1B; g_a[3] = 0; g_a[4] = 1;
    g_yes = -0x61;
    w8(0xF097Cu, rb(0xF097Cu) | 8);
    forced(3, -0xE, -0x63, -100);
    run_both(R_27FCC, 11, "A 27FCC target restart");
    expect(has("right 439 374 TARGET") && has("text 444 374 19 1\n"), "A TARGET box");
    expect(has("yesno Are you sure you want to Restart? -97"), "A restart confirmed");
    expect(count_of("list 1 1B 0") >= 2 && has("text 310 374 19 1\n"), "A REDUCE 4 -> 1, RESTART rebuilds the list");

    /* --- BUG(1995) 2: an empty list shows no PIRACY and writes no border -1 (fixed); --original does --- */
    base_clean(); only(3);
    g_a[0] = 1; g_a[1] = 0; g_a[2] = 0x1B; g_a[3] = 0; g_a[4] = 0;
    forced(1, -1);
    run_both(R_27FCC, 12, "A empty fixed");
    expect(!has("PIRACY") && !has("border -1 15"), "A empty list fixed");
    base_clean(); only(3);
    g_original = 1;
    forced(1, -1);
    run_both(R_27FCC, 12, "A empty original");
    expect(has("PIRACY") && has("border -1 15"), "A empty list --original");

    /* --- BUG(1995) 3: the stale slot after the last entry left: REMOVE ONE disabled (fixed) / enabled (original) --- */
    base_clean(); only(3);
    put_tokens(31, 1, 1);
    w8(NAT(1) + 0x99, 10);
    g_a[0] = 1; g_a[1] = 3; g_a[2] = 0x15; g_a[3] = 0; g_a[4] = 0;
    forced(3, -0x65, -0x65, -0x63);
    run_both(R_27FCC, 13, "A stale fixed");
    expect(has("click F596C 1 -101 not offered"), "A stale fixed: REMOVE ONE not offered on the empty list");
    expect(count_before("57CE8 31 1 0 1", "click F596C 1 -99") == 1, "A stale fixed: one removal before RESTART");
    base_clean(); only(3);
    put_tokens(31, 1, 1);
    w8(NAT(1) + 0x99, 10);
    g_original = 1;
    forced(3, -0x65, -0x65, -0x63);
    run_both(R_27FCC, 13, "A stale original");
    expect(count_before("57CE8 31 1 0 1", "click F596C 1 -99") == 2, "A stale --original: the stale slot loses a second token");

    /* --- Flood: victim units on PO (2) and NILE (city 5 + nothing) -> NILE; losses min(5, 17) --- */
    base_clean(); only(3);
    put_tokens(18, 1, 2);
    put_city(111, 1, 1); w8(TR1(111), rb(TR1(111)) | 0x80);
    set_rec(0, 0x19, -1, 1);
    g_a[0] = 0;
    forced(0);
    run_both(R_2D4F0, 14, "A flood nile");
    snprintf(s, sizeof s, "box The NILE river Floods.\n%s, choose sites of losses.", nm(1));
    expect(has(s), "A flood NILE chosen");
    expect(has("list 1 19 4") && has("text 310 374 19 5\n"), "A flood list plain 4, 5 units");
    expect(s16(PL(2)) == 0 && s16(PL(1)) == 2 && s16(PL(4)) == 5, "A flood plain sums");

    /* --- Flood: TIGRIS (fixed) / TIGRUS (original); Engineering caps 7 --- */
    base_clean(); only(3);
    put_tokens(48, 1, 4); put_tokens(47, 1, 5);
    w8(NAT(1) + 0x2A, 1);
    set_rec(0, 0x19, -1, 1);
    g_a[0] = 0;
    run_both(R_2D4F0, 15, "A flood tigris");
    expect(has("The TIGRIS-EUPHRATES river Floods.") && has("text 310 374 19 7\n"), "A TIGRIS, cap 7");
    base_clean(); only(3);
    put_tokens(48, 1, 4);
    set_rec(0, 0x19, -1, 1);
    g_original = 1;
    run_both(R_2D4F0, 15, "A flood tigrus");
    expect(has("The TIGRUS-EUPHRATES river Floods."), "A TIGRUS --original");

    /* --- Flood tie: human -> 0x1EAEC with both plains listed; computer -> 0x2E45C --- */
    base_clean(); only(3);
    put_tokens(18, 1, 3); put_tokens(9, 1, 3);
    set_rec(0, 0x19, -1, 1);
    g_a[0] = 0;
    run_both(R_2D4F0, 16, "A flood tie human");
    expect(has("1EAEC 1 19 ->"), "A tie: human chooser");
    base_clean(); only(3);
    w8(NAT(1) + 0x9A, 2);
    put_tokens(18, 1, 3); put_tokens(9, 1, 3);
    set_rec(0, 0x19, -1, 1);
    run_both(R_2D4F0, 16, "A flood tie computer");
    expect(has("2E45C 1 -> 2") && has("260C4 1 3 19 2"), "A tie: computer chooser, last listed");

    /* --- Flood: no units, a coastal city: human f207B4, Engineering reduces; no coastal city: unaffected --- */
    base_clean(); only(3);
    put_city(40, 1, 1); w8(TR0(40), rb(TR0(40)) | 2);
    w8(NAT(1) + 0x2A, 1);
    set_rec(0, 0x19, -1, 1);
    g_a[0] = 0;
    run_both(R_2D4F0, 17, "A flood coastal");
    snprintf(s, sizeof s, "%s has no units on any flood plain so a coastal city must be reduced.", nm(1));
    expect(has(s) && has("207B4 1 1 19") && has("78FEC 40 1") && has("wait 2000"), "A coastal city reduced");
    base_clean(); only(3);
    w8(NAT(1) + 0x9A, 2);
    put_city(40, 1, 1); w8(TR0(40), rb(TR0(40)) | 2);
    set_rec(0, 0x19, -1, 1);
    run_both(R_2D4F0, 17, "A flood coastal computer");
    snprintf(s, sizeof s, "msg 2000 1 40 %s eliminates a coastal city in %s. 1", nm(1), tn(40));
    expect(has("2E1D0 1 19 -> 40") && has("78F04 40 1") && has(s), "A computer coastal city eliminated");
    base_clean(); only(3);
    set_rec(0, 0x19, -1, 1);
    run_both(R_2D4F0, 18, "A flood unaffected");
    snprintf(s, sizeof s, "%s has no units on any flood plain or coastal cities, so is unaffected by Flood.", nm(1));
    expect(has(s) && has("resolved 0"), "A unaffected");

    /* --- Flood secondary <= 10: 2 human loses part (cap 7, Engineering) -> screen; 3 computer all -> 260C4;
           Pirates -> 260C4(10, raw) --- */
    base_clean(); only(3);
    put_tokens(111, 1, 2);
    put_tokens(115, 3, 5); put_tokens(112, 3, 4); w8(NAT(3) + 0x2A, 1);
    w8(NAT(2) + 0x9A, 2); put_tokens(113, 2, 2);
    put_tokens(114, 10, 6);
    set_rec(0, 0x19, -1, 1);
    g_a[0] = 0;
    forced(2, -0x66, -1);
    run_both(R_2D4F0, 19, "A flood secondary small");
    expect(has("The NILE river flooding eliminates all secondary units on the flood plain."), "A secondary <= 10 msg");
    expect(has("msg -500 1 111 The NILE"), "A secondary msg centred on the plain's first territory");
    expect(has("260C4 2 2 19 4"), "A secondary computer losing all");
    snprintf(s, sizeof s, "box Flood also ravages %s.\nChoose sites of losses.", nm(3));
    expect(has(s) && has("text 310 374 19 7\n"), "A secondary human partial screen, Engineering cap 7");
    expect(has("260C4 10 6 19 4") || has("CUT"), "A Pirates lose all, uncapped");

    /* --- Flood secondary > 10, human victim 1, computer m = 2: fixed -> 260C4; original -> human screen --- */
    base_clean(); only(3);
    put_tokens(111, 1, 2);
    w8(NAT(2) + 0x9A, 2); put_tokens(115, 2, 8);
    put_tokens(112, 3, 8);
    set_rec(0, 0x19, -1, 1);
    g_a[0] = 0;
    forced(2, -0x66, -1);
    run_both(R_2D4F0, 20, "A flood big fixed");
    expect(has("assign secondary losses for Flood."), "A > 10: assignment screen");
    {
        char s2[160];
        snprintf(s2, sizeof s2, "box Flood also ravages %s.", nm(2));
        expect(!has(s2), "A > 10 fixed: no human screen for the computer");
    }
    base_clean(); only(3);
    put_tokens(111, 1, 2);
    w8(NAT(2) + 0x9A, 2); put_tokens(115, 2, 8);
    put_tokens(112, 3, 8);
    set_rec(0, 0x19, -1, 1);
    g_original = 1;
    forced(2, -0x66, -1);
    run_both(R_2D4F0, 20, "A flood big original");
    {
        char s2[160];
        snprintf(s2, sizeof s2, "box Flood also ravages %s.", nm(2));
        expect(has(s2) || rb(SECL + 2) == 0, "A > 10 --original: the computer gets the human screen");
    }

    /* --- Epidemic: Medicine + Roadbuilding: 16 - 8 + 5 = 13, capped at 3 - 1 + 4 = 6 --- */
    base_clean(); only(3);
    put_city(50, 1, 1); put_tokens(51, 1, 3);
    w8(NAT(1) + 0x2D, 1); w8(NAT(1) + 0x28, 1);
    set_rec(0, 0x1B, -1, 1);
    g_a[0] = 0;
    run_both(R_2E59C, 21, "A epidemic primary");
    snprintf(s, sizeof s, "box Epidemic ravages %s.\nChoose sites of losses.", nm(1));
    expect(has(s) && has("text 310 374 19 6\n") && has("list 1 1B 0"), "A epidemic 6");
    expect(!has("2EC38"), "A epidemic tokens to spare: no target");
    /* only cities: f2EC38's target */
    base_clean(); only(3);
    put_city(50, 1, 1); put_city(52, 1, 1); put_tokens(51, 1, 1);
    set_rec(0, 0x1B, -1, 1);
    run_both(R_2E59C, 22, "A epidemic cities");
    expect(has("2EC38 1 8 -> 2") && has("text 444 374 19 2\n"), "A epidemic target from f2EC38");
    /* secondary: trader 3 exempt; 2 with Medicine assigned 3 -> saved, message with the victim's panel */
    base_clean(); only(3);
    put_tokens(51, 1, 1);
    put_tokens(60, 2, 4); w8(NAT(2) + 0x2D, 1);
    put_tokens(61, 3, 9);
    set_rec(0, 0x1B, 3, 1);
    run_both(R_2E59C, 23, "A epidemic medicine");
    snprintf(s, sizeof s, "msg -500 1 -200 %s saved from Epidemic by MEDICINE. 1", nm(2));
    expect(has(s), "A saved by MEDICINE (victim's panel)");
    expect(has("left Assignments for Epidemic secondary losses.\n"), "A human title with the period");
    expect(s16(SECC + 6) == 0, "A trader exempt");

    /* --- the three thresholds exactly: Famine 20, Epidemic 25, Flood 10 secondary units are all shown / lost --- */
    base_clean(); only(4);
    put_tokens(5, 1, 1);
    put_tokens(10, 2, 8); put_tokens(11, 3, 8); put_tokens(12, 4, 4);
    set_rec(0, 0x15, -1, 1);
    g_a[0] = 0;
    forced(2, -0x65, -1);
    run_both(R_25A38, 30, "A famine exactly 20");
    expect(has("left Assignments for Famine secondary losses\n") && !has("assign secondary losses for Famine."),
           "A famine 20: shown, not assigned");
    base_clean(); only(4);
    put_tokens(5, 1, 1);
    put_tokens(10, 2, 11); put_tokens(11, 3, 11); put_tokens(12, 4, 6);
    set_rec(0, 0x1B, -1, 1);
    g_a[0] = 0;
    forced(1, -1);
    run_both(R_2E59C, 31, "A epidemic exactly 25");
    expect(has("left Assignments for Epidemic secondary losses.\n") && !has("assign secondary losses for Epidemic."),
           "A epidemic 25: shown, not assigned");
    base_clean(); only(3);
    put_tokens(111, 1, 2);
    put_tokens(115, 2, 6); put_tokens(112, 3, 4);
    set_rec(0, 0x19, -1, 1);
    g_a[0] = 0;
    forced(2, -0x66, -1);
    run_both(R_2D4F0, 32, "A flood exactly 10");
    expect(has("flooding eliminates all secondary units") && has("260C4 2 6 19 4") && has("260C4 3 4 19 4"),
           "A flood 10: all lost");

    printf("A. hand vectors: %d checks, %d failed\n", g_checks - c0, g_fail - f0);
}

/* ---- B..E: random states --------------------------------------------------------------------------------------- */

static void part_b(void)
{
    int f0 = g_fail, i, orig, n, kinds[6] = { 0x15, 0x19, 0x1B, 7, 8, 9 };
    char what[64];
    for (i = 0; i < 6000; i++) {
        orig = i & 1;
        base_clean();
        random_world(0);
        g_original = orig;
        n = pick_victim();
        g_a[0] = n;
        g_a[2] = kinds[rnd(6)];
        g_a[3] = g_a[2] == 0x19 ? (int)rnd(5) : 0;
        g_a[1] = (int)rnd(14) - (rnd(10) == 0 ? 2 : 0);
        g_a[4] = rnd(3) == 0 ? (int)rnd(4) : 0;
        if (g_a[2] == 7 || g_a[2] == 8) {
            int t;
            for (t = 0; t < 157; t++) {
                w8(0xF7984u + (uint32_t)t, rnd(6) == 0);
                w8(0xF7A24u + (uint32_t)t, rnd(6) == 0);
            }
        }
        snprintf(g_text, sizeof g_text, "%s loses units.", nm(n));
        snprintf(what, sizeof what, "B 27FCC run %d", i);
        run_both(R_27FCC, (uint32_t)i + 1000u, what);
    }
    printf("B. 0x27FCC random: 6000 runs, %d failed\n", g_fail - f0);
}

static void part_c(void)
{
    int f0 = g_fail, i, m, a;
    char what[64];
    for (i = 0; i < 3000; i++) {
        base_clean();
        random_world(0);
        g_original = i & 1;
        a = pick_victim();
        for (m = 1; m <= 10; m++)
            w16(SECC + 2u * (uint32_t)m, rnd(3) == 0 ? 0 : (int)rnd(11));
        for (m = 0; m < 14; m++)
            w8(SECL + (uint32_t)m, (int)rnd(9));
        snprintf(g_text, sizeof g_text, "Secondary losses for %s.", nm(a));
        g_a[0] = a;
        g_a[1] = rnd(3) == 0 ? (int)rnd(10) : -1;
        g_a[2] = 1 + (int)rnd(25);
        snprintf(what, sizeof what, "C run %d", i);
        switch (i % 3) {
        case 0:
            run_both(R_291D0, (uint32_t)i + 5000u, what);
            break;
        case 1:
            if (m_cols(a, g_a[1], NULL) == 0) {   /* the callers never pass an empty set (division by 0) */
                g_ds = R;
                w16(SECC + 2u * (uint32_t)(a == 9 ? 1 : 9), 3);
                if (g_a[1] == (a == 9 ? 1 : 9) || rb(NAT(a == 9 ? 1 : 9) + 0x9A) == 0) {
                    g_a[1] = -1;
                    w8(NAT(a == 9 ? 1 : 9) + 0x9A, 3);
                }
            }
            run_both(R_29550, (uint32_t)i + 5000u, what);
            break;
        default:
            w8(SECL + (uint32_t)a, (int)rnd(12));
            w8(NAT(a) + 0x08, (int)rnd(5));
            run_both(R_27CEC, (uint32_t)i + 5000u, what);
            break;
        }
    }
    printf("C. 0x291D0 / 0x29550 / 0x27CEC random: 3000 runs, %d failed\n", g_fail - f0);
}

static void part_d(void)
{
    int f0 = g_fail, i, w, n, g;
    static const int ids[3] = { 0x15, 0x19, 0x1B };
    static const int which[3] = { R_25A38, R_2D4F0, R_2E59C };
    char what[64];
    for (w = 0; w < 3; w++)
        for (i = 0; i < 3000; i++) {
            base_clean();
            random_world(0);
            g_original = i & 1;
            n = pick_victim();
            g = rnd(3) == 0 ? 1 + (int)rnd(9) : -1;
            if (g == n || w != 2)
                g = -1;   /* Famine and Flood cannot be traded (game.h's stacks): the giver is -1 */
            set_rec(0, ids[w], g, n);
            g_a[0] = 0;
            snprintf(what, sizeof what, "D %X run %d", (unsigned)ids[w], i);
            run_both(which[w], (uint32_t)(i + 20000 * (w + 1)), what);
        }
    printf("D. Famine / Flood / Epidemic random: 9000 runs, %d failed\n", g_fail - f0);
}

static void part_e(const char *savedir, const char *gamedir)
{
    int f0 = g_fail, s, w, i, n, runs = 0;
    char path[4][300];
    static const int ids[3] = { 0x15, 0x19, 0x1B };
    static const int which[3] = { R_25A38, R_2D4F0, R_2E59C };
    char what[96];
    snprintf(path[0], sizeof path[0], "%s\\SAVGAMEA.DAT", savedir);
    snprintf(path[1], sizeof path[1], "%s\\savgameb.dat", savedir);
    snprintf(path[2], sizeof path[2], "%s\\savgame0.dat", gamedir);
    snprintf(path[3], sizeof path[3], "%s\\savgame1.dat", gamedir);
    for (s = 0; s < 4; s++)
        for (w = 0; w < 3; w++)
            for (i = 0; i < 100; i++) {
                if (!load_save(path[s])) {
                    printf("E: cannot read %s\n", path[s]);
                    g_fail++;
                    break;
                }
                memcpy(g_ds + (PANEL - DS_BASE), g_ds + (0xBB414u - DS_BASE), 1002);
                w32(PBASE, s16(PANEL));
                g_nforced = 0;
                g_nfm = 0;
                g_yes = -1;
                g_unatt = 0;
                g_original = i & 1;
                if (i >= 50)
                    random_world(1);   /* the same board, other players' kinds and advances */
                n = pick_victim();
                set_rec(0, ids[w], w == 2 && rnd(3) == 0 ? 1 + (int)rnd(9) : -1, n);
                g_a[0] = 0;
                snprintf(what, sizeof what, "E save %d calamity %X run %d", s, (unsigned)ids[w], i);
                run_both(which[w], (uint32_t)(i + 1000 * s + 100000 * (w + 1)), what);
                runs++;
            }
    printf("E. real saves: %d runs, %d failed\n", runs, g_fail - f0);
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:\\work\\advciv_run";
    const char *savedir = argc > 2 ? argv[2] : "C:\\work\\advciv_play";

    setvbuf(stdout, NULL, _IONBF, 0);
    if (state_init(gamedir) != 0) {
        fprintf(stderr, "cannot load %s/AC_MAIN.EXE\n", gamedir);
        return 2;
    }
    R = g_ds;
    g_pristine = malloc(DS_SIZE);
    M = malloc(DS_SIZE);
    g_real = malloc(LOGSZ);
    g_model = malloc(LOGSZ);
    if (g_pristine == NULL || M == NULL || g_real == NULL || g_model == NULL)
        return 2;
    memcpy(g_pristine, R, DS_SIZE);
    g_log = g_real;
    g_len = &g_real_len;

    part_a();
    part_b();
    part_c();
    part_d();
    part_e(savedir, gamedir);

    printf("coverage (port side): REMOVE ONE %ld, REMOVE ALL %ld, REDUCE %ld, RESTART %ld, FINISHED %ld, 29550 + %ld, "
           "- %ld, Grain arrows %ld, 291D0 waits %ld, 0x260C4 %ld, empty-list draws %ld, calamities completed %ld; not_ported %d, cut %d\n",
           g_cov[0], g_cov[1], g_cov[2], g_cov[3], g_cov[4], g_cov[5], g_cov[6], g_cov[7], g_cov[8], g_cov[9],
           g_cov[10], g_cov[11], g_np, g_cuts);
    printf("%d checks, %d failed\n", g_checks, g_fail);
    return g_fail == 0 ? 0 : 1;
}
