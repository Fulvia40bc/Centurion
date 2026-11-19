/* census_test [GAMEDIR [PLAYDIR]]: unit checks for module CENSUS (census.c) that need no game run.

   GAMEDIR holds AC_MAIN.EXE (the data object, state_init; default C:\work\advciv_run, only read) and savgame0.dat
   (start of MOVEMENT R4); PLAYDIR holds SAVGAMEA.DAT (the R4 autosave, start of SHIP CONSTRUCTION) and savgameb.dat
   ("end of Round 3", start of TAXATION R4) (default C:\work\advciv_play, only read). Saves are read with saveload.c's
   f5F05C into the data object, as the game does. Expectations are written here from the disassembly, independently
   of census.c and wqsort.c:
     A. the comparator 0x13DE8, exhaustively: every pair of token bytes +0x98 (0..255, compared as int8) for every
        pair of nations 0..13 (a == b included), against the rule read at 0x13E17..0x13E46; plus the int16 tie test
        with nation words outside 0..13 that index records still inside the data object.
     B. 0x13D44 on the three real saves: the f79270 call (100, int16 [0xF06B0], -200, "", 0) made first, before any
        write; the count int16 [0xF0698], the order 0xF069A[0..count-1] against the model; the slots past the count
        unchanged; no other byte of the data object changed. The order the original itself stored in each save (its
        last census) is printed beside the port's, and must agree when the tokens on the board did not change since
        that census (SAVGAMEA.DAT: SHIP MAINTENANCE only ran after it, G).
     C. 20,000 random states (on a copy of the R4 state): +0x9A of nations 1..9 zero or not, +0x98 from a small range
        (many ties) or any byte (int8 order), garbage in the order slots, the count and the nation records 0, 10..13
        (+0x98 and +0x9A, so a loop over 0..9 or 1..10 would show);
        the same checks as B. The model sorts with an independent Watcom-qsort model (the insertion sort that
        0x8C331 runs for n < 16: gaps 3w then w, compare (j - gap, j), swap while > 0) on a plain int array with an
        independent comparator, and also with a plain selection sort by (tokens desc, nation asc): both must agree
        with the port.
     D. wc_qsort's compare trace on the census array: wc_qsort called directly on 0xF069A with a recording wrapper of
        f13DE8 must make exactly the model's compare calls, pair by pair, for random states (the order of compares is
        Watcom's, not only the result).
   Prints each failure, the counts per part, and exits 0 when all pass.

   Build: clang -std=c11 port/test/census_test.c port/src/census.c port/src/wqsort.c port/src/saveload.c
          port/src/state.c port/src/le_image.c -Iport/src -o census_test.exe   (f79270 and what saveload.c needs
          besides, game_rand, f76F70 and the heap handles, are stubs below, as in loop_test.c) */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "state.h"
#include "game.h"
#include "kit.h"
#include "wqsort.h"
#include "saveload.h"
#include "census.h"

#define NAT(n, off) (0xF14CCu + 199u * (uint32_t)(n) + (uint32_t)(off))
#define COUNT 0xF0698u
#define ORDER 0xF069Au

static int g_tests, g_fail;

static void fail(const char *fmt, const char *what, long a, long b)
{
    g_fail++;
    if (g_fail <= 40) {
        fprintf(stderr, "FAIL %s: ", what);
        fprintf(stderr, fmt, a, b);
        fputc('\n', stderr);
    }
}

/* ---- stubs for saveload.c (as loop_test.c's) ------------------------------------------------------------------- */

int f78AE4(void)
{
    uint32_t seed = D32(0xCAC5Cu) * 0x15A4E35u + 1u;
    setD32(0xCAC5Cu, seed);
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

/* ---- f79270 stub --------------------------------------------------------------------------------------------- */

static int g_msg_calls;
static int g_msg_args[4];
static char g_msg_text[64];
static uint8_t *g_msg_ds;             /* the data object at the time of the call */

void f79270(int wait, int n, int t, const char *text, int anim)
{
    g_msg_calls++;
    g_msg_args[0] = wait;
    g_msg_args[1] = n;
    g_msg_args[2] = t;
    g_msg_args[3] = anim;
    snprintf(g_msg_text, sizeof g_msg_text, "%s", text);
    memcpy(g_msg_ds, g_ds, DS_SIZE);
}

/* ---- the model ------------------------------------------------------------------------------------------------ */

/* 0x13DE8 as read: bytes at 0xF1564 + 199n compared as signed bytes, then the nation words as int16 */
static int model_cmp(int16_t a, int16_t b)
{
    int ta = (int)(int8_t)g_ds[0xF1564u + 199u * (uint32_t)(int32_t)a - DS_BASE];
    int tb = (int)(int8_t)g_ds[0xF1564u + 199u * (uint32_t)(int32_t)b - DS_BASE];

    if (ta != tb)
        return ta > tb ? -1 : 1;
    return a <= b ? -1 : 1;
}

/* compare trace, filled by the model and by the wrapper */
static int16_t g_tr_model[512][2], g_tr_port[512][2];
static int g_ntr_model, g_ntr_port;

/* 0x8C331 for n < 16 on an int array: for gap = 3, then 1 (elements): i = gap, 2gap, ... < n: j = i; while j > 0 and
   cmp(a[j - gap], a[j]) > 0: swap, j -= gap */
static void model_wsort(int16_t *a, int n)
{
    int gap, i, j;
    int16_t t;

    if (n <= 1)
        return;
    for (gap = 3; gap > 0; gap -= 2) {
        for (i = gap; i < n; i += gap) {
            for (j = i; j > 0; j -= gap) {
                if (g_ntr_model < 512) {
                    g_tr_model[g_ntr_model][0] = a[j - gap];
                    g_tr_model[g_ntr_model][1] = a[j];
                }
                g_ntr_model++;
                if (model_cmp(a[j - gap], a[j]) <= 0)
                    break;
                t = a[j - gap];
                a[j - gap] = a[j];
                a[j] = t;
            }
        }
    }
}

/* the order by the rule: tokens (int8) descending, nation ascending, by selection */
static void model_select(int16_t *a, int n)
{
    int i, j, best;
    int16_t t;

    for (i = 0; i < n; i++) {
        best = i;
        for (j = i + 1; j < n; j++) {
            int tb = (int8_t)D8(NAT(a[best], 0x98)), tj = (int8_t)D8(NAT(a[j], 0x98));
            if (tj > tb || (tj == tb && a[j] < a[best]))
                best = j;
        }
        t = a[i];
        a[i] = a[best];
        a[best] = t;
    }
}

/* ---- checks --------------------------------------------------------------------------------------------------- */

static uint8_t *g_before;

/* runs f13D44 on the current data object and checks it against the model; returns the count */
static int check_census(const char *what, int16_t *out)
{
    int16_t ins[10], sel[10];
    int n, k, cnt;
    uint32_t a;

    memcpy(g_before, g_ds, DS_SIZE);
    n = 0;
    for (k = 1; k <= 9; k++)
        if (D8(NAT(k, 0x9A)) != 0)
            ins[n] = sel[n] = (int16_t)k, n++;
    g_ntr_model = 0;
    model_wsort(ins, n);
    model_select(sel, n);

    g_msg_calls = 0;
    f13D44();

    g_tests++;
    if (g_msg_calls != 1)
        fail("%ld message calls (want %ld)", what, g_msg_calls, 1);
    else {
        if (g_msg_args[0] != 100 || g_msg_args[1] != (int)(int16_t)D16(0xF06B0u) || g_msg_args[2] != -200 ||
            g_msg_args[3] != 0 || g_msg_text[0] != 0)
            fail("message args wait %ld nation %ld", what, g_msg_args[0], g_msg_args[1]);
        if (memcmp(g_msg_ds, g_before, DS_SIZE) != 0)
            fail("data changed before the message (%ld %ld)", what, 0, 0);
    }
    cnt = S16(COUNT);
    if (cnt != n)
        fail("count %ld, want %ld", what, cnt, n);
    for (k = 0; k < n && k < 10; k++) {
        int16_t got = S16(ORDER + 2u * (uint32_t)k);
        if (got != ins[k])
            fail("slot %ld: %ld vs insertion model", what, k, got), k = 10;
        else if (got != sel[k])
            fail("slot %ld: %ld vs selection model", what, k, got), k = 10;
        if (out != NULL && k < 10)
            out[k] = got;
    }
    /* nothing else changed: the count, the first n slots; the rest as before */
    for (a = DS_BASE; a < DS_BASE + DS_SIZE; a++) {
        if (a >= COUNT && a < COUNT + 2)
            continue;
        if (a >= ORDER && a < ORDER + 2u * (uint32_t)n)
            continue;
        if (g_ds[a - DS_BASE] != g_before[a - DS_BASE]) {
            fail("byte 0x%lX changed (was 0x%lX)", what, (long)a, g_before[a - DS_BASE]);
            break;
        }
    }
    return n;
}

static uint32_t g_rng = 0x2468ACE1u;
static uint32_t rnd(uint32_t n)
{
    g_rng ^= g_rng << 13;
    g_rng ^= g_rng >> 17;
    g_rng ^= g_rng << 5;
    return n ? g_rng % n : g_rng;
}

static int rec_cmp(const void *a, const void *b)
{
    const uint8_t *pa = a, *pb = b;
    if (g_ntr_port < 512) {
        g_tr_port[g_ntr_port][0] = (int16_t)(pa[0] | pa[1] << 8);
        g_tr_port[g_ntr_port][1] = (int16_t)(pb[0] | pb[1] << 8);
    }
    g_ntr_port++;
    return f13DE8(a, b);
}

static void randomize(void)
{
    int k, mode = (int)rnd(3);

    for (k = 0; k <= 13; k++) {
        if (k >= 1 && k <= 9)
            setD8(NAT(k, 0x9A), (uint8_t)(rnd(4) == 0 ? 0 : 1 + rnd(3)));
        else                                                  /* records 0, 10..13 "in play": the loop is 1..9 only */
            setD8(NAT(k, 0x9A), (uint8_t)rnd(256));
        if (mode == 0)
            setD8(NAT(k, 0x98), (uint8_t)rnd(3));             /* heavy ties */
        else if (mode == 1)
            setD8(NAT(k, 0x98), (uint8_t)rnd(56));            /* the game's range */
        else
            setD8(NAT(k, 0x98), (uint8_t)rnd(256));           /* any byte: int8 order */
    }
    for (k = 0; k < 10; k++)
        setD16(ORDER + 2u * (uint32_t)k, (uint16_t)rnd(0x10000));
    setD16(COUNT, (uint16_t)rnd(0x10000));
    setD16(0xF06B0u, (uint16_t)(1 + rnd(9)));
}

/* ---- main ----------------------------------------------------------------------------------------------------- */

static uint8_t *g_pristine;

static int load_save(const char *path)
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
    return 1;
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:\\work\\advciv_run";
    const char *playdir = argc > 2 ? argv[2] : "C:\\work\\advciv_play";
    char path[3][512];
    const char *label[3] = { "SAVGAMEA.DAT (S(SHIP CONSTRUCTION) R4)", "savgameb.dat (end of R3)",
                             "savgame0.dat (S(MOVEMENT) R4)" };
    uint8_t *r4;
    int s, k, a, b, ta, tb, it, fa = 0, fb = 0, fc = 0, fd = 0;

    if (state_init(gamedir) != 0)
        return 2;
    g_pristine = malloc(DS_SIZE);
    g_before = malloc(DS_SIZE);
    g_msg_ds = malloc(DS_SIZE);
    r4 = malloc(DS_SIZE);
    if (g_pristine == NULL || g_before == NULL || g_msg_ds == NULL || r4 == NULL)
        return 2;
    memcpy(g_pristine, g_ds, DS_SIZE);
    snprintf(path[0], sizeof path[0], "%s\\SAVGAMEA.DAT", playdir);
    snprintf(path[1], sizeof path[1], "%s\\savgameb.dat", playdir);
    snprintf(path[2], sizeof path[2], "%s\\savgame0.dat", gamedir);

    /* A. the comparator */
    {
        uint8_t ea[2], eb[2];
        for (a = 0; a <= 13; a++)
            for (b = 0; b <= 13; b++)
                for (ta = 0; ta < 256; ta++)
                    for (tb = 0; tb < 256; tb++) {
                        int want, got;
                        if (a == b && ta != tb)
                            continue;
                        setD8(NAT(a, 0x98), (uint8_t)ta);
                        setD8(NAT(b, 0x98), (uint8_t)tb);
                        want = (int8_t)ta > (int8_t)tb ? -1 : (int8_t)ta < (int8_t)tb ? 1 : (a <= b ? -1 : 1);
                        ea[0] = (uint8_t)a; ea[1] = 0;
                        eb[0] = (uint8_t)b; eb[1] = 0;
                        got = f13DE8(ea, eb);
                        g_tests++;
                        if (got != want)
                            fail("cmp %ld -> %ld", "A", (long)(a * 100 + b), got), fa++;
                    }
        /* int16 tie test: words -5..20 (records inside the data object), equal token bytes forced */
        for (a = -5; a <= 20; a++)
            for (b = -5; b <= 20; b++) {
                int want, got;
                setD8(0xF1564u + 199u * (uint32_t)a, 7);
                setD8(0xF1564u + 199u * (uint32_t)b, 7);
                ea[0] = (uint8_t)a; ea[1] = (uint8_t)((uint16_t)a >> 8);
                eb[0] = (uint8_t)b; eb[1] = (uint8_t)((uint16_t)b >> 8);
                want = a <= b ? -1 : 1;
                got = f13DE8(ea, eb);
                g_tests++;
                if (got != want)
                    fail("tie %ld -> %ld", "A", (long)(a * 100 + b), got), fa++;
            }
        memcpy(g_ds, g_pristine, DS_SIZE);
    }

    /* B. the real saves */
    for (s = 0; s < 3; s++) {
        int16_t saved[10], port[10];
        int scount, n, same;

        if (!load_save(path[s])) {
            fprintf(stderr, "cannot read %s\n", path[s]);
            g_fail++;
            fb++;
            continue;
        }
        if (s == 0)
            memcpy(r4, g_ds, DS_SIZE);
        scount = S16(COUNT);
        for (k = 0; k < 10; k++)
            saved[k] = S16(ORDER + 2u * (uint32_t)k);
        {
            int before = g_fail;
            n = check_census(label[s], port);
            fb += g_fail - before;
        }
        same = scount == n;
        for (k = 0; k < n && same; k++)
            same = saved[k] == port[k];
        printf("%s\n  tokens on board:", label[s]);
        for (k = 1; k <= 9; k++)
            if (D8(NAT(k, 0x9A)) != 0)
                printf(" %d:%d", k, (int)(int8_t)D8(NAT(k, 0x98)));
        printf("\n  saved order (count %d):", scount);
        for (k = 0; k < scount && k < 10; k++)
            printf(" %d", saved[k]);
        printf("\n  port order  (count %d):", n);
        for (k = 0; k < n; k++)
            printf(" %d", port[k]);
        printf("\n  %s\n", same ? "same as the original's last census" : "differs from the original's last census");
        if (s == 0 && !same) {
            g_fail++;
            fb++;
            fprintf(stderr, "FAIL B: SAVGAMEA.DAT order differs from the saved census\n");
        }
    }

    /* C. random states */
    for (it = 0; it < 20000; it++) {
        int before = g_fail;
        memcpy(g_ds, r4, DS_SIZE);
        randomize();
        check_census("C", NULL);
        fc += g_fail - before;
    }

    /* D. the compare trace */
    for (it = 0; it < 20000; it++) {
        int16_t m[10];
        int n = 0;
        memcpy(g_ds, r4, DS_SIZE);
        randomize();
        n = (int)rnd(10);
        for (k = 0; k < n; k++) {
            m[k] = (int16_t)(1 + rnd(9));                    /* duplicates allowed: equal elements compared too */
            setD16(ORDER + 2u * (uint32_t)k, (uint16_t)m[k]);
        }
        g_ntr_model = g_ntr_port = 0;
        model_wsort(m, n);
        wc_qsort(DP(ORDER), (size_t)n, 2, rec_cmp);
        g_tests++;
        if (g_ntr_model != g_ntr_port) {
            fail("compares %ld vs model %ld", "D", g_ntr_port, g_ntr_model), fd++;
            continue;
        }
        for (k = 0; k < g_ntr_model && k < 512; k++)
            if (g_tr_model[k][0] != g_tr_port[k][0] || g_tr_model[k][1] != g_tr_port[k][1]) {
                fail("compare %ld differs (n %ld)", "D", k, n), fd++;
                break;
            }
        for (k = 0; k < n; k++)
            if (S16(ORDER + 2u * (uint32_t)k) != m[k]) {
                fail("slot %ld differs (n %ld)", "D", k, n), fd++;
                break;
            }
    }

    printf("A comparator: %d failures\nB real saves: %d failures\nC random states: %d failures\n"
           "D compare trace: %d failures\n%d checks, %d failures\n", fa, fb, fc, fd, g_tests, g_fail);
    return g_fail == 0 ? 0 : 1;
}
