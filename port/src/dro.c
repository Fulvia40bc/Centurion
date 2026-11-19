#include "dro.h"

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

int dro_parse(const uint8_t *data, size_t len, dro_file *out)
{
    size_t hdr, pos;
    uint16_t major, minor;
    uint32_t pairs, length_ms, i;
    uint8_t hardware, format, compression, short_code, long_code, map_len;
    const uint8_t *codemap;
    dro_event *ev = NULL;
    uint32_t n = 0, cap = 0;
    uint64_t pending_ms = 0;

    if (out == NULL)
        return -1;
    memset(out, 0, sizeof *out);
    if (data == NULL)
        return -1;
    if (len >= 8 && memcmp(data, "DBRAWOPL", 8) != 0)
        return -3;                              /* not a DRO file */
    if (len < 26)
        return -2;                              /* truncated header */
    major = rd_u16le(data + 8);
    minor = rd_u16le(data + 10);
    if (major != 2 || minor != 0)
        return -3;                              /* not DRO v2.0 */
    pairs = rd_u32le(data + 12);
    length_ms = rd_u32le(data + 16);
    hardware = data[20];
    format = data[21];
    compression = data[22];
    short_code = data[23];
    long_code = data[24];
    map_len = data[25];
    if (format != 0 || compression != 0)
        return -4;                              /* unsupported */
    hdr = 26 + (size_t)map_len;
    if (len < hdr || (uint64_t)pairs * 2u > (uint64_t)(len - hdr))
        return -2;                              /* truncated */
    codemap = data + 26;
    pos = hdr;
    for (i = 0; i < pairs; i++) {
        uint8_t index = data[pos];
        uint8_t value = data[pos + 1];
        pos += 2;
        if (index == short_code) {
            pending_ms += (uint64_t)value + 1u;
            continue;
        }
        if (index == long_code) {
            pending_ms += ((uint64_t)value + 1u) * 256u;
            continue;
        }
        {
            unsigned ci = index & 0x7Fu;
            if (ci >= map_len)
                goto bad_map;
            if (pending_ms > 0xFFFFFFFFull) {    /* delay_ms is u32: refuse instead of wrapping */
                free(ev);
                return -4;
            }
            if (n == cap) {
                uint32_t ncap = cap == 0 ? 256 : cap * 2;
                dro_event *nev = realloc(ev, (size_t)ncap * sizeof *nev);
                if (nev == NULL)
                    goto oom;
                ev = nev;
                cap = ncap;
            }
            ev[n].delay_ms = (uint32_t)pending_ms;
            ev[n].bank = (uint8_t)(index >> 7);
            ev[n].reg = codemap[ci];
            ev[n].value = value;
            n++;
            pending_ms = 0;
        }
    }
    out->ev = ev;
    out->count = n;
    out->length_ms = length_ms;
    out->hardware = hardware;
    return 0;

bad_map:
    free(ev);
    return -4;                                  /* codemap index out of range */
oom:
    free(ev);
    return -5;
}

void dro_free(dro_file *f)
{
    if (f == NULL)
        return;
    free(f->ev);
    f->ev = NULL;
    f->count = 0;
}
