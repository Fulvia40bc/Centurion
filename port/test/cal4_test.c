/* cal4_test [GAMEDIR [SAVEDIR]]: unit checks for module CAL4 (cal4.c, Civil War) that need no game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init; default C:\work\advciv_run, only read), LANDADJ.DAT and
   COASTADJ.DAT (the land and coast lists, read into the test's own heap blocks so the handles [0xF8E7C + 4t] /
   [0xF90FC + 4t] resolve) and the saves savgame0.dat (start of MOVEMENT R4) and savgame1.dat (R4, all human); SAVEDIR
   holds SAVGAMEA.DAT and savgameb.dat (default C:\work\advciv_play, only read). All four saves are read with
   saveload.c's f5F05C as the game does.

   cal4.c is built with -DCAL4_TEST: its calls into code that is not ported yet (the computers' 0x2C190, 0x2C894,
   0x2D10C, 0x2D1C0; CAL2's NationLosesUnits 0x27FCC) come to recorders here, which trace the call, act out a plausible
   effect on the data object (units moved into the faction, losses taken, cities / tokens annexed) and return.
   formulas.c (0x2AC94) and helpers2.c (0x2C778) are linked: both the port and the model call them.

   Every other routine cal4.c calls is a stub below that appends one line to a trace (drawing included); those with
   data effects apply them on the data object g_ds (the city and token edits of rules.c / move.c, the borders, the
   clamp, the current dialog, cal1.c's choice list 0x28CFC for kinds 4..9). Clicks (f78BA4), the YES/NO answers and
   the side chosen in the losses box (f5CDE8) come from a script: a seeded random choice among the codes the screen
   offers (enabled buttons only, map clicks on listed entries, the arrows, -100, codes that do nothing), the same seed
   for the port and the model; a run that clicks more than 3000 times is cut off ("CUT") identically on both sides.

   The model is a second implementation of the five routines, written from the disassembly (not from cal4.c): it runs
   on a copy of the data object (g_ds pointed at the copy), calls the same stubs, and so writes the trace it expects.
   After each run the two traces must be equal and the whole data object must equal the model's copy.
     A. hand vectors (expected values written out): no Civil War (the beneficiary is the victim), quelled, the faction
        sizes (Philosophy, Music, Drama & Poetry, Democracy), the ranking's tie order, the whole annexation with its
        marks, ALL on tokens (fixed: the target pays; --original: it does not), the texts ("beneficiary",
        "First"/"Second" in the computer's messages; --original as the binary), the "no longer exists" path, the
        Military losses' marks and automatic removals on the real adjacency lists, 0x2D3E0's counts, 0x2EE6C's views;
     B. 0x29E34 on 6,000 random states (humans, computers, PBEM bit, advances, toggles, stale faction records);
     C. 0x2B6EC alone on 6,000 (modes 4 / 5 / 6, random sources / targets / counts, restarts with and without the
        confirmation); D. 0x2AD50 alone on 4,000 random faction boards; E. 0x2EE6C / 0x2D3E0 on 2,000;
     F. 0x29E34 on the four real saves, every nation in play as the victim, 40 runs per save and victim;
     G. deliberately broken models (14, one change each) must each be caught.
   Every random part runs with and without --original.
   Prints each failure (the first differing trace line or byte), the counts per part, the coverage of the port's
   paths, and exits 0 when all pass.

   Build: clang -std=c11 -DCAL4_TEST port/test/cal4_test.c port/src/cal4.c port/src/formulas.c port/src/helpers2.c
          port/src/saveload.c port/src/state.c port/src/le_image.c -Iport/src -o cal4_test.exe */

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cal1.h"
#include "cal4.h"
#include "conflict.h"
#include "fg.h"
#include "formulas.h"
#include "game.h"
#include "helpers2.h"
#include "kit.h"
#include "map.h"
#include "mapdata.h"
#include "move.h"
#include "panel.h"
#include "rules.h"
#include "saveload.h"
#include "state.h"
#include "ui.h"

/* ---- addresses (from the listing, written out here) ---------------------------------------------------------- */

#define NAT(n)      (0xF14CCu + 199u * (uint32_t)(n))
#define AR(r)       (0xE31B8u + 69u * (uint32_t)(r))
#define TS(t)       (0xE2C18u + 3u * (uint32_t)(t))
#define LIST        0xF7CA0u
#define LIST_N      0xF7C9Cu
#define BORDERS     0xFFB60u
#define CREC(k)     (0xF77E4u + 3u * (uint32_t)(k))
#define P12         0xF7984u
#define P13         0xF7A24u

/* ---- byte access on the current data object (g_ds: the port's or the model's copy) ---------------------------- */

static uint8_t *R;          /* the port's data object (state_init's) */
static uint8_t *M;          /* the model's copy */
static uint8_t *g_pristine;

static int      rb(uint32_t a) { return g_ds[a - DS_BASE]; }
static int      sb(uint32_t a) { return (int8_t)g_ds[a - DS_BASE]; }
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
static const char *tn(int t) { return str((uint32_t)s32(0xC77D8u + 4u * (uint32_t)t)); }

/* ---- traces --------------------------------------------------------------------------------------------------- */

#define LOGSZ (1u << 21)
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
static int g_steps, g_en2A, g_en3F, g_en40, g_np, g_cuts, g_unexpected;
static uint32_t g_screen;
static jmp_buf g_jb;
static int g_forced[64], g_nforced, g_iforced;   /* hand vectors: these clicks first, then the script */
static int g_yes = -1;                           /* hand vectors: the YES/NO answer (-1 = the script) */
static int g_side = 0;                           /* hand vectors: f5CDE8's answer (0 = the script) */
static long g_cov[24];

static uint32_t srnd(uint32_t n)
{
    g_srng ^= g_srng << 13;
    g_srng ^= g_srng >> 17;
    g_srng ^= g_srng << 5;
    return n ? g_srng % n : 0;
}

static void cut_check(void)
{
    if (++g_steps > 3000) {
        tr("CUT");
        g_cuts++;
        longjmp(g_jb, 1);
    }
}

static int pick(void)
{
    int cnt = s32(LIST_N);
    for (;;) {
        switch (srnd(22)) {
        case 0: return -100;
        case 1: case 2: if (cnt > 0) return (int)srnd((uint32_t)cnt); break;
        case 3: return -9;
        case 4: return -10;
        case 5: if (srnd(3) == 0) return -0x63; break;
        case 6: case 7: case 8: case 9: case 10: case 11: if (g_en3F == 1) return -0x65; break;
        case 12: case 13: if (g_en40 == 1) return -0x66; break;
        case 14: case 15: case 16: if (g_en2A == 1) return -1; break;
        case 17: return -5;
        case 18: return -0x67;
        case 19: return -0x62;
        case 20: return -0xB;
        default: return -0x20;
        }
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

static void eff_78E74(int t, int n)
{
    uint32_t rec = NAT(n);
    int c;
    if (!okt(t) || !okn(n))
        return;
    w8(TS(t) + 2, rb(TS(t) + 2) | 1);
    w8(TS(t), n);
    w8(TS(t) + 1, rb(0xF40FCu));
    c = sb(rec + 0x51);
    if (c >= 0 && c < 9) {
        w8(rec + 0x52u + (uint32_t)c, t + 1);
        w8(rec + 0x51, c + 1);
    }
}

static void eff_57E28(int t, int n, int k)   /* tokens on, the marker, the nation's area list +0x60 / +0x61 */
{
    uint32_t a, rec = NAT(n);
    int i, c;
    if (!okt(t) || !okn(n) || k <= 0)
        return;
    a = AR(t + 1);
    if (sb(a + 1u + (uint32_t)n) != n) {
        w8(a + 1, rb(a + 1) + 1);
        w8(a + 1u + (uint32_t)n, n);
    }
    w8(a + 0x0Eu + (uint32_t)n, rb(a + 0x0Eu + (uint32_t)n) + k);
    c = sb(rec + 0x60);
    for (i = 0; i < c; i++)
        if (rb(rec + 0x61u + (uint32_t)i) == t + 1)
            return;
    if (c >= 0 && c < 0x30) {
        w8(rec + 0x61u + (uint32_t)c, t + 1);
        w8(rec + 0x60, c + 1);
    }
}

static void eff_57CE8(int t, int n, int k)   /* tokens off; at 0 the marker and the list entry go (as f581C4) */
{
    uint32_t a, rec = NAT(n);
    int i, j, c;
    if (!okt(t) || !okn(n) || k <= 0)
        return;
    a = AR(t + 1);
    w8(a + 0x0Eu + (uint32_t)n, rb(a + 0x0Eu + (uint32_t)n) - k);
    if (rb(a + 1u + (uint32_t)n) == 0 || sb(a + 0x0Eu + (uint32_t)n) != 0)
        return;
    w8(a + 1, rb(a + 1) - 1);
    w8(a + 1u + (uint32_t)n, 0);
    c = sb(rec + 0x60);
    for (j = 0; j < c; j++)
        if (rb(rec + 0x61u + (uint32_t)j) == t + 1) {
            for (i = j; i < c - 1; i++)
                w8(rec + 0x61u + (uint32_t)i, rb(rec + 0x62u + (uint32_t)i));
            w8(rec + 0x60, c - 1);
            break;
        }
}

/* ---- the adjacency blocks (the test's own heap: handles tagged 0x40000000, 0x40100000) ------------------------- */

static uint8_t *g_adj[2];
static uint32_t g_adjlen[2];

static int load_adj(const char *gamedir, const char *name, int which, uint32_t ptrs)
{
    char path[1024];
    FILE *fp;
    uint8_t hdr[4], w[2];
    uint32_t size, k = 0, t, i, n;

    snprintf(path, sizeof path, "%s/%s", gamedir, name);
    fp = fopen(path, "rb");
    if (fp == NULL)
        return 0;
    if (fread(hdr, 1, 4, fp) != 4) { fclose(fp); return 0; }
    size = (uint32_t)hdr[0] | (uint32_t)hdr[1] << 8 | (uint32_t)hdr[2] << 16 | (uint32_t)hdr[3] << 24;
    g_adj[which] = calloc(1, size + 64);
    g_adjlen[which] = size + 64;
    if (g_adj[which] == NULL) { fclose(fp); return 0; }
    for (t = 0; t < 160; t++) {
        w32(ptrs + 4u * t, (int32_t)(0x40000000u | (uint32_t)which << 20 | 2u * k));
        if (fread(w, 1, 2, fp) != 2) { fclose(fp); return 0; }
        n = (uint32_t)(int16_t)(w[0] | w[1] << 8);
        if (2u * k + 2u > g_adjlen[which]) { fclose(fp); return 0; }
        g_adj[which][2u * k] = w[0]; g_adj[which][2u * k + 1] = w[1];
        k++;
        for (i = 0; i < n; i++) {
            if (fread(w, 1, 2, fp) != 2 || 2u * k + 2u > g_adjlen[which]) { fclose(fp); return 0; }
            g_adj[which][2u * k] = w[0]; g_adj[which][2u * k + 1] = w[1];
            k++;
        }
    }
    fclose(fp);
    return 1;
}

/* the model's own read of a list word (handle -> the block) */
static int adjw(uint32_t h)
{
    uint32_t which = (h >> 20) & 0xF, off = h & 0xFFFFFu;
    if ((h & 0xC0000000u) != 0x40000000u || which > 1 || g_adj[which] == NULL || off + 2u > g_adjlen[which])
        return 0;
    return (int16_t)(g_adj[which][off] | g_adj[which][off + 1] << 8);
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

/* the three rule helpers formulas.c / helpers2.c link against: never reached from 0x2AC94 / 0x2C778 */
int  f78D08(int t, int n) { tr("UNEXPECTED 78D08 %d %d", t, n); g_unexpected++; return 0; }
void f75CA8(int c, uint32_t rec) { tr("UNEXPECTED 75CA8 %d %X", c, (unsigned)rec); g_unexpected++; }
void f75D84(int c, uint32_t rec, uint32_t giver) { tr("UNEXPECTED 75D84 %d %X %X", c, (unsigned)rec, (unsigned)giver); g_unexpected++; }

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
    if ((h & 0xC0000000u) == 0x40000000u) {   /* the adjacency blocks */
        uint32_t which = (h >> 20) & 0xF;
        if (which > 1 || g_adj[which] == NULL || off > g_adjlen[which] || n > g_adjlen[which] - off)
            return NULL;
        return g_adj[which] + off;
    }
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
void f3CD6C(int8_t pic) { tr("bg %d", pic); }
void f7876C(void) { tr("phase"); }
void f3B05C(uint32_t dlg) { tr("dialog %X", (unsigned)dlg); w32(0xF8E78u, (int32_t)dlg); }
void f459E4(int n) { tr("panel %d", n); }
void f3CBBC(void) { tr("paste"); }
void f3AB7C(const char *text) { tr("box %s", text); }
void f78A20(int t) { tr("wait %d", t); }
void f79270(int wait, int n, int t, const char *text, int anim) { tr("msg %d %d %d %s %d", wait, n, t, text, anim); }
void f3F670(void) { tr("clrb"); memset(g_ds + (BORDERS - DS_BASE), 0, 0xA0); }
void f3F624(int t, int c) { tr("border %d %d", t, c); w8(BORDERS + (uint32_t)t, c); }
void f7853C(uint32_t dlg) { tr("append %X", (unsigned)dlg); g_screen = dlg; }
void f78664(uint32_t dlg) { tr("remove %X", (unsigned)dlg); g_screen = 0; }
void f3AF8C(uint32_t spr, int x, int y) { tr("sprite %X %d %d", (unsigned)spr, x, y); }
void f3A3A0(int x, int y, const char *s) { tr("right %d %d %s", x, y, s); }
void f3A044(int x, int y, const char *s) { tr("print %d %d %s", x, y, s); }
void f3A0B0(int x, int y, int w, const char *s) { tr("text %d %d %d %s", x, y, w, s); }
void f3C9D0(int x, int y, int w, int h, int flags) { tr("frame %d %d %d %d %d", x, y, w, h, flags); }
void f3C3D0(uint32_t dlg, int id)
{
    tr("off %X %X", (unsigned)dlg, (unsigned)id);
    if (id == 0x2A) g_en2A = 0;
    if (id == 0x3F) g_en3F = 0;
    if (id == 0x40) g_en40 = 0;
}
void f3C314(uint32_t dlg, int id)
{
    tr("on %X %X", (unsigned)dlg, (unsigned)id);
    if (id == 0x2A) g_en2A = 1;
    if (id == 0x3F) g_en3F = 1;
    if (id == 0x40) g_en40 = 1;
}
void f3C284(uint32_t dlg, int id, uint32_t label) { tr("label %X %X %s", (unsigned)dlg, (unsigned)id, str(label)); }
void f3BB48(uint32_t dlg, int id, int x) { tr("x %X %X %d", (unsigned)dlg, (unsigned)id, x); }
void f3BFBC(uint32_t dlg, int id, int enabled) { tr("enable %X %X %d", (unsigned)dlg, (unsigned)id, enabled); }
void f3C0D8(uint32_t dlg, int id, int style) { tr("style %X %X %d", (unsigned)dlg, (unsigned)id, style); }
void f3C940(uint32_t dlg) { tr("disable %X", (unsigned)dlg); }
void f3C8C4(uint32_t dlg) { tr("reenable %X", (unsigned)dlg); }
void f3DA60(int t) { tr("centre %d", t); }
void f3D52C(int dx, int dy) { tr("scroll %d %d", dx, dy); }
void f3D9FC(int n) { tr("home %d", n); }
void f78F04(int t, int n) { tr("78F04 %d %d", t, n); eff_78F04(t, n); }
void f78E74(int t, int n) { tr("78E74 %d %d", t, n); eff_78E74(t, n); }
void f57E28(int t, int n, int k) { tr("57E28 %d %d %d", t, n, k); eff_57E28(t, n, k); }
void f57CE8(int t, int n, int flag, int k) { tr("57CE8 %d %d %d %d", t, n, flag, k); eff_57CE8(t, n, k); }
void f1E69C(int k) { tr("done %d", k); }
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
int f5CDE8(int anim, const char *title)
{
    int r = g_side != 0 ? g_side : (srnd(2) ? 12 : 13);
    tr("lossbox %d %s %d %d %d %d -> %d", anim, title, rb(0xF5488u), rb(0xF5498u), rb(0xF5489u), rb(0xF5499u), r);
    return r;
}

/* cal1.c's 0x28CFC for the kinds this module passes (4, 5, 6, 7, 8, 9): the choice list */
static int city_of(int t, int n) { return (rb(TS(t) + 2) & 1) && rb(TS(t)) == n; }
void f28CFC(int n, int kind, int s)
{
    int t, c = 0, take;
    tr("list %d %d %d", n, kind, s);
    w32(LIST_N, 0);
    for (t = 0; t < 157; t++)
        w32(LIST + 4u * (uint32_t)t, -1);
    for (t = 0; t < 157; t++) {
        switch (kind) {
        case 4: case 9: case 0x15: take = city_of(t, n) || rb(AR(t + 1) + 0x0Eu + (uint32_t)n) != 0; break;
        case 5: take = city_of(t, n); break;
        case 6: take = rb(AR(t + 1) + 0x0Eu + (uint32_t)n) != 0; break;
        case 7: take = rb(P12 + (uint32_t)t) != 0; break;
        case 8: take = rb(P13 + (uint32_t)t) != 0; break;
        default: take = 0; break;
        }
        if (take) {
            w32(LIST + 4u * (uint32_t)c, t);
            c++;
            w32(LIST_N, c);
        }
    }
}

int f78BA4(uint32_t dlg, int map)
{
    int r;
    cut_check();
    if (g_iforced < g_nforced)
        r = g_forced[g_iforced++];
    else
        r = pick();
    tr("click %X %d %d", (unsigned)dlg, map, r);
    if (g_ds == R) {
        if (r == -0x65) g_cov[0]++;
        if (r == -0x66) g_cov[1]++;
        if (r == -0x63) g_cov[2]++;
        if (r >= 0) g_cov[3]++;
        if (g_screen != 0xBFE28u) g_cov[19]++;   /* a click wait outside the chooser (none expected) */
    }
    return r;
}

/* the recorders of the calls that are not ported (cal4.c with -DCAL4_TEST) */

/* units of n removed down to target (f27FCC, f2C894): tokens first, then cities; adjust the counters if asked */
static void take_losses(int n, int count, int kind, int target, int counters)
{
    int t, left = count;
    for (t = 0; t < 157 && left > target; t++) {
        if (kind == 7 && !rb(P12 + (uint32_t)t)) continue;
        if (kind == 8 && !rb(P13 + (uint32_t)t)) continue;
        while (left > target && rb(AR(t + 1) + 0x0Eu + (uint32_t)n) != 0) {
            eff_57CE8(t, n, 1);
            if (counters) {
                w8(NAT(n) + 0x98, rb(NAT(n) + 0x98) - 1);
                w8(NAT(n) + 0x99, rb(NAT(n) + 0x99) + 1);
            }
            left--;
        }
    }
    for (t = 0; t < 157 && left > target; t++) {
        if (kind == 7 && !rb(P12 + (uint32_t)t)) continue;
        if (kind == 8 && !rb(P13 + (uint32_t)t)) continue;
        if (city_of(t, n)) {
            eff_78F04(t, n);
            left -= 5;
        }
    }
}

void cal4_test_f2C190(int x, int count, const char *text, int src, int target, int mode, int flag)
{
    int t, left = count;
    tr("2C190 %d %d %s %d %d %d %d", x, count, text, src, target, mode, flag);
    if (g_ds == R) g_cov[4]++;
    for (t = 0; t < 157 && left >= 5; t++)
        if (city_of(t, src)) {
            eff_78F04(t, src);
            eff_78E74(t, target);
            left -= 5;
        }
    for (t = 0; t < 157 && left > 0; t++)
        while (left > 0 && rb(AR(t + 1) + 0x0Eu + (uint32_t)src) != 0) {
            eff_57CE8(t, src, 1);
            w8(NAT(src) + 0x98, rb(NAT(src) + 0x98) - 1);
            w8(NAT(src) + 0x99, rb(NAT(src) + 0x99) + 1);
            eff_57E28(t, target, 1);
            w8(NAT(target) + 0x98, rb(NAT(target) + 0x98) + 1);
            w8(NAT(target) + 0x99, rb(NAT(target) + 0x99) - 1);
            left--;
        }
}

void cal4_test_f2C894(int n, int count, char *text, int ov, int flag, int mode)
{
    tr("2C894 %d %d %s %d %d %d", n, count, text, ov, flag, mode);
    if (g_ds == R) g_cov[5]++;
    snprintf(text, 8, "%d", count);   /* the real one itoa's into the caller's buffer */
    take_losses(n, count, mode, ov, 0);   /* the faction's counters left stale, as the real 0x2C894 does (D) */
}

void cal4_test_f27FCC(int n, int count, const char *text, int kind, int sub, int target)
{
    tr("27FCC %d %d %s %d %d %d", n, count, text, kind, sub, target);
    if (g_ds == R) g_cov[6]++;
    take_losses(n, count, kind, target, 1);
}

void cal4_test_f2D10C(int v, int k, int o, int b)
{
    int t;
    tr("2D10C %d %d %d %d", v, k, o, b);
    if (g_ds == R) g_cov[7]++;
    for (t = 0; t < 157 && k > 0; t++)
        if (city_of(t, o)) {
            w8(AR(t + 1) + 0x3Bu + (uint32_t)v, 1);
            eff_78F04(t, o);
            eff_78E74(t, b);
            k--;
        }
}

void cal4_test_f2D1C0(int v, int k, int o, int b)
{
    int t;
    tr("2D1C0 %d %d %d %d", v, k, o, b);
    if (g_ds == R) g_cov[8]++;
    for (t = 0; t < 157 && k > 0; t++)
        while (k > 0 && rb(AR(t + 1) + 0x0Eu + (uint32_t)o) != 0) {
            w8(AR(t + 1) + 0x3Bu + (uint32_t)v, 1);
            eff_57CE8(t, o, 1);
            w8(NAT(o) + 0x98, rb(NAT(o) + 0x98) - 1);
            w8(NAT(o) + 0x99, rb(NAT(o) + 0x99) + 1);
            eff_57E28(t, b, 1);
            w8(NAT(b) + 0x98, rb(NAT(b) + 0x98) + 1);
            w8(NAT(b) + 0x99, rb(NAT(b) + 0x99) - 1);
            k--;
        }
}

/* ==== the model (g_ds == M), written from the disassembly ======================================================= */

static int g_mut;   /* 0 = the listing; 1.. = deliberately broken (part G) */

static int m_human(int n) { return (rb(0xE1624u) & 0x80) == 0 && sb(NAT(n) + 0x9A) == 3; }
static int m_tok(int t, int n) { return rb(AR(t + 1) + 0x0Eu + (uint32_t)n); }
static void m_inc(uint32_t a) { w8(a, rb(a) + 1); }
static void m_dec(uint32_t a) { w8(a, rb(a) - 1); }

static void m_bg3(void)
{
    f3CD6C(3);
    w8(0xEE168u, 3);
    fg_vbcopy(0, 639, 0, 479, 0, 479, s32(0xF52CCu), s32(0xF52C8u));
}

/* one token of a at t to b with both nations' board / stock bytes */
static void m_tok_move(int t, int a, int b)
{
    f57CE8(t, a, 0, 1);
    m_dec(NAT(a) + 0x98);
    m_inc(NAT(a) + 0x99);
    f57E28(t, b, 1);
    m_inc(NAT(b) + 0x98);
    m_dec(NAT(b) + 0x99);
}

static void m2EE6C(int n, int t, int left)
{
    char b[40];
    fg_vbopen(s32(0xF52C8u));
    f3C9D0(265, 390, 110, 19, 5);
    if (t >= 0) {
        fg_setcolor(5);
        f3A0B0(265, 394, 110, tn(t));
    }
    f3C9D0(310, 370, 19, 19, 5);
    fg_setcolor(5);
    snprintf(b, sizeof b, "%d", left);
    f3A0B0(310, 374, g_mut == 10 ? 19 : 20, b);
    fg_vbclose();
    f3B05C(0xF596Cu);
    f459E4(n);
    f3F670();
    if (t >= 0)
        f3F624(t, 15);
    if (t >= 0)
        f3DA60(t);
    else if (n <= 9)
        f3D9FC(n);
    else
        f3D52C(0, 0);
    f3CBBC();
    f78A20(1500);
}

static int m2D3E0(int n)
{
    char b[120];
    int i, r;
    for (i = 0; i < 14; i++) {
        w8(0xF547Cu + (uint32_t)i, 0);
        w8(0xF548Cu + (uint32_t)i, 0);
    }
    if (g_mut == 11) {
        w8(0xF5488u, rb(0xF1F7Fu));
        w8(0xF5489u, rb(0xF1EB8u));
    } else {
        w8(0xF5488u, rb(0xF1EB8u));
        w8(0xF5489u, rb(0xF1F7Fu));
    }
    w8(0xF5498u, rb(0xF1E71u));
    w8(0xF5499u, rb(0xF1F38u));
    m_bg3();
    f7853C(0xC0214u);
    f3B05C(0xF596Cu);
    f3D9FC(n);
    f3CBBC();
    snprintf(b, sizeof b, "%s, Choose side to play.", nm(n));
    r = f5CDE8(0, b);
    f78664(0xC0214u);
    return r;
}

/* 0x2B6EC's three blocks: areas 0..160, territory states 0..159, nations 1..13 */
static void m_snap(int save)
{
    uint8_t *a = load_heap_ptr((uint32_t)s32(0xEED74u), 161 * 69);
    uint8_t *s = load_heap_ptr((uint32_t)s32(0xEED7Cu), 480);
    uint8_t *n = load_heap_ptr((uint32_t)s32(0xEED78u), 14 * 199);
    int last = g_mut == 13 ? 11 : 13;
    if (a == NULL || s == NULL || n == NULL)
        return;
    if (save) {
        memcpy(a, g_ds + (AR(0) - DS_BASE), 161 * 69);
        memcpy(s, g_ds + (TS(0) - DS_BASE), 480);
        memcpy(n + 199, g_ds + (NAT(1) - DS_BASE), 13 * 199);
    } else {
        memcpy(g_ds + (AR(0) - DS_BASE), a, 161 * 69);
        memcpy(g_ds + (NAT(1) - DS_BASE), n + 199, (size_t)last * 199);
        memcpy(g_ds + (TS(0) - DS_BASE), s, 480);
    }
}

static void m2B6EC(int x, int count, const char *text, int src, int target, int mode)
{
    char b[120];
    int city = 0, left, gone, sel, redraw, r, i, t;
    uint32_t mark;

    w16(0xBB400u, 21);
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
    if (mode == 4) {
        f3A044(336, 374, "SELECT");
        city = 5;
        f3C284(0xBFE28u, 0x3F, 0xB3301u);
        f3C284(0xBFE28u, 0x40, 0xB330Cu);
        f3BB48(0xBFE28u, 0x3F, 219);
        f3BFBC(0xBFE28u, 0x40, 1);
        f3C0D8(0xBFE28u, 0x40, 1);
    } else if (mode == 5) {
        f3A044(336, 374, "ANNEX");
        city = 1;
        f3C284(0xBFE28u, 0x3F, 0xB331Du);
        f3BB48(0xBFE28u, 0x3F, 270);
        f3BFBC(0xBFE28u, 0x40, 0);
        f3C0D8(0xBFE28u, 0x40, 2);
    } else if (mode == 6) {
        f3A044(336, 374, "ANNEX");
        city = 1;
        f3BB48(0xBFE28u, 0x3F, 219);
        f3BFBC(0xBFE28u, 0x40, 1);
        f3C0D8(0xBFE28u, 0x40, 1);
        f3C284(0xBFE28u, 0x3F, 0xB3329u);
        f3C284(0xBFE28u, 0x40, 0xB3333u);
    }
    fg_vbclose();
    f3AB7C(text);
    f7876C();
    f7853C(0xBFE28u);
    f3B05C(0xF596Cu);
    left = count;
    f28CFC(src, mode, 0);
    redraw = 1;
    sel = 0;
    gone = 0;
    for (;;) {
        int empty = s32(LIST_N) <= 0;
        fg_vbopen(s32(0xF52C8u));
        f3C9D0(265, 390, 110, 19, 5);
        fg_setcolor(5);
        if (!empty || g_original)
            f3A0B0(265, 394, 110, tn(s32(LIST + 4u * (uint32_t)sel)));
        f3C9D0(310, 370, 19, 19, 5);
        fg_setcolor(5);
        snprintf(b, sizeof b, "%d", left);
        f3A0B0(310, 374, 19, b);
        fg_vbclose();
        f459E4(x);
        f3F670();
        if (!empty || g_original)
            f3F624(s32(LIST + 4u * (uint32_t)sel), 15);
        if (left != 0) {
            f3C3D0(0xF596Cu, 0x2A);
            f3C314(0xF596Cu, 0x3F);
            f3C314(0xF596Cu, 0x40);
        } else {
            f3C314(0xF596Cu, 0x2A);
            f3C3D0(0xF596Cu, 0x3F);
            f3C3D0(0xF596Cu, 0x40);
        }
        if (left < 0) {
            f3C3D0(0xF596Cu, 0x3F);
            f3C3D0(0xF596Cu, 0x40);
            f3C3D0(0xF596Cu, 0x2A);
        }
        if (redraw) {
            if (s32(LIST_N) > 0)
                f3DA60(s32(LIST + 4u * (uint32_t)sel));
            else
                f3D52C(0, 0);
            redraw = 0;
        }
        f3CBBC();
        r = f78BA4(0xF596Cu, 1);
        if (r == -100)
            continue;
        if (r >= 0) {
            sel = r;
            redraw = 1;
            continue;
        }
        if (r == -1)
            break;
        t = s32(LIST + 4u * (uint32_t)sel);
        mark = AR(t + 1) + 0x3Bu + (uint32_t)x;
        if (r == -0x65 || r == -0x66) {
            if (rb(TS(t) + 2) & 1) {
                if (mode != 4 || g_mut == 9)
                    w8(mark, 1);
                f78F04(t, src);
                f78E74(t, target);
                left -= city;
                gone = 1;
                if (g_ds == M) g_cov[r == -0x65 ? 9 : 10]++;
            } else if (r == -0x65) {
                left--;
                m_tok_move(t, src, target);
                if (g_mut == 5 || sb(AR(t + 1) + 0x0Eu + (uint32_t)src) == 0)
                    gone = 1;
            } else {
                while (left != 0 && m_tok(t, src) != 0) {
                    left--;
                    m_tok_move(t, src, target);
                    if (g_original || g_mut == 4)
                        m_inc(NAT(target) + 0x99);
                }
                if (mode != 4)
                    w8(mark, 1);
                if (sb(AR(t + 1) + 0x0Eu + (uint32_t)src) == 0)
                    gone = 1;
            }
            redraw = 1;
        } else if (r == -0x63) {
            snprintf(b, sizeof b, "Are you sure you want to Restart?");
            if (!(rb(0xF097Cu) & 8) || f795A8(b) == -0x61) {
                m_snap(0);
                left = count;
                f28CFC(src, mode, 0);
                sel = 0;
                redraw = 1;
            }
        } else if (r == -10) {
            if (s32(LIST_N) - 1 > sel) {
                sel++;
                redraw = 1;
            }
        } else if (r == -9) {
            if (sel > 0) {
                sel--;
                redraw = 1;
            }
        }
        if (gone) {
            for (i = sel; i < s32(LIST_N) - 1; i++)
                w32(LIST + 4u * (uint32_t)i, s32(LIST + 4u * (uint32_t)(i + 1)));
            w32(LIST_N, s32(LIST_N) - 1);
            f79A9C(&sel);
            gone = 0;
        }
    }
    f78664(0xBFE28u);
    load_heap_free((uint32_t)s32(0xEED74u));
    w32(0xEED74u, 0);
    load_heap_free((uint32_t)s32(0xEED7Cu));
    w32(0xEED7Cu, 0);
    load_heap_free((uint32_t)s32(0xEED78u));
    w32(0xEED78u, 0);
}

/* 0x2AD50's neighbourhood: arr = the areas of f next to the other faction g */
static int m_has(int t, int f) { return city_of(t, f) || m_tok(t, f) != 0; }

static void m_near(uint32_t arr, int f, int g)
{
    int t, j, n;
    uint32_t h;
    for (t = 0; t < 157; t++) {
        if (!m_has(t, g))
            continue;
        w8(arr + (uint32_t)t, 1);
        h = (uint32_t)s32(0xF8E7Cu + 4u * (uint32_t)t);
        n = adjw(h);
        for (j = 1; j <= n; j++)
            w8(arr + (uint32_t)adjw(h + 2u * (uint32_t)j), 1);
        h = (uint32_t)s32(0xF90FCu + 4u * (uint32_t)t);
        n = adjw(h);
        for (j = 1; j <= n; j++)
            w8(arr + (uint32_t)adjw(h + 2u * (uint32_t)j), 1);
    }
    for (t = 0; t < 157; t++)
        if (rb(arr + (uint32_t)t))
            w8(arr + (uint32_t)t, m_has(t, f));
}

static int m_sum(uint32_t arr, int f)
{
    int t, s = 0;
    for (t = 0; t < 157; t++)
        if (rb(arr + (uint32_t)t)) {
            if (city_of(t, f))
                s += 5;
            else
                s += sb(AR(t + 1) + 0x0Eu + (uint32_t)f);
        }
    return s;
}

static int m_auto(uint32_t arr, int f, uint32_t msg)
{
    int left = 5, t, x;
    m_bg3();
    fg_vbopen(s32(0xF52C8u));
    fg_setcolor(5);
    f3A3A0(303, 374, "LEFT TO");
    f3A044(336, 374, "REMOVE");
    fg_vbclose();
    f3AB7C(str(msg));
    f7876C();
    f3B05C(0xF596Cu);
    f3C940((uint32_t)s32(0xF8E78u));
    m2EE6C(f, -1, left);
    for (t = 0; t < 157; t++) {
        if (!rb(arr + (uint32_t)t))
            continue;
        if (city_of(t, f)) {
            f78F04(t, f);
            left -= 5;
        } else {
            x = sb(AR(t + 1) + 0x0Eu + (uint32_t)f);
            if (x == 0)
                continue;
            f57CE8(t, f, 0, x);
            w8(NAT(f) + 0x98, rb(NAT(f) + 0x98) - x);
            w8(NAT(f) + 0x99, rb(NAT(f) + 0x99) + x);
            left -= x;
        }
        w8(arr + (uint32_t)t, 0);
        m2EE6C(f, t, left);
    }
    f3C8C4((uint32_t)s32(0xF8E78u));
    if (g_ds == M) g_cov[11]++;
    return left;
}

static void m2AD50(int v, int ben)
{
    char b[100];
    int a1, a2, r1 = 5, r2 = 5, mode, ov, flag;
    memset(g_ds + (P12 - DS_BASE), 0, 157);
    memset(g_ds + (P13 - DS_BASE), 0, 157);
    m_near(P13, 13, 12);
    m_near(P12, 12, 13);
    a1 = m_sum(P12, 12);
    a2 = m_sum(P13, 13);
    if (a1 > 0 && (g_mut == 7 ? a1 < 5 : a1 <= 5))
        r1 = m_auto(P12, 12, 0xB3173u);
    mode = a1 <= 5 ? (g_mut == 6 ? 7 : 9) : 7;
    ov = f2AC94(12, r1);
    flag = f2C778(v) != 12;
    if (r1 > 0) {
        if (m_human(v)) {
            snprintf(b, sizeof b, "%s, choose MILITARY losses for First Faction.", nm(v));
            cal4_test_f27FCC(12, r1, b, mode, 0, ov);
        } else {
            snprintf(b, sizeof b, "%s choosing MILITARY losses for First Faction.", nm(v));
            cal4_test_f2C894(12, r1, b, ov, flag, mode);
        }
    }
    if (a2 > 0 && a2 <= 5)
        r2 = m_auto(P13, 13, 0xB322Cu);
    if (sb(0xF1F7Fu) != 0 || sb(0xF1F38u) != 0) {
        mode = a2 <= 5 ? 9 : 8;
        ov = f2AC94(13, r2);
        if (r2 > 0) {
            if (m_human(ben)) {
                snprintf(b, sizeof b, "%s, choose MILITARY losses for Second Faction.", nm(ben));
                cal4_test_f27FCC(13, r2, b, mode, 0, ov);
            } else {
                snprintf(b, sizeof b, "%s choosing MILITARY losses for Second Faction.", nm(ben));
                cal4_test_f2C894(13, r2, b, ov, !flag, mode);
            }
        }
    }
}

static void m29E34(int k)
{
    char b[160];
    int nat[9], key[9], c = 0, v, i, j, x, f1, f2, ben, ch, ann, o, t, idx, last;
    const char *bn = g_original ? "beneficary" : "beneficiary";
    if (g_mut == 12)
        bn = "beneficary";

    memset(nat, 0, sizeof nat);
    memset(key, 0, sizeof key);
    v = sb(CREC(k) + 2);
    w32(0xF40F8u, v);
    for (i = 1; i <= 9; i++)
        if (rb(NAT(i) + 0x9A)) {
            nat[c] = i;
            key[c] = (int16_t)((9 - sb(NAT(i) + 0x51)) * 5 + sb(NAT(i) + 0x99));
            c++;
        }
    last = s32(0xF40ECu) - 2;
    if (last > 7)
        last = 7;
    for (i = last; i >= 0; i--)
        for (j = 0; j <= i; j++)
            if (g_mut == 1 ? key[j + 1] >= key[j] : key[j + 1] > key[j]) {
                x = key[j]; key[j] = key[j + 1]; key[j + 1] = x;
                x = nat[j]; nat[j] = nat[j + 1]; nat[j + 1] = x;
            }
    if (nat[0] == v) {
        snprintf(b, sizeof b, "%s has the most units in stock so there is no Civil War.", nm(v));
        f79270(-500, v, -200, b, 1);
        f1E69C(k);
        if (g_ds == M) g_cov[12]++;
        return;
    }
    if (rb(NAT(v) + 0x36)) {
        f1 = 0;
        f2 = 15;
    } else {
        f1 = 15 + (rb(NAT(v) + 0x30) ? 5 : 0) + (rb(NAT(v) + 0x2F) ? 5 : 0) + (rb(NAT(v) + 0x34) ? (g_mut == 2 ? 5 : 10) : 0);
        f2 = 20;
    }
    if (f1 + f2 >= 5 * sb(NAT(v) + 0x51) + sb(NAT(v) + 0x98)) {
        snprintf(b, sizeof b, "There are no units available for a second faction, so %s quells Civil War with no losses.",
                 nm(v));
        f79270(-500, v, -200, b, 1);
        f1E69C(k);
        if (g_ds == M) g_cov[13]++;
        return;
    }
    snprintf(b, sizeof b, "Civil War breaks out in %s.", nm(v));
    f79270(-500, v, -200, b, 1);
    ben = nat[0];
    if (f1 > 0) {
        if (m_human(v)) {
            snprintf(b, sizeof b, "%s, you are the victim of Civil War. Choose units for the first faction.", nm(v));
            m2B6EC(v, f1, b, v, 12, 4);
        } else {
            snprintf(b, sizeof b, "%s is the victim of Civil War. Choosing units for the first faction.", nm(v));
            cal4_test_f2C190(v, f1, b, v, 12, 4, 0);
        }
    }
    if (f2 > 0) {
        if (m_human(ben)) {
            if (f1 > 0)
                snprintf(b, sizeof b, "%s, as beneficiary of the Civil War, choose units for the first faction.", nm(ben));
            else
                snprintf(b, sizeof b, "%s, you are the victim of Civil War but %s will choose units for the first faction.",
                         nm(v), nm(ben));
            m2B6EC(v, f2, b, v, 12, 4);
        } else {
            if (f1 > 0)
                snprintf(b, sizeof b, "%s is a beneficiary of the Civil War. Choosing units for the first faction.", nm(ben));
            else
                snprintf(b, sizeof b, "%s is the victim of Civil War but %s is choosing units for the first faction.",
                         nm(v), nm(ben));
            cal4_test_f2C190(v, f2, b, v, 12, 4, 1);
        }
    }
    for (t = 0; t < 157; t++) {
        if (city_of(t, v)) {
            f78F04(t, v);
            f78E74(t, 13);
        } else {
            while (m_tok(t, v))
                m_tok_move(t, v, 13);
        }
    }
    if (rb(NAT(v) + 0x35)) {
        m2AD50(v, ben);
        if (g_ds == M) g_cov[14]++;
    }
    if (sb(0xF1F7Fu) == 0 && sb(0xF1F38u) == 0) {
        snprintf(b, sizeof b,
                 "The Second Faction no longer exists due to Military losses.  %s will take control of the First Faction",
                 nm(v));
        f79270(-500, v, -200, b, 1);
        ch = 12;
        ann = 0;
        if (g_ds == M) g_cov[15]++;
    } else {
        if (m_human(v)) {
            ch = m2D3E0(v);
        } else {
            ch = f2C778(v);
            snprintf(b, sizeof b, "%s chooses to play the %s Faction.", nm(v), ch == 12 ? "First" : "Second");
            f79270((rb(0xF0980u) & 4) ? -500 : 2500, v, -200, b, 1);
        }
        ann = 1;
    }
    o = ch == 12 ? 13 : 12;
    for (t = 0; t < 157; t++) {
        if (city_of(t, ch)) {
            f78F04(t, ch);
            f78E74(t, v);
        } else {
            while (m_tok(t, ch))
                m_tok_move(t, ch, v);
        }
    }
    if (!ann) {
        f1E69C(k);
        return;
    }
    w8(NAT(v) + 0x9B, 1);
    if ((9 - sb(NAT(ben) + 0x51)) * 5 + sb(NAT(ben) + 0x99) >= 5 * sb(NAT(o) + 0x51) + sb(NAT(o) + 0x98) &&
        9 - sb(NAT(ben) + 0x51) >= sb(NAT(o) + 0x51) &&
        (g_mut == 3 || sb(NAT(ben) + 0x99) >= sb(NAT(o) + 0x98))) {
        for (t = 0; t < 157; t++) {
            if (city_of(t, o)) {
                f78F04(t, o);
                f78E74(t, ben);
                w8(AR(t + 1) + 0x3Bu + (uint32_t)v, 1);
                continue;
            }
            while (m_tok(t, o)) {
                m_tok_move(t, o, ben);
                if (sb(AR(t + 1) + 0x0Eu + (uint32_t)v) == 0)
                    w8(AR(t + 1) + 0x3Bu + (uint32_t)v, 1);
            }
        }
        snprintf(b, sizeof b, "%s annexes the %s faction.", nm(ben), o == 13 ? "second" : "first");
        f79270(-500, ben, -200, b, 1);
        f1E69C(k);
        if (g_ds == M) g_cov[16]++;
        return;
    }
    if (g_ds == M) g_cov[17]++;
    for (idx = 0; 5 * sb(NAT(o) + 0x51) + sb(NAT(o) + 0x98) > 0 && idx < c; idx++) {
        int room, stock;
        ben = nat[idx];
        if (ben == v && g_mut != 14)
            continue;
        room = 9 - sb(NAT(ben) + 0x51);
        if (sb(NAT(o) + 0x51) > 0) {
            if (room >= sb(NAT(o) + 0x51)) {
                for (t = 0; t < 157; t++)
                    if (city_of(t, o)) {
                        f78F04(t, o);
                        f78E74(t, ben);
                    }
                snprintf(b, sizeof b, "%s annexes %s faction cities.", nm(ben), o == 13 ? "second" : "first");
                f79270(-500, ben, -200, b, 1);
            } else if (m_human(ben)) {
                snprintf(b, sizeof b, "%s, you are a %s of Civil War. Choose cities to annex.", nm(ben), bn);
                m2B6EC(v, room, b, o, ben, 5);
            } else {
                snprintf(b, sizeof b, "%s is a %s of Civil War, but cannot fully absorb the %s Faction cities.  "
                         "Choosing cities to annex.", nm(ben), bn, g_original || o == 13 ? "Second" : "First");
                f79270(2500, ben, -200, b, 1);
                cal4_test_f2D10C(v, room, o, ben);
            }
        }
        if (rb(NAT(o) + 0x98) == 0)
            continue;
        stock = sb(NAT(ben) + 0x99);
        if (sb(NAT(o) + 0x98) > stock) {
            if (m_human(ben)) {
                snprintf(b, sizeof b, "%s, you are a %s of Civil War. Choose tokens to annex.", nm(ben), bn);
                m2B6EC(v, stock, b, o, ben, 6);
            } else {
                snprintf(b, sizeof b, "%s is a %s of Civil War, but cannot fully absorb the %s Faction tokens. "
                         "Choosing tokens to annex.", nm(ben), bn, g_original || o == 13 ? "Second" : "First");
                f79270(2500, ben, -200, b, 1);
                cal4_test_f2D1C0(v, stock, o, ben);
            }
        } else {
            for (t = 0; t < 157; t++)
                while (m_tok(t, o))
                    m_tok_move(t, o, ben);
            snprintf(b, sizeof b, "%s annexes %s faction tokens.", nm(ben), o == 13 ? "second" : "first");
            f79270(-500, g_mut == 8 ? ben : v, -200, b, 1);
        }
    }
    if (5 * sb(NAT(o) + 0x51) + sb(NAT(o) + 0x98) > 0) {
        if (g_ds == M) g_cov[18]++;
        for (t = 0; t < 157; t++) {
            if (city_of(t, o))
                f78F04(t, o);
            while (m_tok(t, o)) {
                f57CE8(t, o, 0, 1);
                m_dec(NAT(o) + 0x98);
                m_inc(NAT(o) + 0x99);
            }
        }
    }
    f1E69C(k);
}

/* ==== harness ===================================================================================================== */

enum { R_29E34, R_2AD50, R_2B6EC, R_2D3E0, R_2EE6C };

static int g_checks, g_fail;
static uint32_t g_rng = 12345u;
static int g_a[6], g_ret_real, g_ret_model;
static char g_text[100];

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

static int call(int which, int model)
{
    switch (which) {
    case R_29E34: if (model) m29E34(g_a[0]); else f29E34(g_a[0]); return 0;
    case R_2AD50: if (model) m2AD50(g_a[0], g_a[1]); else f2AD50(g_a[0], g_a[1]); return 0;
    case R_2B6EC:
        if (model) m2B6EC(g_a[0], g_a[1], g_text, g_a[2], g_a[3], g_a[4]);
        else f2B6EC(g_a[0], g_a[1], g_text, g_a[2], g_a[3], g_a[4]);
        return 0;
    case R_2D3E0: return model ? m2D3E0(g_a[0]) : f2D3E0(g_a[0]);
    case R_2EE6C: if (model) m2EE6C(g_a[0], g_a[1], g_a[2]); else f2EE6C(g_a[0], g_a[1], g_a[2]); return 0;
    }
    return 0;
}

static void side(int model, int which, uint32_t seed)
{
    int r = 0;
    g_log = model ? g_model : g_real;
    g_len = model ? &g_model_len : &g_real_len;
    *g_len = 0;
    g_log[0] = 0;
    g_ds = model ? M : R;
    g_srng = seed * 2654435761u + 1u;
    g_steps = 0;
    g_en2A = g_en3F = g_en40 = -1;
    g_screen = 0;
    g_iforced = 0;
    heap_reset();
    if (setjmp(g_jb) == 0)
        r = call(which, model);
    if (model) g_ret_model = r; else g_ret_real = r;
    g_ds = R;
}

/* runs the port and the model from the same state; returns 1 when both agree (quiet: no FAIL line, for part G) */
static int compare(int which, uint32_t seed, const char *what, int quiet)
{
    size_t i;
    const char *a, *b;
    int line = 1;

    memcpy(M, R, DS_SIZE);
    side(0, which, seed);
    side(1, which, seed);
    if (strcmp(g_real, g_model) != 0) {
        if (quiet)
            return 0;
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
        return 0;
    }
    for (i = 0; i < DS_SIZE; i++)
        if (R[i] != M[i]) {
            if (!quiet)
                printf("FAIL %s: byte 0x%X port %02X model %02X\n", what, (unsigned)(i + DS_BASE), R[i], M[i]);
            return 0;
        }
    if (g_ret_real != g_ret_model) {
        if (!quiet)
            printf("FAIL %s: return %d / %d\n", what, g_ret_real, g_ret_model);
        return 0;
    }
    return 1;
}

static int run_both(int which, uint32_t seed, const char *what)
{
    int ok = compare(which, seed, what, 0);
    g_checks++;
    if (!ok)
        g_fail++;
    return ok;
}

static int has(const char *s) { return strstr(g_real, s) != NULL; }

/* ---- states ---------------------------------------------------------------------------------------------------- */

static void put_tokens(int t, int n, int k) { eff_57E28(t, n, k); }

static int put_city(int t, int n)
{
    if (sb(NAT(n) + 0x51) >= 9 || (rb(TS(t) + 2) & 1))
        return 0;
    eff_78E74(t, n);
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
        memset(g_ds + (rec + 0x61 - DS_BASE), 0, 0x30);
        w8(rec + 0x98, 0);
        w8(rec + 0x99, 0);
    }
}

static void base_clean(void)
{
    int n, a;
    memcpy(R, g_pristine, DS_SIZE);
    g_ds = R;
    clear_board();
    for (n = 1; n <= 13; n++) {
        w8(NAT(n) + 0x9A, n <= 9 ? 3 : 0);
        for (a = 0x24; a <= 0x3B; a++)
            w8(NAT(n) + (uint32_t)a, 0);
        w8(NAT(n) + 0x99, n <= 9 ? 30 : 0);
        w8(NAT(n) + 0x9B, 0);
        w8(NAT(n) + 0x3D, n);
    }
    w32(0xF40ECu, 9);
    w16(0xF40FCu, 3);
    w8(0xE1624u, rb(0xE1624u) & 0x6F);
    w8(0xF0980u, rb(0xF0980u) & ~4);
    w8(0xF097Cu, rb(0xF097Cu) & ~8);
    w8(0xF4102u, 15);
    g_nforced = 0;
    g_yes = -1;
    g_side = 0;
    g_original = 0;
}

/* tokens on the board with the board counter kept */
static void give_tokens(int t, int n, int k)
{
    put_tokens(t, n, k);
    w8(NAT(n) + 0x98, rb(NAT(n) + 0x98) + k);
}

/* a random game state on top of the current R */
static void random_world(void)
{
    int n, t, k, a, inplay = 0;
    g_ds = R;
    clear_board();
    for (n = 1; n <= 9; n++) {
        w8(NAT(n) + 0x9A, rnd(7) == 0 ? 0 : rnd(3) == 0 ? 2 : 3);
        inplay += rb(NAT(n) + 0x9A) != 0;
        for (a = 0x24; a <= 0x3B; a++)
            w8(NAT(n) + (uint32_t)a, rnd(5) == 0);
        w8(NAT(n) + 0x99, (int)rnd(40));
        w8(NAT(n) + 0x9B, 0);
        w8(NAT(n) + 0x3D, n);
    }
    if (inplay < 2) {
        w8(NAT(1) + 0x9A, 3);
        w8(NAT(2) + 0x9A, 3);
        inplay = 2;
        for (n = 3; n <= 9; n++)
            inplay += rb(NAT(n) + 0x9A) != 0;
    }
    w32(0xF40ECu, rnd(10) == 0 ? inplay - 1 : inplay);   /* now and then a stale player count */
    for (n = 1; n <= 10; n++) {
        if (n <= 9 && rb(NAT(n) + 0x9A) == 0)
            continue;
        k = (int)rnd(10);
        for (a = 0; a < 40 && sb(NAT(n) + 0x51) < k; a++)
            put_city((int)rnd(157), n);
    }
    for (k = 0; k < 140; k++) {
        n = 1 + (int)rnd(9);
        t = (int)rnd(157);
        if (rb(NAT(n) + 0x9A) && !(rb(TS(t) + 2) & 1))
            give_tokens(t, n, 1 + (int)rnd(4));
    }
    /* now and then stale faction records (counters left by an earlier Civil War or by 0x2C894) */
    for (n = 12; n <= 13; n++) {
        w8(NAT(n) + 0x9A, rnd(3) == 0 ? 3 : 0);
        w8(NAT(n) + 0x99, rnd(4) == 0 ? (int)rnd(256) : 0);
        if (rnd(8) == 0)
            w8(NAT(n) + 0x98, (int)rnd(6));
    }
    w8(0xE1624u, (rb(0xE1624u) & 0x6F) | (rnd(20) == 0 ? 0x80 : 0));
    w8(0xF0980u, (rb(0xF0980u) & ~4) | (rnd(2) ? 4 : 0));
    w8(0xF097Cu, (rb(0xF097Cu) & ~8) | (rnd(2) ? 8 : 0));
    w16(0xF40FCu, 1 + (int)rnd(12));
    for (t = 0; t < 157; t++) {   /* the AI value table 0x2C778 reads */
        if (rnd(4) == 0)
            w8(0xEED84u + 160u * (1u + rnd(9)) + (uint32_t)t, (int)rnd(256));
    }
}

/* the victim v holds the Civil War card as record k */
static void set_victim(int k, int v)
{
    w8(CREC(k), 0x17);
    w8(CREC(k) + 1, 0xFF);
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

static void force(const int *codes, int n)
{
    int i;
    for (i = 0; i < n && i < 64; i++)
        g_forced[i] = codes[i];
    g_nforced = n;
}

/* ---- A. hand vectors ------------------------------------------------------------------------------------------- */

static int tokat(int t, int n) { return rb(AR(t + 1) + 0x0Eu + (uint32_t)n); }
static int markat(int t, int v) { return rb(AR(t + 1) + 0x3Bu + (uint32_t)v); }

/* x is in q's list (base 0xF8E7C land, 0xF90FC coast) */
static int in_list(uint32_t base, int q, int x)
{
    uint32_t h = (uint32_t)s32(base + 4u * (uint32_t)q);
    int j, n = adjw(h);
    for (j = 1; j <= n; j++)
        if (adjw(h + 2u * (uint32_t)j) == x)
            return 1;
    return 0;
}

/* territory q is next to t (either list of either territory names the other) */
static int adjacent(int q, int t)
{
    int j, n, w;
    uint32_t h;
    for (w = 0; w < 2; w++) {
        h = (uint32_t)s32((w ? 0xF90FCu : 0xF8E7Cu) + 4u * (uint32_t)q);
        n = adjw(h);
        for (j = 1; j <= n; j++)
            if (adjw(h + 2u * (uint32_t)j) == t)
                return 1;
        h = (uint32_t)s32((w ? 0xF90FCu : 0xF8E7Cu) + 4u * (uint32_t)t);
        n = adjw(h);
        for (j = 1; j <= n; j++)
            if (adjw(h + 2u * (uint32_t)j) == q)
                return 1;
    }
    return 0;
}

static void part_a(void)
{
    int f0 = g_fail, i, o, t, n;
    char e[300];

    /* A1 the beneficiary is the victim: no Civil War */
    base_clean();
    w8(NAT(4) + 0x99, 40);   /* 4 has the most in stock */
    set_victim(2, 4);
    g_a[0] = 2;
    run_both(R_29E34, 1, "A1");
    snprintf(e, sizeof e, "msg -500 4 -200 %s has the most units in stock so there is no Civil War. 1\ndone 2\n", nm(4));
    expect(strcmp(g_real, e) == 0, "A1 the message and 0x1E69C only");
    expect(s32(0xF40F8u) == 4, "A1 [0xF40F8] = the victim");

    /* A2 ties: the lower nation first (only a strictly greater key swaps) */
    base_clean();
    set_victim(0, 1);
    g_a[0] = 0;
    run_both(R_29E34, 1, "A2 all tied, victim 1");
    expect(has("there is no Civil War"), "A2 all tied: nation 1 ranks first, no Civil War for 1");
    base_clean();
    put_city(10, 1);   /* 1 now ranks lower: 70 */
    for (i = 0; i < 9; i++) put_city(20 + i, 3);
    give_tokens(40, 3, 30);
    set_victim(0, 3);
    run_both(R_29E34, 1, "A2 victim 3, beneficiary 2");
    expect(has("Civil War breaks out"), "A2 Civil War for 3");
    snprintf(e, sizeof e, "box %s, as beneficiary of the Civil War, choose units for the first faction.", nm(2));
    expect(has(e), "A2 nation 2 is the beneficiary (the first of the tied)");

    /* A3 quelled: f1 + f2 = 35 >= units */
    base_clean();
    for (i = 0; i < 6; i++) put_city(30 + i, 5);
    give_tokens(50, 5, 5);   /* 35 units */
    w8(NAT(5) + 0x99, 0);
    set_victim(0, 5);
    run_both(R_29E34, 1, "A3 quelled 35");
    expect(has("quells Civil War with no losses") && has("done 0") && !has("breaks out"), "A3 35 units: quelled");
    give_tokens(51, 5, 1);   /* 36 */
    run_both(R_29E34, 1, "A3 36");
    expect(has("breaks out") && has("list 5 4 0") && has("text 310 374 19 15"), "A3 36 units: the victim chooses 15");
    /* Democracy +10, Music +5, Drama & Poetry +5: f1 = 35, so 55 units are quelled */
    base_clean();
    for (i = 0; i < 9; i++) put_city(30 + i, 5);
    give_tokens(50, 5, 10);
    w8(NAT(5) + 0x99, 0);
    w8(NAT(5) + 0x34, 1); w8(NAT(5) + 0x30, 1); w8(NAT(5) + 0x2F, 1);
    set_victim(0, 5);
    run_both(R_29E34, 1, "A3 advances quelled");
    expect(has("quells"), "A3 35 + 20 >= 55: quelled");
    give_tokens(51, 5, 1);
    run_both(R_29E34, 1, "A3 advances");
    expect(has("text 310 374 19 35"), "A3 the first faction's count is 35");
    /* Philosophy: f1 = 0, the beneficiary chooses 15 for the first faction */
    base_clean();
    for (i = 0; i < 4; i++) put_city(30 + i, 5);
    give_tokens(50, 5, 1);   /* 21 > 15 */
    w8(NAT(5) + 0x99, 0);
    w8(NAT(5) + 0x36, 1);
    set_victim(0, 5);
    run_both(R_29E34, 1, "A3 Philosophy");
    snprintf(e, sizeof e, "box %s, you are the victim of Civil War but %s will choose units for the first faction.",
             nm(5), nm(1));
    expect(has(e) && has("text 310 374 19 15"), "A3 Philosophy: the beneficiary's 15, the message to the victim");
    expect(strstr(g_real, "text 310 374 19 15") == strstr(g_real, "text 310 374 19"), "A3 Philosophy: one chooser only");

    /* A4 a whole run, all human: victim 5 (cities 30, 31, tokens 10 at 40, 41, 42), beneficiary 1 */
    for (i = 0; i < 4; i++) {
        static const int c[] = { -0x66, -0x66, -0x66, -1, -0x66, -0x66, -0x66, -1 };
        base_clean();
        g_original = i & 1;
        put_city(30, 5); put_city(31, 5);
        give_tokens(40, 5, 10); give_tokens(41, 5, 10); give_tokens(42, 5, 10);
        w8(NAT(5) + 0x99, 0);
        w8(NAT(1) + 0x99, 40);
        set_victim(1, 5);
        g_a[0] = 1;
        g_side = i < 2 ? 12 : 13;
        force(c, 8);   /* the victim: ALL 30, ALL 31, ALL 40 (5 of 10), FINISHED; 1: ALL 40, ALL 41, ALL 42 (5), FINISHED */
        snprintf(e, sizeof e, "A4 side %d original %d", g_side, g_original);
        run_both(R_29E34, 77, e);
        expect(has("annexes the") && rb(NAT(5) + 0x9B) == 1, "A4 the whole annexation, +0x9B");
        if (g_side == 12) {
            expect(city_of(30, 5) && city_of(31, 5) && tokat(40, 5) == 10 && tokat(41, 5) == 10 && tokat(42, 5) == 5,
                   "A4 side 12: the first faction back to the victim");
            expect(tokat(42, 1) == 5 && markat(42, 5) == 0 && has("annexes the second faction"),
                   "A4 side 12: the second faction's 5 tokens to 1, no mark (the victim is there)");
            expect(sb(NAT(5) + 0x98) == 25 && sb(NAT(5) + 0x99) == 5, "A4 side 12: the victim's counters");
            expect(sb(NAT(12) + 0x99) == (g_original ? 25 : 0),
                   g_original ? "A4 original: faction 12's stock drifts by the 25 ALL tokens"
                              : "A4 fixed: faction 12's stock back to 0");
        } else {
            expect(city_of(30, 1) && city_of(31, 1) && markat(30, 5) == 1 && markat(31, 5) == 1,
                   "A4 side 13: the first faction's cities to 1, marked");
            expect(tokat(40, 1) == 10 && tokat(41, 1) == 10 && tokat(42, 1) == 5 && tokat(42, 5) == 5 &&
                   markat(40, 5) == 1 && markat(41, 5) == 1 && markat(42, 5) == 0,
                   "A4 side 13: its tokens to 1, marked where the victim has none");
            expect(has("annexes the first faction") && sb(NAT(1) + 0x99) == 15, "A4 side 13: 1 paid 25 tokens");
        }
    }
    g_original = 0;

    /* A5 ALL on tokens in mode 6 (the beneficiary's annexing): fixed, the target pays from its stock */
    for (i = 0; i < 2; i++) {
        static const int c[] = { -0x66, -1 };
        base_clean();
        g_original = i;
        give_tokens(60, 13, 8);
        w8(NAT(13) + 0x99, -8);
        w8(NAT(2) + 0x99, 6);
        g_a[0] = 5; g_a[1] = 6; g_a[2] = 13; g_a[3] = 2; g_a[4] = 6;
        snprintf(g_text, sizeof g_text, "annex tokens");
        force(c, 2);
        run_both(R_2B6EC, 5, i ? "A5 original" : "A5");
        expect(tokat(60, 2) == 6 && sb(NAT(2) + 0x98) == 6 && tokat(60, 13) == 2, "A5 six tokens to 2");
        expect(sb(NAT(2) + 0x99) == (i ? 6 : 0), i ? "A5 original: 2's stock unchanged" : "A5 fixed: 2 paid 6");
        expect(markat(60, 5) == 1, "A5 ALL marks the area for the victim");
        expect(has("label BFE28 3F ANNEX ONE") && has("label BFE28 40 ANNEX ALL") && has("x BFE28 3F 219") &&
               has("print 336 374 ANNEX") && has("box annex tokens") && has("append BFE28"), "A5 mode 6's screen");
        expect(has("free ") && s32(0xEED74u) == 0 && s32(0xEED78u) == 0 && s32(0xEED7Cu) == 0 && has("remove BFE28"),
               "A5 blocks freed, dialog removed");
    }
    g_original = 0;
    /* ONE on a token pays and does not mark */
    {
        static const int c[] = { -0x65, -1 };
        base_clean();
        give_tokens(60, 13, 3);
        g_a[0] = 5; g_a[1] = 1; g_a[2] = 13; g_a[3] = 2; g_a[4] = 6;
        force(c, 2);
        run_both(R_2B6EC, 5, "A5 ONE");
        expect(markat(60, 5) == 0 && sb(NAT(2) + 0x99) == 29 && tokat(60, 2) == 1, "A5 ONE: paid, no mark");
    }
    /* mode 5: ANNEX, the ALL button hidden (style 2) yet enabled; a city counts 1 */
    {
        static const int c[] = { -0x66, -1 };
        base_clean();
        put_city(70, 12); put_city(71, 12);
        g_a[0] = 5; g_a[1] = 1; g_a[2] = 12; g_a[3] = 3; g_a[4] = 5;
        force(c, 2);
        run_both(R_2B6EC, 5, "A5 mode 5");
        expect(city_of(70, 3) && city_of(71, 12) && markat(70, 5) == 1, "A5 mode 5: the invisible ALL takes one city");
        expect(has("style BFE28 40 2") && has("enable BFE28 40 0") && has("on F596C 40") && has("x BFE28 3F 270"),
               "A5 mode 5: ALL hidden, then on");
    }
    /* mode 4, a city with 3 left: -2, everything off; RESTART (confirmed) brings it back */
    {
        static const int c[] = { -0x65, -0x63, 1, -0x66, -1 };
        base_clean();
        put_city(70, 5); give_tokens(80, 5, 2);
        g_a[0] = 5; g_a[1] = 3; g_a[2] = 5; g_a[3] = 12; g_a[4] = 4;
        force(c, 5);
        g_yes = -0x61;
        w8(0xF097Cu, rb(0xF097Cu) | 8);
        run_both(R_2B6EC, 5, "A5 mode 4 overshoot");
        expect(has("text 310 374 19 -2") && has("yesno Are you sure you want to Restart? -97"), "A5 left -2, restart asked");
        expect(city_of(70, 5) && tokat(80, 12) == 2 && tokat(80, 5) == 0 && sb(NAT(12) + 0x99) == -2,
               "A5 after RESTART: the city back, the 2 tokens to faction 12");
        expect(has("text 310 374 19 1"), "A5 one left at FINISHED");
    }
    /* reviewer: mode 4, the only unit a city taken with 3 left: the list empties (left -2); the original then shows
       the stale entry (the territory just taken: name and highlight), the fix shows neither (BUG(1995) 4) */
    for (i = 0; i < 2; i++) {
        static const int c[] = { -0x65, -1 };
        const char *p;
        int nb = 0, nt = 0;
        base_clean();
        g_original = i;
        g_yes = -1;
        put_city(70, 5);
        g_a[0] = 5; g_a[1] = 3; g_a[2] = 5; g_a[3] = 12; g_a[4] = 4;
        force(c, 2);
        run_both(R_2B6EC, 5, i ? "A5 emptied list original" : "A5 emptied list");
        for (p = g_real; (p = strstr(p, "border 70 15")) != NULL; p++) nb++;
        snprintf(e, sizeof e, "text 265 394 110 %s", tn(70));
        for (p = g_real; (p = strstr(p, e)) != NULL; p++) nt++;
        expect(s32(LIST_N) == 0 && has("text 310 374 19 -2") && city_of(70, 12), "A5 emptied list: left -2, city taken");
        expect(nb == (i ? 2 : 1) && nt == (i ? 2 : 1),
               i ? "A5 emptied list original: the stale entry shown again" : "A5 emptied list fixed: nothing shown");
    }
    g_original = 0;

    /* A6 the texts of a computer's annexing when the victim plays the second faction (o = 12) */
    for (i = 0; i < 2; i++) {
        static const int c[] = { -0x65, -0x65, -0x65, -1 };
        base_clean();
        g_original = i;
        for (n = 1; n <= 9; n++) w8(NAT(n) + 0x99, 0);
        w8(NAT(2) + 0x9A, 2);
        w8(NAT(2) + 0x99, 2);   /* 2: 8 cities, key 7, the beneficiary */
        t = 0;
        for (n = 1; n <= 9; n++)
            if (n != 5)
                for (o = 0; o < (n == 2 ? 8 : 9); o++) put_city(t++, n);
        for (o = 0; o < 9; o++) put_city(100 + o, 5);
        give_tokens(120, 5, 20);
        set_victim(0, 5);
        g_side = 13;
        force(c, 4);   /* the victim: ONE on three cities */
        g_a[0] = 0;
        run_both(R_29E34, 9, i ? "A6 original" : "A6");
        expect(has("2C190 5 20 "), "A6 the computer beneficiary's 20 through 0x2C190");
        snprintf(e, sizeof e, "msg 2500 2 -200 %s is a %s of Civil War, but cannot fully absorb the %s Faction cities.  "
                 "Choosing cities to annex. 1", nm(2), i ? "beneficary" : "beneficiary", i ? "Second" : "First");
        expect(has(e), i ? "A6 original text" : "A6 fixed text (beneficiary, First)");
        expect(has("2D10C 5 1 12 2"), "A6 0x2D10C(v, room 1, faction 12, 2)");
        snprintf(e, sizeof e, "box %s, you are a %s of Civil War. Choose cities to annex.", nm(1),
                 i ? "beneficary" : "beneficiary");
        expect(has(e) && has("list 12 5 0"), "A6 the human 1 (room 0) on the cities screen");
        expect(sb(NAT(12) + 0x51) == 0, "A6 what nobody could take is destroyed");
    }
    g_original = 0;

    /* A7 the Military losses on the real lists: 12 in t, 13 next to it in u and far away in w */
    {
        int u = -1, w = -1, j, nn;
        for (t = 0; t < 157 && u < 0; t++) {   /* u in t's land list and t in u's */
            uint32_t h = (uint32_t)s32(0xF8E7Cu + 4u * (uint32_t)t);
            nn = adjw(h);
            for (j = 1; j <= nn; j++) {
                o = adjw(h + 2u * (uint32_t)j);
                if (o >= 0 && o < 157 && o != t && in_list(0xF8E7Cu, o, t)) { u = o; break; }
            }
        }
        t--;
        for (o = 0; o < 157 && w < 0; o++)
            if (o != t && o != u && !adjacent(o, t) && !adjacent(o, u))
                w = o;
        base_clean();
        give_tokens(t, 12, 3);
        give_tokens(u, 13, 4);
        give_tokens(w, 13, 6);
        g_a[0] = 3; g_a[1] = 4;
        run_both(R_2AD50, 3, "A7");
        expect(rb(P12 + (uint32_t)t) == 0 && rb(P13 + (uint32_t)u) == 0 && rb(P13 + (uint32_t)w) == 0,
               "A7 the marks cleared by the removals, the far area never marked");
        expect(tokat(t, 12) == 0 && tokat(u, 13) == 0 && tokat(w, 13) == 5, "A7 the adjacent units removed, one far token lost");
        snprintf(e, sizeof e, "27FCC 12 2 %s, choose MILITARY losses for First Faction. 9 0 -3", nm(3));
        expect(has(e), "A7 the first faction's rest 2 in kind 9 by the victim, down to -3");
        snprintf(e, sizeof e, "27FCC 13 1 %s, choose MILITARY losses for Second Faction. 9 0 0", nm(4));
        expect(has(e), "A7 the second faction's rest 1 by the beneficiary");
        expect(has("box Removing First Faction units adjacent to Second Faction for Military losses.") &&
               has("box Removing Second Faction units adjacent to First Faction for Military losses.") &&
               has("wait 1500"), "A7 the automatic screens");
        expect(sb(NAT(12) + 0x98) == 0 && sb(NAT(12) + 0x99) == 3 && sb(NAT(13) + 0x98) == 5, "A7 the faction counters");
        printf("A7: t %d (%s), u %d, w %d\n", t, tn(t), u, w);
    }

    /* A8 0x2D3E0's counts and return */
    base_clean();
    put_city(5, 12); put_city(6, 12); give_tokens(7, 12, 4); give_tokens(8, 13, 9); put_city(9, 13);
    g_a[0] = 3;
    g_side = 13;
    run_both(R_2D3E0, 1, "A8");
    snprintf(e, sizeof e, "lossbox 0 %s, Choose side to play. 4 2 9 1 -> 13", nm(3));
    expect(has(e) && g_ret_real == 13, "A8 the counts of 12 / 13 in the losses box, 13 returned");
    expect(has("append C0214") && has("remove C0214") && has("home 3"), "A8 dialog 0xC0214, the home view");

    /* A9 0x2EE6C's three views */
    base_clean();
    g_a[0] = 12; g_a[1] = -1; g_a[2] = 5;
    run_both(R_2EE6C, 1, "A9 faction, none");
    expect(has("scroll 0 0") && !has("centre") && has("text 310 374 20 5") && has("wait 1500"), "A9 faction: the map as it is");
    g_a[0] = 4;
    run_both(R_2EE6C, 1, "A9 nation, none");
    expect(has("home 4"), "A9 nation: the home view");
    g_a[1] = 17;
    run_both(R_2EE6C, 1, "A9 territory");
    snprintf(e, sizeof e, "text 265 394 110 %s", tn(17));
    expect(has(e) && has("border 17 15") && has("centre 17"), "A9 territory: name, highlight, centre");

    /* A10 Military removes the whole second faction: "no longer exists", the first faction back, no annexation */
    {
        static const int c[] = { -0x66, -0x66, -0x66, -1, -0x66, -0x66, -1 };
        base_clean();
        for (o = 0; o < 4; o++) put_city(60 + o, 4);
        give_tokens(70, 4, 20);
        w8(NAT(4) + 0x99, 0);
        w8(NAT(4) + 0x35, 1);
        set_victim(0, 4);
        force(c, 7);
        g_a[0] = 0;
        run_both(R_29E34, 5, "A10");
        expect(has("box Removing Second Faction units adjacent to First Faction for Military losses.") &&
               has("The Second Faction no longer exists due to Military losses.") && !has("lossbox"),
               "A10 the second faction removed, no choice of side");
        expect(city_of(60, 4) && city_of(63, 4) && tokat(70, 4) == 10 && rb(NAT(4) + 0x9B) == 0 && !has("annexes"),
               "A10 the first faction back (less its 5 losses), no annexation");
    }
    printf("A. hand vectors: %d failed\n", g_fail - f0);
}

/* ---- B..F random and real -------------------------------------------------------------------------------------- */

static void part_b(void)
{
    int i, f0 = g_fail, v, k;
    char what[64];
    for (i = 0; i < 6000; i++) {
        base_clean();
        random_world();
        g_original = i & 1;
        do v = 1 + (int)rnd(9); while (rb(NAT(v) + 0x9A) == 0 && rnd(5) != 0);
        k = (int)rnd(12);
        set_victim(k, v);
        g_a[0] = k;
        snprintf(what, sizeof what, "B %d", i);
        run_both(R_29E34, 1000u + (uint32_t)i, what);
    }
    g_original = 0;
    printf("B. 0x29E34 on 6000 random states: %d failed\n", g_fail - f0);
}

static void part_c(void)
{
    int i, f0 = g_fail, n;
    char what[64];
    for (i = 0; i < 6000; i++) {
        base_clean();
        random_world();
        g_original = i & 1;
        g_a[4] = 4 + (int)rnd(3);
        g_a[0] = 1 + (int)rnd(9);
        g_a[2] = g_a[4] == 4 ? 1 + (int)rnd(9) : 12 + (int)rnd(2);
        g_a[3] = g_a[4] == 4 ? 12 : 1 + (int)rnd(9);
        if (g_a[4] != 4) {   /* a faction with units to annex */
            for (n = 0; n < 6; n++) {
                int t = (int)rnd(157);
                if (rnd(2) && !(rb(TS(t) + 2) & 1)) put_city(t, g_a[2]);
                else if (!(rb(TS(t) + 2) & 1)) give_tokens(t, g_a[2], 1 + (int)rnd(5));
            }
        }
        g_a[1] = (int)rnd(25);
        snprintf(g_text, sizeof g_text, "Choose %d", g_a[1]);
        snprintf(what, sizeof what, "C %d mode %d", i, g_a[4]);
        run_both(R_2B6EC, 2000u + (uint32_t)i, what);
    }
    g_original = 0;
    printf("C. 0x2B6EC on 6000 random states: %d failed\n", g_fail - f0);
}

static void part_d(void)
{
    int i, f0 = g_fail, n, t, f;
    char what[64];
    for (i = 0; i < 4000; i++) {
        base_clean();
        random_world();
        g_original = i & 1;
        for (f = 12; f <= 13; f++)
            for (n = 0; n < 1 + (int)rnd(8); n++) {
                t = (int)rnd(157);
                if (rb(TS(t) + 2) & 1) continue;
                if (rnd(4) == 0) put_city(t, f);
                else give_tokens(t, f, 1 + (int)rnd(4));
            }
        g_a[0] = 1 + (int)rnd(9);
        g_a[1] = 1 + (int)rnd(9);
        snprintf(what, sizeof what, "D %d", i);
        run_both(R_2AD50, 3000u + (uint32_t)i, what);
    }
    g_original = 0;
    printf("D. 0x2AD50 on 4000 random faction boards: %d failed\n", g_fail - f0);
}

static void part_e(void)
{
    int i, f0 = g_fail;
    char what[64];
    for (i = 0; i < 2000; i++) {
        base_clean();
        random_world();
        if (i & 1) {
            g_a[0] = 1 + (int)rnd(13);
            g_a[1] = (int)rnd(160) - 20;
            g_a[2] = (int)rnd(30) - 5;
            snprintf(what, sizeof what, "E 2EE6C %d", i);
            run_both(R_2EE6C, 4000u + (uint32_t)i, what);
        } else {
            g_a[0] = 1 + (int)rnd(9);
            snprintf(what, sizeof what, "E 2D3E0 %d", i);
            run_both(R_2D3E0, 4000u + (uint32_t)i, what);
        }
    }
    printf("E. 0x2EE6C / 0x2D3E0 on 2000: %d failed\n", g_fail - f0);
}

static void part_f(const char *savedir, const char *gamedir)
{
    static const char *const SAVES[4] = { "SAVGAMEA.DAT", "savgameb.dat", "savgame0.dat", "savgame1.dat" };
    uint8_t *base = malloc(DS_SIZE);
    char path[1024], what[64];
    int s, i, n, v, f0 = g_fail, runs = 0;

    for (s = 0; s < 4 && base != NULL; s++) {
        snprintf(path, sizeof path, "%s/%s", s < 2 ? savedir : gamedir, SAVES[s]);
        if (!load_save(path)) {
            printf("FAIL F: cannot read %s\n", path);
            g_fail++;
            continue;
        }
        memcpy(base, R, DS_SIZE);
        g_ds = R;
        printf("F: %s round %d, players", SAVES[s], (int16_t)(rb(0xF40FCu) | rb(0xF40FDu) << 8));
        for (n = 1; n <= 9; n++)
            printf(" %d", rb(NAT(n) + 0x9A));
        printf(", cities");
        for (n = 1; n <= 9; n++)
            printf(" %d", sb(NAT(n) + 0x51));
        printf(", board");
        for (n = 1; n <= 9; n++)
            printf(" %d", sb(NAT(n) + 0x98));
        printf(", stock");
        for (n = 1; n <= 9; n++)
            printf(" %d", sb(NAT(n) + 0x99));
        printf("\n");
        for (v = 1; v <= 9; v++) {
            for (i = 0; i < 40; i++) {
                memcpy(R, base, DS_SIZE);
                g_ds = R;
                if (rb(NAT(v) + 0x9A) == 0)
                    break;
                if (i >= 2) {
                    for (n = 1; n <= 9; n++)
                        if (rb(NAT(n) + 0x9A) != 0 && rnd(3) == 0)
                            w8(NAT(n) + 0x9A, rnd(2) ? 3 : 2);
                    if (rnd(2)) w8(NAT(v) + 0x35, 1);
                    if (rnd(4) == 0) w8(NAT(v) + 0x36, 1);
                    if (rnd(3) == 0) w8(NAT(v) + 0x99, 0);
                    if (i % 4 != 3) {   /* much of the stock onto the victim's own areas, cities on some: a real war */
                        int t, a = 0;
                        for (t = 0; t < 157; t++)
                            if (rb(AR(t + 1) + 0x0Eu + (uint32_t)v) != 0 && sb(NAT(v) + 0x99) > 0) {
                                int x = (int)rnd((uint32_t)sb(NAT(v) + 0x99) + 1u);
                                if (rnd(3) == 0 && !(rb(TS(t) + 2) & 1) && sb(NAT(v) + 0x51) < 9 && a++ < 4) {
                                    while (rb(AR(t + 1) + 0x0Eu + (uint32_t)v) != 0) {   /* tokens become a city */
                                        eff_57CE8(t, v, 1);
                                        w8(NAT(v) + 0x98, rb(NAT(v) + 0x98) - 1);
                                        w8(NAT(v) + 0x99, rb(NAT(v) + 0x99) + 1);
                                    }
                                    eff_78E74(t, v);
                                    continue;
                                }
                                give_tokens(t, v, x);
                                w8(NAT(v) + 0x99, sb(NAT(v) + 0x99) - x);
                            }
                    }
                    w8(0xF0980u, (rb(0xF0980u) & ~4) | (rnd(2) ? 4 : 0));
                    w8(0xF097Cu, (rb(0xF097Cu) & ~8) | (rnd(2) ? 8 : 0));
                }
                g_original = i & 1;
                set_victim(0, v);
                g_a[0] = 0;
                snprintf(what, sizeof what, "F %s v%d %d", SAVES[s], v, i);
                run_both(R_29E34, 5000u + (uint32_t)(100 * v + i), what);
                runs++;
            }
        }
    }
    g_original = 0;
    free(base);
    printf("F: the real saves, %d runs, %d failed\n", runs, g_fail - f0);
}

/* ---- G. broken models ------------------------------------------------------------------------------------------ */

static void part_g(void)
{
    int mut, i, caught, which;
    char what[80];
    for (mut = 1; mut <= 14; mut++) {
        caught = 0;
        for (i = 0; i < 4000 && !caught; i++) {
            base_clean();
            random_world();
            g_original = i & 1;
            which = mut == 4 || mut == 5 || mut == 9 || mut == 13 ? R_2B6EC
                  : mut == 6 || mut == 7 ? R_2AD50 : mut == 10 ? R_2EE6C : mut == 11 ? R_2D3E0 : R_29E34;
            if (mut == 6 || mut == 7) {
                int n, t, f;
                for (f = 12; f <= 13; f++)
                    for (n = 0; n < 1 + (int)rnd(8); n++) {
                        t = (int)rnd(157);
                        if (!(rb(TS(t) + 2) & 1)) give_tokens(t, f, 1 + (int)rnd(3));
                    }
                g_a[0] = 1 + (int)rnd(9); g_a[1] = 1 + (int)rnd(9);
            } else if (which == R_2B6EC) {
                g_a[4] = 4 + (int)rnd(3);
                g_a[0] = 1 + (int)rnd(9);
                g_a[2] = 1 + (int)rnd(9);
                g_a[3] = g_a[4] == 4 ? 12 : 1 + (int)rnd(9);
                g_a[1] = (int)rnd(25);
                snprintf(g_text, sizeof g_text, "Choose %d", g_a[1]);
            } else if (which == R_2EE6C) {
                g_a[0] = 1 + (int)rnd(13); g_a[1] = (int)rnd(157); g_a[2] = (int)rnd(20);
            } else if (which == R_2D3E0) {
                give_tokens(3, 12, 1 + (int)rnd(5)); give_tokens(4, 13, 1 + (int)rnd(5));
                g_a[0] = 1 + (int)rnd(9);
            } else {
                int v;
                do v = 1 + (int)rnd(9); while (rb(NAT(v) + 0x9A) == 0);
                set_victim(0, v);
                g_a[0] = 0;
                if (mut == 12) g_original = 0;
            }
            g_mut = mut;
            memcpy(M, R, DS_SIZE);
            if (!compare(which, 9000u + (uint32_t)i, "G", 1))
                caught = 1;
            g_mut = 0;
        }
        snprintf(what, sizeof what, "G broken model %d caught (after %d runs)", mut, i);
        expect(caught, what);
        printf("G: broken model %d %s after %d runs\n", mut, caught ? "caught" : "NEVER caught", i);
    }
    g_original = 0;
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
    g_log = g_real;
    g_len = &g_real_len;
    if (!load_adj(gamedir, "LANDADJ.DAT", 0, 0xF8E7Cu) || !load_adj(gamedir, "COASTADJ.DAT", 1, 0xF90FCu)) {
        fprintf(stderr, "cannot read the adjacency files in %s\n", gamedir);
        return 2;
    }
    memcpy(g_pristine, R, DS_SIZE);

    part_a();
    part_b();
    part_c();
    part_d();
    part_e();
    part_f(savedir, gamedir);
    part_g();

    printf("coverage (port side): ONE %ld, ALL %ld, RESTART %ld, map picks %ld, 0x2C190 %ld, 0x2C894 %ld, 0x27FCC %ld, "
           "0x2D10C %ld, 0x2D1C0 %ld\n", g_cov[0], g_cov[1], g_cov[2], g_cov[3], g_cov[4], g_cov[5], g_cov[6],
           g_cov[7], g_cov[8]);
    printf("  (model side) city ONE %ld, city ALL %ld, automatic removals %ld, no Civil War %ld, quelled %ld, "
           "Military %ld, no second faction %ld, whole annexation %ld, partial %ld, leftovers destroyed %ld\n",
           g_cov[9], g_cov[10], g_cov[11], g_cov[12], g_cov[13], g_cov[14], g_cov[15], g_cov[16], g_cov[17], g_cov[18]);
    printf("not_ported %d, cut %d, unexpected %d\n", g_np, g_cuts, g_unexpected);
    printf("%d checks, %d failed\n", g_checks, g_fail);
    return g_fail == 0 && g_unexpected == 0 ? 0 : 1;
}
