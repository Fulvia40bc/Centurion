/* showflic_test [GAMEDIR]: fg_flicsize / fg_showflic (fg.c, TASK 17) against synthetic FLI/FLC files built here and,
   when GAMEDIR/ACIV.CT4 exists (default C:\work\advciv_play), against every *.FLC entry of it, frame by frame, with
   the independent model flc.c.

   Part a (always): one synthetic file per chunk type (COLOR_256, COLOR_64, BRUN, COPY, LC, SS2 with a skip-lines word
   and a last-byte word, BLACK), a prefix frame, a 0-chunk frame, an unknown chunk type 0x20, an unknown frame type, a
   bad magic (return 2), a missing file (return 1), a chunk that overruns its size, count 0 stopped by Esc after 5
   frames (fg_test_key in the input hook), count 2 (two passes, frames counted), flags 0x10 (palette untouched), and
   flags & 2 at (630, 470) with W = H = 20 (rows wrap past x 639 into the next scanline; rows y >= 480 not drawn).
   The expected screen and palette are written out by hand below, not by calling the code under test.

   Part b (needs GAMEDIR): for every kind-6 entry of ACIV.CT4: fg_imagebuf + fg_flicsize + fg_move as the game does
   (0x79B64: x = (0x270-w)/2+8, y = (0x13F-h)/2+6), fg_showflic("", 1, 7) headless with fg_set_input_hook capturing
   the W x H window and fg_dac() after every frame; then flc_open/flc_next over the same bytes and compare frame k's
   window and palette (flc.c's 8-bit palette >> 2) with the capture. All frames of all files must match. Prints one
   line per file: name, W x H, frames, delay ticks, chunk types with counts, frames compared, mismatches.

   Build: clang -std=c11 -Wall -Wextra -Werror -pedantic port/test/showflic_test.c port/src/fg.c port/src/gif.c port/src/catalog.c port/src/fgfont.c port/src/palette.c port/src/png_write.c port/src/flc.c port/src/plat_sdl3.c -Iport/src -I<sdl include> <sdl lib> -o showflic_test.exe
   (in the sandbox: gcc ... showflic_test.c fg.c gif.c catalog.c fgfont.c palette.c png_write.c flc.c with the plat_* stubs below)
   Run:   showflic_test [GAMEDIR] */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "fg.h"
#include "flc.h"
#include "catalog.h"

/* ---- stubs for what fg.c links against (as in junk_test.c; no window is opened) ---------------------------------- */

uint8_t *g_ds;          /* fg.c reads a few data-object fields; the test never reaches them */
int g_original;

static uint64_t s_ticks;
uint64_t plat_ticks_ms(void) { s_ticks += 16; return s_ticks; }
void plat_sleep_ms(uint32_t ms) { s_ticks += ms; }
int plat_pointer(int on) { return on; }
void plat_present(const uint8_t *fb, const uint8_t pal[256][3]) { (void)fb; (void)pal; }
int plat_poll(plat_event *ev) { (void)ev; return 0; }
void plat_mouse(int *x, int *y, int *buttons) { *x = 0; *y = 0; *buttons = 0; }
void plat_shutdown(void) {}

/* ---- checks ----------------------------------------------------------------------------------------------------- */

static int fails;

#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL %s (line %d)\n", msg, __LINE__); fails++; } } while (0)

/* ---- a tiny FLI/FLC builder ------------------------------------------------------------------------------------- */

static uint8_t fb[1 << 20];
static size_t flen;

static void w16(size_t o, uint32_t v) { fb[o] = (uint8_t)v; fb[o + 1] = (uint8_t)(v >> 8); }
static void w32(size_t o, uint32_t v) { w16(o, v); w16(o + 2, v >> 16); }

static void f_hdr(uint32_t magic, uint32_t frames, uint32_t w, uint32_t h, uint32_t speed)
{
    memset(fb, 0, 128);
    flen = 128;
    w16(4, magic);
    w16(6, frames);
    w16(8, w);
    w16(10, h);
    if (magic == 0xAF12u)
        w32(16, speed);
    else
        w16(16, speed);
}

/* a 0xF1FA frame header (size, type, chunk count, 8 reserved); returns its offset */
static size_t f_frame(void)
{
    size_t at = flen;

    w32(at, 16);
    w16(at + 4, 0xF1FA);
    w16(at + 6, 0);
    memset(fb + at + 8, 0, 8);
    flen += 16;
    return at;
}

static void f_chunk(size_t fat, uint32_t ctype, const uint8_t *body, size_t blen)
{
    size_t at = flen;
    uint32_t nc = (uint32_t)fb[fat + 6] | ((uint32_t)fb[fat + 7] << 8);

    w32(at, (uint32_t)(6 + blen));
    w16(at + 4, ctype);
    if (blen > 0)
        memcpy(fb + at + 6, body, blen);
    flen = at + 6 + blen;
    w16(fat + 6, nc + 1);
    w32(fat, (uint32_t)(flen - fat));
}

static void f_prefix(uint32_t skip)
{
    size_t at = flen;

    w32(at, 6 + skip);
    w16(at + 4, 0xF100);
    flen += 6 + skip;                         /* the skipped bytes (zeros) */
}

/* ---- part a ----------------------------------------------------------------------------------------------------- */

static int play_mem(int count, int flags)
{
    fg_imagebuf(fb, (uint32_t)flen);
    return fg_showflic("", count, flags);
}

static void clear_screen(uint8_t v)
{
    memset(fg_screen(), v, 640 * 480);
}

static int pix(int x, int y) { return fg_screen()[(size_t)y * 640 + x]; }

static void test_a(void)
{
    int w, h, rc, i;

    /* fg_flicsize */
    f_hdr(0xAF12, 1, 320, 200, 1000);
    CHECK(fg_flicsize(fb, &w, &h) == 0 && w == 320 && h == 200, "flicsize AF12");
    f_hdr(0xAF11, 1, 640, 480, 70);
    CHECK(fg_flicsize(fb, &w, &h) == 0 && w == 640 && h == 480, "flicsize AF11");
    memset(fb, 0, 128);
    CHECK(fg_flicsize(fb, &w, &h) == 0 && w == -1 && h == -1, "flicsize bad magic");

    /* missing file -> 1 */
    fg_init();
    fg_set_headless(1);
    rc = fg_showflic("no_such_file.flc", 1, 0);
    CHECK(rc == 1, "missing file returns 1");

    /* bad magic in memory -> 2 */
    fg_init();
    fg_set_headless(1);
    memset(fb, 0, 256);
    flen = 256;
    rc = play_mem(1, 4);
    CHECK(rc == 2, "bad magic returns 2");

    /* COLOR_256 (type 4): packets=1, skip=2, count=3, triples; dac entries from index 2, bytes >> 2 */
    fg_init();
    fg_set_headless(1);
    f_hdr(0xAF12, 1, 4, 4, 100);
    {
        size_t fat = f_frame();
        uint8_t body[2 + 2 + 9];

        body[0] = 1; body[1] = 0;             /* 1 packet */
        body[2] = 2;                          /* skip 2 */
        body[3] = 3;                          /* count 3 */
        body[4] = 0xFF; body[5] = 0x00; body[6] = 0x00;
        body[7] = 0x80; body[8] = 0x40; body[9] = 0x20;
        body[10] = 0x0C; body[11] = 0x08; body[12] = 0x04;
        f_chunk(fat, 4, body, sizeof body);
    }
    rc = play_mem(1, 5);                      /* memory, no delay */
    CHECK(rc == 0, "COLOR_256 rc");
    {
        const uint8_t(*dac)[3] = fg_dac();

        CHECK(dac[2][0] == 0x3F && dac[2][1] == 0 && dac[2][2] == 0, "COLOR_256 dac[2] >>2");
        CHECK(dac[3][0] == 0x20 && dac[3][1] == 0x10 && dac[3][2] == 0x08, "COLOR_256 dac[3]");
        CHECK(dac[4][0] == 0x03 && dac[4][1] == 0x02 && dac[4][2] == 0x01, "COLOR_256 dac[4]");
        CHECK(dac[0][0] == 0 && dac[5][0] == 0, "COLOR_256 other entries untouched");
    }

    /* COLOR_64 (type 11): values & 0x3F */
    fg_init();
    fg_set_headless(1);
    f_hdr(0xAF12, 1, 4, 4, 100);
    {
        size_t fat = f_frame();
        uint8_t body[2 + 2 + 9];

        body[0] = 1; body[1] = 0;
        body[2] = 2;
        body[3] = 3;
        body[4] = 0xFF; body[5] = 0x00; body[6] = 0x00;
        body[7] = 0x80; body[8] = 0x40; body[9] = 0x20;
        body[10] = 0x0C; body[11] = 0x08; body[12] = 0x04;
        f_chunk(fat, 11, body, sizeof body);
    }
    rc = play_mem(1, 5);
    CHECK(rc == 0, "COLOR_64 rc");
    {
        const uint8_t(*dac)[3] = fg_dac();

        CHECK(dac[2][0] == 0x3F, "COLOR_64 dac[2] &0x3F");
        CHECK(dac[3][0] == 0 && dac[3][1] == 0 && dac[3][2] == 0x20, "COLOR_64 dac[3]");
        CHECK(dac[4][0] == 0x0C && dac[4][1] == 0x08 && dac[4][2] == 0x04, "COLOR_64 dac[4]");
    }

    /* flags 0x10: the palette chunk is skipped entirely */
    fg_init();
    fg_set_headless(1);
    f_hdr(0xAF12, 1, 4, 4, 100);
    {
        size_t fat = f_frame();
        uint8_t body[2 + 2 + 9];

        body[0] = 1; body[1] = 0;
        body[2] = 0;
        body[3] = 3;
        body[4] = 0xFF; body[5] = 0xFF; body[6] = 0xFF;
        body[7] = 0xFF; body[8] = 0xFF; body[9] = 0xFF;
        body[10] = 0xFF; body[11] = 0xFF; body[12] = 0xFF;
        f_chunk(fat, 4, body, sizeof body);
    }
    rc = play_mem(1, 4 | 0x10);
    CHECK(rc == 0, "flags 0x10 rc");
    CHECK(fg_dac()[0][0] == 0 && fg_dac()[1][1] == 0 && fg_dac()[2][2] == 0, "flags 0x10 palette untouched");

    /* BRUN (15): W=8 H=2; row0: packets 2: (3,0xAA), (-2, 0x01 0x02); row1: packets 1: (-8, 0xB0..0xB7) */
    fg_init();
    fg_set_headless(1);
    clear_screen(0);
    f_hdr(0xAF12, 1, 8, 2, 100);
    {
        size_t fat = f_frame();
        uint8_t body[1 + 2 + 3 + 1 + 1 + 8];
        size_t n = 0;

        body[n++] = 2;                        /* row 0: 2 packets */
        body[n++] = 3; body[n++] = 0xAA;
        body[n++] = (uint8_t)-2; body[n++] = 0x01; body[n++] = 0x02;
        body[n++] = 1;                        /* row 1: 1 packet */
        body[n++] = (uint8_t)-8;
        for (i = 0; i < 8; i++)
            body[n++] = (uint8_t)(0xB0 + i);
        f_chunk(fat, 15, body, n);
    }
    rc = play_mem(1, 5);
    CHECK(rc == 0, "BRUN rc");
    {
        static const uint8_t want0[8] = { 0xAA, 0xAA, 0xAA, 0x01, 0x02, 0, 0, 0 };
        static const uint8_t want1[8] = { 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7 };
        CHECK(memcmp(fg_screen(), want0, 8) == 0, "BRUN row 0");
        CHECK(memcmp(fg_screen() + 640, want1, 8) == 0, "BRUN row 1");
    }

    /* COPY (16): W=6 (rows padded to 8), H=2 */
    fg_init();
    fg_set_headless(1);
    clear_screen(0);
    f_hdr(0xAF12, 1, 6, 2, 100);
    {
        size_t fat = f_frame();
        uint8_t body[16];

        for (i = 0; i < 6; i++)
            body[i] = (uint8_t)(1 + i);
        body[6] = 0xEE; body[7] = 0xEE;       /* padding */
        for (i = 0; i < 6; i++)
            body[8 + i] = (uint8_t)(7 + i);
        body[14] = 0xEE; body[15] = 0xEE;
        f_chunk(fat, 16, body, sizeof body);
    }
    rc = play_mem(1, 5);
    CHECK(rc == 0, "COPY rc");
    {
        static const uint8_t want0[6] = { 1, 2, 3, 4, 5, 6 };
        static const uint8_t want1[6] = { 7, 8, 9, 10, 11, 12 };
        CHECK(memcmp(fg_screen(), want0, 6) == 0 && memcmp(fg_screen() + 640, want1, 6) == 0, "COPY rows (padding skipped)");
    }

    /* LC (12): W=8 H=4; start line 1, 1 line: packets: (skip 1, c 2, lit 11 22), (skip 0, c -3, v 99) */
    fg_init();
    fg_set_headless(1);
    clear_screen(0);
    f_hdr(0xAF12, 1, 8, 4, 100);
    {
        size_t fat = f_frame();
        uint8_t body[2 + 2 + 1 + 4 + 3];
        size_t n = 0;

        body[n++] = 1; body[n++] = 0;         /* start line 1 */
        body[n++] = 1; body[n++] = 0;         /* 1 line */
        body[n++] = 2;                        /* 2 packets */
        body[n++] = 1; body[n++] = 2; body[n++] = 0x11; body[n++] = 0x22;
        body[n++] = 0; body[n++] = (uint8_t)-3; body[n++] = 0x99;
        f_chunk(fat, 12, body, n);
    }
    rc = play_mem(1, 5);
    CHECK(rc == 0, "LC rc");
    {
        static const uint8_t want[8] = { 0, 0x11, 0x22, 0x99, 0x99, 0x99, 0, 0 };
        CHECK(memcmp(fg_screen() + 640, want, 8) == 0, "LC row 1");
    }

    /* SS2 (7): W=10 H=6; lines=1; word 0xFFFB (skip 5 lines -> y=5), word 0x8077 (last byte 0x77 at xr=9, y=5),
       then packet line (count 2): (skip 1, c -2, pair 11 22), (skip 2, c 1, lit AB CD) */
    fg_init();
    fg_set_headless(1);
    clear_screen(0);
    f_hdr(0xAF12, 1, 10, 6, 100);
    {
        size_t fat = f_frame();
        uint8_t body[2 + 2 + 2 + 2 + 2 + 2 + 2 + 2];
        size_t n = 0;

        body[n++] = 1; body[n++] = 0;         /* 1 line */
        body[n++] = 0xFB; body[n++] = 0xFF;   /* 0xFFFB: skip 5 lines */
        body[n++] = 0x77; body[n++] = 0x80;   /* 0x8077: last byte 0x77 */
        body[n++] = 2; body[n++] = 0;         /* 2 packets */
        body[n++] = 1; body[n++] = (uint8_t)-2; body[n++] = 0x11; body[n++] = 0x22;
        body[n++] = 2; body[n++] = 1; body[n++] = 0xAB; body[n++] = 0xCD;
        f_chunk(fat, 7, body, n);
    }
    rc = play_mem(1, 5);
    CHECK(rc == 0, "SS2 rc");
    {
        static const uint8_t want[10] = { 0, 0x11, 0x22, 0x11, 0x22, 0, 0, 0xAB, 0xCD, 0x77 };
        CHECK(memcmp(fg_screen() + 5 * 640, want, 10) == 0, "SS2 row 5 (skip, last byte, packets)");
    }

    /* BLACK (13): W=4 H=2 over a 0x55 screen */
    fg_init();
    fg_set_headless(1);
    clear_screen(0x55);
    f_hdr(0xAF12, 1, 4, 2, 100);
    {
        size_t fat = f_frame();
        f_chunk(fat, 13, NULL, 0);
    }
    rc = play_mem(1, 5);
    CHECK(rc == 0, "BLACK rc");
    CHECK(pix(0, 0) == 0 && pix(3, 1) == 0 && pix(0, 2) == 0x55 && pix(3, 2) == 0x55, "BLACK rows 0..1 zeroed");

    /* prefix frame + 0-chunk frame + unknown chunk type 0x20: all fine, rc 0, screen untouched */
    fg_init();
    fg_set_headless(1);
    clear_screen(0x21);
    f_hdr(0xAF12, 3, 4, 4, 100);
    f_prefix(10);
    {
        size_t fat = f_frame();               /* 0 chunks */
        uint8_t junk[4] = { 1, 2, 3, 4 };

        (void)fat;
        fat = f_frame();
        f_chunk(fat, 0x20, junk, sizeof junk); /* unknown chunk: skipped */
    }
    rc = play_mem(1, 5);
    CHECK(rc == 0, "prefix/0-chunk/unknown-chunk rc");
    CHECK(pix(0, 0) == 0x21 && pix(3, 3) == 0x21, "prefix/0-chunk/unknown-chunk screen untouched");

    /* unknown frame type: stop, return 0 */
    fg_init();
    fg_set_headless(1);
    f_hdr(0xAF12, 2, 4, 4, 100);
    {
        size_t at = flen;

        w32(at, 16);
        w16(at + 4, 0xF1FB);                  /* not F1FA/F100 */
        flen += 16;
    }
    rc = play_mem(1, 5);
    CHECK(rc == 0, "unknown frame type returns 0");

    /* chunk overruns its size: COPY with csize 6 (empty body), W=4 H=1 -> crem < 0 -> end of pass, count 1 -> stop */
    fg_init();
    fg_set_headless(1);
    clear_screen(0);
    f_hdr(0xAF12, 1, 4, 1, 100);
    {
        size_t fat = f_frame();
        size_t at = flen;

        w32(at, 6);                           /* chunk size 6: header only */
        w16(at + 4, 16);
        flen += 6;
        w16(fat + 6, 1);
        w32(fat, (uint32_t)(flen - fat));
    }
    rc = play_mem(1, 5);
    CHECK(rc == 0, "overrun chunk: end of pass, rc 0");

    /* (the Esc and count-2 cases run in test_a_esc below, where the hook state is local) */

    /* flags & 2 at (630, 470), W = H = 20, BLACK: rows wrap past x 639 into the next scanline; y >= 480 not drawn */
    fg_init();
    fg_set_headless(1);
    clear_screen(0x33);
    f_hdr(0xAF12, 1, 20, 20, 100);
    {
        size_t fat = f_frame();
        f_chunk(fat, 13, NULL, 0);
    }
    fg_move(630, 470);
    rc = play_mem(1, 4 | 2 | 1);              /* memory, at cursor, no delay */
    CHECK(rc == 0, "wrap rc");
    CHECK(pix(630, 470) == 0 && pix(639, 470) == 0 && pix(0, 471) == 0 && pix(9, 471) == 0, "wrap row 470");
    CHECK(pix(630, 479) == 0 && pix(639, 479) == 0, "row 479 to the right edge; the tail past 640*480 is dropped");
    CHECK(pix(10, 479) == 0x33 && pix(629, 479) == 0x33 && pix(629, 470) == 0x33 && pix(10, 471) == 0x33, "outside the rect untouched");
    /* rows y >= 480 are off the 640x480 buffer: the y >= 480 clip and the off >= 640*480 guard keep them from
       writing past s_screen; the ASan build proves no out-of-bounds write */
}

/* Esc / pass-count cases with a local hook */
static int g_hook_calls;
static int g_hook_esc_at;

static void esc_hook(void)
{
    g_hook_calls++;
    if (g_hook_calls == g_hook_esc_at)
        fg_test_key(27);
}

static void test_a_esc(void)
{
    int rc, i;

    /* count 0 stopped by Esc after 5 frames */
    fg_init();
    fg_set_headless(1);
    f_hdr(0xAF12, 10, 4, 4, 100);
    for (i = 0; i < 10; i++)
        (void)f_frame();
    g_hook_calls = 0;
    g_hook_esc_at = 5;
    fg_set_input_hook(esc_hook);
    rc = play_mem(0, 5);                      /* count 0: endless until Esc */
    fg_set_input_hook(NULL);
    CHECK(rc == 0, "Esc stop rc 0");
    CHECK(g_hook_calls == 5, "Esc after 5 frames");

    /* count 2: two passes over 3 frames */
    fg_init();
    fg_set_headless(1);
    f_hdr(0xAF12, 3, 4, 4, 100);
    for (i = 0; i < 3; i++)
        (void)f_frame();
    g_hook_calls = 0;
    g_hook_esc_at = 1000000;
    fg_set_input_hook(esc_hook);
    rc = play_mem(2, 5);
    fg_set_input_hook(NULL);
    CHECK(rc == 0, "count 2 rc 0");
    CHECK(g_hook_calls == 6, "count 2 shows 2*3 frames");
}

/* ---- part b: the catalog's FLCs against flc.c -------------------------------------------------------------------- */

static uint8_t *g_cap_pix;        /* frames * w * h */
static uint8_t (*g_cap_dac)[256][3];
static int g_cap_w, g_cap_h, g_cap_n, g_cap_max;
static int g_cap_x0, g_cap_y0;

static void cap_hook(void)
{
    const uint8_t *scr;
    int r, c;

    if (g_cap_n >= g_cap_max)
        return;
    scr = fg_screen();
    for (r = 0; r < g_cap_h; r++)
        for (c = 0; c < g_cap_w; c++)
            g_cap_pix[(size_t)g_cap_n * g_cap_w * g_cap_h + (size_t)r * g_cap_w + c] =
                scr[(size_t)(g_cap_y0 + r) * 640 + (g_cap_x0 + c)];
    memcpy(g_cap_dac[g_cap_n], fg_dac(), sizeof g_cap_dac[0]);
    g_cap_n++;
}

/* count the chunk types of a file (for the report line) */
static void count_chunks(const uint8_t *d, size_t len, uint32_t counts[17])
{
    size_t pos = 0x80;

    memset(counts, 0, 17 * sizeof *counts);
    while (pos + 6 <= len) {
        uint32_t fsize = (uint32_t)d[pos] | ((uint32_t)d[pos + 1] << 8) | ((uint32_t)d[pos + 2] << 16) | ((uint32_t)d[pos + 3] << 24);
        uint32_t ftype = (uint32_t)d[pos + 4] | ((uint32_t)d[pos + 5] << 8);
        size_t p;

        if (fsize < 6 || pos + fsize > len)
            break;
        if (ftype == 0xF1FAu) {
            uint32_t nch = (uint32_t)d[pos + 6] | ((uint32_t)d[pos + 7] << 8);
            p = pos + 16;
            while (nch-- > 0 && p + 6 <= pos + fsize) {
                uint32_t csize = (uint32_t)d[p] | ((uint32_t)d[p + 1] << 8) | ((uint32_t)d[p + 2] << 16) | ((uint32_t)d[p + 3] << 24);
                uint32_t ctype = (uint32_t)d[p + 4] | ((uint32_t)d[p + 5] << 8);

                if (ctype <= 16)
                    counts[ctype]++;
                if (csize < 6)
                    break;
                p += csize;
            }
        }
        pos += fsize;
    }
}

static int part_b(const char *gamedir)
{
    char path[1100];
    uint8_t *catdata;
    long catsize;
    FILE *f;
    catalog c;
    int j, files = 0, badfiles = 0;

    snprintf(path, sizeof path, "%s/ACIV.CT4", gamedir);
    f = fopen(path, "rb");
    if (f == NULL) {
        printf("part b: no %s, skipped\n", path);
        return 0;
    }
    fseek(f, 0, SEEK_END);
    catsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    catdata = malloc((size_t)catsize);
    if (catdata == NULL || fread(catdata, 1, (size_t)catsize, f) != (size_t)catsize) {
        fclose(f);
        return 1;
    }
    fclose(f);
    if (cat_open(&c, catdata, (size_t)catsize) != 0) {
        printf("part b: cat_open failed on %s\n", path);
        return 1;
    }
    for (j = 0; j < c.count; j++) {
        const uint8_t *d;
        size_t len;
        flc_anim a;
        int w, h, x0, y0, rc, k, ncmp, mism;
        uint32_t counts[17];
        uint32_t magic, frames, delay;

        if (c.entries[j].kind != 6)
            continue;
        d = cat_data(&c, &c.entries[j]);
        len = c.entries[j].length;
        if (d == NULL || flc_open(&a, d, len) != 0) {
            printf("%-12s flc_open failed\n", c.entries[j].name);
            fails++;
            badfiles++;
            continue;
        }
        files++;
        fg_init();
        fg_set_headless(1);
        fg_flicsize(d, &w, &h);
        x0 = (0x270 - w) / 2 + 8;
        y0 = (0x13F - h) / 2 + 6;
        fg_move(x0, y0);
        fg_imagebuf((uint8_t *)d, (uint32_t)len);
        g_cap_w = w;
        g_cap_h = h;
        g_cap_x0 = x0;
        g_cap_y0 = y0;
        g_cap_max = (int)a.frames + 2;
        if ((size_t)g_cap_max * w * h > (64u << 20))
            g_cap_max = (int)((64u << 20) / ((size_t)w * h));
        g_cap_pix = malloc((size_t)g_cap_max * w * h);
        g_cap_dac = malloc((size_t)g_cap_max * sizeof *g_cap_dac);
        g_cap_n = 0;
        fg_set_input_hook(cap_hook);
        rc = fg_showflic("", 1, 7);           /* 7 = 4|2|1: memory, at cursor, no delay */
        fg_set_input_hook(NULL);
        CHECK(rc == 0, "showflic rc 0 on catalog file");

        /* compare with flc.c, frame by frame */
        ncmp = g_cap_n < (int)a.frames ? g_cap_n : (int)a.frames;
        mism = 0;
        for (k = 0; k < ncmp; k++) {
            int r, cc, bad = 0;

            if (flc_next(&a) != 0) {
                bad = 1;
            } else {
                for (r = 0; r < h && !bad; r++) {
                    if (memcmp(g_cap_pix + (size_t)k * w * h + (size_t)r * w, a.pixels + (size_t)r * w, (size_t)w) != 0)
                        bad = 1;
                }
                for (cc = 0; cc < 256 && !bad; cc++)
                    if ((g_cap_dac[k][cc][0] != (a.pal[cc][0] >> 2)) ||
                        (g_cap_dac[k][cc][1] != (a.pal[cc][1] >> 2)) ||
                        (g_cap_dac[k][cc][2] != (a.pal[cc][2] >> 2)))
                        bad = 2;
            }
            if (bad) {
                mism++;
                if (mism == 1)
                    printf("  first mismatch in %s frame %d (pixels=%d pal=%d)\n", c.entries[j].name, k, bad == 1, bad == 2);
            }
        }
        if (mism)
            fails++;
        badfiles += mism != 0;

        magic = (uint32_t)d[4] | ((uint32_t)d[5] << 8);
        frames = (uint32_t)d[6] | ((uint32_t)d[7] << 8);
        if (magic == 0xAF11u)
            delay = ((uint32_t)((uint32_t)d[16] | ((uint32_t)d[17] << 8)) * 1429u) / 5500u;
        else
            delay = ((uint32_t)d[16] | ((uint32_t)d[17] << 8) | ((uint32_t)d[18] << 16) | ((uint32_t)d[19] << 24)) / 55u;
        count_chunks(d, len, counts);
        {
            char chunks[200];
            size_t o = 0;

            chunks[0] = 0;
            for (k = 0; k <= 16; k++)
                if (counts[k])
                    o += (size_t)snprintf(chunks + o, sizeof chunks - o, "%s%u:%u", o ? " " : "", k, counts[k]);
            printf("%-12s %dx%d frames=%u delay=%u chunks[%s] compared=%d mismatches=%d\n", c.entries[j].name, w, h,
                   frames, delay, chunks, ncmp, mism);
        }
        free(g_cap_pix);
        free(g_cap_dac);
        g_cap_pix = NULL;
        g_cap_dac = NULL;
        flc_close(&a);
    }
    printf("part b: %d FLC files, %d with mismatches\n", files, badfiles);
    free(catdata);
    return badfiles != 0;
}

int main(int argc, char **argv)
{
    const char *gamedir = argc > 1 ? argv[1] : "C:/work/advciv_play";

    test_a();
    test_a_esc();
    part_b(gamedir);

    if (fails == 0)
        printf("showflic_test: all checks pass\n");
    else
        printf("showflic_test: %d FAILURES\n", fails);
    return fails != 0;
}
