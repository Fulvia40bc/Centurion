#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/catalog.h"

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

static void to_lower(char *s)
{
    for (; *s != '\0'; s++)
        if (*s >= 'A' && *s <= 'Z')
            *s = (char)(*s - 'A' + 'a');
}

static int check_bad_buffers(void)
{
    uint8_t empty[24] = {0};
    uint8_t bad[24] = {0};
    catalog c;

    if (cat_open(&c, NULL, 0) >= 0)
        return -1;
    if (cat_open(&c, empty, 0) >= 0)
        return -1;
    if (cat_open(&c, empty, 10) >= 0)
        return -1;
    bad[12] = 100;
    bad[16] = 10;
    if (cat_open(&c, bad, sizeof bad) >= 0)
        return -1;
    empty[12] = 48;
    if (cat_open(&c, empty, sizeof empty) >= 0)
        return -1;
    return 0;
}

int main(int argc, char **argv)
{
    static const char *names[5] = { "ACIV.CT1", "ACIV.CT2", "ACIV.CT3", "ACIV.CT4", "ACIV.CT5" };
    int i;
    int rc = 0;

    if (argc != 2) {
        fprintf(stderr, "usage: %s GAMEDIR\n", argv[0]);
        return 2;
    }
    if (check_bad_buffers() != 0) {
        fprintf(stderr, "cat_open did not reject the malformed directories\n");
        return 1;
    }
    for (i = 0; i < 5; i++) {
        char path[4096];
        uint8_t *buf;
        size_t size;
        catalog c;
        int j;

        snprintf(path, sizeof path, "%s/%s", argv[1], names[i]);
        buf = load_file(path, &size);
        if (buf == NULL) {
            fprintf(stderr, "%s: cannot read\n", path);
            return 2;
        }
        if (cat_open(&c, buf, size) != 0) {
            fprintf(stderr, "%s: cat_open failed\n", names[i]);
            free(buf);
            return 1;
        }
        for (j = 0; j < c.count; j++) {
            const cat_entry *e = &c.entries[j];
            char lname[13];
            const cat_entry *found;
            const uint8_t *data;

            printf("%s %s %" PRIu32 " %" PRIu32 " %" PRIu32 "\n",
                   names[i], e->name, e->offset, e->length, e->kind);
            memcpy(lname, e->name, sizeof lname);
            to_lower(lname);
            found = cat_find(&c, lname);
            if (found != e) {
                fprintf(stderr, "%s: cat_find(\"%s\") did not return the entry\n", names[i], lname);
                rc = 1;
            }
            data = cat_data(&c, e);
            if (data != buf + e->offset) {
                fprintf(stderr, "%s: cat_data(%s) out of range\n", names[i], e->name);
                rc = 1;
            }
        }
        if (cat_find(&c, "no_such.fil") != NULL) {
            fprintf(stderr, "%s: cat_find found a name that is absent\n", names[i]);
            rc = 1;
        }
        if (cat_data(&c, NULL) != NULL) {
            fprintf(stderr, "%s: cat_data(NULL) is not NULL\n", names[i]);
            rc = 1;
        }
        free(buf);
    }
    return rc;
}
