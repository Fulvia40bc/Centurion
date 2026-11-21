/* aitrade_test [GAMEDIR [PLAYDIR]]: unit checks for module AITRADE (aitrade.c) that need no game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init; default C:\work\advciv_run, only read) and the saves
   savgame0.dat (R4 at MOVEMENT) and savgame3.dat..savgame9.dat (R4 at TRADE / calamities, edited); PLAYDIR holds
   SAVGAMEA.DAT (the R4 autosave: seven computers; default C:\work\advciv_play, only read). The saves are read with
   saveload.c's f5F05C as the game does.

   aitrade.c calls only ported code, so it is built as is (no recorders of its own). The callees outside formulas.c
   are stand-ins here that append one line to a trace and act as their headers describe: game_rand f78AE4 on the seed
   [0xCAC5C] ("rand"), the shuffle f76F70 (one draw per card), f6DD98 and f195D8 (the hand values; f195D8 adds 7 while
   the Mining flag [0xF6754] is set, so the order of f194E4 matters), f194E4 (sets that flag from +0x29), f75074 (the
   accepted response). f690AC, f6B928, f7CAB4 are formulas.c's (linked); f726DC is saveload.c's (part C).

   The model is a second implementation of the twelve routines written here from the listing (not from aitrade.c),
   in the listing's terms: every byte field is read as the original reads it (the dword at the listing's displacement,
   sar 24; words sar 16), the loops with the listing's labels. Each check runs the port, keeps its data object and
   trace, restores the start state, runs the model, and compares the whole data object byte for byte (the seed
   included) and the traces line for line. Both modes (--original and fixed) are checked. Parts:
     A. hand vectors with the expected values written out: 0x68788's values and order (Mining); 0x68B44's want count
        -1; 0x6937C's truthful declare (slot 2 = slot 10, no draw); 0x68C90's draw count; 0x6AF34 clears 0x110608,
        not the responses; 0x698A4 with 70 rows (item 4: the fixed mode keeps three of n's rows, the others intact).
     B. random differential checks: the twelve routines called directly on random states (a real save with random
        hands, offers, rows, responses, trust bytes, flags, levels, sessions, phases, seeds and stale globals).
     C. the real saves: the three trade steps for every computer as trade.c calls them (DECLARE OFFERS, f726DC,
        COUNTER OFFERS, a response list per offerer, ACCEPT TRADES), port against model; counts printed.
     D. mutations: deliberately broken copies of the model must each be caught by part B's checks.
   Prints each failure (the first differing trace line or byte), the counts per part, and exits 0 when all pass.

   Build (from the repo root; artefacts outside the repo):
     clang -std=c11 -Wall -Wextra -Iport/src port/test/aitrade_test.c port/src/aitrade.c port/src/formulas.c
       port/src/saveload.c port/src/state.c port/src/le_image.c -o aitrade_test.exe */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aitrade.h"
#include "formulas.h"
#include "game.h"
#include "info.h"
#include "saveload.h"
#include "state.h"
#include "trade.h"

#define NAT(n) (0xF14CCu + 199u * (uint32_t)(n))
#define SEED   0xCAC5Cu
#define MINING 0xF6754u

/* ---- trace ------------------------------------------------------------------------------------------------------ */

#define LOGSZ (1u << 20)
static char *g_log[2];
static size_t g_len[2];
static int g_side;

static void tr(const char *fmt, ...)
{
    va_list ap;
    char line[160];
    size_t n;
    va_start(ap, fmt);
    vsnprintf(line, sizeof line, fmt, ap);
    va_end(ap);
    n = strlen(line);
    if (g_len[g_side] + n + 2 < LOGSZ) {
        memcpy(g_log[g_side] + g_len[g_side], line, n);
        g_len[g_side] += n;
        g_log[g_side][g_len[g_side]++] = '\n';
        g_log[g_side][g_len[g_side]] = 0;
    }
}

/* ---- stand-ins for the callees outside formulas.c --------------------------------------------------------------- */

int f78AE4(void)
{
    uint32_t s = D32(SEED) * 0x15A4E35u + 1u;
    setD32(SEED, s);
    tr("rand");
    return (int)((s >> 16) & 0x7FFF);
}

void f76F70(int n, uint8_t *cards)
{
    uint8_t tmp[160];
    int i, k, m = n;
    tr("shuffle %d %d", n, n > 0 ? cards[0] : -1);
    if (n <= 0 || n > 160)
        return;
    memcpy(tmp, cards, (size_t)n);
    for (i = 0; i < n; i++) {
        k = (int)((uint32_t)f78AE4() * (uint32_t)m >> 15);
        cards[i] = tmp[k];
        memmove(tmp + k, tmp + k + 1, (size_t)(m - k - 1));
        m--;
    }
}

int f6DD98(int n)
{
    int i, t = 0, c;
    for (i = 1; i <= 18; i++) {
        c = S8(NAT(n) + (uint32_t)i);
        t += c * c * ((i + 1) >> 1);
    }
    tr("cards %d -> %d", n, t);
    return t;
}

int f195D8(int n)
{
    int i, t = 0, c;
    for (i = 1; i <= 18; i++) {
        c = S8(NAT(n) + (uint32_t)i);
        t += c * c * ((i + 1) >> 1);
    }
    if (D32(MINING) != 0)
        t += 7;
    tr("value %d -> %d", n, t);
    return t;
}

void f194E4(int n)
{
    tr("mining %d", n);
    setD32(MINING, D8(NAT(n) + 0x29u) != 0 ? 1u : 0u);
}

void f75074(int i)
{
    tr("trade %d", i);
}

/* ---- link stubs for formulas.c and saveload.c (not reached by aitrade.c) ---------------------------------------- */

int f78D08(int t, int n) { (void)t; (void)n; return 0; }

#define TAG 0x80000000u
static uint8_t *s_blk[32];
static uint32_t s_blen[32];

uint32_t load_heap_alloc(uint32_t size)
{
    uint32_t i;
    if (size == 0 || size > 0xFFFFFu)
        return 0;
    for (i = 0; i < 32; i++)
        if (s_blk[i] == NULL) {
            s_blk[i] = calloc(1, size);
            s_blen[i] = size;
            return TAG | (i + 1u) << 20;
        }
    return 0;
}

uint8_t *load_heap_ptr(uint32_t h, uint32_t n)
{
    uint32_t slot = (h & ~TAG) >> 20, off = h & 0xFFFFFu;
    if ((h & TAG) == 0 || slot < 1 || slot > 32 || s_blk[slot - 1] == NULL || off > s_blen[slot - 1] ||
        n > s_blen[slot - 1] - off)
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

/* ============================================================================================================== */
/* The model, from the listing                                                                                    */
/* ============================================================================================================== */

static int g_mut;   /* part D: which deliberate break is active (0 none) */

static int32_t d32(uint32_t a) { return S32(a); }
static int32_t sar24(uint32_t a) { return (int32_t)D32(a) >> 24; }   /* dword [a] sar 0x18 = int8 at a + 3 */
static int32_t sar16(uint32_t a) { return (int32_t)D32(a) >> 16; }   /* dword [a] sar 0x10 = int16 at a + 2 */
static void w32(uint32_t a, int32_t v) { setD32(a, (uint32_t)v); }
static void w8(uint32_t a, int32_t v) { setD8(a, (uint8_t)v); }
static int32_t half(int32_t x) { return (int32_t)(x - (x >> 31)) >> 1; }   /* mov eax,edx; sar edx,1F; sub; sar */
static int32_t imul(int32_t a, int32_t b) { return (int32_t)((uint32_t)a * (uint32_t)b); }
static int32_t lvl(void) { return (int32_t)((uint16_t)(D16(0xE1624u) << 4) >> 14); }
static int32_t r100(void) { uint32_t r = (uint32_t)f78AE4(); return (int32_t)((((r * 4u - r) * 8u + r) * 4u) >> 15); }
static int32_t crow(void) { return sar16(0x112546u); }
static void rec_copy(uint32_t d, uint32_t s) { memmove(DP(d), DP(s), 0x31 * 4 + 2 + 1); }

static void m_68A64(int32_t n, int32_t x)
{
    int32_t m, i, edi;
    (void)x;
    memset(DP(0x1105A0u), 0, 9);
    for (m = 1; m <= 9; m++) {
        if (D8(0xF1566u + 199u * (uint32_t)m) == 0 || m == n)
            continue;
        for (i = 0x12; i >= 1; i--) {
            if (sar24(199u * (uint32_t)m + (uint32_t)i + 0xF14C9u) == 0)
                continue;
            edi = d32(0xF590Cu + 4u * (uint32_t)m);
            if (edi != -1 && d32(0xF590Cu + 4u * (uint32_t)m) >= 7 && half(i - 1) >= 7)
                continue;
            memset(DP(0x1105A0u), 1, (size_t)(half(i - 1) + 1));
        }
    }
}

static void m_68788(int32_t n)
{
    uint32_t r = 0xF14CCu + 199u * (uint32_t)n;
    int32_t i, s, fl, j, e, v, k, c;
    uint8_t t[16];

    m_68A64(n, sar24(r + 0xB8u));
    if (sar16(0xF5474u) == 1) {
        for (i = 0x12; i >= 1; i--) {
            if ((int8_t)D8(r + (uint32_t)i) == 0)
                continue;
            if (d32(0xF590Cu + 4u * (uint32_t)n) >= 7 && half(i - 1) >= 7)
                continue;
            s = half(i - 1);
            if (s - 1 > sar24(199u * (uint32_t)n + 0xF151Au))
                continue;
            w32(0x1105D4u + 4u * (uint32_t)n, s);
            break;
        }
    }
    memset(DP(0x1101ACu), 0, 0x120);
    w32(0x1103ECu, 0);
    w32(0x1103F0u, 0);
    for (i = 0x12; i >= 1; i--) {
        if (D8(r + (uint32_t)i) == 0 || D8(0x1105A0u + (uint32_t)half(i - 1)) == 0)
            continue;
        e = d32(0x1103ECu) << 4;
        w32((uint32_t)e + 0x1101ACu, i);
        c = (int8_t)D8(r + (uint32_t)i);
        w32((uint32_t)e + 0x1101B4u, c);
        fl = 0;
        for (j = 0; j < 6; j++)
            if (sar24((uint32_t)j + 0xBDD39u) == i) {
                fl = 1;
                break;
            }
        if (i == 1 || i == 2) {
            v = imul(imul(c, c), (i + 1) >> 1);
        } else if (D8(r + 0x29u) != 0 && fl != 0) {
            v = imul(imul(c + 2, c + 2), (i + 1) >> 1);
            c = c + 1;
        } else {
            v = imul(imul(c + 1, c + 1), (i + 1) >> 1);
        }
        if (g_mut == 1 && i > 2)
            v = imul(imul(c + 1, c + 1), i >> 1);
        w32((uint32_t)(d32(0x1103ECu) << 4) + 0x1101B0u, v);
        w32((uint32_t)(d32(0x1103ECu) << 4) + 0x1101B8u, c);
        w32(0x1103ECu, d32(0x1103ECu) + 1);
    }
    for (k = d32(0x1103ECu) - 2; k >= 0; k--)
        for (j = 0; j <= k; j++)
            if (d32(16u * (uint32_t)j + 0x1101C0u) > d32(16u * (uint32_t)j + 0x1101B0u)) {
                memcpy(t, DP(16u * (uint32_t)j + 0x1101BCu), 16);
                memcpy(DP(16u * (uint32_t)j + 0x1101BCu), DP(16u * (uint32_t)j + 0x1101ACu), 16);
                memcpy(DP(16u * (uint32_t)j + 0x1101ACu), t, 16);
            }
}

/* the want count of 0x68B44 / 0x698A4 (the same instructions) */
static int32_t want_count(int32_t n)
{
    int32_t k = d32(4u * (uint32_t)d32(0x1103ECu) + 0xC8DA0u), s = 0, j;
    if (d32(0x1103ECu) < 8) {
        for (j = k; j < d32(0x1103ECu); j++)
            s += d32(16u * (uint32_t)j + 0x1101B4u);
        if (d32(0x110600u) < 0)
            s++;
        if (s < 3)
            k--;
    }
    if (d32(0x110600u) < -1 || f7CAB4(n) != 0) {
        if (k > 3)
            k = 3;
    } else if (sar16(2u * (uint32_t)n + 0x112C98u) / 300 < 25) {
        if (k > 3)
            k = 3;
    }
    if (g_mut == 2 && k < 0)
        k = 0;
    return k;
}

static void m_68B44(int32_t n)
{
    int32_t k, off = sar24(0xCA99Fu), j;
    uint32_t r = 0xF14CCu + 199u * (uint32_t)n;
    k = d32(4u * (uint32_t)d32(0x1103ECu) + 0xC8DA0u);
    (void)k;
    memset(DP(r + 0x9Cu + (uint32_t)off), 0, 5);
    k = want_count(n);
    w32(0x1105ACu + 4u * (uint32_t)n, k);
    if (k == 0)
        return;
    for (j = 0; j < k; j++)
        w8(199u * (uint32_t)n + (uint32_t)(off + j) + 0xF1568u, (int32_t)D8(16u * (uint32_t)j + 0x1101ACu));
    w32(0x11059Cu, j);
}

static void m_68FDC(int32_t n)
{
    uint32_t r = 0xF14CCu + 199u * (uint32_t)n;
    int32_t i, v;
    w32(0x110600u, 0);
    w32(0x110604u, 0);
    if (D8(r + 0x13u) == 0)
        return;
    for (i = 0; sar24(r + 0x10u) > i; i++) {
        v = f690AC(sar24(r + (uint32_t)i + 0x11u), n);
        if (v < d32(0x110600u) || (g_mut == 3 && v == d32(0x110600u))) {
            w32(0x110604u, sar24(r + (uint32_t)i + 0x11u));
            w32(0x110600u, v);
        }
    }
}

static void m_6937C(int32_t n, int32_t *tp, int32_t cnt, int32_t k, int32_t flag)
{
    uint32_t r = 0xF14CCu + 199u * (uint32_t)n;
    int32_t one = 1, top, j, i, q, c = 0, a = 0, R = 0, B = 0, f2c = 0, e, f, w, m, v, third;

    if (*tp != 0) {
        if (D8(0xF4102u) == 0x0C) {
            w8(r + (uint32_t)sar24(0xCA9AAu) + 0x9Cu, (int32_t)D8(r + (uint32_t)sar24(0xCA9ACu) + 0x9Cu));
            return;
        }
        v = sar24((uint32_t)(crow() << 4) + (uint32_t)sar24(0xCA9ADu) + 0x111115u);
        if (v != 1 && v != 2) {
            w8((uint32_t)(crow() << 4) + (uint32_t)sar24(0xCA9AAu) + 0x111118u,
               (int32_t)D8((uint32_t)(crow() << 4) + (uint32_t)sar24(0xCA9ADu) + 0x111118u));
            return;
        }
        *tp = 0;
    }
    top = 8;
    for (j = 0; j < 9; j++)
        if (sar24((uint32_t)j + 0x11059Du) == 0) {
            top = j - 1;
            break;
        }
    if (top > d32(0x1105D4u + 4u * (uint32_t)n))
        top = d32(0x1105D4u + 4u * (uint32_t)n);
    top = top + top + 2;
    i = 0;
L_696E3:
    if (i >= cnt)
        goto L_697EA;
    a = 0;
    if (D8(0xF4102u) != 0x0C && d32(28u * (uint32_t)k + 0x110408u) >= 0)
        a = 1;
    R = r100() >= 0x42;
    B = D8(0xF4102u) != 0x0C;
    q = top;
L_697C1:
    if (q < 1)
        goto L_696DD;
    c = q;
    if (D8(0xF4102u) == 0x0C) {
        for (w = 0; w < d32(0x1105ACu + 4u * (uint32_t)n); w++)
            if (sar24((uint32_t)sar24(0xCA9ABu) + (uint32_t)w + r + 0x99u) == c) {
                c = 0;
                break;
            }
    }
    if (c == 0)
        goto L_6975E;
    if (D8(0xF4102u) != 0x0C && a != 0) {
        f2c = 0;
        m = d32(28u * (uint32_t)k + 0x110400u);
        for (w = 0; w < d32(0x1105ACu + 4u * (uint32_t)m); w++)
            if (sar24(199u * (uint32_t)m + (uint32_t)(sar24(0xCA9ABu) + w) + 0xF1565u) == c) {
                f2c = 1;
                break;
            }
    }
    if (D8(0xF4102u) != 0x0C && a != 0 && f2c == 0)
        c = 0;
    e = 0;
    if (R != 0 && c != 0)
        for (w = 0; w < 2; w++) {
            if (D8(0xF4102u) == 0x0C)
                v = sar24(199u * (uint32_t)n + (uint32_t)(sar24(0xCA9A9u) + w) + 0xF1565u);
            else
                v = sar24((uint32_t)(crow() << 4) + (uint32_t)w + 0x111115u);
            if (v == c) {
                e = 1;
                break;
            }
        }
    if (e != 0)
        c = 0;
    f = 0;
    if (B != 0 && c != 0) {
        m = d32(28u * (uint32_t)k + 0x110400u);
        for (w = 0; w < 3; w++)
            if (sar24(199u * (uint32_t)m + (uint32_t)(sar24(0xCA9A9u) + w) + 0xF1565u) == c) {
                f = 1;
                break;
            }
    }
    if (f != 0)
        c = 0;
    if (c == 0)
        goto L_6975E;
    if (D8(0xF4102u) == 0x0C)
        w8(r + (uint32_t)sar24(0xCA9AAu) + 0x9Cu, c);
    else
        w8((uint32_t)(i + sar24(0xCA9AAu)) + (uint32_t)(crow() << 4) + 0x111118u, c);
    top = c - 1;
    if (g_mut == 4)
        top = c;
L_696DD:
    i++;
    goto L_696E3;
L_6975E:
    if (R != 0 && q == 1 && c == 0) {
        q = top + q;
        R = 0;
    }
    if (B != 0 && q == 1 && c == 0) {
        q = top + q;
        B = 0;
    }
    if (a != 0 && q == 1 && c == 0 && g_mut != 5) {
        q = top + 1;
        a = 0;
    }
    q--;
    goto L_697C1;
L_697EA:
    if (c == 0 || flag != 0)
        return;
    if (D8(0xF4102u) == 0x0C)
        third = sar24((uint32_t)sar24(0xCA9ACu) + r + 0x99u);
    else if (d32(28u * (uint32_t)k + 0x110408u) > 0)
        third = sar24((uint32_t)(crow() << 4) + (uint32_t)sar24(0xCA9ADu) + 0x111115u);
    else
        third = -1;
    if (half(c - 1) >= half(third - 1))
        return;
    m_6937C(n, &one, 1, k, 0);
}

static void m_68C90(int32_t n)
{
    uint32_t r = 0xF14CCu + 199u * (uint32_t)n;
    int32_t t, m3, j, e, cc, fl, k, x[3], s;

    if (sar24(r + 0xB9u) == 1)
        t = r100() >= 0x42;
    else
        t = r100() < 0x42;
    w32(0x1106B0u + 4u * (uint32_t)n, 0);
    if (d32(0x110600u) < 0) {
        m3 = 2;
        t = 0;
    } else {
        m3 = 3;
        if (sar16(0xF5474u) == (int32_t)(D8(0xE1623u) & 0xF)) {
            if (r100() < 0x50 - 20 * lvl())
                w32(0x1106B0u + 4u * (uint32_t)n, 1);
        }
    }
    j = 0;
    for (e = d32(0x11059Cu); e < d32(0x1103ECu) && m3 > 0; e++) {
        cc = d32(16u * (uint32_t)e + 0x1101B4u);
        while (cc > 0 && m3 > 0) {
            w8(r + (uint32_t)j + 0x9Cu, (int32_t)D8(16u * (uint32_t)e + 0x1101ACu));
            j++;
            cc--;
            if (j == 2) {
                j = sar24(0xCA9ACu);
                if (t == 0 && d32(0x110600u) >= 0) {
                    w8(r + (uint32_t)j + 0x9Cu, (int32_t)D8(16u * (uint32_t)d32(0x1103ECu) + 0x11019Cu));
                    m3--;
                }
            }
            m3--;
        }
    }
    fl = 0;
    if (sar24(r + (uint32_t)sar24(0xCA9A0u) + 0x99u) == 0) {
        fl = 1;
        w8(r + (uint32_t)sar24(0xCA9A0u) + 0x9Cu, (int32_t)D8(0x110604u));
    }
    m_6937C(n, &t, 1, 0, fl);
    if (d32(0x1105ACu + 4u * (uint32_t)n) > 1)
        f76F70(d32(0x1105ACu + 4u * (uint32_t)n), DP(r + 0x9Cu + (uint32_t)sar24(0xCA9ABu)));
    if (fl != 0 || t == 0)
        return;
    x[0] = sar24(r + 0x99u);
    x[1] = sar24(r + 0x9Au);
    x[2] = sar24(r + (uint32_t)sar24(0xCA9A0u) + 0x99u);
    for (k = 1; k >= 0; k--)
        for (j = 0; j <= k; j++)
            if (x[j + 1] > x[j]) {
                s = x[j];
                x[j] = x[j + 1];
                x[j + 1] = s;
            }
    w8(r + 0x9Cu, x[0]);
    w8(r + 0x9Du, x[1]);
    w8(r + (uint32_t)sar24(0xCA9A0u) + 0x9Cu, x[2]);
    w8(r + (uint32_t)sar24(0xCA99Eu) + 0x9Cu, x[2]);
}

static void m_68730(int32_t n)
{
    m_68FDC(n);
    m_68788(n);
    m_68B44(n);
    if (d32(4u * (uint32_t)n + 0x1105ACu) != 0)
        m_68C90(n);
    else
        w8(199u * (uint32_t)n + 0xF1568u, 0);
}

/* the frame of 0x6AD70 (base -0x1FC, held at -0x94, 29) / 0x6AF34 (base -0x1D4, held at -0x6C, 19), words */
static int32_t frame_word(const int32_t *tab, int nheld, int32_t off, int32_t q, int32_t i, int32_t c, int32_t n)
{
    int32_t idx = off / 4, top = 90 + nheld;
    if (off < 0)
        return 0;
    if (idx < top)
        return tab[idx];
    if (idx == top) return q;
    if (idx == top + 1) return i;
    if (idx == top + 2) return c;
    if (idx == top + 3) return n;
    if (idx == top + 4) return (int32_t)(0xF14CCu + 199u * (uint32_t)n);
    return 0;
}

static void m_filter(int32_t n, int resp)
{
    int32_t tab[90 + 29];
    int nheld = resp ? 19 : 29;
    uint32_t r = 0xF14CCu + 199u * (uint32_t)n;
    int32_t m, s, c, i, q, cnt, t1, t2;

    memset(tab, 0, sizeof tab);
    for (m = 1; m <= 9; m++) {
        if (D8(199u * (uint32_t)m + 0xF1566u) == 0)
            continue;
        m_68A64(m, sar24(199u * (uint32_t)n + 0xF1584u));
        for (s = 0; s < 9; s++)
            tab[(36 * m + 4 * s) / 4] = sar24((uint32_t)s + 0x11059Du);
    }
    memset(tab + 90, 0, (size_t)nheld * 4u);
    for (c = 1; c <= 0x12; c++)
        if ((int8_t)D8(r + (uint32_t)c) == d32(4u * (uint32_t)c + 0xC8D58u))
            tab[90 + c] = 1;
    cnt = resp ? sar16(0x112566u) : d32(0x112560u);
    for (i = 0; i < cnt; i++)
        for (q = 0; q < 5; q++) {
            if (resp) {
                c = sar24(20u * (uint32_t)i + (uint32_t)q + 0x111495u);
                if (c == 0)
                    continue;
                m = sar24(20u * (uint32_t)i + 0x1114A3u);
            } else {
                c = sar24(21u * (uint32_t)i + (uint32_t)q + 0x110605u);
                if (c == 0)
                    continue;
                m = sar24(21u * (uint32_t)i + 0x110617u);
            }
            t1 = frame_word(tab, nheld, 36 * m + 4 * half(c - 1), q, i, c, n);
            t2 = frame_word(tab, nheld, 360 + 4 * c, q, i, c, n);
            if (t1 != 0 && t2 == 0)
                continue;
            if (g_mut == 6 && resp)
                w8(20u * (uint32_t)i + (uint32_t)q + 0x111498u, 0);
            else
                w8(21u * (uint32_t)i + (uint32_t)q + 0x110608u, 0);
        }
}

static int m_wants(int32_t m, int32_t id)
{
    int32_t w;
    for (w = 0; w < 5; w++)
        if (sar24(199u * (uint32_t)m + (uint32_t)(sar24(0xCA99Fu) + w) + 0xF1565u) == id)
            return 1;
    return 0;
}

static void m_6A5FC(int32_t n, int32_t k, int32_t *out, int32_t cflag)
{
    uint32_t rn = 0xF14CCu + 199u * (uint32_t)n, rm, rw;
    int32_t m, m3, mt = 0, e, q, t, j, any, low, id, cc, cnt, v, pass, ok, o, kk, sc[3], cd[3], ts;

    memset(DP((uint32_t)(crow() << 4) + 0x111118u), 0, 8);
    m = d32(28u * (uint32_t)k + 0x110400u);
    rm = 0xF14CCu + 199u * (uint32_t)m;
    m3 = 3;
    *out = 0;
    if (d32(0x110600u) < -1 || cflag != 0) {
        m3--;
        *out = 1;
    }
    for (e = d32(0x1103ECu) - 1; e >= 0; e--)
        for (q = 0; q < 5; q++)
            if (sar24((uint32_t)sar24(0xCA9ABu) + (uint32_t)q + rm + 0x99u) == d32(16u * (uint32_t)e + 0x1101ACu))
                mt++;
    if (sar24(rn + 0xB9u) == 1)
        t = r100() >= 0x4B;
    else
        t = r100() < 0x4B;
    if (*out != 0)
        t = 0;
    if (mt + (t == 0 ? 1 : 0) == 0)
        return;
    j = 0;
    any = 0;
    low = d32(16u * (uint32_t)d32(0x1103ECu) + 0x1101A4u);
    for (pass = 1; pass <= 2; pass++) {
        if (pass == 2) {
            if (m3 <= 0)
                break;
            if (any == 0)
                t = 0;
        }
        for (e = d32(0x11059Cu); e < d32(0x1103ECu) && m3 > 0; e++) {
            id = d32(16u * (uint32_t)e + 0x1101ACu);
            if (pass == 1) {
                if (!m_wants(m, id))
                    continue;
                any = 1;
            } else {
                o = 0;
                for (q = 0; q < 3; q++)
                    if (sar24(199u * (uint32_t)m + (uint32_t)(sar24(0xCA99Du) + q) + 0xF1565u) == id) {
                        o = 1;
                        break;
                    }
                if (m_wants(m, id) || o)
                    continue;
            }
            cc = d32(16u * (uint32_t)e + 0x1101B4u);
            while (cc > 0 && m3 > 0) {
                rw = (uint32_t)(crow() << 4) + 0x111118u;
                w8(rw + (uint32_t)j, id);
                j++;
                if (d32(0x1103ECu) - 1 == e)
                    low--;
                cc--;
                if (j == 2) {
                    j = sar24(0xCA9ADu);
                    if (t == 0 && d32(0x110600u) >= 0 && low != 0) {
                        w8((uint32_t)(crow() << 4) + (uint32_t)j + 0x111118u,
                           (int32_t)D8(16u * (uint32_t)d32(0x1103ECu) + 0x11019Cu));
                        m3--;
                    }
                }
                m3--;
            }
        }
    }
    if (m3 > 0) {
        w8((uint32_t)(crow() << 4) + 0x111118u, 0);
        return;
    }
    if (*out != 0) {
        w8((uint32_t)(crow() << 4) + (uint32_t)j + 0x111118u, (int32_t)D8(0x110604u));
        j++;
    }
    m_6937C(n, &t, d32(28u * (uint32_t)k + 0x1103F8u) - 2, k, *out);
    cnt = 0;
    for (e = 0; e < 5; e++) {
        v = sar24((uint32_t)(crow() << 4) + (uint32_t)e + 0x111115u);
        if (v == 0)
            continue;
        for (q = 0; q < 5; q++)
            if (sar24((uint32_t)sar24(0xCA9ABu) + (uint32_t)q + rm + 0x99u) == v)
                cnt++;
    }
    if (cnt == 0 && g_mut != 7) {
        w8((uint32_t)(crow() << 4) + 0x111118u, 0);
        return;
    }
    if (*out != 0 || t == 0)
        return;
    rw = (uint32_t)(crow() << 4) + 0x111118u;
    if ((int8_t)D8(rw) == 0)
        return;
    sc[0] = (int8_t)D8(rw);
    sc[1] = (int8_t)D8(rw + 1u);
    sc[2] = (int8_t)D8(rw + (uint32_t)sar24(0xCA9ADu));
    for (q = 0; q < 3; q++)
        cd[q] = sc[q] & 0xFF;
    for (q = 0; q < 3; q++) {
        id = (int8_t)cd[q];
        if (id == 0)
            continue;
        for (kk = 0; kk < 5; kk++)
            if (sar24((uint32_t)sar24(0xCA9ABu) + (uint32_t)kk + rm + 0x99u) == id)
                sc[q] = imul(sc[q], 2);
    }
    for (kk = 1; kk >= 0; kk--)
        for (q = 0; q <= kk; q++)
            if (sc[q + 1] > sc[q]) {
                ts = sc[q]; sc[q] = sc[q + 1]; sc[q + 1] = ts;
                ts = cd[q]; cd[q] = cd[q + 1]; cd[q + 1] = ts;
            }
    ok = 0;
    (void)ok;
    w8(rw, cd[0]);
    w8(rw + 1u, cd[1]);
    w8(rw + (uint32_t)sar24(0xCA9ADu), cd[2]);
    w8(rw + (uint32_t)sar24(0xCA9AAu), cd[2]);
}

static uint8_t m_frame[0x5B0];
static int m_pruned, m_pruned_big;   /* instrumentation of the model only (part A) */
static uint8_t m_rows[((0xB0000u + 0x72C00u) - 0x111118u) / 16u * 16u];

static void m_698A4(int32_t n)
{
    uint32_t r = 0xF14CCu + 199u * (uint32_t)n;
    int32_t h = 0, added = 0, out = 0, kw, i, q, w, c, m, mt, x, ex, d, tr_, tot = 0, avg, th, low, s, g, k, j, t;
    int32_t cnt, rows, idx, v, a, b;
    uint32_t cb, sz, lim;
    uint8_t tmp[28], *buf;

    if (d32(4u * (uint32_t)n + 0x1105ACu) == 0)
        return;
    m_68FDC(n);
    m_68788(n);
    kw = want_count(n);
    if (kw != 0) {
        for (j = 0; j < kw; j++)
            ;
        w32(0x11059Cu, j);
    }
    for (i = 0; i < d32(0x112560u); i++)
        memmove(DP(21u * (uint32_t)i + 0x110608u), DP(21u * (uint32_t)i + 0x110ECCu), 21);
    m_filter(n, 0);
    w32(0x1104B8u, 0);
    for (i = 0; i < d32(0x112560u); i++) {
        m = sar24(21u * (uint32_t)i + 0x110617u);
        if (m == n)
            continue;
        mt = 0;
        for (q = 0; q < 3; q++) {
            c = sar24(21u * (uint32_t)i + (uint32_t)q + 0x110605u);
            if (c == 0)
                continue;
            for (w = 0; w < d32(4u * (uint32_t)n + 0x1105ACu); w++)
                if (sar24((uint32_t)sar24(0xCA9ABu) + (uint32_t)w + r + 0x99u) == c) {
                    mt++;
                    break;
                }
        }
        cb = 28u * (uint32_t)d32(0x1104B8u);
        w32(cb + 0x1103F4u, mt);
        w32(cb + 0x1103F8u, 3);
        rec_copy(0xF14CCu, r);
        for (q = 0; q < 3; q++) {
            uint32_t aa = (uint32_t)sar24(21u * (uint32_t)i + (uint32_t)q + 0x110605u) + 0xF14CCu;
            setD8(aa, (uint8_t)(D8(aa) + 1u));
        }
        a = f6DD98(0);
        b = f6DD98(n);
        w32(28u * (uint32_t)d32(0x1104B8u) + 0x1103FCu, a - b);
        ex = 0;
        for (x = 0; x < 3; x++) {
            if ((D8(0xE1625u) & 0x20u) != 0 && x != 2)
                continue;
            rec_copy(0xF14CCu, r);
            for (q = 0; q < 3; q++)
                if (q != x) {
                    uint32_t aa = (uint32_t)sar24(21u * (uint32_t)i + (uint32_t)q + 0x110605u) + 0xF14CCu;
                    setD8(aa, (uint8_t)(D8(aa) + 1u));
                }
            a = f195D8(0);
            b = f195D8(n);
            ex += a - b;
        }
        if ((D8(0xE1625u) & 0x20u) == 0)
            ex /= 3;
        d = d32(28u * (uint32_t)d32(0x1104B8u) + 0x1103FCu) - ex;
        if (d != 0) {
            tr_ = sar24(r + (uint32_t)sar24(21u * (uint32_t)i + 0x110617u) + 0xAEu);
            if (tr_ < 0)
                tr_ = 0;
            if (tr_ > 0x64)
                tr_ = 0x64;
            w32(28u * (uint32_t)d32(0x1104B8u) + 0x11040Cu, ex + imul(d, tr_) / 100);
        }
        cb = 28u * (uint32_t)d32(0x1104B8u);
        tot += d32(cb + 0x11040Cu);
        w32(cb + 0x110400u, sar24(21u * (uint32_t)i + 0x110617u));
        w32(cb + 0x110404u, i);
        w32(0x1104B8u, d32(0x1104B8u) + 1);
    }
    for (x = 0; x < 2; x++)
        for (k = d32(0x1104B8u) - 2; k >= 0; k--)
            for (j = 0; j <= k; j++)
                if (d32(28u * (uint32_t)(j + 1) + 0x1103F4u + (x == 0 ? 8u : 0u)) >
                    d32(28u * (uint32_t)j + 0x1103F4u + (x == 0 ? 8u : 0u))) {
                    memcpy(tmp, DP(28u * (uint32_t)(j + 1) + 0x1103F4u), 28);
                    memcpy(DP(28u * (uint32_t)(j + 1) + 0x1103F4u), DP(28u * (uint32_t)j + 0x1103F4u), 28);
                    memcpy(DP(28u * (uint32_t)j + 0x1103F4u), tmp, 28);
                }
    for (k = 0; k < d32(0x1104B8u); k++) {
        m = d32(28u * (uint32_t)k + 0x110400u);
        w32(0x1105FCu, f6B928(n, m));
        avg = tot / d32(0x1104B8u);
        if (avg < d32(4u * (uint32_t)lvl() + 0xC8D40u))
            avg = d32(4u * (uint32_t)lvl() + 0xC8D40u);
        th = avg + imul(d32(0x110600u), avg) / 20;
        th += (d32(28u * (uint32_t)k + 0x1103F4u) != 0 || d32(0x110600u) < -1) ? 0 : imul(avg, 5) / 20;
        th -= (d32(0x1105FCu) > 0 && d32(0x110600u) < -1) ? imul(d32(0x1105FCu), avg) / 20 : 0;
        th += imul(sar24(r + (uint32_t)m + 0xA5u), avg) / 20;
        th += imul(avg, imul(sar24(r + (uint32_t)m + 0xBAu), 30)) / 20;
        if (g_mut == 8)
            th += 1;
        if (d32(0x110600u) != 0) {
            h = f690AC(sar24((uint32_t)sar24(0xCA9A0u) + r + 0x99u), m);
            if (h < -1) {
                h = half(h) < -5 ? half(h) : -5;
                if (m == d32(0xF52D4u))
                    th -= imul(avg, 3) / 20;
                if (sar24(199u * (uint32_t)m + 0xF1563u) == 3)
                    th += imul(h, avg) / 20;
            }
        }
        low = 10;
        for (j = 0; j < 5; j++) {
            c = sar24(199u * (uint32_t)m + (uint32_t)(sar24(0xCA9ABu) + j) + 0xF1565u);
            if (c == 0)
                continue;
            s = half(c - 1);
            if (s < low)
                low = s;
        }
        if (d32(4u * (uint32_t)n + 0x1105D4u) < low)
            th += 500;
        if (d32(28u * (uint32_t)k + 0x11040Cu) <= th)
            continue;
        m_6A5FC(n, k, &out, h < -1);
        if (sar24((uint32_t)(crow() << 4) + 0x111115u) == 0)
            continue;
        rec_copy(0xF14CCu, r);
        for (q = 0; q < 3; q++) {
            idx = d32(28u * (uint32_t)k + 0x110404u);
            uint32_t aa = (uint32_t)sar24(21u * (uint32_t)idx + (uint32_t)q + 0x110605u) + 0xF14CCu;
            setD8(aa, (uint8_t)(D8(aa) + 1u));
        }
        for (q = 0; q < 3; q++) {
            v = sar24((uint32_t)(crow() << 4) + (uint32_t)(q < 2 ? q : q + 3) + 0x111115u);
            if (v >= 1 && v <= 0x12)
                setD8((uint32_t)v + 0xF14CCu, (uint8_t)(D8((uint32_t)v + 0xF14CCu) - 1u));
        }
        a = f6DD98(0);
        b = f6DD98(n);
        g = a - b;
        if (th - imul(avg, 2) / 20 > g)
            continue;
        w8((uint32_t)(crow() << 4) + 0x111125u, m);
        w8((uint32_t)(crow() << 4) + 0x111126u, n);
        added++;
        setD16(0x112548u, (uint16_t)(D16(0x112548u) + 1u));
    }
    if (out == 0 || added <= (g_mut == 9 ? 2 : 3))
        return;
    m_pruned++;
    if (crow() > 56)
        m_pruned_big++;
    memset(m_frame, 0, sizeof m_frame);
    cnt = 0;
    for (i = 0; i < crow(); i++)
        if (sar24(16u * (uint32_t)i + 0x111123u) == n && cnt < 59) {
            memcpy(m_frame + 0x380 + 8 * cnt, &i, 4);
            v = sar24(16u * (uint32_t)i + 0x111122u);
            memcpy(m_frame + 0x384 + 8 * cnt, &v, 4);
            cnt++;
        }
    for (k = cnt - 2; k >= 0; k--)
        for (j = 0; j <= k; j++) {
            int32_t m1, m0;
            memcpy(&m1, m_frame + 0x380 + 8 * (j + 1) + 4, 4);
            memcpy(&m0, m_frame + 0x380 + 8 * j + 4, 4);
            if (d32(40u * (uint32_t)n + 4u * (uint32_t)m1 + 0xF52E0u) > d32(40u * (uint32_t)n + 4u * (uint32_t)m0 + 0xF52E0u)) {
                memcpy(tmp, m_frame + 0x380 + 8 * (j + 1), 8);
                memcpy(m_frame + 0x380 + 8 * (j + 1), m_frame + 0x380 + 8 * j, 8);
                memcpy(m_frame + 0x380 + 8 * j, tmp, 8);
            }
        }
    rows = crow();
    buf = (g_original && rows <= 90) ? m_frame : m_rows;
    lim = buf == m_frame ? (uint32_t)sizeof m_frame : (uint32_t)sizeof m_rows;
    sz = rows > 0 ? (uint32_t)rows << 4 : 0;
    if (sz > sizeof m_rows)
        sz = sizeof m_rows;
    memcpy(buf, DP(0x111118u), sz);
    for (j = 3; j < cnt; j++) {
        memcpy(&t, m_frame + 0x380 + 8 * j, 4);
        if (t >= 0 && ((uint32_t)t << 4) + 0xEu < lim)
            buf[((uint32_t)t << 4) + 0xEu] = 0;
    }
    setD16(0x112548u, 0);
    for (i = 0; i < rows && ((uint32_t)i << 4) < sz; i++) {
        if ((int8_t)buf[((uint32_t)i << 4) + 0xEu] == 0)
            continue;
        t = (int16_t)D16(0x112548u);
        setD16(0x112548u, (uint16_t)(t + 1));
        memcpy(DP(((uint32_t)t << 4) + 0x111118u), buf + ((uint32_t)i << 4), 16);
    }
}

static void m_6B0EC(int32_t n)
{
    uint32_t r = 0xF14CCu + 199u * (uint32_t)n;
    uint8_t cp[199], tmp[28];
    int32_t hc, sum = 0, i, q, w, c, mt, ex, d, tr_, avg, th, m, h, a, b, k, j, x, id, cnt;

    hc = sar24(r + (uint32_t)sar24(0xCA9A0u) + 0x99u) >= 0x13;
    m_68FDC(n);
    m_filter(n, 1);
    if (sar16(0x112566u) > 0) {
        memcpy(cp, DP(r), 199);
        id = sar24(r + 0x99u); if (id >= 0 && id < 199) cp[id]--;
        id = sar24(r + 0x9Au); if (id >= 0 && id < 199) cp[id]--;
        id = sar24(r + 0xA3u); if (id >= 0 && id < 199) cp[id]--;
        f194E4(n);
        for (i = 0; i < sar16(0x112566u); i++) {
            mt = 0;
            for (q = 0; q < 3; q++) {
                c = sar24(20u * (uint32_t)i + (uint32_t)q + 0x111495u);
                if (c == 0)
                    continue;
                for (w = 0; w < d32(4u * (uint32_t)n + 0x1105ACu); w++)
                    if (sar24((uint32_t)sar24(0xCA99Fu) + (uint32_t)w + r + 0x99u) == c) {
                        mt++;
                        break;
                    }
            }
            memcpy(DP(0xF14CCu), cp, 199);
            w32(28u * (uint32_t)i + 0x1104D4u, mt);
            for (q = 0; q < 3; q++) {
                uint32_t aa = (uint32_t)sar24(20u * (uint32_t)i + (uint32_t)q + 0x111495u) + 0xF14CCu;
                setD8(aa, (uint8_t)(D8(aa) + 1u));
            }
            a = f195D8(0);
            b = f195D8(n);
            w32(28u * (uint32_t)i + 0x1104BCu, a - b);
            w32(28u * (uint32_t)i + 0x1104D0u, 3);
            ex = 0;
            for (x = 0; x < 3; x++) {
                if ((D8(0xE1625u) & 0x20u) != 0 && x != 2)
                    continue;
                memcpy(DP(0xF14CCu), cp, 199);
                for (q = 0; q < 3; q++)
                    if (q != x) {
                        uint32_t aa = (uint32_t)sar24(20u * (uint32_t)i + (uint32_t)q + 0x111495u) + 0xF14CCu;
                        setD8(aa, (uint8_t)(D8(aa) + 1u));
                    }
                a = f195D8(0);
                b = f195D8(n);
                ex += a - b;
            }
            if ((D8(0xE1625u) & 0x20u) == 0)
                ex /= 3;
            d = d32(28u * (uint32_t)i + 0x1104BCu) - ex;
            if (d != 0) {
                tr_ = sar24(r + (uint32_t)sar24(20u * (uint32_t)i + 0x1114A2u) + 0xAEu);
                if (g_mut == 10)
                    tr_ = sar24(r + (uint32_t)sar24(20u * (uint32_t)i + 0x1114A3u) + 0xAEu);
                if (tr_ < 0)
                    tr_ = 0;
                if (tr_ > 0x64)
                    tr_ = 0x64;
                w32(28u * (uint32_t)i + 0x1104C0u, ex + imul(d, tr_) / 100);
                if (hc) {
                    a = f6B928(n, d32(28u * (uint32_t)i + 0x1104C8u));
                    w32(0x1105FCu, a);
                    w32(28u * (uint32_t)i + 0x1104C0u, d32(28u * (uint32_t)i + 0x1104C0u) + (a > 0 ? half(a) : 0));
                }
            }
            sum += d32(28u * (uint32_t)i + 0x1104C0u);
            w32(28u * (uint32_t)i + 0x1104C4u, i);
            w32(28u * (uint32_t)i + 0x1104C8u, sar24(20u * (uint32_t)i + 0x1114A3u));
        }
    }
    for (i = 0; i < sar16(0x112566u); i++) {
        w32(0x1105FCu, f6B928(n, d32(28u * (uint32_t)i + 0x1104C8u)));
        avg = sum / sar16(0x112566u);
        if (avg < d32(4u * (uint32_t)lvl() + 0xC8D4Cu))
            avg = d32(4u * (uint32_t)lvl() + 0xC8D4Cu);
        th = avg + imul(d32(0x110600u), avg) / 20;
        th += (d32(28u * (uint32_t)i + 0x1104D4u) != 0 || d32(0x110600u) < -1) ? 0 : imul(avg, 5) / 20;
        th += imul(avg, imul(d32(4u * (uint32_t)n + 0x1106B0u), 40)) / 20;
        th -= (d32(0x1105FCu) > 0 && d32(0x110600u) < -1) ? imul(d32(0x1105FCu), avg) / 20 : 0;
        m = d32(28u * (uint32_t)i + 0x1104C8u);
        th += imul(sar24(r + (uint32_t)m + 0xA5u), avg) / 20;
        th += imul(avg, imul(sar24(r + (uint32_t)m + 0xBAu), 30)) / 20;
        if (d32(0x110600u) != 0) {
            h = f690AC(sar24((uint32_t)sar24(0xCA9A0u) + r + 0x99u), m);
            if (h < -1) {
                h = half(h) < -5 ? half(h) : -5;
                if (d32(0xF52D4u) == m)
                    th -= imul(avg, 3) / 20;
                if (sar24(199u * (uint32_t)m + 0xF1563u) == 3)
                    th += imul(h, avg) / 20;
            }
        }
        w32(28u * (uint32_t)i + 0x1104CCu, d32(28u * (uint32_t)i + 0x1104C0u) - th);
    }
    cnt = sar16(0x112566u);
    for (k = cnt - 2; k >= 0; k--)
        for (j = 0; j <= k; j++)
            if (d32(28u * (uint32_t)(j + 1) + 0x1104CCu) > d32(28u * (uint32_t)j + 0x1104CCu)) {
                memcpy(tmp, DP(28u * (uint32_t)(j + 1) + 0x1104BCu), 28);
                memcpy(DP(28u * (uint32_t)(j + 1) + 0x1104BCu), DP(28u * (uint32_t)j + 0x1104BCu), 28);
                memcpy(DP(28u * (uint32_t)j + 0x1104BCu), tmp, 28);
            }
    for (i = 0; i < sar16(0x112566u); i++)
        if (d32(28u * (uint32_t)i + 0x1104CCu) >= (g_mut == 11 ? 1 : 0)) {
            f75074(d32(28u * (uint32_t)i + 0x1104C4u));
            return;
        }
}

/* ============================================================================================================== */
/* The harness                                                                                                    */
/* ============================================================================================================== */

static uint8_t *g_pristine, *g_start, *g_after;
static int g_checks, g_fail, g_quiet;

enum { R_68730, R_68788, R_68A64, R_68B44, R_68C90, R_68FDC, R_6937C, R_698A4, R_6A5FC, R_6AD70, R_6AF34, R_6B0EC, R_N };
static const char *const RNAME[R_N] = { "68730", "68788", "68A64", "68B44", "68C90", "68FDC", "6937C", "698A4",
                                        "6A5FC", "6AD70", "6AF34", "6B0EC" };

struct call { int r, n, k, cnt, flag, c; int32_t truth; };

static void run(const struct call *cl, int model)
{
    int32_t t = cl->truth, out = 0;
    switch (cl->r) {
    case R_68730: model ? m_68730(cl->n) : f68730(cl->n); break;
    case R_68788: model ? m_68788(cl->n) : f68788(cl->n); break;
    case R_68A64: model ? m_68A64(cl->n, cl->k) : f68A64(cl->n, cl->k); break;
    case R_68B44: model ? m_68B44(cl->n) : f68B44(cl->n); break;
    case R_68C90: model ? m_68C90(cl->n) : f68C90(cl->n); break;
    case R_68FDC: model ? m_68FDC(cl->n) : f68FDC(cl->n); break;
    case R_6937C: model ? m_6937C(cl->n, &t, cl->cnt, cl->k, cl->flag) : f6937C(cl->n, &t, cl->cnt, cl->k, cl->flag);
                  tr("truth %d", t); break;
    case R_698A4: model ? m_698A4(cl->n) : f698A4(cl->n); break;
    case R_6A5FC: model ? m_6A5FC(cl->n, cl->k, &out, cl->c) : f6A5FC(cl->n, cl->k, &out, cl->c);
                  tr("out %d", out); break;
    case R_6AD70: model ? m_filter(cl->n, 0) : f6AD70(cl->n); break;
    case R_6AF34: model ? m_filter(cl->n, 1) : f6AF34(cl->n); break;
    case R_6B0EC: model ? m_6B0EC(cl->n) : f6B0EC(cl->n); break;
    default: break;
    }
}

/* runs the port and the model from the current state; 1 when they agree (the port's state is left) */
static int compare(const struct call *cl, const char *what)
{
    uint32_t a;
    size_t l;
    memcpy(g_start, g_ds, DS_SIZE);
    g_len[0] = g_len[1] = 0;
    g_log[0][0] = g_log[1][0] = 0;
    g_side = 0;
    run(cl, 0);
    memcpy(g_after, g_ds, DS_SIZE);
    memcpy(g_ds, g_start, DS_SIZE);
    g_side = 1;
    run(cl, 1);
    g_side = 0;
    g_checks++;
    if (strcmp(g_log[0], g_log[1]) != 0) {
        const char *p = g_log[0], *q = g_log[1];
        int line = 1;
        while (*p && *p == *q) {
            if (*p == '\n')
                line++;
            p++, q++;
        }
        if (!g_quiet) {
            l = strcspn(p, "\n");
            printf("FAIL %s %s n=%d (original %d): trace line %d\n  port : %.*s\n  model: %.*s\n", what,
                   RNAME[cl->r], cl->n, g_original, line, (int)l, p, (int)strcspn(q, "\n"), q);
        }
        g_fail++;
        return 0;
    }
    for (a = 0; a < DS_SIZE; a++)
        if (g_after[a] != g_ds[a]) {
            if (!g_quiet)
                printf("FAIL %s %s n=%d (original %d): byte 0x%X port %02X model %02X\n", what, RNAME[cl->r], cl->n,
                       g_original, (unsigned)(a + DS_BASE), g_after[a], g_ds[a]);
            g_fail++;
            return 0;
        }
    memcpy(g_ds, g_after, DS_SIZE);
    return 1;
}

static void expect(int ok, const char *what)
{
    g_checks++;
    if (!ok) {
        g_fail++;
        printf("FAIL A: %s\n", what);
    }
}

/* ---- random states ---------------------------------------------------------------------------------------------- */

static uint32_t g_rs = 12345;
static uint32_t rnd(void) { g_rs = g_rs * 1103515245u + 12345u; return (g_rs >> 8) & 0xFFFFFFu; }
static int srnd(int n) { return n > 0 ? (int)(rnd() % (uint32_t)n) : 0; }

static int g_nsaves;
static uint8_t *g_saves[16];

static void randomize(void)
{
    int n, i, k;
    uint32_t rec;

    memcpy(g_ds, g_saves[srnd(g_nsaves)], DS_SIZE);
    for (n = 0; n <= 9; n++) {
        rec = NAT(n);
        if (n > 0)
            setD8(rec + 0x9Au, (uint8_t)(srnd(5) == 0 ? 0 : srnd(3) == 0 ? 3 : 2));
        for (i = 1; i <= 18; i++)
            setD8(rec + (uint32_t)i, (uint8_t)(srnd(3) == 0 ? srnd(5) : 0));
        k = srnd(4);
        setD8(rec + 0x13u, (uint8_t)k);
        for (i = 0; i < 8; i++)
            setD8(rec + 0x14u + (uint32_t)i, (uint8_t)(i < k ? 0x13 + srnd(12) : 0));
        for (i = 0x24; i <= 0x3B; i++)
            setD8(rec + (uint32_t)i, (uint8_t)(srnd(3) == 0));
        setD8(rec + 0x51u, (uint8_t)srnd(10));
        setD8(rec + 0xBBu, (uint8_t)srnd(256));
        setD8(rec + 0xBCu, (uint8_t)srnd(3));
        for (i = 0; i < 13; i++)
            setD8(rec + 0x9Cu + (uint32_t)i, 0);
        if (srnd(2)) {
            setD8(rec + 0x9Cu, (uint8_t)(1 + srnd(18)));
            setD8(rec + 0x9Du, (uint8_t)(1 + srnd(18)));
            setD8(rec + 0x9Eu, (uint8_t)(srnd(4) ? 1 + srnd(18) : 0));
            for (i = 5; i < 10; i++)
                setD8(rec + 0x9Cu + (uint32_t)i, (uint8_t)(srnd(2) ? 1 + srnd(18) : 0));
            setD8(rec + 0xA6u, (uint8_t)(srnd(4) == 0 ? 0x13 + srnd(12) : srnd(19)));
        }
        for (i = 0; i < 10; i++) {
            setD8(rec + 0xA8u + (uint32_t)i, (uint8_t)(srnd(7) - 3));
            setD8(rec + 0xB1u + (uint32_t)i, (uint8_t)(srnd(140) - 20));
            setD8(rec + 0xBDu + (uint32_t)i, (uint8_t)(srnd(4) == 0));
        }
        setD32(0xF590Cu + 4u * (uint32_t)n, (uint32_t)(srnd(10) - 1));
        setD32(0x1105D4u + 4u * (uint32_t)n, (uint32_t)srnd(9));
        setD32(0x1105ACu + 4u * (uint32_t)n, (uint32_t)(srnd(7) - 1));
        setD32(0x1106B0u + 4u * (uint32_t)n, (uint32_t)srnd(2));
        setD16(0x112C9Au + 2u * (uint32_t)n, (uint16_t)srnd(12000));
        for (i = 0; i < 10; i++)
            setD32(0xF52E0u + 40u * (uint32_t)n + 4u * (uint32_t)i, (uint32_t)(srnd(11) - 5));
    }
    setD16(0xF5476u, (uint16_t)(1 + srnd(3)));
    setD8(0xE1623u, (uint8_t)((srnd(5) + 2) << 4 | (1 + srnd(3))));
    setD16(0xE1624u, (uint16_t)rnd());
    setD8(0xF4102u, srnd(2) ? 0x0C : 0x0D);
    setD32(0xF52D4u, (uint32_t)(1 + srnd(9)));
    setD16(0xF40FCu, (uint16_t)(1 + srnd(12)));
    for (i = 0; i < 0x1C2; i++)
        setD16(0xEE9F0u + 2u * (uint32_t)i, (uint16_t)srnd(3000));
    setD32(SEED, rnd() << 8 | (uint32_t)srnd(256));
    setD32(MINING, (uint32_t)srnd(2));
    /* the public offers 0x110ECC */
    k = srnd(9);
    setD32(0x112560u, (uint32_t)k);
    for (i = 0; i < 8; i++) {
        uint32_t o = 0x110ECCu + 21u * (uint32_t)i;
        int j;
        for (j = 0; j < 21; j++)
            setD8(o + (uint32_t)j, 0);
        if (i >= k)
            continue;
        for (j = 0; j < 3; j++)
            setD8(o + (uint32_t)j, (uint8_t)(srnd(4) ? 1 + srnd(18) : 0));
        for (j = 5; j < 10; j++)
            setD8(o + (uint32_t)j, (uint8_t)(srnd(2) ? 1 + srnd(18) : 0));
        setD8(o + 0x12u, (uint8_t)(1 + srnd(9)));
        setD8(o + 0x14u, 0xFF);
    }
    /* counter-offer rows */
    k = srnd(10) == 0 ? 55 + srnd(30) : srnd(16);
    memset(DP(0x111118u), 0, 0x380);
    setD16(0x112548u, (uint16_t)k);
    for (i = 0; i < k; i++) {
        uint32_t r = 0x111118u + 16u * (uint32_t)i;
        int j;
        for (j = 0; j < 8; j++)
            setD8(r + (uint32_t)j, (uint8_t)(srnd(3) ? srnd(19) : 0));
        setD8(r + 0xDu, (uint8_t)(1 + srnd(9)));
        setD8(r + 0xEu, (uint8_t)(1 + srnd(9)));
    }
    /* responses */
    k = srnd(8);
    setD16(0x112568u, (uint16_t)k);
    for (i = 0; i < 8; i++) {
        uint32_t r = 0x111498u + 20u * (uint32_t)i;
        int j;
        for (j = 0; j < 16; j++)
            setD8(r + (uint32_t)j, (uint8_t)(j < 8 ? (srnd(3) ? srnd(19) : 0) : 0));
        setD8(r + 0xDu, (uint8_t)(1 + srnd(9)));
        setD8(r + 0xEu, (uint8_t)(1 + srnd(9)));
    }
    /* stale scratch: candidates, accept table, offer copy, list */
    for (i = 0; i < 16; i++) {
        uint32_t c = 0x1103F4u + 28u * (uint32_t)i;
        setD32(c + 0x00u, (uint32_t)srnd(4));
        setD32(c + 0x04u, 3);
        setD32(c + 0x08u, (uint32_t)(srnd(200) - 100));
        setD32(c + 0x0Cu, (uint32_t)(1 + srnd(9)));
        setD32(c + 0x10u, (uint32_t)srnd(8));
        setD32(c + 0x14u, srnd(4) == 0 ? (uint32_t)(srnd(3) - 1) : 0u);
        setD32(c + 0x18u, (uint32_t)(srnd(200) - 50));
    }
    setD32(0x1104B8u, (uint32_t)srnd(8));
    setD32(0x110600u, (uint32_t)(srnd(3) == 0 ? -srnd(16) : 0));
    setD32(0x110604u, (uint32_t)(srnd(3) == 0 ? 0x13 + srnd(12) : 0));
    for (i = 0; i < 9; i++)
        setD8(0x1105A0u + (uint32_t)i, (uint8_t)(srnd(3) != 0));
    setD32(0x11059Cu, (uint32_t)srnd(4));
    setD8(0xE1625u, (uint8_t)rnd());
}

static void random_call(struct call *cl)
{
    int n;
    memset(cl, 0, sizeof *cl);
    cl->r = srnd(R_N);
    do
        n = 1 + srnd(9);
    while (D8(NAT(n) + 0x9Au) == 0 && srnd(4));
    cl->n = n;
    cl->k = srnd(8);
    cl->cnt = srnd(5) == 0 ? srnd(3) : 1;
    cl->flag = srnd(3) == 0;
    cl->c = srnd(2);
    cl->truth = srnd(2);
    if (cl->r == R_6937C || cl->r == R_6A5FC || cl->r == R_68C90 || cl->r == R_68B44) {
        /* these expect the ranked list of n (as their callers leave it) */
        f68FDC(n);
        f68788(n);
        if (cl->r != R_68B44 && srnd(2))
            f68B44(n);
    }
}

/* ---- part A ----------------------------------------------------------------------------------------------------- */

static void part_a(void)
{
    uint32_t r1 = NAT(1), r2 = NAT(2);
    int i, draws;
    struct call cl;
    uint32_t seed;

    memcpy(g_ds, g_saves[0], DS_SIZE);
    for (i = 1; i <= 9; i++)
        setD8(NAT(i) + 0x9Au, 0);
    setD8(r1 + 0x9Au, 2);
    setD8(r2 + 0x9Au, 3);
    for (i = 1; i <= 0x13; i++) {
        setD8(r1 + (uint32_t)i, 0);
        setD8(r2 + (uint32_t)i, 0);
    }
    setD8(r1 + 1, 2); setD8(r1 + 4, 1); setD8(r1 + 12, 3); setD8(r1 + 17, 1);
    setD8(r1 + 0x29u, 1);
    setD8(r2 + 18, 1);                        /* nation 2 holds a card of stack 8: every stack counts */
    setD32(0xF590Cu + 8u, (uint32_t)-1);
    setD16(0xF5476u, 2);
    f68788(1);
    expect(S32(0x1103ECu) == 4, "68788 count 4");
    expect(S32(0x1101ACu) == 12 && S32(0x1101B0u) == 150 && S32(0x1101B8u) == 4, "68788 entry 0: card 12, 150, used 4");
    expect(S32(0x1101BCu) == 17 && S32(0x1101C0u) == 36, "68788 entry 1: card 17, 36");
    expect(S32(0x1101CCu) == 4 && S32(0x1101D0u) == 18 && S32(0x1101D8u) == 2, "68788 entry 2: card 4 (Mining), 18");
    expect(S32(0x1101DCu) == 1 && S32(0x1101E0u) == 4 && S32(0x1101E4u) == 2, "68788 entry 3: card 1, 4");

    /* 0x68B44: a one-card list worth < 3 gives the want count -1 */
    for (i = 1; i <= 18; i++)
        setD8(r1 + (uint32_t)i, 0);
    setD8(r1 + 5, 1);
    setD8(r1 + 0x13u, 0);
    f68FDC(1);
    f68788(1);
    setD32(0x11059Cu, 7);
    f68B44(1);
    expect(S32(0x1105ACu + 4u) == -1, "68B44 want count -1");
    expect(S32(0x11059Cu) == 0, "68B44 first entry 0 for k < 0");

    /* 0x6937C truthful declare: slot 2 = slot 10, no draw */
    setD8(0xF4102u, 0x0C);
    setD8(r1 + 0xA6u, 9);
    setD8(r1 + 0x9Eu, 0);
    seed = D32(SEED);
    {
        int32_t t = 1;
        f6937C(1, &t, 1, 0, 0);
    }
    expect(D8(r1 + 0x9Eu) == 9 && D32(SEED) == seed, "6937C truthful: slot 2 = slot 10, no game_rand");

    /* 0x68C90's draws with a bad calamity: 1 + f6937C's 1 + one per wanted card (k = 3) */
    for (i = 1; i <= 18; i++)
        setD8(r1 + (uint32_t)i, (uint8_t)(i <= 9 ? 1 : 0));
    setD8(r1 + 0x13u, 1);
    setD8(r1 + 0x14u, 0x1E);                 /* Piracy -10 */
    setD8(r2 + 3, 1); setD8(r2 + 10, 2);
    for (i = 0; i < 13; i++)
        setD8(r1 + 0x9Cu + (uint32_t)i, 0);
    f68FDC(1);
    f68788(1);
    f68B44(1);
    setD32(0x1105ACu + 4u, 3);
    memset(&cl, 0, sizeof cl);
    cl.r = R_68C90;
    cl.n = 1;
    g_len[0] = 0;
    g_log[0][0] = 0;
    g_side = 0;
    run(&cl, 0);
    draws = 0;
    {
        const char *p = g_log[0];
        while ((p = strstr(p, "rand")) != NULL) {
            draws++;
            p += 4;
        }
    }
    expect(draws == 5, "68C90 with a bad calamity: 1 + 1 + 3 draws");
    expect(D8(r1 + 0xA6u) == 0x1E, "68C90: the calamity in slot 10");

    /* 0x6AF34 clears 0x110608, not the responses */
    memset(DP(0x110608u), 0x55, 21);
    setD16(0x112568u, 1);
    memset(DP(0x111498u), 0, 20);
    setD8(0x111498u, 15);                    /* nobody else holds stack 7 */
    setD8(0x1114A6u, 2);
    for (i = 1; i <= 18; i++)
        setD8(r2 + (uint32_t)i, 0);
    f6AF34(1);
    expect(D8(0x111498u) == 15 && D8(0x110608u) == 0 && D8(0x110609u) == 0x55, "6AF34 clears 0x110608 + q, not 0x111498");

    g_original = 0;
}

/* ---- part A2: 0x698A4's pruning with more than 56 rows (item 4), on biased random states ------------------------ */

static void part_a2(void)
{
    int mode, tries, i, j, before_rows, big, pruned, ok_rows;
    struct call cl;
    uint32_t r1 = NAT(1);

    for (mode = 0; mode <= 1; mode++) {
        g_original = mode;
        big = pruned = ok_rows = 0;
        for (tries = 0; tries < 30000 && big < 12; tries++) {
            int p0 = m_pruned, b0 = m_pruned_big;
            randomize();
            for (i = 1; i <= 9; i++)
                setD8(NAT(i) + 0x9Au, 2);
            setD8(r1 + 0x13u, 1);
            setD8(r1 + 0x14u, 0x1E);                 /* Piracy: [0x110600] = -10, so out = 1 */
            for (i = 1; i <= 18; i++)
                setD8(r1 + (uint32_t)i, (uint8_t)(i <= 8 ? 2 + srnd(2) : 0));   /* low cards to give */
            setD32(0x1105ACu + 4u, 3);
            setD32(0x1105D4u + 4u, 8);                /* no +500 */
            setD32(0xF52D4u, 0);
            memset(DP(0xEE9F0u), 0, 0x384);           /* no score gaps */
            for (i = 5; i < 10; i++)
                setD8(r1 + 0x9Cu + (uint32_t)i, (uint8_t)(13 + srnd(6)));
            for (j = 2; j <= 9; j++)
                for (i = 5; i < 10; i++)
                    setD8(NAT(j) + 0x9Cu + (uint32_t)i, (uint8_t)(1 + srnd(8)));
            for (i = 0; i < 10; i++) {
                setD8(r1 + 0xA8u + (uint32_t)i, (uint8_t)-3);
                setD8(r1 + 0xB1u + (uint32_t)i, 100);
                setD8(r1 + 0xBDu + (uint32_t)i, 0);
            }
            setD32(0x112560u, 8);
            for (i = 0; i < 8; i++) {
                uint32_t o = 0x110ECCu + 21u * (uint32_t)i;
                for (j = 0; j < 3; j++)
                    setD8(o + (uint32_t)j, (uint8_t)(13 + srnd(6)));   /* high cards offered */
                setD8(o + 0x12u, (uint8_t)(2 + i));
            }
            before_rows = 57 + srnd(25);
            setD16(0x112548u, (uint16_t)before_rows);
            for (i = 0; i < before_rows; i++) {
                uint32_t r = 0x111118u + 16u * (uint32_t)i;
                memset(DP(r), 0, 16);
                setD8(r, (uint8_t)(1 + srnd(18)));
                setD8(r + 0xDu, (uint8_t)(2 + srnd(8)));
                setD8(r + 0xEu, (uint8_t)(2 + srnd(8)));
            }
            memset(&cl, 0, sizeof cl);
            cl.r = R_698A4;
            cl.n = 1;
            if (!compare(&cl, "A2"))
                continue;
            if (m_pruned > p0)
                pruned++;
            if (m_pruned_big > b0) {
                int mine = 0, others = 0;
                big++;
                for (i = 0; i < S16(0x112548u); i++) {
                    if (S8(0x111118u + 16u * (uint32_t)i + 0xEu) == 1)
                        mine++;
                    else
                        others++;
                }
                if (mode == 0) {
                    expect(mine == 3 && others == before_rows, "698A4 fixed: three of n's rows kept, the others intact");
                    ok_rows++;
                }
            }
        }
        printf("A2 (original %d): %d states, %d pruned, %d with more than 56 rows\n", mode, tries, pruned, big);
        expect(big > 0, "A2 reached the pruning with more than 56 rows");
    }
    (void)ok_rows;
    g_original = 0;
}

/* ---- part C: the three steps on a real save ------------------------------------------------------------------- */

static int g_tot_offers, g_tot_rows, g_tot_resp, g_tot_comp;

static void part_c_save(int s, const char *name, int derived)
{
    int n, r, k, ok;
    struct call cl;
    char what[64];

    memcpy(g_ds, g_saves[s], DS_SIZE);
    if (derived) {
        /* every nation in play a computer, hands topped up to 7..9 commodities (seeded; calamities kept) */
        g_rs = 4242u + (uint32_t)s;
        for (n = 1; n <= 9; n++) {
            int cards = 0, i;
            if (D8(NAT(n) + 0x9Au) == 0)
                continue;
            setD8(NAT(n) + 0x9Au, 2);
            for (i = 1; i <= 0x12; i++)
                cards += S8(NAT(n) + (uint32_t)i);
            for (k = 7 + srnd(3); cards < k; cards++) {
                i = 1 + srnd(18);
                setD8(NAT(n) + (uint32_t)i, (uint8_t)(D8(NAT(n) + (uint32_t)i) + 1u));
            }
            if (srnd(3) == 0 && S8(NAT(n) + 0x13u) == 0) {
                setD8(NAT(n) + 0x13u, 1);
                setD8(NAT(n) + 0x14u, (uint8_t)(0x13 + srnd(12)));
            }
        }
    }
    setD8(0xF4102u, 0x0C);
    for (n = 1; n <= 9; n++) {
        int cards = 0, i;
        setD32(0x1105ACu + 4u * (uint32_t)n, 0);
        memset(DP(NAT(n) + 0x9Cu), 0, 13);
        for (i = 1; i <= 0x13; i++)
            cards += S8(NAT(n) + (uint32_t)i);
        if (D8(NAT(n) + 0x9Au) != 2 || cards < 6)
            continue;
        g_tot_comp++;
        memset(&cl, 0, sizeof cl);
        cl.r = R_68730;
        cl.n = n;
        snprintf(what, sizeof what, "C %s declare", name);
        if (!compare(&cl, what))
            return;
    }
    f726DC();
    g_tot_offers += S32(0x112560u);
    setD8(0xF4102u, 0x0D);
    memset(DP(0x111118u), 0, 0x380);
    setD16(0x112548u, 0);
    for (n = 1; n <= 9; n++) {
        if (D8(NAT(n) + 0x9Au) != 2 || S32(0x1105ACu + 4u * (uint32_t)n) == 0)
            continue;
        memset(&cl, 0, sizeof cl);
        cl.r = R_698A4;
        cl.n = n;
        snprintf(what, sizeof what, "C %s counter", name);
        if (!compare(&cl, what))
            return;
    }
    g_tot_rows += S16(0x112548u);
    setD8(0xF4102u, 0x0E);
    for (n = 1; n <= 9; n++) {
        if (D8(NAT(n) + 0x9Au) != 2 || S8(NAT(n) + 0x9Cu) == 0)
            continue;
        k = 0;
        memset(DP(0x111498u), 0, 0x8C);
        for (r = 0; r < S16(0x112548u) && k < 7; r++)
            if (S8(0x111118u + 16u * (uint32_t)r + 0xDu) == n) {
                memcpy(DP(0x111498u + 20u * (uint32_t)k), DP(0x111118u + 16u * (uint32_t)r), 16);
                setD8(0x111498u + 20u * (uint32_t)k + 0x12u, (uint8_t)r);
                k++;
            }
        setD16(0x112568u, (uint16_t)k);
        g_tot_resp += k;
        memset(&cl, 0, sizeof cl);
        cl.r = R_6B0EC;
        cl.n = n;
        snprintf(what, sizeof what, "C %s accept", name);
        ok = compare(&cl, what);
        if (!ok)
            return;
    }
}

/* ---- main ------------------------------------------------------------------------------------------------------- */

static int load_save(const char *path, uint8_t **dst)
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
    *dst = malloc(DS_SIZE);
    if (*dst == NULL)
        return 0;
    memcpy(*dst, g_ds, DS_SIZE);
    return 1;
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:\\work\\advciv_run";
    const char *playdir = argc > 2 ? argv[2] : "C:\\work\\advciv_play";
    static const char *const GSAVES[] = { "savgame0.dat", "savgame3.dat", "savgame4.dat", "savgame5.dat",
                                          "savgame6.dat", "savgame7.dat", "savgame8.dat", "savgame9.dat" };
    const char *names[16];
    char path[1024];
    int i, f0, c0, mut, caught, b;
    struct call cl;

    if (state_init(gamedir) != 0) {
        printf("cannot load the data object from %s\n", gamedir);
        return 2;
    }
    g_pristine = malloc(DS_SIZE);
    g_start = malloc(DS_SIZE);
    g_after = malloc(DS_SIZE);
    g_log[0] = malloc(LOGSZ);
    g_log[1] = malloc(LOGSZ);
    if (!g_pristine || !g_start || !g_after || !g_log[0] || !g_log[1])
        return 2;
    memcpy(g_pristine, g_ds, DS_SIZE);
    snprintf(path, sizeof path, "%s\\SAVGAMEA.DAT", playdir);
    if (load_save(path, &g_saves[g_nsaves]))
        names[g_nsaves++] = "SAVGAMEA.DAT";
    for (i = 0; i < 8; i++) {
        snprintf(path, sizeof path, "%s\\%s", gamedir, GSAVES[i]);
        if (load_save(path, &g_saves[g_nsaves]))
            names[g_nsaves++] = GSAVES[i];
    }
    if (g_nsaves == 0) {
        printf("no saves found\n");
        return 2;
    }
    printf("saves: %d\n", g_nsaves);
    for (i = 0; i < g_nsaves; i++) {
        int n, j, cards;
        memcpy(g_ds, g_saves[i], DS_SIZE);
        printf("  %s phase %d session %d, status/cards:", names[i], D8(0xF4102u), S16(0xF5476u));
        for (n = 1; n <= 9; n++) {
            for (cards = 0, j = 1; j <= 0x13; j++)
                cards += S8(NAT(n) + (uint32_t)j);
            printf(" %d/%d", D8(NAT(n) + 0x9Au), cards);
        }
        printf("\n");
    }

    part_a();
    part_a2();
    printf("A: %d checks, %d failures\n", g_checks, g_fail);

    f0 = g_fail;
    c0 = g_checks;
    for (i = 0; i < 40000; i++) {
        randomize();
        g_original = i & 1;
        random_call(&cl);
        compare(&cl, "B");
        if (g_fail - f0 > 20)
            break;
    }
    g_original = 0;
    printf("B: %d checks, %d failures\n", g_checks - c0, g_fail - f0);

    f0 = g_fail;
    c0 = g_checks;
    for (b = 0; b <= 1; b++)
        for (i = 0; i < g_nsaves; i++)
            for (g_original = 0; g_original <= 1; g_original++)
                part_c_save(i, names[i], b);
    g_original = 0;
    printf("C: %d checks, %d failures (computers declaring %d, offers %d, rows %d, responses %d)\n", g_checks - c0,
           g_fail - f0, g_tot_comp, g_tot_offers, g_tot_rows, g_tot_resp);

    f0 = g_fail;
    caught = 0;
    for (mut = 1; mut <= 11; mut++) {
        int before = g_fail, saved_checks = g_checks;
        g_mut = mut;
        g_quiet = 1;
        g_rs = 777u + (uint32_t)mut;
        for (b = 0; b < 20000 && g_fail == before; b++) {
            randomize();
            g_original = b & 1;
            random_call(&cl);
            compare(&cl, "D");
        }
        g_original = 0;
        if (g_fail > before) {
            caught++;
            g_fail = before;
        } else {
            printf("FAIL D: mutation %d not caught\n", mut);
            g_fail++;
        }
        g_checks = saved_checks + 1;
    }
    g_mut = 0;
    g_quiet = 0;
    printf("D: %d of 11 mutations caught\n", caught);
    printf("total: %d checks, %d failures\n", g_checks, g_fail);
    return g_fail == 0 ? 0 : 1;
}
