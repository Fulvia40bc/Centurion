/* cards_test [GAMEDIR [PLAYDIR]]: unit checks for module CARDS (cards.c) that need no game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init; default C:\work\advciv_run, only read) and the saves
   savgame0.dat / savgame1.dat (R4, savgame1 all human); PLAYDIR holds SAVGAMEA.DAT and savgameb.dat (default
   C:\work\advciv_play, only read). The four are read with saveload.c's f5F05C as the game does.

   cards.c is built with -DCARDS_TEST: its four calls into code that is not ported (0x6DBAC, 0x6DE04, 0x6EDA8, PBEM's
   0x6E158) and the PBEM-only blocks come to recorders here. 0x6DBAC and 0x6EDA8 only write a trace line; 0x6DE04
   acts out a plausible computer (buys from stack 0 for 2 while the treasury holds 3, so the caller's own test of 6
   decides when it stops; returns (1 << 8) + 1, else 0);
   0x6E158 and the PBEM blocks stop the run (the port's not_ported) with a trace line.

   Every routine cards.c calls is a stub below that appends one line to a trace (drawing included). Those with data
   effects apply them: the cards (f76E50, f771F8, f6E460, f76F14, f195D8 as rules.h / info.h describe them), the
   shuffle (f76F70, f77184 and game_rand f78AE4 on the seed [0xCAC5C], as game.h describes them), the current dialog
   [0xF8E78], the player-button bytes 0xF06C0 of f783B8; the button states (enabled, label, x, y, style) live in a
   side table, compared like the data. f6DB00 is formulas.c's (linked). Clicks (f78BA4) come from a script: a seeded
   choice among the codes the dialog offers (only enabled buttons; the mouse x for the column / hand hit tests), -100
   and codes that do nothing; after a step budget the choice ends the screen. A run that loops on is cut after 3,000
   clicks ("ABORT", the same in both).

   The model is a second implementation of the eighteen routines written from the disassembly (docs: the listing,
   not cards.c), on the same data object and stubs. Each check runs the port, keeps its data object, trace and side
   table, restores the start state, runs the model, and compares all three: the whole data object byte for byte (the
   seed included), the traces line for line, the button table. Parts:
     A. hand vectors (expected values written out): 0x77510; 0x77278 on one pile (the seed advanced once); the BUY
        variant's hit test (between columns: stack -100 and treasury - 58 with --original, ignored when fixed; the
        unaffordable column: treasury -1 with --original); the empty stack 8 (paid 18 with --original); CANCEL and
        +0x3C; the discard hit test on a known hand; the empty messages.
     B. 0x77278 on 20,000 random pile / stack states (each with and without --original).
     C. 0x6BD98 on 3,000 random states (players, cities, hands, stacks, treasury, variants 0x20 / 0x40, humans and
        computers, buy codes, PBEM), each with and without --original.
     D. 0x6E770 on 3,000 random states (hands over 8, calamities only, computers, PBEM).
     E. 0x6FBDC, 0x6FE68, 0x6FFC4, 0x71D30, 0x71E34, 0x6CDA0, 0x6CF50, 0x6D010, 0x6D0C4 called directly, 4,000 calls.
     F. 0x6BD98 then 0x6E770 on the four real saves, 100 click scripts each, both modes.
   Prints each failure (the first differing trace line or byte), the counts per part, and exits 0 when all pass.

   Build: clang -std=c11 -DCARDS_TEST port/test/cards_test.c port/src/cards.c port/src/formulas.c port/src/saveload.c
          port/src/state.c port/src/le_image.c -Iport/src -o cards_test.exe */

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cards.h"
#include "fg.h"
#include "formulas.h"
#include "game.h"
#include "info.h"
#include "junk.h"
#include "kit.h"
#include "loop.h"
#include "panel.h"
#include "rules.h"
#include "saveload.h"
#include "state.h"
#include "ui.h"

/* ---- addresses (from the listing, written out here) --------------------------------------------------------------- */

#define NAT(n)     (0xF14CCu + 199u * (uint32_t)(n))
#define STK(s)     (0xF06D0u + 25u * (uint32_t)(s))
#define PIL(s)     (0xF07B4u + 25u * (uint32_t)(s))
#define DLG_ACQ    0x1106D8u
#define DLG_DISC   0x110ADCu
#define DLG_BOX    0xBBBECu
#define SEED       0xCAC5Cu

/* ---- traces --------------------------------------------------------------------------------------------------- */

#define LOGSZ (1u << 21)
static char *g_log[2];
static size_t g_len[2];
static int g_cur;          /* 0 the port's run, 1 the model's */

static void tr(const char *fmt, ...)
{
    va_list ap;
    char *log = g_log[g_cur];
    size_t *len = &g_len[g_cur];
    int k;

    if (*len + 400 >= LOGSZ)
        return;
    va_start(ap, fmt);
    k = vsnprintf(log + *len, LOGSZ - *len, fmt, ap);
    va_end(ap);
    if (k > 0)
        *len += (size_t)k;
    log[(*len)++] = '\n';
    log[*len] = 0;
}

/* ---- the side table: button states per dialog ------------------------------------------------------------------- */

typedef struct {
    int8_t   en[3][256];
    uint32_t label[3][256];
    int16_t  x[3][256], y[3][256];
    int8_t   style[3][256];
    int8_t   saved[3][256];
} Buttons;
static Buttons g_btn, g_btn_saved[2];

static int dix(uint32_t dlg)
{
    return dlg == DLG_ACQ ? 0 : dlg == DLG_DISC ? 1 : 2;
}

/* ---- the click script ----------------------------------------------------------------------------------------- */

static uint32_t g_srng, g_srng0;
static int g_steps;
static jmp_buf g_abort;
static int g_budget = 40;

static uint32_t srnd(uint32_t n)
{
    g_srng ^= g_srng << 13;
    g_srng ^= g_srng >> 17;
    g_srng ^= g_srng << 5;
    return n ? g_srng % n : 0;
}

static void stop_run(const char *why)
{
    tr("ABORT %s", why);
    longjmp(g_abort, 1);
}

static int pick(uint32_t dlg)
{
    int d = dix(dlg), k;
    int over = ++g_steps > g_budget;

    if (g_steps > 3000)
        stop_run("steps");
    if (d == 2) {                                  /* REVEAL GOODS: CONTINUE */
        if (over || srnd(3) == 0)
            return -1;
        return srnd(2) ? -100 : -5;
    }
    if (d == 1) {                                  /* the discard screen: the hand (0x8D) */
        if (over) {
            setD32(0xE1628u, 0x33);                /* the first card held */
            return -0x35;
        }
        k = (int)srnd(6);
        if (k == 0)
            return -100;
        if (k == 1)
            return srnd(2) ? -5 : 3;
        setD32(0xE1628u, (uint32_t)(0x20 + (int)srnd(0x32 + 40 * 19)));
        return -0x35;
    }
    /* the acquire dialog */
    if (g_btn.en[0][0x8C] == 1) {                  /* the stack choice: a column or CANCEL */
        if (over || srnd(5) == 0)
            return -1;
        if (srnd(6) == 0)
            return -100;
        setD32(0xE1628u, (uint32_t)(0xD0 + (int)srnd(9 * 39 + 0x40)));
        return -0x34;
    }
    if (over) {
        if (g_btn.en[0][0x2A] == 1)
            return -1;
        if (g_btn.en[0][0x8A] == 1)
            return -0x32;
        return -1;
    }
    for (;;) {
        k = (int)srnd(8);
        if (k == 0)
            return -100;
        if (k == 1)
            return srnd(2) ? -5 : -0x35;
        if (k <= 3 && g_btn.en[0][0x8A] == 1)
            return -0x32;
        if (k <= 5 && g_btn.en[0][0x8B] == 1)
            return -0x33;
        if (k == 6 && g_btn.en[0][0x2A] == 1)
            return -1;
        if (k == 7)
            return -100;
    }
}

/* ---- stubs: drawing and screens (trace only) ---------------------------------------------------------------------- */

int  fg_vbopen(int h) { tr("vbopen %d", h); return 0; }
void fg_vbclose(void) { tr("vbclose"); }
int  fg_vbcopy(int a, int b, int c, int d, int e, int f, int s, int t)
{
    tr("vbcopy %d %d %d %d %d %d %d %d", a, b, c, d, e, f, s, t);
    return 0;
}
int  fg_vbpaste(int a, int b, int c, int d, int e, int f) { tr("vbpaste %d %d %d %d %d %d", a, b, c, d, e, f); return 0; }
int  fg_setcolor(int c) { tr("color %d", c); return 0; }
void fg_move(int x, int y) { tr("move %d %d", x, y); }
void f39F90(int f) { tr("font %d", f); }
void f3A0B0(int x, int y, int w, const char *s) { tr("center %d %d %d '%s'", x, y, w, s); }
void f3A044(int x, int y, const char *s) { tr("print %d %d '%s'", x, y, s); }
void f3A18C(int x, int y, int w, int c, const char *s) { tr("title %d %d %d %d '%s'", x, y, w, c, s); }
int  f3A42C(int x, int y, int w, uint32_t al, const char *t, int32_t *maxw, int draw)
{
    tr("wrap %d %d %d %u '%s' %d", x, y, w, (unsigned)al, t, draw);
    *maxw = 0x22;
    return y + 12;
}
void f3AF8C(uint32_t spr, int x, int y) { tr("sprite %X %d %d", (unsigned)spr, x, y); }
void f3CD6C(int8_t pic) { tr("picture %d", pic); }
void f3CC14(void) { tr("showback"); }
void f3CCBC(void) { tr("screen2back"); }
void f3B05C(uint32_t dlg) { tr("drawdlg %X", (unsigned)dlg); setD32(0xF8E78u, dlg); }
void f3C9D0(int x, int y, int w, int h, int f) { tr("frame %d %d %d %d %d", x, y, w, h, f); }
void f78884(void) { tr("hide"); }
void f788AC(void) { tr("unhide"); }
void f78A20(int t) { tr("wait %d", t); }
void f79270(int wait, int n, int t, const char *text, int anim) { tr("msg %d %d %d '%s' %d", wait, n, t, text, anim); }
void f7983C(const char *text) { tr("box '%s'", text); }
void f553C0(void) { tr("music"); }
void f71F2C(int x, int y, int w, int n) { tr("hand_down %d %d %d %d", x, y, w, n); }
void f71FB0(int x, int y, int w, int n) { tr("hand_up %d %d %d %d", x, y, w, n); }
void f194E4(int n) { tr("mining %d", n); }
int  junk_makegif(int a, int b, int c, int d, const char *name) { tr("makegif %d %d %d %d %s", a, b, c, d, name); return 0; }
int  junk_showgif(const char *name, int f) { tr("showgif %s %d", name, f); return 0; }

/* ---- stubs: buttons --------------------------------------------------------------------------------------------- */

/* part A2's fixed click lists (3 entries each) take precedence over the picker */
static const int *g_fixed;
static int g_fixed_x[3];
static int g_fixed_i;

int  f78BA4(uint32_t dlg, int map)
{
    int r;
    if (g_fixed != NULL) {
        if (g_fixed_i >= 3)
            stop_run("fixed");
        r = g_fixed[g_fixed_i];
        setD32(0xE1628u, (uint32_t)g_fixed_x[g_fixed_i]);
        g_fixed_i++;
    } else {
        r = pick(dlg);
    }
    tr("click %X %d -> %d x %d", (unsigned)dlg, map, r, S32(0xE1628u));
    return r;
}
void f3BB48(uint32_t dlg, int id, int x) { tr("btn_x %X %d %d", (unsigned)dlg, id, x); g_btn.x[dix(dlg)][id & 0xFF] = (int16_t)x; }
void f3BC64(uint32_t dlg, int id, int y) { tr("btn_y %X %d %d", (unsigned)dlg, id, y); g_btn.y[dix(dlg)][id & 0xFF] = (int16_t)y; }
void f3C284(uint32_t dlg, int id, uint32_t l) { tr("label %X %d %X", (unsigned)dlg, id, (unsigned)l); g_btn.label[dix(dlg)][id & 0xFF] = l; }
void f3C0D8(uint32_t dlg, int id, int s) { tr("style %X %d %d", (unsigned)dlg, id, s); g_btn.style[dix(dlg)][id & 0xFF] = (int8_t)s; }
void f3BFBC(uint32_t dlg, int id, int e) { tr("set_en %X %d %d", (unsigned)dlg, id, e); g_btn.en[dix(dlg)][id & 0xFF] = (int8_t)e; }
void f3C48C(uint32_t dlg, int id) { tr("enable %X %d", (unsigned)dlg, id); g_btn.en[dix(dlg)][id & 0xFF] = 1; }
void f3C530(uint32_t dlg, int id) { tr("disable %X %d", (unsigned)dlg, id); g_btn.en[dix(dlg)][id & 0xFF] = 0; }
void f3C940(uint32_t dlg)
{
    int d = dix(dlg), i;
    tr("dlg_off %X", (unsigned)dlg);
    for (i = 0; i < 256; i++) {
        g_btn.saved[d][i] = g_btn.en[d][i];
        g_btn.en[d][i] = 0;
    }
}
void f3C8C4(uint32_t dlg)
{
    int d = dix(dlg);
    tr("dlg_on %X", (unsigned)dlg);
    memcpy(g_btn.en[d], g_btn.saved[d], 256);
}
void f783B8(uint32_t dlg, int base)
{
    int p;
    tr("players %X %d", (unsigned)dlg, base);
    for (p = 0; p < S32(0xF40ECu) && p < 16; p++) {
        int n = S16(0xF06B0u + 2u * (uint32_t)p);
        if (n >= 0 && n < 14)
            setD8(0xF06C0u + (uint32_t)n, (uint8_t)p);
    }
}

/* ---- stubs: the cards (data effects, from rules.h / info.h / game.h) ------------------------------------------------ */

int f78AE4(void)
{
    uint32_t s = D32(SEED) * 0x15A4E35u + 1u;
    setD32(SEED, s);
    return (int)((s >> 16) & 0x7FFF);
}

void f76F70(int n, uint8_t *cards)
{
    uint8_t tmp[160];
    int i, k, m = n;
    tr("shuffle %d", n);
    if (n <= 0 || n > 160)
        return;
    memcpy(tmp, cards, (size_t)n);
    for (i = 0; i < n; i++) {
        k = (int)((uint32_t)f78AE4() * (uint32_t)m >> 15);
        cards[i] = tmp[k];
        memmove(tmp + k, tmp + k + 1, (size_t)(m - k - 1));
        m--;
    }
}

void f77184(int n, const uint8_t *src, int len, uint8_t *dst)
{
    int i;
    uint8_t s[32];
    tr("insert %d %d %d", n, len, n > 0 ? src[0] : -1);
    if (n < 0 || n > 32)
        return;
    memcpy(s, src, (size_t)n);
    for (i = len - 1; i >= 0; i--)
        dst[i + n] = dst[i];
    for (i = 0; i < n; i++)
        dst[i] = s[i];
}

int f76E50(int s, int n)
{
    uint32_t st = 0xF06D0u + (uint32_t)(25 * s);
    int cnt = S16(st), c;
    uint32_t rec = NAT(n);

    if (cnt == 0) {
        tr("draw %d %d -> 0", s, n);
        return 0;
    }
    c = S8(st + 5u + (uint32_t)cnt);
    setD16(st, (uint16_t)(cnt - 1));
    if (c > 18) {
        int k = S8(rec + 0x13);
        setD8(rec + 0x14u + (uint32_t)k, (uint8_t)c);
        setD8(rec + 0x13, (uint8_t)(k + 1));
    } else {
        setD8(rec + (uint32_t)c, (uint8_t)(D8(rec + (uint32_t)c) + 1u));
    }
    tr("draw %d %d -> %d", s, n, c);
    return c;
}

void f771F8(int c)
{
    int s = c <= 18 ? (c - 1) / 2 : S8(0xCA9C1u + (uint32_t)c);
    int k = S16(PIL(s));
    tr("discard %d", c);
    setD8(PIL(s) + 6u + (uint32_t)k, (uint8_t)c);
    setD16(PIL(s), (uint16_t)(k + 1));
}

void f6E460(int n)
{
    uint32_t rec = NAT(n);
    int k = S8(rec + 0x13), i, j;
    tr("sortcal %d", n);
    for (i = 0; i < k && i < 8; i++)
        for (j = 0; j + 1 < k - i && j + 1 < 8; j++)
            if (S8(rec + 0x14u + (uint32_t)j) > S8(rec + 0x15u + (uint32_t)j)) {
                uint8_t a = D8(rec + 0x14u + (uint32_t)j), g = D8(rec + 0x1Cu + (uint32_t)j);
                setD8(rec + 0x14u + (uint32_t)j, D8(rec + 0x15u + (uint32_t)j));
                setD8(rec + 0x15u + (uint32_t)j, a);
                setD8(rec + 0x1Cu + (uint32_t)j, D8(rec + 0x1Du + (uint32_t)j));
                setD8(rec + 0x1Du + (uint32_t)j, g);
            }
}

int f76F14(int n)
{
    int i, t = 0;
    for (i = 1; i <= 0x13; i++)
        t += S8(NAT(n) + (uint32_t)i);
    return t;
}

int f195D8(int n)
{
    int i, t = 0, c;
    for (i = 1; i <= 18; i++) {
        c = S8(NAT(n) + (uint32_t)i);
        t += c * c * ((i + 1) >> 1);
    }
    tr("value %d -> %d", n, t);
    return t;
}

/* ---- link stubs for formulas.c and saveload.c (not reached by cards.c) ---------------------------------------------- */

int f78D08(int t, int n) { (void)t; (void)n; return 0; }

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

/* ---- recorders for the calls cards.c does not port --------------------------------------------------------------- */

void cards_test_f6DBAC(int p, int n) { tr("AI f6DBAC %d %d", p, n); }
int  cards_test_f6DE04(int n)
{
    uint32_t rec = NAT(n);
    tr("AI f6DE04 %d", n);
    if (S8(rec + 0x3E) >= 3 && S16(STK(0)) > 0) {
        f76E50(0, n);
        setD8(rec + 0x3E, (uint8_t)(D8(rec + 0x3E) - 2u));
        setD8(rec + 0x99, (uint8_t)(D8(rec + 0x99) + 2u));
        setD32(0xF590Cu + 4u * (uint32_t)n, 0);
        return (1 << 8) + 1;
    }
    return 0;
}
void cards_test_f6EDA8(int n) { tr("AI f6EDA8 %d", n); }
void cards_test_f6E158(void) { tr("PBEM f6E158"); longjmp(g_abort, 1); }
void cards_test_pbem(uint32_t a) { tr("PBEM %X", (unsigned)a); longjmp(g_abort, 1); }

/* ============================================================================================================== */
/* The model: a second implementation of the eighteen routines, from the disassembly.                            */
/* ============================================================================================================== */

static int8_t  ms8(uint32_t a) { return S8(a); }
static int16_t ms16(uint32_t a) { return S16(a); }
static int32_t ms32(uint32_t a) { return S32(a); }
static void    mw8(uint32_t a, int v) { setD8(a, (uint8_t)v); }
static void    mw16(uint32_t a, int v) { setD16(a, (uint16_t)v); }
static void    mw32(uint32_t a, int32_t v) { setD32(a, (uint32_t)v); }
static const char *mname(int n) { return DSTR(D32(0xC768Cu + 4u * (uint32_t)n)); }
static int mbg(void) { return ms32(0xF52CCu); }
static int mbk(void) { return ms32(0xF52C8u); }
static int mpbem(void) { return (D8(0xE1624u) & 0x80) != 0; }

static void m_71E34(int s)
{
    uint32_t r = s == -1 ? 0x11158Cu : 0x111524u + 8u * (uint32_t)s;
    int x = ms16(r), y = ms16(r + 2), w = ms16(r + 4), h = ms16(r + 6);
    fg_vbopen(mbg());
    fg_vbpaste(x, x + w - 1, y, y + h - 1, x, y + h - 1);
    fg_vbclose();
}

static void m_71D30(int s)
{
    uint32_t r = s == -1 ? 0x11158Cu : 0x111524u + 8u * (uint32_t)s;
    int x = ms16(r), y = ms16(r + 2), w = ms16(r + 4), h = ms16(r + 6);
    fg_vbcopy(x, x + w - 1, y, y + h - 1, x, y + h - 1, mbg(), mbk());
}

static void m_6FE68(int x, int y, int n, int show, int col)
{
    char b[20];
    f194E4(n);
    fg_vbcopy(x, x + 49, y - 10, y + 39, x, y + 39, mbg(), mbk());
    fg_vbopen(mbk());
    fg_setcolor(col);
    f39F90(1);
    f3A0B0(x, y - 10, 50, "POINTS");
    sprintf(b, "%d", f195D8(n));
    f3A0B0(x, y, 50, b);
    if (n) {
        f3AF8C(0xF49ECu + 8u * (uint32_t)n, x, y + 10);
        sprintf(b, "%d", ms8(NAT(n) + 0x3E));
        f3A044(x + 27, y + 20, b);
    }
    fg_vbclose();
    if (show == 1) {
        f78884();
        fg_vbpaste(x, x + 49, y - 10, y + 39, x, y + 39);
        f788AC();
    }
}

static void m_6FFC4(int x, int y, int show)
{
    fg_vbcopy(x, x + 49, y - 10, y + 39, x, y + 39, mbg(), mbk());
    if (show != 1)
        return;
    fg_vbopen(mbk());
    fg_vbclose();
    f78884();
    fg_vbpaste(x, x + 49, y - 10, y + 39, x, y + 39);
    f788AC();
}

static void m_6FBDC(int n, int dis)
{
    uint32_t keep = 0;
    if (mpbem() || f76F14(n) < 1 || ms8(0xE1620u) < 2)
        return;
    f78884();
    junk_makegif(245, 245 + 149, 190, 190 + 99, "junk");
    f788AC();
    if (dis == 1) {
        keep = D32(0xF8E78u);
        f3C940(keep);
    }
    fg_vbopen(mbk());
    f3C9D0(245, 190, 150, 100, 3);
    fg_setcolor(5);
    f3A0B0(245, 210, 150, mname(n));
    fg_vbclose();
    f3BB48(DLG_BOX, 42, 265);
    f3BC64(DLG_BOX, 42, 250);
    f3C284(DLG_BOX, 42, 0xB8FC8u);
    f3B05C(DLG_BOX);
    f78884();
    fg_vbpaste(245, 394, 190, 289, 245, 289);
    f788AC();
    while (f78BA4(DLG_BOX, 0) != -1)
        ;
    if (dis == 1) {
        mw32(0xF8E78u, (int32_t)keep);
        f3C8C4(keep);
    }
    f3C284(DLG_BOX, 42, 0xB8FD5u);
    f78884();
    fg_move(245, 190);
    junk_showgif("junk", 3);
    f3CCBC();
    f788AC();
}

static void m_strip(int x, int y, int w, int n, int up)
{
    fg_vbcopy(x, x + w - 1, y, y + 45, x, y + 45, mbg(), mbk());
    fg_vbopen(mbk());
    if (up)
        f71FB0(x, y, w, n);
    else
        f71F2C(x, y, w, n);
    fg_vbclose();
    f78884();
    fg_vbpaste(x, x + w - 1, y, y + 45, x, y + 45);
    f788AC();
}

static int m_sx(void) { return ms32(0xF40ECu) >= 8 ? 209 : 7; }
static int m_sw(void) { return ms32(0xF40ECu) >= 8 ? 370 : 500; }

static void m_6CF50(int n)
{
    char b[20];
    fg_vbopen(mbk());
    f3AF8C(0xF49ECu + 8u * (uint32_t)n, 515, 360);
    fg_setcolor(5);
    sprintf(b, "%d", ms8(NAT(n) + 0x3E));
    m_71E34(1);
    f3A044(546, 368, b);
    fg_vbclose();
    f78884();
    fg_vbpaste(508, 617, 354, 388, 508, 388);
    f788AC();
}

static void m_6CB8C(void)
{
    static const int16_t R[4][4] = { { 236, 98, 351, 112 }, { 546, 368, 30, 20 }, { 595, 295, 40, 20 },
                                     { 250, 27, 150, 25 } };
    int i, k;
    f3CD6C(ms8(0xEE168u));
    fg_vbcopy(0, 639, 0, 479, 0, 479, mbg(), mbk());
    fg_vbopen(mbk());
    mw16(0x11158Cu, ms32(0xF40ECu) >= 8 ? 209 : 7);
    mw16(0x11158Eu, 406);
    mw16(0x111590u, ms32(0xF40ECu) >= 8 ? 370 : 500);
    mw16(0x111592u, 46);
    for (i = 0; i < 4; i++)
        for (k = 0; k < 4; k++)
            mw16(0x111524u + 8u * (uint32_t)i + 2u * (uint32_t)k, R[i][k]);
    f3A18C(0, 8, 640, 176, "Acquire Goods");
    fg_setcolor(13);
    f3A0B0(58, 21, 32, "Stored");
    f3A0B0(95, 21, 32, "Acqrd");
    f3A0B0(132, 21, 32, "Buy");
    f3A0B0(169, 21, 32, "Total");
    fg_vbclose();
    f3B05C(DLG_ACQ);
    f3CC14();
}

static void m_6CDA0(int hl)
{
    int i, row = 0, n, cnt = ms16(0xF06AEu);
    char b[52];
    for (i = 0; i < cnt; i++) {
        n = ms16(0xF06B0u + 2u * (uint32_t)i);
        if (D8(NAT(n) + 0x9A) == 0)
            continue;
        f3AF8C(0xF4ACCu + 8u * (uint32_t)n, (53 - D16(0xF4AD0u + 8u * (uint32_t)n)) / 2, 49 * row + 43);
        f39F90(1);
        fg_setcolor(hl == i ? 15 : 5);
        f3A0B0(0, 49 * row + 43 + D16(0xF4AD2u + 8u * (uint32_t)n) + 2, 53, mname(n));
        fg_setcolor(hl == i ? 15 : 5);
        f3AF8C(0xF4D14u, 58, 49 * row + 36);
        sprintf(b, "%d", ms8(0x110AD0u + (uint32_t)i));
        f39F90(2);
        f3A0B0(59, 49 * row + 50, 30, b);
        row++;
    }
    f39F90(1);
}

static void m_6D178(int p, int n)
{
    uint32_t rec = NAT(n);
    int cities, s, j, c;
    char b[72];
    int32_t mw;

    f3CD6C(ms8(0xEE168u));
    cities = ms8(rec + 0x51);
    f78884();
    m_71E34(0);
    fg_setcolor(5);
    f3A044(246, 98, "You acquired...");
    f788AC();
    mw32(0x110AC8u, 0);
    fg_setcolor(15);
    for (s = 0; s < cities; s++)
        for (j = 0; j < ms32(0x110AC4u); j++) {
            c = (int8_t)f76E50(s, n);
            if (c) {
                f78884();
                f3AF8C(0xF4C1Cu + 8u * (uint32_t)c, 236 + 39 * s, 110 + 50 * j);
                f788AC();
                mw32(0x110AC8u, ms32(0x110AC8u) + 1);
            } else {
                sprintf(b, "Stack %d Empty", s + 1);
                f78884();
                f3A42C(236 + 39 * s, 115 + 50 * j, 34, 2, b, &mw, 1);
                f788AC();
            }
            f78A20(50);
        }
    f6E460(n);
    m_strip(m_sx(), 406, m_sw(), n, 1);
    m_6FE68(585, 415, n, 1, 15);
    f3AF8C(0xF4D14u, 95, 49 * p + 36);
    f39F90(2);
    sprintf(b, "%d", ms32(0x110AC8u));
    fg_setcolor(15);
    f3A0B0(96, 49 * p + 50, 30, b);
    f39F90(1);
    f78884();
    m_71E34(2);
    f788AC();
    f3C530(DLG_ACQ, 0x8A);
    c = ms16(0xF0684u + 2u * (uint32_t)n);
    if (c == -97) {
        int card = (int8_t)f76E50(8, n);
        if (card != 0 && f6DB00(n) != 0) {
            f7983C("View Good Bought");
            f3C530(DLG_ACQ, 0x8B);
            f78884();
            fg_setcolor(5);
            f3A044(246, 160, "You bought...");
            f788AC();
            fg_setcolor(15);
            mw32(0x110ACCu, ms32(0x110ACCu) + 1);
            f78884();
            f3AF8C(0xF4C1Cu + 8u * (uint32_t)card, 353, 160);
            f788AC();
            mw8(rec + 0x3C, D8(rec + 0x3C) + 1);
            mw8(rec + 0x3E, D8(rec + 0x3E) - 18);
            mw8(rec + 0x99, D8(rec + 0x99) + 18);
            mw32(0xF590Cu + 4u * (uint32_t)n, 8);
        }
    } else {
        if (c != -98 && f6DB00(n)) {
            f3C48C(DLG_ACQ, 0x8B);
            if (mpbem())
                mw16(0xF5478u, 1);
        } else {
            f3C530(DLG_ACQ, 0x8B);
        }
    }
    f3C48C(DLG_ACQ, 42);
}

static void m_6D564(int p, int n)
{
    uint32_t rec = NAT(n);
    int st = 0, lim, i, x, r, hit;
    int card;
    char b[80];
    int32_t mw;

    f3CD6C(ms8(0xEE168u));
    m_71E34(0);
    fg_setcolor(5);
    if ((D8(0xE1624u) & 0x20) == 0) {
        f78884();
        fg_setcolor(5);
        f3A044(246, 98, "You bought...");
        f788AC();
        fg_setcolor(15);
        st = 8;
        card = (int8_t)f76E50(8, n);
        if (card == 0 && !g_original) {
            card = -1;
        } else {
            mw32(0x110ACCu, ms32(0x110ACCu) + 1);
            f78884();
            f3AF8C(0xF4C1Cu + 8u * (uint32_t)card, 275, 110);
            f788AC();
            mw8(rec + 0x3C, D8(rec + 0x3C) + 1);
            if (mpbem())
                mw16(0xF5478u, 1);
            mw32(0xF590Cu + 4u * (uint32_t)n, 8);
        }
    } else {
        f78884();
        fg_setcolor(15);
        f3A044(246, 98, "Choose stack to buy from...");
        lim = 8;
        for (i = 0; i < 9; i++) {
            int t = ms8(rec + 0x3E), cnt = ms16(STK(i));
            if (cnt != 0 && t >= 2 * i + 2) {
                fg_setcolor(15);
                f39F90(2);
                f3AF8C(0xF4D14u, 236 + 39 * i, 110);
                sprintf(b, "%d", i + 1);
                f3A0B0(236 + 39 * i, 124, 32, b);
                f39F90(1);
            } else if (t < 2 * i + 2) {
                lim = i;
                break;
            } else {
                fg_setcolor(15);
                sprintf(b, "Stack %d Empty", i + 1);
                f3A42C(236 + 39 * i, 115, 34, 2, b, &mw, 1);
            }
        }
        f788AC();
        mw8(rec + 0x3C, D8(rec + 0x3C) + 1);
        if (mpbem())
            mw16(0xF5478u, 1);
        f3C48C(DLG_ACQ, 0x8C);
        f3C530(DLG_ACQ, 0x8B);
        f3C284(DLG_ACQ, 42, 0xB8CC3u);
        f3C48C(DLG_ACQ, 42);
        card = -1;
        for (;;) {
            r = f78BA4(DLG_ACQ, 0);
            if (r == -1) {
                if (!g_original)
                    mw8(rec + 0x3C, D8(rec + 0x3C) - 1);
                break;
            }
            if (r != -52)
                continue;
            hit = -100;
            for (i = 0, x = 236; i <= lim && hit == -100; i++, x += 39) {
                int mx = ms32(0xE1628u);
                if (ms16(STK(i)) == 0 || mx < x || mx > x + 32)
                    continue;
                if (!g_original && ms8(rec + 0x3E) < 2 * i + 2)
                    continue;
                hit = i;
            }
            if (hit == -100 && !g_original)
                continue;
            st = hit;
            card = (int8_t)f76E50(st, n);
            mw32(0xF590Cu + 4u * (uint32_t)n, st);
            break;
        }
        f3C530(DLG_ACQ, 0x8C);
        f78884();
        m_71E34(0);
        if (card != -1) {
            mw32(0x110ACCu, ms32(0x110ACCu) + 1);
            fg_setcolor(5);
            f3A044(246, 98, "You bought...");
            f3AF8C(0xF4C1Cu + 8u * (uint32_t)card, 275, 110);
        }
        f788AC();
    }
    if (card != -1) {
        int cost = ((st + 1) * 2) & 0xFF;
        mw8(rec + 0x3E, D8(rec + 0x3E) - cost);
        mw8(rec + 0x99, D8(rec + 0x99) + cost);
        m_6CF50(n);
        f6E460(n);
        m_strip(m_sx(), 406, m_sw(), n, 1);
        m_6FE68(585, 415, n, 1, 15);
        f3AF8C(0xF4D14u, 132, 49 * p + 36);
        f39F90(2);
        sprintf(b, "%d", ms32(0x110ACCu));
        fg_setcolor(15);
        f3A0B0(133, 49 * p + 50, 30, b);
        f39F90(1);
    }
    f78884();
    m_71E34(2);
    f788AC();
    if (f6DB00(n))
        f3C48C(DLG_ACQ, 0x8B);
    else
        f3C530(DLG_ACQ, 0x8B);
    f3C284(DLG_ACQ, 42, 0xB8CD8u);
    f3C48C(DLG_ACQ, 42);
}

static void m_counters(int p, int n)
{
    char b[100];
    int y = 49 * p;
    f39F90(2);
    fg_setcolor(5);
    f3AF8C(0xF4D14u, 95, y + 36);
    sprintf(b, "%d", ms32(0x110AC8u));
    f3A0B0(96, y + 50, 30, b);
    f3AF8C(0xF4D14u, 132, y + 36);
    sprintf(b, "%d", ms32(0x110ACCu));
    f3A0B0(133, y + 50, 30, b);
    f3AF8C(0xF4D14u, 169, y + 36);
    sprintf(b, "%d", f76F14(n));
    f3A0B0(170, y + 50, 30, b);
    f39F90(1);
}

static void m_6BD98(void)
{
    uint8_t c[9], na[9], t;
    int k = 0, sum = 0, n, i, j, p, cnt, human = 0, r;
    char b[100];
    uint32_t rec;

    memset(c, 0, sizeof c);
    memset(na, 0, sizeof na);
    for (n = 1; n <= 9; n++) {
        rec = NAT(n);
        if (D8(rec + 0x9A) == 0)
            continue;
        sum += ms8(rec + 0x51);
        c[k] = D8(rec + 0x51);
        na[k] = (uint8_t)n;
        k++;
    }
    if (sum == 0) {
        f79270(100, ms16(0xF06B0u), -200, "", 0);
        return;
    }
    f553C0();
    if (mpbem())
        cards_test_pbem(0x6BE7Au);
    cnt = ms32(0xF40ECu);
    for (i = cnt - 1; i >= 0; i--)
        for (j = 0; j < i; j++) {
            int less = (int8_t)c[i] < (int8_t)c[j] || ((int8_t)c[i] == (int8_t)c[j] && (int8_t)na[i] < (int8_t)na[j]);
            if (less) {
                t = c[i]; c[i] = c[j]; c[j] = t;
                t = na[i]; na[i] = na[j]; na[j] = t;
            }
        }
    mw16(0xF06AEu, cnt);
    for (i = 0; i < cnt; i++)
        mw16(0xF06B0u + 2u * (uint32_t)i, (int8_t)na[i]);
    f783B8(DLG_ACQ, ms32(0xF5960u));
    for (i = 0; i < ms16(0xF06AEu); i++)
        mw8(0x110AD0u + (uint32_t)i, f76F14(ms16(0xF06B0u + 2u * (uint32_t)i)));
    mw32(0x110AC4u, (D8(0xE1624u) & 0x40) ? 2 : 1);
    mw8(0xEE168u, ms32(0xF40ECu) >= 8 ? 11 : 10);
    m_6CB8C();
    for (p = 0; p < ms16(0xF06AEu); p++) {
        n = ms16(0xF06B0u + 2u * (uint32_t)p);
        rec = NAT(n);
        mw8(rec + 0x3C, 0);
        mw32(0xF40F8u, n);
        mw32(0xF590Cu + 4u * (uint32_t)n, -1);
        if (D8(rec + 0x9A) != 0 && ms8(rec + 0x51) == 0) {
            m_6CDA0(-1);
            fg_setcolor(5);
            f39F90(2);
            for (k = 1; k <= 3; k++) {
                f3AF8C(0xF4D14u, 58 + 37 * k, 49 * p + 36);
                if (k == 3) {
                    sprintf(b, "%d", f76F14(n));
                    f3A0B0(59 + 37 * k, 49 * p + 50, 30, b);
                } else {
                    f3A0B0(59 + 37 * k, 49 * p + 50, 30, "0");
                }
            }
            f39F90(1);
            f78A20(500);
            continue;
        }
        if (mpbem() ? ms8(0xE1622u) == n : ms8(rec + 0x9A) == 3) {
            human = 1;
            f3C530(DLG_ACQ, 0x8B);
            f3C530(DLG_ACQ, 42);
            m_6CF50(n);
            m_6CDA0(p);
            f39F90(2);
            fg_setcolor(5);
            f78884();
            m_71E34(3);
            f3A18C(0, 30, 640, 35, mname(n));
            f788AC();
            m_6FFC4(585, 415, 1);
            m_strip(m_sx(), 406, m_sw(), n, 0);
            f78884();
            m_71E34(0);
            f788AC();
            m_6FBDC(n, 1);
            f3C48C(DLG_ACQ, 0x8A);
            m_strip(m_sx(), 406, m_sw(), n, 1);
            m_6FE68(585, 415, n, 1, 15);
            mw32(0x110ACCu, 0);
            if (mpbem() && ms16(0xF5478u) == 1 && ms8(rec + 0x51) == 9 && ms16(STK(8)) != 0)
                cards_test_f6E158();
            for (;;) {
                f3CD6C(ms8(0xEE168u));
                r = f78BA4(DLG_ACQ, 0);
                if (r == -1)
                    break;
                if (r == -50)
                    m_6D178(p, n);
                else if (r == -51)
                    m_6D564(p, n);
            }
            m_counters(p, n);
            continue;
        }
        f3C940(D32(0xF8E78u));
        m_6CDA0(-1);
        f78884();
        m_71E34(3);
        f3A18C(0, 30, 640, 35, mname(n));
        m_71E34(0);
        f788AC();
        cards_test_f6DBAC(p, n);
        m_strip(m_sx(), 406, m_sw(), n, 0);
        f78A20(1500);
        if (mpbem() && ms16(0xF0684u + 2u * (uint32_t)n) != -97) {
            cards_test_pbem(0x6C827u);
            continue;
        }
        if (ms16(0xF0684u + 2u * (uint32_t)n) == -97) {
            if (f76E50(8, n) != 0) {
                mw32(0x110ACCu, 1);
                mw32(0xF590Cu + 4u * (uint32_t)n, 8);
                mw8(rec + 0x3E, D8(rec + 0x3E) - 18);
                mw8(rec + 0x99, D8(rec + 0x99) + 18);
            }
        } else {
            int total = 0, got;
            do {
                got = cards_test_f6DE04(n) != 0;
                total += got;
                mw32(0x110ACCu, g_original ? got : total);
            } while (got && ms8(rec + 0x3E) >= 6);
        }
        f78884();
        m_71E34(0);
        f788AC();
        m_counters(p, n);
        f3C8C4(D32(0xF8E78u));
    }
    m_6CDA0(-1);
    if (mpbem() && !human)
        cards_test_pbem(0x6C9FEu);
    if (mpbem() && D16(0xF5478u))
        cards_test_pbem(0x6CAACu);
    if (mpbem())
        cards_test_pbem(0x6CB5Du);
}

static void m_6EC84(int n)
{
    f3CD6C(ms8(0xEE168u));
    fg_vbcopy(0, 639, 0, 479, 0, 479, mbg(), mbk());
    fg_vbopen(mbk());
    f3A18C(0, 8, 640, 176, "Discard Excess Goods");
    f3A18C(0, 30, 640, 35, mname(n));
    fg_setcolor(13);
    f3A044(70, 185, "Choose goods to discard ...");
    f3A044(80, 270, "Remaining to discard:");
    fg_vbclose();
    f3C0D8(DLG_DISC, 42, 2);
    f3BFBC(DLG_DISC, 42, 0);
    f3BFBC(DLG_DISC, 0x8D, 1);
    f3B05C(DLG_DISC);
    f3CC14();
}

static void m_left(int n)
{
    char b[12];
    fg_vbcopy(204, 223, 270, 289, 204, 289, mbg(), mbk());
    sprintf(b, "%d", f76F14(n) - 8);
    fg_vbopen(mbk());
    fg_setcolor(13);
    f3A044(204, 270, b);
    fg_vbclose();
    fg_vbpaste(204, 223, 270, 289, 204, 289);
}

static void m_6E870(void)
{
    int n, r, i, j, sel, x, w, cc, any;
    uint32_t rec;

    mw32(0x11254Cu, 40);
    mw32(0x112550u, 10);
    mw8(0xEE168u, 17);
    for (n = 1; n <= 9; n++) {
        rec = NAT(n);
        if (D8(rec + 0x9A) == 0 || f76F14(n) <= 8)
            continue;
        if (!(mpbem() ? ms8(0xE1622u) == n : ms8(rec + 0x9A) == 3)) {
            if (ms8(rec + 0x9A) == 2)
                cards_test_f6EDA8(n);
            else
                mw16(0xF4096u, 1);
            continue;
        }
        m_6EC84(n);
        m_strip(50, 200, 500, n, 0);
        m_6FBDC(n, 1);
        m_strip(50, 200, 500, n, 1);
        m_6FE68(585, 209, n, 1, 15);
        for (;;) {
            if (!g_original) {
                for (any = 0, i = 1; i <= 18; i++)
                    if (ms8(rec + (uint32_t)i) > 0)
                        any = 1;
                if (!any)
                    break;
            }
            f3CD6C(ms8(0xEE168u));
            m_left(n);
            r = f78BA4(DLG_DISC, 0);
            if (r != -53)
                continue;
            mw16(0xF4096u, 1);
            sel = -100;
            x = 50;
            w = ms32(0x11254Cu) < 32 ? ms32(0x11254Cu) : 32;
            for (i = 1; i <= 18 && sel == -100; i++) {
                int mx = ms32(0xE1628u);
                cc = ms8(rec + (uint32_t)i);
                if (cc == 0)
                    continue;
                if (cc == 1) {
                    if (mx >= x && x + w >= mx)
                        sel = i;
                    x += ms32(0x11254Cu);
                    continue;
                }
                for (j = 0; j < cc; j++) {
                    if (mx >= x && x + w >= mx)
                        sel = i;
                    x += ms32(0x112550u);
                }
                x += ms32(0x11254Cu) - ms32(0x112550u);
            }
            if (sel == -100)
                continue;
            f771F8(sel);
            mw8(rec + (uint32_t)sel, D8(rec + (uint32_t)sel) - 1);
            m_strip(50, 200, 500, n, 1);
            m_6FE68(585, 209, n, 1, 15);
            if (f76F14(n) <= 8) {
                fg_vbcopy(204, 223, 270, 289, 204, 289, mbg(), mbk());
                {
                    char b[12];
                    sprintf(b, "%d", f76F14(n) - 8);
                    fg_vbopen(mbk());
                    fg_setcolor(13);
                    f3A044(204, 270, b);
                    fg_vbclose();
                    fg_vbpaste(204, 223, 270, 289, 204, 289);
                }
                break;
            }
        }
    }
}

static int m_77510(int s)
{
    uint32_t pl = PIL(s);
    int cnt = ms16(pl), i, j, at = -1;
    for (i = 0; i < cnt && at < 0; i++)
        for (j = 1; j <= 4; j++)
            if (ms8(0xCA9E0u + (uint32_t)j) == ms8(pl + 6u + (uint32_t)i)) {
                at = i;
                break;
            }
    if (at < 0)
        return 0;
    for (i = at; i < cnt - 1; i++)
        mw8(pl + 6u + (uint32_t)i, D8(pl + 7u + (uint32_t)i));
    mw16(pl, cnt - 1);
    return 1;
}

static void m_77278(void)
{
    int s, f;
    for (s = 0; s <= 8; s++) {
        uint32_t pl = PIL(s), st = STK(s);
        if (ms16(pl) != 0) {
            f = (s >= 1 && s <= 4) ? m_77510(s) : 0;
            f76F70(ms16(pl), DP(pl + 6));
            f77184(ms16(pl), DP(pl + 6), ms16(st), DP(st + 6));
            mw16(st, (uint16_t)(D16(st) + D16(pl)));
            if (f) {
                f77184(1, DP(0xCA9E0u + (uint32_t)s), ms16(st), DP(st + 6));
                mw16(st, (uint16_t)(D16(st) + 1));
            }
        }
        mw16(pl, 0);
    }
}

static void m_6E770(void)
{
    int n, any = 0;
    mw16(0xF06AEu, 0);
    for (n = 1; n <= 9; n++)
        if (D8(NAT(n) + 0x9A) != 0) {
            int k = ms16(0xF06AEu);
            mw16(0xF06B0u + 2u * (uint32_t)k, n);
            mw16(0xF06AEu, k + 1);
        }
    f783B8(DLG_DISC, ms32(0xF5964u));
    for (n = 1; n <= 9 && !any; n++)
        if (D8(NAT(n) + 0x9A) != 0 && f76F14(n) > 8)
            any = 1;
    if (any)
        m_6E870();
    else
        f79270(100, ms16(0xF06B0u), -200, "", 0);
    if (mpbem())
        mw16(0xF4098u, 1);
    else
        m_77278();
}

/* ============================================================================================================== */
/* The runner                                                                                                     */
/* ============================================================================================================== */

static uint8_t *g_start, *g_after[2];
static int g_fail, g_checks, g_shown;

typedef void (*Fn)(void *);

static void run_side(int side, Fn fn, void *arg)
{
    g_cur = side;
    g_len[side] = 0;
    g_log[side][0] = 0;
    memcpy(g_ds, g_start, DS_SIZE);
    memset(&g_btn, 0, sizeof g_btn);
    g_btn.en[0][0x8A] = 1;   /* the dialog's state as 0x7784C leaves it (G for the test: ACQUIRE on) */
    g_srng = g_srng0;
    g_steps = 0;
    if (setjmp(g_abort) == 0)
        fn(arg);
    memcpy(g_after[side], g_ds, DS_SIZE);
    g_btn_saved[side] = g_btn;
}

/* coverage: how many checked runs reached each path (from the port's trace) */
static const char *const COV[] = { "-> -50 ", "-> -51 ", "-> -52 ", "-> -53 ", "draw 8 ", "draw -100", "Empty'",
                                   "View Good", "AI f6DE04", "AI f6DBAC", "AI f6EDA8", "ABORT steps", "PBEM",
                                   "discard ", "shuffle ", "insert 1 ", "msg 100", "makegif", "label 1106D8 42 B8CC3",
                                   "title 0 30" };
#define NCOV (sizeof COV / sizeof COV[0])
static int g_cov[NCOV];

static void compare(const char *what, Fn port, Fn model, void *arg)
{
    size_t i;
    g_checks++;
    memcpy(g_start, g_ds, DS_SIZE);
    run_side(0, port, arg);
    run_side(1, model, arg);
    for (i = 0; i < NCOV; i++)
        g_cov[i] += strstr(g_log[0], COV[i]) != NULL;
    memcpy(g_ds, g_start, DS_SIZE);
    if (g_len[0] != g_len[1] || memcmp(g_log[0], g_log[1], g_len[0]) != 0) {
        const char *a = g_log[0], *b = g_log[1];
        int line = 1;
        while (*a && *a == *b) {
            if (*a == '\n')
                line++;
            a++, b++;
        }
        while (a > g_log[0] && a[-1] != '\n')
            a--, b--;
        g_fail++;
        if (g_shown++ < 12) {
            printf("FAIL %s (original %d): trace line %d\n  port : %.120s\n  model: %.120s\n", what, g_original, line,
                   a, b);
        }
        return;
    }
    for (i = 0; i < DS_SIZE; i++)
        if (g_after[0][i] != g_after[1][i]) {
            g_fail++;
            if (g_shown++ < 12)
                printf("FAIL %s (original %d): byte 0x%X port %02X model %02X\n", what, g_original,
                       (unsigned)(i + DS_BASE), g_after[0][i], g_after[1][i]);
            return;
        }
    if (memcmp(&g_btn_saved[0], &g_btn_saved[1], sizeof g_btn) != 0) {
        g_fail++;
        if (g_shown++ < 12)
            printf("FAIL %s (original %d): button table\n", what, g_original);
    }
}

static void p_6BD98(void *a) { (void)a; f6BD98(); }
static void m_6BD98w(void *a) { (void)a; m_6BD98(); }
static void p_6E770(void *a) { (void)a; f6E770(); }
static void m_6E770w(void *a) { (void)a; m_6E770(); }
static void p_77278(void *a) { (void)a; f77278(); }
static void m_77278w(void *a) { (void)a; m_77278(); }

typedef struct { int k, a, b, c, d, e; } Call;
static void p_call(void *v)
{
    Call *c = v;
    switch (c->k) {
    case 0: f6FBDC(c->a, c->b); break;
    case 1: f6FE68(c->a, c->b, c->c, c->d, c->e); break;
    case 2: f6FFC4(c->a, c->b, c->c, c->d); break;
    case 3: f71D30(c->a); break;
    case 4: f71E34(c->a); break;
    case 5: f6CDA0(c->a); break;
    case 6: f6CF50(c->a); break;
    case 7: f6D010(c->a, c->b, c->c, c->d); break;
    case 8: f6D0C4(c->a, c->b, c->c, c->d); break;
    case 9: f6CB8C(); break;
    case 10: f6EC84(c->a); break;
    default: break;
    }
}
static void m_call(void *v)
{
    Call *c = v;
    switch (c->k) {
    case 0: m_6FBDC(c->a, c->b); break;
    case 1: m_6FE68(c->a, c->b, c->c, c->d, c->e); break;
    case 2: m_6FFC4(c->a, c->b, c->d); break;
    case 3: m_71D30(c->a); break;
    case 4: m_71E34(c->a); break;
    case 5: m_6CDA0(c->a); break;
    case 6: m_6CF50(c->a); break;
    case 7: m_strip(c->a, c->b, c->c, c->d, 0); break;
    case 8: m_strip(c->a, c->b, c->c, c->d, 1); break;
    case 9: m_6CB8C(); break;
    case 10: m_6EC84(c->a); break;
    default: break;
    }
}

/* ---- random states ------------------------------------------------------------------------------------------------ */

static uint32_t g_rng = 12345;
static uint32_t rnd(uint32_t n)
{
    g_rng = g_rng * 1103515245u + 12345u;
    return n ? (g_rng >> 8) % n : 0;
}

static uint8_t *g_pristine;

static void random_cards(void)
{
    int s, i, n, k;
    for (s = 0; s < 9; s++) {
        k = (int)rnd(20);
        setD16(STK(s), (uint16_t)k);
        for (i = 0; i < 19; i++)
            setD8(STK(s) + 6u + (uint32_t)i, (uint8_t)(rnd(8) == 0 ? 0x13 + rnd(12) : 2 * s + 1 + rnd(2)));
        k = (int)rnd(4) == 0 ? (int)rnd(19) : (int)rnd(4);
        setD16(PIL(s), (uint16_t)k);
        for (i = 0; i < 19; i++)
            setD8(PIL(s) + 6u + (uint32_t)i, (uint8_t)(rnd(5) == 0 ? 0x13 + rnd(12) : 2 * s + 1 + rnd(2)));
    }
    (void)n;
}

static void random_state(int forDiscard)
{
    int n, k, players = 0, humans = 0, i;
    uint32_t rec;

    memcpy(g_ds, g_pristine, DS_SIZE);
    random_cards();
    for (n = 1; n <= 9; n++) {
        rec = NAT(n);
        k = (int)rnd(10);
        setD8(rec + 0x9A, (uint8_t)(k < 2 || players >= 8 ? 0 : k < 8 ? 3 : 2));
        if (D8(rec + 0x9A) == 0)
            continue;
        players++;
        humans += D8(rec + 0x9A) == 3;
        setD8(rec + 0x51, (uint8_t)(rnd(3) == 0 ? 0 : rnd(10)));
        setD8(rec + 0x3E, (uint8_t)(rnd(40)));
        setD8(rec + 0x99, (uint8_t)(rnd(30)));
        setD8(rec + 0x3C, (uint8_t)rnd(3));
        for (i = 1; i <= 18; i++)
            setD8(rec + (uint32_t)i, (uint8_t)(forDiscard ? (rnd(3) == 0 ? rnd(4) : 0) : (rnd(4) == 0 ? rnd(3) : 0)));
        k = forDiscard && rnd(12) == 0 ? 9 + (int)rnd(2) : (int)rnd(3);
        setD8(rec + 0x13, (uint8_t)k);
        if (k > 8)   /* calamities only, over 8 */
            for (i = 1; i <= 18; i++)
                setD8(rec + (uint32_t)i, 0);
        for (i = 0; i < 8; i++)
            setD8(rec + 0x14u + (uint32_t)i, (uint8_t)(i < k ? 0x13 + rnd(12) : 0));
        setD16(0xF0684u + 2u * (uint32_t)n, (uint16_t)(rnd(10) == 0 ? -0x61 : rnd(10) == 0 ? -0x62 : 0));
    }
    setD32(0xF40ECu, (uint32_t)players);
    setD8(0xE1620u, (uint8_t)humans);
    setD8(0xE1624u, (uint8_t)((rnd(2) ? 0x20 : 0) | (rnd(2) ? 0x40 : 0) | (rnd(15) == 0 ? 0x80 : 0)));
    setD8(0xE1622u, (uint8_t)(1 + rnd(9)));
    setD16(0xF5478u, (uint16_t)rnd(2));
    setD32(SEED, rnd(0xFFFFFFFFu) ^ (rnd(0xFFFF) << 16));
    setD16(0xF06AEu, (uint16_t)rnd(3));
    setD16(0xF06B0u, (uint16_t)(1 + rnd(9)));
    g_srng0 = 0x9E3779B9u ^ (uint32_t)rnd(0x7FFFFFFF);
    g_budget = (int)(5 + rnd(60));
}

/* ---- part A: hand vectors ------------------------------------------------------------------------------------------- */

static void expect(int ok, const char *what)
{
    g_checks++;
    if (!ok) {
        g_fail++;
        printf("FAIL A: %s\n", what);
    }
}

static void part_a(void)
{
    uint32_t seed;
    int r, i;
    static const uint8_t pile[4] = { 5, 0x13, 7, 0x15 };

    /* 0x77510: the first of 0x13 / 0x15 / 0x17 / 0x19 out of pile 1 */
    memcpy(g_ds, g_pristine, DS_SIZE);
    setD16(PIL(1), 4);
    memcpy(DP(PIL(1) + 6), pile, 4);
    r = f77510(1);
    expect(r == 1 && S16(PIL(1)) == 3 && D8(PIL(1) + 6) == 5 && D8(PIL(1) + 7) == 7 && D8(PIL(1) + 8) == 0x15,
           "0x77510 takes 0x13 out of {5, 0x13, 7, 0x15}");
    r = f77510(0);
    expect(r == 0 && S16(PIL(0)) == 0, "0x77510 on an empty pile");

    /* 0x77278: stack 1 {3, 4, 4}, pile 1 {0x13, 4}: the pile without 0x13 (one card, one draw) in front, then 0x13 */
    memcpy(g_ds, g_pristine, DS_SIZE);
    for (i = 0; i < 9; i++) {
        setD16(PIL(i), 0);
        setD16(STK(i), 0);
    }
    setD16(STK(1), 3);
    setD8(STK(1) + 6, 3); setD8(STK(1) + 7, 4); setD8(STK(1) + 8, 4);
    setD16(PIL(1), 2);
    setD8(PIL(1) + 6, 0x13); setD8(PIL(1) + 7, 4);
    setD32(SEED, 0x12345678u);
    g_cur = 0; g_len[0] = 0;
    f77278();
    seed = 0x12345678u * 0x15A4E35u + 1u;
    expect(S16(STK(1)) == 5 && D8(STK(1) + 6) == 0x13 && D8(STK(1) + 7) == 4 && D8(STK(1) + 8) == 3 &&
           D8(STK(1) + 9) == 4 && D8(STK(1) + 10) == 4 && S16(PIL(1)) == 0 && D32(SEED) == seed,
           "0x77278 stack {0x13, 4, 3, 4, 4}, seed advanced once");

    /* 0x77278: game_rand once per card f76F70 shuffles (0x76FBB is in its loop): pile 0 {1, 2, 1} draws three
       numbers; pile 2 holding only its calamity 0x15 gets f76F70 with 0 cards (no draw) and 0x15 back on stack 2 */
    memcpy(g_ds, g_pristine, DS_SIZE);
    for (i = 0; i < 9; i++) {
        setD16(PIL(i), 0);
        setD16(STK(i), 0);
    }
    setD16(PIL(0), 3);
    setD8(PIL(0) + 6, 1); setD8(PIL(0) + 7, 2); setD8(PIL(0) + 8, 1);
    setD16(PIL(2), 1);
    setD8(PIL(2) + 6, 0x15);
    setD32(SEED, 0x12345678u);
    g_cur = 0; g_len[0] = 0;
    f77278();
    seed = 0x12345678u;
    for (i = 0; i < 3; i++)
        seed = seed * 0x15A4E35u + 1u;
    expect(S16(STK(0)) == 3 && S16(PIL(0)) == 0 && S16(STK(2)) == 1 && D8(STK(2) + 6) == 0x15 && S16(PIL(2)) == 0 &&
           D32(SEED) == seed, "0x77278 three cards drawn three numbers, a calamity-only pile none");

}

/* The column and empty-stack vectors run through a fixed click list instead of the picker. */

static void part_a2(void)
{
    static const int clicks[] = { -0x34, -1, -100 };
    uint32_t rec = NAT(1);
    int i, mode;

    for (mode = 0; mode < 6; mode++)
        for (g_original = 1; g_original >= 0; g_original--) {
            memcpy(g_ds, g_pristine, DS_SIZE);
            for (i = 0; i < 9; i++) {
                setD16(STK(i), 3);
                setD8(STK(i) + 6, (uint8_t)(2 * i + 1)); setD8(STK(i) + 7, (uint8_t)(2 * i + 1));
                setD8(STK(i) + 8, (uint8_t)(2 * i + 2));
            }
            setD8(0xE1624u, 0x20);
            setD8(rec + 0x3E, 5); setD8(rec + 0x99, 10); setD8(rec + 0x3C, 0);
            setD32(0x110ACCu, 0);
            memset(&g_btn, 0, sizeof g_btn);
            g_fixed = clicks;
            g_fixed_i = 0;
            if (mode == 0)
                g_fixed_x[0] = 0xEC + 78 + 5;          /* stack 2's column: unaffordable (6 > 5) */
            else if (mode == 1)
                g_fixed_x[0] = 0xEC + 0x22;            /* between the columns of stacks 0 and 1 */
            else if (mode == 2)
                g_fixed_x[0] = 0xEC + 39 + 3;          /* stack 1 (4 <= 5) */
            else if (mode == 3) {
                g_fixed_x[0] = 0;                      /* CANCEL at once */
                g_fixed_i = 1;
            } else if (mode == 4) {                    /* no variant, stack 8 empty */
                setD8(0xE1624u, 0);
                setD8(rec + 0x3E, 20);
                setD16(STK(8), 0);
            } else {                                   /* no variant, stack 8 with its card */
                setD8(0xE1624u, 0);
                setD8(rec + 0x3E, 20);
            }
            g_cur = 0; g_len[0] = 0;
            if (setjmp(g_abort) == 0)
                f6D564(0, 1);
            g_fixed = NULL;
            switch (mode) {
            case 0:
                if (g_original)
                    expect(S8(rec + 0x3E) == -1 && D8(rec + 0x99) == 16 && D8(rec + 0x06) == 1 && S16(STK(2)) == 2 &&
                           S32(0x110ACCu) == 1, "BUY: the unaffordable column buys with --original (treasury -1)");
                else
                    expect(S8(rec + 0x3E) == 5 && D8(rec + 0x99) == 10 && S16(STK(2)) == 3 && S32(0x110ACCu) == 0 &&
                           D8(rec + 0x3C) == 0, "BUY: the unaffordable column ignored, CANCEL takes +0x3C back");
                break;
            case 1:
                if (g_original)
                    expect(S8(rec + 0x3E) == 5 - 58 && D8(rec + 0x99) == 10 + 58 && S32(0xF590Cu + 4u) == -100,
                           "BUY: between the columns, stack -100 and 58 paid with --original");
                else
                    expect(S8(rec + 0x3E) == 5 && D8(rec + 0x99) == 10 && D8(rec + 0x3C) == 0,
                           "BUY: between the columns ignored");
                break;
            case 2:
                expect(S8(rec + 0x3E) == 1 && D8(rec + 0x99) == 14 && D8(rec + 0x04) == 1 && S16(STK(1)) == 2 &&
                           D8(rec + 0x3C) == 1 && S32(0xF590Cu + 4u) == 1 && S32(0x110ACCu) == 1,
                       "BUY: stack 1 for 4");
                break;
            case 3:
                expect(S8(rec + 0x3E) == 5 && D8(rec + 0x3C) == (g_original ? 1 : 0),
                       "BUY: CANCEL leaves +0x3C 1 with --original, 0 fixed");
                break;
            case 4:
                if (g_original)
                    expect(S8(rec + 0x3E) == 2 && D8(rec + 0x99) == 28 && S32(0x110ACCu) == 1 && D8(rec + 0x3C) == 1,
                           "BUY: empty stack 8 paid 18 with --original");
                else
                    expect(S8(rec + 0x3E) == 20 && D8(rec + 0x99) == 10 && S32(0x110ACCu) == 0 && D8(rec + 0x3C) == 0,
                           "BUY: empty stack 8 buys nothing");
                break;
            default:
                expect(S8(rec + 0x3E) == 2 && D8(rec + 0x99) == 28 && S32(0x110ACCu) == 1 && D8(rec + 0x12) == 1 &&
                           S16(STK(8)) == 2 && S32(0xF590Cu + 4u) == 8, "BUY: stack 8's top card (18) for 18");
                break;
            }
        }
    g_original = 0;

    /* the discard hit test: hand {1 x card 3, 3 x card 5, 1 x card 9} + 5 others to make 10: positions from 0x32:
       card 3 at 50..82, card 5 copies at 90, 100, 110 (then +30), card 9 at 150 */
    {
        static const int dclicks[] = { -0x35, -0x35, -100 };
        for (g_original = 1; g_original >= 0; g_original--) {
            memcpy(g_ds, g_pristine, DS_SIZE);
            for (i = 1; i <= 9; i++)
                setD8(NAT(i) + 0x9A, 0);
            setD8(rec + 0x9A, 3);
            setD8(0xE1620u, 1);
            setD8(0xE1624u, 0);
            for (i = 1; i <= 18; i++)
                setD8(rec + (uint32_t)i, 0);
            setD8(rec + 0x13, 0);
            setD8(rec + 3, 1); setD8(rec + 5, 3); setD8(rec + 9, 1); setD8(rec + 11, 5);
            for (i = 0; i < 9; i++)
                setD16(PIL(i), 0);
            memset(&g_btn, 0, sizeof g_btn);
            g_fixed = dclicks;
            g_fixed_i = 0;
            g_fixed_x[0] = 105;    /* card 5 (the third copy region starts at 110, so 105 is in the second's) */
            g_fixed_x[1] = 151;    /* card 9 */
            g_cur = 0; g_len[0] = 0;
            if (setjmp(g_abort) == 0)
                f6E870();
            g_fixed = NULL;
            expect(D8(rec + 5) == 2 && D8(rec + 9) == 0 && S16(PIL(2)) == 1 && D8(PIL(2) + 6) == 5 && S16(PIL(4)) == 1 &&
                       D8(PIL(4) + 6) == 9 && S16(0xF4096u) == 1, "DISCARD: x 105 is card 5, x 151 card 9");
        }
    }
    g_original = 0;
}

/* ---- the real saves ------------------------------------------------------------------------------------------------ */

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

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:\\work\\advciv_run";
    const char *playdir = argc > 2 ? argv[2] : "C:\\work\\advciv_play";
    int i, f0, k;
    char path[1024];
    static const char *const SAVES[4][2] = { { "p", "SAVGAMEA.DAT" }, { "p", "savgameb.dat" }, { "g", "savgame0.dat" },
                                             { "g", "savgame1.dat" } };

    if (state_init(gamedir) != 0) {
        printf("cannot load the data object from %s\n", gamedir);
        return 2;
    }
    g_pristine = malloc(DS_SIZE);
    g_start = malloc(DS_SIZE);
    g_after[0] = malloc(DS_SIZE);
    g_after[1] = malloc(DS_SIZE);
    g_log[0] = malloc(LOGSZ);
    g_log[1] = malloc(LOGSZ);
    if (!g_pristine || !g_start || !g_after[0] || !g_after[1] || !g_log[0] || !g_log[1])
        return 2;
    memcpy(g_pristine, g_ds, DS_SIZE);

    part_a();
    part_a2();
    printf("A: %d checks, %d failures\n", g_checks, g_fail);

    f0 = g_fail;
    k = g_checks;
    for (i = 0; i < 20000; i++) {
        memcpy(g_ds, g_pristine, DS_SIZE);
        random_cards();
        setD32(SEED, rnd(0xFFFFFFFFu) ^ (rnd(0xFFFF) << 16));
        g_original = i & 1;
        compare("B 0x77278", p_77278, m_77278w, NULL);
    }
    printf("B: %d checks, %d failures\n", g_checks - k, g_fail - f0);

    f0 = g_fail;
    k = g_checks;
    for (i = 0; i < 3000; i++) {
        random_state(0);
        for (g_original = 0; g_original <= 1; g_original++)
            compare("C 0x6BD98", p_6BD98, m_6BD98w, NULL);
    }
    g_original = 0;
    printf("C: %d checks, %d failures\n", g_checks - k, g_fail - f0);

    f0 = g_fail;
    k = g_checks;
    for (i = 0; i < 3000; i++) {
        random_state(1);
        for (g_original = 0; g_original <= 1; g_original++)
            compare("D 0x6E770", p_6E770, m_6E770w, NULL);
    }
    g_original = 0;
    printf("D: %d checks, %d failures\n", g_checks - k, g_fail - f0);

    f0 = g_fail;
    k = g_checks;
    for (i = 0; i < 4000; i++) {
        Call c;
        int j;

        random_state((int)rnd(2));
        for (j = 0; j < 14; j++)
            setD16(0x111524u + 2u * (uint32_t)rnd(56), (uint16_t)rnd(700));
        for (j = 0; j < 9; j++)
            setD16(0xF06B0u + 2u * (uint32_t)j, (uint16_t)(1 + rnd(9)));
        c.k = (int)rnd(11);
        c.a = c.k <= 2 || c.k >= 7 ? (int)rnd(640) : (int)rnd(14) - 1;
        if (c.k == 0 || c.k == 6 || c.k == 10)
            c.a = 1 + (int)rnd(9);
        if (c.k == 5)
            c.a = (int)rnd(10) - 1;
        c.b = c.k == 0 ? (int)rnd(3) : (int)rnd(480);
        c.c = c.k == 1 || c.k == 2 ? (int)rnd(10) : (int)rnd(600);
        c.d = c.k == 7 || c.k == 8 ? 1 + (int)rnd(9) : (int)rnd(3);
        c.e = (int)rnd(256);
        if (c.k == 5)
            setD16(0xF06AEu, (uint16_t)rnd(9));
        compare("E helper", p_call, m_call, &c);
    }
    printf("E: %d checks, %d failures\n", 4000, g_fail - f0);

    f0 = g_fail;
    k = g_checks;
    for (i = 0; i < 4; i++) {
        int s;
        snprintf(path, sizeof path, "%s\\%s", SAVES[i][0][0] == 'p' ? playdir : gamedir, SAVES[i][1]);
        if (!load_save(path)) {
            printf("FAIL F: cannot read %s\n", path);
            g_fail++;
            continue;
        }
        {
            int n, cities = 0, cards = 0;
            for (n = 1; n <= 9; n++)
                if (D8(NAT(n) + 0x9A) != 0) {
                    cities += S8(NAT(n) + 0x51);
                    cards += f76F14(n);
                }
            printf("F: %s round %d, players %d (humans %d), cities %d, cards held %d\n", SAVES[i][1], S16(0xF40FCu),
                   S32(0xF40ECu), S8(0xE1620u), cities, cards);
        }
        memcpy(g_start, g_ds, DS_SIZE);
        {
            uint8_t *base = malloc(DS_SIZE);
            if (base == NULL)
                return 2;
            memcpy(base, g_ds, DS_SIZE);
            for (s = 0; s < 100; s++) {
                memcpy(g_ds, base, DS_SIZE);
                g_srng0 = 0xA5A5A5A5u + (uint32_t)s * 7919u;
                g_budget = 5 + s % 40;
                for (g_original = 0; g_original <= 1; g_original++) {
                    compare("F 0x6BD98", p_6BD98, m_6BD98w, NULL);
                    compare("F 0x6E770", p_6E770, m_6E770w, NULL);
                }
            }
            free(base);
        }
    }
    g_original = 0;
    printf("F: %d checks, %d failures\n", g_checks - k, g_fail - f0);

    printf("coverage (checked runs reaching each path):");
    for (i = 0; i < (int)NCOV; i++)
        printf("%s [%s] %d", i % 5 ? "," : "\n ", COV[i], g_cov[i]);
    printf("\n%s: %d checks, %d failures\n", g_fail ? "FAILED" : "PASSED", g_checks, g_fail);
    return g_fail ? 1 : 0;
}
