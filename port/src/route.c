/* Module ROUTE of docs/M6_PLAN.md: the ship-route queries over N.RL, 0x5843C, 0x584B4, 0x58568, 0x5861C, 0x58888,
   0x58A74 and 0x58DCC. Literal translations; the address of each instruction a line comes from is in its comment.
   D = read in the code, G = inference. Prototypes and the data layout: route.h (the tables: game.h, loaded by
   gfxload.c's f3EED8).

   Twins (D: the listings were compared instruction by instruction with the addresses normalised): 0x58568 is 0x584B4
   with table 2's addresses (0x101DC8 / 0x101DCC / 0x101DCD / 0x101DD0 for 0xFFD44 / 0xFFD48 / 0xFFD49 / 0xFFD4C), and
   0x58DCC is 0x58A74 with those and the index 0x107550 for 0x107410; nothing else differs. 0x5861C has the two tables
   as two copies of one loop (0x5865D..0x5876B table 2, 0x58770..0x5887A table 1) that differ the same way. Each twin
   pair is one static body here with the table's address as a parameter; the comments give table 1's addresses, then
   table 2's.

   Records (D): table T (0xFFD44 or 0x101DC8) holds an int32 count, then 8-byte records from T + 4: +0 uint8 from, +1
   uint8 to, +2 int16 routes (read as dword [rec] sar 16), +4 int32 first route. Route r is the 4 bytes at
   [0x10740C] + 4r. The block field holds a port heap handle (load_heap_ptr), never a C pointer; a byte outside the
   block (the original reads whatever lies there) stops the port with a message (port guard, G: the routes of 3.RL to
   8.RL all lie inside, D: route_test).
   Other data (D): nation record +0x2B (0xF14CC + 199n + 0x2B, imul n, 0xC7: G Astronomy, the inventory's reading) picks
   table 2; area record of territory u +0x0E + n (0xE31B8 + 69(u + 1) + 0x0E + n, read as dword [0xE31C4 + 69(u + 1) +
   n - 1] sar 24) are nation n's tokens there; uint8 0xC80DC[u] the territory type (obj2: 8 except the eight open-sea
   areas 17, 35, 51, 60, 77, 89, 125, 134, which have 0..7).

   Runtime: 0x7D5D0 memset(eax, edx, ebx) -> memset. __CHK dropped. No game_rand, no UI call (f44CAC only on the port
   guard). No 1995 bug found that changes a result on the shipped N.RL files; g_original is not read here.

   Oddities kept (D unless said):
     - f584B4 / f58568 and f5861C measure only the first route of a record. Every record of 3.RL..8.RL lists its routes
       by non-decreasing length (D: route_test), so the first is the shortest and the answer is the intended one (G).
     - f58A74 / f58DCC do not copy a blocked territory into the record, and a rejected route stays in the slot the next
       route overwrites: a shorter later route would leave stale bytes behind its b, and 0x58888 would then measure
       and reverse them. Latent on the shipped files (routes sorted by length, see above).
     - f58A74 / f58DCC write the route that ends the scan (>= k intermediates) into slot [0x109E00] without counting it:
       0x108820 holds one more filled record than [0x109E00] says. Callers read only [0x109E00] records (G).
     - No bound on [0x109E00]: slot 350 would be 0x109E00 itself. The most routes of one record is 323 (table 2 of
       4.RL..8.RL; D: route_test), so at most 323 slots are used.
     - When int16 0x107410[a] (0x107550[a]) is -1, the scan starts at record -1, the 8 bytes at 0xFFD40 (0x101DC4):
       int32 [0xFFD40] (0, 2 or -50: its writers 0x664A3 / 0x66880 and the image) then the table count. It could only
       match a == b == 0 with [0xFFD40] == 0, and then its route count (int16 0xFFD42) is 0: nothing is read (D for the
       writers, G that 0x101DC4 never matches either: its byte is read like 0xFFD40's).
     - The redo pass blocks on the counters alone (block[u] != 0), not on tokens and type again; the counters are not
       recounted, and each round clears the smallest one (the first u on a tie). */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "state.h"
#include "game.h"
#include "ui.h"
#include "route.h"

#define RT_TYPE 0xC80DCu   /* uint8[160] territory type */

/* record e of table tab (int32 count at tab, records from tab + 4), 32-bit wrap as the code (e may be -1) */
static uint32_t rec(uint32_t tab, int32_t e)
{
    return tab + 4u + 8u * (uint32_t)e;
}

/* area record of territory u (record u + 1), field off + n: 69(u + 1) + n + 0xE31B8 + off */
static uint32_t ar(int32_t u, int32_t n, uint32_t off)
{
    return GAME_AREA + 69u * (uint32_t)(u + 1) + (uint32_t)n + off;
}

/* nation n has Astronomy: imul eax, n, 0xC7; cmp byte [eax + 0xF14F7], 0 */
static int astronomy(int32_t n)
{
    return D8(GAME_NATION + 199u * (uint32_t)n + 0x2Bu) != 0;
}

/* the byte at handle h in the N.RL block (port guard: quit with a message if it does not resolve) */
static uint8_t rlb(uint32_t h)
{
    const uint8_t *p = load_heap_ptr(h, 1);

    if (p == NULL) {
        snprintf((char *)DP(0xF5500u), 0x1F4, "N.RL route byte 0x%08X outside its block", (unsigned)h);
        f44CAC();
    }
    return *p;
}

/* ---- 0x5843C, 0x584B4, 0x58568: is there a route shorter than k ------------------------------------------------- */

/* 0x584B4 (tab 0xFFD44) / 0x58568 (tab 0x101DC8) */
static int first_short(uint32_t tab, int32_t a, int32_t b, int32_t k)
{
    int32_t e, c;                                               /* [ebp-0xc], [ebp-0x8] */
    uint32_t p;                                                 /* [ebp-0x4] */

    for (e = 0; e < S32(tab); e++) {                            /* 0x584CD; 0x584DD / 0x58591 cmp [tab]; jge */
        if ((int32_t)D8(rec(tab, e)) != a)                      /* 0x584EC / 0x585A0 [8e + tab+4] */
            continue;
        if ((int32_t)D8(rec(tab, e) + 1u) != b)                 /* 0x58500 / 0x585B4 [8e + tab+5] */
            continue;
        p = D32(GAME_RL_BLOCK) + (D32(rec(tab, e) + 4u) << 2);  /* 0x58514..0x58524 [0x10740C] + 4 * [8e + tab+8] */
        for (c = 0; c < 4; c++)                                 /* 0x58529; 0x58536 cmp 4; jge */
            if (rlb(p + (uint32_t)c) == 0xFF)                   /* 0x58542 cmp 0xFF; jne */
                break;
        if (c < k)                                              /* 0x58553 cmp [ebp-0x10]; jge 0x58561 */
            return 1;
        return 0;                                               /* 0x58561 */
    }
    return 0;                                                   /* 0x584E3 jge 0x58561 */
}

int f584B4(int a, int b, int k)
{
    return first_short(GAME_RL_TABLE1, a, b, k);
}

int f58568(int a, int b, int k)
{
    return first_short(GAME_RL_TABLE2, a, b, k);
}

int f5843C(int a, int b, int n, int k)
{
    int32_t t;                                                  /* [ebp-0x4] */

    if (k == 0)                                                 /* 0x58458 cmp [ebp-0x8], 0; jne */
        return 0;                                               /* 0x5845E */
    if (a > b) {                                                /* 0x58467 cmp; jle 0x5847E */
        t = a;                                                  /* 0x5846F */
        a = b;                                                  /* 0x58475 */
        b = t;                                                  /* 0x5847B */
    }
    if (astronomy(n))                                           /* 0x5847E imul 0xC7; 0x58485 cmp [eax + 0xF14F7], 0 */
        return f58568(a, b, k);                                 /* 0x58497 */
    return f584B4(a, b, k);                                     /* 0x584A9 */
}

/* ---- 0x5861C: the territories reachable from t ------------------------------------------------------------------ */

/* One table's loop of 0x5861C: 0x58770..0x5887A (tab 0xFFD44) / 0x5865D..0x5876B (tab 0x101DC8) */
static void reach_scan(uint32_t tab, int32_t t, int32_t k)
{
    int32_t e, c;                                               /* [ebp-0x18], [ebp-0x14] */
    uint32_t p;                                                 /* [ebp-0x10] */

    for (e = 0; e < S32(tab); e++) {                            /* 0x58770 / 0x5865D; 0x58814 / 0x58701 cmp; jge */
        if ((int32_t)D8(rec(tab, e)) != t                       /* 0x5881F / 0x58710 [8e + tab+4] == t: je */
            && (int32_t)D8(rec(tab, e) + 1u) != t)              /* 0x58833 / 0x58724 [8e + tab+5]: jne next */
            continue;
        if ((int32_t)D8(rec(tab, e)) == t) {                    /* 0x58849 / 0x5873A; jne 0x587AE / 0x5869B */
            p = D32(GAME_RL_BLOCK) + (D32(rec(tab, e) + 4u) << 2);   /* 0x58861..0x58872 / 0x58752..0x58763 */
            for (c = 0; c < 4; c++)                             /* 0x58875; 0x58780 / 0x5866D cmp 4; jge */
                if (rlb(p + (uint32_t)c) == 0xFF)               /* 0x58793 / 0x58680 */
                    break;
            if (c >= k)                                         /* 0x5879D / 0x5868A cmp [ebp-0x1c]; jge next */
                continue;
            setD32(ROUTE_REACH + 4u * D8(rec(tab, e) + 1u), 1); /* 0x587A5 / 0x58692 the to byte; 0x58800 / 0x586ED */
        } else {
            p = D32(GAME_RL_BLOCK) + (D32(rec(tab, e) + 4u) << 2);   /* 0x587AE..0x587BF / 0x5869B..0x586AC */
            for (c = 0; c < 4; c++)                             /* 0x587C2; 0x587CF / 0x586BC cmp 4; jge */
                if (rlb(p + (uint32_t)c) == 0xFF)               /* 0x587E2 / 0x586CF */
                    break;
            if (c >= k)                                         /* 0x587EC / 0x586D9; jge next */
                continue;
            setD32(ROUTE_REACH + 4u * D8(rec(tab, e)), 1);      /* 0x587F4 / 0x586E1 the from byte; 0x58800 / 0x586ED */
        }
    }
}

void f5861C(int t, int n, int k)
{
    memset(DP(ROUTE_REACH), 0, 0x280);                          /* 0x58644 memset(0x107EE4, 0, 0x280) */
    if (astronomy(n))                                           /* 0x58649 imul 0xC7; 0x58650 cmp; je 0x58770 */
        reach_scan(GAME_RL_TABLE2, t, k);
    else
        reach_scan(GAME_RL_TABLE1, t, k);
}

/* ---- 0x58A74, 0x58DCC: the usable routes a -> b ----------------------------------------------------------------- */

/* 0x58A74 (tab 0xFFD44, first 0x107410) / 0x58DCC (tab 0x101DC8, first 0x107550). Called with a <= b (f58888). */
static void paths(uint32_t tab, uint32_t first, int32_t a, int32_t b, int32_t n, int32_t k)
{
    /* [ebp-0x178] int16[160]; port: 256 words, so a route byte above 159 (none in 3.RL..8.RL, D: route_test) stays
       inside the array; the scans below read only 0..159 as the original */
    int16_t blk[256];
    int32_t e, j, i, u, flag;                                   /* [ebp-0x28], [ebp-0x24], [ebp-0x20], -, [ebp-0x14] */
    int32_t best, bu;                                           /* [ebp-0xc], [ebp-0x10] */
    uint32_t p;                                                 /* [ebp-0x1c] */

    setD32(ROUTE_COUNT, 0);                                     /* 0x58AA9 (edx = 0) */
    memset(blk, 0, sizeof blk);                                 /* 0x58AAF memset([ebp-0x178], 0, 0x140) */
    for (e = S16(first + 2u * (uint32_t)a); ; e++) {            /* 0x58AB7 dword [2a + first-2] sar 16 */
        if (e >= S32(tab))                                      /* 0x58ACF cmp [tab]; jge 0x58DC4 */
            return;
        if ((int32_t)D8(rec(tab, e)) != a)                      /* 0x58ADE [8e + tab+4] */
            continue;
        if ((int32_t)D8(rec(tab, e) + 1u) == b)                 /* 0x58AF2 [8e + tab+5]; 0x58B01 cmp edi; jne */
            break;
    }

    /* the first pass: block on territories of type 8 where nation n has no tokens */
    for (j = 0; ; j++) {                                        /* 0x58B05 xor eax, edi (b ^ b); 0x58BEA inc */
        if (S16(rec(tab, e) + 2u) <= j)                         /* 0x58BF0 dword [8e + tab+4] sar 16; jle 0x58C3B */
            break;
        p = D32(GAME_RL_BLOCK) + ((D32(rec(tab, e) + 4u) + (uint32_t)j) << 2);   /* 0x58C02..0x58C17 */
        setD8(ROUTE_BUF + (D32(ROUTE_COUNT) << 4), (uint8_t)a); /* 0x58C1A..0x58C26 */
        flag = 0;                                               /* 0x58C2E */
        for (i = 0; i < 4; i++) {                               /* 0x58C33; 0x58B2F cmp 4; jge 0x58BBB */
            u = rlb(p + (uint32_t)i);                           /* 0x58B3F */
            if (u == 0xFF)                                      /* 0x58B46 cmp 0xFF; je 0x58BBB */
                break;
            if (S8(ar(u, n, 0x0Eu)) == 0                        /* 0x58B5A..0x58B77 dword sar 24; jne 0x58B0F */
                && D8(RT_TYPE + (uint32_t)u) == 8) {            /* 0x58B86 [u + 0xC80DC]; 0x58B91 cmp 8; jne 0x58B0F */
                blk[u]++;                                       /* 0x58BA7 inc word [ebp + 2u - 0x178] */
                flag = 1;                                       /* 0x58BAF */
            } else {
                setD8(ROUTE_BUF + 1u + (D32(ROUTE_COUNT) << 4) + (uint32_t)i, (uint8_t)u);   /* 0x58B0F..0x58B23 */
            }
        }
        setD8(ROUTE_BUF + 1u + (D32(ROUTE_COUNT) << 4) + (uint32_t)i, (uint8_t)b);   /* 0x58BBB..0x58BCB */
        if (i >= k)                                             /* 0x58BD4 cmp [ebp-0x2c]; jge 0x58C3B */
            break;
        if ((int16_t)flag == 0)                                 /* 0x58BDC test si, si; jne */
            setD32(ROUTE_COUNT, D32(ROUTE_COUNT) + 1u);         /* 0x58BE1 inc [0x109E00] */
    }

    for (;;) {
        if (S32(ROUTE_COUNT) != 0)                              /* 0x58C3B cmp [0x109E00], 0; jne 0x58DC4 */
            return;
        best = 0x270F;                                          /* 0x58C48 */
        bu = -1;                                                /* 0x58C4F */
        for (u = 0; u < 0xA0; u++) {                            /* 0x58C56; 0x58C63 cmp 0xA0; jge 0x58CA4 */
            if (blk[u] == 0)                                    /* 0x58C6F dword [ebp + 2u - 0x17A] sar 16; je */
                continue;
            if (blk[u] >= (int16_t)best)                        /* 0x58C88 cmp ax, word [ebp-0xc]; jge */
                continue;
            best = (uint16_t)blk[u];                            /* 0x58C91 mov ax, word; 0x58C99 (eax's high word: u's, 0) */
            bu = u;                                             /* 0x58C9F */
        }
        if ((int16_t)bu == -1)                                  /* 0x58CA4 movsx eax, word [ebp-0x10]; cmp -1; je 0x58DC4 */
            return;
        blk[(int16_t)bu] = 0;                                   /* 0x58CB1..0x58CB7 */
        memset(DP(ROUTE_BUF), 0xFF, 0x15E0);                    /* 0x58CD0 memset(0x108820, 0xFF, 0x15E0) */

        /* the redo: block on the counters left */
        for (j = 0; ; j++) {                                    /* 0x58CD5 (ecx = 0 kept across memset); 0x58D6F */
            if (S16(rec(tab, e) + 2u) <= j)                     /* 0x58D75 sar 16; jle 0x58C3B */
                break;
            p = D32(GAME_RL_BLOCK) + ((D32(rec(tab, e) + 4u) + (uint32_t)j) << 2);   /* 0x58D8B..0x58DA0 */
            setD8(ROUTE_BUF + (D32(ROUTE_COUNT) << 4), (uint8_t)a);   /* 0x58DA3..0x58DB1 */
            flag = 0;                                           /* 0x58DB7 */
            for (i = 0; i < 4; i++) {                           /* 0x58DBC; 0x58CFF cmp 4; jge 0x58D3D */
                u = rlb(p + (uint32_t)i);                       /* 0x58D0B */
                if (u == 0xFF)                                  /* 0x58D12 cmp 0xFF; je 0x58D3D */
                    break;
                if (blk[u] != 0)                                /* 0x58D26 dword [ebp + 2u - 0x17A] sar 16; je 0x58CDD */
                    flag = 1;                                   /* 0x58D34 */
                else
                    setD8(ROUTE_BUF + 1u + (D32(ROUTE_COUNT) << 4) + (uint32_t)i, (uint8_t)u);   /* 0x58CDD..0x58CF3 */
            }
            setD8(ROUTE_BUF + 1u + (D32(ROUTE_COUNT) << 4) + (uint32_t)i, (uint8_t)b);   /* 0x58D3D..0x58D4C */
            if (i >= k)                                         /* 0x58D55 cmp [ebp-0x2c]; jge 0x58C3B */
                break;
            if ((int16_t)flag == 0)                             /* 0x58D61 test di, di; jne */
                setD32(ROUTE_COUNT, D32(ROUTE_COUNT) + 1u);     /* 0x58D66 */
        }
    }
}

void f58A74(int a, int b, int n, int k)
{
    paths(GAME_RL_TABLE1, GAME_RL_FIRST1, a, b, n, k);
}

void f58DCC(int a, int b, int n, int k)
{
    paths(GAME_RL_TABLE2, GAME_RL_FIRST2, a, b, n, k);
}

/* ---- 0x58888: the usable routes a -> b, in a's direction -------------------------------------------------------- */

void f58888(int a, int b, int n, int k)
{
    int32_t swapped, r, i, tmp;                                 /* [ebp-0x8], [ebp-0x10], [ebp-0xc], [ebp-0xc] */
    uint32_t p;                                                 /* [ebp-0x4] */

    if (k == 0) {                                               /* 0x588A4 test edx, edx; jne */
        setD32(ROUTE_COUNT, 0);                                 /* 0x588AB (edx = k = 0) */
        return;
    }
    swapped = 0;                                                /* 0x588B7 */
    if (a > b) {                                                /* 0x588BD cmp; jle 0x588DB */
        tmp = a;                                                /* 0x588C5 [ebp-0x10] */
        a = b;                                                  /* 0x588CB */
        b = tmp;                                                /* 0x588D1 */
        swapped = 1;                                            /* 0x588D4 */
    }
    memset(DP(ROUTE_BUF), 0xFF, 0x15E0);                        /* 0x588EA memset(0x108820, 0xFF, 0x15E0) */
    if (astronomy(n))                                           /* 0x588EF imul 0xC7; 0x588F6 cmp; je 0x58912 */
        f58DCC(a, b, n, k);                                     /* 0x5890B */
    else
        f58A74(a, b, n, k);                                     /* 0x5891E */

    if (swapped != 0)                                           /* 0x58923 cmp [ebp-0x8], 0; je 0x58A10 */
        for (r = 0; r < S32(ROUTE_COUNT); r++) {                /* 0x5892D; 0x5893D cmp [0x109E00]; jge 0x58A10 */
            p = ROUTE_BUF + ((uint32_t)r << 4);                 /* 0x58949..0x58956 */
            tmp = D8(p);                                        /* 0x5895E */
            setD8(p, D8(p + 5u));                               /* 0x58966..0x5896C */
            setD8(p + 5u, (uint8_t)tmp);                        /* 0x5896E..0x58974 */
            tmp = D8(p + 1u);                                   /* 0x5897C */
            setD8(p + 1u, D8(p + 4u));                          /* 0x58985..0x5898B */
            setD8(p + 4u, (uint8_t)tmp);                        /* 0x5898E..0x58994 */
            tmp = D8(p + 2u);                                   /* 0x5899C */
            setD8(p + 2u, D8(p + 3u));                          /* 0x589A5..0x589AB */
            setD8(p + 3u, (uint8_t)tmp);                        /* 0x589AE..0x589B4 */
            while (D8(p) == 0xFF) {                             /* 0x589B7 cmp 0xFF; jne 0x58934 */
                setD8(p, D8(p + 1u));                           /* 0x589CC..0x589D5 */
                setD8(p + 1u, D8(p + 2u));                      /* 0x589D7..0x589E0 */
                setD8(p + 2u, D8(p + 3u));                      /* 0x589E3..0x589EC */
                setD8(p + 3u, D8(p + 4u));                      /* 0x589EF..0x589F8 */
                setD8(p + 4u, D8(p + 5u));                      /* 0x589FB..0x58A04 */
                setD8(p + 5u, 0xFF);                            /* 0x58A0A */
            }
        }

    for (r = 0; r < S32(ROUTE_COUNT); r++) {                    /* 0x58A10; 0x58A5D cmp [0x109E00]; jge 0x58A6E */
        for (i = 2; i < 6; i++)                                 /* 0x58A65; 0x58A1D cmp 6; jge 0x58A3E */
            if (D8(ROUTE_BUF + ((uint32_t)r << 4) + (uint32_t)i) == 0xFF)   /* 0x58A2C, 0x58A37 cmp 0xFF; jne */
                break;
        /* 0x58A3E mov al, [ebp-0xc]; dec al; mov ah, [ebp-0x14]; sub ah, al; 0x58A4E byte [16r + 0x108826] */
        setD8(ROUTE_BUF + ((uint32_t)r << 4) + 6u, (uint8_t)((uint8_t)k - (uint8_t)(i - 1)));
    }
}
