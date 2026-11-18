/* font_test FONTDIR OUT.png
   Draws all 96 characters of BLOCK05.FGF, COGNAC10.FGF and COGNAC19.FGF (extracted from ACIV.CT1, e.g. by
   tools/ct_list.py) and sample lines with a 0x7F colour change and a 0x7F '_' underline into a 640x480 indexed PNG,
   one baseline guide row per line (colour 8). Prints each font's header values. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/fgfont.h"
#include "../src/png_write.h"

enum { W = 640, H = 480 };
static uint8_t fb[W * H];
static uint8_t pal[256][3];

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

static void guide(int y)
{
    int x;
    for (x = 0; x < W; x++)
        if (fb[y * W + x] == 0)
            fb[y * W + x] = 8;
}

int main(int argc, char **argv)
{
    static const char *names[3] = { "BLOCK05.FGF", "COGNAC10.FGF", "COGNAC19.FGF" };
    static const char sample[] = "Advanced Civilization: \x7F\x0C" "Crete\x7F\x0F builds a city in \x7F_Knossos\x7F_.";
    uint8_t *data[3];
    fg_font f[3];
    int i, y = 4;

    if (argc != 3) {
        fprintf(stderr, "usage: font_test FONTDIR OUT.png\n");
        return 2;
    }
    pal[8][0] = pal[8][1] = pal[8][2] = 0x40;                        /* baseline guides */
    pal[12][0] = 0xFF; pal[12][1] = 0x55; pal[12][2] = 0x55;          /* light red */
    pal[14][0] = 0xFF; pal[14][1] = 0xFF; pal[14][2] = 0x55;          /* yellow */
    pal[15][0] = pal[15][1] = pal[15][2] = 0xFF;                      /* white */
    for (i = 0; i < 3; i++) {
        char path[1024], line[97];
        size_t size;
        int c, x;
        uint8_t color;

        snprintf(path, sizeof path, "%s/%s", argv[1], names[i]);
        data[i] = load_file(path, &size);
        if (data[i] == NULL || fgf_define(data[i], size, &f[i]) != 0) {
            fprintf(stderr, "font_test: cannot define %s\n", path);
            return 1;
        }
        printf("%s: %u bytes per row, height %u, baseline %d, cell %d bytes\n", names[i], f[i].bpr, f[i].height,
               f[i].baseline, f[i].cell);
        for (c = 0; c < 95; c++)
            line[c] = (char)(32 + c);   /* 32..126; 127 is the escape */
        y += f[i].height - f[i].baseline + 2;
        x = 4; color = 15;
        fgf_print(fb, W, H, &f[i], &x, y, line, 48, 2, &color);
        guide(y);
        y += f[i].height + 2;
        x = 4; color = 15;
        fgf_print(fb, W, H, &f[i], &x, y, line + 48, 47, 2, &color);
        guide(y);
        y += f[i].height + 2;
        x = 4; color = 14;
        fgf_print(fb, W, H, &f[i], &x, y, sample, (int)strlen(sample), 0, &color);
        guide(y);
        printf("  sample line width %d (computed %d), tallest glyph %d rows\n", x - 4,
               fgf_width(&f[i], sample, (int)strlen(sample), 0), fgf_string_height(&f[i], sample, (int)strlen(sample)));
        y += f[i].baseline + 6;
    }
    if (png_write_indexed(argv[2], fb, W, H, (const uint8_t (*)[3])pal) != 0) {
        fprintf(stderr, "font_test: cannot write %s\n", argv[2]);
        return 1;
    }
    for (i = 0; i < 3; i++)
        free(data[i]);
    return 0;
}
