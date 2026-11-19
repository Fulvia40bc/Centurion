/* kit_test [GAMEDIR]: unit checks for module KIT (kit.c) and Watcom's qsort (wqsort.c) that need no game run.

   It loads the data object from GAMEDIR/AC_MAIN.EXE as the port does (state_init: le_image.c; GAMEDIR defaults to the
   run copy C:\work\advciv_run, only read). Expectations are written here from the disassembly's semantics,
   independently of kit.c / wqsort.c:
     Q. wc_qsort against a second implementation of 0x8C331 written here on plain index arrays (positions and tags, no
        pointers, no byte widths; read from the listing 0x8C2B3..0x8C769): many random arrays with many ties, element
        sizes 1, 2, 3, 4, 8, 12, 16 at base alignments 0..3 (so all three swap modes), lengths 0..300 (insertion sort,
        middle pivot, median of three, ninther). Each element carries a unique tag; the comparator compares only a key
        per tag. Checked: the exact final permutation, every comparator call in order (both tags), the filler bytes of
        each element, guard bytes around the block. Plus hand vectors worked out on paper (ties in the insertion sort,
        the pivot's mode dependence, the partition running pc below base).
     B. the by-id helpers and the whole-dialog routines on dialog records written into the data object (at 0x1082E0),
        expectations from a plain array of records: every byte of the data object must equal the snapshot taken before
        plus exactly the expected changes; the stubs' call trace (f3A7B0 with which "Button not found", f3B4E0 with
        which record, fg_vbopen / fg_vbclose) must equal the expected trace; plus 0x7844C, 0x3F6E4 / 0x3F758 and
        0x448EC (on a fake surface: each pixel through 0xC2720, in column order).
     C. call traces of 0x79270 (its wait / view / animation modes), 0x795A8 and 0x7983C with the UI stubbed, written by
        hand from the listing, and the "junk" save / restore of the box area on a fake screen.
   Prints each failure, the counts per part, and exits 0 when all pass.

   Link: kit_test.c kit.c wqsort.c state.c le_image.c. Every routine kit.c calls is a stub below. */

#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/state.h"
#include "../src/game.h"
#include "../src/fg.h"
#include "../src/map.h"
#include "../src/mapdata.h"
#include "../src/panel.h"
#include "../src/kit.h"
#include "../src/wqsort.h"

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
#define CHECK(c, ...) check((c) != 0, __LINE__, __VA_ARGS__)

/* ---- random numbers (xorshift64*) ------------------------------------------------------------------------------ */

static uint64_t g_rs = 0x9E3779B97F4A7C15ull;
static uint32_t rnd(void)
{
    g_rs ^= g_rs >> 12;
    g_rs ^= g_rs << 25;
    g_rs ^= g_rs >> 27;
    return (uint32_t)((g_rs * 0x2545F4914F6CDD1Dull) >> 32);
}
static int rndn(int n) { return n > 0 ? (int)(rnd() % (uint32_t)n) : 0; }

/* ---- call trace and stubs ------------------------------------------------------------------------------------- */

static char g_trace[1 << 16];
static int g_last_color = -1000;          /* consecutive identical fg_setcolor calls are traced once */
static int g_trace_px;                    /* 1: trace fg_point */

static void tr(const char *fmt, ...)
{
    size_t len = strlen(g_trace);
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(g_trace + len, sizeof g_trace - len, fmt, ap);
    va_end(ap);
    g_last_color = -1000;
}

/* fake surface: the screen (fg_getpixel / fg_point / the box paste) */
static uint8_t g_surf[480][640];
static int g_color, g_curx, g_cury;
static int g_points[640 * 480][2];
static int g_npoints;

int fg_vbopen(int handle) { tr("vbopen(%d) ", handle); return 0; }
void fg_vbclose(void) { tr("vbclose "); }
int fg_vbcopy(int minx, int maxx, int miny, int maxy, int newx, int newy, int source, int dest)
{
    tr("vbcopy(%d,%d,%d,%d,%d,%d,%d,%d) ", minx, maxx, miny, maxy, newx, newy, source, dest);
    return 0;
}
int fg_vbpaste(int minx, int maxx, int miny, int maxy, int x, int y)
{
    int r, c;
    tr("vbpaste(%d,%d,%d,%d,%d,%d) ", minx, maxx, miny, maxy, x, y);
    for (r = miny; r <= maxy; r++)                 /* the box appears on the screen */
        for (c = minx; c <= maxx; c++)
            if (r >= 0 && r < 480 && c >= 0 && c < 640)
                g_surf[r][c] = (uint8_t)(r * 3 + c);
    return 0;
}
int fg_setcolor(int color)
{
    if (color != g_last_color) {
        tr("setcolor(%d) ", color);
        g_last_color = color;
    }
    g_color = color;
    return 0;
}
int fg_getcolor(void) { return g_color; }
void fg_move(int x, int y) { tr("move(%d,%d) ", x, y); g_curx = x; g_cury = y; }
int fg_getxpos(void) { return g_curx; }
int fg_getypos(void) { return g_cury; }
int fg_point(int x, int y)
{
    if (g_trace_px && g_npoints < 640 * 480) {
        g_points[g_npoints][0] = x;
        g_points[g_npoints][1] = y;
        g_npoints++;
    }
    if (x >= 0 && x < 640 && y >= 0 && y < 480)
        g_surf[y][x] = (uint8_t)g_color;
    return 0;
}
int fg_getpixel(int x, int y) { return x >= 0 && x < 640 && y >= 0 && y < 480 ? g_surf[y][x] : 0; }

static int g_fgf_width = 100;             /* what fgf_width returns */
static int g_3a42c_ret = 30;              /* what f3A42C returns */
int fgf_width(const char *s, int n) { tr("fgfw(%d) ", n); (void)s; return g_fgf_width; }
int f3A42C(int x, int y, int w, uint32_t align, const char *text, int32_t *maxw, int draw)
{
    tr("3A42C(%d,%d,%d,%u,%s,%d) ", x, y, w, (unsigned)align, text, draw);
    *maxw = w;
    return g_3a42c_ret;
}

void f78884(void) { tr("78884 "); }
void f788AC(void) { tr("788AC "); }
void f3C940(uint32_t dlg) { tr("3C940(%X) ", (unsigned)dlg); }
void f3C8C4(uint32_t dlg) { tr("3C8C4(%X) ", (unsigned)dlg); }
void f3C828(uint32_t dlg) { tr("3C828(%X) ", (unsigned)dlg); }
void f3C7A0(uint32_t dlg) { tr("3C7A0(%X) ", (unsigned)dlg); }
void f3C9D0(int x, int y, int w, int h, int flags) { tr("3C9D0(%d,%d,%d,%d,%d) ", x, y, w, h, flags); }
void f3C284(uint32_t dlg, int id, uint32_t label) { tr("3C284(%X,%X,%X) ", (unsigned)dlg, id, (unsigned)label); }
void f3B05C(uint32_t dlg) { tr("3B05C(%X) ", (unsigned)dlg); setD32(0xF8E78u, dlg); }   /* stores the current dialog, as the real one */
void f3BB48(uint32_t dlg, int id, int x) { tr("3BB48(%X,%X,%d) ", (unsigned)dlg, id, x); }
void f3BC64(uint32_t dlg, int id, int y) { tr("3BC64(%X,%X,%d) ", (unsigned)dlg, id, y); }
void f3CD6C(int8_t pic) { tr("3CD6C(%d,bg=%d) ", pic, D8(0xEE168u)); }
void f7876C(void) { tr("7876C "); }
void f3A990(void) { tr("3A990 "); }
void f459E4(int n) { tr("459E4(%d) ", n); }
void f7853C(uint32_t dlg) { tr("7853C(%X) ", (unsigned)dlg); }
void f78664(uint32_t dlg) { tr("78664(%X) ", (unsigned)dlg); }
void f3A9F4(const char *text) { tr("3A9F4(%s,y=%d,h=%d) ", text, S16(0xBB3FCu), S16(0xBB400u)); }
void f3F670(void) { tr("3F670 "); }
void f3D9FC(int n) { tr("3D9FC(%d) ", n); }
void f3D52C(int dx, int dy) { tr("3D52C(%d,%d) ", dx, dy); }
void f3F624(int t, int c) { tr("3F624(%d,%d) ", t, c); }
void f3DA60(int t) { tr("3DA60(%d) ", t); }
void f3CBBC(void) { tr("3CBBC "); }
void f3CCBC(void) { tr("3CCBC "); }
void f78A20(int t) { tr("78A20(%d) ", t); }
void f3A7B0(uint32_t text) { tr("3A7B0(%X) ", (unsigned)text); }
void f3B4E0(uint32_t rec) { tr("3B4E0(%X) ", (unsigned)rec); }

static int g_7ba64[16], g_n7ba64, g_i7ba64;   /* f7BA64's answers, the last repeated */
static int g_78ba4[16], g_n78ba4, g_i78ba4;   /* f78BA4's answers */
int f7BA64(void)
{
    int r = g_n7ba64 == 0 ? 0 : g_7ba64[g_i7ba64 < g_n7ba64 ? g_i7ba64 : g_n7ba64 - 1];
    g_i7ba64++;
    tr("7BA64 ");
    return r;
}
int f78BA4(uint32_t dlg, int map)
{
    int r = g_n78ba4 == 0 ? -1 : g_78ba4[g_i78ba4 < g_n78ba4 ? g_i78ba4 : g_n78ba4 - 1];
    g_i78ba4++;
    tr("78BA4(%X,%d) ", (unsigned)dlg, map);
    if (g_i78ba4 > 50)
        return -1;                        /* a loop that never ends: stop it, the trace shows the excess */
    return r;
}

static jmp_buf g_np_jmp;
_Noreturn void not_ported(uint32_t addr)
{
    tr("NP(%X) ", (unsigned)addr);
    longjmp(g_np_jmp, 1);
}

/* module ANIM (anim.c, own test anim_test.c): recorded here as the not_ported stops they were before 19 Nov */
void f7A0A0(int id) { (void)id; not_ported(0x7A0A0u); }
void f79ADC(const char *n, const char *w) { (void)n; (void)w; not_ported(0x79ADCu); }
void f79C70(const char *g, const char *w) { (void)g; (void)w; not_ported(0x79C70u); }

static void reset_trace(void) { g_trace[0] = 0; g_last_color = -1000; }

/* ---- snapshot helpers ------------------------------------------------------------------------------------------ */

static uint8_t *g_before, *g_want;

static void snap(void)
{
    memcpy(g_before, g_ds, DS_SIZE);
    memcpy(g_want, g_ds, DS_SIZE);
}
static void want8(uint32_t a, uint8_t v) { g_want[a - DS_BASE] = v; }
static int ds_diff(void)               /* number of bytes that differ from the expectation; the first printed */
{
    uint32_t i;
    int n = 0;
    for (i = 0; i < DS_SIZE; i++)
        if (g_ds[i] != g_want[i]) {
            if (n == 0)
                fprintf(stderr, "  byte 0x%X: %02X, expected %02X (was %02X)\n", (unsigned)(i + DS_BASE), g_ds[i],
                        g_want[i], g_before[i]);
            n++;
        }
    return n;
}

/* ================================================================================================================
   Q. Watcom qsort
   ================================================================================================================ */

/* The second implementation (written from 0x8C331..0x8C769 and the helpers 0x8C2B3 / 0x8C2D9, on positions):
   a[] holds tags; the comparator is the sign of key[x] - key[y]; every call is logged as (x, y). pivcopy = 1 in the
   size-4 aligned mode (the pivot is compared as a copy taken before the partition), else the pivot is swapped to the
   first position and compared there. */
typedef struct {
    const int32_t *key;
    int32_t (*log)[2];
    int nlog, cap;
} qmodel;

static int qm_cmp(qmodel *m, int32_t x, int32_t y)
{
    if (m->nlog < m->cap) {
        m->log[m->nlog][0] = x;
        m->log[m->nlog][1] = y;
    }
    m->nlog++;
    return m->key[x] > m->key[y] ? 1 : m->key[x] < m->key[y] ? -1 : 0;
}

static void qm_swap(int32_t *a, int i, int j) { int32_t t = a[i]; a[i] = a[j]; a[j] = t; }

/* position of the median of the elements at positions p, q, r (the three-way test of 0x8C2D9) */
static int qm_med(qmodel *m, const int32_t *a, int p, int q, int r)
{
    if (qm_cmp(m, a[p], a[q]) > 0)
        return qm_cmp(m, a[p], a[r]) <= 0 ? p : (qm_cmp(m, a[q], a[r]) > 0 ? q : r);
    if (qm_cmp(m, a[p], a[r]) >= 0)
        return p;
    return qm_cmp(m, a[q], a[r]) > 0 ? r : q;
}

static void qm_sort(qmodel *m, int32_t *a, int n, int pivcopy)
{
    int st_lo[64], st_n[64], sp = 0;
    int lo = 0, cnt = n;

    for (;;) {
        if (cnt >= 16) {
            int mid = lo + cnt / 2, first = lo, last = lo + cnt - 1;
            int pa, pb, pc, pd, end, s, k, L, R, r;
            int32_t ptag = 0;
            if (cnt > 29) {
                if (cnt > 42) {
                    int d = cnt / 8;
                    first = qm_med(m, a, lo, lo + d, lo + 2 * d);
                    mid = qm_med(m, a, mid - d, mid, mid + d);
                    last = qm_med(m, a, last - 2 * d, last - d, last);
                }
                mid = qm_med(m, a, first, mid, last);
            }
            if (pivcopy)
                ptag = a[mid];
            else
                qm_swap(a, lo, mid);
            pa = pb = lo;
            pc = pd = lo + cnt - 1;
            for (;;) {
                for (; pb <= pc; pb++) {
                    r = qm_cmp(m, a[pb], pivcopy ? ptag : a[lo]);
                    if (r > 0)
                        break;
                    if (r == 0)
                        qm_swap(a, pa++, pb);
                }
                for (; pb <= pc; pc--) {
                    r = qm_cmp(m, a[pc], pivcopy ? ptag : a[lo]);
                    if (r < 0)
                        break;
                    if (r == 0)
                        qm_swap(a, pc, pd--);
                }
                if (pb > pc)
                    break;
                qm_swap(a, pb++, pc--);
            }
            end = lo + cnt;
            s = pa - lo < pb - pa ? pa - lo : pb - pa;
            for (k = 0; k < s; k++)
                qm_swap(a, lo + k, pb - s + k);
            s = pd - pc < end - pd - 1 ? pd - pc : end - pd - 1;
            for (k = 0; k < s; k++)
                qm_swap(a, pb + k, end - s + k);
            L = pb - pa;
            R = pd - pc;
            if (R >= L) {
                st_lo[sp] = end - R;
                st_n[sp++] = R;
                cnt = L;
                continue;
            }
            if (L > 1) {
                st_lo[sp] = lo;
                st_n[sp++] = L;
                lo = end - R;
                cnt = R;
                continue;
            }
        } else if (cnt > 1) {
            int g, i, j;
            for (g = 3; g > 0; g -= 2)
                for (i = lo + g; i < lo + cnt; i += g)
                    for (j = i; j > lo && qm_cmp(m, a[j - g], a[j]) > 0; j -= g)
                        qm_swap(a, j - g, j);
        }
        if (sp == 0)
            return;
        sp--;
        lo = st_lo[sp];
        cnt = st_n[sp];
    }
}

/* the element layout for wc_qsort: the tag in the first bytes (1, 2 or 4 of them), the rest filler */
static size_t g_qw;
static const int32_t *g_qkey;
static int32_t (*g_qlog)[2];
static int g_qnlog, g_qcap;

static int32_t tag_of(const uint8_t *p, size_t w)
{
    if (w == 1)
        return p[0];
    if (w < 4)
        return p[0] | p[1] << 8;
    return (int32_t)((uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24);
}
static size_t tag_bytes(size_t w) { return w == 1 ? 1 : w < 4 ? 2 : 4; }
static uint8_t filler(int32_t tag, size_t k) { return (uint8_t)(tag * 31 + (int32_t)k * 7 + 0x5A); }

static int q_cmp(const void *x, const void *y)
{
    int32_t a = tag_of(x, g_qw), b = tag_of(y, g_qw);
    if (g_qnlog < g_qcap) {
        g_qlog[g_qnlog][0] = a;
        g_qlog[g_qnlog][1] = b;
    }
    g_qnlog++;
    return g_qkey[a] - g_qkey[b];          /* any magnitude: only the sign counts */
}

static _Alignas(16) uint8_t g_qbuf[16 + 300 * 16 + 16];

/* sorts tags 0..n-1 with keys key[] both ways; returns 1 if all agree. *differs_stable = 1 if the result is not the
   stable order */
static int q_case(const int32_t *key, int n, size_t w, int off, int32_t *out, int *differs_stable, int verbose)
{
    static int32_t model_a[300], model_log[1 << 16][2], c_log[1 << 16][2];
    qmodel m;
    uint8_t *base = g_qbuf + 16 + off;
    size_t tb = tag_bytes(w), k;
    int i, ok = 1, mode = ((off | (int)w) & 3) != 0 ? 2 : (w > 4);

    memset(g_qbuf, 0xEE, sizeof g_qbuf);
    for (i = 0; i < n; i++) {
        uint8_t *e = base + (size_t)i * w;
        int32_t t = i;
        for (k = 0; k < tb; k++)
            e[k] = (uint8_t)(t >> (8 * k));
        for (k = tb; k < w; k++)
            e[k] = filler(t, k);
        model_a[i] = i;
    }
    g_qw = w;
    g_qkey = key;
    g_qlog = c_log;
    g_qnlog = 0;
    g_qcap = 1 << 16;
    wc_qsort(base, (size_t)n, w, q_cmp);

    m.key = key;
    m.log = model_log;
    m.nlog = 0;
    m.cap = 1 << 16;
    qm_sort(&m, model_a, n, mode == 0);

    for (i = 0; i < n; i++) {
        uint8_t *e = base + (size_t)i * w;
        int32_t t = tag_of(e, w);
        out[i] = t;
        if (t != model_a[i]) {
            if (ok && verbose)
                fprintf(stderr, "  n %d w %u off %d: position %d holds tag %d, model %d\n", n, (unsigned)w, off, i,
                        (int)t, (int)model_a[i]);
            ok = 0;
        }
        for (k = tb; k < w; k++)
            if (t >= 0 && t < n && e[k] != filler(t, k)) {
                if (ok && verbose)
                    fprintf(stderr, "  n %d w %u: element %d filler broken\n", n, (unsigned)w, i);
                ok = 0;
            }
        if (t < 0 || t >= n)
            ok = 0;                        /* not one of the tags */
        else if (i > 0 && out[i - 1] >= 0 && out[i - 1] < n && key[out[i - 1]] > key[t])
            ok = 0;                        /* not sorted at all */
    }
    for (k = 0; k < 16 + (size_t)off; k++)
        if (g_qbuf[k] != 0xEE)
            ok = 0;
    for (k = 16 + (size_t)off + (size_t)n * w; k < sizeof g_qbuf; k++)
        if (g_qbuf[k] != 0xEE)
            ok = 0;
    if (g_qnlog != m.nlog) {
        if (ok && verbose)
            fprintf(stderr, "  n %d w %u off %d: %d compares, model %d\n", n, (unsigned)w, off, g_qnlog, m.nlog);
        ok = 0;
    } else
        for (i = 0; i < g_qnlog && i < g_qcap; i++)
            if (c_log[i][0] != model_log[i][0] || c_log[i][1] != model_log[i][1]) {
                if (ok && verbose)
                    fprintf(stderr, "  n %d w %u off %d: compare %d is (%d,%d), model (%d,%d)\n", n, (unsigned)w, off,
                            i, (int)c_log[i][0], (int)c_log[i][1], (int)model_log[i][0], (int)model_log[i][1]);
                ok = 0;
                break;
            }
    /* the stable order: by key, then by tag */
    *differs_stable = 0;
    for (i = 1; i < n && ok; i++)
        if (key[out[i - 1]] == key[out[i]] && out[i - 1] > out[i])
            *differs_stable = 1;
    return ok;
}

static void part_q(long *cases, long *bad, long *unstable, long *compares)
{
    static const size_t widths[] = { 1, 2, 3, 4, 4, 4, 8, 12, 16, 2 };
    static const int32_t ranges[] = { 1, 2, 3, 4, 8, 50, 100000 };
    static int32_t key[300], out[300];
    int iter, i, ds;

    /* hand vectors (worked out on paper from the listing) */
    {
        /* H1: n 5 < 16, gap 3 then 1: keys 1 0 1 0 1 -> gap 3 swaps positions 0 and 3; gap 1 finds no pair > 0 */
        static const int32_t k1[5] = { 1, 0, 1, 0, 1 }, e1[5] = { 3, 1, 2, 0, 4 };
        size_t w;
        for (w = 1; w <= 16; w = w * 2) {
            CHECK(q_case(k1, 5, w, 0, out, &ds, 1), "H1 w %u: model disagrees", (unsigned)w);
            CHECK(memcmp(out, e1, sizeof e1) == 0, "H1 w %u: %d %d %d %d %d", (unsigned)w, out[0], out[1], out[2],
                  out[3], out[4]);
            CHECK(ds == 1, "H1: the stable order would be 1 3 0 2 4");
        }
    }
    {
        /* H2: 16 equal keys. Mode 0 (w 4, aligned): all compare equal against the copy, nothing moves (16 compares).
           Mode 2 (w 4 at base+2): the middle (tag 8) is swapped into position 0 first, then nothing moves. */
        int32_t k2[16], e2a[16], e2b[16];
        for (i = 0; i < 16; i++) {
            k2[i] = 7;
            e2a[i] = i;
            e2b[i] = i;
        }
        e2b[0] = 8;
        e2b[8] = 0;
        CHECK(q_case(k2, 16, 4, 0, out, &ds, 1), "H2 mode 0: model disagrees");
        CHECK(memcmp(out, e2a, sizeof e2a) == 0 && g_qnlog == 16, "H2 mode 0: order or %d compares", g_qnlog);
        CHECK(q_case(k2, 16, 4, 2, out, &ds, 1), "H2 mode 2: model disagrees");
        CHECK(memcmp(out, e2b, sizeof e2b) == 0 && g_qnlog == 16, "H2 mode 2: order or %d compares", g_qnlog);
        CHECK(q_case(k2, 16, 16, 0, out, &ds, 1), "H2 mode 1: model disagrees");
        CHECK(memcmp(out, e2b, sizeof e2b) == 0, "H2 mode 1: order");
    }
    {
        /* H3: 16 keys 1 except tag 8 = 0, mode 0: the pivot copy (tag 8) is the strict minimum. pb stops at 0; pc runs
           from 15 down past base (to -1), swapping tag 8 to 15 on the way (pd 14); the right run of 1 element (tag 8)
           goes to position 0 (position 15 gets tag 0); right part (1, 15) pushed; positions 1..15 are all equal. */
        int32_t k3[16];
        static const int32_t e3[16] = { 8, 1, 2, 3, 4, 5, 6, 7, 15, 9, 10, 11, 12, 13, 14, 0 };
        for (i = 0; i < 16; i++)
            k3[i] = i == 8 ? 0 : 1;
        CHECK(q_case(k3, 16, 4, 0, out, &ds, 1), "H3: model disagrees");
        CHECK(memcmp(out, e3, sizeof e3) == 0, "H3: %d %d %d %d .. %d %d", out[0], out[1], out[2], out[3], out[14],
              out[15]);
    }

    /* random cases */
    for (iter = 0; iter < 60000; iter++) {
        size_t w = widths[rndn(10)];
        int off = rndn(4), n, kr = (int)ranges[rndn(7)], pat = rndn(6), sel = rndn(4);
        n = sel == 0 ? rndn(16) : sel == 1 ? 16 + rndn(14) : sel == 2 ? 30 + rndn(13) : 43 + rndn(258);
        if (w == 1 && n > 256)
            n = 256;
        for (i = 0; i < n; i++)
            switch (pat) {
            case 0: case 1: key[i] = rndn(kr); break;              /* random */
            case 2: key[i] = i * kr / (n > 0 ? n : 1); break;      /* sorted with ties */
            case 3: key[i] = (n - i) * kr / (n > 0 ? n : 1); break;/* reverse */
            case 4: key[i] = (i < n / 2 ? i : n - i) % (kr + 1); break;   /* organ pipe */
            default: key[i] = 5; break;                             /* all equal */
            }
        (*cases)++;
        if (!q_case(key, n, w, off, out, &ds, *bad < 5))
            (*bad)++;
        *unstable += ds;
        *compares += g_qnlog;
    }
}

/* ================================================================================================================
   B. by-id helpers on dialog records in the data object
   ================================================================================================================ */

#define DLG 0x1082E0u          /* a dialog area (the title menu's working copy, 50 records) */
#define BACKBUF_H 7            /* [0xF52C8] in these tests */

typedef struct {
    int16_t x, y, w, h;
    uint32_t label;
    int8_t color, noflash;
    uint16_t id;
    int8_t en, style, pressed, font;
} rec_t;

static rec_t g_recs[50];
static int16_t g_count;

static uint32_t rec_addr(int i) { return DLG + 2u + 20u * (uint32_t)i; }

static void put16(uint32_t a, uint16_t v) { setD16(a, v); }
static void write_dialog(void)
{
    int i;
    put16(DLG, (uint16_t)g_count);
    for (i = 0; i < 50; i++) {
        uint32_t r = rec_addr(i);
        put16(r + 0, (uint16_t)g_recs[i].x);
        put16(r + 2, (uint16_t)g_recs[i].y);
        put16(r + 4, (uint16_t)g_recs[i].w);
        put16(r + 6, (uint16_t)g_recs[i].h);
        setD32(r + 8, g_recs[i].label);
        setD8(r + 0x0C, (uint8_t)g_recs[i].color);
        setD8(r + 0x0D, (uint8_t)g_recs[i].noflash);
        put16(r + 0x0E, g_recs[i].id);
        setD8(r + 0x10, (uint8_t)g_recs[i].en);
        setD8(r + 0x11, (uint8_t)g_recs[i].style);
        setD8(r + 0x12, (uint8_t)g_recs[i].pressed);
        setD8(r + 0x13, (uint8_t)g_recs[i].font);
    }
}

static void random_dialog(void)
{
    static const uint16_t ids[] = { 0x2A, 0x41, 0xCA, 0xCB, 0x12, 0xFFFF, 0x8000 };
    int i;
    g_count = (int16_t)(rndn(8) == 0 ? -rndn(5) : rndn(21));
    for (i = 0; i < 50; i++) {
        g_recs[i].x = (int16_t)(rnd() & 0xFFFF);
        g_recs[i].y = (int16_t)(rnd() & 0xFFFF);
        g_recs[i].w = (int16_t)(rnd() & 0xFFFF);
        g_recs[i].h = (int16_t)(rnd() & 0xFFFF);
        g_recs[i].label = rnd();
        g_recs[i].color = (int8_t)rnd();
        g_recs[i].noflash = (int8_t)rnd();
        g_recs[i].id = ids[rndn(7)];
        g_recs[i].en = (int8_t)rnd();
        g_recs[i].style = (int8_t)rnd();
        g_recs[i].pressed = (int8_t)rnd();
        g_recs[i].font = (int8_t)rnd();
    }
    write_dialog();
}

/* the first record of the plain array with this id among the first count, or -1 */
static int find_rec(int qid)
{
    int i;
    for (i = 0; i < g_count; i++)
        if (g_recs[i].id == (uint16_t)qid)
            return i;
    return -1;
}

static void part_b(long *ops, long *bad)
{
    static const uint32_t nf[10] = { 0xB3C14, 0xB3C47, 0xB3C69, 0xB3C7A, 0xB3C9C, 0xB3CBE, 0xB3CF1, 0xB3D57, 0xB3D9B,
                                     0xB3DAC };
    char want[4096];
    int iter, i, k;

    for (k = 0; k < 10; k++)
        CHECK(strcmp(DSTR(nf[k]), "Button not found") == 0, "string 0x%X", (unsigned)nf[k]);
    setD32(0xF52C8u, BACKBUF_H);

    for (iter = 0; iter < 40000; iter++) {
        int fn = rndn(13), qid, found, v = (int)(int8_t)rnd(), ret = 0, wret = 0, ok;
        uint32_t r;
        random_dialog();
        if (rndn(3) == 0 && g_count > 0)
            qid = g_recs[rndn(g_count)].id;   /* an id that is there */
        else
            qid = (int)(rnd() & 0xFF);
        if (rndn(4) == 0)
            qid += 0x10000 * (rndn(2) ? 1 : -1);   /* the high half is ignored: 16-bit compare */
        found = find_rec(qid);
        r = found >= 0 ? rec_addr(found) : 0;
        snap();
        reset_trace();
        want[0] = 0;
        switch (fn) {
        case 0:
            f3B728(DLG, qid);
            if (found >= 0)
                snprintf(want, sizeof want, "vbopen(%d) 3B4E0(%X) vbclose ", BACKBUF_H, (unsigned)r);
            else
                snprintf(want, sizeof want, "3A7B0(%X) ", (unsigned)nf[0]);
            break;
        case 1:
            f3B914(DLG, qid, v);
            if (found >= 0)
                want8(r + 0x12, (uint8_t)v);
            else
                snprintf(want, sizeof want, "3A7B0(%X) ", (unsigned)nf[1]);
            break;
        case 2: case 3: case 4: case 5:
            ret = fn == 2 ? f3BA30(DLG, qid) : fn == 3 ? f3BABC(DLG, qid) : fn == 4 ? f3BBD8(DLG, qid) : f3BCF4(DLG, qid);
            if (found >= 0)
                wret = fn == 2 ? g_recs[found].w : fn == 3 ? g_recs[found].h : fn == 4 ? g_recs[found].x : g_recs[found].y;
            else
                snprintf(want, sizeof want, "3A7B0(%X) ", (unsigned)nf[fn]);
            break;
        case 6:
            f3BEA0(DLG, qid, v);
            if (found >= 0)
                want8(r + 0x0C, (uint8_t)v);
            else
                snprintf(want, sizeof want, "3A7B0(%X) ", (unsigned)nf[6]);
            break;
        case 7:
            f3C1F4(DLG, qid, v);
            if (found >= 0)
                want8(r + 0x0D, (uint8_t)v);
            else
                snprintf(want, sizeof want, "3A7B0(%X) ", (unsigned)nf[7]);
            break;
        case 8: case 9:
            if (fn == 8)
                f3C48C(DLG, qid);
            else
                f3C530(DLG, qid);
            if (found >= 0) {
                want8(r + 0x10, fn == 8 ? 1 : 0);
                snprintf(want, sizeof want, "3B4E0(%X) ", (unsigned)r);   /* no fg_vbopen: the active surface */
            } else
                snprintf(want, sizeof want, "3A7B0(%X) ", (unsigned)nf[fn]);
            break;
        case 10: case 11:
            if (fn == 10)
                f3C6A0(DLG);
            else
                f3C720(DLG);
            snprintf(want, sizeof want, "vbopen(%d) ", BACKBUF_H);
            for (i = 0; i < g_count; i++) {
                want8(rec_addr(i) + 0x10, fn == 10 ? 1 : 0);
                snprintf(want + strlen(want), sizeof want - strlen(want), "3B4E0(%X) ", (unsigned)rec_addr(i));
            }
            strcat(want, "vbclose ");
            break;
        default: {
            /* 0x7844C: players, their nations, the current nation, a base record */
            int players = rndn(10) - 1, base = rndn(5), cur = rndn(10);
            setD32(0xF40ECu, (uint32_t)players);
            for (i = 0; i < 9; i++)
                setD16(0xF06B0u + 2u * (uint32_t)i, (uint16_t)(rndn(3) == 0 ? cur : rndn(12)));
            setD32(0xF40F8u, (uint32_t)cur);
            snap();
            f7844C(DLG, base);
            snprintf(want, sizeof want, "vbopen(%d) ", BACKBUF_H);
            for (i = 0; i < players; i++) {
                int16_t nat = (int16_t)(g_before[0xF06B0u + 2u * (uint32_t)i - DS_BASE] |
                                        g_before[0xF06B0u + 2u * (uint32_t)i + 1 - DS_BASE] << 8);
                want8(rec_addr(base + i) + 0x0C, nat == cur ? 15 : 0);
                snprintf(want + strlen(want), sizeof want - strlen(want), "3B4E0(%X) ", (unsigned)rec_addr(base + i));
            }
            strcat(want, "vbclose ");
            break;
        }
        }
        ok = ds_diff() == 0;
        ok = ok && strcmp(g_trace, want) == 0;
        ok = ok && ret == wret;
        (*ops)++;
        if (!ok) {
            if (*bad < 5)
                fprintf(stderr, "B: fn %d id %X count %d found %d: trace '%s' want '%s', ret %d want %d\n", fn,
                        (unsigned)qid, g_count, found, g_trace, want, ret, wret);
            (*bad)++;
        }
    }

    /* 0x3F6E4 / 0x3F758 */
    for (k = 0; k < 2; k++) {
        uint32_t a;
        for (a = 0xFFB00u; a < 0xFFE00u; a++)
            setD8(a, (uint8_t)(rnd() | 1));
        snap();
        for (a = 0; a < 0xA0; a++)
            want8((k == 0 ? 0xFFC00u : 0xFFCA0u) + a, 0);
        reset_trace();
        if (k == 0)
            f3F6E4();
        else
            f3F758();
        CHECK(ds_diff() == 0 && g_trace[0] == 0, "0x%X: data", k == 0 ? 0x3F6E4 : 0x3F758);
    }

    /* 0x448EC on the fake surface: column by column, each pixel through the table at 0xC2720 */
    for (k = 0; k < 40; k++) {
        static uint8_t old[480][640];
        int x = rndn(600), y = rndn(440), w = rndn(50) - 5, h = rndn(50) - 5, n = 0, okp = 1, c, rr;
        for (rr = 0; rr < 480; rr++)
            for (c = 0; c < 640; c++)
                g_surf[rr][c] = (uint8_t)rnd();
        memcpy(old, g_surf, sizeof old);
        g_trace_px = 1;
        g_npoints = 0;
        reset_trace();
        snap();
        f448EC(x, y, w, h);
        g_trace_px = 0;
        for (c = x; c < x + w; c++)
            for (rr = y; rr < y + h; rr++) {
                if (n >= g_npoints || g_points[n][0] != c || g_points[n][1] != rr)
                    okp = 0;
                n++;
                if (g_surf[rr][c] != g_ds[0xC2720u - DS_BASE + old[rr][c]])
                    okp = 0;
                old[rr][c] = g_surf[rr][c];
            }
        CHECK(okp && n == g_npoints && memcmp(old, g_surf, sizeof old) == 0 && ds_diff() == 0,
              "0x448EC (%d,%d,%d,%d): %d points, %d expected", x, y, w, h, g_npoints, n);
    }
}

/* ================================================================================================================
   C. call traces of 0x79270, 0x795A8, 0x7983C
   ================================================================================================================ */

#define VBC "vbcopy(0,639,0,479,0,479,11,7) "   /* fg_vbcopy with [0xF52CC] = 11, [0xF52C8] = 7 */

static void set_answers(const int *a7, int n7, const int *a78, int n78)
{
    memcpy(g_7ba64, a7, sizeof(int) * (size_t)n7);
    g_n7ba64 = n7;
    g_i7ba64 = 0;
    memcpy(g_78ba4, a78, sizeof(int) * (size_t)n78);
    g_n78ba4 = n78;
    g_i78ba4 = 0;
}

static void run79270(const char *name, int wait, int n, int t, const char *text, int anim, int anims_on,
                     const char *want)
{
    volatile int np = 1;
    setD16(0xBB3FCu, 0x111);
    setD16(0xBB400u, 0x22);
    setD8(0xEE168u, 9);
    setD8(0xF097Cu, (uint8_t)(anims_on ? 0xFF : 0xFB));
    reset_trace();
    if (setjmp(g_np_jmp) == 0) {
        f79270(wait, n, t, text, anim);
        np = 0;
    }
    CHECK(strcmp(g_trace, want) == 0, "0x79270 %s:\n   got  '%s'\n   want '%s'", name, g_trace, want);
    if (np == 0) {
        CHECK(S16(0xBB3FCu) == 0x111 && S16(0xBB400u) == 0x22, "0x79270 %s: area y/h not restored", name);
        CHECK(D8(0xEE168u) == (wait == -560 ? 9 : 3), "0x79270 %s: [0xEE168] = %d", name, D8(0xEE168u));
    }
}

static void part_c(void)
{
    static const int z1[1] = { 0 }, one[1] = { 1 };
    static const int clicks[3] = { -100, -7, -1 }, minus1[1] = { -1 };
    char want[4096];
    const char *fill = "3CD6C(3,bg=9) " VBC "7876C ";
    int r, x, y;

    setD32(0xF52C8u, 7);
    setD32(0xF52CCu, 11);
    setD32(0xF783Cu, 0x13);

    /* -500, home view, three clicks (-100 and -7 loop, -1 ends) */
    set_answers(z1, 1, clicks, 3);
    setD16(0xF40BAu, 0);
    snprintf(want, sizeof want, "%s459E4(3) 7853C(BB800) 3BB48(F596C,2A,265) 3BC64(F596C,2A,428) 3B05C(F596C) "
             "3A9F4(Hello,y=332,h=123) 3F670 3D9FC(3) 3CBBC 7BA64 78BA4(F596C,0) 7BA64 78BA4(F596C,0) 7BA64 "
             "78BA4(F596C,0) 78664(BB800) ", fill);
    run79270("-500 home", -500, 3, -200, "Hello", 0, 0, want);

    /* -500 unattended: 250 ms, or 1000 with [0xF40BA] */
    set_answers(one, 1, minus1, 1);
    snprintf(want, sizeof want, "%s459E4(1) 7853C(BB800) 3BB48(F596C,2A,265) 3BC64(F596C,2A,428) 3B05C(F596C) "
             "3D52C(0,0) 3CBBC 7BA64 78A20(250) 78664(BB800) ", fill);
    run79270("-500 unattended", -500, 1, -250, "", 0, 0, want);
    set_answers(one, 1, minus1, 1);
    setD16(0xF40BAu, 3);
    snprintf(want, sizeof want, "%s459E4(1) 7853C(BB800) 3BB48(F596C,2A,265) 3BC64(F596C,2A,428) 3B05C(F596C) "
             "3D52C(0,0) 3CBBC 7BA64 78A20(1000) 78664(BB800) ", fill);
    run79270("-500 unattended SRD", -500, 1, -250, "", 0, 0, want);
    setD16(0xF40BAu, 0);

    /* a timed message on territory 17 (only it highlighted) */
    snprintf(want, sizeof want, "%s459E4(2) 3C828(F596C) 3B05C(F596C) 3F670 3F624(17,15) 3DA60(17) 3CBBC 78A20(250) "
             "3C7A0(F596C) ", fill);
    run79270("250 t17", 250, 2, 17, "", 0, 0, want);
    /* t 0 counts as a territory */
    snprintf(want, sizeof want, "%s459E4(2) 3C828(F596C) 3B05C(F596C) 3F670 3F624(0,15) 3DA60(0) 3CBBC 78A20(5) "
             "3C7A0(F596C) ", fill);
    run79270("5 t0", 5, 2, 0, "", 0, 0, want);
    /* -560: no redraw, no panel, no view; the wait of -560 still called */
    run79270("-560", -560, 1, -300, "x", 0, 0,
             "3A990 3C828(F596C) 3A9F4(x,y=332,h=123) 3CBBC 78A20(-560) 3C7A0(F596C) ");
    run79270("-560 t5", -560, 1, 5, "", 0, 0, "3A990 3C828(F596C) 3CBBC 78A20(-560) 3C7A0(F596C) ");
    run79270("-560 home", -560, 4, -200, "", 0, 0, "3A990 3C828(F596C) 3CBBC 78A20(-560) 3C7A0(F596C) ");
    /* -550, t -25: highlight 25 added, no clearing; -298 with animations off shows nothing */
    snprintf(want, sizeof want, "%s459E4(4) 3C828(F596C) 3B05C(F596C) 3A9F4(y,y=332,h=123) 3F624(25,15) 3DA60(25) "
             "3CBBC 3C7A0(F596C) ", fill);
    run79270("-550 t-25 zeus off", -550, 4, -25, "y", -298, 0, want);
    /* -300: no status panel */
    snprintf(want, sizeof want, "%s3C828(F596C) 3B05C(F596C) 3D52C(0,0) 3CBBC 78A20(100) 3C7A0(F596C) ", fill);
    run79270("100 -300", 100, 6, -300, "", 0, 0, want);
    /* animations: 1 and 2 with animations off do nothing; on, anything but -298/-299 is the calamity */
    snprintf(want, sizeof want, "%s459E4(6) 3C828(F596C) 3B05C(F596C) 3D52C(0,0) 3CBBC 78A20(100) 3C7A0(F596C) ",
             fill);
    run79270("anim 2 off", 100, 6, -250, "", 2, 0, want);
    run79270("anim 7 off", 100, 6, -250, "", 7, 0, want);
    snprintf(want, sizeof want, "%s459E4(6) 3C828(F596C) 3B05C(F596C) 3D52C(0,0) 3CBBC NP(7A0A0) ", fill);
    run79270("anim 1 on", 100, 6, -250, "", 1, 1, want);
    snprintf(want, sizeof want, "%s459E4(6) 3C828(F596C) 3B05C(F596C) 3D52C(0,0) 3CBBC NP(79ADC) ", fill);
    run79270("zeus on", 100, 6, -250, "", -298, 1, want);
    snprintf(want, sizeof want, "%s459E4(6) 3C828(F596C) 3B05C(F596C) 3D52C(0,0) 3CBBC NP(79C70) ", fill);
    run79270("brpc on", 100, 6, -250, "", -299, 1, want);
    run79270("brpc off", 100, 6, -250, "", -299, 0, want);

    /* 0x795A8: text width 100 -> W 180; f3A42C 30 -> H 95; X 230, Y 192 ((480 - 95) / 2 = 192), buttons at 240 */
    {
        int i, same = 1;
        for (y = 0; y < 480; y++)
            for (x = 0; x < 640; x++)
                g_surf[y][x] = 0x21;
        setD32(0xF8E78u, 0x1234);
        set_answers(z1, 1, (const int[]){ -100, -5, -0x62 }, 3);
        g_fgf_width = 100;
        g_3a42c_ret = 30;
        g_color = 0;
        reset_trace();
        r = f795A8("Are you sure?");
        CHECK(r == -0x62, "0x795A8 returns %d", r);
        CHECK(D32(0xF8E78u) == 0x1234, "0x795A8: [0xF8E78] = %X", (unsigned)D32(0xF8E78u));
        snprintf(want, sizeof want, "fgfw(13) 3A42C(0,0,160,0,Are you sure?,0) 78884 788AC 3C940(1234) vbopen(7) "
                 "3C9D0(230,192,180,95,3) setcolor(5) 3A42C(240,202,160,2,Are you sure?,1) 3BB48(BBFD8,CA,240) "
                 "3BC64(BBFD8,CA,258) 3C284(BBFD8,CA,B9582) 3BB48(BBFD8,CB,240) 3BC64(BBFD8,CB,238) "
                 "3C284(BBFD8,CB,B9586) 3B05C(BBFD8) vbclose 78884 vbpaste(230,409,192,286,230,286) 788AC "
                 "78BA4(BBFD8,0) 78BA4(BBFD8,0) 78BA4(BBFD8,0) 3C8C4(1234) 78884 move(230,192) setcolor(33) "
                 "setcolor(5) 3CCBC 788AC ");
        CHECK(strcmp(g_trace, want) == 0, "0x795A8:\n   got  '%s'\n   want '%s'", g_trace, want);
        CHECK(strcmp(DSTR(0xB9582u), "YES") == 0 && strcmp(DSTR(0xB9586u), "NO") == 0, "YES / NO strings");
        /* the box area back as it was (random screen, the paste scribbles over the box) */
        for (y = 0; y < 480; y++)
            for (x = 0; x < 640; x++)
                g_surf[y][x] = (uint8_t)rnd();
        {
            static uint8_t keep[480][640];
            memcpy(keep, g_surf, sizeof keep);
            set_answers(z1, 1, (const int[]){ -0x61 }, 1);
            setD32(0xF8E78u, 0);
            reset_trace();
            g_fgf_width = 400;            /* W 410 -> 350 (W - 20 > 350) */
            g_3a42c_ret = 61;             /* H 126, X 145, Y 177, bx 145 + 95 */
            r = f795A8("long");
            CHECK(r == -0x61, "0x795A8 returns %d", r);
            CHECK(strstr(g_trace, "3C940") == NULL && strstr(g_trace, "3C8C4") == NULL, "0x795A8: no current dialog");
            CHECK(strstr(g_trace, "3C9D0(145,177,350,126,3)") != NULL && strstr(g_trace, "3BB48(BBFD8,CA,240)") != NULL
                  && strstr(g_trace, "3BC64(BBFD8,CA,274)") != NULL && strstr(g_trace, "3BC64(BBFD8,CB,254)") != NULL,
                  "0x795A8 W 350: '%s'", g_trace);
            for (i = 0; i < 480 * 640; i++)
                if (((uint8_t *)g_surf)[i] != ((uint8_t *)keep)[i])
                    same = 0;
            CHECK(same, "0x795A8: the screen under the box not restored");
        }
        /* W 351..370 stay: width 355 -> tw 365 -> W 365, X 137, bx 137 + 102 */
        set_answers(z1, 1, (const int[]){ -0x61 }, 1);
        g_fgf_width = 355;
        g_3a42c_ret = 61;
        reset_trace();
        f795A8("w");
        CHECK(strstr(g_trace, "3C9D0(137,177,365,126,3)") != NULL && strstr(g_trace, "3BB48(BBFD8,CB,239)") != NULL,
              "0x795A8 W 365: '%s'", g_trace);
    }

    /* 0x7983C: no f78884 around the save; CONTINUE at x + (W - 110) / 2, H = text + 0x2D */
    setD32(0xF8E78u, 0x5678);
    set_answers((const int[]){ 0, 0 }, 2, (const int[]){ -100, -3, -1 }, 3);
    g_fgf_width = 100;
    g_3a42c_ret = 30;                     /* H 75, Y 202, X 230, bx 265 */
    for (y = 0; y < 480; y++)
        for (x = 0; x < 640; x++)
            g_surf[y][x] = 0x21;
    g_color = 0;
    reset_trace();
    f7983C("Done.");
    snprintf(want, sizeof want, "fgfw(5) 3A42C(0,0,160,0,Done.,0) 3C940(5678) vbopen(7) 3C9D0(230,202,180,75,3) "
             "setcolor(5) 3A42C(240,212,160,2,Done.,1) 3BB48(BBBEC,2A,265) 3BC64(BBBEC,2A,248) 3C284(BBBEC,2A,B9593) "
             "3B05C(BBBEC) vbclose 78884 vbpaste(230,409,202,276,230,276) 788AC 7BA64 78BA4(BBBEC,0) 7BA64 "
             "78BA4(BBBEC,0) 7BA64 78BA4(BBBEC,0) 3C8C4(5678) 78884 move(230,202) setcolor(33) setcolor(5) 3CCBC "
             "788AC ");
    CHECK(strcmp(g_trace, want) == 0, "0x7983C:\n   got  '%s'\n   want '%s'", g_trace, want);
    CHECK(D32(0xF8E78u) == 0x5678, "0x7983C: [0xF8E78] = %X", (unsigned)D32(0xF8E78u));
    CHECK(strcmp(DSTR(0xB9593u), "CONTINUE") == 0, "CONTINUE string");
    /* unattended, no current dialog (the port guard) */
    setD32(0xF8E78u, 0);
    set_answers(one, 1, minus1, 1);
    setD16(0xF40BAu, 2);
    reset_trace();
    f7983C("Done.");
    CHECK(strstr(g_trace, "7BA64 78A20(1000) 78884") != NULL && strstr(g_trace, "3C940") == NULL &&
          strstr(g_trace, "3C8C4") == NULL && strstr(g_trace, "78BA4") == NULL, "0x7983C unattended: '%s'", g_trace);
    setD16(0xF40BAu, 0);

    /* (review) 0x7983C: the box area back as it was on a random screen; mixed attended / unattended rounds:
       f7BA64 0 -> click -100 (loops), then 1 -> 250 ms and -1 ends */
    {
        static uint8_t keep[480][640];
        int i, same = 1;
        for (y = 0; y < 480; y++)
            for (x = 0; x < 640; x++)
                g_surf[y][x] = (uint8_t)rnd();
        memcpy(keep, g_surf, sizeof keep);
        setD32(0xF8E78u, 0x5678);
        set_answers((const int[]){ 0, 1 }, 2, (const int[]){ -100 }, 1);
        g_fgf_width = 100;
        g_3a42c_ret = 30;
        reset_trace();
        f7983C("Done.");
        CHECK(strstr(g_trace, "788AC 7BA64 78BA4(BBBEC,0) 7BA64 78A20(250) 3C8C4(5678) ") != NULL,
              "0x7983C mixed rounds: '%s'", g_trace);
        for (i = 0; i < 480 * 640; i++)
            if (((uint8_t *)g_surf)[i] != ((uint8_t *)keep)[i])
                same = 0;
        CHECK(same, "0x7983C: the screen under the box not restored");
    }
    /* (review) H > 480: Y = (480 - H) / 2 rounds toward zero (sar after adding the sign bit, 0x79638 / 0x798E2).
       0x795A8: H = 500 + 0x41 = 565, Y = -85 / 2 = -42 (floor would be -43); YES at Y + H - 0x1D = 494, NO at 474.
       0x7983C: H = 500 + 0x2D = 545, Y = -65 / 2 = -32; CONTINUE at 484. */
    setD32(0xF8E78u, 0);
    g_fgf_width = 100;
    g_3a42c_ret = 500;
    set_answers(z1, 1, (const int[]){ -0x62 }, 1);
    reset_trace();
    r = f795A8("tall");
    CHECK(r == -0x62 && strstr(g_trace, "3C9D0(230,-42,180,565,3)") != NULL &&
          strstr(g_trace, "3BC64(BBFD8,CA,494)") != NULL && strstr(g_trace, "3BC64(BBFD8,CB,474)") != NULL &&
          strstr(g_trace, "vbpaste(230,409,-42,522,230,522)") != NULL && strstr(g_trace, "move(230,-42)") != NULL,
          "0x795A8 H 565: '%s'", g_trace);
    set_answers(z1, 1, minus1, 1);
    reset_trace();
    f7983C("tall");
    CHECK(strstr(g_trace, "3C9D0(230,-32,180,545,3)") != NULL && strstr(g_trace, "3BC64(BBBEC,2A,484)") != NULL &&
          strstr(g_trace, "vbpaste(230,409,-32,512,230,512)") != NULL, "0x7983C H 545: '%s'", g_trace);
    g_3a42c_ret = 30;

    /* (review) 0x79270 -500 mixed rounds: attended -100 loops, then unattended (250 ms) ends */
    set_answers((const int[]){ 0, 1 }, 2, (const int[]){ -100 }, 1);
    snprintf(want, sizeof want, "%s459E4(1) 7853C(BB800) 3BB48(F596C,2A,265) 3BC64(F596C,2A,428) 3B05C(F596C) "
             "3D52C(0,0) 3CBBC 7BA64 78BA4(F596C,0) 7BA64 78A20(250) 78664(BB800) ", fill);
    run79270("-500 mixed", -500, 1, -250, "", 0, 0, want);
    /* (review) -560 with t -250: no scroll; -560 with anim -299 and animations off still reaches 0x79C70; -560 with
       anim 7 and animations on reaches 0x7A0A0 (the animation test does not look at wait) */
    run79270("-560 t-250", -560, 1, -250, "", 0, 0, "3A990 3C828(F596C) 3CBBC 78A20(-560) 3C7A0(F596C) ");
    run79270("-560 brpc off", -560, 1, -250, "", -299, 0, "3A990 3C828(F596C) 3CBBC NP(79C70) ");
    run79270("-560 anim 7 on", -560, 1, -250, "", 7, 1, "3A990 3C828(F596C) 3CBBC NP(7A0A0) ");
    /* (review) -550 with t -200 (home view) and anim 1 with animations off: nothing shown, no wait */
    snprintf(want, sizeof want, "%s459E4(5) 3C828(F596C) 3B05C(F596C) 3F670 3D9FC(5) 3CBBC 3C7A0(F596C) ", fill);
    run79270("-550 home anim 1 off", -550, 5, -200, "", 1, 0, want);
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:/work/advciv_run";
    long qcases = 0, qbad = 0, qunstable = 0, qcmp = 0, bops = 0, bbad = 0;
    int t0, f0, t1, f1;

    if (state_init(gamedir) != 0)
        return 1;
    g_before = malloc(DS_SIZE);
    g_want = malloc(DS_SIZE);
    if (g_before == NULL || g_want == NULL)
        return 1;

    part_q(&qcases, &qbad, &qunstable, &qcmp);
    printf("Q. qsort: hand vectors %d of %d pass; %ld random cases, %ld disagree with the model "
           "(%ld compares checked one by one; %ld cases end in a non-stable tie order)\n",
           g_tests - g_fail, g_tests, qcases, qbad, qcmp, qunstable);
    t0 = g_tests;
    f0 = g_fail;

    part_b(&bops, &bbad);
    printf("B. by-id helpers: %ld random operations, %ld disagree; fixed checks %d of %d pass\n", bops, bbad,
           (g_tests - t0) - (g_fail - f0), g_tests - t0);
    t1 = g_tests;
    f1 = g_fail;

    part_c();
    printf("C. traces of 0x79270 / 0x795A8 / 0x7983C: %d of %d pass\n", (g_tests - t1) - (g_fail - f1),
           g_tests - t1);

    return g_fail == 0 && qbad == 0 && bbad == 0 ? 0 : 1;
}
