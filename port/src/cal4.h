#ifndef CAL4_H
#define CAL4_H

/* Module CAL4 of docs/M6_PLAN.md (cal4.c), the human side of Civil War (calamity 0x17): the calamity 0x29E34 (called by
   CAL1's phase driver 0x1E0A0, jump table 0x1E070 case 0x17, at 0x1E2B3), the Military losses 0x2AD50, the unit
   chooser 0x2B6EC ("ChooseFirstFaction", also the beneficiary's annexing screen), the human's side choice 0x2D3E0 and
   the automatic-step display 0x2EE6C (also called by the computer's 0x2C298, M7). 0x2AC94 is formulas.c's, 0x2C778
   helpers2.c's. The computers' parts (0x2C190, 0x2C298, 0x2C894, 0x2D10C, 0x2D1C0: M7) and CAL2's NationLosesUnits
   0x27FCC are not ported yet: their call sites stop the port through not_ported(address); the list is in cal4.c's
   file comment.

   cal1.c still carries a static not_ported stub named f29E34 (its handler for card 0x17): this header exports the real
   one, so that stub is to be replaced by an #include "cal4.h" (Claude, when these files are merged; this module does
   not touch cal1.c).

   Conventions: those of ui.h, game.h, kit.h, rules.h and cal1.h (fXXXXX = the original at 0xXXXXX, Watcom register
   arguments eax, edx, ebx, ecx, then the stack; void unless a caller reads eax). D = read in the code, G = inference.

   Globals (D):
     the calamity list 0xF77E4 + 3k {uint8 card id, int8 giver, uint8 victim} (cal1.h); int32 [0xF40F8] the current
       nation; int32 [0xF40EC] the players in play
     nation records 0xF14CC + 199n (rules.h): +0x2F Drama & Poetry, +0x30 Music, +0x34 Democracy, +0x35 Military,
       +0x36 Philosophy, +0x51 int8 cities, +0x52.. their territories + 1, +0x98 int8 tokens on the board, +0x99 int8
       stock, +0x9A 0 out / 2 computer / 3 human, +0x9B uint8 set to 1 by a Civil War that ends with a choice of side
       (G: the meaning). The two factions are the hidden nations 12 FIRST FACTION (0xF1E20) and 13 SECOND FACTION
       (0xF1EE7): real nation records whose +0x51 / +0x98 / +0x99 are kept city by city and token by token
     area record r = t + 1: 0xE31B8 + 69r: +0x0E + n tokens of n (+0x1A faction 12, +0x1B faction 13), +0x3B + n the
       "lost territory" mark of the victim n (written 1 here); territory state t: 0xE2C18 + 3t (+0 owner, +2 bit 0x01 a
       city)
     uint8 0xF7984[157] / 0xF7A24[157]: the first / second faction's areas next to the other faction (0x2AD50; the
       choice-list kinds 7 / 8 of cal1.c's 0x28CFC read them)
     the land / coast lists [0xF8E7C + 4t] / [0xF90FC + 4t] (port handles: int16 count, then int16 territories)
     the choice list int32 [0xF7C9C] / 0xF7CA0[157]; the snapshots [0xEED74] (areas 0..160), [0xEED7C] (territory
       states), [0xEED78] (nations 1..13) of 0x2B6EC (port handles, freed and zeroed at FINISHED)
     uint8 0xF547C[14] / 0xF548C[14] the counts of the losses box (conflict.h), set by 0x2D3E0 for nations 12 / 13
     dialogs: 0xF596C the panel; 0xBFE28 the chooser's (arrows 0x39 / 0x3A -> -9 / -10, ONE 0x3F -> -0x65, ALL 0x40
       -> -0x66, RESTART 0x41 -> -0x63, FINISHED 0x2A -> -1; labels, x, enabled and style rewritten in the data
       object); 0xC0214 the side choice (PLAY FIRST FACTION 0xB2, PLAY SECOND FACTION 0xB3, 12 / 13 through 0x5CDE8);
       [0xF8E78] the current dialog
     int16 [0xBB400] the message area's height (0x15 from 0x2B6EC on); byte [0xF0980] bit 0x04 (the computers'
       messages wait for CONTINUE); byte [0xF097C] bit 0x08 (restart confirmations) */

void f29E34(int k);        /* D: Civil War for calamity record k. Victim v = byte 0xF77E6 + 3k, [0xF40F8] = v. The
                              beneficiary B = the nation in play (1..9) with the most units in stock ((9 - cities) * 5 +
                              stock; bubble sort, ties keep the lower nation). B == v: no Civil War. Factions: f1 = 15
                              (+5 Music, +5 Drama & Poetry, +10 Democracy) and f2 = 20, or 0 and 15 with Philosophy; f1 +
                              f2 >= v's units: quelled. Else v chooses f1 units, then B f2 units, both into nation 12
                              (human: f2B6EC mode 4; computer: 0x2C190, M7); everything v has left goes to 13; Military:
                              f2AD50(v, B); the side v plays (f2D3E0 / 0x2C778), whose units return to v; the other
                              faction annexed by B whole, or by the nations in rank order (cities: f2B6EC mode 5 /
                              0x2D10C, tokens: mode 6 / 0x2D1C0), the rest destroyed; f1E69C(k). Text fixed unless
                              g_original: "beneficary" (x4), and the computer's annexing messages name the "Second
                              Faction" also when the first is annexed. One caller (0x1E2B3) */
void f2AD50(int v, int b); /* D: Military losses: the faction areas next to the other faction marked in 0xF7984 (12's) and
                              0xF7A24 (13's); each faction loses 5 unit points: with at most 5 next to the other all of
                              them go automatically (1.5 s per area through f2EE6C), the rest r through NationLosesUnits
                              0x27FCC (CAL2) for a human (v for the first faction, b for the second) or 0x2C894 (M7), in
                              kind 9 (anywhere) or 7 / 8 (the marked areas), down to f2AC94(faction, r); the second
                              faction's only while it has tokens or cities. One caller (0x2A331) */
void f2B6EC(int x, int count, const char *text, int src, int target, int mode);
                           /* D: ChooseFirstFaction: a human moves count unit points from src to target (stack arguments
                              target, mode; ret 8): mode 4 SELECT ONE / SELECT ALL (a city counts 5), 5 ANNEX (cities,
                              1 each; the ALL button invisible), 6 ANNEX ONE / ANNEX ALL (tokens). x is the nation of the
                              panel and of the marks +0x3B + x (modes 5, 6). Snapshots for RESTART; FINISHED only when
                              nothing is left. BUG(1995) fixed unless g_original: ALL left the target's stock unpaid (see
                              cal4.c). Callers: 0x2A105, 0x2A1D9 (mode 4), 0x2A8AA (5), 0x2AA5D (6) */
int  f2D3E0(int n);        /* D: the human victim n picks the faction to play: the counts of 12 / 13 in the losses box
                              0x5CDE8 with dialog 0xC0214, "%s, Choose side to play."; returns 0x5CDE8's 12 or 13. One
                              caller (0x2A3B7) reads eax */
void f2EE6C(int n, int t, int left);   /* D: one automatic step: territory t's name (t >= 0) and left in the chooser's
                              boxes, the panel of n, t highlighted and centred (else the home view of n <= 9, or the map
                              as it is), 1500 ms. Callers: 0x2AD50 (4 sites), 0x2C298 (M7, 3 sites) */

#endif
