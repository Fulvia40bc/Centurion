#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/catalog.h"
#include "../src/gif.h"

static uint8_t *load_file(const char *path, size_t *size_out)
{
    FILE *f;
    long n;
    uint8_t *buf;

    f = fopen(path, "rb");
    if (f == NULL)
        return NULL;
    if (fseek(f, 0, SEEK_END) != 0 || (n = ftell(f)) < 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }
    buf = malloc(n > 0 ? (size_t)n : 1);
    if (buf == NULL) {
        fclose(f);
        return NULL;
    }
    if (n > 0 && fread(buf, 1, (size_t)n, f) != (size_t)n) {
        free(buf);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *size_out = (size_t)n;
    return buf;
}

static uint32_t crc32_buf(const uint8_t *data, size_t len)
{
    uint32_t crc = 0xFFFFFFFFu;
    size_t i;
    int k;

    for (i = 0; i < len; i++) {
        crc ^= data[i];
        for (k = 0; k < 8; k++)
            crc = (crc & 1u) ? (crc >> 1) ^ 0xEDB88320u : crc >> 1;
    }
    return crc ^ 0xFFFFFFFFu;
}

static void wr_u16le(FILE *f, uint16_t v)
{
    fputc((int)(v & 0xFFu), f);
    fputc((int)((v >> 8) & 0xFFu), f);
}

static void wr_u32le(FILE *f, uint32_t v)
{
    fputc((int)(v & 0xFFu), f);
    fputc((int)((v >> 8) & 0xFFu), f);
    fputc((int)((v >> 16) & 0xFFu), f);
    fputc((int)((v >> 24) & 0xFFu), f);
}

static int write_bmp(const char *path, const gif_image *img)
{
    FILE *f;
    uint32_t row_size = ((uint32_t)img->w + 3u) & ~3u;
    uint32_t img_size = row_size * (uint32_t)img->h;
    uint32_t data_off = 14u + 40u + 256u * 4u;
    uint32_t y, x, pad;
    int i;

    f = fopen(path, "wb");
    if (f == NULL)
        return -1;
    fputc('B', f);
    fputc('M', f);
    wr_u32le(f, data_off + img_size);
    wr_u32le(f, 0);
    wr_u32le(f, data_off);
    wr_u32le(f, 40);
    wr_u32le(f, img->w);
    wr_u32le(f, img->h);
    wr_u16le(f, 1);
    wr_u16le(f, 8);
    wr_u32le(f, 0);
    wr_u32le(f, img_size);
    wr_u32le(f, 2835);
    wr_u32le(f, 2835);
    wr_u32le(f, 256);
    wr_u32le(f, 256);
    for (i = 0; i < 256; i++) {
        fputc(img->pal[i][2], f);
        fputc(img->pal[i][1], f);
        fputc(img->pal[i][0], f);
        fputc(0, f);
    }
    for (y = img->h; y-- > 0; ) {
        for (x = 0; x < img->w; x++)
            fputc(img->pixels[(size_t)y * img->w + x], f);
        for (pad = img->w; pad < row_size; pad++)
            fputc(0, f);
    }
    if (ferror(f)) {
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

static int decode_one(const char *name, const uint8_t *data, size_t len,
                      const char *outdir, int *count)
{
    gif_image img;
    char path[4096];
    int rc = gif_decode(data, len, &img);

    if (rc != 0) {
        fprintf(stderr, "%s: gif_decode error %d\n", name, rc);
        return 1;
    }
    printf("%s %u %u %u %08X\n", name, (unsigned)img.w, (unsigned)img.h,
           (unsigned)img.ncolors, crc32_buf(img.pixels, (size_t)img.w * img.h));
    (*count)++;
    snprintf(path, sizeof path, "%s/%s.bmp", outdir, name);
    if (write_bmp(path, &img) != 0)
        fprintf(stderr, "%s: cannot write %s\n", name, path);
    gif_free(&img);
    return 0;
}

int main(int argc, char **argv)
{
    static const char *cts[2] = { "ACIV.CT1", "ACIV.CT2" };
    static const char *loose[4] = { "P2", "P3", "P4", "PAL.PAL" };
    int rc = 0;
    int count = 0;
    int i;

    if (argc != 3) {
        fprintf(stderr, "usage: %s GAMEDIR OUTDIR\n", argv[0]);
        return 2;
    }
    for (i = 0; i < 2; i++) {
        char path[4096];
        uint8_t *buf;
        size_t size;
        catalog c;
        int j;

        snprintf(path, sizeof path, "%s/%s", argv[1], cts[i]);
        buf = load_file(path, &size);
        if (buf == NULL) {
            fprintf(stderr, "%s: cannot read\n", path);
            return 2;
        }
        if (cat_open(&c, buf, size) != 0) {
            fprintf(stderr, "%s: cat_open failed\n", cts[i]);
            free(buf);
            return 1;
        }
        for (j = 0; j < c.count; j++) {
            if (c.entries[j].kind == 2) {
                const uint8_t *d = cat_data(&c, &c.entries[j]);
                if (d == NULL) {
                    fprintf(stderr, "%s: cat_data failed\n", c.entries[j].name);
                    rc = 1;
                    continue;
                }
                rc |= decode_one(c.entries[j].name, d, c.entries[j].length, argv[2], &count);
            }
        }
        free(buf);
    }
    for (i = 0; i < 4; i++) {
        char path[4096];
        uint8_t *buf;
        size_t size;

        snprintf(path, sizeof path, "%s/%s", argv[1], loose[i]);
        buf = load_file(path, &size);
        if (buf == NULL) {
            fprintf(stderr, "%s: cannot read\n", path);
            return 2;
        }
        rc |= decode_one(loose[i], buf, size, argv[2], &count);
        free(buf);
    }
    if (count != 68) {
        fprintf(stderr, "decoded %d GIFs, expected 68\n", count);
        rc = 1;
    }
    return rc;
}
