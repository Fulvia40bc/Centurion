#include "wav.h"

#include <stdlib.h>
#include <string.h>

static uint16_t rd_u16le(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t rd_u32le(const uint8_t *p)
{
    return ((uint32_t)p[0])
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

int wav_decode(const uint8_t *data, size_t len, wav_sound *out)
{
    size_t pos;
    int have_fmt = 0, have_data = 0;
    uint16_t tag = 0, channels = 0, block_align = 0, bits = 0;
    uint32_t rate = 0;
    const uint8_t *dpos = NULL;
    size_t dsize = 0;

    if (out == NULL)
        return -1;
    memset(out, 0, sizeof *out);
    if (data == NULL || len < 12)
        return -1;
    if (memcmp(data, "RIFF", 4) != 0 || memcmp(data + 8, "WAVE", 4) != 0)
        return -3;

    pos = 12;
    while (pos + 8 <= len) {
        const uint8_t *id = data + pos;
        uint32_t size = rd_u32le(data + pos + 4);
        size_t body = pos + 8;
        if ((size_t)size > len - body)
            return -2;                          /* chunk runs past the end of the buffer */
        if (memcmp(id, "fmt ", 4) == 0 && !have_fmt) {
            if (size < 16)
                return -3;
            tag = rd_u16le(data + body);
            channels = rd_u16le(data + body + 2);
            rate = rd_u32le(data + body + 4);
            block_align = rd_u16le(data + body + 12);
            bits = rd_u16le(data + body + 14);
            have_fmt = 1;
        } else if (memcmp(id, "data", 4) == 0 && !have_data) {
            if (!have_fmt)
                return -3;                      /* fmt must come before data */
            dpos = data + body;
            dsize = size;
            have_data = 1;
        }
        pos = body + (size_t)size + (size & 1u); /* odd chunk sizes are padded to even */
    }
    if (!have_fmt || !have_data)
        return -3;
    if (tag != 1u)
        return -4;                              /* not PCM */
    if (channels < 1u || channels > 2u)
        return -4;
    if (bits != 8u && bits != 16u)
        return -4;
    if (block_align != (uint16_t)(channels * (bits / 8u)))
        return -4;

    {
        uint32_t frame_bytes = (uint32_t)channels * (bits / 8u);
        uint32_t frames = (uint32_t)(dsize / frame_bytes);  /* an odd trailing byte of 16-bit data is dropped */
        uint32_t n = frames * channels;
        int16_t *s;

        out->rate = rate;
        out->channels = channels;
        out->frames = frames;
        if (n == 0) {
            out->samples = NULL;
            return 0;
        }
        s = malloc((size_t)n * sizeof *s);
        if (s == NULL)
            return -5;
        if (bits == 8u) {
            uint32_t i;
            for (i = 0; i < n; i++)
                s[i] = (int16_t)(((int)dpos[i] - 128) * 256);  /* unsigned 8-bit, centred on 128 */
        } else {
            uint32_t i;
            for (i = 0; i < n; i++) {
                uint32_t v = rd_u16le(dpos + 2u * i);
                int32_t sv = (v >= 0x8000u) ? (int32_t)v - 0x10000 : (int32_t)v;
                s[i] = (int16_t)sv;
            }
        }
        out->samples = s;
    }
    return 0;
}

void wav_free(wav_sound *w)
{
    if (w == NULL)
        return;
    free(w->samples);
    w->samples = NULL;
    w->rate = 0;
    w->channels = 0;
    w->frames = 0;
}
