#ifndef OPL_H
#define OPL_H

#include <stddef.h>
#include <stdint.h>

typedef struct opl opl;

opl  *opl_new(void);                                  /* an OPL3 at its native rate, 49716 Hz; NULL = no memory */
void  opl_free(opl *o);
void  opl_reset(opl *o);
void  opl_write(opl *o, int bank, uint8_t reg, uint8_t value);   /* bank 0: ports 0x388/0x389, 1: 0x38A/0x38B */
void  opl_render(opl *o, int16_t *out, int frames);   /* interleaved stereo at 49716 Hz, the chip's own output */

#endif
