/* cal1_test [GAMEDIR [SAVEDIR]]: unit checks for module CAL1 (cal1.c) that need no game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init; default C:\work\advciv_run, only read) and the saves
   savgame0.dat (start of MOVEMENT R4) and savgame1.dat (R4, all human); SAVEDIR holds SAVGAMEA.DAT and savgameb.dat
   (default C:\work\advciv_play, only read). All four are read with saveload.c's f5F05C as the game does.

   cal1.c is built with -DCAL1_TEST: its calls into code that is not ported yet (the computers' 0x213C0, 0x20C70,
   0x24EB8, 0x252BC, 0x27518; ANIM's 0x7A4A0 / 0x7A4C4; CAL2's 0x291D0 / 0x29550; the other calamities' handlers) come
   to recorders here, which trace the call, act out a plausible effect where the caller depends on one (0x213C0 /
   0x20C70 return a city of the nation, 0x27518 / 0x29550 fill the losses 0xF547C) and return.

   Every other routine cal1.c calls is a stub below that appends one line to a trace (drawing included); those with
   data effects apply them on the data object g_ds (rules.c's city and card edits, f57E28's tokens, the borders, the
   clamp, a human's REDUCE CITY as the first city of the nation reduced). Clicks (f78BA4) and the YES/NO answers come
   from a script: a seeded random choice among the codes the screen offers (enabled buttons, map clicks on listed
   entries, the arrows, -100, codes that do nothing), the same seed for the port and the model; a run that clicks more
   than 3000 times is cut off ("CUT") identically on both sides. game_rand is the real generator on the seed 0xCAC5C,
   so every draw shows in the data object.

   The model is a second implementation of the sixteen routines, written from the disassembly (not from cal1.c): it
   runs on a copy of the data object (g_ds pointed at the copy), calls the same stubs, and so writes the trace it
   expects. After each run the two traces must be equal and the whole data object must equal the model's copy.
     A. hand vectors (expected values written out): 0x1E340, 0x1E5B8's order, Zeus's discards with the givers
        (fixed: aligned; --original: shifted only in the ids), 0x1E69C, Superstition's counts and texts, Slave Revolt's
        loss, Civil Disorder's count, Iconoclasm's secondary counts, Treachery's article ("an African", "a Thracian";
        --original "a African"), the Skull & Crossbones message, 0x2468C mode 1 with one secondary nation (fixed: left
        0, FINISHED; --original: REMOVE on entry -1), RESTART's marks, 0x28CFC's kinds, the three text fixes.
     B. 0x28CFC on 20,000 random states (every kind); C. Zeus, 0x1E5B8, 0x1E69C on 10,000;
     D. the six calamities on 3,000 random states each (humans, computers, PBEM, toggles); E. 0x2468C alone on 6,000
        (modes 0 / 1 / 2, restarts); F. 0x1E0A0 on 4,000 random hands; G. 0x1E0A0 on the four real saves with injected
        calamities, 300 runs each. Every random part runs with and without --original.
   Prints each failure (the first differing trace line or byte), the counts per part, and exits 0 when all pass.

   Build: clang -std=c11 -DCAL1_TEST port/test/cal1_test.c port/src/cal1.c port/src/saveload.c port/src/state.c
          port/src/le_image.c -Iport/src -o cal1_test.exe */

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cal1.h"
#include "city.h"
#include "fg.h"
#include "game.h"
#include "kit.h"
#include "loop.h"
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
#define CCOUNT      0xF7808u
#define MARK        0xF7AC4u
#define SECC        0xF796Cu
#define SECL        0xF547Cu

/* ---- byte access on the current data object (g_ds: the port's or the model's copy) ---------------------------- */

static uint8_t *R;          /* the port's data object (state_init's) */
static uint8_t *M;          /* the model's copy */
static uint8_t *g_pristine;

static int      rb(uint32_t a) { return g_ds[a - DS_BASE]; }
static int      sb(uint32_t a) { return (int8_t)g_ds[a - DS_BASE]; }
static int      s16(uint32_t a) { return (int16_t)(g_ds[a - DS_BASE] | g_ds[a + 1 - DS_BASE] << 8); }
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
static const char *adj(int n) { return str((uint32_t)s32(0xC76C4u + 4u * (uint32_t)n)); }
static const char *tn(int t) { return str((uint32_t)s32(0xC77D8u + 4u * (uint32_t)t)); }
static const char *cn(int c) { return str((uint32_t)s32(0xC775Cu + 4u * (uint32_t)c)); }

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
static int g_steps, g_en2A, g_en3F, g_np, g_cuts;
static uint32_t g_screen;
static jmp_buf g_jb;
static int g_forced[64], g_nforced, g_iforced;   /* hand vectors: these clicks first, then the script */
static int g_yes = -1;                           /* hand vectors: the YES/NO answer (-1 = the script) */
static long g_cov[8];   /* port side: 2468C REMOVE, RESTART, 207B4 picks, f5C880, 213C0, 2468C empty-left fixes,
                           Zeus discards, handlers stopped */

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
        case 3: return -9;
        case 4: return -10;
        case 5: if (g_screen == 0xBDE30u) return -16; return -0x63;
        case 6: case 7: case 8: if (g_screen == 0xBEE78u && g_en3F == 1) return -0x65; break;
        case 9: if (g_screen == 0xBEE78u && g_en2A == 1) return -1; if (g_screen == 0xBDE30u) return -16; break;
        case 10: return -5;
        case 11: return g_screen == 0xBDE30u ? -1 : -0x64;
        case 12: return -0x66;
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
    if (c >= 0 && c < 0x40) {
        w8(rec + 0x52u + (uint32_t)c, t + 1);
        w8(rec + 0x51, c + 1);
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
}

static void eff_78FEC(int t, int n)
{
    uint32_t rec = NAT(n);
    int lim, k;
    if (!okt(t) || !okn(n))
        return;
    eff_78F04(t, n);
    lim = (rb(TR1(t)) >> 3) & 7;
    k = sb(rec + 0x99) < lim ? sb(rec + 0x99) : lim;
    if (k <= 0)
        return;
    eff_57E28(t, n, k);
    w8(rec + 0x99, sb(rec + 0x99) - k);
    w8(rec + 0x98, sb(rec + 0x98) + k);
}

static void eff_771F8(int c)
{
    int s = c <= 18 ? (c - 1) / 2 : sb(0xCA9C1u + (uint32_t)c);
    uint32_t p;
    int cnt;
    if (c < 1 || c > 0x1E || s < 0 || s > 8)
        return;
    p = 0xF07B4u + 25u * (uint32_t)s;
    cnt = s16(p);
    if (cnt < 0 || cnt >= 19)
        return;
    w8(p + 6u + (uint32_t)cnt, c);
    w16(p, cnt + 1);
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
void f783B8(uint32_t dlg, int base) { tr("783B8 %X %d", (unsigned)dlg, base); }
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
}
void f3C314(uint32_t dlg, int id)
{
    tr("on %X %X", (unsigned)dlg, (unsigned)id);
    if (id == 0x2A) g_en2A = 1;
    if (id == 0x3F) g_en3F = 1;
}
void f3C284(uint32_t dlg, int id, uint32_t label) { tr("label %X %X %s", (unsigned)dlg, (unsigned)id, str(label)); }
void f3C720(uint32_t dlg) { tr("alloff %X", (unsigned)dlg); }
void f3C6A0(uint32_t dlg) { tr("allon %X", (unsigned)dlg); }
void f3DA60(int t) { tr("centre %d", t); }
void f3D52C(int dx, int dy) { tr("scroll %d %d", dx, dy); }
void f553C0(void) { tr("music"); }
void f7C960(void) { tr("scores"); }
void f22B00(void) { tr("elim"); }
void f5E7B4(void) { tr("hook"); }
void f78F04(int t, int n) { tr("78F04 %d %d", t, n); eff_78F04(t, n); }
void f78E74(int t, int n) { tr("78E74 %d %d", t, n); eff_78E74(t, n); }
void f78FEC(int t, int n) { tr("78FEC %d %d", t, n); eff_78FEC(t, n); }
void f57E28(int t, int n, int k) { tr("57E28 %d %d %d", t, n, k); eff_57E28(t, n, k); }
void f771F8(int c) { tr("discard %d", c); eff_771F8(c); }
int  f789EC(int n) { tr("article %d", n); return !(n == 4 || n == 5 || n == 8); }
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
int f5C880(const int32_t *list, int cnt, const char *msg, int mode)
{
    int n = list[0], j;
    tr("reduce %d %d %s %d", n, cnt, msg, mode);
    if (g_ds == R)
        g_cov[3]++;
    cut_check();
    if (okn(n) && sb(NAT(n) + 0x51) > 0) {
        j = rb(NAT(n) + 0x52) - 1;
        eff_78FEC(j, n);
    }
    return n;
}
void f5C750(int n, int loss, const char *msg) { tr("support %d %d [%s]", n, loss, msg); }

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
        if (r == -0x65 && g_screen == 0xBEE78u) g_cov[0]++;
        if (r == -0x63) g_cov[1]++;
        if (r == -16 && g_screen == 0xBDE30u) g_cov[2]++;
    }
    return r;
}

/* the recorders of the calls that are not ported (cal1.c with -DCAL1_TEST) */
int cal1_test_f213C0(int n)
{
    int r = okn(n) && sb(NAT(n) + 0x51) > 0 ? rb(NAT(n) + 0x52) : 0;
    tr("213C0 %d -> %d", n, r);
    if (g_ds == R)
        g_cov[4]++;
    return r;
}
int cal1_test_f20C70(int p, int v)
{
    int c = okn(v) ? sb(NAT(v) + 0x51) : 0;
    int r = c > 0 ? rb(NAT(v) + 0x52u + (uint32_t)(c - 1)) : 0;
    tr("20C70 %d %d -> %d", p, v, r);
    return r;
}
void cal1_test_f24EB8(int c, int v, int k) { tr("24EB8 %d %d %d", c, v, k); }
void cal1_test_f252BC(int c, int v, int k) { tr("252BC %d %d %d", c, v, k); }
void cal1_test_f27518(int n, int trader, int total, int kind)
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
void cal1_test_f7A4A0(void) { tr("7A4A0"); }
void cal1_test_f7A4C4(int anim) { tr("7A4C4 %d", anim); }
void cal1_test_f291D0(int a, int b, const char *text) { tr("291D0 %d %d %s", a, b, text); }
void cal1_test_f29550(int a, int b, int total, const char *text)
{
    int m, rest = total, x;
    tr("29550 %d %d %d %s", a, b, total, text);
    memset(g_ds + (SECL - DS_BASE), 0, 14);
    for (m = 9; m >= 1; m--) {
        x = s16(SECC + 2u * (uint32_t)m);
        if (x > rest) x = rest;
        w8(SECL + (uint32_t)m, x);
        rest -= x;
    }
}
void cal1_test_handler(uint32_t addr, int k)
{
    tr("handler %X %d", (unsigned)addr, k);
    if (g_ds == R)
        g_cov[7]++;
}

/* ==== the model (g_ds == M), written from the disassembly ======================================================= */

static void m_bg3(void)
{
    f3CD6C(3);
    w8(0xEE168u, 3);
    fg_vbcopy(0, 639, 0, 479, 0, 479, s32(0xF52CCu), s32(0xF52C8u));
}

static void m_area(void) { w16(0xBB3FAu, 169); w16(0xBB3FCu, 340); w16(0xBB3FEu, 300); w16(0xBB400u, 35); }

static int m_human(uint32_t rec) { return (rb(0xE1624u) & 0x80) == 0 && sb(rec + 0x9A) == 3; }
static int m_cpu(uint32_t rec) { return (rb(0xE1624u) & 0x80) != 0 || sb(rec + 0x9A) == 2; }

static int m1E340(void)
{
    int n;
    for (n = 1; n <= 9; n++)
        if (rb(NAT(n) + 0x13))
            return 1;
    return 0;
}

static void m1E5B8(void)
{
    int id, n, j, c;
    w32(CCOUNT, 0);
    for (id = 19; id <= 30; id++)
        for (n = 1; n <= 9; n++) {
            if (rb(NAT(n) + 0x13) == 0)
                continue;
            for (j = 0; j < 8; j++)
                if (sb(NAT(n) + 0x14u + (uint32_t)j) == id && s32(CCOUNT) < 12) {   /* the port's guard */
                    c = s32(CCOUNT);
                    w8(CREC(c), rb(NAT(n) + 0x14u + (uint32_t)j));
                    w8(CREC(c) + 1, rb(NAT(n) + 0x1Cu + (uint32_t)j));
                    w8(CREC(c) + 2, n);
                    w32(CCOUNT, c + 1);
                }
        }
}

static void m1E388(void)
{
    char out[300], p[100];
    int n, d, j, i, x, c, cnt;
    uint32_t r;
    for (n = 1; n <= 9; n++) {
        r = NAT(n);
        if (rb(r + 0x9A) == 0 || sb(r + 0x13) < 3)
            continue;
        d = sb(r + 0x13) - 2;
        snprintf(out, sizeof out, "Zeus takes pity upon %s and discards ", nm(n));
        for (j = 0; j < d; j++) {
            cnt = sb(r + 0x13);
            x = (int8_t)(((uint32_t)f78AE4() * (uint32_t)cnt) >> 15);
            c = sb(r + 0x14u + (uint32_t)x);
            f771F8(c);
            if (g_ds == M) { /* the model and the port trace the same; coverage on the port side only */ }
            snprintf(p, sizeof p, j == 0 ? "%s" : j == d - 1 ? " and %s" : ", %s", cn(c));
            for (i = x; i < sb(r + 0x13) - 1; i++) {
                w8(r + 0x14u + (uint32_t)i, rb(r + 0x15u + (uint32_t)i));
                if (!g_original)
                    w8(r + 0x1Cu + (uint32_t)i, rb(r + 0x1Du + (uint32_t)i));
            }
            w8(r + 0x13, sb(r + 0x13) - 1);
            w8(r + 0x14u + (uint32_t)sb(r + 0x13), 0);
            if (!g_original)
                w8(r + 0x1Cu + (uint32_t)sb(r + 0x13), 0xFF);
            if (strlen(out) + strlen(p) < sizeof out)
                strcat(out, p);
        }
        strcat(out, ".");
        f79270(-500, n, -200, out, -298);
    }
}

static void m1E69C(int k)
{
    int v = sb(CREC(k) + 2), id = rb(CREC(k)), j, at = -1, c;
    uint32_t r = NAT(v);
    w16(0xF4094u, 1);
    for (j = 0; j < sb(r + 0x13); j++)
        if (rb(r + 0x14u + (uint32_t)j) == id) {
            at = j;
            break;
        }
    if (at < 0) {
        if (!g_original)
            return;
        at = 0;
    }
    c = sb(r + 0x13);
    for (j = at; j < c - 1; j++) {
        w8(r + 0x14u + (uint32_t)j, rb(r + 0x15u + (uint32_t)j));
        w8(r + 0x1Cu + (uint32_t)j, rb(r + 0x1Du + (uint32_t)j));
    }
    w8(r + 0x13, c - 1);
    w8(r + 0x14u + (uint32_t)sb(r + 0x13), 0);      /* 0x1E774 / 0x1E784 read the count again each time: with an */
    w8(r + 0x1Cu + (uint32_t)sb(r + 0x13), 0xFF);   /* empty hand (--original) the first write lands on +0x13 */
    f771F8((int8_t)id);
}

static void m_add(int t)
{
    int c = s32(LIST_N);
    w32(LIST + 4u * (uint32_t)c, t);
    w32(LIST_N, c + 1);
}

static int m_city(int t, int n) { return (rb(TS(t) + 2) & 1) && rb(TS(t)) == n; }

static void m28CFC(int n, int kind, int s)
{
    int t, j;
    w32(LIST_N, 0);
    for (t = 0; t < 157; t++)
        w32(LIST + 4u * (uint32_t)t, -1);
    switch (kind) {
    case 0x15: case 4: case 9:
        for (t = 0; t < 157; t++)
            if (m_city(t, n) || rb(AR(t + 1) + 0x0Eu + (uint32_t)n)) m_add(t);
        break;
    case 0x1B:
        for (t = 0; t < 157; t++)
            if (m_city(t, n) || sb(AR(t + 1) + 0x0Eu + (uint32_t)n) >= 2) m_add(t);
        break;
    case 7:
        for (t = 0; t < 157; t++) if (rb(0xF7984u + (uint32_t)t)) m_add(t);
        break;
    case 8:
        for (t = 0; t < 157; t++) if (rb(0xF7A24u + (uint32_t)t)) m_add(t);
        break;
    case 6:
        for (t = 0; t < 157; t++) if (rb(AR(t + 1) + 0x0Eu + (uint32_t)n)) m_add(t);
        break;
    case 3:
        for (t = 0; t < 157; t++) if (rb(AR(t + 1) + 0x24u + (uint32_t)n)) m_add(t);
        break;
    case 2: case 5:
        for (t = 0; t < 157; t++) if (m_city(t, n)) m_add(t);
        break;
    case 0x1E:
        for (t = 0; t < 157; t++)
            if ((rb(TS(t) + 2) & 1) && rb(MARK + (uint32_t)rb(TS(t))) && (rb(TR0(t)) & 2)) m_add(t);
        break;
    case 0x19:
        for (j = 0; j < s16(0xBEE0Au + 22u * (uint32_t)s); j++) {
            t = s16(0xBEE10u + 22u * (uint32_t)s + 2u * (uint32_t)j);
            if ((m_city(t, n) && (rb(TR1(t)) & 0x80)) || rb(AR(t + 1) + 0x0Eu + (uint32_t)n)) m_add(t);
        }
        break;
    default:
        break;
    }
}

static int m207B4(int p, int v, int kind)
{
    char b[100] = "";
    int t, sel = 0, re = 1, go = 1, r = 0, n9 = 9 - sb(NAT(p) + 0x51), eng = rb(NAT(v) + 0x2A), cnt;
    if (kind == 0x14) {
        if (p != v) {
            snprintf(b, sizeof b, "%s is a victim of Treachery by %s.  %s select the city to be %s.", nm(v), nm(p),
                     nm(p), n9 ? "replaced" : "eliminated");
            f3C284(0xBDE30u, 0x58, n9 ? 0xB1D15u : 0xB1D22u);
        } else {
            snprintf(b, sizeof b, "%s is a victim of his own Treachery. Select the city to be reduced.", nm(v));
            f3C284(0xBDE30u, 0x58, 0xB1D75u);
        }
    } else if (kind == 0x1A) {
        snprintf(b, sizeof b, "%s, choose territory for Barbarians to invade.", nm(p));
        f3C284(0xBDE30u, 0x58, 0xB1DB0u);
    } else if (kind == 0x19) {
        snprintf(b, sizeof b, "%s select your coastal city to be %s due to Flooding.", nm(v), eng ? "reduced" : "eliminated");
        f3C284(0xBDE30u, 0x58, eng ? 0xB1E05u : 0xB1E11u);
    }
    if (kind != 0x1A) {
        w32(LIST_N, 0);
        for (t = 0; t < 157; t++)
            w32(LIST + 4u * (uint32_t)t, -1);
        for (t = 0; t < 157; t++)
            if ((kind == 0x14 && m_city(t, v)) || (kind == 0x19 && m_city(t, v) && (rb(TR0(t)) & 2)))
                m_add(t);
    }
    f7853C(0xBDE30u);
    m_bg3();
    fg_vbopen(s32(0xF52C8u));
    f3AF8C(0xF51ACu, 251, 405);
    f3AF8C(0xF51B4u, 376, 405);
    f3A9F4(b);
    fg_setcolor(5);
    fg_vbclose();
    f7876C();
    f3B05C(0xF596Cu);
    w32(0xF40F8u, v);
    while (go) {
        cnt = s32(LIST_N);
        fg_vbopen(s32(0xF52C8u));
        f3C9D0(265, 405, 110, 19, 5);
        fg_setcolor(5);
        if (g_original || cnt > 0)
            f3A0B0(265, 409, 110, tn(s32(LIST + 4u * (uint32_t)sel)));
        fg_vbclose();
        f459E4(p);
        f3F670();
        if (g_original || cnt > 0)
            f3F624(s32(LIST + 4u * (uint32_t)sel), 15);
        f3CBBC();
        if (re) {
            if (g_original || cnt > 0)
                f3DA60(s32(LIST + 4u * (uint32_t)sel));
            else
                f3D52C(0, 0);
            cal1_test_f7A4C4(-600);
            re = 0;
        }
        r = f78BA4(0xF596Cu, 1);
        if (r == -100)
            continue;
        if (r >= 0) { sel = r; re = 1; continue; }
        if (r == -16) { r = s32(LIST + 4u * (uint32_t)sel); go = 0; }
        else if (r == -10) { if (sel < s32(LIST_N) - 1) { sel++; re = 1; } }
        else if (r == -9) { if (sel > 0) { sel--; re = 1; } }
    }
    f78664(0xBDE30u);
    return r;
}

static void m20FC4(int t, int p, int v)
{
    char a[100], b[100];
    uint32_t P = NAT(p), V = NAT(v);
    int j, at = -1, c, lim, k, an;
    snprintf(a, sizeof a, "%s %s city in %s was ", f789EC(sb(V + 0x3D)) ? "An" : "A", adj(sb(V + 0x3D)), tn(t));
    f3F670();
    f3F624(t, 15);
    f3DA60(t);
    f78A20(500);
    if (p != v) {
        if (sb(P + 0x51) != 9) {
            f78F04(t, v);
            f78E74(t, p);
            w8(TS(t), rb(P + 0x3D));
            w8(TS(t) + 1, rb(0xF40FCu));
            if (m_cpu(P)) {
                an = f789EC(sb(P + 0x3D));
                snprintf(b, sizeof b, "replaced by %s %s city.", g_original ? (an ? "a" : "an") : (an ? "an" : "a"),
                         adj(sb(P + 0x3D)));
                strcat(a, b);
            }
        } else {
            strcat(a, "eliminated.");
            f78F04(t, sb(V + 0x3D));
        }
        if (m_cpu(P)) {
            f79270(2000, v, t, a, 1);
        } else {
            f3DA60(t);
            f459E4(p);
            f3CBBC();
        }
        return;
    }
    w8(TS(t) + 2, rb(TS(t) + 2) & 0xFE);
    w8(TS(t), 0);
    w8(TS(t) + 1, 0xFF);
    c = sb(V + 0x51);
    for (j = 0; j < c; j++)
        if (rb(V + 0x52u + (uint32_t)j) == t + 1) { at = j; break; }
    if (at >= 0 || g_original) {
        if (at < 0) at = 0;
        for (j = at; j < c - 1; j++)
            w8(V + 0x52u + (uint32_t)j, rb(V + 0x53u + (uint32_t)j));
        w8(V + 0x51, c - 1);
        w8(V + 0x52u + (uint32_t)sb(V + 0x51), 0);
    }
    lim = ((rb(TR1(t)) << 2) & 0xFF) >> 5;
    lim += sb(NAT(v) + 0x27);
    k = sb(V + 0x99) < lim ? sb(V + 0x99) : lim;
    for (j = 0; j < k; j++)
        f57E28(t, v, 1);
    w8(V + 0x99, rb(V + 0x99) - k);
    w8(V + 0x98, rb(V + 0x98) + k);
    if (m_cpu(V)) {
        strcat(a, "reduced.");
        f79270(2000, v, t, a, 1);
    } else {
        f3DA60(t);
        f459E4(v);
        f3CBBC();
    }
}

static void m20564(int k)
{
    char b[100];
    uint32_t V = NAT(sb(CREC(k) + 2)), P;
    int j, idx = -1, g, t, vn = sb(V + 0x3D), pn;
    if (rb(V + 0x51) == 0) {
        snprintf(b, sizeof b, "%s has no cities and cannot be affected by Treachery.", nm(vn));
        f79270(-500, vn, -200, b, 1);
        m1E69C(k);
        return;
    }
    for (j = 0; j < 8; j++)
        if (sb(V + 0x14u + (uint32_t)j) == 0x14) { idx = j; break; }
    if (idx < 0 && !g_original)
        g = sb(CREC(k) + 1);
    else
        g = sb(V + 0x1Cu + (uint32_t)(idx < 0 ? 0 : idx));
    P = g == -1 ? V : NAT(g);
    pn = sb(P + 0x3D);
    if (m_human(P)) {
        t = m207B4(pn, vn, 0x14);
    } else {
        if (rb(P + 0x3D) != rb(V + 0x3D))
            snprintf(b, sizeof b, "%s is a victim of Treachery by %s.", nm(vn), nm(pn));
        else
            snprintf(b, sizeof b, "%s is a victim of his own Treachery.", nm(vn));
        f79270(-500, vn, -200, b, 1);
        t = cal1_test_f20C70(pn, vn) - 1;
    }
    if (P != V && sb(P + 0x51) < 9)
        w8(AR(t + 1) + 0x3Bu + (uint32_t)vn, 1);
    f3A990();
    f3CBBC();
    m20FC4(t, pn, vn);
    m1E69C(k);
}

static int m23574(int n)
{
    int r = cal1_test_f213C0((int8_t)n), j;
    for (j = 0; j < 9; j++)
        if (rb(NAT((int8_t)n) + 0x52u + (uint32_t)j) == r)
            return r;
    return -1;
}

/* the computer's reductions of Superstition (with the highlight and 500 ms), Civil Disorder and Iconoclasm */
static void m_cpu_reduce(int n, int t, const char *fmt, int ms)
{
    char b[200];
    f78FEC(t, n);
    snprintf(b, sizeof b, fmt, nm(n), tn(t));
    f79270((rb(0xF0980u) & 4) ? -500 : ms, n, t, b, 1);
}

static void m23230(int k)
{
    char b[200];
    uint32_t V = NAT(sb(CREC(k) + 2));
    int32_t n = sb(V + 0x3D);
    int c, i, t;
    w32(0xF40F8u, n);
    m_bg3();
    if (rb(V + 0x51) == 0) {
        snprintf(b, sizeof b, "%s has no cities to be affected by Superstition.", nm(n));
        f79270(-500, n, -200, b, 1);
        m1E69C(k);
        return;
    }
    c = rb(V + 0x39) ? 0 : rb(V + 0x38) ? 1 : rb(V + 0x37) ? 2 : 3;
    snprintf(b, sizeof b, "%s has been affected by Superstition. ", nm(n));
    if (sb(V + 0x51) < c) {
        c = 0;
        strcat(b, "All your cities have been reduced. ");
        while (rb(V + 0x51))
            f78FEC(rb(V + 0x52) - 1, n);
    } else {
        strcat(b, c == 2 ? "Mysticism acts as a restraint. " : c == 1 ? "Deism acts as a restraint. " :
                  c == 0 ? (g_original ? "Enlightenment nullifys the effect." : "Enlightenment nullifies the effect.") : "");
    }
    f79270(-500, n, -200, b, 1);
    f3A990();
    f3CBBC();
    for (i = 0; i < c; i++) {
        if (m_human(V)) {
            snprintf(b, sizeof b, "You must reduce %d cit%s.", c - i, c - i == 1 ? "y" : "ies");
            f5C880(&n, 1, b, 0);
            f3A924();
        } else {
            t = m23574(n) - 1;
            f3F670();
            f3F624(t, 15);
            f3DA60(t);
            f78A20(500);
            m_cpu_reduce(n, t, "%s reduces a city in %s.", 2000);
        }
    }
    m1E69C(k);
}

static void m235E4(int k)
{
    char b[100];
    uint32_t V = NAT(sb(CREC(k) + 2));
    int n = sb(V + 0x3D), loss;
    w16(0xF7AFEu, n);
    w32(0xF40F8u, n);
    loss = rb(V + 0x29) ? 20 : 15;
    if (rb(V + 0x39))
        loss -= 5;
    if (loss > sb(V + 0x98))
        loss = sb(V + 0x98);
    w16(0xF7AFCu, loss);
    snprintf(b, sizeof b, "%s suffers a Slave Revolt", nm(n));
    if (2 * sb(V + 0x51) <= sb(V + 0x98) - s16(0xF7AFCu)) {
        strcat(b, " and emerges without any losses.");
        f79270(-500, n, -200, b, 1);
    } else {
        strcat(b, ".");
        f79270(-500, n, -200, b, 1);
        f3A990();
        f3CBBC();
        b[0] = 0;
        f5C750(n, s16(0xF7AFCu), b);
    }
    m1E69C(k);
}

static void m237A8(int k)
{
    char b[100];
    uint32_t V = NAT(sb(CREC(k) + 2));
    int32_t n = sb(V + 0x3D);
    int c, i;
    w32(0xF40F8u, n);
    m_bg3();
    if (rb(V + 0x51) == 0) {
        snprintf(b, sizeof b, "%s has no cities to be affected by Civil Disorder.", nm(n));
        f79270(-500, n, -200, b, 1);
        m1E69C(k);
        return;
    }
    c = sb(V + 0x51) - 3 - (rb(V + 0x30) != 0) - (rb(V + 0x2F) != 0) - (rb(V + 0x33) != 0) - (rb(V + 0x34) != 0) +
        (rb(V + 0x28) != 0) + (rb(V + 0x35) != 0);
    if (c < 0)
        c = 0;
    snprintf(b, sizeof b, "%s has been struck by Civil Disorder%s", nm(n), c == 0 ? " and is unaffected." : ". ");
    f79270(-500, n, -200, b, 1);
    for (i = 0; i < c; i++) {
        if (m_human(V)) {
            snprintf(b, sizeof b, "You must reduce %d cit%s.", c - i, c - i == 1 ? "y" : "ies");
            f5C880(&n, 1, b, 0);
            f3A924();
        } else {
            m_cpu_reduce(n, cal1_test_f213C0((int8_t)n) - 1, "%s reduces a city in %s.", 2000);
        }
    }
    m1E69C(k);
}

static void m23ACC(int k)
{
    char b[100];
    int tr_ = sb(CREC(k) + 1), c, i, tot, x;
    uint32_t V = NAT(sb(CREC(k) + 2));
    int32_t n = sb(V + 0x3D), m;
    w32(0xF40F8u, n);
    m_area();
    m_bg3();
    if (rb(V + 0x51) == 0) {
        snprintf(b, sizeof b, "%s has no cities to be affected by Iconoclasm.", nm(n));
        f79270(-500, n, -200, b, 1);
        m1E69C(k);
        return;
    }
    c = 4 - (rb(V + 0x33) != 0) - (rb(V + 0x36) != 0) - 3 * (rb(V + 0x3B) != 0) + (rb(V + 0x3A) != 0) +
        (rb(V + 0x28) != 0);
    if (c < 0)
        c = 0;
    if (c > sb(V + 0x51))
        c = sb(V + 0x51);
    snprintf(b, sizeof b, "%s has been struck by Iconoclasm & Heresy%s", nm(n), c ? "." : " and is unaffected.");
    f79270(-500, n, -200, b, 1);
    for (i = 0; i < c; i++) {
        if (m_human(V)) {
            snprintf(b, sizeof b, "%s, as Iconoclasm's primary victim you must reduce %d cit%s.", nm(n), c - i,
                     c - i == 1 ? "y" : "ies");
            f5C880(&n, 1, b, 0);
            f3A924();
        } else {
            m_cpu_reduce(n, cal1_test_f213C0((int8_t)n) - 1, "%s reduces a city in %s due to Iconoclasm.", 2000);
        }
    }
    w16(0xBB400u, 21);
    tot = 0;
    for (m = 1; m <= 9; m++) {
        x = 0;
        if (rb(NAT(m) + 0x9A) && m != n && m != tr_ && !rb(NAT(m) + 0x3B)) {
            x = sb(NAT(m) + 0x51) > 2 ? 2 : sb(NAT(m) + 0x51);
            if (rb(NAT(m) + 0x36) && x == 2)
                x = 1;
        }
        w16(SECC + 2u * (uint32_t)m, x);
        tot += (int16_t)x;
    }
    if (tot == 0) {
        f79270(-500, n, -200, "There are no eligible secondary losses", 1);
        m1E69C(k);
        return;
    }
    if (m_human(V)) {
        f3A924();
        if (tot <= 2) {
            for (m = 1; m <= 9; m++)
                w8(SECL + (uint32_t)m, rb(SECC + 2u * (uint32_t)m));
            cal1_test_f291D0(n, -1, "Assignments for Iconoclasm secondary losses");
        } else {
            snprintf(b, sizeof b, "%s, assign secondary losses for Iconoclasm", nm(n));
            cal1_test_f29550(n, -1, 2, b);
        }
    } else {
        for (m = 1; m <= 9; m++)
            w8(SECL + (uint32_t)m, rb(SECC + 2u * (uint32_t)m));
        if (tot <= 2)
            cal1_test_f291D0(n, -1, "Assignments for Iconoclasm secondary losses");
        else
            cal1_test_f27518(n, tr_, 2, 0x1D);
    }
    for (m = 1; m <= 9; m++)
        for (i = 0; i < sb(SECL + (uint32_t)m); i++) {
            if (m_human(NAT(m))) {
                x = sb(SECL + (uint32_t)m) - i;
                snprintf(b, sizeof b, "%s, as an Iconoclasm secondary victim you must reduce %d cit%s.", nm(m), x,
                         x == 1 ? "y" : "ies");
                f5C880(&m, 1, b, 0);
                f3A924();
            } else {
                m_cpu_reduce(m, cal1_test_f213C0((int8_t)m) - 1,
                             g_original ? "%s reduces a city in %s ." : "%s reduces a city in %s.", 2500);
            }
        }
    m1E69C(k);
}

static void m_snap(int save)   /* 0x2468C's three blocks: areas 0..160, territory states 0..159, nations 1..11 */
{
    uint8_t *a = load_heap_ptr((uint32_t)s32(0xEED74u), 0x2B65u), *ts = load_heap_ptr((uint32_t)s32(0xEED7Cu), 0x1E0u),
            *na = load_heap_ptr((uint32_t)s32(0xEED78u), 0x954u);
    uint8_t *d = g_ds - DS_BASE;
    if (a) { if (save) memcpy(a, d + 0xE31B8u, 161 * 69); else memcpy(d + 0xE31B8u, a, 161 * 69); }
    if (ts) { if (save) memcpy(ts, d + 0xE2C18u, 480); else memcpy(d + 0xE2C18u, ts, 480); }
    if (na) { if (save) memcpy(na + 199, d + NAT(1), 11 * 199); else memcpy(d + NAT(1), na + 199, 11 * 199); }
}

static void m_marks(int c, int v, int mode)
{
    int i;
    for (i = 0; i < 12; i++)
        w8(MARK + (uint32_t)i, 0);
    if (mode == 0 || mode == 2)
        w8(MARK + (uint32_t)v, 1);
    else if (mode == 1)
        for (i = 1; i <= 9; i++)
            if (rb(NAT(i) + 0x9A) && i != v && i != c)
                w8(MARK + (uint32_t)i, 1);
}

static void m2468C(int c, int v, int k, int mode)
{
    char b[100];
    int kind = mode == 2 ? 2 : 0x1E, re = 1, sel = 0, rem = 0, left = k, r, t, o, i, cnt;
    int32_t s;
    w32(0xEED74u, (int32_t)load_heap_alloc(0x2B65));
    w32(0xEED7Cu, (int32_t)load_heap_alloc(0x1E0));
    w32(0xEED78u, (int32_t)load_heap_alloc(0x954));
    m_snap(1);
    m_marks(c, v, mode);
    m_bg3();
    fg_vbopen(s32(0xF52C8u));
    f3AF8C(0xF51ACu, 251, 405);
    f3AF8C(0xF51B4u, 376, 405);
    fg_setcolor(5);
    f3A3A0(303, 389, "LEFT TO");
    f3A044(336, 389, "REMOVE");
    fg_vbclose();
    if (mode == 2)
        snprintf(b, sizeof b, "%s, select cities to be received thru Tax Revolt.", nm(c));
    else if (mode == 0)
        snprintf(b, sizeof b, "%s, select %s coastal cities to be taken by Pirates.", nm(c), adj(v));
    else
        snprintf(b, sizeof b, "%s, select secondary coastal cities to be taken by Pirates.", nm(v));
    f3AB7C(b);
    f7876C();
    f7853C(0xBEE78u);
    f3B05C(0xF596Cu);
    m28CFC(v, kind, 0);
    if (!g_original && s32(LIST_N) == 0 && left > 0)
        left = 0;
    for (;;) {
        cnt = s32(LIST_N);
        fg_vbopen(s32(0xF52C8u));
        f3C9D0(265, 405, 110, 19, 5);
        fg_setcolor(5);
        if (g_original || cnt > 0)
            f3A0B0(265, 409, 110, tn(s32(LIST + 4u * (uint32_t)sel)));
        f3C9D0(310, 385, 19, 19, 5);
        fg_setcolor(5);
        snprintf(b, sizeof b, "%d", left);
        f3A0B0(310, 389, 19, b);
        fg_vbclose();
        f3F670();
        if (g_original || cnt > 0)
            f3F624(s32(LIST + 4u * (uint32_t)sel), 15);
        if (left > 0) { f3C3D0(0xF596Cu, 0x2A); f3C314(0xF596Cu, 0x3F); }
        else { f3C314(0xF596Cu, 0x2A); f3C3D0(0xF596Cu, 0x3F); }
        if (re) {
            f459E4(v);
            if (cnt > 0) f3DA60(s32(LIST + 4u * (uint32_t)sel)); else f3D52C(0, 0);
        }
        f3CBBC();
        if (re) {
            if (rb(0xF4102u) == 15)
                cal1_test_f7A4C4(-600);
            re = 0;
        }
        r = f78BA4(0xF596Cu, 1);
        if (r == -100)
            continue;
        if (r >= 0) { sel = r; re = 1; continue; }
        if (r == -1)
            break;
        if (r == -0x65) {
            t = s32(LIST + 4u * (uint32_t)sel);
            if (mode == 1)
                w8(MARK + (uint32_t)rb(TS(t)), 0);
            o = rb(TS(t));
            f78F04(t, o);
            if (mode != 2) {
                f78E74(t, 10);
                w8(AR(t + 1) + 0x3Bu + (uint32_t)o, 2);
                for (i = 1; i <= 9; i++)
                    if (sb(AR(t + 1) + 0x3Bu + (uint32_t)i) == 1)
                        w8(AR(t + 1) + 0x3Bu + (uint32_t)i, 2);
            } else {
                f78E74(t, c);
                w8(AR(t + 1) + 0x3Bu + (uint32_t)o, 1);
            }
            left--;
            rem = 1;
            re = 1;
        } else if (r == -0x63) {
            if ((rb(0xF097Cu) & 8) == 0 || f795A8("Are you sure you want to Restart?") == -0x61) {
                m_snap(0);
                if (!g_original)
                    m_marks(c, v, mode);
                left = k;
                m28CFC(v, kind, 0);
                sel = 0;
                re = 1;
                if (!g_original && s32(LIST_N) == 0 && left > 0)
                    left = 0;
            }
        } else if (r == -10) {
            if (sel < s32(LIST_N) - 1) { sel++; re = 1; }
        } else if (r == -9) {
            if (sel > 0) { sel--; re = 1; }
        }
        if (rem) {
            for (i = sel; i < s32(LIST_N) - 1; i++)
                w32(LIST + 4u * (uint32_t)i, s32(LIST + 4u * (uint32_t)i + 4));
            w32(LIST_N, s32(LIST_N) - 1);
            s = sel;
            f79A9C(&s);
            sel = s;
            rem = 0;
            m28CFC(v, kind, 0);
            if (!g_original && s32(LIST_N) == 0 && left > 0)
                left = 0;
        }
    }
    f78664(0xBEE78u);
    load_heap_free((uint32_t)s32(0xEED74u)); w32(0xEED74u, 0);
    load_heap_free((uint32_t)s32(0xEED7Cu)); w32(0xEED7Cu, 0);
    load_heap_free((uint32_t)s32(0xEED78u)); w32(0xEED78u, 0);
}

static int m_coastal_cities(int n)
{
    int j, c = 0;
    for (j = 0; j < sb(NAT(n) + 0x51); j++)
        if (rb(TR0(rb(NAT(n) + 0x52u + (uint32_t)j) - 1)) & 2)
            c++;
    return c;
}

static void m241D8(int k)
{
    char b[300];
    int v = sb(CREC(k) + 2), c = sb(CREC(k) + 1), room, cnt, m, take, anim = 1;
    uint32_t C;
    if (c == -1)
        c = v;
    C = NAT(c);
    w32(0xF40F8u, v);
    if (sb(NAT(10) + 0x51) == 9) {
        snprintf(b, sizeof b, "The management of Skull & Crossbones, Inc. regret to inform you that they are unable to "
                 "add new cities to their portfolio.  %s is spared from Piracy.", nm(v));
        goto msg;
    }
    room = 9 - sb(NAT(10) + 0x51);
    if (room > 4) room = 4;
    w32(LIST_N, m_coastal_cities(v));
    if (s32(LIST_N) == 0) {
        snprintf(b, sizeof b, "The primary victim %s has no coastal cities for the Pirates to %s.", nm(v),
                 g_original ? "takeover" : "take over");
        f79270(-500, v, -200, b, 1);
    } else {
        snprintf(b, sizeof b, "%s falls victim to Pirates.", nm(v));
        f79270(-500, v, -200, b, 1);
        take = room < 2 ? room : (s32(LIST_N) > 2 ? 2 : s32(LIST_N));
        if (m_human(C)) {
            m2468C(c, v, take, 0);
        } else {
            f3C720(0xF596Cu);
            cal1_test_f24EB8(c, v, take);
            f3C6A0(0xF596Cu);
        }
    }
    if (room == 1 && s32(LIST_N) > 1) {
        snprintf(b, sizeof b, "The Pirates are unable to add more cities.  The remaining primary and all secondary "
                 "victims are spared.");
        goto msg;
    }
    if (sb(NAT(10) + 0x51) == 9) {
        snprintf(b, sizeof b, "The Pirates are unable to add more cities.  The secondary victims are spared.");
        goto msg;
    }
    room = 9 - sb(NAT(10) + 0x51);
    if (room > 2) room = 2;
    cnt = 0;
    for (m = 1; m <= 9; m++)
        if (rb(NAT(m) + 0x9A) && m != v && m != c)
            cnt += m_coastal_cities(m);
    w32(LIST_N, cnt);
    if (cnt == 0) {
        snprintf(b, sizeof b, "Secondary victims have no coastal cities for the Pirates to %s.",
                 g_original ? "takeover" : "take over");
        f79270(-500, v, -200, b, 1);
    } else if (m_human(NAT(v))) {
        m2468C(c, v, room < 2 ? room : (cnt > 2 ? 2 : cnt), 1);
    } else {
        f3A924();
        snprintf(b, sizeof b, "%s selects secondary coastal cities to be taken by Pirates.", nm(v));
        f79270((rb(0xF0980u) & 4) ? -500 : 2500, v, -200, b, 1);
        f3C720(0xF596Cu);
        f3CBBC();
        cal1_test_f252BC(c, v, room < 2 ? room : (s32(LIST_N) > 2 ? 2 : s32(LIST_N)));
        f3C6A0(0xF596Cu);
    }
    if (room == 1 && s32(LIST_N) > 1) {
        snprintf(b, sizeof b, "The Pirates are unable to add more cities.  The remaining secondary victims are spared.");
        goto msg;
    }
    m1E69C(k);
    return;
msg:
    f79270(-500, v, -200, b, anim);
    m1E69C(k);
}

static void m1E0A0(void)
{
    int n, k, v, cnt;
    memset(g_ds + (0xF409Au - DS_BASE), 0, 20);
    if (!m1E340()) {
        f79270(100, s16(0xF06B0u), -200, "", 0);
        return;
    }
    f553C0();
    m_area();
    if (!(rb(0xE1624u) & 0x10))
        m1E388();
    m1E5B8();
    for (cnt = 0, n = 1; n <= 9; n++)
        if (rb(NAT(n) + 0x9A))
            w16(0xF06B0u + 2u * (uint32_t)cnt++, n);
    w16(0xF06AEu, cnt);
    f783B8(0xF596Cu, s32(0xF5940u));
    cal1_test_f7A4A0();
    for (k = 0; k < s32(CCOUNT); k++) {
        v = sb(CREC(k) + 2);
        if (rb(NAT(v) + 0x9A)) {
            w32(0xF4204u + 4u * (uint32_t)v, sb(NAT(v) + 0x98) * 100 / s32(0xF4898u) > 75);
            f7C960();
            w32(0xF783Cu, sb(CREC(k)));
            switch (rb(CREC(k))) {
            case 0x13: cal1_test_handler(0x1E7A0u, k); break;
            case 0x14: m20564(k); break;
            case 0x15: cal1_test_handler(0x25A38u, k); break;
            case 0x16: m23230(k); break;
            case 0x17: cal1_test_handler(0x29E34u, k); break;
            case 0x18: m235E4(k); break;
            case 0x19: cal1_test_handler(0x2D4F0u, k); break;
            case 0x1A: cal1_test_handler(0x217A0u, k); break;
            case 0x1B: cal1_test_handler(0x2E59Cu, k); break;
            case 0x1C: m237A8(k); break;
            case 0x1D: m23ACC(k); break;
            case 0x1E: m241D8(k); break;
            default: break;
            }
        }
        f22B00();
        f5E7B4();
    }
}

/* ==== runs ======================================================================================================== */

enum { R_1E0A0, R_1E340, R_1E388, R_1E5B8, R_1E69C, R_20564, R_207B4, R_20FC4, R_23230, R_23574, R_235E4, R_237A8,
       R_23ACC, R_241D8, R_2468C, R_28CFC };

static int g_checks, g_fail;
static uint32_t g_rng = 12345u;
static int g_a[4], g_ret_real, g_ret_model;

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
    case R_1E0A0: if (model) m1E0A0(); else f1E0A0(); return 0;
    case R_1E340: return model ? m1E340() : f1E340();
    case R_1E388: if (model) m1E388(); else f1E388(); return 0;
    case R_1E5B8: if (model) m1E5B8(); else f1E5B8(); return 0;
    case R_1E69C: if (model) m1E69C(g_a[0]); else f1E69C(g_a[0]); return 0;
    case R_20564: if (model) m20564(g_a[0]); else f20564(g_a[0]); return 0;
    case R_207B4: return model ? m207B4(g_a[0], g_a[1], g_a[2]) : f207B4(g_a[0], g_a[1], g_a[2]);
    case R_20FC4: if (model) m20FC4(g_a[0], g_a[1], g_a[2]); else f20FC4(g_a[0], g_a[1], g_a[2]); return 0;
    case R_23230: if (model) m23230(g_a[0]); else f23230(g_a[0]); return 0;
    case R_23574: return model ? m23574(g_a[0]) : f23574(g_a[0]);
    case R_235E4: if (model) m235E4(g_a[0]); else f235E4(g_a[0]); return 0;
    case R_237A8: if (model) m237A8(g_a[0]); else f237A8(g_a[0]); return 0;
    case R_23ACC: if (model) m23ACC(g_a[0]); else f23ACC(g_a[0]); return 0;
    case R_241D8: if (model) m241D8(g_a[0]); else f241D8(g_a[0]); return 0;
    case R_2468C: if (model) m2468C(g_a[0], g_a[1], g_a[2], g_a[3]); else f2468C(g_a[0], g_a[1], g_a[2], g_a[3]); return 0;
    case R_28CFC: if (model) m28CFC(g_a[0], g_a[1], g_a[2]); else f28CFC(g_a[0], g_a[1], g_a[2]); return 0;
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
    g_en2A = g_en3F = -1;
    g_screen = 0;
    g_iforced = 0;
    heap_reset();
    if (setjmp(g_jb) == 0)
        r = call(which, model);
    if (model) g_ret_model = r; else g_ret_real = r;
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
    if (g_ret_real != g_ret_model) {
        printf("FAIL %s: return %d / %d\n", what, g_ret_real, g_ret_model);
        g_fail++;
        return 0;
    }
    return 1;
}

static int has(const char *s) { return strstr(g_real, s) != NULL; }
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
    uint32_t a = AR(t + 1);
    if (k <= 0)
        return;
    if (sb(a + 1u + (uint32_t)n) != n) {
        w8(a + 1, rb(a + 1) + 1);
        w8(a + 1u + (uint32_t)n, n);
    }
    w8(a + 0x0Eu + (uint32_t)n, sb(a + 0x0Eu + (uint32_t)n) + k);
}

static int put_city(int t, int n, int round)
{
    uint32_t rec = NAT(n);
    int c = sb(rec + 0x51);
    if (c >= 9 || (rb(TS(t) + 2) & 1))
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
        w8(rec + 0x13, 0);
        memset(g_ds + (rec + 0x14 - DS_BASE), 0, 8);
        memset(g_ds + (rec + 0x1C - DS_BASE), 0xFF, 8);
    }
}

static void give_cal(int n, int id, int giver)   /* calamity id into n's hand with its giver */
{
    uint32_t rec = NAT(n);
    int c = sb(rec + 0x13);
    if (c >= 8)
        return;
    w8(rec + 0x14u + (uint32_t)c, id);
    w8(rec + 0x1Cu + (uint32_t)c, giver);
    w8(rec + 0x13, c + 1);
}

static void set_rec(int k, int id, int giver, int v)
{
    w8(CREC(k), id);
    w8(CREC(k) + 1, giver);
    w8(CREC(k) + 2, v);
}

static void base_clean(void)
{
    int n, a;
    memcpy(R, g_pristine, DS_SIZE);
    g_ds = R;
    clear_board();
    for (n = 1; n <= 11; n++) {
        w8(NAT(n) + 0x9A, n <= 9 ? 3 : 0);
        for (a = 0x24; a <= 0x3B; a++)
            w8(NAT(n) + (uint32_t)a, 0);
        w8(NAT(n) + 0x98, 20);
        w8(NAT(n) + 0x99, 20);
        w8(NAT(n) + 0x3D, n);
    }
    for (n = 0; n < 157; n++)
        w8(TR0(n), rb(TR0(n)) & ~2);
    w16(0xF40FCu, 3);
    w8(0xE1624u, rb(0xE1624u) & 0x6F);
    w8(0xF0980u, rb(0xF0980u) & ~4);
    w8(0xF097Cu, rb(0xF097Cu) & ~8);
    w8(0xF4102u, 15);
    w32(0xF4898u, 55);
    w32(CCOUNT, 0);
    g_nforced = 0;
    g_yes = -1;
    g_original = 0;
}

/* a random game state on top of the current R: players, advances, the board, cities, the hands */
static void random_world(int keep_board)
{
    int n, t, k, c, a, round = 1 + (int)rnd(12), inplay = 0;
    g_ds = R;
    w16(0xF40FCu, round);
    if (!keep_board) {
        clear_board();
        for (t = 0; t < 157; t++) {
            w8(TR0(t), (rb(TR0(t)) & ~2) | (rnd(2) ? 2 : 0));
            w8(TR1(t), (rb(TR1(t)) & 0x47) | (int)rnd(6) << 3 | (rnd(4) == 0 ? 0x80 : 0));
        }
    }
    for (n = 1; n <= 9; n++) {
        if (!keep_board)
            w8(NAT(n) + 0x9A, rnd(6) == 0 ? 0 : rnd(4) == 0 ? 2 : 3);
        else if (rb(NAT(n) + 0x9A) != 0)
            w8(NAT(n) + 0x9A, rnd(4) == 0 ? 2 : 3);
        inplay += rb(NAT(n) + 0x9A) != 0;
        for (a = 0x24; a <= 0x3B; a++)
            if (rnd(4) == 0)
                w8(NAT(n) + (uint32_t)a, rnd(5) != 0);
        w8(NAT(n) + 0x98, (int)rnd(45));
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
            c = n == 10 ? (int)rnd(10) : (int)rnd(10);
            for (k = 0; k < c * 3 && sb(NAT(n) + 0x51) < c; k++)
                put_city((int)rnd(157), n, rnd(3) == 0 ? round : (int)rnd(round + 1));
        }
        for (k = 0; k < 120; k++) {
            n = 1 + (int)rnd(9);
            t = (int)rnd(157);
            if (rb(NAT(n) + 0x9A) && !(rb(TS(t) + 2) & 1))
                put_tokens(t, n, 1 + (int)rnd(4));
        }
        for (k = 0; k < 40; k++)
            w8(AR(1 + rnd(157)) + 0x24u + rnd(10), (int)rnd(3));
    }
    for (t = 0; t < 157; t++) {
        w8(0xF7984u + (uint32_t)t, rnd(5) == 0);
        w8(0xF7A24u + (uint32_t)t, rnd(6) == 0);
    }
    w8(0xE1624u, (rb(0xE1624u) & 0x6F) | (rnd(25) == 0 ? 0x80 : 0) | (rnd(5) == 0 ? 0x10 : 0));
    w8(0xF0980u, (rb(0xF0980u) & ~4) | (rnd(2) ? 4 : 0));
    w8(0xF097Cu, (rb(0xF097Cu) & ~8) | (rnd(2) ? 8 : 0));
    w8(0xF4102u, rnd(8) == 0 ? 6 : 15);
    w32(0xF4898u, 40 + (int)rnd(20));
    w32(0xCAC5Cu, (int32_t)(rnd(0x10000) << 16 | rnd(0x10000)));
}

/* random hands: every id at most once (the 12 calamity cards), givers drawn (-1) or another nation */
static void random_hands(int pct)
{
    int id, n, g;
    for (n = 1; n <= 9; n++) {
        w8(NAT(n) + 0x13, 0);
        memset(g_ds + (NAT(n) + 0x14 - DS_BASE), 0, 8);
        memset(g_ds + (NAT(n) + 0x1C - DS_BASE), 0xFF, 8);
    }
    for (id = 0x13; id <= 0x1E; id++) {
        if ((int)rnd(100) >= pct)
            continue;
        n = 1 + (int)rnd(9);
        if (rb(NAT(n) + 0x9A) == 0 && rnd(3) != 0)
            continue;
        g = rnd(3) == 0 ? -1 : 1 + (int)rnd(9);
        if (g == n)
            g = -1;
        give_cal(n, id, g);
    }
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

static void part_a(void)
{
    int f0 = g_fail, i, x, cnt, id[4], gv[4];
    uint32_t seed, s;
    char e[200];

    /* A1 0x1E340 */
    base_clean();
    run_both(R_1E340, 1, "A1 none");
    expect(g_ret_real == 0, "A1 no calamity -> 0");
    give_cal(7, 0x16, -1);
    run_both(R_1E340, 1, "A1 one");
    expect(g_ret_real == 1, "A1 a calamity -> 1");
    w8(NAT(7) + 0x9A, 0);
    run_both(R_1E340, 1, "A1 out of play");
    expect(g_ret_real == 1, "A1 no +0x9A test");

    /* A1b 0x1E0A0 with nothing held: the 100 ms home view of the first player, nothing else */
    base_clean();
    w16(0xF06B0u, 4);
    run_both(R_1E0A0, 1, "A1b empty phase");
    expect(strcmp(g_real, "msg 100 4 -200  0\n") == 0, "A1b only f79270(100, first, -200, \"\", 0)");

    /* A2 0x1E5B8: by id, then nation, then slot */
    base_clean();
    give_cal(3, 0x1E, 5); give_cal(3, 0x14, -1);
    give_cal(1, 0x16, 2);
    give_cal(8, 0x14 + 0, 1);   /* a second Treachery (impossible in play; checks the order) */
    give_cal(2, 0x13, -1);
    run_both(R_1E5B8, 1, "A2");
    expect(S32(CCOUNT) == 5, "A2 five records");
    expect(D8(CREC(0)) == 0x13 && D8(CREC(0) + 2) == 2 && D8(CREC(1)) == 0x14 && D8(CREC(1) + 2) == 3 &&
           S8(CREC(1) + 1) == -1 && D8(CREC(2)) == 0x14 && D8(CREC(2) + 2) == 8 && D8(CREC(2) + 1) == 1 &&
           D8(CREC(3)) == 0x16 && D8(CREC(3) + 1) == 2 && D8(CREC(4)) == 0x1E && D8(CREC(4) + 1) == 5,
           "A2 order id / nation, givers copied");

    /* A3 Zeus: 4 calamities -> 2 discards; the expected picks from the generator written out */
    for (i = 0; i < 2; i++) {
        base_clean();
        g_original = i;
        id[0] = 0x14; id[1] = 0x16; id[2] = 0x1C; id[3] = 0x1E;
        gv[0] = 2; gv[1] = -1; gv[2] = 4; gv[3] = 6;
        for (x = 0; x < 4; x++)
            give_cal(5, id[x], gv[x]);
        seed = 0x12345678u;
        setD32(0xCAC5Cu, seed);
        run_both(R_1E388, 1, i ? "A3 Zeus original" : "A3 Zeus");
        /* the model of the draws: idx = rand * count >> 15 */
        s = seed;
        cnt = 4;
        {
            int ids[4], gs[4], nn = 4, k2, j2, ix;
            char exp[200];
            for (k2 = 0; k2 < 4; k2++) { ids[k2] = id[k2]; gs[k2] = gv[k2]; }
            snprintf(exp, sizeof exp, "Zeus takes pity upon CRETE and discards ");
            for (k2 = 0; k2 < 2; k2++) {
                s = s * 0x15A4E35u + 1u;
                ix = (int)((((s >> 16) & 0x7FFF) * (uint32_t)nn) >> 15);
                strcat(exp, k2 == 0 ? "" : " and ");
                strcat(exp, k2 == 0 ? "" : "");
                {
                    char nm2[40];
                    snprintf(nm2, sizeof nm2, "%s", cn(ids[ix]));
                    strcat(exp, nm2);
                }
                for (j2 = ix; j2 < nn - 1; j2++) {
                    ids[j2] = ids[j2 + 1];
                    if (!i) gs[j2] = gs[j2 + 1];
                }
                nn--;
            }
            strcat(exp, ".");
            snprintf(e, sizeof e, "msg -500 5 -200 %s -298", exp);
            expect(has(e), i ? "A3 Zeus text (original)" : "A3 Zeus text");
            expect(S8(NAT(5) + 0x13) == 2 && D8(NAT(5) + 0x14) == ids[0] && D8(NAT(5) + 0x15) == ids[1] &&
                   D8(NAT(5) + 0x16) == 0, "A3 two left");
            if (!i)
                expect(S8(NAT(5) + 0x1C) == gs[0] && S8(NAT(5) + 0x1D) == gs[1] && D8(NAT(5) + 0x1E) == 0xFF,
                       "A3 givers move with the cards (fixed)");
            else
                expect(S8(NAT(5) + 0x1C) == gv[0] && S8(NAT(5) + 0x1D) == gv[1] && S8(NAT(5) + 0x1E) == gv[2],
                       "A3 givers left in place (original)");
            expect(D32(0xCAC5Cu) == s, "A3 two draws");
        }
        (void)cnt;
    }
    g_original = 0;

    /* A4 0x1E69C */
    base_clean();
    give_cal(6, 0x15, 3); give_cal(6, 0x1C, -1); give_cal(6, 0x1D, 2);
    set_rec(0, 0x1C, -1, 6);
    g_a[0] = 0;
    run_both(R_1E69C, 1, "A4");
    expect(S8(NAT(6) + 0x13) == 2 && D8(NAT(6) + 0x14) == 0x15 && D8(NAT(6) + 0x15) == 0x1D && D8(NAT(6) + 0x16) == 0 &&
           S8(NAT(6) + 0x1C) == 3 && S8(NAT(6) + 0x1D) == 2 && D8(NAT(6) + 0x1E) == 0xFF && S16(0xF4094u) == 1 &&
           has("discard 28"), "A4 card out, givers along, discarded");
    set_rec(0, 0x1E, -1, 6);
    run_both(R_1E69C, 1, "A4 not held");
    expect(S8(NAT(6) + 0x13) == 2 && !has("discard"), "A4 not held: nothing (fixed)");
    g_original = 1;
    run_both(R_1E69C, 1, "A4 not held original");
    expect(S8(NAT(6) + 0x13) == 1 && D8(NAT(6) + 0x14) == 0x1D && has("discard 30"), "A4 not held: index 0 (original)");
    g_original = 0;

    /* A5 Superstition: 3 / Mysticism 2 / Deism 1 / Enlightenment 0, all cities when fewer */
    {
        static const int adv[4] = { 0, 0x37, 0x38, 0x39 };
        static const int want[4] = { 3, 2, 1, 0 };
        for (i = 0; i < 4; i++) {
            base_clean();
            for (x = 0; x < 5; x++) put_city(10 + x, 4, 1);
            if (adv[i]) w8(NAT(4) + (uint32_t)adv[i], 1);
            give_cal(4, 0x16, -1);
            set_rec(0, 0x16, -1, 4);
            g_a[0] = 0;
            run_both(R_23230, 1, "A5");
            expect(count_of("reduce 4 1 ") == want[i], "A5 Superstition count");
            expect(i != 3 || has("Enlightenment nullifies the effect."), "A5 text nullifies");
        }
        base_clean();
        put_city(10, 4, 1); put_city(11, 4, 1);
        set_rec(0, 0x16, -1, 4); give_cal(4, 0x16, -1);
        run_both(R_23230, 1, "A5 all");
        expect(S8(NAT(4) + 0x51) == 0 && has("All your cities have been reduced.") && !has("reduce 4"),
               "A5 two cities: all reduced, no screen");
        g_original = 1;
        base_clean(); g_original = 1;
        put_city(10, 4, 1); w8(NAT(4) + 0x39, 1);
        set_rec(0, 0x16, -1, 4); give_cal(4, 0x16, -1);
        run_both(R_23230, 1, "A5 original text");
        expect(has("Enlightenment nullifys the effect."), "A5 text nullifys (original)");
        g_original = 0;
    }

    /* A6 Slave Revolt: 15, 20 with Mining, -5 with Enlightenment, capped by the board */
    {
        static const int mi[4] = { 0, 1, 1, 0 }, en[4] = { 0, 0, 1, 1 }, bd[4] = { 30, 30, 30, 9 }, want[4] = { 15, 20, 15, 9 };
        for (i = 0; i < 4; i++) {
            base_clean();
            w8(NAT(2) + 0x29, mi[i]); w8(NAT(2) + 0x39, en[i]); w8(NAT(2) + 0x98, bd[i]);
            for (x = 0; x < 8; x++) put_city(20 + x, 2, 1);
            set_rec(0, 0x18, -1, 2); give_cal(2, 0x18, -1);
            g_a[0] = 0;
            run_both(R_235E4, 1, "A6");
            snprintf(e, sizeof e, "support 2 %d []", want[i]);
            expect(S16(0xF7AFCu) == want[i] && S16(0xF7AFEu) == 2 && has(e), "A6 loss and support");
        }
        base_clean();
        put_city(20, 2, 1); w8(NAT(2) + 0x98, 17);
        set_rec(0, 0x18, -1, 2); give_cal(2, 0x18, -1);
        run_both(R_235E4, 1, "A6 none");
        expect(has("AFRICA suffers a Slave Revolt") == 0 && has("IBERIA suffers a Slave Revolt and emerges without any "
               "losses.") && !has("support"), "A6 2 <= 17 - 15: no losses");
    }

    /* A7 Civil Disorder: cities - 3 - Music - D&P - Law - Democracy + Roadbuilding + Military */
    base_clean();
    for (x = 0; x < 8; x++) put_city(30 + x, 3, 1);
    w8(NAT(3) + 0x30, 1); w8(NAT(3) + 0x34, 1); w8(NAT(3) + 0x35, 1);
    set_rec(0, 0x1C, -1, 3); give_cal(3, 0x1C, -1);
    g_a[0] = 0;
    run_both(R_237A8, 1, "A7");
    expect(count_of("reduce 3 1 ") == 4 && has("You must reduce 4 cities.") && has("You must reduce 1 city."),
           "A7 8 - 3 - 2 + 1 = 4");
    base_clean();
    for (x = 0; x < 3; x++) put_city(30 + x, 3, 1);
    set_rec(0, 0x1C, -1, 3); give_cal(3, 0x1C, -1);
    run_both(R_237A8, 1, "A7 none");
    expect(has("ILLYRIA has been struck by Civil Disorder and is unaffected.") && !has("reduce"), "A7 3 cities: none");

    /* A8 Iconoclasm: primary 4 - Law - Philosophy (+ Roadbuilding), the secondary counts */
    base_clean();
    for (x = 0; x < 6; x++) put_city(40 + x, 1, 1);
    w8(NAT(1) + 0x33, 1); w8(NAT(1) + 0x28, 1);
    for (x = 0; x < 3; x++) put_city(50 + x, 2, 1);
    for (x = 0; x < 3; x++) put_city(60 + x, 3, 1);
    w8(NAT(3) + 0x36, 1);
    put_city(70, 4, 1); w8(NAT(4) + 0x3B, 1);
    put_city(71, 5, 1);
    for (x = 6; x <= 9; x++) w8(NAT(x) + 0x9A, 0);
    w8(NAT(5) + 0x9A, 3);
    set_rec(0, 0x1D, 5, 1); give_cal(1, 0x1D, 5);
    g_a[0] = 0;
    run_both(R_23ACC, 1, "A8");
    expect(count_of("reduce 1 1 ") == 4, "A8 primary 4 - 1 + 1");
    expect(S16(SECC + 4) == 2 && S16(SECC + 6) == 1 && S16(SECC + 8) == 0 && S16(SECC + 10) == 0,
           "A8 secondary 2 / 1 (Philosophy) / 0 (Theology) / 0 (trader)");
    expect(has("29550 1 -1 2 AFRICA, assign secondary losses for Iconoclasm"), "A8 total 3 > 2: the human assigns");
    expect(count_of("reduce 2 1 AFRICA") == 0 && count_of("reduce 3 1 ILLYRIA, as an Iconoclasm secondary victim") == 1 &&
           count_of("reduce 2 1 IBERIA, as an Iconoclasm secondary victim") == 1, "A8 the assigned losses reduced");

    /* A9 Treachery's article, a computer beneficiary (text only shown to computers) */
    {
        static const int bn[3] = { 1, 4, 6 };
        static const char *const fixed[3] = { "replaced by an African city.", "replaced by a Thracian city.",
                                              "replaced by an Asian city." };
        static const char *const orig[3] = { "replaced by a African city.", "replaced by an Thracian city.",
                                             "replaced by a Asian city." };
        for (i = 0; i < 6; i++) {
            base_clean();
            g_original = i >= 3;
            w8(NAT(bn[i % 3]) + 0x9A, 2);
            put_city(80, 9, 1); put_city(81, 9, 1);
            set_rec(0, 0x14, bn[i % 3], 9); give_cal(9, 0x14, bn[i % 3]);
            g_a[0] = 0;
            run_both(R_20564, 1, "A9");
            expect(has(g_original ? orig[i % 3] : fixed[i % 3]), "A9 the article");
            expect(has("A Egyptian city in") == 0 && has("An Egyptian city in"), "A9 0x21021 right in both");
            expect(D8(AR(82) + 0x3B + 9) == 1 && D8(TS(81)) == bn[i % 3], "A9 the city changes owner, mark 1");
        }
        g_original = 0;
    }

    /* A10 Piracy: the Pirates full */
    base_clean();
    for (x = 0; x < 9; x++) put_city(90 + x, 10, 1);
    put_city(100, 7, 1);
    set_rec(0, 0x1E, -1, 7); give_cal(7, 0x1E, -1);
    g_a[0] = 0;
    run_both(R_241D8, 1, "A10");
    expect(has("msg -500 7 -200 The management of Skull & Crossbones, Inc. regret to inform you that they are unable "
               "to add new cities to their portfolio.  ASSYRIA is spared from Piracy. 1"), "A10 Skull & Crossbones");
    /* A10b no coastal cities: the text fix */
    base_clean();
    put_city(100, 7, 1);
    set_rec(0, 0x1E, -1, 7); give_cal(7, 0x1E, -1);
    run_both(R_241D8, 1, "A10b");
    expect(has("no coastal cities for the Pirates to take over.") && !has("takeover"), "A10b take over");

    /* A11 0x2468C mode 1, one secondary nation with two coastal cities, take 2 */
    for (i = 0; i < 2; i++) {
        base_clean();
        g_original = i;
        for (x = 3; x <= 9; x++) w8(NAT(x) + 0x9A, 0);
        w8(NAT(3) + 0x9A, 3);
        w8(TR0(110), rb(TR0(110)) | 2); w8(TR0(111), rb(TR0(111)) | 2);
        put_city(110, 3, 1); put_city(111, 3, 1);
        g_a[0] = 2; g_a[1] = 1; g_a[2] = 2; g_a[3] = 1;   /* c = 2 (trader), v = 1 */
        /* only enabled buttons: fixed, FINISHED after the first REMOVE; --original, REMOVE twice (the second on -1) */
        g_nforced = i ? 3 : 2;
        g_forced[0] = -0x65; g_forced[1] = i ? -0x65 : -1; g_forced[2] = -1;
        run_both(R_2468C, 1, i ? "A11 original" : "A11");
        if (!i) {
            expect(S8(NAT(3) + 0x51) == 1 && S8(NAT(10) + 0x51) == 1 && D8(TS(110)) == 10,
                   "A11 one city to the Pirates (fixed)");
            expect(has("text 310 389 19 0") && count_of("click F596C 1 ") == 2 && !has("78F04 -1"),
                   "A11 left 0 after the first REMOVE, FINISHED");
        } else {
            expect(has("78F04 -1 ") && has("text 265 409 110 PIRACY") && has("border -1 15"),
                   "A11 REMOVE on entry -1, \"PIRACY\" (original)");
        }
    }
    g_original = 0;
    g_nforced = 0;

    /* A12 RESTART restores the marks of mode 1 (fixed); --original leaves nation 3 unmarked after the RESTART */
    for (i = 0; i < 2; i++) {
        base_clean();
        g_original = i;
        for (x = 4; x <= 9; x++) w8(NAT(x) + 0x9A, 0);
        w8(TR0(110), rb(TR0(110)) | 2); w8(TR0(120), rb(TR0(120)) | 2);
        put_city(110, 3, 1); put_city(120, 4, 1); w8(NAT(4) + 0x9A, 3);
        w8(0xF097Cu, rb(0xF097Cu) | 8);
        g_a[0] = 1; g_a[1] = 1; g_a[2] = 2; g_a[3] = 1;
        g_nforced = 5;
        g_forced[0] = -0x65; g_forced[1] = -0x63; g_forced[2] = -0x65; g_forced[3] = -0x65; g_forced[4] = -1;
        g_yes = -0x61;
        run_both(R_2468C, 1, i ? "A12 original" : "A12");
        if (!i)
            expect(S8(NAT(3) + 0x51) == 0 && S8(NAT(4) + 0x51) == 0 && S8(NAT(10) + 0x51) == 2 &&
                   has("yesno Are you sure you want to Restart? -97") && !has("78F04 -1"),
                   "A12 after RESTART both taken (fixed)");
        else
            expect(S8(NAT(3) + 0x51) == 1 && S8(NAT(4) + 0x51) == 0 && has("78F04 -1"),
                   "A12 after RESTART nation 3 not offered, REMOVE on -1 (original)");
    }
    g_original = 0;
    g_nforced = 0;
    g_yes = -1;

    /* A13 0x28CFC kinds */
    base_clean();
    put_city(5, 2, 1); put_tokens(6, 2, 1); put_tokens(7, 2, 2); w8(AR(9) + 0x24 + 2, 1);
    w8(0xF7984u + 11, 1); w8(0xF7A24u + 12, 1);
    g_a[0] = 2; g_a[2] = 0;
    {
        static const int kinds[9] = { 0x15, 0x1B, 6, 3, 2, 7, 8, 0x13, 0x1E };
        static const int want[9][4] = { { 3, 5, 6, 7 }, { 2, 5, 7, -1 }, { 2, 6, 7, -1 }, { 1, 8, -1, -1 },
                                        { 1, 5, -1, -1 }, { 1, 11, -1, -1 }, { 1, 12, -1, -1 }, { 0, -1, -1, -1 },
                                        { 0, -1, -1, -1 } };
        for (i = 0; i < 9; i++) {
            g_a[1] = kinds[i];
            run_both(R_28CFC, 1, "A13");
            expect(S32(LIST_N) == want[i][0] && S32(LIST) == want[i][1] && S32(LIST + 4) == want[i][2] &&
                   S32(LIST + 8) == want[i][3] && S32(LIST + 4 * 156) == -1, "A13 kind");
        }
        w8(MARK + 2, 1); w8(TR0(5), rb(TR0(5)) | 2);
        g_a[1] = 0x1E;
        run_both(R_28CFC, 1, "A13 1E");
        expect(S32(LIST_N) == 1 && S32(LIST) == 5, "A13 kind 0x1E: a marked coastal city");
        /* river 4 (the table's fifth row: 111, 115, 112, 113, 114) */
        put_tokens(115, 2, 1); put_city(113, 2, 1); w8(TR1(113), rb(TR1(113)) | 0x80); put_city(112, 2, 1);
        w8(TR1(112), rb(TR1(112)) & 0x7F);
        g_a[1] = 0x19; g_a[2] = 4;
        run_both(R_28CFC, 1, "A13 19");
        expect(S32(LIST_N) == 2 && S32(LIST) == 115 && S32(LIST + 4) == 113, "A13 kind 0x19: tokens, flood-plain city");
    }

    /* A14 Iconoclasm's computer text: no space before the period (fixed) */
    base_clean();
    put_city(40, 1, 1); put_city(50, 2, 1); w8(NAT(2) + 0x9A, 2);
    for (x = 3; x <= 9; x++) w8(NAT(x) + 0x9A, 0);
    w8(NAT(1) + 0x3B, 1);
    set_rec(0, 0x1D, -1, 1); give_cal(1, 0x1D, -1);
    g_a[0] = 0;
    run_both(R_23ACC, 1, "A14");
    snprintf(e, sizeof e, "msg 2500 2 50 IBERIA reduces a city in %s. 1", tn(50));
    expect(has(e), "A14 \"in %s.\"");
    g_original = 1;
    base_clean(); g_original = 1;
    put_city(40, 1, 1); put_city(50, 2, 1); w8(NAT(2) + 0x9A, 2);
    for (x = 3; x <= 9; x++) w8(NAT(x) + 0x9A, 0);
    w8(NAT(1) + 0x3B, 1);
    set_rec(0, 0x1D, -1, 1); give_cal(1, 0x1D, -1);
    run_both(R_23ACC, 1, "A14 original");
    snprintf(e, sizeof e, "msg 2500 2 50 IBERIA reduces a city in %s . 1", tn(50));
    expect(has(e), "A14 \"in %s .\" (original)");
    g_original = 0;

    printf("A: hand vectors, %d failed\n", g_fail - f0);
}

/* ---- B..G. random states ---------------------------------------------------------------------------------------- */

static void part_b(void)
{
    static const int kinds[13] = { 0x15, 4, 9, 0x1B, 7, 8, 6, 3, 2, 5, 0x1E, 0x19, 0x11 };
    int i, f0 = g_fail;
    char what[64];
    for (i = 0; i < 20000; i++) {
        if (i % 50 == 0) {
            memcpy(R, g_pristine, DS_SIZE);
            random_world(0);
        }
        g_ds = R;
        g_a[0] = 1 + (int)rnd(11);
        g_a[1] = kinds[rnd(13)];
        g_a[2] = (int)rnd(5);
        if (g_a[1] == 0x1E)
            for (int m = 0; m < 12; m++) w8(MARK + (uint32_t)m, rnd(3) == 0);
        g_original = i & 1;
        snprintf(what, sizeof what, "B %d kind %X", i, (unsigned)g_a[1]);
        run_both(R_28CFC, (uint32_t)i, what);
    }
    g_original = 0;
    printf("B: 0x28CFC, 20000 runs, %d failed\n", g_fail - f0);
}

static void part_c(void)
{
    int i, f0 = g_fail, n, j, which;
    char what[64];
    for (i = 0; i < 10000; i++) {
        memcpy(R, g_pristine, DS_SIZE);
        random_world(0);
        random_hands(85);
        for (n = 1; n <= 9; n++)   /* now and then more cards, also repeated ids, so Zeus has work */
            if (rnd(3) == 0)
                for (j = (int)rnd(5); j > 0; j--)
                    give_cal(n, 0x13 + (int)rnd(12), rnd(2) ? -1 : 1 + (int)rnd(9));
        g_original = i & 1;
        which = i % 3 == 0 ? R_1E388 : i % 3 == 1 ? R_1E5B8 : R_1E69C;
        if (which == R_1E69C) {
            g_ds = R;
            f1E5B8();
            g_a[0] = S32(CCOUNT) > 0 ? (int)rnd((uint32_t)S32(CCOUNT)) : 0;
            if (S32(CCOUNT) == 0 || rnd(10) == 0)
                set_rec(g_a[0], 0x13 + (int)rnd(12), -1, 1 + (int)rnd(9));   /* a card perhaps not held */
        }
        snprintf(what, sizeof what, "C %d", i);
        run_both(which, (uint32_t)i, what);
        if (which == R_1E388)
            g_cov[6] += count_of("rand ");
    }
    g_original = 0;
    printf("C: Zeus / 0x1E5B8 / 0x1E69C, 10000 runs, %d failed\n", g_fail - f0);
}

static void part_d(void)
{
    static const int ids[6] = { 0x14, 0x16, 0x18, 0x1C, 0x1D, 0x1E };
    static const int rs[6] = { R_20564, R_23230, R_235E4, R_237A8, R_23ACC, R_241D8 };
    int i, f0 = g_fail, c, v, g, k;
    char what[64];
    for (i = 0; i < 18000; i++) {
        c = i % 6;
        memcpy(R, g_pristine, DS_SIZE);
        random_world(0);
        random_hands(40);
        g_ds = R;
        v = 1 + (int)rnd(9);
        if (rb(NAT(v) + 0x9A) == 0)
            w8(NAT(v) + 0x9A, rnd(3) ? 3 : 2);
        g = rnd(3) == 0 ? -1 : 1 + (int)rnd(9);
        if (g == v || (g > 0 && rb(NAT(g) + 0x9A) == 0))
            g = -1;
        give_cal(v, ids[c], g);
        k = (int)rnd(4);
        set_rec(k, ids[c], g, v);
        w32(CCOUNT, k + 1);
        g_a[0] = k;
        if (ids[c] == 0x1E && rnd(4) == 0)   /* the Pirates near full */
            while (sb(NAT(10) + 0x51) < 8 && put_city((int)rnd(157), 10, 1) >= 0)
                ;
        g_original = (i / 6) & 1;
        snprintf(what, sizeof what, "D %d id %X", i, (unsigned)ids[c]);
        run_both(rs[c], (uint32_t)i, what);
    }
    g_original = 0;
    printf("D: the six calamities, 18000 runs, %d failed\n", g_fail - f0);
}

static void part_e(void)
{
    int i, f0 = g_fail, c, v, mode, k, m;
    char what[64];
    for (i = 0; i < 6000; i++) {
        memcpy(R, g_pristine, DS_SIZE);
        random_world(0);
        g_ds = R;
        mode = i % 3;
        v = 1 + (int)rnd(9);
        c = 1 + (int)rnd(9);
        w8(NAT(v) + 0x9A, 3);
        if (mode == 1 && rnd(2) == 0)
            c = v;
        if (mode == 2)
            k = sb(NAT(v) + 0x51) > 0 ? 1 + (int)rnd((uint32_t)sb(NAT(v) + 0x51)) : 0;
        else if (mode == 0)
            k = 1 + (int)rnd(2);
        else {
            for (k = 0, m = 1; m <= 9; m++)
                if (rb(NAT(m) + 0x9A) && m != v && m != c)
                    for (int j = 0; j < sb(NAT(m) + 0x51); j++)
                        k += (rb(TR0(rb(NAT(m) + 0x52u + (uint32_t)j) - 1)) & 2) != 0;
            k = k > 2 ? 2 : k;
        }
        g_a[0] = c; g_a[1] = v; g_a[2] = k; g_a[3] = mode;
        g_original = (i / 3) & 1;
        snprintf(what, sizeof what, "E %d mode %d", i, mode);
        run_both(R_2468C, (uint32_t)i, what);
    }
    g_original = 0;
    printf("E: 0x2468C, 6000 runs, %d failed\n", g_fail - f0);
}

static void part_f(void)
{
    int i, f0 = g_fail;
    char what[64];
    for (i = 0; i < 4000; i++) {
        memcpy(R, g_pristine, DS_SIZE);
        random_world(0);
        random_hands(i % 4 == 0 ? 100 : 45);
        g_original = i & 1;
        snprintf(what, sizeof what, "F %d", i);
        run_both(R_1E0A0, (uint32_t)i, what);
    }
    g_original = 0;
    printf("F: 0x1E0A0, 4000 runs, %d failed\n", g_fail - f0);
}

static void part_g(const char *savedir, const char *gamedir)
{
    static const char *const SAVES[4] = { "SAVGAMEA.DAT", "savgameb.dat", "savgame0.dat", "savgame1.dat" };
    uint8_t *base = malloc(DS_SIZE);
    char path[1024], what[64];
    int s, i, n, f0 = g_fail;

    for (s = 0; s < 4 && base != NULL; s++) {
        snprintf(path, sizeof path, "%s/%s", s < 2 ? savedir : gamedir, SAVES[s]);
        if (!load_save(path)) {
            printf("FAIL G: cannot read %s\n", path);
            g_fail++;
            continue;
        }
        memcpy(base, R, DS_SIZE);
        g_ds = R;
        printf("G: %s round %d, players", SAVES[s], S16(0xF40FCu));
        for (n = 1; n <= 9; n++)
            printf(" %d", D8(NAT(n) + 0x9A));
        printf(", cities");
        for (n = 1; n <= 10; n++)
            printf(" %d", S8(NAT(n) + 0x51));
        printf(", calamities held");
        for (n = 1; n <= 9; n++)
            printf(" %d", S8(NAT(n) + 0x13));
        printf("\n");
        for (i = 0; i < 300; i++) {
            memcpy(R, base, DS_SIZE);
            g_ds = R;
            if (i > 0) {
                random_hands(i % 3 == 0 ? 100 : 50);
                for (n = 1; n <= 9; n++)
                    if (rb(NAT(n) + 0x9A) != 0 && rnd(4) == 0)
                        w8(NAT(n) + 0x9A, rnd(2) ? 3 : 2);
                w8(0xF4102u, 15);
                w8(0xF0980u, (rb(0xF0980u) & ~4) | (rnd(2) ? 4 : 0));
            }
            g_original = i & 1;
            snprintf(what, sizeof what, "G %s %d", SAVES[s], i);
            run_both(R_1E0A0, 700000u + (uint32_t)i, what);
        }
    }
    g_original = 0;
    free(base);
    printf("G: the real saves, 1200 runs, %d failed\n", g_fail - f0);
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
    part_e();
    part_f();
    part_g(savedir, gamedir);

    printf("coverage (port side): 2468C REMOVE %ld, RESTART %ld, 207B4 picks %ld, REDUCE CITY %ld, 0x213C0 %ld, "
           "Zeus draws %ld, other handlers %ld; not_ported %d, cut %d\n",
           g_cov[0], g_cov[1], g_cov[2], g_cov[3], g_cov[4], g_cov[6], g_cov[7], g_np, g_cuts);
    printf("%d checks, %d failed\n", g_checks, g_fail);
    return g_fail == 0 ? 0 : 1;
}
