/* Module CAL3 of docs/M6_PLAN.md, the human side: Volcano / Earthquake. Literal translations; the address of each
   instruction a line comes from is in its comment. D = read in the code, G = inference. Prototypes and globals: cal3.h.

     0x1E7A0  f1E7A0  Volcano / Earthquake (calamity 0x13, handler of CAL1's f1E0A0)          844 bytes
     0x1EAEC  f1EAEC  the human's site chooser (volcano 0x13, Earthquake city 0x12, Flood 0x19) 1,084
     0x1F25C  f1F25C  the eruption                                                              712
     0x1F524  f1F524  the Earthquake: candidate pairs, scores, the choice                     2,232
     0x1FDDC  f1FDDC  the human's Earthquake pair chooser                                       652
     0x203B4  f203B4  the Earthquake's result and message                                       432

   Not ported here (each call site stops the port with not_ported(address); the static stubs below carry the names,
   so the call sites read as in the original and a later translation that exports one of these names collides with
   its stub at compile time instead of being shadowed by it). Built with -DCAL3_TEST (port/test/cal3_test.c) they go
   to the test's recorders instead, which return.
     M7 (AI):  0x1EF28  int f1EF28(int n, int32_t *scores): a computer's volcano among the tied ones (eax n, edx the
                        caller's int32 scores[3] on the stack); returns the volcano index: 0x1EAAB
               0x20068  void f20068(int n): orders the tied Earthquake pairs 0xF7300 (eax unused; the caller then takes
                        pair 0): 0x1FD0E
               0x201B8  int f201B8(int n): a computer's own city hit when no pair exists; returns the territory: 0x1FDB3
     ANIM:     0x7A4C4  void f7A4C4(int anim): the calamity animation f7A0A0([0xF783C]) when bit 0x04 of [0xF097C] is
                        set (eax -600 here): 0x1EE6F (every redraw of 0x1EAEC), 0x1FFB5 (every redraw of 0x1FDDC)
               (0x7A0A0 is also reached through kit.c's f79270 (anim 1 / 2) and conflict.c's f5CDE8 (anim 2000), which
               stop at it themselves when animations are on)
   CAL1's f1E69C (the card out of the hand) is cal1.h's. So today the no-city message and the eruption run up to the
   losses box (f5CDE8 stops at 0x7A4C4 for its nonzero anim argument), the Earthquake of a human victim runs up to its
   chooser's first redraw (0x7A4C4) when it has a choice, and to f79270's message when it has none.

   game_rand (f78AE4): none in these six functions (D: no call of 0x78AE4). The animation (0x7A0A0's kaboom3 roll,
   M6_PLAN egg 1) and the computers' choices draw on their own (ANIM, M7).

   Play by e-mail (dropped by Erik's ruling): no PBEM routine is called here. The bit-0x80 tests of byte [0xE1624]
   (0x1EA3D, 0x1FCDF, 0x1FD87) are kept literally: they only send a PBEM human down the computer's path.

   Watcom runtime -> standard C:
     0x7DB3D sprintf -> snprintf bounded to the stack buffer (100 bytes: 0x1E7A0 [ebp-0xC4], 0x1EAEC [ebp-0x8C],
             0x1F25C / 0x1FDDC [ebp-0x88]; 0x203B4 200 bytes at [ebp-0x144] and 100 at [ebp-0x7C]); the inline strcat
             (repne scasb + the byte-pair loop, 0x20494) -> cat(), bounded likewise; the formats are written out with
             their addresses in the comments
     0x7D5E8 strupr (D: a..z -> A..Z only, in place) -> upper() below
     0x7D5D0 memset -> memset on the data object
   __CHK dropped. Watcom's register convention keeps edx / ebx / esi across calls: so 0x1EAEC's relabel (0x1EC17) gets
   edx = 0x2A loaded before the sprintf, 0x1F524 stores esi = 0 as the pair index after 0x20068 (0x1FD13), and 0x1F25C
   passes edx = its buffer as f5CDE8's title (0x1F4FF; loaded for the strupr at 0x1F4EF).

   Data (D, read in the code at the addresses given):
     nation record n: 0xF14CC + 199n (rules.h): +0x2A Engineering, +0x51 int8 cities (read as dword [rec + 0x4E] sar
       24), +0x52 + i city i's territory + 1, +0x98 int8 tokens on the board, +0x99 int8 stock, +0x9A 0 out / 2 computer
       / 3 human (dword [rec + 0x97] sar 24)
     territory state t: 0xE2C18 + 3t (+0 owner, +2 bit 0x01 city); 0x1E7A0 tests the volcanoes' territories through
       hard-coded addresses: 0xE2CB4 / 0xE2CB6 (52), 0xE2D95 / 0xE2D97 (127), 0xE2CFC / 0xE2CFE (76), 0xE2D02 /
       0xE2D04 (78), 0xE2D11 / 0xE2D13 (83), the table's territories (D: checked against 0xBDE18)
     area record r = t + 1: 0xE31B8 + 69r, +0x0E + k int8 tokens of nation k
     the land / coast lists [0xF8E7C + 4t] / [0xF90FC + 4t]: port handles into the heap blocks of landadj.dat /
       coastadj.dat (game.h): int16 count, int16 territories
     the river table 0xBEE08 + 22s (cal1.h): int16 +2 the count, char * +4 the name, int16 +8 + 2j the territories
     names 0xC768C[n], adjectives 0xC76C4[n], territory names 0xC77D8[t]; volcano table and names: cal3.h
     sprites 0xF51AC / 0xF51B4 (the two arrows); [0xF52C8] back buffer, [0xF52CC] background buffer, int8 [0xEE168]
       the background number; int16 [0xBB400] the message area's height (0x14 during the eruption, 0x23 after)

   The Earthquake's fixed tables (0x1F524): ten entries per city, nine cities (0xF6F68..0xF72EB), then the counts. D
   (landadj.dat / coastadj.dat of the run copy): the most entries any territory that can hold a city gets is 10
   (SYRIA, land list only); only the seas have more (W. MEDITERRANEAN 22). So the blocks never overflow.

   1995 bugs, fixed unless g_original (each marked BUG(1995) in the code; grades code / effect):
     1. 0x1EAEC types 0x13 / 0x19 (D / G): f78BA4's map branch compares the clicked TERRITORY with the list, which here
        holds volcano indices 0..2 (or river indices 0..4): a click on the volcano's own territories selects nothing,
        and a click on territory 0..2 (SUDETEN, SCYTHIA, KUBAN; 0..4 for rivers, with MIKOP, NALCHIK) selects the
        volcano (river) of that number. Fixed: map clicks are ignored for these two types (the arrows choose), as a
        click on no candidate. Type 0x12 (the list holds territories) keeps its map clicks.
     2. 0x1FDDC (D / G): the same map branch searches 0xF7CA0, but 0x1F524's pair branch fills only 0xF7300, so a click
        is matched against a stale list (whatever the last chooser left: D, 0x1E7A0 writes 0xF7CA0 only on its volcano
        path, which excludes the Earthquake) and selects an unrelated pair, or none. Fixed: map clicks are ignored.
   Kept as the original (D code, G effect unless marked):
     - 0x1FDDC removes dialog 0xBE21C (the Volcano's) instead of its own 0xBE608 (M6_PLAN egg 27): f78664 matches the
       first record's id and removes int16 [dlg] records; both dialogs hold 3 records starting with id 0x39 (D: the
       bytes), so the result is the same. Kept literally.
     - 0x203B4 passes -t to f79270 to keep t2's highlight; for t == 0 (SUDETEN) -0 is 0, which clears every highlight
       but t's (G: cosmetic; f79270's interface cannot say "centre on 0 and keep the others").
     - 0x203B4 reduces the neighbouring city without testing its owner's Engineering: the scores (0x1F524) already give
       a neighbour with Engineering 0, so a chosen neighbour never has it (D).
     - 0x1EAEC with a type other than 0x12 / 0x13 / 0x19 shows an uninitialised stack text (port: an empty text) and
       draws no name; no caller passes one (D). With an empty list it shows entry 0 (stale); no caller passes one
       (D: a volcano or river tie has two entries, the Earthquake's own-city list one per city and 0x1F524 is reached
       only with cities).
     - 0x1E7A0's index of the maximum ([ebp-0x1C]) is uninitialised when every score is negative (G: unreachable,
       scores are 6s and token counts); port: 0.
     - 0x1F524's unique-maximum branch falls through to f203B4 with uninitialised territories if no entry has the
       maximum (unreachable: the maximum is an entry's score, D). Port guard: no call.
   Text (Erik's ruling of 19 Nov: only obvious errors): none changed. "volcanos" (0xB1A05) is a dictionary variant of
   "volcanoes", not a misspelling (TEXT_SWEEP.md proposed it; for Erik). The two spaces after "Earthquake strikes %s!"
   (0xB1BB4) are typewriter style.
   Easter eggs / oddities: the volcano table (M6_PLAN egg 19: Mt. Vesuvius over CAMPANIA and NEAPOLIS, Mt. Etna over
   MILAZZO and SYRACUSE, Mt. Thera alone on THERA), shouted in capitals by strupr ("MT. VESUVIUS ERUPTION CASUALTIES");
   the copy-paste dialog removal above (egg 27); the Earthquake chooser's text says "territories" where the volcano
   chooser's twin (0xB1A99, used for the own-city choice) says "territory". */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "anim.h"
#include "cal1.h"
#include "cal2.h"
#include "cal3.h"
#include "conflict.h"
#include "fg.h"
#include "game.h"
#include "kit.h"
#include "map.h"
#include "mapdata.h"
#include "move.h"
#include "panel.h"
#include "rules.h"
#include "state.h"
#include "ui.h"

#define NATION(n)    (0xF14CCu + 199u * (uint32_t)(n))
#define NAME(n)      DSTR(D32(0xC768Cu + 4u * (uint32_t)(n)))
#define ADJ(n)       DSTR(D32(0xC76C4u + 4u * (uint32_t)(n)))
#define TERR_NAME(t) DSTR(D32(0xC77D8u + 4u * (uint32_t)(t)))
#define TSTATE(t)    (0xE2C18u + 3u * (uint32_t)(t))
#define AREA(r)      (0xE31B8u + 69u * (uint32_t)(r))
#define CAL_VICTIM(k) (0xF77E6u + 3u * (uint32_t)(k))  /* int8 (dword [0xF77E3 + 3k] sar 24) */
#define VOLC(v, j)   (0xBDE18u + 4u * (uint32_t)(v) + 2u * (uint32_t)(j))   /* int16 territory */
#define VOLC_NAME(v) DSTR(D32(0xBDE24u + 4u * (uint32_t)(v)))
#define RIVER(s)     (0xBEE08u + 22u * (uint32_t)(s))
#define EQ(i, j)     (0xF6F68u + 100u * (uint32_t)(i) + 10u * (uint32_t)(j))  /* entry j of city i (10 bytes) */
#define EQ_N(i)      (0xF72ECu + 2u * (uint32_t)(i))                          /* int16 */
#define PAIR(k)      (0xF7300u + 8u * (uint32_t)(k))                          /* 8 bytes */
#define PAIR_SEL     0xF77E0u   /* int16 */
#define LANDADJ      0xF8E7Cu   /* uint32[160]: handles */
#define COASTADJ     0xF90FCu   /* uint32[160]: handles */
#define ATT(n, o)    (0xF52E0u + 40u * (uint32_t)(n) + 4u * (uint32_t)(o))    /* int32 [10][10], read as a word */
#define PRIO(n, t)   (0xEED84u + 160u * (uint32_t)(n) + (uint32_t)(t))        /* int8 [10][160] */
#define CHOICE       0xF7CA0u   /* int32[] */
#define CHOICE_N     0xF7C9Cu   /* int32 */
#define CALAMITY     0xF783Cu   /* int32 */
#define LOSS_TOK     0xF547Cu   /* uint8[14] */
#define LOSS_CITY    0xF548Cu   /* uint8[14] */
#define FLAGS        0xE1624u
#define PBEM_BIT     0x80u
#define PANEL_DLG    0xF596Cu
#define SITE_DLG     0xBE21Cu
#define PAIR_DLG     0xBE608u
#define FINISH_DLG   0xC5214u
#define BACKBUF      0xF52C8u
#define BACKGROUND   0xF52CCu
#define BGNUM        0xEE168u
#define AREA_H       0xBB400u   /* int16 */

/* ---- calls into code that is not ported yet (see the file comment) ----------------------------------------------- */

#ifdef CAL3_TEST
int  cal3_test_f1EF28(int n, int32_t *scores);
void cal3_test_f20068(int n);
int  cal3_test_f201B8(int n);
void cal3_test_f7A4C4(int anim);
#define f1EF28(n, scores) cal3_test_f1EF28(n, scores)
#define f20068(n)         cal3_test_f20068(n)
#define f201B8(n)         cal3_test_f201B8(n)
#define f7A4C4(anim)      cal3_test_f7A4C4(anim)
#else
#include "aical1.h"                             /* f1EF28, f20068, f201B8: the computers' Volcano / Earthquake choices (M7) */

#endif

/* ---- small shared pieces ------------------------------------------------------------------------------------------ */

/* the inline strcat copy (repne scasb, then the byte-pair loop, 0x20494..0x204BC), bounded to the caller's buffer */
static void cat(char *buf, size_t size, const char *s)
{
    size_t n = strlen(buf);
    if (n + 1 < size)
        snprintf(buf + n, size - n, "%s", s);
}

/* 0x7D5E8 strupr: each byte a..z (sub dl, 0x61; cmp dl, 0x19; ja) becomes A..Z, in place */
static void upper(char *s)
{
    for (; *s != 0; s++)
        if ((uint8_t)(*s - 'a') <= 0x19u)
            *s = (char)(*s - 'a' + 'A');
}

/* the background 3 into the back buffer: f3CD6C(3), byte [0xEE168] = 3 (stored after the call), fg_vbcopy of the
   whole screen from [0xF52CC] to [0xF52C8] (0x1EB12..0x1EB44, 0x1F410..0x1F445, 0x1FE00..0x1FE32) */
static void background3(void)
{
    f3CD6C(3);
    setD8(BGNUM, 3);
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BACKGROUND), S32(BACKBUF));
}

/* the word at h + off in a land / coast list (a port handle; the original reads [p] with movsx / mov ax). Port guard:
   an unresolved handle reads 0, so its list is empty */
static uint16_t lword(uint32_t h, uint32_t off)
{
    const uint8_t *p = load_heap_ptr(h + off, 2);
    return p != NULL ? (uint16_t)(p[0] | p[1] << 8) : 0;
}

/* ==== 0x1E7A0 Volcano / Earthquake =================================================================================== */

void f1E7A0(int k)
{
    char buf[100];       /* [ebp-0xC4] */
    int32_t has[3];      /* [ebp-0x54] */
    int32_t score[3];    /* [ebp-0x60] */
    int32_t n;           /* [ebp-0x44] */
    uint32_t rec;        /* [ebp-0x18] */
    int32_t cnt;         /* [ebp-0x40] */
    int32_t i;           /* [ebp-0x3C], [ebp-0x38] */
    int32_t j;           /* [ebp-0x34] */
    int32_t t;           /* [ebp-0x28] */
    int32_t m;           /* [ebp-0x30] */
    int32_t v = 0;       /* [ebp-0x2C] (set on every path that reads it) */
    int32_t best;        /* [ebp-0x24] */
    int32_t ties;        /* [ebp-0x20] */
    int32_t idx = 0;     /* [ebp-0x1C]: uninitialised in the original when every score is negative (file comment) */
    int32_t c;

    n = S8(CAL_VICTIM(k));                                          /* 0x1E7C7 dword [0xF77E3 + 3k] sar 24 */
    rec = NATION(n);                                                /* 0x1E7E1 */
    cnt = 0;                                                        /* 0x1E7E6 */
    for (i = 0; i < 3; i++) {                                       /* 0x1E7E9, 0x1E7F4 cmp 3; jge */
        has[i] = 0;                                                 /* 0x1E7FF */
        score[i] = 0;                                               /* 0x1E806 */
    }
    if (D8(rec + 0x51u) == 0) {                                     /* 0x1E80F cmp byte [rec + 0x51], 0; jne */
        snprintf(buf, sizeof buf, "%s has no cities to be affected by volcanos or earthquakes.", NAME(n));
                                                                    /* 0x1E82C (0xB1A05) */
        f79270(-500, n, -200, buf, 1);                              /* 0x1E849 */
        f1E69C(k);                                                  /* 0x1EADB */
        return;
    }
    /* the volcanoes whose territories hold a city of n (hard-coded addresses, file comment) */
    if (((D8(TSTATE(52) + 2u) & 1) != 0 && (int32_t)D8(TSTATE(52)) == n) ||        /* 0x1E853 / 0x1E85E */
        ((D8(TSTATE(127) + 2u) & 1) != 0 && (int32_t)D8(TSTATE(127)) == n)) {      /* 0x1E868 / 0x1E873 */
        cnt++;                                                      /* 0x1E880 */
        has[0] = 1;                                                 /* 0x1E883 */
        v = 0;                                                      /* 0x1E88C */
    }
    if (((D8(TSTATE(76) + 2u) & 1) != 0 && (int32_t)D8(TSTATE(76)) == n) ||        /* 0x1E88F / 0x1E89A */
        ((D8(TSTATE(78) + 2u) & 1) != 0 && (int32_t)D8(TSTATE(78)) == n)) {        /* 0x1E8A4 / 0x1E8AF */
        cnt++;                                                      /* 0x1E8BC */
        has[1] = 1;                                                 /* 0x1E8C4 */
        v = 1;                                                      /* 0x1E8C7 */
    }
    if ((D8(TSTATE(83) + 2u) & 1) != 0 && (int32_t)D8(TSTATE(83)) == n) {         /* 0x1E8CA / 0x1E8D5 */
        cnt++;                                                      /* 0x1E8E2 */
        has[2] = 1;                                                 /* 0x1E8E5 */
        v = 2;                                                      /* 0x1E8EC */
    }
    if (cnt == 0) {                                                 /* 0x1E8F3 je 0x1EAD0 */
        f1F524(n);                                                  /* 0x1EAD3 the Earthquake */
        f1E69C(k);                                                  /* 0x1EADB */
        return;
    }
    if (cnt > 1) {                                                  /* 0x1E900 cmp edx, 1; jle 0x1EAB3 */
        for (i = 0; i < 3; i++) {                                   /* 0x1E909, 0x1E916 cmp 3; jge */
            if (has[i] == 0)                                        /* 0x1E923 */
                continue;
            score[i] = 0;                                           /* 0x1E930 */
            for (j = 0; j < 2; j++) {                               /* 0x1E934, 0x1E93F cmp 2; jge */
                t = S16(VOLC(i, j));                                /* 0x1E94D dword [2j + 4i + 0xBDE16] sar 16 */
                if (t == -1)                                        /* 0x1E95D */
                    continue;
                if ((D8(TSTATE(t) + 2u) & 1) != 0 &&                /* 0x1E96E test dl, 1 */
                    D8(TSTATE(t)) != 0xA) {                         /* 0x1E985 and eax, 0xFF; cmp 0xA; je */
                    score[i] += 6;                                  /* 0x1E998 */
                    continue;
                }
                for (m = 0; m < 9; m++)                             /* 0x1E99F, 0x1E9AC cmp 9; jge */
                    score[i] += S8(AREA(t + 1) + 0x0Fu + (uint32_t)m);   /* 0x1E9C8 dword [69(t+1) + m + 0xE31C4]
                                                                       sar 24: the tokens of nation m + 1 */
            }
        }
        best = 0;                                                   /* 0x1E9D9 */
        ties = 0;                                                   /* 0x1E9DC */
        for (i = 0; i < 3; i++) {                                   /* 0x1E9DF, 0x1E9FF cmp 3; jge */
            if (score[i] < best)                                    /* 0x1EA0F jl */
                continue;
            if (score[i] == best)                                   /* 0x1EA1D jne 0x1E9E4 */
                ties++;                                             /* 0x1EA24 */
            else
                ties = 0;                                           /* 0x1E9E6 */
            best = score[i];                                        /* 0x1E9F0 */
            idx = i;                                                /* 0x1E9F6 */
        }
        if (ties == 0) {                                            /* 0x1EA29 */
            v = idx;                                                /* 0x1EA2F */
        } else if ((D8(FLAGS) & PBEM_BIT) == 0 &&                   /* 0x1EA3D test dh, 0x80 */
                   S8(rec + 0x9Au) == 3) {                          /* 0x1EA4B dword [rec + 0x97] sar 24; cmp 3 */
            setD32(CHOICE_N, 0);                                    /* 0x1EA55 */
            for (i = 0; i < 3; i++) {                               /* 0x1EA5A, 0x1EA65 cmp 3; jge */
                if (score[i] != best)                               /* 0x1EA77 */
                    continue;
                c = S32(CHOICE_N);                                  /* 0x1EA79 */
                setD32(CHOICE_N, (uint32_t)(c + 1));                /* 0x1EA86 */
                setD32(CHOICE + 4u * (uint32_t)c, (uint32_t)i);     /* 0x1EA8E */
            }
            v = f1EAEC(n, 0x13);                                    /* 0x1EA9E */
        } else {
            v = f1EF28(n, score);                                   /* 0x1EAAB [M7] */
        }
    }
    f1F25C(v, n);                                                   /* 0x1EAB9 */
    f1E69C(k);                                                      /* 0x1EAC1 */
}

/* ==== 0x1EAEC: the human's site chooser ================================================================================ */

int f1EAEC(int n, int type)
{
    char buf[100];       /* [ebp-0x8C] */
    int32_t idx;         /* [ebp-0x1C] */
    int32_t redraw;      /* [ebp-0x18] */
    int32_t r;           /* [ebp-0x20] */
    int32_t j;           /* [ebp-0x24] */
    int32_t c;

    idx = 0;                                                        /* 0x1EB08 */
    redraw = 1;                                                     /* 0x1EB0B */
    buf[0] = 0;   /* port: the original leaves it uninitialised for a type other than the three (file comment) */
    background3();                                                  /* 0x1EB17..0x1EB44 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x1EB53 */
    f3AF8C(0xF51ACu, 0xFB, 0x195);                                  /* 0x1EB6A left arrow */
    f3AF8C(0xF51B4u, 0x178, 0x195);                                 /* 0x1EB7E right arrow */
    fg_vbclose();                                                   /* 0x1EB83 */
    f459E4(n);                                                      /* 0x1EB8B */
    if (type == 0x13) {                                             /* 0x1EB96 */
        snprintf(buf, sizeof buf, "%s is about to undergo a Volcanic Eruption. Choose which volcano will erupt.",
                 NAME(n));                                          /* 0x1EBB4 (0xB1A41) */
        f3C284(SITE_DLG, 0x2A, 0xB1A8Eu);                           /* 0x1EC17 "ERUPT HERE" (edx = 0x2A from 0x1EBAF) */
    } else if (type == 0x12) {                                      /* 0x1EBC6 */
        snprintf(buf, sizeof buf, "%s is about to be hit by an Earthquake. Choose which territory will be hit.",
                 NAME(n));                                          /* 0x1EC05 (0xB1A99) */
        f3C284(SITE_DLG, 0x2A, 0xB1AE5u);                           /* 0x1EC17 "STRIKE HERE" */
    } else if (type == 0x19) {                                      /* 0x1EBE8 */
        snprintf(buf, sizeof buf, "%s is about to be Flooded. Choose which river will flood.", NAME(n));
                                                                    /* 0x1EC05 (0xB1AF1) */
        f3C284(SITE_DLG, 0x2A, 0xB1B2Bu);                           /* 0x1EC17 "FLOOD HERE" */
    }
    f3A9F4(buf);                                                    /* 0x1EC22 */
    fg_setcolor(5);                                                 /* 0x1EC29 */
    f7853C(SITE_DLG);                                               /* 0x1EC36 */
    f3B05C(PANEL_DLG);                                              /* 0x1EC40 */
    for (;;) {                                                      /* 0x1EC45 */
        fg_vbopen(S32(BACKBUF));                                    /* 0x1EC4C */
        f3C9D0(0x109, 0x195, 0x6E, 0x13, 5);                        /* 0x1EC6A the sunken name box */
        fg_setcolor(5);                                             /* 0x1EC71 */
        c = S32(CHOICE + 4u * (uint32_t)idx);                       /* 0x1EC8C / 0x1ECB2 / 0x1ECD3 */
        if (type == 0x13)                                           /* 0x1EC82 */
            f3A0B0(0x109, 0x199, 0x6E, VOLC_NAME(c));               /* 0x1EC9C [4c + 0xBDE24], 0x1ECFD */
        else if (type == 0x12)                                      /* 0x1ECA8 */
            f3A0B0(0x109, 0x199, 0x6E, TERR_NAME(c));               /* 0x1ECC2 [4c + 0xC77D8], 0x1ECFD */
        else if (type == 0x19)                                      /* 0x1ECCE */
            f3A0B0(0x109, 0x199, 0x6E, cal2_river_text(DSTR(D32(RIVER(c) + 4u))));   /* 0x1ECF1 [22c + 0xBEE0C], 0x1ECFD (text fix: cal2.h) */
        fg_vbclose();                                               /* 0x1ED02 */
        f3F670();                                                   /* 0x1ED07 */
        if (type == 0x13) {                                         /* 0x1ED12 */
            f3F624(S16(VOLC(c, 0)), 0xF);                           /* 0x1ED2D dword [4c + 0xBDE16] sar 16 */
            if (S16(VOLC(c, 1)) != -1)                              /* 0x1ED46 dword [4c + 0xBDE18] sar 16 */
                f3F624(S16(VOLC(c, 1)), 0xF);                       /* 0x1ED68 */
        } else if (type == 0x12) {                                  /* 0x1ED75 */
            f3F624(c, 0xF);                                         /* 0x1ED85 */
        } else if (type == 0x19) {                                  /* 0x1ED92 */
            for (j = 0; S16(RIVER(c) + 2u) > j; j++)                /* 0x1ED99, 0x1EDC6 dword [22c + 0xBEE08] sar 16 */
                f3F624(S16(RIVER(c) + 8u + 2u * (uint32_t)j), 0xF); /* 0x1EDF8 dword [22c + 2j + 0xBEE0E] sar 16 */
        }
        if (redraw != 0) {                                          /* 0x1EDFF */
            if (type == 0x13)                                       /* 0x1EE0E */
                f3DA60(S16(VOLC(c, 0)));                            /* 0x1EE58 */
            else if (type == 0x12)                                  /* 0x1EE28 */
                f3DA60(c);                                          /* 0x1EE58 */
            else if (type == 0x19)                                  /* 0x1EE38 */
                f3DA60(S16(RIVER(c) + 8u));                         /* 0x1EE58 dword [22c + 0xBEE0E] sar 16 */
        }
        f3CBBC();                                                   /* 0x1EE5D */
        if (redraw != 0) {                                          /* 0x1EE62 */
            f7A4C4(-600);                                           /* 0x1EE6F [ANIM] eax 0xFFFFFDA8 */
            redraw = 0;                                             /* 0x1EE74 (edx = 0, kept across the call) */
        }
        r = f78BA4(PANEL_DLG, 1);                                   /* 0x1EE81 */
        if (r == -100)                                              /* 0x1EE89 */
            continue;
        if (r >= 0) {                                               /* 0x1EE96 test ecx, ecx; jl */
            /* BUG(1995) 1: for a volcano or a river the list holds indices, not territories, so the map click selects
               the wrong candidate (file comment, 1); ignored as a click on nothing */
            if (!g_original && type != 0x12)
                continue;
            idx = r;                                                /* 0x1EE9D */
            redraw = 1;                                             /* 0x1EEA0 */
            continue;
        }
        if (r == -1)                                                /* 0x1EEAF */
            break;
        if ((uint32_t)r < (uint32_t)-10)                            /* 0x1EEB7 cmp eax, -0xA; jb (unsigned) */
            continue;
        if (r == -10) {                                             /* 0x1EEC0 jbe 0x1EEE7 */
            if (S32(CHOICE_N) - 1 > idx) {                          /* 0x1EEED cmp eax, [ebp-0x1C]; jle */
                idx++;                                              /* 0x1EEF9 */
                redraw = 1;                                         /* 0x1EEFC */
            }
            continue;
        }
        if (r == -9 && idx > 0) {                                   /* 0x1EEC2 cmp -9; jne; 0x1EECB cmp 0; jle */
            idx--;                                                  /* 0x1EED8 */
            redraw = 1;                                             /* 0x1EEDB */
        }
    }
    f78664(SITE_DLG);                                               /* 0x1EF0D */
    return S32(CHOICE + 4u * (uint32_t)idx);                        /* 0x1EF15 */
}

/* ==== 0x1F25C the eruption ============================================================================================ */

void f1F25C(int v, int n)
{
    char buf[100];       /* [ebp-0x88] */
    int32_t j;           /* [ebp-0x14] */
    int32_t t;           /* [ebp-0x1C] */
    int32_t k;           /* [ebp-0x18] */
    uint32_t o;
    uint8_t tok;

    setD16(AREA_H, 0x14);                                           /* 0x1F285 */
    memset(DP(LOSS_TOK), 0, 0xE);                                   /* 0x1F28E memset(0xF547C, 0, 0xE) */
    memset(DP(LOSS_CITY), 0, 0xE);                                  /* 0x1F29F memset(0xF548C, 0, 0xE) */
    for (j = 0; j < 2; j++) {                                       /* 0x1F2A6, 0x1F2B1 cmp 2; jge */
        t = S16(VOLC(v, j));                                        /* 0x1F2C3 dword [2j + 4v + 0xBDE16] sar 16 */
        if (t == -1)                                                /* 0x1F2D3 */
            continue;
        if ((D8(TSTATE(t) + 2u) & 1) != 0) {                        /* 0x1F2F0 test dl, 1; je */
            o = D8(TSTATE(t));                                      /* 0x1F2FB */
            setD8(LOSS_CITY + o, (uint8_t)(D8(LOSS_CITY + o) + 1u));   /* 0x1F306 inc byte [owner + 0xF548C] */
            f78F04(t, D8(TSTATE(t)));                               /* 0x1F323 the city destroyed */
            continue;
        }
        if ((D8(TSTATE(t) + 2u) & 1) != 0)                          /* 0x1F32A the same byte tested again; jne */
            continue;
        for (k = 1; k <= 9; k++) {                                  /* 0x1F339, 0x1F348 cmp 9; jg */
            tok = D8(AREA(t + 1) + 0x0Eu + (uint32_t)k);            /* 0x1F365 byte [69(t+1) + k + 0xE31C6] */
            if (tok == 0)                                           /* 0x1F36B test bh, bh */
                continue;
            setD8(LOSS_TOK + (uint32_t)k, (uint8_t)(D8(LOSS_TOK + (uint32_t)k) + tok));   /* 0x1F38B */
            setD8(NATION(k) + 0x99u, (uint8_t)(D8(NATION(k) + 0x99u) + tok));   /* 0x1F3B9 stock (0xF1565 + 199k) */
            setD8(NATION(k) + 0x98u, (uint8_t)(D8(NATION(k) + 0x98u) - tok));   /* 0x1F3DF board (0xF1564 + 199k) */
            setD8(AREA(t + 1) + 0x0Eu + (uint32_t)k, 0);            /* 0x1F3FA */
            f581C4(t, k);                                           /* 0x1F406 */
        }
    }
    background3();                                                  /* 0x1F415..0x1F445 */
    f7876C();                                                       /* 0x1F44D */
    f7853C(FINISH_DLG);                                             /* 0x1F457 */
    f3B05C(PANEL_DLG);                                              /* 0x1F461 */
    f3F670();                                                       /* 0x1F466 */
    f459E4(n);                                                      /* 0x1F46E */
    f3F624(S16(VOLC(v, 0)), 0xF);                                   /* 0x1F485 dword [4v + 0xBDE16] sar 16 */
    if (S16(VOLC(v, 1)) != -1)                                      /* 0x1F497 dword [4v + 0xBDE18] sar 16 */
        f3F624(S16(VOLC(v, 1)), 0xF);                               /* 0x1F4AE */
    f3DA60(S16(VOLC(v, 0)));                                        /* 0x1F4C0 */
    f3CBBC();                                                       /* 0x1F4C5 */
    snprintf(buf, sizeof buf, "%s eruption casualties", VOLC_NAME(v));   /* 0x1F4E1 (0xB1B36) */
    upper(buf);                                                     /* 0x1F4F5 strupr(buf) */
    f5CDE8(2000, buf);                                              /* 0x1F4FF eax 0x7D0, edx = buf */
    f78664(FINISH_DLG);                                             /* 0x1F50E */
    setD16(AREA_H, 0x23);                                           /* 0x1F513 */
}

/* ==== 0x1F524 the Earthquake ========================================================================================== */

void f1F524(int n)
{
    uint32_t rec;        /* [ebp-0x1C] */
    int32_t i;           /* [ebp-0x44] */
    int32_t j;           /* [ebp-0x40] */
    int32_t m;           /* [ebp-0x3C] */
    int32_t fresh;       /* [ebp-0x38] */
    int32_t t;           /* [ebp-0x2C] */
    int32_t t2;          /* [ebp-0x28] */
    uint32_t p;          /* [ebp-0x18]: a list handle */
    int32_t best;        /* [ebp-0x34] */
    int32_t ties;        /* [ebp-0x30] */
    int32_t a;           /* [ebp-0x24] */
    int32_t b;           /* [ebp-0x20] */
    int32_t c;
    int32_t e;
    uint32_t o;

    rec = NATION(n);                                                /* 0x1F54A */
    setD32(CALAMITY, 0x12);                                         /* 0x1F554 */

    /* the candidate entries: land neighbours, then coast neighbours not listed yet, each with score 6 */
    for (i = 0; S8(rec + 0x51u) > i; i++) {                         /* 0x1F55A, 0x1F56E dword [rec + 0x4E] sar 24 */
        t = (int32_t)D8(rec + 0x52u + (uint32_t)i) - 1;             /* 0x1F57D..0x1F585 */
        p = D32(LANDADJ + 4u * (uint32_t)t);                        /* 0x1F58C */
        for (j = 1; (int16_t)lword(p, 0) >= j; j++) {               /* 0x1F596, 0x1F5A8 movsx word [p]; jl */
            setD16(EQ(i, j - 1), (uint16_t)t);                      /* 0x1F5DC */
            setD16(EQ(i, j - 1) + 2u, lword(p, 2u * (uint32_t)j));  /* 0x1F615 */
            setD16(EQ(i, j - 1) + 4u, 6);                           /* 0x1F63D */
        }
        setD16(EQ_N(i), lword(p, 0));                               /* 0x1F655 */
        p = D32(COASTADJ + 4u * (uint32_t)t);                       /* 0x1F660 */
        for (j = 1; (int16_t)lword(p, 0) >= j; j++) {               /* 0x1F66A, 0x1F794 movsx word [p]; jl */
            fresh = 1;                                              /* 0x1F7A0 */
            for (m = 0; S16(EQ_N(i)) > m; m++) {                    /* 0x1F7A9, 0x1F67F dword [2i + 0xF72EA] sar 16 */
                if (D16(EQ(i, m) + 2u) == lword(p, 2u * (uint32_t)j)) {   /* 0x1F6C6 cmp ax, bx; jne */
                    fresh = 0;                                      /* 0x1F6CD */
                    break;
                }
            }
            if (fresh == 0)                                         /* 0x1F6D0 */
                continue;
            c = S16(EQ_N(i));                                       /* 0x1F6DD dword [2i + 0xF72EA] sar 16 */
            setD16(EQ(i, c), (uint16_t)t);                          /* 0x1F70B */
            setD16(EQ(i, c) + 2u, lword(p, 2u * (uint32_t)j));      /* 0x1F746 */
            setD16(EQ_N(i), (uint16_t)(D16(EQ_N(i)) + 1u));         /* 0x1F766 inc edx; mov word */
            setD16(EQ(i, c) + 4u, 6);                               /* 0x1F781 */
        }
    }

    /* the scores: 12 for a neighbouring city of another nation without Engineering, else 0 */
    best = 0;                                                       /* 0x1F7B3 */
    ties = 0;                                                       /* 0x1F7B6 */
    for (i = 0; S8(rec + 0x51u) > i; i++) {                         /* 0x1F7B9, 0x1F7CA */
        for (j = 0; S16(EQ_N(i)) > j; j++) {                        /* 0x1F7D8, 0x1F961 */
            t2 = S16(EQ(i, j) + 2u);                                /* 0x1F994 dword [100i + 10j + 0xF6F68] sar 16 */
            if ((D8(TSTATE(t2) + 2u) & 1) == 0 ||                   /* 0x1F9AC test dl, 1; je 0x1F88D */
                (int32_t)D8(TSTATE(t2)) == n) {                     /* 0x1F9CC cmp eax, n; je 0x1F88D */
                setD16(EQ(i, j) + 4u, 0);                           /* 0x1F8AF */
            } else {
                o = D8(TSTATE(t2));                                 /* 0x1F9E1 */
                if (D8(NATION(o) + 0x2Au) != 0) {                   /* 0x1FA14 byte [199o + 0xF14F6]; jne 0x1F882 */
                    setD16(EQ(i, j) + 4u, 0);                       /* 0x1F884 */
                } else {
                    setD16(EQ(i, j) + 4u, (uint16_t)(D16(EQ(i, j) + 4u) + 6u));   /* 0x1FA24 add word, 6 */
                    if (D8(TSTATE(t2)) != 0xA) {                    /* 0x1FA43 cmp 0xA; jne 0x1F7E0 */
                        /* 0x1F82F word [4 * owner + 40n + 0xF52E0] (the owner read again, 0x1F7EC) */
                        setD16(EQ(i, j) + 6u, D16(ATT(n, D8(TSTATE(t2)))));
                        /* 0x1F850 movsx cx, byte [160n + t2 + 0xEED84] */
                        setD16(EQ(i, j) + 8u, (uint16_t)(int16_t)S8(PRIO(n, t2)));
                    } else {
                        setD16(EQ(i, j) + 6u, (uint16_t)-9000);     /* 0x1FA71 edi 0xFFFFDCD8 */
                        setD16(EQ(i, j) + 8u, (uint16_t)-9000);     /* 0x1FA99 */
                    }
                }
            }
            /* 0x1F8B7 the maximum and its ties */
            if (S16(EQ(i, j) + 4u) < best)                          /* 0x1F8E1 dword [.. + 0xF6F6A] sar 16; jl */
                continue;
            if (S16(EQ(i, j) + 4u) == best)                         /* 0x1F913 jne 0x1F91F */
                ties++;                                             /* 0x1F91A */
            else
                ties = 0;                                           /* 0x1F921 */
            best = S16(EQ(i, j) + 4u);                              /* 0x1F94E */
        }
    }

    if (best == 0) {                                                /* 0x1FAA9 je 0x1FD42 */
        /* no neighbouring city to damage: one of n's own cities */
        setD32(CHOICE_N, (uint32_t)best);                           /* 0x1FD42 (edx = best = 0) */
        for (i = 0; S8(rec + 0x51u) > i; i++) {                     /* 0x1FD48, 0x1FD5C */
            c = S32(CHOICE_N);                                      /* 0x1FD6C */
            setD32(CHOICE + 4u * (uint32_t)c, (uint32_t)((int32_t)D8(rec + 0x52u + (uint32_t)i) - 1));   /* 0x1FD7A */
            setD32(CHOICE_N, (uint32_t)(c + 1));                    /* 0x1FD80 */
        }
        if ((D8(FLAGS) & PBEM_BIT) == 0 && S8(rec + 0x9Au) == 3)    /* 0x1FD87, 0x1FD9C */
            a = f1EAEC(n, 0x12);                                    /* 0x1FDA9 */
        else
            a = f201B8(n);                                          /* 0x1FDB3 [M7] */
        b = -1;                                                     /* 0x1FDBB */
    } else if (ties == 0) {                                         /* 0x1FAB6 jne 0x1FB91 */
        /* a unique maximum: that pair */
        for (i = 0; S8(rec + 0x51u) > i; i++) {                     /* 0x1FABC, 0x1FAD0 */
            for (j = 0; S16(EQ_N(i)) > j; j++) {                    /* 0x1FAD9, 0x1FAF6 */
                if (S16(EQ(i, j) + 4u) != best)                     /* 0x1FB27 */
                    continue;
                f203B4(S16(EQ(i, j)), S16(EQ(i, j) + 2u), n);       /* 0x1FB82 (eax [.. + 0xF6F66] sar 16, edx
                                                                       [.. + 0xF6F68] sar 16, ebx n) */
                return;
            }
        }
        return;   /* port guard: 0x1FAD3 jle 0x1FDC2 would call f203B4 with uninitialised territories (unreachable) */
    } else {
        /* ties: the tied pairs {own city, neighbour, attitude, priority} */
        setD32(CHOICE_N, 0);                                        /* 0x1FB93 */
        for (i = 0; S8(rec + 0x51u) > i; i++) {                     /* 0x1FB99, 0x1FBB0 */
            for (j = 0; S16(EQ_N(i)) > j; j++) {                    /* 0x1FBB8, 0x1FBD3 */
                if (S16(EQ(i, j) + 4u) != best)                     /* 0x1FC04 */
                    continue;
                c = S32(CHOICE_N);                                  /* 0x1FC26 */
                setD16(PAIR(c), D16(EQ(i, j)));                     /* 0x1FC34 */
                setD16(PAIR(c) + 2u, D16(EQ(i, j) + 2u));           /* 0x1FC64 */
                setD16(PAIR(c) + 6u, D16(EQ(i, j) + 8u));           /* 0x1FC94 */
                setD16(0xF72FCu + 8u * (uint32_t)(c + 1), D16(EQ(i, j) + 6u));   /* 0x1FCCD = PAIR(c) + 4 */
                setD32(CHOICE_N, (uint32_t)(c + 1));                /* 0x1FCD5 */
            }
        }
        if ((D8(FLAGS) & PBEM_BIT) == 0 && S8(rec + 0x9Au) == 3) {  /* 0x1FCDF, 0x1FCF4 */
            setD16(PAIR_SEL, (uint16_t)f1FDDC(n));                  /* 0x1FCFC, 0x1FD01 mov word, ax */
        } else {
            f20068(n);                                              /* 0x1FD0E [M7] */
            setD16(PAIR_SEL, 0);                                    /* 0x1FD13 (esi = 0, kept across the call) */
        }
        e = S16(PAIR_SEL);                                          /* 0x1FD1A dword [0xF77DE] sar 16 */
        a = S16(PAIR(e));                                           /* 0x1FD23 dword [8e + 0xF72FE] sar 16 */
        b = S16(PAIR(e) + 2u);                                      /* 0x1FD30 dword [8e + 0xF7300] sar 16 */
    }
    f203B4(a, b, n);                                                /* 0x1FDCB */
}

/* ==== 0x1FDDC: the human's Earthquake pair chooser ===================================================================== */

int f1FDDC(int n)
{
    char buf[100];       /* [ebp-0x88] */
    int32_t idx;         /* [ebp-0x20] */
    int32_t redraw;      /* [ebp-0x1C] */
    int32_t r;           /* [ebp-0x18] */

    idx = 0;                                                        /* 0x1FDF6 */
    redraw = 1;                                                     /* 0x1FDF9 */
    background3();                                                  /* 0x1FE05..0x1FE32 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x1FE41 */
    f3AF8C(0xF51ACu, 0xC4, 0x195);                                  /* 0x1FE58 left arrow */
    f3AF8C(0xF51B4u, 0x1AF, 0x195);                                 /* 0x1FE6C right arrow */
    fg_vbclose();                                                   /* 0x1FE71 */
    f459E4(n);                                                      /* 0x1FE79 */
    snprintf(buf, sizeof buf, "%s is about to be hit by an Earthquake. Choose which territories will be hit.",
             NAME(n));                                              /* 0x1FE95 (0xB1B4D) */
    f3A9F4(buf);                                                    /* 0x1FEA3 */
    fg_setcolor(5);                                                 /* 0x1FEAA */
    f7853C(PAIR_DLG);                                               /* 0x1FEB7 */
    f3B05C(PANEL_DLG);                                              /* 0x1FEC1 */
    for (;;) {                                                      /* 0x1FEC6 */
        fg_vbopen(S32(BACKBUF));                                    /* 0x1FECD */
        f3C9D0(0xD2, 0x195, 0xDC, 0x13, 5);                         /* 0x1FEEB the sunken name box */
        fg_setcolor(5);                                             /* 0x1FEF2 */
        snprintf(buf, sizeof buf, "%s/%s", TERR_NAME(S16(PAIR(idx))), TERR_NAME(S16(PAIR(idx) + 2u)));
                                                                    /* 0x1FF3B (0xB1B9B): own city / neighbour */
        f3A0B0(0xD2, 0x199, 0xDC, buf);                             /* 0x1FF4D */
        fg_vbclose();                                               /* 0x1FF52 */
        f3F670();                                                   /* 0x1FF57 */
        f3F624(S16(PAIR(idx)), 0xF);                                /* 0x1FF6E dword [8idx + 0xF72FE] sar 16 */
        f3F624(S16(PAIR(idx) + 2u), 0xF);                           /* 0x1FF85 dword [8idx + 0xF7300] sar 16 */
        if (redraw != 0)                                            /* 0x1FF8D */
            f3DA60(S16(PAIR(idx)));                                 /* 0x1FF9E */
        f3CBBC();                                                   /* 0x1FFA3 */
        if (redraw != 0) {                                          /* 0x1FFA8 */
            f7A4C4(-600);                                           /* 0x1FFB5 [ANIM] */
            redraw = 0;                                             /* 0x1FFBA (edx = 0, kept across the call) */
        }
        r = f78BA4(PANEL_DLG, 1);                                   /* 0x1FFC7 */
        if (r == -100)                                              /* 0x1FFCF */
            continue;
        if (r >= 0) {                                               /* 0x1FFDC test ecx, ecx; jl */
            /* BUG(1995): the map branch of f78BA4 searches the stale list 0xF7CA0, not the pairs (file comment, 2);
               ignored as a click on nothing */
            if (!g_original)
                continue;
            idx = r;                                                /* 0x1FFE3 */
            redraw = 1;                                             /* 0x1FFE6 */
            continue;
        }
        if (r == -1)                                                /* 0x1FFF5 */
            break;
        if ((uint32_t)r < (uint32_t)-10)                            /* 0x1FFFD cmp eax, -0xA; jb (unsigned) */
            continue;
        if (r == -10) {                                             /* 0x20006 jbe 0x2002D */
            if (S32(CHOICE_N) - 1 > idx) {                          /* 0x20033 */
                idx++;                                              /* 0x2003F */
                redraw = 1;                                         /* 0x20042 */
            }
            continue;
        }
        if (r == -9 && idx > 0) {                                   /* 0x20008, 0x20011 */
            idx--;                                                  /* 0x2001E */
            redraw = 1;                                             /* 0x20021 */
        }
    }
    f78664(SITE_DLG);                                               /* 0x20053: the Volcano's dialog, see the file comment */
    return idx;                                                     /* 0x20058 */
}

/* ==== 0x203B4 the Earthquake's result ================================================================================= */

void f203B4(int t, int t2, int n)
{
    char text[200];      /* [ebp-0x144] */
    char part[100];      /* [ebp-0x7C] */
    uint32_t o;

    snprintf(text, sizeof text, "Earthquake strikes %s!  The city in %s was %s.", NAME(n), TERR_NAME(t),
             D8(NATION(n) + 0x2Au) != 0 ? DSTR(0xB1BA1u) : DSTR(0xB1BA9u));   /* 0x20412 (0xB1BB4): "reduced" with
                                                                       Engineering (0x203DA), else "eliminated" */
    if (t2 != -1) {                                                 /* 0x2041A */
        o = D8(TSTATE(t2));                                         /* 0x20432 */
        if (o != 0xA)                                               /* 0x2043E cmp edx, 0xA; je */
            snprintf(part, sizeof part, " The neighboring %s city in %s was reduced.", ADJ(o), TERR_NAME(t2));
                                                                    /* 0x20472 (0xB1BE3) */
        else
            snprintf(part, sizeof part, " The neighboring Pirate city in %s was eliminated.", TERR_NAME(t2));
                                                                    /* 0x2048C (0xB1C0F) */
        cat(text, sizeof text, part);                               /* 0x20494..0x204BC */
    }
    if (D8(NATION(n) + 0x2Au) != 0)                                 /* 0x204C6 */
        f78FEC(t, n);                                               /* 0x204D5 reduced to tokens */
    else
        f78F04(t, n);                                               /* 0x204E2 removed */
    if (t2 != -1) {                                                 /* 0x204E7 */
        o = D8(TSTATE(t2));                                         /* 0x204FB */
        if (o != 0xA)                                               /* 0x2050D cmp ebx, 0xA; je */
            f78FEC(t2, D8(TSTATE(t2)));                             /* 0x2051D */
        else
            f78F04(t2, D8(TSTATE(t2)));                             /* 0x2052F */
        f3F624(t2, 0xF);                                            /* 0x2053C */
    }
    f79270(-500, n, -t, text, 2);                                   /* 0x20556 (ebx = -t, push 2) */
}
