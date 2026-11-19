/* mono_test [GAMEDIR [SAVEDIR]]: unit checks for module MONO (mono.c) that need no game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init), LANDADJ.DAT (the land lists) and the saves savgame0.dat
   (start of MOVEMENT R4) and savgame1.dat (R4, all human); default C:\work\advciv_run, only read. SAVEDIR holds
   SAVGAMEA.DAT and savgameb.dat (default C:\work\advciv_play, only read). The saves are read with saveload.c's f5F05C
   as the game does.
   mono.c is built with -DMONO_TEST: its call into the computers' code (0x21E8C) comes to a recorder here that acts
   out a plausible effect (the first candidate converted as the human's CONVERT does, no screen) and returns.

   The land lists: LANDADJ.DAT is parsed here; the port side gets them as the game has them (one heap block, a handle
   per territory at 0xF8E7C + 4t, the neighbours not in play dropped as 0x3EED8 does, by a mask the test chooses); the
   model keeps its own arrays, filtered by the same mask in its own way.

   Every routine mono.c calls is a stub below that appends one line to a trace (drawing included); those with data
   effects apply them on the buffer B: f78F04 / f78E74 (rules.h: the city removed / built, the city list), f57CE8 /
   f57E28 (move.h: tokens, marker, count, the territory list, f581C4), f3F670 / f3F624 (the border colours). Clicks
   (f78BA4) come from a script: a seeded random choice among the codes the screen may get (map clicks on listed
   entries, the arrows -9 / -10, CONVERT -16, FINISHED -1, -100 and codes that do nothing there), the same seed for
   the port and the model; a run that clicks more than 3000 times is cut off ("CUT") identically on both sides.

   The model is a second implementation of the three routines, written from the disassembly (not from mono.c): it
   runs on a copy of the data object, calls the same leaf stubs, and so writes the trace it expects. After each run
   the two traces must be equal and the whole data object must equal the model's copy. Parts:
     A. hand vectors (expected values written out): the candidate rules (stock, Theology / Monotheism of the owners,
        the Pirates' city, 9 cities, n's own territories, Barbarian tokens), the human's conversion of a city and of
        tokens, FINISHED, the empty message, the computer / PBEM hand-over, the text helper with and without
        --original;
     B. 0x227B0 on 30,000 random states; C. 0x21914 on 12,000 random states with click scripts (humans, computers,
        PBEM, --original);
     D. 0x21914 on the four real saves, 300 runs each with Monotheism / Theology given out at random.
   Prints each failure (the first differing trace line or byte), the counts per part, and exits 0 when all pass.

   Build: clang -std=c11 -DMONO_TEST port/test/mono_test.c port/src/mono.c port/src/saveload.c port/src/state.c
          port/src/le_image.c -Iport/src -o mono_test.exe */

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fg.h"
#include "game.h"
#include "kit.h"
#include "loop.h"
#include "map.h"
#include "mapdata.h"
#include "mono.h"
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
#define ADJPTR      0xF8E7Cu

/* ---- the buffer the code under test and the stubs act on: g_ds (the port's run) or the model's copy ------------ */

static uint8_t *B;
static uint8_t *M;
static uint8_t *g_pristine;

static uint8_t  b8(uint32_t a) { return B[a - DS_BASE]; }
static int      s8(uint32_t a) { return (int8_t)B[a - DS_BASE]; }
static int      s16(uint32_t a) { return (int16_t)(B[a - DS_BASE] | B[a + 1 - DS_BASE] << 8); }
static int32_t  s32(uint32_t a)
{
    return (int32_t)((uint32_t)B[a - DS_BASE] | (uint32_t)B[a + 1 - DS_BASE] << 8 | (uint32_t)B[a + 2 - DS_BASE] << 16 |
                     (uint32_t)B[a + 3 - DS_BASE] << 24);
}
static void w8(uint32_t a, int v) { B[a - DS_BASE] = (uint8_t)v; }
static void w16(uint32_t a, int v) { B[a - DS_BASE] = (uint8_t)v; B[a + 1 - DS_BASE] = (uint8_t)(v >> 8); }
static void w32(uint32_t a, int32_t v)
{
    uint32_t u = (uint32_t)v;
    B[a - DS_BASE] = (uint8_t)u; B[a + 1 - DS_BASE] = (uint8_t)(u >> 8);
    B[a + 2 - DS_BASE] = (uint8_t)(u >> 16); B[a + 3 - DS_BASE] = (uint8_t)(u >> 24);
}
static const char *str(uint32_t a) { return (const char *)(B + (a - DS_BASE)); }
static const char *name_of(int n) { return str((uint32_t)s32(0xC768Cu + 4u * (uint32_t)n)); }
static const char *tname(int t) { return str((uint32_t)s32(0xC77D8u + 4u * (uint32_t)t)); }

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

/* ---- the land lists ------------------------------------------------------------------------------------------- */

static int g_raw_n[160], g_raw[160][32];   /* LANDADJ.DAT as read */
static uint8_t g_mask[160];                /* in play (the test's choice; 0x3EED8 drops the others) */
static int g_madj_n[160], g_madj[160][32]; /* the model's filtered copy */
static uint32_t g_adj_handle;               /* the heap block of the port's lists */

static int load_landadj(const char *gamedir)
{
    char path[1024];
    FILE *fp;
    uint8_t b[2], sz[4];
    int t, i, n;

    snprintf(path, sizeof path, "%s/LANDADJ.DAT", gamedir);
    fp = fopen(path, "rb");
    if (fp == NULL)
        return 0;
    if (fread(sz, 4, 1, fp) != 1) {
        fclose(fp);
        return 0;
    }
    for (t = 0; t < 160; t++) {
        if (fread(b, 2, 1, fp) != 1)
            break;
        n = (int16_t)(b[0] | b[1] << 8);
        if (n < 0 || n > 32)
            break;
        g_raw_n[t] = n;
        for (i = 0; i < n; i++) {
            if (fread(b, 2, 1, fp) != 1)
                break;
            g_raw[t][i] = (int16_t)(b[0] | b[1] << 8);
        }
    }
    fclose(fp);
    return t == 160;
}

/* ---- heap (port handles) -------------------------------------------------------------------------------------- */

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
    if ((h & TAG) == 0 || slot < 1 || slot > 32 || h == g_adj_handle)
        return;
    free(s_blk[slot - 1]);
    s_blk[slot - 1] = NULL;
}

/* the port's lists in g_ds (as 0x3EED8 leaves them: count, neighbours, then the left-out ones dropped) and the
   model's own filtered copy, both by g_mask */
static void set_adj(void)
{
    uint8_t *blk;
    uint32_t off = 0;
    int t, i, k;

    if (g_adj_handle == 0)
        g_adj_handle = load_heap_alloc(2u * (160u + 160u * 32u));
    blk = load_heap_ptr(g_adj_handle, 2u * (160u + 160u * 32u));
    for (t = 0; t < 160; t++) {
        uint32_t base = off;
        int cnt = 0;
        for (i = 0; i < g_raw_n[t]; i++)
            if (g_mask[g_raw[t][i]] != 0) {
                blk[base + 2u + 2u * (uint32_t)cnt] = (uint8_t)g_raw[t][i];
                blk[base + 3u + 2u * (uint32_t)cnt] = (uint8_t)(g_raw[t][i] >> 8);
                cnt++;
            }
        blk[base] = (uint8_t)cnt;
        blk[base + 1] = 0;
        setD32(ADJPTR + 4u * (uint32_t)t, g_adj_handle + base);
        off += 2u + 2u * (uint32_t)g_raw_n[t];
    }
    for (t = 0; t < 160; t++) {
        k = 0;
        for (i = 0; i < g_raw_n[t]; i++)
            if (g_mask[g_raw[t][i]])
                g_madj[t][k++] = g_raw[t][i];
        g_madj_n[t] = k;
    }
}

/* ---- the click script, the cut -------------------------------------------------------------------------------- */

static uint32_t g_srng;
static int g_steps;
static jmp_buf g_jb;
static int g_np, g_cuts;
static long g_cov[8];   /* port side: city conversions, token conversions, FINISHED, 0x21E8C, empty messages, screens */

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
        switch (srnd(14)) {
        case 0: return -100;
        case 1: case 2: if (cnt > 0) return (int)srnd((uint32_t)cnt); break;
        case 3: case 4: return -9;
        case 5: case 6: return -10;
        case 7: return -16;
        case 8: if (srnd(3) == 0) return -1; break;
        case 9: return -11;
        case 10: return -13;
        case 11: return -5;
        case 12: return -0x58;
        default: return -2;
        }
    }
}

static int g_forced[32], g_nforced, g_iforced;   /* hand vectors: these clicks first, then -1 */

/* ---- shared effects (on B) ------------------------------------------------------------------------------------ */

static void eff_581C4(int t, int n)   /* move.h's f581C4 */
{
    uint32_t a = AR(t + 1), rec = NAT(n);
    int c, j, k;
    if (s8(a + 1u + (uint32_t)n) == 0 || s8(a + 0x0Eu + (uint32_t)n) != 0)
        return;
    w8(a + 1, b8(a + 1) - 1);
    w8(a + 1u + (uint32_t)n, 0);
    c = s8(rec + 0x60);
    for (j = 0; j < c; j++)
        if (b8(rec + 0x61u + (uint32_t)j) == t + 1)
            break;
    if (j < c) {
        for (k = j; k < c - 1; k++)
            w8(rec + 0x61u + (uint32_t)k, b8(rec + 0x62u + (uint32_t)k));
        w8(rec + 0x61u + (uint32_t)(c - 1), 0);
        w8(rec + 0x60, c - 1);
    }
}

static void eff_57E28(int t, int n, int k)   /* move.h's f57E28 */
{
    uint32_t a = AR(t + 1), rec = NAT(n);
    int c, j;
    if (k <= 0)
        return;
    if (s8(a + 1u + (uint32_t)n) != n) {
        w8(a + 1, b8(a + 1) + 1);
        w8(a + 1u + (uint32_t)n, n);
    }
    w8(a + 0x0Eu + (uint32_t)n, b8(a + 0x0Eu + (uint32_t)n) + k);
    c = s8(rec + 0x60);
    for (j = 0; j < c; j++)
        if (b8(rec + 0x61u + (uint32_t)j) == t + 1)
            return;
    if (c >= 0 && c < 0x60) {
        w8(rec + 0x61u + (uint32_t)c, t + 1);
        w8(rec + 0x60, c + 1);
    }
}

static void eff_57CE8(int t, int n, int flag, int k)   /* move.h's f57CE8 */
{
    uint32_t a = AR(t + 1);
    if (k > 0) {
        w8(a + 0x0Eu + (uint32_t)n, b8(a + 0x0Eu + (uint32_t)n) - k);
        if (flag == 1)
            w8(a + 0x24u + (uint32_t)n, b8(a + 0x24u + (uint32_t)n) - k);
    }
    eff_581C4(t, n);
}

static void eff_city_add(int t, int n)   /* rules.h's f78E74 */
{
    uint32_t rec = NAT(n);
    w8(TS(t) + 2, b8(TS(t) + 2) | 1);
    w8(TS(t), n);
    w8(TS(t) + 1, b8(0xF40FCu));
    w8(rec + 0x52u + (uint32_t)s8(rec + 0x51), t + 1);
    w8(rec + 0x51, s8(rec + 0x51) + 1);
}

static void eff_city_del(int t, int n)   /* rules.h's f78F04 */
{
    uint32_t rec = NAT(n);
    int c, j, k;
    w8(TS(t) + 2, b8(TS(t) + 2) & ~1);
    w8(TS(t), 0);
    w8(TS(t) + 1, 0xFF);
    if (n < 0 || n > 13)
        return;
    c = s8(rec + 0x51);
    for (j = 0; j < c && j < 9; j++)
        if (b8(rec + 0x52u + (uint32_t)j) == t + 1)
            break;
    if (j < c && j < 9) {
        for (k = j; k < c - 1; k++)
            w8(rec + 0x52u + (uint32_t)k, b8(rec + 0x53u + (uint32_t)k));
        w8(rec + 0x52u + (uint32_t)(c - 1), 0);
        w8(rec + 0x51, c - 1);
    }
}

/* ---- stubs (both sides; B is g_ds for the port, M for the model) ---------------------------------------------- */

_Noreturn void not_ported(uint32_t addr)
{
    tr("NOT PORTED %X", (unsigned)addr);
    g_np++;
    longjmp(g_jb, 1);
}

int f78AE4(void) { return 0; }
void f76F70(int n, uint8_t *cards) { (void)n; (void)cards; }

int fg_vbcopy(int minx, int maxx, int miny, int maxy, int newx, int newy, int source, int dest)
{
    tr("vbcopy %d %d %d %d %d %d %d %d", minx, maxx, miny, maxy, newx, newy, source, dest);
    return 0;
}
int fg_vbopen(int handle) { tr("vbopen %d", handle); return 0; }
void fg_vbclose(void) { tr("vbclose"); }
int fg_setcolor(int color) { tr("color %d", color); return 0; }
void f783B8(uint32_t dlg, int base) { tr("783B8 %X %d", (unsigned)dlg, base); }
void f553C0(void) { tr("music"); }
void f3CD6C(int8_t pic) { tr("bg %d", pic); }
void f7876C(void) { tr("phase"); }
void f3B05C(uint32_t dlg) { tr("dialog %X", (unsigned)dlg); }
void f459E4(int n) { tr("panel %d", n); }
void f3CBBC(void) { tr("paste"); }
void f3A9F4(const char *text) { tr("left %s", text); }
void f78A20(int t) { tr("wait %d", t); }
void f79270(int wait, int n, int t, const char *text, int anim)
{
    tr("msg %d %d %d [%s] %d", wait, n, t, text, anim);
    if (B == g_ds)
        g_cov[4]++;
}
void f3F670(void) { tr("clrb"); memset(B + (BORDERS - DS_BASE), 0, 0xA0); }
void f3F624(int t, int c) { tr("border %d %d", t, c); w8(BORDERS + (uint32_t)t, c); }
void f7853C(uint32_t dlg) { tr("append %X", (unsigned)dlg); if (B == g_ds) g_cov[5]++; }
void f78664(uint32_t dlg) { tr("remove %X", (unsigned)dlg); }
void f3AF8C(uint32_t spr, int x, int y) { tr("sprite %X %d %d", (unsigned)spr, x, y); }
void f3A0B0(int x, int y, int w, const char *s) { tr("text %d %d %d %s", x, y, w, s); }
void f3C9D0(int x, int y, int w, int h, int flags) { tr("frame %d %d %d %d %d", x, y, w, h, flags); }
void f3DA60(int t) { tr("centre %d", t); }
void f3D52C(int dx, int dy) { tr("scroll %d %d", dx, dy); }
void f78F04(int t, int n) { tr("78F04 %d %d", t, n); eff_city_del(t, n); if (B == g_ds) g_cov[0]++; }
void f78E74(int t, int n) { tr("78E74 %d %d", t, n); eff_city_add(t, n); }
void f57CE8(int t, int n, int flag, int k) { tr("57CE8 %d %d %d %d", t, n, flag, k); eff_57CE8(t, n, flag, k); }
void f57E28(int t, int n, int k) { tr("57E28 %d %d %d", t, n, k); eff_57E28(t, n, k); if (B == g_ds) g_cov[1]++; }

int f78BA4(uint32_t dlg, int map)
{
    int r;
    cut_check();
    if (g_nforced > 0)
        r = g_iforced < g_nforced ? g_forced[g_iforced++] : -1;
    else
        r = pick();
    tr("click %X %d %d", (unsigned)dlg, map, r);
    if (B == g_ds && r == -1)
        g_cov[2]++;
    return r;
}

/* the computers' conversion (M7): a recorder that converts entry 0 as the human's CONVERT does (no screen) */
void mono_test_f21E8C(int n)
{
    int t = s32(LIST), k, x, o;
    tr("21E8C %d %d %d", n, s32(LIST_N), t);
    if (B == g_ds)
        g_cov[3]++;
    if (t < 0 || t > 156)
        return;
    if (b8(TS(t) + 2) & 1) {
        o = b8(TS(t));
        w8(AR(t + 1) + 0x3Bu + (uint32_t)o, 1);
        eff_city_del(t, o);
        eff_city_add(t, n);
        return;
    }
    for (k = 1; k <= 9; k++) {
        x = s8(AR(t + 1) + 0x0Eu + (uint32_t)k);
        if (b8(AR(t + 1) + 0x0Eu + (uint32_t)k) == 0)
            continue;
        w8(AR(t + 1) + 0x3Bu + (uint32_t)k, 1);
        eff_57CE8(t, k, 0, x);
        w8(NAT(k) + 0x98, b8(NAT(k) + 0x98) - x);
        w8(NAT(k) + 0x99, b8(NAT(k) + 0x99) + x);
        eff_57E28(t, n, x);
        w8(NAT(n) + 0x99, b8(NAT(n) + 0x99) - x);
        w8(NAT(n) + 0x98, b8(NAT(n) + 0x98) + x);
    }
}

/* ---- the model (B == M), written from the disassembly ---------------------------------------------------------- */

/* 0x227B0 */
static void m227B0(int n)
{
    uint8_t mine[157], near[157];
    int t, j, k, total, okay, stock, cities;

    w32(LIST_N, 0);
    for (t = 0; t < 157; t++)
        w32(LIST + 4u * (uint32_t)t, -1);
    for (t = 0; t < 157; t++)
        mine[t] = ((b8(TS(t) + 2) & 1) && b8(TS(t)) == n) || b8(AR(t + 1) + 0x0Eu + (uint32_t)n) != 0;
    memset(near, 0, sizeof near);
    for (t = 0; t < 157; t++)
        if (mine[t])
            for (j = 0; j < g_madj_n[t]; j++)
                near[g_madj[t][j]] = 1;
    cities = s8(NAT(n) + 0x51);
    stock = s8(NAT(n) + 0x99);
    for (t = 0; t < 157; t++) {
        int add = 0;
        if (!near[t] || mine[t])
            continue;
        if (cities != 9 && (b8(TS(t) + 2) & 1)) {
            int o = b8(TS(t));
            if (o != 10 && b8(NAT(o) + 0x3A) == 0 && b8(NAT(o) + 0x3B) == 0)
                add = 1;
        }
        if (!add && !g_original && (b8(TS(t) + 2) & 1))
            continue;   /* the fix: a city territory is a candidate by the city rule only */
        if (!add) {
            total = 0;
            okay = 1;
            for (k = 1; k <= 9; k++) {
                if (k == n || b8(AR(t + 1) + 0x0Eu + (uint32_t)k) == 0)
                    continue;
                total += s8(AR(t + 1) + 0x0Eu + (uint32_t)k);
                if (b8(NAT(k) + 0x3A) || b8(NAT(k) + 0x3B))
                    okay = 0;
            }
            add = okay && total != 0 && total <= stock;
        }
        if (add) {
            int c = s32(LIST_N);
            w32(LIST + 4u * (uint32_t)c, t);
            w32(LIST_N, c + 1);
        }
    }
}

/* 0x21A70 */
static void m21A70(int n)
{
    char text[256];
    int sel = 0, fresh = 1, r, t, k, x, o;

    snprintf(text, sizeof text, "%s, possession of Monotheism allows conversion of a neighboring territory.", name_of(n));
    f7853C(0xBE9F4u);
    f3CD6C(3);
    w8(0xEE168u, 3);
    fg_vbcopy(0, 639, 0, 479, 0, 479, s32(0xF52CCu), s32(0xF52C8u));
    fg_vbopen(s32(0xF52C8u));
    f3AF8C(0xF51ACu, 251, 385);
    f3AF8C(0xF51B4u, 376, 385);
    f3A9F4(text);
    fg_setcolor(5);
    fg_vbclose();
    f7876C();
    f3B05C(0xF596Cu);
    w32(0xF40F8u, n);
    for (;;) {
        t = s32(LIST + 4u * (uint32_t)sel);
        fg_vbopen(s32(0xF52C8u));
        f3C9D0(265, 385, 110, 19, 5);
        fg_setcolor(5);
        f3A0B0(265, 389, 110, tname(t));
        fg_vbclose();
        f3F670();
        f3F624(t, 15);
        if (fresh) {
            f459E4(n);
            f3DA60(t);
        }
        f3CBBC();
        fresh = 0;
        r = f78BA4(0xF596Cu, 1);
        if (r == -100)
            continue;
        if (r >= 0) {
            sel = r;
            fresh = 1;
            continue;
        }
        if (r == -1)
            break;
        if (r == -9) {
            if (sel > 0) {
                sel--;
                fresh = 1;
            }
            continue;
        }
        if (r == -10) {
            if (sel < s32(LIST_N) - 1) {
                sel++;
                fresh = 1;
            }
            continue;
        }
        if (r != -16)
            continue;
        t = s32(LIST + 4u * (uint32_t)sel);
        if (b8(TS(t) + 2) & 1) {
            o = b8(TS(t));
            w8(AR(t + 1) + 0x3Bu + (uint32_t)o, 1);
            f78F04(t, o);
            f78E74(t, n);
        } else {
            for (k = 1; k <= 9; k++) {
                if (b8(AR(t + 1) + 0x0Eu + (uint32_t)k) == 0)
                    continue;
                w8(AR(t + 1) + 0x3Bu + (uint32_t)k, 1);
                x = s8(AR(t + 1) + 0x0Eu + (uint32_t)k);
                f57CE8(t, k, 0, x);
                w8(NAT(k) + 0x98, b8(NAT(k) + 0x98) - x);
                w8(NAT(k) + 0x99, b8(NAT(k) + 0x99) + x);
                f57E28(t, n, x);
                w8(NAT(n) + 0x99, b8(NAT(n) + 0x99) - x);
                w8(NAT(n) + 0x98, b8(NAT(n) + 0x98) + x);
            }
        }
        f459E4(n);
        f3D52C(0, 0);
        f3CBBC();
        f78A20(1000);
        break;
    }
    f78664(0xBE9F4u);
}

/* 0x21914 */
static void m21914(void)
{
    int n, c = 0, any = 0;

    w16(0xBB3FAu, 169);
    w16(0xBB3FCu, 340);
    w16(0xBB3FEu, 300);
    w16(0xBB400u, 35);
    for (n = 1; n <= 9; n++)
        if (b8(NAT(n) + 0x9A) != 0)
            w16(0xF06B0u + 2u * (uint32_t)c++, n);
    w16(0xF06AEu, c);
    f783B8(0xF596Cu, s32(0xF5940u));
    for (n = 1; n <= 9; n++) {
        if (b8(NAT(n) + 0x9A) == 0 || b8(NAT(n) + 0x3A) == 0)
            continue;
        m227B0(n);
        if (s32(LIST_N) == 0)
            continue;
        any = 1;
        f553C0();
        if (s8(NAT(n) + 0x9A) == 3 && (b8(0xE1624u) & 0x80) == 0)
            m21A70(n);
        else
            mono_test_f21E8C(n);
    }
    if (!any)
        f79270(100, s16(0xF06B0u), -200, "", 0);
}

/* ---- running both sides ---------------------------------------------------------------------------------------- */

enum { R_227B0, R_21914, R_21A70 };

static int g_checks, g_fail;
static uint32_t g_rng = 12345u;
static int g_arg;

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
    case R_227B0: if (model) m227B0(g_arg); else f227B0(g_arg); break;
    case R_21914: if (model) m21914(); else f21914(); break;
    case R_21A70: if (model) m21A70(g_arg); else f21A70(g_arg); break;
    }
}

static void side(int model, int which, uint32_t seed)
{
    g_log = model ? g_model : g_real;
    g_len = model ? &g_model_len : &g_real_len;
    *g_len = 0;
    g_log[0] = 0;
    B = model ? M : g_ds;
    g_srng = seed * 2654435761u + 1u;
    g_steps = 0;
    g_iforced = 0;
    if (setjmp(g_jb) == 0)
        call(which, model);
    B = g_ds;
}

/* runs the port and the model from the same state; returns 1 when both agree */
static int run_both(int which, uint32_t seed, const char *what)
{
    size_t i;
    const char *a, *b;
    int line = 1;

    memcpy(M, g_ds, DS_SIZE);
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
        printf("FAIL %s: trace line %d\n  port : %.120s\n  model: %.120s\n", what, line, a, b);
        g_fail++;
        return 0;
    }
    for (i = 0; i < DS_SIZE; i++)
        if (g_ds[i] != M[i]) {
            printf("FAIL %s: byte 0x%X port %02X model %02X\n", what, (unsigned)(i + DS_BASE), g_ds[i], M[i]);
            g_fail++;
            return 0;
        }
    return 1;
}

/* ---- states ---------------------------------------------------------------------------------------------------- */

static void put_tokens(int t, int n, int k)   /* n gets k tokens in t (marker, count, territory list) */
{
    uint32_t a = AR(t + 1), rec = NAT(n);
    int c, j;
    if (k <= 0)
        return;
    if (s8(a + 1u + (uint32_t)n) != n) {
        w8(a + 1, b8(a + 1) + 1);
        w8(a + 1u + (uint32_t)n, n);
    }
    w8(a + 0x0Eu + (uint32_t)n, s8(a + 0x0Eu + (uint32_t)n) + k);
    c = s8(rec + 0x60);
    for (j = 0; j < c; j++)
        if (b8(rec + 0x61u + (uint32_t)j) == t + 1)
            return;
    if (c < 0x60) {
        w8(rec + 0x61u + (uint32_t)c, t + 1);
        w8(rec + 0x60, c + 1);
    }
}

static void put_city(int t, int n, int round)
{
    uint32_t rec = NAT(n);
    int c = s8(rec + 0x51);
    if (c >= 9 || (b8(TS(t) + 2) & 1))
        return;
    w8(TS(t) + 2, b8(TS(t) + 2) | 1);
    w8(TS(t), n);
    w8(TS(t) + 1, round);
    w8(rec + 0x52u + (uint32_t)c, t + 1);
    w8(rec + 0x51, c + 1);
}

static void clear_board(void)
{
    int n, t;
    for (t = 0; t < 160; t++) {
        uint32_t a = AR(t + 1);
        memset(B + (a + 1 - DS_BASE), 0, 68);
        w8(TS(t), 0);
        w8(TS(t) + 1, 0xFF);
        w8(TS(t) + 2, 0);
    }
    for (n = 1; n <= 11; n++) {
        uint32_t rec = NAT(n);
        w8(rec + 0x51, 0);
        w8(rec + 0x60, 0);
        memset(B + (rec + 0x52 - DS_BASE), 0, 9);
        memset(B + (rec + 0x61 - DS_BASE), 0, 0x60);
    }
}

static void mask_all(void)
{
    memset(g_mask, 1, sizeof g_mask);
    set_adj();
}

static void base_clean(void)
{
    int n;
    memcpy(g_ds, g_pristine, DS_SIZE);
    B = g_ds;
    clear_board();
    for (n = 1; n <= 11; n++) {
        w8(NAT(n) + 0x9A, 0);
        w8(NAT(n) + 0x3A, 0);
        w8(NAT(n) + 0x3B, 0);
        w8(NAT(n) + 0x98, 0);
        w8(NAT(n) + 0x99, 0);
    }
    w16(0xF40FCu, 5);
    w8(0xE1624u, b8(0xE1624u) & 0x7F);
    g_nforced = 0;
    mask_all();
}

/* a random game state: players, Monotheism / Theology, stocks, cities (the Pirates' too), tokens (Barbarians too) */
static void random_state(void)
{
    int n, t, k, round = 1 + (int)rnd(12), kind = (int)rnd(4);
    B = g_ds;
    w16(0xF40FCu, round);
    clear_board();
    for (n = 1; n <= 11; n++) {
        uint32_t rec = NAT(n);
        k = (int)rnd(100);
        w8(rec + 0x9A, n > 9 ? 0 : k < 55 ? 3 : k < 85 ? 2 : 0);
        w8(rec + 0x3A, rnd(kind == 0 ? 2 : 4) == 0);
        w8(rec + 0x3B, rnd(6) == 0);
        w8(rec + 0x98, (int)rnd(60));
        w8(rec + 0x99, rnd(8) == 0 ? (int)rnd(4) : (int)rnd(50));
    }
    for (t = 0; t < 157; t++) {
        if (rnd(8) == 0) {
            put_city(t, rnd(12) == 0 ? 10 : 1 + (int)rnd(9), (int)rnd(16));
            if (rnd(10) == 0)
                put_tokens(t, 1 + (int)rnd(9), 1 + (int)rnd(3));
        } else if (rnd(5) < 2) {
            k = rnd(4) == 0 ? 2 : 1;
            while (k-- > 0)
                put_tokens(t, rnd(15) == 0 ? 11 : 1 + (int)rnd(9), 1 + (int)rnd(rnd(3) == 0 ? 20 : 6));
        }
        for (n = 1; n <= 9; n++)
            if (rnd(30) == 0)
                w8(AR(t + 1) + 0x3Bu + (uint32_t)n, 1);
    }
    if (rnd(6) == 0)   /* a nation at 9 cities (only the count is read) */
        w8(NAT(1 + (int)rnd(9)) + 0x51, 9);
    w8(0xE1624u, (b8(0xE1624u) & 0x7F) | (rnd(10) == 0 ? 0x80 : 0));
    w32(0xF5940u, (int)rnd(8));
    for (t = 0; t < 160; t++)
        g_mask[t] = rnd(4) == 0 ? rnd(8) != 0 : 1;
    set_adj();
}

static int load_save(const char *path)
{
    FILE *fp;
    int16_t version = 0;
    char desc[0x34];

    memcpy(g_ds, g_pristine, DS_SIZE);
    fp = fopen(path, "rb");
    if (fp == NULL)
        return 0;
    memset(desc, 0, sizeof desc);
    f5F05C(&version, desc, fp);
    fclose(fp);
    setD32(0xF40ECu, (uint32_t)(S8(0xE1620u) + S8(0xE1621u)));
    return 1;
}

/* ---- A. hand vectors ------------------------------------------------------------------------------------------- */

static int list_is(const int *want, int cnt)
{
    int i;
    if (S32(LIST_N) != cnt)
        return 0;
    for (i = 0; i < cnt; i++)
        if (S32(LIST + 4u * (uint32_t)i) != want[i])
            return 0;
    for (i = cnt; i < 157; i++)
        if (S32(LIST + 4u * (uint32_t)i) != -1)
            return 0;
    return 1;
}

static void part_a(void)
{
    int f0 = g_fail, a = -1, b, c, t, i, w[4];
    char what[96];

    /* a: the first territory with two land neighbours b < c (the expected lists below use only these three) */
    for (t = 0; t < 157 && a < 0; t++)
        if (g_raw_n[t] >= 2)
            a = t;
    b = g_raw[a][0];
    c = g_raw[a][1];
    if (b > c) { i = b; b = c; c = i; }

    /* A1: nation 1's tokens in a; nation 2 has 3 tokens in b; stock 5 -> [b] */
    g_arg = 1;
    base_clean();
    put_tokens(a, 1, 4);
    put_tokens(b, 2, 3);
    w8(NAT(1) + 0x99, 5);
    f227B0(1);
    w[0] = b;
    expect(list_is(w, 1), "A1 tokens within stock");
    run_both(R_227B0, 1, "A1 model");
    /* A2: stock 2 < 3 -> [] ; stock 3 -> [b] */
    w8(NAT(1) + 0x99, 2);
    f227B0(1);
    expect(list_is(w, 0), "A2 tokens over stock");
    w8(NAT(1) + 0x99, 3);
    f227B0(1);
    expect(list_is(w, 1), "A2 tokens equal to stock");
    /* A3: nation 2 holds Theology, then Monotheism -> []; a second nation's tokens add up */
    w8(NAT(2) + 0x3B, 1);
    f227B0(1);
    expect(list_is(w, 0), "A3 Theology protects");
    w8(NAT(2) + 0x3B, 0);
    w8(NAT(2) + 0x3A, 1);
    f227B0(1);
    expect(list_is(w, 0), "A3 Monotheism protects");
    w8(NAT(2) + 0x3A, 0);
    put_tokens(b, 3, 2);
    w8(NAT(1) + 0x99, 4);
    f227B0(1);
    expect(list_is(w, 0), "A3 two nations 5 > stock 4");
    w8(NAT(1) + 0x99, 5);
    f227B0(1);
    expect(list_is(w, 1), "A3 two nations 5 <= stock 5");
    /* A4: Barbarian tokens are not counted; alone they make no candidate */
    base_clean();
    put_tokens(a, 1, 4);
    put_tokens(b, 11, 6);
    w8(NAT(1) + 0x99, 1);
    f227B0(1);
    expect(list_is(w, 0), "A4 Barbarians alone");
    put_tokens(b, 2, 1);
    f227B0(1);
    expect(list_is(w, 1), "A4 Barbarians not counted");
    /* A5: a city of nation 3 in c -> [b, c] (ascending); the owner's Theology, the Pirates, 9 cities */
    w8(NAT(1) + 0x99, 5);
    put_city(c, 3, 2);
    f227B0(1);
    w[0] = b;
    w[1] = c;
    expect(list_is(w, 2), "A5 city listed");
    w8(NAT(3) + 0x3B, 1);
    f227B0(1);
    expect(list_is(w, 1), "A5 owner with Theology");
    w8(NAT(3) + 0x3B, 0);
    w8(NAT(1) + 0x51, 9);
    f227B0(1);
    expect(list_is(w, 1), "A5 converter at 9 cities");
    w8(NAT(1) + 0x51, 0);
    w8(TS(c), 10);
    f227B0(1);
    expect(list_is(w, 1), "A5 the Pirates' city");
    w8(TS(c), 3);
    /* A5b: the owner holds Theology and nation 2 has 2 tokens in the city territory c: fixed, c is not listed (a
       CONVERT would take the protected city); --original lists it through the token rule */
    w8(NAT(3) + 0x3B, 1);
    put_tokens(c, 2, 2);
    f227B0(1);
    expect(list_is(w, 1), "A5b protected city with tokens (fixed)");
    run_both(R_227B0, 12, "A5b model");
    g_original = 1;
    f227B0(1);
    expect(list_is(w, 2), "A5b protected city with tokens (original)");
    run_both(R_227B0, 13, "A5b original model");
    g_original = 0;
    w8(NAT(3) + 0x3B, 0);
    /* A6: n's own territories are never candidates: n's tokens in b too */
    put_tokens(b, 1, 1);
    f227B0(1);
    w[0] = c;
    expect(list_is(w, 1), "A6 own territory excluded");
    run_both(R_227B0, 2, "A6 model");
    /* A7: a left-out neighbour (not in play) is dropped from the lists */
    g_mask[c] = 0;
    set_adj();
    f227B0(1);
    expect(list_is(w, 0), "A7 left-out neighbour");
    run_both(R_227B0, 3, "A7 model");
    mask_all();

    /* A8: the phase, human 1 converts the city in c (select c by a map click on entry 1, CONVERT) */
    base_clean();
    w8(NAT(1) + 0x9A, 3);
    w8(NAT(1) + 0x3A, 1);
    w8(NAT(1) + 0x99, 5);
    w8(NAT(2) + 0x9A, 3);
    w8(NAT(3) + 0x9A, 3);
    put_tokens(a, 1, 4);
    w8(NAT(1) + 0x98, 4);
    put_tokens(b, 2, 3);
    w8(NAT(2) + 0x98, 3);
    put_city(c, 3, 2);
    g_nforced = 2;
    g_forced[0] = 1;
    g_forced[1] = -16;
    if (run_both(R_21914, 4, "A8 model")) {
        expect((D8(TS(c) + 2) & 1) != 0 && D8(TS(c)) == 1 && S8(NAT(1) + 0x51) == 1 && S8(NAT(3) + 0x51) == 0,
               "A8 the city changes hands");
        expect(D8(AR(c + 1) + 0x3B + 3) == 1, "A8 the owner's lost mark");
        expect(S8(AR(b + 1) + 0x0E + 2) == 3 && S8(NAT(1) + 0x99) == 5, "A8 nothing else converted");
        expect(strstr(g_real, "wait 1000") != NULL && strstr(g_real, "music") != NULL, "A8 the 1000 ms pause, music");
        expect(S16(0xBB3FAu) == 0xA9 && S16(0xBB3FCu) == 0x154 && S16(0xBB3FEu) == 0x12C && S16(0xBB400u) == 0x23,
               "A8 message area");
        expect(S16(0xF06AEu) == 3 && S16(0xF06B0u) == 1 && S16(0xF06B2u) == 2 && S16(0xF06B4u) == 3, "A8 player list");
    }
    /* A9: the same state, CONVERT on entry 0: nation 2's 3 tokens in b become nation 1's */
    base_clean();
    w8(NAT(1) + 0x9A, 3);
    w8(NAT(1) + 0x3A, 1);
    w8(NAT(1) + 0x99, 5);
    w8(NAT(2) + 0x9A, 3);
    w8(NAT(2) + 0x99, 10);
    put_tokens(a, 1, 4);
    w8(NAT(1) + 0x98, 4);
    put_tokens(b, 2, 3);
    w8(NAT(2) + 0x98, 3);
    g_nforced = 1;
    g_forced[0] = -16;
    if (run_both(R_21914, 5, "A9 model")) {
        expect(S8(AR(b + 1) + 0x0E + 2) == 0 && S8(AR(b + 1) + 0x0E + 1) == 3, "A9 the tokens converted");
        expect(S8(AR(b + 1) + 1 + 2) == 0 && S8(AR(b + 1) + 1 + 1) == 1 && S8(AR(b + 1) + 1) == 1, "A9 the markers");
        expect(S8(NAT(2) + 0x98) == 0 && S8(NAT(2) + 0x99) == 13, "A9 the loser's board and stock");
        expect(S8(NAT(1) + 0x98) == 7 && S8(NAT(1) + 0x99) == 2, "A9 the converter's board and stock");
        expect(D8(AR(b + 1) + 0x3B + 2) == 1, "A9 the lost mark");
        expect(S32(0xF40F8u) == 1, "A9 the current nation");
    }
    /* A10: FINISHED at once: no conversion */
    base_clean();
    w8(NAT(1) + 0x9A, 3);
    w8(NAT(1) + 0x3A, 1);
    w8(NAT(1) + 0x99, 5);
    w8(NAT(2) + 0x9A, 3);
    put_tokens(a, 1, 4);
    put_tokens(b, 2, 3);
    g_nforced = 1;
    g_forced[0] = -1;
    if (run_both(R_21914, 6, "A10 model"))
        expect(S8(AR(b + 1) + 0x0E + 2) == 3 && strstr(g_real, "wait 1000") == NULL && strstr(g_real, "remove BE9F4"),
               "A10 FINISHED converts nothing");
    /* A11: nobody holds Monotheism: the empty message (100 ms, the first player, home view) */
    w8(NAT(1) + 0x3A, 0);
    g_nforced = 0;
    if (run_both(R_21914, 7, "A11 model"))
        expect(strstr(g_real, "msg 100 1 -200 [] 0") != NULL && strstr(g_real, "music") == NULL, "A11 the empty message");
    /* A12: a holder without candidates: the empty message too */
    w8(NAT(1) + 0x3A, 1);
    w8(NAT(1) + 0x99, 1);
    if (run_both(R_21914, 8, "A12 model"))
        expect(strstr(g_real, "msg 100 1 -200 [] 0") != NULL && strstr(g_real, "append") == NULL, "A12 no candidate");
    /* A13: a computer holder and a PBEM human go to 0x21E8C */
    w8(NAT(1) + 0x99, 5);
    w8(NAT(1) + 0x9A, 2);
    {
        uint8_t *keep = malloc(DS_SIZE);
        if (keep != NULL)
            memcpy(keep, g_ds, DS_SIZE);
        if (run_both(R_21914, 9, "A13 model"))
            expect(strstr(g_real, "21E8C 1 1") != NULL && strstr(g_real, "msg ") == NULL, "A13 computer");
        if (keep != NULL) {   /* the recorder converted b: back to the state before it */
            memcpy(g_ds, keep, DS_SIZE);
            free(keep);
        }
    }
    w8(NAT(1) + 0x9A, 3);
    w8(0xE1624u, D8(0xE1624u) | 0x80);
    if (run_both(R_21914, 10, "A13 PBEM model"))
        expect(strstr(g_real, "21E8C 1 1") != NULL, "A13 PBEM human");
    w8(0xE1624u, D8(0xE1624u) & 0x7F);
    /* A14: arrows clamp at both ends, then CONVERT converts the entry shown */
    base_clean();
    w8(NAT(1) + 0x9A, 3);
    w8(NAT(1) + 0x3A, 1);
    w8(NAT(1) + 0x99, 9);
    put_tokens(a, 1, 4);
    put_tokens(b, 2, 3);
    put_tokens(c, 4, 2);
    g_nforced = 6;
    g_forced[0] = -9;
    g_forced[1] = -10;
    g_forced[2] = -10;
    g_forced[3] = -10;
    g_forced[4] = -9;
    g_forced[5] = -16;
    if (run_both(R_21914, 11, "A14 model"))
        expect(S8(AR(b + 1) + 0x0E + 2) == 0 && S8(AR(c + 1) + 0x0E + 4) == 2, "A14 arrows");
    /* A15: the text helper */
    g_original = 0;
    expect(strcmp(mono_converts_fmt(0xB1EEAu), "%s converts %s by Monotheism!") == 0, "A15 fixed text 1");
    expect(strcmp(mono_converts_fmt(0xB1F07u), "%s converts %s by Monotheism!") == 0, "A15 fixed text 2");
    g_original = 1;
    expect(strcmp(mono_converts_fmt(0xB1EEAu), "%s converts %s by Montheism!") == 0, "A15 original text 1");
    expect(strcmp(mono_converts_fmt(0xB1F07u), "%s converts %s by Montheism!") == 0, "A15 original text 2");
    g_original = 0;
    snprintf(what, sizeof what, "A: hand vectors (a %d, b %d, c %d)", a, b, c);
    g_nforced = 0;
    printf("%s, %d failed\n", what, g_fail - f0);
}

/* ---- B. 0x227B0 on random states --------------------------------------------------------------------------------- */

static void part_b(void)
{
    int i, f0 = g_fail;
    long listed = 0, empty = 0;
    char what[64];
    for (i = 0; i < 30000; i++) {
        memcpy(g_ds, g_pristine, DS_SIZE);
        random_state();
        g_arg = 1 + (int)rnd(9);
        g_original = i & 1;
        snprintf(what, sizeof what, "B%d%s", i, g_original ? " original" : "");
        run_both(R_227B0, (uint32_t)i, what);
        if (S32(LIST_N) > 0)
            listed += S32(LIST_N);
        else
            empty++;
    }
    g_original = 0;
    printf("B: 0x227B0, 30000 runs, %d failed (entries listed %ld, empty lists %ld)\n", g_fail - f0, listed, empty);
}

/* ---- C. 0x21914 on random states ----------------------------------------------------------------------------------- */

static void part_c(void)
{
    int i, f0 = g_fail, np0 = g_np, cut0 = g_cuts;
    char what[64];
    memset(g_cov, 0, sizeof g_cov);
    for (i = 0; i < 12000; i++) {
        memcpy(g_ds, g_pristine, DS_SIZE);
        random_state();
        g_original = i & 1;
        snprintf(what, sizeof what, "C%d%s", i, g_original ? " original" : "");
        run_both(R_21914, 100000u + (uint32_t)i, what);
    }
    g_original = 0;
    printf("C: 0x21914, 12000 runs, %d failed (screens %ld, city conversions %ld, token conversions %ld, FINISHED %ld, "
           "0x21E8C %ld, empty messages %ld, not ported %d, cut %d)\n", g_fail - f0, g_cov[5], g_cov[0], g_cov[1],
           g_cov[2], g_cov[3], g_cov[4], g_np - np0, g_cuts - cut0);
}

/* ---- D. the real saves ------------------------------------------------------------------------------------------- */

static void part_d(const char *savedir, const char *gamedir)
{
    static const char *const SAVES[4] = { "SAVGAMEA.DAT", "savgameb.dat", "savgame0.dat", "savgame1.dat" };
    uint8_t *base = malloc(DS_SIZE);
    char path[1024], what[64];
    int s, i, n, f0 = g_fail;

    memset(g_cov, 0, sizeof g_cov);
    for (s = 0; s < 4 && base != NULL; s++) {
        snprintf(path, sizeof path, "%s/%s", s < 2 ? savedir : gamedir, SAVES[s]);
        if (!load_save(path)) {
            printf("FAIL D: cannot read %s\n", path);
            g_fail++;
            continue;
        }
        B = g_ds;
        mask_all();   /* the R4 game's map: every territory kept (G: an 8-nation map, see the report) */
        memcpy(base, g_ds, DS_SIZE);
        printf("D: %s round %d, players", SAVES[s], S16(0xF40FCu));
        for (n = 1; n <= 9; n++)
            printf(" %d", D8(NAT(n) + 0x9A));
        printf("\n");
        for (i = 0; i < 300; i++) {
            memcpy(g_ds, base, DS_SIZE);
            B = g_ds;
            if (i > 0)   /* Monotheism and Theology given out at random; now and then a larger stock */
                for (n = 1; n <= 9; n++) {
                    w8(NAT(n) + 0x3A, rnd(3) == 0);
                    w8(NAT(n) + 0x3B, rnd(6) == 0);
                    if (rnd(4) == 0)
                        w8(NAT(n) + 0x99, (int)rnd(50));
                }
            g_original = i & 1;
            snprintf(what, sizeof what, "D %s %d", SAVES[s], i);
            run_both(R_21914, 500000u + (uint32_t)i, what);
        }
    }
    g_original = 0;
    free(base);
    printf("D: the real saves, 1200 runs, %d failed (screens %ld, city conversions %ld, token conversions %ld, "
           "0x21E8C %ld, empty messages %ld)\n", g_fail - f0, g_cov[5], g_cov[0], g_cov[1], g_cov[3], g_cov[4]);
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:\\work\\advciv_run";
    const char *savedir = argc > 2 ? argv[2] : "C:\\work\\advciv_play";

    if (state_init(gamedir) != 0) {
        fprintf(stderr, "cannot load %s/AC_MAIN.EXE\n", gamedir);
        return 2;
    }
    if (!load_landadj(gamedir)) {
        fprintf(stderr, "cannot read %s/LANDADJ.DAT\n", gamedir);
        return 2;
    }
    g_pristine = malloc(DS_SIZE);
    M = malloc(DS_SIZE);
    g_real = malloc(LOGSZ);
    g_model = malloc(LOGSZ);
    if (g_pristine == NULL || M == NULL || g_real == NULL || g_model == NULL)
        return 2;
    memcpy(g_pristine, g_ds, DS_SIZE);
    B = g_ds;
    g_log = g_real;
    g_len = &g_real_len;

    part_a();
    part_b();
    part_c();
    part_d(savedir, gamedir);

    printf("%d checks, %d failed\n", g_checks, g_fail);
    return g_fail == 0 ? 0 : 1;
}
