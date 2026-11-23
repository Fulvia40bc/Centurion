/* The start of a new game in AC_MAIN.EXE: main's loading part 0x113FE..0x1184A (game_load), the routines of that part
   that no other file translates (0x12DA4, 0x128F0, 0x7784C, 0x7892C), the trade-card stacks (0x6BA20 with the shuffle
   0x76F70, the insert 0x77184 and game_rand 0x78AE4), the nation records (0x45D30) and the music start (0x7B854 with
   the AIL file helpers 0x7D2E8, 0x7D334, 0x7CB9D, 0x7CCFD). Literal translations; the address of each instruction a
   line comes from is in its comment. D = read in the code, G = inference. Prototypes and the data layouts: game.h.

   Watcom runtime -> standard C: 0x7D5D0 memset, 0x7D610 strcmp, 0x7DB3D sprintf -> snprintf, 0x822B5 malloc,
   [0xCAC70] (AIL's free hook) free, the inline 2-bytes-a-step string copies strcpy, the DOS open/filelength/read/close
   of AIL's file helpers (0x93F92, 0x941E5, 0x93E64, 0x941E0) fopen/fseek/ftell/fread/fclose, _dos_gettime (0x87E29)
   timespec_get + localtime. The stack check __CHK is dropped, the debug log 0x78B0C is a no-op (left out). */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "game.h"
#include "fg.h"
#include "snd.h"

/* ---- Heap blocks of this file (port) ---------------------------------------------------------------------------
   [0xF626C] holds the loaded XMI file's heap block in the original (0x7B932, 0x7B954). A 64-bit pointer does not fit
   there, so the port keeps the block here and stores a handle in the field: 0 = none, 1 = s_heap_F626C (the only
   block this field ever holds at a time: the old one is freed before a new one is stored, 0x7B8FE). */
static uint8_t *s_heap_F626C;
static size_t s_heap_F626C_len;                       /* its length, for the port's sound (snd_init_sequence) */

static uint32_t heap_F626C_put(uint8_t *block)
{
    s_heap_F626C = block;
    return block != NULL ? 1u : 0u;
}

static uint8_t *heap_F626C_get(uint32_t handle)
{
    return handle == 1u ? s_heap_F626C : NULL;
}

/* 0x822B5 Watcom malloc (also AIL's allocation hook [0xCAC6C]): NULL for a size of 0 or above 0xFFFFFFD4 unsigned
   (LIBRARY_CALLS: "returns NULL for 0 or more than -0x2C"), unlike the host's malloc(0) */
static uint8_t *wc_malloc(long size)
{
    uint32_t u = (uint32_t)size;

    if (u == 0 || u > 0xFFFFFFD4u)
        return NULL;
    return malloc(u);
}

/* ---- Random numbers and the card helpers ---------------------------------------------------------------------- */

/* 0x78AE4: game_rand: seed [0xCAC5C] = seed * 0x15A4E35 + 1 (imul, 32 bits), returns (seed >> 16) & 0x7FFF */
int f78AE4(void)
{
    uint32_t seed = D32(GAME_SEED) * 0x15A4E35u;      /* 0x78AF1 imul eax,[0xCAC5C],0x15A4E35 */
    seed++;                                           /* 0x78AFB inc eax */
    setD32(GAME_SEED, seed);                          /* 0x78AFC */
    return (int)((seed >> 16) & 0x7FFFu);             /* 0x78B01 sar 0x10; and 0x7FFF (the sign bits are masked off) */
}

/* 0x76F70: shuffle cards[0..n): copy them to a 160-byte local, then n times draw index rand*n >> 15 (unsigned shift),
   append that card and close the gap. Arguments: eax n, edx cards. */
void f76F70(int n, uint8_t *cards)
{
    uint8_t local[0xA0];                              /* [ebp-0xB4]: 160 bytes (sub esp,0xB4 at 0x76F7D) */
    int32_t count = n;                                /* [ebp-0x14] */
    int32_t out = 0;                                  /* [ebp-4] (0x76F8B) */
    int32_t i;                                        /* [ebp-0xC] */
    int32_t r;                                        /* [ebp-8] */

    for (i = 0; i < count; i++)                       /* 0x76F99 */
        local[i] = cards[i];                          /* 0x76FA7..0x76FAC (n > 160 overflows, as in the original) */
    while (count != 0) {                              /* 0x76FB5 cmp [ebp-0x14],0; je */
        r = (int32_t)(((uint32_t)f78AE4() * (uint32_t)count) >> 15);   /* 0x76FBB call; imul; shr eax,0xF */
        cards[out++] = local[r];                      /* 0x76FCA..0x76FDF */
        for (i = r; count - 1 > i; i++)               /* 0x76FE4, 0x76FEF dec eax; cmp; jle */
            local[i] = local[i + 1];                  /* 0x76FFB [ebp+i-0xB3] -> 0x77005 [ebp+i-0xB4] */
        count--;                                      /* 0x77011 */
    }
}

/* 0x77184: insert src[0..n) in front of dst, whose first len bytes move up by n. Arguments: eax n, edx src, ebx len,
   ecx dst. */
void f77184(int n, const uint8_t *src, int len, uint8_t *dst)
{
    int32_t i;                                        /* [ebp-4] */

    for (i = len - 1; i >= 0; i--)                    /* 0x771A0 dec; 0x771AF cmp 0; jl */
        dst[i + n] = dst[i];                          /* 0x771B5..0x771C6 */
    for (i = 0; i < n; i++)                           /* 0x771CA, 0x771DA */
        dst[i] = src[i];                              /* 0x771DF..0x771ED */
}

/* ---- Trade-card stacks (0x6BA20) ------------------------------------------------------------------------------ */

/* 0x6BA20: clear the card fields of nation records 0..9, set the nine stacks' a/b counts, deal their commodity cards,
   shuffle each stack, then shuffle the calamities 0xC8DEC[s] into stacks 1..8 behind the first [0xF40F0] cards and put
   the calamities 0xC8DF8[s] on top of stacks 1..4. Layout: game.h. */
void f6BA20(void)
{
    uint8_t buf[0x14];                                /* [ebp-0x40]..[ebp-0x2D] */
    int32_t s;                                        /* [ebp-0x2C] */
    int32_t j;                                        /* [ebp-0x28] */
    int32_t n;                                        /* [ebp-0x20] */
    uint32_t rec;                                     /* [ebp-0x1C] */
    uint8_t card;                                     /* [ebp-0x18] */
    int32_t k;

    for (s = 0; s < 0xA; s++) {                       /* 0x6BA37, 0x6BA83 cmp 0xA */
        rec = GAME_NATION + (uint32_t)(s * GAME_NATION_SIZE);   /* 0x6BA89 imul 0xC7; + 0xF14CC */
        for (k = 1; k < 0x12; k++)                    /* 0x6BA9A k = 1; 0x6BA42 cmp 0x12 */
            setD8(rec + (uint32_t)k, 0);              /* 0x6BA4E */
        setD8(rec + 0x13, 0);                         /* 0x6BA56 */
        for (k = 0; k < 8; k++)                       /* 0x6BA5C, 0x6BA6D cmp 8 */
            setD8(rec + (uint32_t)k + 0x14, 0);       /* 0x6BA73 */
        /* 0x6BA79: the store after the loop, with k = 8: rec + 0x24 = 0xFF (G: the source meant +0x1C..+0x23 inside
           the loop; 0x45D30 sets those for records 1..13 and clears +0x24 again, so only record 0 keeps +0x24 = 0xFF.
           Kept as it is: the data object must match the original, and nothing shows that it matters, G) */
        setD8(rec + (uint32_t)k + 0x1C, 0xFF);
    }

    /* 0x6BAA3..0x6BB39: a and b of each stack (the stores in the original order) */
    setD16(0xF06D2u, 7);                              /* 0x6BABC stack 0 a */
    setD16(0xF06D4u, 7);                              /* 0x6BAC3 stack 0 b */
    setD16(0xF06EBu, 7);                              /* 0x6BACA stack 1 */
    setD16(0xF06EDu, 8);                              /* 0x6BAD1 */
    setD16(0xF0704u, 8);                              /* 0x6BAD8 stack 2 */
    setD16(0xF0706u, 9);                              /* 0x6BADF */
    setD16(0xF071Du, 7);                              /* 0x6BAE5 stack 3 */
    setD16(0xF071Fu, 8);                              /* 0x6BAEC */
    setD16(0xF0736u, 6);                              /* 0x6BAF3 stack 4 */
    setD16(0xF0738u, 7);                              /* 0x6BAFA */
    setD16(0xF074Fu, 5);                              /* 0x6BB01 stack 5 */
    setD16(0xF0751u, 6);                              /* 0x6BB08 */
    setD16(0xF0768u, 5);                              /* 0x6BB0F stack 6 */
    setD16(0xF076Au, 6);                              /* 0x6BB16 */
    setD16(0xF0783u, 5);                              /* 0x6BB1D stack 7 b */
    setD16(0xF079Cu, 5);                              /* 0x6BB24 stack 8 b */
    setD16(0xF0781u, 4);                              /* 0x6BB32 stack 7 a */
    setD16(0xF079Au, 4);                              /* 0x6BB39 stack 8 a */
    card = 1;                                         /* 0x6BB40 */

    for (s = 0; s < 9; s++) {                         /* 0x6BB45, 0x6BC1D cmp 9 */
        uint32_t st = GAME_STACK + (uint32_t)(s * GAME_STACK_SIZE);   /* lea 3s; *8; + s */
        n = 0;                                        /* 0x6BC25 */
        setD16(0xF07B4u + (uint32_t)(s * GAME_STACK_SIZE), 0);        /* 0x6BC36 */
        for (j = 0; S16(st + 2) > j; j++)             /* 0x6BC40; 0x6BB5F dword [st] sar 16 = word a; jle */
            setD8(st + GAME_STACK_CARDS + (uint32_t)n++, card);       /* 0x6BB7F..0x6BB8A */
        card++;                                       /* 0x6BB95 */
        for (j = 0; S16(st + 4) > j; j++)             /* 0x6BB98; 0x6BBB1 dword [st+2] sar 16 = word b */
            setD8(st + GAME_STACK_CARDS + (uint32_t)n++, card);       /* 0x6BBD1..0x6BBDC */
        setD16(st, (uint16_t)n);                      /* 0x6BBE7 */
        f76F70(n, DP(st + GAME_STACK_CARDS));         /* 0x6BC0C */
        card++;                                       /* 0x6BC14 */
    }

    for (s = 1; s < 9; s++) {                         /* 0x6BC48, 0x6BCF4 cmp 9 */
        uint32_t st = GAME_STACK + (uint32_t)(s * GAME_STACK_SIZE);
        n = 0;                                        /* 0x6BCFC */
        buf[n++] = D8(0xC8DECu + (uint32_t)s);        /* 0x6BD08..0x6BD0E: the stack's calamity */
        for (j = S32(UI_PLAYERS_B); S16(st) > j; j++) /* 0x6BD12 j = [0xF40F0]; 0x6BC66 dword [st-2] sar 16 = count */
            buf[n++] = D8(st + GAME_STACK_CARDS + (uint32_t)j);       /* 0x6BC8F..0x6BC95 */
        f76F70(n, buf);                               /* 0x6BCA1 */
        f77184(n, buf, S32(UI_PLAYERS_B), DP(st + GAME_STACK_CARDS)); /* 0x6BCCB: in front of the first [0xF40F0] */
        setD16(st, (uint16_t)(D16(UI_PLAYERS_B) + (uint32_t)n));      /* 0x6BCD0 bx = word [0xF40F0]; add ebx,n */
    }

    for (s = 1; s < 5; s++) {                         /* 0x6BD1F, 0x6BD2E cmp 5 */
        uint32_t st = GAME_STACK + (uint32_t)(s * GAME_STACK_SIZE);
        f77184(1, DP(0xC8DF8u + (uint32_t)s), S16(st), DP(st + GAME_STACK_CARDS));   /* 0x6BD70 */
        setD16(st, (uint16_t)(D16(st) + 1));          /* 0x6BD81 inc word */
    }
}

/* ---- Nation records (0x45D30) --------------------------------------------------------------------------------- */

/* 0x45D30: the token stock [0xF4898] by player count, then nation records 1..13 (199 bytes at 0xF14CC). Layout:
   game.h. */
void f45D30(void)
{
    int32_t n;                                        /* [ebp-0xC] */
    uint32_t rec;                                     /* [ebp-4] */
    int32_t j;                                        /* [ebp-8] */
    int32_t k;                                        /* [ebp-0x10] */

    if (S32(UI_PLAYERS_B) % 2 == 0)                   /* 0x45D45..0x45D58 idiv 2 (signed); test edx */
        setD32(GAME_TOKEN_STOCK, 0x37);               /* 0x45D5A */
    else
        setD32(GAME_TOKEN_STOCK, 0x2F);               /* 0x45D66 */
    if (S32(UI_PLAYERS_B) == 7)                       /* 0x45D70 */
        setD32(GAME_TOKEN_STOCK, 0x37);               /* 0x45D79 */
    if (S32(UI_PLAYERS_B) == 8)                       /* 0x45D83 */
        setD32(GAME_TOKEN_STOCK, 0x2F);               /* 0x45D8C */

    for (n = 1; n < 0xE; n++) {                       /* 0x45D96, 0x45F48 cmp 0xE */
        rec = GAME_NATION + (uint32_t)(n * GAME_NATION_SIZE);   /* 0x45F4E imul 0xC7 */
        if (n < 0xA)                                  /* 0x45F62 cmp 0xA; jl */
            setD8(rec + 0x9A, 0);                     /* 0x45DA5 */
        else
            setD8(rec + 0x9A, 3);                     /* 0x45F6E */
        for (j = 0; j <= 0x37; j++)                   /* 0x45DAE / 0x45F77; 0x45DB9 cmp 0x37; jg */
            setD8(rec + (uint32_t)j + 0x61, 0xFF);    /* 0x45DC5 */
        setD8(rec + 0x3D, (uint8_t)n);                /* 0x45DD1 */
        setD8(rec + 0x60, 0);                         /* 0x45DD7 */
        setD8(rec + 0x98, 0);                         /* 0x45DDE */
        setD8(rec + 0x99, D8(GAME_TOKEN_STOCK));      /* 0x45DED */
        setD8(rec + 0x3E, 0);                         /* 0x45DF6 */
        setD8(rec + 0x3F, 2);                         /* 0x45DFD */
        setD8(rec + 0x40, 0);                         /* 0x45E04 */
        for (k = 0; k < 4; k++) {                     /* 0x45E08, 0x45E15 cmp 4 */
            setD16(rec + (uint32_t)(2 * k) + 0x45, 0);   /* 0x45E23 */
            setD8(rec + (uint32_t)k + 0x4D, 0);       /* 0x45E2F */
        }
        setD8(rec + 0x51, 0);                         /* 0x45E38 */
        for (k = 0; k < 0xA; k++)                     /* 0x45E3C, 0x45E49 cmp 0xA */
            setD8(rec + (uint32_t)k + 0x52, 0);       /* 0x45E55 */
        for (k = 0; k < 0xD; k++)                     /* 0x45E5B, 0x45E68 cmp 0xD */
            setD8(rec + (uint32_t)k + 0x9C, 0);       /* 0x45E74 */
        setD8(rec + 0x5C, 0);                         /* 0x45E80 */
        setD16(rec + 0x5E, 0);                        /* 0x45E87 */
        setD8(rec + 0x9B, 0);                         /* 0x45E90 */
        for (k = 0; k < 9; k++) {                     /* 0x45E97, 0x45EA4 cmp 9 */
            setD8(rec + (uint32_t)k + 0xA9, 0);       /* 0x45EB0 */
            setD8(rec + (uint32_t)k + 0xB2, 0x14);    /* 0x45EBD */
        }
        for (k = 1; k <= 0x12; k++)                   /* 0x45EC6, 0x45ED5 cmp 0x12; jg */
            setD8(rec + (uint32_t)k, 0);              /* 0x45EE1 */
        for (k = 0; k <= 0x17; k++)                   /* 0x45EE6, 0x45EF3 cmp 0x17; jg */
            setD8(rec + (uint32_t)k + 0x24, 0);       /* 0x45EFF */
        for (j = 0; j < 8; j++) {                     /* 0x45F05, 0x45F12 cmp 8 */
            setD8(rec + (uint32_t)j + 0x14, 0);       /* 0x45F1E */
            setD8(rec + (uint32_t)j + 0x1C, 0xFF);    /* 0x45F28 */
        }
        setD8(rec + 0xBB, 1);                         /* 0x45F31 */
        setD8(rec + 0xBC, 1);                         /* 0x45F3B */
    }
}

/* ---- Music (0x7B854) and AIL 3.03's file helpers -------------------------------------------------------------- */

/* 0x7D2E8: AIL's file size: [0xCAC68] = 0; open the file (DOS open 0x93F92, mode 0x200: read, binary, G); if it does
   not open, [0xCAC68] = 3 and -1; else filelength (0x941E5; -1 -> [0xCAC68] = 5), close (0x941E0), the length.
   Relative to the current directory (the port runs in the game folder). cdecl, one stack argument. */
static long f7D2E8(const char *name)
{
    FILE *f;
    long size;

    setD32(0xCAC68u, 0);                              /* 0x7D2EA */
    f = fopen(name, "rb");                            /* 0x7D2FA */
    if (f == NULL) {                                  /* 0x7D304 cmp eax,-1 */
        setD32(0xCAC68u, 3);                          /* 0x7D309 */
        return -1;                                    /* 0x7D313 (eax = -1 from the open) */
    }
    size = -1;
    if (fseek(f, 0, SEEK_END) == 0)                   /* 0x7D314 filelength */
        size = ftell(f);
    if (size == -1)                                   /* 0x7D31B */
        setD32(0xCAC68u, 5);                          /* 0x7D320 */
    fclose(f);                                        /* 0x7D32C */
    return size;                                      /* 0x7D331 */
}

/* 0x7D334: AIL's file read: the whole file into dest, or into a new block from the allocation hook [0xCAC6C] (malloc)
   when dest is NULL; returns the block, or NULL with [0xCAC68] = 3 (no file), 2 (no memory) or 5 (short read). A
   block it allocated is freed (the hook [0xCAC70], free) on the error paths. cdecl, stack: name, dest. */
static uint8_t *f7D334(const char *name, uint8_t *dest)
{
    long size;                                        /* edi */
    uint8_t *block = dest;                            /* ecx */
    FILE *f;                                          /* esi */

    setD32(0xCAC68u, 0);                              /* 0x7D338 */
    size = f7D2E8(name);                              /* 0x7D343 */
    if (size == -1) {                                 /* 0x7D34D */
        setD32(0xCAC68u, 3);                          /* 0x7D352 */
        return NULL;                                  /* 0x7D35C */
    }
    if (block == NULL)                                /* 0x7D365 */
        block = wc_malloc(size);                      /* 0x7D369 call [0xCAC6C] (0x822B5) with eax = size */
    if (block == NULL) {                              /* 0x7D371 */
        setD32(0xCAC68u, 2);                          /* 0x7D375 */
        return NULL;
    }
    f = fopen(name, "rb");                            /* 0x7D38B open(name, 0x200) */
    if (f == NULL) {                                  /* 0x7D395 */
        free(block);                                  /* 0x7D39C call [0xCAC70] (a caller's dest too; none is passed here) */
        setD32(0xCAC68u, 3);                          /* 0x7D352 */
        return NULL;
    }
    if ((long)fread(block, 1, (size_t)size, f) != size) {   /* 0x7D3A8 read(fd, block, size); 0x7D3AD cmp eax,edi */
        free(block);                                  /* 0x7D3B3 */
        setD32(0xCAC68u, 5);                          /* 0x7D3B9 */
        fclose(f);   /* port: the original leaves the DOS handle open here (0x7D3C3 jmp 0x7D35C) */
        return NULL;
    }
    fclose(f);                                        /* 0x7D3C7 */
    return block;                                     /* 0x7D3CC */
}

/* 0x7CB9D: AIL's free of a locked block: if p, unlock p..p+size (0x7CD14, DPMI, no data written) and free it through
   the hook [0xCAC70]. cdecl, stack: p, size. The port's p is the handle in [0xF626C]. */
static void f7CB9D(uint32_t handle, long size)
{
    (void)size;                                       /* the unlock's range: nothing to unlock in the port */
    if (handle != 0) {                                /* 0x7CBA2 test esi,esi */
        free(heap_F626C_get(handle));                 /* 0x7CBB6 call [0xCAC70] */
        s_heap_F626C = NULL;   /* port: the table slot; the data field keeps its stale value, as the original's */
    }
}

/* 0x7CCFD: AIL's lock of p..p+size (0x7CC6F: DPMI 0600h, no data written). Nothing to lock in the port. */
static void f7CCFD(uint32_t handle, long size)
{
    (void)handle;
    (void)size;
}

/* 0x7B854: play music: the XMI file name (a data string) and the sequence number in it. Arguments: eax name, edx seq.
   The AIL calls go to snd.c (the port's sound, 22 Nov); what the original writes to its data is translated as before:
   the name 0xF61D8, the sequence 0xF6258, the block [0xF626C] and AIL's file error [0xCAC68]. The image given to
   AIL_init_sequence is the game's block, unless the sound engine plays a set of its own (snd_music_image: the GM
   engine plays the g file of the same song, read by snd_ail.c apart from the data object). */
void f7B854(const char *name, int seq)
{
    long size;                                        /* [ebp-0x14] */
    uint8_t *block;

    if (D8(0xF625Du) == 0)                            /* 0x7B86E: music off */
        return;
    if (strcmp(name, DSTR(0xF61D8u)) == 0 && S8(0xF6258u) == seq) {   /* 0x7B883; 0x7B88C dword [0xF6255] sar 24 */
        if (snd_sequence_status(D32(0xF6284u)) != 4)  /* 0x7B899..0x7B8B1: the same song again */
            snd_start_sequence(D32(0xF6284u));        /* 0x7B8C0: restarted unless it is playing */
        return;
    }
    snd_end_sequence(D32(0xF6284u));                  /* 0x7B8CC */
    if (strcmp(name, DSTR(0xF61D8u)) != 0) {          /* 0x7B8DC; 0x7B8E3 je 0x7B9B6 */
        size = f7D2E8(DSTR(0xF61D8u));                /* 0x7B8EE: the old name's file size (AIL's helper opens a
                                                         file of that name; it is in the catalog, so -1, G) */
        f7CB9D(D32(0xF626Cu), size);                  /* 0x7B8FE: free the old file's block */
        size = cat_size(name, S32(0xF40E4u));         /* 0x7B910 cat_size(name, [0xF40E4] = ACIV.CT5) */
        if (size != -2) {                             /* 0x7B91E: in the catalog */
            block = wc_malloc(size);                  /* 0x7B926 (size 0 or -1, a DOS error, makes the malloc fail) */
            setD32(0xF626Cu, heap_F626C_put(block));  /* 0x7B932 */
            s_heap_F626C_len = size > 0 ? (size_t)size : 0;
            if (block != NULL)   /* port guard: the original would hand cat_load a null buffer here */
                cat_load(name, block, S32(0xF40E4u)); /* 0x7B93C; the result is not checked */
        } else {                                      /* 0x7B946: not in the catalog: a file of that name */
            block = f7D334(name, NULL);               /* 0x7B94C */
            setD32(0xF626Cu, heap_F626C_put(block));  /* 0x7B954 */
        }
        if (D32(0xF626Cu) == 0) {                     /* 0x7B959 */
            /* 0x7B96B sprintf(0xF5500, name): the name itself is the format (XMI names hold no '%'), then quit */
            snprintf((char *)DP(0xF5500u), 0x1F4, "%s", name);
            f44CAC();                                 /* 0x7B973 */
        }
        strcpy((char *)DP(0xF61D8u), name);           /* 0x7B978..0x7B997 inline copy */
        size = f7D2E8(name);                          /* 0x7B99E */
        if (size > 0)                                 /* a loose file (in the catalog this is -1) */
            s_heap_F626C_len = (size_t)size;
        f7CCFD(D32(0xF626Cu), size);                  /* 0x7B9AE: lock the block */
    }
    {   /* 0x7B9B6..0x7B9EE: AIL_init_sequence([0xF6284], [0xF626C], seq); on a result <= 0, AIL's error text (its
           buffer 0x11AF7C) and the name go to 0xF5500 ("%s-%s\n", 0xB9745) and the game quits (f44CAC) */
        size_t len = 0;
        const uint8_t *img = snd_music_image(name, &len);
        if (img == NULL) {
            img = heap_F626C_get(D32(0xF626Cu));
            len = img != NULL ? s_heap_F626C_len : 0;
        }
        if (snd_init_sequence(D32(0xF6284u), img, len, seq) <= 0) {
            snprintf((char *)DP(0x11AF7Cu), 0x100, "%s", snd_last_error());   /* G: AIL's error buffer, 256 bytes */
            snprintf((char *)DP(0xF5500u), 0x1F4, DSTR(0xB9745u), DSTR(0x11AF7Cu), name);
            f44CAC();
        }
    }
    setD8(0xF6258u, (uint8_t)seq);                    /* 0x7B9F6 */
    snd_set_sequence_volume(D32(0xF6284u), (int)(D16(0xF097Eu) & 0x7F) * 127 / 9, 0);   /* 0x7B9FB..0x7BA27 */
    snd_start_sequence(D32(0xF6284u));                /* 0x7BA36 */
    snd_set_sequence_loop_count(D32(0xF6284u), 0);    /* 0x7BA47: forever */
    /* 0x7BA54: the debug log "Music File Loaded" (0x78B0C, a no-op) */
}

/* ---- main's helpers on the way into the game ------------------------------------------------------------------ */

/* 0x12DA4 (main 0x113FE): clear byte [0xF40FE], word [0xF5478], dword [0xEDF58], 20 bytes at 0xF6E76, 100 at 0xF6E12
   (meanings G) */
static void f12DA4(void)
{
    setD8(0xF40FEu, 0);                               /* 0x12DBC */
    setD16(0xF5478u, 0);                              /* 0x12DC2 */
    setD32(0xEDF58u, 0);                              /* 0x12DD0 */
    memset(DP(0xF6E76u), 0, 0x14);                    /* 0x12DD6 */
    memset(DP(0xF6E12u), 0, 0x64);                    /* 0x12DE7 */
}

/* 0x128F0 (main 0x116F0): the names of the map's western nation by the board (bit 0x02 of 0xE1624, G: the larger
   board): IBERIA / Iberian / IBER, else ITALY / Italian / ITAL, copied into the strings the pointers 0xC7694, 0xC76CC,
   0xC766C point to (data link addresses: 0xB646A, 0xB64D8, 0xB643A); with [0xF60F0] set also three letters at
   0xF607F..0xF6081 ("BER" / "TAL": letters 1..3 of nation 2's opening-map override name at 0xF6064 + 13 * 2, the
   dead developer switch of OVERVIEW §6 egg 14). Then per player count some territory bytes (in 0xE1638's records
   and beyond) lose their high nibble or a flag (meanings G). */
static void f128F0(void)
{
    if (D8(0xE1624u) & 0x02) {                        /* 0x12902 */
        if (D32(0xF60F0u) != 0) {                     /* 0x1290F */
            setD8(0xF607Fu, 0x42);                    /* 0x1291D 'B' */
            setD8(0xF6081u, 0x52);                    /* 0x12925 'R' */
            setD8(0xF6080u, 0x45);                    /* 0x1292B 'E' */
        }
        strcpy((char *)DP(D32(0xC7694u)), DSTR(0xB0E11u));   /* 0x12931.. "IBERIA" */
        strcpy((char *)DP(D32(0xC76CCu)), DSTR(0xB0E18u));   /* 0x12956.. "Iberian" */
        strcpy((char *)DP(D32(0xC766Cu)), DSTR(0xB0E20u));   /* 0x12980.. "IBER" */
        setD16(0xC0770u, 8);                          /* 0x129AA (dx = 8 from 0x12961) */
        setD16(0xC0784u, 0x9B);                       /* 0x129B1 (bx = 0x9B from 0x1298B) */
    } else {                                          /* 0x129BD */
        if (D32(0xF60F0u) != 0) {                     /* 0x129C3 */
            setD8(0xF6080u, 0x41);                    /* 0x129CB 'A' */
            setD8(0xF607Fu, 0x54);                    /* 0x129D3 'T' */
            setD8(0xF6081u, 0x4C);                    /* 0x129D9 'L' */
        }
        strcpy((char *)DP(D32(0xC7694u)), DSTR(0xB0E25u));   /* 0x129DF.. "ITALY" */
        strcpy((char *)DP(D32(0xC76CCu)), DSTR(0xB0E2Bu));   /* 0x12A04.. "Italian" */
        strcpy((char *)DP(D32(0xC766Cu)), DSTR(0xB0E33u));   /* 0x12A29.. "ITAL" */
        setD16(0xC0770u, 0xA2);                       /* 0x12A58 (cx = 0xA2 from 0x12A34) */
        setD16(0xC0784u, 0x36);                       /* 0x12A5F (si = 0x36 from 0x12A53) */
    }
    if (S32(UI_PLAYERS_B) == 3) {                     /* 0x12A66 */
        uint8_t ah;
        setD8(0xE2B69u, (uint8_t)(D8(0xE2B69u) & 0x0F));   /* 0x12A6F */
        ah = (uint8_t)(D8(0xE2033u) & 0x0F);          /* 0x12A76 */
        setD8(0xE2033u, ah);                          /* 0x12A81 */
        setD8(0xE2033u, (uint8_t)(ah | 0x10));        /* 0x12A8A */
    }
    if (S32(UI_PLAYERS_B) >= 4) {                     /* 0x12A90 cmp 4; jl (signed) */
        setD8(0xE1822u, (uint8_t)(D8(0xE1822u) & 0x0F));   /* 0x12AA8 */
        setD8(0xE18AEu, (uint8_t)(D8(0xE18AEu) & 0x0F));   /* 0x12ABA */
        setD8(0xE172Du, (uint8_t)(D8(0xE172Du) & 0x0F));   /* 0x12AC0 */
    }
    if (S32(UI_PLAYERS_B) == 6) {                     /* 0x12AC6 */
        setD8(0xE16E7u, (uint8_t)(D8(0xE16E7u) & 0x0F));   /* 0x12ACF */
        setD8(0xE26A0u, (uint8_t)(D8(0xE26A0u) & 0x0F));   /* 0x12AD6 */
    }
    if (S32(UI_PLAYERS_B) == 5 || S32(UI_PLAYERS_B) == 6)   /* 0x12ADD..0x12AEA */
        setD8(0xE17DCu, (uint8_t)(D8(0xE17DCu) & 0xFD));    /* 0x12AEC and -3 */
}

/* One dialog of 0x7784C: n button records at index base.., each the 20-byte template 0xCAC40 with x, w and the id
   i + idbase; enabled < 0: the template's enabled byte stays (the first dialog), else it is set. The bar is 634 pixels
   (0x27A) wide from x = 3: q = 634 / n each, the remainder r (0..4, G for r <= 4) spread as one more pixel on buttons
   n-1 (A), 0 (B), 2 (C), 3 (E), the x of the later buttons moved along. fix = {A, B, C, E}. */
static void f7784C_dialog(uint32_t dlg, int32_t base, int32_t n, int32_t q, const int32_t fix[4], int32_t idbase,
                          int enabled)
{
    int32_t i;                                        /* [ebp-0x20] */

    for (i = 0; i < n; i++) {                         /* 0x779DB cmp i,n; jge */
        uint32_t rec = dlg + DLG_REC(base + i);       /* lea [4*(base+i)] + (base+i); 4*eax + dlg+2 */
        int32_t x, w;
        memcpy(DP(rec), DP(0xCAC40u), BTN_SIZE);      /* 0x779F2..0x77A02: 5 movsd */
        x = i * q + 3 + (i > 0 ? fix[1] : 0);         /* 0x77A03 jle -> 0; 0x7791A imul; add 3 */
        x += (i > 2 ? fix[2] : 0);                    /* 0x77926 */
        x += (i > 3 ? fix[3] : 0);                    /* 0x77935 */
        setD16(rec + BTN_X, (uint16_t)x);             /* 0x77955 */
        w = q + (i == 0 ? fix[1] : 0);                /* 0x7795D..0x7796D */
        w += (i == 2 ? fix[2] : 0);                   /* 0x7796F */
        w += (i == 3 ? fix[3] : 0);                   /* 0x7797E */
        w += (n - 1 == i ? fix[0] : 0);               /* 0x7798D..0x7799D */
        setD16(rec + BTN_W, (uint16_t)w);             /* 0x779B0 */
        setD16(rec + BTN_ID, (uint16_t)(i + idbase)); /* 0x779CD */
        if (enabled >= 0)
            setD8(rec + BTN_ENABLED, (uint8_t)enabled);   /* e.g. 0x77B33 (0), 0x77F46 (1) */
    }
}

/* 0x7784C (main 0x11704): build the player-button rows of eight dialogs, one button per player (n = eax = [0xF40EC]),
   from the templates; their record indexes go to 0xF5940..0xF5968 (G: each dialog's first player button, and for
   three of them the count after). The edx (0x4F) and ebx (1) main sets before the call are for the f3BFBC after it
   (0x7784C saves and restores them). Dialog: template, first id, enabled byte (D):
     0xF596C  <- 0xBB414   9   template's      0x111594 <- 0xC95E4  9  0      0x111980 <- 0xC99D0  9  0
     0x111D6C <- 0xC9DD0   9   0               0x112158 <- 0xCA1BC  0x12  1   0x1106D8 <- 0xC8E00  9  0
     0x110ADC <- 0xC91EC   9   0               0xF634C  <- 0xBD554  9  0 */
void f7784C(int n)
{
    int32_t q, r;                                     /* [ebp-0x18], [ebp-0x14] */
    int32_t fix[4] = { 0, 0, 0, 0 };                  /* A [ebp-0x10], B [ebp-0xC], C [ebp-8], E [ebp-4] (0x77887) */
    int32_t base;                                     /* [ebp-0x1C] */

    q = 0x27A / n;                                    /* 0x77864..0x77871 idiv (signed) */
    r = 0x27A - q * n;                                /* 0x7787D */
    if (r == 1)                                       /* 0x77898 */
        fix[0] = r;                                   /* 0x7789D */
    else if (r == 2) {                                /* 0x778A5 */
        fix[0] = 1;                                   /* 0x778AF */
        fix[1] = 1;
    } else if (r == 3) {                              /* 0x778BA */
        fix[0] = 1;                                   /* 0x778C4 */
        fix[1] = 1;
        fix[2] = 1;
    } else if (r == 4) {                              /* 0x778D2 */
        fix[0] = 1;                                   /* 0x778DC */
        fix[1] = 1;
        fix[2] = 1;
        fix[3] = 1;
    }

    memcpy(DP(0xF596Cu), DP(0xBB414u), 1002);         /* 0x778E8..0x778F9 rep movsd 0xFA; movsw */
    base = S16(0xF596Cu);                             /* 0x778FB dword [0xF596A] sar 16 */
    setD32(0xF5940u, (uint32_t)base);                 /* 0x77909 */
    f7784C_dialog(0xF596Cu, base, n, q, fix, 9, -1);  /* 0x7790E..0x77A13 */
    setD16(0xF596Cu, (uint16_t)(D16(0xF596Cu) + (uint32_t)n));   /* 0x77A18..0x77A26 */

    memcpy(DP(0x111594u), DP(0xC95E4u), 1002);        /* 0x77A2D..0x77A41 */
    setD32(0xF5944u, (uint32_t)S16(0xF596Cu));        /* 0x77A32, 0x77A43: the first dialog's new count */
    base = S16(0x111594u);                            /* 0x77A48 */
    setD32(0xF5948u, (uint32_t)base);                 /* 0x77A56 */
    f7784C_dialog(0x111594u, base, n, q, fix, 9, 0);  /* 0x77A5B..0x77B78 */

    memcpy(DP(0x111980u), DP(0xC99D0u), 1002);        /* 0x77B7D..0x77B95 (bx = word [0x111594] read first, 0x77B8C) */
    setD16(0x111594u, (uint16_t)(D16(0x111594u) + (uint32_t)n));   /* 0x77B97, 0x77B9E */
    base = S16(0x111980u);                            /* 0x77B99 dword [0x11197E] sar 16 */
    setD32(0xF594Cu, (uint32_t)base);                 /* 0x77BAE */
    f7784C_dialog(0x111980u, base, n, q, fix, 9, 0);  /* 0x77BB3..0x77CD0 */

    setD16(0x111980u, (uint16_t)(D16(0x111980u) + (uint32_t)n));   /* 0x77CD5..0x77CE3 */
    memcpy(DP(0x111D6Cu), DP(0xC9DD0u), 1002);        /* 0x77CDE..0x77CFE */
    setD32(0xF5950u, (uint32_t)S16(0x111980u));       /* 0x77CEF, 0x77D00: the third dialog's new count */
    base = S16(0x111D6Cu);                            /* 0x77D05 */
    setD32(0xF5954u, (uint32_t)base);                 /* 0x77D13 */
    f7784C_dialog(0x111D6Cu, base, n, q, fix, 9, 0);  /* 0x77D18..0x77E35 */

    setD16(0x111D6Cu, (uint16_t)(D16(0x111D6Cu) + (uint32_t)n));   /* 0x77E3A..0x77E4D */
    memcpy(DP(0x112158u), DP(0xCA1BCu), 1002);        /* 0x77E48..0x77E5B */
    base = S16(0x112158u);                            /* 0x77E5D */
    setD32(0xF5958u, (uint32_t)base);                 /* 0x77E6B */
    f7784C_dialog(0x112158u, base, n, q, fix, 0x12, 1);   /* 0x77E70..0x77F8C */

    setD16(0x112158u, (uint16_t)(D16(0x112158u) + (uint32_t)n));   /* 0x77F91..0x77F9F */
    memcpy(DP(0x1106D8u), DP(0xC8E00u), 1002);        /* 0x77F9A..0x77FBA */
    setD32(0xF595Cu, (uint32_t)S16(0x112158u));       /* 0x77FAB, 0x77FBC: the fifth dialog's new count */
    base = S16(0x1106D8u);                            /* 0x77FC1 */
    setD32(0xF5960u, (uint32_t)base);                 /* 0x77FCF */
    f7784C_dialog(0x1106D8u, base, n, q, fix, 9, 0);  /* 0x77FD4..0x780F1 */

    memcpy(DP(0x110ADCu), DP(0xC91ECu), 1002);        /* 0x780F6..0x7810E (dx = word [0x1106D8] read first) */
    setD16(0x1106D8u, (uint16_t)(D16(0x1106D8u) + (uint32_t)n));   /* 0x78110, 0x78117 */
    base = S16(0x110ADCu);                            /* 0x78112 */
    setD32(0xF5964u, (uint32_t)base);                 /* 0x78127 */
    f7784C_dialog(0x110ADCu, base, n, q, fix, 9, 0);  /* 0x7812C..0x78249 */

    memcpy(DP(0xF634Cu), DP(0xBD554u), 1002);         /* 0x7824E..0x78266 (bx = word [0x110ADC] read first) */
    setD16(0x110ADCu, (uint16_t)(D16(0x110ADCu) + (uint32_t)n));   /* 0x78268, 0x7826F */
    base = S16(0xF634Cu);                             /* 0x7826A */
    setD32(0xF5968u, (uint32_t)base);                 /* 0x7827F */
    f7784C_dialog(0xF634Cu, base, n, q, fix, 9, 0);   /* 0x78284..0x783A1 */

    setD16(0xF634Cu, (uint16_t)(D16(0xF634Cu) + (uint32_t)n));     /* 0x783A6 add word [0xF634C],ax */
}

/* 0x7892C (main 0x11777): the mouse-visibility stack's top = 1 (0x112A0C), without calling fg_mousevis */
static void f7892C(void)
{
    setD8(0x112A0Cu, 1);                              /* 0x78939 */
}

/* 0x87E29 _dos_gettime(t): t+0 hour, +1 minute, +2 second, +3 hundredths (INT 21h AH=2Ch), from the local clock */
static void dos_gettime(uint32_t t)
{
    struct timespec ts;
    time_t now;
    struct tm *lt;
    long hs = 0;

    if (timespec_get(&ts, TIME_UTC) == TIME_UTC) {
        now = ts.tv_sec;
        hs = ts.tv_nsec / 10000000L;
    } else
        now = time(NULL);
    lt = localtime(&now);
    setD8(t + 0, (uint8_t)(lt != NULL ? lt->tm_hour : 0));   /* 0x87E33 [ebx] = ch */
    setD8(t + 1, (uint8_t)(lt != NULL ? lt->tm_min : 0));    /* [ebx+1] = cl */
    setD8(t + 2, (uint8_t)(lt != NULL ? lt->tm_sec : 0));    /* [ebx+2] = dh */
    setD8(t + 3, (uint8_t)hs);                               /* [ebx+3] = dl */
}

/* ---- Port entry points ---------------------------------------------------------------------------------------- */

/* main 0x113FE..0x11555 for a new game (after title_newgame returned 1; main.c goes on with game_continue_load and
   game_continue_rest, 0x115B0..0x1184A), in the original order, stopping before main's
   call of 0x2EFA0 at 0x1184F (the verification point: the caller dumps the data object there, ds_dump).
   Covered: 0x113FE (0x12DA4), 0x11403 (the PBEM test: the port always takes 0x114CF), 0x114CF..0x11555 (the "Loading
   game data and maps" screen), 0x115B0..0x1184A.
   Left out: 0x11410..0x114CA (PBEM start), 0x11557..0x115A9 (restore: not a new game), the debug log at 0x115C4, the
   setjmp(0xF58A0) at 0x115CE (the target of the in-game RESTORE's longjmp 0x3FF82 only; 0x22F36, 0x230B2 and 0x3FE9B jump
   to 0xF58D4, main 0x11036; main.c sets it between
   game_continue_load and game_continue_rest; the jmp_buf differs from DOSBox-X by design),
   0x1187F (the PBEM start 0x12C38), and the restore path 0x1188C when bit 0x04 is clear (returns -1).
   use_seed: the seed dword 0xCAC5C gets seed_override instead of the clock value; everything else as the original
   (the clock bytes 0xF422C..0xF422F are still written). Returns 0. */
int game_load(uint32_t seed_override, int use_seed)
{
    f12DA4();                                         /* 0x113FE */
    /* 0x11403 test byte [0xE1624],0x80: PBEM (0x11410..0x114CA) dropped; the menu clears 0x80 (0x4F7CC) */

    f39F90(2);                                        /* 0x114D4 */
    f78884();                                         /* 0x114D9: the pointer hidden */
    f3CD6C(8);                                        /* 0x114E3: background 8 into [0xF52CC] */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(0xF52CCu), S32(0xF52C8u));   /* 0x1150A -> the back buffer */
    fg_vbopen(S32(0xF52C8u));                         /* 0x11519 */
    fg_setcolor(5);                                   /* 0x11523 */
    f3A0B0(0, 0xE6, 0x280, DSTR(0xB0D22u));           /* 0x1153C "Loading game data and maps" */
    f39F90(1);                                        /* 0x11546 */
    fg_vbclose();                                     /* 0x1154B */
    f3CC14();                                         /* 0x11550: on the screen */

    (void)seed_override;
    (void)use_seed;
    return 0;                                         /* 0x11555 jmp 0x115B0: main goes on with game_continue_load,
                                                         its setjmp(0xF58A0) and game_continue_rest */
}

/* main 0x115B0..0x1184A, where the new-game path (0x11555) and the title menu's restore (0x115A9, title_restore) meet:
   the tables, maps and sprites, the minimap, the map variant, the map buffer, the button enables; for a new game
   (bit 0x04 of [0xE1624]) the seed, the card stacks, the area and nation records and the music. A restored game
   returns -1 at 0x1177C: main goes on at 0x1188C (0x14AA4, not ported). */
int game_continue(uint32_t seed_override, int use_seed)
{
    game_continue_load();
    return game_continue_rest(seed_override, use_seed);
}

/* main 0x115B0..0x115C4: the tables, maps and sprites (before main's setjmp(0xF58A0) at 0x115CE) */
void game_continue_load(void)
{
    f12394();                                         /* 0x115B0: terrflag.dat */
    f3D170();                                         /* 0x115B5: mapdataz.dat, borders1/2.dat */
    f11A8C();                                         /* 0x115BA: the game sprites */
    /* 0x115C4: debug log "Just loaded sprites" (no-op) */
}

/* main 0x115D3..0x1184A: after the setjmp(0xF58A0) at 0x115CE, which the in-game RESTORE jumps back to */
int game_continue_rest(uint32_t seed_override, int use_seed)
{
    uint32_t seed;
    uint8_t bl, ch;

    fg_vbopen(S32(0xF52C8u));                         /* 0x115DA */
    fg_move(0, 0);                                    /* 0x115E6 */
    cat_show(DSTR(0xB0D51u), 3, S32(0xF40D8u));       /* 0x115FB "minimap.gif", CT2 */
    f3AE90(GAME_SPR_MINIMAP,                          /* 0x11628 */
           S16(0xBC7E0u + 2u * D32(UI_PLAYERS_B)),    /* 0x11616 dword [2*p + 0xBC7DE] sar 16 */
           S16(0xBC7ECu + 2u * D32(UI_PLAYERS_B)),    /* 0x1160F dword [2*p + 0xBC7EA] sar 16 */
           0x9A, 0x33);                               /* 0x11608 ecx, 0x1160D push */
    fg_vbclose();                                     /* 0x1162D */

    setD32(UI_PLAYERS_A, (uint32_t)(S8(UI_SETUP_HUMANS) + S8(UI_SETUP_COMPUTERS)));   /* 0x11632..0x11645 */

    /* 0x1164A..0x11670: the map variant = 7 * (bit 0x02 of word [0xE1624]) + [0xF40F0] - 2 (shl eax,0xE; shr ax,0xF) */
    setD16(GAME_MAP_VARIANT, (uint16_t)(7u * ((D16(UI_SETUP_FLAGS) >> 1) & 1u) + D32(UI_PLAYERS_B) - 2u));
    {
        int32_t v = S16(GAME_MAP_VARIANT);            /* 0x11676 dword [0xF6112] sar 16 */
        uint16_t dx = D16(0xBC7AEu + (uint32_t)(2 * v));   /* 0x11684 */
        uint16_t ax = D16(0xBC7CAu + (uint32_t)(2 * v));   /* 0x1168C */
        setD16(0xF6110u, dx);                         /* 0x11694 */
        setD16(0xF6112u, ax);                         /* 0x1169B */
    }

    bl = D8(UI_SETUP_FLAGS);                          /* 0x1167E */
    if ((bl & 0x04) == 0 || (bl & 0x80) == 0) {       /* 0x116A1 je 0x116AB; 0x116A6 jne 0x116F0 (PBEM new game) */
        fg_vbopen(S32(0xF52C4u));                     /* 0x116B2: the map buffer */
        fg_move(0, 0);                                /* 0x116BE */
        cat_show(DSTR(0xB0D5Du), 3, S32(0xF40D8u));   /* 0x116D4 "map&pal.gif", CT2 */
        f12B00();                                     /* 0x116DC: territories in play */
        f3EED8();                                     /* 0x116E1: adjacency, N.RL */
        f44974();                                     /* 0x116E6: grey out the rest on the map */
        fg_vbclose();                                 /* 0x116EB */
    }

    f128F0();                                         /* 0x116F0 */
    f7784C(S32(UI_PLAYERS_A));                        /* 0x11704 */
    f3BFBC(0xC1A5Cu, 0x4F, 1);                        /* 0x1170E (edx, ebx from 0x116F5/0x116FF) */
    f3BFBC(0xC1A5Cu, 0x50, 1);                        /* 0x11722 */
    f3BFBC(0xF596Cu, 7, 1);                           /* 0x11736 */
    f3BFBC(0xF596Cu, 6, 1);                           /* 0x1174A */
    f3BFBC(0xF596Cu, 3, 1);                           /* 0x1175E */
    f3BFBC(0xF596Cu, 4, 1);                           /* 0x11772 */
    f7892C();                                         /* 0x11777 */

    if ((D8(UI_SETUP_FLAGS) & 0x04) == 0)             /* 0x1177C je 0x1188C: a restored game */
        return -1;

    dos_gettime(0xF422Cu);                            /* 0x1178E _dos_gettime(0xF422C) */
    seed = (uint32_t)D8(0xF422Fu) << 24               /* 0x11795..0x117A3: hundredths */
         | (uint32_t)D8(0xF422Cu) << 16               /* 0x1179E, 0x117A6: hour */
         | (uint32_t)D8(0xF422Eu) << 8                /* 0x117AD, 0x117B8: second */
         | (uint32_t)D8(0xF422Du);                    /* 0x117C6, 0x117D2: minute */
    ch = 0;                                           /* 0x117C4 xor ch,ch */
    setD16(0xF40FCu, 1);                              /* 0x117CB (di = 1) */
    setD8(0x110F74u, ch);                             /* 0x117D4 */
    setD32(GAME_SEED, use_seed ? seed_override : seed);   /* 0x117DA (port: --seed replaces the clock value) */

    f6BA20();                                         /* 0x117E0: the trade-card stacks */
    f12B5C();                                         /* 0x117E5: the area records */
    f12BB8();                                         /* 0x117EA */
    f45D30();                                         /* 0x117EF: the nations */
    memset(DP(0xEE9F0u), 0, 0x384);                   /* 0x117FB (ebx = 0x384 from 0x117BB) */
    setD8(0xF408Cu, ch);                              /* 0x1180A */
    setD32(0xF4230u, 0);                              /* 0x11814 */
    memset(DP(0xF0684u), 0, 0x14);                    /* 0x1181A */

    if ((D8(UI_SETUP_FLAGS) & 0x80) == 0) {           /* 0x1181F; 0x11828 jne 0x1187F: the PBEM start 0x12C38, dropped */
        if (D8(0xF097Cu) & 0x02) {                    /* 0x1182A: music on in the settings (aciv.cfg) */
            setD8(0xF4102u, 0x15);                    /* 0x11835 */
            f7B854(DSTR(D32(0xCAC35u)), S8(0xCAC3Du));    /* 0x1184A: record 21 of the music table (gmap__1c.xmi) */
        }
    }
    return 0;                                         /* 0x1184F: main calls 0x2EFA0 next (not ported) */
}

/* port: write the data object (DS_SIZE bytes from link address DS_BASE) to path, as DOSBox-X's
   MEMDUMPBIN 0188:2A9000 72C00 does for the original; 0 ok, -1 on error */
int ds_dump(const char *path)
{
    FILE *f = fopen(path, "wb");
    int rc = 0;

    if (f == NULL)
        return -1;
    if (fwrite(g_ds, 1, DS_SIZE, f) != DS_SIZE)
        rc = -1;
    if (fclose(f) != 0)
        rc = -1;
    return rc;
}
