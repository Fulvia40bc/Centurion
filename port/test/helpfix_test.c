/* helpfix_test GAMEDIR: each help file with a fix, read straight from ACIV.CT3 (own catalogue reader, not the
   port's), run through help_fix; prints the fixed phrase in context and checks that only it changed */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu1.h"

static unsigned long u32(const unsigned char *p) { return p[0] | p[1] << 8 | (unsigned long)p[2] << 16 | (unsigned long)p[3] << 24; }

int main(int argc, char **argv)
{
    static const char *const files[] = { "BUYCIV.HLP", "TRADWANT.HLP", "TRADOFFR.HLP", "CIVILWAR.HLP", "SUPERSTN.HLP" };
    static const char *const after[] = { "Acquisition Color", "Salt?\" Simply", "button, only", "by an F (", "the effect to" };
    char path[512]; FILE *f; long len; unsigned char *ct; int fails = 0; size_t k;
    snprintf(path, sizeof path, "%s/ACIV.CT3", argc > 1 ? argv[1] : ".");
    f = fopen(path, "rb"); if (!f) { perror(path); return 2; }
    fseek(f, 0, SEEK_END); len = ftell(f); fseek(f, 0, SEEK_SET);
    ct = malloc((size_t)len); if (fread(ct, 1, (size_t)len, f) != (size_t)len) return 2; fclose(f);
    for (k = 0; k < 5; k++) {
        unsigned long base = 0; int found = 0;
        for (;;) {            /* chained 32-entry blocks of 24 bytes; a link entry (name 0xFF x12) points to the next */
            int i;
            for (i = 0; i < 33 && !found; i++) {
                const unsigned char *e = ct + base + 24 * (unsigned long)i;
                if (memcmp(e, "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 12) == 0) { base = u32(e + 12); i = -1; continue; }
                if (e[0] == 0) break;
                if (strncmp((const char *)e, files[k], 12) == 0) {
                    unsigned long off = u32(e + 12), n = u32(e + 16);
                    char *t = malloc(n + 1 + 8), *o = malloc(n + 1), lower[16]; size_t j, d = 0;
                    memcpy(t, ct + off, n); t[n] = 0; memcpy(o, t, n + 1);
                    for (j = 0; files[k][j]; j++) lower[j] = (char)(files[k][j] | 0x20); lower[j] = 0;
                    help_fix(lower, t);
                    { const char *p = strstr(t, after[k]);
                      printf("%-13s %s  ...%.40s...\n", files[k], p ? "fixed" : "NOT FIXED", p ? (p > t + 15 ? p - 15 : t) : "");
                      if (!p) fails++; }
                    for (j = 0; o[j] && t[j] == o[j]; j++) ;   /* first difference */
                    d = strlen(t) > strlen(o) ? strlen(t) - strlen(o) : strlen(o) - strlen(t);
                    if (strcmp(t + strlen(t) - 20, o + strlen(o) - 20) != 0) { printf("  tail changed\n"); fails++; }
                    printf("  first change at %zu, length %zu -> %zu (%zu)\n", j, strlen(o), strlen(t), d);
                    found = 1;
                }
            }
            if (found || i < 33) break;
        }
        if (!found) { printf("%s not in the catalogue\n", files[k]); fails++; }
    }
    printf("%s\n", fails ? "FAILED" : "all ok");
    return fails != 0;
}
