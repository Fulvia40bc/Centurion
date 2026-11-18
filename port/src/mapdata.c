/* Module MAP1 of docs/MAP_PLAN.md, the data half: the map scroll in pixels (0x449B8, 0x449E0), the home view of a
   nation (0x3D9FC), the view centred on a territory (0x3DA60), the highlight setters (0x3F624, 0x3F670, 0x3F698,
   0x3F70C), the territory alias ids (0x3E89C, 0x3E8FC), the map hit test (0x3D2A4) and the release of the map heap
   blocks (0x3F558). Literal translations; the address of each instruction a line comes from is in its comment. D = read
   in the code, G = inference. Prototypes and the data: mapdata.h; the drawing half (f3D52C, f787F8): map.h.

   Watcom runtime -> standard C: 0x7D5D0 memset(eax, edx, ebx), 0x877E6 free (load_heap_free for the heap fields).
   __CHK dropped.

   Heap fields (port, G): the original keeps malloc pointers in 32-bit data fields. Every field 0x3F558 frees was
   allocated through load.c's load_heap_alloc (0xFF648, 0xFF8CC, 0xFF8D0 in load.c's f3EA7C / f3EC20; 0xFFB54, 0xFFB58,
   0xFFB5C, 0x10740C in gfxload.c's f3EED8 through its heap_alloc), so each holds a handle with offset 0 and
   load_heap_free releases it; none of them is a sprite record (ui_sprite_*). The record fields [0xFF64C + 4t] are
   handles into the mapdataz block (block + 2k), which f3D2A4 reads with load_heap_ptr. */

#include <stdio.h>
#include <string.h>

#include "state.h"
#include "game.h"
#include "map.h"
#include "mapdata.h"

#define MAP_SCROLL_COL   0xF4884u   /* int32 */
#define MAP_SCROLL_ROW   0xF4888u   /* int32 */
#define MAP_HOME_COL     0xC19E4u   /* int32[10] */
#define MAP_HOME_ROW     0xC1A0Cu   /* int32[10] */
#define MAP_CANON        0xC803Cu   /* uint8[160] */
#define MAP_BORDER_COL   0xFFB60u   /* uint8[160] */
#define MAP_TOKEN_COL    0xFFC00u   /* uint8[160] */
#define MAP_SHIP_COL     0xFFCA0u   /* uint8[160] */
#define MAP_SEA_MODE     0x108804u  /* int8: 1 land, 2 sea */
#define MAP_PHASE        0xF4102u   /* uint8 */
#define MAP_ALIAS_ON     0x109E14u  /* int16 */
#define MAP_FLAGS        0xE1624u   /* uint8 (the low byte of the flags word) */

/* ---- 0x449B8, 0x449E0: the scroll in pixels ------------------------------------------------------------------- */

int32_t f449B8(void)
{
    int32_t v = S32(MAP_SCROLL_COL);                              /* 0x449C6 mov edx,[0xF4884] */

    return (int32_t)((uint32_t)v * 78u);                          /* 0x449CC..0x449DA: (5v * 8 - v) * 2 */
}

int32_t f449E0(void)
{
    int32_t v = S32(MAP_SCROLL_ROW);                              /* 0x449F0 mov eax,[0xF4888] */
    int32_t q = v / 2;                                            /* 0x449F5..0x449FC: sub (v sar 31); sar 1 (truncated) */
    int32_t r = v % 2;                                            /* 0x44A0C..0x44A1B: idiv 2, remainder in edx */

    return (int32_t)((uint32_t)q * 71u + (uint32_t)r * 35u);      /* 0x449FE ecx = (9q * 8) - q; 0x44A1D (9r * 4) - r; add */
}

/* ---- 0x3D9FC: the home view of nation n ----------------------------------------------------------------------- */

void f3D9FC(int n)
{
    if ((D8(MAP_FLAGS) & 0x02) != 0 && n == 2) {                  /* 0x3DA10 test byte [0xE1624],2; 0x3DA19 cmp n,2 */
        setD32(MAP_SCROLL_COL, D32(MAP_HOME_COL));                /* 0x3DA1F [0xC19E4] -> [0xF4884] */
        setD32(MAP_SCROLL_ROW, D32(MAP_HOME_ROW));                /* 0x3DA29 [0xC1A0C]; 0x3DA49 -> [0xF4888] */
    } else {
        setD32(MAP_SCROLL_COL, D32(MAP_HOME_COL + 4u * (uint32_t)n));   /* 0x3DA33 [4n + 0xC19E4] */
        setD32(MAP_SCROLL_ROW, D32(MAP_HOME_ROW + 4u * (uint32_t)n));   /* 0x3DA42 [4n + 0xC1A0C]; 0x3DA49 */
    }
    f3D52C(0, 0);                                                 /* 0x3DA4E..0x3DA52 */
}

/* ---- 0x3DA60: centre the map on territory t ------------------------------------------------------------------- */

void f3DA60(int t)
{
    uint32_t c = D8(MAP_CANON + (uint32_t)t);                     /* 0x3DA79 movzx: byte [t + 0xC803C] */

    /* 0x3DA82..0x3DA99: dword [35c + 0xE1648] sar 24 = int8 terr[c] + 0x13 */
    setD32(MAP_SCROLL_COL, (uint32_t)(int32_t)S8(GAME_TERR + GAME_TERR_SIZE * c + 0x13u));   /* 0x3DA9C */
    /* 0x3DAA1..0x3DAB8: dword [35c + 0xE1649] sar 24 = int8 terr[c] + 0x14 */
    setD32(MAP_SCROLL_ROW, (uint32_t)(int32_t)S8(GAME_TERR + GAME_TERR_SIZE * c + 0x14u));   /* 0x3DABB */
    f787F8(t);                                                    /* 0x3DAC3: t's name, not c's */
    f3D52C(0, 0);                                                 /* 0x3DACC */
}

/* ---- 0x3F624, 0x3F670, 0x3F698, 0x3F70C: the highlight setters ------------------------------------------------- */

void f3F624(int t, int c)
{
    setD8(MAP_BORDER_COL + (uint32_t)t, (uint8_t)c);              /* 0x3F640 mov [edx + 0xFFB60], al */
}

void f3F670(void)
{
    memset(DP(MAP_BORDER_COL), 0, 0xA0);                          /* 0x3F68B memset(0xFFB60, 0, 0xA0) */
}

void f3F698(int t, int c)
{
    setD8(MAP_TOKEN_COL + (uint32_t)t, (uint8_t)c);               /* 0x3F6B4 mov [edx + 0xFFC00], al */
}

void f3F70C(int t, int c)
{
    setD8(MAP_SHIP_COL + (uint32_t)t, (uint8_t)c);                /* 0x3F728 mov [edx + 0xFFCA0], al */
}

/* ---- 0x3E89C, 0x3E8FC: the alias ids -------------------------------------------------------------------------- */

int f3E89C(int t)
{
    int32_t r = t;                                                /* 0x3E8B4 [ebp-4] = t */

    if (t == 0x39)                                                /* 0x3E8B7 */
        r = 0x9D;                                                 /* 0x3E8BD */
    else if (t == 0x0D)                                           /* 0x3E8C9 */
        r = 0x9E;                                                 /* 0x3E8CE */
    else if (t == 0x87)                                           /* 0x3E8E1 */
        r = 0x9F;                                                 /* 0x3E8E9 */
    return r;                                                     /* 0x3E8F0 */
}

int f3E8FC(int t)
{
    int32_t r = t;                                                /* 0x3E915 [ebp-8] = t */
    uint32_t c = D8(MAP_CANON + (uint32_t)t);                     /* 0x3E91D movzx: byte [t + 0xC803C] */

    if (c == 0x39) {                                              /* 0x3E926 */
        if (S16(MAP_ALIAS_ON) == 1)                               /* 0x3E92C dword [0x109E12] sar 16; cmp 1 */
            r = 0x9D;                                             /* 0x3E939 */
    } else if (c == 0x0D) {                                       /* 0x3E945 */
        if (S16(MAP_ALIAS_ON) == 1)                               /* 0x3E94A */
            r = 0x9E;                                             /* 0x3E957 */
    } else if (c == 0x87) {                                       /* 0x3E963 */
        if (S16(MAP_ALIAS_ON) == 1)                               /* 0x3E96B */
            r = 0x9F;                                             /* 0x3E978 */
    }
    return r;                                                     /* 0x3E97F */
}

/* ---- 0x3D2A4: the map hit test -------------------------------------------------------------------------------- */

/* word [h] of the mapdataz block through a handle. Port guard (G): with the block missing or freed the original reads
   whatever memory the pointer names; the port quits with a message instead (as gfxload.c's blkp). */
static uint16_t map_word(uint32_t h)
{
    const uint8_t *p = load_heap_ptr(h, 2);

    if (p == NULL) {
        snprintf((char *)DP(0xF5500u), 0x1F4, "mapdataz word 0x%08X outside its block", (unsigned)h);
        f44CAC();
    }
    return (uint16_t)(p[0] | p[1] << 8);
}

int f3D2A4(int x, int y)
{
    int32_t mx;          /* [ebp-0x34] */
    int32_t my;          /* [ebp-0x30] */
    int32_t t;           /* [ebp-0x3C] */
    uint32_t rec;        /* [ebp-0x24]: the record's handle (a pointer in the original) */
    uint16_t n;          /* [ebp-0x1C], low word */
    uint16_t ytop;       /* [ebp-0x18], low word */
    uint16_t ybot;       /* [ebp-0x14], low word */
    int32_t idx;         /* [ebp-0x2C]: word index inside the record */
    int32_t row;         /* [ebp-0x38] */
    int32_t i;           /* [ebp-0x28] */
    uint16_t x1;         /* [ebp-0x10], low word */
    uint16_t x2;         /* [ebp-0x20], low word */

    mx = (int32_t)((uint32_t)f449B8() + (uint32_t)x - 8u);        /* 0x3D2BD..0x3D2C8 */
    my = (int32_t)((uint32_t)f449E0() + (uint32_t)y - 6u);        /* 0x3D2CB..0x3D2D6 */
    for (t = 0; t < 0xA0; t++) {                                  /* 0x3D2D9; 0x3D2E6 cmp 0xA0; jge 0x3D427 */
        if (D8(GAME_INPLAY + (uint32_t)t) == 0)                   /* 0x3D2F6 byte [t + 0xEE16C]; test; je */
            continue;
        rec = D32(GAME_MAPDATA_PTR + 4u * (uint32_t)t);           /* 0x3D303 [4t + 0xFF64C] */
        n = map_word(rec);                                        /* 0x3D310 */
        ytop = map_word(rec + 2u);                                /* 0x3D319 */
        ybot = map_word(rec + 4u);                                /* 0x3D323 */
        idx = 3;                                                  /* 0x3D32A */
        if ((int16_t)ytop > my)                                   /* 0x3D331 movsx; cmp; jg (signed) */
            continue;
        if ((int16_t)ybot < my)                                   /* 0x3D33C movsx; cmp; jl */
            continue;
        row = (int16_t)ytop - 1;                                  /* 0x3D345 movsx; dec */
        for (i = 0; (int16_t)n > i; i++) {                        /* 0x3D34D; 0x3D35A movsx n; cmp; jle -> next t */
            x1 = map_word(rec + 2u * (uint32_t)idx++);            /* 0x3D367..0x3D375 */
            x2 = map_word(rec + 2u * (uint32_t)idx++);            /* 0x3D378..0x3D386 */
            if ((int16_t)x1 >= 0)                                 /* 0x3D389 movsx; test; jl */
                row++;                                            /* 0x3D394: a new row */
            else
                x1 = (uint16_t)(0u - x1);                         /* 0x3D399 neg dword (its low word): the same row */
            if (my != row)                                        /* 0x3D39C cmp; jne */
                continue;
            if ((int16_t)x1 > mx)                                 /* 0x3D3A4 movsx; cmp; jg */
                continue;
            if ((int16_t)x2 < mx)                                 /* 0x3D3AD movsx; cmp; jl */
                continue;
            if (S8(MAP_SEA_MODE) == 2 || D8(MAP_PHASE) == 4)      /* 0x3D3B6 dword [0x108801] sar 24; 0x3D3C3 cmp byte */
                return t;                                         /* 0x3D3CC */
            if (t == 0x9D)                                        /* 0x3D3D4 */
                return 0x39;                                      /* 0x3D3DC */
            if (t == 0x9E)                                        /* 0x3D3F1 */
                return 0x0D;                                      /* 0x3D3F8 */
            if (t == 0x9F)                                        /* 0x3D40D */
                t = 0x87;                                         /* 0x3D415 */
            return t;                                             /* 0x3D41C */
        }
    }
    return -100;                                                  /* 0x3D427 mov eax,0xFFFFFF9C */
}

/* ---- 0x3F558: free the map heap blocks ------------------------------------------------------------------------- */

void f3F558(void)
{
    uint32_t h;

    h = D32(GAME_LANDADJ_BLOCK);                                  /* 0x3F56A [0xFFB54] */
    if (h != 0) {
        load_heap_free(h);                                        /* 0x3F578 free */
        setD32(GAME_LANDADJ_BLOCK, 0);                            /* 0x3F57D */
    }
    h = D32(GAME_COASTADJ_BLOCK);                                 /* 0x3F583 [0xFFB58] */
    if (h != 0) {
        load_heap_free(h);                                        /* 0x3F591 */
        setD32(GAME_COASTADJ_BLOCK, 0);                           /* 0x3F596 */
    }
    h = D32(GAME_SEAADJ_BLOCK);                                   /* 0x3F59C [0xFFB5C] */
    if (h != 0) {
        load_heap_free(h);                                        /* 0x3F5A8 */
        setD32(GAME_SEAADJ_BLOCK, 0);                             /* 0x3F5AF */
    }
    h = D32(GAME_RL_BLOCK);                                       /* 0x3F5B4 [0x10740C] */
    if (h != 0) {
        load_heap_free(h);                                        /* 0x3F5C2 */
        if (g_original)
            /* BUG(1995): the store after this free goes to [0xFFB5C] (0x3F5C7 mov [0xFFB5C],ebx), already 0 after the
               seaadj free above, so [0x10740C] keeps the freed block's address (a dangling pointer, D). Harmless in the
               original as long as the next new game or restore reloads N.RL before another free (G: both paths run
               0x3EED8). In the port the field keeps a stale handle whose slot load_heap_alloc may hand out again. */
            setD32(GAME_SEAADJ_BLOCK, 0);
        else
            setD32(GAME_RL_BLOCK, 0);
    }
    h = D32(GAME_MAPDATA_BLOCK);                                  /* 0x3F5CD [0xFF648] */
    if (h != 0) {
        load_heap_free(h);                                        /* 0x3F5DB */
        setD32(GAME_MAPDATA_BLOCK, 0);                            /* 0x3F5E0 */
    }
    h = D32(GAME_BORDERS1_BLOCK);                                 /* 0x3F5E6 [0xFF8CC] */
    if (h != 0) {
        load_heap_free(h);                                        /* 0x3F5F2 */
        setD32(GAME_BORDERS1_BLOCK, 0);                           /* 0x3F5F9 */
    }
    h = D32(GAME_BORDERS2_BLOCK);                                 /* 0x3F5FE [0xFF8D0] */
    if (h != 0) {
        load_heap_free(h);                                        /* 0x3F60C */
        setD32(GAME_BORDERS2_BLOCK, 0);                           /* 0x3F611 */
    }
}
