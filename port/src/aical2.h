#ifndef AICAL2_H
#define AICAL2_H

/* Module AICAL2 of M7 batch 2 (aical2.c): the computers' side of the unit-loss calamities and of Civil War.
     CAL2 (cal2.c's computer calls): 0x260C4 the unit losses of Famine, Epidemic and Flood, 0x27518 the secondary
       losses assigned by a computer victim, 0x27850 the computer's Grain against Famine, 0x2E45C the computer's river
       among the tied flood plains;
     CAL4 (cal4.c's computer calls): 0x2C190 the faction choice screen with 0x2C298 (the units chosen for a faction,
       Iberia's AI rows rewritten), 0x2C894 a faction's Military losses, 0x2D10C / 0x2D1C0 the beneficiary's annexing
       of cities / tokens.
   The call sites stop the port today through static not_ported stubs of the same names in cal1.c (f27518), cal2.c
   (f27850, f260C4, f27518, f2E45C) and cal4.c (f2C190, f2C894, f2D10C, f2D1C0); this header exports the real ones, so
   those stubs are to be replaced by an #include "aical2.h" (Claude, when the modules are linked; this module does not
   touch those files).

   Conventions: those of ui.h, game.h, kit.h, rules.h, cal1.h, cal2.h and cal4.h (fXXXXX = the original at 0xXXXXX,
   Watcom register arguments eax, edx, ebx, ecx, then the stack; void unless a caller reads eax). D = read in the
   code, G = inference. aical2.c's file comment has the addresses, the data, the 1995 bugs and the eggs.

   Globals written (D): the board (area records 0xE31B8 + 69r: +0x0E + n tokens, +0x3B + v the "lost territory"
   marks), territory states 0xE2C18 + 3t, nation records +0x51.. (cities), +0x60.. (token areas), +0x98 / +0x99; the
   choice list int32 [0xF7C9C] / 0xF7CA0[157] (entries taken out); uint8 0xF547C[14] (0x27518 and 0x27850); int16
   0xF409A[n] (0x27850); the border highlights 0xFFB60; int16 [0xBB400] = 0x15 (0x2C190). Not in the save (M6_PLAN
   risk 9): int32 [0xBEDE8] (0x2E45C: Iberia's own river, Rhone or Po by the player count) and Iberia's AI rows
   0xC0638..0xC0647 / 0xC06CC..0xC06DF (0x2C298: by the player count). */

/* ---- CAL2: Famine, Epidemic, Flood -------------------------------------------------------------------------- */

void f260C4(int n, int L, int kind, int s);   /* D: computer n loses L unit points to calamity kind (0x15 Famine, 0x1B
                                                 Epidemic, 0x19 Flood with s = the flood plain, else -1): the choice
                                                 list f28CFC(n, kind, s); "%s ravages %s." (1500 ms); Famine /
                                                 Epidemic: R cities eliminated (each f213C0 (AICAL1), f78FEC, its tokens
                                                 off while L > 0; Epidemic keeps 1), Flood: n's flood-plain cities
                                                 eliminated (f78F04, 5 each) then every token on the plain; then token
                                                 plans (three passes or by the AI table 0xEED84 when dword 0xF4204[n]
                                                 != 0; Epidemic over n's own area list), merged per territory and
                                                 applied with a message each. BUG(1995) fixed unless g_original: the
                                                 token passes could plan more than a territory holds and hang with none
                                                 left (aical2.c). Callers: cal2.c's 0x25A38, 0x2D4F0, 0x2E59C */
void f27518(int n, int trader, int total, int kind);   /* D: computer victim n assigns `total` secondary losses:
                                                 nations ranked by int32 0xF52E0[10n + k] + 100 * int8 0x109E23[9(n-1) +
                                                 k] (out: -99999; bubble sort, descending), those with int16 0xF796C[k] >
                                                 0 take 0xF547C[k] (cleared first): the first all of its count, the
                                                 second and third at most a half share, the rest what is left; then
                                                 "Assignments for %s secondary losses" (0xC76E0[kind]) through f291D0(n,
                                                 trader, text). Callers: cal1.c's 0x23ACC, cal2.c's 0x25A38, 0x2D4F0,
                                                 0x2E59C */
void f27850(int n);                           /* D: computer n's Grain against Famine: the GRAIN / USED / LOSSES /
                                                 REDUCED TO boxes, int16 0xF409A[n] = cards used, uint8 0xF547C[n]
                                                 reduced by 4 a card (1 s, then 2.5 s). The hand test reads the advance
                                                 bytes +0x25..+0x36 and +0x2C, 0x24 past the cards (kept: an AI choice,
                                                 aical2.c). Caller: cal2.c's 0x25A38 (twice) */
int  f2E45C(int n);                           /* D: computer n's plain among the tied ones in 0xF7CA0: int32 [0xBEDE8] =
                                                 ([0xF40F0] == 3); the first listed plain that is not n's own river
                                                 (int32 0xBEDE0[n]), shown "%s chooses the %s to flood." with the
                                                 plain's territories highlighted (f79270(-500, n, -200)); returns it
                                                 (list[count] when every one is n's own, G unreachable). Text fixed
                                                 unless g_original: "choses", "TIGRUS". Caller: cal2.c's 0x2DB77 */

/* ---- CAL4: Civil War ----------------------------------------------------------------------------------------- */

void f2C190(int x, int count, const char *text, int src, int target, int mode, int flag);
                                              /* D: the computer's counterpart of 0x2B6EC (stack target, mode, flag;
                                                 ret 0xC): int16 [0xBB400] = 0x15, background 3, "LEFT TO" and
                                                 "SELECT" (mode 4) / "ANNEX" (5, 6), f3AB7C(text), f7876C, f28CFC(src,
                                                 mode, 0), then f2C298(x, count, src, target, (int8)flag). Callers:
                                                 cal4.c's 0x2A141 (flag 0), 0x2A247 (flag 1) */
void f2C298(int x, int count, int src, int target, int flag);
                                              /* D: count unit points of src to target (stack flag, ret 4): Iberia's
                                                 rows of 0xC0618 / 0xC06B8 rewritten by [0xF40F0]; keep = (count >= 20,
                                                 flag 0); cities first (5 each, at most 4; by row x of 0xC0618 with
                                                 keep, a second pass from index 1 if short), then tokens in two passes
                                                 (row x - 1 of 0xC06B8, then any), each step shown by cal4.c's f2EE6C.
                                                 One caller (0x2C28B) */
void f2C894(int n, int count, char *text, int ov, int flag, int mode);
                                              /* D: computer takes faction n's Military losses down to ov (stack flag,
                                                 mode; ret 8): "LEFT TO" / "REMOVE", f3AB7C(text), f28CFC(n, mode, 0);
                                                 two passes over the list (the first by rows n of 0xC0618 and n - 1 of
                                                 0xC06B8: for n 12 / 13 these read past the 10-nation tables, kept
                                                 byte-exact), each step shown 500 / 1500 ms; itoa's into text. BUG(1995)
                                                 fixed unless g_original: the faction's +0x98 / +0x99 were not updated,
                                                 and an empty list showed entry -1 (aical2.c). Callers: cal4.c's 0x2B428
                                                 (n 12), 0x2B6DC (n 13) */
void f2D10C(int v, int k, int o, int b);      /* D: computer b annexes k cities of faction o: each 0x20C70(b, o)
                                                 (AICAL1) - 1, mark +0x3B + v, f78F04 / f78E74, "%s annexes the city in
                                                 %s" (1500 ms). Caller: cal4.c's 0x2A90F */
void f2D1C0(int v, int k, int o, int b);      /* D: computer b annexes k tokens of faction o over o's area list (a
                                                 pass by row v - 1 of 0xC06B8, then the rest, repeated while k != 0);
                                                 "%s annexes tokens in %s" (1500 ms) per area. BUG(1995) fixed unless
                                                 g_original: a pass that moves nothing ends the loop (the original hangs
                                                 when o's count was stale). Caller: cal4.c's 0x2AAB9 */

#endif
