#ifndef OPL2_H
#define OPL2_H
/* A YM3812 (OPL2, the AdLib's chip), written from scratch for the port (opl2.c). One chip; 49715.9 Hz samples. */
#include <stdint.h>

#define OPL2_RATE 49716                  /* 14318180 / 288 = 49715.9 Hz, the chip's own sample rate */

void    opl2_reset(void);                /* power-on: every register 0, every envelope silent */
void    opl2_write(uint8_t reg, uint8_t val);   /* port 388h = reg, 389h = val */
int16_t opl2_sample(void);               /* the next sample: the 9 channels summed, clipped to 16 bits */

#endif
