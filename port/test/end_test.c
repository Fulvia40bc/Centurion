/* end_test [GAMEDIR [SAVEDIR]]: unit checks for module END (end.c) that need no game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init; default C:\work\advciv_run, only read) and the saves
   savgame0.dat / savgame1.dat (R4, savgame1 all human); SAVEDIR holds SAVGAMEA.DAT and savgameb.dat (default
   C:\work\advciv_play, only read). The four are read with saveload.c's f5F05C as the game does. The test runs in its
   working directory, where the /C branch appends to ctest.txt (the test removes it before each such run).

   end.c is built with -DEND_TEST: its two library calls without a port counterpart (Fastgraph/Fonts' release 0x8E0F8,
   AIL_set_sequence_loop_count 0x85BBF) come to recorders here. formulas.c's f7762C (the commas of the /C log) is
   linked as it is. Every other routine end.c calls is a stub below that appends one line to a trace (drawing
   included); those with data effects apply them: the button record's label / x / y (f3C284 / f3BB48 / f3BC64, on
   dialog 0xBB800 in the data object), the palettes (fg_getdacs writes a known palette, fg_gifpal another; fg_setdacs
   traces a checksum of what it is given), f147C4 returns the test's score table, fg_intkey "presses" a key after a
   scripted number of polls, f78BA4 answers from a seeded script (-100, codes that do nothing, then EXIT -1).

   The model is a second implementation of the five routines, written from the disassembly (not from end.c): the
   standings sort is a stable insertion sort (the listing's bubble sort moves only strictly greater scores, so equal
   scores keep the nation order: the same result by a different algorithm); the /C log is formatted by the model's own
   comma routine and compared with the file the port wrote. It runs on a copy of the data object and writes the trace
   it expects; after each run the traces, the whole data object and (for /C) the file must agree.
     A. hand vectors (expected values written out): the credits (63 records, 53 windows of 11 lines, 30 pastes each,
        the title "A D V A N C E D   C I V I L I Z A T I O N ~" in the title style at window 0's last line, the
        "Producer" underline, 1,589 waits of 40 ms without a key, a key ending the roll at once); the standings (order
        with ties, EXIT's y for 3 rows, the label put back (fixed) or left "EXIT" (--original)); the /C log of known
        numbers ("1,234,567", "-12,345"); the palette helpers; the font release.
     B. f14294 on 3,000 random states (players, in-play bytes, [0xF40EC] at, under and over the nations in play, tied,
        negative and 16-bit-overflowing scores, music on / off, /C on / off), each with and without --original.
     C. f1330C on 400 random key times (and none).
     D. deliberately broken models (seven kinds) must each be caught.
   Prints each failure, the counts per part, and exits 0 when all pass.

   Build: clang -std=c11 -DEND_TEST port/test/end_test.c port/src/end.c port/src/formulas.c port/src/saveload.c
          port/src/state.c port/src/le_image.c -Iport/src -o end_test.exe */

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "end.h"
#include "fg.h"
#include "formulas.h"
#include "game.h"
#include "saveload.h"
#include "state.h"
#include "ui.h"

static uint8_t *R, *M, *g_pristine, *g_saves[4];
static int g_nsaves;

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
static const char *pstr(uint32_t a) { return str((uint32_t)s32(a)); }

/* ---- traces --------------------------------------------------------------------------------------------------- */

#define LOGSZ (1u << 21)
static char *g_real, *g_model, *g_log;
static size_t g_real_len, g_model_len, *g_len;

static void tr(const char *fmt, ...)
{
    va_list ap;
    int k;
    if (*g_len + 600 >= LOGSZ)
        return;
    va_start(ap, fmt);
    k = vsnprintf(g_log + *g_len, LOGSZ - *g_len, fmt, ap);
    va_end(ap);
    if (k > 0)
        *g_len += (size_t)k;
    g_log[(*g_len)++] = '\n';
    g_log[*g_len] = 0;
}

static uint32_t g_rng = 0x13579BDFu;
static uint32_t rnd(uint32_t n)
{
    g_rng = g_rng * 1103515245u + 12345u;
    return n ? (g_rng >> 8) % n : 0;
}

/* ---- scripts ------------------------------------------------------------------------------------------------------ */

static uint32_t g_srng;
static int g_clicks, g_polls, g_keyat;   /* g_keyat: the poll that gets a key (-1 none) */
static int g_score[10];
static jmp_buf g_jb;

static uint32_t srnd(uint32_t n)
{
    g_srng ^= g_srng << 13;
    g_srng ^= g_srng >> 17;
    g_srng ^= g_srng << 5;
    return n ? g_srng % n : 0;
}

/* ---- heap stubs (saveload.c) -------------------------------------------------------------------------------------- */

#define TAG 0x80000000u
#define SLOTS 64
static uint8_t *s_blk[SLOTS];
static uint32_t s_len[SLOTS];

uint32_t load_heap_alloc(uint32_t size)
{
    uint32_t i;
    if (size == 0 || size > 0xFFFFFu)
        return 0;
    for (i = 0; i < SLOTS; i++)
        if (s_blk[i] == NULL) {
            s_blk[i] = calloc(1, size);
            s_len[i] = size;
            return TAG | (i + 1u) << 20;
        }
    return 0;
}

uint8_t *load_heap_ptr(uint32_t h, uint32_t n)
{
    uint32_t slot = (h & ~TAG) >> 20, off = h & 0xFFFFFu;
    if ((h & TAG) == 0 || slot < 1 || slot > SLOTS || s_blk[slot - 1] == NULL || off > s_len[slot - 1] ||
        n > s_len[slot - 1] - off)
        return NULL;
    return s_blk[slot - 1] + off;
}

void load_heap_free(uint32_t h)
{
    uint32_t slot = (h & ~TAG) >> 20;
    if ((h & TAG) == 0 || slot < 1 || slot > SLOTS)
        return;
    free(s_blk[slot - 1]);
    s_blk[slot - 1] = NULL;
}

int f78AE4(void)
{
    uint32_t s = (uint32_t)s32(0xCAC5Cu) * 0x15A4E35u + 1u;
    w32(0xCAC5Cu, (int32_t)s);
    return (int)((s >> 16) & 0x7FFF);
}
void f76F70(int n, uint8_t *cards) { (void)n; (void)cards; }
int f78D08(int t, int n) { (void)t; (void)n; return 0; }   /* formulas.c's f46A4C / f46B34 only (not reached here) */

/* ---- stubs (both sides) --------------------------------------------------------------------------------------- */

_Noreturn void not_ported(uint32_t addr)
{
    tr("NOT PORTED %X", (unsigned)addr);
    longjmp(g_jb, 1);
}

int f147C4(int n) { tr("score %d", n); return g_score[n]; }
void f3CD6C(int8_t pic) { tr("bg %d", pic); }
int fg_vbcopy(int minx, int maxx, int miny, int maxy, int newx, int newy, int source, int dest)
{
    tr("vbcopy %d %d %d %d %d %d %d %d", minx, maxx, miny, maxy, newx, newy, source, dest);
    return 0;
}
int fg_vbopen(int handle) { tr("vbopen %d", handle); return 0; }
void fg_vbclose(void) { tr("vbclose"); }
int fg_vbpaste(int minx, int maxx, int miny, int maxy, int x, int y)
{
    tr("paste %d %d %d %d %d %d", minx, maxx, miny, maxy, x, y);
    return 0;
}
int fg_setcolor(int color) { tr("color %d", color); return 0; }
void f39F90(int font) { tr("font %d", font); }
void f3A18C(int x, int y, int w, int color, const char *s) { tr("title %d %d %d %d %s", x, y, w, color, s); }
void f3AF8C(uint32_t spr, int x, int y) { tr("sprite %X %d %d", (unsigned)spr, x, y); }
void f3A0B0(int x, int y, int w, const char *s) { tr("text %d %d %d %s", x, y, w, s); }
int fgf_width(const char *s, int n) { int i, w = 0; for (i = 0; i < n; i++) w += 5 + (s[i] & 7); tr("width %s %d", s, n); return w; }
void fg_move(int x, int y) { tr("move %d %d", x, y); }
int fg_draw(int x, int y) { tr("draw %d %d", x, y); return 0; }
void f3A148(int x, int y, int w, int value) { tr("number %d %d %d %d", x, y, w, value); }
static uint32_t btn(uint32_t dlg, int id)   /* the first record with this id (16-bit compare), 0 if none */
{
    int i;
    for (i = 0; i < s16(dlg); i++)
        if ((s16(dlg + 2u + 20u * (uint32_t)i + 0x0Eu) & 0xFFFF) == (id & 0xFFFF))
            return dlg + 2u + 20u * (uint32_t)i;
    return 0;
}
void f3C284(uint32_t dlg, int id, uint32_t label)
{
    uint32_t r = btn(dlg, id);
    tr("label %X %X %s", (unsigned)dlg, (unsigned)id, str(label));
    if (r) w32(r + 8u, (int32_t)label);
}
void f3BB48(uint32_t dlg, int id, int x) { uint32_t r = btn(dlg, id); tr("x %X %X %d", (unsigned)dlg, (unsigned)id, x); if (r) w16(r, x); }
void f3BC64(uint32_t dlg, int id, int y) { uint32_t r = btn(dlg, id); tr("y %X %X %d", (unsigned)dlg, (unsigned)id, y); if (r) w16(r + 2u, y); }
void f3B05C(uint32_t dlg) { tr("dialog %X", (unsigned)dlg); w32(0xF8E78u, (int32_t)dlg); }
void f3CC14(void) { tr("show"); }
void f7B854(const char *name, int seq) { tr("music [%s] %d", name, seq); }
void f78884(void) { tr("hide"); }
int f3FB94(int id) { tr("dispatch %X", (unsigned)id); return -100; }
int cat_show(const char *name, int flags, int handle) { tr("cat %s %d %d", name, flags, handle); return 0; }
void f78A20(int t) { tr("wait %d (%d)", t, s32(0xBB3F4u)); }
int f78BA4(uint32_t dlg, int map)
{
    int r;
    if (++g_clicks > 200)
        r = -1;
    else
        switch (srnd(6)) {
        case 0: case 1: r = -100; break;
        case 2: r = -5; break;
        case 3: r = -10; break;
        default: r = -1; break;
        }
    tr("click %X %d %d", (unsigned)dlg, map, r);
    return r;
}
void fg_intkey(unsigned char *key, unsigned char *aux)
{
    int k = g_polls++ == g_keyat;
    *key = (unsigned char)(k ? 0x1B : 0);
    *aux = 0;
    tr("intkey %d", k);
}
int fg_getdacs(int start, int count, uint8_t *values)
{
    int i;
    tr("getdacs %d %d %X", start, count, (unsigned)(values - g_ds) + DS_BASE);
    for (i = 0; i < 3 * count; i++)
        values[i] = (uint8_t)((i * 7 + 3) & 0x3F);
    return 0;
}
int fg_setdacs(int start, int count, const uint8_t *values)
{
    uint32_t sum = 0;
    int i;
    for (i = 0; i < 3 * count; i++)
        sum = sum * 31u + values[i];
    tr("setdacs %d %d %X %08X", start, count, (unsigned)(values - g_ds) + DS_BASE, (unsigned)sum);
    return 0;
}
int fg_gifpal(const char *filename, uint8_t *palette)
{
    int i;
    tr("gifpal %s %X", filename, (unsigned)(palette - g_ds) + DS_BASE);
    for (i = 0; i < 768; i++)
        palette[i] = (uint8_t)((i * 13 + 5) & 0x3F);
    return 0;
}
void end_test_fgf_release(int font) { tr("release %d", font); }
void end_test_ail_loop_count(int32_t seq, int count) { tr("ail loop %d %d", (int)seq, count); }

/* ==== the model (from the listing) ================================================================================== */

static int g_mut;   /* 0 = the listing; 1..7 = deliberately broken (part D) */

static void m_3CDF0(void)
{
    fg_getdacs(0, 256, g_ds + (0xF8B40u - DS_BASE));
    memset(g_ds + (0xF5D58u - DS_BASE), 0, 768);
    fg_setdacs(0, 256, g_ds + (0xF5D58u - DS_BASE));
}

static void m_3CE44(const char *name)
{
    if (name[0] != 0)
        fg_gifpal(name, g_ds + (0xF8B40u - DS_BASE));
    memcpy(g_ds + (0xF5D58u - DS_BASE), g_ds + (0xF8B40u - DS_BASE), 768);
    fg_setdacs(0, 256, g_ds + (0xF5D58u - DS_BASE));
}

static void m_39F50(int i)
{
    end_test_fgf_release(i == -1 ? -1 : s16(0xF8B2Cu + 2u * (uint32_t)i));
}

static void m_1330C(void)
{
    int windows = s32(0xC8038u) - 10, k, line, j, rec, kind, w, x0, y, started;
    const char *s;
    m_3CDF0();
    fg_vbopen(s32(0xF52CCu));
    fg_move(0, 0);
    cat_show(pstr(0xC19D0u), 3, s32(0xF40D8u));
    fg_vbclose();
    w8(0xF52D0u, 21);
    fg_vbcopy(0, 639, 0, 479, 0, 479, s32(0xF52CCu), s32(0xF52C8u));
    fg_vbopen(s32(0xF52C8u));
    fg_vbpaste(0, 639, 0, 479, 0, 479);
    fg_vbclose();
    m_3CE44("pal.pal");
    f39F90(2);
    fg_setcolor(13);
    for (k = 0; k < windows; k++) {
        started = 0;
        fg_vbcopy(80, 559, 90, 419, 80, 419, s32(0xF52CCu), s32(0xF52C8u));
        fg_vbclose();
        fg_vbopen(s32(0xF52C8u));
        for (line = 0; line < 11; line++) {
            rec = k + line;
            kind = sb(0xC7EFCu + 5u * (uint32_t)rec);
            s = pstr(0xC7EFDu + 5u * (uint32_t)rec);
            y = 90 + 30 * line;
            if (kind == 2 && g_mut != 6) {
                f3A18C(80, y, 480, 69, s);
                f39F90(2);
                fg_setcolor(13);
            } else {
                f3A0B0(80, y, 480, s);
            }
            if (kind == 1) {
                w = fgf_width(s, (int)strlen(s));
                x0 = 78 + (480 - w) / 2;
                fg_move(x0, y + 21);
                fg_draw(x0 + w + 2, y + 21);
            }
        }
        fg_vbclose();
        for (j = 0; j < 30; j++) {
            fg_intkey(g_ds + (0xE1634u - DS_BASE), g_ds + (0xE1635u - DS_BASE));
            if (rb(0xE1634u))
                return;
            if (!started) {
                f78A20(40);
                started = 1;
            }
            fg_vbopen(s32(0xF52C8u));
            fg_vbpaste(80, 559, 90 + j, 389 + j, 80, 389);
            fg_vbclose();
            if (j < 29 || g_mut == 7)
                f78A20(40);
        }
    }
}

/* the model's own thousands commas */
static void m_commas(int32_t v, char *out)
{
    char d[16];
    int n = 0, i, neg = v < 0;
    uint32_t u = neg ? 0u - (uint32_t)v : (uint32_t)v;
    do { d[n++] = (char)('0' + u % 10u); u /= 10u; } while (u);
    if (neg) *out++ = '-';
    for (i = n - 1; i >= 0; i--) {
        *out++ = d[i];
        if (i > 0 && i % (g_mut == 5 ? 4 : 3) == 0)
            *out++ = ',';
    }
    *out = 0;
}

static char g_expect[2000];

static void m_14294(void)
{
    int sc[9], na[9], cnt = 0, n, i, j, r, rows, w, x0;
    char b1[32], b2[32], b3[32], b4[32];
    uint32_t keep;
    for (n = 1; n <= 9; n++)
        if (rb(0xF14CCu + 199u * (uint32_t)n + 0x9Au) != 0) {
            sc[cnt] = (int16_t)f147C4(n);
            na[cnt] = n;
            cnt++;
        }
    /* stable insertion sort, descending */
    for (i = 1; i < cnt; i++) {
        int s = sc[i], a = na[i];
        for (j = i; j > 0 && (g_mut == 1 ? sc[j - 1] <= s : sc[j - 1] < s); j--) {
            sc[j] = sc[j - 1];
            na[j] = na[j - 1];
        }
        sc[j] = s;
        na[j] = a;
    }
    if (s32(0xF628Cu) != 0) {
        m_commas(s32(0xF605Cu), b1);
        m_commas(s32(0xF6060u), b2);
        m_commas((int32_t)((uint32_t)s32(0xF6060u) + 0x100934u), b3);
        m_commas((int32_t)((uint32_t)s32(0xF605Cu) - (uint32_t)s32(0xF6060u)), b4);
        snprintf(g_expect, sizeof g_expect, "End of game %d.\nMemory available after game is loaded: %s\n"
                 "Max memory dynamically allocated by arrays: %s\nMax Memory used including .map analysis: %s\n"
                 "Memory Headroom: %s\nMax WorldStack: %d   Max MoveStack: %d  Max RouteList: %d\nRoutingMax: %d\n",
                 (int)s32(0xBC8DCu), b1, b2, b3, b4, (int)s32(0x112A8Cu), (int)s32(0x112A90u), (int)s32(0x109E04u),
                 (int)s32(0x10818Cu));
        /* f7762C writes each number into 0xF5500 in turn: the last one with the tails of longer ones after it */
        strcpy((char *)(g_ds + (0xF5500u - DS_BASE)), b1);
        strcpy((char *)(g_ds + (0xF5500u - DS_BASE)), b2);
        strcpy((char *)(g_ds + (0xF5500u - DS_BASE)), b3);
        strcpy((char *)(g_ds + (0xF5500u - DS_BASE)), b4);
        w32(0xBC8DCu, s32(0xBC8DCu) + 1);
        if (s32(0xF6290u) < s32(0xBC8E0u) - 1)
            w32(0xF6290u, s32(0xF6290u) + 1);
        else
            w32(0xF6290u, 0);
        f3FB94(0x51);
        return;
    }
    f3CD6C(23);
    fg_vbcopy(0, 639, 0, 479, 0, 479, s32(0xF52CCu), s32(0xF52C8u));
    fg_vbopen(s32(0xF52C8u));
    f3A18C(2, 6, 640, 176, "Final Civilization Standings");
    fg_setcolor(5);
    f39F90(2);
    rows = s32(0xF40ECu) < cnt ? s32(0xF40ECu) : cnt;   /* the port's guard */
    if (rows < 0)
        rows = 0;
    for (r = 0; r < rows; r++) {
        const char *nm = pstr(0xC768Cu + 4u * (uint32_t)na[r]);
        f3AF8C(0xF4A5Cu + 8u * (uint32_t)na[r], 219, 49 * r + 42);
        f3AF8C(0xF4A5Cu + 8u * (uint32_t)na[r], 387, 49 * r + 42);
        f3A0B0(0, 49 * r + 40, 640, nm);
        w = fgf_width(nm, (int)strlen(nm));
        x0 = (640 - w) / 2 - 2;
        fg_move(x0, 49 * r + 60);
        fg_draw(x0 + w + 2, 49 * r + 60);
        f3A148(0, 49 * r + 63, 640, g_mut == 2 ? na[r] : sc[r]);
    }
    f39F90(1);
    keep = (uint32_t)s32(0xBB80Au);
    f3C284(0xBB800u, 42, 0xB0EDEu);
    f3BB48(0xBB800u, 42, 265);
    f3BC64(0xBB800u, 42, 49 * rows + 40 + (476 - (49 * rows + 40) - 19) / (g_mut == 3 ? 1 : 2));
    fg_vbclose();
    f3B05C(0xBB800u);
    f3CC14();
    f7B854(str(0xCAB1Cu), 0);
    if (rb(0xF625Du) != 0)
        end_test_ail_loop_count(s32(0xF6284u), 1);
    while (f78BA4(0xBB800u, 0) != -1)
        ;
    if (!g_original && g_mut != 4)
        f3C284(0xBB800u, 42, keep);
    f78884();
    m_1330C();
}

/* ==== running and comparing ============================================================================================ */

static int g_checks, g_fail, g_quiet;

static void check(int ok, const char *what)
{
    g_checks++;
    if (!ok) {
        g_fail++;
        printf("FAIL %s\n", what);
    }
}

static char g_file[2000];

static void read_ctest(void)
{
    FILE *f = fopen("ctest.txt", "r");   /* text mode: the port writes "at" as the original did (CR LF on DOS) */
    size_t n = 0;
    g_file[0] = 0;
    if (f != NULL) {
        n = fread(g_file, 1, sizeof g_file - 1, f);
        fclose(f);
    }
    g_file[n] = 0;
}

enum { RUN_14294, RUN_1330C };

static int run_both(int what, uint32_t seed, int keyat, const char *tag)
{
    int ok = 1;
    size_t i;
    memcpy(M, R, DS_SIZE);
    remove("ctest.txt");
    g_expect[0] = 0;
    g_ds = R;
    g_log = g_real; g_len = &g_real_len; g_real_len = 0; g_real[0] = 0;
    g_srng = seed; g_clicks = 0; g_polls = 0; g_keyat = keyat;
    if (setjmp(g_jb) == 0) {
        if (what == RUN_14294) f14294(); else f1330C();
    }
    read_ctest();
    g_ds = M;
    g_log = g_model; g_len = &g_model_len; g_model_len = 0; g_model[0] = 0;
    g_srng = seed; g_clicks = 0; g_polls = 0; g_keyat = keyat;
    if (setjmp(g_jb) == 0) {
        if (what == RUN_14294) m_14294(); else m_1330C();
    }
    g_ds = R;
    if (strcmp(g_file, g_expect) != 0) {
        ok = 0;
        if (!g_quiet)
            printf("FAIL %s (original %d): ctest.txt\n--- port:\n%s--- model:\n%s", tag, g_original, g_file, g_expect);
    }
    if (g_real_len != g_model_len || memcmp(g_real, g_model, g_real_len) != 0) {
        ok = 0;
        if (!g_quiet) {
            const char *p = g_real, *q = g_model;
            int line = 1;
            while (*p && *p == *q) { if (*p == '\n') line++; p++; q++; }
            while (p > g_real && p[-1] != '\n') p--;
            while (q > g_model && q[-1] != '\n') q--;
            printf("FAIL %s (original %d): trace line %d\n  port : %.120s\n  model: %.120s\n", tag, g_original, line, p, q);
        }
    }
    for (i = 0; i < DS_SIZE && ok; i++)
        if (R[i] != M[i]) {
            ok = 0;
            if (!g_quiet)
                printf("FAIL %s (original %d): byte 0x%X port %02X model %02X\n", tag, g_original,
                       (unsigned)(i + DS_BASE), R[i], M[i]);
        }
    return ok;
}

static int count_lines(const char *log, const char *prefix)
{
    int c = 0;
    size_t n = strlen(prefix);
    const char *p = log;
    while (*p) {
        if (strncmp(p, prefix, n) == 0) c++;
        p = strchr(p, '\n');
        if (p == NULL) break;
        p++;
    }
    return c;
}

/* ---- states ------------------------------------------------------------------------------------------------------ */

static void make_state(int s)
{
    int n, cnt = 0, k;
    memcpy(R, g_saves[s], DS_SIZE);
    g_ds = R;
    for (n = 1; n <= 9; n++) {
        w8(0xF14CCu + 199u * (uint32_t)n + 0x9Au, rnd(3) == 0 ? 0 : (rnd(2) ? 2 : 3));
        if (rb(0xF14CCu + 199u * (uint32_t)n + 0x9Au) != 0) cnt++;
        switch (rnd(6)) {
        case 0: g_score[n] = (int)rnd(40000); break;          /* over 32767: the int16 store wraps */
        case 1: g_score[n] = -(int)rnd(300); break;
        default: g_score[n] = 50 * (int)rnd(8); break;      /* many ties */
        }
    }
    k = (int)rnd(8);
    w32(0xF40ECu, k == 0 ? cnt + 1 : (k == 1 ? cnt - 1 : cnt));
    w32(0xF628Cu, rnd(4) == 0);
    w8(0xF625Du, (int)rnd(2));
    w32(0xF6284u, (int)rnd(100));
    w32(0xBC8DCu, (int)rnd(50));
    w32(0xBC8E0u, (int)rnd(30) - 2);
    w32(0xF6290u, (int)rnd(30));
    w32(0xF605Cu, (int32_t)(rnd(0x7FFFFFFF) - 0x3FFFFFFF));
    w32(0xF6060u, (int32_t)rnd(20000000));
    w32(0x112A8Cu, (int)rnd(7));
    w32(0x112A90u, (int)rnd(7));
    w32(0x109E04u, (int)rnd(400));
    w32(0x10818Cu, (int)rnd(3000));
}

/* ==== A. hand vectors ================================================================================================ */

static void part_a(void)
{
    int f0 = g_fail, n;
    char want[200];
    g_original = 0;
    memcpy(R, g_saves[0], DS_SIZE);
    g_ds = R;
    check(s32(0xC8038u) == 63, "A 63 credit records");
    check(strcmp(pstr(0xC7EFDu + 50u), "A D V A N C E D   C I V I L I Z A T I O N ~") == 0 && sb(0xC7EFCu + 50u) == 2,
          "A record 10 the title, type 2");
    check(strcmp(pstr(0xC7EFDu + 5u * 21u), "AI Design Nemesis") == 0 && sb(0xC7EFCu + 5u * 21u) == 1,
          "A record 21 AI Design Nemesis, a heading");

    /* the credits without a key */
    check(run_both(RUN_1330C, 1, -1, "A credits"), "A credits: port = model");
    check(count_lines(g_real, "paste 80 559 ") == 53 * 30, "A credits: 53 windows of 30 pastes");
    check(count_lines(g_real, "wait 40") == 53 * 30,
          "A credits: 1,590 waits (one before each window, one after each paste but the window's last)");
    check(count_lines(g_real, "intkey") == 53 * 30, "A credits: one key poll per paste");
    check(strstr(g_real, "title 80 390 480 69 A D V A N C E D   C I V I L I Z A T I O N ~") != NULL,
          "A credits: the title in window 0's last line (y 0x5A + 300), colour 0x45");
    n = 0;
    {
        int w = 0, i;
        const char *s = "Producer";
        for (i = 0; s[i]; i++) w += 5 + (s[i] & 7);
        snprintf(want, sizeof want, "move %d %d\ndraw %d %d", 78 + (480 - w) / 2, 90 + 30 * 2 + 21,
                 78 + (480 - w) / 2 + w + 2, 90 + 30 * 2 + 21);   /* window 0: record 12 is line 2 */
        n = strstr(g_real, want) != NULL;
    }
    check(n, "A credits: the Producer underline");
    check(strstr(g_real, "cat message.gif 3") != NULL && rb(0xF52D0u) == 0x15, "A credits: message.gif, [0xF52D0] = 0x15");
    check(run_both(RUN_1330C, 1, 0, "A credits key") && count_lines(g_real, "paste 80 559 ") == 0 &&
          count_lines(g_real, "intkey") == 1, "A credits: a key at the first poll ends the roll");

    /* the palettes */
    memcpy(R, g_saves[0], DS_SIZE);
    f3CDF0();
    check(rb(0xF8B40u) == 3 && rb(0xF8B41u) == 10 && rb(0xF5D58u) == 0 && rb(0xF5D58u + 767u) == 0,
          "A 3CDF0: the palette saved, the work buffer black");
    f3CE44("pal.pal");
    check(rb(0xF8B40u) == 5 && rb(0xF5D58u + 1u) == 18 && memcmp(R + (0xF8B40u - DS_BASE), R + (0xF5D58u - DS_BASE), 768) == 0,
          "A 3CE44: the GIF's table into both buffers");
    w8(0xF8B40u, 0x22);
    f3CE44("");
    check(rb(0xF5D58u) == 0x22, "A 3CE44(\"\"): the saved palette re-applied");
    g_log = g_real; g_len = &g_real_len; g_real_len = 0; g_real[0] = 0;
    w16(0xF8B2Cu + 2u, 7);
    f39F50(1);
    f39F50(-1);
    m_39F50(1);
    m_39F50(-1);
    check(strcmp(g_real, "release 7\nrelease -1\nrelease 7\nrelease -1\n") == 0,
          "A 39F50: font 1 -> its number, -1 -> all (port, then model)");

    /* the standings: nations 2 (100), 5 (300), 7 (100), three players; fixed: the label back; --original: EXIT */
    for (n = 0; n < 2; n++) {
        int i;
        g_original = n;
        memcpy(R, g_saves[0], DS_SIZE);
        for (i = 1; i <= 9; i++)
            w8(0xF14CCu + 199u * (uint32_t)i + 0x9Au, (i == 2 || i == 5 || i == 7) ? 3 : 0);
        g_score[2] = 100; g_score[5] = 300; g_score[7] = 100;
        w32(0xF40ECu, 3);
        w32(0xF628Cu, 0);
        check(run_both(RUN_14294, 7, -1, "A standings"), "A standings: port = model");
        snprintf(want, sizeof want, "text 0 40 640 %s", pstr(0xC768Cu + 20u));
        check(strstr(g_real, want) != NULL, "A standings: nation 5 first");
        snprintf(want, sizeof want, "text 0 89 640 %s", pstr(0xC768Cu + 8u));
        check(strstr(g_real, want) != NULL, "A standings: then nation 2 (tie with 7: nation order)");
        check(strstr(g_real, "y BB800 2A 322") != NULL, "A standings: EXIT at y 322 for three rows");
        check(s32(0xBB80Au) == (n ? 0xB0EDE : 0xB0EAB),
              n ? "A standings --original: EXIT stays" : "A standings fixed: CONTINUE again");
        check(strstr(g_real, "music [gcivil2_.xmi ] 0") != NULL, "A standings: gcivil2");
    }
    g_original = 0;

    /* the /C log with known numbers */
    memcpy(R, g_saves[0], DS_SIZE);
    w32(0xF628Cu, 1);
    w32(0xBC8DCu, 4); w32(0xBC8E0u, 3); w32(0xF6290u, 2);
    w32(0xF605Cu, 1234567); w32(0xF6060u, 1246912);
    w32(0x112A8Cu, 5); w32(0x112A90u, 6); w32(0x109E04u, 123); w32(0x10818Cu, 4567);
    check(run_both(RUN_14294, 8, -1, "A ctest"), "A /C: port = model");
    check(strstr(g_file, "End of game 4.\nMemory available after game is loaded: 1,234,567\n") != NULL &&
          strstr(g_file, "Max memory dynamically allocated by arrays: 1,246,912\n") != NULL &&
          strstr(g_file, "Max Memory used including .map analysis: 2,297,844\n") != NULL &&
          strstr(g_file, "Memory Headroom: -12,345\n") != NULL &&
          strstr(g_file, "Max WorldStack: 5   Max MoveStack: 6  Max RouteList: 123\nRoutingMax: 4567\n") != NULL,
          "A /C: the log lines");
    check(s32(0xBC8DCu) == 5 && s32(0xF6290u) == 0 && strstr(g_real, "dispatch 51") != NULL,
          "A /C: game 5, the cycle back to 0, START NEW GAME");
    remove("ctest.txt");
    printf("A. hand vectors: %d failed\n", g_fail - f0);
}

/* ==== B, C random runs; D broken models =============================================================================== */

static void part_b(int runs)
{
    int i, fails = 0;
    char tag[40];
    for (i = 0; i < runs; i++) {
        g_original = i & 1;
        make_state(i % g_nsaves);
        snprintf(tag, sizeof tag, "B run %d", i);
        if (!run_both(RUN_14294, 100u + (uint32_t)i, (int)rnd(2000) - 1, tag))
            fails++;
        g_checks++;
    }
    g_fail += fails;
    printf("B. f14294: %d runs, %d differ\n", runs, fails);
}

static void part_c(int runs)
{
    int i, fails = 0;
    char tag[40];
    for (i = 0; i < runs; i++) {
        memcpy(R, g_saves[i % g_nsaves], DS_SIZE);
        g_ds = R;
        if (i & 1) w32(0xBB3F4u, 10);
        snprintf(tag, sizeof tag, "C run %d", i);
        if (!run_both(RUN_1330C, 1u, i % 7 == 0 ? -1 : (int)rnd(1700), tag))
            fails++;
        g_checks++;
    }
    g_fail += fails;
    printf("C. f1330C: %d runs, %d differ\n", runs, fails);
}

static void part_d(void)
{
    int mut, i, caught;
    char what[60];
    g_quiet = 1;
    for (mut = 1; mut <= 7; mut++) {
        g_mut = mut;
        caught = 0;
        for (i = 0; i < 2000 && caught == 0; i++) {
            g_original = 0;
            if (mut >= 6) {
                memcpy(R, g_saves[i % g_nsaves], DS_SIZE);
                g_ds = R;
                caught += !run_both(RUN_1330C, 1u, -1, "D");
            } else {
                make_state(i % g_nsaves);
                if (mut == 5) {
                    w32(0xF628Cu, 1);
                    w32(0xF605Cu, 1000000 + (int32_t)rnd(1000000));
                }
                caught += !run_both(RUN_14294, 1u + (uint32_t)i, -1, "D");
            }
        }
        snprintf(what, sizeof what, "D broken model %d caught (after %d runs)", mut, i);
        check(caught > 0, what);
        printf("  %s: %s\n", what, caught ? "yes" : "NO");
    }
    g_mut = 0;
    g_quiet = 0;
    remove("ctest.txt");
}

/* ==== main ============================================================================================================ */

static int load_save(const char *path, int slot)
{
    FILE *fp;
    int16_t version = 0;
    char desc[0x34];
    int i;

    memcpy(R, g_pristine, DS_SIZE);
    g_ds = R;
    for (i = 0; i < SLOTS; i++) { free(s_blk[i]); s_blk[i] = NULL; }
    fp = fopen(path, "rb");
    if (fp == NULL)
        return 0;
    memset(desc, 0, sizeof desc);
    f5F05C(&version, desc, fp);
    fclose(fp);
    g_saves[slot] = malloc(DS_SIZE);
    if (g_saves[slot] == NULL)
        return 0;
    memcpy(g_saves[slot], R, DS_SIZE);
    printf("save %s: \"%s\"\n", path, desc);
    return 1;
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:\\work\\advciv_run";
    const char *savedir = argc > 2 ? argv[2] : "C:\\work\\advciv_play";
    char path[600];
    const char *dirs[4], *names[4] = {"SAVGAMEA.DAT", "savgameb.dat", "savgame0.dat", "savgame1.dat"};
    int i;

    setvbuf(stdout, NULL, _IONBF, 0);
    dirs[0] = dirs[1] = savedir;
    dirs[2] = dirs[3] = gamedir;
    if (state_init(gamedir) != 0) {
        fprintf(stderr, "cannot load %s/AC_MAIN.EXE\n", gamedir);
        return 2;
    }
    R = g_ds;
    g_pristine = malloc(DS_SIZE);
    M = malloc(DS_SIZE);
    g_real = malloc(LOGSZ);
    g_model = malloc(LOGSZ);
    if (g_pristine == NULL || M == NULL || g_real == NULL || g_model == NULL)
        return 2;
    memcpy(g_pristine, R, DS_SIZE);
    g_log = g_real;
    g_len = &g_real_len;
    for (i = 0; i < 4; i++) {
        snprintf(path, sizeof path, "%s/%s", dirs[i], names[i]);
        if (load_save(path, g_nsaves))
            g_nsaves++;
    }
    if (g_nsaves == 0)
        return 2;

    part_a();
    part_b(3000);
    part_c(400);
    part_d();

    printf("%d checks, %d failed\n", g_checks, g_fail);
    return g_fail == 0 ? 0 : 1;
}
