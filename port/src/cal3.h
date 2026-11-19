#ifndef CAL3_H
#define CAL3_H

/* Module CAL3 of docs/M6_PLAN.md (cal3.c), the human side: Volcano / Earthquake (calamity card 0x13). The handler
   0x1E7A0 (called by CAL1's f1E0A0 at 0x1E18C), the site chooser 0x1EAEC (also Flood's, CAL2's 0x2D4F0 at 0x2DB0C),
   the eruption 0x1F25C, the earthquake 0x1F524 with its human pair chooser 0x1FDDC and its result 0x203B4. The
   computers' parts (0x1EF28, 0x20068, 0x201B8: M7) and the animation 0x7A4C4 (ANIM) are not ported yet: their call
   sites stop the port through not_ported(address); the list is in cal3.c's file comment.

   cal1.c still carries a static not_ported stub named f1E7A0 (and CAL2's Flood will call f1EAEC): this header exports
   the real ones, so that stub is to be replaced by an #include "cal3.h" (Claude, when the modules are merged; this
   module does not touch cal1.c).

   Conventions: those of ui.h, game.h, kit.h, rules.h and cal1.h (fXXXXX = the original at 0xXXXXX, Watcom register
   arguments eax, edx, ebx, ecx, then the stack; void unless a caller reads eax). D = read in the code, G = inference.

   Globals (D):
     the volcano table int16 0xBDE18[3][2] (territories; -1 none): Mt. Vesuvius 52 CAMPANIA / 127 NEAPOLIS, Mt. Etna
       76 MILAZZO / 78 SYRACUSE, Mt. Thera 83 THERA / -1; their names char *0xBDE24[3]
     the Earthquake's candidate blocks 0xF6F68 + 100i (city i of the victim) of ten 10-byte entries {int16 own city
       territory, int16 neighbour, int16 score, int16 attitude (low word of int32 0xF52E0[n][owner], f7C960's score
       matrix), int16 priority (int8 0xEED84[n][neighbour], the .bpm table)}, counts int16 0xF72EC[9]; the tied pairs
       0xF7300 + 8k {int16 own city, int16 neighbour, int16 attitude, int16 priority} with the count int32 [0xF7C9C];
       int16 [0xF77E0] the pair chosen
     the choice list int32 0xF7CA0[] / [0xF7C9C] (volcano indices, the victim's city territories, or river indices for
       Flood); int32 [0xF783C] the calamity being resolved (0x12 for the Earthquake)
     uint8 0xF547C[14] / 0xF548C[14] the tokens / cities lost (conflict.h's losses box f5CDE8)
     dialogs 0xBE21C (arrows 0x39 / 0x3A -> -9 / -10, button 0x2A relabelled ERUPT / STRIKE / FLOOD HERE -> -1),
       0xBE608 (the same for the pair chooser, STRIKE HERE), 0xC5214 (FINISHED), all appended to 0xF596C */

#include <stdint.h>

void f1E7A0(int k);        /* D: Volcano / Earthquake for the calamity record k (victim n = byte 0xF77E6 + 3k): no
                              cities -> the message (CONTINUE); else the volcanoes whose territories hold a city of n:
                              none -> f1F524(n); one -> it; several -> scored (6 per territory with a city of an owner
                              other than 10, else the tokens of nations 1..9 there), a unique maximum wins, ties: a
                              human (not PBEM) chooses (f1EAEC(n, 0x13)), a computer 0x1EF28 (M7); then f1F25C(v, n).
                              Always f1E69C(k) at the end. One caller (CAL1's f1E0A0, 0x1E18C) */
int  f1EAEC(int n, int type);  /* D: the human n picks one of the candidates in 0xF7CA0 / [0xF7C9C]: type 0x13 a
                              volcano, 0x12 an Earthquake city, 0x19 a river (Flood). Arrows -9 / -10, map clicks
                              (index into the list), the HERE button -1 returns 0xF7CA0[idx]. BUG(1995) fixed unless
                              g_original: map clicks for 0x13 / 0x19 (see cal3.c). Callers read eax (0x1E7A0, 0x1F524,
                              CAL2's 0x2D4F0) */
void f1F25C(int v, int n); /* D: volcano v erupts on n's turn: in each of its territories a city is destroyed
                              (0xF548C[owner] + 1, f78F04) or, without a city, the tokens of nations 1..9 go back to
                              stock (0xF547C[k] += tokens, f581C4); then the casualty box f5CDE8(2000, "MT. ... ERUPTION
                              CASUALTIES") over the volcano's highlighted territories. One caller (0x1EAB9) */
void f1F524(int n);        /* D: Earthquake for victim n ([0xF783C] = 0x12): the neighbouring territories of each of
                              n's cities are scored (12 for a city of another nation without Engineering, else 0);
                              maximum 0: n's own city is hit, chosen by a human (f1EAEC(n, 0x12)) or 0x201B8 (M7);
                              a unique maximum: that pair; ties: the tied pairs to 0xF7300, chosen by a human
                              (f1FDDC) or ordered by 0x20068 (M7, then pair 0). Then f203B4. One caller (0x1EAD3) */
int  f1FDDC(int n);        /* D: the human n picks one of the tied Earthquake pairs 0xF7300 (count [0xF7C9C]);
                              returns its index. BUG(1995) fixed unless g_original: map clicks (see cal3.c). One caller
                              (0x1FCFC, stored as int16 [0xF77E0]) */
void f203B4(int t, int t2, int n);   /* D: the Earthquake hits n's city in t (reduced with n's Engineering, else
                              removed) and, if t2 != -1, the neighbouring city in t2 (the Pirates' removed, any other
                              reduced); the message through f79270(-500, n, -t, text, 2). Callers 0x1FB82, 0x1FDCB */

#endif
