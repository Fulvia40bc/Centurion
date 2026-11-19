/* Save-file writing of AC_MAIN.EXE (module S1 of docs/MAP_PLAN.md, the writing half): 0x5E9B0 (write a whole save body
   to fp) and 0x5E8EC (the autosave). The mirror of saveload.c's f5F05C for version 0x38, the only version written
   (D, docs/INVENTORY_M6.json; the field list is read against the verified reader f5F05C — where the task's list and
   the reader differ, the reader's addresses are used: the play-by-email name is 0x109E84 (not 0xEE20C) and the second
   1600-byte block is 0xEED84 (not 0xEF3C4)).

   The reader f5F05C skips 13 bytes between 0x110F74 and 0xF590C into locals: 11 bytes of the version stamp at 0xBC8E4
   ("1.11B1" + NULs, as title.c's 0x4F66E reads it) and the 2 bytes at 0xF0982. The writer writes both back from the
   data object, so a read-write round trip is byte-identical.

   Runtime calls (Watcom C runtime -> standard C): 0x7D8xx fwrite(ptr eax, size edx, n ebx, fp ecx); 0x7D8C8 fopen;
   0x7D95E fclose; 0x7DB3D sprintf. Files are opened relative to the current folder (the port runs in the game folder)
   with the original's modes "wb" / "ab". Prototypes: savewrite.h. */

#include <stdio.h>
#include <string.h>
#include "game.h"
#include "saveload.h"
#include "savewrite.h"
#include "state.h"

/* ---- Runtime helpers ------------------------------------------------------------------------------------------- */

/* fwrite(link address a, size, n, fp) (0x7DB6D mirrored) from the data object. Port guard: a NULL fp (a caller whose
   fopen failed) writes nothing instead of crashing the C library. */
static void wr(uint32_t a, size_t size, size_t n, FILE *fp)
{
    if (fp != NULL)
        (void)fwrite(DP(a), size, n, fp);
}

/* fwrite(p, size, n, fp) from a C pointer; same guard */
static void wrl(const void *p, size_t size, size_t n, FILE *fp)
{
    if (fp != NULL)
        (void)fwrite(p, size, n, fp);
}

/* ---- 0x5E9B0: write the save body (version 0x38) -------------------------------------------------------------------- */

/* The range list, in file order (link address, size, count), exactly as the reader f5F05C reads it for version 0x38. */
void f5E9B0(char *desc, FILE *fp)
{
    static const uint8_t tag[0x16] = "Advanced Civilization";  /* + NUL, 22 bytes */
    uint8_t vb[2];
    int32_t i;

    vb[0] = 0x38;
    vb[1] = 0x00;
    wrl(vb, 2, 1, fp);                                /* int16 version 0x38 */
    wrl(tag, 1, 0x16, fp);                            /* the 22-byte tag */

    desc[50] = 0;                                     /* the writer terminates the caller's buffer */
    wrl(desc, 0x33, 1, fp);                           /* 51 bytes of description */

    wr(0xF40F4u, 4, 1, fp);                           /* the seed the round started with */
    wr(0xE1620u, 6, 1, fp);                           /* setup bytes */
    wr(0xF14CCu, 0xC7, 0xE, fp);                      /* nations 14 x 199 */
    wr(0xE31B8u, 0x45, 0xA1, fp);                     /* areas 161 x 69 */
    wr(0xF06D0u, 0x19, 9, fp);                        /* card stacks 9 x 25 */
    wr(0xE2C18u, 3, 0xA0, fp);                        /* territory state 160 x 3 */
    wr(0xF4102u, 1, 1, fp);                           /* phase */
    wr(0xF40FCu, 2, 1, fp);                           /* round */
    wr(0xF0698u, 2, 1, fp);
    wr(0xF069Au, 2, 0xA, fp);
    wr(0xF06AEu, 2, 1, fp);
    wr(0xF06B0u, 2, 8, fp);                           /* player order */
    wr(0xF5476u, 2, 1, fp);
    wr(0x111118u, 0x10, 0x38, fp);                    /* 56 x 16 */
    wr(0x112548u, 2, 1, fp);
    wr(GAME_SEED, 4, 1, fp);                          /* the random seed 0xCAC5C */
    wr(0xF4104u, 0x20, 8, fp);                        /* players 8 x 32 */
    wr(0xF5474u, 2, 1, fp);
    wr(0x109E84u, 1, 0x14, fp);                       /* the play-by-email name (reader's address) */
    wr(0xF5478u, 2, 1, fp);

    if ((D8(0xE1624u) & 0x01) != 0) {                 /* play-by-email: the record lists */
        wr(0xF09BAu, 2, 0xB, fp);                     /* 11 int16 counts (entries 0..10) */
        for (i = 1; i <= 0xB; i++) {
            int16_t cnt = S16(0xF09BAu + 2u * (uint32_t)i);
            uint32_t want;
            if (cnt <= 0)
                continue;
            want = (uint32_t)((int32_t)cnt * 10);
            {
                uint8_t *p = load_heap_ptr(D32(0xF0984u + 4u * (uint32_t)i), want);
                if (p != NULL) {
                    wrl(p, 1, want, fp);
                } else {
                    /* port guard: the original writes from the handle + offset whatever it is; the port
                       writes zeros, so the file size stays right */
                    static const uint8_t zeros[10] = {0};
                    int32_t j;
                    for (j = 0; j < cnt; j++)
                        wrl(zeros, 1, 10, fp);
                }
            }
        }
    }

    wr(0xF408Cu, 1, 1, fp);
    wr(0xF09D4u, 1, 0xC, fp);
    wr(0xF40FEu, 1, 1, fp);
    wr(0xF07B4u, 0x19, 9, fp);
    wr(0xF0898u, 0x19, 9, fp);
    wr(0xF4094u, 2, 1, fp);
    wr(0xF4098u, 2, 1, fp);
    wr(0xF409Au, 2, 0xA, fp);
    wr(0xEE9F0u, 2, 0x1C2, fp);                       /* 450 int16, score history */
    wr(0x110F74u, 1, 0x1A1, fp);                      /* 417 bytes, trades */
    wr(0xBC8E4u, 1, 0xB, fp);                         /* the 11-byte version stamp "1.11B1" + NULs (skipped by the reader) */
    wr(0xF0982u, 2, 1, fp);                           /* 2 bytes (skipped by the reader) */
    wr(0xF590Cu, 4, 0xA, fp);
    wr(0x1105D4u, 4, 0xA, fp);
    wr(0x1105ACu, 4, 0xA, fp);
    wr(0xF40F0u, 4, 1, fp);
    wr(0xF0044u, 1, 0x640, fp);                       /* 1600 bytes */
    wr(0xEED84u, 1, 0x640, fp);                       /* 1600 bytes (reader's address) */
    wr(0xF4898u, 4, 1, fp);
    wr(0xF5230u, 1, 0xD, fp);                         /* 13 bytes */
    wr(0x111498u, 0x14, 7, fp);                       /* 7 x 20 */
    wr(0x112568u, 2, 1, fp);
    wr(0xF4100u, 2, 1, fp);
    wr(0xF47D4u, 1, 0xA2, fp);                        /* 162 bytes */
    wr(0xF4878u, 1, 9, fp);
    wr(0xF2A94u, 0xC7, 0xE, fp);                      /* second nation table 14 x 199 */
    wr(0x109E24u, 1, 0x51, fp);                       /* 81 bytes */
    wr(0xF5934u, 1, 0xC, fp);
    wr(0xF1FB0u, 0xC7, 0xE, fp);                      /* third nation table 14 x 199 */
    wr(0xE5D20u, 0x45, 0xA1, fp);                     /* second area table 161 x 69 */
    wr(0xE2DF8u, 3, 0xA0, fp);                        /* second territory state 160 x 3 */
    wr(0xEDF58u, 4, 1, fp);
    wr(0xEDF5Cu, 4, 1, fp);
    wr(0x112A94u, 2, 0x12C, fp);                      /* the random table, 300 int16 */
    wr(0xEFA04u, 1, 0x640, fp);                       /* 1600 bytes */
    wr(0xF0684u, 2, 0xA, fp);
    wr(0xF6E76u, 2, 0xA, fp);
    wr(0xF6E12u, 2, 0x32, fp);                        /* 50 int16 */
    wr(0xF6D54u, 1, 0xBE, fp);                        /* 190 bytes */
    wr(0xF488Cu, 1, 0xA, fp);
    wr(0xF3580u, 2, 0x585, fp);                       /* 1413 int16 */
    wr(0xF4230u, 4, 1, fp);
    wr(0x1106B0u, 4, 0xA, fp);                        /* 10 x 4 */
}

/* ---- 0x5E8EC: the autosave ----------------------------------------------------------------------------------------- */

void f5E8EC(int phase)
{
    char desc[0x3D];
    FILE *fp;
    uint32_t crc;
    uint8_t b[4];

    /* 0x5E904..0x5E913: the 61-byte local [ebp-0x60] starts as a copy of 0xC6E00 (zeros), so the description's tail
       written by f5E9B0 is zeros (Claude, 2026-11-19: the tail was uninitialised; checked against the original's
       autosaves). Then description = sprintf("AutoSaved after %s - R%d", phase name, int16 [0xF40FC]); the phase
       names are the table of dword pointers at 0xCA9F4 (index = the phase byte) */
    memcpy(desc, DP(0xC6E00u), sizeof desc);
    snprintf(desc, sizeof desc, "AutoSaved after %s - R%d",
             DSTR(D32(0xCA9F4u + 4u * (uint32_t)phase)), (int)S16(0xF40FCu));
    fp = fopen("savgamea.dat", "wb");
    if (fp == NULL)                                   /* port guard: the original goes on with a NULL file */
        return;
    setD8(0xF5230u, 0);                               /* byte [0xF5230] = 0 before the writer */
    f5E9B0(desc, fp);
    fclose(fp);
    crc = f7BBF0("savgamea.dat");
    fp = fopen("savgamea.dat", "ab");
    if (fp == NULL)
        return;
    b[0] = (uint8_t)crc;
    b[1] = (uint8_t)(crc >> 8);
    b[2] = (uint8_t)(crc >> 16);
    b[3] = (uint8_t)(crc >> 24);
    (void)fwrite(b, 1, 4, fp);                        /* the 4-byte CRC, little-endian */
    fclose(fp);
}
