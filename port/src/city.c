/* Module CITY of docs/M6_PLAN.md: CITY CONSTRUCTION 0x32214 (4,564 bytes) with the human's screen, its candidate
   list 0x334F4 (724), REMOVE SURPLUS 0x5C238 (752) with its token removal 0x5C528 (260), CITY SUPPORT 0x5C62C (292)
   with the support check of one nation 0x5C750 (304) and the human's city chooser 0x5C880 (1,204, REDUCE CITY).
   Literal translations; the address of each instruction a line comes from is in its comment. D = read in the code,
   G = inference. Prototypes and globals: city.h.

     0x32214  f32214  CITY CONSTRUCTION (phase 8 of the game loop, call at 0x54B8A)
     0x334F4  f334F4  the candidate sites of a nation (callers 0x32ACC, 0x32B5F)
     0x5C238  f5C238  REMOVE SURPLUS (phase 9, f5C238(1) at 0x54BDD; the AI's simulations call it with 0)
     0x5C528  f5C528  k tokens of a nation removed from a territory (callers 0x5C3DA, 0x5C444, 0x5C4D5)
     0x5C62C  f5C62C  CITY SUPPORT (phases 10 and 17, calls at 0x54C11 and 0x550CC)
     0x5C750  f5C750  the support check of one nation (callers 0x5C6DF, CAL1's Slave Revolt 0x2378F)
     0x5C880  f5C880  REDUCE CITY: a human picks a city to reduce (callers 0x5C840, CAL1's 0x23477, 0x239EE, 0x23D5C,
                      0x24137)

   Not ported here (module M7, AI; each call site stops the port with not_ported(address); the static stubs below
   carry the names, so the call sites read as in the original and a later translation that exports one of these names
   collides with its stub at compile time instead of being shadowed by it):
     0x333E8  a computer builds a city (0x78E74, f57CE8, the city state's bit 0x02 cleared, "%s builds a city in %s."
              or "... using Architecture.", f79270); f32214 0x330B8 (k tokens, arch 0) and 0x33241 (arch 0x63)
     0x5DE5C  a computer reduces one of its cities; f5C750 0x5C862
   So a CITY CONSTRUCTION with computers stops at the first computer that builds, and a CITY SUPPORT at the first
   computer short of tokens; an all-human game (0 computers) runs all three phases to the end. Built with -DCITY_TEST
   (port/test/city_test.c) the two go to the test's recorders instead, which return.

   Play by e-mail (dropped by Erik's ruling): no PBEM routine is called from these functions. The bit-0x80 tests of
   byte [0xE1624], the local-player byte [0xE1622] (int8, read as dword [0xE161F] sar 24), the phase test of 0x5C750
   and the PBEM interaction flag int16 [0xF408E] (written by 0x32214, read by the loop) are kept literally: they only
   choose between the human screen, the automatic build and the computer path.

   Watcom runtime -> standard C:
     0x7DB3D sprintf -> snprintf bounded to the stack buffer: 100 bytes for 0x32214 ([ebp-0xCC]..[ebp-0x68]), 0x5C528
             ([ebp-0x7C]..[ebp-0x18]) and 0x5C62C ([ebp-0x80]..[ebp-0x1C]); 152 for 0x5C750 ([ebp-0xB0]..[ebp-0x18]);
             200 for 0x5C880 ([ebp-0xFC]..[ebp-0x34], also its inline strcpy at 0x5C9B9)
     0x8C897 itoa(v, buf, 10) -> snprintf "%d"
     0x7D5D0 memset -> memset in g_ds
     0x8A080 clock: the computer's wait in 0x32214 (0x32CA2 start, 0x33344 / 0x33375 the busy waits until start + (2000,
             or unattended 500 / 1000 with int16 [0xF40BA], / [0xBB3F4]) / 10 hundredths, unsigned ja). The port's clock
             (ui_kit2.c, static there) advances only inside f78A20, so the port waits f78A20(that time) when nothing
             waited since the start (no 300 ms highlight, no 0x333E8 message), and not at all otherwise: every path
             that waits first spends at least the target there (0x333E8's message takes 2000 ms, or 500 / 1500
             unattended, per its record; its CONTINUE box comes only with the toggle bit 0x01 of [0xF0980], which also
             skips this wait). G (0x333E8 is not translated yet), as tax.c's final wait
   __CHK dropped. No game_rand f78AE4 call in these functions (D).

   Data (D, read in the code at the addresses given):
     int16 0xF06AE / 0xF06B0[] the player list (nations 1..9 with +0x9A != 0, ascending), read as dword [0xF06AE] sar 16
       for the first; int32 [0xF40F8] the current nation; int32 [0xF5940] the first player button of the panel
     0xF596C the map screen's panel dialog; 0xC154C the city construction buttons (0x39 (251,365), 0x3A (376,365), 0x3B
       (296,385) and 0x3C (330,385) disabled in the data, 0x3D BUILD CITY (220,405), 0x2A FINISHED (220,425)); 0xC5DD8
       the reduce buttons (0x39 (251,385), 0x3A (376,385), 0x3E REDUCE CITY (220,405)); the dispatcher returns -9, -10,
       -11, -12, -13, -14 and -1 for 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E and 0x2A (dispatch.c)
     sprites 0xF51AC / 0xF51B4 the left / right arrows; [0xF52C8] back buffer, [0xF52CC] background buffer; int8
       [0xEE168] the background number (3)
     names 0xC768C[n], territory names 0xC77D8[t] (0xC77D4 = entry -1 is "PIRACY", the last calamity name)
     area record r = 0xE31B8 + 69r (territory r - 1): +1 int8 nations there, +0x0E + n int8 n's tokens; area record 114
       is FAYUM (territory 113); territory record t = 0xE1638 + 35t, byte +1: bit 0x40 a city site, bits 3..5 the
       population limit; territory state t = 0xE2C18 + 3t: +0 owner, +1 the round it was built, +2 bit 0x01 a city,
       bit 0x02 a site the AI marked (G)
     nation n = 0xF14CC + 199n: +0x27 Agriculture, +0x31 Architecture, +0x3E int8 treasury, +0x51 int8 cities, +0x52 +
       j city j's territory + 1, +0x98 int8 tokens on the board, +0x99 int8 stock, +0x9A 0 out / 2 computer / 3 human
     int16 0xF3580[9][157] the AI's territory flags (row n - 1; the low byte's bit 0x80 tested)
     uint8 [0xF0980] bit 0x01 the "City Construction AI Pause" toggle (G: a CONTINUE box instead of 2000 ms); int16
       [0xF40BA] the self-running demo; int16 [0xF40B2] the dead cost switch; int16 [0xF40FC] the round
     the empty "nothing happened" messages 0xB3ACA, 0xB5D54 and 0xB5DC7 ("")

   1995 bugs, fixed unless g_original (each marked BUG(1995) in the code; grades D code / G effect):
     1. 0x32214, a computer with more candidates than free city slots and Architecture (0x32D51, 0x32DCC, 0x32E66,
        0x32F1B): the four reorder passes compare the tokens of area record s (69s + 0xE31C7 + n - 1), territory s - 1,
        with the need of site s; every other read of the function uses record s + 1. So the computer's sites are
        ordered by its neighbour territory's tokens. Fixed: record s + 1. D (code), G (effect: the order of the
        computer's candidates, hence which sites it builds when it cannot build all; M7 checks this first, with
        --original, against DOSBox-X).
     2. 0x5C750: the loop ends only when board - loss >= 2 * cities. Board - loss is negative when the Slave Revolt
        loss int16 [0xF7AFC] (capped to the board when the calamity struck, 0x23686) exceeds the tokens on the board by
        the time of phase 17 (later calamities); with no city left nothing can raise it: the human gets the REDUCE CITY
        screen with an empty list for ever (and 2 below), a computer loops in 0x5DE5C. Fixed: with no city (int8 +0x51
        <= 0) the check returns; M6_PLAN risk 6. D (code), G (reach).
     3. 0x5C880 with an empty list (no city of the listed nations): entry 0xF7CA0[0] is the filler -1, so the box
        shows territory -1's name "PIRACY" (0xC77D4), the owner is read from byte 0xE2C15 (before the territory states),
        f3F624(-1, 15) writes 15 into byte 0xFFB5F, the top byte of the heap pointer [0xFFB5C] (seaadj.dat's block, a
        port handle the write invalidates), f3DA60(-1) reads before 0xC803C, and REDUCE calls f78FEC(-1, owner).
        Fixed: with an empty list the box stays empty, the owner is 0, no highlight is set, the map is redrawn as it
        is (f3D52C(0, 0)) and REDUCE CITY only leaves the screen (no f78FEC, the list untouched). REDUCE must still
        leave: dialog 0xC5DD8 has no FINISHED and nothing on the panel returns -1 here (dispatch.c: only 0x2A, and
        the zoom's own wait), so in the original REDUCE CITY is the screen's only exit. Unreachable from 0x5C750 with
        fix 2; the calamity callers are G. The shape of tax.c's fix 3 (0x66D8C), except for the exit.
   Kept as the original (D unless marked):
     - 0x334F4's Architecture rule hard-codes 3 / 6 / 12 and ignores the dead switch [0xF40B2] (M6_PLAN egg 11: with
       the switch set, cities cost 1 and 2 tokens; no store writes it, D).
     - 0x32214's human builds are inline (no 0x78E74) and do not clear bit 0x02 of the city state, which 0x333E8 does
       (G: a site the AI marked keeps its mark under a human city; only the AI reads the bit).
     - 0x32214: max [ebp-0x44] and min [ebp-0x40] are set only when Architecture is held and keep their values across
       nations; the clamp contrib = min(contrib, max) runs every turn of the loop (contrib is 0 without Architecture).
     - 0x32214's computer path reads area record 0 (territory -1) for entries already built (-1) in its Architecture
       pass (G: record 0 is all zero in the data, so the test after it fails).
     - 0x32214's automatic build ("%s builds a city in %s", without the period that 0x333E8's text has) is reached
       only in PBEM without need_screen: without PBEM need_screen is always 1, so humans always get the screen.
     - 0x5C238 removes all of n's tokens from a city territory whoever owns the city, and adds 1 to the population
       limit with Agriculture whether or not n is alone there (unlike f78D08); with show == 0 it calls 0x5C528 for
       AI-marked sites also with k == 0.
     - 0x5C880: after REDUCE fg_vbopen is called without its fg_vbclose (f3CBBC's fg_vbopen / fg_vbclose follows);
       mode 1's inner loop appends a city once per matching list entry.
   Easter eggs / oddities: the special case of FAYUM (territory 113: one nation alone there with >= 6 tokens and
   Agriculture) forces the PBEM screen (G: a leftover; the rules give Fayum nothing special); the empty "nothing
   happened" messages (M6_PLAN egg 15); "TREASURY" and "CONTRIBUTION" are each stored twice (0xB3A9E / 0xB3AB4,
   0xB3AA7 / 0xB3ABD, egg 28); the dead cost switch [0xF40B2] (egg 11). */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "city.h"
#include "fg.h"
#include "game.h"
#include "kit.h"
#include "map.h"
#include "mapdata.h"
#include "move.h"
#include "panel.h"
#include "rules.h"
#include "start.h"
#include "state.h"
#include "ui.h"

#define NATION(n)    (0xF14CCu + 199u * (uint32_t)(n))
#define NAME(n)      DSTR(D32(0xC768Cu + 4u * (uint32_t)(n)))
#define TERR_NAME(t) DSTR(D32(0xC77D8u + 4u * (uint32_t)(t)))
#define AREA(r)      (0xE31B8u + 69u * (uint32_t)(r))   /* area record r (territory r - 1) */
#define TOKENS(r, n) S8(AREA(r) + 0x0Eu + (uint32_t)(n))
#define TSTATE(t)    (0xE2C18u + 3u * (uint32_t)(t))
#define TERRFLAG(t)  D8(0xE1639u + 35u * (uint32_t)(t))  /* territory record +1 */
#define SITE_BIT(t)  ((TERRFLAG(t) >> 6) & 1)             /* add al, al; shr al, 7 */
#define POP_LIMIT(t) ((TERRFLAG(t) >> 3) & 7)             /* shl al, 2; shr al, 5 */
#define FLAGS        0xE1624u
#define PBEM_BIT     0x80u
#define LOCAL_PBEM   0xE1622u   /* int8: dword [0xE161F] sar 24 */
#define PHASE        0xF4102u
#define ROUND        0xF40FCu   /* int16 */
#define CUR_NATION   0xF40F8u
#define PANEL_DLG    0xF596Cu
#define PLAYER_BASE  0xF5940u
#define PLAYERS      0xF06AEu   /* int16 count */
#define PLAYER_LIST  0xF06B0u   /* int16[] */
#define BACKBUF      0xF52C8u
#define BACKGROUND   0xF52CCu
#define BUILD_DLG    0xC154Cu
#define REDUCE_DLG   0xC5DD8u
#define CHOICE       0xF7CA0u   /* int32[] */
#define CHOICE_N     0xF7C9Cu   /* int32 */
#define NEED         0xF7F20u   /* int32[] */
#define TOGGLES      0xF0980u
#define SRD          0xF40BAu   /* int16 */
#define SURPLUS      0xF5934u   /* uint8[12] */
#define REDUCED      0x109E20u  /* int16 */
#define PBEM_ASK     0xF408Eu   /* int16 */
#define AI_FLAGS     0xF3580u   /* int16[9][157] */
#define COST_SWITCH  0xF40B2u   /* int16 */

/* ---- calls into code that is not ported yet (see the file comment) ----------------------------------------------- */

#ifdef CITY_TEST
void city_test_f333E8(int t, int n, int k, int arch);
void city_test_f5DE5C(int n, const char *msg);
#define f333E8(t, n, k, arch) city_test_f333E8(t, n, k, arch)
#define f5DE5C(n, msg)        city_test_f5DE5C(n, msg)
#else
#include "aicards.h"                            /* f333E8, f5DE5C: the computers' city build and support (M7) */

#endif

/* The player list int16 [0xF06AE] / 0xF06B0[]: the nations 1..9 in play, ascending (0x32233..0x32286, 0x5C272..
   0x5C2BD, 0x5C641..0x5C68B: the same inline loop in each) */
static void player_list(void)
{
    int n;
    setD16(PLAYERS, 0);                                               /* mov word [0xF06AE], dx */
    for (n = 1; n <= 9; n++) {
        if (D8(NATION(n) + 0x9A) != 0) {                              /* byte [199n + 0xF1566] */
            int16_t p = S16(PLAYERS);                                 /* movsx edx, ax */
            setD16(PLAYERS, (uint16_t)(p + 1));
            setD16(PLAYER_LIST + 2u * (uint32_t)(int32_t)p, (uint16_t)n);
        }
    }
}

/* The map screen in the back buffer (0x32500..0x32534, 0x5C2CF..0x5C304, 0x5CA75..0x5CAAA, 0x32C56..0x32C88) */
static void map_screen(void)
{
    f3CD6C(3);
    setD8(0xEE168u, 3);
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BACKGROUND), S32(BACKBUF));
}

/* ---- 0x334F4 the candidate sites --------------------------------------------------------------------------------- */

void f334F4(int arch, int n)
{
    uint32_t rec;     /* [ebp-0x4] */
    int32_t cost6;    /* [ebp-0xC] */
    int32_t cost12;   /* [ebp-0x8] */
    int32_t i;        /* [ebp-0x14] */
    int32_t t;        /* [ebp-0x10] */
    int32_t c;

    rec = NATION(n);                                                  /* 0x3351A */
    if (S16(COST_SWITCH) == 0) {                                      /* 0x3351D the dead switch (egg 11) */
        cost6 = 6;                                                    /* 0x33527 */
        cost12 = 0xC;                                                 /* 0x3352E */
    } else {
        cost6 = 1;                                                    /* 0x33537 */
        cost12 = 2;                                                   /* 0x3353E */
    }
    setD32(CHOICE_N, 0);                                              /* 0x33547 */
    for (i = 0; i < 0x9D; i++)                                        /* 0x33557 */
        setD32(CHOICE + 4u * (uint32_t)i, 0xFFFFFFFFu);               /* 0x33563 */
    for (t = 0; t < 0x9D; t++) {                                      /* 0x336D4 */
        if ((D8(TSTATE(t) + 2) & 1) != 0)                             /* 0x336ED a city stands there */
            continue;
        if (arch != 0 && S8(AREA(t + 1) + 1) == 1) {                  /* 0x336FF, 0x33712 n alone (sar 24) */
            if (SITE_BIT(t) == 1 && TOKENS(t + 1, n) >= 3             /* 0x33746, 0x3376C */
                && TOKENS(t + 1, n) + S8(rec + 0x3E) >= 6) {          /* 0x3379C tokens + treasury */
                c = S32(CHOICE_N);                                    /* 0x337A5 */
                setD32(NEED + 4u * (uint32_t)c, 6);                   /* 0x337B2 [4*(c+1) + 0xF7F1C] */
                setD32(CHOICE_N, (uint32_t)(c + 1));                  /* 0x336C0 */
                setD32(CHOICE + 4u * (uint32_t)c, (uint32_t)t);       /* 0x336C8 */
                continue;
            }
            /* 0x3357A: a site that failed falls here and is dropped by the site test */
            if (SITE_BIT(t) != 0)                                     /* 0x3359B */
                continue;
            if (TOKENS(t + 1, n) < 6)                                 /* 0x335BD */
                continue;
            if (TOKENS(t + 1, n) + S8(rec + 0x3E) < 0xC)              /* 0x335ED */
                continue;
            c = S32(CHOICE_N);                                        /* 0x335F6 */
            setD32(NEED + 4u * (uint32_t)c, 0xC);                     /* 0x33603 */
            setD32(CHOICE_N, (uint32_t)(c + 1));                      /* 0x336C0 */
            setD32(CHOICE + 4u * (uint32_t)c, (uint32_t)t);           /* 0x336C8 */
            continue;
        }
        /* 0x33613 */
        if (SITE_BIT(t) == 1 && TOKENS(t + 1, n) >= cost6) {          /* 0x33634, 0x33656 */
            c = S32(CHOICE_N);                                        /* 0x3365B */
            setD32(NEED + 4u * (uint32_t)c, (uint32_t)cost6);         /* 0x336B3 */
        } else if (SITE_BIT(t) == 0 && TOKENS(t + 1, n) >= cost12) {  /* 0x33687, 0x336A5 */
            c = S32(CHOICE_N);                                        /* 0x336AA */
            setD32(NEED + 4u * (uint32_t)c, (uint32_t)cost12);        /* 0x336B3 */
        } else {
            continue;
        }
        setD32(CHOICE_N, (uint32_t)(c + 1));                          /* 0x336C0 */
        setD32(CHOICE + 4u * (uint32_t)c, (uint32_t)t);               /* 0x336C8 */
    }
}

/* ---- 0x32214 CITY CONSTRUCTION ----------------------------------------------------------------------------------- */

void f32214(void)
{
    char     buf[100];       /* [ebp-0xCC] */
    int32_t  none;           /* [ebp-0x54] 1 until a nation has a candidate */
    int32_t  max;            /* [ebp-0x44] the largest contribution (kept across nations) */
    int32_t  min;            /* [ebp-0x40] the smallest contribution (kept across nations) */
    int32_t  i;              /* [ebp-0x68] */
    int32_t  need_screen;    /* [ebp-0x34] */
    int32_t  special;        /* [ebp-0x38] */
    int32_t  n;              /* [ebp-0x30] */
    uint32_t rec;            /* [ebp-0x1C] */
    int32_t  idx;            /* [ebp-0x58] n - 1 */
    int32_t  arch;           /* [ebp-0x3C] */
    int32_t  sel;            /* [ebp-0x60] (its address goes to f79A9C) */
    int32_t  redraw;         /* [ebp-0x20] */
    int32_t  contrib;        /* [ebp-0x48] */
    int32_t  r;              /* [ebp-0x5C] */
    int32_t  half;           /* [ebp-0x24] */
    int32_t  cnt;            /* [ebp-0x2C] */
    int32_t  j;              /* [ebp-0x64] */
    int32_t  tmp;            /* [ebp-0x28] */
    int32_t  k;              /* [ebp-0x50] */
    int32_t  d;              /* [ebp-0x4C] */
    int32_t  sites[160];     /* [ebp-0x5CC] */
    int32_t  needs[160];     /* [ebp-0x34C] */
    int32_t  c, ms;
    int      waited;         /* port: a wait ran since the computer's start (the file comment) */

    none = 1;                                                         /* 0x3222C */
    max = 0;                                                          /* 0x32235 */
    min = 0;                                                          /* 0x32238 */
    player_list();                                                    /* 0x32242..0x32286 */
    f783B8(PANEL_DLG, S32(PLAYER_BASE));                              /* 0x32295 */
    need_screen = 0;                                                  /* 0x3229A */
    for (i = 1; i <= 9; i++) {                                        /* 0x322AC */
        if (D8(NATION(i) + 0x31) != 0) {                              /* 0x322B9 byte [199i + 0xF14FD] Architecture */
            need_screen = 1;                                          /* 0x322C3 */
            break;
        }
    }
    if ((D8(FLAGS) & PBEM_BIT) == 0)                                  /* 0x322CA */
        need_screen = 1;                                              /* 0x322D3 */
    special = 0;                                                      /* 0x322DC */
    if (S8(AREA(114) + 1) == 1) {                                     /* 0x322DF dword [0xE5070] sar 24: FAYUM */
        for (i = 0; i < 9; i++) {                                     /* 0x322F7 */
            if (S8(AREA(114) + 2u + (uint32_t)i) == 0)                /* 0x32300 dword [i + 0xE5071] sar 24: marker */
                continue;
            if (S8(AREA(114) + 0x0Fu + (uint32_t)i) < 6)              /* 0x32310 dword [i + 0xE507E] sar 24: tokens */
                continue;
            if (D8(NATION(i + 1) + 0x27) == 0)                        /* 0x32328 Agriculture */
                continue;
            special = 1;                                              /* 0x32331 */
            break;
        }
    }

    for (n = 1; n <= 9; n++) {                                        /* 0x32338, 0x32B08 */
        rec = NATION(n);                                              /* 0x32B20 */
        setD32(CUR_NATION, (uint32_t)n);                              /* 0x32B26 */
        idx = n - 1;                                                  /* 0x32B2F */
        if (D8(rec + 0x9A) == 0)                                      /* 0x32B35 */
            continue;
        if (S8(rec + 0x51) >= 9)                                      /* 0x32B48 */
            continue;
        arch = S8(rec + 0x31);                                        /* 0x32B56 dword [rec+0x2E] sar 24 */
        f334F4(arch, n);                                              /* 0x32B5F */
        if (S32(CHOICE_N) == 0)                                       /* 0x32B6A */
            continue;
        none = 0;                                                     /* 0x32B70 */
        if (need_screen == 0 && special == 0 && S8(rec + 0x51) + S32(CHOICE_N) > 9)   /* 0x32B76..0x32B8E */
            need_screen = 1;                                          /* 0x32B90 */
        if (need_screen != 0 || special != 0)                         /* 0x32B97, 0x32B9D */
            setD16(PBEM_ASK, 1);                                      /* 0x32BA3 */

        if (S8(rec + 0x9A) == 3 && S8(rec + 0x51) < 9) {              /* 0x32BB8, 0x32BC9 a human */
            if ((D8(FLAGS) & PBEM_BIT) != 0 && (need_screen != 0 || special != 0)) {   /* 0x32BD4..0x32BE0 */
                if ((D8(FLAGS) & PBEM_BIT) == 0)                      /* 0x32BE9 (never) */
                    continue;
                if (S8(LOCAL_PBEM) != n)                              /* 0x32BFA */
                    continue;
            }
            if (need_screen == 0 && special == 0) {                   /* 0x32C05, 0x32C0F */
                /* 0x32C15: every candidate at once (PBEM only) */
                for (sel = 0; sel < S32(CHOICE_N); sel++) {           /* 0x32365 */
                    c = S32(CHOICE + 4u * (uint32_t)sel);
                    setD8(TSTATE(c) + 2, (uint8_t)(D8(TSTATE(c) + 2) | 1));          /* 0x32387 */
                    setD8(TSTATE(c), (uint8_t)n);                                    /* 0x323A6 */
                    setD8(TSTATE(c) + 1, D8(ROUND));                                 /* 0x323C5 the round's low byte */
                    setD8(rec + 0x52u + (uint32_t)S8(rec + 0x51),
                          (uint8_t)(D8(CHOICE + 4u * (uint32_t)sel) + 1));           /* 0x323E3 */
                    setD8(rec + 0x51, (uint8_t)(D8(rec + 0x51) + 1));                /* 0x323E9 */
                    setD8(AREA(c + 1) + 0x0Fu + (uint32_t)idx,
                          (uint8_t)(D8(AREA(c + 1) + 0x0Fu + (uint32_t)idx)
                                    - D8(NEED + 4u * (uint32_t)sel)));               /* 0x32412 */
                    setD8(rec + 0x99, (uint8_t)(D8(rec + 0x99) + D8(NEED + 4u * (uint32_t)sel)));   /* 0x32425 */
                    setD8(rec + 0x98, (uint8_t)(D8(rec + 0x98) - D8(NEED + 4u * (uint32_t)sel)));   /* 0x32438 */
                    f581C4(c, n);                                                    /* 0x3244B */
                    snprintf(buf, sizeof buf, "%s builds a city in %s", NAME(n), TERR_NAME(c));   /* 0x32479 (0xB3A6C) */
                    if ((D8(TOGGLES) & 1) != 0)                                      /* 0x3248D */
                        f79270(-500, n, S32(CHOICE + 4u * (uint32_t)sel), buf, 0);   /* 0x32344, 0x3235A */
                    else
                        f79270(2000, n, S32(CHOICE + 4u * (uint32_t)sel), buf, 0);   /* 0x32496, 0x3235A */
                }
                continue;                                             /* 0x3236E jge 0x32B02 */
            }

            /* 0x324B1: the human's screen */
            f7853C(BUILD_DLG);                                        /* 0x324B6 */
            if (arch != 0) {                                          /* 0x324BE */
                f3C314(PANEL_DLG, 0x3B);                              /* 0x324CC */
                f3C314(PANEL_DLG, 0x3C);                              /* 0x324DB */
            } else {
                f3C3D0(PANEL_DLG, 0x3B);                              /* 0x324EC */
                f3C3D0(PANEL_DLG, 0x3C);                              /* 0x324FB */
            }
            map_screen();                                             /* 0x32505..0x32534 */
            fg_vbopen(S32(BACKBUF));                                  /* 0x32543 */
            f3AF8C(0xF51ACu, 0xFB, 0x16D);                            /* 0x3255A site arrows */
            f3AF8C(0xF51B4u, 0x178, 0x16D);                           /* 0x3256E */
            f3AF8C(0xF51ACu, 0x128, 0x181);                           /* 0x32582 treasury arrows */
            f3AF8C(0xF51B4u, 0x14A, 0x181);                           /* 0x32596 */
            fg_setcolor(5);                                           /* 0x3259D */
            f3A0B0(0xAA, 0x154, 0x12C, DSTR(0xB3A83u));               /* 0x325B9 "CANDIDATE SITES FOR CITIES" */
            fg_setcolor(f3BF30(PANEL_DLG, 0x3B) == 1 ? 5 : 0x17);     /* 0x325C8..0x325DF */
            f3A3A0(0x125, 0x185, DSTR(0xB3A9Eu));                     /* 0x325F6 "TREASURY" */
            f3A044(0x15A, 0x185, DSTR(0xB3AA7u));                     /* 0x3260A "CONTRIBUTION" */
            fg_vbclose();                                             /* 0x3260F */
            f7876C();                                                 /* 0x32614 */
            f3B05C(PANEL_DLG);                                        /* 0x32620 */
            sel = 0;                                                  /* 0x32625 */
            redraw = 1;                                               /* 0x32628 */
            contrib = 0;                                              /* 0x3262F */
            f7844C(PANEL_DLG, S32(PLAYER_BASE));                      /* 0x3263D */
            for (;;) {
                /* 0x32642 */
                if (S8(rec + 0x51) >= 9)                              /* 0x3264B */
                    break;
                if (S32(CHOICE_N) <= 0)                               /* 0x32654 */
                    break;
                fg_vbopen(S32(BACKBUF));                              /* 0x32668 */
                f3C9D0(0x109, 0x16D, 0x6E, 0x13, 5);                  /* 0x32686 the site's box */
                fg_setcolor(5);                                       /* 0x3268D */
                f3A0B0(0x109, 0x171, 0x6E, TERR_NAME(S32(CHOICE + 4u * (uint32_t)sel)));   /* 0x326B5 */
                if (arch != 0 && redraw != 0) {                       /* 0x326BD, 0x326C5 */
                    max = 0;                                          /* 0x326D1 */
                    if (SITE_BIT(S32(CHOICE + 4u * (uint32_t)sel)) == 1)   /* 0x326FC */
                        max = 3;                                      /* 0x32701 */
                    else
                        max = 6;                                      /* 0x3270A */
                    if (S8(rec + 0x3E) < max)                         /* 0x3271A treasury */
                        max = S8(rec + 0x3E);                         /* 0x32728 */
                    contrib = S32(NEED + 4u * (uint32_t)sel)
                              - TOKENS(S32(CHOICE + 4u * (uint32_t)sel) + 1, n);   /* 0x3275A */
                    if (contrib < 0)                                  /* 0x3275F */
                        contrib = 0;                                  /* 0x32767 */
                    min = contrib;                                    /* 0x3276D */
                }
                if (contrib > max)                                    /* 0x32773 */
                    contrib = max;                                    /* 0x3277B */
                f3C9D0(0x136, 0x181, 0x13, 0x13, 5);                  /* 0x32791 the contribution's box */
                fg_setcolor(f3BF30(PANEL_DLG, 0x3B) == 1 ? 5 : 0x17); /* 0x327A0..0x327B7 */
                snprintf(buf, sizeof buf, "%d", contrib);             /* 0x327D3 itoa(contrib, buf, 10) */
                f3A0B0(0x136, 0x185, 0x13, buf);                      /* 0x327E7 */
                fg_vbclose();                                         /* 0x327EC */
                f3F670();                                             /* 0x327F1 */
                f3F624(S32(CHOICE + 4u * (uint32_t)sel), 15);         /* 0x32805 */
                if (redraw != 0) {                                    /* 0x3280A */
                    f459E4(n);                                        /* 0x32813 */
                    f3DA60(S32(CHOICE + 4u * (uint32_t)sel));         /* 0x32824 */
                    redraw = 0;                                       /* 0x32829 */
                }
                f3CBBC();                                             /* 0x3282C */
                r = f78BA4(PANEL_DLG, 1);                             /* 0x3283B */
                if (r == -100)                                        /* 0x32843 */
                    continue;
                if (r >= 0) {                                         /* 0x32852 jl */
                    sel = r;                                          /* 0x32857 a map click on a listed site */
                    redraw = 1;                                       /* 0x3285A */
                    continue;
                }
                if (r == -1)                                          /* 0x32869 FINISHED */
                    break;
                switch (r) {                                          /* 0x32875 r + 13 (unsigned) <= 4: table 0x32200 */
                case -9:                                              /* 0x32889 */
                    if (sel > 0) {                                    /* 0x3288D jle */
                        sel--;                                        /* 0x32896 */
                        redraw = 1;                                   /* 0x32899 */
                    }
                    break;
                case -10:                                             /* 0x328A5 */
                    if (S32(CHOICE_N) - 1 > sel) {                    /* 0x328AE jle */
                        sel++;                                        /* 0x328B7 */
                        redraw = 1;                                   /* 0x328BA */
                    }
                    break;
                case -11:                                             /* 0x328C6 */
                    if (contrib > min)                                /* 0x328CC jle */
                        contrib--;                                    /* 0x328D5 */
                    break;
                case -12:                                             /* 0x328DD */
                    if (contrib < max)                                /* 0x328E3 jge */
                        contrib++;                                    /* 0x328EC */
                    break;
                case -13:                                             /* 0x328F4 BUILD CITY */
                    c = S32(CHOICE + 4u * (uint32_t)sel);
                    setD8(TSTATE(c) + 2, (uint8_t)(D8(TSTATE(c) + 2) | 1));          /* 0x32907 */
                    setD8(TSTATE(c), (uint8_t)n);                                    /* 0x32926 */
                    setD8(TSTATE(c) + 1, D8(ROUND));                                 /* 0x32946 */
                    setD8(rec + 0x52u + (uint32_t)S8(rec + 0x51),
                          (uint8_t)(D8(CHOICE + 4u * (uint32_t)sel) + 1));           /* 0x32964 */
                    setD8(rec + 0x51, (uint8_t)(D8(rec + 0x51) + 1));                /* 0x3296A */
                    setD8(rec + 0x3E, (uint8_t)(D8(rec + 0x3E) - (uint8_t)contrib)); /* 0x32973 treasury */
                    setD8(AREA(c + 1) + 0x0Fu + (uint32_t)idx,
                          (uint8_t)(D8(AREA(c + 1) + 0x0Fu + (uint32_t)idx)
                                    - (uint8_t)(D8(NEED + 4u * (uint32_t)sel) - (uint8_t)contrib)));   /* 0x329A7 */
                    setD8(rec + 0x99, (uint8_t)(D8(rec + 0x99) + D8(NEED + 4u * (uint32_t)sel)));     /* 0x329BA stock */
                    setD8(rec + 0x98, (uint8_t)(D8(rec + 0x98)
                                                - (uint8_t)(D8(NEED + 4u * (uint32_t)sel) - (uint8_t)contrib)));   /* 0x329D0 */
                    f581C4(S32(CHOICE + 4u * (uint32_t)sel), n);                     /* 0x329E3 */
                    f3D52C(0, 0);                                                    /* 0x329EC */
                    for (i = sel; S32(CHOICE_N) - 1 > i; i++)                        /* 0x329FF..0x32A08 */
                        setD32(CHOICE + 4u * (uint32_t)i, D32(CHOICE + 4u * (uint32_t)(i + 1)));   /* 0x32A17 */
                    setD32(CHOICE_N, (uint32_t)(S32(CHOICE_N) - 1));                 /* 0x32A20 */
                    if (contrib != 0) {                                              /* 0x32A2A Architecture used */
                        arch = 0;                                                    /* 0x32A32 */
                        fg_vbopen(S32(BACKBUF));                                     /* 0x32A3C */
                        fg_setcolor(0x17);                                           /* 0x32A46 */
                        snprintf(buf, sizeof buf, "%d", contrib);                    /* 0x32A62 itoa */
                        f3A0B0(0x136, 0x185, 0x13, buf);                             /* 0x32A76 */
                        f3A3A0(0x125, 0x185, DSTR(0xB3AB4u));                        /* 0x32A8A "TREASURY" */
                        f3A044(0x15A, 0x185, DSTR(0xB3ABDu));                        /* 0x32A9E "CONTRIBUTION" */
                        fg_vbclose();                                                /* 0x32AA3 */
                        f3C3D0(PANEL_DLG, 0x3B);                                     /* 0x32AB2 */
                        f3C3D0(PANEL_DLG, 0x3C);                                     /* 0x32AC1 */
                    }
                    f334F4(arch, n);                                                 /* 0x32ACC the list rebuilt */
                    f79A9C(&sel);                                                    /* 0x32AD9 */
                    redraw = 1;                                                      /* 0x32ADE */
                    contrib = 0;                                                     /* 0x32AE3 */
                    f459E4(n);                                                       /* 0x32AE9 */
                    f3CBBC();                                                        /* 0x32AEE */
                    break;
                default:                                              /* 0x3287B ja 0x32642 */
                    break;
                }
            }
            f78664(BUILD_DLG);                                        /* 0x32AFD */
            continue;
        }

        /* 0x32C1D: a computer */
        if (S8(rec + 0x9A) != 2)                                      /* 0x32C29 */
            continue;
        if (S8(rec + 0x51) >= 9)                                      /* 0x32C3B */
            continue;
        if (S32(CHOICE_N) == 0)                                       /* 0x32C49 */
            continue;
        none = 0;                                                     /* 0x32C53 */
        map_screen();                                                 /* 0x32C5D..0x32C88 */
        f3B05C(PANEL_DLG);                                            /* 0x32C95 */
        f459E4(n);                                                    /* 0x32C9D */
        /* 0x32CA2 start = clock(): see the wait at the end */
        waited = 0;
        f3F670();                                                     /* 0x32CAA */
        half = S8(rec + 0x98) / 2;                                    /* 0x32CB2..0x32CC4 signed, toward zero */
        if (S8(rec + 0x51) + S32(CHOICE_N) > 9) {                     /* 0x32CCD..0x32CDB jle 0x33063 */
            if (D8(rec + 0x31) != 0) {                                /* 0x32CE4 Architecture: reorder the list */
                for (i = 0; i < S32(CHOICE_N); i++) {                 /* 0x32CFC */
                    sites[i] = S32(CHOICE + 4u * (uint32_t)i);        /* 0x32D16 */
                    needs[i] = S32(NEED + 4u * (uint32_t)i);          /* 0x32D2A */
                }
                cnt = S32(CHOICE_N);                                  /* 0x32D33 */
                setD32(CHOICE_N, 0);                                  /* 0x32D38 */
                /* BUG(1995) 1: the four passes read area record s (territory s - 1) instead of s + 1; `c` is
                   the record read */
                for (i = 0; i < cnt; i++) {                           /* 0x32D49 pass 1: tokens enough */
                    c = g_original ? sites[i] : sites[i] + 1;         /* 0x32D5B..0x32D67 69s */
                    if (TOKENS(c, n) < needs[i])                      /* 0x32D6A..0x32D7F */
                        continue;
                    j = S32(CHOICE_N);                                /* 0x32D84 */
                    setD32(CHOICE + 4u * (uint32_t)j, (uint32_t)sites[i]);   /* 0x32D90 */
                    setD32(NEED + 4u * (uint32_t)j, (uint32_t)needs[i]);     /* 0x32DA4 */
                    setD32(CHOICE_N, (uint32_t)(j + 1));              /* 0x32DAB */
                }
                for (i = 0; i < cnt; i++) {                           /* 0x32DC0 pass 2: short, state bit 0x02 */
                    c = g_original ? sites[i] : sites[i] + 1;         /* 0x32DD6..0x32DE2 */
                    if (TOKENS(c, n) >= needs[i])                     /* 0x32DE5..0x32DFA */
                        continue;
                    if ((D8(TSTATE(sites[i]) + 2) & 2) == 0)          /* 0x32E0F */
                        continue;
                    j = S32(CHOICE_N);                                /* 0x32E1B */
                    setD32(CHOICE + 4u * (uint32_t)j, (uint32_t)sites[i]);   /* 0x32E27 */
                    setD32(NEED + 4u * (uint32_t)j, (uint32_t)needs[i]);     /* 0x32E3B */
                    setD32(CHOICE_N, (uint32_t)(j + 1));              /* 0x32E42 */
                }
                for (i = 0; i < cnt; i++) {                           /* 0x32E5A pass 3: short, AI flag 0x80 clear */
                    c = g_original ? sites[i] : sites[i] + 1;         /* 0x32E70..0x32E7C */
                    if (TOKENS(c, n) >= needs[i])                     /* 0x32E7F..0x32E94 */
                        continue;
                    if ((D8(TSTATE(sites[i]) + 2) & 2) != 0)          /* 0x32EA9 */
                        continue;
                    if ((D8(AI_FLAGS + 0x13Au * (uint32_t)idx + 2u * (uint32_t)sites[i]) & 0x80) != 0)   /* 0x32EC3 */
                        continue;
                    j = S32(CHOICE_N);                                /* 0x32ED0 */
                    setD32(CHOICE + 4u * (uint32_t)j, (uint32_t)sites[i]);   /* 0x32EDC */
                    setD32(NEED + 4u * (uint32_t)j, (uint32_t)needs[i]);     /* 0x32EF0 */
                    setD32(CHOICE_N, (uint32_t)(j + 1));              /* 0x32EF7 */
                }
                for (i = 0; i < cnt; i++) {                           /* 0x32F0F pass 4: the rest */
                    c = g_original ? sites[i] : sites[i] + 1;         /* 0x32F25..0x32F31 */
                    if (TOKENS(c, n) >= needs[i])                     /* 0x32F34..0x32F49 */
                        continue;
                    if ((D8(TSTATE(sites[i]) + 2) & 2) != 0)          /* 0x32F5E */
                        continue;
                    if ((D8(AI_FLAGS + 0x13Au * (uint32_t)idx + 2u * (uint32_t)sites[i]) & 0x80) == 0)   /* 0x32F78 */
                        continue;
                    j = S32(CHOICE_N);                                /* 0x32F85 */
                    setD32(CHOICE + 4u * (uint32_t)j, (uint32_t)sites[i]);   /* 0x32F91 */
                    setD32(NEED + 4u * (uint32_t)j, (uint32_t)needs[i]);     /* 0x32FA3 [4*(j+1) + 0xF7F1C] */
                    setD32(CHOICE_N, (uint32_t)(j + 1));              /* 0x32FAA */
                }
            }
            /* 0x32FB4: bubble sort by need, ascending (stable: strict >) */
            for (i = 1; i < S32(CHOICE_N); i++) {                     /* 0x32FC3 */
                for (j = S32(CHOICE_N) - 1; j >= i; j--) {            /* 0x32FD4, 0x32FE2 */
                    if (S32(NEED + 4u * (uint32_t)(j - 1)) <= S32(NEED + 4u * (uint32_t)j))   /* 0x32FFE jle */
                        continue;
                    tmp = S32(CHOICE + 4u * (uint32_t)(j - 1));       /* 0x33005 [4j + 0xF7C9C] */
                    setD32(CHOICE + 4u * (uint32_t)(j - 1), D32(CHOICE + 4u * (uint32_t)j));   /* 0x3301C */
                    setD32(CHOICE + 4u * (uint32_t)j, (uint32_t)tmp); /* 0x33029 */
                    tmp = S32(NEED + 4u * (uint32_t)(j - 1));         /* 0x33033 [4j + 0xF7F1C] */
                    setD32(NEED + 4u * (uint32_t)(j - 1), D32(NEED + 4u * (uint32_t)j));       /* 0x3304A */
                    setD32(NEED + 4u * (uint32_t)j, (uint32_t)tmp);   /* 0x33057 */
                }
            }
        }
        /* 0x33063: every site it has the tokens for */
        for (i = 0; i < S32(CHOICE_N); i++) {                         /* 0x330D1 */
            c = S32(CHOICE + 4u * (uint32_t)i);                       /* 0x330E3 */
            if (c == -1)                                              /* 0x330EA */
                continue;
            if (S8(rec + 0x51) >= 9)                                  /* 0x330F8 */
                continue;
            if (TOKENS(c + 1, n) < S32(NEED + 4u * (uint32_t)i))      /* 0x3311F..0x3312E */
                continue;
            f3F670();                                                 /* 0x33130 */
            f3F624(c, 15);                                            /* 0x33144 */
            f3DA60(S32(CHOICE + 4u * (uint32_t)i));                   /* 0x33153 */
            f3CBBC();                                                 /* 0x33158 */
            f78A20(300);                                              /* 0x33162 */
            waited = 1;
            if (TOKENS(S32(CHOICE + 4u * (uint32_t)i) + 1, n) <= S32(NEED + 4u * (uint32_t)i))   /* 0x3319E jle */
                k = TOKENS(S32(CHOICE + 4u * (uint32_t)i) + 1, n);    /* 0x3306A..0x33088 */
            else
                k = S32(NEED + 4u * (uint32_t)i);                     /* 0x331A4 */
            setD8(rec + 0x98, (uint8_t)(D8(rec + 0x98) - (uint8_t)k));   /* 0x33094 */
            setD8(rec + 0x99, (uint8_t)(D8(rec + 0x99) + (uint8_t)k));   /* 0x330A0 */
            f333E8(S32(CHOICE + 4u * (uint32_t)i), n, k, 0);          /* 0x330B8 [M7] */
            setD32(CHOICE + 4u * (uint32_t)i, 0xFFFFFFFFu);           /* 0x330C0 */
        }
        /* 0x331AF: with Architecture one more site, paid partly from the treasury */
        if (D8(rec + 0x31) != 0) {                                    /* 0x331B2 */
            for (i = 0; i < S32(CHOICE_N); i++) {                     /* 0x3325F */
                if (S8(rec + 0x51) >= half)                           /* 0x33277 jge 0x3332E */
                    break;
                if (S8(rec + 0x51) >= 9)                              /* 0x33280 */
                    continue;
                c = S32(CHOICE + 4u * (uint32_t)i);   /* -1 once built: area record 0 is read (the file comment) */
                if (TOKENS(c + 1, n) + S8(rec + 0x3E) < S32(NEED + 4u * (uint32_t)i))   /* 0x332A5..0x332BF */
                    continue;
                if (S8(AREA(c + 1) + 1) != 1)                         /* 0x332D5 dword [.. + 0xE31B6] sar 24 */
                    continue;
                if (TOKENS(c + 1, n) <= S32(NEED + 4u * (uint32_t)i)) /* 0x3331D jle */
                    k = TOKENS(c + 1, n);                             /* 0x331C6..0x331E4 */
                else
                    k = S32(NEED + 4u * (uint32_t)i);                 /* 0x33323 */
                d = S32(NEED + 4u * (uint32_t)i) - k;                 /* 0x331F9 */
                if (arch == 0 || d == 0)                              /* 0x331FC, 0x33202 */
                    continue;
                setD8(rec + 0x98, (uint8_t)(D8(rec + 0x98) - (uint8_t)k));               /* 0x3320E */
                setD8(rec + 0x99, (uint8_t)(D8(rec + 0x99) + (uint8_t)((uint8_t)k + (uint8_t)d)));   /* 0x3321D */
                setD8(rec + 0x3E, (uint8_t)(D8(rec + 0x3E) - (uint8_t)d));               /* 0x33229 treasury */
                f333E8(S32(CHOICE + 4u * (uint32_t)i), n, k, 0x63);   /* 0x33241 [M7] */
                waited = 1;
                setD32(CHOICE + 4u * (uint32_t)i, 0xFFFFFFFFu);       /* 0x33249 */
                arch = 0;                                             /* 0x33256 */
            }
        }
        /* 0x3332E: the wait until 2 s after the start (port: the file comment) */
        if ((D8(TOGGLES) & 1) != 0)                                   /* 0x3332E jne 0x32B02 */
            continue;
        if (f7BA64() == 0)                                            /* 0x3333B */
            ms = 2000;                                                /* 0x33344..0x3336E */
        else
            ms = S16(SRD) == 0 ? 500 : 1000;                          /* 0x33375..0x333B0 */
        if (!waited)
            f78A20(ms);
    }
    /* 0x333B7 */
    if (none != 0)
        f79270(100, S16(PLAYER_LIST), -200, DSTR(0xB3ACAu), 0);       /* 0x333D7 dword [0xF06AE] sar 16; "" */
}

/* ---- 0x5C238 REMOVE SURPLUS -------------------------------------------------------------------------------------- */

void f5C238(int show)
{
    int32_t nothing;   /* [ebp-0x18] */
    int32_t n;         /* [ebp-0x28] */
    int32_t idx;       /* [ebp-0x24] n - 1 */
    int32_t agri;      /* [ebp-0x2C] */
    int32_t t;         /* [ebp-0x20] */
    int32_t k;         /* [ebp-0x1C] */
    int32_t lim;

    nothing = 1;                                                      /* 0x5C250 */
    if (show != 0) {                                                  /* 0x5C257 */
        memset(DP(SURPLUS), 0, 0xC);                                  /* 0x5C26D */
        player_list();                                                /* 0x5C272..0x5C2BD */
        f783B8(PANEL_DLG, S32(PLAYER_BASE));                          /* 0x5C2CA */
        map_screen();                                                 /* 0x5C2D4..0x5C304 */
        f7876C();                                                     /* 0x5C30C */
        f3C720(PANEL_DLG);                                            /* 0x5C316 every button disabled */
    }
    for (n = 1; n <= 9; n++) {                                        /* 0x5C32A */
        idx = n - 1;                                                  /* 0x5C338 */
        agri = D8(NATION(n) + 0x27) != 0;                             /* 0x5C342 setne */
        (void)idx;
        for (t = 0; t < 0x9D; t++) {                                  /* 0x5C3E8 */
            if (show == 0 && (D8(TSTATE(t) + 2) & 2) != 0) {          /* 0x5C3F8, 0x5C40C an AI-marked site */
                k = TOKENS(t + 1, n);                                 /* 0x5C42C */
                f5C528(k, t, n, show);                                /* 0x5C444 (also k == 0) */
                continue;
            }
            /* 0x5C35F */
            if ((D8(TSTATE(t) + 2) & 1) == 1) {                       /* 0x5C38A a city */
                if (TOKENS(t + 1, n) > 0) {                           /* 0x5C39C jle */
                    k = TOKENS(t + 1, n);                             /* 0x5C3BD */
                    setD8(SURPLUS + (uint32_t)n, (uint8_t)(D8(SURPLUS + (uint32_t)n) + (uint8_t)k));   /* 0x5C3C6 */
                    f5C528(k, t, n, show);                            /* 0x5C3DA */
                    nothing = 0;                                      /* 0x5C3DF */
                }
                continue;
            }
            /* 0x5C44B */
            lim = POP_LIMIT(t) + agri;                                /* 0x5C46B..0x5C476 */
            if (TOKENS(t + 1, n) > lim) {                             /* 0x5C479 jle */
                k = TOKENS(t + 1, n) - (POP_LIMIT(t) + agri);         /* 0x5C4C2 */
                f5C528(k, t, n, show);                                /* 0x5C4D5 */
                nothing = 0;                                          /* 0x5C4DA */
            }
        }
    }
    if (show != 0)                                                    /* 0x5C4E2 */
        f3C6A0(PANEL_DLG);                                            /* 0x5C4ED every button enabled */
    if (nothing != 0 && show != 0)                                    /* 0x5C4F2, 0x5C4F8 */
        f79270(100, S16(PLAYER_LIST), -200, DSTR(0xB5D54u), 0);       /* 0x5C518 "" */
}

/* ---- 0x5C528 k tokens removed ------------------------------------------------------------------------------------ */

void f5C528(int k, int t, int n, int show)
{
    char buf[100];   /* [ebp-0x7C] */

    setD8(AREA(t + 1) + 0x0Eu + (uint32_t)n,
          (uint8_t)(D8(AREA(t + 1) + 0x0Eu + (uint32_t)n) - (uint8_t)k));   /* 0x5C55E */
    f581C4(t, n);                                                     /* 0x5C56A */
    setD8(NATION(n) + 0x98, (uint8_t)(D8(NATION(n) + 0x98) - (uint8_t)k));  /* 0x5C579 board */
    setD8(NATION(n) + 0x99, (uint8_t)(D8(NATION(n) + 0x99) + (uint8_t)k));  /* 0x5C589 stock */
    if (show == 0)                                                    /* 0x5C58F */
        return;
    snprintf(buf, sizeof buf, "%s - %d token%s removed from %s", NAME(n), k,
             k > 1 ? DSTR(0xB5D55u) : DSTR(0xB5D57u), TERR_NAME(t));  /* 0x5C5D1 (0xB5D58; "s" / "") */
    if (f7BA64() == 0)                                                /* 0x5C5D9 */
        f79270(1000, n, t, buf, 0);                                   /* 0x5C5E2..0x5C620 */
    else if (S16(SRD) == 0)                                           /* 0x5C605 */
        f79270(500, n, t, buf, 0);                                    /* 0x5C60F */
    else
        f79270(1500, n, t, buf, 0);                                   /* 0x5C61B, 0x5C620 */
}

/* ---- 0x5C62C CITY SUPPORT ---------------------------------------------------------------------------------------- */

void f5C62C(void)
{
    char    buf[100];   /* [ebp-0x80] */
    int32_t n;          /* [ebp-0x1C] */
    int     loss;

    player_list();                                                    /* 0x5C641..0x5C68B */
    f783B8(PANEL_DLG, S32(PLAYER_BASE));                              /* 0x5C69D */
    for (n = 1; n <= 9; n++) {                                        /* 0x5C6EA */
        setD32(CUR_NATION, (uint32_t)n);                              /* 0x5C6F9 */
        if (S16(0xF7AFEu) == n)                                       /* 0x5C70A dword [0xF7AFC] sar 16 */
            snprintf(buf, sizeof buf, "City Support Check with Slave revolt losses for %s.", NAME(n));   /* 0x5C6B7 (0xB5D93) */
        else
            snprintf(buf, sizeof buf, "City Support Check for %s.", NAME(n));   /* 0x5C6B7 (0xB5D78) */
        if (S16(0xF7AFEu) == n)                                       /* 0x5C6CA */
            loss = S16(0xF7AFCu);                                     /* 0x5C6CF dword [0xF7AFA] sar 16 */
        else
            loss = 0;                                                 /* 0x5C6DA */
        f5C750(n, loss, buf);                                         /* 0x5C6DF */
    }
    if (S16(REDUCED) == 0)                                            /* 0x5C71C dword [0x109E1E] sar 16 */
        f79270(100, S16(PLAYER_LIST), -200, DSTR(0xB5DC7u), 0);       /* 0x5C741 "" */
}

/* ---- 0x5C750 the support check of one nation --------------------------------------------------------------------- */

void f5C750(int n, int loss, const char *msg)
{
    char     buf[152];   /* [ebp-0xB0] */
    int32_t  self;       /* [ebp-0x18]: the argument n, whose address is the one-entry list for f5C880 */
    uint32_t rec;        /* [ebp-0x8] */
    int      saved;      /* [ebp-0xC] */

    self = n;
    rec = NATION(n);                                                  /* 0x5C77B */
    if (D8(rec + 0x9A) == 0)                                          /* 0x5C781 */
        return;
    for (;;) {
        /* 0x5C78E */
        if (S8(rec + 0x98) - loss >= 2 * S8(rec + 0x51))              /* 0x5C791..0x5C7AA jge: supported */
            return;
        if (!g_original && S8(rec + 0x51) <= 0)   /* BUG(1995) 2: no city left, the loop could never end */
            return;
        setD16(REDUCED, 1);                                           /* 0x5C7B0 */
        if (((D8(FLAGS) & PBEM_BIT) == 0 && S8(rec + 0x9A) == 3)      /* 0x5C7BF, 0x5C7D0 */
            || ((D8(FLAGS) & PBEM_BIT) != 0 && D8(PHASE) != 0xF)) {   /* 0x5C7D5, 0x5C7DE */
            if ((D8(FLAGS) & PBEM_BIT) != 0 && S8(LOCAL_PBEM) != n)   /* 0x5C7ED..0x5C806 (PBEM) */
                return;
            f7844C(PANEL_DLG, S32(PLAYER_BASE));                      /* 0x5C813 */
            snprintf(buf, sizeof buf, "%s You must reduce your cities.", msg);   /* 0x5C833 (0xB5DC8) */
            f5C880(&self, 1, buf, 1);                                 /* 0x5C840 */
        } else {
            saved = loss;                                             /* 0x5C84D */
            setD8(rec + 0x98, (uint8_t)(D8(rec + 0x98) - (uint8_t)loss));    /* 0x5C856 */
            f5DE5C(n, msg);                                           /* 0x5C862 [M7] */
            setD8(rec + 0x98, (uint8_t)(D8(rec + 0x98) + (uint8_t)saved));   /* 0x5C86D */
        }
    }
}

/* ---- 0x5C880 REDUCE CITY ----------------------------------------------------------------------------------------- */

int f5C880(const int32_t *list, int cnt, const char *msg, int mode)
{
    char    buf[200];   /* [ebp-0xFC] */
    int32_t i;          /* [ebp-0x28] */
    int32_t t;          /* [ebp-0x24] */
    int32_t sel;        /* [ebp-0x20] (its address goes to f79A9C) */
    int32_t loop;       /* [ebp-0x1C] */
    int32_t redraw;     /* [ebp-0x14] */
    int32_t owner;      /* [ebp-0x10] */
    int32_t r;          /* [ebp-0x18] */
    int32_t c;
    int     empty;      /* port: BUG(1995) fix 3 (the file comment) */

    setD32(CHOICE_N, 0);                                              /* 0x5C8A3 */
    for (i = 0; i < 0x9D; i++)                                        /* 0x5C8B4 */
        setD32(CHOICE + 4u * (uint32_t)i, 0xFFFFFFFFu);               /* 0x5C8C0 */
    if (mode == 1) {                                                  /* 0x5C8CD */
        for (t = 0; t < 0x9D; t++)                                    /* 0x5C8E4 */
            for (i = 0; i < cnt; i++) {                               /* 0x5C8FE */
                if ((int32_t)D8(TSTATE(t)) != list[i])                /* 0x5C926 */
                    continue;
                if ((D8(TSTATE(t) + 2) & 1) != 1)                     /* 0x5C946 */
                    continue;
                if ((int32_t)D8(TSTATE(t) + 1) != S16(ROUND))         /* 0x5C968 dword [0xF40FA] sar 16 */
                    continue;
                c = S32(CHOICE_N);                                    /* 0x5C96C */
                setD32(CHOICE_N, (uint32_t)(c + 1));                  /* 0x5C979 */
                setD32(CHOICE + 4u * (uint32_t)c, (uint32_t)t);       /* 0x5C981 */
            }
    }
    if (S32(CHOICE_N) != 0)                                           /* 0x5C98C */
        snprintf(buf, sizeof buf, "%s First using cities built this round.", msg);   /* 0x5C9A5 (0xB5DE8) */
    else
        snprintf(buf, sizeof buf, "%s", msg);                         /* 0x5C9AF..0x5C9D1 inline strcpy */
    if (S32(CHOICE_N) == 0) {                                         /* 0x5C9DA */
        for (t = 0; t < 0x9D; t++)                                    /* 0x5C9EB */
            for (i = 0; i < cnt; i++) {                               /* 0x5CA05 */
                if ((int32_t)D8(TSTATE(t)) != list[i])                /* 0x5CA2C */
                    continue;
                if ((D8(TSTATE(t) + 2) & 1) != 1)                     /* 0x5CA49 */
                    continue;
                c = S32(CHOICE_N);                                    /* 0x5CA4E */
                setD32(CHOICE_N, (uint32_t)(c + 1));                  /* 0x5CA5B */
                setD32(CHOICE + 4u * (uint32_t)c, (uint32_t)t);       /* 0x5CA63 */
            }
    }
    f7853C(REDUCE_DLG);                                               /* 0x5CA70 */
    map_screen();                                                     /* 0x5CA7A..0x5CAAA */
    fg_vbopen(S32(BACKBUF));                                          /* 0x5CAB8 */
    f3AF8C(0xF51ACu, 0xFB, 0x181);                                    /* 0x5CACF */
    f3AF8C(0xF51B4u, 0x178, 0x181);                                   /* 0x5CAE3 */
    fg_setcolor(5);                                                   /* 0x5CAEA */
    fg_vbclose();                                                     /* 0x5CAF2 */
    f3A924();                                                         /* 0x5CAF7 */
    f3A9F4(buf);                                                      /* 0x5CB02 */
    f7876C();                                                         /* 0x5CB07 */
    f3B05C(PANEL_DLG);                                                /* 0x5CB13 */
    sel = 0;                                                          /* 0x5CB18 */
    redraw = 1;                                                       /* 0x5CB20 */
    loop = 1;                                                         /* 0x5CB23 */
    owner = 0;
    while (loop != 0) {                                               /* 0x5CB26 */
        empty = !g_original && S32(CHOICE_N) <= 0;   /* BUG(1995) 3: an empty list reads entry -1 */
        fg_vbopen(S32(BACKBUF));                                      /* 0x5CB37 */
        f3C9D0(0x109, 0x181, 0x6E, 0x13, 5);                          /* 0x5CB55 */
        fg_setcolor(5);                                               /* 0x5CB5C */
        if (!empty)
            f3A0B0(0x109, 0x185, 0x6E, TERR_NAME(S32(CHOICE + 4u * (uint32_t)sel)));   /* 0x5CB84 */
        fg_vbclose();                                                 /* 0x5CB89 */
        owner = empty ? 0 : (int32_t)D8(TSTATE(S32(CHOICE + 4u * (uint32_t)sel)));     /* 0x5CBA5 */
        f3F670();                                                     /* 0x5CBAE */
        if (!empty)
            f3F624(S32(CHOICE + 4u * (uint32_t)sel), 15);             /* 0x5CBC2 */
        if (redraw != 0) {                                            /* 0x5CBCA */
            f459E4(owner);                                            /* 0x5CBD1 */
            if (!empty)
                f3DA60(S32(CHOICE + 4u * (uint32_t)sel));             /* 0x5CBE2 */
            else
                f3D52C(0, 0);
            redraw = 0;                                               /* 0x5CBE7 */
        }
        f3CBBC();                                                     /* 0x5CBEA */
        r = f78BA4(PANEL_DLG, 1);                                     /* 0x5CBF9 */
        if (r == -100)                                                /* 0x5CC01 */
            continue;
        if (r >= 0) {                                                 /* 0x5CC10 jl */
            sel = r;                                                  /* 0x5CC15 a map click on a listed city */
            redraw = 1;                                               /* 0x5CC18 */
            continue;
        }
        if (r == -1)                                                  /* 0x5CC27 */
            break;
        if ((uint32_t)r < (uint32_t)-0xA) {                           /* 0x5CC36 jb 0x5CD11 */
            if (r == -0xE && empty) {
                /* BUG(1995) fix 3: nothing to reduce; REDUCE CITY only leaves. It must leave: dialog 0xC5DD8 has no
                   FINISHED (0x2A), and no panel button of 0xF596C returns -1 here, so REDUCE is the only way out */
                loop = 0;
            } else if (r == -0xE) {                                   /* 0x5CD11 REDUCE CITY */
                f78FEC(S32(CHOICE + 4u * (uint32_t)sel), owner);      /* 0x5CC91 */
                f3D52C(0, 0);                                         /* 0x5CC9A */
                for (i = sel; S32(CHOICE_N) - 1 > i; i++)             /* 0x5CCAD..0x5CCB6 */
                    setD32(CHOICE + 4u * (uint32_t)i, D32(CHOICE + 4u * (uint32_t)(i + 1)));   /* 0x5CCC5 */
                setD32(CHOICE_N, (uint32_t)(S32(CHOICE_N) - 1));      /* 0x5CCDD */
                f79A9C(&sel);                                         /* 0x5CCE3 */
                redraw = 1;                                           /* 0x5CCE8 */
                fg_vbopen(S32(BACKBUF));                              /* 0x5CCF2 (no fg_vbclose follows) */
                f459E4(owner);                                        /* 0x5CCFD */
                f3CBBC();                                             /* 0x5CD04 */
                loop = 0;                                             /* 0x5CD09 */
            }
        } else if (r == -0xA) {                                       /* 0x5CC3C jbe 0x5CC63 */
            if (S32(CHOICE_N) - 1 > sel) {                            /* 0x5CC6C jle */
                sel++;                                                /* 0x5CC75 */
                redraw = 1;                                           /* 0x5CC78 */
            }
        } else if (r == -9) {                                         /* 0x5CC3E */
            if (sel > 0) {                                            /* 0x5CC4B jle */
                sel--;                                                /* 0x5CC54 */
                redraw = 1;                                           /* 0x5CC57 */
            }
        }
    }
    f78664(REDUCE_DLG);                                               /* 0x5CD24 */
    return owner;                                                     /* 0x5CD29 */
}
