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

int game_load(uint32_t seed_override, int use_seed);  /* main 0x114CF..0x1184A for a new game: the "Loading game data and maps" screen, f12394, f3D170, f11A8C, setjmp(0xF58A0), the minimap sprite, [0xF40EC], the map variant 0xF6110..0xF6114, the map buffer [0xF52C4] (map&pal.gif, f12B00, f3EED8, f44974), 0x128F0, 0x7784C, the button enables (0x3BFBC), 0x7892C, the seed (_dos_gettime into 0xF422C..0xF422F and the clock seed at 0x11789..0x117DA, or seed_override when use_seed; also int16 [0xF40FC] = 1 and [0x110F74] = 0 there), f6BA20, f12B5C, f12BB8, f45D30, memset(0xEE9F0, 0, 0x384), memset(0xF0684, 0, 0x14), [0xF408C] = 0, [0xF4230] = 0, the music (f7B854 if [0xF097C] & 2). Stops before main's call of 0x2EFA0 (0x1184F). 0 ok (G: port design) */
/* port: the heap blocks the original keeps pointers to in 32-bit data fields (load.c). A field holds a handle,
   0x80000000 | slot << 20 | byte offset, so "block + offset" arithmetic on the field works as on the pointer. */
uint32_t load_heap_alloc(uint32_t size);              /* 0 if out of memory or size 0 (Watcom malloc) */
uint8_t *load_heap_ptr(uint32_t handle, uint32_t n);  /* n bytes at the handle, or NULL */
void     load_heap_free(uint32_t handle);
int ds_dump(const char *path);         /* port: write the data object, DS_SIZE bytes from link address DS_BASE, to the file path; 0 ok, -1 on error */

#endif
