/* movephase_test [GAMEDIR [ADJDIR]]: unit tests for module MOVE, the phase (movephase.c), that need no game run.

   It loads the data object from GAMEDIR/AC_MAIN.EXE as the port does (state_init; GAMEDIR defaults to the run copy
   C:\work\advciv_run, only read). movephase.c is compiled with -DMOVEPHASE_TEST, so its two calls into the computers'
   code (0x353C4, 0x461BC) go to recorders here, which return. The UI routines are stubs that record their calls and,
   for f3D434 / f78BA4 / f795A8 / f7BA64, return scripted values; f3D434's script names panel buttons, which run the
   real dispatcher cases of move.c (dcase_26 ..), as the original's f3D434 -> f3FB94 does. The primitives (move.c), the
   highlight setters and alias ids (mapdata.c) and the heap handles (load.c) are the real, separately tested ones.
   Expectations are written here from the disassembly, independently of movephase.c:
     A. random differential runs of the helpers against a second implementation (on plain copies of the records):
        f577B0, f57940, f5799C, f5790C, f57A90, f57AFC, f57BD8, f57810 (cursors, both modes, the BUG(1995) 2 guard),
        f582CC, f58330 (random adjacency lists in heap blocks), f4D82C (random move lists, sentinel and merges);
        f574B0 and f57254 / f57160 by their call traces and data.
     B. the computers' playback with random land and ship move lists: f4D518 / f4DACC with the four animated moves,
        show 0 and show 1 (Show AI off: only the sort; on: the waits); the data object against the model's after
        each run.
     C. the phase f55574 on the original's own save at the start of MOVEMENT R4 (GAMEDIR/savgame0.dat) made all-human
        in memory, with scripted clicks: everyone FINISHED; Africa's land move TUNISIA, count 2, THAPSUS (as the
        DOSBox-X dumps of OVERVIEW §5) with and without UNDO; RESTART; FIND UNMOVED; a ship poked in at SABRATA:
        MOVE SHIPS, SABRATA, EMBARK, DISEMBARK, THAPSUS, UNDO; the nothing-left loop; BUG(1995) 1 (no tokens on the
        board) in both modes; a computer's turn with REPLAY and CONTINUE (stack calls, the seed put back). The whole
        data object is compared with the model's, except the UI-only ranges listed at same_ds.
   The adjacency lists come from ADJDIR (default C:\work\advciv_play): LANDADJ.DAT, COASTADJ.DAT, SEAADJ.DAT, read
   here by an independent loader (gfxload.c's f3EED8 needs the whole map set-up).
   Prints each failure, the counts per part, and exits 0 when all pass.

   Build (from the repo root):
     clang -std=c11 -Wall -Wextra -DMOVEPHASE_TEST -Iport/src port/test/movephase_test.c port/src/movephase.c
       port/src/move.c port/src/mapdata.c port/src/load.c port/src/saveload.c port/src/state.c port/src/le_image.c
       -o movephase_test.exe */

#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/state.h"
#include "../src/game.h"
#include "../src/fg.h"
#include "../src/ui.h"
#include "../src/kit.h"
#include "../src/map.h"
#include "../src/mapdata.h"
#include "../src/move.h"
#include "../src/movephase.h"
#include "../src/mstack.h"
#include "../src/panel.h"
#include "../src/saveload.h"
#include "../src/shippanel.h"
#include "../src/stack.h"
#include "../src/start.h"

/* ---- trace, checks ------------------------------------------------------------------------------------------------ */

static char g_trace[1 << 18];
static int g_tests, g_fail;

static void tr(const char *fmt, ...)
{
    size_t len = strlen(g_trace);
    va_list ap;
    if (len > sizeof g_trace - 200)
        return;
    va_start(ap, fmt);
    vsnprintf(g_trace + len, sizeof g_trace - len, fmt, ap);
    va_end(ap);
}

static void check(int ok, const char *what)
{
    g_tests++;
    if (!ok) {
        g_fail++;
        printf("FAIL %s\n", what);
    }
}

/* ---- scripted input ------------------------------------------------------------------------------------------- */

#define BTN(id) (0x1000 + (id))      /* a panel button in f3D434's script */
static int g_clicks[256], g_nclicks, g_iclick;
static int g_ba4[16], g_nba4, g_iba4;      /* f78BA4 */
static int g_yesno[8], g_nyesno, g_iyesno; /* f795A8 */
static int g_unattended;                   /* f7BA64 */
static int g_pressed[0x200];               /* the pressed byte of the panel's buttons, by id */

static void script(const int *c, int n)
{
    memcpy(g_clicks, c, sizeof(int) * (size_t)n);
    g_nclicks = n;
    g_iclick = 0;
}

int f3D434(int x, int y, uint32_t dlg)
{
    int c, code;
    (void)x; (void)y; (void)dlg;
    if (g_iclick >= g_nclicks) {
        printf("FAIL f3D434 called beyond the script\n");
        g_fail++;
        tr("CLICK-END ");
        return -1;
    }
    c = g_clicks[g_iclick++];
    if (c < 0x1000) {
        tr("click=%d ", c);
        return c;
    }
    code = -100;
    switch (c - 0x1000) {   /* the dispatcher's cases (dispatch.c / move.c) */
    case 0x26: code = dcase_26(code); break;
    case 0x27: code = dcase_27(code); break;
    case 0x28: code = dcase_28(code); break;
    case 0x29: code = dcase_29(code); break;
    case 0x2A: code = -1; break;
    case 0x2B: code = dcase_2B(code); break;
    case 0x2C: code = dcase_2C(code); break;
    case 0x2D: code = dcase_2D(code); break;
    case 0x2E: code = dcase_2E(code); break;
    case 0x41: code = -0x63; break;
    case 0xC4: code = dcase_C4(code); break;
    case 0xC5: code = dcase_C5(code); break;
    case 0xC6: code = dcase_C6(code); break;
    case 0xC7: code = dcase_C7(code); break;
    case 0xC8: code = dcase_C8(code); break;
    default: printf("FAIL script button %X\n", c - 0x1000); g_fail++; break;
    }
    tr("btn%X=%d ", c - 0x1000, code);
    return code;
}

void fg_mousepos(int *x, int *y, int *b) { *x = 300; *y = 200; *b = 1; }
void f775F0(void) { setD32(0xE1630u, 0); }
int fg_vbopen(int handle) { (void)handle; return 0; }
void fg_vbclose(void) { }
int fg_vbcopy(int a, int b, int c, int d, int e, int f, int s, int t)
{
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)s; (void)t;
    tr("vbcopy ");
    return 0;
}
int fg_setcolor(int c) { (void)c; return 0; }
void f3CD6C(int8_t pic) { (void)pic; }
void f7876C(void) { tr("7876C "); }
void f7853C(uint32_t dlg) { tr("7853C(%X) ", (unsigned)dlg); }
void f78664(uint32_t dlg) { tr("78664(%X) ", (unsigned)dlg); }
void f5D14C(int n, int i) { tr("5D14C(%d,%d) ", n, i); }
void f3B914(uint32_t dlg, int id, int p) { (void)dlg; g_pressed[id & 0x1FF] = p; tr("3B914(%X,%d) ", id, p); }
void f3B7CC(uint32_t dlg, int id) { (void)dlg; g_pressed[id & 0x1FF] = 1; tr("3B7CC(%X) ", id); }
void f3B870(uint32_t dlg, int id) { (void)dlg; g_pressed[id & 0x1FF] = 0; tr("3B870(%X) ", id); }
int f3B9A4(uint32_t dlg, int id) { (void)dlg; return g_pressed[id & 0x1FF]; }
void f3BFBC(uint32_t dlg, int id, int en) { (void)dlg; tr("3BFBC(%X,%d) ", id, en); }
void f3C314(uint32_t dlg, int id) { (void)dlg; tr("on(%X) ", id); }
void f3C3D0(uint32_t dlg, int id) { (void)dlg; tr("off(%X) ", id); }
void f3B05C(uint32_t dlg) { setD32(0xF8E78u, dlg); tr("3B05C "); }
void f3A9F4(const char *text) { tr("MSG[%s] ", text); }
void f3A924(void) { }
void f3A990(void) { tr("3A990 "); }
void f78884(void) { }
void f788AC(void) { }
void f459E4(int n) { tr("459E4(%d) ", n); }
void f3CBBC(void) { }
void f7844C(uint32_t dlg, int base) { (void)dlg; (void)base; }
void f783B8(uint32_t dlg, int base) { (void)dlg; (void)base; tr("783B8 "); }
void f79270(int wait, int n, int t, const char *text, int anim) { tr("79270(%d,%d,%d,[%s],%d) ", wait, n, t, text, anim); }
void f3C828(uint32_t dlg) { tr("3C828(%X) ", (unsigned)dlg); }
void f3C7A0(uint32_t dlg) { tr("3C7A0(%X) ", (unsigned)dlg); }
void f3CC14(void) { }
void f3D52C(int dx, int dy) { (void)dx; (void)dy; }
void f787F8(int t) { tr("787F8(%d) ", t); }
void f426D0(int i) { (void)i; }
void f572CC(int t) { setD16(0xF09E0u, 1); tr("572CC(%d) ", t); }   /* the real one's data write: int16 [0xF09E0] = 1 */
void f78A20(int t) { tr("W%d ", t); }
void f7A8D0(void) { tr("7A8D0 "); }
void f7A940(void) { tr("PUSH "); }
void f7AC60(void) { tr("POP "); }
void f7AD24(void) { tr("DROP "); }
void f7ABD0(void) { tr("7ABD0 "); }
void f3F6E4(void) { memset(DP(0xFFC00u), 0, 0xA0); }
void f3F758(void) { memset(DP(0xFFCA0u), 0, 0xA0); }
int f7BA64(void) { return g_unattended; }
int f78BA4(uint32_t dlg, int map)
{
    int c = g_iba4 < g_nba4 ? g_ba4[g_iba4++] : -1;
    (void)dlg; (void)map;
    tr("78BA4=%d ", c);
    return c;
}
int f795A8(const char *text)
{
    int r = g_iyesno < g_nyesno ? g_yesno[g_iyesno++] : -0x62;
    tr("YESNO[%s]=%d ", text, r);
    return r;
}
void f3A780(const char *msg) { tr("UNEXPECTED 3A780[%s] ", msg); }
_Noreturn void f44CAC(void) { printf("FAIL quit: %s\n", DSTR(0xF5500u)); exit(2); }
int f78AE4(void) { tr("UNEXPECTED rand "); return 0; }
void f76F70(int n, uint8_t *cards) { (void)n; (void)cards; }
void (*g_not_ported_hook)(uint32_t addr);
_Noreturn void not_ported(uint32_t addr) { printf("FAIL not_ported(0x%X)\n", (unsigned)addr); exit(2); }

/* the computers' code (MOVEPHASE_TEST hooks): each uses up random numbers (the seed changes) */
void movephase_test_f353C4(int n) { tr("AI353C4(%d,seed %08X) ", n, (unsigned)D32(0xCAC5Cu)); setD32(0xCAC5Cu, D32(0xCAC5Cu) * 3u + 7u); }
void movephase_test_f461BC(int n) { tr("AI461BC(%d) ", n); setD32(0xCAC5Cu, D32(0xCAC5Cu) * 5u + 1u); }

/* ---- addresses (from the listing's constants, written out here) ---------------------------------------------------- */

#define AREA_BASE 0xE31B8u
#define AREA_LEN  (161u * 69u)
#define NAT_BASE  0xF14CCu
#define NAT_LEN   (14u * 199u)
#define TS_BASE   0xE2C18u
#define AR(t, off) (AREA_BASE + 69u * (uint32_t)((t) + 1) + (uint32_t)(off))
#define NA(n, off) (NAT_BASE + 199u * (uint32_t)(n) + (uint32_t)(off))

/* ---- the model's copy and its accessors ---------------------------------------------------------------------------- */

static uint8_t *m_ds, *g_real, *g_before;
static uint32_t rng = 12345;

static uint32_t rnd(uint32_t n) { rng = rng * 1103515245u + 12345u; return n ? (rng >> 8) % n : 0; }
static int   M8(uint32_t a) { return m_ds[a - DS_BASE]; }
static int   MS8(uint32_t a) { return (int8_t)m_ds[a - DS_BASE]; }
static int   MS16(uint32_t a) { return (int16_t)(m_ds[a - DS_BASE] | m_ds[a - DS_BASE + 1] << 8); }
static int32_t MS32(uint32_t a) { const uint8_t *p = m_ds + (a - DS_BASE); return (int32_t)((uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24); }
static void  MW8(uint32_t a, int v) { m_ds[a - DS_BASE] = (uint8_t)v; }
static void  MW16(uint32_t a, int v) { m_ds[a - DS_BASE] = (uint8_t)v; m_ds[a - DS_BASE + 1] = (uint8_t)(v >> 8); }
static void  MW32(uint32_t a, int32_t v) { uint8_t *p = m_ds + (a - DS_BASE); uint32_t u = (uint32_t)v; p[0] = (uint8_t)u; p[1] = (uint8_t)(u >> 8); p[2] = (uint8_t)(u >> 16); p[3] = (uint8_t)(u >> 24); }

/* the verified primitives (move.c), run on the model's copy */
static void on_model(void) { g_real = g_ds; g_ds = m_ds; }
static void off_model(void) { g_ds = g_real; }
static void m_f57CE8(int t, int n, int flag, int k) { on_model(); f57CE8(t, n, flag, k); off_model(); }
static void m_f57E28(int t, int n, int k) { on_model(); f57E28(t, n, k); off_model(); }
static void m_f580CC(int a, int b, int n) { on_model(); f580CC(a, b, n); off_model(); }
static void m_f576F8(int t) { on_model(); f576F8(t); off_model(); }
static int  m_alias(int t)   /* f3E8FC, written from mapdata.h's description */
{
    int c = M8(0xC803Cu + (uint32_t)t);
    if (MS16(0x109E14u) == 1 && (c == 0x39 || c == 0x0D || c == 0x87))
        return c == 0x39 ? 0x9D : c == 0x0D ? 0x9E : 0x9F;
    return t;
}

static void model_sync(void) { memcpy(m_ds, g_ds, DS_SIZE); }

/* first differing byte outside the excluded ranges, or 0 */
typedef struct { uint32_t a, b; } range;
static const range g_skip_ui[] = {
    {0xFFB60u, 0xFFD40u},   /* border / token / ship box colours (UI only) */
    {0xE1628u, 0xE1634u},   /* the mouse */
    {0xF4884u, 0xF488Cu},   /* the map scroll (f3DA60 / f3D9FC) */
    {0xF8E78u, 0xF8E7Cu},   /* the dialog last drawn */
    {0xF5500u, 0xF56F4u},   /* the message buffer */
};
static int same_ds_skip(const char *what, const range *extra, int nextra)
{
    uint32_t a;
    int i, skip;
    char buf[200];

    for (a = DS_BASE; a < DS_BASE + DS_SIZE; a++) {
        if (g_ds[a - DS_BASE] == m_ds[a - DS_BASE])
            continue;
        skip = 0;
        for (i = 0; i < (int)(sizeof g_skip_ui / sizeof g_skip_ui[0]); i++)
            if (a >= g_skip_ui[i].a && a < g_skip_ui[i].b)
                skip = 1;
        for (i = 0; i < nextra; i++)
            if (a >= extra[i].a && a < extra[i].b)
                skip = 1;
        if (skip)
            continue;
        snprintf(buf, sizeof buf, "%s: byte 0x%X is %02X, model %02X", what, (unsigned)a, g_ds[a - DS_BASE], m_ds[a - DS_BASE]);
        check(0, buf);
        return 0;
    }
    check(1, what);
    return 1;
}
static int same_ds(const char *what) { return same_ds_skip(what, NULL, 0); }
static int diff_ds(void)   /* 1 if the data object differs from the model's outside the UI ranges */
{
    uint32_t a;
    int i, skip;
    for (a = DS_BASE; a < DS_BASE + DS_SIZE; a++) {
        if (g_ds[a - DS_BASE] == m_ds[a - DS_BASE])
            continue;
        skip = 0;
        for (i = 0; i < (int)(sizeof g_skip_ui / sizeof g_skip_ui[0]); i++)
            if (a >= g_skip_ui[i].a && a < g_skip_ui[i].b)
                skip = 1;
        if (!skip)
            return 1;
    }
    return 0;
}

/* the waits (f78A20 values) in the trace, as "a,b,c," */
static void waits(char *out, size_t size)
{
    static char copy[sizeof g_trace];
    char *tok;
    out[0] = 0;
    strcpy(copy, g_trace);
    for (tok = strtok(copy, " "); tok != NULL; tok = strtok(NULL, " "))
        if (tok[0] == 'W') {
            size_t l = strlen(out);
            snprintf(out + l, size - l, "%d,", atoi(tok + 1));
        }
}

/* ---- A. the helpers -------------------------------------------------------------------------------------------------- */

static int m577B0(int n) { int r; for (r = 1; r <= 157; r++) if (MS8(AREA_BASE + 69u * (uint32_t)r + (uint32_t)n + 0x24u) != 0) return r - 1; return -1; }
static int m57940(int n) { int r; for (r = 1; r <= 157; r++) if (M8(AREA_BASE + 69u * (uint32_t)r + (uint32_t)n + 0x24u) != 0) return 1; return 0; }
static int m5799C(int n) { int k; for (k = 0; k < M8(NA(n, 0x40)); k++) if (M8(NA(n, 0x41 + k)) > 0 || MS8(NA(n, 0x4D + k)) != 0) return 1; return 0; }
static int m57A90(int t, int n) { int k; for (k = 0; k < M8(NA(n, 0x40)); k++) if (MS16(NA(n, 0x45 + 2 * k)) == t + 1) return 1; return 0; }
static int m57AFC(int n) { int k; for (k = 0; k < M8(NA(n, 0x40)); k++) if (M8(NA(n, 0x4D + k)) != 0) return 1; return 0; }
static int m57BD8(int t, int n)
{
    int i, k, mode = MS8(0x108804u);
    if (mode == 1) {
        if (M8(AR(t, 1)) == 0)
            return 0;
        for (i = 1; i <= 9; i++)
            if (MS8(AR(t, 1 + i)) == n)
                return 1;
        return 0;
    }
    if (mode == 2)
        for (k = 0; k < 4; k++)
            if (M8(NA(n, 0x45 + 2 * k)) == t + 1)
                return 1;
    return 0;
}
static int m57810(int n)   /* the fixed version: one wrap, then -1 */
{
    int pass, r, k;
    if (MS8(0x108804u) == 1) {
        for (pass = 0; pass < 2; pass++) {
            for (r = MS32(0x109E18u) + 1; r <= 157; r++)
                if (M8(AREA_BASE + 69u * (uint32_t)r + (uint32_t)n + 0x24u) != 0) {
                    MW32(0x109E18u, r);
                    return r - 1;
                }
            MW32(0x109E18u, 0);
        }
        return -1;
    }
    for (pass = 0; pass < 2; pass++) {
        for (k = MS32(0x109E1Cu) + 1; k < 4; k++)
            if (M8(NA(n, 0x45 + 2 * k)) != 0 && M8(NA(n, 0x41 + k)) > 0) {
                MW32(0x109E1Cu, k);
                return M8(NA(n, 0x45 + 2 * k)) - 1;
            }
        MW32(0x109E1Cu, -1);
    }
    return -1;
}

/* adjacency lists in three heap blocks: list t = int16 count, then the entries */
#define ADJ_W 24
static int16_t g_adj[3][160][ADJ_W];
static uint32_t g_adj_block[3];
static void install_adj(void)
{
    static const uint32_t ptr[3] = {GAME_LANDADJ_PTR, GAME_COASTADJ_PTR, GAME_SEAADJ_PTR};
    int l, t, i;
    for (l = 0; l < 3; l++) {
        if (g_adj_block[l] == 0)
            g_adj_block[l] = load_heap_alloc(160u * 2u * ADJ_W);
        for (t = 0; t < 160; t++) {
            uint8_t *p = load_heap_ptr(g_adj_block[l] + 2u * ADJ_W * (uint32_t)t, 2u * ADJ_W);
            for (i = 0; i < ADJ_W; i++) {
                p[2 * i] = (uint8_t)g_adj[l][t][i];
                p[2 * i + 1] = (uint8_t)((uint16_t)g_adj[l][t][i] >> 8);
            }
            setD32(ptr[l] + 4u * (uint32_t)t, g_adj_block[l] + 2u * ADJ_W * (uint32_t)t);
        }
    }
}
static int in_list(int l, int t, int u) { int i; for (i = 1; i <= g_adj[l][t][0]; i++) if (g_adj[l][t][i] == u) return 1; return 0; }
static int m582CC(int t, int u) { return in_list(0, t, u) ? 1 : -2; }
static int m58330(int t, int u, int n)
{
    int astro = M8(NA(n, 0x2B)) != 0;
    int k = M8(AR(t, 0x2D + n));
    if (M8(NA(n, 0x41 + k)) > 1)
        return in_list(astro ? 2 : 1, t, u) ? 1 : -2;
    if (in_list(1, t, u))
        return 1;
    return astro ? -3 : -2;
}

static int g_wild = 1;   /* ship words outside 0..160 (part A); part B keeps them as the game writes them */
static void random_records(void)
{
    uint32_t a;
    int r, n, k;
    for (a = AREA_BASE; a < AREA_BASE + AREA_LEN; a++)
        g_ds[a - DS_BASE] = 0;
    for (r = 1; r <= 160; r++)
        for (n = 1; n <= 9; n++)
            if (rnd(6) == 0) {
                setD8(AREA_BASE + 69u * (uint32_t)r + 1u + (uint32_t)n, (uint8_t)(rnd(4) == 0 ? rnd(12) : (uint32_t)n));
                setD8(AREA_BASE + 69u * (uint32_t)r + 1u, (uint8_t)(D8(AREA_BASE + 69u * (uint32_t)r + 1u) + 1u));
                setD8(AREA_BASE + 69u * (uint32_t)r + 0x0Eu + (uint32_t)n, (uint8_t)(rnd(4) == 0 ? rnd(256) : rnd(7)));
                setD8(AREA_BASE + 69u * (uint32_t)r + 0x24u + (uint32_t)n, (uint8_t)(rnd(3) == 0 ? 0 : rnd(4) == 0 ? rnd(256) : rnd(6)));
                setD8(AREA_BASE + 69u * (uint32_t)r + 0x2Du + (uint32_t)n, (uint8_t)rnd(4));
            }
    for (n = 1; n <= 9; n++) {
        memset(DP(NA(n, 0)), 0, 199);   /* the territory list +0x60.. too: it only grows otherwise */
        setD8(NA(n, 0x40), (uint8_t)rnd(5));
        for (k = 0; k < 4; k++) {
            setD8(NA(n, 0x41 + k), (uint8_t)(rnd(3) == 0 ? 0 : rnd(4) == 0 ? rnd(256) : rnd(6)));
            setD16(NA(n, 0x45 + 2 * k), (uint16_t)(rnd(3) == 0 ? 0 : (g_wild && rnd(8) == 0) ? rnd(0x10000) : 1 + rnd(160)));
            setD8(NA(n, 0x4D + k), (uint8_t)(rnd(3) == 0 ? 0 : rnd(4) == 0 ? rnd(256) : rnd(4)));
        }
        setD8(NA(n, 0x2B), (uint8_t)rnd(2));
    }
}

static void part_a(int iters)
{
    int it, n, t, u, i, r1, r2, ok = 1, okl = 1, oks = 1, okf = 1;
    char buf[200];

    for (it = 0; it < iters; it++) {
        random_records();
        setD8(0x108804u, (uint8_t)(rnd(5) == 0 ? rnd(4) : 1 + rnd(2)));
        setD32(0x109E18u, (uint32_t)rnd(157));
        setD32(0x109E1Cu, (uint32_t)rnd(5) - 1u);
        model_sync();
        n = 1 + (int)rnd(9);
        t = (int)rnd(160);
        if (f577B0(n) != m577B0(n) || f57940(n) != m57940(n) || f5799C(n) != m5799C(n)
            || f5790C(n) != (m57940(n) || m5799C(n)) || f57A90(t, n) != m57A90(t, n) || f57AFC(n) != m57AFC(n)
            || f57BD8(t, n) != m57BD8(t, n)) {
            if (ok) {
                snprintf(buf, sizeof buf, "A helpers it %d n %d t %d: 577B0 %d/%d 57BD8 %d/%d", it, n, t, f577B0(n),
                         m577B0(n), f57BD8(t, n), m57BD8(t, n));
                check(0, buf);
            }
            ok = 0;
        }
        /* FIND UNMOVED: call it several times; in the original mode only when something qualifies */
        for (i = 0; i < 6; i++) {
            int any = MS8(0x108804u) == 1 ? m57940(n) : 0;
            int k;
            if (MS8(0x108804u) != 1)
                for (k = 0; k < 4; k++)
                    if (M8(NA(n, 0x45 + 2 * k)) != 0 && M8(NA(n, 0x41 + k)) > 0)
                        any = 1;
            g_original = (any && (it & 1)) ? 1 : 0;
            r1 = f57810(n);
            r2 = m57810(n);
            g_original = 0;
            if (r1 != r2 || memcmp(g_ds, m_ds, DS_SIZE) != 0) {
                if (okf) {
                    snprintf(buf, sizeof buf, "A f57810 it %d call %d: %d, model %d", it, i, r1, r2);
                    check(0, buf);
                }
                okf = 0;
            }
        }
    }
    check(ok, "A f577B0 f57940 f5799C f5790C f57A90 f57AFC f57BD8 (random)");
    check(okf, "A f57810 cursors and the BUG(1995) 2 guard (random)");

    /* f582CC / f58330 on random lists */
    for (it = 0; it < iters / 20; it++) {
        int l;
        for (l = 0; l < 3; l++)
            for (t = 0; t < 160; t++) {
                g_adj[l][t][0] = (int16_t)rnd(ADJ_W);
                for (i = 1; i < ADJ_W; i++)
                    g_adj[l][t][i] = (int16_t)rnd(160);
            }
        install_adj();
        random_records();
        model_sync();
        for (i = 0; i < 200; i++) {
            t = (int)rnd(160);
            u = (int)rnd(160);
            n = 1 + (int)rnd(9);
            if (rnd(3) == 0 && g_adj[0][t][0] > 0)
                u = g_adj[0][t][1 + rnd((uint32_t)g_adj[0][t][0])];
            if (f582CC(t, u) != m582CC(t, u))
                okl = 0;
            if (rnd(2) == 0 && g_adj[1][t][0] > 0)
                u = g_adj[1][t][1 + rnd((uint32_t)g_adj[1][t][0])];
            else if (rnd(2) == 0 && g_adj[2][t][0] > 0)
                u = g_adj[2][t][1 + rnd((uint32_t)g_adj[2][t][0])];
            if (f58330(t, u, n) != m58330(t, u, n))
                oks = 0;
        }
    }
    check(okl, "A f582CC (random lists)");
    check(oks, "A f58330 (random lists, points, Astronomy)");

    /* f4D82C */
    ok = 1;
    for (it = 0; it < iters / 4; it++) {
        int cnt = (int)rnd(60), m, j, k;
        uint32_t loc[256];
        for (i = 0; i <= 200; i++)
            setD32(0x107694u + 4u * (uint32_t)i, rng = rng * 69069u + 1u);
        for (i = 0; i < cnt; i++) {
            setD8(0x107694u + 4u * (uint32_t)i, (uint8_t)(rnd(8) == 0 ? 0xA0 + rnd(0x60) : rnd(6)));
            setD8(0x107695u + 4u * (uint32_t)i, (uint8_t)rnd(6));
            setD8(0x107696u + 4u * (uint32_t)i, (uint8_t)(rnd(5) == 0 ? 51 + rnd(40) : rnd(4) == 0 ? rnd(256) : 1 + rnd(3)));
        }
        setD32(0x1079B4u, (uint32_t)cnt);
        model_sync();
        f4D82C();
        /* the model: sentinel, partition, two stable-direction bubble passes, merges */
        MW8(0x107694u + 4u * (uint32_t)cnt, 0xFF);
        for (i = 0; i < cnt; i++)
            loc[i] = (uint32_t)MS32(0x107694u + 4u * (uint32_t)i);
        m = 0;
        for (i = 0; i < cnt; i++)
            if ((int8_t)(loc[i] >> 16) < 50)
                MW32(0x107694u + 4u * (uint32_t)m++, (int32_t)loc[i]);
        k = m;
        for (i = 0; i < cnt; i++)
            if ((int8_t)(loc[i] >> 16) >= 50)
                MW32(0x107694u + 4u * (uint32_t)k++, (int32_t)loc[i]);
        MW32(0x1079B4u, k);
        for (i = 1; i < m; i++)
            for (j = m - 1; j >= i; j--)
                if (M8(0x107695u + 4u * (uint32_t)(j - 1)) > M8(0x107695u + 4u * (uint32_t)j)) {
                    int32_t x = MS32(0x107694u + 4u * (uint32_t)(j - 1));
                    MW32(0x107694u + 4u * (uint32_t)(j - 1), MS32(0x107694u + 4u * (uint32_t)j));
                    MW32(0x107694u + 4u * (uint32_t)j, x);
                }
        for (i = 1; i < m; i++)
            for (j = m - 1; j >= i; j--)
                if (M8(0x107694u + 4u * (uint32_t)(j - 1)) > M8(0x107694u + 4u * (uint32_t)j)) {
                    int32_t x = MS32(0x107694u + 4u * (uint32_t)(j - 1));
                    MW32(0x107694u + 4u * (uint32_t)(j - 1), MS32(0x107694u + 4u * (uint32_t)j));
                    MW32(0x107694u + 4u * (uint32_t)j, x);
                }
        for (i = 0; i < m - 1; i++) {
            if (M8(0x107694u + 4u * (uint32_t)i) > 160)
                continue;
            j = i;
            while (M8(0x107694u + 4u * (uint32_t)(j + 1)) == M8(0x107694u + 4u * (uint32_t)i)
                   && M8(0x107695u + 4u * (uint32_t)(j + 1)) == M8(0x107695u + 4u * (uint32_t)i)) {
                j++;
                MW8(0x107694u + 4u * (uint32_t)j, 0xFF);
            }
            MW8(0x107696u + 4u * (uint32_t)i, M8(0x107696u + 4u * (uint32_t)i) + (j - i));
        }
        if (memcmp(g_ds, m_ds, DS_SIZE) != 0) {
            if (ok) {
                snprintf(buf, sizeof buf, "A f4D82C it %d (count %d)", it, cnt);
                check(0, buf);
            }
            ok = 0;
        }
    }
    check(ok, "A f4D82C (random lists)");

    /* f574B0 */
    for (i = 0; i <= 6; i++) {
        setD16(0xF09E0u, (uint16_t)i);
        g_trace[0] = 0;
        f574B0();
        snprintf(buf, sizeof buf, "3BFBC(C4,%d) 3BFBC(C5,%d) 3BFBC(C6,%d) 3BFBC(C7,%d) 3BFBC(C8,%d) %s %s %s %s %s ",
                 i >= 1, i >= 2, i >= 3, i >= 4, i >= 5, i == 1 ? "3B7CC(C4)" : "3B870(C4)",
                 i == 2 ? "3B7CC(C5)" : "3B870(C5)", i == 3 ? "3B7CC(C6)" : "3B870(C6)",
                 i == 4 ? "3B7CC(C7)" : "3B870(C7)", i == 5 ? "3B7CC(C8)" : "3B870(C8)");
        check(strcmp(g_trace, buf) == 0, "A f574B0 trace");
    }

    /* f57254: PBEM always asks; else only with bit 0x10 */
    {
        static const struct { int pbem, tog, ans, want, asked; } v[] = {
            {0, 0x00, -0x62, 1, 0}, {0, 0x10, -0x61, 1, 1}, {0, 0x10, -0x62, 0, 1},
            {1, 0x00, -0x61, 1, 1}, {1, 0x00, -0x62, 0, 1}, {1, 0x10, -0x61, 1, 1}};
        uint8_t f = D8(0xE1624u), tg = D8(0xF097Cu);
        for (i = 0; i < 6; i++) {
            setD8(0xE1624u, (uint8_t)(v[i].pbem ? 0x80 : 0));
            setD8(0xF097Cu, (uint8_t)v[i].tog);
            g_yesno[0] = v[i].ans; g_nyesno = 1; g_iyesno = 0;
            g_trace[0] = 0;
            r1 = f57254(1);
            check(r1 == v[i].want && (strstr(g_trace, "YESNO[Are you sure that you are Finished movement for AFRICA?]") != NULL) == v[i].asked,
                  "A f57254 (PBEM, toggle, answer)");
        }
        setD8(0xE1624u, f);
        setD8(0xF097Cu, tg);
    }
}

/* ---- B. the computers' playback ---------------------------------------------------------------------------------- */

static void m_playback(int n, int show)
{
    int i, from, to, c, fa, j, k, cnt, type;

    if (MS32(0x1079B4u) > 0) {
        MW8(0x108804u, 1);
        if (show) {
            on_model();
            f4D82C();   /* tested in A against its own model */
            off_model();
        }
        for (i = 0; i < MS32(0x1079B4u); i++) {
            from = M8(0x107694u + 4u * (uint32_t)i);
            to = M8(0x107695u + 4u * (uint32_t)i);
            if (from == 0xFF || from == 0xC9)
                continue;
            if (from == 0xC8) {
                MW8(TS_BASE + 3u * (uint32_t)to + 2u, M8(TS_BASE + 3u * (uint32_t)to + 2u) | 2);
                continue;
            }
            c = MS8(0x107696u + 4u * (uint32_t)i);
            fa = 0;
            if (c > 75)
                c -= 75, fa = 1;
            else if (c > 50)
                c -= 50, fa = 1;
            (void)fa;
            m_f57CE8(from, n, 1, c);
            m_f57E28(to, n, c);
        }
    }
    if (MS32(0x107D3Cu) > 0) {
        MW8(0x108804u, 2);
        for (i = 0; i < MS32(0x107D3Cu); i++) {
            uint32_t r6 = 0x1079B8u + 6u * (uint32_t)i;
            from = M8(r6);
            to = M8(r6 + 1);
            type = M8(r6 + 4);
            if (type == 3) {
                int t2 = m_alias(from);
                MW8(AR(from, 0x2D + n), M8(r6 + 3));
                for (j = 0; j < MS8(r6 + 2); j++) {
                    m_f57CE8(M8(0xC803Cu + (uint32_t)from), n, 1, 1);
                    k = M8(AR(t2, 0x2D + n));
                    MW8(NA(n, 0x4D + k), M8(NA(n, 0x4D + k)) + 1);
                }
            } else if (type == 4 || type == 5) {
                int t2, d, w;
                MW8(AR(to, 0x2D + n), M8(r6 + 3));
                t2 = m_alias(to);
                cnt = type == 4 ? MS8(r6 + 2) : MS8(NA(n, 0x4D + M8(AR(t2, 0x2D + n))));
                w = MS16(NA(n, 0x45 + 2 * M8(AR(t2, 0x2D + n))));
                d = M8((uint32_t)(0xC803B + w));
                for (j = 0; j < cnt; j++) {
                    m_f57E28(d, n, 1);
                    k = M8(AR(t2, 0x2D + n));
                    MW8(NA(n, 0x4D + k), M8(NA(n, 0x4D + k)) - 1);
                }
            } else if (type == 10) {
                MW8(AR(from, 0x2D + n), M8(r6 + 3));
                m_f580CC(from, to, n);
                k = M8(AR(to, 0x2D + n));
                MW8(NA(n, 0x41 + k), M8(NA(n, 0x41 + k)) - 1);
                m_f576F8(from);
            }
        }
        MW8(0x108804u, 1);
    }
}

static void part_b(int iters)
{
    int it, i, n, show, ok = 1;
    char buf[200];
    uint8_t tog = D8(0xF097Cu), tog2 = D8(0xF097Du);

    g_wild = 0;

    for (it = 0; it < iters; it++) {
        random_records();
        n = 1 + (int)rnd(9);
        setD32(0xF40F8u, (uint32_t)n);
        setD16(0x109E14u, (uint16_t)rnd(2));
        setD32(0x1079B4u, rnd(4) == 0 ? 0 : rnd(40));
        for (i = 0; i < S32(0x1079B4u); i++) {
            uint32_t a = 0x107694u + 4u * (uint32_t)i;
            int f = rnd(10) == 0 ? (rnd(2) ? 0xC8 : rnd(2) ? 0xC9 : 0xFF) : (int)rnd(157);
            setD8(a, (uint8_t)f);
            setD8(a + 1, (uint8_t)rnd(157));
            setD8(a + 2, (uint8_t)(rnd(6) == 0 ? 50 + 1 + rnd(4) : rnd(6) == 0 ? 75 + 1 + rnd(4) : 1 + rnd(3)));
            setD8(a + 3, (uint8_t)rnd(256));
        }
        setD32(0x107D3Cu, rnd(3) == 0 ? 0 : rnd(12));
        for (i = 0; i < S32(0x107D3Cu); i++) {
            uint32_t a = 0x1079B8u + 6u * (uint32_t)i;
            static const int types[] = {3, 4, 5, 10, 6, 9, 2, 11, 0xFF};
            setD8(a, (uint8_t)rnd(160));
            setD8(a + 1, (uint8_t)rnd(160));
            setD8(a + 2, (uint8_t)rnd(4));
            setD8(a + 3, (uint8_t)rnd(4));
            setD8(a + 4, (uint8_t)types[rnd(9)]);
            setD8(a + 5, (uint8_t)rnd(256));
        }
        show = (int)rnd(2);
        setD8(0xF097Cu, (uint8_t)(rnd(2) ? 0x40 : 0));
        setD8(0xF097Du, (uint8_t)rnd(2));
        setD8(0xF4102u, 5);
        model_sync();
        g_trace[0] = 0;
        f4D518(n, show);
        m_playback(n, show);
        if (diff_ds()) {
            if (ok) {
                snprintf(buf, sizeof buf, "B playback it %d n %d show %d", it, n, show);
                same_ds(buf);
            }
            ok = 0;
        }
    }
    check(ok, "B f4D518 / f4DACC / f5A084 / f5A2C4 / f5A4BC / f5A750: data (random lists, show 0 and 1)");

    /* the waits of one shown land move, Speedup off and on, and the end of the list */
    random_records();
    n = 1;
    setD32(0xF40F8u, 1);
    setD8(AR(10, 2), 1);
    setD8(AR(10, 1), 1);
    setD8(AR(10, 0x0F), 3);
    setD8(AR(10, 0x25), 3);
    setD32(0x1079B4u, 1);
    setD32(0x107694u, 10u | 11u << 8 | 1u << 16);
    setD32(0x107D3Cu, 0);
    setD8(0xF097Cu, 0x40);
    setD8(0xF097Du, 0);
    g_trace[0] = 0;
    f4D518(1, 1);
    {
        char seq[200];
        waits(seq, sizeof seq);
        check(strcmp(seq, "100,100,100,100,500,100,100,100,500,300,300,") == 0, "B the waits of a shown land move (no Speedup)");
        if (strcmp(seq, "100,100,100,100,500,100,100,100,500,300,300,") != 0)
            printf("   waits: %s\n", seq);
    }
    check(strstr(g_trace, "MSG[AFRICA is moving.]") != NULL && S16(0xBB3FCu) != 0x14C, "B \"%s is moving.\" with the message area restored");
    setD8(0xF097Du, 1);
    setD32(0x1079B4u, 1);
    setD32(0x107694u, 10u | 11u << 8 | 1u << 16);
    g_trace[0] = 0;
    f4D518(1, 1);
    {
        char seq[200];
        waits(seq, sizeof seq);
        check(strcmp(seq, "100,100,300,100,100,100,300,") == 0, "B the waits with Speedup AI Movement");
        if (strcmp(seq, "100,100,300,100,100,100,300,") != 0)
            printf("   waits: %s\n", seq);
    }
    setD8(0xF097Cu, tog);
    setD8(0xF097Du, tog2);
}

/* ---- C. the phase on the R4 save ------------------------------------------------------------------------------------ */

static uint8_t *g_save0;
static int load_save(const char *path)
{
    int16_t ver;
    char desc[64];
    FILE *fp = fopen(path, "rb");
    if (fp == NULL)
        return -1;
    f5F05C(&ver, desc, fp);
    fclose(fp);
    printf("C. %s: \"%s\", phase %d, round %d\n", path, desc, D8(0xF4102u), S16(0xF40FCu));
    return ver;
}

static int load_adj(const char *dir)
{
    static const char *names[3] = {"LANDADJ.DAT", "COASTADJ.DAT", "SEAADJ.DAT"};
    char path[400];
    int l, t, i;
    for (l = 0; l < 3; l++) {
        FILE *fp;
        uint8_t b[4];
        snprintf(path, sizeof path, "%s/%s", dir, names[l]);
        fp = fopen(path, "rb");
        if (fp == NULL)
            return -1;
        if (fread(b, 4, 1, fp) != 1) { fclose(fp); return -1; }
        for (t = 0; t < 160; t++) {
            int cnt;
            if (fread(b, 2, 1, fp) != 1) { fclose(fp); return -1; }
            cnt = (int16_t)(b[0] | b[1] << 8);
            if (cnt > ADJ_W - 1)
                printf("C. %s: territory %d has %d neighbours, more than the test keeps\n", names[l], t, cnt);
            g_adj[l][t][0] = (int16_t)(cnt > ADJ_W - 1 ? ADJ_W - 1 : cnt);
            for (i = 0; i < cnt; i++) {
                if (fread(b, 2, 1, fp) != 1) { fclose(fp); return -1; }
                if (i < ADJ_W - 1)
                    g_adj[l][t][1 + i] = (int16_t)(b[0] | b[1] << 8);
            }
        }
        fclose(fp);
    }
    install_adj();
    return 0;
}

/* the state the phase starts from: the save, every nation in play human (or as kinds[] says), confirmations off */
static void base(const int *kinds)
{
    int n;
    memcpy(g_ds, g_save0, DS_SIZE);
    install_adj();
    for (n = 1; n <= 9; n++)
        if (D8(NA(n, 0x9A)) != 0)
            setD8(NA(n, 0x9A), (uint8_t)(kinds != NULL && kinds[n] ? kinds[n] : 3));
    setD8(0xF097Cu, (uint8_t)(D8(0xF097Cu) & ~0x18u));
    setD8(0xE1624u, (uint8_t)(D8(0xE1624u) & ~0x80u));
    memset(g_pressed, 0, sizeof g_pressed);
}

static int g_order[16], g_norder;

/* the model of the phase's set-up (0x55574..0x558AA) */
static void m_setup(void)
{
    int i, pass, n;
    MW16(0xBB3FAu, 0xA9); MW16(0xBB3FCu, 0x14E); MW16(0xBB3FEu, 0x12C); MW16(0xBB400u, 0x22);
    for (i = 0; i < 157; i++) {
        MW8(TS_BASE + 3u * (uint32_t)i + 2u, M8(TS_BASE + 3u * (uint32_t)i + 2u) & ~2);
        if ((M8(TS_BASE + 3u * (uint32_t)i + 2u) & 1) == 0) {
            MW8(TS_BASE + 3u * (uint32_t)i, 0);
            MW8(TS_BASE + 3u * (uint32_t)i + 1u, 0);
        }
    }
    g_norder = 0;
    for (pass = 0; pass < 2; pass++)
        for (i = 0; i < MS16(0xF0698u); i++) {
            n = MS16(0xF069Au + 2u * (uint32_t)i);
            if ((M8(NA(n, 0x35)) != 0) == pass)
                g_order[g_norder++] = n;
        }
    MW16(0xF06AEu, g_norder);
    for (i = 0; i < g_norder; i++)
        MW16(0xF06B0u + 2u * (uint32_t)i, g_order[i]);
    MW32(0xF09E4u, 0);
    for (n = 1; n <= 9; n++)
        if (M8(NA(n, 0x28)) != 0) {
            MW32(0xF09E4u, 1);
            break;
        }
    MW8(0xEE168u, 3);
    MW8(0x108804u, 1);
    MW16(0xF09E0u, 1);
}

/* the model of a human's turn set-up (0x55934, 0x55D25..0x55F87); returns the start territory */
static int m_turn(int n)
{
    int r, k, t, pts;
    MW32(0xF40F8u, n);
    MW32(0x109E18u, 0);
    MW32(0x109E1Cu, -1);
    for (r = 1; r <= 160; r++) {
        uint32_t a = AREA_BASE + 69u * (uint32_t)r + (uint32_t)n;
        MW8(a + 0x24u, M8(a + 1u) != 0 ? M8(a + 0x0Eu) : 0);
    }
    if (M8(NA(n, 0x40)) == 0)
        MW8(0x108804u, 1);
    MW32(0x1087DCu + 4u * (uint32_t)n, -1);
    pts = M8(NA(n, 0x25)) != 0 ? 5 : 4;
    for (k = 0; k < 4; k++)
        MW8(NA(n, 0x41 + k), pts);
    t = MS32(0xC4990u + 4u * (uint32_t)n);
    if (t == -1 || MS8(AR(t, 0x0E + n)) == 0)
        t = m577B0(n);
    MW32(0x109E10u, t);
    m_f576F8(t);
    MW16(0xF09E0u, 1);   /* f572CC(t) */
    MW32(0xF06CCu, 0);
    return t;
}

/* the model of choosing a land source t (0x56357..0x56433) */
static void m_choose_land(int n, int t)
{
    MW32(0x109E10u, t);
    if (MS32(0x1087DCu + 4u * (uint32_t)n) == -1) {
        MW32(0x1087DCu + 4u * (uint32_t)n, 0);
        MW32(0xC4990u + 4u * (uint32_t)n, t);
    }
    MW32(0x10881Cu, 0);
    m_f576F8(t);
    MW16(0xF09E0u, 1);   /* f572CC(src) */
    MW32(0xF06CCu, 1);
}

/* the model of one land step src -> t of k tokens (0x564C3..0x5694A); returns the new source */
static int m_land_step(int n, int src, int t, int k)
{
    int steps = MS32(0x10881Cu) + 1, smax = (M8(NA(n, 0x28)) != 0) + 1;
    MW32(0x10881Cu, steps);
    m_f57CE8(src, n, steps == 1, k);
    m_f57E28(t, n, k);
    MW32(0x108810u, 1);
    MW32(0x108808u, src);
    MW32(0x10880Cu, t);
    MW32(0x108814u, steps == 1);
    MW32(0x108818u, k);
    if (steps < smax && MS8(AR(t, 1)) == 1
        && ((M8(TS_BASE + 3u * (uint32_t)t + 2u) & 1) == 0 || M8(TS_BASE + 3u * (uint32_t)t) == n)) {
        src = t;   /* Roadbuilding: the stack goes on */
    } else {
        if (MS8(AR(t, 1)) != 1 || ((M8(TS_BASE + 3u * (uint32_t)t + 2u) & 1) == 1 && M8(TS_BASE + 3u * (uint32_t)t) != n))
            MW32(0xF06CCu, 0);
        MW16(0xF09E0u, 1);   /* f572CC(src) */
    }
    if (steps == smax)
        MW32(0xF06CCu, 0);
    return src;
}

static void m_end(void)
{
    MW8(0x108804u, 0);
    MW32(0xEED74u, 0);
    MW32(0xEED78u, 0);
}

static int nation_index(int n) { int i; for (i = 0; i < g_norder; i++) if (g_order[i] == n) return i; return -1; }

/* the model of the whole phase: the set-up, each nation's turn in order (Africa's by afr when given), the end */
static void m_phase(void (*afr)(void))
{
    int i;
    m_setup();
    for (i = 0; i < g_norder; i++)
        if (g_order[i] == 1 && afr != NULL)
            afr();
        else
            (void)m_turn(g_order[i]);
    m_end();
}

/* the clicks: FINISHED for every nation before and after Africa (index ia), Africa's own clicks in the middle */
static void script_around(int ia, const int *mine, int nmine)
{
    int c[256], k = 0, i;
    for (i = 0; i < ia && i < g_norder; i++)
        c[k++] = BTN(0x2A);
    for (i = 0; i < nmine; i++)
        c[k++] = mine[i];
    for (i = ia + 1; i < g_norder; i++)
        c[k++] = BTN(0x2A);
    script(c, k);
}

static int  g_first;
static void afr_move(void)      /* TUNISIA, count 2, THAPSUS */
{
    (void)m_turn(1);
    m_choose_land(1, 100);
    MW16(0xF09E0u, 2);
    (void)m_land_step(1, 100, 101, 2);
}
static void afr_undo(void)      /* the same and UNDO MOVE (move.c's case, on the model) */
{
    afr_move();
    on_model();
    (void)dcase_28(-100);
    off_model();
}
static void afr_restart(void)   /* TUNISIA, THAPSUS, RESTART: records back from the turn's snapshot, the turn again */
{
    static uint8_t area[161 * 69], nat[199];
    memcpy(area, m_ds + (AREA_BASE - DS_BASE), sizeof area);
    memcpy(nat, m_ds + (NA(1, 0) - DS_BASE), sizeof nat);
    (void)m_turn(1);
    m_choose_land(1, 100);
    (void)m_land_step(1, 100, 101, 1);
    memcpy(m_ds + (AREA_BASE - DS_BASE), area, sizeof area);
    memcpy(m_ds + (NA(1, 0) - DS_BASE), nat, sizeof nat);
    (void)m_turn(1);
}
static void afr_find(void)      /* FIND UNMOVED: the first territory with movable tokens chosen as the source */
{
    (void)m_turn(1);
    g_first = m57810(1);
    m_choose_land(1, g_first);
}
static void afr_ship(void)      /* MOVE SHIPS, SABRATA, EMBARK, DISEMBARK, THAPSUS, UNDO MOVE */
{
    int k;
    (void)m_turn(1);
    on_model();
    (void)dcase_27(-100);
    off_model();
    MW32(0x109E10u, 102);
    m_f576F8(102);
    MW32(0xF06CCu, 1);
    on_model();
    (void)dcase_2B(-100);
    (void)dcase_2C(-100);
    off_model();
    m_f580CC(102, 101, 1);
    MW32(0x108810u, 2);
    MW32(0x108808u, 102);
    MW32(0x10880Cu, 101);
    k = M8(AR(101, 0x2D + 1));
    MW8(NA(1, 0x41 + k), M8(NA(1, 0x41 + k)) - 1);
    m_f576F8(102);
    MW32(0x109E10u, 101);
    if (M8(NA(1, 0x41 + M8(AR(101, 0x2D + 1)))) == 0)
        MW32(0xF06CCu, 0);
    on_model();
    (void)dcase_28(-100);
    off_model();
}

static void poke_ship(void)     /* one ship of Africa at SABRATA (as the DOSBox-X run of OVERVIEW §5) */
{
    setD8(NA(1, 0x40), 1);
    setD16(NA(1, 0x45), 103);
    setD8(NA(1, 0x4D), 0);
    setD8(AR(102, 0x1B + 1), (uint8_t)(D8(AR(102, 0x1B + 1)) + 1u));
    setD8(AR(102, 0x2D + 1), 0);
}

static void part_c(const char *gamedir, const char *adjdir)
{
    char path[400];
    int ia, n, i, t;

    snprintf(path, sizeof path, "%s/savgame0.dat", gamedir);
    if (load_save(path) < 0) {
        printf("C. %s not found: skipped\n", path);
        return;
    }
    if (load_adj(adjdir) < 0) {
        printf("C. adjacency files in %s not found: skipped\n", adjdir);
        return;
    }
    g_save0 = malloc(DS_SIZE);
    memcpy(g_save0, g_ds, DS_SIZE);

    /* C1: everyone FINISHED */
    base(NULL);
    model_sync();
    m_phase(NULL);
    printf("   movement order:");
    for (i = 0; i < g_norder; i++)
        printf(" %s%s", DSTR(D32(0xC768Cu + 4u * (uint32_t)g_order[i])), D8(NA(g_order[i], 0x35)) ? "(Military)" : "");
    printf("\n");
    script_around(g_norder, NULL, 0);
    g_trace[0] = 0;
    f55574();
    same_ds("C1 everyone FINISHED: the data object");
    check(g_iclick == g_nclicks, "C1 every click used");
    check(strstr(g_trace, "PUSH") == NULL && strstr(g_trace, "AI") == NULL, "C1 no computer's turn");

    ia = nation_index(1);
    if (ia < 0) {
        printf("C. Africa not in the movement order: the rest skipped\n");
        return;
    }
    printf("   Africa (position %d): tokens Tunisia %d, Thapsus %d, Sabrata %d; Thapsus in Tunisia's land list %d, in "
           "Sabrata's coast list %d; Roadbuilding %d, Clothmaking %d, Astronomy %d; ships %d; start 0xC4990[1] = %d\n",
           ia, S8(AR(100, 0x0F)), S8(AR(101, 0x0F)), S8(AR(102, 0x0F)), in_list(0, 100, 101), in_list(1, 102, 101),
           D8(NA(1, 0x28)), D8(NA(1, 0x25)), D8(NA(1, 0x2B)), D8(NA(1, 0x40)), S32(0xC4990u + 4u));

    /* C2: TUNISIA, count 2, THAPSUS */
    {
        const int mine[] = {100, BTN(0xC5), 101, BTN(0x2A)};
        base(NULL);
        model_sync();
        m_phase(afr_move);
        script_around(ia, mine, 4);
        g_trace[0] = 0;
        f55574();
        same_ds("C2 Africa: TUNISIA, count 2, THAPSUS");
        check(S8(AR(100, 0x0F)) == (int8_t)g_save0[AR(100, 0x0F) - DS_BASE] - 2
              && S8(AR(101, 0x0F)) == (int8_t)g_save0[AR(101, 0x0F) - DS_BASE] + 2, "C2 two tokens from Tunisia to Thapsus");
        check(S32(0xC4990u + 4u) == 100, "C2 Tunisia is Africa's next start territory");
        check(g_iclick == g_nclicks, "C2 every click used");
    }

    /* C3: the same, then UNDO MOVE, then FINISHED */
    {
        const int mine[] = {100, BTN(0xC5), 101, BTN(0x28), BTN(0x2A)};
        base(NULL);
        model_sync();
        m_phase(afr_undo);
        script_around(ia, mine, 5);
        g_trace[0] = 0;
        f55574();
        same_ds("C3 Africa: the move and UNDO MOVE");
        for (t = 0; t < 157; t++)
            if (S8(AR(t, 0x0F)) != (int8_t)g_save0[AR(t, 0x0F) - DS_BASE])
                break;
        check(t == 157, "C3 Africa's tokens as in the save after UNDO");
    }

    /* C4: RESTART after a move (Restart Confirmations off), then FINISHED */
    {
        const int mine[] = {100, 101, BTN(0x41), BTN(0x2A)};
        base(NULL);
        model_sync();
        m_phase(afr_restart);
        script_around(ia, mine, 4);
        g_trace[0] = 0;
        f55574();
        same_ds("C4 Africa: a move, RESTART, FINISHED");
        check(strstr(g_trace, "YESNO") == NULL, "C4 no question with the toggle off");
    }

    /* C5: FIND UNMOVED chooses the first territory with movable tokens; then FINISHED */
    {
        const int mine[] = {BTN(0x29), BTN(0x2A)};
        base(NULL);
        model_sync();
        m_phase(afr_find);
        script_around(ia, mine, 2);
        g_trace[0] = 0;
        f55574();
        same_ds("C5 Africa: FIND UNMOVED");
        printf("   FIND UNMOVED chose %s (%d)\n", DSTR(D32(0xC77D8u + 4u * (uint32_t)g_first)), g_first);
    }

    /* C6: the ship: MOVE SHIPS, SABRATA, EMBARK, DISEMBARK, THAPSUS, UNDO MOVE, FINISHED */
    {
        const int mine[] = {BTN(0x27), 102, BTN(0x2B), BTN(0x2C), 101, BTN(0x28), BTN(0x2A)};
        base(NULL);
        poke_ship();
        model_sync();
        m_phase(afr_ship);
        memset(g_pressed, 0, sizeof g_pressed);
        script_around(ia, mine, 7);
        g_trace[0] = 0;
        f55574();
        check(in_list(1, 102, 101), "C6 Thapsus is in Sabrata's coast list");
        same_ds("C6 Africa's ship: MOVE SHIPS, SABRATA, EMBARK, DISEMBARK, THAPSUS, UNDO MOVE");
        check(S16(NA(1, 0x45)) == 103 && D8(NA(1, 0x4D)) == 0, "C6 the ship back at Sabrata, nothing aboard");
        check(strstr(g_trace, "MSG[You may not move your ship") == NULL && strstr(g_trace, "MSG[There are still") == NULL,
              "C6 the sailing allowed, FINISHED accepted");
    }

    /* C7: FINISHED refused while tokens are aboard; DISEMBARK, then FINISHED */
    {
        const int mine[] = {BTN(0x27), 102, BTN(0x2B), BTN(0x2A), BTN(0x2C), BTN(0x2A)};
        base(NULL);
        poke_ship();
        script_around(ia, mine, 6);
        g_trace[0] = 0;
        f55574();
        check(strstr(g_trace, "MSG[There are still tokens left on your ships. You must disembark them before movement can be ended.]") != NULL
              && g_iclick == g_nclicks, "C7 FINISHED refused with tokens aboard");
    }

    /* C8: BUG(1995) 1: Africa with no token on the board, both modes */
    for (i = 0; i < 2; i++) {
        uint8_t before;
        base(NULL);
        for (t = 0; t < 160; t++)
            if (D8(AR(t, 2)) != 0) {
                setD8(AR(t, 2), 0);
                setD8(AR(t, 0x0F), 0);
                setD8(AR(t, 1), (uint8_t)(D8(AR(t, 1)) - 1u));
            }
        before = D8(0xFFB5Fu);
        g_original = i;
        script_around(ia, NULL, 0);
        g_trace[0] = 0;
        f55574();
        g_original = 0;
        if (i == 0)
            check(D8(0xFFB5Fu) == before && strstr(g_trace, "787F8(-1)") == NULL,
                  "C8 BUG(1995) 1 fixed: no write into 0xFFB5F, no \"PIRACY\" label");
        else
            check(D8(0xFFB5Fu) == 15 && strstr(g_trace, "787F8(-1)") != NULL,
                  "C8 BUG(1995) 1 kept with --original: 15 in 0xFFB5F, f787F8(-1)");
        check(g_iclick == g_nclicks, "C8 Africa's turn skipped without a click");
    }

    /* C9: nothing left to move: Africa has one token (in Tunisia) and moves it; the message; RESTART answered NO;
       FINISHED without the confirmation. Both confirmations on, the others answer YES */
    {
        int c[64], k = 0;
        base(NULL);
        setD8(0xF097Cu, (uint8_t)(D8(0xF097Cu) | 0x18u));
        for (t = 0; t < 160; t++)
            if (D8(AR(t, 2)) != 0) {
                setD8(AR(t, 2), 0);
                setD8(AR(t, 0x0F), 0);
                setD8(AR(t, 1), (uint8_t)(D8(AR(t, 1)) - 1u));
            }
        setD8(AR(100, 2), 1);
        setD8(AR(100, 0x0F), 1);
        setD8(AR(100, 1), (uint8_t)(D8(AR(100, 1)) + 1u));
        for (i = 0; i < ia; i++)
            c[k++] = BTN(0x2A);
        c[k++] = 100;
        c[k++] = 101;
        c[k++] = BTN(0x41);
        c[k++] = BTN(0x2A);
        for (i = ia + 1; i < g_norder; i++)
            c[k++] = BTN(0x2A);
        script(c, k);
        g_nyesno = 0;
        for (i = 0; i < ia; i++)
            g_yesno[g_nyesno++] = -0x61;
        g_yesno[g_nyesno++] = -0x62;
        for (i = ia + 1; i < g_norder; i++)
            g_yesno[g_nyesno++] = -0x61;
        g_iyesno = 0;
        g_trace[0] = 0;
        f55574();
        check(strstr(g_trace, "MSG[You have no more tokens (or ships) to move.  Press FINISHED.]") != NULL
              && strstr(g_trace, "YESNO[Restart African Movement?]=-98") != NULL
              && strstr(g_trace, "Finished movement for AFRICA") == NULL && g_iclick == g_nclicks
              && S8(AR(101, 0x0F)) == (int8_t)g_save0[AR(101, 0x0F) - DS_BASE] + 1,
              "C9 nothing left: the message, RESTART refused, FINISHED without the confirmation");
        setD8(0xF097Cu, (uint8_t)(D8(0xF097Cu) & ~0x18u));
    }

    /* C11: Roadbuilding. a) TUNISIA, THAPSUS (empty: the stack goes on), then a land neighbour of THAPSUS (the second
       step, f57CE8 without the movable flag). b) THAPSUS held by another nation too: the stack stops after one step
       ([0xF06CC] = 0; Africa moves last, so the byte is still Africa's at the end) */
    {
        int x = -1, j;
        for (j = 1; j <= g_adj[0][101][0]; j++)
            if (g_adj[0][101][j] != 100 && g_adj[0][101][j] < 157) {
                x = g_adj[0][101][j];
                break;
            }
        if (x < 0 || ia != g_norder - 1) {
            printf("C11 skipped (no neighbour of Thapsus, or Africa not last)\n");
        } else {
            const int mine_a[] = {100, 101, x, BTN(0x2A)};
            const int mine_b[] = {100, 101, BTN(0x2A)};
            int src;
            base(NULL);
            setD8(NA(1, 0x28), 1);
            model_sync();
            m_setup();
            for (i = 0; i < g_norder; i++)
                if (g_order[i] != 1)
                    (void)m_turn(g_order[i]);
            (void)m_turn(1);
            m_choose_land(1, 100);
            src = m_land_step(1, 100, 101, 1);
            check(src == 101, "C11a the model chains (Thapsus empty, no city)");
            (void)m_land_step(1, src, x, 1);
            m_end();
            script_around(ia, mine_a, 4);
            g_trace[0] = 0;
            f55574();
            same_ds("C11a Roadbuilding: TUNISIA, THAPSUS, and on to its neighbour");
            check(g_iclick == g_nclicks, "C11a every click used");

            base(NULL);
            setD8(NA(1, 0x28), 1);
            setD8(AR(101, 1 + 3), 3);   /* Illyria's marker and a token at Thapsus */
            setD8(AR(101, 0x0E + 3), 1);
            setD8(AR(101, 1), (uint8_t)(D8(AR(101, 1)) + 1u));
            model_sync();
            m_setup();
            for (i = 0; i < g_norder; i++)
                if (g_order[i] != 1)
                    (void)m_turn(g_order[i]);
            (void)m_turn(1);
            m_choose_land(1, 100);
            (void)m_land_step(1, 100, 101, 1);
            m_end();
            script_around(ia, mine_b, 3);
            g_trace[0] = 0;
            f55574();
            same_ds("C11b Roadbuilding: a shared THAPSUS stops the stack");
            check(S32(0xF06CCu) == 0, "C11b no source left chosen");
        }
    }

    /* C10: Iberia a computer: REPLAY, then CONTINUE */
    {
        int kinds[10] = {0};
        const char *p1, *p2;
        kinds[2] = 2;
        base(kinds);
        setD32(0xCAC5Cu, 0x12345678u);
        n = nation_index(2);
        {
            int c[64], k = 0;
            for (i = 0; i < g_norder; i++)
                if (i != n)
                    c[k++] = BTN(0x2A);
            script(c, k);
        }
        g_ba4[0] = -100; g_ba4[1] = -0x5E; g_ba4[2] = -1; g_nba4 = 3; g_iba4 = 0;
        g_unattended = 0;
        g_trace[0] = 0;
        f55574();
        p1 = strstr(g_trace, "AI353C4(2,");
        p2 = p1 ? strstr(p1 + 1, "AI353C4(2,") : NULL;
        check(p1 != NULL && p2 != NULL && strncmp(p1, p2, 24) == 0 && strstr(p2 + 1, "AI353C4") == NULL,
              "C10 REPLAY plans again from the turn's seed");
        check(strstr(g_trace, "79270(-550,2,-200,[IBERIA considers movement.],0) 3C828(F596C) PUSH AI353C4(2,") != NULL
              && strstr(g_trace, "POP AI461BC(2) 3C7A0(F596C) 79270(-550,2,-200,[Movement for IBERIA complete.],0) 7853C(C59EC)") != NULL
              && strstr(g_trace, "78BA4=-100 78BA4=-94 78664(C59EC) POP 3A990") != NULL
              && strstr(g_trace, "78BA4=-1 78664(C59EC) DROP 3A990") != NULL,
              "C10 the computer's turn: World State push / pop / drop, REPLAY, CONTINUE");
        check(g_iclick == g_nclicks, "C10 the humans' clicks used");
        /* unattended: no click wait, CONTINUE after 250 ms */
        base(kinds);
        script(g_clicks, g_nclicks);
        g_unattended = 1;
        g_trace[0] = 0;
        f55574();
        g_unattended = 0;
        check(strstr(g_trace, "W250 78664(C59EC) DROP") != NULL, "C10 unattended: 250 ms, then CONTINUE");
    }
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:/work/advciv_run";
    const char *adjdir = argc > 2 ? argv[2] : "C:/work/advciv_play";
    int f0;

    setvbuf(stdout, NULL, _IONBF, 0);
    if (state_init(gamedir) != 0)
        return 2;
    m_ds = malloc(DS_SIZE);
    g_before = malloc(DS_SIZE);
    if (m_ds == NULL || g_before == NULL)
        return 2;
    memcpy(g_before, g_ds, DS_SIZE);

    f0 = g_fail;
    part_a(20000);
    printf("A. helpers: %d checks, %d failures\n", g_tests, g_fail - f0);
    f0 = g_fail;
    {
        int t0 = g_tests;
        part_b(5000);
        printf("B. playback: %d checks, %d failures\n", g_tests - t0, g_fail - f0);
    }
    f0 = g_fail;
    {
        int t0 = g_tests;
        part_c(gamedir, adjdir);
        printf("C. the phase: %d checks, %d failures\n", g_tests - t0, g_fail - f0);
    }
    printf("%d checks, %d failures\n", g_tests, g_fail);
    return g_fail == 0 ? 0 : 1;
}
