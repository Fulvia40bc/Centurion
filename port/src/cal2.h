#ifndef CAL2_H
#define CAL2_H

/* Module CAL2 of docs/M6_PLAN.md (cal2.c), the human side: the unit-loss calamities Famine 0x25A38, Flood 0x2D4F0
   and Epidemic 0x2E59C (handlers of the calamity phase 0x1E0A0), the human's Grain screen 0x27CEC, NationLosesUnits
   0x27FCC (the human removes units; also CAL4's Civil War 0x2AD50), and the secondary-loss screens 0x291D0 (shown),
   0x29550 (the human victim assigns them) with its redraw 0x29C94. 0x2EC38 is helpers2.h's. The computers' parts
   (0x27850, 0x260C4, 0x27518, 0x2E45C, and CAL1's 0x2E1D0: M7), the animation hook 0x7A4C4 (ANIM) and CAL3's river
   chooser 0x1EAEC are not ported yet: their call sites stop the port through not_ported(address) (cal2.c's file
   comment has the list with each one's signature).

   cal1.c still carries static not_ported stubs named f25A38 / f2D4F0 / f2E59C (its handlers) and f291D0 / f29550
   (Iconoclasm's secondary losses): this header exports the real ones, so those stubs are to be replaced by an
   #include "cal2.h" (Claude, when the modules are merged; this module does not touch cal1.c).

   Conventions: those of ui.h, game.h, kit.h, rules.h and cal1.h (fXXXXX = the original at 0xXXXXX, Watcom register
   arguments eax, edx, ebx, ecx, then the stack; void unless a caller reads eax). D = read in the code, G = inference.

   Globals (D):
     the calamity list 0xF77E4 + 3k {uint8 id, int8 giver (-1 drawn), uint8 victim} (cal1.h)
     int16 0xF796C[11] units of each nation subject to secondary losses (index 10 written by Flood only, 0xF7980)
     uint8 0xF547C[14] the losses assigned to each nation (read as dword [0xF5479 + n] sar 24); 0xF548C[14] cleared by
       Flood (the losses box's city counts, conflict.h)
     int16 0xF409A[10] Grain cards spent against Famine (cleared by 0x1E0A0, read by ADV's 0x1818C)
     the flood plains 0xBEE08 + 22s, s 0..4 {int16 units of the victim (written here), int16 count, char *river,
       int16 territories[7]}: RHONE (138, 143), PO (18, 19), DANUBE (9, 126, 16), TIGRUS-EUPHRATES (48, 47, 46, 68,
       71, 69, 70), NILE (111, 115, 112, 113, 114)
     the choice list int32 [0xF7C9C] / 0xF7CA0[157] (cal1.h's f28CFC fills it for 0x27FCC)
     the message area int16 0xBB3FA / 0xBB3FC / 0xBB3FE / 0xBB400; int32 [0xF40F8] the current nation
     snapshots of 0x27FCC (port heap handles): [0xEED74] areas 0..160, [0xEED7C] territory states, [0xEED78] nations
       1..13 */

#include <stdint.h>

/* ---- the calamities (k = the record index of 0xF77E4; one caller each, 0x1E0A0's jump table) ------------------- */

void f25A38(int k);   /* D: Famine (card 0x15): loss = min(5 * cities + tokens, 10) into 0xF547C[v]; Pottery (+0x24) and
                         Grain cards (+0x08): the human's f27CEC (computer 0x27850); loss > 0: f27FCC (computer 0x260C4),
                         else "Your usage of Grain has prevented Famine in %s."; secondary: 0xF796C[m] = min(5 * cities +
                         tokens, 8) for the others; total <= 20: all of it (f291D0), else the human assigns 20 (f29550;
                         computer 0x27518); then each m with losses, Grain first (f27CEC / 0x27850 + 2000 ms), f27FCC or
                         0x260C4. f1E69C(k); message height 0x23 */
void f2D4F0(int k);   /* D: Flood (card 0x19): the victim's units on each of the 5 flood plains (a flood-plain city
                         counts 5); none: a coastal city reduced (Engineering) or eliminated (human f207B4, computer
                         0x2E1D0), or nothing; else the plain with the most (last of equals; ties: human 0x1EAEC (CAL3),
                         computer 0x2E45C), losses min(units, 17, 7 with Engineering) through f27FCC (computer 0x260C4);
                         secondary units on the plain min(units, 10, 7 with Engineering): total <= 10 lost outright,
                         else 10 assigned (f29550; computer 0x27518); the Pirates lose theirs (0x260C4). BUG(1995) fixed
                         unless g_original: the assigned losses chose the screen by the victim's human flag (cal2.c) */
void f2E59C(int k);   /* D: Epidemic (card 0x1B): losses 16, -8 Medicine (+0x2D), +5 Roadbuilding (+0x28), at most
                         tokens - token areas + 4 * cities; human f27FCC (target f2EC38 when only cities can go),
                         computer 0x260C4; secondary min(that, 10) for the others but the trader; total <= 25 all of it,
                         else 25 assigned (f29550 / 0x27518); each m: Medicine -5 (floor 0), Roadbuilding +5, capped;
                         > 0 f27FCC / 0x260C4, reduced to 0 "%s saved from Epidemic by MEDICINE." */

/* ---- the screens ----------------------------------------------------------------------------------------------- */

void f27CEC(int n);   /* D: the human chooses how many Grain cards to spend against Famine: dialog 0xBF264 (arrows 0x39 /
                         0x3A -> -9 / -10, FINISHED -1); int16 0xF409A[n] = used (from 0), each card -4 losses on
                         0xF547C[n] (bytes, may go below 0). One caller (0x25A38, twice) */

/* 0x27FCC NationLosesUnits (its own trace name): human n removes L unit points listed by f28CFC(n, kind, sub) (kind
   0x15 Famine, 0x19 Flood with sub = the plain, 0x1B Epidemic: a city is reduced and 1 token stays; 7 / 8 / 9 Civil
   War, without REDUCE CITY), until exactly `target` are left (FINISHED). Dialog 0xBF650: REMOVE ONE 0x3F -> -0x65,
   REMOVE ALL 0x40 -> -0x66, REDUCE CITY 0x3E -> -0xE, RESTART 0x41 -> -0x63, FINISHED 0x2A -> -1, arrows -9 / -10, map
   clicks on listed territories. Snapshots in [0xEED74] / [0xEED7C] / [0xEED78] for RESTART. Stack arguments sub,
   target (ret 8). Callers: 0x25A38, 0x2D4F0, 0x2E59C, CAL4's 0x2AD50. BUG(1995) fixes unless g_original: cal2.c */
void f27FCC(int n, int L, const char *text, int kind, int sub, int target);

/* 0x291D0: the secondary losses 0xF547C of the nations m 1..9 in play (m != a, m != b, 0xF796C[m] != 0) shown in
   columns (name, token, number) with CONTINUE (0xBB800 at 0x109, 0x1AE); unattended: one wait of 250 / 1000 ms.
   Returns at once when no nation qualifies. Callers: 0x25A38, 0x2E59C, CAL1's 0x23ACC, 0x27518 (M7) */
void f291D0(int a, int b, const char *text);

/* 0x29550: human victim a assigns `total` secondary losses (at most the sum of 0xF796C) over the same columns with
   +/- buttons (ids 0xCC + j / 0xD3 + j appended to the panel 0xF596C, codes -0x67 - j / -0x6E - j), own click loop
   with 150 ms auto-repeat; CONTINUE (0xBFA3C) only when nothing is left. 0xF547C cleared first. Callers: 0x25A38,
   0x2D4F0, 0x2E59C, CAL1's 0x23ACC */
void f29550(int a, int b, int total, const char *text);

/* 0x29C94: 0x29550's numbers: LEFT TO box and each column's box (list = the caller's column nations, w the column
   width, x0 the first column); CONTINUE (0x2A of 0xF596C) disabled while left > 0, then pasted, only for a human a
   without PBEM. Stack arguments left, x0 (ret 8). One caller (0x29550, three call sites) */
void f29C94(int a, int b, const int32_t *list, int w, int left, int x0);

const char *cal2_river_text(const char *r);   /* port: a flood plain's river name with the "TIGRUS" text fix unless
                                                g_original (Flood's messages and CAL3's 0x1EAEC) */

#endif
