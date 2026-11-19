#ifndef CAL5_H
#define CAL5_H

/* Module CAL5 of docs/M6_PLAN.md (cal5.c): the Barbarian Hordes calamity 0x217A0 (card 0x1A, called by the calamity
   phase 0x1E0A0 at 0x1E2DA) with the invasion 0x1C3D0, the landing 0x1C9FC, one move 0x1D234, the route search
   0x1D858, its sort 0x1DEAC and loss count 0x1DFFC, and the world-state snapshot 0x7A4F0 / 0x7AB24. The barbarian
   routing runs for every victim, human or computer (D: no test of +0x9A in 0x1C3D0); only the choice of the invaded
   territory differs (a human director picks it through cal1.c's f207B4, a computer by game_rand, inline here).

   Conventions: those of ui.h, game.h, kit.h, rules.h and conflict.h (fXXXXX = the original at 0xXXXXX, Watcom
   register arguments eax, edx, ebx, ecx, then the stack; void unless a caller reads eax; records as uint32_t link
   addresses; a pointer the original passes to a local or a global buffer as a C pointer). D = read in the code, G =
   inference.

   Globals (D):
     route buffer 0xF6E94: 4-byte entries {+0 low nibble: the victim's losses 3 * cities + tokens (mod 16), high
       nibble: barbarians left after the first move (depth-0 entries only); +1 the losses of everybody (0x1DFFC, low
       byte); +2 the depth of the move that hurts (0xFF: no damage reachable); +3 the territory of the first move};
       10 entries written at depth 0, followed directly by the visited bytes 0xF6EBC[160] (157..159 preset to 1)
     int32 [0xF6E8C] the search depth, int32 [0xF6E90] the sort's swap temp, int32 [0xF6F5C] the BRPC call counter
     (never reset), int32 [0xF6F60] 1 = BRPC mode (the deep search; spinner every 250 calls)
     the choice list int32 0xF7CA0[157] / [0xF7C9C] (the equal-best destinations; cal1.c's f207B4 kind 0x1A shows it)
     heap handles [0xEED74] areas 1..160 (0x2B20 bytes), [0xEED78] nations 1..11 (0x88D), [0xEED7C] territory states
     0..159 (0x1E0): 0x7A4F0 / 0x7AB24 (port handles, load_heap_*)
     uint8 0xF547C[14] / 0xF548C[14] tokens / cities lost (conflict.h); area +0x19 the barbarians (nation 11), +0x3B + n
     the "lost territory" mark (2 here); territory byte +0 high nibble the barbarians' landing nation, +1 bits 3..5 the
     population limit */

#include <stdint.h>

void f217A0(int k);           /* D: Barbarian Hordes for calamity record k (0xF77E4 + 3k): victim v; the director is the
                                 record's giver unless -1, else the nation 1..9 in play, not v, with the highest
                                 5 * (9 - cities) + stock (strictly above 0; BUG(1995) fixed unless g_original when nobody
                                 scores above 0, cal5.c); Crete (5) is spared with a message, else f1C3D0(v, director);
                                 then f1E69C(k). Caller: cal1.c's f1E0A0 (0x1E2DA), eax unused */
void f1C3D0(int n, int d);    /* D: the invasion of victim n directed by d: the messages, the landing f1C9FC, the first
                                 move of 15 barbarians (f1D234), then while the barbarians exceed the limit of their
                                 territory: the route search f1D858 under a snapshot (and the BRPC's 20-barbarian search
                                 when no route), the next move; "deadlocked" / "wiped out" end it. 1.5 s and 4 s clock
                                 waits (port: f78A20). One caller (0x218CE) */
void f1C9FC(int n);           /* D: the landing list 0xF7CA0 / [0xF7C9C] for victim n: n's landing zones (territory byte
                                 +0 high nibble == n) where n has tokens or a city, the most tokens (a city counts 6)
                                 first; else the empty zones, else all zones, each tried by a search (f1D858) and the
                                 best kept. BUG(1995) fixed unless g_original: the runs past the list (cal5.c) */
void f1D234(int n, int d, int k, int32_t *pt);
                              /* D: one move of k barbarians of victim n from *pt (-1: none yet) to the list's
                                 destination: the "settle" path (route entry 0 at depth 0 with no losses), the invasion
                                 (one destination, or the director's choice: f207B4(d, n, 0x1A) for a hot-seat human,
                                 else game_rand) with the battle f5B224 / f5B588 and the losses box f5CDE8; *pt = the new
                                 territory. Callers 0x1C4AA, 0x1C8F9 */
void f1D858(int n, int t, int k, uint8_t *out, const uint8_t *vis);
                              /* D: recursive route search of k barbarians from t (n the victim); vis: 160 visited
                                 bytes (copied); out: the best entry (and at depth 0 the 10 best). Each unvisited land /
                                 coast neighbour is tried on the real records (restored from 3 heap blocks after each
                                 try): a neighbour holding n's tokens or city gives an entry; else, if no neighbour
                                 does, the move goes on with the excess. ret 4 (vis on the stack). Callers 0x1C595,
                                 0x1C773, 0x1CECE, 0x1D181, itself 0x1DD94 */
void f1DEAC(int count, uint8_t *e);
                              /* D: sorts count 4-byte entries: by the low nibble of +0 descending (exchange passes),
                                 then by +2 ascending (bubble from the end; both stable); swaps through [0xF6E90] */
int  f1DFFC(void);            /* D: the sum of int8 0xF547C[1..10] and of 0xF548C[1..10]. BUG(1995) fixed unless
                                 g_original: the original adds only the first non-zero 0xF548C entry (cal5.c) */
void f7A4F0(void);            /* D: snapshot: [0xEED74] = malloc(0x2B20) <- areas 1..160, [0xEED78] = malloc(0x88D) <-
                                 nations 1..11, [0xEED7C] = malloc(0x1E0) <- territory states (port handles) */
void f7AB24(void);            /* D: restore the snapshot of f7A4F0 (areas, territory states, nations), free the three
                                 blocks and zero the fields */

#endif
