#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/catalog.h"
#include "../src/xmi.h"
#include "../src/xmiseq.h"

typedef struct { uint64_t tick; uint8_t len; uint8_t msg[3]; } rec;

typedef struct {
    rec *v;
    size_t n, cap;
    uint64_t cur_tick;
    uint32_t ons, offs, trigs;
    uint32_t xoffs;          /* note-offs with no open note-on: real 0x8n events written in the EVNT stream,
                                forwarded since 22d (the queue still sends its own off for the same key) */
    int open[16][128];       /* notes currently sounding per (channel, key) */
} ctx;

static void out_cb(void *user, const uint8_t *msg, int len)
{
    ctx *c = user;
    rec *r;

    if (c->n == c->cap) {
        size_t ncap = c->cap == 0 ? 1024 : c->cap * 2;
        rec *nv = realloc(c->v, ncap * sizeof *nv);
        if (nv == NULL)
            return;
        c->v = nv;
        c->cap = ncap;
    }
    r = &c->v[c->n++];
    r->tick = c->cur_tick;
    r->len = (uint8_t)len;
    memcpy(r->msg, msg, (size_t)len);
    if ((msg[0] & 0xF0u) == 0x90u) {
        c->ons++;
        c->open[msg[0] & 0x0Fu][msg[1]]++;
    } else if ((msg[0] & 0xF0u) == 0x80u) {
        int ch = msg[0] & 0x0Fu, key = msg[1];

        c->offs++;
        if (c->open[ch][key] > 0)
            c->open[ch][key]--;                 /* pairs with a note-on */
        else
            c->xoffs++;                         /* a real EVNT 0x8n: no open note-on to pair with */
    }
}

static void trig_cb(void *user, int channel, int value)
{
    (void)channel;
    (void)value;
    ((ctx *)user)->trigs++;
}

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

static void put_u32be(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

static size_t put_vlq(uint8_t *p, uint64_t v)
{
    uint8_t tmp[10];
    size_t n = 0, i;

    do {
        tmp[n++] = (uint8_t)(v & 0x7Fu);
        v >>= 7;
    } while (v != 0);
    for (i = 0; i < n; i++)
        p[i] = (uint8_t)(tmp[n - 1 - i] | (i + 1 < n ? 0x80u : 0u));
    return n;
}

static int write_mid(const char *path, const ctx *c, uint64_t last_tick)
{
    FILE *f;
    uint8_t *trk;
    size_t tn = 0, tcap = 4096, i;
    uint64_t prev = 0;
    static const uint8_t tempo[7] = { 0x00, 0xFF, 0x51, 0x03, 0x0F, 0x42, 0x40 }; /* delta 0, tempo 1,000,000 us/qn */
    uint8_t hdr[14];
    uint8_t sz[4];

    trk = malloc(tcap);
    if (trk == NULL)
        return -1;
    memcpy(trk, tempo, sizeof tempo);
    tn = sizeof tempo;
    for (i = 0; i < c->n; i++) {
        const rec *r = &c->v[i];
        size_t need = 5 + 3;
        if (tn + need > tcap) {
            while (tn + need > tcap)
                tcap *= 2;
            {
                uint8_t *nt = realloc(trk, tcap);
                if (nt == NULL) {
                    free(trk);
                    return -1;
                }
                trk = nt;
            }
        }
        tn += put_vlq(trk + tn, r->tick - prev);
        prev = r->tick;
        memcpy(trk + tn, r->msg, r->len);
        tn += r->len;
    }
    if (tn + 8 > tcap) {                        /* longest end-of-track: 5-byte VLQ + FF 2F 00 */
        uint8_t *nt = realloc(trk, tn + 8);
        if (nt == NULL) {
            free(trk);
            return -1;
        }
        trk = nt;
    }
    tn += put_vlq(trk + tn, last_tick - prev);  /* end of track at the stop tick */
    trk[tn++] = 0xFF;
    trk[tn++] = 0x2F;
    trk[tn++] = 0x00;

    memcpy(hdr, "MThd", 4);
    put_u32be(hdr + 4, 6);
    hdr[8] = 0; hdr[9] = 0;                     /* format 0 */
    hdr[10] = 0; hdr[11] = 1;                   /* one track */
    hdr[12] = 0; hdr[13] = 120;                 /* division 120 */
    put_u32be(sz, (uint32_t)tn);

    f = fopen(path, "wb");
    if (f == NULL) {
        free(trk);
        return -1;
    }
    if (fwrite(hdr, 1, 14, f) != 14 || fwrite("MTrk", 1, 4, f) != 4 ||
        fwrite(sz, 1, 4, f) != 4 || fwrite(trk, 1, tn, f) != tn) {
        fclose(f);
        free(trk);
        return -1;
    }
    fclose(f);
    free(trk);
    return 0;
}

int main(int argc, char **argv)
{
    char path[4096];
    uint8_t *buf;
    size_t size;
    catalog c;
    const cat_entry *e;
    const uint8_t *d;
    xmi_song song;
    xmi_seq *seq;
    ctx cx;
    unsigned long seconds, t, steps;
    uint64_t last_tick;
    int rc = 0;

    if (argc != 5) {
        fprintf(stderr, "usage: %s GAMEDIR NAME SECONDS OUT.mid\n", argv[0]);
        return 2;
    }
    seconds = strtoul(argv[3], NULL, 10);
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
    e = cat_find(&c, argv[2]);
    if (e == NULL) {
        fprintf(stderr, "%s: not in ACIV.CT5\n", argv[2]);
        free(buf);
        return 1;
    }
    d = cat_data(&c, e);
    if (d == NULL || xmi_parse(d, e->length, &song) != 0) {
        fprintf(stderr, "%s: xmi_parse failed\n", e->name);
        free(buf);
        return 1;
    }
    memset(&cx, 0, sizeof cx);
    seq = xmi_seq_new(&song, out_cb, trig_cb, &cx);
    if (seq == NULL) {
        fprintf(stderr, "xmi_seq_new failed\n");
        xmi_free(&song);
        free(buf);
        return 1;
    }
    steps = seconds * 120ul;
    cx.cur_tick = 0;
    xmi_seq_advance(seq, 0);          /* flush events at tick 0 */
    for (t = 1; t <= steps; t++) {
        cx.cur_tick = t;
        xmi_seq_advance(seq, 1);
    }
    last_tick = steps;
    cx.cur_tick = last_tick;
    xmi_seq_stop(seq);
    if (write_mid(argv[4], &cx, last_tick) != 0) {
        fprintf(stderr, "%s: cannot write\n", argv[4]);
        rc = 1;
    }
    {
        int ch, key, leftover = 0;

        for (ch = 0; ch < 16; ch++)
            for (key = 0; key < 128; key++)
                leftover += cx.open[ch][key];
        printf("%s %u %u %u %u %u\n", e->name, cx.ons, cx.offs, xmi_seq_loops(seq), cx.trigs, cx.xoffs);
        if (leftover != 0) {
            fprintf(stderr, "%s: %d note-ons left sounding (ons %u, offs %u, %u extra EVNT note-offs)\n",
                    e->name, leftover, cx.ons, cx.offs, cx.xoffs);
            rc = 1;
        } else if (cx.xoffs != 0) {
            /* offs may exceed ons: each real EVNT 0x8n is forwarded AND the queue still sends its own off */
            printf("%s: %u note-off(s) were real EVNT 0x8n events, not the sequencer's own\n",
                   e->name, cx.xoffs);
        }
    }
    xmi_seq_free(seq);
    xmi_free(&song);
    free(cx.v);
    free(buf);
    return rc;
}
