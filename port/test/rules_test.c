/* rules_test [GAMEDIR]: unit vectors for module RULES (rules.c) that need no game run.

   It loads the data object from GAMEDIR/AC_MAIN.EXE as the port does (state_init: le_image.c; GAMEDIR defaults to the
   run copy C:\work\advciv_run, only read), writes synthetic contents into the records the routines use and applies one
   routine. Expectations are written here from the disassembly's semantics, independently of rules.c:
     A. hand vectors for the 16 routines: every byte the routine should change is listed with its value, worked out by
        hand (quirks included: signed counts and compares, byte wrap, the int16 ship words, the discard table of the
        data object, the 1995 fixes and their --original paths), and the whole data object must equal the snapshot
        taken before plus exactly those bytes;
     B. a random differential run: a second implementation of the 16 routines (and of the three move.c primitives they
        call) on a plain copy of the data object, written from the listing, not from rules.c or move.c; both apply the
        same random operations to the same random records; the touched tables and the return values must agree after
        every operation, the whole data object at the end. Once with the fixes, once with --original.
   Prints each failure, the counts per part, and exits 0 when all pass.

   Link: rules_test.c rules.c move.c mapdata.c load.c state.c le_image.c (move.c for f57CE8, f57E28, f576F8; the others
   because move.c needs them). The UI routines those files call are stubs that only complain if reached. */

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
#include "../src/rules.h"

/* ---- stubs for what move.c and the loaders reference (none is reached by the RULES routines) ------------------ */

static int g_stub_calls;
static void stub(const char *name) { g_stub_calls++; printf("FAIL unexpected call of %s\n", name); }

int fg_vbopen(int handle) { (void)handle; stub("fg_vbopen"); return 0; }
void fg_vbclose(void) { stub("fg_vbclose"); }
int f3B9A4(uint32_t dlg, int id) { (void)dlg; (void)id; stub("f3B9A4"); return 0; }
void f3B7CC(uint32_t dlg, int id) { (void)dlg; (void)id; stub("f3B7CC"); }
void f3B870(uint32_t dlg, int id) { (void)dlg; (void)id; stub("f3B870"); }
void f3BFBC(uint32_t dlg, int id, int en) { (void)dlg; (void)id; (void)en; stub("f3BFBC"); }
void f3C3D0(uint32_t dlg, int id) { (void)dlg; (void)id; stub("f3C3D0"); }
void f3CBBC(void) { stub("f3CBBC"); }
void f3D52C(int dx, int dy) { (void)dx; (void)dy; stub("f3D52C"); }
void f787F8(int t) { (void)t; stub("f787F8"); }
void f426D0(int i) { (void)i; stub("f426D0"); }
void f572CC(int t) { (void)t; stub("f572CC"); }
void f3A780(const char *msg) { (void)msg; stub("f3A780"); }
_Noreturn void f44CAC(void) { fprintf(stderr, "rules_test: quit\n"); exit(2); }

/* ---- addresses (from the listing's constants, written out here) ----------------------------------------------- */

#define AREA_BASE 0xE31B8u
#define AREA_LEN  (161u * 69u)
#define NAT_BASE  0xF14CCu
#define NAT_LEN   (14u * 199u)
#define TS_BASE   0xE2C18u
#define TS_LEN    (160u * 3u)
#define TERR_BASE 0xE1638u
#define CARD_BASE 0xF06D0u            /* the stacks 0xF06D0.. and the discard piles 0xF07B4.. with room after them */
#define CARD_LEN  0x300u
#define AR(t, off) (AREA_BASE + 69u * (uint32_t)((t) + 1) + (uint32_t)(off))   /* field off of territory t's record */
#define NA(n, off) (NAT_BASE + 199u * (uint32_t)(n) + (uint32_t)(off))         /* field off of nation n's record */
#define TS(t, off) (TS_BASE + 3u * (uint32_t)(t) + (uint32_t)(off))
#define TR(t, off) (TERR_BASE + 35u * (uint32_t)(t) + (uint32_t)(off))
#define ST(s, off) (0xF06D0u + 25u * (uint32_t)(s) + (uint32_t)(off))         /* stack s */
#define PI(s, off) (0xF07B4u + 25u * (uint32_t)(s) + (uint32_t)(off))         /* discard pile s */

static uint8_t *g_before, *g_want;
static struct { uint32_t a; uint8_t v; } g_exp[1024];
static int g_nexp;
static int g_tests, g_fail;

static void clear_records(void)
{
    memset(DP(AREA_BASE), 0, AREA_LEN);
    memset(DP(NAT_BASE), 0, NAT_LEN);
    memset(DP(TS_BASE), 0, TS_LEN);
    memset(DP(CARD_BASE), 0, CARD_LEN);
}

static void snap(void)
{
    memcpy(g_before, g_ds, DS_SIZE);
    g_nexp = 0;
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

/* the data object must equal the snapshot plus the expected bytes, and ret must be want_ret */
static void verify(const char *name, int ret, int want_ret)
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
    if (diffs != 0 || ret != want_ret) {
        g_fail++;
        printf("FAIL %s: %d bytes differ, returned %d (expected %d)\n", name, diffs, ret, want_ret);
    }
}

static void set8(uint32_t a, uint8_t v) { setD8(a, v); }
static void set16(uint32_t a, uint16_t v) { setD16(a, v); }

/* ==== A. hand vectors ========================================================================================== */

static void part_a(void)
{
    int n;

    /* -- 0x78D08: population limit -- */
    clear_records();
    set8(TR(30, 1), 0x1C);              /* 0001 1100: bits 3..5 = 3 */
    snap();
    verify("78D08 limit 3, no advance", f78D08(30, 4), 3);
    set8(NA(4, 0x27), 1);
    snap();
    verify("78D08 advance, area empty (no bonus)", f78D08(30, 4), 3);
    set8(AR(30, 1), 1); set8(AR(30, 1 + 4), 4);
    snap();
    verify("78D08 advance, alone", f78D08(30, 4), 4);
    set8(AR(30, 1), 2);
    snap();
    verify("78D08 advance, two nations", f78D08(30, 4), 3);
    set8(AR(30, 1), 1); set8(AR(30, 1 + 4), 2);
    snap();
    verify("78D08 advance, marker of another", f78D08(30, 4), 3);
    set8(AR(30, 1 + 4), 4); set8(NA(4, 0x27), 0x80);
    snap();
    verify("78D08 advance byte 0x80 counts as held (+1)", f78D08(30, 4), 4);
    set8(TR(30, 1), 0xC7);              /* 1100 0111: bits 3..5 = 0 */
    snap();
    verify("78D08 other bits ignored", f78D08(30, 4), 1);
    set8(TR(30, 1), 0x38); set8(AR(30, 1), 0x81);
    snap();
    verify("78D08 limit 7, count byte 0x81 (signed, not 1)", f78D08(30, 4), 7);

    /* -- 0x78E74: city built -- */
    clear_records();
    set8(0xF40FCu, 0x0B);
    set8(TS(12, 2), 0x06); set8(TS(12, 1), 0xFF);
    set8(NA(3, 0x51), 2); set8(NA(3, 0x52), 5); set8(NA(3, 0x53), 9);
    snap();
    f78E74(12, 3);
    E8(TS(12, 2), 0x07); E8(TS(12, 0), 3); E8(TS(12, 1), 0x0B); E8(NA(3, 0x54), 13); E8(NA(3, 0x51), 3);
    verify("78E74 third city", 0, 0);
    set8(NA(3, 0x51), 0xFE); set8(NA(3, 0x50), 0x77);   /* count -2: the entry goes to +0x50 */
    snap();
    f78E74(255, 3);                     /* t + 1 = 256 -> byte 0 */
    E8(TS(255, 2), (uint8_t)(g_before[TS(255, 2) - DS_BASE] | 1)); E8(TS(255, 0), 3); E8(TS(255, 1), 0x0B);
    E8(NA(3, 0x50), 0); E8(NA(3, 0x51), 0xFF);
    verify("78E74 negative count, t + 1 byte wrap", 0, 0);
    set8(0xF40FCu, 0);

    /* -- 0x78F04: city removed -- */
    clear_records();
    set8(TS(12, 0), 3); set8(TS(12, 1), 7); set8(TS(12, 2), 0x07);
    set8(NA(3, 0x51), 4); set8(NA(3, 0x52), 5); set8(NA(3, 0x53), 13); set8(NA(3, 0x54), 9); set8(NA(3, 0x55), 13);
    snap();
    f78F04(12, 3);                      /* [5,13,9,13] -> [5,9,13], first 13 removed, slot 3 = 0 */
    E8(TS(12, 0), 0); E8(TS(12, 1), 0xFF); E8(TS(12, 2), 0x06);
    E8(NA(3, 0x53), 9); E8(NA(3, 0x54), 13); E8(NA(3, 0x55), 0); E8(NA(3, 0x51), 3);
    verify("78F04 first match, shift", 0, 0);
    snap();
    f78F04(12, 3);                      /* [5,9,13] -> [5,9], last */
    E8(TS(12, 1), 0xFF);
    E8(NA(3, 0x54), 0); E8(NA(3, 0x51), 2);
    verify("78F04 last entry", 0, 0);
    snap();
    f78F04(40, 3);                      /* not held: the territory state still cleared, the list kept (fix) */
    E8(TS(40, 1), 0xFF);
    verify("78F04 not in list (fixed)", 0, 0);
    g_original = 1;
    snap();
    f78F04(40, 3);                      /* --original stand-in index 0: [5,9] -> [9], slot 1 = 0 */
    E8(TS(40, 1), 0xFF); E8(NA(3, 0x52), 9); E8(NA(3, 0x53), 0); E8(NA(3, 0x51), 1);
    verify("78F04 not in list --original", 0, 0);
    g_original = 0;
    set8(NA(3, 0x51), 0x82); set8(NA(3, 0x52), 41);
    snap();
    f78F04(40, 3);                      /* count negative: the search does not run -> not found (fix) */
    E8(TS(40, 1), 0xFF);
    verify("78F04 negative count", 0, 0);
    set8(NA(3, 0x51), 0); set8(NA(3, 0x52), 41);
    g_original = 1;
    snap();
    f78F04(40, 3);                      /* --original, empty list: count 0 -> 0xFF (0x78FD2), then 0x78FD8 reads -1 and
                                           clears +0x52 - 1 = the count itself: count 0 again, +0x52 kept */
    E8(TS(40, 1), 0xFF);
    verify("78F04 --original, empty list: the count clears itself", 0, 0);
    g_original = 0;

    /* -- 0x78FEC: city reduced to tokens -- */
    clear_records();
    set8(TR(20, 1), 0x20);              /* limit 4 */
    set8(TS(20, 0), 6); set8(TS(20, 2), 1); set8(TS(20, 1), 3);
    set8(NA(6, 0x51), 1); set8(NA(6, 0x52), 21);
    set8(NA(6, 0x99), 10); set8(NA(6, 0x98), 20); set8(NA(6, 0x27), 2);
    snap();
    f78FEC(20, 6);                      /* area empty: limit 4 + byte 2 = 6 tokens */
    E8(TS(20, 0), 0); E8(TS(20, 1), 0xFF); E8(TS(20, 2), 0); E8(NA(6, 0x52), 0); E8(NA(6, 0x51), 0);
    E8(AR(20, 1), 1); E8(AR(20, 1 + 6), 6); E8(AR(20, 0x0E + 6), 6); E8(NA(6, 0x60), 1); E8(NA(6, 0x61), 21);
    E8(NA(6, 0x99), 4); E8(NA(6, 0x98), 26);
    verify("78FEC empty area, bonus is the byte (+2)", 0, 0);

    clear_records();
    set8(TR(20, 1), 0x20);
    set8(NA(6, 0x99), 10); set8(NA(6, 0x27), 1);
    set8(AR(20, 1), 1); set8(AR(20, 1 + 2), 2); set8(AR(20, 0x0E + 2), 3);   /* another nation alone there */
    snap();
    f78FEC(20, 6);                      /* not found in the list: the list untouched; limit 4, no bonus */
    E8(TS(20, 1), 0xFF);
    E8(AR(20, 1), 2); E8(AR(20, 1 + 6), 6); E8(AR(20, 0x0E + 6), 4); E8(NA(6, 0x60), 1); E8(NA(6, 0x61), 21);
    E8(NA(6, 0x99), 6); E8(NA(6, 0x98), 4);
    verify("78FEC other nation there, no bonus", 0, 0);

    clear_records();
    set8(TR(20, 1), 0x20);
    set8(NA(6, 0x99), 10); set8(NA(6, 0x27), 1);
    set8(AR(20, 1), 1); set8(AR(20, 0x0E + 6), 2);   /* count 1, n's tokens there (no marker): bonus */
    set8(NA(6, 0x60), 1); set8(NA(6, 0x61), 21);
    snap();
    f78FEC(20, 6);                      /* limit 5: marker != 6 -> count 2, marker 6; tokens 2 + 5 */
    E8(TS(20, 1), 0xFF);
    E8(AR(20, 1), 2); E8(AR(20, 1 + 6), 6); E8(AR(20, 0x0E + 6), 7);
    E8(NA(6, 0x99), 5); E8(NA(6, 0x98), 5);
    verify("78FEC count 1 with n's tokens: bonus", 0, 0);

    clear_records();
    set8(TR(20, 1), 0x38);              /* limit 7, stock 3 */
    set8(NA(6, 0x99), 3); set8(NA(6, 0x98), 0xFF);
    snap();
    f78FEC(20, 6);
    E8(TS(20, 1), 0xFF);
    E8(AR(20, 1), 1); E8(AR(20, 1 + 6), 6); E8(AR(20, 0x0E + 6), 3); E8(NA(6, 0x60), 1); E8(NA(6, 0x61), 21);
    E8(NA(6, 0x99), 0); E8(NA(6, 0x98), 2);
    verify("78FEC stock below the limit, board byte wraps", 0, 0);

    clear_records();
    set8(TR(20, 1), 0x08);              /* limit 1, stock -2: k = -2, no token; stock 0, board -2 */
    set8(NA(6, 0x99), 0xFE); set8(NA(6, 0x98), 5);
    snap();
    f78FEC(20, 6);
    E8(TS(20, 1), 0xFF); E8(NA(6, 0x99), 0); E8(NA(6, 0x98), 3);
    verify("78FEC negative stock", 0, 0);

    clear_records();
    set8(TR(20, 1), 0x10);              /* limit 2 + byte 0xFF (-1) = 1 */
    set8(NA(6, 0x99), 9); set8(NA(6, 0x27), 0xFF);
    snap();
    f78FEC(20, 6);
    E8(TS(20, 1), 0xFF);
    E8(AR(20, 1), 1); E8(AR(20, 1 + 6), 6); E8(AR(20, 0x0E + 6), 1); E8(NA(6, 0x60), 1); E8(NA(6, 0x61), 21);
    E8(NA(6, 0x99), 8); E8(NA(6, 0x98), 1);
    verify("78FEC signed advance byte", 0, 0);

    /* -- 0x7C760 / 0x7C7C4: nations present -- */
    clear_records();
    snap();
    verify("7C760 none", f7C760(50), 0);
    set8(AR(50, 1 + 7), 7); set8(AR(50, 1 + 3), 2); set8(AR(50, 1 + 11), 11);
    snap();
    verify("7C760 first is 7 (marker 2 at 3 does not count)", f7C760(50), 7);
    verify("7C7C4 not 7 -> 11", f7C7C4(50, 7), 11);
    verify("7C7C4 not 3 -> 7", f7C7C4(50, 3), 7);
    set8(AR(50, 1 + 7), 0); set8(AR(50, 1 + 12), 12);
    snap();
    verify("7C760 nation 12 not looked at", f7C760(50), 11);
    verify("7C7C4 only 11, excluded", f7C7C4(50, 11), 0);
    set8(AR(-1, 1 + 1), 1);
    snap();
    verify("7C760 t = -1 (record 0)", f7C760(-1), 1);

    /* -- 0x789EC -- */
    {
        static const int want[14] = { 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1 };
        char name[64];
        snap();
        for (n = 0; n < 14; n++) {
            snprintf(name, sizeof name, "789EC(%d)", n);
            verify(name, f789EC(n), want[n]);
        }
        verify("789EC(-4)", f789EC(-4), 1);
    }

    /* -- 0x78D98: ship removed -- */
    clear_records();
    set8(NA(5, 0x40), 3);
    set16(NA(5, 0x45), 11); set16(NA(5, 0x47), 21); set16(NA(5, 0x49), 11); set16(NA(5, 0x4B), 0x1234);
    set8(NA(5, 0x41), 4); set8(NA(5, 0x42), 3); set8(NA(5, 0x43), 2); set8(NA(5, 0x4E), 2);
    set8(AR(20, 0x1B + 5), 1); set8(AR(20, 0x2D + 5), 1);
    set8(AR(10, 0x1B + 5), 2); set8(AR(10, 0x2D + 5), 2);
    snap();
    f78D98(1, 5);                       /* ship 1 (at 20) goes: words [11, 11, 0x1234 -> 0]; count 2; ships at 20: 0 */
    E16(NA(5, 0x47), 11); E16(NA(5, 0x49), 0x1234); E16(NA(5, 0x4B), 0); E8(NA(5, 0x40), 2);
    E8(AR(20, 0x1B + 5), 0); E32(0xF40F8u, 5);
    E8(AR(10, 0x2D + 5), 0);            /* f576F8(10): sel 2 -> word 0x1234 no -> 3 -> word 0 no -> 0 -> 11 yes */
    verify("78D98 middle ship, points and loads not shifted", 0, 0);

    clear_records();
    set8(NA(5, 0x40), 1); set16(NA(5, 0x45), 0x0115);
    set8(AR(0x114, 0x1B + 5), 7);
    snap();
    f78D98(0, 5);                       /* int16 word 0x115 = 277: area record 277; no ship left: no f576F8, [0xF40F8] */
    E16(NA(5, 0x45), 0); E8(NA(5, 0x40), 0); E8(AR(0x114, 0x1B + 5), 6);
    verify("78D98 last ship, int16 word as record index", 0, 0);

    clear_records();
    set8(NA(2, 0x40), 4);
    set16(NA(2, 0x45), 31); set16(NA(2, 0x47), 32); set16(NA(2, 0x49), 33); set16(NA(2, 0x4B), 34);
    set8(AR(33, 0x1B + 2), 1);
    set8(AR(30, 0x2D + 2), 0); set8(AR(31, 0x2D + 2), 1); set8(AR(32, 0x2D + 2), 2);
    snap();
    f78D98(3, 2);                       /* last of four: no shift; the others' selections already right */
    E16(NA(2, 0x4B), 0); E8(NA(2, 0x40), 3); E8(AR(33, 0x1B + 2), 0); E32(0xF40F8u, 2);
    verify("78D98 ship 3 of 4", 0, 0);
    setD32(0xF40F8u, 0);

    clear_records();
    set8(NA(5, 0x40), 1); set16(NA(5, 0x45), 0xFFFF); set16(NA(5, 0x47), 0x0203);
    snap();
    f78D98(0, 5);                       /* 0x78DD2 sar 16: word 0xFFFF = -1, area record -1 (below the table): byte
                                           0xE31B8 - 69 + 4 + 0x1C decremented; words [0x0203, 0, 0, 0]; no ship left */
    E16(NA(5, 0x45), 0x0203); E16(NA(5, 0x47), 0); E8(NA(5, 0x40), 0);
    E8(0xE31B8u - 69u + 4u + 0x1Cu, (uint8_t)(g_before[0xE31B8u - 69u + 4u + 0x1Cu - DS_BASE] - 1u));
    verify("78D98 int16 word -1 (signed record index)", 0, 0);
    set8(0xE31B8u - 69u + 4u + 0x1Cu, g_before[0xE31B8u - 69u + 4u + 0x1Cu - DS_BASE]);   /* restored */

    /* -- 0x57CA4: tokens from a to b -- */
    clear_records();
    set8(AR(3, 1), 1); set8(AR(3, 1 + 2), 2); set8(AR(3, 0x0E + 2), 2); set8(AR(3, 0x24 + 2), 2);
    set8(NA(2, 0x60), 1); set8(NA(2, 0x61), 4);
    snap();
    f57CA4(3, 7, 2, 1, 2);              /* all leave 3 (list [4] -> [] then [8]) */
    E8(AR(3, 0x0E + 2), 0); E8(AR(3, 0x24 + 2), 0); E8(AR(3, 1), 0); E8(AR(3, 1 + 2), 0);
    E8(AR(7, 1), 1); E8(AR(7, 1 + 2), 2); E8(AR(7, 0x0E + 2), 2); E8(NA(2, 0x61), 8);
    verify("57CA4 whole stack moves", 0, 0);
    snap();
    f57CA4(7, 3, 2, 0, 1);              /* flag 0: +0x24 of 7 untouched */
    E8(AR(7, 0x0E + 2), 1); E8(AR(3, 1), 1); E8(AR(3, 1 + 2), 2); E8(AR(3, 0x0E + 2), 1);
    E8(NA(2, 0x60), 2); E8(NA(2, 0x62), 4);
    verify("57CA4 one token, flag 0", 0, 0);

    /* -- 0x460C0: movable tokens -- */
    clear_records();
    set8(AR(0, 1 + 4), 4); set8(AR(0, 0x0E + 4), 5); set8(AR(0, 0x24 + 4), 1);
    set8(AR(159, 1 + 4), 0x80); set8(AR(159, 0x0E + 4), 0xFE);
    set8(AR(70, 0x0E + 4), 3); set8(AR(70, 0x24 + 4), 3);              /* no marker: 0 */
    set8(AR(-1, 1 + 4), 4); set8(AR(-1, 0x0E + 4), 9);                 /* record 0 is not visited */
    set8(AR(160, 1 + 4), 4); set8(AR(160, 0x0E + 4), 9);               /* record 161 neither */
    set8(AR(80, 1 + 3), 3); set8(AR(80, 0x0E + 3), 6);                 /* another nation */
    snap();
    f460C0(4);
    E8(AR(0, 0x24 + 4), 5); E8(AR(159, 0x24 + 4), 0xFE); E8(AR(70, 0x24 + 4), 0);
    verify("460C0", 0, 0);

    /* -- 0x46158: ship points -- */
    clear_records();
    set8(NA(7, 0x40), 3); set8(NA(7, 0x44), 9);
    snap();
    f46158(7);
    E8(NA(7, 0x41), 4); E8(NA(7, 0x42), 4); E8(NA(7, 0x43), 4);
    verify("46158 three ships, no Clothmaking", 0, 0);
    set8(NA(7, 0x25), 0x80); set8(NA(7, 0x40), 6);
    snap();
    f46158(7);                          /* 6 ships: +0x41..+0x46 (the last two inside the words) */
    E8(NA(7, 0x41), 5); E8(NA(7, 0x42), 5); E8(NA(7, 0x43), 5); E8(NA(7, 0x44), 5); E8(NA(7, 0x45), 5);
    E8(NA(7, 0x46), 5);
    verify("46158 Clothmaking, count past 4", 0, 0);

    /* -- 0x76E50: draw -- */
    clear_records();
    snap();
    verify("76E50 empty stack", f76E50(3, 2), 0);
    set16(ST(3, 0), 3); set8(ST(3, 6), 7); set8(ST(3, 7), 0x17); set8(ST(3, 8), 8);
    set8(NA(2, 0x13), 1); set8(NA(2, 0x14), 0x1E); set8(NA(2, 0x1C), 4); set8(NA(2, 0x1D), 0xFF);
    snap();
    {
        int r = f76E50(3, 2);           /* top card 8: commodity */
        E16(ST(3, 0), 2); E8(NA(2, 8), 1);
        verify("76E50 commodity", r, 8);
        snap();
        r = f76E50(3, 2);               /* 0x17: calamity appended at slot 1; giver byte stays 0xFF */
        E16(ST(3, 0), 1); E8(NA(2, 0x15), 0x17); E8(NA(2, 0x13), 2);
        verify("76E50 calamity", r, 0x17);
        set8(ST(3, 6), 0);
        snap();
        r = f76E50(3, 2);               /* card byte 0: count still decremented, nothing given */
        E16(ST(3, 0), 0);
        verify("76E50 card 0", r, 0);
        set16(ST(3, 0), 1); set8(ST(3, 6), 0x90);
        snap();
        r = f76E50(3, 2);               /* 0x90 = -112: commodity path, byte at the record - 112 */
        E16(ST(3, 0), 0); E8(NA(2, 0) - 112u, 1);
        verify("76E50 negative card byte", r, -112);
        set16(ST(3, 0), 0xFFFF); set8(ST(3, 4), 0x1B);
        snap();
        r = f76E50(3, 2);               /* count -1: -2 stored, card at +5 + (-1) = +4 */
        E16(ST(3, 0), 0xFFFE); E8(NA(2, 0x16), 0x1B); E8(NA(2, 0x13), 3);
        verify("76E50 negative count", r, 0x1B);
    }

    /* -- 0x771F8: discard -- */
    clear_records();
    snap();
    f771F8(5);
    E16(PI(2, 0), 1); E8(PI(2, 6), 5);
    verify("771F8 commodity 5 -> pile 2", 0, 0);
    snap();
    f771F8(6);
    E16(PI(2, 0), 2); E8(PI(2, 7), 6);
    verify("771F8 commodity 6 -> pile 2, appended", 0, 0);
    {
        static const int pile[12] = { 1, 1, 2, 2, 3, 3, 4, 4, 5, 6, 7, 8 };   /* from the table at 0xCA9D4 */
        int c, cnt[9] = { 0, 0, 2, 0, 0, 0, 0, 0, 0 };
        char name[64];
        for (c = 0x13; c <= 0x1E; c++) {
            int s = pile[c - 0x13];
            snap();
            f771F8(c);
            E16(PI(s, 0), (uint16_t)(cnt[s] + 1)); E8(PI(s, 6 + cnt[s]), (uint8_t)c);
            cnt[s]++;
            snprintf(name, sizeof name, "771F8 calamity 0x%X -> pile %d", c, s);
            verify(name, 0, 0);
        }
    }
    clear_records();
    snap();
    f771F8(0);                          /* (0 - 1) / 2 = 0 */
    E16(PI(0, 0), 1); E8(PI(0, 6), 0);
    verify("771F8 card 0 -> pile 0", 0, 0);
    snap();
    f771F8(-2);                         /* (-3) / 2 = -1: pile -1 is 0xF079B, card at 0xF07A1 + 0 */
    E16(0xF079Bu, 1); E8(0xF07A1u, 0xFE);
    verify("771F8 card -2 -> pile -1 (truncation)", 0, 0);
    set16(PI(1, 0), 0xFFFF);
    snap();
    f771F8(3);                          /* count -1: card at +5 */
    E16(PI(1, 0), 0); E8(PI(1, 5), 3);
    verify("771F8 negative count", 0, 0);

    /* -- 0x6E460: sort calamities -- */
    clear_records();
    set8(NA(8, 0x13), 4);
    set8(NA(8, 0x14), 0x1E); set8(NA(8, 0x15), 0x13); set8(NA(8, 0x16), 0x85); set8(NA(8, 0x17), 0x15);
    set8(NA(8, 0x1C), 1); set8(NA(8, 0x1D), 2); set8(NA(8, 0x1E), 3); set8(NA(8, 0x1F), 4);
    set8(NA(8, 0x18), 0x01);            /* past the count: not sorted */
    snap();
    f6E460(8);                          /* signed: 0x85 (-123), 0x13, 0x15, 0x1E; givers 3, 2, 4, 1 */
    E8(NA(8, 0x14), 0x85); E8(NA(8, 0x15), 0x13); E8(NA(8, 0x16), 0x15); E8(NA(8, 0x17), 0x1E);
    E8(NA(8, 0x1C), 3); E8(NA(8, 0x1D), 2); E8(NA(8, 0x1E), 4); E8(NA(8, 0x1F), 1);
    verify("6E460 signed sort, givers along", 0, 0);
    set8(NA(8, 0x14), 0x16); set8(NA(8, 0x15), 0x16); set8(NA(8, 0x16), 0x14); set8(NA(8, 0x17), 0x16);
    set8(NA(8, 0x1C), 1); set8(NA(8, 0x1D), 2); set8(NA(8, 0x1E), 3); set8(NA(8, 0x1F), 4);
    snap();
    f6E460(8);                          /* bubble from the end, swaps only on >: [0x14 g3, 0x16 g1, 0x16 g2, 0x16 g4] */
    E8(NA(8, 0x14), 0x14); E8(NA(8, 0x15), 0x16); E8(NA(8, 0x16), 0x16); E8(NA(8, 0x17), 0x16);
    E8(NA(8, 0x1C), 3); E8(NA(8, 0x1D), 1); E8(NA(8, 0x1E), 2); E8(NA(8, 0x1F), 4);
    verify("6E460 equal ids keep their order", 0, 0);
    set8(NA(8, 0x13), 0x81);
    snap();
    f6E460(8);
    verify("6E460 negative count", 0, 0);

    /* -- 0x75CA8 / 0x75D84: take and give -- */
    clear_records();
    set8(NA(1, 3), 2);
    snap();
    f75CA8(3, NA(1, 0));
    E8(NA(1, 3), 1);
    verify("75CA8 commodity", 0, 0);
    snap();
    f75CA8(0, NA(1, 0));
    f75CA8(0x100, NA(1, 0));            /* al = 0 */
    verify("75CA8 card 0 ignored", 0, 0);
    snap();
    f75CA8(0x1F0, NA(1, 0));            /* al = 0xF0 = -16: commodity path, byte at rec - 16 */
    E8(NA(1, 0) - 16u, 0xFF);
    verify("75CA8 negative card byte", 0, 0);
    set8(NA(1, 0x13), 3); set8(NA(1, 0x14), 0x13); set8(NA(1, 0x15), 0x1B); set8(NA(1, 0x16), 0x1B);
    set8(NA(1, 0x1C), 5); set8(NA(1, 0x1D), 6); set8(NA(1, 0x1E), 7);
    snap();
    f75CA8(0x1B, NA(1, 0));             /* first 0x1B: [0x13 5, 0x1B 7], slot 2 = 0 / 0xFF */
    E8(NA(1, 0x15), 0x1B); E8(NA(1, 0x1D), 7); E8(NA(1, 0x16), 0); E8(NA(1, 0x1E), 0xFF); E8(NA(1, 0x13), 2);
    verify("75CA8 calamity, first match", 0, 0);
    snap();
    f75CA8(0x1D, NA(1, 0));             /* not held: kept (fix) */
    verify("75CA8 calamity not held (fixed)", 0, 0);
    g_original = 1;
    snap();
    f75CA8(0x1D, NA(1, 0));             /* --original stand-in index 0: [0x1B 7], slot 1 = 0 / 0xFF */
    E8(NA(1, 0x14), 0x1B); E8(NA(1, 0x1C), 7); E8(NA(1, 0x15), 0); E8(NA(1, 0x1D), 0xFF); E8(NA(1, 0x13), 1);
    verify("75CA8 calamity not held --original", 0, 0);
    set8(NA(1, 0x13), 0); set8(NA(1, 0x14), 0x1B); set8(NA(1, 0x1C), 7);
    snap();
    f75CA8(0x1B, NA(1, 0));             /* --original, none held: count -> 0xFF; 0x75D68 clears +0x14 - 1 = the count
                                           (0 again); 0x75D78 reads 0: +0x1C = 0xFF; +0x14 kept */
    E8(NA(1, 0x1C), 0xFF);
    verify("75CA8 --original, none held: the count clears itself", 0, 0);
    g_original = 0;

    clear_records();
    set8(NA(9, 0x3D), 9); set8(NA(4, 0x13), 1); set8(NA(4, 0x14), 0x14); set8(NA(4, 0x1C), 2);
    snap();
    f75D84(0x16, NA(4, 0), NA(9, 0));
    E8(NA(4, 0x15), 0x16); E8(NA(4, 0x1D), 9); E8(NA(4, 0x13), 2);
    verify("75D84 calamity with giver", 0, 0);
    snap();
    f75D84(18, NA(4, 0), NA(9, 0));
    E8(NA(4, 18), 1);
    verify("75D84 commodity 18", 0, 0);
    snap();
    f75D84(0x200, NA(4, 0), NA(9, 0));
    verify("75D84 card 0 ignored", 0, 0);
    set8(NA(4, 0x13), 0xFF);            /* count -1: the card lands on +0x13 (the count itself), then count + 1 */
    snap();
    f75D84(0x1C, NA(4, 0), NA(9, 0));  /* the second store reads the count again: 0x1C -> giver at +0x1C + 28 */
    E8(NA(4, 0x13), 0x1D); E8(NA(4, 0x38), 9);
    verify("75D84 negative count, count read again", 0, 0);

    /* f75CA8 on a record that is not a nation slot of the table (the trade code passes 0xF14CC, record 0) */
    clear_records();
    set8(NA(0, 0x13), 1); set8(NA(0, 0x14), 0x19); set8(NA(0, 0x1C), 3);
    snap();
    f75CA8(0x19, NA(0, 0));
    E8(NA(0, 0x14), 0); E8(NA(0, 0x1C), 0xFF); E8(NA(0, 0x13), 0);
    verify("75CA8 record 0, last calamity", 0, 0);
}

/* ==== B. random differential run ============================================================================== */

/* the model: a plain copy of the data object, addressed by link address */
static uint8_t *m_ds;
#define M(a) m_ds[(uint32_t)(a) - DS_BASE]
#define MA(t, off) M(AR(t, off))
#define MN(n, off) M(NA(n, off))

static int ms8(uint32_t a) { return (int8_t)M(a); }
static int ms16(uint32_t a) { return (int16_t)(uint16_t)(M(a) | M(a + 1) << 8); }
static void mw16(uint32_t a, int v) { M(a) = (uint8_t)v; M(a + 1) = (uint8_t)((unsigned)v >> 8); }

/* the three move.c primitives, from their semantics */
static void m_581C4(int t, int n)
{
    int j, cnt;
    if (MA(t, 1 + n) == 0 || ms8(AR(t, 0x0E + n)) != 0)
        return;
    MA(t, 1)--;
    MA(t, 1 + n) = 0;
    for (j = 0; j < ms8(NA(n, 0x60)); j++) {
        if (MN(n, 0x61 + j) != t + 1)
            continue;
        cnt = ms8(NA(n, 0x60));
        if (cnt - 1 - j > 0)
            memmove(&MN(n, 0x61 + j), &MN(n, 0x62 + j), (size_t)(cnt - 1 - j));
        MN(n, 0x60)--;
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
    if (ms8(AR(t, 1 + n)) != n) {
        MA(t, 1)++;
        MA(t, 1 + n) = (uint8_t)n;
    }
    MA(t, 0x0E + n) = (uint8_t)(MA(t, 0x0E + n) + (k & 0xFF));
    cnt = ms8(NA(n, 0x60));
    for (j = 0; j < cnt && !there; j++)
        there = MN(n, 0x61 + j) == t + 1;
    if (!there) {
        M(NA(n, 0x61) + (uint32_t)cnt) = (uint8_t)(t + 1);
        MN(n, 0x60)++;
    }
}

static void m_576F8(int t)
{
    int n = (int)(M(0xF40F8u) | M(0xF40F9u) << 8 | M(0xF40FAu) << 16 | (uint32_t)M(0xF40FBu) << 24);
    int step;
    for (step = 0; step < 5; step++) {
        if (ms16(NA(n, 0x45 + 2 * MA(t, 0x2D + n))) == t + 1)
            return;
        MA(t, 0x2D + n) = (uint8_t)(((MA(t, 0x2D + n) + 1) & 0xFF) % 4);
    }
}

/* the 16 routines */
static int m_limit(int t) { return (M(TR(t, 1)) >> 3) & 7; }

static int m_78D08(int t, int n)
{
    int lim = m_limit(t);
    if (MN(n, 0x27) != 0 && ms8(AR(t, 1)) == 1 && ms8(AR(t, 1 + n)) == n)
        lim++;
    return lim;
}

static void m_78E74(int t, int n)
{
    M(TS(t, 2)) |= 1;
    M(TS(t, 0)) = (uint8_t)n;
    M(TS(t, 1)) = M(0xF40FCu);
    M(NA(n, 0x52) + (uint32_t)ms8(NA(n, 0x51))) = (uint8_t)(t + 1);
    MN(n, 0x51)++;
}

static void m_78F04(int t, int n)
{
    int cnt = ms8(NA(n, 0x51)), j, at = -1;
    M(TS(t, 2)) &= 0xFE;
    M(TS(t, 0)) = 0;
    M(TS(t, 1)) = 0xFF;
    for (j = 0; j < cnt; j++)
        if (MN(n, 0x52 + j) == t + 1) {
            at = j;
            break;
        }
    if (at < 0) {
        if (!g_original)
            return;
        at = 0;
    }
    for (j = at; j < cnt - 1; j++)
        MN(n, 0x52 + j) = MN(n, 0x53 + j);
    MN(n, 0x51)--;
    M(NA(n, 0x52) + (uint32_t)ms8(NA(n, 0x51))) = 0;
}

static void m_78FEC(int t, int n)
{
    int lim, k, j, cnt;
    m_78F04(t, n);
    lim = m_limit(t);
    cnt = ms8(AR(t, 1));
    if (cnt == 0 || (cnt == 1 && ms8(AR(t, 0x0E + n)) != 0))
        lim += ms8(NA(n, 0x27));
    k = ms8(NA(n, 0x99));
    if (lim < k)
        k = lim;
    for (j = 0; j < k; j++)
        m_57E28(t, n, 1);
    MN(n, 0x99) = (uint8_t)(MN(n, 0x99) - k);
    MN(n, 0x98) = (uint8_t)(MN(n, 0x98) + k);
}

static int m_7C760(int t)
{
    int m;
    for (m = 1; m <= 11; m++)
        if (ms8(AR(t, 1 + m)) == m)
            return m;
    return 0;
}

static int m_7C7C4(int t, int n)
{
    int m;
    for (m = 1; m <= 11; m++)
        if (m != n && ms8(AR(t, 1 + m)) == m)
            return m;
    return 0;
}

static int m_789EC(int n) { return !(n == 4 || n == 5 || n == 8); }

static void m_78D98(int k, int n)
{
    int v = ms16(NA(n, 0x45 + 2 * k)), j;
    for (j = k; j < 3; j++)
        mw16(NA(n, 0x45 + 2 * j), ms16(NA(n, 0x47 + 2 * j)));
    MN(n, 0x40)--;
    M(AREA_BASE + 69u * (uint32_t)v + 0x1Bu + (uint32_t)n)--;
    mw16(NA(n, 0x4B), 0);
    for (j = 0; j < MN(n, 0x40); j++) {
        M(0xF40F8u) = (uint8_t)n; M(0xF40F9u) = (uint8_t)(n >> 8); M(0xF40FAu) = 0; M(0xF40FBu) = 0;
        m_576F8(ms16(NA(n, 0x45 + 2 * j)) - 1);
    }
}

static void m_57CA4(int a, int b, int n, int flag, int k)
{
    m_57CE8(a, n, flag, k);
    m_57E28(b, n, k);
}

static void m_460C0(int n)
{
    int t;
    for (t = 0; t < 160; t++)
        MA(t, 0x24 + n) = MA(t, 1 + n) != 0 ? MA(t, 0x0E + n) : 0;
}

static void m_46158(int n)
{
    int k;
    for (k = 0; k < MN(n, 0x40); k++)
        MN(n, 0x41 + k) = MN(n, 0x25) ? 5 : 4;
}

static int m_76E50(int s, int n)
{
    int cnt = ms16(ST(s, 0)), c;
    if (cnt == 0)
        return 0;
    mw16(ST(s, 0), cnt - 1);
    c = ms8(ST(s, 6) + (uint32_t)(cnt - 1));
    if (c == 0)
        return 0;
    if (c > 18) {
        int old = ms8(NA(n, 0x13));         /* the count is stored first, then the card (it may land on the count) */
        MN(n, 0x13)++;
        M(NA(n, 0x14) + (uint32_t)old) = (uint8_t)c;
    } else {
        M(NA(n, 0) + (uint32_t)c)++;
    }
    return c;
}

static void m_771F8(int c)
{
    static const int calpile[12] = { 1, 1, 2, 2, 3, 3, 4, 4, 5, 6, 7, 8 };
    int s, cnt;
    if (c <= 18)
        s = c >= 1 ? (c - 1) / 2 : -((1 - c) / 2);      /* C99 truncation written out */
    else
        s = (c <= 0x1E) ? calpile[c - 0x13] : ms8(0xCA9C1u + (uint32_t)c);
    cnt = ms16(PI(s, 0));
    mw16(PI(s, 0), cnt + 1);
    M(PI(s, 6) + (uint32_t)cnt) = (uint8_t)c;
}

static void m_6E460(int n)
{
    int cnt = ms8(NA(n, 0x13)), pass, j;
    uint8_t x;
    /* bubble passes from the end, as many as the routine makes (count - 1), swapping on a strict signed > */
    for (pass = 1; pass < cnt; pass++) {
        for (j = cnt - 1; j >= pass; j--) {
            if ((int8_t)MN(n, 0x13 + j) > (int8_t)MN(n, 0x14 + j)) {
                x = MN(n, 0x13 + j); MN(n, 0x13 + j) = MN(n, 0x14 + j); MN(n, 0x14 + j) = x;
                x = MN(n, 0x1B + j); MN(n, 0x1B + j) = MN(n, 0x1C + j); MN(n, 0x1C + j) = x;
            }
        }
    }
}

static void m_75CA8(int c, uint32_t rec)
{
    int8_t cb = (int8_t)c;
    int cnt, j, at = -1;
    if (cb == 0)
        return;
    if (cb <= 18) {
        M(rec + (uint32_t)(int)cb)--;
        return;
    }
    cnt = ms8(rec + 0x13);
    for (j = 0; j < cnt; j++)
        if ((int8_t)M(rec + 0x14 + (uint32_t)j) == cb) {
            at = j;
            break;
        }
    if (at < 0) {
        if (!g_original)
            return;
        at = 0;
    }
    for (j = at; j < cnt - 1; j++) {
        M(rec + 0x14 + (uint32_t)j) = M(rec + 0x15 + (uint32_t)j);
        M(rec + 0x1C + (uint32_t)j) = M(rec + 0x1D + (uint32_t)j);
    }
    M(rec + 0x13)--;
    M(rec + 0x14 + (uint32_t)ms8(rec + 0x13)) = 0;
    M(rec + 0x1C + (uint32_t)ms8(rec + 0x13)) = 0xFF;
}

static void m_75D84(int c, uint32_t rec, uint32_t giver)
{
    int8_t cb = (int8_t)c;
    if (cb == 0)
        return;
    if (cb <= 18) {
        M(rec + (uint32_t)(int)cb)++;
        return;
    }
    M(rec + 0x14 + (uint32_t)ms8(rec + 0x13)) = (uint8_t)cb;
    M(rec + 0x1C + (uint32_t)ms8(rec + 0x13)) = M(giver + 0x3D);
    M(rec + 0x13)++;
}

static uint32_t g_rng = 12345;
static int rnd(int n)
{
    g_rng = g_rng * 1103515245u + 12345u;
    return (int)((g_rng >> 16) % (uint32_t)n);
}

/* random records within the ranges the operations below keep inside the tables */
static void randomize(void)
{
    uint32_t i;
    int n, t, s;

    for (i = 0; i < AREA_LEN; i++)
        g_ds[AREA_BASE + i - DS_BASE] = (uint8_t)(rnd(4) == 0 ? rnd(256) : rnd(4));
    for (t = 0; t < 12; t++)                           /* markers as the game keeps them, sometimes */
        for (n = 1; n <= 11; n++)
            if (rnd(3) == 0)
                setD8(AR(t, 1 + n), (uint8_t)(rnd(2) ? n : 0));
    for (i = 0; i < NAT_LEN; i++)
        g_ds[NAT_BASE + i - DS_BASE] = (uint8_t)(rnd(4) == 0 ? rnd(256) : rnd(9));
    for (n = 0; n < 14; n++) {
        setD8(NA(n, 0x40), (uint8_t)rnd(5));
        for (i = 0; i < 4; i++)                         /* ship words: territory + 1 in 0..12, high byte 0 */
            setD16(NA(n, 0x45 + 2 * i), (uint16_t)rnd(13));
        setD8(NA(n, 0x51), (uint8_t)(rnd(8) == 0 ? 0xFC + rnd(4) : rnd(10)));   /* city count, sometimes < 0 */
        for (i = 0; i < 10; i++)
            setD8(NA(n, 0x52 + i), (uint8_t)rnd(13));
        setD8(NA(n, 0x13), (uint8_t)(rnd(8) == 0 ? 0xFC + rnd(4) : rnd(9)));   /* calamity count */
        for (i = 0; i < 8; i++)
            setD8(NA(n, 0x14 + i), (uint8_t)(rnd(6) == 0 ? rnd(256) : 0x13 + rnd(12)));
        setD8(NA(n, 0x60), (uint8_t)rnd(8));
        setD8(NA(n, 0x99), (uint8_t)(rnd(6) == 0 ? rnd(256) : rnd(30)));
        setD8(NA(n, 0x27), (uint8_t)(rnd(4) == 0 ? rnd(256) : rnd(2)));
        setD8(NA(n, 0x25), (uint8_t)(rnd(2) ? 0 : rnd(256)));
    }
    for (i = 0; i < TS_LEN; i++)
        g_ds[TS_BASE + i - DS_BASE] = (uint8_t)rnd(256);
    for (t = 0; t < 160; t++)
        setD8(TR(t, 1), (uint8_t)rnd(256));
    for (i = 0; i < CARD_LEN; i++)
        g_ds[CARD_BASE + i - DS_BASE] = (uint8_t)(rnd(3) == 0 ? rnd(256) : 1 + rnd(0x1E));
    for (s = 0; s < 9; s++) {
        setD16(ST(s, 0), (uint16_t)(rnd(8) == 0 && s > 0 ? -rnd(4) : rnd(20)));
        setD16(PI(s, 0), (uint16_t)(rnd(8) == 0 && s > 0 ? -rnd(4) : rnd(12)));
    }
    setD8(0xF40FCu, (uint8_t)rnd(256));
    memcpy(m_ds, g_ds, DS_SIZE);
}

static int tables_differ(void)
{
    static const uint32_t r[][2] = { { AREA_BASE, AREA_LEN }, { NAT_BASE, NAT_LEN }, { TS_BASE, TS_LEN },
                                     { CARD_BASE, CARD_LEN }, { 0xF40F8u, 8 } };
    size_t i;
    for (i = 0; i < sizeof r / sizeof r[0]; i++)
        if (memcmp(DP(r[i][0]), &M(r[i][0]), r[i][1]) != 0)
            return 1;
    return 0;
}

static void part_b(long ops, long *done, long *bad)
{
    long i;
    uint32_t a;
    int t, n, op, r1, r2, skip;

    randomize();
    snap();
    for (i = 0; i < ops; i++) {
        if (i % 1000 == 999)
            randomize();
        t = rnd(12);
        n = 1 + rnd(9);
        op = rnd(16);
        r1 = r2 = skip = 0;
        switch (op) {
        case 0: { int tt = rnd(160); r1 = m_78D08(tt, n); r2 = f78D08(tt, n); break; }
        case 1: m_78E74(t, n); f78E74(t, n); break;
        case 2: m_78F04(t, n); f78F04(t, n); break;
        case 3: m_78FEC(t, n); f78FEC(t, n); break;
        case 4: { int tt = rnd(161) - 1; r1 = m_7C760(tt); r2 = f7C760(tt); break; }
        case 5: { int tt = rnd(161) - 1, nn = rnd(14); r1 = m_7C7C4(tt, nn); r2 = f7C7C4(tt, nn); break; }
        case 6: { int x = rnd(20) - 3; r1 = m_789EC(x); r2 = f789EC(x); break; }
        case 7:
            {   /* a ship count of 0 (wraps to 255) or over 4, or a ship word that is not territory + 1, is not a game
                   state: the routine would index the area table with it */
                int cnt = D8(NA(n, 0x40)), j, ok = cnt >= 1 && cnt <= 4, k;
                for (j = 0; j < 4; j++)
                    ok = ok && S16(NA(n, 0x45 + 2 * j)) >= 0 && S16(NA(n, 0x45 + 2 * j)) <= 160;
                if (!ok) {
                    skip = 1;
                    break;
                }
                k = rnd(cnt);
                m_78D98(k, n); f78D98(k, n);
            }
            break;
        case 8: { int b = rnd(12), f = rnd(3), k = rnd(6) - 1; m_57CA4(t, b, n, f, k); f57CA4(t, b, n, f, k); break; }
        case 9: { int nn = rnd(14); m_460C0(nn); f460C0(nn); break; }
        case 10: m_46158(n); f46158(n); break;
        case 11: { int s = rnd(9); r1 = m_76E50(s, n); r2 = f76E50(s, n); break; }
        case 12: {
            int c = rnd(34) - 1, s, cnt;
            if (c > 0x1E)
                c = -rnd(12);                           /* piles -6..-1 lie inside the stacks */
            s = c >= 1 ? (c - 1) / 2 : -((1 - c) / 2);
            if (c > 18)
                s = (int8_t)D8(0xCA9C1u + (uint32_t)c);
            cnt = (int16_t)D16(PI(s, 0));
            if (cnt < -4 || cnt > 40) {     /* a random word there: the card would land outside */
                skip = 1;
                break;
            }
            m_771F8(c); f771F8(c);
            break;
        }
        case 13: m_6E460(n); f6E460(n); break;
        case 14: {
            int c = rnd(4) == 0 ? rnd(512) : 0x13 + rnd(12);
            m_75CA8(c, NA(n, 0)); f75CA8(c, NA(n, 0));
            break;
        }
        default: {
            int c = rnd(4) == 0 ? rnd(512) : 0x13 + rnd(12), g = rnd(14);
            m_75D84(c, NA(n, 0), NA(g, 0)); f75D84(c, NA(n, 0), NA(g, 0));
            break;
        }
        }
        if (skip)
            continue;
        (*done)++;
        if (r1 != r2 || tables_differ()) {
            (*bad)++;
            if (*bad <= 5)
                printf("FAIL random op %ld: routine %d t %d n %d, returns %d / model %d\n", i, op, t, n, r2, r1);
            memcpy(m_ds, g_ds, DS_SIZE);                /* resync and go on */
        }
    }
    /* the whole data object agrees with the model, and nothing outside the tables changed */
    if (memcmp(m_ds, g_ds, DS_SIZE) != 0) {
        (*bad)++;
        printf("FAIL random run: the data objects differ outside the compared tables\n");
    }
    for (a = DS_BASE; a < DS_BASE + DS_SIZE; a++) {
        if ((a >= AREA_BASE && a < AREA_BASE + AREA_LEN) || (a >= NAT_BASE && a < NAT_BASE + NAT_LEN) ||
            (a >= TS_BASE && a < TS_BASE + TS_LEN) || (a >= CARD_BASE && a < CARD_BASE + CARD_LEN) ||
            (a >= 0xF40F8u && a < 0xF4100u) || (a >= TERR_BASE && a < TERR_BASE + 160u * 35u))
            continue;
        if (g_ds[a - DS_BASE] != g_before[a - DS_BASE]) {
            (*bad)++;
            printf("FAIL random run wrote 0x%05X outside the tables\n", (unsigned)a);
            break;
        }
    }
    setD32(0xF40F8u, 0);
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:/work/advciv_run";
    long done = 0, bad = 0, done2 = 0, bad2 = 0;

    if (state_init(gamedir) != 0)
        return 1;
    g_before = malloc(DS_SIZE);
    g_want = malloc(DS_SIZE);
    m_ds = malloc(DS_SIZE);
    if (g_before == NULL || g_want == NULL || m_ds == NULL)
        return 1;

    part_a();
    printf("A. hand vectors: %d of %d pass\n", g_tests - g_fail, g_tests);

    part_b(200000, &done, &bad);
    printf("B. random differential run: %ld operations, %ld disagree\n", done, bad);
    g_original = 1;
    part_b(100000, &done2, &bad2);
    g_original = 0;
    printf("B. random differential run, --original: %ld operations, %ld disagree\n", done2, bad2);
    if (g_stub_calls != 0)
        printf("FAIL %d unexpected stub calls\n", g_stub_calls);

    return g_fail == 0 && bad == 0 && bad2 == 0 && g_stub_calls == 0 ? 0 : 1;
}
