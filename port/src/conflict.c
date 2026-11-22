/* Module CONFLICT of docs/M6_PLAN.md: the CONFLICT phase and the shared conflict rules. Literal translations; the
   address of each instruction a line comes from is in its comment. D = read in the code, G = inference. Prototypes,
   records and globals: conflict.h.

     0x5AAA4  f5AAA4  CONFLICT (phase 7; also the AI's trials with show = 0)       1,920 bytes
     0x5B224  f5B224  the attack on a city                                           868
     0x5B588  f5B588  the token battle                                             1,156
     0x5BA0C  f5BA0C  the plunder after a city is taken                            1,316
     0x5C110  f5C110  the conflict matrix row                                        188
     0x5C1CC  f5C1CC  the pre-movement owner of a territory                          108
     0x5CDE8  f5CDE8  the losses box ("CONFLICT RESOLUTION - UNITS REMOVED")         868

   Not ported here (each call site stops the port with not_ported(address); the static stubs carry the names, so a
   later translation that exports one of them collides with its stub at compile time instead of being shadowed):
     0x5BF30  the computer's plunder transfer (M7, AI; its only caller is 0x5BECB, for a computer attacker)
     0x7A4C4  the calamity animation (module ANIM: f7A0A0([0xF783C]) when bit 0x04 of [0xF097C] is set; called by
              0x5CDE8 only with anim != 0, which 0x5AAA4 never passes)
   So in an all-human game the whole phase runs; with computers it stops when a computer takes a city with tokens and
   stock left (0x5BF30). With -DCONFLICT_TEST (port/test/conflict_test.c) both go to the test's recorders, which return.

   game_rand (f78AE4): one call, in 0x5BA0C (0x5BB02), only with show, a city owner o <= 9 and at least one commodity
   card in o's hand (D). 0x5B224 and the rest of the module draw nothing (D: no call of 0x78AE4 in 0x5AAA4..0x5CDE8
   other than 0x5BB02).

   Play by e-mail (dropped by Erik's ruling): no PBEM routine is called here. The bit-0x80 tests of byte [0xE1624] are
   kept as data tests: 0x5AAA4's first nation (the local player int8 [0xE1622], read as dword [0xE161F] sar 24),
   0x5BA0C's human test (the local player gets the transfer screen), and 0x5B588's PBEM-only rule (Military nations
   cost the later non-Military ones a token each), translated with its BUG(1995) below.

   Watcom runtime -> standard C:
     0x7DB3D sprintf -> snprintf bounded to the stack buffer: 200 bytes in 0x5BA0C ([ebp-0xFC]..[ebp-0x34]), 20 in
             0x5CDE8 ([ebp-0x4C]..[ebp-0x38]); the formats are written out (their addresses in the comments)
     0x8C897 itoa(v, buf, 10) -> snprintf "%d"
     0x7D5D0 memset(eax, edx, ebx) -> memset on the data object
     repne scasb (strlen) -> strlen
   __CHK dropped.

   1995 bugs, fixed unless g_original (each marked BUG(1995) in the code; grades code / effect):
     1. 0x5C110 (D / G): the city owner (a nation number) is compared with the 0-based row index i (0x5C179), so a city
        whose owner has no tokens there marks the row of nation owner + 1, not the owner's: the owner gets no hostility
        towards its attackers, and nation owner + 1 (if not present itself) gets hostility towards every nation there.
        Fixed: owner == i + 1. The matrix 0x109E24 and the hostility bytes are in the save, so the fix changes the
        autosave after CONFLICT whenever a city with no defenders is attacked (byte compares: run with --original).
     2. 0x5CDE8 (D / D): the short-name table 0xC7664 has 10 entries (nations 0..9); for nations 10..13 whose full name
        is wider than the column the code reads the next table, the full names: the Pirates show "" (PIRATE ->
        names[0]), the Barbarians "AFRICA", the factions "IBERIA" / "ILLYRIA". Fixed: their full name is shown.
     3. 0x5B588, PBEM only (D / G): the Military rule tests and decrements pair[count], one past the last pair
        (uninitialised stack in the original), instead of pair[j], the nation that loses the token. Fixed: pair[j]. With
        g_original the port reads its zeroed entry past the list (the original's stack garbage cannot be reproduced),
        so the rule then only sets the compaction flag.
   Guards in both modes (the original's behaviour cannot be reproduced; D code, G unreachable in play):
     - 0x5AAA4's list of contested records is 60 ints ([ebp-0x144]); a 61st entry would overwrite its own locals (the
       nation loop counter, then show). The port's list has room for all 157 records.
     - 0x5B224's attacker [ebp-0x10] is uninitialised when the nation count +1 is not 0 but no marker +2..+12 is
       positive: only the factions 12 / 13 there (markers +0x0D / +0x0E), or inconsistent records. The port then
       clears the pending plunder and returns: no attack. (Starting it at 0 would not be a no-op: the "tokens of
       nation 0", byte +0x0E, are the marker of nation 13, 13 >= every threshold, so "nation 0" would take the city.)
     - 0x5CDE8 divides by the number of nations with losses (idiv, 0x5CE6A): 0 faults in the original (a calamity or a
       conflict where only nations not in play lose). The port uses column width 0 then (nothing is drawn: the same
       loop finds no column).
   Kept as the original:
     - 0x5B224: a city of the Pirates (owner 10) is replaced by 7 / 5 / 6 tokens (by Engineering) without the stock
       limit, so the Pirates' int8 stock +0x99 goes negative by that much each time (D; the stock is never used, G).
     - The hidden nations 10..13 are "in play" (+0x9A = 3) in every save at hand (D, conflict_test part C), so the
       losses box gives the Pirates a column when they lose tokens, and its division by zero above cannot happen in a
       conflict (any loss is counted).
     - 0x5BA0C: the plunder draw includes the owner's calamity cards (+0x14..), but only when it holds at least one
       commodity card (D); the card list 0x112584 is 32 bytes before its count 0x1125A4, written byte by byte through
       the data object, so even an over-long hand behaves as in the original.
     - 0x5B588's groups are formed by the tokens at the start (+50 with Metalworking), never updated: each pass round
       the groups takes one token from every member that still has one (D).
   Text (Erik's ruling of 19 Nov, obvious errors only, --original keeps them):
     - "%s,you have no stock to pillage with." (0xB5CDE) -> "%s, you have no stock to pillage with." (missing space).
     - For M7 (0x5BF30, not ported): "%s, choosing number units to transfer from stock to treasury." (0xB5D16) lacks
       "of"; a missing word is not on the ruling's list: for Erik.
   Easter eggs / oddities: the empty "no conflicts" message 0xB5C52 (a 100 ms home view of the first player, M6_PLAN
   egg 15); the plunder can take a calamity card (it moves with the owner as its giver, f75D84); the losses box has two
   hidden exits -0x50 / -0x51 that only the Civil War's faction buttons use (0x2D3E0's dialog 0xC0214). */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "anim.h"
#include "conflict.h"
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
#define SHORT(n)     DSTR(D32(0xC7664u + 4u * (uint32_t)(n)))   /* 10 entries (0..9); n 10..13 read 0xC768C[n - 10] */
#define AREA_REC(r)  (0xE31B8u + 69u * (uint32_t)(r))           /* area record r = territory r - 1 */
#define PREV_AREA(r) (0xE8888u + 69u * (uint32_t)(r))           /* the copy before the movement */
#define TS(t)        (0xE2C18u + 3u * (uint32_t)(t))            /* territory state t (t = -1 reads 0xE2C15..) */
#define PREV_TS(t)   (0xE2FD8u + 3u * (uint32_t)(t))
#define TERR(t)      (0xE1638u + 35u * (uint32_t)(t))
#define POPLIMIT(t)  ((((unsigned)D8(TERR(t) + 1u) << 2) & 0xFFu) >> 5)   /* shl al, 2; shr al, 5 */
#define LOSS_TOK     0xF547Cu   /* uint8 [14] */
#define LOSS_CITY    0xF548Cu   /* uint8 [14] */
#define MATRIX       0x109E24u  /* uint8 [9][9] */
#define PL_PENDING   0x109E08u
#define PL_ATT       0x109E09u  /* read as dword [0x109E06] sar 24 */
#define PL_OWN       0x109E0Au  /* read as dword [0x109E07] sar 24 */
#define PL_REC       0x109E0Cu  /* int16, read as dword [0x109E0A] sar 16 */
#define CARD_LIST    0x112584u
#define CARD_N       0x1125A4u  /* int32 */
#define REAL_RES     0xBB3F8u   /* uint8 */
#define FLAGS        0xE1624u
#define PBEM_BIT     0x80u
#define HUMANS       0xE1620u   /* int8, read as dword [0xE161D] sar 24 */
#define PBEM_LOCAL   0xE1622u   /* int8 */
#define CUR_NATION   0xF40F8u   /* int32 */
#define TURN         0xF40FCu   /* the round (int16; 0x5B224 stores its low byte) */
#define PANEL        0xF596Cu
#define PLAYER_BASE  0xF5940u   /* int32 */
#define PLAYERS      0xF06AEu   /* int16 count */
#define PLAYER_LIST  0xF06B0u   /* int16 [] */
#define BACKBUF      0xF52C8u
#define BACKGROUND   0xF52CCu
#define BGNUM        0xEE168u
#define CONFLICT_DLG 0xC5214u
#define PLUNDER_DLG  0xC61C4u

/* ---- calls into code that is not ported yet (see the file comment) ----------------------------------------------- */

#ifdef CONFLICT_TEST
void conflict_test_f5BF30(int n, int amount, int show);
void conflict_test_f7A4C4(int anim);
#define f5BF30(n, amount, show) conflict_test_f5BF30(n, amount, show)
#define f7A4C4(anim)            conflict_test_f7A4C4(anim)
#else
#include "aiship.h"                             /* f5BF30: a computer's plunder transfer (M7) */

#endif

/* the background 3 into the back buffer: f3CD6C(3), byte [0xEE168] = 3 (stored after the call), fg_vbcopy of the
   whole screen from [0xF52CC] to [0xF52C8] (0x5ADB5..0x5ADEE, 0x5BB9E..0x5BBD8) */
static void background3(void)
{
    f3CD6C(3);
    setD8(BGNUM, 3);
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BACKGROUND), S32(BACKBUF));
}

/* ---- 0x5AAA4 CONFLICT -------------------------------------------------------------------------------------------- */

void f5AAA4(int show)
{
    int32_t  list[157];   /* [ebp-0x144], 60 ints in the original (the file comment) */
    int      count;       /* [ebp-0x30] */
    int      any;         /* [ebp-0x20] */
    int      nothing;     /* [ebp-0x1C] */
    int      i;           /* [ebp-0x54] */
    int      j;           /* [ebp-0x4C] */
    int      r;           /* [ebp-0x48] */
    int      limit;       /* [ebp-0x28] */
    int      sum;         /* [ebp-0x34] */
    int      pass;        /* [ebp-0x2C] */
    int      k;           /* [ebp-0x38] */
    int      nn;
    int      city;        /* [ebp-0x24] */
    int      a, b;        /* [ebp-0x44], [ebp-0x40] */
    int      o;           /* [ebp-0x44] (second use) */
    int32_t  saved;       /* [ebp-0x3C] */
    uint32_t rec;         /* [ebp-0x18] */

    count = 0;                                                        /* 0x5AAC1 */
    any = 0;                                                          /* 0x5AAC4 */
    nothing = 1;                                                      /* 0x5AAC7 */
    if (show != 0) {                                                  /* 0x5AACE */
        setD16(0xBB3FAu, 0xA9);                                       /* 0x5AAEE the message area */
        setD16(0xBB3FCu, 0x154);                                      /* 0x5AAF5 */
        setD16(0xBB3FEu, 0x12C);                                      /* 0x5AAFC */
        setD16(0xBB400u, 0x14);                                       /* 0x5AB03 */
        setD16(PLAYERS, 0);                                           /* 0x5AB0A */
        for (i = 1; i <= 9; i++) {                                    /* 0x5AB11..0x5AB24 */
            if (D8(NATION(i) + 0x9Au) != 0) {                         /* 0x5AB2D byte [199i + 0xF1566] */
                int16_t p = S16(PLAYERS);                             /* 0x5AB37 movsx edx, ax */
                setD16(PLAYERS, (uint16_t)(p + 1));                   /* 0x5AB41 */
                setD16(PLAYER_LIST + 2u * (uint32_t)(int32_t)p, (uint16_t)i);   /* 0x5AB4A */
            }
        }
        f783B8(PANEL, S32(PLAYER_BASE));                              /* 0x5AB64 (ebx = 0x51 rides along) */
        memset(DP(MATRIX), 0, 0x51);                                  /* 0x5AB70 memset(0x109E24, 0, ebx = 0x51) */
    }
    for (k = 0; k < 0x3C; k++)                                        /* 0x5AB75..0x5AB96 */
        list[k] = 0xFF;
    count = 0;                                                        /* 0x5AB9A */
    any = 0;                                                          /* 0x5AB9D */
    for (r = 1; r <= 0x9D; r++) {                                     /* 0x5ABA0..0x5AC3D */
        rec = AREA_REC(r);                                            /* 0x5AC43..0x5AC59 */
        nn = S8(rec + 1u);                                            /* 0x5AC5F movsx */
        if (!(nn > 1)) {                                              /* 0x5AC63 jg 0x5ACAF */
            if ((D8(TS(r - 1) + 2u) & 1u) != 1u)                      /* 0x5AC68..0x5AC85 no city: next */
                continue;
            if (nn != 1)                                              /* 0x5AC87..0x5AC8B (nn > 1 cannot hold here) */
                continue;
            if (D8(rec + D8(TS(r - 1)) + 0x0Eu) != 0)                 /* 0x5AC8D..0x5ACAD the owner's tokens there */
                continue;
        }
        limit = (int)POPLIMIT(r - 1);                                 /* 0x5ACAF..0x5ACD2 */
        sum = 0;                                                      /* 0x5ACD7 */
        for (i = 0; i < 0x0B; i++)                                    /* 0x5ACDA, 0x5ABB2..0x5ABC7 nations 1..11 */
            sum += S8(rec + (uint32_t)i + 0x0Fu);                     /* 0x5ABBE dword [rec + i + 0xC] sar 24 */
        if (!(sum > limit)) {                                         /* 0x5ABCC jg 0x5AC16 */
            if ((D8(TS(r - 1) + 2u) & 1u) != 1u)                      /* 0x5ABD1..0x5ABEE */
                continue;
            if (S8(rec + D8(TS(r - 1)) + 0x0Eu) != 0)                 /* 0x5ABF0..0x5AC14 [owner + rec + 0xB] sar 24 */
                continue;
        }
        list[count] = r;                                              /* 0x5AC1C */
        count++;                                                      /* 0x5AC26 */
        any = 1;                                                      /* 0x5AC29 */
    }

    if (any != 0) {                                                   /* 0x5ACE2 */
        nothing = 0;                                                  /* 0x5ACEE */
        if (show != 0) {                                              /* 0x5ACF4 */
            if ((D8(FLAGS) & PBEM_BIT) != 0)                          /* 0x5ACFE */
                setD32(CUR_NATION, (uint32_t)(int32_t)S8(PBEM_LOCAL));          /* 0x5AD03 dword [0xE161F] sar 24 */
            else
                setD32(CUR_NATION, (uint32_t)(int32_t)S16(PLAYER_LIST));        /* 0x5AD0D dword [0xF06AE] sar 16 */
        }
        for (pass = 0; pass <= 1; pass++) {                           /* 0x5AD1A..0x5AD3A */
            for (k = 0; k < count; k++) {                             /* 0x5AD2B..0x5AD53 */
                if (pass == 0) {                                      /* 0x5AD55 */
                    if ((D8(TS(list[k] - 1) + 2u) & 1u) != 0)         /* 0x5AD72 a city: not in pass 0 */
                        continue;
                } else {
                    if ((D8(TS(list[k] - 1) + 2u) & 1u) == 0)         /* 0x5AD97 no city: not in pass 1 */
                        continue;
                }
                if (list[k] == 0)                                     /* 0x5ADA0..0x5ADAC done */
                    continue;
                if (show != 0) {                                      /* 0x5ADB3 */
                    background3();                                    /* 0x5ADB5..0x5ADEE */
                    f7876C();                                         /* 0x5ADF1 */
                    f7853C(CONFLICT_DLG);                             /* 0x5ADFB */
                    if (S8(HUMANS) > 1)                               /* 0x5AE00..0x5AE0B */
                        f3C3D0(PANEL, 7);                             /* 0x5AE17 */
                    f3B05C(PANEL);                                    /* 0x5AE21 */
                    f3CBBC();                                         /* 0x5AE26 */
                }
                sum = 0;                                              /* 0x5AE2D (dead store, as the original) */
                setD8(PL_PENDING, 0);                                 /* 0x5AE39 */
                setD8(PL_OWN, 0);                                     /* 0x5AE3E */
                setD8(PL_ATT, 0);                                     /* 0x5AE48 */
                memset(DP(LOSS_TOK), 0, 0x0E);                        /* 0x5AE53 */
                memset(DP(LOSS_CITY), 0, 0x0E);                       /* 0x5AE64 */
                rec = AREA_REC(list[k]);                              /* 0x5AE69..0x5AE86 */
                f5C110(rec);                                          /* 0x5AE8C */
                f3F670();                                             /* 0x5AE91 */
                f3F624((int)D8(rec) - 1, 0x0F);                       /* 0x5AEA6 */
                city = D8(TS(list[k] - 1) + 2u) & 1u;                 /* 0x5AEAB..0x5AECE */
                if (city != 0) {                                      /* 0x5AED4 */
                    f5B224(rec);                                      /* 0x5AEDB */
                    list[k] = 0;                                      /* 0x5AEE5 */
                } else if (S8(rec + 1u) >= 2) {                       /* 0x5AEEE..0x5AEF8 */
                    f5B588(list[k]);                                  /* 0x5AF04 */
                    list[k] = city;                                   /* 0x5AF0C ecx = [ebp-0x24] = 0 */
                }
                if (show != 0) {                                      /* 0x5AF13 */
                    r = D8(rec);                                      /* 0x5AF1D..0x5AF22 */
                    if (S8(PREV_AREA(r) + 1u) == 1) {                 /* 0x5AF31 dword [0xE8886 + 69r] sar 24 */
                        a = f5C1CC(r - 1);                            /* 0x5AF47 */
                        b = f7C760((int)D8(rec) - 1);                 /* 0x5AF5A */
                        if (a > 0) {                                  /* 0x5AF62 */
                            uint32_t ar = AREA_REC(D8(rec));          /* 0x5AF68..0x5AF79 */
                            if (S8(ar + 0x0Eu + (uint32_t)a) == 0) {  /* 0x5AF7E [a + 69r + 0xE31C3] sar 24 */
                                if (S8(ar + 0x3Bu + (uint32_t)b) == 0)   /* 0x5AF8F [b + 69r + 0xE31F0] sar 24 */
                                    setD8(ar + 0x3Bu + (uint32_t)a, 1);  /* 0x5AFA0 */
                                else
                                    setD8(ar + 0x3Bu + (uint32_t)b, 0);  /* 0x5AFAE */
                            }
                        } else if (b <= 9) {                          /* 0x5AFB6..0x5AFBC */
                            setD8(AREA_REC(D8(rec)) + 0x3Bu + (uint32_t)b, 0);   /* 0x5AFD6 */
                        }
                    }
                    r = D8(rec);                                      /* 0x5AFDC..0x5AFE1 */
                    if ((D8(PREV_TS(r - 1) + 2u) & 1u) != 0              /* 0x5AFF2 a city before the movement */
                        && (D8(TS(r - 1) + 2u) & 1u) == 0) {             /* 0x5B003 and none now */
                        o = D8(PREV_TS(r - 1));                       /* 0x5B010 its owner */
                        r = D8(rec);                                  /* 0x5B01B..0x5B030 */
                        b = f7C760(r - 1);                            /* 0x5B036 */
                        if (S8(AREA_REC(r) + 0x3Bu + (uint32_t)b) == 0 && o <= 9)   /* 0x5B03B..0x5B04D */
                            setD8(AREA_REC(D8(rec)) + 0x3Bu + (uint32_t)o, 1);       /* 0x5B069 */
                        else {
                            r = D8(rec);                              /* 0x5B072..0x5B087 */
                            setD8(AREA_REC(r) + 0x3Bu + (uint32_t)f7C760(r - 1), 0);   /* 0x5B08F, 0x5B094 */
                        }
                    }
                    f3DA60((int)D8(rec) - 1);                         /* 0x5B0A6 */
                    f7844C(PANEL, S32(PLAYER_BASE));                  /* 0x5B0B6 */
                    f5CDE8(0, DSTR(0xB5C2Eu));                        /* 0x5B0C2 "CONFLICT RESOLUTION - UNITS REMOVED" */
                    f78664(CONFLICT_DLG);                             /* 0x5B0CC */
                }
                if (D8(PL_PENDING) != 0) {                            /* 0x5B0D1 */
                    saved = S32(CUR_NATION);                          /* 0x5B0DE */
                    f5BA0C(show);                                     /* 0x5B0E9 */
                    setD32(CUR_NATION, (uint32_t)saved);              /* 0x5B0F1 */
                }
            }
        }
    }

    if (show != 0) {                                                  /* 0x5B0FB the hostility */
        for (i = 0; i < 9; i++) {                                     /* 0x5B105..0x5B125 */
            for (j = 0; j < 9; j++) {                                 /* 0x5B114..0x5B13C */
                int      v = S8(MATRIX + 9u * (uint32_t)i + (uint32_t)j);         /* 0x5B14D [0x109E21 + 9i + j] sar 24 */
                uint32_t h = NATION(i + 1) + 0xA9u + (uint32_t)j;                 /* 0x5B156..0x5B160 [0xF1575 + ...] */
                if (v == 1) {                                         /* 0x5B163 */
                    setD8(h, (uint8_t)(D8(h) + 1));                   /* 0x5B168 inc */
                    h = NATION(i + v) + 0xA9u + (uint32_t)j;          /* 0x5B16E..0x5B179 (i + 1 again) */
                    if (S8(h) > 5)                                    /* 0x5B17C..0x5B188 */
                        setD8(h, 5);                                  /* 0x5B198 */
                } else {
                    setD8(h, (uint8_t)(D8(h) - 3));                   /* 0x5B1A1 sub 3 */
                    if (S8(h) < 0)                                    /* 0x5B1A8..0x5B1C0 */
                        setD8(h, 0);                                  /* 0x5B1D5 */
                }
            }
        }
    }
    if (nothing != 0 && show != 0)                                    /* 0x5B1E0..0x5B1EA */
        f79270(100, S16(PLAYER_LIST), -200, DSTR(0xB5C52u), 0);       /* 0x5B206 "" (dword [0xF06AE] sar 16) */
    f3C314(PANEL, 7);                                                 /* 0x5B215 */
}

/* ---- 0x5B224 the attack on a city -------------------------------------------------------------------------------- */

void f5B224(uint32_t rec)
{
    int     r;         /* [ebp-0x24] */
    int16_t owner;     /* [ebp-0x14] */
    int16_t a = 0;     /* [ebp-0x10] the attacker; uninitialised in the original if no marker is found (the guard below) */
    int16_t replace;   /* [ebp-0x18] */
    int     need;      /* [ebp-0x20] */
    int     k;         /* [ebp-0x1C] */

    r = D8(rec);                                                      /* 0x5B23F */
    owner = (int16_t)D8(TS(r - 1));                                   /* 0x5B255 */
    if (S8(rec + 1u) > 1)                                             /* 0x5B262 */
        f5B588(r);                                                    /* 0x5B26E */
    if (S8(rec + 1u) == 0) {                                          /* 0x5B276 */
        setD8(PL_PENDING, 0);                                         /* 0x5B280 */
        return;
    }
    for (k = 0; k < 0x0B; k++) {                                      /* 0x5B28B..0x5B29C */
        if (S8(rec + (uint32_t)k + 2u) <= 0)                          /* 0x5B2A4..0x5B2AA movsx, jle */
            continue;
        if (k + 1 == owner) {                                         /* 0x5B2AC..0x5B2B6 the owner first: no attack */
            setD8(PL_PENDING, (uint8_t)((k + 1) ^ owner));            /* 0x5B2B8 xor dl, al (0) */
            return;
        }
        a = (int16_t)(k + 1);                                         /* 0x5B2C8 */
        break;
    }
    if (a == 0) {   /* guard (file comment): no marker +2..+12 found, [ebp-0x10] uninitialised in the original */
        setD8(PL_PENDING, 0);
        return;
    }
    if (S8(NATION(owner) + 0x2Au) == 1 && S8(NATION(a) + 0x2Au) == 0) {          /* 0x5B2D0..0x5B2FD Engineering */
        replace = 7;                                                  /* 0x5B2FF */
        need = 8;                                                     /* 0x5B305 */
    } else if (S8(NATION(owner) + 0x2Au) == 0 && S8(NATION(a) + 0x2Au) == 1) {   /* 0x5B30E..0x5B33B */
        replace = 5;                                                  /* 0x5B33D */
        need = 6;                                                     /* 0x5B343 */
    } else {
        replace = 6;                                                  /* 0x5B34C */
        need = 7;                                                     /* 0x5B352 */
    }
    if (S8(rec + 0x0Eu + (uint32_t)a) < need) {                       /* 0x5B359..0x5B369 [a + rec + 0xB] sar 24 */
        setD8(PL_PENDING, 0);                                         /* 0x5B371 the attack fails */
        setD8(LOSS_TOK + (uint32_t)a, (uint8_t)(D8(LOSS_TOK + (uint32_t)a) + D8(rec + 0x0Eu + (uint32_t)a)));    /* 0x5B38D */
        setD8(NATION(a) + 0x99u, (uint8_t)(D8(NATION(a) + 0x99u) + D8(rec + 0x0Eu + (uint32_t)a)));             /* 0x5B3AF */
        setD8(NATION(a) + 0x98u, (uint8_t)(D8(NATION(a) + 0x98u) - D8(rec + 0x0Eu + (uint32_t)a)));             /* 0x5B3D1 */
        f57CE8(r - 1, a, 0, S8(rec + 0x0Eu + (uint32_t)a));           /* 0x5B3EE */
        return;
    }
    if (replace > S8(NATION(owner) + 0x99u) && owner != 10)           /* 0x5B3FB..0x5B41D the stock, not the Pirates' */
        replace = (int16_t)S8(NATION(owner) + 0x99u);                 /* 0x5B429 movsx ax, byte */
    f78F04(r - 1, owner);                                             /* 0x5B43D */
    setD8(LOSS_CITY + (uint32_t)owner, (uint8_t)(D8(LOSS_CITY + (uint32_t)owner) + 1));   /* 0x5B446 inc */
    f57E28(r - 1, owner, replace);                                    /* 0x5B458 */
    setD8(NATION(owner) + 0x99u, (uint8_t)(D8(NATION(owner) + 0x99u) - (uint8_t)replace));   /* 0x5B46A */
    setD8(NATION(owner) + 0x98u, (uint8_t)(D8(NATION(owner) + 0x98u) + (uint8_t)replace));   /* 0x5B47D */
    f5B588(r);                                                        /* 0x5B486 */
    if (S8(NATION(a) + 0x9Au) == 2) {                                 /* 0x5B48B..0x5B4A1 a computer */
        if (S8(rec + 0x0Eu + (uint32_t)a) >= 6                        /* 0x5B4A7..0x5B4B7 */
            || (D8(NATION(a) + 0x31u) != 0 && S8(rec + 0x0Eu + (uint32_t)a) >= 3)) {   /* 0x5B4B9..0x5B4DC Architecture */
            setD8(TS((int)D8(rec) - 1) + 2u, (uint8_t)(D8(TS((int)D8(rec) - 1) + 2u) | 2u));   /* 0x5B4EF */
            setD8(TS((int)D8(rec) - 1), (uint8_t)a);                  /* 0x5B50C */
            setD8(TS((int)D8(rec) - 1) + 1u, D8(TURN));               /* 0x5B52E the round's low byte */
        }
    }
    if (S8(rec + 0x0Eu + (uint32_t)a) > 0 && a != 11) {               /* 0x5B534..0x5B54C not the Barbarians */
        setD8(PL_PENDING, 1);                                         /* 0x5B54E */
        setD8(PL_ATT, (uint8_t)a);                                    /* 0x5B558 */
        setD8(PL_OWN, (uint8_t)owner);                                /* 0x5B560 */
        setD16(PL_REC, (uint16_t)r);                                  /* 0x5B568 */
        return;
    }
    setD8(PL_PENDING, 0);                                             /* 0x5B578 */
}

/* ---- 0x5B588 the token battle ------------------------------------------------------------------------------------ */

void f5B588(int r)
{
    uint8_t  pair[2 * 12];   /* [ebp-0x58] {tokens (+50), nation} x 11, plus the entry past the list (BUG 3) */
    uint8_t  copy[2 * 11];   /* [ebp-0x74] */
    uint32_t rec = AREA_REC(r);
    int      city;           /* [ebp-0x28] */
    int      count;          /* [ebp-0x2C] */
    int      total;          /* [ebp-0x20] */
    int      flag;           /* [ebp-0x1C] */
    int      i;              /* [ebp-0x30] */
    int      j;              /* [ebp-0x34] */
    int      old;            /* [ebp-0x18] */
    int      limit;          /* [ebp-0x24] */
    int      cur;            /* [ebp-0x38] */
    int      x;
    uint8_t  t0, t1;

    memset(pair, 0, sizeof pair);
    city = D8(TS(r - 1) + 2u) & 1u;                                   /* 0x5B59F..0x5B5B9 */
    count = 0;                                                        /* 0x5B5BE */
    total = 0;                                                        /* 0x5B5C1 */
    for (i = 1; i <= 0x0B; i++) {                                     /* 0x5B5C4..0x5B5D7 */
        if (D8(rec + 0x0Eu + (uint32_t)i) == 0)                       /* 0x5B5EF byte [0xE31C6 + 69r + i] */
            continue;
        pair[2 * count] = D8(rec + 0x0Eu + (uint32_t)i);              /* 0x5B614 */
        total += S8(rec + 0x0Eu + (uint32_t)i);                       /* 0x5B62A..0x5B633 [0xE31C3 + ...] sar 24 */
        if (D8(NATION(i) + 0x26u) != 0)                               /* 0x5B636..0x5B645 Metalworking */
            pair[2 * count] = (uint8_t)(pair[2 * count] + 0x32);      /* 0x5B64A */
        pair[2 * count + 1] = (uint8_t)i;                             /* 0x5B65F */
        count++;                                                      /* 0x5B652 */
    }
    flag = 0;                                                         /* 0x5B66A */
    if ((D8(FLAGS) & PBEM_BIT) != 0) {                                /* 0x5B66D PBEM only: the Military rule */
        for (i = 0; i < count; i++) {                                 /* 0x5B67C..0x5B68D */
            if (D8(NATION(pair[2 * i + 1]) + 0x35u) == 0)             /* 0x5B693..0x5B6AC */
                continue;
            for (j = i + 1; j < count; j++) {                         /* 0x5B6AE..0x5B6CA */
                if (D8(NATION(pair[2 * j + 1]) + 0x35u) != 0)         /* 0x5B6CC..0x5B6E5 */
                    continue;
                /* BUG(1995) 3: the original reads and decrements pair[count] (0x5B6E7, 0x5B6FA), one past the list */
                x = g_original ? count : j;
                if (pair[2 * x] == 0) {                               /* 0x5B6EE..0x5B6F5 and 0xFF; jle */
                    flag = 1;                                         /* 0x5B6B7 */
                    continue;
                }
                pair[2 * x]--;                                        /* 0x5B6FA */
                f57CE8(r - 1, pair[2 * j + 1], 0, 1);                 /* 0x5B716 */
                setD8(NATION(pair[2 * j + 1]) + 0x98u, (uint8_t)(D8(NATION(pair[2 * j + 1]) + 0x98u) - 1));   /* 0x5B72D */
                setD8(NATION(pair[2 * j + 1]) + 0x99u, (uint8_t)(D8(NATION(pair[2 * j + 1]) + 0x99u) + 1));   /* 0x5B745 */
                setD8(LOSS_TOK + pair[2 * j + 1], (uint8_t)(D8(LOSS_TOK + pair[2 * j + 1]) + 1));            /* 0x5B757 */
            }
        }
    }
    if (flag != 0) {                                                  /* 0x5B762 the empty pairs dropped */
        for (i = 0; i < count; i++) {                                 /* 0x5B76E..0x5B791 */
            copy[2 * i] = pair[2 * i];
            copy[2 * i + 1] = pair[2 * i + 1];
        }
        old = count;                                                  /* 0x5B796 */
        count = 0;                                                    /* 0x5B79B */
        for (i = 0; i < old; i++) {                                   /* 0x5B79E..0x5B7D9 */
            if (copy[2 * i] == 0)                                     /* 0x5B7B4..0x5B7BD */
                continue;
            pair[2 * count] = copy[2 * i];                            /* 0x5B7D4 word copy */
            pair[2 * count + 1] = copy[2 * i + 1];
            count++;
        }
    }
    if (count <= 1)                                                   /* 0x5B7DB */
        return;
    for (i = 1; i < count; i++) {                                     /* 0x5B7E5..0x5B7FA bubble sort, ascending */
        for (j = count - 1; j >= i; j--) {                            /* 0x5B7FC..0x5B811 */
            if (pair[2 * (j - 1)] > pair[2 * j]) {                    /* 0x5B819..0x5B823 unsigned, jbe */
                t0 = pair[2 * (j - 1)];                               /* 0x5B828 */
                t1 = pair[2 * (j - 1) + 1];
                pair[2 * (j - 1)] = pair[2 * j];                      /* 0x5B83B */
                pair[2 * (j - 1) + 1] = pair[2 * j + 1];
                pair[2 * j] = t0;                                     /* 0x5B846 */
                pair[2 * j + 1] = t1;
            }
        }
    }
    limit = (int)POPLIMIT(r - 1);                                     /* 0x5B84D..0x5B870 */
    i = 0;                                                            /* 0x5B875 */
    for (;;) {                                                        /* 0x5B878 */
        if (!((city == 0 && total > limit && S8(rec + 1u) > 1)        /* 0x5B878..0x5B89D [0xE31B6 + 69r] sar 24 */
              || (city != 0 && S8(rec + 1u) > 1)))                    /* 0x5B89F..0x5B8C0 */
            break;
        cur = pair[2 * i];                                            /* 0x5B8C6 the group's tokens */
        while (i < count && pair[2 * i] == cur) {                     /* 0x5B8D2..0x5B8ED */
            int n = pair[2 * i + 1];                                  /* 0x5B8F8 */
            if (S8(rec + 0x0Eu + (uint32_t)n) > 0) {                  /* 0x5B90B..0x5B917 */
                f57CE8(r - 1, n, 0, 1);                               /* 0x5B931 */
                if (D8(REAL_RES) != 0) {                              /* 0x5B936 back to stock */
                    setD8(NATION(n) + 0x98u, (uint8_t)(D8(NATION(n) + 0x98u) - 1));   /* 0x5B951 */
                    setD8(NATION(n) + 0x99u, (uint8_t)(D8(NATION(n) + 0x99u) + 1));   /* 0x5B969 */
                }
                setD8(LOSS_TOK + (uint32_t)n, (uint8_t)(D8(LOSS_TOK + (uint32_t)n) + 1));   /* 0x5B97B */
                total--;                                              /* 0x5B984 */
            }
            i++;                                                      /* 0x5B98A */
        }
        if (i >= count)                                               /* 0x5B992..0x5B998 */
            i = 0;                                                    /* 0x5B9A0 */
    }
    if (S8(rec + 0x18u) != 0) {                                       /* 0x5B9A8..0x5B9BE the Pirates' tokens */
        setD8(LOSS_TOK + 10u, (uint8_t)(D8(LOSS_TOK + 10u) + D8(rec + 0x18u)));   /* 0x5B9D9 byte [0xF5486] */
        f57CE8(r - 1, 10, 0, S8(rec + 0x18u));                        /* 0x5B9FE */
    }
}

/* ---- 0x5BA0C the plunder ----------------------------------------------------------------------------------------- */

void f5BA0C(int show)
{
    char     buf[200];   /* [ebp-0xFC] */
    uint32_t orec;       /* [ebp-0x18] */
    int      i;          /* [ebp-0x34] */
    int      j;          /* [ebp-0x1C] */
    int      card;       /* [ebp-0x20] */
    int      chosen;     /* [ebp-0x24] */
    int      amount;     /* [ebp-0x28] */
    int      code;       /* [ebp-0x2C] */
    int      a;
    int32_t  idx;

    if (show != 0) {                                                  /* 0x5BA27 */
        if (S8(PL_OWN) <= 9) {                                        /* 0x5BA31..0x5BA3C dword [0x109E07] sar 24 */
            orec = NATION(S8(PL_OWN));                                /* 0x5BA42..0x5BA57 */
            setD32(CARD_N, 0);                                        /* 0x5BA4F */
            for (i = 1; i <= 0x12; i++) {                             /* 0x5BA5A..0x5BA6D the commodity cards */
                if (D8(orec + (uint32_t)i) == 0)                      /* 0x5BA75 */
                    continue;
                for (j = 0; S8(orec + (uint32_t)i) > j; j++) {        /* 0x5BA7B..0x5BA94 */
                    idx = S32(CARD_N);                                /* 0x5BA96 */
                    setD32(CARD_N, (uint32_t)(idx + 1));              /* 0x5BA9F */
                    setD8(CARD_LIST + (uint32_t)idx, (uint8_t)i);     /* 0x5BAA7 */
                }
            }
            if (S32(CARD_N) > 0) {                                    /* 0x5BAAF */
                if (D8(orec + 0x13u) != 0) {                          /* 0x5BABF the calamities too */
                    for (i = 0; S8(orec + 0x13u) > i; i++) {          /* 0x5BAC8..0x5BADF [rec + 0x10] sar 24 */
                        idx = S32(CARD_N);                            /* 0x5BAE7 */
                        setD32(CARD_N, (uint32_t)(idx + 1));          /* 0x5BAF1 */
                        setD8(CARD_LIST + (uint32_t)idx, D8(orec + (uint32_t)i + 0x14u));   /* 0x5BAF7, 0x5BAFA */
                    }
                }
                idx = (int32_t)(((uint32_t)f78AE4() * (uint32_t)S32(CARD_N)) >> 15);   /* 0x5BB02 game_rand; imul; shr 15 */
                card = S8(CARD_LIST + (uint32_t)idx);                 /* 0x5BB11 [idx + 0x112581] sar 24 */
                f75CA8((int8_t)card, orec);                           /* 0x5BB24 from the owner */
                f75D84((int8_t)card, NATION(S8(PL_ATT)), orec);       /* 0x5BB45 to the attacker, the owner as giver */
                snprintf(buf, sizeof buf, "%s gets a random trade good from %s as plunder.",   /* 0x5BB81 (0xB5C53) */
                         NAME(S8(PL_ATT)), NAME(S8(PL_OWN)));
                f79270(0x9C4, S8(PL_ATT), -200, buf, 0);              /* 0x5BB99 */
            }
        }
        background3();                                                /* 0x5BB9E..0x5BBD8 */
        f7876C();                                                     /* 0x5BBDB */
    }
    chosen = 0;                                                       /* 0x5BBE2 */
    amount = S8(NATION(S8(PL_ATT)) + 0x99u);                          /* 0x5BBE5..0x5BBF9 [0xF1562 + 199a] sar 24 */
    if (amount > 3)                                                   /* 0x5BBFC */
        amount = 3;                                                   /* 0x5BC01 */
    if (amount <= 0) {                                                /* 0x5BC09 */
        if (show != 0) {                                              /* 0x5BEDA */
            if (g_original)   /* BUG(1995) (text): the missing space after the comma (0xB5CDE) */
                snprintf(buf, sizeof buf, "%s,you have no stock to pillage with.", NAME(S8(PL_ATT)));    /* 0x5BF08 */
            else
                snprintf(buf, sizeof buf, "%s, you have no stock to pillage with.", NAME(S8(PL_ATT)));
            f79270(0x9C4, S8(PL_ATT), -200, buf, 0);                  /* 0x5BF20 */
        }
        return;
    }
    if (((D8(FLAGS) & PBEM_BIT) == 0 && S8(NATION(S8(PL_ATT)) + 0x9Au) == 3)    /* 0x5BC13..0x5BC38 a human */
        || ((D8(FLAGS) & PBEM_BIT) != 0 && D8(PL_ATT) == D8(PBEM_LOCAL))) {     /* 0x5BC3A..0x5BC52 the PBEM player */
        if (show == 0)                                                /* 0x5BC58 */
            return;
        setD32(CUR_NATION, (uint32_t)(int32_t)S8(PL_ATT));            /* 0x5BC72 */
        fg_vbopen(S32(BACKBUF));                                      /* 0x5BC77 */
        f3AF8C(0xF51ACu, 0x128, 0x181);                               /* 0x5BC8E the two arrows */
        f3AF8C(0xF51B4u, 0x14A, 0x181);                               /* 0x5BCA2 */
        f3A3A0(0x125, 0x185, DSTR(0xB5C83u));                         /* 0x5BCB6 "UNITS TO" */
        f3A044(0x15A, 0x185, DSTR(0xB5C8Cu));                         /* 0x5BCCA "TRANSFER" */
        fg_vbclose();                                                 /* 0x5BCCF */
        snprintf(buf, sizeof buf, "%s, you may transfer %s%d unit%s from stock to your treasury.",   /* 0x5BD1E (0xB5CA0) */
                 NAME(S8(PL_ATT)),
                 amount == 1 ? DSTR(0xB5C98u) : DSTR(0xB5C99u),       /* 0x5BCF0 "" / "up to " */
                 amount,
                 amount == 1 ? DSTR(0xB5C95u) : DSTR(0xB5C96u));      /* 0x5BCD7 "" / "s" */
        f3A9F4(buf);                                                  /* 0x5BD2C */
        f7853C(PLUNDER_DLG);                                          /* 0x5BD3B (edx = 7 rides along) */
        f3C314(PANEL, 7);                                             /* 0x5BD45 */
        f3B05C(PANEL);                                                /* 0x5BD4F */
        for (;;) {                                                    /* 0x5BD54 */
            fg_vbopen(S32(BACKBUF));                                  /* 0x5BD5A */
            f3C9D0(0x136, 0x181, 0x13, 0x13, 5);                      /* 0x5BD75 */
            fg_setcolor(5);                                           /* 0x5BD7C */
            snprintf(buf, sizeof buf, "%d", chosen);                  /* 0x5BD98 itoa(chosen, buf, 10) */
            f3A0B0(0x136, 0x185, 0x13, buf);                          /* 0x5BDAC */
            fg_vbclose();                                             /* 0x5BDB1 */
            f459E4(S8(PL_ATT));                                       /* 0x5BDBE */
            f3F670();                                                 /* 0x5BDC3 */
            f3F624(S16(PL_REC) - 1, 0x0F);                            /* 0x5BDD6 dword [0x109E0A] sar 16 */
            f3DA60(S16(PL_REC) - 1);                                  /* 0x5BDE6 */
            f3CBBC();                                                 /* 0x5BDEB */
            code = f78BA4(PANEL, 0);                                  /* 0x5BDF5 */
            if (code == -100)                                         /* 0x5BE00 */
                continue;
            if (code == -1) {                                         /* 0x5BE0C FINISHED (id 0x2A) */
                a = S8(PL_ATT);                                       /* 0x5BE11 */
                setD8(NATION(a) + 0x99u, (uint8_t)(D8(NATION(a) + 0x99u) - (uint8_t)chosen));   /* 0x5BE23 */
                setD8(NATION(a) + 0x3Eu, (uint8_t)(D8(NATION(a) + 0x3Eu) + (uint8_t)chosen));   /* 0x5BE36 */
                f459E4(a);                                            /* 0x5BE3C */
                f3CBBC();                                             /* 0x5BE41 */
                f78A20(1000);                                         /* 0x5BE4B */
                break;
            }
            if ((uint32_t)code < (uint32_t)-12)                       /* 0x5BE55 jb */
                continue;
            if (code == -12) {                                        /* 0x5BE5E jbe: up */
                if (chosen < amount)                                  /* 0x5BE7E */
                    chosen++;                                         /* 0x5BE8D */
                continue;
            }
            if (code == -11 && chosen > 0)                            /* 0x5BE60..0x5BE6D down */
                chosen--;                                             /* 0x5BE76 */
        }
        f78664(PLUNDER_DLG);                                          /* 0x5BE9A */
        return;
    }
    if (S8(NATION(S8(PL_ATT)) + 0x9Au) == 2)                          /* 0x5BEA9..0x5BEC3 a computer */
        f5BF30(S8(PL_ATT), amount, show);                             /* 0x5BECB (M7) */
}

/* ---- 0x5C110 the conflict matrix row ----------------------------------------------------------------------------- */

void f5C110(uint32_t rec)
{
    int8_t i, j;   /* [ebp-0x8], [ebp-0x4]: signed byte counters */
    int    owner_row;

    for (i = 0; i < 9; i++) {                                         /* 0x5C126..0x5C13A */
        if (D8(rec + (uint32_t)i + 2u) == 0) {                        /* 0x5C147 not present */
            if ((D8(TS((int)D8(rec) - 1) + 2u) & 1u) == 0)            /* 0x5C14E..0x5C16B no city */
                continue;
            /* BUG(1995): the owner (a nation number) is compared with the row index i (0x5C179); nation i + 1 meant */
            owner_row = g_original ? i : i + 1;
            if ((int)D8(TS((int)D8(rec) - 1)) != owner_row)           /* 0x5C16D..0x5C17B */
                continue;
        }
        for (j = 0; j < 9; j++) {                                     /* 0x5C17D..0x5C191 */
            if (D8(rec + (uint32_t)j + 2u) == 0)                      /* 0x5C19A */
                continue;
            if (j == i)                                               /* 0x5C1A1..0x5C1A7 */
                continue;
            setD8(MATRIX + 9u * (uint32_t)i + (uint32_t)j, 1);        /* 0x5C1BA */
        }
    }
}

/* ---- 0x5C1CC the pre-movement owner ------------------------------------------------------------------------------ */

int f5C1CC(int t)
{
    int r = -1;   /* [ebp-0x8] */
    int i;        /* [ebp-0xC] */

    for (i = 0; i < 9; i++) {                                         /* 0x5C1EF..0x5C200 */
        if (S8(PREV_AREA(t + 1) + (uint32_t)i + 2u) == i + 1) {       /* 0x5C214 [0xE8887 + 69(t+1) + i] sar 24 */
            r = i + 1;                                                /* 0x5C229 */
            break;
        }
    }
    return r;
}

/* ---- 0x5CDE8 the losses box -------------------------------------------------------------------------------------- */

int f5CDE8(int anim, const char *title)
{
    char        buf[20];   /* [ebp-0x4C] */
    int         result;    /* [ebp-0x18] */
    int         count;     /* [ebp-0x20] */
    int         n;         /* [ebp-0x34] */
    int         colw;      /* [ebp-0x14] */
    int         x0;        /* [ebp-0x1C] */
    int         col;       /* [ebp-0x24] */
    int         go;        /* [ebp-0x2C] */
    int         code;      /* [ebp-0x28] */
    int         width;
    const char *name;

    result = 0;                                                       /* 0x5CE04 */
    count = 0;                                                        /* 0x5CE07 */
    for (n = 1; n <= 0x0D; n++) {                                     /* 0x5CE0A..0x5CE1D */
        if (D8(NATION(n) + 0x9Au) == 0)                               /* 0x5CE26 */
            continue;
        if (D8(LOSS_TOK + (uint32_t)n) != 0 || D8(LOSS_CITY + (uint32_t)n) != 0)   /* 0x5CE33..0x5CE46 */
            count++;                                                  /* 0x5CE4B */
    }
    width = count == 2 ? 0xC8 : 0x12C;                                /* 0x5CE50..0x5CE5D */
    colw = count != 0 ? width / count : 0;                            /* 0x5CE6A idiv (0 faults in the original) */
    x0 = count == 2 ? 0xDC : 0xAA;                                    /* 0x5CE6F..0x5CE7C */
    col = 0;                                                          /* 0x5CE86 */
    f3A924();                                                         /* 0x5CE89 */
    f3A9F4(title);                                                    /* 0x5CE91 */
    fg_vbopen(S32(BACKBUF));                                          /* 0x5CE9D */
    fg_vbcopy(0xAA, 0x1D5, 0x178, 0x1A2, 0xAA, 0x1A2, S32(BACKGROUND), S32(BACKBUF));   /* 0x5CED1 */
    fg_setcolor(5);                                                   /* 0x5CEE0 */
    for (n = 1; n <= 0x0D; n++) {                                     /* 0x5CEE8, 0x5D00D */
        if (D8(NATION(n) + 0x9Au) == 0)                               /* 0x5D01E */
            continue;
        if (D8(LOSS_TOK + (uint32_t)n) == 0 && D8(LOSS_CITY + (uint32_t)n) == 0)   /* 0x5D02B..0x5D03E */
            continue;
        fg_setcolor(5);                                               /* 0x5D042 */
        if (fgf_width(NAME(n), (int)strlen(NAME(n))) >= colw) {       /* 0x5D057..0x5D07D jge */
            /* BUG(1995): 0xC7664 has short names for 0..9 only; 10..13 read the next table (the file comment) */
            name = (n > 9 && !g_original) ? NAME(n) : SHORT(n);       /* 0x5CEF0 */
            f3A0B0(colw * col + x0, 0x16D, colw, name);               /* 0x5CF0A */
        } else {
            f3A0B0(colw * col + x0, 0x16D, colw, NAME(n));            /* 0x5D083..0x5D09B, 0x5CF0A */
        }
        f3AF8C(0xF489Cu + 8u * (uint32_t)n,                           /* 0x5CF46 the token, centred */
               colw * col + x0 + (colw - (int)D16(0xF48A0u + 8u * (uint32_t)n)) / 2, 0x178);
        snprintf(buf, sizeof buf, "%d", S8(LOSS_TOK + (uint32_t)n));  /* 0x5CF61 (0xB5E10) [0xF5479 + n] sar 24 */
        f3A0B0(colw * col + x0 + 1, 0x18C, colw, buf);                /* 0x5CF81 */
        f3AF8C(0xF497Cu + 8u * (uint32_t)n,                           /* 0x5CFC1 the city, centred */
               colw * col + x0 + (colw - (int)D16(0xF4980u + 8u * (uint32_t)n)) / 2, 0x197);
        snprintf(buf, sizeof buf, "%d", S8(LOSS_CITY + (uint32_t)n)); /* 0x5CFDC (0xB5E13) [0xF5489 + n] sar 24 */
        f3A0B0(colw * col + x0 + 1, 0x1A8, colw, buf);                /* 0x5CFFC */
        col++;                                                        /* 0x5D004 */
    }
    fg_vbclose();                                                     /* 0x5D0A0 */
    f3CBBC();                                                         /* 0x5D0A5 */
    f3D52C(0, 0);                                                     /* 0x5D0AE */
    go = 1;                                                           /* 0x5D0B3 */
    if (anim != 0)                                                    /* 0x5D0BA */
        f7A4C4(anim);                                                 /* 0x5D0C3 (ANIM) */
    while (go != 0) {                                                 /* 0x5D0C8 */
        if (f7BA64() == 0) {                                          /* 0x5D0D2 */
            code = f78BA4(PANEL, 0);                                  /* 0x5D0E2 */
        } else {
            f78A20(S16(0xF40BAu) == 0 ? 0xFA : 0x3E8);                /* 0x5D0EC..0x5D107 unattended */
            code = -1;                                                /* 0x5D10C */
        }
        if (code == -1)                                               /* 0x5D10F */
            break;
        if ((uint32_t)code < (uint32_t)-0x51)                         /* 0x5D118 jb */
            continue;
        if (code == -0x51) {                                          /* 0x5D11D jbe: SECOND FACTION */
            result = 13;                                              /* 0x5D132 */
            go = 0;                                                   /* 0x5D13B */
        } else if (code == -0x50) {                                   /* 0x5D11F: FIRST FACTION */
            result = 12;                                              /* 0x5D124 */
            go = 0;                                                   /* 0x5D12D */
        }
    }
    return result;                                                    /* 0x5D140 */
}
