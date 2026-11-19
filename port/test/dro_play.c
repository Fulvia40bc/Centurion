#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/dro.h"
#include "../src/opl.h"

#define OPL_RATE 49716u

static uint8_t *load_file(const char *path, size_t *size_out)
{
    FILE *f = fopen(path, "rb");
    long n;
    uint8_t *buf;

    if (f == NULL)
        return NULL;
    if (fseek(f, 0, SEEK_END) != 0 || (n = ftell(f)) < 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }
    buf = malloc(n > 0 ? (size_t)n : 1);
    if (buf == NULL || (n > 0 && fread(buf, 1, (size_t)n, f) != (size_t)n)) {
        fclose(f);
        free(buf);
        return NULL;
    }
    fclose(f);
    *size_out = (size_t)n;
    return buf;
}

static void put_u16le(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
}

static void put_u32le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static int write_wav(const char *path, const int16_t *samples, uint32_t frames)
{
    FILE *f = fopen(path, "wb");
    uint8_t hdr[44];
    uint32_t data_size = frames * 2u * 2u;
    uint32_t i;

    if (f == NULL)
        return -1;
    memcpy(hdr, "RIFF", 4);
    put_u32le(hdr + 4, 36 + data_size);
    memcpy(hdr + 8, "WAVE", 4);
    memcpy(hdr + 12, "fmt ", 4);
    put_u32le(hdr + 16, 16);
    put_u16le(hdr + 20, 1);
    put_u16le(hdr + 22, 2);
    put_u32le(hdr + 24, OPL_RATE);
    put_u32le(hdr + 28, OPL_RATE * 4u);
    put_u16le(hdr + 32, 4);
    put_u16le(hdr + 34, 16);
    memcpy(hdr + 36, "data", 4);
    put_u32le(hdr + 40, data_size);
    if (fwrite(hdr, 1, 44, f) != 44) {
        fclose(f);
        return -1;
    }
    for (i = 0; i < frames * 2u; i++) {
        uint8_t b[2];
        put_u16le(b, (uint16_t)samples[i]);
        if (fwrite(b, 1, 2, f) != 2) {
            fclose(f);
            return -1;
        }
    }
    fclose(f);
    return 0;
}

int main(int argc, char **argv)
{
    uint8_t *buf;
    size_t size;
    dro_file dro;
    opl *o;
    uint64_t total_ms = 0;
    uint64_t rendered = 0;
    uint64_t target_frames;
    uint32_t i;
    int16_t *out;
    int64_t sum_l = 0, sum_r = 0;
    int rc = 0;

    if (argc != 3) {
        fprintf(stderr, "usage: %s FILE.dro OUT.wav\n", argv[0]);
        return 2;
    }
    buf = load_file(argv[1], &size);
    if (buf == NULL) {
        fprintf(stderr, "%s: cannot read\n", argv[1]);
        return 2;
    }
    if (dro_parse(buf, size, &dro) != 0) {
        fprintf(stderr, "%s: dro_parse failed\n", argv[1]);
        free(buf);
        return 1;
    }
    o = opl_new();
    if (o == NULL) {
        fprintf(stderr, "opl_new failed\n");
        dro_free(&dro);
        free(buf);
        return 1;
    }
    target_frames = ((uint64_t)dro.length_ms * OPL_RATE + 500u) / 1000u;
    out = malloc((size_t)(target_frames > 0 ? target_frames : 1) * 2u * sizeof(int16_t));
    if (out == NULL) {
        fprintf(stderr, "out of memory\n");
        opl_free(o);
        dro_free(&dro);
        free(buf);
        return 1;
    }
    for (i = 0; i < dro.count; i++) {
        uint64_t want;
        total_ms += dro.ev[i].delay_ms;
        want = (total_ms * OPL_RATE + 500u) / 1000u;   /* round to nearest, exact integer math */
        if (want > target_frames)
            want = target_frames;
        if (want > rendered) {
            opl_render(o, out + 2 * (size_t)rendered, (int)(want - rendered));
            rendered = want;
        }
        opl_write(o, dro.ev[i].bank, dro.ev[i].reg, dro.ev[i].value);
    }
    if (rendered < target_frames) {
        opl_render(o, out + 2 * (size_t)rendered, (int)(target_frames - rendered));
        rendered = target_frames;
    }
    for (i = 0; i < (uint32_t)rendered; i++) {
        sum_l += out[2 * i];
        sum_r += out[2 * i + 1];
    }
    if (write_wav(argv[2], out, (uint32_t)rendered) != 0) {
        fprintf(stderr, "%s: cannot write\n", argv[2]);
        rc = 1;
    }
    printf("%u %u %llu %lld %lld\n", dro.count, dro.length_ms,
           (unsigned long long)rendered, (long long)sum_l, (long long)sum_r);
    free(out);
    opl_free(o);
    dro_free(&dro);
    free(buf);
    return rc;
}
