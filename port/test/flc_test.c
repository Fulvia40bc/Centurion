#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/catalog.h"
#include "../src/flc.h"

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

static int play_one(const char *name, const uint8_t *data, size_t len, int *count)
{
    flc_anim a;
    uint32_t first_pix, first_pal, ring_pix, ring_pal;
    uint32_t i;
    int rc = flc_open(&a, data, len);
    int bad = 0;

    if (rc != 0) {
        fprintf(stderr, "%s: flc_open error %d\n", name, rc);
        return 1;
    }
    printf("%s %u %u %u %u\n", name, (unsigned)a.w, (unsigned)a.h,
           (unsigned)a.frames, (unsigned)a.speed_ms);
    rc = flc_next(&a);
    if (rc != 0) {
        fprintf(stderr, "%s: flc_next error %d on frame 0\n", name, rc);
        flc_close(&a);
        return 1;
    }
    first_pix = crc32_buf(a.pixels, (size_t)a.w * a.h);
    first_pal = crc32_buf((const uint8_t *)a.pal, sizeof a.pal);
    printf("%u %08X %08X\n", (unsigned)a.index, first_pix, first_pal);
    for (i = 1; i < a.frames; i++) {
        rc = flc_next(&a);
        if (rc != 0) {
            fprintf(stderr, "%s: flc_next error %d on frame %u\n", name, rc, (unsigned)i);
            bad = 1;
            break;
        }
        printf("%u %08X %08X\n", (unsigned)a.index,
               crc32_buf(a.pixels, (size_t)a.w * a.h),
               crc32_buf((const uint8_t *)a.pal, sizeof a.pal));
    }
    rc = flc_next(&a);
    if (rc != 0) {
        fprintf(stderr, "%s: flc_next error %d on the ring frame\n", name, rc);
        flc_close(&a);
        return 1;
    }
    ring_pix = crc32_buf(a.pixels, (size_t)a.w * a.h);
    ring_pal = crc32_buf((const uint8_t *)a.pal, sizeof a.pal);
    printf("%u %08X %08X\n", (unsigned)a.index, ring_pix, ring_pal);
    if (ring_pix != first_pix || ring_pal != first_pal || a.index != 0) {
        fprintf(stderr, "%s: ring frame does not return to frame 0 (pix %08X vs %08X, pal %08X vs %08X, index %u)\n",
                name, ring_pix, first_pix, ring_pal, first_pal, (unsigned)a.index);
        bad = 1;
    }
    (*count)++;
    flc_close(&a);
    return bad;
}

int main(int argc, char **argv)
{
    static const char *loose[2] = { "AH_LOGO.FLC", "ADVCIVIL.FLC" };
    int rc = 0;
    int count = 0;
    int i;

    if (argc != 2) {
        fprintf(stderr, "usage: %s GAMEDIR\n", argv[0]);
        return 2;
    }
    {
        char path[4096];
        uint8_t *buf;
        size_t size;
        catalog c;
        int j;

        snprintf(path, sizeof path, "%s/ACIV.CT4", argv[1]);
        buf = load_file(path, &size);
        if (buf == NULL) {
            fprintf(stderr, "%s: cannot read\n", path);
            return 2;
        }
        if (cat_open(&c, buf, size) != 0) {
            fprintf(stderr, "ACIV.CT4: cat_open failed\n");
            free(buf);
            return 1;
        }
        for (j = 0; j < c.count; j++) {
            if (c.entries[j].kind == 6) {
                const uint8_t *d = cat_data(&c, &c.entries[j]);
                if (d == NULL) {
                    fprintf(stderr, "%s: cat_data failed\n", c.entries[j].name);
                    rc = 1;
                    continue;
                }
                rc |= play_one(c.entries[j].name, d, c.entries[j].length, &count);
            }
        }
        free(buf);
    }
    for (i = 0; i < 2; i++) {
        char path[4096];
        uint8_t *buf;
        size_t size;

        snprintf(path, sizeof path, "%s/%s", argv[1], loose[i]);
        buf = load_file(path, &size);
        if (buf == NULL) {
            fprintf(stderr, "%s: cannot read\n", path);
            return 2;
        }
        rc |= play_one(loose[i], buf, size, &count);
        free(buf);
    }
    if (count != 16) {
        fprintf(stderr, "played %d FLC files, expected 16\n", count);
        rc = 1;
    }
    return rc;
}
