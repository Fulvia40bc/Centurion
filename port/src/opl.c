#include "opl.h"

#include <stdlib.h>

#include "../third_party/nuked-opl3/opl3.h"

#define OPL_NATIVE_RATE 49716u

struct opl {
    opl3_chip chip;
};

opl *opl_new(void)
{
    opl *o = malloc(sizeof *o);

    if (o == NULL)
        return NULL;
    OPL3_Reset(&o->chip, OPL_NATIVE_RATE);
    return o;
}

void opl_free(opl *o)
{
    free(o);
}

void opl_reset(opl *o)
{
    if (o != NULL)
        OPL3_Reset(&o->chip, OPL_NATIVE_RATE);
}

void opl_write(opl *o, int bank, uint8_t reg, uint8_t value)
{
    if (o == NULL)
        return;
    /* Nuked-OPL3 selects the port bank with bit 8 of the register number */
    OPL3_WriteReg(&o->chip, (uint16_t)(reg | (bank != 0 ? 0x100u : 0u)), value);
}

void opl_render(opl *o, int16_t *out, int frames)
{
    int i;

    if (o == NULL || out == NULL)
        return;
    for (i = 0; i < frames; i++)
        OPL3_Generate(&o->chip, out + 2 * i);   /* one stereo sample per call, unbuffered */
}
