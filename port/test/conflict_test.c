/* conflict_test [GAMEDIR [SAVEDIR]]: unit tests for module CONFLICT (conflict.c) that need no game run.

   It loads the data object from GAMEDIR/AC_MAIN.EXE as the port does (state_init; GAMEDIR defaults to the run copy
   C:\work\advciv_run, only read) and the territory records from GAMEDIR's terrflag.dat (f12394: the population limits
   are not in the save). conflict.c is compiled with -DCONFLICT_TEST, so its two calls into unported code (0x5BF30, the
   computer's plunder transfer; 0x7A4C4, the calamity animation) go to recorders here, which return. The UI routines
   are stubs that record their calls in order; f78BA4 answers from a seeded code stream, f7BA64 with a per-run flag,
   game_rand (f78AE4) is the LCG of game.h on [0xCAC5C] and counts its calls. The primitives conflict.c calls (move.c's
   f57CE8 / f57E28 / f581C4, rules.c's f78F04 / f7C760 / f75CA8 / f75D84, mapdata.c's highlights and f3DA60) are the
   real, separately tested ones.
     A. random differential runs of f5AAA4 (both values of show, attended and not), and through it f5B224, f5B588,
        f5BA0C, f5C110, f5C1CC and f5CDE8's click loop, against a second implementation written here from the listing,
        not from conflict.c, on a plain copy of the data object (m_ds). The model reuses the verified primitives by
        running them on its copy (g_ds swapped). States: a real save with random conflicts added through the
        primitives (cities of nations 1..10, tokens of 1..11, owners without defenders, Engineering / Metalworking /
        Architecture / Military, stocks, cards and calamities, PBEM, the pre-movement copies, the hostility bytes).
        After every run the whole data object, the number of game_rand calls, the code-stream position and the calls
        of the unported routines must equal the model's. Then deliberately broken models must be caught.
     B. scripted scenarios checked against the order of the listing and the expected values: the losses box (columns,
        positions, the short names and BUG 2 in both modes, the exits -0x50 / -0x51 / -1, unattended, no losses), the
        plunder (the card draw, the transfer screen and its clicks, one unit, no stock and its text fix), the phase's
        screen order, the empty message, BUG 1 in both modes, the hostility limits.
     C. the saves: SAVEDIR/SAVGAMEA.DAT and savgameb.dat, GAMEDIR/savgame0.dat (start of MOVEMENT R4) and savgame1.dat
        (R4, all human) read with saveload.c's f5F05C; f5AAA4(1) run on each against the model, the contested areas
        and the hidden nations' in-play bytes printed (for the DOSBox-X check plan).
   Prints each failure, the counts per part, and exits 0 when all pass.

   Build (from the repo root):
     clang -std=c11 -Wall -Wextra -DCONFLICT_TEST -Iport/src port/test/conflict_test.c port/src/conflict.c
       port/src/move.c port/src/rules.c port/src/mapdata.c port/src/load.c port/src/saveload.c port/src/state.c
       port/src/le_image.c -o conflict_test.exe */

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
#include "../src/conflict.h"
#include "../src/fg.h"
#include "../src/ui.h"
#include "../src/kit.h"
#include "../src/map.h"
#include "../src/mapdata.h"
#include "../src/move.h"
#include "../src/panel.h"
#include "../src/rules.h"
#include "../src/saveload.h"
#include "../src/start.h"

/* ---- trace, checks ----------------------------------------------------------------------------------------------- */

static char g_trace[1 << 17];
static int g_quiet;   /* > 0 while the model runs (no trace) */
static int g_tests, g_fail;

static void tr(const char *fmt, ...)
{
    size_t len;
    va_list ap;
    if (g_quiet)
        return;
    len = strlen(g_trace);
    va_start(ap, fmt);
    vsnprintf(g_trace + len, sizeof g_trace - len, fmt, ap);
    va_end(ap);
}

static int check(int ok, const char *what)
{
    g_tests++;
    if (!ok) {
        g_fail++;
        printf("FAIL %s\n", what);
    }
    return ok;
}

static int has(const char *s) { return strstr(g_trace, s) != NULL; }

/* ---- the code stream of f78BA4 (the port) and of the model ------------------------------------------------------- */

static const int k_codes[] = { -100, -12, -12, -12, -11, -1, -1, 5, -0x50, -0x51, -2, -11 };
typedef struct { uint32_t s; int used; } stream;
static stream g_port_codes, g_model_codes;
static int g_script[64], g_nscript, g_iscript;   /* part B: an explicit script instead of the stream */
static int g_unattended;

static int next_code(stream *st)
{
    st->s = st->s * 1103515245u + 12345u;
    st->used++;
    return k_codes[(st->s >> 16) % (sizeof k_codes / sizeof k_codes[0])];
}

/* ---- stubs ------------------------------------------------------------------------------------------------------- */

int fg_vbopen(int handle) { tr("vbopen(%X) ", (unsigned)handle); return 0; }
void fg_vbclose(void) { tr("vbclose "); }
int fg_vbcopy(int a, int b, int c, int d, int e, int f, int s, int t)
{
    tr("vbcopy(%d,%d,%d,%d,%d,%d,%X,%X) ", a, b, c, d, e, f, (unsigned)s, (unsigned)t);
    return 0;
}
int fg_setcolor(int c) { tr("setcolor(%d) ", c); return 0; }
int fgf_width(const char *s, int n) { (void)s; return 8 * n; }   /* 8 pixels a character (test font) */
void f3CD6C(int8_t pic) { tr("3CD6C(%d) ", pic); }
void f3C3D0(uint32_t dlg, int id) { tr("off(%X,%X) ", (unsigned)dlg, id); }
void f3C314(uint32_t dlg, int id) { tr("on(%X,%X) ", (unsigned)dlg, id); }
void f459E4(int n) { tr("459E4(%d) ", n); }
void f3A924(void) { tr("3A924 "); }
void f3A9F4(const char *text) { tr("3A9F4[%s] ", text); }
void f7844C(uint32_t dlg, int base) { tr("7844C(%X,%d) ", (unsigned)dlg, base); }
void f3CBBC(void) { tr("3CBBC "); }
void f78664(uint32_t dlg) { tr("78664(%X) ", (unsigned)dlg); }
void f7853C(uint32_t dlg) { tr("7853C(%X) ", (unsigned)dlg); }
void f7876C(void) { tr("7876C "); }
void f3B05C(uint32_t dlg) { tr("3B05C(%X) ", (unsigned)dlg); }
void f79270(int wait, int n, int t, const char *text, int anim) { tr("79270(%d,%d,%d,[%s],%d) ", wait, n, t, text, anim); }
void f78A20(int t) { tr("78A20(%d) ", t); }
int f7BA64(void) { tr("7BA64 "); return g_unattended; }
void f3AF8C(uint32_t spr, int x, int y) { tr("3AF8C(%X,%d,%d) ", (unsigned)spr, x, y); }
void f3C9D0(int x, int y, int w, int h, int flags) { tr("3C9D0(%d,%d,%d,%d,%d) ", x, y, w, h, flags); }
void f3A0B0(int x, int y, int w, const char *s) { tr("3A0B0(%d,%d,%d,[%s]) ", x, y, w, s); }
void f3A3A0(int x, int y, const char *s) { tr("3A3A0(%d,%d,[%s]) ", x, y, s); }
void f3A044(int x, int y, const char *s) { tr("3A044(%d,%d,[%s]) ", x, y, s); }
void f783B8(uint32_t dlg, int base) { tr("783B8(%X,%d) ", (unsigned)dlg, base); }
void f3D52C(int dx, int dy) { tr("3D52C(%d,%d) ", dx, dy); }
void f787F8(int t) { tr("787F8(%d) ", t); }
int f78BA4(uint32_t dlg, int map)
{
    int code;
    if (g_nscript > 0) {
        if (g_iscript >= g_nscript) {
            printf("FAIL f78BA4 called beyond the script\n");
            g_fail++;
            code = -1;
        } else
            code = g_script[g_iscript++];
    } else
        code = next_code(&g_port_codes);
    tr("78BA4(%X,%d)=%d ", (unsigned)dlg, map, code);
    return code;
}

/* game_rand as game.h describes it (0x78AE4) */
static int g_rand_calls;
int f78AE4(void)
{
    uint32_t s = D32(0xCAC5Cu) * 0x15A4E35u + 1u;
    setD32(0xCAC5Cu, s);
    g_rand_calls++;
    tr("rand ");
    return (int)((s >> 16) & 0x7FFF);
}

/* the unported routines (CONFLICT_TEST hooks) */
static char g_ai_port[4096], g_ai_model[4096];
static void logto(char *log, size_t size, const char *fmt, ...)
{
    size_t len = strlen(log);
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(log + len, size - len, fmt, ap);
    va_end(ap);
}
void conflict_test_f5BF30(int n, int amount, int show)
{
    tr("AI5BF30(%d,%d,%d) ", n, amount, show);
    logto(g_ai_port, sizeof g_ai_port, "5BF30(%d,%d,%d) ", n, amount, show);
}
void conflict_test_f7A4C4(int anim)
{
    tr("ANIM7A4C4(%d) ", anim);
    logto(g_ai_port, sizeof g_ai_port, "7A4C4(%d) ", anim);
}

/* reached by nothing here; the linked files need them */
_Noreturn void not_ported(uint32_t addr)
{
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
void f76F70(int n, uint8_t *cards) { (void)n; (void)cards; }

/* ---- addresses (from the listing's constants, written out here) ------------------------------------------------- */

#define AREA0     0xE31B8u
#define AREA_LEN  (161u * 69u)
#define NAT0      0xF14CCu
#define TS0       0xE2C18u
#define REC(r)    (AREA0 + 69u * (uint32_t)(r))
#define NA(n, o)  (NAT0 + 199u * (uint32_t)(n) + (uint32_t)(o))
#define TSA(t)    (TS0 + 3u * (uint32_t)(t))
#define PREVA(r)  (0xE8888u + 69u * (uint32_t)(r))
#define PREVT(t)  (0xE2FD8u + 3u * (uint32_t)(t))
#define LOSST     0xF547Cu
#define LOSSC     0xF548Cu
#define MAT       0x109E24u
#define PEND      0x109E08u
#define PATT      0x109E09u
#define POWN      0x109E0Au
#define PREC      0x109E0Cu

/* ---- the model: a second implementation on m_ds ---------------------------------------------------------------- */

static uint8_t *m_ds, *g_real, *g_base[4];
static int g_mut;   /* 0 = the listing; 1.. = deliberately broken (part A's last step) */

static int      M8(uint32_t a) { return m_ds[a - DS_BASE]; }
static int      MS8(uint32_t a) { return (int8_t)m_ds[a - DS_BASE]; }
static int      MS16(uint32_t a) { return (int16_t)(m_ds[a - DS_BASE] | m_ds[a - DS_BASE + 1] << 8); }
static int32_t  MS32(uint32_t a) { const uint8_t *p = m_ds + (a - DS_BASE); return (int32_t)((uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24); }
static void     MW8(uint32_t a, int v) { m_ds[a - DS_BASE] = (uint8_t)v; }
static void     MW16(uint32_t a, int v) { m_ds[a - DS_BASE] = (uint8_t)v; m_ds[a - DS_BASE + 1] = (uint8_t)(v >> 8); }
static void     MW32(uint32_t a, int32_t v) { uint8_t *p = m_ds + (a - DS_BASE); uint32_t u = (uint32_t)v; p[0] = (uint8_t)u; p[1] = (uint8_t)(u >> 8); p[2] = (uint8_t)(u >> 16); p[3] = (uint8_t)(u >> 24); }
static void     MADD8(uint32_t a, int v) { MW8(a, M8(a) + v); }

static void mb(void) { g_real = g_ds; g_ds = m_ds; g_quiet++; }
static void me(void) { g_ds = g_real; g_quiet--; }
static void m_take(int t, int n, int k) { mb(); f57CE8(t, n, 0, k); me(); }
static void m_put(int t, int n, int k) { mb(); f57E28(t, n, k); me(); }
static void m_uncity(int t, int n) { mb(); f78F04(t, n); me(); }
static int  m_first(int t) { int v; mb(); v = f7C760(t); me(); return v; }
static void m_card_from(int c, uint32_t rec) { mb(); f75CA8(c, rec); me(); }
static void m_card_to(int c, uint32_t rec, uint32_t giver) { mb(); f75D84(c, rec, giver); me(); }
static void m_centre(int t) { mb(); f3DA60(t); me(); }
static void m_highlight(int t) { mb(); f3F670(); f3F624(t, 15); me(); }
static int  m_rand_calls;
static int  m_rand(void)
{
    uint32_t s = (uint32_t)MS32(0xCAC5Cu) * 0x15A4E35u + 1u;
    MW32(0xCAC5Cu, (int32_t)s);
    m_rand_calls++;
    return (int)((s >> 16) & 0x7FFF);
}
static int m_code(void) { return next_code(&g_model_codes); }

static int m_poplimit(int t) { return (M8(0xE1638u + 35u * (uint32_t)t + 1u) >> 3) & 7; }
static int m_tok(int r, int n) { return MS8(REC(r) + 0x0Eu + (uint32_t)n); }
static int m_city(int t) { return M8(TSA(t) + 2u) & 1; }

/* 0x5CDE8's click loop (the drawing writes no data) */
static int m_box(void)
{
    for (;;) {
        int c = g_unattended ? -1 : m_code();
        if (c == -1)
            return 0;
        if (c == -0x51)
            return 13;
        if (c == -0x50)
            return 12;
    }
}

/* 0x5B588 */
static void m_battle(int r)
{
    int cnt[12] = {0}, nat[12] = {0}, np = 0, total = 0, n, i, j, x, flag = 0, limit, idx, grp;
    int city = m_city(r - 1);
    for (n = 1; n <= 11; n++) {
        int tk = M8(REC(r) + 0x0Eu + (uint32_t)n);
        if (tk == 0)
            continue;
        cnt[np] = (tk + ((M8(NA(n, 0x26)) && g_mut != 1) ? 50 : 0)) & 0xFF;
        nat[np] = n;
        total += (int8_t)tk;
        np++;
    }
    if (M8(0xE1624u) & 0x80) {
        for (i = 0; i < np; i++) {
            if (!M8(NA(nat[i], 0x35)))
                continue;
            for (j = i + 1; j < np; j++) {
                if (M8(NA(nat[j], 0x35)))
                    continue;
                x = g_original ? np : j;   /* the entry past the list is 0 (cnt[] zeroed) */
                if (cnt[x] == 0) {
                    flag = 1;
                    continue;
                }
                cnt[x]--;
                m_take(r - 1, nat[j], 1);
                MADD8(NA(nat[j], 0x98), -1);
                MADD8(NA(nat[j], 0x99), 1);
                MADD8(LOSST + (uint32_t)nat[j], 1);
            }
        }
    }
    if (flag) {
        int k = 0;
        for (i = 0; i < np; i++)
            if (cnt[i]) {
                cnt[k] = cnt[i];
                nat[k] = nat[i];
                k++;
            }
        np = k;
    }
    if (np < 2)
        return;
    for (i = 1; i < np; i++) {   /* insertion sort, stable like the original's bubble sort with strict > */
        int c = cnt[i], v = nat[i];
        for (j = i; j > 0 && cnt[j - 1] > c; j--) {
            cnt[j] = cnt[j - 1];
            nat[j] = nat[j - 1];
        }
        cnt[j] = c;
        nat[j] = v;
    }
    limit = m_poplimit(r - 1);
    idx = 0;
    while ((!city && total > limit && MS8(REC(r) + 1u) > 1) || (city && MS8(REC(r) + 1u) > 1)) {
        grp = cnt[idx];
        do {
            n = nat[idx];
            if (m_tok(r, n) > 0) {
                m_take(r - 1, n, 1);
                if (M8(0xBB3F8u) != 0 || g_mut == 7) {
                    MADD8(NA(n, 0x98), -1);
                    MADD8(NA(n, 0x99), 1);
                }
                MADD8(LOSST + (uint32_t)n, 1);
                total--;
            }
            idx++;
        } while (idx < np && cnt[idx] == grp);
        if (idx >= np)
            idx = 0;
    }
    if (m_tok(r, 10) != 0) {
        MADD8(LOSST + 10u, M8(REC(r) + 0x18u));
        m_take(r - 1, 10, m_tok(r, 10));
    }
}

/* 0x5B224 */
static void m_attack(int r)
{
    int o = M8(TSA(r - 1)), a = 0, n, rep, need, eo, ea, tk;
    if (MS8(REC(r) + 1u) > 1)
        m_battle(r);
    if (MS8(REC(r) + 1u) == 0) {
        MW8(PEND, 0);
        return;
    }
    for (n = 1; n <= 11; n++)
        if (MS8(REC(r) + 1u + (uint32_t)n) > 0) {
            if (n == o) {
                MW8(PEND, 0);
                return;
            }
            a = n;
            break;
        }
    if (a == 0) {   /* no nation 1..11 there (only the factions): undefined in the listing, no attack in the port */
        MW8(PEND, 0);
        return;
    }
    eo = MS8(NA(o, 0x2A));
    ea = MS8(NA(a, 0x2A));
    if (eo == 1 && ea == 0) { rep = 7; need = 8; }
    else if (eo == 0 && ea == 1) { rep = 5; need = 6; }
    else { rep = 6; need = 7; }
    if (g_mut == 2)
        need = 7;
    tk = m_tok(r, a);
    if (tk < need) {
        MW8(PEND, 0);
        MADD8(LOSST + (uint32_t)a, tk);
        MADD8(NA(a, 0x99), tk);
        MADD8(NA(a, 0x98), -tk);
        m_take(r - 1, a, tk);
        return;
    }
    if (o != 10 && rep > MS8(NA(o, 0x99)))
        rep = MS8(NA(o, 0x99));
    m_uncity(r - 1, o);
    MADD8(LOSSC + (uint32_t)o, 1);
    m_put(r - 1, o, rep);
    MADD8(NA(o, 0x99), -rep);
    MADD8(NA(o, 0x98), rep);
    m_battle(r);
    tk = m_tok(r, a);
    if (MS8(NA(a, 0x9A)) == 2 && (tk >= 6 || (M8(NA(a, 0x31)) && tk >= 3))) {
        MW8(TSA(r - 1) + 2u, M8(TSA(r - 1) + 2u) | 2);
        MW8(TSA(r - 1), a);
        MW8(TSA(r - 1) + 1u, M8(0xF40FCu));
    }
    if (tk > 0 && a != 11) {
        MW8(PEND, 1);
        MW8(PATT, a);
        MW8(POWN, o);
        MW16(PREC, r);
    } else
        MW8(PEND, 0);
}

/* 0x5C110 */
static void m_matrix(int r)
{
    int i, j, o = M8(TSA(r - 1)), present_or_owner;
    int orig = g_mut == 3 ? !g_original : g_original;
    for (i = 0; i < 9; i++) {
        present_or_owner = M8(REC(r) + 2u + (uint32_t)i) != 0
                           || (m_city(r - 1) && o == (orig ? i : i + 1));
        if (!present_or_owner)
            continue;
        for (j = 0; j < 9; j++)
            if (j != i && M8(REC(r) + 2u + (uint32_t)j) != 0)
                MW8(MAT + 9u * (uint32_t)i + (uint32_t)j, 1);
    }
}

/* 0x5C1CC */
static int m_before(int t)
{
    int n;
    for (n = 1; n <= 9; n++)
        if (MS8(PREVA(t + 1) + 1u + (uint32_t)n) == n)
            return n;
    return -1;
}

/* 0x5BA0C */
static void m_plunder(int show)
{
    int a = MS8(PATT), o = MS8(POWN), amount, chosen, c, i, k, human;
    if (show) {
        MW8(0xEE168u, 3);   /* after the card draw in the listing; no data in between depends on it */
        if (o <= 9) {
            uint32_t orec = NA(o, 0);
            int32_t at;
            /* the count lives in memory right after the 32-byte list: bump it first, then store the byte (as the
               listing does), so an over-long hand behaves the same */
            MW32(0x1125A4u, 0);
            for (c = 1; c <= 18; c++)
                for (k = 0; k < MS8(orec + (uint32_t)c); k++) {
                    at = MS32(0x1125A4u);
                    MW32(0x1125A4u, at + 1);
                    MW8(0x112584u + (uint32_t)at, c);
                }
            if (MS32(0x1125A4u) > 0) {
                if (g_mut != 5)
                    for (i = 0; i < MS8(orec + 0x13u); i++) {
                        at = MS32(0x1125A4u);
                        MW32(0x1125A4u, at + 1);
                        MW8(0x112584u + (uint32_t)at, M8(orec + 0x14u + (uint32_t)i));
                    }
                c = (int8_t)M8(0x112584u + (((uint32_t)m_rand() * (uint32_t)MS32(0x1125A4u)) >> 15));
                m_card_from(c, orec);
                m_card_to(c, NA(a, 0), orec);
            }
        }
    }
    amount = MS8(NA(a, 0x99));
    if (amount > 3)
        amount = 3;
    if (amount <= 0)
        return;
    if (M8(0xE1624u) & 0x80)
        human = M8(PATT) == M8(0xE1622u);
    else
        human = MS8(NA(a, 0x9A)) == 3;
    if (human) {
        if (!show)
            return;
        MW32(0xF40F8u, a);
        chosen = 0;
        for (;;) {
            m_centre(MS16(PREC) - 1);
            mb();
            f3F670();
            f3F624(MS16(PREC) - 1, 15);
            me();
            c = m_code();
            if (c == -1)
                break;
            if (c == -12 && chosen < amount)
                chosen++;
            else if (c == -11 && chosen > 0)
                chosen--;
        }
        MADD8(NA(a, 0x99), -chosen);
        MADD8(NA(a, 0x3E), chosen);
        return;
    }
    if (MS8(NA(a, 0x9A)) == 2)
        logto(g_ai_model, sizeof g_ai_model, "5BF30(%d,%d,%d) ", a, amount, show);
}

/* 0x5AAA4 */
static void m_conflict(int show)
{
    int list[160], cnt = 0, r, n, i, j, pass, sum, saved;
    if (show) {
        int p = 0;
        MW16(0xBB3FAu, 0xA9);
        MW16(0xBB3FCu, 0x154);
        MW16(0xBB3FEu, 0x12C);
        MW16(0xBB400u, 0x14);
        for (n = 1; n <= 9; n++)
            if (M8(NA(n, 0x9A)))
                MW16(0xF06B0u + 2u * (uint32_t)p++, n);
        MW16(0xF06AEu, p);
        memset(m_ds + (MAT - DS_BASE), 0, 81);
    }
    for (r = 1; r <= 157; r++) {
        int nn = MS8(REC(r) + 1u), t = r - 1, own_empty;
        own_empty = m_city(t) && MS8(REC(r) + 0x0Eu + (uint32_t)M8(TSA(t))) == 0;
        if (!(nn > 1 || (nn == 1 && own_empty)))
            continue;
        for (sum = 0, n = 1; n <= 11; n++)
            sum += m_tok(r, n);
        if (sum > m_poplimit(t) || own_empty)
            list[cnt++] = r;
    }
    if (cnt > 0) {
        if (show)
            MW32(0xF40F8u, (M8(0xE1624u) & 0x80) ? MS8(0xE1622u) : MS16(0xF06B0u));
        for (pass = 0; pass < 2; pass++)
            for (i = 0; i < cnt; i++) {
                r = list[i];
                if (m_city(r - 1) != pass || r == 0)
                    continue;
                MW8(PEND, 0);
                MW8(POWN, 0);
                MW8(PATT, 0);
                memset(m_ds + (LOSST - DS_BASE), 0, 14);
                memset(m_ds + (LOSSC - DS_BASE), 0, 14);
                m_matrix(r);
                m_highlight(r - 1);
                if (pass == 1) {
                    m_attack(r);
                    list[i] = 0;
                } else if (MS8(REC(r) + 1u) >= 2) {
                    m_battle(r);
                    list[i] = 0;
                }
                if (show) {
                    MW8(0xEE168u, 3);   /* the background number, set before the screen is drawn */
                    if (g_mut != 6 && MS8(PREVA(r) + 1u) == 1) {
                        int a = m_before(r - 1), b = m_first(r - 1);
                        if (a > 0) {
                            if (m_tok(r, a) == 0) {
                                if (MS8(REC(r) + 0x3Bu + (uint32_t)b) == 0)
                                    MW8(REC(r) + 0x3Bu + (uint32_t)a, 1);
                                else
                                    MW8(REC(r) + 0x3Bu + (uint32_t)b, 0);
                            }
                        } else if (b <= 9)
                            MW8(REC(r) + 0x3Bu + (uint32_t)b, 0);
                    }
                    if ((M8(PREVT(r - 1) + 2u) & 1) && !m_city(r - 1)) {
                        int o = M8(PREVT(r - 1)), b = m_first(r - 1);
                        if (MS8(REC(r) + 0x3Bu + (uint32_t)b) == 0 && o <= 9)
                            MW8(REC(r) + 0x3Bu + (uint32_t)o, 1);
                        else
                            MW8(REC(r) + 0x3Bu + (uint32_t)b, 0);
                    }
                    m_centre(r - 1);
                    m_box();
                }
                if (M8(PEND)) {
                    saved = MS32(0xF40F8u);
                    m_plunder(show);
                    MW32(0xF40F8u, saved);
                }
            }
    }
    if (show)
        for (i = 0; i < 9; i++)
            for (j = 0; j < 9; j++) {
                uint32_t h = NA(i + 1, 0xA9 + j);
                int v;
                if (M8(MAT + 9u * (uint32_t)i + (uint32_t)j) == 1) {
                    v = MS8(h) + 1;
                    v = (int8_t)v;
                    MW8(h, v > (g_mut == 4 ? 4 : 5) ? (g_mut == 4 ? 4 : 5) : v);
                } else {
                    v = (int8_t)(M8(h) - 3);
                    MW8(h, v < 0 ? 0 : v);
                }
            }
}

/* ---- running both ---------------------------------------------------------------------------------------------- */

static uint32_t g_rs;
static int rnd(int n)
{
    g_rs = g_rs * 1664525u + 1013904223u;
    return (int)((g_rs >> 8) % (uint32_t)n);
}

static int g_firstdiff;
static int same_ds(void)
{
    uint32_t i;
    for (i = 0; i < DS_SIZE; i++)
        if (g_ds[i] != m_ds[i]) {
            g_firstdiff = (int)(i + DS_BASE);
            return 0;
        }
    return 1;
}

/* a random state on top of a base: conflicts made with the primitives, so the records stay consistent */
static void gen_state(const uint8_t *base)
{
    int n, i, k, t, m, o, j;
    int inplay[12], ninplay = 0;

    memcpy(g_ds, base, DS_SIZE);
    for (n = 1; n <= 9; n++)
        if (D8(NA(n, 0x9A)) != 0) {
            setD8(NA(n, 0x9A), rnd(3) ? 3 : 2);
            inplay[ninplay++] = n;
        }
    if (ninplay == 0)
        for (n = 1; n <= 3; n++) {
            setD8(NA(n, 0x9A), 3);
            inplay[ninplay++] = n;
        }
    for (n = 10; n <= 13; n++)
        setD8(NA(n, 0x9A), (uint8_t)(rnd(4) == 0 ? 2 * rnd(2) : 3));   /* the saves have 3 */
    for (n = 1; n <= 13; n++) {
        setD8(NA(n, 0x26), rnd(3) == 0);
        setD8(NA(n, 0x2A), rnd(3) == 0);
        setD8(NA(n, 0x31), rnd(3) == 0);
        setD8(NA(n, 0x35), rnd(4) == 0);
        setD8(NA(n, 0x99), (uint8_t)(rnd(25) - 2));
        setD8(NA(n, 0x3E), (uint8_t)rnd(20));
        for (i = 1; i <= 0x12; i++)
            setD8(NA(n, i), rnd(4) == 0 ? (uint8_t)rnd(3) : 0);
        k = rnd(3);
        setD8(NA(n, 0x13), (uint8_t)k);
        for (i = 0; i < 8; i++) {
            setD8(NA(n, 0x14 + i), i < k ? (uint8_t)(0x13 + rnd(12)) : 0);
            setD8(NA(n, 0x1C + i), i < k ? (uint8_t)rnd(10) : 0xFF);
        }
        for (j = 0; j < 9; j++)
            setD8(NA(n, 0xA9 + j), (uint8_t)rnd(6));
    }
    setD8(0xE1624u, (uint8_t)((D8(0xE1624u) & 0x7F) | (rnd(10) == 0 ? 0x80 : 0)));
    setD8(0xE1622u, (uint8_t)rnd(10));
    setD8(0xE1620u, (uint8_t)rnd(4));
    setD8(0xBB3F8u, (uint8_t)rnd(2));
    setD32(0xF40F8u, (uint32_t)rnd(10));
    setD32(0xCAC5Cu, (uint32_t)rnd(0x7FFFFFFF));
    for (i = 0; i < 81; i++)
        setD8(MAT + (uint32_t)i, rnd(4) == 0);
    setD8(PEND, (uint8_t)rnd(2));
    setD8(PATT, (uint8_t)rnd(12));
    setD8(POWN, (uint8_t)rnd(12));
    memcpy(DP(0xE8888u), DP(AREA0), 0x2B65);
    memcpy(DP(0xE2FD8u), DP(TS0), 0x1E0);
    m = 1 + rnd(5);
    for (i = 0; i < m; i++) {
        t = rnd(157);
        if ((D8(TSA(t) + 2u) & 1) == 0 && rnd(2)) {
            o = rnd(6) == 0 ? 10 : inplay[rnd(ninplay)];
            if (S8(NA(o, 0x51)) < 9 && S8(NA(o, 0x51)) >= 0)
                f78E74(t, o);
        }
        k = 1 + rnd(3);
        for (j = 0; j < k; j++) {
            n = rnd(8) == 0 ? 10 + rnd(2) : inplay[rnd(ninplay)];
            f57E28(t, n, 1 + rnd(9));
        }
        if ((D8(TSA(t) + 2u) & 1) && rnd(2)) {
            o = D8(TSA(t));
            k = S8(REC(t + 1) + 0x0Eu + (uint32_t)o);
            if (k > 0)
                f57CE8(t, o, 0, k);
        }
        /* the pre-movement copy of this territory */
        if (rnd(2)) {
            int nb = inplay[rnd(ninplay)];
            setD8(PREVA(t + 1) + 1u, (uint8_t)(1 + (rnd(3) == 0)));
            for (j = 1; j <= 11; j++)
                setD8(PREVA(t + 1) + 1u + (uint32_t)j, 0);
            setD8(PREVA(t + 1) + 1u + (uint32_t)nb, (uint8_t)nb);
        }
        if (rnd(3) == 0) {
            setD8(PREVT(t) + 2u, (uint8_t)(D8(PREVT(t) + 2u) | 1));
            setD8(PREVT(t), (uint8_t)(1 + rnd(10)));
        }
        for (j = 0; j < 10; j++)
            setD8(REC(t + 1) + 0x3Bu + (uint32_t)j, (uint8_t)rnd(2));
    }
}

static void run_one(const uint8_t *base, int show, int *fails, int report)
{
    char what[200];
    int rp, rm;
    uint32_t seed = (uint32_t)rnd(0x7FFFFFFF);

    gen_state(base);
    g_unattended = rnd(5) == 0;
    memcpy(m_ds, g_ds, DS_SIZE);
    g_port_codes.s = g_model_codes.s = seed;
    g_port_codes.used = g_model_codes.used = 0;
    g_nscript = 0;
    g_ai_port[0] = g_ai_model[0] = 0;
    g_rand_calls = m_rand_calls = 0;
    g_trace[0] = 0;
    f5AAA4(show);
    m_conflict(show);
    rp = same_ds();
    rm = rp && g_rand_calls == m_rand_calls && g_port_codes.used == g_model_codes.used
         && strcmp(g_ai_port, g_ai_model) == 0;
    if (!rm) {
        (*fails)++;
        if (report && *fails <= 5) {
            snprintf(what, sizeof what, "A show %d unattended %d original %d: ds %s (first 0x%X: port %02X model %02X), "
                     "rand %d/%d, codes %d/%d, AI [%s]/[%s]", show, g_unattended, g_original, rp ? "same" : "differs",
                     rp ? 0 : g_firstdiff, rp ? 0 : D8((uint32_t)g_firstdiff), rp ? 0 : m_ds[g_firstdiff - (int)DS_BASE],
                     g_rand_calls, m_rand_calls, g_port_codes.used, g_model_codes.used, g_ai_port, g_ai_model);
            check(0, what);
        }
    }
}

static int g_nbase;
static long g_stat_conflicts, g_stat_rand, g_stat_ai, g_stat_cities;

static void part_a(int trials)
{
    int i, fails = 0, mut;
    for (i = 0; i < trials; i++) {
        int before = fails;
        run_one(g_base[i % g_nbase], i & 1, &fails, 1);
        if (fails == before) {
            g_tests++;
            if (has("3A9F4[CONFLICT RESOLUTION"))
                g_stat_conflicts++;
            if (g_rand_calls)
                g_stat_rand++;
            if (g_ai_port[0])
                g_stat_ai++;
            if (has("3C9D0(310,385"))
                g_stat_cities++;
        }
    }
    printf("A. %d runs (original %d): %d differ; %ld with a losses box, %ld with a plunder draw, %ld with a computer's "
           "transfer, %ld with the human transfer screen\n", trials, g_original, fails, g_stat_conflicts, g_stat_rand,
           g_stat_ai, g_stat_cities);
    g_stat_conflicts = g_stat_rand = g_stat_ai = g_stat_cities = 0;
    /* the deliberately broken models must be caught */
    for (mut = 1; mut <= 7; mut++) {
        int caught = 0;
        g_mut = mut;
        for (i = 0; i < 3000 && caught == 0; i++)
            run_one(g_base[i % g_nbase], 1, &caught, 0);
        g_mut = 0;
        {
            char what[80];
            snprintf(what, sizeof what, "A broken model %d caught (original %d)", mut, g_original);
            check(caught > 0, what);
        }
    }
}

/* ---- B. scripted scenarios --------------------------------------------------------------------------------------- */

static void script(const int *codes, int n)
{
    memcpy(g_script, codes, sizeof(int) * (size_t)n);
    g_nscript = n;
    g_iscript = 0;
}

static void clear_conflicts(void)
{
    int r;
    /* every area record without tokens and every city removed, so only the scenario's territories matter */
    for (r = 1; r <= 160; r++)
        memset(DP(REC(r) + 1u), 0, 68);
    for (r = 0; r < 160; r++) {
        setD8(TSA(r), 0);
        setD8(TSA(r) + 1u, 0xFF);
        setD8(TSA(r) + 2u, 0);
    }
    for (r = 1; r <= 13; r++) {
        setD8(NA(r, 0x60), 0);
        setD8(NA(r, 0x51), 0);
    }
    memcpy(DP(0xE8888u), DP(AREA0), 0x2B65);
    memcpy(DP(0xE2FD8u), DP(TS0), 0x1E0);
}

static void part_b_box(void)
{
    int n, r;
    const int ret12[] = { -11, -0x50 }, ret13[] = { 5, -0x51 }, ret0[] = { -100, -1 };
    char exp[200];

    memcpy(g_ds, g_base[0], DS_SIZE);
    for (n = 1; n <= 13; n++) {
        setD8(NA(n, 0x9A), n <= 5 || n >= 10 ? 3 : 0);
        setD16(0xF48A0u + 8u * (uint32_t)n, 0x12);
        setD16(0xF4980u + 8u * (uint32_t)n, 0x0F);
    }
    memset(DP(LOSST), 0, 14);
    memset(DP(LOSSC), 0, 14);
    /* five columns: 1, 3, 11, 12, 13 (6 is not in play: not counted) */
    setD8(LOSST + 1u, 4); setD8(LOSSC + 3u, 1); setD8(LOSST + 11u, 2); setD8(LOSST + 12u, 1); setD8(LOSSC + 13u, 2);
    setD8(LOSST + 6u, 9);
    g_unattended = 0;
    for (g_original = 0; g_original <= 1; g_original++) {
        g_trace[0] = 0;
        script(ret0, 2);
        r = f5CDE8(0, "TITLE");
        check(r == 0, "B box: -1 returns 0");
        check(strncmp(g_trace, "3A924 3A9F4[TITLE] vbopen(", 26) == 0, "B box: title first");
        check(has("vbcopy(170,469,376,418,170,418,"), "B box: the area restored from the background");
        /* colw = 300 / 5 = 60, x0 = 170; AFRICA 48 < 60 full; BARBARIAN 72 >= 60; FIRST FACTION 104 >= 60 */
        check(has("3A0B0(170,365,60,[AFRICA]) 3AF8C(F48A4,191,376) 3A0B0(171,396,60,[4]) 3AF8C(F4984,192,407) "
                  "3A0B0(171,424,60,[0])"), "B box: column 1");
        check(has("3A0B0(230,365,60,[ILLYRIA])"), "B box: column 2 (ILLYRIA 56 < 60)");
        snprintf(exp, sizeof exp, "3A0B0(290,365,60,[%s])", g_original ? "AFRICA" : "BARBARIAN");
        check(has(exp), "B box: BUG 2, the Barbarians' short name");
        snprintf(exp, sizeof exp, "3A0B0(350,365,60,[%s])", g_original ? "IBERIA" : "FIRST FACTION");
        check(has(exp), "B box: BUG 2, the first faction");
        snprintf(exp, sizeof exp, "3A0B0(410,365,60,[%s]) 3AF8C(F4904,431,376) 3A0B0(411,396,60,[0]) 3AF8C(F49E4,432,407) "
                 "3A0B0(411,424,60,[2])", g_original ? "ILLYRIA" : "SECOND FACTION");
        check(has(exp), "B box: column 5");
        check(!has("[9]"), "B box: a nation not in play has no column");
        check(has("vbclose 3CBBC 3D52C(0,0) 7BA64 78BA4(F596C,0)=-100 7BA64 78BA4(F596C,0)=-1 "), "B box: the wait");
    }
    g_original = 0;
    script(ret12, 2);
    check(f5CDE8(0, "T") == 12, "B box: -0x50 returns 12");
    script(ret13, 2);
    g_trace[0] = 0;
    check(f5CDE8(5, "T") == 13 && has("ANIM7A4C4(5) 7BA64"), "B box: -0x51 returns 13, the animation before the wait");
    /* two nations: colw 100, x0 220 */
    memset(DP(LOSST), 0, 14);
    memset(DP(LOSSC), 0, 14);
    setD8(LOSST + 2u, 1);
    setD8(LOSST + 4u, 3);
    g_unattended = 1;
    setD16(0xF40BAu, 0);
    g_nscript = 0;
    g_trace[0] = 0;
    check(f5CDE8(0, "T") == 0, "B box: unattended returns 0");
    check(has("3A0B0(220,365,100,[IBERIA])") && has("3A0B0(320,365,100,[THRACE])"), "B box: two columns at 220");
    check(has("7BA64 78A20(250) ") && !has("78BA4"), "B box: unattended 250 ms");
    setD16(0xF40BAu, 3);
    g_trace[0] = 0;
    f5CDE8(0, "T");
    check(has("7BA64 78A20(1000) "), "B box: the demo's 1000 ms");
    setD16(0xF40BAu, 0);
    /* nobody: the original divides by 0; the port draws nothing */
    memset(DP(LOSST), 0, 14);
    g_trace[0] = 0;
    f5CDE8(0, "T");
    check(!has("3A0B0") && has("78A20(250)"), "B box: no losses, no column, no fault");
    g_unattended = 0;
}

/* territory t: tokens of n */
static void put(int t, int n, int k) { f57E28(t, n, k); }

static void part_b_plunder(void)
{
    int r, t = 20, a = 1, o = 2;
    const int clicks[] = { -1, -12, -100, -12, -12, -12, -11, 5, -1 };
    const int one[] = { -12, -12, -1 };
    char exp[300];

    for (g_original = 0; g_original <= 1; g_original++) {
        memcpy(g_ds, g_base[0], DS_SIZE);
        clear_conflicts();
        setD8(0xE1624u, (uint8_t)(D8(0xE1624u) & 0x7F));
        setD8(NA(a, 0x9A), 3);
        setD8(NA(o, 0x9A), 3);
        setD8(NA(a, 0x99), 5);
        setD8(NA(a, 0x3E), 2);
        setD8(NA(o, 0x99), 10);
        setD8(NA(a, 0x2A), 0);
        setD8(NA(o, 0x2A), 0);
        setD8(NA(a, 0x26), 0);
        setD8(NA(o, 0x26), 0);
        memset(DP(NA(o, 1)), 0, 0x12);
        setD8(NA(o, 3), 1);        /* one card of id 3 */
        setD8(NA(o, 0x13), 1);     /* one calamity 0x15 */
        setD8(NA(o, 0x14), 0x15);
        memset(DP(NA(a, 1)), 0, 0x1B);
        f78E74(t, o);
        put(t, a, 8);              /* 8 >= 7: the city falls, 6 of o's stock replace it, a battle */
        setD8(0xBB3F8u, 1);
        setD32(0xCAC5Cu, 12345);
        g_rand_calls = 0;
        g_trace[0] = 0;
        script(clicks, 9);
        f5AAA4(1);
        r = t + 1;
        check(g_rand_calls == 1, "B plunder: one game_rand");
        {
            uint32_t s = 12345u * 0x15A4E35u + 1u;
            int idx = (int)((((s >> 16) & 0x7FFF) * 2u) >> 15);   /* two cards: [3, 0x15] */
            int got3 = D8(NA(a, 3)) == 1, gotc = S8(NA(a, 0x13)) == 1 && D8(NA(a, 0x14)) == 0x15 && D8(NA(a, 0x1C)) == D8(NA(o, 0x3D));
            check(idx == 0 ? got3 && D8(NA(o, 3)) == 0 : gotc && S8(NA(o, 0x13)) == 0, "B plunder: the drawn card moved");
            snprintf(exp, sizeof exp, "%s gets a random trade good from %s as plunder.", "AFRICA", "IBERIA");
            check(has(exp), "B plunder: the message");
            printf("B. plunder draw: index %d of 2 (%s)\n", idx, idx == 0 ? "the commodity card" : "the calamity");
        }
        check(D8(LOSSC + (uint32_t)o) == 1, "B plunder: the city counted");
        check((D8(TSA(t) + 2u) & 1) == 0, "B plunder: the city removed");
        check(has("3C9D0(310,385,19,19,5) setcolor(5) 3A0B0(310,389,19,[0])"), "B plunder: the box with 0");
        check(has("you may transfer up to 3 units from stock to your treasury."), "B plunder: the text");
        check(has("vbopen(") && has("3AF8C(F51AC,296,385) 3AF8C(F51B4,330,385) 3A3A0(293,389,[UNITS TO]) "
              "3A044(346,389,[TRANSFER]) vbclose "), "B plunder: arrows and labels");
        check(has("7853C(C61C4) on(F596C,7) 3B05C(F596C) "), "B plunder: the dialog");
        check(has("3A0B0(310,389,19,[3])") && !has("3A0B0(310,389,19,[4])"), "B plunder: up to 3 only");
        check(S8(NA(a, 0x3E)) == 4, "B plunder: treasury + 2");
        check(has("459E4(1) 3CBBC 78A20(1000) 78664(C61C4)"), "B plunder: the end");
        check(S32(0xF40F8u) == S16(0xF06B0u), "B plunder: [0xF40F8] restored");
        check(S8(REC(r) + 0x0Eu + (uint32_t)o) == 0
              || S8(REC(r) + 0x0Eu + (uint32_t)o) + S8(REC(r) + 0x0Eu + (uint32_t)a) <= ((D8(0xE1638u + 35u * (uint32_t)t + 1u) >> 3) & 7),
              "B plunder: the battle after the replacement ends at the limit or with no defenders");
        /* one unit */
        memcpy(g_ds, g_base[0], DS_SIZE);
        clear_conflicts();
        setD8(0xE1624u, (uint8_t)(D8(0xE1624u) & 0x7F));
        setD8(NA(a, 0x9A), 3);
        setD8(NA(a, 0x99), 1);
        setD8(PEND, 1); setD8(PATT, (uint8_t)a); setD8(POWN, 10); setD16(PREC, 5);
        g_trace[0] = 0;
        g_rand_calls = 0;
        script(one, 3);
        f5BA0C(1);
        check(has("AFRICA, you may transfer 1 unit from stock to your treasury.") && g_rand_calls == 0,
              "B plunder: one unit, the Pirates have no cards");
        check(S8(NA(a, 0x99)) == 0, "B plunder: one unit moved");
        /* no stock */
        setD8(NA(a, 0x99), 0);
        g_trace[0] = 0;
        f5BA0C(1);
        snprintf(exp, sizeof exp, "79270(2500,1,-200,[AFRICA,%syou have no stock to pillage with.],0)", g_original ? "" : " ");
        check(has(exp), "B plunder: no stock (the text fix)");
        /* a human with show == 0 */
        setD8(NA(a, 0x99), 3);
        g_trace[0] = 0;
        f5BA0C(0);
        check(g_trace[0] == 0 && S8(NA(a, 0x99)) == 3, "B plunder: a human without show gets nothing");
        /* a computer */
        setD8(NA(a, 0x9A), 2);
        g_trace[0] = 0;
        f5BA0C(0);
        check(has("AI5BF30(1,3,0)"), "B plunder: the computer goes to 0x5BF30");
    }
    g_original = 0;
    g_nscript = 0;
}

static void part_b_phase(void)
{
    int t = 30, r = 31;
    const int fin[] = { -1 };
    char exp[200];

    /* nothing contested */
    memcpy(g_ds, g_base[0], DS_SIZE);
    clear_conflicts();
    g_trace[0] = 0;
    f5AAA4(1);
    snprintf(exp, sizeof exp, "79270(100,%d,-200,[],0) on(F596C,7) ", S16(0xF06B0u));
    check(has(exp) && !has("3A9F4"), "B phase: the empty message");
    check(D16(0xBB400u) == 0x14 && D16(0xBB3FAu) == 0xA9, "B phase: the message area");
    g_trace[0] = 0;
    f5AAA4(0);
    check(strcmp(g_trace, "on(F596C,7) ") == 0, "B phase: show 0, nothing: only button 7");

    /* BUG 1: a city of 3 with no defenders, nations 1 and 5 next to it (no battle needed: only nation 1 there) */
    for (g_original = 0; g_original <= 1; g_original++) {
        memcpy(g_ds, g_base[0], DS_SIZE);
        clear_conflicts();
        setD8(0xE1624u, (uint8_t)(D8(0xE1624u) & 0x7F));
        setD8(0xE1620u, 2);
        setD8(NA(1, 0x9A), 3);
        setD8(NA(3, 0x9A), 3);
        setD8(NA(1, 0x2A), 0);
        setD8(NA(3, 0x2A), 0);
        setD8(NA(1, 0x99), 0);
        f78E74(t, 3);
        put(t, 1, 3);                         /* 3 < 7: the attack fails */
        memset(DP(MAT), 0, 81);
        memset(DP(NA(3, 0xA9)), 2, 9);
        memset(DP(NA(4, 0xA9)), 2, 9);
        g_trace[0] = 0;
        script(fin, 1);
        f5AAA4(1);
        check(D8(MAT + 9u * 0u + 2u) == 0 && D8(MAT + 9u * (g_original ? 3u : 2u) + 0u) == 1
              && D8(MAT + 9u * (g_original ? 2u : 3u) + 0u) == 0, "B phase: BUG 1, the owner's row");
        check(S8(NA(g_original ? 4 : 3, 0xA9)) == 3 && S8(NA(g_original ? 3 : 4, 0xA9)) == 0,
              "B phase: BUG 1, the hostility follows the row");
        check(S8(REC(r) + 0x0Eu + 1u) == 0 && D8(LOSST + 1u) == 3 && S8(NA(1, 0x99)) == 3, "B phase: the failed attack");
        check((D8(TSA(t) + 2u) & 1) == 1, "B phase: the city stands");
        check(has("3CD6C(3) vbcopy(0,639,0,479,0,479,") && has("7876C 7853C(C5214) off(F596C,7) 3B05C(F596C) 3CBBC "),
              "B phase: the screen");
        snprintf(exp, sizeof exp, "7844C(F596C,%d) 3A924 3A9F4[CONFLICT RESOLUTION - UNITS REMOVED] ", S32(0xF5940u));
        check(has(exp), "B phase: the losses box after the map");
        check(has("78BA4(F596C,0)=-1 78664(C5214) on(F596C,7) "), "B phase: FINISHED, the dialog removed, button 7");
        check(!has("79270(100"), "B phase: no empty message");
        check(S32(0xF40F8u) == S16(0xF06B0u), "B phase: the first player is current");
    }
    g_original = 0;
    /* the guard of 0x5B224: a defenceless city of 3 with only the second faction's tokens there (marker +0x0E = 13,
       which a "nation 0" attacker would read as 13 tokens): no attack, in both modes */
    for (g_original = 0; g_original <= 1; g_original++) {
        memcpy(g_ds, g_base[0], DS_SIZE);
        clear_conflicts();
        setD8(0xE1624u, (uint8_t)(D8(0xE1624u) & 0x7F));
        setD8(NA(3, 0x9A), 3);
        setD8(NA(3, 0x99), 10);
        f78E74(t, 3);
        put(t, 13, 2);
        g_rand_calls = 0;
        g_trace[0] = 0;
        script(fin, 1);
        f5AAA4(1);
        check((D8(TSA(t) + 2u) & 1) == 1 && D8(TSA(t)) == 3 && D8(LOSSC + 3u) == 0 && D8(PEND) == 0
              && S8(REC(r) + 0x0Eu + 3u) == 0 && S8(NA(3, 0x99)) == 10 && g_rand_calls == 0,
              "B phase: only a faction on a city, no attack (the guard)");
        check(has("3A9F4[CONFLICT RESOLUTION - UNITS REMOVED] ") && !has("3A0B0(") && has("78BA4(F596C,0)=-1 "),
              "B phase: the guard's losses box has no column");
    }
    g_original = 0;
    /* the hostility limits: 5 stays 5 on a meeting, 1 goes to 0 without */
    memcpy(g_ds, g_base[0], DS_SIZE);
    clear_conflicts();
    setD8(NA(1, 0x9A), 3);
    setD8(NA(2, 0x9A), 3);
    put(t, 1, 4);
    put(t, 2, 4);                             /* 8 > the limit: a battle */
    memset(DP(MAT), 0, 81);
    setD8(NA(1, 0xA9 + 1), 5);
    setD8(NA(2, 0xA9 + 0), 4);
    setD8(NA(1, 0xA9 + 2), 1);
    script(fin, 1);
    f5AAA4(1);
    check(S8(NA(1, 0xA9 + 1)) == 5 && S8(NA(2, 0xA9 + 0)) == 5 && S8(NA(1, 0xA9 + 2)) == 0, "B phase: hostility limits");
    check(D8(MAT + 1u) == 1 && D8(MAT + 9u) == 1, "B phase: the matrix of the battle");
    g_nscript = 0;
}

/* ---- C. the saves ------------------------------------------------------------------------------------------------ */

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

static void part_c_one(const char *path, int keep)
{
    char desc[64], buf[200];
    int r, n, cnt = 0, fails = 0;
    if (load_save(path, desc) < 0) {
        printf("C. %s not found: skipped\n", path);
        return;
    }
    printf("C. %s: \"%s\", phase %d, round %d, humans %d, computers %d; in play (+0x9A) of 10..13: %d %d %d %d\n", path,
           desc, D8(0xF4102u), S16(0xF40FCu), S8(0xE1620u), S8(0xE1621u), D8(NA(10, 0x9A)), D8(NA(11, 0x9A)),
           D8(NA(12, 0x9A)), D8(NA(13, 0x9A)));
    for (r = 1; r <= 157; r++) {
        int nn = S8(REC(r) + 1u), city = D8(TSA(r - 1) + 2u) & 1;
        if (nn > 1 || (city && nn == 1)) {
            printf("   area %d %s: %d nation(s)%s", r, DSTR(D32(0xC77D8u + 4u * (uint32_t)(r - 1))), nn,
                   city ? " city" : "");
            for (n = 1; n <= 11; n++)
                if (S8(REC(r) + 0x0Eu + (uint32_t)n))
                    printf(" %d:%d", n, S8(REC(r) + 0x0Eu + (uint32_t)n));
            printf(" (limit %d)\n", (D8(0xE1638u + 35u * (uint32_t)(r - 1) + 1u) >> 3) & 7);
            cnt++;
        }
    }
    printf("   %d areas with two nations or a foreign token on a city\n", cnt);
    for (r = 0, n = 0; r <= 160; r++)
        n += D8(REC(r)) != (unsigned)r;
    check(n == 0, "C. every area record's byte +0 is its own number");
    if (keep >= 0)
        memcpy(g_base[keep], g_ds, DS_SIZE);
    /* the phase on the save itself, against the model */
    memcpy(m_ds, g_ds, DS_SIZE);
    g_unattended = 0;
    g_port_codes.s = g_model_codes.s = 7;
    g_port_codes.used = g_model_codes.used = 0;
    g_ai_port[0] = g_ai_model[0] = 0;
    g_rand_calls = m_rand_calls = 0;
    g_trace[0] = 0;
    f5AAA4(1);
    m_conflict(1);
    snprintf(buf, sizeof buf, "C. f5AAA4(1) on %s equals the model", path);
    if (!same_ds())
        fails++;
    check(fails == 0 && g_rand_calls == m_rand_calls, buf);
    printf("   f5AAA4(1): %s, %d game_rand\n", has("3A9F4[CONFLICT") ? "conflicts resolved" : "nothing contested",
           g_rand_calls);
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:/work/advciv_run";
    const char *savedir = argc > 2 ? argv[2] : "C:/work/advciv_play";
    char cwd[400], path[400];
    int i;

    if (state_init(gamedir) != 0)
        return 1;
    if (getcwd(cwd, sizeof cwd) == NULL)
        cwd[0] = 0;
    if (chdir(gamedir) == 0) {   /* terrflag.dat: the territory records are not in the save */
        f12394();
        (void)chdir(cwd);
    }
    m_ds = malloc(DS_SIZE);
    for (i = 0; i < 4; i++)
        g_base[i] = malloc(DS_SIZE);
    if (m_ds == NULL || g_base[3] == NULL)
        return 1;
    for (i = 0; i < 4; i++)
        memcpy(g_base[i], g_ds, DS_SIZE);
    g_nbase = 1;

    /* C first: the saves become the bases of A and B */
    snprintf(path, sizeof path, "%s/savgame1.dat", gamedir);
    part_c_one(path, 0);
    snprintf(path, sizeof path, "%s/savgame0.dat", gamedir);
    part_c_one(path, 1);
    snprintf(path, sizeof path, "%s/SAVGAMEA.DAT", savedir);
    part_c_one(path, 2);
    snprintf(path, sizeof path, "%s/savgameb.dat", savedir);
    part_c_one(path, 3);
    g_nbase = 4;
    printf("C. %d fail so far\n", g_fail);

    g_rs = 20261119u;
    part_a(40000);
    g_original = 1;
    part_a(10000);
    g_original = 0;
    printf("A. %d fail so far\n", g_fail);

    part_b_box();
    part_b_plunder();
    part_b_phase();
    printf("B. %d fail so far\n", g_fail);

    printf("%d checks, %d fail\n", g_tests, g_fail);
    return g_fail == 0 ? 0 : 1;
}
