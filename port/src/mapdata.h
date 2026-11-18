#ifndef MAPDATA_H
#define MAPDATA_H

/* Module MAP1 of docs/MAP_PLAN.md, the data half (mapdata.c): the map scroll in pixels, the home view of a nation and
   the view centred on a territory, the highlight setters, the territory alias ids, the map hit test and the release of
   the map heap blocks. The drawing half (0x3D52C and its helpers) is map.h / map.c.
   Conventions: those of ui.h and game.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx, ebx,
   ecx; D = read in the code, G = inference).

   Data these routines use (D, read in the code at the addresses given in mapdata.c):
     0xF4884  int32  map scroll column (78 px steps), clamped by f3D52C to int16 [0xF6110]..[0xF6112]
     0xF4888  int32  map scroll row (half steps: (row/2)*71 + (row%2)*35 px), clamped by f3D52C to 0..7
     0xC19E4  int32[10]  home column of nation n {0,4,4,4,8,10,12,14,14,11}; 0xC1A0C int32[10] home row {2,8,0,0,0,4,0,
              0,2,8} (obj2)
     0xC803C  uint8[160]  canonical territory: the identity below 157, then 157 -> 57, 158 -> 13, 159 -> 135 (obj2)
     Territory record (game.h, 0xE1638 + 35t): +0x13 int8 the scroll column, +0x14 int8 the scroll row that centres
              the map on t (read by f3DA60; game.h still calls the two bytes "not touched")
     0xFFB60, 0xFFC00, 0xFFCA0  uint8[160] each: border highlight colour, token box colour, ship box colour (0 = none)
     int16 [0x109E14]  1 = the alias part of the map is in use (G), read by f3E8FC
     int8 [0x108804]   1 = land mode, 2 = sea mode (buttons 0x26/0x27); byte [0xF4102] the phase (G)
   Heap fields freed by f3F558 (all allocated with load.c's load_heap_alloc, directly or through gfxload.c's
   heap_alloc; the fields hold load_heap handles): [0xFFB54], [0xFFB58], [0xFFB5C] (landadj, coastadj, seaadj),
   [0x10740C] (N.RL), [0xFF648] (mapdataz), [0xFF8CC], [0xFF8D0] (borders1/2). */

#include <stdint.h>

int32_t f449B8(void);                 /* D: map scroll x in pixels: [0xF4884] * 78 (int32). Callers 0x3D1D4, 0x3D2BD, 0x3D5BA read eax */
int32_t f449E0(void);                 /* D: map scroll y in pixels: v = [0xF4888]; (v / 2) * 71 + (v % 2) * 35 (signed idiv: truncated, remainder with v's sign). Callers 0x3D1DC, 0x3D2CB, 0x3D5C2 read eax */
void f3D9FC(int n);                   /* D: home view of nation n: [0xF4884] = 0xC19E4[n], [0xF4888] = 0xC1A0C[n]; entry 0 instead when bit 0x02 of byte [0xE1624] is set and n == 2; then f3D52C(0, 0). Callers do not read eax (two tail positions, 0x2EF82 and 0x7942F, G) */
void f3DA60(int t);                   /* D: centre the map on territory t: c = 0xC803C[t]; [0xF4884] = int8 terr[c]+0x13, [0xF4888] = int8 terr[c]+0x14; f787F8(t) (t's name in the panel); f3D52C(0, 0). 35 call sites, eax unused (G for the tail positions) */
void f3F624(int t, int c);            /* D: border highlight colour: byte [0xFFB60 + t] = c */
void f3F670(void);                    /* D: clear every border highlight: memset(0xFFB60, 0, 0xA0) */
void f3F698(int t, int c);            /* D: token box colour: byte [0xFFC00 + t] = c */
void f3F70C(int t, int c);            /* D: ship box colour: byte [0xFFCA0 + t] = c */
int  f3E89C(int t);                   /* D: territory -> alias id: 0x39 -> 0x9D, 0x0D -> 0x9E, 0x87 -> 0x9F, any other t unchanged */
int  f3E8FC(int t);                   /* D: if int16 [0x109E14] == 1 and 0xC803C[t] is 0x39 / 0x0D / 0x87: 0x9D / 0x9E / 0x9F; otherwise t itself (not the canonical value) */
int  f3D2A4(int x, int y);            /* D: map hit test at screen point (x, y): the first territory t < 160 in play (0xEE16C) whose mapdataz spans contain (x + scroll x - 8, y + scroll y - 6); t as is when int8 [0x108804] == 2 or byte [0xF4102] == 4, else 0x9D/0x9E/0x9F -> 0x39/0x0D/0x87; -100 if none. Callers 0x3D517 and 0x78CC2 read eax */
void f3F558(void);                    /* D: free the map heap blocks (above), each only if non-zero, and zero its field. BUG(1995) fixed unless g_original: after freeing [0x10740C] the original zeroes [0xFFB5C] instead (0x3F5C7) */

#endif
