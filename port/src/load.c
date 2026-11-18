/* The map and table loaders of main's new-game path in AC_MAIN.EXE: terrflag.dat (0x12394, with its error exit
   0x128BC), the map geometry (0x3D170: mapdataz.dat 0x3EA7C, borders1/2.dat 0x3EC20), the map variant's in-play bytes
   (0x12B00), the area and territory-state tables (0x12B5C, 0x12BB8) and the play-by-email start (0x12C38).
   Literal translations; the address of each instruction a line comes from is in its comment. D = read in the code,
   G = inference. Prototypes and the data layouts: game.h.

   Runtime calls (Watcom C runtime -> standard C): 0x7D8C8 fopen, 0x7DB6D fread, 0x7D95E fclose, 0x7D5D0 memset,
   0x7DB3D sprintf (-> snprintf bounded to [0xF5500]'s 500 bytes, as title.c), 0x822B5 malloc (-> load_heap_alloc,
   calloc underneath). Every file is opened relative to the current directory with mode "rb" (the strings 0xB0DA3,
   0xB3F11, 0xB3F3A, 0xB3F63), as the original, which runs in the game folder. Watcom __CHK is dropped.

   Heap blocks (port, G). The original keeps malloc pointers in 32-bit data fields, and pointers INTO those blocks
   (block + 2*k). A 64-bit pointer does not fit, so the port keeps the blocks in its own table and stores a handle in
   the field: LOAD_HEAP_TAG | slot << 20 | byte offset. The offset sits in the low 20 bits, so handle + n is the
   handle of the byte n further on, exactly as pointer + n was in the original, and code that adds to a stored
   pointer can add to the handle the same way. load_heap_ptr(h, n) gives the C pointer for n bytes at h (NULL when
   they are not inside the block). The fields written here, which differ from a DOSBox-X dump by design:
     0xFF648 (mapdataz block), 0xFF64C..0xFF8CB (uint32[160], handles into it),
     0xFF8CC, 0xFF8D0 (borders1/2 blocks), 0xFF8D4..0xFFB47 (uint32[157], handles into them).
   load_heap_free is for the translation of 0x3F558, which frees these blocks (free 0x877E6) and zeroes the fields. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"

/* ---- Port: heap blocks behind 32-bit handles ------------------------------------------------------------------- */

#define LOAD_HEAP_TAG    0x80000000u
#define LOAD_HEAP_SHIFT  20
#define LOAD_HEAP_OFF    0x000FFFFFu   /* byte offset inside the block: blocks up to 1 MB (the largest here: 50,148) */
#define LOAD_HEAP_SLOTS  1024          /* slot numbers 1..1024 in bits 20..30 */

/* Not yet in game.h (Claude moves them there): the port's handle scheme for the heap pointers of this file, open to
   the other translations (0x3F558 frees, 0x44800 / 0x3D18C read the records). */
uint32_t load_heap_alloc(uint32_t size);
uint8_t *load_heap_ptr(uint32_t handle, uint32_t n);
void     load_heap_free(uint32_t handle);

static struct {
    uint8_t *p;
    uint32_t size;
} s_heap[LOAD_HEAP_SLOTS];

/* malloc(size) as the game calls it (0x822B5): a handle with offset 0, or 0 as Watcom's malloc returns NULL for 0
   (LIBRARY_CALLS.md) or when memory runs out. The block is zeroed (the original's is not; it lives outside the data
   object, so this changes nothing a dump compares). */
uint32_t load_heap_alloc(uint32_t size)
{
    uint32_t i;

    if (size == 0 || size > LOAD_HEAP_OFF + 1u)
        return 0;
    for (i = 0; i < LOAD_HEAP_SLOTS; i++)
        if (s_heap[i].p == NULL) {
            s_heap[i].p = calloc(1, size);
            if (s_heap[i].p == NULL)
                return 0;
            s_heap[i].size = size;
            return LOAD_HEAP_TAG | (i + 1u) << LOAD_HEAP_SHIFT;
        }
    return 0;
}

/* The C pointer for n bytes at handle h (block + offset), or NULL if h is no live block or the n bytes run past it. */
uint8_t *load_heap_ptr(uint32_t h, uint32_t n)
{
    uint32_t slot = (h & ~LOAD_HEAP_TAG) >> LOAD_HEAP_SHIFT;
    uint32_t off = h & LOAD_HEAP_OFF;

    if ((h & LOAD_HEAP_TAG) == 0 || slot < 1 || slot > LOAD_HEAP_SLOTS || s_heap[slot - 1].p == NULL)
        return NULL;
    if (off > s_heap[slot - 1].size || n > s_heap[slot - 1].size - off)
        return NULL;
    return s_heap[slot - 1].p + off;
}

/* free (0x877E6) of the block whose handle (offset 0) a data field holds; 0 or an unknown handle is ignored. */
void load_heap_free(uint32_t h)
{
    uint32_t slot = (h & ~LOAD_HEAP_TAG) >> LOAD_HEAP_SHIFT;

    if ((h & LOAD_HEAP_TAG) == 0 || (h & LOAD_HEAP_OFF) != 0 || slot < 1 || slot > LOAD_HEAP_SLOTS)
        return;
    free(s_heap[slot - 1].p);
    s_heap[slot - 1].p = NULL;
    s_heap[slot - 1].size = 0;
}

/* word [h] = v, the original's "mov word ptr [reg], ax" into a heap block. Port guard (G): a store outside the block
   (the file's data longer than its size field, or malloc failed) would corrupt the original's heap; the port quits
   with a message instead. The shipped files fit (D: mapdataz 50,148 bytes of records for a 50,148-byte block,
   borders1 38,090 for 38,094, borders2 51,850 for 89,948). */
static void put16(uint32_t h, uint16_t v, uint32_t name)
{
    uint8_t *p = load_heap_ptr(h, 2);

    if (p == NULL) {
        snprintf((char *)DP(0xF5500u), 0x1F4, "%s: data past its block", DSTR(name));
        f44CAC();
    }
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
}

/* fread(&local, 2, 1, fp) / fread(&local, 4, 1, fp) into a little-endian local: the bytes fread does not deliver (end
   of file) leave the local as it was, as in the original. */
static void fread16(uint16_t *v, FILE *fp)
{
    uint8_t b[2] = { (uint8_t)*v, (uint8_t)(*v >> 8) };

    (void)fread(b, 2, 1, fp);
    *v = (uint16_t)(b[0] | b[1] << 8);
}

static void fread32(uint32_t *v, FILE *fp)
{
    uint8_t b[4] = { (uint8_t)*v, (uint8_t)(*v >> 8), (uint8_t)(*v >> 16), (uint8_t)(*v >> 24) };

    (void)fread(b, 4, 1, fp);
    *v = (uint32_t)b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 | (uint32_t)b[3] << 24;
}

/* Territory record t's field at offset off (35-byte records at 0xE1638: lea [8t]; add t; shl 2; sub t, D 0x12442) */
#define TERR(t, off) (GAME_TERR + (uint32_t)GAME_TERR_SIZE * (uint32_t)(t) + (uint32_t)(off))

/* ---- 0x128BC: "Can't open file - %s" and quit ------------------------------------------------------------------- */

_Noreturn void f128BC(const char *name)
{
    snprintf((char *)DP(0xF5500u), 0x1F4, DSTR(0xB0DFBu), name);   /* 0x128DD sprintf(0xF5500, "Can't open file - %s\n", name) */
    f44CAC();                                                     /* 0x128E5 */
}

/* ---- 0x12394: load terrflag.dat ------------------------------------------------------------------------------- */

void f12394(void)
{
    FILE *fp;                 /* [ebp-0x20] */
    uint16_t version = 0;     /* [ebp-0x18] (port: 0 instead of the stack's leftover if the read fails) */
    int32_t t;                /* [ebp-0x1C] */

    fp = fopen(DSTR(0xB0DA6u), DSTR(0xB0DA3u));                   /* 0x123B3 fopen("terrflag.dat", "rb") */
    if (fp != NULL) {                                             /* 0x123BF je 0x12416 */
        fread16(&version, fp);                                    /* 0x123D1 fread(&version, 2, 1, fp) */
        if ((int16_t)version != 3) {                              /* 0x123D6 movsx; cmp eax,3 */
            snprintf((char *)DP(0xF5500u), 0x1F4, DSTR(0xB0DB3u), /* 0x123F0 "terrflag.dat is wrong version.  It is %d and should be %d\n" */
                     (int)(int16_t)version, 3);
            f44CAC();                                             /* 0x123F8 */
        }
        (void)fread(DP(GAME_TERR), 0x23, 0xA0, fp);               /* 0x1240F fread(0xE1638, 35, 160, fp) */
    } else {
        f128BC(DSTR(0xB0DEEu));                                   /* 0x1241B "terrflag.dat": does not return */
    }
    fclose(fp);                                                   /* 0x12425 */

    for (t = 0; t < 0xA0; t++) {                                  /* 0x12435 */
        /* point 0: overview x, then +8; overview y, then +6 (signed idiv, the word stores truncate) */
        setD16(0xEE270u + 2u * (uint32_t)t,                       /* 0x12480 x0 * 624 / 1716 */
               (uint16_t)((int32_t)S16(TERR(t, GAME_TERR_X0)) * 624 / 1716));
        setD16(TERR(t, GAME_TERR_X0), (uint16_t)(D16(TERR(t, GAME_TERR_X0)) + 8));   /* 0x12499 add word [rec+3],8 */
        setD16(0xEE3B0u + 2u * (uint32_t)t,                       /* 0x124D8 y0 * 319 / 568 */
               (uint16_t)((int32_t)S16(TERR(t, GAME_TERR_Y0)) * 319 / 568));
        setD16(TERR(t, GAME_TERR_Y0), (uint16_t)(D16(TERR(t, GAME_TERR_Y0)) + 6));   /* 0x124F1 add word [rec+5],6 */
        /* point 1 */
        setD16(0xEE770u + 2u * (uint32_t)t,                       /* 0x12537 x1 * 624 / 1716 */
               (uint16_t)((int32_t)S16(TERR(t, GAME_TERR_X1)) * 624 / 1716));
        setD16(TERR(t, GAME_TERR_X1), (uint16_t)(D16(TERR(t, GAME_TERR_X1)) + 8));   /* 0x12550 */
        setD16(0xEE8B0u + 2u * (uint32_t)t,                       /* 0x12591 y1 * 319 / 568 */
               (uint16_t)((int32_t)S16(TERR(t, GAME_TERR_Y1)) * 319 / 568));
        setD16(TERR(t, GAME_TERR_Y1), (uint16_t)(D16(TERR(t, GAME_TERR_Y1)) + 6));   /* 0x125AA */
        /* point 2 */
        setD16(0xEE4F0u + 2u * (uint32_t)t,                       /* 0x125EE x2 * 624 / 1716 */
               (uint16_t)((int32_t)S16(TERR(t, GAME_TERR_X2)) * 624 / 1716));
        setD16(TERR(t, GAME_TERR_X2), (uint16_t)(D16(TERR(t, GAME_TERR_X2)) + 8));   /* 0x12607 */
        setD16(0xEE630u + 2u * (uint32_t)t,                       /* 0x12648 y2 * 319 / 568 */
               (uint16_t)((int32_t)S16(TERR(t, GAME_TERR_Y2)) * 319 / 568));
        setD16(TERR(t, GAME_TERR_Y2), (uint16_t)(D16(TERR(t, GAME_TERR_Y2)) + 6));   /* 0x12661 */
        /* point 3: offsets only */
        setD16(TERR(t, GAME_TERR_X3), (uint16_t)(D16(TERR(t, GAME_TERR_X3)) + 8));   /* 0x1267A */
        setD16(TERR(t, GAME_TERR_Y3), (uint16_t)(D16(TERR(t, GAME_TERR_Y3)) + 6));   /* 0x12693 */
    }

    /* x3 = 5050 (0x13BA) in territories 17, 35, 51, 77, 89, 125, 134, 60 (0x126A0..0x126D8, in the code's order) */
    setD16(0xE189Au, 0x13BA);                                     /* 0x126A7 t 17  +0x0F */
    setD16(0xE1B10u, 0x13BA);                                     /* 0x126AE t 35 */
    setD16(0xE1D40u, 0x13BA);                                     /* 0x126B5 t 51 */
    setD16(0xE20CEu, 0x13BA);                                     /* 0x126BC t 77 */
    setD16(0xE2272u, 0x13BA);                                     /* 0x126C3 t 89 */
    setD16(0xE275Eu, 0x13BA);                                     /* 0x126CA t 125 */
    setD16(0xE2899u, 0x13BA);                                     /* 0x126D1 t 134 */
    setD16(0xE1E7Bu, 0x13BA);                                     /* 0x126D8 t 60 */

    for (t = 0; t < 0xA0; t++) {                                  /* 0x126EA */
        setD8(TERR(t, GAME_TERR_FLAGS), (uint8_t)(D8(TERR(t, GAME_TERR_FLAGS)) & ~0x02));   /* 0x12704 and byte [rec+2],-3 */
        setD8(TERR(t, GAME_TERR_FLAGS), (uint8_t)(D8(TERR(t, GAME_TERR_FLAGS)) & ~0x04));   /* 0x1271C and byte [rec+2],-5 */
    }

    /* flag 0x02 (0x12725..0x12794) */
    setD8(0xE188Du, (uint8_t)(D8(0xE188Du) | 0x02));              /* 0x12747 t 17  +0x02 */
    setD8(0xE1D33u, (uint8_t)(D8(0xE1D33u) | 0x02));              /* 0x1274D t 51 */
    setD8(0xE2265u, (uint8_t)(D8(0xE2265u) | 0x02));              /* 0x12753 t 89 */
    setD8(0xE288Cu, (uint8_t)(D8(0xE288Cu) | 0x02));              /* 0x12758 t 134 */
    setD8(0xE1B03u, (uint8_t)(D8(0xE1B03u) | 0x02));              /* 0x12782 t 35 */
    setD8(0xE20C1u, (uint8_t)(D8(0xE20C1u) | 0x02));              /* 0x12788 t 77 */
    setD8(0xE2751u, (uint8_t)(D8(0xE2751u) | 0x02));              /* 0x1278E t 125 */
    setD8(0xE1E6Eu, (uint8_t)(D8(0xE1E6Eu) | 0x02));              /* 0x12794 t 60 */
    /* flag 0x04 (0x1279A..0x12836) */
    setD8(0xE1CEDu, (uint8_t)(D8(0xE1CEDu) | 0x04));              /* 0x127BC t 49 */
    setD8(0xE21B6u, (uint8_t)(D8(0xE21B6u) | 0x04));              /* 0x127C2 t 84 */
    setD8(0xE1E28u, (uint8_t)(D8(0xE1E28u) | 0x04));              /* 0x127C8 t 58 */
    setD8(0xE2288u, (uint8_t)(D8(0xE2288u) | 0x04));              /* 0x127CD t 90 */
    setD8(0xE1D10u, (uint8_t)(D8(0xE1D10u) | 0x04));              /* 0x127F7 t 50 */
    setD8(0xE21D9u, (uint8_t)(D8(0xE21D9u) | 0x04));              /* 0x127FD t 85 */
    setD8(0xE1E4Bu, (uint8_t)(D8(0xE1E4Bu) | 0x04));              /* 0x12803 t 59 */
    setD8(0xE22ABu, (uint8_t)(D8(0xE22ABu) | 0x04));              /* 0x1280F t 91 */
    setD8(0xE207Bu, (uint8_t)(D8(0xE207Bu) | 0x04));              /* 0x1281E t 75 */
    setD8(0xE20E4u, (uint8_t)(D8(0xE20E4u) | 0x04));              /* 0x12830 t 78 */
    setD8(0xE209Eu, (uint8_t)(D8(0xE209Eu) | 0x04));              /* 0x12836 t 76 */

    memset(DP(0xF4234u), 0, 0x5A0);                               /* 0x1284A memset(0xF4234, 0, 0x5A0) */
    setD8(0xF4360u, 1);                                           /* 0x1284F +300 */
    setD8(0xF4365u, 1);                                           /* 0x12855 +305 */
    setD8(0xF4414u, 1);                                           /* 0x1285B +480 */
    setD8(0xF4415u, 1);                                           /* 0x12861 +481 */
    setD8(0xF441Eu, 1);                                           /* 0x12867 +490 */
    setD8(0xF4556u, 1);                                           /* 0x1286D +802 */
    setD8(0xF4557u, 1);                                           /* 0x12873 +803 */
    setD8(0xF4558u, 1);                                           /* 0x12879 +804 */
    setD8(0xF4559u, 1);                                           /* 0x1287F +805 */
    setD8(0xF45F6u, 1);                                           /* 0x12885 +962 */
    setD8(0xF45F7u, 1);                                           /* 0x1288B +963 */
    setD8(0xF45F8u, 1);                                           /* 0x12891 +964 */
    setD8(0xF45F9u, 1);                                           /* 0x12897 +965 */
    setD8(0xF469Au, 1);                                           /* 0x1289D +1126 */
    setD8(0xF46B3u, 1);                                           /* 0x128A3 +1151 */
    setD8(0xF47ACu, 1);                                           /* 0x128A9 +1400 */
}

/* ---- 0x3EA7C: load mapdataz.dat --------------------------------------------------------------------------------

   Port guard (G) for a file that does not open: the original prints "Can't find mapdataz.dat" (f3A780: printf, then
   waits for a key) and reads on from a NULL FILE, which is undefined (G: a crash or garbage). The port prints the
   same message and then quits through the game's own "Can't open file - %s" exit (f128BC). The same in 0x3EC20. */

void f3EA7C(void)
{
    FILE *fp;                 /* [ebp-0x30] */
    uint32_t size = 0;        /* [ebp-0x2C] */
    uint32_t k = 0;           /* [ebp-0x1C]: words stored so far; only its low 16 bits form the record offset */
    uint32_t rec;             /* [ebp-0x28]: the record's handle (pointer in the original) */
    uint16_t n = 0, y = 0, z = 0;   /* [ebp-0x10], [ebp-0x24], [ebp-0x14] (only their low words are read) */
    uint16_t x1 = 0, x2 = 0;  /* [ebp-0x18], [ebp-0x20] */
    int32_t t;                /* [ebp-0x38] */
    int32_t j;                /* [ebp-0x3C] */
    int32_t idx;              /* [ebp-0x34]: word index inside the record */

    fp = fopen(DSTR(0xB3F14u), DSTR(0xB3F11u));                   /* 0x3EA9E fopen("mapdataz.dat", "rb") */
    if (fp == NULL) {                                             /* 0x3EAAA */
        f3A780(DSTR(0xB3F21u));                                   /* 0x3EAB1 "Can't find mapdataz.dat\n" */
        f128BC(DSTR(0xB3F14u));                                   /* port guard (see above) */
    }
    fread32(&size, fp);                                           /* 0x3EAC6 fread(&size, 4, 1, fp) */
    setD32(GAME_MAPDATA_BLOCK, load_heap_alloc(size));            /* 0x3EACE malloc(size), 0x3EAD3 [0xFF648] */
    k = 0;                                                        /* 0x3EADA */
    for (t = 0; t < 0xA0; t++) {                                  /* 0x3EAEA */
        rec = D32(GAME_MAPDATA_BLOCK) + 2u * (uint16_t)k;         /* 0x3EAF9 mov ax,word [ebp-0x1C]; lea [2*eax]; add */
        setD32(GAME_MAPDATA_PTR + 4u * (uint32_t)t, rec);         /* 0x3EB0E [4*t + 0xFF64C] */
        fread16(&n, fp);                                          /* 0x3EB32 n */
        fread16(&y, fp);                                          /* 0x3EB47 y */
        fread16(&z, fp);                                          /* 0x3EB5C third word */
        put16(rec + 0, n, 0xB3F14u);                              /* 0x3EB67 */
        k++;                                                      /* 0x3EB6D */
        put16(rec + 2, y, 0xB3F14u);                              /* 0x3EB76 */
        k++;                                                      /* 0x3EB7D */
        put16(rec + 4, z, 0xB3F14u);                              /* 0x3EB86 */
        k++;                                                      /* 0x3EB8D */
        idx = 3;                                                  /* 0x3EB90 */
        for (j = 0; (int16_t)n > j; j++) {                        /* 0x3EBA4 movsx word n; cmp; jle */
            fread16(&x1, fp);                                     /* 0x3EBC1 */
            fread16(&x2, fp);                                     /* 0x3EBD6 */
            put16(rec + 2u * (uint32_t)idx++, x1, 0xB3F14u);      /* 0x3EBDB..0x3EBEB */
            k++;                                                  /* 0x3EBF1 */
            put16(rec + 2u * (uint32_t)idx++, x2, 0xB3F14u);      /* 0x3EBF4..0x3EC04 */
            k++;                                                  /* 0x3EC0A */
        }
    }
    fclose(fp);                                                   /* 0x3EC12 */
}

/* ---- 0x3EC20: load borders1.dat (records 0..64) and borders2.dat (65..156) ------------------------------------- */

/* A border value as stored (0x3ED60..0x3ED6B, 0x3ED1A..0x3ED22; the same at 0x3EEAA..): v >= 0 -> v + 8, v < 0 -> v - 8
   (neg; add 8; neg), the low word kept. */
static uint16_t border_value(uint16_t v)
{
    if ((int16_t)v < 0)                                           /* movsx; test; jl */
        return (uint16_t)(v - 8);
    return (uint16_t)(v + 8);
}

void f3EC20(void)
{
    FILE *fp;                 /* [ebp-0x28] */
    uint32_t size = 0;        /* [ebp-0x24]: set to 0 once (0x3EC35); borders2 reuses it */
    uint32_t k;               /* [ebp-0x10]: words stored in the current block; low 16 bits form the offset */
    uint32_t rec;             /* [ebp-0x20] */
    uint16_t n = 0, y = 0, v = 0;   /* [ebp-0x18], [ebp-0x1C], [ebp-0x14] */
    int32_t r;                /* [ebp-0x30] */
    int32_t j;                /* [ebp-0x34] */
    int32_t idx;              /* [ebp-0x2C] */

    fp = fopen(DSTR(0xB3F3Du), DSTR(0xB3F3Au));                   /* 0x3EC42 fopen("borders1.dat", "rb") */
    if (fp == NULL) {                                             /* 0x3EC4E */
        f3A780(DSTR(0xB3F4Au));                                   /* 0x3EC55 "Can't find borders1.dat\n" */
        f128BC(DSTR(0xB3F3Du));                                   /* port guard (see 0x3EA7C) */
    }
    fread32(&size, fp);                                           /* 0x3EC6A */
    setD32(GAME_BORDERS1_BLOCK, load_heap_alloc(size));           /* 0x3EC72 malloc, 0x3EC77 [0xFF8CC] */
    k = 0;                                                        /* 0x3EC7E */
    for (r = 0; r < 0x41; r++) {                                  /* 0x3EC8E */
        rec = D32(GAME_BORDERS1_BLOCK) + 2u * (uint16_t)k;        /* 0x3EC9A mov cx,word [ebp-0x10]; add ecx,ecx; add */
        setD32(GAME_BORDER_PTR + 4u * (uint32_t)r, rec);          /* 0x3ECAA [4*r + 0xFF8D4] */
        fread16(&n, fp);                                          /* 0x3ECCE */
        fread16(&y, fp);                                          /* 0x3ECE3 */
        put16(rec + 0, n, 0xB3F3Du);                              /* 0x3ECEE */
        k++;                                                      /* 0x3ECF4 */
        put16(rec + 2, (uint16_t)(y + 6), 0xB3F3Du);              /* 0x3ECF7..0x3ED02 y + 6 */
        k++;                                                      /* 0x3ED09 */
        idx = 2;                                                  /* 0x3ED0C */
        for (j = 0; (int16_t)n > j; j++) {                        /* 0x3ED3E movsx word n; cmp; jle */
            fread16(&v, fp);                                      /* 0x3ED5B */
            put16(rec + 2u * (uint32_t)idx++, border_value(v), 0xB3F3Du);   /* 0x3ED24..0x3ED2F */
            k++;                                                  /* 0x3ED35 */
        }
    }
    fclose(fp);                                                   /* 0x3ED73 */

    fp = fopen(DSTR(0xB3F66u), DSTR(0xB3F63u));                   /* 0x3ED82 fopen("borders2.dat", "rb") */
    if (fp == NULL) {                                             /* 0x3ED8E */
        f3A780(DSTR(0xB3F73u));                                   /* 0x3ED95 "Can't find borders2.dat\n" */
        f128BC(DSTR(0xB3F66u));                                   /* port guard (see 0x3EA7C) */
    }
    fread32(&size, fp);                                           /* 0x3EDAA */
    setD32(GAME_BORDERS2_BLOCK, load_heap_alloc(size));           /* 0x3EDB2 malloc, 0x3EDB7 [0xFF8D0] */
    k = 0;                                                        /* 0x3EDBE */
    for (r = 0x41; r < 0x9D; r++) {                               /* 0x3EDC1, 0x3EDD0 */
        rec = D32(GAME_BORDERS2_BLOCK) + 2u * (uint16_t)k;        /* 0x3EDDF mov ax,word [ebp-0x10]; lea [2*eax]; add */
        setD32(GAME_BORDER_PTR + 4u * (uint32_t)r, rec);          /* 0x3EDF4 */
        fread16(&n, fp);                                          /* 0x3EE18 */
        fread16(&y, fp);                                          /* 0x3EE2D */
        put16(rec + 0, n, 0xB3F66u);                              /* 0x3EE38 */
        k++;                                                      /* 0x3EE3E */
        put16(rec + 2, (uint16_t)(y + 6), 0xB3F66u);              /* 0x3EE41..0x3EE4C */
        k++;                                                      /* 0x3EE53 */
        idx = 2;                                                  /* 0x3EE56 */
        for (j = 0; (int16_t)n > j; j++) {                        /* 0x3EE88 */
            fread16(&v, fp);                                      /* 0x3EEA5 */
            put16(rec + 2u * (uint32_t)idx++, border_value(v), 0xB3F66u);   /* 0x3EE6E..0x3EE79 / 0x3EEB8..0x3EEC3 */
            k++;                                                  /* 0x3EE7F */
        }
    }
    fclose(fp);                                                   /* 0x3EECB */
}

/* ---- 0x3D170: the map geometry -------------------------------------------------------------------------------- */

void f3D170(void)
{
    f3EA7C();                                                     /* 0x3D17D */
    f3EC20();                                                     /* 0x3D182 */
}

/* ---- 0x12B00: in play in this map variant --------------------------------------------------------------------- */

void f12B00(void)
{
    int32_t t;                                                    /* [ebp-0x4] */

    for (t = 0; t < 0xA0; t++)                                    /* 0x12B1E */
        setD8(GAME_INPLAY + (uint32_t)t,                          /* 0x12B4C byte [t + 0xEE16C] */
              D8(TERR(t, GAME_TERR_INPLAY) + (uint32_t)(int32_t)S16(GAME_MAP_VARIANT)));   /* 0x12B3A dword [0xF6112] sar 16; 0x12B42 [35t + v + 0xE164D] */
}

/* ---- 0x12B5C: the area records -------------------------------------------------------------------------------- */

void f12B5C(void)
{
    int32_t r;                                                    /* [ebp-0xC] */

    memset(DP(GAME_AREA), 0, 0x2B65);                             /* 0x12B7A memset(0xE31B8, 0, 0x2B65) */
    for (r = 0; r < 0xA1; r++)                                    /* 0x12B8C */
        setD8(GAME_AREA + (uint32_t)GAME_AREA_SIZE * (uint32_t)r, (uint8_t)r);   /* 0x12BA7 byte [69r + 0xE31B8] = r */
}

/* ---- 0x12BB8: the 3-byte territory records -------------------------------------------------------------------- */

void f12BB8(void)
{
    int32_t t;                                                    /* [ebp-0x4] */
    uint32_t a;

    for (t = 0; t < 0xA0; t++) {                                  /* 0x12BD7 */
        a = GAME_TSTATE + 3u * (uint32_t)t;                       /* lea [4t]; sub t */
        setD8(a + 2, (uint8_t)(D8(a + 2) & ~0x01));               /* 0x12BEC and byte [3t + 0xE2C1A],-2 */
        setD8(a + 2, (uint8_t)(D8(a + 2) & ~0x02));               /* 0x12BFF and byte [..],-3 */
        setD8(a + 0, 0);                                          /* 0x12C14 byte [3t + 0xE2C18] = 0 */
        setD8(a + 2, (uint8_t)(D8(a + 2) & 0x03));                /* 0x12C26 and byte [..],3 */
    }
}

/* ---- 0x12C38: the play-by-email start --------------------------------------------------------------------------

   Its only caller is main's play-by-email branch (0x1187F), which the port leaves out (Erik's ruling, OVERVIEW §7.5),
   so game_load does not call it. Translated for completeness. Player record p: 32 bytes at 0xF4100 + 32*p (G), int8
   nation at +4 (dword [32p + 0xF4101] sar 24), uint8 territory at +5. */

void f12C38(void)
{
    int32_t p;                                                    /* [ebp-0x4] */
    int32_t nat;                                                  /* the nation, re-read at every use in the original */
    uint32_t e;                                                   /* territory + 1 = the area record */
    uint32_t na;                                                  /* the nation record */

    for (p = 0; p < S32(UI_PLAYERS_A); p++) {                     /* 0x12C57 cmp [0xF40EC]; jge */
        nat = S8(0xF4104u + 32u * (uint32_t)p);                   /* 0x12C6C dword [32p + 0xF4101] sar 24 */
        na = GAME_NATION + (uint32_t)(nat * GAME_NATION_SIZE);    /* imul eax,eax,0xC7 */
        setD8(na + 0x9A, 3);                                      /* 0x12C7B byte [199n + 0xF1566] = 3 */
        setD8(na + 0x99, (uint8_t)(D8(na + 0x99) - 1));           /* 0x12C97 dec byte [199n + 0xF1565] */
        setD8(na + 0x98, (uint8_t)(D8(na + 0x98) + 1));           /* 0x12CB2 inc byte [199n + 0xF1564] */
        setD8(na + 0x60, (uint8_t)(D8(na + 0x60) + 1));           /* 0x12CCD inc byte [199n + 0xF152C] */
        setD8(na + 0x61, (uint8_t)(D8(0xF4105u + 32u * (uint32_t)p) + 1));   /* 0x12CD9..0x12CF6 [199n + 0xF152D] = terr + 1 */
        e = (uint32_t)D8(0xF4105u + 32u * (uint32_t)p) + 1u;      /* 0x12D02..0x12D0D movzx; lea edx,[eax+1] */
        setD8(GAME_AREA + GAME_AREA_SIZE * e + 1,                 /* 0x12D17 inc byte [69e + 0xE31B9] */
              (uint8_t)(D8(GAME_AREA + GAME_AREA_SIZE * e + 1) + 1));
        setD8(GAME_AREA + GAME_AREA_SIZE * e + 1 + (uint32_t)nat,  /* 0x12D5B [69e + n + 0xE31B9] = byte [32p + 0xF4104] */
              D8(0xF4104u + 32u * (uint32_t)p));
        setD8(GAME_AREA + GAME_AREA_SIZE * e + 0x0E + (uint32_t)nat, 1);   /* 0x12D90 [69e + n + 0xE31C6] = 1 */
    }
}
