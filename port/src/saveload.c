/* Save-file reading of AC_MAIN.EXE (module S1 of docs/MAP_PLAN.md, the loading half): 0x5F05C (read a whole save file
   into the game globals), 0x7BBF0 (CRC check of a save file), 0x7BB84 (CRC-32 update), 0x7BCD4 (the pre-rolled random
   table) and 0x726DC (trade-offer table rebuild). Literal translations; the address of each instruction a line comes
   from is in its comment. D = read in the code, G = inference. Prototypes: saveload.h.

   Callers: f5F05C from the restore dialog 0x5FF28 (save.c; the other 11 callers are the play-by-email loaders, dropped);
   f7BBF0 from 0x5FF28 and the autosave 0x5E96C; f726DC from 0x5FF28 when the save's phase is 14 (ACCEPT TRADES).
   f7BB84 has three more callers in main's start-up (0x1058F, 0x105B0, 0x105D2), not part of this module.

   Runtime calls (Watcom C runtime -> standard C): 0x7DB6D fread(ptr eax, size edx, n ebx, fp ecx); 0x7D8C8 fopen;
   0x7D95E fclose; 0x7D5D0 memset; 0x822B5 malloc (Watcom: NULL for 0 or above 0xFFFFFFD4, wc_malloc below); 0x877E6
   free (load_heap_free for the blocks kept in data fields). 0x7D2E8 is AIL's file-size helper, translated below as a
   static copy of newgame.c's (that one is static there). Files are opened relative to the current folder (the port
   runs in the game folder) with the original's mode "rb" (0xB976D). __CHK dropped. The debug stubs 0x78B0C (log, at
   0x5FA2D with "savegame") and 0x78B24 (trace, at 0x7BC33 and 0x7BCC1 with "Fptr", "CRC check") are no-ops in the
   original and are left out, each with a comment at its call site.

   Heap pointers in 32-bit data fields: [0xF0984 + 4i], i = 1..11, the save's record lists (10-byte records, counts
   int16 [0xF09BA + 2i]). f5F05C frees them and allocates new ones; the port stores load_heap_alloc handles there, so
   0xF0988..0xF09B3 differs from a DOSBox-X dump by design whenever a list is allocated (flag 0x01 of [0xE1624]).

   1995 bugs fixed (the original's behaviour kept under g_original, marked BUG(1995)):
     A (G) f5F05C reads the 22-byte "Advanced Civilization" tag for every version, although files before version 0x28
       have none (D: the slot dialog 0x601D8 skips it only from 0x28, f7BBF0's header is 0x35 = 2 + 51 bytes before
       0x28, the save 0x5E7D0 writes it only from 0x28). Every later field of such a file is read 22 bytes late.
     B (G) f5F05C keeps the previous game's random table 0x112A94 for versions 0x2D and 0x2E: their 300 file bytes are
       read into its first byte one by one (discarded, as version 0x2C's 270), but only the other old versions go on to
       regenerate it with f7BCD4 (D: 0x5F7E5 jmp 0x5F7C5 -> 0x5F7B8 jge 0x5F82D, past the call at 0x5F828). The fix
       calls f7BCD4 for them too (300 more random numbers).
   Neither is reachable with a save the game then accepts: the restore dialog rejects versions below 0x35 (0x2C with
   play-by-email) after loading (f6016C).

   Noted, not changed:
     - The list counts: the file holds 11 int16 at 0xF09BA (entries 0..10) but the lists are 1..11 with count i at
       0xF09BA + 2i (D 0x5F322: dword [2i + 0xF09B8] sar 16). Entry 0 is read and unused, list 11's count (0xF09D0) is
       not in the file: the reader uses whatever is in memory, as the writer 0x5EBFB..0x5EC73 does. A file written with
       list 11 non-empty and read in a fresh session would be read misaligned (G). Not fixable in the reader.
     - f726DC counts the goods to shuffle with "if (a[k]) k++" over j < 5, i.e. the length of the leading run of
       non-zero bytes, not the number of non-zero bytes (D 0x72726 [eax + ebp - 0x2C] with eax = [ebp-0x1C], the
       count). Same result while the goods are packed at the front (G: they are). */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "saveload.h"

/* ---- Runtime helpers ------------------------------------------------------------------------------------------- */

/* 0x822B5 Watcom malloc, for a block the code keeps in a local only (0x7BBF0's file buffer): NULL for a size of 0 or
   above 0xFFFFFFD4 unsigned, as newgame.c's wc_malloc. Port: zeroed (calloc), so a short read leaves zeros instead of
   old heap bytes (G: never happens with an intact file). */
static uint8_t *wc_malloc(long size)
{
    uint32_t u = (uint32_t)size;

    if (u == 0 || u > 0xFFFFFFD4u)
        return NULL;
    return calloc(1, u);
}

/* 0x7D2E8: AIL's file size (a copy of newgame.c's static f7D2E8): [0xCAC68] = 0; open the file (DOS open 0x93F92,
   mode 0x200: read, binary, G); if it does not open, [0xCAC68] = 3 and -1; else filelength (0x941E5; -1 ->
   [0xCAC68] = 5), close (0x941E0), the length. cdecl, one stack argument. */
static long f7D2E8(const char *name)
{
    FILE *f;
    long size;

    setD32(0xCAC68u, 0);                              /* 0x7D2EA */
    f = fopen(name, "rb");                            /* 0x7D2FA */
    if (f == NULL) {                                  /* 0x7D304 cmp eax,-1 */
        setD32(0xCAC68u, 3);                          /* 0x7D309 */
        return -1;                                    /* 0x7D313 */
    }
    size = -1;
    if (fseek(f, 0, SEEK_END) == 0)                   /* 0x7D314 filelength */
        size = ftell(f);
    if (size == -1)                                   /* 0x7D31B */
        setD32(0xCAC68u, 5);                          /* 0x7D320 */
    fclose(f);                                        /* 0x7D32C */
    return size;                                      /* 0x7D331 */
}

/* fread(link address a, size, n, fp) (0x7DB6D) into the data object. Port guard: a NULL fp (a caller whose fopen
   failed; the restore dialog only offers files that exist) reads nothing instead of crashing the C library. */
static void rd(uint32_t a, size_t size, size_t n, FILE *fp)
{
    if (fp != NULL)
        (void)fread(DP(a), size, n, fp);
}

/* fread(p, size, n, fp) into a local (C pointer); same guard */
static void rdl(void *p, size_t size, size_t n, FILE *fp)
{
    if (fp != NULL)
        (void)fread(p, size, n, fp);
}

/* ---- 0x5F05C: read a save file into the game globals ------------------------------------------------------------- */

/* Area record r of the 61-byte table at src -> the 69-byte table at dst (versions below 0x34; D 0x5FA41..0x5FC7B for
   0x10A560 -> 0xE31B8, 0x5FC80..0x5FEBC for 0x10CBC0 -> 0xE5D20; the two loops are the same code with other bases).
   Offsets dst <- src: +0 <- +0, +1 <- +1, +2..+0xE <- +2..+0xE, +0xF..+0x1B <- +0xF..+0x1B, +0x1C..+0x24 <- same,
   +0x2E..+0x36 <- +0x26..+0x2E, +0x3C..+0x44 <- +0x34..+0x3C, +0x25..+0x2D <- same, words +0x37 <- +0x2F and
   +0x39 <- +0x31, byte +0x3B <- +0x33. */
static void area61to69(uint32_t dst, uint32_t src)
{
    int32_t r;      /* [ebp-0x1C] */
    int32_t j;      /* [ebp-0x20] */

    for (r = 0; r <= 0xA0; r++) {                     /* 0x5FC19 cmp 0xA0; jg (0x5FE5A) */
        uint32_t s = src + 61u * (uint32_t)r;         /* (r << 4 - r) << 2 + r */
        uint32_t d = dst + 69u * (uint32_t)r;         /* (r << 4 + r) << 2 + r */
        setD8(d + 0, D8(s + 0));                      /* 0x5FC46 (0x5FE87) */
        setD8(d + 1, D8(s + 1));                      /* 0x5FC72 (0x5FEB3) */
        for (j = 0; j < 0xD; j++) {                   /* 0x5FA51 cmp 0xD; jge (0x5FC90) */
            setD8(d + 2 + (uint32_t)j, D8(s + 2 + (uint32_t)j));         /* 0x5FA83 (0x5FCC4) */
            setD8(d + 0xF + (uint32_t)j, D8(s + 0xF + (uint32_t)j));     /* 0x5FAB5 (0x5FCF8) */
        }
        for (j = 0; j < 9; j++) {                     /* 0x5FAD9 cmp 9; jge (0x5FD1C) */
            setD8(d + 0x1C + (uint32_t)j, D8(s + 0x1C + (uint32_t)j));   /* 0x5FB00 (0x5FD43) */
            setD8(d + 0x2E + (uint32_t)j, D8(s + 0x26 + (uint32_t)j));   /* 0x5FB32 (0x5FD77) */
            setD8(d + 0x3C + (uint32_t)j, D8(s + 0x34 + (uint32_t)j));   /* 0x5FB66 (0x5FDA9) */
            setD8(d + 0x25 + (uint32_t)j, D8(s + 0x25 + (uint32_t)j));   /* 0x5FB95 (0x5FDD6) */
        }
        setD16(d + 0x37, D16(s + 0x2F));              /* 0x5FBB6 (0x5FDF7) */
        setD16(d + 0x39, D16(s + 0x31));              /* 0x5FBE2 (0x5FE23) */
        setD8(d + 0x3B, D8(s + 0x33));                /* 0x5FC0D (0x5FE4E) */
    }
}

/* 0x5F05C: version (eax), desc (edx), fp (ebx). The version is re-read from *version (movsx word [eax]) at every
   test; it does not change after the first read. */
void f5F05C(int16_t *version, char *desc, FILE *fp)
{
    int32_t i;                  /* [ebp-0x1C] */
    int32_t j;                  /* [ebp-0x20] */
    uint8_t tag[0x16];          /* [ebp-0x48] the 22-byte "Advanced Civilization" tag, discarded */
    uint8_t skip11[0xB];        /* [ebp-0x30] 11 bytes skipped */
    uint8_t skip2[2];           /* [ebp-0x10] 2 bytes skipped */
    uint8_t skip130[0x82];      /* [ebp-0xCC] version 0x2E: 130 bytes skipped */
    uint8_t vb[2];
    int32_t v;

    for (i = 1; i <= 0xB; i++) {                      /* 0x5F07B, 0x5F08A cmp 0xB; jg */
        if (D32(0xF0984u + 4u * (uint32_t)i) != 0) {  /* 0x5F093 test esi,esi */
            load_heap_free(D32(0xF0984u + 4u * (uint32_t)i));   /* 0x5F0A8 free */
            setD32(0xF0984u + 4u * (uint32_t)i, 0);   /* 0x5F0B2 */
        }
    }

    /* 0x5F0CB fread(version, 2, 1, fp): bytes not delivered leave *version as it was */
    vb[0] = (uint8_t)(uint16_t)*version;
    vb[1] = (uint8_t)((uint16_t)*version >> 8);
    rdl(vb, 2, 1, fp);
    *version = (int16_t)(uint16_t)(vb[0] | vb[1] << 8);
    v = *version;

    if (g_original || v >= 0x28)
        rdl(tag, 1, 0x16, fp);                        /* 0x5F0E0 fread(local, 1, 22, fp) */
    /* BUG(1995) A: the original reads the tag whatever the version; files before 0x28 have none (see the header) */

    rdl(desc, 0x33, 1, fp);                           /* 0x5F0F5 fread(desc, 51, 1, fp) */
    rd(0xF40F4u, 4, 1, fp);                           /* 0x5F10C the seed the round started with */
    rd(0xE1620u, 6, 1, fp);                           /* 0x5F123 setup bytes 0xE1620..0xE1625 */
    rd(0xF14CCu, 0xC7, 0xE, fp);                      /* 0x5F13A nations 14 x 199 */
    v = *version;                                     /* 0x5F142 movsx */
    if (v >= 0x34)                                    /* 0x5F145 cmp 0x34; jl */
        rd(0xE31B8u, 0x45, 0xA1, fp);                 /* 0x5F14A..0x5F170 areas 161 x 69 */
    else
        rd(0x10A560u, 0x3D, 0xA1, fp);                /* 0x5F15E..0x5F170 161 x 61, converted at the end */
    rd(0xF06D0u, 0x19, 9, fp);                        /* 0x5F187 card stacks 9 x 25 */
    rd(0xE2C18u, 3, 0xA0, fp);                        /* 0x5F19E territory state 160 x 3 */
    rd(0xF4102u, 1, 1, fp);                           /* 0x5F1B2 phase */
    rd(0xF40FCu, 2, 1, fp);                           /* 0x5F1C9 */
    rd(0xF0698u, 2, 1, fp);                           /* 0x5F1E0 */
    rd(0xF069Au, 2, 0xA, fp);                         /* 0x5F1F7 */
    rd(0xF06AEu, 2, 1, fp);                           /* 0x5F20E */
    rd(0xF06B0u, 2, 8, fp);                           /* 0x5F225 player order */
    rd(0xF5476u, 2, 1, fp);                           /* 0x5F23C */
    rd(0x111118u, 0x10, 0x38, fp);                    /* 0x5F253 56 x 16 */
    rd(0x112548u, 2, 1, fp);                          /* 0x5F26A */
    rd(GAME_SEED, 4, 1, fp);                          /* 0x5F281 the random seed 0xCAC5C (file offset 0x3D0C) */
    rd(0xF4104u, 0x20, 8, fp);                        /* 0x5F298 players 8 x 32 */
    rd(0xF5474u, 2, 1, fp);                           /* 0x5F2AF */
    rd(0x109E84u, 1, 0x14, fp);                       /* 0x5F2C6 the play-by-email name */
    rd(0xF5478u, 2, 1, fp);                           /* 0x5F2DD */

    if ((D8(0xE1624u) & 0x01) != 0) {                 /* 0x5F2E2 test byte [0xE1624],1; je 0x5F3A4 */
        rd(0xF09BAu, 2, 0xB, fp);                     /* 0x5F301 11 int16 counts (entries 0..10) */
        for (i = 1; i <= 0xB; i++) {                  /* 0x5F306, 0x5F315 cmp 0xB; jg */
            if (S16(0xF09BAu + 2u * (uint32_t)i) == 0)   /* 0x5F322 dword [2i + 0xF09B8] sar 16; test; je */
                continue;
            /* 0x5F33D..0x5F352 [0xF0984 + 4i] = malloc(10 * count) (0x822B5; a negative count -> NULL) */
            setD32(0xF0984u + 4u * (uint32_t)i,
                   load_heap_alloc((uint32_t)((int32_t)S16(0xF09BAu + 2u * (uint32_t)i) * 10)));
            for (j = 0; S16(0xF09BAu + 2u * (uint32_t)i) > j; j++) {   /* 0x5F359, 0x5F371 cmp count,j; jle */
                /* 0x5F37F..0x5F39D fread([0xF0984 + 4i] + 10j, 10, 1, fp): the handle plus the offset, as the
                   pointer was. Port guard: if malloc failed the original reads into address 10j; the port reads the
                   record into a scratch buffer, so the file position advances the same way. */
                uint8_t *p = load_heap_ptr(D32(0xF0984u + 4u * (uint32_t)i) + 10u * (uint32_t)j, 10);
                uint8_t scratch[10];
                rdl(p != NULL ? (void *)p : (void *)scratch, 10, 1, fp);
            }
        }
    }

    rd(0xF408Cu, 1, 1, fp);                           /* 0x5F3B3 */
    rd(0xF09D4u, 1, 0xC, fp);                         /* 0x5F3CA */
    rd(0xF40FEu, 1, 1, fp);                           /* 0x5F3DE */
    rd(0xF07B4u, 0x19, 9, fp);                        /* 0x5F3F5 9 x 25 */
    rd(0xF0898u, 0x19, 9, fp);                        /* 0x5F40C 9 x 25 */
    rd(0xF4094u, 2, 1, fp);                           /* 0x5F423 */
    rd(0xF4098u, 2, 1, fp);                           /* 0x5F43A */
    rd(0xF409Au, 2, 0xA, fp);                         /* 0x5F451 */
    rd(0xEE9F0u, 2, 0x1C2, fp);                       /* 0x5F468 450 int16 (score history) */
    rd(0x110F74u, 0x1A1, 1, fp);                      /* 0x5F47F 417 bytes (trades) */
    rdl(skip11, 1, 0xB, fp);                          /* 0x5F494 11 bytes into a local */
    rdl(skip2, 2, 1, fp);                             /* 0x5F4A9 2 bytes into a local */
    rd(0xF590Cu, 4, 0xA, fp);                         /* 0x5F4C0 */
    rd(0x1105D4u, 4, 0xA, fp);                        /* 0x5F4D7 */
    rd(0x1105ACu, 4, 0xA, fp);                        /* 0x5F4EE */
    rd(0xF40F0u, 4, 1, fp);                           /* 0x5F505 */

    v = *version;                                     /* 0x5F50D movsx */
    if (v >= 0x30) {                                  /* 0x5F510 cmp 0x30; jl */
        rd(0xF0044u, 1, 0x640, fp);                   /* 0x5F527 1600 bytes */
        rd(0xEED84u, 1, 0x640, fp);                   /* 0x5F53E 1600 bytes */
    } else if (v >= 0x25 && v < 0x30) {               /* 0x5F548 cmp 0x25; jl; 0x5F551 cmp 0x30; jge */
        /* versions 0x25..0x2F: stored [t][n], kept [n][t] (10 x 160), read byte by byte */
        for (i = 0; i < 0xA0; i++)                    /* 0x5F55C, 0x5F567 cmp 0xA0; jge */
            for (j = 0; j <= 9; j++)                  /* 0x5F572, 0x5F57D cmp 9; jg */
                rd(0xF0044u + 160u * (uint32_t)j + (uint32_t)i, 1, 1, fp);   /* 0x5F589..0x5F5A6 (5j << 5) */
        for (i = 0; i < 0xA0; i++)                    /* 0x5F5AF, 0x5F5BA cmp 0xA0; jge */
            for (j = 0; j <= 9; j++)                  /* 0x5F5C5, 0x5F5D0 cmp 9; jg */
                rd(0xEED84u + 160u * (uint32_t)j + (uint32_t)i, 1, 1, fp);   /* 0x5F5DC..0x5F5F7 */
    }

    rd(0xF4898u, 4, 1, fp);                           /* 0x5F610 */
    rd(0xF5230u, 1, 0xD, fp);                         /* 0x5F627 */
    rd(0x111498u, 0x14, 7, fp);                       /* 0x5F63E 7 x 20 */
    rd(0x112568u, 2, 1, fp);                          /* 0x5F655 */
    rd(0xF4100u, 2, 1, fp);                           /* 0x5F66C */
    rd(0xF47D4u, 1, 0xA2, fp);                        /* 0x5F683 162 bytes */
    rd(0xF4878u, 1, 9, fp);                           /* 0x5F69A */
    rd(0xF2A94u, 0xC7, 0xE, fp);                      /* 0x5F6B1 second nation table 14 x 199 */
    rd(0x109E24u, 1, 0x51, fp);                       /* 0x5F6C8 81 bytes */
    rd(0xF5934u, 1, 0xC, fp);                         /* 0x5F6DF */
    rd(0xF1FB0u, 0xC7, 0xE, fp);                      /* 0x5F6F6 third nation table 14 x 199 */
    v = *version;                                     /* 0x5F6FE movsx */
    if (v >= 0x34)                                    /* 0x5F701 cmp 0x34; jl */
        rd(0xE5D20u, 0x45, 0xA1, fp);                 /* 0x5F706..0x5F72C second area table 161 x 69 */
    else
        rd(0x10CBC0u, 0x3D, 0xA1, fp);                /* 0x5F71A..0x5F72C 161 x 61, converted at the end */
    rd(0xE2DF8u, 3, 0xA0, fp);                        /* 0x5F743 second territory state 160 x 3 */
    rd(0xEDF58u, 4, 1, fp);                           /* 0x5F75A */
    rd(0xEDF5Cu, 4, 1, fp);                           /* 0x5F771 */

    v = *version;                                     /* 0x5F779 movsx */
    if (v >= 0x2F) {                                  /* 0x5F77C cmp 0x2F; jl */
        rd(0x112A94u, 2, 0x12C, fp);                  /* 0x5F793 the random table, 300 int16 */
    } else if (v == 0x2D || v == 0x2E) {              /* 0x5F79D, 0x5F7A2 */
        for (i = 0; i < 0x1E; i++)                    /* 0x5F7A9, 0x5F7B4 cmp 0x1E; jge 0x5F82D */
            for (j = 0; j <= 9; j++)                  /* 0x5F7C0, 0x5F7CB cmp 9; jg */
                rd(0x112A94u, 1, 1, fp);              /* 0x5F7E0 one byte into the table's first byte */
        if (!g_original)
            f7BCD4();
        /* BUG(1995) B: the original skips the regeneration for these two versions (jge 0x5F82D, past 0x5F828) and
           keeps the previous game's table */
    } else {
        if (v == 0x2C) {                              /* 0x5F7E7 cmp 0x2C; jne 0x5F828 */
            for (i = 0; i < 0x1E; i++)                /* 0x5F7EE, 0x5F7F9 cmp 0x1E; jge */
                for (j = 0; j < 9; j++)               /* 0x5F801, 0x5F80C cmp 9; jge */
                    rd(0x112A94u, 1, 1, fp);          /* 0x5F821 */
        }
        f7BCD4();                                     /* 0x5F828 */
    }

    if (*version == 0x2E)                             /* 0x5F830 cmp 0x2E; jne */
        rdl(skip130, 1, 0x82, fp);                    /* 0x5F84B 130 bytes into a local */
    if (*version >= 0x31)                             /* 0x5F856 cmp 0x31; jl */
        rd(0xEFA04u, 1, 0x640, fp);                   /* 0x5F86D 1600 bytes */
    if (*version >= 0x32) {                           /* 0x5F878 cmp 0x32; jl 0x5F8C4 */
        rd(0xF0684u, 2, 0xA, fp);                     /* 0x5F88F */
        rd(0xF6E76u, 2, 0xA, fp);                     /* 0x5F8A6 */
        rd(0xF6E12u, 2, 0x32, fp);                    /* 0x5F8BD */
    } else {
        for (i = 1; i <= 9; i++) {                    /* 0x5F8C4, 0x5F8D3 cmp 9; jg */
            setD16(0xF0684u + 2u * (uint32_t)i, 0);   /* 0x5F8DE */
            setD16(0xF6E76u + 2u * (uint32_t)i, 0);   /* 0x5F8E9 */
        }
    }
    if (*version >= 0x33) {                           /* 0x5F8F9 cmp 0x33; jl 0x5F917 */
        rd(0xF6D54u, 1, 0xBE, fp);                    /* 0x5F910 190 bytes (10 x 19) */
    } else {
        for (i = 1; i <= 9; i++)                      /* 0x5F917, 0x5F926 cmp 9; jg */
            for (j = 1; j <= 0x12; j++)               /* 0x5F92C, 0x5F93B cmp 0x12; jg */
                setD8(0xF6D54u + 19u * (uint32_t)i + (uint32_t)j, 0);   /* 0x5F941..0x5F957 (5i << 2) - i */
    }
    if (*version >= 0x35) {                           /* 0x5F965 cmp 0x35; jl 0x5F983 */
        rd(0xF488Cu, 1, 0xA, fp);                     /* 0x5F97C */
    } else {
        for (i = 1; i <= 9; i++)                      /* 0x5F983, 0x5F992 cmp 9; jg */
            setD8(0xF488Cu + (uint32_t)i, 0);         /* 0x5F99D */
    }
    if (*version >= 0x36)                             /* 0x5F9AB cmp 0x36; jl */
        rd(0xF3580u, 2, 0x585, fp);                   /* 0x5F9C2 1413 int16 */
    if (*version >= 0x37)                             /* 0x5F9CD cmp 0x37; jl 0x5F9EB */
        rd(0xF4230u, 4, 1, fp);                       /* 0x5F9E4 */
    else
        setD32(0xF4230u, 0);                          /* 0x5F9ED */
    if (*version >= 0x38)                             /* 0x5F9F9 cmp 0x38; jl 0x5FA17 */
        rd(0x1106B0u, 4, 0xA, fp);                    /* 0x5FA10 */
    else
        memset(DP(0x1106B0u), 0, 0x28);               /* 0x5FA23 memset(0x1106B0, 0, 40) */

    /* 0x5FA2D f78B0C("savegame"): the debug log stub, a no-op, left out */

    if (*version < 0x34) {                            /* 0x5FA38 cmp 0x34; jge 0x5FEC1 */
        area61to69(0xE31B8u, 0x10A560u);              /* 0x5FA41..0x5FC7B */
        area61to69(0xE5D20u, 0x10CBC0u);              /* 0x5FC80..0x5FEBC */
    }

    {
        uint8_t al = D8(0xE1623u);                    /* 0x5FEC1 */
        if ((uint32_t)(al >> 4) < 3) {                /* 0x5FECA shr ah,4; 0x5FECF cmp edx,3; jge */
            al &= 0x0F;                               /* 0x5FED4 */
            setD8(0xE1623u, al);                      /* 0x5FED8 */
            setD8(0xE1623u, (uint8_t)(al | 0x30));    /* 0x5FEDD or bl,0x30; 0x5FEE0 */
        }
    }

    setD32(0xF09E4u, 0);                              /* 0x5FEED */
    for (i = 1; i <= 9; i++) {                        /* 0x5FEF3, 0x5FEFE cmp 9; jg */
        if (D8(GAME_NATION + (uint32_t)GAME_NATION_SIZE * (uint32_t)i + 0x28) != 0) {   /* 0x5FF04 imul 0xC7; [+0xF14F4] */
            setD32(0xF09E4u, 1);                      /* 0x5FF15 */
            break;
        }
    }
}

/* ---- 0x7BB84: CRC-32 update ---------------------------------------------------------------------------------------- */

/* n (eax), crc (edx), buf (ebx); table at 0x11260C, built by f7BB14 (title.c) at start-up */
uint32_t f7BB84(int32_t n, uint32_t crc, const uint8_t *buf)
{
    uint32_t count = (uint32_t)n;                     /* [ebp-0x18] */
    const uint8_t *p = buf;                           /* [ebp-0xC] */

    while (--count != 0xFFFFFFFFu) {                  /* 0x7BBA4 dec; cmp -1; je */
        uint32_t hi = (crc >> 8) & 0xFFFFFFu;         /* 0x7BBB0 shr 8; and 0xFFFFFF */
        uint32_t t = D32(0x11260Cu + 4u * ((*p++ ^ crc) & 0xFFu));   /* 0x7BBBB..0x7BBD0 */
        crc = hi ^ t;                                 /* 0x7BBE0 */
    }
    return crc;                                       /* 0x7BBE7 */
}

/* ---- 0x7BBF0: CRC check of a save file ------------------------------------------------------------------------------ */

uint32_t f7BBF0(const char *name)
{
    long size;                  /* [ebp-0x1C] */
    uint8_t *buf;               /* [ebp-0x20], [ebp-0x14] */
    FILE *fp;                   /* [ebp-0x10] */
    uint32_t crc;               /* [ebp-0x18] */
    long hdr;

    size = f7D2E8(name);                              /* 0x7BC0A */
    buf = wc_malloc(size);                            /* 0x7BC18 malloc */
    /* 0x7BC33 f78B24(size, "Fptr", "CRC check"): debug trace stub, a no-op, left out */
    fp = fopen(name, DSTR(0xB976Du));                 /* 0x7BC40 fopen(name, "rb") */
    /* Port guard (G): the original goes on with a NULL file or buffer (fread through NULL) and, for a file shorter
       than its header, runs f7BB84 over a negative count (4 billion bytes). The port reports such a file as corrupt
       (non-zero), which is what the callers then do. */
    if (fp == NULL || buf == NULL || size < 2) {
        if (fp != NULL)
            fclose(fp);
        free(buf);
        return 0xFFFFFFFFu;
    }
    (void)fread(buf, (size_t)size, 1, fp);            /* 0x7BC56 fread(buf, size, 1, fp) */
    fclose(fp);                                       /* 0x7BC60 */
    crc = 0;                                          /* 0x7BC65 [ebp-0x18] = edx (0, set for fclose and kept) */
    hdr = (int16_t)(uint16_t)(buf[0] | buf[1] << 8) >= 0x28 ? 0x4B : 0x35;   /* 0x7BC6B movsx; cmp 0x28; jl */
    if (size < hdr) {                                 /* port guard, as above */
        free(buf);
        return 0xFFFFFFFFu;
    }
    crc = f7BB84((int32_t)(size - hdr), crc, buf + hdr);   /* 0x7BC7F..0x7BCA5 */
    free(buf);                                        /* 0x7BCB0 free */
    /* 0x7BCC1 f78B24(0, "Fptr", "CRC check"): debug trace stub, a no-op, left out */
    return crc;                                       /* 0x7BCC6 */
}

/* ---- 0x7BCD4: the pre-rolled random table --------------------------------------------------------------------------- */

void f7BCD4(void)
{
    int32_t i;                  /* [ebp-0x14] */
    int32_t j;                  /* [ebp-0x10] */

    for (i = 0; i < 0x1E; i++) {                      /* 0x7BCE9, 0x7BCEC / 0x7BCFF cmp 0x1E; jge */
        for (j = 0; j <= 9; j++) {                    /* 0x7BCF4, 0x7BD12 cmp 9; jg */
            uint32_t r = (uint32_t)f78AE4();          /* 0x7BD18 game_rand */
            uint32_t x = (r * 30000u) >> 15;          /* 0x7BD1D..0x7BD33: r*31, *4 + r, << 4, *16 - (2000r); shr 0xF */
            setD16(0x112A94u + 20u * (uint32_t)i + 2u * (uint32_t)j, (uint16_t)x);   /* 0x7BD4A mov word */
        }
    }
}

/* ---- 0x726DC: rebuild the trade-offer table ------------------------------------------------------------------------- */

/* Records at 0x110ECC, 21 bytes: +0x00..+0x0C the nation's 13 bytes +0x9C..+0xA8 (the first 5 shuffled unless the
   variant 'Real Goods in trade are known', [0xE1625] & 0x20, is on), +0x0D..+0x11 a second shuffle of the first 5,
   +0x12 the nation, +0x13 0, +0x14 0xFF. Count [0x112560]. */
void f726DC(void)
{
    uint8_t goods[5];           /* [ebp-0x2C] */
    int32_t n;                  /* [ebp-0x18] the nation, then the record */
    int32_t k;                  /* [ebp-0x1C] */
    int32_t j;                  /* [ebp-0x20], [ebp-0x24] */
    uint32_t nat, rec;

    memset(DP(0x110ECCu), 0, 0xA8);                   /* 0x726FD memset(0x110ECC, 0, 168) */
    setD32(0x112560u, 0);                             /* 0x72709 */
    for (n = 1; n <= 9; n++) {                        /* 0x7270F, 0x7279A cmp 9; jg */
        nat = GAME_NATION + (uint32_t)GAME_NATION_SIZE * (uint32_t)n;
        if (D8(nat + 0x9A) == 0)                      /* 0x727AB [+0xF1566]; test; je */
            continue;
        if (S8(nat + 0x9C) == 0)                      /* 0x727BC dword [+0xF1565] sar 0x18; test; je */
            continue;
        rec = 0x110ECCu + 21u * D32(0x112560u);       /* 0x727C9..0x727DB */
        setD8(rec + 0x12, (uint8_t)n);                /* 0x727E2 */
        setD8(rec + 0x13, 0);                         /* 0x727E8 */
        memcpy(DP(rec), DP(nat + 0x9C), 0xD);         /* 0x727EE..0x7281A rep movsd/movsb, 13 bytes */
        if ((D8(0xE1625u) & 0x20) == 0) {             /* 0x7281D test bh,0x20; jne 0x72774 */
            memcpy(goods, DP(nat + 0x9C), 5);         /* 0x7282C..0x72854 */
            k = 0;                                    /* 0x72859 */
            for (j = 0; j < 5; j++)                   /* 0x7285C, 0x7271D cmp 5; jge */
                if (goods[k] != 0)                    /* 0x72723 [eax + ebp - 0x2C], eax = k (not j, D) */
                    k++;                              /* 0x72731 */
            f76F70(k, goods);                         /* 0x7273C */
            memcpy(DP(0x110ECCu + 21u * D32(0x112560u)), goods, 5);   /* 0x72741..0x72771 */
        }
        setD8(0x110EE0u + 21u * D32(0x112560u), 0xFF);   /* 0x72774..0x72783 [edx + 4*(5*edx) + 0x110EE0] = -1 */
        setD32(0x112560u, D32(0x112560u) + 1);        /* 0x7278B lea ecx,[edx+1]; 0x7278E */
    }

    for (n = 0; n < S32(0x112560u); n++) {            /* 0x72866, 0x72909 cmp; jge (signed) */
        rec = 0x110ECCu + 21u * (uint32_t)n;
        for (j = 0; j < 5; j++)                       /* 0x72913, 0x72874 cmp 5; jge */
            setD8(rec + 0xD + (uint32_t)j, D8(rec + (uint32_t)j));   /* 0x728A6 -> 0x728AC [+0x110ED9] */
        k = 0;                                        /* 0x728B6 */
        for (j = 0; j < 5; j++)                       /* 0x728B9, 0x728D5 cmp 5; jge */
            if (D8(rec + 0xD + (uint32_t)k) != 0)     /* 0x728DE [21n + k + 0x110ED9] */
                k++;                                  /* 0x728EB */
        f76F70(k, DP(rec + 0xD));                     /* 0x728F0..0x728FB */
    }
}
