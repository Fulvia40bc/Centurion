/* le_test AC_MAIN.EXE [N]: load object N (default both) with le_load_object and print base, size and a checksum;
   with OUTDIR as a third argument, write objN.bin there to compare with tools/le_unbind.py. */
#include <stdio.h>
#include <stdlib.h>

#include "../src/le_image.h"

int main(int argc, char **argv)
{
    FILE *f;
    long n;
    uint8_t *exe;
    int k;

    if (argc < 2) {
        fprintf(stderr, "usage: le_test AC_MAIN.EXE [OUTDIR]\n");
        return 2;
    }
    f = fopen(argv[1], "rb");
    if (f == NULL) {
        fprintf(stderr, "le_test: cannot open %s\n", argv[1]);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    exe = malloc((size_t)n);
    if (exe == NULL || fread(exe, 1, (size_t)n, f) != (size_t)n)
        return 1;
    fclose(f);
    for (k = 1; k <= 2; k++) {
        le_object o;
        uint32_t i, sum = 0;
        int rc = le_load_object(exe, (size_t)n, k, &o);
        if (rc != 0) {
            fprintf(stderr, "le_test: object %d: error %d\n", k, rc);
            return 1;
        }
        for (i = 0; i < o.size; i++)
            sum = sum * 31u + o.bytes[i];
        printf("object %d: base 0x%08X size 0x%X hash %08X\n", k, (unsigned)o.base, (unsigned)o.size, (unsigned)sum);
        if (argc > 2) {
            char path[1024];
            snprintf(path, sizeof path, "%s/obj%d.bin", argv[2], k);
            f = fopen(path, "wb");
            if (f == NULL || fwrite(o.bytes, 1, o.size, f) != o.size)
                return 1;
            fclose(f);
        }
        le_free_object(&o);
    }
    free(exe);
    return 0;
}
