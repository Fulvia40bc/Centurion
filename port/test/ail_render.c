/* ail_render GAMEDIR SONG.XMI SECONDS SF2 OUT.wav [WAV@SECOND ...]
   Plays one XMIDI song from ACIV.CT5 through the ail_* facade (init, start, volume 112) and starts each named WAV
   at its second on sample slots 1..3 (loop count 1, volume 127), rendering offline in 1/100 s pieces with
   ail_set_live(1); prints the statuses seen at each second and writes a 44.1 kHz 16-bit stereo WAV.
   SF2 "-" or "none" means no SoundFont (the state machine still runs, silently). For listening, not byte checks. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/ail.h"
#include "../src/catalog.h"

enum { RATE = 44100, PIECE = RATE / 100 };   /* 1/100 s pieces */

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
    if (buf == NULL || (n > 0 && fread(buf, 1, (size_t)n, f) != (size_t)n)) {
        free(buf);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *size = (size_t)n;
    return buf;
}

static void put32(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }
static void put16(uint8_t *p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }

static int write_wav(const char *path, const int16_t *pcm, size_t frames)
{
    uint8_t h[44];
    FILE *f;
    size_t i, bytes = frames * 4;

    memcpy(h, "RIFF", 4); put32(h + 4, (uint32_t)(36 + bytes)); memcpy(h + 8, "WAVEfmt ", 8);
    put32(h + 16, 16); put16(h + 20, 1); put16(h + 22, 2); put32(h + 24, RATE); put32(h + 28, RATE * 4);
    put16(h + 32, 4); put16(h + 34, 16); memcpy(h + 36, "data", 4); put32(h + 40, (uint32_t)bytes);
    f = fopen(path, "wb");
    if (f == NULL)
        return -1;
    if (fwrite(h, 1, sizeof h, f) != sizeof h) {
        fclose(f);
        return -1;
    }
    for (i = 0; i < frames * 2; i++) {  /* little-endian regardless of host */
        uint8_t b[2];
        put16(b, (uint16_t)pcm[i]);
        if (fwrite(b, 1, 2, f) != 2) {
            fclose(f);
            return -1;
        }
    }
    return fclose(f);
}

int main(int argc, char **argv)
{
    char path[1024];
    uint8_t *ct5;
    size_t size, frames = 0, total;
    catalog cat;
    const cat_entry *e;
    const char *sf2;
    int32_t sh, h[3];
    long starts[3];
    const cat_entry *we[3];
    int nwav = 0, i, sec, rc = 0;
    int16_t *pcm;

    if (argc < 6) {
        fprintf(stderr, "usage: ail_render GAMEDIR SONG.XMI SECONDS SF2 OUT.wav [WAV@SECOND ...]\n");
        return 2;
    }
    if (argc > 9) {
        fprintf(stderr, "ail_render: at most 3 WAV@SECOND arguments (slots 1..3)\n");
        return 2;
    }
    nwav = argc - 6;
    for (i = 0; i < nwav; i++) {
        const char *at = strchr(argv[6 + i], '@');
        if (at == NULL || at == argv[6 + i] || atoi(at + 1) < 0) {
            fprintf(stderr, "ail_render: bad WAV@SECOND '%s'\n", argv[6 + i]);
            return 2;
        }
        starts[i] = atol(at + 1);
    }

    snprintf(path, sizeof path, "%s/ACIV.CT5", argv[1]);
    ct5 = load_file(path, &size);
    if (ct5 == NULL || cat_open(&cat, ct5, size) != 0) {
        fprintf(stderr, "ail_render: cannot read %s\n", path);
        return 1;
    }
    e = cat_find(&cat, argv[2]);
    if (e == NULL) {
        fprintf(stderr, "ail_render: %s not in %s\n", argv[2], path);
        return 1;
    }
    for (i = 0; i < nwav; i++) {
        char name[16];
        const char *at = strchr(argv[6 + i], '@');
        size_t nl = (size_t)(at - argv[6 + i]);
        if (nl >= sizeof name) {
            fprintf(stderr, "ail_render: WAV name too long '%s'\n", argv[6 + i]);
            return 2;
        }
        memcpy(name, argv[6 + i], nl);
        name[nl] = '\0';
        we[i] = cat_find(&cat, name);
        if (we[i] == NULL) {
            fprintf(stderr, "ail_render: %s not in %s\n", name, path);
            return 1;
        }
    }

    sf2 = (strcmp(argv[4], "-") == 0 || strcmp(argv[4], "none") == 0) ? NULL : argv[4];
    ail_init(NULL, sf2);
    ail_set_live(1);

    sh = ail_allocate_sequence();
    if (sh == 0 || ail_init_sequence(sh, cat_data(&cat, e), e->length, 0) != 1) {
        fprintf(stderr, "ail_render: init_sequence failed: %s", ail_last_error());
        return 1;
    }
    ail_start_sequence(sh);
    ail_set_sequence_volume(sh, 112, 0);

    for (i = 0; i < nwav; i++) {
        h[i] = ail_allocate_sample();
        if (h[i] == 0) {
            fprintf(stderr, "ail_render: no sample handle: %s", ail_last_error());
            return 1;
        }
        ail_init_sample(h[i]);
        if (ail_set_sample_file(h[i], cat_data(&cat, we[i]), we[i]->length) != 1) {
            fprintf(stderr, "ail_render: %s refused: %s", we[i]->name, ail_last_error());
            return 1;
        }
        ail_set_sample_loop_count(h[i], 1);
        ail_set_sample_volume(h[i], 127);
    }

    total = (size_t)atol(argv[3]) * RATE;
    pcm = malloc((total + PIECE) * 4);
    if (pcm == NULL)
        return 1;
    while (frames < total) {
        size_t piece = total - frames;
        if (piece > PIECE)
            piece = PIECE;
        for (i = 0; i < nwav; i++)
            if ((long)frames == starts[i] * RATE)
                ail_start_sample(h[i]);
        ail_render(pcm + frames * 2, (int)piece);
        frames += piece;
        if (frames % RATE == 0) {
            sec = (int)(frames / RATE);
            printf("sec %d: seq %d samples", sec, ail_sequence_status(sh));
            for (i = 0; i < nwav; i++)
                printf(" %d", ail_sample_status(h[i]));
            printf(" active %d\n", ail_active_sample_count());
        }
    }
    if (write_wav(argv[5], pcm, frames) != 0) {
        fprintf(stderr, "ail_render: cannot write %s\n", argv[5]);
        rc = 1;
    } else {
        printf("%s + %d wav(s): %zu frames (%.1f s) -> %s\n", argv[2], nwav, frames, (double)frames / RATE, argv[5]);
    }
    ail_shutdown();
    free(pcm);
    free(ct5);
    return rc;
}
