#include "png_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t crc_table[256];

static uint32_t crc32_update(uint32_t c, const uint8_t *p, size_t n)
{
    size_t i;

    if (crc_table[1] == 0) {
        uint32_t k;
        for (k = 0; k < 256; k++) {
            uint32_t v = k;
            int b;
            for (b = 0; b < 8; b++)
                v = (v & 1) ? 0xEDB88320u ^ (v >> 1) : v >> 1;
            crc_table[k] = v;
        }
    }
    for (i = 0; i < n; i++)
        c = crc_table[(c ^ p[i]) & 0xFF] ^ (c >> 8);
    return c;
}

static void be32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24); p[1] = (uint8_t)(v >> 16); p[2] = (uint8_t)(v >> 8); p[3] = (uint8_t)v;
}

static int chunk(FILE *f, const char *type, const uint8_t *data, size_t n)
{
    uint8_t hdr[8], crc[4];
    uint32_t c;

    be32(hdr, (uint32_t)n);
    memcpy(hdr + 4, type, 4);
    c = crc32_update(0xFFFFFFFFu, hdr + 4, 4);
    c = crc32_update(c, data, n) ^ 0xFFFFFFFFu;
    be32(crc, c);
    return fwrite(hdr, 1, 8, f) == 8 && (n == 0 || fwrite(data, 1, n, f) == n) && fwrite(crc, 1, 4, f) == 4 ? 0 : -1;
}

int png_write_indexed(const char *path, const uint8_t *pixels, int w, int h, const uint8_t pal[256][3])
{
    static const uint8_t sig[8] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n' };
    size_t row = (size_t)w + 1, raw = row * (size_t)h, nblocks = (raw + 65534) / 65535, zlen, i, pos, z;
    uint8_t ihdr[13], *zbuf;
    uint32_t a = 1, b = 0;
    FILE *f;
    int rc;

    zlen = 2 + raw + 5 * nblocks + 4;
    zbuf = malloc(zlen);
    if (zbuf == NULL)
        return -1;
    zbuf[0] = 0x78; zbuf[1] = 0x01;   /* zlib header: deflate, 32 KB window, no preset dictionary */
    z = 2;
    pos = 0;
    for (i = 0; i < nblocks; i++) {
        size_t n = raw - pos < 65535 ? raw - pos : 65535, k;
        zbuf[z++] = (uint8_t)(i + 1 == nblocks);           /* BFINAL, BTYPE 00 = stored */
        zbuf[z++] = (uint8_t)n; zbuf[z++] = (uint8_t)(n >> 8);
        zbuf[z++] = (uint8_t)~n; zbuf[z++] = (uint8_t)(~n >> 8);
        for (k = 0; k < n; k++, pos++) {                    /* the filtered image: filter byte 0, then the row */
            size_t yy = pos / row, xx = pos % row;
            uint8_t v = xx == 0 ? 0 : pixels[yy * (size_t)w + xx - 1];
            zbuf[z++] = v;
            a = (a + v) % 65521u;
            b = (b + a) % 65521u;
        }
    }
    be32(zbuf + z, (b << 16) | a);
    z += 4;
    f = fopen(path, "wb");
    if (f == NULL) {
        free(zbuf);
        return -1;
    }
    be32(ihdr, (uint32_t)w); be32(ihdr + 4, (uint32_t)h);
    ihdr[8] = 8; ihdr[9] = 3; ihdr[10] = 0; ihdr[11] = 0; ihdr[12] = 0;
    rc = fwrite(sig, 1, 8, f) == 8 ? 0 : -1;
    if (rc == 0) rc = chunk(f, "IHDR", ihdr, 13);
    if (rc == 0) rc = chunk(f, "PLTE", &pal[0][0], 768);
    if (rc == 0) rc = chunk(f, "IDAT", zbuf, z);
    if (rc == 0) rc = chunk(f, "IEND", NULL, 0);
    if (fclose(f) != 0)
        rc = -1;
    free(zbuf);
    return rc;
}
