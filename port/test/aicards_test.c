/* aicards_test [GAMEDIR [PLAYDIR]]: unit checks for aicards.c (the computers' parts of CARDS and CITY) that need no
   game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init; default C:\work\advciv_run, only read), terrflag.dat (f12394:
   the population limits and territory flags are not in the save), LANDADJ.DAT (the land lists, parsed into one heap
   block with a handle per territory at 0xF8E7C + 4t, as the game keeps them) and savgame0.dat (R4 at MOVEMENT);
   PLAYDIR holds SAVGAMEA.DAT (the R4 autosave: seven computers; default C:\work\advciv_play, only read). The saves are
   read with saveload.c's f5F05C as the game does.

   aicards.c is linked as it is (it calls no unported routine). Every routine it calls is a stub here that appends one
   line to a trace; those with data effects apply them as their headers describe: the cards (rules.h's f76E50, f771F8;
   info.h's f76F14, f194E4), the cities (rules.h's f78E74, f78FEC in a reduced form: the state bytes and the city list),
   the tokens (move.h's f57CE8: the area's tokens), f17EDC (adv.h: a seeded answer and a write of int16 [0xF4092]); the
   UI kit, fg.c and kit.c's f79270 only trace; f78BA4 and f7BA64 answer from a seeded script. f76F14 also counts its
   calls and cuts a run after 4,000 ("CUT", the same on both sides): the hang of 0x6EDA8 with --original.

   The model is a second implementation of the six routines written here from the listing (not from aicards.c), in the
   listing's own terms (dword [rec + k - 3] sar 24 card reads, the 16-byte table rows of 0x6F078 at 0x1101AC..0x1101B4,
   0x5DE5C's pair arrays as 8-byte rows with the listing's copy loops). Each check runs the port, keeps its data object,
   trace and return value, restores the start state, runs the model, and compares all three. Parts:
     A. hand vectors with the expected values written out: 0x6F078's ranking (counts with the city bonus, values with
        and without Mining) and its empty-hand return [0x11019C]; 0x6DE04's variant tiers and stack-8 purchase; 0x6EDA8
        on a calamities-only hand (fixed: ends; --original: hangs, or writes at a garbage id); 0x5DE5C's second-pass
        row n (risk 8) and its order; 0x333E8's state bits and texts.
     B. random differential checks: each routine called directly on random states (hands with calamities, stacks,
        piles, treasuries, cities on the nation's own territories, the turn bytes, AI flags, toggles, variants,
        [0x11019C]), both modes.
     C. the real saves: every routine for every computer of the two R4 saves (cities placed on their areas and a
        dealt hand, as R4 has none yet), port against model.
     D. mutations: deliberately broken copies of the model must each be caught by part B's checks.
   Prints each failure, the counts per part, and exits 0 when all pass.

   Build (from the repo root):
     clang -std=c11 -Wall -Wextra -Iport/src port/test/aicards_test.c port/src/aicards.c port/src/saveload.c
       port/src/load.c port/src/state.c port/src/le_image.c -o aicards_test.exe
   (f78AE4, f76F70, f44CAC, f3A780 are named by load.c only: stubs here that stop the run if reached.)
   Result (21 Nov): A 12, C 84, B 42,000 checks (30,000 fixed, 12,000 --original) and the 5,000 rankings as one, 0 fail; D 9 of 9
   mutations caught. */

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <direct.h>
#define chdir _chdir
#define getcwd _getcwd

#include "adv.h"
#include "aicards.h"
#include "fg.h"
#include "game.h"
#include "info.h"
#include "kit.h"
#include "move.h"
#include "rules.h"
#include "saveload.h"
#include "state.h"
#include "ui.h"

void f12394(void);   /* load.c: terrflag.dat into the territory records */

#define NAT(n)   (0xF14CCu + 199u * (uint32_t)(n))
#define STK(s)   (0xF06D0u + 25u * (uint32_t)(s))
#define PIL(s)   (0xF07B4u + 25u * (uint32_t)(s))
#define TSA(t)   (0xE2C18u + 3u * (uint32_t)(t))
#define AREA(r)  (0xE31B8u + 69u * (uint32_t)(r))

/* ---- traces ---------------------------------------------------------------------------------------------------- */

#define LOGSZ (1u << 21)
static char *g_log[2];
static size_t g_len[2];
static int g_cur;
static int g_mut;

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

/* ---- stubs ---------------------------------------------------------------------------------------------------------- */

int f76E50(int s, int n)
{
    uint32_t st = STK(s);
    int16_t c = S16(st);
    int8_t card;
    tr("f76E50 %d %d", s, n);
    if (c == 0)
        return 0;
    card = S8(st + 5u + (uint32_t)c);
    setD16(st, (uint16_t)(c - 1));
    if (card > 18) {
        uint32_t r = NAT(n);
        int8_t k = S8(r + 0x13u);
        setD8(r + 0x14u + (uint32_t)k, (uint8_t)card);
        setD8(r + 0x13u, (uint8_t)(k + 1));
    } else
        setD8(NAT(n) + (uint32_t)card, (uint8_t)(D8(NAT(n) + (uint32_t)card) + 1u));
    return card;
}

void f771F8(int c)
{
    int s = c <= 18 ? (c - 1) / 2 : S8(0xCA9C1u + (uint32_t)c);
    uint32_t p = PIL(s);
    int16_t k = S16(p);
    tr("f771F8 %d", c);
    if (k >= 0 && k < 19)
        setD8(p + 6u + (uint32_t)k, (uint8_t)c);
    setD16(p, (uint16_t)(k + 1));
}

void f78E74(int t, int n)
{
    uint32_t r = NAT(n);
    int8_t k = S8(r + 0x51u);
    tr("f78E74 %d %d", t, n);
    setD8(TSA(t) + 2u, (uint8_t)(D8(TSA(t) + 2u) | 1u));
    setD8(TSA(t), (uint8_t)n);
    setD8(TSA(t) + 1u, D8(0xF40FCu));
    if (k >= 0 && k < 9)
        setD8(r + 0x52u + (uint32_t)k, (uint8_t)(t + 1));
    setD8(r + 0x51u, (uint8_t)(k + 1));
}

void f78FEC(int t, int n)
{
    uint32_t r = NAT(n);
    int k = S8(r + 0x51u), i, j;
    tr("f78FEC %d %d", t, n);
    setD8(TSA(t) + 2u, (uint8_t)(D8(TSA(t) + 2u) & 0xFEu));
    setD8(TSA(t), 0);
    setD8(TSA(t) + 1u, 0xFF);
    for (i = 0; i < k && i < 9; i++)
        if (D8(r + 0x52u + (uint32_t)i) == (uint8_t)(t + 1)) {
            for (j = i; j < k - 1 && j < 8; j++)
                setD8(r + 0x52u + (uint32_t)j, D8(r + 0x53u + (uint32_t)j));
            setD8(r + 0x52u + (uint32_t)(k - 1 < 8 ? k - 1 : 8), 0);
            setD8(r + 0x51u, (uint8_t)(k - 1));
            break;
        }
}

void f57CE8(int t, int n, int flag, int k)
{
    tr("f57CE8 %d %d %d %d", t, n, flag, k);
    if (k > 0)
        setD8(AREA(t + 1) + 0x0Eu + (uint32_t)n, (uint8_t)(D8(AREA(t + 1) + 0x0Eu + (uint32_t)n) - (uint8_t)k));
}

int f76F14(int n)
{
    int i, s = 0;
    for (i = 1; i <= 0x13; i++)
        s += S8(NAT(n) + (uint32_t)i);
    tr("f76F14 %d = %d", n, s);
    step();
    return s;
}

void f194E4(int n) { tr("f194E4 %d", n); setD32(0xF68B0u, 0); }

int f17EDC(int n, int budget, int draw)
{
    int r = srnd(3) != 0;
    tr("f17EDC %d %d %d = %d", n, budget, draw, r);
    if (r)
        setD16(0xF4092u, 1);
    return r;
}

void f79270(int wait, int n, int t, const char *text, int anim) { tr("f79270 %d %d %d [%s] %d", wait, n, t, text, anim); }
int f7BA64(void) { int r = srnd(4) == 0; tr("f7BA64 %d", r); step(); return r; }
int f78BA4(uint32_t dlg, int map)
{
    static const int codes[] = { -100, -100, -3, 7, -1 };
    int r = codes[srnd(5)];
    tr("f78BA4 %X %d = %d", (unsigned)dlg, map, r);
    step();
    return r;
}
void f78A20(int t) { tr("f78A20 %d", t); }
void f3A044(int x, int y, const char *s) { tr("f3A044 %d %d [%s]", x, y, s); }
int f3A42C(int x, int y, int w, uint32_t align, const char *text, int32_t *maxw, int draw)
{
    tr("f3A42C %d %d %d %u [%s] %d", x, y, w, (unsigned)align, text, draw);
    *maxw = w;
    return y + 15;
}
void f3AF8C(uint32_t spr, int x, int y) { tr("f3AF8C %X %d %d", (unsigned)spr, x, y); }
void f39F90(int font) { tr("f39F90 %d", font); }
void f3A0B0(int x, int y, int w, const char *s) { tr("f3A0B0 %d %d %d [%s]", x, y, w, s); }
int fg_setcolor(int c) { tr("fg_setcolor %d", c); return 0; }
void f78884(void) { tr("f78884"); }
void f788AC(void) { tr("f788AC"); }
void f3CD6C(int8_t pic) { tr("f3CD6C %d", pic); }
int fg_vbcopy(int a, int b, int c, int d, int e, int f, int g, int h) { tr("fg_vbcopy %d %d %d %d %d %d %d %d", a, b, c, d, e, f, g, h); return 0; }
int fg_vbopen(int h) { tr("fg_vbopen %d", h); return 0; }
void fg_vbclose(void) { tr("fg_vbclose"); }
void f3A18C(int x, int y, int w, int color, const char *s) { tr("f3A18C %d %d %d %d [%s]", x, y, w, color, s); }
void f3C0D8(uint32_t dlg, int id, int style) { tr("f3C0D8 %X %d %d", (unsigned)dlg, id, style); }
void f3BFBC(uint32_t dlg, int id, int en) { tr("f3BFBC %X %d %d", (unsigned)dlg, id, en); }
void f3B05C(uint32_t dlg) { tr("f3B05C %X", (unsigned)dlg); setD32(0xF8E78u, dlg); }
void f3CC14(void) { tr("f3CC14"); }

/* named by load.c, never reached */
int f78AE4(void) { printf("unexpected f78AE4\n"); exit(2); }
void f76F70(int n, uint8_t *c) { printf("unexpected f76F70 %d %p\n", n, (void *)c); exit(2); }
_Noreturn void f44CAC(void) { printf("unexpected f44CAC\n"); exit(2); }
void f3A780(const char *m) { printf("unexpected f3A780 %s\n", m); exit(2); }

/* ---- the model (from the listing) -------------------------------------------------------------------------------- */

static int32_t sar24(uint32_t a) { return (int32_t)D32(a) >> 24; }
static int32_t sar16(uint32_t a) { return (int32_t)D32(a) >> 16; }
static int32_t sd(uint32_t a) { return S32(a); }
static void wd(uint32_t a, int32_t v) { setD32(a, (uint32_t)v); }
static void bump(uint32_t a, int v) { if (ds_valid(a, 1)) setD8(a, (uint8_t)(D8(a) + (uint8_t)v)); }

static void m_f6DBAC(int p, int n)
{
    char b[52];
    int32_t v20 = sar24(0xC7u * (uint32_t)n + 0xF151Au), v1c, v18, v14;
    int8_t v10;
    snprintf(b, sizeof b, DSTR(0xB8CE1u), DSTR(D32(4u * (uint32_t)n + 0xC768Cu)));
    fg_setcolor(5);
    f3A044(0xF6, 0x62, b);
    wd(0x110AC8u, 0);
    for (v1c = 0; v1c < v20; v1c++)
        for (v18 = 0; v18 < sd(0x110AC4u); v18++) {
            v10 = (int8_t)f76E50(v1c, n);
            if (v10 == 0) {
                snprintf(b, sizeof b, DSTR(0xB8CF0u), v1c + 1);
                f78884();
                f3A42C((4 * v1c + v1c) * 8 - v1c + 0xEC, (((4 * v18 - v18) << 3) + v18) * 2 + 0x73, 0x22, 2, b, &v14, 1);
                f788AC();
            } else {
                f78884();
                f3AF8C(0xF4D14u, (4 * v1c + v1c) * 8 - v1c + 0xEC,
                       (((4 * v18 - v18) << 3) + v18) * (g_mut == 9 ? 1 : 2) + 0x6E);
                f788AC();
                wd(0x110AC8u, sd(0x110AC8u) + 1);
            }
        }
    f3AF8C(0xF4D14u, 0x5F, ((4 * p - p) << 4) + p + 0x24);
    f39F90(2);
    snprintf(b, sizeof b, "%d", sd(0x110AC8u));
    fg_setcolor(5);
    f3A0B0(0x60, ((4 * p - p) << 4) + p + 0x32, 0x1E, b);
    f39F90(1);
}

static int m_f6DE04(int n)
{
    int32_t v14 = 0, v10 = 0, v18 = 0;
    uint32_t vc = 0xF14CCu + 0xC7u * (uint32_t)n;
    if ((D8(0xE1624u) & 0x20u) != 0) {
        if (!(sar24(vc + 0x4Eu) > 3) && !(sar24(vc + 0x3Bu) < (g_mut == 1 ? 7 : 6))) {
            if (!(sar24(vc + 3u) < 2) || !(sar24(vc + 2u) < 2))
                v10 = 3;
            else if (!(sar24(vc + 1u) < 2) || !(sar24(vc) < 2))
                v10 = 2;
            else if (!((int8_t)D8(vc + 2u) < 2) || !((int8_t)D8(vc + 1u) < 2))
                v10 = 1;
        } else if (!(sar24(vc + 0x4Eu) > 5) && !(sar24(vc + 0x3Bu) < 0xA)) {
            if (!(sar24(vc + 7u) < 2) || !(sar24(vc + 6u) < 2))
                v10 = 5;
            else if (!(sar24(vc + 5u) < 2) || !(sar24(vc + 4u) < 2))
                v10 = 4;
        } else if (!(sar24(vc + 0x3Bu) < 0x12) && sar24(vc + 0x96u) < 0x12) {
            if (!(sar24(vc + 9u) < 2) || !(sar24(vc + 8u) < 2))
                v10 = 6;
            else if (!(sar24(vc + 0xBu) < 2) || !(sar24(vc + 0xAu) < 2))
                v10 = 7;
            else if (!(sar24(vc + 0xDu) < 2) || !(sar24(vc + 0xCu) < 2))
                v10 = 8;
            else if (!(sar24(vc + 0xFu) < 2) || !(sar24(vc + 0xEu) < 2))
                v10 = 9;
        }
        {
            int32_t edx = v10 - 1;
            if (sar16((uint32_t)edx + 8u * (uint32_t)(3 * edx) + 0xF06CEu) == 0)
                v10 = 0;
        }
    } else if (!(sar24(vc + 0x3Bu) < 0x12)) {
        f194E4(n);
        if (sar24(vc + 0x96u) < 2 * sar24(vc + 0x4Eu))
            goto L_6E0E8;
        if (D8((uint32_t)n + 0xF488Cu) != 0) {
            if (f17EDC(n, -1, 0) == 0)
                goto L_6E0E8;
            if (f17EDC(n, -1, 0) != 0) {
                if (!(f76F14(n) > 8))
                    goto L_6E0E8;
            }
        }
        if (!(sar24(vc + 0xFu) < 1) && (g_mut == 2 || !(sar24(vc + 0xEu) < 1)))
            goto L_6E0E8;
        if (sar24(vc + 0x4Eu) != 9)
            goto L_6E0EF;
        if (!(sar24(vc + 0x96u) < 0x19))
            goto L_6E0EF;
L_6E0E8:
        v10 = 9;
    }
L_6E0EF:
    if (v10 > 0 && !(sar24(vc + 0x3Bu) < v10 + v10))
        v14 = f76E50(v10 - 1, n);
    if (v14 != 0) {
        wd(4u * (uint32_t)n + 0xF590Cu, v10 - 1);
        setD8(vc + 0x3Eu, (uint8_t)(D8(vc + 0x3Eu) - (uint8_t)(v10 + v10)));
        setD8(vc + 0x99u, (uint8_t)(D8(vc + 0x99u) + (uint8_t)(v10 + v10)));
        v18 = (v10 << 8) + 1;
    }
    return v18;
}

static int m_f6F078(int n)
{
    uint32_t v14 = 0xF14CCu + 0xC7u * (uint32_t)n, ebx;
    int32_t v20, v28, v18, v1c, edx, eax;
    uint8_t t38[16];
    memset(DP(0x1101ACu), 0, 0x120);
    wd(0x1103ECu, 0);
    v20 = sar24(v14 + 0x4Eu);
    for (v28 = 0x12; v28 >= 1; v28--) {
        if (D8(v14 + (uint32_t)v28) == 0)
            continue;
        ebx = D32(0x1103ECu) << 4;
        wd(ebx + 0x1101ACu, v28);
        wd(ebx + 0x1101B4u, (int8_t)D8(v14 + (uint32_t)v28));
        if (g_mut == 4 ? (v28 + 1) / 2 < v20 : !((v28 + 1) / 2 > v20))
            wd(ebx + 0x1101B4u, sd(ebx + 0x1101B4u) + 1);
        v1c = 0;
        for (v18 = 0; v18 < 6; v18++)
            if (sar24((uint32_t)v18 + 0xBDD39u) == v28) {
                v1c = 1;
                break;
            }
        edx = (int8_t)D8(v14 + (uint32_t)v28);
        eax = edx;
        if (D8(v14 + 0x29u) != 0 && v1c != 0) {
            edx++;
            eax++;
        }
        edx *= eax;
        eax = ((v28 + 1) >> 1) * edx;
        wd((D32(0x1103ECu) << 4) + 0x1101B0u, eax);
        wd(0x1103ECu, sd(0x1103ECu) + 1);
    }
    for (v28 = sd(0x1103ECu) - 2; v28 >= 0; v28--)
        for (v18 = 0; v18 <= v28; v18++) {
            int gt = g_mut == 3 ? sd(((uint32_t)v18 << 4) + 0x1101C0u) >= sd(((uint32_t)v18 << 4) + 0x1101B0u)
                                : sd(((uint32_t)v18 << 4) + 0x1101C0u) > sd(((uint32_t)v18 << 4) + 0x1101B0u);
            if (gt) {
                memcpy(t38, DP(((uint32_t)v18 << 4) + 0x1101BCu), 16);
                memcpy(DP(((uint32_t)v18 << 4) + 0x1101BCu), DP(((uint32_t)v18 << 4) + 0x1101ACu), 16);
                memcpy(DP(((uint32_t)v18 << 4) + 0x1101ACu), t38, 16);
            }
        }
    return sd((D32(0x1103ECu) << 4) + 0x11019Cu);
}

static void m_f6EDA8(int n)
{
    char b[100];
    int32_t v20 = 0x4B, v28, v1c, v2c, v34, v30, v38, v24, lim;
    uint32_t v18 = 0xF14CCu + 0xC7u * (uint32_t)n;
    f3CD6C((int8_t)sar24(0xEE165u));
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, sd(0xF52CCu), sd(0xF52C8u));
    fg_vbopen(sd(0xF52C8u));
    f3A18C(0, 8, 0x280, 0xB0, DSTR(0xB8E60u));
    f3A18C(0, 0x1E, 0x280, 0x23, DSTR(D32(4u * (uint32_t)n + 0xC768Cu)));
    fg_setcolor(0xD);
    snprintf(b, sizeof b, DSTR(0xB8E75u), DSTR(D32(4u * (uint32_t)n + 0xC768Cu)));
    f3A044(v20 + 0xF, 0xBA, b);
    f3C0D8(0x110ADCu, 0x2A, 1);
    f3BFBC(0x110ADCu, 0x2A, 1);
    f3BFBC(0x110ADCu, 0x8D, 0);
    f3B05C(0x110ADCu);
    fg_vbclose();
    f3CC14();
    v28 = v20 + 5;
    for (v1c = 1; v1c <= 0x12; v1c++)
        setD8((uint32_t)v1c + 0xF14CCu, 0);
L_6EF15:
    if (f76F14(n) > 8) {
        v2c = m_f6F078(n);   /* the model's own 0x6F078 (review: the model no longer calls the port's) */
        if (!g_original && sd(0x1103ECu) == 0)
            goto L_6EFBF;
        if (v2c <= 0)
            goto L_6EF15;
        v34 = 0;
        lim = (int32_t)((uint32_t)((int32_t)((uint32_t)v2c + 1u) / 2) * 2u + 1u) + (g_mut == 5 ? 2 : 0);
        for (v1c = 0x12; !(lim > v1c); v1c--) {
            if (sar24((uint32_t)v1c + 0xF14C9u) == 0)
                continue;
            bump((uint32_t)v2c + 0xF14CCu, 1);
            bump(v18 + (uint32_t)v2c, -1);
            bump((uint32_t)v1c + 0xF14CCu, -1);
            bump(v18 + (uint32_t)v1c, 1);
            v34 = 1;
            break;
        }
        if (v34 != 0)
            goto L_6EF15;
        bump((uint32_t)v2c + 0xF14CCu, 1);
        bump(v18 + (uint32_t)v2c, -1);
        goto L_6EF15;
    }
L_6EFBF:
    for (v1c = 1; v1c <= 0x12; v1c++) {
        v30 = sar24((uint32_t)v1c + 0xF14C9u);
        for (v38 = 0; v38 < v30; v38++) {
            f771F8(v1c);
            bump((uint32_t)v1c + 0xF14CCu, -1);
            f3AF8C(((uint32_t)v1c << 3) + 0xF4C1Cu, v28, 0xC8);
            v28 += sd(0x11254Cu);
        }
    }
    do {
        if (f7BA64() == 0)
            v24 = f78BA4(0x110ADCu, 0);
        else {
            f78A20((int16_t)D16(0xF40BAu) == 0 ? 0xFA : 0x3E8);
            v24 = -1;
        }
    } while (v24 != -1);
}

static void m_f333E8(int t, int n, int k, int arch)
{
    char b[100];
    f78E74(t, n);
    f57CE8(t, n, 0, k);
    setD8(3u * (uint32_t)t + 0xE2C1Au, (uint8_t)(D8(3u * (uint32_t)t + 0xE2C1Au) & 0xFDu));
    setD8(3u * (uint32_t)t + 0xE2C1Au, (uint8_t)(D8(3u * (uint32_t)t + 0xE2C1Au) & 0x03u));
    snprintf(b, sizeof b, DSTR(arch == 0 ? 0xB3ACBu : 0xB3AE3u), DSTR(D32(4u * (uint32_t)n + 0xC768Cu)),
             DSTR(D32(4u * (uint32_t)t + 0xC77D8u)));
    if ((D8(0xF0980u) & 1u) != 0)
        f79270(-500, n, t, b, 0);
    else if (f7BA64() == 0)
        f79270(g_mut == 8 ? 2500 : 0x7D0, n, t, b, 0);
    else
        f79270((int16_t)D16(0xF40BAu) != 0 ? 0x5DC : 0x1F4, n, t, b, 0);
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

/* the frame's two pair arrays: A at [ebp-0xAAC] and B at [ebp-0x5AC], rows of {t, score} */
static int32_t mA[160][2], mB[160][2];

static void m_pass(int n, int second)
{
    uint32_t v1c = 0xF14CCu + 0xC7u * (uint32_t)n;
    int32_t v28 = n - 1, v30, v34, v14, v2c, v38, row, t;
    uint32_t v18;
    char b[100];
    int32_t s48[2];

    v30 = 0;
    for (v34 = 0; sar24(v1c + 0x4Eu) > v34; v34++) {
        uint32_t tt = D8(v1c + (uint32_t)v34 + 0x52u);
        int eq = D8(3u * (tt - 1u) + 0xE2C19u) == (uint32_t)sar16(0xF40FAu);
        if (second ? eq : !eq)
            continue;
        mA[v30][0] = (int32_t)tt - 1;
        v18 = D32(4u * (uint32_t)mA[v30][0] + 0xF8E7Cu);
        v2c = 0;
        for (v14 = 1; m_land(v18) >= v14; v14++) {
            int32_t r = m_land(2u * (uint32_t)v14 + v18) + 1;
            v2c += sar24((uint32_t)r * 69u + (uint32_t)v28 + 0xE31C4u);
        }
        mA[v30][1] = v2c;
        v30++;
    }
    if (v30 == 0)
        return;
    for (v34 = v30 - 2; v34 >= 0; v34--)
        for (v14 = 0; v14 <= v34; v14++) {
            uint8_t bl = (uint8_t)(D8(35u * (uint32_t)mA[v14 + 1][0] + 0xE1639u) << 2) >> 5;
            uint8_t al = (uint8_t)(D8(35u * (uint32_t)mA[v14][0] + 0xE1639u) << 2) >> 5;
            if (bl > al) {
                memcpy(s48, mA[v14 + 1], 8);
                memcpy(mA[v14 + 1], mA[v14], 8);
                memcpy(mA[v14], s48, 8);
            }
        }
    for (v34 = v30 - 2; v34 >= 0; v34--)
        for (v14 = 0; v14 <= v34; v14++)
            if (g_mut == 7 ? mA[v14 + 1][1] < mA[v14][1] : mA[v14 + 1][1] > mA[v14][1]) {
                memcpy(s48, mA[v14 + 1], 8);
                memcpy(mA[v14 + 1], mA[v14], 8);
                memcpy(mA[v14], s48, 8);
            }
    for (v34 = 0; v34 < v30; v34++)
        memcpy(mB[v34], mA[v34], 8);
    v38 = v30;
    v30 = 0;
    row = second && g_mut != 6 ? n : v28;
    for (v34 = 0; v34 < v38; v34++)
        if ((D8((uint32_t)row * 0x13Au + 2u * (uint32_t)mB[v34][0] + 0xF3580u) & 0x80u) != 0)
            memcpy(mA[v30++], mB[v34], 8);
    for (v34 = 0; v34 < v38; v34++)
        if ((D8((uint32_t)row * 0x13Au + 2u * (uint32_t)mB[v34][0] + 0xF3580u) & 0x80u) == 0)
            memcpy(mA[v30++], mB[v34], 8);
    for (v34 = 0; v34 < v30; v34++)
        memcpy(mB[v34], mA[v34], 8);
    v38 = v30;
    v30 = 0;
    for (v34 = 0; v34 < v38; v34++)
        if ((D8(35u * (uint32_t)mB[v34][0] + 0xE1639u) & 0x40u) != 0)
            memcpy(mA[v30++], mB[v34], 8);
    for (v34 = 0; v34 < v38; v34++)
        if ((D8(35u * (uint32_t)mB[v34][0] + 0xE1639u) & 0x40u) == 0)
            memcpy(mA[v30++], mB[v34], 8);
    t = mA[0][0];
    f78FEC(t, n);
    snprintf(b, sizeof b, DSTR(second ? 0xB5E8Du : 0xB5E7Fu), DSTR(D32(4u * (uint32_t)n + 0xC768Cu)),
             DSTR(D32(4u * (uint32_t)t + 0xC77D8u)));
    if ((D8(0xF0980u) & 0x10u) != 0)
        f79270(-500, n, t, b, 0);
    else
        f79270(0x9C4, n, t, b, 0);
    mA[0][0] = -999;   /* marks "reduced" for the caller */
}

static void m_f5DE5C(int n)
{
    uint32_t v1c = 0xF14CCu + 0xC7u * (uint32_t)n;
    if (2 * sar24(v1c + 0x4Eu) > sar24(v1c + 0x95u)) {
        mA[0][0] = 0;
        m_pass(n, 0);
        if (mA[0][0] == -999)
            return;
    }
    if (!(2 * sar24(v1c + 0x4Eu) > sar24(v1c + 0x95u)))
        return;
    m_pass(n, 1);
}

/* ---- the harness ------------------------------------------------------------------------------------------------- */

enum { F6DBAC, F6DE04, F6EDA8, F6F078, F333E8, F5DE5C, NFN };
static const char *g_fname[NFN] = { "6DBAC", "6DE04", "6EDA8", "6F078", "333E8", "5DE5C" };

typedef struct { int fn; int a[4]; } Call;

static uint8_t *g_start, *g_port_ds, *g_base[3];
static int g_nbase;
static int g_tests, g_fail, g_quiet;
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

static int run(const Call *c, int model)
{
    int r = 0;
    g_cur = model;
    g_len[model] = 0;
    g_log[model][0] = 0;
    g_srng = g_scr;
    g_steps = 0;
    g_cut_on = 1;
    if (setjmp(g_cut) != 0) {
        g_cut_on = 0;
        return -12345;
    }
    switch (c->fn) {
    case F6DBAC: if (model) m_f6DBAC(c->a[0], c->a[1]); else f6DBAC(c->a[0], c->a[1]); break;
    case F6DE04: r = model ? m_f6DE04(c->a[0]) : f6DE04(c->a[0]); break;
    case F6EDA8: if (model) m_f6EDA8(c->a[0]); else f6EDA8(c->a[0]); break;
    case F6F078: r = model ? m_f6F078(c->a[0]) : f6F078(c->a[0]); break;
    case F333E8: if (model) m_f333E8(c->a[0], c->a[1], c->a[2], c->a[3]); else f333E8(c->a[0], c->a[1], c->a[2], c->a[3]); break;
    case F5DE5C: if (model) m_f5DE5C(c->a[0]); else f5DE5C(c->a[0], NULL); break;
    default: break;
    }
    g_cut_on = 0;
    return r;
}

static int compare_one(const Call *c, const char *tag)
{
    int p, m;
    size_t i;
    memcpy(g_start, g_ds, DS_SIZE);
    p = run(c, 0);
    memcpy(g_port_ds, g_ds, DS_SIZE);
    memcpy(g_ds, g_start, DS_SIZE);
    m = run(c, 1);
    if (p != m) {
        if (!g_quiet)
            printf("   %s %s: returns port %d model %d\n", tag, g_fname[c->fn], p, m);
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

static uint32_t g_rs = 777u;
static uint32_t rnd(uint32_t n)
{
    g_rs ^= g_rs << 13;
    g_rs ^= g_rs >> 17;
    g_rs ^= g_rs << 5;
    return n ? g_rs % n : 0;
}
static int32_t rr(int32_t lo, int32_t hi) { return lo + (int32_t)rnd((uint32_t)(hi - lo + 1)); }

/* cities for n on territories of its own area list (or random ones), with the state bytes set */
static void place_cities(int n, int k)
{
    uint32_t r = NAT(n);
    int i, cnt = S8(r + 0x60u), have = 0;
    for (i = 0; i < k; i++) {
        int t = cnt > 0 && rnd(3) ? (int)D8(r + 0x61u + rnd((uint32_t)cnt)) - 1 : rr(0, 156);
        if (t < 0 || t > 156)
            t = rr(0, 156);
        setD8(r + 0x52u + (uint32_t)have, (uint8_t)(t + 1));
        setD8(TSA(t), (uint8_t)n);
        setD8(TSA(t) + 1u, (uint8_t)(rnd(2) ? D8(0xF40FCu) : rr(0, 5)));
        setD8(TSA(t) + 2u, (uint8_t)(1 | (rnd(2) << 1)));
        have++;
    }
    setD8(r + 0x51u, (uint8_t)have);
}

static void random_state(void)
{
    int j, i, s;
    memcpy(g_ds, g_base[rnd((uint32_t)g_nbase)], DS_SIZE);
    for (j = 1; j <= 9; j++) {
        uint32_t r = NAT(j);
        int calam = rnd(5) == 0;
        for (i = 1; i <= 0x12; i++)
            setD8(r + (uint32_t)i, (uint8_t)(calam ? 0 : rnd(3) == 0 ? rr(1, 3) : 0));
        setD8(r + 0x13u, (uint8_t)(calam ? rr(9, 11) : rr(0, 2)));
        setD8(r + 0x29u, (uint8_t)rnd(2));
        setD8(r + 0x3Eu, (uint8_t)rr(0, 40));
        setD8(r + 0x98u, (uint8_t)rr(0, 30));
        setD8(r + 0x99u, (uint8_t)rr(0, 40));
        place_cities(j, rr(0, 9));
    }
    for (s = 0; s < 9; s++) {
        int c = rnd(5) == 0 ? 0 : rr(1, 19);
        setD16(STK(s), (uint16_t)c);
        for (i = 0; i < 19; i++)
            setD8(STK(s) + 6u + (uint32_t)i, (uint8_t)(rnd(6) == 0 ? rr(0x13, 0x1E) : 2 * s + 1 + (int)rnd(2)));
        setD16(PIL(s), (uint16_t)rr(0, 5));
    }
    setD16(STK(-1), (uint16_t)rnd(2));
    setD32(0x110AC4u, (uint32_t)rr(1, 2));
    setD8(0xE1624u, (uint8_t)((D8(0xE1624u) & ~0x20u) | (rnd(2) ? 0x20u : 0u)));
    for (i = 0; i < 10; i++)
        setD8(0xF488Cu + (uint32_t)i, (uint8_t)rnd(2));
    setD8(0xF0980u, (uint8_t)rnd(256));
    setD16(0xF40BAu, (uint16_t)rnd(2));
    setD16(0xF40FCu, (uint16_t)rr(0, 5));
    setD32(0x11254Cu, (uint32_t)rr(10, 40));
    setD32(0x11019Cu, rnd(3) == 0 ? 0u : rnd(2) ? (uint32_t)rr(-3, 25) : rnd(0xFFFFFFFFu));
    for (i = 0; i < 9 * 157 + 160; i++)
        setD8(0xF3580u + 2u * (uint32_t)i, (uint8_t)(rnd(4) == 0 ? 0x80 : 0));
    setD8(0xEE168u, (uint8_t)rr(0, 20));
}

static int part_b(int count, int quiet)
{
    int k, bad = 0, keepq = g_quiet;
    char tag[64];
    g_quiet = quiet;
    for (k = 0; k < count; k++) {
        Call c;
        memset(&c, 0, sizeof c);
        c.fn = k % NFN;
        random_state();
        g_scr = rnd(0xFFFFFFFFu);
        c.a[0] = rr(1, 9);
        if (c.fn == F6DBAC) {
            c.a[1] = c.a[0];
            c.a[0] = rr(0, 8);
        } else if (c.fn == F333E8) {
            c.a[1] = c.a[0];
            c.a[0] = rr(0, 156);
            c.a[2] = rr(0, 12);
            c.a[3] = rnd(2) ? 0 : 0x63;
            setD8(NAT(c.a[1]) + 0x51u, (uint8_t)rr(0, 8));
        }
        snprintf(tag, sizeof tag, "B%d #%d%s", c.fn, k, g_original ? " orig" : "");
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

/* the model's 0x6F078 as a separate check as well (the model's 0x6EDA8 calls the model's own) */
static int part_b_rank(int count, int quiet)
{
    int k, bad = 0;
    for (k = 0; k < count; k++) {
        int n, p, m;
        random_state();
        n = rr(1, 9);
        memcpy(g_start, g_ds, DS_SIZE);
        g_cur = 0;
        p = f6F078(n);
        memcpy(g_port_ds, g_ds, DS_SIZE);
        memcpy(g_ds, g_start, DS_SIZE);
        m = m_f6F078(n);
        if (p != m || memcmp(g_port_ds, g_ds, DS_SIZE) != 0) {
            bad++;
            if (!quiet)
                printf("   B 6F078 #%d differs (port %d model %d)\n", k, p, m);
        }
    }
    if (!quiet)
        check(bad == 0, "B 0x6F078 port vs model");
    return bad;
}

/* ---- part A: hand vectors ------------------------------------------------------------------------------------------ */

static void hand0(int n)
{
    int i;
    for (i = 1; i <= 0x13; i++)
        setD8(NAT(n) + (uint32_t)i, 0);
}

static void part_a(void)
{
    int r, n = 3;
    uint32_t rec = NAT(n);
    Call c;
    char buf[200];

    memcpy(g_ds, g_base[0], DS_SIZE);
    g_cur = 0;
    g_srng = 3;
    /* A1 0x6F078: cards 1 x1, 4 x1, 18 x1; cities 2; Mining off: values 1, 2, 9 -> order 18, 4, 1; returns 1 */
    hand0(n);
    setD8(rec + 1u, 1);
    setD8(rec + 4u, 1);
    setD8(rec + 18u, 1);
    setD8(rec + 0x51u, 2);
    setD8(rec + 0x29u, 0);
    r = f6F078(n);
    check(r == 1 && S32(0x1103ECu) == 3 && S32(0x1101ACu) == 18 && S32(0x1101B0u) == 9 && S32(0x1101B4u) == 1
          && S32(0x1101BCu) == 4 && S32(0x1101C0u) == 2 && S32(0x1101C4u) == 2 && S32(0x1101D4u) == 2,
          "A1 0x6F078 ranking without Mining");
    setD8(rec + 0x29u, 1);   /* Mining: 4 -> (1+1)^2 * 2 = 8, 18 -> 36 */
    r = f6F078(n);
    check(r == 1 && S32(0x1101B0u) == 36 && S32(0x1101C0u) == 8, "A1 0x6F078 ranking with Mining");
    hand0(n);
    setD32(0x11019Cu, 0x01020304u);
    r = f6F078(n);
    check(r == 0x01020304 && S32(0x1103ECu) == 0, "A1 0x6F078 empty hand returns [0x11019C]");

    /* A2 0x6DE04 with the variant: 2 cities, treasury 6, two of card 5: stack 2 (s 3) for 6 */
    memcpy(g_ds, g_base[0], DS_SIZE);
    hand0(n);
    setD8(0xE1624u, (uint8_t)(D8(0xE1624u) | 0x20u));
    setD8(rec + 0x51u, 2);
    setD8(rec + 0x3Eu, 6);
    setD8(rec + 0x99u, 10);
    setD8(rec + 5u, 2);
    setD16(STK(2), 3);
    setD8(STK(2) + 8u, 6);
    r = f6DE04(n);
    check(r == 0x301 && S8(rec + 0x3Eu) == 0 && S8(rec + 0x99u) == 16 && S32(0xF590Cu + 4u * 3u) == 2
          && S8(rec + 6u) == 1 && S16(STK(2)) == 2, "A2 0x6DE04 variant tier 1: stack 2 for 6");
    /* without the variant: treasury 18, stock >= 2 cities, no reserve flag, Gold and Ivory held: stack 8 for 18 */
    setD8(0xE1624u, (uint8_t)(D8(0xE1624u) & ~0x20u));
    setD8(rec + 0x3Eu, 18);
    setD8(rec + 0x99u, 10);
    setD8(0xF488Cu + 3u, 0);
    setD8(rec + 17u, 1);
    setD8(rec + 18u, 1);
    setD16(STK(8), 1);
    setD8(STK(8) + 6u, 17);
    r = f6DE04(n);
    check(r == 0x901 && S8(rec + 0x3Eu) == 0 && S8(rec + 0x99u) == 28 && S8(rec + 17u) == 2,
          "A2 0x6DE04 no variant, Gold and Ivory: stack 8 for 18");

    /* A3 0x6EDA8 on a hand of 9 calamities: fixed, it ends; --original: [0x11019C] = 5 discards card 5 once */
    memcpy(g_ds, g_base[0], DS_SIZE);
    hand0(n);
    setD8(rec + 0x13u, 9);
    setD32(0x11019Cu, 5);
    memset(&c, 0, sizeof c);
    c.fn = F6EDA8;
    c.a[0] = n;
    g_scr = 1;
    r = run(&c, 0);
    check(r == 0 && S8(rec + 5u) == 0 && S8(rec + 0x13u) == 9, "A3 0x6EDA8 calamities only: ends (fixed)");
    memcpy(g_ds, g_base[0], DS_SIZE);
    hand0(n);
    setD8(rec + 0x13u, 9);
    setD32(0x11019Cu, 5);
    setD16(PIL(2), 0);
    g_original = 1;
    r = run(&c, 0);
    check(r == 0 && S8(rec + 5u) == -1 && S16(PIL(2)) == 1 && D8(PIL(2) + 6u) == 5 && D8(0xF14D1u) == 0,
          "A3 0x6EDA8 --original: the garbage id 5 is discarded (n[5] = -1, pile 2)");
    memcpy(g_ds, g_base[0], DS_SIZE);
    hand0(n);
    setD8(rec + 0x13u, 9);
    setD32(0x11019Cu, 0);
    r = run(&c, 0);
    check(r == -12345, "A3 0x6EDA8 --original: [0x11019C] = 0 hangs (cut)");
    g_original = 0;

    /* A4 0x5DE5C: 2 cities not built this turn, equal limits and scores: the AI flag of row n (not n - 1) picks */
    memcpy(g_ds, g_base[0], DS_SIZE);
    {
        int t1 = 20, t2 = 30;
        setD8(rec + 0x51u, 2);
        setD8(rec + 0x52u, (uint8_t)(t1 + 1));
        setD8(rec + 0x53u, (uint8_t)(t2 + 1));
        setD8(rec + 0x98u, 3);
        setD16(0xF40FCu, 4);
        setD8(TSA(t1) + 1u, 1);
        setD8(TSA(t2) + 1u, 1);
        setD8(TSA(t1), (uint8_t)n);
        setD8(TSA(t2), (uint8_t)n);
        setD8(0xE1639u + 35u * (uint32_t)t1, (uint8_t)(D8(0xE1639u + 35u * (uint32_t)t2) & ~0x40u));
        setD8(0xE1639u + 35u * (uint32_t)t2, D8(0xE1639u + 35u * (uint32_t)t1));
        {
            int q;
            for (q = 0; q < 160; q++)
                setD8(AREA(q) + 0x0Eu + (uint32_t)n, 0);
        }
        setD8(0xF3580u + 314u * (uint32_t)(n - 1) + 2u * (uint32_t)t1, 0x80);
        setD8(0xF3580u + 314u * (uint32_t)(n - 1) + 2u * (uint32_t)t2, 0);
        setD8(0xF3580u + 314u * (uint32_t)n + 2u * (uint32_t)t1, 0);
        setD8(0xF3580u + 314u * (uint32_t)n + 2u * (uint32_t)t2, 0x80);
        setD8(0xF0980u, 0);
        g_cur = 0;
        g_len[0] = 0;
        g_log[0][0] = 0;
        f5DE5C(n, NULL);
        snprintf(buf, sizeof buf, "f78FEC %d %d", t2, n);
        check(strstr(g_log[0], buf) != NULL && strstr(g_log[0], "f79270 2500") != NULL,
              "A4 0x5DE5C second pass: row n's AI flag first (t2), 2500 ms");
        /* a city built this turn wins over the others (first pass, row n - 1) */
        memcpy(g_ds, g_base[0], DS_SIZE);
        setD8(rec + 0x51u, 2);
        setD8(rec + 0x52u, (uint8_t)(t1 + 1));
        setD8(rec + 0x53u, (uint8_t)(t2 + 1));
        setD8(rec + 0x98u, 3);
        setD16(0xF40FCu, 4);
        setD8(TSA(t1) + 1u, 1);
        setD8(TSA(t2) + 1u, 4);
        setD8(0xF0980u, 0x10);
        g_len[0] = 0;
        g_log[0][0] = 0;
        f5DE5C(n, NULL);
        snprintf(buf, sizeof buf, "f78FEC %d %d", t2, n);
        check(strstr(g_log[0], buf) != NULL && strstr(g_log[0], "f79270 -500") != NULL,
              "A4 0x5DE5C first pass: the city built this turn, CONTINUE wait");
        /* 2 * cities <= tokens: nothing */
        setD8(rec + 0x98u, 4);
        g_len[0] = 0;
        g_log[0][0] = 0;
        f5DE5C(n, NULL);
        check(strstr(g_log[0], "f78FEC") == NULL, "A4 0x5DE5C enough tokens: no reduction");
    }

    /* A5 0x333E8: bit 0x02 cleared, the byte masked to 3; the texts */
    memcpy(g_ds, g_base[0], DS_SIZE);
    setD8(TSA(40) + 2u, 0xFE);
    setD8(0xF0980u, 1);
    g_len[0] = 0;
    g_log[0][0] = 0;
    f333E8(40, n, 6, 0x63);
    snprintf(buf, sizeof buf, "f79270 -500 %d 40 [%s builds a city in %s using Architecture.]",
             n, DSTR(D32(0xC768Cu + 4u * (uint32_t)n)), DSTR(D32(0xC77D8u + 160u)));
    check(D8(TSA(40) + 2u) == 1 && strstr(g_log[0], buf) != NULL, "A5 0x333E8 state bits and the Architecture text");
    printf("A. %d checks, %d fail\n", g_tests, g_fail);
}

/* ---- part C: the real saves ------------------------------------------------------------------------------------------ */

static void part_c(int base, const char *name)
{
    int n, f;
    char tag[80];
    printf("   %s:", name);
    for (n = 1; n <= 9; n++) {
        if (S8(NAT(n) + 0x9Au) != 2)
            continue;
        for (f = 0; f < NFN; f++) {
            Call c;
            int q;
            memcpy(g_ds, g_base[base], DS_SIZE);
            g_rs = (uint32_t)(n * 31 + f);
            place_cities(n, 2 + n % 6);   /* R4 has no cities yet: two to seven on n's own areas */
            for (q = 1; q <= 0x12; q++)   /* and a dealt hand of 9 to 14 cards */
                setD8(NAT(n) + (uint32_t)q, (uint8_t)((n + q) % 3 == 0 ? 2 : (n + q) % 4 == 0 ? 1 : 0));
            setD8(NAT(n) + 0x3Eu, (uint8_t)(6 * (n % 4)));
            memset(&c, 0, sizeof c);
            c.fn = f;
            c.a[0] = n;
            if (f == F6DBAC) {
                c.a[0] = n % 7;
                c.a[1] = n;
            } else if (f == F333E8) {
                c.a[0] = S8(NAT(n) + 0x60u) > 0 ? D8(NAT(n) + 0x61u) - 1 : 0;
                c.a[1] = n;
                c.a[2] = 6;
                c.a[3] = 0;
            }
            g_scr = (uint32_t)(n + 100 * f);
            snprintf(tag, sizeof tag, "C %s n%d %s", name, n, g_fname[f]);
            check(compare_one(&c, tag), tag);
            if (f == F6DE04)
                printf(" n%d:buy %X", n, (unsigned)f6DE04(n));
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
        g_rs = 9090u + (uint32_t)i;
        bad = part_b(3000, 1) + part_b_rank(500, 1);
        snprintf(buf, sizeof buf, "D mutation %d caught (%d of 3500 checks differ)", g_mut, bad);
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
    part_c(0, "SAVGAMEA.DAT");
    part_c(1, "savgame0.dat");
    printf("C. %d fail so far\n", g_fail);
    part_b(30000, 0);
    part_b_rank(5000, 0);
    g_original = 1;
    part_b(12000, 0);
    g_original = 0;
    printf("B. %d fail so far\n", g_fail);
    part_d();
    printf("%d checks, %d fail\n", g_tests, g_fail);
    return g_fail == 0 ? 0 : 1;
}
