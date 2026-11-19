/* tax_test [GAMEDIR [SAVEDIR]]: unit checks for module TAX (tax.c) that need no game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init; default C:\work\advciv_run, only read); SAVEDIR holds the
   real saves savgameb.dat ("end of Round 3", the start of TAXATION R4) and SAVGAMEA.DAT (default C:\work\advciv_play,
   only read); savgame0.dat comes from GAMEDIR. All three are read with saveload.c's f5F05C as the game does.
   tax.c is built with -DTAX_TEST:
   its five calls into code that is not ported (0x667B0, 0x66B54, 0x66C2C, 0x2468C, 0x28CFC) come to recorders here
   that act out a plausible effect (the computer's rate by the formula of 0x667B0's record; cities moved or dropped,
   last city first) and return; the choice list of n's cities follows 0x28CFC's kind-2 code (157 entries -1 first,
   then n's city territories in ascending order).

   Every routine tax.c calls is a stub below that appends one line to a trace (drawing included), and the few with
   data effects apply them (f3F670 / f3F624 the border colours, f78F04 the city removal of rules.h plus a marker bit in
   the area record so that RESTART's copy of the areas is seen, f79A9C the clamp). Clicks (f78BA4) and YES/NO answers
   (f795A8) come from a script: a seeded random choice among the codes the screen offers (only enabled buttons, map
   clicks on listed territories, -100, codes that do nothing), the same seed for the port and the model.

   The model is a second implementation of the four routines, written from the disassembly (not from tax.c): it works
   on a copy of the data object and writes the trace it expects. After each run the two traces must be equal and the
   whole data object must equal the model's copy. Parts:
     A. hand vectors (expected values written out): nobody with cities; tax paid with byte wrap; the Democracy text
        fixed and with --original; the forced rate; the tie order of the absorbers; 0x66D8C's empty list with and
        without the fix (byte 0xFFB5F, the "PIRACY" box).
     B. 0x66050 on 4,000 random states (players, cities, stock, rate, Coinage, Democracy, PBEM), each with and without
        --original; C. 0x6685C on 3,000 random revolts; D. 0x66488 on 3,000 random rate screens; E. 0x66D8C on 4,000
        random removal screens (selection, arrows, REMOVE, RESTART with and without the confirmation, FINISHED);
     F. 0x66050 on the three real saves, 300 click scripts each.
   Prints each failure (the first differing trace line or byte), the counts per part, and exits 0 when all pass.

   Build: clang -std=c11 -DTAX_TEST port/test/tax_test.c port/src/tax.c port/src/saveload.c port/src/state.c
          port/src/le_image.c -Iport/src -o tax_test.exe */

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fg.h"
#include "game.h"
#include "kit.h"
#include "map.h"
#include "mapdata.h"
#include "panel.h"
#include "rules.h"
#include "saveload.h"
#include "start.h"
#include "state.h"
#include "tax.h"
#include "ui.h"

/* ---- addresses (from the listing, written out here) ---------------------------------------------------------- */

#define NAT(n)      (0xF14CCu + 199u * (uint32_t)(n))
#define AREA_BASE   0xE31B8u
#define TS_BASE     0xE2C18u
#define LIST        0xF7CA0u
#define LIST_N      0xF7C9Cu
#define BORDERS     0xFFB60u

/* ---- the buffer the shared helpers act on: g_ds (the port's run) or the model's copy --------------------------- */

static uint8_t *B;
static uint8_t *M;   /* the model's copy */

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

/* ---- traces --------------------------------------------------------------------------------------------------- */

#define LOGSZ (1u << 20)
static char *g_real, *g_model;
static size_t g_real_len, g_model_len;

static void vapp(char *log, size_t *len, const char *fmt, va_list ap)
{
    int k;
    if (*len + 600 >= LOGSZ)
        return;
    k = vsnprintf(log + *len, LOGSZ - *len, fmt, ap);
    if (k > 0)
        *len += (size_t)k;
    log[(*len)++] = '\n';
    log[*len] = 0;
}

static void tr(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vapp(g_real, &g_real_len, fmt, ap);
    va_end(ap);
}

static void mt(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vapp(g_model, &g_model_len, fmt, ap);
    va_end(ap);
}

/* ---- the click script ----------------------------------------------------------------------------------------- */

static uint32_t g_srng;
static int g_steps;
static int g_rem_en, g_fin_en;   /* the port's REMOVE / FINISHED state, from f3C314 / f3C3D0 */

static uint32_t srnd(uint32_t n)
{
    g_srng ^= g_srng << 13;
    g_srng ^= g_srng >> 17;
    g_srng ^= g_srng << 5;
    return n ? g_srng % n : 0;
}

/* the rate screen: only the panel dialog's codes plus the arrows; ends with -1 after 30 clicks */
static int pick_rate(void)
{
    static const int codes[] = { -100, -12, -12, -11, -11, -1, -5, 3, -13, -10 };
    if (++g_steps > 30)
        return -1;
    return codes[srnd(sizeof codes / sizeof codes[0])];
}

/* the removal screen: enabled buttons only, map clicks on listed territories; after 40 clicks REMOVE until done */
static int pick_remove(int rem_en, int fin_en, int count)
{
    uint32_t c;
    if (++g_steps > 40)
        return rem_en ? -0x65 : -1;
    for (;;) {
        c = srnd(10);
        switch (c) {
        case 0: return -100;
        case 1: if (count > 0) return (int)srnd((uint32_t)count); break;
        case 2: return -9;
        case 3: return -10;
        case 4: if (srnd(4) == 0) return -0x63; break;
        case 5: case 6: if (rem_en) return -0x65; break;
        case 7: if (fin_en && srnd(3) == 0) return -1; break;
        case 8: return -0x40;   /* nothing */
        default: return -0x70;  /* nothing (unsigned below -0x63, not -0x65) */
        }
    }
}

static int pick_yesno(void) { return srnd(2) ? -0x61 : -0x62; }

/* ---- shared effects of the stubs (on B) -------------------------------------------------------------------------- */

static void city_remove(int t, int n)   /* rules.h's f78F04, plus a marker bit in area t + 1 */
{
    uint32_t rec = NAT(n);
    int c = s8(rec + 0x51), j, k;
    w8(TS_BASE + 3u * (uint32_t)t + 2, b8(TS_BASE + 3u * (uint32_t)t + 2) & ~1);
    w8(TS_BASE + 3u * (uint32_t)t, 0);
    w8(TS_BASE + 3u * (uint32_t)t + 1, 0xFF);
    for (j = 0; j < c; j++)
        if (b8(rec + 0x52 + (uint32_t)j) == t + 1)
            break;
    if (j < c) {
        for (k = j; k < c - 1; k++)
            w8(rec + 0x52 + (uint32_t)k, b8(rec + 0x53 + (uint32_t)k));
        w8(rec + 0x52 + (uint32_t)(c - 1), 0);
        w8(rec + 0x51, c - 1);
    }
    w8(AREA_BASE + 69u * (uint32_t)(t + 1) + 0x3B + (uint32_t)n, b8(AREA_BASE + 69u * (uint32_t)(t + 1) + 0x3B + (uint32_t)n) ^ 0x40);
}

static void city_add(int t, int m)
{
    uint32_t rec = NAT(m);
    int c = s8(rec + 0x51);
    w8(TS_BASE + 3u * (uint32_t)t + 2, b8(TS_BASE + 3u * (uint32_t)t + 2) | 1);
    w8(TS_BASE + 3u * (uint32_t)t, m);
    w8(TS_BASE + 3u * (uint32_t)t + 1, b8(0xF40FCu));
    w8(rec + 0x52 + (uint32_t)c, t + 1);
    w8(rec + 0x51, c + 1);
}

static void city_move(int m, int n, int k)   /* m takes k of n's cities, the last first */
{
    int i, c, t;
    for (i = 0; i < k; i++) {
        c = s8(NAT(n) + 0x51);
        if (c <= 0 || s8(NAT(m) + 0x51) >= 9)
            break;
        t = b8(NAT(n) + 0x52 + (uint32_t)(c - 1)) - 1;
        city_remove(t, n);
        city_add(t, m);
    }
}

static void city_drop(int n, int k)
{
    int i, c;
    for (i = 0; i < k; i++) {
        c = s8(NAT(n) + 0x51);
        if (c <= 0)
            break;
        city_remove(b8(NAT(n) + 0x52 + (uint32_t)(c - 1)) - 1, n);
    }
}

/* 0x28CFC kind 2 (D, read in 0x28D18..0x28D42 and 0x28FD8..0x29040): count 0, entries 0..156 = -1, then each
   territory t 0..156 (ascending) whose state byte +2 has bit 0x01 (a city) and whose byte +0 (unsigned) equals n */
static void build_list(int n)
{
    int t, cnt = 0;
    w32(LIST_N, 0);
    for (t = 0; t < 157; t++)
        w32(LIST + 4u * (uint32_t)t, -1);
    for (t = 0; t < 157; t++)
        if ((b8(TS_BASE + 3u * (uint32_t)t + 2) & 1) != 0 && b8(TS_BASE + 3u * (uint32_t)t) == n)
            w32(LIST + 4u * (uint32_t)cnt++, t);
    w32(LIST_N, cnt);
}

static int ai_rate(int n)   /* 0x667B0 per its record */
{
    int c = s8(NAT(n) + 0x51), s = s8(NAT(n) + 0x99), r;
    if (2 * c > s)
        r = 1;
    else if (s16(0xF40FCu) >= 9 && s >= 25 && c <= 6)
        r = 3;
    else
        r = 2;
    while (r > 1 && c * r > s)
        r--;
    return r;
}

/* ---- stubs (the port's run: B == g_ds) ---------------------------------------------------------------------------- */

static jmp_buf g_jb;
static int g_np;

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
void f3D9FC(int n) { tr("home %d", n); }
void f3A990(void) { tr("strip"); }
void f3A924(void) { tr("area"); }
void f3A9F4(const char *text) { tr("left %s", text); }
void f3AB7C(const char *text) { tr("box %s", text); }
void f78A20(int t) { tr("wait %d", t); }
void f7844C(uint32_t dlg, int base) { tr("7844C %X %d %d", (unsigned)dlg, base, (int)S32(0xF40F8u)); }
void f79270(int wait, int n, int t, const char *text, int anim) { tr("msg %d %d %d %s %d", wait, n, t, text, anim); }
void f3F670(void) { tr("clrb"); memset(DP(BORDERS), 0, 0xA0); }
void f3F624(int t, int c) { tr("border %d %d", t, c); setD8(BORDERS + (uint32_t)t, (uint8_t)c); }
void f7853C(uint32_t dlg) { tr("append %X", (unsigned)dlg); }
void f78664(uint32_t dlg) { tr("remove %X", (unsigned)dlg); }
void f3AF8C(uint32_t spr, int x, int y) { tr("sprite %X %d %d", (unsigned)spr, x, y); }
void f3A3A0(int x, int y, const char *s) { tr("right %d %d %s", x, y, s); }
void f3A044(int x, int y, const char *s) { tr("print %d %d %s", x, y, s); }
void f3A0B0(int x, int y, int w, const char *s) { tr("text %d %d %d %s", x, y, w, s); }
void f3C9D0(int x, int y, int w, int h, int flags) { tr("frame %d %d %d %d %d", x, y, w, h, flags); }
void f3C3D0(uint32_t dlg, int id)
{
    tr("off %X %X", (unsigned)dlg, (unsigned)id);
    if (id == 0x3F) g_rem_en = 0;
    if (id == 0x2A) g_fin_en = 0;
}
void f3C314(uint32_t dlg, int id)
{
    tr("on %X %X", (unsigned)dlg, (unsigned)id);
    if (id == 0x3F) g_rem_en = 1;
    if (id == 0x2A) g_fin_en = 1;
}
void f3DA60(int t) { tr("centre %d", t); }
void f3D52C(int dx, int dy) { tr("scroll %d %d", dx, dy); }
void f78F04(int t, int n) { tr("78F04 %d %d", t, n); city_remove(t, n); }
int f795A8(const char *text) { int r = pick_yesno(); tr("yesno %s %d", text, r); return r; }
void f79A9C(int32_t *p)
{
    if (S32(LIST_N) <= *p)
        *p = S32(LIST_N) - 1;
    if (*p < 0)
        *p = 0;
}
int f78BA4(uint32_t dlg, int map)
{
    int r = map == 0 ? pick_rate() : pick_remove(g_rem_en, g_fin_en, S32(LIST_N));
    tr("click %X %d %d", (unsigned)dlg, map, r);
    return r;
}

int tax_test_f667B0(int n) { tr("667B0 %d", n); return ai_rate(n); }
void tax_test_f66B54(int m, int n, int k) { tr("66B54 %d %d %d", m, n, k); city_move(m, n, k); }
void tax_test_f66C2C(int n, int k) { tr("66C2C %d %d", n, k); city_drop(n, k); }
void tax_test_f2468C(int c, int v, int k, int mode) { tr("2468C %d %d %d %d", c, v, k, mode); city_move(c, v, k); }
void tax_test_f28CFC(int n, int kind, int x) { tr("28CFC %d %d %d", n, kind, x); build_list(n); }

/* ---- the model (B == M) ------------------------------------------------------------------------------------------- */

static int pbem(void) { return (b8(0xE1624u) & 0x80) != 0; }
static int human_here(int n) { return (s8(NAT(n) + 0x9A) == 3 && !pbem()) || (pbem() && s8(0xE1622u) == n); }
static void m_clrb(void) { mt("clrb"); memset(M + (BORDERS - DS_BASE), 0, 0xA0); }
static void m_border(int t, int c) { mt("border %d %d", t, c); w8(BORDERS + (uint32_t)t, c); }
static void m_mapscreen(void)
{
    mt("bg 3");
    w8(0xEE168u, 3);
    mt("vbcopy 0 639 0 479 0 479 %d %d", s32(0xF52CCu), s32(0xF52C8u));
}

static void m66D8C(int n, int k);

static void m66488(int n)
{
    uint32_t rec = NAT(n);
    int max, v, r;

    w32(0xFFD40u, -50);
    w32(0xF40F8u, n);
    mt("7844C F596C %d %d", s32(0xF5940u), n);
    if (pbem() && s8(0xE1622u) != n)
        return;
    m_mapscreen();
    mt("phase");
    max = s8(rec + 0x99) / s8(rec + 0x51);
    if (max > 3)
        max = 3;
    if (max == 0) {
        mt("area");
        mt("box %s, a tax revolt is unavoidable.\nThe tax rate has been set to 1 to minimize the effect.", name_of(n));
        m_clrb();
        mt("panel %d", n);
        mt("home %d", n);
        mt("dialog F596C");
        mt("7844C F596C %d %d", s32(0xF5940u), n);
        mt("paste");
        w8(rec + 0x3F, 1);
        mt("wait 4000");
        return;
    }
    mt("append C817C");
    mt("vbopen %d", s32(0xF52C8u));
    mt("sprite F51AC 296 385");
    mt("sprite F51B4 330 385");
    mt("color 5");
    mt("right 293 389 TAX");
    mt("print 346 389 RATE");
    mt("vbclose");
    mt("left %s, set your tax rate.", name_of(n));
    mt("dialog F596C");
    v = s8(rec + 0x3F) < max ? s8(rec + 0x3F) : max;
    m_clrb();
    mt("panel %d", n);
    mt("home %d", n);
    for (;;) {
        mt("vbopen %d", s32(0xF52C8u));
        mt("frame 310 385 19 19 5");
        mt("color 5");
        mt("text 310 389 19 %d", v);
        mt("vbclose");
        mt("paste");
        r = pick_rate();
        mt("click F596C 0 %d", r);
        if (r == -1) {
            w8(rec + 0x3F, v);
            break;
        }
        if (r == -12 && v < max)
            v++;
        else if (r == -11 && v > 1)
            v--;
    }
    mt("remove C817C");
}

static void m6685C(int n, int k)
{
    int nat[9], sc[9], cnt = 0, m, i, j, take, x;

    w32(0xFFD40u, 2);
    for (m = 1; m <= 9; m++) {
        if (b8(NAT(m) + 0x9A) == 0 || m == n || s32(0x11017Cu + 4u * (uint32_t)m) != 0)
            continue;
        /* stable insertion, descending by the 16-bit score */
        int s = (int16_t)(uint16_t)(5 * (9 - s8(NAT(m) + 0x51)) + s8(NAT(m) + 0x99));
        for (i = cnt; i > 0 && sc[i - 1] < s; i--) {
            nat[i] = nat[i - 1];
            sc[i] = sc[i - 1];
        }
        nat[i] = m;
        sc[i] = s;
        cnt++;
    }
    mt("msg -500 %d -200 %s suffers a Tax Revolt! 0", n, name_of(n));
    mt("strip");
    mt("paste");
    for (j = 0; j < cnt && k > 0; j++) {
        m = nat[j];
        if (s8(NAT(m) + 0x51) >= 9)
            continue;
        take = 9 - s8(NAT(m) + 0x51);
        if (k < take)
            take = k;
        if (s8(NAT(m) + 0x9A) == 3 && !pbem()) {
            mt("2468C %d %d %d 2", m, n, take);
            city_move(m, n, take);
        } else {
            mt("66B54 %d %d %d", m, n, take);
            city_move(m, n, take);
        }
        k -= take;
    }
    if (k > 0) {
        mt("msg -500 %d -200 %s's revolting cities can no longer be absorbed by the other nations. 0", n, name_of(n));
        if (s8(NAT(n) + 0x9A) == 3 && !pbem())
            m66D8C(n, k);
        else {
            mt("66C2C %d %d", n, k);
            city_drop(n, k);
        }
    }
    x = s8(NAT(n) + 0x51) * s8(NAT(n) + 0x3F);
    w8(NAT(n) + 0x3E, b8(NAT(n) + 0x3E) + x);
    w8(NAT(n) + 0x99, b8(NAT(n) + 0x99) - x);
}

static uint8_t g_sv_area[161 * 69], g_sv_ts[160 * 3], g_sv_nat[12 * 199];

static void m66D8C(int n, int k)
{
    int left = k, sel = 0, redraw = 1, r, cnt, i;
    int32_t ent;

    memcpy(g_sv_area, M + (AREA_BASE - DS_BASE), sizeof g_sv_area);
    memcpy(g_sv_ts, M + (TS_BASE - DS_BASE), sizeof g_sv_ts);
    memcpy(g_sv_nat + 199, M + (NAT(1) - DS_BASE), 11 * 199);
    m_mapscreen();
    mt("vbopen %d", s32(0xF52C8u));
    mt("sprite F51AC 251 405");
    mt("sprite F51B4 376 405");
    mt("color 5");
    mt("right 303 389 LEFT TO");
    mt("print 336 389 REMOVE");
    mt("vbclose");
    mt("box %s, select cities to be removed", name_of(n));
    mt("phase");
    mt("append C8568");
    mt("dialog F596C");
    mt("28CFC %d 2 0", n);
    build_list(n);
    for (;;) {
        cnt = s32(LIST_N);
        ent = s32(LIST + 4u * (uint32_t)sel);
        mt("vbopen %d", s32(0xF52C8u));
        mt("frame 265 405 110 19 5");
        mt("color 5");
        if (g_original || cnt > 0)
            mt("text 265 409 110 %s", str((uint32_t)s32(0xC77D8u + 4u * (uint32_t)ent)));
        mt("frame 310 385 19 19 5");
        mt("color 5");
        mt("text 310 389 19 %d", left);
        mt("vbclose");
        m_clrb();
        if (g_original || cnt > 0)
            m_border(ent, 15);
        if (left > 0) {
            mt("off F596C 2A");
            mt("on F596C 3F");
        } else {
            mt("on F596C 2A");
            mt("off F596C 3F");
        }
        if (redraw) {
            mt("panel %d", n);
            if (cnt > 0)
                mt("centre %d", ent);
            else
                mt("scroll 0 0");
            redraw = 0;
        }
        mt("paste");
        r = pick_remove(left > 0, left <= 0, cnt);
        mt("click F596C 1 %d", r);
        if (r == -1)
            break;
        if (r == -100)
            continue;
        if (r >= 0) {
            sel = r;
            redraw = 1;
            continue;
        }
        switch (r) {
        case -0x65:
            mt("78F04 %d %d", ent, n);
            city_remove(ent, n);
            left--;
            /* the list shifted, the count lowered and sel clamped, then rebuilt */
            for (i = sel; i < cnt - 1; i++)
                w32(LIST + 4u * (uint32_t)i, s32(LIST + 4u * (uint32_t)(i + 1)));
            w32(LIST_N, cnt - 1);
            if (sel >= cnt - 1)
                sel = cnt - 2;
            if (sel < 0)
                sel = 0;
            mt("28CFC %d 2 0", n);
            build_list(n);
            redraw = 1;
            break;
        case -0x63: {
            char q[200];
            int yes = 1;
            snprintf(q, sizeof q, "Restart %s Tax Revolt?", str((uint32_t)s32(0xC76C4u + 4u * (uint32_t)n)));
            if (b8(0xF097Cu) & 8) {
                int a = pick_yesno();
                mt("yesno %s %d", q, a);
                yes = a == -0x61;
            }
            if (yes) {
                memcpy(M + (AREA_BASE - DS_BASE), g_sv_area, sizeof g_sv_area);
                memcpy(M + (NAT(1) - DS_BASE), g_sv_nat + 199, 11 * 199);
                memcpy(M + (TS_BASE - DS_BASE), g_sv_ts, sizeof g_sv_ts);
                left = k;
                mt("28CFC %d 2 0", n);
                build_list(n);
                sel = 0;
                redraw = 1;
            }
            break;
        }
        case -10:
            if (sel < cnt - 1) {
                sel++;
                redraw = 1;
            }
            break;
        case -9:
            if (sel > 0) {
                sel--;
                redraw = 1;
            }
            break;
        default:
            break;
        }
    }
    mt("remove C8568");
    w32(0xEED74u, 0);
    w32(0xEED7Cu, 0);
    w32(0xEED78u, 0);
}

static void m66050(void)
{
    int n, any = 0, first, has = 0, waited = 0, tax, old, r, c;
    int flag[10] = { 0 };

    w16(0xBB3FAu, 0xA9);
    w16(0xBB3FCu, 0x154);
    w16(0xBB3FEu, 0x12C);
    w16(0xBB400u, 0x23);
    w16(0xF06AEu, 0);
    for (n = 1; n <= 9; n++) {
        w32(0x11017Cu + 4u * (uint32_t)n, 0);
        if (b8(NAT(n) + 0x9A) != 0) {
            int p = s16(0xF06AEu);
            w16(0xF06B0u + 2u * (uint32_t)p, n);
            w16(0xF06AEu, p + 1);
        }
    }
    mt("783B8 F596C %d", s32(0xF5940u));
    first = s16(0xF06B0u);
    for (n = 1; n <= 9; n++)
        if (s8(NAT(n) + 0x51) > 0) {
            has = 1;
            break;
        }
    if (!has) {
        mt("msg 100 %d -200  0", first);
        return;
    }
    m_mapscreen();
    mt("phase");
    mt("dialog F596C");
    mt("panel %d", first);
    mt("paste");
    mt("home %d", first);
    for (n = 1; n <= 9; n++) {
        uint32_t rec = NAT(n);
        w32(0xF40F8u, n);
        if (b8(rec + 0x9A) == 0 || b8(rec + 0x51) == 0)
            continue;
        c = s8(rec + 0x51);
        if (b8(rec + 0x2C) != 0) {
            if (human_here(n)) {
                m66488(n);
                waited = 1;
            } else {
                old = s8(rec + 0x3F);
                mt("667B0 %d", n);
                r = ai_rate(n);
                w8(rec + 0x3F, r);
                if (r != old && !pbem()) {
                    mt("strip");
                    mt("left %s sets its Tax Rate to %d.", name_of(n), r);
                    mt("panel %d", n);
                    mt("paste");
                    mt("home %d", n);
                    mt("wait 2000");
                    waited = 1;
                }
            }
        }
        tax = c * s8(rec + 0x3F);
        if (s8(rec + 0x99) >= tax) {
            w8(rec + 0x3E, b8(rec + 0x3E) + tax);
            w8(rec + 0x99, b8(rec + 0x99) - tax);
        } else {
            w32(0x11017Cu + 4u * (uint32_t)n, 1);
            flag[n] = 1;
            any = 1;
        }
    }
    if (any)
        for (n = 1; n <= 9; n++) {
            if (!flag[n])
                continue;
            if (b8(NAT(n) + 0x34) != 0) {
                mt("area");
                mt("left Democra%s spares %s from a Tax Revolt.", g_original ? "ry" : "cy", name_of(n));
                mt("panel %d", n);
                mt("paste");
                mt("home %d", n);
                mt("wait 3000");
            } else {
                int kept = s8(NAT(n) + 0x99) / s8(NAT(n) + 0x3F);
                if (!g_original)
                    w32(0xF40F8u, n);
                mt("7844C F596C %d %d", s32(0xF5940u), s32(0xF40F8u));
                m6685C(n, s8(NAT(n) + 0x51) - kept);
            }
            waited = 1;
        }
    if (!waited)
        mt("wait 500");
}

/* ---- running and comparing -------------------------------------------------------------------------------------- */

static int g_checks, g_fail, g_shown;

static void reset_logs(void) { g_real_len = g_model_len = 0; g_real[0] = g_model[0] = 0; }

enum { R_PHASE, R_RATE, R_REVOLT, R_REMOVE };

/* run one routine in the port and in the model from the same state (g_ds as set up), compare */
static int run_both(int which, int n, int k, uint32_t seed, const char *what)
{
    int ok = 1;
    reset_logs();
    memcpy(M, g_ds, DS_SIZE);
    /* the port */
    B = g_ds;
    g_srng = seed; g_steps = 0; g_rem_en = g_fin_en = -1;
    if (setjmp(g_jb) == 0) {
        switch (which) {
        case R_PHASE: f66050(); break;
        case R_RATE: f66488(n); break;
        case R_REVOLT: f6685C(n, k); break;
        default: f66D8C(n, k); break;
        }
    }
    /* the model */
    B = M;
    g_srng = seed; g_steps = 0;
    switch (which) {
    case R_PHASE: m66050(); break;
    case R_RATE: m66488(n); break;
    case R_REVOLT: m6685C(n, k); break;
    default: m66D8C(n, k); break;
    }
    B = g_ds;
    g_checks++;
    if (strcmp(g_real, g_model) != 0) {
        ok = 0;
        if (g_shown++ < 8) {
            const char *a = g_real, *b = g_model;
            int line = 1;
            while (*a && *a == *b) { if (*a == '\n') line++; a++; b++; }
            while (a > g_real && a[-1] != '\n') { a--; b--; }
            printf("FAIL %s: trace line %d\n  port : %.120s\n  model: %.120s\n", what, line, a, b);
        }
    } else if (memcmp(g_ds, M, DS_SIZE) != 0) {
        uint32_t i;
        ok = 0;
        for (i = 0; i < DS_SIZE && g_ds[i] == M[i]; i++)
            ;
        if (g_shown++ < 8)
            printf("FAIL %s: data 0x%X port %02X model %02X\n", what, (unsigned)(i + DS_BASE), g_ds[i], M[i]);
    }
    if (!ok)
        g_fail++;
    return ok;
}

/* ---- random states ------------------------------------------------------------------------------------------------ */

static uint32_t g_rng = 0x2468ACE1u;
static uint32_t rnd(uint32_t n)
{
    g_rng ^= g_rng << 13;
    g_rng ^= g_rng >> 17;
    g_rng ^= g_rng << 5;
    return n ? g_rng % n : 0;
}

static uint8_t *g_pristine;

/* nations 1..9 with random players and distinct city territories; nations 10, 11 untouched */
static void random_state(int coinage_pct, int pbem_pct)
{
    int used[160] = { 0 }, n, j, t, c;
    uint32_t rec;

    memcpy(g_ds, g_pristine, DS_SIZE);
    B = g_ds;
    for (t = 0; t < 160; t++) {
        w8(TS_BASE + 3u * (uint32_t)t, 0);
        w8(TS_BASE + 3u * (uint32_t)t + 1, 0xFF);
        w8(TS_BASE + 3u * (uint32_t)t + 2, (int)rnd(256) & ~1);
    }
    for (j = 0; j < 161 * 69; j++)
        w8(AREA_BASE + (uint32_t)j, (int)rnd(256));
    for (n = 1; n <= 9; n++) {
        rec = NAT(n);
        w8(rec + 0x9A, rnd(5) == 0 ? 0 : (rnd(2) ? 3 : 2));
        c = rnd(6) == 0 ? 0 : (int)rnd(10);
        w8(rec + 0x51, 0);
        for (j = 0; j < 9; j++)
            w8(rec + 0x52 + (uint32_t)j, 0);
        for (j = 0; j < c; j++) {
            do t = (int)rnd(157); while (used[t]);   /* 0x28CFC lists territories 0..156 */
            used[t] = 1;
            city_add(t, n);
        }
        w8(rec + 0x99, (int)rnd(rnd(3) == 0 ? 12 : 56));
        w8(rec + 0x3E, (int)rnd(256));
        w8(rec + 0x3F, rnd(10) == 0 ? 0 : 1 + (int)rnd(3));
        w8(rec + 0x2C, (int)rnd(100) < coinage_pct ? 1 : 0);
        w8(rec + 0x34, rnd(4) == 0 ? 1 : 0);
    }
    w16(0xF40FCu, 1 + (int)rnd(20));
    w8(0xE1624u, (b8(0xE1624u) & 0x7F) | ((int)rnd(100) < pbem_pct ? 0x80 : 0));
    w8(0xE1622u, 1 + (int)rnd(9));
    w8(0xF097Cu, (b8(0xF097Cu) & ~8) | (rnd(2) ? 8 : 0));
    w32(0xF40F8u, (int32_t)rnd(12));
    w32(0xFFD40u, (int32_t)rnd(100));
    w32(LIST_N, (int32_t)rnd(5));
    for (j = 0; j < 0xA0; j++)
        w8(BORDERS + (uint32_t)j, (int)rnd(3));
    w8(BORDERS - 1, 0x80);   /* the top byte of [0xFFB5C], as a handle has it */
    for (n = 1; n <= 9; n++)
        w32(0x11017Cu + 4u * (uint32_t)n, (int32_t)rnd(2));
}

/* ---- A. hand vectors ------------------------------------------------------------------------------------------------ */

static void expect(int cond, const char *what)
{
    g_checks++;
    if (!cond) {
        g_fail++;
        printf("FAIL A: %s\n", what);
    }
}

static void clear_nations(void)
{
    int n;
    memcpy(g_ds, g_pristine, DS_SIZE);
    B = g_ds;
    for (n = 1; n <= 9; n++) {
        memset(DP(NAT(n) + 0x2C), 0, 1);
        memset(DP(NAT(n) + 0x34), 0, 1);
        memset(DP(NAT(n) + 0x51), 0, 10);
        w8(NAT(n) + 0x9A, 0);
    }
    w8(0xE1624u, b8(0xE1624u) & 0x7F);
}

static void part_a(void)
{
    int f0 = g_fail;

    /* A1 nobody with cities: the 100 ms home view of the first player, nothing else */
    clear_nations();
    w8(NAT(3) + 0x9A, 3);
    w8(NAT(7) + 0x9A, 2);
    run_both(R_PHASE, 0, 0, 1, "A1");
    expect(strstr(g_real, "msg 100 3 -200  0\n") != NULL && strstr(g_real, "bg ") == NULL, "A1 nobody: one message");
    expect(S16(0xF06AEu) == 2 && S16(0xF06B0u) == 3 && S16(0xF06B2u) == 7, "A1 player list 3, 7");

    /* A2 tax paid, bytes wrap: 3 cities, rate 2, stock 10, treasury 250 -> treasury 0, stock 4 */
    clear_nations();
    w8(NAT(2) + 0x9A, 2);
    city_add(10, 2); city_add(11, 2); city_add(12, 2);
    w8(NAT(2) + 0x3F, 2);
    w8(NAT(2) + 0x99, 10);
    w8(NAT(2) + 0x3E, 250);
    run_both(R_PHASE, 0, 0, 1, "A2");
    expect(D8(NAT(2) + 0x3E) == 0 && D8(NAT(2) + 0x99) == 4 && S32(0x11017Cu + 8) == 0, "A2 treasury 0, stock 4");
    expect(strstr(g_real, "wait 500\n") != NULL, "A2 the 500 ms wait");

    /* A3 revolt with Democracy: the text, fixed and original; the stock is not touched */
    for (g_original = 0; g_original <= 1; g_original++) {
        clear_nations();
        w8(NAT(9) + 0x9A, 3);
        city_add(20, 9); city_add(21, 9);
        w8(NAT(9) + 0x3F, 3);
        w8(NAT(9) + 0x99, 5);
        w8(NAT(9) + 0x34, 1);
        run_both(R_PHASE, 0, 0, 1, "A3");
        expect(strstr(g_real, g_original ? "left Democrary spares EGYPT from a Tax Revolt.\n" :
                                           "left Democracy spares EGYPT from a Tax Revolt.\n") != NULL ||
               strstr(g_real, g_original ? "left Democrary spares Egypt from a Tax Revolt.\n" :
                                           "left Democracy spares Egypt from a Tax Revolt.\n") != NULL,
               g_original ? "A3 'Democrary' with --original" : "A3 'Democracy' fixed");
        expect(D8(NAT(9) + 0x99) == 5 && S32(0x11017Cu + 36) == 1, "A3 flag set, stock kept");
    }
    g_original = 0;

    /* A4 the forced rate: stock 1 < cities 2 -> max 0, rate 1, 4000 ms, no dialog */
    clear_nations();
    w8(NAT(4) + 0x9A, 3);
    city_add(30, 4); city_add(31, 4);
    w8(NAT(4) + 0x3F, 3);
    w8(NAT(4) + 0x99, 1);
    run_both(R_RATE, 4, 0, 1, "A4");
    expect(D8(NAT(4) + 0x3F) == 1 && strstr(g_real, "wait 4000\n") != NULL && strstr(g_real, "append C817C") == NULL &&
           S32(0xFFD40u) == -50, "A4 forced rate 1");

    /* A5 the absorbers' order: scores 5*(9-c)+s; ties keep the nation order (bubble sort, strict >) */
    clear_nations();
    w8(NAT(1) + 0x9A, 2);   /* revolting nation */
    for (int i = 0; i < 6; i++) city_add(40 + i, 1);
    w8(NAT(3) + 0x9A, 2); city_add(50, 3); w8(NAT(3) + 0x99, 5);                    /* 5*8+5 = 45 */
    w8(NAT(5) + 0x9A, 2); w8(NAT(5) + 0x99, 0); city_add(51, 5); city_add(52, 5);    /* 5*7+0 = 35 */
    w8(NAT(6) + 0x9A, 2); city_add(53, 6); w8(NAT(6) + 0x99, 5);                    /* 45, after 3 */
    w8(NAT(8) + 0x9A, 2); w8(NAT(8) + 0x99, 50);                                    /* 45 + 50 = 95 */
    w32(0x11017Cu + 4, 1);
    w32(0x11017Cu + 12, 0); w32(0x11017Cu + 20, 0); w32(0x11017Cu + 24, 0); w32(0x11017Cu + 32, 0);
    run_both(R_REVOLT, 1, 20, 1, "A5");
    {
        const char *p8 = strstr(g_real, "66B54 8 1 9"), *p3 = strstr(g_real, "66B54 3 1 "),
                   *p6 = strstr(g_real, "66B54 6 1 ");
        expect(p8 && p3 && p6 && p8 < p3 && p3 < p6 && strstr(g_real, "66B54 5 1") == NULL, "A5 order 8, 3, 6");
        expect(S32(0xFFD40u) == 2, "A5 help topic 2");
    }

    /* A6 0x66D8C with an empty list after the last REMOVE: the fix and --original */
    for (g_original = 0; g_original <= 1; g_original++) {
        clear_nations();
        w8(NAT(2) + 0x9A, 3);
        city_add(60, 2);
        w8(0xFFB5Fu, 0x80);
        w8(0xF097Cu, b8(0xF097Cu) & ~8);
        /* seed chosen so the script removes the city first: any script ends with REMOVE then FINISHED */
        run_both(R_REMOVE, 2, 1, 7, "A6");
        expect(D8(0xFFB5Fu) == (g_original ? 15 : 0x80), g_original ? "A6 --original writes 0xFFB5F" :
                                                                       "A6 fixed: 0xFFB5F kept");
        expect((strstr(g_real, "text 265 409 110 PIRACY\n") != NULL) == (g_original != 0), "A6 the PIRACY box");
        expect(S8(NAT(2) + 0x51) == 0 && D32(0xEED74u) == 0 && D32(0xEED78u) == 0 && D32(0xEED7Cu) == 0,
               "A6 city removed, snapshots freed");
    }
    g_original = 0;
    printf("A: hand vectors, %d failed\n", g_fail - f0);
}

/* ---- B..F --------------------------------------------------------------------------------------------------------- */

static void part_b(void)
{
    int i, f0 = g_fail, np0 = g_np;
    char what[64];
    for (i = 0; i < 4000; i++) {
        uint32_t keep = g_rng;
        for (g_original = 0; g_original <= 1; g_original++) {
            g_rng = keep;
            random_state(40, 5);
            snprintf(what, sizeof what, "B%d%s", i, g_original ? " original" : "");
            run_both(R_PHASE, 0, 0, 1000u + (uint32_t)i, what);
        }
    }
    g_original = 0;
    printf("B: 0x66050, 8000 runs, %d failed (not_ported reached %d times)\n", g_fail - f0, g_np - np0);
}

/* a nation with cities (and in play when in_play): random among them, 0 if there is none (the caller redraws) */
static int pick_nation(int in_play)
{
    int cand[9], cnt = 0, n;
    for (n = 1; n <= 9; n++)
        if (S8(NAT(n) + 0x51) != 0 && (!in_play || D8(NAT(n) + 0x9A) != 0))
            cand[cnt++] = n;
    return cnt == 0 ? 0 : cand[rnd((uint32_t)cnt)];
}

static void part_c(void)
{
    int i, n, c, f0 = g_fail;
    char what[64];
    for (i = 0; i < 3000; i++) {
        do random_state(40, 5); while ((n = pick_nation(1)) == 0);
        c = S8(NAT(n) + 0x51);
        g_original = (int)rnd(2);
        snprintf(what, sizeof what, "C%d", i);
        run_both(R_REVOLT, n, 1 + (int)rnd((uint32_t)c), 5000u + (uint32_t)i, what);
    }
    g_original = 0;
    printf("C: 0x6685C, 3000 runs, %d failed\n", g_fail - f0);
}

static void part_d(void)
{
    int i, n, f0 = g_fail;
    char what[64];
    for (i = 0; i < 3000; i++) {
        do random_state(100, 10); while ((n = pick_nation(0)) == 0);
        snprintf(what, sizeof what, "D%d", i);
        run_both(R_RATE, n, 0, 9000u + (uint32_t)i, what);
    }
    printf("D: 0x66488, 3000 runs, %d failed\n", g_fail - f0);
}

static void part_e(void)
{
    int i, n, c, f0 = g_fail;
    char what[64];
    for (i = 0; i < 4000; i++) {
        do random_state(0, 0); while ((n = pick_nation(0)) == 0);
        c = S8(NAT(n) + 0x51);
        g_original = (int)rnd(2);
        snprintf(what, sizeof what, "E%d", i);
        run_both(R_REMOVE, n, rnd(3) == 0 ? c : 1 + (int)rnd((uint32_t)c), 20000u + (uint32_t)i, what);
    }
    g_original = 0;
    printf("E: 0x66D8C, 4000 runs, %d failed\n", g_fail - f0);
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

static void part_f(const char *savedir, const char *gamedir)
{
    static const char *const SAVES[3] = { "savgameb.dat", "SAVGAMEA.DAT", "savgame0.dat" };
    uint8_t *base = malloc(DS_SIZE);
    char path[1024], what[64];
    int s, i, n, f0 = g_fail, np0 = g_np, coin;

    for (s = 0; s < 3 && base != NULL; s++) {
        snprintf(path, sizeof path, "%s/%s", s < 2 ? savedir : gamedir, SAVES[s]);
        if (!load_save(path)) {
            printf("FAIL F: cannot read %s\n", path);
            g_fail++;
            continue;
        }
        memcpy(base, g_ds, DS_SIZE);
        B = g_ds;
        coin = 0;
        for (n = 1; n <= 9; n++)
            coin += D8(NAT(n) + 0x2C) != 0;
        printf("F: %s round %d, cities", SAVES[s], S16(0xF40FCu));
        for (n = 1; n <= 9; n++)
            printf(" %d", S8(NAT(n) + 0x51));
        printf(", Coinage %d\n", coin);
        for (i = 0; i < 300; i++) {
            memcpy(g_ds, base, DS_SIZE);
            if (i > 0)   /* give the humans Coinage now and then, so the rate screen runs on real records */
                for (n = 1; n <= 9; n++)
                    if (rnd(3) == 0)
                        setD8(NAT(n) + 0x2C, 1);
            g_original = i & 1;
            snprintf(what, sizeof what, "F %s %d", SAVES[s], i);
            run_both(R_PHASE, 0, 0, 40000u + (uint32_t)i, what);
        }
    }
    g_original = 0;
    free(base);
    printf("F: 0x66050 on the real saves, %d failed (not_ported reached %d times)\n", g_fail - f0, g_np - np0);
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

    part_a();
    part_b();
    part_c();
    part_d();
    part_e();
    part_f(savedir, gamedir);

    printf("%d checks, %d failed\n", g_checks, g_fail);
    return g_fail == 0 ? 0 : 1;
}
