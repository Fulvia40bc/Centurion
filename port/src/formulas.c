/* Module FORMULAS of docs/M6_PLAN.md (TASK 15): the small pure formulas of the game loop, item 5 of "Mistral
   candidates". Fifteen functions of AC_MAIN.EXE, each read from the disassembly (D); meanings of the fields are G
   and in formulas.h. None draws on the screen, waits for input or calls game_rand; each reads only data-object
   bytes through state.h, and only 0x7762C writes (into the caller's buffer). No 1995 fix here, so no g_original
   path. Conventions as rules.c: fXXXXX = the original at 0xXXXXX, register arguments eax, edx in order, address
   arithmetic in uint32_t (the 386 wraps), __CHK dropped, each line carries the address of the instruction it
   comes from. The one helper called is rules.h's f78D08 (not copied). Prototypes: formulas.h.

   Notation of the listing: "S8(a)" = the byte at a read signed (dword [a-3] sar 24 / movsx); "D8(a)" = the byte
   at a unsigned (mov al / and 0xFF); "S16(a)" = the int16 at a (dword [a-2] sar 16). Division is C / and %
   (truncates toward zero, like idiv). */

#include <stdint.h>

#include "state.h"
#include "game.h"
#include "rules.h"
#include "formulas.h"

/* nation record n: 0xF14CC + 199n (imul reg, n, 0xC7), as rules.c's nrec() */
static uint32_t nrec(int32_t n)
{
    return GAME_NATION + 199u * (uint32_t)n;
}

/* 0x1B4D4: 1 if the stock cannot pay the tax (cities * tax rate) */
int f1B4D4(int n)
{
    uint32_t rec = nrec(n);
    int32_t tax = (int32_t)S8(rec + 0x51u) * (int32_t)S8(rec + 0x3Fu);      /* 0x1B4FD, 0x1B506, imul 0x1B50C */

    return (int32_t)S8(rec + 0x99u) < tax ? 1 : 0;                          /* 0x1B515, setl 0x1B521 */
}

/* 0x1B538: 1 if n holds exactly a full set of the 18 commodities (the table 0xBD541..0xBD552 is constant) */
int f1B538(int n)
{
    uint32_t rec = nrec(n);
    int32_t i;

    for (i = 1; i <= 18; i++)                                               /* 0x1B57D..0x1B58C cmp al, ah */
        if (D8(rec + (uint32_t)i) == D8(0xBD540u + (uint32_t)i))
            return 1;
    return 0;
}

/* 0x1C2DC: the token shortfall: tax + need - stock, 0 if the stock covers it */
int f1C2DC(int n)
{
    uint32_t rec = nrec(n);
    int32_t tax = (int32_t)S8(rec + 0x51u) * (int32_t)S8(rec + 0x3Fu);      /* 0x1C30C..0x1C31B */
    int32_t need = 0, i = 0, total, s;

    while ((int32_t)S8(rec + 0x60u) > i) {                                  /* 0x1C366: the count is re-read */
        uint32_t t = D8(rec + 0x61u + (uint32_t)i);                         /* 0x1C377: the area record index */
        int32_t v = (int32_t)S8(GAME_AREA + 69u * t + 0x0Eu + (uint32_t)n); /* 0x1C38F dword [69t + (n-1) + 0xE31C4] sar 24 */
        if (v >= 2)
            need += 2;                                                      /* 0x1C39D */
        else if (v == 1)
            need += 1;                                                      /* 0x1C352 */
        i++;
    }
    total = tax + need;                                                     /* 0x1C3A9 */
    s = (int32_t)S8(rec + 0x99u);                                           /* 0x1C3B1 */
    return s >= total ? 0 : total - s;                                      /* 0x1C3C6 */
}

/* 0x193AC / 0x19448: the value of the goods held at base (0xF67E4 / 0xF14CC): sum over i = 1..18 of
   ((i+1)>>1) * c * c, weights 1 1 2 2 ... 9 9; with the Mining bonus flag 0xF6754 set, the chosen commodity
   0xF68B8[D32(0xF68B0)] counts one more (read only when c != 0, the selection only when the flag is set) */
static int goods_value(uint32_t base)
{
    int32_t sum = 0, i;

    for (i = 1; i <= 18; i++) {                                             /* 0x193D8 / 0x19474 */
        int32_t c = (int32_t)S8(base + (uint32_t)i);
        if (c != 0                                                          /* 0x19407 */
            && D8(0xF6754u) != 0                                            /* 0x1940B */
            && (int32_t)S8(0xF68B8u + D32(0xF68B0u)) == i)                  /* 0x19419, 32-bit address */
            c = c + 1;                                                      /* 0x19439 */
        sum += ((i + 1) >> 1) * c * c;                                      /* 0x193E4 imul, 0x193EB sar, 0x193F0 add */
    }
    return sum;
}

int f193AC(void) { return goods_value(0xF67E4u); }
int f19448(void) { return goods_value(GAME_NATION); }

/* 0x176BC: the cost of advance a = word [0xF6E12 + 10n + 2k] (the table 0xBD480, int16, a 0..23, is constant),
   capped at the credit S32(0xF6764 + 4a); a may be negative, the addresses wrap in 32 bits (-1: 0xBD47E, 0xF6760) */
int f176BC(int n, int k)
{
    int32_t a = (int32_t)S16(0xF6E12u + 10u * (uint32_t)n + 2u * (uint32_t)k);   /* 0x176E5 */
    int32_t cost = (int32_t)S16(0xBD480u + 2u * (uint32_t)a);                     /* 0x176EF */
    int32_t credit = (int32_t)S32(0xF6764u + 4u * (uint32_t)a);                    /* 0x17729 */

    if (cost > credit)
        return credit;                                                          /* 0x1773B */
    return cost;                                                                /* 0x1774D */
}

/* 0x2AC94: what is left of x unit points after removing them with the cities (5 each, c = rec+0x51, the
   treasury rec+0x98 as the free part); the two subtractions wrap at 32 bits, a negative result is normal */
int f2AC94(int n, int x)
{
    uint32_t rec = nrec(n);
    int32_t q = x / 5, r = x % 5;                                               /* 0x2ACBA, 0x2ACC8 */
    int32_t c = (int32_t)S8(rec + 0x51u);                                       /* 0x2ACD4 */
    int32_t m, b;

    if (r == 0 && q <= c)
        return 0;                                                               /* 0x2ACE3, 0x2ACEA */
    m = c < q ? c : q;                                                          /* 0x2ACFA */
    b = (int32_t)S8(rec + 0x98u);                                               /* 0x2AD10 */
    if ((int32_t)((uint32_t)x - 5u * (uint32_t)m) <= b)
        return 0;                                                               /* 0x2AD2A */
    return (int32_t)((uint32_t)x - 5u * (uint32_t)(m + 1));                     /* 0x2AD32 */
}

/* 0x6B928: (sb - sa) / 50 of the score history words [turn-1][a], [turn-1][b] of 0xEE9F0, 0 if sa > sb; the
   index is kept exactly like this (the table is written [nation][turn]; past turn 10 it reads on, no guard) */
int f6B928(int a, int b)
{
    int32_t turn = (int32_t)S16(0xF40FCu);                                      /* 0x6B93F */
    uint32_t off = 90u * (uint32_t)(turn - 1);                                  /* 0x6B947..0x6B95A */
    int32_t sa = (int32_t)S16(0xEE9F0u + off + 2u * (uint32_t)a);               /* 0x6B961 */
    int32_t sb = (int32_t)S16(0xEE9F0u + off + 2u * (uint32_t)b);               /* 0x6B971 */

    if (sa > sb)
        return 0;                                                               /* 0x6B981 */
    return (sb - sa) / 50;                                                      /* 0x6B99D */
}

/* 0x7CAB4: 1 if none of n's purchase slots j = 1..9 (0 = 0xF52E0 + 40n + 4j) is negative; j = 0 is not read */
int f7CAB4(int n)
{
    int32_t j;

    for (j = 1; j <= 9; j++)                                                    /* 0x7CAF4 */
        if ((int32_t)S32(0xF52E0u + 40u * (uint32_t)n + 4u * (uint32_t)j) < 0)
            return 0;                                                           /* 0x7CAFB */
    return 1;
}

/* 0x6DB00: whether n can buy a card: bit 5 of 0xE1624 decides; t = the treasury rec+0x3E */
int f6DB00(int n)
{
    uint32_t rec = nrec(n);
    int flag = (D8(0xE1624u) >> 5) & 1;                                         /* 0x6DB15: word read, shl 10, shr ax 15 */
    int32_t t = (int32_t)S8(rec + 0x3Eu);                                       /* 0x6DB30 */

    if (flag == 0)
        return t >= 18 ? 1 : 0;                                                 /* 0x6DB39 */
    else {
        int32_t s;
        for (s = 0; s <= 8; s++)                                                /* 0x6DB75 */
            if ((int32_t)S16(0xF06D0u + 25u * (uint32_t)s) != 0 && t >= 2 * s + 2)
                return 1;                                                       /* 0x6DB9B (a negative count counts) */
    }
    return 0;
}

/* 0x7762C: v with thousands commas into buf (the callers pass 0xF5500, DP of it here) */
void f7762C(int v, char *buf)
{
    char *p = buf;
    int32_t i = 0, had = 0, first = 0;
    int8_t d[32];

    d[0] = 0;
    if (v < 0) {
        v = (int32_t)(0u - (uint32_t)v);                                        /* 0x7765C: INT_MIN stays INT_MIN */
        *p++ = '-';
    }
    while (v != 0) {                                                            /* 0x7766D..0x776A6 */
        had = 1;
        d[i] = (int8_t)(v % 10);                                                /* C remainder, negative when v is */
        v = v / 10;
        i++;
    }
    if (had == 1)
        i = i - 1;                                                              /* 0x776AE */
    do {
        if (i % 3 == 2 && first == 1)
            *p++ = ',';                                                         /* 0x776C7 */
        *p++ = (char)(d[i] + '0');
        first = 1;
        i = i - 1;
    } while (i >= 0);                                                           /* 0x776E7, 0x77702 */
    *p = 0;                                                                     /* 0x7770C */
}

/* 0x46A4C: the tokens over the population limits in n's areas where n is alone */
int f46A4C(int n)
{
    uint32_t rec = nrec(n);
    int32_t sum = 0, i = 0;

    while ((int32_t)S8(rec + 0x60u) > i) {                                      /* 0x46A8E */
        int32_t s = (int32_t)D8(rec + 0x61u + (uint32_t)i) - 1;                 /* 0x46AA3 */
        int32_t lim = f78D08(s, n);                                             /* 0x46AB5 */
        uint32_t A = GAME_AREA + 69u * (uint32_t)(s + 1);                       /* 0x46ADD */
        int32_t tok;

        if ((D8(GAME_TSTATE + 3u * (uint32_t)s + 2u) & 2u) != 0) {             /* 0x46AC9: skip */
            i++;
            continue;
        }
        if ((int32_t)S8(A + 1u) != 1) {                                         /* 0x46ADD: skip */
            i++;
            continue;
        }
        tok = (int32_t)S8(A + 0x0Eu + (uint32_t)n);                             /* 0x46B00 */
        if (lim < tok)
            sum += tok - lim;                                                   /* 0x46B12, 0x46B20 */
        i++;
    }
    return sum;
}

/* 0x46B34: 1 if the population limits of n's areas cover the token supply 0xF4898 */
int f46B34(int n)
{
    uint32_t rec = nrec(n);
    int32_t sum = 0, i = 0;

    while ((int32_t)S8(rec + 0x60u) > i) {                                      /* 0x46A8E-style count re-read */
        sum += f78D08((int32_t)D8(rec + 0x61u + (uint32_t)i) - 1, n);           /* 0x46B8D */
        i++;
    }
    return sum >= (int32_t)S32(0xF4898u) ? 1 : 0;                               /* 0x46B9A setge */
}

/* 0x690AC: the harm the calamity card c (0x13..0x1E) would do to n; a switch through the jump table at 0x6907C;
   "+x" = D8(rec+x) != 0 (the advances at +0x24.., e.g. +0x39 Enlightenment, +0x28 Roadbuilding) */
int f690AC(int c, int n)
{
    uint32_t rec = nrec(n);
    int32_t v;

    if ((uint32_t)c - 0x13u > 0xBu)                                             /* 0x690CC..0x690E6 */
        return 0;
    switch (c) {
    case 0x13: case 0x15: case 0x17: case 0x19:
        return 0;
    case 0x14:
        return -5;
    case 0x1A:
        return -12;
    case 0x1E:
        return -10;
    case 0x16:
        if (D8(rec + 0x39u) != 0)
            return -1;
        if (D8(rec + 0x38u) != 0)
            return -5;
        if (D8(rec + 0x37u) != 0)
            return -10;
        return -15;
    case 0x18:
        v = -15;
        if (D8(rec + 0x29u) != 0)
            v -= 5;
        if (D8(rec + 0x39u) != 0)
            v += 5;
        return v;
    case 0x1B:
        v = -16;
        if (D8(rec + 0x2Du) != 0)
            v += 8;
        if (D8(rec + 0x28u) != 0)
            v -= 5;
        return v;
    case 0x1C:
        v = (int32_t)S8(rec + 0x51u) - 3;
        if (D8(rec + 0x30u) != 0)
            v -= 1;
        if (D8(rec + 0x2Fu) != 0)
            v -= 1;
        if (D8(rec + 0x33u) != 0)
            v -= 1;
        if (D8(rec + 0x34u) != 0)
            v -= 1;
        if (D8(rec + 0x28u) != 0)
            v += 1;
        if (D8(rec + 0x35u) != 0)
            v += 1;
        break;
    case 0x1D:
        v = 4;
        if (D8(rec + 0x33u) != 0)
            v -= 1;
        if (D8(rec + 0x36u) != 0)
            v -= 1;
        if (D8(rec + 0x3Bu) != 0)
            v -= 3;
        if (D8(rec + 0x3Au) != 0)
            v += 1;
        if (D8(rec + 0x28u) != 0)
            v += 1;
        break;
    }
    /* the tail (0x6929D / 0x69347) */
    if (v < 0)
        v = 0;
    v = -(5 * v);
    return v == 0 ? -1 : v;
}

/* 0x667B0: the computer's tax rate for n (1..3): c = cities rec+0x51, s = stock rec+0x99 */
int f667B0(int n)
{
    uint32_t rec = nrec(n);
    int32_t c = (int32_t)S8(rec + 0x51u);                                       /* 0x667D9 */
    int32_t s = (int32_t)S8(rec + 0x99u);                                       /* 0x667E9 */
    int32_t rate;

    if (2 * c > s)
        rate = 1;                                                               /* 0x667F2 */
    else if ((int32_t)S16(0xF40FCu) >= 9 && s >= 25 && c <= 6)
        rate = 3;                                                               /* 0x667FF..0x66820 */
    else
        rate = 2;
    while (rate > 1 && c * rate > s)                                            /* 0x66827..0x6684D */
        rate--;
    return rate;
}
