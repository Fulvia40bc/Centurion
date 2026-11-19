#ifndef JUNK_H
#define JUNK_H

/* One shared "junk" screen save (TASK 13): the original's fg_makegif (0x8CE50, not in fg.c) writes a screen
   rectangle to a GIF file named "junk" / "junk2" in the current folder and fg_showgif (0x88F98) reads it back
   (docs/LIBRARY_CALLS.md); the files exist only to be read back. The port keeps one slot per name in memory
   instead, shared by every module (info.c, menu1.c, menu2.c, stack.c, kit.c, later the game loop), as the original
   shares the one file per name. */

int junk_makegif(int minx, int maxx, int miny, int maxy, const char *name);
int junk_showgif(const char *name, int flags);

#endif
