#ifndef STATE_H
#define STATE_H

/* The original's data object (link addresses 0xB0000..0x122BFF), loaded from AC_MAIN.EXE at start-up by
   state_init() with its internal pointers at their link addresses (le_image.c). Translated code reads and writes the
   game's globals and tables at the original addresses through these accessors, so a translation can be checked line
   by line against the disassembly and the whole image compared with a DOSBox-X dump of the data object
   (MEMDUMPBIN 0188:2A9000 72C00, tools/dosbox/README.md). Little-endian, any alignment, as on the 386. */

#include <stddef.h>
#include <stdint.h>

#define DS_BASE 0xB0000u
#define DS_SIZE 0x72C00u

extern uint8_t *g_ds;   /* DS_SIZE bytes; g_ds[a - DS_BASE] is the byte at link address a */
extern int g_original;  /* --original: keep the 1995 bugs (fixed by default; each fix is marked BUG(1995) in the code) */

int  state_init(const char *gamedir);   /* loads <gamedir>/AC_MAIN.EXE's data object; 0 ok (message on stderr) */
int  ds_valid(uint32_t a, uint32_t n);  /* 1 if [a, a+n) lies inside the data object */

static inline uint8_t  D8(uint32_t a)  { return g_ds[a - DS_BASE]; }
static inline int8_t   S8(uint32_t a)  { return (int8_t)g_ds[a - DS_BASE]; }
static inline uint16_t D16(uint32_t a) { const uint8_t *p = g_ds + (a - DS_BASE); return (uint16_t)(p[0] | p[1] << 8); }
static inline int16_t  S16(uint32_t a) { return (int16_t)D16(a); }
static inline uint32_t D32(uint32_t a)
{
    const uint8_t *p = g_ds + (a - DS_BASE);
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
static inline int32_t  S32(uint32_t a) { return (int32_t)D32(a); }
static inline void setD8(uint32_t a, uint8_t v)   { g_ds[a - DS_BASE] = v; }
static inline void setD16(uint32_t a, uint16_t v) { uint8_t *p = g_ds + (a - DS_BASE); p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static inline void setD32(uint32_t a, uint32_t v)
{
    uint8_t *p = g_ds + (a - DS_BASE);
    p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24);
}
/* A pointer the original keeps in its data (a link address in 0xB0000..) as a C pointer into g_ds. */
static inline uint8_t *DP(uint32_t a)   { return g_ds + (a - DS_BASE); }
static inline const char *DSTR(uint32_t a) { return (const char *)(g_ds + (a - DS_BASE)); }

#endif
