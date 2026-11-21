/* aicity_test [GAMEDIR [SAVEDIR]]: unit tests for module AICITY (aicity.c, M7 batch 1) that need no game run.

   GAMEDIR (default C:\work\advciv_run, only read) holds AC_MAIN.EXE (the data object, state_init), terrflag.dat (the
   territory records: f12394), LANDADJ.DAT (the land lists, put into one heap block with a handle per territory at
   0xF8E7C + 4t as f3EED8 does), N.RL (the ship routes for the save's player count, loaded into 0xFFD44 / 0x101DC8 /
   0x107410 / 0x107550 and [0x10740C] as f3EED8 does) and savgame0.dat (start of MOVEMENT R4); SAVEDIR (default
   C:\work\advciv_play, only read) holds SAVGAMEA.DAT (the R4 autosave, seven computers). The saves are read with
   saveload.c's f5F05C.

   aicity.c is compiled with -DAICITY_TEST: its fifteen calls into the rest of the batch (AIQ, AISEA, AIPLAN) come to
   the recorders below. Each recorder appends a line (its arguments and results) to the trace of the side that runs,
   and acts out a small deterministic effect computed from the data object (lists from the real land lists and the
   board, scores from 0xEED84, a token moved with f57CA4, a need cut, a flag set), so the paths behind them are
   exercised. The primitives the module calls are the real, separately tested ones: stack.c / mstack.c (both stacks
   and the snapshots), rules.c / move.c (f78D08, f7C7C4, f57CA4, f460C0, f46158), route.c (f5843C, f58888 on the real
   N.RL), helpers2.c (f3530C), wqsort.c (Watcom's qsort).

   The model is a second implementation of the nine routines written from the disassembly (frame-offset names, the
   original's labels and stage order; not from aicity.c). Each case: a base state (a save, randomised: tokens through
   f57E28, movable counts, advances, treasuries, ships on coastal territories, ranks, relations, the AI table
   0x112A94, 0xF488C, 0xEFA04, 0xEED84, the failure cache 0xF8558, 0xF3580, planned sites, the shadow tables, the
   flags word, [0x107ED8] / [0x107EDC] / [0x107EE0], the move-list counts, [0xBB3F0], [0xF5908]); the routine under
   test runs on it through aicity.c, the result is kept, the base is restored and the model runs. The whole data
   object, the return value, the out-parameters and the two traces must be equal.
     A. random differential runs of each routine (f337C8, f33ECC, f3433C, f348B0, f3566C, f353C4, f3580C modes 0/1/2,
        f33964 modes 0/1/2, f79128), with and without g_original (no routine reads it but the fixed garbage path);
     B. hand vectors: f337C8 worked out by hand, the 0xC8 / 0xC9 records (+3 not written, mode 2 at 4 * count: the
        inventory's "unscaled index" claim refuted), f79128 on the real map, the garbage fix of f33964;
     C. mutation check: twelve deliberately broken copies of the model (g_mut) must each be caught by part A's runs.
   Coverage counters of the model's paths are printed. Exits 0 when all pass.

   Build (from the repo root; artefacts in the scratchpad):
     clang -std=c11 -Wall -Wextra -O1 -ffunction-sections -Wl,--gc-sections -DAICITY_TEST -Iport/src port/test/aicity_test.c port/src/aicity.c port/src/stack.c
       port/src/mstack.c port/src/rules.c port/src/move.c port/src/mapdata.c port/src/route.c port/src/helpers2.c port/src/wqsort.c
       port/src/load.c port/src/saveload.c port/src/state.c port/src/le_image.c -o aicity_test.exe */

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <direct.h>
#define chdir _chdir
#define getcwd _getcwd

#include "../src/state.h"
#include "../src/game.h"
#include "../src/aicity.h"
#include "../src/helpers2.h"
#include "../src/mstack.h"
#include "../src/route.h"
#include "../src/rules.h"
#include "../src/move.h"
#include "../src/saveload.h"
#include "../src/stack.h"
#include "../src/wqsort.h"
#include "../src/fg.h"
#include "../src/junk.h"
#include "../src/map.h"
#include "../src/shippanel.h"
#include "../src/ui.h"

/* ---- checks ------------------------------------------------------------------------------------------------------ */

static int g_tests, g_fail;

static int check(int ok, const char *what)
{
    g_tests++;
    if (!ok) {
        g_fail++;
        printf("FAIL %s\n", what);
    }
    return ok;
}

/* ---- stubs for stack.c (none may be called: the depths stay inside the limits) ---------------------------------- */

static jmp_buf g_quit_jb;
static int g_quits;

_Noreturn void f44CAC(void)
{
    g_quits++;
    printf("f44CAC: %s\n", DSTR(0xF5500u));
    longjmp(g_quit_jb, 1);
}
void f3AF54(uint32_t spr) { (void)spr; }
void f3CC68(void) {}
void f3CCBC(void) {}
void f5E7D0(void) {}
void f3A990(void) {}
void f3C284(uint32_t dlg, int id, uint32_t label) { (void)dlg; (void)id; (void)label; }
void f3C314(uint32_t dlg, int id) { (void)dlg; (void)id; }
void f3C7A0(uint32_t dlg) { (void)dlg; }
void f3C828(uint32_t dlg) { (void)dlg; }
int f5FF28(void) { return -15; }
void f78884(void) {}
void f788AC(void) {}
int junk_makegif(int minx, int maxx, int miny, int maxy, const char *name)
{
    (void)minx; (void)maxx; (void)miny; (void)maxy; (void)name;
    return 0;
}
_Noreturn void port_longjmp(uint32_t jmpbuf) { (void)jmpbuf; longjmp(g_quit_jb, 2); }
/* the screen side of move.c / mapdata.c / stack.c / saveload.c (never reached by these routines) */
static long g_ui_calls;
void f3A780(const char *msg) { (void)msg; g_ui_calls++; }
void f3AB7C(const char *text) { (void)text; g_ui_calls++; }
void f3B7CC(uint32_t dlg, int id) { (void)dlg; (void)id; g_ui_calls++; }
void f3B870(uint32_t dlg, int id) { (void)dlg; (void)id; g_ui_calls++; }
int f3B9A4(uint32_t dlg, int id) { (void)dlg; (void)id; g_ui_calls++; return 0; }
void f3BFBC(uint32_t dlg, int id, int enabled) { (void)dlg; (void)id; (void)enabled; g_ui_calls++; }
void f3C3D0(uint32_t dlg, int id) { (void)dlg; (void)id; g_ui_calls++; }
void f3CBBC(void) { g_ui_calls++; }
void f3D52C(int dx, int dy) { (void)dx; (void)dy; g_ui_calls++; }
void f426D0(int i) { (void)i; g_ui_calls++; }
void f572CC(int t) { (void)t; g_ui_calls++; }
void f76F70(int n, uint8_t *cards) { (void)n; (void)cards; g_ui_calls++; }
void f787F8(int t) { (void)t; g_ui_calls++; }
int f78AE4(void) { g_ui_calls++; return 0; }
int f78BA4(uint32_t dlg, int map) { (void)dlg; (void)map; g_ui_calls++; return -1; }
void fg_move(int x, int y) { (void)x; (void)y; g_ui_calls++; }
void fg_vbclose(void) { g_ui_calls++; }
int fg_vbopen(int handle) { (void)handle; g_ui_calls++; return 0; }
int junk_showgif(const char *name, int flags) { (void)name; (void)flags; g_ui_calls++; return 0; }

/* ---- the traces --------------------------------------------------------------------------------------------------- */

#define TRSZ (1u << 20)
typedef struct {
    char *buf;
    size_t len;
    uint32_t hash;
    long lines;
} trace_t;
static trace_t g_tr[2];   /* 0 port, 1 model */
static int g_side;

static void tr(const char *fmt, ...)
{
    char line[2048];
    trace_t *t = &g_tr[g_side];
    va_list ap;
    int k, i;
    va_start(ap, fmt);
    k = vsnprintf(line, sizeof line, fmt, ap);
    va_end(ap);
    if (k < 0)
        return;
    if (k >= (int)sizeof line)
        k = (int)sizeof line - 1;
    for (i = 0; i < k; i++)
        t->hash = (t->hash ^ (uint8_t)line[i]) * 16777619u;
    t->hash = (t->hash ^ '\n') * 16777619u;
    t->lines++;
    if (t->len + (size_t)k + 2 < TRSZ) {
        memcpy(t->buf + t->len, line, (size_t)k);
        t->len += (size_t)k;
        t->buf[t->len++] = '\n';
        t->buf[t->len] = 0;
    }
}

static int g_n48028[2];   /* f48028's calls in this run, per side (the same sequence on both) */

static void tr_reset(void)
{
    int s;
    for (s = 0; s < 2; s++) {
        g_n48028[s] = 0;
        g_tr[s].len = 0;
        g_tr[s].buf[0] = 0;
        g_tr[s].hash = 2166136261u;
        g_tr[s].lines = 0;
    }
}

/* ---- records, written out here from the listing ----------------------------------------------------------------- */

#define NAT(n)   (0xF14CCu + 199u * (uint32_t)(n))
#define AREA(r)  (0xE31B8u + 69u * (uint32_t)(r))
#define TERR(t)  (0xE1638u + 35u * (uint32_t)(t))
#define TST(t)   (0xE2C18u + 3u * (uint32_t)(t))

static int tok(int t, int n) { return S8(AREA(t + 1) + 0x0Eu + (uint32_t)n); }
static int mov(int t, int n) { return S8(AREA(t + 1) + 0x24u + (uint32_t)n); }

/* the land lists as read from LANDADJ.DAT (not pruned; the data object's handles point at the same lists) */
static int g_adj_n[160], g_adj[160][40];

/* ---- recorders: the calls into AIQ / AISEA / AIPLAN ------------------------------------------------------------- */

static void tr_list(const char *head, const int32_t *l, int32_t c)
{
    char b[1800];
    size_t len = 0;
    int i;
    b[0] = 0;
    for (i = 0; i < c && i < 160 && len + 8 < sizeof b; i++)
        len += (size_t)snprintf(b + len, sizeof b - len, " %d", l[i]);
    tr("%s ->%d:%s", head, c, b);
}

void aicity_test_f48028(int32_t *cnt, int32_t *list, int t, int n, int filter)
{
    char h[80];
    int i, u, ok;
    *cnt = 0;
    if (t >= 0 && t < 160)
        for (i = 0; i < g_adj_n[t]; i++) {
            u = g_adj[t][i];
            if (u < 0 || u > 156)
                continue;
            if (filter == 9)   /* n alone; on every second call also an area n shares (a list that changes) */
                ok = (S8(AREA(u + 1) + 1u) == 1 || (g_n48028[g_side] % 2 == 1 && S8(AREA(u + 1) + 1u) == 2)) &&
                     S8(AREA(u + 1) + 1u + (uint32_t)n) == n && !(D8(TST(u) + 2u) & 2);
            else if (filter == 0x15)
                ok = mov(u, n) > 0 && S8(AREA(u + 1) + 1u) >= 2;
            else
                ok = 1;
            if (ok)
                list[(*cnt)++] = u;
        }
    g_n48028[g_side]++;
    snprintf(h, sizeof h, "48028 t%d n%d f%X", t, n, filter);
    tr_list(h, list, *cnt);
}

void aicity_test_f4892C(int t, int32_t *cnt, int32_t *out, int count, int32_t *list, int n, int filter)
{
    char h[80];
    int i, j, k, u, v, dup;
    *cnt = 0;
    for (i = 0; i < count; i++) {
        v = list[i];
        if (v < 0 || v > 156)
            continue;
        for (j = 0; j < g_adj_n[v]; j++) {
            u = g_adj[v][j];
            if (u < 0 || u > 156 || u == t)
                continue;
            dup = 0;
            for (k = 0; k < count; k++)
                dup |= list[k] == u;
            for (k = 0; k < *cnt; k++)
                dup |= out[k] == u;
            if (dup)
                continue;
            if (filter == 9 && !(S8(AREA(u + 1) + 1u) == 1 && S8(AREA(u + 1) + 1u + (uint32_t)n) == n))
                continue;
            if (*cnt < 160)
                out[(*cnt)++] = u;
        }
    }
    snprintf(h, sizeof h, "4892C t%d c%d n%d f%X", t, count, n, filter);
    tr_list(h, out, *cnt);
}

void aicity_test_f48D44(int32_t *cnt, int32_t *list, int n, int t, int flags)
{
    char h[80];
    int i, j, x;
    for (i = 0; i < *cnt; i++)   /* stable: bubble, descending by the score 0xEED84[n][u] */
        for (j = 0; j + 1 < *cnt - i; j++)
            if (S8(0xEED84u + 160u * (uint32_t)n + (uint32_t)list[j + 1]) >
                S8(0xEED84u + 160u * (uint32_t)n + (uint32_t)list[j])) {
                x = list[j];
                list[j] = list[j + 1];
                list[j + 1] = x;
            }
    snprintf(h, sizeof h, "48D44 n%d t%d f%X", n, t, flags);
    tr_list(h, list, *cnt);
}

int aicity_test_f7C838(int t, int n, int k, int32_t *left)
{
    *left = S8(AREA(t + 1) + 1u) >= 2 ? ((t + n + D8(0x112A94u + (uint32_t)t)) & 1) : 1;
    tr("7C838 t%d n%d k%d -> %d", t, n, k, *left);
    return 0;
}

int aicity_test_f495C0(int t, int n, int mode, int32_t *enemy)
{
    int i, m, u, v, best = 0;
    for (i = 0; t >= 0 && t < 160 && i < g_adj_n[t]; i++) {
        u = g_adj[t][i];
        if (u < 0 || u > 156)
            continue;
        for (m = 1; m <= 11; m++) {
            if (m == n)
                continue;
            if (mode == 1 && m <= 9 && !(S8(0xF06C0u + (uint32_t)m) > S8(0xF06C0u + (uint32_t)n)))
                continue;
            v = tok(u, m);
            if (v > best) {
                best = v;
                *enemy = m;
            }
        }
    }
    tr("495C0 t%d n%d m%d -> %d e%d", t, n, mode, best, *enemy);
    return best;
}

int aicity_test_f49790(int r, int t, int n, int enemy)
{
    int lim = (D8(TERR(t) + 1u) >> 3) & 7, thr, v, res;
    if (r == 0)
        res = 0;
    else {
        thr = (lim - 1 > 1 ? lim - 1 : 1) + 1;
        v = tok(t, n);
        res = r < thr ? 1 : (v < lim ? v : lim);
    }
    tr("49790 r%d t%d n%d e%d -> %d", r, t, n, enemy, res);
    return res;
}

int aicity_test_f49934(int t, int n, int mode)
{
    int i, u, res = 0;
    for (i = 0; t >= 0 && t < 160 && i < g_adj_n[t]; i++) {
        u = g_adj[t][i];
        if (u >= 0 && u <= 156 && tok(u, n) > 0)
            res = 1;
    }
    tr("49934 t%d n%d m%d -> %d", t, n, mode, res);
    return res;
}

void aicity_test_f7701C(int len, uint8_t *buf, int k)
{
    uint8_t tmp[200];
    int i, rot;
    char b[800];
    size_t l = 0;
    if (len <= 0 || len > 200)
        return;
    rot = (int)(D16(0x112A94u + 2u * (uint32_t)((k - 1) % 10)) % (unsigned)len);
    for (i = 0; i < len; i++)
        tmp[i] = buf[(i + rot) % len];
    memcpy(buf, tmp, (size_t)len);
    b[0] = 0;
    for (i = 0; i < len && l + 6 < sizeof b; i++)
        l += (size_t)snprintf(b + l, sizeof b - l, " %d", buf[i]);
    tr("7701C len%d k%d ->%s", len, k, b);
}

void aicity_test_f160EC(int n)
{
    setD8(0xF62A8u + (uint32_t)n, (uint8_t)(D8(0xF62A8u + (uint32_t)n) + 1));
    tr("160EC n%d", n);
}

void aicity_test_f7BD5C(int n)
{
    int t;
    for (t = 0; t < 157; t++)
        setD8(0xF3580u + 314u * (uint32_t)(n - 1) + 2u * (uint32_t)t, tok(t, n) > 1 ? 0x80 : 0);
    tr("7BD5C n%d", n);
}

void aicity_test_f46BB8(int n)
{
    int i, a, u, d = -1;
    for (i = 0; i < S8(NAT(n) + 0x60u); i++) {
        a = D8(NAT(n) + 0x61u + (uint32_t)i);
        u = a - 1;
        if (u >= 0 && u <= 156 && mov(u, n) >= 2 && g_adj_n[u] > 0 && g_adj[u][0] <= 156) {
            d = g_adj[u][0];
            f57CA4(u, d, n, 1, 1);
            tr("46BB8 n%d %d->%d", n, u, d);
            return;
        }
    }
    tr("46BB8 n%d none", n);
}

void aicity_test_f36918(int n, int t, int32_t *need, int code)
{
    int32_t c;
    tr("36918 n%d t%d need%d code%d", n, t, *need, code);
    if (*need > 0 && D8(NAT(n) + 0x40u) > 0) {
        (*need)--;
        c = S32(0x1079B4u);
        setD8(0x107694u + 4u * (uint32_t)c, 0x7E);
        setD8(0x107695u + 4u * (uint32_t)c, (uint8_t)t);
        setD8(0x107696u + 4u * (uint32_t)c, 1);
        setD8(0x107697u + 4u * (uint32_t)c, (uint8_t)code);
        setD32(0x1079B4u, (uint32_t)(c + 1));
        c = S32(0x107D3Cu);   /* and a ship record in list 2 (6 bytes) */
        if (c >= 0 && c < 150) {
            memset(DP(0x1079B8u + 6u * (uint32_t)c), 0, 6);
            setD8(0x1079B8u + 6u * (uint32_t)c + 1u, (uint8_t)t);
            setD8(0x1079B8u + 6u * (uint32_t)c + 4u, 4);
            setD32(0x107D3Cu, (uint32_t)(c + 1));
        }
    }
}

int aicity_test_f374E0(int j, int t, int n, int max, int a)
{
    int r = ((int)D8(0x108820u + 16u * (uint32_t)j + 6u) * 3 + j + t + n) % 9 - 2;
    tr("374E0 j%d t%d n%d max%d a%d F5908=%d -> %d", j, t, n, max, a, S32(0xF5908u), r);
    return r;
}

void aicity_test_f648C8(int k, int flags)
{
    int i, a, u, d;
    tr("648C8 k%d f%d F5908=%d", k, flags, S32(0xF5908u));
    for (i = 0; i < S8(NAT(k) + 0x60u); i++) {
        a = D8(NAT(k) + 0x61u + (uint32_t)i);
        u = a - 1;
        if (u >= 0 && u <= 156 && mov(u, k) > 0 && g_adj_n[u] > 0) {
            d = g_adj[u][(k + u) % g_adj_n[u]];
            if (d >= 0 && d <= 156) {
                f57CA4(u, d, k, 1, 1);
                tr("648C8 k%d %d->%d", k, u, d);
            }
            return;
        }
    }
}

int aicity_test_f49A1C(int t, int n, int k, int exclude)
{
    int r = (tok(t, n) + t + exclude + n) % 3 == 0;
    tr("49A1C t%d n%d k%d x%d -> %d", t, n, k, exclude, r);
    return r;
}

/* ---- the model: a second implementation from the listing --------------------------------------------------------- */

static int g_mut;          /* 0 = the model; 1..12 = deliberately broken copies (part C) */
static long g_cov[40];     /* coverage of the model's paths */

#define C_LEN(t) g_adj_n[t]

static int m_lvl(void) { uint16_t ax = (uint16_t)(D16(0xE1624u) << 4); return (uint16_t)(ax >> 14); }
static int m_b5(void) { uint16_t ax = (uint16_t)(D16(0xE1624u) << 10); return (uint16_t)(ax >> 15); }
static int m_lim(int t) { uint8_t al = (uint8_t)(D8(0xE1639u + 35u * (uint32_t)t) << 2); return (uint8_t)(al >> 5); }
static int32_t m_rd8(uint32_t dword_addr) { return (int32_t)D32(dword_addr) >> 24; }   /* dword [x] sar 24 */
static int32_t m_rd16(uint32_t dword_addr) { return (int32_t)D32(dword_addr) >> 16; }  /* dword [x] sar 16 */

static void m_rec(int b0, int b1, int b2, int b3)   /* a move record, each byte as the listing stores it */
{
    int32_t c = S32(0x1079B4u);
    setD8(0x107694u + 4u * (uint32_t)c, (uint8_t)b0);
    setD8(0x107695u + 4u * (uint32_t)c, (uint8_t)b1);
    setD8(0x107697u + 4u * (uint32_t)c, (uint8_t)b3);
    setD32(0x1079B4u, (uint32_t)(c + 1));
    setD8(0x107692u + 4u * (uint32_t)(c + 1), (uint8_t)b2);
}

static int32_t m_f337C8(int32_t v28)
{
    uint32_t v4 = 0xF14CCu + (uint32_t)v28 * 0xC7u;
    int32_t v10, v20, ebx, v8, vc, v24, v18, v14, v1c, a;
    v10 = S32(0xF4898u);
    v10 = (v10 - (v10 >> 31)) >> 1;
    if (v28 == 5)
        v10 -= 2;
    v20 = m_rd8(v4 + 0x95u);
    ebx = m_rd16(0x112AE0u + 2u * (uint32_t)v28) / 300;
    if (ebx < ((m_lvl() + 1) * 5) * 2)
        v10 -= 5;
    if (v20 < v10) {
        v8 = 0;
        for (vc = 0; m_rd8(v4 + 0x5Du) > vc; vc++) {
            a = D8(v4 + (uint32_t)vc + 0x61u);
            if (m_rd8((uint32_t)a + 4u * (17u * (uint32_t)a) + 0xE31B6u) > 1) {
                v8 = 1;
                break;
            }
        }
        g_cov[0]++;
        if (v20 > 0x15 || v8 != 0)
            return 1;
        return 0;
    }
    v24 = m_rd8(v4 + 0x4Eu);
    v18 = v24;
    while (!(v18 + v18 > v20) && !(v20 <= (g_mut == 1 ? 0x11 : 0x12))) {
        v20 -= 6;
        v18++;
    }
    v14 = v18 - v24;
    v1c = v14 < 3 ? v14 : 3;
    if (v1c == 3) {
        g_cov[1]++;
        v1c -= m_rd16(0x112AF4u + 2u * (uint32_t)v28) / 300 < 0x19;
    }
    if (v1c + v24 > 9)
        v1c = 9 - v24;
    return v1c;
}

static void m_f33ECC(int32_t *v2c, uint8_t *v24, int32_t v28, int32_t v10)
{
    int32_t v14 = v28 - 1, v20, v1c, v18;
    uint16_t vc;
    uint8_t loc[0xA0];
    (void)v14;
    *v2c = 0;
    for (v20 = 0; v20 < 0x9D; v20++) {
        if (D8(0xEE16Cu + (uint32_t)v20) == 0)
            continue;
        if (D8(0xE2C1Au + 3u * (uint32_t)v20) & 1)
            continue;
        if (D8(0xE2C1Au + 3u * (uint32_t)v20) & 2)
            continue;
        if (!(D8(0xE1639u + 35u * (uint32_t)v20) & 0x40))
            goto L340B6;
        if (m_rd8(0xE31C4u + 69u * (uint32_t)(v20 + 1) + (uint32_t)(v28 - 1)) <= 0)
            goto L33F70;
        if (v10 == 0 || v10 == 2)
            goto L3406D;
        /* L33F07 */
        if (v10 != 1)
            continue;
        if (!(D8(0xE163Au + 35u * (uint32_t)v20) & 1))
            continue;
        v24[2 * *v2c] = (uint8_t)v20;
        v24[2 * *v2c + 1] = (uint8_t)(D8(0xEFA04u + 160u * (uint32_t)v28 + (uint32_t)v20) + 0xA);
        *v2c += 1;
        continue;
    L33F70:
        if (v10 != 0)
            continue;
        if (aicity_test_f49934(v20, v28, 0) == 0)
            continue;
        v24[2 * *v2c] = (uint8_t)v20;
        v24[2 * *v2c + 1] = (uint8_t)(D8(0xEFA04u + 160u * (uint32_t)v28 + (uint32_t)v20) + 0xA);
        *v2c += 1;
        continue;
    L3406D:
        v24[2 * *v2c] = (uint8_t)v20;
        v24[2 * *v2c + 1] = (uint8_t)(D8(0xEFA04u + 160u * (uint32_t)v28 + (uint32_t)v20) + 0xA);
        *v2c += 1;
        continue;
    L340B6:
        if (v10 != 0)
            continue;
        if (m_lim(v20) == 1)
            goto L3412F;
        if (v28 == 4 && v20 == 1)
            goto L3412F;
        if (v28 != 4 && v28 != 3)
            continue;
        if (m_lim(v20) != 2)
            continue;
    L3412F:
        if (m_rd8(0xE31B6u + 4u * 17u * (uint32_t)(v20 + 1) + (uint32_t)(v20 + 1)) != 1)
            continue;
        if (m_rd8(0xE31B7u + 69u * (uint32_t)(v20 + 1) + (uint32_t)(v28 - 1)) != v28)
            continue;
        g_cov[2]++;
        v24[2 * *v2c] = (uint8_t)v20;
        v24[2 * *v2c + 1] = (uint8_t)(9 - m_lim(v20));
        *v2c += 1;
    }
    for (v20 = *v2c - 2; v20 >= 0; v20--)
        for (v1c = 0; v1c <= v20; v1c++) {
            int8_t dl = (int8_t)v24[2 * v1c + 3], ch = (int8_t)v24[2 * v1c + 1];
            if (g_mut == 2 ? dl < ch : dl > ch) {
                vc = (uint16_t)(v24[2 * v1c + 2] | v24[2 * v1c + 3] << 8);
                v24[2 * v1c + 2] = v24[2 * v1c];
                v24[2 * v1c + 3] = v24[2 * v1c + 1];
                v24[2 * v1c] = (uint8_t)vc;
                v24[2 * v1c + 1] = (uint8_t)(vc >> 8);
            }
        }
    v20 = 0;
    while (v20 < *v2c) {
        v1c = v20;
        while (v24[2 * v20 + 1] == v24[2 * v1c + 3] && v1c + 1 < *v2c)
            v1c++;
        if (v20 == v1c) {
            v20++;
            continue;
        }
        g_cov[3]++;
        for (v18 = v20; v18 <= v1c; v18++)
            loc[v18 - v20] = v24[2 * v18];
        aicity_test_f7701C(v1c - v20 + 1, loc, v28);
        for (v18 = v20; v18 <= v1c; v18++)
            v24[2 * v18] = loc[v18 - v20];
        v20 = v1c + 1;
    }
}

static int32_t m_f348B0(int32_t v5c, int32_t v18, int32_t *v54, int32_t *v10);
static void m_f3566C(int32_t vc);
static int32_t m_f3580C(int32_t v74, int32_t v70, int32_t v44, int32_t v38, int32_t arg8);

static int32_t m_f3433C(int32_t v48, int32_t v44)
{
    int32_t v2c = -1, v28 = 0, v24 = 0, v40 = v48 + 1, v34 = v44 - 1, v3c = 0, v38 = 0, v30 = 0;
    int32_t v20, v1c, v14, v18, ebx, ecx, esi, r, eax;
    (void)v24;
    if (D8(0xF14FDu + 199u * (uint32_t)v44) != 0 && S32(0x107EE0u) == 0) {
        v38 = m_rd8(0xF1507u + 199u * (uint32_t)v44) - 2 * (m_b5() ? 7 : 9);
        if (v38 > 6)
            v38 = 6;
        else if (v38 < 0)
            v38 = 0;
    }
    if (m_rd8(0xE31B6u + 69u * (uint32_t)v40) >= 2) {
        if (m_rd8(0xE31B6u + 69u * (uint32_t)v40) != 2)
            return v2c;
        if (m_rd8(0xE31C4u + 69u * (uint32_t)v40 + (uint32_t)v34) == 0)
            return v2c;
    }
    v20 = m_f348B0(v48, v44, &v28, &v30);
    v1c = 0;
    if (m_rd8(0xE31B6u + 69u * (uint32_t)v40) == 1) {
        if (m_rd8(0xE31B7u + 69u * (uint32_t)v40 + (uint32_t)v34) != v44)
            goto L344CD;
    }
    if (m_rd8(0xE31B6u + 69u * (uint32_t)v40) != 2)
        goto L3450E;
    if (m_rd8(0xE31B7u + 69u * (uint32_t)v40 + (uint32_t)v34) != v44)
        goto L3450E;
L344CD:
    v24 = f7C7C4(v48, v44);
    if (v24 != v28) {
        v20 = 0;
        v28 = v24;
    }
    v1c = m_rd8(0xE31C3u + 69u * (uint32_t)v40 + (uint32_t)v28);
L3450E:
    if (v20 + v1c <= 0)
        goto L34804;
    if (v28 == 0xB)
        goto L34804;
    if (S32(0xF52E0u + 40u * (uint32_t)v28 + 4u * (uint32_t)v44) >= 0)
        goto L346A9;
    ebx = (m_rd8(0xF1563u + 199u * (uint32_t)v28) == 3) + 1;
    ecx = 5 - m_lvl();
    ebx *= ecx;
    eax = 200 / ebx;
    esi = S32(0xF52E0u + 40u * (uint32_t)v44 + 4u * (uint32_t)v28) - eax;
    r = v48 % 4 + 9;
    if (m_rd16(0x112A90u + 20u * (uint32_t)r + 2u * (uint32_t)v28) / 300 > esi)
        goto L34643;
    if (m_rd8(0xF1563u + 199u * (uint32_t)v28) == 2) {
        g_cov[4]++;
        goto L34804;
    }
L34601:
    g_cov[5]++;
    v14 = 0;
    if (D8(0xF14F2u + 199u * (uint32_t)v28) != 0 && D8(0xF14F2u + 199u * (uint32_t)v44) == 0)
        v14 = 1;
    if (g_mut == 3)
        goto L346A9;
    if (v14 + 2 < v20)
        v20 = v14 + 2;
    goto L346A9;
L34643:
    ebx = m_rd16(0x112A90u + 20u * (uint32_t)r + 2u * (uint32_t)v44) / 300;
    if (ebx <= S32(0xF52E0u + 40u * (uint32_t)v44 + 4u * (uint32_t)v28) / ecx)
        goto L34601;
L346A9:
    if (!(D8(0xE1639u + 35u * (uint32_t)v48) & 0x40))
        return -999;
    v18 = 0;
    if (D8(0xF14F2u + 199u * (uint32_t)v44) == 0 && D8(0xF14F2u + 199u * (uint32_t)v28) != 0)
        v18 = 1;
    ebx = 3 * m_rd8(0xF4889u + (uint32_t)v44) + 4 - v18;
    if (v20 + v1c > ebx)
        v2c = -(v20 + v1c);
    else
        v2c = (6 - v3c) + (v20 + v1c - 1 + v18) - m_rd8(0xE31C3u + 69u * (uint32_t)v40 + (uint32_t)v44);
    if (S32(0x107EE0u) != 0 || S32(0x107EDCu) != 0 || D8(0xF14FDu + 199u * (uint32_t)v44) == 0 || v38 <= 2)
        return v2c;
    if (m_rd8(0xE31C3u + 69u * (uint32_t)v40 + (uint32_t)v44) + v2c <= 7)
        return v2c;
    g_cov[6]++;
    v2c -= m_rd8(0xE31C3u + 69u * (uint32_t)v40 + (uint32_t)v44) + v2c - 7;
    setD32(0x107EDCu, 1);
    return v2c;
L34804:
    if (D8(0xE1639u + 35u * (uint32_t)v48) & 0x40) {
        v2c = (v1c == 0 ? 0 : v1c - 1) + 6 - v3c - m_rd8(0xE31C3u + 69u * (uint32_t)v40 + (uint32_t)v44);
        return v2c;
    }
    if (v30 != 0 || v1c != 0)
        return v2c;
    g_cov[7]++;
    v2c = 0xC - v38 - m_rd8(0xE31C3u + 69u * (uint32_t)v40 + (uint32_t)v44);
    if (v38 > 0)
        setD32(0x107EE0u, 1);
    return v2c;
}

/* the ship pass of 0x348B0 for nation v40 (0x34D00..0x34E5F; the human pass 0x3505F..0x351C0 is the same code) */
static void m_ships(int32_t v5c, int32_t v40, int32_t v30, uint32_t vc, int32_t *vc4)
{
    int32_t v6c[4], v4c, v20, v44, v58, v50, cnt, v28, v2c = v40 - 1, save;
    uint8_t v7c[16];
    memset(v6c, 0, 16);
    for (v4c = 0; (int32_t)D8(0xF150Cu + 199u * (uint32_t)v40) > v4c; v4c++) {
        v20 = m_rd16(0xF150Fu + 199u * (uint32_t)v40 + 2u * (uint32_t)v4c) - 1;
        v44 = D8(0xF150Du + 199u * (uint32_t)v40 + (uint32_t)v4c);
        if (f5843C(v20, v5c, v40, v44) != 1)
            continue;
        g_cov[8]++;
        f58888(v20, v5c, v40, v44);
        memset(DP(0xF8418u), 0xFF, 0xA0);
        memset(DP(0xF84B8u), 0xFF, 0xA0);
        memset(DP(0xF8194u), 0xFF, 0x280);
        for (v58 = 0; v58 < S32(0x109E00u); v58++) {
            save = S32(0xF5908u);
            setD32(0xF5908u, 0);
            v28 = aicity_test_f374E0(v58, v5c, v40, 0x3E7, 1);
            setD32(0xF5908u, (uint32_t)save);
            setD8(0x108828u + 16u * (uint32_t)v58, (uint8_t)v28);
        }
        cnt = S32(0x109E00u);
        if (cnt < 0x14) {
            for (v50 = cnt - 2; v50 >= 0; v50--)
                for (v58 = 0; v58 <= v50; v58++)
                    if ((int8_t)D8(0x108838u + 16u * (uint32_t)v58) > (int8_t)D8(0x108828u + 16u * (uint32_t)v58)) {
                        memcpy(v7c, DP(0x108830u + 16u * (uint32_t)v58), 16);
                        memcpy(DP(0x108830u + 16u * (uint32_t)v58), DP(0x108820u + 16u * (uint32_t)v58), 16);
                        memcpy(DP(0x108820u + 16u * (uint32_t)v58), v7c, 16);
                    }
        } else {
            g_cov[9]++;
            wc_qsort(DP(0x108820u), (size_t)cnt, 0x10, f3530C);
        }
        v6c[v4c] = m_rd8(0x108825u);
    }
    for (v50 = 0; (int32_t)D8(0xF150Cu + 199u * (uint32_t)v40) > v50; v50++)
        if (v6c[v50] > vc4[v2c])
            vc4[v2c] = v6c[v50];
    if ((int8_t)D8(0xBCCDCu + 5u * (uint32_t)m_rd8(vc + 0x59u) + 85u * (uint32_t)v30) > (int8_t)D8(vc + 0x51u)) {
        g_cov[10]++;
        vc4[v2c] /= 4 - m_lvl();
    }
}

static int32_t m_f348B0(int32_t v5c, int32_t v18, int32_t *v54, int32_t *v10)
{
    int32_t ve8[9], va0[9], vc4[9], v5e8[160], v368[160];
    int32_t v30, v34, v40, v1c = 0, v14 = 0, v50, v48, v58, v24, v3c;
    uint32_t vc;
    memset(ve8, 0, 0x24);
    memset(va0, 0, 0x24);
    memset(vc4, 0, 0x24);
    memset(v5e8, 0, sizeof v5e8);
    memset(v368, 0, sizeof v368);
    v30 = v18 - 1;
    vc = 0xF14CCu + 199u * (uint32_t)v18;
    v34 = m_rd8(0xF06BDu + (uint32_t)v18);
    f7A940();
    f7AA1C();
    for (v40 = 1; v40 <= 9; v40++)
        if (m_rd8(0xF06BDu + (uint32_t)v40) > v34)
            m_f3566C(v40);
    aicity_test_f48028(&v1c, v5e8, v5c, v18, 0xB);
    aicity_test_f4892C(v5c, &v14, v368, v1c, v5e8, v18, 0xB);
    for (v50 = 0; v50 < v1c; v50++) {
        v48 = v5e8[v50] + 1;
        if (m_rd8(0xE31B6u + 69u * (uint32_t)v48) == 0)
            continue;
        for (v58 = 0; v58 < 9; v58++) {
            if (v58 == v30)
                continue;
            if (!(m_rd8(0xF06BEu + (uint32_t)v58) > v34) && v58 + 1 != 0xB)
                continue;
            v24 = m_rd8(0xE31DAu + 69u * (uint32_t)v48 + (uint32_t)v58);
            if (v24 == 0)
                continue;
            va0[v58] += v24;
            ve8[v58] += v24;
        }
    }
    for (v50 = 0; v50 < v14; v50++) {
        v48 = v368[v50] + 1;
        if (m_rd8(0xE31B6u + 69u * (uint32_t)v48) == 0)
            continue;
        for (v58 = 0; v58 < 9; v58++) {
            if (v58 == v30)
                continue;
            if (!(m_rd8(0xF06BEu + (uint32_t)v58) > v34) && v58 + 1 != 0xB)
                continue;
            if (D8(0xF14F4u + 199u * (uint32_t)(v58 + 1)) == 0)
                continue;
            v24 = m_rd8(0xE31DAu + 69u * (uint32_t)v48 + (uint32_t)v58);
            if (v24 == 0)
                continue;
            g_cov[11]++;
            va0[v58] += v24;
            ve8[v58] += g_mut == 4 ? v24 : v24 - 1;
        }
    }
    v3c = 0;
    for (v50 = 0; v50 < 9; v50++)
        if (ve8[v50] > v3c) {
            v3c = ve8[v50];
            *v54 = v50 + 1;
        }
    *v10 = 0;
    for (v50 = 0; v50 < 9; v50++)
        if (va0[v50] > *v10)
            *v10 = va0[v50];
    if (v3c < 5 && (D8(0xE1638u + 35u * (uint32_t)v5c) & 2)) {
        for (v40 = 1; v40 <= 9; v40++) {
            if (m_rd8(0xF06BDu + (uint32_t)v40) <= v34)
                continue;
            if (D8(0xF150Cu + 199u * (uint32_t)v40) == 0)
                continue;
            m_ships(v5c, v40, v30, vc, vc4);
        }
        v3c = 0;
        for (v50 = 0; v50 < 9; v50++)
            if (ve8[v50] + vc4[v50] > v3c) {
                v3c = ve8[v50] + vc4[v50];
                *v54 = v50 + 1;
            }
    }
    f7AC60();
    f7AE14();
    if (v3c != 0)
        return v3c;
    g_cov[12]++;
    f7A940();
    f7AA1C();
    for (v40 = 1; v40 <= 9; v40++) {
        if (m_rd8(0xF1563u + 199u * (uint32_t)v40) != 3)
            continue;
        if (m_rd8(0xF06BDu + (uint32_t)v40) <= v34)
            continue;
        if (D8(0xF150Cu + 199u * (uint32_t)v40) == 0)
            continue;
        g_cov[13]++;
        setD8(0xF1529u + 199u * (uint32_t)v40, 0x63);
        m_ships(v5c, v40, v30, vc, vc4);
    }
    v3c = 0;
    for (v50 = 0; v50 < 9; v50++)
        if (ve8[v50] + vc4[v50] > v3c) {
            v3c = ve8[v50] + vc4[v50];
            *v54 = v50 + 1;
        }
    if ((D8(0xF14F2u + 199u * (uint32_t)*v54) != 0 && D8(0xF14F2u + 199u * (uint32_t)v18) == 0 ? 1 : 2) < v3c)
        v3c = D8(0xF14F2u + 199u * (uint32_t)*v54) != 0 && D8(0xF14F2u + 199u * (uint32_t)v18) == 0 ? 1 : 2;
    f7AC60();
    f7AE14();
    return v3c;
}

static void m_cols(uint32_t dst, uint32_t src, int32_t j)   /* the six column bytes, in the listing's order */
{
    setD8(dst + 0x02u + (uint32_t)j, D8(src + 0x02u + (uint32_t)j));
    setD8(dst + 0x0Fu + (uint32_t)j, D8(src + 0x0Fu + (uint32_t)j));
    setD8(dst + 0x1Cu + (uint32_t)j, D8(src + 0x1Cu + (uint32_t)j));
    setD8(dst + 0x2Eu + (uint32_t)j, D8(src + 0x2Eu + (uint32_t)j));
    setD8(dst + 0x25u + (uint32_t)j, D8(src + 0x25u + (uint32_t)j));
    if (g_mut != 7)
        setD8(dst + 0x3Cu + (uint32_t)j, D8(src + 0x3Cu + (uint32_t)j));
}

static int32_t m_f33964(int32_t v58, int32_t *v40, int32_t v54, int32_t v50);

static void m_f353C4(int32_t v38)
{
    int32_t v18, v34, v2c, v1c, v20, v28, v24;
    f7A940();
    f7AA1C();
    v18 = m_rd8(0xF06BDu + (uint32_t)v38);
    for (v34 = 1; v34 <= 9; v34++) {
        if (m_rd8(0xF06BDu + (uint32_t)v34) <= v18)
            continue;
        if (D8(0xF150Cu + 199u * (uint32_t)v34) == 0)
            continue;
        g_cov[14]++;
        f460C0(v34);
        f46158(v34);
        setD8(0xF1529u + 199u * (uint32_t)v34, 0x63);
        v2c = S32(0xF5908u);
        setD32(0xF5908u, 0);
        aicity_test_f160EC(v34);
        aicity_test_f7BD5C(v34);
        memset(DP(0xF8558u + 160u * (uint32_t)(v34 - 1)), 0, 0xA0);
        v1c = m_f337C8(v34);
        v20 = 0;
        m_f33964(v1c, &v20, v34, 2);
        aicity_test_f46BB8(v34);
        setD32(0xF5908u, (uint32_t)v2c);
        memcpy(DP(0xF09E8u + 199u * (uint32_t)v34), DP(0xF14CCu + 199u * (uint32_t)v34), 199);
        for (v28 = 1; v28 <= 0xA0; v28++)
            for (v24 = 0; v24 < 9; v24++)
                m_cols(0xEB3F0u + 69u * (uint32_t)v28, 0xE31B8u + 69u * (uint32_t)v28, v24);
    }
    f7AC60();
    f7AE14();
}

static void m_f3566C(int32_t vc)
{
    int32_t v4 = vc - 1, v8;
    memcpy(DP(0xF14CCu + 199u * (uint32_t)vc), DP(0xF09E8u + 199u * (uint32_t)vc), 199);
    for (v8 = 1; v8 <= 0xA0; v8++)
        m_cols(0xE31B8u + 69u * (uint32_t)v8, 0xEB3F0u + 69u * (uint32_t)v8, v4);
}

static int32_t m_f79128(int32_t v24, int32_t v20, int32_t v1c)
{
    int32_t v18, v14, v10, vc;
    for (v18 = 1; v18 <= C_LEN(v24); v18++) {
        v10 = g_adj[v24][v18 - 1];
        for (v14 = 1; v14 <= C_LEN(v20); v14++) {
            if (g_adj[v20][v14 - 1] != v10)
                continue;
            if (g_mut == 10)
                return v10;
            vc = v10 + 1;
            if ((D8(0xE2C1Au + 3u * (uint32_t)v10) & 1) && D8(0xE2C18u + 3u * (uint32_t)v10) == (uint32_t)v1c)
                return v10;
            if (m_rd8(0xE31B6u + 69u * (uint32_t)vc) == 0) {
                if (!(D8(0xE2C1Au + 3u * (uint32_t)v10) & 1))
                    return v10;
            }
            if (m_rd8(0xE31B6u + 69u * (uint32_t)vc) != 1)
                continue;
            if (m_rd8(0xE31B6u + 69u * (uint32_t)vc + (uint32_t)v1c) != v1c)
                continue;
            return v10;
        }
    }
    return -1;
}

static int32_t m_f3580C(int32_t v74, int32_t v70, int32_t v44, int32_t v38, int32_t arg8)
{
    uint32_t vc = 0xF14CCu + 199u * (uint32_t)v44;
    int32_t v68 = m_rd8(vc + 0x24u), v64 = m_rd8(vc + 0x25u), v34 = v44 - 1, v2c = v74;
    int32_t v3c = m_rd8(0xE31C4u + 69u * (uint32_t)(v70 + 1) + (uint32_t)v34);
    int32_t v394[160], v614[160], v40 = 0, v10 = 0, v48 = 0, v6c, v5c, v14, v58, v60, v30, v28 = 0, v50, v4c;   /* v28: f495C0 writes it (0 first, 0x495E1) before f49790 reads it */
    int32_t v20, v24, v18, v1c, v54, edx, ebx, eax, i;
    uint8_t v114[160];
    memset(v394, 0, sizeof v394);
    memset(v614, 0, sizeof v614);
    memset(v114, 0, sizeof v114);

    if (v38 != 0)
        goto L3598A;
    aicity_test_f48028(&v40, v394, v70, v44, 0x15);
    if (v40 == 0)
        goto L3598A;
    aicity_test_f48D44(&v40, v394, v44, v70, 1);
    for (v6c = 0; v6c < v40; v6c++) {
        aicity_test_f7C838(v394[v6c], v44, 0, &v48);
        if (v48 == 0)
            continue;
        while (v74 > 0 && m_rd8(0xE31DAu + 69u * (uint32_t)(v394[v6c] + 1) + (uint32_t)v34) > 0) {
            g_cov[15]++;
            f57CA4(v394[v6c], v70, v44, 1, 1);
            m_rec(v394[v6c], v70, 1, 0x20);
            v74--;
        }
    }
L3598A:
    aicity_test_f48028(&v40, v394, v70, v44, 9);
    if (v40 == 0)
        goto L35AD8;
    aicity_test_f48D44(&v40, v394, v44, v70, 1);
    for (v5c = 0; v5c < v40 && v74 > 0; v5c++) {
        v14 = v394[v5c];
        v58 = v14 + 1;
        v60 = v68 + m_lim(v14);
        while (v74 > 0 && m_rd8(0xE31DAu + 69u * (uint32_t)v58 + (uint32_t)v34) > 0 &&
               m_rd8(0xE31C4u + 69u * (uint32_t)v58 + (uint32_t)v34) > v60) {
            g_cov[16]++;
            f57CA4(v14, v70, v44, 1, 1);
            m_rec(v14, v70, 1, v38 == 0 ? 0 : 0x1B);
            v74--;
        }
    }
L35AD8:
    if (v40 <= 0 || m_rd8(vc + 0x5Au) <= 0)
        goto L35C60;
    v5c = 0;
    v30 = 0xA;
    memset(v114, 0, 0xA0);
    while (v74 > 0 && v30 > 0) {
        if (v114[v5c] == 0) {
            v14 = v394[v5c];
            v58 = v14 + 1;
            v50 = aicity_test_f495C0(v14, v44, 1, &v28);
            v50 = aicity_test_f49790(v50, v14, v44, v28);
            if (v50 == 0)
                v50 = 1;
            if (m_rd8(0xE31DAu + 69u * (uint32_t)v58 + (uint32_t)v34) > 0 &&
                m_rd8(0xE31C4u + 69u * (uint32_t)v58 + (uint32_t)v34) > v50) {
                g_cov[17]++;
                f57CA4(v14, v70, v44, 1, 1);
                m_rec(v14, v70, 1, v38 == 0 ? 1 : 0x1C);
                v74--;
                setD8(vc + 0x5Du, (uint8_t)(D8(vc + 0x5Du) - 1));
            } else
                v114[v5c] = 1;
        }
        v5c++;
        if (v5c == v40) {
            v5c = 0;
            v30--;
        }
    }
L35C60:
    aicity_test_f48028(&v40, v394, v70, v44, 9);
    if (v74 <= 0 || v40 == 0 || m_rd8(vc + 0x5Au) <= 0)
        goto L35E21;
    aicity_test_f48D44(&v40, v394, v44, v70, 1);
    v5c = 0;
    v30 = 0xA;
    memset(v114, 0, 0xA0);
    while (v74 > 0 && v30 > 0) {
        if (v114[v5c] == 0) {
            v14 = v394[v5c];
            v58 = v14 + 1;
            v50 = aicity_test_f495C0(v14, v44, 1, &v28);
            v50 = aicity_test_f49790(v50, v14, v44, v28);
            if (v50 == 0)
                v50 = 1;
            if (m_rd8(0xE31DAu + 69u * (uint32_t)v58 + (uint32_t)v34) > 0 &&
                m_rd8(0xE31C4u + 69u * (uint32_t)v58 + (uint32_t)v34) > v50) {
                g_cov[18]++;
                f57CA4(v14, v70, v44, 1, 1);
                m_rec(v14, v70, 1, v38 == 0 ? 2 : 0x1D);
                v74--;
                setD8(vc + 0x5Du, (uint8_t)(D8(vc + 0x5Du) - 1));
            } else
                v114[v5c] = 1;
        }
        v5c++;
        if (v5c == v40) {
            v5c = 0;
            v30--;
        }
    }
L35E21:
    memset(v114, 0, 0xA0);
    v20 = 0;
    if (v74 <= 0 || v40 == 0 || m_rd8(vc + 0x5Au) <= 0)
        goto L36162;
    f7A940();
    f7AA1C();
    v24 = v74;
L35E6D:
    v5c = 0;
    v30 = 0xA;
    while (v74 > 0 && v30 > 0) {
        if (v114[v5c] == 0) {
            v14 = v394[v5c];
            v58 = v14 + 1;
            v50 = aicity_test_f495C0(v14, v44, 1, &v28);
            v50 = aicity_test_f49790(v50, v14, v44, v28);
            v4c = S32(0xF5908u);
            setD32(0xF5908u, 0);
            if (m_rd8(0xE31DAu + 69u * (uint32_t)v58 + (uint32_t)v34) <= 0)
                goto L3601A;
            if (m_rd8(0xE31C4u + 69u * (uint32_t)v58 + (uint32_t)v34) <= v50)
                goto L3601A;
            if (g_mut != 5 && (D8(0xE163Au + 35u * (uint32_t)v14) & 4))
                goto L3601A;
            if (v38 == 0 && v50 == 0)
                goto L35F71;
            if (v38 == 1)
                goto L35F71;
            if (v38 != 2)
                goto L3601A;
        L35F71:
            if (aicity_test_f49A1C(v14, v44, 1, v70) != 0)
                goto L35FAD;
            if (m_rd8(vc + 0x5Au) <= 0)
                goto L3601A;
            if (!(D8(0xF3580u + 0x13Au * (uint32_t)v34 + 2u * (uint32_t)v14) & 0x80))
                goto L3601A;
        L35FAD:
            g_cov[19]++;
            f57CA4(v14, v70, v44, 1, 1);
            m_rec(v14, v70, 1, v38 == 0 ? 3 : 0x1E);
            v74--;
            setD8(vc + 0x5Du, (uint8_t)(D8(vc + 0x5Du) - 1));
            goto L36025;
        L3601A:
            v114[v5c] = 1;
        L36025:
            setD32(0xF5908u, (uint32_t)v4c);
        }
        v5c++;
        if (v5c == v40) {
            v5c = 0;
            v30--;
        }
    }
    f7A688();
    f7A7E8();
    if (v20 != 0)
        goto L36158;
    v18 = m_rd8(0xF06BDu + (uint32_t)v44);
    memset(v114, 0, 0xA0);
    for (v1c = 1; v1c <= 9; v1c++) {
        if (m_rd8(0xF06BDu + (uint32_t)v1c) <= v18)
            continue;
        f460C0(v1c);
        f46158(v1c);
        v4c = S32(0xF5908u);
        setD32(0xF5908u, 0);
        aicity_test_f648C8(v1c, 0);
        setD32(0xF5908u, (uint32_t)v4c);
        for (v5c = 0; v5c < v40; v5c++)
            if (m_rd8(0xE31C3u + 69u * (uint32_t)(v394[v5c] + 1) + (uint32_t)v1c) != 0) {
                v114[v5c] = 1;
                v20 = 1;
            }
    }
    if (v20 != 0) {
        g_cov[20]++;
        if (g_mut == 9) {
            f7A700();
            f7A85C();
        } else {
            f7AF28();
            f7AFB8();
        }
        v74 = v24;
        goto L35E6D;
    }
    f7A700();
    f7A85C();
L36158:
    f7AD24();
    f7AD7C();
L36162:
    if (v40 <= 0 || v64 == 0)
        goto L36319;
    aicity_test_f4892C(v70, &v10, v614, v40, v394, v44, 9);
    if (v10 <= 0)
        goto L36319;
    aicity_test_f48D44(&v10, v614, v44, v70, 1);
    for (v5c = 0; v5c < v10 && v74 > 0; v5c++) {
        v14 = v614[v5c];
        v58 = v14 + 1;
        while (v74 > 0 && m_rd8(0xE31DAu + 69u * (uint32_t)v58 + (uint32_t)v34) > 0 &&
               m_rd8(0xE31C4u + 69u * (uint32_t)v58 + (uint32_t)v34) > 1) {
            g_cov[21]++;
            v54 = m_f79128(v14, v70, v44);
            f57CA4(v14, v54, v44, 1, 1);
            m_rec(v14, v54, 0x33, v38 == 0 ? 0x12 : 0x1F);
            f57CA4(v54, v70, v44, 0, 1);
            m_rec(v54, v70, 0x4C, v38 == 0 ? 0x12 : 0x1F);
            v74--;
        }
    }
L36319:
    if (v74 > 0 && (D8(0xE1638u + 35u * (uint32_t)v70) & 2) && D16(0xBB3F0u) != 0) {
        g_cov[22]++;
        aicity_test_f36918(v44, v70, &v74, (int)(int8_t)(uint8_t)v38);
    }
    memset(v114, 0, 0xA0);
    v20 = 0;
    if (v74 <= 0 || v40 == 0 || m_rd8(vc + 0x5Au) <= 0)
        goto L3667A;
    f7A940();
    f7AA1C();
    v24 = v74;
L363A1:
    v5c = 0;
    v30 = 0xA;
    while (v74 > 0 && v30 > 0) {
        if (v114[v5c] == 0) {
            v14 = v394[v5c];
            v58 = v14 + 1;
            v50 = aicity_test_f495C0(v14, v44, 1, &v28);
            v50 = aicity_test_f49790(v50, v14, v44, v28);
            v4c = S32(0xF5908u);
            setD32(0xF5908u, 0);
            if (m_rd8(0xE31DAu + 69u * (uint32_t)v58 + (uint32_t)v34) <= 0)
                goto L36532;
            if (m_rd8(0xE31C4u + 69u * (uint32_t)v58 + (uint32_t)v34) <= v50)
                goto L36532;
            if (v38 == 0 && v50 == 0)
                goto L36487;
            if (v38 == 1)
                goto L36487;
            if (v38 != 2)
                goto L36532;
        L36487:
            if (aicity_test_f49A1C(v14, v44, 1, v70) != 0)
                goto L364C5;
            if (m_rd8(vc + 0x5Au) <= 0)
                goto L364BF;
            if (D8(0xF3580u + 0x13Au * (uint32_t)v34 + 2u * (uint32_t)v14) & 0x80)
                goto L364C5;
        L364BF:
            if (v38 != 2)
                goto L36532;
            g_cov[23]++;
        L364C5:
            g_cov[24]++;
            f57CA4(v14, v70, v44, 1, 1);
            m_rec(v14, v70, 1, v38 == 0 ? 3 : 0x1E);
            v74--;
            setD8(vc + 0x5Du, (uint8_t)(D8(vc + 0x5Du) - 1));
            goto L3653D;
        L36532:
            v114[v5c] = 1;
        L3653D:
            setD32(0xF5908u, (uint32_t)v4c);
        }
        v5c++;
        if (v5c == v40) {
            v5c = 0;
            v30--;
        }
    }
    f7A688();
    f7A7E8();
    if (v20 != 0)
        goto L36670;
    v18 = m_rd8(0xF06BDu + (uint32_t)v44);
    memset(v114, 0, 0xA0);
    for (v1c = 1; v1c <= 9; v1c++) {
        if (m_rd8(0xF06BDu + (uint32_t)v1c) <= v18)
            continue;
        f460C0(v1c);
        f46158(v1c);
        v4c = S32(0xF5908u);
        setD32(0xF5908u, 0);
        aicity_test_f648C8(v1c, 0);
        setD32(0xF5908u, (uint32_t)v4c);
        for (v5c = 0; v5c < v40; v5c++)
            if (m_rd8(0xE31C3u + 69u * (uint32_t)(v394[v5c] + 1) + (uint32_t)v1c) != 0) {
                v114[v5c] = 1;
                v20 = 1;
            }
    }
    if (v20 != 0) {
        g_cov[25]++;
        f7AF28();
        f7AFB8();
        v74 = v24;
        goto L363A1;
    }
    f7A700();
    f7A85C();
L36670:
    f7AD24();
    f7AD7C();
L3667A:
    if (v38 != 0)
        goto L3679B;
    if (m_rd8(0xE31C3u + 69u * (uint32_t)(v70 + 1) + (uint32_t)v44) >= 0xC && m_rd8(0x107ED5u) == 1) {
        g_cov[26]++;
        v74 = -1;
    }
    if (v74 != 0) {
        setD8(0xE2C1Au + 3u * (uint32_t)v70, (uint8_t)(D8(0xE2C1Au + 3u * (uint32_t)v70) & 0xFD));
        setD8(0xE2C18u + 3u * (uint32_t)v70, 0);
        setD16(0xE31EFu + 69u * (uint32_t)(v70 + 1), 0);
        return v74;
    }
    g_cov[27]++;
    eax = S32(0x1079B4u);
    edx = 4 * eax;
    setD8(0x107694u + 4u * (uint32_t)eax, 0xC8);
    eax++;
    setD32(0x1079B4u, (uint32_t)eax);
    setD8(0x107692u + 4u * (uint32_t)eax, 0);
    setD8(0x107695u + (uint32_t)edx, (uint8_t)v70);
    setD8(0xE2C1Au + 3u * (uint32_t)v70, (uint8_t)(D8(0xE2C1Au + 3u * (uint32_t)v70) | 2));
    setD8(0xE2C18u + 3u * (uint32_t)v70, (uint8_t)v44);
    setD16(0xE31EFu + 69u * (uint32_t)(v70 + 1), (uint16_t)(int16_t)(int8_t)D8(0xE31C7u + 69u * (uint32_t)(v70 + 1) + (uint32_t)v34));
    return 0;
L3679B:
    if (v38 == 1) {
        if (v74 == 0)
            return 0;
        if (D8(0xE2C1Au + 3u * (uint32_t)v70) & 1)
            return v74;
        ebx = 69 * (v70 + 1);
        i = f7C7C4(v70, v44);
        ebx = m_rd8((uint32_t)ebx + (uint32_t)i + 0xE31C3u);
        if (ebx > m_lim(v70))
            return v74;
        if (m_rd8(0xE31B6u + 69u * (uint32_t)(v70 + 1)) != 1)
            return v74;
        g_cov[28]++;
        if (v2c - v74 >= 2)
            return 0;
        if (v2c - v74 < 1)
            return v74;
        if (v3c <= 0)
            return v74;
        return 0;
    }
    if (v38 != 2)
        return v74;
    eax = S32(0x1079B4u);
    if (g_mut != 6)
        eax <<= 2;
    if (v74 != 0 && v2c - v74 < arg8)
        return v74;
    g_cov[29]++;
    setD8(0x107694u + (uint32_t)eax, 0xC9);
    setD8(0x107696u + (uint32_t)eax, 0);
    edx = S32(0x1079B4u) + 1;
    setD8(0x107691u + 4u * (uint32_t)edx, D8(0xE2C18u + 3u * (uint32_t)v70));
    setD32(0x1079B4u, (uint32_t)edx);
    return 0;
}

static int32_t m_f33964(int32_t v58, int32_t *v40, int32_t v54, int32_t v50)
{
    uint32_t vc;
    int32_t v48, v28, v24, v4c, v18 = 0, v30, v2c, v20, v1c, v38, v34, v14, v10, v44, v3c;
    uint8_t v198[0x198];
    if (v58 <= 0)
        return v58;
    memset(v198, 0, sizeof v198);
    vc = 0xF14CCu + 199u * (uint32_t)v54;
    v48 = v54 - 1;
    v28 = v58 - *v40;
    v24 = 0;
    for (v4c = 0; m_rd8(vc + 0x5Du) > v4c; v4c++) {
        int32_t a = D8(vc + (uint32_t)v4c + 0x61u);
        if (!(D8(0xE2C1Au + 3u * (uint32_t)(a - 1)) & 2))
            continue;
        if (m_rd8(0xE31C3u + 69u * (uint32_t)a + (uint32_t)v54) < 3)
            continue;
        v24++;
    }
    v30 = 0;   /* the fixed garbage read (see aicity.c) */
    if (v28 <= 0 || m_rd8(vc + 0x4Eu) + v24 >= 9) {
        g_cov[30]++;
        goto L33CA8;
    }
    m_f33ECC(&v18, v198, v54, v50);
    v30 = 0;
    v2c = 0;
    while (v30 < v28 && v2c < v18 && m_rd8(0xF151Au + 199u * (uint32_t)v54) + v30 + v24 < 9) {
        v20 = v198[2 * v2c];
        if (D8(0xF8558u + 160u * (uint32_t)v48 + (uint32_t)v20) != 0) {
            v2c++;
            continue;
        }
        v1c = m_f3433C(v20, v54);
        if (v1c < 0) {
            setD8(0xF8558u + 160u * (uint32_t)v48 + (uint32_t)v20, 0x63);
            v2c++;
            continue;
        }
        f7A940();
        v38 = S32(0x1079B4u);
        v34 = S32(0x107D3Cu);
        v14 = f78D08(v20, v54);
        v10 = m_rd8(0xE31C4u + 69u * (uint32_t)(v20 + 1) + (uint32_t)v48);
        if (v10 > v14)
            v10 = v14;
        setD8(vc + 0x5Du, (uint8_t)(D8(vc + 0x5Du) - (uint8_t)v10));
        v44 = m_f3580C(v1c, v20, v54, 0, v1c);
        if (v44 == 0)
            goto L33BE8;
        if (v44 != 1 || D8(0xF488Cu + (uint32_t)v54) == 0 || !(D8(0xE1639u + 35u * (uint32_t)v20) & 0x40) ||
            m_rd8(0xE31C4u + 69u * (uint32_t)(v20 + 1) + (uint32_t)v48) < 6)
            goto L33CF0;
        g_cov[31]++;
    L33BE8:
        if (v44 == 1) {
            int32_t eax = S32(0x1079B4u), edx = 4 * eax;
            eax++;
            setD8(0x107690u + 4u * (uint32_t)eax, 0xC8);
            setD32(0x1079B4u, (uint32_t)eax);
            setD8(0x107695u + (uint32_t)edx, (uint8_t)v20);
            setD8(0xE2C1Au + 3u * (uint32_t)v20, (uint8_t)(D8(0xE2C1Au + 3u * (uint32_t)v20) | 2));
            setD8(0xE2C18u + 3u * (uint32_t)v20, (uint8_t)v54);
            setD16(0xE31EFu + 69u * (uint32_t)(v20 + 1), (uint16_t)(int16_t)(int8_t)D8(0xE31C7u + 69u * (uint32_t)(v20 + 1) + (uint32_t)v48));
        }
        g_cov[32]++;
        f7AD24();
        v30++;
        v2c++;
        if (m_rd8(0xE31C3u + 69u * (uint32_t)(v20 + 1) + (uint32_t)v54) < 0xC)
            continue;
        setD8(0x107ED8u, 1);
        break;
    L33CF0:
        f7AC60();
        v2c++;
        setD32(0x1079B4u, (uint32_t)v38);
        if (g_mut != 8)
            setD32(0x107D3Cu, (uint32_t)v34);
        setD8(0xF8558u + 160u * (uint32_t)v48 + (uint32_t)v20, (uint8_t)v44);
    }
L33CA8:
    if (v30 >= v28 || S32(0x107EE0u) != 0 || D8(0xF14FDu + 199u * (uint32_t)v54) == 0)
        goto L33EBD;
    v3c = m_rd8(0xF1507u + 199u * (uint32_t)v54) - 2 * (m_b5() == 0 ? 9 : 7);
    if (v3c > 3)
        v3c = 3;
    else if (v3c < 0) {
        v3c = m_rd8(0xF1507u + 199u * (uint32_t)v54);
        if (v3c > 3)
            v3c = 3;
    }
    if (v3c <= 0)
        goto L33EBD;
    for (v4c = 0; v4c < 0x9D; v4c++) {
        int32_t c = D8(0xF8558u + 160u * (uint32_t)v48 + (uint32_t)v4c);
        if (c <= 0 || c > v3c)
            continue;
        g_cov[33]++;
        v1c = m_f3433C(v4c, v54);
        if (v1c < 0) {
            setD8(0xF8558u + 160u * (uint32_t)v48 + (uint32_t)v4c, 0x63);
            continue;
        }
        f7A940();
        v38 = S32(0x1079B4u);
        v34 = S32(0x107D3Cu);
        v44 = m_f3580C(v1c - (int32_t)D8(0xF8558u + 160u * (uint32_t)v48 + (uint32_t)v4c), v4c, v54, 0, v1c);
        if (v44 == 0) {
            g_cov[34]++;
            f7AD24();
            v30++;
            continue;
        }
        f7AC60();
        setD32(0x1079B4u, (uint32_t)v38);
        setD32(0x107D3Cu, (uint32_t)v34);
        setD8(0xF8558u + 160u * (uint32_t)v48 + (uint32_t)v4c, (uint8_t)v44);
    }
L33EBD:
    *v40 += v30;
    return v30;
}

/* ---- heap / land lists / routes ----------------------------------------------------------------------------------- */

static int load_landadj(const char *gamedir)
{
    char path[1024];
    FILE *fp;
    uint8_t hdr[4], *blk, w[2];
    uint32_t size, off = 0, h;
    int t, i, n;

    snprintf(path, sizeof path, "%s/LANDADJ.DAT", gamedir);
    fp = fopen(path, "rb");
    if (fp == NULL)
        return 0;
    if (fread(hdr, 1, 4, fp) != 4) {
        fclose(fp);
        return 0;
    }
    size = (uint32_t)hdr[0] | (uint32_t)hdr[1] << 8 | (uint32_t)hdr[2] << 16 | (uint32_t)hdr[3] << 24;
    h = load_heap_alloc(size + 64);
    blk = load_heap_ptr(h, size + 64);
    if (blk == NULL) {
        fclose(fp);
        return 0;
    }
    for (t = 0; t < 160; t++) {
        setD32(0xF8E7Cu + 4u * (uint32_t)t, h + off);
        if (fread(w, 1, 2, fp) != 2 || off + 2 > size + 64) {
            fclose(fp);
            return 0;
        }
        blk[off] = w[0];
        blk[off + 1] = w[1];
        off += 2;
        n = (int16_t)(w[0] | w[1] << 8);
        if (n < 0 || n > 40) {
            fclose(fp);
            return 0;
        }
        g_adj_n[t] = n;
        for (i = 0; i < n; i++) {
            if (fread(w, 1, 2, fp) != 2 || off + 2 > size + 64) {
                fclose(fp);
                return 0;
            }
            blk[off] = w[0];
            blk[off + 1] = w[1];
            off += 2;
            g_adj[t][i] = (int16_t)(w[0] | w[1] << 8);
        }
    }
    fclose(fp);
    return 1;
}

static int load_rl(const char *gamedir, int players)
{
    char name[512];
    FILE *fp;
    uint8_t hdr[4], *p;
    uint32_t m;
    int t, e, s;

    snprintf(name, sizeof name, "%s/%d.RL", gamedir, players);
    fp = fopen(name, "rb");
    if (fp == NULL)
        return 0;
    if (fread(hdr, 4, 1, fp) != 1 || fread(DP(0xFFD44u), 0x2084, 1, fp) != 1 || fread(DP(0x101DC8u), 0x5644, 1, fp) != 1) {
        fclose(fp);
        return 0;
    }
    m = (uint32_t)hdr[0] | (uint32_t)hdr[1] << 8 | (uint32_t)hdr[2] << 16 | (uint32_t)hdr[3] << 24;
    load_heap_free(D32(0x10740Cu));
    setD32(0x10740Cu, load_heap_alloc(m));
    p = load_heap_ptr(D32(0x10740Cu), m);
    if (p == NULL || fread(p, 1, m, fp) != m) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    for (s = 0; s < 2; s++)
        for (t = 0; t < 160; t++) {
            uint32_t tab = s ? 0x101DC8u : 0xFFD44u, idx = s ? 0x107550u : 0x107410u;
            setD16(idx + 2u * (uint32_t)t, 0xFFFF);
            for (e = 0; e < S32(tab); e++)
                if (D8(tab + 4u + 8u * (uint32_t)e) == (uint32_t)t) {
                    setD16(idx + 2u * (uint32_t)t, (uint16_t)e);
                    break;
                }
        }
    return 1;
}

static int load_save(const char *path)
{
    int16_t ver;
    char desc[64];
    FILE *fp = fopen(path, "rb");
    if (fp == NULL)
        return -1;
    f5F05C(&ver, desc, fp);
    fclose(fp);
    printf("save %s: \"%s\", phase %d, round %d, players %d, humans %d, computers %d\n", path, desc, D8(0xF4102u),
           S16(0xF40FCu), S32(0xF40F0u), S8(0xE1620u), S8(0xE1621u));
    return ver;
}

/* ---- random states ------------------------------------------------------------------------------------------------ */

static uint32_t g_rs = 20261121u;
static uint32_t rnd(uint32_t n)
{
    g_rs = g_rs * 1103515245u + 12345u;
    return n ? (g_rs >> 8) % n : 0;
}

#define NBASE 2
static uint8_t *g_base[NBASE], *g_case, *g_after;
static int g_nbase;
static int g_coastal[160], g_ncoastal;

static int pick_nation(void)   /* a nation in play (+0x9A != 0), else 1..9 */
{
    int k, n;
    for (k = 0; k < 20; k++) {
        n = 1 + (int)rnd(9);
        if (D8(NAT(n) + 0x9Au) != 0)
            return n;
    }
    return 1 + (int)rnd(9);
}

static int pick_t(int n)   /* a territory next to one of n's areas, or any */
{
    int c = S8(NAT(n) + 0x60u), a, u;
    if (c > 0 && rnd(4) != 0) {
        a = D8(NAT(n) + 0x61u + rnd((uint32_t)c)) - 1;
        if (a >= 0 && a <= 156) {
            if (rnd(3) == 0)
                return a;
            if (g_adj_n[a] > 0) {
                u = g_adj[a][rnd((uint32_t)g_adj_n[a])];
                if (u >= 0 && u <= 156)
                    return u;
            }
        }
    }
    return (int)rnd(157);
}

static void randomize(void)
{
    int n, t, i, k, m;
    uint16_t f;

    f = D16(0xE1624u);
    f = (uint16_t)((f & ~(3u << 10) & ~(1u << 5)) | rnd(4) << 10 | rnd(2) << 5);
    setD16(0xE1624u, f);
    for (n = 1; n <= 9; n++) {
        uint32_t r = NAT(n);
        if (rnd(6) == 0)
            setD8(r + 0x9Au, (uint8_t)(rnd(2) ? 2 : 3));
        setD8(r + 0x26u, (uint8_t)(rnd(3) == 0));
        setD8(r + 0x27u, (uint8_t)(rnd(3) == 0));
        setD8(r + 0x28u, (uint8_t)(rnd(3) == 0));
        setD8(r + 0x31u, (uint8_t)(rnd(3) == 0));
        setD8(r + 0x3Eu, (uint8_t)rnd(32));
        setD8(r + 0x5Cu, (uint8_t)rnd(16));
        setD8(r + 0x51u, (uint8_t)rnd(rnd(4) == 0 ? 10 : 5));
        setD8(r + 0x98u, (uint8_t)rnd(56));
        k = (int)rnd(5);
        setD8(r + 0x40u, (uint8_t)(g_ncoastal ? k : 0));
        for (i = 0; i < 4; i++) {
            setD16(r + 0x45u + 2u * (uint32_t)i, (uint16_t)(g_ncoastal ? g_coastal[rnd((uint32_t)g_ncoastal)] + 1 : 0));
            setD8(r + 0x41u + (uint32_t)i, (uint8_t)rnd(7));
        }
    }
    /* tokens */
    for (k = (int)rnd(60); k > 0; k--) {
        n = rnd(8) == 0 ? 10 + (int)rnd(2) : 1 + (int)rnd(9);
        t = (int)rnd(157);
        if (rnd(2) == 0 && S8(NAT(n < 10 ? n : 1) + 0x60u) > 0 && n < 10) {
            t = D8(NAT(n) + 0x61u + rnd((uint32_t)S8(NAT(n) + 0x60u))) - 1;
            if (t < 0 || t > 156)
                t = (int)rnd(157);
            if (g_adj_n[t] > 0 && rnd(2) == 0 && g_adj[t][0] <= 156)
                t = g_adj[t][rnd((uint32_t)g_adj_n[t])];
            if (t < 0 || t > 156)
                t = (int)rnd(157);
        }
        f57E28(t, n, 1 + (int)rnd(rnd(5) == 0 ? 16 : 6));
    }
    for (n = 1; n <= 11; n++)
        f460C0(n);
    for (k = (int)rnd(20); k > 0; k--) {   /* some tokens already moved */
        uint32_t a = AREA(1 + rnd(157)) + 0x24u + 1u + rnd(9);
        setD8(a, (uint8_t)rnd(3));
    }
    for (n = 1; n <= 9; n++)
        setD8(NAT(n) + 0x5Du, (uint8_t)(rnd(5) == 0 ? (int)rnd(3) - 1 : (int)rnd(25)));
    /* ranks, relations, tables */
    for (n = 1; n <= 9; n++)
        setD8(0xF06C0u + (uint32_t)n, (uint8_t)n);
    for (n = 9; n > 1; n--) {
        int j = 1 + (int)rnd((uint32_t)n);
        uint8_t x = D8(0xF06C0u + (uint32_t)n);
        setD8(0xF06C0u + (uint32_t)n, D8(0xF06C0u + (uint32_t)j));
        setD8(0xF06C0u + (uint32_t)j, x);
    }
    for (i = 0; i < 100; i++)
        setD32(0xF52E0u + 4u * (uint32_t)i, (uint32_t)((int32_t)rnd(160) - 80));
    for (i = 0; i < 4; i++)   /* past the table (enemy 10) */
        setD32(0xF5470u + 4u * (uint32_t)i, (uint32_t)((int32_t)rnd(160) - 80));
    for (i = 0; i < 300; i++)
        setD16(0x112A94u + 2u * (uint32_t)i, (uint16_t)rnd(30000));
    for (n = 0; n <= 9; n++)
        setD8(0xF488Cu + (uint32_t)n, (uint8_t)rnd(3));
    for (i = 0; i < 1600; i++) {
        setD8(0xEFA04u + (uint32_t)i, (uint8_t)((int)rnd(40) - 10));
        setD8(0xEED84u + (uint32_t)i, (uint8_t)((int)rnd(120) - 10));
    }
    for (i = 0; i < 9 * 160; i++) {
        m = (int)rnd(20);
        setD8(0xF8558u + (uint32_t)i, (uint8_t)(m < 14 ? 0 : m < 18 ? (int)rnd(5) + 1 : 0x63));
    }
    for (i = 0; i < 9 * 157; i++)
        setD16(0xF3580u + 2u * (uint32_t)i, (uint16_t)(rnd(3) == 0 ? 0x80 : 0));
    for (t = 0; t < 157; t++) {
        uint8_t b = D8(TST(t) + 2u);
        if (rnd(12) == 0) {
            b = (uint8_t)(b | 2);
            setD8(TST(t), (uint8_t)(1 + rnd(9)));
        } else if (rnd(10) == 0)
            b = (uint8_t)(b & ~2u);
        setD8(TST(t) + 2u, b);
        setD8(0xEE16Cu + (uint32_t)t, (uint8_t)(rnd(15) != 0));
    }
    /* the shadow tables: a copy of the board with changes */
    memcpy(DP(0xF09E8u), DP(0xF14CCu), 199u * 14u);
    memcpy(DP(0xEB3F0u), DP(0xE31B8u), 69u * 161u);
    for (k = (int)rnd(80); k > 0; k--)
        setD8(0xEB3F0u + 69u * (1u + rnd(160)) + 2u + rnd(0x43), (uint8_t)rnd(7));
    for (k = (int)rnd(20); k > 0; k--) {   /* fields only: the ship count +0x40 and the lists stay valid */
        static const uint8_t off[] = { 0x26, 0x27, 0x28, 0x31, 0x3E, 0x51, 0x5C, 0x5D, 0x98, 0x99 };
        setD8(0xF09E8u + 199u * (1u + rnd(9)) + off[rnd(sizeof off)], (uint8_t)rnd(30));
    }
    setD32(0x107EE0u, (uint32_t)(rnd(5) == 0));
    setD32(0x107EDCu, (uint32_t)(rnd(5) == 0));
    setD8(0x107ED8u, (uint8_t)(rnd(4) == 0));
    setD32(0x1079B4u, rnd(12));
    setD32(0x107D3Cu, rnd(12));
    for (i = 0; i < 0x320; i++)   /* stale records */
        setD8(0x107694u + (uint32_t)i, (uint8_t)rnd(256));
    setD16(0xBB3F0u, (uint16_t)(rnd(3) != 0));
    setD32(0xF5908u, rnd(1000));
}

/* ---- the differential runs ------------------------------------------------------------------------------------------ */

enum { F337C8, F33ECC, F3433C, F348B0, F3566C, F353C4, F3580C, F33964, F79128, NF };
static const char *k_names[NF] = { "f337C8", "f33ECC", "f3433C", "f348B0", "f3566C", "f353C4", "f3580C", "f33964",
                                   "f79128" };
static long g_runs[NF], g_bad[NF];

typedef struct {
    int32_t ret, o1, o2;
    uint8_t list[0x198];
} result_t;

static void call(int side, int f, const int *a, result_t *r)
{
    int32_t x = a[5], y = a[6];
    memset(r, 0, sizeof *r);
    g_side = side;
    if (side == 0) {
        switch (f) {
        case F337C8: r->ret = f337C8(a[0]); break;
        case F33ECC: f33ECC(&x, r->list, a[0], a[1]); r->o1 = x; break;
        case F3433C: r->ret = f3433C(a[0], a[1]); break;
        case F348B0: r->ret = f348B0(a[0], a[1], &x, &y); r->o1 = x; r->o2 = y; break;
        case F3566C: f3566C(a[0]); break;
        case F353C4: f353C4(a[0]); break;
        case F3580C: r->ret = f3580C(a[0], a[1], a[2], a[3], a[4]); break;
        case F33964: r->ret = f33964(a[0], &x, a[2], a[3]); r->o1 = x; break;
        default: r->ret = f79128(a[0], a[1], a[2]); break;
        }
    } else {
        switch (f) {
        case F337C8: r->ret = m_f337C8(a[0]); break;
        case F33ECC: m_f33ECC(&x, r->list, a[0], a[1]); r->o1 = x; break;
        case F3433C: r->ret = m_f3433C(a[0], a[1]); break;
        case F348B0: r->ret = m_f348B0(a[0], a[1], &x, &y); r->o1 = x; r->o2 = y; break;
        case F3566C: m_f3566C(a[0]); break;
        case F353C4: m_f353C4(a[0]); break;
        case F3580C: r->ret = m_f3580C(a[0], a[1], a[2], a[3], a[4]); break;
        case F33964: r->ret = m_f33964(a[0], &x, a[2], a[3]); r->o1 = x; break;
        default: r->ret = m_f79128(a[0], a[1], a[2]); break;
        }
    }
}

static void make_args(int f, int *a)
{
    int n = pick_nation();
    memset(a, 0, 7 * sizeof *a);
    switch (f) {
    case F337C8: case F3566C: case F353C4: a[0] = f == F3566C ? 1 + (int)rnd(9) : n; break;
    case F33ECC: a[0] = n; a[1] = (int)rnd(3); break;
    case F3433C: a[0] = pick_t(n); a[1] = n; break;
    case F348B0: a[0] = pick_t(n); a[1] = n; a[5] = rnd(2) ? 0 : (int)rnd(12); a[6] = (int)rnd(50) - 5; break;
    case F3580C: a[0] = (int)rnd(rnd(4) == 0 ? 30 : 14); a[1] = pick_t(n); a[2] = n; a[3] = (int)rnd(3); a[4] = (int)rnd(7); break;
    case F33964: a[0] = (int)rnd(6) - 1; a[2] = n; a[3] = (int)rnd(3); a[5] = (int)rnd(4); break;
    default: a[0] = pick_t(n); a[1] = pick_t(n); a[2] = n; break;
    }
}

static int first_diff(const uint8_t *x, const uint8_t *y)
{
    uint32_t i;
    for (i = 0; i < DS_SIZE; i++)
        if (x[i] != y[i])
            return (int)i;
    return -1;
}

static const char *trace_diff(void)
{
    static char b[600];
    const char *p = g_tr[0].buf, *q = g_tr[1].buf;
    int line = 1;
    while (*p && *p == *q) {
        if (*p == '\n')
            line++;
        p++;
        q++;
    }
    while (p > g_tr[0].buf && p[-1] != '\n') {
        p--;
        q--;
    }
    snprintf(b, sizeof b, "line %d: port \"%.120s\" model \"%.120s\"", line, p, q);
    return b;
}

/* one case; 1 = equal. verbose prints the first difference */
static int run_case(int f, int verbose)
{
    int a[7], d;
    result_t rp, rm;
    int ok = 1;
    int saved_depth_w, saved_depth_m;

    memcpy(g_ds, g_base[rnd((uint32_t)g_nbase)], DS_SIZE);
    randomize();
    make_args(f, a);
    memcpy(g_case, g_ds, DS_SIZE);
    saved_depth_w = S32(0xCAC54u);
    saved_depth_m = S32(0xCAC58u);
    tr_reset();
    if (setjmp(g_quit_jb) != 0)
        return 0;
    call(0, f, a, &rp);
    memcpy(g_after, g_ds, DS_SIZE);
    memcpy(g_ds, g_case, DS_SIZE);
    call(1, f, a, &rm);
    d = first_diff(g_after, g_ds);
    if (d >= 0) {
        ok = 0;
        if (verbose)
            printf("  %s(%d,%d,%d,%d,%d) data differ at 0x%X: port %02X model %02X\n", k_names[f], a[0], a[1], a[2],
                   a[3], a[4], (unsigned)(DS_BASE + (uint32_t)d), g_after[d], g_ds[d]);
    }
    if (rp.ret != rm.ret || rp.o1 != rm.o1 || rp.o2 != rm.o2 || memcmp(rp.list, rm.list, sizeof rp.list) != 0) {
        ok = 0;
        if (verbose)
            printf("  %s(%d,%d,%d,%d,%d): port %d %d %d, model %d %d %d\n", k_names[f], a[0], a[1], a[2], a[3], a[4],
                   rp.ret, rp.o1, rp.o2, rm.ret, rm.o1, rm.o2);
    }
    if (g_tr[0].hash != g_tr[1].hash || g_tr[0].lines != g_tr[1].lines) {
        ok = 0;
        if (verbose)
            printf("  %s trace differs (%ld / %ld lines): %s\n", k_names[f], g_tr[0].lines, g_tr[1].lines, trace_diff());
    }
    if (S32(0xCAC54u) != saved_depth_w || S32(0xCAC58u) != saved_depth_m) {
        ok = 0;
        if (verbose)
            printf("  %s: stack depths not restored\n", k_names[f]);
    }
    return ok;
}

static void part_a(int per_fn)
{
    int f, i, shown;
    char buf[120];
    for (f = 0; f < NF; f++) {
        shown = 0;
        for (i = 0; i < per_fn; i++) {
            g_runs[f]++;
            if (!run_case(f, shown < 3)) {
                g_bad[f]++;
                shown++;
            }
        }
        snprintf(buf, sizeof buf, "A. %s: %d random runs equal the model (g_original %d)", k_names[f], per_fn,
                 g_original);
        check(g_bad[f] == 0, buf);
    }
}

/* ---- B. hand vectors ------------------------------------------------------------------------------------------------ */

static void part_b(void)
{
    int32_t cnt, c;
    int r, t, n = 2, ok;
    char buf[200];

    memcpy(g_ds, g_base[0], DS_SIZE);
    tr_reset();
    g_side = 0;
    /* f337C8 by hand: stock 55 -> 27; Crete 25; tokens 30, cities 2: k 2 -> tokens 30, 24 (k 3), 18 stops (not > 18):
       e = 2 */
    setD32(0xF4898u, 55);
    setD16(0xE1624u, (uint16_t)(D16(0xE1624u) & ~(3u << 10)));   /* level 0: lim -5 when roll/300 < 10 */
    setD16(0x112AE2u + 2u * 5u, 29999);                           /* row 4, Crete: 99 */
    setD8(NAT(5) + 0x98u, 30);
    setD8(NAT(5) + 0x51u, 2);
    check(f337C8(5) == 2, "B. f337C8: Crete, 30 tokens, 2 cities -> 2");
    setD8(NAT(5) + 0x98u, 40);   /* 40 -> 34 (3) -> 28 (4) ... 2k <= tokens: k 2: 4<=40 t34 k3; 6<=34 t28 k4; 8<=28 t22 k5; 10<=22 t16 k6: stop -> e 4 -> 3 */
    setD16(0x112AF6u + 2u * 5u, 0);   /* row 5, Crete (int16 0x112AF6 + 2n): 0 < 25 -> 2 */
    r = f337C8(5);
    check(r == 2, "B. f337C8: e = 3 cut to 2 by the row-5 roll");
    setD16(0x112AF6u + 2u * 5u, 29999);
    check(f337C8(5) == 3, "B. f337C8: e = 3 kept");
    setD8(NAT(5) + 0x51u, 8);
    check(f337C8(5) == 1, "B. f337C8: capped at 9 cities");
    setD8(NAT(5) + 0x98u, 20);   /* lim 25 (no -5): 20 < 25 */
    setD8(NAT(5) + 0x60u, 0);
    check(f337C8(5) == 0, "B. f337C8: short of tokens, 20 <= 21, nothing shared -> 0");
    setD8(NAT(5) + 0x98u, 22);
    check(f337C8(5) == 1, "B. f337C8: short of tokens, 22 > 21 -> 1");

    /* f3580C's mode-2 record: at 0x107694 + 4 * count, +1 = owner of t, +2 = 0, +3 untouched */
    memcpy(g_ds, g_base[0], DS_SIZE);
    for (t = 0; t < 157; t++)
        for (c = 1; c <= 11; c++)
            setD8(AREA(t + 1) + 0x24u + (uint32_t)c, 0);
    setD8(NAT(n) + 0x5Du, 0);   /* no sweeps */
    setD8(NAT(n) + 0x28u, 0);
    setD16(0xBB3F0u, 0);
    t = 30;
    setD8(TST(t), 7);
    setD32(0x1079B4u, 5);
    memset(DP(0x107694u), 0xAB, 0x40);
    r = f3580C(0, t, n, 2, 3);
    ok = r == 0 && S32(0x1079B4u) == 6 && D8(0x107694u + 20u) == 0xC9 && D8(0x107695u + 20u) == 7 &&
         D8(0x107696u + 20u) == 0 && D8(0x107697u + 20u) == 0xAB && D8(0x107694u + 5u) == 0xAB;
    check(ok, "B. f3580C mode 2: the 0xC9 record at 4 * count (not the inventory's unscaled index), +3 untouched");
    setD32(0x1079B4u, 5);
    r = f3580C(4, t, n, 2, 1);   /* start - need = 0 < 1 -> need */
    check(r == 4 && S32(0x1079B4u) == 5, "B. f3580C mode 2: short of s -> the need, no record");
    /* mode 0 with need 0: the 0xC8 record, the planned site */
    setD32(0x1079B4u, 2);
    setD8(0x107ED8u, 0);
    r = f3580C(0, t, n, 0, 0);
    snprintf(buf, sizeof buf, "B. f3580C mode 0, need 0: 0xC8 record, planned site of n (r %d, count %d)", r,
             S32(0x1079B4u));
    check(r == 0 && S32(0x1079B4u) == 3 && D8(0x107694u + 8u) == 0xC8 && D8(0x107695u + 8u) == (uint8_t)t &&
          D8(0x107696u + 8u) == 0 && D8(0x107697u + 8u) == 0xAB && D8(TST(t)) == (uint8_t)n &&
          (D8(TST(t) + 2u) & 2) && S16(AREA(t + 1) + 0x37u) == tok(t, n), buf);

    /* f79128 on the real map: every result is a common neighbour usable by n, or -1 when none is */
    memcpy(g_ds, g_base[0], DS_SIZE);
    ok = 1;
    for (t = 0; t < 157 && ok; t++) {
        int u, i, j;
        for (u = 0; u < 157; u++) {
            int got = f79128(t, u, n), exp = -1;
            for (i = 0; i < g_adj_n[t] && exp < 0; i++)
                for (j = 0; j < g_adj_n[u]; j++)
                    if (g_adj[u][j] == g_adj[t][i]) {
                        int v = g_adj[t][i];
                        if (((D8(TST(v) + 2u) & 1) && D8(TST(v)) == (uint32_t)n) ||
                            (S8(AREA(v + 1) + 1u) == 0 && !(D8(TST(v) + 2u) & 1)) ||
                            (S8(AREA(v + 1) + 1u) == 1 && S8(AREA(v + 1) + 1u + (uint32_t)n) == n)) {
                            exp = v;
                            break;
                        }
                    }
            if (got != exp) {
                printf("  f79128(%d, %d, %d) = %d, expected %d\n", t, u, n, got, exp);
                ok = 0;
                break;
            }
        }
    }
    check(ok, "B. f79128 on the real land lists: the first usable common neighbour in t1's order, else -1");

    /* f33964's garbage path: cities 8 + one planned site >= 9 -> nothing planned, *count unchanged */
    memcpy(g_ds, g_base[0], DS_SIZE);
    setD8(NAT(n) + 0x51u, 8);
    setD8(NAT(n) + 0x31u, 0);   /* no Architecture retry */
    t = D8(NAT(n) + 0x61u) - 1;
    if (S8(NAT(n) + 0x60u) > 0 && t >= 0) {
        setD8(TST(t) + 2u, (uint8_t)(D8(TST(t) + 2u) | 2));
        if (tok(t, n) < 3)
            f57E28(t, n, 3);
        cnt = 0;
        r = f33964(1, &cnt, n, 0);
        check(r == 0 && cnt == 0, "B. f33964: cities + planned >= 9 -> 0 (the original adds stack garbage, fixed)");
    } else
        printf("B. f33964 garbage case skipped (nation %d holds no area)\n", n);
}

/* ---- C. mutants ---------------------------------------------------------------------------------------------------- */

static void part_c(int per_fn)
{
    static const struct { int mut, f; const char *what; } k_mut[] = {
        { 1, F337C8, "f337C8: the 18-token floor read as 17" },
        { 2, F33ECC, "f33ECC: priorities sorted ascending" },
        { 3, F3433C, "f3433C: no cap of the threat when a fight is expected" },
        { 4, F348B0, "f348B0: road moves counted in full in A" },
        { 5, F3580C, "f3580C: the first guarded sweep ignores territory flag 0x04" },
        { 6, F3580C, "f3580C: the mode-2 record at the unscaled index (the inventory's claim)" },
        { 7, F353C4, "f353C4: column +0x3C not copied" },
        { 8, F33964, "f33964: list 2's count not restored after a failed trial" },
        { 9, F3580C, "f3580C: the look-ahead restores the snapshot instead of peeking the stacks" },
        { 10, F79128, "f79128: the first common neighbour, usable or not" },
        { 7, F3566C, "f3566C: column +0x3C not restored" },
        { 2, F33964, "f33964 through f33ECC: priorities ascending" },
    };
    int m, i, caught;
    char buf[160];
    for (m = 0; m < (int)(sizeof k_mut / sizeof k_mut[0]); m++) {
        g_mut = k_mut[m].mut;
        caught = 0;
        for (i = 0; i < per_fn && !caught; i++)
            caught = !run_case(k_mut[m].f, 0);
        snprintf(buf, sizeof buf, "C. mutant %d caught (%s) after %d runs", g_mut, k_mut[m].what, i);
        check(caught, buf);
        if (caught)
            printf("%s\n", buf);
    }
    g_mut = 0;
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:/work/advciv_run";
    const char *savedir = argc > 2 ? argv[2] : "C:/work/advciv_play";
    char cwd[400], path[400];
    int i, t, players;

    setvbuf(stdout, NULL, _IONBF, 0);
    if (state_init(gamedir) != 0)
        return 1;
    for (i = 0; i < 2; i++) {
        g_tr[i].buf = malloc(TRSZ);
        if (g_tr[i].buf == NULL)
            return 1;
    }
    g_case = malloc(DS_SIZE);
    g_after = malloc(DS_SIZE);
    for (i = 0; i < NBASE; i++)
        g_base[i] = malloc(DS_SIZE);
    if (g_case == NULL || g_after == NULL || g_base[NBASE - 1] == NULL)
        return 1;
    if (getcwd(cwd, sizeof cwd) == NULL)
        cwd[0] = 0;
    if (chdir(gamedir) == 0) {   /* terrflag.dat: the territory records are not in the save */
        f12394();
        (void)chdir(cwd);
    }
    if (!load_landadj(gamedir)) {
        printf("cannot read %s/LANDADJ.DAT\n", gamedir);
        return 1;
    }
    f7A8D0();   /* the World State Stack's blocks */
    printf("stack depths at start: world %d, move list %d\n", S32(0xCAC54u), S32(0xCAC58u));

    snprintf(path, sizeof path, "%s/savgame0.dat", gamedir);
    if (load_save(path) >= 0) {
        players = S32(0xF40F0u);
        if (!load_rl(gamedir, players))
            printf("cannot load %d.RL\n", players);
        memcpy(g_base[g_nbase++], g_ds, DS_SIZE);
    }
    snprintf(path, sizeof path, "%s/SAVGAMEA.DAT", savedir);
    if (load_save(path) >= 0) {
        players = S32(0xF40F0u);
        if (!load_rl(gamedir, players))
            printf("cannot load %d.RL\n", players);
        memcpy(g_base[g_nbase++], g_ds, DS_SIZE);
    }
    if (g_nbase == 0) {
        printf("no save found\n");
        return 1;
    }
    for (t = 0; t < 157; t++)
        if (D8(TERR(t)) & 2)
            g_coastal[g_ncoastal++] = t;
    printf("%d coastal territories; %d bases\n", g_ncoastal, g_nbase);

    part_b();
    printf("B. %d fail so far\n", g_fail);
    part_a(3000);
    g_original = 1;
    part_a(500);
    g_original = 0;
    printf("A. %d fail so far; runs:", g_fail);
    for (i = 0; i < NF; i++)
        printf(" %s %ld", k_names[i], g_runs[i]);
    printf("\ncoverage:");
    for (i = 0; i < 35; i++)
        printf(" %d:%ld", i, g_cov[i]);
    printf("\n");
    part_c(3000);
    check(g_ui_calls == 0, "no screen routine called");
    printf("%d checks, %d fail, %d quits, %ld screen calls\n", g_tests, g_fail, g_quits, g_ui_calls);
    return g_fail == 0 ? 0 : 1;
}
