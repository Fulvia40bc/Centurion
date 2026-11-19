/* Save-file writing of AC_MAIN.EXE (module S1 of docs/MAP_PLAN.md, the writing half): 0x5E9B0 (write a whole save body
   to fp) and 0x5E8EC (the autosave to savgamea.dat with the CRC appended). Translations in savewrite.c. The reader is
   saveload.c's f5F05C (VERIFIED); the writer is its mirror for format version 0x38, the only version it writes. */

#ifndef SAVEWRITE_H
#define SAVEWRITE_H

#include <stdio.h>

void f5E9B0(char *desc, FILE *fp);   /* D: write the save body (version 0x38) to fp: int16 0x38, the 22-byte "Advanced Civilization" tag, desc[50] = 0 then 51 bytes of desc, then the whole state in file order (the range list in savewrite.c). If bit 0x01 of [0xE1624] is set, also the 11 int16 counts at 0xF09BA and the 10-byte records of the heap lists [0xF0984 + 4i] */
void f5E8EC(int phase);              /* D: the autosave after a phase: desc = "AutoSaved after <phase name> - R<round>", fopen("savgamea.dat", "wb"), [0xF5230] = 0, f5E9B0(desc, fp), fclose, then append f7BBF0("savgamea.dat") as 4 little-endian bytes ("ab") */

#endif
