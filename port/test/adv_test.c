/* adv_test [GAMEDIR [SAVEDIR]]: unit checks for module ADV (adv.c) that need no game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init; default C:\work\advciv_run, only read) and the saves
   savgame0.dat (start of MOVEMENT R4) and savgame1.dat (R4, all human); SAVEDIR holds SAVGAMEA.DAT and savgameb.dat
   (default C:\work\advciv_play, only read). All four are read with saveload.c's f5F05C as the game does.
   adv.c is built with -DADV_TEST: its calls into the computers' code (0x1A888, 0x177EC, 0x1B59C, 0x19700) and into
   CARDS (0x6FBDC, 0x6D0C4) come to recorders here. 0x177EC picks a scripted advance among those in state 2 (or -1);
   0x1B59C succeeds or fails by script and on success leaves record 0 holding a scripted part of the hand and a
   scripted [0xF6750], as the real routine leaves its choice there; 0x6D0C4 sets the hand spacing as its f71FB0 does.

   Every routine adv.c calls is a stub below that appends one line to a trace (drawing included); those with data
   effects apply them to the data object: the button-by-id helpers (label, colour, pressed, enabled bytes of the
   record; x / y / w / h read back), f194E4 / f195D8 / f6DD98 / f17760 (after info.h's and game.h's descriptions),
   f771F8 (the discard pile, after rules.h), f71F2C / f71FB0 (the spacing [0x11254C] / [0x112550]), the dispatcher
   f3FB94 (id -> code, the advance ids store [0xF6758], as dispatch.c's table), the heap (handles counted from 1 per
   run). 0x176BC and 0x193AC are formulas.c's real code. Clicks come from a script: fg_mousepos returns a seeded random
   click on an enabled button of 0xF634C (sometimes anywhere), late in a run FINISHED or ACQUIRE; f78BA4 (the wrap-up)
   returns seeded codes; a run with more than 300 clicks is cut ("CUT") identically on both sides.

   The model is a second implementation of the eight routines, written from the disassembly (not from adv.c): it
   runs on a copy of the data object (g_ds is pointed at the copy while it runs, so the stubs and formulas.c act on
   it), calls the same stubs, and so writes the trace it expects. After each run the two traces must be equal and the
   whole data object must equal the model's copy. Parts:
     A. hand vectors (expected values written out): 0x17EDC's states, credits (Pottery's row), colours with the
        Alternate Colors toggle, the budget form; 0x18AEC with credit 60 > cost 45 (fixed 0, --original -15); the
        treasury left by one human for the next (fixed 0, --original 2); 0xF40AE after the phase (fixed 1, --original
        0); a computer's third advance (fixed count 3 and owned, --original 2 and not owned); hand clicks moving card
        18 three times (Excess 36, ACQUIRE spends them); 0x19170's fit of 18 single cards (step 18, overlap 3) and
        of widths reaching exactly 337 and 336 (the > 0x150 test); the wrap-up's row geometry.
     B. 0x17EDC on 40,000 random states (budget -1 or given, draw 0 / 1, the toggle);
     C. 0x1818C (both modes) / 0x18DD8 / 0x18AEC / 0x18C68 / 0x19170 on 20,000 random selections;
     D. 0x162D0 on 6,000 random games (humans, computers, PBEM bits, Grain, stale treasury, click scripts), both modes;
     E. 0x1A3A0 on 6,000 random states (attended and unattended);
     F. 0x162D0 and 0x1A3A0 on the four real saves, 150 click scripts each, both modes.
   Prints each failure (the first differing trace line or byte), the counts per part, and exits 0 when all pass.
   Also prints how often the port's traces reached each path (dispatcher codes, purchases, RESTART, the computers'
   recorders, the Famine strip, Excess, the Alternate Colors, the message). Checked against 16 deliberately broken
   copies of adv.c (one line each: comparisons, offsets, a dropped restore / credit / Grain, each BUG(1995) fix
   dropped): every one fails.

   Build: clang -std=c11 -DADV_TEST port/test/adv_test.c port/src/adv.c port/src/formulas.c port/src/saveload.c
          port/src/state.c port/src/le_image.c -Iport/src -o adv_test.exe */

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adv.h"
#include "fg.h"
#include "formulas.h"
#include "game.h"
#include "info.h"
#include "kit.h"
#include "panel.h"
#include "rules.h"
#include "saveload.h"
#include "state.h"
#include "ui.h"

/* ---- addresses (from the listing, written out here) ------------------------------------------------------------ */

#define NAT(n)     (0xF14CCu + 199u * (uint32_t)(n))
#define R0         0xF14CCu
#define DLG        0xF634Cu
#define WRAP       0xBD940u
#define T_SEL      0xF6758u
#define T_PREV     0xF675Cu
#define T_HAND     0xF6760u
#define T_CRED     0xF6764u
#define T_ST       0xF67C4u
#define T_NEED     0xF67DCu
#define T_TU       0xF67E0u
#define T_GOODS    0xF67E4u
#define T_GSTEP    0xF6738u
#define T_GOVER    0xF673Cu
#define T_MBOUGHT  0xF6740u
#define T_MUSED    0xF6744u
#define T_AITR     0xF6750u
#define T_MFLAG    0xF6754u
#define T_LW       0xF68ACu
#define T_MCH      0xF68B0u
#define T_MCNT     0xF68B4u
#define T_MLIST    0xF68B8u
#define T_MIDX     0xF68C0u
#define T_CARRY    0xF68C8u
#define T_SPENT(n) (0xF6D54u + 19u * (uint32_t)(n))
#define T_BUY(n)   (0xF6E12u + 10u * (uint32_t)(n))
#define T_CNT(n)   (0xF6E76u + 2u * (uint32_t)(n))
#define T_GRAIN(n) (0xF409Au + 2u * (uint32_t)(n))
#define T_HSTEP    0x11254Cu
#define T_HOVER    0x112550u
#define MX         0xE1628u
#define MY         0xE162Cu
#define MB         0xE1630u

static int  b8(uint32_t a) { return D8(a); }
static int  s8(uint32_t a) { return S8(a); }
static int  s16(uint32_t a) { return S16(a); }
static int32_t s32(uint32_t a) { return S32(a); }
static void w8(uint32_t a, int v) { setD8(a, (uint8_t)v); }
static void w16(uint32_t a, int v) { setD16(a, (uint16_t)v); }
static void w32(uint32_t a, int32_t v) { setD32(a, (uint32_t)v); }
static const char *str(uint32_t a) { return DSTR(a); }
static int cost_of(int i) { return s16(0xBD480u + 2u * (uint32_t)i); }

/* ---- traces ---------------------------------------------------------------------------------------------------- */

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

/* ---- the script RNG, the cut ------------------------------------------------------------------------------------- */

static uint32_t g_srng;
static uint32_t srnd(uint32_t n)
{
    g_srng ^= g_srng << 13; g_srng ^= g_srng >> 17; g_srng ^= g_srng << 5;
    return n ? g_srng % n : 0;
}
static jmp_buf g_jb;
static int g_clicks, g_unatt, g_cuts;
static int g_p177[8], g_np177, g_k177;   /* part A: the scripted 0x177EC answers (-2: random) */
static int g_b59c;                        /* part A: 1 = 0x1B59C succeeds paying from the treasury only */
static int32_t g_tu_seen[10];             /* part A: [0xF67E0] when 0x6FBDC is reached for n */
static int g_forced[16][2], g_nforced, g_kforced;   /* part A: fixed clicks before the random script */
#define CUTN 300

/* ---- heap (handles per run from 1) ------------------------------------------------------------------------------ */

static uint8_t g_heap[8][256];
static int g_heap_n;
uint32_t load_heap_alloc(uint32_t size)
{
    tr("malloc %u", (unsigned)size);
    if (size == 0 || size > 256 || g_heap_n >= 8)
        return 0;
    memset(g_heap[g_heap_n], 0xCD, 256);
    return (uint32_t)(++g_heap_n);
}
uint8_t *load_heap_ptr(uint32_t h, uint32_t n)
{
    if (h == 0 || h > 8 || n > 256)
        return NULL;
    return g_heap[h - 1];
}
void load_heap_free(uint32_t h) { tr("free %u", (unsigned)h); }

/* ---- leaf stubs ---------------------------------------------------------------------------------------------------- */

_Noreturn void not_ported(uint32_t addr) { fprintf(stderr, "not_ported 0x%X\n", (unsigned)addr); exit(3); }
int f78AE4(void) { return 0; }
void f76F70(int n, uint8_t *cards) { (void)n; (void)cards; }
int f78D08(int t, int n) { (void)t; (void)n; return 0; }   /* formulas.c's f46A4C / f46B34 only, never called here */

static uint32_t find_rec(uint32_t dlg, int id)
{
    int i, c = s16(dlg);
    for (i = 0; i < c; i++)
        if ((int16_t)D16(dlg + 2u + 20u * (uint32_t)i + 0x0Eu) == (int16_t)id)
            return dlg + 2u + 20u * (uint32_t)i;
    return 0;
}
void f3C284(uint32_t dlg, int id, uint32_t label)
{
    uint32_t r = find_rec(dlg, id);
    tr("label %X %X %s", (unsigned)dlg, (unsigned)id, str(label));
    if (r) w32(r + 8, (int32_t)label);
}
void f3BEA0(uint32_t dlg, int id, int color) { uint32_t r = find_rec(dlg, id); tr("lcol %X %X %d", (unsigned)dlg, (unsigned)id, color); if (r) w8(r + 0x0C, color); }
void f3B914(uint32_t dlg, int id, int p) { uint32_t r = find_rec(dlg, id); tr("press %X %X %d", (unsigned)dlg, (unsigned)id, p); if (r) w8(r + 0x12, p); }
void f3BFBC(uint32_t dlg, int id, int e) { uint32_t r = find_rec(dlg, id); tr("en %X %X %d", (unsigned)dlg, (unsigned)id, e); if (r) w8(r + 0x10, e); }
void f3C314(uint32_t dlg, int id) { uint32_t r = find_rec(dlg, id); tr("onb %X %X", (unsigned)dlg, (unsigned)id); if (r) w8(r + 0x10, 1); }
void f3C3D0(uint32_t dlg, int id) { uint32_t r = find_rec(dlg, id); tr("offb %X %X", (unsigned)dlg, (unsigned)id); if (r) w8(r + 0x10, 0); }
void f3C48C(uint32_t dlg, int id) { uint32_t r = find_rec(dlg, id); tr("ona %X %X", (unsigned)dlg, (unsigned)id); if (r) w8(r + 0x10, 1); }
void f3C530(uint32_t dlg, int id) { uint32_t r = find_rec(dlg, id); tr("offa %X %X", (unsigned)dlg, (unsigned)id); if (r) w8(r + 0x10, 0); }
int f3BBD8(uint32_t dlg, int id) { uint32_t r = find_rec(dlg, id); return r ? s16(r) : 0; }
int f3BCF4(uint32_t dlg, int id) { uint32_t r = find_rec(dlg, id); return r ? s16(r + 2) : 0; }
int f3BA30(uint32_t dlg, int id) { uint32_t r = find_rec(dlg, id); return r ? s16(r + 4) : 0; }
int f3BABC(uint32_t dlg, int id) { uint32_t r = find_rec(dlg, id); return r ? s16(r + 6) : 0; }
void f3B05C(uint32_t dlg) { tr("dialog %X", (unsigned)dlg); w32(0xF8E78u, (int32_t)dlg); }
void f3CC14(void) { tr("show"); }
void f3CD6C(int8_t pic) { tr("bg %d", pic); }
void f783B8(uint32_t dlg, int base) { tr("783B8 %X %d", (unsigned)dlg, base); }
void f39F90(int font) { tr("font %d", font); }
void f3A0B0(int x, int y, int w, const char *s) { tr("text %d %d %d %s", x, y, w, s); }
void f3A044(int x, int y, const char *s) { tr("print %d %d %s", x, y, s); }
void f3A3A0(int x, int y, const char *s) { tr("right %d %d %s", x, y, s); }
void f3A18C(int x, int y, int w, int color, const char *s) { tr("title %d %d %d %d %s", x, y, w, color, s); }
int f3A42C(int x, int y, int w, uint32_t align, const char *text, int32_t *maxw, int draw)
{
    tr("wrap %d %d %d %u %s %d", x, y, w, (unsigned)align, text, draw);
    *maxw = 0;
    return y + 20;
}
void f3AF8C(uint32_t spr, int x, int y) { tr("sprite %X %d %d", (unsigned)spr, x, y); }
void f3C9D0(int x, int y, int w, int h, int flags) { tr("frame %d %d %d %d %d", x, y, w, h, flags); }
void f3C5D4(uint32_t dlg, int id) { tr("click %X %X", (unsigned)dlg, (unsigned)id); }
void f775F0(void) { tr("release"); w32(MB, 0); }
void f78884(void) { tr("hide"); }
void f788AC(void) { tr("unhide"); }
void f78A20(int t) { tr("wait %d", t); }
int f7BA64(void) { tr("7BA64 %d", g_unatt); return g_unatt; }
void f3B4E0(uint32_t rec) { tr("button %X", (unsigned)rec); }
void f79270(int wait, int n, int t, const char *text, int anim) { tr("msg %d %d %d [%s] %d", wait, n, t, text, anim); }
int fg_vbcopy(int a, int b, int c, int d, int e, int f, int g, int h) { tr("vbcopy %d %d %d %d %d %d %d %d", a, b, c, d, e, f, g, h); return 0; }
int fg_vbopen(int h) { tr("vbopen %d", h); return 0; }
void fg_vbclose(void) { tr("vbclose"); }
int fg_vbpaste(int a, int b, int c, int d, int e, int f) { tr("vbpaste %d %d %d %d %d %d", a, b, c, d, e, f); return 0; }
int fg_setcolor(int c) { tr("color %d", c); return 0; }
int fg_rect(int a, int b, int c, int d) { tr("rect %d %d %d %d", a, b, c, d); return 0; }
void fg_move(int x, int y) { tr("move %d %d", x, y); }
int fg_draw(int x, int y) { tr("draw %d %d", x, y); return 0; }
int fgf_width(const char *s, int n) { (void)s; return 6 * n; }

int f3FB94(int id)   /* dispatch.c's table for the ids of 0xF634C (the panel ids 3..8 do nothing here) */
{
    tr("dispatch %X", (unsigned)id);
    if (id >= 0x8E && id <= 0xA5) { w32(T_SEL, id - 0x8E); return -0x3E; }
    switch (id) {
    case 0xA6: return -0x3F;
    case 0xA7: return -0x40;
    case 0xA8: return -0x41;
    case 0xA9: return -0x42;
    case 0xAA: return -0x43;
    case 0xAB: return -0x44;
    case 0x75: return -0x1D;
    case 0x41: return -0x63;
    case 0x2A: return -1;
    default:   return -100;
    }
}

static const int g_mining_goods[5] = { 4, 12, 11, 16, 18 };
void f194E4(int n)   /* info.h */
{
    int k, c = 1;
    tr("194E4 %d", n);
    for (k = 0; k < 6; k++) { w8(T_MLIST + (uint32_t)k, 0); w8(T_MIDX + (uint32_t)k, 0); }
    if (b8(NAT(n) + 0x29) != 0 && s32(T_MBOUGHT) == 0 && s32(T_MUSED) == 0) {
        w8(T_MFLAG, 1);
        for (k = 0; k < 5; k++)
            if (s8(NAT(n) + (uint32_t)g_mining_goods[k]) > 0) {
                w8(T_MLIST + (uint32_t)c, g_mining_goods[k]);
                w8(T_MIDX + (uint32_t)c, k + 1);
                c++;
            }
        w32(T_MCNT, c);
    } else {
        w8(T_MFLAG, 0);
    }
    w32(T_MCH, 0);
}
static int hand_value(uint32_t R, int plus)
{
    int i, c, s = 0;
    for (i = 1; i <= 18; i++) {
        c = s8(R + (uint32_t)i) + (i == plus ? 1 : 0);
        s += c * c * ((i + 1) >> 1);
    }
    return s;
}
int f195D8(int n)
{
    int best = hand_value(NAT(n), 0), k, v;
    if (b8(T_MFLAG) != 0)
        for (k = 1; k < s32(T_MCNT) && k < 6; k++) {
            v = hand_value(NAT(n), b8(T_MLIST + (uint32_t)k));
            if (v > best)
                best = v;
        }
    tr("195D8 %d = %d", n, best);
    return best;
}
int f6DD98(int n) { int v = hand_value(NAT(n), 0); tr("6DD98 %d = %d", n, v); return v; }
int f17760(int n)
{
    int i, s = 0;
    for (i = 0; i < 24; i++)
        if (s8(NAT(n) + 0x24u + (uint32_t)i) == 1)
            s += cost_of(i);
    tr("17760 %d = %d", n, s);
    return s;
}
void f771F8(int c)   /* rules.h */
{
    int s = c <= 18 ? (c - 1) / 2 : s8(0xCA9C1u + (uint32_t)c);
    uint32_t p = 0xF07B4u + 25u * (uint32_t)s;
    int k = s16(p);
    tr("discard %d -> %d", c, s);
    if (k >= 0 && k < 19)
        w8(p + 6u + (uint32_t)k, c);
    w16(p, k + 1);
}
static void fit_hand(uint32_t R, int w)
{
    int st = 40, ov = 10, width, i, c;
    do {
        width = 0;
        for (i = 1; i <= 18; i++) {
            c = s8(R + (uint32_t)i);
            if (c > 0)
                width += st + (c - 1) * ov;
        }
        width += s8(R + 0x13) * st;
        if (width > w) { st -= 2; if (ov > 3) ov--; }
    } while (width > w && st > -100);
    w32(T_HSTEP, st);
    w32(T_HOVER, ov);
}
void f71FB0(int x, int y, int w, int n) { tr("hand up %d %d %d %d", x, y, w, n); fit_hand(NAT(n), w); }
void f71F2C(int x, int y, int w, int n)
{
    int i, c = 0, st = 40;
    tr("hand down %d %d %d %d", x, y, w, n);
    for (i = 1; i <= 0x13; i++)
        c += s8(NAT(n) + (uint32_t)i);
    while (c * st > w && st > 2)
        st -= 2;
    w32(T_HSTEP, st);
}

/* the click script of 0x162D0's own loop */
void fg_mousepos(int *x, int *y, int *b)
{
    int i, c = s16(DLG), en[64], ne = 0, k, id;
    uint32_t r;
    if (++g_clicks > CUTN) {
        tr("CUT");
        longjmp(g_jb, 1);
    }
    for (i = 0; i < c && i < 64; i++)
        if (s8(DLG + 2u + 20u * (uint32_t)i + 0x10u) == 1)
            en[ne++] = i;
    *b = 1;
    if (g_kforced < g_nforced) {
        *x = g_forced[g_kforced][0];
        *y = g_forced[g_kforced][1];
        g_kforced++;
        tr("mouse %d %d", *x, *y);
        return;
    }
    if (g_clicks > 60) {   /* late: FINISHED when it is on, else ACQUIRE (one of them is always on, see adv.c) */
        r = find_rec(DLG, 0x2A);
        if (r == 0 || s8(r + 0x10) != 1)
            r = find_rec(DLG, 0xA6);
        if (r == 0)
            r = DLG + 2u;
        *x = s16(r) + s16(r + 4) / 2;
        *y = s16(r + 2) + s16(r + 6) / 2;
        tr("mouse %d %d", *x, *y);
        return;
    }
    r = find_rec(DLG, 0xA6);
    if (r != 0 && s8(r + 0x10) == 1 && srnd(100) < 45) {   /* ACQUIRE when it is on */
        *x = s16(r) + 3;
        *y = s16(r + 2) + 3;
    } else if ((r = find_rec(DLG, 0x75)) != 0 && s8(r + 0x10) == 1 && srnd(100) < 40) {   /* a card of the hand */
        *x = s16(r) + (int)srnd(260);
        *y = s16(r + 2) + 5;
    } else if ((r = find_rec(DLG, 0xA8)) != 0 && s8(r + 0x10) == 1 && srnd(100) < 25) {   /* treasury + */
        *x = s16(r) + 3;
        *y = s16(r + 2) + 3;
    } else if ((r = find_rec(DLG, 0x41)) != 0 && s8(r + 0x10) == 1 && srnd(100) < 6) {   /* RESTART when on */
        *x = s16(r) + 3;
        *y = s16(r + 2) + 3;
    } else if (ne == 0 || srnd(100) < 8) {
        *x = (int)srnd(640);
        *y = (int)srnd(480);
    } else {
        k = en[srnd((uint32_t)ne)];
        r = DLG + 2u + 20u * (uint32_t)k;
        id = s16(r + 0x0E);
        if ((id == 0x2A || id == 0x41) && g_clicks < 25 && srnd(100) < 85) {   /* fewer early exits / restarts */
            k = en[srnd((uint32_t)ne)];
            r = DLG + 2u + 20u * (uint32_t)k;
        }
        *x = s16(r) + (int)srnd((uint32_t)(s16(r + 4) > 250 ? 250 : s16(r + 4) > 0 ? s16(r + 4) : 1));
        *y = s16(r + 2) + (int)srnd((uint32_t)(s16(r + 6) > 0 ? s16(r + 6) : 1));
    }
    tr("mouse %d %d", *x, *y);
}

int f78BA4(uint32_t dlg, int map)   /* the wrap-up's wait */
{
    static const int codes[] = { -100, -0x50, -0x53, -0x57, -5, -0x4F, -0x58, -1 };
    int c;
    if (++g_clicks > CUTN) {
        tr("CUT");
        longjmp(g_jb, 1);
    }
    c = srnd(4) == 0 ? -1 : codes[srnd(8)];
    tr("78BA4 %X %d = %d", (unsigned)dlg, map, c);
    return c;
}

/* ---- the recorders (adv.c built with -DADV_TEST) ------------------------------------------------------------------ */

void adv_test_f6FBDC(int n, int disable)
{
    tr("6FBDC %d %d", n, disable);
    if (n >= 0 && n < 10)
        g_tu_seen[n] = s32(T_TU);
}
void adv_test_f6D0C4(int x, int y, int w, int n) { tr("6D0C4 %d %d %d %d", x, y, w, n); fit_hand(NAT(n), w); }
void adv_test_f1A888(int n) { tr("1A888 %d", n); }
int adv_test_f177EC(int n)
{
    int c[24], k = 0, i, r;
    for (i = 0; i < 24; i++)
        if (s8(T_ST + (uint32_t)i) == 2)
            c[k++] = i;
    if (g_np177 > 0) {
        r = g_k177 < g_np177 ? g_p177[g_k177++] : -1;
    } else {
        r = (k == 0 || srnd(100) < 15) ? -1 : c[srnd((uint32_t)k)];
    }
    tr("177EC %d = %d (%d)", n, r, k);
    return r;
}
int adv_test_f1B59C(int n, int need)
{
    int i, h, ok;
    memcpy(DP(R0), DP(NAT(n)), 199);
    if (g_b59c) {   /* part A: no card, the treasury pays */
        for (i = 1; i <= 18; i++)
            w8(R0 + (uint32_t)i, 0);
        w32(T_AITR, need > 0 ? need : 0);
        tr("1B59C %d %d = 1", n, need);
        return 1;
    }
    ok = srnd(100) < 70;
    if (ok) {
        for (i = 1; i <= 18; i++) {
            h = s8(NAT(n) + (uint32_t)i);
            w8(R0 + (uint32_t)i, h > 0 ? (int)srnd((uint32_t)h + 1) : 0);
        }
        h = s8(NAT(n) + 0x3E);
        w32(T_AITR, h > 0 ? (int32_t)srnd((uint32_t)h + 1) : 0);
    }
    tr("1B59C %d %d = %d", n, need, ok);
    return ok;
}
void adv_test_f19700(int n, int count, uint32_t list)
{
    int i;
    char b[64];
    size_t l = 0;
    for (i = 0; i < count && i < 5; i++)
        l += (size_t)snprintf(b + l, sizeof b - l, " %d", s16(list + 2u * (uint32_t)i));
    b[l] = 0;
    tr("19700 %d %d %X%s", n, count, (unsigned)list, b);
}

/* ==== the model: a second reading of the disassembly ============================================================== */

static void m_itoa(int32_t v, char *b) { snprintf(b, 52, "%ld", (long)v); }
static const char *m_metal(void) { return str(D32(0xBDD44u + 4u * (uint32_t)s8(0xF68BDu + D32(T_MCH) + 3u))); }

static int m17EDC(int n, int budget, int draw)
{
    uint32_t R = NAT(n);
    int any = 0, a, h, base, have, st, col, pre;
    if (budget == -1) {
        w32(T_HAND, f195D8(n));
        for (a = 0; a < 24; a++) {
            int sum = 0;
            for (h = 0; h < 24; h++)
                if (b8(R + 0x24u + (uint32_t)h) != 0)
                    sum += s8(0xBD240u + 24u * (uint32_t)h + (uint32_t)a);
            w32(T_CRED + 4u * (uint32_t)a, sum);
        }
        base = s32(T_CARRY) + s32(T_HAND) + s8(R + 0x3E);
    } else {
        base = budget;
    }
    for (a = 0; a < 24; a++) {
        f3C284(DLG, 0x8E + a, D32(0xC7DC4u + 4u * (uint32_t)a));
        if (b8(R + 0x24u + (uint32_t)a) != 0) {
            w8(T_ST + (uint32_t)a, 1);
            f3C284(DLG, 0x8E + a, D32(0xC7E24u + 4u * (uint32_t)a));
            if (draw == 1)
                f3BEA0(DLG, 0x8E + a, 14);
            continue;
        }
        have = base + (budget == -1 ? s32(T_CRED + 4u * (uint32_t)a) : 0);
        pre = s16(0xBD4B0u + 2u * (uint32_t)a);
        if (have >= cost_of(a) && (pre == 0 || b8(R + 0x24u + (uint32_t)pre) != 0)) {
            st = 2;
            col = (b8(0xF0981u) & 1) ? 0x90 : 10;
            any = 1;
        } else {
            st = 0;
            col = 12;
        }
        w8(T_ST + (uint32_t)a, st);
        if (draw == 1)
            f3BEA0(DLG, 0x8E + a, col);
    }
    if (any == 1)
        w16(0xF4092u, 1);
    return any;
}

static int m18AEC(void)
{
    int s = s32(T_SEL), cost = cost_of(s), raw = s32(T_CRED + 4u * (uint32_t)s);
    int capped = raw > cost ? cost : raw, need, t;
    char b[52];
    need = cost - capped - f193AC() - s32(T_TU) - s32(T_CARRY);
    if (s32(T_TU) > 0 && need < 0) {
        t = s32(T_TU) + need;
        w32(T_TU, t < 0 ? 0 : t);
        need = cost - (g_original ? raw : capped) - f193AC() - s32(T_TU) - s32(T_CARRY);
        f3C9D0(0x239, 0x10C, 26, 19, 5);
        m_itoa(s32(T_TU), b);
        fg_setcolor(5);
        f3A0B0(0x239, 0x110, 26, b);
    }
    f3A3A0(0x222, 0x134, need >= 0 ? "Still needed" : "Excess");
    if (need > 0)
        f3C48C(DLG, 0xA8);
    else
        f3C530(DLG, 0xA8);
    return need;
}

static void m_box(int ly, uint32_t label, int by, int32_t v, int ty)
{
    char b[52];
    f3A3A0(0x222, ly, str(label));
    f3C9D0(0x239, by, 26, 19, 5);
    m_itoa(v, b);
    fg_setcolor(5);
    f3A0B0(0x239, ty, 26, b);
}

static void m1818C(int n, int mode)
{
    char b[52];
    int i, s;
    int32_t mw;
    const char *nm;
    f3CD6C((int8_t)s8(0xEE168u));
    fg_vbcopy(0, 639, 0, 479, 0, 479, s32(0xF52CCu), s32(0xF52C8u));
    w16(0x11158Cu, 7); w16(0x11158Eu, 406); w16(0x111590u, 570); w16(0x111592u, 46);
    fg_vbopen(s32(0xF52C8u));
    f3A18C(0, 9, 640, 0xB0, "Acquire Civilization Tools");
    f3A18C(0, 33, 640, 35, str(D32(0xC768Cu + 4u * (uint32_t)n)));
    if (mode == 0)
        f71F2C(7, 406, 570, n);
    else
        f71FB0(7, 406, 570, n);
    if (mode == 1) {
        fg_setcolor(14);
        f3A0B0(585, 415, 50, "POINTS");
        m_itoa(f195D8(n), b);
        f3A0B0(585, 435, 50, b);
        fg_setcolor(5);
    }
    f3AF8C(0xF49ECu + 8u * (uint32_t)n, 575, 366);
    m_itoa(s8(NAT(n) + 0x3E), b);
    fg_setcolor(5);
    f3A044(601, 369, b);
    for (i = 0; i < 24; i++) {
        int x = f3BBD8(DLG, 0x8E + i), y = f3BCF4(DLG, 0x8E + i), w = f3BA30(DLG, 0x8E + i), h = f3BABC(DLG, 0x8E + i);
        int lo = b8(0xBD4E0u + 2u * (uint32_t)i), hi = s8(0xBD4E1u + 2u * (uint32_t)i);
        if (hi != 0) {
            fg_setcolor(hi);
            fg_rect(x - 2, x + w / 2, y - 2, y + h + 1);
            fg_setcolor(lo);
            fg_rect(x + w / 2 + 1, x + w + 1, y - 2, y + h + 1);
        } else {
            fg_setcolor(lo);
            fg_rect(x - 2, x + w + 1, y - 2, y + h + 1);
        }
    }
    w32(T_NEED, 0);
    s = s32(T_SEL);
    if (s >= 0) {
        if (s32(T_PREV) != -1)
            f3B914(DLG, (int16_t)(s32(T_PREV) + 0x8E), 0);
        f3B914(DLG, (int16_t)(s + 0x8E), 1);
        w32(T_PREV, s);
        fg_setcolor(14);
        nm = str(D32(0xC7DC4u + 4u * (uint32_t)s));
        f3A044(277, 156, nm);
        fg_move(275, 166);
        fg_draw(277 + fgf_width(nm, (int)strlen(nm)), 166);
        f3A42C(260, 171, 209, 0, str(D32(0xC7E9Cu + 4u * (uint32_t)s)), &mw, 1);
        if (s8(T_ST + (uint32_t)s) == 2) {
            memcpy(DP(R0), DP(NAT(n)), 199);
            memset(DP(T_GOODS), 0, 19);
            fg_setcolor(5);
            fg_setcolor(5);
            f3A044(281, 81, "Goods toward purchase");
            f3AF8C(0xF51ACu, 553, 268);
            f3AF8C(0xF51B4u, 598, 268);
            m_box(204, 0xB157Au, 199, s32(T_CARRY), 203);
            m_box(227, 0xB1588u, 222, s32(T_CRED + 4u * (uint32_t)s), 226);
            f3A3A0(0x222, 250, "Goods");
            f3C9D0(0x239, 245, 26, 19, 5);
            m_itoa(f193AC(), b);
            fg_setcolor(5);
            f3A0B0(0x239, 249, 26, b);
            m_box(274, 0xB1596u, 268, s32(T_TU), 272);
            w32(T_NEED, m18AEC());
            f3C9D0(0x239, 301, 26, 19, 5);
            m_itoa(s32(T_NEED) < 0 ? -s32(T_NEED) : s32(T_NEED), b);
            fg_setcolor(5);
            f3A0B0(0x239, 305, 26, b);
            if (b8(T_MFLAG) != 0) {
                f3AF8C(0xF51ACu, 478, 169);
                f3AF8C(0xF51B4u, 555, 169);
                f3A0B0(478, 153, 90, "Apply Mining to");
                f3C9D0(493, 169, 60, 19, 5);
                fg_setcolor(5);
                f3A0B0(493, 173, 60, m_metal());
            }
        }
    }
    if (s16(T_GRAIN(n)) != 0) {
        fg_setcolor(5);
        f3A42C(460, 30, 85, 0, "Already used to reduce Famine losses.", &mw, 1);
        for (i = 0; i < s16(T_GRAIN(n)); i++)
            f3AF8C(0xF4C5Cu, 548 + 10 * i, 24);
    } else {
        fg_vbcopy(460, 547, 30, 69, 460, 69, s32(0xF52CCu), s32(0xF52C8u));
        fg_vbcopy(548, 638, 24, 69, 548, 69, s32(0xF52CCu), s32(0xF52C8u));
    }
    fg_vbclose();
    f3C3D0(DLG, 0xA9); f3C3D0(DLG, 0x75); f3C3D0(DLG, 0xA7); f3C3D0(DLG, 0xA8);
    f3C3D0(DLG, 0xA6); f3C3D0(DLG, 0xAA); f3C3D0(DLG, 0xAB);
    s = s32(T_SEL);
    if (s >= 0 && s8(T_ST + (uint32_t)s) == 2) {
        f3C314(DLG, 0xA9); f3C314(DLG, 0x75); f3C314(DLG, 0xA7); f3C314(DLG, 0xA8);
        if (b8(T_MFLAG) != 0) { f3C314(DLG, 0xAA); f3C314(DLG, 0xAB); }
    }
    f3B05C(DLG);
    f3CC14();
    if (s32(T_NEED) < 0) { f3C48C(DLG, 0xA6); f3C530(DLG, 0x2A); }
    else { f3C530(DLG, 0xA6); f3C48C(DLG, 0x2A); }
}

static void m18C68(int n)
{
    char b[52];
    f3CD6C((int8_t)s8(0xEE168u));
    fg_vbcopy(7, 576, 406, 451, 7, 451, s32(0xF52CCu), s32(0xF52C8u));
    fg_vbcopy(585, 634, 415, 444, 585, 444, s32(0xF52CCu), s32(0xF52C8u));
    fg_vbopen(s32(0xF52C8u));
    f71FB0(7, 406, 570, n);
    fg_setcolor(5);
    fg_setcolor(14);
    f39F90(1);
    f3A0B0(585, 415, 50, "POINTS");
    m_itoa(f195D8(n), b);
    f3A0B0(585, 435, 50, b);
    fg_vbclose();
    f78884();
    fg_vbpaste(585, 634, 415, 444, 585, 444);
    fg_vbpaste(7, 576, 406, 451, 7, 451);
    f788AC();
}

static void m18DD8(int n)
{
    char b[52];
    int lw;
    f3CD6C((int8_t)s8(0xEE168u));
    fg_vbopen(s32(0xF52C8u));
    f3C9D0(0x239, 245, 26, 19, 5);
    m_itoa(f193AC(), b);
    fg_setcolor(5);
    f3A0B0(0x239, 249, 26, b);
    f3C9D0(0x239, 268, 26, 19, 5);
    m_itoa(s32(T_TU), b);
    fg_setcolor(5);
    f3A0B0(0x239, 272, 26, b);
    lw = s32(T_LW);
    fg_vbcopy(546 - lw, 545, 308, 319, 546 - lw, 319, s32(0xF52CCu), s32(0xF52C8u));
    w32(T_NEED, m18AEC());
    f3C9D0(0x239, 301, 26, 19, 5);
    m_itoa(s32(T_NEED) < 0 ? -s32(T_NEED) : s32(T_NEED), b);
    fg_setcolor(5);
    f3A0B0(0x239, 305, 26, b);
    if (b8(T_MFLAG) != 0) {
        f3C9D0(493, 169, 60, 19, 5);
        fg_setcolor(5);
        f3A0B0(493, 173, 60, m_metal());
    }
    fg_vbclose();
    fg_vbcopy(601, 630, 369, 380, 601, 380, s32(0xF52CCu), s32(0xF52C8u));
    fg_vbopen(s32(0xF52C8u));
    m_itoa(s8(NAT(n) + 0x3E) - s32(T_TU), b);
    fg_setcolor(5);
    f3A044(601, 369, b);
    fg_vbclose();
    f78884();
    lw = s32(T_LW);
    fg_vbpaste(546 - lw, 546, 308, 320, 546 - lw, 320);
    fg_vbpaste(569, 594, 245, 263, 569, 263);
    fg_vbpaste(569, 594, 268, 286, 569, 286);
    fg_vbpaste(569, 594, 301, 319, 569, 319);
    fg_vbpaste(601, 630, 369, 380, 601, 380);
    if (b8(T_MFLAG) != 0)
        fg_vbpaste(493, 552, 169, 187, 493, 187);
    f788AC();
    if (s32(T_NEED) > 0) { f3C530(DLG, 0xA6); f3C48C(DLG, 0x2A); }
    else { f3C48C(DLG, 0xA6); f3C530(DLG, 0x2A); }
}

static void m19170(void)
{
    int i, j, total, x, c;
    f3CD6C((int8_t)s8(0xEE168u));
    fg_vbcopy(266, 601, 98, 143, 266, 143, s32(0xF52CCu), s32(0xF52C8u));
    w32(T_GOVER, 10);
    w32(T_GSTEP, 40);
    for (;;) {
        total = 0;
        for (i = 1; i <= 18; i++) {
            c = s8(T_GOODS + (uint32_t)i);
            if (c == 0)
                continue;
            total += c == 1 ? s32(T_GSTEP) : s32(T_GSTEP) + (c - 1) * s32(T_GOVER);
        }
        total += s8(0xF67F7u) * s32(T_GSTEP);
        if (total <= 336)
            break;
        w32(T_GSTEP, s32(T_GSTEP) - 2);
        if (s32(T_GOVER) > 3)
            w32(T_GOVER, s32(T_GOVER) - 1);
    }
    fg_vbopen(s32(0xF52C8u));
    x = 266;
    for (i = 1; i <= 18; i++) {
        c = s8(T_GOODS + (uint32_t)i);
        if (c == 1) {
            f3AF8C(0xF4C1Cu + 8u * (uint32_t)i, x, 98);
            x += s32(T_GSTEP);
        } else if (c != 0) {
            for (j = 0; j < c; j++) {
                f3AF8C(0xF4C1Cu + 8u * (uint32_t)i, x, 98);
                x += s32(T_GOVER);
            }
            x += s32(T_GSTEP) - s32(T_GOVER);
        }
    }
    for (j = 0; j < s8(0xF67F7u); j++) {
        f3AF8C(0xF4C1Cu + 8u * (uint32_t)s8(0xF67F8u + (uint32_t)j), x, 98);
        x += s32(T_GSTEP);
    }
    fg_vbclose();
    f78884();
    fg_vbpaste(266, 601, 98, 143, 266, 143);
    f788AC();
}

static int m_strip(uint32_t base, int x, uint32_t stepa, uint32_t overa)
{
    int w = s32(stepa) < 32 ? s32(stepa) : 32, i, j, hit = -100, mx = s32(MX);
    for (i = 1; i <= 18 && hit == -100; i++) {
        int c = s8(base + (uint32_t)i);
        if (c == 0)
            continue;
        if (c == 1) {
            if (mx >= x && mx <= x + w)
                hit = i;
            x += s32(stepa);
            continue;
        }
        for (j = 0; j < c; j++) {
            if (mx >= x && mx <= x + w)
                hit = i;
            x += s32(overa);
        }
        x += s32(stepa) - s32(overa);
    }
    return hit;
}

/* 1: RESTART */
static int m_human(int n, const uint8_t *piles, int *none)
{
    uint32_t R = NAT(n);
    char b[52];
    int i, j, k, code, rel = 1, mx, my, mb, c;

    w32(T_SEL, -1);
    w32(T_PREV, -1);
    w16(T_CNT(n), 0);
    for (i = 0; i < 24; i++) {
        f3B914(DLG, 0x8E + i, 0);
        f3C284(DLG, 0x8E + i, D32(0xC7DC4u + 4u * (uint32_t)i));
    }
    f194E4(n);
    if (m17EDC(n, -1, 1) == 0)
        return 0;
    *none = 0;
    m1818C(n, 0);
    adv_test_f6FBDC(n, 1);
    adv_test_f6D0C4(7, 406, 570, n);
    fg_setcolor(14);
    f3A0B0(585, 415, 50, "POINTS");
    m_itoa(f195D8(n), b);
    f3A0B0(585, 435, 50, b);
    fg_setcolor(5);
    f3CD6C((int8_t)s8(0xEE168u));
    for (;;) {
        if (rel)
            f775F0();
        rel = 1;
        while (s32(MB) == 0) {
            fg_mousepos(&mx, &my, &mb);
            w32(MX, mx); w32(MY, my); w32(MB, mb);
        }
        code = -100;
        for (i = 0; i < s16(DLG); i++) {
            uint32_t r = DLG + 2u + 20u * (uint32_t)i;
            int x = s16(r), y = s16(r + 2);
            if (s8(r + 0x10) == 1 && x <= s32(MX) && s32(MX) < x + s16(r + 4) && y <= s32(MY) && s32(MY) < y + s16(r + 6)) {
                f3C5D4(DLG, s16(r + 0x0E));
                code = f3FB94(s16(r + 0x0E));
                break;
            }
        }
        switch (code) {
        case -1:
            return 0;
        case -0x63:
            if (load_heap_ptr(D32(0xEED78u), 199))
                memcpy(DP(R), load_heap_ptr(D32(0xEED78u), 199), 199);
            memcpy(DP(0xF07B4u), piles, 225);
            w16(T_CNT(n), 0);
            return 1;
        case -0x40:
        case -0x41:
            if (code == -0x40 && s32(T_TU) > 0)
                w32(T_TU, s32(T_TU) - 1);
            if (code == -0x41 && s8(R + 0x3E) > s32(T_TU))
                w32(T_TU, s32(T_TU) + 1);
            m18DD8(n);
            f78A20(150);
            w32(MB, 0);
            rel = 0;
            break;
        case -0x43:
            if (s32(T_MCH) > 0)
                w32(T_MCH, s32(T_MCH) - 1);
            m18DD8(n);
            break;
        case -0x44:
            if (s32(T_MCH) < s32(T_MCNT) - 1)
                w32(T_MCH, s32(T_MCH) + 1);
            m18DD8(n);
            break;
        case -0x3E:
            m1818C(n, 1);
            if (s8(T_ST + (uint32_t)s32(T_SEL)) == 2)
                m18DD8(n);
            break;
        case -0x3F: {
            int s = s32(T_SEL);
            w8(R + 0x24u + (uint32_t)s, 1);
            if (s == 5)
                w32(T_MBOUGHT, 1);
            c = s16(T_CNT(n));
            w16(T_CNT(n), c + 1);
            w16(T_BUY(n) + 2u * (uint32_t)c, s);
            f3BEA0(DLG, (int16_t)(s + 0x8E), 15);
            f3C284(DLG, (int16_t)(s + 0x8E), D32(0xC7E24u + 4u * (uint32_t)s));
            f3C314(DLG, 0x41);
            f3C48C(DLG, 0x2A);
            w32(T_CARRY, s32(T_NEED) < 0 ? -s32(T_NEED) : 0);
            w8(R + 0x3E, b8(R + 0x3E) - b8(T_TU));
            w8(R + 0x99, b8(R + 0x99) + b8(T_TU));
            w32(T_TU, 0);
            for (i = 1; i <= 18; i++)
                for (j = 0; j < s8(T_GOODS + (uint32_t)i); j++) {
                    w8(T_SPENT(n) + (uint32_t)i, b8(T_SPENT(n) + (uint32_t)i) + 1);
                    w8(R + (uint32_t)i, b8(R + (uint32_t)i) - 1);
                    f771F8(i);
                }
            w32(T_HAND, f6DD98(n));
            for (k = 0; k < 24; k++) {
                int st, col, pre = s16(0xBD4B0u + 2u * (uint32_t)k);
                if (b8(R + 0x24u + (uint32_t)k) != 0) {
                    w8(T_ST + (uint32_t)k, 1);
                    continue;
                }
                if (s32(T_CARRY) + s32(T_CRED + 4u * (uint32_t)k) + s32(T_HAND) + s8(R + 0x3E) >= cost_of(k)
                    && (pre == 0 || b8(R + 0x24u + (uint32_t)pre) != 0)) {
                    st = 2;
                    col = (b8(0xF0981u) & 1) ? 0x90 : 10;
                } else {
                    st = 0;
                    col = 12;
                }
                w8(T_ST + (uint32_t)k, st);
                f3BEA0(DLG, 0x8E + k, col);
            }
            k = s32(T_MCH);
            f194E4(n);
            if (k != 0) {
                w32(T_MUSED, 1);
                w8(T_MFLAG, 0);
            }
            m1818C(n, 1);
            break;
        }
        case -0x1D:
        case -0x42:
            k = code == -0x1D ? m_strip(R0, 7, T_HSTEP, T_HOVER) : m_strip(T_GOODS, 266, T_GSTEP, T_GOVER);
            if (k == -100)
                break;
            if (code == -0x1D) {
                w8(R0 + (uint32_t)k, b8(R0 + (uint32_t)k) - 1);
                w8(T_GOODS + (uint32_t)k, b8(T_GOODS + (uint32_t)k) + 1);
            } else {
                w8(R0 + (uint32_t)k, b8(R0 + (uint32_t)k) + 1);
                w8(T_GOODS + (uint32_t)k, b8(T_GOODS + (uint32_t)k) - 1);
            }
            m18C68(0);
            m19170();
            m18DD8(n);
            break;
        default:
            break;
        }
    }
}

static int m_slot(int n, int k) { return s16(T_BUY(n) + 2u * (uint32_t)k); }

static void m_computer(int n, int *none)
{
    uint32_t R = NAT(n);
    int i, j, k, budget, need, used = 0, a;
    char b[52];

    w32(T_SEL, -1); w32(T_PREV, -1); w32(T_TU, 0); w32(T_AITR, 0);
    for (i = 0; i < 24; i++)
        f3B914(DLG, 0x8E + i, 0);
    for (k = 0; k < 3; k++)
        w16(T_BUY(n) + 2u * (uint32_t)k, -1);
    f194E4(n);
    w16(T_CNT(n), 0);
    if (m17EDC(n, -1, 1) != 0) {
        for (k = 0; k < 3; k++) {
            adv_test_f1A888(n);
            w16(T_BUY(n) + 2u * (uint32_t)k, adv_test_f177EC(n));
            if (m_slot(n, k) == -1)
                break;
            if (k == 2) {   /* the third: no ownership trial, and counted only when fixed */
                if (!g_original)
                    w16(T_CNT(n), s16(T_CNT(n)) + 1);
                break;
            }
            w8(R + 0x24u + (uint32_t)m_slot(n, k), 1);
            budget = s8(R + 0x3E) + f195D8(n);
            for (j = 0; j <= k; j++)
                budget += f176BC(n, j) - cost_of(m_slot(n, j));
            w16(T_CNT(n), s16(T_CNT(n)) + 1);
            if (m17EDC(n, budget, 0) == 0)
                break;
        }
        need = cost_of(m_slot(n, 0)) - f176BC(n, 0);
        used += f176BC(n, 0);
        for (k = 1; k < 3; k++)
            if (m_slot(n, k) != -1) {
                need += cost_of(m_slot(n, k)) - f176BC(n, k);
                used += f176BC(n, k);
            }
        if (s16(T_CNT(n)) > 0) {
            for (i = 0; i < s16(T_CNT(n)); i++)
                w8(R + 0x24u + (uint32_t)m_slot(n, i), 0);
            if (adv_test_f1B59C(n, need) == 0) {
                for (i = 0; i < s16(T_CNT(n)); i++)
                    w8(R + 0x24u + (uint32_t)m_slot(n, i), 0);
                w16(T_CNT(n), 0);
            }
        }
    }
    if (s16(T_CNT(n)) <= 0)
        return;
    *none = 0;
    for (i = 1; i <= 18; i++)
        for (j = 0; j < s8(R0 + (uint32_t)i); j++) {
            w8(T_SPENT(n) + (uint32_t)i, b8(T_SPENT(n) + (uint32_t)i) + 1);
            w8(R + (uint32_t)i, b8(R + (uint32_t)i) - 1);
            f771F8(i);
        }
    w8(R + 0x3E, b8(R + 0x3E) - b8(T_AITR));
    w8(R + 0x99, b8(R + 0x99) + b8(T_AITR));
    for (i = 0; i < s16(T_CNT(n)); i++) {
        a = m_slot(n, i);
        w8(R + 0x24u + (uint32_t)a, 1);
        f3BEA0(DLG, (int16_t)(a + 0x8E), 15);
        f3C284(DLG, (int16_t)(a + 0x8E), D32(0xC7E24u + 4u * (uint32_t)a));
    }
    memcpy(DP(T_GOODS + 1), DP(R0 + 1), 18);
    w32(T_SEL, -1);
    m1818C(n, 0);
    m19170();
    fg_setcolor(5);
    f3A044(281, 81, "Goods toward purchase");
    if (b8(T_MFLAG) != 0) {
        f3A0B0(478, 153, 90, "Applied Mining to");
        f3C9D0(493, 169, 60, 19, 5);
        fg_setcolor(5);
        f3A0B0(493, 173, 60, m_metal());
    }
    if (used != 0)
        m_box(227, 0xB151Bu, 222, used, 226);
    f3A3A0(0x222, 250, "Goods Used");
    f3C9D0(0x239, 245, 26, 19, 5);
    m_itoa(f193AC(), b);
    fg_setcolor(5);
    f3A0B0(0x239, 249, 26, b);
    m_box(274, 0xB1533u, 268, s32(T_AITR), 272);
    f3AF8C(0xF49ECu + 8u * (uint32_t)n, 575, 366);
    m_itoa(s8(R + 0x3E), b);
    fg_setcolor(5);
    f3A044(601, 369, b);
    adv_test_f19700(n, s16(T_CNT(n)), T_BUY(n));
}

static void m162D0(void)
{
    uint8_t piles[225];
    int n, i, none = 1, cnt = 0, pbem, type;
    uint32_t R;
    for (i = 1; i <= 9; i++)
        if (b8(NAT(i) + 0x9A) != 0) {
            w16(0xF06B0u + 2u * (uint32_t)cnt, i);
            cnt++;
        }
    w16(0xF06AEu, cnt);
    f783B8(DLG, s32(0xF5968u));
    w32(0xEED78u, (int32_t)load_heap_alloc(199));
    w32(T_LW, fgf_width("Still needed", 12));
    f3BFBC(DLG, 0x41, 0);
    w8(0xEE168u, 0x12);
    for (n = 1; n <= 9; n++) {
        R = NAT(n);
        if (load_heap_ptr(D32(0xEED78u), 199))
            memcpy(load_heap_ptr(D32(0xEED78u), 199), DP(R), 199);
        memcpy(piles, DP(0xF07B4u), 225);
again:
        memset(DP(T_SPENT(n) + 1), 0, 18);
        w32(0xF40F8u, n);
        w32(T_CARRY, 0);
        w32(T_MBOUGHT, 0);
        w32(T_MUSED, 0);
        if (!g_original)
            w32(T_TU, 0);
        if (b8(R + 0x9A) != 0) {
            if (s16(T_GRAIN(n)) != 0)
                w8(R + 8, b8(R + 8) - b8(T_GRAIN(n)));
            pbem = (b8(0xE1624u) & 0x80) != 0;
            type = s8(R + 0x9A);
            if ((type == 3 && !pbem) || (pbem && s8(0xE1622u) == n)) {
                if (m_human(n, piles, &none))
                    goto again;
            } else if (type == 2) {
                m_computer(n, &none);
            } else if (pbem) {
                m17EDC(n, -1, 0);
            }
            if (s16(T_GRAIN(n)) != 0)
                w8(R + 8, b8(R + 8) + b8(T_GRAIN(n)));
        }
        w16(R + 0x5E, f17760(n));
    }
    if (g_original)
        w16(0xF40AEu, 0);
    load_heap_free(D32(0xEED78u));
    w32(0xEED78u, 0);
    if (none)
        f79270(100, s16(0xF06B0u), -200, "", 0);
}

static void m1A3A0(void)
{
    int n, row = 0, j, cnt, step, top, code, found = 0;
    char b[100];
    for (n = 1; n <= 9 && !found; n++)
        found = b8(NAT(n) + 0x9A) != 0 && s16(T_CNT(n)) > 0;
    if (!found)
        return;
    memset(DP(R0), 0, 199);
    f3CD6C(0x19);
    fg_vbcopy(0, 639, 0, 479, 0, 479, s32(0xF52CCu), s32(0xF52C8u));
    fg_vbopen(s32(0xF52C8u));
    f39F90(2);
    snprintf(b, sizeof b, "Tool Acquisition Wrapup");
    f3A18C(0, 6, 640, 0xB0, b);
    fg_setcolor(13);
    f3A0B0(61, 26, 90, "Acquired");
    f3A044(190, 26, "Using");
    for (n = 1; n <= 9; n++) {
        if (b8(NAT(n) + 0x9A) == 0)
            continue;
        top = 51 * row;
        f3AF8C(0xF4ACCu + 8u * (uint32_t)n, (51 - (int)D16(0xF4AD0u + 8u * (uint32_t)n)) / 2 + 5, top + 45);
        fg_setcolor(15);
        f3A0B0(5, top + 47 + (int)D16(0xF4AD2u + 8u * (uint32_t)n), 51, str(D32(0xC768Cu + 4u * (uint32_t)n)));
        cnt = s16(T_CNT(n));
        if (cnt > 0) {
            step = 51 / (cnt + 1);
            for (j = 0; j < cnt; j++) {
                fg_setcolor(13);
                f3A0B0(61, top + 38 + step - 5 + step * j, 90, str(D32(0xC7E24u + 4u * (uint32_t)m_slot(n, j))));
            }
            memcpy(DP(R0 + 1), DP(T_SPENT(n) + 1), 18);
            f71FB0(160, top + 38, 460, 0);
        }
        if (row != 0) {
            fg_move(61, top + 35);
            fg_setcolor(5);
            fg_draw(601, top + 35);
        }
        row++;
    }
    for (j = 0; j < s16(WRAP); j++)
        f3B4E0(WRAP + 2u + 20u * (uint32_t)j);
    fg_vbclose();
    w32(0xF8E78u, WRAP);
    f3CC14();
    do {
        if (f7BA64() == 0) {
            code = f78BA4(WRAP, 0);
        } else {
            f78A20(s16(0xF40BAu) == 0 ? 250 : 1000);
            code = -1;
        }
    } while (code != -1);
}

/* ==== the harness ================================================================================================= */

static uint8_t *g_pristine, *g_base, *g_P, *g_M;
static long g_fail, g_runs;
static uint32_t g_rng = 12345;
static uint32_t rnd(uint32_t n)
{
    g_rng ^= g_rng << 13; g_rng ^= g_rng >> 17; g_rng ^= g_rng << 5;
    return n ? g_rng % n : 0;
}

enum { W_17EDC, W_1818C, W_18DD8, W_18AEC, W_18C68, W_19170, W_162D0, W_1A3A0 };
static int g_arg1, g_arg2, g_arg3;
static void coverage(void);
static int32_t g_ret;

static void run_one(int which, int model)
{
    if (setjmp(g_jb) != 0) {
        if (!model)
            g_cuts++;
        return;
    }
    switch (which) {
    case W_17EDC: g_ret = model ? m17EDC(g_arg1, g_arg2, g_arg3) : f17EDC(g_arg1, g_arg2, g_arg3); break;
    case W_1818C: if (model) m1818C(g_arg1, g_arg2); else f1818C(g_arg1, g_arg2); break;
    case W_18DD8: if (model) m18DD8(g_arg1); else f18DD8(g_arg1); break;
    case W_18AEC: g_ret = model ? m18AEC() : f18AEC(); break;
    case W_18C68: if (model) m18C68(g_arg1); else f18C68(g_arg1); break;
    case W_19170: if (model) m19170(); else f19170(); break;
    case W_162D0: if (model) m162D0(); else f162D0(); break;
    case W_1A3A0: if (model) m1A3A0(); else f1A3A0(); break;
    }
}

/* runs the port on g_base -> g_P and the model -> g_M with the same script seed; compares; returns 1 if equal */
static int compare(int which, uint32_t seed, const char *what)
{
    int32_t rp, rm;
    uint32_t i;
    uint8_t *keep = g_ds;
    size_t a;

    memcpy(g_P, g_base, DS_SIZE);
    memcpy(g_M, g_base, DS_SIZE);
    g_runs++;
    g_ds = g_P; g_log = g_real; g_len = &g_real_len; g_real_len = 0; g_real[0] = 0;
    g_srng = seed; g_clicks = 0; g_heap_n = 0; g_k177 = 0; g_ret = 0; g_kforced = 0;
    run_one(which, 0);
    rp = g_ret;
    g_ds = g_M; g_log = g_model; g_len = &g_model_len; g_model_len = 0; g_model[0] = 0;
    g_srng = seed; g_clicks = 0; g_heap_n = 0; g_k177 = 0; g_ret = 0; g_kforced = 0;
    run_one(which, 1);
    rm = g_ret;
    g_ds = keep;
    coverage();
    if (g_real_len != g_model_len || memcmp(g_real, g_model, g_real_len) != 0) {
        size_t line = 1, start = 0;
        for (a = 0; a < g_real_len && a < g_model_len && g_real[a] == g_model[a]; a++)
            if (g_real[a] == '\n') { line++; start = a + 1; }
        if (g_fail++ < 12) {
            char lp[200], lm[200];
            size_t k;
            for (k = 0; k < 199 && start + k < g_real_len && g_real[start + k] != '\n'; k++) lp[k] = g_real[start + k];
            lp[k] = 0;
            for (k = 0; k < 199 && start + k < g_model_len && g_model[start + k] != '\n'; k++) lm[k] = g_model[start + k];
            lm[k] = 0;
            printf("FAIL %s (seed %u, original %d): trace line %zu: port [%s] model [%s]\n", what, (unsigned)seed,
                   g_original, line, lp, lm);
        }
        return 0;
    }
    if (rp != rm) {
        if (g_fail++ < 12)
            printf("FAIL %s (seed %u): returns port %d model %d\n", what, (unsigned)seed, (int)rp, (int)rm);
        return 0;
    }
    for (i = 0; i < DS_SIZE; i++)
        if (g_P[i] != g_M[i]) {
            if (g_fail++ < 12)
                printf("FAIL %s (seed %u, original %d): byte 0x%X port %02X model %02X\n", what, (unsigned)seed,
                       g_original, (unsigned)(i + DS_BASE), g_P[i], g_M[i]);
            return 0;
        }
    return 1;
}

/* coverage of the port's runs: occurrences of trace lines */
static const char *g_covname[] = { "dispatch 8", "dispatch 9", "dispatch A6", "dispatch A7", "dispatch A8", "dispatch A9",
                                   "dispatch AA", "dispatch AB", "dispatch 75", "dispatch 41", "dispatch 2A", "19700 ",
                                   "1B59C ", "177EC ", "Already used", "Excess", "lcol F634C 8E 144", "CUT", "msg 100",
                                   "discard ", "78BA4 ", "sprite F4C", "rect " };
static long g_cov[32];
static void coverage(void)
{
    size_t i;
    const char *p;
    for (i = 0; i < sizeof g_covname / sizeof g_covname[0]; i++)
        for (p = g_real; (p = strstr(p, g_covname[i])) != NULL; p++)
            g_cov[i]++;
}

static void expect(int ok, const char *what)
{
    if (!ok) {
        g_fail++;
        printf("FAIL %s\n", what);
    }
}

/* the dialog 0xF634C as newgame.c builds it (template 0xBD554, 1002 bytes), without the player buttons */
static void base_dialog(void)
{
    uint8_t *keep = g_ds;
    g_ds = g_base;
    memcpy(DP(DLG), DP(0xBD554u), 1002);
    w32(0xF5968u, s16(DLG));
    g_ds = keep;
}

/* random game state in g_base (on top of whatever it holds) */
static void randomize(int humans_only)
{
    uint8_t *keep = g_ds;
    int n, i, k;
    g_ds = g_base;
    for (n = 1; n <= 9; n++) {
        uint32_t R = NAT(n);
        int t = (int)rnd(10);
        w8(R + 0x9A, t < 1 ? 0 : t < 3 && !humans_only ? 2 : t < 4 && !humans_only ? 1 : 3);
        for (i = 1; i <= 18; i++)
            w8(R + (uint32_t)i, rnd(3) == 0 ? (int)rnd(5) : 0);
        k = (int)rnd(3);
        w8(R + 0x13, k);
        for (i = 0; i < 8; i++) { w8(R + 0x14u + (uint32_t)i, i < k ? 0x13 + (int)rnd(12) : 0); w8(R + 0x1Cu + (uint32_t)i, 0xFF); }
        for (i = 0; i < 24; i++)
            w8(R + 0x24u + (uint32_t)i, rnd(4) == 0);
        w8(R + 0x3E, (int)rnd(rnd(4) == 0 ? 120 : 30));
        w8(R + 0x99, (int)rnd(40));
        w16(T_GRAIN(n), rnd(5) == 0 ? (int)rnd(3) : 0);
        if (s8(R + 8) < s16(T_GRAIN(n)))
            w8(R + 8, s16(T_GRAIN(n)));
    }
    for (i = 0; i < 9; i++) {
        uint32_t p = 0xF07B4u + 25u * (uint32_t)i;
        k = (int)rnd(4);
        w16(p, k);
        for (n = 0; n < 19; n++)
            w8(p + 6u + (uint32_t)n, n < k ? 2 * i + 1 : 0);
    }
    w8(0xE1624u, rnd(6) == 0 ? (b8(0xE1624u) | 0x80) : (b8(0xE1624u) & 0x7F));
    w8(0xE1622u, (int)rnd(10));
    w8(0xF0981u, rnd(2) ? 1 : 0);
    w32(T_TU, rnd(3) == 0 ? (int32_t)rnd(6) : 0);
    w32(T_CARRY, (int32_t)rnd(30));
    w16(0xF40AEu, (int)rnd(2));
    w16(0xF40BAu, rnd(4) == 0);
    g_ds = keep;
}

static int load_save(const char *path)
{
    FILE *fp = fopen(path, "rb");
    int16_t version;
    char desc[64];
    if (fp == NULL)
        return 0;
    f5F05C(&version, desc, fp);
    fclose(fp);
    return 1;
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:\\work\\advciv_run";
    const char *savedir = argc > 2 ? argv[2] : "C:\\work\\advciv_play";
    long cnt[8] = { 0 }, okc[8] = { 0 };
    int i, n, rep, mode;
    char path[600];
    const char *saves[4][2];

    if (state_init(gamedir) != 0)
        return 2;
    g_pristine = malloc(DS_SIZE); g_base = malloc(DS_SIZE); g_P = malloc(DS_SIZE); g_M = malloc(DS_SIZE);
    g_real = malloc(LOGSZ); g_model = malloc(LOGSZ);
    if (!g_pristine || !g_base || !g_P || !g_M || !g_real || !g_model)
        return 2;
    memcpy(g_pristine, g_ds, DS_SIZE);
    g_log = g_real; g_len = &g_real_len;

    /* ---- A. hand vectors ---------------------------------------------------------------------------------------- */
    {
        uint8_t *keep = g_ds;
        int r;
        /* A1 0x17EDC: nation 1 with cards id 1 x3 (9) and id 5 x2 (12), treasury 10, no advance: nothing; treasury 30
           (51): Pottery, Clothmaking, Mysticism (45, 45, 50) in state 2, colour 0x0A (0x90 with the toggle) */
        memcpy(g_base, g_pristine, DS_SIZE); base_dialog();
        g_ds = g_base;
        memset(DP(NAT(1)), 0, 199);
        w8(NAT(1) + 1, 3); w8(NAT(1) + 5, 2); w8(NAT(1) + 0x3E, 10);
        w8(0xF6754u, 0); w32(T_CARRY, 0); w16(0xF4092u, 0);
        g_log = g_real; g_len = &g_real_len; g_real_len = 0;
        r = f17EDC(1, -1, 1);
        expect(r == 0 && s16(0xF4092u) == 0 && s32(T_HAND) == 21, "A1 0x17EDC nothing affordable at 31");
        w8(NAT(1) + 0x3E, 30);
        r = f17EDC(1, -1, 1);
        expect(r == 1 && s16(0xF4092u) == 1, "A1 0x17EDC affordable at 51");
        for (i = 0; i < 24; i++) {
            int want = (i == 0 || i == 1 || i == 19) ? 2 : 0;
            if (s8(T_ST + (uint32_t)i) != want) { expect(0, "A1 state"); break; }
        }
        expect(b8(find_rec(DLG, 0x8E) + 0x0C) == 0x0A && b8(find_rec(DLG, 0x8E + 11) + 0x0C) == 0x0C, "A1 colours");
        w8(0xF0981u, 1);
        f17EDC(1, -1, 1);
        expect(b8(find_rec(DLG, 0x8F) + 0x0C) == 0x90, "A1 Alternate Tool Acquisition Colors");
        w8(0xF0981u, 0);
        /* A2: Pottery held (row 0: 10 to Clothmaking, Metalworking, Agriculture, Roadbuilding, Mining, Engineering,
           Democracy, Monotheism): credits, the upper-case label, state 1; Roadbuilding needs Engineering */
        w8(NAT(1) + 0x24, 1); w8(NAT(1) + 0x3E, 127);
        f17EDC(1, -1, 1);
        expect(s32(T_CRED + 4) == 10 && s32(T_CRED + 8) == 10 && s32(T_CRED + 4 * 7) == 0 && s32(T_CRED + 4 * 16) == 10
               && s32(T_CRED + 4 * 22) == 10 && s32(T_CRED) == 0, "A2 credits of Pottery");
        expect(s8(T_ST) == 1 && D32(find_rec(DLG, 0x8E) + 8) == D32(0xC7E24u) && b8(find_rec(DLG, 0x8E) + 0x0C) == 14,
               "A2 owned");
        expect(s8(T_ST + 4) == 0 && s8(T_ST + 6) == 2, "A2 Roadbuilding needs Engineering");
        w8(NAT(1) + 0x24 + 6, 1);
        f17EDC(1, -1, 0);
        expect(s8(T_ST + 4) == 2, "A2 Roadbuilding with Engineering");
        /* A2b: a given budget ignores the credits: 44 buys nothing of cost 45 although Clothmaking has 10 credit */
        w8(NAT(1) + 0x24 + 6, 0);
        r = f17EDC(1, 44, 0);
        expect(r == 0 && s8(T_ST + 1) == 0, "A2b budget without credits");
        /* A3 0x18AEC: Pottery (45), credit 60, treasury used 2, no goods, carry 0 */
        w32(T_SEL, 0); w32(T_CRED, 60); w32(T_TU, 2); w32(T_CARRY, 0); memset(DP(T_GOODS), 0, 19); w8(T_MFLAG, 0);
        g_original = 0;
        r = f18AEC();
        expect(r == 0 && s32(T_TU) == 0, "A3 0x18AEC capped credit (fixed)");
        w32(T_TU, 2);
        g_original = 1;
        r = f18AEC();
        expect(r == -15 && s32(T_TU) == 0, "A3 0x18AEC raw credit (--original)");
        g_original = 0;
        /* A8 0x19170: 18 single goods: step 40 -> 18, overlap 10 -> 3 */
        for (i = 1; i <= 18; i++) w8(T_GOODS + (uint32_t)i, 1);
        w8(0xF67F7u, 0);
        f19170();
        expect(s32(T_GSTEP) == 18 && s32(T_GOVER) == 3, "A8 0x19170 fit");
        {   /* A8b: these counts reach a width of exactly 337 at step 20, overlap 3: one more step (18) is taken */
            static const int c8[18] = { 1, 5, 0, 1, 1, 4, 2, 4, 4, 0, 0, 1, 3, 2, 3, 1, 1, 0 };
            for (i = 1; i <= 18; i++) w8(T_GOODS + (uint32_t)i, c8[i - 1]);
            f19170();
            expect(s32(T_GSTEP) == 18 && s32(T_GOVER) == 3, "A8b 0x19170 fit at exactly 337");
        }
        {   /* A8c: these counts reach exactly 336 at step 24, overlap 3: that fits, no further step */
            static const int c8c[18] = { 1, 1, 0, 1, 2, 1, 2, 1, 1, 2, 0, 0, 3, 2, 3, 0, 1, 0 };
            for (i = 1; i <= 18; i++) w8(T_GOODS + (uint32_t)i, c8c[i - 1]);
            f19170();
            expect(s32(T_GSTEP) == 24 && s32(T_GOVER) == 3, "A8c 0x19170 fit at exactly 336");
        }
        g_ds = keep;
    }
    {   /* A4..A6 through 0x162D0 */
        uint8_t *keep;
        int ok;
        /* A4: humans 1 and 2; nation 1 scripted by the random script is not controllable, so the treasury left over is
           planted: [0xF67E0] = 2 before the phase, which is exactly what a first human leaves. Nation 1 is out, nation
           2 human with Pottery affordable: 0x6FBDC sees [0xF67E0] = 0 (fixed) or 2 (--original) */
        for (mode = 0; mode < 2; mode++) {
            memcpy(g_base, g_pristine, DS_SIZE); base_dialog();
            keep = g_ds; g_ds = g_base;
            for (n = 1; n <= 9; n++) memset(DP(NAT(n)), 0, 199);
            w8(NAT(2) + 0x9A, 3); w8(NAT(2) + 0x3E, 50);
            w8(0xE1624u, 0); w32(T_TU, 2); w16(0xF40AEu, 1);
            g_ds = keep;
            g_original = mode;
            memset(g_tu_seen, 0xFF, sizeof g_tu_seen);
            ok = compare(W_162D0, 77, "A4 0x162D0");
            expect(ok && g_tu_seen[2] == (mode ? 2 : 0), mode ? "A4 treasury left over (--original 2)" : "A4 treasury left over (fixed 0)");
            /* A5 (same run): 0xF40AE after the phase */
            expect(g_P[0xF40AEu - DS_BASE] == (mode ? 0 : 1),
                   mode ? "A5 0xF40AE cleared (--original)" : "A5 0xF40AE kept (fixed)");
        }
        /* A6: computer nation 3 with treasury 100, cards id 18 x3 and id 17 x2 (hand 117) and nothing held; 0x177EC
           answers Pottery, Clothmaking, Astronomy (the budget after two is 100 + 117 - 90 = 127 >= 80); 0x1B59C pays
           all 170 from the treasury: fixed count 3 and Astronomy held, --original count 2 (paid all the same) */
        for (mode = 0; mode < 2; mode++) {
            memcpy(g_base, g_pristine, DS_SIZE); base_dialog();
            keep = g_ds; g_ds = g_base;
            for (n = 1; n <= 9; n++) memset(DP(NAT(n)), 0, 199);
            w8(NAT(3) + 0x9A, 2); w8(NAT(3) + 0x3E, 100); w8(NAT(3) + 18, 3); w8(NAT(3) + 17, 2);
            w8(0xE1624u, 0);
            g_ds = keep;
            g_original = mode;
            g_np177 = 3; g_p177[0] = 0; g_p177[1] = 1; g_p177[2] = 7; g_b59c = 1;
            ok = compare(W_162D0, 78, "A6 0x162D0");
            g_np177 = 0; g_b59c = 0;
            keep = g_ds; g_ds = g_P;
            expect(ok && s16(T_CNT(3)) == (mode ? 2 : 3) && b8(NAT(3) + 0x24 + 7) == (mode ? 0 : 1)
                   && b8(NAT(3) + 0x24) == 1 && b8(NAT(3) + 0x25) == 1 && s8(NAT(3) + 0x3E) == 100 - (45 + 45 + 80),
                   mode ? "A6 third advance (--original: paid, not held)" : "A6 third advance (fixed: held)");
            g_ds = keep;
            /* A9: the wrap-up of that state: nation 3 alone in play (row 0), step 51 / 4 = 12 with 3 advances */
            if (mode == 0) {
                memcpy(g_base, g_P, DS_SIZE);
                g_unatt = 1;
                ok = compare(W_1A3A0, 79, "A9 0x1A3A0");
                expect(ok && strstr(g_real, "text 61 45 90 POTTERY") != NULL && strstr(g_real, "text 61 57 90 CLOTHMAKING") != NULL
                       && strstr(g_real, "text 61 69 90 ASTRONOMY") != NULL && strstr(g_real, "wait 250") != NULL,
                       "A9 wrap-up rows (y 0x26 + 12 - 5 + 12j)");
                g_unatt = 0;
            }
        }
        /* A7: the hand click through the loop: human 1, cards 3 x1, 5 x2, treasury 45 */
        g_original = 0;
        memcpy(g_base, g_pristine, DS_SIZE); base_dialog();
        keep = g_ds; g_ds = g_base;
        for (n = 1; n <= 9; n++) memset(DP(NAT(n)), 0, 199);
        w8(NAT(1) + 0x9A, 3); w8(NAT(1) + 3, 1); w8(NAT(1) + 5, 2); w8(NAT(1) + 0x3E, 45);
        w8(0xE1624u, 0);
        g_ds = keep;
        /* cards id 3 x1 (7..39) and id 18 x3 (47, 57, 67; step 40, overlap 10), treasury 0, hand 2 + 81 = 83:
           Pottery (73, 61), the hand at x 50 three times (card 18 each time: goods 81, still needed -36), ACQUIRE
           (508, 347), FINISHED (131, 347) */
        keep = g_ds; g_ds = g_base;
        w8(NAT(1) + 5, 0); w8(NAT(1) + 18, 3); w8(NAT(1) + 0x3E, 0);
        n = s16(0xF07B4u + 25u * 8u);
        g_ds = keep;
        g_nforced = 6;
        g_forced[0][0] = 73;  g_forced[0][1] = 61;
        g_forced[1][0] = 50;  g_forced[1][1] = 420;
        g_forced[2][0] = 50;  g_forced[2][1] = 420;
        g_forced[3][0] = 50;  g_forced[3][1] = 420;
        g_forced[4][0] = 508; g_forced[4][1] = 347;
        g_forced[5][0] = 131; g_forced[5][1] = 347;
        ok = compare(W_162D0, 5, "A7 0x162D0");
        g_nforced = 0;
        keep = g_ds; g_ds = g_P;
        expect(ok && b8(T_SPENT(1) + 18) == 3 && b8(NAT(1) + 18) == 0 && b8(NAT(1) + 3) == 1 && b8(NAT(1) + 0x24) == 1
               && strstr(g_real, "right 546 308 Excess") != NULL && strstr(g_real, "text 569 305 26 36") != NULL
               && s16(T_CNT(1)) == 1 && s16(T_BUY(1)) == 0 && s16(0xF07B4u + 25u * 8u) == n + 3
               && strstr(g_real, "dispatch A6") != NULL,
               "A7 three hand clicks on card 18 (Excess 36), ACQUIRE: spent 3, Pottery held, pile 8 + 3");
        if (getenv("ADV_DUMP_A7") != NULL) {
            printf("spent %d hand18 %d hand3 %d pot %d carry %d cnt %d buy %d pile %d/%d\n", b8(T_SPENT(1) + 18), b8(NAT(1) + 18),
                   b8(NAT(1) + 3), b8(NAT(1) + 0x24), (int)s32(T_CARRY), s16(T_CNT(1)), s16(T_BUY(1)), s16(0xF07B4u + 25u * 8u), n);
            fputs(g_real, stdout);
        }
        g_ds = keep;
    }
    printf("A. hand vectors: %ld failures so far\n", g_fail);

    /* ---- B. 0x17EDC ---------------------------------------------------------------------------------------------- */
    for (rep = 0; rep < 40000; rep++) {
        memcpy(g_base, g_pristine, DS_SIZE); base_dialog(); randomize(0);
        g_arg1 = 1 + (int)rnd(9); g_arg2 = rnd(2) ? -1 : (int)rnd(300) - 20; g_arg3 = (int)rnd(3);
        g_original = (int)rnd(2);
        cnt[0]++; okc[0] += compare(W_17EDC, (uint32_t)rep, "B 0x17EDC");
    }
    /* ---- C. the screen helpers -------------------------------------------------------------------------------- */
    for (rep = 0; rep < 20000; rep++) {
        int which = W_1818C + (int)rnd(5);
        memcpy(g_base, g_pristine, DS_SIZE); base_dialog(); randomize(0);
        {
            uint8_t *keep = g_ds;
            g_ds = g_base;
            n = 1 + (int)rnd(9);
            f17EDC(n, -1, 0);   /* states and credits as the screen finds them (both sides start from them) */
            w32(T_SEL, rnd(5) == 0 ? -1 : (int32_t)rnd(24));
            w32(T_PREV, rnd(3) == 0 ? -1 : (int32_t)rnd(24));
            for (i = 1; i <= 18; i++) w8(T_GOODS + (uint32_t)i, rnd(4) == 0 ? (int)rnd(4) : 0);
            w8(0xF67F7u, 0);
            w8(T_MFLAG, (int)rnd(2)); w32(T_MCNT, 1 + (int)rnd(5)); w32(T_MCH, (int32_t)rnd(3));
            for (i = 0; i < 6; i++) { w8(T_MLIST + (uint32_t)i, 4); w8(T_MIDX + (uint32_t)i, (int)rnd(6)); }
            w32(T_LW, (int32_t)rnd(80));
            w32(T_CRED + 4u * (uint32_t)rnd(24), (int32_t)rnd(80));
            if (which == W_18AEC && s32(T_SEL) < 0) w32(T_SEL, 0);
            if (which == W_18DD8 && s32(T_SEL) < 0) w32(T_SEL, 3);
            g_ds = keep;
        }
        g_arg1 = which == W_18C68 ? 0 : n; g_arg2 = (int)rnd(2);
        g_original = (int)rnd(2);
        cnt[1]++; okc[1] += compare(which, (uint32_t)rep, "C helpers");
    }
    /* ---- D. 0x162D0 ---------------------------------------------------------------------------------------------- */
    for (rep = 0; rep < 6000; rep++) {
        memcpy(g_base, g_pristine, DS_SIZE); base_dialog(); randomize(rep % 3 == 0);
        g_original = rep & 1;
        cnt[2]++; okc[2] += compare(W_162D0, (uint32_t)rep * 7u + 1u, "D 0x162D0");
    }
    /* ---- E. 0x1A3A0 ---------------------------------------------------------------------------------------------- */
    for (rep = 0; rep < 6000; rep++) {
        uint8_t *keep;
        memcpy(g_base, g_pristine, DS_SIZE); base_dialog(); randomize(0);
        keep = g_ds; g_ds = g_base;
        for (n = 1; n <= 9; n++) {
            int c = rnd(3) == 0 ? (int)rnd(4) : 0;
            w16(T_CNT(n), c);
            for (i = 0; i < 5; i++) w16(T_BUY(n) + 2u * (uint32_t)i, (int)rnd(24));
            for (i = 1; i <= 18; i++) w8(T_SPENT(n) + (uint32_t)i, rnd(4) == 0 ? (int)rnd(3) : 0);
        }
        g_ds = keep;
        g_unatt = rnd(3) == 0;
        g_original = rep & 1;
        cnt[3]++; okc[3] += compare(W_1A3A0, (uint32_t)rep, "E 0x1A3A0");
    }
    g_unatt = 0;
    /* ---- F. the real saves ------------------------------------------------------------------------------------- */
    saves[0][0] = gamedir; saves[0][1] = "savgame0.dat";
    saves[1][0] = gamedir; saves[1][1] = "savgame1.dat";
    saves[2][0] = savedir; saves[2][1] = "SAVGAMEA.DAT";
    saves[3][0] = savedir; saves[3][1] = "savgameb.dat";
    for (i = 0; i < 4; i++) {
        uint8_t *save_img = malloc(DS_SIZE);
        snprintf(path, sizeof path, "%s/%s", saves[i][0], saves[i][1]);
        memcpy(g_ds, g_pristine, DS_SIZE);
        if (save_img == NULL || !load_save(path)) {
            printf("FAIL F: cannot read %s\n", path);
            g_fail++;
            free(save_img);
            continue;
        }
        memcpy(save_img, g_ds, DS_SIZE);
        for (rep = 0; rep < 150; rep++) {
            memcpy(g_base, save_img, DS_SIZE); base_dialog();
            g_original = rep & 1;
            cnt[4]++; okc[4] += compare(W_162D0, (uint32_t)(rep * 31 + i), path);
            memcpy(g_base, g_P, DS_SIZE);   /* the wrap-up after that phase */
            g_unatt = rep % 5 == 0;
            cnt[5]++; okc[5] += compare(W_1A3A0, (uint32_t)(rep * 17 + i), path);
            g_unatt = 0;
        }
        free(save_img);
    }

    printf("B. 0x17EDC: %ld / %ld\n", okc[0], cnt[0]);
    printf("C. helpers: %ld / %ld\n", okc[1], cnt[1]);
    printf("D. 0x162D0 random: %ld / %ld\n", okc[2], cnt[2]);
    printf("E. 0x1A3A0 random: %ld / %ld\n", okc[3], cnt[3]);
    printf("F. real saves: 0x162D0 %ld / %ld, 0x1A3A0 %ld / %ld\n", okc[4], cnt[4], okc[5], cnt[5]);
    printf("coverage (port traces):");
    for (i = 0; i < (int)(sizeof g_covname / sizeof g_covname[0]); i++)
        printf(" [%s]=%ld", g_covname[i], g_cov[i]);
    printf("\n");
    printf("runs %ld, cut %d, failures %ld\n", g_runs, g_cuts, g_fail);
    return g_fail == 0 ? 0 : 1;
}
