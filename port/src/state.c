#include "state.h"

#include <stdio.h>
#include <stdlib.h>

#include "le_image.h"

uint8_t *g_ds;
int g_original;

int state_init(const char *gamedir)
{
    char path[1024];
    FILE *f;
    long n;
    uint8_t *exe;
    le_object o;
    int rc;

    snprintf(path, sizeof path, "%s/AC_MAIN.EXE", gamedir);
    f = fopen(path, "rb");
    if (f == NULL) {
        fprintf(stderr, "state: cannot open %s\n", path);
        return -1;
    }
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    exe = malloc(n > 0 ? (size_t)n : 1);
    if (exe == NULL || fread(exe, 1, (size_t)n, f) != (size_t)n) {
        fclose(f);
        free(exe);
        fprintf(stderr, "state: cannot read %s\n", path);
        return -1;
    }
    fclose(f);
    rc = le_load_object(exe, (size_t)n, 2, &o);
    free(exe);
    if (rc != 0 || o.base != DS_BASE || o.size != DS_SIZE) {
        fprintf(stderr, "state: %s is not the AC_MAIN.EXE this port follows (error %d, data object 0x%X+0x%X)\n", path,
                rc, (unsigned)o.base, (unsigned)o.size);
        le_free_object(&o);
        return -1;
    }
    free(g_ds);
    g_ds = o.bytes;   /* keep the buffer (DS_SIZE + a page) */
    return 0;
}

int ds_valid(uint32_t a, uint32_t n)
{
    return a >= DS_BASE && n <= DS_SIZE && a - DS_BASE <= DS_SIZE - n;
}
