/* loop_test [GAMEDIR [SAVEDIR]]: unit checks for module LOOP (loop.c) that need no game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init; default C:\work\advciv_run, only read); SAVEDIR holds the
   real saves SAVGAMEA.DAT (the R4 autosave, start of SHIP CONSTRUCTION), savgameb.dat ("end of Round 3") and
   savgame0.dat (start of MOVEMENT R4) (default GAMEDIR). The test writes savgamea.dat / savgameb.dat in the CURRENT
   folder: run it in a scratch folder. Saves are read with saveload.c's f5F05C (VERIFIED) into the data object, as
   the game does. Expectations are written here from the disassembly, independently of loop.c:
     A. 0x7C960 (score matrix) on the three real states and 2,000 random edits of each: the 400-byte matrix 0xF52E0,
        the leader [0xF52D4] (first maximum, start -9999, strict >; unchanged when nobody beats -9999), the exact
        order of f147C4 calls (i, then every j in play), and no other byte of the data object changed. f147C4 is a
        stub computing game.h's score formula (module N2's routine is tested there).
     B. 0x22B00 (eliminations) on edited R4 states: hand cases (a computer out with cards, calamities and givers in
        other hands; a city or tokens saving a nation; a human out while humans remain; the last human -> 0x22DCC's
        screen and longjmp(0xF58D4); the last opponent out -> 0x22F3C's screen, 0x14294, f788AC, longjmp; PBEM ->
        not_ported(0x230B8)) and 3,000 random states against a model written here (messages, card discards in order,
        the counters, the givers, 0x7784C after each; the whole data object compared).
     C. 0x54824's transition table: from each phase p (0..20, and 21, 0x80, 0xFF) with every unported routine
        recorded and returning (loop.c built with -DLOOP_TEST), one turn of the loop: which routines run in which
        order, the music, the autosave's argument with the phase byte and int16 [0xF5476] at that moment, the phase
        byte that follows, the data the case writes (whole data object compared, except the score matrix and the
        round label), the variants of phases 12 and 14 (sessions, the human / computer minimum, bit 0x10 of
        [0xE1625], [0xF4090]), the slot-0 WAV of phases 15 / 16, phase 20's files (savgameb.dat; the game end with
        0x55304's 4-byte growth and remove("savgameb.dat")), and f7BCD4's 300 random numbers in phase 0.
     D. 0x553C0's music and WAV per phase (0..21) and per switch (music, effects, digital sound).
     E. 0x14AA4 (override maps, the 0x640-byte copy); F. 0x55500 on random states; G. game_loop_restored /
        game_loop_new by the bits 0x04 / 0x80 of [0xE1624]; H. 0x55258 and 0x55304 on the real R4 file.
   The save writer f5E9B0 is a stub that writes the header (version 0x38, the tag, the 51-byte description with
   desc[50] = 0) and then the body of the real file in use (savewrite.c is VERIFIED to write exactly that body).
   Prints each failure, the counts per part, and exits 0 when all pass.

   Build: clang -std=c11 -DLOOP_TEST port/test/loop_test.c port/src/loop.c port/src/saveload.c port/src/state.c
          port/src/le_image.c port/src/snd.c -Iport/src -o loop_test.exe   (every other routine loop.c calls is a stub
          below; snd.c without a backend gives the old no-op AIL results) */

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "state.h"
#include "game.h"
#include "fg.h"
#include "ui.h"
#include "info.h"
#include "kit.h"
#include "menu2.h"
#include "rules.h"
#include "saveload.h"
#include "savewrite.h"
#include "stack.h"
#include "start.h"
#include "loop.h"

/* ---- Checks and the call trace ------------------------------------------------------------------------------------ */

static int g_tests, g_fail;

static void check(int ok, int line, const char *fmt, ...)
{
    va_list ap;

    g_tests++;
    if (ok)
        return;
    g_fail++;
    fprintf(stderr, "FAIL line %d: ", line);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
}
#define CHECK(ok, ...) check((ok), __LINE__, __VA_ARGS__)

enum { TR_MAX = 600, TR_LEN = 120 };
static char g_tr[TR_MAX][TR_LEN];
static int g_ntr;

static void tr(const char *fmt, ...)
{
    va_list ap;

    if (g_ntr >= TR_MAX)
        return;
    va_start(ap, fmt);
    vsnprintf(g_tr[g_ntr++], TR_LEN, fmt, ap);
    va_end(ap);
}

/* the expected trace, as a list of lines */
static char g_ex[TR_MAX][TR_LEN];
static int g_nex;

static void ex(const char *fmt, ...)
{
    va_list ap;

    if (g_nex >= TR_MAX)
        return;
    va_start(ap, fmt);
    vsnprintf(g_ex[g_nex++], TR_LEN, fmt, ap);
    va_end(ap);
}

static void check_trace(int line, const char *what)
{
    int i, n = g_ntr > g_nex ? g_ntr : g_nex;

    for (i = 0; i < n; i++) {
        const char *a = i < g_ntr ? g_tr[i] : "(none)";
        const char *e = i < g_nex ? g_ex[i] : "(none)";
        if (strcmp(a, e) != 0) {
            check(0, line, "%s: trace line %d is \"%s\", expected \"%s\"", what, i, a, e);
            return;
        }
    }
    check(1, line, "");
}

/* ---- The data object: bases and the whole-image compare --------------------------------------------------------- */

static uint8_t *g_exp;                                /* the expected image, DS_SIZE bytes */
#define E8(a, v)  (g_exp[(a) - DS_BASE] = (uint8_t)(v))
static void E16(uint32_t a, uint16_t v) { E8(a, v); E8(a + 1, v >> 8); }
static void E32(uint32_t a, uint32_t v) { E16(a, (uint16_t)v); E16(a + 2, (uint16_t)(v >> 16)); }

static void exp_from_ds(void) { memcpy(g_exp, g_ds, DS_SIZE); }

/* ranges loop.c rewrites every turn (the score matrix, the leader, the round label): taken from the actual image */
static void exp_mask_turn(void)
{
    memcpy(g_exp + (0xF52E0u - DS_BASE), DP(0xF52E0u), 0x190);
    memcpy(g_exp + (0xF52D4u - DS_BASE), DP(0xF52D4u), 4);
    memcpy(g_exp + (0x109E7Cu - DS_BASE), DP(0x109E7Cu), 8);
}

static void check_ds(int line, const char *what)
{
    uint32_t i, bad = 0;

    for (i = 0; i < DS_SIZE; i++)
        if (g_ds[i] != g_exp[i]) {
            if (bad < 6)
                check(0, line, "%s: byte 0x%X is %02X, expected %02X", what, (unsigned)(i + DS_BASE), g_ds[i],
                      g_exp[i]);
            bad++;
        }
    if (bad == 0)
        check(1, line, "");
    else if (bad >= 6)
        check(0, line, "%s: %u bytes differ in all", what, (unsigned)bad);
}

#define NAT(n) (0xF14CCu + 199u * (uint32_t)(n))
static const char *const NAMES[10] = { "", "AFRICA", "IBERIA", "ILLYRIA", "THRACE", "CRETE", "ASIA", "ASSYRIA",
                                        "BABYLON", "EGYPT" };

/* ---- Stubs: the save files ---------------------------------------------------------------------------------------- */

/* game_rand (newgame.c's f78AE4), counted */
static long g_rand_calls;
int f78AE4(void)
{
    uint32_t seed = D32(GAME_SEED) * 0x15A4E35u + 1u;
    setD32(GAME_SEED, seed);
    g_rand_calls++;
    return (int)((seed >> 16) & 0x7FFFu);
}

void f76F70(int n, uint8_t *cards)
{
    (void)n;
    (void)cards;
}

#define TAG 0x80000000u
static uint8_t *s_blk[16];
static uint32_t s_len[16];

uint32_t load_heap_alloc(uint32_t size)
{
    uint32_t i;
    if (size == 0 || size > 0xFFFFFu)
        return 0;
    for (i = 0; i < 16; i++)
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
    if ((h & TAG) == 0 || slot < 1 || slot > 16 || s_blk[slot - 1] == NULL || off > s_len[slot - 1] ||
        n > s_len[slot - 1] - off)
        return NULL;
    return s_blk[slot - 1] + off;
}

void load_heap_free(uint32_t h)
{
    uint32_t slot = (h & ~TAG) >> 20;
    if ((h & TAG) == 0 || slot < 1 || slot > 16)
        return;
    free(s_blk[slot - 1]);
    s_blk[slot - 1] = NULL;
}

/* the body f5E9B0 writes after the header (the real file's bytes 0x4B .. size - 4) */
static uint8_t *g_body;
static long g_body_len;

void f5E9B0(char *desc, FILE *fp)
{
    static const uint8_t tag[0x16] = "Advanced Civilization";
    uint8_t vb[2] = { 0x38, 0 };

    desc[50] = 0;
    tr("write %s", desc);
    (void)fwrite(vb, 2, 1, fp);
    (void)fwrite(tag, 1, 0x16, fp);
    (void)fwrite(desc, 0x33, 1, fp);
    if (g_body_len > 0)
        (void)fwrite(g_body, 1, (size_t)g_body_len, fp);
}

void f5E8EC(int phase)
{
    tr("autosave %d phase=%d F5476=%d", phase, D8(0xF4102u), S16(0xF5476u));
}

/* ---- Stubs: the screens and the other modules -------------------------------------------------------------------- */

static jmp_buf g_jb;
static volatile int g_jrc;                    /* why the run ended: 2 label stop, 3 not_ported, 4 port_longjmp, 5 quit */
static int g_label_calls, g_label_stop;   /* f3C284: the round label; the g_label_stop-th call ends the run */
static int g_log_score;
static int g_ast_ret;
static int g_wait_ret[8], g_nwait, g_iwait;

_Noreturn void not_ported(uint32_t addr)
{
    tr("np 0x%X", (unsigned)addr);
    g_jrc = 3;
    longjmp(g_jb, 1);
}

_Noreturn void port_longjmp(uint32_t jmpbuf)
{
    tr("longjmp 0x%X", (unsigned)jmpbuf);
    g_jrc = 4;
    longjmp(g_jb, 1);
}

void port_checkpoint(uint32_t addr) { (void)addr; }   /* main.c's comparison point (fe16d67): nothing to compare here */

_Noreturn void f44CAC(void)
{
    tr("quit F5500=%d", D8(0xF5500u));
    g_jrc = 5;
    longjmp(g_jb, 1);
}

/* the hooks of the recorded unported routines (part C) */
static int g_hook_55574, g_hook_734E4_trades, g_hook_734E4_F4090, g_hook_1E0A0;

void loop_test_unported(uint32_t addr)
{
    tr("unported 0x%X", (unsigned)addr);
    if (addr == 0x55574u && g_hook_55574) {          /* the movement changes the board */
        setD8(0xE31B8u + 69u * 10u + 5u, (uint8_t)(D8(0xE31B8u + 69u * 10u + 5u) ^ 0x5A));
        setD8(0xE2C18u + 30u, (uint8_t)(D8(0xE2C18u + 30u) ^ 0x33));
        setD8(NAT(2) + 0x50u, (uint8_t)(D8(NAT(2) + 0x50u) ^ 0x11));
    }
    if (addr == 0x734E4u) {
        if (g_hook_734E4_trades)
            setD8(0x110F74u, 1);
        if (g_hook_734E4_F4090)
            setD16(0xF4090u, 1);
    }
    if (addr == 0x1E0A0u && g_hook_1E0A0) {
        setD16(0xF7AFCu, 5);
        setD16(0xF7AFEu, 6);
    }
}

void f3C284(uint32_t dlg, int id, uint32_t label)
{
    g_label_calls++;
    if (g_label_calls >= g_label_stop) {
        g_jrc = 2;
        longjmp(g_jb, 1);
    }
    tr("label %X %X %s", (unsigned)dlg, (unsigned)id, DSTR(label));
}

void f7B854(const char *name, int seq) { tr("music %s %d", name, seq); }
void f7B504(int s, const char *name) { tr("wav %d %s", s, name); }
void f763FC(void) { tr("wrapup"); }
void f7784C(int n) { (void)n; loop_test_unported(0x7784Cu); }   /* recorded like a stubbed phase routine (the model expects it) */
void f7983C(const char *text) { tr("box %s", text); }
int f136B0(int mode)
{
    tr("ast %d", mode);
    return g_ast_ret;
}
void f15FE0(int n, const char *name) { tr("bpm %d %s", n, name); }
void f771F8(int c) { tr("discard %d", c); }
void f79270(int wait, int n, int t, const char *text, int anim) { tr("msg %d %d %d %s %d", wait, n, t, text, anim); }

/* f76F14: the cards of n, the signed bytes +0x01..+0x13 summed (info.h) */
int f76F14(int n)
{
    int s = 0, i;
    for (i = 1; i <= 0x13; i++)
        s += S8(NAT(n) + (uint32_t)i);
    return s;
}

/* f147C4: game.h's score formula, written here; the call order is logged when g_log_score */
static int card_value(int k)
{
    int s = 0, i;
    for (i = 1; i <= 18; i++) {
        int c = S8(NAT(k) + (uint32_t)i);
        s += c * c * ((i + 1) >> 1);
    }
    return s;
}

static int score_of(int n)
{
    return S16(NAT(n) + 0x5Eu) + card_value(S8(NAT(n) + 0x3Du)) + S8(NAT(n) + 0x3Eu) + 100 * S8(NAT(n) + 0x5Cu) +
           50 * S8(NAT(n) + 0x51u);
}

static int g_score_calls[200], g_nscore;
int f147C4(int n)
{
    if (g_log_score && g_nscore < 200)
        g_score_calls[g_nscore++] = n;
    return score_of(n);
}

void f3CD6C(int8_t pic) { tr("bg %d", pic); }
int fg_vbcopy(int minx, int maxx, int miny, int maxy, int newx, int newy, int source, int dest)
{
    tr("vbcopy %d %d %d %d %d %d %d %d", minx, maxx, miny, maxy, newx, newy, source, dest);
    return 0;
}
int fg_vbopen(int handle)
{
    tr("vbopen %d", handle);
    return 0;
}
void fg_vbclose(void) { tr("vbclose"); }
int fg_setcolor(int color)
{
    tr("color %d", color);
    return 0;
}
void f39F90(int font) { tr("font %d", font); }
void f3A0B0(int x, int y, int w, const char *s) { tr("text %d %d %d %s", x, y, w, s); }
void f3BB48(uint32_t dlg, int id, int x) { tr("setx %X %X %d", (unsigned)dlg, (unsigned)id, x); }
void f3BC64(uint32_t dlg, int id, int y) { tr("sety %X %X %d", (unsigned)dlg, (unsigned)id, y); }
void f3B05C(uint32_t dlg) { tr("dialog %X", (unsigned)dlg); }
void f3CC14(void) { tr("show"); }
void f788AC(void) { tr("pointer"); }
int f78BA4(uint32_t dlg, int map)
{
    int r = g_iwait < g_nwait ? g_wait_ret[g_iwait++] : -1;
    tr("wait %X %d -> %d", (unsigned)dlg, map, r);
    return r;
}

/* ---- Set-up --------------------------------------------------------------------------------------------------------- */

static void crc_table(void)
{
    int32_t i, k;

    memcpy(DP(0xB976Du), "rb", 3);
    for (i = 0; i <= 0xFF; i++) {
        uint32_t c = (uint32_t)i;
        for (k = 8; k > 0; k--)
            c = (c & 1) ? (c >> 1) ^ 0xEDB88320u : c >> 1;
        setD32(0x11260Cu + 4u * (uint32_t)i, c);
    }
}

static uint8_t *read_file(const char *path, long *len)
{
    FILE *f = fopen(path, "rb");
    uint8_t *b;
    long n;

    if (f == NULL)
        return NULL;
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    b = malloc(n > 0 ? (size_t)n : 1);
    if (b == NULL || fread(b, 1, (size_t)n, f) != (size_t)n) {
        fclose(f);
        free(b);
        return NULL;
    }
    fclose(f);
    *len = n;
    return b;
}

static int write_file(const char *path, const uint8_t *b, long n)
{
    FILE *f = fopen(path, "wb");
    int ok;

    if (f == NULL)
        return 0;
    ok = fwrite(b, 1, (size_t)n, f) == (size_t)n;
    fclose(f);
    return ok;
}

static uint8_t *g_pristine;                           /* the data object as state_init made it, with the CRC table */

/* a real save loaded into a fresh data object; [0xF40EC] (players) set as f5FF28 does after the load */
static int load_save(const char *path, uint8_t *out)
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
    memcpy(out, g_ds, DS_SIZE);
    return 1;
}

static uint32_t g_rng = 0x12345678u;
static uint32_t rnd(uint32_t n)
{
    g_rng ^= g_rng << 13;
    g_rng ^= g_rng >> 17;
    g_rng ^= g_rng << 5;
    return n ? g_rng % n : 0;
}

/* ---- A. 0x7C960 ----------------------------------------------------------------------------------------------------- */

static void test_matrix_state(const char *what)
{
    int32_t sc[10], m[10][10], best = -9999, i, j, k = 0, ncalls = 0, calls[200];
    uint32_t leader = 0x5A5A5A5Au;
    int ok = 1;

    setD32(0xF52D4u, leader);
    memset(m, 0, sizeof m);
    for (i = 1; i <= 9; i++)
        sc[i] = score_of(i);
    for (i = 1; i <= 9; i++) {
        if (D8(NAT(i) + 0x9Au) == 0)
            continue;
        calls[ncalls++] = i;
        if (sc[i] > best) {
            best = sc[i];
            leader = (uint32_t)i;
        }
        for (j = 1; j <= 9; j++)
            if (D8(NAT(j) + 0x9Au) != 0) {
                calls[ncalls++] = j;
                m[i][j] = sc[j] - sc[i];
            }
    }
    exp_from_ds();
    for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++)
            E32(0xF52E0u + 40u * (uint32_t)i + 4u * (uint32_t)j, (uint32_t)m[i][j]);
    E32(0xF52D4u, leader);
    g_nscore = 0;
    g_log_score = 1;
    f7C960();
    g_log_score = 0;
    check_ds(__LINE__, what);
    if (g_nscore != ncalls)
        ok = 0;
    for (k = 0; ok && k < ncalls; k++)
        if (g_score_calls[k] != calls[k])
            ok = 0;
    CHECK(ok, "%s: f147C4 call order (%d calls, expected %d)", what, g_nscore, ncalls);
}

static void part_a(uint8_t *const bases[3])
{
    int b, t, n, i;
    char what[64];
    int f0 = g_fail;

    for (b = 0; b < 3; b++) {
        memcpy(g_ds, bases[b], DS_SIZE);
        snprintf(what, sizeof what, "A save %d", b);
        test_matrix_state(what);
        for (t = 0; t < 2000; t++) {
            memcpy(g_ds, bases[b], DS_SIZE);
            for (n = 1; n <= 9; n++) {
                uint32_t r = NAT(n);
                if (rnd(4) == 0)
                    setD8(r + 0x9Au, (uint8_t)(rnd(3) == 0 ? 0 : 2 + rnd(2)));
                if (t % 3 == 0) {                     /* few values: many ties */
                    setD8(r + 0x5Cu, (uint8_t)rnd(3));
                    setD8(r + 0x51u, 0);
                    setD8(r + 0x3Eu, 0);
                    setD16(r + 0x5Eu, 0);
                    for (i = 1; i <= 18; i++)
                        setD8(r + (uint32_t)i, 0);
                } else {
                    setD8(r + 0x5Cu, (uint8_t)rnd(17));
                    setD8(r + 0x51u, (uint8_t)rnd(10));
                    setD8(r + 0x3Eu, (uint8_t)rnd(256));
                    setD16(r + 0x5Eu, (uint16_t)rnd(3000));
                    for (i = 1; i <= 18; i++)
                        setD8(r + (uint32_t)i, (uint8_t)(rnd(3) == 0 ? rnd(5) : 0));
                }
                if (t % 97 == 0)
                    setD8(r + 0x5Cu, (uint8_t)(int8_t)-100);   /* below -9999: nobody leads */
            }
            if (t % 211 == 0)
                for (n = 1; n <= 9; n++)
                    setD8(NAT(n) + 0x9Au, 0);         /* nobody in play */
            snprintf(what, sizeof what, "A save %d random %d", b, t);
            test_matrix_state(what);
        }
    }
    printf("A. score matrix 0x7C960: %s\n", g_fail == f0 ? "pass" : "FAIL");
}

/* ---- B. 0x22B00 ----------------------------------------------------------------------------------------------------- */

/* runs f22B00 under the jump buffer; returns 0 when it returned, else the longjmp code */
static int run_22B00(void)
{
    g_ntr = 0;
    g_iwait = 0;
    if (setjmp(g_jb) == 0) {
        f22B00();
        return 0;
    }
    return 1;
}

/* the model (written from the listing): eliminations on g_exp, expected trace in g_ex; returns 1 if it reaches a
   terminal screen (0x22DCC / 0x22F3C), which the random part avoids */
static int model_22B00(void)
{
#define M8(a)  g_exp[(a) - DS_BASE]
#define MS8(a) ((int8_t)g_exp[(a) - DS_BASE])
    int n, c, j, mm;

    for (n = 1; n <= 9; n++) {
        uint32_t r = NAT(n);
        if (M8(r + 0x9A) == 0 || MS8(r + 0x98) != 0 || MS8(r + 0x51) != 0)
            continue;
        if (MS8(r + 0x9A) == 2) {
            M8(0xE1621u)--;
            ex("msg -500 %d -200 %s has been eliminated from the game. 0", n, NAMES[n]);
        } else if ((M8(0xE1624u) & 0x80) == 0 && MS8(0xE1620u) > 1) {
            M8(0xE1620u)--;
            ex("msg -500 %d -200 %s has been eliminated from the game. 0", n, NAMES[n]);
        } else
            return 1;
        M8(r + 0x9A) = 0;
        {
            uint32_t p = (uint32_t)g_exp[0xF40EC - DS_BASE] | (uint32_t)g_exp[0xF40ED - DS_BASE] << 8 |
                         (uint32_t)g_exp[0xF40EE - DS_BASE] << 16 | (uint32_t)g_exp[0xF40EF - DS_BASE] << 24;
            E32(0xF40ECu, p - 1);
            if (p - 1 == 1)
                return 1;
        }
        for (c = 1; c <= 18; c++)
            while (M8(r + (uint32_t)c) != 0) {
                ex("discard %d", c);
                M8(r + (uint32_t)c)--;
            }
        for (j = 0; j < MS8(r + 0x13); j++)
            ex("discard %d", MS8(r + 0x14 + (uint32_t)j));
        for (mm = 1; mm <= 9; mm++) {
            uint32_t q = NAT(mm);
            if (M8(q + 0x9A) == 0)
                continue;
            for (j = 0; j < MS8(q + 0x13); j++)
                if (MS8(q + 0x1C + (uint32_t)j) == n)
                    M8(q + 0x1C + (uint32_t)j) = 0xFF;
        }
        ex("unported 0x7784C");
    }
    return 0;
#undef M8
#undef MS8
}

static void end_screen_ex(const char *line1, int y1)
{
    ex("bg 21");
    ex("vbcopy 0 639 0 479 0 479 %d %d", S32(0xF52CCu), S32(0xF52C8u));
    ex("vbopen %d", S32(0xF52C8u));
    ex("color 13");
    ex("font 2");
    ex("text 0 %d 640 %s", y1, line1);
}

static void end_screen_wait_ex(void)
{
    int i;

    ex("vbclose");
    ex("setx BB800 2A 265");
    ex("sety BB800 2A 400");
    ex("dialog BB800");
    ex("show");
    for (i = 0; i < g_nwait; i++)
        ex("wait BB800 0 -> %d", g_wait_ret[i]);
}

static void part_b(const uint8_t *r4)
{
    int f0 = g_fail, t, n, c, rc;

    /* B1: the R4 state as it is: nobody is out */
    memcpy(g_ds, r4, DS_SIZE);
    exp_from_ds();
    g_nex = 0;
    rc = run_22B00();
    CHECK(rc == 0, "B1 returned by longjmp");
    check_trace(__LINE__, "B1");
    check_ds(__LINE__, "B1");

    /* B2: IBERIA (computer) loses its last tokens, with cards in hand and givers in other hands */
    memcpy(g_ds, r4, DS_SIZE);
    setD8(NAT(2) + 0x98u, 0);
    setD8(NAT(2) + 0x01u, 2);
    setD8(NAT(2) + 0x05u, 1);
    setD8(NAT(2) + 0x12u, 1);
    setD8(NAT(2) + 0x13u, 2);
    setD8(NAT(2) + 0x14u, 0x13);
    setD8(NAT(2) + 0x15u, 0x19);
    setD8(NAT(2) + 0x1Cu, 4);
    setD8(NAT(3) + 0x13u, 3);
    setD8(NAT(3) + 0x14u, 0x14);
    setD8(NAT(3) + 0x15u, 0x15);
    setD8(NAT(3) + 0x16u, 0x16);
    setD8(NAT(3) + 0x1Cu, 2);
    setD8(NAT(3) + 0x1Du, 9);
    setD8(NAT(3) + 0x1Eu, 2);
    setD8(NAT(6) + 0x13u, 1);                         /* ASIA is not in play: its giver stays */
    setD8(NAT(6) + 0x1Cu, 2);
    setD8(NAT(4) + 0x13u, 1);                         /* a giver byte past the count stays */
    setD8(NAT(4) + 0x1Cu, 9);
    setD8(NAT(4) + 0x1Du, 2);
    exp_from_ds();
    E8(0xE1621u, 6);
    E32(0xF40ECu, 7);
    E8(NAT(2) + 0x9Au, 0);
    E8(NAT(2) + 0x01u, 0);
    E8(NAT(2) + 0x05u, 0);
    E8(NAT(2) + 0x12u, 0);
    E8(NAT(3) + 0x1Cu, 0xFF);
    E8(NAT(3) + 0x1Eu, 0xFF);
    g_nex = 0;
    ex("msg -500 2 -200 IBERIA has been eliminated from the game. 0");
    ex("discard 1");
    ex("discard 1");
    ex("discard 5");
    ex("discard 18");
    ex("discard 19");
    ex("discard 25");
    ex("unported 0x7784C");
    rc = run_22B00();
    CHECK(rc == 0, "B2 returned by longjmp");
    check_trace(__LINE__, "B2");
    check_ds(__LINE__, "B2");

    /* B3: no tokens but a city; tokens but no city; out of play with neither */
    memcpy(g_ds, r4, DS_SIZE);
    setD8(NAT(2) + 0x98u, 0);
    setD8(NAT(2) + 0x51u, 1);
    setD8(NAT(3) + 0x51u, 0);
    setD8(NAT(6) + 0x98u, 0);
    exp_from_ds();
    g_nex = 0;
    rc = run_22B00();
    CHECK(rc == 0, "B3 returned by longjmp");
    check_trace(__LINE__, "B3");
    check_ds(__LINE__, "B3");

    /* B4: two computers (THRACE, BABYLON) out in one call, in n order */
    memcpy(g_ds, r4, DS_SIZE);
    setD8(NAT(4) + 0x98u, 0);
    setD8(NAT(8) + 0x98u, 0);
    exp_from_ds();
    g_nex = 0;
    (void)model_22B00();
    rc = run_22B00();
    CHECK(rc == 0 && g_nex == 4, "B4 returned by longjmp or model length %d", g_nex);
    check_trace(__LINE__, "B4");
    check_ds(__LINE__, "B4");

    /* B5: a human (AFRICA) out while another human (THRACE) remains */
    memcpy(g_ds, r4, DS_SIZE);
    setD8(NAT(4) + 0x9Au, 3);
    setD8(0xE1620u, 2);
    setD8(0xE1621u, 6);
    setD8(NAT(1) + 0x98u, 0);
    setD8(NAT(1) + 0x51u, 0);
    exp_from_ds();
    E8(0xE1620u, 1);
    E32(0xF40ECu, 7);
    E8(NAT(1) + 0x9Au, 0);
    for (c = 1; c <= 18; c++)
        E8(NAT(1) + (uint32_t)c, 0);
    g_nex = 0;
    ex("msg -500 1 -200 AFRICA has been eliminated from the game. 0");
    for (c = 1; c <= 18; c++)
        for (n = 0; n < S8(NAT(1) + (uint32_t)c); n++)
            ex("discard %d", c);
    for (n = 0; n < S8(NAT(1) + 0x13u); n++)
        ex("discard %d", S8(NAT(1) + 0x14u + (uint32_t)n));
    ex("unported 0x7784C");
    rc = run_22B00();
    CHECK(rc == 0, "B5 returned by longjmp");
    check_trace(__LINE__, "B5");
    check_ds(__LINE__, "B5");

    /* B6: the last human out: 0x22DCC's screen (the wait loops on -100 and other codes until -1), longjmp(0xF58D4) */
    memcpy(g_ds, r4, DS_SIZE);
    setD8(NAT(1) + 0x98u, 0);
    setD8(NAT(1) + 0x51u, 0);
    exp_from_ds();
    g_nwait = 3;
    g_wait_ret[0] = -100;
    g_wait_ret[1] = 5;
    g_wait_ret[2] = -1;
    g_nex = 0;
    end_screen_ex("AFRICA, none of your cities or units", 0x32);
    ex("text 0 80 640 remain on the board.");
    ex("text 0 110 640 You have been eliminated from the game.");
    end_screen_wait_ex();
    ex("longjmp 0xF58D4");
    rc = run_22B00();
    CHECK(rc == 1, "B6 did not end in a longjmp");
    check_trace(__LINE__, "B6");
    check_ds(__LINE__, "B6");

    /* B7: every computer out: the seventh leaves one player -> 0x22F3C (AFRICA has conquered the known world) */
    memcpy(g_ds, r4, DS_SIZE);
    for (n = 2; n <= 9; n++)
        setD8(NAT(n) + 0x98u, 0);
    exp_from_ds();
    g_nex = 0;
    for (n = 2; n <= 9; n++) {
        if (n == 6)
            continue;
        ex("msg -500 %d -200 %s has been eliminated from the game. 0", n, NAMES[n]);
        E8(NAT(n) + 0x9Au, 0);
        if (n != 9)
            ex("unported 0x7784C");
    }
    E8(0xE1621u, 0);
    E32(0xF40ECu, 1);
    g_nwait = 1;
    g_wait_ret[0] = -1;
    end_screen_ex("AFRICA, you have conquered the known world!", 0x50);
    end_screen_wait_ex();
    ex("unported 0x14294");
    ex("pointer");
    ex("longjmp 0xF58D4");
    rc = run_22B00();
    CHECK(rc == 1, "B7 did not end in a longjmp");
    check_trace(__LINE__, "B7");
    check_ds(__LINE__, "B7");

    /* B8: a human out with play by e-mail on: not_ported(0x230B8) */
    memcpy(g_ds, r4, DS_SIZE);
    setD8(0xE1624u, (uint8_t)(D8(0xE1624u) | 0x80));
    setD8(NAT(1) + 0x98u, 0);
    setD8(NAT(1) + 0x51u, 0);
    g_nex = 0;
    ex("np 0x230B8");
    rc = run_22B00();
    CHECK(rc == 1, "B8 did not stop");
    check_trace(__LINE__, "B8");

    /* B9: random states against the model (terminal cases skipped) */
    for (t = 0; t < 3000; t++) {
        int players = 0, terminal;
        memcpy(g_ds, r4, DS_SIZE);
        for (n = 1; n <= 9; n++) {
            uint32_t r = NAT(n);
            int k;
            setD8(r + 0x9Au, (uint8_t)(rnd(5) == 0 ? 0 : (rnd(3) == 0 ? 3 : 2)));
            setD8(r + 0x98u, (uint8_t)(rnd(5) < 2 ? 0 : 1 + rnd(20)));
            setD8(r + 0x51u, (uint8_t)(rnd(5) < 3 ? 0 : 1 + rnd(3)));
            for (c = 1; c <= 18; c++)
                setD8(r + (uint32_t)c, (uint8_t)(rnd(4) == 0 ? rnd(4) : 0));
            k = (int)rnd(5);
            setD8(r + 0x13u, (uint8_t)k);
            for (c = 0; c < 8; c++) {
                setD8(r + 0x14u + (uint32_t)c, (uint8_t)(c < k ? 0x13 + rnd(12) : 0));
                setD8(r + 0x1Cu + (uint32_t)c, (uint8_t)(rnd(3) == 0 ? 0xFF : 1 + rnd(9)));
            }
            if (D8(r + 0x9Au) != 0)
                players++;
        }
        setD32(0xF40ECu, (uint32_t)players);
        setD8(0xE1620u, (uint8_t)(1 + rnd(3)));
        setD8(0xE1621u, (uint8_t)rnd(8));
        exp_from_ds();
        g_nex = 0;
        terminal = model_22B00();
        if (terminal)
            continue;
        rc = run_22B00();
        CHECK(rc == 0, "B9 %d returned by longjmp", t);
        check_trace(__LINE__, "B9");
        check_ds(__LINE__, "B9");
    }
    printf("B. eliminations 0x22B00, 0x22DCC, 0x22F3C: %s\n", g_fail == f0 ? "pass" : "FAIL");
}

/* ---- C. the transition table of 0x54824 --------------------------------------------------------------------------- */

static uint8_t *g_base;                               /* the R4 state prepared for part C */
static uint8_t *g_r4file;
static long g_r4len;

static int run_loop(int stop)
{
    int rc;

    g_ntr = 0;
    g_label_calls = 0;
    g_label_stop = stop;
    g_jrc = 0;
    if (setjmp(g_jb) == 0)
        f54824();
    rc = g_jrc;
    return rc;
}

static int run_entry(int restored)
{
    int rc;

    g_ntr = 0;
    g_label_calls = 0;
    g_label_stop = 1;
    g_jrc = 0;
    if (setjmp(g_jb) == 0) {
        if (restored)
            game_loop_restored();
        else
            game_loop_new();
    }
    rc = g_jrc;
    return rc;
}

static void reset_hooks(void)
{
    g_hook_55574 = g_hook_734E4_trades = g_hook_734E4_F4090 = g_hook_1E0A0 = 0;
    g_ast_ret = 0;
}

/* the R4 state with what part C needs: music, effects and digital sound on, no cards, no WAV in slot 0 */
static void base_c(void)
{
    int n, i;

    memcpy(g_ds, g_base, DS_SIZE);
    for (n = 0; n <= 13; n++)
        for (i = 1; i <= 0x13; i++)
            setD8(NAT(n) + (uint32_t)i, 0);
    setD8(0xF097Cu, (uint8_t)(D8(0xF097Cu) | 0x03));
    setD8(0xF625Cu, 1);
    setD8(0xF6118u, 0);
}

static void ex_label(void) { ex("label F596C 101 %d", S16(0xF40FCu)); }

static const char *music_of(int p)
{
    return p >= 12 && p <= 14 ? "gtrade2c.xmi" : p == 15 || p == 16 ? "gcalam3c.xmi" : "gmap__1c.xmi";
}

static void ex_music(int p)
{
    ex("music %s 0", music_of(p));
    if (p >= 12 && p <= 14)
        ex("wav 2 marketpl.wav");
}

/* a phase with the simple shape: f553C0 (if music), the routines, P next, A(p) */
static void simple_phase(int line, int p, int music, const uint32_t *calls, int ncalls, int next)
{
    int i, rc;
    char what[32];

    base_c();
    setD8(0xF4102u, (uint8_t)p);
    exp_from_ds();
    E8(0xF4102u, next);
    g_nex = 0;
    ex_label();
    if (music)
        ex_music(p);
    for (i = 0; i < ncalls; i++)
        ex("unported 0x%X", (unsigned)calls[i]);
    ex("autosave %d phase=%d F5476=%d", p, next, S16(0xF5476u));
    rc = run_loop(2);
    snprintf(what, sizeof what, "C phase %d", p);
    CHECK(rc == 2, "%s: run ended with %d", what, rc);
    check_trace(line, what);
    exp_mask_turn();
    check_ds(line, what);
}

static void part_c(void)
{
    static const uint32_t c1[] = { 0x673A8u }, c2[] = { 0x13D44u }, c3[] = { 0x60AE0u, 0x60BD8u },
                          c4[] = { 0x60AE0u, 0x61940u }, c9[] = { 0x5C238u }, c16[] = { 0x21914u };
    int f0 = g_fail, rc, i, n;
    uint32_t seed;

    reset_hooks();
    simple_phase(__LINE__, 1, 1, c1, 1, 2);
    simple_phase(__LINE__, 2, 1, c2, 1, 3);
    simple_phase(__LINE__, 3, 1, c3, 2, 4);
    simple_phase(__LINE__, 4, 1, c4, 2, 5);
    simple_phase(__LINE__, 9, 1, c9, 1, 10);
    simple_phase(__LINE__, 16, 0, c16, 1, 17);

    /* 0 TAXATION: f7BCD4's 300 game_rand before 0x66050 */
    base_c();
    setD8(0xF4102u, 0);
    exp_from_ds();
    seed = D32(GAME_SEED);
    for (i = 0; i < 300; i++) {
        seed = seed * 0x15A4E35u + 1u;
        E16(0x112A94u + 2u * (uint32_t)i, (uint16_t)((int32_t)((seed >> 16) & 0x7FFF) * 30000 >> 15));
    }
    E32(GAME_SEED, seed);
    E8(0xF4102u, 1);
    g_nex = 0;
    ex_label();
    ex_music(0);
    ex("unported 0x66050");
    ex("autosave 0 phase=1 F5476=%d", S16(0xF5476u));
    g_rand_calls = 0;
    rc = run_loop(2);
    CHECK(rc == 2 && g_rand_calls == 300, "C phase 0: end %d, %ld game_rand", rc, g_rand_calls);
    check_trace(__LINE__, "C phase 0");
    exp_mask_turn();
    check_ds(__LINE__, "C phase 0");

    /* 5 MOVEMENT: the copies before and after 0x55574 (which changes the board here), REPLAY's round */
    base_c();
    setD8(0xF4102u, 5);
    memset(DP(0xE8888u), 0xEE, 0x2B65);               /* the destinations hold a pattern first */
    memset(DP(0xE2FD8u), 0xEE, 0x1E0);
    memset(DP(0xE5D20u), 0xEE, 0x2B65);
    memset(DP(0xE2DF8u), 0xEE, 0x1E0);
    memset(DP(0xF1FB0u), 0xEE, 0xAE2);
    exp_from_ds();
    memcpy(g_exp + (0xE8888u - DS_BASE), DP(0xE31B8u), 0x2B65);
    memcpy(g_exp + (0xE2FD8u - DS_BASE), DP(0xE2C18u), 0x1E0);
    E8(0xE31B8u + 69u * 10u + 5u, D8(0xE31B8u + 69u * 10u + 5u) ^ 0x5A);
    E8(0xE2C18u + 30u, D8(0xE2C18u + 30u) ^ 0x33);
    E8(NAT(2) + 0x50u, D8(NAT(2) + 0x50u) ^ 0x11);
    memcpy(g_exp + (0xE5D20u - DS_BASE), g_exp + (0xE31B8u - DS_BASE), 0x2B65);
    memcpy(g_exp + (0xE2DF8u - DS_BASE), g_exp + (0xE2C18u - DS_BASE), 0x1E0);
    memcpy(g_exp + (0xF1FB0u - DS_BASE), g_exp + (0xF14CCu - DS_BASE), 0xAE2);
    E32(0xEDF58u, 1);
    E32(0xEDF5Cu, (uint32_t)(int32_t)S16(0xF40FCu));
    E8(0xF4102u, 7);
    g_nex = 0;
    ex_label();
    ex_music(5);
    ex("unported 0x55574");
    ex("autosave 5 phase=7 F5476=%d", S16(0xF5476u));
    g_hook_55574 = 1;
    rc = run_loop(2);
    g_hook_55574 = 0;
    CHECK(rc == 2, "C phase 5: end %d", rc);
    check_trace(__LINE__, "C phase 5");
    exp_mask_turn();
    check_ds(__LINE__, "C phase 5");

    /* 6 VIEW MOVEMENT (PBEM only): stops at 0x59298 */
    base_c();
    setD8(0xF4102u, 6);
    setD8(0xF40FEu, 1);
    exp_from_ds();
    E8(0xF40FEu, 0);
    g_nex = 0;
    ex_label();
    ex_music(6);
    ex("np 0x59298");
    rc = run_loop(2);
    CHECK(rc == 3, "C phase 6: end %d", rc);
    check_trace(__LINE__, "C phase 6");
    exp_mask_turn();
    check_ds(__LINE__, "C phase 6");

    /* 7 CONFLICT: [0xF40FE] and bit 0x01 of [0xE1624] cleared */
    base_c();
    setD8(0xF4102u, 7);
    setD8(0xF40FEu, 1);
    setD8(0xE1624u, (uint8_t)(D8(0xE1624u) | 1));
    exp_from_ds();
    E8(0xF40FEu, 0);
    E8(0xE1624u, D8(0xE1624u) & ~1u);
    E8(0xF4102u, 8);
    g_nex = 0;
    ex_label();
    ex_music(7);
    ex("unported 0x5AAA4");
    ex("autosave 7 phase=8 F5476=%d", S16(0xF5476u));
    rc = run_loop(2);
    CHECK(rc == 2, "C phase 7: end %d", rc);
    check_trace(__LINE__, "C phase 7");
    exp_mask_turn();
    check_ds(__LINE__, "C phase 7");

    /* 8 CITY CONSTRUCTION: int16 [0xF408E] = 0 */
    base_c();
    setD8(0xF4102u, 8);
    setD16(0xF408Eu, 5);
    exp_from_ds();
    E16(0xF408Eu, 0);
    E8(0xF4102u, 9);
    g_nex = 0;
    ex_label();
    ex_music(8);
    ex("unported 0x32214");
    ex("autosave 8 phase=9 F5476=%d", S16(0xF5476u));
    rc = run_loop(2);
    CHECK(rc == 2, "C phase 8: end %d", rc);
    check_trace(__LINE__, "C phase 8");
    exp_mask_turn();
    check_ds(__LINE__, "C phase 8");

    /* 10 CITY SUPPORT: [0x109E20] = 0; [0xF5476] = 1 only after the autosave */
    base_c();
    setD8(0xF4102u, 10);
    setD16(0x109E20u, 5);
    setD16(0xF5476u, 7);
    exp_from_ds();
    E16(0x109E20u, 0);
    E16(0xF5476u, 1);
    E8(0xF4102u, 11);
    g_nex = 0;
    ex_label();
    ex_music(10);
    ex("unported 0x5C62C");
    ex("autosave 10 phase=11 F5476=7");
    rc = run_loop(2);
    CHECK(rc == 2, "C phase 10: end %d", rc);
    check_trace(__LINE__, "C phase 10");
    exp_mask_turn();
    check_ds(__LINE__, "C phase 10");

    /* 11 ACQUIRE GOODS: no f553C0; [0xF5478] = 0, [0x110F74] = 0 after */
    base_c();
    setD8(0xF4102u, 11);
    setD16(0xF5478u, 5);
    setD8(0x110F74u, 1);
    exp_from_ds();
    E16(0xF5478u, 0);
    E8(0x110F74u, 0);
    E8(0xF4102u, 12);
    g_nex = 0;
    ex_label();
    ex("unported 0x6BD98");
    ex("autosave 11 phase=12 F5476=%d", S16(0xF5476u));
    rc = run_loop(2);
    CHECK(rc == 2, "C phase 11: end %d", rc);
    check_trace(__LINE__, "C phase 11");
    exp_mask_turn();
    check_ds(__LINE__, "C phase 11");

    /* 12a DECLARE OFFERS with two nations holding the minimum (6) */
    base_c();
    setD8(0xF4102u, 12);
    setD8(NAT(2) + 0x01u, 6);
    setD8(NAT(3) + 0x02u, 6);
    setD32(0xF4230u, 0);
    for (n = 0; n < 12; n++)
        setD8(0xF09D4u + (uint32_t)n, 1);
    exp_from_ds();
    E32(0xF4230u, 1);
    for (n = 0; n <= 9; n++)
        E8(0xF09D4u + (uint32_t)n, 0);
    E8(0xF4102u, 13);
    g_nex = 0;
    ex_label();
    ex_music(12);
    ex("unported 0x6F270");
    ex("autosave 12 phase=13 F5476=%d", S16(0xF5476u));
    rc = run_loop(2);
    CHECK(rc == 2, "C phase 12a: end %d", rc);
    check_trace(__LINE__, "C phase 12a");
    exp_mask_turn();
    check_ds(__LINE__, "C phase 12a");

    /* 12b: only one nation with the minimum, a session was held before: the message, phase 15, no autosave */
    base_c();
    setD8(0xF4102u, 12);
    setD8(NAT(2) + 0x01u, 6);
    setD8(NAT(6) + 0x01u, 6);                         /* ASIA is not in play */
    setD32(0xF4230u, 1);
    exp_from_ds();
    E8(0xF4102u, 15);
    snprintf((char *)g_exp + (0xF5500u - DS_BASE), 0x1F4,
             "Lack of nations with sufficient trade goods (6 min) cancels this round of trading.");
    g_nex = 0;
    ex_label();
    ex("box Lack of nations with sufficient trade goods (6 min) cancels this round of trading.");
    rc = run_loop(2);
    CHECK(rc == 2, "C phase 12b: end %d", rc);
    check_trace(__LINE__, "C phase 12b");
    exp_mask_turn();
    check_ds(__LINE__, "C phase 12b");

    /* 12c: no session ever held: no message */
    base_c();
    setD8(0xF4102u, 12);
    setD32(0xF4230u, 0);
    exp_from_ds();
    E8(0xF4102u, 15);
    g_nex = 0;
    ex_label();
    rc = run_loop(2);
    CHECK(rc == 2, "C phase 12c: end %d", rc);
    check_trace(__LINE__, "C phase 12c");
    exp_mask_turn();
    check_ds(__LINE__, "C phase 12c");

    /* 12d, 13b, 14f: byte [0xF408C] set (PBEM only) */
    for (i = 12; i <= 14; i++) {
        base_c();
        setD8(0xF4102u, (uint8_t)i);
        setD8(0xF408Cu, 1);
        exp_from_ds();
        g_nex = 0;
        ex_label();
        if (i == 12)
            ex("wrapup");
        ex("np 0x50D08");
        rc = run_loop(2);
        CHECK(rc == 3, "C phase %d with [0xF408C]: end %d", i, rc);
        check_trace(__LINE__, "C phase 12..14 with [0xF408C]");
        exp_mask_turn();
        check_ds(__LINE__, "C phase 12..14 with [0xF408C]");
    }

    /* 13 COUNTER OFFERS */
    base_c();
    setD8(0xF4102u, 13);
    exp_from_ds();
    E8(0xF4102u, 14);
    g_nex = 0;
    ex_label();
    ex_music(13);
    ex("unported 0x723DC");
    ex("autosave 13 phase=14 F5476=%d", S16(0xF5476u));
    rc = run_loop(2);
    CHECK(rc == 2, "C phase 13: end %d", rc);
    check_trace(__LINE__, "C phase 13");
    exp_mask_turn();
    check_ds(__LINE__, "C phase 13");

    /* 14 ACCEPT TRADES, variants:
       a no trades made: phase 15 (a WAV name in slot 0 stays: only phases 15 / 16 free it)
       b trades, F09D4 1..5 set; min 3: human AFRICA 3 (counted), computers 6, 7, 5 (not: a computer needs 6), 6:
         count 4 = need (8 + 1) / 2, session 1 < 2: phase 12, session 2
       c the same with session 2 already: phase 15
       d CRETE holds 5: count 3 < 4: phase 15;  e the same with bit 0x10 of [0xE1625]: need 2: phase 12
       g [0xF4090] set by 0x734E4: no wrap-up, no autosave, phase 14 again
       h as d with 7 players (need 4): phase 15;  i as d with 5 players (need 3): phase 12 */
    {
        static const struct {
            char tag;
            int trades, f4090, session, crete, bit10, next, nsess, wrapup, autosave, players;
        } V[] = {
            { 'a', 0, 0, 1, 6, 0, 15, 1, 0, 1, 8 }, { 'b', 1, 0, 1, 6, 0, 12, 2, 1, 1, 8 },
            { 'c', 1, 0, 2, 6, 0, 15, 2, 1, 1, 8 }, { 'd', 1, 0, 1, 5, 0, 15, 1, 1, 1, 8 },
            { 'e', 1, 0, 1, 5, 1, 12, 2, 1, 1, 8 }, { 'g', 1, 1, 1, 6, 0, 14, 1, 0, 0, 8 },
            { 'h', 1, 0, 1, 5, 0, 15, 1, 1, 1, 7 }, { 'i', 1, 0, 1, 5, 0, 12, 2, 1, 1, 5 },
        };
        size_t v;
        char what[32];

        for (v = 0; v < sizeof V / sizeof V[0]; v++) {
            base_c();
            setD8(0xF4102u, 14);
            setD8(0xE1623u, 0x32);
            setD16(0xF5476u, (uint16_t)V[v].session);
            setD16(0xF4100u, 9);
            setD8(0x110F74u, 1);
            setD32(0xF40ECu, (uint32_t)V[v].players); /* need (players + 1) / 2: 4, 4, 3 */
            setD8(0xF6118u, 'x');
            setD8(0xF6119u, 0);
            if (V[v].bit10)
                setD8(0xE1625u, (uint8_t)(D8(0xE1625u) | 0x10));
            for (n = 0; n < 12; n++)
                setD8(0xF09D4u + (uint32_t)n, (uint8_t)(n >= 1 && n <= 5));
            setD8(NAT(1) + 0x01u, 3);
            setD8(NAT(2) + 0x01u, 6);
            setD8(NAT(3) + 0x01u, 7);
            setD8(NAT(4) + 0x01u, 5);
            setD8(NAT(5) + 0x01u, (uint8_t)V[v].crete);
            setD8(NAT(7) + 0x01u, 9);                 /* ASSYRIA does not want a session (F09D4 0) */
            exp_from_ds();
            E8(0x110F74u, V[v].trades);
            E16(0xF4090u, (uint16_t)V[v].f4090);
            E16(0xF4100u, 0);
            E8(0xF4102u, V[v].next);
            E16(0xF5476u, (uint16_t)V[v].nsess);
            g_nex = 0;
            ex_label();
            ex_music(14);
            ex("unported 0x734E4");
            if (V[v].wrapup)
                ex("wrapup");
            if (V[v].autosave)
                ex("autosave 14 phase=%d F5476=%d", V[v].next, V[v].nsess);
            if (V[v].next == 15) {                    /* the next turn's top frees slot 0's WAV before the label */
                ex("unported 0x7CB9D");
                E8(0xF6118u, 0);
            }
            g_hook_734E4_trades = V[v].trades;
            g_hook_734E4_F4090 = V[v].f4090;
            rc = run_loop(2);
            reset_hooks();
            snprintf(what, sizeof what, "C phase 14%c", V[v].tag);
            CHECK(rc == 2, "%s: end %d", what, rc);
            check_trace(__LINE__, what);
            exp_mask_turn();
            check_ds(__LINE__, what);
        }
    }

    /* 15 CALAMITIES: the slot-0 WAV freed at the top (0x7CB9D), the counters, [0xF7AFC/E] cleared after 0x1E0A0 */
    base_c();
    setD8(0xF4102u, 15);
    memcpy(DP(0xF6118u), "boom.wav", 9);
    setD16(0xF5476u, 5);
    setD8(0xF408Cu, 1);
    setD16(0xF4094u, 3);
    setD16(0xF7AFCu, 7);
    setD16(0xF7AFEu, 8);
    exp_from_ds();
    E8(0xF6118u, 0);
    E16(0xF5476u, 1);
    E8(0xF408Cu, 0);
    E16(0xF4094u, 0);
    E16(0xF7AFCu, 0);
    E16(0xF7AFEu, 0);
    E8(0xF4102u, 16);
    g_nex = 0;
    ex("unported 0x7CB9D");
    ex_label();
    ex("unported 0x1E0A0");
    ex("autosave 15 phase=16 F5476=1");
    g_hook_1E0A0 = 1;
    rc = run_loop(2);
    reset_hooks();
    CHECK(rc == 2, "C phase 15: end %d", rc);
    check_trace(__LINE__, "C phase 15");
    exp_mask_turn();
    check_ds(__LINE__, "C phase 15");

    /* 15 without digital sound: the WAV name stays */
    base_c();
    setD8(0xF4102u, 15);
    setD8(0xF625Cu, 0);
    memcpy(DP(0xF6118u), "boom.wav", 9);
    exp_from_ds();
    E16(0xF5476u, 1);
    E8(0xF4102u, 16);
    g_nex = 0;
    ex_label();
    ex("unported 0x1E0A0");
    ex("autosave 15 phase=16 F5476=1");
    rc = run_loop(2);
    CHECK(rc == 2, "C phase 15 no digital: end %d", rc);
    check_trace(__LINE__, "C phase 15 no digital");
    exp_mask_turn();
    check_ds(__LINE__, "C phase 15 no digital");

    /* 17 CITY SUPPORT again */
    base_c();
    setD8(0xF4102u, 17);
    setD16(0x109E20u, 5);
    setD16(0xF7AFCu, 7);
    setD16(0xF7AFEu, 8);
    exp_from_ds();
    E16(0x109E20u, 0);
    E16(0xF7AFCu, 0);
    E16(0xF7AFEu, 0);
    E8(0xF4102u, 18);
    g_nex = 0;
    ex_label();
    ex_music(17);
    ex("unported 0x5C62C");
    ex("autosave 17 phase=18 F5476=%d", S16(0xF5476u));
    rc = run_loop(2);
    CHECK(rc == 2, "C phase 17: end %d", rc);
    check_trace(__LINE__, "C phase 17");
    exp_mask_turn();
    check_ds(__LINE__, "C phase 17");

    /* 18 ACQUIRE CIVILIZATION: [0xF09E4] = some nation (in play or not) has +0x28 */
    for (i = 0; i < 2; i++) {
        base_c();
        setD8(0xF4102u, 18);
        setD16(0xF4092u, 3);
        setD32(0xF09E4u, 7);
        for (n = 1; n <= 9; n++)
            setD8(NAT(n) + 0x28u, 0);
        if (i == 0)
            setD8(NAT(6) + 0x28u, 1);                 /* ASIA, not in play */
        exp_from_ds();
        E16(0xF4092u, 0);
        E32(0xF09E4u, i == 0 ? 1u : 0u);
        E8(0xF4102u, 19);
        g_nex = 0;
        ex_label();
        ex_music(18);
        ex("unported 0x162D0");
        ex("unported 0x1A3A0");
        ex("autosave 18 phase=19 F5476=%d", S16(0xF5476u));
        rc = run_loop(2);
        CHECK(rc == 2, "C phase 18 (%d): end %d", i, rc);
        check_trace(__LINE__, "C phase 18");
        exp_mask_turn();
        check_ds(__LINE__, "C phase 18");
    }

    /* 19 DISCARD EXCESS */
    base_c();
    setD8(0xF4102u, 19);
    setD16(0xF4096u, 3);
    exp_from_ds();
    E16(0xF4096u, 0);
    E8(0xF4102u, 20);
    g_nex = 0;
    ex_label();
    ex_music(19);
    ex("unported 0x6E770");
    ex("autosave 19 phase=20 F5476=%d", S16(0xF5476u));
    rc = run_loop(2);
    CHECK(rc == 2, "C phase 19: end %d", rc);
    check_trace(__LINE__, "C phase 19");
    exp_mask_turn();
    check_ds(__LINE__, "C phase 19");

    /* 20a SHOW AST, the game goes on: savgameb.dat written, phase 0, autosave(20) */
    {
        long len = 0;
        uint8_t *f;
        char desc[0x33];
        uint32_t crc;

        base_c();
        setD8(0xF4102u, 20);
        (void)write_file("savgamea.dat", g_r4file, g_r4len);
        remove("savgameb.dat");
        g_body = g_r4file + 0x4B;
        g_body_len = g_r4len - 0x4B - 4;
        exp_from_ds();
        E8(0xF4102u, 0);
        memset(desc, 0, sizeof desc);
        snprintf(desc, sizeof desc, "AutoSaved at end of Round %d", S16(0xF40FCu));
        g_nex = 0;
        ex_label();
        ex_music(20);
        ex("write %s", desc);
        ex("ast 3");
        ex("autosave 20 phase=0 F5476=%d", S16(0xF5476u));
        rc = run_loop(2);
        CHECK(rc == 2, "C phase 20a: end %d", rc);
        check_trace(__LINE__, "C phase 20a");
        exp_mask_turn();
        check_ds(__LINE__, "C phase 20a");
        f = read_file("savgameb.dat", &len);
        CHECK(f != NULL && len == g_r4len, "C phase 20a: savgameb.dat %ld bytes, expected %ld", len, g_r4len);
        if (f != NULL && len == g_r4len) {
            crc = f7BB84((int32_t)(len - 0x4B - 4), 0, f + 0x4B);
            CHECK(f[0] == 0x38 && memcmp(f + 2, "Advanced Civilization", 22) == 0 && memcmp(f + 24, desc, 0x33) == 0 &&
                      memcmp(f + 0x4B, g_body, (size_t)g_body_len) == 0 &&
                      f[len - 4] == (uint8_t)crc && f[len - 3] == (uint8_t)(crc >> 8) &&
                      f[len - 2] == (uint8_t)(crc >> 16) && f[len - 1] == (uint8_t)(crc >> 24),
                  "C phase 20a: savgameb.dat content");
            CHECK(f7BBF0("savgameb.dat") == 0, "C phase 20a: savgameb.dat CRC");
        }
        free(f);
    }

    /* 20b SHOW AST, someone won: no autosave; 0x14294, 0x55304 (savgamea.dat + 4 bytes), savgameb.dat removed,
       [0xF5500] = 0, quit */
    {
        long len = 0;
        uint8_t *f;
        char d1[0x33], d2[0x33];
        FILE *probe;

        base_c();
        setD8(0xF4102u, 20);
        setD8(0xF5500u, 'Z');
        (void)write_file("savgamea.dat", g_r4file, g_r4len);
        g_body = g_r4file + 0x4B;
        g_body_len = g_r4len - 0x4B - 4;
        g_ast_ret = 1;
        memset(d1, 0, sizeof d1);
        snprintf(d1, sizeof d1, "AutoSaved at end of Round %d", S16(0xF40FCu));
        memcpy(d2, g_r4file + 24, sizeof d2);        /* f5F05C reads the old description into the buffer; the */
        snprintf(d2, sizeof d2, "AutoSaved at End of Game");   /* sprintf leaves its tail after the NUL */
        d2[50] = 0;
        g_nex = 0;
        ex_label();
        ex_music(20);
        ex("write %s", d1);
        ex("ast 3");
        ex("unported 0x14294");
        ex("write %s", d2);
        ex("quit F5500=0");
        rc = run_loop(2);
        reset_hooks();
        CHECK(rc == 5, "C phase 20b: end %d", rc);
        check_trace(__LINE__, "C phase 20b");
        probe = fopen("savgameb.dat", "rb");
        CHECK(probe == NULL, "C phase 20b: savgameb.dat not removed");
        if (probe != NULL)
            fclose(probe);
        f = read_file("savgamea.dat", &len);
        CHECK(f != NULL && len == g_r4len + 4, "C phase 20b: savgamea.dat %ld bytes, expected %ld", len, g_r4len + 4);
        if (f != NULL && len == g_r4len + 4) {
            CHECK(memcmp(f + 24, d2, 0x33) == 0 && memcmp(f + 0x4B, g_r4file + 0x4B, (size_t)(g_r4len - 0x4B)) == 0 &&
                      f[len - 4] == 0 && f[len - 3] == 0 && f[len - 2] == 0 && f[len - 1] == 0,
                  "C phase 20b: savgamea.dat = header + body + old CRC + 0");
            CHECK(f7BBF0("savgamea.dat") == 0, "C phase 20b: savgamea.dat CRC");
        }
        free(f);
    }

    /* above 20: only 0x22B00, the phase stays (the original loops there for ever) */
    {
        static const int P[] = { 21, 0x80, 0xFF };
        size_t k;
        for (k = 0; k < sizeof P / sizeof P[0]; k++) {
            base_c();
            setD8(0xF4102u, (uint8_t)P[k]);
            exp_from_ds();
            g_nex = 0;
            ex_label();
            ex_label();
            rc = run_loop(3);
            CHECK(rc == 2, "C phase %d: end %d", P[k], rc);
            check_trace(__LINE__, "C phase above 20");
            exp_mask_turn();
            check_ds(__LINE__, "C phase above 20");
        }
    }

    /* PBEM on: phase 5 stops at 0x6E53C after its autosave (next phase 6) */
    base_c();
    setD8(0xF4102u, 5);
    setD8(0xE1624u, (uint8_t)(D8(0xE1624u) | 0x80));
    g_nex = 0;
    ex_label();
    ex_music(5);
    ex("unported 0x55574");
    ex("autosave 5 phase=6 F5476=%d", S16(0xF5476u));
    ex("np 0x6E53C");
    rc = run_loop(2);
    CHECK(rc == 3, "C phase 5 PBEM: end %d", rc);
    check_trace(__LINE__, "C phase 5 PBEM");

    /* two turns in a row: 18 -> 19 -> 20 (the label shows the round each time) */
    base_c();
    setD8(0xF4102u, 18);
    g_nex = 0;
    ex_label();
    ex_music(18);
    ex("unported 0x162D0");
    ex("unported 0x1A3A0");
    ex("autosave 18 phase=19 F5476=%d", S16(0xF5476u));
    ex_label();
    ex_music(19);
    ex("unported 0x6E770");
    ex("autosave 19 phase=20 F5476=%d", S16(0xF5476u));
    rc = run_loop(3);
    CHECK(rc == 2 && D8(0xF4102u) == 20, "C 18..19: end %d, phase %d", rc, D8(0xF4102u));
    check_trace(__LINE__, "C 18..19");

    printf("C. game loop 0x54824, transition table: %s\n", g_fail == f0 ? "pass" : "FAIL");
}

/* ---- D. 0x553C0 ----------------------------------------------------------------------------------------------------- */

static void part_d(void)
{
    int f0 = g_fail, p, flags, dig;

    for (p = 0; p <= 21; p++)
        for (flags = 0; flags < 4; flags++)
            for (dig = 0; dig < 2; dig++) {
                char what[48];
                memcpy(g_ds, g_base, DS_SIZE);
                setD8(0xF4102u, (uint8_t)p);
                setD8(0xF097Cu, (uint8_t)((D8(0xF097Cu) & ~3u) | (unsigned)flags));
                setD8(0xF625Cu, (uint8_t)dig);
                exp_from_ds();
                g_ntr = 0;
                g_nex = 0;
                if (flags & 2)
                    ex("music %s 0", music_of(p));
                if ((flags & 1) && p >= 12 && p <= 14)
                    ex("wav 2 marketpl.wav");
                f553C0();
                snprintf(what, sizeof what, "D phase %d flags %d digital %d", p, flags, dig);
                check_trace(__LINE__, what);
                check_ds(__LINE__, what);
            }
    printf("D. phase music 0x553C0: %s\n", g_fail == f0 ? "pass" : "FAIL");
}

/* ---- E. 0x14AA4, F. 0x55500, G. the entries, H. 0x55258 / 0x55304 ------------------------------------------------- */

static void part_efgh(void)
{
    int f0 = g_fail, t, n, rc, cnt;

    /* E: override maps for THRACE (in play) and ASIA (not), the copy 0xEED84 -> 0xEF3C4 */
    memcpy(g_ds, g_base, DS_SIZE);
    for (n = 0; n < 0x640; n++)
        setD8(0xEF3C4u + (uint32_t)n, 0xAA);
    setD32(0xF60E8u + 16u, 1);
    setD32(0xF60E8u + 24u, 1);
    memcpy(DP(0xF6064u + 13u * 4u), "thrace.bpm", 11);
    exp_from_ds();
    memcpy(g_exp + (0xEF3C4u - DS_BASE), DP(0xEED84u), 0x640);
    g_ntr = 0;
    g_nex = 0;
    ex("bpm 4 thrace.bpm");
    f14AA4();
    check_trace(__LINE__, "E 0x14AA4");
    check_ds(__LINE__, "E 0x14AA4");

    /* F: 0x55500 */
    for (t = 0; t < 5000; t++) {
        int want;
        memcpy(g_ds, g_base, DS_SIZE);
        setD8(0xE1623u, (uint8_t)(rnd(16) << 4 | rnd(16)));
        cnt = 0;
        for (n = 1; n <= 9; n++) {
            int i, s = 0;
            setD8(NAT(n) + 0x9Au, (uint8_t)(rnd(3) == 0 ? 0 : 2 + rnd(2)));
            for (i = 1; i <= 0x13; i++) {
                setD8(NAT(n) + (uint32_t)i, (uint8_t)(rnd(3) == 0 ? rnd(4) : 0));
                s += S8(NAT(n) + (uint32_t)i);
            }
            if (D8(NAT(n) + 0x9Au) != 0 && s >= (D8(0xE1623u) >> 4))
                cnt++;
        }
        want = cnt >= 2;
        exp_from_ds();
        CHECK(f55500() == want, "F %d: f55500 != %d", t, want);
        check_ds(__LINE__, "F");
    }

    /* G: main 0x1188C by the bits of [0xE1624] */
    {
        static const struct { uint8_t flags; int restored, copy, pbem; } V[] = {
            { 0x00, 1, 1, 0 }, { 0x02, 1, 1, 0 }, { 0x04, 1, 0, 0 }, { 0x80, 1, 0, 0 }, { 0x84, 1, 0, 1 },
            { 0x06, 0, 0, 0 }, { 0x84, 0, 0, 1 },
        };
        size_t v;
        for (v = 0; v < sizeof V / sizeof V[0]; v++) {
            base_c();
            setD8(0xF4102u, 4);
            setD8(0xE1624u, V[v].flags);
            for (n = 0; n < 0x640; n++)
                setD8(0xEF3C4u + (uint32_t)n, 0xAA);
            exp_from_ds();
            if (V[v].copy)
                memcpy(g_exp + (0xEF3C4u - DS_BASE), DP(0xEED84u), 0x640);
            g_nex = 0;
            if (V[v].pbem)
                ex("np 0x14A00");
            rc = run_entry(V[v].restored);
            CHECK(rc == (V[v].pbem ? 3 : 2), "G %u: end %d", (unsigned)v, rc);
            check_trace(__LINE__, "G");
            exp_mask_turn();
            check_ds(__LINE__, "G");
        }
    }

    /* H: 0x55258 and 0x55304 on the real R4 file */
    {
        long len = 0;
        uint8_t *f;
        char desc[0x33];
        uint32_t crc;

        memcpy(g_ds, g_base, DS_SIZE);
        setD16(0xF40FCu, 7);
        g_body = g_r4file + 0x4B;
        g_body_len = g_r4len - 0x4B - 4;
        remove("savgameb.dat");
        g_ntr = 0;
        f55258();
        f = read_file("savgameb.dat", &len);
        memset(desc, 0, sizeof desc);
        snprintf(desc, sizeof desc, "AutoSaved at end of Round 7");
        CHECK(f != NULL && len == g_r4len && memcmp(f + 24, desc, 0x33) == 0, "H 0x55258: savgameb.dat");
        if (f != NULL && len == g_r4len) {
            crc = f7BB84((int32_t)(len - 0x4B - 4), 0, f + 0x4B);
            CHECK(memcmp(f + len - 4, g_r4file + len - 4, 4) == 0 && f[len - 4] == (uint8_t)crc,
                  "H 0x55258: the CRC is the original file's");
        }
        free(f);

        (void)write_file("savgamea.dat", g_r4file, g_r4len);
        memset(g_ds, 0, DS_SIZE);
        memcpy(g_ds, g_pristine, DS_SIZE);
        f55304();
        f = read_file("savgamea.dat", &len);
        memcpy(desc, g_r4file + 24, sizeof desc);    /* the old description's tail stays after the NUL */
        snprintf(desc, sizeof desc, "AutoSaved at End of Game");
        desc[50] = 0;
        CHECK(f != NULL && len == g_r4len + 4 && memcmp(f + 24, desc, 0x33) == 0 &&
                  memcmp(f + 0x4B, g_r4file + 0x4B, (size_t)(g_r4len - 0x4B)) == 0 &&
                  f[len - 1] == 0 && f[len - 2] == 0 && f[len - 3] == 0 && f[len - 4] == 0,
              "H 0x55304: savgamea.dat grows by 4 (old CRC kept, 0 appended)");
        CHECK(f7BBF0("savgamea.dat") == 0, "H 0x55304: still intact");
        CHECK(D8(0xF4102u) == g_base[0xF4102u - DS_BASE] && D8(0xF40FCu) == g_base[0xF40FCu - DS_BASE] &&
                  memcmp(DP(0xF14CCu), g_base + (0xF14CCu - DS_BASE), 199 * 14) == 0,
              "H 0x55304: the autosave reloaded into the game");
        free(f);
        remove("savgamea.dat");
        remove("savgameb.dat");
    }
    printf("E-H. 0x14AA4, 0x55500, main 0x1188C, 0x55258 / 0x55304: %s\n", g_fail == f0 ? "pass" : "FAIL");
}

/* ---- main ------------------------------------------------------------------------------------------------------------ */

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:\\work\\advciv_run";
    const char *savedir = argc > 2 ? argv[2] : gamedir;
    static const char *const SAVES[3] = { "SAVGAMEA.DAT", "savgameb.dat", "savgame0.dat" };
    uint8_t *bases[3];
    char path[1024];
    int b;

    if (state_init(gamedir) != 0) {
        fprintf(stderr, "cannot load %s/AC_MAIN.EXE\n", gamedir);
        return 2;
    }
    crc_table();
    g_pristine = malloc(DS_SIZE);
    g_exp = malloc(DS_SIZE);
    g_base = malloc(DS_SIZE);
    if (g_pristine == NULL || g_exp == NULL || g_base == NULL)
        return 2;
    memcpy(g_pristine, g_ds, DS_SIZE);
    for (b = 0; b < 3; b++) {
        bases[b] = malloc(DS_SIZE);
        snprintf(path, sizeof path, "%s/%s", savedir, SAVES[b]);
        if (bases[b] == NULL || !load_save(path, bases[b])) {
            fprintf(stderr, "cannot read %s\n", path);
            return 2;
        }
    }
    snprintf(path, sizeof path, "%s/%s", savedir, SAVES[0]);
    g_r4file = read_file(path, &g_r4len);
    if (g_r4file == NULL || g_r4len < 0x100) {
        fprintf(stderr, "cannot read %s\n", path);
        return 2;
    }
    memcpy(g_base, bases[0], DS_SIZE);
    printf("R4: phase %d, round %d, humans %d, computers %d, players %d, [0xE1624] %02X\n",
           g_base[0xF4102u - DS_BASE], (int)(int16_t)(g_base[0xF40FCu - DS_BASE] | g_base[0xF40FDu - DS_BASE] << 8),
           g_base[0xE1620u - DS_BASE], g_base[0xE1621u - DS_BASE], g_base[0xF40ECu - DS_BASE],
           g_base[0xE1624u - DS_BASE]);

    part_a(bases);
    part_b(bases[0]);
    part_c();
    part_d();
    part_efgh();

    printf("%d checks, %d failed\n", g_tests, g_fail);
    return g_fail == 0 ? 0 : 1;
}
