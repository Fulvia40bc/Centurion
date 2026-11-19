#ifndef GAME_H
#define GAME_H

/* The new-game loading of AC_MAIN.EXE: what main runs between the setup screens and the first phase (0x114CF..0x1184A),
   and the routines it calls there: the map files (0x12394, 0x3D170, 0x3EED8), the sprites (0x11A8C), the map variant
   (0x12B00, 0x44974), the card stacks (0x6BA20), the area, territory and nation tables (0x12B5C, 0x12BB8, 0x45D30,
   0x12C38) and the music (0x7B854). Prototypes shared by the translations, with the data layouts these routines set up.

   Conventions: those of ui.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments eax, edx, ebx, ecx, then
   the stack; void unless a caller uses eax, all call sites checked; data records as uint32_t link addresses; text
   as const char *). Grades: D = read in the code at the address given, G = inference.

   Runtime calls in these routines (Watcom C runtime -> standard C):
     0x7D8C8 fopen, 0x7D95E fclose, 0x7DB6D fread(buf eax, size edx, n ebx, fp ecx), 0x7D5D0 memset(eax, edx, ebx),
     0x7DB3D sprintf (stack, cdecl), 0x7D610 strcmp(eax, edx) (D: the word-at-a-time compare at 0x7D610), 0x822B5
     malloc(eax) (LIBRARY_CALLS.md: the edx/ebx values the callers leave are not arguments).
   Every fopen here uses mode "rb" (0xB0DA3, 0xB3F11, 0xB3F3A, 0xB3F63, 0xB3F8C, 0xB3FB3, 0xB3FDC, 0xB4009) and a name
   relative to the current directory, except N.RL (0x3EED8), which gets the game-folder prefix [0xF5820].

   Heap pointers the original keeps in 32-bit data fields (the port keeps the blocks in its own tables and stores
   handles or nothing there; a DOSBox-X dump differs in these by design). Several are pointers INTO a block (block +
   2*k), so a handle has to carry the offset as well (e.g. block number in the high bits, byte offset in the low):
     0xFF648                  mapdataz.dat block (0x3EA7C)
     0xFF64C..0xFF8CB         uint32[160] -> each territory's record inside it (0x3EB0E)
     0xFF8CC, 0xFF8D0         borders1.dat, borders2.dat blocks (0x3EC77, 0x3EDB7)
     0xFF8D4..0xFFB47         uint32[157] -> each border's record inside them (0x3ECAA, 0x3EDF4)
     0xFFB54, 0xFFB58, 0xFFB5C  landadj, coastadj, seaadj blocks (0x3EF31, 0x3F033, 0x3F13B)
     0xF8E7C, 0xF90FC, 0xF937C  uint32[160] each -> the territories' land / coast / sea lists inside them
     0x10740C                 the N.RL block (0x3F453)
     +0 of every sprite record 0xF48A4..0xF5224 (0x11A8C through 0x3AE90; ui_sprite_put's handles)
     0xF626C                  the loaded XMI file (0x7B932/0x7B954; only with music on)
   Also different by design: the jmp_buf at 0xF58A0 (setjmp at main 0x115CE, 0x2C bytes of registers), the C
   runtime's own state in the data object (FILE table and errno of fopen/fread, malloc's heap list 0xCB6AC), the
   catalog's directory cache (0xDA446.., cat_show), the AIL state (0xF6284 sequence handle, [0xCAC68] file error) and
   the game-folder string at 0xF5820 (the port's own path). */

#include <stdint.h>
#include "ui.h"

/* ---- Territories (TERRFLAG.DAT, 0x12394) -----------------------------------------------------------------------

   TERRFLAG.DAT (5602 bytes, D): int16 version (must be 3, else "terrflag.dat is wrong version..." and quit, 0x123DA),
   then 160 records of 35 bytes read straight to 0xE1638 (fread(0xE1638, 0x23, 0xA0), 0x1240F).
   Territory record t at 0xE1638 + 35*t (lea 9t, shl 2, sub t: D 0x12442):
     +0x00  uint8   (not touched by these routines)
     +0x01  uint8   (not touched)
     +0x02  uint8   flags: 0x12394 clears 0x02 and 0x04 in every record (0x12704, 0x1271C), then sets 0x02 in
                    territories 17, 35, 51, 60, 77, 89, 125, 134 (0x12725..0x12794) and 0x04 in 49, 50, 58, 59, 75, 76,
                    78, 84, 85, 90, 91 (0x1279A..0x12836) (D; meaning G)
     +0x03  int16 x0, +0x05 int16 y0   point 0 (G: where the territory's tokens or name go)
     +0x07  int16 x1, +0x09 int16 y1   point 1
     +0x0B  int16 x2, +0x0D int16 y2   point 2
     +0x0F  int16 x3, +0x11 int16 y3   point 3; 0x12394 sets x3 = 5050 (0x13BA, off the map, G: hidden) in the eight
                                       territories that get flag 0x02 (0x126A0..0x126D8)
                    After the scaled copies below, 0x12394 adds 8 to every x and 6 to every y (G: the map picture's
                    margin; borders1/2 get the same offsets)
     +0x13  2 bytes (not touched)
     +0x15  uint8[14]  in play per map variant (G: 1 = the territory is on the board in that variant): 0x12B00 copies
                    byte +0x15+[0xF6114] of every record to 0xEE16C
   Overview coordinates (int16[160] each, written by 0x12394 from the file values before the +8/+6, signed idiv):
     0xEE270 x0*624/1716, 0xEE3B0 y0*319/568, 0xEE4F0 x2*624/1716, 0xEE630 y2*319/568, 0xEE770 x1*624/1716,
     0xEE8B0 y1*319/568 (D 0x12442..0x12648; 1716 x 568 = the map picture [0xF52C4], 624 x 319 G: the overview map)
   Also by 0x12394: memset(0xF4234, 0, 0x5A0) then 1 in the 16 bytes at 0xF4234 + 300, 305, 480, 481, 490, 802..805,
   962..965, 1126, 1151, 1400 (D 0x1283C..0x128A9; 0x5A0 = 9 x 160, G: uint8[9][160], nation x territory).

   Map variant (main 0x1164A..0x1169B, D): int16 [0xF6114] = 7 * (bit 0x02 of the flags word 0xE1624) + [0xF40F0] - 2,
   0..13; then [0xF6110] = int16 0xBC7AE[v], [0xF6112] = int16 0xBC7CA[v] (meaning G). 0x12B00 reads v as
   dword [0xF6112] sar 16 (D 0x12B3A).
     0xEE16C  uint8[160]  in play in this game (0x12B00); 0 = left out: dropped from the adjacency lists (0x3EED8) and
                          recoloured on the map (0x44974) */
#define GAME_TERR_COUNT   160
#define GAME_TERR         0xE1638u
#define GAME_TERR_SIZE    35
#define GAME_TERR_FLAGS   0x02
#define GAME_TERR_X0      0x03
#define GAME_TERR_Y0      0x05
#define GAME_TERR_X1      0x07
#define GAME_TERR_Y1      0x09
#define GAME_TERR_X2      0x0B
#define GAME_TERR_Y2      0x0D
#define GAME_TERR_X3      0x0F
#define GAME_TERR_Y3      0x11
#define GAME_TERR_INPLAY  0x15
#define GAME_MAP_VARIANT  0xF6114u
#define GAME_INPLAY       0xEE16Cu

/* ---- Map geometry (0x3EA7C mapdataz.dat, 0x3EC20 borders1/2.dat), heap blocks ----------------------------------

   mapdataz.dat (D 0x3EA7C): uint32 size (= the rest of the file: the block malloc'd), then 160 records read word by
   word into the block, the record of territory t at [0xFF64C + 4*t]:
     int16 n (spans), int16 y (first row), int16 (not read by 0x44800), then n pairs int16 x1, x2.
     The spans (0x44800, D): row = y - 1 at the start; a pair with x1 >= 0 starts the next row (row + 1), x1 < 0
     continues the same row from -x1; pixels x1..x2 inclusive. Map-picture coordinates, no offset added.
   borders1.dat (records 0..64) and borders2.dat (65..156) (D 0x3EC20): uint32 size, then per record int16 n,
   int16 y (stored + 6), n int16 values stored moved 8 away from 0 (v >= 0: v + 8; v < 0: v - 8; the sign kept, G: a
   flag like the spans'). Record r at [0xFF8D4 + 4*r]. Size fields can exceed the data (borders2: 89948 for 51850
   bytes; only the allocation uses them).
   A file that does not open: "Can't find X\n" through f3A780 (printf + getch), then the reads go on with a NULL FILE
   (D 0x3EAAC, 0x3EC50, 0x3ED90; the original crashes there, G). */
#define GAME_MAPDATA_BLOCK  0xFF648u
#define GAME_MAPDATA_PTR    0xFF64Cu   /* uint32[160] */
#define GAME_BORDERS1_BLOCK 0xFF8CCu
#define GAME_BORDERS2_BLOCK 0xFF8D0u
#define GAME_BORDER_PTR     0xFF8D4u   /* uint32[157] */
#define GAME_BORDER_COUNT   157

/* ---- Adjacency and N.RL (0x3EED8) ------------------------------------------------------------------------------

   landadj.dat, coastadj.dat, seaadj.dat (D 0x3EED8..0x3F1F9): uint32 size, then 160 records int16 n, int16 t[n]
   (territory numbers), word by word into the blocks [0xFFB54], [0xFFB58], [0xFFB5C]; record t at [0xF8E7C + 4*t]
   (land), [0xF90FC + 4*t] (coast), [0xF937C + 4*t] (sea).
   Then every list of every territory loses the neighbours u with byte [0xEE16C + u] == 0 (shifted down, n - 1;
   D 0x3F1FE..0x3F38F). The lists live in the heap blocks, so this changes nothing in the data object.
   0xF95FC  uint8[157][157]  land adjacency matrix: memset 0x6049 bytes, then [a*157 + u] = 1 for each u in land list
                             a, a = 0..156 (D 0x3F391..0x3F407; a u above 156 would land in the next row)
   N.RL, n = [0xF40F0] (sprintf "%s%d.RL" with the prefix [0xF5820], 0x3F41E; opened without a NULL check, 0x3F433):
     uint32 m; 0x2084 bytes to 0xFFD44; 0x5644 bytes to 0x101DC8; m bytes to the heap block [0x10740C] (D 0x3F436..)
     0xFFD44   int32 count, then 8-byte records at 0xFFD48 (+0 uint8 territory; up to 0x410 records)
     0x101DC8  int32 count, then 8-byte records at 0x101DCC (+0 uint8 territory; up to 0xAC8 records)
     0x107410  int16[160]  first record of the 0xFFD48 table whose +0 byte is t, else -1 (D 0x3F4B3..0x3F531)
     0x107550  int16[160]  the same for the 0x101DCC table (G: both tables sorted by territory; meaning G, AI) */
#define GAME_LANDADJ_BLOCK  0xFFB54u
#define GAME_COASTADJ_BLOCK 0xFFB58u
#define GAME_SEAADJ_BLOCK   0xFFB5Cu
#define GAME_LANDADJ_PTR    0xF8E7Cu   /* uint32[160] */
#define GAME_COASTADJ_PTR   0xF90FCu   /* uint32[160] */
#define GAME_SEAADJ_PTR     0xF937Cu   /* uint32[160] */
#define GAME_LAND_MATRIX    0xF95FCu   /* uint8[157][157] */
#define GAME_RL_TABLE1      0xFFD44u
#define GAME_RL_TABLE2      0x101DC8u
#define GAME_RL_BLOCK       0x10740Cu
#define GAME_RL_FIRST1      0x107410u  /* int16[160] */
#define GAME_RL_FIRST2      0x107550u  /* int16[160] */

/* ---- Area and per-territory tables (0x12B5C, 0x12BB8, 0x12C38) -------------------------------------------------

   0xE31B8  161 records of 69 bytes (0x2B65 bytes, memset by 0x12B5C; record r at 0xE31B8 + 69*r, lea 17r, shl 2, add r:
            D 0x12B95). +0 uint8 = r (0x12BA7). Record t + 1 belongs to territory t (0x12C38 indexes it with the
            territory + 1, D 0x12D0D; G: record 0 = none). 0x12C38 (play-by-email start only) increments +1, writes
            the player's byte +4 to +1+n and 1 to +0x0E+n, n = that byte (G: the nation; meaning of the fields G)
   0xE2C18  160 records of 3 bytes: 0x12BB8 clears +2 bits 0x01, 0x02, then +0 = 0, then +2 &= 3 (so +2 = 0); +1
            untouched (D 0x12BEC..0x12C26; meaning G)
   Player table (G: 32-byte records, read by 0x12C38 for p < [0xF40EC]): int8 [0xF4104 + 32*p] (G: nation n),
   uint8 [0xF4105 + 32*p] (G: start territory). 0xF4102 is a separate byte main writes (0x15 at 0x11835, 0 at 0x11886;
   G: the music table index of 0x7B854's caller). */
#define GAME_AREA       0xE31B8u
#define GAME_AREA_SIZE  69
#define GAME_AREA_COUNT 161
#define GAME_TSTATE     0xE2C18u   /* 3-byte records [160] */

/* ---- Nations (0x45D30) -----------------------------------------------------------------------------------------

   0xF14CC  records of 199 bytes (0xC7, imul: D 0x45F4E), n = 0..13 (0x45D30 fills 1..13, 0x6BA20 clears parts of
            0..9; 1..9 G: the nations, 10..13 G: the other powers). 0x45D30 writes, for n = 1..13 (D 0x45D96..0x45F42):
     +0x01..+0x12  0          +0x14..+0x1B  0          +0x1C..+0x23  0xFF        +0x24..+0x3B  0
     +0x3D  n                 +0x3E  0                 +0x3F  2                  +0x40  0
     +0x45, +0x47, +0x49, +0x4B  int16 0               +0x4D..+0x50  0           +0x51  0
     +0x52..+0x5B  0          +0x5C  0                 +0x5E  int16 0
     +0x60  0 (G: count of the area list)              +0x61..+0x98  0xFF (56; G: areas with tokens), then +0x98 = 0
     +0x98  0 (G: tokens on the board)                 +0x99  [0xF4898] (G: tokens in stock)
     +0x9A  0 for n < 10, 3 for n >= 10 (written first, 0x45DA5 / 0x45F6E)       +0x9B  0
     +0x9C..+0xA8  0          +0xA9..+0xB1  0          +0xB2..+0xBA  20 (0x14)   +0xBB  1     +0xBC  1
   Untouched: +0x00, +0x13, +0x3C, +0x41..+0x44, +0x5D, +0x5F, +0xBD..+0xC6.
   0xF4898  int32: 0x37 (55) if [0xF40F0] is even, 0x2F (47) if odd; then 55 if it is 7 and 47 if it is 8 (D 0x45D45..)
   0x12C38 (PBEM only): +0x9A = 3, +0x99 - 1, +0x98 + 1, +0x60 + 1, +0x61 = territory + 1 (D 0x12C66..0x12CF6). */
#define GAME_NATION       0xF14CCu
#define GAME_NATION_SIZE  199
#define GAME_TOKEN_STOCK  0xF4898u

/* ---- Trade-card stacks (0x6BA20) -------------------------------------------------------------------------------

   0xF06D0  9 stacks of 25 bytes, stack s at 0xF06D0 + 25*s (lea 3s, *8, + s: D 0x6BB53):
     +0x00  int16  number of cards
     +0x02  int16  a: cards of the first commodity, +0x04 int16 b: of the second (constants written by 0x6BA20:
                   (7,7) (7,8) (8,9) (7,8) (6,7) (5,6) (5,6) (4,5) (4,5), D 0x6BABC..0x6BB39)
     +0x06  uint8[19]  the cards. 0x6BA20: card ids 2s+1 (a of them) and 2s+2 (b), shuffled (f76F70); for s = 1..8 the
                   cards from index [0xF40F0] on are shuffled together with the calamity [0xC8DEC + s] (0x14, 0x16,
                   0x18, 0x1A..0x1E) and put in front of the first [0xF40F0] cards (f77184), count = players + that;
                   for s = 1..4 the calamity [0xC8DF8 + s] (0x13, 0x15, 0x17, 0x19) goes in front, count + 1 (D; G:
                   tradable / non-tradable calamities, the first [0xF40F0] cards at the end are drawn first)
   int16 [0xF07B4 + 25*s] = 0 for s = 0..8 (D 0x6BC36; G: a second table of the same shape, e.g. the discards)
   0x6BA20 also clears nation records 0..9 (199 bytes): +0x01..+0x11 = 0, +0x13 = 0, +0x14..+0x1B = 0, and writes 0xFF
   to +0x24 (D 0x6BA35..0x6BA79: the store after the loop uses the final index 8, rec + 8 + 0x1C; G: the source meant
   +0x1C..+0x23, which 0x45D30 sets to 0xFF afterwards for 1..13, so only record 0 keeps +0x24 = 0xFF).
   The shuffles draw random numbers: seed [0xCAC5C] (OVERVIEW §5), set from the clock at main 0x117DA just before. */
#define GAME_STACK       0xF06D0u
#define GAME_STACK_SIZE  25
#define GAME_STACK_COUNT 9
#define GAME_STACK_CARDS 0x06
#define GAME_SEED        0xCAC5Cu

/* ---- Sprite records made by 0x11A8C (8 bytes each, ui.h's layout; captured with f3AE90 from the back buffer
   [0xF52C8] after cat_show(name, 3, [0xF40D8] = ACIV.CT2) at (0, 0)). Arrays of 14 are indexed 1..13 or 1..9 (entry 0
   unused); x0 = 50*(i-1) + 10, x1 = 20*(i-1) + 14 (D 0x11B5F.., 0x11BC5..):
     c_invld.gif:  0xF5214 (79, 446) 485x34, 0xF521C (55, 24) 116x25, 0xF5224 (17, 144) 251x25
     tokens2.gif:  0xF489C[14] i 1..13 (x0, 50) 18x18; 0xF497C[14] 1..13 (x0, 85) 15x15; 0xF4E3C[14] 1..13 (x1, 455)
                   11x10; 0xF4DCC[14] (x1, 441) 11x10; 0xF4D5C[14] (x1, 427) 11x10; 0xF490C[14] i 1..9 (x0, 15) 28x18
     tokens.gif:   0xF4ACC[14] 1..13 (x0, 50) 24x23; 0xF4BAC[14] (x0, 85) 24x23; 0xF4B3C[14] (x0, 15) 40x23;
                   0xF49EC[14] (x0, 130) 23x23; 0xF4A5C[14] i 1..9 (x0, 170) 35x35;
                   0xF4D2C + 8k, k 0..5: (72 + 16k, 362) 13x13;
                   0xF518C (200, 225) 54x28, 0xF5194 (200, 280) 54x28, 0xF519C (207, 327) 27x13,
                   0xF51A4 (207, 355) 27x13, 0xF517C (16, 295) 30x20, 0xF5184 (17, 296) 28x18;
                   for j 0..9 (y = 316 + 10j, h 9): 0xF4EAC + 8j (364) w 9, 0xF4EFC + 8j (378) w 11, 0xF4F4C + 8j
                   (392) w 13; 0xF4F9C + 8j (414) w 9, 0xF4FEC + 8j (428) w 11, 0xF503C + 8j (442) w 13;
                   0xF508C + 8j (462) w 11, 0xF50DC + 8j (476) w 13, 0xF512C + 8j (490) w 15
     cardtiny.gif: 0xF4C1C[34] i 1..33: ((i-1)%11 * 40, (i-1)/11 * 60) 32x46 (signed idiv by 11)
   With 0x118F8's twelve (0xF51AC..0xF5204, ui.h) and main's minimap 0xF520C (0x11628), 286 sprites in all: more than
   ui_widgets.c's UI_SPR_MAX (128) holds. */
#define GAME_SPR_C_INVLD0  0xF5214u
#define GAME_SPR_C_INVLD1  0xF521Cu
#define GAME_SPR_C_INVLD2  0xF5224u
#define GAME_SPR_MINIMAP   0xF520Cu
#define GAME_SPR_CARDTINY  0xF4C1Cu    /* [34], 1..33 */

/* ---- Music (0x7B854) -------------------------------------------------------------------------------------------

   Music table at 0xCAB78: 9-byte records {uint32 name (link address of an XMI name, e.g. "gmap__1c.xmi"), 3 bytes,
   int8 sequence at +8} (D 0x55408..0x5541A); main plays record 21 (0xCAC35: name, 0xCAC3D: sequence, 0x1183C).
   0xF625D int8   music on (0x7B854 does nothing if 0; set by the sound start-up, G)
   0xF61D8 char[] the name now loaded (strcmp 0x7B883, strcpy 0x7B981)
   0xF6258 int8   its sequence index (compared as dword [0xF6255] sar 24, 0x7B88C; written 0x7B9F6)
   0xF6284        AIL sequence handle; 0xF626C the XMI file's heap block; [0xF40E4] = ACIV.CT5's handle
   0xF097E        music volume (& 0x7F) * 127 / 9 -> AIL_set_sequence_volume (D 0x7B9FB)
   The other routines 0x7B854 calls are AIL 3.03's file and memory helpers, not game code (D, bodies at the
   addresses): 0x7D2E8 file size (open 0x93F92 with 0x200, filelength 0x941E5, close; error -> [0xCAC68]), 0x7D334
   read a whole file (into a block from [0xCAC6C] when the destination is 0), 0x7CB9D free a locked block (unlock
   0x7CD14, then [0xCAC70]), 0x7CCFD lock a range (0x7CC6F(p, p + size)); 0x78B0C is the debug log (a no-op). */

/* ---- Prototypes ------------------------------------------------------------------------------------------------ */

void f12394(void);                     /* D: load terrflag.dat (version 3, else message in 0xF5500 and f44CAC; can't open -> f128BC) into 0xE1638, the overview coordinates 0xEE270..0xEE8B0, +8/+6 on the points, flags 0x02/0x04 and x3 = 5050 of the listed territories, 0xF4234 (layout above) */
_Noreturn void f128BC(const char *name);  /* D: sprintf(0xF5500, "Can't open file - %s\n", name), quit (f44CAC). One caller (0x1241B, name 0xB0DEE) */
void f3D170(void);                     /* D: f3EA7C then f3EC20 (main 0x115B5) */
void f3EA7C(void);                     /* D: load mapdataz.dat into a heap block [0xFF648], record pointers [0xFF64C + 4*t] (layout above) */
void f3EC20(void);                     /* D: load borders1.dat (records 0..64, block [0xFF8CC]) and borders2.dat (65..156, block [0xFF8D0]); pointers [0xFF8D4 + 4*r]; y + 6, values +-8 */
void f11A8C(void);                     /* D: cut the game sprites (list above) from c_invld.gif, tokens2.gif, tokens.gif and cardtiny.gif (CT2) in the back buffer [0xF52C8], inside fg_vbopen/fg_vbclose */
void f12B00(void);                     /* D: 0xEE16C[t] = territory t's byte +0x15 + [0xF6114] (in play in this map variant), t 0..159 */
void f3EED8(void);                     /* D: load landadj/coastadj/seaadj.dat, drop the neighbours not in play (0xEE16C), build the land matrix 0xF95FC, load <prefix><players>.RL and its per-territory indexes 0x107410/0x107550 (layout above) */
void f44974(void);                     /* D: f44800(t) for every territory t with 0xEE16C[t] == 0 (main runs it in the map buffer [0xF52C4], after map&pal.gif) */
void f44800(int t);                    /* D: recolour territory t's spans (mapdataz record [0xFF64C + 4*t]) on the active surface: each pixel = 0xC2620[fg_getpixel(x, y)] via fg_setcolor + fg_point (G: greys out a territory not in play) */
void f6BA20(void);                     /* D: clear nation records 0..9's card fields, set the stacks' a/b counts, deal and shuffle the nine trade-card stacks 0xF06D0 with their calamities (layout above); uses random numbers */
void f76F70(int n, uint8_t *cards);    /* D: shuffle cards[0..n): copy to a 160-byte local, then n times take index rand*n >> 15 (unsigned) and close the gap. cards: a data table (DP) or a caller's stack buffer (0x6BCA1), hence a C pointer. n > 160 overflows the local (D 0x76F7D: sub esp,0xB4) */
void f77184(int n, const uint8_t *src, int len, uint8_t *dst);  /* D: insert src[0..n) in front of dst: dst[i+n] = dst[i] for i = len-1 .. 0, then dst[i] = src[i] for i < n. Pointers as f76F70 */
int  f78AE4(void);                     /* D: game_rand: [0xCAC5C] = [0xCAC5C] * 0x15A4E35 + 1, returns (seed >> 16) & 0x7FFF */
void f12B5C(void);                     /* D: memset(0xE31B8, 0, 0x2B65), then byte 0xE31B8 + 69*r = r for r 0..160 */
void f12BB8(void);                     /* D: the 160 three-byte records at 0xE2C18: +2 &= ~1, &= ~2, +0 = 0, +2 &= 3 */
void f45D30(void);                     /* D: [0xF4898] = tokens by player count, initialise nation records 1..13 (layout above) */
void f12C38(void);                     /* D: the play-by-email start (only caller main 0x1187F, PBEM branch: dropped in the port): put each player's first token on its start territory (area, nation fields above) */
void f7B854(const char *name, int seq);  /* D: play music: if [0xF625D] and name == [0xF61D8] and seq == [0xF6258]: restart the sequence unless playing (status 4); else end it, load name from ACIV.CT5 (or the file when cat_size gives -2) unless already loaded, AIL_init_sequence(seq) ("%s-%s\n" and quit on failure), volume, start, loop forever. Callers pass data strings (0xCAB1C, the music table's names) and ignore eax. The port plays its own sound: only the data writes above are translated (G: port design) */

/* ---- Port entry points ----------------------------------------------------------------------------------------- */

void game_continue_load(void);           /* main 0x115B0..0x115C4 (before the setjmp(0xF58A0) at 0x115CE) */
int  game_continue_rest(uint32_t seed_override, int use_seed);   /* main 0x115D3..0x1184A */
int game_continue(uint32_t seed_override, int use_seed);  /* main 0x115B0..0x1184A, shared by a new game (game_load's tail) and a restore (after title_restore): returns -1 for a restored game at 0x1177C (main then goes on at 0x1188C), else as game_load */
int game_load(uint32_t seed_override, int use_seed);  /* main 0x114CF..0x1184A for a new game: the "Loading game data and maps" screen, f12394, f3D170, f11A8C, setjmp(0xF58A0), the minimap sprite, [0xF40EC], the map variant 0xF6110..0xF6114, the map buffer [0xF52C4] (map&pal.gif, f12B00, f3EED8, f44974), 0x128F0, 0x7784C, the button enables (0x3BFBC), 0x7892C, the seed (_dos_gettime into 0xF422C..0xF422F and the clock seed at 0x11789..0x117DA, or seed_override when use_seed; also int16 [0xF40FC] = 1 and [0x110F74] = 0 there), f6BA20, f12B5C, f12BB8, f45D30, memset(0xEE9F0, 0, 0x384), memset(0xF0684, 0, 0x14), [0xF408C] = 0, [0xF4230] = 0, the music (f7B854 if [0xF097C] & 2). Stops before main's call of 0x2EFA0 (0x1184F). 0 ok (G: port design) */
/* port: the heap blocks the original keeps pointers to in 32-bit data fields (load.c). A field holds a handle,
   0x80000000 | slot << 20 | byte offset, so "block + offset" arithmetic on the field works as on the pointer. */
uint32_t load_heap_alloc(uint32_t size);              /* 0 if out of memory or size 0 (Watcom malloc) */
uint8_t *load_heap_ptr(uint32_t handle, uint32_t n);  /* n bytes at the handle, or NULL */
void     load_heap_free(uint32_t handle);
int ds_dump(const char *path);         /* port: write the data object, DS_SIZE bytes from link address DS_BASE, to the file path; 0 ok, -1 on error */

/* ---- Nation choice, human players (module N1 of MAP_PLAN.md: 0x2EFA0 and its helpers) --------------------------

   Main calls 0x2EFA0 at 0x1184F, after the loading step above, and it returns to 0x11854 (the data-compare point).
   Data the choice uses (D, read in the code at the addresses given; obj2 = the initial data in disasm/le/obj2.bin):
     0xC0794  10 records of 14 bytes, record v at 0xC0794 + 14*v (lea 7v, then *2: 0x2F111): int32 count, uint8
              nations[10]. nations[0] is 0 in all ten and is dropped after the copy (0xF7B00). obj2, nations after the
              0: v0 1-4; v1, v2 1-5; v3 6-9; v4 1-6; v5 1, 5-9; v6, v7, v8 1-9; v9 1-7, 9 (count = these + 1)
     0xF7C78  int32 v (0x2F09B..0x2F10B): [0xF40F0] - 2; if [0xF40F0] >= 6: + 2, then 9 if bit 0x02 of word
              [0xE1624] (shl 0xE, shr ax 0xF) is set and [0xF40F0] == 6; else if [0xF40F0] >= 4: + 1 if that bit is
              clear, + 1 if [0xF40F0] == 5. (Bit 0x02: set by START GAME when the total > 3, ui.h.)
     0xF7B00  14 bytes: record v copied (3 movsd + movsw, 0x2F128); then byte 0xF7B04[i] = 0xF7B05[i] for i < count - 1
              and count - 1 (0x2F13D..0x2F172): the list without its 0 (the last old byte stays). 0x2F9F8 reads the
              count and int8 0xF7B04[i] (dword [i + 0xF7B01] sar 24, 0x2FAD6)
     0xF7C9C  int32 count, 0xF7CA0 int32[9]: the candidates, 0xF7CA0[i] = int8 0xF7B04[i] for i < [0xF7B00] (0x2F1C7..
              0x2F1E0: [4*(count+1) + 0xF7C9C]). SELECT removes entry idx: shift down, count - 1 (0x2F7EF..0x2F864). Other
              code uses the same list and clamp (0x79A9C callers 0x24E59, 0x28CAA, 0x2C13F, 0x5CCE3; 0x61E62 reads
              0xF7CA0; G: a general choice list)
     0xF7C00  uint32 heap block: the zoom map GIF, name [0xC1A38 + 4*[0xF40F0]] (zoom3/4/5/6.gif, zoom78.gif for 7 and
              8; obj2), from ACIV.CT1 [0xF40D4]; 0xF7C04 uint16 its size (cat_size's low word, 0x2FA29)
     0xF7C08  uint32[9] heap blocks: candidate i's "<ch_ name>.gif" (sprintf "%s.gif" of [0xC1494 + 4n], 0x2FAE6);
              0xF7C2C uint16[9] their sizes
     0xF7C40  uint32[9] heap blocks: candidate i's "<cv_ name>.gif" ([0xC14BC + 4n], 0x2FB86); 0xF7C64 uint16[9] sizes
              0x2F9F8 zeroes all 9 slots and sizes first. Slot i = candidate i: SELECT frees slot idx (0x2F7C7, 0x2F7D6),
              shifts slots and sizes with the list, zeroes slot [count] (0x2F879, 0x2F880); the end frees and zeroes
              the map and every non-zero slot (0x2F93E..0x2F9BA). So 0xF7C00, 0xF7C08[9] and 0xF7C40[9] are 0 again at
              0x11854 (D); the size words keep their values.
     0xC1494  uint32[10] -> the ch_ picture names, 0xC14BC uint32[10] -> the cv_ names (literals 0xB39CB.. and
              0xB3A1A..; [0] = ""). 0x2FC04 strcpy's INTO the literals of ch_ slots 1, 2, 7, 9 and cv_ slots 1, 2, 3, 5,
              7, 9 (inline copy loops; each new name fits the one it replaces): writes in g_ds, DP(D32(0xC1498)) etc.
     0xC14E4  int16[10][4] {x, y, w, h}: nation n's card rectangle at 0xC14E4 + 8n (read as dword [8n + 0xC14E2],
              [+0xC14E4], [+0xC14E6], [+0xC14E8] sar 16, 0x2F2D1..0x2F428). Card drawn at (x + 8, y + 6) (the map
              margin), copied with fg_vbtccopy(x+8, x+8+w-1, y+6, y+6+h-1, x+8, y+6+h-1, [0xF52CC], [0xF52C8]).
              0x2FC04 rewrites n = 1 (0xC14EC..0xC14F2), 2 (0xC14F4..), 7 (0xC151C..), 9 (0xC152C..0xC1532)
     0xC076C, 0xC0780  int16[10] x, y of nation n's 35x35 token, sprite 0xF4A5C + 8n, drawn for n 1..9 with nation
              byte +0x9A != 0 (0x2F459..0x2F48F, 0x2F8CA..0x2F900)
     0xC768C  uint32[] -> nation names ([0] "", 1 "AFRICA" .. 9 "EGYPT"), centred in the sunken box (0x109, 0x185,
              0x6E, 0x13) at y 0x189 (f3A0B0, 0x2F4DB)
     0xC08AC  uint32[9] -> "Zero" .. "Eight", the %s of "Player %s choose your nation" (0xB378D, 0x2F25F)
     Player records 0xF40E4 + 32p, 1-based: SELECT sets byte +0 = the nation for p = 1..[0xE1620] (0x2F72A; G: the
              computer players follow, 0x302C4). p = 0 would be 0xF40E4 itself, ACIV.CT5's handle; 0x12C38's
              [0xF4104 + 32*p] is the same record with p from 0.
     Nation byte +0x9A (0xF1566 + 199n): SELECT writes 3 (human, 0x2F7B6); != 0 counts as taken (token drawn)
     Dialog 0xC08D0 (obj2, 4 records): 0x39 left arrow (251,389, 13x19, style 2), 0x3A right arrow (376,389, 13x19,
              style 2), 0x58 SELECT (251,409, 138x19), 3 AST (251,429, 138x19); the arrows' pictures are the sprites
              0xF51AC / 0xF51B4 drawn at (0xFB, 0x185) / (0x178, 0x185) (0x2F178, 0x2F18C). 0x2EFA0 hit-tests it by
              hand (0x2F525..0x2F59B): int16 count [0xC08D0], enabled byte == 1, x <= [0xE1628] < x + w, y <=
              [0xE162C] < y + h; then f3C5D4(0xC08D0, id) and f300E0(id)
     Also written by 0x2EFA0: int16 0xBB3FA, 0xBB3FC, 0xBB3FE, 0xBB400 = 169, 340, 300, 35 (the message area, 0x2EFD1);
              int8 0xEE168 = 6 (0x2F017; G: the picture shown); int8 0xF52D0 = 0 (0x2F34F: the background buffer no
              longer holds a clean picture, so f3CD6C reloads it)
   Heap pointers the choice keeps in 32-bit data fields (port handles: load_heap_alloc / load_heap_ptr / load_heap_free
   as gfxload.c; ui_sprite_put / ui_sprite_pixels for sprite records):
     0xF7C00, 0xF7C08[9], 0xF7C40[9] (malloc 0x2FA34, 0x2FB25, 0x2FBC6; free above). 0 at 0x11854, as said.
     +0 of 0x2EFA0's local sprite record [ebp-0x34] (8 bytes on its stack): f3AE90 0x2F05B (the message area of
       [0xF52CC]), f3AF8C 0x2F29F, f3AF54 0x2F932. The ui.h routines take link addresses, so the port needs the record
       at one (G: port design, e.g. 8 bytes of the stack range 0x121C00..0x122BFF that by_design.txt excludes), with
       +0 cleared before f3AE90: a stale value of 1..UI_SPR_MAX would make ui_sprite_put reuse another record's slot.
     [0xF522C] (read only, 0x2F91B and 0x30116): ui_screen.c's ui_heap_F522C (extern in title.c, not in ui.h).
   Pictures from memory: the original sets a block with 0x8240F and calls fg_showgif(an empty string: 0xB378C, 0xB37AA,
   0xB37AB, 0xB37AC, 0xB37AD, 7): flags 4 = from the fg_imagebuf buffer, 2 = at the cursor (fg_move before), 1 = keep
   the palette. In the port, ui.h's f8240F only writes the size into the data object, and fg.c's fg_showgif reads the
   buffer fg.h's fg_imagebuf was given. So where the original calls 0x8240F with a block, the translation calls
   fg_imagebuf(load_heap_ptr(block, size), size) (and f8240F for the data fields, G); at 0x2F91B and 0x30116 both get
   ui_heap_F522C, 0xFF00, so fg.c keeps no pointer to a freed card block.
   Runtime calls (Watcom C runtime -> standard C): 0x7DB3D sprintf; 0x822B5 malloc (load_heap_alloc for these fields);
   0x877E6 free (load_heap_free; for a sprite record free(ui_sprite_pixels(spr))); 0x8A080 clock (Watcom: hundredths of
   a second since start-up, CLOCKS_PER_SEC 100, LIBRARY_CALLS.md) -> clock() converted to hundredths (clock() * 100 /
   CLOCKS_PER_SEC, G). A headless run must not wait in f78A20 (MAP_PLAN.md risk 3; fg.c's fg_waitfor returns at once
   when headless). Catalog: cat_size 0xA24A7, cat_load 0xA25B3 (fg.h).
   0x136B0 (the AST chart, ast.c) and 0x302C4 (the computer nations, nations.c) are ported: modules N2 and N3. */

void f2EFA0(void);                     /* D: the nation choice (choose.gif, f3CD6C(6)): the message area saved in a local sprite, list v and the candidates (above), f2FC04, f2F9F8, the zoom map from memory at (8, 6); for p = 1..[0xE1620] "Player %s choose your nation" (f3AB7C) and the loop over the candidates (arrows -9/-10 move idx with the cv_ card drawn back, SELECT -16 takes it, -100 redraws); then the taken tokens, f3CC14 and f78A20(750) if there are humans; clean-up; with [0xE1621] > 0: f3CC14, f3C940(0xC08D0), f302C4, f3C7A0(0xC08D0). Its caller main 0x1184F ignores eax (0x6B9AC takes no argument) */
void f2F9F8(void);                     /* D: load from ACIV.CT1 [0xF40D4] the zoom map into [0xF7C00] (size word 0xF7C04), zero the 9 slots of 0xF7C08/0xF7C2C and of 0xF7C40/0xF7C64, then for i < [0xF7B00] the ch_ GIFs, then the cv_ GIFs (cat_size, malloc of the low word, cat_load). One caller (0x2F1E7) */
void f2FC04(void);                     /* D: the picture names and card rectangles for [0xF40F0] players (strcpy into the literals, int16 writes; above): 3 players af3, italy, ill3, cret3, else af48, ibera, illy, crete; 4: cv_cret3; 5 or 6: ass56, else ass78; 6: egy6, else egy78. One caller (0x2F12D) */
int  f300E0(int id);                   /* D: the choice dialog's button id -> code (16-bit unsigned compares): 0x39 -> -9, 0x3A -> -10, 0x58 -> -16; 3: f8240F([0xF522C], 0xFF00), f136B0(1), f3CC14, f3CD6C(6), -100; else -100. id = the record's int16 +0x0E (sar 16) */
void f3AB7C(const char *text);         /* D: message box sized to its text, centred in the area 0xBB3FA..0xBB400 (w - 10 for the wrapping, align 2 = centred in both f3A42C calls), in the back buffer [0xF52C8] (fg_vbopen/vbclose): fill 13, boxes 4 (and at inset 2) and 14 (inset 1), text colour 5. Callers pass caller stack buffers (0x2F2A9) and data strings (0x2B220), so a C pointer; none reads eax (12 call sites) */
void f3A9F4(const char *text);         /* D: as f3AB7C with align 0 (left) in both f3A42C calls. Callers pass data strings (0xF5500, 0x1D8EC) and stack buffers; eax unused after the 34 call sites (a few are tail positions, G) */
void f3AF54(uint32_t spr);             /* D: if word [spr+4] != 0: free([spr]) (0x3AF7A), word [spr+4] = 0; +0 and +6 untouched. Callers: 0x121A0's 28 records in 0xF48A4..0xF522F (by_design.txt) and 0x2EFA0's stack record. Port (G): free(ui_sprite_pixels(spr)), ui_sprite_put(spr, NULL) and +0 = 0, so the freed slot cannot alias two records later; the original's dangling +0 is a by-design byte range anyway */
void f79A9C(int32_t *p);               /* D: clamp *p: if [0xF7C9C] <= *p then *p = [0xF7C9C] - 1; then if *p < 0, *p = 0 (signed). Unusual prologue (push 0x10, __CHK, push edx, push ebp). All 9 callers pass a caller's stack int32 (lea; 0x62835 forwards one), hence a C pointer; void (no caller reads eax; 0x62835 is a tail position, G) */
void f78A20(int t);                    /* D: busy wait: start = clock(); loop while (unsigned) start + (t / [0xBB3F4]) / 10 > clock() (signed idiv twice, 0x78A49, 0x78A58). [0xBB3F4] = 1 (obj2), 10 with the /C switch (0x10494, "Continuous testing", OVERVIEW §6). So t ms in Watcom's hundredths and the DOS clock's ~55 ms steps. eax at the return is clock()'s; no caller reads it (89 call sites; 6 tail positions, G) */
void f3C940(uint32_t dlg);             /* D: disable a dialog, saving its state: mouse hidden (f78884); for i < int16 [dlg]: int8 0xF8E44[i] = byte [dlg + 20i + 0x12], that byte = 0, redraw record i (f3B4E0(dlg + 2 + 20i), active surface); f788AC. dlg + 20i + 0x12 is record i's +0x10, the ENABLED byte (not +0x12 pressed as INVENTORY_2EFA0.json says; ui.h's name UI_SAVED_PRESSED notwithstanding) */
void f3C7A0(uint32_t dlg);             /* D: undo f3C940: fg_vbopen([0xF52C8]); for i < int16 [dlg]: byte [dlg + 20i + 0x12] (record i's enabled byte) = 0xF8E44[i], f3B4E0(dlg + 2 + 20i); fg_vbclose. No mouse hiding */
_Noreturn void not_ported(uint32_t addr);  /* port (choose.c): prints "advciv: 0xADDR is not ported yet" (addr in hex, e.g. 0x136B0) and ends the program through plat_shutdown and exit(3); call sites of routines not yet translated call it (dispatch.c's cases) */

/* ---- AST chart, button kit, click wait, dispatcher, score helpers (module N2 of MAP_PLAN.md) --------------------

   The AST chart 0x136B0 (D unless marked). f136B0(mode), mode in eax. Callers: 0x3012F mode 1 (the nation choice's
   AST button, choose.c), 0x3FBDA mode 2 (dispatcher id 3), 0x55202 mode 3 (G: the AST phase; the only caller that
   reads eax). It draws into the background buffer: f3CD6C(4) (astblan2.gif), fg_vbopen([0xF52CC]) until the
   fg_vbclose at 0x13B99 / 0x13CA7, font 1 (f39F90(1); n starts as ecx = 1, kept across that call, 0x136F5).
   For n = 1..9, rec = 0xF14CC + 199n: mode 1 takes every n, the other modes only n with byte +0x9A != 0 (0x13AE4).
     Name shown: in mode 1 only if n is in the choice list (0xF7CA0[i] == n for some i < [0xF7C9C], 0x13703); in the
     other modes always. fg_setcolor(0x13) first (0x13739; nothing is drawn in it), then the name [0xC768C + 4n]
     centred (f3A0B0, width 0x35) with y = 0x86 + 25(n-1): colour 5 at (0x48, y + 1), colour 15 at (0x47, y); for
     n == 3 (ILLYRIA) one more colour-5 copy at (0x48, y) before those (0x1375D).
     Mode 1 stops there for n. The other modes evaluate n's AST step (0x13804..0x13A49), pos = int8 rec+0x5C:
       AST table 0xBCFDA: int16 pairs {era, need}, 17 per nation (68 bytes), nation n at 0xBCFDA + 68(n-1), entry k
       at +4k (lea/shl: (n-1)*17*4). cur = entry[pos].era (0x1381B), next = entry[pos+1].era (0x13841, 0xBCFDE),
       need = entry[pos+1].need (0x13979, 0xBCFE0). obj2: 4 or 5 squares of era 0 first, era 1..3 squares carry
       need 2, 3, 4 (never read: need is used only for era 4), then 2 or 3 squares of era 4 with need 1200..1900 and
       entry 16 = {5, 0}. E.g. AFRICA: 5 x 0, 3 x 1, 3 x 2, 3 x 3, {4, 1300}, {4, 1600}, {5, 0}.
       Scores (int16 locals): city (c = int8 rec+0x51, tested in this order): c == 0: 0; c == 1: 1; c >= 2 and
         next == 1: 2; c >= 3 and next == 2: 3; c >= 4 and next == 3: 4; c >= 5: 5; else 0 (0x1384C..0x138D9).
         groups: g = f1486C(n); 4 if g == 5 and next >= 3, else 3 if g >= 3, else 0 (0x138E0..0x1390B).
         advances: 4 if at least 9 of the 24 bytes rec+0x24.. are non-zero, else 0 (0x13912..0x13943).
       int16 rec+0x5E = f17760(n) (0x13957; modes 2 and 3).
       step: -1 if c < 1 and cur != 0 (0x1399A); else +1 if next == 5 and city >= 5 (also won = 1, 0x139C0); next
         == 4 and int16 rec+0x5E >= need (16-bit signed compare, 0x139DB) and city >= 5; next == 3 and city +
         advances + groups >= 12; next == 2 and city + groups >= 6; next == 1 and city >= 2; next == 0; else 0.
       Mode 3 applies it: byte rec+0x5C + 1 (0x13A60) or - 1 (0x13A70). Modes 2 and 3 then draw the token sprite
       0xF4ACC + 8n at (0x83 + 26*pos, 0x80 + 25(n-1)) with pos read after that (f3AF8C, 0x13AB5).
       G: entry 16 is the last square; a nation standing on it would read entry 17, the next nation's entry 0 (or
       past the table for n = 9). Reaching it sets won, which mode 3 returns (G: the game then ends).
   Modes 1 and 2 (0x13B20..0x13BC2): f3BB48(0xBB800, 0x2A, 0x109) and f3BC64(0xBB800, 0x2A, 0x192) move CONTINUE to
     (265, 402) (obj2: (265, 450)); sprite 0xF521C at (0x106, 0x18F); f3B4E0 for each record of 0xBB800 (count
     dword [0xBB7FE] sar 16); fg_vbclose; f3CC68; then r = f78BA4(0xBB800, 0) until r == -1 (every other value loops).
     Returns 0 (0x13D31). The moved CONTINUE stays in 0xBB800 (the data compare at 0x11854 sees it).
   Mode 3 (0x13BC7..0x13D16): t = int16 [0xF40FC] (dword [0xF40FA] sar 16), at most 44; for n = 1..9 with byte
     +0x9A != 0: int16 [0xEE9F0 + 90n + 2t] = f147C4(n) (0x13C24); then int16 [0xF40FC] + 1 (0x13C45); sprites
     0xF521C at (0x106, 0x180) and 0xF5224 at (0xC2, 0x19D); f3B4E0 for each record of 0xBC8F0; fg_vbclose; [0xF8E78]
     = 0xBC8F0 (0x13CAC); f3CC68; fg_mousepos(0xE1628, 0xE162C, 0xE1630); loop: if f7BA64() == 0, r = f78BA4(0xBC8F0,
     0), else f78A20(int16 [0xF40BA] == 0 ? 250 : 2000) and r = -1; until r == -1. Returns won == 1 (sete, 0x13D24).
   Data the chart uses:
     0xBB800  dialog, 1 record (obj2): 0x2A CONTINUE (265, 450, 110 x 19), style 1, font 1
     0xBC8F0  dialog, 6 records (obj2), y 416, 49 x 19: 5 SYSTEM (197), 8 MAP (246), 4 CENSUS (295), 6 TOOLS (344),
              7 GOODS (393); 0x2A FINISHED (265, 387, 110 x 19). f78BA4 hands their ids to f3FB94, so mode 3 opens
              those screens from the chart (G: the AST phase's menu)
     0xEE9F0  int16[10][45] score history, row n, column min(turn, 44) (memset 0x384 by game_load; also written at
              0x44DF2 and read at 0x44E49, 0x453B1, 0x6B961; G: the scoring chart, MENU2)
     0xF40FC  int16 (G: the turn; game_load sets 1 at 0x117CB, 0x53183 writes it too)
     0xC768C  uint32[10] -> nation names (N1 above); 0xC197C[4] -> "astblan2.gif" (obj2)
     Sprites (records of 0x11A8C): 0xF4ACC + 8n (tokens.gif, 24 x 23), 0xF521C and 0xF5224 (c_invld.gif pieces).

   Nation record fields the scoring reads (0xF14CC + 199n; bytes read signed, as dword [..-3] sar 24, D):
     +0x01..+0x12  int8 c_i  trade cards of card id i held (f6DD98: sum of c_i * c_i * ((i + 1) >> 1) over i 1..18;
                   card ids 2s+1 and 2s+2 belong to stack s (0x6BA20), so (i + 1) >> 1 = s + 1, G: the face value)
     +0x24..+0x3B  int8[24]  advances: f17760 counts byte == 1, f1486C byte > 0, f136B0's count byte != 0
     +0x3D  int8   the nation's own number (0x45D30 writes n); f147C4 passes it to f6DD98
     +0x3E  int8   added to the score as it is (G: the treasury)
     +0x51  int8   cities (G): 50 points each in f147C4, the city score of f136B0
     +0x5C  int8   AST square 0..16: 100 points each in f147C4; f136B0 mode 3 moves it by the step
     +0x5E  int16  advance points = f17760(n), written by f136B0 (0x13957) and 0x17635 (0xF152A + 199n)
     +0x9A  int8   0 = not in play (N1: 3 human; G: 2 computer)
   0xBD480  int16[24] advance costs (obj2): 45 45 80 110 140 180 140 80 110 140 230 60 60 120 110 170 200 180 240 50 80
            150 220 250 (read as dword [0xBD47E + 2i] sar 16; also by 0x16A29..0x17101)
   0xF62A0..0xF62A4  uint8 flags of f1486C's five advance groups. f1486C clears them, then each group counts once (the
            first owned advance ends its loop) and sets its flag: advances 0..6 -> 0xF62A0; 6..10 -> 0xF62A1; if
            advance 19 (+0x37) is owned: count, 0xF62A1 again, and 10..14 is skipped (0x1492F); else 10..14 -> 0xF62A2;
            14..18 -> 0xF62A3; 19..23 -> 0xF62A4. Advances 6, 10 and 14 sit in two groups and advance 19 can count
            twice (D). No reader of 0xF62A0..0xF62A4 exists in the code (direct or indexed), so the 0xF62A1 written for
            the third group changes nothing.

   Self-running demo (f7BA64): int16 [0xF40BA] = the self-running demo's player count (/SRD, 0x1034D after "%d player
   Self Running Demo in effect."; also main 0x11110); int16 [0xF40B8]: the only store is f7BA64's clear (0x7BAC9),
   G: always 0. f7BA64 reads them as dword [0xF40B6] / [0xF40B8] sar 16.

   The button dispatcher 0x3FB94 (D). f3FB94(id), id in eax, and the record's int16 +0x0E sign-extended at every
   call site (0x78C9A: [rec+0xC] sar 16). Callers: f78BA4 (0x78C9A), the map click 0x3D434 (0x3D4F5), the text
   input 0x60694 (0x60A3D), 0x166C1, 0x29B5F, and 0x14294 (0x147B2, a tail position); all use eax.
     code = -100 (int32 [ebp-0x3C], 0x3FBAC); index = (id - 3) as uint16 (cmp ax, 0x104, 0x3FBB9): above 0x104 ->
     0x426C1, which returns code; else jmp cs:[4*index + 0x3F780] (0x3FBC8).
     Jump table 0x3F780..0x3FB93: 261 dwords for ids 3..0x107, in the code object (obj1.bin offset 0x2F780) right
     before the function, each entry a fixup (fixup 0x3FBCC -> 0x3F780 for the jmp). Default 0x426C1 (-100) for ids
     0x11, 0x1A, 0x54, 0x55, 0x59..0x68, 0xAC..0xAF, 0xEB, 0x102, 0x104..0x106 (and every id outside 3..0x107).
     Entries with an action (other modules; [] = what they call):
       3 (0x3FBD0): f3CCBC, f136B0(2), f3CC14, -100      4 (0x3FBF1): f3CCBC, 0x13E50, f3CC14, -100
       5 (0x3FC0D): 0x42754, -100    6 (0x3FC27): 0x19C10, -100    7 (0x3FC39): 0x76A9C, -100    8 (0x3FC4B): 0x44A34, -100
       9..0x10 (0x3FF87 + 0x50k): the player buttons, nation int16 [0xF06B0 + 2k] (read as dword [0xF06AE + 2k] sar
         16; 0x459E4, 0x3D9FC; 0x3E374 with
         int16 [0x107690] != 0); 0x1B..0x22 (0x402A7..0x4035C): scroll (0x3D52C); 0x23 (0x40375) mini-map click;
         0x24 (0x403C8) detail level byte [0xC1A34]; 0x25 (0x406E0) ZOOM; 0x26 (0x407DC), 0x27 (0x40B38) land/sea;
         0x28 (0x40C4A) the move; 0x2B..0x32 (0x40F80, 0x41223, 0x4152E, 0x417A5, 0x40E45, 0x410AE, 0x41380, 0x416C6)
         ships; 0xC4..0xC8 (0x40858, 0x408E9, 0x4097C, 0x40A10, 0x40AA4) amount radio (f3B9A4 != 0 -> 0x426C1);
         0x4F (0x3FDAB) save, -1; 0x50 (0x3FEA0) restore (0x5FF28 -15 -> -1, else longjmp 0xF58A0); 0x51 (0x3FDCF)
         new game (longjmp 0xF58D4); 0x52 (0x3FD26) 0x436FC, -1; 0x53 (0x3FD6D) 0x4290C, -1; 0x56 (0x3FD07) 0x44D5C,
         -1; 0x57 (0x3FD91) quit ([0xF5500] = "", f44CAC); 0xE2 (0x3FD49) 0x43E88, -1; 0x101 (0x4041D) replay;
         0x103 (0x3FC5D) Trading Wrapup (0x763FC, "junk" GIF), -100; 0x107 (0x3FCE0) 0x545AC (PBEM), -1.
       0x8E..0xA5 (0x41CB7, then 0x41CD4 + 0x20(k-1)): int32 [0xF6758] = id - 0x8E (0..23), return -0x3E.
     Entries that only return a constant (mov [ebp-0x3C], imm, D; 0x426BA falls into 0x426C1):
       0x29 -0x79   0x2A -1   0x33 -8   0x34, 0x36 -5   0x35, 0x37 -6   0x38 -7   0x39..0x3E -9..-0xE   0x3F -0x65
       0x40 -0x66   0x41 -0x63   0x42..0x4D 0..11   0x4E -0xF   0x58 -0x10   0x69..0x6F -0x7B..-0x81
       0x70..0x74 -0x11..-0x15   0x75 -0x1D   0x76 -0x1C   0x77 -0x24   0x78 -0x1E   0x79 -0x20   0x7A -0x22
       0x7B -0x1F   0x7C -0x21   0x7D -0x23   0x7E -0x25   0x7F -0x7A   0x80..0x84 -0x26..-0x2A   0x85 -0x2B
       0x86 -0x2C   0x87..0x8D -0x2F..-0x35   0xA6 -0x3F   0xA7 -0x40   0xA8 -0x41   0xA9 -0x42   0xAA -0x43
       0xAB -0x44   0xB0 -0x58   0xB1 -0x59   0xB2..0xB9 -0x50..-0x57   0xBA..0xC1 -0x48..-0x4F   0xC2 -0x5A
       0xC3 -0x5B   0xC9 -0x5E   0xCA -0x61   0xCB -0x62   0xCC..0xD9 -0x67..-0x74   0xDA -0x75
       0xDB..0xDD -0x76..-0x78   0xDE..0xE1 -0x83..-0x86   0xE3..0xE8 -0x87..-0x8C   0xE9 -0xAC   0xEA -0x95
       0xEC..0x100 -0x97..-0xAB
     Port (MAP_PLAN N2): ids 3, 0x2A and the constant entries; the other action entries call not_ported(the entry's
     address) until their modules come.

   Heap pointers in data fields: these routines keep none of their own. They read the sprite records' +0 handles
   (0xF4ACC + 8n, 0xF521C, 0xF5224, through f3AF8C) and [0xF522C] (through f3CD6C). f3C284 stores a text address in a
   button record's +0x08: data strings, or 0x109E7C (0x3FB94's sprintf buffer), but two callers pass caller stack
   buffers (0x516EE lea ebx, [ebp-0x280]; 0x54128 lea ebx, [ebp-0xA4]), which the port has to place at link
   addresses (G: port design, as 0x2EFA0's sprite record). 0x3FB94's cases 0x50 and 0x51 free the heap fields
   0xF0984[1..11] and [0xEED74], [0xEED78], [0xEED7C], [0xEED80] (load_heap_free, when ported). */

int  f136B0(int mode);                 /* D: the AST chart (layout above): mode 1 from the nation choice (choosable names only, CONTINUE), 2 from the dispatcher (names, steps evaluated but not applied, tokens; CONTINUE), 3 the AST phase (steps applied, score history 0xEE9F0, turn + 1, dialog 0xBC8F0 until FINISHED / unattended). Returns 0 in modes 1 and 2; in mode 3, 1 if a nation reached the last square, else 0. 3 callers; 0x55202 reads eax */
int  f147C4(int n);                    /* D: nation n's score: int16 +0x5E + f6DD98(int8 +0x3D) + int8 +0x3E + 100 * int8 +0x5C + 50 * int8 +0x51 (int32 sum). 5 callers, all read eax */
int  f1486C(int n);                    /* D: how many of the five advance groups nation n has an advance in (0..5; advance 19 may count twice), with the flags 0xF62A0..0xF62A4 (layout above). One caller (0x138E3) */
int  f17760(int n);                    /* D: nation n's advance points: sum of int16 0xBD480[i] over i 0..23 with int8 +0x24+i == 1. 3 callers read eax (0x13957 and 0x17635 store ax into +0x5E) */
int  f6DD98(int n);                    /* D: value of nation n's trade cards: sum over i 1..18 of c*c*((i+1) >> 1), c = int8 [0xF14CC + 199n + i]. 6 call sites, all read eax; 0x69B59 and 0x6A28E pass n = 0 (record 0) */
int  f7BA64(void);                     /* D: unattended check: 0 if int16 [0xF40B8] and [0xF40BA] are both 0; else fg_intkey(0xE1634, 0xE1635): Esc with [0xF40BA] != 0 quits ([0xF5500] = "", f44CAC), Esc otherwise clears [0xF40B8] and returns 0, any other key: f78508 (flush: fg_intkey until key 0, D 0x7851B); then fg_mousepos(0xE1628, 0xE162C, 0xE1630): a button down returns 0, none 1. 20 callers read eax */
int  f78BA4(uint32_t dlg, int map);    /* D: the generic click wait: f775F0, then fg_mousepos until [0xE1630] != 0; the first record of dlg with enabled byte == 1 and x <= [0xE1628] < x + w, y <= [0xE162C] < y + h: f3C5D4(dlg, id), return f3FB94(id); otherwise, if map == 1 and [0xE162C] <= 0x145: t = f3D2A4(x, y) (MAP1, not ported: not_ported(0x3D2A4)), return the first i < [0xF7C9C] with int32 0xF7CA0[i] == t; else -100. dlg is an immediate at 69 of the 70 call sites (0x710BB: a local holding one); all read eax */
void f3BB48(uint32_t dlg, int id, int x);   /* D: x (int16 rec+0x00) of the button id = x (16-bit id compare, first match), no redraw; "Button not found" (0xB3C8B, f3A7B0) if none. 50 call sites, none reads eax */
void f3BD80(uint32_t dlg, int id, int w);   /* D: w (int16 rec+0x04) of the button id; "Button not found" (0xB3CCF) if none. 12 call sites, eax unused */
void f3BE10(uint32_t dlg, int id, int h);   /* D: h (int16 rec+0x06) of the button id; "Button not found" (0xB3CE0) if none. 2 call sites: 0x141F7, and 0x14283 in a tail position of 0x13E50, whose only caller (dispatcher id 4) ignores eax */
void f3C0D8(uint32_t dlg, int id, int style);  /* D: style byte rec+0x11 = (int8)style (byte store, 0x3C15F) of the button id; "Button not found" (0xB3D35) if none. 34 call sites, eax unused (0x74EB9 writes ah next) */
void f3C284(uint32_t dlg, int id, uint32_t label);  /* D: label (uint32 rec+0x08) of the button id = label, a link address (data strings and 0x109E7C; two callers pass stack buffers, see above); "Button not found" (0xB3D68) if none; no redraw. 51 call sites, eax unused */
void f3B7CC(uint32_t dlg, int id);     /* D: draw the button id pressed (f3B2D8, which sets rec+0x12 = 1) in the back buffer: fg_vbopen([0xF52C8]) .. fg_vbclose; "Button not found" (0xB3C25) if none. 14 call sites, eax unused (0x4080C and 0x40B6A write al / ah next) */
void f3B870(uint32_t dlg, int id);     /* D: draw the button id up (f3B0D4, rec+0x12 = 0) in the back buffer; "Button not found" (0xB3C36) if none. 37 call sites, eax unused */
int  f3B9A4(uint32_t dlg, int id);     /* D: the pressed byte rec+0x12 of the button id, sign-extended (dword [dlg + 20i + 0x11] sar 24, 0x3BA24); "Button not found" (0xB3C58) and 0 if none. 8 call sites, all read eax */
void f3C828(uint32_t dlg);             /* D: disable a dialog in the back buffer, saving its state: fg_vbopen([0xF52C8]); for i < int16 [dlg]: int8 0xF8E44[i] = byte [dlg + 20i + 0x12] (record i's +0x10, the ENABLED byte), that byte = 0, f3B4E0(dlg + 2 + 20i); fg_vbclose. The back-buffer twin of f3C940 (no mouse hiding). 6 call sites, eax unused */
void f3C8C4(uint32_t dlg);             /* D: undo f3C828 / f3C940 on the active surface: f78884; for i < int16 [dlg]: byte [dlg + 20i + 0x12] (enabled) = 0xF8E44[i], f3B4E0(dlg + 2 + 20i); f788AC. No fg_vbopen (unlike f3C7A0). 13 call sites, eax unused; 0x2C76A and 0x2D0FB are tail positions of 0x2C298 / 0x2C894, whose callers end in tail positions too (G) */
void f3A148(int x, int y, int w, int value);  /* D: value centred: itoa(value, 20-byte stack buffer, 10) (0x8C897; standard C: "%d"), f3A0B0(x, y, w, buffer). 7 call sites, eax unused */
void f3A3A0(int x, int y, const char *s);     /* D: s right-aligned, right edge at x, top at y: if strlen(s) > 128 (unsigned, 0x3A3CE) f3A044(x, y, "String too long" 0xB3BAA); else fg_move(x - fgf_width(s, strlen(s)), y + int16 [0xF8B00]), f3AD08(s). Callers pass data strings and stack buffers (0x450AD, 0x501FC, 0x512D3, 0x60989, 0x757B2), hence a C pointer. 40 call sites, eax unused */
int  f3FB94(int id);                   /* D: the button dispatcher: id -> code through the jump table 0x3F780 (layout above), -100 = nothing. 6 call sites, all use eax (0x147B2 as a tail position) */

/* ---- Computer nations (module N3 of docs/MAP_PLAN.md, nations.c) -----------------------------------------------
   Nation byte +0x9A (0xF1566 + 199n): 2 = computer, written by 0x30174. The orders per player count, the jump tables
   and the two BUG(1995) fixes are in nations.c's file comment. */
void f30174(int n);                    /* D: nation n to the computer: byte +0x9A = 2; f78884; its token (sprite 0xF4A5C + 8n at int16 0xC076C[n], 0xC0780[n], f3AF8C); f3C9D0(0x109, 0x185, 0x6E, 0x13, 5); fg_setcolor(5); the name [0xC768C + 4n] centred (f3A0B0(0x109, 0x189, 0x6E)); f788AC; f78A20(750). Called only from 0x302C4 (50 call sites); eax unused (each is followed by mov eax,[rem]) */
void f302C4(void);                     /* D: the computer takes the nations the humans left, by [0xF40F0] = 3..8, with rem = [0xF40F0] - (nations 1..9 with +0x9A == 3), fixed orders, pairs by game_rand*2 >> 15 and triples by a switch on which of them are human (game_rand*3 >> 15); f30174 for each. One caller, 0x2EFA0 at 0x2F9DE (eax unused: 0x2F9E3 mov eax,0xC08D0). BUG(1995) A (rem++ after Africa in every block, Iberia with 4 and 5 players, Asia with 5) and B (unguarded takes in the 7-player (3,4,5) switch) fixed unless g_original */

void f7784C(int n);                    /* D: the player buttons for n players (dialogs 0xF596C and 0x111594); newgame.c, also called by the elimination check f22B00 */
extern void (*g_not_ported_hook)(uint32_t addr);   /* port: called by not_ported before it exits (main.c: --dump-ds there) */

#endif
