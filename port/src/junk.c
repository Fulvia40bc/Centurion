/* The shared "junk" screen save (TASK 13), replacing the five per-module copies (info.c's info_makegif /
   info_showgif, menu1.c's junk_save / junk_show, menu2.c's junk_save / junk_show, stack.c's stack_makegif /
   stack_showgif, kit.c's kit_makegif / kit_showgif): the original saves a screen rectangle with fg_makegif
   (0x8CE50, not in fg.c) to a GIF file named "junk" (or "junk2") and puts it back with fg_showgif(name, 3), one
   file per name shared by all of them. The port keeps the rectangle in one slot per name instead; no junk file is
   written to the game folder (MAP_PLAN.md risk 7).

   The screen stays identical at every call site: fg_makegif reads the active surface (the open virtual buffer,
   else the screen) and fg_showgif writes the active surface at the graphics cursor (flags bit 1), opaque, so the
   slot is read with fg_getpixel and written back with fg_point (both on the active surface, both clipped to the
   clip region; fg_showgif clips only right and bottom, but every call site runs with the full-surface clip, as
   map.c restores it after drawing the map). fg_point moves neither the cursor nor the clip region and fg_showgif
   leaves the colour alone; the colour is saved and put back around the loop here as the copies did. flags bit 0
   (keep the palette) needs no work in memory: the slot holds no palette, and a GIF written by fg_makegif carries
   the current DAC, so showing without the flag would restore the same palette.

   Prototypes: junk.h. */

#include <stddef.h>
#include <stdint.h>

#include "fg.h"
#include "junk.h"

#define SCR_W 640
#define SCR_H 480

typedef struct {
    int valid;
    int32_t w, h;
    uint8_t px[SCR_W * SCR_H];
} junk_slot;

static junk_slot s_slot[2];       /* 0 = "junk", 1 = "junk2" */

static int ci(int ch)
{
    return ch >= 'A' && ch <= 'Z' ? ch - 'A' + 'a' : ch;
}

/* the slot for a file name, compared without case as DOS compares file names; -1 for any other name */
static int slot_of(const char *name)
{
    static const char *const names[2] = { "junk", "junk2" };
    int i, j;

    if (name == NULL)
        return -1;
    for (i = 0; i < 2; i++) {
        for (j = 0; names[i][j] != 0; j++)
            if (name[j] == 0 || ci((unsigned char)name[j]) != (unsigned char)names[i][j])
                break;
        if (names[i][j] == 0 && name[j] == 0)
            return i;
    }
    return -1;
}

/* fg_makegif(minx, maxx, miny, maxy, name) (0x8CE50): the rectangle of the active surface (the open virtual
   buffer, else the screen) into the slot "name". Returns 0, or 1 for any other name or a rectangle that does not
   fit the 640x480 screen (port guard (G): the game's rectangles are all inside; the slot is emptied, as a failed
   write would leave no readable file). */
int junk_makegif(int minx, int maxx, int miny, int maxy, const char *name)
{
    int s = slot_of(name);
    junk_slot *g;
    int32_t w = maxx - minx + 1, h = maxy - miny + 1, r, c, v;

    if (s < 0)
        return 1;
    g = &s_slot[s];
    if (w <= 0 || h <= 0 || (int64_t)w * h > (int64_t)sizeof g->px) {
        g->valid = 0;
        return 1;
    }
    for (r = 0; r < h; r++)
        for (c = 0; c < w; c++) {
            v = fg_getpixel(minx + c, miny + r);          /* the active surface; -1 outside the clip region */
            g->px[r * w + c] = (uint8_t)(v < 0 ? 0 : v);
        }
    g->w = w;
    g->h = h;
    g->valid = 1;
    return 0;
}

/* fg_showgif(name, flags) (0x88F98) for flags 2 and 3: the slot's rectangle back at the graphics cursor
   (upper-left), opaque, onto the active surface; the cursor, the colour and the clip region are unchanged.
   Without flags bit 1 the original draws at the descriptor's left/top (0,0 for a rectangle fg_makegif wrote).
   Returns 1 when the slot is empty or the name has no slot (fg_showgif returns 1 when the file does not exist),
   else 0. */
int junk_showgif(const char *name, int flags)
{
    int s = slot_of(name);
    junk_slot *g;
    int32_t x0, y0, r, c;
    int color;

    if (s < 0)
        return 1;
    g = &s_slot[s];
    if (!g->valid)
        return 1;
    if (flags & 2) {                                    /* 0x8927D: at the graphics cursor */
        x0 = fg_getxpos();
        y0 = fg_getypos();
    } else {
        x0 = 0;
        y0 = 0;
    }
    color = fg_getcolor();
    for (r = 0; r < g->h; r++)
        for (c = 0; c < g->w; c++) {
            fg_setcolor(g->px[r * g->w + c]);
            fg_point(x0 + c, y0 + r);
        }
    fg_setcolor(color);
    return 0;
}
