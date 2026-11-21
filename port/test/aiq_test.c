/* aiq_test [GAMEDIR [SAVEDIR]]: unit tests for module AIQ (aiq.c) that need no game run.

   It loads the data object from GAMEDIR/AC_MAIN.EXE as the port does (state_init; GAMEDIR defaults to the run copy
   C:\work\advciv_run, only read), the territory records from GAMEDIR's terrflag.dat (f12394: the population limits and
   site bits are not in the save), the land lists from SAVEDIR's LANDADJ.DAT (an independent loader that drops the
   neighbours not in play as f3EED8 does, into heap blocks as gfxload.c stores them) and the route tables of
   GAMEDIR/<players>.RL (as f3EED8 loads them), so the ported f5843C sees real routes. aiq.c is compiled with
   -DAIQ_TEST: its one call into unported code (0x4D29C, AISEA) goes to a recorder here that returns a value made from
   the state. f5B588 (conflict.c) is replaced by a recording fake that changes the area record, the losses 0xF547C and
   the nation records 1..11 deterministically (so a missing restore shows), and logs [0xBB3F8] at each call. The other
   helpers are the real, separately tested ones: rules.c (f78D08, f7C760, f7C7C4), formulas.c (f7CAB4), route.c
   (f5843C), mstack.c (f7A778 / f7A7B0); move.c is linked for rules.c, with its UI callees as stubs that must not run.
     A. random differential runs of all nineteen routines against a second implementation written here from the
        listing (not from aiq.c), on a copy of the data object (the model runs with g_ds swapped to its copy and uses
        the same verified helpers there). States: the two real saves below with random areas (nations, tokens,
        markers, movable tokens, marks, inconsistent counts), territory states, territory bits, the AI flags 0xF3580,
        the .BPM flags 0xF0044, the scores 0xEED84 / 0xEF3C4 / 0xEFA04, the movement order 0xF06C0, the matrix 0xF52E0,
        the pre-rolled table 0x112A94 (with values near 30000), the AI level, nation records (advances, ships, the area
        list, AST square, kinds), the move list 0x107694, the switches 0xF488C / 0x108164 / 0xF09E4 / [0xF5470], in-play
        bytes. After every call: the result, the output lists and cells, the fakes' logs, and the whole data object
        must equal the model's. Both values of g_original.
     B. hand vectors from the listing: f7701C's two-row pick and BUG(1995) 1 in both modes; f46998's t = 157 (the next
        row's flag, area record 158); f39C08's contested site never counted while the trial runs; f7C838's restore
        (everything but 0xF547C); f495C0's scan after the trial and its restore; f48028's empty jump-table cases;
        f4876C appending; f49790's limit 0; f48D44's flag 0x10 counting only.
     C. the saves: SAVEDIR/SAVGAMEA.DAT (the R4 autosave) and GAMEDIR/savgame0.dat (the start of MOVEMENT R4), read
        with saveload.c's f5F05C: the start of f461BC for every computer (f7BD5C, f46998, f160EC as at 0x46220..
        0x46230), then f39C08, f7B044 and f468EC, against the model; the numbers printed for the DOSBox-X plan, and
        the .BPM 0x04 counts per nation (f160EC's unbounded list).
     D. mutations: twenty deliberately broken models, each must be caught by part A's runs.
   Prints each failure (the first few), the counts per part, and exits 0 when all pass.

   Build (from the repo root; any C11 compiler):
     clang -std=c11 -Wall -Wextra -DAIQ_TEST -Iport/src port/test/aiq_test.c port/src/aiq.c port/src/rules.c
       port/src/move.c port/src/formulas.c port/src/route.c port/src/load.c port/src/mstack.c port/src/saveload.c
       port/src/state.c port/src/le_image.c -o aiq_test.exe */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <direct.h>
#define chdir _chdir
#define getcwd _getcwd

#include "../src/state.h"
#include "../src/game.h"
#include "../src/ui.h"
#include "../src/aiq.h"
#include "../src/conflict.h"
#include "../src/formulas.h"
#include "../src/map.h"
#include "../src/mapdata.h"
#include "../src/move.h"
#include "../src/mstack.h"
#include "../src/panel.h"
#include "../src/route.h"
#include "../src/rules.h"
#include "../src/saveload.h"

static long g_rand_calls;   /* game_rand: AIQ draws none (D) */

/* ---- stubs: quit, messages, and move.c's UI callees (never reached here) ------------------------------------------ */

_Noreturn void f44CAC(void) { fprintf(stderr, "aiq_test: quit: %s\n", DSTR(0xF5500u)); exit(2); }
void f3A780(const char *msg) { fprintf(stderr, "aiq_test: message: %s\n", msg); exit(2); }
static _Noreturn void ui_called(const char *name) { fprintf(stderr, "aiq_test: unexpected UI call %s\n", name); exit(4); }
int  fg_vbopen(int h) { (void)h; ui_called("fg_vbopen"); }
void fg_vbclose(void) { ui_called("fg_vbclose"); }
void f3B7CC(uint32_t d, int i) { (void)d; (void)i; ui_called("f3B7CC"); }
void f3B870(uint32_t d, int i) { (void)d; (void)i; ui_called("f3B870"); }
int  f3B9A4(uint32_t d, int i) { (void)d; (void)i; ui_called("f3B9A4"); }
void f3BFBC(uint32_t d, int i, int e) { (void)d; (void)i; (void)e; ui_called("f3BFBC"); }
void f3C3D0(uint32_t d, int i) { (void)d; (void)i; ui_called("f3C3D0"); }
void f3CBBC(void) { ui_called("f3CBBC"); }
void f3D52C(int dx, int dy) { (void)dx; (void)dy; ui_called("f3D52C"); }
void f3DA60(int t) { (void)t; ui_called("f3DA60"); }
int  f3E89C(int t) { (void)t; ui_called("f3E89C"); }
int  f3E8FC(int t) { (void)t; ui_called("f3E8FC"); }
void f3F624(int t, int c) { (void)t; (void)c; ui_called("f3F624"); }
void f3F670(void) { ui_called("f3F670"); }
void f3F698(int t, int c) { (void)t; (void)c; ui_called("f3F698"); }
void f3F70C(int t, int c) { (void)t; (void)c; ui_called("f3F70C"); }
void f426D0(int i) { (void)i; ui_called("f426D0"); }
void f572CC(int t) { (void)t; ui_called("f572CC"); }
void f76F70(int n, uint8_t *cards) { (void)n; (void)cards; ui_called("f76F70"); }
int  f78AE4(void)   /* game_rand (game.h's LCG), counted: no routine of AIQ may draw */
{
    g_rand_calls++;
    setD32(GAME_SEED, D32(GAME_SEED) * 0x15A4E35u + 1u);
    return (int)((D32(GAME_SEED) >> 16) & 0x7FFF);
}

/* ---- bookkeeping ------------------------------------------------------------------------------------------------- */

static long g_tests, g_fail;

static int check(int ok, const char *fmt, ...)
{
    g_tests++;
    if (!ok) {
        g_fail++;
        if (g_fail <= 40) {
            va_list ap;
            printf("FAIL ");
            va_start(ap, fmt);
            vprintf(fmt, ap);
            va_end(ap);
            printf("\n");
        }
    }
    return ok;
}

static uint32_t g_rs = 20261121u;
static uint32_t rnd(void) { g_rs = g_rs * 1664525u + 1013904223u; return g_rs >> 8; }
static int rn(int n) { return n > 0 ? (int)(rnd() % (uint32_t)n) : 0; }

/* ---- the fakes' logs: the port's calls and the model's must be the same ------------------------------------------ */

static char g_log[2][8192];
static int  g_side;   /* 0 the port runs, 1 the model */

static void logf_(const char *fmt, ...)
{
    size_t len = strlen(g_log[g_side]);
    va_list ap;
    if (len > sizeof g_log[0] - 64)
        return;
    va_start(ap, fmt);
    vsnprintf(g_log[g_side] + len, sizeof g_log[0] - len, fmt, ap);
    va_end(ap);
}

/* f5B588 fake: deterministic losses in area record r, the losses table, the nation records (restored by f7A778 /
   f7A7B0 in the routines under test), the Pirates removed; logs r and [0xBB3F8] */
void f5B588(int r)
{
    uint32_t rec = 0xE31B8u + 69u * (uint32_t)r;
    int m;
    logf_("B(%d,%d) ", r, D8(0xBB3F8u));
    for (m = 1; m <= 11; m++) {
        int tok = (int8_t)D8(rec + 0x0Eu + (uint32_t)m), loss;
        if (tok <= 0)
            continue;
        loss = (r * 7 + m * 3 + tok) % 3;
        if (m == 10)
            loss = tok;
        if (loss > tok)
            loss = tok;
        setD8(rec + 0x0Eu + (uint32_t)m, (uint8_t)(tok - loss));
        if (tok - loss == 0 && (int8_t)D8(rec + 1u + (uint32_t)m) == m) {
            setD8(rec + 1u + (uint32_t)m, 0);
            setD8(rec + 1u, (uint8_t)(D8(rec + 1u) - 1));
        }
        setD8(0xF547Cu + (uint32_t)m, (uint8_t)(D8(0xF547Cu + (uint32_t)m) + loss));
        setD8(0xF14CCu + 199u * (uint32_t)m + 0x98u, (uint8_t)(D8(0xF14CCu + 199u * (uint32_t)m + 0x98u) - loss));
        if (D8(0xBB3F8u) != 0)
            setD8(0xF14CCu + 199u * (uint32_t)m + 0x99u, (uint8_t)(D8(0xF14CCu + 199u * (uint32_t)m + 0x99u) + loss));
        setD8(0xF14CCu + 199u * (uint32_t)m + 0x61u, (uint8_t)(r + m));
    }
}

/* the AISEA recorder of aiq.c (AIQ_TEST): a value made from the state, logged */
int aiq_test_f4D29C(int t, int n)
{
    int v = ((int8_t)D8(0xE31B8u + 69u * (uint32_t)(t + 1) + 0x0Eu + (uint32_t)n) + t + 3 * n) % 4;
    if (v < 0)
        v = -v;
    logf_("S(%d,%d)=%d ", t, n, v);
    return v;
}

/* ---- land lists and routes ---------------------------------------------------------------------------------------- */

#define ADJ_W 24
static int16_t g_adj[160][ADJ_W];     /* the model's copy: [t][0] count, then the neighbours (after the in-play drop) */
static int16_t g_raw[160][ADJ_W];
static uint32_t g_adj_block;

static int load_adj(const char *dir)
{
    char path[400];
    FILE *fp;
    uint8_t b[4];
    int t, i, cnt;
    snprintf(path, sizeof path, "%s/LANDADJ.DAT", dir);
    fp = fopen(path, "rb");
    if (fp == NULL)
        return -1;
    if (fread(b, 4, 1, fp) != 1) { fclose(fp); return -1; }
    for (t = 0; t < 160; t++) {
        if (fread(b, 2, 1, fp) != 1) { fclose(fp); return -1; }
        cnt = (int16_t)(b[0] | b[1] << 8);
        if (cnt > ADJ_W - 1) {
            printf("LANDADJ.DAT: territory %d has %d neighbours, more than the test keeps\n", t, cnt);
            fclose(fp);
            return -1;
        }
        g_raw[t][0] = (int16_t)cnt;
        for (i = 0; i < cnt; i++) {
            if (fread(b, 2, 1, fp) != 1) { fclose(fp); return -1; }
            g_raw[t][1 + i] = (int16_t)(b[0] | b[1] << 8);
        }
    }
    fclose(fp);
    return 0;
}

/* the lists without the neighbours not in play (0x3F1FE..), into one heap block, record t at [0xF8E7C + 4t] */
static void install_adj(void)
{
    int t, i, k;
    if (g_adj_block == 0)
        g_adj_block = load_heap_alloc(160u * 2u * ADJ_W);
    for (t = 0; t < 160; t++) {
        uint8_t *p = load_heap_ptr(g_adj_block + 2u * ADJ_W * (uint32_t)t, 2u * ADJ_W);
        k = 0;
        for (i = 1; i <= g_raw[t][0]; i++)
            if (D8(GAME_INPLAY + (uint32_t)g_raw[t][i]) != 0)
                g_adj[t][1 + k++] = g_raw[t][i];
        g_adj[t][0] = (int16_t)k;
        for (i = 0; i < ADJ_W; i++) {
            p[2 * i] = (uint8_t)g_adj[t][i];
            p[2 * i + 1] = (uint8_t)((uint16_t)g_adj[t][i] >> 8);
        }
        setD32(GAME_LANDADJ_PTR + 4u * (uint32_t)t, g_adj_block + 2u * ADJ_W * (uint32_t)t);
    }
}

static int load_rl(const char *gamedir, int players)
{
    char name[512];
    FILE *fp;
    uint8_t hdr[4], *p;
    uint32_t m;
    int s, t, e;
    snprintf(name, sizeof name, "%s/%d.RL", gamedir, players);
    fp = fopen(name, "rb");
    if (fp == NULL)
        return -1;
    if (fread(hdr, 4, 1, fp) != 1 || fread(DP(GAME_RL_TABLE1), 0x2084, 1, fp) != 1
        || fread(DP(GAME_RL_TABLE2), 0x5644, 1, fp) != 1) {
        fclose(fp);
        return -1;
    }
    m = (uint32_t)hdr[0] | (uint32_t)hdr[1] << 8 | (uint32_t)hdr[2] << 16 | (uint32_t)hdr[3] << 24;
    setD32(GAME_RL_BLOCK, load_heap_alloc(m));
    p = load_heap_ptr(D32(GAME_RL_BLOCK), m);
    if (p == NULL || fread(p, 1, m, fp) != m) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    for (s = 0; s < 2; s++) {
        uint32_t tab = s ? GAME_RL_TABLE2 : GAME_RL_TABLE1, first = s ? GAME_RL_FIRST2 : GAME_RL_FIRST1;
        for (t = 0; t < 160; t++) {
            setD16(first + 2u * (uint32_t)t, 0xFFFF);
            for (e = 0; e < S32(tab); e++)
                if (D8(tab + 4u + 8u * (uint32_t)e) == (unsigned)t) {
                    setD16(first + 2u * (uint32_t)t, (uint16_t)e);
                    break;
                }
        }
    }
    return 0;
}

/* ======================================================================================================================
   The model: written from the listing (docs: the asm of each routine), on g_ds swapped to the model's copy.
   ==================================================================================================================== */

static int g_mut;   /* part D: 0 = the faithful model */

#define MA(r)        (0xE31B8u + 69u * (uint32_t)(r))
#define M_NAT(r)     S8(MA(r) + 1u)
#define M_MRK(r, n)  S8(MA(r) + 1u + (uint32_t)(n))
#define M_TOK(r, n)  S8(MA(r) + 0x0Eu + (uint32_t)(n))
#define M_MOV(r, n)  S8(MA(r) + 0x24u + (uint32_t)(n))
#define M_NR(n)      (0xF14CCu + 199u * (uint32_t)(n))
#define M_TS(t)      (0xE2C18u + 3u * (uint32_t)(t))
#define M_TR(t)      (0xE1638u + 35u * (uint32_t)(t))
#define M_FL(n, t)   (0xF3580u + 314u * (uint32_t)((n) - 1) + 2u * (uint32_t)(t))
#define M_SC(n, t)   S8(0xEED84u + 160u * (uint32_t)(n) + (uint32_t)(t))

static int m_lim(int t) { return (D8(M_TR(t) + 1u) >> 3) & 7; }
static int m_site(int t) { return (D8(M_TR(t) + 1u) >> 6) & 1; }
static int m_city(int t) { return D8(M_TS(t) + 2u) & 1; }
static int m_mk2(int t) { return (D8(M_TS(t) + 2u) >> 1) & 1; }
static int m_lvl(void) { return (D16(0xE1624u) >> 10) & 3; }
static int m_alone(int r, int n) { return M_NAT(r) == 1 && M_MRK(r, n) == n; }
static int m_tab(int row, int col) { return S16(0x112A94u + 20u * (uint32_t)row + 2u * (uint32_t)col); }
static int m_ord(int n) { return (int8_t)D8(0xF06C0u + (uint32_t)n); }
static int m_enemyfl(int n, int u) { return (D8(M_FL(n, u)) & 0x10) || (D8(M_FL(n, u) + 1u) & 0x02); }

/* 0x48028 */
static void m48028(int32_t *cnt, int32_t *out, int t, int n, int f)
{
    int i;
    *cnt = 0;
    for (i = 1; i <= g_adj[t][0]; i++) {
        int u = g_adj[t][i], r = u + 1, take = 0, lim;
        switch (f) {
        case 0: take = M_NAT(r) == 0 && !m_city(u); break;
        case 1: case 0x11:
            lim = m_lim(u) + (D8(M_NR(n) + 0x27u) != 0);
            take = m_alone(r, n) && (f == 1 ? M_TOK(r, n) < lim : (g_mut == 1 ? M_TOK(r, n) >= lim : M_TOK(r, n) > lim));
            break;
        case 2: take = M_MOV(r, n) > 0; break;
        case 7: take = m_alone(r, n) && !m_site(u); break;
        case 8: take = m_alone(r, n); break;
        case 9: take = m_alone(r, n) && !m_mk2(u); break;
        case 0x0A: take = M_MOV(r, n) > 0 && !m_mk2(u); break;
        case 0x0B: take = 1; break;
        case 0x0C: take = M_TOK(r, n) > 0; break;
        case 0x0F: take = m_alone(r, n) && !(D8(M_TR(u)) & 2) && !m_mk2(u); break;
        case 0x12: take = (D8(M_FL(n, u)) & 0x80) != 0; break;
        case 0x13: take = M_NAT(r) == 0 && !m_city(u) && D8(0xF4234u + 160u * (uint32_t)(n - 1) + (uint32_t)u); break;
        case 0x14: take = m_site(u) && m_enemyfl(n, u) && m_ord(n) > m_ord(f7C7C4(u, n)); break;
        case 0x15: take = M_NAT(r) > 1 && M_MRK(r, n) == n; break;
        case 0x17: take = m_site(u) && M_TOK(r, n) == 0 && M_NAT(r) == 1 && m_enemyfl(n, u)
                          && m_ord(n) > m_ord(f7C7C4(u, n)); break;
        case 0x18: take = M_NAT(r) == 1 && M_TOK(r, n) == 0; break;
        default: break;
        }
        if (take)
            out[(*cnt)++] = u;
    }
}

/* 0x4876C */
static void m4876C(int32_t *cnt, int32_t *out, int t, int n, int f)
{
    int i;
    if (g_mut == 15)
        *cnt = 0;
    for (i = 1; i <= g_adj[t][0]; i++) {
        int u = g_adj[t][i], r = u + 1, o, take;
        if (f != 0x16 || M_NAT(r) != 1 || M_MRK(r, n) == n || !(D8(0xF0044u + 160u * (uint32_t)n + (uint32_t)u) & 2))
            continue;
        o = f7C7C4(u, n);
        take = S32(0xF52E0u + 40u * (uint32_t)n + 4u * (uint32_t)o) >= 0;
        if (!take) {
            int h = (S8(M_NR(o) + 0x9Au) == 3) ? 2 : 1;
            int lhs = m_tab(25 + u % 3, n - 1) / 300;
            int rhs = S32(0xF52E0u + 40u * (uint32_t)o + 4u * (uint32_t)n) - 200 / (h * (5 - m_lvl()));
            take = lhs > rhs;
        }
        if (take)
            out[(*cnt)++] = u;
    }
}

/* 0x4892C */
static int m_in(const int32_t *l, int c, int v) { int i; for (i = 0; i < c; i++) if (l[i] == v) return 1; return 0; }
static void m4892C(int start, int32_t *cnt, int32_t *out, int count, const int32_t *list, int n, int f)
{
    int i, j;
    *cnt = 0;
    if (S32(0xF09E4u) == 0)
        return;
    for (i = 0; i < count; i++) {
        int t = list[i];
        int qual = (m_city(t) && D8(M_TS(t)) == n) || (M_NAT(t + 1) == 0 && !m_city(t)) || m_alone(t + 1, n);
        if (!qual)
            continue;
        for (j = 1; j <= g_adj[t][0]; j++) {
            int u = g_adj[t][j], take = 0;
            if ((u == start && g_mut != 4) || m_in(list, count, u) || m_in(out, *cnt, u))
                continue;
            if (f == 0) take = M_NAT(u + 1) == 0 && !m_city(u);
            else if (f == 9) take = m_alone(u + 1, n) && !m_mk2(u);
            else if (f == 0x0B) take = 1;
            else if (f == 0x14) take = m_site(u) && m_enemyfl(n, u) && m_ord(n) > m_ord(f7C7C4(u, n));
            else if (f == 0x17) take = m_site(u) && M_TOK(u + 1, n) == 0 && M_NAT(u + 1) == 1 && m_enemyfl(n, u)
                                       && m_ord(n) > m_ord(f7C7C4(u, n));
            if (take)
                out[(*cnt)++] = u;
        }
    }
}

/* 0x48D44: a stable partition by a predicate; keep = drop the others */
typedef int (*m_pred)(int n, int t, int u);
static void m_part(int32_t *cnt, int32_t *list, int n, int t, m_pred p, int keep_only)
{
    int32_t tmp[200];
    int c = *cnt, i, k = 0;
    memcpy(tmp, list, sizeof(int32_t) * (size_t)c);
    for (i = 0; i < c; i++)
        if (p(n, t, tmp[i]))
            list[k++] = tmp[i];
    if (!keep_only)
        for (i = 0; i < c; i++)
            if (!p(n, t, tmp[i]))
                list[k++] = tmp[i];
    *cnt = k;
}
static int p_site(int n, int t, int u) { (void)n; (void)t; return m_site(u); }
static int p_f4234(int n, int t, int u)
{
    (void)t;
    if (D8(0xF4234u + 160u * (uint32_t)(n - 1) + (uint32_t)u)) {
        if (g_mut != 3)
            setD8(0x108190u, 1);
        return 0;
    }
    return 1;
}
static int p_nocity(int n, int t, int u) { (void)n; (void)t; return !m_city(u); }
static int p_le(int n, int t, int u) { return M_SC(n, u) <= M_SC(n, t); }
static int p_bpm(int n, int t, int u) { (void)t; return (D8(0xF0044u + 160u * (uint32_t)n + (uint32_t)u) & 2) != 0; }
static int p_105(int n, int t, int u) { (void)t; return M_SC(n, u) > 105; }
static int p_city(int n, int t, int u) { (void)n; (void)t; return m_city(u); }
static int p_hi4(int n, int t, int u) { (void)t; return (D8(M_FL(n, u) + 1u) & 4) != 0; }

static void m48D44(int32_t *cnt, int32_t *list, int n, int t, int fl)
{
    int i, j, c;
    if (*cnt <= 0)
        return;
    if (fl & 0x80) m_part(cnt, list, n, t, p_site, 0);
    for (i = *cnt - 2; i >= 0; i--)
        for (j = 0; j <= i; j++) {
            int a = M_SC(n, list[j + 1]), b = M_SC(n, list[j]);
            if (g_mut == 2 ? a >= b : a > b) { int x = list[j]; list[j] = list[j + 1]; list[j + 1] = x; }
        }
    if (fl & 0x10) {
        c = *cnt;
        *cnt = 0;
        for (i = 0; i < c; i++)
            if (M_SC(n, list[i]) >= 0)
                (*cnt)++;
    }
    if (fl & 0x20) {   /* drops, then appends in order (the flag byte at each drop) */
        int32_t tmp[200];
        c = *cnt;
        memcpy(tmp, list, sizeof(int32_t) * (size_t)c);
        *cnt = 0;
        for (i = 0; i < c; i++)
            if (p_f4234(n, t, tmp[i]))
                list[(*cnt)++] = tmp[i];
    }
    if (fl & 0x200) m_part(cnt, list, n, t, p_nocity, 1);
    if (fl & 0x02) m_part(cnt, list, n, t, p_le, 0);
    if (fl & 0x04) m_part(cnt, list, n, t, p_bpm, 0);
    if (fl & 0x400) m_part(cnt, list, n, t, p_105, 1);
    if (fl & 0x100) m_part(cnt, list, n, t, p_city, 0);
    if (fl & 0x40) m_part(cnt, list, n, t, p_hi4, 0);
}

/* 0x47BA0 */
static int p_over(int n, int t, int u) { (void)t; return M_TOK(u + 1, n) > f78D08(u, n); }
static void m47BA0(int32_t *cnt, int32_t *list, int n, int mode, int fl)
{
    int t, i, j;
    *cnt = 0;
    for (t = 0; t < 157; t++) {
        int r = t + 1, take = 0;
        if (!D8(0xEE16Cu + (uint32_t)t))
            continue;
        if (mode == 3) {
            take = !m_mk2(t) && M_MOV(r, n) > 0 && (M_NAT(r) == 1 || (M_NAT(r) > 1 && (D8(0xF0044u + 160u * (uint32_t)n + (uint32_t)t) & 2)));
            if (g_mut == 14 && M_MRK(r, n) != n)
                take = 0;
        } else if (mode == 0x0E) {
            take = !m_mk2(t) && m_site(t) && (D8(M_TR(t) + 2u) & 1) && m_alone(r, n)
                   && M_TOK(r, n) > m_lim(t) + S8(M_NR(n) + 0x27u);
        }
        if (take)
            list[(*cnt)++] = t;
    }
    if (fl == 0 || *cnt <= 0)
        return;
    if (fl & 1)
        for (i = 1; i < *cnt; i++)
            for (j = *cnt - 1; j >= i; j--)
                if (M_SC(n, list[j - 1]) > M_SC(n, list[j])) { int x = list[j]; list[j] = list[j - 1]; list[j - 1] = x; }
    if (fl & 8)
        m_part(cnt, list, n, 0, p_over, 0);
}

/* 0x495C0 */
static int m495C0(int t, int n, int mode, int32_t *enemy)
{
    int best = 0, i, j;
    uint8_t keep[69];
    *enemy = 0;
    for (i = 1; i <= g_adj[t][0]; i++) {
        int r = g_adj[t][i] + 1, trial = 0;
        if (M_NAT(r) == 0)
            continue;
        if (M_NAT(r) > 1) {
            trial = 1;
            memcpy(keep, DP(MA(r)), 69);
            f7A778();
            setD8(0xBB3F8u, 0);
            f5B588(r);
            setD8(0xBB3F8u, 1);
            f7A7B0();
            if (g_mut == 5)
                memcpy(DP(MA(r)), keep, 69);
        }
        for (j = 1; j <= 9; j++) {
            if (j == n || (mode == 1 && m_ord(j) <= m_ord(n)))
                continue;
            if (M_TOK(r, j) > best) { best = M_TOK(r, j); *enemy = j; }
        }
        if (trial)
            memcpy(DP(MA(r)), keep, 69);
    }
    return best;
}

/* 0x49790 */
static int m49790(int r, int t, int n, int e)
{
    int lim, thr;
    if (r == 0)
        return 0;
    lim = m_lim(t) - 1;
    if (g_mut == 6 ? lim < 1 : lim == 0)
        lim = 1;
    thr = lim + 1 + S8(M_NR(n) + 0x26u) - S8(M_NR(e) + 0x26u);
    if (r < thr)
        return 1;
    if (m_tab(12 + t % 8, n - 1) / 300 < 95 - 10 * m_lvl())
        return M_TOK(t + 1, n) <= m_lim(t) ? M_TOK(t + 1, n) : m_lim(t);
    return 1;
}

/* 0x49934 */
static int m49934(int t, int n, int mode)
{
    int i, k;
    for (i = 1; i <= g_adj[t][0]; i++)
        if (M_TOK(g_adj[t][i] + 1, n) != 0)
            return 1;
    if (mode == 1 && g_mut != 17)
        return 0;
    for (k = 0; k < D8(M_NR(n) + 0x40u); k++)
        if (f5843C(S16(M_NR(n) + 0x45u + 2u * (uint32_t)k) - 1, t, n, D8(M_NR(n) + 0x41u + (uint32_t)k)) == 1)
            return 1;
    return 0;
}

/* 0x4D110 (before 0x4CC5C, which calls it) */
static int m4D110(int t, int32_t *cnt, const int32_t *list, int m, int k)
{
    int32_t ring[200];
    int32_t c2;
    int i, sum = 0, pass;
    m4892C(t, &c2, ring, *cnt, list, m, 0x0B);
    for (pass = 0; pass < 2; pass++) {
        const int32_t *l = pass ? ring : list;
        int c = pass ? c2 : *cnt;
        for (i = 0; i < c; i++) {
            int r = l[i] + 1, s;
            if (M_NAT(r) == 0)
                continue;
            s = M_TOK(r, m) - k;
            if (s > 0 && (!m_site(r - 1) || s <= 4 || g_mut == 16))
                sum += s;
        }
    }
    return sum;
}

/* 0x4CC5C */
static int m4CC5C(int t, int n, int mode)
{
    int r = t + 1, e = 0, m = 0, k, base, s;
    int32_t c, l[200];
    for (k = 1; k <= 12; k++)
        if (k != n && M_TOK(r, k) > e) { e = M_TOK(r, k); m = k; }
    if (e == 0)
        return M_NAT(r) == 0 ? 1 : -1;
    base = m_lim(t) - 1;
    if (base == 0)
        base = 1;
    if (!(m_ord(m) < m_ord(n) || (g_mut == 7 ? m >= 9 : m > 9))) {
        if (mode == 3 || f7CAB4(n) == 0)
            return -1;
        m48028(&c, l, t, m, 0x0C);
        if (c != 0) {
            s = m4D110(t, &c, l, m, 0);
            if (s > 2)
                s = 2;
            return M_TOK(r, m) + s + base - M_TOK(r, n) + S8(M_NR(m) + 0x26u) - S8(M_NR(n) + 0x26u);
        }
    }
    return M_TOK(r, m) + base - M_TOK(r, n) + S8(M_NR(m) + 0x26u) - S8(M_NR(n) + 0x26u);
}

/* 0x4CEE4 */
static int m4CEE4(int t, int n, int32_t *out)
{
    int m = D8(M_TS(t)), e = M_TOK(t + 1, m), dE = S8(M_NR(m) + 0x2Au) - S8(M_NR(n) + 0x2Au), sea, s, extra;
    int32_t c, l[200];
    if (m_ord(m) < m_ord(n) || m > 9) {
        *out = e + 7 + dE;
        return *out;
    }
    m48028(&c, l, t, m, 0x0C);
    sea = aiq_test_f4D29C(t, m);
    if (c + sea == 0) {
        *out = e + 7 + dE;
        return *out;
    }
    *out = 7 + dE + (g_mut == 19 ? e : 0);
    s = m4D110(t, &c, l, m, 0);
    extra = s + sea == 0 ? 0 : s + sea - 1 + (D8(M_NR(m) + 0x26u) != 0 && D8(M_NR(n) + 0x26u) == 0);
    return e + 7 + extra + dE;
}

/* 0x7BD5C */
static void m7BD5C(int n)
{
    int t, i, k;
    int32_t nb[200], ring[200], c2;
    int sea = S8(M_NR(n) + 0x28u);
    memset(DP(0xF3580u + 314u * (uint32_t)(n - 1)), 0, 314);
    for (t = 0; t < 157; t++) {
        int r = t + 1, nn = 0;
        uint8_t lo = 0, hi = 0;
        if (M_NAT(r) == 0) {
            int occ = 0, own = 0, foreign = 0, own2 = 0, foreign2 = 0, rings = 0, best = 0, bm = 0;
            for (i = 1; i <= g_adj[t][0]; i++) {
                int u = g_adj[t][i];
                nb[nn++] = u;
                if (M_NAT(u + 1) == 0)
                    continue;
                occ++;
                if (m_alone(u + 1, n)) { own = 1; continue; }
                foreign = 1;
                if (g_mut != 8) { best = 0; bm = 11; }
                for (k = 1; k <= 9; k++)
                    if (k != n && M_TOK(u + 1, k) > best) { best = M_TOK(u + 1, k); bm = k; }
            }
            if (sea) {
                m4892C(t, &c2, ring, nn, nb, n, 0x0B);
                for (i = 0; i < c2; i++) {
                    rings++;
                    if (m_alone(ring[i] + 1, n)) own2 = 1; else foreign2 = 1;
                }
            }
            if (occ == 0 || (foreign && !own)) lo |= 0x01;
            else if (!foreign && own) lo |= 0x04;
            else if (foreign && own) {
                if (bm > 9) lo |= 0x04;
                else {
                    int lim = m_lim(t) - 1;
                    if (lim == 0) lim = 1;
                    if (lim + 1 + S8(M_NR(n) + 0x26u) - S8(M_NR(bm) + 0x26u) <= best) hi |= 0x04;
                }
            }
            if (rings == 0 || foreign2) lo |= 0x02;
            else if (own2) lo |= 0x08;
        } else if (M_NAT(r) != 1) {
            if (M_MRK(r, n) == n) hi |= 0x02; else lo |= 0x10;
        } else if (f7C760(t) != n) {
            lo |= 0x10;
        } else {
            int a58 = 0, a54 = 0, a50 = 0, a4c = 0;
            if (g_adj[t][0] == 0)
                a58 = 1;
            for (i = 1; i <= g_adj[t][0]; i++) {
                int u = g_adj[t][i], u1 = u + 1;
                nb[nn++] = u;
                if (M_NAT(u1) == 0 && !m_city(u))
                    continue;
                if (m_city(u) || (M_NAT(u1) == 2 && M_MRK(u1, n) == n && M_TOK(u1, 11) > 0) || m_alone(u1, n))
                    a58 = 1;
                else
                    a54 = 1;
            }
            if (sea) {
                m4892C(t, &c2, ring, nn, nb, n, 0x0B);
                for (i = 0; i < c2; i++) {
                    int u = ring[i], u1 = u + 1;
                    if ((M_NAT(u1) == 0 && !m_city(u)) || m_city(u)
                        || (M_NAT(u1) == 2 && M_MRK(u1, n) == n && M_TOK(u1, 11) > 0) || m_alone(u1, n))
                        a50 = 1;
                    else
                        a4c = 1;
                }
            }
            (void)a58;
            (void)a50;   /* hi 0x01 whenever no second-ring entry is foreign, a50 or not (0x7C48F..0x7C4BC) */
            lo |= a54 ? 0x20 : 0x80;
            if (a4c) lo |= 0x40; else hi |= 0x01;
        }
        setD8(M_FL(n, t), (uint8_t)(D8(M_FL(n, t)) | lo));
        setD8(M_FL(n, t) + 1u, (uint8_t)(D8(M_FL(n, t) + 1u) | hi));
        if (m_enemyfl(n, t)) {
            int o = f7C7C4(t, n), s;
            nn = 0;
            for (i = 1; i <= g_adj[t][0]; i++)
                if (m_enemyfl(n, g_adj[t][i]) && f7C7C4(g_adj[t][i], n) == o)
                    nb[nn++] = g_adj[t][i];
            if (nn == 0) {
                setD8(M_FL(n, t) + 1u, (uint8_t)(D8(M_FL(n, t) + 1u) | 0x08));
            } else if (nn == 1) {
                s = nb[0];
                nn = 0;
                for (i = 1; i <= g_adj[s][0]; i++)
                    if (m_enemyfl(n, g_adj[s][i]) && f7C7C4(g_adj[s][i], n) == o)
                        nb[nn++] = g_adj[s][i];
                if (nn == 1 && nb[0] == s)
                    setD8(M_FL(n, t) + 1u, (uint8_t)(D8(M_FL(n, t) + 1u) | 0x08));
            }
        }
    }
}

/* 0x7C838 */
static int m7C838(int t, int n, int k, int32_t *left)
{
    int r = t + 1;
    uint8_t keep[69];
    memcpy(keep, DP(MA(r)), 69);
    f7A778();
    setD8(MA(r) + 0x0Eu + (uint32_t)n, (uint8_t)(D8(MA(r) + 0x0Eu + (uint32_t)n) + k));
    if (M_MRK(r, n) != n) {
        setD8(MA(r) + 1u, (uint8_t)(D8(MA(r) + 1u) + 1));
        setD8(MA(r) + 1u + (uint32_t)n, (uint8_t)n);
    }
    if (g_mut != 9)
        memset(DP(0xF547Cu), 0, 10);
    setD8(0xBB3F8u, 0);
    f5B588(r);
    setD8(0xBB3F8u, 1);
    *left = M_NAT(r);
    f7A7B0();
    memcpy(DP(MA(r)), keep, 69);
    return S8(0xF547Cu + (uint32_t)n);
}

/* 0x7B044 */
static int m_div4(int v) { return g_mut == 10 ? (v >> 2) : v / 4; }
static int m7B044(int n)
{
    uint32_t nr = M_NR(n);
    int cnt = S8(nr + 0x60u), i, over = 0, f1 = 0, mk = 0, mv = 0, val = 0;
    int64_t att = 0, res;
    for (i = 0; i < cnt; i++) {
        int a = D8(nr + 0x61u + (uint32_t)i), t = a - 1, lim = m_lim(t), v = S8(0xEED84u + 160u * (uint32_t)n + (uint32_t)t);
        val += m_div4(v);
        if (m_alone(a, n) && D8(nr + 0x27u))
            lim++;
        if (D8(M_TR(t) + 2u) & 1)
            f1++;
        if (m_mk2(t)) {
            mk++;
            mv += S8(0xEFA04u + 160u * (uint32_t)n + (uint32_t)t);
        } else if (M_NAT(a) == 1 && M_TOK(a, n) > lim) {
            over += M_TOK(a, n) - lim;
        }
    }
    for (i = 0; i < S32(0x1079B4u); i++) {
        int m, idx, w;
        int32_t c;
        if (D8(0x107694u + 4u * (uint32_t)i) != 0xC9)
            continue;
        m = D8(0x107695u + 4u * (uint32_t)i);
        idx = S8(0xBCCE0u + 85u * (uint32_t)(m - 1) + 5u * (uint32_t)S8(nr + 0x5Cu)) + (S8(M_NR(m) + 0x9Au) == 3);
        w = S32(0xC2864u + 12u * (uint32_t)idx + 4u * (uint32_t)m_lvl()) / 100;
        c = (int32_t)((uint32_t)(S32(0xF52E0u + 40u * (uint32_t)n + 4u * (uint32_t)m) + (S8(M_NR(m) + 0x9Au) == 3 ? 200 : 50))
                      * (uint32_t)w);
        if (c > 0)
            att += c;
    }
    if (D8(0xF488Cu + (uint32_t)n) == 0 && S32(0x108164u + 4u * (uint32_t)n) == 0)
        res = 500 * S8(nr + 0x51u) + 500 * mk + 100 * mv + 300 * cnt + val - 200 * over - 200 * f1 + att;
    else
        res = 500 * S8(nr + 0x51u) + 500 * mk + 100 * mv + (300 * cnt + val) / 4 - 300 * over - 200 * f1 + att;
    res = (int32_t)(uint32_t)res;
    return res > 0x7FFF ? 0x7FFF : (int)res;
}

/* 0x468EC */
static int m468EC(void)
{
    int m;
    if (m_lvl() != 0)
        return 0;
    for (m = 1; m <= 9; m++)
        if (S8(M_NR(m) + 0x9Au) == 3 && S8(M_NR(m) + 0x99u) < 4 && S8(M_NR(m) + 0x51u) > 0
            && S32(0xF5470u) >= (g_mut == 20 ? 4 : 3) && D8(M_NR(m) + 0x40u) < 4)
            return 1;
    return 0;
}

/* 0x46998 */
static void m46998(int n)
{
    int t;
    for (t = 0; t <= (g_mut == 11 ? 156 : 157); t++)
        if ((D8(M_FL(n, t) + 1u) & 8) && M_SC(n, t) > 0)
            setD8(MA(t + 1) + 0x3Bu + (uint32_t)n, 1);   /* the "!= 1" test only skips an equal write */
}

/* 0x39C08 */
static int m39C08(int n)
{
    int t, c = 0;
    uint8_t keep[69];
    for (t = 0; t < 157; t++) {
        int r = t + 1;
        if (!D8(0xEE16Cu + (uint32_t)t))
            continue;
        if (m_city(t) && D8(M_TS(t)) == n) { c++; continue; }
        if (!m_site(t))
            continue;
        if ((D8(M_FL(n, t)) & 4) || m_alone(r, n)) { c++; continue; }
        if (M_NAT(r) <= 1 || M_TOK(r, n) == 0)
            continue;
        if (f7C7C4(t, n) > 9) { c++; continue; }
        memcpy(keep, DP(MA(r)), 69);
        f7A778();
        setD8(0xBB3F8u, 0);
        f5B588(r);
        setD8(0xBB3F8u, 1);
        f7A7B0();
        if (g_mut == 18 && m_alone(r, n))
            c++;
        memcpy(DP(MA(r)), keep, 69);
        if (m_alone(r, n))
            c++;
    }
    return c;
}

/* 0x160EC */
static void m160EC(int n)
{
    int t, k = 0;
    setD8(0xF62A8u + (uint32_t)n, 0);
    for (t = 0; t < 160; t++) {
        int c, b;
        if (!(D8(0xF0044u + 160u * (uint32_t)n + (uint32_t)t) & 4))
            continue;
        setD8(0xF62A8u + (uint32_t)n, 1);
        setD8(0xF62B8u + 5u * (uint32_t)(n - 1) + (uint32_t)k, (uint8_t)t);
        k++;
        c = D8(0xC803Cu + (uint32_t)t);
        b = S8(0xEF3C4u + 160u * (uint32_t)n + (uint32_t)t);
        setD8(0xEED84u + 160u * (uint32_t)n + (uint32_t)t,
              (uint8_t)((g_mut == 12 ? M_TOK(c + 1, n) >= 0 : M_TOK(c + 1, n) > 0) ? b + 5 : b));
    }
    setD32(0xF62B4u, (uint32_t)k);
}

/* 0x7701C */
static void m7701C(int count, uint8_t *buf, int k)
{
    uint8_t loc[192];
    int left = count, out = 0, row = 0, idx, i;
    memset(loc, 0, sizeof loc);
    memcpy(loc, buf, (size_t)count);
    while (left > 0) {
        int d = 30000 / (left < 30000 ? left : 30000);
        int a = S16(0x112A92u + 20u * (uint32_t)(row++ % 30) + 2u * (uint32_t)k) / d;
        if (a >= left) {
            idx = left - 1;
        } else if (g_mut == 13) {
            idx = a;
        } else {
            idx = S16(0x112A92u + 20u * (uint32_t)(row++ % 30) + 2u * (uint32_t)k) / d;
            if (!g_original && idx >= count)
                idx = left - 1;
        }
        buf[out++] = loc[idx];
        for (i = idx; i < left - 1; i++)
            loc[i] = loc[i + 1];
        left--;
    }
}

/* ======================================================================================================================
   Part A: random states, port against model
   ==================================================================================================================== */

static uint8_t *g_base[2];
static int      g_nbase;
static uint8_t *M;

static void rand_state(void)
{
    int r, t, m, k, n;
    memcpy(g_ds, g_base[rn(g_nbase)], DS_SIZE);
    /* the area records 1..160 */
    for (r = 1; r <= 160; r++) {
        uint32_t a = MA(r);
        int cnt = 0;
        if (rn(3) == 0)
            continue;   /* keep the save's record */
        for (m = 1; m <= 13; m++) {
            setD8(a + 1u + (uint32_t)m, 0);
            setD8(a + 0x0Eu + (uint32_t)m, 0);
        }
        for (m = 1; m <= 11; m++) {
            setD8(a + 0x24u + (uint32_t)m, 0);
            setD8(a + 0x3Bu + (uint32_t)m, (uint8_t)(rn(4) == 0));
        }
        k = rn(5) == 0 ? 0 : 1 + rn(rn(2) ? 1 : 3);
        while (k-- > 0) {
            m = 1 + rn(rn(8) == 0 ? 11 : 9);
            if ((int8_t)D8(a + 1u + (uint32_t)m) == m)
                continue;
            setD8(a + 1u + (uint32_t)m, (uint8_t)m);
            setD8(a + 0x0Eu + (uint32_t)m, (uint8_t)(1 + rn(9)));
            if (m <= 11)
                setD8(a + 0x24u + (uint32_t)m, (uint8_t)rn(1 + D8(a + 0x0Eu + (uint32_t)m)));
            cnt++;
        }
        if (rn(20) == 0)
            setD8(a + 0x0Eu + 12u, (uint8_t)rn(4));   /* a faction's tokens without its marker */
        setD8(a + 1u, (uint8_t)(rn(25) == 0 ? rn(4) : cnt));
    }
    for (t = 0; t < 160; t++) {
        if (rn(2)) {
            setD8(M_TS(t) + 2u, (uint8_t)rn(4));
            setD8(M_TS(t), (uint8_t)(rn(4) ? rn(10) : rn(14)));
        }
        if (rn(6) == 0)
            setD8(M_TR(t) + 1u, (uint8_t)rnd());
        if (rn(6) == 0)
            setD8(M_TR(t), (uint8_t)(D8(M_TR(t)) ^ 2));
        if (rn(6) == 0)
            setD8(M_TR(t) + 2u, (uint8_t)(D8(M_TR(t) + 2u) ^ 1));
        if (rn(30) == 0)
            setD8(0xEE16Cu + (uint32_t)t, (uint8_t)(D8(0xEE16Cu + (uint32_t)t) ^ 1));
    }
    for (k = 0; k < 9 * 314; k++)
        if (rn(3) == 0)
            setD8(0xF3580u + (uint32_t)k, (uint8_t)rnd());
    for (k = 0; k < 1600; k++) {
        if (rn(3) == 0) setD8(0xF0044u + (uint32_t)k, (uint8_t)(rn(5) == 0 ? rnd() : rn(8)));
        if (rn(3) == 0) setD8(0xEED84u + (uint32_t)k, (uint8_t)(rn(4) ? rn(256) : 100 + rn(28)));
        if (rn(3) == 0) setD8(0xEF3C4u + (uint32_t)k, (uint8_t)rnd());
        if (rn(3) == 0) setD8(0xEFA04u + (uint32_t)k, (uint8_t)rnd());
    }
    for (k = 0; k < 9 * 160; k++)
        if (rn(4) == 0)
            setD8(0xF4234u + (uint32_t)k, (uint8_t)(rn(3) == 0));
    for (k = 0; k < 14; k++)
        if (rn(2))
            setD8(0xF06C0u + (uint32_t)k, (uint8_t)(rn(8) == 0 ? rnd() : rn(10)));
    for (k = 0; k < 110; k++)
        if (rn(3) == 0)
            setD32(0xF52E0u + 4u * (uint32_t)k, (uint32_t)(rn(600) - 300));
    for (k = 0; k < 300; k++)
        if (rn(2))
            setD16(0x112A94u + 2u * (uint32_t)k, (uint16_t)(rn(10) == 0 ? 29990 + rn(10) : rn(30000)));
    if (rn(2))
        setD16(0xE1624u, (uint16_t)((D16(0xE1624u) & ~0x0C00u) | (uint32_t)rn(rn(10) == 0 ? 4 : 3) << 10));
    for (n = 0; n <= 13; n++) {
        uint32_t nr = M_NR(n);
        if (rn(3) == 0)
            continue;
        setD8(nr + 0x26u, (uint8_t)rn(2));
        setD8(nr + 0x27u, (uint8_t)rn(2));
        setD8(nr + 0x28u, (uint8_t)rn(2));
        setD8(nr + 0x2Au, (uint8_t)rn(2));
        setD8(nr + 0x2Bu, (uint8_t)rn(2));
        if (n >= 1 && n <= 9)
            setD8(nr + 0x9Au, (uint8_t)(rn(3) == 0 ? 0 : rn(2) ? 2 : 3));
        setD8(nr + 0x99u, (uint8_t)rn(12));
        setD8(nr + 0x51u, (uint8_t)rn(5));
        setD8(nr + 0x40u, (uint8_t)rn(5));
        for (k = 0; k < 4; k++) {
            setD8(nr + 0x41u + (uint32_t)k, (uint8_t)rn(6));
            setD16(nr + 0x45u + 2u * (uint32_t)k, (uint16_t)(1 + rn(157)));
        }
        setD8(nr + 0x5Cu, (uint8_t)rn(16));
        k = rn(20);
        setD8(nr + 0x60u, (uint8_t)k);
        while (k-- > 0)
            setD8(nr + 0x61u + (uint32_t)k, (uint8_t)(1 + rn(160)));
    }
    k = rn(4) == 0 ? 0 : rn(30);
    setD32(0x1079B4u, (uint32_t)k);
    while (k-- > 0) {
        setD8(0x107694u + 4u * (uint32_t)k, (uint8_t)(rn(2) ? 0xC9 : rnd()));
        setD8(0x107695u + 4u * (uint32_t)k, (uint8_t)(rn(10) == 0 ? rn(14) : 1 + rn(9)));
    }
    for (n = 0; n < 10; n++) {
        setD8(0xF488Cu + (uint32_t)n, (uint8_t)(rn(5) == 0));
        setD32(0x108164u + 4u * (uint32_t)n, (uint32_t)(rn(5) == 0));
    }
    setD32(0xF09E4u, (uint32_t)(rn(8) != 0));
    setD32(0xF5470u, (uint32_t)rn(6));
    setD8(0xBB3F8u, 1);
}

static void rand_list(int32_t *l, int *c, int max)
{
    int i, k = rn(max + 1);
    *c = 0;
    for (i = 0; i < k; i++) {
        int v = rn(157);
        if (!m_in(l, *c, v))
            l[(*c)++] = v;
    }
}

/* one call, port and model; fn = 0..18 */
static const char *const k_names[19] = {
    "f48028", "f4876C", "f4892C", "f48D44", "f47BA0", "f495C0", "f49790", "f49934", "f4CC5C", "f4CEE4",
    "f4D110", "f7BD5C", "f7C838", "f7B044", "f468EC", "f46998", "f39C08", "f160EC", "f7701C"
};


/* one call of routine fn on the port and on the model, every argument drawn before either runs */
static int one_call(int fn)
{
    static const int k_f2[6] = {0, 9, 0x0B, 0x14, 0x17, 3};
    int32_t pl[200], ml[200], in[200], ring_p[200], ring_m[200];
    int32_t pc, mc, pout = -77, mout = -77;
    uint8_t pb[64], mb[64];
    int pr = 0, mr = 0, nin, i;
    static const int k_live[17] = {0, 1, 2, 7, 8, 9, 0x0A, 0x0B, 0x0C, 0x0F, 0x11, 0x12, 0x13, 0x14, 0x15, 0x17, 0x18};
    int t = rn(10) == 0 ? 157 + rn(3) : rn(157), n = 1 + rn(9), a = rn(3) ? k_live[rn(17)] : rn(26), b = rn(0x800);
    int c = rn(5);
    int filt1 = rn(2) ? 0x16 : a, f2 = k_f2[rn(6)], mode4 = c == 0 ? 0x0E : c == 1 ? 3 : rn(16);
    int e6 = rn(12), m10 = 1 + rn(11), cnt18 = 2 + rn(40), k18 = rn(11), st2, pre;
    uint8_t *save = g_ds;

    if (fn != 3 && fn != 6 && fn != 10 && fn != 2 && t > 156)
        t = rn(157);
    rand_list(in, &nin, rn(3) == 0 ? 40 : 12);
    st2 = rn(4) == 0 && nin > 0 ? in[0] : rn(157);
    pre = fn == 1 ? rn(3) : nin;
    memcpy(pl, in, sizeof pl);
    memcpy(ml, in, sizeof ml);
    pc = mc = pre;
    for (i = 0; i < 64; i++)
        pb[i] = mb[i] = (uint8_t)rnd();
    memcpy(M, g_ds, DS_SIZE);
    g_log[0][0] = g_log[1][0] = 0;

    for (g_side = 0; g_side < 2; g_side++) {
        int s = g_side;
        int32_t *l = s ? ml : pl, *cp = s ? &mc : &pc, *op = s ? &mout : &pout, *ring = s ? ring_m : ring_p;
        uint8_t *bb = s ? mb : pb;
        int r = 0;
        if (s)
            g_ds = M;
        switch (fn) {
        case 0: if (s) m48028(cp, l, t, n, a); else f48028(cp, l, t, n, a); break;
        case 1: if (s) m4876C(cp, l, t, n, filt1); else f4876C(cp, l, t, n, filt1); break;
        case 2:
            if (s) m4892C(st2, cp, ring, nin, in, n, f2); else f4892C(st2, cp, ring, nin, in, n, f2);
            memcpy(l, ring, sizeof(int32_t) * (size_t)(*cp >= 0 && *cp <= 200 ? *cp : 0));
            break;
        case 3: if (s) m48D44(cp, l, n, t, b); else f48D44(cp, l, n, t, b); break;
        case 4: if (s) m47BA0(cp, l, n, mode4, b & 0x0F); else f47BA0(cp, l, n, mode4, b & 0x0F); break;
        case 5: r = s ? m495C0(t, n, c % 3, op) : f495C0(t, n, c % 3, op); break;
        case 6: r = s ? m49790(a - 2, t, n, e6) : f49790(a - 2, t, n, e6); break;
        case 7: r = s ? m49934(t, n, c % 3) : f49934(t, n, c % 3); break;
        case 8: r = s ? m4CC5C(t, n, c) : f4CC5C(t, n, c); break;
        case 9: r = s ? m4CEE4(t, n, op) : f4CEE4(t, n, op); break;
        case 10: r = s ? m4D110(t, cp, in, m10, c % 4) : f4D110(t, cp, in, m10, c % 4); break;
        case 11: if (s) m7BD5C(n); else f7BD5C(n); break;
        case 12: r = s ? m7C838(t, n, c, op) : f7C838(t, n, c, op); break;
        case 13: r = s ? m7B044(n) : f7B044(n); break;
        case 14: r = s ? m468EC() : f468EC(); break;
        case 15: if (s) m46998(n); else f46998(n); break;
        case 16: r = s ? m39C08(n) : f39C08(n); break;
        case 17: if (s) m160EC(n); else f160EC(n); break;
        case 18: if (s) m7701C(cnt18, bb, k18); else f7701C(cnt18, bb, k18); break;
        default: break;
        }
        if (s) mr = r; else pr = r;
        g_ds = save;
    }
    return pr == mr && pc == mc && pout == mout
           && memcmp(pl, ml, sizeof(int32_t) * (size_t)(pc > 0 && pc <= 200 ? pc : 0)) == 0
           && memcmp(pb, mb, sizeof pb) == 0 && strcmp(g_log[0], g_log[1]) == 0 && memcmp(g_ds, M, DS_SIZE) == 0;
}

static long g_calls[19], g_bad[19];

static void part_a(long runs, const char *tag)
{
    long i, bad[19] = {0}, cnt[19] = {0};
    int fn;
    for (i = 0; i < runs; i++) {
        if (i % 4 == 0)
            rand_state();
        fn = rn(19);
        cnt[fn]++;
        if (!one_call(fn))
            bad[fn]++;
    }
    for (fn = 0; fn < 19; fn++) {
        g_calls[fn] += cnt[fn];
        g_bad[fn] += bad[fn];
        if (tag != NULL)
            check(bad[fn] == 0, "A%s %s: %ld of %ld calls differ from the model", tag, k_names[fn], bad[fn], cnt[fn]);
    }
}

/* ======================================================================================================================
   Part B: hand vectors
   ==================================================================================================================== */

static void clear_area(int r)
{
    int m;
    uint32_t a = MA(r);
    setD8(a + 1u, 0);
    for (m = 1; m <= 13; m++) {
        setD8(a + 1u + (uint32_t)m, 0);
        setD8(a + 0x0Eu + (uint32_t)m, 0);
    }
}

static void put(int t, int m, int k)   /* k tokens of nation m in territory t, marker and count kept consistent */
{
    uint32_t a = MA(t + 1);
    if ((int8_t)D8(a + 1u + (uint32_t)m) != m) {
        setD8(a + 1u + (uint32_t)m, (uint8_t)m);
        setD8(a + 1u, (uint8_t)(D8(a + 1u) + 1));
    }
    setD8(a + 0x0Eu + (uint32_t)m, (uint8_t)k);
}

static void part_b(void)
{
    uint8_t buf[8], *before = malloc(DS_SIZE);
    int i, r, t, u, n, keep;
    int32_t cnt, list[64], e;

    if (before == NULL)
        return;
    /* B1. f7701C: count 7, column k - 1 = 2 (k = 3): rows 0 / 1 pick 7 = count (BUG 1), rows 2 / 3 pick 0 */
    memcpy(g_ds, g_base[0], DS_SIZE);
    for (i = 0; i < 30; i++)
        setD16(0x112A92u + 20u * (uint32_t)i + 6u, 0);
    setD16(0x112A92u + 6u, 100);
    setD16(0x112A92u + 26u, 29999);
    setD16(0x112A92u + 46u, 29999);
    for (keep = 0; keep < 2; keep++) {
        g_original = keep;
        for (i = 0; i < 7; i++)
            buf[i] = (uint8_t)(10 + i);
        f7701C(7, buf, 3);
        check(buf[0] == (keep ? 0 : 16), "B1 f7701C first pick of a second-row 7 of 7 (%s): %d",
              keep ? "original: the zeroed byte" : "fixed: the last entry", buf[0]);
        check(buf[1] == 10, "B1 f7701C second pick, rows 2 / 3: %d", buf[1]);
    }
    g_original = 0;
    /* a pick in range on the first row takes the next row: row 0 = 0 (pick 0 < 7), row 1 = 4285 * 3 (pick 3) */
    setD16(0x112A92u + 6u, 0);
    setD16(0x112A92u + 26u, 4285 * 3);
    for (i = 0; i < 7; i++)
        buf[i] = (uint8_t)(10 + i);
    f7701C(7, buf, 3);
    check(buf[0] == 13, "B1 f7701C: the second row decides a pick in range (%d)", buf[0]);

    /* B2. f46998 at t = 157: the next row's entry 0 (n = 1) and the byte after the table (n = 9) */
    memcpy(g_ds, g_base[0], DS_SIZE);
    memset(DP(0xF3580u), 0, 9 * 314);
    for (n = 1; n <= 9; n += 8) {
        uint32_t flag = 0xF3580u + 314u * (uint32_t)(n - 1) + 315u;
        uint8_t old = D8(flag);
        setD8(flag, (uint8_t)(old | 8));
        setD8(0xEED84u + 160u * (uint32_t)n + 157u, 3);
        setD8(MA(158) + 0x3Bu + (uint32_t)n, 0);
        memcpy(before, g_ds, DS_SIZE);
        f46998(n);
        check(D8(MA(158) + 0x3Bu + (uint32_t)n) == 1, "B2 f46998(%d) marks area record 158 from flag byte 0x%X", n,
              (unsigned)flag);
        setD8(MA(158) + 0x3Bu + (uint32_t)n, 0);
        check(memcmp(before, g_ds, DS_SIZE) == 0, "B2 f46998(%d) writes nothing else", n);
        setD8(flag, old);
    }

    /* B3. f39C08: a contested site of n = 2 (2 vs 3 tokens of nation 4): the trial runs, the site is not counted */
    memcpy(g_ds, g_base[0], DS_SIZE);
    for (t = 0; t < 157; t++) {
        clear_area(t + 1);
        setD8(M_TS(t) + 2u, 0);
        setD8(M_TR(t) + 1u, (uint8_t)(D8(M_TR(t) + 1u) & ~0x40u));
        setD8(0xEE16Cu + (uint32_t)t, 1);
    }
    memset(DP(0xF3580u), 0, 9 * 314);
    t = 40;
    setD8(M_TR(t) + 1u, (uint8_t)(D8(M_TR(t) + 1u) | 0x40u));
    put(t, 2, 2);
    put(t, 4, 3);
    setD8(M_TR(41) + 1u, (uint8_t)(D8(M_TR(41) + 1u) | 0x40u));
    put(41, 2, 1);                               /* n alone on a site: counted */
    memset(DP(0xF547Cu), 0, 10);
    memcpy(before, g_ds, DS_SIZE);
    g_side = 0;
    g_log[0][0] = 0;
    r = f39C08(2);
    check(r == 1, "B3 f39C08: the lone site counts, the contested one does not (%d)", r);
    check(strcmp(g_log[0], "B(41,0) ") == 0, "B3 f39C08: one trial f5B588(41) with [0xBB3F8] = 0 (%s)", g_log[0]);
    check(memcmp(DP(MA(41)), before + (MA(41) - DS_BASE), 69) == 0, "B3 f39C08: the area record restored");
    check(memcmp(DP(0xF1593u), before + (0xF1593u - DS_BASE), 0x88D) == 0, "B3 f39C08: the nations restored");
    check(D8(0xBB3F8u) == 1, "B3 f39C08: [0xBB3F8] = 1 after the trial");
    check(memcmp(DP(0xF547Cu), before + (0xF547Cu - DS_BASE), 10) != 0, "B3 f39C08: the trial's losses stay in 0xF547C");

    /* B4. f7C838: 3 tokens of n = 2 added to t = 40 (nation 4 there, 2 absent) */
    clear_area(41);
    put(40, 4, 3);
    memset(DP(0xF547Cu), 0xEE, 10);
    memcpy(before, g_ds, DS_SIZE);
    g_log[0][0] = 0;
    r = f7C838(40, 2, 3, &e);
    check(strcmp(g_log[0], "B(41,0) ") == 0, "B4 f7C838: one trial (%s)", g_log[0]);
    /* the fake: n = 2 gets 3 -> loss (41*7 + 6 + 3) % 3 = 2; nation 4 has 3 -> (287 + 12 + 3) % 3 = 2 */
    check(r == 2 && e == 2, "B4 f7C838: losses of n 2 and two nations left (%d, %d)", r, (int)e);
    check(D8(0xF547Cu) == 0 && D8(0xF547Cu + 4u) == 2 && D8(0xF547Cu + 9u) == 0,
          "B4 f7C838: 0xF547C cleared before the trial and kept after");
    memcpy(DP(0xF547Cu), before + (0xF547Cu - DS_BASE), 10);
    check(memcmp(before, g_ds, DS_SIZE) == 0, "B4 f7C838: everything else restored");

    /* B5. f495C0 around t = 40: neighbour u contested (nations 3: 5, 5: 4), the other neighbours empty or 6: 2 */
    memcpy(g_ds, before, DS_SIZE);
    clear_area(41);
    u = g_adj[40][1];
    clear_area(u + 1);
    put(u, 3, 5);
    put(u, 5, 4);
    for (i = 2; i <= g_adj[40][0]; i++)
        clear_area(g_adj[40][i] + 1);
    if (g_adj[40][0] > 1)
        put(g_adj[40][2], 6, 2);
    memcpy(before, g_ds, DS_SIZE);
    g_log[0][0] = 0;
    r = f495C0(40, 2, 0, &e);
    {   /* the fake's losses: nation 3: (7(u+1) + 9 + 5) % 3, nation 5: (7(u+1) + 15 + 4) % 3 */
        int l3 = (7 * (u + 1) + 9 + 5) % 3, l5 = (7 * (u + 1) + 15 + 4) % 3;
        int w = 5 - l3 >= 4 - l5 ? 5 - l3 : 4 - l5, wn = 5 - l3 >= 4 - l5 ? 3 : 5;
        check(r == w && e == wn, "B5 f495C0: the strongest after the trial (%d of %d, want %d of %d)", r, (int)e, w, wn);
    }
    check(memcmp(before + (0xF1593u - DS_BASE), DP(0xF1593u), 0x88D) == 0
          && memcmp(before + (MA(u + 1) - DS_BASE), DP(MA(u + 1)), 69) == 0,
          "B5 f495C0: the area and the nations restored");

    /* B6. f48028: the empty jump-table cases and filters past 0x18 append nothing */
    {
        static const int empty[] = {3, 4, 5, 6, 0x0D, 0x0E, 0x10, 0x16, 0x19, 0x7F, -1};
        for (i = 0; i < (int)(sizeof empty / sizeof empty[0]); i++) {
            cnt = 99;
            f48028(&cnt, list, 40, 2, empty[i]);
            check(cnt == 0, "B6 f48028 filter 0x%X: empty (%d)", (unsigned)empty[i], (int)cnt);
        }
        cnt = 99;
        f48028(&cnt, list, 40, 2, 0x0B);
        check(cnt == g_adj[40][0], "B6 f48028 filter 0x0B: every neighbour (%d of %d)", (int)cnt, g_adj[40][0]);
    }

    /* B7. f4876C appends after the caller's entries */
    memcpy(g_ds, before, DS_SIZE);
    u = g_adj[40][1];
    clear_area(u + 1);
    put(u, 3, 2);
    setD8(0xF0044u + 160u * 2u + (uint32_t)u, 2);
    setD32(0xF52E0u + 40u * 2u + 4u * 3u, 5);   /* attitude of 2 towards 3 >= 0: taken */
    cnt = 2;
    list[0] = 111;
    list[1] = 112;
    f4876C(&cnt, list, 40, 2, 0x16);
    check(cnt >= 3 && list[0] == 111 && list[1] == 112 && list[2] == u, "B7 f4876C appends (%d, %d)", (int)cnt,
          (int)list[2]);

    /* B8. f49790 with population limit 0: max'(-1) stays -1, so r = 1 passes the first test */
    setD8(M_TR(40) + 1u, (uint8_t)(D8(M_TR(40) + 1u) & ~0x38u));
    setD8(M_NR(2) + 0x26u, 0);
    setD8(M_NR(3) + 0x26u, 0);
    setD16(0x112A94u + 20u * (uint32_t)(12 + 40 % 8) + 2u, 0);   /* 0 / 300 < 95 - 10 L: min(tokens, limit 0) */
    clear_area(41);
    put(40, 2, 3);
    r = f49790(1, 40, 2, 3);
    check(r == 0, "B8 f49790(1, limit 0): min(3, 0) = 0 (%d)", r);

    /* B9. f48D44 flag 0x10: scores -1, 5, -2, 3 in -> sorted 5, 3, -1, -2, the count 2, the rest left in place */
    for (i = 0; i < 4; i++)
        setD8(0xEED84u + 160u * 2u + 10u + (uint32_t)i, (uint8_t)(int8_t)(i == 0 ? -1 : i == 1 ? 5 : i == 2 ? -2 : 3));
    cnt = 4;
    list[0] = 10; list[1] = 11; list[2] = 12; list[3] = 13;
    f48D44(&cnt, list, 2, 0, 0x10);
    check(cnt == 2 && list[0] == 11 && list[1] == 13 && list[2] == 10 && list[3] == 12,
          "B9 f48D44 0x10: sorted, the count shortened only (%d: %d %d %d %d)", (int)cnt, (int)list[0], (int)list[1],
          (int)list[2], (int)list[3]);
    free(before);
}

/* ======================================================================================================================
   Part C: the saves
   ==================================================================================================================== */

static const char *nation_name(int n) { return DSTR(D32(0xC768Cu + 4u * (uint32_t)n)); }

static int load_save(const char *path, const char *gamedir)
{
    int16_t ver;
    char desc[64];
    FILE *fp = fopen(path, "rb");
    if (fp == NULL)
        return -1;
    f5F05C(&ver, desc, fp);
    fclose(fp);
    /* the map variant and the in-play bytes as main computes them (0x1164A, f12B00), then the lists and routes */
    setD16(GAME_MAP_VARIANT, (uint16_t)(7 * ((D16(0xE1624u) >> 1) & 1) + S32(0xF40F0u) - 2));
    f12B00();
    install_adj();
    if (load_rl(gamedir, S32(0xF40F0u)) != 0)
        printf("C. %d.RL not loaded\n", S32(0xF40F0u));
    memcpy(DP(0xEF3C4u), DP(0xEED84u), 0x640);   /* the loop's 0x14AA4 after a restore */
    printf("C. %s: \"%s\", phase %d, round %d, players %d (humans %d, computers %d), AI level %d, [0xF09E4] %d\n",
           path, desc, D8(0xF4102u), S16(0xF40FCu), S32(0xF40F0u), S8(0xE1620u), S8(0xE1621u),
           (D16(0xE1624u) >> 10) & 3, S32(0xF09E4u));
    return ver;
}

static void part_c_one(const char *path, const char *gamedir, int slot)
{
    int n, i, t, bits[16], pr, mr, j, first = -1;
    uint8_t *save = g_ds;
    if (load_save(path, gamedir) < 0) {
        printf("C. %s not found: skipped\n", path);
        return;
    }
    /* the movement phase's start before the first f461BC (0x556E4..0x557F1, f783B8): the order from the census
       list (nations without Military +0x35 first), 0xF06C0[nation] = its position, [0xF09E4] = some nation holds +0x28 */
    setD16(0xF06AEu, 0);
    for (i = 0; i < 2; i++)
        for (t = 0; t < S16(0xF0698u); t++) {
            int nn = S16(0xF069Au + 2u * (uint32_t)t), k = S16(0xF06AEu);
            if ((D8(M_NR(nn) + 0x35u) != 0) != (i == 1))
                continue;
            setD16(0xF06AEu, (uint16_t)(k + 1));
            setD16(0xF06B0u + 2u * (uint32_t)k, (uint16_t)nn);
        }
    if (S32(0xF40ECu) == 0)   /* the player count main sets (not in the save) */
        setD32(0xF40ECu, D32(0xF40F0u));
    for (i = 0; i < S32(0xF40ECu); i++)
        setD8(0xF06C0u + (uint32_t)S16(0xF06B0u + 2u * (uint32_t)i), (uint8_t)i);
    setD32(0xF09E4u, 0);
    for (n = 1; n <= 9; n++)
        if (D8(M_NR(n) + 0x28u) != 0) {
            setD32(0xF09E4u, 1);
            break;
        }
    printf("   at the movement's start: [0xF09E4] %d; movement order 0xF06C0:", S32(0xF09E4u));
    for (n = 1; n <= 9; n++)
        printf(" %d", S8(0xF06C0u + (uint32_t)n));
    printf("; player list 0xF06B0:");
    for (i = 0; i < S16(0xF06AEu) && i < 9; i++)
        printf(" %d", S16(0xF06B0u + 2u * (uint32_t)i));
    printf("\n   .BPM 0x04 counts (f160EC's list holds 5 a nation):");
    for (n = 1; n <= 9; n++) {
        int k = 0;
        for (t = 0; t < 160; t++)
            k += (D8(0xF0044u + 160u * (uint32_t)n + (uint32_t)t) & 4) != 0;
        printf(" %d", k);
        check(k <= 5, "C. %s nation %d flags %d territories with .BPM 0x04 (more than f160EC's 5)", path, n, k);
    }
    printf("\n");
    memcpy(M, g_ds, DS_SIZE);
    for (i = 0; i < S16(0xF06AEu); i++) {
        n = S16(0xF06B0u + 2u * (uint32_t)i);
        if (S8(M_NR(n) + 0x9Au) != 2)
            continue;
        g_log[0][0] = g_log[1][0] = 0;
        g_side = 0;
        f7BD5C(n); f46998(n); f160EC(n);
        g_side = 1; g_ds = M;
        m7BD5C(n); m46998(n); m160EC(n);
        g_ds = save;
        check(memcmp(g_ds, M, DS_SIZE) == 0, "C. %s %s: f7BD5C, f46998, f160EC equal the model", path, nation_name(n));
        memset(bits, 0, sizeof bits);
        for (t = 0; t < 157; t++)
            for (j = 0; j < 16; j++)
                bits[j] += (D16(0xF3580u + 314u * (uint32_t)(n - 1) + 2u * (uint32_t)t) >> j & 1) != 0;
        if (first < 0)
            first = i;
        printf("   %-8s%s flags lo", nation_name(n), i == first ? " (first computer)" : "");
        for (j = 0; j < 8; j++)
            printf(" %02X:%d", 1 << j, bits[j]);
        printf(" hi");
        for (j = 8; j < 12; j++)
            printf(" %02X:%d", 1 << (j - 8), bits[j]);
        g_side = 0; pr = f39C08(n);
        g_side = 1; g_ds = M; mr = m39C08(n); g_ds = save;
        check(pr == mr && memcmp(g_ds, M, DS_SIZE) == 0 && strcmp(g_log[0], g_log[1]) == 0, "C. %s %s: f39C08",
              path, nation_name(n));
        printf("; f39C08 %d", pr);
        g_side = 0; pr = f7B044(n);
        g_side = 1; g_ds = M; mr = m7B044(n); g_ds = save;
        check(pr == mr, "C. %s %s: f7B044 %d / %d", path, nation_name(n), pr, mr);
        printf(", f7B044 %d, 0xF62A8 %d, list", pr, D8(0xF62A8u + (uint32_t)n));
        for (j = 0; j < S32(0xF62B4u) && j < 8; j++)
            printf(" %d", D8(0xF62B8u + 5u * (uint32_t)(n - 1) + (uint32_t)j));
        printf("\n");
    }
    g_side = 0; pr = f468EC();
    g_side = 1; g_ds = M; mr = m468EC(); g_ds = save;
    check(pr == mr, "C. %s: f468EC", path);
    printf("   f468EC %d ([0xF5470] = %d); fake f5B588 calls: %s\n", pr, S32(0xF5470u), g_log[0][0] ? g_log[0] : "none");
    memcpy(g_base[slot], g_ds, DS_SIZE);   /* part A starts from the analysed save */
}

/* ======================================================================================================================
   main
   ==================================================================================================================== */

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:/work/advciv_run";
    const char *savedir = argc > 2 ? argv[2] : "C:/work/advciv_play";
    char cwd[400], path[400];
    int k, caught, missed = 0;

    if (state_init(gamedir) != 0)
        return 1;
    if (getcwd(cwd, sizeof cwd) == NULL)
        cwd[0] = 0;
    if (chdir(gamedir) == 0) {   /* terrflag.dat: the territory records are not in the save */
        f12394();
        (void)chdir(cwd);
    }
    if (load_adj(savedir) != 0) {
        printf("cannot read %s/LANDADJ.DAT\n", savedir);
        return 1;
    }
    M = malloc(DS_SIZE);
    g_base[0] = malloc(DS_SIZE);
    g_base[1] = malloc(DS_SIZE);
    if (M == NULL || g_base[0] == NULL || g_base[1] == NULL)
        return 1;
    memcpy(g_base[0], g_ds, DS_SIZE);
    memcpy(g_base[1], g_ds, DS_SIZE);

    snprintf(path, sizeof path, "%s/savgame0.dat", gamedir);
    part_c_one(path, gamedir, 0);
    snprintf(path, sizeof path, "%s/SAVGAMEA.DAT", savedir);
    part_c_one(path, gamedir, 1);
    g_nbase = 2;
    printf("C. %ld fail so far\n", g_fail);

    part_b();
    printf("B. %ld fail so far\n", g_fail);

    part_a(60000, "");
    g_original = 1;
    part_a(20000, " (original)");
    g_original = 0;
    for (k = 0; k < 19; k++)
        printf("   %s: %ld calls, %ld differ\n", k_names[k], g_calls[k], g_bad[k]);
    check(g_rand_calls == 0, "A. no game_rand call (%ld)", g_rand_calls);
    printf("A. %ld fail so far\n", g_fail);

    for (g_mut = 1; g_mut <= 20; g_mut++) {
        memset(g_bad, 0, sizeof g_bad);
        part_a(20000, NULL);
        for (k = 0, caught = 0; k < 19; k++)
            caught += g_bad[k] != 0;
        if (!check(caught > 0, "D. mutation %d not caught", g_mut))
            missed++;
        else
            printf("   mutation %2d caught (%d routine(s) differ)\n", g_mut, caught);
    }
    g_mut = 0;
    printf("D. %d of 20 mutations missed\n", missed);

    printf("%ld checks, %ld fail\n", g_tests, g_fail);
    return g_fail == 0 ? 0 : 1;
}
