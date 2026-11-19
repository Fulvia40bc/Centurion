/* Module CENSUS of docs/M6_PLAN.md: the census phase 0x13D44 (164 bytes) and its qsort comparator 0x13DE8 (104 bytes,
   reached only by address). Literal translations; the address of each instruction a line comes from is in its
   comment. D = read in the code, G = inference. Prototypes and globals: census.h.

   Callees: f79270 (kit.c, the game-screen message), wc_qsort (wqsort.c, Watcom's qsort 0x8C331, D: call at 0x13DDA).
   No game_rand, no input; the only wait is f79270's 100 ms (f78A20). Runtime: __CHK dropped (0x13D4C, 0x13DED).
   Callers (D): the game loop 0x54824 at 0x5497D (phase 2), and the PBEM routine 0x5195C at 0x52F22 (dropped by Erik's
   ruling).

   Notes (D unless said):
     - The text is the empty string at 0xB0EC0 (a lone zero in the zero run after "CONTINUE"): the census shows no
       message, only the home view of the nation int16 [0xF06B0] for 100 ms (M6_PLAN egg 15, the "nothing happened"
       messages; G: a text removed).
     - The nation for f79270 is read as dword [0xF06AE] sar 16 and the qsort count as dword [0xF0696] sar 16: the
       int16 at 0xF06B0 / 0xF0698, sign-extended.
     - The array 0xF069A is 2 mod 4 and the width 2, so wc_qsort runs in its mode 2 (swaps through 0x8C2B3); the count
       is at most 9, so only its insertion sort (gaps 3, then 1) runs. That compares distinct elements only, and the
       comparator is a strict total order on distinct nation numbers (it never returns 0), so the result is fully
       determined: tokens on the board descending, ties by nation number ascending (G: any correct sort agrees).
     - +0x98 is compared as int8 (cmp bl, bh; jle / jl): a count of 128 or more would sort as negative. Not reachable
       (G: a nation has 55 tokens), kept as is.
     - The comparator's tie test is int16 (cmp dx, bx; jle).
   No 1995 bug found, no text slip (the text is empty). */

#include <stdint.h>
#include "state.h"
#include "kit.h"
#include "wqsort.h"
#include "census.h"

/* an int16 element of the array qsort passes (little-endian, any alignment, as on the 386) */
static int16_t elem16(const void *p)
{
    const uint8_t *b = (const uint8_t *)p;
    return (int16_t)(uint16_t)(b[0] | b[1] << 8);
}

/* 0x13D44 CENSUS */
void f13D44(void)
{
    int32_t n;                                        /* [ebp-0x10] */
    int16_t k;

    f79270(100,                                       /* 0x13D69 eax = 0x64 */
           (int32_t)D32(0xF06AEu) >> 16,              /* 0x13D63 edx = [0xF06AE]; 0x13D6E sar edx, 0x10 */
           -200,                                      /* 0x13D5E ebx = 0xFFFFFF38 (home view) */
           DSTR(0xB0EC0u),                            /* 0x13D59 ecx = 0xB0EC0 ("") */
           0);                                        /* 0x13D57 push 0 (no animation); call 0x13D71 */
    setD16(0xF0698u, 0);                              /* 0x13D78 mov word [0xF0698], dx (edx = 0) */
    for (n = 1; n <= 9; n++) {                        /* 0x13D7F; 0x13D8E cmp 9; jg 0x13DC2; 0x13D8B inc */
        if (D8(0xF1566u + 199u * (uint32_t)n) == 0)   /* 0x13D94 imul edx, n, 0xC7; 0x13D9B mov ah, [edx + 0xF1566] */
            continue;                                 /* 0x13DA3 je 0x13D88 */
        k = S16(0xF0698u);                            /* 0x13DA5 mov ax, [0xF0698]; 0x13DAB movsx edx, ax */
        setD16(0xF0698u, (uint16_t)(k + 1));          /* 0x13DAE inc eax; 0x13DAF mov [0xF0698], ax */
        setD16(0xF069Au + 2u * (uint32_t)(int32_t)k, (uint16_t)n);   /* 0x13DB8 mov [2*edx + 0xF069A], ax */
    }
    wc_qsort(DP(0xF069Au),                            /* 0x13DD2 eax = 0xF069A */
             (size_t)(uint32_t)((int32_t)D32(0xF0696u) >> 16),   /* 0x13DCC edx = [0xF0696]; 0x13DD7 sar edx, 0x10 */
             2,                                       /* 0x13DC7 ebx = 2 */
             f13DE8);                                 /* 0x13DC2 ecx = 0x13DE8; call 0x8C331 at 0x13DDA */
}

/* 0x13DE8: the census comparator */
int f13DE8(const void *a, const void *b)
{
    int8_t bl, bh;
    int16_t dx, bx;

    bl = S8(0xF1564u + 199u * (uint32_t)(int32_t)elem16(a));   /* 0x13E02 movsx edx, word [a]; imul 0xC7; 0x13E17 */
    bh = S8(0xF1564u + 199u * (uint32_t)(int32_t)elem16(b));   /* 0x13E0E movsx eax, word [b]; imul 0xC7; 0x13E1D */
    if (bl > bh)                                      /* 0x13E23 cmp bl, bh; jle 0x13E31 */
        return -1;                                    /* 0x13E27 */
    if (bl < bh)                                      /* 0x13E31 cmp bl, bh; jl 0x13E46 */
        return 1;                                     /* 0x13E46 */
    dx = elem16(a);                                   /* 0x13E38 mov dx, [a] */
    bx = elem16(b);                                   /* 0x13E3E mov bx, [b] */
    if (dx <= bx)                                     /* 0x13E41 cmp dx, bx; jle 0x13E27 */
        return -1;
    return 1;                                         /* 0x13E46 */
}
