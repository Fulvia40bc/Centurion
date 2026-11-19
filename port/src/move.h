#ifndef MOVE_H
#define MOVE_H

/* Module MOVE of docs/MAP_PLAN.md (move.c): the eight movement primitives (pure record edits) and the button
   dispatcher 0x3FB94's movement cases as dcase_<id>(code), for dispatch.c to call.

   Conventions: those of ui.h and game.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx, ebx,
   ecx; void unless a caller reads eax). D = read in the code, G = inference. Records (D, offsets as the code adds them;
   meanings G unless said):
     area record of territory t: 0xE31B8 + 69(t + 1) (record 0 unused)
       +0x01        uint8  number of nations with tokens there
       +0x01 + n    int8   n when nation n has tokens there, else 0 (n = 1..9)
       +0x0E + n    int8   nation n's tokens there
       +0x1B + n    uint8  nation n's ships there
       +0x24 + n    int8   nation n's tokens there that may still move (set to +0x0E + n when its movement starts,
                           D 0x55D8F)
       +0x2D + n    uint8  index 0..3 of nation n's selected ship there
     nation record n: 0xF14CC + 199n
       +0x40        uint8  ships
       +0x41 + k    uint8  ship k's movement points left (4, or 5 with Clothmaking, +0x25: D 0x55E90..0x55EBE)
       +0x45 + 2k   int16  ship k's territory + 1 (the low byte; the movement code reads both widths, see move.c)
       +0x4D + k    int8   tokens aboard ship k
       +0x60        int8   number of territories in the list below
       +0x61 + j    uint8  territory + 1 of each territory where nation n has tokens (no ships counted) */

#include <stdint.h>

/* The primitives */
void f57CE8(int t, int n, int flag, int k);   /* D: k > 0: tokens +0x0E+n of t -= k, and with flag == 1 also +0x24+n; then
                                                 f581C4(t, n). 28 callers, eax unused */
void f57E28(int t, int n, int k);             /* D: k > 0: if marker +1+n (int8) != n: count +1 += 1, marker = n; tokens
                                                 +0x0E+n += k; t + 1 appended to nation n's list (+0x60/+0x61) unless
                                                 there. 23 callers, eax unused */
void f580CC(int a, int b, int n);             /* D: nation n's selected ship of a (index area[a] +0x2D+n) moves to b:
                                                 ships +0x1B+n of a - 1, f581C4(a, n), its word +0x45+2k = (old &
                                                 0xFF00) | (b + 1), selection of b = that of a, ships of b + 1. 3 callers */
void f581C4(int t, int n);                    /* D: marker +1+n != 0 and tokens +0x0E+n == 0: count +1 - 1, marker = 0,
                                                 t + 1 taken out of nation n's list. 10 callers, eax unused */
void f576F8(int t);                           /* D: n = int32 [0xF40F8]; up to 5 times: stop if int16 word +0x45+2k of
                                                 the selected ship k = area[t] +0x2D+n, minus 1, equals t; else k =
                                                 (k + 1) % 4 stored. 11 callers, eax unused */
int  f576A8(int t, int n, int x);             /* D: x if f57A10(t, n) == 1, else 5 (f57B5C called, its result unused) */
int  f57A10(int t, int n);                    /* D: 1 if a ship k < +0x40 of nation n has word +0x45+2k == t + 1 and
                                                 +0x41+k > 0, else 0 */
int  f57B5C(int t, int n);                    /* D: 1 if a ship k < +0x40 of nation n has +0x4D+k != 0 and word
                                                 +0x45+2k == t + 1, else 0 */

/* The dispatcher cases (jump table 0x3F780, entry = id - 3; targets D). code is the dispatcher's local ([ebp-0x3C],
   -100 on entry); each returns what the original's case returns. Labels from the dialog 0xC4A3C / 0xC4E28 records. */
int  dcase_26(int code);   /* 0x407DC MOVE TOKENS: already pressed -> code; else pressed, [0x108804] = 1, [0xF06CC] = 0,
                              f572CC([0x109E10]), 0x27 up; -3 */
int  dcase_27(int code);   /* 0x40B38 MOVE SHIPS: already pressed -> code; else pressed, [0x108804] = 2, [0xF06CC] = 0,
                              0x26 up, 0xC4..0xC8 disabled and up; -2 */
int  dcase_28(int code);   /* 0x40C4A UNDO MOVE: the last land move ([0x108810] == 1) or ship move taken back; -4 */
int  dcase_29(int code);   /* 0x40E1D FIND UNMOVED: -0x79 */
int  dcase_2B(int code);   /* 0x40F80 EMBARK: one token of [0x109E10] onto the selected ship of f3E8FC([0x109E10]); code */
int  dcase_2C(int code);   /* 0x41223 DISEMBARK: one token off the selected ship of f3E8FC([0x109E10]); code */
int  dcase_2D(int code);   /* 0x4152E DISEMBARK ALL: every token off that ship; code */
int  dcase_2E(int code);   /* 0x417A5 NEXT SHIP: the selection of [0x109E10] advanced, ship box recoloured; -2 */
int  dcase_2F(int code);   /* 0x40E45 EMBRK (second coast): as 0x2B on the ship of f3E89C([0x109E10]); code */
int  dcase_30(int code);   /* 0x410AE DSMBRK (second coast): as 0x2C, see move.c; code */
int  dcase_31(int code);   /* 0x41380 DSMBRK-A (second coast): as 0x2D, see move.c; code */
int  dcase_32(int code);   /* 0x416C6 NXT SHP (second coast): the selection of f3E89C([0x109E10]) advanced; code */
int  dcase_C4(int code);   /* 0x40858 count 1: already pressed -> code; else pressed, the others up, int16 [0xF09E0] = 1; code */
int  dcase_C5(int code);   /* 0x408E9 count 2 (as C4, [0xF09E0] = 2) */
int  dcase_C6(int code);   /* 0x4097C count 3 */
int  dcase_C7(int code);   /* 0x40A10 count 4 */
int  dcase_C8(int code);   /* 0x40AA4 count 5 */

#endif
