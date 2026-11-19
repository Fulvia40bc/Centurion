/* helpers2_test [VECTORS.txt]: runs the HELPERS2 module's 14 routines (helpers2.c, TASK 16) on vectors, with the
   shared runner vecrun.c (vecrun.h has the vector-file format and the checks).

   It does not need the game folder and never calls state_init: g_ds = calloc(DS_SIZE + 0x1000, 1) here, and
   g_original stays 0 (f75C08 calls rules.c's f75CA8, which has the 1995 fix).
   Link: helpers2_test.c vecrun.c helpers2.c rules.c move.c mapdata.c load.c state.c le_image.c (the stub block
   below is rules_test.c's; none of it is reached).

   Argument conventions in the vector file: as TASK 15's decimal int32 args, except f75C08's args are record link
   addresses (passed as (uint32_t)arg) and the comparators' args are element link addresses (0x108820 + 16i,
   passed as DP((uint32_t)arg)). R void for f75C08, f75DF0, f75EB4, f75F08, f63950.

   The hand cases below (at least 3 per routine, worked out by hand from the TASK 16 text, the named edge cases
   included) always run, with or without a vector file; no vector file is kept in the repo.

   Build: clang -std=c11 -Wall -Wextra -Werror -pedantic port/test/helpers2_test.c port/test/vecrun.c port/src/helpers2.c port/src/rules.c port/src/move.c port/src/mapdata.c port/src/load.c port/src/state.c port/src/le_image.c -Iport/src -o helpers2_test.exe
   Run:   helpers2_test [VECTORS.txt] */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/state.h"
#include "../src/game.h"
#include "../src/move.h"
#include "../src/rules.h"
#include "../src/helpers2.h"
#include "vecrun.h"

/* ---- stubs for what move.c and the loaders reference (none is reached by the HELPERS2 routines) ---------------- */

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
_Noreturn void f44CAC(void) { fprintf(stderr, "helpers2_test: quit\n"); exit(2); }

/* ---- the function table for vecrun ------------------------------------------------------------------------------ */

static const char *const FUNCS[] = {
    "f75BD4", "f75C08", "f75DF0", "f75EB4", "f75F08", "f63950", "f63AF0", "f63B7C", "f63C6C", "f2EC38",
    "f2C778", "f3530C", "f35368", "f65C4C"
};
#define NFUNC 14
static const int FUNC_NARGS[NFUNC] = { 1, 2, 1, 2, 2, 1, 1, 1, 1, 2, 1, 2, 2, 2 };

int vec_func_count(void) { return NFUNC; }
const char *vec_func_name(int id) { return FUNCS[id]; }
int vec_func_nargs(int id) { return FUNC_NARGS[id]; }

int vec_func_id(const char *name)
{
    int i;
    for (i = 0; i < NFUNC; i++)
        if (strcmp(name, FUNCS[i]) == 0)
            return i;
    return -1;
}

/* call the function; returns 1 when it returns void */
int vec_call(int id, int a, int b, int *ret)
{
    switch (id) {
    case 0:  *ret = f75BD4(a); return 0;
    case 1:  f75C08((uint32_t)a, (uint32_t)b); return 1;
    case 2:  f75DF0(a); return 1;
    case 3:  f75EB4(a, b); return 1;
    case 4:  f75F08(a, b); return 1;
    case 5:  f63950(a); return 1;
    case 6:  *ret = f63AF0(a); return 0;
    case 7:  *ret = f63B7C(a); return 0;
    case 8:  *ret = f63C6C(a); return 0;
    case 9:  *ret = f2EC38(a, b); return 0;
    case 10: *ret = f2C778(a); return 0;
    case 11: *ret = f3530C(DP((uint32_t)a), DP((uint32_t)b)); return 0;
    case 12: *ret = f35368(DP((uint32_t)a), DP((uint32_t)b)); return 0;
    default: *ret = f65C4C(DP((uint32_t)a), DP((uint32_t)b)); return 0;
    }
}

/* ---- hand-case builders --------------------------------------------------------------------------------------- */

static vec_t hv(const char *func, int a, int b)
{
    vec_t v;
    memset(&v, 0, sizeof v);
    snprintf(v.func, sizeof v.func, "%s", func);
    snprintf(v.tline, sizeof v.tline, "T %s %d %d", func, a, b);
    v.a = a;
    v.b = b;
    v.has_ret = 1;
    return v;
}

static void hm(vec_t *v, uint32_t addr, const uint8_t *b, int n)
{
    vec_mem *m = &v->m[v->nm++];
    m->addr = addr;
    m->len = n;
    memcpy(m->b, b, (size_t)n);
}

static void hc(vec_t *v, uint32_t addr, const uint8_t *b, int n)
{
    vec_mem *c = &v->c[v->nc++];
    c->addr = addr;
    c->len = n;
    memcpy(c->b, b, (size_t)n);
}

#define NA(n, off) (0xF14CCu + 199u * (uint32_t)(n) + (uint32_t)(off))   /* field off of nation n's record */
#define REC0 0xF14CCu
#define REC1 (0xF14CCu + 199u)      /* nation 1 */
#define REC2 (0xF14CCu + 398u)      /* nation 2 */
#define R12  (0xF14CCu + 199u * 12u)   /* nation 12 */
#define R13  (0xF14CCu + 199u * 13u)   /* nation 13 */
#define EL(i) (0x108820u + 16u * (uint32_t)(i))   /* ship-route element i */
#define B(...) ((const uint8_t[]){ __VA_ARGS__ })

/* ---- hand cases (worked out by hand from the TASK 16 text) ------------------------------------------------------ */

static void hand_cases(void)
{
    vec_t v;

    /* f75BD4: (int8_t)c <= 0x12 -> 0; else S8(0xC95C5 + c) from the data object */
    v = hv("f75BD4", 0x12, 0); v.ret = 0; vec_hand_vector(&v);
    v = hv("f75BD4", -5, 0); v.ret = 0; vec_hand_vector(&v);
    v = hv("f75BD4", 0x13, 0); hm(&v, 0xC95C8u, B(0), 1); v.ret = 0; vec_hand_vector(&v);
    v = hv("f75BD4", 0x114, 0);                       /* int8 0x14: reads 0xC95D9 */
    hm(&v, 0xC95D9u, B(1), 1); v.ret = 1; vec_hand_vector(&v);
    v = hv("f75BD4", 0x7F, 0);                        /* c = 127 reads past the calamities: 0xC9644 */
    hm(&v, 0xC9644u, B(0x10), 1); v.ret = 16; vec_hand_vector(&v);

    /* f75C08: swap the offers; a, b are record addresses */
    v = hv("f75C08", REC0, REC1);
    hm(&v, 0x112570u, B(1, 0, 0, 0, 0), 5);           /* side 0 gives card 1 */
    hm(&v, 0x112579u, B(2, 0, 0, 0, 0), 5);           /* side 1 gives card 2 */
    hm(&v, REC0 + 1u, B(2), 1); hm(&v, REC1 + 2u, B(5), 1);
    v.has_ret = 0;
    hc(&v, REC0 + 1u, B(1), 1); hc(&v, REC0 + 2u, B(1), 1);
    hc(&v, REC1 + 1u, B(1), 1); hc(&v, REC1 + 2u, B(4), 1);
    vec_hand_vector(&v);
    /* a calamity the giver does not hold: that remove changes nothing (g_original = 0) */
    v = hv("f75C08", REC0, REC1);
    hm(&v, 0x112570u, B(0x14, 0, 0, 0, 0), 5);
    hm(&v, 0x112579u, B(0x15, 0, 0, 0, 0), 5);
    hm(&v, REC0 + 0x13u, B(1), 1); hm(&v, REC0 + 0x14u, B(0x14), 1);   /* a holds calamity 0x14 */
    hm(&v, REC0 + 0x3Du, B(7), 1); hm(&v, REC1 + 0x3Du, B(9), 1);
    v.has_ret = 0;
    hc(&v, REC0 + 0x13u, B(1), 1); hc(&v, REC0 + 0x14u, B(0x15), 1); hc(&v, REC0 + 0x1Cu, B(9), 1);
    hc(&v, REC1 + 0x13u, B(1), 1); hc(&v, REC1 + 0x14u, B(0x14), 1); hc(&v, REC1 + 0x1Cu, B(7), 1);
    vec_hand_vector(&v);
    v = hv("f75C08", REC0, REC1);                     /* no cards offered: nothing changes */
    v.has_ret = 0;
    vec_hand_vector(&v);

    /* f75DF0: clear the 13 offer bytes at rec + 0x9C + S8(0xCA9A0), delete rows with +0x0D == n, same i retested */
    v = hv("f75DF0", 1, 0);
    hm(&v, NA(1, 0x9C), B(0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA), 13);
    hm(&v, 0x112548u, B(3, 0), 2);                    /* 3 rows */
    hm(&v, 0x111118u, B(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 1, 14, 15), 16);   /* row 0: +0x0D = 1 */
    hm(&v, 0x111128u, B(21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 1, 34, 35), 16); /* row 1: +0x0D = 1 */
    hm(&v, 0x111138u, B(41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 2, 54, 55), 16); /* row 2: +0x0D = 2 */
    v.has_ret = 0;
    hc(&v, NA(1, 0x9C), B(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), 13);
    hc(&v, 0x112548u, B(1, 0), 2);
    hc(&v, 0x111118u, B(41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 2, 54, 55), 16); /* row 0 = old row 2 */
    hc(&v, 0x111128u, B(41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 2, 54, 55), 16); /* row 1 = old row 2 too
                                                                                                  (the shift left it) */
    vec_hand_vector(&v);                             /* two matching rows next to each other */
    v = hv("f75DF0", 1, 0);                           /* S8(0xCA9A0) = -2: p = rec + 0x9A */
    hm(&v, 0xCA9A0u, B(0xFE), 1);
    hm(&v, NA(1, 0x9A), B(0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB), 15);
    hm(&v, 0x112548u, B(0, 0), 2);
    v.has_ret = 0;
    hc(&v, NA(1, 0x9A), B(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), 13);   /* 0x9A..0xA6 cleared, 0xA7/0xA8 keep */
    vec_hand_vector(&v);
    v = hv("f75DF0", 1, 0);                           /* count -1: no row runs, the offer is still cleared */
    hm(&v, 0x112548u, B(0xFF, 0xFF), 2);
    hm(&v, NA(1, 0x9C), B(0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC), 13);
    v.has_ret = 0;
    hc(&v, NA(1, 0x9C), B(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), 13);
    vec_hand_vector(&v);

    /* f75EB4: f75DF0(a) when a's offer is non-empty, then f75DF0(b) when b's is (read after the first call) */
    v = hv("f75EB4", 1, 2);
    hm(&v, NA(1, 0x9C), B(1), 1);                     /* a has an offer */
    hm(&v, 0x112548u, B(1, 0), 2);
    hm(&v, 0x111118u, B(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0), 16);
    v.has_ret = 0;
    hc(&v, NA(1, 0x9C), B(0), 1);
    hc(&v, 0x112548u, B(0, 0), 2);
    vec_hand_vector(&v);
    v = hv("f75EB4", 1, 1);                           /* a == b: the first call cleared it, no second */
    hm(&v, NA(1, 0x9C), B(1), 1);
    hm(&v, 0x112548u, B(1, 0), 2);
    hm(&v, 0x111118u, B(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0), 16);
    v.has_ret = 0;
    hc(&v, NA(1, 0x9C), B(0), 1);
    hc(&v, 0x112548u, B(0, 0), 2);
    vec_hand_vector(&v);
    v = hv("f75EB4", 1, 2);                           /* neither has an offer: nothing */
    v.has_ret = 0;
    vec_hand_vector(&v);

    /* f75F08: delete rows whose +0x0E is a or b */
    v = hv("f75F08", 1, 2);
    hm(&v, 0x112548u, B(3, 0), 2);
    hm(&v, 0x111118u, B(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 1, 0), 16);   /* e = 1 */
    hm(&v, 0x111128u, B(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 2, 0), 16);   /* e = 2 */
    hm(&v, 0x111138u, B(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 3, 0), 16);   /* e = 3 */
    v.has_ret = 0;
    hc(&v, 0x112548u, B(1, 0), 2);
    hc(&v, 0x111118u, B(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 3, 0), 16);
    hc(&v, 0x111128u, B(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 3, 0), 16);   /* stale row 1 = old row 2 */
    vec_hand_vector(&v);                             /* matches in a row */
    v = hv("f75F08", 1, 2);
    hm(&v, 0x112548u, B(1, 0), 2);
    hm(&v, 0x111118u, B(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 3, 0), 16);
    v.has_ret = 0;                                    /* no match: nothing */
    vec_hand_vector(&v);
    v = hv("f75F08", 1, 1);
    hm(&v, 0x112548u, B(1, 0), 2);
    hm(&v, 0x111118u, B(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 1, 0), 16);
    v.has_ret = 0;
    hc(&v, 0x112548u, B(0, 0), 2);
    vec_hand_vector(&v);

    /* f63950: coastal territories (terr byte & 2) with v = S8(0xE31B8 + 69(t+1) + 0x0E + n) > 1 are listed */
    v = hv("f63950", 1, 0);
    hm(&v, 0xE1638u, B(0x02), 1);                     /* t=0 coastal */
    hm(&v, 0xE31B8u + 69u + 0x0Eu + 1u, B(1), 1);      /* v = 1: not listed */
    hm(&v, 0xE1638u + 35u * 159u, B(0x02), 1);        /* t=159 coastal */
    hm(&v, 0xE31B8u + 69u * 160u + 0x0Eu + 1u, B(5), 1);   /* v = 5: listed */
    v.has_ret = 0;
    hc(&v, 0x10F30Cu, B(1, 0, 0, 0), 4);
    hc(&v, 0x10F310u, B(159, 0, 0, 0), 4);
    vec_hand_vector(&v);
    v = hv("f63950", 1, 0);                           /* nothing coastal: count 0 */
    v.has_ret = 0;
    hc(&v, 0x10F30Cu, B(0, 0, 0, 0), 4);
    vec_hand_vector(&v);
    v = hv("f63950", 2, 0);                           /* two listed, in t order */
    hm(&v, 0xE1638u + 35u * 3u, B(0x02), 1);
    hm(&v, 0xE1638u + 35u * 7u, B(0x06), 1);
    hm(&v, 0xE31B8u + 69u * 4u + 0x0Eu + 2u, B(2), 1);
    hm(&v, 0xE31B8u + 69u * 8u + 0x0Eu + 2u, B(9), 1);
    v.has_ret = 0;
    hc(&v, 0x10F30Cu, B(2, 0, 0, 0), 4);
    hc(&v, 0x10F310u, B(3, 0, 0, 0, 7, 0, 0, 0), 8);
    vec_hand_vector(&v);

    /* f63AF0: ships at 2 treasury each, slots and s = S8(rec+0x3E), lim 18 (or 8 with variant bit 0x20) */
    v = hv("f63AF0", 1, 0);
    hm(&v, 0x110148u, B(3, 0, 0, 0), 4); hm(&v, NA(1, 0x3E), B(10), 1);
    v.ret = 3;                                        /* 10 -> 8 -> 6, slots run out */
    hc(&v, 0x110148u, B(0, 0, 0, 0), 4);
    vec_hand_vector(&v);
    v = hv("f63AF0", 1, 0);                           /* variant bit: lim 8, s = 7 -> 7,5,3 then s <= 1 */
    hm(&v, 0xE1624u, B(0x20), 1);
    hm(&v, 0x110148u, B(5, 0, 0, 0), 4); hm(&v, NA(1, 0x3E), B(7), 1);
    v.ret = 3;
    hc(&v, 0x110148u, B(2, 0, 0, 0), 4);
    vec_hand_vector(&v);
    v = hv("f63AF0", 1, 0);                           /* s = 8 with lim 8: 8 >= 8 and 6 <= 8 -> stop at once */
    hm(&v, 0xE1624u, B(0x20), 1);
    hm(&v, 0x110148u, B(5, 0, 0, 0), 4); hm(&v, NA(1, 0x3E), B(8), 1);
    v.ret = 0;
    vec_hand_vector(&v);
    v = hv("f63AF0", 1, 0);                           /* no slots */
    hm(&v, 0x110148u, B(0, 0, 0, 0), 4); hm(&v, NA(1, 0x3E), B(10), 1);
    v.ret = 0;
    vec_hand_vector(&v);

    /* f63B7C */
    v = hv("f63B7C", 1, 0);                           /* s=127, x=0x40000040: v = INT_MAX, v+1 wraps -> 0 */
    hm(&v, NA(1, 0x3E), B(0x7F), 1);
    hm(&v, 0x11014Cu, B(0x40, 0, 0, 0x40), 4);
    hm(&v, 0x110148u, B(1, 0, 0, 0), 4);
    hm(&v, 0xE31B8u + 69u + 0x0Eu + 1u, B(2), 1);     /* area 1 holds 2 tokens: k = 1 */
    v.ret = 0;
    vec_hand_vector(&v);
    v = hv("f63B7C", 1, 0);                           /* tokens only in area 158: not counted -> 0 */
    hm(&v, NA(1, 0x3E), B(10), 1);
    hm(&v, 0x11014Cu, B(0, 0, 0, 0), 4);
    hm(&v, 0x110148u, B(2, 0, 0, 0), 4);
    hm(&v, 0xE31B8u + 69u * 158u + 0x0Eu + 1u, B(5), 1);
    v.ret = 0;
    vec_hand_vector(&v);
    v = hv("f63B7C", 1, 0);                           /* v = 10, area 1 has 2 tokens: buys, slot dec */
    hm(&v, NA(1, 0x3E), B(10), 1);
    hm(&v, 0x11014Cu, B(0, 0, 0, 0), 4);
    hm(&v, 0x110148u, B(2, 0, 0, 0), 4);
    hm(&v, 0xE31B8u + 69u + 0x0Eu + 1u, B(2), 1);
    v.ret = 1;
    hc(&v, 0x110148u, B(1, 0, 0, 0), 4);
    vec_hand_vector(&v);

    /* f63C6C */
    v = hv("f63C6C", 1, 0);
    hm(&v, 0x110148u, B(1, 0, 0, 0), 4);
    hm(&v, 0x11014Cu, B(0, 0, 0, 0), 4); hm(&v, 0x110150u, B(0, 0, 0, 0), 4);
    hm(&v, 0xE31B8u + 69u * 2u + 0x0Eu + 1u, B(3), 1);   /* area 2 holds 3 */
    v.ret = 1;
    hc(&v, 0x110148u, B(0, 0, 0, 0), 4);
    vec_hand_vector(&v);
    v = hv("f63C6C", 1, 0);                           /* a ship was already bought (0x11014C != 0) */
    hm(&v, 0x110148u, B(1, 0, 0, 0), 4);
    hm(&v, 0x11014Cu, B(1, 0, 0, 0), 4);
    v.ret = 0;
    vec_hand_vector(&v);
    v = hv("f63C6C", 1, 0);                           /* no 3-token area */
    hm(&v, 0x110148u, B(1, 0, 0, 0), 4);
    hm(&v, 0x11014Cu, B(0, 0, 0, 0), 4); hm(&v, 0x110150u, B(0, 0, 0, 0), 4);
    hm(&v, 0xE31B8u + 69u * 2u + 0x0Eu + 1u, B(2), 1);
    v.ret = 0;
    vec_hand_vector(&v);

    /* f2EC38: the Epidemic helper. limits 2, 1, 3 (terr bytes t=5: 0x10, t=6: 0x08, t=7: 0x18) give v = 3, 4, 2,
       sorted 2, 3, 4 */
    v = hv("f2EC38", 1, 10);
    hm(&v, NA(1, 0x51), B(3), 1);
    hm(&v, NA(1, 0x52), B(5, 6, 7), 3);
    hm(&v, 0xE1638u + 35u * 5u + 1u, B(0x10), 1);
    hm(&v, 0xE1638u + 35u * 6u + 1u, B(0x08), 1);
    hm(&v, 0xE1638u + 35u * 7u + 1u, B(0x18), 1);
    v.ret = 1; vec_hand_vector(&v);                   /* L = 10 returns 1 */
    v = hv("f2EC38", 1, 8); v.ret = 0;                /* L = 8 returns 0 (b reaches 4) */
    hm(&v, NA(1, 0x51), B(3), 1); hm(&v, NA(1, 0x52), B(5, 6, 7), 3);
    hm(&v, 0xE1638u + 35u * 5u + 1u, B(0x10), 1);
    hm(&v, 0xE1638u + 35u * 6u + 1u, B(0x08), 1);
    hm(&v, 0xE1638u + 35u * 7u + 1u, B(0x18), 1);
    vec_hand_vector(&v);
    v = hv("f2EC38", 1, 3);
    hm(&v, NA(1, 0x51), B(3), 1); hm(&v, NA(1, 0x52), B(5, 6, 7), 3);
    hm(&v, 0xE1638u + 35u * 5u + 1u, B(0x10), 1);
    hm(&v, 0xE1638u + 35u * 6u + 1u, B(0x08), 1);
    hm(&v, 0xE1638u + 35u * 7u + 1u, B(0x18), 1);
    v.ret = 0; vec_hand_vector(&v);                   /* L = 3 returns 0 */
    v = hv("f2EC38", 1, 1);
    hm(&v, NA(1, 0x51), B(3), 1); hm(&v, NA(1, 0x52), B(5, 6, 7), 3);
    hm(&v, 0xE1638u + 35u * 5u + 1u, B(0x10), 1);
    hm(&v, 0xE1638u + 35u * 6u + 1u, B(0x08), 1);
    hm(&v, 0xE1638u + 35u * 7u + 1u, B(0x18), 1);
    v.ret = -1; vec_hand_vector(&v);                  /* L = 1 returns -1 */
    v = hv("f2EC38", 1, 10);                          /* c = 11: port guard */
    hm(&v, NA(1, 0x51), B(11), 1);
    v.ret = 0; vec_hand_vector(&v);

    /* f2C778: the Civil War faction, 12 or 13; e = S8(0xEED83 + 160n + t1) */
    v = hv("f2C778", 1, 0);
    hm(&v, R12 + 0x51u, B(1), 1); hm(&v, R12 + 0x52u, B(3), 1);
    hm(&v, R12 + 0x60u, B(1), 1); hm(&v, R12 + 0x61u, B(4), 1);
    hm(&v, R13 + 0x60u, B(2), 1); hm(&v, R13 + 0x61u, B(5), 1); hm(&v, R13 + 0x62u, B(6), 1);
    hm(&v, 0xEED83u + 160u + 3u, B(1), 1);            /* city t1=3: e=1 -> +10 */
    hm(&v, 0xEED83u + 160u + 4u, B(0), 1);            /* terr t1=4: e=0 -> +1 */
    hm(&v, 0xEED83u + 160u + 5u, B(2), 1);            /* +2 */
    hm(&v, 0xEED83u + 160u + 6u, B(0xFF), 1);         /* e=-1 -> +1 */
    v.ret = 12;                                       /* s = {10+2, 2+1} = {12, 3}: 12 < 3 false */
    vec_hand_vector(&v);
    v = hv("f2C778", 1, 0);                           /* s[0] < s[1] -> 13 */
    hm(&v, R13 + 0x51u, B(1), 1); hm(&v, R13 + 0x52u, B(3), 1);
    hm(&v, 0xEED83u + 160u + 3u, B(5), 1);
    v.ret = 13;                                       /* s = {0, 10} */
    vec_hand_vector(&v);
    v = hv("f2C778", 1, 0);                           /* tie -> 12 */
    v.ret = 12;
    vec_hand_vector(&v);

    /* comparators: elements at 0x108820 + 16i, args are element link addresses */
    v = hv("f3530C", EL(0), EL(1));
    hm(&v, EL(0) + 8u, B(5), 1); hm(&v, EL(1) + 8u, B(3), 1);
    v.ret = -1; vec_hand_vector(&v);                  /* 5 > 3 -> -1 */
    v = hv("f3530C", EL(2), EL(2));                   /* a == b */
    hm(&v, EL(2) + 8u, B(7), 1);
    v.ret = 0; vec_hand_vector(&v);
    v = hv("f3530C", EL(3), EL(4));                   /* 0x80 (-128) below 1 */
    hm(&v, EL(3) + 8u, B(0x80), 1); hm(&v, EL(4) + 8u, B(1), 1);
    v.ret = 1; vec_hand_vector(&v);
    v = hv("f35368", EL(0), EL(1));                   /* offset 7 */
    hm(&v, EL(0) + 7u, B(9), 1); hm(&v, EL(1) + 7u, B(9), 1);
    v.ret = 0; vec_hand_vector(&v);
    v = hv("f35368", EL(2), EL(3));
    hm(&v, EL(2) + 7u, B(0x80), 1); hm(&v, EL(3) + 7u, B(0x7F), 1);
    v.ret = 1; vec_hand_vector(&v);                   /* -128 < 127 */
    v = hv("f35368", EL(4), EL(5));
    hm(&v, EL(4) + 7u, B(2), 1); hm(&v, EL(5) + 7u, B(3), 1);
    v.ret = 1; vec_hand_vector(&v);
    v = hv("f65C4C", EL(0), EL(1));                   /* int16 at +0x0E: 0x8000 below 0x7FFF */
    hm(&v, EL(0) + 0x0Eu, B(0x00, 0x80), 2); hm(&v, EL(1) + 0x0Eu, B(0xFF, 0x7F), 2);
    v.ret = 1; vec_hand_vector(&v);
    v = hv("f65C4C", EL(2), EL(3));
    hm(&v, EL(2) + 0x0Eu, B(0x64, 0), 2); hm(&v, EL(3) + 0x0Eu, B(0x9C, 0xFF), 2);
    v.ret = -1; vec_hand_vector(&v);                  /* 100 > -100 */
    v = hv("f65C4C", EL(4), EL(4));
    hm(&v, EL(4) + 0x0Eu, B(0x34, 0x12), 2);
    v.ret = 0; vec_hand_vector(&v);
}

int main(int argc, char **argv)
{
    g_ds = calloc(DS_SIZE + 0x1000, 1);
    if (g_ds == NULL || vec_init() != 0)
        return 2;

    hand_cases();
    if (argc > 1 && vec_run_file(argv[1]) < 0)
        return 1;

    vec_print_summary("helpers2_test");
    if (g_stub_calls != 0)
        printf("FAIL %d unexpected stub calls\n", g_stub_calls);

    return vec_exit_status() == 0 && g_stub_calls == 0 ? 0 : 1;
}
