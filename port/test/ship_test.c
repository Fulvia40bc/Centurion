/* ship_test [GAMEDIR [SAVEDIR]]: unit tests for module SHIP (ship.c) that need no game run.

   It loads the data object from GAMEDIR/AC_MAIN.EXE as the port does (state_init; GAMEDIR defaults to the run copy
   C:\work\advciv_run, only read). ship.c is compiled with -DSHIP_TEST, so its three calls into the computers' code
   (0x61254, 0x62840, 0x461BC) go to recorders here, which return. The UI routines are stubs that record their calls
   (name and arguments, in order) and, for f78BA4 / f795A8 / f7BA64, return scripted values. The primitives ship.c
   calls (move.c's f57CE8 / f581C4 / f576F8, rules.c's f78D98, mapdata.c's highlight setters and f3DA60) are the real,
   separately tested ones.
     A. random differential runs of the pure helpers against a second implementation written here from the listing,
        not from ship.c, on a plain copy of the data object (m_ds): f60AE0, f62658, f62774, f62488, f6106C, f7A598,
        f7A610. The model reuses the verified primitives by running them on its copy (g_ds swapped). After every
        operation the whole data object must equal the model's.
     B. scripted phase scenarios for f60BD8 and f61940 (clicks, Restart with and without the confirmation, the
        computers' option table, the extra ships, the housekeeping), checked byte for byte against the model's
        expectation of each step, and the call traces against the order of the listing; each 1995 fix in both modes.
     C. the saves: SAVEDIR/SAVGAMEA.DAT (the R4 autosave, start of SHIP CONSTRUCTION) read with saveload.c's f5F05C;
        the ship order, each nation's resources and candidate sites are printed (for the DOSBox-X check plan), and
        savgame0.dat (GAMEDIR, the original's own autosave after phase 4) is compared with it nation by nation.
   Prints each failure, the counts per part, and exits 0 when all pass.

   Build (from the repo root):
     clang -std=c11 -Wall -Wextra -DSHIP_TEST -Iport/src port/test/ship_test.c port/src/ship.c port/src/move.c
       port/src/rules.c port/src/mapdata.c port/src/load.c port/src/saveload.c port/src/state.c port/src/le_image.c
       -o ship_test.exe */

#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <direct.h>
#define chdir _chdir
#define getcwd _getcwd

#include "../src/state.h"
#include "../src/game.h"
#include "../src/fg.h"
#include "../src/ui.h"
#include "../src/kit.h"
#include "../src/map.h"
#include "../src/mapdata.h"
#include "../src/move.h"
#include "../src/panel.h"
#include "../src/rules.h"
#include "../src/saveload.h"
#include "../src/ship.h"
#include "../src/stack.h"
#include "../src/start.h"

/* ---- trace and stubs --------------------------------------------------------------------------------------------- */

static char g_trace[65536];
static int g_tests, g_fail;

static void tr(const char *fmt, ...)
{
    size_t len = strlen(g_trace);
    va_list ap;
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

/* scripted answers */
static int g_codes[64], g_ncodes, g_icode;   /* f78BA4 */
static int g_yesno[8], g_nyesno, g_iyesno;   /* f795A8 */
static int g_r62840;                         /* ship_test_f62840 */

static void script(const int *codes, int n)
{
    memcpy(g_codes, codes, sizeof(int) * (size_t)n);
    g_ncodes = n;
    g_icode = 0;
}

int fg_vbopen(int handle) { tr("vbopen(%X) ", (unsigned)handle); return 0; }
void fg_vbclose(void) { tr("vbclose "); }
int fg_vbcopy(int a, int b, int c, int d, int e, int f, int s, int t)
{
    tr("vbcopy(%d,%d,%d,%d,%d,%d,%X,%X) ", a, b, c, d, e, f, (unsigned)s, (unsigned)t);
    return 0;
}
int fg_setcolor(int c) { tr("setcolor(%d) ", c); return 0; }
void f3CD6C(int8_t pic) { tr("3CD6C(%d) ", pic); }
void f3C3D0(uint32_t dlg, int id) { tr("off(%X,%X) ", (unsigned)dlg, id); }
void f3C314(uint32_t dlg, int id) { tr("on(%X,%X) ", (unsigned)dlg, id); }
void f459E4(int n) { tr("459E4(%d) ", n); }
void f3A924(void) { tr("3A924 "); }
void f3AB7C(const char *text) { tr("3AB7C[%s] ", text); }
void f3A9F4(const char *text) { tr("3A9F4[%s] ", text); }
void f7844C(uint32_t dlg, int base) { tr("7844C(%X,%d) ", (unsigned)dlg, base); }
void f3CBBC(void) { tr("3CBBC "); }
void f78664(uint32_t dlg) { tr("78664(%X) ", (unsigned)dlg); }
void f7853C(uint32_t dlg) { tr("7853C(%X) ", (unsigned)dlg); }
void f7876C(void) { tr("7876C "); }
void f3B05C(uint32_t dlg) { tr("3B05C(%X) ", (unsigned)dlg); }
void f79270(int wait, int n, int t, const char *text, int anim) { tr("79270(%d,%d,%d,[%s],%d) ", wait, n, t, text, anim); }
void f7A8D0(void) { tr("7A8D0 "); }
void f7A940(void) { tr("7A940 "); }
void f7ABD0(void) { tr("7ABD0 "); }
void f7AC60(void) { tr("7AC60 "); }
void f78A20(int t) { tr("78A20(%d) ", t); }
static int g_unattended;
int f7BA64(void) { tr("7BA64 "); return g_unattended; }
void f3A990(void) { tr("3A990 "); }
void f3AF8C(uint32_t spr, int x, int y) { tr("3AF8C(%X,%d,%d) ", (unsigned)spr, x, y); }
void f3C9D0(int x, int y, int w, int h, int flags) { tr("3C9D0(%d,%d,%d,%d,%d) ", x, y, w, h, flags); }
void f3A0B0(int x, int y, int w, const char *s) { tr("3A0B0(%d,%d,%d,[%s]) ", x, y, w, s); }
void f783B8(uint32_t dlg, int base) { tr("783B8(%X,%d) ", (unsigned)dlg, base); }
void f3D52C(int dx, int dy) { tr("3D52C(%d,%d) ", dx, dy); }
void f787F8(int t) { tr("787F8(%d) ", t); }
int f78BA4(uint32_t dlg, int map)
{
    int code;
    if (g_icode >= g_ncodes) {
        printf("FAIL f78BA4 called beyond the script\n");
        g_fail++;
        code = -1;
    } else
        code = g_codes[g_icode++];
    tr("78BA4(%X,%d)=%d ", (unsigned)dlg, map, code);
    return code;
}
int f795A8(const char *text)
{
    int r = g_iyesno < g_nyesno ? g_yesno[g_iyesno++] : -0x62;
    tr("795A8[%s]=%d ", text, r);
    return r;
}
/* 0x79A9C written here from the listing (game.h's description), not copied from ui_kit2.c */
void f79A9C(int32_t *p)
{
    int32_t c = S32(0xF7C9Cu);
    if (c <= *p)
        *p = c - 1;
    if (*p < 0)
        *p = 0;
    tr("79A9C=%d ", (int)*p);
}

/* the computers' code (SHIP_TEST hooks) */
void ship_test_f61254(int n) { tr("AI61254(%d) ", n); }
int ship_test_f62840(int n) { tr("AI62840(%d)=%d ", n, g_r62840); return g_r62840; }
void ship_test_f461BC(int n) { tr("AI461BC(%d) ", n); }

/* reached by nothing here; their callers' files need them */
static jmp_buf g_np_jmp;
static int g_np_armed;
_Noreturn void not_ported(uint32_t addr)
{
    tr("NOTPORTED(%X) ", (unsigned)addr);
    if (g_np_armed)
        longjmp(g_np_jmp, 1);
    printf("FAIL not_ported(0x%X)\n", (unsigned)addr);
    exit(2);
}
void (*g_not_ported_hook)(uint32_t addr);
int f3B9A4(uint32_t dlg, int id) { (void)dlg; (void)id; tr("UNEXPECTED 3B9A4 "); return 0; }
void f3B7CC(uint32_t dlg, int id) { (void)dlg; (void)id; tr("UNEXPECTED 3B7CC "); }
void f3B870(uint32_t dlg, int id) { (void)dlg; (void)id; tr("UNEXPECTED 3B870 "); }
void f3BFBC(uint32_t dlg, int id, int en) { (void)dlg; (void)id; (void)en; tr("UNEXPECTED 3BFBC "); }
void f426D0(int i) { (void)i; tr("UNEXPECTED 426D0 "); }
void f572CC(int t) { (void)t; tr("UNEXPECTED 572CC "); }
void f3A780(const char *msg) { tr("UNEXPECTED 3A780[%s] ", msg); }
_Noreturn void f44CAC(void) { printf("FAIL quit: %s\n", DSTR(0xF5500u)); exit(2); }
int f78AE4(void) { tr("UNEXPECTED rand "); return 0; }
void f76F70(int n, uint8_t *cards) { (void)n; (void)cards; }

/* ---- addresses (from the listing's constants, written out here) ------------------------------------------------- */

#define AREA_BASE 0xE31B8u
#define AREA_LEN  (161u * 69u)
#define NAT_BASE  0xF14CCu
#define NAT_LEN   (14u * 199u)
#define TS_BASE   0xE2C18u
#define TS_LEN    (160u * 3u)
#define AR(t, off) (AREA_BASE + 69u * (uint32_t)((t) + 1) + (uint32_t)(off))
#define NA(n, off) (NAT_BASE + 199u * (uint32_t)(n) + (uint32_t)(off))
#define TR(t)      (0xE1638u + 35u * (uint32_t)(t))

/* ---- the model: a second implementation on m_ds ---------------------------------------------------------------- */

static uint8_t *m_ds, *g_save, *g_real;

static uint8_t  M8(uint32_t a) { return m_ds[a - DS_BASE]; }
static int      MS8(uint32_t a) { return (int8_t)m_ds[a - DS_BASE]; }
static int      MS16(uint32_t a) { return (int16_t)(m_ds[a - DS_BASE] | m_ds[a - DS_BASE + 1] << 8); }
static int32_t  MS32(uint32_t a) { const uint8_t *p = m_ds + (a - DS_BASE); return (int32_t)((uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24); }
static void     MW8(uint32_t a, int v) { m_ds[a - DS_BASE] = (uint8_t)v; }
static void     MW16(uint32_t a, int v) { m_ds[a - DS_BASE] = (uint8_t)v; m_ds[a - DS_BASE + 1] = (uint8_t)(v >> 8); }
static void     MW32(uint32_t a, int32_t v) { uint8_t *p = m_ds + (a - DS_BASE); uint32_t u = (uint32_t)v; p[0] = (uint8_t)u; p[1] = (uint8_t)(u >> 8); p[2] = (uint8_t)(u >> 16); p[3] = (uint8_t)(u >> 24); }
static int      canon(int t) { return m_ds[0xC803Cu + (uint32_t)t - DS_BASE]; }

/* the verified primitives, run on the model's copy */
static void on_model_begin(void) { g_real = g_ds; g_ds = m_ds; }
static void on_model_end(void) { g_ds = g_real; }
static void m_f581C4(int t, int n) { on_model_begin(); f581C4(t, n); on_model_end(); }
static void m_f57CE8(int t, int n, int k) { on_model_begin(); f57CE8(t, n, 0, k); on_model_end(); }
static void m_f78D98(int k, int n) { on_model_begin(); f78D98(k, n); on_model_end(); }
/* mapdata.c's f3DA60 writes the scroll; f3F670 / f3F624 the highlights */
static void m_f3DA60(int t) { on_model_begin(); f3DA60(t); on_model_end(); }

static void m_order(void)
{
    int cnt = 0, pass, i, n;
    for (pass = 0; pass < 2; pass++)
        for (i = 0; i < MS16(0xF0698u); i++) {
            n = MS16(0xF069Au + 2u * (uint32_t)i);
            if ((M8(NA(n, 0x35)) != 0) != (pass == 1))
                continue;
            MW16(0xF06B0u + 2u * (uint32_t)cnt, n);
            cnt++;
        }
    MW16(0xF06AEu, cnt);
}

static void m_sites(int n)
{
    int t, tok, cnt = 0, i;
    for (i = 0; i < 157; i++)
        MW32(0xF7CA0u + 4u * (uint32_t)i, -1);
    MW32(0xF7C9Cu, 0);
    /* 0x626AD dword [rec + 0x95] sar 24 is the byte +0x98 (the tokens on the board), not +0x99 (the stock) */
    if (!(MS8(NA(n, 0x98)) > 2 || MS8(NA(n, 0x51)) > 0))
        return;
    for (t = 0; t < 160; t++) {
        if (!(M8(TR(t)) & 2))
            continue;
        tok = MS8(AR(canon(t), 0x0E + n));
        if (tok >= 2 || (tok >= 1 && MS8(NA(n, 0x3E)) >= 1)) {
            MW32(0xF7CA0u + 4u * (uint32_t)cnt, t);
            cnt++;
        }
    }
    MW32(0xF7C9Cu, cnt);
}

static void m_human_ship(int t, int n, int32_t *sel)
{
    int k = 0, c;
    MW8(AR(t, 0x1B + n), M8(AR(t, 0x1B + n)) + 1);
    while (MS16(NA(n, 0x45 + 2 * k)) != 0)
        k = (k + 1) & 0xFF;
    MW16(NA(n, 0x45 + 2 * k), t + 1);
    MW8(AR(t, 0x2D + n), k);
    MW8(NA(n, 0x40), M8(NA(n, 0x40)) + 1);
    m_f3DA60(t);
    m_sites(n);
    c = MS32(0xF7C9Cu);
    if (c <= *sel)
        *sel = c - 1;
    if (*sel < 0)
        *sel = 0;
}

static void m_computer_ship(int t, int levy, int treasury, int n)
{
    int k = M8(NA(n, 0x40));
    MW16(NA(n, 0x45 + 2 * k), t + 1);
    MW8(NA(n, 0x40), k + 1);
    MW8(NA(n, 0x3E), M8(NA(n, 0x3E)) - treasury);
    MW8(NA(n, 0x99), M8(NA(n, 0x99)) + treasury + levy);
    MW8(NA(n, 0x98), M8(NA(n, 0x98)) - levy);
    if (levy > 0)
        m_f57CE8(t, n, levy);
    MW8(AR(t, 0x1B + n), M8(AR(t, 0x1B + n)) + 1);
    MW8(AR(t, 0x2D + n), k);
}

static void m_maintain(int k, int scrap, int tp1, int useToken, int n)
{
    int tok = useToken != 0, tre = useToken == 0;
    if (scrap == 0) {
        MW8(NA(n, 0x3E), M8(NA(n, 0x3E)) - tre);
        MW8(NA(n, 0x99), M8(NA(n, 0x99)) + tre + tok);
        MW8(NA(n, 0x98), M8(NA(n, 0x98)) - tok);
        m_f57CE8(tp1 - 1, n, tok);
    } else
        m_f78D98(k, n);
}

static void m_snap(int back)
{
    static const struct { uint32_t live, snap, len; } r[3] = {
        { 0xE31FDu, 0x1139B0u, 0x2B20u }, { 0xE2C18u, 0x1137D0u, 0x1E0u }, { 0xF1593u, 0x112CECu, 0x88Du } };
    int i;
    for (i = 0; i < 3; i++)
        if (back)
            memcpy(m_ds + (r[i].live - DS_BASE), m_ds + (r[i].snap - DS_BASE), r[i].len);
        else
            memcpy(m_ds + (r[i].snap - DS_BASE), m_ds + (r[i].live - DS_BASE), r[i].len);
}

/* ---- comparisons ----------------------------------------------------------------------------------------------- */

static int first_diff(uint32_t *where)
{
    uint32_t i;
    for (i = 0; i < DS_SIZE; i++)
        if (g_ds[i] != m_ds[i]) {
            *where = DS_BASE + i;
            return 1;
        }
    return 0;
}

static int same_ds(const char *what)
{
    uint32_t a;
    char buf[200];
    if (!first_diff(&a))
        return 1;
    snprintf(buf, sizeof buf, "%s: data differs first at 0x%X (port %02X, model %02X)", what, (unsigned)a,
             g_ds[a - DS_BASE], m_ds[a - DS_BASE]);
    check(0, buf);
    return 0;
}

static void sync_model(void) { memcpy(m_ds, g_ds, DS_SIZE); }

/* ---- random states ------------------------------------------------------------------------------------------- */

static uint32_t g_rng = 12345u;
static uint32_t rnd(uint32_t n) { g_rng = g_rng * 1103515245u + 12345u; return ((g_rng >> 8) & 0xFFFFFFu) % n; }

static void clear_world(void)
{
    memset(g_ds + (AREA_BASE - DS_BASE), 0, AREA_LEN);
    memset(g_ds + (NAT_BASE - DS_BASE), 0, NAT_LEN);
    memset(g_ds + (TS_BASE - DS_BASE), 0, TS_LEN);
}

static void random_world(void)
{
    int t, n;
    clear_world();
    for (t = 0; t < 160; t++)
        setD8(TR(t), (uint8_t)((D8(TR(t)) & ~2u) | (rnd(3) == 0 ? 2u : 0u)));
    for (t = 0; t < 157; t++)
        for (n = 1; n <= 9; n++)
            if (rnd(4) == 0) {
                setD8(AR(t, 0x0E + n), (uint8_t)(rnd(8) - 1));   /* -1..6 */
                if (D8(AR(t, 0x0E + n)) != 0) {
                    setD8(AR(t, 1 + n), (uint8_t)n);
                    setD8(AR(t, 1), (uint8_t)(D8(AR(t, 1)) + 1));
                    if (S8(NA(n, 0x60)) < 40) {
                        setD8(NA(n, 0x61) + D8(NA(n, 0x60)), (uint8_t)(t + 1));
                        setD8(NA(n, 0x60), (uint8_t)(D8(NA(n, 0x60)) + 1));
                    }
                }
            }
    for (n = 1; n <= 9; n++) {
        setD8(NA(n, 0x3E), (uint8_t)(rnd(10) - 2));
        setD8(NA(n, 0x99), (uint8_t)(rnd(12) - 2));
        setD8(NA(n, 0x98), (uint8_t)rnd(30));
        setD8(NA(n, 0x51), (uint8_t)(rnd(4) - 1));
        setD8(NA(n, 0x35), (uint8_t)(rnd(3) == 0));
    }
}

/* ---- A. helpers against the model --------------------------------------------------------------------------- */

static void part_a(int ops)
{
    int op, i, n, t, k, ok = 1;
    int32_t sel, msel;
    char what[120];

    for (op = 0; op < ops && ok; op++) {
        random_world();
        n = 1 + (int)rnd(9);
        g_trace[0] = 0;
        switch (op % 6) {
        case 0: {   /* f60AE0 */
            int cnt = (int)rnd(10);
            setD16(0xF0698u, (uint16_t)cnt);
            for (i = 0; i < cnt; i++)
                setD16(0xF069Au + 2u * (uint32_t)i, (uint16_t)(1 + rnd(9)));
            setD32(0xF5940u, rnd(40));
            sync_model();
            f60AE0();
            m_order();
            snprintf(what, sizeof what, "A f60AE0 op %d", op);
            ok = same_ds(what);
            snprintf(what, sizeof what, "783B8(F596C,%d) ", (int)D32(0xF5940u));
            check(strcmp(g_trace, what) == 0, "A f60AE0 trace");
            break;
        }
        case 1:     /* f62658 */
            sync_model();
            f62658(n);
            m_sites(n);
            snprintf(what, sizeof what, "A f62658 op %d", op);
            ok = same_ds(what);
            break;
        case 2:     /* f62774 */
            setD8(NA(n, 0x98), (uint8_t)(3 + rnd(5)));   /* board > 2: f62658 lists sites */
            k = (int)rnd(4);
            for (i = 0; i < 4; i++)
                setD16(NA(n, 0x45 + 2 * i), (uint16_t)(i < k ? 1 + rnd(160) : 0));
            if (k > 0 && rnd(3) == 0)   /* a hole: a slot freed before the last */
                setD16(NA(n, 0x45 + 2 * (int)rnd((uint32_t)k)), 0);
            setD8(NA(n, 0x40), (uint8_t)k);
            t = (int)rnd(160);
            sel = (int32_t)rnd(12) - 1;
            msel = sel;
            sync_model();
            f62774(t, n, &sel);
            m_human_ship(t, n, &msel);
            snprintf(what, sizeof what, "A f62774 op %d", op);
            ok = same_ds(what);
            check(sel == msel, "A f62774 sel");
            break;
        case 3: {   /* f62488 */
            int levy = (int)rnd(3), tre = (int)rnd(3);
            k = (int)rnd(4);
            setD8(NA(n, 0x40), (uint8_t)k);
            t = (int)rnd(157);
            if (levy > 0) {   /* tokens there to pay with */
                setD8(AR(t, 0x0E + n), (uint8_t)(levy + rnd(3)));
                if (D8(AR(t, 1 + n)) == 0) {
                    setD8(AR(t, 1 + n), (uint8_t)n);
                    setD8(AR(t, 1), (uint8_t)(D8(AR(t, 1)) + 1));
                    setD8(NA(n, 0x61) + D8(NA(n, 0x60)), (uint8_t)(t + 1));
                    setD8(NA(n, 0x60), (uint8_t)(D8(NA(n, 0x60)) + 1));
                }
            }
            setD8(0xF097Fu, (uint8_t)(rnd(2) ? 0x80 : 0));
            g_unattended = (int)rnd(2);
            setD16(0xF40BAu, (uint16_t)rnd(2));
            sync_model();
            f62488(t, levy, tre, n);
            m_computer_ship(t, levy, tre, n);
            snprintf(what, sizeof what, "A f62488 op %d", op);
            ok = same_ds(what);
            {   /* the message and the second display */
                char exp[600], msg[200];
                const char *how = tre && levy ? "Treasury and Levy" : tre ? "Treasury" : "Levy";
                int w = (D8(0xF097Fu) & 0x80) ? -500 : !g_unattended ? 1000 : D16(0xF40BAu) ? 1000 : 500;
                snprintf(msg, sizeof msg, "%s constructed a ship in %s using %s.",
                         DSTR(D32(0xC768Cu + 4u * (uint32_t)n)), DSTR(D32(0xC77D8u + 4u * (uint32_t)t)), how);
                snprintf(exp, sizeof exp, "79270(-550,%d,%d,[%s],0) 78A20(500) %s79270(%d,%d,%d,[%s],0) ", n, t, msg,
                         (D8(0xF097Fu) & 0x80) ? "" : "3CBBC 7BA64 ", w, n, t, msg);
                check(strcmp(g_trace, exp) == 0, "A f62488 trace");
                if (strcmp(g_trace, exp) != 0)
                    printf("  got  %s\n  want %s\n", g_trace, exp);
            }
            break;
        }
        case 4: {   /* f6106C, maintain or scrap */
            int scrap = rnd(3) == 0, useTok = (int)rnd(2);
            k = (int)rnd(4) + 1;
            setD8(NA(n, 0x40), (uint8_t)k);
            for (i = 0; i < 4; i++) {
                int tt = (int)rnd(157);
                setD16(NA(n, 0x45 + 2 * i), (uint16_t)(i < k ? tt + 1 : 0));
                if (i < k)
                    setD8(AR(tt, 0x1B + n), (uint8_t)(D8(AR(tt, 0x1B + n)) + 1));
            }
            i = (int)rnd((uint32_t)k);
            t = S16(NA(n, 0x45 + 2 * i)) - 1;
            if (useTok && !scrap) {
                setD8(AR(t, 0x0E + n), (uint8_t)(1 + rnd(3)));
                if (D8(AR(t, 1 + n)) == 0) {
                    setD8(AR(t, 1 + n), (uint8_t)n);
                    setD8(AR(t, 1), (uint8_t)(D8(AR(t, 1)) + 1));
                    setD8(NA(n, 0x61) + D8(NA(n, 0x60)), (uint8_t)(t + 1));
                    setD8(NA(n, 0x60), (uint8_t)(D8(NA(n, 0x60)) + 1));
                }
            }
            setD8(0xF097Fu, (uint8_t)(rnd(2) ? 0x40 : 0));
            g_unattended = (int)rnd(2);
            sync_model();
            f6106C(i, scrap, t + 1, useTok, n);
            m_maintain(i, scrap, t + 1, useTok, n);
            snprintf(what, sizeof what, "A f6106C op %d", op);
            ok = same_ds(what);
            {
                char exp[600], msg[200];
                int w = !g_unattended ? 1000 : D16(0xF40BAu) ? 1000 : 500;
                if (!scrap)
                    snprintf(msg, sizeof msg, "%s maintains a ship in %s using %s.", DSTR(D32(0xC768Cu + 4u * (uint32_t)n)),
                             DSTR(D32(0xC77D8u + 4u * (uint32_t)t)), useTok ? "Levy" : "Treasury");
                else
                    snprintf(msg, sizeof msg, "%s doesn't maintain a ship in %s.", DSTR(D32(0xC768Cu + 4u * (uint32_t)n)),
                             DSTR(D32(0xC77D8u + 4u * (uint32_t)t)));
                if (D8(0xF097Fu) & 0x40)
                    snprintf(exp, sizeof exp, "79270(-550,%d,%d,[%s],0) %s79270(-500,%d,%d,[%s],0) 3A990 3CBBC ", n, t,
                             msg, scrap ? "78A20(500) " : "", n, t, msg);
                else
                    snprintf(exp, sizeof exp, "79270(-550,%d,%d,[%s],0) %s3CBBC 7BA64 79270(%d,%d,%d,[%s],0) ", n, t, msg,
                             scrap ? "78A20(500) " : "", w, n, t, msg);
                check(strcmp(g_trace, exp) == 0, "A f6106C trace");
                if (strcmp(g_trace, exp) != 0)
                    printf("  got  %s\n  want %s\n", g_trace, exp);
            }
            break;
        }
        default: {  /* f7A598 / f7A610 */
            uint32_t a;
            for (a = 0x112CECu; a < 0x112CECu + 0x88Du; a++)
                setD8(a, (uint8_t)rnd(256));
            for (a = 0x1137D0u; a < 0x1139B0u + 0x2B20u; a++)
                setD8(a, (uint8_t)rnd(256));
            sync_model();
            if (rnd(2)) {
                f7A598();
                m_snap(0);
            } else {
                f7A610();
                m_snap(1);
            }
            snprintf(what, sizeof what, "A snapshot op %d", op);
            ok = same_ds(what);
            break;
        }
        }
    }
}

/* ---- B. phase scenarios ------------------------------------------------------------------------------------- */

static void give_tokens(int t, int n, int k)   /* k tokens of n at t (canonical), board + k */
{
    if (D8(AR(t, 1 + n)) == 0) {
        setD8(AR(t, 1 + n), (uint8_t)n);
        setD8(AR(t, 1), (uint8_t)(D8(AR(t, 1)) + 1));
        setD8(NA(n, 0x61) + D8(NA(n, 0x60)), (uint8_t)(t + 1));
        setD8(NA(n, 0x60), (uint8_t)(D8(NA(n, 0x60)) + 1));
    }
    setD8(AR(t, 0x0E + n), (uint8_t)(D8(AR(t, 0x0E + n)) + k));
    setD8(NA(n, 0x98), (uint8_t)(D8(NA(n, 0x98)) + k));
}

static void give_ship(int t, int n)
{
    int k = D8(NA(n, 0x40));
    setD16(NA(n, 0x45 + 2 * k), (uint16_t)(t + 1));
    setD8(NA(n, 0x40), (uint8_t)(k + 1));
    setD8(AR(t, 0x1B + n), (uint8_t)(D8(AR(t, 0x1B + n)) + 1));
}

static void set_order(const int *ns, int cnt)
{
    int i;
    setD16(0xF06AEu, (uint16_t)cnt);
    for (i = 0; i < cnt; i++)
        setD16(0xF06B0u + 2u * (uint32_t)i, (uint16_t)ns[i]);
}

static void base_state(void)
{
    int t;
    clear_world();
    for (t = 0; t < 160; t++)
        setD8(TR(t), (uint8_t)(D8(TR(t)) & ~2u));
    setD8(0xE1621u, 0);
    setD8(0xE1624u, 0);
    setD16(0xF40FCu, 4);
    setD8(0xF097Cu, 0);
    setD8(0xF097Fu, 0);
    setD32(0xF5940u, 7);
    setD32(0xF5470u, 0);
    g_trace[0] = 0;
    g_icode = g_ncodes = 0;
    g_iyesno = g_nyesno = 0;
}

static int has(const char *s) { return strstr(g_trace, s) != NULL; }

/* the UI-side writes of the phase routines that the model does not compute: copied into the model */
static void copy_ui_state(void)
{
    static const uint32_t addr[][2] = {
        { 0xBB3FAu, 8 }, { 0xEE168u, 1 }, { 0xF40F8u, 4 }, { 0x108804u, 1 }, { 0x10F2C0u, 8 }, { 0xEED74u, 8 },
        { 0xF4884u, 8 }, { 0xFFB60u, 0xA0 }, { 0xF7C9Cu, 4 + 4 * 160 }, { 0xF5500u, 0x1F4 } };
    size_t i;
    for (i = 0; i < sizeof addr / sizeof addr[0]; i++)
        memcpy(m_ds + (addr[i][0] - DS_BASE), g_ds + (addr[i][0] - DS_BASE), addr[i][1]);
}

static void part_b_maintenance(int original)
{
    static const int one[] = { 1 };
    int codes[] = { -100, 77, -5, -8, -6 };
    char buf[200];
    const int n = 1;

    g_original = original;
    base_state();
    setD8(NA(n, 0x9A), 3);
    setD8(NA(n, 0x3E), 1);
    setD8(NA(n, 0x99), 5);
    give_tokens(10, n, 3);
    give_tokens(57, n, 1);
    give_tokens(40, n, 2);
    give_ship(10, n);    /* k0: code 77, kept for nothing */
    give_ship(20, n);    /* k1: treasury */
    give_ship(35, n);    /* k2: scrapped */
    give_ship(157, n);   /* k3 -> k2 after the scrap: levy at the second coast of 57 (its last token there) */
    set_order(one, 1);
    script(codes, 5);
    sync_model();
    f60BD8();
    /* the model, step by step */
    MW8(NA(n, 0x3E), M8(NA(n, 0x3E)) - 1); MW8(NA(n, 0x99), M8(NA(n, 0x99)) + 1);
    m_f78D98(2, n);
    MW8(AR(57, 0x0E + n), M8(AR(57, 0x0E + n)) - 1); MW8(NA(n, 0x99), M8(NA(n, 0x99)) + 1);
    MW8(NA(n, 0x98), M8(NA(n, 0x98)) - 1); m_f581C4(original ? 157 : 57, n);
    copy_ui_state();
    snprintf(buf, sizeof buf, "B maintenance (original %d)", original);
    same_ds(buf);
    check(D8(AR(57, 1 + n)) == (original ? n : 0), "B maintenance: BUG A, the marker at 57");
    check(has(", select maintenance options for\nShip #1 in"), "B maintenance: the first screen's text");
    snprintf(buf, sizeof buf, "Ship #3 in %s]", DSTR(D32(0xC77D8u + 4u * 157u)));
    check(has(buf), "B maintenance: the fourth screen shows the ship on the second coast");
    check(g_icode == 5, "B maintenance: every scripted click used");
    check(!has("79270(250"), "B maintenance: no empty message");
    check(!has("AI") && !has("7A940"), "B maintenance: no AI, no housekeeping without computers");
    check(strncmp(g_trace, "3CD6C(3) vbcopy(0,639,0,479,0,479,", 34) == 0, "B maintenance: the background first");
    check(has("7ABD0 ") && strstr(g_trace, "7ABD0 ") > strstr(g_trace, "78664(C6E40)"), "B maintenance: f7ABD0 at the end");
    /* enables of the first screen: treasury 2 > 0 -> on 0x34; 3 tokens at 10 -> on 0x35 */
    check(has("3DA60") || has("787F8(10)"), "B maintenance: centred on the ship");
    check(has("on(F596C,34) on(F596C,35) 459E4(1) 3A924 3AB7C["), "B maintenance: the first screen's buttons");
    /* the third screen (35: no tokens, treasury 0) and the fourth (157: 1 token at 57) */
    check(has("off(F596C,34) off(F596C,35) 459E4"), "B maintenance: both disabled");
    check(has("off(F596C,34) on(F596C,35) 459E4"), "B maintenance: treasury button disabled when empty");
    g_original = 0;
}

static void part_b_maintenance_misc(void)
{
    static const int ord[] = { 2, 3, 4 };
    static const int codes[] = { -5 };
    base_state();
    /* nobody acted: 0 ships everywhere, no computers -> the empty message for the first nation */
    setD8(NA(2, 0x9A), 3);
    setD8(NA(3, 0x9A), 2);
    set_order(ord, 2);
    f60BD8();
    check(has("AI61254(3) ") && has("7ABD0 79270(250,2,-200,[],0) "), "B maintenance: empty message, computer called");
    /* computers: the housekeeping after every nation but the last, from round 7 with the message and the floor */
    base_state();
    setD8(0xE1621u, 1);
    setD16(0xF40FCu, 7);
    setD8(NA(2, 0x9A), 3);
    setD8(NA(2, 0x3E), 1);
    give_ship(12, 2);
    setD8(NA(3, 0x9A), 2);
    give_ship(14, 3);
    setD8(NA(4, 0x9A), 0);   /* out of play: housekeeping only */
    set_order(ord, 3);
    script(codes, 1);
    f60BD8();
    check(has("78664(C6E40) 79270(-550,2,-250,[Maintenance housekeeping for "), "B maintenance: housekeeping message");
    check(has("7A940 AI461BC(2) 7AC60 78A20(250) AI61254(3) "), "B maintenance: housekeeping after the human");
    check(has("AI61254(3) 79270(-550,3,-250,") && has("AI461BC(3) 7AC60 78A20(250) 7ABD0"),
          "B maintenance: housekeeping after the computer, not after the last");
    check(!has("AI461BC(4)"), "B maintenance: none after the last nation");
    check(!has("79270(250"), "B maintenance: someone acted");
}

static int site_list_is(const int *ts, int cnt)
{
    int i;
    if (S32(0xF7C9Cu) != cnt)
        return 0;
    for (i = 0; i < cnt; i++)
        if (S32(0xF7CA0u + 4u * (uint32_t)i) != ts[i])
            return 0;
    return 1;
}

static void part_b_construction_human(int confirm)
{
    static const int one[] = { 1 };
    const int n = 1;
    char buf[200];
    int32_t msel = 0;
    /* -10 next, -9 back, map click on index 1, BY BOTH at 20, FROM TREASURY at 10, RESTART (asked with confirm: NO,
       then again YES), BY LEVY at 10, FINISHED */
    int c1[] = { -10, -9, 1, -7, -5, -99, -6, -1 };
    int c2[] = { -10, -9, 1, -7, -5, -99, -99, -6, -1 };

    base_state();
    setD8(0xF097Cu, confirm ? 8 : 0);
    setD8(NA(n, 0x9A), 3);
    setD8(NA(n, 0x3E), 5);
    setD8(NA(n, 0x99), 6);
    setD8(TR(10), (uint8_t)(D8(TR(10)) | 2));
    setD8(TR(20), (uint8_t)(D8(TR(20)) | 2));
    setD8(TR(30), (uint8_t)(D8(TR(30)) | 2));
    give_tokens(10, n, 3);
    give_tokens(20, n, 1);
    give_tokens(40, n, 4);   /* inland */
    set_order(one, 1);
    if (confirm) {
        script(c2, 9);
        g_yesno[0] = -0x62;
        g_yesno[1] = -0x61;
        g_nyesno = 2;
    } else
        script(c1, 8);
    sync_model();
    f61940();
    /* the model: after RESTART the state is the start's; then BY LEVY at 10 */
    MW8(AR(10, 0x0E + n), M8(AR(10, 0x0E + n)) - 2);
    MW8(NA(n, 0x98), M8(NA(n, 0x98)) - 2);
    MW8(NA(n, 0x99), M8(NA(n, 0x99)) + 2);
    m_f581C4(10, n);
    m_human_ship(10, n, &msel);
    copy_ui_state();
    snprintf(buf, sizeof buf, "B construction human (confirm %d)", confirm);
    same_ds(buf);
    check(D32(0xEED74u) == 0 && D32(0xEED78u) == 0, "B construction: heap blocks freed");
    check(D8(0x108804u) == 1, "B construction: land mode at the end");
    check(has("3A9F4[CANDIDATE SITES FOR SHIP CONSTRUCTION] 7844C(F596C,7) "), "B construction: the title");
    check(has("78BA4(F596C,1)=-10 ") && g_icode == g_ncodes, "B construction: all clicks used, map clicks on");
    {   /* after BY BOTH at 20 (its only token): the sites are [10] */
        static const int after[] = { 10, 20 };
        check(site_list_is(after, 2), "B construction: the sites at the end");
    }
    snprintf(buf, sizeof buf, "3A0B0(265,389,110,[%s]) ", DSTR(D32(0xC77D8u + 4u * 20u)));
    check(has(buf), "B construction: the site box shows the second site");
    check(confirm ? has("795A8[Restart ") && g_iyesno == 2 : !has("795A8"), "B construction: the Restart question");
    check(!has("79270(250"), "B construction: no empty message");
    check(has("78664(C722C) 7ABD0 ") , "B construction: dialog removed, stack freed");
    /* the enables before the first build: tokens 3 at 10 -> 0x37 on; treasury 5 -> 0x36 on; 0x38 on */
    check(has("off(F596C,36) off(F596C,37) off(F596C,38) on(F596C,37) on(F596C,36) on(F596C,38) "),
          "B construction: button enables");
}

static void part_b_construction_four(void)
{
    static const int one[] = { 1 };
    static const int codes[] = { -5 };
    static const int fin[] = { -1 };
    const int n = 1;
    base_state();
    setD8(NA(n, 0x9A), 3);
    setD8(NA(n, 0x3E), 4);
    setD8(NA(n, 0x99), 3);
    setD8(TR(10), (uint8_t)(D8(TR(10)) | 2));
    give_tokens(10, n, 3);   /* board 3 > 2 */
    give_ship(10, n);
    give_ship(10, n);
    give_ship(10, n);
    set_order(one, 1);
    script(codes, 1);
    f61940();
    check(D8(NA(n, 0x40)) == 4 && g_icode == 1, "B construction: the screen ends by itself at 4 ships");
    /* a human with 4 ships goes to the computers' search */
    base_state();
    setD8(NA(n, 0x9A), 3);
    setD8(NA(n, 0x99), 3);
    setD8(TR(10), (uint8_t)(D8(TR(10)) | 2));
    give_tokens(10, n, 3);   /* board 3 > 2 */
    give_ship(10, n); give_ship(10, n); give_ship(10, n); give_ship(10, n);
    set_order(one, 1);
    g_r62840 = -1;
    f61940();
    check(has("AI62840(1)=-1 ") && !has("3A9F4[CANDIDATE"), "B construction: 4 ships -> the AI search");
    check(has("79270(250,1,-200,[],0)"), "B construction: nobody built -> empty message");
    /* nothing to do: board 2 and no cities -> skipped without sites, however large the stock (0x61AAD reads +0x98) */
    base_state();
    setD8(NA(n, 0x9A), 3);
    setD8(NA(n, 0x99), 30);
    setD8(TR(10), (uint8_t)(D8(TR(10)) | 2));
    give_tokens(10, n, 2);
    set_order(one, 1);
    f61940();
    check(!has("78BA4") && has("79270(250,1,-200,[],0)"), "B construction: board 2, stock 30, no cities: nothing");
    /* board 3, stock 0, no cities -> the screen (the site at 10 with 3 tokens) */
    base_state();
    setD8(NA(n, 0x9A), 3);
    setD8(NA(n, 0x99), 0);
    setD8(TR(10), (uint8_t)(D8(TR(10)) | 2));
    give_tokens(10, n, 3);
    set_order(one, 1);
    script(fin, 1);
    f61940();
    check(has("3A9F4[CANDIDATE") && g_icode == 1 && !has("79270(250"), "B construction: board 3, stock 0, no cities: the screen");
}

static void part_b_construction_ai(int original)
{
    static const int one[] = { 2 };
    const int n = 2;
    char buf[200];
    g_original = original;
    base_state();
    setD8(NA(n, 0x9A), 2);
    setD8(NA(n, 0x3E), 7);
    setD8(NA(n, 0x99), 1);
    setD8(NA(n, 0x51), 1);
    setD32(0xF5470u, 3);
    setD8(TR(10), (uint8_t)(D8(TR(10)) | 2));
    setD8(TR(21), (uint8_t)(D8(TR(21)) | 2));
    setD8(TR(31), (uint8_t)(D8(TR(31)) | 2));
    give_tokens(10, n, 3);
    give_tokens(21, n, 1);   /* coastal, 1 token; territory 20 (record 21) has 5 */
    give_tokens(20, n, 5);
    give_tokens(31, n, 4);   /* coastal, 4 tokens; territory 30 (record 31) has 0 */
    /* the option table: option 1, one ship at 10 for treasury 1 + levy 1 */
    memset(g_ds + (0x10F6B8u - DS_BASE), 0, 36);
    setD8(0x10F6B8u + 18u + 2u, 1);
    setD8(0x10F6B8u + 18u + 3u, 10);
    setD8(0x10F6B8u + 18u + 7u, 1);
    setD8(0x10F6B8u + 18u + 11u, 1);
    g_r62840 = 1;
    set_order(one, 1);
    sync_model();
    f61940();
    /* model: the option's ship, then extra ships while stock < 4, ships < 4, treasury >= 2: candidates 21 and 31
       (10 has a ship now); fixed: by n's tokens there (31: 4, 21: 1) -> 31 first; original: by records 21 / 31 =
       territories 20 / 30 (5, 0) -> 21 first. Stock 1 + 2 = 3 < 4 after the option's ship, then +2 = 5 stops. */
    m_computer_ship(10, 1, 1, n);
    m_computer_ship(original ? 21 : 31, 0, 2, n);
    copy_ui_state();
    snprintf(buf, sizeof buf, "B construction AI (original %d)", original);
    same_ds(buf);
    check(!has("79270(250"), "B construction AI: no empty message");
    check(has("AI62840(2)=1 79270(-550,2,10,"), "B construction AI: the option's ship first");

    /* a tie (2 and 2 tokens; records 21 / 31 both empty under --original): no swap, the list order (21 first) stays */
    base_state();
    setD8(NA(n, 0x9A), 2);
    setD8(NA(n, 0x3E), 2);
    setD8(NA(n, 0x99), 3);
    setD8(NA(n, 0x51), 1);
    setD32(0xF5470u, 3);
    setD8(TR(21), (uint8_t)(D8(TR(21)) | 2));
    setD8(TR(31), (uint8_t)(D8(TR(31)) | 2));
    give_tokens(21, n, 2);
    give_tokens(31, n, 2);
    g_r62840 = -1;
    set_order(one, 1);
    sync_model();
    f61940();
    m_computer_ship(21, 0, 2, n);
    copy_ui_state();
    snprintf(buf, sizeof buf, "B construction AI, a tie (original %d)", original);
    same_ds(buf);

    /* BUG C: no candidate (every coastal territory of its list has its ship) */
    base_state();
    setD8(NA(n, 0x9A), 2);
    setD8(NA(n, 0x3E), 7);
    setD8(NA(n, 0x99), 3);
    setD8(NA(n, 0x51), 1);
    setD32(0xF5470u, 3);
    setD8(TR(10), (uint8_t)(D8(TR(10)) | 2));
    give_tokens(10, n, 3);
    give_ship(10, n);
    g_r62840 = -1;
    set_order(one, 1);
    sync_model();
    f61940();
    if (original)
        m_computer_ship(0, 0, 2, n);   /* cand[0] as the port starts it: 0 */
    copy_ui_state();
    snprintf(buf, sizeof buf, "B construction AI, no candidate (original %d)", original);
    same_ds(buf);
    g_original = 0;
}

/* ---- C. the saves ------------------------------------------------------------------------------------------- */

static int load_save(const char *path, char *desc)
{
    int16_t ver;
    FILE *fp = fopen(path, "rb");
    if (fp == NULL)
        return -1;
    f5F05C(&ver, desc, fp);
    fclose(fp);
    return ver;
}

static char g_cwd[400];

static void part_c(const char *gamedir, const char *savedir)
{
    char path[400], desc[64];
    int i, n, j;
    static uint8_t nat_a[14 * 199];

    if (chdir(gamedir) == 0) {   /* terrflag.dat: the territory records are not in the save */
        f12394();
        (void)chdir(g_cwd);
    }
    snprintf(path, sizeof path, "%s/SAVGAMEA.DAT", savedir);
    if (load_save(path, desc) < 0) {
        printf("C. %s not found: skipped\n", path);
        return;
    }
    printf("C. %s: \"%s\", phase %d, round %d, humans %d, computers %d, Restart Confirmations %s\n", path, desc,
           D8(0xF4102u), S16(0xF40FCu), S8(0xE1620u), S8(0xE1621u), (D8(0xF097Cu) & 8) ? "on" : "off");
    g_trace[0] = 0;
    f60AE0();
    printf("   ship order:");
    for (i = 0; i < S16(0xF06AEu); i++)
        printf(" %s", DSTR(D32(0xC768Cu + 4u * (uint32_t)S16(0xF06B0u + 2u * (uint32_t)i))));
    printf("\n");
    for (i = 0; i < S16(0xF06AEu); i++) {
        n = S16(0xF06B0u + 2u * (uint32_t)i);
        f62658(n);
        printf("   %d %-9s %s treasury %d stock %d board %d cities %d ships %d military %d; sites (%d):", n,
               DSTR(D32(0xC768Cu + 4u * (uint32_t)n)), D8(NA(n, 0x9A)) == 3 ? "human" : D8(NA(n, 0x9A)) == 2 ? "computer" : "out",
               S8(NA(n, 0x3E)), S8(NA(n, 0x99)), S8(NA(n, 0x98)), S8(NA(n, 0x51)), D8(NA(n, 0x40)), D8(NA(n, 0x35)),
               S32(0xF7C9Cu));
        for (j = 0; j < S32(0xF7C9Cu); j++) {
            int t = S32(0xF7CA0u + 4u * (uint32_t)j);
            printf(" %s(%d:%d)", DSTR(D32(0xC77D8u + 4u * (uint32_t)t)), t, S8(AR(D8(0xC803Cu + (uint32_t)t), 0x0E + n)));
        }
        printf("\n");
    }
    for (n = 1; n <= 9; n++)
        if (D8(NA(n, 0x9A)) == 3) {
            f62658(n);
            check(S32(0xF7C9Cu) > 0 && strcmp(DSTR(D32(0xC77D8u + 4u * (uint32_t)S32(0xF7CA0u))), "SABRATA") == 0,
                  "C. the human's first candidate site in R4 is SABRATA");
        }
    memcpy(nat_a, g_ds + (NAT_BASE - DS_BASE), sizeof nat_a);
    snprintf(path, sizeof path, "%s/savgame0.dat", gamedir);
    if (load_save(path, desc) < 0) {
        printf("   %s not found\n", path);
        return;
    }
    printf("   %s: \"%s\", phase %d; ships built in phase 4 (nation: ships before -> after, treasury, stock, board):\n",
           path, desc, D8(0xF4102u));
    for (n = 1; n <= 9; n++) {
        const uint8_t *a = nat_a + 199 * n;
        if (D8(NA(n, 0x9A)) == 0)
            continue;
        if (a[0x40] != D8(NA(n, 0x40)) || a[0x3E] != D8(NA(n, 0x3E))) {
            printf("     %-9s %d -> %d, %d -> %d, %d -> %d, %d -> %d; new at", DSTR(D32(0xC768Cu + 4u * (uint32_t)n)),
                   a[0x40], D8(NA(n, 0x40)), (int8_t)a[0x3E], S8(NA(n, 0x3E)), (int8_t)a[0x99], S8(NA(n, 0x99)),
                   (int8_t)a[0x98], S8(NA(n, 0x98)));
            for (j = a[0x40]; j < D8(NA(n, 0x40)); j++)
                printf(" %s", DSTR(D32(0xC77D8u + 4u * (uint32_t)(S16(NA(n, 0x45 + 2 * j)) - 1))));
            printf("\n");
        }
    }
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:/work/advciv_run";
    const char *savedir = argc > 2 ? argv[2] : "C:/work/advciv_play";
    int fa;

    if (state_init(gamedir) != 0)
        return 1;
    m_ds = malloc(DS_SIZE);
    g_save = malloc(DS_SIZE);
    if (m_ds == NULL || g_save == NULL)
        return 1;
    memcpy(g_save, g_ds, DS_SIZE);

    part_a(60000);
    fa = g_fail;
    printf("A. helpers against the model: %d checks, %d fail\n", g_tests, g_fail);
    g_original = 1;
    part_a(12000);
    g_original = 0;
    printf("A. --original: %d fail so far\n", g_fail);

    memcpy(g_ds, g_save, DS_SIZE);
    part_b_maintenance(0);
    part_b_maintenance(1);
    part_b_maintenance_misc();
    part_b_construction_human(0);
    part_b_construction_human(1);
    part_b_construction_four();
    part_b_construction_ai(0);
    part_b_construction_ai(1);
    printf("B. phase scenarios: %d fail (all parts so far, A had %d)\n", g_fail, fa);

    memcpy(g_ds, g_save, DS_SIZE);
    if (getcwd(g_cwd, sizeof g_cwd) == NULL)
        g_cwd[0] = 0;
    part_c(gamedir, savedir);

    printf("%d checks, %d fail\n", g_tests, g_fail);
    return g_fail == 0 ? 0 : 1;
}
