#include "palette.h"

#include <string.h>

void pal_gif_to_dac(const uint8_t gif[][3], int ncolors, uint8_t dac[256][3])
{
    int i, k;

    memset(dac, 0, 256 * 3);
    for (i = 0; i < ncolors && i < 256; i++)
        for (k = 0; k < 3; k++)
            dac[i][k] = (uint8_t)(gif[i][k] >> 2);
}

void pal_dac_to_rgb(const uint8_t dac[256][3], uint8_t rgb[256][3])
{
    int i, k;

    for (i = 0; i < 256; i++)
        for (k = 0; k < 3; k++) {
            uint8_t v = (uint8_t)(dac[i][k] & 0x3F);
            rgb[i][k] = (uint8_t)(v << 2 | v >> 4);
        }
}
