#ifndef LOOP_H
#define LOOP_H

/* Module LOOP of docs/M6_PLAN.md (loop.c): the game loop 0x54824 with its 21 phase cases, the restored game's set-up
   0x14AA4, the empty hook 0x5E7B4, the end-of-round save 0x55258 (savgameb.dat), the end-of-game mark 0x55304
   (savgamea.dat), the phase music 0x553C0, the trade-session check 0x55500, the elimination check 0x22B00 with its
   two end screens 0x22DCC (last human out) and 0x22F3C (one player left), and the AI's score matrix 0x7C960. The
   save writer 0x5E9B0 and the autosave 0x5E8EC are savewrite.h's (VERIFIED). Plus the two entries main.c calls at
   0x1188C.

   Conventions: those of ui.h, game.h and kit.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax,
   edx, ebx, ecx, then the stack; void unless a caller reads eax). D = read in the code, G = inference. The phase
   routines that are not ported yet stop the port through not_ported(their address); loop.c's file comment lists
   them in phase order (the work order of the next modules). */

#include <stdint.h>

/* ---- main 0x1188C..0x118C9 (port entries) ------------------------------------------------------------------------

   main 0x1188C: if bits 0x04 (new game) and 0x80 (PBEM) of byte [0xE1624] are both clear, 0x14AA4; then, if both
   0x04 and 0x80 are set (a new PBEM game), 0x14A00, 0x4FE30, int16 [0xF408A] = -0x5C and back to the title menu
   0x11168 (PBEM, dropped by Erik's ruling: not_ported(0x14A00)); else 0x118C9: call 0x54824.
   After 0x54824 (D): main 0x118CE..0x118F4 is f39F50(-1), fg_mousefin, fg_setmode([0xF6058]) and main's return,
   but no path reaches it: 0x54824 leaves its loop only into 0x5522C, which ends in f44CAC (quit to DOS, exit(0) at
   0x7D578); the other ways out are longjmps (0x22DCC / 0x22F3C and the dispatcher's START NEW GAME to 0xF58D4, its
   RESTORE GAME to 0xF58A0) or f44CAC (QUIT, the stack overflow messages). So the two entries never return. */
_Noreturn void game_loop_restored(void);   /* main 0x1188C..0x118C9 for a restored game (bit 0x04 clear): f14AA4, then f54824 */
_Noreturn void game_loop_new(void);        /* main 0x1188C..0x118C9 for a new game (bit 0x04 set, so no 0x14AA4): f54824 */

/* ---- The loop and its helpers ------------------------------------------------------------------------------------ */

_Noreturn void f54824(void);   /* D: the game loop (callers: main 0x118C9). While done == 0: in phases 15 / 16 with digital sound the slot-0 sample stopped and its WAV freed; the round label (sprintf(0x109E7C, "%d", int16 [0xF40FC]) on button 0x101 of 0xF596C); f7C960; switch (byte [0xF4102]) over the jump table 0x547D0 (21 cases, listed in loop.c); f22B00. After the loop (done = f136B0(3) in phase 20): 0x14294 (Final Civilization Standings), f55304, remove("savgameb.dat"), [0xF5500] = 0, f44CAC */
void f14AA4(void);             /* D: restored game: for n 1..9 in play (+0x9A != 0) with the override flag int32 [0xF60E8 + 4n] set (never: main clears them), f15FE0(n, 0xF6064 + 13n); then memcpy(0xEF3C4, 0xEED84, 0x640). One caller (main 0x1189C) */
void f5E7B4(void);             /* D: empty (__CHK, push/pop ebp, ret). 21 call sites in 0x54824, 1 in 0x1E0A0 (CAL1) */
void f55258(void);             /* D: savgameb.dat rewritten (fopen "wb"): f5E9B0("AutoSaved at end of Round <int16 [0xF40FC]>", fp) from a 51-byte zeroed buffer (copy of 0xC65C0), fclose, then f7BBF0("savgameb.dat") appended as 4 bytes ("ab"). One caller (0x551EA, phase 20) */
void f55304(void);             /* D: savgamea.dat marked "AutoSaved at End of Game": fopen "r+b", f5F05C(&version, desc, fp) (the last autosave reloaded into the game globals), the description, rewind, f5E9B0, fclose, then f7BBF0("savgamea.dat") appended. "r+b" does not truncate: the old CRC stays after the body and the new CRC (0) follows, the file grows by 4 bytes (D). One caller (0x55231) */
void f553C0(void);             /* D: phase music and sample: AIL_end_sequence if the music now loaded (0xF61D8) is 0xCAB1C's (stricmp); music on (bit 0x02 of [0xF097C]): f7B854(name, seq) of the 9-byte record 0xCAB78 + 9 * int8 [0xF4102]; effects on (bit 0x01): a WAV in the record (+4) is loaded into slot 2 (f7B504) and, with digital sound [0xF625C], started looping at the effects volume, else slot 2's sample is ended. 22 call sites (19 in 0x54824; 0x1E0D3, 0x21A11, 0x6BE68) */
int  f55500(void);             /* D: 1 if at least two nations n 1..9 in play hold f76F14(n) >= byte [0xE1623] >> 4 (the minimum goods) cards, else 0. One caller (0x54CC2, phase 12) */
void f22B00(void);             /* D: elimination check: each n 1..9 in play with no tokens on the board (int8 +0x98 == 0) and no cities (int8 +0x51 == 0) is eliminated (message through f79270(-500, n, -200, ...); the last human goes to f22DCC; one player left -> f22F3C), its cards discarded, its givers in the others' calamity lists set to 0xFF; then 0x7784C([0xF40EC]). 23 call sites (22 in 0x54824, one in 0x1E0A0 at 0x1E191) */
_Noreturn void f22DCC(int n);  /* D: "<NAME>, none of your cities or units / remain on the board. / You have been eliminated from the game." on background 21, CONTINUE (0xBB800 id 0x2A moved to (0x109, 0x190)), wait for -1; then longjmp(0xF58D4, 1) to the title menu (PBEM: quit). One caller (0x22B9C) */
_Noreturn void f22F3C(void);   /* D: "<NAME>, you have conquered the known world!" for the first nation in play, background 21, CONTINUE as f22DCC; then 0x14294 (Final Civilization Standings), f788AC, longjmp(0xF58D4, 1) (PBEM: quit). One caller (0x22C38) */
void f7C960(void);             /* D: the AI's score matrix: memset(0xF52E0, 0, 400); for i 1..9 in play: s_i = f147C4(i), leader int32 [0xF52D4] = the first i with the highest score (start -9999, strict >), int32 [0xF52E0 + 40i + 4j] = f147C4(j) - s_i for each j 1..9 in play. Also reads a dead maximum of int8 0xBCCE0[85(i-1) + 5 * AST(i)]. Callers: 0x548FF (every turn of the loop), 0x1E241 (0x1E0A0) */

#endif
