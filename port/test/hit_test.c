/* hit_test [GAMEDIR]: a self-check of the map hit test f3D2A4 (mapdata.c, 0x3D2A4) that needs no game run.

   It loads the data object from GAMEDIR/AC_MAIN.EXE as the port does (state_init: le_image.c), changes into GAMEDIR
   (the original runs there; the port's main does the same with plat_chdir) and runs the port's own loaders:
   f12394 (terrflag.dat: the in-play bytes per map variant), f3EA7C (mapdataz.dat into a load_heap block, record handles
   at [0xFF64C + 4t]) and, per map variant, f12B00 (0xEE16C). Files are only read. GAMEDIR defaults to the run copy
   C:\work\advciv_run.

   Then, for every map variant 0..13 and one extra set with every territory in play, and for nine scroll positions
   (column 0, 7, 14 x row 0, 3, 7), it calls f3D2A4 for every screen point x 0..639, y 0..0x145 (the area f78BA4 hands
   to it) with int8 [0x108804] = 2, so the territory comes back without the alias mapping. Independently of mapdata.c it
   reads MAPDATAZ.DAT itself (uint32 size, then 160 records: int16 n, int16 ytop, int16 ybottom, n pairs int16 x1, x2;
   a pair with x1 >= 0 starts the next row, the rows starting at ytop - 1; x1 < 0 continues the row from -x1), paints
   every in-play territory into a 1716 x 568 raster (the map picture) in territory order, and checks for each point:
     - the expected answer: the lowest in-play territory whose spans contain the map point (x + 78 col - 8,
       y + 71 (row / 2) + 35 (row % 2) - 6), or -100 if none;
     - that a returned territory's shape contains the point (read from the file records directly).
   A second pass with [0x108804] = 1 (land mode) and phase 0 checks the alias mapping (157/158/159 -> 57/13/135).
   It prints a coarse grid of the answers at one scroll, the counts, and exits 0 when every point agrees.

   Link: hit_test.c mapdata.c load.c state.c le_image.c. The few routines those files call that this check never
   reaches (f3D52C, f787F8 of map.c; f3A780, f44CAC of the UI) are stubs below: f3A780 only prints its message (the
   loader's own guard then quits through f44CAC), the others stop with a message. */

#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/state.h"
#include "../src/game.h"
#include "../src/map.h"
#include "../src/mapdata.h"

#define PIC_W 1716
#define PIC_H 568
#define NONE  0xFFFF

/* ---- stubs for routines outside this check ------------------------------------------------------------------- */

void f3A780(const char *msg)
{
    fprintf(stderr, "hit_test: %s", msg);
}

_Noreturn void f44CAC(void)
{
    fprintf(stderr, "hit_test: quit: %s\n", DSTR(0xF5500u));
    exit(2);
}

void f3D52C(int dx, int dy)
{
    fprintf(stderr, "hit_test: f3D52C(%d, %d) reached\n", dx, dy);
    exit(2);
}

void f787F8(int t)
{
    fprintf(stderr, "hit_test: f787F8(%d) reached\n", t);
    exit(2);
}

/* ---- the independent reading of MAPDATAZ.DAT ---------------------------------------------------------------- */

typedef struct {
    int row, a, b;
} span;

static struct {
    int n, ytop, ybot, lastrow;
    span *s;
} g_rec[160];

static uint16_t g_first[PIC_H][PIC_W];   /* lowest in-play territory painted at a pixel, NONE if none */
static uint8_t g_count[PIC_H][PIC_W];    /* how many in-play territories cover it */

static int rd16(const uint8_t *buf, long len, long *pos, int *v)
{
    if (*pos + 2 > len)
        return -1;
    *v = (int16_t)(uint16_t)(buf[*pos] | buf[*pos + 1] << 8);
    *pos += 2;
    return 0;
}

static int read_mapdataz(const char *path)
{
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    long len, pos = 4;
    int t, i;

    if (f == NULL) {
        fprintf(stderr, "hit_test: cannot open %s\n", path);
        return -1;
    }
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = malloc((size_t)len);
    if (buf == NULL || fread(buf, 1, (size_t)len, f) != (size_t)len) {
        fclose(f);
        return -1;
    }
    fclose(f);
    for (t = 0; t < 160; t++) {
        int n, ytop, ybot, row;
        if (rd16(buf, len, &pos, &n) || rd16(buf, len, &pos, &ytop) || rd16(buf, len, &pos, &ybot) || n < 0)
            return -1;
        g_rec[t].n = n;
        g_rec[t].ytop = ytop;
        g_rec[t].ybot = ybot;
        g_rec[t].s = calloc((size_t)(n > 0 ? n : 1), sizeof(span));
        if (g_rec[t].s == NULL)
            return -1;
        row = ytop - 1;
        for (i = 0; i < n; i++) {
            int a, b;
            if (rd16(buf, len, &pos, &a) || rd16(buf, len, &pos, &b))
                return -1;
            if (a >= 0)
                row++;
            else
                a = -a;
            g_rec[t].s[i].row = row;
            g_rec[t].s[i].a = a;
            g_rec[t].s[i].b = b;
        }
        g_rec[t].lastrow = row;
    }
    printf("MAPDATAZ.DAT: %ld bytes, size field %u, records end at %ld\n", len,
           (unsigned)(buf[0] | buf[1] << 8 | buf[2] << 16 | (uint32_t)buf[3] << 24), pos);
    free(buf);
    return 0;
}

/* does territory t's shape (from the file) contain map point (mx, my)? */
static int in_shape(int t, int mx, int my)
{
    int i;

    for (i = 0; i < g_rec[t].n; i++)
        if (g_rec[t].s[i].row == my && g_rec[t].s[i].a <= mx && mx <= g_rec[t].s[i].b)
            return 1;
    return 0;
}

static void paint(const uint8_t *inplay)
{
    int t, i, x;

    for (i = 0; i < PIC_H; i++)
        for (x = 0; x < PIC_W; x++) {
            g_first[i][x] = NONE;
            g_count[i][x] = 0;
        }
    for (t = 0; t < 160; t++) {
        if (!inplay[t])
            continue;
        for (i = 0; i < g_rec[t].n; i++) {
            const span *s = &g_rec[t].s[i];
            if (s->row < 0 || s->row >= PIC_H)
                continue;
            for (x = s->a < 0 ? 0 : s->a; x <= s->b && x < PIC_W; x++) {
                if (g_first[s->row][x] == NONE)
                    g_first[s->row][x] = (uint16_t)t;
                if (g_count[s->row][x] < 255)
                    g_count[s->row][x]++;
            }
        }
    }
}

/* ---- the check ----------------------------------------------------------------------------------------------- */

static long g_points, g_agree, g_hits, g_hit_in_shape, g_misses, g_miss_empty, g_overlap;

static void check_scroll(int col, int row)
{
    int sx, sy;
    int px = col * 78;                                  /* own arithmetic, not f449B8 / f449E0 */
    int py = (row / 2) * 71 + (row % 2) * 35;

    setD32(0xF4884u, (uint32_t)col);
    setD32(0xF4888u, (uint32_t)row);
    for (sy = 0; sy <= 0x145; sy++)
        for (sx = 0; sx < 640; sx++) {
            int r = f3D2A4(sx, sy);
            int mx = sx + px - 8, my = sy + py - 6;
            int expect = -100;
            if (mx >= 0 && mx < PIC_W && my >= 0 && my < PIC_H && g_first[my][mx] != NONE) {
                expect = g_first[my][mx];
                if (g_count[my][mx] > 1)
                    g_overlap++;
            }
            g_points++;
            if (r == expect)
                g_agree++;
            else if (g_points - g_agree <= 10)
                printf("  DISAGREE scroll (%d,%d) screen (%d,%d) map (%d,%d): f3D2A4 %d, expected %d\n", col, row, sx,
                       sy, mx, my, r, expect);
            if (r >= 0) {
                g_hits++;
                if (r < 160 && in_shape(r, mx, my))
                    g_hit_in_shape++;
            } else {
                g_misses++;
                if (expect == -100)
                    g_miss_empty++;
            }
        }
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:/work/advciv_run";
    static const int cols[3] = { 0, 7, 14 }, rows[3] = { 0, 3, 7 };
    uint8_t inplay[160];
    int v, t, i, j, ok;
    long alias_points = 0, alias_agree = 0;
    long same_ybot = 0;

    if (state_init(gamedir) != 0)
        return 1;
    if (_chdir(gamedir) != 0) {
        fprintf(stderr, "hit_test: cannot change into %s\n", gamedir);
        return 1;
    }
    f12394();                                            /* terrflag.dat -> 0xE1638 (in-play bytes +0x15..) */
    f3EA7C();                                            /* mapdataz.dat -> heap block, handles at 0xFF64C */
    if (read_mapdataz("MAPDATAZ.DAT") != 0) {
        fprintf(stderr, "hit_test: cannot read MAPDATAZ.DAT\n");
        return 1;
    }
    for (t = 0; t < 160; t++)
        if (g_rec[t].ybot == g_rec[t].lastrow)
            same_ybot++;
    printf("records whose third word equals their last span row: %ld of 160\n", same_ybot);
    for (j = 0; j < 3; j++) {
        static const int alias[3][2] = { { 157, 57 }, { 158, 13 }, { 159, 135 } };
        int a = alias[j][0], b = alias[j][1], same = g_rec[a].n == g_rec[b].n && g_rec[a].ytop == g_rec[b].ytop;
        for (i = 0; same && i < g_rec[a].n; i++)
            same = g_rec[a].s[i].row == g_rec[b].s[i].row && g_rec[a].s[i].a == g_rec[b].s[i].a &&
                   g_rec[a].s[i].b == g_rec[b].s[i].b;
        printf("alias record %d (%d spans, rows %d..%d) vs %d (%d spans, rows %d..%d): %s\n", a, g_rec[a].n,
               g_rec[a].ytop, g_rec[a].ybot, b, g_rec[b].n, g_rec[b].ytop, g_rec[b].ybot,
               same ? "identical spans" : "different spans");
    }

    setD8(0x108804u, 2);                                 /* sea mode: f3D2A4 returns t without the alias mapping */
    setD8(0xF4102u, 0);
    for (v = 0; v <= 14; v++) {
        long p0 = g_points, a0 = g_agree;
        if (v < 14) {
            setD16(GAME_MAP_VARIANT, (uint16_t)v);
            f12B00();                                    /* 0xEE16C = byte +0x15+v of every territory */
        } else {
            memset(DP(GAME_INPLAY), 1, 160);             /* every territory in play */
        }
        for (t = 0; t < 160; t++)
            inplay[t] = D8(GAME_INPLAY + (uint32_t)t) != 0;
        paint(inplay);
        for (i = 0; i < 3; i++)
            for (j = 0; j < 3; j++)
                check_scroll(cols[i], rows[j]);
        {
            int n = 0;
            for (t = 0; t < 160; t++)
                n += inplay[t];
            printf("%-10s %3d territories in play: %ld points, %ld agree\n", v < 14 ? "variant" : "all", n,
                   g_points - p0, g_agree - a0);
        }
        if (v == 1) {                                    /* the grid printout: variant 1 (3 players), scroll (0, 0) */
            int sx, sy;
            setD32(0xF4884u, 0);
            setD32(0xF4888u, 0);
            printf("grid, variant 1, scroll (0,0): f3D2A4(x, y), '.' = -100\n     ");
            for (sx = 8; sx <= 0x277; sx += 32)
                printf("%4d", sx);
            printf("\n");
            for (sy = 6; sy <= 0x144; sy += 16) {
                printf("%4d ", sy);
                for (sx = 8; sx <= 0x277; sx += 32) {
                    int r = f3D2A4(sx, sy);
                    if (r < 0)
                        printf("   .");
                    else
                        printf("%4d", r);
                }
                printf("\n");
            }
        }
    }

    /* the alias mapping: land mode, phase 0, every territory in play */
    setD8(0x108804u, 1);
    setD8(0xF4102u, 0);
    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++) {
            int sx, sy, px = cols[i] * 78, py = (rows[j] / 2) * 71 + (rows[j] % 2) * 35;
            setD32(0xF4884u, (uint32_t)cols[i]);
            setD32(0xF4888u, (uint32_t)rows[j]);
            for (sy = 0; sy <= 0x145; sy++)
                for (sx = 0; sx < 640; sx++) {
                    int r = f3D2A4(sx, sy), mx = sx + px - 8, my = sy + py - 6, expect = -100;
                    if (mx >= 0 && mx < PIC_W && my >= 0 && my < PIC_H && g_first[my][mx] != NONE) {
                        expect = g_first[my][mx];
                        expect = expect == 157 ? 57 : expect == 158 ? 13 : expect == 159 ? 135 : expect;
                    }
                    alias_points++;
                    alias_agree += r == expect;
                }
        }

    printf("\nraw pass: %ld points checked, %ld agree with the file model (%ld disagree)\n", g_points, g_agree,
           g_points - g_agree);
    printf("  %ld hits, %ld of them inside the returned territory's shape; %ld misses (-100), %ld of them on no shape\n",
           g_hits, g_hit_in_shape, g_misses, g_miss_empty);
    printf("  %ld points lie in more than one in-play shape (the lowest territory wins)\n", g_overlap);
    printf("alias pass: %ld points checked, %ld agree\n", alias_points, alias_agree);
    ok = g_agree == g_points && g_hit_in_shape == g_hits && alias_agree == alias_points;
    printf("%s\n", ok ? "OK" : "FAILED");
    return ok ? 0 : 1;
}
