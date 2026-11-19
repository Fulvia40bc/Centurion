/* formulas_test [VECTORS.txt]: runs the FORMULAS module's 15 functions (formulas.c, TASK 15) on vectors, with the
   shared runner vecrun.c (vecrun.h has the vector-file format and the checks).

   It does not need the game folder and never calls state_init: g_ds = calloc(DS_SIZE + 0x1000, 1) here (the extra
   page is for reads past the data object's end, e.g. f6B928's turn > 10).
   Link: formulas_test.c vecrun.c formulas.c rules.c move.c mapdata.c load.c state.c le_image.c (the stub block
   below is rules_test.c's; none of it is reached).

   The hand cases below (at least 3 per function, worked out by hand from the TASK 15 text, edge cases included)
   always run, with or without a vector file; no vector file is kept in the repo.

   Build: clang -std=c11 -Wall -Wextra -Werror -pedantic port/test/formulas_test.c port/test/vecrun.c port/src/formulas.c port/src/rules.c port/src/move.c port/src/mapdata.c port/src/load.c port/src/state.c port/src/le_image.c -Iport/src -o formulas_test.exe
   Run:   formulas_test [VECTORS.txt] */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/state.h"
#include "../src/game.h"
#include "../src/move.h"
#include "../src/rules.h"
#include "../src/formulas.h"
#include "vecrun.h"

/* ---- stubs for what move.c and the loaders reference (none is reached by the FORMULAS routines) ---------------- */

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
_Noreturn void f44CAC(void) { fprintf(stderr, "formulas_test: quit\n"); exit(2); }

/* ---- the function table for vecrun ------------------------------------------------------------------------------ */

static const char *const FUNCS[] = {
    "f1B4D4", "f1B538", "f1C2DC", "f193AC", "f19448", "f176BC", "f2AC94", "f6B928", "f7CAB4", "f6DB00",
    "f7762C", "f46A4C", "f46B34", "f690AC", "f667B0"
};
#define NFUNC 15
static const int FUNC_NARGS[NFUNC] = { 1, 1, 1, 0, 0, 2, 2, 2, 1, 1, 2, 1, 1, 2, 1 };

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
    case 0:  *ret = f1B4D4(a); return 0;
    case 1:  *ret = f1B538(a); return 0;
    case 2:  *ret = f1C2DC(a); return 0;
    case 3:  *ret = f193AC(); return 0;
    case 4:  *ret = f19448(); return 0;
    case 5:  *ret = f176BC(a, b); return 0;
    case 6:  *ret = f2AC94(a, b); return 0;
    case 7:  *ret = f6B928(a, b); return 0;
    case 8:  *ret = f7CAB4(a); return 0;
    case 9:  *ret = f6DB00(a); return 0;
    case 10: f7762C(a, (char *)DP((uint32_t)b)); return 1;
    case 11: *ret = f46A4C(a); return 0;
    case 12: *ret = f46B34(a); return 0;
    case 13: *ret = f690AC(a, b); return 0;
    default: *ret = f667B0(a); return 0;
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
#define B(...) ((const uint8_t[]){ __VA_ARGS__ })

/* ---- hand cases (worked out by hand from the TASK 15 text) ------------------------------------------------------ */

static void hand_cases(void)
{
    vec_t v;

    /* f1B4D4: tax = S8(+0x51) * S8(+0x3F); ret = S8(+0x99) < tax */
    v = hv("f1B4D4", 1, 0); hm(&v, NA(1, 0x51), B(3), 1); hm(&v, NA(1, 0x3F), B(4), 1); hm(&v, NA(1, 0x99), B(12), 1);
    v.ret = 0; vec_hand_vector(&v);                                   /* 12 < 12 is false */
    v = hv("f1B4D4", 1, 0); hm(&v, NA(1, 0x51), B(3), 1); hm(&v, NA(1, 0x3F), B(4), 1); hm(&v, NA(1, 0x99), B(11), 1);
    v.ret = 1; vec_hand_vector(&v);                                   /* 11 < 12 */
    v = hv("f1B4D4", 1, 0); hm(&v, NA(1, 0x51), B(0xFD), 1); hm(&v, NA(1, 0x3F), B(4), 1); hm(&v, NA(1, 0x99), B(0xF3), 1);
    v.ret = 1; vec_hand_vector(&v);                                   /* -13 < -12 (tax -12) */

    /* f1B538: D8(rec+i) == D8(0xBD540+i) returns 1 on the FIRST match, i = 1..18; table 7 7 7 8 8 9 7 8 6 7 5 6 5 6 4 5 4 5 */
    {
        static const uint8_t tbl[18] = { 7, 7, 7, 8, 8, 9, 7, 8, 6, 7, 5, 6, 5, 6, 4, 5, 4, 5 };
        v = hv("f1B538", 1, 0);
        hm(&v, 0xBD541u, tbl, 18);
        hm(&v, NA(1, 1), tbl, 18);
        v.ret = 1; vec_hand_vector(&v);                               /* exact full set: matches at i = 1 */
        v = hv("f1B538", 1, 0);
        hm(&v, 0xBD541u, tbl, 18);
        /* rec bytes all 0, table values all nonzero: no i matches -> 0 */
        v.ret = 0; vec_hand_vector(&v);
        v = hv("f1B538", 1, 0);                                        /* all zero: 0 == 0 at i = 1 */
        v.ret = 1; vec_hand_vector(&v);
    }

    /* f1C2DC: tax = S8(+0x51)*S8(+0x3F); loop while S8(+0x60) > i: t = D8(+0x61+i), v = S8(0xE31B8 + 69t + 0x0E + n);
       v>=2: need+=2, v==1: need+=1; total = tax+need; ret = S8(+0x99) >= total ? 0 : total - S8(+0x99) */
    v = hv("f1C2DC", 1, 0);
    hm(&v, NA(1, 0x51), B(2), 1); hm(&v, NA(1, 0x3F), B(3), 1);           /* tax 6 */
    hm(&v, NA(1, 0x60), B(2), 1); hm(&v, NA(1, 0x61), B(0), 1); hm(&v, NA(1, 0x62), B(1), 1);
    hm(&v, 0xE31B8u + 0x0Eu + 1u, B(2), 1);                              /* t=0, v=2: need 2 */
    hm(&v, 0xE31B8u + 69u + 0x0Eu + 1u, B(1), 1);                        /* t=1, v=1: need 3 */
    hm(&v, NA(1, 0x99), B(9), 1);
    v.ret = 0; vec_hand_vector(&v);                                     /* total 9, s 9 -> 0 */
    v = hv("f1C2DC", 1, 0);
    hm(&v, NA(1, 0x51), B(2), 1); hm(&v, NA(1, 0x3F), B(3), 1);
    hm(&v, NA(1, 0x60), B(2), 1); hm(&v, NA(1, 0x61), B(0), 1); hm(&v, NA(1, 0x62), B(1), 1);
    hm(&v, 0xE31B8u + 0x0Eu + 1u, B(2), 1);
    hm(&v, 0xE31B8u + 69u + 0x0Eu + 1u, B(1), 1);
    hm(&v, NA(1, 0x99), B(8), 1);
    v.ret = 1; vec_hand_vector(&v);                                     /* total 9, s 8 -> 1 */
    v = hv("f1C2DC", 1, 0);
    hm(&v, NA(1, 0x51), B(2), 1); hm(&v, NA(1, 0x3F), B(3), 1);
    hm(&v, NA(1, 0x60), B(1), 1); hm(&v, NA(1, 0x61), B(0), 1);
    hm(&v, 0xE31B8u + 0x0Eu + 1u, B(0xFB), 1);                            /* v=-5: nothing added */
    hm(&v, NA(1, 0x99), B(3), 1);
    v.ret = 3; vec_hand_vector(&v);                                     /* total 6, s 3 -> 3 */

    /* f193AC: sum over i=1..18 of ((i+1)>>1)*c*c, c = S8(0xF67E4+i); Mining flag 0xF6754 + selection 0xF68B8+D32(0xF68B0) */
    v = hv("f193AC", 0, 0);
    v.ret = 0; vec_hand_vector(&v);                                     /* all zero */
    v = hv("f193AC", 0, 0);
    hm(&v, 0xF67E4u + 1u, B(2), 1); hm(&v, 0xF67E4u + 2u, B(3), 1);       /* 1*4 + 1*9 */
    v.ret = 13; vec_hand_vector(&v);
    v = hv("f193AC", 0, 0);
    hm(&v, 0xF67E4u + 1u, B(1), 1); hm(&v, 0xF67E4u + 2u, B(3), 1);
    hm(&v, 0xF6754u, B(1), 1); hm(&v, 0xF68B0u, B(0, 0, 0, 0), 4); hm(&v, 0xF68B8u, B(2), 1);
    v.ret = 17; vec_hand_vector(&v);                                    /* i=2 counts 4: 1 + 16 */
    /* f19448: same with base 0xF14CC (nation record 0) */
    v = hv("f19448", 0, 0);
    hm(&v, 0xF14CCu + 3u, B(2), 1);                                      /* i=3: weight 2, 2*4 */
    v.ret = 8; vec_hand_vector(&v);

    /* f176BC: p = 0xF6E12 + 10n + 2k; a = S16(p); cost = S16(0xBD480+2a); credit = S32(0xF6764+4a);
       ret = cost > credit ? credit : cost */
    v = hv("f176BC", 0, 0);
    hm(&v, 0xF6E12u, B(5, 0), 2);                                        /* a = 5 */
    hm(&v, 0xBD480u + 10u, B(180, 0), 2);                                /* the cost table: cost[5] = 180 */
    hm(&v, 0xF6778u, B(200, 0, 0, 0), 4);
    v.ret = 180; vec_hand_vector(&v);
    v = hv("f176BC", 0, 0);
    hm(&v, 0xF6E12u, B(5, 0), 2);
    hm(&v, 0xBD480u + 10u, B(180, 0), 2);
    hm(&v, 0xF6778u, B(100, 0, 0, 0), 4);
    v.ret = 100; vec_hand_vector(&v);                                   /* capped at the credit */
    v = hv("f176BC", 1, 2);                                              /* p = 0xF6E12 + 10 + 4 */
    hm(&v, 0xF6E12u + 14u, B(0xFF, 0xFF), 2);                            /* a = -1: 0xBD47E, 0xF6760 */
    hm(&v, 0xBD47Eu, B(7, 0), 2);
    hm(&v, 0xF6760u, B(3, 0, 0, 0), 4);
    v.ret = 3; vec_hand_vector(&v);                                     /* negative a wraps: cost 7 > credit 3 */

    /* f2AC94: q = x/5, r = x%5; c = S8(+0x51); r==0 && q<=c -> 0; m = min(c,q); b = S8(+0x98);
       (int32)(x - 5m) <= b -> 0; else (int32)(x - 5(m+1)) (32-bit wrap) */
    v = hv("f2AC94", 1, 10);
    hm(&v, NA(1, 0x51), B(3), 1);
    v.ret = 0; vec_hand_vector(&v);                                     /* r=0, q=2 <= 3 */
    v = hv("f2AC94", 1, 11);
    hm(&v, NA(1, 0x51), B(3), 1); hm(&v, NA(1, 0x98), B(0), 1);
    v.ret = -4; vec_hand_vector(&v);                                    /* m=2, 11-10=1 > 0 -> 11-15 */
    v = hv("f2AC94", 1, 2147483647);
    hm(&v, NA(1, 0x51), B(0x80), 1); hm(&v, NA(1, 0x98), B(0), 1);       /* c=-128: x-5m wraps to -2147483009 <= 0 */
    v.ret = 0; vec_hand_vector(&v);
    v = hv("f2AC94", 1, 8);
    hm(&v, NA(1, 0x51), B(10), 1); hm(&v, NA(1, 0x98), B(2), 1);
    v.ret = -2; vec_hand_vector(&v);                                    /* m=1, 8-5=3 > 2 -> 8-10 */

    /* f6B928: turn = S16(0xF40FC); off = 90*(turn-1); sa = S16(0xEE9F0+off+2a); sb = S16(0xEE9F0+off+2b);
       sa > sb -> 0; else (sb-sa)/50 */
    v = hv("f6B928", 0, 1);
    hm(&v, 0xF40FCu, B(1, 0), 2);
    hm(&v, 0xEE9F0u, B(100, 0), 2); hm(&v, 0xEE9F2u, B(160, 0), 2);
    v.ret = 1; vec_hand_vector(&v);                                     /* (160-100)/50 */
    v = hv("f6B928", 0, 1);
    hm(&v, 0xF40FCu, B(1, 0), 2);
    hm(&v, 0xEE9F0u, B(200, 0), 2); hm(&v, 0xEE9F2u, B(100, 0), 2);
    v.ret = 0; vec_hand_vector(&v);                                     /* sa > sb */
    v = hv("f6B928", 0, 1);                                              /* turn 11: reads past the 900-byte table */
    hm(&v, 0xF40FCu, B(11, 0), 2);
    hm(&v, 0xEE9F0u + 900u, B(100, 0), 2); hm(&v, 0xEE9F0u + 902u, B(160, 0), 2);
    v.ret = 1; vec_hand_vector(&v);

    /* f7CAB4: j=1..9: S32(0xF52E0 + 40n + 4j) < 0 -> 0; else 1 (j=0 not read) */
    v = hv("f7CAB4", 0, 0);
    v.ret = 1; vec_hand_vector(&v);
    v = hv("f7CAB4", 0, 0);
    hm(&v, 0xF52E0u + 12u, B(0xFF, 0xFF, 0xFF, 0xFF), 4);               /* j=3 negative */
    v.ret = 0; vec_hand_vector(&v);
    v = hv("f7CAB4", 0, 0);
    hm(&v, 0xF52E0u, B(0x9C, 0xFF, 0xFF, 0xFF), 4);                      /* j=0 negative: not read */
    v.ret = 1; vec_hand_vector(&v);

    /* f6DB00: flag = bit 5 of 0xE1624; t = S8(+0x3E); flag 0: t >= 18; flag 1: any stack s=0..8 with
       S16(0xF06D0+25s) != 0 and t >= 2s+2 */
    v = hv("f6DB00", 1, 0);
    hm(&v, NA(1, 0x3E), B(18), 1);
    v.ret = 1; vec_hand_vector(&v);
    v = hv("f6DB00", 1, 0);
    hm(&v, NA(1, 0x3E), B(17), 1);
    v.ret = 0; vec_hand_vector(&v);
    v = hv("f6DB00", 1, 0);
    hm(&v, 0xE1624u, B(0x20), 1); hm(&v, NA(1, 0x3E), B(4), 1);
    hm(&v, 0xF06D0u, B(1, 0), 2);                                     /* s=0 non-empty, 4 >= 2 */
    v.ret = 1; vec_hand_vector(&v);
    v = hv("f6DB00", 1, 0);
    hm(&v, 0xE1624u, B(0x20), 1); hm(&v, NA(1, 0x3E), B(3), 1);
    hm(&v, 0xF06D0u + 25u, B(7, 0), 2);                               /* s=1: 3 >= 4 false; s=0 empty */
    v.ret = 0; vec_hand_vector(&v);
    v = hv("f6DB00", 1, 0);
    hm(&v, 0xE1624u, B(0x20), 1); hm(&v, NA(1, 0x3E), B(2), 1);
    hm(&v, 0xF06D0u, B(0xFF, 0xFF), 2);                               /* negative count counts as non-empty */
    v.ret = 1; vec_hand_vector(&v);

    /* f7762C: v with thousands commas into DP(arg); C lines check the string bytes at 0xF5500 */
    v = hv("f7762C", 0, 0xF5500);
    v.has_ret = 0;
    hc(&v, 0xF5500u, B('0', 0), 2);
    vec_hand_vector(&v);
    v = hv("f7762C", 1234, 0xF5500);
    v.has_ret = 0;
    hc(&v, 0xF5500u, B('1', ',', '2', '3', '4', 0), 6);
    vec_hand_vector(&v);
    v = hv("f7762C", -1234567, 0xF5500);
    v.has_ret = 0;
    hc(&v, 0xF5500u, B('-', '1', ',', '2', '3', '4', ',', '5', '6', '7', 0), 11);
    vec_hand_vector(&v);
    v = hv("f7762C", (-2147483647 - 1), 0xF5500);                        /* INT_MIN: negative remainders */
    v.has_ret = 0;
    hc(&v, 0xF5500u, B(0x2D, 0x2E, 0x2C, 0x2F, 0x2C, 0x29, 0x2C, 0x2C, 0x28, 0x2D, 0x2C, 0x2A, 0x2C, 0x28, 0), 15);
    vec_hand_vector(&v);
    v = hv("f7762C", 1000000, 0xF5500);
    v.has_ret = 0;
    hc(&v, 0xF5500u, B('1', ',', '0', '0', '0', ',', '0', '0', '0', 0), 10);
    vec_hand_vector(&v);

    /* f46A4C: while S8(+0x60) > i: s = D8(+0x61+i)-1; lim = f78D08(s,n); skip if D8(0xE2C18+3s+2)&2;
       A = 0xE31B8 + 69(s+1); skip if S8(A+1) != 1; tok = S8(A+0x0E+n); lim < tok -> sum += tok-lim */
    v = hv("f46A4C", 1, 0);
    hm(&v, NA(1, 0x60), B(2), 1); hm(&v, NA(1, 0x61), B(1), 1); hm(&v, NA(1, 0x62), B(3), 1);
    /* i=0: s=0, lim = f78D08(0,1) = pop_limit(0) = 0 (terr byte 0); not skipped; A = 0xE31B8+69; S8(A+1)=1;
       tok = S8(A+0x0E+1) = 5 -> sum += 5 */
    hm(&v, 0xE31B8u + 69u + 1u, B(1), 1);
    hm(&v, 0xE31B8u + 69u + 0x0Eu + 1u, B(5), 1);
    /* i=1: s=2, lim = pop_limit(2) = 7 (terr byte 0x38); D8(0xE2C18+8) & 2 set -> skipped */
    hm(&v, 0xE1638u + 35u * 2u + 1u, B(0x38), 1);
    hm(&v, 0xE2C18u + 3u * 2u + 2u, B(2), 1);
    v.ret = 5; vec_hand_vector(&v);
    v = hv("f46A4C", 1, 0);
    hm(&v, NA(1, 0x60), B(1), 1); hm(&v, NA(1, 0x61), B(1), 1);
    hm(&v, 0xE2C18u + 2u, B(2), 1);                                  /* skipped at the territory flag */
    v.ret = 0; vec_hand_vector(&v);
    v = hv("f46A4C", 1, 0);
    hm(&v, NA(1, 0x60), B(1), 1); hm(&v, NA(1, 0x61), B(1), 1);
    hm(&v, 0xE31B8u + 69u + 1u, B(2), 1);                             /* S8(A+1) != 1: skipped */
    v.ret = 0; vec_hand_vector(&v);
    v = hv("f46A4C", 1, 0);
    hm(&v, NA(1, 0x60), B(1), 1); hm(&v, NA(1, 0x61), B(1), 1);
    hm(&v, 0xE1638u + 1u, B(0x38), 1);                               /* lim 7 */
    hm(&v, 0xE31B8u + 69u + 1u, B(1), 1);
    hm(&v, 0xE31B8u + 69u + 0x0Eu + 1u, B(7), 1);                     /* tok 7: 7 < 7 false */
    v.ret = 0; vec_hand_vector(&v);

    /* f46B34: sum of f78D08(D8(+0x61+i)-1, n) over the loop; ret = sum >= S32(0xF4898) */
    v = hv("f46B34", 1, 0);
    hm(&v, NA(1, 0x60), B(1), 1); hm(&v, NA(1, 0x61), B(1), 1);
    hm(&v, 0xE1638u + 1u, B(0x38), 1);                               /* pop_limit(0) = 7 */
    hm(&v, 0xF4898u, B(7, 0, 0, 0), 4);
    v.ret = 1; vec_hand_vector(&v);                                     /* 7 >= 7 */
    v = hv("f46B34", 1, 0);
    hm(&v, NA(1, 0x60), B(1), 1); hm(&v, NA(1, 0x61), B(1), 1);
    hm(&v, 0xE1638u + 1u, B(0x38), 1);
    hm(&v, 0xF4898u, B(8, 0, 0, 0), 4);
    v.ret = 0; vec_hand_vector(&v);
    v = hv("f46B34", 1, 0);
    hm(&v, NA(1, 0x60), B(2), 1); hm(&v, NA(1, 0x61), B(1), 1); hm(&v, NA(1, 0x62), B(2), 1);
    hm(&v, 0xE1638u + 1u, B(0x38), 1);                               /* t=0: 7, +1 bonus (rec+0x27, alone) */
    hm(&v, NA(1, 0x27), B(1), 1);
    hm(&v, 0xE31B8u + 69u + 1u, B(1), 1); hm(&v, 0xE31B8u + 69u + 2u, B(1), 1);
    hm(&v, 0xF4898u, B(55, 0, 0, 0), 4);                              /* t=1: 0 -> sum 8 < 55 */
    v.ret = 0; vec_hand_vector(&v);

    /* f690AC: the calamity harm switch */
    v = hv("f690AC", 0x13, 1); v.ret = 0; vec_hand_vector(&v);
    v = hv("f690AC", 0x14, 1); v.ret = -5; vec_hand_vector(&v);
    v = hv("f690AC", 0x12, 1); v.ret = 0; vec_hand_vector(&v);              /* out of range */
    v = hv("f690AC", 0x1F, 1); v.ret = 0; vec_hand_vector(&v);
    v = hv("f690AC", 0x1A, 1); v.ret = -12; vec_hand_vector(&v);
    v = hv("f690AC", 0x1E, 1); v.ret = -10; vec_hand_vector(&v);
    v = hv("f690AC", 0x16, 1);
    hm(&v, NA(1, 0x39), B(1), 1);
    v.ret = -1; vec_hand_vector(&v);
    v = hv("f690AC", 0x16, 1);
    hm(&v, NA(1, 0x38), B(1), 1);
    v.ret = -5; vec_hand_vector(&v);
    v = hv("f690AC", 0x16, 1); v.ret = -15; vec_hand_vector(&v);
    v = hv("f690AC", 0x18, 1);
    hm(&v, NA(1, 0x29), B(1), 1); hm(&v, NA(1, 0x39), B(1), 1);
    v.ret = -15; vec_hand_vector(&v);                                    /* -15 -5 +5 */
    v = hv("f690AC", 0x1B, 1);
    hm(&v, NA(1, 0x2D), B(1), 1); hm(&v, NA(1, 0x28), B(1), 1);
    v.ret = -13; vec_hand_vector(&v);                                    /* -16 +8 -5 */
    v = hv("f690AC", 0x1C, 1);
    hm(&v, NA(1, 0x51), B(10), 1); hm(&v, NA(1, 0x30), B(1), 1); hm(&v, NA(1, 0x28), B(1), 1);
    v.ret = -35; vec_hand_vector(&v);                                    /* v=7-1+1=7 -> -35 */
    v = hv("f690AC", 0x1C, 1);
    hm(&v, NA(1, 0x51), B(3), 1);
    v.ret = -1; vec_hand_vector(&v);                                     /* v=0 -> -0 -> -1 */
    v = hv("f690AC", 0x1D, 1);
    hm(&v, NA(1, 0x33), B(1), 1); hm(&v, NA(1, 0x36), B(1), 1); hm(&v, NA(1, 0x3B), B(1), 1);
    hm(&v, NA(1, 0x3A), B(1), 1); hm(&v, NA(1, 0x28), B(1), 1);
    v.ret = -5; vec_hand_vector(&v);                                     /* 4-1-1-3+1+1 = 1 -> -5 */

    /* f667B0: c = S8(+0x51), s = S8(+0x99); 2c > s -> 1; else turn>=9 && s>=25 && c<=6 -> 3 else 2;
       while rate>1 && c*rate > s: rate-- */
    v = hv("f667B0", 1, 0);
    hm(&v, NA(1, 0x51), B(10), 1); hm(&v, NA(1, 0x99), B(5), 1);
    v.ret = 1; vec_hand_vector(&v);                                     /* 20 > 5 */
    v = hv("f667B0", 1, 0);
    hm(&v, NA(1, 0x51), B(5), 1); hm(&v, NA(1, 0x99), B(20), 1);
    v.ret = 2; vec_hand_vector(&v);                                     /* turn 0: rate 2, 10 <= 20 */
    v = hv("f667B0", 1, 0);
    hm(&v, NA(1, 0x51), B(6), 1); hm(&v, NA(1, 0x99), B(25), 1); hm(&v, 0xF40FCu, B(9, 0), 2);
    v.ret = 3; vec_hand_vector(&v);                                     /* 18 <= 25: rate 3 */
    v = hv("f667B0", 1, 0);
    hm(&v, NA(1, 0x51), B(6), 1); hm(&v, NA(1, 0x99), B(13), 1); hm(&v, 0xF40FCu, B(9, 0), 2);
    v.ret = 2; vec_hand_vector(&v);                                     /* 18 > 13 -> 2, 12 <= 13 */
    v = hv("f667B0", 1, 0);
    hm(&v, NA(1, 0x51), B(3), 1); hm(&v, NA(1, 0x99), B(100), 1); hm(&v, 0xF40FCu, B(8, 0), 2);
    v.ret = 2; vec_hand_vector(&v);                                     /* turn 8: rate 2 */
}

int main(int argc, char **argv)
{
    g_ds = calloc(DS_SIZE + 0x1000, 1);
    if (g_ds == NULL || vec_init() != 0)
        return 2;

    hand_cases();
    if (argc > 1 && vec_run_file(argv[1]) < 0)
        return 1;

    vec_print_summary("formulas_test");
    if (g_stub_calls != 0)
        printf("FAIL %d unexpected stub calls\n", g_stub_calls);

    return vec_exit_status() == 0 && g_stub_calls == 0 ? 0 : 1;
}
