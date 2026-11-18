/* New-game loading of AC_MAIN.EXE, part 2: the game sprites (0x11A8C), the adjacency tables and N.RL (0x3EED8), and
   the recolouring of the territories not in play (0x44974 and its callee 0x44800). Literal translations; the address
   of each instruction a line comes from is in its comment. D = read in the code, G = inference. Prototypes and the
   data layouts: game.h (and ui.h for the sprite records).

   Watcom runtime -> standard C: 0x7D8C8 fopen, 0x7DB6D fread(buf eax, size edx, n ebx, fp ecx), 0x7D95E fclose,
   0x7D5D0 memset(eax, edx, ebx), 0x7DB3D sprintf (snprintf here), 0x822B5 malloc (LIBRARY_CALLS.md: the edx/ebx its
   callers leave are not arguments). __CHK dropped. Files are opened relative to the current directory (the port runs
   in the game folder), except N.RL, which gets the prefix [0xF5820] as in the original.

   Heap pointers in data fields (port, G): the original keeps malloc'd addresses in 32-bit data fields, some of them
   pointing INTO a block (block + 2*k). The port uses load.c's handle scheme (load_heap_alloc / load_heap_ptr /
   load_heap_free: LOAD_HEAP_TAG | slot << 20 | byte offset), because 0x44800 reads the mapdataz handles load.c stores
   at [0xFF64C + 4*t], and 0x3F558 frees the blocks of both files (0xFF648, 0xFF8CC, 0xFF8D0, 0xFFB54, 0xFFB58, 0xFFB5C,
   0x10740C) the same way. Adding to a handle adds to its offset, so block + 2*k carries over unchanged (offsets up to
   1 MB; the largest block here, 8.RL's, is 696,824 bytes). */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "fg.h"

/* load.c's handle scheme (not yet in game.h) */
uint32_t load_heap_alloc(uint32_t size);
uint8_t *load_heap_ptr(uint32_t handle, uint32_t n);

/* ---- Port: heap blocks behind handles --------------------------------------------------------------------------- */

/* 0x822B5 malloc(size) -> the dword at field. The original does not check the result; the port stops with a message
   rather than write through a null pointer (port). */
static void heap_alloc(uint32_t field, uint32_t size)
{
    uint32_t h = load_heap_alloc(size);

    if (h == 0) {
        snprintf((char *)DP(0xF5500u), 0x1F4, "out of memory for the block at 0x%X (%u bytes)", (unsigned)field,
                 (unsigned)size);
        f44CAC();
    }
    setD32(field, h);
}

/* The C pointer for the word at handle h. Port guard (G): a word outside every block (data longer than its size
   field) would corrupt or read the original's heap; the port quits with a message instead. */
static uint8_t *blkp(uint32_t h)
{
    uint8_t *p = load_heap_ptr(h, 2);

    if (p == NULL) {
        snprintf((char *)DP(0xF5500u), 0x1F4, "heap word 0x%08X outside its block", (unsigned)h);
        f44CAC();
    }
    return p;
}

/* word access to a heap block through a handle, little-endian as the 386 */
static uint16_t blk16(uint32_t h)
{
    const uint8_t *p = blkp(h);
    return (uint16_t)(p[0] | p[1] << 8);
}

static int32_t sblk16(uint32_t h)                   /* movsx eax, word ptr [h] */
{
    return (int16_t)blk16(h);
}

static void setblk16(uint32_t h, uint16_t v)
{
    uint8_t *p = blkp(h);
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
}

/* the low word / the dword a fread(&local, 2 or 4, 1, fp) left in a stack local (kept as bytes, little-endian) */
static uint16_t le16(const uint8_t *b) { return (uint16_t)(b[0] | b[1] << 8); }
static uint32_t le32(const uint8_t *b)
{
    return (uint32_t)b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 | (uint32_t)b[3] << 24;
}

/* ---- 0x11A8C: the game sprites ---------------------------------------------------------------------------------- */

/* 0x11A8C: show c_invld.gif, tokens2.gif, tokens.gif and cardtiny.gif (ACIV.CT2) one after the other at (0, 0) in the
   back buffer [0xF52C8] and cut 273 sprites out of them with f3AE90 (spr eax, x edx, y ebx, w ecx, h stack). The
   arrays of 14 records are indexed 1..13 (or 1..9), record i at base + 8*i (shl 3, D). Called from main 0x115BA. */
void f11A8C(void)
{
    int32_t i;                                                  /* [ebp-0x18] */

    fg_vbopen((int)D32(0xF52C8u));                              /* 0x11AA8 the back buffer */
    fg_move(0, 0);                                              /* 0x11AB4 */
    cat_show(DSTR(0xB0D73u), 3, (int)D32(0xF40D8u));            /* 0x11ACA "c_invld.gif", flags 3, CT2 */
    f3AE90(0xF5214u, 0x4F, 0x1BE, 0x1E5, 0x22);                 /* 0x11AE8 (79, 446) 485 x 34 */
    f3AE90(0xF521Cu, 0x37, 0x18, 0x74, 0x19);                   /* 0x11B03 (55, 24) 116 x 25 */
    f3AE90(0xF5224u, 0x11, 0x90, 0xFB, 0x19);                   /* 0x11B1E (17, 144) 251 x 25 */

    fg_move(0, 0);                                              /* 0x11B27 */
    cat_show(DSTR(0xB0D7Fu), 3, (int)D32(0xF40D8u));            /* 0x11B42 "tokens2.gif" */
    for (i = 1; i <= 0xD; i++) {                                /* 0x11B4A, 0x11B55 cmp 0xD; jg */
        /* x0 = ((i-1)*3*8 + (i-1))*2 + 10 = 50*(i-1) + 10 (0x11B62..0x11B73); x1 = (i-1)*5*4 + 14 (0x11BC8..0x11BD5) */
        f3AE90(0xF489Cu + 8u * (uint32_t)i, 50 * (i - 1) + 10, 0x32, 0x12, 0x12);    /* 0x11B8D */
        f3AE90(0xF497Cu + 8u * (uint32_t)i, 50 * (i - 1) + 10, 0x55, 0xF, 0xF);      /* 0x11BC0 */
        f3AE90(0xF4E3Cu + 8u * (uint32_t)i, 20 * (i - 1) + 14, 0x1C7, 0xB, 0xA);     /* 0x11BEF */
        f3AE90(0xF4DCCu + 8u * (uint32_t)i, 20 * (i - 1) + 14, 0x1B9, 0xB, 0xA);     /* 0x11C1E */
        f3AE90(0xF4D5Cu + 8u * (uint32_t)i, 20 * (i - 1) + 14, 0x1AB, 0xB, 0xA);     /* 0x11C4D */
        if (i <= 9)                                                                  /* 0x11C52 cmp 9; jg */
            f3AE90(0xF490Cu + 8u * (uint32_t)i, 50 * (i - 1) + 10, 0xF, 0x1C, 0x12); /* 0x11C8A */
    }

    fg_move(0, 0);                                              /* 0x11C98 */
    cat_show(DSTR(0xB0D8Bu), 3, (int)D32(0xF40D8u));            /* 0x11CAE "tokens.gif" */
    for (i = 1; i < 0xE; i++) {                                 /* 0x11CBB, 0x11CC6 cmp 0xE; jge */
        f3AE90(0xF4ACCu + 8u * (uint32_t)i, 50 * (i - 1) + 10, 0x32, 0x18, 0x17);    /* 0x11CFE */
        f3AE90(0xF4BACu + 8u * (uint32_t)i, 50 * (i - 1) + 10, 0x55, 0x18, 0x17);    /* 0x11D31 */
        f3AE90(0xF4B3Cu + 8u * (uint32_t)i, 50 * (i - 1) + 10, 0xF, 0x28, 0x17);     /* 0x11D64 */
        f3AE90(0xF49ECu + 8u * (uint32_t)i, 50 * (i - 1) + 10, 0x82, 0x17, 0x17);    /* 0x11D97 */
        if (i <= 9)                                                                  /* 0x11D9F cmp 9; jg */
            f3AE90(0xF4A5Cu + 8u * (uint32_t)i, 50 * (i - 1) + 10, 0xAA, 0x23, 0x23);/* 0x11DD6 */
    }
    f3AE90(0xF4D2Cu, 0x48, 0x16A, 0xD, 0xD);                    /* 0x11DF6 */
    f3AE90(0xF4D34u, 0x58, 0x16A, 0xD, 0xD);                    /* 0x11E11 */
    f3AE90(0xF4D3Cu, 0x68, 0x16A, 0xD, 0xD);                    /* 0x11E2C */
    f3AE90(0xF4D44u, 0x78, 0x16A, 0xD, 0xD);                    /* 0x11E47 */
    f3AE90(0xF4D4Cu, 0x88, 0x16A, 0xD, 0xD);                    /* 0x11E62 */
    f3AE90(0xF4D54u, 0x98, 0x16A, 0xD, 0xD);                    /* 0x11E7D */
    f3AE90(0xF518Cu, 0xC8, 0xE1, 0x36, 0x1C);                   /* 0x11E98 */
    f3AE90(0xF5194u, 0xC8, 0x118, 0x36, 0x1C);                  /* 0x11EB3 */
    f3AE90(0xF519Cu, 0xCF, 0x147, 0x1B, 0xD);                   /* 0x11ECE */
    f3AE90(0xF51A4u, 0xCF, 0x163, 0x1B, 0xD);                   /* 0x11EE9 */
    f3AE90(0xF517Cu, 0x10, 0x127, 0x1E, 0x14);                  /* 0x11F04 */
    f3AE90(0xF5184u, 0x11, 0x128, 0x1C, 0x12);                  /* 0x11F1F */
    for (i = 0; i < 0xA; i++) {                                 /* 0x11F26, 0x11F31 cmp 0xA; jge */
        /* y = i*5*2 + 0x13C (0x11F3E..0x11F49), h 9; record +8*i, +0x50 or +0xA0 on top (0x11F8C, 0x11FBF) */
        f3AE90(0xF4EACu + 8u * (uint32_t)i, 0x16C, 10 * i + 0x13C, 9, 9);                 /* 0x11F66 */
        f3AE90(0xF4EACu + 0x50u + 8u * (uint32_t)i, 0x17A, 10 * i + 0x13C, 0xB, 9);       /* 0x11F99 */
        f3AE90(0xF4EACu + 0xA0u + 8u * (uint32_t)i, 0x188, 10 * i + 0x13C, 0xD, 9);       /* 0x11FCE */
        f3AE90(0xF4F9Cu + 8u * (uint32_t)i, 0x19E, 10 * i + 0x13C, 9, 9);                 /* 0x11FFE */
        f3AE90(0xF4F9Cu + 0x50u + 8u * (uint32_t)i, 0x1AC, 10 * i + 0x13C, 0xB, 9);       /* 0x12031 */
        f3AE90(0xF4F9Cu + 0xA0u + 8u * (uint32_t)i, 0x1BA, 10 * i + 0x13C, 0xD, 9);       /* 0x12066 */
        f3AE90(0xF508Cu + 8u * (uint32_t)i, 0x1CE, 10 * i + 0x13C, 0xB, 9);               /* 0x12096 */
        f3AE90(0xF508Cu + 0x50u + 8u * (uint32_t)i, 0x1DC, 10 * i + 0x13C, 0xD, 9);       /* 0x120C9 */
        f3AE90(0xF508Cu + 0xA0u + 8u * (uint32_t)i, 0x1EA, 10 * i + 0x13C, 0xF, 9);       /* 0x120FE */
    }

    fg_move(0, 0);                                              /* 0x1210C */
    cat_show(DSTR(0xB0D96u), 3, (int)D32(0xF40D8u));            /* 0x12127 "cardtiny.gif" */
    for (i = 1; i <= 0x21; i++)                                 /* 0x1212F, 0x1213A cmp 0x21; jg */
        f3AE90(0xF4C1Cu + 8u * (uint32_t)i,                     /* 0x12188 */
               (i - 1) % 11 * 40,                               /* 0x12168 idiv 11: remainder *5 *8 */
               (i - 1) / 11 * 60,                               /* 0x1214E idiv 11: quotient *15 *4 */
               0x20, 0x2E);
    fg_vbclose();                                               /* 0x1218F */
}

/* ---- 0x3EED8: adjacency lists, land matrix, N.RL ---------------------------------------------------------------- */

/* 0x3EED8: load landadj.dat, coastadj.dat and seaadj.dat (uint32 size, then 160 records int16 n, int16 t[n], copied
   word by word into a heap block; the record pointers go to 0xF8E7C / 0xF90FC / 0xF937C), drop the neighbours not in
   play (byte 0xEE16C[u] == 0) from every list, build the land matrix 0xF95FC[157][157], load <prefix><players>.RL and
   index its two tables by territory. Called from main after 0x12B00. */
void f3EED8(void)
{
    uint8_t size[4] = {0, 0, 0, 0};   /* [ebp-0x2c] = 0 (0x3EEEF); fread(&size, 4, 1, fp) for each file */
    uint8_t n[2] = {0, 0};            /* [ebp-0x18]: the low word of the dword local */
    uint8_t v[2] = {0, 0};            /* [ebp-0x1c] */
    uint8_t m[4] = {0, 0, 0, 0};      /* [ebp-0x24] */
    FILE *fp;                         /* [ebp-0x38] */
    uint32_t p;                       /* [ebp-0x28] (loading), [ebp-0x30] (pruning), [ebp-0x34] (matrix): a handle */
    int32_t k;                        /* [ebp-0x20] words stored in the block so far (read as a word, 0x3EF57) */
    int32_t t;                        /* [ebp-0x50] */
    int32_t i;                        /* [ebp-0x3c] */
    int32_t j;                        /* [ebp-0x4c] (loading), [ebp-0x40] (matrix) */
    int32_t e;                        /* [ebp-0x54] */
    int32_t s;                        /* [ebp-0x48] */
    int32_t cnt;                      /* [ebp-0x44] */
    char name[0x200];                 /* [ebp-0x68], 20 bytes in the original; the port's prefix is a longer path */

    /* landadj.dat -> [0xFFB54], 0xF8E7C */
    fp = fopen(DSTR(0xB3F8Fu), DSTR(0xB3F8Cu));                 /* 0x3EEFC fopen("landadj.dat", "rb") */
    if (fp == NULL) {                                           /* 0x3EF04 */
        f3A780(DSTR(0xB3F9Bu));                                 /* 0x3EF0F "Can't find landadj.dat\n" */
        f128BC(DSTR(0xB3F8Fu));   /* port: the original reads on from a NULL FILE (G: a crash); quit as 0x128BC does */
    }
    (void)fread(size, 4, 1, fp);                                /* 0x3EF24 */
    heap_alloc(GAME_LANDADJ_BLOCK, le32(size));                 /* 0x3EF2C malloc, 0x3EF31 [0xFFB54] */
    k = 0;                                                      /* 0x3EF38 */
    for (t = 0; t < 0xA0; t++) {                                /* 0x3EF3D, 0x3EF48 cmp 0xA0; jge */
        p = D32(GAME_LANDADJ_BLOCK) + 2u * (uint16_t)k;         /* 0x3EF57 mov bx, word [ebp-0x20]; add ebx,ebx */
        setD32(GAME_LANDADJ_PTR + 4u * (uint32_t)t, p);         /* 0x3EF67 */
        (void)fread(n, 2, 1, fp);                               /* 0x3EF8B */
        setblk16(p, le16(n));                                   /* 0x3EF96 */
        k++;                                                    /* 0x3EF9C */
        j = 1;                                                  /* 0x3EF9F */
        for (i = 0; (int16_t)le16(n) > i; i++) {                /* 0x3EFB3 movsx word [ebp-0x18]; jle */
            (void)fread(v, 2, 1, fp);                           /* 0x3EFCC */
            setblk16(p + 2u * (uint32_t)j, le16(v));            /* 0x3EFD1..0x3EFE1 */
            j++;
            k++;                                                /* 0x3EFE7 */
        }
    }
    fclose(fp);                                                 /* 0x3EFEF */

    /* coastadj.dat -> [0xFFB58], 0xF90FC */
    fp = fopen(DSTR(0xB3FB6u), DSTR(0xB3FB3u));                 /* 0x3EFFE fopen("coastadj.dat", "rb") */
    if (fp == NULL) {                                           /* 0x3F006 */
        f3A780(DSTR(0xB3FC3u));                                 /* 0x3F011 "Can't find coastadj.dat\n" */
        f128BC(DSTR(0xB3FB6u));                                 /* port, as above */
    }
    (void)fread(size, 4, 1, fp);                                /* 0x3F026 */
    heap_alloc(GAME_COASTADJ_BLOCK, le32(size));                /* 0x3F02E, 0x3F033 [0xFFB58] */
    k = 0;                                                      /* 0x3F03A */
    for (t = 0; t < 0xA0; t++) {                                /* 0x3F04A */
        p = D32(GAME_COASTADJ_BLOCK) + 2u * (uint16_t)k;        /* 0x3F059 mov ax, word [ebp-0x20]; lea ebx,[2*eax] */
        setD32(GAME_COASTADJ_PTR + 4u * (uint32_t)t, p);        /* 0x3F06F */
        (void)fread(n, 2, 1, fp);                               /* 0x3F093 */
        setblk16(p, le16(n));                                   /* 0x3F09E */
        k++;                                                    /* 0x3F0A4 */
        j = 1;                                                  /* 0x3F0A7 */
        for (i = 0; (int16_t)le16(n) > i; i++) {                /* 0x3F0BB */
            (void)fread(v, 2, 1, fp);                           /* 0x3F0D4 */
            setblk16(p + 2u * (uint32_t)j, le16(v));            /* 0x3F0D9..0x3F0E9 */
            j++;
            k++;                                                /* 0x3F0EF */
        }
    }
    fclose(fp);                                                 /* 0x3F0F7 */

    /* seaadj.dat -> [0xFFB5C], 0xF937C */
    fp = fopen(DSTR(0xB3FDFu), DSTR(0xB3FDCu));                 /* 0x3F106 fopen("seaadj.dat", "rb") */
    if (fp == NULL) {                                           /* 0x3F10E */
        f3A780(DSTR(0xB3FEAu));                                 /* 0x3F119 "Can't find seaadj.dat\n" */
        f128BC(DSTR(0xB3FDFu));                                 /* port, as above */
    }
    (void)fread(size, 4, 1, fp);                                /* 0x3F12E */
    heap_alloc(GAME_SEAADJ_BLOCK, le32(size));                  /* 0x3F136, 0x3F13B [0xFFB5C] */
    k = 0;                                                      /* 0x3F142 */
    for (t = 0; t < 0xA0; t++) {                                /* 0x3F152 */
        p = D32(GAME_SEAADJ_BLOCK) + 2u * (uint16_t)k;          /* 0x3F161 */
        setD32(GAME_SEAADJ_PTR + 4u * (uint32_t)t, p);          /* 0x3F171 */
        (void)fread(n, 2, 1, fp);                               /* 0x3F195 */
        setblk16(p, le16(n));                                   /* 0x3F1A0 */
        k++;                                                    /* 0x3F1A6 */
        j = 1;                                                  /* 0x3F1A9 */
        for (i = 0; (int16_t)le16(n) > i; i++) {                /* 0x3F1BD */
            (void)fread(v, 2, 1, fp);                           /* 0x3F1D6 */
            setblk16(p + 2u * (uint32_t)j, le16(v));            /* 0x3F1DB..0x3F1EB */
            j++;
            k++;                                                /* 0x3F1F1 */
        }
    }
    fclose(fp);                                                 /* 0x3F1F9 */

    /* Drop the neighbours u with byte [0xEE16C + u] == 0 from every list: shift the rest down, n - 1, and look at the
       same position again (dec then inc of [ebp-0x54]). The index u is the signed word (movsx). */
    for (i = 0; i < 0xA0; i++) {                                /* 0x3F200, 0x3F20B cmp 0xA0; jge */
        p = D32(GAME_LANDADJ_PTR + 4u * (uint32_t)i);           /* 0x3F21B */
        for (e = 1; sblk16(p) >= e; e++)                        /* 0x3F225, 0x3F26F cmp; jl */
            if (D8((uint32_t)(sblk16(p + 2u * (uint32_t)e) + (int32_t)GAME_INPLAY)) == 0) {   /* 0x3F282, 0x3F285 */
                for (s = e; sblk16(p) - 1 >= s; s++)            /* 0x3F292, 0x3F234 dec eax; cmp; jl */
                    setblk16(p + 2u * (uint32_t)s, blk16(p + 2u * (uint32_t)s + 2u));   /* 0x3F254 */
                setblk16(p, (uint16_t)(blk16(p) - 1));          /* 0x3F260 dec word [eax] */
                e--;                                            /* 0x3F266 */
            }
        p = D32(GAME_COASTADJ_PTR + 4u * (uint32_t)i);          /* 0x3F29A */
        for (e = 1; sblk16(p) >= e; e++)                        /* 0x3F2A4, 0x3F2EA */
            if (D8((uint32_t)(sblk16(p + 2u * (uint32_t)e) + (int32_t)GAME_INPLAY)) == 0) {   /* 0x3F2FD, 0x3F300 */
                for (s = e; sblk16(p) - 1 >= s; s++)            /* 0x3F30D, 0x3F2B3 */
                    setblk16(p + 2u * (uint32_t)s, blk16(p + 2u * (uint32_t)s + 2u));   /* 0x3F2D3 */
                setblk16(p, (uint16_t)(blk16(p) - 1));          /* 0x3F2DB */
                e--;                                            /* 0x3F2E1 */
            }
        p = D32(GAME_SEAADJ_PTR + 4u * (uint32_t)i);            /* 0x3F315 */
        for (e = 1; sblk16(p) >= e; e++)                        /* 0x3F31F, 0x3F365 */
            if (D8((uint32_t)(sblk16(p + 2u * (uint32_t)e) + (int32_t)GAME_INPLAY)) == 0) {   /* 0x3F37C, 0x3F37F */
                for (s = e; sblk16(p) - 1 >= s; s++)            /* 0x3F38C, 0x3F32E */
                    setblk16(p + 2u * (uint32_t)s, blk16(p + 2u * (uint32_t)s + 2u));   /* 0x3F34E */
                setblk16(p, (uint16_t)(blk16(p) - 1));          /* 0x3F356 */
                e--;                                            /* 0x3F35C */
            }
    }

    /* The land matrix: [a*157 + u] = 1 for each u left in land list a, a = 0..156 (a u above 156 lands in the next
       row, a negative one before it: the original indexes without a check) */
    memset(DP(GAME_LAND_MATRIX), 0, 0x6049);                    /* 0x3F39D memset(0xF95FC, 0, 0x6049) */
    for (s = 0; s < 0x9D; s++) {                                /* 0x3F3A4, 0x3F3AF cmp 0x9D; jge */
        p = D32(GAME_LANDADJ_PTR + 4u * (uint32_t)s);           /* 0x3F3BB [ebp-0x34] */
        cnt = sblk16(p);                                        /* 0x3F3C8 [ebp-0x44] */
        j = 1;                                                  /* 0x3F3CE [ebp-0x40] */
        for (i = 0; i < cnt; i++) {                             /* 0x3F3D7, 0x3F3E2 cmp; jge */
            int32_t u = sblk16(p + 2u * (uint32_t)j);           /* 0x3F3F5 movsx ebx, word */
            j++;                                                /* 0x3F3ED */
            setD8((uint32_t)(u + s * 0x9D + (int32_t)GAME_LAND_MATRIX), 1);   /* 0x3F3F8 imul 0x9D; 0x3F3FF */
        }
    }

    /* <prefix><players>.RL: uint32 m; 0x2084 bytes to 0xFFD44; 0x5644 bytes to 0x101DC8; m bytes to [0x10740C] */
    snprintf(name, sizeof name, DSTR(0xB4001u), DSTR(0xF5820u), (int)S32(0xF40F0u));   /* 0x3F41E "%s%d.RL" */
    fp = fopen(name, DSTR(0xB4009u));                           /* 0x3F42E "rb" */
    if (fp == NULL)                                             /* (no check in the original, 0x3F433) */
        f128BC(name);   /* port: the original reads on from a NULL FILE (G: a crash); quit as 0x128BC does */
    (void)fread(m, 4, 1, fp);                                   /* 0x3F446 */
    heap_alloc(GAME_RL_BLOCK, le32(m));                         /* 0x3F44E malloc, 0x3F453 [0x10740C] */
    (void)fread(DP(GAME_RL_TABLE1), 0x2084, 1, fp);             /* 0x3F46A fread(0xFFD44, 0x2084, 1, fp) */
    (void)fread(DP(GAME_RL_TABLE2), 0x5644, 1, fp);             /* 0x3F481 fread(0x101DC8, 0x5644, 1, fp) */
    (void)fread(load_heap_ptr(D32(GAME_RL_BLOCK), le32(m)), 1, le32(m), fp);   /* 0x3F496 fread([0x10740C], 1, m, fp) */
    fclose(fp);                                                 /* 0x3F49E */

    /* For each territory t, the first record of each table whose +0 byte is t, else -1 */
    for (i = 0; i < 0xA0; i++) {                                /* 0x3F4A5, 0x3F520 cmp 0xA0; jge */
        setD16(GAME_RL_FIRST1 + 2u * (uint32_t)i, 0xFFFF);      /* 0x3F531 */
        setD16(GAME_RL_FIRST2 + 2u * (uint32_t)i, 0xFFFF);      /* 0x3F53E */
        for (e = 0; e < S32(GAME_RL_TABLE1); e++)               /* 0x3F546, 0x3F4B3 cmp [0xFFD44]; jge */
            if ((int32_t)D8(0xFFD48u + 8u * (uint32_t)e) == i) {        /* 0x3F4C1, 0x3F4CD */
                setD16(GAME_RL_FIRST1 + 2u * (uint32_t)i, (uint16_t)e); /* 0x3F4D8 */
                break;
            }
        for (e = 0; e < S32(GAME_RL_TABLE2); e++)               /* 0x3F4E2, 0x3F4ED cmp [0x101DC8]; jge */
            if ((int32_t)D8(0x101DCCu + 8u * (uint32_t)e) == i) {       /* 0x3F4FB, 0x3F507 */
                setD16(GAME_RL_FIRST2 + 2u * (uint32_t)i, (uint16_t)e); /* 0x3F512 */
                break;
            }
    }
}

/* ---- 0x44974, 0x44800: territories not in play ------------------------------------------------------------------ */

/* 0x44800: recolour territory t's area on the active surface (main: the map buffer [0xF52C4]) through the table
   0xC2620. The mapdataz record [0xFF64C + 4*t]: int16 n, int16 y, one word not read, then n span pairs x1, x2. The row
   starts at y - 1; a pair with x1 >= 0 moves to the next row, x1 < 0 stays on the row and starts at -x1 (neg of the
   dword local, of which the word is read back, 0x44891/0x44894). Each pixel x1..x2: colour 0xC2620[fg_getpixel(x, row)]
   (fg_getpixel is -1 outside the clip region, so the original reads 0xC261F there; fg_point then draws nothing). */
void f44800(int t)
{
    uint32_t rec;                                               /* [ebp-0x20], a handle (port) */
    uint16_t n, y, x1, x2;                                      /* the low words of [ebp-0x18], [ebp-0x14], [ebp-0x10], [ebp-0x1c] */
    int32_t k, row, i, x;                                       /* [ebp-0x28], [ebp-0x2c], [ebp-0x24], [ebp-0x30] */

    rec = D32(GAME_MAPDATA_PTR + 4u * (uint32_t)t);             /* 0x44819 */
    /* Port guard (G): a record whose block was never loaded draws nothing (the original would read through its
       pointer). */
    if (load_heap_ptr(rec, 2) == NULL)
        return;
    n = blk16(rec);                                             /* 0x44826 */
    y = blk16(rec + 2u);                                        /* 0x4482F */
    k = 3;                                                      /* 0x44836 */
    row = (int16_t)y - 1;                                       /* 0x4483D movsx; dec */
    for (i = 0; (int16_t)n > i; i++) {                          /* 0x44845, 0x44852 movsx word [ebp-0x18]; jle */
        x1 = blk16(rec + 2u * (uint32_t)k);                     /* 0x4486A */
        k++;
        x2 = blk16(rec + 2u * (uint32_t)k);                     /* 0x4487B */
        k++;
        if ((int16_t)x1 >= 0)                                   /* 0x44881 movsx; test; jl */
            row++;                                              /* 0x4488C */
        else
            x1 = (uint16_t)(0u - x1);                           /* 0x44891 neg dword [ebp-0x10] */
        for (x = (int16_t)x1; (int16_t)x2 >= x; x++) {          /* 0x44894 movsx; 0x448A3 movsx word [ebp-0x1c]; jl */
            fg_setcolor(D8((uint32_t)(fg_getpixel(x, row) + (int32_t)0xC2620)));   /* 0x448B4, 0x448BC, 0x448C8 */
            fg_point(x, row);                                   /* 0x448D8 */
        }
    }
}

/* 0x44974: f44800(t) for every territory t with byte [0xEE16C + t] == 0 (not in play in this map variant) */
void f44974(void)
{
    int32_t t;                                                  /* [ebp-0x8] */

    for (t = 0; t < 0xA0; t++)                                  /* 0x44985, 0x44992 cmp 0xA0; jge */
        if (D8(GAME_INPLAY + (uint32_t)t) == 0)                 /* 0x4499E, 0x449A4 test dl,dl; jne */
            f44800(t);                                          /* 0x449AB */
}
