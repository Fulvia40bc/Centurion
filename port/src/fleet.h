#ifndef FLEET_H
#define FLEET_H
/* Fleet orders (EXE segment 0823), see fleet.c and docs/FLEET_NOTES.md. Fleet record k at DS:110E + 12k (0..2 Roman,
 * 3..5 enemy, 6 = an army at sea): +0/+2/+4 ship counts by type, +6 zone, +7 home province, +8 status (0 Ready, 1 Moved,
 * 2 Forming, FFh none), +9 legion aboard (FFh none; enemy / record 6: the army record of the marines), +0x0A men per ship. */
#include <stdint.h>

uint8_t  fleet_select_2c6f(uint8_t z);           /* 0x2C6F: the Roman fleet to move from sea z, 0xFF = none / Esc */
void     fleet_info_2f5a(uint8_t f);             /* 0x2F5A: one-row box of fleet f at the top of the screen */
void     fleet_move_2b40(uint8_t f);             /* 0x2B40: click a neighbouring sea; fights and moves; map_build at the end */
uint16_t fleet_fight_armies_4ada(uint8_t f, uint8_t z);   /* 0x4ADA: every army at sea in z fights fleet f; 0 = stopped */
void     fleet_army_to_record6_531f(uint16_t i); /* 0x531F: army i becomes fleet record 6 (one rng_roll(3)) */
void     fleet_retreat_5244(uint8_t k);          /* 0x5244: Roman fleet k retreats to a neighbouring sea or is destroyed */

/* from the other files */
void     army_meets_4bac(uint16_t name, unsigned numeral, unsigned zone, unsigned army);   /* enemy.c: 0x4BAC */
uint8_t  fleets_in2d60(uint8_t z);                                /* province.c: 0x2D60 (A69C, A69E[], A6A1) */
void     province_fleet_row(uint8_t *fb, int y, uint8_t f);       /* province.c: 0x2E96 fleet_row drawn on fb */
void     map_sprite_8e62(uint8_t *fb, uint16_t spr, int x, int y);/* map.c: 0x8E62 with an INIT descriptor (W(0xC0F2) ...) */

#endif
