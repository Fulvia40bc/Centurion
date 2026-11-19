/* cal3_test [GAMEDIR [SAVEDIR]]: unit checks for module CAL3 (cal3.c) that need no game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init; default C:\work\advciv_run, only read), LANDADJ.DAT and
   COASTADJ.DAT (the land and coast lists) and the saves savgame0.dat (start of MOVEMENT R4) and savgame1.dat (R4, all
   human); SAVEDIR holds SAVGAMEA.DAT and savgameb.dat (default C:\work\advciv_play, only read). The four saves are read
   with saveload.c's f5F05C as the game does.

   cal3.c is built with -DCAL3_TEST: its calls into code that is not ported yet (the computers' 0x1EF28, 0x20068,
   0x201B8; ANIM's 0x7A4C4) come to recorders here, which trace the call and act out a plausible effect where the
   caller depends on one (0x1EF28 returns the last volcano with the highest score, 0x201B8 the last listed city,
   0x20068 reverses the pair list so the caller's read of pair 0 shows) and return.

   Every other routine cal3.c calls is a stub below that appends one line to a trace (drawing included); those with
   data effects apply them on the data object g_ds (rules.c's city removal / reduction, move.c's f581C4, the border
   highlights). The land / coast lists are the real files' lists in a heap block, one handle per territory at 0xF8E7C /
   0xF90FC as gfxload.c's f3EED8 stores them. Clicks (f78BA4) come from a script: a seeded random choice among the
   arrows, the HERE button, -100, codes that do nothing and map clicks (indices into the current list, as f78BA4
   returns them); the same seed for the port and the model; a run that clicks more than 3000 times is cut ("CUT").

   The model is a second implementation of the six routines, written from the disassembly (not from cal3.c): it runs
   on a copy of the data object (g_ds pointed at the copy), calls the same stubs, and so writes the trace it expects.
   After each run the two traces must be equal and the whole data object must equal the model's copy.
     A. hand vectors (expected values written out): the volcano table and names; the no-city message; an eruption of
        Mt. Vesuvius (city destroyed, tokens back to stock, "MT. VESUVIUS ERUPTION CASUALTIES"); a unique Earthquake
        pair with its text (Engineering: "reduced"; the Pirates' neighbour "eliminated"); the own-city choice; the
        map-click fixes (fixed: ignored; --original: the stale / index match).
     B. f1E7A0 on 6,000 random states built on the four saves (cities around the volcanoes and of every nation,
        tokens, Engineering, humans / computers, PBEM, attitudes, priorities, a stale choice list), each with and
        without --original.
     C. f1F524 alone on 4,000 random states (many neighbouring cities: ties, pairs, Pirates); D. f1EAEC with type
        0x19 (rivers) and 0x12 on 2,000 random lists; E. f203B4 on 2,000 random pairs.
     F. deliberately broken models (nine kinds) must each be caught by part B / C runs.
   Prints each failure (the first differing trace line or byte), the counts per part, and exits 0 when all pass.

   Build: clang -std=c11 -DCAL3_TEST port/test/cal3_test.c port/src/cal3.c port/src/saveload.c port/src/state.c
          port/src/le_image.c -Iport/src -o cal3_test.exe */

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cal1.h"
#include "cal3.h"
#include "conflict.h"
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

/* ---- byte access on the current data object (g_ds: the port's or the model's copy) ---------------------------- */

static uint8_t *R;          /* the port's data object (state_init's) */
static uint8_t *M;          /* the model's copy */
static uint8_t *g_pristine;
static uint8_t *g_saves[4];
static int g_nsaves;

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
static const char *pstr(uint32_t a) { return str((uint32_t)s32(a)); }

/* the model's own address arithmetic (from the listing) */
static uint32_t nat(int n) { return 0xF14CCu + (uint32_t)(n * 199); }
static uint32_t tst(int t) { return 0xE2C18u + (uint32_t)(t * 3); }
static uint32_t area(int t) { return 0xE31B8u + (uint32_t)((t + 1) * 69); }   /* the area record of territory t */

/* ---- traces --------------------------------------------------------------------------------------------------- */

#define LOGSZ (1u << 20)
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
static int g_steps, g_cuts, g_np;
static jmp_buf g_jb;
static int g_forced[64], g_nforced, g_iforced;
static long g_cov[8];   /* port side: map clicks, -9, -10, pairs chosen by a human, own-city choices, eruptions,
                           unique pairs, computer choices */

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

/* ---- the test's random generator (states) ----------------------------------------------------------------------- */

static uint32_t g_rng = 0x2468ACE1u;
static uint32_t rnd(uint32_t n)
{
    g_rng = g_rng * 1103515245u + 12345u;
    return n ? (g_rng >> 8) % n : 0;
}

/* ---- the land / coast lists ------------------------------------------------------------------------------------ */

static int g_land_n[160], g_land[160][32], g_coast_n[160], g_coast[160][32];

static int read_lists(const char *path, int cnt[160], int list[160][32])
{
    FILE *f = fopen(path, "rb");
    uint8_t b[2];
    int t, i;
    if (f == NULL)
        return 0;
    if (fseek(f, 4, SEEK_SET) != 0) { fclose(f); return 0; }
    for (t = 0; t < 160; t++) {
        if (fread(b, 2, 1, f) != 1) { fclose(f); return 0; }
        cnt[t] = (int16_t)(b[0] | b[1] << 8);
        if (cnt[t] < 0 || cnt[t] > 32) { fclose(f); return 0; }
        for (i = 0; i < cnt[t]; i++) {
            if (fread(b, 2, 1, f) != 1) { fclose(f); return 0; }
            list[t][i] = (int16_t)(b[0] | b[1] << 8);
        }
    }
    fclose(f);
    return 1;
}

/* ---- heap stubs (saveload.c's lists and the adjacency block) ----------------------------------------------------- */

#define TAG 0x80000000u
#define SLOTS 64
static uint8_t *s_blk[SLOTS];
static uint32_t s_len[SLOTS];
static int s_adj_slot = -1;

uint32_t load_heap_alloc(uint32_t size)
{
    uint32_t i;
    if (size == 0 || size > 0xFFFFFu)
        return 0;
    for (i = 0; i < SLOTS; i++)
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
    if ((h & TAG) == 0 || slot < 1 || slot > SLOTS || s_blk[slot - 1] == NULL || off > s_len[slot - 1] ||
        n > s_len[slot - 1] - off)
        return NULL;
    return s_blk[slot - 1] + off;
}

void load_heap_free(uint32_t h)
{
    uint32_t slot = (h & ~TAG) >> 20;
    if ((h & TAG) == 0 || slot < 1 || slot > SLOTS || (int)slot - 1 == s_adj_slot)
        return;
    free(s_blk[slot - 1]);
    s_blk[slot - 1] = NULL;
}

static void heap_reset(void)
{
    int i;
    for (i = 0; i < SLOTS; i++)
        if (i != s_adj_slot) {
            free(s_blk[i]);
            s_blk[i] = NULL;
        }
}

static uint32_t g_adj_handle;

/* the lists into one block: land records then coast records, as f3EED8 stores them (int16 n, int16 t[n]) */
static void build_adj(void)
{
    uint32_t size = 2u * (160u * 33u * 2u), off = 0;
    uint8_t *blk;
    int t, i;
    g_adj_handle = load_heap_alloc(size);
    s_adj_slot = (int)(((g_adj_handle & ~TAG) >> 20) - 1u);
    blk = load_heap_ptr(g_adj_handle, size);
    for (t = 0; t < 160; t++) {
        w32(0xF8E7Cu + 4u * (uint32_t)t, (int32_t)(g_adj_handle + off));
        blk[off] = (uint8_t)g_land_n[t]; blk[off + 1] = (uint8_t)(g_land_n[t] >> 8);
        for (i = 0; i < g_land_n[t]; i++) {
            blk[off + 2 + 2 * (uint32_t)i] = (uint8_t)g_land[t][i];
            blk[off + 3 + 2 * (uint32_t)i] = (uint8_t)(g_land[t][i] >> 8);
        }
        off += 2u + 2u * (uint32_t)g_land_n[t];
    }
    for (t = 0; t < 160; t++) {
        w32(0xF90FCu + 4u * (uint32_t)t, (int32_t)(g_adj_handle + off));
        blk[off] = (uint8_t)g_coast_n[t]; blk[off + 1] = (uint8_t)(g_coast_n[t] >> 8);
        for (i = 0; i < g_coast_n[t]; i++) {
            blk[off + 2 + 2 * (uint32_t)i] = (uint8_t)g_coast[t][i];
            blk[off + 3 + 2 * (uint32_t)i] = (uint8_t)(g_coast[t][i] >> 8);
        }
        off += 2u + 2u * (uint32_t)g_coast_n[t];
    }
}

/* ---- shared effects (on g_ds) --------------------------------------------------------------------------------- */

static int okt(int t) { return t >= 0 && t < 160; }
static int okn(int n) { return n >= 1 && n <= 13; }

static void eff_78F04(int t, int n)   /* rules.h's f78F04 */
{
    uint32_t rec = nat(n);
    int c, j, k;
    if (!okt(t) || !okn(n))
        return;
    w8(tst(t) + 2, rb(tst(t) + 2) & ~1);
    w8(tst(t), 0);
    w8(tst(t) + 1, 0xFF);
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
    uint32_t rec = nat(n);
    int c = sb(rec + 0x51);
    w8(tst(t) + 2, rb(tst(t) + 2) | 1);
    w8(tst(t), n);
    w8(tst(t) + 1, 3);
    w8(rec + 0x52u + (uint32_t)c, t + 1);
    w8(rec + 0x51, c + 1);
}

static void eff_78FEC(int t, int n)   /* f78F04, then min(stock, limit) tokens of n placed */
{
    uint32_t rec = nat(n);
    int lim, k;
    if (!okt(t) || !okn(n))
        return;
    eff_78F04(t, n);
    lim = (rb(0xE1639u + 35u * (uint32_t)t) >> 3) & 7;
    k = sb(rec + 0x99) < lim ? sb(rec + 0x99) : lim;
    if (k <= 0)
        return;
    if (sb(area(t) + 1u + (uint32_t)n) != n) {
        w8(area(t) + 1, rb(area(t) + 1) + 1);
        w8(area(t) + 1u + (uint32_t)n, n);
    }
    w8(area(t) + 0x0Eu + (uint32_t)n, rb(area(t) + 0x0Eu + (uint32_t)n) + k);
    w8(rec + 0x99, sb(rec + 0x99) - k);
    w8(rec + 0x98, sb(rec + 0x98) + k);
}

static void eff_581C4(int t, int n)
{
    if (!okt(t) || !okn(n))
        return;
    if (sb(area(t) + 1u + (uint32_t)n) != 0 && rb(area(t) + 0x0Eu + (uint32_t)n) == 0) {
        w8(area(t) + 1, rb(area(t) + 1) - 1);
        w8(area(t) + 1u + (uint32_t)n, 0);
    }
}

/* ---- stubs (both sides) --------------------------------------------------------------------------------------- */

/* cal2.c's shared river-name text fix (CAL2 module; its own test checks it) */
const char *cal2_river_text(const char *r)
{
    return (!g_original && strcmp(r, "TIGRUS-EUPHRATES") == 0) ? "TIGRIS-EUPHRATES" : r;
}

_Noreturn void not_ported(uint32_t addr)
{
    tr("NOT PORTED %X", (unsigned)addr);
    g_np++;
    longjmp(g_jb, 1);
}

int f78AE4(void)   /* saveload.c's f7BCD4 only (not reached here) */
{
    uint32_t s = (uint32_t)s32(0xCAC5Cu) * 0x15A4E35u + 1u;
    w32(0xCAC5Cu, (int32_t)s);
    return (int)((s >> 16) & 0x7FFF);
}
void f76F70(int n, uint8_t *cards) { (void)n; (void)cards; }   /* saveload.c's 0x726DC only */

int fg_vbcopy(int minx, int maxx, int miny, int maxy, int newx, int newy, int source, int dest)
{
    tr("vbcopy %d %d %d %d %d %d %d %d", minx, maxx, miny, maxy, newx, newy, source, dest);
    return 0;
}
int fg_vbopen(int handle) { tr("vbopen %d", handle); return 0; }
void fg_vbclose(void) { tr("vbclose"); }
int fg_setcolor(int color) { tr("color %d", color); return 0; }
void f3CD6C(int8_t pic) { tr("bg %d (%d)", pic, rb(0xEE168u)); }
void f7876C(void) { tr("phase"); }
void f3B05C(uint32_t dlg) { tr("dialog %X", (unsigned)dlg); }
void f459E4(int n) { tr("panel %d", n); }
void f3CBBC(void) { tr("paste"); }
void f3A9F4(const char *text) { tr("left %s", text); }
void f79270(int wait, int n, int t, const char *text, int anim)
{
    int i;
    char b[200];
    for (i = 0; i < 160 && i < (int)sizeof b - 1; i++)
        b[i] = (char)('0' + (rb(0xFFB60u + (uint32_t)i) != 0));
    b[i] = 0;
    tr("msg %d %d %d %s %d [%s]", wait, n, t, text, anim, b);
}
void f3F670(void) { tr("clrb"); memset(g_ds + (0xFFB60u - DS_BASE), 0, 0xA0); }
void f3F624(int t, int c) { tr("border %d %d", t, c); w8(0xFFB60u + (uint32_t)t, c); }
void f7853C(uint32_t dlg) { tr("append %X", (unsigned)dlg); }
void f78664(uint32_t dlg) { tr("remove %X", (unsigned)dlg); }
void f3AF8C(uint32_t spr, int x, int y) { tr("sprite %X %d %d", (unsigned)spr, x, y); }
void f3A0B0(int x, int y, int w, const char *s) { tr("text %d %d %d %s", x, y, w, s); }
void f3C9D0(int x, int y, int w, int h, int flags) { tr("frame %d %d %d %d %d", x, y, w, h, flags); }
void f3C284(uint32_t dlg, int id, uint32_t label) { tr("label %X %X %s", (unsigned)dlg, (unsigned)id, str(label)); }
void f3DA60(int t) { tr("centre %d", t); }
void f78F04(int t, int n) { tr("78F04 %d %d", t, n); eff_78F04(t, n); }
void f78FEC(int t, int n) { tr("78FEC %d %d", t, n); eff_78FEC(t, n); }
void f581C4(int t, int n) { tr("581C4 %d %d", t, n); eff_581C4(t, n); }
void f1E69C(int k) { tr("1E69C %d", k); }
int f5CDE8(int anim, const char *title)
{
    int i;
    char b[64];
    for (i = 0; i < 14; i++) {
        b[2 * i] = (char)('A' + rb(0xF547Cu + (uint32_t)i) % 26);
        b[2 * i + 1] = (char)('a' + rb(0xF548Cu + (uint32_t)i) % 26);
    }
    b[28] = 0;
    tr("5CDE8 %d %s %s h%d", anim, title, b, s16(0xBB400u));
    return 0;
}

/* the click script: the codes a chooser can get from f78BA4 (map indices only below [0xF7C9C], as f78BA4 returns) */
static int g_maxclicks;
int f78BA4(uint32_t dlg, int map)
{
    int r, cnt = s32(0xF7C9Cu);
    cut_check();
    if (g_iforced < g_nforced) {
        r = g_forced[g_iforced++];
    } else if (g_steps > g_maxclicks) {
        r = -1;
    } else {
        switch (srnd(12)) {
        case 0: r = -100; break;
        case 1: case 2: r = -9; break;
        case 3: case 4: case 5: r = -10; break;
        case 6: case 7: r = cnt > 0 ? (int)srnd((uint32_t)cnt) : -100; break;
        case 8: r = -1; break;
        case 9: r = -5; break;
        case 10: r = -11; break;
        default: r = -0x50; break;
        }
    }
    tr("click %X %d %d", (unsigned)dlg, map, r);
    if (g_ds == R) {
        if (r >= 0) g_cov[0]++;
        if (r == -9) g_cov[1]++;
        if (r == -10) g_cov[2]++;
    }
    return r;
}

/* ---- recorders for the unported calls (cal3.c built with -DCAL3_TEST) --------------------------------------------- */

int cal3_test_f1EF28(int n, int32_t *scores)
{
    int i, b = 0;
    for (i = 0; i < 3; i++)
        if (scores[i] >= scores[b])
            b = i;
    tr("AI 1EF28 %d %d %d %d -> %d", n, (int)scores[0], (int)scores[1], (int)scores[2], b);
    if (g_ds == R) g_cov[7]++;
    return b;
}

void cal3_test_f20068(int n)
{
    int c = s32(0xF7C9Cu), i;
    uint8_t tmp[8];
    tr("AI 20068 %d %d", n, c);
    if (g_ds == R) g_cov[7]++;
    for (i = 0; i < c / 2; i++) {   /* reverse the pairs, so the caller's pair 0 is the last one */
        memcpy(tmp, g_ds + (0xF7300u + 8u * (uint32_t)i - DS_BASE), 8);
        memcpy(g_ds + (0xF7300u + 8u * (uint32_t)i - DS_BASE), g_ds + (0xF7300u + 8u * (uint32_t)(c - 1 - i) - DS_BASE), 8);
        memcpy(g_ds + (0xF7300u + 8u * (uint32_t)(c - 1 - i) - DS_BASE), tmp, 8);
    }
}

int cal3_test_f201B8(int n)
{
    int c = s32(0xF7C9Cu), t = c > 0 ? s32(0xF7CA0u + 4u * (uint32_t)(c - 1)) : -1;
    tr("AI 201B8 %d %d -> %d", n, c, t);
    if (g_ds == R) g_cov[7]++;
    return t;
}

void cal3_test_f7A4C4(int anim) { tr("ANIM 7A4C4 %d", anim); }

/* ==== the model (from the listing) ================================================================================== */

static int g_mut;   /* 0 = the listing; 1..9 = deliberately broken (part F) */

static int m_1EAEC(int n, int type);
static void m_1F25C(int v, int n);
static void m_1F524(int n);
static int m_1FDDC(int n);
static void m_203B4(int t, int t2, int n);

static void m_bg3(void)
{
    f3CD6C(3);
    w8(0xEE168u, 3);
    fg_vbcopy(0, 639, 0, 479, 0, 479, s32(0xF52CCu), s32(0xF52C8u));
}

static void m_1E7A0(int k)
{
    char b[100];
    int n = sb(0xF77E6u + 3u * (uint32_t)k), sc[3] = {0, 0, 0}, in[3] = {0, 0, 0}, cnt = 0, v = 0, i, j, m, t;
    int best, ties, idx = 0;
    uint32_t rec = nat(n);
    int32_t sc32[3];
    if (rb(rec + 0x51) == 0) {
        snprintf(b, sizeof b, "%s has no cities to be affected by volcanos or earthquakes.", pstr(0xC768Cu + 4u * (uint32_t)n));
        f79270(-500, n, -200, b, 1);
        f1E69C(k);
        return;
    }
    /* 0xE2CB4.. : the volcano territories' states, as the listing names them */
    if (((rb(0xE2CB6u) & 1) && rb(0xE2CB4u) == n) || ((rb(0xE2D97u) & 1) && rb(0xE2D95u) == n)) { cnt++; in[0] = 1; v = 0; }
    if (((rb(0xE2CFEu) & 1) && rb(0xE2CFCu) == n) || ((rb(0xE2D04u) & 1) && rb(0xE2D02u) == n)) { cnt++; in[1] = 1; v = 1; }
    if ((rb(0xE2D13u) & 1) && rb(0xE2D11u) == n) { cnt++; in[2] = 1; v = 2; }
    if (cnt == 0) {
        m_1F524(n);
        f1E69C(k);
        return;
    }
    if (cnt >= 2) {
        for (i = 0; i < 3; i++) {
            if (!in[i])
                continue;
            sc[i] = 0;
            for (j = 0; j < 2; j++) {
                t = s16(0xBDE18u + 4u * (uint32_t)i + 2u * (uint32_t)j);
                if (t == -1)
                    continue;
                if ((rb(tst(t) + 2) & 1) && (g_mut == 1 || rb(tst(t)) != 10)) {
                    sc[i] += 6;
                    continue;
                }
                for (m = 1; m <= 9; m++)
                    sc[i] += sb(area(t) + 0x0Eu + (uint32_t)m);
            }
        }
        best = 0;
        ties = 0;
        for (i = 0; i < 3; i++) {
            if (g_mut == 2 ? sc[i] > best : sc[i] >= best) {
                ties = sc[i] == best ? ties + 1 : 0;
                best = sc[i];
                idx = i;
            }
        }
        if (ties == 0) {
            v = idx;
        } else if (!(rb(0xE1624u) & 0x80) && sb(rec + 0x9A) == 3) {
            w32(0xF7C9Cu, 0);
            for (i = 0; i < 3; i++)
                if (sc[i] == best) {
                    w32(0xF7CA0u + 4u * (uint32_t)s32(0xF7C9Cu), i);
                    w32(0xF7C9Cu, s32(0xF7C9Cu) + 1);
                }
            v = m_1EAEC(n, 0x13);
        } else {
            for (i = 0; i < 3; i++)
                sc32[i] = sc[i];
            v = cal3_test_f1EF28(n, sc32);
        }
    }
    m_1F25C(v, n);
    f1E69C(k);
}

static int m_1EAEC(int n, int type)
{
    char b[100];
    int idx = 0, again = 1, r, c, j, nm = s32(0xC768Cu + 4u * (uint32_t)n);
    m_bg3();
    fg_vbopen(s32(0xF52C8u));
    f3AF8C(0xF51ACu, 251, 405);
    f3AF8C(0xF51B4u, 376, 405);
    fg_vbclose();
    f459E4(n);
    b[0] = 0;
    switch (type) {
    case 0x13:
        snprintf(b, sizeof b, "%s is about to undergo a Volcanic Eruption. Choose which volcano will erupt.", str((uint32_t)nm));
        f3C284(0xBE21Cu, 42, 0xB1A8Eu);
        break;
    case 0x12:
        snprintf(b, sizeof b, "%s is about to be hit by an Earthquake. Choose which territory will be hit.", str((uint32_t)nm));
        f3C284(0xBE21Cu, 42, 0xB1AE5u);
        break;
    case 0x19:
        snprintf(b, sizeof b, "%s is about to be Flooded. Choose which river will flood.", str((uint32_t)nm));
        f3C284(0xBE21Cu, 42, 0xB1B2Bu);
        break;
    default:
        break;
    }
    f3A9F4(b);
    fg_setcolor(5);
    f7853C(0xBE21Cu);
    f3B05C(0xF596Cu);
    for (;;) {
        c = s32(0xF7CA0u + 4u * (uint32_t)idx);
        fg_vbopen(s32(0xF52C8u));
        f3C9D0(265, 405, 110, 19, 5);
        fg_setcolor(5);
        if (type == 0x13) f3A0B0(265, 409, 110, pstr(0xBDE24u + 4u * (uint32_t)c));
        if (type == 0x12) f3A0B0(265, 409, 110, pstr(0xC77D8u + 4u * (uint32_t)c));
        if (type == 0x19) {
            const char *rv = pstr(0xBEE0Cu + 22u * (uint32_t)c);   /* the "TIGRUS" text fix unless --original */
            f3A0B0(265, 409, 110, (!g_original && strcmp(rv, "TIGRUS-EUPHRATES") == 0) ? "TIGRIS-EUPHRATES" : rv);
        }
        fg_vbclose();
        f3F670();
        if (type == 0x13) {
            f3F624(s16(0xBDE18u + 4u * (uint32_t)c), 15);
            if (s16(0xBDE1Au + 4u * (uint32_t)c) != -1)
                f3F624(s16(0xBDE1Au + 4u * (uint32_t)c), 15);
        }
        if (type == 0x12)
            f3F624(c, 15);
        if (type == 0x19)
            for (j = 0; j < s16(0xBEE0Au + 22u * (uint32_t)c); j++)
                f3F624(s16(0xBEE10u + 22u * (uint32_t)c + 2u * (uint32_t)j), 15);
        if (again) {
            if (type == 0x13) f3DA60(s16(0xBDE18u + 4u * (uint32_t)c));
            if (type == 0x12) f3DA60(c);
            if (type == 0x19) f3DA60(s16(0xBEE10u + 22u * (uint32_t)c));
        }
        f3CBBC();
        if (again) {
            cal3_test_f7A4C4(-600);
            again = 0;
        }
        r = f78BA4(0xF596Cu, 1);
        if (r == -1)
            break;
        if (r >= 0) {
            if (g_original || type == 0x12 || g_mut == 8) {
                idx = r;
                again = 1;
            }
        } else if (r == -9) {
            if (idx > 0) { idx--; again = 1; }
        } else if (r == -10) {
            if (idx < s32(0xF7C9Cu) - (g_mut == 7 ? 0 : 1)) { idx++; again = 1; }
        }
    }
    f78664(0xBE21Cu);
    return s32(0xF7CA0u + 4u * (uint32_t)idx);
}

static void m_1F25C(int v, int n)
{
    char b[100], *p;
    int j, t, k, tok, o;
    w16(0xBB400u, 20);
    memset(g_ds + (0xF547Cu - DS_BASE), 0, 14);
    memset(g_ds + (0xF548Cu - DS_BASE), 0, 14);
    for (j = 0; j < 2; j++) {
        t = s16(0xBDE18u + 4u * (uint32_t)v + 2u * (uint32_t)j);
        if (t == -1)
            continue;
        if (rb(tst(t) + 2) & 1) {
            o = rb(tst(t));
            w8(0xF548Cu + (uint32_t)o, rb(0xF548Cu + (uint32_t)o) + 1);
            f78F04(t, o);
            continue;
        }
        for (k = 1; k <= 9; k++) {
            tok = rb(area(t) + 0x0Eu + (uint32_t)k);
            if (tok == 0)
                continue;
            w8(0xF547Cu + (uint32_t)k, rb(0xF547Cu + (uint32_t)k) + tok);
            if (g_mut != 5)
                w8(nat(k) + 0x99, rb(nat(k) + 0x99) + tok);
            w8(nat(k) + 0x98, rb(nat(k) + 0x98) - tok);
            w8(area(t) + 0x0Eu + (uint32_t)k, 0);
            f581C4(t, k);
        }
    }
    m_bg3();
    f7876C();
    f7853C(0xC5214u);
    f3B05C(0xF596Cu);
    f3F670();
    f459E4(n);
    f3F624(s16(0xBDE18u + 4u * (uint32_t)v), 15);
    if (s16(0xBDE1Au + 4u * (uint32_t)v) != -1)
        f3F624(s16(0xBDE1Au + 4u * (uint32_t)v), 15);
    f3DA60(s16(0xBDE18u + 4u * (uint32_t)v));
    f3CBBC();
    snprintf(b, sizeof b, "%s eruption casualties", pstr(0xBDE24u + 4u * (uint32_t)v));
    for (p = b; *p; p++)
        if (*p >= 'a' && *p <= 'z')
            *p = (char)(*p - 32);
    f5CDE8(2000, b);
    f78664(0xC5214u);
    w16(0xBB400u, 35);
}

/* the candidate entry j of city i: 0xF6F68 + 100i + 10j {city, neighbour, score, attitude, priority} */
static uint32_t ent(int i, int j) { return 0xF6F68u + (uint32_t)(100 * i + 10 * j); }

static int hw(uint32_t h) { const uint8_t *q = load_heap_ptr(h, 2); return q ? (int16_t)(q[0] | q[1] << 8) : 0; }

static void m_1F524(int n)
{
    uint32_t rec = nat(n);
    int i, j, m, t, u, c, best = 0, ties = 0, s, o, a = 0, b = -1, e, dup;
    w32(0xF783Cu, 18);
    for (i = 0; i < sb(rec + 0x51); i++) {
        t = rb(rec + 0x52u + (uint32_t)i) - 1;
        /* the land list (as the model reads it from the file, checked against the handle's) */
        c = hw((uint32_t)s32(0xF8E7Cu + 4u * (uint32_t)t));
        if (okt(t) && c != g_land_n[t])
            tr("MODEL: land list %d differs", t);
        for (j = 0; j < c; j++) {
            u = okt(t) ? g_land[t][j] : 0;
            w16(ent(i, j), t);
            w16(ent(i, j) + 2, u);
            w16(ent(i, j) + 4, 6);
        }
        w16(0xF72ECu + 2u * (uint32_t)i, c);
        c = hw((uint32_t)s32(0xF90FCu + 4u * (uint32_t)t));
        for (j = 0; j < c; j++) {
            u = okt(t) ? g_coast[t][j] : 0;
            dup = 0;
            if (g_mut != 3)
                for (m = 0; m < s16(0xF72ECu + 2u * (uint32_t)i); m++)
                    if (u16(ent(i, m) + 2) == (u & 0xFFFF))
                        dup = 1;
            if (dup)
                continue;
            m = s16(0xF72ECu + 2u * (uint32_t)i);
            w16(ent(i, m), t);
            w16(ent(i, m) + 2, u);
            w16(ent(i, m) + 4, 6);
            w16(0xF72ECu + 2u * (uint32_t)i, m + 1);
        }
    }
    for (i = 0; i < sb(rec + 0x51); i++)
        for (j = 0; j < s16(0xF72ECu + 2u * (uint32_t)i); j++) {
            u = s16(ent(i, j) + 2);
            o = rb(tst(u));
            if (!(rb(tst(u) + 2) & 1) || o == n)
                w16(ent(i, j) + 4, 0);
            else if (rb(nat(o) + 0x2A) != 0 && g_mut != 4)
                w16(ent(i, j) + 4, 0);
            else {
                w16(ent(i, j) + 4, s16(ent(i, j) + 4) + 6);
                if (o == 10) {
                    w16(ent(i, j) + 6, -9000);
                    w16(ent(i, j) + 8, -9000);
                } else {
                    w16(ent(i, j) + 6, u16(0xF52E0u + (uint32_t)(40 * n + 4 * o)));
                    w16(ent(i, j) + 8, sb(0xEED84u + (uint32_t)(160 * n + u)));
                }
            }
            s = s16(ent(i, j) + 4);
            if (s > best) { best = s; ties = 0; }
            else if (s == best) ties++;
        }
    if (best == 0) {
        w32(0xF7C9Cu, 0);
        for (i = 0; i < sb(rec + 0x51); i++) {
            w32(0xF7CA0u + 4u * (uint32_t)i, rb(rec + 0x52u + (uint32_t)i) - 1);
            w32(0xF7C9Cu, i + 1);
        }
        if (!(rb(0xE1624u) & 0x80) && sb(rec + 0x9A) == 3) {
            a = m_1EAEC(n, 0x12);
        } else {
            a = cal3_test_f201B8(n);
        }
        m_203B4(a, -1, n);
        return;
    }
    if (ties == 0) {
        for (i = 0; i < sb(rec + 0x51); i++)
            for (j = 0; j < s16(0xF72ECu + 2u * (uint32_t)i); j++)
                if (s16(ent(i, j) + 4) == best) {
                    m_203B4(s16(ent(i, j)), s16(ent(i, j) + 2), n);
                    return;
                }
        return;
    }
    w32(0xF7C9Cu, 0);
    for (i = 0; i < sb(rec + 0x51); i++)
        for (j = 0; j < s16(0xF72ECu + 2u * (uint32_t)i); j++)
            if (s16(ent(i, j) + 4) == best) {
                c = s32(0xF7C9Cu);
                w16(0xF7300u + 8u * (uint32_t)c, u16(ent(i, j)));
                w16(0xF7302u + 8u * (uint32_t)c, u16(ent(i, j) + 2));
                w16(0xF7304u + 8u * (uint32_t)c, u16(ent(i, j) + (g_mut == 9 ? 8 : 6)));
                w16(0xF7306u + 8u * (uint32_t)c, u16(ent(i, j) + (g_mut == 9 ? 6 : 8)));
                w32(0xF7C9Cu, c + 1);
            }
    if (!(rb(0xE1624u) & 0x80) && sb(rec + 0x9A) == 3) {
        w16(0xF77E0u, m_1FDDC(n));
    } else {
        cal3_test_f20068(n);
        w16(0xF77E0u, 0);
    }
    e = s16(0xF77E0u);
    a = s16(0xF7300u + 8u * (uint32_t)e);
    b = s16(0xF7302u + 8u * (uint32_t)e);
    m_203B4(a, b, n);
}

static int m_1FDDC(int n)
{
    char b[100];
    int idx = 0, again = 1, r;
    m_bg3();
    fg_vbopen(s32(0xF52C8u));
    f3AF8C(0xF51ACu, 196, 405);
    f3AF8C(0xF51B4u, 431, 405);
    fg_vbclose();
    f459E4(n);
    snprintf(b, sizeof b, "%s is about to be hit by an Earthquake. Choose which territories will be hit.",
             pstr(0xC768Cu + 4u * (uint32_t)n));
    f3A9F4(b);
    fg_setcolor(5);
    f7853C(0xBE608u);
    f3B05C(0xF596Cu);
    for (;;) {
        int c1 = s16(0xF7300u + 8u * (uint32_t)idx), c2 = s16(0xF7302u + 8u * (uint32_t)idx);
        fg_vbopen(s32(0xF52C8u));
        f3C9D0(210, 405, 220, 19, 5);
        fg_setcolor(5);
        snprintf(b, sizeof b, "%s/%s", pstr(0xC77D8u + 4u * (uint32_t)c1), pstr(0xC77D8u + 4u * (uint32_t)c2));
        f3A0B0(210, 409, 220, b);
        fg_vbclose();
        f3F670();
        f3F624(c1, 15);
        f3F624(c2, 15);
        if (again)
            f3DA60(c1);
        f3CBBC();
        if (again) {
            cal3_test_f7A4C4(-600);
            again = 0;
        }
        r = f78BA4(0xF596Cu, 1);
        if (r == -1)
            break;
        if (r >= 0) {
            if (g_original || g_mut == 8) { idx = r; again = 1; }
        } else if (r == -9) {
            if (idx > 0) { idx--; again = 1; }
        } else if (r == -10) {
            if (idx < s32(0xF7C9Cu) - (g_mut == 7 ? 0 : 1)) { idx++; again = 1; }
        }
    }
    f78664(0xBE21Cu);
    return idx;
}

static void m_203B4(int t, int t2, int n)
{
    char b[200], q[100];
    int eng = rb(nat(n) + 0x2A) != 0, o;
    snprintf(b, sizeof b, "Earthquake strikes %s!  The city in %s was %s.", pstr(0xC768Cu + 4u * (uint32_t)n),
             pstr(0xC77D8u + 4u * (uint32_t)t), eng ? "reduced" : "eliminated");
    if (t2 != -1) {
        o = rb(tst(t2));
        if (o == 10)
            snprintf(q, sizeof q, " The neighboring Pirate city in %s was eliminated.", pstr(0xC77D8u + 4u * (uint32_t)t2));
        else
            snprintf(q, sizeof q, " The neighboring %s city in %s was reduced.", pstr(0xC76C4u + 4u * (uint32_t)o),
                     pstr(0xC77D8u + 4u * (uint32_t)t2));
        strncat(b, q, sizeof b - strlen(b) - 1);
    }
    if (eng)
        f78FEC(t, n);
    else
        f78F04(t, n);
    if (t2 != -1) {
        o = rb(tst(t2));
        if (o == 10 && g_mut != 6)
            f78F04(t2, o);
        else
            f78FEC(t2, o);
        f3F624(t2, 15);
    }
    f79270(-500, n, -t, b, 2);
}

/* ==== running and comparing ============================================================================================ */

static int g_checks, g_fail;

static void check(int ok, const char *what)
{
    g_checks++;
    if (!ok) {
        g_fail++;
        printf("FAIL %s\n", what);
    }
}

enum { RUN_1E7A0, RUN_1F524, RUN_1EAEC, RUN_203B4 };

static int g_quiet_fail;   /* part F: count, do not print */

/* runs the port on R and the model on M from the same start (R as it is now); returns 1 if they agree */
static int run_both(int what, int a, int b, int c, uint32_t seed, int maxclicks, const char *tag)
{
    volatile int ok = 1;
    size_t i;
    int pr = 0;
    memcpy(M, R, DS_SIZE);
    /* the port */
    g_ds = R;
    g_log = g_real; g_len = &g_real_len; g_real_len = 0; g_real[0] = 0;
    g_srng = seed; g_steps = 0; g_iforced = 0; g_maxclicks = maxclicks;
    if (setjmp(g_jb) == 0) {
        switch (what) {
        case RUN_1E7A0: f1E7A0(a); break;
        case RUN_1F524: f1F524(a); break;
        case RUN_1EAEC: pr = f1EAEC(a, b); tr("return %d", pr); break;
        default: f203B4(a, b, c); break;
        }
    }
    /* the model */
    g_ds = M;
    g_log = g_model; g_len = &g_model_len; g_model_len = 0; g_model[0] = 0;
    g_srng = seed; g_steps = 0; g_iforced = 0;
    if (setjmp(g_jb) == 0) {
        switch (what) {
        case RUN_1E7A0: m_1E7A0(a); break;
        case RUN_1F524: m_1F524(a); break;
        case RUN_1EAEC: pr = m_1EAEC(a, b); tr("return %d", pr); break;
        default: m_203B4(a, b, c); break;
        }
    }
    g_ds = R;
    if (g_real_len != g_model_len || memcmp(g_real, g_model, g_real_len) != 0) {
        ok = 0;
        if (!g_quiet_fail) {
            const char *p = g_real, *q = g_model;
            int line = 1;
            while (*p && *p == *q) {
                if (*p == '\n') line++;
                p++; q++;
            }
            while (p > g_real && p[-1] != '\n') p--;
            while (q > g_model && q[-1] != '\n') q--;
            printf("FAIL %s (original %d): trace line %d\n  port : %.150s\n  model: %.150s\n", tag, g_original, line,
                   p, q);
        }
    }
    for (i = 0; i < DS_SIZE && ok; i++)
        if (R[i] != M[i]) {
            ok = 0;
            if (!g_quiet_fail)
                printf("FAIL %s (original %d): byte 0x%X port %02X model %02X\n", tag, g_original,
                       (unsigned)(i + DS_BASE), R[i], M[i]);
        }
    return ok;
}

/* ---- states ------------------------------------------------------------------------------------------------------ */

static void clear_cities(void)
{
    int t, n;
    for (t = 0; t < 160; t++) {
        w8(tst(t), 0);
        w8(tst(t) + 1, 0xFF);
        w8(tst(t) + 2, 0);
    }
    for (n = 1; n <= 13; n++) {
        w8(nat(n) + 0x51, 0);
        memset(g_ds + (nat(n) + 0x52u - DS_BASE), 0, 10);
    }
}

static int is_sea(int t) { return g_land_n[t] == 0 && g_coast_n[t] > 6; }

static void add_city(int t, int n)
{
    if (!okt(t) || is_sea(t) || (rb(tst(t) + 2) & 1) || sb(nat(n) + 0x51) >= 9)
        return;
    eff_78E74(t, n);
}

/* a random state on save s: cities around the volcanoes and around the victim's cities, tokens, flags */
static int make_state(int s, int quake_bias)
{
    int n, i, t, k, cnt, u;
    static const int volc[5] = {52, 127, 76, 78, 83};
    memcpy(R, g_saves[s], DS_SIZE);
    g_ds = R;
    clear_cities();
    n = 1 + (int)rnd(9);
    for (k = 1; k <= 10; k++) {
        w8(nat(k) + 0x2A, rnd(3) == 0);
        w8(nat(k) + 0x99, (int)rnd(30));
        w8(nat(k) + 0x98, (int)rnd(40));
        if (k <= 9 && k != n)
            w8(nat(k) + 0x9A, rnd(4) == 0 ? 0 : (rnd(2) ? 2 : 3));
    }
    w8(nat(n) + 0x9A, rnd(5) == 0 ? 2 : 3);
    if (rnd(10) == 0) w8(0xE1624u, rb(0xE1624u) | 0x80); else w8(0xE1624u, rb(0xE1624u) & ~0x80);
    /* the victim's cities: at the volcanoes (unless the Earthquake is wanted) and elsewhere */
    cnt = (int)rnd(10);
    for (i = 0; i < cnt; i++) {
        if (!quake_bias && rnd(3) == 0)
            t = volc[rnd(5)];
        else
            t = (int)rnd(157);
        add_city(t, n);
    }
    /* other nations' cities: next to the victim's (ties) and at the volcanoes */
    for (i = 0; i < sb(nat(n) + 0x51); i++) {
        t = rb(nat(n) + 0x52u + (uint32_t)i) - 1;
        for (k = 0; k < g_land_n[t] + g_coast_n[t]; k++) {
            u = k < g_land_n[t] ? g_land[t][k] : g_coast[t][k - g_land_n[t]];
            if (rnd(3) == 0)
                add_city(u, rnd(6) == 0 ? 10 : 1 + (int)rnd(9));
        }
    }
    for (i = 0; i < 5; i++)
        if (rnd(3) == 0)
            add_city(volc[i], rnd(5) == 0 ? 10 : 1 + (int)rnd(9));
    /* tokens at the volcanoes and elsewhere */
    for (i = 0; i < 5; i++)
        for (k = 1; k <= 11; k++)
            w8(area(volc[i]) + 0x0Eu + (uint32_t)k, rnd(3) == 0 ? (int)rnd(6) : 0);
    for (i = 0; i < 40; i++) {
        t = (int)rnd(157);
        k = 1 + (int)rnd(9);
        w8(area(t) + 0x0Eu + (uint32_t)k, (int)rnd(5));
        w8(area(t) + 1u + (uint32_t)k, k);
    }
    for (i = 0; i < 100; i++)
        w32(0xF52E0u + 4u * (uint32_t)i, (int32_t)rnd(2000) - 1000);
    for (i = 0; i < 1600; i++)
        w8(0xEED84u + (uint32_t)i, (int)rnd(256));
    /* a stale choice list (what the last chooser left) */
    w32(0xF7C9Cu, (int)rnd(12));
    for (i = 0; i < 20; i++)
        w32(0xF7CA0u + 4u * (uint32_t)i, (int)rnd(160));
    k = (int)rnd(12);
    w8(0xF77E4u + 3u * (uint32_t)k, 0x13);
    w8(0xF77E6u + 3u * (uint32_t)k, n);
    w8(0xF097Cu, rnd(2) ? 4 : 0);
    return k;
}

/* ==== A. hand vectors ================================================================================================ */

static void part_a(void)
{
    int f0 = g_fail, t, k;
    char want[300];
    g_original = 0;
    memcpy(R, g_saves[0], DS_SIZE);
    g_ds = R;
    check(s16(0xBDE18u) == 52 && s16(0xBDE1Au) == 127 && s16(0xBDE1Cu) == 76 && s16(0xBDE1Eu) == 78 &&
          s16(0xBDE20u) == 83 && s16(0xBDE22u) == -1, "A volcano table 52/127 76/78 83/-1");
    check(strcmp(pstr(0xBDE24u), "Mt. Vesuvius") == 0 && strcmp(pstr(0xBDE28u), "Mt. Etna") == 0 &&
          strcmp(pstr(0xBDE2Cu), "Mt. Thera") == 0, "A volcano names");

    /* the no-city message */
    clear_cities();
    w8(0xF77E6u, 3);
    snprintf(want, sizeof want, "msg -500 3 -200 %s has no cities to be affected by volcanos or earthquakes. 1",
             pstr(0xC768Cu + 12u));
    check(run_both(RUN_1E7A0, 0, 0, 0, 1, 50, "A no cities") && strstr(g_real, want) != NULL &&
          strstr(g_real, "1E69C 0") != NULL, "A no-city message and the card out");

    /* an eruption of Mt. Vesuvius: n = 2 has a city in CAMPANIA (52), nations 3 and 4 have tokens in NEAPOLIS (127) */
    memcpy(R, g_saves[0], DS_SIZE);
    clear_cities();
    add_city(52, 2);
    w8(area(127) + 0x0Eu + 3u, 2); w8(area(127) + 1u + 3u, 3);
    w8(area(127) + 0x0Eu + 4u, 1); w8(area(127) + 1u + 4u, 4);
    w8(nat(3) + 0x99, 5); w8(nat(3) + 0x98, 10);
    w8(0xF77E6u + 3u, 2);
    check(run_both(RUN_1E7A0, 1, 0, 0, 2, 50, "A Vesuvius"), "A Vesuvius: port = model");
    check(strstr(g_real, "5CDE8 2000 MT. VESUVIUS ERUPTION CASUALTIES") != NULL, "A Vesuvius: the title in capitals");
    check((rb(tst(52) + 2) & 1) == 0 && rb(0xF548Cu + 2u) == 1, "A Vesuvius: the city destroyed, counted");
    check(rb(area(127) + 0x0Eu + 3u) == 0 && sb(nat(3) + 0x99) == 7 && sb(nat(3) + 0x98) == 8 &&
          rb(0xF547Cu + 3u) == 2 && rb(0xF547Cu + 4u) == 1, "A Vesuvius: tokens back to stock, counted");
    check(s16(0xBB400u) == 0x23, "A Vesuvius: the message area height back to 0x23");

    /* the Earthquake: n = 5 with Engineering, one city in t; one neighbour city of nation 6 (no Engineering) */
    memcpy(R, g_saves[0], DS_SIZE);
    clear_cities();
    for (t = 0; t < 157 && (is_sea(t) || g_land_n[t] < 2); t++)
        ;
    add_city(t, 5);
    add_city(g_land[t][0], 6);
    w8(nat(5) + 0x2A, 1);
    w8(nat(6) + 0x2A, 0);
    w8(nat(5) + 0x9A, 3);
    w8(0xF77E6u + 6u, 5);
    snprintf(want, sizeof want, "Earthquake strikes %s!  The city in %s was reduced. The neighboring %s city in %s "
             "was reduced.", pstr(0xC768Cu + 20u), pstr(0xC77D8u + 4u * (uint32_t)t), pstr(0xC76C4u + 24u),
             pstr(0xC77D8u + 4u * (uint32_t)g_land[t][0]));
    check(run_both(RUN_1E7A0, 2, 0, 0, 3, 50, "A quake") && strstr(g_real, want) != NULL, "A Earthquake text");
    check(strstr(g_real, "78FEC") != NULL && rb(0xF783Cu) == 0x12, "A Earthquake: reduced, [0xF783C] = 0x12");

    /* the Pirates' neighbour, no Engineering: eliminated twice */
    memcpy(R, g_saves[0], DS_SIZE);
    clear_cities();
    add_city(t, 5);
    add_city(g_land[t][0], 10);
    w8(nat(5) + 0x2A, 0);
    w8(nat(10) + 0x2A, 0);
    w8(0xF77E6u, 5);
    snprintf(want, sizeof want, "was eliminated. The neighboring Pirate city in %s was eliminated.",
             pstr(0xC77D8u + 4u * (uint32_t)g_land[t][0]));
    check(run_both(RUN_1E7A0, 0, 0, 0, 4, 50, "A pirate") && strstr(g_real, want) != NULL, "A Pirate neighbour");

    /* no neighbouring city: the human chooses one of the own cities (0x12 list = territories) */
    memcpy(R, g_saves[0], DS_SIZE);
    clear_cities();
    add_city(t, 5);
    add_city(g_land[t][1], 5);
    w8(nat(5) + 0x2A, 0);
    w8(nat(5) + 0x9A, 3);
    w8(0xE1624u, rb(0xE1624u) & ~0x80);
    w8(0xF77E6u, 5);
    g_forced[0] = 1; g_forced[1] = -1; g_nforced = 2;
    check(run_both(RUN_1E7A0, 0, 0, 0, 5, 50, "A own city"), "A own city: port = model");
    snprintf(want, sizeof want, "Earthquake strikes %s!  The city in %s was eliminated.", pstr(0xC768Cu + 20u),
             pstr(0xC77D8u + 4u * (uint32_t)g_land[t][1]));
    check(strstr(g_real, want) != NULL && strstr(g_real, "label BE21C 2A STRIKE HERE") != NULL,
          "A own city: the map click picks the second city");
    g_nforced = 0;

    /* the map-click fixes: a volcano tie (map click 1 then HERE): fixed ignores it, --original takes volcano 1 */
    for (k = 0; k < 2; k++) {
        g_original = k;
        memcpy(R, g_saves[0], DS_SIZE);
        clear_cities();
        add_city(52, 4);
        add_city(76, 4);
        for (t = 1; t <= 9; t++) {   /* equal scores: no tokens in NEAPOLIS and SYRACUSE */
            w8(area(127) + 0x0Eu + (uint32_t)t, 0);
            w8(area(78) + 0x0Eu + (uint32_t)t, 0);
        }
        w8(nat(4) + 0x9A, 3);
        w8(0xE1624u, rb(0xE1624u) & ~0x80);
        w8(0xF77E6u, 4);
        g_forced[0] = 1; g_forced[1] = -1; g_nforced = 2;
        check(run_both(RUN_1E7A0, 0, 0, 0, 6, 50, "A volcano click"), "A volcano click: port = model");
        check(strstr(g_real, k ? "MT. ETNA ERUPTION" : "MT. VESUVIUS ERUPTION") != NULL,
              k ? "A volcano click --original: index 1 = Mt. Etna" : "A volcano click fixed: ignored, Mt. Vesuvius");
        g_nforced = 0;
    }
    g_original = 0;
    printf("A. hand vectors: %d failed\n", g_fail - f0);
}

/* ==== B..E random runs =============================================================================================== */

static void part_b(int runs)
{
    int i, k, fails = 0;
    char tag[64];
    for (i = 0; i < runs; i++) {
        g_original = i & 1;
        k = make_state(i % g_nsaves, (i & 6) == 0);
        snprintf(tag, sizeof tag, "B run %d", i);
        if (!run_both(RUN_1E7A0, k, 0, 0, 1000u + (uint32_t)i, 40, tag))
            fails++;
        if (strstr(g_real, "5CDE8 2000") != NULL) g_cov[5]++;
        if (strstr(g_real, "Choose which territories") != NULL) g_cov[3]++;
        g_checks++;
    }
    g_fail += fails;
    printf("B. f1E7A0: %d runs, %d differ\n", runs, fails);
}

static void part_c(int runs)
{
    int i, n, fails = 0;
    char tag[64];
    for (i = 0; i < runs; i++) {
        g_original = i & 1;
        n = sb(0xF77E6u + 3u * (uint32_t)make_state(i % g_nsaves, 1));
        if (sb(nat(n) + 0x51) == 0)
            add_city((int)rnd(157), n);
        if (sb(nat(n) + 0x51) == 0)
            continue;
        snprintf(tag, sizeof tag, "C run %d", i);
        if (!run_both(RUN_1F524, n, 0, 0, 5000u + (uint32_t)i, 40, tag))
            fails++;
        if (strstr(g_real, "neighboring") != NULL && strstr(g_real, "click") == NULL && strstr(g_real, "AI") == NULL)
            g_cov[6]++;
        if (strstr(g_real, "label BE21C 2A STRIKE") != NULL) g_cov[4]++;
        if (strstr(g_real, "Choose which territories") != NULL) g_cov[3]++;
        g_checks++;
    }
    g_fail += fails;
    printf("C. f1F524: %d runs, %d differ\n", runs, fails);
}

static void part_d(int runs)
{
    int i, j, fails = 0, type, cnt;
    char tag[64];
    for (i = 0; i < runs; i++) {
        g_original = i & 1;
        memcpy(R, g_saves[i % g_nsaves], DS_SIZE);
        g_ds = R;
        type = (i & 2) ? 0x19 : 0x12;
        cnt = 1 + (int)rnd(type == 0x19 ? 5 : 9);
        w32(0xF7C9Cu, cnt);
        for (j = 0; j < cnt; j++)
            w32(0xF7CA0u + 4u * (uint32_t)j, type == 0x19 ? (int)rnd(5) : (int)rnd(157));
        snprintf(tag, sizeof tag, "D run %d", i);
        if (!run_both(RUN_1EAEC, 1 + (int)rnd(9), type, 0, 9000u + (uint32_t)i, 60, tag))
            fails++;
        g_checks++;
    }
    g_fail += fails;
    printf("D. f1EAEC (rivers, own cities): %d runs, %d differ\n", runs, fails);
}

static void part_e(int runs)
{
    int i, fails = 0, t, t2, n;
    char tag[64];
    for (i = 0; i < runs; i++) {
        g_original = i & 1;
        make_state(i % g_nsaves, 1);
        n = 1 + (int)rnd(9);
        t = (int)rnd(157);
        t2 = rnd(4) == 0 ? -1 : (int)rnd(157);
        add_city(t, n);
        if (t2 >= 0)
            add_city(t2, rnd(4) == 0 ? 10 : 1 + (int)rnd(9));
        snprintf(tag, sizeof tag, "E run %d", i);
        if (!run_both(RUN_203B4, t, t2, n, 0, 10, tag))
            fails++;
        g_checks++;
    }
    g_fail += fails;
    printf("E. f203B4: %d runs, %d differ\n", runs, fails);
}

/* ==== F. deliberately broken models =================================================================================== */

static void part_f(void)
{
    int mut, i, caught, k;
    char what[80];
    g_quiet_fail = 1;
    for (mut = 1; mut <= 9; mut++) {
        g_mut = mut;
        caught = 0;
        for (i = 0; i < 4000 && caught == 0; i++) {
            g_original = 0;
            if (mut == 8 || (i & 1)) {
                k = make_state(i % g_nsaves, (i & 2) == 0);
                caught += !run_both(RUN_1E7A0, k, 0, 0, 20000u + (uint32_t)i, 40, "F");
            } else {
                int n = sb(0xF77E6u + 3u * (uint32_t)make_state(i % g_nsaves, 1));
                if (sb(nat(n) + 0x51) == 0)
                    add_city((int)rnd(157), n);
                caught += !run_both(RUN_1F524, n, 0, 0, 30000u + (uint32_t)i, 40, "F");
            }
        }
        snprintf(what, sizeof what, "F broken model %d caught (after %d runs)", mut, i);
        check(caught > 0, what);
        printf("  %s: %s\n", what, caught ? "yes" : "NO");
    }
    g_mut = 0;
    g_quiet_fail = 0;
}

/* ==== main ============================================================================================================ */

static int load_save(const char *path, int slot)
{
    FILE *fp;
    int16_t version = 0;
    char desc[0x34];

    memcpy(R, g_pristine, DS_SIZE);
    g_ds = R;
    heap_reset();
    fp = fopen(path, "rb");
    if (fp == NULL)
        return 0;
    memset(desc, 0, sizeof desc);
    f5F05C(&version, desc, fp);
    fclose(fp);
    g_saves[slot] = malloc(DS_SIZE);
    if (g_saves[slot] == NULL)
        return 0;
    memcpy(g_saves[slot], R, DS_SIZE);
    printf("save %s: \"%s\" (version %d)\n", path, desc, version);
    return 1;
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:\\work\\advciv_run";
    const char *savedir = argc > 2 ? argv[2] : "C:\\work\\advciv_play";
    char path[600];
    const char *names[4][2] = {{NULL, "SAVGAMEA.DAT"}, {NULL, "savgameb.dat"}, {NULL, "savgame0.dat"},
                               {NULL, "savgame1.dat"}};
    int i;

    setvbuf(stdout, NULL, _IONBF, 0);
    names[0][0] = names[1][0] = savedir;
    names[2][0] = names[3][0] = gamedir;
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
    snprintf(path, sizeof path, "%s/LANDADJ.DAT", gamedir);
    if (!read_lists(path, g_land_n, g_land))
        return 2;
    snprintf(path, sizeof path, "%s/COASTADJ.DAT", gamedir);
    if (!read_lists(path, g_coast_n, g_coast))
        return 2;
    build_adj();   /* the handles go into the pristine image, so every save keeps them */
    memcpy(g_pristine, R, DS_SIZE);
    for (i = 0; i < 4; i++) {
        snprintf(path, sizeof path, "%s/%s", names[i][0], names[i][1]);
        if (load_save(path, g_nsaves))
            g_nsaves++;
    }
    if (g_nsaves == 0)
        return 2;
    /* the saves' own handles at 0xF8E7C / 0xF90FC: f5F05C does not touch them (checked) */
    for (i = 0; i < g_nsaves; i++)
        check(memcmp(g_saves[i] + (0xF8E7Cu - DS_BASE), g_pristine + (0xF8E7Cu - DS_BASE), 0x500) == 0,
              "the list handles survive the save");

    part_a();
    part_b(6000);
    part_c(4000);
    part_d(2000);
    part_e(2000);
    part_f();

    printf("coverage (port side): map clicks %ld, -9 %ld, -10 %ld, human pair choices %ld, own-city choices %ld, "
           "eruptions %ld, unique pairs %ld, computer choices %ld; not_ported %d, cut %d\n",
           g_cov[0], g_cov[1], g_cov[2], g_cov[3], g_cov[4], g_cov[5], g_cov[6], g_cov[7], g_np, g_cuts);
    printf("%d checks, %d failed\n", g_checks, g_fail);
    return g_fail == 0 ? 0 : 1;
}
