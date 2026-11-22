/* Module RULES of docs/M6_PLAN.md: the shared rule helpers 0x78D08, 0x78E74, 0x78F04, 0x78FEC, 0x78D98, 0x789EC,
   0x7C760, 0x7C7C4, 0x771F8, 0x76E50, 0x6E460, 0x75CA8, 0x75D84, 0x57CA4, 0x460C0, 0x46158. Literal translations; the
   address of each instruction a line comes from is in its comment. D = read in the code, G = inference. Prototypes and
   the record layout: rules.h (and move.h for the area record and the ship fields).

   Callees, all ported: f57CE8, f57E28, f576F8 (move.c). No library call, no game_rand, no text, no wait.
   Runtime: __CHK dropped. Byte arithmetic on the records wraps as on the 386 (uint8_t stores); counts the code reads
   with "dword [x - 3] sar 24" or movsx are int8, those it reads with "mov al; and eax, 0xFF" are uint8.

   1995 bugs fixed (the original's behaviour kept under g_original, marked BUG(1995)):
     1. 0x78F04 f78F04 (city removed) looks the city up in n's list (+0x52..) and shifts the list down from the index
        it found, kept in [ebp-0x8]. When the city is not in the list, [ebp-0x8] is never written (D: 0x78F98 is its
        only store) and the shift starts at whatever the stack held; the count is decremented and the slot past the
        new end cleared anyway, so a real entry is lost, and a negative garbage index writes below the list (G). The
        fix leaves the list and its count alone when the city is not there (the territory-state writes still
        happen). --original cannot reproduce the stack garbage: the port starts the index at 0 (the whole list
        shifted down, its first entry lost). G: whether a caller reaches this path is to be checked in DOSBox-X
        (M6_PLAN risk 7).
     2. 0x75CA8 f75CA8 (calamity card taken): the same with the calamity lists (+0x14.., givers +0x1C..) when the
        calamity is not held (D: 0x75D0D is the only store of [ebp-0x8]). The fix leaves both lists and the count
        alone; --original starts the index at 0 as in 1. G: reach to be checked as in 1.

   Oddities kept (D unless said):
     - f78D08 and f78FEC compute the Agriculture bonus differently: f78D08 adds 1 when +0x27 != 0 and the area has
       exactly one nation whose marker is n; f78FEC adds the byte +0x27 itself (signed) when the area is empty, or has
       one nation and n has tokens there. Same result while +0x27 holds 0 or 1 (G) and markers agree with tokens.
       f78D08 gives no bonus for an empty area (G: the AI's estimate for an area it has not entered yet).
     - f78D98 shifts only the territory words +0x45.. of the remaining ships; their movement points (+0x41..) and loads
       (+0x4D..) stay at their old indices (G: harmless when the callers, ship maintenance 0x60BD8 / 0x6106C / 0x61254,
       run with every ship empty and the points reset by f46158 before the next movement). The area's selection
       +0x2D+n of the removed ship's territory is not touched unless another ship of n stays there (f576F8 then fixes
       it).
     - f78D98 reads ship k's word as int16 (sar 16) and uses it as the area record index; f576F8 gets word - 1.
     - f78E74, f76E50 and f75D84 append without a bound (10 city slots +0x52..+0x5B, 8 calamity slots +0x14..+0x1B).
     - f76E50 does not write the giver byte +0x1C + count of a drawn calamity (it keeps what was there: 0xFF after
       f75CA8 or the new-game fill).
     - f771F8 maps card 0 to pile 0 ((0 - 1) / 2 truncates to 0) and appends it.
     - f460C0 stores the marker byte itself (ah, 0) when the marker is 0. */

#include <stdint.h>
#include "state.h"
#include "game.h"
#include "move.h"
#include "rules.h"

/* area record of territory t (record t + 1), field off + n: 69(t + 1) + n + 0xE31B8 + off, 32-bit wrap as the code */
static uint32_t ar(int32_t t, int32_t n, uint32_t off)
{
    return GAME_AREA + 69u * (uint32_t)(t + 1) + (uint32_t)n + off;
}

/* nation record n: 0xF14CC + 199n (imul reg, n, 0xC7) */
static uint32_t nrec(int32_t n)
{
    return GAME_NATION + 199u * (uint32_t)n;
}

/* territory state t: 0xE2C18 + 3t (lea 4t; sub t) */
static uint32_t ts(int32_t t)
{
    return GAME_TSTATE + 3u * (uint32_t)t;
}

/* population limit of territory t: bits 3..5 of 0xE1639 + 35t (lea 8t; add t; shl 2; sub t; shl al, 2; shr al, 5) */
static int32_t pop_limit(int32_t t)
{
    uint8_t al = D8(GAME_TERR + 35u * (uint32_t)t + 1u);
    al = (uint8_t)(al << 2);
    al = (uint8_t)(al >> 5);
    return (int32_t)al;
}

/* ==== territories and cities ===================================================================================== */

/* 0x78D08: population limit of territory t for nation n (eax t, edx n) */
int f78D08(int t, int n)
{
    int32_t limit;                                                  /* [ebp-0x8] */
    uint32_t a;                                                     /* [ebp-0x4] */

    limit = pop_limit(t);                                           /* 0x78D1E..0x78D40 */
    if (D8(nrec(n) + 0x27u) != 0) {                                 /* 0x78D4A cmp byte [199n + 0xF14F3], 0; je */
        a = GAME_AREA + 69u * (uint32_t)(t + 1);                    /* 0x78D53..0x78D6A */
        if ((int32_t)S8(a + 1u) == 1                                /* 0x78D70 movsx [a + 1]; cmp 1; jne */
            && (int32_t)S8(a + (uint32_t)n + 1u) == n)              /* 0x78D7F movsx [a + n + 1]; cmp n; jne */
            limit++;                                                /* 0x78D8B inc dword [ebp-0x8] */
    }
    return limit;                                                   /* 0x78D8E */
}

/* 0x78E74: nation n builds a city in territory t (eax t, edx n) */
void f78E74(int t, int n)
{
    int32_t cnt;

    setD8(ts(t) + 2u, (uint8_t)(D8(ts(t) + 2u) | 1u));            /* 0x78E97 or byte [3t + 0xE2C1A], 1 */
    setD8(ts(t), (uint8_t)n);                                       /* 0x78EAF mov [3t + 0xE2C18], al (n) */
    setD8(ts(t) + 1u, D8(0xF40FCu));                                /* 0x78EC1 / 0x78EC7 [3t + 0xE2C19] = [0xF40FC] */
    cnt = (int32_t)S8(nrec(n) + 0x51u);                             /* 0x78ED9 dword [199n + 0xF151A] sar 24 */
    setD8(nrec(n) + (uint32_t)cnt + 0x52u, (uint8_t)(t + 1));       /* 0x78ECD mov bl, t; inc bl; 0x78EE9 mov */
    setD8(nrec(n) + 0x51u, (uint8_t)(D8(nrec(n) + 0x51u) + 1u));   /* 0x78EF7 inc byte [199n + 0xF151D] */
}

/* 0x78F04: nation n's city in territory t removed (eax t, edx n) */
void f78F04(int t, int n)
{
    uint32_t rec;                                                   /* [ebp-0x4] */
    int32_t i;                                                      /* [ebp-0xC] */
    int32_t at;                                                     /* [ebp-0x8] */
    int found = 0;                                                  /* port: whether [ebp-0x8] was written */

    rec = nrec(n);                                                  /* 0x78F1B..0x78F29 */
    setD8(ts(t) + 2u, (uint8_t)(D8(ts(t) + 2u) & 0xFEu));          /* 0x78F38 and byte [3t + 0xE2C1A], -2 */
    setD8(ts(t), 0);                                                /* 0x78F4B xor dh, dh; 0x78F4D mov */
    setD8(ts(t) + 1u, 0xFFu);                                       /* 0x78F5F mov byte [3t + 0xE2C19], -1 */
    at = 0;                                                         /* port: uninitialised in the original */
    for (i = 0; (int32_t)S8(rec + 0x51u) > i; i++) {                /* 0x78F73 dword [rec + 0x4E] sar 24; jle 0x78F9E */
        if ((int32_t)D8(rec + (uint32_t)i + 0x52u) == t + 1) {      /* 0x78F87 mov dl, [rec + i + 0x52]; and 0xFF; cmp */
            at = i;                                                 /* 0x78F9B mov [ebp-0x8], eax */
            found = 1;
            break;                                                  /* falls into 0x78F9E */
        }
    }
    /* BUG(1995) 1: a city not in the list leaves [ebp-0x8] unwritten; the original shifts from stack garbage and
       decrements the count anyway (see the file header). Fixed: the list stays as it is. */
    if (!found && !g_original)
        return;
    for (i = at; (int32_t)S8(rec + 0x51u) - 1 > i; i++)             /* 0x78FAC sar 24; dec eax; cmp; jle 0x78FCF */
        setD8(rec + (uint32_t)i + 0x52u, D8(rec + (uint32_t)i + 0x53u));   /* 0x78FC7 / 0x78FCA */
    setD8(rec + 0x51u, (uint8_t)(D8(rec + 0x51u) - 1u));            /* 0x78FD2 dec byte [rec + 0x51] */
    setD8(rec + (uint32_t)(int32_t)S8(rec + 0x51u) + 0x52u, 0);     /* 0x78FD8 sar 24; 0x78FE1 mov byte [.. + 0x52], 0 */
}

/* 0x78FEC: nation n's city in territory t reduced to tokens (eax t, edx n) */
void f78FEC(int t, int n)
{
    uint32_t rec;                                                   /* [ebp-0xC] */
    int32_t limit;                                                  /* [ebp-0x10] */
    int32_t k;                                                      /* [ebp-0x14] */
    int32_t i;                                                      /* [ebp-0x18] */
    int32_t cnt;

    rec = nrec(n);                                                  /* 0x79004..0x79012 */
    f78F04(t, n);                                                   /* 0x7901B */
    cnt = (int32_t)S8(ar(t, 0, 0x01u));                             /* 0x7902B dword [69(t+1) + 0xE31B6] sar 24: +1 */
    if (cnt == 0                                                    /* 0x79035 test eax, eax; je 0x79074 */
        || (cnt == 1                                                /* 0x7904E cmp eax, 1; jne 0x790A5 */
            && (int32_t)S8(GAME_AREA + 69u * (uint32_t)(t + cnt) + (uint32_t)n + 0x0Eu) != 0)) {
                                                                    /* 0x79056 add edx, eax (t + 1); 0x79067 [.. + n +
                                                                       0xE31C3] sar 24: +0x0E+n; test; je 0x790A5 */
        limit = pop_limit(t)                                        /* 0x79077..0x79093 mov dl, al (edx 0..255) */
              + (int32_t)S8(rec + 0x27u);                           /* 0x79098 dword [rec + 0x24] sar 24; add edx, eax */
    } else {
        limit = pop_limit(t);                                       /* 0x790A5..0x790C7 */
    }
    k = (int32_t)S8(rec + 0x99u);                                   /* 0x790CD dword [rec + 0x96] sar 24 */
    if (k >= limit)                                                 /* 0x790D6 cmp eax, [ebp-0x10]; jl 0x790DE */
        k = limit;                                                  /* 0x790DB */
    for (i = 0; i < k; i++)                                         /* 0x790EE cmp eax, [ebp-0x14]; jge 0x79108 */
        f57E28(t, n, 1);                                            /* 0x79101 */
    setD8(rec + 0x99u, (uint8_t)(D8(rec + 0x99u) - (uint8_t)k));   /* 0x7910E sub byte [rec + 0x99], al */
    setD8(rec + 0x98u, (uint8_t)(D8(rec + 0x98u) + (uint8_t)k));   /* 0x7911A add byte [rec + 0x98], al */
}

/* 0x7C760: the first nation present in territory t (eax t) */
int f7C760(int t)
{
    int32_t found = 0;                                              /* [ebp-0x8] */
    int32_t i;                                                      /* [ebp-0xC] */

    for (i = 0; i < 11; i++) {                                      /* 0x7C78B cmp [ebp-0xC], 0xB; jge 0x7C7BB */
        if ((int32_t)S8(ar(t, i, 0x02u)) == i + 1) {               /* 0x7C7A3 dword [69(t+1) + i + 0xE31B7] sar 24 */
            found = i + 1;                                          /* 0x7C7B8 */
            break;
        }
    }
    return found;                                                   /* 0x7C7BB */
}

/* 0x7C7C4: the first nation other than n present in territory t (eax t, edx n) */
int f7C7C4(int t, int n)
{
    int32_t found = 0;                                              /* [ebp-0x8] */
    int32_t i;                                                      /* [ebp-0xC] */

    for (i = 0; i < 11; i++) {                                      /* 0x7C7F2 cmp [ebp-0xC], 0xB; jge 0x7C82D */
        if (n - 1 == i)                                             /* 0x7C7FB dec eax; 0x7C7FF cmp eax, edi; je */
            continue;
        if ((int32_t)S8(ar(t, i, 0x02u)) == i + 1) {               /* 0x7C815 dword [.. + 0xE31B7] sar 24; cmp i + 1 */
            found = i + 1;                                          /* 0x7C82A */
            break;
        }
    }
    return found;                                                   /* 0x7C82D */
}

/* 0x789EC: the article of nation n's adjective (eax n): 0 for "A", 1 for "An" (callers: 0x21021, 0x2110B). Kept
   literal; the text errors are in the callers (module CAL1, 0x20FC4), not here (D): 0x21021 maps 1 / 0 to 0xB1E20
   "An" / 0xB1E23 "A", but 0x2110B maps 1 / 0 to 0xB1E3B "a" / 0xB1E3D "an" ("replaced by a African city", "an
   Thracian city"), and 10 Pirate / 11 Barbarian get 1 ("An Pirate city in ..."; "replaced by a Pirate city" comes out
   right only through the inversion). A fix belongs in CAL1 under !g_original, at both call sites together. */
int f789EC(int n)
{
    if (n == 4 || n == 5 || n == 8)                                 /* 0x789FF / 0x78A05 / 0x78A0B */
        return 0;                                                   /* 0x78A11 */
    return 1;                                                       /* 0x78A17 */
}

/* ==== ships and movement ========================================================================================= */

/* 0x78D98: ship k of nation n removed (eax k, edx n) */
void f78D98(int k, int n)
{
    uint32_t rec;                                                   /* [ebp-0x8] */
    int32_t nm1;                                                    /* [ebp-0xC] */
    int32_t v;                                                      /* [ebp-0x10] */
    int32_t i;                                                      /* [ebp-0x14] */
    uint32_t a;

    rec = nrec(n);                                                  /* 0x78DAF..0x78DBD */
    nm1 = n - 1;                                                    /* 0x78DC3 dec eax */
    v = (int32_t)S16(rec + 2u * (uint32_t)k + 0x45u);               /* 0x78DCF dword [rec + 2k + 0x43] sar 16 */
    for (i = k; i < 3; i++)                                         /* 0x78DE6 cmp [ebp-0x14], 3; jge 0x78E0A */
        setD16(rec + 2u * (uint32_t)i + 0x45u, D16(rec + 2u * (uint32_t)i + 0x47u));   /* 0x78E00 / 0x78E04 */
    setD8(rec + 0x40u, (uint8_t)(D8(rec + 0x40u) - 1u));            /* 0x78E0D dec byte [rec + 0x40] */
    a = GAME_AREA + 69u * (uint32_t)v + (uint32_t)nm1 + 0x1Cu;      /* 0x78E13..0x78E1F 69v + n - 1 */
    setD8(a, (uint8_t)(D8(a) - 1u));                                /* 0x78E22 dec byte [.. + 0xE31D4] */
    setD16(rec + 0x4Bu, 0);                                         /* 0x78E2B mov word [rec + 0x4B], 0 */
    for (i = 0; (int32_t)D8(rec + 0x40u) > i; i++) {                /* 0x78E41 mov al, [rec + 0x40]; and 0xFF; jle */
        setD32(0xF40F8u, (uint32_t)n);                              /* 0x78E51 mov [0xF40F8], eax */
        f576F8((int32_t)S16(rec + 2u * (uint32_t)i + 0x45u) - 1);   /* 0x78E5E sar 16; dec eax; 0x78E65 call */
    }
}

/* 0x57CA4: k tokens of nation n from a to b (eax a, edx b, ebx n, ecx flag, [esp+4] k; ret 4) */
void f57CA4(int a, int b, int n, int flag, int k)
{
    f57CE8(a, n, flag, k);                                          /* 0x57CC0..0x57CCC */
    f57E28(b, n, k);                                                /* 0x57CD1..0x57CDA */
}

/* 0x460C0: the start of nation n's movement: every token may move (eax n) */
void f460C0(int n)
{
    int32_t nm1;                                                    /* [ebp-0x4] */
    int32_t r;                                                      /* [ebp-0x8] */
    uint8_t ah;

    nm1 = n - 1;                                                    /* 0x460D9 dec eax */
    for (r = 1; r <= 0xA0; r++) {                                   /* 0x460F2 cmp [ebp-0x8], 0xA0; jg 0x4614F */
        ah = D8(GAME_AREA + 69u * (uint32_t)r + (uint32_t)nm1 + 0x02u);     /* 0x46123 mov ah, [ecx + 0xE31BA] */
        if (ah == 0)                                                /* 0x46129 test ah, ah; je 0x460E6 */
            setD8(GAME_AREA + 69u * (uint32_t)r + (uint32_t)nm1 + 0x25u, ah);   /* 0x460E6 mov [ebx + 0xE31DD], ah */
        else
            setD8(GAME_AREA + 69u * (uint32_t)r + (uint32_t)nm1 + 0x25u,         /* 0x46147 mov [edx + 0xE31DD], al */
                  D8(GAME_AREA + 69u * (uint32_t)r + (uint32_t)nm1 + 0x0Fu));   /* 0x46141 mov al, [ebx + 0xE31C7] */
    }
}

/* 0x46158: movement points of nation n's ships (eax n) */
void f46158(int n)
{
    uint32_t rec;                                                   /* [ebp-0x4] */
    int32_t k;                                                      /* [ebp-0x8] */

    rec = nrec(n);                                                  /* 0x4616D..0x4617B */
    for (k = 0; (int32_t)D8(rec + 0x40u) > k; k++)                  /* 0x4618E mov al, [rec + 0x40]; and 0xFF; jle */
        setD8(rec + (uint32_t)k + 0x41u,                            /* 0x461AE mov [rec + k + 0x41], al */
              (uint8_t)((D8(rec + 0x25u) != 0 ? 1 : 0) + 4));       /* 0x461A4 test bl, bl; setne al; add al, 4 */
}

/* ==== cards ====================================================================================================== */

/* 0x76E50: the top card of stack s drawn by nation n (eax s, edx n) */
int f76E50(int s, int n)
{
    int32_t card = 0;                                               /* [ebp-0x4] */
    int32_t old;
    uint32_t st = GAME_STACK + 25u * (uint32_t)s;                   /* lea 3s; shl 3; add s */

    if ((int32_t)S16(st) != 0) {                                    /* 0x76E79 dword [25s + 0xF06CE] sar 16; test; je */
        old = (int32_t)S16(st);                                     /* 0x76E98 mov dx, [..]; movsx ebx, dx */
        setD16(st, (uint16_t)(D16(st) - 1u));                       /* 0x76EA2 dec edx; 0x76EA3 mov word [..], dx */
        card = (int32_t)S8(st + (uint32_t)old + 0x05u);             /* 0x76EBB dword [old + 25s + 0xF06D2] sar 24 */
    }
    if (card != 0) {                                                /* 0x76EC8 cmp [ebp-0x4], 0; je 0x76F0B */
        if (card > 0x12) {                                          /* 0x76ED8 cmp esi, 0x12; jle 0x76F02 */
            old = (int32_t)S8(nrec(n) + 0x13u);                     /* 0x76EDD mov dl, [199n + 0xF14DF]; movsx ebx, dl */
            setD8(nrec(n) + 0x13u, (uint8_t)(D8(nrec(n) + 0x13u) + 1u));   /* 0x76EE6 inc dl; 0x76EE8 mov */
            setD8(nrec(n) + (uint32_t)old + 0x14u, (uint8_t)card);  /* 0x76EFA mov [ebx + 199n + 0xF14E0], al */
        } else {
            setD8(nrec(n) + (uint32_t)card,                         /* 0x76F05 inc byte [199n + card + 0xF14CC] */
                  (uint8_t)(D8(nrec(n) + (uint32_t)card) + 1u));
        }
    }
    return card;                                                    /* 0x76F0B */
}

/* 0x771F8: card c onto its discard pile (eax c) */
void f771F8(int c)
{
    int32_t s;                                                      /* [ebp-0x4] */
    int32_t old;
    uint32_t pile;

    if (c <= 0x12)                                                  /* 0x7720D cmp [ebp-0x8], 0x12; jg 0x77222 */
        s = (c - 1) / 2;                                            /* 0x77216 dec; cdq-like sar 31; sub; sar: truncates */
    else
        s = (int32_t)S8(0xCA9C1u + (uint32_t)c);                    /* 0x77225 dword [c + 0xCA9BE] sar 24 */
    pile = 0xF07B4u + 25u * (uint32_t)s;                            /* 0x77234..0x77240 */
    old = (int32_t)S16(pile);                                       /* 0x77242 mov dx, [..]; movsx ebx, dx */
    setD16(pile, (uint16_t)(D16(pile) + 1u));                       /* 0x7724C inc edx; 0x7724D mov word [..], dx */
    setD8(pile + (uint32_t)old + 0x06u, (uint8_t)c);                /* 0x7726A mov [ebx + 25s + 0xF07BA], al */
}

/* 0x6E460: nation n's calamity cards sorted ascending, their givers along (eax n) */
void f6E460(int n)
{
    uint32_t rec;                                                   /* [ebp-0x8] */
    int32_t i;                                                      /* [ebp-0x10] */
    int32_t j;                                                      /* [ebp-0xC] */
    uint8_t tmp;                                                    /* [ebp-0x4] */

    rec = nrec(n);                                                  /* 0x6E475..0x6E483 */
    if (D8(rec + 0x13u) == 0)                                       /* 0x6E489 cmp byte [rec + 0x13], 0; je 0x6E534 */
        return;
    for (i = 1; (int32_t)S8(rec + 0x13u) > i; i++) {                /* 0x6E4A5 dword [rec + 0x10] sar 24; jle 0x6E534 */
        for (j = (int32_t)S8(rec + 0x13u) - 1; j >= i; j--) {       /* 0x6E4B4 dec eax; 0x6E4C3 cmp; jl 0x6E49C */
            if ((int8_t)D8(rec + (uint32_t)j + 0x13u)               /* 0x6E4D4 mov al, [rec + j + 0x13] */
                <= (int8_t)D8(rec + (uint32_t)j + 0x14u))           /* 0x6E4D7 mov bl; cmp al, bl; jle 0x6E4BA */
                continue;
            tmp = D8(rec + (uint32_t)j + 0x13u);                    /* 0x6E4E4 / 0x6E4E7 */
            setD8(rec + (uint32_t)j + 0x13u, D8(rec + (uint32_t)j + 0x14u));   /* 0x6E4F6 / 0x6E4F9 */
            setD8(rec + (uint32_t)j + 0x14u, tmp);                  /* 0x6E505 */
            tmp = D8(rec + (uint32_t)j + 0x1Bu);                    /* 0x6E50E / 0x6E511 */
            setD8(rec + (uint32_t)j + 0x1Bu, D8(rec + (uint32_t)j + 0x1Cu));   /* 0x6E520 / 0x6E523 */
            setD8(rec + (uint32_t)j + 0x1Cu, tmp);                  /* 0x6E52F */
        }
    }
}

/* 0x75CA8: card c taken from the nation record at rec (al c, edx rec) */
void f75CA8(int c, uint32_t rec)
{
    int8_t cb = (int8_t)c;                                          /* 0x75CB9 mov [ebp-0x4], al */
    int32_t i;                                                      /* [ebp-0xC] */
    int32_t at;                                                     /* [ebp-0x8] */
    int found = 0;                                                  /* port: whether [ebp-0x8] was written */

    if ((int32_t)cb == 0)                                           /* 0x75CBF movsx; test; je 0x75D7C */
        return;
    if ((int32_t)cb <= 0x12) {                                      /* 0x75CCF cmp eax, 0x12; jg 0x75CE2 */
        setD8(rec + (uint32_t)(int32_t)cb, (uint8_t)(D8(rec + (uint32_t)(int32_t)cb) - 1u));   /* 0x75CDB dec [eax] */
        return;
    }
    at = 0;                                                         /* port: uninitialised in the original */
    for (i = 0; (int32_t)S8(rec + 0x13u) > i; i++) {                /* 0x75CF2 dword [rec + 0x10] sar 24; jle 0x75D13 */
        if (D8(rec + (uint32_t)i + 0x14u) == (uint8_t)cb) {         /* 0x75D03 mov al, [..]; mov dh, c; cmp al, dh */
            at = i;                                                 /* 0x75D10 mov [ebp-0x8], eax */
            found = 1;
            break;
        }
    }
    /* BUG(1995) 2: a calamity not held leaves [ebp-0x8] unwritten; the original shifts from stack garbage and
       decrements the count anyway (see the file header). Fixed: the lists stay as they are. */
    if (!found && !g_original)
        return;
    for (i = at; (int32_t)S8(rec + 0x13u) - 1 > i; i++) {           /* 0x75D24 sar 24; dec eax; cmp; jle 0x75D56 */
        setD8(rec + (uint32_t)i + 0x14u, D8(rec + (uint32_t)i + 0x15u));   /* 0x75D3C / 0x75D3F */
        setD8(rec + (uint32_t)i + 0x1Cu, D8(rec + (uint32_t)i + 0x1Du));   /* 0x75D4E / 0x75D51 */
    }
    setD8(rec + 0x13u, (uint8_t)(D8(rec + 0x13u) - 1u));            /* 0x75D59 dec byte [rec + 0x13] */
    setD8(rec + (uint32_t)(int32_t)S8(rec + 0x13u) + 0x14u, 0);     /* 0x75D68 mov byte [.. + 0x14], 0 */
    setD8(rec + (uint32_t)(int32_t)S8(rec + 0x13u) + 0x1Cu, 0xFFu); /* 0x75D78 mov byte [.. + 0x1C], -1 */
}

/* 0x75D84: card c given to the nation record at rec by the record at giver (al c, edx rec, ebx giver) */
void f75D84(int c, uint32_t rec, uint32_t giver)
{
    int8_t cb = (int8_t)c;                                          /* 0x75D94 mov [ebp-0x4], al */

    if ((int32_t)cb == 0)                                           /* 0x75D9D movsx; test; je 0x75DEC */
        return;
    if ((int32_t)cb <= 0x12) {                                      /* 0x75DA9 cmp eax, 0x12; jg 0x75DBB */
        setD8(rec + (uint32_t)(int32_t)cb, (uint8_t)(D8(rec + (uint32_t)(int32_t)cb) + 1u));   /* 0x75DB5 inc [eax] */
        return;
    }
    setD8(rec + (uint32_t)(int32_t)S8(rec + 0x13u) + 0x14u, (uint8_t)cb);         /* 0x75DBE sar 24; 0x75DCC mov */
    setD8(rec + (uint32_t)(int32_t)S8(rec + 0x13u) + 0x1Cu, D8(giver + 0x3Du));   /* 0x75DE0 / 0x75DE3 */
    setD8(rec + 0x13u, (uint8_t)(D8(rec + 0x13u) + 1u));            /* 0x75DE9 inc byte [rec + 0x13] */
}
