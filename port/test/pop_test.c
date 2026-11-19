/* pop_test [GAMEDIR]: unit vectors for module POP (pop.c) that need no game run.

   It loads the data object from GAMEDIR/AC_MAIN.EXE as the port does (state_init: le_image.c; GAMEDIR defaults to the
   run copy C:\work\advciv_run, only read), clears the area records (0xE31B8, 161 x 69) and the nation records
   (0xF14CC, 14 x 199), writes synthetic contents and runs the phase 0x673A8 (and through it ManualPopExpansion
   0x67758) with every UI routine stubbed. The stubs record what they receive (a call trace); f78BA4 hands out clicks.

   pop.c is compiled into this file with not_ported renamed to a hook (game.h is included first, so its prototype keeps
   the real name): the hook stands in for the two routines that are not ported yet, so the statements after their call
   sites run as they will once the modules land:
     0x28CFC (CAL1) with kind 3: the choice list of n's territories with an allowance, built here from the listing
             (0x28D18..0x28D42, 0x28F6E..0x28FC8: count 0, 157 entries -1, then t 0..156 with area +0x24+n != 0);
     0x680DC (AI): only recorded.

   Expectations are written here from the disassembly's semantics, independently of pop.c:
     A. hand vectors: every byte the phase should change is listed with its value, worked out by hand (automatic
        expansion with the marker, the no-stock message, the computer's hook; a manual session with ADD MAX, the
        next arrow and ADD ONE, the entry removal and list rebuild; the DISTRIBUTE ALL guard of BUG(1995) with tokens
        in territory 158, which 0x28CFC never lists), and the whole data object must equal the snapshot taken before
        plus exactly those bytes;
     B. a random differential run: a second implementation of 0x673A8 / 0x67758 (written from the listing, not from
        pop.c) replays on a copy of the data object the clicks and YES/NO answers the stubs handed to pop.c; the
        whole data object and the call trace must agree after every phase.
   Prints each failure, the counts per part, and exits 0 when all pass.

   Link: pop_test.c state.c le_image.c (pop.c is included). */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/state.h"
#include "../src/game.h"

static void hook_not_ported(uint32_t addr);
#define not_ported hook_not_ported
#include "../src/pop.c"
#undef not_ported

/* ---- addresses (written out here from the listing) ------------------------------------------------------------ */

#define AREA_BASE 0xE31B8u
#define AREA_LEN  (161u * 69u)
#define NAT_BASE  0xF14CCu
#define NAT_LEN   (14u * 199u)
#define TS_BASE   0xE2C18u
#define AR(t, off) (AREA_BASE + 69u * (uint32_t)((t) + 1) + (uint32_t)(off))   /* field off of territory t's record */
#define NA(n, off) (NAT_BASE + 199u * (uint32_t)(n) + (uint32_t)(off))         /* field off of nation n's record */

/* ---- trace, clicks ---------------------------------------------------------------------------------------------- */

static char g_trace[1 << 20];
static size_t g_tlen;
static char g_mtrace[1 << 20];
static size_t g_mlen;

static void tr(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (g_tlen < sizeof g_trace)
        g_tlen += (size_t)vsnprintf(g_trace + g_tlen, sizeof g_trace - g_tlen, fmt, ap);
    va_end(ap);
}

static void mtr(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (g_mlen < sizeof g_mtrace)
        g_mlen += (size_t)vsnprintf(g_mtrace + g_mlen, sizeof g_mtrace - g_mlen, fmt, ap);
    va_end(ap);
}

static uint32_t g_rng = 0x12345678u;
static uint32_t rnd(uint32_t n)
{
    g_rng ^= g_rng << 13;
    g_rng ^= g_rng >> 17;
    g_rng ^= g_rng << 5;
    return n ? g_rng % n : 0;
}

/* scripted clicks (part A) or random ones (part B); both recorded for the model */
static int g_script[64], g_nscript, g_iscript;
static int g_random;                  /* 1: f78BA4 chooses at random among the enabled buttons */
static int g_budget;                  /* random clicks left in this session */
static int g_forced;                  /* 0: DISTRIBUTE ALL not yet forced at the end */
static int g_clicks[1 << 16], g_nclicks;
static int g_yes[1 << 12], g_nyes;
static int g_yes_script = -0x61;      /* part A's answer */
static int g_en[0x100];               /* enabled state of the panel buttons as the stubs saw it */
static long g_sessions, g_allclicks, g_allyes, g_ai, g_msgs;   /* part B coverage */

/* ---- stubs -------------------------------------------------------------------------------------------------------- */

void f783B8(uint32_t dlg, int base) { (void)dlg; (void)base; }
void f3CD6C(int8_t pic) { (void)pic; }
int fg_vbcopy(int a, int b, int c, int d, int e, int f, int g, int h)
{
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h;
    return 0;
}
void f7876C(void) { }
void f3B05C(uint32_t dlg) { (void)dlg; }
void f459E4(int n) { tr("P%d ", n); }
void f3CBBC(void) { }
void f3D9FC(int n) { tr("V%d ", n); }
void f7844C(uint32_t dlg, int base) { (void)dlg; (void)base; tr("B "); }
void f79270(int wait, int n, int t, const char *text, int anim)
{
    tr("M%d:%d:%d:%s:%d ", wait, n, t, text, anim);
    g_msgs++;
}
void f78A20(int t) { tr("W%d ", t); }
int fg_vbopen(int h) { (void)h; return 0; }
void fg_vbclose(void) { }
void f3AF8C(uint32_t spr, int x, int y) { (void)spr; (void)x; (void)y; }
int fg_setcolor(int c) { (void)c; return 0; }
void f3A3A0(int x, int y, const char *s) { (void)x; (void)y; (void)s; }
void f3A044(int x, int y, const char *s) { (void)x; (void)y; (void)s; }
void f3A9F4(const char *text) { tr("Q%s ", text); }
void f7853C(uint32_t dlg)
{
    tr("D%X ", (unsigned)dlg);
    g_sessions++;
    g_budget = (int)rnd(40);          /* a new session of 0x67758 (its only call of f7853C) */
    g_forced = 0;
}
void f3C9D0(int x, int y, int w, int h, int flags) { (void)x; (void)y; (void)w; (void)h; (void)flags; }
void f3A0B0(int x, int y, int w, const char *s) { (void)x; (void)y; (void)w; tr("T%s ", s); }
void f3F670(void) { }
void f3F624(int t, int c) { tr("H%d,%d ", t, c); }
void f3C3D0(uint32_t dlg, int id) { (void)dlg; g_en[id & 0xFF] = 0; tr("-%X ", id); }
void f3C314(uint32_t dlg, int id) { (void)dlg; g_en[id & 0xFF] = 1; tr("+%X ", id); }
void f3DA60(int t) { tr("C%d ", t); }
void f3D52C(int dx, int dy) { tr("Z%d,%d ", dx, dy); }
void f78664(uint32_t dlg) { tr("X%X ", (unsigned)dlg); }

/* game.h's f79A9C: clamp *p to the list [0xF7C9C] */
void f79A9C(int32_t *p)
{
    int32_t c = S32(0xF7C9Cu);
    if (c <= *p)
        *p = c - 1;
    if (*p < 0)
        *p = 0;
}

int f795A8(const char *text)
{
    int a = g_random ? (rnd(2) ? -0x61 : -0x62) : g_yes_script;
    tr("Y%s ", text);
    if (g_nyes < (int)(sizeof g_yes / sizeof g_yes[0]))
        g_yes[g_nyes++] = a;
    return a;
}

int f78BA4(uint32_t dlg, int map)
{
    int r, k;
    (void)dlg; (void)map;
    if (!g_random) {
        r = g_iscript < g_nscript ? g_script[g_iscript++] : -1;
    } else if (g_budget > 0) {
        g_budget--;
        k = (int)rnd(14);
        switch (k) {
        case 0: case 1: r = S32(0xF7C9Cu) > 0 ? (int)rnd((uint32_t)S32(0xF7C9Cu)) : -100; break;
        case 2: r = -9; break;
        case 3: r = -0xA; break;
        case 4: case 5: r = g_en[0xDC] ? -0x77 : -100; break;
        case 6: r = g_en[0xDD] ? -0x78 : -100; break;
        case 7: r = g_en[0xDB] && rnd(4) == 0 ? -0x76 : -100; break;
        case 8: r = rnd(3) == 0 ? -0x63 : -100; break;
        case 9: r = -100; break;
        case 10: r = (int)rnd(2) ? -5 : -0x79; break;      /* codes the screen ignores */
        case 11: r = (int)rnd(2) ? -0x75 : -8; break;
        default: r = g_en[0x2A] ? -1 : -100; break;
        }
    } else if (!g_forced && g_en[0xDB]) {
        g_forced = 1;
        r = -0x76;
    } else {
        r = -1;
    }
    tr("K%d ", r);
    if (g_nclicks < (int)(sizeof g_clicks / sizeof g_clicks[0]))
        g_clicks[g_nclicks++] = r;
    return r;
}

/* the heap blocks of 0x822B5 / 0x877E6, handles as load.c's (tag | slot << 20 | offset) */
static uint8_t *g_blk[16];
static uint32_t g_blksz[16];
uint32_t load_heap_alloc(uint32_t size)
{
    uint32_t i;
    for (i = 0; i < 16; i++)
        if (g_blk[i] == NULL) {
            g_blk[i] = calloc(1, size);
            g_blksz[i] = size;
            return 0x80000000u | (i + 1u) << 20;
        }
    return 0;
}
uint8_t *load_heap_ptr(uint32_t h, uint32_t n)
{
    uint32_t slot = (h & 0x7FFFFFFFu) >> 20, off = h & 0xFFFFFu;
    if ((h & 0x80000000u) == 0 || slot < 1 || slot > 16 || g_blk[slot - 1] == NULL || off + n > g_blksz[slot - 1])
        return NULL;
    return g_blk[slot - 1] + off;
}
void load_heap_free(uint32_t h)
{
    uint32_t slot = (h & 0x7FFFFFFFu) >> 20;
    if ((h & 0x80000000u) == 0 || slot < 1 || slot > 16)
        return;
    free(g_blk[slot - 1]);
    g_blk[slot - 1] = NULL;
}

/* 0x28CFC kind 3 on a data object image ds (the stand-in for CAL1, shared by the hook and the model) */
static void list3(uint8_t *ds, int32_t n)
{
    int32_t t, c = 0;
    for (t = 0; t < 0x9D; t++)
        memcpy(ds + (0xF7CA0u + 4u * (uint32_t)t - DS_BASE), "\xFF\xFF\xFF\xFF", 4);
    for (t = 0; t < 0x9D; t++)
        if (ds[AR(t, 0x24 + n) - DS_BASE] != 0) {
            uint32_t v = (uint32_t)t;
            uint8_t *p = ds + (0xF7CA0u + 4u * (uint32_t)c - DS_BASE);
            p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24);
            c++;
        }
    ds[0xF7C9Cu - DS_BASE] = (uint8_t)c;
    ds[0xF7C9Du - DS_BASE] = (uint8_t)(c >> 8);
    ds[0xF7C9Eu - DS_BASE] = 0;
    ds[0xF7C9Fu - DS_BASE] = 0;
}

static void hook_not_ported(uint32_t addr)
{
    if (addr == 0x28CFCu) {
        list3(g_ds, S32(0xF40F8u));      /* the caller's n is [ebp-0x18]; f673A8 set [0xF40F8] = n before the call */
        tr("L ");
    } else if (addr == 0x680DCu) {
        tr("A%d ", S32(0xF40F8u));
        g_ai++;
    } else {
        printf("unexpected not_ported(0x%X)\n", (unsigned)addr);
        exit(2);
    }
}

/* ---- the model: 0x673A8 and 0x67758 on a copy of the data object, from the listing ----------------------------- */

static uint8_t *M;
#define MB(a)  (M[(a) - DS_BASE])
static int8_t   ms8(uint32_t a)  { return (int8_t)MB(a); }
static int16_t  ms16(uint32_t a) { return (int16_t)(MB(a) | MB(a + 1) << 8); }
static int32_t  ms32(uint32_t a) { return (int32_t)((uint32_t)MB(a) | (uint32_t)MB(a + 1) << 8 | (uint32_t)MB(a + 2) << 16 | (uint32_t)MB(a + 3) << 24); }
static void mw16(uint32_t a, int v) { MB(a) = (uint8_t)v; MB(a + 1) = (uint8_t)(v >> 8); }
static void mw32(uint32_t a, int32_t v) { mw16(a, v); mw16(a + 2, v >> 16); }
static const char *mstr(uint32_t ptr_at) { return (const char *)&MB((uint32_t)ms32(ptr_at)); }

static int g_mclick, g_myes;

static void m_allow(int n)
{
    int a;
    for (a = 1; a <= 160; a++) {
        int8_t tok = ms8(AREA_BASE + 69u * (uint32_t)a + 0x0Eu + (uint32_t)n);
        MB(AREA_BASE + 69u * (uint32_t)a + 0x24u + (uint32_t)n) = (uint8_t)(tok == 0 ? 0 : tok == 1 ? 1 : 2);
    }
}

static void m_add(int t, int n)
{
    MB(AR(t, 0x0E + n))++;
    MB(AR(t, 0x24 + n))--;
    MB(NA(n, 0x99))--;
    MB(NA(n, 0x98))++;
}

static void m_manual(int n)
{
    static uint8_t sa[161 * 69], sn[12 * 199], st[160 * 3];
    int sel = 0, redraw = 1, remove = 0, r, i, t, cnt, added;
    char buf[64];

    mw16(0xBB400u, 0x15);
    memcpy(sa, &MB(AREA_BASE), sizeof sa);
    memcpy(sn + 199, &MB(NA(1, 0)), 11 * 199);
    memcpy(st, &MB(TS_BASE), sizeof st);
    m_allow(n);
    mtr("Q%s, you have insufficient stock for full expansion. Choose territories to be expanded. ", mstr(0xC768Cu + 4u * (uint32_t)n));
    mtr("D%X ", 0xC8954u);
    mtr("B ");
    list3(M, n);
    mtr("L ");
    for (;;) {
        t = ms32(0xF7CA0u + 4u * (uint32_t)sel);
        mtr("T%s ", mstr(0xC77D8u + 4u * (uint32_t)t));
        mtr("T%d ", ms8(NA(n, 0x99)));
        mtr("H%d,15 ", t);
        if (ms8(NA(n, 0x99)) > 0)
            mtr("-2A +DC +DD +DB ");
        else
            mtr("+2A -DC -DD -DB ");
        if (redraw) {
            mtr("P%d ", n);
            if (ms32(0xF7C9Cu) > 0)
                mtr("C%d ", t);
            else
                mtr("Z0,0 ");
            redraw = 0;
        }
        r = g_mclick < g_nclicks ? g_clicks[g_mclick++] : -1;   /* a broken pop.c may stop early */
        mtr("K%d ", r);
        if (r == -100)
            continue;
        if (r >= 0) { sel = r; redraw = 1; continue; }
        if (r == -1)
            break;
        switch (r) {
        case -9: if (sel > 0) { sel--; redraw = 1; } break;
        case -0xA: if (sel < ms32(0xF7C9Cu) - 1) { sel++; redraw = 1; } break;
        case -0x77:
            t = ms32(0xF7CA0u + 4u * (uint32_t)sel);
            m_add(t, n);
            if (MB(AR(t, 0x24 + n)) == 0)
                remove = 1;
            redraw = 1;
            break;
        case -0x78:
            t = ms32(0xF7CA0u + 4u * (uint32_t)sel);
            while (MB(NA(n, 0x99)) != 0 && MB(AR(t, 0x24 + n)) != 0)
                m_add(t, n);
            remove = 1;
            redraw = 1;
            break;
        case -0x76:
            while (MB(NA(n, 0x99)) != 0) {
                added = 0;
                for (i = 0; i < ms32(0xF7C9Cu); i++) {
                    t = ms32(0xF7CA0u + 4u * (uint32_t)i);
                    if (MB(NA(n, 0x99)) != 0 && MB(AR(t, 0x24 + n)) != 0) {
                        m_add(t, n);
                        added = 1;
                    }
                }
                if (!added)
                    break;      /* the fix */
            }
            remove = 1;
            redraw = 1;
            break;
        case -0x63:
            snprintf(buf, sizeof buf, "Restart %s Population Expansion?", mstr(0xC76C4u + 4u * (uint32_t)n));
            if (MB(0xF097Cu) & 0x08) {
                mtr("Y%s ", buf);
                if ((g_myes < g_nyes ? g_yes[g_myes++] : -0x62) != -0x61)
                    break;
            }
            memcpy(&MB(AREA_BASE), sa, sizeof sa);
            memcpy(&MB(NA(1, 0)), sn + 199, 11 * 199);
            memcpy(&MB(TS_BASE), st, sizeof st);
            m_allow(n);
            list3(M, n);
            mtr("L ");
            sel = 0;
            redraw = 1;
            break;
        default:
            break;
        }
        if (remove) {
            cnt = ms32(0xF7C9Cu);
            for (i = sel; i < cnt - 1; i++)
                mw32(0xF7CA0u + 4u * (uint32_t)i, ms32(0xF7CA0u + 4u * (uint32_t)(i + 1)));
            mw32(0xF7C9Cu, cnt - 1);
            if (sel >= cnt - 1)
                sel = cnt - 2;
            if (sel < 0)
                sel = 0;
            remove = 0;
            list3(M, n);
            mtr("L ");
        }
    }
    mtr("X%X ", 0xC8954u);
    mw32(0xEED74u, 0);
    mw32(0xEED7Cu, 0);
    mw32(0xEED78u, 0);
    mw16(0xBB400u, 0x23);
}

static void m_phase(void)
{
    int n, i, need, k = 0;
    mw16(0xBB3FAu, 0xA9);
    mw16(0xBB3FCu, 0x154);
    mw16(0xBB3FEu, 0x12C);
    mw16(0xBB400u, 0x23);
    for (n = 1; n <= 9; n++)
        if (MB(NA(n, 0x9A)) != 0) {
            mw16(0xF06B0u + 2u * (uint32_t)k, n);
            k++;
        }
    mw16(0xF06AEu, k);
    MB(0xEE168u) = 3;
    mtr("P%d ", ms16(0xF06B0u));
    mtr("V%d ", ms16(0xF06B0u));
    for (n = 1; n <= 9; n++) {
        int pbem = (MB(0xE1624u) & 0x80) != 0;
        mw32(0xF40F8u, n);
        MB(0xF488Cu + (uint32_t)n) = 0;
        if (pbem ? ms8(0xE1622u) != n : MB(NA(n, 0x9A)) == 0)
            continue;
        need = 0;
        for (i = 0; i < ms8(NA(n, 0x60)); i++) {
            int8_t tok = ms8(AREA_BASE + 69u * MB(NA(n, 0x61 + i)) + 0x0Eu + (uint32_t)n);
            need += tok >= 2 ? 2 : tok == 1 ? 1 : 0;
        }
        if (ms8(NA(n, 0x99)) >= need) {
            for (i = 0; i < ms8(NA(n, 0x60)); i++) {
                uint32_t a = AREA_BASE + 69u * MB(NA(n, 0x61 + i));
                int8_t tok = ms8(a + 0x0Eu + (uint32_t)n);
                if (tok > 2) {
                    MB(a + 0x0Eu + (uint32_t)n) += 2;
                    MB(NA(n, 0x98)) += 2;
                    MB(NA(n, 0x99)) -= 2;
                } else if (tok > 0) {
                    MB(NA(n, 0x98)) += (uint8_t)tok;
                    MB(NA(n, 0x99)) -= (uint8_t)tok;
                    MB(a + 1u + (uint32_t)n) = (uint8_t)n;
                    MB(a + 0x0Eu + (uint32_t)n) = (uint8_t)(2 * tok);
                }
            }
        } else {
            mtr("B ");
            if (ms8(NA(n, 0x99)) <= 0)
                mtr("M-500:%d:-200:%s, you have no stock for expansion.:0 ", n, mstr(0xC768Cu + 4u * (uint32_t)n));
            else if (ms8(NA(n, 0x9A)) == 3)
                m_manual(n);
            else if (ms8(NA(n, 0x9A)) == 2)
                mtr("A%d ", n);
        }
    }
    if (MB(0xE1624u) & 0x80)
        MB(0xF40FEu) = 1;
    mtr("W1000 ");
}

/* ---- harness ------------------------------------------------------------------------------------------------------- */

static uint8_t *g_before, *g_want;
static struct { uint32_t a; uint8_t v; } g_exp[4096];
static int g_nexp;
static int g_tests, g_fail;

static void clear_records(void)
{
    int n;
    memset(DP(AREA_BASE), 0, AREA_LEN);
    memset(DP(NAT_BASE), 0, NAT_LEN);
    for (n = 1; n <= 13; n++)
        setD8(NA(n, 0x9A), 0);
    setD8(0xE1624u, (uint8_t)(D8(0xE1624u) & 0x7F));
}

static void start(void)
{
    memcpy(g_before, g_ds, DS_SIZE);
    g_nexp = 0;
    g_tlen = 0;
    g_trace[0] = 0;
    g_nclicks = g_nyes = 0;
    memset(g_en, 0, sizeof g_en);
}

static void E8(uint32_t a, uint8_t v) { g_exp[g_nexp].a = a; g_exp[g_nexp].v = v; g_nexp++; }
static void E16(uint32_t a, uint16_t v) { E8(a, (uint8_t)v); E8(a + 1, (uint8_t)(v >> 8)); }
static void E32(uint32_t a, uint32_t v) { E16(a, (uint16_t)v); E16(a + 2, (uint16_t)(v >> 16)); }

/* the expectations every phase run has: message area, player list, [0xF40F8] = 9, 0xF488C[1..9] = 0, 0xEE168 = 3 */
static void E_phase(const int *players, int np)
{
    int i;
    E16(0xBB3FAu, 0xA9); E16(0xBB3FCu, 0x154); E16(0xBB3FEu, 0x12C); E16(0xBB400u, 0x23);
    E16(0xF06AEu, (uint16_t)np);
    for (i = 0; i < np; i++)
        E16(0xF06B0u + 2u * (uint32_t)i, (uint16_t)players[i]);
    E32(0xF40F8u, 9);
    for (i = 1; i <= 9; i++)
        E8(0xF488Cu + (uint32_t)i, 0);
    E8(0xEE168u, 3);
}

/* the list 0x28CFC leaves: entries v[0..c), the rest of the 157 -1 */
static void E_list(const int *v, int c)
{
    int i;
    E32(0xF7C9Cu, (uint32_t)c);
    for (i = 0; i < 157; i++)
        E32(0xF7CA0u + 4u * (uint32_t)i, i < c ? (uint32_t)v[i] : 0xFFFFFFFFu);
}

static void verify(const char *name, const char *want_sub)
{
    uint32_t i;
    int k, diffs = 0;

    memcpy(g_want, g_before, DS_SIZE);
    for (k = 0; k < g_nexp; k++)
        g_want[g_exp[k].a - DS_BASE] = g_exp[k].v;
    for (i = 0; i < DS_SIZE; i++)
        if (g_ds[i] != g_want[i]) {
            if (diffs < 8)
                printf("  %s: byte 0x%05X is %02X, expected %02X (was %02X)\n", name, (unsigned)(i + DS_BASE),
                       g_ds[i], g_want[i], g_before[i]);
            diffs++;
        }
    g_tests++;
    if (diffs != 0 || (want_sub != NULL && strstr(g_trace, want_sub) == NULL)) {
        g_fail++;
        printf("FAIL %s: %d bytes differ\n", name, diffs);
        if (want_sub != NULL && strstr(g_trace, want_sub) == NULL)
            printf("  trace %s\n  lacks %s\n", g_trace, want_sub);
    }
}

/* tokens k of nation n in territory t, with the marker and the area-list entry */
static void put(int n, int t, int k)
{
    int c = S8(NA(n, 0x60));
    setD8(AR(t, 0x0E + n), (uint8_t)k);
    setD8(AR(t, 1 + n), (uint8_t)n);
    setD8(NA(n, 0x61 + c), (uint8_t)(t + 1));
    setD8(NA(n, 0x60), (uint8_t)(c + 1));
    setD8(NA(n, 0x98), (uint8_t)(D8(NA(n, 0x98)) + k));
}

static void part_a(void)
{
    static const int p3[] = { 3, 4, 5 };
    static const int p1[] = { 1 };
    static const int l1[] = { 10 }, l3[] = { 0 };
    char want[200];

    /* A1: nation 3 automatic (1 -> 2, 2 -> 4 with the marker, 5 -> 7), nation 4 out of stock (message), nation 5 a
       computer short of stock (the hook) */
    clear_records();
    setD8(NA(3, 0x9A), 3); put(3, 0, 1); put(3, 7, 2); put(3, 100, 5); setD8(NA(3, 0x99), 10);
    setD8(AR(0, 1 + 3), 0);                           /* marker missing: the expansion writes it */
    setD8(NA(4, 0x9A), 3); put(4, 50, 1); setD8(NA(4, 0x99), 0);
    setD8(NA(5, 0x9A), 2); put(5, 60, 3); put(5, 61, 1); setD8(NA(5, 0x99), 2);
    start();
    g_random = 0; g_nscript = 0;
    f673A8();
    E_phase(p3, 3);
    E8(AR(0, 0x0E + 3), 2); E8(AR(0, 1 + 3), 3);
    E8(AR(7, 0x0E + 3), 4);
    E8(AR(100, 0x0E + 3), 7);
    E8(NA(3, 0x98), 8 + 5); E8(NA(3, 0x99), 5);
    snprintf(want, sizeof want, "B M-500:4:-200:%s, you have no stock for expansion.:0 B A5 W1000 ",
             DSTR(D32(0xC768Cu + 16)));
    verify("A1 automatic, no stock, computer", want);

    /* A2: nation 1 human, t5 one token, t10 three, t20 one: need 4, stock 2. ADD MAX on t5 (allowance 1: one token,
       stock 1, entry removed, list rebuilt [10, 20], selection 0), next (t20), ADD ONE (stock 0, allowance 0: removed,
       list [10], selection clamped to 0), FINISHED */
    clear_records();
    setD8(NA(1, 0x9A), 3); put(1, 5, 1); put(1, 10, 3); put(1, 20, 1); setD8(NA(1, 0x99), 2);
    start();
    g_random = 0; g_nscript = 4; g_iscript = 0;
    g_script[0] = -0x78; g_script[1] = -0xA; g_script[2] = -0x77; g_script[3] = -1;
    f673A8();
    E_phase(p1, 1);
    E8(AR(5, 0x0E + 1), 2); E8(AR(5, 0x24 + 1), 0);
    E8(AR(10, 0x24 + 1), 2);
    E8(AR(20, 0x0E + 1), 2); E8(AR(20, 0x24 + 1), 0);
    E8(NA(1, 0x99), 0); E8(NA(1, 0x98), 5 + 2);
    E_list(l1, 1);
    E32(0xEED74u, 0); E32(0xEED78u, 0); E32(0xEED7Cu, 0);
    verify("A2 manual: ADD MAX on t5", "H5,15 -2A +DC +DD +DB P1 C5 K-120 L ");
    verify("A2 trace: rebuilt list, selection 0 = t10", "H10,15 -2A +DC +DD +DB P1 C10 K-10 ");
    verify("A2 trace: next = t20, ADD ONE", "T1 H20,15 -2A +DC +DD +DB P1 C20 K-119 L ");
    verify("A2 trace: clamped to t10, stock 0, FINISHED", "T0 H10,15 +2A -DC -DD -DB P1 C10 K-1 XC8954 W1000 ");

    /* A3: BUG(1995) guard: nation 1 human, t3 one token, t158 three (allowance 2, never listed): need 3, stock 2.
       DISTRIBUTE ALL adds 1 to t3, then a pass adds nothing: the fix stops (stock 1 left), the entry is removed and
       the list is empty; then FINISHED (the stub ignores that it is disabled) */
    clear_records();
    setD8(NA(1, 0x9A), 3); put(1, 3, 1); put(1, 158, 3); setD8(NA(1, 0x99), 2);
    start();
    g_random = 0; g_nscript = 2; g_iscript = 0;
    g_script[0] = -0x76; g_script[1] = -1;
    f673A8();
    E_phase(p1, 1);
    E8(AR(3, 0x0E + 1), 2); E8(AR(3, 0x24 + 1), 0);
    E8(AR(158, 0x24 + 1), 2);
    E8(NA(1, 0x99), 1); E8(NA(1, 0x98), 4 + 1);
    E_list(l3, 0);
    E32(0xEED74u, 0); E32(0xEED78u, 0); E32(0xEED7Cu, 0);
    verify("A3 DISTRIBUTE ALL guard", "K-118 L TPIRACY T1 H-1,15");

    /* A4 (reviewer): the guard with a duplicate area-list entry: nation 1 human, t3 two tokens listed twice in
       +0x61.. (need 4, allowance 2), stock 3. DISTRIBUTE ALL adds 2 to t3 in two passes, the third adds nothing:
       stock 1 left, the entry removed (selection clamped from -1 to 0), the rebuilt list empty; then FINISHED */
    clear_records();
    setD8(NA(1, 0x9A), 3); put(1, 3, 2); setD8(NA(1, 0x99), 3);
    setD8(NA(1, 0x61 + 1), 3 + 1); setD8(NA(1, 0x60), 2);
    start();
    g_random = 0; g_nscript = 2; g_iscript = 0;
    g_script[0] = -0x76; g_script[1] = -1;
    f673A8();
    E_phase(p1, 1);
    E8(AR(3, 0x0E + 1), 4); E8(AR(3, 0x24 + 1), 0);
    E8(NA(1, 0x99), 1); E8(NA(1, 0x98), 2 + 2);
    E_list(l3, 0);
    E32(0xEED74u, 0); E32(0xEED78u, 0); E32(0xEED7Cu, 0);
    verify("A4 DISTRIBUTE ALL guard, duplicate area-list entry", "H3,15 -2A +DC +DD +DB P1 C3 K-118 L TPIRACY T1");
}

/* a random board: nations 1..9 out / computer / human, tokens on random territories, area lists in random order */
static void random_board(void)
{
    int n, k, i, t, used[160];
    clear_records();
    if (rnd(8) == 0)
        setD8(0xF097Cu, (uint8_t)(D8(0xF097Cu) ^ 0x08));
    for (n = 1; n <= 9; n++) {
        static const uint8_t kinds[] = { 0, 2, 3, 3, 3 };
        int need = 0;
        setD8(NA(n, 0x9A), kinds[rnd(5)]);
        if (D8(NA(n, 0x9A)) == 0 && rnd(3) != 0)
            continue;
        memset(used, 0, sizeof used);
        k = (int)rnd(12);
        for (i = 0; i < k; i++) {
            int tok = rnd(4) == 0 ? 1 + (int)rnd(12) : 1 + (int)rnd(3);
            if (rnd(50) == 0)
                tok = 0x80 + (int)rnd(0x80);              /* a count read as negative: no need, allowance 2 */
            if (rnd(60) == 0)
                tok = 0;                                  /* a stale list entry */
            do t = rnd(80) == 0 ? 157 + (int)rnd(3) : (int)rnd(157); while (used[t]);   /* 157..159: never listed */
            used[t] = 1;
            put(n, t, tok);
            if (rnd(6) == 0)
                setD8(AR(t, 1 + n), 0);                   /* marker missing: 1 or 2 tokens get it written */
            need += tok >= 0x80 ? 0 : tok >= 2 ? 2 : tok;
        }
        setD8(NA(n, 0x99), (uint8_t)(rnd(5) == 0 ? 0 : (int)rnd((uint32_t)need + 4)));
    }
    if (rnd(20) == 0) {
        setD8(0xE1624u, (uint8_t)(D8(0xE1624u) | 0x80));
        setD8(0xE1622u, (uint8_t)(1 + rnd(9)));
    }
}

static long part_b(long runs)
{
    long r, bad = 0;
    uint32_t i;
    int diffs;

    M = malloc(DS_SIZE);
    if (M == NULL)
        exit(1);
    for (r = 0; r < runs; r++) {
        random_board();
        start();
        memcpy(M, g_ds, DS_SIZE);
        g_random = 1;
        f673A8();
        g_mlen = 0;
        g_mtrace[0] = 0;
        g_mclick = g_myes = 0;
        m_phase();
        g_allclicks += g_nclicks;
        g_allyes += g_nyes;
        diffs = 0;
        for (i = 0; i < DS_SIZE; i++)
            if (g_ds[i] != M[i]) {
                if (diffs < 6 && bad < 5)
                    printf("  B run %ld: byte 0x%05X is %02X, model %02X (was %02X)\n", r, (unsigned)(i + DS_BASE),
                           g_ds[i], M[i], g_before[i]);
                diffs++;
            }
        if (diffs != 0 || strcmp(g_trace, g_mtrace) != 0 || g_mclick != g_nclicks || g_myes != g_nyes) {
            if (bad < 5) {
                printf("FAIL B run %ld: %d bytes differ, clicks %d/%d, answers %d/%d\n", r, diffs, g_mclick,
                       g_nclicks, g_myes, g_nyes);
                if (strcmp(g_trace, g_mtrace) != 0)
                    printf("  trace %.600s\n  model %.600s\n", g_trace, g_mtrace);
            }
            bad++;
        }
    }
    free(M);
    return bad;
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:/work/advciv_run";
    long runs = 20000, bad;
    int a_tests, a_fail;

    if (state_init(gamedir) != 0)
        return 1;
    g_before = malloc(DS_SIZE);
    g_want = malloc(DS_SIZE);
    if (g_before == NULL || g_want == NULL)
        return 1;

    g_original = 0;
    part_a();
    a_tests = g_tests;
    a_fail = g_fail;
    printf("A. hand vectors: %d of %d pass\n", a_tests - a_fail, a_tests);

    bad = part_b(runs);
    printf("B. random differential run: %ld phases, %ld disagree (%ld manual sessions, %ld clicks, %ld RESTART "
           "questions, %ld no-stock messages, %ld computer hooks)\n", runs, bad, g_sessions, g_allclicks, g_allyes,
           g_msgs, g_ai);

    return g_fail == 0 && bad == 0 ? 0 : 1;
}
