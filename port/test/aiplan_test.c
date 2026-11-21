/* aiplan_test [GAMEDIR [PLAYDIR]]: unit checks for module AIPLAN (aiplan.c) that need no game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init; default C:\work\advciv_run, only read), terrflag.dat (f12394:
   the population limits are not in the save), LANDADJ.DAT (the land lists, parsed here into one heap block with a
   handle per territory at 0xF8E7C + 4t, as the game keeps them) and savgame0.dat (the R4 game at the start of
   MOVEMENT); PLAYDIR holds SAVGAMEA.DAT (the R4 autosave: seven computers; default C:\work\advciv_play, only read). The
   saves are read with saveload.c's f5F05C as the game does.

   aiplan.c is built with -DAIPLAN_TEST: its calls into the other modules of M7 batch 1 (AIQ 0x48028, 0x4876C,
   0x48D44, 0x495C0, 0x49790, 0x47BA0, 0x4CC5C, 0x4CEE4, 0x7C838, 0x160EC; AICITY 0x33964, 0x3580C; AISEA 0x4AC50) come
   to recorders here. Each appends one line to a trace and acts out a plausible effect, deterministic in the data
   object, its arguments, a per-check seed and its position in the call sequence (so a call made out of order or with
   other arguments changes everything after it): neighbour lists from the real land lists (filter 0: empty and
   cityless, as 0x48028; 0x16: held by one other nation, as 0x4876C; the others a seeded choice), a score sort for
   0x48D44, n's territories with movable tokens for 0x47BA0, seeded numbers for the queries, the what-if 0x7C838 writes
   the losses 0xF547C and answers "n loses everything" half the time, 0x3580C moves a token into t and adds a move
   record (so the stacks' undo is visible), 0x33964 may plan a city (+1 to *count), 0x4AC50 takes part of *k.
   The silent 0x5AAA4(0) / 0x5C238(0) are recorders too: they change nation records (undone by the World State Stack)
   and the bytes 0xF5934 outside it. The World State Stack and Move List Stack are the test's own copies of stack.c /
   mstack.c's ranges (areas 0xE31FD 0x2B20, territory states 0xE2C18 0x1E0, nations 0xF1593 0x88D; the move lists with
   their counts), traced; overflow or underflow fails the check. The primitives aiplan.c calls (rules.c's f78D08 /
   f7C760 / f7C7C4 / f57CA4 with move.c, formulas.c's f46A4C / f46B34) are the real, separately tested ones.

   The model is a second implementation of the fifteen routines written here from the listing (not from aiplan.c), in
   the listing's own terms: every field is read as the original reads it (dword [base + 69r + n - 1 + disp] sar 24,
   word reads sar 16) at the listing's displacements. Each check runs the port, keeps its data object, trace and
   outputs, restores the start state, runs the model, and compares all three: the whole data object byte for byte,
   the traces line for line, the return value and out-parameters. A run that makes more than 20,000 recorder calls is
   cut off ("CUT"), identically on both sides. Parts:
     A. hand vectors with the expected values written out: 0x49C88's one-below-limit rule and record; 0x474A0's roll
        (row 10 + c % 10, counter + 1 only for a roll, type 0x13 / 0x15); 0x46BB8's 4-byte clear; 0x49E04's first
        target only; 0x4C074's shared list ([0xF7C9C] left at the support count); 0x46CBC's empty list with [0xC28AC]
        (the stale word of the previous sibling, aiplan.c items 1./2.); 0x4C9B8's success / failure paths of the stacks; 0x49A1C's guard.
     B. random differential checks: each of the fifteen routines called directly with random arguments on random
        states (a real save with tokens moved about by the real primitives, random AI tables, flags, marks, scores,
        levels, order bytes, globals, the pre-rolled table, the roll counter).
     C. the two real saves: every routine for every computer nation, from the save's own state (the AI tables as the
        save has them), port against model; the counts of move records printed for the check plan.
     D. mutations: fifteen deliberately broken copies of the model must each be caught by part B's checks.
   Prints each failure (the first differing trace line or byte), the counts per part, and exits 0 when all pass.

   Build (from the repo root):
     clang -std=c11 -Wall -Wextra -DAIPLAN_TEST -Iport/src port/test/aiplan_test.c port/src/aiplan.c port/src/rules.c
       port/src/move.c port/src/formulas.c port/src/saveload.c port/src/load.c port/src/state.c port/src/le_image.c
       -o aiplan_test.exe */

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <direct.h>
#define chdir _chdir
#define getcwd _getcwd

#include "aiplan.h"
#include "city.h"
#include "conflict.h"
#include "formulas.h"
#include "game.h"
#include "move.h"
#include "mstack.h"
#include "rules.h"
#include "saveload.h"
#include "stack.h"
#include "state.h"

void f12394(void);   /* load.c: terrflag.dat into the territory records */

/* ---- addresses (from the listing, written out here) ------------------------------------------------------------ */

#define T_NAT(n)    (0xF14CCu + 199u * (uint32_t)(n))
#define T_AREA(r)   (0xE31B8u + 69u * (uint32_t)(r))
#define T_TS(t)     (0xE2C18u + 3u * (uint32_t)(t))
#define T_TERR(t)   (0xE1638u + 35u * (uint32_t)(t))
#define T_BPM(n, t) (0xF0044u + 160u * (uint32_t)(n) + (uint32_t)(t))
#define T_SC(n, t)  (0xEED84u + 160u * (uint32_t)(n) + (uint32_t)(t))
#define T_FL(n, t)  (0xF3580u + 314u * (uint32_t)((n) - 1) + 2u * (uint32_t)(t))
#define T_TAB(r, n) (0x112A94u + 20u * (uint32_t)(r) + 2u * (uint32_t)((n) - 1))

/* ---- trace, checks ------------------------------------------------------------------------------------------- */

#define LOGSZ (1u << 20)
static char g_log[2][LOGSZ];
static size_t g_len[2];
static int g_cur;            /* 0 the port's run, 1 the model's */
static int g_tests, g_fail, g_verbose = 1;

static void tr(const char *fmt, ...)
{
    va_list ap;
    int k;
    if (g_len[g_cur] + 300 >= LOGSZ)
        return;
    va_start(ap, fmt);
    k = vsnprintf(g_log[g_cur] + g_len[g_cur], LOGSZ - g_len[g_cur], fmt, ap);
    va_end(ap);
    if (k > 0)
        g_len[g_cur] += (size_t)k;
    g_log[g_cur][g_len[g_cur]++] = '\n';
    g_log[g_cur][g_len[g_cur]] = 0;
}

static int check(int ok, const char *what)
{
    g_tests++;
    if (!ok) {
        g_fail++;
        if (g_verbose)
            printf("FAIL %s\n", what);
    }
    return ok;
}

/* ---- the land lists ------------------------------------------------------------------------------------------ */

static int g_adjn[160], g_adj[160][32];
static uint32_t g_adj_block;

static int load_landadj(const char *gamedir)
{
    char path[1024];
    FILE *fp;
    uint8_t b[2], sz[4];
    int t, i, k;
    uint8_t *blk;
    uint32_t off;

    snprintf(path, sizeof path, "%s/LANDADJ.DAT", gamedir);
    fp = fopen(path, "rb");
    if (fp == NULL)
        return 0;
    if (fread(sz, 4, 1, fp) != 1) {
        fclose(fp);
        return 0;
    }
    for (t = 0; t < 160; t++) {
        if (fread(b, 2, 1, fp) != 1)
            break;
        k = (int16_t)(b[0] | b[1] << 8);
        if (k < 0 || k > 32)
            break;
        g_adjn[t] = k;
        for (i = 0; i < k; i++) {
            if (fread(b, 2, 1, fp) != 1)
                break;
            g_adj[t][i] = (int16_t)(b[0] | b[1] << 8);
        }
    }
    fclose(fp);
    if (t != 160)
        return 0;
    g_adj_block = load_heap_alloc(2u * (160u + 160u * 32u));
    blk = load_heap_ptr(g_adj_block, 2u * (160u + 160u * 32u));
    if (blk == NULL)
        return 0;
    off = 0;
    for (t = 0; t < 160; t++) {
        setD32(0xF8E7Cu + 4u * (uint32_t)t, g_adj_block + off);
        blk[off] = (uint8_t)g_adjn[t];
        blk[off + 1] = 0;
        for (i = 0; i < g_adjn[t]; i++) {
            blk[off + 2 + 2 * i] = (uint8_t)g_adj[t][i];
            blk[off + 3 + 2 * i] = (uint8_t)((unsigned)g_adj[t][i] >> 8);
        }
        off += 2u + 2u * (uint32_t)g_adjn[t];
    }
    return 1;
}

/* ---- the recorders: deterministic effects ---------------------------------------------------------------------- */

static uint32_t g_seed;      /* per check */
static uint32_t g_calls;     /* per run */
static jmp_buf g_cut;
static int g_cut_on;
#define BUDGET 20000u

static uint32_t hh(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    uint32_t x = g_seed ^ (g_calls * 0x165667B1u);
    x ^= a * 0x9E3779B1u;
    x = (x ^ (x >> 15)) * 0x85EBCA77u;
    x ^= b * 0xC2B2AE3Du;
    x = (x ^ (x >> 13)) * 0x27D4EB2Fu;
    x ^= c * 0x9E3779B1u + d;
    x = (x ^ (x >> 16)) * 0x85EBCA77u;
    return x ^ (x >> 13);
}

static void tick(void)
{
    g_calls++;
    if (g_calls > BUDGET && g_cut_on)
        longjmp(g_cut, 1);
}

static int t_tok(int t, int n) { return (int8_t)D8(T_AREA(t + 1) + 0x0Eu + (uint32_t)n); }
static int t_cnt(int t)        { return (int8_t)D8(T_AREA(t + 1) + 1u); }

static void trace_list(const char *who, int a, int b, int c, const int32_t *cnt, const int32_t *list)
{
    char buf[600];
    int i, len;
    len = snprintf(buf, sizeof buf, "%s %d %d %d -> %d:", who, a, b, c, (int)*cnt);
    for (i = 0; i < *cnt && i < 40 && len < 560; i++)
        len += snprintf(buf + len, sizeof buf - (size_t)len, " %d", (int)list[i]);
    tr("%s", buf);
}

/* scripted answers for part A (count -1: not scripted) */
static int g_scr_n = -1, g_scr[32];
static int g_scr_c838 = -1000;   /* f7C838's answer when not -1000 */
static int g_scr_cc5c = -1000;   /* f4CC5C's answer */
static int g_scr_3580 = -1000;   /* f3580C's answer */

void aiplan_test_f48028(int32_t *cnt, int32_t *list, int t, int n, int filter)
{
    int i, u;
    tick();
    *cnt = 0;
    if (g_scr_n >= 0) {
        for (i = 0; i < g_scr_n; i++)
            list[(*cnt)++] = g_scr[i];
    } else if (t >= 0 && t < 160) {
        for (i = 0; i < g_adjn[t]; i++) {
            u = g_adj[t][i];
            if (u < 0 || u >= 157)
                continue;
            if (filter == 0) {
                if (t_cnt(u) == 0 && (D8(T_TS(u) + 2u) & 1) == 0)
                    list[(*cnt)++] = u;
            } else if (hh((uint32_t)t, (uint32_t)u, (uint32_t)n, (uint32_t)filter) % 3u != 0)
                list[(*cnt)++] = u;
        }
    }
    trace_list("48028", t, n, filter, cnt, list);
}

void aiplan_test_f4876C(int32_t *cnt, int32_t *list, int t, int n, int filter)
{
    int i, j, u, o, dup;
    tick();
    if (filter == 0x16 && t >= 0 && t < 160 && g_scr_n < 0)
        for (i = 0; i < g_adjn[t]; i++) {
            u = g_adj[t][i];
            if (u < 0 || u >= 157 || t_cnt(u) != 1)
                continue;
            o = f7C7C4(u, n);
            if (o == 0 || hh((uint32_t)u, (uint32_t)n, (uint32_t)o, 0x16u) % 2u != 0)
                continue;
            for (dup = 0, j = 0; j < *cnt; j++)
                if (list[j] == u)
                    dup = 1;
            if (!dup && *cnt < 160)
                list[(*cnt)++] = u;
        }
    trace_list("4876C", t, n, filter, cnt, list);
}

void aiplan_test_f48D44(int32_t *cnt, int32_t *list, int n, int t, int flags)
{
    int32_t tmp[160];
    int i, j, k = 0;
    tick();
    for (i = 0; i < *cnt && i < 160; i++)   /* 0x10: keep score >= 0 */
        if (!(flags & 0x10) || (int8_t)D8(T_SC(n, list[i])) >= 0)
            tmp[k++] = list[i];
    for (i = 1; i < k; i++)                 /* stable insertion sort, score descending */
        for (j = i; j > 0 && (int8_t)D8(T_SC(n, tmp[j - 1])) < (int8_t)D8(T_SC(n, tmp[j])); j--) {
            int32_t x = tmp[j];
            tmp[j] = tmp[j - 1];
            tmp[j - 1] = x;
        }
    for (i = 0; i < k; i++)
        list[i] = tmp[i];
    *cnt = k;
    trace_list("48D44", n, t, flags, cnt, list);
}

int aiplan_test_f495C0(int t, int n, int mode, int32_t *enemy)
{
    uint32_t h;
    tick();
    h = hh((uint32_t)t, (uint32_t)n, (uint32_t)mode, 0x495C0u);
    *enemy = 1 + (int32_t)(h % 9u);
    tr("495C0 %d %d %d -> %d e%d", t, n, mode, (int)((h >> 8) % 4u), (int)*enemy);
    return (int)((h >> 8) % 4u);
}

int aiplan_test_f49790(int r, int t, int n, int enemy)
{
    int v;
    tick();
    v = r == 0 ? 0 : (int)((uint32_t)(r + 7) * 13u + hh((uint32_t)t, (uint32_t)n, (uint32_t)enemy, 0x49790u)) % 3;
    tr("49790 %d %d %d %d -> %d", r, t, n, enemy, v);
    return v;
}

void aiplan_test_f47BA0(int32_t *cnt, int32_t *list, int n, int mode, int flags)
{
    int t, i, j;
    tick();
    *cnt = 0;
    for (t = 0; t < 157; t++) {
        int r = t + 1;
        int8_t mv = (int8_t)D8(T_AREA(r) + 0x24u + (uint32_t)n), tk = (int8_t)D8(T_AREA(r) + 0x0Eu + (uint32_t)n);
        if (mode == 3 && mv > 0 && (D8(T_TS(t) + 2u) & 2) == 0)
            list[(*cnt)++] = t;
        else if (mode == 0x0E && tk > 2 && (D8(T_TERR(t) + 1u) & 0x40) != 0)
            list[(*cnt)++] = t;
    }
    if (flags & 1)
        for (i = 1; i < *cnt; i++)
            for (j = i; j > 0 && (int8_t)D8(T_SC(n, list[j - 1])) > (int8_t)D8(T_SC(n, list[j])); j--) {
                int32_t x = list[j];
                list[j] = list[j - 1];
                list[j - 1] = x;
            }
    trace_list("47BA0", n, mode, flags, cnt, list);
}

int aiplan_test_f4CC5C(int t, int n, int mode)
{
    int v;
    tick();
    v = g_scr_cc5c != -1000 ? g_scr_cc5c : (int)(hh((uint32_t)t, (uint32_t)n, (uint32_t)mode, 0x4CC5Cu) % 5u) - 1;
    tr("4CC5C %d %d %d -> %d", t, n, mode, v);
    return v;
}

int aiplan_test_f4CEE4(int t, int n, int32_t *out)
{
    uint32_t h;
    tick();
    h = hh((uint32_t)t, (uint32_t)n, 0, 0x4CEE4u);
    *out = (int32_t)(h % 8u);
    tr("4CEE4 %d %d -> %d o%d", t, n, (int)((h >> 4) % 6u) - 1, (int)*out);
    return (int)((h >> 4) % 6u) - 1;
}

int aiplan_test_f7C838(int t, int n, int k, int32_t *left)
{
    uint32_t h;
    int i, v;
    tick();
    h = hh((uint32_t)t, (uint32_t)n, (uint32_t)k, 0x7C838u);
    memset(DP(0xF547Cu), 0, 10);
    for (i = 1; i <= 9; i++)
        setD8(0xF547Cu + (uint32_t)i, (uint8_t)(hh((uint32_t)i, h, 1u, 2u) % 8u == 0 ? 0xFEu   /* signed bytes */
                                                                               : hh((uint32_t)i, h, 1u, 2u) % 5u));
    *left = (int32_t)(h % 3u);
    if (g_scr_c838 != -1000)
        v = g_scr_c838;
    else if ((h >> 3) & 1u)
        v = t >= -1 && t < 160 ? t_tok(t, n) : 0;
    else
        v = (int)((h >> 5) % 4u);
    tr("7C838 %d %d %d -> %d l%d", t, n, k, v, (int)*left);
    return v;
}

void aiplan_test_f160EC(int n)
{
    tick();
    setD8(0xF62A8u + (uint32_t)n, (uint8_t)hh((uint32_t)n, 0, 0, 0x160ECu));
    tr("160EC %d", n);
}

void aiplan_test_f33964(int goal, int32_t *count, int n, int mode)
{
    tick();
    if (hh((uint32_t)goal, (uint32_t)*count, (uint32_t)n, (uint32_t)mode) % 2u)
        (*count)++;
    tr("33964 %d %d %d -> %d", goal, n, mode, (int)*count);
}

int aiplan_test_f3580C(int need, int t, int n, int mode, int s)
{
    int v;
    int32_t c;
    tick();
    v = g_scr_3580 != -1000 ? g_scr_3580 : (int)(hh((uint32_t)need, (uint32_t)t, (uint32_t)n, (uint32_t)s) % 2u);
    if (t >= 0 && t < 157) {   /* a token into t and its move record: the stacks must undo both on failure */
        f57E28(t, n, 1);
        c = (int32_t)D32(0x1079B4u);
        if (c >= 0 && c < 200) {
            setD8(0x107694u + 4u * (uint32_t)c, (uint8_t)t);
            setD8(0x107695u + 4u * (uint32_t)c, (uint8_t)t);
            setD8(0x107696u + 4u * (uint32_t)c, 1);
            setD8(0x107697u + 4u * (uint32_t)c, 0xC8);
            setD32(0x1079B4u, (uint32_t)(c + 1));
        }
    }
    tr("3580C %d %d %d %d %d -> %d", need, t, n, mode, s, v);
    return v;
}

int aiplan_test_f4AC50(int t, int n, int32_t *k, int mode)
{
    int x;
    tick();
    x = (int)(hh((uint32_t)t, (uint32_t)n, (uint32_t)*k, (uint32_t)mode) % 3u);
    if (x > *k)
        x = *k;
    *k -= x;
    tr("4AC50 %d %d %d -> %d k%d", t, n, mode, x, (int)*k);
    return x;
}

/* the silent conflict and surplus removal (conflict.h / city.h), recorders here */
void f5AAA4(int show)
{
    int n;
    tick();
    for (n = 1; n <= 9; n++)
        if (hh((uint32_t)n, 0x5AAA4u, 0, 0) % 3u == 0)
            setD8(T_NAT(n) + 0x98u, (uint8_t)(D8(T_NAT(n) + 0x98u) - 1));
    setD8(0xF5934u, (uint8_t)(D8(0xF5934u) + 1));   /* outside the stack: stays */
    tr("5AAA4 %d", show);
}

void f5C238(int show)
{
    int n;
    tick();
    for (n = 1; n <= 9; n++)
        if (hh((uint32_t)n, 0x5C238u, 0, 0) % 4u == 0)
            setD8(T_NAT(n) + 0x51u, (uint8_t)(D8(T_NAT(n) + 0x51u) + 1));
    setD8(0xF5935u, (uint8_t)(D8(0xF5935u) + 1));
    tr("5C238 %d", show);
}

/* ---- the two stacks (stack.c / mstack.c ranges) ------------------------------------------------------------------ */

static uint8_t s_ws_area[10][0x2B20], s_ws_ts[10][0x1E0], s_ws_nat[10][0x88D];
static uint8_t s_ms_l1[7][800], s_ms_l2[7][900];
static int32_t s_ms_c1[7], s_ms_c2[7];
static int g_stack_err;

void f7A940(void)
{
    int32_t d = (int32_t)D32(0xCAC54u);
    tick();
    tr("WS push %d", d);
    if (d >= 9) {
        g_stack_err = 1;
        return;
    }
    d++;
    setD32(0xCAC54u, (uint32_t)d);
    memcpy(s_ws_area[d], DP(0xE31FDu), 0x2B20);
    memcpy(s_ws_ts[d], DP(0xE2C18u), 0x1E0);
    memcpy(s_ws_nat[d], DP(0xF1593u), 0x88D);
}

void f7AC60(void)
{
    int32_t d = (int32_t)D32(0xCAC54u);
    tick();
    tr("WS pop %d", d);
    if (d < 0) {
        g_stack_err = 1;
        return;
    }
    memcpy(DP(0xE31FDu), s_ws_area[d], 0x2B20);
    memcpy(DP(0xE2C18u), s_ws_ts[d], 0x1E0);
    memcpy(DP(0xF1593u), s_ws_nat[d], 0x88D);
    setD32(0xCAC54u, (uint32_t)(d - 1));
}

void f7AD24(void)
{
    int32_t d = (int32_t)D32(0xCAC54u);
    tick();
    tr("WS drop %d", d);
    if (d < 0) {
        g_stack_err = 1;
        return;
    }
    setD32(0xCAC54u, (uint32_t)(d - 1));
}

void f7AA1C(void)
{
    int32_t d = (int32_t)D32(0xCAC58u);
    int32_t c1 = (int32_t)D32(0x1079B4u), c2 = (int32_t)D32(0x107D3Cu);
    tick();
    tr("MS push %d %d %d", d, c1, c2);
    if (d >= 6 || c1 < 0 || c1 > 200 || c2 < 0 || c2 > 150) {
        g_stack_err = 1;
        return;
    }
    d++;
    setD32(0xCAC58u, (uint32_t)d);
    s_ms_c1[d] = c1;
    s_ms_c2[d] = c2;
    memcpy(s_ms_l1[d], DP(0x107694u), 4u * (uint32_t)c1);
    memcpy(s_ms_l2[d], DP(0x1079B8u), 6u * (uint32_t)c2);
}

void f7AE14(void)
{
    int32_t d = (int32_t)D32(0xCAC58u);
    tick();
    tr("MS pop %d", d);
    if (d < 0) {
        g_stack_err = 1;
        return;
    }
    setD32(0x1079B4u, (uint32_t)s_ms_c1[d]);
    setD32(0x107D3Cu, (uint32_t)s_ms_c2[d]);
    memcpy(DP(0x107694u), s_ms_l1[d], 4u * (uint32_t)s_ms_c1[d]);
    memcpy(DP(0x1079B8u), s_ms_l2[d], 6u * (uint32_t)s_ms_c2[d]);
    setD32(0xCAC58u, (uint32_t)(d - 1));
}

void f7AD7C(void)
{
    int32_t d = (int32_t)D32(0xCAC58u);
    tick();
    tr("MS drop %d", d);
    if (d < 0) {
        g_stack_err = 1;
        return;
    }
    setD32(0xCAC58u, (uint32_t)(d - 1));
}

/* the rest of what the linked files reference (move.c's button cases, saveload.c's table, rules.c's ship removal):
   none is reached from aiplan.c's routines; reaching one ends the test */
_Noreturn void not_ported(uint32_t addr)
{
    printf("not_ported(0x%X) reached\n", (unsigned)addr);
    exit(3);
}
static _Noreturn void unexpected(const char *what)
{
    printf("unexpected call of %s\n", what);
    exit(3);
}
void f3A780(const char *msg) { (void)msg; unexpected("f3A780"); }
void f3B7CC(uint32_t dlg, int id) { (void)dlg; (void)id; unexpected("f3B7CC"); }
void f3B870(uint32_t dlg, int id) { (void)dlg; (void)id; unexpected("f3B870"); }
int  f3B9A4(uint32_t dlg, int id) { (void)dlg; (void)id; unexpected("f3B9A4"); }
void f3BFBC(uint32_t dlg, int id, int en) { (void)dlg; (void)id; (void)en; unexpected("f3BFBC"); }
void f3C3D0(uint32_t dlg, int id) { (void)dlg; (void)id; unexpected("f3C3D0"); }
void f3CBBC(void) { unexpected("f3CBBC"); }
void f3D52C(int dx, int dy) { (void)dx; (void)dy; unexpected("f3D52C"); }
void f3DA60(int t) { (void)t; unexpected("f3DA60"); }
int  f3E89C(int t) { (void)t; unexpected("f3E89C"); }
int  f3E8FC(int t) { (void)t; unexpected("f3E8FC"); }
void f3F624(int t, int c) { (void)t; (void)c; unexpected("f3F624"); }
void f3F670(void) { unexpected("f3F670"); }
void f3F698(int t, int c) { (void)t; (void)c; unexpected("f3F698"); }
void f3F70C(int t, int c) { (void)t; (void)c; unexpected("f3F70C"); }
void f426D0(int i) { (void)i; unexpected("f426D0"); }
_Noreturn void f44CAC(void) { unexpected("f44CAC"); }
void f572CC(int t) { (void)t; unexpected("f572CC"); }
void f76F70(int n, uint8_t *cards) { (void)n; (void)cards; unexpected("f76F70"); }
int  f78AE4(void) { unexpected("f78AE4 (game_rand)"); }
void fg_vbclose(void) { unexpected("fg_vbclose"); }
int  fg_vbopen(int handle) { (void)handle; unexpected("fg_vbopen"); }

/* ===================================================================================================================
   The model: the fifteen routines again, from the listing. Fields are read as the listing reads them: a byte through
   "mov eax, dword ptr [X]; sar eax, 0x18" (the byte at X + 3, signed), a word through "sar 0x10".
   =================================================================================================================== */

static int g_mut;   /* part D: which deliberate break is active (0 none) */

#define L24(a)  ((int32_t)D32((uint32_t)(a)) >> 24)
#define L16(a)  ((int32_t)D32((uint32_t)(a)) >> 16)
#define U(v)    ((uint32_t)(int32_t)(v))
#define R69(r)  (69u * U(r))
#define LIM(t)  ((int32_t)((uint8_t)(D8(35u * U(t) + 0xE1639u) << 2) >> 5))   /* lea 9t; shl 2; sub t: 35t */

static int32_t lw(uint32_t h)   /* movsx word [h] in a land list */
{
    const uint8_t *p = load_heap_ptr(h, 2);
    if (p)
        return (int16_t)(p[0] | p[1] << 8);
    return ds_valid(h, 2) ? (int16_t)D16(h) : 0;
}

static int32_t m_level(void)    /* mov ax, word [0xE1624]; shl eax, 4; shr ax, 0xE */
{
    uint32_t eax = D16(0xE1624u);
    eax <<= 4;
    return (int32_t)((eax & 0xFFFFu) >> 14);
}

int m46CBC(int a, int b);
int32_t *aiplan_test_slot38(void);   /* aiplan.c (AIPLAN_TEST): the real word */
int m49A1C(int a, int b, int c, int d);
int f49C88_model(int a, int b);
void m471CC(int32_t *a, int32_t *b, int c);
void m4A088(int a, int b, int c);
void m4C074(int a);
int m4A2E0(int a, int b, int c, int d, const int32_t *e, int f, int g, int h);

/* The word at [ebp-0x38] of 0x46BB8's children (the model's own copy; 0x47BA0 leaves its list pointer there,
   0x471CC n - 1, 0x46CBC its final avail) */
static int32_t ms_slot38;

/* 0x46BB8 */
void m46BB8(int v28)
{
    int32_t v18, v20, v24, v1C, lst[160];
    if (f46B34(v28) != 0)
        return;
    memset(DP(0x10819Cu), 0, 0xA0);
    if (g_mut == 11)
        memset(DP(D32(0xC2860u)), 0, 0x280);
    else
        memset(DP(D32(0xC2860u)), 0, 4);
    memset(DP(0x10823Cu), 0, 0xA0);
    v20 = 1;
    v24 = 1;
L_46C21:
    if (v24 == 0)
        goto L_46CA9;
    v24 = 0;
    aiplan_test_f47BA0(&v18, lst, v28, 3, 1);
    ms_slot38 = 0x00310000;
    if (v20 != 0 && D8(0xC28ACu) != 0) {
        m471CC(&v18, lst, v28);
        ms_slot38 = v28 - 1;
        v20 = 0;
    }
    for (v1C = 0; v1C < v18; v1C++)
        if (m46CBC(lst[v1C], v28) == 1) {
            v24 = 1;
            setD32(4u * U(v28) + 0x108164u, 1);
        }
    goto L_46C21;
L_46CA9:
    aiplan_test_f160EC(v28);
}

/* 0x474A0 (ret 8) */
void m474A0(int v28, int v34, int32_t *v30, int32_t *v2C, int p8, const int32_t *pC)
{
    int32_t v18 = v34 - 1, v1C = v28 + 1, v14, v24, v8, lst[160], vC, v10, v20, ebx, edx, pass;
    if (*v30 <= 0)
        return;
    if ((D8(314u * U(v18) + 2u * U(v28) + 0xF3580u) & 0x20) == 0)
        return;
    if (L24(U(v28) + 0x108199u) > 0)
        return;
    v14 = 0;
    for (v24 = 0; v24 < p8; v24++)
        if (D8(160u * U(v34) + U(pC[v24]) + 0xF0044u) & 2) {
            v14 = 1;
            break;
        }
    if (v14 == 0 && p8 > 1)
        return;
    aiplan_test_f48028(&v8, lst, v28, v34, 0xB);
    aiplan_test_f48D44(&v8, lst, v34, v28, 1);
    for (pass = 0; pass < 2; pass++)
        for (v24 = 0; v24 < v8; v24++) {
            vC = lst[v24];
            if (pass == 0) {
                if ((D8(314u * U(v18) + 2u * U(vC) + 0xF3581u) & 2) == 0)
                    continue;
            } else if ((D8(314u * U(v18) + 2u * U(vC) + 0xF3580u) & 0x10) == 0)
                continue;
            v10 = aiplan_test_f4CC5C(vC, v34, 3);
            if (v10 <= 0)
                continue;
            if (L24(R69(v1C) + U(v18) + 0xE31DAu) < v10)
                continue;
            if (f78D08(vC, v34) < v10)
                continue;
            if (aiplan_test_f7C838(vC, v34, v10, &v20) != 0)
                continue;
            ebx = (int32_t)D32(0x108198u);
            edx = ebx % 10 + 10;
            if (g_mut == 2)
                edx = ebx % 10 + 11;
            if (g_mut != 3)
                setD32(0x108198u, U(ebx + 1));
            edx = L16(20u * U(edx) + 2u * U(v18) + 0x112A92u);
            if (edx / 300 >= 0x5A - m_level() * 20)
                continue;
            f57CA4(v28, vC, v34, 1, v10);
            setD8(R69(vC + 1) + U(v18) + 0xE31F4u, 0);
            edx = (int32_t)D32(0x1079B4u);
            setD8(4u * U(edx) + 0x107694u, (uint8_t)v28);
            setD8(4u * U(edx) + 0x107695u, (uint8_t)vC);
            setD8(4u * U(edx) + 0x107697u, pass == 0 ? 0x13 : 0x15);
            setD32(0x1079B4u, U(edx + 1));
            setD8(4u * U(edx) + 0x107696u, (uint8_t)v10);
            *v30 -= v10;
            if (D32(0xC28B4u) != 0)
                setD8(U(v28) + 0x10819Cu, 1);
            *v2C = 1;
        }
}

/* 0x46CBC */
int m46CBC(int v48, int v50)
{
    int32_t v44 = 0, v18 = v48 + 1, v40 = v50 - 1, v14, lst[160], v38 = ms_slot38, v34, v30, v1C, v3C, v2C, v4C, v28, v24,
            v20, eax, edx;
    uint8_t lo;
    aiplan_test_f48028(&v14, lst, v48, v50, 0);
    aiplan_test_f4876C(&v14, lst, v48, v50, 0x16);
    aiplan_test_f48D44(&v14, lst, v50, v48, D8(U(v50) + 0xF488Cu) == 0 ? 0x36 : 0x436);
    if (v14 != 0) {
        v38 = L24(R69(v18) + U(v40) + 0xE31DAu);
        v34 = L24(R69(v18) + U(v40) + 0xE31C4u) - v38;
        v30 = 0;
        v1C = aiplan_test_f495C0(v48, v50, 0, &v3C);
        v1C = aiplan_test_f49790(v1C, v48, v50, v3C);
        if (D8(160u * U(v50) + U(v48) + 0xF0044u) & 1)
            v30 = v1C;
        else if (v1C != 0) {
            if (m49A1C(v48, v50, v1C, -1) == 0)
                v30 = v1C;
        }
        if (v30 != 0 && v34 == 0)
            v38 -= v1C;
        if (v44 == 0 && v38 > 0)
            v44 = 1;
    }
    if (D8(0xC28ACu) != 0)
        m474A0(v48, v50, &v38, &v44, v14, lst);
    if (v14 == 0)
        { ms_slot38 = v38; return v44; }
    v2C = v38;
    for (v4C = 0; v4C < v14 && v38 > 0; v4C++) {
        v28 = lst[v4C];
        v24 = v28 + 1;
        if ((int8_t)D8(160u * U(v50) + U(v28) + 0xEED84u) > (int8_t)D8(160u * U(v50) + U(v48) + 0xEED84u) &&
            D8(U(v48) + 0x10823Cu) == 0 &&
            ((D8(314u * U(v40) + 2u * U(v28) + 0xF3580u) & 0x80) || (D8(314u * U(v40) + 2u * U(v28) + 0xF3580u) & 4))) {
            v38 -= f49C88_model(v48, v50);
            setD8(U(v48) + 0x10823Cu, 1);
            continue;
        }
        v20 = 1;
        if ((D8(160u * U(v50) + U(v28) + 0xF0044u) & 2) && L24(R69(v24) + 0xE31B6u) == 1) {
            v20 = v38;
            if (LIM(v28) < v20)
                v20 = LIM(v28);
        } else if ((D8(160u * U(v50) + U(v28) + 0xF0044u) & 2) && L24(R69(v24) + 0xE31B6u) > 1) {
            eax = f7C760(v28);
            v20 = L24(R69(v24) + U(eax) + 0xE31C3u);
            if (v20 > v38) {
                edx = v38 + v20;
                v20 = edx > LIM(v28) ? 0 : v38;
            }
        } else {
            lo = D8(314u * U(v40) + 2u * U(v28) + 0xF3580u);
            if ((D8(314u * U(v40) + 2u * U(v28) + 0xF3581u) & 4) || (lo & 0x20) || (lo & 0x40)) {
                v20 = aiplan_test_f495C0(v28, v50, 0, &v3C);
                if (v20 == 0)
                    v20 = 1;
                else if (LIM(v28) < v20)
                    v20 = LIM(v28);
                if (v20 > v38)
                    v20 = v38;
            }
        }
        if (v20 == 0)
            continue;
        f57CA4(v48, v28, v50, 1, v20);
        if (L24(R69(v24) + 0xE31B6u) == 1)
            setD8(R69(v24) + U(v40) + 0xE31F4u, 0);
        edx = (int32_t)D32(0x1079B4u);
        setD8(4u * U(edx) + 0x107694u, (uint8_t)v48);
        setD8(4u * U(edx) + 0x107695u, (uint8_t)v28);
        setD8(4u * U(edx) + 0x107697u, g_mut == 1 ? 6 : 5);
        setD32(0x1079B4u, U(edx + 1));
        setD8(4u * U(edx) + 0x107696u, (uint8_t)v20);
        v38 -= v20;
    }
    if (v2C != v38 && D32(0xC28B4u) != 0)
        setD8(U(v48) + 0x10819Cu, 1);
    ms_slot38 = v38;
    return v44;
}

/* 0x471CC */
void m471CC(int32_t *v44, int32_t *v4C, int v3C)
{
    int32_t v38 = v3C - 1, v48, v40, v18, v1C, l2[160], v34 = v3C, v30, v2C, v28, v24, v10, v20, cp[160], v14;
    uint32_t mk;
    for (v48 = 0; v48 < *v44; v48++) {
        v40 = v4C[v48];
        v18 = v40 + 1;
        if (g_mut == 12)
            v34 = 0;
        aiplan_test_f48028(&v1C, l2, v40, v3C, 0);
        aiplan_test_f4876C(&v1C, l2, v40, v3C, 0x16);
        aiplan_test_f48D44(&v1C, l2, v3C, v40, 0x36);
        if (v1C != 0) {
            v34 = L24(R69(v18) + U(v38) + 0xE31DAu);
            v30 = L24(R69(v18) + U(v38) + 0xE31C4u) - v34;
            v2C = 0;
            v28 = aiplan_test_f495C0(v40, v3C, 0, &v24);
            v28 = aiplan_test_f49790(v28, v40, v3C, v24);
            if (D8(160u * U(v3C) + U(v40) + 0xF0044u) & 1)
                v2C = v28;
            else if (v28 != 0 && m49A1C(v40, v3C, v28, -1) == 0)
                v2C = v28;
            if (v2C != 0 && v30 == 0)
                v34 -= v28;
        }
        if (v34 <= 0)
            continue;
        if ((D8(314u * U(v38) + 2u * U(v40) + 0xF3580u) & 0x20) == 0)
            continue;
        if (L24(U(v40) + 0x108199u) > 0)
            continue;
        v10 = 0;
        for (v20 = 0; v20 < v1C; v20++)
            if (D8(160u * U(v3C) + U(l2[v20]) + 0xF0044u) & 2) {
                v10 = 1;
                break;
            }
        if (v10 != 0 || v1C <= 1)
            setD32(D32(0xC2860u) + 4u * U(v40), 1);
    }
    memcpy(cp, v4C, 4u * U(*v44));
    v14 = *v44;
    *v44 = 0;
    mk = D32(0xC2860u);
    for (v48 = 0; v48 < v14; v48++)
        if (D32(mk + 4u * U(cp[v48])) != 0)
            v4C[(*v44)++] = cp[v48];
    for (v48 = 0; v48 < v14; v48++)
        if (D32(mk + 4u * U(cp[v48])) == 0)
            v4C[(*v44)++] = cp[v48];
}

/* the spare measure shared by 0x478D8 and 0x47A44 */
static int32_t m_measure(int n, const int32_t *cnt)
{
    int32_t edx, eax;
    f7A940();
    f5AAA4(0);
    f5C238(0);
    edx = L24(199u * U(n) + 0xF151Au) + *cnt;
    edx += edx;
    eax = L24(199u * U(n) + 0xF1561u);
    edx += 2;
    eax -= g_mut == 14 ? 1 : 2;
    eax -= edx;
    f7AC60();
    return eax;
}

/* 0x478D8 */
void m478D8(int v10, int32_t *v1C, int v18, int32_t *v14)
{
    int32_t v4, vC, lst[160], v8;
    if (L24(199u * U(v18) + 0xF151Au) + *v1C >= 9)
        return;
    v4 = m_measure(v18, v1C);
    if (v4 <= 0 && *v14 == 0) {
        *v14 = 1;
        m4A088(v18, 0, 1);
        m4C074(v18);
        v4 = m_measure(v18, v1C);
    }
    if (v4 <= 0)
        return;
    setD8(199u * U(v18) + 0xF1529u, (uint8_t)v4);
    aiplan_test_f47BA0(&vC, lst, v18, 0xE, 0);
    if (vC == 0)
        return;
    v8 = *v1C;
    aiplan_test_f33964(v10 + 1, v1C, v18, 1);
    if (v8 != *v1C)
        setD32(0x108194u, 1);
}

/* 0x47A44 */
void m47A44(int v14, int32_t *v10, int vC, int32_t *v8)
{
    int32_t v4;
    if (L24(199u * U(vC) + 0xF1561u) < 0x14)
        return;
    if (f46A4C(vC) == 0)
        return;
    if (L24(199u * U(vC) + 0xF151Au) + *v10 >= 9)
        return;
    v4 = m_measure(vC, v10);
    if (v4 <= 0 && *v8 == 0) {
        *v8 = 1;
        m4A088(vC, 0, 1);
        m4C074(vC);
        v4 = m_measure(vC, v10);
    }
    if (v4 <= 0)
        return;
    setD8(199u * U(vC) + 0xF1529u, (uint8_t)v4);
    aiplan_test_f33964(v14 + (int32_t)D32(0x108194u) + 1, v10, vC, 0);
}

/* 0x49A1C */
int m49A1C(int v30, int v34, int v24, int v2C)
{
    int32_t v10 = 0, vC = 0, v14 = v34 - 1, v1C, v18, v20, lst[160], v28, edx, ebx, ecx;
    uint8_t v8;
    if (L24(0x1082D9u) != 0)
        return v10;
    setD8(0x1082DCu, 1);
    f7A940();
    v1C = (int32_t)D32(0x1079B4u);
    v18 = v30 + 1;
    aiplan_test_f48028(&v20, lst, v30, v34, 0xA);
    for (v28 = 0; v28 < v20; v28++)
        if (lst[v28] == v2C) {
            lst[v28] = -1;
            break;
        }
    setD8(R69(v18) + 0xE31B9u, 0);
    setD8(R69(v18) + U(v34) + 0xE31B9u, 0);
    setD8(R69(v18) + U(v34) + 0xE31C6u, 0);
    v8 = D8(160u * U(v34) + U(v30) + 0xF0044u);
    if ((v8 & 2) == 0)
        setD8(160u * U(v34) + U(v30) + 0xF0044u, (uint8_t)(D8(160u * U(v34) + U(v30) + 0xF0044u) | 2));
    setD32(0xC28B4u, 0);
    for (v28 = 0; v28 < v20; v28++) {
        if (lst[v28] == -1)
            continue;
        if (m46CBC(lst[v28], v34) != 1)
            continue;
        if (L24(R69(v18) + U(v14) + 0xE31C4u) == 0)
            continue;
        vC = 1;
        break;
    }
    setD32(0xC28B4u, 1);
    if (vC != 0) {
        edx = v30 % 8 + 0xC;
        edx = L16(20u * U(edx) + 2u * U(v14) + 0x112A92u);
        ebx = edx / 300;
        ecx = 0x5A - (m_level() * 16 - m_level());
        if (ebx < ecx) {
            if (L24(R69(v18) + U(v14) + 0xE31C4u) >= v24)
                v10 = 1;
        } else if (L24(R69(v18) + U(v14) + 0xE31C4u) != 0)
            v10 = 1;
    }
    if (g_mut != 4)
        setD8(160u * U(v34) + U(v30) + 0xF0044u, v8);
    setD32(0x1079B4u, U(v1C));
    f7AC60();
    setD8(0x1082DCu, 0);
    return v10;
}

/* 0x49C88 */
int f49C88_model(int v2C, int v28)
{
    int32_t v10 = 0, v20 = v28 - 1, v14, v24, v1C, v18, edx;
    uint32_t vC;
    v14 = L24(R69(v2C + 1) + U(v20) + 0xE31C4u) - f78D08(v2C, v28);
    if (v14 <= 0)
        return v10;
    vC = D32(4u * U(v2C) + 0xF8E7Cu);
    for (v24 = 1; lw(vC) >= v24 && v14 > 0; v24++) {
        v1C = lw(vC + 2u * U(v24));
        v18 = v1C + 1;
        if ((D8(314u * U(v20) + 2u * U(v1C) + 0xF3580u) & 0x20) == 0)
            continue;
        if (D8(314u * U(v20) + 2u * U(v1C) + 0xF3581u) & 2)
            continue;
        if (LIM(v1C) - L24(R69(v18) + U(v20) + 0xE31C4u) != 1)
            continue;
        f57CA4(v2C, v1C, v28, 1, 1);
        edx = (int32_t)D32(0x1079B4u);
        setD8(4u * U(edx) + 0x107694u, (uint8_t)v2C);
        setD8(4u * U(edx) + 0x107695u, (uint8_t)v1C);
        setD8(4u * U(edx) + 0x107697u, 0x17);
        setD32(0x1079B4u, U(edx + 1));
        setD8(4u * U(edx) + 0x107696u, 1);
        v14--;
        v10++;
    }
    return v10;
}

/* 0x49E04 */
void m49E04(int v20)
{
    uint32_t v18 = 0xF14CCu + 199u * U(v20);
    int32_t v40 = v20 - 1, v48, eax, v38, v3C, v28, v2C, v34, v1C, lst[160], v44, v30, v24, edx;
    for (v48 = 0; (eax = L24(v18 + 0x5Du)) > v48; v48++)
        setD32(4u * U(v48) + 0xF7CA0u, D8(v18 + U(v48) + 0x61u));
    setD32(0xF7C9Cu, U(eax));
    for (v48 = 0; v48 < (int32_t)D32(0xF7C9Cu); v48++) {
        v38 = (int32_t)D32(4u * U(v48) + 0xF7CA0u);
        v3C = v38 - 1;
        if (L24(R69(v38) + 0xE31B6u) != 1)
            continue;
        if (L24(R69(v38) + U(v40) + 0xE31B7u) != v20)
            continue;
        if (L24(R69(v38) + U(v40) + 0xE31DAu) <= 0)
            continue;
        if (D8(3u * U(v3C) + 0xE2C1Au) & 2)
            continue;
        v28 = f78D08(v3C, v20);
        v2C = v28 >= 3 ? 2 : v28;
        v34 = L24(R69(v38) + U(v40) + 0xE31C4u) - v2C;
        if (v34 <= 0)
            continue;
        aiplan_test_f48028(&v1C, lst, v3C, v20, 0x13);
        aiplan_test_f48D44(&v1C, lst, v20, v3C, 0x12);
        if (v1C <= 0)
            continue;
        v44 = 0;
        v30 = lst[v44];
        v24 = f78D08(v30, v20);
        while (v44 < v1C && v34 > 0) {
            while (v34 > 0 && L24(R69(v38) + U(v40) + 0xE31DAu) > 0 && L24(R69(v30 + 1) + U(v40) + 0xE31C4u) < v24) {
                f57CA4(v3C, v30, v20, 1, 1);
                edx = (int32_t)D32(0x1079B4u);
                setD8(4u * U(edx) + 0x107694u, (uint8_t)v3C);
                setD8(4u * U(edx) + 0x107695u, (uint8_t)v30);
                setD8(4u * U(edx) + 0x107697u, 0x18);
                setD32(0x1079B4u, U(edx + 1));
                setD8(4u * U(edx) + 0x107696u, 1);
                v34--;
            }
            v44++;
            if (g_mut == 8 && v44 < v1C) {   /* the "fixed" reading: reload the target */
                v30 = lst[v44];
                v24 = f78D08(v30, v20);
            }
        }
    }
}

/* 0x4A088 */
void m4A088(int v4C, int v40, int v48)
{
    uint32_t v10 = 0xF14CCu + 199u * U(v4C);
    int32_t v3C = v4C - 1, v14 = 0, v2C, lst[160], v20, v24, v44, v30, v38 = 0, v28, v34, v1C, v18, esi;
    for (v2C = 0; L24(v10 + 0x5Du) > v2C; v2C++)
        lst[v2C] = D8(v10 + U(v2C) + 0x61u);
    v20 = L24(v10 + 0x5Du);
    for (v2C = 0; v2C < v20; v2C++) {
        v24 = lst[v2C];
        v44 = v24 - 1;
        v30 = f78D08(v44, v4C);
        if ((D8(3u * U(v44) + 0xE2C1Au) & 2) && (int32_t)D8(3u * U(v44) + 0xE2C18u) == v4C)
            continue;
        if (L24(R69(v24) + U(v3C) + 0xE31B7u) != v4C)
            continue;
        if (L24(R69(v24) + 0xE31B6u) == 1 && L24(R69(v24) + U(v3C) + 0xE31C4u) > v30)
            goto L_4A270;
        if (L24(R69(v24) + 0xE31B6u) < 1)
            continue;
        esi = L24(R69(v24) + U(v3C) + 0xE31C4u);
        v38 = aiplan_test_f7C838(v44, v4C, 0, &v28);
        if (esi != v38)
            continue;
L_4A270:
        if (L24(R69(v24) + U(v3C) + 0xE31DAu) <= 0)
            continue;
        if (L24(R69(v24) + 0xE31B6u) == 1)
            v34 = L24(R69(v24) + U(v3C) + 0xE31C4u) - v30 + (g_mut == 9 ? 1 : 0);
        else
            v34 = L24(R69(v24) + U(v3C) + 0xE31DAu) == v38 ? v38 : 0;
        if (v34 <= 0)
            continue;
        v1C = v44;
        v18 = 1;
        m4A2E0(v34, v44, v4C, v14, &v1C, v18, v40, v48);
    }
}

/* 0x4A2E0 (ret 0x10) */
int m4A2E0(int v58, int v5C, int v54, int v50, const int32_t *p8, int pC, int p10, int p14)
{
    int32_t v4C, v2C, v24, v40, v18, v38, v3C, v14, lst[160], v34, v30, v48, v44, v10, v20, v1C, v28, ebx, edx;
    uint32_t vC;
    if (v50 >= (int32_t)D32(0xC28B0u))
        return 0;
    v50++;
    v4C = v54 - 1;
    v2C = 0;
    vC = D32(4u * U(v5C) + 0xF8E7Cu);
    v24 = 0;
    for (v40 = 1; lw(vC) >= v40; v40++) {
        v18 = lw(vC + 2u * U(v40));
        v38 = 1;
        for (v3C = 0; v3C < pC; v3C++)
            if (v18 == p8[v3C])
                v38 = 0;
        if (v38 == 0)
            continue;
        v14 = v18 + 1;
        if (L24(R69(v14) + 0xE31B6u) == 1 && L24(R69(v14) + U(v4C) + 0xE31B7u) == v54 &&
            L24(R69(v14) + U(v4C) + 0xE31C4u) < f78D08(v18, v54))
            goto L_4A409;
        if (L24(R69(v14) + 0xE31B6u) != 0)
            continue;
        if (D8(3u * U(v18) + 0xE2C1Au) & 1)
            continue;
L_4A409:
        lst[v24++] = v18;
    }
    aiplan_test_f48D44(&v24, lst, v54, v5C, 6);
    for (v40 = 0; v40 < v24 && v58 > 0; v40++) {
        v34 = lst[v40];
        v30 = v58;
        v48 = f78D08(v34, v54);
        if (L24(R69(v34 + 1) + U(v4C) + 0xE31C4u) + v30 > v48)
            v30 = v48 - L24(R69(v34 + 1) + U(v4C) + 0xE31C4u);
        while (v58 > 0 && v30 > 0) {
            f57CA4(v5C, v34, v54, 1, 1);
            edx = (int32_t)D32(0x1079B4u);
            setD8(4u * U(edx) + 0x107694u, (uint8_t)v5C);
            setD8(4u * U(edx) + 0x107695u, (uint8_t)v34);
            setD8(4u * U(edx) + 0x107697u, (uint8_t)(v50 + (g_mut == 5 ? 7 : 6)));
            setD32(0x1079B4u, U(edx + 1));
            setD8(4u * U(edx + 1) + 0x107692u, 1);
            v58--;
            v30--;
            v2C++;
        }
    }
    if (p10 != 1 && v58 > 0 && (D8(35u * U(v5C) + 0xE1638u) & 2))
        v2C += aiplan_test_f4AC50(v5C, v54, &v58, v50);
    if (v58 > 0 && p14 == 1) {
        vC = D32(4u * U(v5C) + 0xF8E7Cu);
        v24 = 0;
        for (v40 = 1; lw(vC) >= v40; v40++) {
            v18 = lw(vC + 2u * U(v40));
            v38 = 1;
            for (v3C = 0; v3C < pC; v3C++)
                if (v18 == p8[v3C])
                    v38 = 0;
            if (v38 == 0)
                continue;
            v14 = v18 + 1;
            if (L24(R69(v14) + 0xE31B6u) != 1)
                continue;
            if (L24(R69(v14) + U(v4C) + 0xE31B7u) == v54)
                continue;
            ebx = L24(R69(v14) + U(f7C760(v18)) + 0xE31C3u);
            if (ebx >= LIM(v18))
                continue;
            lst[v24++] = v18;
        }
        aiplan_test_f48D44(&v24, lst, v54, v5C, 6);
        for (v40 = 0; v40 < v24 && v58 > 0; v40++) {
            v34 = lst[v40];
            v44 = f7C760(v34);
            v30 = v58;
            if (v30 + L24(R69(v34 + 1) + U(v44) + 0xE31C3u) > LIM(v34))
                v30 = LIM(v34) - L24(R69(v34 + 1) + U(v44) + 0xE31C3u);
            while (v58 > 0 && v30 > 0) {
                f57CA4(v5C, v34, v54, 1, 1);
                edx = (int32_t)D32(0x1079B4u);
                setD8(4u * U(edx) + 0x107694u, (uint8_t)v5C);
                setD8(4u * U(edx) + 0x107695u, (uint8_t)v34);
                setD8(4u * U(edx) + 0x107697u, 0x22);
                setD8(4u * U(edx) + 0x107696u, 1);
                setD32(0x1079B4u, U(edx + 1));
                v58--;
                v30--;
                v2C++;
            }
        }
    }
    if (v58 > 0) {
        vC = D32(4u * U(v5C) + 0xF8E7Cu);
        v24 = 0;
        for (v40 = 1; lw(vC) >= v40; v40++) {
            v18 = lw(vC + 2u * U(v40));
            v38 = 1;
            for (v3C = 0; v3C < pC; v3C++)
                if (v18 == p8[v3C])
                    v38 = 0;
            if (v38 == 0)
                continue;
            v14 = v18 + 1;
            if (L24(R69(v14) + 0xE31B6u) == 1 && L24(R69(v14) + U(v4C) + 0xE31B7u) == v54 &&
                L24(R69(v14) + U(v4C) + 0xE31DAu) > 0) {
                ebx = L24(R69(v14) + U(v4C) + 0xE31C4u) - f78D08(v18, v54);
                if (L24(R69(v14) + U(v4C) + 0xE31DAu) > ebx)
                    goto L_4A989;
            }
            if (L24(R69(v14) + 0xE31B6u) <= 1)
                continue;
            if (L24(R69(v14) + U(v4C) + 0xE31B7u) != v54)
                continue;
            if (L24(R69(v14) + U(v4C) + 0xE31DAu) <= 0)
                continue;
L_4A989:
            lst[v24++] = v18;
        }
        aiplan_test_f48D44(&v24, lst, v54, v5C, 6);
        v10 = v24;
        for (v40 = 0; v40 < pC; v40++)
            lst[v24++] = p8[v40];
        for (v40 = 0; v40 < v10 && v58 > 0; v40++) {
            v20 = lst[v40];
            v1C = v20 + 1;
            v48 = f78D08(v20, v54);
            v28 = L24(R69(v1C) + U(v4C) + 0xE31DAu) > v58 ? v58 : L24(R69(v1C) + U(v4C) + 0xE31DAu);
            if (v28 > v48)
                v28 = v48;
            v30 = m4A2E0(v28, v20, v54, v50, lst, v24, p10, p14);
            if (v30 + L24(R69(v1C) + U(v4C) + 0xE31C4u) > LIM(v20)) {
                ebx = L16(R69(v1C) + 0xE31EDu);
                if (ebx != 0)
                    v30 = L16(R69(v1C) + 0xE31EDu) - L24(R69(v1C) + U(v4C) + 0xE31C4u);
                else if (L24(R69(v1C) + 0xE31B6u) <= 1)
                    v30 = LIM(v20) - L24(R69(v1C) + U(v4C) + 0xE31C4u);
            }
            while (v58 > 0 && v30 > 0) {
                f57CA4(v5C, v20, v54, 1, 1);
                edx = (int32_t)D32(0x1079B4u);
                setD8(4u * U(edx) + 0x107694u, (uint8_t)v5C);
                setD8(4u * U(edx) + 0x107695u, (uint8_t)v20);
                setD8(4u * U(edx) + 0x107697u, (uint8_t)(v50 + 6));
                setD8(4u * U(edx) + 0x107696u, 1);
                setD32(0x1079B4u, U(edx + 1));
                v58--;
                v30--;
                v2C++;
            }
        }
    }
    return v2C;
}

/* 0x4BBDC */
void m4BBDC(int v44)
{
    uint32_t v18 = 0xF14CCu + 199u * U(v44);
    int32_t v38 = v44 - 1, v2C = 0, v48, v20, v4C, v34, l7[0x140], l2[160], v24, v40, v28, v3C, edx, esi;
    for (v48 = 0; L24(v18 + 0x5Du) > v48; v48++) {
        v20 = D8(v18 + U(v48) + 0x61u);
        v4C = v20 - 1;
        (void)f7C7C4(v4C, v44);
        if (L24(R69(v20) + 0xE31B6u) <= 1)
            continue;
        if (L24(R69(v20) + U(v38) + 0xE31B7u) != v44)
            continue;
        esi = L24(R69(v20) + U(v38) + 0xE31C4u);
        if (aiplan_test_f7C838(v4C, v44, 0, &v34) != esi)
            continue;
        if (L24(R69(v20) + U(v38) + 0xE31DAu) <= 0)
            continue;
        if (L24(R69(v20) + 0xE31F0u) != 0)
            continue;
        l7[v2C++] = v4C;
    }
    for (v48 = 0; v48 < v2C; v48++) {
        aiplan_test_f48028(&v24, l2, l7[v48], v44, 0x17);
        v40 = L24(R69(l7[v48] + 1) + U(v38) + 0xE31DAu);
        if (v24 <= 0 || v40 <= 0)
            continue;
        edx = (int32_t)D32(0x1079B4u);
        setD8(4u * U(edx) + 0x107694u, (uint8_t)l7[v48]);
        setD8(4u * U(edx) + 0x107695u, (uint8_t)l2[0]);
        setD8(4u * U(edx) + 0x107697u, 0x29);
        setD32(0x1079B4u, U(edx + 1));
        setD8(4u * U(edx) + 0x107696u, (uint8_t)v40);
        f57CA4(l7[v48], l2[0], v44, 1, v40);
        l7[v48] = -1;
    }
    for (v48 = 0; v48 < v2C; v48++) {
        if (l7[v48] == -1)
            continue;
        v4C = l7[v48];
        v20 = v4C + 1;
        aiplan_test_f48028(&v24, l2, v4C, v44, 0xC);
        v40 = L24(R69(v20) + U(v38) + 0xE31DAu);
        if (v24 <= 0 || v40 <= 0)
            continue;
        for (v28 = 0; v28 < v24; v28++) {
            if (L24(R69(l2[v28] + 1) + 0xE31B6u) != (g_mut == 6 ? 1 : 2))
                continue;
            v3C = aiplan_test_f4CC5C(l2[v28], v44, 0);
            while (v40 > 0 && v3C > 0) {
                edx = (int32_t)D32(0x1079B4u);
                setD8(4u * U(edx) + 0x107694u, (uint8_t)v4C);
                setD8(4u * U(edx) + 0x107695u, (uint8_t)l2[v28]);
                setD8(4u * U(edx) + 0x107697u, 0x29);
                setD32(0x1079B4u, U(edx + 1));
                setD8(4u * U(edx) + 0x107696u, 1);
                f57CA4(v4C, l2[v28], v44, 1, 1);
                v40--;
                v3C--;
            }
            l7[v48] = -1;
        }
    }
    for (v48 = 0; v48 < v2C; v48++) {
        if (l7[v48] == -1)
            continue;
        v4C = l7[v48];
        v20 = v4C + 1;
        aiplan_test_f48028(&v24, l2, v4C, v44, 0x18);
        v40 = L24(R69(v20) + U(v38) + 0xE31DAu);
        if (v24 <= 0 || v40 <= 0)
            continue;
        for (v28 = 0; v28 < v24; v28++) {
            while (v40 > 0) {
                edx = (int32_t)D32(0x1079B4u);
                setD8(4u * U(edx) + 0x107694u, (uint8_t)v4C);
                setD8(4u * U(edx) + 0x107695u, (uint8_t)l2[v28]);
                setD8(4u * U(edx) + 0x107697u, 0x29);
                setD32(0x1079B4u, U(edx + 1));
                setD8(4u * U(edx) + 0x107696u, 1);
                f57CA4(v4C, l2[v28], v44, 1, 1);
                v40--;
            }
            l7[v48] = -1;
        }
    }
}

/* 0x4C074 */
void m4C074(int v68)
{
    uint32_t v18 = 0xF14CCu + 199u * U(v68), v1C;
    int32_t v6C = v68 - 1, v2C, v24, eax, v5C, v40, v44, v64, v3C, lst[160], v48, v34, v38, v54, v50, v28, v60, v4C,
            v58, v20, ecx, edx, ebx, own[160], ownc = 0, ok;
    v2C = L24(v18 + 0x23u);
    for (v24 = 0; (eax = L24(v18 + 0x5Du)) > v24; v24++)
        setD32(4u * U(v24) + 0xF7CA0u, D8(v18 + U(v24) + 0x61u));
    setD32(0xF7C9Cu, U(eax));
    if (g_mut == 7) {   /* the "fixed" reading: walk a private copy of n's list */
        ownc = eax;
        for (v24 = 0; v24 < ownc; v24++)
            own[v24] = (int32_t)D32(4u * U(v24) + 0xF7CA0u);
    }
    for (v24 = 0; v24 < (g_mut == 7 ? ownc : (int32_t)D32(0xF7C9Cu)); v24++) {
        v5C = g_mut == 7 ? own[v24] : (int32_t)D32(4u * U(v24) + 0xF7CA0u);
        v40 = v5C - 1;
        v44 = f78D08(v40, v68);
        if (D8(3u * U(v40) + 0xE2C1Au) & 2)
            continue;
        if (D8(3u * U(v40) + 0xE2C1Au) & 1)
            continue;
        if (L24(R69(v5C) + 0xE31B6u) != 1)
            continue;
        if (L24(R69(v5C) + U(v6C) + 0xE31B7u) != v68)
            continue;
        if (L24(R69(v5C) + U(v6C) + 0xE31DAu) <= 0)
            continue;
        if (L24(R69(v5C) + U(v6C) + 0xE31C4u) <= v44)
            continue;
        v64 = L24(R69(v5C) + U(v6C) + 0xE31C4u) - v44;
        v1C = D32(4u * U(v40) + 0xF8E7Cu);
        v3C = 0;
        for (v48 = 1; lw(v1C) >= v48; v48++) {
            v34 = lw(v1C + 2u * U(v48));
            if (L24(R69(v34 + 1) + 0xE31B6u) != 1)
                continue;
            if (L24(R69(v34 + 1) + U(v6C) + 0xE31B7u) != 0)
                continue;
            lst[v3C++] = v34;
        }
        if (v3C == 0)
            continue;
        for (v38 = 0; v38 < v3C && v64 > 0; v38++) {
            v54 = lst[v38];
            v50 = f7C760(v54);
            v28 = L24(199u * U(v50) + 0xF14EFu) - v2C;
            v60 = L24(R69(v54 + 1) + U(v50) + 0xE31C3u);
            v1C = D32(4u * U(v54) + 0xF8E7Cu);
            setD32(0xF7C9Cu, 0);
            for (v48 = 1; lw(v1C) >= v48; v48++) {
                v34 = lw(v1C + 2u * U(v48));
                if (L24(R69(v34 + 1) + 0xE31B6u) != 1)
                    continue;
                if (D8(3u * U(v34) + 0xE2C1Au) & 2)
                    continue;
                if (L24(R69(v34 + 1) + U(v6C) + 0xE31B7u) != v68)
                    continue;
                ebx = L24(R69(v34 + 1) + U(v6C) + 0xE31C4u);
                if (ebx <= f78D08(v34, v68))
                    continue;
                eax = (int32_t)D32(0xF7C9Cu);
                setD32(0xF7C9Cu, U(eax + 1));
                setD32(4u * U(eax) + 0xF7CA0u, U(v34));
            }
            if ((int32_t)D32(0xF7C9Cu) < 1)
                continue;
            v4C = 0;
            for (v48 = 0; v48 < (int32_t)D32(0xF7C9Cu); v48++) {
                eax = (int32_t)D32(4u * U(v48) + 0xF7CA0u);
                ebx = L24(R69(eax + 1) + U(v6C) + 0xE31C4u);
                v4C += ebx - f78D08(eax, v68);
            }
            (void)aiplan_test_f7C838(v54, v68, v4C, &v58);
            ok = 0;
            if (L24(U(v50) + 0xF5479u) + L24(U(v68) + 0xF5479u) > 0 &&
                (g_mut == 13 ? D8(U(v50) + 0xF547Cu) >= D8(U(v68) + 0xF547Cu)
                             : (int8_t)D8(U(v50) + 0xF547Cu) >= (int8_t)D8(U(v68) + 0xF547Cu)))
                ok = 1;
            if (!ok && LIM(v54) != 1)
                continue;
            edx = L24(199u * U(v68) + 0xF1564u);
            ecx = 100 - 10 * m_level() + 10 * edx;
            edx = v54 % 4 + 3;
            edx = L16(20u * U(edx) + 2u * U(v6C) + 0x112A92u);
            if (edx / 300 >= ecx)
                continue;
            v20 = -999;
            if (LIM(v54) == 1) {
                if (!(L24(U(v50) + 0xF5479u) + L24(U(v68) + 0xF5479u) > 0 &&
                      (int8_t)D8(U(v50) + 0xF547Cu) >= (int8_t)D8(U(v68) + 0xF547Cu))) {
                    if ((int8_t)D8(U(v68) + 0xF06C0u) > (int8_t)D8(U(v50) + 0xF06C0u))
                        continue;
                    v20 = 1;
                }
            }
            if (v20 == -999)
                v20 = v60 + v28 + ((v60 + v28) < v4C);
            for (v48 = 0; v48 < (int32_t)D32(0xF7C9Cu); v48++) {
                while (v20 > 0) {
                    eax = (int32_t)D32(4u * U(v48) + 0xF7CA0u);
                    if (L24(R69(eax + 1) + U(v6C) + 0xE31DAu) <= 0)
                        break;
                    if (L24(R69(eax + 1) + U(v6C) + 0xE31C4u) <= f78D08(eax, v68))
                        break;
                    f57CA4(eax, v54, v68, 1, 1);
                    v20--;
                    v64--;
                    edx = (int32_t)D32(0x1079B4u);
                    setD8(4u * U(edx) + 0x107694u, (uint8_t)D32(4u * U(v48) + 0xF7CA0u));
                    setD8(4u * U(edx) + 0x107695u, (uint8_t)v54);
                    setD8(4u * U(edx) + 0x107697u, 0x11);
                    setD32(0x1079B4u, U(edx + 1));
                    setD8(4u * U(edx + 1) + 0x107692u, 1);
                }
            }
        }
    }
}

/* 0x4C6E4 */
void m4C6E4(int v44)
{
    int32_t v30 = v44 - 1, v40, v2C, v28, v20, v24, v18, lst[160], v3C, v1C, v34, edx;
    for (v40 = 0; v40 < (g_mut == 10 ? 0x9C : 0x9D); v40++) {
        if ((D8(314u * U(v30) + 2u * U(v40) + 0xF3581u) & 2) == 0)
            continue;
        v2C = 1;
        v28 = aiplan_test_f7C838(v40, v44, 0, &v20);
        if (L24(R69(v40 + 1) + U(v30) + 0xE31C4u) <= v28)
            v2C = 0;
        if (v2C != 0)
            continue;
        v24 = aiplan_test_f7C838(v40, v44, 1, &v20);
        if (v24 > v28)
            continue;
        if (L24(R69(v40 + 1) + U(v30) + 0xE31C4u) + 1 < v24)
            continue;
        if ((D8(35u * U(v40) + 0xE1639u) & 0x40) == 0 && (D8(160u * U(v44) + U(v40) + 0xF0044u) & 2) == 0)
            continue;
        aiplan_test_f48028(&v18, lst, v40, v44, 0xA);
        aiplan_test_f48D44(&v18, lst, v44, v40, 1);
        v34 = 0;
        for (v3C = 0; v3C < v18; v3C++) {
            v1C = lst[v3C];
            if (L24(R69(v1C + 1) + U(v30) + 0xE31C4u) <= f78D08(v1C, v44))
                continue;
            f57CA4(v1C, v40, v44, 1, 1);
            edx = (int32_t)D32(0x1079B4u);
            setD8(4u * U(edx) + 0x107694u, (uint8_t)v1C);
            setD8(4u * U(edx) + 0x107695u, (uint8_t)v40);
            setD8(4u * U(edx) + 0x107697u, 0x1A);
            setD32(0x1079B4u, U(edx + 1));
            setD8(4u * U(edx + 1) + 0x107692u, 1);
            v34 = 1;
            break;
        }
        if (v34 != 0)
            continue;
        aiplan_test_f48028(&v18, lst, v40, v44, 0x12);
        aiplan_test_f48D44(&v18, lst, v44, v40, 1);
        for (v3C = 0; v3C < v18; v3C++) {
            v1C = lst[v3C];
            if (L24(R69(v1C + 1) + U(v30) + 0xE31DAu) <= 0)
                continue;
            f57CA4(v1C, v40, v44, 1, 1);
            edx = (int32_t)D32(0x1079B4u);
            setD8(4u * U(edx) + 0x107694u, (uint8_t)v1C);
            setD8(4u * U(edx) + 0x107695u, (uint8_t)v40);
            setD8(4u * U(edx) + 0x107697u, 0x1A);
            setD32(0x1079B4u, U(edx + 1));
            setD8(4u * U(edx) + 0x107696u, 1);
            break;
        }
    }
}

/* 0x4C9B8 */
void m4C9B8(int v28, int v2C)
{
    int32_t v20 = 0, v24 = v28 - 1, v18, v1C, lst[160], v14, v10, t;
    if (L24(199u * U(v28) + 0xF1561u) < 0xE)
        return;
    v18 = 0;
    for (v1C = 1; v1C <= 0x9D; v1C++)
        if (L24(R69(v1C) + U(v24) + 0xE31F1u) == v2C)
            lst[v18++] = v1C - 1;
    if (v18 == 0)
        return;
    aiplan_test_f48D44(&v18, lst, v28, v20, 5);
    for (v1C = 0; v1C < v18; v1C++) {
        t = lst[v1C];
        v14 = -1;
        if (D32(0x107D40u) == 0 && (D8(3u * U(t) + 0xE2C1Au) & 1) && L24(R69(t + 1) + 0xE31B6u) == 0) {
            v14 = aiplan_test_f4CEE4(t, v28, &v10);
            setD32(0x107D40u, 1);
        } else if ((D8(3u * U(t) + 0xE2C1Au) & 1) == 0) {
            if (L24(R69(t + 1) + 0xE31B6u) <= 1)
                v14 = aiplan_test_f4CC5C(t, v28, 4);
            else if (L24(R69(t + 1) + U(v24) + 0xE31B7u) == v28)
                v14 = aiplan_test_f4CC5C(t, v28, 4);
        }
        if (v14 == -1)
            continue;
        f7A940();
        f7AA1C();
        if (aiplan_test_f3580C(v14, t, v28, 1, v14) != 0) {
            if (g_mut == 15) {
                f7AE14();
                f7AC60();
            } else {
                f7AC60();
                f7AE14();
            }
            setD32(0x107D40u, 0);
        } else {
            f7AD24();
            f7AD7C();
            setD8(R69(t + 1) + 0xE31F3u, D8(R69(t + 1) + U(v24) + 0xE31C7u));
        }
    }
}

/* ---- the harness ------------------------------------------------------------------------------------------------ */

enum { F46BB8, F46CBC, F471CC, F474A0, F478D8, F47A44, F49A1C, F49C88, F49E04, F4A088, F4A2E0, F4BBDC, F4C074,
       F4C6E4, F4C9B8, NFN };
static const char *g_fname[NFN] = { "46BB8", "46CBC", "471CC", "474A0", "478D8", "47A44", "49A1C", "49C88", "49E04",
                                    "4A088", "4A2E0", "4BBDC", "4C074", "4C6E4", "4C9B8" };

typedef struct {
    int fn;
    int a[8];
    int32_t list[160];
    int32_t cnt;
    int32_t slot38;
} Call;

typedef struct {
    int ret, cut, stack_err;
    int32_t o1, o2, cnt, slot38;
    int32_t list[160];
} Out;

static uint8_t *g_start, *g_port_ds;

static void run(const Call *c, int model, Out *o)
{
    int32_t a1, a2, cnt;
    int32_t list[160];
    memset(o, 0, sizeof *o);
    memcpy(list, c->list, sizeof list);
    cnt = c->cnt;
    a1 = c->a[2];
    a2 = c->a[3];
    g_calls = 0;
    g_stack_err = 0;
    g_cut_on = 1;
    if (setjmp(g_cut) != 0) {
        o->cut = 1;
        tr("CUT");
        g_cut_on = 0;
        return;
    }
    ms_slot38 = c->slot38;
    *aiplan_test_slot38() = c->slot38;
    switch (c->fn) {
    case F46BB8: model ? m46BB8(c->a[0]) : f46BB8(c->a[0]); break;
    case F46CBC: o->ret = model ? m46CBC(c->a[0], c->a[1]) : f46CBC(c->a[0], c->a[1]); break;
    case F471CC: model ? m471CC(&cnt, list, c->a[0]) : f471CC(&cnt, list, c->a[0]); break;
    case F474A0:
        model ? m474A0(c->a[0], c->a[1], &a1, &a2, cnt, list) : f474A0(c->a[0], c->a[1], &a1, &a2, cnt, list);
        break;
    case F478D8: model ? m478D8(c->a[0], &a1, c->a[1], &a2) : f478D8(c->a[0], &a1, c->a[1], &a2); break;
    case F47A44: model ? m47A44(c->a[0], &a1, c->a[1], &a2) : f47A44(c->a[0], &a1, c->a[1], &a2); break;
    case F49A1C:
        o->ret = model ? m49A1C(c->a[0], c->a[1], c->a[4], c->a[5]) : f49A1C(c->a[0], c->a[1], c->a[4], c->a[5]);
        break;
    case F49C88: o->ret = model ? f49C88_model(c->a[0], c->a[1]) : f49C88(c->a[0], c->a[1]); break;
    case F49E04: model ? m49E04(c->a[0]) : f49E04(c->a[0]); break;
    case F4A088: model ? m4A088(c->a[0], c->a[1], c->a[4]) : f4A088(c->a[0], c->a[1], c->a[4]); break;
    case F4A2E0:
        o->ret = model ? m4A2E0(c->a[0], c->a[1], c->a[4], c->a[5], list, cnt, c->a[6], c->a[7])
                       : f4A2E0(c->a[0], c->a[1], c->a[4], c->a[5], list, cnt, c->a[6], c->a[7]);
        break;
    case F4BBDC: model ? m4BBDC(c->a[0]) : f4BBDC(c->a[0]); break;
    case F4C074: model ? m4C074(c->a[0]) : f4C074(c->a[0]); break;
    case F4C6E4: model ? m4C6E4(c->a[0]) : f4C6E4(c->a[0]); break;
    case F4C9B8: model ? m4C9B8(c->a[0], c->a[1]) : f4C9B8(c->a[0], c->a[1]); break;
    default: break;
    }
    g_cut_on = 0;
    o->o1 = a1;
    o->o2 = a2;
    o->cnt = cnt;
    memcpy(o->list, list, sizeof list);
    o->stack_err = g_stack_err;
    o->slot38 = model ? ms_slot38 : *aiplan_test_slot38();
}

/* one differential check from the current g_ds; the state is left as the port left it; returns 1 if equal */
static int compare_one(const Call *c, const char *tag)
{
    Out op, om;
    char buf[300];
    size_t i;
    int ok = 1;

    memcpy(g_start, g_ds, DS_SIZE);
    g_cur = 0;
    g_len[0] = 0;
    g_log[0][0] = 0;
    run(c, 0, &op);
    memcpy(g_port_ds, g_ds, DS_SIZE);
    memcpy(g_ds, g_start, DS_SIZE);
    g_cur = 1;
    g_len[1] = 0;
    g_log[1][0] = 0;
    run(c, 1, &om);
    g_cur = 0;

    if (op.ret != om.ret || op.o1 != om.o1 || op.o2 != om.o2 || op.cnt != om.cnt || op.cut != om.cut ||
        op.slot38 != om.slot38 ||
        memcmp(op.list, om.list, sizeof op.list) != 0) {
        ok = 0;
        snprintf(buf, sizeof buf, "%s %s(%d %d ..): outputs port %d %d %d %d cut%d / model %d %d %d %d cut%d", tag,
                 g_fname[c->fn], c->a[0], c->a[1], op.ret, (int)op.o1, (int)op.o2, (int)op.cnt, op.cut, om.ret,
                 (int)om.o1, (int)om.o2, (int)om.cnt, om.cut);
        if (g_verbose && !g_mut)
            printf("  %s\n", buf);
    }
    if (strcmp(g_log[0], g_log[1]) != 0) {
        const char *p = g_log[0], *q = g_log[1];
        int line = 1;
        ok = 0;
        while (*p && *p == *q) {
            if (*p == '\n')
                line++;
            p++;
            q++;
        }
        if (g_verbose && !g_mut) {
            const char *pe = strchr(p, '\n'), *qe = strchr(q, '\n');
            printf("  %s %s: trace differs at line %d: port \"%.*s\" model \"%.*s\"\n", tag, g_fname[c->fn], line,
                   (int)(pe ? pe - p : 60), p, (int)(qe ? qe - q : 60), q);
        }
    }
    for (i = 0; i < DS_SIZE; i++)
        if (g_port_ds[i] != g_ds[i]) {
            ok = 0;
            if (g_verbose && !g_mut)
                printf("  %s %s(%d %d): data differ at 0x%X: port %02X model %02X\n", tag, g_fname[c->fn], c->a[0],
                       c->a[1], (unsigned)(i + DS_BASE), g_port_ds[i], g_ds[i]);
            break;
        }
    if (op.stack_err || om.stack_err) {
        ok = 0;
        if (g_verbose && !g_mut)
            printf("  %s %s: stack error\n", tag, g_fname[c->fn]);
    }
    memcpy(g_ds, g_port_ds, DS_SIZE);
    return ok;
}

/* ---- random states ------------------------------------------------------------------------------------------------ */

static uint32_t g_rs = 20261121u;
static uint32_t rnd(uint32_t n)
{
    g_rs = g_rs * 1103515245u + 12345u;
    return n ? (g_rs >> 8) % n : 0;
}

static uint8_t *g_base[2];
static int g_nbase;

static int nation_list_has(int n, int t)
{
    int i, k = (int8_t)D8(T_NAT(n) + 0x60u);
    for (i = 0; i < k; i++)
        if (D8(T_NAT(n) + 0x61u + (uint32_t)i) == t + 1)
            return 1;
    return 0;
}

static void random_state(void)
{
    int n, t, i, k;
    memcpy(g_ds, g_base[rnd((uint32_t)g_nbase)], DS_SIZE);
    /* tokens moved about by the real primitives */
    k = (int)rnd(120);
    for (i = 0; i < k; i++) {
        n = 1 + (int)rnd(9);
        t = (int)rnd(157);
        if (rnd(3) == 0 && (int8_t)D8(T_NAT(n) + 0x60u) < 60)
            f57E28(t, n, 1 + (int)rnd(4));
        else if (nation_list_has(n, t) && rnd(2) == 0)
            f57CE8(t, n, 1, 1);
    }
    for (n = 1; n <= 9; n++) {
        if (rnd(3) == 0)
            f460C0(n);   /* movable = tokens */
        else
            for (t = 0; t < 160; t++) {
                int8_t tk = (int8_t)D8(T_AREA(t + 1) + 0x0Eu + (uint32_t)n);
                setD8(T_AREA(t + 1) + 0x24u + (uint32_t)n, (uint8_t)(tk > 0 ? (int)rnd((uint32_t)tk + 1) : 0));
            }
        for (t = 0; t < 160; t++) {
            setD8(T_BPM(n, t), (uint8_t)(rnd(4) == 0 ? rnd(8) : 0));
            setD8(T_SC(n, t), (uint8_t)((int)rnd(160) - 30));
            if (t < 157) {
                setD8(T_FL(n, t), (uint8_t)(rnd(3) == 0 ? rnd(256) : 0));
                setD8(T_FL(n, t) + 1u, (uint8_t)(rnd(3) == 0 ? rnd(8) : 0));
            }
            setD8(T_AREA(t + 1) + 0x3Bu + (uint32_t)n, (uint8_t)(rnd(6) == 0 ? rnd(3) : 0));
        }
        setD8(T_NAT(n) + 0x26u, (uint8_t)rnd(2));
        setD8(T_NAT(n) + 0x27u, (uint8_t)rnd(2));
        setD8(T_NAT(n) + 0x9Bu, (uint8_t)((int)rnd(5) - 2));
        setD8(T_NAT(n) + 0x98u, (uint8_t)(10 + rnd(46)));
        setD8(0xF06C0u + (uint32_t)n, (uint8_t)((int)rnd(12) - 1));
        setD8(0xF488Cu + (uint32_t)n, (uint8_t)rnd(2));
    }
    for (t = 0; t < 160; t++) {
        setD8(T_AREA(t + 1) + 0x3Bu, (uint8_t)(rnd(5) == 0 ? rnd(6) : 0));
        setD16(T_AREA(t + 1) + 0x37u, (uint16_t)(rnd(4) == 0 ? rnd(8) : 0));
        if (rnd(10) == 0)
            setD8(T_TS(t) + 2u, (uint8_t)(D8(T_TS(t) + 2u) ^ (1u << rnd(2))));
        setD8(0x10819Cu + (uint32_t)t, (uint8_t)(rnd(4) == 0));
        setD8(0x10823Cu + (uint32_t)t, (uint8_t)(rnd(4) == 0));
        setD32(0x107EE4u + 4u * (uint32_t)t, (uint32_t)(rnd(3) == 0));
    }
    for (i = 0; i < 300; i++)
        setD16(0x112A94u + 2u * (uint32_t)i, (uint16_t)rnd(30000));
    setD16(0xE1624u, (uint16_t)((D16(0xE1624u) & ~0x0C00u) | (uint16_t)(rnd(4) << 10)));
    setD8(0xC28ACu, (uint8_t)(rnd(4) != 0));
    setD32(0xC28B0u, 1u + rnd(4));
    setD32(0xC28B4u, (uint32_t)(rnd(5) != 0));
    setD8(0x1082DCu, (uint8_t)(rnd(10) == 0));
    setD32(0x108198u, rnd(1000));
    setD32(0x108194u, rnd(2));
    setD32(0x107D40u, rnd(2));
    setD32(0x1079B4u, rnd(40));
    setD32(0x107D3Cu, rnd(10));
    setD32(0xCAC54u, (uint32_t)-1);
    setD32(0xCAC58u, (uint32_t)-1);
}

static int random_nation(void)
{
    return 1 + (int)rnd(9);
}

static void random_call(Call *c)
{
    int n = random_nation(), i, k;
    memset(c, 0, sizeof *c);
    c->fn = (int)rnd(NFN);
    c->a[0] = n;
    {   /* the stale word 0x46CBC reads with an empty list: 0, n - 1, the list pointer, or a small leftover avail */
        static const int32_t pick[] = { 0, 0x00310000, -3, 1, 2, 5 };
        uint32_t w = rnd(7);
        c->slot38 = w == 6 ? n - 1 : pick[w];
    }
    k = (int8_t)D8(T_NAT(n) + 0x60u);
    switch (c->fn) {
    case F46CBC:
    case F49C88:
        c->a[1] = n;
        c->a[0] = k > 0 && rnd(4) ? D8(T_NAT(n) + 0x61u + rnd((uint32_t)k)) - 1 : (int)rnd(157);
        break;
    case F471CC:
        c->cnt = 0;
        for (i = 0; i < k && c->cnt < 160; i++)
            if (rnd(3))
                c->list[c->cnt++] = D8(T_NAT(n) + 0x61u + (uint32_t)i) - 1;
        break;
    case F474A0:
        c->a[1] = n;
        c->a[0] = k > 0 ? D8(T_NAT(n) + 0x61u + rnd((uint32_t)k)) - 1 : (int)rnd(157);
        c->a[2] = (int)rnd(8) - 2;
        c->a[3] = (int)rnd(2);
        c->cnt = (int)rnd(4);
        for (i = 0; i < c->cnt; i++)
            c->list[i] = (int)rnd(157);
        break;
    case F478D8:
    case F47A44:
        c->a[0] = (int)rnd(4);
        c->a[1] = n;
        c->a[2] = (int)rnd(5);
        c->a[3] = (int)rnd(2);
        break;
    case F49A1C:
        c->a[1] = n;
        c->a[0] = k > 0 && rnd(3) ? D8(T_NAT(n) + 0x61u + rnd((uint32_t)k)) - 1 : (int)rnd(157);
        c->a[4] = (int)rnd(5);
        c->a[5] = rnd(2) ? -1 : (int)rnd(157);
        break;
    case F4A088:
        c->a[1] = (int)rnd(2);
        c->a[4] = (int)rnd(2);
        break;
    case F4A2E0:
        c->a[4] = n;
        c->a[1] = k > 0 ? D8(T_NAT(n) + 0x61u + rnd((uint32_t)k)) - 1 : (int)rnd(157);
        c->a[0] = (int)rnd(8);
        c->a[5] = (int)rnd(3);
        c->a[6] = (int)rnd(2);
        c->a[7] = (int)rnd(2);
        c->cnt = 1 + (int)rnd(3);
        c->list[0] = c->a[1];
        for (i = 1; i < c->cnt; i++)
            c->list[i] = (int)rnd(157);
        break;
    case F4C9B8:
        c->a[1] = (int)rnd(3);
        break;
    default:
        break;
    }
}

/* ---- part A: hand vectors ------------------------------------------------------------------------------------------ */

static void clear_nation(int n)   /* all of n's tokens off the board (real primitives) */
{
    int t;
    for (t = 0; t < 160; t++) {
        int8_t tk = (int8_t)D8(T_AREA(t + 1) + 0x0Eu + (uint32_t)n);
        if (tk > 0)
            f57CE8(t, n, 1, tk);
    }
}

static int find_land(int want_n, int *u1, int *u2)   /* a territory t < 157 with two neighbours < 157, all empty */
{
    int t, i, j;
    for (t = 0; t < 157; t++) {
        if (t_cnt(t) != 0 || (D8(T_TS(t) + 2u) & 1))
            continue;
        for (i = 0; i < g_adjn[t]; i++)
            for (j = 0; j < g_adjn[t]; j++) {
                int a = g_adj[t][i], b = g_adj[t][j];
                if (i != j && a < 157 && b < 157 && t_cnt(a) == 0 && t_cnt(b) == 0 && (D8(T_TS(a) + 2u) & 1) == 0 &&
                    (D8(T_TS(b) + 2u) & 1) == 0 && want_n-- <= 0) {
                    *u1 = a;
                    *u2 = b;
                    return t;
                }
            }
    }
    return -1;
}

static void part_a(void)
{
    int n = 3, s, u1, u2, i, r;
    int32_t avail, ret, list[160];
    char buf[200];

    /* A1: 0x49C88 */
    memcpy(g_ds, g_base[0], DS_SIZE);
    clear_nation(n);
    s = find_land(0, &u1, &u2);
    check(s >= 0, "A1 a free territory");
    setD8(T_NAT(n) + 0x27u, 0);
    f57E28(s, n, 5);
    f57E28(u1, n, 1);
    f57E28(u2, n, 1);
    for (i = 0; i < g_adjn[s]; i++)
        setD8(T_FL(n, g_adj[s][i]), 0);
    setD8(T_FL(n, u1), 0x20);
    setD8(T_FL(n, u1) + 1u, 0);
    setD8(T_FL(n, u2), 0x20);
    setD8(T_FL(n, u2) + 1u, 2);   /* hi bit 0x02 excludes u2 */
    {
        int lim_s = f78D08(s, n), lim_u1 = (int)((uint8_t)(D8(T_TERR(u1) + 1u) << 2) >> 5);
        setD32(0x1079B4u, 3);
        g_calls = 0;
        r = f49C88(s, n);
        snprintf(buf, sizeof buf, "A1 0x49C88: s %d (limit %d) u1 %d (limit %d): moved %d", s, lim_s, u1, lim_u1, r);
        if (lim_u1 == 2 && lim_s < 5) {
            check(r == 1 && t_tok(u1, n) == 2 && t_tok(s, n) == 4 && D32(0x1079B4u) == 4 &&
                      D8(0x107694u + 12u) == s && D8(0x107695u + 12u) == u1 && D8(0x107696u + 12u) == 1 &&
                      D8(0x107697u + 12u) == 0x17,
                  buf);
        } else
            check(r == (lim_u1 == 2 ? 1 : 0), buf);
    }

    /* A2: 0x474A0's roll: counter 7 -> row 17; pass 1 type 0x13 */
    memcpy(g_ds, g_base[0], DS_SIZE);
    clear_nation(n);
    s = find_land(1, &u1, &u2);
    f57E28(s, n, 4);
    f460C0(n);
    setD8(T_FL(n, s), 0x20);
    setD8(0x10819Cu + (uint32_t)s, 0);
    setD32(0x108198u, 7);
    setD16(0xE1624u, (uint16_t)(D16(0xE1624u) & ~0x0C00u));      /* level 0: threshold 90 */
    setD16(T_TAB(17, n), 300 * 89);                                /* 89 < 90: the roll passes */
    setD8(T_FL(n, u1) + 1u, 2);
    setD8(T_FL(n, u1), 0);
    g_scr_n = 1;
    g_scr[0] = u1;
    g_scr_cc5c = 2;
    g_scr_c838 = 0;
    avail = 4;
    ret = 0;
    setD32(0x1079B4u, 0);
    setD32(0xC28B4u, 1);
    f474A0(s, n, &avail, &ret, 0, list);
    snprintf(buf, sizeof buf, "A2 0x474A0 roll row 17: counter %u avail %d ret %d records %u type %02X",
             (unsigned)D32(0x108198u), (int)avail, (int)ret, (unsigned)D32(0x1079B4u), D8(0x107697u));
    check(D32(0x108198u) == 8 && avail == 2 && ret == 1 && D32(0x1079B4u) == 1 && D8(0x107697u) == 0x13 &&
              D8(0x107696u) == 2 && t_tok(u1, n) == 2 && D8(0x10819Cu + (uint32_t)s) == 1,
          buf);
    setD16(T_TAB(18, n), 300 * 90);                                /* 90: the roll fails, the counter still moves */
    avail = 2;
    setD8(0x10819Cu + (uint32_t)s, 0);
    f474A0(s, n, &avail, &ret, 0, list);
    check(D32(0x108198u) == 9 && avail == 2 && D32(0x1079B4u) == 1, "A2 a failed roll counts, moves nothing");
    g_scr_cc5c = 0;                                                /* no need: no roll, no count */
    setD8(0x10819Cu + (uint32_t)s, 0);
    f474A0(s, n, &avail, &ret, 0, list);
    check(D32(0x108198u) == 9, "A2 no roll without a candidate");
    g_scr_n = -1;
    g_scr_cc5c = g_scr_c838 = -1000;

    /* A3: 0x46BB8 clears exactly 4 bytes of the mark array */
    memcpy(g_ds, g_base[0], DS_SIZE);
    for (i = 0; i < 160; i++)
        setD32(0x107EE4u + 4u * (uint32_t)i, 1);
    clear_nation(n);   /* nothing to spread */
    setD8(0xC28ACu, 0);
    setD32(0xCAC54u, (uint32_t)-1);
    if (f46B34(n) == 0) {
        g_calls = 0;
        f46BB8(n);
        check(D32(0x107EE4u) == 0 && D32(0x107EE8u) == 1 && D32(0x107EE4u + 4u * 159u) == 1,
              "A3 0x46BB8 clears entry 0 of the marks only");
    } else
        check(1, "A3 (f46B34 true: nothing cleared)");

    /* A4: 0x49E04 fills only the first target */
    memcpy(g_ds, g_base[0], DS_SIZE);
    clear_nation(n);
    s = find_land(2, &u1, &u2);
    setD8(T_NAT(n) + 0x27u, 0);
    f57E28(s, n, 6);
    f460C0(n);
    setD8(T_TS(s) + 2u, 0);
    setD8(T_SC(n, u1), 50);
    setD8(T_SC(n, u2), 40);
    g_scr_n = 2;
    g_scr[0] = u1;
    g_scr[1] = u2;
    setD32(0x1079B4u, 0);
    f49E04(n);
    {
        int lu1 = f78D08(u1, n), m = f78D08(s, n);
        int spare = 6 - (m >= 3 ? 2 : m);
        int exp1 = spare < lu1 ? spare : lu1;
        snprintf(buf, sizeof buf, "A4 0x49E04: u1 %d tokens %d (limit %d), u2 %d tokens %d", u1, t_tok(u1, n), lu1, u2,
                 t_tok(u2, n));
        check(t_tok(u1, n) == exp1 && t_tok(u2, n) == 0 && (int)D32(0x1079B4u) == exp1 &&
                  (exp1 == 0 || D8(0x107697u) == 0x18),
              buf);
    }
    g_scr_n = -1;

    /* A5: 0x4C074 leaves [0xF7C9C] at the last support list */
    memcpy(g_ds, g_base[1], DS_SIZE);
    {
        int found = 0, nn;
        for (nn = 1; nn <= 9 && !found; nn++) {
            int k0 = (int8_t)D8(T_NAT(nn) + 0x60u);
            memcpy(g_start, g_ds, DS_SIZE);
            f4C074(nn);
            if ((int)D32(0xF7C9Cu) != k0)
                found = 1;
            if (!found)
                memcpy(g_ds, g_start, DS_SIZE);
        }
        printf("   A5: 0x4C074 overwrote its list in the save: %s\n", found ? "yes" : "no nation had a candidate");
        check(1, "A5");
    }

    /* A6: 0x46CBC with an empty list and [0xC28AC]: avail is the stale word (0 here): no attack, no roll */
    memcpy(g_ds, g_base[0], DS_SIZE);
    setD8(0xC28ACu, 1);
    setD32(0x108198u, 5);
    g_scr_n = 0;
    clear_nation(n);
    s = find_land(3, &u1, &u2);
    f57E28(s, n, 3);
    f460C0(n);
    setD8(T_FL(n, s), 0x20);
    setD8(0x10819Cu + (uint32_t)s, 0);
    *aiplan_test_slot38() = 0;
    r = f46CBC(s, n);
    check(r == 0 && D32(0x108198u) == 5 && *aiplan_test_slot38() == 0,
          "A6 0x46CBC empty list with the stale word 0: no attack, the word kept");
    g_scr_n = -1;

    /* A7: 0x49A1C's guard */
    memcpy(g_ds, g_base[0], DS_SIZE);
    setD8(0x1082DCu, 1);
    g_calls = 0;
    r = f49A1C(5, n, 1, -1);
    check(r == 0 && g_calls == 0 && D8(0x1082DCu) == 1, "A7 0x49A1C guarded");

    /* A8: 0x4C9B8: a failed plan restores both stacks and clears [0x107D40]; a success keeps the move */
    memcpy(g_ds, g_base[0], DS_SIZE);
    setD32(0xCAC54u, (uint32_t)-1);
    setD32(0xCAC58u, (uint32_t)-1);
    setD8(T_NAT(n) + 0x98u, 20);
    for (i = 1; i <= 160; i++)
        setD8(T_AREA(i) + 0x3Bu + (uint32_t)n, 0);
    clear_nation(n);
    s = find_land(4, &u1, &u2);
    setD8(T_AREA(s + 1) + 0x3Bu + (uint32_t)n, 2);
    setD8(T_TS(s) + 2u, 0);
    setD32(0x107D40u, 1);
    g_scr_cc5c = 3;
    g_scr_3580 = 1;
    setD32(0x1079B4u, 2);
    memcpy(g_start, g_ds, DS_SIZE);
    f4C9B8(n, 2);
    check(t_tok(s, n) == 0 && D32(0x1079B4u) == 2 && D32(0x107D40u) == 0 && (int32_t)D32(0xCAC54u) == -1 &&
              (int32_t)D32(0xCAC58u) == -1 && g_stack_err == 0,
          "A8 0x4C9B8 failure: both stacks popped");
    memcpy(g_ds, g_start, DS_SIZE);
    g_scr_3580 = 0;
    f4C9B8(n, 2);
    check(t_tok(s, n) == 1 && D32(0x1079B4u) == 3 && D8(T_AREA(s + 1) + 0x3Bu) == 1 && (int32_t)D32(0xCAC54u) == -1 &&
              (int32_t)D32(0xCAC58u) == -1 && g_stack_err == 0,
          "A8 0x4C9B8 success: both stacks dropped, +0x3B = tokens");
    g_scr_cc5c = g_scr_3580 = -1000;
    printf("A. %d fail so far\n", g_fail);
}

/* ---- part B: random differential checks ---------------------------------------------------------------------------- */

static int part_b(int count, int quiet)
{
    Call c;
    int i, bad = 0, per[NFN] = { 0 }, cut = 0;
    char tag[40];
    for (i = 0; i < count; i++) {
        random_state();
        random_call(&c);
        g_seed = rnd(0xFFFFFFu) * 977u + (uint32_t)i;
        snprintf(tag, sizeof tag, "B%d", i);
        if (!compare_one(&c, tag)) {
            bad++;
            if (!quiet && !g_mut)
                check(0, tag);
        } else if (!quiet)
            check(1, tag);
        per[c.fn]++;
        if (strstr(g_log[0], "CUT"))
            cut++;
    }
    if (!quiet && !g_mut) {
        printf("B. %d checks:", count);
        for (i = 0; i < NFN; i++)
            printf(" %s %d", g_fname[i], per[i]);
        printf(" (%d cut)\n", cut);
    }
    return bad;
}

/* ---- part C: the real saves -------------------------------------------------------------------------------------- */

static void part_c(int b, const char *name)
{
    Call c;
    int n, fn;
    char tag[60];
    printf("C. %s: computers", name);
    for (n = 1; n <= 9; n++)
        if (D8(T_NAT(n) + 0x9Au) == 2)
            printf(" %d", n);
    printf("\n");
    for (n = 1; n <= 9; n++) {
        if (D8(T_NAT(n) + 0x9Au) != 2)
            continue;
        printf("   n %d:", n);
        for (fn = 0; fn < NFN; fn++) {
            memcpy(g_ds, g_base[b], DS_SIZE);
            setD32(0xCAC54u, (uint32_t)-1);
            setD32(0xCAC58u, (uint32_t)-1);
            f460C0(n);   /* as the movement phase does before a nation's turn (0x55D8F) */
            setD32(0x1079B4u, 0);
            setD32(0x107D3Cu, 0);
            memset(&c, 0, sizeof c);
            c.fn = fn;
            c.a[0] = n;
            if (fn == F46CBC || fn == F49C88 || fn == F474A0 || fn == F49A1C) {
                c.a[0] = (int8_t)D8(T_NAT(n) + 0x60u) > 0 ? D8(T_NAT(n) + 0x61u) - 1 : 0;
                c.a[1] = n;
                c.a[2] = 3;
                c.a[4] = 1;
                c.a[5] = -1;
            } else if (fn == F478D8 || fn == F47A44) {
                c.a[0] = 1;
                c.a[1] = n;
            } else if (fn == F4A088)
                c.a[4] = 1;
            else if (fn == F4A2E0) {
                c.a[1] = (int8_t)D8(T_NAT(n) + 0x60u) > 0 ? D8(T_NAT(n) + 0x61u) - 1 : 0;
                c.a[0] = 3;
                c.a[4] = n;
                c.cnt = 1;
                c.list[0] = c.a[1];
                c.a[7] = 1;
            } else if (fn == F4C9B8)
                c.a[1] = 1;
            else if (fn == F471CC) {
                int i, k = (int8_t)D8(T_NAT(n) + 0x60u);
                for (i = 0; i < k; i++)
                    c.list[c.cnt++] = D8(T_NAT(n) + 0x61u + (uint32_t)i) - 1;
            }
            g_seed = (uint32_t)(n * 100 + fn);
            snprintf(tag, sizeof tag, "C %s n%d", name, n);
            check(compare_one(&c, tag), tag);
            printf(" %s:%d", g_fname[fn], (int)D32(0x1079B4u));
        }
        printf("\n");
    }
}

/* ---- part D: mutations -------------------------------------------------------------------------------------------- */

static void part_d(void)
{
    static const int muts[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    int i, bad, caught = 0;
    char buf[100];
    uint32_t keep = g_rs;
    for (i = 0; i < (int)(sizeof muts / sizeof muts[0]); i++) {
        g_mut = muts[i];
        g_rs = 777u + (uint32_t)i;
        bad = part_b(6000, 1);
        snprintf(buf, sizeof buf, "D mutation %d caught (%d of 6000 checks differ)", g_mut, bad);
        printf("   %s\n", buf);
        if (bad > 0)
            caught++;
        check(bad > 0, buf);
    }
    g_mut = 0;
    g_rs = keep;
    printf("D. %d of %d mutations caught\n", caught, (int)(sizeof muts / sizeof muts[0]));
}

/* ---- main ---------------------------------------------------------------------------------------------------------- */

static int load_save(const char *path, int slot)
{
    FILE *fp = fopen(path, "rb");
    int16_t ver = 0;
    char desc[64];
    if (fp == NULL) {
        printf("cannot open %s\n", path);
        return 0;
    }
    f5F05C(&ver, desc, fp);
    fclose(fp);
    setD32(0xCAC54u, (uint32_t)-1);
    setD32(0xCAC58u, (uint32_t)-1);
    memcpy(g_base[slot], g_ds, DS_SIZE);
    printf("   %s: version %d \"%.40s\"\n", path, ver, desc);
    return 1;
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:/work/advciv_run";
    const char *playdir = argc > 2 ? argv[2] : "C:/work/advciv_play";
    char cwd[400], path[400];
    int t, zero = 0;
    uint8_t *init;

    if (state_init(gamedir) != 0)
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
    for (t = 0; t < 157; t++)
        if (((uint8_t)(D8(T_TERR(t) + 1u) << 2) >> 5) == 0)
            zero++;
    printf("   population limit 0: %d of 157 territories\n", zero);
    printf("   initial [0xC2860] 0x%X [0xC28AC] %u [0xC28B0] %u [0xC28B4] %u [0x108198] %u\n", (unsigned)D32(0xC2860u),
           D8(0xC28ACu), (unsigned)D32(0xC28B0u), (unsigned)D32(0xC28B4u), (unsigned)D32(0x108198u));
    g_start = malloc(DS_SIZE);
    g_port_ds = malloc(DS_SIZE);
    g_base[0] = malloc(DS_SIZE);
    g_base[1] = malloc(DS_SIZE);
    init = malloc(DS_SIZE);
    if (g_start == NULL || g_port_ds == NULL || g_base[0] == NULL || g_base[1] == NULL || init == NULL)
        return 1;
    memcpy(init, g_ds, DS_SIZE);
    snprintf(path, sizeof path, "%s/savgame0.dat", gamedir);
    if (!load_save(path, 0))
        return 1;
    memcpy(g_ds, init, DS_SIZE);
    snprintf(path, sizeof path, "%s/SAVGAMEA.DAT", playdir);
    if (!load_save(path, 1))
        return 1;
    g_nbase = 2;

    part_a();

    memcpy(g_ds, g_base[0], DS_SIZE);
    part_c(0, "savgame0.dat (start of MOVEMENT R4)");
    memcpy(g_ds, g_base[1], DS_SIZE);
    part_c(1, "SAVGAMEA.DAT (R4 autosave)");
    printf("C. %d fail so far\n", g_fail);

    part_b(60000, 0);
    g_original = 1;
    part_b(10000, 0);
    g_original = 0;
    printf("B. %d fail so far\n", g_fail);

    part_d();

    printf("%d checks, %d fail\n", g_tests, g_fail);
    return g_fail == 0 ? 0 : 1;
}
