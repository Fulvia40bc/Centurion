/* aiadv_test [GAMEDIR [PLAYDIR]]: unit checks for module ADVAI (aiadv.c) that need no game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init; default C:\work\advciv_run, only read), terrflag.dat (f12394:
   the territory flags are not in the save), LANDADJ.DAT (the land lists, parsed into one heap block with a handle per
   territory at 0xF8E7C + 4t, as the game keeps them) and savgame0.dat (R4 at MOVEMENT); PLAYDIR holds SAVGAMEA.DAT
   (the R4 autosave: seven computers; default C:\work\advciv_play, only read). The saves are read with saveload.c's
   f5F05C as the game does.

   aiadv.c is linked as it is (it calls no unported routine). Every routine it calls that is not formulas.c's is a stub
   here that appends one line to a trace: game_rand f78AE4 (the LCG on [0xCAC5C], game.h), f17760 and f76F14 (as
   game.h / info.h describe them), the UI kit (by-id stores into the dialog records as kit.h / game.h describe them),
   fg.c, junk.c; the click wait f78BA4 and the unattended check f7BA64 answer from a seeded script (-100, other codes,
   then -1). formulas.c (f1B538, f1B4D4, f1C2DC, f19448) and load.c (the heap) are the real, separately tested ones.

   The model is a second implementation of the six routines written here from the listing (not from aiadv.c), in the
   listing's own terms: fields read as the original reads them (dword [..] sar 24 / sar 16 at the listing's
   displacements), the odometer of 0x1B59C with its eighteen carry flags and the flag block at their frame offsets,
   0x1BE08 with the listing's labels. Each check runs the port, keeps its data object, trace and outputs, restores the
   start state, runs the model, and compares all three: the whole data object byte for byte (the seed included), the
   traces line for line, the return values and out-parameters. Parts:
     A. hand vectors with the expected values written out: 0x1B45C (a draw only when the score beats the cost);
        0x1BE08 / 0x1B59C on a two-commodity hand (exact fit; the excess kept below the tolerance; F6750); 0x1BE08 with
        no commodity (stale-word fix: e = 0); 0x177EC's weights for three candidates and its special case (no draw);
        0x19700's text for one, two and three advances in both modes (the " and " slip with --original).
     B. random differential checks: each routine called directly on random states (a real save with random hands,
        advances, states, scores, attitudes, AI tables, credits, Mining lists, dialogs, seeds), both modes.
     C. the real saves: f1A888, f177EC and f1B59C for every computer of the R4 autosave and savgame0, the states set as
        0x17EDC would (owned 1; hand + treasury + credit >= cost 2), port against model; the choices printed.
     D. mutations: deliberately broken copies of the model must each be caught by part B's checks.
   Prints each failure (the first differing trace line or byte), the counts per part, and exits 0 when all pass.

   Build (from the repo root):
     clang -std=c11 -Wall -Wextra -Iport/src port/test/aiadv_test.c port/src/aiadv.c port/src/formulas.c
       port/src/saveload.c port/src/load.c port/src/state.c port/src/le_image.c -o aiadv_test.exe
   (f78D08, f76F70, f44CAC, f3A780 are named by formulas.c / load.c only: stubs here that stop the run if reached.)
   Result (21 Nov): A 13, C 70, B 42,000 (30,000 fixed + 12,000 --original) checks, 0 fail; D 9 of 9 mutations caught. */

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <direct.h>
#define chdir _chdir
#define getcwd _getcwd

#include "aiadv.h"
#include "fg.h"
#include "formulas.h"
#include "game.h"
#include "info.h"
#include "junk.h"
#include "kit.h"
#include "saveload.h"
#include "state.h"
#include "ui.h"

void f12394(void);   /* load.c: terrflag.dat into the territory records */

/* ---- traces ---------------------------------------------------------------------------------------------------- */

#define LOGSZ (1u << 22)
static char *g_log[2];
static size_t g_len[2];
static int g_cur;            /* 0 the port's run, 1 the model's */
static int g_mut;            /* part D: the model's mutation */

static void tr(const char *fmt, ...)
{
    va_list ap;
    char *log = g_log[g_cur];
    size_t *len = &g_len[g_cur];
    int k;

    if (*len + 600 >= LOGSZ)
        return;
    va_start(ap, fmt);
    k = vsnprintf(log + *len, LOGSZ - *len, fmt, ap);
    va_end(ap);
    if (k > 0)
        *len += (size_t)k;
    log[(*len)++] = '\n';
    log[*len] = 0;
}

/* ---- the per-run script (f7BA64, f78BA4) ------------------------------------------------------------------------- */

static uint32_t g_srng;
static int g_steps;
static jmp_buf g_cut;
static int g_cut_on;

static uint32_t srnd(uint32_t n)
{
    g_srng = g_srng * 1103515245u + 12345u;
    return n ? (g_srng >> 8) % n : 0;
}

static void step(void)
{
    if (++g_steps > 4000 && g_cut_on) {
        tr("CUT");
        longjmp(g_cut, 1);
    }
}

/* ---- stubs: game.h / info.h / the UI kit / fg.c / junk.c ------------------------------------------------------- */

int f78AE4(void)
{
    uint32_t s = D32(0xCAC5Cu) * 0x15A4E35u + 1u;
    setD32(0xCAC5Cu, s);
    tr("rand %u", (unsigned)((s >> 16) & 0x7FFFu));
    return (int)((s >> 16) & 0x7FFFu);
}

int f17760(int n)
{
    int i, s = 0;
    for (i = 0; i < 24; i++)
        if (D8(0xF14CCu + 199u * (uint32_t)n + 0x24u + (uint32_t)i) == 1)
            s += S16(0xBD480u + 2u * (uint32_t)i);
    tr("f17760 %d = %d", n, s);
    return s;
}

int f76F14(int n)
{
    int i, s = 0;
    for (i = 1; i <= 0x13; i++)
        s += S8(0xF14CCu + 199u * (uint32_t)n + (uint32_t)i);
    tr("f76F14 %d = %d", n, s);
    return s;
}

static uint32_t find_btn(uint32_t dlg, int id)
{
    int i, c = S16(dlg);
    for (i = 0; i < c && i < 60; i++)
        if (D16(dlg + 2u + 20u * (uint32_t)i + 0x0Eu) == (uint16_t)id)
            return dlg + 2u + 20u * (uint32_t)i;
    return 0;
}

void f3B4E0(uint32_t rec) { tr("f3B4E0 %X en%d pr%d", (unsigned)rec, D8(rec + 0x10u), D8(rec + 0x12u)); }
void f3C530(uint32_t dlg, int id)
{
    uint32_t r = find_btn(dlg, id);
    tr("f3C530 %X %d %s", (unsigned)dlg, id, r ? "" : "notfound");
    if (r)
        setD8(r + 0x10u, 0);
}
void f3BB48(uint32_t dlg, int id, int x)
{
    uint32_t r = find_btn(dlg, id);
    tr("f3BB48 %X %d %d", (unsigned)dlg, id, x);
    if (r)
        setD16(r, (uint16_t)x);
}
void f3BC64(uint32_t dlg, int id, int y)
{
    uint32_t r = find_btn(dlg, id);
    tr("f3BC64 %X %d %d", (unsigned)dlg, id, y);
    if (r)
        setD16(r + 2u, (uint16_t)y);
}
void f3C284(uint32_t dlg, int id, uint32_t label)
{
    uint32_t r = find_btn(dlg, id);
    tr("f3C284 %X %d %X", (unsigned)dlg, id, (unsigned)label);
    if (r)
        setD32(r + 8u, label);
}
void f3B05C(uint32_t dlg) { tr("f3B05C %X", (unsigned)dlg); setD32(0xF8E78u, dlg); }
int f3A42C(int x, int y, int w, uint32_t align, const char *text, int32_t *maxw, int draw)
{
    int lines = 1;
    const char *p;
    for (p = text; *p; p++)
        if (*p == '\n')
            lines++;
    tr("f3A42C %d %d %d %u [%s] %d", x, y, w, (unsigned)align, text, draw);
    *maxw = w - 3;
    return y + 15 * lines;
}
void f3C9D0(int x, int y, int w, int h, int flags) { tr("f3C9D0 %d %d %d %d %d", x, y, w, h, flags); }
void f78884(void) { tr("f78884"); }
void f788AC(void) { tr("f788AC"); }
void f3CCBC(void) { tr("f3CCBC"); }
int f7BA64(void)
{
    int r = srnd(4) == 0;
    tr("f7BA64 %d", r);
    step();
    return r;
}
int f78BA4(uint32_t dlg, int map)
{
    static const int codes[] = { -100, -100, -3, 7, -1 };
    int r = codes[srnd(5)];
    tr("f78BA4 %X %d = %d", (unsigned)dlg, map, r);
    step();
    return r;
}
void f78A20(int t) { tr("f78A20 %d", t); }
int fg_vbopen(int h) { tr("fg_vbopen %d", h); return 0; }
void fg_vbclose(void) { tr("fg_vbclose"); }
int fg_setcolor(int c) { tr("fg_setcolor %d", c); return 0; }
int fg_vbpaste(int a, int b, int c, int d, int e, int f) { tr("fg_vbpaste %d %d %d %d %d %d", a, b, c, d, e, f); return 0; }
void fg_move(int x, int y) { tr("fg_move %d %d", x, y); }
int fgf_width(const char *s, int n) { tr("fgf_width [%s] %d", s, n); return 7 * n; }
int junk_makegif(int a, int b, int c, int d, const char *name) { tr("makegif %d %d %d %d %s", a, b, c, d, name); return 0; }
int junk_showgif(const char *name, int flags) { tr("showgif %s %d", name, flags); return 0; }

/* named by formulas.c / load.c, never reached by these routines */
int f78D08(int t, int n) { printf("unexpected f78D08 %d %d\n", t, n); exit(2); }
void f76F70(int n, uint8_t *c) { printf("unexpected f76F70 %d %p\n", n, (void *)c); exit(2); }
_Noreturn void f44CAC(void) { printf("unexpected f44CAC\n"); exit(2); }
void f3A780(const char *m) { printf("unexpected f3A780 %s\n", m); exit(2); }

/* ---- the model (from the listing) -------------------------------------------------------------------------------- */

static int32_t sar24(uint32_t a) { return (int32_t)D32(a) >> 24; }   /* dword [a] sar 24 = the byte at a + 3 */
static int32_t sar16(uint32_t a) { return (int32_t)D32(a) >> 16; }   /* dword [a] sar 16 = the word at a + 2 */
static int32_t sd(uint32_t a) { return S32(a); }
static void wd(uint32_t a, int32_t v) { setD32(a, (uint32_t)v); }
static int32_t idv(int32_t a, int32_t b) { return b == 0 ? 0 : b == -1 ? (int32_t)(0u - (uint32_t)a) : a / b; }

static int m_f1B45C(int32_t i)
{
    int32_t v0c = 0, ebx, edx;
    uint32_t eax;
    if (sar16(2u * (uint32_t)i + 0xBD47Eu) < sd(4u * (uint32_t)i + 0xF62ECu)) {
        edx = sar16(2u * (uint32_t)i + 0xBD47Eu);
        ebx = sd(4u * (uint32_t)i + 0xF62ECu);
        eax = (uint32_t)f78AE4();
        ebx -= edx;
        edx = (int32_t)eax;
        eax <<= 2;
        eax -= (uint32_t)edx;
        eax <<= 3;
        eax += (uint32_t)edx;
        eax <<= 2;
        eax >>= 15;
        if (g_mut == 1)
            eax >>= 1;
        if ((int32_t)eax < ebx)
            v0c = 1;
    }
    return v0c;
}

static void m_fill(void)   /* the state-2 list (0x17829-type loops) */
{
    int32_t v58, eax;
    wd(0xF7C9Cu, 0);
    for (v58 = 0; v58 <= 0x17; v58++)
        if (sar24((uint32_t)v58 + 0xF67C1u) == 2) {
            eax = sd(0xF7C9Cu);
            wd(0xF7C9Cu, eax + 1);
            wd(4u * (uint32_t)eax + 0xF7CA0u, v58);
        }
}

static void m_squeeze(void)
{
    int32_t v58, v54 = 0;
    for (v58 = 0; v58 < sd(0xF7C9Cu); v58++)
        if (sd(4u * (uint32_t)v58 + 0xF7CA0u) != -1) {
            wd(4u * (uint32_t)v54 + 0xF7CA0u, sd(4u * (uint32_t)v58 + 0xF7CA0u));
            v54++;
        }
    wd(0xF7C9Cu, v54);
}

static int m_f177EC(int n)
{
    uint32_t v18 = 0xF14CCu + 0xC7u * (uint32_t)n, ebx, eax;
    int32_t v24 = -1, v58, v54, v50, v4c, v2c, v44, v40, v34, v30, v3c, v38, v20, v1c, v28, edx, ecx, esi;
    int32_t w[4] = { 0, 0, 0, 0 };
    uint32_t lead;

    m_fill();
    v50 = 0;
    for (v58 = 0; v58 <= 0x17; v58++)
        if (D8(v18 + (uint32_t)v58 + 0x24u) != 0)
            v50 |= sar16(2u * (uint32_t)v58 + 0xBD50Eu);
    v4c = v50;
    v2c = 0;
    for (v58 = 0; v58 < 5; v58++) {
        if (v50 & 1)
            v2c++;
        v50 >>= 1;
    }
    v50 = v4c;
    v44 = 0;
    ebx = 5u * (uint32_t)(sar24(v18 + 0x59u) + 1);
    eax = 85u * (uint32_t)(n - 1);
    if (sar24(ebx + eax + 0xBCCDAu) > v2c) {
        v44 = 1;
        for (v58 = 0; v58 < sd(0xF7C9Cu); v58++)
            if ((sar16(2u * (uint32_t)sd(4u * (uint32_t)v58 + 0xF7CA0u) + 0xBD50Eu) | v50) == v50)
                wd(4u * (uint32_t)v58 + 0xF7CA0u, -1);
    }
    m_squeeze();
    v40 = 0;
    if (sd(0xF7C9Cu) == 0)
        m_fill();
    else if (v44 != 0 && g_mut != 2)
        v40 = 1;
    v34 = f17760(n);
    edx = 5 * (sar24(v18 + 0x59u) + 1);
    eax = 85u * (uint32_t)(n - 1);
    edx = sar24((uint32_t)edx + eax + 0xBCCDCu);
    v30 = edx * 100;
    v3c = 0;
    if (v30 > v34) {
        v3c = 1;
        for (v58 = 0; v58 < sd(0xF7C9Cu); v58++)
            if (sar16(2u * (uint32_t)sd(4u * (uint32_t)v58 + 0xF7CA0u) + 0xBD47Eu) + v34 < v30)
                wd(4u * (uint32_t)v58 + 0xF7CA0u, -1);
    }
    m_squeeze();
    v38 = 0;
    if (sd(0xF7C9Cu) == 0)
        m_fill();
    else if (v3c != 0)
        v38 = 1;
    for (v58 = sd(0xF7C9Cu) - 2; v58 >= 0; v58--)
        for (v54 = 0; v54 <= v58; v54++) {
            int32_t a1 = sd(4u * (uint32_t)v54 + 0xF7CA4u), a0 = sd(4u * (uint32_t)v54 + 0xF7CA0u);
            int gt = g_mut == 3 ? sd(4u * (uint32_t)a1 + 0xF62ECu) >= sd(4u * (uint32_t)a0 + 0xF62ECu)
                                : sd(4u * (uint32_t)a1 + 0xF62ECu) > sd(4u * (uint32_t)a0 + 0xF62ECu);
            if (gt) {
                v20 = a1;
                wd(4u * (uint32_t)v54 + 0xF7CA4u, a0);
                wd(4u * (uint32_t)v54 + 0xF7CA0u, v20);
            }
        }
    lead = D32(0xF52D4u);
    {
        uint32_t la = lead * 0xC7u;
        if (ds_valid(la + 0xF1563u, 4) && sar24(la + 0xF1563u) == 3 && D8(la + 0xF1501u) != 0
            && ds_valid(4u * lead + 40u * (uint32_t)n + 0xF52E0u, 4)
            && sd(4u * lead + 40u * (uint32_t)n + 0xF52E0u) > 0x1F4) {
            v24 = sd(0xF7CA0u);
            goto L_17E8A;
        }
    }
    if (sd(0xF7C9Cu) > 4)
        wd(0xF7C9Cu, 4);
    ebx = 4u * D32(0xF7CA4u);
    ecx = sd(0xF7CA0u);
    esi = sd(0xF7CA8u);
    switch (sd(0xF7C9Cu) - 1) {
    case 0:
        w[0] = sd(4u * (uint32_t)ecx + 0xF62ECu);
        break;
    case 1:
        w[0] = sd(4u * (uint32_t)ecx + 0xF62ECu) * 2;
        w[1] = sd(ebx + 0xF62ECu);
        break;
    case 2:
        edx = sd(4u * (uint32_t)ecx + 0xF62ECu);
        w[0] = edx + idv(edx * 33 * 2, 100);
        edx = sd(ebx + 0xF62ECu);
        w[1] = edx + idv(edx * 33, 100);
        w[2] = sd(4u * (uint32_t)esi + 0xF62ECu);
        break;
    case 3:
        edx = sd(4u * (uint32_t)ecx + 0xF62ECu);
        w[0] = edx + idv(edx * 25 * 2, 100);
        edx = sd(ebx + 0xF62ECu);
        w[1] = edx + idv(edx * 15 * (g_mut == 4 ? 3 : 2), 100);
        edx = sd(4u * (uint32_t)esi + 0xF62ECu);
        w[2] = edx + idv(edx * 5 * 4, 100);
        w[3] = sd(4u * D32(0xF7CACu) + 0xF62ECu);
        break;
    default:
        break;
    }
    v1c = 0;
    for (v58 = 0; v58 < sd(0xF7C9Cu); v58++)
        v1c += w[v58];
    v28 = (int32_t)(((uint32_t)f78AE4() * (uint32_t)v1c) >> 15);
    edx = w[0] + w[1];
    switch (sd(0xF7C9Cu) - 1) {
    case 0:
        v24 = sd(0xF7CA0u);
        break;
    case 1:
        v24 = v28 < w[0] ? sd(0xF7CA0u) : sd(0xF7CA4u);
        break;
    case 2:
        if (v28 < w[0])
            v24 = sd(0xF7CA0u);
        else if (!(v28 < w[0]) && !(edx <= v28))
            v24 = sd(0xF7CA4u);
        else
            v24 = sd(0xF7CA8u);
        break;
    case 3:
        if (v28 < w[0])
            v24 = sd(0xF7CA0u);
        else if (!(v28 < w[0]) && !(edx <= v28))
            v24 = sd(0xF7CA4u);
        else if (!(w[0] + w[1] > v28) && w[0] + w[1] + w[2] > v28)
            v24 = sd(0xF7CA8u);
        else
            v24 = sd(0xF7CACu);
        break;
    default:
        break;
    }
L_17E8A:
    if (f76F14(n) > 8)
        return v24;
    if (f1B538(n) != 0)
        return v24;
    if (v40 != 0 || v38 != 0)
        return v24;
    if (m_f1B45C(v24) != 0)
        return v24;
    if (f1B4D4(n) != 0)
        return v24;
    return -1;
}

static int32_t m_land(uint32_t h)
{
    const uint8_t *p = load_heap_ptr(h, 2);
    if (p != NULL)
        return (int16_t)(p[0] | p[1] << 8);
    if (ds_valid(h, 2))
        return S16(h);
    return 0;
}

static void m_f1A888(int n)
{
    uint32_t v18 = 0xF14CCu + 0xC7u * (uint32_t)n;
    int32_t v4c = n - 1, v34 = -5000, v28 = 0, v30 = 0, v54, v60, v5c, v58, v50, v48, v1c, v20, v24, v44, v40, v3c, v38, v2c;
    int32_t e;

    memset(DP(0xF62ECu), 0, 0x60);
    for (v60 = 1; v60 <= 9; v60++) {
        if (D8(0xC7u * (uint32_t)v60 + 0xF1566u) == 0)
            continue;
        v28 = sd(40u * (uint32_t)n + 4u * (uint32_t)v60 + 0xF52E0u);
        if (sd(40u * (uint32_t)n + 4u * (uint32_t)v60 + 0xF52E0u) > v34) {
            v34 = sd(40u * (uint32_t)n + 4u * (uint32_t)v60 + 0xF52E0u);
            v30 = v60;
        }
    }
    v28 = idv(v28, sd(0xF40ECu));
    for (v54 = 0; v54 <= 0x17; v54++) {
        if (D8(0xC7u * (uint32_t)n + (uint32_t)v54 + 0xF14F0u) != 0)
            continue;
        if (sar24((uint32_t)v54 + 0xF67C1u) != 2)
            continue;
        switch (v54) {
        case 0:
            v50 = 0x2D;
            if (sd(0xF52D4u) == n && v34 < -200)
                v50 += 0x32;
            wd(0xF62ECu, v50);
            break;
        case 1:
            v50 = 0x2D;
            if ((uint32_t)(n - 1) <= 8) {
                static const int8_t t1[9] = { 30, 99, -10, -10, 50, 0, 0, -20, -20 };
                if (t1[n - 1] == 99) {
                    if (sd(0xF40F0u) == 3)
                        v50 += 0xA;
                } else
                    v50 += t1[n - 1];
            }
            if (D8(v18 + 0x2Bu) != 0)
                v50 += 0xA;
            wd(0xF62F0u, v50);
            break;
        case 2:
            v50 = 0x50;
            for (v60 = 0; v60 < 9; v60++) {
                if (sar24(9u * (uint32_t)n + (uint32_t)v60 + 0x109E21u) == 0)
                    continue;
                if (D8((g_mut == 5 ? (uint32_t)(v60 + 1) : (uint32_t)v60) * 0xC7u + 0xF14F2u) == 0)
                    continue;
                v50 += 0x50;
                break;
            }
            for (v60 = 0; sar24(v18 + 0x5Du) > v60; v60++) {
                v48 = D8(v18 + (uint32_t)v60 + 0x61u);
                if (sar24((uint32_t)v48 * 69u + 0xE31B6u) > 1) {
                    for (v5c = 0; v5c < 9; v5c++) {
                        if (v5c == v4c)
                            continue;
                        if ((uint32_t)v48 * 69u + 0xE31B8u + 2u == 0)
                            continue;
                        if (D8((uint32_t)(v5c + 1) * 0xC7u + 0xF14F2u) == 0)
                            continue;
                        v50 += 0x28;
                        v60 = 0x64;
                        break;
                    }
                }
                v20 = (int32_t)D32(4u * (uint32_t)v48 + 0xF8E78u);
                for (v58 = 1; m_land((uint32_t)v20) >= v58; v58++) {
                    v1c = m_land(2u * (uint32_t)v58 + (uint32_t)v20) + 1;
                    if (sar24((uint32_t)v1c * 69u + 0xE31B6u) <= 1)
                        continue;
                    for (v5c = 0; v5c < 9; v5c++) {
                        if (v5c == v4c)
                            continue;
                        if ((uint32_t)v1c * 69u + 0xE31B8u + 2u == 0)
                            continue;
                        if (D8((uint32_t)(v5c + 1) * 0xC7u + 0xF14F2u) == 0)
                            continue;
                        v50 += 0x28;
                        v60 = 0x64;
                        v58 = 0x64;
                        break;
                    }
                }
            }
            if (D8(v18 + 0x35u) != 0)
                for (v60 = 1; v60 <= 9; v60++)
                    if (D8(0xC7u * (uint32_t)v60 + 0xF1501u) == 0)
                        v50 += 0x14;
            if (D8(v18 + 0x2Bu) != 0)
                v50 += 0x1E;
            if (D8(v18 + 0x28u) != 0)
                v50 += 0xA;
            wd(0xF62F4u, v50);
            break;
        case 3:
            v50 = 0x6E;
            if (n == 1 || n == 5)
                v50 += 0x32;
            else if (n == 8 || n == 9)
                v50 -= 0x14;
            if (sar24((uint32_t)n + 0xF5931u) > 0)
                v50 += 0x1E;
            wd(0xF62F8u, v50);
            break;
        case 4:
            v50 = 0x8C;
            if ((uint32_t)n == 1u)
                v50 -= 0x14;
            else if (n == 5)
                v50 -= 0x1E;
            if (v28 > 0xC8) {
                v24 = idv((v28 - 0xC8) * 5 * 8, 0x12C) + 0x3C;
                if (v24 > 0x64)
                    v24 = 0x64;
                v50 += v24;
            }
            e = sar24(v18 + 0x95u);
            if (idv(e * 25 * 4, sd(0xF4898u)) > 0x4B)
                v50 += 0x64;
            for (v60 = 0; v60 < 9; v60++)
                if (sar24(9u * (uint32_t)n + (uint32_t)v60 + 0x109E21u) != 0) {
                    v50 += 0x19;
                    break;
                }
            wd(0xF62FCu, v50);
            break;
        case 5:
            v50 = 0xB4;
            if (D8(v18 + 0x2Cu) != 0)
                v50 += 0x28;
            wd(0xF6300u, v50);
            break;
        case 6:
            v50 = 0x8C;
            if (n == 8)
                v50 += 0x23;
            else if (n == 9)
                v50 += 0x32;
            wd(0xF6304u, v50);
            break;
        case 7:
            v50 = 0x50;
            for (v60 = 0; v60 < 9; v60++)
                if (sar24(9u * (uint32_t)n + (uint32_t)v60 + 0x109E21u) != 0) {
                    v50 += 0x14;
                    break;
                }
            if (n == 1)
                v50 += 0x28;
            else if (n == 5)
                v50 += 0x32;
            if (D8(v18 + 0x35u) != 0)
                v50 += 0x3C;
            wd(0xF6308u, v50);
            break;
        case 8:
            v50 = 0x6E;
            if (D8(v18 + 0x29u) != 0)
                v50 += 0x28;
            if (f1C2DC(n) > 0)
                v50 += 0x28;
            wd(0xF630Cu, v50);
            break;
        case 9:
            v50 = 0x8C;
            if (D8(v18 + 0x28u) != 0)
                v50 += 0x1E;
            if (!(v34 > 0x64))
                v50 += 0x14;
            if (!(v34 < 0x12C))
                v50 -= 0x14;
            if (sd(0xF52D4u) == n)
                v50 += 0x32;
            wd(0xF6310u, v50);
            break;
        case 10:
            v50 = 0xB4;
            if (!(sd(0xF678Cu) < 0x64))
                v50 += 0x32;
            wd(0xF6314u, v50);
            break;
        case 11:
        case 12:
            v50 = 0x3C;
            v44 = v40 = v3c = 0;
            for (v60 = 1; v60 <= 9; v60++) {
                if (D8(0xC7u * (uint32_t)v60 + 0xF1566u) == 0)
                    continue;
                if (sar24(0xC7u * (uint32_t)v60 + 0xF1562u) > v44) {
                    v44 = sar24(0xC7u * (uint32_t)v60 + 0xF1562u);
                    v40 = v60;
                }
                if (D8(0xC7u * (uint32_t)v60 + 0xF1501u) != 0)
                    v3c = 1;
            }
            if (v40 == n)
                v50 -= 0x1E;
            e = sar24(v18 + 0x4Eu) * 5 + sar24(v18 + 0x95u);
            if (e > 0x23)
                v50 += e - 0x23;
            if (v3c != 0)
                v50 += 0xA;
            if (D8(v18 + 0x35u) != 0)
                v50 += 0x14;
            if (D8(v18 + 0x28u) != 0)
                v50 += 0x14;
            wd(4u * (uint32_t)v54 + 0xF62ECu, v50);
            break;
        case 13:
            v50 = 0x78;
            v38 = 0;
            for (v60 = 0; sar24(v18 + 0x5Du) > v60; v60++) {
                v48 = D8(v18 + (uint32_t)v60 + 0x61u);
                if ((D8((uint32_t)(v48 - 1) * 35u + 0xE1639u) & 0x40u) != 0)
                    v38++;
            }
            if (v38 < 9)
                v50 += (9 - v38) * 25;
            if (n == 1)
                v50 += 0x14;
            else if (n == 4)
                v50 += 0x1E;
            wd(0xF6320u, v50);
            break;
        case 14:
            wd(0xF6324u, 0x46);
            break;
        case 15:
            v50 = 0xAA - 0x1E;
            if (!(v34 > 0x64))
                v50 += 0x1E;
            if (D8(v18 + 0x35u) != 0)
                v50 += 0x14;
            if (D8(v18 + 0x28u) != 0)
                v50 += 0x28;
            if (D8(v18 + 0x3Au) != 0)
                v50 += 0x14;
            wd(0xF6328u, v50);
            break;
        case 16:
            v50 = 0xC8 - 0x28;
            if (!(v34 > 0x64))
                v50 += 0x28;
            if (!(sd(0xF67A4u) < 0x64))
                v50 += 0x28;
            if (D8(v18 + 0x35u) != 0)
                v50 += 0x14;
            if (D8(v18 + 0x28u) != 0)
                v50 += 0x14;
            wd(0xF632Cu, v50);
            break;
        case 17:
            v50 = 0xB4;
            v2c = 0;
            for (v60 = 1; v60 <= 9; v60++) {
                if (D8(0xC7u * (uint32_t)v60 + 0xF1566u) == 0)
                    continue;
                if (D8(0xC7u * (uint32_t)v60 + 0xF1501u) == 0)
                    v50 += 0x14;
                else
                    v50 -= 0x14;
                if (D8(0xC7u * (uint32_t)v60 + 0xF14F2u) != 0)
                    v50 += 0xA;
                if (sar24(0xC7u * (uint32_t)v60 + 0xF1563u) == 3 && (v2c == 0 || g_mut == 6)) {
                    v2c = 1;
                    v50 += 0x64;
                }
            }
            if (v30 != n && D8(0xC7u * (uint32_t)v30 + 0xF1501u) != 0)
                v50 += 0x32;
            if (D8(v18 + 0x26u) != 0)
                v50 += 0x1E;
            if (D8(v18 + 0x2Bu) != 0)
                v50 += 0x1E;
            if (D8(v18 + 0x28u) != 0)
                v50 += 0x14;
            if (n == 1 || n == 5)
                v50 += 0x14;
            else if (n == 8 || n == 9)
                v50 -= 0x14;
            wd(0xF6330u, v50);
            break;
        case 18:
            v50 = 0xA0;
            if (!(sd(0xF67ACu) < 0x78))
                v50 += 0x50;
            if (D8(v18 + 0x28u) != 0)
                v50 += 0x14;
            if (D8(v18 + 0x3Au) != 0)
                v50 += 0x14;
            wd(0xF6334u, v50);
            break;
        case 19:
            v50 = 0x32;
            if (D8(v18 + 0x38u) != 0)
                v50 -= 0xA;
            if (D8(v18 + 0x39u) != 0)
                v50 -= 0x1E;
            wd(0xF6338u, v50);
            break;
        case 20:
            v50 = 0x50;
            if (D8(v18 + 0x37u) != 0)
                v50 -= 0xA;
            if (D8(v18 + 0x39u) != 0)
                v50 -= 0x1E;
            wd(0xF633Cu, v50);
            break;
        case 21:
            v50 = 0x96;
            if (D8(v18 + 0x37u) != 0)
                v50 -= 0xA;
            if (D8(v18 + 0x38u) != 0)
                v50 -= 0x1E;
            if (D8(v18 + 0x35u) != 0)
                v50 += 0x14;
            for (v60 = 1; v60 <= 9; v60++) {
                if (D8(0xC7u * (uint32_t)v60 + 0xF1566u) == 0)
                    continue;
                if (D8(0xC7u * (uint32_t)v60 + 0xF1505u) != 0)
                    v50 += 0x1E;
                if (D8(0xC7u * (uint32_t)v60 + 0xF1506u) != 0)
                    v50 += 0x28;
            }
            wd(0xF6340u, v50);
            break;
        case 22:
            v50 = 0xDC;
            if (D8(v18 + 0x28u) != 0)
                v50 += 0xA;
            if (D8(v18 + 0x2Bu) != 0)
                v50 += 0x14;
            if (D8(v18 + 0x35u) != 0)
                for (v60 = 1; v60 <= 9; v60++)
                    if (D8(0xC7u * (uint32_t)v60 + 0xF1566u) != 0 && D8(0xC7u * (uint32_t)v60 + 0xF1501u) == 0)
                        v50 += 0xA;
            for (v60 = 1; v60 <= 9; v60++) {
                if (D8(0xC7u * (uint32_t)v60 + 0xF1566u) == 0)
                    continue;
                if (D8(0xC7u * (uint32_t)v60 + 0xF1505u) == 0)
                    v50 -= 0x14;
                if (D8(0xC7u * (uint32_t)v60 + 0xF1506u) == 0)
                    v50 += 0x28;
                if (D8(0xC7u * (uint32_t)v60 + 0xF1507u) == 0)
                    v50 -= 0x28;
            }
            wd(0xF6344u, v50);
            break;
        case 23:
            v50 = 0xFA;
            if (D8(v18 + 0x28u) != 0)
                v50 += 0x14;
            if (D8(v18 + 0x3Au) != 0)
                v50 -= 0x64;
            if (D8(v18 + 0x3Au) == 0)
                for (v60 = 1; v60 <= 9; v60++)
                    if (D8(0xC7u * (uint32_t)v60 + 0xF1566u) != 0 && D8(0xC7u * (uint32_t)v60 + 0xF1506u) != 0)
                        v50 += 0x32;
            wd(0xF6348u, v50);
            break;
        }
    }
}

/* 0x1BE08 in the listing's labels; e (the [ebp-0x20] word) is 0 when the prefix loop never writes it (the fix) */
static int m_f1BE08(int32_t need, int32_t *p38, int32_t *p34, int32_t *p10x)
{
    int32_t v30, v8, v2c, v28, v20 = 0, v24, v10, vc, v18, v1c, v14, eax, edx;
    uint32_t a;

    wd(0xF6D4Cu, 0);
    for (v30 = 1; v30 <= 0x12; v30++) {
        if ((int8_t)D8(0xF14CCu + (uint32_t)v30) <= 0)
            continue;
        a = (D32(0xF6D4Cu) << 5);
        wd(a + 0xF68D0u, (int8_t)D8(0xF14CCu + (uint32_t)v30));
        wd(a + 0xF68D4u, v30);
        if (v30 == 4 || v30 == 0xC || v30 == 0xB || v30 == 0x10 || v30 == 0x12)
            wd(a + 0xF68DCu, 1);
        else
            wd(a + 0xF68DCu, 0);
        edx = (v30 + 1) >> 1;
        wd(a + 0xF68E0u, edx);
        eax = (int8_t)D8(0xF14CCu + (uint32_t)v30);
        wd(a + 0xF68E4u, edx * (eax * eax));
        wd(a + 0xF68CCu, edx * (eax * eax));
        if (sd(a + 0xF68DCu) != 0)
            wd(a + 0xF68E8u, edx * ((eax + 1) * (eax + 1)));
        wd(0xF6D4Cu, sd(0xF6D4Cu) + 1);
    }
    v8 = -1;
L_1BF40:
    if (D8(0xF14CCu + 0x29u) != 0) {
        for (v30 = v8 + 1; v30 < sd(0xF6D4Cu); v30++) {
            v8 = -1;
            if (sd(((uint32_t)v30 << 5) + 0xF68DCu) == 0)
                continue;
            v8 = v30;
            wd(((uint32_t)v30 << 5) + 0xF68D8u, 1);
            wd(((uint32_t)v30 << 5) + 0xF68CCu, sd(((uint32_t)v30 << 5) + 0xF68E8u));
            for (v2c = 1; v2c < sd(0xF68B4u); v2c++)
                if (sar24((uint32_t)v2c + 0xF68B5u) == sd(((uint32_t)v30 << 5) + 0xF68D4u))
                    wd(0xF68B0u, v2c);
            goto L_1BFF1;
        }
    }
L_1BFF1:
    v28 = 0;
    for (v30 = 0; v30 < sd(0xF6D4Cu); v30++) {
        v28 += sd(((uint32_t)v30 << 5) + 0xF68CCu);
        if (!(v28 + sd(0xF674Cu) < need)) {
            v20 = v30;
            break;
        }
    }
    if (v28 + sd(0xF674Cu) < need) {
        if (v8 == -1)
            return 0;
        if (sd(0xF6D4Cu) - 1 <= v8)
            return 0;
        wd(((uint32_t)v8 << 5) + 0xF68D8u, 0);
        wd(((uint32_t)v8 << 5) + 0xF68CCu, sd(((uint32_t)v8 << 5) + 0xF68E4u));
        goto L_1BF40;
    }
    if (!(v28 + sd(0xF6748u) > need)) {
        *p10x = (int32_t)((1u << ((uint32_t)(v20 + 1) & 31u)) - 1u);
        return 1;
    }
    v10 = 0;
    *p38 = v28 + sd(0xF6748u) - need;
    *p34 = (int32_t)((1u << ((uint32_t)(v20 + 1) & 31u)) - 1u);
    for (v30 = v20 - 1; v30 >= 0; v30--) {
        v28 -= sd(((uint32_t)v30 << 5) + 0xF68CCu);
        if (v28 + sd(0xF674Cu) > need && v28 + sd(0xF6748u) > need) {
            edx = v28 + sd(0xF6748u) - need;
            if (edx < *p38) {
                *p38 = edx;
                *p34 = (int32_t)((1u << ((uint32_t)v20 & 31u)) | ((1u << ((uint32_t)v30 & 31u)) - 1u));
            }
            continue;
        }
        if (!(v28 + sd(0xF674Cu) < need) && !(v28 + sd(0xF6748u) > need)) {
            *p10x = (int32_t)((1u << ((uint32_t)v20 & 31u)) | ((1u << ((uint32_t)v30 & 31u)) - 1u));
            return 1;
        }
        v28 += sd(((uint32_t)v30 << 5) + 0xF68CCu);
        v1c = (int32_t)((1u << ((uint32_t)v20 & 31u)) | ((1u << ((uint32_t)(v30 + 1) & 31u)) - 1u));
        v18 = (int32_t)(1u << ((uint32_t)v20 & 31u));
        vc = v1c;
        while (!(vc < v18)) {
            v24 = v28;
            v14 = vc;
            for (v2c = 0; sd(0xF6D4Cu) - 1 > v2c; v2c++)
                if ((v14 & sd(4u * (uint32_t)v2c + 0xBC800u)) == 0)
                    v24 -= sd(((uint32_t)v2c << 5) + 0xF68CCu);
            if (!(v24 + sd(0xF674Cu) < need) && !(v24 + sd(0xF6748u) > need))
                v10 = vc;
            else if (!(v24 + sd(0xF674Cu) < need) && !(v24 + sd(0xF6748u) < need)) {
                edx = v24 + sd(0xF6748u) - need;
                if (g_mut == 7 ? edx <= *p38 : edx < *p38) {
                    *p38 = edx;
                    *p34 = vc;
                }
            }
            vc--;
        }
        if (v10 != 0) {
            *p10x = v10;
            return 1;
        }
        return 0;
    }
    return 0;
}

static void m_write(uint32_t mask, uint32_t tab, uint32_t cnt)
{
    int32_t i;
    for (i = 1; i <= 0x12; i++)
        setD8(0xF14CCu + (uint32_t)i, 0);
    for (i = 0; i < sd(cnt); i++)
        if ((mask & D32(4u * (uint32_t)i + 0xBC800u)) != 0)
            setD8(0xF14CCu + D32(((uint32_t)i << 5) + tab + 8u), D8(((uint32_t)i << 5) + tab + 4u));
}

static int m_f1B59C(int n, int32_t need)
{
    uint32_t v18 = 0xF14CCu + 0xC7u * (uint32_t)n;
    int32_t v20 = 0x3E7, v38 = 0, v28, v1c = 3, v34 = 0x7FFFFFFF, v30 = 0, v2c = 0, d;
    int32_t flag[19];          /* [ebp-0xDC + 4(k-1)] */
    int32_t blk[18];           /* [ebp-0x94 + 4m] */
    int32_t k, i;

    memmove(DP(0xF14CCu), DP(v18), 199);
    v28 = sd(0xF68B0u);
    wd(0xF6748u, f1C2DC(n));
    wd(0xF674Cu, sar24(v18 + 0x3Bu));
    if (sd(0xF6748u) > sd(0xF674Cu))
        wd(0xF6748u, sd(0xF674Cu));
    memset(blk, 0, sizeof blk);
    for (k = 1; k <= 18; k++)
        flag[k] = 0;
    for (;;) {
        if (v20 < v1c) {
            m_write((uint32_t)v38, 0xF6B0Cu, 0xF6D50u);
            wd(0xF68B0u, v28);
            goto L_paid;
        }
        if (m_f1BE08(need, &v34, &v30, &v2c) == 1) {
            m_write((uint32_t)v2c, 0xF68CCu, 0xF6D4Cu);
            goto L_paid;
        }
        if (v34 < v20) {
            v20 = v34;
            v38 = v30;
            v28 = sd(0xF68B0u);
            for (i = 0; i < sd(0xF6D4Cu); i++)
                memmove(DP(((uint32_t)i << 5) + 0xF6B0Cu), DP(((uint32_t)i << 5) + 0xF68CCu), 32);
            wd(0xF6D50u, sd(0xF6D4Cu));
        }
        if (v20 < v1c) {
            m_write((uint32_t)v38, 0xF6B0Cu, 0xF6D50u);
            wd(0xF68B0u, v28);
            goto L_paid;
        }
        /* the odometer: digit k's byte, then its carry flag; a digit that does not carry clears the flags below */
        for (k = 1;; k++) {
            int32_t c = k <= 2 ? (int8_t)D8(0xF14CCu + (uint32_t)k) : sar24(0xF14CCu + (uint32_t)k - 3u);
            if (c <= 1) {
                setD8(0xF14CCu + (uint32_t)k, D8(v18 + (uint32_t)k));
                flag[k] = 1;
            } else
                setD8(0xF14CCu + (uint32_t)k, (uint8_t)(D8(0xF14CCu + (uint32_t)k) - 1u));
            if (flag[k] == 0) {
                for (i = k - 1; i >= 1; i--)
                    flag[i] = 0;
                break;
            }
            if (k == 18)
                return 0;
            if ((k & 1) == 0 && k <= (g_mut == 8 ? 14 : 16)) {
                int32_t m = k;   /* [ebp-0x8C] for k 2 (m 2) .. [ebp-0x54] for k 16 (m 16) */
                if (blk[m] == 0) {
                    blk[m] = 1;
                    v1c += 3;
                }
            }
        }
    }
L_paid:
    d = need - f19448();
    wd(0xF6750u, d);
    if (d < 0)
        wd(0xF6750u, 0);
    return 1;
}

static void m_cat(char *b, size_t size, uint32_t s)
{
    size_t l = strlen(b);
    if (l + 1 < size)
        snprintf(b + l, size - l, "%s", DSTR(s));
}

static void m_f19700(int n, int cnt, uint32_t list)
{
    char b[300];
    int32_t v10, v28, v24, v18, v34, v30, v2c, v1c, v20;
    uint32_t v14, dlg;

    snprintf(b, sizeof b, DSTR(0xB15E6u), DSTR(D32(4u * (uint32_t)n + 0xC768Cu)));
    for (v10 = 0; v10 < cnt; v10++) {
        m_cat(b, sizeof b, D32(4u * (uint32_t)S16(2u * (uint32_t)v10 + list) + 0xC7DC4u));
        if (cnt > 2) {
            int32_t lim = g_original || g_mut == 9 ? cnt - 1 : cnt - 2;
            if (lim > v10)
                m_cat(b, sizeof b, 0xB15F8u);
            else if (g_original || v10 == cnt - 2)
                m_cat(b, sizeof b, 0xB15FCu);
        } else if (cnt == 2 && v10 == 0)
            m_cat(b, sizeof b, 0xB1603u);
    }
    m_cat(b, sizeof b, 0xB160Au);
    v14 = D32(0xF8E78u);
    f78884();
    for (v10 = 0; (int16_t)D16(D32(0xF8E78u)) > v10; v10++) {
        dlg = D32(0xF8E78u);
        setD8((uint32_t)v10 + 0xF8E44u, D8(dlg + 20u * (uint32_t)v10 + 0x12u));
        if ((int16_t)D16(dlg + 20u * (uint32_t)v10 + 0x10u) < 0x8E || (int16_t)D16(dlg + 20u * (uint32_t)v10 + 0x10u) > 0xA5) {
            f3B4E0(20u * (uint32_t)v10 + D32(0xF8E78u) + 2u);
            setD8(D32(0xF8E78u) + 20u * (uint32_t)v10 + 0x12u, 0);
        }
    }
    f788AC();
    v28 = fgf_width(b, (int)strlen(b)) + 0xA;
    v24 = v28 > 0xB4 ? v28 : 0xB4;
    if (v24 - 0x14 > 0x15E)
        v24 = 0x15E;
    v18 = f3A42C(0, 0, v24 - 0x14, 0, b, &v1c, 0) + 0x2D;
    v34 = (0x280 - v24) / 2;
    v30 = (0x1E0 - v18) / 2;
    v2c = v34 + (v24 - 0x6E) / 2;
    f78884();
    junk_makegif(v34, v34 + v24 - 1, v30, v30 + v18 - 1, DSTR(0xB160Cu));
    f788AC();
    {
        static const int ids[9] = { 5, 3, 4, 6, 7, 8, 0x2A, 0x41, 0xA6 };
        int q;
        for (q = 0; q < 9; q++)
            f3C530(D32(0xF8E78u), ids[q]);
    }
    fg_vbopen(S32(0xF52C8u));
    f3C9D0(v34, v30, v24, v18, 3);
    fg_setcolor(5);
    f3A42C(v34 + 0xA, v30 + 0xA, v24 - 0x14, 2, b, &v1c, 1);
    f3BB48(0xBBBECu, 0x2A, v2c);
    f3BC64(0xBBBECu, 0x2A, v30 + v18 - 0x1D);
    f3C284(0xBBBECu, 0x2A, 0xB1611u);
    f3B05C(0xBBBECu);
    fg_vbclose();
    f78884();
    fg_vbpaste(v34, v34 + v24 - 1, v30, v30 + v18 - 1, v34, v30 + v18 - 1);
    f788AC();
    do {
        if (f7BA64() == 0)
            v20 = f78BA4(0xBBBECu, 0);
        else {
            f78A20((int16_t)D16(0xF40BAu) == 0 ? 0xFA : 0x3E8);
            v20 = -1;
        }
    } while (v20 == -100 || v20 != -1);
    setD32(0xF8E78u, v14);
    f78884();
    for (v10 = 0; (int16_t)D16(D32(0xF8E78u)) > v10; v10++) {
        setD8(D32(0xF8E78u) + 20u * (uint32_t)v10 + 0x12u, D8((uint32_t)v10 + 0xF8E44u));
        f3B4E0(D32(0xF8E78u) + 20u * (uint32_t)v10 + 2u);
    }
    f788AC();
    setD8(0xF52D0u, 0);
    f78884();
    fg_move(v34, v30);
    junk_showgif(DSTR(0xB161Au), 3);
    f3CCBC();
    f788AC();
}

/* ---- the harness ------------------------------------------------------------------------------------------------- */

enum { F1B45C, F177EC, F1A888, F1BE08, F1B59C, F19700, NFN };
static const char *g_fname[NFN] = { "1B45C", "177EC", "1A888", "1BE08", "1B59C", "19700" };

typedef struct {
    int fn;
    int a[4];
    uint32_t list;
} Call;

static uint8_t *g_start, *g_port_ds, *g_base[3];
static int g_nbase;
static int g_tests, g_fail;
static int g_quiet;
static uint32_t g_scr;

static void check(int ok, const char *what)
{
    g_tests++;
    if (!ok) {
        g_fail++;
        if (!g_quiet)
            printf("FAIL %s\n", what);
    }
}

typedef struct {
    int ret;
    int32_t o[3];
} Out;

static Out run(const Call *c, int model)
{
    Out o;
    o.ret = 0;
    o.o[0] = 0x7FFFFFFF;
    o.o[1] = 0;
    o.o[2] = 0;
    g_cur = model;
    g_len[model] = 0;
    g_log[model][0] = 0;
    g_srng = g_scr;
    g_steps = 0;
    g_cut_on = 1;
    if (setjmp(g_cut) != 0) {
        o.ret = -12345;
        g_cut_on = 0;
        return o;
    }
    switch (c->fn) {
    case F1B45C: o.ret = model ? m_f1B45C(c->a[0]) : f1B45C(c->a[0]); break;
    case F177EC: o.ret = model ? m_f177EC(c->a[0]) : f177EC(c->a[0]); break;
    case F1A888: if (model) m_f1A888(c->a[0]); else f1A888(c->a[0]); break;
    case F1BE08:
        if (model)
            o.ret = m_f1BE08(c->a[1], &o.o[0], &o.o[1], &o.o[2]);
        else
            o.ret = f1BE08(c->a[0], c->a[1], &o.o[0], &o.o[1], &o.o[2], c->a[2]);
        break;
    case F1B59C: o.ret = model ? m_f1B59C(c->a[0], c->a[1]) : f1B59C(c->a[0], c->a[1]); break;
    case F19700: if (model) m_f19700(c->a[0], c->a[1], c->list); else f19700(c->a[0], c->a[1], c->list); break;
    default: break;
    }
    g_cut_on = 0;
    return o;
}

/* run the port and the model from the same start; 1 when the data, traces and outputs agree */
static int compare_one(const Call *c, const char *tag)
{
    Out p, m;
    size_t i;
    memcpy(g_start, g_ds, DS_SIZE);
    p = run(c, 0);
    memcpy(g_port_ds, g_ds, DS_SIZE);
    memcpy(g_ds, g_start, DS_SIZE);
    m = run(c, 1);
    if (p.ret != m.ret || p.o[0] != m.o[0] || p.o[1] != m.o[1] || p.o[2] != m.o[2]) {
        if (!g_quiet)
            printf("   %s %s: outputs port %d %d %d %d model %d %d %d %d\n", tag, g_fname[c->fn], p.ret, p.o[0], p.o[1],
                   p.o[2], m.ret, m.o[0], m.o[1], m.o[2]);
        return 0;
    }
    if (g_len[0] != g_len[1] || memcmp(g_log[0], g_log[1], g_len[0]) != 0) {
        if (!g_quiet) {
            size_t k = 0, line = 0;
            while (k < g_len[0] && k < g_len[1] && g_log[0][k] == g_log[1][k]) {
                if (g_log[0][k] == '\n')
                    line++;
                k++;
            }
            printf("   %s %s: traces differ at line %u\n", tag, g_fname[c->fn], (unsigned)line);
        }
        return 0;
    }
    for (i = 0; i < DS_SIZE; i++)
        if (g_port_ds[i] != g_ds[i]) {
            if (!g_quiet)
                printf("   %s %s: data differ at 0x%X: port %02X model %02X\n", tag, g_fname[c->fn],
                       (unsigned)(DS_BASE + i), g_port_ds[i], g_ds[i]);
            return 0;
        }
    return 1;
}

/* ---- random states ----------------------------------------------------------------------------------------------- */

static uint32_t g_rs = 12345u;
static uint32_t rnd(uint32_t n)
{
    g_rs ^= g_rs << 13;
    g_rs ^= g_rs >> 17;
    g_rs ^= g_rs << 5;
    return n ? g_rs % n : 0;
}
static int32_t rr(int32_t lo, int32_t hi) { return lo + (int32_t)rnd((uint32_t)(hi - lo + 1)); }

#define NAT(n) (0xF14CCu + 199u * (uint32_t)(n))

static void random_state(int small_hands)
{
    int j, i;
    memcpy(g_ds, g_base[rnd((uint32_t)g_nbase)], DS_SIZE);
    for (j = 0; j <= 9; j++) {
        uint32_t r = NAT(j);
        if (j >= 1)
            setD8(r + 0x9Au, (uint8_t)(rnd(4) == 0 ? 0 : rnd(3) == 0 ? 3 : 2));
        for (i = 1; i <= 0x12; i++)
            setD8(r + (uint32_t)i, (uint8_t)(rnd(small_hands ? 4 : 2) == 0 ? rr(1, small_hands ? 2 : 4) : 0));
        setD8(r + 0x13u, (uint8_t)rr(0, 2));
        {
            uint32_t odds = rnd(3) == 0 ? 12u : 3u;   /* few advances (the group goal cuts) or many */
            for (i = 0; i < 24; i++)
                setD8(r + 0x24u + (uint32_t)i, (uint8_t)(rnd(odds) == 0));
        }
        setD8(r + 0x3Eu, (uint8_t)rr(0, 40));
        setD8(r + 0x3Fu, (uint8_t)rr(1, 3));
        setD8(r + 0x51u, (uint8_t)rr(0, 9));
        setD8(r + 0x5Cu, (uint8_t)rr(0, 16));
        setD8(r + 0x98u, (uint8_t)rr(0, 55));
        setD8(r + 0x99u, (uint8_t)rr(0, 55));
    }
    for (i = 0; i < 24; i++) {
        setD8(0xF67C4u + (uint32_t)i, (uint8_t)rr(0, 2));
        setD32(0xF62ECu + 4u * (uint32_t)i, (uint32_t)rr(-100, 400));
        setD32(0xF6764u + 4u * (uint32_t)i, (uint32_t)rr(0, 150));
    }
    for (i = 0; i < 100; i++)
        setD32(0xF52E0u + 4u * (uint32_t)i, (uint32_t)rr(-700, 700));
    setD32(0xF52D4u, (uint32_t)rr(0, 9));
    setD32(0xF40ECu, (uint32_t)rr(1, 9));
    setD32(0xF40F0u, (uint32_t)rr(3, 9));
    setD32(0xF4898u, (uint32_t)rr(1, 55));
    for (i = 0; i < 100; i++)
        setD8(0x109E24u + (uint32_t)i, (uint8_t)(rnd(5) == 0));
    for (i = 0; i < 10; i++)
        setD8(0xF5934u + (uint32_t)i, (uint8_t)rr(-1, 2));
    for (i = 0; i < 9 * 85; i++)
        if (rnd(3) == 0)
            setD8(0xBCCE2u + (uint32_t)i, (uint8_t)rr(-1, 6));
    setD32(0xF68B4u, (uint32_t)rr(1, 6));
    for (i = 0; i < 6; i++) {
        static const uint8_t mg[5] = { 4, 12, 11, 16, 18 };
        setD8(0xF68B8u + (uint32_t)i, mg[rnd(5)]);
    }
    setD32(0xF68B0u, (uint32_t)rr(0, 5));
    setD8(0xF6754u, (uint8_t)rnd(2));
    setD32(0xF6748u, (uint32_t)rr(0, 20));
    setD32(0xF674Cu, (uint32_t)rr(0, 30));
    setD32(0xCAC5Cu, (uint32_t)rnd(0xFFFFFFFFu));
    setD16(0xF40BAu, (uint16_t)rnd(2));
}

static void random_dialog(void)
{
    uint32_t dlg = 0xF634Cu;
    int i, c = rr(0, 30);
    setD16(dlg, (uint16_t)c);
    for (i = 0; i < c; i++) {
        uint32_t r = dlg + 2u + 20u * (uint32_t)i;
        static const int ids[] = { 5, 3, 4, 6, 7, 8, 0x2A, 0x41, 0xA6, 0x8E, 0x95, 0xA5, 0x75, 0xA7 };
        setD16(r + 0x0Eu, (uint16_t)(rnd(3) == 0 ? rr(0x8E, 0xA5) : ids[rnd(14)]));
        setD8(r + 0x10u, (uint8_t)rnd(2));
        setD8(r + 0x12u, (uint8_t)rnd(2));
    }
    setD32(0xF8E78u, dlg);
}

static int part_b(int count, int quiet)
{
    int k, bad = 0, keepq = g_quiet;
    char tag[64];
    g_quiet = quiet;
    for (k = 0; k < count; k++) {
        Call c;
        int fn = k % NFN;
        memset(&c, 0, sizeof c);
        c.fn = fn;
        random_state(fn == F1B59C || fn == F1BE08);
        c.a[0] = rr(1, 9);
        g_scr = rnd(0xFFFFFFFFu);
        switch (fn) {
        case F1B45C:
            c.a[0] = rr(-1, 23);
            break;
        case F177EC:
            if (rnd(2))
                setD32(0xF52D4u, (uint32_t)c.a[0] % 9u + 1u);
            break;
        case F1BE08:
            c.a[1] = rr(-5, 60);
            c.a[2] = (int)rnd(2);
            if (rnd(4) == 0) {
                int i;
                for (i = 1; i <= 0x12; i++)
                    setD8(0xF14CCu + (uint32_t)i, 0);   /* no commodity: the stale-word fix */
            } else if (rnd(2)) {
                int i;   /* wider hands, a small treasury and a large shortfall: the walk back and the subsets */
                for (i = 1; i <= 0x12; i++)
                    setD8(0xF14CCu + (uint32_t)i, (uint8_t)(rnd(2) ? rr(1, 3) : 0));
                setD32(0xF674Cu, (uint32_t)rr(0, 4));
                setD32(0xF6748u, (uint32_t)rr(0, 60));
                c.a[1] = rr(5, 150);
            }
            if (rnd(2))
                setD8(0xF14CCu + 0x29u, 1);
            break;
        case F1B59C:
            c.a[1] = rr(0, 50);
            if (rnd(2)) {
                int i;   /* high commodities with no exact fit: the late tolerance steps */
                uint32_t r = NAT(c.a[0]);
                for (i = 1; i <= 0x12; i++)
                    setD8(r + (uint32_t)i, 0);
                setD8(r + 17u, (uint8_t)rr(1, 2));
                setD8(r + 18u, (uint8_t)rnd(2));
                setD8(r + (uint32_t)rr(1, 16), (uint8_t)rr(1, 3));
                setD8(r + (uint32_t)rr(1, 16), (uint8_t)rr(1, 2));
                setD8(r + 0x3Eu, (uint8_t)rr(0, 2));
                c.a[1] = rr(1, 60);
            }
            break;
        case F19700: {
            int i;
            random_dialog();
            c.a[1] = rr(0, 3);
            c.list = 0xF6E12u + 10u * (uint32_t)c.a[0];
            for (i = 0; i < 5; i++)
                setD16(c.list + 2u * (uint32_t)i, (uint16_t)rr(0, 23));
            break;
        }
        default:
            break;
        }
        snprintf(tag, sizeof tag, "B%d #%d%s", fn, k, g_original ? " orig" : "");
        if (!compare_one(&c, tag)) {
            bad++;
            if (!quiet)
                check(0, tag);
        } else if (!quiet)
            check(1, tag);
    }
    g_quiet = keepq;
    return bad;
}

/* ---- part A: hand vectors ------------------------------------------------------------------------------------------ */

static void clear_hand(int n)
{
    int i;
    for (i = 1; i <= 0x13; i++)
        setD8(NAT(n) + (uint32_t)i, 0);
}

static void part_a(void)
{
    int r;
    int32_t ex = 0x7FFFFFFF, mk = 0, xa = 0;
    char buf[300], want[300];
    Call c;

    memcpy(g_ds, g_base[0], DS_SIZE);
    g_cur = 0;
    g_srng = 1;
    /* A1 0x1B45C: score = cost + 50: one draw; seed 0 -> rand 0 -> 0 < 50: 1 */
    setD32(0xF62ECu + 4u * 3u, (uint32_t)(S16(0xBD480u + 6u) + 50));
    setD32(0xCAC5Cu, 0);
    r = f1B45C(3);
    check(r == 1 && D32(0xCAC5Cu) == 1u, "A1 0x1B45C draw, buys");
    setD32(0xF62ECu + 4u * 3u, (uint32_t)S16(0xBD480u + 6u));   /* score == cost: no draw */
    r = f1B45C(3);
    check(r == 0 && D32(0xCAC5Cu) == 1u, "A1 0x1B45C no draw");

    /* A2 0x1BE08 / 0x1B59C: commodity 1 x2 (4), 3 x1 (2), treasury 0 */
    clear_hand(0);
    setD8(0xF14CCu + 0x29u, 0);
    setD8(0xF14CCu + 1u, 2);
    setD8(0xF14CCu + 3u, 1);
    setD32(0xF674Cu, 0);
    setD32(0xF6748u, 0);
    r = f1BE08(1, 6, &ex, &mk, &xa, 1);
    check(r == 1 && xa == 3 && D32(0xF6D4Cu) == 2u && S32(0xF68CCu) == 4 && S32(0xF68ECu) == 2, "A2 0x1BE08 exact fit");
    ex = 0x7FFFFFFF;
    r = f1BE08(1, 5, &ex, &mk, &xa, 0);
    check(r == 0 && ex == 1 && mk == 3, "A2 0x1BE08 excess 1, mask 3");
    clear_hand(5);
    setD8(NAT(5) + 0x29u, 0);
    setD8(NAT(5) + 1u, 2);
    setD8(NAT(5) + 3u, 1);
    setD8(NAT(5) + 0x3Eu, 0);
    r = f1B59C(5, 5);
    check(r == 1 && D8(0xF14CDu) == 2 && D8(0xF14CFu) == 1 && D32(0xF6750u) == 0, "A2 0x1B59C pays with both, F6750 0");
    /* A3 no commodity: the fixed e = 0: excess = min(shortfall, treasury) - need, mask 1, no walk */
    clear_hand(0);
    setD32(0xF674Cu, 20);
    setD32(0xF6748u, 9);
    ex = 0x7FFFFFFF;
    mk = 0;
    r = f1BE08(1, 4, &ex, &mk, &xa, 0);
    check(r == 0 && ex == 5 && mk == 1 && D32(0xF6D4Cu) == 0, "A3 0x1BE08 no commodity: e = 0");

    /* A4 0x177EC: three candidates 0, 1, 2 scored 100, 50, 10; goals off; n holds 9 cards (kept) */
    memcpy(g_ds, g_base[0], DS_SIZE);
    {
        int i, n = 2;
        uint32_t row = 0xBCCE2u + 5u * (uint32_t)S8(NAT(n) + 0x5Cu) + 85u * (uint32_t)(n - 1);
        for (i = 0; i < 24; i++) {
            setD8(0xF67C4u + (uint32_t)i, (uint8_t)(i < 3 ? 2 : 1));
            setD8(NAT(n) + 0x24u + (uint32_t)i, 0);
        }
        setD32(0xF62ECu, 10);
        setD32(0xF62F0u, 100);
        setD32(0xF62F4u, 50);
        setD8(row, 0xFF);
        setD8(row + 2u, 0xFF);
        setD32(0xF52D4u, (uint32_t)n);
        clear_hand(n);
        setD8(NAT(n) + 1u, 9);
        setD32(0xCAC5Cu, 0);   /* rand 0: r = 0 -> the first, advance 1 */
        g_srng = 1;
        r = f177EC(n);
        check(r == 1 && S32(0xF7C9Cu) == 3 && S32(0xF7CA0u) == 1 && S32(0xF7CA4u) == 2 && S32(0xF7CA8u) == 0
              && D32(0xCAC5Cu) == 1u, "A4 0x177EC sorted list, weighted pick, one draw");
        /* weights 166, 66, 10 (total 242): a seed whose draw lands past 232 picks the third */
        {
            uint32_t s;
            for (s = 1; s < 200000u; s++) {
                uint32_t nx = s * 0x15A4E35u + 1u, v = (nx >> 16) & 0x7FFFu;
                if (((v * 242u) >> 15) >= 232u)
                    break;
            }
            setD32(0xCAC5Cu, s);
            r = f177EC(n);
            check(r == 0, "A4 0x177EC third by weight (166 + 66 <= r)");
        }
        /* the special case: a human leader with Military and attitude > 500: the top, no draw */
        setD32(0xF52D4u, 3);
        setD8(NAT(3) + 0x9Au, 3);
        setD8(NAT(3) + 0x35u, 1);
        setD32(0xF52E0u + 40u * (uint32_t)n + 12u, 501);
        setD32(0xCAC5Cu, 77);
        r = f177EC(n);
        check(r == 1 && D32(0xCAC5Cu) == 77u, "A4 0x177EC human leader: the top, no draw");
    }

    /* A5 0x19700: the texts */
    {
        static const struct { int cnt, orig; const char *fmt; } v[] = {
            { 1, 0, "%s has acquired \n%s." },
            { 2, 0, "%s has acquired \n%s and \n%s." },
            { 3, 0, "%s has acquired \n%s, \n%s and \n%s." },
            { 3, 1, "%s has acquired \n%s, \n%s, \n%s and \n." },
        };
        int q;
        for (q = 0; q < 4; q++) {
            uint32_t list = 0xF6E12u + 10u * 4u;
            const char *nm = DSTR(D32(0xC768Cu + 16u));
            const char *a0, *a1, *a2;
            char *p;
            memcpy(g_ds, g_base[0], DS_SIZE);
            setD16(list, 0);
            setD16(list + 2u, 15);
            setD16(list + 4u, 10);
            a0 = DSTR(D32(0xC7DC4u));
            a1 = DSTR(D32(0xC7DC4u + 60u));
            a2 = DSTR(D32(0xC7DC4u + 40u));
            if (v[q].cnt == 1)
                snprintf(want, sizeof want, v[q].fmt, nm, a0);
            else if (v[q].cnt == 2)
                snprintf(want, sizeof want, v[q].fmt, nm, a0, a1);
            else
                snprintf(want, sizeof want, v[q].fmt, nm, a0, a1, a2);
            g_original = v[q].orig;
            g_cur = 0;
            g_len[0] = 0;
            g_log[0][0] = 0;
            g_srng = 5;
            g_steps = 0;
            random_dialog();
            f19700(4, v[q].cnt, list);
            g_original = 0;
            snprintf(buf, sizeof buf, "fgf_width [%s]", want);
            p = strstr(g_log[0], buf);
            snprintf(buf, sizeof buf, "A5 0x19700 text, %d advances%s", v[q].cnt, v[q].orig ? " (--original)" : "");
            check(p != NULL, buf);
        }
    }
    (void)c;
    printf("A. %d checks, %d fail\n", g_tests, g_fail);
}

/* ---- part C: the real saves ----------------------------------------------------------------------------------------- */

static void states_as_17EDC(int n)
{
    int i, j;
    int32_t hand = 0, tr_ = S8(NAT(n) + 0x3Eu);
    for (i = 1; i <= 0x12; i++) {
        int32_t c = S8(NAT(n) + (uint32_t)i);
        hand += ((i + 1) >> 1) * c * c;
    }
    for (i = 0; i < 24; i++) {
        int32_t cr = 0;
        for (j = 0; j < 24; j++)
            if (D8(NAT(n) + 0x24u + (uint32_t)j) != 0)
                cr += S8(0xBD240u + 24u * (uint32_t)j + (uint32_t)i);
        setD32(0xF6764u + 4u * (uint32_t)i, (uint32_t)cr);
        if (D8(NAT(n) + 0x24u + (uint32_t)i) != 0)
            setD8(0xF67C4u + (uint32_t)i, 1);
        else
            setD8(0xF67C4u + (uint32_t)i, (uint8_t)(hand + tr_ + cr >= S16(0xBD480u + 2u * (uint32_t)i) ? 2 : 0));
    }
}

static void part_c(int base, const char *name, int extra)
{
    int n;
    char tag[80];
    printf("   %s%s:", name, extra ? " (treasury + 60, a dealt hand)" : "");
    for (n = 1; n <= 9; n++) {
        Call c;
        int ch;
        if (S8(NAT(n) + 0x9Au) != 2)
            continue;
        memcpy(g_ds, g_base[base], DS_SIZE);
        if (extra) {   /* R4: no cities, so no cards and no treasury yet: the real advances with money and a dealt hand */
            int q;
            setD8(NAT(n) + 0x3Eu, (uint8_t)(S8(NAT(n) + 0x3Eu) + extra));
            for (q = 1; q <= 0x12; q++)
                setD8(NAT(n) + (uint32_t)q, (uint8_t)((n + q) % 5 == 0 ? 2 : (n + q) % 7 == 0 ? 1 : 0));
        }
        states_as_17EDC(n);
        memset(&c, 0, sizeof c);
        c.a[0] = n;
        g_scr = (uint32_t)n;
        c.fn = F1A888;
        snprintf(tag, sizeof tag, "C %s n%d 1A888", name, n);
        check(compare_one(&c, tag), tag);
        f1A888(n);
        c.fn = F177EC;
        snprintf(tag, sizeof tag, "C %s n%d 177EC", name, n);
        check(compare_one(&c, tag), tag);
        ch = f177EC(n);
        printf(" n%d:%d(%d cand, top %d)", n, ch, S32(0xF7C9Cu), S32(0xF7C9Cu) > 0 ? S32(0xF7CA0u) : -1);
        if (ch < 0 && S32(0xF7C9Cu) > 0)
            ch = S32(0xF7CA0u);   /* the top candidate's payment, to exercise 0x1B59C on the real hands */
        if (ch >= 0) {
            int32_t cost = S16(0xBD480u + 2u * (uint32_t)ch), cr = S32(0xF6764u + 4u * (uint32_t)ch);
            c.fn = F1B59C;
            c.a[1] = cost - (cost > cr ? cr : cost);
            snprintf(tag, sizeof tag, "C %s n%d 1B59C", name, n);
            check(compare_one(&c, tag), tag);
            printf("(need %d)", c.a[1]);
        }
    }
    printf("\n");
}

/* ---- part D: mutations ----------------------------------------------------------------------------------------------- */

static void part_d(void)
{
    int i, bad, caught = 0, nm = 9;
    char buf[100];
    uint32_t keep = g_rs;
    for (i = 1; i <= nm; i++) {
        g_mut = i;
        g_rs = 4242u + (uint32_t)i;
        g_original = 0;
        bad = part_b(6000, 1);
        snprintf(buf, sizeof buf, "D mutation %d caught (%d of 6000 checks differ)", g_mut, bad);
        printf("   %s\n", buf);
        if (bad > 0)
            caught++;
        check(bad > 0, buf);
    }
    g_mut = 0;
    g_rs = keep;
    printf("D. %d of %d mutations caught\n", caught, nm);
}

/* ---- main ------------------------------------------------------------------------------------------------------------ */

static int g_adjn[160];
static int16_t g_adj[160][32];

static int load_landadj(const char *gamedir)
{
    char path[1024];
    FILE *fp;
    uint8_t b[2], sz[4];
    int t, i, k;
    uint8_t *blk;
    uint32_t off, h;

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
    h = load_heap_alloc(2u * (160u + 160u * 32u));
    blk = load_heap_ptr(h, 2u * (160u + 160u * 32u));
    if (blk == NULL)
        return 0;
    off = 0;
    for (t = 0; t < 160; t++) {
        setD32(0xF8E7Cu + 4u * (uint32_t)t, h + off);
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

static int load_save(const char *path, int slot, uint8_t *init)
{
    FILE *fp = fopen(path, "rb");
    int16_t ver = 0;
    char desc[64];
    memcpy(g_ds, init, DS_SIZE);
    if (fp == NULL) {
        printf("cannot open %s\n", path);
        return 0;
    }
    f5F05C(&ver, desc, fp);
    fclose(fp);
    memcpy(g_base[slot], g_ds, DS_SIZE);
    printf("   %s: version %d \"%.40s\"\n", path, ver, desc);
    return 1;
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:/work/advciv_run";
    const char *playdir = argc > 2 ? argv[2] : "C:/work/advciv_play";
    char cwd[400], path[400];
    uint8_t *init;

    if (state_init(gamedir) != 0)
        return 1;
    if (getcwd(cwd, sizeof cwd) == NULL)
        cwd[0] = 0;
    if (chdir(gamedir) == 0) {
        f12394();
        (void)chdir(cwd);
    }
    if (!load_landadj(gamedir)) {
        printf("cannot read %s/LANDADJ.DAT\n", gamedir);
        return 1;
    }
    g_log[0] = malloc(LOGSZ);
    g_log[1] = malloc(LOGSZ);
    g_start = malloc(DS_SIZE);
    g_port_ds = malloc(DS_SIZE);
    g_base[0] = malloc(DS_SIZE);
    g_base[1] = malloc(DS_SIZE);
    init = malloc(DS_SIZE);
    if (!g_log[0] || !g_log[1] || !g_start || !g_port_ds || !g_base[0] || !g_base[1] || !init)
        return 1;
    memcpy(init, g_ds, DS_SIZE);
    snprintf(path, sizeof path, "%s/SAVGAMEA.DAT", playdir);
    if (!load_save(path, 0, init))
        return 1;
    snprintf(path, sizeof path, "%s/savgame0.dat", gamedir);
    if (!load_save(path, 1, init))
        return 1;
    g_nbase = 2;

    part_a();

    part_c(0, "SAVGAMEA.DAT", 0);
    part_c(1, "savgame0.dat", 0);
    part_c(0, "SAVGAMEA.DAT", 60);
    part_c(1, "savgame0.dat", 60);
    printf("C. %d fail so far\n", g_fail);

    part_b(30000, 0);
    g_original = 1;
    part_b(12000, 0);
    g_original = 0;
    printf("B. %d fail so far\n", g_fail);

    part_d();

    printf("%d checks, %d fail\n", g_tests, g_fail);
    return g_fail == 0 ? 0 : 1;
}
