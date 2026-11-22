/* Module HELPERS2 of docs/M6_PLAN.md (TASK 16): trade bookkeeping, ship counts, the qsort comparators and two
   calamity helpers - the small routines of "Mistral candidates" items 5-7 that read and write only data-object
   bytes. Each read from the disassembly (D); field meanings are G and in helpers2.h. Conventions as TASK 15 /
   rules.c: fXXXXX = the original at 0xXXXXX, register arguments eax, edx in order, data only through state.h,
   address arithmetic in uint32_t, __CHK dropped, each line carries the address of its instruction. The ported
   helpers of rules.h (f75CA8, f75D84, f78D08) are called, not copied. One routine here has a 1995 fix of its own,
   f2EC38 (below); f75C08 calls f75CA8, whose fix is rules.c's (the test leaves g_original at 0). Prototypes:
   helpers2.h.

   1995 bugs (fixed unless g_original):
     1. 0x2EC94 in f2EC38 (the Epidemic helper when n has no spare tokens) passes n's city entry D8(rec + 0x52 + i)
        to f78D08 as it is, but the city list holds territory + 1 (0x78ECD) and every other reader subtracts 1
        (0x233B8, 0x242F8), so each city gets the limit of the next territory (D code, D effect). The fix passes
        the entry - 1.

   Notation as TASK 15: rec = 0xF14CC + 199n (nation record n), area r = 0xE31B8 + 69r, terr t = 0xE1638 + 35t.
   "S8(a)" = the byte at a signed (dword [a-3] sar 24 / movsx), "D8(a)" = the byte at a unsigned, "S16(a)" = the
   int16 at a. When a step says "wraps", compute in uint32_t and cast to int32_t. */

#include <stdint.h>
#include <string.h>

#include "state.h"
#include "game.h"
#include "rules.h"
#include "helpers2.h"

/* nation record n: 0xF14CC + 199n (imul reg, n, 0xC7), as rules.c's nrec() */
static uint32_t nrec(int32_t n)
{
    return GAME_NATION + 199u * (uint32_t)n;
}

/* ---- trade bookkeeping ----------------------------------------------------------------------------------------- */

/* 0x112570..0x112574 = the 5 cards side 0 gives, 0x112579..0x11257D = the 5 cards side 1 gives (int8, 0 = none).
   The counter-offer rows: 16-byte rows at 0x111118 + 16i, count int16 at 0x112548; row +0x0D = the nation whose
   offer the row answers, row +0x0E = the nation that made the row. Nation +0x9C..+0xA8 = n's own offer. */

/* 0x75BD4: 1 if the calamity card c is tradable; the table at 0xC95C5 is read from the data object (c up to 127
   reads past the calamities, as the original) */
int f75BD4(int c)
{
    int8_t cb = (int8_t)c;                                              /* 0x75BE4: the code keeps al */

    if ((int32_t)cb <= 0x12)
        return 0;                                                       /* 0x75BEB, 0x75C01 */
    return (int32_t)S8(0xC95C5u + (uint32_t)(int32_t)cb);               /* 0x75BF4 dword [c + 0xC95C2] sar 24 */
}

/* 0x75C08: swap the offers between the nation records at a and b; each card byte is read just before its call,
   the two removes interleaved in one loop, the two adds in the next */
void f75C08(uint32_t a, uint32_t b)
{
    int32_t j;

    for (j = 0; j <= 4; j++) {                                          /* 0x75C2D */
        f75CA8((int)S8(0x112570u + (uint32_t)j), a);                    /* 0x75C39, 0x75C42 */
        f75CA8((int)S8(0x112579u + (uint32_t)j), b);                    /* 0x75C4D, 0x75C56 */
    }
    for (j = 0; j <= 4; j++) {                                          /* 0x75C6A */
        f75D84((int)S8(0x112579u + (uint32_t)j), a, b);                 /* 0x75C79, 0x75C82 */
        f75D84((int)S8(0x112570u + (uint32_t)j), b, a);                 /* 0x75C90, 0x75C99 */
    }
}

/* Delete counter-offer row i: shift the rows above it down (the last row is not cleared), then dec the count.
   The code is duplicated in f75DF0 (0x75E3E loop test, 0x75E64 four movsd, 0x75E6A dec word) and f75F08 (0x75F2D,
   0x75F53, 0x75F59). */
static void del_row(int32_t i)
{
    int32_t k = i;

    while ((int32_t)S16(0x112548u) - 1 > k) {                           /* re-read each time */
        memcpy(DP(0x111118u + 16u * (uint32_t)k), DP(0x111128u + 16u * (uint32_t)k), 16);
        k++;
    }
    setD16(0x112548u, (uint16_t)(D16(0x112548u) - 1u));                /* dec word */
}

/* 0x75DF0: clear n's own offer (13 bytes at rec + 0x9C + S8(0xCA9A0), which holds 0 in the EXE but is read anyway,
   signed) and delete every counter-offer row answering n (row +0x0D == n); after a deletion the same i is tested
   again. A negative count runs no row. */
void f75DF0(int n)
{
    uint32_t rec = nrec(n);
    uint32_t p = rec + 0x9Cu + (uint32_t)(int32_t)S8(0xCA9A0u);        /* 0x75E07..0x75E2A, uint32_t */
    int32_t i = 0;                                                      /* 0x75E31 */

    memset(DP(p), 0, 13);                                               /* 0x75E2C the library memset */
    while ((int32_t)S16(0x112548u) > i) {                               /* 0x75E7D, re-read each time */
        if ((int32_t)S8(0x111118u + 16u * (uint32_t)i + 0x0Du) == n) { /* 0x75E90 dword [16i + 0x111122] sar 24 */
            del_row(i);                                                 /* 0x75E74 dec: test the same i again */
        } else {
            i++;                                                        /* 0x75E7A */
        }
    }
}

/* 0x75EB4: f75DF0 for a, then for b, each when its own offer is non-empty; b's byte is read after the first call
   (with a == b the first call has cleared it) */
void f75EB4(int a, int b)
{
    if ((int32_t)S8(nrec(a) + 0x9Cu) != 0)
        f75DF0(a);                                                      /* 0x75ED1, 0x75EE1 */
    if ((int32_t)S8(nrec(b) + 0x9Cu) != 0)
        f75DF0(b);                                                      /* 0x75EED, 0x75EFD */
}

/* 0x75F08: delete the counter-offer rows made by a or b (row +0x0E); after a deletion the same i is tested again */
void f75F08(int a, int b)
{
    int32_t i = 0;                                                      /* 0x75F22 */

    while ((int32_t)S16(0x112548u) > i) {                               /* 0x75F6C */
        int32_t e = (int32_t)S8(0x111118u + 16u * (uint32_t)i + 0x0Eu); /* 0x75F7F dword [16i + 0x111123] sar 24 */
        if (e == a || e == b)                                           /* 0x75F8B, 0x75F95, 0x75F9E */
            del_row(i);                                                 /* 0x75F63: test the same i again */
        else
            i++;
    }
}

/* ---- ship counts ----------------------------------------------------------------------------------------------- */

/* 0x110148 int32 = ship slots still free; the results go to 0x11014C (f63AF0), 0x110150 (f63B7C) and 0x110154
   (f63C6C) - the caller stores them, not these routines. rec + 0x3E int8 = the treasury. Bit 0x20 of 0xE1624 = a
   game variant. The token byte of nation n in area record r is S8(0xE31B8 + 69r + 0x0E + n) (dword [69r + (n-1) +
   0xE31C4] sar 24). */

/* 0x63950: list n's coastal territories (terr byte & 0x02) holding more than 1 token: count at 0x10F30C (written
   before the entry), entries at 0x10F310 + 4k, t = 0..159 */
void f63950(int n)
{
    int32_t t;

    setD32(0x10F30Cu, 0);                                               /* 0x6396D */
    for (t = 0; t <= 159; t++) {                                        /* 0x6397E */
        if ((D8(GAME_TERR + 35u * (uint32_t)t) & 0x02u) != 0) {         /* 0x63998, 0x6399E */
            int32_t v = (int32_t)S8(GAME_AREA + 69u * (uint32_t)(t + 1) + 0x0Eu + (uint32_t)n);   /* 0x639B6 */
            if (v > 1) {                                                /* 0x639BF */
                int32_t k = (int32_t)S32(0x10F30Cu);                    /* 0x639C4 */
                setD32(0x10F30Cu, (uint32_t)(k + 1));                   /* 0x639D1 */
                setD32(0x10F310u + 4u * (uint32_t)k, (uint32_t)t);      /* 0x639D9 */
            }
        }
    }
}

/* the ship limit: 8 with the variant bit 0x20 of 0xE1624, else 18 (0x63B25..0x63B3C / 0x63BC6..0x63BDD) */
static int32_t ship_lim(void)
{
    return (D8(0xE1624u) & 0x20u) != 0 ? 8 : 18;
}

/* 0x63AF0: how many ships n can still buy, 2 treasury each, while the slots last and the treasury is above 1 and
   not in [lim, lim+2]; rec + 0x3E is never written (s is a local) */
int f63AF0(int n)
{
    uint32_t rec = nrec(n);
    int32_t cnt = 0;                                                    /* 0x63B06 */
    int32_t s = (int32_t)S8(rec + 0x3Eu);                               /* 0x63B10 */
    int32_t lim;

    if ((int32_t)S32(0x110148u) <= 0)
        return 0;                                                       /* 0x63B1C */
    lim = ship_lim();
    for (;;) {
        if ((int32_t)S32(0x110148u) <= 0)
            break;                                                      /* 0x63B3F */
        if (s <= 1)
            break;                                                      /* 0x63B48 */
        if (s >= lim && s - 2 <= lim)
            break;                                                      /* 0x63B51, 0x63B59 */
        cnt++;                                                          /* 0x63B64 */
        setD32(0x110148u, (uint32_t)(S32(0x110148u) - 1));              /* 0x63B67 */
        s = s - 2;                                                      /* 0x63B6D */
    }
    return cnt;                                                         /* 0x63B73 */
}

/* 0x63B7C: can n buy one more ship with the tokens it holds: v = s - 2x (wraps), needs an area with 2+ tokens
   (areas 1..157 only) and v above 0 and not in [lim, lim+1] (v + 1 wraps: v = INT_MAX returns 0) */
int f63B7C(int n)
{
    uint32_t rec = nrec(n);
    int32_t x = (int32_t)S32(0x11014Cu);                                /* 0x63B9D */
    int32_t s = (int32_t)S8(rec + 0x3Eu);                               /* 0x63BA2 */
    int32_t v = (int32_t)((uint32_t)s - 2u * (uint32_t)x);              /* 0x63BA8, 0x63BAD wraps */
    int32_t lim = ship_lim();
    int32_t k = 0, r;

    if ((int32_t)S32(0x110148u) <= 0)
        return 0;                                                       /* 0x63BB9 */
    for (r = 1; r <= 157; r++)                                          /* 0x63BF4 cmp 0x9D; jg */
        if ((int32_t)S8(GAME_AREA + 69u * (uint32_t)r + 0x0Eu + (uint32_t)n) >= 2)
            k++;                                                        /* 0x63C0F, 0x63C18, 0x63C20 */
    if (k <= 0)
        return 0;                                                       /* 0x63C25 */
    if ((int32_t)S32(0x110148u) <= 0)
        return 0;                                                       /* 0x63C2B: re-read, cannot fail here */
    if (v <= 0)
        return 0;                                                       /* 0x63C34 */
    if (v >= lim && (int32_t)((uint32_t)v + 1u) <= lim)
        return 0;                                                       /* 0x63C3D, 0x63C45: v + 1 wraps */
    setD32(0x110148u, (uint32_t)(S32(0x110148u) - 1));                  /* 0x63C57 */
    return 1;                                                           /* 0x63C54 */
}

/* 0x63C6C: can n buy a ship needing 3+ tokens in one area: only when no ships were bought yet (0x11014C and
   0x110150 both 0, each read only when the test before it passed) and some area 1..157 holds 3+ tokens */
int f63C6C(int n)
{
    int32_t k = 0, r;

    if ((int32_t)S32(0x110148u) <= 0)
        return 0;                                                       /* 0x63C8C */
    if ((int32_t)S32(0x11014Cu) != 0)
        return 0;                                                       /* 0x63C99 */
    if ((int32_t)S32(0x110150u) != 0)
        return 0;                                                       /* 0x63CA2 */
    for (r = 1; r <= 157; r++)                                          /* 0x63CBD..0x63CE9 */
        if ((int32_t)S8(GAME_AREA + 69u * (uint32_t)r + 0x0Eu + (uint32_t)n) >= 3)
            k++;
    if (k <= 0)
        return 0;                                                       /* 0x63CEE */
    setD32(0x110148u, (uint32_t)(S32(0x110148u) - 1));                  /* 0x63D00 */
    return 1;                                                           /* 0x63CFD */
}

/* ---- the qsort comparators ------------------------------------------------------------------------------------- */

/* The elements are the 16-byte ship-route records at 0x108820 (350 of them), passed to wc_qsort(DP(0x108820),
   count, 16, cmp) (wqsort.h): +0..+5 the stops (0xFF = none), +6 moves left, +7 int8 tokens carried, +8 int8
   tokens collected, +0x0E int16 the route's score. Descending order; the byte compares are signed. */

/* 0x3530C: tokens collected, +8 */
int f3530C(const void *a, const void *b)
{
    const uint8_t *pa = a, *pb = b;
    int32_t x = (int32_t)(int8_t)pa[8], y = (int32_t)(int8_t)pb[8];

    if (x > y)
        return -1;                                                      /* 0x35338, 0x3533D */
    if (x < y)
        return 1;                                                       /* 0x35353, 0x35357 */
    return 0;                                                           /* 0x35361 */
}

/* 0x35368: tokens carried, +7 */
int f35368(const void *a, const void *b)
{
    const uint8_t *pa = a, *pb = b;
    int32_t x = (int32_t)(int8_t)pa[7], y = (int32_t)(int8_t)pb[7];

    if (x > y)
        return -1;                                                      /* 0x35394, 0x35399 */
    if (x < y)
        return 1;                                                       /* 0x353AF, 0x353B3 */
    return 0;                                                           /* 0x353BD */
}

/* 0x65C4C: the route's score, the signed int16 at +0x0E read byte by byte, little-endian */
int f65C4C(const void *a, const void *b)
{
    const uint8_t *pa = a, *pb = b;
    int32_t x = (int32_t)(int16_t)(pa[0x0E] | pa[0x0F] << 8);
    int32_t y = (int32_t)(int16_t)(pb[0x0E] | pb[0x0F] << 8);

    if (x > y)
        return -1;                                                      /* 0x65C79, 0x65C7F */
    if (x < y)
        return 1;                                                       /* 0x65C97, 0x65C9C */
    return 0;                                                           /* 0x65CA6 */
}

/* ---- two calamity helpers -------------------------------------------------------------------------------------- */

/* 0x2EC38: the Epidemic helper when n has no spare tokens. c = S8(rec + 0x51) (n's cities), re-read at every loop
   test; L = the losses. v[i] = 5 - f78D08(t1, n) with t1 = D8(rec + 0x52 + i) passed as it is (no -1: a 1995 slip
   reading the next territory's limit; fixed by default, see BUG(1995) below). v is bubble-sorted ascending; then a = b = L is paid down city by
   city: every difference wraps, "x - y >= 0" tests the sign of the wrapped difference. */
int f2EC38(int n, int L)
{
    uint32_t rec = nrec(n);
    int32_t v[10];
    int32_t c, i, j, a, b, w;

    c = (int32_t)S8(rec + 0x51u);
    if (c > 10)                                                         /* port guard (G: unreachable, 9 city tokens;
                                                                           the original's 9 stack slots overflow
                                                                           into L with c >= 11) */
        return 0;
    for (i = 0; (int32_t)S8(rec + 0x51u) > i; i++) {                    /* 0x2EC75, 0x2EC78 */
        uint32_t t1 = D8(rec + 0x52u + (uint32_t)i);                    /* 0x2EC8C and 0xFF */
        /* BUG(1995) 1: the city list holds territory + 1 (0x78ECD), and every other reader subtracts 1 (0x233B8,
           0x242F8); 0x2EC94 passes the entry as it is, so the limit is that of the next territory. Fixed by default
           (Claude, 19 Nov, Erik's standing rule); --original keeps it. */
        v[i] = 5 - f78D08(g_original ? (int)t1 : (int)t1 - 1, n);      /* 0x2EC94, 0x2EC99 */
    }
    for (i = 1; (int32_t)S8(rec + 0x51u) > i; i++)                      /* 0x2ECAB..0x2ED10 bubble sort ascending */
        for (j = (int32_t)S8(rec + 0x51u) - 1; j >= i; j--)             /* 0x2ECBD */
            if (v[j - 1] > v[j]) {                                      /* 0x2ECEA, 0x2ECEE..0x2ED0C */
                int32_t tmp = v[j - 1];
                v[j - 1] = v[j];
                v[j] = tmp;
            }
    a = L;                                                              /* 0x2ED12..0x2ED1B */
    b = L;
    i = 0;                                                              /* 0x2ED20 */
    while ((int32_t)S8(rec + 0x51u) > i) {                              /* 0x2EDCC, 0x2EDCF */
        w = v[i];
        if ((int32_t)((uint32_t)a - (uint32_t)w) >= 0                   /* 0x2EDEE */
            && (int32_t)((uint32_t)a - 4u) >= 0                         /* 0x2EDF8 */
            && (int32_t)((uint32_t)b - (uint32_t)w) >= 0                /* 0x2EE0A */
            && (int32_t)((uint32_t)b - 4u) >= 0) {                      /* 0x2EE14 */
            if ((int32_t)((uint32_t)a - (uint32_t)w) >= 0
                && (int32_t)((uint32_t)a - 4u) == 0)                    /* 0x2ED83, 0x2ED8B -> 0x2EDA7 */
                return 0;
            if ((int32_t)((uint32_t)b - (uint32_t)w) >= 0
                && (int32_t)((uint32_t)b - 4u) == 0)                    /* 0x2ED9D, 0x2EDA5 */
                return 0;
            a = (int32_t)((uint32_t)a - (uint32_t)w);                   /* 0x2EDBF */
            b = (int32_t)((uint32_t)b - 4u);                            /* 0x2EDC2 */
            i++;                                                        /* 0x2EDC9 */
        } else {
            int32_t d1, d2;
            if ((int32_t)((uint32_t)a - (uint32_t)w) >= 0
                && (int32_t)((uint32_t)a - 4u) < 0)                     /* 0x2EE2A, 0x2EE34 */
                return 0;
            if ((int32_t)((uint32_t)b - (uint32_t)w) >= 0
                && (int32_t)((uint32_t)b - 4u) < 0)                     /* 0x2EE46, 0x2EE54 -> 0x2EE5A */
                return 0;
            d1 = (int32_t)((uint32_t)a - (uint32_t)w);                  /* 0x2ED2E */
            d2 = (int32_t)((uint32_t)b - (uint32_t)w);                  /* 0x2ED3E */
            if (d1 > d2)
                return d1;                                              /* 0x2ED40, 0x2ED50 */
            return d2;                                                  /* 0x2ED62 */
        }
    }
    return 0;                                                           /* 0x2EE5F (the result was 0 at 0x2EC54) */
}

/* 0x2C778: the computer's Civil War faction, 12 or 13: s[f] scores nation f + 12's cities (+10/+5) and territories
   (+2/+1) by e = S8(0xEED83 + 160n + t1) with t1 = t + 1 landing on 0xEED84 + 160n + t (the AI's value of territory
   t for n; 160n is 32-bit). Return 12 + (s[0] < s[1]); a tie gives 12. */
int f2C778(int n)
{
    int32_t s[2] = { 0, 0 };                                            /* 0x2C790..0x2C799 */
    int32_t f;

    for (f = 0; f <= 1; f++) {                                          /* 0x2C7A4 */
        uint32_t r = 0xF14CCu + 199u * (uint32_t)(f + 12);              /* nation 12, then 13: 0x2C7B1..0x2C7C1 */
        int32_t i;
        for (i = 0; (int32_t)S8(r + 0x51u) > i; i++) {                  /* 0x2C7D9 */
            uint32_t t1 = D8(r + 0x52u + (uint32_t)i);                  /* 0x2C7EA */
            int32_t e = (int32_t)S8(0xEED83u + 160u * (uint32_t)n + t1); /* 0x2C802 dword [t1 + 160n + 0xEED80] sar 24 */
            s[f] += e > 0 ? 10 : 5;                                     /* 0x2C814, 0x2C816, 0x2C7CB */
        }
        for (i = 0; (int32_t)S8(r + 0x60u) > i; i++) {                  /* 0x2C831 */
            uint32_t t1 = D8(r + 0x61u + (uint32_t)i);                  /* 0x2C846 */
            int32_t e = (int32_t)S8(0xEED83u + 160u * (uint32_t)n + t1); /* 0x2C85E */
            s[f] += e > 0 ? 2 : 1;                                      /* 0x2C870, 0x2C872, 0x2C824 */
        }
    }
    return 12 + (s[0] < s[1] ? 1 : 0);                                  /* 0x2C87C setl, 0x2C887 */
}
