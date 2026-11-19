#ifndef MSTACK_H
#define MSTACK_H

/* Module STK of docs/M6_PLAN.md (mstack.c): the rest of the World State Stack (peek 0x7AF28, drop 0x7AD24; stack.c
   has allocate 0x7A8D0, push 0x7A940, free 0x7ABD0, pop 0x7AC60), the Move List Stack (push 0x7AA1C, peek 0x7AFB8,
   pop 0x7AE14, drop 0x7AD7C) and three fixed snapshots (world state 0x7A688 / 0x7A700, nations 0x7A778 / 0x7A7B0, move
   lists 0x7A7E8 / 0x7A85C). Callers are the computer player's searches (AIQ, AISEA, AIMOVE), the movement phase and
   ship construction.

   Conventions: those of ui.h and game.h (fXXXXX = the original at 0xXXXXX; no arguments, and no caller reads eax, G
   from the inventory's call sites). Grades: D = read in the code, G = inference. The data layouts are in mstack.c's
   file comment; the World State Stack's in stack.c's. */

void f7A688(void);   /* D: world snapshot: areas 0xE31FD (0x2B20) -> 0x117CC0, territory state 0xE2C18 (0x1E0) -> 0x117AE0, nations 0xF1593 (0x88D) -> 0x116518 */
void f7A700(void);   /* D: back from the world snapshot: 0x117CC0 -> 0xE31FD (0x2B20), 0x117AE0 -> 0xE2C18 (0x1E0), 0x116518 -> 0xF1593 (0x88D) */
void f7A778(void);   /* D: nations snapshot: 0xF1593 (0x88D) -> 0x116FFC */
void f7A7B0(void);   /* D: back from the nations snapshot: 0x116FFC (0x88D) -> 0xF1593 */
void f7A7E8(void);   /* D: move-list snapshot: [0x11AB48] = [0x1079B4], 4 * it bytes 0x107694 -> 0x11A828; [0x11AED0] = [0x107D3C], 6 * it bytes 0x1079B8 -> 0x11AB4C (counts unchecked) */
void f7A85C(void);   /* D: back from the move-list snapshot: [0x1079B4] = [0x11AB48], 4 * it bytes 0x11A828 -> 0x107694; [0x107D3C] = [0x11AED0], 6 * it bytes 0x11AB4C -> 0x1079B8 */
void f7AA1C(void);   /* D: Move List Stack push: if depth [0xCAC58] >= 6, sprintf(0xF5500, "Move List Stack has blown its max size. > %d", 7) and quit (f44CAC); else depth + 1 (high-water mark [0x112A90]); slot d: count 1 [0x1079B4] -> 0x112A54[d] and, if > 0, a new block of 4 * count (handle 0x112A1C[d]) with list 1 0x107694; count 2 [0x107D3C] -> 0x112A70[d] and, if > 0, a new block of 6 * count (handle 0x112A38[d]) with list 2 0x1079B8 */
void f7AD24(void);   /* D: World State Stack drop: if depth [0xCAC54] < 0, "World State Stack has popped once to often." into 0xF5500 and quit (f44CAC; the text slip fixed unless g_original); else depth - 1 (the slot's blocks stay allocated, nothing is copied) */
void f7AD7C(void);   /* D: Move List Stack drop: if depth [0xCAC58] < 0, "Move Stack has popped once to often." and quit (text fixed unless g_original); else slot d's two blocks freed, its handles and counts zeroed, depth - 1 */
void f7AE14(void);   /* D: Move List Stack pop: if depth [0xCAC58] < 0, "Move Stack has popped once to often." and quit (text fixed unless g_original); else slot d's counts back into [0x1079B4] / [0x107D3C] and, for a count > 0, its list back into 0x107694 / 0x1079B8 and its block freed (handle zeroed; the counts stay in the slot), depth - 1 */
void f7AF28(void);   /* D: World State Stack peek: slot [0xCAC54] back into areas 0xE31FD, territory state 0xE2C18, nations 0xF1593, depth unchanged (no check for an empty stack) */
void f7AFB8(void);   /* D: Move List Stack peek: slot [0xCAC58]'s counts back into [0x1079B4] / [0x107D3C] and, for a count > 0, its list into 0x107694 / 0x1079B8; depth and blocks unchanged (no check for an empty stack) */

#endif
