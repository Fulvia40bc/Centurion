#ifndef STATE_H
#define STATE_H
#include <stdint.h>

/* Image of the original's data segment; see state.c. */
extern uint8_t g_ds[0x10000];

/* Accessors with the original's widths and signedness. */
#define B(a)   (g_ds[(uint16_t)(a)])                                  /* u8  */
#define SB(a)  ((int8_t)g_ds[(uint16_t)(a)])                          /* s8 (cbw/cwde) */
#define W(a)   ((uint16_t)(g_ds[(uint16_t)(a)] | (g_ds[(uint16_t)((a) + 1)] << 8)))
#define SW(a)  ((int16_t)W(a))
static inline void setW(uint16_t a, uint16_t v) { g_ds[a] = (uint8_t)v; g_ds[(uint16_t)(a + 1)] = (uint8_t)(v >> 8); }

int state_init(const char *ovl0_path);   /* DS image from OVL0.OVL (file 0xB400 = DS:0000) */
int state_load(const char *path);        /* .CSV -> DS via the save block table at DS:5BDE */
int state_save(const char *path);        /* DS -> .CSV */

void end_turn(void);                     /* EXE 0x09BA, see endturn.c */

#endif
