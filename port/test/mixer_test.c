#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/catalog.h"
#include "../src/mixer.h"
#include "../src/wav.h"

#define RATE     44100
#define SECONDS  6
#define PIECE    1000

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

    if (f == NULL)
        return -1;
    memcpy(hdr, "RIFF", 4);
    put_u32le(hdr + 4, 36 + data_size);
    memcpy(hdr + 8, "WAVE", 4);
    memcpy(hdr + 12, "fmt ", 4);
    put_u32le(hdr + 16, 16);
    put_u16le(hdr + 20, 1);                 /* PCM */
    put_u16le(hdr + 22, 2);                 /* stereo */
    put_u32le(hdr + 24, RATE);
    put_u32le(hdr + 28, RATE * 4u);
    put_u16le(hdr + 32, 4);
    put_u16le(hdr + 34, 16);
    memcpy(hdr + 36, "data", 4);
    put_u32le(hdr + 40, data_size);
    if (fwrite(hdr, 1, 44, f) != 44) {
        fclose(f);
        return -1;
    }
    for (uint32_t i = 0; i < frames * 2u; i++) {
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

static int get_sound(const catalog *c, const char *name, wav_sound *w)
{
    const cat_entry *e = cat_find(c, name);
    const uint8_t *d;

    if (e == NULL)
        return -1;
    d = cat_data(c, e);
    if (d == NULL)
        return -1;
    return wav_decode(d, e->length, w);
}

int main(int argc, char **argv)
{
    static const struct { long frame; const char *name; int volume, pan; } starts[4] = {
        { 0,     "MARKETPL.WAV", 127, 64 },
        { 22050, "KABOOM3.WAV",   100, 0 },
        { 44100, "BRPC.WAV",       90, 127 },
        { 88200, "MARKETPL.WAV",  127, 32 },
    };
    char path[4096];
    uint8_t *buf;
    size_t size;
    catalog c;
    wav_sound snd[3];
    const char *names[3] = { "MARKETPL.WAV", "KABOOM3.WAV", "BRPC.WAV" };
    mixer *m;
    int16_t *out;
    int32_t total = SECONDS * RATE;
    int32_t fp = 0;
    int si = 0;
    int64_t sumL = 0, sumR = 0;
    int i;
    int rc = 0;

    if (argc != 3) {
        fprintf(stderr, "usage: %s GAMEDIR OUT.wav\n", argv[0]);
        return 2;
    }
    snprintf(path, sizeof path, "%s/ACIV.CT5", argv[1]);
    buf = load_file(path, &size);
    if (buf == NULL || cat_open(&c, buf, size) != 0) {
        fprintf(stderr, "%s: cannot read\n", path);
        return 2;
    }
    for (i = 0; i < 3; i++) {
        if (get_sound(&c, names[i], &snd[i]) != 0) {
            fprintf(stderr, "%s: cannot decode\n", names[i]);
            free(buf);
            return 1;
        }
    }
    m = mix_new();
    if (m == NULL) {
        fprintf(stderr, "mix_new failed\n");
        return 1;
    }
    out = malloc((size_t)total * 2u * sizeof(int16_t));
    if (out == NULL) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }
    while (fp < total) {
        int32_t piece = total - fp;
        if (piece > PIECE)
            piece = PIECE;
        if (si < 4 && starts[si].frame > fp && starts[si].frame - fp < piece)
            piece = (int32_t)(starts[si].frame - fp);
        while (si < 4 && starts[si].frame == fp) {
            const wav_sound *s = NULL;
            for (i = 0; i < 3; i++)
                if (strcmp(names[i], starts[si].name) == 0)
                    s = &snd[i];
            if (mix_play(m, s, starts[si].volume, starts[si].pan) < 0) {
                fprintf(stderr, "no free voice at frame %ld\n", starts[si].frame);
                rc = 1;
            }
            si++;
        }
        mix_render(m, out + 2 * (size_t)fp, (int)piece);
        fp += piece;
    }
    for (i = 0; i < total; i++) {
        sumL += out[2 * i];
        sumR += out[2 * i + 1];
    }
    if (write_wav(argv[2], out, (uint32_t)total) != 0) {
        fprintf(stderr, "%s: cannot write\n", argv[2]);
        rc = 1;
    }
    printf("%d %" PRId64 " %" PRId64 " %u\n", (int)total, sumL, sumR, mix_clipped(m));
    mix_free(m);
    free(out);
    for (i = 0; i < 3; i++)
        wav_free(&snd[i]);
    free(buf);
    return rc;
}
