#ifndef NEGO_H
#define NEGO_H
/* The negotiation with a foreign province: EXE 0x5F82 from 0x602E (0x6570 the screen, the dialogue loop 0x604B..0x64DD,
 * the Cleopatra scenes). See nego.c and docs/NEGO_LOGIC_NOTES.md / NEGO_SCREEN_NOTES.md / NEGO_DATA_NOTES.md. */
#include <stdint.h>

/* 5F82 from 0x603C (after its cursor push 1D55(0), which the caller does): 0x6570 nego_screen, then the dialogue.
   Returns the word [bp-0xC]: 0 = 6570 failed (MNEGO missing), Withdraw, or the battle lost; 0xFFFF = the battle won, an
   alliance (any province) or any Cleopatra ending. The caller then does 0x64E0: 53F8(z) when nonzero, 1D8A.
   b06e = the battle thunk 0xB06E(army, L, z) (legions.c's), used for "Go to War" with army 0xFF (the province's troops).
   Draws on g_scr / g_wrk and takes its input through screen.c (menus and clicks answer "click" / "key" script lines). */
uint16_t nego_run(uint8_t L, uint8_t z, uint16_t (*b06e)(int army, int L, int z));

#endif
