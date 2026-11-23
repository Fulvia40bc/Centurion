/* cal5_test [GAMEDIR [SAVEDIR]]: unit checks for module CAL5 (cal5.c, the Barbarian Hordes) that need no game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init; default C:\work\advciv_run, only read), TERRFLAG.DAT (the
   territory records: landing zones and population limits are not in the save; f12394), LANDADJ.DAT / COASTADJ.DAT
   (the land / coast lists, parsed here and put into heap blocks behind the handles 0xF8E7C / 0xF90FC as 0x3EED8 does,
   with the neighbours not in play dropped by a mask the test chooses) and the saves savgame0.dat (start of MOVEMENT
   R4) and savgame1.dat (R4, all human); SAVEDIR holds SAVGAMEA.DAT and savgameb.dat (default C:\work\advciv_play, only
   read). The four saves are read with saveload.c's f5F05C as the game does.

   cal5.c calls nothing that is not ported (its file comment), so it is built as it is. The primitives it calls are the
   real, separately tested ones: move.c's f57CE8 / f57E28, conflict.c's f5B224 / f5B588 / f5CDE8 (with rules.c,
   mapdata.c), load.c's heap. Every other callee is a stub below that appends one line to a trace: the messages
   f79270 (with their texts), the waits f78A20, the drawing, the sound f7B504, cal1.c's f1E69C; game_rand f78AE4 is
   the LCG of game.h on the seed [0xCAC5C] (so every draw shows in the data object); cal1.c's f207B4 (the human
   director's choice, kind 0x1A) answers from a seeded stream with an entry of the list 0xF7CA0; f7BA64 says
   "unattended" so the losses box f5CDE8 waits 250 ms instead of a click.

   The model is a second implementation of the nine routines written from the listing (the disassembly at
   0x1C3D0..0x1E06B, 0x217A0..0x21912, 0x7A4F0, 0x7AB24; not from cal5.c): 0x1C9FC's local frame is modelled as the
   bytes [ebp-0x50]..[ebp-0x29] with a mask of the bytes the code writes, 0x1D858's entries as records, the route
   buffer as the data object's bytes. It runs on a copy of the data object (g_ds pointed at the copy), calls the same
   stubs and primitives, and so writes the trace it expects. After each run the two traces, the number of game_rand
   calls and the whole data object (the heap handles included: both sides allocate and free in the same order) must be
   equal. Parts:
     A. hand vectors (expected values written out): 0x1DFFC (BUG 1, both modes), 0x1DEAC's order with ties, the
        snapshot round trip, 0x217A0's director (giver, score, the victim skipped, nobody: BUG 2 in both modes) and
        Crete's message, 0x1C9FC's three lists (tokens / city 6, empty zones, all zones), BUG 3b with Iberia's and
        Egypt's six zones each holding as many tokens as the low nibble of land[0] (--original lists territory 0 for
        each land[] pair read as an entry while its low nibble still matches, then n), BUG 3a (ten
        equal route entries: territory 0 from the visited bytes, --original), the "no longer any damage" and "finally
        settle" paths, the invasion with the losses box, the BRPC (deadlocked) and "wiped out".
     B. f217A0 on random states built from the four saves: a victim 1..9 (human or computer), its tokens and cities in
        and around its landing zones, other nations' tokens and cities, barbarians, stocks, the director (a giver, or
        none), PBEM, sound on / off, the adjacency mask; each run with and without --original.
     C. f217A0 for every victim 1..9 on each of the four saves as they are (the nation put in play if it is not),
        against the model; the landing list, the messages and the moves printed (for the DOSBox-X check plan).
     D. deliberately broken models (one change each) must be caught by part B's comparison.
   Prints each failure (the first differing trace line or byte), the counts per part, and exits 0 when all pass.

   Build (from the repo root):
     clang -std=c11 -Wall -Wextra -Iport/src port/test/cal5_test.c port/src/cal5.c port/src/conflict.c
       port/src/move.c port/src/rules.c port/src/mapdata.c port/src/load.c port/src/saveload.c port/src/state.c
       port/src/le_image.c port/src/snd.c -o cal5_test.exe */

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <direct.h>
#define chdir _chdir
#define getcwd _getcwd

#include "cal1.h"
#include "cal5.h"
#include "conflict.h"
#include "fg.h"
#include "game.h"
#include "kit.h"
#include "map.h"
#include "mapdata.h"
#include "menu2.h"
#include "move.h"
#include "panel.h"
#include "rules.h"
#include "saveload.h"
#include "start.h"
#include "state.h"
#include "ui.h"

/* ---- addresses (from the listing, written out here) ------------------------------------------------------------ */

#define NA(n, o)    (0xF14CCu + 199u * (uint32_t)(n) + (uint32_t)(o))
#define AR(r, o)    (0xE31B8u + 69u * (uint32_t)(r) + (uint32_t)(o))
#define TS(t, o)    (0xE2C18u + 3u * (uint32_t)(t) + (uint32_t)(o))
#define TF(t, o)    (0xE1638u + 35u * (uint32_t)(t) + (uint32_t)(o))
#define CREC(k, o)  (0xF77E4u + 3u * (uint32_t)(k) + (uint32_t)(o))
#define RT          0xF6E94u
#define VIS         0xF6EBCu
#define DEP         0xF6E8Cu
#define SWP         0xF6E90u
#define BCALLS      0xF6F5Cu
#define BMODE       0xF6F60u
#define LST         0xF7CA0u
#define LSTN        0xF7C9Cu
#define LOSST       0xF547Cu
#define LOSSC       0xF548Cu
#define SNA         0xEED74u
#define SNN         0xEED78u
#define SNT         0xEED7Cu

/* ---- byte access on the current data object --------------------------------------------------------------------- */

static int      rb(uint32_t a) { return g_ds[a - DS_BASE]; }
static int      sb(uint32_t a) { return (int8_t)g_ds[a - DS_BASE]; }
static int      s16(uint32_t a) { return (int16_t)(g_ds[a - DS_BASE] | g_ds[a + 1 - DS_BASE] << 8); }
static int32_t  s32(uint32_t a)
{
    return (int32_t)((uint32_t)g_ds[a - DS_BASE] | (uint32_t)g_ds[a + 1 - DS_BASE] << 8 |
                     (uint32_t)g_ds[a + 2 - DS_BASE] << 16 | (uint32_t)g_ds[a + 3 - DS_BASE] << 24);
}
static void w8(uint32_t a, int v) { g_ds[a - DS_BASE] = (uint8_t)v; }
static void w16(uint32_t a, int v) { g_ds[a - DS_BASE] = (uint8_t)v; g_ds[a + 1 - DS_BASE] = (uint8_t)(v >> 8); }
static void w32(uint32_t a, int32_t v)
{
    uint32_t u = (uint32_t)v;
    g_ds[a - DS_BASE] = (uint8_t)u; g_ds[a + 1 - DS_BASE] = (uint8_t)(u >> 8);
    g_ds[a + 2 - DS_BASE] = (uint8_t)(u >> 16); g_ds[a + 3 - DS_BASE] = (uint8_t)(u >> 24);
}
static const char *str(uint32_t a) { return (const char *)(g_ds + (a - DS_BASE)); }
static const char *nm(int n) { return str((uint32_t)s32(0xC768Cu + 4u * (uint32_t)n)); }
static const char *tn(int t) { return str((uint32_t)s32(0xC77D8u + 4u * (uint32_t)t)); }

/* ---- traces, checks ----------------------------------------------------------------------------------------------- */

#define LOGSZ (1u << 22)
static char *g_tp, *g_tm, *g_log;          /* the port's and the model's traces; the current one */
static size_t g_tp_len, g_tm_len, *g_len;
static int g_tests, g_fail;

static void tr(const char *fmt, ...)
{
    va_list ap;
    int k;
    if (*g_len + 700 >= LOGSZ)
        return;
    va_start(ap, fmt);
    k = vsnprintf(g_log + *g_len, LOGSZ - *g_len, fmt, ap);
    va_end(ap);
    if (k > 0)
        *g_len += (size_t)k;
    g_log[(*g_len)++] = '\n';
    g_log[*g_len] = 0;
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


/* ---- the human director's stream, game_rand ----------------------------------------------------------------------- */

static uint32_t g_pick_port, g_pick_model, *g_pick;
static int g_rand_port, g_rand_model, *g_rand;

int f78AE4(void)
{
    uint32_t s = (uint32_t)s32(0xCAC5Cu) * 0x15A4E35u + 1u;
    w32(0xCAC5Cu, (int32_t)s);
    (*g_rand)++;
    tr("rand %d", (int)((s >> 16) & 0x7FFF));
    return (int)((s >> 16) & 0x7FFF);
}

int f207B4(int p, int v, int kind)
{
    int32_t cnt = s32(LSTN), t;
    *g_pick = *g_pick * 1103515245u + 12345u;
    t = cnt > 0 ? s32(LST + 4u * ((*g_pick >> 16) % (uint32_t)cnt)) : -1;
    tr("207B4(%d,%d,0x%X) list %d -> %d", p, v, kind, cnt, t);
    return t;
}

/* ---- stubs (trace lines; no data effect unless stated) ---------------------------------------------------------- */

void f79270(int wait, int n, int t, const char *text, int anim) { tr("79270(%d,%d,%d,[%s],%d)", wait, n, t, text, anim); }
void f78A20(int t) { tr("78A20(%d)", t); }
void f78884(void) { tr("78884"); }
void f788AC(void) { tr("788AC"); }
void f7B504(int s, const char *name) { tr("7B504(%d,%s)", s, name); }
void f1E69C(int k) { tr("1E69C(%d)", k); }
int fg_vbopen(int handle) { tr("vbopen(%X)", (unsigned)handle); return 0; }
void fg_vbclose(void) { tr("vbclose"); }
int fg_vbcopy(int a, int b, int c, int d, int e, int f, int s, int t)
{
    tr("vbcopy(%d,%d,%d,%d,%d,%d,%X,%X)", a, b, c, d, e, f, (unsigned)s, (unsigned)t);
    return 0;
}
int fg_setcolor(int c) { tr("setcolor(%d)", c); return 0; }
int fgf_width(const char *s, int n) { (void)s; return 8 * n; }
void f3CD6C(int8_t pic) { tr("3CD6C(%d)", pic); }
void f3C3D0(uint32_t dlg, int id) { tr("off(%X,%X)", (unsigned)dlg, id); }
void f3C314(uint32_t dlg, int id) { tr("on(%X,%X)", (unsigned)dlg, id); }
void f459E4(int n) { tr("459E4(%d)", n); }
void f3A924(void) { tr("3A924"); }
void f3A990(void) { tr("3A990"); }
void f3A9F4(const char *text) { tr("3A9F4[%s]", text); }
void f7844C(uint32_t dlg, int base) { tr("7844C(%X,%d)", (unsigned)dlg, base); }
void f3CBBC(void) { tr("3CBBC"); }
void f78664(uint32_t dlg) { tr("78664(%X)", (unsigned)dlg); }
void f7853C(uint32_t dlg) { tr("7853C(%X)", (unsigned)dlg); }
void f7876C(void) { tr("7876C"); }
void f3B05C(uint32_t dlg) { tr("3B05C(%X)", (unsigned)dlg); }
int f7BA64(void) { tr("7BA64"); return 1; }
void f3AF8C(uint32_t spr, int x, int y) { tr("3AF8C(%X,%d,%d)", (unsigned)spr, x, y); }
void f3C9D0(int x, int y, int w, int h, int flags) { tr("3C9D0(%d,%d,%d,%d,%d)", x, y, w, h, flags); }
void f3A0B0(int x, int y, int w, const char *s) { tr("3A0B0(%d,%d,%d,[%s])", x, y, w, s); }
void f3A3A0(int x, int y, const char *s) { tr("3A3A0(%d,%d,[%s])", x, y, s); }
void f3A044(int x, int y, const char *s) { tr("3A044(%d,%d,[%s])", x, y, s); }
void f783B8(uint32_t dlg, int base) { tr("783B8(%X,%d)", (unsigned)dlg, base); }
void f3D52C(int dx, int dy) { tr("3D52C(%d,%d)", dx, dy); }
void f787F8(int t) { tr("787F8(%d)", t); }
int f78BA4(uint32_t dlg, int map) { tr("UNEXPECTED 78BA4(%X,%d)", (unsigned)dlg, map); return -1; }
_Noreturn void not_ported(uint32_t addr)
{
    printf("FAIL not_ported(0x%X)\n", (unsigned)addr);
    exit(2);
}

/* module ANIM's 0x7A4C4 (anim.c, own test): conflict.c's f5CDE8 calls it only with an animation id, never from CAL5
   (always eax = 0), so reaching it is a failure */
void f7A4C4(int anim) { (void)anim; not_ported(0x7A4C4u); }
void (*g_not_ported_hook)(uint32_t addr);
int f3B9A4(uint32_t dlg, int id) { (void)dlg; (void)id; tr("UNEXPECTED 3B9A4"); return 0; }
void f3B7CC(uint32_t dlg, int id) { (void)dlg; (void)id; tr("UNEXPECTED 3B7CC"); }
void f3B870(uint32_t dlg, int id) { (void)dlg; (void)id; tr("UNEXPECTED 3B870"); }
void f3BFBC(uint32_t dlg, int id, int en) { (void)dlg; (void)id; (void)en; tr("UNEXPECTED 3BFBC"); }
void f426D0(int i) { (void)i; tr("UNEXPECTED 426D0"); }
void f572CC(int t) { (void)t; tr("UNEXPECTED 572CC"); }
void f3A780(const char *msg) { tr("UNEXPECTED 3A780[%s]", msg); }
_Noreturn void f44CAC(void) { printf("FAIL quit: %s\n", str(0xF5500u)); exit(2); }
/* aiship.c's f5BF30 (conflict.c's plunder by a computer, M7): not reached by these cases */
void f5BF30(int n, int amount, int show) { printf("FAIL f5BF30(%d, %d, %d)\n", n, amount, show); exit(2); }
void f76F70(int n, uint8_t *cards) { (void)n; (void)cards; }

/* ---- the land / coast lists ------------------------------------------------------------------------------------- */

static int g_raw_n[2][160], g_raw[2][160][32];   /* [0] land, [1] coast, as read */
static uint32_t g_adj_h[2];
static uint8_t g_mask[160];                      /* 1 = in play (kept in the lists) */

static int load_adj(const char *gamedir, const char *file, int w)
{
    char path[400];
    uint8_t b[4];
    FILE *fp;
    int t, i;

    snprintf(path, sizeof path, "%s/%s", gamedir, file);
    fp = fopen(path, "rb");
    if (fp == NULL)
        return 0;
    if (fread(b, 4, 1, fp) != 1) {
        fclose(fp);
        return 0;
    }
    for (t = 0; t < 160; t++) {
        if (fread(b, 2, 1, fp) != 1) {
            fclose(fp);
            return 0;
        }
        g_raw_n[w][t] = (int16_t)(b[0] | b[1] << 8);
        if (g_raw_n[w][t] < 0 || g_raw_n[w][t] > 32) {
            fclose(fp);
            return 0;
        }
        for (i = 0; i < g_raw_n[w][t]; i++) {
            if (fread(b, 2, 1, fp) != 1) {
                fclose(fp);
                return 0;
            }
            g_raw[w][t][i] = (int16_t)(b[0] | b[1] << 8);
        }
    }
    fclose(fp);
    return 1;
}

/* the lists by g_mask into the two blocks, handles at 0xF8E7C / 0xF90FC of the current data object */
static void set_adj(void)
{
    int w, t, i, cnt;
    for (w = 0; w < 2; w++) {
        uint8_t *blk;
        uint32_t off = 0;
        if (g_adj_h[w] == 0)
            g_adj_h[w] = load_heap_alloc(2u * (160u + 160u * 32u));
        blk = load_heap_ptr(g_adj_h[w], 2u * (160u + 160u * 32u));
        for (t = 0; t < 160; t++) {
            cnt = 0;
            for (i = 0; i < g_raw_n[w][t]; i++)
                if (g_mask[g_raw[w][t][i]] != 0) {
                    blk[off + 2u + 2u * (uint32_t)cnt] = (uint8_t)g_raw[w][t][i];
                    blk[off + 3u + 2u * (uint32_t)cnt] = (uint8_t)(g_raw[w][t][i] >> 8);
                    cnt++;
                }
            blk[off] = (uint8_t)cnt;
            blk[off + 1] = 0;
            w32((w == 0 ? 0xF8E7Cu : 0xF90FCu) + 4u * (uint32_t)t, (int32_t)(g_adj_h[w] + off));
            off += 2u + 2u * (uint32_t)g_raw_n[w][t];
        }
    }
}

/* the list of t (w 0 land, 1 coast) as the data object's handle shows it */
static int adj_n(int w, int t)
{
    const uint8_t *p = load_heap_ptr((uint32_t)s32((w == 0 ? 0xF8E7Cu : 0xF90FCu) + 4u * (uint32_t)t), 2);
    return p ? (int16_t)(p[0] | p[1] << 8) : 0;
}
static int adj_at(int w, int t, int i)
{
    const uint8_t *p = load_heap_ptr((uint32_t)s32((w == 0 ? 0xF8E7Cu : 0xF90FCu) + 4u * (uint32_t)t) + 2u + 2u * (uint32_t)i, 2);
    return p ? (int16_t)(p[0] | p[1] << 8) : 0;
}

/* ==== the model ================================================================================================== */

static int g_mut;   /* 0 = the listing; 1.. = deliberately broken (part D) */

static int m_barb(int t) { return sb(AR(t + 1, 0x19)); }
static int m_lim(int t) { return (rb(TF(t, 1)) >> 3) & 7; }
static int m_city_of(int t, int n) { return (rb(TS(t, 2)) & 1) && rb(TS(t, 0)) == n; }
static void m_push(int32_t v) { int32_t c = s32(LSTN); if (c >= 0 && c < 157) { w32(LST + 4u * (uint32_t)c, v); w32(LSTN, c + 1); } }

/* 0x1DFFC */
static int m_1DFFC(void)
{
    int s = 0, i;
    for (i = 1; i <= 10; i++)
        s += sb(LOSST + (uint32_t)i);
    for (i = 1; i <= 10 && g_mut != 1; i++) {
        int c = sb(LOSSC + (uint32_t)i);
        if (c == 0)
            continue;
        s += c;
        if (g_original)
            break;
    }
    return s;
}

/* 0x1DEAC: entries as 4-byte records at p */
static void m_1DEAC(int count, uint8_t *p)
{
    int pass, j;
    uint8_t tmp[4];
    for (pass = count - 2; pass >= 0; pass--)
        for (j = 0; j <= pass; j++)
            if ((p[4 * j + 4] & 15) > (p[4 * j] & 15)) {
                memcpy(tmp, p + 4 * j + 4, 4);
                memcpy(p + 4 * j + 4, p + 4 * j, 4);
                memcpy(p + 4 * j, tmp, 4);
                memcpy(g_ds + (SWP - DS_BASE), tmp, 4);
            }
    for (pass = 1; pass < count; pass++)
        for (j = count - 1; j >= pass; j--)
            if (g_mut == 2 ? p[4 * (j - 1) + 2] >= p[4 * j + 2] : p[4 * (j - 1) + 2] > p[4 * j + 2]) {
                memcpy(tmp, p + 4 * (j - 1), 4);
                memcpy(p + 4 * (j - 1), p + 4 * j, 4);
                memcpy(p + 4 * j, tmp, 4);
                memcpy(g_ds + (SWP - DS_BASE), tmp, 4);
            }
}

/* 0x7A4F0 / 0x7AB24 */
static void m_snap(void)
{
    uint32_t a = load_heap_alloc(0x2B20), nn = load_heap_alloc(0x88D), t = load_heap_alloc(0x1E0);
    w32(SNA, (int32_t)a);
    w32(SNN, (int32_t)nn);
    w32(SNT, (int32_t)t);
    memcpy(load_heap_ptr(a, 0x2B20), g_ds + (AR(1, 0) - DS_BASE), 0x2B20);
    memcpy(load_heap_ptr(t, 0x1E0), g_ds + (TS(0, 0) - DS_BASE), 0x1E0);
    memcpy(load_heap_ptr(nn, 0x88D), g_ds + (NA(1, 0) - DS_BASE), 0x88D);
}
static void m_unsnap(void)
{
    memcpy(g_ds + (AR(1, 0) - DS_BASE), load_heap_ptr((uint32_t)s32(SNA), 0x2B20), 0x2B20);
    memcpy(g_ds + (TS(0, 0) - DS_BASE), load_heap_ptr((uint32_t)s32(SNT), 0x1E0), 0x1E0);
    if (g_mut != 3)
        memcpy(g_ds + (NA(1, 0) - DS_BASE), load_heap_ptr((uint32_t)s32(SNN), 0x88D), 0x88D);
    load_heap_free((uint32_t)s32(SNA));
    w32(SNA, 0);
    load_heap_free((uint32_t)s32(SNN));
    w32(SNN, 0);
    load_heap_free((uint32_t)s32(SNT));
    w32(SNT, 0);
}

/* 0x1D858: entries as records, written back as bytes */
typedef struct { uint8_t b[4]; } ent;

static void m_1D858(int n, int t, int k, uint8_t *out, const uint8_t *vis_in)
{
    uint8_t vis[160];
    int cand[32], nc = 0, i, j, near = 0;
    ent e[10];
    uint8_t *sa, *sn, *st;
    uint32_t ha, hn, ht;

    for (i = 0; i < 10; i++) {   /* the port's stand-in for the uninitialised entries */
        e[i].b[0] = 0x0F;
        e[i].b[1] = e[i].b[2] = e[i].b[3] = 0xFF;
    }
    if (s32(BMODE) != 0) {
        int32_t c = s32(BCALLS) + 1;
        w32(BCALLS, c);
        if (c % 250 == 0) {
            snprintf((char *)(g_ds + (0xF5500u - DS_BASE)), 500, "%s %s", str(0xBDD88u),
                     str((uint32_t)s32(0xBB404u + 4u * (uint32_t)(c / 250 % 4))));
            fg_vbopen(s32(0xF52C8u));
            f3A990();
            f3A9F4(str(0xF5500u));
            fg_vbclose();
            f3CBBC();
        }
    }
    memcpy(vis, vis_in, 160);
    for (i = 0; i < adj_n(0, t); i++) {
        int u = adj_at(0, t, i);
        if (!vis[u])
            cand[nc++] = u;
    }
    for (i = 0; i < adj_n(1, t); i++) {
        int u = adj_at(1, t, i), seen = 0;
        for (j = 0; j < nc; j++)
            if (cand[j] == u)
                seen = 1;
        if (!seen && !vis[u])
            cand[nc++] = u;
    }
    if (nc > 10)
        nc = 10;
    vis[t] = 1;
    if (nc == 0) {
        out[0] &= 0xF0;
        out[1] = 0;
        out[2] = 0xFF;
        out[3] = (uint8_t)t;
        return;
    }
    for (i = 0; i < nc && !near; i++)
        if (rb(AR(cand[i] + 1, 0x0E + n)) != 0 || m_city_of(cand[i], n))
            near = 1;
    ha = load_heap_alloc(0x2A51);
    hn = load_heap_alloc(0x88D);
    ht = load_heap_alloc(0x1D7);
    sa = load_heap_ptr(ha, 0x2A51);
    sn = load_heap_ptr(hn, 0x88D);
    st = load_heap_ptr(ht, 0x1D7);
    memcpy(sa, g_ds + (AR(1, 0) - DS_BASE), 0x2A51);
    memcpy(st, g_ds + (TS(0, 0) - DS_BASE), 0x1D7);
    memcpy(sn, g_ds + (NA(1, 0) - DS_BASE), 0x88D);
    for (i = 0; i < nc; i++) {
        int c = cand[i], at;
        memcpy(g_ds + (AR(1, 0) - DS_BASE), sa, 0x2A51);
        memcpy(g_ds + (TS(0, 0) - DS_BASE), st, 0x1D7);
        memcpy(g_ds + (NA(1, 0) - DS_BASE), sn, 0x88D);
        at = rb(AR(c + 1, 0x0E + n)) != 0 || m_city_of(c, n);
        f57E28(c, 11, k);
        memset(g_ds + (LOSST - DS_BASE), 0, 14);
        memset(g_ds + (LOSSC - DS_BASE), 0, 14);
        if (rb(TS(c, 2)) & 1)
            f5B224(AR(c + 1, 0));
        else
            f5B588(c + 1);
        if (at) {
            int lo = (rb(LOSSC + (uint32_t)n) * 3 + rb(LOSST + (uint32_t)n)) & 15;
            if (g_mut == 4)
                lo = (rb(LOSSC + (uint32_t)n) * 2 + rb(LOSST + (uint32_t)n)) & 15;
            e[i].b[0] = (uint8_t)((e[i].b[0] & 0xF0) | lo);
            e[i].b[1] = (uint8_t)m_1DFFC();
            e[i].b[2] = (uint8_t)s32(DEP);
            e[i].b[3] = (uint8_t)c;
            if (s32(DEP) == 0)
                e[i].b[0] = (uint8_t)((e[i].b[0] & 0x0F) | (m_barb(c) & 15) << 4);
        } else if (!near && m_barb(c) - m_lim(c) > 0 && g_mut != 5) {
            int ex = m_barb(c) - m_lim(c);
            w32(DEP, s32(DEP) + 1);
            m_1D858(n, c, ex, e[i].b, vis);
            e[i].b[3] = (uint8_t)c;
            w32(DEP, s32(DEP) - 1);
        } else {
            e[i].b[0] &= 0xF0;
            e[i].b[1] = (uint8_t)m_1DFFC();
            e[i].b[2] = 0xFF;
            e[i].b[3] = (uint8_t)c;
        }
    }
    memcpy(g_ds + (AR(1, 0) - DS_BASE), sa, 0x2A51);
    memcpy(g_ds + (TS(0, 0) - DS_BASE), st, 0x1D7);
    memcpy(g_ds + (NA(1, 0) - DS_BASE), sn, 0x88D);
    load_heap_free(ha);
    load_heap_free(hn);
    load_heap_free(ht);
    m_1DEAC(nc, (uint8_t *)e);
    memcpy(out, e[0].b, 4);
    if (s32(DEP) == 0)
        memcpy(out, e, 40);
}

/* the runs over the route buffer (data-object bytes): by +2 and the nibble chosen */
static void m_route_run(int hi)
{
    int i;
    w32(LSTN, 0);
    for (i = 0; (g_original || i < 10) && s32(LSTN) < 157; i++) {
        uint8_t *p = g_ds + (RT - DS_BASE) + 4 * i, *q = g_ds + (RT - DS_BASE);
        if (p[2] != q[2])
            break;
        if (hi ? (p[0] >> 4) != (q[0] >> 4) : (p[0] & 15) != (q[0] & 15))
            break;
        m_push(p[3]);
    }
}

/* 0x1C9FC: the frame [ebp-0x50]..[ebp-0x29] as bytes; ok[] marks the bytes the code has written */
static uint8_t g_fr[0x28], g_ok[0x28];
#define F_E     0x00   /* [ebp-0x50] entries */
#define F_LAND  0x18   /* [ebp-0x38] int16[6] */
#define F_N     0x24   /* [ebp-0x2C] n */

static void fr_put(int off, int v) { g_fr[off] = (uint8_t)v; g_ok[off] = 1; }
/* entry i as the run reads it: 0 when its bytes are not all known (the port's stand-in stops there) */
static int fr_entry(int i, int count, uint8_t out[4])
{
    int b;
    if (!g_original && i >= count)
        return 0;
    if (4 * i + 4 > 0x28)
        return 0;
    for (b = 0; b < 4; b++) {
        if (!g_ok[4 * i + b] && !(i < count))
            return 0;
        out[b] = g_fr[4 * i + b];
    }
    return 1;
}

static void m_restore157(void)
{
    const uint8_t *a = load_heap_ptr((uint32_t)s32(SNA), 0x2B20), *s = load_heap_ptr((uint32_t)s32(SNT), 0x1E0),
                  *m = load_heap_ptr((uint32_t)s32(SNN), 0x88D);
    memcpy(g_ds + (AR(1, 0) - DS_BASE), a, 157 * 69);
    memcpy(g_ds + (TS(0, 0) - DS_BASE), s, 157 * 3);
    memcpy(g_ds + (NA(1, 0) - DS_BASE), m, 11 * 199);
}

static void m_clear_vis(void)
{
    memset(g_ds + (VIS - DS_BASE), 0, 160);
    w8(VIS + 157u, 1);
    w8(VIS + 158u, 1);
    w8(VIS + 159u, 1);
}

static void m_1C9FC(int n)
{
    int land[6], nl = 0, t, i, j, cnt;
    uint8_t x[4], x0[4];

    memset(g_fr, 0, sizeof g_fr);
    memset(g_ok, 0, sizeof g_ok);
    for (i = 0; i < 4; i++)
        fr_put(F_N + i, (uint8_t)((uint32_t)n >> (8 * i)));
    for (i = 0; i < 12; i++)
        fr_put(F_LAND + i, 0);
    for (t = 0; t < 157; t++)
        if ((rb(TF(t, 0)) >> 4) == n && nl < 6) {
            land[nl] = t;
            fr_put(F_LAND + 2 * nl, t & 0xFF);
            fr_put(F_LAND + 2 * nl + 1, t >> 8);
            nl++;
        }
    w32(LSTN, 0);
    for (i = 0; i < nl; i++)
        if (rb(AR(land[i] + 1, 0x0E + n)) != 0 || m_city_of(land[i], n))
            m_push(land[i]);
    cnt = s32(LSTN);
    if (cnt >= 1) {
        if (cnt == 1)
            return;
        for (i = 0; i < cnt; i++) {
            int tt = s32(LST + 4u * (uint32_t)i);
            int v = m_city_of(tt, n) ? 6 : (rb(AR(tt + 1, 0x0E + n)) & 15);
            g_fr[4 * i] = (uint8_t)((g_fr[4 * i] & 0xF0) | v);
            g_ok[4 * i] = 1;
            fr_put(4 * i + 3, tt & 0xFF);
        }
        for (i = cnt - 2; i >= 0; i--)
            for (j = 0; j <= i; j++)
                if ((g_fr[4 * j + 4] & 15) > (g_fr[4 * j] & 15)) {
                    uint8_t tmp[4], okt[4];
                    memcpy(tmp, g_fr + 4 * j + 4, 4); memcpy(okt, g_ok + 4 * j + 4, 4);
                    memcpy(g_fr + 4 * j + 4, g_fr + 4 * j, 4); memcpy(g_ok + 4 * j + 4, g_ok + 4 * j, 4);
                    memcpy(g_fr + 4 * j, tmp, 4); memcpy(g_ok + 4 * j, okt, 4);
                }
        w32(LSTN, 0);
        memcpy(x0, g_fr, 4);
        for (i = 0; fr_entry(i, cnt, x); i++) {
            if ((x[0] & 15) != (x0[0] & 15))
                break;
            if (g_mut == 6 && i == cnt - 1 && cnt > 2)
                break;
            m_push(x[3]);
        }
        return;
    }
    for (i = 0; i < nl; i++)
        if (sb(AR(land[i] + 1, 1)) == 0 && !(rb(TS(land[i], 2)) & 1))
            m_push(land[i]);
    cnt = s32(LSTN);
    if (cnt < 1) {
        w32(LSTN, 0);
        for (i = 0; i < nl; i++)
            m_push(land[i]);
        cnt = s32(LSTN);
        if (cnt <= 1)
            return;
        m_snap();
        for (i = 0; i < cnt; i++) {
            int tt = s32(LST + 4u * (uint32_t)i), left;
            m_restore157();
            f57E28(tt, 11, 15);
            memset(g_ds + (LOSST - DS_BASE), 0, 14);
            memset(g_ds + (LOSSC - DS_BASE), 0, 14);
            if (rb(TS(tt, 2)) & 1)
                f5B224(AR(tt + 1, 0));
            else
                f5B588(tt + 1);
            left = (int16_t)sb(AR(tt + 1, 0x19));
            m_clear_vis();
            w32(DEP, 0);
            m_1D858(n, tt, left, g_ds + (RT - DS_BASE), g_ds + (VIS - DS_BASE));
            for (j = 0; j < 4; j++)
                fr_put(4 * i + j, rb(RT + (uint32_t)j));
            fr_put(4 * i + 3, tt & 0xFF);
        }
    } else {
        if (cnt == 1)
            return;
        m_snap();
        for (i = 0; i < cnt; i++) {
            int tt = s32(LST + 4u * (uint32_t)i);
            m_restore157();
            m_clear_vis();
            w32(DEP, 0);
            m_1D858(n, tt, 15 - m_lim(tt), g_ds + (RT - DS_BASE), g_ds + (VIS - DS_BASE));
            for (j = 0; j < 4; j++)
                fr_put(4 * i + j, rb(RT + (uint32_t)j));
            fr_put(4 * i + 3, tt & 0xFF);
        }
    }
    m_1DEAC(cnt, g_fr);
    w32(LSTN, 0);
    memcpy(x0, g_fr, 4);
    for (i = 0; fr_entry(i, cnt, x); i++) {
        if (x[2] != x0[2] || (x[0] & 15) != (x0[0] & 15))
            break;
        m_push(x[3]);
    }
    m_unsnap();
}

static void m_box(uint32_t title)
{
    w16(0xBB400u, 0x14);
    f3CD6C(3);
    fg_vbcopy(0, 639, 0, 479, 0, 479, s32(0xF52CCu), s32(0xF52C8u));
    f7876C();
    f7853C(0xC5214u);
    f3B05C(0xF596Cu);
    f5CDE8(0, str(title));
    f78664(0xC5214u);
    w16(0xBB400u, 0x23);
}

static void m_1D234(int n, int d, int k, int32_t *pt)
{
    char buf[100];
    int i, j, dest, cnt = s32(LSTN);

    if (*pt != -1 && rb(RT + 1) == 0 && rb(RT + 2) == 0) {
        uint8_t *r = g_ds + (RT - DS_BASE);
        for (i = cnt - 2; i >= 0; i--)
            for (j = 0; j <= i; j++)
                if ((r[4 * j + 4] >> 4) > (r[4 * j] >> 4)) {
                    uint8_t tmp[4];
                    memcpy(tmp, r + 4 * j + 4, 4);
                    memcpy(r + 4 * j + 4, r + 4 * j, 4);
                    memcpy(r + 4 * j, tmp, 4);
                    memcpy(g_ds + (SWP - DS_BASE), tmp, 4);
                }
        m_route_run(1);
        if ((r[0] >> 4) == 0) {
            int ex = m_barb(*pt) - m_lim(*pt);
            f57CE8(*pt, 11, 0, ex);
            snprintf(buf, sizeof buf, "The Barbarian Hordes can no longer do any damage to %s units in adjacent territories.",
                     str((uint32_t)s32(0xC76C4u + 4u * (uint32_t)n)));
            f79270(-500, n, *pt, buf, 2);
            if (sb(AR(*pt + 1, 0x0E + n)) > 0)
                w8(AR(*pt + 1, 0x3B + n), 2);
            return;
        }
        f57CE8(*pt, 11, 0, k);
        dest = s32(LSTN) > 1 ? s32(LST + 4u * (((uint32_t)f78AE4() * (uint32_t)s32(LSTN)) >> 15)) : s32(LST);
        *pt = dest;
        f57E28(dest, 11, r[0] >> 4);
        snprintf(buf, sizeof buf, "The Barbarians finally settle in %s.", tn(dest));
        f79270(-500, n, dest, buf, 2);
        if (sb(AR(dest + 1, 0x0E + n)) > 0)
            w8(AR(dest + 1, 0x3B + n), 2);
        return;
    }
    if (cnt == 1)
        dest = s32(LST);
    else if (!(rb(0xE1624u) & 0x80) && sb(NA(d, 0x9A)) == 3)
        dest = f207B4(d, n, 0x1A);
    else
        dest = s32(LST + 4u * (((uint32_t)f78AE4() * (uint32_t)cnt) >> 15));
    if (*pt != -1)
        f57CE8(*pt, 11, 0, k);
    f57E28(dest, 11, k);
    snprintf(buf, sizeof buf, "%s is invaded.", tn(dest));
    f79270(-500, n, dest, buf, 2);
    memset(g_ds + (LOSST - DS_BASE), 0, 14);
    memset(g_ds + (LOSSC - DS_BASE), 0, 14);
    if (rb(TS(dest, 2)) & 1)
        f5B224(AR(dest + 1, 0));
    else
        f5B588(dest + 1);
    w8(AR(dest + 1, 0x3B + n), 2);
    if (m_1DFFC() != 0)
        m_box(cnt == 1 ? 0xB196Fu : 0xB19A1u);
    *pt = dest;
}

static void m_1C3D0(int n, int d)
{
    char buf[120];
    int32_t t = -1;
    int ex;

    w32(BMODE, 0);
    w8(0xEE168u, 3);
    snprintf(buf, sizeof buf, "%s falls victim to the Barbarian Hordes.", nm(n));
    f79270(-500, n, -200, buf, 2);
    f79270(-550, n, -200, "The Barbarians consider their destination.", 2);
    f78884();
    m_1C9FC(n);
    f78A20(1500);
    f788AC();
    m_1D234(n, d, 15, &t);
    while ((ex = m_barb(t) - m_lim(t)) > 0) {
        m_snap();
        m_clear_vis();
        f79270(-550, n, t, "The Barbarians consider their destination.", 2);
        f78884();
        w32(DEP, 0);
        m_1D858(n, t, ex, g_ds + (RT - DS_BASE), g_ds + (VIS - DS_BASE));
        f78A20(1500);
        f788AC();
        m_route_run(0);
        m_unsnap();
        if (rb(RT + 2) == 0xFF) {
            int h = s16(0xBB400u), y = s16(0xBB3FCu);
            w32(BMODE, 1);
            w16(0xBB3FCu, 0x14C);
            w16(0xBB400u, 0x7B);
            f79270(-550, n, t, str(0xBDD88u), -299);
            f78884();
            if (rb(0xF097Cu) & 1)
                f7B504(0, "brpc.wav");
            m_snap();
            m_clear_vis();
            w32(DEP, 0);
            m_1D858(n, t, 20, g_ds + (RT - DS_BASE), g_ds + (VIS - DS_BASE));
            w16(0xBB400u, h);
            w16(0xBB3FCu, y);
            m_route_run(0);
            m_unsnap();
            f78A20(4000);
            f788AC();
            if (rb(RT + 2) == 0xFF) {
                if (m_barb(t) > m_lim(t))
                    f57CE8(t, 11, 0, m_barb(t) - m_lim(t));
                snprintf(buf, sizeof buf, "The BRPC is deadlocked. They decide to settle in %s.", tn(t));
                f79270(-500, n, t, buf, 2);
                return;
            }
        }
        m_1D234(n, d, ex, &t);
        if (sb(NA(n, 0x98)) == 0 && sb(NA(n, 0x51)) == 0 && g_mut != 7) {
            if (m_barb(t) > m_lim(t))
                f57CE8(t, 11, 0, m_barb(t) - m_lim(t));
            snprintf(buf, sizeof buf, "%s has been wiped out by the Barbarians. Satisfied with a job well done they settle in %s.",
                     nm(n), tn(t));
            f79270(-500, n, t, buf, 2);
            return;
        }
    }
}

static void m_217A0(int k)
{
    int v = sb(CREC(k, 2)), g = sb(CREC(k, 1)), d = -999, best = 0, i;

    if (g != -1) {
        d = g;
    } else {
        for (i = 1; i <= 9; i++) {
            int sc;
            if (rb(NA(i, 0x9A)) == 0 || i == v)
                continue;
            sc = 5 * (9 - sb(NA(i, 0x51))) + sb(NA(i, 0x99));
            if (sc > best || (g_mut == 8 && sc == best)) {
                best = (int16_t)sc;
                d = i;
            }
        }
        if (d == -999) {
            if (g_original)
                d = 0;
            else {
                int b = -1000000;
                d = v;
                for (i = 1; i <= 9; i++)
                    if (rb(NA(i, 0x9A)) != 0 && i != v && 5 * (9 - sb(NA(i, 0x51))) + sb(NA(i, 0x99)) > b) {
                        b = 5 * (9 - sb(NA(i, 0x51))) + sb(NA(i, 0x99));
                        d = i;
                    }
            }
        }
    }
    if (v == 5)
        f79270(-500, 5, -200, "Crete can not be the Victim of Barbarian Hordes.", 1);
    else
        m_1C3D0(v, d);
    f1E69C(k);
}

/* ==== running both sides ========================================================================================== */

static uint8_t *R, *M, *g_base[4];
static int g_nbase;
static uint32_t g_firstdiff;

static void to_port(void) { g_ds = R; g_log = g_tp; g_len = &g_tp_len; g_pick = &g_pick_port; g_rand = &g_rand_port; }
static void to_model(void) { g_ds = M; g_log = g_tm; g_len = &g_tm_len; g_pick = &g_pick_model; g_rand = &g_rand_model; }

static void reset_logs(uint32_t pick)
{
    g_tp_len = g_tm_len = 0;
    g_tp[0] = g_tm[0] = 0;
    g_pick_port = g_pick_model = pick;
    g_rand_port = g_rand_model = 0;
}

/* the first differing trace line, printed */
static void show_tracediff(void)
{
    const char *a = g_tp, *b = g_tm;
    int line = 1;
    while (*a && *a == *b) {
        if (*a == '\n')
            line++;
        a++;
        b++;
    }
    while (a > g_tp && a[-1] != '\n') { a--; b--; }
    printf("   trace line %d:\n     port : %.200s\n     model: %.200s\n", line, a, b);
}

static int compare(const char *what)
{
    uint32_t i;
    int ok = 1;
    if (strcmp(g_tp, g_tm) != 0) {
        ok = 0;
        if (g_mut == 0) {
            printf("FAIL %s: traces differ\n", what);
            show_tracediff();
        }
    }
    if (ok && g_rand_port != g_rand_model)
        ok = 0;
    if (ok)
        for (i = 0; i < DS_SIZE; i++)
            if (R[i] != M[i]) {
                ok = 0;
                g_firstdiff = i + DS_BASE;
                if (g_mut == 0)
                    printf("FAIL %s: data differ at 0x%X: port %02X model %02X\n", what, (unsigned)g_firstdiff, R[i], M[i]);
                break;
            }
    return ok;
}

/* f217A0(k) on the state in R, then the model on a copy; 1 if equal */
static int run_both(int k, uint32_t pick, const char *what, int quiet_fail)
{
    int ok;
    memcpy(M, R, DS_SIZE);
    reset_logs(pick);
    to_port();
    f217A0(k);
    to_model();
    m_217A0(k);
    to_port();
    ok = compare(what);
    if (!quiet_fail) {
        g_tests++;
        if (!ok)
            g_fail++;
    }
    return ok;
}

/* ==== state helpers (on g_ds) ===================================================================================== */

static uint32_t g_rs = 1;
static uint32_t rnd(uint32_t n)
{
    g_rs ^= g_rs << 13;
    g_rs ^= g_rs >> 17;
    g_rs ^= g_rs << 5;
    return n ? g_rs % n : 0;
}

/* tokens of n in t: through the primitives, with the board / stock bytes kept consistent */
static void put_tok(int t, int n, int k)
{
    if (k <= 0)
        return;
    f57E28(t, n, k);
    w8(NA(n, 0x98), sb(NA(n, 0x98)) + k);
}
static void del_tok(int t, int n)
{
    int k = sb(AR(t + 1, 0x0E + n));
    if (k <= 0)
        return;
    f57CE8(t, n, 0, k);
    w8(NA(n, 0x98), sb(NA(n, 0x98)) - k);
}
static void put_city(int t, int n)
{
    if ((rb(TS(t, 2)) & 1) || sb(NA(n, 0x51)) >= 9)
        return;
    f78E74(t, n);
}
static void clear_area(int t)
{
    int n;
    for (n = 1; n <= 11; n++)
        del_tok(t, n);
    if (rb(TS(t, 2)) & 1) {
        int o = rb(TS(t, 0));
        if (o >= 1 && o <= 10)
            f78F04(t, o);
    }
}

static void calrec(int k, int giver, int v)
{
    w8(CREC(k, 0), 0x1A);
    w8(CREC(k, 1), giver);
    w8(CREC(k, 2), v);
}

static int land_of(int n, int *zones)
{
    int t, c = 0;
    for (t = 0; t < 157; t++)
        if ((rb(TF(t, 0)) >> 4) == n)
            zones[c++] = t;
    return c;
}

static void base_state(int b)
{
    memcpy(R, g_base[b], DS_SIZE);
    g_ds = R;
    w32(SNA, 0); w32(SNN, 0); w32(SNT, 0);
    memset(g_mask, 1, sizeof g_mask);
    set_adj();
    w8(0xBB3F8u, 1);
    w8(0xF097Cu, (uint8_t)(rb(0xF097Cu) & ~0x04));   /* animations off: the port's f79270 would stop at ANIM */
}

/* ==== A. hand vectors ============================================================================================= */

static void part_a(void)
{
    char what[200];
    uint8_t e[40];
    int zones[8], nz, i, t;

    /* 0x1DFFC */
    for (g_original = 0; g_original <= 1; g_original++) {
        base_state(0);
        memset(DP(LOSST), 0, 14);
        memset(DP(LOSSC), 0, 14);
        w8(LOSST + 2u, 3); w8(LOSST + 7u, 1); w8(LOSST + 11u, 9);   /* 11 is not counted */
        w8(LOSSC + 3u, 2); w8(LOSSC + 6u, 1); w8(LOSSC + 0u, 5);    /* 0 is not counted */
        snprintf(what, sizeof what, "A 0x1DFFC (%s): %d", g_original ? "original: the first city loss only" : "fixed: all", f1DFFC());
        check(f1DFFC() == (g_original ? 4 + 2 : 4 + 3), what);
    }
    g_original = 0;

    /* 0x1DEAC: low nibble descending, then +2 ascending, both stable */
    {
        static const uint8_t in[] = { 0x02, 1, 3, 10,  0x15, 2, 0, 11,  0x05, 3, 3, 12,  0x31, 4, 0, 13,  0x05, 5, 0, 14,
                                      0x00, 6, 0xFF, 15 };
        static const uint8_t want[] = { 11, 14, 13, 12, 10, 15 };
        int okk = 1;
        memcpy(e, in, sizeof in);
        f1DEAC(6, e);
        for (i = 0; i < 6; i++)
            okk &= e[4 * i + 3] == want[i];
        if (!check(okk, "A 0x1DEAC order (5@0 twice kept in order, 1@0; 5@3, 2@3; 0@FF)"))
            printf("   got %d %d %d %d %d %d\n", e[3], e[7], e[11], e[15], e[19], e[23]);
        check(rb(SWP) != 0 || rb(SWP + 3u) != 0, "A 0x1DEAC swaps through [0xF6E90]");
    }

    /* the snapshot round trip */
    base_state(0);
    {
        uint8_t *keep = malloc(DS_SIZE);
        memcpy(keep, R, DS_SIZE);
        f7A4F0();
        check(s32(SNA) != 0 && s32(SNN) != 0 && s32(SNT) != 0, "A 0x7A4F0 three handles");
        w8(AR(160, 5), 0x77); w8(TS(159, 1), 0x66); w8(NA(11, 0xC6), 0x55); w8(AR(1, 0), 0x44); w8(NA(1, 0), 0x33);
        f7AB24();
        check(s32(SNA) == 0 && s32(SNN) == 0 && s32(SNT) == 0, "A 0x7AB24 fields zeroed");
        check(memcmp(keep, R, DS_SIZE) == 0, "A 0x7A4F0 / 0x7AB24 round trip (areas 1..160, states 0..159, nations 1..11)");
        free(keep);
    }

    /* 0x217A0: Crete */
    base_state(0);
    calrec(0, -1, 5);
    run_both(0, 1, "A Crete vs model", 0);
    check(strstr(g_tp, "79270(-500,5,-200,[Crete can not be the Victim of Barbarian Hordes.],1)\n1E69C(0)") != NULL,
          "A Crete: the message (text kept), then 0x1E69C");

    /* the director: a giver; the best score; nobody (BUG 2) */
    for (g_original = 0; g_original <= 1; g_original++) {
        int d;
        base_state(0);
        w8(0xE1624u, rb(0xE1624u) & 0x7F);
        for (i = 1; i <= 9; i++) {
            w8(NA(i, 0x9A), 3);
            w8(NA(i, 0x51), 9);
            w8(NA(i, 0x99), 0);
        }
        /* Egypt (9) the victim with tokens in two zones, so the landing list has two entries and the director picks */
        nz = land_of(9, zones);
        for (i = 0; i < nz; i++)
            clear_area(zones[i]);
        put_tok(zones[0], 9, 3);
        put_tok(zones[1], 9, 3);
        calrec(0, -1, 9);
        run_both(0, 77, "A director nobody vs model", 0);
        d = g_original ? 0 : 1;   /* fixed: the first of equals (all score 0) is AFRICA */
        snprintf(what, sizeof what, "207B4(%d,9,0x1A)", d);
        if (g_original)
            check(strstr(g_tp, "207B4(") == NULL && strstr(g_tp, "rand ") != NULL,
                  "A director nobody (original): nation 0's +0x9A is 0, so game_rand picks");
        else
            check(strstr(g_tp, what) != NULL, "A director nobody (fixed): AFRICA, a human, picks");
        /* scores: Babylon (8) stock 12 cities 7 -> 22; Asia (6) stock 2 cities 3 -> 32; Egypt is the victim */
        base_state(0);
        w8(0xE1624u, rb(0xE1624u) & 0x7F);
        for (i = 1; i <= 9; i++) {
            w8(NA(i, 0x9A), 3);
            w8(NA(i, 0x51), 9);
            w8(NA(i, 0x99), 0);
        }
        w8(NA(8, 0x51), 7); w8(NA(8, 0x99), 12);
        w8(NA(6, 0x51), 3); w8(NA(6, 0x99), 2);
        w8(NA(9, 0x51), 0); w8(NA(9, 0x99), 40);
        nz = land_of(9, zones);
        for (i = 0; i < nz; i++)
            clear_area(zones[i]);
        put_tok(zones[0], 9, 3);
        put_tok(zones[1], 9, 3);
        calrec(0, -1, 9);
        run_both(0, 77, "A director score vs model", 0);
        check(strstr(g_tp, "207B4(6,9,0x1A)") != NULL, "A director: ASIA (32 beats 22; the victim's 85 skipped)");
        /* a giver */
        calrec(0, 3, 9);
        w8(NA(3, 0x9A), 2);   /* a computer: game_rand */
        run_both(0, 77, "A director giver vs model", 0);
        check(strstr(g_tp, "207B4(") == NULL && strstr(g_tp, "rand ") != NULL, "A director: the giver ILLYRIA, a computer");
    }
    g_original = 0;

    /* 0x1C9FC path A: Egypt's tokens 5 and 3 -> the 5; a city counts 6 */
    base_state(1);
    nz = land_of(9, zones);
    for (i = 0; i < nz; i++)
        clear_area(zones[i]);
    put_tok(zones[2], 9, 5);
    put_tok(zones[4], 9, 3);
    to_port();
    g_tp_len = 0;
    f1C9FC(9);
    check(s32(LSTN) == 1 && s32(LST) == zones[2], "A 0x1C9FC tokens: the zone with 5");
    put_city(zones[4], 9);
    f1C9FC(9);
    check(s32(LSTN) == 1 && s32(LST) == zones[4], "A 0x1C9FC a city counts 6");
    put_tok(zones[2], 9, 1);
    f1C9FC(9);
    check(s32(LSTN) == 2 && s32(LST) == zones[2] && s32(LST + 4u) == zones[4], "A 0x1C9FC 6 tokens and a city: both, in zone order");

    /* BUG 3b: Iberia and Egypt (six zones each) with the same tokens in every zone, as many as the low nibble of
       land[0]: --original then reads land[0..1] as entry 6 (equal), land[2..3] as entry 7 and so on; +3 = 0 */
    for (g_original = 0; g_original <= 1; g_original++) {
        static const int who[2] = { 2, 9 };
        int w;
        for (w = 0; w < 2; w++) {
            int n = who[w], extra = 0, j, want, lo;
            base_state(1);
            nz = land_of(n, zones);
            for (i = 0; i < nz; i++)
                clear_area(zones[i]);
            lo = zones[0] & 15;
            for (i = 0; i < nz; i++)
                put_tok(zones[i], n, lo == 0 ? 16 : lo);
            memcpy(M, R, DS_SIZE);
            reset_logs(5);
            to_port();
            f1C9FC(n);
            to_model();
            m_1C9FC(n);
            to_port();
            check(compare("A 0x1C9FC BUG 3b vs model"), "A 0x1C9FC BUG 3b vs model");
            for (j = 0; j < 3 && (zones[2 * j] & 15) == lo; j++)
                extra++;
            if (extra == 3 && (n & 15) == lo)
                extra++;
            want = nz + (g_original ? extra : 0);
            snprintf(what, sizeof what, "A 0x1C9FC BUG 3b %s (%s): %d entries, want %d", nm(n),
                     g_original ? "original" : "fixed", s32(LSTN), want);
            check(nz == 6 && s32(LSTN) == want, what);
            for (i = 6; i < s32(LSTN); i++)
                check(s32(LST + 4u * (uint32_t)i) == 0, "A 0x1C9FC BUG 3b: the frame entries list territory 0");
            if (g_original)
                printf("A. BUG 3b: %s's zones %d %d %d %d %d %d with %d tokens each: %d extra entries (territory 0) with "
                       "--original\n", nm(n), zones[0], zones[1], zones[2], zones[3], zones[4], zones[5], lo == 0 ? 16 : lo, extra);
        }
    }
    g_original = 0;

    /* 0x1C9FC paths B and C against the model (empty zones; all zones occupied by others) */
    for (i = 0; i < 2; i++) {
        int n = 2;
        base_state(1);
        nz = land_of(n, zones);
        for (t = 0; t < nz; t++)
            clear_area(zones[t]);
        if (i == 1)
            for (t = 0; t < nz; t++)
                put_tok(zones[t], 1 + (t % 9 == n - 1 ? 3 : t % 9), 2);
        put_tok(zones[0] == 0 ? 1 : 0, n, 2);
        memcpy(M, R, DS_SIZE);
        reset_logs(5);
        to_port();
        f1C9FC(n);
        to_model();
        m_1C9FC(n);
        to_port();
        snprintf(what, sizeof what, "A 0x1C9FC path %c (Iberia's %d zones) vs model", i ? 'C' : 'B', nz);
        check(compare(what), what);
        printf("A. 0x1C9FC path %c for IBERIA: %d in the list:", i ? 'C' : 'B', s32(LSTN));
        for (t = 0; t < s32(LSTN); t++)
            printf(" %s", tn(s32(LST + 4u * (uint32_t)t)));
        printf("\n");
    }

    /* BUG 3a: ten equal route entries (a hand-made route) */
    for (g_original = 0; g_original <= 1; g_original++) {
        int32_t tt = 95;
        base_state(1);
        clear_area(tt);
        for (i = 0; i < 10; i++) {
            w8(RT + 4u * (uint32_t)i, 0x30);
            w8(RT + 4u * (uint32_t)i + 1u, 0);
            w8(RT + 4u * (uint32_t)i + 2u, 0);
            w8(RT + 4u * (uint32_t)i + 3u, (uint8_t)(90 + i));
        }
        memset(DP(VIS), 0, 160);
        w8(VIS + 157u, 1); w8(VIS + 158u, 1); w8(VIS + 159u, 1);
        w32(LSTN, 10);
        put_tok(tt, 11, 9);
        to_port();
        reset_logs(3);
        f1D234(1, 2, 9 - ((rb(TF(tt, 1)) >> 3) & 7), &tt);
        snprintf(what, sizeof what, "A BUG 3a settle (%s): list %d", g_original ? "original" : "fixed", s32(LSTN));
        check(s32(LSTN) == (g_original ? 10 : 10), what);   /* +2 = 0, high nibble 3: entry 10 = {0,0,0,0} has 0 */
    }
    /* BUG 3a through 0x1D234's run: ten entries {0x00, 0, 0, 90 + i} (no barbarians left): the run goes on into the
       visited bytes (entries {0,0,0,0}) up to entry 49 with --original; then "no longer any damage" */
    for (g_original = 0; g_original <= 1; g_original++) {
        int32_t tt = 95;
        base_state(1);
        clear_area(tt);
        for (i = 0; i < 10; i++)
            w32(RT + 4u * (uint32_t)i, (int32_t)((uint32_t)(90 + i) << 24));
        memset(DP(VIS), 0, 160);
        w8(VIS + 157u, 1); w8(VIS + 158u, 1); w8(VIS + 159u, 1);
        w32(LSTN, 10);
        put_tok(tt, 11, 9);
        to_port();
        reset_logs(3);
        f1D234(1, 2, 3, &tt);
        snprintf(what, sizeof what, "A BUG 3a run (%s): %d entries", g_original ? "original" : "fixed", s32(LSTN));
        check(s32(LSTN) == (g_original ? 49 : 10), what);
        if (g_original)
            check(s32(LST + 4u * 10u) == 0 && s32(LST + 4u * 48u) == 0, "A BUG 3a: territory 0 from the visited bytes");
        check(strstr(g_tp, "no longer do any damage to African units") != NULL && tt == 95, "A BUG 3a: no longer any damage");
    }
    g_original = 0;

    /* the losses box, "can no longer", BRPC deadlocked, wiped out: against the model, the trace inspected */
    {
        int n = 4, z;   /* THRACE: one zone (SCYTHIA) */
        nz = land_of(n, zones);
        z = zones[0];
        /* invasion of a lone zone with 2 tokens of n: wiped out if n has nothing else */
        base_state(1);
        w8(NA(n, 0x9A), 3);
        for (t = 0; t < 157; t++) {
            del_tok(t, n);
            if ((rb(TS(t, 2)) & 1) && rb(TS(t, 0)) == n)
                f78F04(t, n);
        }
        w8(NA(n, 0x98), 0);
        put_tok(z, n, 2);
        put_tok(adj_at(0, z, 0), n, 1);
        calrec(0, -1, n);
        run_both(0, 9, "A Thrace wiped out vs model", 0);
        check(strstr(g_tp, "SCYTHIA is invaded.") != NULL, "A Thrace: SCYTHIA invaded");
        check(strstr(g_tp, "3B05C(F596C)") != NULL && strstr(g_tp, "BARBARIAN CONFLICT - UNITS REMOVED") != NULL,
              "A Thrace: the losses box");
        check(strstr(g_tp, "THRACE has been wiped out by the Barbarians.") != NULL || strstr(g_tp, "deadlocked") != NULL ||
              strstr(g_tp, "no longer") != NULL || strstr(g_tp, "finally settle") != NULL,
              "A Thrace: an ending message");
        printf("A. Thrace with 2 tokens in SCYTHIA: %s\n",
               strstr(g_tp, "wiped out") ? "wiped out" : strstr(g_tp, "deadlocked") ? "deadlocked" :
               strstr(g_tp, "no longer") ? "no longer any damage" : "other");
        /* nothing of Thrace anywhere near: the BRPC */
        base_state(1);
        w8(NA(n, 0x9A), 3);
        for (t = 0; t < 157; t++) {
            del_tok(t, n);
            if ((rb(TS(t, 2)) & 1) && rb(TS(t, 0)) == n)
                f78F04(t, n);
        }
        w8(0xF097Cu, rb(0xF097Cu) | 1);
        calrec(0, -1, n);
        run_both(0, 9, "A Thrace BRPC vs model", 0);
        check(strstr(g_tp, "Plundering Committee") != NULL && strstr(g_tp, ",-299)") != NULL, "A BRPC: the message, picture -299");
        check(strstr(g_tp, "7B504(0,brpc.wav)") != NULL && strstr(g_tp, "78A20(4000)") != NULL, "A BRPC: brpc.wav, 4 s");
        printf("A. BRPC run: %s; %d spinner redraws\n", strstr(g_tp, "deadlocked") ? "deadlocked" : "a route found",
               (int)(strstr(g_tp, "3A9F4[The next") ? 1 : 0));
    }
}

/* ==== B. random states ============================================================================================ */

static void random_state(int *pk)
{
    int v, zones[8], nz, i, t, n, k = (int)rnd(12);

    base_state((int)rnd((uint32_t)g_nbase));
    v = 1 + (int)rnd(9);
    if (rnd(30) == 0)
        v = 5;
    w8(0xE1624u, (uint8_t)(rnd(6) == 0 ? rb(0xE1624u) | 0x80 : rb(0xE1624u) & 0x7F));
    for (i = 1; i <= 9; i++)
        if (rnd(3) == 0)
            w8(NA(i, 0x9A), rnd(5) == 0 ? 0 : (rnd(2) ? 2 : 3));
    w8(NA(v, 0x9A), rnd(2) ? 2 : 3);
    for (i = 1; i <= 9; i++)
        if (rnd(3) == 0)
            w8(NA(i, 0x99), (int)rnd(30));
    w8(0xF097Cu, (uint8_t)((rb(0xF097Cu) & ~1) | (int)rnd(2)));
    w8(0xF625Cu, (uint8_t)rnd(2));
    w8(0xBB3F8u, rnd(8) ? 1 : 0);
    /* the victim's zones and around them */
    nz = land_of(v, zones);
    for (i = 0; i < nz; i++) {
        int r = (int)rnd(6);
        if (r <= 1)
            clear_area(zones[i]);
        if (r == 2)
            put_tok(zones[i], v, 1 + (int)rnd(8));
        if (r == 3) {
            clear_area(zones[i]);
            put_city(zones[i], v);
        }
        if (r == 4) {
            clear_area(zones[i]);
            put_tok(zones[i], 1 + (int)rnd(10), 1 + (int)rnd(5));
        }
    }
    for (i = (int)rnd(25); i > 0; i--) {
        int zz = zones[rnd((uint32_t)nz)], w = (int)rnd(2), cnt = adj_n(w, zz);
        if (cnt == 0)
            continue;
        t = adj_at(w, zz, (int)rnd((uint32_t)cnt));
        if (t >= 157)
            continue;
        n = rnd(3) == 0 ? v : 1 + (int)rnd(11);
        switch (rnd(5)) {
        case 0: clear_area(t); break;
        case 1: put_tok(t, n, 1 + (int)rnd(6)); break;
        case 2: if (n <= 10) { clear_area(t); put_city(t, n); } break;
        default: put_tok(t, v, 1 + (int)rnd(3)); break;
        }
    }
    if (rnd(4) == 0) {   /* the victim reduced to almost nothing */
        for (t = 0; t < 157; t++)
            if (rnd(3) != 0) {
                del_tok(t, v);
                if ((rb(TS(t, 2)) & 1) && rb(TS(t, 0)) == v)
                    f78F04(t, v);
            }
    }
    if (rnd(5) == 0) {   /* some neighbours out of play */
        for (t = 0; t < 157; t++)
            g_mask[t] = rnd(8) != 0;
        set_adj();
    }
    w32(0xCAC5Cu, (int32_t)rnd(0x7FFFFFFF));
    w32(BCALLS, (int32_t)rnd(1000));
    calrec(k, rnd(2) ? -1 : 1 + (int)rnd(9), v);
    *pk = k;
}

static long g_paths[8];

static void part_b(int runs)
{
    int r, k, fails = 0, start = g_fail;
    char what[100];
    for (r = 0; r < runs; r++) {
        random_state(&k);
        snprintf(what, sizeof what, "B run %d (original %d)", r, g_original);
        if (!run_both(k, (uint32_t)r * 7u + 1u, what, 0))
            fails++;
        g_paths[0] += strstr(g_tp, "is invaded.") != NULL;
        g_paths[1] += strstr(g_tp, "finally settle") != NULL;
        g_paths[2] += strstr(g_tp, "no longer") != NULL;
        g_paths[3] += strstr(g_tp, "Plundering") != NULL;
        g_paths[4] += strstr(g_tp, "deadlocked") != NULL;
        g_paths[5] += strstr(g_tp, "wiped out") != NULL;
        g_paths[6] += strstr(g_tp, "207B4(") != NULL;
        g_paths[7] += strstr(g_tp, "UNITS REMOVED") != NULL;
        if (g_fail - start > 5)
            break;
    }
    printf("B. %d runs (original %d), %d differ; paths so far: invaded %ld, settle %ld, no damage %ld, BRPC %ld, "
           "deadlocked %ld, wiped out %ld, human pick %ld, losses box %ld\n",
           r, g_original, fails, g_paths[0], g_paths[1], g_paths[2], g_paths[3], g_paths[4], g_paths[5], g_paths[6], g_paths[7]);
}

/* ==== C. the saves ================================================================================================ */

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

static void part_c(void)
{
    int b, v, i;
    char what[120];
    for (b = 0; b < g_nbase; b++)
        for (v = 1; v <= 9; v++) {
            const char *p;
            base_state(b);
            if (rb(NA(v, 0x9A)) == 0)
                w8(NA(v, 0x9A), 2);
            calrec(0, -1, v);
            snprintf(what, sizeof what, "C save %d victim %d vs model", b, v);
            run_both(0, 11, what, 0);
            printf("C. save %d, %-8s: ", b, nm(v));
            /* the messages in order */
            for (p = g_tp, i = 0; (p = strstr(p, "79270(-500,")) != NULL && i < 6; p++, i++) {
                const char *s = strchr(p, '['), *e = strchr(p, ']');
                if (s && e && e > s)
                    printf("%s\"%.*s\"", i ? " / " : "", (int)(e - s - 1), s + 1);
            }
            printf(" [%d rand]\n      clicks:", g_rand_port);
            /* the click waits in order when attended: C = CONTINUE of a -500 message, F = FINISHED of the losses box
               (the stub f7BA64 says unattended here), P = the director's pick through f207B4 */
            for (p = g_tp; *p; ) {
                const char *nl = strchr(p, '\n');
                if (strncmp(p, "79270(-500,", 11) == 0)
                    printf(" C");
                else if (strncmp(p, "7853C(C5214)", 12) == 0)
                    printf(" F");
                else if (strncmp(p, "207B4(", 6) == 0)
                    printf(" P");
                if (nl == NULL)
                    break;
                p = nl + 1;
            }
            printf("\n");
        }
}

/* ==== D. broken models ============================================================================================ */

static void part_d(void)
{
    int mut, r, k, caught;
    char what[100];
    uint32_t keep = g_rs;
    for (mut = 1; mut <= 8; mut++) {
        caught = 0;
        g_rs = 4242u + (uint32_t)mut;
        g_mut = mut;
        for (r = 0; r < 400 && !caught; r++) {
            g_original = r & 1;
            random_state(&k);
            if (mut == 3 || mut == 6) {   /* make the mutated piece reachable more often */
                int zones[8], nz = land_of(sb(CREC(k, 2)), zones), i;
                for (i = 0; i < nz && mut == 6; i++) {
                    clear_area(zones[i]);
                    put_tok(zones[i], sb(CREC(k, 2)), 2);
                }
            }
            if (!run_both(k, 99, "D", 1))
                caught = 1;
        }
        g_mut = 0;
        g_original = 0;
        snprintf(what, sizeof what, "D broken model %d caught (after %d runs)", mut, r);
        check(caught, what);
    }
    g_rs = keep;
}

/* ==== main ======================================================================================================== */

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:/work/advciv_run";
    const char *savedir = argc > 2 ? argv[2] : "C:/work/advciv_play";
    const char *names[4] = { "savgame1.dat", "savgame0.dat", "SAVGAMEA.DAT", "savgameb.dat" };
    char cwd[400], path[400], desc[64];
    int i;

    g_tp = malloc(LOGSZ);
    g_tm = malloc(LOGSZ);
    to_port();
    g_tp[0] = 0;
    if (g_tp == NULL || g_tm == NULL)
        return 1;
    if (state_init(gamedir) != 0)
        return 1;
    if (getcwd(cwd, sizeof cwd) == NULL)
        cwd[0] = 0;
    if (chdir(gamedir) == 0) {   /* terrflag.dat: the territory records are not in the save */
        f12394();
        (void)chdir(cwd);
    }
    if (!load_adj(gamedir, "LANDADJ.DAT", 0) || !load_adj(gamedir, "COASTADJ.DAT", 1)) {
        printf("cannot read %s/LANDADJ.DAT or COASTADJ.DAT\n", gamedir);
        return 1;
    }
    R = g_ds;
    M = malloc(DS_SIZE);
    for (i = 0; i < 4; i++)
        g_base[i] = malloc(DS_SIZE);
    if (M == NULL || g_base[3] == NULL)
        return 1;
    memset(g_mask, 1, sizeof g_mask);
    set_adj();
    {
        uint8_t *pristine = malloc(DS_SIZE);
        memcpy(pristine, R, DS_SIZE);
        for (i = 0; i < 4; i++) {
            snprintf(path, sizeof path, "%s/%s", i < 2 ? gamedir : savedir, names[i]);
            memcpy(R, pristine, DS_SIZE);
            if (load_save(path, desc) < 0) {
                printf("%s not found: skipped\n", path);
                continue;
            }
            printf("save %d: %s \"%s\", round %d, phase %d\n", g_nbase, path, desc, s16(0xF40FCu), rb(0xF4102u));
            memcpy(g_base[g_nbase++], R, DS_SIZE);
        }
        free(pristine);
    }
    if (g_nbase == 0)
        return 1;

    part_a();
    printf("A. %d checks, %d fail so far\n", g_tests, g_fail);
    part_c();
    printf("C. %d fail so far\n", g_fail);
    g_rs = 20261119u;
    part_b(argc > 3 ? atoi(argv[3]) : 1500);
    g_original = 1;
    part_b(argc > 3 ? atoi(argv[3]) / 2 : 750);
    g_original = 0;
    printf("B. %d fail so far\n", g_fail);
    part_d();
    printf("%d checks, %d fail\n", g_tests, g_fail);
    return g_fail == 0 ? 0 : 1;
}
