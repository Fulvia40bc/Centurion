/* Module TRADE of docs/M6_PLAN.md, the human side: the trade steps of the game loop (DECLARE OFFERS, COUNTER OFFERS,
   ACCEPT TRADES), the offer composer, the trade itself and its screens. Literal translations; the address of each
   instruction a line comes from is in its comment. D = read in the code, G = inference. Prototypes: trade.h.

     0x6F270  f6F270  DECLARE OFFERS (phase 12 of the game loop, call at 0x54D02)                 484 bytes
     0x6F454  f6F454  the Declare Trade Offers screen                                           1,288
     0x6F95C  f6F95C  the commodity chart                                                         248
     0x6FA54  f6FA54  the Declare screen's labels                                                 216
     0x6FB2C  f6FB2C  the response screen's labels                                                176
     0x6FDC8  f6FDC8  a hand strip redrawn                                                        160
     0x70074  f70074  a card into the current group                                               544
     0x70294  f70294  the current group cleared (CLEAR SECTION)                                 1,292
     0x707A0  f707A0  the scratch hand (record 0) rebuilt                                         484
     0x70984  f70984  a card out of record 0                                                      232
     0x70A6C  f70A6C  switch the composer's group                                               1,584
     0x7109C  f7109C  the composer's click loop                                                 1,568
     0x716BC  f716BC  a calamity out of a nation record                                           172
     0x71768  f71768  DONE: the offer checked                                                   1,024
     0x71B68  f71B68  the offer-error box                                                         456
     0x721A0  f721A0  a hand face up with frames                                                  572
     0x723DC  f723DC  COUNTER OFFERS (phase 13, call at 0x54DC2)                                  768
     0x72928  f72928  the Make Counter Offers screen                                            1,484
     0x72EF4  f72EF4  one counter-offer                                                           208
     0x72FC4  f72FC4  the Respond To Offer screen                                               1,312
     0x734E4  f734E4  ACCEPT TRADES (phase 14, call at 0x54E2D)                                 1,208
     0x73D4C  f73D4C  the response list of a nation                                               720
     0x7401C  f7401C  the Accept Trades screen                                                  3,384
     0x74D54  f74D54  the response rows' buttons reset                                            684
     0x75000  f75000  the scroll buttons                                                          116
     0x75074  f75074  a trade carried out                                                       1,196
     0x75520  f75520  the Trade Details screen                                                  1,276
     0x75A1C  f75A1C  the trade detail block                                                      440
     0x75FB4  f75FB4  the Trade Summary screen                                                    812
   Ported elsewhere and called here (never re-translated): 0x75BD4 0x75C08 0x75DF0 0x75EB4 0x75F08 (helpers2.c), 0x75CA8
   0x75D84 0x6E460 (rules.c), 0x6FBDC 0x6FE68 0x71D30 0x71E34 (cards.c), 0x71F2C 0x71FB0 0x76F14 0x194E4 0x195D8
   (info.c), 0x726DC (saveload.c), 0x76F70 (newgame.c), 0x783B8 (panel.c), 0x795A8 0x3B914 0x3C1F4 0x448EC (kit.c).
   0x70984 and 0x716BC are near-twins of rules.c's 0x75CA8 (M6_PLAN item 15), translated separately.

   Not ported here (each call site stops the port with not_ported(address); the static stubs below carry the names, so
   the call sites read as in the original and a later translation that exports one of these names collides with its
   stub at compile time instead of being shadowed by it). Built with -DTRADE_TEST (port/test/trade_test.c) they go to
   the test's recorders instead, which return.
     M7 (TRADEAI):  void f68730(int n)  a computer declares its offer (0x6F32B)
                    void f698A4(int n)  a computer answers the offers (0x724E8)
                    void f6B0EC(int n)  a computer accepts a response (0x73887; it calls f75074 back)
                    (0x68788, 0x68A64, 0x68B44, 0x68C90, 0x68FDC, 0x6937C, 0x6A5FC, 0x6AD70, 0x6AF34 are reached only
                    through these three: no call site here)
     PBEM (dropped by Erik's ruling, PBEM_DROPPED):
                    void f53C30(void)   the turn order sent to the host (0x726CC)
                    void f7399C(int i, int pref)  a preference radio of the Accept screen (0x7375F.. 0x73845)
   So today an all-human game runs the three steps to the end; a computer holding the goods stops at its TRADEAI routine.
   The PBEM tests of bit 0x80 of byte [0xE1624] and the local-player byte [0xE1622] are kept literally (as cards.c
   does): only the calls into PBEM routines stop.

   game_rand f78AE4 (D: no direct call in these 29 functions): only through f726DC (0x72473, the offer records'
   shuffles), f76F70 at 0x72692 (the turn order: one draw per player) and at 0x735E7 (each counter-offer row's first
   three cards, unless variant 0x20 of [0xE1625]; one draw per card counted), and the computers' routines (M7).

   Watcom runtime -> standard C:
     0x7DB3D sprintf -> snprintf bounded to the stack buffer: 100 bytes in 0x6F454 ([ebp-0x84]..[ebp-0x20]), 0x72928,
             0x72FC4, 0x7401C, 0x75520; 52 in 0x75FB4 ([ebp-0x60]..[ebp-0x2C]); 100 for 0x71768's sprintf temporary and
             200 for its message ([ebp-0x174]..[ebp-0xAC]); the inline strcpy / strcat copies of 0x71768 -> cat()
     0x8C897 itoa(v, buf, 10) -> snprintf "%d" (0x6F547)
     0x7D5D0 memset -> memset on the data object; the rep movsd / movsw / movsb record copies -> memcpy / memmove
     0x8CE50 fg_makegif / 0x88F98 fg_showgif "junk" -> junk_makegif / junk_showgif (junk.h: kept in memory)
   __CHK dropped. Register-carried arguments (D): f3A18C's colour in ecx is set before sprintf and kept by it (0xB0 for
   the titles, 0x23 for the nation names), as cards.c and adv.c read it.

   Data (D, read in the code at the addresses given):
     nation record n: 0xF14CC + 199n (rules.h): +0x01..+0x12 commodities, +0x13 calamities, +0x14.. ids, +0x1C..
       givers; +0x3D the nation's own number; +0x9A 0 out / 2 computer / 3 human; +0x9C..+0xA8 the declared offer
       (13 bytes, by the slot table: +0x9C/+0x9D truly stated, +0x9E.. bluffs, +0xA1..+0xA5 wanted, +0xA6.. the cards
       really given for the bluffs); +0xB1 + m the computer's trust in m (-15..100); +0xBD + m 1 = this nation
       passed a calamity to m (D, 0x7524C / 0x752DA: the giver's record gets the mark; when m passes a calamity on to
       p, every other computer's mark on m moves to p, 0x75285..0x752B5); +0xBE..+0xC6 cleared in session 1 for
       computers. Record 0 is the scratch hand.
     slot-group table 0xCA9A0 (int8, one array read across its parts): start1[k] 0xCA9A0+k = 0, 2, 5, 10 (offset in
       the nation's +0x9C..); count[k] 0xCA9A4+k = 2, 1, 5, 1, 1; start2[k] 0xCA9AC+k = 0, 2, 5, 10, 5 (the slot
       rectangle, and the offset in a counter-offer row); int16 0xCA9B2+2k = 0, 1, 1, 0, 0 (hand drawn with frames).
       Groups k: 0 truly stated, 1 bluff, 2 wanted, 3 bluff replacement (own offer), 4 bluff replacement (counter).
       Several reads cross the parts (k = 4 or -1, the byte 0xCA9A8, 0xCA9B0): read here as bytes of one array.
     slot rectangles int16 0x111524 + 8s {x, y, w, h}, s 0..12; s 13 = 0x11158C the hand strip (cards.h)
     int32 [0x110EC8] composer mode: 0 own offer (dialog 0x111594), 1 counter-offer (0x111D6C); int32 [0x112558] the
       current group (-1 none), [0x112554] the new one; int16 [0x112542] bluff slots (1); int16 [0x112548] the
       counter-offer rows made; [0x11254C] / [0x112550] the hand's card steps (info.h)
     counter-offer rows 0x111118 + 16r: +0..+1 truly stated, +2..+4 bluffs, +5..+7 replacements, +8..+0xC the shuffled
       copy shown to the offerer (0x734E4), +0xD the offerer, +0xE the responder, +0xF the offer's +0x13
     public offer records 0x110ECC + 21i (count [0x112560], saveload.c's f726DC): +0..+0xC the offer, +0xD..+0x11 its
       shown cards (shuffled), +0x12 the nation, +0x13, +0x14 the last nation that answered it (-1 none)
     response list 0x111498 + 20i (count int16 [0x112568]): +0..+0xF a copy of the row, int16 +0x10 the gain, +0x12 the
       row, +0x13 the PBEM preference
     trade log: int8 [0x110F74] trades, 0x110F75 + 5t the offerer's cards, 0x111015 + 5t the responder's, 0x1110B5[t]
       / 0x1110D5[t] the two nations, 0x1110F5[t] the session; detail block 0x112570 + 9s (s 0 offerer, 1 responder):
       +0..+1 truly stated, +2..+4 replacements, +5..+7 bluffs, int8 +8 1 honest / -1 bluffed
     int32 0x1105AC[n] the number of cards n wants (G: read by the computers); byte 0xF09D4[n] 1 = another round
     int16 [0xF5476] the session (dword [0xF5474] sar 16); byte [0xE1623] low nibble sessions, high nibble minimum
     int16 [0xF40C2] the dead "show all hands" switch (OVERVIEW §6.19); int16 [0xF4100] 0 reject all / 1 PBEM order
     dialogs: 0x111594 Declare (0x70..0x73 groups -0x11..-0x14, 0x75 hand -0x1D, 0x76 chart -0x1C, 0x7E CLEAR
       SECTION -0x25, 0x7F CLEAR ALL -0x7A, 0x2A FINISHED -1); 0x111980 Counter Offers (rows 0x80.. -0x26.., 0x85 PREV
       -0x2B, 0x86 NEXT -0x2C); 0x111D6C Respond (0x70, 0x71, 0x74 -0x15, 0x75, 0x76, 0x7E, 0x7F, 0x2A RESPONSE
       COMPLETE); 0x112158 Accept (0x87..0x89 ACCEPT -0x2F..-0x31, radios 0xEC + 3j + row, players 0x12.. -0x36..);
       0xBB800 CONTINUE; 0xBBBEC the box's CONTINUE

   1995 bugs, fixed unless g_original (each marked BUG(1995) in the code; grades code / effect):
     1. 0x71768 own offer (D / D): the message buffer [ebp-0x174] is written only when the offer is incomplete; a
        complete offer with a bluff but no replacement appends "You must declare a bluff replacement." to stack garbage.
        The port cannot reproduce the garbage: both modes start from an empty text (no mode difference; for the
        DOSBox-X check, a frame of this box shows what the original prints).
     2. 0x71768 counter-offer (D / G): the want count [ebp-0x24] is never set in mode 1, yet every accepted
        counter-offer stores it into 0x1105AC[n] (in the save; the computers compare it, 0x6B1D1). The port cannot
        reproduce the garbage: --original stores 0; fixed: mode 1 leaves 0x1105AC[n] (n's own offer's count).
     3. 0x7109C chart click (D / G): the card is 2 * row + column with no range check, and the chart button 0x76
        reaches y 448 while the 9th row ends at 446: a click on its last two pixel rows (y 447..448) gives card 19 or
        20, a calamity, as a bluff or a wanted card. Fixed: such a click is ignored.
     4. 0x70984 / 0x716BC (D / G): a calamity not in the record leaves the start index [ebp-0x8] unwritten; the shift
        starts from stack garbage and the count drops anyway. --original starts at index 0 (as rules.c's f75CA8);
        fixed: the record stays. G: unreachable, the cards placed come from the same hand.
     5. 0x75074 (D / G): the offerer's public record is searched by nation and the index [ebp-0x34] stays unwritten if
        none matches. --original uses index 0; fixed: the log gets no cards for the offerer. G: unreachable, every
        nation with an offer has a record (f726DC).
     6. Text (Erik's ruling of 19 Nov, a missing article): "The responses to %s offer..." with the nation adjective
        (0xB92A3 / 0xC76C4) reads "The responses to African offer..."; fixed: "The responses to the African offer..."
        ("your" stays: "The responses to your offer...").
   Kept as the original (D code, G effect unless marked):
     - 0x73D4C clears 7 response records (memset 0x8C) with no bound: with 9 players an offer can get 8 responses and the
       8th record lands on the slot rectangles 0, 1 and the x, y of 2 (0x111524..0x111537), which the next screen
       rewrites (D: 0x6F454, 0x72FC4 write them before any read).
     - 0x734E4 counts the cards to shuffle with the count as index (row + 8 + count), so it counts leading cards; the
       same number whenever the filled slots come first (always, G: f71768 requires both truly stated cards).
     - 0x75520 (Trade Details) shows "%s was bluffing with" and the partner's declared bluffs even for an honest
       partner (G: design).
     - 0x7401C enables the preference radios 0xEF.. (style 2, invisible) without PBEM; their codes do nothing there.
     - 0x734E4's player buttons show another nation's responses read-only (f74D54 disables ACCEPT, 0x7401C enables it
       only for [0xF40F8]); the INVENTORY's suspicion that ACCEPT could take another nation's response is refuted.
     - 0x75074 marks a passed calamity only from the first replacement slot (+2): one bluff slot exists (D).
     - 0x721A0's fit loop can drive the big step negative (the loop ends because it keeps falling).
     - 0x71B68 has no unattended check (f7BA64): only a human's offer reaches it.

   Easter eggs and oddities: the "Replace your bluffs with..." label exists in six copies (0xB8ED5, 0xB8F1D, 0xB8F65,
   0xB8F94, 0xB901C, 0xB904B), "Truly stated" and "Bluff" in six each, all drawn as a colour-5 shadow then colour 7;
   "N O N E" letter-spaced (M6_PLAN egg 24); the dead "show all hands" switch [0xF40C2] lets a computer's own offer be
   seen on the Accept screen and skips the 2-second view (OVERVIEW §6.19); "If Not Lying" heads the gains; the
   computers' trust (+0xB1) moves by 20 / 15 / 10 by AI level (0xCA994) times the partner's honesty, clamped to
   -15..100; "Truly stated" (composer) vs "Truly Stated" (Accept screen) left as style. New (D): the question "Do you
   want another round of trading?" (0x725E2) is asked only with PBEM or with bit 0x80 of aciv.cfg's first byte
   [0xF097C], which no code sets (the option toggles 0x43A73..0x44718 write only 0x01, 0x02, 0x04, 0x08, 0x10, 0x40;
   start-up clears 0x20 at 0x107FB and 0x05 at 0x1081F): a dead option, reachable by editing aciv.cfg; without it
   every human gets 0xF09D4[n] = 1 silently. */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cards.h"
#include "fg.h"
#include "game.h"
#include "helpers2.h"
#include "info.h"
#include "junk.h"
#include "kit.h"
#include "panel.h"
#include "rules.h"
#include "saveload.h"
#include "state.h"
#include "trade.h"
#include "ui.h"

#define NATION(n)    (0xF14CCu + 199u * (uint32_t)(n))
#define NAME(n)      DSTR(D32(0xC768Cu + 4u * (uint32_t)(n)))
#define ADJ(n)       DSTR(D32(0xC76C4u + 4u * (uint32_t)(n)))
#define CARD(c)      (0xF4C1Cu + 8u * (uint32_t)(int32_t)(c))
#define TOKEN(n)     (0xF4ACCu + 8u * (uint32_t)(n))
#define TOKEN_W(n)   ((int32_t)D16(0xF4AD0u + 8u * (uint32_t)(n)))
#define TOKEN_H(n)   ((int32_t)D16(0xF4AD2u + 8u * (uint32_t)(n)))
#define CARD_BACK    0xF4D14u
#define CARD_EMPTY   0xF4D1Cu   /* cardtiny 32 */
#define CARD_LIT     0xF4D24u   /* cardtiny 33 */
#define SLOT_X(s)    ((int32_t)S16(0x111524u + 8u * (uint32_t)(s)))   /* dword [8s + 0x111522] sar 16 */
#define SLOT_Y(s)    ((int32_t)S16(0x111526u + 8u * (uint32_t)(s)))   /* dword [8s + 0x111524] sar 16 */
#define START1(k)    ((int32_t)S8(0xCA9A0u + (uint32_t)(k)))          /* dword [k + 0xCA99D] sar 24 */
#define COUNT(k)     ((int32_t)S8(0xCA9A4u + (uint32_t)(k)))          /* dword [k + 0xCA9A1] sar 24 */
#define START2(k)    ((int32_t)S8(0xCA9ACu + (uint32_t)(k)))          /* dword [k + 0xCA9A9] sar 24 */
#define FLAG16(k)    ((int32_t)S16(0xCA9B2u + 2u * (uint32_t)(k)))    /* dword [2k + 0xCA9B0] sar 16 */
#define ROW(r)       (0x111118u + 16u * (uint32_t)(r))
#define OFFER(i)     (0x110ECCu + 21u * (uint32_t)(i))
#define RESP(i)      (0x111498u + 20u * (uint32_t)(i))
#define FLAGS        0xE1624u
#define FLAGS2       0xE1625u
#define PBEM_BIT     0x80u
#define HUMANS       0xE1620u   /* int8: dword [0xE161D] sar 24 */
#define LOCAL_PBEM   0xE1622u   /* int8: dword [0xE161F] sar 24 */
#define SETUP_TRADE  0xE1623u   /* low nibble sessions, high nibble the minimum goods */
#define CFG0         0xF097Cu
#define CUR_NATION   0xF40F8u
#define PLAYERS_N    0xF40ECu   /* int32 */
#define PLAYERS      0xF06AEu   /* int16 count */
#define PLAYER_LIST  0xF06B0u   /* int16[] */
#define SESSION      0xF5476u   /* int16: dword [0xF5474] sar 16 */
#define SHOW_ALL     0xF40C2u   /* int16 */
#define SRD          0xF40BAu   /* int16 */
#define REJECT_ALL   0xF4100u   /* int16 */
#define ANOTHER      0xF09D4u   /* uint8[n] */
#define WANTS        0x1105ACu  /* int32[n] */
#define BACKBUF      0xF52C8u
#define BACKGROUND   0xF52CCu
#define CUR_DIALOG   0xF8E78u
#define PICTURE      0xEE168u   /* int8: dword [0xEE165] sar 24 */
#define MODE         0x110EC8u  /* int32 */
#define GROUP        0x112558u  /* int32 */
#define NEW_GROUP    0x112554u  /* int32 */
#define NBLUFF       0x112542u  /* int16: dword [0x112540] sar 16 */
#define SHOWN        0x112546u  /* int16: dword [0x112544] sar 16 */
#define CROW         0x112548u  /* int16: dword [0x112546] sar 16 */
#define NRESP        0x112568u  /* int16: dword [0x112566] sar 16 */
#define BIG          0x11254Cu  /* int32 */
#define SMALL        0x112550u  /* int32 */
#define NOFFERS      0x112560u  /* int32 */
#define CSCROLL      0x11255Cu  /* int32 */
#define ASCROLL      0x112564u  /* int32 */
#define OFFER_IDX    0x11256Cu  /* int32 */
#define TRADES       0x110F74u  /* int8: dword [0x110F71] sar 24 */
#define DETAIL       0x112570u
#define DECL_DLG     0x111594u
#define COUNTER_DLG  0x111980u
#define RESPOND_DLG  0x111D6Cu
#define ACCEPT_DLG   0x112158u
#define CONT_DLG     0xBB800u
#define BOX_DLG      0xBBBECu

/* ---- calls into code that is not ported (see the file comment) --------------------------------------------------- */

#ifdef TRADE_TEST
void trade_test_f68730(int n);
void trade_test_f698A4(int n);
void trade_test_f6B0EC(int n);
void trade_test_pbem(uint32_t addr, int a, int b);
#define f68730(n)      trade_test_f68730(n)
#define f698A4(n)      trade_test_f698A4(n)
#define f6B0EC(n)      trade_test_f6B0EC(n)
#define f53C30()       trade_test_pbem(0x53C30u, 0, 0)
#define f7399C(i, p)   trade_test_pbem(0x7399Cu, i, p)
#else
static void f68730(int n)                       /* M7 (TRADEAI): a computer declares its offer */
{
    (void)n;
    not_ported(0x68730u);
}

static void f698A4(int n)                       /* M7 (TRADEAI): a computer answers the offers */
{
    (void)n;
    not_ported(0x698A4u);
}

static void f6B0EC(int n)                       /* M7 (TRADEAI): a computer accepts a response */
{
    (void)n;
    not_ported(0x6B0ECu);
}

static void f53C30(void)                        /* PBEM_DROPPED: the turn order to the host */
{
    not_ported(0x53C30u);
}

static void f7399C(int i, int pref)             /* PBEM_DROPPED: a preference radio */
{
    (void)i; (void)pref;
    not_ported(0x7399Cu);
}
#endif

/* ---- small shared pieces ------------------------------------------------------------------------------------------ */

/* the inline strcat copies (repne scasb, then the byte-pair loop), bounded to the caller's stack buffer */
static void cat(char *buf, size_t size, const char *s)
{
    size_t n = strlen(buf);
    if (n + 1 < size)
        snprintf(buf + n, size - n, "%s", s);
}

/* the bluff shift: 20 * bit 13 of the word [0xE1624] (mov ax, word; shl eax, 2; shr ax, 0xF; lea [4*dx+dx]; shl 2):
   variant "Real Goods in trade are known" (bit 0x20 of 0xE1625) */
static int32_t shift20(void)
{
    return 20 * (int32_t)((uint16_t)(D16(FLAGS) << 2) >> 15);
}

/* the player list 0xF06AE / 0xF06B0: nations 1..9 with +0x9A != 0 (0x6F284..0x6F2CE, 0x723F1.., 0x72612..,
   0x72F0C..) */
static void player_list(void)
{
    int32_t n;
    int16_t k;

    setD16(PLAYERS, 0);
    for (n = 1; n <= 9; n++) {
        if (D8(NATION(n) + 0x9Au) == 0)                             /* mov dl, [199n + 0xF1566]; test dl, dl */
            continue;
        k = S16(PLAYERS);                                           /* movsx edx, ax */
        setD16(PLAYERS, (uint16_t)(k + 1));                         /* inc eax; mov word [0xF06AE], ax */
        setD16(PLAYER_LIST + 2u * (uint32_t)(int32_t)k, (uint16_t)n);   /* mov word [2*edx + 0xF06B0], ax */
    }
}

/* the whole background into the back buffer (fg_vbcopy 0..0x27F x 0..0x1DF, [0xF52CC] -> [0xF52C8]) */
static void background_to_back(void)
{
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BACKGROUND), S32(BACKBUF));
}

/* the record copy rep movsd 0x31 + movsw + movsb: 199 bytes */
static void copy_nation(uint32_t dst, uint32_t src)
{
    memmove(DP(dst), DP(src), 199);
}

/* ================================================================================================================ */
/* DECLARE OFFERS                                                                                                    */
/* ================================================================================================================ */

/* 0x6F270 DECLARE OFFERS */
void f6F270(void)
{
    int32_t n;                                                      /* [ebp-0x18] */
    int32_t mn;

    player_list();                                                  /* 0x6F284..0x6F2CE */
    f783B8(DECL_DLG, S32(0xF5948u));                                /* 0x6F2E0 (ebx = 1, ignored) */
    setD32(MODE, 0);                                                /* 0x6F2EE */
    setD16(NBLUFF, 1);                                              /* 0x6F2F4 mov word [0x112542], bx (1) */
    setD8(PICTURE, 9);                                              /* 0x6F2FB */
    for (n = 1; n <= 9; n++) {                                      /* 0x6F301, 0x6F336 */
        setD32(WANTS + 4u * (uint32_t)n, 0);                        /* 0x6F345 */
        memset(DP(NATION(n) + 0x9Cu), 0, 0xD);                      /* 0x6F364 */
        if (f76F14(n) < (int)(D8(SETUP_TRADE) >> 4)) {             /* 0x6F378, 0x6F384 jl 0x6F43B */
            setD8(NATION(n) + 0x9Cu, 0);                            /* 0x6F43D */
            continue;
        }
        if (S8(NATION(n) + 0x9Au) != 3) {                           /* 0x6F38C sar 24; cmp 3; jne 0x6F306 */
            mn = (int32_t)(D8(SETUP_TRADE) >> 4);                   /* 0x6F306 */
            if (mn < 6)                                             /* 0x6F312 cmp edx, 6; jge */
                mn = 6;
            if (f76F14(n) >= mn)                                    /* 0x6F31F, 0x6F324 cmp eax, edx; jl */
                f68730(n);                                          /* 0x6F32B (M7) */
            continue;
        }
        if ((D8(FLAGS) & PBEM_BIT) != 0 && S8(LOCAL_PBEM) != n)     /* 0x6F3A4, 0x6F3A9, 0x6F3B6 (PBEM kept literal) */
            continue;
        setD32(CUR_NATION, (uint32_t)n);                            /* 0x6F3C2 */
        memset(DP(NATION(n) + 0x9Cu), 0, 0xD);                      /* 0x6F3DF */
        f6F454(n);                                                  /* 0x6F3E7 */
        f6FBDC(n, 1);                                               /* 0x6F3F4 */
        f6FE68(0x203, 0x19F, n, 1, 0xF);                            /* 0x6F40D */
        setD32(GROUP, 0xFFFFFFFFu);                                 /* 0x6F412 */
        f70A6C(-0x11, n);                                           /* 0x6F424 */
        f7109C(n, DECL_DLG);                                        /* 0x6F431 */
    }
}

/* 0x6F454 the Declare Trade Offers screen of n */
void f6F454(int n)
{
    char     buf[100];                                              /* [ebp-0x84] */
    int32_t  k;                                                     /* [ebp-0x14] the row */
    int32_t  m;                                                     /* [ebp-0x1C] */
    int32_t  j;                                                     /* [ebp-0x18] */

    f3CD6C(S8(PICTURE));                                            /* 0x6F473 */
    background_to_back();                                           /* 0x6F49B */
    fg_vbopen(S32(BACKBUF));                                        /* 0x6F4AC */
    k = 0;                                                          /* 0x6F4B4 */
    for (m = 1; m <= 9; m++) {                                      /* 0x6F4B7, 0x6F5A8 */
        if (D8(NATION(m) + 0x9Au) == 0)                             /* 0x6F5B5 */
            continue;
        f3AF8C(TOKEN(m), (0x35 - TOKEN_W(m)) / 2 + 4, 49 * k + 0xD);   /* 0x6F602 */
        f39F90(1);                                                  /* 0x6F60C */
        fg_setcolor(m == n ? 0xF : 5);                              /* 0x6F611..0x6F4C9 */
        f3A0B0(4, 49 * k + 0xD + TOKEN_H(m) + 2, 0x35, NAME(m));    /* 0x6F50C */
        f3AF8C(CARD_BACK, 0x3B, 49 * k + 6);                        /* 0x6F52F */
        snprintf(buf, sizeof buf, "%d", f76F14(m));                 /* 0x6F542, 0x6F547 itoa(.., 10) */
        f39F90(2);                                                  /* 0x6F551 */
        fg_setcolor(m == n ? 0xF : 5);                              /* 0x6F556..0x6F56B */
        f3A0B0(0x3C, 49 * k + 0x14, 0x1E, buf);                     /* 0x6F597 */
        k++;                                                        /* 0x6F59F */
    }
    f6F95C(1);                                                      /* 0x6F640 (edx, ebx, ecx, esi kept over it) */
    setD16(0x11158Cu, 7);                                           /* 0x6F645 */
    setD16(0x11158Eu, 0x196);                                       /* 0x6F64C */
    setD16(0x111590u, 0x1F4);                                       /* 0x6F653 */
    setD16(0x111592u, 0x2E);                                        /* 0x6F65A */
    for (m = 0; S16(NBLUFF) > m; m++) {                             /* 0x6F663, 0x6F66E */
        f3AF8C(CARD_EMPTY, 39 * m + 0x181, 0x78);                   /* 0x6F69C */
        setD16(0x111574u + 8u * (uint32_t)m, (uint16_t)(39 * m + 0x181));   /* 0x6F6AD */
        setD16(0x111576u + 8u * (uint32_t)m, 0x78);                 /* 0x6F6B8 */
        setD16(0x111578u + 8u * (uint32_t)m, 0x20);                 /* 0x6F6C5 */
        setD16(0x11157Au + 8u * (uint32_t)m, 0x2E);                 /* 0x6F6D2 */
    }
    for (j = 0; j < 2; j++) {                                       /* 0x6F6E0, 0x6F6EB */
        f3AF8C(CARD_EMPTY, 39 * j + 0x88, 0x78);                    /* 0x6F712 */
        setD16(0x111524u + 8u * (uint32_t)j, (uint16_t)(39 * j + 0x88));   /* 0x6F724 */
        setD16(0x111526u + 8u * (uint32_t)j, 0x78);                 /* 0x6F72F */
        setD16(0x111528u + 8u * (uint32_t)j, 0x20);                 /* 0x6F73C */
        setD16(0x11152Au + 8u * (uint32_t)j, 0x2E);                 /* 0x6F749 */
    }
    for (j = 0; S16(NBLUFF) > j; j++) {                             /* 0x6F757, 0x6F762 */
        f3AF8C(CARD_EMPTY, 39 * j + 0xE2, 0x78);                    /* 0x6F790 */
        setD16(0x111534u + 8u * (uint32_t)j, (uint16_t)(39 * j + 0xE2));   /* 0x6F7A2 */
        setD16(0x111536u + 8u * (uint32_t)j, 0x78);                 /* 0x6F7AD */
        setD16(0x111538u + 8u * (uint32_t)j, 0x20);                 /* 0x6F7BA */
        setD16(0x11153Au + 8u * (uint32_t)j, 0x2E);                 /* 0x6F7C7 */
    }
    f39F90(1);                                                      /* 0x6F7D8 */
    fg_setcolor(5);                                                 /* 0x6F7DF */
    for (j = 0; j < 5; j++) {                                       /* 0x6F7E9, 0x6F7F4 */
        f3AF8C(CARD_EMPTY, 60 * j + 0x88, 0xC8);                    /* 0x6F81B */
        setD16(0x11154Cu + 8u * (uint32_t)j, (uint16_t)(60 * j + 0x88));   /* 0x6F82C */
        setD16(0x11154Eu + 8u * (uint32_t)j, 0xC8);                 /* 0x6F837 */
        setD16(0x111550u + 8u * (uint32_t)j, 0x20);                 /* 0x6F844 */
        setD16(0x111552u + 8u * (uint32_t)j, 0x2E);                 /* 0x6F851 */
        if (j < 4)                                                  /* 0x6F85E cmp eax, 4; jge */
            f3A0B0(60 * j + 0xA8, 0xDC, 0x1E, DSTR(0xB8EB0u));      /* 0x6F884 "or" */
    }
    f71F2C(7, 0x196, 0x1F4, n);                                     /* 0x6F8A0 */
    snprintf(buf, sizeof buf, DSTR(0xB8EB3u), (int)S16(SESSION));   /* 0x6F8BF "Declare Trade Offers - Session %d" */
    f3A18C(0, 8, 0x280, 0xB0, buf);                                 /* 0x6F8DA (ecx 0xB0 kept over sprintf) */
    f3A18C(0, 0x1E, 0x280, 0x23, NAME(n));                          /* 0x6F8FB */
    fg_setcolor(5);                                                 /* 0x6F902 */
    f6FA54();                                                       /* 0x6F90A */
    fg_vbclose();                                                   /* 0x6F90F */
    f3BFBC(DECL_DLG, 0x103, S8(TRADES) != 0 ? 1 : 0);               /* 0x6F914..0x6F93D */
    f3B05C(DECL_DLG);                                               /* 0x6F947 */
    f3CC14();                                                       /* 0x6F94C */
}

/* 0x6F95C the commodity chart: cardtiny i + 1 at (0x235 + 40 (i % 2), 0x18 + 47 (i / 2)), i 0..17 */
void f6F95C(int frame)
{
    int32_t w = (int32_t)D16(0xF4C28u);                             /* 0x6F976 [ebp-0x1C] */
    int32_t h = (int32_t)D16(0xF4C2Au);                             /* 0x6F981 [ebp-0x18] */
    int32_t i;                                                      /* [ebp-0x20] */

    for (i = 0; i <= 0x11; i++) {                                   /* 0x6F98C, 0x6F997 cmp 0x11; jg */
        f3AF8C(0xF4C1Cu + 8u * (uint32_t)i + 8u, 40 * (i % 2) + 0x235, 47 * (i / 2) + 0x18);   /* 0x6F9ED (idiv) */
        if (frame == 1)                                             /* 0x6F9F5 */
            f448EC(40 * (i % 2) + 0x235, 47 * (i / 2) + 0x18, w, h);    /* 0x6FA3F */
    }
}

/* 0x6FA54 the Declare screen's labels (font 1): colour 5 at (+1, +1) for the first, then colour 7 */
void f6FA54(void)
{
    f39F90(1);                                                      /* 0x6FA69 */
    fg_setcolor(5);                                                 /* 0x6FA70 */
    f3A044(0x182, 0x6D, DSTR(0xB8ED5u));                            /* 0x6FA87 "Replace your bluffs with..." */
    f3A044(0x89, 0x6C, DSTR(0xB8EF1u));                             /* 0x6FA9B "Truly stated" */
    f3A044(0xED, 0x6C, DSTR(0xB8EFEu));                             /* 0x6FAAF "Bluff" */
    f3A044(0x93, 0xBC, DSTR(0xB8F04u));                             /* 0x6FAC3 "You want to trade for..." */
    fg_setcolor(7);                                                 /* 0x6FACA */
    f3A044(0x181, 0x6C, DSTR(0xB8F1Du));                            /* 0x6FAE1 */
    f3A044(0x88, 0x6C, DSTR(0xB8F39u));                             /* 0x6FAF5 */
    f3A044(0xEC, 0x6C, DSTR(0xB8F46u));                             /* 0x6FB09 */
    f3A044(0x92, 0xBC, DSTR(0xB8F4Cu));                             /* 0x6FB1D */
}

/* 0x6FB2C the response screen's labels at y 0xE6 / 0xE7 */
void f6FB2C(void)
{
    f39F90(1);                                                      /* 0x6FB41 */
    fg_setcolor(5);                                                 /* 0x6FB48 */
    f3A044(0x167, 0xE7, DSTR(0xB8F65u));                            /* 0x6FB5F "Replace your bluffs with..." */
    f3A044(0x6E, 0xE7, DSTR(0xB8F81u));                             /* 0x6FB73 "Truly stated" */
    f3A044(0xD2, 0xE7, DSTR(0xB8F8Eu));                             /* 0x6FB87 "Bluff" */
    fg_setcolor(7);                                                 /* 0x6FB8E */
    f3A044(0x166, 0xE6, DSTR(0xB8F94u));                            /* 0x6FBA5 */
    f3A044(0x6D, 0xE6, DSTR(0xB8FB0u));                             /* 0x6FBB9 */
    f3A044(0xD1, 0xE6, DSTR(0xB8FBDu));                             /* 0x6FBCD */
}

/* 0x6FDC8 n's hand strip redrawn in the back buffer and pasted (frame: 0 f71FB0, else f721A0) */
void f6FDC8(int n, int frame)
{
    fg_vbopen(S32(BACKBUF));                                        /* 0x6FDE7 */
    f71D30(-1);                                                     /* 0x6FDF4 */
    if (frame == 0)                                                 /* 0x6FDF9 */
        f71FB0(7, 0x196, 0x1F4, n);                                 /* 0x6FE11 */
    else
        f721A0(7, 0x196, 0x1F4, n);                                 /* 0x6FE2A */
    f78884();                                                       /* 0x6FE2F */
    fg_vbpaste(7, 0x1FA, 0x196, 0x1C3, 7, 0x1C3);                   /* 0x6FE4C */
    f788AC();                                                       /* 0x6FE54 */
    fg_vbclose();                                                   /* 0x6FE59 */
}

/* the card c drawn at slot s, into the back buffer then on the screen with the mouse hidden (0x700FC..0x70182,
   0x70200..0x7027A) */
static void slot_card(int32_t s, int8_t c)
{
    fg_vbopen(S32(BACKBUF));                                        /* 0x70109 */
    f3AF8C(CARD(c), SLOT_X(s), SLOT_Y(s));                          /* 0x7013D */
    fg_vbclose();                                                   /* 0x70142 */
    f78884();                                                       /* 0x70147 */
    f3AF8C(CARD(c), SLOT_X(s), SLOT_Y(s));                          /* 0x7017D */
    f788AC();                                                       /* 0x70182 */
}

/* 0x70074 card c (al) into the first empty slot of the current group; 1 placed, 0 full */
int f70074(int c, int n)
{
    int8_t  cb = (int8_t)c;                                         /* 0x70087 mov [ebp-0x10], al */
    int32_t k = S32(GROUP);
    int32_t st;                                                     /* [ebp-0x18] */
    int32_t j;                                                      /* [ebp-0x1C] */

    if (S32(MODE) == 1) {                                           /* 0x70092 */
        st = START2(k);                                             /* 0x700A4 */
        for (j = 0; COUNT(S32(GROUP)) > j; j++) {                   /* 0x700B0, 0x700BB */
            uint32_t a = ROW(S16(CROW)) + (uint32_t)(st + j);
            if (S8(a) != 0)                                         /* 0x700E3 dword [.. + 0x111115] sar 24 */
                continue;
            setD8(a, (uint8_t)cb);                                  /* 0x70103 */
            slot_card(st + j, cb);                                  /* 0x70109..0x70182 */
            return 1;                                               /* 0x70187 */
        }
        return 0;
    }
    st = START2(k);                                                 /* 0x7019A (the rectangles) */
    for (j = 0; COUNT(S32(GROUP)) > j; j++) {                       /* 0x701A6, 0x701B1 */
        uint32_t a = NATION(n) + 0x9Cu + (uint32_t)(START1(S32(GROUP)) + j);   /* 0x701C8 dword [k + 0xCA99D] sar 24 */
        if (S8(a) != 0)                                             /* 0x701DD dword [.. + 0xF1565] sar 24 */
            continue;
        setD8(a, (uint8_t)cb);                                      /* 0x701FA */
        slot_card(st + j, cb);                                      /* 0x70200..0x7027A */
        return 1;                                                   /* 0x7027F */
    }
    return 0;
}

/* the empty slot s redrawn: background into the back buffer, the empty card (spr), then the rectangle to the screen
   and the card again with the mouse hidden (0x7034D..0x703F4 and its copies) */
static void slot_empty(int32_t s, uint32_t spr)
{
    fg_vbopen(S32(BACKBUF));                                        /* 0x70354 */
    f71D30(s);                                                      /* 0x70362 */
    f3AF8C(spr, SLOT_X(s), SLOT_Y(s));                              /* 0x7039E */
    fg_vbclose();                                                   /* 0x703A3 */
    f78884();                                                       /* 0x703A8 */
    f71E34(s);                                                      /* 0x703B3 */
    f3AF8C(spr, SLOT_X(s), SLOT_Y(s));                              /* 0x703EF */
    f788AC();                                                       /* 0x703F4 */
}

/* 0x70294 the current group cleared and redrawn (CLEAR SECTION -0x25 with lit 1; CLEAR ALL with lit 0) */
void f70294(int n, int lit)
{
    uint32_t spr = CARD(0x20 + (lit == 1 ? 1 : 0));                 /* sete al; add eax, 0x20 */
    int32_t  st;                                                    /* [ebp-0x14] */
    int32_t  j;                                                     /* [ebp-0x18] */

    if (S32(MODE) == 1) {                                           /* 0x702AE */
        st = START2(S32(GROUP));                                    /* 0x702C9 */
        for (j = 0; COUNT(S32(GROUP)) > j; j++)                     /* 0x702CE, 0x702D9 */
            setD8(ROW(S16(CROW)) + (uint32_t)(st + j), 0);          /* 0x70300 */
        for (j = 0; COUNT(S32(GROUP)) > j; j++) {                   /* 0x7030B, 0x70316 */
            if (S8(ROW(S16(CROW)) + (uint32_t)(st + j)) != 0)       /* 0x7033F (always 0 here) */
                continue;
            slot_empty(st + j, spr);                                /* 0x7034D..0x703F4 */
        }
        if (S32(GROUP) == 1) {                                      /* 0x703FE cmp eax, 1 (eax = [0x112558]) */
            st = (int32_t)S8(0xCA9B0u);                             /* 0x70407 dword [0xCA9AD] sar 24: start2[4] */
            for (j = 0; (int32_t)S8(0xCA9A8u) > j; j++)             /* 0x7041F dword [0xCA9A5] sar 24: count[4] */
                setD8(ROW(S16(CROW)) + (uint32_t)(st + j), 0);      /* 0x70440 */
            for (j = 0; (int32_t)S8(0xCA9A8u) > j; j++) {           /* 0x70456 */
                if (S8(ROW(S16(CROW)) + (uint32_t)(st + j)) != 0)   /* 0x70479 */
                    continue;
                slot_empty(st + j, CARD_EMPTY);                     /* 0x70487..0x7050A */
            }
        }
    } else {
        st = START2(S32(GROUP));                                    /* 0x7051F */
        for (j = 0; COUNT(S32(GROUP)) > j; j++)                     /* 0x70527, 0x70532 */
            setD8(NATION(n) + 0x9Cu + (uint32_t)(START1(S32(GROUP)) + j), 0);   /* 0x7055A */
        for (j = 0; COUNT(S32(GROUP)) > j; j++) {                   /* 0x70565, 0x70570 */
            if (S8(NATION(n) + 0x9Cu + (uint32_t)(START1(S32(GROUP)) + j)) != 0)   /* 0x7059A */
                continue;
            slot_empty(st + j, spr);                                /* 0x705A8..0x7064F */
        }
        if (S32(GROUP) == 1) {                                      /* 0x70659 cmp eax, 1 */
            st = (int32_t)S8(0xCA9AFu);                             /* 0x70662 dword [0xCA9AC] sar 24: start2[3] */
            for (j = 0; (int32_t)S8(0xCA9A7u) > j; j++)             /* 0x7067A dword [0xCA9A4] sar 24: count[3] */
                setD8(NATION(n) + 0x9Cu + (uint32_t)((int32_t)S8(0xCA9A3u) + j), 0);   /* 0x7068E dword [0xCA9A0] sar 24 */
            for (j = 0; (int32_t)S8(0xCA9A7u) > j; j++) {           /* 0x706B2 */
                if (S8(NATION(n) + 0x9Cu + (uint32_t)((int32_t)S8(0xCA9A3u) + j)) != 0)   /* 0x706D5 */
                    continue;
                slot_empty(st + j, CARD_EMPTY);                     /* 0x706E3..0x70766 */
            }
        }
    }
    copy_nation(NATION(0), NATION(n));                              /* 0x70770..0x7078B */
    f707A0(n);                                                      /* 0x7078F */
}

/* 0x707A0 record 0 = n less the cards already placed, then its hand strip */
void f707A0(int n)
{
    int32_t st;                                                     /* [ebp-0x18] */
    int32_t j;                                                      /* [ebp-0x20] */
    int32_t c;                                                      /* [ebp-0x1C] */

    copy_nation(NATION(0), NATION(n));                              /* 0x707B8..0x707D9 */
    if (S32(MODE) == 1) {                                           /* 0x707DA */
        st = (int32_t)S8(0xCA9ACu);                                 /* 0x707E3 dword [0xCA9A9] sar 24: start2[0] */
        for (j = 0; (int32_t)S8(0xCA9A4u) > j; j++) {               /* 0x707FB dword [0xCA9A1] sar 24: count[0] */
            c = S8(ROW(S16(CROW)) + (uint32_t)(st + j));            /* 0x70819 */
            if (c == 0)
                continue;
            f70984(c);                                              /* 0x7083D */
        }
        st = (int32_t)S8(0xCA9B0u);                                 /* 0x70844 dword [0xCA9AD] sar 24: start2[4] */
        for (j = 0; (int32_t)S8(0xCA9A8u) > j; j++) {               /* 0x7085C dword [0xCA9A5] sar 24: count[4] */
            c = S8(ROW(S16(CROW)) + (uint32_t)(st + j));            /* 0x7087E */
            if (c == 0)
                continue;
            f70984(c);                                              /* 0x708A2 */
        }
    } else {
        for (j = 0; (int32_t)S8(0xCA9A4u) > j; j++) {               /* 0x708B6: count[0] */
            c = S8(NATION(n) + 0x9Cu + (uint32_t)((int32_t)S8(0xCA9A0u) + j));   /* 0x708D7 start1[0] */
            if (c == 0)
                continue;
            f70984(c);                                              /* 0x708FF */
        }
        for (j = 0; (int32_t)S8(0xCA9A7u) > j; j++) {               /* 0x70913 dword [0xCA9A4] sar 24: count[3] */
            c = S8(NATION(n) + 0x9Cu + (uint32_t)((int32_t)S8(0xCA9A3u) + j));   /* 0x70934 start1[3] */
            if (c == 0)
                continue;
            f70984(c);                                              /* 0x7095C */
        }
    }
    f6FDC8(0, FLAG16(S32(GROUP)));                                  /* 0x70963 dword [2k + 0xCA9B0] sar 16 */
}

/* 0x70984 card c out of nation record 0 (the scratch hand) */
void f70984(int c)
{
    const uint32_t rec = NATION(0);                                 /* 0x709AE [ebp-0x4] */
    int32_t i;                                                      /* [ebp-0xC] */
    int32_t at = 0;                                                 /* [ebp-0x8] (uninitialised in the original) */
    int     found = 0;                                              /* port: whether [ebp-0x8] was written */

    if (c <= 0x12) {                                                /* 0x7099A cmp [ebp-0x10], 0x12; jg */
        setD8(NATION(0) + (uint32_t)c, (uint8_t)(D8(NATION(0) + (uint32_t)c) - 1u));   /* 0x709A3 dec byte [c + 0xF14CC] */
        return;
    }
    for (i = 0; (int32_t)S8(rec + 0x13u) > i; i++) {                /* 0x709C2 dword [rec + 0x10] sar 24 */
        if ((int32_t)S8(rec + 0x14u + (uint32_t)i) == c) {          /* 0x709D6 dword [rec + i + 0x11] sar 24 */
            at = i;                                                 /* 0x709E6 */
            found = 1;
            break;
        }
    }
    /* BUG(1995): a calamity not held leaves [ebp-0x8] unwritten (stack garbage in the original); --original starts
       at index 0 as rules.c's f75CA8 does. Fixed: the record stays. */
    if (!found && !g_original)
        return;
    for (i = at; (int32_t)S8(rec + 0x13u) - 1 > i; i++) {           /* 0x709F7 sar 24; dec eax; cmp; jle */
        setD8(rec + 0x14u + (uint32_t)i, D8(rec + 0x15u + (uint32_t)i));   /* 0x70A15 */
        setD8(rec + 0x1Cu + (uint32_t)i, D8(rec + 0x1Du + (uint32_t)i));   /* 0x70A27 */
    }
    setD8(rec + 0x13u, (uint8_t)(D8(rec + 0x13u) - 1u));            /* 0x70A2F dec byte [rec + 0x13] */
    setD8(rec + 0x14u + (uint32_t)(int32_t)S8(rec + 0x13u), 0);     /* 0x70A3E */
    setD8(rec + 0x1Cu + (uint32_t)(int32_t)S8(rec + 0x13u), 0xFFu); /* 0x70A4E */
}

/* the empty slots of group k drawn with spr in the back buffer (0x70AE7..0x70B61, 0x70B72..0x70BE9 and the new-group
   loops 0x70BF8.., 0x70C79..) */
static void group_empties(int32_t k, int n, uint32_t spr)
{
    int32_t st = START2(k);                                         /* [ebp-0x14] */
    int32_t j;                                                      /* [ebp-0x18] */

    for (j = 0; COUNT(k) > j; j++) {
        int8_t c;
        if (S32(MODE) == 1)
            c = S8(ROW(S16(CROW)) + (uint32_t)(st + j));            /* 0x70B29 dword [.. + 0x111115] sar 24 */
        else
            c = S8(NATION(n) + 0x9Cu + (uint32_t)(START1(k) + j));  /* 0x70BB1 dword [.. + 0xF1565] sar 24 */
        if (c != 0)
            continue;
        f3AF8C(spr, SLOT_X(st + j), SLOT_Y(st + j));                /* 0x70B5C / 0x70BE4 / 0x70C72 / 0x70CF0 */
    }
}

/* one case of 0x70A6C: f707A0(n), the hand / chart buttons (hand_on: 0x75 enabled, 0x76 disabled, the order of the
   two calls as in each case), the chart, the heading in colour 15, fg_vbclose and f3CC14 */
static void group_case(uint32_t dlg, int n, int hand_on, int frame, int x, int y, uint32_t text)
{
    f707A0(n);
    if (hand_on) {
        f3C314(dlg, 0x75);
        f3C3D0(dlg, 0x76);
    } else {
        f3C3D0(dlg, 0x75);
        f3C314(dlg, 0x76);
    }
    fg_vbopen(S32(BACKBUF));
    f6F95C(frame);
    fg_setcolor(0xF);
    f3A044(x, y, DSTR(text));
}

/* 0x70A6C switch the composer to group -0x11 - code */
void f70A6C(int code, int n)
{
    int32_t k = -0x11 - code;                                       /* 0x70A86 */
    int32_t mode;

    setD32(NEW_GROUP, (uint32_t)k);                                 /* 0x70A94 */
    if (k == S32(GROUP))                                            /* 0x70A99 cmp eax, ebx; je 0x70D9B */
        return;
    fg_vbopen(S32(BACKBUF));                                        /* 0x70AA8 */
    mode = S32(MODE);                                               /* 0x70AAD */
    if (mode == 0)                                                  /* 0x70AB6 */
        f6FA54();                                                   /* 0x70ABA */
    else if (mode == 1)                                             /* 0x70AC1 */
        f6FB2C();                                                   /* 0x70AC6 */
    if (S32(GROUP) != -1)                                           /* 0x70ADE / 0x70B69 cmp -1; je 0x70BEB */
        group_empties(S32(GROUP), n, CARD_EMPTY);                   /* 0x70AE7..0x70BE9 */
    group_empties(S32(NEW_GROUP), n, CARD_LIT);                     /* 0x70BEB..0x70CF5 */
    setD32(GROUP, D32(NEW_GROUP));                                  /* 0x70CFC */
    fg_vbclose();                                                   /* 0x70D01 */
    if (S32(MODE) == 0) {                                           /* 0x70D06 jne 0x70EFA */
        switch (code + 0x14) {                                      /* 0x70D19 cmp eax, 3; ja 0x70D91; jump table 0x70A5C */
        case 3:                                                     /* -0x11 -> 0x70D2A */
            group_case(DECL_DLG, n, 1, 1, SLOT_X(0), SLOT_Y(0) - 0xC, 0xB8FF0u);   /* "Truly stated" */
            break;
        case 2:                                                     /* -0x12 -> 0x70DA4 */
            group_case(DECL_DLG, n, 0, 0, SLOT_X(2) + 0xA, SLOT_Y(2) - 0xC, 0xB8FFDu);   /* "Bluff" */
            break;
        case 1:                                                     /* -0x13 -> 0x70E1E */
            group_case(DECL_DLG, n, 0, 0, 0x92, 0xBC, 0xB9003u);    /* "You want to trade for..." */
            break;
        case 0:                                                     /* -0x14 -> 0x70E8B (0x76 off first, then 0x75 on) */
            f707A0(n);                                              /* 0x70E8E */
            f3C3D0(DECL_DLG, 0x76);                                 /* 0x70E9D */
            f3C314(DECL_DLG, 0x75);                                 /* 0x70EAC */
            fg_vbopen(S32(BACKBUF));                                /* 0x70EB7 */
            f6F95C(1);                                              /* 0x70EC4 */
            fg_setcolor(0xF);                                       /* 0x70ECB */
            f3A044(0x181, 0x6C, DSTR(0xB901Cu));                    /* 0x70EE2 "Replace your bluffs with..." */
            break;
        default:                                                    /* 0x70D91 */
            break;
        }
        fg_vbclose();                                               /* 0x70D91 / 0x70E0B / 0x70E78 / 0x70EE7 */
        f3CC14();                                                   /* 0x70D96 / 0x70E10 / 0x70E7D / 0x70EEC */
        return;
    }
    /* mode 1 (0x70EFA): unsigned compares on code */
    if ((uint32_t)code < (uint32_t)-0x12) {                         /* 0x70F00 jb 0x71083 */
        if (code == -0x15) {                                        /* 0x71083 cmp eax, -0x15; je 0x71009 */
            f707A0(n);                                              /* 0x7100C */
            f3C3D0(RESPOND_DLG, 0x76);                              /* 0x7101B */
            f3C314(RESPOND_DLG, 0x75);                              /* 0x7102A */
            fg_vbopen(S32(BACKBUF));                                /* 0x71036 */
            f6F95C(1);                                              /* 0x71043 */
            fg_setcolor(0xF);                                       /* 0x7104A */
            f3A044(SLOT_X(5), SLOT_Y(5) - 0xC, DSTR(0xB904Bu));     /* 0x71066 "Replace your bluffs with..." */
        }
    } else if (code == -0x12) {                                     /* 0x70F06 jbe 0x70F8F */
        group_case(RESPOND_DLG, n, 0, 0, SLOT_X(2) + 0xA, SLOT_Y(2) - 0xC, 0xB9045u);   /* "Bluff" */
    } else if (code == -0x11) {                                     /* 0x70F0C */
        group_case(RESPOND_DLG, n, 1, 1, SLOT_X(0), SLOT_Y(0) - 0xC, 0xB9038u);   /* "Truly stated" */
    }
    fg_vbclose();                                                   /* 0x71070 / 0x71088 / 0x70F7C / 0x70FF6 */
    f3CC14();                                                       /* 0x71075 / 0x7108D / 0x70F81 / 0x70FFB */
}

/* 0x7109C the composer's click loop for n on dlg */
void f7109C(int n, uint32_t dlg)
{
    int32_t  code;                                                  /* [ebp-0x44] */
    int32_t  saved, st, j, x, big, sel, cnt, col, g;
    const uint32_t rec = NATION(0);                                 /* [ebp-0x14] */

    for (;;) {
        code = f78BA4(dlg, 0);                                      /* 0x710BB */
        f3CD6C(S8(PICTURE));                                        /* 0x710CB */
        if (code == -100)                                           /* 0x710D3 */
            continue;
        if (code == -1) {                                           /* 0x710DB */
            if (f71768(n) == -1)                                    /* 0x710E3; cmp eax, ebx; je 0x716B1 */
                return;
            continue;
        }
        if (code <= -0x11 && code >= -0x15) {                       /* 0x710F5 jg; 0x710FA jl */
            f70A6C(code, n);                                        /* 0x716A7 */
            continue;
        }
        if (code == -0x25) {                                        /* 0x7110E CLEAR SECTION */
            f70294(n, 1);                                           /* 0x7111B */
            continue;
        }
        if (code == -0x7A) {                                        /* 0x71125 CLEAR ALL */
            saved = S32(GROUP);                                     /* 0x71133 [ebp-0x18] */
            for (j = 0; j < 3; j++) {                               /* 0x71138, 0x71143 */
                int32_t mode = S32(MODE);                           /* 0x71149 */
                if (mode != 0 && !(mode == 1 && j != 2))            /* 0x7114F je 0x7115E; cmp 1; jne; cmp j, 2; je */
                    continue;
                setD32(GROUP, (uint32_t)j);                         /* 0x71161 */
                f70294(n, 0);                                       /* 0x7116B */
            }
            setD32(GROUP, (uint32_t)saved);                         /* 0x71175 */
            st = START2(saved);                                     /* 0x7117A [ebp-0x1C] */
            for (j = 0; COUNT(S32(GROUP)) > j; j++) {               /* 0x71188, 0x71193 (every slot, filled or not) */
                fg_vbopen(S32(BACKBUF));                            /* 0x711B1 */
                f71D30(st + j);                                     /* 0x711BF */
                f3AF8C(CARD_LIT, SLOT_X(st + j), SLOT_Y(st + j));   /* 0x711E9 */
                fg_vbclose();                                       /* 0x711EE */
                f78884();                                           /* 0x711F3 */
                f71E34(st + j);                                     /* 0x711FE */
                f3AF8C(CARD_LIT, SLOT_X(st + j), SLOT_Y(st + j));   /* 0x71228 */
                f788AC();                                           /* 0x7122D */
            }
            continue;
        }
        if (code == -0x1D) {                                        /* 0x7123A the hand */
            sel = -100;                                             /* 0x71243 [ebp-0x38] */
            x = 7;                                                  /* 0x7124A [ebp-0x34] */
            big = S32(BIG) >= 0x20 ? 0x20 : S32(BIG);               /* 0x71258..0x7126C [ebp-0x28] */
            for (j = 1; j <= 0x12; j++) {                           /* 0x7126F, 0x7127E cmp 0x12; jg */
                int32_t c = S8(rec + (uint32_t)j);                  /* 0x7128E movsx */
                if (c == 1) {                                       /* 0x71291 */
                    if (S32(0xE1628u) >= x && x + big >= S32(0xE1628u))   /* 0x712A0 jl; 0x712B2 jl */
                        sel = j;                                    /* 0x712B7 */
                    x += S32(BIG);                                  /* 0x712BA, 0x71322 */
                } else if (c != 0) {                                /* 0x712CC */
                    int32_t m;                                      /* [ebp-0x3C] */
                    for (m = 0; S8(rec + (uint32_t)j) > m; m++) {   /* 0x712D0, 0x712E4 */
                        if (S32(0xE1628u) >= x && x + big >= S32(0xE1628u))   /* 0x712F3, 0x71305 */
                            sel = j;                                /* 0x7130A */
                        x += S32(SMALL);                            /* 0x7130D */
                    }
                    x += S32(BIG) - S32(SMALL);                     /* 0x71317..0x71322 */
                }
                if (sel != -100)                                    /* 0x71325 je 0x71278 */
                    break;
            }
            if (sel == -100 && (S32(GROUP) == 3 || S32(GROUP) == 4) && D8(rec + 0x13u) != 0) {   /* 0x7132F..0x71355 */
                int32_t m;                                          /* [ebp-0x40] */
                for (m = 0; (int32_t)S8(rec + 0x13u) > m; m++) {    /* 0x7135D, 0x71370 */
                    if (S32(0xE1628u) >= x && x + 0x20 >= S32(0xE1628u)   /* 0x71388, 0x71398 */
                        && f75BD4(S8(rec + 0x14u + (uint32_t)m)) != 0) {   /* 0x713A6 */
                        sel = S8(rec + 0x14u + (uint32_t)m);        /* 0x713BB */
                        break;
                    }
                    x += S32(BIG);                                  /* 0x71362 */
                }
            }
            if (sel == -100)                                        /* 0x713BE */
                continue;
            if (f70074((int8_t)sel, n) != 0) {                      /* 0x713CF movsx eax, byte [ebp-0x38]; test; je */
                if (sel <= 0x12)                                    /* 0x713DB */
                    setD8(NATION(0) + (uint32_t)sel, (uint8_t)(D8(NATION(0) + (uint32_t)sel) - 1u));   /* 0x713E3 */
                else
                    f716BC(sel, 0);                                 /* 0x713F0 */
            }
            g = S32(GROUP);                                         /* 0x713F5 / 0x713FA */
            if (g == 0) {                                           /* 0x71402 */
                if (S32(MODE) == 0) {                               /* 0x7140C */
                    if (S8(NATION(n) + 0x9Du) != 0) {               /* 0x71417 dword [199n + 0xF1566] sar 24 */
                        f70A6C(-0x12, n);                           /* 0x71430 */
                        continue;
                    }
                } else if (S8(ROW(S16(CROW)) + 1u) != 0) {          /* 0x71446 dword [16r + 0x111116] sar 24 */
                    f70A6C(-0x12, n);                               /* 0x7145B */
                    continue;
                }
            } else if (g == 3) {                                    /* 0x71465 */
                if (S32(MODE) != 0)                                 /* 0x71470 jne 0x710B6 */
                    continue;
                f6FDC8(0, FLAG16(g));                               /* 0x71478..0x71483 dword [2k + 0xCA9B0] sar 16 */
                if (S8(NATION(n) + 0xA5u + (uint32_t)(int32_t)S8(0xCA9A7u)) != 0)   /* 0x7148F..0x71497: count[3], +0xF156E + 3 */
                    f70A6C(-0x13, n);                               /* 0x714B1 */
                continue;
            }
            f6FDC8(0, FLAG16(g));                                   /* 0x714BB..0x714C6 */
            continue;
        }
        if (code == -0x1C) {                                        /* 0x714D3 the chart */
            int32_t dx = S32(0xE1628u) - 0x235;                     /* 0x714E1 [ebp-0x30] */
            if (dx >= 0 && dx <= 0x20)                              /* 0x714EE jl; 0x714F4 jg */
                col = 1;                                            /* 0x714F6 [ebp-0x2C] */
            else if (dx >= 0x28 && dx <= 0x48)                      /* 0x71505 jl; 0x7150F jg */
                col = 2;                                            /* 0x71515 */
            else
                continue;
            sel = (S32(0xE162Cu) - 0x18) / 0x2F * 2 + col;          /* 0x7151C..0x71538 (idiv) */
            /* BUG(1995): the chart button reaches y 448 while row 8 ends at 446, so a click at y 447..448 gives card 19
               or 20, a calamity, as a bluff or a wanted card. Fixed: ignored. */
            if (!g_original && sel > 0x12)
                continue;
            g = S32(GROUP) / 3;                                     /* 0x7154A (idiv) [ebp-0x24] */
            cnt = 0;                                                /* 0x71551 [ebp-0x20] */
            if (S32(MODE) == 0) {                                   /* 0x71554 */
                st = (int32_t)S8(0xCA9A1u + 3u * (uint32_t)g);      /* 0x71569 dword [3g + 0xCA99E] sar 24 */
                for (j = 0; (int32_t)S8(0xCA9A5u + 3u * (uint32_t)g) > j; j++)   /* 0x7158E dword [3g + 0xCA9A2] sar 24 */
                    if (S8(NATION(n) + 0x9Cu + (uint32_t)(st + j)) != 0)   /* 0x715A9 */
                        cnt++;                                      /* 0x715BA */
            } else {
                for (j = 0; (int32_t)S8(0xCA9A5u) > j; j++)         /* 0x715CA dword [0xCA9A2] sar 24 */
                    if (S8(ROW(S16(CROW)) + 2u + (uint32_t)j) != 0) /* 0x715E5 dword [16r + j + 0x111117] sar 24 */
                        cnt++;                                      /* 0x715F5 */
            }
            if (S32(GROUP) % 3 == 2 || cnt + 2 < f76F14(n))         /* 0x71609 (idiv) cmp edx, 2; 0x7161E jge */
                f70074((int8_t)sel, n);                             /* 0x71629 */
            if (S32(GROUP) != 1)                                    /* 0x7162E */
                continue;
            if (S32(MODE) == 0) {                                   /* 0x71641 */
                if (S8(NATION(n) + 0x9Du + (uint32_t)(int32_t)S16(NBLUFF)) != 0)   /* 0x71655 [edx + eax + 0xF1566] */
                    f70A6C(-0x14, n);                               /* 0x7166F */
            } else if (S8(ROW(S16(CROW)) + 1u + (uint32_t)(int32_t)S16(NBLUFF)) != 0) {   /* 0x7168D [.. + 0x111116] */
                f70A6C(-0x15, n);                                   /* 0x716A7 */
            }
            continue;
        }
    }
}

/* 0x716BC calamity c out of nation record m (the only caller passes 0): the freed last slots stay as they were */
void f716BC(int c, int m)
{
    const uint32_t rec = NATION(m);                                 /* 0x716E1 [ebp-0x4] */
    int32_t i;                                                      /* [ebp-0xC] */
    int32_t at = 0;                                                 /* [ebp-0x8] (uninitialised in the original) */
    int     found = 0;

    for (i = 0; (int32_t)S8(rec + 0x13u) > i; i++) {                /* 0x716F1 dword [rec + 0x10] sar 24 */
        if ((int32_t)S8(rec + 0x14u + (uint32_t)i) == c) {          /* 0x71705 dword [rec + i + 0x11] sar 24 */
            at = i;                                                 /* 0x71715 */
            found = 1;
            break;
        }
    }
    /* BUG(1995): as 0x70984, a calamity not held shifts from an unwritten index; --original starts at 0. Fixed: the
       record stays. */
    if (!found && !g_original)
        return;
    for (i = at; (int32_t)S8(rec + 0x13u) - 1 > i; i++) {           /* 0x71726 */
        setD8(rec + 0x14u + (uint32_t)i, D8(rec + 0x15u + (uint32_t)i));   /* 0x71744 */
        setD8(rec + 0x1Cu + (uint32_t)i, D8(rec + 0x1Du + (uint32_t)i));   /* 0x71756 */
    }
    setD8(rec + 0x13u, (uint8_t)(D8(rec + 0x13u) - 1u));            /* 0x7175E dec byte [rec + 0x13] */
}

/* 0x71768 DONE: the offer (mode 0) or the counter-offer (mode 1) checked; -15 rejected (message box), -1 accepted */
int f71768(int n)
{
    char     text[200];                                             /* [ebp-0x174] */
    char     tmp[100];                                              /* [ebp-0xAC] */
    const uint32_t p = NATION(n) + 0x9Cu;                           /* 0x71799 [ebp-0x18] */
    int32_t  total = 0;                                             /* [ebp-0x28] */
    int32_t  a = 0;                                                 /* [ebp-0x30] truly stated */
    int32_t  b;                                                     /* [ebp-0x2C] bluffs, then the replacements */
    int32_t  want = 0;                                              /* [ebp-0x24] (never set in mode 1) */
    int32_t  ok, ok2 = 1, ok3 = 1, ok4;                             /* [ebp-0x48], [ebp-0x44], [ebp-0x40], [ebp-0x3C] */
    int32_t  st, j, r;

    /* BUG(1995): in mode 0 the original writes the text only when the offer is incomplete, so a complete offer
       without its bluff replacement appends to stack garbage (see the file header). Both modes start empty. */
    text[0] = 0;
    if (S32(MODE) == 0) {                                           /* 0x7179C */
        ok = 1;                                                     /* 0x717B1 */
        st = START1(0);                                             /* 0x717BA dword [0xCA99D] sar 24 */
        for (j = 0; COUNT(0) > j; j++)                              /* 0x717D0 dword [0xCA9A1] sar 24 */
            if (S8(p + (uint32_t)(st + j)) != 0)                    /* 0x717E6 movsx */
                a++;
        if (a != 2)                                                 /* 0x717F5 */
            ok = 0;
        b = 0;                                                      /* 0x71802 */
        st = START1(1);                                             /* 0x71805 dword [0xCA99E] sar 24 */
        for (j = 0; COUNT(1) > j; j++)                              /* 0x7181B dword [0xCA9A2] sar 24 */
            if (S8(p + (uint32_t)(st + j)) != 0)
                b++;
        total += b;                                                 /* 0x71843 */
        if (b == 0)                                                 /* 0x71849 */
            ok = 0;
        want = 0;                                                   /* 0x71852 */
        st = START1(2);                                             /* 0x71855 dword [0xCA99F] sar 24 */
        for (j = 0; COUNT(2) > j; j++)                              /* 0x7186B dword [0xCA9A3] sar 24 */
            if (S8(p + (uint32_t)(st + j)) != 0)
                want++;
        if (want == 0)                                              /* 0x71893 */
            ok = 0;
        if (a == 0 && b == 0 && want == 0)                          /* 0x7189A..0x718AC */
            ok = 1;
        if (ok == 0)                                                /* 0x718B3 */
            snprintf(text, sizeof text, "%s", DSTR(0xB9067u));      /* 0x718C5 inline strcpy "Your offer is incomplete.\n" */
        b = 0;                                                      /* 0x718E0 the replacements */
        st = START1(3);                                             /* 0x718E3 dword [0xCA9A0] sar 24 */
        for (j = 0; COUNT(3) > j; j++)                              /* 0x718FB dword [0xCA9A4] sar 24 */
            if (S8(p + (uint32_t)(st + j)) != 0)
                b++;
    } else {
        text[0] = 0;                                                /* 0x71926 */
        ok = 1;                                                     /* 0x71934 */
        for (j = 0; COUNT(0) > j; j++)                              /* 0x71948 */
            if (S8(ROW(S16(CROW)) + (uint32_t)j) != 0)              /* 0x71963 dword [16r + j + 0x111115] sar 24 */
                a++;
        if (a != 2)                                                 /* 0x71978 */
            ok = 0;
        b = 0;                                                      /* 0x71985 */
        for (j = 0; COUNT(1) > j; j++)                              /* 0x71993 */
            if (S8(ROW(S16(CROW)) + 2u + (uint32_t)j) != 0)         /* 0x719AE dword [.. + 0x111117] sar 24 */
                b++;
        total += b;                                                 /* 0x719C6 */
        if (b == 0)                                                 /* 0x719CC */
            ok = 0;
        if (a == 0 && b == 0)                                       /* 0x719D3, 0x719D9 */
            ok = 1;
        if (ok == 0) {                                              /* 0x719E6 */
            snprintf(tmp, sizeof tmp, "%s", DSTR(0xB9082u));        /* 0x719F8 sprintf(tmp, "Your offer is incomplete.\n") */
            cat(text, sizeof text, tmp);                            /* 0x71A0C..0x71A2D */
        }
        b = 0;                                                      /* 0x71A30 the replacements */
        for (j = 0; COUNT(4) > j; j++)                              /* 0x71A3E dword [0xCA9A5] sar 24 */
            if (S8(ROW(S16(CROW)) + 5u + (uint32_t)j) != 0)         /* 0x71A59 dword [.. + 0x11111A] sar 24 */
                b++;
    }
    ok4 = 1;                                                        /* 0x71A6E */
    if (b < total) {                                                /* 0x71A78 cmp; jge */
        cat(text, sizeof text, DSTR(0xB909Du));                     /* 0x71A88..0x71AA9 "You must declare a bluff replacement." */
        ok4 = 0;                                                    /* 0x71AAC */
    }
    if (ok == 0 || ok2 == 0 || ok3 == 0 || ok4 == 0) {              /* 0x71AAF..0x71AC5 */
        f71B68(text);                                               /* 0x71AD2 */
        r = -15;                                                    /* 0x71AD7 */
    } else {
        /* BUG(1995): mode 1 never sets [ebp-0x24], yet stores it (stack garbage in the original). --original stores 0
           (the port cannot reproduce the garbage); fixed: mode 1 leaves n's own offer's count. */
        if (S32(MODE) == 0 || g_original)
            setD32(WANTS + 4u * (uint32_t)n, (uint32_t)want);       /* 0x71AE7 */
        r = -1;                                                     /* 0x71AEE */
    }
    if (r == -1 && S32(MODE) == 1 && a != 0 && b != 0) {            /* 0x71AF1..0x71B0D */
        const uint32_t o = OFFER(S32(OFFER_IDX));                   /* 0x71B0F */
        setD8(ROW(S16(CROW)) + 0xDu, D8(o + 0x12u));                /* 0x71B35 */
        setD8(ROW(S16(CROW)) + 0xEu, (uint8_t)n);                   /* 0x71B3E */
        setD8(ROW(S16(CROW)) + 0xFu, D8(o + 0x13u));                /* 0x71B4A */
        setD8(o + 0x14u, (uint8_t)n);                               /* 0x71B53 */
    }
    return r;                                                       /* 0x71B59 */
}

/* 0x71B68 the offer-error box: text wrapped in 0xB4 px, box 200 wide at x 220, centred vertically, CONTINUE */
void f71B68(const char *text)
{
    int32_t  maxw;                                                  /* [ebp-0x1C] */
    int32_t  h, x, y, bx;                                           /* [ebp-0x18], [ebp-0x2C], [ebp-0x28], [ebp-0x24] */
    uint32_t saved;                                                 /* [ebp-0x14] */
    int32_t  r;                                                     /* [ebp-0x20] */

    h = f3A42C(0, 0, 0xB4, 2, text, &maxw, 0) + 0x2D;               /* 0x71B97, 0x71B9F */
    x = 0xDC;                                                       /* 0x71BA3 */
    y = (0x1E0 - h) / 2;                                            /* 0x71BAA..0x71BBB (sar trick: toward zero) */
    bx = x + 0x2D;                                                  /* 0x71BC3 */
    junk_makegif(x, x + 0xC7, y, y + h - 1, DSTR(0xB90C3u));        /* 0x71BE7 fg_makegif(.., "junk") */
    saved = D32(CUR_DIALOG);                                        /* 0x71BEC */
    f3C940(saved);                                                  /* 0x71BF7 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x71C03 */
    f3C9D0(x, y, 0xC8, h, 3);                                       /* 0x71C1B */
    fg_setcolor(5);                                                 /* 0x71C22 */
    f3A42C(x + 0xA, y + 0xA, 0xB4, 2, text, &maxw, 1);              /* 0x71C4A */
    fg_vbclose();                                                   /* 0x71C4F */
    f3BB48(BOX_DLG, 0x2A, bx);                                      /* 0x71C61 */
    f3BC64(BOX_DLG, 0x2A, y + h - 0x1D);                            /* 0x71C7B */
    f3C284(BOX_DLG, 0x2A, 0xB90C8u);                                /* 0x71C8F "CONTINUE" */
    f3B05C(BOX_DLG);                                                /* 0x71C99 */
    f78884();                                                       /* 0x71C9E */
    fg_vbpaste(x, x + 0xC7, y, y + h - 1, x, y + h - 1);            /* 0x71CC8 */
    f788AC();                                                       /* 0x71CD0 */
    do
        r = f78BA4(BOX_DLG, 0);                                     /* 0x71CDC */
    while (r != -1);                                                /* 0x71CE7 */
    setD32(CUR_DIALOG, saved);                                      /* 0x71CEF */
    f3C8C4(saved);                                                  /* 0x71CF4 */
    f78884();                                                       /* 0x71CF9 */
    fg_move(x, y);                                                  /* 0x71D06 */
    junk_showgif(DSTR(0xB90D1u), 3);                                /* 0x71D15 fg_showgif("junk", 3) */
    f3CCBC();                                                       /* 0x71D1D */
    f788AC();                                                       /* 0x71D22 */
}

/* 0x721A0 n's hand face up as f71FB0, plus f448EC(x, y, w, h) of cardtiny 1's size after each card */
void f721A0(int x, int y, int w, int n)
{
    const int32_t fw = (int32_t)D16(0xF4C28u);                      /* 0x721C6 [ebp-0x14] */
    const int32_t fh = (int32_t)D16(0xF4C2Au);                      /* 0x721D1 [ebp-0x10] */
    const uint32_t rec = NATION(n);                                 /* 0x721F8 [ebp-0xC] */
    int32_t total;                                                  /* [ebp-0x1C] */
    int32_t i, k;                                                   /* [ebp-0x24], [ebp-0x20] */
    int32_t cx;                                                     /* [ebp-0x18] */

    setD32(BIG, 0x28);                                              /* 0x721DE */
    setD32(SMALL, 0x0A);                                            /* 0x721E4 */
    do {
        total = 0;                                                  /* 0x721FD */
        for (i = 1; i <= 0x12; i++) {                               /* 0x72200, 0x7223C */
            if (D8(rec + (uint32_t)i) == 0)                         /* 0x72245 mov dl, [..]; test dl, dl */
                continue;
            if (S8(rec + (uint32_t)i) == 1)                         /* 0x72259 sar 24; cmp 1 */
                total += S32(BIG);                                  /* 0x72267, 0x7222C */
            else
                total += (S8(rec + (uint32_t)i) - 1) * S32(SMALL) + S32(BIG);   /* 0x72213..0x7222C */
        }
        total += S8(rec + 0x13u) * S32(BIG);                        /* 0x7226E dword [199n + 0xF14DC] sar 24; imul */
        if (total > w) {                                            /* 0x72286 jle */
            setD32(BIG, (uint32_t)(S32(BIG) - 2));                  /* 0x7228B, 0x72293 */
            if (S32(SMALL) > 3)                                     /* 0x72299 */
                setD32(SMALL, (uint32_t)(S32(SMALL) - 1));          /* 0x722A1 */
        }
    } while (total > w);                                            /* 0x722AD jg 0x721FB */

    cx = x;                                                         /* 0x722B8 */
    for (i = 1; i <= 0x12; i++) {                                   /* 0x722BB, 0x72331 */
        if (S8(rec + (uint32_t)i) == 1) {                           /* 0x7233D */
            f3AF8C(CARD(i), cx, y);                                 /* 0x7235A */
            f448EC(cx, y, fw, fh);                                  /* 0x7236B */
            cx += S32(BIG);                                         /* 0x72370, 0x72328 */
            continue;
        }
        if (S8(rec + (uint32_t)i) == 0)                             /* 0x722CA */
            continue;
        for (k = 0; S8(rec + (uint32_t)i) > k; k++) {               /* 0x722D3, 0x722E7 */
            f3AF8C(CARD(i), cx, y);                                 /* 0x722FD */
            f448EC(cx, y, fw, fh);                                  /* 0x7230E */
            cx += S32(SMALL);                                       /* 0x72318 */
        }
        cx += S32(BIG) - S32(SMALL);                                /* 0x7231D..0x72328 */
    }
    if (D8(rec + 0x13u) != 0)                                       /* 0x7237A cmp byte [rec+0x13], 0 */
        for (k = 0; S8(rec + 0x13u) > k; k++) {                     /* 0x72382, 0x7238D */
            f3AF8C(CARD(S8(rec + 0x14u + (uint32_t)k)), cx, y);     /* 0x723B5 */
            f448EC(cx, y, fw, fh);                                  /* 0x723C6 */
            cx += S32(BIG);                                         /* 0x723D0 */
        }
}

/* ================================================================================================================ */
/* COUNTER OFFERS                                                                                                    */
/* ================================================================================================================ */

/* 0x723DC COUNTER OFFERS */
void f723DC(void)
{
    int32_t n;                                                      /* [ebp-0x18] */
    int32_t code;                                                   /* [ebp-0x20] */
    int32_t i;                                                      /* [ebp-0x24] */
    uint8_t order[16];                                              /* [ebp-0x2C], 8 bytes in the original (8 players at most) */

    player_list();                                                  /* 0x723F1..0x7243B */
    f783B8(COUNTER_DLG, S32(0xF594Cu));                             /* 0x7244D (ebx = 1, ignored) */
    setD16(NBLUFF, 1);                                              /* 0x72457 mov word [0x112542], bx */
    memset(DP(0x111118u), 0, 0x380);                                /* 0x72467 */
    setD16(CROW, 0);                                                /* 0x7246C */
    f726DC();                                                       /* 0x72473 (game_rand) */
    if (S32(NOFFERS) == 0)                                          /* 0x72478 je 0x726D1 */
        return;
    setD8(PICTURE, 2);                                              /* 0x7248C */
    for (n = 1; n <= 9; n++) {                                      /* 0x72492, 0x724FD */
        if (f76F14(n) < (int)(D8(SETUP_TRADE) >> 4))                /* 0x7251B jl */
            continue;
        if (S8(NATION(n) + 0x9Au) != 3) {                           /* 0x72532 jne 0x724E5 */
            f698A4(n);                                              /* 0x724E8 (M7) */
            setD8(ANOTHER + (uint32_t)n, 1);                        /* 0x724F0 */
            continue;
        }
        if ((D8(FLAGS) & PBEM_BIT) != 0 && S8(LOCAL_PBEM) != n)     /* 0x7253A, 0x7253F, 0x7254C (PBEM kept literal) */
            continue;
        setD32(CUR_NATION, (uint32_t)n);                            /* 0x72556 */
        setD32(CSCROLL, 0);                                         /* 0x7255B */
        f72928(n, 0);                                               /* 0x72566 */
        f6FBDC(n, 1);                                               /* 0x72573 */
        f72928(n, 1);                                               /* 0x72580 */
        for (;;) {
            code = f78BA4(COUNTER_DLG, 0);                          /* 0x7258C */
            if (code == -100)                                       /* 0x72597 */
                continue;
            if (code == -1)                                         /* 0x7259F */
                break;
            if (code == -0x2C) {                                    /* 0x7249A NEXT PAGE */
                setD32(CSCROLL, (uint32_t)(S32(CSCROLL) + 5));      /* 0x7249F */
            } else if (code == -0x2B) {                             /* 0x724AE PREV PAGE */
                setD32(CSCROLL, (uint32_t)(S32(CSCROLL) - 5));      /* 0x724B3 */
            } else if (code <= -0x26 && code >= -0x2A) {            /* 0x724C5 jg; 0x724CF jl */
                f72EF4(code, n);                                    /* 0x724DB */
            } else {
                continue;                                           /* 0x72585 (no redraw) */
            }
            f72928(n, 1);                                           /* 0x72580 */
        }
        if ((D8(FLAGS) & PBEM_BIT) == 0 && (D8(CFG0) & 0x80) == 0) {    /* 0x725B1, 0x725B3 */
            setD8(ANOTHER + (uint32_t)n, 1);                        /* 0x724F0 */
            continue;
        }
        if (S16(SESSION) >= (int)(D8(SETUP_TRADE) & 0x0F))          /* 0x725C0..0x725D7 jge */
            continue;
        code = f795A8(DSTR(0xB90D6u));                              /* 0x725E2 "Do you want another round of trading?" */
        if (code == -0x61)                                          /* 0x725ED YES */
            setD8(ANOTHER + (uint32_t)n, 1);                        /* 0x724F0 */
        else if (code == -0x62)                                     /* 0x725F9 NO */
            setD8(ANOTHER + (uint32_t)n, 0);                        /* 0x72607 */
    }
    player_list();                                                  /* 0x72612..0x7265C */
    for (i = 0; i < S32(PLAYERS_N); i++)                            /* 0x7265E, 0x7266B */
        order[i] = D8(PLAYER_LIST + 2u * (uint32_t)i);              /* 0x72685 (the low byte) */
    f76F70(S32(PLAYERS_N), order);                                  /* 0x72692 (game_rand) */
    for (i = 0; i < S32(PLAYERS_N); i++)                            /* 0x72697, 0x726A2 */
        setD16(PLAYER_LIST + 2u * (uint32_t)i, (uint16_t)(int16_t)(int8_t)order[i]);   /* 0x726B0 movsx dx, byte */
    if ((D8(FLAGS) & PBEM_BIT) != 0)                                /* 0x726C3 */
        f53C30();                                                   /* 0x726CC (PBEM_DROPPED) */
}

/* 0x72928 the Make Counter Offers screen of n, hand face down (up == 0) or face up */
void f72928(int n, int up)
{
    char    buf[100];                                               /* [ebp-0x90] */
    int32_t cnt, i, r, row, j;                                      /* [ebp-0x20], [ebp-0x28], [ebp-0x1C], [ebp-0x28], [ebp-0x24] */
    int8_t  c;                                                      /* [ebp-0x14] */
    int32_t a;
    uint32_t b;

    setD8(PICTURE, 0xC);                                            /* 0x72945 */
    f3CD6C(0xC);                                                    /* 0x72951 */
    background_to_back();                                           /* 0x72979 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x72988 */
    if (up == 0) {                                                  /* 0x72990 */
        setD32(BIG, 0x28);                                          /* 0x72996 */
        cnt = f76F14(n);                                            /* 0x729A3 */
        while (S32(BIG) * cnt > 0x23A)                              /* 0x729AB imul; cmp 0x23A; jle */
            setD32(BIG, (uint32_t)(S32(BIG) - 2));                  /* 0x729BB */
        for (i = 0; i < cnt; i++)                                   /* 0x729C6, 0x729D1 */
            f3AF8C(CARD_BACK, i * S32(BIG) + 7, 0x196);             /* 0x729F0 */
    } else {
        f71FB0(7, 0x196, 0x23A, n);                                 /* 0x72A09 */
        f6FE68(0x249, 0x19F, n, 0, 0xF);                            /* 0x72A1F */
        fg_vbopen(S32(BACKBUF));                                    /* 0x72A2B */
    }
    setD16(COUNTER_DLG, D16(0xF5950u));                             /* 0x72A33..0x72A3B */
    fg_setcolor(5);                                                 /* 0x72A41 */
    for (r = S32(CSCROLL); S32(CSCROLL) + 5 > r && r < S32(NOFFERS); r++) {   /* 0x72A4E, 0x72CB2..0x72CCC */
        row = r - S32(CSCROLL);                                     /* 0x72CDD */
        a = S8(OFFER(r) + 0x12u);                                   /* 0x72D02 dword [21r + 0x110EDB] sar 24 */
        f3AF8C(TOKEN(a), (0x35 - TOKEN_W(a)) / 2 + 3, 50 * row + 0x6C);   /* 0x72D49 */
        f3A0B0(3, 50 * row + 0x6C + TOKEN_H(a) + 2, 0x35, NAME(a)); /* 0x72DB0 */
        for (j = 0; S16(NBLUFF) + 2 > j; j++) {                     /* 0x72DB5, 0x72AAB */
            c = S8(OFFER(r) + (uint32_t)j);                         /* 0x72ACF */
            if (c == 0)                                             /* 0x72B06 */
                f3AF8C(CARD_EMPTY, 39 * j + 0x39, 50 * row + 0x65); /* 0x72A99, 0x72AA0 */
            else if (j < 2)                                         /* 0x72B0D */
                f3AF8C(CARD(c), 39 * j + 0x39, 50 * row + 0x65);    /* 0x72B16..0x72AA0 */
            else
                f3AF8C(CARD(c), 39 * j + 0x39 + shift20(), 50 * row + 0x65);   /* 0x72A56..0x72AA0 */
        }
        for (j = 0; j < 5; j++) {                                   /* 0x72B2D, 0x72B44 */
            c = S8(OFFER(r) + (uint32_t)(START1(2) + j));           /* 0x72B4A dword [0xCA99F] sar 24; 0x72B67 */
            if (c == 0)                                             /* 0x72BA2 */
                f3AF8C(CARD_EMPTY, 39 * j + 0x120, 50 * row + 0x65);   /* 0x72B34, 0x72B39 */
            else
                f3AF8C(CARD(c), 39 * j + 0x120, 50 * row + 0x65);   /* 0x72BA6..0x72B39 */
        }
        b = COUNTER_DLG + 2u + 20u * (uint32_t)(S32(0xF5950u) + row);   /* 0x72BB4..0x72BCB lea [4*eax + 0x111982] */
        memcpy(DP(b), DP(0xC9DBCu), 20);                            /* 0x72BD2..0x72BD6 movsd x5 */
        setD16(b + 2u, (uint16_t)(D16(b + 2u) + (uint32_t)(50 * row)));   /* 0x72BED..0x72BFD y += 50 row */
        setD16(b + 0xEu, (uint16_t)(D16(b + 0xEu) + (uint32_t)row));   /* 0x72C11..0x72C30 id += row */
        if (S8(OFFER(r) + 0x12u) != n && S8(OFFER(r) + 0x14u) != n) /* 0x72C44, 0x72C5F */
            setD8(b + 0x10u, 1);                                    /* 0x72C80 */
        else
            setD8(b + 0x10u, 0);                                    /* 0x72C9E */
        setD16(COUNTER_DLG, (uint16_t)(D16(COUNTER_DLG) + 1u));     /* 0x72CA5 inc word [0x111980] */
    }
    snprintf(buf, sizeof buf, DSTR(0xB90FCu), (int)S16(SESSION));   /* 0x72DD9 "Make Counter Offers - Session %d" */
    f3A18C(0, 6, 0x280, 0xB0, buf);                                 /* 0x72DF4 */
    f3A18C(0, 0x1E, 0x280, 0x23, NAME(n));                          /* 0x72E15 */
    fg_setcolor(0xD);                                               /* 0x72E1C */
    f3A044(0x52, 0x51, DSTR(0xB911Du));                             /* 0x72E33 "The offers..." */
    f3A044(0x139, 0x51, DSTR(0xB912Bu));                            /* 0x72E47 "They want..." */
    fg_vbclose();                                                   /* 0x72E4C */
    f3BFBC(COUNTER_DLG, 0x103, S8(TRADES) != 0 ? 1 : 0);            /* 0x72E51..0x72E7A */
    f3B05C(COUNTER_DLG);                                            /* 0x72E84 */
    if (S32(CSCROLL) > 0)                                           /* 0x72E89 jle */
        f3C314(COUNTER_DLG, 0x85);                                  /* 0x72E9C */
    else
        f3C3D0(COUNTER_DLG, 0x85);                                  /* 0x72EAD */
    if (S32(CSCROLL) + 5 < S32(NOFFERS))                            /* 0x72EC0 jge */
        f3C314(COUNTER_DLG, 0x86);                                  /* 0x72ECE */
    else
        f3C3D0(COUNTER_DLG, 0x86);                                  /* 0x72EDF */
    f3CC14();                                                       /* 0x72EE4 */
}

/* 0x72EF4 one counter-offer by n to the offer row -0x26 - code */
void f72EF4(int code, int n)
{
    player_list();                                                  /* 0x72F0C..0x72F56 */
    f783B8(RESPOND_DLG, S32(0xF5954u));                             /* 0x72F63 */
    memset(DP(ROW(S16(CROW))), 0, 8);                               /* 0x72F7F */
    f72FC4(code, n);                                                /* 0x72F8F */
    setD32(GROUP, 0xFFFFFFFFu);                                     /* 0x72F94 */
    f70A6C(-0x11, n);                                               /* 0x72FA2 */
    f7109C(n, RESPOND_DLG);                                         /* 0x72FAF */
    setD16(CROW, (uint16_t)(D16(CROW) + 1u));                       /* 0x72FB4 inc word [0x112548] */
}

/* 0x72FC4 the Respond To Offer screen (n answers the offer row -0x26 - code) */
void f72FC4(int code, int n)
{
    char    buf[100];                                               /* [ebp-0x88] */
    int32_t i, j;                                                   /* [ebp-0x20], [ebp-0x1C] */
    int8_t  c;                                                      /* [ebp-0x14] */
    uint32_t o;

    setD32(OFFER_IDX, (uint32_t)(-0x26 - code + S32(CSCROLL)));     /* 0x72FDE..0x72FEE */
    setD8(PICTURE, 0xF);                                            /* 0x72FFA */
    setD32(MODE, 1);                                                /* 0x73005 */
    f3CD6C(0xF);                                                    /* 0x7300B */
    background_to_back();                                           /* 0x73033 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x73042 */
    for (i = 0; i <= 0x11; i++)                                     /* 0x7304A, 0x73057 */
        f3AF8C(0xF4C1Cu + 8u * (uint32_t)i + 8u, 40 * (i % 2) + 0x235, 47 * (i / 2) + 0x18);   /* 0x730A9 (idiv) */
    o = OFFER(S32(OFFER_IDX));
    for (j = 0; S16(NBLUFF) + 2 > j; j++) {                         /* 0x730B2, 0x73116 */
        c = S8(o + (uint32_t)j);                                    /* 0x7313D */
        if (c == 0)                                                 /* 0x73160 */
            f3AF8C(CARD_EMPTY, 39 * j + 0x6D, 0x75);                /* 0x730FF..0x7310B */
        else if (j < 2)                                             /* 0x73165 */
            f3AF8C(CARD(c), 39 * j + 0x6D, 0x75);                   /* 0x7316E..0x7310B */
        else
            f3AF8C(CARD(c), 39 * j + 0x6D + shift20(), 0x75);       /* 0x730B7..0x7310B */
    }
    for (j = 0; j < 5; j++) {                                       /* 0x73187, 0x731A3 */
        c = S8(o + (uint32_t)(START1(2) + j));                      /* 0x731A9 dword [0xCA99F] sar 24; 0x731CB */
        if (c == 0)                                                 /* 0x731F2 */
            f3AF8C(CARD_EMPTY, 39 * j + 0x153, 0x75);               /* 0x7318E..0x73198 */
        else
            f3AF8C(CARD(c), 39 * j + 0x153, 0x75);                  /* 0x731F4..0x73198 */
    }
    for (i = 0; S16(NBLUFF) > i; i++) {                             /* 0x73207, 0x73214 */
        f3AF8C(CARD_EMPTY, 39 * i + 0x166, 0xF2);                   /* 0x73242 */
        setD16(0x11154Cu + 8u * (uint32_t)i, (uint16_t)(39 * i + 0x166));   /* 0x73253 */
        setD16(0x11154Eu + 8u * (uint32_t)i, 0xF2);                 /* 0x7325E */
        setD16(0x111550u + 8u * (uint32_t)i, 0x20);                 /* 0x7326B */
        setD16(0x111552u + 8u * (uint32_t)i, 0x2E);                 /* 0x73278 */
    }
    for (j = 0; j < 2; j++) {                                       /* 0x73284, 0x73291 */
        f3AF8C(CARD_EMPTY, 39 * j + 0x6D, 0xF2);                    /* 0x732B5 */
        setD16(0x111524u + 8u * (uint32_t)j, (uint16_t)(39 * j + 0x6D));   /* 0x732C4 */
        setD16(0x111526u + 8u * (uint32_t)j, 0xF2);                 /* 0x732CF */
        setD16(0x111528u + 8u * (uint32_t)j, 0x20);                 /* 0x732DC */
        setD16(0x11152Au + 8u * (uint32_t)j, 0x2E);                 /* 0x732E9 */
    }
    for (j = 0; S16(NBLUFF) > j; j++) {                             /* 0x732F5, 0x73302 */
        f3AF8C(CARD_EMPTY, 39 * j + 0xC7, 0xF2);                    /* 0x73330 */
        setD16(0x111534u + 8u * (uint32_t)j, (uint16_t)(39 * j + 0xC7));   /* 0x73341 */
        setD16(0x111536u + 8u * (uint32_t)j, 0xF2);                 /* 0x7334C */
        setD16(0x111538u + 8u * (uint32_t)j, 0x20);                 /* 0x73359 */
        setD16(0x11153Au + 8u * (uint32_t)j, 0x2E);                 /* 0x73366 */
    }
    setD16(0x11158Cu, 7);                                           /* 0x73386 */
    setD16(0x11158Eu, 0x196);                                       /* 0x7338D */
    setD16(0x111590u, 0x1F4);                                       /* 0x73394 */
    setD16(0x111592u, 0x2E);                                        /* 0x7339B */
    f71FB0(7, 0x196, 0x1F4, n);                                     /* 0x733B3 */
    snprintf(buf, sizeof buf, DSTR(0xB9138u), (int)S16(SESSION));   /* 0x733D2 "Respond To Offer - Session %d" */
    f3A18C(0, 8, 0x280, 0xB0, buf);                                 /* 0x733ED */
    f3A18C(0, 0x1E, 0x280, 0x23, NAME(n));                          /* 0x7340E */
    fg_setcolor(0xD);                                               /* 0x73415 */
    snprintf(buf, sizeof buf, DSTR(0xB9156u), NAME(S8(OFFER(S32(OFFER_IDX)) + 0x12u)));   /* 0x73450 "%s offers for trade..." */
    f3A044(0x6D, 0x61, buf);                                        /* 0x73462 */
    f3A044(0x153, 0x61, DSTR(0xB916Du));                            /* 0x73476 "... and wants ..." */
    f6FB2C();                                                       /* 0x7347B */
    f6FE68(0x203, 0x19F, n, 0, 0xF);                                /* 0x73491 (the back buffer still open) */
    fg_vbclose();                                                   /* 0x73496 */
    f3BFBC(RESPOND_DLG, 0x103, S8(TRADES) != 0 ? 1 : 0);            /* 0x7349B..0x734C4 */
    f3B05C(RESPOND_DLG);                                            /* 0x734CE */
    f3CC14();                                                       /* 0x734D3 */
}

/* ================================================================================================================ */
/* ACCEPT TRADES                                                                                                     */
/* ================================================================================================================ */

/* 0x734E4 ACCEPT TRADES */
void f734E4(void)
{
    int32_t  m, r, j, cnt, p, code;                                 /* [ebp-0x2C], [ebp-0x2C], [ebp-0x28], [ebp-0x20], [ebp-0x24], [ebp-0x1C] */
    int32_t  cur;                                                   /* [ebp-0x30] */
    uint32_t saved;                                                 /* [ebp-0x18] */

    if (S32(NOFFERS) == 0)                                          /* 0x734F9 */
        return;
    if (S16(SESSION) == 1)                                          /* 0x73506..0x73511 */
        for (m = 1; m <= 9; m++)                                    /* 0x73513 (m = eax = 1), 0x7351E */
            if (S8(NATION(m) + 0x9Au) == 2)                         /* 0x73534 */
                memset(DP(NATION(m) + 0xBEu), 0, 9);                /* 0x73551 */
    f783B8(ACCEPT_DLG, S32(0xF5958u));                              /* 0x73565 */
    for (r = 0; S16(CROW) > r; r++) {                               /* 0x7356A, 0x735F2 */
        for (j = 0; j < 3; j++)                                     /* 0x73601, 0x73578 */
            setD8(ROW(r) + 8u + (uint32_t)j, D8(ROW(r) + (uint32_t)j));   /* 0x7359A */
        cnt = 0;                                                    /* 0x735A4 */
        for (j = 0; j < 3; j++)                                     /* 0x735A7, 0x735B8 */
            if (D8(ROW(r) + 8u + (uint32_t)cnt) != 0)               /* 0x735C1 [16r + cnt + 0x111120]: cnt as index */
                cnt++;
        if ((D8(FLAGS2) & 0x20) == 0)                               /* 0x735D3 */
            f76F70(cnt, DP(ROW(r) + 8u));                           /* 0x735E7 (game_rand) */
    }
    setD16(NBLUFF, 1);                                              /* 0x73612 */
    setD32(GROUP, 0);                                               /* 0x73619 */
    setD32(MODE, 0);                                                /* 0x7361F */
    setD8(PICTURE, 2);                                              /* 0x73625 */
    for (p = 0; p < S32(PLAYERS_N); p++) {                          /* 0x7362B, 0x738AB */
        cur = S16(PLAYER_LIST + 2u * (uint32_t)p);                  /* 0x738BD dword [2p + 0xF06AE] sar 16 */
        setD32(CUR_NATION, (uint32_t)cur);                          /* 0x738CD */
        if (S8(NATION(cur) + 0x9Cu) == 0)                           /* 0x738D9 dword [.. + 0xF1565] sar 24 */
            continue;
        setD32(ASCROLL, 0);                                         /* 0x738E8 */
        if (S8(NATION(cur) + 0x9Au) != 3) {                         /* 0x738FE jne 0x7384F: a computer */
            f73D4C(cur);                                            /* 0x73852 */
            f7401C(cur, 0);                                         /* 0x7385C */
            saved = D32(CUR_DIALOG);                                /* 0x73861 */
            if (S16(SHOW_ALL) == 0) {                               /* 0x73869 */
                f3C940(saved);                                      /* 0x73875 */
                f78A20(2000);                                       /* 0x7387F */
            }
            f6B0EC(cur);                                            /* 0x73887 (M7) */
            setD32(CUR_DIALOG, saved);                              /* 0x73896 */
            if (S16(SHOW_ALL) == 0)                                 /* 0x7389B */
                f3C7A0(saved);                                      /* 0x738A0 */
            continue;
        }
        if ((D8(FLAGS) & PBEM_BIT) != 0 && S8(LOCAL_PBEM) != cur)   /* 0x73907..0x73922 (PBEM kept literal) */
            continue;
        f73D4C(cur);                                                /* 0x73927 */
        if (S8(HUMANS) != 1)                                        /* 0x7392C dword [0xE161D] sar 24; je */
            f7401C(cur, 0);                                         /* 0x7393E */
        f6FBDC(cur, 1);                                             /* 0x7394B */
        f7401C(cur, 1);                                             /* 0x73958 */
        for (;;) {
            f3CD6C(S8(PICTURE));                                    /* 0x73967 */
            code = f78BA4(ACCEPT_DLG, 0);                           /* 0x73971 (edx = 0 kept over f3CD6C) */
            if (code == -100)                                       /* 0x7397C */
                continue;
            if (code == -1)                                         /* 0x73984 */
                break;
            if (code == -0x2C) {                                    /* 0x73636 NEXT PAGE */
                setD32(ASCROLL, (uint32_t)(S32(ASCROLL) + 3));      /* 0x7363B */
                f7401C(cur, 1);                                     /* 0x7364A */
            } else if (code == -0x2B) {                             /* 0x73657 PREV PAGE */
                setD32(ASCROLL, (uint32_t)(S32(ASCROLL) - 3));      /* 0x7365C */
                f7401C(cur, 1);                                     /* 0x7366B */
            } else if (code <= -0x36 && code >= -0x3D) {            /* 0x73678 jg; 0x7367D jl: a player button */
                setD32(ASCROLL, 0);                                 /* 0x7368A */
                cur = S16(PLAYER_LIST + 2u * (uint32_t)(-0x36 - code));   /* 0x73693 dword [2i + 0xF06AE] sar 16 */
                f73D4C(cur);                                        /* 0x736A3 */
                f7401C(cur, 1);                                     /* 0x736B0 */
            } else if (code <= -0x2F && code >= -0x31 && (D8(FLAGS) & PBEM_BIT) == 0) {   /* 0x736BD, 0x736C2, 0x736C8 */
                f75074(-0x2F - code + S32(ASCROLL));                /* 0x736E3 */
                break;                                              /* 0x736E8 jmp 0x738A5: the next player */
            } else if ((D8(FLAGS) & PBEM_BIT) != 0) {               /* 0x736ED (PBEM only, dropped) */
                if (code <= -0x2F && code >= -0x31)                 /* 0x736FD, 0x73702 */
                    f7399C(-0x2F - code, 0);                        /* 0x73845 */
                else if (code <= -0x97 && code >= -0x99)            /* 0x7371A, 0x73721 */
                    f7399C(-0x97 - code, 1);
                else if (code <= -0x9A && code >= -0x9C)            /* 0x73742, 0x73747 */
                    f7399C(-0x9A - code, 2);                        /* 0x7375F */
                else if (code <= -0x9D && code >= -0x9F)            /* 0x7376F, 0x73773 */
                    f7399C(-0x9D - code, 3);                        /* 0x7378B */
                else if (code <= -0xA0 && code >= -0xA2)            /* 0x73798, 0x737A0 */
                    f7399C(-0xA0 - code, 4);                        /* 0x737B8 */
                else if (code <= -0xA3 && code >= -0xA5)            /* 0x737C5, 0x737CC */
                    f7399C(-0xA3 - code, 5);                        /* 0x737E4 */
                else if (code <= -0xA6 && code >= -0xA8)            /* 0x737F1, 0x737F9 */
                    f7399C(-0xA6 - code, 6);                        /* 0x73811 */
                else if (code <= -0xA9 && code >= -0xAB)            /* 0x7381E, 0x73829 */
                    f7399C(-0xA9 - code, 7);                        /* 0x73845 */
            }
        }
    }
}

/* 0x73D4C n's response list: every counter-offer row answering n, with the gain it would bring, sorted */
void f73D4C(int n)
{
    const uint32_t rec = NATION(n);                                 /* 0x73D72 [ebp-0x18] */
    int32_t v0;                                                     /* [ebp-0x24] */
    int32_t r, j, m, k;                                             /* [ebp-0x30], [ebp-0x1C], [ebp-0x28] */
    uint8_t tmp[20];                                                /* [ebp-0x44] */

    setD16(NRESP, 0);                                               /* 0x73D81 */
    memset(DP(0x111498u), 0, 0x8C);                                 /* 0x73D8A (7 records, no bound: see the file header) */
    f194E4(n);                                                      /* 0x73D92 */
    v0 = f195D8(n);                                                 /* 0x73D9A */
    for (r = 0; S16(CROW) > r; r++) {                               /* 0x73DA4, 0x73E29 */
        if (S8(ROW(r) + 0xDu) != n)                                 /* 0x73E40 dword [16r + 0x111122] sar 24 */
            continue;
        k = S16(NRESP);                                             /* 0x73E50 dword [0x112566] sar 16 */
        setD8(RESP(k) + 0x12u, (uint8_t)r);                         /* 0x73E68 */
        setD8(RESP(k) + 0x13u, 0);                                  /* 0x73E70 */
        memcpy(DP(RESP(k)), DP(ROW(r)), 16);                        /* 0x73E94..0x73E9B */
        copy_nation(NATION(0), rec);                                /* 0x73EB5..0x73EB9 */
        f75CA8(S8(rec + 0x9Cu), NATION(0));                         /* 0x73ECB dword [rec + 0x99] sar 24 */
        f75CA8(S8(rec + 0x9Du), NATION(0));                         /* 0x73EE1 */
        f75CA8(S8(rec + 0xA6u), NATION(0));                         /* 0x73EF7 */
        f75CA8(S8(rec + 0xA7u), NATION(0));                         /* 0x73F0D */
        f75CA8(S8(rec + 0xA8u), NATION(0));                         /* 0x73F23 */
        for (j = 0; j < 5; j++)                                     /* 0x73F2A, 0x73DB2 */
            f75D84(S8(RESP(S16(NRESP)) + 8u + (uint32_t)j), NATION(0), NATION(0));   /* 0x73DD5, 0x73DE0 */
        f194E4(0);                                                  /* 0x73DE9 */
        k = S16(NRESP);                                             /* 0x73DFE */
        setD16(NRESP, (uint16_t)(k + 1));                           /* 0x73E14 */
        setD16(RESP(k) + 0x10u, (uint16_t)(f195D8(0) - v0));        /* 0x73DF0, 0x73E05, 0x73E1B */
    }
    for (m = S16(NRESP) - 2; m >= 0; m--)                           /* 0x73F32..0x73F44, 0x73F51..0x73F5B */
        for (j = 0; j <= m; j++)                                    /* 0x73F4C, 0x73F6E cmp; jg */
            if (S16(RESP(j + 1) + 0x10u) > S16(RESP(j) + 0x10u)) {  /* 0x73F92..0x73FA4 cmp dx, cx; jle */
                memcpy(tmp, DP(RESP(j + 1)), 20);                   /* 0x73FBD */
                memcpy(DP(RESP(j + 1)), DP(RESP(j)), 20);           /* 0x73FEB */
                memcpy(DP(RESP(j)), tmp, 20);                       /* 0x74006 */
            }
}

/* the three cards at y, x0 + 39j, as 0x72A56..0x72B28: an empty slot, a truly stated card, or a bluff shifted */
static void three_cards(uint32_t src, int32_t x0, int32_t y)
{
    int32_t j;
    int8_t  c;

    for (j = 0; S16(NBLUFF) + 2 > j; j++) {
        c = S8(src + (uint32_t)j);
        if (c == 0)
            f3AF8C(CARD_EMPTY, 39 * j + x0, y);
        else if (j < 2)
            f3AF8C(CARD(c), 39 * j + x0, y);
        else
            f3AF8C(CARD(c), 39 * j + x0 + shift20(), y);
    }
}

/* 0x7401C the Accept Trades screen of n (mode 0: a first look / a computer; 1: the human's choice) */
void f7401C(int n, int mode)
{
    char     buf[100];                                              /* [ebp-0xA0] */
    const uint32_t rec = NATION(n);                                 /* 0x74047 [ebp-0x18] */
    int32_t  ro;                                                    /* [ebp-0x28] */
    int32_t  cnt, i, j, r, row, b, idx, any;                        /* [ebp-0x30], [ebp-0x38], [ebp-0x1C], [ebp-0x2C], [ebp-0x38] */
    int8_t   c;                                                     /* [ebp-0x14] */

    if (S8(rec + 0x9Au) == 2 || mode == 0)                          /* 0x7405A, 0x7405F */
        ro = 1;                                                     /* 0x74065 */
    else
        ro = S32(CUR_NATION) != n ? 1 : 0;                          /* 0x7406E setne */
    setD8(PICTURE, 0xD);                                            /* 0x74088 */
    setD16(NBLUFF, 1);                                              /* 0x74093 */
    f3CD6C(0xD);                                                    /* 0x7409A */
    background_to_back();                                           /* 0x740C1 */
    f74D54();                                                       /* 0x740C9 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x740DF */
    setD16(0x111590u, 0x23A);                                       /* 0x740F1 */
    setD16(0x111592u, 0x2E);                                        /* 0x740F8 */
    setD16(0x11158Cu, 7);                                           /* 0x740FF */
    setD16(0x11158Eu, 0x196);                                       /* 0x74106 */
    if (ro != 0 || mode == 0) {                                     /* 0x74110 jne; 0x74114 jne 0x7417F */
        setD32(BIG, 0x28);                                          /* 0x7411A */
        cnt = f76F14(n);                                            /* 0x74127 */
        while (S32(BIG) * cnt > 0x23A)                              /* 0x7412F */
            setD32(BIG, (uint32_t)(S32(BIG) - 2));                  /* 0x7413F */
        for (i = 0; i < cnt; i++)                                   /* 0x74148, 0x74155 */
            f3AF8C(CARD_BACK, i * S32(BIG) + 7, 0x196);             /* 0x74178 */
    } else {
        f707A0(n);                                                  /* 0x74182 */
        f6FE68(0x249, 0x19F, n, ro, 0xF);                           /* 0x74196 (ecx = ro = 0, kept over f707A0) */
        fg_vbopen(S32(BACKBUF));                                    /* 0x741A2 */
        fg_setcolor(0xD);                                           /* 0x741AC */
        f3A044(7, 0x180, DSTR(0xB91A8u));                           /* 0x741C3 "Does not" */
        f3A044(7, 0x18A, DSTR(0xB91B1u));                           /* 0x741D7 "include initial offer" */
    }
    if (ro == 0 || (S8(rec + 0x9Au) == 2 && S16(SHOW_ALL) != 0)) {  /* 0x741DC..0x74203 */
        for (j = 0; j < 2; j++) {                                   /* 0x74209, 0x74216 */
            c = (int8_t)D8(rec + 0x9Cu + (uint32_t)(START1(0) + j));   /* 0x7422A */
            f3AF8C(CARD(c), 39 * j + 0xCF, 0x58);                   /* 0x7425B (no test for an empty slot) */
        }
        for (i = 0; (int32_t)S8(0xCA9A7u) > i; i++) {               /* 0x74262, 0x7426F dword [0xCA9A4] sar 24 */
            c = (int8_t)D8(rec + 0x9Cu + (uint32_t)((int32_t)S8(0xCA9A3u) + i));   /* 0x7428E dword [0xCA9A0] sar 24 */
            if (c == 0)                                             /* 0x7429B */
                continue;
            f3AF8C(CARD(c), 39 * i + 0x133, 0x58);                  /* 0x742C7 */
        }
    } else {
        idx = -100;                                                 /* 0x742CE [ebp-0x24] */
        for (i = 0; i < S32(NOFFERS); i++)                          /* 0x742D5, 0x742E2 */
            if (S8(OFFER(i) + 0x12u) == n) {                        /* 0x74306 */
                idx = i;                                            /* 0x7430D */
                break;
            }
        if (idx != -100) {                                          /* 0x74310 */
            three_cards(OFFER(idx) + 0xDu, 0xCF, 0x58);             /* 0x7431A..0x743F4 (the public copy +0x0D) */
        } else {
            for (j = 0; S16(NBLUFF) + 2 > j; j++)                   /* 0x743F6, 0x74403 */
                f3AF8C(CARD_EMPTY, 39 * j + 0xCF, 0x58);            /* 0x74434 */
        }
    }
    setD16(SHOWN, 0);                                               /* 0x74443 */
    setD16(ACCEPT_DLG, D16(0xF595Cu));                              /* 0x7443D, 0x7444A */
    for (r = S32(ASCROLL); S32(ASCROLL) + 3 > r && S16(NRESP) > r; r++) {   /* 0x74455, 0x749A6..0x749C2 */
        row = r - S32(ASCROLL);                                     /* 0x749D3 [ebp-0x38] */
        b = S8(RESP(r) + 0xEu);                                     /* 0x749FB dword [20r + 0x1114A3] sar 24 */
        f3AF8C(TOKEN(b), (0x35 - TOKEN_W(b)) / 2 + 0x39, 50 * row + 0xC3);   /* 0x74A42 */
        f3A0B0(0x39, 50 * row + 0xC3 + TOKEN_H(b) + 2, 0x35, NAME(b));   /* 0x74AAB */
        three_cards(RESP(r) + 8u, 0x81, 50 * row + 0xBE);           /* 0x74AB0, 0x744B4..0x74534 */
        setD16(SHOWN, (uint16_t)(D16(SHOWN) + 1u));                 /* 0x74539 inc word [0x112546] */
        if (ro != 0)                                                /* 0x74540 jne 0x749A0 */
            continue;
        snprintf(buf, sizeof buf, DSTR(0xB91C7u), (int)S16(RESP(r) + 0x10u));   /* 0x74578 "%+d" (dword [20r + 0x1114A6] sar 16) */
        f3A0B0(((D8(FLAGS) & PBEM_BIT) != 0 ? 0x95 : 0xA9) + 0x81, 50 * row + 0xD2, 0x5C, buf);   /* 0x74599..0x745B3 */
        f3BFBC(ACCEPT_DLG, (int16_t)(row + 0x87), 1);               /* 0x745CD */
        f3C0D8(ACCEPT_DLG, (int16_t)(row + 0x87), 1);               /* 0x745E7 */
        f3B914(ACCEPT_DLG, (int16_t)(row + 0x87),                   /* 0x74630 */
               ((D8(FLAGS) & PBEM_BIT) != 0 && S8(RESP(r) + 0x13u) == 0) ? 1 : 0);   /* 0x745EC..0x74628 */
        if ((D8(FLAGS) & PBEM_BIT) != 0) {                          /* 0x74635 */
            f3BFBC(ACCEPT_DLG, (int16_t)(row + 0xEC), 1);           /* 0x74657 */
            for (j = 0; j < 7; j++)                                 /* 0x74671..0x7470D: 0xEC, 0xEF, .. 0xFE */
                f3C0D8(ACCEPT_DLG, (int16_t)(row + 0xEC + 3 * j), 1);
        }
        f3B914(ACCEPT_DLG, (int16_t)(row + 0xEC), S8(RESP(r) + 0x13u) == 1 ? 1 : 0);   /* 0x74712..0x74749 */
        for (j = 1; j < 7; j++) {                                   /* 0x7474E..0x7499B, unrolled six times */
            if (S16(NRESP) <= j)                                    /* cmp eax, j; jle */
                continue;
            f3BFBC(ACCEPT_DLG, (int16_t)(row + 0xEC + 3 * j), 1);
            f3B914(ACCEPT_DLG, (int16_t)(row + 0xEC + 3 * j), S8(RESP(r) + 0x13u) == j + 1 ? 1 : 0);
        }
    }
    if (S16(SHOWN) == 0) {                                          /* 0x74ABA */
        f39F90(2);                                                  /* 0x74ACB */
        fg_setcolor(0xD);                                           /* 0x74AD2 */
        f3A0B0(0, 0xD2, 0x280, DSTR(0xB91CBu));                     /* 0x74AEB "N O N E" */
        f39F90(1);                                                  /* 0x74AF5 */
    }
    if (ro == 0) {                                                  /* 0x74AFF */
        any = 0;                                                    /* 0x74B05 [ebp-0x20] */
        for (i = 0; S16(NRESP) > i; i++)                            /* 0x74B08, 0x74B13 */
            if (S8(RESP(i) + 0x13u) != 0) {                         /* 0x74B36 */
                any = 1;                                            /* 0x74B3A */
                break;
            }
        if (S16(SHOWN) != 0 && any == 0) {                          /* 0x74B49, 0x74B4D */
            f3C284(ACCEPT_DLG, 0x2A, 0xB91D3u);                     /* 0x74B64 "REJECT ALL OFFERS" */
            setD16(REJECT_ALL, 0);                                  /* 0x74B69 */
        } else if ((D8(FLAGS) & PBEM_BIT) != 0 && any == 1) {       /* 0x74B7B, 0x74B81 */
            f3C284(ACCEPT_DLG, 0x2A, 0xB91E5u);                     /* 0x74B97 "ACCEPT PREFERRED ORDER" */
            setD16(REJECT_ALL, 1);                                  /* 0x74B9C */
        } else {
            f3C284(ACCEPT_DLG, 0x2A, 0xB91FCu);                     /* 0x74BBB "CONTINUE" */
        }
    } else {
        f3C284(ACCEPT_DLG, 0x2A, 0xB9205u);                         /* 0x74BBB "CONTINUE" (the second copy) */
    }
    snprintf(buf, sizeof buf, DSTR(0xB920Eu), (int)S16(SESSION));   /* 0x74BDA "Accept Trades - Session %d" */
    f3A18C(0, 8, 0x280, 0xB0, buf);                                 /* 0x74BF5 */
    f3A18C(0, 0x1E, 0x280, 0x23, NAME(n));                          /* 0x74C16 */
    fg_setcolor(0xD);                                               /* 0x74C1D */
    if (ro == 0) {                                                  /* 0x74C25 */
        f3A044(0x133, 0x4C, DSTR(0xB9229u));                        /* 0x74C3E "Bluffs replaced by..." */
        f3A044(0xCF, 0x4C, DSTR(0xB923Fu));                         /* 0x74C52 "Truly Stated" */
        f3A044(0xD9, 0x3A, DSTR(0xB924Cu));                         /* 0x74C66 "You initially offered..." */
        if ((D8(FLAGS) & PBEM_BIT) != 0)                            /* 0x74C71 */
            f3A0B0(0x17A, 0xAA, 0xF6, DSTR(0xB9265u));              /* 0x74C8A "Trade Acceptance Order Preference" */
        f3A044(((D8(FLAGS) & PBEM_BIT) != 0 ? 0xA1 : 0xB5) + 0x81, 0xAA, DSTR(0xB9287u));   /* 0x74CC6 "If Not Lying" */
    } else {
        f3A044(0xD9, 0x4C, DSTR(0xB9294u));                         /* 0x74CC6 "Offers..." */
    }
    if (ro != 0 && !g_original)
        /* BUG(1995) text (Erik's ruling, a missing article): "The responses to African offer..." -> "... to the
           African offer...". "your" keeps the original's format. */
        snprintf(buf, sizeof buf, "The responses to the %s offer...", ADJ(n));
    else
        snprintf(buf, sizeof buf, DSTR(0xB92A3u), ro == 0 ? DSTR(0xB929Eu) : ADJ(n));   /* 0x74CF5 "The responses to %s offer..." */
    f3A044(0x31, 0xAA, buf);                                        /* 0x74D07 */
    fg_vbclose();                                                   /* 0x74D0C */
    f3B05C(ACCEPT_DLG);                                             /* 0x74D16 */
    f75000();                                                       /* 0x74D1B */
    if (ro == 0)                                                    /* 0x74D20 */
        f3C314(ACCEPT_DLG, 0x2A);                                   /* 0x74D30 */
    else
        f3C3D0(ACCEPT_DLG, 0x2A);                                   /* 0x74D41 */
    f3CC14();                                                       /* 0x74D46 */
}

/* 0x74D54 the three response rows' buttons of 0x112158 reset */
void f74D54(void)
{
    int32_t row;                                                    /* [ebp-0xC] */
    int32_t j;

    for (row = 0; row < 3; row++) {                                 /* 0x74D66, 0x74E81 */
        f3BFBC(ACCEPT_DLG, (int16_t)(row + 0x87), 0);               /* 0x74E9E */
        f3C0D8(ACCEPT_DLG, (int16_t)(row + 0x87), 2);               /* 0x74EB9 */
        if ((D8(FLAGS) & PBEM_BIT) != 0) {                          /* 0x74ECD jne 0x74D70 */
            f3C284(ACCEPT_DLG, (int16_t)(row + 0x87), 0xB92CDu);    /* 0x74D7D "REJECT" */
            f3BB48(ACCEPT_DLG, (int16_t)(row + 0x87), 0x17A);       /* 0x74D98 */
            f3BD80(ACCEPT_DLG, (int16_t)(row + 0x87), 0x32);        /* 0x74DB3 */
            f3C1F4(ACCEPT_DLG, (int16_t)(row + 0x87), 1);           /* 0x74DCE */
        } else {
            f3C284(ACCEPT_DLG, (int16_t)(row + 0x87), 0xB92C0u);    /* 0x74EE3 "ACCEPT OFFER" */
            f3BB48(ACCEPT_DLG, (int16_t)(row + 0x87), 0x198);       /* 0x74EFE */
            f3BD80(ACCEPT_DLG, (int16_t)(row + 0x87), 0x6E);        /* 0x74F19 */
            f3C1F4(ACCEPT_DLG, (int16_t)(row + 0x87), 0);           /* 0x74F31 */
            for (j = 0; j < 7; j++)                                 /* 0x74F4C..0x74FEE: 0xEC, 0xEF, .. 0xFE */
                f3C0D8(ACCEPT_DLG, (int16_t)(row + 0xEC + 3 * j), 2);
        }
        for (j = 0; j < 7; j++)                                     /* 0x74DE6..0x74E76 */
            f3BFBC(ACCEPT_DLG, (int16_t)(row + 0xEC + 3 * j), 0);
    }
}

/* 0x75000 the scroll buttons of the Accept screen */
void f75000(void)
{
    if (S32(ASCROLL) > 0)                                           /* 0x7500E jle */
        f3C314(ACCEPT_DLG, 0x85);                                   /* 0x75021 */
    else
        f3C3D0(ACCEPT_DLG, 0x85);                                   /* 0x75032 */
    if (S32(ASCROLL) + 3 < S16(NRESP))                              /* 0x75048 jge */
        f3C314(ACCEPT_DLG, 0x86);                                   /* 0x75056 */
    else
        f3C3D0(ACCEPT_DLG, 0x86);                                   /* 0x75067 */
}

/* the computers' calamity givers (0x75246..0x752BC, 0x752CF..0x75346): x passed a calamity to y */
static void passed_calamity(uint32_t recx, int32_t x, int32_t y)
{
    int32_t m;                                                      /* [ebp-0x18] */

    setD8(recx + 0xBDu + (uint32_t)y, 1);                           /* 0x7524C / 0x752DA */
    for (m = 1; m <= 9; m++) {                                      /* 0x75253, 0x75262 */
        if (S8(NATION(m) + 0x9Au) != 2)                             /* 0x75278 */
            continue;
        if (m == y)                                                 /* 0x75283 */
            continue;
        if (D8(NATION(m) + 0xBDu + (uint32_t)x) == 0)               /* 0x7528F [199m + x + 0xF1589] */
            continue;
        setD8(NATION(m) + 0xBDu + (uint32_t)x, 0);                  /* 0x752A5 */
        setD8(NATION(m) + 0xBDu + (uint32_t)y, 1);                  /* 0x752B5 */
    }
}

/* a computer's trust in its partner (0x75423..0x75491, 0x754A5..0x75513): += table[level] * honesty, -15..100 */
static void trust(uint32_t rec, int32_t other, uint32_t honesty)
{
    uint32_t lvl = (uint32_t)((uint16_t)(D16(FLAGS) << 4) >> 14);   /* 0x75429..0x75438 shl eax, 4; shr ax, 0xE */
    int8_t   d = (int8_t)(S8(0xCA994u + 4u * lvl) * S8(honesty));   /* 0x75443 al; 0x7544A imul bl (al) */
    uint32_t a = rec + 0xB1u + (uint32_t)other;

    setD8(a, (uint8_t)(D8(a) + (uint8_t)d));                        /* 0x7544C add byte [..], al */
    if (S8(a) > 0x64)                                               /* 0x75458 dword [.. + 0xAE] sar 24; jle */
        setD8(a, 0x64);                                             /* 0x7546C */
    if (S8(a) < -0xF)                                               /* 0x75482 jge */
        setD8(a, (uint8_t)-0xF);                                    /* 0x7548D */
}

/* 0x75074 response i of 0x111498 carried out */
void f75074(int i)
{
    uint8_t  pre_a[199], pre_b[199];                                /* [ebp-0x1CC], [ebp-0x104] */
    const int32_t r = S8(RESP(i) + 0x12u);                          /* 0x7509B [ebp-0x30] */
    const uint32_t rec_a = NATION(S8(RESP(i) + 0xDu));              /* 0x750B4 [ebp-0x24] */
    const uint32_t rec_b = NATION(S8(RESP(i) + 0xEu));              /* 0x750DA [ebp-0x20] */
    const int32_t a = S8(rec_a + 0x3Du);                            /* 0x750F7 [ebp-0x2C] */
    const int32_t b = S8(rec_b + 0x3Du);                            /* 0x75103 [ebp-0x28] */
    int32_t  t, k, j;                                               /* [ebp-0x38], [ebp-0x18], [ebp-0x3C] */
    int32_t  idx = 0;                                               /* [ebp-0x34] (uninitialised if no record matches) */
    int      found = 0;

    memcpy(pre_a, DP(rec_a), 199);                                  /* 0x7510C..0x7511E */
    memcpy(pre_b, DP(rec_b), 199);                                  /* 0x7511F..0x75131 */
    t = S8(TRADES);                                                 /* 0x75132 dword [0x110F71] sar 24 */
    for (k = 0; k < S32(NOFFERS); k++)                              /* 0x7513F, 0x7514A */
        if (S8(OFFER(k) + 0x12u) == a) {                            /* 0x75161..0x75170 */
            idx = k;                                                /* 0x75175 */
            found = 1;
            break;
        }
    /* BUG(1995): no record for a leaves [ebp-0x34] unwritten; --original uses index 0. Fixed: no cards logged for a.
       G: unreachable (f726DC made a record for every nation with an offer). */
    for (j = 0; j < 5; j++)                                         /* 0x75178, 0x75185 */
        setD8(0x110F75u + (uint32_t)(5 * t + j),
              (found || g_original) ? D8(OFFER(idx) + 0xDu + (uint32_t)j) : 0);   /* 0x751B2 [21idx + j + 0x110ED9] */
    for (j = 0; j < 5; j++)                                         /* 0x751C0, 0x751CD */
        setD8(0x111015u + (uint32_t)(5 * t + j), D8(ROW(r) + 8u + (uint32_t)j));   /* 0x751EF [16r + j + 0x111120] */
    setD8(0x1110B5u + (uint32_t)t, (uint8_t)a);                     /* 0x75203 */
    setD8(0x1110D5u + (uint32_t)t, (uint8_t)b);                     /* 0x7520F */
    setD8(0x1110F5u + (uint32_t)t, D8(SESSION));                    /* 0x7521B mov dl, byte [0xF5476] */
    setD8(TRADES, (uint8_t)(D8(TRADES) + 1u));                      /* 0x75224 inc byte [0x110F74] */
    f75A1C(rec_a, r);                                               /* 0x75230 */
    if (S8(DETAIL + 2u) > 0x12)                                     /* 0x75235 dword [0x11256F] sar 24 */
        passed_calamity(rec_a, a, b);                               /* 0x75246..0x752BC */
    if (S8(DETAIL + 9u + 2u) > 0x12)                                /* 0x752BE dword [0x112578] sar 24 */
        passed_calamity(rec_b, b, a);                               /* 0x752CF..0x75346 */
    f75C08(rec_a, rec_b);                                           /* 0x7534E */
    f6E460(a);                                                      /* 0x75356 */
    f6E460(b);                                                      /* 0x7535E */
    f75DF0(S8(RESP(i) + 0xDu));                                     /* 0x75379 dword [20i + 0x1114A2] sar 24 */
    f75EB4(a, b);                                                   /* 0x75384 */
    f75F08(a, b);                                                   /* 0x7538F */
    if (S8(rec_a + 0x9Au) == 3)                                     /* 0x753A0 dword [rec + 0x97] sar 24 */
        f75520(0, a, b, pre_a);                                     /* 0x753B3 */
    if ((D8(FLAGS) & PBEM_BIT) == 0 && S8(rec_b + 0x9Au) == 3)      /* 0x753B8, 0x753CD */
        f75520(1, b, a, pre_b);                                     /* 0x753E3 */
    if (S8(rec_a + 0x9Au) == 2 && S8(rec_b + 0x9Au) == 2)           /* 0x753F4, 0x75405 */
        f75FB4(a);                                                  /* 0x7540D */
    if (S8(rec_a + 0x9Au) == 2)                                     /* 0x7541E */
        trust(rec_a, b, DETAIL + 9u + 8u);                          /* 0x7543D byte [0x112581] */
    if (S8(rec_b + 0x9Au) == 2)                                     /* 0x754A0 */
        trust(rec_b, a, DETAIL + 8u);                               /* 0x754BF byte [0x112578] */
}

/* the cards of one row of the Trade Details screen: count[3] slots from block + off at x 0xC8 + 39 (j + 2) (0x7570C..,
   0x75827.., 0x758D2..) */
static void detail_slots(uint32_t src, int32_t y)
{
    int32_t j;
    int32_t c;

    for (j = 0; (int32_t)S8(0xCA9A7u) > j; j++) {                   /* dword [0xCA9A4] sar 24 */
        c = S8(src + (uint32_t)j);                                  /* dword [9s + j + ..] sar 24 */
        if (c == 0)
            f3AF8C(CARD_EMPTY, 39 * (j + 2) + 0xC8, y);
        else
            f3AF8C(CARD(c), 39 * (j + 2) + 0xC8, y);
    }
}

/* the CONTINUE wait of 0x75520 / 0x75FB4 (0x759C9..0x75A12, 0x76288..0x762D1) */
static void continue_wait(void)
{
    int32_t r;

    do {
        if (f7BA64() == 0) {
            r = f78BA4(CONT_DLG, 0);
        } else {
            f78A20(S16(SRD) == 0 ? 0xFA : 0x3E8);
            r = -1;
        }
    } while (r == -100 || r != -1);
}

/* 0x75520 the Trade Details screen of side (0 offerer, 1 responder) n, who traded with p; pre = n's record before */
void f75520(int side, int n, int p, const uint8_t *pre)
{
    char    buf[100];                                               /* [ebp-0x90] */
    const int32_t o = side == 0 ? 1 : 0;                            /* 0x75545 sete [ebp-0x10] */
    int32_t j;

    setD8(PICTURE, 0xE);                                            /* 0x75550 */
    f3CD6C(0xE);                                                    /* 0x7555C */
    background_to_back();                                           /* 0x75584 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x75593 */
    snprintf(buf, sizeof buf, "%s", DSTR(0xB92D4u));                /* 0x755AC sprintf(buf, "Trade Details") */
    f3A18C(0, 6, 0x280, 0xB0, buf);                                 /* 0x755C7 */
    f3A18C(0, 0x1E, 0x280, 0x23, NAME(n));                          /* 0x755E8 */
    fg_vbclose();                                                   /* 0x755ED */
    if (side == 1) {                                                /* 0x755F2 */
        f3CC14();                                                   /* 0x755F8 */
        f6FBDC(n, 0);                                               /* 0x75602 */
    }
    fg_vbopen(S32(BACKBUF));                                        /* 0x7560E */
    f39F90(1);                                                      /* 0x7561B */
    fg_setcolor(0xD);                                               /* 0x75627 */
    f3A044(0x1E, 0x31, DSTR(0xB92E2u));                             /* 0x75643 "Pre-Trade" */
    memmove(DP(NATION(0)), pre, 199);                               /* 0x75655..0x75659 */
    f71FB0(0x37, 0x3C, 0x1CC, 0);                                   /* 0x75661 */
    f6FE68(0x203, 0x46, 0, 1, 0xD);                                 /* 0x75679 */
    fg_vbclose();                                                   /* 0x7567E */
    fg_vbopen(S32(BACKBUF));                                        /* 0x7568A */
    fg_setcolor(0xD);                                               /* 0x75694 */
    f3A3A0(0xBE, 0x8C, DSTR(0xB92ECu));                             /* 0x756AD "You traded" */
    for (j = 0; j < 2; j++)                                         /* 0x756B2, 0x756BD */
        f3AF8C(CARD(S8(DETAIL + 9u * (uint32_t)side + (uint32_t)j)), 39 * j + 0xC8, 0x78);   /* 0x756D2, 0x75705 */
    detail_slots(DETAIL + 9u * (uint32_t)side + 2u, 0x78);          /* 0x7570C..0x75781 */
    snprintf(buf, sizeof buf, DSTR(0xB92F7u), NAME(p));             /* 0x757A0 " to %s" */
    f3A3A0(0xBE, 0xB4, buf);                                        /* 0x757B2 */
    f3A3A0(0xBE, 0xDC, DSTR(0xB92FEu));                             /* 0x757C6 "You received" */
    for (j = 0; j < 2; j++)                                         /* 0x757CB, 0x757D8 */
        f3AF8C(CARD(S8(DETAIL + 9u * (uint32_t)o + (uint32_t)j)), 39 * j + 0xC8, 0xC8);   /* 0x757ED, 0x75820 */
    detail_slots(DETAIL + 9u * (uint32_t)o + 2u, 0xC8);             /* 0x75827..0x7589C */
    snprintf(buf, sizeof buf, DSTR(0xB930Bu), NAME(p));             /* 0x758BB "%s was bluffing with" */
    f3A3A0(0xFA, 0x118, buf);                                       /* 0x758CD */
    detail_slots(DETAIL + 9u * (uint32_t)o + 5u, 0x104);            /* 0x758D2..0x75947 (dword [9o + j + 0x112572] sar 24) */
    f3A044(0x1E, 0x149, DSTR(0xB9320u));                            /* 0x75958 "Post-Trade" */
    f71FB0(0x37, 0x154, 0x1CC, n);                                  /* 0x7596F */
    fg_vbclose();                                                   /* 0x75974 */
    f6FE68(0x203, 0x154, n, 1, 0xD);                                /* 0x7598D */
    f3BB48(CONT_DLG, 0x2A, 0x109);                                  /* 0x759A1 */
    f3BC64(CONT_DLG, 0x2A, 0x1AE);                                  /* 0x759B5 */
    f3B05C(CONT_DLG);                                               /* 0x759BF */
    f3CC14();                                                       /* 0x759C4 */
    continue_wait();                                                /* 0x759C9..0x75A12 */
}

/* 0x75A1C the trade detail block from the offerer's record rec and counter-offer row r */
void f75A1C(uint32_t rec, int r)
{
    int32_t i;                                                      /* [ebp-0xC] */
    int32_t s;

    memset(DP(DETAIL), 0, 0x12);                                    /* 0x75A40 */
    setD8(DETAIL + 0u, D8(rec + 0x9Cu));                            /* 0x75A4E */
    setD8(DETAIL + 1u, D8(rec + 0x9Du));                            /* 0x75A5C */
    s = (int32_t)S8(0xCA9A3u);                                      /* 0x75A61 dword [0xCA9A0] sar 24: start1[3] */
    setD8(DETAIL + 2u, D8(rec + 0x9Cu + (uint32_t)s));              /* 0x75A73 */
    setD8(DETAIL + 3u, D8(rec + 0x9Du + (uint32_t)s));              /* 0x75A83 */
    setD8(DETAIL + 4u, D8(rec + 0x9Eu + (uint32_t)s));              /* 0x75A92 */
    s = (int32_t)S8(0xCA9A1u);                                      /* 0x75A97 dword [0xCA99E] sar 24: start1[1] */
    setD8(DETAIL + 5u, D8(rec + 0x9Cu + (uint32_t)s));              /* 0x75AA9 */
    setD8(DETAIL + 6u, D8(rec + 0x9Du + (uint32_t)s));              /* 0x75AB9 */
    setD8(DETAIL + 7u, D8(rec + 0x9Eu + (uint32_t)s));              /* 0x75AC8 */
    setD8(DETAIL + 8u, 1);                                          /* 0x75AD4 */
    for (i = 2; i < 5; i++)                                         /* 0x75AD4, 0x75AE5 */
        if (D8(DETAIL + (uint32_t)i) != D8(DETAIL + 3u + (uint32_t)i)) {   /* 0x75AEE..0x75AFD cmp dl, dh */
            setD8(DETAIL + 8u, 0xFFu);                              /* 0x75B01 */
            break;
        }
    setD8(DETAIL + 9u, D8(ROW(r) + 0u));                            /* 0x75B14 */
    setD8(DETAIL + 10u, D8(ROW(r) + 1u));                           /* 0x75B25 */
    setD8(DETAIL + 11u, D8(ROW(r) + 5u));                           /* 0x75B36 */
    setD8(DETAIL + 12u, D8(ROW(r) + 6u));                           /* 0x75B47 */
    setD8(DETAIL + 13u, D8(ROW(r) + 7u));                           /* 0x75B58 */
    setD8(DETAIL + 14u, D8(ROW(r) + 2u));                           /* 0x75B69 */
    setD8(DETAIL + 15u, D8(ROW(r) + 3u));                           /* 0x75B7A */
    setD8(DETAIL + 17u, 1);                                         /* 0x75B8D */
    setD8(DETAIL + 16u, D8(ROW(r) + 4u));                           /* 0x75B93 */
    for (i = 2; i < 5; i++)                                         /* 0x75B98, 0x75BA7 */
        if (D8(DETAIL + 9u + (uint32_t)i) != D8(DETAIL + 12u + (uint32_t)i)) {   /* 0x75BB0..0x75BBF cmp dl, cl */
            setD8(DETAIL + 17u, 0xFFu);                             /* 0x75BC3 */
            break;
        }
}

/* 0x75FB4 the Trade Summary screen (the last trade, between two computers); a is stored and never used */
void f75FB4(int a)
{
    char    buf[52];                                                /* [ebp-0x60]..[ebp-0x2C] */
    const int32_t t = S8(TRADES) - 1;                               /* 0x7604B..0x76053 [ebp-0x24] */
    const int32_t y = 0xDC;                                         /* 0x76057 [ebp-0x1C] */
    int32_t  na, nb, j, c;                                          /* [ebp-0x2C], [ebp-0x28] */

    (void)a;                                                        /* 0x75FC9 mov [ebp-0x18], eax */
    setD8(PICTURE, 0xE);                                            /* 0x75FCC */
    f3CD6C(0xE);                                                    /* 0x75FD8 */
    background_to_back();                                           /* 0x76000 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x7600F */
    snprintf(buf, sizeof buf, DSTR(0xB932Bu), (int)S16(SESSION));   /* 0x7602E "Trade Summary - Session %d" */
    f3A18C(0, 6, 0x280, 0xB0, buf);                                 /* 0x76046 */
    fg_setcolor(5);                                                 /* 0x76061 */
    f39F90(2);                                                      /* 0x7606E */
    f3A0B0(0, y + 0xF, 0x280, DSTR(0xB9346u));                      /* 0x76085 "<-->" */
    f39F90(1);                                                      /* 0x7608F */
    na = S8(0x1110B5u + (uint32_t)t);                               /* 0x7609D dword [t + 0x1110B2] sar 24 */
    f3AF8C(TOKEN(na), (0x35 - TOKEN_W(na)) / 2 + 0xF5, y + 5);      /* 0x760D1 */
    f3A0B0(0xF5, y + 5 + TOKEN_H(na) + 2, 0x35, NAME(na));          /* 0x76108 */
    for (j = 0; j < 5; j++) {                                       /* 0x7610D, 0x7611A */
        c = S8(0x110F75u + (uint32_t)(5 * t + j));                  /* 0x7612F dword [5t + j + 0x110F72] sar 24 */
        if (c == 0)                                                 /* 0x7613E */
            continue;
        f3AF8C(CARD(c), 0xCB - 39 * j, y);                          /* 0x76168 */
    }
    nb = S8(0x1110D5u + (uint32_t)t);                               /* 0x76178 dword [t + 0x1110D2] sar 24 */
    f3AF8C(TOKEN(nb), (0x35 - TOKEN_W(nb)) / 2 + 0x154, y + 5);     /* 0x761AC */
    f3A0B0(0x154, y + 5 + TOKEN_H(nb) + 2, 0x35, NAME(nb));         /* 0x761E8 */
    for (j = 0; j < 5; j++) {                                       /* 0x761ED, 0x761F8 */
        c = S8(0x111015u + (uint32_t)(5 * t + j));                  /* 0x7620D dword [5t + j + 0x111012] sar 24 */
        if (c == 0)                                                 /* 0x7621C */
            continue;
        f3AF8C(CARD(c), 39 * j + 0x195, y);                         /* 0x76245 */
    }
    fg_vbclose();                                                   /* 0x7624C */
    f3BB48(CONT_DLG, 0x2A, 0x109);                                  /* 0x76260 */
    f3BC64(CONT_DLG, 0x2A, 0x1AE);                                  /* 0x76274 */
    f3B05C(CONT_DLG);                                               /* 0x7627E */
    f3CC14();                                                       /* 0x76283 */
    continue_wait();                                                /* 0x76288..0x762D1 */
}
