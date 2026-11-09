#ifndef NAVAL_H
#define NAVAL_H
/* The naval battle, OVL0 section 5 (file 0x8550-0xB3F0, loaded at 0823:B0E0); disasm/OVL_NAVAL.asm,
 * docs/NAVAL_INDEX.md, NAVAL_LOGIC_NOTES.md, NAVAL_SCREEN_NOTES.md, NAVAL_DATA_NOTES.md, FLEET_NOTES.md section 7.
 *
 * Fleet records F = DS:110E + 12k: 0..2 Roman, 3..5 enemy, 6 = an army at sea (built by EXE 0x531F, fleet.c). */
#include <stdint.h>

/* section 5 B0E0 via thunk B09E (fleet move 0x2C24): Roman fleet f enters sea z and fights every enemy fleet 3..5 there
   (B12A(e, f)); returns 1 = the way is clear (every battle had result bit 1, or there was none), 0 = not clear */
uint16_t naval_enter_b0e0(uint8_t f, uint8_t z);
/* section 5 B12A via thunk B0C2 (0x4B22 in 4ADA, 0x5221 in End Turn): the battle of enemy record `enemy` (3..6) against
   Roman fleet f. Result: 0 cancelled (Esc at the flagship choice / graphics not loaded; in the port also: no screen to
   fight on, --end-turn), 1 Rome lost, 2 Rome won, 3 both fleets gone, 4 both still have ships (retreat / not re-engaged).
   Ported in naval.c (logic, timing) and naval_gfx.c (drawing); see naval.c's header for the pacing and the 1990 bugs. */
uint16_t naval_battle_b12a(uint8_t enemy, uint8_t f);

#endif
