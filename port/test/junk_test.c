/* junk_test: the shared "junk" screen save (junk.c) against the real fg.c surfaces.
   Covers: an empty slot, an unknown name, save + show on the screen, overwriting a slot, both names ("junk" and
   "junk2", case-insensitive, separate slots), save from / show onto a different surface (an open virtual buffer),
   clipping at the right/bottom edge, and the cursor, colour and clip region left unchanged. Prints one FAIL line
   per broken check and exits 0 when all pass.
   Build: clang -std=c11 port/test/junk_test.c port/src/junk.c port/src/fg.c port/src/gif.c port/src/catalog.c port/src/fgfont.c port/src/palette.c port/src/png_write.c -Iport/src -o junk_test.exe
   (plat_* and g_ds are stubbed below; no game data is read) */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "platform.h"
#include "fg.h"
#include "junk.h"

/* ---- stubs for what fg.c links against -------------------------------------------------------------------------- */

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

#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL %s\n", msg); fails++; } } while (0)

static int pat(int x, int y) { return (x * 7 + y * 13 + 1) & 0xFF; }

/* fill a rectangle of the active surface with pat() through the fg calls */
static void draw_rect(int x0, int y0, int w, int h)
{
    int r, c;
    for (r = 0; r < h; r++)
        for (c = 0; c < w; c++) {
            fg_setcolor(pat(x0 + c, y0 + r));
            fg_point(x0 + c, y0 + r);
        }
}

static void draw_rect2(int x0, int y0, int w, int h)   /* a second, different pattern */
{
    int r, c;
    for (r = 0; r < h; r++)
        for (c = 0; c < w; c++) {
            fg_setcolor((pat(x0 + c, y0 + r) + 0x5A) & 0xFF);
            fg_point(x0 + c, y0 + r);
        }
}

static int rect_is(int x0, int y0, int w, int h, int which)
{
    int r, c;
    for (r = 0; r < h; r++)
        for (c = 0; c < w; c++) {
            int want = which == 0 ? pat(x0 + c, y0 + r) : (pat(x0 + c, y0 + r) + 0x5A) & 0xFF;
            if (fg_getpixel(x0 + c, y0 + r) != want)
                return 0;
        }
    return 1;
}

static uint8_t s_vb[640 * 480];

int main(void)
{
    int h, r, c, rc;

    fg_init();
    fg_set_headless(1);

    /* empty slots and unknown names: fg_showgif returns 1 when the file does not exist */
    CHECK(junk_showgif("junk", 3) == 1, "empty junk returns 1");
    CHECK(junk_showgif("junk2", 3) == 1, "empty junk2 returns 1");
    CHECK(junk_showgif("nope", 3) == 1, "unknown name show returns 1");
    CHECK(junk_makegif(0, 9, 0, 9, "nope") == 1, "unknown name save returns 1");
    CHECK(fg_getpixel(5, 5) == 0, "empty show draws nothing");

    /* save + show on the screen */
    draw_rect(10, 20, 100, 50);                       /* pattern 1 in (10,20)-(109,69) */
    fg_move(0, 0);
    fg_setcolor(7);
    CHECK(junk_makegif(10, 109, 20, 69, "junk") == 0, "save junk returns 0");
    CHECK(fg_getxpos() == 0 && fg_getypos() == 0 && fg_getcolor() == 7, "makegif leaves cursor and colour");
    draw_rect2(10, 20, 100, 50);                      /* overwrite with pattern 2 */
    CHECK(rect_is(10, 20, 100, 50, 1), "pattern 2 drawn");
    fg_move(10, 20);
    fg_setcolor(7);
    rc = junk_showgif("junk", 3);
    CHECK(rc == 0, "show junk returns 0");
    CHECK(rect_is(10, 20, 100, 50, 0), "show restores pattern 1");
    CHECK(fg_getxpos() == 10 && fg_getypos() == 20, "showgif leaves the cursor");
    CHECK(fg_getcolor() == 7, "showgif leaves the colour");
    CHECK(fg_getpixel(5, 5) == 0 && fg_getpixel(110, 20) == 0, "showgif touches only the rectangle");

    /* overwrite the slot: the second save wins */
    draw_rect(10, 20, 100, 50);                       /* pattern 1 again */
    CHECK(junk_makegif(10, 109, 20, 69, "junk") == 0, "re-save junk");
    draw_rect2(10, 20, 100, 50);                      /* pattern 2 */
    draw_rect(10, 20, 100, 50);                       /* pattern 1 */
    CHECK(junk_makegif(10, 109, 20, 69, "JUNK") == 0, "case-insensitive save (JUNK)");
    draw_rect2(10, 20, 100, 50);
    fg_move(10, 20);
    CHECK(junk_showgif("junk", 3) == 0, "show after overwrite");
    CHECK(rect_is(10, 20, 100, 50, 0), "overwritten slot restores pattern 1");

    /* both names, separate slots */
    draw_rect(0, 0, 4, 4);
    CHECK(junk_makegif(0, 3, 0, 3, "junk2") == 0, "save junk2");
    draw_rect2(0, 0, 4, 4);
    fg_move(0, 0);
    CHECK(junk_showgif("Junk2", 3) == 0, "show junk2 case-insensitive");
    CHECK(rect_is(0, 0, 4, 4, 0), "junk2 slot independent of junk");
    CHECK(rect_is(10, 20, 100, 50, 0), "junk slot untouched by junk2");

    /* a different surface: save from an open virtual buffer, show into it; the screen keeps its bytes */
    h = fg_vbdefine(s_vb, 640, 480);
    CHECK(h >= 0, "vbdefine");
    fg_vbopen(h);
    draw_rect(100, 100, 50, 40);
    CHECK(junk_makegif(100, 149, 100, 139, "junk") == 0, "save from the open vb");
    fg_vbclose();
    draw_rect2(100, 100, 50, 40);                     /* screen now has pattern 2 there */
    fg_vbopen(h);
    draw_rect2(100, 100, 50, 40);                     /* vb now has pattern 2 */
    fg_move(100, 100);
    CHECK(junk_showgif("junk", 3) == 0, "show into the open vb");
    CHECK(rect_is(100, 100, 50, 40, 0), "vb got the saved pattern 1");
    fg_vbclose();
    CHECK(rect_is(100, 100, 50, 40, 1), "screen kept pattern 2 (show wrote the vb only)");

    /* clipping at the right/bottom edge: the cursor at (630, 470), a 20x20 slot */
    draw_rect(0, 0, 20, 20);
    CHECK(junk_makegif(0, 19, 0, 19, "junk2") == 0, "save 20x20");
    draw_rect2(0, 0, 640, 480);                       /* clear everything to pattern 2 */
    fg_move(630, 470);
    CHECK(junk_showgif("junk2", 3) == 0, "show at (630,470)");
    {
        int ok = 1;
        for (r = 0; r < 10; r++)
            for (c = 0; c < 10; c++)
                if (fg_getpixel(630 + c, 470 + r) != pat(c, r))
                    ok = 0;
        CHECK(ok, "clipped show writes the visible 10x10");
    }
    CHECK(fg_getpixel(0, 471) == ((pat(0, 471) + 0x5A) & 0xFF), "no wrap past the right edge");

    /* a rectangle bigger than the 640x480 slot empties it (port guard, as info.c's copy had) */
    CHECK(junk_makegif(0, 639, 0, 480, "junk") == 1, "oversized save returns 1");
    CHECK(junk_showgif("junk", 3) == 1, "slot emptied by the failed save");

    if (fails == 0)
        printf("junk_test: all checks pass\n");
    else
        printf("junk_test: %d FAILURES\n", fails);
    return fails != 0;
}
