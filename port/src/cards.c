/* Module CARDS of docs/M6_PLAN.md: ACQUIRE GOODS 0x6BD98 (3,572 bytes, DrawTradingCards) with its screen 0x6CB8C
   (532, DrawInitialDrawingScreen), the left column 0x6CDA0 (432), the treasury box 0x6CF50 (192), the hand strip
   0x6D010 / 0x6D0C4 (180 each), ACQUIRE 0x6D178 (1,004) and BUY 0x6D564 (1,436); DISCARD EXCESS 0x6E770 (256) with the
   human's loop 0x6E870 (1,044) and its screen 0x6EC84 (292); the reshuffle 0x77278 (664) with 0x77510 (224); the trade
   screens' helpers REVEAL GOODS 0x6FBDC (492), the POINTS box 0x6FE68 (348), its eraser 0x6FFC4 (176) and the two
   rectangle copies 0x71D30 (260) / 0x71E34 (248). Literal translations; the address of each instruction a line comes
   from is in its comment. D = read in the code, G = inference. Prototypes and globals: cards.h.

     0x6BD98  f6BD98  ACQUIRE GOODS (phase 11 of the game loop, call at 0x54C6D)
     0x6CB8C  f6CB8C  the acquire screen (caller 0x6C072)
     0x6CDA0  f6CDA0  the left column (9 call sites in 0x6BD98)
     0x6CF50  f6CF50  the treasury box (0x6C26E, 0x6D9BA)
     0x6D010  f6D010  a hand face down in the strip (0x6C312, 0x6C61E, 0x6EAE8)
     0x6D0C4  f6D0C4  a hand face up in the strip (0x6C37A, 0x6D342, 0x6D9FB, 0x6E993, 0x6EB0C, ADV's 0x162D0)
     0x6D178  f6D178  ACQUIRE, the human (0x6C565)
     0x6D564  f6D564  BUY, the human (0x6C581)
     0x6E770  f6E770  DISCARD EXCESS (phase 19, call at 0x55186)
     0x6E870  f6E870  the discards (0x6E826)
     0x6EC84  f6EC84  the discard screen (0x6EAD1)
     0x77278  f77278  the reshuffle (0x6E854; PBEM's 0x52F33)
     0x77510  f77510  a non-tradable calamity out of a pile (0x773C1)
     0x6FBDC  f6FBDC  REVEAL GOODS (0x162D0, 0x6BD98, 0x6E870, TRADE's 0x6F270, 0x723DC, 0x734E4, 0x75520)
     0x6FE68  f6FE68  the POINTS box (also TRADE)
     0x6FFC4  f6FFC4  the POINTS box cleared (0x6C2D6)
     0x71D30  f71D30  a restore rectangle into the back buffer (TRADE only)
     0x71E34  f71E34  a restore rectangle to the screen

   Not ported here (each call site stops the port with not_ported(address); the static stubs below carry the names, so
   the call sites read as in the original and a later translation that exports one of these names collides with its
   stub at compile time instead of being shadowed by it):
     0x6DBAC  a computer acquires its cards face down (M7, AI; 0x6C5E1)
     0x6DE04  a computer decides on a purchase and buys (M7, AI; 0x6C6B2; it calls ADV's 0x17EDC)
     0x6EDA8  a computer discards down to 8 (M7, AI; 0x6EC61; it calls 0x6F078)
     0x6E158  PBEM's local buy screen for a 9-city nation (dropped; 0x6C3D9)
   0x6F078 (the computer's ranking of its goods) and ADV's 0x17EDC are reached only through 0x6EDA8 / 0x6DE04: no stub
   here. So an ACQUIRE GOODS with a computer that has cities stops at 0x6DBAC, a DISCARD EXCESS at a computer over 8
   cards at 0x6EDA8; an all-human game runs both phases to the end. Built with -DCARDS_TEST (port/test/cards_test.c)
   the four go to the test's recorders instead, which return.

   Play by e-mail (dropped by Erik's ruling; the setup screen cannot start one): the PBEM-only blocks of 0x6BD98 stop
   with not_ported(their address): the snapshot of the stacks and nations into the heap fields [0xEED80] / [0xEED78]
   (0x6BE7A..0x6BF26), 0x6E158 (0x6C3D9), the computer's pre-declared buy codes (0x6C827..0x6C9CE), the wait for the
   remote players (0x6C9FE..0x6CA8F), the restore of the snapshots (0x6CAAC..0x6CB52) and their free (0x6CB5D). The
   other bit-0x80 tests of byte [0xE1624], the local-player byte [0xE1622] (int8, read as dword [0xE161F] sar 24), the
   flags int16 [0xF5478] / [0xF4098] and the pre-declared buy codes int16 0xF0684[n] (-0x61 buy, -0x62 do not; written
   only by PBEM code and the save reader) are kept literally.

   Watcom runtime -> standard C:
     0x7DB3D sprintf -> snprintf bounded to the stack buffer: 72 bytes in 0x6D178 ([ebp-0x60]..[ebp-0x18]), 80 in
             0x6D564 ([ebp-0x64]..[ebp-0x14])
     0x8C897 itoa(v, buf, 10) -> snprintf "%d" into a buffer of the original's size (0x6BD98 [ebp-0xB8], 0x6CDA0
             [ebp-0x50], 0x6CF50 [ebp-0x24], 0x6E870 [ebp-0x40], 0x6FE68 [ebp-0x24])
     inline strcpy of "0" / "?" (0x6C895, 0x6C8DD, 0x6C902) -> PBEM only, dropped
     0x8CE50 fg_makegif / 0x88F98 fg_showgif "junk" -> junk_makegif / junk_showgif (junk.h: kept in memory)
   __CHK dropped; the debug logger 0x78B0C ("DrawTradingCards", "DrawInitialDrawingScreen") is a no-op and dropped.
   game_rand f78AE4: only through f76F70 in f77278: one f76F70 call per non-empty discard pile, piles in the order
   0..8, and f76F70 draws once per card it shuffles (0x76FBB, in its loop), so a reshuffle draws as many numbers as
   the piles hold cards, less the non-tradable calamities 0x77510 takes out of piles 1..4 first (a pile holding only
   that calamity still gets its f76F70 call, with 0 cards: no draw) (D). No other routine here draws a random number.

   Register-carried arguments (D, as the callees keep their registers): 0x6CB8C's f3A18C gets edx = 8 and ebx = 0x280
   from the rectangle stores just before; 0x6D3E3 edx = 0x8A rides through f71E34 and f788AC into f3C530; 0x6D448 edx =
   0x8B through f7983C into f3C530; 0x6D831 ebx = "CANCEL" through f3C530 into f3C284; 0x6C3E6 edx = 0 through f3CD6C
   into f78BA4; 0x6C077 ecx = 0 (not an argument of f6CB8C). Each is written out at its call.

   Data (D, read in the code at the addresses given): cards.h's globals; nation n = 0xF14CC + 199n: +0x01..+0x12 int8
   commodity cards, +0x13 int8 calamities, +0x3C int8 the cards bought this turn (read only by PBEM 0x53018), +0x3E int8
   treasury, +0x51 int8 cities, +0x99 int8 stock, +0x9A 0 out / 2 computer / 3 human; byte [0xE1620] the humans;
   sprites 0xF4ACC + 8n (token 24x23), 0xF49EC + 8n (token 23x23), 0xF4C1C + 8c (cardtiny c), 0xF4D14 (a card back);
   names 0xC768C[n]; 0xCA9E0[1..4] = 0x13, 0x15, 0x17, 0x19 (obj2); dialog 0xBBBEC (CONTINUE 0x2A, kit.h's f7983C).

   1995 bugs, fixed unless g_original (each marked BUG(1995) in the code; grades D code / G effect):
     1. 0x6D8E8: a stack-choice click (-0x34) between two columns (or right of the last) leaves s = -100; the code then
        draws f76E50(-100, n) (the "stack" 2,500 bytes before 0xF06D0, inside the data object) and pays 2 * (s + 1) as
        a byte: treasury - 58, stock + 58 (D). Fixed: the click is ignored (the loop waits on).
     2. 0x6D8A7: the columns of the hit test run to i <= max, max being the first stack the treasury cannot pay (8 when
        every stack is affordable): the column of that stack, drawn empty, still buys its card when it holds one, and
        the treasury goes below 0 (D). Fixed: the hit test also needs treasury >= 2i + 2.
     3. 0x6D800: +0x3C counts a purchase before the choice; CANCEL leaves it counted (D code; G effect: read only by
        PBEM 0x53018, but the byte is in the save). Fixed: CANCEL takes it back.
     4. 0x6D5F3: without the variant BUY takes stack 8's top card unchecked: from an empty stack f76E50 returns 0 and the
        nation pays 18, gets nothing, "Buy" counts 1 and card 0's sprite is drawn (f6DB00 only tests the treasury
        there) (D code, G reach: stack 8 is empty only after many purchases). Fixed: nothing is bought, paid or
        counted; BUY stays as f6DB00 says.
     5. 0x6C6C1: the computer's "Buy" counter [0x110ACC] is the result of its last 0x6DE04 call (0 or 1), so a computer
        that bought shows 0 when its last try did not buy (D; G: only the screen). Fixed: the purchases are counted.
     6. 0x6EB2A (M6_PLAN risk 6): the discard loop ends only when the hand is down to 8, and only commodities can be
        clicked: a hand over 8 with no commodity left (calamities only) waits for ever, FINISHED being disabled (D code,
        G reach: calamities are resolved in phase 15, before). Fixed: with no commodity left the loop ends.
   Kept as the original (G unless marked):
     - 0x6D41F: the PBEM pre-declared buy draws stack 8's card before testing f6DB00; when that fails the card is kept
       unpaid (D code; reachable only with a pre-declared code -0x61 in int16 0xF0684[n], which only PBEM writes).
     - 0x6CF50: the treasury number is printed on the screen (0x6CFCD comes after f71E34's fg_vbclose) and the box is
       then pasted from the last opened buffer, the background (fg.c's fg_vbpaste), over it; whether the original
       shows the number at all is for the DOSBox-X frame (G).
     - 0x6BDC8: the sort works on 8 local {cities, nation} pairs; with 9 players the ninth pair would overlay the
       cities local [ebp-0x4C] (D code, unreachable: at most 8 players). An eliminated nation leaves [0xF40EC] in step
       (0x22C26), so no zero pair is sorted in (G).
   Easter eggs / oddities: the trace names "DrawTradingCards" (0xB8BF5) and "DrawInitialDrawingScreen" (0xB8C0E)
   passed to the no-op logger; the empty "no goods" (0xB8BF4) and "no discards" (0xB8E18) messages (100 ms home view
   of the first player); REVEAL GOODS saves the screen to a GIF file "junk" and relabels the shared CONTINUE of 0xBBBEC
   "NOT REVEAL GDS", which stays (M6_PLAN eggs 17, 22); the PBEM placeholders "0" / "?" in the computer's counters
   (0xB8C06..0xB8C0C); the column head "Acqrd" (Erik: keep); the "Buy" counter is the only one with three label
   strings "You bought..." (0xB8C7C, 0xB8C8A, 0xB8CCA: the compiler did not merge them). */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cards.h"
#include "fg.h"
#include "formulas.h"
#include "game.h"
#include "info.h"
#include "junk.h"
#include "kit.h"
#include "loop.h"
#include "panel.h"
#include "rules.h"
#include "state.h"
#include "ui.h"

#define NATION(n)   (0xF14CCu + 199u * (uint32_t)(n))
#define NAME(n)     DSTR(D32(0xC768Cu + 4u * (uint32_t)(n)))
#define FLAGS       0xE1624u
#define PBEM_BIT    0x80u
#define LOCAL_PBEM  0xE1622u   /* int8: dword [0xE161F] sar 24 */
#define HUMANS      0xE1620u   /* int8: dword [0xE161D] sar 24 */
#define CUR_NATION  0xF40F8u
#define PLAYERS_N   0xF40ECu   /* int32 */
#define PLAYERS     0xF06AEu   /* int16 count */
#define PLAYER_LIST 0xF06B0u   /* int16[] */
#define BACKBUF     0xF52C8u
#define BACKGROUND  0xF52CCu
#define CUR_DIALOG  0xF8E78u
#define PICTURE     0xEE168u   /* int8 */
#define ACQ_DLG     0x1106D8u
#define DISC_DLG    0x110ADCu
#define BOX_DLG     0xBBBECu
#define HAND_SIZE   0x110AD0u  /* int8[] per list position */
#define PER_CITY    0x110AC4u  /* int32 */
#define ACQUIRED    0x110AC8u  /* int32 */
#define BOUGHT      0x110ACCu  /* int32 */
#define BOUGHT_FROM 0xF590Cu   /* int32[] per nation */
#define BUY_CODE    0xF0684u   /* int16[] per nation (PBEM) */
#define RECT        0x111524u  /* int16 [14][4] */
#define STACK(s)    (0xF06D0u + 25u * (uint32_t)(s))
#define PILE(s)     (0xF07B4u + 25u * (uint32_t)(s))
#define MOUSE_X     0xE1628u
#define SPACING1    0x11254Cu
#define SPACING2    0x112550u
#define CARD_BACK   0xF4D14u
#define CARDTINY(c) (0xF4C1Cu + 8u * (uint32_t)(int32_t)(c))
#define TOKEN23(n)  (0xF49ECu + 8u * (uint32_t)(n))
#define TOKEN24(n)  (0xF4ACCu + 8u * (uint32_t)(n))
#define ROW(r)      (49 * (r))   /* lea eax,[4r]; sub eax,r; shl eax,4; add eax,r */
#define COL(s)      (39 * (s))   /* lea eax,[4s]; add eax,s; shl eax,3; sub eax,s */

/* ---- calls into code that is not ported yet (see the file comment) ----------------------------------------------- */

#ifdef CARDS_TEST
void cards_test_f6DBAC(int p, int n);
int  cards_test_f6DE04(int n);
void cards_test_f6EDA8(int n);
void cards_test_f6E158(void);
void cards_test_pbem(uint32_t addr);
#define f6DBAC(p, n)  cards_test_f6DBAC(p, n)
#define f6DE04(n)     cards_test_f6DE04(n)
#define f6EDA8(n)     cards_test_f6EDA8(n)
#define f6E158()      cards_test_f6E158()
#define pbem_dropped(a) cards_test_pbem(a)
#else
#include "aicards.h"                            /* f6DBAC, f6DE04, f6EDA8: the computers' cards (M7) */

static void f6E158(void)                        /* PBEM (dropped): the local player's buy screen */
{
    not_ported(0x6E158u);
}

static void pbem_dropped(uint32_t addr)         /* PBEM (dropped): a PBEM-only block of 0x6BD98 */
{
    not_ported(addr);
}
#endif

/* ---- 0x71D30 / 0x71E34 the restore rectangles ------------------------------------------------------------------- */

void f71D30(int s)
{
    int x, y, w, h;

    if (s == -1) {                                                    /* 0x71D48 */
        x = S16(0x11158Cu);                                           /* 0x71D72 dword [0x11158A] sar 16 */
        y = S16(0x11158Eu);                                           /* 0x71D54 */
        w = S16(0x111590u);                                           /* 0x71D7E */
        h = S16(0x111592u);                                           /* 0x71D5A */
    } else {
        x = S16(RECT + 8u * (uint32_t)s);                             /* 0x71DBF dword [8s + 0x111522] sar 16 */
        y = S16(RECT + 8u * (uint32_t)s + 2u);                        /* 0x71D9F */
        w = S16(RECT + 8u * (uint32_t)s + 4u);                        /* 0x71DF9 */
        h = S16(RECT + 8u * (uint32_t)s + 6u);                        /* 0x71DAC */
    }
    fg_vbcopy(x, x + w - 1, y, y + h - 1, x, y + h - 1, S32(BACKGROUND), S32(BACKBUF));   /* 0x71E22 */
}

void f71E34(int s)
{
    int x, y, w, h;

    fg_vbopen(S32(BACKGROUND));                                       /* 0x71E51 */
    if (s == -1) {                                                    /* 0x71E59 */
        x = S16(0x11158Cu);                                           /* 0x71E75 */
        y = S16(0x11158Eu);                                           /* 0x71E5F */
        w = S16(0x111590u);                                           /* 0x71E81 */
        h = S16(0x111592u);                                           /* 0x71E65 */
    } else {
        x = S16(RECT + 8u * (uint32_t)s);                             /* 0x71F09 */
        y = S16(RECT + 8u * (uint32_t)s + 2u);                        /* 0x71E93 */
        w = S16(RECT + 8u * (uint32_t)s + 4u);                        /* 0x71EEB */
        h = S16(RECT + 8u * (uint32_t)s + 6u);                        /* 0x71EA0 */
    }
    fg_vbpaste(x, x + w - 1, y, y + h - 1, x, y + h - 1);             /* 0x71F17 */
    fg_vbclose();                                                     /* 0x71F1F */
}

/* ---- 0x6FE68 / 0x6FFC4 the POINTS box --------------------------------------------------------------------------- */

void f6FE68(int x, int y, int n, int show, int color)
{
    char buf[20];   /* [ebp-0x24] */

    f194E4(n);                                                        /* 0x6FE87 */
    fg_vbcopy(x, x + 0x31, y - 10, y + 0x27, x, y + 0x27, S32(BACKGROUND), S32(BACKBUF));   /* 0x6FEBE */
    fg_vbopen(S32(BACKBUF));                                          /* 0x6FECD */
    fg_setcolor(color);                                               /* 0x6FED9 [ebp+8] */
    f39F90(1);                                                        /* 0x6FEE6 */
    f3A0B0(x, y - 10, 0x32, DSTR(0xB8FE9u));                          /* 0x6FEFE "POINTS" */
    snprintf(buf, sizeof buf, "%d", f195D8(n));                       /* 0x6FF0E, 0x6FF13 itoa(.., 10) */
    f3A0B0(x, y, 0x32, buf);                                          /* 0x6FF26 */
    if (n != 0) {                                                     /* 0x6FF2B */
        f3AF8C(TOKEN23(n), x, y + 10);                                /* 0x6FF45 */
        snprintf(buf, sizeof buf, "%d", S8(NATION(n) + 0x3E));        /* 0x6FF62 dword [199n + 0xF1507] sar 24 */
        f3A044(x + 0x1B, y + 0x14, buf);                              /* 0x6FF76 */
    }
    fg_vbclose();                                                     /* 0x6FF7B */
    if (show == 1) {                                                  /* 0x6FF80 */
        f78884();                                                     /* 0x6FF86 */
        fg_vbpaste(x, x + 0x31, y - 10, y + 0x27, x, y + 0x27);       /* 0x6FFAF */
        f788AC();                                                     /* 0x6FFB7 */
    }
}

void f6FFC4(int x, int y, int n, int show)
{
    (void)n;                                                          /* ebx [ebp-0xC] is never read */
    fg_vbcopy(x, x + 0x31, y - 10, y + 0x27, x, y + 0x27, S32(BACKGROUND), S32(BACKBUF));   /* 0x70013 */
    if (show == 1) {                                                  /* 0x7001B */
        fg_vbopen(S32(BACKBUF));                                      /* 0x70028 */
        fg_vbclose();                                                 /* 0x70030 */
        f78884();                                                     /* 0x70035 */
        fg_vbpaste(x, x + 0x31, y - 10, y + 0x27, x, y + 0x27);       /* 0x7005E */
        f788AC();                                                     /* 0x70066 */
    }
}

/* ---- 0x6FBDC REVEAL GOODS ---------------------------------------------------------------------------------------- */

void f6FBDC(int n, int disable)
{
    int      x, y, bx;          /* [ebp-0x24], [ebp-0x20], [ebp-0x1C] */
    uint32_t saved = 0;         /* [ebp-0x14] (written only when disable == 1) */
    int      r;                 /* [ebp-0x18] */

    if ((D8(FLAGS) & PBEM_BIT) != 0)                                  /* 0x6FBF6 */
        return;
    if (f76F14(n) <= 0)                                               /* 0x6FC06; jle */
        return;
    if (S8(HUMANS) <= 1)                                              /* 0x6FC13 dword [0xE161D] sar 24; jle */
        return;
    x = 0xF5;                                                         /* 0x6FC24 */
    y = 0xBE;                                                         /* 0x6FC2B */
    bx = x + 0x14;                                                    /* 0x6FC38 */
    f78884();                                                         /* 0x6FC3B */
    junk_makegif(x, x + 0x95, y, y + 0x63, DSTR(0xB8FC3u));           /* 0x6FC5D fg_makegif(.., "junk") (port: junk.c) */
    f788AC();                                                         /* 0x6FC65 */
    if (disable == 1) {                                               /* 0x6FC6D */
        saved = D32(CUR_DIALOG);                                      /* 0x6FC72 */
        f3C940(saved);                                                /* 0x6FC7A */
    }
    fg_vbopen(S32(BACKBUF));                                          /* 0x6FC86 */
    f3C9D0(x, y, 0x96, 0x64, 3);                                      /* 0x6FCA0 */
    fg_setcolor(5);                                                   /* 0x6FCA7 */
    f3A0B0(x, y + 0x14, 0x96, NAME(n));                               /* 0x6FCC7 */
    fg_vbclose();                                                     /* 0x6FCCC */
    f3BB48(BOX_DLG, 0x2A, bx);                                        /* 0x6FCDE */
    f3BC64(BOX_DLG, 0x2A, y + 0x3C);                                  /* 0x6FCF3 */
    f3C284(BOX_DLG, 0x2A, 0xB8FC8u);                                  /* 0x6FD07 "REVEAL GOODS" */
    f3B05C(BOX_DLG);                                                  /* 0x6FD11 */
    f78884();                                                         /* 0x6FD16 */
    fg_vbpaste(x, x + 0x95, y, y + 0x63, x, y + 0x63);                /* 0x6FD3E */
    f788AC();                                                         /* 0x6FD46 */
    do
        r = f78BA4(BOX_DLG, 0);                                       /* 0x6FD52 */
    while (r == -100 || r != -1);                                     /* 0x6FD5D, 0x6FD65 */
    if (disable == 1) {                                               /* 0x6FD6A */
        setD32(CUR_DIALOG, saved);                                    /* 0x6FD73 */
        f3C8C4(saved);                                                /* 0x6FD78 */
    }
    f3C284(BOX_DLG, 0x2A, 0xB8FD5u);                                  /* 0x6FD8C "NOT REVEAL GDS" */
    f78884();                                                         /* 0x6FD91 */
    fg_move(x, y);                                                    /* 0x6FD9E */
    junk_showgif(DSTR(0xB8FE4u), 3);                                  /* 0x6FDAD fg_showgif("junk", 3) (port: junk.c) */
    f3CCBC();                                                         /* 0x6FDB5 */
    f788AC();                                                         /* 0x6FDBA */
}

/* ---- 0x6D010 / 0x6D0C4 the hand strip ---------------------------------------------------------------------------- */

void f6D010(int x, int y, int w, int n)
{
    fg_vbcopy(x, x + w - 1, y, y + 0x2D, x, y + 0x2D, S32(BACKGROUND), S32(BACKBUF));   /* 0x6D05D */
    fg_vbopen(S32(BACKBUF));                                          /* 0x6D06C */
    f71F2C(x, y, w, n);                                               /* 0x6D080 */
    fg_vbclose();                                                     /* 0x6D085 */
    f78884();                                                         /* 0x6D08A */
    fg_vbpaste(x, x + w - 1, y, y + 0x2D, x, y + 0x2D);               /* 0x6D0B1 */
    f788AC();                                                         /* 0x6D0B9 */
}

void f6D0C4(int x, int y, int w, int n)
{
    fg_vbcopy(x, x + w - 1, y, y + 0x2D, x, y + 0x2D, S32(BACKGROUND), S32(BACKBUF));   /* 0x6D111 */
    fg_vbopen(S32(BACKBUF));                                          /* 0x6D120 */
    f71FB0(x, y, w, n);                                               /* 0x6D134 */
    fg_vbclose();                                                     /* 0x6D139 */
    f78884();                                                         /* 0x6D13E */
    fg_vbpaste(x, x + w - 1, y, y + 0x2D, x, y + 0x2D);               /* 0x6D165 */
    f788AC();                                                         /* 0x6D16D */
}

/* the strip's x and w by the player count (0x6C2DE.., 0x6D30E.., 0x6D9C7..: signed compares with 8) */
static int strip_x(void) { return S32(PLAYERS_N) < 8 ? 7 : 0xD1; }
static int strip_w(void) { return S32(PLAYERS_N) < 8 ? 0x1F4 : 0x172; }

/* ---- 0x6CF50 the treasury box ------------------------------------------------------------------------------------ */

void f6CF50(int n)
{
    char buf[20];   /* [ebp-0x24] */

    fg_vbopen(S32(BACKBUF));                                          /* 0x6CF6D */
    f3AF8C(TOKEN23(n), 0x203, 0x168);                                 /* 0x6CF8A */
    fg_setcolor(5);                                                   /* 0x6CF91 */
    snprintf(buf, sizeof buf, "%d", S8(NATION(n) + 0x3E));            /* 0x6CFB1 dword [199n + 0xF1507] sar 24 */
    f71E34(1);                                                        /* 0x6CFC3 (its fg_vbclose leaves the screen active) */
    f3A044(0x222, 0x170, buf);                                        /* 0x6CFCD edx = 0x170, ebx = buf kept by f71E34 */
    fg_vbclose();                                                     /* 0x6CFD2 */
    f78884();                                                         /* 0x6CFD7 */
    fg_vbpaste(0x1FC, 0x269, 0x162, 0x184, 0x1FC, 0x184);             /* 0x6CFFA */
    f788AC();                                                         /* 0x6D002 */
}

/* ---- 0x6CB8C DrawInitialDrawingScreen ----------------------------------------------------------------------------- */

void f6CB8C(void)
{
    f3CD6C(S8(PICTURE));                                              /* 0x6CBA6 dword [0xEE165] sar 24 */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BACKGROUND), S32(BACKBUF));   /* 0x6CBCE */
    fg_vbopen(S32(BACKBUF));                                          /* 0x6CBDD */
    setD16(0x11158Cu, (uint16_t)(S32(PLAYERS_N) < 8 ? 7 : 0xD1));     /* 0x6CC07 the hand strip, slot 13 */
    setD16(0x11158Eu, 0x196);                                         /* 0x6CC0D */
    setD16(0x111590u, (uint16_t)(S32(PLAYERS_N) < 8 ? 0x1F4 : 0x172));   /* 0x6CC3E */
    setD16(0x111592u, 0x2E);                                          /* 0x6CC49 */
    setD16(0x111524u, 0xEC);                                          /* 0x6CC50 slot 0: the drawn cards */
    setD16(0x111526u, 0x62);                                          /* 0x6CC57 */
    setD16(0x111528u, 0x15F);                                         /* 0x6CC5E */
    setD16(0x11152Cu, 0x222);                                         /* 0x6CC65 slot 1: the treasury number */
    setD16(0x11152Au, 0x70);                                          /* 0x6CC85 */
    setD16(0x11152Eu, 0x170);                                         /* 0x6CC90 */
    setD16(0x111530u, 0x1E);                                          /* 0x6CC97 */
    setD16(0x111532u, 0x14);                                          /* 0x6CC9E */
    setD16(0x111534u, 0x253);                                         /* 0x6CCA5 slot 2 */
    setD16(0x111538u, 0x28);                                          /* 0x6CCAC */
    setD16(0x11153Au, 0x14);                                          /* 0x6CCB3 */
    setD16(0x111536u, 0x127);                                         /* 0x6CCD3 */
    setD16(0x11153Cu, 0xFA);                                          /* 0x6CCDE slot 3: the title */
    setD16(0x11153Eu, 0x1B);                                          /* 0x6CCE5 */
    setD16(0x111542u, 0x19);                                          /* 0x6CCEC */
    setD16(0x111540u, 0x96);                                          /* 0x6CCF7 */
    /* 0x6CCFE 0x78B0C("DrawInitialDrawingScreen"): the no-op logger */
    f3A18C(0, 8, 0x280, 0xB0, DSTR(0xB8C27u));                        /* 0x6CD0F "Acquire Goods" (edx 8, ebx 0x280 from the
                                                                         stores at 0x6CCCE / 0x6CCC9) */
    fg_setcolor(0xD);                                                 /* 0x6CD16 */
    f3A0B0(0x3A, 0x15, 0x20, DSTR(0xB8C35u));                         /* 0x6CD32 "Stored" */
    f3A0B0(0x5F, 0x15, 0x20, DSTR(0xB8C3Cu));                         /* 0x6CD4B "Acqrd" */
    f3A0B0(0x84, 0x15, 0x20, DSTR(0xB8C42u));                         /* 0x6CD64 "Buy" */
    f3A0B0(0xA9, 0x15, 0x20, DSTR(0xB8C46u));                         /* 0x6CD7D "Total" */
    fg_vbclose();                                                     /* 0x6CD82 */
    f3B05C(ACQ_DLG);                                                  /* 0x6CD8C */
    f3CC14();                                                         /* 0x6CD91 */
}

/* ---- 0x6CDA0 the left column -------------------------------------------------------------------------------------- */

void f6CDA0(int p)
{
    char buf[52];   /* [ebp-0x50] */
    int  r, i, n;   /* [ebp-0x18] the row, [ebp-0x1C] the list position, [ebp-0x14] */

    r = 0;                                                            /* 0x6CDB8 */
    for (i = 0; S16(PLAYERS) > i; i++) {                              /* 0x6CDBB, 0x6CEA3 dword [0xF06AC] sar 16; jle */
        n = S16(PLAYER_LIST + 2u * (uint32_t)i);                      /* 0x6CEB7 dword [2i + 0xF06AE] sar 16 */
        if (D8(NATION(n) + 0x9A) == 0)                                /* 0x6CECB */
            continue;
        f3AF8C(TOKEN24(n), (0x35 - (int)D16(TOKEN24(n) + 4u)) / 2, ROW(r) + 0x2B);   /* 0x6CF16 (signed /2) */
        f39F90(1);                                                    /* 0x6CF20 */
        fg_setcolor(i == p ? 0xF : 5);                                /* 0x6CF28, 0x6CDC9 */
        f3A0B0(0, ROW(r) + 0x2B + (int)D16(TOKEN24(n) + 6u) + 2, 0x35, NAME(n));   /* 0x6CE09 word [8n + 0xF4AD2] */
        fg_setcolor(i == p ? 0xF : 5);                                /* 0x6CE11, 0x6CE23 */
        f3AF8C(CARD_BACK, 0x3A, ROW(r) + 0x24);                       /* 0x6CE49 */
        snprintf(buf, sizeof buf, "%d", S8(HAND_SIZE + (uint32_t)i)); /* 0x6CE62 dword [i + 0x110ACD] sar 24 */
        f39F90(2);                                                    /* 0x6CE6C */
        f3A0B0(0x3B, ROW(r) + 0x32, 0x1E, buf);                       /* 0x6CE92 */
        r++;                                                          /* 0x6CE9A */
    }
    f39F90(1);                                                        /* 0x6CF40 */
}

/* ---- 0x6D178 ACQUIRE ---------------------------------------------------------------------------------------------- */

void f6D178(int p, int n)
{
    char    buf[72];      /* [ebp-0x60] */
    int32_t maxw;         /* [ebp-0x18] */
    int     cities;       /* [ebp-0x24] */
    int     s, j;         /* [ebp-0x20], [ebp-0x1C] */
    int8_t  card;         /* [ebp-0x14] */

    f3CD6C(S8(PICTURE));                                              /* 0x6D19A */
    cities = S8(NATION(n) + 0x51);                                    /* 0x6D1AC dword [199n + 0xF151A] sar 24 */
    f78884();                                                         /* 0x6D1B2 */
    f71E34(0);                                                        /* 0x6D1B9 */
    fg_setcolor(5);                                                   /* 0x6D1C0 */
    f3A044(0xF6, 0x62, DSTR(0xB8C4Cu));                               /* 0x6D1D7 "You acquired..." */
    f788AC();                                                         /* 0x6D1DC */
    setD32(ACQUIRED, 0);                                              /* 0x6D1E5 */
    fg_setcolor(0xF);                                                 /* 0x6D1EB */
    for (s = 0; s < cities; s++) {                                    /* 0x6D1F5, 0x6D203 jge */
        for (j = 0; j < S32(PER_CITY); j++) {                         /* 0x6D20E, 0x6D28D jge */
            card = (int8_t)f76E50(s, n);                              /* 0x6D29F; byte [ebp-0x14] */
            if (card == 0) {                                          /* 0x6D2AB */
                snprintf(buf, sizeof buf, "Stack %d Empty", s + 1);   /* 0x6D221 (0xB8C5C) */
                f78884();                                             /* 0x6D229 */
                f3A42C(0xEC + COL(s), 50 * j + 0x73, 0x22, 2, buf, &maxw, 1);   /* 0x6D270 */
                f788AC();                                             /* 0x6D275 */
            } else {
                f78884();                                             /* 0x6D2B3 */
                f3AF8C(CARDTINY(card), 0xEC + COL(s), 50 * j + 0x6E); /* 0x6D2F1 */
                f788AC();                                             /* 0x6D2F6 */
                setD32(ACQUIRED, D32(ACQUIRED) + 1u);                 /* 0x6D2FB */
            }
            f78A20(50);                                               /* 0x6D27F */
        }
    }
    f6E460(n);                                                        /* 0x6D309 */
    f6D0C4(strip_x(), 0x196, strip_w(), n);                           /* 0x6D342 */
    f6FE68(0x249, 0x19F, n, 1, 0xF);                                  /* 0x6D35B */
    f3AF8C(CARD_BACK, 0x5F, ROW(p) + 0x24);                           /* 0x6D37E */
    f39F90(2);                                                        /* 0x6D390 */
    snprintf(buf, sizeof buf, "%d", S32(ACQUIRED));                   /* 0x6D39A itoa(.., 10) */
    fg_setcolor(0xF);                                                 /* 0x6D3A1 */
    f3A0B0(0x60, ROW(p) + 0x32, 0x1E, buf);                           /* 0x6D3CA */
    f39F90(1);                                                        /* 0x6D3D4 */
    f78884();                                                         /* 0x6D3D9 */
    f71E34(2);                                                        /* 0x6D3E8 */
    f788AC();                                                         /* 0x6D3ED */
    f3C530(ACQ_DLG, 0x8A);                                            /* 0x6D3F7 edx = 0x8A from 0x6D3E3 */
    if (S16(BUY_CODE + 2u * (uint32_t)n) == -0x61) {                  /* 0x6D3FF dword [2n + 0xF0682] sar 16 (PBEM) */
        s = 8;                                                        /* 0x6D412 */
        card = (int8_t)f76E50(s, n);                                  /* 0x6D41F */
        if (card != 0 && f6DB00(n) != 0) {                            /* 0x6D42B, 0x6D436 (the card drawn first:
                                                                         kept unpaid when f6DB00 fails, see above) */
            f7983C(DSTR(0xB8C6Bu));                                   /* 0x6D44D "View Good Bought" */
            f3C530(ACQ_DLG, 0x8B);                                    /* 0x6D457 edx = 0x8B from 0x6D448 */
            f78884();                                                 /* 0x6D45C */
            fg_setcolor(5);                                           /* 0x6D463 */
            f3A044(0xF6, 0xA0, DSTR(0xB8C7Cu));                       /* 0x6D47A "You bought..." */
            f788AC();                                                 /* 0x6D47F */
            fg_setcolor(0xF);                                         /* 0x6D486 */
            setD32(BOUGHT, D32(BOUGHT) + 1u);                         /* 0x6D495 */
            f78884();                                                 /* 0x6D49B */
            f3AF8C(CARDTINY(card), 0x161, 0xA0);                      /* 0x6D4B6 */
            f788AC();                                                 /* 0x6D4BB */
            setD8(NATION(n) + 0x3C, (uint8_t)(D8(NATION(n) + 0x3C) + 1u));   /* 0x6D4C7 */
            setD8(NATION(n) + 0x3E, (uint8_t)(D8(NATION(n) + 0x3E) - 0x12u));   /* 0x6D4D4 treasury */
            setD8(NATION(n) + 0x99, (uint8_t)(D8(NATION(n) + 0x99) + 0x12u));   /* 0x6D4E2 stock */
            setD32(BOUGHT_FROM + 4u * (uint32_t)n, 8);                /* 0x6D4EC */
        }
    } else if (S16(BUY_CODE + 2u * (uint32_t)n) == -0x62 || f6DB00(n) == 0) {   /* 0x6D506, 0x6D50E */
        f3C530(ACQ_DLG, 0x8B);                                        /* 0x6D521 */
    } else {
        f3C48C(ACQ_DLG, 0x8B);                                        /* 0x6D532 */
        if ((D8(FLAGS) & PBEM_BIT) != 0)                              /* 0x6D53D */
            setD16(0xF5478u, 1);                                      /* 0x6D542 */
    }
    f3C48C(ACQ_DLG, 0x2A);                                            /* 0x6D555 */
}

/* ---- 0x6D564 BUY -------------------------------------------------------------------------------------------------- */

void f6D564(int p, int n)
{
    char    buf[80];      /* [ebp-0x64] */
    int32_t maxw;         /* [ebp-0x14] */
    int     s = 0;        /* [ebp-0x18] the stack bought from (set before any read where a card was bought) */
    int     max;          /* [ebp-0x1C] */
    int     i;            /* [ebp-0x20] */
    int     x;            /* [ebp-0x24] */
    int     r;            /* [ebp-0x28] */
    int8_t  card;         /* [ebp-0x10] */
    uint32_t rec = NATION(n);

    f3CD6C(S8(PICTURE));                                              /* 0x6D585 */
    f71E34(0);                                                        /* 0x6D58C */
    fg_setcolor(5);                                                   /* 0x6D593 */
    if ((D16(FLAGS) << 10 & 0xFFFFu) >> 15 == 0) {                    /* 0x6D598..0x6D5AD bit 0x20: buy from any stack */
        f78884();                                                     /* 0x6D5B3 */
        fg_setcolor(5);                                               /* 0x6D5BA */
        f3A044(0xF6, 0x62, DSTR(0xB8C8Au));                           /* 0x6D5D1 "You bought..." */
        f788AC();                                                     /* 0x6D5D6 */
        fg_setcolor(0xF);                                             /* 0x6D5DD */
        s = 8;                                                        /* 0x6D5EA */
        card = (int8_t)f76E50(s, n);                                  /* 0x6D5F3 */
        if (!g_original && card == 0) {
            /* BUG(1995) 4: an empty stack 8 gave card 0, counted and paid 18; fixed: nothing bought (see above) */
            card = -1;
        } else {
            setD32(BOUGHT, D32(BOUGHT) + 1u);                         /* 0x6D5FB */
            f78884();                                                 /* 0x6D601 */
            f3AF8C(CARDTINY(card), 0x113, 0x6E);                      /* 0x6D61C */
            f788AC();                                                 /* 0x6D621 */
            setD8(rec + 0x3C, (uint8_t)(D8(rec + 0x3C) + 1u));        /* 0x6D62D */
            if ((D8(FLAGS) & PBEM_BIT) != 0)                          /* 0x6D639 */
                setD16(0xF5478u, 1);                                  /* 0x6D63E */
            setD32(BOUGHT_FROM + 4u * (uint32_t)n, 8);                /* 0x6D64A */
        }
    } else {
        f78884();                                                     /* 0x6D65A */
        fg_setcolor(0xF);                                             /* 0x6D661 */
        f3A044(0xF6, 0x62, DSTR(0xB8C98u));                           /* 0x6D67D "Choose stack to buy from..." */
        max = 8;                                                      /* 0x6D682 */
        for (i = 0; i < 9; i++) {                                     /* 0x6D687, 0x6D72A jge 0x6D7F4 */
            if (S16(STACK(i)) != 0 && S8(rec + 0x3E) >= 2 * i + 2) {  /* 0x6D74A, 0x6D76C jl (dword [rec+0x3B] sar 24) */
                fg_setcolor(0xF);                                     /* 0x6D774 */
                f39F90(2);                                            /* 0x6D781 */
                f3AF8C(CARD_BACK, 0xEC + COL(i), 0x6E);               /* 0x6D7A7 */
                snprintf(buf, sizeof buf, "%d", i + 1);               /* 0x6D7B8 itoa(i + 1, .., 10) */
                f3A0B0(0xEC + COL(i), 0x7C, 0x20, buf);               /* 0x6D7E0 */
                f39F90(1);                                            /* 0x6D7EA */
                continue;
            }
            if (S8(rec + 0x3E) < 2 * i + 2) {                         /* 0x6D6A7 jge 0x6D6B6 */
                max = i;                                              /* 0x6D6AE the first stack it cannot pay */
                break;
            }
            if (S16(STACK(i)) == 0) {                                 /* 0x6D6CC */
                fg_setcolor(0xF);                                     /* 0x6D6D2 */
                snprintf(buf, sizeof buf, "Stack %d Empty", i + 1);   /* 0x6D6E8 (0xB8CB4) */
                f3A42C(0xEC + COL(i), 0x73, 0x22, 2, buf, &maxw, 1);  /* 0x6D71F */
            }
        }
        f788AC();                                                     /* 0x6D7F4 */
        setD8(rec + 0x3C, (uint8_t)(D8(rec + 0x3C) + 1u));            /* 0x6D800 counted before the choice */
        if ((D8(FLAGS) & PBEM_BIT) != 0)                              /* 0x6D806 */
            setD16(0xF5478u, 1);                                      /* 0x6D80F */
        f3C48C(ACQ_DLG, 0x8C);                                        /* 0x6D822 */
        f3C530(ACQ_DLG, 0x8B);                                        /* 0x6D836 */
        f3C284(ACQ_DLG, 0x2A, 0xB8CC3u);                              /* 0x6D845 "CANCEL" (ebx from 0x6D831) */
        f3C48C(ACQ_DLG, 0x2A);                                        /* 0x6D856 */
        card = -1;                                                    /* 0x6D85B cl = -1 */
        for (;;) {
            r = f78BA4(ACQ_DLG, 0);                                   /* 0x6D865 */
            if (r == -100)                                            /* 0x6D870 */
                continue;
            if (r == -1) {                                            /* 0x6D878: CANCEL */
                if (!g_original)   /* BUG(1995): +0x3C stayed counted after CANCEL (0x6D800) */
                    setD8(rec + 0x3C, (uint8_t)(D8(rec + 0x3C) - 1u));
                break;
            }
            if (r != -0x34)                                           /* 0x6D884 */
                continue;
            s = -100;                                                 /* 0x6D889 */
            x = 0xEC;                                                 /* 0x6D890 */
            for (i = 0; i <= max; i++) {                              /* 0x6D899, 0x6D8A7 jg */
                if (S16(STACK(i)) != 0 && S32(MOUSE_X) >= x && x + 0x20 >= S32(MOUSE_X)   /* 0x6D8C4, 0x6D8D0, 0x6D8E0 */
                    && (g_original || S8(rec + 0x3E) >= 2 * i + 2))   /* BUG(1995) 2: the first unaffordable stack was
                                                                         clickable (the loop runs to i <= max) */
                    s = i;                                            /* 0x6D8E5 */
                x += 0x27;                                            /* 0x6D8E8 */
                if (s != -100)                                        /* 0x6D8EC */
                    break;
            }
            if (!g_original && s == -100)   /* BUG(1995) 1: a click between the columns bought from "stack" -100 */
                continue;
            card = (int8_t)f76E50(s, n);                              /* 0x6D8F8 */
            setD32(BOUGHT_FROM + 4u * (uint32_t)n, (uint32_t)s);      /* 0x6D906 */
            break;
        }
        f3C530(ACQ_DLG, 0x8C);                                        /* 0x6D917 */
        f78884();                                                     /* 0x6D91C */
        f71E34(0);                                                    /* 0x6D923 */
        if (card != -1) {                                             /* 0x6D92C */
            setD32(BOUGHT, D32(BOUGHT) + 1u);                         /* 0x6D939 */
            fg_setcolor(5);                                           /* 0x6D93E */
            f3A044(0xF6, 0x62, DSTR(0xB8CCAu));                       /* 0x6D955 "You bought..." */
            f3AF8C(CARDTINY(card), 0x113, 0x6E);                      /* 0x6D970 */
        }
        f788AC();                                                     /* 0x6D975 */
    }
    if (card != -1) {                                                 /* 0x6D97A */
        uint8_t cost = (uint8_t)(2u * (uint8_t)((uint8_t)s + 1u));    /* 0x6D98E..0x6D999 dl = s + 1, add dl, dl */
        setD8(rec + 0x3E, (uint8_t)(D8(rec + 0x3E) - cost));          /* 0x6D99D treasury */
        setD8(rec + 0x99, (uint8_t)(D8(rec + 0x99) + cost));          /* 0x6D9B1 stock */
        f6CF50(n);                                                    /* 0x6D9BA */
        f6E460(n);                                                    /* 0x6D9C2 */
        f6D0C4(strip_x(), 0x196, strip_w(), n);                       /* 0x6D9FB */
        f6FE68(0x249, 0x19F, n, 1, 0xF);                              /* 0x6DA14 */
        f3AF8C(CARD_BACK, 0x84, ROW(p) + 0x24);                       /* 0x6DA37 */
        f39F90(2);                                                    /* 0x6DA49 */
        snprintf(buf, sizeof buf, "%d", S32(BOUGHT));                 /* 0x6DA53 itoa(.., 10) */
        fg_setcolor(0xF);                                             /* 0x6DA5A */
        f3A0B0(0x85, ROW(p) + 0x32, 0x1E, buf);                       /* 0x6DA83 */
        f39F90(1);                                                    /* 0x6DA8D */
    }
    f78884();                                                         /* 0x6DA92 */
    f71E34(2);                                                        /* 0x6DA9C */
    f788AC();                                                         /* 0x6DAA1 */
    if (f6DB00(n) != 0)                                               /* 0x6DAA9 */
        f3C48C(ACQ_DLG, 0x8B);                                        /* 0x6DABC */
    else
        f3C530(ACQ_DLG, 0x8B);                                        /* 0x6DACD */
    f3C284(ACQ_DLG, 0x2A, 0xB8CD8u);                                  /* 0x6DAE1 "FINISHED" */
    f3C48C(ACQ_DLG, 0x2A);                                            /* 0x6DAF0 */
}

/* ---- 0x6BD98 ACQUIRE GOODS (DrawTradingCards) ---------------------------------------------------------------------- */

/* the row's three counters at row p: "Acqrd" [0x110AC8], "Buy" [0x110ACC], the hand (0x6C40E..0x6C548, the same
   sequence at 0x6C6FA..0x6C9C9 for the computer): font 2, colour 5, card backs at 0x5F / 0x84 / 0xA9 */
static void row_counters(int p, int n)
{
    char buf[100];   /* [ebp-0xB8] */

    f39F90(2);                                                        /* 0x6C413 / 0x6C6FF */
    fg_setcolor(5);                                                   /* 0x6C41A / 0x6C706 */
    f3AF8C(CARD_BACK, 0x5F, ROW(p) + 0x24);                           /* 0x6C440 / 0x6C72C */
    snprintf(buf, sizeof buf, "%d", S32(ACQUIRED));                   /* 0x6C455 / 0x6C741 */
    f3A0B0(0x60, ROW(p) + 0x32, 0x1E, buf);                           /* 0x6C47E / 0x6C76A */
    f3AF8C(CARD_BACK, 0x84, ROW(p) + 0x24);                           /* 0x6C4A1 / 0x6C78D */
    snprintf(buf, sizeof buf, "%d", S32(BOUGHT));                     /* 0x6C4B6 / 0x6C7A2 */
    f3A0B0(0x85, ROW(p) + 0x32, 0x1E, buf);                           /* 0x6C4DF / 0x6C7CB */
    f3AF8C(CARD_BACK, 0xA9, ROW(p) + 0x24);                           /* 0x6C502 / 0x6C7EE */
    snprintf(buf, sizeof buf, "%d", f76F14(n));                       /* 0x6C51A / 0x6C806 */
    f3A0B0(0xAA, ROW(p) + 0x32, 0x1E, buf);                           /* 0x6C543 / 0x6C9BF */
    f39F90(1);                                                        /* 0x6C54D / 0x6C9C9 */
}

void f6BD98(void)
{
    char     buf[100];       /* [ebp-0xB8] */
    uint8_t  pairs[9][2];    /* [ebp-0x5C]: {cities, nation}, 8 pairs; pairs[8] stands for the bytes of the cities
                                local [ebp-0x4C] that a ninth player would overlay (port: never reached, <= 8 players) */
    uint8_t  tmp[2];         /* [ebp-0x1C] (a word) */
    int      i, j, k;        /* [ebp-0x30], [ebp-0x48], [ebp-0x44] */
    int      count;          /* [ebp-0x48] in the first loop */
    int      total;          /* [ebp-0x2C] the cities of all nations in play */
    int      cities;         /* [ebp-0x4C] */
    int      n;              /* [ebp-0x3C] */
    int      p;              /* [ebp-0x34] the list position */
    int      any_human;      /* [ebp-0x24] */
    int      r;              /* [ebp-0x20] */
    int      card;           /* [ebp-0x40] */
    int      again;          /* byte [ebp-0x18] */
    uint32_t rec;            /* [ebp-0x28] */

    for (i = 0; i < 8; i++) {                                         /* 0x6BDB2..0x6BDD3 */
        pairs[i][0] = 0;                                              /* 0x6BDC8 */
        pairs[i][1] = 0;                                              /* 0x6BDCF */
    }
    pairs[8][0] = pairs[8][1] = 0;                                    /* port: the overlay's bytes (see above) */
    count = 0;                                                        /* 0x6BDD7 */
    total = 0;                                                        /* 0x6BDDA */
    for (n = 1; n <= 9; n++) {                                        /* 0x6BDDD, 0x6BDEC jg */
        rec = NATION(n);                                              /* 0x6BE00 */
        if (D8(rec + 0x9A) == 0)                                      /* 0x6BE06 */
            continue;
        cities = S8(rec + 0x51);                                      /* 0x6BE16 dword [rec+0x4E] sar 24 */
        total += cities;                                              /* 0x6BE1F */
        pairs[count][0] = (uint8_t)cities;                            /* 0x6BE28 (count <= 8: at most 8 players) */
        pairs[count][1] = (uint8_t)n;                                 /* 0x6BE32 */
        count++;                                                      /* 0x6BE39 */
    }
    if (total == 0) {                                                 /* 0x6BE41 */
        f79270(100, S16(PLAYER_LIST), -200, DSTR(0xB8BF4u), 0);       /* 0x6BE5E (the list of the last phase; "") */
        return;
    }
    f553C0();                                                         /* 0x6BE68 */
    if ((D8(FLAGS) & PBEM_BIT) != 0)                                  /* 0x6BE6D */
        pbem_dropped(0x6BE7Au);                                       /* the snapshots [0xEED78] / [0xEED80] */
    /* 0x6BF2B: the pairs sorted ascending by (cities, nation), signed bytes */
    for (i = (S32(PLAYERS_N) > 9 ? 9 : S32(PLAYERS_N)) - 1; i >= 0; i--)   /* 0x6BF31, 0x6BF48 (port: at most the 9 pairs) */
        for (j = 0; j < i; j++)                                       /* 0x6BF40, 0x6BF65 jge */
            if ((int8_t)pairs[i][0] < (int8_t)pairs[j][0]             /* 0x6BF7A jl */
                || ((int8_t)pairs[i][0] == (int8_t)pairs[j][0]        /* 0x6BF8A jne */
                    && (int8_t)pairs[i][1] < (int8_t)pairs[j][1])) {  /* 0x6BF9A jge */
                memcpy(tmp, pairs[i], 2);                             /* 0x6BFA4 */
                memcpy(pairs[i], pairs[j], 2);                        /* 0x6BFB2 */
                memcpy(pairs[j], tmp, 2);                             /* 0x6BFBD */
            }
    setD16(PLAYERS, D16(PLAYERS_N));                                  /* 0x6BFCC word [0xF40EC] */
    for (k = 0; k < S32(PLAYERS_N) && k < 9; k++)                     /* 0x6BFD2, 0x6BFE0 jge (port: k < 9) */
        setD16(PLAYER_LIST + 2u * (uint32_t)k, (uint16_t)(int16_t)(int8_t)pairs[k][1]);   /* 0x6BFF4 movsx ax, byte */
    f783B8(ACQ_DLG, S32(0xF5960u));                                   /* 0x6C00B */
    for (p = 0; S16(PLAYERS) > p; p++)                                /* 0x6C010, 0x6C01B jle */
        setD8(HAND_SIZE + (uint32_t)p, (uint8_t)f76F14(S16(PLAYER_LIST + 2u * (uint32_t)p)));   /* 0x6C03D */
    setD32(PER_CITY, ((D16(FLAGS) << 9 & 0xFFFFu) >> 15) + 1u);       /* 0x6C045..0x6C05E bit 0x40: 2 goods per city */
    setD8(PICTURE, (uint8_t)((S32(PLAYERS_N) >= 8) + 0xA));           /* 0x6C063 setge; add al, 0xA */
    f6CB8C();                                                         /* 0x6C072 DrawInitialDrawingScreen */
    any_human = 0;                                                    /* 0x6C077 */
    for (p = 0; S16(PLAYERS) > p; p++) {                              /* 0x6C07A, 0x6C165 jle 0x6C9DD */
        n = S16(PLAYER_LIST + 2u * (uint32_t)p);                      /* 0x6C179 */
        rec = NATION(n);                                              /* 0x6C194 */
        setD8(NATION(n) + 0x3C, 0);                                   /* 0x6C1A0 */
        setD32(CUR_NATION, (uint32_t)n);                              /* 0x6C1A9 */
        setD32(BOUGHT_FROM + 4u * (uint32_t)n, 0xFFFFFFFFu);          /* 0x6C1B6 */
        if (D8(NATION(n) + 0x9A) != 0 && S8(NATION(n) + 0x51) == 0) { /* 0x6C1CA, 0x6C1DE */
            f6CDA0(-1);                                               /* 0x6C1E4 */
            fg_setcolor(5);                                           /* 0x6C1EB */
            f39F90(2);                                                /* 0x6C1FD */
            for (k = 1; k < 4; k++) {                                 /* 0x6C202, 0x6C0DA jge */
                f3AF8C(CARD_BACK, 37 * k + 0x3A, ROW(p) + 0x24);      /* 0x6C10D */
                if (k >= 3) {                                         /* 0x6C115 */
                    snprintf(buf, sizeof buf, "%d", f76F14(n));       /* 0x6C090, 0x6C095 itoa */
                    f3A0B0(37 * k + 0x3B, ROW(p) + 0x32, 0x1E, buf);  /* 0x6C0CF */
                } else {
                    f3A0B0(37 * k + 0x3B, ROW(p) + 0x32, 0x1E, DSTR(0xB8C06u));   /* 0x6C0CF ecx = "0" (0x6C141) */
                }
            }
            f39F90(1);                                                /* 0x6C150 */
            f78A20(500);                                              /* 0x6C15A */
            continue;
        }
        if ((S8(NATION(n) + 0x9A) == 3 && (D8(FLAGS) & PBEM_BIT) == 0)          /* 0x6C21A, 0x6C21F */
            || ((D8(FLAGS) & PBEM_BIT) != 0 && S8(LOCAL_PBEM) == n)) {          /* 0x6C228, 0x6C23D */
            /* 0x6C246: a human */
            any_human = 1;                                            /* 0x6C246 */
            f3C530(ACQ_DLG, 0x8B);                                    /* 0x6C257 */
            f3C530(ACQ_DLG, 0x2A);                                    /* 0x6C266 */
            f6CF50(n);                                                /* 0x6C26E */
            f6CDA0(p);                                                /* 0x6C276 */
            f39F90(2);                                                /* 0x6C280 */
            fg_setcolor(5);                                           /* 0x6C287 */
            f78884();                                                 /* 0x6C28F */
            f71E34(3);                                                /* 0x6C299 */
            f3A18C(0, 0x1E, 0x280, 0x23, NAME(n));                    /* 0x6C2BA */
            f788AC();                                                 /* 0x6C2BF */
            f6FFC4(0x249, 0x19F, n, 1);                               /* 0x6C2D6 */
            f6D010(strip_x(), 0x196, strip_w(), n);                   /* 0x6C312 */
            f78884();                                                 /* 0x6C317 */
            f71E34(0);                                                /* 0x6C31E */
            f788AC();                                                 /* 0x6C323 */
            f6FBDC(n, 1);                                             /* 0x6C330 REVEAL GOODS */
            f3C48C(ACQ_DLG, 0x8A);                                    /* 0x6C33F */
            f6D0C4(strip_x(), 0x196, strip_w(), n);                   /* 0x6C37A */
            f6FE68(0x249, 0x19F, n, 1, 0xF);                          /* 0x6C393 */
            setD32(BOUGHT, 0);                                        /* 0x6C3A0 */
            if ((D8(FLAGS) & PBEM_BIT) != 0 && S16(0xF5478u) == 1     /* 0x6C3A6, 0x6C3AB dword [0xF5476] sar 16 */
                && S8(NATION(n) + 0x51) == 9 && S16(STACK(8)) != 0)   /* 0x6C3C8, 0x6C3CD dword [0xF0796] sar 16 */
                f6E158();                                             /* 0x6C3D9 (PBEM) */
            for (;;) {
                f3CD6C(S8(PICTURE));                                  /* 0x6C3E8 */
                r = f78BA4(ACQ_DLG, 0);                               /* 0x6C3F2 edx = 0 from 0x6C3E6 */
                if (r == -100)                                        /* 0x6C3FD */
                    continue;
                if (r == -1) {                                        /* 0x6C405: FINISHED */
                    row_counters(p, n);                               /* 0x6C40E..0x6C54D */
                    break;
                }
                if (r == -0x32)                                       /* 0x6C55A ACQUIRE */
                    f6D178(p, n);                                     /* 0x6C565 */
                else if (r == -0x33)                                  /* 0x6C572 BUY */
                    f6D564(p, n);                                     /* 0x6C581 */
            }
            continue;
        }
        /* 0x6C58B: a computer (or, in PBEM, a remote player) */
        f3C940(D32(CUR_DIALOG));                                      /* 0x6C590 */
        f6CDA0(-1);                                                   /* 0x6C59A */
        f78884();                                                     /* 0x6C59F */
        f71E34(3);                                                    /* 0x6C5A9 */
        f3A18C(0, 0x1E, 0x280, 0x23, NAME(n));                        /* 0x6C5CA */
        f71E34(0);                                                    /* 0x6C5D1 */
        f788AC();                                                     /* 0x6C5D6 */
        f6DBAC(p, n);                                                 /* 0x6C5E1 [M7] */
        f6D010(strip_x(), 0x196, strip_w(), n);                       /* 0x6C61E */
        f78A20(1500);                                                 /* 0x6C628 */
        if ((D8(FLAGS) & PBEM_BIT) != 0 && S16(BUY_CODE + 2u * (uint32_t)n) != -0x61) {   /* 0x6C633, 0x6C64E */
            pbem_dropped(0x6C827u);                                   /* the pre-declared codes, "0" / "?" */
            continue;
        }
        if (S16(BUY_CODE + 2u * (uint32_t)n) == -0x61) {              /* 0x6C664 */
            card = f76E50(8, n);                                      /* 0x6C671 */
            if (card != 0) {                                          /* 0x6C67C */
                setD32(BOUGHT, 1);                                    /* 0x6C680 */
                setD32(BOUGHT_FROM + 4u * (uint32_t)n, 8);            /* 0x6C68D */
                setD8(rec + 0x3E, (uint8_t)(D8(rec + 0x3E) - 0x12u)); /* 0x6C69B */
                setD8(rec + 0x99, (uint8_t)(D8(rec + 0x99) + 0x12u)); /* 0x6C6A2 */
            }
        } else {
            int bought = 0;   /* port: the purchases counted (BUG(1995) 5) */
            again = 1;                                                /* 0x6C6AB */
            do {
                int got = f6DE04(n) != 0;                             /* 0x6C6B2 [M7]; setne */
                bought += got;
                /* BUG(1995) 5: the counter held only the last call's result; fixed: the purchases */
                setD32(BOUGHT, (uint32_t)(g_original ? got : bought));   /* 0x6C6C1 */
                if (!got || S8(rec + 0x3E) < 6)                       /* 0x6C6C6, 0x6C6D8 jge (dword [199n + 0xF1507]
                                                                         sar 24) */
                    again = 0;                                        /* 0x6C6DF */
            } while (again != 0);                                     /* 0x6C6E5 */
        }
        f78884();                                                     /* 0x6C6E9 */
        f71E34(0);                                                    /* 0x6C6F0 */
        f788AC();                                                     /* 0x6C6F5 */
        row_counters(p, n);                                           /* 0x6C6FA..0x6C9C9 */
        f3C8C4(D32(CUR_DIALOG));                                      /* 0x6C9D3 */
    }
    f6CDA0(-1);                                                       /* 0x6C9E2 */
    if ((D8(FLAGS) & PBEM_BIT) != 0 && any_human == 0)                /* 0x6C9E7, 0x6C9F4 */
        pbem_dropped(0x6C9FEu);                                       /* the wait for FINISHED (remote players) */
    if ((D8(FLAGS) & PBEM_BIT) != 0 && D16(0xF5478u) != 0)            /* 0x6CA91, 0x6CA9E */
        pbem_dropped(0x6CAACu);                                       /* the stacks and nations restored */
    if ((D8(FLAGS) & PBEM_BIT) != 0)                                  /* 0x6CB54 */
        pbem_dropped(0x6CB5Du);                                       /* the snapshots freed */
}

/* ---- 0x6EC84 the discard screen ----------------------------------------------------------------------------------- */

void f6EC84(int n)
{
    f3CD6C(S8(PICTURE));                                              /* 0x6ECA3 */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BACKGROUND), S32(BACKBUF));   /* 0x6ECCB */
    fg_vbopen(S32(BACKBUF));                                          /* 0x6ECDA */
    f3A18C(0, 8, 0x280, 0xB0, DSTR(0xB8E19u));                        /* 0x6ECF8 "Discard Excess Goods" */
    f3A18C(0, 0x1E, 0x280, 0x23, NAME(n));                            /* 0x6ED19 */
    fg_setcolor(0xD);                                                 /* 0x6ED20 */
    f3A044(0x46, 0xB9, DSTR(0xB8E2Eu));                               /* 0x6ED37 "Choose goods to discard ..." */
    f3A044(0x50, 0x10E, DSTR(0xB8E4Au));                              /* 0x6ED4B "Remaining to discard:" */
    fg_vbclose();                                                     /* 0x6ED50 */
    f3C0D8(DISC_DLG, 0x2A, 2);                                        /* 0x6ED64 FINISHED: style 2 (no label) */
    f3BFBC(DISC_DLG, 0x2A, 0);                                        /* 0x6ED75 disabled */
    f3BFBC(DISC_DLG, 0x8D, 1);                                        /* 0x6ED89 the hand enabled */
    f3B05C(DISC_DLG);                                                 /* 0x6ED93 */
    f3CC14();                                                         /* 0x6ED98 */
}

/* ---- 0x6E870 the discards ----------------------------------------------------------------------------------------- */

/* "Remaining to discard:" count, hand - 8 (0x6EB2A..0x6EBD0 and 0x6E9C2..0x6EA5C) */
static void remaining(int n)
{
    char buf[12];   /* [ebp-0x40] */

    fg_vbcopy(0xCC, 0xDF, 0x10E, 0x121, 0xCC, 0x121, S32(BACKGROUND), S32(BACKBUF));   /* 0x6EB63 / 0x6E9EE */
    snprintf(buf, sizeof buf, "%d", f76F14(n) - 8);                   /* 0x6EB7E / 0x6EA09 itoa(.., 10) */
    fg_vbopen(S32(BACKBUF));                                          /* 0x6EB89 / 0x6EA15 */
    fg_setcolor(0xD);                                                 /* 0x6EB93 / 0x6EA1F */
    f3A044(0xCC, 0x10E, buf);                                         /* 0x6EBA8 / 0x6EA34 */
    fg_vbclose();                                                     /* 0x6EBAD / 0x6EA39 */
    fg_vbpaste(0xCC, 0xDF, 0x10E, 0x121, 0xCC, 0x121);                /* 0x6EBD0 / 0x6EA5C */
}

/* port: 1 if nation n holds any commodity card (the guard of BUG(1995) 6) */
static int holds_commodity(int n)
{
    int i;
    for (i = 1; i <= 0x12; i++)
        if (S8(NATION(n) + (uint32_t)i) > 0)
            return 1;
    return 0;
}

void f6E870(void)
{
    int      n;            /* [ebp-0x30] */
    int      r;            /* [ebp-0x2C] */
    int      sel;          /* [ebp-0x28] */
    int      x;            /* [ebp-0x24] */
    int      w;            /* [ebp-0x20] */
    int      i, j;         /* [ebp-0x34], [ebp-0x1C] */
    int      c;
    uint32_t rec;          /* [ebp-0x18] */

    setD32(SPACING1, 0x28);                                           /* 0x6E896 */
    setD32(SPACING2, 0xA);                                            /* 0x6E89C */
    setD8(PICTURE, 0x11);                                             /* 0x6E8A2 */
    for (n = 1; n <= 9; n++) {                                        /* 0x6E8A8, 0x6EA6A jg */
        if (D8(NATION(n) + 0x9A) == 0)                                /* 0x6EA81 */
            continue;
        if (f76F14(n) <= 8)                                           /* 0x6EA90 jle */
            continue;
        if (!((S8(NATION(n) + 0x9A) == 3 && (D8(FLAGS) & PBEM_BIT) == 0)          /* 0x6EAA2, 0x6EAA7 */
              || ((D8(FLAGS) & PBEM_BIT) != 0 && S8(LOCAL_PBEM) == n))) {         /* 0x6EAB0, 0x6EAC5 */
            if (S8(NATION(n) + 0x9A) == 2)                            /* 0x6EC59 */
                f6EDA8(n);                                            /* 0x6EC61 [M7] */
            else
                setD16(0xF4096u, 1);                                  /* 0x6EC6B */
            continue;
        }
        f6EC84(n);                                                    /* 0x6EAD1 */
        f6D010(0x32, 0xC8, 0x1F4, n);                                 /* 0x6EAE8 */
        f6FBDC(n, 1);                                                 /* 0x6EAF5 */
        f6D0C4(0x32, 0xC8, 0x1F4, n);                                 /* 0x6EB0C */
        f6FE68(0x249, 0xD1, n, 1, 0xF);                               /* 0x6EB25 */
        for (;;) {                                                    /* 0x6EB2A */
            if (!g_original && !holds_commodity(n))
                break;   /* BUG(1995) 6: a hand over 8 with no commodity left could never end (M6_PLAN risk 6) */
            f3CD6C(S8(PICTURE));                                      /* 0x6EB32 */
            remaining(n);                                             /* 0x6EB37..0x6EBD5 */
            r = f78BA4(DISC_DLG, 0);                                  /* 0x6EBDF */
            if (r == -100)                                            /* 0x6EBEA */
                continue;
            if (r != -0x35)                                           /* 0x6EBF6 */
                continue;
            setD16(0xF4096u, 1);                                      /* 0x6EC09 */
            sel = -100;                                               /* 0x6EC10 */
            x = 0x32;                                                 /* 0x6EC13 */
            rec = NATION(n);                                          /* 0x6EC28 */
            w = S32(SPACING1);                                        /* 0x6EC2B */
            if (w >= 0x20)                                            /* 0x6EC30 jl */
                w = 0x20;                                             /* 0x6EC35 */
            for (i = 1; i <= 0x12; i++) {                             /* 0x6EC3D, 0x6E8B6 jg */
                c = S8(rec + (uint32_t)i);                            /* 0x6E8C6 movsx */
                if (c == 1) {                                         /* 0x6E8C9 */
                    if (S32(MOUSE_X) >= x && x + w >= S32(MOUSE_X))   /* 0x6E8D8 jl, 0x6E8EA jl */
                        sel = i;                                      /* 0x6E8EF */
                    x += S32(SPACING1);                               /* 0x6E8F2, 0x6E95A */
                } else if (c != 0) {                                  /* 0x6E904 */
                    for (j = 0; c > j; j++) {                         /* 0x6E908, 0x6E91F jle */
                        if (S32(MOUSE_X) >= x && x + w >= S32(MOUSE_X))   /* 0x6E92B, 0x6E93D */
                            sel = i;                                  /* 0x6E942 */
                        x += S32(SPACING2);                           /* 0x6E94A */
                    }
                    x += S32(SPACING1) - S32(SPACING2);               /* 0x6E954, 0x6E95A */
                }
                if (sel != -100)                                      /* 0x6E95D */
                    break;
            }
            if (sel == -100)                                          /* 0x6E967 */
                continue;
            f771F8(sel);                                              /* 0x6E974 */
            setD8(rec + (uint32_t)sel, (uint8_t)(D8(rec + (uint32_t)sel) - 1u));   /* 0x6E97F */
            f6D0C4(0x32, 0xC8, 0x1F4, n);                             /* 0x6E993 */
            f6FE68(0x249, 0xD1, n, 1, 0xF);                           /* 0x6E9AC */
            if (f76F14(n) > 8)                                        /* 0x6E9BC jg 0x6EB2A */
                continue;
            remaining(n);                                             /* 0x6E9C2..0x6EA61 */
            break;
        }
    }
}

/* ---- 0x6E770 DISCARD EXCESS --------------------------------------------------------------------------------------- */

void f6E770(void)
{
    int n;      /* [ebp-0x18] */
    int over;   /* [ebp-0x14] */

    setD16(PLAYERS, 0);                                               /* 0x6E786 */
    for (n = 1; n <= 9; n++) {                                        /* 0x6E78D, 0x6E79C jg */
        if (D8(NATION(n) + 0x9A) == 0)                                /* 0x6E7A9 */
            continue;
        {
            int16_t c = S16(PLAYERS);                                 /* 0x6E7B3 movsx edx, ax */
            setD16(PLAYERS, (uint16_t)(c + 1));                       /* 0x6E7BD */
            setD16(PLAYER_LIST + 2u * (uint32_t)(int32_t)c, (uint16_t)n);   /* 0x6E7C6 */
        }
    }
    f783B8(DISC_DLG, S32(0xF5964u));                                  /* 0x6E7DD */
    over = 0;                                                         /* 0x6E7E2 */
    for (n = 1; n <= 9; n++) {                                        /* 0x6E7E5, 0x6E7F4 jg */
        if (D8(NATION(n) + 0x9A) == 0)                                /* 0x6E801 */
            continue;
        if (f76F14(n) > 8) {                                          /* 0x6E80E, 0x6E813 jle */
            over = 1;                                                 /* 0x6E818 */
            break;
        }
    }
    if (over != 0)                                                    /* 0x6E822 */
        f6E870();                                                     /* 0x6E826 */
    else
        f79270(100, S16(PLAYER_LIST), -200, DSTR(0xB8E18u), 0);       /* 0x6E846 dword [0xF06AE] sar 16; "" */
    if ((D8(FLAGS) & PBEM_BIT) == 0)                                  /* 0x6E84B */
        f77278();                                                     /* 0x6E854 */
    else
        setD16(0xF4098u, 1);                                          /* 0x6E85B */
}

/* ---- 0x77278 / 0x77510 the reshuffle ------------------------------------------------------------------------------ */

int f77510(int s)
{
    uint32_t pile = PILE(s);   /* [ebp-0x4] */
    int      found = 0;        /* [ebp-0xC] */
    int      idx = 0;          /* [ebp-0x10] */
    int      i, j, c;          /* [ebp-0x18], [ebp-0x14], [ebp-0x8] */

    for (i = 0; S16(pile) > i; i++) {                                 /* 0x77548, 0x77559 jle */
        c = S8(pile + (uint32_t)i + 6u);                              /* 0x77567 dword [pile + i + 3] sar 24 */
        for (j = 1; j < 5; j++)                                       /* 0x7756D, 0x7757C jge */
            if (S8(0xCA9E0u + (uint32_t)j) == c) {                    /* 0x7758B dword [j + 0xCA9DD] sar 24 */
                idx = i;                                              /* 0x77598 */
                found = 1;                                            /* 0x7759B */
                break;
            }
        if (found != 0)                                               /* 0x775A2 */
            break;
    }
    if (found != 0) {                                                 /* 0x775A8 */
        for (i = idx; S16(pile) - 1 > i; i++)                         /* 0x775B1, 0x775C3 jle */
            setD8(pile + (uint32_t)i + 6u, D8(pile + (uint32_t)i + 7u));   /* 0x775D7 */
        setD16(pile, (uint16_t)(D16(pile) - 1u));                     /* 0x775DF dec word */
    }
    return found;                                                     /* 0x775E2 */
}

void f77278(void)
{
    int s;       /* [ebp-0x18] */
    int found;   /* [ebp-0x14] */

    for (s = 0; s < 9; s++) {                                         /* 0x7728E, 0x77384 jge */
        if (S16(PILE(s)) != 0) {                                      /* 0x773A4 dword [25s + 0xF07B2] sar 16 */
            if (s < 1 || s > 4) {                                     /* 0x773AE jl, 0x773B8 jg */
                f76F70(S16(PILE(s)), DP(PILE(s) + 6u));               /* 0x772C7 */
                f77184(S16(PILE(s)), DP(PILE(s) + 6u), S16(STACK(s)), DP(STACK(s) + 6u));   /* 0x7732E */
                setD16(STACK(s), (uint16_t)(D16(STACK(s)) + D16(PILE(s))));   /* 0x7735F add ecx, eax (16 bits) */
            } else {
                found = f77510(s) != 0;                               /* 0x773C1; setne */
                f76F70(S16(PILE(s)), DP(PILE(s) + 6u));               /* 0x77404 */
                f77184(S16(PILE(s)), DP(PILE(s) + 6u), S16(STACK(s)), DP(STACK(s) + 6u));   /* 0x7746B */
                setD16(STACK(s), (uint16_t)(D16(STACK(s)) + D16(PILE(s))));   /* 0x77495 add word */
                if (found != 0) {                                     /* 0x7749C */
                    f77184(1, DP(0xCA9E0u + (uint32_t)s), S16(STACK(s)), DP(STACK(s) + 6u));   /* 0x774E6 */
                    setD16(STACK(s), (uint16_t)(D16(STACK(s)) + 1u)); /* 0x774F7 inc word */
                }
            }
        }
        setD16(PILE(s), 0);                                           /* 0x77376 */
    }
}
