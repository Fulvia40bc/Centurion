#ifndef ROUTE_H
#define ROUTE_H

/* Module ROUTE of docs/M6_PLAN.md (route.c): ship routes over N.RL. The queries the AI asks of the route tables that
   gfxload.c's f3EED8 loads from <prefix><players>.RL (layout in game.h):
     0xFFD44   int32 count, then 8-byte records at 0xFFD48: table 1 (without Astronomy)
     0x101DC8  int32 count, then 8-byte records at 0x101DCC: table 2 (with Astronomy, nation record +0x2B != 0)
               record: +0 uint8 from, +1 uint8 to, +2 int16 number of routes, +4 int32 first route (in 4-byte units)
     [0x10740C] the route block (a port heap handle, load_heap_ptr): route r is the 4 bytes at block + 4r, the
               intermediate territories in order, 0xFF-terminated when fewer than 4
     0x107410 / 0x107550  int16[160]  first record of table 1 / 2 whose from byte is t, else -1

   Results (D):
     0x108820  350 records of 16 bytes (0x15E0): +0..+5 the path from a to b (a, the intermediates, b, then 0xFF),
               +6 the points left at the end (k - steps), +7..+15 untouched here (G: +8 used by 0x4D29C)
     int32 [0x109E00]  number of records kept in 0x108820
     0x107EE4  int32[160]  1 for each territory a ship of the nation can reach from t (f5861C)

   Conventions: those of ui.h and game.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx, ebx,
   ecx; void unless a caller reads eax). D = read in the code, G = inference. n is a nation (record 0xF14CC + 199n), k
   the movement points. */

#include <stdint.h>

#define ROUTE_BUF    0x108820u   /* 350 records of 16 bytes */
#define ROUTE_COUNT  0x109E00u   /* int32 */
#define ROUTE_REACH  0x107EE4u   /* int32[160] */

int  f5843C(int a, int b, int n, int k);   /* D: k == 0 -> 0; swaps a and b so that a <= b; Astronomy (+0x2B of nation n) -> f58568(a, b, k), else f584B4(a, b, k). 6 callers read eax (== 1) */
int  f584B4(int a, int b, int k);          /* D: table 1: the first record with from == a and to == b (linear scan from record 0); 1 if its first route has fewer than k intermediates (0xFF-terminated, at most 4), else 0; 0 if no record */
int  f58568(int a, int b, int k);          /* D: f584B4 on table 2 */
void f5861C(int t, int n, int k);          /* D: memset(0x107EE4, 0, 0x280); then for every record of table 2 (Astronomy) or 1 with t as from (or else as to) whose first route has fewer than k intermediates: int32 0x107EE4[the other end] = 1. 3 callers, eax unused */
void f58888(int a, int b, int n, int k);   /* D: k == 0 -> [0x109E00] = 0; else swaps so a <= b, memset(0x108820, 0xFF, 0x15E0), f58DCC (Astronomy) or f58A74 (a, b, n, k); if swapped each kept path is reversed and shifted to byte 0; then +6 of each = (uint8)(k - (index of the first 0xFF from byte 2, at most 6) + 1). 12 callers, eax unused */
void f58A74(int a, int b, int n, int k);   /* D: the routes a -> b of table 1 (from record int16 0x107410[a]) into 0x108820, [0x109E00] the count; a route through a territory u of type 8 (0xC80DC[u]) where nation n has no tokens (area +0x0E+n) is blocked and counted in a local int16[160]; the first route with >= k intermediates ends the scan; if none was kept, the most rarely blocking u is cleared and the scan redone (blocking now on a nonzero counter), until one is kept or no counter is left. Called only by f58888 */
void f58DCC(int a, int b, int n, int k);   /* D: f58A74 on table 2 (index 0x107550). Called only by f58888 */

#endif
