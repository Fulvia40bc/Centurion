#ifndef STACK_H
#define STACK_H

/* Module STACK of docs/MAP_PLAN.md (stack.c): the World State Stack (0x7A8D0 allocate, 0x7A940 push, 0x7ABD0 free,
   0x7AC60 pop), the release of the game sprites 0x121A0, and the bodies of the dispatcher 0x3FB94's cases SAVE GAME
   (0x4F), RESTORE GAME (0x50), START NEW GAME (0x51) and REPLAY (0x101) as dcase_<id>(code), for dispatch.c to call
   (code = the dispatcher's local [ebp-0x3C], -100 on entry; the return value is what the case returns).

   Conventions: those of ui.h and game.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx, ebx,
   ecx; dialogs and sprite records as uint32_t link addresses; void unless a caller reads eax). Grades: D = read in
   the code, G = inference. The data layouts are in stack.c's file comment. */

#include <stdint.h>

/* The two longjmps of the dispatcher (port, implemented in main.c with C setjmp/longjmp): jmpbuf is the link address
   of the original's jmp_buf, 0xF58D4 (main's setjmp at 0x11036, the title menu: case 0x51) or 0xF58A0 (main's setjmp
   at 0x115CE, the game screen after a restore: case 0x50). The original passes val 1 (edx, D 0x3FE8C / 0x3FF73). */
_Noreturn void port_longjmp(uint32_t jmpbuf);

void f121A0(void);                     /* D: free the game sprites that 0x11A8C cut (f3AF54 on each record): c_invld 0xF5214, 0xF521C, 0xF5224; for i 1..13: 0xF489C, 0xF497C, 0xF4E3C, 0xF4DCC + 8i (0xF490C + 8i for i <= 9); for i 1..13: 0xF4ACC, 0xF4BAC, 0xF4B3C, 0xF49EC + 8i (0xF4A5C + 8i for i <= 9); 0xF4D2C..0xF4D54; the minimap 0xF520C; 0xF518C, 0xF5194, 0xF517C, 0xF5184; for i 0..29: 0xF4EAC, 0xF4F9C, 0xF508C + 8i; cardtiny 0xF4C1C + 8i, i 1..33. BUG(1995) fixed unless g_original: 0xF4D5C + 8i (i 1..13), 0xF519C and 0xF51A4 are freed too. One caller (dispatcher id 0x51) */
void f7A8D0(void);                     /* D: allocate the World State Stack once (if [0xF5240] == 0): for i 0..9, [0xF5240 + 4i] = malloc(0x2B20) (area records 1..160), [0xF5268 + 4i] = malloc(0x88D) (nation records 1..11), [0xF5290 + 4i] = malloc(0x1E0) (the 160 territory-state records); port handles (load_heap_alloc) */
void f7A940(void);                     /* D: push: if depth [0xCAC54] >= 9, sprintf(0xF5500, "World State Stack has blown its max size. > %d", 10) and quit (f44CAC); else depth + 1 (high-water mark [0x112A8C]), and the slot gets areas 0xE31FD (0x2B20), territory state 0xE2C18 (0x1E0), nations 0xF1593 (0x88D) */
void f7ABD0(void);                     /* D: if [0xF5240] != 0: free the 30 blocks, zero the 30 fields, depth [0xCAC54] = -1 */
void f7AC60(void);                     /* D: pop: if depth [0xCAC54] < 0, "World State Stack has popped once to often." into 0xF5500 and quit (f44CAC; the text slip fixed unless g_original); else the slot back into areas 0xE31FD, territory state 0xE2C18, nations 0xF1593, depth - 1 */

/* The dispatcher cases (jump table 0x3F780, entry id - 3; targets D, each entry checked against its fixup). */
int  dcase_4F(int code);               /* -> 0x3FDAB SAVE GAME: f3CC68, f3CCBC, f5E7D0; returns -1 (ecx = -1 kept across the call) */
int  dcase_50(int code);               /* -> 0x3FEA0 RESTORE GAME: f3CC68, f3CCBC, f5FF28; -15 (cancel, or a rejected file) returns -1 (0x3FD35); else byte [0xE1624] &= ~4, f7ABD0, free + zero 0xF0984[1..11], [0xEED74], [0xEED7C], [0xEED78], [0xEED80] (each if non-0), f3C7A0([0xF8E78]), port_longjmp(0xF58A0): does not return */
int  dcase_51(int code);               /* -> 0x3FDCF START NEW GAME: f3CC68, f121A0, f7ABD0, f3F558, the same frees as 0x50, f3C7A0([0xF8E78]), port_longjmp(0xF58D4): never returns */
int  dcase_101(int code);              /* -> 0x4041D REPLAY: [0xEDF58] == 0 -> code (-100); int16 [0x107692] != 0 (a replay is showing) -> -1; else the post-movement state of round [0xEDF5C] on the map until REPLAY is pressed again, then the state put back; returns -100 */

#endif
