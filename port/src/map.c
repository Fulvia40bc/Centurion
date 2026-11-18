/* The map window of AC_MAIN.EXE, drawing half of module MAP1 (docs/MAP_PLAN.md): 0x3D52C (scroll by dx, dy and redraw
   the map window and the mini-map frame), 0x3D18C (territory border), 0x3DAD8 (land tokens), 0x3DE40 (ships),
   0x3E98C / 0x3E9DC / 0x3EA2C (number sprites), 0x3CBBC (lower panel, back buffer -> screen), 0x787F8 (territory name
   label) and 0x7876C (phase name label). Literal translations; the address of each instruction a line comes from is in
   its comment. D = read in the code, G = inference. Prototypes: map.h; the scroll helpers 0x449B8 / 0x449E0 are in the
   other half (mapdata.h).

   Watcom runtime -> standard C: 0x7DB3D sprintf -> snprintf bounded to [0xF5500]'s 500 bytes (as load.c). __CHK
   dropped.

   Data layouts (game.h; offsets in the records, D at the addresses given):
     territory t at 0xE1638 + 35t: +0x01 byte (bits 3..5: the marker index, 0x3D7B9 shl al,2; shr al,5),
       +0x03/+0x05 point 0 (tokens), +0x07/+0x09 point 1 (ships), +0x0B/+0x0D point 2 (city), +0x0F/+0x11 point 3
       (marker; x3 >= 5000 = hidden). The code reads each int16 as a dword two bytes lower, sar 16.
     area record t + 1 at 0xE31B8 + 69(t+1): +0x01 int8 (G: nations present), +0x01+k uint8 k 1..9 (G: nation k
       present), +0x0E+n int8 tokens of nation n (n 1..13), +0x1B+n int8 ships of nation n (n 1..9), +0x24+n int8
       (phase 5: G, tokens moved), +0x2D+n uint8 (phase 5: G, the selected ship of nation n). Each int8 is read as a
       dword three bytes lower, sar 24.
     0xE2C18 + 3t: +0 uint8 (G: the city owner), +2 bit 0 (G: a city stands there).
     nation n at 0xF14CC + 199n: +0x27 byte (G: active), +0x41+s uint8 and +0x4D+s int8 (G: per-ship values).
     [0xF40F8] int32 the current player P (1-based, G: the nation), byte [0xF4102] the phase, int8 [0xC1A34] the map
     detail level, int8 [0x108804] land (1) / sea (2) mode.
   Sprite records (8 bytes, ui.h; +0 a ui_sprite_put handle in the port): 0xF489C + 8n tokens, 0xF490C + 8n ships,
   0xF497C + 8n cities, 0xF4D2C + 8k markers, 0xF4E3C + 8n small tokens, 0xF4EAC / 0xF4F9C / 0xF508C + 8n the three
   number sets (30 records each: 0..9 w 9, 10..19 w 11, 20..29 w 13; the third set 11/13/15), 0xF517C / 0xF5184 the
   ship markers, 0xF520C the mini-map.

   Heap pointers (port, G): the border records [0xFF8D4 + 4t] are load.c handles (game.h), read through load_heap_ptr.

   1995 bugs: none found in these ten routines (no g_original branches). Oddities, kept (D):
   - 0x3DAD8 / 0x3DE40 draw the box around the current player's stack in colour 0xFFC00[t] / 0xFFCA0[t] even when that
     colour is 0 (only the 2-pixel depth depends on it): a colour-0 frame at depth 1 (or whatever fg_boxdepth was left
     at) around every stack of the current player.
   - 0x3DE40 prints the phase-5 ship value with one more shadow copy, at (x+9, y+5), only when the current player is
     nation 2 (0x3E24A cmp dword [ebp-0x2c],2).
   - 0x3DE40 tests the uint8 +0x41+s for < 0 (0x3E2FE test ebx,ebx; jl), which cannot happen.
   - Tokens are drawn for t < 157 only (0x3DE12 cmp 0x9D), ships for t < 160 (0x3E348 cmp 0xA0): the alias
     territories 157..159 can hold ships (0x3D2A4 keeps the alias in sea mode). */

#include <stdio.h>
#include "game.h"
#include "fg.h"
#include "map.h"
#include "mapdata.h"

/* territory record field (int16 at 0xE1638 + 35t + off) */
static int32_t terr16(int32_t t, uint32_t off)
{
    return (int32_t)S16(GAME_TERR + 35u * (uint32_t)t + off);
}

/* area record t + 1, int8 at +off */
static int32_t area8(int32_t t, uint32_t off)
{
    return (int32_t)S8(GAME_AREA + 69u * (uint32_t)(t + 1) + off);
}

/* ---- 0x3D18C: territory border ---------------------------------------------------------------------------------- */

/* The border record [0xFF8D4 + 4t] (borders1/2.dat, f3EC20): int16 n, int16 y, then n int16 values. A value >= 0
   moves to the next row (y + 1) and plots x; a negative one plots -x on the same row. Each point is moved by the map
   scroll and plotted only inside the map window (8..0x277, 6..0x144). */
void f3D18C(int t, int color)
{
    int32_t sx, sy;                                             /* [ebp-0x28], [ebp-0x24] */
    uint32_t rec;                                               /* [ebp-0x18], a handle (port) */
    uint16_t n, y, v;                                           /* the low words of [ebp-0x10], [ebp-0xc], [ebp-0x14] */
    int32_t k, i;                                               /* [ebp-0x20], [ebp-0x1c] */
    const uint8_t *p;

    if (t == 0x9D)                                              /* 0x3D1A4 */
        t = 0x39;
    if (t == 0x9E)                                              /* 0x3D1B4 */
        t = 0x0D;
    if (t == 0x9F)                                              /* 0x3D1C4 */
        t = 0x87;
    sx = f449B8();                                              /* 0x3D1D4 */
    sy = f449E0();                                              /* 0x3D1DC */
    fg_setcolor(color);                                         /* 0x3D1E8 */
    rec = D32(GAME_BORDER_PTR + 4u * (uint32_t)t);              /* 0x3D1F3 [4*t + 0xFF8D4] */
    /* Port guard (G): a record whose block was never loaded draws nothing (the original reads through its
       pointer). */
    if ((p = load_heap_ptr(rec, 4)) == NULL)
        return;
    n = (uint16_t)(p[0] | p[1] << 8);                       /* 0x3D200 mov ax,[eax] */
    y = (uint16_t)(p[2] | p[3] << 8);                           /* 0x3D209 mov ax,[eax+2] */
    k = 2;                                                      /* 0x3D210 */
    for (i = 0; (int16_t)n > i; i++) {                          /* 0x3D219, 0x3D273 movsx word [ebp-0x10]; jle */
        p = load_heap_ptr(rec + 2u * (uint32_t)k, 2);           /* 0x3D27C..0x3D284: k++ * 2 + rec */
        if (p == NULL)                                          /* port guard (G): data past the block */
            return;
        k++;
        v = (uint16_t)(p[0] | p[1] << 8);                       /* 0x3D287 mov ax,[eax] */
        if ((int16_t)v < 0)                                     /* 0x3D28D movsx; test; jl */
            v = (uint16_t)(0u - v);                             /* 0x3D21E neg dword [ebp-0x14] */
        else
            y++;                                                /* 0x3D298 inc dword [ebp-0xc] */
        if ((int16_t)v - sx >= 8 && (int16_t)v - sx <= 0x277    /* 0x3D221..0x3D239 */
            && (int16_t)y - sy >= 6 && (int16_t)y - sy <= 0x144)   /* 0x3D23B..0x3D253 */
            fg_point((int16_t)v - sx, (int16_t)y - sy);         /* 0x3D265 */
    }
}

/* ---- 0x3D52C: scroll and redraw --------------------------------------------------------------------------------- */

void f3D52C(int dx, int dy)
{
    int32_t col, row;                                           /* [ebp-0x30], [ebp-0x2c] */
    int32_t sx, sy;                                             /* [ebp-0x18], [ebp-0x14] */
    int32_t t, k, idx, m;                                       /* [ebp-0x20], [ebp-0x1c], [ebp-0x24], [ebp-0x28] */
    int32_t v;

    col = S32(0xF4884u) + dx;                                   /* 0x3D546..0x3D550 */
    row = S32(0xF4888u) + dy;                                   /* 0x3D553..0x3D55D */
    if ((int32_t)S16(0xF6110u) > col)                           /* 0x3D560 dword [0xF610E] sar 16; cmp; jle */
        col = (int32_t)S16(0xF6110u);                           /* 0x3D56D, 0x3D588 */
    else if ((int32_t)S16(0xF6112u) < col)                      /* 0x3D574 dword [0xF6110] sar 16; cmp; jge */
        col = (int32_t)S16(0xF6112u);                           /* 0x3D583, 0x3D588 */
    if (row < 0)                                                /* 0x3D58E */
        row = 0;
    else if (row > 7)                                           /* 0x3D59E */
        row = 7;
    setD32(0xF4884u, (uint32_t)col);                            /* 0x3D5AD */
    setD32(0xF4888u, (uint32_t)row);                            /* 0x3D5B5 */
    sx = f449B8();                                              /* 0x3D5BA */
    sy = f449E0();                                              /* 0x3D5C2 */
    fg_vbcopy(sx, sx + 0x26F, sy, sy + 0x13E, 8, 0x144,         /* 0x3D5F9: 624 x 319, lower-left (8, 0x144) */
              S32(0xF52C4u), S32(0xF52C8u));                    /*   map picture -> back buffer */
    fg_vbopen(S32(0xF52C8u));                                   /* 0x3D608 */
    fg_setclip(8, 0x277, 6, 0x144);                             /* 0x3D61E */

    for (t = 0; t < 0xA0; t++)                                  /* 0x3D626, 0x3D633 */
        if (D8(0xFFB60u + (uint32_t)t) != 0)                    /* 0x3D63F test dl,dl */
            f3D18C(t, (int)D8(0xFFB60u + (uint32_t)t));         /* 0x3D65B */

    /* markers at point 3 */
    for (t = 0; t < 0x9D; t++) {                                /* 0x3D662, 0x3D767 */
        if (D8(GAME_INPLAY + (uint32_t)t) == 0)                 /* 0x3D777 test dh,dh */
            continue;
        if (terr16(t, GAME_TERR_X3) >= 0x1388)                  /* 0x3D792 dword [35t + 0xE1645] sar 16; cmp 5000 */
            continue;
        idx = (int32_t)((uint8_t)(D8(GAME_TERR + 35u * (uint32_t)t + 1u) << 2) >> 5);   /* 0x3D7B3..0x3D7C4 */
        if (area8(t, 1) == 1) {                                 /* 0x3D7D2 dword [69(t+1) + 0xE31B6] sar 24; cmp 1 */
            for (k = 1; k <= 9; k++)                            /* 0x3D7E5, 0x3D672 cmp 9; jg */
                if (D8(0xE31B9u + 69u * (uint32_t)(t + 1) + (uint32_t)k) != 0      /* 0x3D68F */
                    && D8(0xF14F3u + 199u * (uint32_t)k) != 0) {                    /* 0x3D6A0 nation k +0x27 */
                    idx++;                                      /* 0x3D703 */
                    break;
                }
        } else if (area8(t, 1) == 0                             /* 0x3D6B7 sar 24; test; jne */
                   && (D8(0xE2C1Au + 3u * (uint32_t)t) & 1) != 0                    /* 0x3D6D1 */
                   && D8(0xF14F3u + 199u * (uint32_t)D8(0xE2C18u + 3u * (uint32_t)t)) != 0) {   /* 0x3D6E6..0x3D6F7 */
            idx++;                                              /* 0x3D703 */
        }
        m = (idx > 0) ? idx - 1 : 0;                            /* 0x3D706..0x3D714 */
        f3AF8C(0xF4D2Cu + 8u * (uint32_t)m,                     /* 0x3D75C */
               terr16(t, GAME_TERR_X3) - sx,                    /* 0x3D745 */
               terr16(t, GAME_TERR_Y3) - sy);                   /* 0x3D728 */
    }

    /* cities at point 2 */
    for (t = 0; t < 0x9D; t++) {                                /* 0x3D7ED, 0x3D7FA */
        if ((int32_t)S8(0xC1A34u) < 1)                          /* 0x3D807 dword [0xC1A31] sar 24; cmp 1; jl */
            continue;
        if ((D8(0xE2C1Au + 3u * (uint32_t)t) & 1) == 0)         /* 0x3D820 */
            continue;
        if (terr16(t, GAME_TERR_X2) >= 0x1388)                  /* 0x3D83C dword [35t + 0xE1641] sar 16 */
            continue;
        v = (int32_t)D8(0xE2C18u + 3u * (uint32_t)t);           /* 0x3D85C: the owner */
        f3AF8C(0xF497Cu + 8u * (uint32_t)v,                     /* 0x3D8AA */
               terr16(t, GAME_TERR_X2) - sx,                    /* 0x3D893 */
               terr16(t, GAME_TERR_Y2) - sy);                   /* 0x3D876 */
    }

    if (D8(0xF4102u) == 5 && (int32_t)S8(0x108804u) == 2) {     /* 0x3D8B4, 0x3D8BD dword [0x108801] sar 24 */
        f3DAD8(sx, sy);                                         /* 0x3D8D0 tokens, then the ships on top */
        f3DE40(sx, sy);                                         /* 0x3D8DB */
    } else {
        f3DE40(sx, sy);                                         /* 0x3D8E8 ships, then the tokens on top */
        f3DAD8(sx, sy);                                         /* 0x3D8F3 */
    }
    fg_setclip(0, 0x27F, 0, 0x1DF);                             /* 0x3D906 */
    f78884();                                                   /* 0x3D90E */
    fg_vbpaste(0, 0x27F, 0, 0x144, 0, 0x144);                   /* 0x3D928 rows 0..0x144 to the screen */
    f788AC();                                                   /* 0x3D930 */
    fg_vbclose();                                               /* 0x3D935 */

    fg_vbopen(S32(0xF52C8u));                                   /* 0x3D941 */
    f3AF8C(GAME_SPR_MINIMAP, 0x1E1, 0x170);                     /* 0x3D958 */
    fg_setcolor(0xF);                                           /* 0x3D95F */
    fg_boxdepth(1, 1);                                          /* 0x3D96B */
    v = (S32(0xF4888u) * 7) / 2;                                /* 0x3D970..0x3D986 lea 8r; sub r; cdq-style sar */
    fg_box(S32(0xF4884u) * 7 + 0x1E1, S32(0xF4884u) * 7 + 0x218,   /* 0x3D9B4: x 0x1E1+7c .. 0x218+7c */
           v + 0x170, v + 0x18A);                               /*   y 0x170+7r/2 .. 0x18A+7r/2 */
    fg_vbclose();                                               /* 0x3D9BC */
    f78884();                                                   /* 0x3D9C1 */
    fg_vbpaste(0x1E0, 0x27B, 0x16F, 0x1A3, 0x1E0, 0x1A3);       /* 0x3D9E4 the mini-map rectangle */
    f788AC();                                                   /* 0x3D9EC */
}

/* ---- 0x3DAD8: land tokens --------------------------------------------------------------------------------------- */

void f3DAD8(int sx, int sy)
{
    int32_t t, k, i, n;                                         /* [ebp-0x2c], [ebp-0x28], [ebp-0x20], [ebp-0x24] */
    int32_t x, y;                                               /* [ebp-0x34], [ebp-0x30] */
    int32_t a, b, c, d;                                         /* [ebp-0x1c], [ebp-0x18], [ebp-0x14], [ebp-0x10] */
    int32_t p, v;

    for (t = 0; t < 0x9D; t++) {                                /* 0x3DAF1, 0x3DE12 */
        if ((int32_t)S8(0xC1A34u) < 2)                          /* 0x3DE1B dword [0xC1A31] sar 24; cmp 2; jl */
            continue;
        i = 0;                                                  /* 0x3DE2A */
        for (k = 0; k < 0xD; k++) {                             /* 0x3DE2D, 0x3DB11 cmp 0xD; jge */
            v = area8(t, 0x0Fu + (uint32_t)k);                  /* 0x3DB1E dword [69(t+1) + k + 0xE31C4] sar 24 */
            if (v <= 0)                                         /* 0x3DB27 */
                continue;
            if (area8(t, 0x0Fu + (uint32_t)k) > 0x1D)           /* 0x3DB3E, 0x3DB47 cmp 0x1D; jg */
                continue;
            if (S32(0xF40F8u) - 1 == k)                         /* 0x3DB4C..0x3DB55 */
                continue;
            x = terr16(t, GAME_TERR_X0) - sx + 7 * i;           /* 0x3DB68 dword [35t + 0xE1639] sar 16 .. 0x3DB82 */
            y = terr16(t, GAME_TERR_Y0) - sy + 7 * i;           /* 0x3DB96 dword [35t + 0xE163B] sar 16 .. 0x3DBB0 */
            n = k + 1;                                          /* 0x3DBB7 */
            f3AF8C(0xF489Cu + 8u * (uint32_t)n, x, y);          /* 0x3DBCB */
            f3E98C((int)area8(t, 0x0Fu + (uint32_t)k), x, y);   /* 0x3DBE9, 0x3DBF4 */
            i++;                                                /* 0x3DBFC */
        }
        /* the current player's stack, last, on top */
        p = S32(0xF40F8u);                                      /* 0x3DC04 */
        if (area8(t, 0x0Eu + (uint32_t)p) <= 0)                 /* 0x3DC0A dword [P + 69(t+1) + 0xE31C3] sar 24 */
            continue;
        if (area8(t, 0x0Eu + (uint32_t)p) > 0x1D)               /* 0x3DC2C, 0x3DC36 */
            continue;
        x = terr16(t, GAME_TERR_X0) - sx + 7 * i;               /* 0x3DC50..0x3DC6A */
        y = terr16(t, GAME_TERR_Y0) - sy + 7 * i;               /* 0x3DC7E..0x3DC98 */
        n = p;                                                  /* 0x3DC9B */
        f3AF8C(0xF489Cu + 8u * (uint32_t)n, x, y);              /* 0x3DCAF */
        a = b = c = d = 0;                                      /* 0x3DCB4..0x3DCBF */
        fg_setcolor((int)D8(0xFFC00u + (uint32_t)t));           /* 0x3DCD1 */
        if ((int32_t)S8(0x108804u) == 1 && D8(0xFFC00u + (uint32_t)t) != 0) {   /* 0x3DCD6..0x3DCF0 */
            fg_boxdepth(2, 2);                                  /* 0x3DCF6 */
            a = b = 1;                                          /* 0x3DD03, 0x3DD06 */
            c = d = 2;                                          /* 0x3DD0E, 0x3DD11 */
        }
        fg_box(x - a, x - a + 0x12 + c - 1, y - b, y - b + 0x12 + d - 1);   /* 0x3DD3E */
        if ((int32_t)S8(0x108804u) == 1 && D8(0xFFC00u + (uint32_t)t) != 0)  /* 0x3DD43..0x3DD5D */
            fg_boxdepth(1, 1);                                  /* 0x3DD63 */
        f3E98C((int)area8(t, 0x0Eu + (uint32_t)n), x, y);       /* 0x3DD84, 0x3DD8F */
        if (D8(0xF4102u) == 5) {                                /* 0x3DD94 */
            p = S32(0xF40F8u);                                  /* 0x3DDAB */
            if (area8(t, 0x24u + (uint32_t)p) <= 0)             /* 0x3DDB3 dword [P + 69(t+1) + 0xE31D9] sar 24 */
                continue;
            if (area8(t, 0x24u + (uint32_t)p) > 0x1D)           /* 0x3DDD1, 0x3DDDB */
                continue;
            f3E9DC((int)area8(t, 0x24u + (uint32_t)p), x + 0x12, y);   /* 0x3DDF9, 0x3DE07 */
        }
    }
}

/* ---- 0x3DE40: ships --------------------------------------------------------------------------------------------- */

void f3DE40(int sx, int sy)
{
    int32_t t, k, i, n;                                         /* [ebp-0x34], [ebp-0x30], [ebp-0x24], [ebp-0x2c] */
    int32_t x, y;                                               /* [ebp-0x44], [ebp-0x40] */
    int32_t a, b, c, d;                                         /* [ebp-0x20], [ebp-0x1c], [ebp-0x18], [ebp-0x14] */
    int32_t xx, yy, w;                                          /* [ebp-0x3c], [ebp-0x38], [ebp-0x28] */
    int32_t p, v;
    uint32_t s;

    for (t = 0; t < 0xA0; t++) {                                /* 0x3DE5A, 0x3E348 */
        if ((int32_t)S8(0xC1A34u) < 3)                          /* 0x3E351 dword [0xC1A31] sar 24; cmp 3; jl */
            continue;
        i = 0;                                                  /* 0x3E360 */
        for (k = 0; k < 9; k++) {                               /* 0x3E363, 0x3DE7A cmp 9; jge */
            v = area8(t, 0x1Cu + (uint32_t)k);                  /* 0x3DE87 dword [69(t+1) + k + 0xE31D1] sar 24 */
            if (v <= 0)                                         /* 0x3DE90 */
                continue;
            if (area8(t, 0x1Cu + (uint32_t)k) > 4)              /* 0x3DEA7, 0x3DEB0 cmp 4; jg */
                continue;
            if (S32(0xF40F8u) - 1 == k)                         /* 0x3DEB5..0x3DEBE */
                continue;
            x = terr16(t, GAME_TERR_X1) - sx + 7 * i;           /* 0x3DED1 dword [35t + 0xE163D] sar 16 .. 0x3DEEB */
            y = terr16(t, GAME_TERR_Y1) - sy + 7 * i;           /* 0x3DEFF dword [35t + 0xE163F] sar 16 .. 0x3DF19 */
            n = k + 1;                                          /* 0x3DF20 */
            f3AF8C(0xF490Cu + 8u * (uint32_t)n, x, y);          /* 0x3DF34 */
            f3E98C((int)area8(t, 0x1Cu + (uint32_t)k), x, y);   /* 0x3DF52, 0x3DF5D */
            i++;                                                /* 0x3DF65 */
        }
        /* the current player's ships, last, on top */
        p = S32(0xF40F8u);                                      /* 0x3DF6D */
        if (area8(t, 0x1Bu + (uint32_t)p) <= 0)                 /* 0x3DF73 dword [P + 69(t+1) + 0xE31D0] sar 24 */
            continue;
        if (area8(t, 0x1Bu + (uint32_t)p) > 4)                  /* 0x3DF95, 0x3DF9F */
            continue;
        x = terr16(t, GAME_TERR_X1) - sx + 7 * i;               /* 0x3DFB9..0x3DFD3 */
        y = terr16(t, GAME_TERR_Y1) - sy + 7 * i;               /* 0x3DFE7..0x3E005 */
        n = p;                                                  /* 0x3E008 */
        f3AF8C(0xF490Cu + 8u * (uint32_t)n, x, y);              /* 0x3E01E */
        a = b = c = d = 0;                                      /* 0x3E023..0x3E02C */
        fg_setcolor((int)D8(0xFFCA0u + (uint32_t)t));           /* 0x3E03E */
        if ((int32_t)S8(0x108804u) == 2 && D8(0xFFCA0u + (uint32_t)t) != 0) {   /* 0x3E043..0x3E05D */
            fg_boxdepth(2, 2);                                  /* 0x3E063 */
            a = b = 1;                                          /* 0x3E070, 0x3E073 */
            c = d = 2;                                          /* 0x3E07B, 0x3E07E */
        }
        fg_box(x - a, x - a + 0x1C + c - 1, y - b, y - b + 0x12 + d - 1);   /* 0x3E0AB: 28 x 18 */
        if ((int32_t)S8(0x108804u) == 2 && D8(0xFFCA0u + (uint32_t)t) != 0)  /* 0x3E0B0..0x3E0CA */
            fg_boxdepth(1, 1);                                  /* 0x3E0D0 */

        if (D8(0xF4102u) == 5) {                                /* 0x3E0D8 */
            p = S32(0xF40F8u);                                  /* 0x3E0F1 */
            s = D8(0xE31E5u + 69u * (uint32_t)(t + 1) + (uint32_t)p);   /* 0x3E0F6: area +0x2D+P */
            if ((int32_t)S8(GAME_NATION + 199u * (uint32_t)p + 0x4Du + s) > 0)  /* 0x3E109 dword [s + 199P + 0xF1516] sar 24 */
                f3AF8C((int32_t)S8(0x108804u) == 2 ? 0xF517Cu : 0xF5184u,      /* 0x3E123..0x3E139 */
                       x - a, y - b);                           /* 0x3E13E: edx = x - [ebp-0x20], ebx = y - [ebp-0x1c] */
        }
        f3E98C((int)area8(t, 0x1Bu + (uint32_t)n), x, y);       /* 0x3E15C, 0x3E167 */
        if (D8(0xF4102u) == 5) {                                /* 0x3E16C */
            p = S32(0xF40F8u);                                  /* 0x3E189 */
            s = D8(0xE31E5u + 69u * (uint32_t)(t + 1) + (uint32_t)p);   /* 0x3E18E */
            if ((int32_t)S8(GAME_NATION + 199u * (uint32_t)p + 0x4Du + s) > 0) {   /* 0x3E1A1..0x3E1AD */
                xx = x + 8;                                     /* 0x3E1B6 */
                yy = y + 4;                                     /* 0x3E1BF */
                f39F90(0);                                      /* 0x3E1C7 */
                f3AF8C(0xF4E3Cu + 8u * (uint32_t)n, xx, yy);    /* 0x3E1DD */
                p = S32(0xF40F8u);                              /* 0x3E1F2 */
                s = D8(0xE31E5u + 69u * (uint32_t)(t + 1) + (uint32_t)p);   /* 0x3E1F7 */
                snprintf((char *)DP(0xF5500u), 0x1F4, DSTR(0xB3F08u),       /* 0x3E21F sprintf(0xF5500, "%d", ...) */
                         (int)S8(GAME_NATION + 199u * (uint32_t)p + 0x4Du + s));   /* 0x3E20A */
                fg_setcolor(5);                                 /* 0x3E229 */
                f3A0B0(xx + 1, yy + 2, 0xB, DSTR(0xF5500u));    /* 0x3E245 */
                if (n == 2)                                     /* 0x3E24A cmp dword [ebp-0x2c],2 */
                    f3A0B0(xx + 1, yy + 1, 0xB, DSTR(0xF5500u));   /* 0x3E262 */
                fg_setcolor(0xF);                               /* 0x3E269 */
                f3A0B0(xx, yy + 1, 0xB, DSTR(0xF5500u));        /* 0x3E282 */
                f39F90(1);                                      /* 0x3E28C */
            }
        }
        if (D8(0xF4102u) == 5) {                                /* 0x3E291 */
            p = S32(0xF40F8u);                                  /* 0x3E2AE */
            s = D8(0xE31E5u + 69u * (uint32_t)(t + 1) + (uint32_t)p);   /* 0x3E2B4 */
            if ((int32_t)D8(GAME_NATION + 199u * (uint32_t)p + 0x41u + s) <= 0)  /* 0x3E2C6 byte [199P + s + 0xF150D]; test; jle */
                continue;
            w = (int32_t)D8(GAME_NATION + 199u * (uint32_t)p + 0x41u + s);   /* 0x3E2E8..0x3E2F8 */
            if (w < 0)                                          /* 0x3E2FE (cannot happen: a byte) */
                continue;
            if (w > 5)                                          /* 0x3E302 */
                continue;
            f3EA2C((int)D8(GAME_NATION + 199u * (uint32_t)p + 0x41u + s), x + 0x1C, y);   /* 0x3E321..0x3E33D */
        }
    }
}

/* ---- 0x3E98C, 0x3E9DC, 0x3EA2C: number sprites ----------------------------------------------------------------- */

void f3E98C(int n, int x, int y)
{
    f3AF8C(0xF4EACu + 8u * (uint32_t)n,                         /* 0x3E9C9 shl 3; 0x3E9CC add 0xF4EAC; 0x3E9D1 */
           x - S32(0xC2820u + 4u * (uint32_t)(n / 10)),         /* 0x3E9BA idiv 10; 0x3E9BF */
           y - 4);                                              /* 0x3E9AE */
}

void f3E9DC(int n, int x, int y)
{
    f3AF8C(0xF4F9Cu + 8u * (uint32_t)n,                         /* 0x3EA1C, 0x3EA21 */
           x - S32(0xC2820u + 4u * (uint32_t)(n / 10)),         /* 0x3EA0A, 0x3EA0F */
           y - 4);                                              /* 0x3E9FE */
}

void f3EA2C(int n, int x, int y)
{
    f3AF8C(0xF508Cu + 8u * (uint32_t)n,                         /* 0x3EA6C, 0x3EA71 */
           x - S32(0xC282Cu + 4u * (uint32_t)(n / 10)),         /* 0x3EA5A, 0x3EA5F */
           y - 4);                                              /* 0x3EA4E */
}

/* ---- 0x3CBBC: lower panel to the screen ------------------------------------------------------------------------- */

void f3CBBC(void)
{
    f78884();                                                   /* 0x3CBCC: mouse hidden first (f3CC14: after) */
    fg_vbopen(S32(0xF52C8u));                                   /* 0x3CBD8 */
    fg_vbpaste(0, 0x27F, 0x145, 0x1DF, 0, 0x1DF);               /* 0x3CBF8 rows 0x145..0x1DF */
    fg_vbclose();                                               /* 0x3CC00 */
    f788AC();                                                   /* 0x3CC05 */
}

/* ---- 0x787F8, 0x7876C: panel labels in the back buffer ---------------------------------------------------------- */

void f787F8(int t)
{
    fg_vbopen(S32(0xF52C8u));                                   /* 0x78815 */
    f39F90(1);                                                  /* 0x78822 */
    fg_setcolor(0x3C);                                          /* 0x78829 */
    fg_rect(0x1F9, 0x264, 0x1AE, 0x1BE);                        /* 0x78845 */
    fg_setcolor(5);                                             /* 0x7884F */
    f3A0B0(0x1F9, 0x1B1, 0x6C, DSTR(D32(0xC77D8u + 4u * (uint32_t)t)));   /* 0x78869, 0x78870 */
    fg_vbclose();                                               /* 0x78875 */
}

void f7876C(void)
{
    fg_vbopen(S32(0xF52C8u));                                   /* 0x78783 */
    f39F90(1);                                                  /* 0x78790 */
    fg_setcolor(0x3C);                                          /* 0x78797 */
    fg_rect(0x1F3, 0x268, 0x14F, 0x15E);                        /* 0x787B3 */
    fg_setcolor(5);                                             /* 0x787BD */
    f3A0B0(0x1E0, 0x152, 0x9B,                                  /* 0x787E4 */
           DSTR(D32(0xCA9F4u + 4u * (uint32_t)(int32_t)S8(0xF4102u))));   /* 0x787CC dword [0xF40FF] sar 24; 0x787DD */
    fg_vbclose();                                               /* 0x787E9 */
}
