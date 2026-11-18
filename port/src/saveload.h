/* Save-file reading of AC_MAIN.EXE (module S1 of docs/MAP_PLAN.md, the loading half): 0x5F05C (read a whole save into
   the game globals), 0x7BBF0 (CRC check of a save file), 0x7BB84 (CRC-32 update), 0x7BCD4 (the pre-rolled random
   table) and 0x726DC (trade-offer table rebuild). Translations in saveload.c; the other half (restore and save dialogs,
   0x5FF28, 0x5E7D0, 0x601D8, 0x60694, 0x6016C, 0x3CD10) is declared in save.h. */

#ifndef SAVELOAD_H
#define SAVELOAD_H

#include <stdio.h>
#include <stdint.h>

void     f5F05C(int16_t *version, char *desc, FILE *fp);  /* D: read save file fp (ebx) into the game globals: int16 version -> *version (eax), the 51-byte description -> desc (edx), then the whole state in file order (saveload.c). Frees and re-reads the 11 heap lists [0xF0984 + 4i] (port handles) */
uint32_t f7BBF0(const char *name);                         /* D: CRC check of a save file: CRC-32 (f7BB84, initial 0) over the file after its header (0x4B bytes from version 0x28, else 0x35); 0 = intact (the file ends with the CRC of what precedes it) */
uint32_t f7BB84(int32_t n, uint32_t crc, const uint8_t *buf);  /* D: CRC-32 update over n (eax) bytes at buf (ebx) from crc (edx): crc = (crc >> 8) ^ table[(byte ^ crc) & 0xFF], table 0x11260C (f7BB14); no inversions */
void     f7BCD4(void);                                     /* D: int16 [30][10] at 0x112A94 = game_rand() * 30000 >> 15, 300 random numbers in row order */
void     f726DC(void);                                     /* D: rebuild the trade-offer records 0x110ECC [8][21] and the count [0x112560] from nations 1..9 (+0x9A, +0x9C..+0xA8), shuffling with f76F70 (random numbers) */

#endif
