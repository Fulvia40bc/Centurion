/* Module MONO of docs/M6_PLAN.md: the MONOTHEISM phase 0x21914 (348 bytes), the human's conversion screen 0x21A70
   (1,052) and the candidate list 0x227B0 (848). Literal translations; the address of each instruction a line comes
   from is in its comment. D = read in the code, G = inference. Prototypes and globals: mono.h.

     0x21914  f21914  MONOTHEISM (phase 16 of the game loop, call at 0x55098; the loop plays no music for it)
     0x21A70  f21A70  a human converts one territory (caller 0x21A37)
     0x227B0  f227B0  the candidate territories of a nation (caller 0x219FD)

   Not ported here (module M7, AI; its call site stops the port with not_ported(0x21E8C); the static stub below
   carries the name, so the call site reads as in the original and a later translation that exports the name collides
   with the stub at compile time instead of being shadowed by it):
     0x21E8C  a computer converts (2,340 bytes): its candidates as 12-byte stack records {t, nation, count}; with fewer
              than 9 cities and board tokens / 2 >= cities + 1 the cities first, bubble-sorted descending by int8
              0xEED84[160n + t] (the opening-map table) then int32 0xF52E0[10n + owner] (the score matrix 0x7C960);
              otherwise the token territories with their largest foreign stack, sorted by 0xEED84, 0xF52E0, count;
              the first one converted as 0x21A70 does, shown with "%s converts %s by Montheism!" (0xB1EEA at 0x221AC,
              0xB1F07 at 0x226A0) through f79270(-550), f78A20(300), then f79270(-500) (CONTINUE, D). Its texts are
              corrected by mono_converts_fmt below (for M7). Risk for M7 (M6_PLAN risk 7, G): when every candidate is
              a city but the city branch is not taken, the token branch sorts 0 records and converts the territory of
              the uninitialised stack record 0.
   So today a MONOTHEISM stops at the first computer (or PBEM human) holding Monotheism with a candidate; an all-human
   game runs it to the end. Built with -DMONO_TEST (port/test/mono_test.c) the call goes to the test's recorder, which
   returns.

   Play by e-mail (dropped by Erik's ruling): no PBEM routine is called here. The bit-0x80 test of byte [0xE1624] is
   kept literally: in PBEM 0x21914 hands every nation, humans too, to 0x21E8C.

   Watcom runtime -> standard C:
     0x7DB3D sprintf -> snprintf bounded to the stack buffer: 200 bytes for 0x21A70 ([ebp-0xF8]..[ebp-0x30])
     0x7D5D0 memset -> memset of the local array (0x22884: 0x274 bytes of 0xFF, the int32 [157] filled with -1)
   __CHK dropped. No game_rand f78AE4 call in these functions (D). No clock() wait other than f78A20(1000) after a
   conversion (D).

   Data (D, read in the code at the addresses given):
     int16 0xBB3FA / 0xBB3FC / 0xBB3FE / 0xBB400 the message area (x, y, w, h) = 0xA9, 0x154, 0x12C, 0x23 (0x21946..
       0x2195B; not restored afterwards, as in tax.c's 0x66083)
     int16 [0xF06AE] / 0xF06B0[] the player list (nations 1..9 with +0x9A != 0, ascending), the first read as dword
       [0xF06AE] sar 16; int32 [0xF5940] the first player button of the panel; int32 [0xF40F8] the current nation
     0xF596C the map screen's panel dialog; 0xBE9F4 the conversion buttons (0x39 (251,385) 13x19, 0x3A (376,385) 13x19,
       0x58 CONVERT TERRITORY (220,405) 200x19, 0x2A FINISHED (220,425) 200x19); the dispatcher returns -9, -10, -16
       and -1 for 0x39, 0x3A, 0x58 and 0x2A (dispatch.c: 0x58 at 0x419AB)
     sprites 0xF51AC / 0xF51B4 the left / right arrows; [0xF52C8] back buffer, [0xF52CC] background buffer; int8
       [0xEE168] the background number (3)
     names 0xC768C[n], territory names 0xC77D8[t]; texts 0xB1E9F "%s, possession of Monotheism allows conversion of a
       neighboring territory." and 0xB1E9E "" (the empty "nothing happened" message; D: its own NUL, right after the
       NUL of "Crete can not be the Victim of Barbarian Hordes.")
     the choice list int32 0xF7CA0[157] / int32 [0xF7C9C]; the land lists: [0xF8E7C + 4t] a handle to int16 count,
       int16 neighbours (landadj.dat, pruned to the territories in play at start-up: gfxload.c; every neighbour of
       territories 0..156 is in 0..156, D: the file)
     nation n = 0xF14CC + 199n: +0x3A Monotheism, +0x3B Theology (D: the advance names 0xC7DC4[i] belong to +0x24 +
       i, Monotheism i = 22, Theology 23), +0x51 int8 cities, +0x98 int8 tokens on the board, +0x99 int8 stock, +0x9A
       0 out / 2 computer / 3 human
     area record r = 0xE31B8 + 69r (territory r - 1): +0x0E + n int8 n's tokens (byte-tested != 0, then added as int8),
       +0x3B + n the "lost territory" mark (conflict.h; G: the meaning); territory state t = 0xE2C18 + 3t: +0 owner,
       +2 bit 0x01 a city

   1995 bugs (fixed unless g_original):
     - 0x227B0 lets a city territory that fails the city rule (owner with Monotheism / Theology, the Pirates' city,
       or n already at 9 cities) fall through to the token rule (0x22AE6); with tokens of an unprotected nation 1..9
       there it is listed, and 0x21A70's CONVERT tests only the city bit (0x21CC0) and converts the city: a protected
       city taken, or a 10th city appended past n's 9-byte city list (+0x52 + 9) by f78E74. Fixed: such a territory
       is not listed (D code; G reach: after conflicts, city building and REMOVE SURPLUS no tokens of nations 1..9
       share a city territory, and the calamities bring only Barbarians (nation 11)).
   The candidate list never holds a territory where n itself has tokens, so the k == n pass of the token conversion
   is unreachable (G).
   Text slips (Erik's ruling of 19 Nov, fixed unless g_original): "Montheism" x2 (0xB1EEA, 0xB1F07) are both in
   0x21E8C, which is not translated yet: mono_converts_fmt gives M7 the corrected format.
   Kept as the original (D unless marked):
     - 0x21A70 never calls f7844C: the player buttons keep the colour 0 that f783B8 gave them (no current player is
       highlighted during the conversion, unlike the city screens; G: an omission, screen only).
     - 0x21A70 prints its prompt with f3A9F4 inside the back-buffer fg_vbopen / fg_vbclose (no f3A924 first, unlike
       0x5C880).
     - 0x227B0 counts the tokens of nations 1..9 only: Barbarian tokens (nation 11) in a candidate territory are not
       counted, not checked for Monotheism and not converted (they stay beside the converter's tokens; G: rare).
     - 0x227B0 tests the city owner's Monotheism / Theology but not the tokens' owners in a city territory (none after
       REMOVE SURPLUS, G); and n's own tokens are never part of a candidate (own territories are excluded).
     - 0x21914 calls f553C0 (the phase music) once per nation with candidates, not once per phase.
     - The message area set at 0x21946 is left as it is after the phase.
   Easter eggs / oddities: the empty "nothing happened" message 0xB1E9E (M6_PLAN egg 15: a 100 ms home view of the
   first player and no text; D: the empty string sits between CAL5's Crete text and this module's prompt); the
   computers' "Montheism" (text slip, in 0x21E8C). */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "fg.h"
#include "game.h"
#include "kit.h"
#include "loop.h"
#include "map.h"
#include "mapdata.h"
#include "mono.h"
#include "move.h"
#include "panel.h"
#include "rules.h"
#include "state.h"
#include "ui.h"

#define NATION(n)    (0xF14CCu + 199u * (uint32_t)(n))
#define NAME(n)      DSTR(D32(0xC768Cu + 4u * (uint32_t)(n)))
#define TERR_NAME(t) DSTR(D32(0xC77D8u + 4u * (uint32_t)(t)))
#define AREA(r)      (0xE31B8u + 69u * (uint32_t)(r))   /* area record r (territory r - 1) */
#define TSTATE(t)    (0xE2C18u + 3u * (uint32_t)(t))
#define FLAGS        0xE1624u
#define PBEM_BIT     0x80u
#define CUR_NATION   0xF40F8u
#define PANEL_DLG    0xF596Cu
#define PLAYER_BASE  0xF5940u
#define PLAYERS      0xF06AEu   /* int16 count */
#define PLAYER_LIST  0xF06B0u   /* int16[] */
#define BACKBUF      0xF52C8u
#define BACKGROUND   0xF52CCu
#define MONO_DLG     0xBE9F4u
#define CHOICE       0xF7CA0u   /* int32[] */
#define CHOICE_N     0xF7C9Cu   /* int32 */
#define LANDADJ      0xF8E7Cu   /* uint32[160]: handles (game.h GAME_LANDADJ_PTR) */
#define CH(i)        S32(CHOICE + 4u * (uint32_t)(i))

/* ---- the call into code that is not ported yet (see the file comment) -------------------------------------------- */

#ifdef MONO_TEST
void mono_test_f21E8C(int n);
#define f21E8C(n) mono_test_f21E8C(n)
#else
static void f21E8C(int n)   /* M7 (AI): computer n converts a territory */
{
    (void)n;
    not_ported(0x21E8Cu);
}
#endif

const char *mono_converts_fmt(uint32_t addr)
{
    if (g_original)   /* BUG(1995): the misspelling "Montheism" (0xB1EEA, 0xB1F07; text) */
        return DSTR(addr);
    return "%s converts %s by Monotheism!";
}

/* a 16-bit list word at the handle h (the land lists live in a heap block, game.h): movsx word [eax] */
static int32_t hword(uint32_t h)
{
    const uint8_t *p = load_heap_ptr(h, 2);
    return p != NULL ? (int32_t)(int16_t)(p[0] | p[1] << 8) : 0;   /* port guard: an unresolved handle reads 0 */
}

/* ---- 0x21914 MONOTHEISM ------------------------------------------------------------------------------------------ */

void f21914(void)
{
    int32_t none;   /* [ebp-0x18] 1 until a nation has a candidate */
    int32_t i;      /* [ebp-0x1C] */
    int32_t n;      /* [ebp-0x20] */
    int16_t p;

    none = 1;                                                         /* 0x21929 */
    setD16(0xBB3FAu, 0xA9);                                           /* 0x21946 the message area */
    setD16(0xBB3FCu, 0x154);                                          /* 0x2194D */
    setD16(0xBB3FEu, 0x12C);                                          /* 0x21954 */
    setD16(0xBB400u, 0x23);                                           /* 0x2195B */
    setD16(PLAYERS, 0);                                               /* 0x21962 */
    for (i = 1; i <= 9; i++) {                                        /* 0x21969, 0x21978 */
        if (D8(NATION(i) + 0x9A) == 0)                                /* 0x21985 byte [199i + 0xF1566] */
            continue;
        p = S16(PLAYERS);                                             /* 0x2198F movsx edx, ax */
        setD16(PLAYERS, (uint16_t)(p + 1));                           /* 0x21999 */
        setD16(PLAYER_LIST + 2u * (uint32_t)(int32_t)p, (uint16_t)i); /* 0x219A2 */
    }
    f783B8(PANEL_DLG, S32(PLAYER_BASE));                              /* 0x219B7 */
    for (n = 1; n <= 9; n++) {                                        /* 0x219BC, 0x219D3 */
        if (D8(NATION(n) + 0x9A) == 0)                                /* 0x219E0 byte [199n + 0xF1566] */
            continue;
        if (D8(NATION(n) + 0x3A) == 0)                                /* 0x219F1 byte [199n + 0xF1506] Monotheism */
            continue;
        f227B0(n);                                                    /* 0x219FD */
        if (S32(CHOICE_N) == 0)                                       /* 0x21A02 */
            continue;
        none = 0;                                                     /* 0x21A0E */
        f553C0();                                                     /* 0x21A11 */
        if (S8(NATION(n) + 0x9A) == 3                                 /* 0x21A1D dword [199n + 0xF1563] sar 24 */
            && (D8(FLAGS) & PBEM_BIT) == 0)                           /* 0x21A2B */
            f21A70(n);                                                /* 0x21A37 */
        else
            f21E8C(n);                                                /* 0x219C8 [M7] */
    }
    if (none != 0)                                                    /* 0x21A3E */
        f79270(100, S16(PLAYER_LIST), -200, DSTR(0xB1E9Eu), 0);       /* 0x21A5E dword [0xF06AE] sar 16; "" */
}

/* ---- 0x21A70 the human's conversion ------------------------------------------------------------------------------ */

void f21A70(int n)
{
    char    buf[200];   /* [ebp-0xF8] */
    int32_t sel;        /* [ebp-0x28] */
    int32_t redraw;     /* [ebp-0x24] */
    int32_t loop;       /* [ebp-0x20] */
    int32_t r;          /* [ebp-0x2C] */
    int32_t k;          /* [ebp-0x30] */
    int32_t x;          /* [ebp-0x1C] */

    snprintf(buf, sizeof buf, "%s, possession of Monotheism allows conversion of a neighboring territory.",
             NAME(n));                                                /* 0x21AA2 (0xB1E9F) */
    f7853C(MONO_DLG);                                                 /* 0x21AAF */
    f3CD6C(3);                                                        /* 0x21AB9 */
    setD8(0xEE168u, 3);                                               /* 0x21AE3 */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BACKGROUND), S32(BACKBUF));   /* 0x21AE9 */
    fg_vbopen(S32(BACKBUF));                                          /* 0x21AF8 */
    f3AF8C(0xF51ACu, 0xFB, 0x181);                                    /* 0x21B0F */
    f3AF8C(0xF51B4u, 0x178, 0x181);                                   /* 0x21B23 */
    f3A9F4(buf);                                                      /* 0x21B2E */
    fg_setcolor(5);                                                   /* 0x21B35 */
    fg_vbclose();                                                     /* 0x21B3D */
    f7876C();                                                         /* 0x21B42 */
    f3B05C(PANEL_DLG);                                                /* 0x21B4E */
    sel = 0;                                                          /* 0x21B53 */
    redraw = 1;                                                       /* 0x21B5B */
    loop = 1;                                                         /* 0x21B5E */
    setD32(CUR_NATION, (uint32_t)n);                                  /* 0x21B64 */
    while (loop != 0) {                                               /* 0x21B69 */
        fg_vbopen(S32(BACKBUF));                                      /* 0x21B7A */
        f3C9D0(0x109, 0x181, 0x6E, 0x13, 5);                          /* 0x21B98 */
        fg_setcolor(5);                                               /* 0x21B9F */
        f3A0B0(0x109, 0x185, 0x6E, TERR_NAME(CH(sel)));               /* 0x21BC7 */
        fg_vbclose();                                                 /* 0x21BCC */
        f3F670();                                                     /* 0x21BD1 */
        f3F624(CH(sel), 15);                                          /* 0x21BE5 */
        if (redraw != 0) {                                            /* 0x21BEF */
            f459E4(n);                                                /* 0x21BF4 */
            f3DA60(CH(sel));                                          /* 0x21C03 */
        }
        f3CBBC();                                                     /* 0x21C08 */
        if (redraw != 0)                                              /* 0x21C0D */
            redraw = 0;                                               /* 0x21C15 */
        r = f78BA4(PANEL_DLG, 1);                                     /* 0x21C22 */
        if (r == -100)                                                /* 0x21C2A */
            continue;
        if (r >= 0) {                                                 /* 0x21C39 jl */
            sel = r;                                                  /* 0x21C3E a map click on a listed territory */
            redraw = 1;                                               /* 0x21C41 */
            continue;
        }
        if (r == -1)                                                  /* 0x21C53 FINISHED */
            break;
        if ((uint32_t)r < (uint32_t)-0xA) {                           /* 0x21C5F jb 0x21E6A */
            if (r != -0x10)                                           /* 0x21E6D */
                continue;
            /* 0x21CAD CONVERT TERRITORY */
            if ((D8(TSTATE(CH(sel)) + 2) & 1) != 0) {                 /* 0x21CC0 a city */
                setD8(AREA(CH(sel) + 1) + 0x3Bu + (uint32_t)D8(TSTATE(CH(sel))), 1);   /* 0x21D04 [69(t+1) + o + 0xE31F3] */
                f78F04(CH(sel), (int)D8(TSTATE(CH(sel))));            /* 0x21D31 */
                f78E74(CH(sel), n);                                   /* 0x21D43 */
            } else {
                for (k = 0; k < 9; k++) {                             /* 0x21D72, 0x21D7F */
                    if (D8(AREA(CH(sel) + 1) + 0x0Fu + (uint32_t)k) == 0)   /* 0x21D9F [69(t+1) + k + 0xE31C7] */
                        continue;
                    setD8(AREA(CH(sel) + 1) + 0x3Cu + (uint32_t)k, 1);      /* 0x21DC3 [.. + 0xE31F4] */
                    x = S8(AREA(CH(sel) + 1) + 0x0Fu + (uint32_t)k);         /* 0x21DE6 dword [.. + 0xE31C4] sar 24 */
                    f57CE8(CH(sel), k + 1, 0, x);                     /* 0x21E05 */
                    setD8(NATION(k + 1) + 0x98, (uint8_t)(D8(NATION(k + 1) + 0x98) - (uint8_t)x));   /* 0x21E17 */
                    setD8(NATION(k + 1) + 0x99, (uint8_t)(D8(NATION(k + 1) + 0x99) + (uint8_t)x));   /* 0x21E2A */
                    f57E28(CH(sel), n, x);                            /* 0x21E40 */
                    setD8(NATION(n) + 0x99, (uint8_t)(D8(NATION(n) + 0x99) - (uint8_t)x));           /* 0x21E4F */
                    setD8(NATION(n) + 0x98, (uint8_t)(D8(NATION(n) + 0x98) + (uint8_t)x));           /* 0x21E5F */
                }
            }
            /* 0x21D48 */
            f459E4(n);                                                /* 0x21D4B */
            f3D52C(0, 0);                                             /* 0x21D54 */
            f3CBBC();                                                 /* 0x21D59 */
            f78A20(1000);                                             /* 0x21D65 */
            loop = 0;                                                 /* 0x21D6A edx (0, kept by f78A20) */
        } else if (r == -0xA) {                                       /* 0x21C65 jbe 0x21C8C */
            if (S32(CHOICE_N) - 1 > sel) {                            /* 0x21C95 jle */
                sel++;                                                /* 0x21C9E */
                redraw = 1;                                           /* 0x21CA1 */
            }
        } else if (r == -9) {                                         /* 0x21C67 */
            if (sel > 0) {                                            /* 0x21C74 jle */
                sel--;                                                /* 0x21C7D */
                redraw = 1;                                           /* 0x21C80 */
            }
        }
    }
    f78664(MONO_DLG);                                                 /* 0x21E7D */
}

/* ---- 0x227B0 the candidate list ---------------------------------------------------------------------------------- */

void f227B0(int n)
{
    int32_t  own[157];    /* [ebp-0x2B0] the territories with n's city or tokens */
    int32_t  adj[157];    /* [ebp-0x524] 1 adjacent, -1 not (or own) */
    int32_t  cnt;         /* [ebp-0x28] */
    int32_t  t;           /* [ebp-0x2C] */
    int32_t  i;           /* [ebp-0x3C] */
    int32_t  j;           /* [ebp-0x38] */
    uint32_t list;        /* [ebp-0x20] */
    int32_t  can_city;    /* [ebp-0x18] */
    int32_t  stock;       /* [ebp-0x1C] */
    int32_t  total = 0;   /* [ebp-0x30] (set before each use, 0x22AEB) */
    int32_t  ok = 0;      /* [ebp-0x34] (set before each use, 0x22AE8) */
    int32_t  o, c;

    setD32(CHOICE_N, 0);                                              /* 0x227CD */
    for (i = 0; i < 0x9D; i++)                                        /* 0x227D3, 0x227DE */
        setD32(CHOICE + 4u * (uint32_t)i, 0xFFFFFFFFu);               /* 0x227EA */
    cnt = 0;                                                          /* 0x227F9 */
    for (t = 0; t < 0x9D; t++) {                                      /* 0x227FC, 0x22807 */
        if (!((D8(TSTATE(t) + 2) & 1) != 0 && (int32_t)D8(TSTATE(t)) == n)   /* 0x22822, 0x2283E n's city */
            && D8(AREA(t + 1) + 0x0Eu + (uint32_t)n) == 0)            /* 0x22856 [69(t+1) + n + 0xE31C6] n's tokens */
            continue;
        own[cnt++] = (int32_t)(int16_t)t;                             /* 0x2285F movsx edx, word [ebp-0x2C] */
    }
    memset(adj, 0xFF, 0x274);                                         /* 0x22884 */
    for (i = 0; i < cnt; i++) {                                       /* 0x22889, 0x22894 */
        list = D32(LANDADJ + 4u * (uint32_t)own[i]);                  /* 0x228A6 */
        for (j = 1; hword(list) >= j; j++)                            /* 0x228B0, 0x228C2 movsx; cmp; jl */
            adj[hword(list + 2u * (uint32_t)j)] = 1;                  /* 0x228D2, 0x228D5 */
    }
    for (i = 0; i < cnt; i++)                                         /* 0x228E4, 0x228EF */
        adj[own[i]] = -1;                                             /* 0x22901 */
    can_city = S8(NATION(n) + 0x51) != 9;                             /* 0x22915..0x22929 dword [199n + 0xF151A] sar 24 */
    stock = S8(NATION(n) + 0x99);                                     /* 0x22933..0x2293C dword [199n + 0xF1562] sar 24 */
    for (t = 0; t < 0x9D; t++) {                                      /* 0x22941, 0x22A3E */
        if (adj[t] == -1)                                             /* 0x22A55 */
            continue;
        if (can_city != 0 && (D8(TSTATE(t) + 2) & 1) != 0) {          /* 0x22A5D, 0x22A71 */
            o = D8(TSTATE(t));                                        /* 0x22A86 */
            if (o != 10                                               /* 0x22A91 not the Pirates */
                && D8(NATION(o) + 0x3A) == 0                          /* 0x22AB3 Monotheism */
                && D8(NATION(o) + 0x3B) == 0)                         /* 0x22AD9 Theology */
                goto add;                                             /* 0x22AE0 je 0x22A1D */
        }
        /* 0x22AE6: the tokens of the other nations */
        if (!g_original && (D8(TSTATE(t) + 2) & 1) != 0)
            continue;   /* BUG(1995): a city territory that failed the city rule above (owner protected, the Pirates,
                           or n at 9 cities) is listed through other nations' tokens there, and 0x21A70's CONVERT
                           (0x21CC0) then takes the city itself (D code; G reach: no tokens of nations 1..9 share a
                           city territory at phase 16) */
        ok = 0;                                                       /* 0x22AE8 */
        total = 0;                                                    /* 0x22AEB */
        for (i = 0; i < 9; i++) {                                     /* 0x22AEE, 0x2294F */
            if (n - 1 == i)                                           /* 0x2295C */
                continue;
            if (D8(AREA(t + 1) + 0x0Fu + (uint32_t)i) == 0)           /* 0x22973 [69(t+1) + i + 0xE31C7] */
                continue;
            total += S8(AREA(t + 1) + 0x0Fu + (uint32_t)i);           /* 0x22998 dword [.. + 0xE31C4] sar 24 */
        }
        if (total != 0 && total <= stock)                             /* 0x2299D, 0x229A9 jg */
            ok = 1;                                                   /* 0x229AB */
        for (i = 0; i < 9; i++) {                                     /* 0x229B4, 0x229BF */
            if (n - 1 == i)                                           /* 0x229CC */
                continue;
            if (D8(AREA(t + 1) + 0x0Fu + (uint32_t)i) == 0)           /* 0x229E3 */
                continue;
            if (D8(NATION(i + 1) + 0x3A) != 0 || D8(NATION(i + 1) + 0x3B) != 0) {   /* 0x229F6, 0x22A09 */
                ok = 0;                                               /* 0x22A14 */
                break;
            }
        }
        if (ok == 0)                                                  /* 0x22A17 */
            continue;
    add:
        c = S32(CHOICE_N);                                            /* 0x22A1D */
        setD32(CHOICE_N, (uint32_t)(c + 1));                          /* 0x22A2A */
        setD32(CHOICE + 4u * (uint32_t)c, (uint32_t)t);               /* 0x22A32 */
    }
}
