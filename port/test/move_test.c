/* move_test [GAMEDIR]: unit vectors for module MOVE (move.c) that need no game run.

   It loads the data object from GAMEDIR/AC_MAIN.EXE as the port does (state_init: le_image.c; GAMEDIR defaults to the
   run copy C:\work\advciv_run, only read), clears the area records (0xE31B8, 161 x 69) and the nation records
   (0xF14CC, 14 x 199), writes synthetic contents and applies one routine. Expectations are written here from the
   disassembly's semantics, independently of move.c:
     A. hand vectors for the eight primitives: every byte the routine should change is listed with its value, worked
        out by hand (quirks included: byte wrap, signed counts, the int16 compare, the high byte f580CC keeps, the
        entry f581C4 skips after a removal), and the whole data object must equal the snapshot taken before plus
        exactly those bytes;
     B. a random differential run: a second implementation of the eight routines on plain byte arrays (written from
        the listing, not from move.c) and move.c apply the same random operations to the same random records; the
        area and nation tables and the return values must agree after every operation, and nothing else may change;
     C. data checks of the dispatcher cases with the UI routines stubbed: the stubs record the calls they receive
        (name and arguments, in order), the data the case changes is checked byte for byte as in A, the call trace
        against the order of the listing.
   Prints each failure, the counts per part, and exits 0 when all pass.

   Link: move_test.c move.c mapdata.c load.c state.c le_image.c. mapdata.c gives the real setters and alias ids
   (f3F698, f3F70C, f3F670, f3F624, f3E89C, f3E8FC, f3DA60); load.c the heap handles for the PBEM log. The UI routines
   move.c and those files call are stubs below. */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/state.h"
#include "../src/game.h"
#include "../src/fg.h"
#include "../src/map.h"
#include "../src/mapdata.h"
#include "../src/shippanel.h"
#include "../src/move.h"

/* ---- call trace and stubs ------------------------------------------------------------------------------------- */

static char g_trace[4096];
static int g_pressed;                     /* what the f3B9A4 stub returns */

static void tr(const char *fmt, ...)
{
    size_t len = strlen(g_trace);
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(g_trace + len, sizeof g_trace - len, fmt, ap);
    va_end(ap);
}

int fg_vbopen(int handle) { tr("vbopen(%d) ", handle); return 0; }
void fg_vbclose(void) { tr("vbclose "); }
int f3B9A4(uint32_t dlg, int id) { tr("3B9A4(%X,%X) ", (unsigned)dlg, id); return g_pressed; }
void f3B7CC(uint32_t dlg, int id) { tr("3B7CC(%X,%X) ", (unsigned)dlg, id); }
void f3B870(uint32_t dlg, int id) { tr("3B870(%X,%X) ", (unsigned)dlg, id); }
void f3BFBC(uint32_t dlg, int id, int en) { tr("3BFBC(%X,%X,%d) ", (unsigned)dlg, id, en); }
void f3C3D0(uint32_t dlg, int id) { tr("3C3D0(%X,%X) ", (unsigned)dlg, id); }
void f3CBBC(void) { tr("3CBBC "); }
void f3D52C(int dx, int dy) { tr("3D52C(%d,%d) ", dx, dy); }
void f787F8(int t) { tr("787F8(%d) ", t); }
void f426D0(int i) { tr("426D0(%d) ", i); }
void f572CC(int t) { tr("572CC(%d) ", t); }
void f3A780(const char *msg) { fprintf(stderr, "move_test: %s", msg); }
_Noreturn void f44CAC(void) { fprintf(stderr, "move_test: quit: %s\n", DSTR(0xF5500u)); exit(2); }

/* ---- addresses (from move.h's layout, written out here) ------------------------------------------------------- */

#define AREA_BASE 0xE31B8u
#define AREA_LEN  (161u * 69u)
#define NAT_BASE  0xF14CCu
#define NAT_LEN   (14u * 199u)
#define AR(t, off) (AREA_BASE + 69u * (uint32_t)((t) + 1) + (uint32_t)(off))   /* field off of territory t's record */
#define NA(n, off) (NAT_BASE + 199u * (uint32_t)(n) + (uint32_t)(off))         /* field off of nation n's record */

static uint8_t *g_before, *g_want;
static struct { uint32_t a; uint8_t v; } g_exp[512];
static int g_nexp;
static int g_tests, g_fail;

static void clear_records(void)
{
    memset(DP(AREA_BASE), 0, AREA_LEN);
    memset(DP(NAT_BASE), 0, NAT_LEN);
}

static void snap(void)
{
    memcpy(g_before, g_ds, DS_SIZE);
    g_nexp = 0;
    g_trace[0] = 0;
}

static void E8(uint32_t a, uint8_t v)
{
    g_exp[g_nexp].a = a;
    g_exp[g_nexp].v = v;
    g_nexp++;
}

static void E16(uint32_t a, uint16_t v)
{
    E8(a, (uint8_t)v);
    E8(a + 1, (uint8_t)(v >> 8));
}

static void E32(uint32_t a, uint32_t v)
{
    E16(a, (uint16_t)v);
    E16(a + 2, (uint16_t)(v >> 16));
}

/* the data object must equal the snapshot plus the expected bytes; ret/want_ret and the trace when given */
static void verify(const char *name, int ret, int want_ret, const char *want_trace)
{
    uint32_t i;
    int k, diffs = 0;

    memcpy(g_want, g_before, DS_SIZE);
    for (k = 0; k < g_nexp; k++)
        g_want[g_exp[k].a - DS_BASE] = g_exp[k].v;
    for (i = 0; i < DS_SIZE; i++)
        if (g_ds[i] != g_want[i]) {
            if (diffs < 6)
                printf("  %s: byte 0x%05X is %02X, expected %02X (was %02X)\n", name, (unsigned)(i + DS_BASE),
                       g_ds[i], g_want[i], g_before[i]);
            diffs++;
        }
    g_tests++;
    if (diffs != 0 || ret != want_ret || (want_trace != NULL && strcmp(g_trace, want_trace) != 0)) {
        g_fail++;
        printf("FAIL %s: %d bytes differ, returned %d (expected %d)\n", name, diffs, ret, want_ret);
        if (want_trace != NULL && strcmp(g_trace, want_trace) != 0)
            printf("  trace    %s\n  expected %s\n", g_trace, want_trace);
    }
}

static void set8(uint32_t a, uint8_t v) { setD8(a, v); }
static void set16(uint32_t a, uint16_t v) { setD16(a, v); }

/* ==== A. hand vectors ========================================================================================== */

static void part_a(void)
{
    /* -- 0x57E28: tokens onto a territory -- */
    clear_records();
    snap();
    f57E28(5, 3, 2);                    /* empty: count 1, marker 3, tokens 2, list [6] */
    E8(AR(5, 1), 1); E8(AR(5, 1 + 3), 3); E8(AR(5, 0x0E + 3), 2); E8(NA(3, 0x60), 1); E8(NA(3, 0x61), 6);
    verify("57E28 empty territory", 0, 0, NULL);

    snap();
    f57E28(5, 3, 1);                    /* marker already 3: count stays, tokens 3, list unchanged */
    E8(AR(5, 0x0E + 3), 3);
    verify("57E28 again", 0, 0, NULL);

    snap();
    f57E28(5, 3, 0);
    f57E28(5, 3, -4);
    verify("57E28 k <= 0", 0, 0, NULL);

    snap();
    f57E28(5, 7, 1);                    /* a second nation: count 2 */
    E8(AR(5, 1), 2); E8(AR(5, 1 + 7), 7); E8(AR(5, 0x0E + 7), 1); E8(NA(7, 0x60), 1); E8(NA(7, 0x61), 6);
    verify("57E28 second nation", 0, 0, NULL);

    clear_records();
    set8(AR(9, 1), 1); set8(AR(9, 1 + 2), 2); set8(AR(9, 0x0E + 2), 0xFF);
    set8(NA(2, 0x60), 4); set8(NA(2, 0x61), 3); set8(NA(2, 0x62), 20); set8(NA(2, 0x63), 10); set8(NA(2, 0x64), 1);
    snap();
    f57E28(9, 2, 2);                    /* tokens wrap 0xFF + 2 = 1; 10 found at index 2: no append */
    E8(AR(9, 0x0E + 2), 0x01);
    verify("57E28 byte wrap, found in list", 0, 0, NULL);

    clear_records();
    set8(NA(4, 0x60), 0xFF);            /* count -1: the loop does not run; entry -1 is +0x60 itself */
    snap();
    f57E28(8, 4, 1);                    /* +0x60 = 9 (t + 1), then + 1 = 10 */
    E8(AR(8, 1), 1); E8(AR(8, 1 + 4), 4); E8(AR(8, 0x0E + 4), 1); E8(NA(4, 0x60), 10);
    verify("57E28 negative list count", 0, 0, NULL);

    clear_records();
    set8(AR(12, 1 + 5), 0x85);          /* marker read signed: -123 != 5 -> count + 1 and marker 5 */
    set8(AR(12, 1), 3);
    snap();
    f57E28(12, 5, 3);
    E8(AR(12, 1), 4); E8(AR(12, 1 + 5), 5); E8(AR(12, 0x0E + 5), 3); E8(NA(5, 0x60), 1); E8(NA(5, 0x61), 13);
    verify("57E28 foreign marker byte", 0, 0, NULL);

    /* -- 0x581C4: leave a territory -- */
    clear_records();
    set8(AR(5, 1), 2); set8(AR(5, 0x0E + 3), 0);    /* marker 0 */
    set8(NA(3, 0x60), 1); set8(NA(3, 0x61), 6);
    snap();
    f581C4(5, 3);
    verify("581C4 marker 0", 0, 0, NULL);

    set8(AR(5, 1 + 3), 3); set8(AR(5, 0x0E + 3), 1);
    snap();
    f581C4(5, 3);
    verify("581C4 tokens left", 0, 0, NULL);

    set8(AR(5, 0x0E + 3), 0x80);
    snap();
    f581C4(5, 3);
    verify("581C4 negative tokens", 0, 0, NULL);

    clear_records();
    set8(AR(5, 1), 2); set8(AR(5, 1 + 3), 3);
    set8(NA(3, 0x60), 3); set8(NA(3, 0x61), 3); set8(NA(3, 0x62), 6); set8(NA(3, 0x63), 9);
    snap();
    f581C4(5, 3);                       /* [3,6,9] -> [3,9,9], count 2 */
    E8(AR(5, 1), 1); E8(AR(5, 1 + 3), 0); E8(NA(3, 0x60), 2); E8(NA(3, 0x62), 9);
    verify("581C4 removes from the middle", 0, 0, NULL);

    clear_records();
    set8(AR(5, 1), 1); set8(AR(5, 1 + 3), 3);
    set8(NA(3, 0x60), 3); set8(NA(3, 0x61), 6); set8(NA(3, 0x62), 6); set8(NA(3, 0x63), 9);
    snap();
    f581C4(5, 3);                       /* [6,6,9] -> [6,9,9], count 2: the second 6 is skipped */
    E8(AR(5, 1), 0); E8(AR(5, 1 + 3), 0); E8(NA(3, 0x60), 2); E8(NA(3, 0x61), 6); E8(NA(3, 0x62), 9);
    verify("581C4 skips the entry after a removal", 0, 0, NULL);

    clear_records();
    set8(AR(0, 1 + 1), 1);              /* count byte 0: wraps to 0xFF; last entry removed */
    set8(NA(1, 0x60), 2); set8(NA(1, 0x61), 7); set8(NA(1, 0x62), 1);
    snap();
    f581C4(0, 1);
    E8(AR(0, 1), 0xFF); E8(AR(0, 1 + 1), 0); E8(NA(1, 0x60), 1);
    verify("581C4 count wrap, last entry", 0, 0, NULL);

    /* -- 0x57CE8: tokens off a territory -- */
    clear_records();
    set8(AR(20, 1), 1); set8(AR(20, 1 + 6), 6); set8(AR(20, 0x0E + 6), 5); set8(AR(20, 0x24 + 6), 4);
    set8(NA(6, 0x60), 1); set8(NA(6, 0x61), 21);
    snap();
    f57CE8(20, 6, 1, 2);
    E8(AR(20, 0x0E + 6), 3); E8(AR(20, 0x24 + 6), 2);
    verify("57CE8 flag 1", 0, 0, NULL);

    snap();
    f57CE8(20, 6, 0, 1);
    E8(AR(20, 0x0E + 6), 2);
    verify("57CE8 flag 0", 0, 0, NULL);

    snap();
    f57CE8(20, 6, 2, 1);
    E8(AR(20, 0x0E + 6), 1);
    verify("57CE8 flag 2 (only 1 counts)", 0, 0, NULL);

    snap();
    f57CE8(20, 6, 1, 0x102);            /* the byte of k: 2 */
    E8(AR(20, 0x0E + 6), 0xFF); E8(AR(20, 0x24 + 6), 0);
    verify("57CE8 byte of k, wrap below 0", 0, 0, NULL);

    clear_records();
    set8(AR(20, 1), 1); set8(AR(20, 1 + 6), 6);     /* tokens 0 but k = 0: f581C4 not reached */
    set8(NA(6, 0x60), 1); set8(NA(6, 0x61), 21);
    snap();
    f57CE8(20, 6, 1, 0);
    verify("57CE8 k = 0", 0, 0, NULL);

    clear_records();
    set8(AR(20, 1), 2); set8(AR(20, 1 + 6), 6); set8(AR(20, 0x0E + 6), 2); set8(AR(20, 0x24 + 6), 2);
    set8(NA(6, 0x60), 2); set8(NA(6, 0x61), 21); set8(NA(6, 0x62), 40);
    snap();
    f57CE8(20, 6, 1, 2);                /* all gone: the nation leaves (count 1, marker 0, list [40]) */
    E8(AR(20, 0x0E + 6), 0); E8(AR(20, 0x24 + 6), 0); E8(AR(20, 1), 1); E8(AR(20, 1 + 6), 0);
    E8(NA(6, 0x60), 1); E8(NA(6, 0x61), 40);
    verify("57CE8 last tokens", 0, 0, NULL);

    /* -- 0x580CC: the selected ship from a to b -- */
    clear_records();
    set8(NA(4, 0x40), 2); set16(NA(4, 0x45 + 2), 0x120B);     /* ship 1 at territory 10, high byte 0x12 */
    set8(AR(10, 0x2D + 4), 1); set8(AR(10, 0x1B + 4), 1);
    set8(AR(10, 1), 1); set8(AR(10, 1 + 4), 4);              /* marker but no tokens: f581C4 drops it */
    set8(NA(4, 0x60), 1); set8(NA(4, 0x61), 11);
    snap();
    f580CC(10, 11, 4);
    E8(AR(10, 0x1B + 4), 0); E8(AR(10, 1), 0); E8(AR(10, 1 + 4), 0); E8(NA(4, 0x60), 0);
    E16(NA(4, 0x45 + 2), 0x120C); E8(AR(11, 0x2D + 4), 1); E8(AR(11, 0x1B + 4), 1);
    verify("580CC high byte kept, nation dropped", 0, 0, NULL);

    clear_records();
    set8(NA(4, 0x40), 1); set16(NA(4, 0x45), 0x0003);
    set8(AR(2, 0x1B + 4), 0);                                /* count 0: wraps */
    set8(AR(2, 1), 1); set8(AR(2, 1 + 4), 4); set8(AR(2, 0x0E + 4), 1);
    set8(AR(30, 0x2D + 4), 3); set8(AR(30, 0x1B + 4), 0xFF);
    snap();
    f580CC(2, 30, 4);                   /* tokens stay: no drop; b's selection 3 -> 0; b's ships 0xFF + 1 = 0 */
    E8(AR(2, 0x1B + 4), 0xFF); E16(NA(4, 0x45), 0x001F); E8(AR(30, 0x2D + 4), 0); E8(AR(30, 0x1B + 4), 0);
    verify("580CC wraps, selection copied", 0, 0, NULL);

    /* -- 0x576F8: the selection onto a ship there -- */
    clear_records();
    setD32(0xF40F8u, 2);
    set16(NA(2, 0x45), 21);             /* ship 0 at 20 */
    set8(AR(20, 0x2D + 2), 2);
    snap();
    f576F8(20);                         /* 2 no, 3 no, 0 yes */
    E8(AR(20, 0x2D + 2), 0);
    verify("576F8 finds ship 0", 0, 0, NULL);

    clear_records();
    set8(AR(20, 0x2D + 2), 1);
    snap();
    f576F8(20);                         /* none: five steps 1->2->3->0->1->2 */
    E8(AR(20, 0x2D + 2), 2);
    verify("576F8 none, five steps", 0, 0, NULL);

    clear_records();
    set16(NA(2, 0x45 + 2), 0x0115);     /* territory 20 + 1 with high byte 1: the int16 compare misses it */
    set8(AR(20, 0x2D + 2), 1);
    snap();
    f576F8(20);
    E8(AR(20, 0x2D + 2), 2);
    verify("576F8 int16 compare", 0, 0, NULL);

    clear_records();
    set16(NA(2, 0x45), 21);
    set8(AR(20, 0x2D + 2), 0xFF);       /* 0xFF + 1 = 0 (byte), 0 % 4 = 0, then ship 0 matches */
    snap();
    f576F8(20);
    E8(AR(20, 0x2D + 2), 0);
    verify("576F8 byte wrap", 0, 0, NULL);

    clear_records();
    setD32(0xF40F8u, 9);
    set16(NA(9, 0x45 + 6), 0x0021);     /* ship 3 at 32 */
    set8(AR(32, 0x2D + 9), 3);
    snap();
    f576F8(32);                         /* already right: nothing written */
    verify("576F8 already on its ship", 0, 0, NULL);

    /* -- 0x57A10, 0x57B5C, 0x576A8 -- */
    clear_records();
    set8(NA(6, 0x40), 2);
    set16(NA(6, 0x45), 31); set8(NA(6, 0x41), 0);         /* ship 0 at 30, no points */
    set16(NA(6, 0x47), 31); set8(NA(6, 0x42), 3);         /* ship 1 at 30, 3 points */
    snap();
    verify("57A10 ship 1 moves", f57A10(30, 6), 1, NULL);
    snap();
    verify("576A8 -> x", f576A8(30, 6, 0x0F), 0x0F, NULL);
    snap();
    verify("57A10 other territory", f57A10(29, 6), 0, NULL);
    set8(NA(6, 0x40), 1);
    snap();
    verify("57A10 only ship 0 counted", f57A10(30, 6), 0, NULL);
    snap();
    verify("576A8 -> 5", f576A8(30, 6, 0x0F), 5, NULL);
    set8(NA(6, 0x40), 2); set8(NA(6, 0x42), 0x80);        /* points read unsigned: 128 > 0 */
    snap();
    verify("57A10 points unsigned", f57A10(30, 6), 1, NULL);
    set16(NA(6, 0x47), 0x011F);                           /* high byte: no match */
    snap();
    verify("57A10 int16 compare", f57A10(30, 6), 0, NULL);
    set16(NA(6, 0x47), 31);
    set8(NA(6, 0x40), 0);
    snap();
    verify("57A10 no ships", f57A10(30, 6), 0, NULL);

    clear_records();
    set8(NA(6, 0x40), 3);
    set16(NA(6, 0x45), 31); set8(NA(6, 0x4D), 0);
    set16(NA(6, 0x47), 41); set8(NA(6, 0x4E), 2);
    set16(NA(6, 0x49), 31); set8(NA(6, 0x4F), 0xFF);      /* load -1: != 0 counts */
    snap();
    verify("57B5C loaded ship 2", f57B5C(30, 6), 1, NULL);
    snap();
    verify("57B5C ship 1 elsewhere", f57B5C(40, 6), 1, NULL);
    set8(NA(6, 0x40), 2);
    snap();
    verify("57B5C ship 2 not counted", f57B5C(30, 6), 0, NULL);
    snap();
    verify("576A8 no moving ship -> 5", f576A8(40, 6, 0x0F), 5, NULL);
    setD32(0xF40F8u, 0);
}

/* ==== B. random differential run ============================================================================== */

static uint8_t m_area[AREA_LEN], m_nat[NAT_LEN];
#define MA(t, off) m_area[69 * ((t) + 1) + (off)]
#define MN(n, off) m_nat[199 * (n) + (off)]

static int m_word(int n, int k)    /* int16 at nation n + 0x45 + 2k, signed */
{
    return (int16_t)(uint16_t)(MN(n, 0x45 + 2 * k) | MN(n, 0x46 + 2 * k) << 8);
}

static void m_581C4(int t, int n)
{
    int cnt, j;
    if (MA(t, 1 + n) == 0 || (int8_t)MA(t, 0x0E + n) != 0)
        return;
    MA(t, 1) = (uint8_t)(MA(t, 1) - 1);
    MA(t, 1 + n) = 0;
    for (j = 0; j < (int8_t)MN(n, 0x60); j++) {
        if (MN(n, 0x61 + j) != t + 1)
            continue;
        cnt = (int8_t)MN(n, 0x60);
        if (cnt - 1 - j > 0)
            memmove(&MN(n, 0x61 + j), &MN(n, 0x62 + j), (size_t)(cnt - 1 - j));
        MN(n, 0x60) = (uint8_t)(MN(n, 0x60) - 1);
    }
}

static void m_57CE8(int t, int n, int flag, int k)
{
    if (k < 1)
        return;
    MA(t, 0x0E + n) = (uint8_t)(MA(t, 0x0E + n) - (k & 0xFF));
    if (flag == 1)
        MA(t, 0x24 + n) = (uint8_t)(MA(t, 0x24 + n) - (k & 0xFF));
    m_581C4(t, n);
}

static void m_57E28(int t, int n, int k)
{
    int j, cnt, there = 0;
    if (k < 1)
        return;
    if ((int8_t)MA(t, 1 + n) != n) {
        MA(t, 1) = (uint8_t)(MA(t, 1) + 1);
        MA(t, 1 + n) = (uint8_t)n;
    }
    MA(t, 0x0E + n) = (uint8_t)(MA(t, 0x0E + n) + (k & 0xFF));
    cnt = (int8_t)MN(n, 0x60);
    for (j = 0; j < cnt && !there; j++)
        there = MN(n, 0x61 + j) == t + 1;
    if (!there) {
        m_nat[199 * n + 0x61 + cnt] = (uint8_t)(t + 1);
        MN(n, 0x60) = (uint8_t)(MN(n, 0x60) + 1);
    }
}

static void m_580CC(int a, int b, int n)
{
    int k;
    MA(a, 0x1B + n) = (uint8_t)(MA(a, 0x1B + n) - 1);
    m_581C4(a, n);
    k = MA(a, 0x2D + n);
    MN(n, 0x45 + 2 * k) = (uint8_t)(b + 1);              /* low byte; the high byte stays */
    MA(b, 0x2D + n) = MA(a, 0x2D + n);
    MA(b, 0x1B + n) = (uint8_t)(MA(b, 0x1B + n) + 1);
}

static void m_576F8(int t, int n)
{
    int step;
    for (step = 0; step < 5; step++) {
        if (m_word(n, MA(t, 0x2D + n)) == t + 1)
            return;
        MA(t, 0x2D + n) = (uint8_t)(((MA(t, 0x2D + n) + 1) & 0xFF) % 4);
    }
}

static int m_57A10(int t, int n)
{
    int k;
    for (k = 0; k < MN(n, 0x40); k++)
        if (m_word(n, k) == t + 1 && MN(n, 0x41 + k) != 0)
            return 1;
    return 0;
}

static int m_57B5C(int t, int n)
{
    int k;
    for (k = 0; k < MN(n, 0x40); k++)
        if (MN(n, 0x4D + k) != 0 && m_word(n, k) == t + 1)
            return 1;
    return 0;
}

static uint32_t g_rng = 12345;
static int rnd(int n)
{
    g_rng = g_rng * 1103515245u + 12345u;
    return (int)((g_rng >> 16) % (uint32_t)n);
}

static void part_b(long ops, long *done, long *bad)
{
    long i;
    uint32_t a;
    int t, n, op, r1, r2;

    for (i = 0; i < (long)AREA_LEN; i++)
        m_area[i] = (uint8_t)(rnd(4) == 0 ? rnd(256) : rnd(4));
    for (i = 0; i < (long)NAT_LEN; i++)
        m_nat[i] = (uint8_t)(rnd(4) == 0 ? rnd(256) : rnd(9));
    for (n = 1; n <= 3; n++) {                          /* some sane lists and ships on the small territories */
        MN(n, 0x60) = (uint8_t)rnd(5);
        MN(n, 0x40) = (uint8_t)rnd(5);
        for (i = 0; i < 4; i++)
            MN(n, 0x46 + 2 * (int)i) = (uint8_t)(rnd(6) == 0);
    }
    memcpy(DP(AREA_BASE), m_area, AREA_LEN);
    memcpy(DP(NAT_BASE), m_nat, NAT_LEN);
    snap();
    for (i = 0; i < ops; i++) {
        t = rnd(8);
        n = 1 + rnd(3);
        op = rnd(8);
        r1 = r2 = 0;
        switch (op) {
        case 0: { int f = rnd(3), k = rnd(6) - 1; m_57CE8(t, n, f, k); f57CE8(t, n, f, k); break; }
        case 1: { int k = rnd(6) - 1; m_57E28(t, n, k); f57E28(t, n, k); break; }
        case 2: { int b = rnd(8); m_580CC(t, b, n); f580CC(t, b, n); break; }
        case 3: m_581C4(t, n); f581C4(t, n); break;
        case 4: setD32(0xF40F8u, (uint32_t)n); m_576F8(t, n); f576F8(t); break;
        case 5: r1 = m_57A10(t, n); r2 = f57A10(t, n); break;
        case 6: r1 = m_57B5C(t, n); r2 = f57B5C(t, n); break;
        default: { int x = rnd(20); r1 = m_57A10(t, n) == 1 ? x : 5; r2 = f576A8(t, n, x); break; }
        }
        (*done)++;
        if (r1 != r2 || memcmp(DP(AREA_BASE), m_area, AREA_LEN) != 0 || memcmp(DP(NAT_BASE), m_nat, NAT_LEN) != 0) {
            (*bad)++;
            if (*bad <= 5)
                printf("FAIL random op %ld: routine %d t %d n %d, returns %d / model %d\n", i, op, t, n, r2, r1);
            memcpy(m_area, DP(AREA_BASE), AREA_LEN);    /* resync and go on */
            memcpy(m_nat, DP(NAT_BASE), NAT_LEN);
        }
    }
    /* nothing outside the two tables (and [0xF40F8]) may have changed */
    for (a = DS_BASE; a < DS_BASE + DS_SIZE; a++) {
        if ((a >= AREA_BASE && a < AREA_BASE + AREA_LEN) || (a >= NAT_BASE && a < NAT_BASE + NAT_LEN) ||
            (a >= 0xF40F8u && a < 0xF40FCu))
            continue;
        if (g_ds[a - DS_BASE] != g_before[a - DS_BASE]) {
            (*bad)++;
            printf("FAIL random run wrote 0x%05X outside the tables\n", (unsigned)a);
            break;
        }
    }
    setD32(0xF40F8u, 0);
}

/* ==== C. the dispatcher cases ================================================================================= */

#define D "F596C"

static void part_c(void)
{
    uint32_t h;
    int r;

    /* 0x26 MOVE TOKENS */
    clear_records();
    setD32(0xF52C8u, 7); setD32(0x109E10u, 100); setD32(0xF06CCu, 1); setD8(0x108804u, 2);
    g_pressed = 0;
    snap();
    r = dcase_26(-100);
    E8(0x108804u, 1); E32(0xF06CCu, 0);
    verify("dcase_26", r, -3, "3B9A4(" D ",26) vbopen(7) 3B7CC(" D ",26) 572CC(100) 3B870(" D ",27) vbclose 3CBBC ");
    g_pressed = 1;
    snap();
    r = dcase_26(-100);
    verify("dcase_26 already pressed", r, -100, "3B9A4(" D ",26) ");

    /* 0x27 MOVE SHIPS */
    g_pressed = 0;
    setD32(0xF06CCu, 1);
    snap();
    r = dcase_27(-100);
    E8(0x108804u, 2); E32(0xF06CCu, 0);
    verify("dcase_27", r, -2, "3B9A4(" D ",27) vbopen(7) 3B7CC(" D ",27) 3B870(" D ",26) 3BFBC(" D ",C4,0) "
           "3BFBC(" D ",C5,0) 3BFBC(" D ",C6,0) 3BFBC(" D ",C7,0) 3BFBC(" D ",C8,0) 3B870(" D ",C4) 3B870(" D ",C5) "
           "3B870(" D ",C6) 3B870(" D ",C7) 3B870(" D ",C8) vbclose 3CBBC ");
    g_pressed = 1;
    snap();
    verify("dcase_27 already pressed", dcase_27(-100), -100, "3B9A4(" D ",27) ");

    /* 0xC4..0xC8 */
    g_pressed = 0;
    snap();
    r = dcase_C4(-100);
    E16(0xF09E0u, 1);
    verify("dcase_C4", r, -100, "3B9A4(" D ",C4) vbopen(7) 3B7CC(" D ",C4) 3B870(" D ",C5) 3B870(" D ",C6) "
           "3B870(" D ",C7) 3B870(" D ",C8) vbclose 3CBBC ");
    snap();
    r = dcase_C6(-100);
    E16(0xF09E0u, 3);
    verify("dcase_C6", r, -100, "3B9A4(" D ",C6) vbopen(7) 3B7CC(" D ",C6) 3B870(" D ",C4) 3B870(" D ",C5) "
           "3B870(" D ",C7) 3B870(" D ",C8) vbclose 3CBBC ");
    snap();
    r = dcase_C5(-100) + dcase_C7(-100) + dcase_C8(-100);
    E16(0xF09E0u, 5);
    verify("dcase_C5, C7, C8", r, -300, NULL);
    g_pressed = 1;
    snap();
    r = dcase_C8(-100);
    verify("dcase_C8 already pressed", r, -100, "3B9A4(" D ",C8) ");
    g_pressed = 0;

    /* 0x29 */
    snap();
    verify("dcase_29", dcase_29(-100), -0x79, "");

    /* 0x28 UNDO MOVE, land: Africa moved 2 tokens Tunisia (100) -> Thapsus (101) as the stack's first step */
    clear_records();
    setD32(0xF40F8u, 1);
    set8(AR(100, 0x24 + 1), 0); set8(AR(100, 0x0E + 1), 0);                      /* source empty (dropped) */
    set8(AR(101, 1), 1); set8(AR(101, 1 + 1), 1); set8(AR(101, 0x0E + 1), 2);    /* destination: 2, none movable */
    set8(NA(1, 0x60), 2); set8(NA(1, 0x61), 103); set8(NA(1, 0x62), 102);
    setD32(0x108810u, 1); setD32(0x108808u, 100); setD32(0x10880Cu, 101); setD32(0x108814u, 1);
    setD32(0x108818u, 2); setD32(0x10881Cu, 1); setD32(0xF06CCu, 0); setD8(0xE1624u, 0); setD16(0xF09B8u, 0);
    snap();
    r = dcase_28(-100);
    E8(AR(101, 0x0E + 1), 0); E8(AR(101, 1), 0); E8(AR(101, 1 + 1), 0);          /* off the destination */
    E8(NA(1, 0x60), 2); E8(NA(1, 0x61), 103); E8(NA(1, 0x62), 101);               /* list [103,102] -> [103] + 101 */
    E8(AR(100, 0x24 + 1), 2);                                                    /* movable back (first step) */
    E8(AR(100, 1), 1); E8(AR(100, 1 + 1), 1); E8(AR(100, 0x0E + 1), 2);          /* onto the source */
    E8(0xFFC00u + 100, 0x0E); E8(0xFFC00u + 101, 5); E32(0xF06CCu, 1); E32(0x10881Cu, 0);
    verify("dcase_28 land", r, -4, "3C3D0(" D ",28) 572CC(100) 3D52C(0,0) 3CBBC ");

    /* the same with the PBEM flag: the log count goes down by one */
    setD8(0xE1624u, 0x80); setD16(0xF09B8u, 5);
    set8(AR(100, 0x24 + 1), 0); set8(AR(100, 0x0E + 1), 0); set8(AR(100, 1), 0); set8(AR(100, 1 + 1), 0);
    set8(AR(101, 1), 1); set8(AR(101, 1 + 1), 1); set8(AR(101, 0x0E + 1), 2);
    set8(NA(1, 0x60), 2); set8(NA(1, 0x61), 103); set8(NA(1, 0x62), 102);
    setD32(0x10881Cu, 1); setD32(0x108814u, 0); setD32(0xF06CCu, 0);
    snap();
    r = dcase_28(-100);
    E8(AR(101, 0x0E + 1), 0); E8(AR(101, 1), 0); E8(AR(101, 1 + 1), 0);
    E8(NA(1, 0x62), 101);
    E8(AR(100, 1), 1); E8(AR(100, 1 + 1), 1); E8(AR(100, 0x0E + 1), 2);          /* not the first step: +0x24 stays */
    E32(0xF06CCu, 1); E32(0x10881Cu, 0); E16(0xF09B8u, 4);
    verify("dcase_28 land, later step, PBEM", r, -4, "3C3D0(" D ",28) 572CC(100) 3D52C(0,0) 3CBBC ");
    setD8(0xE1624u, 0);

    /* 0x28 UNDO MOVE, ship: Africa's ship 0 moved Sabrata (102) -> Thapsus (101) */
    clear_records();
    setD32(0xF40F8u, 1);
    set8(NA(1, 0x40), 1); set16(NA(1, 0x45), 102); set8(NA(1, 0x41), 3);
    set8(AR(101, 0x1B + 1), 1); set8(AR(101, 0x2D + 1), 0);
    setD32(0x108810u, 0); setD32(0x108808u, 102); setD32(0x10880Cu, 101); setD32(0xF06CCu, 0);
    setD32(0x109E10u, 101);
    memset(DP(0xFFB60u), 0, 0xA0); setD8(0xFFB60u + 101, 0x0F);
    snap();
    r = dcase_28(-100);
    E8(AR(101, 0x1B + 1), 0); E16(NA(1, 0x45), 103); E8(AR(102, 0x2D + 1), 0); E8(AR(102, 0x1B + 1), 1);
    E8(NA(1, 0x41), 4);                                                          /* the point back */
    E8(AR(101, 0x2D + 1), 1);                                                    /* f576F8(101): no ship, 5 steps from 0 */
    E32(0xF06CCu, 1); E8(0xFFCA0u + 102, 0x0E); E8(0xFFB60u + 101, 0); E8(0xFFB60u + 102, 0x0F);
    E32(0x109E10u, 102); E32(0xF4884u, 0); E32(0xF4888u, 0);                     /* f3DA60: terr record 102 is 0 */
    verify("dcase_28 ship", r, -4, "3C3D0(" D ",28) 426D0(102) 787F8(102) 3D52C(0,0) 3CBBC ");

    /* 0x2B EMBARK, 0x2C DISEMBARK, 0x2D DISEMBARK ALL at Sabrata */
    clear_records();
    setD32(0xF40F8u, 1); setD32(0x109E10u, 102); setD16(0x109E14u, 0); setD8(0x108804u, 1); setD32(0xF06CCu, 1);
    set8(AR(102, 1), 1); set8(AR(102, 1 + 1), 1); set8(AR(102, 0x0E + 1), 2); set8(AR(102, 0x24 + 1), 1);
    set8(NA(1, 0x60), 1); set8(NA(1, 0x61), 103);
    set8(NA(1, 0x40), 2); set16(NA(1, 0x45), 50); set16(NA(1, 0x47), 103);       /* ship 1 at Sabrata, selected */
    set8(AR(102, 0x2D + 1), 1); set8(AR(102, 0x1B + 1), 1);
    snap();
    r = dcase_2B(-100);
    E8(AR(102, 0x0E + 1), 1); E8(AR(102, 0x24 + 1), 0); E8(NA(1, 0x4E), 1); E32(0xF06CCu, 0);   /* last movable */
    verify("dcase_2B", r, -100, "426D0(102) 3C3D0(" D ",28) 3CBBC ");
    snap();
    r = dcase_2B(-100);                 /* again: tokens 0 -> the nation leaves; +0x24 wraps */
    E8(AR(102, 0x0E + 1), 0); E8(AR(102, 0x24 + 1), 0xFF); E8(NA(1, 0x4E), 2);
    E8(AR(102, 1), 0); E8(AR(102, 1 + 1), 0); E8(NA(1, 0x60), 0);
    verify("dcase_2B last token", r, -100, "426D0(102) 3C3D0(" D ",28) 3CBBC ");
    snap();
    r = dcase_2C(-100);                 /* one back: territory 0xC803B[103] = 102 */
    E8(AR(102, 0x0E + 1), 1); E8(AR(102, 1), 1); E8(AR(102, 1 + 1), 1); E8(NA(1, 0x60), 1); E8(NA(1, 0x61), 103);
    E8(NA(1, 0x4E), 1);
    verify("dcase_2C", r, -100, "426D0(102) 3C3D0(" D ",28) 3CBBC ");
    set8(NA(1, 0x4E), 3);
    snap();
    r = dcase_2D(-100);
    E8(AR(102, 0x0E + 1), 4); E8(NA(1, 0x4E), 0);
    verify("dcase_2D", r, -100, "426D0(102) 3C3D0(" D ",28) 3CBBC ");

    /* DISEMBARK ALL whose selected ship stands elsewhere (Sahara, 103): the tokens and the refresh follow the ship */
    clear_records();
    setD32(0xF40F8u, 1); setD32(0x109E10u, 102); setD16(0x109E14u, 0);
    set8(NA(1, 0x40), 1); set16(NA(1, 0x45), 104); set8(NA(1, 0x4D), 2); set8(AR(102, 0x2D + 1), 0);
    snap();
    r = dcase_2D(-100);
    E8(AR(103, 1), 1); E8(AR(103, 1 + 1), 1); E8(AR(103, 0x0E + 1), 2); E8(NA(1, 0x60), 1); E8(NA(1, 0x61), 104);
    E8(NA(1, 0x4D), 0);
    verify("dcase_2D ship elsewhere", r, -100, "426D0(103) 3C3D0(" D ",28) 3CBBC ");

    /* the PBEM log of EMBARK through a real heap handle */
    clear_records();
    setD32(0xF40F8u, 1); setD32(0x109E10u, 102); setD16(0x109E14u, 0); setD8(0x108804u, 1);
    set8(AR(102, 1), 1); set8(AR(102, 1 + 1), 1); set8(NA(1, 0x60), 1); set8(NA(1, 0x61), 103);
    set8(NA(1, 0x40), 2); set16(NA(1, 0x45), 50); set16(NA(1, 0x47), 103);
    set8(AR(102, 0x2D + 1), 1); set8(AR(102, 0x1B + 1), 1);
    h = load_heap_alloc(10000);
    setD32(0xF09B4u, h); setD16(0xF09B8u, 3); setD8(0xE1624u, 0x80);
    set8(AR(102, 0x0E + 1), 2); set8(AR(102, 0x24 + 1), 2); set8(NA(1, 0x4E), 0);
    snap();
    r = dcase_2B(-100);
    E8(AR(102, 0x0E + 1), 1); E8(AR(102, 0x24 + 1), 1); E8(NA(1, 0x4E), 1); E16(0xF09B8u, 4);
    verify("dcase_2B PBEM", r, -100, "426D0(102) 3C3D0(" D ",28) 3CBBC ");
    {
        uint8_t *p = load_heap_ptr(h, 40);
        g_tests++;
        if (p == NULL || p[32] != 102 || p[38] != 3 || p[39] != 1) {
            g_fail++;
            printf("FAIL dcase_2B PBEM record: %d %d %d\n", p ? p[32] : -1, p ? p[38] : -1, p ? p[39] : -1);
        }
    }
    setD8(0xE1624u, 0);

    /* 0x2E NEXT SHIP: two ships at Sabrata, selection 0 -> 1; fixed: no log; --original: the count grows */
    clear_records();
    setD32(0xF40F8u, 1); setD32(0x109E10u, 102); setD32(0xF06CCu, 1); setD16(0xF09B8u, 3); setD32(0xF09B4u, 0);
    set8(NA(1, 0x40), 2); set16(NA(1, 0x45), 103); set16(NA(1, 0x47), 103); set8(NA(1, 0x42), 4);
    snap();
    r = dcase_2E(-100);
    E8(AR(102, 0x2D + 1), 1); E32(0xF06CCu, 0); E8(0xFFCA0u + 102, 0x0F);
    verify("dcase_2E", r, -2, "426D0(102) ");
    g_original = 1;
    snap();
    r = dcase_2E(-100);                 /* 1 -> 2: ship 2 does not exist (+0x45+4 = 0) -> 3 -> 0 (ship 0 there) */
    E8(AR(102, 0x2D + 1), 0); E16(0xF09B8u, 4); E32(0xF06CCu, 0); E8(0xFFCA0u + 102, 0x0F);
    verify("dcase_2E --original (count grows, handle 0)", r, -2, "426D0(102) ");
    setD32(0xF09B4u, h);
    snap();
    r = dcase_2E(-100);
    E8(AR(102, 0x2D + 1), 1); E16(0xF09B8u, 5);
    verify("dcase_2E --original with a handle", r, -2, "426D0(102) ");
    {
        uint8_t *p = load_heap_ptr(h, 50);
        g_tests++;
        if (p == NULL || p[40] != 102 || p[48] != 0x0B) {
            g_fail++;
            printf("FAIL dcase_2E log record: %d %d\n", p ? p[40] : -1, p ? p[48] : -1);
        }
    }
    g_original = 0;
    set8(NA(1, 0x42), 0); set8(NA(1, 0x41), 0);    /* no points: the ship box gets 5 */
    snap();
    r = dcase_2E(-100);
    E8(AR(102, 0x2D + 1), 0); E8(0xFFCA0u + 102, 5);
    verify("dcase_2E no points left", r, -2, "426D0(102) ");

    /* the second coast: territory 0x39 (57) and its twin 0x9D (157) */
    clear_records();
    setD32(0xF40F8u, 3); setD32(0x109E10u, 57); setD16(0x109E14u, 0); setD8(0x108804u, 2); setD32(0xF09B4u, 0);
    set8(NA(3, 0x40), 2); set16(NA(3, 0x45), 58); set16(NA(3, 0x47), 158);       /* ship 0 at 57, ship 1 at 157 */
    set8(AR(57, 0x2D + 3), 0); set8(AR(157, 0x2D + 3), 1);
    set8(AR(57, 1), 1); set8(AR(57, 1 + 3), 3); set8(AR(57, 0x0E + 3), 2); set8(AR(57, 0x24 + 3), 2);
    set8(NA(3, 0x60), 1); set8(NA(3, 0x61), 58);
    snap();
    r = dcase_2F(-100);                 /* a token of 57 onto ship 1 (the twin's) */
    E8(AR(57, 0x0E + 3), 1); E8(AR(57, 0x24 + 3), 1); E8(NA(3, 0x4E), 1);
    verify("dcase_2F", r, -100, "426D0(57) 3C3D0(" D ",28) 3CBBC ");
    set8(NA(3, 0x4E), 3);
    snap();
    r = dcase_30(-100);                 /* territory from ship 0 (record 57): 0xC803B[58] = 57; ship 1 unloads */
    E8(AR(57, 0x0E + 3), 2); E8(NA(3, 0x4E), 2);
    verify("dcase_30", r, -100, "426D0(57) 3C3D0(" D ",28) 3CBBC ");
    snap();
    r = dcase_31(-100);                 /* the rest of ship 1; refresh at ship 1's territory - 1 = 157 */
    E8(AR(57, 0x0E + 3), 4); E8(NA(3, 0x4E), 0);
    verify("dcase_31", r, -100, "426D0(157) 3C3D0(" D ",28) 3CBBC ");
    setD16(0xF09B8u, 0);
    snap();
    r = dcase_32(-100);                 /* the twin's selection 1 -> 2 -> (no ship) 3 -> 0 -> 1 (ship 1 at 157) */
    E8(AR(157, 0x2D + 3), 1);
    verify("dcase_32", r, -100, "426D0(57) ");
    g_original = 1;
    snap();
    r = dcase_32(-100);
    E8(AR(157, 0x2D + 3), 1); E16(0xF09B8u, 1);
    verify("dcase_32 --original", r, -100, "426D0(57) ");
    g_original = 0;
    setD32(0xF40F8u, 0);
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:/work/advciv_run";
    long done = 0, bad = 0;
    int a_tests, a_fail;

    if (state_init(gamedir) != 0)
        return 1;
    g_before = malloc(DS_SIZE);
    g_want = malloc(DS_SIZE);
    if (g_before == NULL || g_want == NULL)
        return 1;

    part_a();
    a_tests = g_tests;
    a_fail = g_fail;
    printf("A. primitives, hand vectors: %d of %d pass\n", a_tests - a_fail, a_tests);

    part_b(200000, &done, &bad);
    printf("B. random differential run: %ld operations, %ld disagree\n", done, bad);

    part_c();
    printf("C. dispatcher cases: %d of %d pass\n", (g_tests - a_tests) - (g_fail - a_fail), g_tests - a_tests);

    return g_fail == 0 && bad == 0 ? 0 : 1;
}
