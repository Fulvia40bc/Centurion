#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/catalog.h"
#include "../src/wav.h"

static uint8_t *load_file(const char *path, size_t *size)
{
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    long n;

    if (f == NULL)
        return NULL;
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = malloc(n > 0 ? (size_t)n : 1);
    if (buf == NULL || fread(buf, 1, (size_t)n, f) != (size_t)n) {
        fclose(f);
        free(buf);
        return NULL;
    }
    fclose(f);
    *size = (size_t)n;
    return buf;
}

static int ends_wav(const char *name)
{
    size_t n = strlen(name);

    return n >= 4 && name[n - 4] == '.' &&
           (name[n - 3] | 0x20) == 'w' && (name[n - 2] | 0x20) == 'a' && (name[n - 1] | 0x20) == 'v';
}

int main(int argc, char **argv)
{
    char path[4096];
    uint8_t *buf;
    size_t size;
    catalog c;
    int rc = 0;
    int j;
    const char *dump_name = argc > 2 ? argv[2] : NULL;

    if (argc < 2) {
        fprintf(stderr, "usage: %s GAMEDIR [NAME]\n", argv[0]);
        return 2;
    }
    snprintf(path, sizeof path, "%s/ACIV.CT5", argv[1]);
    buf = load_file(path, &size);
    if (buf == NULL) {
        fprintf(stderr, "%s: cannot read\n", path);
        return 2;
    }
    if (cat_open(&c, buf, size) != 0) {
        fprintf(stderr, "ACIV.CT5: cat_open failed\n");
        free(buf);
        return 1;
    }
    for (j = 0; j < c.count; j++) {
        const uint8_t *d;
        wav_sound w;
        int wrc;

        if (!ends_wav(c.entries[j].name))
            continue;
        if (dump_name != NULL && cat_find(&c, dump_name) != &c.entries[j])
            continue;
        d = cat_data(&c, &c.entries[j]);
        if (d == NULL) {
            fprintf(stderr, "%s: cat_data failed\n", c.entries[j].name);
            rc = 1;
            continue;
        }
        wrc = wav_decode(d, c.entries[j].length, &w);
        if (wrc != 0) {
            fprintf(stderr, "%s: wav_decode error %d\n", c.entries[j].name, wrc);
            rc = 1;
            continue;
        }
        if (dump_name != NULL) {
            uint32_t i, n = w.frames * w.channels;
            for (i = 0; i < n; i++)
                printf("%d\n", (int)w.samples[i]);
        } else {
            uint32_t i, n = w.frames * w.channels;
            uint32_t sum = 0;
            for (i = 0; i < n; i++)
                sum += (uint16_t)w.samples[i];
            printf("%s %u %u %u %u\n", c.entries[j].name, (unsigned)w.rate,
                   (unsigned)w.channels, (unsigned)w.frames, (unsigned)sum);
        }
        wav_free(&w);
    }
    free(buf);
    return rc;
}
