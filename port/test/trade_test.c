/* trade_test [GAMEDIR [PLAYDIR]]: unit checks for module TRADE (trade.c) that need no game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init; default C:\work\advciv_run, only read) and the saves
   savgame0.dat / savgame1.dat (R4, savgame1 all human); PLAYDIR holds SAVGAMEA.DAT and savgameb.dat (default
   C:\work\advciv_play, only read). The four are read with saveload.c's f5F05C as the game does.

   trade.c is built with -DTRADE_TEST: its calls into code that is not ported come to recorders here. TRADEAI's
   0x68730 (a computer declares) writes a plausible offer from the computer's hand, 0x698A4 (a computer answers)
   appends a plausible counter-offer row to one offer, 0x6B0EC (a computer accepts) accepts its best response half of
   the time through the side's own 0x75074 (the port's or the model's); PBEM's 0x53C30 / 0x7399C write a trace line.

   Linked for real: saveload.c (f5F05C, f726DC: the public offer records, game_rand), rules.c (f75CA8, f75D84,
   f6E460) and helpers2.c (f75BD4, f75C08, f75DF0, f75EB4, f75F08). Every other routine trade.c calls is a stub below
   that appends one line to a trace (drawing included); those with data effects apply them on the data object as
   their headers describe them: the dialog records (enable, style, label, x, y, w, pressed, no-flash, the
   f3C940 / f3C8C4 / f3C7A0 save area 0xF8E44, the current dialog 0xF8E78), f783B8's labels and 0xF06C0, f76F14,
   f195D8, the shuffle f76F70 with game_rand f78AE4 on the seed [0xCAC5C]. Clicks (f78BA4) come from a script: a
   seeded choice among the codes the dialog offers (only enabled buttons, read from the data object; mouse x / y for
   the hand and the chart), -100 and codes that do nothing; after a step budget the choice ends each screen (CLEAR
   ALL then DONE in the composer, FINISHED / CONTINUE elsewhere). A run that loops on is cut after 4,000 clicks.

   The model is a second implementation of the 29 routines written from the disassembly (the listing, not trade.c),
   on the same data object and stubs. Each check runs the port, keeps its data object and trace, restores the start
   state, runs the model, and compares both: the whole data object byte for byte (the seed included) and the traces
   line for line. Parts:
     A. hand vectors (expected values written out): 0x75A1C's block and honesty bytes; 0x70984 / 0x716BC on a known
        hand (and a calamity not held, both modes); 0x71768's verdicts and texts (incomplete, missing replacement,
        empty offer, the counter-offer row marks, 0x1105AC in mode 1 both modes); the chart click at y 447 (ignored
        fixed, card 19 with --original); the article of "The responses to ... offer..."; 0x721A0's steps.
     B. the three steps chained (0x6F270, 0x723DC, 0x734E4) on 2,000 random states (players, humans / computers,
        hands, sessions, minimum goods, variants 0x20 of 0xE1625, option 0x80 of 0xF097C, the show-all switch, PBEM),
        each with and without --original.
     C. 0x734E4 alone on 1,500 random counter-offer tables, both modes.
     D. 24 routines called directly, 6,000 calls on random states, both modes.
     E. the chain on the four real saves, 60 click scripts each, both modes.
   Prints each failure (the first differing trace line or byte), the counts per part, and exits 0 when all pass.

   Build: clang -std=c11 -DTRADE_TEST port/test/trade_test.c port/src/trade.c port/src/saveload.c port/src/rules.c
          port/src/helpers2.c port/src/state.c port/src/le_image.c -Iport/src -o trade_test.exe */

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cards.h"
#include "fg.h"
#include "game.h"
#include "helpers2.h"
#include "info.h"
#include "junk.h"
#include "kit.h"
#include "panel.h"
#include "rules.h"
#include "saveload.h"
#include "state.h"
#include "trade.h"
#include "ui.h"

/* ---- addresses (from the listing, written out here) --------------------------------------------------------------- */

#define NAT(n)     (0xF14CCu + 199u * (uint32_t)(n))
#define DLG_DECL   0x111594u
#define DLG_CNT    0x111980u
#define DLG_RESP   0x111D6Cu
#define DLG_ACC    0x112158u
#define DLG_CONT   0xBB800u
#define DLG_BOX    0xBBBECu
#define SEED       0xCAC5Cu

/* ---- traces --------------------------------------------------------------------------------------------------- */

#define LOGSZ (1u << 22)
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

/* ---- the click script ----------------------------------------------------------------------------------------- */

static uint32_t g_srng, g_srng0;
static int g_steps, g_budget = 60, g_toggle;
static jmp_buf g_abort;

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

/* the record of button id in dlg (16-bit compare, the first match), 0 if none */
static uint32_t btn(uint32_t dlg, int id)
{
    int i, n;
    if (!ds_valid(dlg, 2))
        return 0;
    n = S16(dlg);
    for (i = 0; i < n && i < 60; i++)
        if (D16(dlg + 2u + 20u * (uint32_t)i + 0xEu) == (uint16_t)id)
            return dlg + 2u + 20u * (uint32_t)i;
    return 0;
}

static int enabled(uint32_t dlg, int id)
{
    uint32_t r = btn(dlg, id);
    return r != 0 && S8(r + 0x10u) == 1;
}

static int pick(uint32_t dlg)
{
    int over = ++g_steps > g_budget;
    int k, p;

    if (g_steps > 4000)
        stop_run("steps");
    if (dlg == DLG_BOX || dlg == DLG_CONT)
        return over || srnd(3) == 0 ? -1 : -100;
    if (dlg == DLG_DECL || dlg == DLG_RESP) {
        if (over)
            return (g_toggle ^= 1) ? -0x7A : -1;
        k = (int)srnd(17);
        if (k == 0)
            return -100;
        if (k == 1)
            return -5;
        if (k <= 4) {
            if (dlg == DLG_DECL)
                return -0x11 - (int)srnd(4);
            p = (int)srnd(3);
            return p == 0 ? -0x11 : p == 1 ? -0x12 : -0x15;
        }
        if (k <= 9 && enabled(dlg, 0x75)) {
            setD32(0xE1628u, (uint32_t)(7 + (int)srnd(520)));
            return -0x1D;
        }
        if (k <= 12 && enabled(dlg, 0x76)) {
            setD32(0xE1628u, (uint32_t)(0x235 + (int)srnd(0x4C)));
            setD32(0xE162Cu, (uint32_t)(0x18 + (int)srnd(431)));
            return -0x1C;
        }
        if (k == 13)
            return -0x25;
        if (k == 14)
            return -0x7A;
        return -1;
    }
    if (dlg == DLG_CNT) {
        if (over)
            return -1;
        k = (int)srnd(9);
        if (k == 0)
            return -100;
        if (k <= 4) {
            p = (int)srnd(5);
            return enabled(dlg, 0x80 + p) ? -0x26 - p : -100;
        }
        if (k == 5)
            return enabled(dlg, 0x85) ? -0x2B : -5;
        if (k == 6)
            return enabled(dlg, 0x86) ? -0x2C : -5;
        if (k == 7)
            return -5;
        return -1;
    }
    if (dlg == DLG_ACC) {
        if (over) {
            if (enabled(dlg, 0x2A))
                return -1;
            for (p = 0; p < S32(0xF40ECu) && p < 8; p++)
                if (S16(0xF06B0u + 2u * (uint32_t)p) == S32(0xF40F8u))
                    return -0x36 - p;
            return -1;
        }
        k = (int)srnd(10);
        if (k == 0)
            return -100;
        if (k <= 3) {
            p = (int)srnd(3);
            return enabled(dlg, 0x87 + p) ? -0x2F - p : -100;
        }
        if (k == 4) {
            p = (int)srnd((uint32_t)(S32(0xF40ECu) > 0 ? S32(0xF40ECu) : 1));
            return -0x36 - (p & 7);
        }
        if (k == 5)
            return enabled(dlg, 0x85) ? -0x2B : -5;
        if (k == 6)
            return enabled(dlg, 0x86) ? -0x2C : -5;
        if (k == 7 && (D8(0xE1624u) & 0x80) != 0)
            return -0x97 - (int)srnd(21);
        return enabled(dlg, 0x2A) ? -1 : -100;
    }
    return -1;
}

/* part A's fixed click lists take precedence over the picker */
static const int *g_fixed;
static const int *g_fixed_x, *g_fixed_y;
static int g_fixed_n, g_fixed_i;

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
void f3A3A0(int x, int y, const char *s) { tr("right %d %d '%s'", x, y, s); }
int  f3A42C(int x, int y, int w, uint32_t al, const char *t, int32_t *maxw, int draw)
{
    tr("wrap %d %d %d %u '%s' %d", x, y, w, (unsigned)al, t, draw);
    *maxw = 0x22;
    return y + 12 * (1 + (int)(strlen(t) / 30));
}
void f3AF8C(uint32_t spr, int x, int y) { tr("spr %X %d %d", (unsigned)spr, x, y); }
void f3CD6C(int8_t pic) { tr("pic %d", pic); }
void f3CC14(void) { tr("showback"); }
void f3CCBC(void) { tr("screen2back"); }
void f3B05C(uint32_t dlg) { tr("drawdlg %X", (unsigned)dlg); setD32(0xF8E78u, dlg); }
void f3C9D0(int x, int y, int w, int h, int f) { tr("frame %d %d %d %d %d", x, y, w, h, f); }
void f78884(void) { tr("hide"); }
void f788AC(void) { tr("unhide"); }
void f78A20(int t) { tr("wait %d", t); }
void f448EC(int x, int y, int w, int h) { tr("rect %d %d %d %d", x, y, w, h); }
void f71F2C(int x, int y, int w, int n) { tr("hand_down %d %d %d %d", x, y, w, n); }
void f71FB0(int x, int y, int w, int n)
{
    tr("hand_up %d %d %d %d", x, y, w, n);
    setD32(0x11254Cu, 0x28);
    setD32(0x112550u, 0x0A);
}
void f194E4(int n) { tr("mining %d", n); }
int  junk_makegif(int a, int b, int c, int d, const char *name) { tr("makegif %d %d %d %d %s", a, b, c, d, name); return 0; }
int  junk_showgif(const char *name, int f) { tr("showgif %s %d", name, f); return 0; }
void f6FBDC(int n, int d) { tr("reveal %d %d", n, d); }
void f6FE68(int x, int y, int n, int show, int c) { tr("points %d %d %d %d %d", x, y, n, show, c); }
void f71D30(int s) { tr("restore %d", s); }
void f71E34(int s) { tr("toscreen %d", s); }

/* ---- stubs: buttons and dialogs (data effects on the records) ----------------------------------------------------- */

static void bset8(uint32_t dlg, int id, uint32_t off, int v, const char *what)
{
    uint32_t r = btn(dlg, id);
    tr("%s %X %d %d", what, (unsigned)dlg, id, v);
    if (r == 0) {
        tr("Button not found");
        return;
    }
    setD8(r + off, (uint8_t)v);
}
static void bset16(uint32_t dlg, int id, uint32_t off, int v, const char *what)
{
    uint32_t r = btn(dlg, id);
    tr("%s %X %d %d", what, (unsigned)dlg, id, v);
    if (r == 0) {
        tr("Button not found");
        return;
    }
    setD16(r + off, (uint16_t)v);
}
void f3BFBC(uint32_t dlg, int id, int e) { bset8(dlg, id, 0x10, e, "set_en"); }
void f3C314(uint32_t dlg, int id) { bset8(dlg, id, 0x10, 1, "enable"); }
void f3C3D0(uint32_t dlg, int id) { bset8(dlg, id, 0x10, 0, "disable"); }
void f3C0D8(uint32_t dlg, int id, int s) { bset8(dlg, id, 0x11, s, "style"); }
void f3B914(uint32_t dlg, int id, int p) { bset8(dlg, id, 0x12, p, "pressed"); }
void f3C1F4(uint32_t dlg, int id, int f) { bset8(dlg, id, 0x0D, f, "noflash"); }
void f3BB48(uint32_t dlg, int id, int x) { bset16(dlg, id, 0x00, x, "btn_x"); }
void f3BC64(uint32_t dlg, int id, int y) { bset16(dlg, id, 0x02, y, "btn_y"); }
void f3BD80(uint32_t dlg, int id, int w) { bset16(dlg, id, 0x04, w, "btn_w"); }
void f3C284(uint32_t dlg, int id, uint32_t l)
{
    uint32_t r = btn(dlg, id);
    tr("label %X %d %X", (unsigned)dlg, id, (unsigned)l);
    if (r == 0) {
        tr("Button not found");
        return;
    }
    setD32(r + 0x08u, l);
}
static void dlg_off(uint32_t dlg)
{
    int i;
    if (!ds_valid(dlg, 2))
        return;
    for (i = 0; i < S16(dlg) && i < 50; i++) {
        setD8(0xF8E44u + (uint32_t)i, D8(dlg + 20u * (uint32_t)i + 0x12u));
        setD8(dlg + 20u * (uint32_t)i + 0x12u, 0);
    }
}
static void dlg_on(uint32_t dlg)
{
    int i;
    if (!ds_valid(dlg, 2))
        return;
    for (i = 0; i < S16(dlg) && i < 50; i++)
        setD8(dlg + 20u * (uint32_t)i + 0x12u, D8(0xF8E44u + (uint32_t)i));
}
void f3C940(uint32_t dlg) { tr("dlg_off %X", (unsigned)dlg); dlg_off(dlg); }
void f3C8C4(uint32_t dlg) { tr("dlg_on %X", (unsigned)dlg); dlg_on(dlg); }
void f3C7A0(uint32_t dlg) { tr("dlg_on_back %X", (unsigned)dlg); dlg_on(dlg); }
void f783B8(uint32_t dlg, int base)
{
    int p, n;
    tr("players %X %d", (unsigned)dlg, base);
    for (p = 0; p < S32(0xF40ECu) && p < 16; p++) {
        n = S16(0xF06B0u + 2u * (uint32_t)p);
        if (n < 0 || n > 13)
            continue;
        setD32(dlg + 20u * (uint32_t)(base + p) + 0x0Au, D32(0xC768Cu + 4u * (uint32_t)n));
        setD8(dlg + 20u * (uint32_t)(base + p) + 0x0Eu, 0);
        setD8(0xF06C0u + (uint32_t)n, (uint8_t)p);
    }
}
int  f795A8(const char *text)
{
    int r = srnd(2) ? -0x61 : -0x62;
    tr("yesno '%s' -> %d", text, r);
    return r;
}
int  f7BA64(void)
{
    int r = srnd(5) == 0;
    tr("unattended -> %d", r);
    return r;
}
int  f78BA4(uint32_t dlg, int map)
{
    int r;
    if (g_fixed != NULL) {
        if (g_fixed_i >= g_fixed_n)
            stop_run("fixed");
        r = g_fixed[g_fixed_i];
        if (g_fixed_x)
            setD32(0xE1628u, (uint32_t)g_fixed_x[g_fixed_i]);
        if (g_fixed_y)
            setD32(0xE162Cu, (uint32_t)g_fixed_y[g_fixed_i]);
        g_fixed_i++;
    } else {
        r = pick(dlg);
    }
    tr("click %X %d -> %d x %d y %d", (unsigned)dlg, map, r, S32(0xE1628u), S32(0xE162Cu));
    return r;
}

/* ---- stubs: cards and the random numbers (data effects) ------------------------------------------------------------ */

int f78AE4(void)
{
    uint32_t s = D32(SEED) * 0x15A4E35u + 1u;
    setD32(SEED, s);
    return (int)((s >> 16) & 0x7FFF);
}

void f76F70(int n, uint8_t *cards)
{
    uint8_t local[160];
    int count = n, out = 0, i, r;
    tr("shuffle %d", n);
    if (n < 0 || n > 160)
        return;
    for (i = 0; i < count; i++)
        local[i] = cards[i];
    while (count != 0) {
        r = (int)(((uint32_t)f78AE4() * (uint32_t)count) >> 15);
        cards[out++] = local[r];
        for (i = r; count - 1 > i; i++)
            local[i] = local[i + 1];
        count--;
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

/* ---- link stubs (not reached from trade.c) -------------------------------------------------------------------------- */

void f576F8(int k) { (void)k; stop_run("f576F8"); }
void f57CE8(int a, int n, int f, int k) { (void)a; (void)n; (void)f; (void)k; stop_run("f57CE8"); }
void f57E28(int a, int n, int k) { (void)a; (void)n; (void)k; stop_run("f57E28"); }

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

/* ---- recorders for the calls trade.c does not port ----------------------------------------------------------------- */

static void m_75074(int i);
static void m_73D4C(int n);

/* a computer's offer: two cards it holds as truly stated, one card as a bluff with a held replacement, wants */
void trade_test_f68730(int n)
{
    uint32_t r = NAT(n);
    int c, k = 0, j;
    tr("AI f68730 %d", n);
    for (c = 1; c <= 18 && k < 2; c++)
        if (S8(r + (uint32_t)c) > k)
            setD8(r + 0x9Cu + (uint32_t)k++, (uint8_t)c);
    if (k < 2)
        return;
    setD8(r + 0x9Eu, (uint8_t)(1 + srnd(18)));
    for (c = 18; c >= 1; c--)
        if (S8(r + (uint32_t)c) > 2) {
            setD8(r + 0xA6u, (uint8_t)c);
            break;
        }
    for (j = 0; j < 1 + (int)srnd(5); j++)
        setD8(r + 0xA1u + (uint32_t)j, (uint8_t)(1 + srnd(18)));
    setD32(0x1105ACu + 4u * (uint32_t)n, (uint32_t)j);
}

/* a computer answers one random offer it did not make with a counter-offer row from its hand */
void trade_test_f698A4(int n)
{
    uint32_t r = NAT(n), row;
    int i, c, k = 0, rows = S16(0x112548u);
    tr("AI f698A4 %d", n);
    if (S32(0x112560u) <= 0 || rows >= 50 || srnd(3) == 0)
        return;
    i = (int)srnd((uint32_t)S32(0x112560u));
    if (S8(0x110ECCu + 21u * (uint32_t)i + 0x12u) == n)
        return;
    row = 0x111118u + 16u * (uint32_t)rows;
    memset(DP(row), 0, 16);
    for (c = 1; c <= 18 && k < 2; c++)
        if (S8(r + (uint32_t)c) > k)
            setD8(row + (uint32_t)k++, (uint8_t)c);
    setD8(row + 2u, (uint8_t)(1 + srnd(18)));
    setD8(row + 5u, (uint8_t)(1 + srnd(18)));
    setD8(row + 0xDu, D8(0x110ECCu + 21u * (uint32_t)i + 0x12u));
    setD8(row + 0xEu, (uint8_t)n);
    setD8(row + 0xFu, D8(0x110ECCu + 21u * (uint32_t)i + 0x13u));
    setD8(0x110ECCu + 21u * (uint32_t)i + 0x14u, (uint8_t)n);
    setD16(0x112548u, (uint16_t)(rows + 1));
}

/* a computer accepts its best response half of the time, through the side's own 0x75074 */
void trade_test_f6B0EC(int n)
{
    tr("AI f6B0EC %d", n);
    if (S16(0x112568u) > 0 && srnd(2) == 0) {
        if (g_cur == 0)
            f75074(0);
        else
            m_75074(0);
    }
}

void trade_test_pbem(uint32_t addr, int a, int b) { tr("PBEM %X %d %d", (unsigned)addr, a, b); }

/* ============================================================================================================== */
/* The model: a second implementation of the 29 routines, from the disassembly.                                  */
/* ============================================================================================================== */

static int32_t b8(uint32_t a) { return S8(a); }
static int32_t w16(uint32_t a) { return S16(a); }
static int32_t d32(uint32_t a) { return S32(a); }
static void    p8(uint32_t a, int32_t v) { setD8(a, (uint8_t)v); }
static void    p16(uint32_t a, int32_t v) { setD16(a, (uint16_t)v); }
static void    p32(uint32_t a, int32_t v) { setD32(a, (uint32_t)v); }
static const char *mname(int32_t n) { return DSTR(D32(0xC768Cu + 4u * (uint32_t)n)); }
static int32_t mbk(void) { return d32(0xF52C8u); }
static int32_t mbg(void) { return d32(0xF52CCu); }
static int     mpbem(void) { return (D8(0xE1624u) & 0x80) != 0; }
static uint32_t mcard(int32_t c) { return 0xF4C1Cu + (uint32_t)(c * 8); }
static int32_t tab(uint32_t a, int32_t k) { return b8(a + (uint32_t)k); }      /* the slot-group table bytes */
static int32_t sx(int32_t s) { return w16(0x111524u + (uint32_t)(8 * s)); }
static int32_t sy(int32_t s) { return w16(0x111526u + (uint32_t)(8 * s)); }
static uint32_t mrow(int32_t r) { return 0x111118u + (uint32_t)(16 * r); }
static uint32_t moff(int32_t i) { return 0x110ECCu + (uint32_t)(21 * i); }
static uint32_t mresp(int32_t i) { return 0x111498u + (uint32_t)(20 * i); }
static int32_t mshift(void) { return (D16(0xE1624u) & 0x2000) ? 20 : 0; }
static void    mlist(void)
{
    int32_t n, c = 0;
    p16(0xF06AEu, 0);
    for (n = 1; n <= 9; n++)
        if (D8(NAT(n) + 0x9Au) != 0) {
            p16(0xF06B0u + (uint32_t)(2 * c), n);
            c++;
            p16(0xF06AEu, c);
        }
}
static void    mfull(void) { fg_vbcopy(0, 639, 0, 479, 0, 479, mbg(), mbk()); }

static void m_6F95C(int f)
{
    int32_t w = D16(0xF4C28u), h = D16(0xF4C2Au), i;
    for (i = 0; i < 18; i++) {
        int32_t x = 0x235 + 40 * (i % 2), y = 0x18 + 47 * (i / 2);
        f3AF8C(mcard(i + 1), x, y);
        if (f == 1)
            f448EC(x, y, w, h);
    }
}

static void m_6FA54(void)
{
    f39F90(1);
    fg_setcolor(5);
    f3A044(386, 109, "Replace your bluffs with...");
    f3A044(137, 108, "Truly stated");
    f3A044(237, 108, "Bluff");
    f3A044(147, 188, "You want to trade for...");
    fg_setcolor(7);
    f3A044(385, 108, "Replace your bluffs with...");
    f3A044(136, 108, "Truly stated");
    f3A044(236, 108, "Bluff");
    f3A044(146, 188, "You want to trade for...");
}

static void m_6FB2C(void)
{
    f39F90(1);
    fg_setcolor(5);
    f3A044(359, 231, "Replace your bluffs with...");
    f3A044(110, 231, "Truly stated");
    f3A044(210, 231, "Bluff");
    fg_setcolor(7);
    f3A044(358, 230, "Replace your bluffs with...");
    f3A044(109, 230, "Truly stated");
    f3A044(209, 230, "Bluff");
}

static void m_721A0(int x, int y, int w, int n)
{
    uint32_t r = NAT(n);
    int32_t fw = D16(0xF4C28u), fh = D16(0xF4C2Au), tot, i, k, cx;
    p32(0x11254Cu, 40);
    p32(0x112550u, 10);
    for (;;) {
        tot = 0;
        for (i = 1; i <= 18; i++) {
            int32_t c = b8(r + (uint32_t)i);
            if (c == 0)
                continue;
            tot += c == 1 ? d32(0x11254Cu) : (c - 1) * d32(0x112550u) + d32(0x11254Cu);
        }
        tot += b8(r + 0x13u) * d32(0x11254Cu);
        if (tot <= w)
            break;
        p32(0x11254Cu, d32(0x11254Cu) - 2);
        if (d32(0x112550u) > 3)
            p32(0x112550u, d32(0x112550u) - 1);
    }
    cx = x;
    for (i = 1; i <= 18; i++) {
        int32_t c = b8(r + (uint32_t)i);
        if (c == 1) {
            f3AF8C(mcard(i), cx, y);
            f448EC(cx, y, fw, fh);
            cx += d32(0x11254Cu);
        } else if (c != 0) {
            for (k = 0; k < c; k++) {
                f3AF8C(mcard(i), cx, y);
                f448EC(cx, y, fw, fh);
                cx += d32(0x112550u);
            }
            cx += d32(0x11254Cu) - d32(0x112550u);
        }
    }
    for (k = 0; k < b8(r + 0x13u); k++) {
        f3AF8C(mcard(b8(r + 0x14u + (uint32_t)k)), cx, y);
        f448EC(cx, y, fw, fh);
        cx += d32(0x11254Cu);
    }
}

static void m_6FDC8(int n, int fr)
{
    fg_vbopen(mbk());
    f71D30(-1);
    if (fr == 0)
        f71FB0(7, 406, 500, n);
    else
        m_721A0(7, 406, 500, n);
    f78884();
    fg_vbpaste(7, 506, 406, 451, 7, 451);
    f788AC();
    fg_vbclose();
}

static void m_70984(int c)
{
    uint32_t r = NAT(0);
    int32_t i, at = -1;
    if (c <= 18) {
        p8(r + (uint32_t)c, b8(r + (uint32_t)c) - 1);
        return;
    }
    for (i = 0; i < b8(r + 0x13u); i++)
        if (b8(r + 0x14u + (uint32_t)i) == c) {
            at = i;
            break;
        }
    if (at < 0) {
        if (!g_original)
            return;
        at = 0;
    }
    for (i = at; i < b8(r + 0x13u) - 1; i++) {
        p8(r + 0x14u + (uint32_t)i, b8(r + 0x15u + (uint32_t)i));
        p8(r + 0x1Cu + (uint32_t)i, b8(r + 0x1Du + (uint32_t)i));
    }
    p8(r + 0x13u, b8(r + 0x13u) - 1);
    p8(r + 0x14u + (uint32_t)b8(r + 0x13u), 0);
    p8(r + 0x1Cu + (uint32_t)b8(r + 0x13u), -1);
}

static void m_716BC(int c, int m)
{
    uint32_t r = NAT(m);
    int32_t i, at = -1;
    for (i = 0; i < b8(r + 0x13u); i++)
        if (b8(r + 0x14u + (uint32_t)i) == c) {
            at = i;
            break;
        }
    if (at < 0) {
        if (!g_original)
            return;
        at = 0;
    }
    for (i = at; i < b8(r + 0x13u) - 1; i++) {
        p8(r + 0x14u + (uint32_t)i, b8(r + 0x15u + (uint32_t)i));
        p8(r + 0x1Cu + (uint32_t)i, b8(r + 0x1Du + (uint32_t)i));
    }
    p8(r + 0x13u, b8(r + 0x13u) - 1);
}

/* the byte of slot j of group k: mode 1 the current row from start2, mode 0 the nation from start1 */
static uint32_t m_slot_byte(int n, int32_t k, int32_t j)
{
    if (d32(0x110EC8u) == 1)
        return mrow(w16(0x112548u)) + (uint32_t)(tab(0xCA9ACu, k) + j);
    return NAT(n) + 0x9Cu + (uint32_t)(tab(0xCA9A0u, k) + j);
}

static void m_707A0(int n)
{
    int32_t j, c;
    memmove(DP(NAT(0)), DP(NAT(n)), 199);
    if (d32(0x110EC8u) == 1) {
        uint32_t row = mrow(w16(0x112548u));
        for (j = 0; j < b8(0xCA9A4u); j++)
            if ((c = b8(row + (uint32_t)(b8(0xCA9ACu) + j))) != 0)
                m_70984(c);
        for (j = 0; j < b8(0xCA9A8u); j++)
            if ((c = b8(row + (uint32_t)(b8(0xCA9B0u) + j))) != 0)
                m_70984(c);
    } else {
        for (j = 0; j < b8(0xCA9A4u); j++)
            if ((c = b8(NAT(n) + 0x9Cu + (uint32_t)(b8(0xCA9A0u) + j))) != 0)
                m_70984(c);
        for (j = 0; j < b8(0xCA9A7u); j++)
            if ((c = b8(NAT(n) + 0x9Cu + (uint32_t)(b8(0xCA9A3u) + j))) != 0)
                m_70984(c);
    }
    m_6FDC8(0, w16(0xCA9B2u + (uint32_t)(2 * d32(0x112558u))));
}

static void m_put(int32_t s, int c)
{
    fg_vbopen(mbk());
    f3AF8C(mcard(c), sx(s), sy(s));
    fg_vbclose();
    f78884();
    f3AF8C(mcard(c), sx(s), sy(s));
    f788AC();
}

static int m_70074(int c, int n)
{
    int32_t k = d32(0x112558u), j, s = tab(0xCA9ACu, k);
    int8_t cc = (int8_t)c;
    for (j = 0; j < tab(0xCA9A4u, k); j++) {
        uint32_t a = m_slot_byte(n, k, j);
        if (b8(a) == 0) {
            p8(a, cc);
            m_put(s + j, cc);
            return 1;
        }
    }
    return 0;
}

static void m_empty(int32_t s, uint32_t spr)
{
    fg_vbopen(mbk());
    f71D30(s);
    f3AF8C(spr, sx(s), sy(s));
    fg_vbclose();
    f78884();
    f71E34(s);
    f3AF8C(spr, sx(s), sy(s));
    f788AC();
}

static void m_70294(int n, int lit)
{
    int32_t k = d32(0x112558u), j;
    uint32_t spr = lit == 1 ? 0xF4D24u : 0xF4D1Cu;
    int32_t s = tab(0xCA9ACu, k);
    for (j = 0; j < tab(0xCA9A4u, k); j++)
        p8(m_slot_byte(n, k, j), 0);
    for (j = 0; j < tab(0xCA9A4u, k); j++)
        if (b8(m_slot_byte(n, k, j)) == 0)
            m_empty(s + j, spr);
    if (k == 1) {
        if (d32(0x110EC8u) == 1) {
            uint32_t row = mrow(w16(0x112548u));
            for (j = 0; j < b8(0xCA9A8u); j++)
                p8(row + (uint32_t)(b8(0xCA9B0u) + j), 0);
            for (j = 0; j < b8(0xCA9A8u); j++)
                if (b8(row + (uint32_t)(b8(0xCA9B0u) + j)) == 0)
                    m_empty(b8(0xCA9B0u) + j, 0xF4D1Cu);
        } else {
            for (j = 0; j < b8(0xCA9A7u); j++)
                p8(NAT(n) + 0x9Cu + (uint32_t)(b8(0xCA9A3u) + j), 0);
            for (j = 0; j < b8(0xCA9A7u); j++)
                if (b8(NAT(n) + 0x9Cu + (uint32_t)(b8(0xCA9A3u) + j)) == 0)
                    m_empty(b8(0xCA9AFu) + j, 0xF4D1Cu);
        }
    }
    memmove(DP(NAT(0)), DP(NAT(n)), 199);
    m_707A0(n);
}

static void m_grp(int n, int32_t k, uint32_t spr)
{
    int32_t j, s = tab(0xCA9ACu, k);
    for (j = 0; j < tab(0xCA9A4u, k); j++)
        if (b8(m_slot_byte(n, k, j)) == 0)
            f3AF8C(spr, sx(s + j), sy(s + j));
}

static void m_70A6C(int code, int n)
{
    int32_t k = -0x11 - code;
    uint32_t d;
    p32(0x112554u, k);
    if (k == d32(0x112558u))
        return;
    fg_vbopen(mbk());
    if (d32(0x110EC8u) == 0)
        m_6FA54();
    else if (d32(0x110EC8u) == 1)
        m_6FB2C();
    if (d32(0x112558u) != -1)
        m_grp(n, d32(0x112558u), 0xF4D1Cu);
    m_grp(n, k, 0xF4D24u);
    p32(0x112558u, k);
    fg_vbclose();
    d = d32(0x110EC8u) == 0 ? DLG_DECL : DLG_RESP;
    if (d32(0x110EC8u) == 0) {
        if (code == -0x11 || code == -0x12 || code == -0x13 || code == -0x14) {
            m_707A0(n);
            if (code == -0x11) {
                f3C314(d, 0x75);
                f3C3D0(d, 0x76);
            } else if (code == -0x14) {
                f3C3D0(d, 0x76);
                f3C314(d, 0x75);
            } else {
                f3C3D0(d, 0x75);
                f3C314(d, 0x76);
            }
            fg_vbopen(mbk());
            m_6F95C(code == -0x11 || code == -0x14);
            fg_setcolor(15);
            if (code == -0x11)
                f3A044(sx(0), sy(0) - 12, "Truly stated");
            else if (code == -0x12)
                f3A044(sx(2) + 10, sy(2) - 12, "Bluff");
            else if (code == -0x13)
                f3A044(146, 188, "You want to trade for...");
            else
                f3A044(385, 108, "Replace your bluffs with...");
        }
    } else if (code == -0x11 || code == -0x12 || code == -0x15) {
        m_707A0(n);
        if (code == -0x11) {
            f3C314(d, 0x75);
            f3C3D0(d, 0x76);
        } else if (code == -0x15) {
            f3C3D0(d, 0x76);
            f3C314(d, 0x75);
        } else {
            f3C3D0(d, 0x75);
            f3C314(d, 0x76);
        }
        fg_vbopen(mbk());
        m_6F95C(code != -0x12);
        fg_setcolor(15);
        if (code == -0x11)
            f3A044(sx(0), sy(0) - 12, "Truly stated");
        else if (code == -0x12)
            f3A044(sx(2) + 10, sy(2) - 12, "Bluff");
        else
            f3A044(sx(5), sy(5) - 12, "Replace your bluffs with...");
    }
    fg_vbclose();
    f3CC14();
}

static void m_71B68(const char *t)
{
    int32_t mw, h, y, r;
    uint32_t sv;
    h = f3A42C(0, 0, 180, 2, t, &mw, 0) + 45;
    y = (480 - h) / 2;
    junk_makegif(220, 419, y, y + h - 1, "junk");
    sv = D32(0xF8E78u);
    f3C940(sv);
    fg_vbopen(mbk());
    f3C9D0(220, y, 200, h, 3);
    fg_setcolor(5);
    f3A42C(230, y + 10, 180, 2, t, &mw, 1);
    fg_vbclose();
    f3BB48(DLG_BOX, 0x2A, 265);
    f3BC64(DLG_BOX, 0x2A, y + h - 29);
    f3C284(DLG_BOX, 0x2A, 0xB90C8u);
    f3B05C(DLG_BOX);
    f78884();
    fg_vbpaste(220, 419, y, y + h - 1, 220, y + h - 1);
    f788AC();
    do
        r = f78BA4(DLG_BOX, 0);
    while (r != -1);
    setD32(0xF8E78u, sv);
    f3C8C4(sv);
    f78884();
    fg_move(220, y);
    junk_showgif("junk", 3);
    f3CCBC();
    f788AC();
}

static int32_t m_cnt(uint32_t base, int32_t n)
{
    int32_t j, c = 0;
    for (j = 0; j < n; j++)
        c += b8(base + (uint32_t)j) != 0;
    return c;
}

static int m_71768(int n)
{
    char text[400];
    int32_t a, b, d, want = 0, total, ok, r;
    uint32_t o = NAT(n) + 0x9Cu, row = mrow(w16(0x112548u));
    text[0] = 0;
    if (d32(0x110EC8u) == 0) {
        a = m_cnt(o + (uint32_t)b8(0xCA9A0u), b8(0xCA9A4u));
        b = m_cnt(o + (uint32_t)b8(0xCA9A1u), b8(0xCA9A5u));
        want = m_cnt(o + (uint32_t)b8(0xCA9A2u), b8(0xCA9A6u));
        ok = a == 2 && b != 0 && want != 0;
        if (a == 0 && b == 0 && want == 0)
            ok = 1;
        if (!ok)
            strcpy(text, "Your offer is incomplete.\n");
        d = m_cnt(o + (uint32_t)b8(0xCA9A3u), b8(0xCA9A7u));
    } else {
        a = m_cnt(row, b8(0xCA9A4u));
        b = m_cnt(row + 2u, b8(0xCA9A5u));
        ok = a == 2 && b != 0;
        if (a == 0 && b == 0)
            ok = 1;
        if (!ok)
            strcat(text, "Your offer is incomplete.\n");
        d = m_cnt(row + 5u, b8(0xCA9A8u));
    }
    total = b;
    if (d < total) {
        strcat(text, "You must declare a bluff replacement.");
        ok = 0;
    }
    if (!ok) {
        m_71B68(text);
        r = -15;
    } else {
        if (d32(0x110EC8u) == 0 || g_original)
            p32(0x1105ACu + (uint32_t)(4 * n), want);
        r = -1;
    }
    if (r == -1 && d32(0x110EC8u) == 1 && a != 0 && d != 0) {
        uint32_t of = moff(d32(0x11256Cu));
        p8(row + 0xDu, b8(of + 0x12u));
        p8(row + 0xEu, n);
        p8(row + 0xFu, b8(of + 0x13u));
        p8(of + 0x14u, n);
    }
    return r;
}

static void m_7109C(int n, uint32_t dlg)
{
    uint32_t r0 = NAT(0);
    for (;;) {
        int32_t code = f78BA4(dlg, 0);
        f3CD6C((int8_t)b8(0xEE168u));
        if (code == -100)
            continue;
        if (code == -1) {
            if (m_71768(n) == -1)
                return;
        } else if (code >= -0x15 && code <= -0x11) {
            m_70A6C(code, n);
        } else if (code == -0x25) {
            m_70294(n, 1);
        } else if (code == -0x7A) {
            int32_t k = d32(0x112558u), g, j, s;
            for (g = 0; g < 3; g++) {
                if (d32(0x110EC8u) == 0 || (d32(0x110EC8u) == 1 && g != 2)) {
                    p32(0x112558u, g);
                    m_70294(n, 0);
                }
            }
            p32(0x112558u, k);
            s = tab(0xCA9ACu, k);
            for (j = 0; j < tab(0xCA9A4u, k); j++) {
                fg_vbopen(mbk());
                f71D30(s + j);
                f3AF8C(0xF4D24u, sx(s + j), sy(s + j));
                fg_vbclose();
                f78884();
                f71E34(s + j);
                f3AF8C(0xF4D24u, sx(s + j), sy(s + j));
                f788AC();
            }
        } else if (code == -0x1D) {
            int32_t sel = -100, x = 7, cap = d32(0x11254Cu) < 32 ? d32(0x11254Cu) : 32, i, m, mx = d32(0xE1628u);
            int32_t k;
            for (i = 1; i <= 18 && sel == -100; i++) {
                int32_t c = b8(r0 + (uint32_t)i);
                if (c == 1) {
                    if (mx >= x && mx <= x + cap)
                        sel = i;
                    x += d32(0x11254Cu);
                } else if (c != 0) {
                    for (m = 0; m < c; m++) {
                        if (mx >= x && mx <= x + cap)
                            sel = i;
                        x += d32(0x112550u);
                    }
                    x += d32(0x11254Cu) - d32(0x112550u);
                }
            }
            k = d32(0x112558u);
            if (sel == -100 && (k == 3 || k == 4) && b8(r0 + 0x13u) != 0) {
                for (m = 0; m < b8(r0 + 0x13u); m++) {
                    int32_t c = b8(r0 + 0x14u + (uint32_t)m);
                    if (mx >= x && mx <= x + 32 && f75BD4(c)) {
                        sel = c;
                        break;
                    }
                    x += d32(0x11254Cu);
                }
            }
            if (sel == -100)
                continue;
            if (m_70074(sel, n)) {
                if (sel <= 18)
                    p8(r0 + (uint32_t)sel, b8(r0 + (uint32_t)sel) - 1);
                else
                    m_716BC(sel, 0);
            }
            k = d32(0x112558u);
            if (k == 0) {
                int full = d32(0x110EC8u) == 0 ? b8(NAT(n) + 0x9Du) != 0 : b8(mrow(w16(0x112548u)) + 1u) != 0;
                if (full)
                    m_70A6C(-0x12, n);
                else
                    m_6FDC8(0, w16(0xCA9B2u));
            } else if (k == 3) {
                if (d32(0x110EC8u) == 0) {
                    m_6FDC8(0, w16(0xCA9B8u));
                    if (b8(NAT(n) + 0xA5u + (uint32_t)b8(0xCA9A7u)) != 0)
                        m_70A6C(-0x13, n);
                }
            } else {
                m_6FDC8(0, w16(0xCA9B2u + (uint32_t)(2 * k)));
            }
        } else if (code == -0x1C) {
            int32_t dx = d32(0xE1628u) - 0x235, col, sel, g, cnt = 0, j, k;
            if (dx >= 0 && dx <= 32)
                col = 1;
            else if (dx >= 40 && dx <= 72)
                col = 2;
            else
                continue;
            sel = 2 * ((d32(0xE162Cu) - 24) / 47) + col;
            if (!g_original && sel > 18)
                continue;
            k = d32(0x112558u);
            g = k / 3;
            if (d32(0x110EC8u) == 0)
                cnt = m_cnt(NAT(n) + 0x9Cu + (uint32_t)b8(0xCA9A1u + (uint32_t)(3 * g)), b8(0xCA9A5u + (uint32_t)(3 * g)));
            else
                cnt = m_cnt(mrow(w16(0x112548u)) + 2u, b8(0xCA9A5u));
            if (k % 3 == 2 || cnt + 2 < f76F14(n))
                m_70074(sel, n);
            if (d32(0x112558u) == 1) {
                j = w16(0x112542u);
                if (d32(0x110EC8u) == 0) {
                    if (b8(NAT(n) + 0x9Du + (uint32_t)j) != 0)
                        m_70A6C(-0x14, n);
                } else if (b8(mrow(w16(0x112548u)) + 1u + (uint32_t)j) != 0) {
                    m_70A6C(-0x15, n);
                }
            }
        }
    }
}

static void m_6F454(int n)
{
    char b[40];
    int32_t m, k = 0, j;
    f3CD6C((int8_t)b8(0xEE168u));
    mfull();
    fg_vbopen(mbk());
    for (m = 1; m <= 9; m++) {
        if (D8(NAT(m) + 0x9Au) == 0)
            continue;
        f3AF8C(0xF4ACCu + (uint32_t)(8 * m), (53 - (int32_t)D16(0xF4AD0u + (uint32_t)(8 * m))) / 2 + 4, 49 * k + 13);
        f39F90(1);
        fg_setcolor(m == n ? 15 : 5);
        f3A0B0(4, 49 * k + 15 + (int32_t)D16(0xF4AD2u + (uint32_t)(8 * m)), 53, mname(m));
        f3AF8C(0xF4D14u, 59, 49 * k + 6);
        sprintf(b, "%d", f76F14(m));
        f39F90(2);
        fg_setcolor(m == n ? 15 : 5);
        f3A0B0(60, 49 * k + 20, 30, b);
        k++;
    }
    m_6F95C(1);
    p16(0x11158Cu, 7);
    p16(0x11158Eu, 406);
    p16(0x111590u, 500);
    p16(0x111592u, 46);
    for (j = 0; j < w16(0x112542u); j++) {
        f3AF8C(0xF4D1Cu, 385 + 39 * j, 120);
        p16(0x111574u + (uint32_t)(8 * j), 385 + 39 * j);
        p16(0x111576u + (uint32_t)(8 * j), 120);
        p16(0x111578u + (uint32_t)(8 * j), 32);
        p16(0x11157Au + (uint32_t)(8 * j), 46);
    }
    for (j = 0; j < 2; j++) {
        f3AF8C(0xF4D1Cu, 136 + 39 * j, 120);
        p16(0x111524u + (uint32_t)(8 * j), 136 + 39 * j);
        p16(0x111526u + (uint32_t)(8 * j), 120);
        p16(0x111528u + (uint32_t)(8 * j), 32);
        p16(0x11152Au + (uint32_t)(8 * j), 46);
    }
    for (j = 0; j < w16(0x112542u); j++) {
        f3AF8C(0xF4D1Cu, 226 + 39 * j, 120);
        p16(0x111534u + (uint32_t)(8 * j), 226 + 39 * j);
        p16(0x111536u + (uint32_t)(8 * j), 120);
        p16(0x111538u + (uint32_t)(8 * j), 32);
        p16(0x11153Au + (uint32_t)(8 * j), 46);
    }
    f39F90(1);
    fg_setcolor(5);
    for (j = 0; j < 5; j++) {
        f3AF8C(0xF4D1Cu, 136 + 60 * j, 200);
        p16(0x11154Cu + (uint32_t)(8 * j), 136 + 60 * j);
        p16(0x11154Eu + (uint32_t)(8 * j), 200);
        p16(0x111550u + (uint32_t)(8 * j), 32);
        p16(0x111552u + (uint32_t)(8 * j), 46);
        if (j < 4)
            f3A0B0(168 + 60 * j, 220, 30, "or");
    }
    f71F2C(7, 406, 500, n);
    sprintf(b, "Declare Trade Offers - Session %d", w16(0xF5476u));
    f3A18C(0, 8, 640, 0xB0, b);
    f3A18C(0, 30, 640, 0x23, mname(n));
    fg_setcolor(5);
    m_6FA54();
    fg_vbclose();
    f3BFBC(DLG_DECL, 0x103, b8(0x110F74u) != 0);
    f3B05C(DLG_DECL);
    f3CC14();
}

static void m_6F270(void)
{
    int32_t n, mn = (int32_t)(D8(0xE1623u) >> 4);
    mlist();
    f783B8(DLG_DECL, d32(0xF5948u));
    p32(0x110EC8u, 0);
    p16(0x112542u, 1);
    p8(0xEE168u, 9);
    for (n = 1; n <= 9; n++) {
        p32(0x1105ACu + (uint32_t)(4 * n), 0);
        memset(DP(NAT(n) + 0x9Cu), 0, 13);
        if (f76F14(n) < mn) {
            p8(NAT(n) + 0x9Cu, 0);
        } else if (b8(NAT(n) + 0x9Au) != 3) {
            if (f76F14(n) >= (mn < 6 ? 6 : mn))
                trade_test_f68730(n);
        } else if (!mpbem() || b8(0xE1622u) == n) {
            p32(0xF40F8u, n);
            memset(DP(NAT(n) + 0x9Cu), 0, 13);
            m_6F454(n);
            f6FBDC(n, 1);
            f6FE68(515, 415, n, 1, 15);
            p32(0x112558u, -1);
            m_70A6C(-0x11, n);
            m_7109C(n, DLG_DECL);
        }
    }
}

static void m_cards3(uint32_t src, int32_t x0, int32_t y)
{
    int32_t j;
    for (j = 0; j < w16(0x112542u) + 2; j++) {
        int32_t c = b8(src + (uint32_t)j), x = x0 + 39 * j;
        if (c == 0)
            f3AF8C(0xF4D1Cu, x, y);
        else
            f3AF8C(mcard(c), j < 2 ? x : x + mshift(), y);
    }
}

static void m_72928(int n, int up)
{
    char b[40];
    int32_t r, row, j, cnt, i;
    p8(0xEE168u, 12);
    f3CD6C(12);
    mfull();
    fg_vbopen(mbk());
    if (up == 0) {
        p32(0x11254Cu, 40);
        cnt = f76F14(n);
        while (d32(0x11254Cu) * cnt > 570)
            p32(0x11254Cu, d32(0x11254Cu) - 2);
        for (i = 0; i < cnt; i++)
            f3AF8C(0xF4D14u, 7 + i * d32(0x11254Cu), 406);
    } else {
        f71FB0(7, 406, 570, n);
        f6FE68(585, 415, n, 0, 15);
        fg_vbopen(mbk());
    }
    p16(DLG_CNT, w16(0xF5950u));
    fg_setcolor(5);
    for (r = d32(0x11255Cu); r < d32(0x11255Cu) + 5 && r < d32(0x112560u); r++) {
        int32_t a = b8(moff(r) + 0x12u);
        uint32_t bt;
        row = r - d32(0x11255Cu);
        f3AF8C(0xF4ACCu + (uint32_t)(8 * a), (53 - (int32_t)D16(0xF4AD0u + (uint32_t)(8 * a))) / 2 + 3, 50 * row + 108);
        f3A0B0(3, 50 * row + 110 + (int32_t)D16(0xF4AD2u + (uint32_t)(8 * a)), 53, mname(a));
        m_cards3(moff(r), 57, 50 * row + 101);
        for (j = 0; j < 5; j++) {
            int32_t c = b8(moff(r) + 5u + (uint32_t)j);
            f3AF8C(c == 0 ? 0xF4D1Cu : mcard(c), 288 + 39 * j, 50 * row + 101);
        }
        bt = DLG_CNT + 2u + (uint32_t)(20 * (d32(0xF5950u) + row));
        memcpy(DP(bt), DP(0xC9DBCu), 20);
        p16(bt + 2u, w16(bt + 2u) + 50 * row);
        p16(bt + 14u, w16(bt + 14u) + row);
        p8(bt + 16u, b8(moff(r) + 0x12u) != n && b8(moff(r) + 0x14u) != n);
        p16(DLG_CNT, w16(DLG_CNT) + 1);
    }
    sprintf(b, "Make Counter Offers - Session %d", w16(0xF5476u));
    f3A18C(0, 6, 640, 0xB0, b);
    f3A18C(0, 30, 640, 0x23, mname(n));
    fg_setcolor(13);
    f3A044(82, 81, "The offers...");
    f3A044(313, 81, "They want...");
    fg_vbclose();
    f3BFBC(DLG_CNT, 0x103, b8(0x110F74u) != 0);
    f3B05C(DLG_CNT);
    if (d32(0x11255Cu) > 0)
        f3C314(DLG_CNT, 0x85);
    else
        f3C3D0(DLG_CNT, 0x85);
    if (d32(0x11255Cu) + 5 < d32(0x112560u))
        f3C314(DLG_CNT, 0x86);
    else
        f3C3D0(DLG_CNT, 0x86);
    f3CC14();
}

static void m_72FC4(int code, int n)
{
    char b[100];
    int32_t i, j;
    uint32_t o;
    p32(0x11256Cu, -0x26 - code + d32(0x11255Cu));
    p8(0xEE168u, 15);
    p32(0x110EC8u, 1);
    f3CD6C(15);
    mfull();
    fg_vbopen(mbk());
    for (i = 0; i < 18; i++)
        f3AF8C(mcard(i + 1), 0x235 + 40 * (i % 2), 0x18 + 47 * (i / 2));
    o = moff(d32(0x11256Cu));
    m_cards3(o, 109, 117);
    for (j = 0; j < 5; j++) {
        int32_t c = b8(o + 5u + (uint32_t)j);
        f3AF8C(c == 0 ? 0xF4D1Cu : mcard(c), 339 + 39 * j, 117);
    }
    for (i = 0; i < w16(0x112542u); i++) {
        f3AF8C(0xF4D1Cu, 358 + 39 * i, 242);
        p16(0x11154Cu + (uint32_t)(8 * i), 358 + 39 * i);
        p16(0x11154Eu + (uint32_t)(8 * i), 242);
        p16(0x111550u + (uint32_t)(8 * i), 32);
        p16(0x111552u + (uint32_t)(8 * i), 46);
    }
    for (j = 0; j < 2; j++) {
        f3AF8C(0xF4D1Cu, 109 + 39 * j, 242);
        p16(0x111524u + (uint32_t)(8 * j), 109 + 39 * j);
        p16(0x111526u + (uint32_t)(8 * j), 242);
        p16(0x111528u + (uint32_t)(8 * j), 32);
        p16(0x11152Au + (uint32_t)(8 * j), 46);
    }
    for (j = 0; j < w16(0x112542u); j++) {
        f3AF8C(0xF4D1Cu, 199 + 39 * j, 242);
        p16(0x111534u + (uint32_t)(8 * j), 199 + 39 * j);
        p16(0x111536u + (uint32_t)(8 * j), 242);
        p16(0x111538u + (uint32_t)(8 * j), 32);
        p16(0x11153Au + (uint32_t)(8 * j), 46);
    }
    p16(0x11158Cu, 7);
    p16(0x11158Eu, 406);
    p16(0x111590u, 500);
    p16(0x111592u, 46);
    f71FB0(7, 406, 500, n);
    sprintf(b, "Respond To Offer - Session %d", w16(0xF5476u));
    f3A18C(0, 8, 640, 0xB0, b);
    f3A18C(0, 30, 640, 0x23, mname(n));
    fg_setcolor(13);
    sprintf(b, "%s offers for trade...", mname(b8(moff(d32(0x11256Cu)) + 0x12u)));
    f3A044(109, 97, b);
    f3A044(339, 97, "... and wants ...");
    m_6FB2C();
    f6FE68(515, 415, n, 0, 15);
    fg_vbclose();
    f3BFBC(DLG_RESP, 0x103, b8(0x110F74u) != 0);
    f3B05C(DLG_RESP);
    f3CC14();
}

static void m_72EF4(int code, int n)
{
    mlist();
    f783B8(DLG_RESP, d32(0xF5954u));
    memset(DP(mrow(w16(0x112548u))), 0, 8);
    m_72FC4(code, n);
    p32(0x112558u, -1);
    m_70A6C(-0x11, n);
    m_7109C(n, DLG_RESP);
    p16(0x112548u, w16(0x112548u) + 1);
}

static void m_723DC(void)
{
    int32_t n, code, i, mn;
    uint8_t ord[16];
    mlist();
    f783B8(DLG_CNT, d32(0xF594Cu));
    p16(0x112542u, 1);
    memset(DP(0x111118u), 0, 0x380);
    p16(0x112548u, 0);
    f726DC();
    if (d32(0x112560u) == 0)
        return;
    p8(0xEE168u, 2);
    mn = (int32_t)(D8(0xE1623u) >> 4);
    for (n = 1; n <= 9; n++) {
        if (f76F14(n) < mn)
            continue;
        if (b8(NAT(n) + 0x9Au) != 3) {
            trade_test_f698A4(n);
            p8(0xF09D4u + (uint32_t)n, 1);
            continue;
        }
        if (mpbem() && b8(0xE1622u) != n)
            continue;
        p32(0xF40F8u, n);
        p32(0x11255Cu, 0);
        m_72928(n, 0);
        f6FBDC(n, 1);
        m_72928(n, 1);
        for (;;) {
            code = f78BA4(DLG_CNT, 0);
            if (code == -100)
                continue;
            if (code == -1)
                break;
            if (code == -0x2C)
                p32(0x11255Cu, d32(0x11255Cu) + 5);
            else if (code == -0x2B)
                p32(0x11255Cu, d32(0x11255Cu) - 5);
            else if (code >= -0x2A && code <= -0x26)
                m_72EF4(code, n);
            else
                continue;
            m_72928(n, 1);
        }
        if (!mpbem() && !(D8(0xF097Cu) & 0x80)) {
            p8(0xF09D4u + (uint32_t)n, 1);
        } else if (w16(0xF5476u) < (int32_t)(D8(0xE1623u) & 15)) {
            code = f795A8("Do you want another round of trading?");
            if (code == -0x61)
                p8(0xF09D4u + (uint32_t)n, 1);
            else if (code == -0x62)
                p8(0xF09D4u + (uint32_t)n, 0);
        }
    }
    mlist();
    for (i = 0; i < d32(0xF40ECu); i++)
        ord[i] = D8(0xF06B0u + (uint32_t)(2 * i));
    f76F70(d32(0xF40ECu), ord);
    for (i = 0; i < d32(0xF40ECu); i++)
        p16(0xF06B0u + (uint32_t)(2 * i), (int8_t)ord[i]);
    if (mpbem())
        trade_test_pbem(0x53C30u, 0, 0);
}

static void m_73D4C(int n)
{
    uint32_t rec = NAT(n);
    int32_t v0, r, j, k, m;
    uint8_t t[20];
    p16(0x112568u, 0);
    memset(DP(0x111498u), 0, 140);
    f194E4(n);
    v0 = f195D8(n);
    for (r = 0; r < w16(0x112548u); r++) {
        if (b8(mrow(r) + 13u) != n)
            continue;
        k = w16(0x112568u);
        p8(mresp(k) + 18u, r);
        p8(mresp(k) + 19u, 0);
        memcpy(DP(mresp(k)), DP(mrow(r)), 16);
        memmove(DP(NAT(0)), DP(rec), 199);
        f75CA8(b8(rec + 0x9Cu), NAT(0));
        f75CA8(b8(rec + 0x9Du), NAT(0));
        f75CA8(b8(rec + 0xA6u), NAT(0));
        f75CA8(b8(rec + 0xA7u), NAT(0));
        f75CA8(b8(rec + 0xA8u), NAT(0));
        for (j = 0; j < 5; j++)
            f75D84(b8(mresp(k) + 8u + (uint32_t)j), NAT(0), NAT(0));
        f194E4(0);
        m = f195D8(0) - v0;
        p16(0x112568u, k + 1);
        p16(mresp(k) + 16u, m);
    }
    for (m = w16(0x112568u) - 2; m >= 0; m--)
        for (j = 0; j <= m; j++)
            if (w16(mresp(j + 1) + 16u) > w16(mresp(j) + 16u)) {
                memcpy(t, DP(mresp(j + 1)), 20);
                memcpy(DP(mresp(j + 1)), DP(mresp(j)), 20);
                memcpy(DP(mresp(j)), t, 20);
            }
}

static void m_74D54(void)
{
    int32_t r, j;
    for (r = 0; r < 3; r++) {
        f3BFBC(DLG_ACC, 0x87 + r, 0);
        f3C0D8(DLG_ACC, 0x87 + r, 2);
        if (mpbem()) {
            f3C284(DLG_ACC, 0x87 + r, 0xB92CDu);
            f3BB48(DLG_ACC, 0x87 + r, 378);
            f3BD80(DLG_ACC, 0x87 + r, 50);
            f3C1F4(DLG_ACC, 0x87 + r, 1);
        } else {
            f3C284(DLG_ACC, 0x87 + r, 0xB92C0u);
            f3BB48(DLG_ACC, 0x87 + r, 408);
            f3BD80(DLG_ACC, 0x87 + r, 110);
            f3C1F4(DLG_ACC, 0x87 + r, 0);
            for (j = 0; j < 7; j++)
                f3C0D8(DLG_ACC, 0xEC + 3 * j + r, 2);
        }
        for (j = 0; j < 7; j++)
            f3BFBC(DLG_ACC, 0xEC + 3 * j + r, 0);
    }
}

static void m_75000(void)
{
    if (d32(0x112564u) > 0)
        f3C314(DLG_ACC, 0x85);
    else
        f3C3D0(DLG_ACC, 0x85);
    if (d32(0x112564u) + 3 < w16(0x112568u))
        f3C314(DLG_ACC, 0x86);
    else
        f3C3D0(DLG_ACC, 0x86);
}

static void m_707A0(int n);

static void m_7401C(int n, int mode)
{
    char b[100];
    uint32_t rec = NAT(n);
    int32_t ro, cnt, i, j, r, idx, any;
    ro = (b8(rec + 0x9Au) == 2 || mode == 0) ? 1 : d32(0xF40F8u) != n;
    p8(0xEE168u, 13);
    p16(0x112542u, 1);
    f3CD6C(13);
    mfull();
    m_74D54();
    fg_vbopen(mbk());
    p16(0x111590u, 570);
    p16(0x111592u, 46);
    p16(0x11158Cu, 7);
    p16(0x11158Eu, 406);
    if (ro || mode == 0) {
        p32(0x11254Cu, 40);
        cnt = f76F14(n);
        while (d32(0x11254Cu) * cnt > 570)
            p32(0x11254Cu, d32(0x11254Cu) - 2);
        for (i = 0; i < cnt; i++)
            f3AF8C(0xF4D14u, 7 + i * d32(0x11254Cu), 406);
    } else {
        m_707A0(n);
        f6FE68(585, 415, n, 0, 15);
        fg_vbopen(mbk());
        fg_setcolor(13);
        f3A044(7, 384, "Does not");
        f3A044(7, 394, "include initial offer");
    }
    if (!ro || (b8(rec + 0x9Au) == 2 && w16(0xF40C2u) != 0)) {
        for (j = 0; j < 2; j++)
            f3AF8C(mcard((int8_t)D8(rec + 0x9Cu + (uint32_t)(b8(0xCA9A0u) + j))), 207 + 39 * j, 88);
        for (j = 0; j < b8(0xCA9A7u); j++) {
            int32_t c = (int8_t)D8(rec + 0x9Cu + (uint32_t)(b8(0xCA9A3u) + j));
            if (c != 0)
                f3AF8C(mcard(c), 307 + 39 * j, 88);
        }
    } else {
        idx = -100;
        for (i = 0; i < d32(0x112560u); i++)
            if (b8(moff(i) + 0x12u) == n) {
                idx = i;
                break;
            }
        if (idx != -100)
            m_cards3(moff(idx) + 13u, 207, 88);
        else
            for (j = 0; j < w16(0x112542u) + 2; j++)
                f3AF8C(0xF4D1Cu, 207 + 39 * j, 88);
    }
    p16(0x112546u, 0);
    p16(DLG_ACC, w16(0xF595Cu));
    for (r = d32(0x112564u); r < d32(0x112564u) + 3 && r < w16(0x112568u); r++) {
        int32_t row = r - d32(0x112564u), bb = b8(mresp(r) + 14u), pref = b8(mresp(r) + 19u);
        f3AF8C(0xF4ACCu + (uint32_t)(8 * bb), (53 - (int32_t)D16(0xF4AD0u + (uint32_t)(8 * bb))) / 2 + 57, 50 * row + 195);
        f3A0B0(57, 50 * row + 197 + (int32_t)D16(0xF4AD2u + (uint32_t)(8 * bb)), 53, mname(bb));
        m_cards3(mresp(r) + 8u, 129, 50 * row + 190);
        p16(0x112546u, w16(0x112546u) + 1);
        if (ro)
            continue;
        sprintf(b, "%+d", w16(mresp(r) + 16u));
        f3A0B0((mpbem() ? 149 : 169) + 129, 50 * row + 210, 92, b);
        f3BFBC(DLG_ACC, 0x87 + row, 1);
        f3C0D8(DLG_ACC, 0x87 + row, 1);
        f3B914(DLG_ACC, 0x87 + row, mpbem() && pref == 0);
        if (mpbem()) {
            f3BFBC(DLG_ACC, 0xEC + row, 1);
            for (j = 0; j < 7; j++)
                f3C0D8(DLG_ACC, 0xEC + 3 * j + row, 1);
        }
        f3B914(DLG_ACC, 0xEC + row, pref == 1);
        for (j = 1; j < 7; j++)
            if (w16(0x112568u) > j) {
                f3BFBC(DLG_ACC, 0xEC + 3 * j + row, 1);
                f3B914(DLG_ACC, 0xEC + 3 * j + row, pref == j + 1);
            }
    }
    if (w16(0x112546u) == 0) {
        f39F90(2);
        fg_setcolor(13);
        f3A0B0(0, 210, 640, "N O N E");
        f39F90(1);
    }
    if (!ro) {
        any = 0;
        for (i = 0; i < w16(0x112568u); i++)
            if (b8(mresp(i) + 19u) != 0) {
                any = 1;
                break;
            }
        if (w16(0x112546u) != 0 && !any) {
            f3C284(DLG_ACC, 0x2A, 0xB91D3u);
            p16(0xF4100u, 0);
        } else if (mpbem() && any) {
            f3C284(DLG_ACC, 0x2A, 0xB91E5u);
            p16(0xF4100u, 1);
        } else {
            f3C284(DLG_ACC, 0x2A, 0xB91FCu);
        }
    } else {
        f3C284(DLG_ACC, 0x2A, 0xB9205u);
    }
    sprintf(b, "Accept Trades - Session %d", w16(0xF5476u));
    f3A18C(0, 8, 640, 0xB0, b);
    f3A18C(0, 30, 640, 0x23, mname(n));
    fg_setcolor(13);
    if (!ro) {
        f3A044(307, 76, "Bluffs replaced by...");
        f3A044(207, 76, "Truly Stated");
        f3A044(217, 58, "You initially offered...");
        if (mpbem())
            f3A0B0(378, 170, 246, "Trade Acceptance Order Preference");
        f3A044((mpbem() ? 161 : 181) + 129, 170, "If Not Lying");
    } else {
        f3A044(217, 76, "Offers...");
    }
    if (!ro)
        sprintf(b, "The responses to %s offer...", "your");
    else if (g_original)
        sprintf(b, "The responses to %s offer...", DSTR(D32(0xC76C4u + 4u * (uint32_t)n)));
    else
        sprintf(b, "The responses to the %s offer...", DSTR(D32(0xC76C4u + 4u * (uint32_t)n)));
    f3A044(49, 170, b);
    fg_vbclose();
    f3B05C(DLG_ACC);
    m_75000();
    if (!ro)
        f3C314(DLG_ACC, 0x2A);
    else
        f3C3D0(DLG_ACC, 0x2A);
    f3CC14();
}

static void m_75A1C(uint32_t rec, int r)
{
    int32_t i;
    uint32_t row = mrow(r);
    memset(DP(0x112570u), 0, 18);
    p8(0x112570u, b8(rec + 0x9Cu));
    p8(0x112571u, b8(rec + 0x9Du));
    for (i = 0; i < 3; i++) {
        p8(0x112572u + (uint32_t)i, b8(rec + 0x9Cu + (uint32_t)(b8(0xCA9A3u) + i)));
        p8(0x112575u + (uint32_t)i, b8(rec + 0x9Cu + (uint32_t)(b8(0xCA9A1u) + i)));
    }
    p8(0x112578u, 1);
    for (i = 2; i < 5; i++)
        if (D8(0x112570u + (uint32_t)i) != D8(0x112573u + (uint32_t)i)) {
            p8(0x112578u, -1);
            break;
        }
    p8(0x112579u, b8(row));
    p8(0x11257Au, b8(row + 1u));
    for (i = 0; i < 3; i++) {
        p8(0x11257Bu + (uint32_t)i, b8(row + 5u + (uint32_t)i));
        p8(0x11257Eu + (uint32_t)i, b8(row + 2u + (uint32_t)i));
    }
    p8(0x112581u, 1);
    for (i = 2; i < 5; i++)
        if (D8(0x112579u + (uint32_t)i) != D8(0x11257Cu + (uint32_t)i)) {
            p8(0x112581u, -1);
            break;
        }
}

static void m_wait(void)
{
    int32_t r;
    for (;;) {
        if (f7BA64() == 0) {
            r = f78BA4(DLG_CONT, 0);
        } else {
            f78A20(w16(0xF40BAu) == 0 ? 250 : 1000);
            r = -1;
        }
        if (r == -1)
            return;
    }
}

static void m_row3(uint32_t src, int32_t y)
{
    int32_t j;
    for (j = 0; j < b8(0xCA9A7u); j++) {
        int32_t c = b8(src + (uint32_t)j);
        f3AF8C(c == 0 ? 0xF4D1Cu : mcard(c), 200 + 39 * (j + 2), y);
    }
}

static void m_75520(int side, int n, int p, const uint8_t *pre)
{
    char b[100];
    int32_t o = side == 0, j;
    p8(0xEE168u, 14);
    f3CD6C(14);
    mfull();
    fg_vbopen(mbk());
    f3A18C(0, 6, 640, 0xB0, "Trade Details");
    f3A18C(0, 30, 640, 0x23, mname(n));
    fg_vbclose();
    if (side == 1) {
        f3CC14();
        f6FBDC(n, 0);
    }
    fg_vbopen(mbk());
    f39F90(1);
    fg_setcolor(13);
    f3A044(30, 49, "Pre-Trade");
    memmove(DP(NAT(0)), pre, 199);
    f71FB0(55, 60, 460, 0);
    f6FE68(515, 70, 0, 1, 13);
    fg_vbclose();
    fg_vbopen(mbk());
    fg_setcolor(13);
    f3A3A0(190, 140, "You traded");
    for (j = 0; j < 2; j++)
        f3AF8C(mcard(b8(0x112570u + (uint32_t)(9 * side + j))), 200 + 39 * j, 120);
    m_row3(0x112572u + (uint32_t)(9 * side), 120);
    sprintf(b, " to %s", mname(p));
    f3A3A0(190, 180, b);
    f3A3A0(190, 220, "You received");
    for (j = 0; j < 2; j++)
        f3AF8C(mcard(b8(0x112570u + (uint32_t)(9 * o + j))), 200 + 39 * j, 200);
    m_row3(0x112572u + (uint32_t)(9 * o), 200);
    sprintf(b, "%s was bluffing with", mname(p));
    f3A3A0(250, 280, b);
    m_row3(0x112575u + (uint32_t)(9 * o), 260);
    f3A044(30, 329, "Post-Trade");
    f71FB0(55, 340, 460, n);
    fg_vbclose();
    f6FE68(515, 340, n, 1, 13);
    f3BB48(DLG_CONT, 0x2A, 265);
    f3BC64(DLG_CONT, 0x2A, 430);
    f3B05C(DLG_CONT);
    f3CC14();
    m_wait();
}

static void m_75FB4(int a)
{
    char b[52];
    int32_t t = b8(0x110F74u) - 1, na, nb, j;
    (void)a;
    p8(0xEE168u, 14);
    f3CD6C(14);
    mfull();
    fg_vbopen(mbk());
    sprintf(b, "Trade Summary - Session %d", w16(0xF5476u));
    f3A18C(0, 6, 640, 0xB0, b);
    fg_setcolor(5);
    f39F90(2);
    f3A0B0(0, 235, 640, "<-->");
    f39F90(1);
    na = b8(0x1110B5u + (uint32_t)t);
    f3AF8C(0xF4ACCu + (uint32_t)(8 * na), (53 - (int32_t)D16(0xF4AD0u + (uint32_t)(8 * na))) / 2 + 245, 225);
    f3A0B0(245, 227 + (int32_t)D16(0xF4AD2u + (uint32_t)(8 * na)), 53, mname(na));
    for (j = 0; j < 5; j++) {
        int32_t c = b8(0x110F75u + (uint32_t)(5 * t + j));
        if (c)
            f3AF8C(mcard(c), 203 - 39 * j, 220);
    }
    nb = b8(0x1110D5u + (uint32_t)t);
    f3AF8C(0xF4ACCu + (uint32_t)(8 * nb), (53 - (int32_t)D16(0xF4AD0u + (uint32_t)(8 * nb))) / 2 + 340, 225);
    f3A0B0(340, 227 + (int32_t)D16(0xF4AD2u + (uint32_t)(8 * nb)), 53, mname(nb));
    for (j = 0; j < 5; j++) {
        int32_t c = b8(0x111015u + (uint32_t)(5 * t + j));
        if (c)
            f3AF8C(mcard(c), 405 + 39 * j, 220);
    }
    fg_vbclose();
    f3BB48(DLG_CONT, 0x2A, 265);
    f3BC64(DLG_CONT, 0x2A, 430);
    f3B05C(DLG_CONT);
    f3CC14();
    m_wait();
}

static void m_pass(uint32_t rx, int32_t x, int32_t y)
{
    int32_t m;
    p8(rx + 0xBDu + (uint32_t)y, 1);
    for (m = 1; m <= 9; m++)
        if (b8(NAT(m) + 0x9Au) == 2 && m != y && D8(NAT(m) + 0xBDu + (uint32_t)x) != 0) {
            p8(NAT(m) + 0xBDu + (uint32_t)x, 0);
            p8(NAT(m) + 0xBDu + (uint32_t)y, 1);
        }
}

static void m_trust(uint32_t rx, int32_t other, int32_t hon)
{
    int32_t lvl = (D16(0xE1624u) >> 10) & 3;
    uint32_t a = rx + 0xB1u + (uint32_t)other;
    p8(a, b8(a) + (int8_t)(b8(0xCA994u + (uint32_t)(4 * lvl)) * hon));
    if (b8(a) > 100)
        p8(a, 100);
    if (b8(a) < -15)
        p8(a, -15);
}

static void m_75074(int i)
{
    uint8_t pa[199], pb[199];
    uint32_t e = mresp(i), ra = NAT(b8(e + 13u)), rb = NAT(b8(e + 14u));
    int32_t r = b8(e + 18u), a = b8(ra + 0x3Du), bb = b8(rb + 0x3Du), t, k, j, idx = -1;
    memcpy(pa, DP(ra), 199);
    memcpy(pb, DP(rb), 199);
    t = b8(0x110F74u);
    for (k = 0; k < d32(0x112560u); k++)
        if (b8(moff(k) + 0x12u) == a) {
            idx = k;
            break;
        }
    for (j = 0; j < 5; j++)
        p8(0x110F75u + (uint32_t)(5 * t + j), idx >= 0 ? b8(moff(idx) + 13u + (uint32_t)j)
                                                       : g_original ? b8(moff(0) + 13u + (uint32_t)j) : 0);
    for (j = 0; j < 5; j++)
        p8(0x111015u + (uint32_t)(5 * t + j), b8(mrow(r) + 8u + (uint32_t)j));
    p8(0x1110B5u + (uint32_t)t, a);
    p8(0x1110D5u + (uint32_t)t, bb);
    p8(0x1110F5u + (uint32_t)t, b8(0xF5476u));
    p8(0x110F74u, b8(0x110F74u) + 1);
    m_75A1C(ra, r);
    if (b8(0x112572u) > 18)
        m_pass(ra, a, bb);
    if (b8(0x11257Bu) > 18)
        m_pass(rb, bb, a);
    f75C08(ra, rb);
    f6E460(a);
    f6E460(bb);
    f75DF0(b8(e + 13u));
    f75EB4(a, bb);
    f75F08(a, bb);
    if (b8(ra + 0x9Au) == 3)
        m_75520(0, a, bb, pa);
    if (!mpbem() && b8(rb + 0x9Au) == 3)
        m_75520(1, bb, a, pb);
    if (b8(ra + 0x9Au) == 2 && b8(rb + 0x9Au) == 2)
        m_75FB4(a);
    if (b8(ra + 0x9Au) == 2)
        m_trust(ra, bb, b8(0x112581u));
    if (b8(rb + 0x9Au) == 2)
        m_trust(rb, a, b8(0x112578u));
}

static void m_734E4(void)
{
    int32_t m, r, j, cnt, p, cur, code;
    uint32_t sv;
    if (d32(0x112560u) == 0)
        return;
    if (w16(0xF5476u) == 1)
        for (m = 1; m <= 9; m++)
            if (b8(NAT(m) + 0x9Au) == 2)
                memset(DP(NAT(m) + 0xBEu), 0, 9);
    f783B8(DLG_ACC, d32(0xF5958u));
    for (r = 0; r < w16(0x112548u); r++) {
        for (j = 0; j < 3; j++)
            p8(mrow(r) + 8u + (uint32_t)j, b8(mrow(r) + (uint32_t)j));
        cnt = 0;
        for (j = 0; j < 3; j++)
            if (D8(mrow(r) + 8u + (uint32_t)cnt) != 0)
                cnt++;
        if (!(D8(0xE1625u) & 0x20))
            f76F70(cnt, DP(mrow(r) + 8u));
    }
    p16(0x112542u, 1);
    p32(0x112558u, 0);
    p32(0x110EC8u, 0);
    p8(0xEE168u, 2);
    for (p = 0; p < d32(0xF40ECu); p++) {
        cur = w16(0xF06B0u + (uint32_t)(2 * p));
        p32(0xF40F8u, cur);
        if (b8(NAT(cur) + 0x9Cu) == 0)
            continue;
        p32(0x112564u, 0);
        if (b8(NAT(cur) + 0x9Au) != 3) {
            m_73D4C(cur);
            m_7401C(cur, 0);
            sv = D32(0xF8E78u);
            if (w16(0xF40C2u) == 0) {
                f3C940(sv);
                f78A20(2000);
            }
            trade_test_f6B0EC(cur);
            setD32(0xF8E78u, sv);
            if (w16(0xF40C2u) == 0)
                f3C7A0(sv);
            continue;
        }
        if (mpbem() && b8(0xE1622u) != cur)
            continue;
        m_73D4C(cur);
        if (b8(0xE1620u) != 1)
            m_7401C(cur, 0);
        f6FBDC(cur, 1);
        m_7401C(cur, 1);
        for (;;) {
            f3CD6C((int8_t)b8(0xEE168u));
            code = f78BA4(DLG_ACC, 0);
            if (code == -100)
                continue;
            if (code == -1)
                break;
            if (code == -0x2C || code == -0x2B) {
                p32(0x112564u, d32(0x112564u) + (code == -0x2C ? 3 : -3));
                m_7401C(cur, 1);
            } else if (code >= -0x3D && code <= -0x36) {
                p32(0x112564u, 0);
                cur = w16(0xF06B0u + (uint32_t)(2 * (-0x36 - code)));
                m_73D4C(cur);
                m_7401C(cur, 1);
            } else if (code >= -0x31 && code <= -0x2F && !mpbem()) {
                m_75074(-0x2F - code + d32(0x112564u));
                break;
            } else if (mpbem()) {
                if (code >= -0x31 && code <= -0x2F)
                    trade_test_pbem(0x7399Cu, -0x2F - code, 0);
                else if (code >= -0xAB && code <= -0x97)
                    trade_test_pbem(0x7399Cu, (-0x97 - code) % 3, (-0x97 - code) / 3 + 1);
            }
        }
    }
}

/* ---- the runs ----------------------------------------------------------------------------------------------------- */

static uint8_t *g_start, *g_after[2];
static int g_fail, g_checks, g_shown;

typedef void (*Fn)(void *);

static void run_side(int side, Fn fn, void *arg)
{
    g_cur = side;
    g_len[side] = 0;
    g_log[side][0] = 0;
    memcpy(g_ds, g_start, DS_SIZE);
    g_srng = g_srng0;
    g_steps = 0;
    g_toggle = 0;
    g_fixed_i = 0;
    if (setjmp(g_abort) == 0)
        fn(arg);
    memcpy(g_after[side], g_ds, DS_SIZE);
}

static const char *const COV[] = {
    "AI f68730", "AI f698A4", "AI f6B0EC", "PBEM", "ABORT", "yesno", "Your offer is incomplete", "must declare",
    "click 111594 0 -> -29", "click 111594 0 -> -28", "click 111D6C 0 -> -28", "click 111D6C 0 -> -29",
    "click 111980 0 -> -3", "click 112158 0 -> -47", "click 112158 0 -> -5", "Trade Details", "Trade Summary",
    "N O N E", "42 B91D3", "rect ", "spr F4CB4", "Respond To Offer", "shuffle ", "to your offer", "The responses to the",
    "value 0", "unattended -> 1", "dlg_off", "spr F4D24"
};
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
        if (g_shown++ < 12)
            printf("FAIL %s (original %d): trace line %d\n  port : %.150s\n  model: %.150s\n", what, g_original, line,
                   a, b);
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
}

static void p_chain(void *a) { (void)a; f6F270(); f723DC(); f734E4(); }
static void m_chain(void *a) { (void)a; m_6F270(); m_723DC(); m_734E4(); }
static void p_accept(void *a) { (void)a; f734E4(); }
static void m_accept(void *a) { (void)a; m_734E4(); }

typedef struct { int k, a, b, c, d; } Call;
static void p_call(void *v)
{
    Call *c = v;
    switch (c->k) {
    case 0: f70984(c->a); break;
    case 1: f716BC(c->a, c->b); break;
    case 2: f75A1C(NAT(c->a), c->b); break;
    case 3: f73D4C(c->a); break;
    case 4: f73D4C(c->a); if (S16(0x112568u) > 0) f75074(c->b % S16(0x112568u)); break;
    case 5: f721A0(c->a, c->b, c->c, c->d); break;
    case 6: tr("-> %d", f71768(c->a)); break;
    case 7: tr("-> %d", f70074(c->a, c->b)); break;
    case 8: f70294(c->a, c->b); break;
    case 9: f707A0(c->a); break;
    case 10: f70A6C(c->a, c->b); break;
    case 11: f72928(c->a, c->b); break;
    case 12: f72FC4(c->a, c->b); break;
    case 13: f73D4C(c->a); f7401C(c->a, c->b); break;
    case 14: f74D54(); break;
    case 15: f75000(); break;
    case 16: f75520(c->a, c->b, c->c, DP(NAT(c->d))); break;
    case 17: f75FB4(c->a); break;
    case 18: f6F454(c->a); break;
    case 19: f6F95C(c->a); break;
    case 20: f6FA54(); f6FB2C(); break;
    case 21: f6FDC8(c->a, c->b); break;
    case 22: f71B68(c->a ? "Your offer is incomplete.\n" : "x"); break;
    case 23: f7109C(c->a, c->b ? DLG_RESP : DLG_DECL); break;
    default: break;
    }
}
static void m_call(void *v)
{
    Call *c = v;
    switch (c->k) {
    case 0: m_70984(c->a); break;
    case 1: m_716BC(c->a, c->b); break;
    case 2: m_75A1C(NAT(c->a), c->b); break;
    case 3: m_73D4C(c->a); break;
    case 4: m_73D4C(c->a); if (S16(0x112568u) > 0) m_75074(c->b % S16(0x112568u)); break;
    case 5: m_721A0(c->a, c->b, c->c, c->d); break;
    case 6: tr("-> %d", m_71768(c->a)); break;
    case 7: tr("-> %d", m_70074(c->a, c->b)); break;
    case 8: m_70294(c->a, c->b); break;
    case 9: m_707A0(c->a); break;
    case 10: m_70A6C(c->a, c->b); break;
    case 11: m_72928(c->a, c->b); break;
    case 12: m_72FC4(c->a, c->b); break;
    case 13: m_73D4C(c->a); m_7401C(c->a, c->b); break;
    case 14: m_74D54(); break;
    case 15: m_75000(); break;
    case 16: m_75520(c->a, c->b, c->c, DP(NAT(c->d))); break;
    case 17: m_75FB4(c->a); break;
    case 18: m_6F454(c->a); break;
    case 19: m_6F95C(c->a); break;
    case 20: m_6FA54(); m_6FB2C(); break;
    case 21: m_6FDC8(c->a, c->b); break;
    case 22: m_71B68(c->a ? "Your offer is incomplete.\n" : "x"); break;
    case 23: m_7109C(c->a, c->b ? DLG_RESP : DLG_DECL); break;
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

static void dialogs(void)
{
    memcpy(DP(DLG_DECL), DP(0xC95E4u), 1002);
    setD32(0xF5948u, (uint32_t)S16(DLG_DECL));
    memcpy(DP(DLG_CNT), DP(0xC99D0u), 1002);
    setD32(0xF594Cu, (uint32_t)S16(DLG_CNT));
    setD32(0xF5950u, (uint32_t)S16(DLG_CNT));
    memcpy(DP(DLG_RESP), DP(0xC9DD0u), 1002);
    setD32(0xF5954u, (uint32_t)S16(DLG_RESP));
    memcpy(DP(DLG_ACC), DP(0xCA1BCu), 1002);
    setD32(0xF5958u, (uint32_t)S16(DLG_ACC));
    setD32(0xF595Cu, (uint32_t)S16(DLG_ACC));
}

static void random_hand(int n)
{
    uint32_t r = NAT(n);
    int c, k;
    for (c = 1; c <= 18; c++)
        setD8(r + (uint32_t)c, (uint8_t)(rnd(3) == 0 ? rnd(4) : 0));
    k = (int)rnd(3);
    setD8(r + 0x13u, (uint8_t)k);
    for (c = 0; c < 8; c++) {
        setD8(r + 0x14u + (uint32_t)c, (uint8_t)(c < k ? 0x13 + rnd(12) : 0));
        setD8(r + 0x1Cu + (uint32_t)c, (uint8_t)(c < k ? rnd(10) : 0xFF));
    }
    for (c = 0; c < 13; c++)
        setD8(r + 0x9Cu + (uint32_t)c, 0);
}

static void random_state(int keep_save)
{
    int n, players = 0, humans = 0, i;
    if (!keep_save)
        memcpy(g_ds, g_pristine, DS_SIZE);
    dialogs();
    for (n = 1; n <= 9; n++) {
        uint8_t kind = (uint8_t)(rnd(5) == 0 ? 0 : rnd(3) == 0 ? 2 : 3);
        if (keep_save && D8(NAT(n) + 0x9Au) != 0)
            kind = (uint8_t)(rnd(3) == 0 ? 2 : 3);
        if (!keep_save || rnd(2))
            random_hand(n);
        if (players >= 8)
            kind = 0;
        setD8(NAT(n) + 0x9Au, kind);
        setD8(NAT(n) + 0x3Du, (uint8_t)n);
        if (kind)
            players++;
        if (kind == 3)
            humans++;
        for (i = 0; i < 9; i++)
            setD8(NAT(n) + 0xB1u + (uint32_t)i, (uint8_t)(rnd(30) - 10));
        for (i = 0; i < 10; i++)
            setD8(NAT(n) + 0xBDu + (uint32_t)i, (uint8_t)(rnd(4) == 0));
    }
    setD32(0xF40ECu, (uint32_t)players);
    setD8(0xE1620u, (uint8_t)humans);
    setD8(0xE1623u, (uint8_t)((3 + rnd(5)) << 4 | (1 + rnd(4))));
    setD8(0xE1624u, (uint8_t)(rnd(12) == 0 ? 0x80 : 0) | (uint8_t)(D8(0xE1624u) & 0x7F));
    setD8(0xE1625u, (uint8_t)((rnd(4) << 2) | (rnd(3) == 0 ? 0x20 : 0)));
    setD8(0xE1622u, (uint8_t)(1 + rnd(9)));
    setD8(0xF097Cu, (uint8_t)(rnd(2) ? 0x80 : 0));
    setD16(0xF40C2u, (uint16_t)(rnd(6) == 0));
    setD16(0xF40BAu, (uint16_t)rnd(2));
    setD16(0xF5476u, (uint16_t)(1 + rnd(3)));
    setD8(0x110F74u, (uint8_t)rnd(4));
    setD32(SEED, rnd(0x7FFFFFFF));
    setD32(0xF52C8u, 1);
    setD32(0xF52CCu, 2);
    setD8(0xEE168u, 2);
    i = 0;
    setD16(0xF06AEu, 0);
    for (n = 1; n <= 9; n++)
        if (D8(NAT(n) + 0x9Au) != 0) {
            setD16(0xF06B0u + 2u * (uint32_t)i, (uint16_t)n);
            i++;
        }
}

/* a counter-offer table for 0x734E4 alone: offers, public records (f726DC) and rows */
static void random_table(void)
{
    int n, k, rows = 0;
    random_state(0);
    for (n = 1; n <= 9; n++) {
        uint32_t r = NAT(n);
        if (D8(r + 0x9Au) == 0 || rnd(4) == 0)
            continue;
        setD8(r + 0x9Cu, (uint8_t)(1 + rnd(18)));
        setD8(r + 0x9Du, (uint8_t)(1 + rnd(18)));
        setD8(r + 0x9Eu, (uint8_t)(1 + rnd(18)));
        for (k = 0; k < 5; k++)
            setD8(r + 0xA1u + (uint32_t)k, (uint8_t)(rnd(2) ? 1 + rnd(18) : 0));
        setD8(r + 0xA6u, (uint8_t)(rnd(4) ? 1 + rnd(18) : 0x13 + rnd(12)));
    }
    memset(DP(0x111118u), 0, 0x380);
    f726DC();
    for (k = 0; k < 20 && S32(0x112560u) > 0; k++) {
        int i = (int)rnd((uint32_t)S32(0x112560u)), m = 1 + (int)rnd(9);
        uint32_t row = 0x111118u + 16u * (uint32_t)rows;
        if (D8(NAT(m) + 0x9Au) == 0 || S8(0x110ECCu + 21u * (uint32_t)i + 0x12u) == m)
            continue;
        setD8(row + 0u, (uint8_t)(1 + rnd(18)));
        setD8(row + 1u, (uint8_t)(1 + rnd(18)));
        setD8(row + 2u, (uint8_t)(1 + rnd(18)));
        setD8(row + 5u, (uint8_t)(rnd(5) ? 1 + rnd(18) : 0x13 + rnd(12)));
        setD8(row + 0xDu, D8(0x110ECCu + 21u * (uint32_t)i + 0x12u));
        setD8(row + 0xEu, (uint8_t)m);
        setD8(row + 0xFu, D8(0x110ECCu + 21u * (uint32_t)i + 0x13u));
        rows++;
    }
    setD16(0x112548u, (uint16_t)rows);
}

static void expect(int ok, const char *what)
{
    g_checks++;
    if (!ok) {
        g_fail++;
        printf("FAIL A: %s (original %d)\n", what, g_original);
    }
}

/* ---- part A: hand vectors ------------------------------------------------------------------------------------------ */

static void part_a(void)
{
    uint32_t r;
    int v;
    static const int clicks_chart[] = { -0x1C, -0x7A, -1 };
    static const int cx[] = { 0x236, 0x236, 0x236 };
    static const int cy[] = { 447, 447, 447 };

    /* 0x75A1C: the offerer 1 bluffed (replacement 7 for bluff 9), the row honest (replacement = bluff 4) */
    memcpy(g_ds, g_pristine, DS_SIZE);
    r = NAT(1);
    memset(DP(r + 0x9Cu), 0, 13);
    setD8(r + 0x9Cu, 3); setD8(r + 0x9Du, 5); setD8(r + 0x9Eu, 9); setD8(r + 0xA6u, 7);
    memset(DP(0x111118u + 16u * 2u), 0, 16);
    setD8(0x111128u + 16u, 11); setD8(0x111129u + 16u, 12); setD8(0x11112Au + 16u, 4); setD8(0x11112Du + 16u, 4);
    f75A1C(r, 2);
    expect(D8(0x112570u) == 3 && D8(0x112571u) == 5 && D8(0x112572u) == 7 && D8(0x112575u) == 9, "75A1C side 0 cards");
    expect(S8(0x112578u) == -1, "75A1C side 0 bluffed");
    expect(D8(0x112579u) == 11 && D8(0x11257Au) == 12 && D8(0x11257Bu) == 4 && D8(0x11257Eu) == 4, "75A1C side 1 cards");
    expect(S8(0x112581u) == 1, "75A1C side 1 honest");

    /* 0x70984 / 0x716BC: record 0 with calamities 0x13 (giver 4), 0x15 (6), 0x17 (2) */
    for (v = 0; v < 2; v++) {
        g_original = v;
        memcpy(g_ds, g_pristine, DS_SIZE);
        r = NAT(0);
        setD8(r + 0x13u, 3);
        setD8(r + 0x14u, 0x13); setD8(r + 0x15u, 0x15); setD8(r + 0x16u, 0x17);
        setD8(r + 0x1Cu, 4); setD8(r + 0x1Du, 6); setD8(r + 0x1Eu, 2);
        setD8(r + 5u, 2);
        f70984(5);
        expect(D8(r + 5u) == 1, "70984 commodity");
        f70984(0x15);
        expect(D8(r + 0x13u) == 2 && D8(r + 0x14u) == 0x13 && D8(r + 0x15u) == 0x17 && D8(r + 0x16u) == 0 &&
               D8(r + 0x1Du) == 2 && D8(r + 0x1Eu) == 0xFF, "70984 calamity");
        f70984(0x1B);   /* not held */
        if (v)
            expect(D8(r + 0x13u) == 1 && D8(r + 0x14u) == 0x17 && D8(r + 0x15u) == 0 && D8(r + 0x1Cu) == 2,
                   "70984 not held, --original index 0");
        else
            expect(D8(r + 0x13u) == 2 && D8(r + 0x14u) == 0x13, "70984 not held, fixed");
        memcpy(g_ds, g_pristine, DS_SIZE);
        r = NAT(3);
        setD8(r + 0x13u, 2);
        setD8(r + 0x14u, 0x19); setD8(r + 0x15u, 0x1A);
        setD8(r + 0x1Cu, 5); setD8(r + 0x1Du, 7);
        f716BC(0x19, 3);
        expect(D8(r + 0x13u) == 1 && D8(r + 0x14u) == 0x1A && D8(r + 0x15u) == 0x1A && D8(r + 0x1Cu) == 7 &&
               D8(r + 0x1Du) == 7, "716BC leaves the freed slot");
    }

    /* 0x71768 */
    for (v = 0; v < 2; v++) {
        g_original = v;
        memcpy(g_ds, g_pristine, DS_SIZE);
        g_cur = 0;
        g_len[0] = 0;
        g_log[0][0] = 0;
        g_fixed = (const int[]){ -1, -1, -1 };
        g_fixed_x = g_fixed_y = NULL;
        g_fixed_n = 3;
        g_fixed_i = 0;
        r = NAT(2);
        memset(DP(r + 0x9Cu), 0, 13);
        setD32(0x110EC8u, 0);
        setD32(0x1105ACu + 8u, 0x55);
        expect(f71768(2) == -1 && S32(0x1105ACu + 8u) == 0, "71768 empty offer accepted, want 0");
        setD8(r + 0x9Cu, 1);
        expect(f71768(2) == -15 && strstr(g_log[0], "Your offer is incomplete.") != NULL, "71768 incomplete");
        g_len[0] = 0;
        g_log[0][0] = 0;
        setD8(r + 0x9Du, 2); setD8(r + 0x9Eu, 3); setD8(r + 0xA1u, 4); setD8(r + 0xA2u, 5);
        expect(f71768(2) == -15 && strstr(g_log[0], "'You must declare a bluff replacement.'") != NULL,
               "71768 no replacement: only the second text");
        setD8(r + 0xA6u, 6);
        expect(f71768(2) == -1 && S32(0x1105ACu + 8u) == 2, "71768 complete, want 2");
        /* mode 1: row 0 answering record 1 (nation 5) */
        setD32(0x110EC8u, 1);
        setD16(0x112548u, 0);
        memset(DP(0x111118u), 0, 16);
        setD32(0x11256Cu, 1);
        setD8(0x110ECCu + 21u + 0x12u, 5);
        setD8(0x110ECCu + 21u + 0x13u, 0x33);
        setD8(0x111118u, 7); setD8(0x111119u, 8); setD8(0x11111Au, 9); setD8(0x11111Du, 10);
        expect(f71768(2) == -1, "71768 counter accepted");
        expect(D8(0x111125u) == 5 && D8(0x111126u) == 2 && D8(0x111127u) == 0x33 && D8(0x110ECCu + 21u + 0x14u) == 2,
               "71768 counter marks");
        expect(S32(0x1105ACu + 8u) == (v ? 0 : 2), "71768 counter: 0x1105AC (0 with --original, kept fixed)");
        g_fixed = NULL;
    }

    /* the chart click at y 447: card 19 placed with --original, ignored fixed */
    for (v = 0; v < 2; v++) {
        g_original = v;
        memcpy(g_ds, g_pristine, DS_SIZE);
        dialogs();
        g_cur = 0;
        g_len[0] = 0;
        g_log[0][0] = 0;
        r = NAT(2);
        random_hand(2);
        setD8(r + 1u, 3);
        setD8(r + 2u, 3);
        setD32(0x110EC8u, 0);
        setD32(0x112558u, 2);   /* the want group */
        g_fixed = clicks_chart;
        g_fixed_x = cx;
        g_fixed_y = cy;
        g_fixed_n = 3;
        g_fixed_i = 0;
        if (setjmp(g_abort) == 0) {
            f7109C(2, DLG_DECL);
        }
        expect((strstr(g_log[0], "spr F4CB4") != NULL) == v, "chart y 447: card 19 only with --original");
        g_fixed = NULL;
        g_fixed_x = g_fixed_y = NULL;
    }

    /* the article */
    for (v = 0; v < 2; v++) {
        g_original = v;
        memcpy(g_ds, g_pristine, DS_SIZE);
        dialogs();
        g_cur = 0;
        g_len[0] = 0;
        g_log[0][0] = 0;
        setD8(NAT(1) + 0x9Au, 2);
        setD16(0x112568u, 0);
        f7401C(1, 0);
        expect(strstr(g_log[0], v ? "'The responses to African offer...'" : "'The responses to the African offer...'")
                   != NULL, "the article");
    }
    g_original = 0;

    /* 0x721A0: 2 of card 1 and 1 of card 3 in 500 px: steps 40 / 10, cards at 7, 17, 57 */
    memcpy(g_ds, g_pristine, DS_SIZE);
    g_cur = 0;
    g_len[0] = 0;
    g_log[0][0] = 0;
    r = NAT(4);
    memset(DP(r + 1u), 0, 0x13);
    setD8(r + 1u, 2);
    setD8(r + 3u, 1);
    f721A0(7, 406, 500, 4);
    expect(S32(0x11254Cu) == 40 && S32(0x112550u) == 10 &&
           strstr(g_log[0], "spr F4C24 7 406\nrect 7 406") != NULL && strstr(g_log[0], "spr F4C24 17 406") != NULL &&
           strstr(g_log[0], "spr F4C34 57 406") != NULL, "721A0 layout");
}

/* ---- the parts ------------------------------------------------------------------------------------------------- */

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
    const char *playdir = argc > 2 ? argv[2] : "C:\\work\\advciv_play";
    char path[400];
    const char *saves[4];
    int i, k, mode, s, fa, fb, fc, fd, fe;
    Call c;

    setvbuf(stdout, NULL, _IONBF, 0);
    if (state_init(gamedir) != 0)
        return 2;
    g_log[0] = malloc(LOGSZ);
    g_log[1] = malloc(LOGSZ);
    g_start = malloc(DS_SIZE);
    g_after[0] = malloc(DS_SIZE);
    g_after[1] = malloc(DS_SIZE);
    g_pristine = malloc(DS_SIZE);
    if (!g_log[0] || !g_log[1] || !g_start || !g_after[0] || !g_after[1] || !g_pristine)
        return 2;
    memcpy(g_pristine, g_ds, DS_SIZE);

    part_a();
    fa = g_fail;
    printf("A hand vectors: %d checks, %d failures\n", g_checks, fa);

    for (mode = 0; mode < 2; mode++) {
        g_original = mode;
        for (i = 0; i < 1000; i++) {
            random_state(0);
            g_srng0 = 0x9E3779B9u ^ (uint32_t)(i * 7919 + mode);
            g_budget = 20 + (int)rnd(120);
            compare("B chain", p_chain, m_chain, NULL);
        }
    }
    fb = g_fail - fa;
    printf("B chain: %d failures\n", fb);

    for (mode = 0; mode < 2; mode++) {
        g_original = mode;
        for (i = 0; i < 750; i++) {
            random_table();
            g_srng0 = 0x85EBCA6Bu ^ (uint32_t)(i * 104729 + mode);
            g_budget = 10 + (int)rnd(60);
            compare("C accept", p_accept, m_accept, NULL);
        }
    }
    fc = g_fail - fa - fb;
    printf("C accept: %d failures\n", fc);

    for (mode = 0; mode < 2; mode++) {
        g_original = mode;
        for (i = 0; i < 3000; i++) {
            char what[40];
            random_table();
            setD32(0x110EC8u, rnd(3) == 0);
            setD32(0x112558u, (uint32_t)((int)rnd(5) - (rnd(8) == 0)));
            setD32(0x11255Cu, rnd(3) * 5);
            setD32(0x112564u, rnd(2) * 3);
            setD32(0x11256Cu, S32(0x112560u) > 0 ? rnd((uint32_t)S32(0x112560u)) : 0);
            setD16(0x112548u, (uint16_t)(S16(0x112548u) > 0 ? rnd((uint32_t)S16(0x112548u)) : 0));
            setD32(0xF40F8u, 1 + rnd(9));
            c.k = (int)rnd(24);
            c.a = 1 + (int)rnd(9);
            c.b = (int)rnd(3);
            c.c = 1 + (int)rnd(9);
            c.d = (int)rnd(10);
            switch (c.k) {
            case 0: c.a = (int)rnd(2) ? 1 + (int)rnd(18) : 0x13 + (int)rnd(12); break;
            case 1: c.a = 0x13 + (int)rnd(12); c.b = (int)rnd(10); break;
            case 2: c.b = (int)rnd(8); break;
            case 4: c.b = (int)rnd(8); break;
            case 5: c.a = 7; c.b = 406; c.c = (int)rnd(2) ? 500 : 100 + (int)rnd(300); c.d = (int)rnd(10); break;
            case 7: c.a = (int)rnd(2) ? 1 + (int)rnd(18) : 0x13 + (int)rnd(12); c.b = 1 + (int)rnd(9); break;
            case 10: c.a = -0x11 - (int)rnd(5); c.b = 1 + (int)rnd(9); break;
            case 11: c.b = (int)rnd(2); break;
            case 12:
                setD32(0x11255Cu, 0);
                if (S32(0x112560u) == 0)
                    c.k = 14;
                else
                    c.a = -0x26 - (int)rnd((uint32_t)(S32(0x112560u) < 5 ? S32(0x112560u) : 5));
                break;
            case 13: c.b = (int)rnd(2); break;
            case 16: c.a = (int)rnd(2); c.b = 1 + (int)rnd(9); c.c = 1 + (int)rnd(9); c.d = (int)rnd(10); break;
            case 17:
                setD8(0x110F74u, (uint8_t)(1 + rnd(5)));
                setD8(0x1110B5u + D8(0x110F74u) - 1u, (uint8_t)(1 + rnd(9)));
                setD8(0x1110D5u + D8(0x110F74u) - 1u, (uint8_t)(1 + rnd(9)));
                break;
            case 19: c.a = (int)rnd(2); break;
            case 21: c.a = (int)rnd(10); c.b = (int)rnd(2); break;
            case 22: c.a = (int)rnd(2); break;
            default: break;
            }
            snprintf(what, sizeof what, "D call %d", c.k);
            g_srng0 = 0xC2B2AE35u ^ (uint32_t)(i * 31 + mode);
            g_budget = 10 + (int)rnd(40);
            compare(what, p_call, m_call, &c);
        }
    }
    fd = g_fail - fa - fb - fc;
    printf("D direct calls: %d failures\n", fd);

    snprintf(path, sizeof path, "%s\\SAVGAMEA.DAT", playdir);
    saves[0] = strdup(path);
    snprintf(path, sizeof path, "%s\\savgameb.dat", playdir);
    saves[1] = strdup(path);
    snprintf(path, sizeof path, "%s\\savgame0.dat", gamedir);
    saves[2] = strdup(path);
    snprintf(path, sizeof path, "%s\\savgame1.dat", gamedir);
    saves[3] = strdup(path);
    k = 0;
    for (s = 0; s < 4; s++) {
        memcpy(g_ds, g_pristine, DS_SIZE);
        if (!load_save(saves[s])) {
            printf("E: cannot read %s\n", saves[s]);
            g_fail++;
            continue;
        }
        memcpy(g_start, g_ds, DS_SIZE);
        for (mode = 0; mode < 2; mode++) {
            g_original = mode;
            for (i = 0; i < 30; i++) {
                memcpy(g_ds, g_start, DS_SIZE);
                random_state(i % 3 != 0);
                g_srng0 = 0x27D4EB2Fu ^ (uint32_t)(s * 1000 + i * 13 + mode);
                g_budget = 30 + (int)rnd(100);
                compare("E saves", p_chain, m_chain, NULL);
                memcpy(g_ds, g_start, DS_SIZE);
                k++;
            }
        }
    }
    fe = g_fail - fa - fb - fc - fd;
    printf("E real saves: %d chains, %d failures\n", k, fe);

    printf("coverage (runs of the port that reached each path):\n");
    for (i = 0; i < (int)NCOV; i++)
        printf("  %-28s %d\n", COV[i], g_cov[i]);
    printf("%d checks, %d failures\n", g_checks, g_fail);
    return g_fail != 0;
}
