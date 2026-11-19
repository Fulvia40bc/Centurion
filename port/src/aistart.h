#ifndef AISTART_H
#define AISTART_H

/* The opening-map ("base map", .BPM) name of each nation for the player count (module START, the computers'
   starting territories): 0x14C6C, aistart.c. */

#include <stdint.h>

/* D: copy the .BPM file name for nation n (1..9) into dst (eax, a caller's stack buffer: 0x14A00 [ebp-0x24],
   0x454D0 [ebp-0x30]; names are 10 characters + NUL): the developers' override name 0xF6064 + 13n if int32
   [0xF60E8 + 4n] != 0 (the flags are never set: main 0x10172 clears 0xF60E8[10] and nothing writes them; only
   Iberia's name gets bytes 1..3 "BER"/"TAL" from 0x128F0), else by n, human or not
   (byte +0x9A == 3), [0xF40F0] players and which other nations are in play (bytes +0x9A != 0). Leaves dst untouched
   for n outside 1..9 or a player count with no map for n. Void: both callers reload eax after the call
   (0x14A65, 0x45942). The file is read by 0x15FE0 (start.h) from ACIV.CT3 ([0xF40DC]). */
void f14C6C(char *dst, int n);

#endif
