/* gm_render GAMEDIR NAME SECONDS SF2 OUT.wav
   Plays one XMIDI song from ACIV.CT5 through xmiseq + gm_synth (FluidSynth) for SECONDS, stops it, renders two more
   seconds of release and reverb tail, and writes a 44.1 kHz 16-bit stereo WAV. For listening, not for byte checks. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/catalog.h"
#include "../src/gm_synth.h"
#include "../src/xmi.h"
#include "../src/xmiseq.h"

enum { RATE = 44100 };

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
    fwrite(h, 1, sizeof h, f);
    for (i = 0; i < frames * 2; i++) {  /* little-endian regardless of host */
        uint8_t b[2];
        put16(b, (uint16_t)pcm[i]);
        fwrite(b, 1, 2, f);
    }
    return fclose(f);
}

int main(int argc, char **argv)
{
    char path[1024];
    uint8_t *ct5;
    size_t size, frames = 0, cap;
    catalog cat;
    const cat_entry *e;
    xmi_song song;
    xmi_seq *seq;
    gm_synth *g;
    int16_t *pcm;
    uint64_t tick, end;
    int rc;

    if (argc != 6) {
        fprintf(stderr, "usage: gm_render GAMEDIR NAME SECONDS SF2 OUT.wav\n");
        return 2;
    }
    snprintf(path, sizeof path, "%s/ACIV.CT5", argv[1]);
    ct5 = load_file(path, &size);
    if (ct5 == NULL || cat_open(&cat, ct5, size) != 0 || (e = cat_find(&cat, argv[2])) == NULL) {
        fprintf(stderr, "gm_render: cannot find %s in %s\n", argv[2], path);
        return 1;
    }
    rc = xmi_parse(cat_data(&cat, e), e->length, &song);
    if (rc != 0) {
        fprintf(stderr, "gm_render: xmi_parse %s failed (%d)\n", argv[2], rc);
        return 1;
    }
    g = gm_synth_new(argv[4], RATE);
    if (g == NULL)
        return 1;
    seq = xmi_seq_new(&song, gm_synth_midi, NULL, g);
    if (seq == NULL)
        return 1;
    end = (uint64_t)atoi(argv[3]) * 120u;
    cap = (size_t)((end + 240) * RATE / 120 + 2);
    pcm = malloc(cap * 4);
    if (pcm == NULL)
        return 1;
    xmi_seq_advance(seq, 0);
    for (tick = 0; tick < end + 240; tick++) {  /* + 2 s of tail after the stop */
        int n = gm_tick_frames(tick, RATE);
        if (tick == end)
            xmi_seq_stop(seq);
        gm_synth_render(g, pcm + frames * 2, n);
        frames += (size_t)n;
        if (tick < end)
            xmi_seq_advance(seq, 1);
    }
    if (write_wav(argv[5], pcm, frames) != 0) {
        fprintf(stderr, "gm_render: cannot write %s\n", argv[5]);
        return 1;
    }
    printf("%s: %llu loops, %zu frames (%.1f s) -> %s\n", argv[2], (unsigned long long)xmi_seq_loops(seq), frames,
           (double)frames / RATE, argv[5]);
    free(pcm);
    xmi_seq_free(seq);
    gm_synth_free(g);
    xmi_free(&song);
    free(ct5);
    return 0;
}
