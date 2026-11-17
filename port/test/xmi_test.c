#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/catalog.h"
#include "../src/xmi.h"

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

static int ends_xmi(const char *name)
{
    size_t n = strlen(name);
    return n >= 4 && name[n - 4] == '.' &&
           (name[n - 3] | 0x20) == 'x' && (name[n - 2] | 0x20) == 'm' && (name[n - 1] | 0x20) == 'i';
}

static int check_song(const char *name, const xmi_song *s)
{
    uint32_t notes = 0, last_tick = 0;
    int ons[16] = {0}, offs[16] = {0};
    uint32_t i;
    int bad = 0;

    for (i = 0; i < s->count; i++) {
        const xmi_event *e = &s->ev[i];
        if ((e->status & 0xF0u) == 0x90u) {
            notes++;
            ons[e->status & 0x0Fu]++;
        }
        if ((e->status & 0xF0u) == 0x80u)
            offs[e->status & 0x0Fu]++;
        if (e->tick > last_tick)
            last_tick = e->tick;
    }
    for (i = 0; i < 16; i++)
        if (ons[i] != offs[i]) {
            fprintf(stderr, "%s: channel %u has %d note-ons but %d note-offs\n",
                    name, (unsigned)i, ons[i], offs[i]);
            bad = 1;
        }
    printf("%s %u %u %u %d\n", name, s->count, notes, last_tick, s->ntimb);
    return bad;
}

int main(int argc, char **argv)
{
    char path[4096];
    uint8_t *buf;
    size_t size;
    catalog c;
    int rc = 0;
    int count = 0;
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
        if (!ends_xmi(c.entries[j].name))
            continue;
        if (dump_name != NULL && cat_find(&c, dump_name) != &c.entries[j])
            continue;
        {
            const uint8_t *d = cat_data(&c, &c.entries[j]);
            xmi_song s;
            int prc;
            if (d == NULL) {
                fprintf(stderr, "%s: cat_data failed\n", c.entries[j].name);
                rc = 1;
                continue;
            }
            prc = xmi_parse(d, c.entries[j].length, &s);
            if (prc != 0) {
                fprintf(stderr, "%s: xmi_parse error %d\n", c.entries[j].name, prc);
                rc = 1;
                continue;
            }
            count++;
            if (dump_name != NULL) {
                uint32_t i;
                for (i = 0; i < s.count; i++)
                    printf("%u %02X %02X %02X\n", s.ev[i].tick,
                           (unsigned)s.ev[i].status, (unsigned)s.ev[i].d1, (unsigned)s.ev[i].d2);
            } else {
                rc |= check_song(c.entries[j].name, &s);
            }
            xmi_free(&s);
        }
    }
    free(buf);
    if (dump_name == NULL && count != 12) {
        fprintf(stderr, "parsed %d XMI files, expected 12\n", count);
        rc = 1;
    }
    return rc;
}
