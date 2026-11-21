/* Module AICAL1 of M7 batch 2 (docs/M6_PLAN.md, the computer parts of CAL1, CAL3 and MONO): literal translations of
   the computers' choices in the city calamities, the Volcano / Earthquake and Monotheism. The address of the
   instruction a line comes from is in its comment. D = read in the code, G = inference. Prototypes: aical1.h.

     0x20C70  f20C70  Treachery: the victim's city the beneficiary takes                     852 bytes
     0x213C0  f213C0  a computer's own city to reduce                                         992
     0x24EB8  f24EB8  Piracy, primary victim (the trader's choice)                          1,028
     0x252BC  f252BC  Piracy, secondary victims (the victim's choice)                         652
     0x25548  f25548  Piracy, the coastal city of a secondary victim                        1,264
     0x2E1D0  f2E1D0  a computer's city from the list 0xF7CA0 (Piracy, Flood)                 652
     0x1EF28  f1EF28  which of the tied volcanoes erupts                                      820
     0x20068  f20068  the Earthquake pairs ordered                                            336
     0x201B8  f201B8  the city an Earthquake hits when no pair exists                         508
     0x21E8C  f21E8C  a computer converts by Monotheism                                     2,340

   Calls (all ported): f28CFC (cal1.c), f78E74 / f78F04 (rules.c), f57CE8 / f57E28 (move.c), f79270 (kit), f78A20
   (ui_kit2.c, the port's virtual clock: a headless run does not wait), mono_converts_fmt (mono.c, the Montheism fix).
   Runtime: __CHK 0x7D4AC dropped; sprintf 0x7DB3D -> snprintf bounded to the original's buffers (100, 100, 152
   bytes); memset 0x7D5D0 on a local; the movsd record copies -> memcpy / struct copies. No call into another
   batch-2 module, no game_rand f78AE4 (and none through f76F70), no clock() of its own, no World State or Move List
   Stack operation, no trace or progress text (D: the calls listed). PBEM_DROPPED: none (no test of bit 0x80 of
   [0xE1624] in these routines, D).

   Locals read before they are written (the stale-local rule, aiplan.c's items 1./2.):
     - 0x2E1D0's best index [ebp-0x14] is written only when an entry has a threat above 0, and read only on the Flood
       path (c == 0x19), whose one caller 0x2D4F0 calls f79270 (0x2D88A) right before it, at the same depth: f79270
       pushes one stack argument, so its frame lies 4 bytes lower and its [ebp-0x1C] (ecx, the text, written at
       0x7928E and never again) is 0x2E1D0's [ebp-0x14] (frames compared: 0x2E1D0 ebp = esp - 20 at the call, f79270
       ebp = esp - 12). So the stale word is the address of 0x2D4F0's text buffer [ebp-0xF0], a stack address in the
       data object (far above any list index), and it is only compared with a list index j < 10 (0x2E365): BI_STALE
       below stands for it (any value >= 157 behaves alike, G). Original behaviour in both modes.
     - 0x24EB8's index [ebp-0x20] is written only when 0x2E1D0's pick is found in its local list: always (D: f28CFC
       kind 0x1E lists only the marked nation v's cities, so the local list is the whole list, and 0x2E1D0 returns a
       list entry). The stale word would be the library's fg_vbclose frame (0x3C720 calls it last); unreachable, the
       port starts at 0.
     - 0x24EB8's second entry [ebp-0x50] when the list holds 2 cities but the local list fewer: unreachable for the
       same reason (local count == list count); the port's local list starts zeroed.
   1995 bugs (fixed unless g_original), graded code / effect:
     1. 0x24EB8 (D / G): the room k the caller computed (9 - Pirate cities, at most 4 or 2) is never read: the computer
        always gives min(cities, 2). With one Pirate slot left the Pirates get a 10th city: f78E74 appends it at byte
        +0x5B of nation 10 (one past the 9-entry city list, the byte before the AST square +0x5C) and the count +0x51
        becomes 10, so the next Piracy computes room = -1 and the 9-city rule is broken (and each later one writes
        further into nation 10's record: +0x5C, the AST square, ...). Fixed: at most k cities (the human chooser
        f2468C gets the same k), so the second city of a two-city list is taken only while k allows, and with k <= 0
        (only a save of an --original game whose Pirates already hold 10 cities gives it) none at all.
     2. 0x21E8C (D / G): when the token branch finds no record (every candidate is a city and the city branch was not
        taken: n has 9 cities, or board tokens / 2 < cities + 1) it converts the territory of the uninitialised stack
        record 0 ([ebp-0x858]). That word is 2,136 bytes under the caller's frame: whatever the deepest earlier call
        left there (the previous computer's own record 0 only if no call in between reached that depth; f553C0's
        catalog / AIL loaders and the human screen 0x21A70's Fastgraph calls do, through library frames the port does
        not have), so it cannot be modelled; a wild value also indexes the name table 0xC77D8 (a crash). Fixed: no
        conversion and no message. --original: the port's stand-in record {t 0, 0, 0} (territory 0 converted).
   Kept as the original (AI choices, deterministic; D code unless marked):
     - 0x213C0's "coastal" loop (0x216E3) starts at j = 0 and runs while j < list[j] (the list's own words, not its
       count), adding n's tokens in the territory numbered like the count, then in each neighbour while its number
       exceeds its index, and on past the record into the next one's words (the adjacency blocks keep the records back
       to back, gfxload.c, as the original's heap block does). Read through the port's block, byte-exact; past the
       block's end the original reads its heap (port guard: 0, which ends the loop; G: unreachable, the last coastal
       record is territory 159, a second coast without a city). The sums are int8 and wrap; the index j is int8.
     - 0x20C70 scans index 9 of the 9-entry city list (byte +0x5B, 0 unless bug 1 above put a 10th Pirate city there).
     - 0x24EB8 tests the list count [0xF7C9C] == 2 (not its own count) for the second city, and appends the list's
       entry 1 ([0xF7CA4]) to the Pirates while it removes and marks its local entry 1 (the same territory, D).
     - 0x2E1D0's Flood fallback uses the index left at the count: it compares the cities with list[count] (the data
       object's word past the list, read there) and otherwise returns list[0].
     - 0x201B8 sums the land list's words 0..count-1 (the count word read as a territory, the last neighbour left
       out), counts equal scores only among the first count-1 entries, and with no score above 0 returns the word
       before the list, [0xF7C9C] (the count, read through the data object).
     - 0x1EF28 reads area record t and territory state t - 1 for each volcano word t (the eruption code uses t + 1 and
       t), so it looks at W. MEDITERRANEAN, TYRAS, PALERMO and C. MEDITERRANEAN instead of CAMPANIA, NEAPOLIS, MILAZZO
       and SYRACUSE, and never at THERA (words 0xBDE18..0xBDE1F only); its sums are int8. Its tie test of volcanoes
       0 and 1 adds the tokens around Vesuvius (area records 34, 35, 54: ROME, SAMNIUM, TARENTUM) against the last
       stack in PALERMO (record 76, assigned, not added), skipping nations only through the first loop's index left
       at 2 (so every nation counts, n's own tokens too, except that nothing counts when n == 3). Record 76 is also
       the first loop's record for the MILAZZO word, so each foreign stack there already adds 2 to the sum (one such
       stack alone makes it 2 and returns 1); only n's own stack there gets as far as the tie test. A city at a
       looked-at territory state adds v for every nation k without a token add (11 v in all).
     - 0x21E8C's token branch tests "in play" (+0x9A) for nation i + 1, i the candidate's index (0x2227A loads
       [ebp-0x28], the candidate loop's index, where the nation loop's [ebp-0x24] is meant): a candidate's stacks are
       looked at only when the nation numbered like its index + 1 is in play; from index 13 on the byte is read past
       the 14 nation records (0xF14CC + 199 (i + 1) + 0x9A, still in the data object, read there). Every stack of
       the first 9 nations is looked at once the test passes, in play or not.
     - 0x21E8C's token conversion also runs k + 1 == n (f57CE8 / f57E28 on n's own tokens cancel; G: unreachable, the
       candidates exclude n's territories); the city branch's records carry no nation / count (only swapped).
   Texts: "%s converts %s by Montheism!" x2 (0xB1EEA, 0xB1F07): Erik's ruling (19 Nov), fixed unless g_original
   through mono_converts_fmt. The other two ("%s is overrun by the Pirates." x3 in this module's data, "Pirates overrun
   %s as a secondary victim.") have no slip.
   Easter eggs: none in these routines (their strings are plain messages, D). Oddities: the volcano off-by-one and
   the hard-coded Vesuvius / Etna neighbours above; the triplicated "%s is overrun by the Pirates." (M6_PLAN egg 28). */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "aical1.h"
#include "cal1.h"
#include "game.h"
#include "kit.h"
#include "mono.h"
#include "move.h"
#include "rules.h"
#include "state.h"

#define NAT(n)       (0xF14CCu + 199u * (uint32_t)(n))
#define AREA(r)      (0xE31B8u + 69u * (uint32_t)(r))            /* area record r = territory r - 1 */
#define TS(t)        (0xE2C18u + 3u * (uint32_t)(t))              /* territory state t */
#define TERR(t)      (0xE1638u + 35u * (uint32_t)(t))             /* territory record t */
#define LAND_M1      0xF8E78u    /* [0xF8E78 + 4c]: the land list of territory c - 1 (0xF8E7C indexed by t) */
#define COAST_M1     0xF90F8u    /* [0xF90F8 + 4c]: the coastal list of territory c - 1 (0xF90FC indexed by t) */
#define LANDADJ      0xF8E7Cu
#define CHOICE_N     0xF7C9Cu    /* int32 */
#define CHOICE       0xF7CA0u    /* int32 [157] */
#define MARKS        0xF7AC4u    /* uint8 [12]: nations whose cities f28CFC(.., 0x1E) lists */
#define OPTIONS      0xF0980u    /* byte: bit 0x04 = the messages wait for CONTINUE */
#define ROUND        0xF40FCu    /* int16 */
#define PAIRS        0xF7300u    /* 8-byte Earthquake pairs */
#define PAIR_TMP     0xF77D8u    /* the 8-byte swap temp */
#define SCORE(n, t)  (0xEED84u + 160u * (uint32_t)(n) + (uint32_t)(t))
#define ATT(n, m)    (0xF52E0u + 40u * (uint32_t)(n) + 4u * (uint32_t)(m))   /* int32 [n][10] */
#define PREF(n, q)   (0xC0618u + 16u * (uint32_t)(n) + 4u * (uint32_t)(q))   /* int32 [n][4] */
#define NAME(n)      DSTR(D32(0xC768Cu + 4u * (uint32_t)(n)))
#define TERR_NAME(t) DSTR(D32(0xC77D8u + 4u * (uint32_t)(t)))
#define BI_STALE     0x7FFFFFFF  /* 0x2E1D0's [ebp-0x14] before it is written: a stack address (file comment) */

/* ---- data helpers --------------------------------------------------------------------------------------------- */

/* bytes / words at computed addresses: the reads the original makes with indices that may leave their table (risk 8)
   go through the data object as the original's do; outside it the port reads 0 (port guard, G: unreachable) */
static uint32_t ub(uint32_t a)  { return ds_valid(a, 1) ? D8(a) : 0u; }
static int32_t  sb(uint32_t a)  { return ds_valid(a, 1) ? S8(a) : 0; }
static int32_t  sd(uint32_t a)  { return ds_valid(a, 4) ? S32(a) : 0; }
static int32_t  ch(int32_t i)   { return sd(CHOICE + 4u * (uint32_t)i); }   /* dword [4i + 0xF7CA0] */
static void     wb(uint32_t a, uint8_t v) { if (ds_valid(a, 1)) setD8(a, v); }

/* the int16 at word k of an adjacency list (movsx word [h + 2k]): the lists live in heap blocks (port handles,
   game.h); a value that is a data-object address (the list of territory -1, [0xF8E78]) is read there */
static int32_t lword(uint32_t h, int32_t k)
{
    uint32_t a = h + 2u * (uint32_t)k;
    const uint8_t *p;

    if ((h & 0x80000000u) != 0) {
        if (((h ^ a) & 0xFFF00000u) != 0)   /* port guard: an offset leaving its block's handle range */
            return 0;
        p = load_heap_ptr(a, 2);
        return p != NULL ? (int16_t)(p[0] | p[1] << 8) : 0;   /* port guard past the block (file comment) */
    }
    return ds_valid(a, 2) ? S16(a) : 0;
}

static int32_t poplimit(int32_t t)   /* mov al, [9t*4 - t + 0xE1639]; shl al, 2; shr al, 5 */
{
    return (int32_t)(((ub(TERR(t) + 1u) << 2) & 0xFFu) >> 5);
}

/* the marks of a city taken by the Pirates (0x24FC3..0x25019 and its twins): area t+1 +0x3B + owner = 2, and every
   +0x3C + j (j 0..8) that is 1 becomes 2 */
static void pirate_marks(int32_t t, int32_t owner)
{
    int32_t j;

    wb(AREA(t + 1) + 0x3Bu + (uint32_t)owner, 2);                                      /* 0x24FC3 */
    for (j = 0; j < 9; j++)                                                            /* 0x24FD7 */
        if (sb(AREA(t + 1) + 0x3Cu + (uint32_t)j) == 1)                                /* 0x24FF0 dword sar 24 */
            wb(AREA(t + 1) + 0x3Cu + (uint32_t)j, 2);                                  /* 0x25012 */
}

static int pirate_wait(void)   /* 0x25037: bit 0x04 of byte [0xF0980]: -500 (CONTINUE), else 2500 ms */
{
    return (D8(OPTIONS) & 0x04u) != 0 ? -500 : 2500;
}

/* ---- 0x20C70 Treachery ------------------------------------------------------------------------------------------- */

int f20C70(int p, int v)
{
    uint32_t np = NAT(p);   /* [ebp-0xC] */
    uint32_t nv = NAT(v);   /* [ebp-0x8] */
    int32_t i;              /* [ebp-0x24] */
    int32_t j;              /* [ebp-0x20] */
    int32_t c, d;
    uint32_t h;             /* [ebp-0x18] / [ebp-0x14] */

    if (p == v)                                                                        /* 0x20CAF */
        return f213C0(S8(nv + 0x3Du));                                                 /* 0x20FA0 dword [+0x3A] sar 24 */
    if (9 - S8(np + 0x51u) != 0) {                                                     /* 0x20CC3; je 0x20E84 */
        for (i = 9; i >= 0; i--) {                                                     /* 0x20CCB, 0x20CDA jl 0x20D74 */
            c = (int32_t)D8(nv + (uint32_t)i + 0x52u);                                 /* 0x20CEA */
            if (c <= 0)                                                                /* 0x20CF4 jle */
                continue;
            h = D32(LAND_M1 + 4u * (uint32_t)c);                                       /* 0x20D04 */
            for (j = 1; lword(h, 0) + 1 > j; j++)                                      /* 0x20D0E, 0x20D24 jle */
                if (ub(AREA(lword(h, j) + 1) + 1u + (uint32_t)S8(np + 0x3Du)) != 0)    /* 0x20D59; test dl, dl */
                    return (int32_t)D8(nv + (uint32_t)i + 0x52u);                      /* 0x20D66 */
        }
        for (i = 9; i >= 0; i--) {                                                     /* 0x20D74, 0x20D83 jl 0x20E1D */
            c = (int32_t)D8(nv + (uint32_t)i + 0x52u);                                 /* 0x20D93 */
            if (c <= 0)                                                                /* 0x20D9D */
                continue;
            h = D32(COAST_M1 + 4u * (uint32_t)c);                                      /* 0x20DAD */
            for (j = 1; lword(h, 0) + 1 > j; j++)                                      /* 0x20DB7, 0x20DCD */
                if (ub(AREA(lword(h, j) + 1) + 1u + (uint32_t)S8(np + 0x3Du)) != 0)    /* 0x20E02; test dh, dh */
                    return (int32_t)D8(nv + (uint32_t)i + 0x52u);                      /* 0x20E0F */
        }
        for (i = 9; i >= 0; i--) {                                                     /* 0x20E1D, 0x20E2C jl 0x20FA7 */
            c = (int32_t)D8(nv + (uint32_t)i + 0x52u);                                 /* 0x20E3C */
            if (c <= 0)                                                                /* 0x20E46 */
                continue;
            if ((ub(TERR(c - 1)) & 0x02u) != 0)                                        /* 0x20E67 */
                return (int32_t)D8(nv + (uint32_t)i + 0x52u);                          /* 0x20E76 */
        }
    } else if (S8(nv + 0x5Cu) > S8(np + 0x5Cu)) {                                      /* 0x20E90 cmp al, bh; setg */
        for (i = 0; S8(nv + 0x51u) > i; i++) {                                         /* 0x20EA9, 0x20EBD jle 0x20FA7 */
            d = (int32_t)D8(nv + (uint32_t)i + 0x52u);                                 /* 0x20ECC */
            if ((ub(TERR((int32_t)D8(AREA(d)) - 1) + 1u) & 0x40u) == 0)                /* 0x20F06; jne 0x20EAE */
                return (int32_t)D8(nv + (uint32_t)i + 0x52u);                          /* 0x20F15 */
        }
    } else {
        for (i = S8(nv + 0x51u) - 1; i >= 0; i--) {                                    /* 0x20F2D, 0x20F38 jl 0x20FA7 */
            d = (int32_t)D8(nv + (uint32_t)i + 0x52u);                                 /* 0x20F46 */
            if ((ub(TERR((int32_t)D8(AREA(d)) - 1) + 1u) & 0x40u) != 0)                /* 0x20F7A; je 0x20F32 */
                return (int32_t)D8(nv + (uint32_t)i + 0x52u);                          /* 0x20F89 */
        }
    }
    return (int32_t)D8(nv + (uint32_t)S8(nv + 0x51u) + 0x51u);                         /* 0x20FB3 the last entry */
}

/* ---- 0x213C0 the own city to reduce (frame: ebp - 0x82, positive offsets) ---------------------------------------- */

int f213C0(int n_in)
{
    int8_t n = (int8_t)n_in;   /* [ebp+0x76] mov byte, al */
    uint32_t nat;              /* [ebp+0x5E] */
    int32_t city[9];           /* [ebp+4i-0xA] */
    int32_t lim[9];            /* [ebp+4i+0x1A] */
    int32_t a, b;              /* [ebp+0x46], [ebp+0x4A] */
    int32_t ia, ib;            /* [ebp+0x3E], [ebp+0x42] */
    int32_t sc, sl;            /* [ebp+0x56], [ebp+0x5A] */
    int8_t i, j;               /* [ebp+0x6E], [ebp+0x7A] */
    int8_t sum, suma, sumb;    /* [ebp+0x6A], [ebp+0x7E], [ebp+0x72] */
    uint32_t ar, h;            /* [ebp+0x62], [ebp+0x4E] / [ebp+0x52] */

    nat = NAT(n);                                                                      /* 0x213F2 */
    for (i = 0; i < 9; i++) {                                                          /* 0x213F7, 0x2141B jge */
        if (ub(nat + (uint32_t)i + 0x52u) == 0) {                                      /* 0x21427 */
            city[i] = 0;                                                               /* 0x21402 */
            lim[i] = 0;                                                                /* 0x2140A */
            continue;
        }
        city[i] = (int32_t)ub(nat + (uint32_t)i + 0x52u);                              /* 0x2143B */
        lim[i] = poplimit(city[i] - 1);                                                /* 0x21473 */
    }
    for (i = 0; i < 9; i++)                                                            /* 0x2147B, 0x2148D jge 0x2153C */
        for (j = 1; j < 9; j++) {                                                      /* 0x21493, 0x214A6 */
            a = lim[j - 1];                                                            /* 0x214B0 */
            b = lim[j];                                                                /* 0x214BB */
            if (b <= a)                                                                /* 0x214C6 jle */
                continue;
            sc = city[j - 1];                                                          /* 0x214D0 */
            sl = lim[j - 1];                                                           /* 0x214DB */
            city[j - 1] = city[j];                                                     /* 0x214F8 */
            lim[j - 1] = lim[j];                                                       /* 0x2150F */
            city[j] = sc;                                                              /* 0x21521 */
            lim[j] = sl;                                                               /* 0x21533 */
        }
    a = lim[0];                                                                        /* 0x2153F */
    b = lim[1];                                                                        /* 0x21545 */
    ia = 0;                                                                            /* 0x2154A */
    ib = 1;                                                                            /* 0x21552 */
    if (a > b && (ub(TERR(city[0] - 1) + 1u) & 0x40u) != 0)                            /* 0x2155B, 0x21570 */
        return city[0];                                                                /* 0x21579 */
    if ((ub(TERR(city[0] - 1) + 1u) & 0x40u) == 0 && city[1] != 0                      /* 0x21599, 0x215A5 */
        && (ub(TERR(city[1] - 1) + 1u) & 0x40u) != 0) {                                /* 0x215BB */
        a = lim[1];                                                                    /* 0x215C7 */
        b = lim[2];                                                                    /* 0x215CD */
        ia = 1;                                                                        /* 0x215D0 */
        ib = 2;                                                                        /* 0x215D7 */
        if (a > b && (ub(TERR(city[1] - 1) + 1u) & 0x40u) != 0)                        /* 0x215E6, 0x215FA */
            return city[1];                                                            /* 0x21603 */
    }
    if (city[1] == 0)                                                                  /* 0x21616 */
        return city[0];                                                                /* 0x2161A */
    sumb = 0;                                                                          /* 0x2162C */
    suma = 0;                                                                          /* 0x21632 */
    for (i = 0; i < 2; i++) {                                                          /* 0x21635, 0x21752 jge 0x21773 */
        ar = AREA(i != 0 ? city[ib] : city[ia]);                                       /* 0x2163D / 0x21762, 0x21654 */
        sum = 0;                                                                       /* 0x21659 */
        h = D32(LAND_M1 + 4u * ub(ar));                                                /* 0x21666 */
        for (j = 1; j < lword(h, 0) + 1; j++)                                          /* 0x21670, 0x21687 jge */
            sum = (int8_t)(sum + (int8_t)ub(AREA(lword(h, j) + 1) + 0x0Eu + (uint32_t)sb(nat + 0x3Du)));   /* 0x216BD */
        h = D32(COAST_M1 + 4u * ub(ar));                                               /* 0x216CC */
        for (j = 0; j < lword(h, j); j++)                                              /* 0x216D8, 0x216F3: j < list[j] */
            sum = (int8_t)(sum + (int8_t)ub(AREA(lword(h, j) + 1) + 0x0Eu + (uint32_t)sb(nat + 0x3Du)));   /* 0x2172B */
        if (i == 0)                                                                    /* 0x21730 */
            suma = sum;                                                                /* 0x21739 */
        else
            sumb = sum;                                                                /* 0x2176E */
    }
    if (suma > sumb)                                                                   /* 0x21776 cmp al, byte; jle */
        return city[ia];                                                               /* 0x2177B */
    return city[ib];                                                                   /* 0x2178C */
}

/* ---- 0x24EB8 Piracy, primary victim ------------------------------------------------------------------------------ */

void f24EB8(int c, int v, int k)
{
    char buf[100];          /* [ebp-0xB8]..[ebp-0x55] */
    int32_t local[157];     /* [ebp-0x54] int32[9] (port: room for the whole list; G: at most 9 cities) */
    int32_t m;              /* [ebp-0x10] */
    int32_t i;              /* [ebp-0x24] / [ebp-0x14] */
    int32_t take;           /* [ebp-0x1C] */
    int32_t t;              /* [ebp-0x18] */
    int32_t idx = 0;        /* [ebp-0x20] (unreachable unwritten, file comment) */
    int32_t j;

    (void)c;                                                                           /* [ebp-0x2C] never read */
    memset(local, 0, sizeof local);   /* port: the unreachable unwritten entries (file comment) */
    m = 0;                                                                             /* 0x24ED9 */
    for (i = 0; i < 12; i++)                                                           /* 0x24EDC, 0x24EE7 */
        setD8(MARKS + (uint32_t)i, 0);                                                 /* 0x24EF2 */
    setD8(MARKS + (uint32_t)v, 1);                                                     /* 0x24EFD */
    f28CFC(v, 0x1E, 0);                                                                /* 0x24F10 */
    for (i = 0; i < S32(CHOICE_N); i++)                                                /* 0x24F15, 0x24F23 */
        if ((int32_t)ub(TS(ch(i))) == v && m < 157)                                    /* 0x24F49 (port guard) */
            local[m++] = ch(i);                                                        /* 0x24F65 */
    take = m > 2 ? 2 : m;                                                              /* 0x24F6B..0x24F7B */
    /* BUG(1995): the room k is never read; the Pirates could get a 10th city (file comment, bug 1) */
    if (!g_original && take > k)
        take = k;
    if (m >= 1 && m <= 2 && (g_original || take > 0)) {                               /* 0x24F7E, 0x24F88 (fix: bug 1) */
        t = local[0];                                                                  /* 0x24F95 */
        f78F04(t, v);                                                                  /* 0x24F9E */
        f78E74(local[0], 10);                                                          /* 0x24FAB */
        pirate_marks(local[0], v);                                                     /* 0x24FC3..0x25019 */
        snprintf(buf, sizeof buf, "%s is overrun by the Pirates.", TERR_NAME(t));      /* 0x25032 (0xB28CF) */
        f79270(pirate_wait(), v, t, buf, 1);                                           /* 0x2506D */
        take--;                                                                        /* 0x25075 */
    }
    if (S32(CHOICE_N) == 2 && (g_original || take > 0)) {                             /* 0x25078 (fix: bug 1) */
        t = local[1];                                                                  /* 0x25088 */
        f78F04(t, v);                                                                  /* 0x25091 */
        f78E74(S32(CHOICE + 4u), 10);                                                  /* 0x250A0 [0xF7CA4] */
        pirate_marks(local[1], v);                                                     /* 0x250B8..0x2510E */
        snprintf(buf, sizeof buf, "%s is overrun by the Pirates.", TERR_NAME(t));      /* 0x25127 (0xB28ED) */
        f79270(pirate_wait(), v, t, buf, 1);                                           /* 0x25162 */
        take--;                                                                        /* 0x2516A */
        return;
    }
    for (i = 0; i < take; i++) {                                                       /* 0x25175, 0x2528E jge */
        t = f2E1D0(v, 0x1E);                                                           /* 0x2529F */
        f78F04(t, v);                                                                  /* 0x252AF */
        for (j = 0; j < S32(CHOICE_N); j++)                                            /* 0x252B4, 0x25188 */
            if (local[j < 157 ? j : 0] == t) {                                         /* 0x2519A (port guard) */
                idx = j;                                                               /* 0x251A1 */
                break;
            }
        f78E74(local[idx], 10);                                                        /* 0x251B0 */
        pirate_marks(local[idx], v);                                                   /* 0x251D0..0x2522C */
        snprintf(buf, sizeof buf, "%s is overrun by the Pirates.", TERR_NAME(t));      /* 0x25245 (0xB290B) */
        f79270(pirate_wait(), v, t, buf, 1);                                           /* 0x25280 */
    }
}

/* ---- 0x252BC Piracy, secondary victims -------------------------------------------------------------------------- */

void f252BC(int c, int v, int k)
{
    char buf[100];          /* [ebp-0xC4]..[ebp-0x61] */
    int32_t list[10];       /* [ebp-0x60] */
    int32_t cnt;            /* [ebp-0x1C] */
    int32_t m;              /* [ebp-0x28] */
    int32_t i, j;           /* [ebp-0x28], [ebp-0x30] */
    int32_t sw;             /* [ebp-0x24] */
    int32_t left;           /* [ebp-0x14] */
    int32_t idx;            /* [ebp-0x18] */
    int32_t t;              /* [ebp-0x20] */
    int32_t owner;          /* [ebp-0x10] */

    memset(list, 0, sizeof list);                                                      /* 0x252E5 memset(.., 0, 0x28) */
    cnt = 0;                                                                           /* 0x252EC */
    for (m = 1; m <= 9; m++) {                                                         /* 0x252EF, 0x252FE jg */
        if (D8(NAT(m) + 0x9Au) == 0)                                                   /* 0x2530B byte [199m + 0xF1566] */
            continue;
        if (S8(NAT(m) + 0x51u) <= 0)                                                   /* 0x25322 sar 24; jle */
            continue;
        if (m == v || m == c)                                                          /* 0x2532C, 0x25334 */
            continue;
        list[cnt++] = m;                                                               /* 0x25349 */
    }
    for (i = cnt - 2; i >= 0; i--)                                                     /* 0x25352, 0x2536C */
        for (j = 0; j <= i; j++)                                                       /* 0x25364, 0x25389 jg */
            if (S32(ATT(v, list[j + 1])) > S32(ATT(v, list[j]))) {                     /* 0x253D1 jle */
                sw = list[j + 1];                                                      /* 0x253DC */
                list[j + 1] = list[j];                                                 /* 0x253E9 */
                list[j] = sw;                                                          /* 0x253F3 */
            }
    left = k;                                                                          /* 0x253FC */
    for (idx = 0; left > 0 && idx < cnt; idx++) {                                      /* 0x25401, 0x25404, 0x2540E */
        t = f25548(list[idx]);                                                         /* 0x25421 */
        if (t < 0)                                                                     /* 0x2542E jl 0x25532 */
            continue;
        owner = (int32_t)ub(TS(t));                                                    /* 0x25444 */
        f78F04(t, list[idx]);                                                          /* 0x25457 */
        f78E74(t, 10);                                                                 /* 0x25464 */
        pirate_marks(t, owner);                                                        /* 0x25480..0x254D3 */
        snprintf(buf, sizeof buf, "Pirates overrun %s as a secondary victim.", TERR_NAME(t));   /* 0x254EC (0xB2929) */
        f79270(pirate_wait(), v, t, buf, 1);                                           /* 0x25527 */
        left--;                                                                        /* 0x2552F */
    }
}

/* ---- 0x25548 Piracy, the city of a secondary victim -------------------------------------------------------------- */

typedef struct { int32_t t, sum; } PRec;   /* 8-byte stack records [ebp + 8i - 0x530] */

/* the two bubble sorts of 0x256A3 / 0x2575C (and 0x258FF / 0x259B8): limit ascending (unsigned bytes, jbe), then
   sum ascending (signed, jle); each swaps neighbours from the end down to i */
static void prec_sort(PRec *r, int32_t m)
{
    int32_t i, j;
    PRec tmp;   /* [ebp-0x30] */

    for (i = 1; i < m; i++)                                                            /* 0x25694, 0x256A9 jge */
        for (j = m - 1; j >= i; j--)                                                   /* 0x256B3, 0x256C4 jl */
            if ((uint8_t)poplimit(r[j - 1].t) > (uint8_t)poplimit(r[j].t)) {           /* 0x25710 cmp bl, al; jbe */
                tmp = r[j - 1];                                                        /* 0x25721 */
                r[j - 1] = r[j];                                                       /* 0x25737 */
                r[j] = tmp;                                                            /* 0x25746 */
            }
    for (i = 1; i < m; i++)                                                            /* 0x2574D, 0x25762 jge 0x257C9 */
        for (j = m - 1; j >= i; j--)                                                   /* 0x25764, 0x25779 */
            if (r[j - 1].sum > r[j].sum) {                                             /* 0x2578F jle */
                tmp = r[j - 1];                                                        /* 0x257A0 */
                r[j - 1] = r[j];                                                       /* 0x257B6 */
                r[j] = tmp;                                                            /* 0x257C5 */
            }
}

int f25548(int n)
{
    PRec rec[160];          /* [ebp-0x530]..[ebp-0x30] */
    uint32_t nat = NAT(n);  /* [ebp-0x8] */
    int32_t m;              /* [ebp-0x24] */
    int32_t i;              /* [ebp-0x20] */
    int32_t k;              /* [ebp-0x1C] */
    int32_t sum;            /* [ebp-0x18] */
    int32_t c, pass;
    uint32_t h;             /* [ebp-0x4] */

    memset(rec, 0, sizeof rec);   /* port: entries past m are never read */
    for (pass = 0; pass < 2; pass++) {                                                 /* 0x2557A / 0x257D7 */
        m = 0;
        for (i = 0; S8(nat + 0x51u) > i; i++) {                                        /* 0x255E3 / 0x25840 */
            c = (int32_t)D8(nat + (uint32_t)i + 0x52u);
            if ((ub(TERR(c - 1)) & 0x02u) == 0)                                        /* 0x25615 / 0x25871 */
                continue;
            if ((ub(TS(c - 1) + 1u) == (uint32_t)S16(ROUND)) != (pass == 0))           /* 0x25648 jne / 0x258A4 je */
                continue;
            if (m >= 160)   /* port guard (G: at most 9 cities) */
                break;
            rec[m].t = c - 1;                                                          /* 0x2565E / 0x258BA */
            h = D32(LANDADJ + 4u * (uint32_t)rec[m].t);                                /* 0x2566F / 0x258CB */
            sum = 0;                                                                   /* 0x2567B / 0x258D7 */
            for (k = 1; lword(h, 0) >= k; k++)                                         /* 0x2567E, 0x25593 jl */
                sum += sb(AREA(lword(h, k) + 1) + 0x0Eu + (uint32_t)n);                /* 0x255C5 [+ n-1 + 0xE31C4] sar 24 */
            rec[m].sum = sum;                                                          /* 0x255D0 / 0x2582D */
            m++;                                                                       /* 0x255DA / 0x25837 */
        }
        if (m != 0) {                                                                  /* 0x2568E je 0x257D7 / 0x258EA */
            prec_sort(rec, m);
            return rec[0].t;                                                           /* 0x257C9 */
        }
    }
    return -1;                                                                         /* 0x25A29 */
}

/* ---- 0x2E1D0 a computer's city from the list --------------------------------------------------------------------- */

int f2E1D0(int n, int c)
{
    int32_t threat[9];          /* [ebp-0x48] */
    int32_t best = 0;           /* [ebp-0xC] */
    int32_t bi = BI_STALE;      /* [ebp-0x14]: unwritten unless a threat > 0 (file comment) */
    int32_t i;                  /* [ebp-0x20] */
    int32_t j = 0;              /* [ebp-0x10] (k in the threat loop) */
    int32_t q;                  /* [ebp-0x18] */
    uint8_t flag;               /* [ebp-0x4] */
    uint32_t h;                 /* [ebp-0x8] */
    uint32_t nat = NAT(n);

    for (i = 0; i < 9; i++)                                                            /* 0x2E1EE, 0x2E1F9 */
        threat[i] = 0;                                                                 /* 0x2E204 */
    for (i = 0; i < S32(CHOICE_N); i++) {                                              /* 0x2E20C, 0x2E21A */
        h = D32(LANDADJ + 4u * (uint32_t)ch(i));                                       /* 0x2E22C */
        for (j = 1; lword(h, 0) + 1 > j; j++)                                          /* 0x2E236, 0x2E24C jle */
            if (i < 9)   /* port guard: the original's frame holds 9 (G: lists of cities, at most 9) */
                threat[i] += sb(AREA(lword(h, j) + 1) + 0x0Eu + (uint32_t)n);          /* 0x2E27A [+ n + 0xE31C3] sar 24 */
    }
    for (i = 0; i < S32(CHOICE_N); i++)                                                /* 0x2E282, 0x2E290 */
        if (i < 9 && threat[i] > best) {                                               /* 0x2E2A4 jle */
            best = threat[i];                                                          /* 0x2E2AD */
            bi = i;                                                                    /* 0x2E2B3 */
        }
    if (c == 0x19) {                                                                   /* 0x2E2B8 */
        for (i = S8(nat + 0x51u) - 1; i >= 0; i--) {                                   /* 0x2E2D3, 0x2E2DE jl 0x2E37E */
            flag = 0;                                                                  /* 0x2E2EA */
            for (j = 0; j < S32(CHOICE_N); j++) {                                      /* 0x2E2EF, 0x2E2FD jge */
                for (q = 0; q < 4; q++)                                                /* 0x2E307, 0x2E312 */
                    if (sd(PREF(n, q)) == ch(j)) {                                     /* 0x2E336 */
                        flag = 1;                                                      /* 0x2E33A */
                        break;
                    }
                if ((int32_t)D8(nat + (uint32_t)i + 0x52u) != ch(j) + 1)               /* 0x2E35B jne */
                    continue;
                if (flag == 0 || bi == j)                                              /* 0x2E363, 0x2E368 jne */
                    return ch(j);                                                      /* 0x2E370 */
            }
        }
        for (i = S8(nat + 0x51u) - 1; i >= 0; i--)                                     /* 0x2E38F, 0x2E39A jl 0x2E44F */
            if ((int32_t)D8(nat + (uint32_t)i + 0x52u) == ch(j) + 1)                   /* 0x2E3C1: j left at the count */
                return ch(j);                                                          /* 0x2E3C8 */
    } else if (c == 0x1E) {                                                            /* 0x2E3D9 */
        for (i = 0; S8(nat + 0x51u) > i; i++)                                          /* 0x2E3E4, 0x2E3FF jle */
            for (j = 0; j < S32(CHOICE_N); j++)                                        /* 0x2E406, 0x2E414 */
                if ((int32_t)D8(nat + (uint32_t)i + 0x52u) - 1 == ch(j))               /* 0x2E435 */
                    return ch(j);                                                      /* 0x2E441 */
    }
    return S32(CHOICE);                                                                /* 0x2E44F [0xF7CA0] */
}

/* ---- 0x1EF28 the volcano ----------------------------------------------------------------------------------------- */

int f1EF28(int n, int32_t *scores)
{
    int8_t v;        /* [ebp-0x10] */
    int8_t j;        /* [ebp-0x4] */
    int8_t k;        /* [ebp-0x8] */
    int8_t sum;      /* [ebp-0x14] */
    int8_t last;     /* [ebp-0xC] */
    int32_t t;

    last = 0;                                                                          /* 0x1EF44 */
    sum = 0;                                                                           /* 0x1EF4A */
    j = 0;
    for (v = 1; v < 3; v++)                                                            /* 0x1EF4D, 0x1EF60 jge 0x1F08F */
        for (j = 0; j < 2; j++)                                                        /* 0x1EF68, 0x1EF7A */
            for (k = 0; k < 11; k++) {                                                 /* 0x1EF7E, 0x1EF90 */
                t = S16(0xBDE14u + 4u * (uint32_t)v + 2u * (uint32_t)j);               /* 0x1EFA1 dword [..+0xBDE12] sar 16 */
                if (sb(AREA(t) + 0x0Fu + (uint32_t)k) > 0 && k + 1 != n) {             /* 0x1EFC7 jle, 0x1EFD1 jne */
                    sum = (int8_t)(sum + v);                                           /* 0x1F087 */
                    continue;
                }
                if ((ub(TS(t - 1) + 2u) & 1u) == 1u                                    /* 0x1F009 */
                    && (int32_t)ub(TS(t - 1)) != n                                     /* 0x1F042 */
                    && ub(TS(t - 1)) != 10u)                                           /* 0x1F07B */
                    sum = (int8_t)(sum + v);                                           /* 0x1F087 */
            }
    if (sum == 1)                                                                      /* 0x1F093 */
        return 0;                                                                      /* 0x1F098 */
    if (sum == 2)                                                                      /* 0x1F0A3 */
        return 1;                                                                      /* 0x1F0A8 */
    if (scores[0] == scores[1] && scores[0] == scores[2]) {                            /* 0x1F0C0, 0x1F0CC */
        if (n != 5)                                                                    /* 0x1F0D4 */
            return 2;                                                                  /* 0x1F24E */
        if (n == 1)                                                                    /* 0x1F0E0 (dead: n is 5) */
            return 0;
        return 1;                                                                      /* 0x1F0E5 */
    }
    if (scores[0] == scores[1]) {                                                      /* 0x1F0FD */
        sum = 0;                                                                       /* 0x1F107 */
        last = 0;                                                                      /* 0x1F10A */
        for (v = 0; v < 9; v++)                                                        /* 0x1F10D, 0x1F11F (the index [ebp-0x10]) */
            if (sb(0xE3AF1u + (uint32_t)v) > 0 && j + 1 != n)                          /* 0x1F130, 0x1F137: j is 2 */
                sum = (int8_t)(sum + (int8_t)D8(0xE3AF1u + (uint32_t)v));              /* 0x1F146 area 34 */
        for (v = 0; v < 9; v++)                                                        /* 0x1F14D */
            if (sb(0xE3B36u + (uint32_t)v) > 0 && j + 1 != n)                          /* 0x1F170, 0x1F177 */
                sum = (int8_t)(sum + (int8_t)D8(0xE3B36u + (uint32_t)v));              /* 0x1F186 area 35 */
        for (v = 0; v < 9; v++)                                                        /* 0x1F18D */
            if (sb(0xE4055u + (uint32_t)v) > 0 && j + 1 != n)                          /* 0x1F1B0, 0x1F1B7 */
                sum = (int8_t)(sum + (int8_t)D8(0xE4055u + (uint32_t)v));              /* 0x1F1C6 area 54 */
        for (v = 0; v < 9; v++)                                                        /* 0x1F1CD */
            if (sb(0xE4643u + (uint32_t)v) > 0 && j + 1 != n)                          /* 0x1F1F0, 0x1F1F7 */
                last = (int8_t)D8(0xE4643u + (uint32_t)v);                             /* 0x1F206 area 76 (assigned) */
        if (sum <= last)                                                               /* 0x1F20E cmp al, byte; jle */
            return 1;                                                                  /* 0x1F0E5 */
        return 0;                                                                      /* 0x1F217 */
    }
    if (scores[0] == scores[2]) {                                                      /* 0x1F22C */
        if (n != 5)                                                                    /* 0x1F233 */
            return 2;                                                                  /* 0x1F24E */
        return scores[0] ^ scores[2];                                                  /* 0x1F238 xor eax, ebx: 0 */
    }
    if (n == 5)                                                                        /* 0x1F245 */
        return 1;                                                                      /* 0x1F0E5 */
    return 2;                                                                          /* 0x1F24E */
}

/* ---- 0x20068 the Earthquake pairs -------------------------------------------------------------------------------- */

static void pair_swap(int32_t j)   /* 0x200E1..0x20119: rec[j+1] -> temp, rec[j] -> rec[j+1], temp -> rec[j] */
{
    uint32_t a = PAIRS + 8u * (uint32_t)j;

    if (!ds_valid(a, 16))   /* port guard (G: at most 155 records fit before the temp) */
        return;
    memmove(DP(PAIR_TMP), DP(a + 8u), 8);   /* 0x200F0 movsd x2 (memmove: j = 154 is the temp itself, G: unreachable) */
    memmove(DP(a + 8u), DP(a), 8);                                                     /* 0x20106 */
    memmove(DP(a), DP(PAIR_TMP), 8);                                                   /* 0x20117 */
}

void f20068(int n)
{
    int32_t i;   /* [ebp-0x8] */
    int32_t j;   /* [ebp-0x4] */

    (void)n;                                                                           /* [ebp-0xC] never read */
    if (S32(CHOICE_N) <= 1)                                                            /* 0x20085 jle 0x201AD */
        return;
    for (i = S32(CHOICE_N) - 2; i >= 0; i--)                                           /* 0x2008E, 0x200A8 */
        for (j = 0; j <= i; j++)                                                       /* 0x200A0, 0x200C4 jg */
            if (S16(PAIRS + 8u * (uint32_t)j + 14u) > S16(PAIRS + 8u * (uint32_t)j + 6u))   /* 0x200DC cmp ax, dx; jle */
                pair_swap(j);
    for (i = S32(CHOICE_N) - 2; i >= 0; i--)                                           /* 0x20120, 0x2013A */
        for (j = 0; j <= i; j++)                                                       /* 0x20132, 0x20156 */
            if (S16(PAIRS + 8u * (uint32_t)j + 12u) > S16(PAIRS + 8u * (uint32_t)j + 4u))   /* 0x2016E cmp ax, bx; jle */
                pair_swap(j);
}

/* ---- 0x201B8 the Earthquake's city without a pair ---------------------------------------------------------------- */

int f201B8(int n)
{
    int16_t score[157];     /* [ebp-0xD0] int16[90] (port: room for the whole list; G: at most 9 cities) */
    int32_t idx = -1;       /* [ebp-0x14] */
    int32_t dups = 0;       /* [ebp-0x8] */
    int32_t best = 0;       /* [ebp-0x4]: compared as its low word */
    int32_t i;              /* [ebp-0x10] */
    int32_t k;              /* [ebp-0x18] */
    int32_t cnt = S32(CHOICE_N) < 157 ? S32(CHOICE_N) : 157;   /* port guard on the array */
    uint32_t h;             /* [ebp-0xC] */

    memset(score, 0, sizeof score);                                                    /* 0x201E7..0x20202 (90 words) */
    for (i = 0; i < S32(CHOICE_N) && i < cnt; i++) {                                   /* 0x20206, 0x20214 */
        h = D32(LANDADJ + 4u * (uint32_t)ch(i));                                       /* 0x20226 */
        for (k = 0; lword(h, 0) > k; k++)                                              /* 0x20230, 0x20246 jle: from 0 */
            score[i] = (int16_t)(score[i] + (int16_t)sb(AREA(lword(h, k) + 1) + 0x0Eu + (uint32_t)n));   /* 0x2026E */
    }
    for (i = 0; S32(CHOICE_N) - 1 > i && i < cnt; i++)                                 /* 0x20278, 0x2028B jle */
        for (k = 0; S32(CHOICE_N) - 1 > k && k < cnt; k++)                             /* 0x20290, 0x202A6 */
            if (score[i] == score[k] && i != k)                                        /* 0x202BE, 0x202C6 */
                dups++;                                                                /* 0x202CE */
    if ((int16_t)dups == 0) {                                                          /* 0x202D3 movsx word; jne 0x20364 */
        for (i = 0; i < S32(CHOICE_N) && i < cnt; i++)                                 /* 0x202DF, 0x202ED */
            if (score[i] == 0)                                                         /* 0x20302 dword [2i - 0xD2] sar 16 */
                idx = i;                                                               /* 0x20309 */
        if (idx < 0) {                                                                 /* 0x2030E jl 0x20323 */
            for (i = 0; i < S32(CHOICE_N) && i < cnt; i++)                             /* 0x20325, 0x20333 jge 0x20314 */
                if (score[i] > (int16_t)best) {                                        /* 0x20349 cmp ax, dx; jle */
                    best = (int32_t)(uint16_t)score[i];                                /* 0x20359 (high word 0: i's) */
                    idx = i;                                                           /* 0x2035F */
                }
        }
    } else {
        idx = 0;                                                                       /* 0x20366 */
        for (i = 0; i < S32(CHOICE_N) && i < cnt; i++)                                 /* 0x20369, 0x20377 */
            if (score[i] > (int16_t)best) {                                            /* 0x2038D cmp ax, bx; jle */
                best = (int32_t)(uint16_t)score[i];                                    /* 0x2039D */
                idx = i;                                                               /* 0x203A3 */
            }
    }
    return ch(idx);                                                                    /* 0x20317: idx -1 reads [0xF7C9C] */
}

/* ---- 0x21E8C Monotheism ------------------------------------------------------------------------------------------ */

typedef struct { int32_t t, nat, cnt; } MRec;   /* 12-byte stack records [ebp + 12i - 0x858] */

static void mrec_swap(MRec *r, int32_t j)   /* the movsd triples through [ebp-0x40] */
{
    MRec tmp = r[j + 1];
    r[j + 1] = r[j];
    r[j] = tmp;
}

/* the bubble passes: i = m - 2 down to 0, j = 0..i, swap when key(j + 1) > key(j) */
static void mrec_sort_score(MRec *r, int32_t m, int32_t n)   /* 0x21F54 / 0x223A8: int8 0xEED84[160n + t] */
{
    int32_t i, j;

    for (i = m - 2; i >= 0; i--)
        for (j = 0; j <= i; j++)
            if (sb(SCORE(n, r[j + 1].t)) > sb(SCORE(n, r[j].t)))                       /* 0x21FEE / 0x22442 cmp al, bl */
                mrec_swap(r, j);
}

void f21E8C(int n)
{
    char buf[152];          /* [ebp-0xD8]..[ebp-0x41] */
    MRec rec[160];          /* [ebp-0x858]..[ebp-0xD8] */
    uint32_t nat = NAT(n);  /* [ebp-0x1C] */
    int32_t m;              /* [ebp-0x20] */
    int32_t i;              /* [ebp-0x28] */
    int32_t j;              /* [ebp-0x24] */
    int32_t t;              /* [ebp-0x34] / [ebp-0x30] */
    int32_t x;              /* [ebp-0x2C] */
    int32_t tok;
    const char *fmt;

    memset(rec, 0, sizeof rec);   /* port: the stand-in of bug 2 (file comment) and the city records' unused fields */
    if (S8(nat + 0x51u) < 9                                                            /* 0x21EC1 jge 0x2225A */
        && S8(nat + 0x98u) / 2 >= S8(nat + 0x51u) + 1) {                               /* 0x21ED3..0x21EE2 jl */
        m = 0;                                                                         /* 0x21EEA */
        for (i = 0; i < S32(CHOICE_N); i++)                                            /* 0x21EED, 0x21EFB */
            if ((ub(TS(ch(i)) + 2u) & 1u) != 0 && m < 160)                             /* 0x21F1C (port guard) */
                rec[m++].t = ch(i);                                                    /* 0x21F41 */
        if (m > 0) {                                                                   /* 0x21F4A jle 0x2225A */
            mrec_sort_score(rec, m, n);                                                /* 0x21F54..0x22052 */
            for (i = m - 2; i >= 0; i--)                                               /* 0x22057..0x22188 */
                for (j = 0; j <= i; j++)
                    if (S32(ATT(n, ub(TS(rec[j + 1].t)))) > S32(ATT(n, ub(TS(rec[j].t)))))   /* 0x22116 jle */
                        mrec_swap(rec, j);
            t = rec[0].t;                                                              /* 0x22193 */
            fmt = mono_converts_fmt(0xB1EEAu);   /* Montheism fixed unless g_original (Erik, 19 Nov) */
            snprintf(buf, sizeof buf, fmt, NAME(n), TERR_NAME(t));                     /* 0x221B8 */
            f79270(-550, n, t, buf, 0);                                                /* 0x221D3 */
            f78A20(300);                                                               /* 0x221DD */
            wb(AREA(t + 1) + 0x3Bu + ub(TS(t)), 1);                                    /* 0x22209 */
            f78F04(t, (int)ub(TS(t)));                                                 /* 0x22228 */
            f78E74(t, n);                                                              /* 0x22233 */
            f79270(-500, n, t, buf, 0);                                                /* 0x2224B */
            return;
        }
    }
    m = 0;                                                                             /* 0x2225C */
    for (i = 0; i < S32(CHOICE_N); i++) {                                              /* 0x2225F, 0x2231D jge 0x223A8 */
        t = ch(i);                                                                     /* 0x22333 [ebp-0x30] */
        if ((ub(TS(ch(i)) + 2u) & 1u) != 0)                                            /* 0x2234F jne 0x22314 */
            continue;
        if (m >= 160)   /* port guard (G: at most 157 candidates) */
            break;
        rec[m].t = ch(i);                                                              /* 0x22371 */
        rec[m].nat = 0;                                                                /* 0x22386 */
        rec[m].cnt = 0;                                                                /* 0x22399 */
        for (j = 0; j < 9; j++) {                                                      /* 0x223A0, 0x2226D jge 0x2230E */
            if (ub(NAT(i + 1) + 0x9Au) == 0)                                           /* 0x22287: [ebp-0x28] = i, not j */
                continue;
            tok = sb(AREA(t + 1) + 0x0Fu + (uint32_t)j);                               /* 0x222A4 [+ j + 0xE31C4] sar 24 */
            if (tok <= rec[m].cnt)                                                     /* 0x222B3 jle */
                continue;
            rec[m].cnt = tok;                                                          /* 0x222E4 */
            rec[m].nat = j + 1;                                                        /* 0x22302 */
        }
        m++;                                                                           /* 0x22311 */
    }
    if (m == 0 && !g_original)   /* BUG(1995): record 0 is uninitialised stack (file comment, bug 2) */
        return;
    mrec_sort_score(rec, m, n);                                                        /* 0x223A8..0x224A6 */
    for (i = m - 2; i >= 0; i--)                                                       /* 0x224AB..0x225AA */
        for (j = 0; j <= i; j++)
            if (S32(ATT(n, rec[j + 1].nat)) > S32(ATT(n, rec[j].nat)))                 /* 0x22546 jle */
                mrec_swap(rec, j);
    for (i = m - 2; i >= 0; i--)                                                       /* 0x225AF..0x2267C */
        for (j = 0; j <= i; j++)
            if (rec[j + 1].cnt > rec[j].cnt)                                           /* 0x22618 jle */
                mrec_swap(rec, j);
    t = rec[0].t;                                                                      /* 0x22687 */
    fmt = mono_converts_fmt(0xB1F07u);   /* Montheism fixed unless g_original (Erik, 19 Nov) */
    snprintf(buf, sizeof buf, fmt, NAME(n), TERR_NAME(t));                             /* 0x226AC */
    f79270(-550, n, t, buf, 0);                                                        /* 0x226C7 */
    f78A20(300);                                                                       /* 0x226D3 */
    for (i = 0; i < 9; i++) {                                                          /* 0x226D8, 0x226E3 jge 0x22238 */
        if (ub(AREA(t + 1) + 0x0Fu + (uint32_t)i) == 0)                                /* 0x22706 test bh, bh */
            continue;
        wb(AREA(t + 1) + 0x3Cu + (uint32_t)i, 1);                                      /* 0x2271D */
        x = sb(AREA(t + 1) + 0x0Fu + (uint32_t)i);                                     /* 0x22740 dword [+ 0xE31C4] sar 24 */
        f57CE8(t, i + 1, 0, x);                                                        /* 0x2274F */
        setD8(NAT(i + 1) + 0x98u, (uint8_t)(D8(NAT(i + 1) + 0x98u) - (uint8_t)x));     /* 0x22761 sub byte [+0xF1564] */
        setD8(NAT(i + 1) + 0x99u, (uint8_t)(D8(NAT(i + 1) + 0x99u) + (uint8_t)x));     /* 0x22774 add byte [+0xF1565] */
        f57E28(t, n, x);                                                               /* 0x22783 */
        setD8(nat + 0x99u, (uint8_t)(D8(nat + 0x99u) - (uint8_t)x));                   /* 0x22792 */
        setD8(nat + 0x98u, (uint8_t)(D8(nat + 0x98u) + (uint8_t)x));                   /* 0x227A2 */
    }
    f79270(-500, n, t, buf, 0);                                                        /* 0x2224B (L_22238) */
}
