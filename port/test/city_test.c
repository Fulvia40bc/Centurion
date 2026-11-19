/* city_test [GAMEDIR [SAVEDIR]]: unit checks for module CITY (city.c) that need no game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init; default C:\work\advciv_run, only read) and the saves
   savgame0.dat (start of MOVEMENT R4) and savgame1.dat (R4, all human); SAVEDIR holds SAVGAMEA.DAT and savgameb.dat
   (default C:\work\advciv_play, only read). All four are read with saveload.c's f5F05C as the game does.
   city.c is built with -DCITY_TEST: its two calls into the computers' code (0x333E8, 0x5DE5C) come to recorders here
   that act out a plausible effect (0x333E8: the city of 0x78E74, the tokens of f57CE8, the state bits &~2 &3; 0x5DE5C:
   the first city of the nation reduced as f78FEC does here) and return.

   Every routine city.c calls is a stub below that appends one line to a trace (drawing included); the few with data
   effects apply them on the buffer B: f581C4 (the marker and territory list, from move.h's record), f3F670 / f3F624
   (the border colours), f79A9C (the clamp), f78FEC (the city removed as f78F04, then min(stock, limit) tokens placed).
   Clicks (f78BA4) come from a script: a seeded random choice among the codes the screen offers (enabled buttons, map
   clicks on listed entries, -100, codes that do nothing), the same seed for the port and the model; a run that clicks
   more than 3000 times is cut off ("CUT") identically on both sides (that is how the endless loop of 0x5C750 under
   --original shows). f7BA64 returns a per-run value (attended / unattended).

   The model is a second implementation of the seven routines, written from the disassembly (not from city.c): it
   runs on a copy of the data object, calls the same leaf stubs, and so writes the trace it expects. After each run
   the two traces must be equal and the whole data object must equal the model's copy. Parts:
     A. hand vectors (expected values written out): 0x334F4's costs (6 / 12, Architecture 3+3 / 6+6, two nations, the
        dead switch [0xF40B2] = 1 -> 1 / 2 but Architecture still 6 / 12); 0x5C238's surplus (a city territory, the
        limit, Agriculture, 0xF5934); the human screen with Architecture (contribution, BUILD, Architecture used once);
        the computer's reorder with and without --original (territory s - 1); 0x5C750 with no city (fixed: returns;
        --original: the endless REDUCE screen, cut); 0x5C880's empty list (fixed, and --original: byte 0xFFB5F,
        "PIRACY").
     B. 0x334F4 on 30,000 random states; C. 0x5C238 (show 1 and 0) on 6,000; D. 0x5C62C on 4,000 and 0x5C880 alone
        on 3,000 (mode 0 / 1, several nations); E. 0x32214 on 8,000 (humans, computers, PBEM, toggles, special);
     F. 0x32214, 0x5C238 and 0x5C62C on the four real saves, 200 click scripts each, with and without --original.
   Prints each failure (the first differing trace line or byte), the counts per part, and exits 0 when all pass.

   Build: clang -std=c11 -DCITY_TEST port/test/city_test.c port/src/city.c port/src/saveload.c port/src/state.c
          port/src/le_image.c -Iport/src -o city_test.exe */

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "city.h"
#include "fg.h"
#include "game.h"
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
#define TF(t)       (0xE1639u + 35u * (uint32_t)(t))
#define LIST        0xF7CA0u
#define LIST_N      0xF7C9Cu
#define NEEDS       0xF7F20u
#define BORDERS     0xFFB60u

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

/* ---- the click script, f7BA64, the cut ------------------------------------------------------------------------ */

static uint32_t g_srng;
static int g_steps, g_unatt, g_en3B;
static jmp_buf g_jb;
static int g_np, g_cuts;
static long g_cov[6];   /* port side: human builds, automatic builds, 0x333E8, 0x333E8 with Architecture, reductions, 0x5DE5C */
static uint32_t g_screen;   /* the dialog appended to the panel now: 0xC154C (build) or 0xC5DD8 (reduce) */

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

/* the codes a screen of 0xF596C may return: map clicks on listed entries, the arrows, the screen's own buttons, -1,
   and codes that do nothing there (-100, -5, -14 / -13 on the other screen, -50) */
static int pick(int build_screen)
{
    int cnt = s32(LIST_N);
    for (;;) {
        switch (srnd(12)) {
        case 0: return -100;
        case 1: case 2: if (cnt > 0) return (int)srnd((uint32_t)cnt); break;
        case 3: return -9;
        case 4: return -10;
        case 5: if (build_screen) { if (g_en3B) return -11; } else return -11; break;
        case 6: if (build_screen) { if (g_en3B) return -12; } else return -50; break;
        case 7: case 8: return build_screen ? -13 : -14;
        case 9: if (srnd(3) == 0) return -1; break;
        case 10: return build_screen ? -14 : -13;
        default: return -5;
        }
    }
}

/* ---- shared effects (on B) ------------------------------------------------------------------------------------ */

/* move.h's f581C4: marker +1+n != 0 and tokens +0x0E+n == 0: count - 1, marker 0, t + 1 out of n's list */
static void eff_581C4(int t, int n)
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

static void eff_city_add(int t, int n)   /* 0x78E74 */
{
    uint32_t rec = NAT(n);
    w8(TS(t) + 2, b8(TS(t) + 2) | 1);
    w8(TS(t), n);
    w8(TS(t) + 1, b8(0xF40FCu));
    w8(rec + 0x52u + (uint32_t)s8(rec + 0x51), t + 1);
    w8(rec + 0x51, s8(rec + 0x51) + 1);
}

static void eff_78FEC(int t, int n)   /* f78F04, then min(stock, limit) tokens of n placed in t */
{
    uint32_t rec = NAT(n), a;
    int c, j, k, lim;
    if (t < 0 || t > 156 || n < 1 || n > 9)
        return;
    w8(TS(t) + 2, b8(TS(t) + 2) & ~1);
    w8(TS(t), 0);
    w8(TS(t) + 1, 0xFF);
    c = s8(rec + 0x51);
    for (j = 0; j < c; j++)
        if (b8(rec + 0x52u + (uint32_t)j) == t + 1)
            break;
    if (j < c) {
        for (k = j; k < c - 1; k++)
            w8(rec + 0x52u + (uint32_t)k, b8(rec + 0x53u + (uint32_t)k));
        w8(rec + 0x52u + (uint32_t)(c - 1), 0);
        w8(rec + 0x51, c - 1);
    }
    lim = (b8(TF(t)) >> 3) & 7;
    k = s8(rec + 0x99) < lim ? s8(rec + 0x99) : lim;
    if (k <= 0)
        return;
    a = AR(t + 1);
    if (s8(a + 1u + (uint32_t)n) != n) {
        w8(a + 1, b8(a + 1) + 1);
        w8(a + 1u + (uint32_t)n, n);
    }
    w8(a + 0x0Eu + (uint32_t)n, b8(a + 0x0Eu + (uint32_t)n) + k);
    w8(rec + 0x99, s8(rec + 0x99) - k);
    w8(rec + 0x98, s8(rec + 0x98) + k);
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
    if ((h & TAG) == 0 || slot < 1 || slot > 32)
        return;
    free(s_blk[slot - 1]);
    s_blk[slot - 1] = NULL;
}

int fg_vbcopy(int minx, int maxx, int miny, int maxy, int newx, int newy, int source, int dest)
{
    tr("vbcopy %d %d %d %d %d %d %d %d", minx, maxx, miny, maxy, newx, newy, source, dest);
    return 0;
}
int fg_vbopen(int handle) { tr("vbopen %d", handle); return 0; }
void fg_vbclose(void) { tr("vbclose"); }
int fg_setcolor(int color) { tr("color %d", color); return 0; }
void f783B8(uint32_t dlg, int base) { tr("783B8 %X %d", (unsigned)dlg, base); }
void f3CD6C(int8_t pic) { tr("bg %d", pic); }
void f7876C(void) { tr("phase"); }
void f3B05C(uint32_t dlg) { tr("dialog %X", (unsigned)dlg); }
void f459E4(int n) { tr("panel %d", n); }
void f3CBBC(void) { tr("paste"); }
void f3A924(void) { tr("area"); }
void f3A9F4(const char *text) { tr("left %s", text); }
void f78A20(int t) { tr("wait %d", t); }
void f7844C(uint32_t dlg, int base) { tr("7844C %X %d %d", (unsigned)dlg, base, s32(0xF40F8u)); }
void f79270(int wait, int n, int t, const char *text, int anim)
{
    tr("msg %d %d %d %s %d", wait, n, t, text, anim);
    if (B == g_ds && strstr(text, "builds a city") != NULL)
        g_cov[1]++;
}
void f3F670(void) { tr("clrb"); memset(B + (BORDERS - DS_BASE), 0, 0xA0); }
void f3F624(int t, int c) { tr("border %d %d", t, c); w8(BORDERS + (uint32_t)t, c); }
void f7853C(uint32_t dlg) { tr("append %X", (unsigned)dlg); g_screen = dlg; }
void f78664(uint32_t dlg) { tr("remove %X", (unsigned)dlg); g_screen = 0; }
void f3AF8C(uint32_t spr, int x, int y) { tr("sprite %X %d %d", (unsigned)spr, x, y); }
void f3A3A0(int x, int y, const char *s) { tr("right %d %d %s", x, y, s); }
void f3A044(int x, int y, const char *s) { tr("print %d %d %s", x, y, s); }
void f3A0B0(int x, int y, int w, const char *s) { tr("text %d %d %d %s", x, y, w, s); }
void f3C9D0(int x, int y, int w, int h, int flags) { tr("frame %d %d %d %d %d", x, y, w, h, flags); }
void f3C3D0(uint32_t dlg, int id) { tr("off %X %X", (unsigned)dlg, (unsigned)id); if (id == 0x3B) g_en3B = 0; }
void f3C314(uint32_t dlg, int id) { tr("on %X %X", (unsigned)dlg, (unsigned)id); if (id == 0x3B) g_en3B = 1; }
int  f3BF30(uint32_t dlg, int id) { tr("en? %X %X %d", (unsigned)dlg, (unsigned)id, g_en3B); return id == 0x3B ? g_en3B : 0; }
void f3C720(uint32_t dlg) { tr("alloff %X", (unsigned)dlg); }
void f3C6A0(uint32_t dlg) { tr("allon %X", (unsigned)dlg); }
void f3DA60(int t) { tr("centre %d", t); }
void f3D52C(int dx, int dy) { tr("scroll %d %d", dx, dy); if (B == g_ds && g_screen == 0xC154Cu) g_cov[0]++; }
void f581C4(int t, int n) { tr("581C4 %d %d", t, n); eff_581C4(t, n); }
void f78FEC(int t, int n) { tr("78FEC %d %d", t, n); eff_78FEC(t, n); if (B == g_ds) g_cov[4]++; }
int  f7BA64(void) { tr("7BA64 %d", g_unatt); return g_unatt; }
void f79A9C(int32_t *p)
{
    if (s32(LIST_N) <= *p)
        *p = s32(LIST_N) - 1;
    if (*p < 0)
        *p = 0;
    tr("clamp %d", (int)*p);
}

static int g_forced[32], g_nforced, g_iforced;   /* hand vectors: these clicks first, then -1 */

int f78BA4(uint32_t dlg, int map)
{
    int r;
    cut_check();
    if (g_nforced > 0)
        r = g_iforced < g_nforced ? g_forced[g_iforced++] : -1;
    else
        r = pick(g_screen == 0xC154Cu);
    tr("click %X %d %d", (unsigned)dlg, map, r);
    return r;
}

/* the computers' routines (M7): recorders with a plausible effect */
void city_test_f333E8(int t, int n, int k, int arch)
{
    uint32_t a = AR(t + 1);
    tr("333E8 %d %d %d %d", t, n, k, arch);
    if (B == g_ds) {
        g_cov[2]++;
        g_cov[3] += arch != 0;
    }
    eff_city_add(t, n);
    if (k > 0) {
        w8(a + 0x0Eu + (uint32_t)n, b8(a + 0x0Eu + (uint32_t)n) - k);
        eff_581C4(t, n);
    }
    w8(TS(t) + 2, b8(TS(t) + 2) & ~2 & 3);
}

void city_test_f5DE5C(int n, const char *msg)
{
    int t;
    tr("5DE5C %d %s", n, msg);
    if (B == g_ds)
        g_cov[5]++;
    cut_check();
    for (t = 0; t < 157; t++)
        if ((b8(TS(t) + 2) & 1) != 0 && b8(TS(t)) == n) {
            eff_78FEC(t, n);
            break;
        }
}

/* ---- the model (B == M), written from the disassembly ---------------------------------------------------------- */

static void m_players(void)
{
    int n, c = 0;
    for (n = 1; n <= 9; n++)
        if (b8(NAT(n) + 0x9A) != 0)
            w16(0xF06B0u + 2u * (uint32_t)c++, n);
    w16(0xF06AEu, c);
}

static void m_mapscreen(void)
{
    f3CD6C(3);
    w8(0xEE168u, 3);
    fg_vbcopy(0, 639, 0, 479, 0, 479, s32(0xF52CCu), s32(0xF52C8u));
}

static int tok(int t, int n) { return s8(AR(t + 1) + 0x0Eu + (uint32_t)n); }
static int site(int t) { return (b8(TF(t)) & 0x40) != 0; }

static void m_add(int t, int need)
{
    int c = s32(LIST_N);
    w32(LIST + 4u * (uint32_t)c, t);
    w32(NEEDS + 4u * (uint32_t)c, need);
    w32(LIST_N, c + 1);
}

static void m_add_t(int t)   /* the choice list of 0x5C880 (no need written) */
{
    int c = s32(LIST_N);
    w32(LIST + 4u * (uint32_t)c, t);
    w32(LIST_N, c + 1);
}

/* 0x334F4 */
static void m334F4(int arch, int n)
{
    int c6 = 6, c12 = 12, t, tre = s8(NAT(n) + 0x3E);
    if (s16(0xF40B2u) != 0) {
        c6 = 1;
        c12 = 2;
    }
    w32(LIST_N, 0);
    for (t = 0; t < 157; t++)
        w32(LIST + 4u * (uint32_t)t, -1);
    for (t = 0; t < 157; t++) {
        if (b8(TS(t) + 2) & 1)
            continue;
        if (arch && s8(AR(t + 1) + 1) == 1) {
            if (site(t) && tok(t, n) >= 3 && tok(t, n) + tre >= 6)
                m_add(t, 6);
            else if (!site(t) && tok(t, n) >= 6 && tok(t, n) + tre >= 12)
                m_add(t, 12);
        } else if (site(t)) {
            if (tok(t, n) >= c6)
                m_add(t, c6);
        } else if (tok(t, n) >= c12) {
            m_add(t, c12);
        }
    }
}

/* the city of n in entry i built inline by 0x32214 (no 0x78E74), `fromtok` of its need paid from the tokens there */
static void m_inline_city(int i, int n, int fromtok)
{
    uint32_t rec = NAT(n);
    int t = s32(LIST + 4u * (uint32_t)i), need = s32(NEEDS + 4u * (uint32_t)i);
    w8(TS(t) + 2, b8(TS(t) + 2) | 1);
    w8(TS(t), n);
    w8(TS(t) + 1, b8(0xF40FCu));
    w8(rec + 0x52u + (uint32_t)s8(rec + 0x51), t + 1);
    w8(rec + 0x51, b8(rec + 0x51) + 1);
    w8(rec + 0x3E, b8(rec + 0x3E) - (need - fromtok));
    w8(AR(t + 1) + 0x0Eu + (uint32_t)n, b8(AR(t + 1) + 0x0Eu + (uint32_t)n) - fromtok);
    w8(rec + 0x99, b8(rec + 0x99) + need);
    w8(rec + 0x98, b8(rec + 0x98) - fromtok);
    f581C4(t, n);
}

static void m_screen(int n, int arch, int *mx, int *mn)
{
    uint32_t rec = NAT(n);
    int sel = 0, redraw = 1, ctb = 0, r, t, i;
    char b[32];

    f7853C(0xC154Cu);
    if (arch) {
        f3C314(0xF596Cu, 0x3B);
        f3C314(0xF596Cu, 0x3C);
    } else {
        f3C3D0(0xF596Cu, 0x3B);
        f3C3D0(0xF596Cu, 0x3C);
    }
    m_mapscreen();
    fg_vbopen(s32(0xF52C8u));
    f3AF8C(0xF51ACu, 251, 365);
    f3AF8C(0xF51B4u, 376, 365);
    f3AF8C(0xF51ACu, 296, 385);
    f3AF8C(0xF51B4u, 330, 385);
    fg_setcolor(5);
    f3A0B0(170, 340, 300, "CANDIDATE SITES FOR CITIES");
    fg_setcolor(f3BF30(0xF596Cu, 0x3B) == 1 ? 5 : 23);
    f3A3A0(293, 389, "TREASURY");
    f3A044(346, 389, "CONTRIBUTION");
    fg_vbclose();
    f7876C();
    f3B05C(0xF596Cu);
    f7844C(0xF596Cu, s32(0xF5940u));
    while (s8(rec + 0x51) < 9 && s32(LIST_N) > 0) {
        t = s32(LIST + 4u * (uint32_t)sel);
        fg_vbopen(s32(0xF52C8u));
        f3C9D0(265, 365, 110, 19, 5);
        fg_setcolor(5);
        f3A0B0(265, 369, 110, tname(t));
        if (arch && redraw) {
            *mx = site(t) ? 3 : 6;
            if (s8(rec + 0x3E) < *mx)
                *mx = s8(rec + 0x3E);
            ctb = s32(NEEDS + 4u * (uint32_t)sel) - tok(t, n);
            if (ctb < 0)
                ctb = 0;
            *mn = ctb;
        }
        if (ctb > *mx)
            ctb = *mx;
        f3C9D0(310, 385, 19, 19, 5);
        fg_setcolor(f3BF30(0xF596Cu, 0x3B) == 1 ? 5 : 23);
        snprintf(b, sizeof b, "%d", ctb);
        f3A0B0(310, 389, 19, b);
        fg_vbclose();
        f3F670();
        f3F624(t, 15);
        if (redraw) {
            f459E4(n);
            f3DA60(t);
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
        if (r == -9 && sel > 0) {
            sel--;
            redraw = 1;
        } else if (r == -10 && sel < s32(LIST_N) - 1) {
            sel++;
            redraw = 1;
        } else if (r == -11 && ctb > *mn) {
            ctb--;
        } else if (r == -12 && ctb < *mx) {
            ctb++;
        } else if (r == -13) {
            m_inline_city(sel, n, s32(NEEDS + 4u * (uint32_t)sel) - ctb);
            f3D52C(0, 0);
            for (i = sel; i < s32(LIST_N) - 1; i++)
                w32(LIST + 4u * (uint32_t)i, s32(LIST + 4u * (uint32_t)(i + 1)));
            w32(LIST_N, s32(LIST_N) - 1);
            if (ctb != 0) {
                arch = 0;
                fg_vbopen(s32(0xF52C8u));
                fg_setcolor(23);
                snprintf(b, sizeof b, "%d", ctb);
                f3A0B0(310, 389, 19, b);
                f3A3A0(293, 389, "TREASURY");
                f3A044(346, 389, "CONTRIBUTION");
                fg_vbclose();
                f3C3D0(0xF596Cu, 0x3B);
                f3C3D0(0xF596Cu, 0x3C);
            }
            m334F4(arch, n);
            {
                int32_t s = sel;
                f79A9C(&s);
                sel = s;
            }
            redraw = 1;
            ctb = 0;
            f459E4(n);
            f3CBBC();
        }
    }
    f78664(0xC154Cu);
}

static void m_computer(int n)
{
    uint32_t rec = NAT(n);
    int idx = n - 1, half, cnt, i, j, p, t, k, d, arch = s8(rec + 0x31), waited = 0;
    int st[160], nd[160];

    m_mapscreen();
    f3B05C(0xF596Cu);
    f459E4(n);
    f3F670();
    half = s8(rec + 0x98) / 2;
    if (s8(rec + 0x51) + s32(LIST_N) > 9) {
        if (b8(rec + 0x31) != 0) {
            cnt = s32(LIST_N);
            for (i = 0; i < cnt; i++) {
                st[i] = s32(LIST + 4u * (uint32_t)i);
                nd[i] = s32(NEEDS + 4u * (uint32_t)i);
            }
            w32(LIST_N, 0);
            for (p = 1; p <= 4; p++)
                for (i = 0; i < cnt; i++) {
                    /* the tokens of area record st (territory st - 1) in the original, st + 1 fixed */
                    int have = s8(AR(g_original ? st[i] : st[i] + 1) + 0x0Eu + (uint32_t)n) >= nd[i];
                    int mark = (b8(TS(st[i]) + 2) & 2) != 0;
                    int flag = (b8(0xF3580u + 314u * (uint32_t)idx + 2u * (uint32_t)st[i]) & 0x80) != 0;
                    if ((p == 1 && have) || (p == 2 && !have && mark) || (p == 3 && !have && !mark && !flag) ||
                        (p == 4 && !have && !mark && flag))
                        m_add(st[i], nd[i]);
                }
        }
        for (i = 1; i < s32(LIST_N); i++)
            for (j = s32(LIST_N) - 1; j >= i; j--)
                if (s32(NEEDS + 4u * (uint32_t)(j - 1)) > s32(NEEDS + 4u * (uint32_t)j)) {
                    int a = s32(LIST + 4u * (uint32_t)(j - 1)), b = s32(NEEDS + 4u * (uint32_t)(j - 1));
                    w32(LIST + 4u * (uint32_t)(j - 1), s32(LIST + 4u * (uint32_t)j));
                    w32(LIST + 4u * (uint32_t)j, a);
                    w32(NEEDS + 4u * (uint32_t)(j - 1), s32(NEEDS + 4u * (uint32_t)j));
                    w32(NEEDS + 4u * (uint32_t)j, b);
                }
    }
    for (i = 0; i < s32(LIST_N); i++) {
        t = s32(LIST + 4u * (uint32_t)i);
        if (t == -1 || s8(rec + 0x51) >= 9 || tok(t, n) < s32(NEEDS + 4u * (uint32_t)i))
            continue;
        f3F670();
        f3F624(t, 15);
        f3DA60(t);
        f3CBBC();
        f78A20(300);
        waited = 1;
        k = tok(t, n) < s32(NEEDS + 4u * (uint32_t)i) ? tok(t, n) : s32(NEEDS + 4u * (uint32_t)i);
        w8(rec + 0x98, b8(rec + 0x98) - k);
        w8(rec + 0x99, b8(rec + 0x99) + k);
        city_test_f333E8(t, n, k, 0);
        w32(LIST + 4u * (uint32_t)i, -1);
    }
    if (b8(rec + 0x31) != 0)
        for (i = 0; i < s32(LIST_N); i++) {
            if (s8(rec + 0x51) >= half)
                break;
            if (s8(rec + 0x51) >= 9)
                continue;
            t = s32(LIST + 4u * (uint32_t)i);
            if (tok(t, n) + s8(rec + 0x3E) < s32(NEEDS + 4u * (uint32_t)i) || s8(AR(t + 1) + 1) != 1)
                continue;
            k = tok(t, n) < s32(NEEDS + 4u * (uint32_t)i) ? tok(t, n) : s32(NEEDS + 4u * (uint32_t)i);
            d = s32(NEEDS + 4u * (uint32_t)i) - k;
            if (!arch || !d)
                continue;
            w8(rec + 0x98, b8(rec + 0x98) - k);
            w8(rec + 0x99, b8(rec + 0x99) + k + d);
            w8(rec + 0x3E, b8(rec + 0x3E) - d);
            city_test_f333E8(t, n, k, 99);
            waited = 1;
            w32(LIST + 4u * (uint32_t)i, -1);
            arch = 0;
        }
    if (b8(0xF0980u) & 1)
        return;
    k = f7BA64() == 0 ? 2000 : (s16(0xF40BAu) == 0 ? 500 : 1000);
    if (!waited)
        f78A20(k);
}

static void m32214(void)
{
    int none = 1, mx = 0, mn = 0, ns = 0, sp = 0, n, i, arch;
    uint32_t rec;
    char b[100];

    m_players();
    f783B8(0xF596Cu, s32(0xF5940u));
    for (i = 1; i <= 9 && !ns; i++)
        ns = b8(NAT(i) + 0x31) != 0;
    if ((b8(0xE1624u) & 0x80) == 0)
        ns = 1;
    if (s8(AR(114) + 1) == 1)
        for (i = 1; i <= 9 && !sp; i++)
            sp = s8(AR(114) + 1u + (uint32_t)i) != 0 && s8(AR(114) + 0x0Eu + (uint32_t)i) >= 6 &&
                 b8(NAT(i) + 0x27) != 0;
    for (n = 1; n <= 9; n++) {
        rec = NAT(n);
        w32(0xF40F8u, n);
        if (b8(rec + 0x9A) == 0 || s8(rec + 0x51) >= 9)
            continue;
        arch = s8(rec + 0x31);
        m334F4(arch, n);
        if (s32(LIST_N) == 0)
            continue;
        none = 0;
        if (!ns && !sp && s8(rec + 0x51) + s32(LIST_N) > 9)
            ns = 1;
        if (ns || sp)
            w16(0xF408Eu, 1);
        if (s8(rec + 0x9A) == 3) {
            if ((b8(0xE1624u) & 0x80) && (ns || sp) && s8(0xE1622u) != n)
                continue;
            if (ns || sp) {
                m_screen(n, arch, &mx, &mn);
                continue;
            }
            for (i = 0; i < s32(LIST_N); i++) {
                int t = s32(LIST + 4u * (uint32_t)i);
                m_inline_city(i, n, s32(NEEDS + 4u * (uint32_t)i));
                snprintf(b, sizeof b, "%s builds a city in %s", name_of(n), tname(t));
                f79270((b8(0xF0980u) & 1) ? -500 : 2000, n, t, b, 0);
            }
            continue;
        }
        if (s8(rec + 0x9A) == 2)
            m_computer(n);
    }
    if (none)
        f79270(100, s16(0xF06B0u), -200, "", 0);
}

/* 0x5C528 */
static void m5C528(int k, int t, int n, int show)
{
    char b[100];
    w8(AR(t + 1) + 0x0Eu + (uint32_t)n, b8(AR(t + 1) + 0x0Eu + (uint32_t)n) - k);
    f581C4(t, n);
    w8(NAT(n) + 0x98, b8(NAT(n) + 0x98) - k);
    w8(NAT(n) + 0x99, b8(NAT(n) + 0x99) + k);
    if (!show)
        return;
    snprintf(b, sizeof b, "%s - %d token%s removed from %s", name_of(n), k, k >= 2 ? "s" : "", tname(t));
    f79270(f7BA64() == 0 ? 1000 : (s16(0xF40BAu) != 0 ? 1500 : 500), n, t, b, 0);
}

/* 0x5C238 */
static void m5C238(int show)
{
    int none = 1, n, t, ag, lim, k;
    if (show) {
        memset(B + (0xF5934u - DS_BASE), 0, 12);
        m_players();
        f783B8(0xF596Cu, s32(0xF5940u));
        m_mapscreen();
        f7876C();
        f3C720(0xF596Cu);
    }
    for (n = 1; n <= 9; n++) {
        ag = b8(NAT(n) + 0x27) ? 1 : 0;
        for (t = 0; t < 157; t++) {
            if (!show && (b8(TS(t) + 2) & 2)) {
                m5C528(tok(t, n), t, n, show);
            } else if (b8(TS(t) + 2) & 1) {
                if (tok(t, n) > 0) {
                    k = tok(t, n);
                    w8(0xF5934u + (uint32_t)n, b8(0xF5934u + (uint32_t)n) + k);
                    m5C528(k, t, n, show);
                    none = 0;
                }
            } else {
                lim = ((b8(TF(t)) >> 3) & 7) + ag;
                if (tok(t, n) > lim) {
                    m5C528(tok(t, n) - lim, t, n, show);
                    none = 0;
                }
            }
        }
    }
    if (show)
        f3C6A0(0xF596Cu);
    if (none && show)
        f79270(100, s16(0xF06B0u), -200, "", 0);
}

/* 0x5C880 */
static int m5C880(const int32_t *list, int cnt, const char *msg, int mode)
{
    char b[200];
    int t, i, sel = 0, redraw = 1, owner = 0, r, empty;

    w32(LIST_N, 0);
    for (t = 0; t < 157; t++)
        w32(LIST + 4u * (uint32_t)t, -1);
    for (t = 0; mode == 1 && t < 157; t++)
        for (i = 0; i < cnt; i++)
            if (b8(TS(t)) == list[i] && (b8(TS(t) + 2) & 1) && b8(TS(t) + 1) == s16(0xF40FCu))
                m_add_t(t);
    if (s32(LIST_N) != 0)
        snprintf(b, sizeof b, "%s First using cities built this round.", msg);
    else
        snprintf(b, sizeof b, "%s", msg);
    if (s32(LIST_N) == 0)
        for (t = 0; t < 157; t++)
            for (i = 0; i < cnt; i++)
                if (b8(TS(t)) == list[i] && (b8(TS(t) + 2) & 1))
                    m_add_t(t);
    f7853C(0xC5DD8u);
    m_mapscreen();
    fg_vbopen(s32(0xF52C8u));
    f3AF8C(0xF51ACu, 251, 385);
    f3AF8C(0xF51B4u, 376, 385);
    fg_setcolor(5);
    fg_vbclose();
    f3A924();
    f3A9F4(b);
    f7876C();
    f3B05C(0xF596Cu);
    for (;;) {
        empty = !g_original && s32(LIST_N) <= 0;
        t = s32(LIST + 4u * (uint32_t)sel);
        fg_vbopen(s32(0xF52C8u));
        f3C9D0(265, 385, 110, 19, 5);
        fg_setcolor(5);
        if (!empty)
            f3A0B0(265, 389, 110, tname(t));
        fg_vbclose();
        owner = empty ? 0 : b8(TS(t));
        f3F670();
        if (!empty)
            f3F624(t, 15);
        if (redraw) {
            f459E4(owner);
            if (empty)
                f3D52C(0, 0);
            else
                f3DA60(t);
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
        if (r == -9) {
            if (sel > 0) {
                sel--;
                redraw = 1;
            }
        } else if (r == -10) {
            if (sel < s32(LIST_N) - 1) {
                sel++;
                redraw = 1;
            }
        } else if (r == -14) {
            if (empty)   /* fix 3: the screen has no FINISHED, so REDUCE CITY is its only way out */
                break;
            f78FEC(t, owner);
            f3D52C(0, 0);
            for (i = sel; i < s32(LIST_N) - 1; i++)
                w32(LIST + 4u * (uint32_t)i, s32(LIST + 4u * (uint32_t)(i + 1)));
            w32(LIST_N, s32(LIST_N) - 1);
            {
                int32_t s = sel;
                f79A9C(&s);
                sel = s;
            }
            fg_vbopen(s32(0xF52C8u));
            f459E4(owner);
            f3CBBC();
            break;
        }
    }
    f78664(0xC5DD8u);
    return owner;
}

/* 0x5C750 */
static void m5C750(int n, int loss, const char *msg)
{
    uint32_t rec = NAT(n);
    char b[152];
    int32_t one = n;
    if (b8(rec + 0x9A) == 0)
        return;
    while (s8(rec + 0x98) - loss < 2 * s8(rec + 0x51)) {
        if (!g_original && s8(rec + 0x51) <= 0)
            return;
        w16(0x109E20u, 1);
        if ((b8(0xE1624u) & 0x80) ? b8(0xF4102u) != 15 : s8(rec + 0x9A) == 3) {
            if ((b8(0xE1624u) & 0x80) && s8(0xE1622u) != n)
                return;
            f7844C(0xF596Cu, s32(0xF5940u));
            snprintf(b, sizeof b, "%s You must reduce your cities.", msg);
            m5C880(&one, 1, b, 1);
        } else {
            w8(rec + 0x98, b8(rec + 0x98) - loss);
            city_test_f5DE5C(n, msg);
            w8(rec + 0x98, b8(rec + 0x98) + loss);
        }
    }
}

/* 0x5C62C */
static void m5C62C(void)
{
    char b[100];
    int n, v;
    m_players();
    f783B8(0xF596Cu, s32(0xF5940u));
    for (n = 1; n <= 9; n++) {
        w32(0xF40F8u, n);
        v = s16(0xF7AFEu) == n;
        snprintf(b, sizeof b, v ? "City Support Check with Slave revolt losses for %s." : "City Support Check for %s.",
                 name_of(n));
        m5C750(n, v ? s16(0xF7AFCu) : 0, b);
    }
    if (s16(0x109E20u) == 0)
        f79270(100, s16(0xF06B0u), -200, "", 0);
}

/* ---- running both sides ---------------------------------------------------------------------------------------- */

enum { R_334F4, R_32214, R_5C238, R_5C62C, R_5C880 };

static int g_checks, g_fail;
static uint32_t g_rng = 12345u;

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

static int32_t g_arg_list[9];
static int g_arg_cnt, g_arg_mode, g_arg1, g_arg2, g_ret_real, g_ret_model;

static void call(int which, int model)
{
    switch (which) {
    case R_334F4: if (model) m334F4(g_arg1, g_arg2); else f334F4(g_arg1, g_arg2); break;
    case R_32214: if (model) m32214(); else f32214(); break;
    case R_5C238: if (model) m5C238(g_arg1); else f5C238(g_arg1); break;
    case R_5C62C: if (model) m5C62C(); else f5C62C(); break;
    case R_5C880:
        if (model)
            g_ret_model = m5C880(g_arg_list, g_arg_cnt, "Reduce test.", g_arg_mode);
        else
            g_ret_real = f5C880(g_arg_list, g_arg_cnt, "Reduce test.", g_arg_mode);
        break;
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
    g_en3B = -1;
    g_screen = 0;
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
    if (which == R_5C880 && g_ret_real != g_ret_model) {
        printf("FAIL %s: return %d / %d\n", what, g_ret_real, g_ret_model);
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
        for (n = 0; n <= 12; n++) {
            w8(a + 1u + (uint32_t)n, 0);
            w8(a + 0x0Eu + (uint32_t)n, 0);
        }
        w8(a + 1, 0);
        w8(TS(t), 0);
        w8(TS(t) + 1, 0xFF);
        w8(TS(t) + 2, 0);
    }
    for (n = 1; n <= 9; n++) {
        uint32_t rec = NAT(n);
        w8(rec + 0x51, 0);
        w8(rec + 0x60, 0);
        memset(B + (rec + 0x52 - DS_BASE), 0, 9);
        memset(B + (rec + 0x61 - DS_BASE), 0, 0x60);
    }
}

/* a random game state on top of the current g_ds: players, advances, treasuries, territories, tokens, cities */
static void random_state(int human_pct, int comp_pct, int fresh_map)
{
    int n, t, k, m, round = 1 + (int)rnd(12);
    B = g_ds;
    w16(0xF40FCu, round);
    if (fresh_map)
        for (t = 0; t < 157; t++)
            w8(TF(t), (b8(TF(t)) & 0x87) | (rnd(3) == 0 ? 0x40 : 0) | (int)rnd(6) << 3);
    clear_board();
    for (n = 1; n <= 9; n++) {
        uint32_t rec = NAT(n);
        k = (int)rnd(100);
        w8(rec + 0x9A, k < human_pct ? 3 : k < human_pct + comp_pct ? 2 : 0);
        w8(rec + 0x27, rnd(3) == 0);
        w8(rec + 0x31, rnd(3) == 0);
        w8(rec + 0x3E, (int)rnd(rnd(4) == 0 ? 40 : 12));
        w8(rec + 0x98, (int)rnd(60));
        w8(rec + 0x99, (int)rnd(55));
    }
    for (t = 0; t < 157; t++) {
        if (rnd(10) == 0) {
            m = 1 + (int)rnd(9);
            put_city(t, m, rnd(2) ? round : (int)rnd(16));
            if (rnd(3) == 0)
                put_tokens(t, 1 + (int)rnd(9), 1 + (int)rnd(4));
        } else if (rnd(5) < 2) {
            k = rnd(4) == 0 ? 2 : 1;
            while (k-- > 0)
                put_tokens(t, 1 + (int)rnd(9), (int)rnd(15));
        }
        if (rnd(12) == 0)
            w8(TS(t) + 2, b8(TS(t) + 2) | 2);
    }
    for (n = 0; n < 9; n++)
        for (t = 0; t < 157; t++)
            w16(0xF3580u + 314u * (uint32_t)n + 2u * (uint32_t)t, rnd(2) ? 0x80 : (int)rnd(0x80));
    w8(0xE1624u, (b8(0xE1624u) & 0x7F) | (rnd(6) == 0 ? 0x80 : 0));
    if ((b8(0xE1624u) & 0x80) != 0 && rnd(3) != 0)   /* PBEM without Architecture: the automatic build, FAYUM */
        for (n = 1; n <= 9; n++)
            w8(NAT(n) + 0x31, 0);
    w8(0xE1622u, 1 + (int)rnd(9));
    w8(0xF4102u, rnd(2) ? 10 : 15);
    w8(0xF0980u, (b8(0xF0980u) & ~1) | (rnd(4) == 0));
    w16(0xF40B2u, rnd(10) == 0);
    w16(0xF40BAu, (int)rnd(2));
    w16(0xF408Eu, 0);
    w16(0x109E20u, 0);
    w16(0xF7AFEu, rnd(4) == 0 ? 1 + (int)rnd(9) : 0);
    w16(0xF7AFCu, (int)rnd(20));
    if (rnd(6) == 0) {   /* FAYUM (territory 113), the special case */
        n = 1 + (int)rnd(9);
        put_tokens(113, n, 4 + (int)rnd(6));
        w8(NAT(n) + 0x27, 1);
    }
    g_unatt = rnd(5) == 0;
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

static int list_has(int t, int need)
{
    int i;
    for (i = 0; i < S32(LIST_N); i++)
        if (S32(LIST + 4u * (uint32_t)i) == t)
            return S32(NEEDS + 4u * (uint32_t)i) == need;
    return need == 0;
}

static void set_site(int t, int site_, int lim)
{
    setD8(TF(t), (uint8_t)((D8(TF(t)) & 0x87) | (site_ ? 0x40 : 0) | lim << 3));
}

static void base_clean(void)
{
    int n;
    memcpy(g_ds, g_pristine, DS_SIZE);
    B = g_ds;
    clear_board();
    for (n = 1; n <= 9; n++) {
        w8(NAT(n) + 0x9A, 0);
        w8(NAT(n) + 0x27, 0);
        w8(NAT(n) + 0x31, 0);
        w8(NAT(n) + 0x3E, 0);
    }
    w16(0xF40FCu, 3);
    w16(0xF40B2u, 0);
    w8(0xE1624u, b8(0xE1624u) & 0x7F);
    w8(0xF0980u, b8(0xF0980u) & ~1);
    g_unatt = 0;
    g_nforced = 0;
}

static void part_a(void)
{
    int f0 = g_fail;

    /* A1 0x334F4 */
    base_clean();
    set_site(10, 1, 3); put_tokens(10, 1, 6);
    set_site(11, 0, 3); put_tokens(11, 1, 12);
    set_site(12, 1, 3); put_tokens(12, 1, 5);
    set_site(13, 0, 3); put_tokens(13, 1, 11);
    set_site(14, 1, 3); put_tokens(14, 1, 2);
    set_site(15, 1, 3); put_tokens(15, 1, 5); put_tokens(15, 2, 1);
    set_site(16, 1, 3); put_tokens(16, 1, 9); put_city(16, 3, 1);
    w8(NAT(1) + 0x3E, 3);
    g_arg1 = 0; g_arg2 = 1;
    run_both(R_334F4, 1, "A1 no arch");
    expect(S32(LIST_N) == 2 && list_has(10, 6) && list_has(11, 12) && S32(LIST + 8) == -1 && S32(LIST + 4 * 156) == -1,
           "A1 costs 6 / 12");
    g_arg1 = 1;
    run_both(R_334F4, 1, "A1 arch");
    expect(S32(LIST_N) == 4 && list_has(12, 6) && list_has(13, 12) && list_has(14, 0) && list_has(15, 0),
           "A1 Architecture 3 + 3, 6 + 6; not with two nations");
    setD16(0xF40B2u, 1);
    g_arg1 = 0;
    run_both(R_334F4, 1, "A1 switch");
    expect(list_has(12, 1) && list_has(13, 2) && list_has(14, 1) && list_has(15, 1) && list_has(16, 0),
           "A1 dead switch: 1 / 2");
    g_arg1 = 1;
    run_both(R_334F4, 1, "A1 switch arch");
    expect(list_has(12, 6) && list_has(13, 12) && list_has(15, 1), "A1 dead switch: Architecture still 6 / 12");

    /* A2 0x5C238 */
    base_clean();
    w8(NAT(2) + 0x9A, 3); w8(NAT(3) + 0x9A, 3);
    put_city(20, 3, 3); put_tokens(20, 2, 3); set_site(20, 1, 2);
    set_site(21, 0, 2); put_tokens(21, 2, 4);
    set_site(22, 0, 2); put_tokens(22, 3, 4); w8(NAT(3) + 0x27, 1);
    w8(NAT(2) + 0x98, 20); w8(NAT(2) + 0x99, 5);
    g_arg1 = 1;
    run_both(R_5C238, 2, "A2");
    expect(S8(AR(21) + 0x0E + 2) == 0 && D8(0xF5934u + 2) == 3, "A2 city territory emptied, 0xF5934[2] = 3");
    expect(S8(AR(22) + 0x0E + 2) == 2 && S8(AR(23) + 0x0E + 3) == 3, "A2 limit 2; Agriculture 3");
    expect(D8(NAT(2) + 0x98) == 15 && D8(NAT(2) + 0x99) == 10, "A2 board 20 - 5, stock 5 + 5");
    expect(strstr(g_real, " - 3 tokens removed from ") != NULL && strstr(g_real, " - 1 token removed from ") != NULL,
           "A2 token / tokens");

    /* A3 the human screen with Architecture: treasury 5, site with 4 tokens alone: need 6, min 2, max 3 */
    {
        static const int clicks[] = { -12, -12, -11, -11, -12, -13 };
        base_clean();
        w8(NAT(1) + 0x9A, 3);
        w8(NAT(1) + 0x31, 1);
        w8(NAT(1) + 0x3E, 5);
        w8(NAT(1) + 0x98, 4);
        w8(NAT(1) + 0x99, 0);
        set_site(30, 1, 3); put_tokens(30, 1, 4);
        memcpy(g_forced, clicks, sizeof clicks);
        g_nforced = 6;
        run_both(R_32214, 3, "A3");
        expect(S8(NAT(1) + 0x3E) == 2 && S8(AR(31) + 0x0E + 1) == 1 && S8(NAT(1) + 0x51) == 1 && D8(TS(30)) == 1 &&
               D8(TS(30) + 1) == 3 && S8(NAT(1) + 0x98) == 1 && S8(NAT(1) + 0x99) == 6,
               "A3 contribution 3: treasury 2, tokens 1, stock + 6");
        expect(strstr(g_real, "text 310 389 19 3\nvbclose") != NULL && strstr(g_real, "text 310 389 19 2\n") != NULL &&
               strstr(g_real, "color 23\ntext 310 389 19 3\nright 293 389 TREASURY") != NULL,
               "A3 contribution shown 2..3, greyed after use");
        /* contribution 0: Architecture kept for the next city */
        base_clean();
        w8(NAT(1) + 0x9A, 3);
        w8(NAT(1) + 0x31, 1);
        w8(NAT(1) + 0x3E, 5);
        set_site(30, 1, 3); put_tokens(30, 1, 6);
        set_site(32, 1, 3); put_tokens(32, 1, 4);
        g_forced[0] = -11; g_forced[1] = -11; g_forced[2] = -13; g_forced[3] = -12; g_forced[4] = -13;
        g_nforced = 5;
        run_both(R_32214, 4, "A3b");
        expect(S8(NAT(1) + 0x51) == 2 && S8(NAT(1) + 0x3E) == 2 && S8(AR(33) + 0x0E + 1) == 1,
               "A3b first city without the treasury, the second with 3");
        g_nforced = 0;
    }

    /* A4 the computer's reorder (8 cities, Architecture): fixed [60, 50], --original [50, 60] */
    for (g_original = 0; g_original <= 1; g_original++) {
        int i;
        base_clean();
        w8(NAT(2) + 0x9A, 2);
        w8(NAT(2) + 0x31, 1);
        w8(NAT(2) + 0x3E, 9);
        w8(NAT(2) + 0x98, 30);
        for (i = 0; i < 8; i++)
            put_city(100 + i, 2, 1);
        set_site(50, 1, 3); put_tokens(50, 2, 3);
        set_site(60, 1, 3); put_tokens(60, 2, 6);
        put_city(49, 5, 1); put_tokens(49, 2, 7);
        run_both(R_32214, 5, g_original ? "A4 original" : "A4");
        expect(g_original ? (S32(LIST) == 50 && S32(LIST + 4) == -1) : (S32(LIST) == -1 && S32(LIST + 4) == 50),
               g_original ? "A4 --original orders by territory s - 1" : "A4 fixed order");
        expect(strstr(g_real, "333E8 60 2 6 0\n") != NULL, "A4 builds 60");
    }
    g_original = 0;

    /* A5 0x5C750 with no city and board < loss */
    for (g_original = 0; g_original <= 1; g_original++) {
        base_clean();
        w8(NAT(3) + 0x9A, 3);
        w8(NAT(3) + 0x98, 5);
        w16(0xF7AFEu, 3);
        w16(0xF7AFCu, 10);
        w16(0x109E20u, 0);
        run_both(R_5C62C, 6, "A5");
        expect(g_original ? (strstr(g_real, "CUT") != NULL && S16(0x109E20u) == 1)
                          : (strstr(g_real, "CUT") == NULL && S16(0x109E20u) == 0 &&
                             strstr(g_real, "msg 100 3 -200  0") != NULL),
               g_original ? "A5 --original never ends" : "A5 fixed: returns, the empty message");
    }
    g_original = 0;

    /* A6 0x5C880 with an empty list */
    for (g_original = 0; g_original <= 1; g_original++) {
        base_clean();
        w8(0xFFB5Fu, 0x80);
        g_arg_list[0] = 4;
        g_arg_cnt = 1;
        g_arg_mode = 1;
        g_forced[0] = -14;
        g_nforced = 1;
        run_both(R_5C880, 7, "A6");
        expect(D8(0xFFB5Fu) == (g_original ? 15 : 0x80), g_original ? "A6 --original writes 0xFFB5F" :
                                                                       "A6 fixed: 0xFFB5F kept");
        expect((strstr(g_real, "text 265 389 110 PIRACY\n") != NULL) == (g_original != 0), "A6 the PIRACY box");
        /* dialog 0xC5DD8 has no FINISHED: REDUCE CITY alone must leave the screen (no -1 click asked for) */
        expect(strstr(g_real, "click F596C 1 -14\n") != NULL && strstr(g_real, "click F596C 1 -1\n") == NULL,
               "A6 REDUCE CITY leaves the empty screen");
        expect((strstr(g_real, "78FEC ") != NULL) == (g_original != 0), "A6 f78FEC only under --original");
        g_nforced = 0;
    }
    g_original = 0;

    /* A7 0x5C880 mode 1: only this round's cities, with the prefix */
    base_clean();
    put_city(70, 4, 3); put_city(71, 4, 2); put_city(72, 5, 3);
    g_arg_list[0] = 4; g_arg_list[1] = 5;
    g_arg_cnt = 2;
    g_arg_mode = 1;
    g_forced[0] = -10; g_forced[1] = -14;
    g_nforced = 2;
    run_both(R_5C880, 8, "A7");
    expect(strstr(g_real, "left Reduce test. First using cities built this round.\n") != NULL &&
           strstr(g_real, "78FEC 72 5\n") != NULL && g_ret_real == 5 && (D8(TS(70) + 2) & 1) == 1,
           "A7 this round's cities first; city 72 of nation 5 reduced");
    g_nforced = 0;
    printf("A: hand vectors, %d failed\n", g_fail - f0);
}

/* ---- B..F ------------------------------------------------------------------------------------------------------- */

static void part_b(void)
{
    int i, f0 = g_fail;
    char what[64];
    for (i = 0; i < 30000; i++) {
        random_state(60, 30, 1);
        g_arg1 = (int)rnd(2);
        g_arg2 = 1 + (int)rnd(9);
        snprintf(what, sizeof what, "B%d", i);
        run_both(R_334F4, (uint32_t)i, what);
    }
    printf("B: 0x334F4, 30000 runs, %d failed\n", g_fail - f0);
}

static void part_c(void)
{
    int i, f0 = g_fail;
    char what[64];
    for (i = 0; i < 6000; i++) {
        random_state(60, 30, 1);
        g_arg1 = (int)rnd(2);
        snprintf(what, sizeof what, "C%d", i);
        run_both(R_5C238, 100000u + (uint32_t)i, what);
    }
    printf("C: 0x5C238, 6000 runs, %d failed\n", g_fail - f0);
}

static void part_d(void)
{
    int i, j, f0 = g_fail, np0 = g_np, cut0 = g_cuts;
    char what[64];
    memset(g_cov, 0, sizeof g_cov);
    for (i = 0; i < 4000; i++) {
        random_state(80, 15, 1);
        g_original = (int)rnd(2);
        snprintf(what, sizeof what, "D%d%s", i, g_original ? " original" : "");
        run_both(R_5C62C, 200000u + (uint32_t)i, what);
    }
    for (i = 0; i < 3000; i++) {
        random_state(80, 15, 1);
        g_original = (int)rnd(2);
        g_arg_cnt = 1 + (int)rnd(3);
        for (j = 0; j < g_arg_cnt; j++)
            g_arg_list[j] = (int)rnd(10);
        g_arg_mode = (int)rnd(2);
        snprintf(what, sizeof what, "D5C880 %d%s", i, g_original ? " original" : "");
        run_both(R_5C880, 300000u + (uint32_t)i, what);
    }
    g_original = 0;
    printf("D: 0x5C62C 4000 runs, 0x5C880 3000 runs, %d failed (reductions %ld, 0x5DE5C %ld, cut %d)\n",
           g_fail - f0, g_cov[4], g_cov[5], g_cuts - cut0);
    (void)np0;
}

static void part_e(void)
{
    int i, f0 = g_fail, cut0 = g_cuts;
    char what[64];
    memset(g_cov, 0, sizeof g_cov);
    for (i = 0; i < 8000; i++) {
        random_state(55, 40, 1);
        g_original = (int)rnd(2);
        snprintf(what, sizeof what, "E%d%s", i, g_original ? " original" : "");
        run_both(R_32214, 400000u + (uint32_t)i, what);
    }
    g_original = 0;
    printf("E: 0x32214, 8000 runs, %d failed (human builds %ld, automatic %ld, 0x333E8 %ld, with Architecture %ld, "
           "cut %d)\n", g_fail - f0, g_cov[0], g_cov[1], g_cov[2], g_cov[3], g_cuts - cut0);
}

static void part_f(const char *savedir, const char *gamedir)
{
    static const char *const SAVES[4] = { "SAVGAMEA.DAT", "savgameb.dat", "savgame0.dat", "savgame1.dat" };
    uint8_t *base = malloc(DS_SIZE);
    char path[1024], what[64];
    int s, i, n, f0 = g_fail, which;

    for (s = 0; s < 4 && base != NULL; s++) {
        snprintf(path, sizeof path, "%s/%s", s < 2 ? savedir : gamedir, SAVES[s]);
        if (!load_save(path)) {
            printf("FAIL F: cannot read %s\n", path);
            g_fail++;
            continue;
        }
        memcpy(base, g_ds, DS_SIZE);
        B = g_ds;
        printf("F: %s round %d, players", SAVES[s], S16(0xF40FCu));
        for (n = 1; n <= 9; n++)
            printf(" %d", D8(NAT(n) + 0x9A));
        printf(", cities");
        for (n = 1; n <= 9; n++)
            printf(" %d", S8(NAT(n) + 0x51));
        printf("\n");
        for (i = 0; i < 600; i++) {
            memcpy(g_ds, base, DS_SIZE);
            B = g_ds;
            which = i % 3 == 0 ? R_32214 : i % 3 == 1 ? R_5C238 : R_5C62C;
            if (i >= 3)   /* now and then Architecture, Agriculture, a Slave Revolt loss, a short board */
                for (n = 1; n <= 9; n++) {
                    if (rnd(4) == 0) w8(NAT(n) + 0x31, 1);
                    if (rnd(4) == 0) w8(NAT(n) + 0x27, 1);
                    if (rnd(6) == 0) w8(NAT(n) + 0x98, (int)rnd(10));
                    if (rnd(8) == 0) { w16(0xF7AFEu, n); w16(0xF7AFCu, (int)rnd(15)); }
                }
            w16(0x109E20u, 0);
            g_arg1 = 1;
            g_unatt = 0;
            g_original = i & 1;
            snprintf(what, sizeof what, "F %s %d", SAVES[s], i);
            run_both(which, 500000u + (uint32_t)i, what);
        }
    }
    g_original = 0;
    free(base);
    printf("F: the real saves, 2400 runs, %d failed\n", g_fail - f0);
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:\\work\\advciv_run";
    const char *savedir = argc > 2 ? argv[2] : "C:\\work\\advciv_play";

    if (state_init(gamedir) != 0) {
        fprintf(stderr, "cannot load %s/AC_MAIN.EXE\n", gamedir);
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
    part_d();
    part_e();
    part_f(savedir, gamedir);

    printf("%d checks, %d failed\n", g_checks, g_fail);
    return g_fail == 0 ? 0 : 1;
}
