#ifndef TRADE_H
#define TRADE_H

/* Module TRADE of docs/M6_PLAN.md (trade.c), the human side of the three trade steps of the game loop 0x54824:
   DECLARE OFFERS 0x6F270 (phase 12, call at 0x54D02), COUNTER OFFERS 0x723DC (phase 13, 0x54DC2) and ACCEPT TRADES
   0x734E4 (phase 14, 0x54E2D), with their screens, the offer composer, the trade itself and its detail screens. The
   computers' side (TRADEAI, M7) and play by e-mail (dropped) stop with not_ported (trade.c's file comment).

   Conventions: those of ui.h, game.h and kit.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax,
   edx, ebx, ecx, then the stack; void unless a caller reads eax; dialogs and records as uint32_t link addresses).
   D = read in the code, G = inference. The data layouts are in trade.c's file comment.

   Called from outside the module (D, the call sites): 0x6F270 / 0x723DC / 0x734E4 (loop.c), 0x75074 (TRADEAI's
   0x6B0EC at 0x6B916), 0x73D4C (PBEM 0x5195C at 0x52AFF), 0x75A1C (PBEM 0x5438C at 0x5454F), 0x75520 (PBEM 0x762E0 at
   0x76396). The others are exported for port/test/trade_test.c. */

#include <stdint.h>

/* ---- the three phase routines ------------------------------------------------------------------------------------ */

void f6F270(void);   /* D: DECLARE OFFERS: player list, f783B8(0x111594, [0xF5948]), mode 0, one bluff slot, background 9;
                        each n 1..9: its offer +0x9C..+0xA8 and want count 0x1105AC[n] cleared; a human holding the
                        minimum goods composes (f6F454, f6FBDC, POINTS, f70A6C(-0x11), f7109C); a computer holding
                        max(minimum, 6) goes to TRADEAI 0x68730 (M7) */
void f723DC(void);   /* D: COUNTER OFFERS: player list, f783B8(0x111980, [0xF594C]), the rows 0x111118 cleared, the
                        public offer records (f726DC, game_rand); with offers: each human holding the minimum answers
                        on f72928 (rows -0x26..-0x2A -> f72EF4, scroll -0x2B / -0x2C, FINISHED -1 with the "another
                        round" question), each computer TRADEAI 0x698A4 (M7); then the turn order shuffled (f76F70) */
void f734E4(void);   /* D: ACCEPT TRADES: with offers: session 1 clears the computers' +0xBE..+0xC6, f783B8(0x112158,
                        [0xF5958]), each row's first three cards copied to +8 and shuffled (unless variant 0x20 of
                        [0xE1625]); then per player in turn order with an offer: a human chooses on f7401C (accept
                        -0x2F..-0x31 -> f75074, players -0x36..-0x3D, scroll -0x2B / -0x2C, -1 done), a computer
                        TRADEAI 0x6B0EC (M7) after a 2 s view */

/* ---- DECLARE OFFERS: screen and composer -------------------------------------------------------------------------- */

void f6F454(int n);                    /* D: the Declare Trade Offers screen of n (left column, chart, slot rects, hand face down, titles, labels, REVIEW 0x103 by [0x110F74]) */
void f6F95C(int frame);                /* D: the 18 commodity cards of the chart at (0x235 + 40 (i % 2), 0x18 + 47 (i / 2)); frame == 1 also f448EC each */
void f6FA54(void);                     /* D: the Declare screen's four labels, colour 5 then 7 */
void f6FB2C(void);                     /* D: the response screen's three labels at y 0xE6, colour 5 then 7 */
void f6FDC8(int n, int frame);         /* D: n's hand strip redrawn: f71D30(-1), frame == 0 f71FB0 else f721A0, pasted */
int  f70074(int c, int n);             /* D: card c into the first empty slot of the current group ([0x112558]); 1 placed, 0 full */
void f70294(int n, int lit);           /* D: the current group's slots cleared and redrawn empty (lit == 1 highlighted); bluff group also clears the replacement; record 0 = n, f707A0(n) */
void f707A0(int n);                    /* D: record 0 = n less the cards already placed (f70984), then f6FDC8(0, flag of the group) */
void f70984(int c);                    /* D: card c out of nation record 0 (twin of rules.c's f75CA8 on record 0). BUG(1995) in trade.c */
void f70A6C(int code, int n);          /* D: switch the composer to group -0x11 - code (0..4): highlight moved, the group's heading, hand / chart buttons 0x75 / 0x76 */
void f7109C(int n, uint32_t dlg);      /* D: the composer's click loop on dlg (0x111594 or 0x111D6C) until f71768 accepts DONE (-1) */
void f716BC(int c, int m);             /* D: calamity c out of nation record m (twin of f70984 / f75CA8; leaves the freed slots). BUG(1995) in trade.c */
int  f71768(int n);                    /* D: DONE: checks the offer (mode 0) or the counter-offer (mode 1); -15 with the message box, -1 accepted. BUG(1995) in trade.c */
void f71B68(const char *text);         /* D: the offer-error box with CONTINUE (dialog 0xBBBEC), "junk" screen save */
void f721A0(int x, int y, int w, int n);   /* D: n's hand face up as f71FB0, plus f448EC on each card */

/* ---- COUNTER OFFERS ---------------------------------------------------------------------------------------------- */

void f72928(int n, int up);            /* D: the Make Counter Offers screen of n (hand face down 0 / up 1), five offers from [0x11255C], row buttons built from 0xC9DBC */
void f72EF4(int code, int n);          /* D: one counter-offer by n to the offer row -0x26 - code: f72FC4, f70A6C(-0x11), f7109C(n, 0x111D6C), row count + 1 */
void f72FC4(int code, int n);          /* D: the Respond To Offer screen (mode 1, background 15, slot rects for the reply) */

/* ---- ACCEPT TRADES ------------------------------------------------------------------------------------------------ */

void f73D4C(int n);                    /* D: n's response list 0x111498 (rows answering n, the gain each would bring, sorted by gain) */
void f7401C(int n, int mode);          /* D: the Accept Trades screen of n (read-only for a computer, mode 0 or another nation than [0xF40F8]) */
void f74D54(void);                     /* D: the three response rows' buttons of 0x112158 reset (ACCEPT OFFER, radios hidden and disabled) */
void f75000(void);                     /* D: scroll buttons 0x85 / 0x86 of 0x112158 by [0x112564] and the response count */
void f75074(int i);                    /* D: carries out response i of 0x111498: the trade log, f75A1C, calamity givers, f75C08, offers withdrawn, the detail screens, the computers' trust */
void f75520(int side, int n, int p, const uint8_t *pre);   /* D: the Trade Details screen for side 0 / 1 (n traded with p; pre = n's 199-byte record before the trade: a caller's stack copy, or 0xF2A94 + 199n from PBEM) */
void f75A1C(uint32_t rec, int r);      /* D: the detail block 0x112570: side 0 from the offerer's record rec, side 1 from counter-offer row r, with the honesty bytes */
void f75FB4(int a);                    /* D: the Trade Summary screen of the last trade (between two computers); a unused */

#endif
