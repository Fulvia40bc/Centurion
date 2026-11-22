/* Module PANEL of docs/MAP_PLAN.md, first half: the nation status panel 0x459E4, the whole-board overview 0x3E374 (the
   ZOOM view), the message-strip eraser 0x3A990, the mirrored sprite 0x3AFF4, the player button labels 0x783B8, and the
   append / remove of a dialog's records to / from the panel dialog 0xF596C (0x7853C / 0x78664). Literal translations;
   the address of each instruction a line comes from is in its comment. D = read in the code, G = inference.
   Prototypes: panel.h. The ship panel 0x5D14C and its helpers are the other half (shippanel.h).

   Watcom runtime -> standard C:
     0x8C897 itoa(value, buf, 10) -> snprintf "%d" into a buffer of the original's size (32 bytes at [ebp-0x30] in
             0x459E4); for radix 10 Watcom writes '-' and the magnitude, as "%d" does (LIBRARY_CALLS.md)
     0x7DB3D sprintf -> snprintf bounded to [0xF5500]'s 500 bytes (as map.c and load.c)
     0x7D5D0 memset -> memset
     rep movsd (5 dwords) -> memmove of 20 bytes inside g_ds (the original copies dword by dword upwards; the ranges of
             0x78664 overlap only with the destination below the source, where an upward copy equals memmove)
   __CHK dropped. Library calls through fg.h in the original argument order (Fastgraph pushes right to left).

   Data (D, read in the code at the addresses given):
     nation n at 0xF14CC + 199n: +0x3E int8 (G: treasury), +0x40 uint8 (G: ships built; read as a byte & 0xFF, the
       only unsigned read here), +0x45 int16[4] (G: ship s's territory + 1 in the low byte), +0x51 int8 (G: cities on
       the map), +0x98 int8 (G: tokens on the map), +0x99 int8 (G: tokens in stock), +0x9A int8 (0 = not in play)
     0xE2C18 + 3t: +0 uint8 the city owner, +2 bit 0 a city stands there (map.c)
     area record t at 0xE31B8 + 69t (territory t - 1): +0x0F + k int8 tokens of nation k + 1 (k 0..12)
     overview points (int16[160], game.h, from TERRFLAG.DAT scaled to 624 x 319): 0xEE270 / 0xEE3B0 point 0 (tokens),
       0xEE770 / 0xEE8B0 point 1 (ships), 0xEE4F0 / 0xEE630 point 2 (cities); the code adds the map margin (8, 6)
     0xC1A38 uint32[] -> zoom3.gif .. zoom6.gif, zoom78.gif (index [0xF40F0], 3..8; obj2), shown from CT1 [0xF40D4]
     0xF06B0 int16[8] the nation of player p (G: turn order); 0xF06C0 uint8[] the player of nation n (written by
       0x783B8); 0xC768C uint32[] -> nation names
     0xF596C the map screen's panel dialog: int16 count, records of 20 bytes from 0xF596E (ui.h's layout)
   Sprite records (8 bytes, ui.h; +0 a ui_sprite_put handle in the port): 0xF4ACC + 8n tokens in stock, 0xF4BAC + 8n
   cities in stock, 0xF4B3C + 8n ships, 0xF489C + 8n tokens on the map, 0xF497C + 8n cities on the map, 0xF49EC + 8n
   treasury (game.h, 0x11A8C); 0xF4DCC + 8n overview cities, 0xF4D5C + 8n overview ships, 0xF4E3C + 8n overview
   tokens (11 x 10 each).

   Heap pointers in data fields: none written. The sprite records' +0 handles are read through ui_sprite_pixels
   (f3AF8C, f3AFF4); [0xF52C8] / [0xF52CC] are Fastgraph buffer handles; [0xF40D4] is the port's catalog handle.

   1995 bugs (fixed unless g_original):
     1. 0x3E374 draws the ships of every nation k 1..10 except k == P - 1, then those of k == P - 1 in a second loop
       that runs only for k 1..8 (0x3E493, 0x3E548, 0x3E552). k is a 1-based nation there (nation record 199k, sprite
       0xF4D5C + 8k), so the P - 1 (right in the token loops, where k is 0-based: area +0x0F + k, sprite 0xF4E3C +
       8(k + 1)) puts nation P - 1's ships on top of a shared territory instead of the viewer's own (D for the code,
       G for the intent). The fix compares with P and lets the second loop run to 10 like the first; without the
       second change the fix would lose Egypt's (P = 9) own ships altogether. The two orders draw the same sprites; only
       the 2-pixel stacking order in a territory with ships of P - 1 or P and others differs.
   Oddities, kept (D):
     - 0x459E4 draws the name box (y 0x14F..0x15E) into the back buffer, outside the rows it restores and pastes
       (0x162..0x1C5): the name reaches the screen with the caller's next paste (the player buttons call f3CBBC right
       after), and with n outside 1..9 the previous name stays in the back buffer.
     - 0x3E374 prints each token count once more in colour 5 at (+1, +1) for k == 2 (nation 3, ILLYRIA), as the AST
       chart does for that nation (ast.c), and as map.c's ship value does for the current player 2 (G: the nation's
       colour needs the heavier shadow).
     - 0x3E374 takes a ship's territory as the low byte of the word, minus 1, without a check: a word with low byte 0
       reads point -1 (0xEE76E / 0xEE8AE) and the byte under the counter array; a low byte above 0xA0 increments a byte
       of the stack above the array (G: never in a real game). The port keeps the reads in g_ds and gives those
       counters scratch bytes (port guard, G); in the original they are stack garbage.
     - 0x3E374 draws cities only for t < 157 (0x3E3ED) and token stacks for area records 1..157 (territories 0..156),
       as map.c does for tokens. */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "fg.h"
#include "game.h"
#include "panel.h"
#include "state.h"
#include "ui.h"

#define BACKBUF   0xF52C8u   /* int32 virtual-buffer handle of the back buffer */
#define BGBUF     0xF52CCu   /* int32 handle of the background-picture buffer */
#define PANEL_DLG 0xF596Cu   /* the map screen's panel dialog */

/* nation n's record field */
static uint32_t nat(int32_t n, uint32_t off)
{
    return GAME_NATION + 199u * (uint32_t)n + off;
}

/* ---- 0x459E4: nation status panel ------------------------------------------------------------------------------- */

void f459E4(int n)
{
    char buf[0x20];                                                 /* [ebp-0x30] */

    f3CD6C(S8(0xEE168u));                                           /* 0x459FA dword [0xEE165] sar 24; 0x45A02 */
    fg_vbcopy(0x3C, 0x9A, 0x162, 0x1C5, 0x3C, 0x1C5,                /* 0x45A2D: lower-left (0x3C, 0x1C5) */
              S32(BGBUF), S32(BACKBUF));
    fg_vbopen(S32(BACKBUF));                                        /* 0x45A3C */
    fg_setcolor(5);                                                 /* 0x45A46 */
    if (n >= 1 && n <= 9) {                                         /* 0x45A4E jl, 0x45A58 jg -> 0x45CF9 */
        fg_setcolor(0x3C);                                          /* 0x45A64 */
        fg_rect(0x3A, 0x9C, 0x14F, 0x15E);                          /* 0x45A7D */
        fg_setcolor(5);                                             /* 0x45A87 */
        f3A0B0(0x3A, 0x152, 0x63, DSTR(D32(0xC768Cu + 4u * (uint32_t)n)));   /* 0x45AA8 */
        fg_setcolor(0x9B);                                          /* 0x45AB2 */
        f39F90(0);                                                  /* 0x45ACB font 0 (BLOCK05) */
        f3A0B0(0x3C, 0x162, 0x5F, DSTR(0xB471Bu));                  /* 0x45AD5 "IN STOCK" (edx, ebx, ecx kept over 0x39F90) */
        f3A0B0(0x3C, 0x18E, 0x5F, DSTR(0xB4724u));                  /* 0x45AEE "ON MAP" */
        f3A0B0(0x3C, 0x1A9, 0x5F, DSTR(0xB472Bu));                  /* 0x45B07 "TREASURY" */
        f39F90(1);                                                  /* 0x45B11 */
        fg_setcolor(5);                                             /* 0x45B18 */

        /* in stock: tokens, cities, ships */
        f3AF8C(0xF4ACCu + 8u * (uint32_t)n, 0x3D, 0x16B);           /* 0x45B35 */
        snprintf(buf, sizeof buf, "%d", (int)S8(nat(n, 0x99)));     /* 0x45B46 dword [199n + 0xF1562] sar 24; 0x45B55 itoa */
        f3A0B0(0x3E, 0x182, 0x16, buf);                             /* 0x45B69 */
        f3AF8C(0xF4BACu + 8u * (uint32_t)n, 0x57, 0x16B);           /* 0x45B83 */
        snprintf(buf, sizeof buf, "%d", 9 - (int)S8(nat(n, 0x51)));  /* 0x45B8F dword [199n + 0xF151A] sar 24; 0x45B9D sub; 0x45BAC */
        f3A0B0(0x57, 0x182, 0x18, buf);                             /* 0x45BC0 */
        f3AF8C(0xF4B3Cu + 8u * (uint32_t)n, 0x72, 0x16B);           /* 0x45BDA */
        snprintf(buf, sizeof buf, "%d", 4 - (int)D8(nat(n, 0x40)));  /* 0x45BE6 mov al,[199n + 0xF150C]; and 0xFF; 0x45C05 */
        f3A0B0(0x72, 0x182, 0x26, buf);                             /* 0x45C19 */

        /* on the map: tokens, cities */
        f3AF8C(0xF489Cu + 8u * (uint32_t)n, 0x49, 0x195);           /* 0x45C33 */
        snprintf(buf, sizeof buf, "%d", (int)S8(nat(n, 0x98)));     /* 0x45C44 dword [199n + 0xF1561] sar 24; 0x45C50 */
        f3A044(0x5C, 0x199, buf);                                   /* 0x45C62 */
        f3AF8C(0xF497Cu + 8u * (uint32_t)n, 0x72, 0x197);           /* 0x45C7C */
        snprintf(buf, sizeof buf, "%d", (int)S8(nat(n, 0x51)));     /* 0x45C8D dword [199n + 0xF151A] sar 24; 0x45C99 */
        f3A044(0x86, 0x199, buf);                                   /* 0x45CAB */

        /* treasury */
        f3AF8C(0xF49ECu + 8u * (uint32_t)n, 0x59, 0x1AF);           /* 0x45CC5 */
        snprintf(buf, sizeof buf, "%d", (int)S8(nat(n, 0x3E)));     /* 0x45CD6 dword [199n + 0xF1507] sar 24; 0x45CE2 */
        f3A044(0x74, 0x1B5, buf);                                   /* 0x45CF4 */
    }
    f78884();                                                       /* 0x45CF9 */
    fg_vbpaste(0x3C, 0x9A, 0x162, 0x1C5, 0x3C, 0x1C5);              /* 0x45D16 */
    f788AC();                                                       /* 0x45D1E */
    fg_vbclose();                                                   /* 0x45D23 */
}

/* ---- 0x3E374: the whole-board overview (ZOOM) ------------------------------------------------------------------- */

/* One stack of ships of nation k at territory t: the loop body 0x3E4AB..0x3E534 (and its copy 0x3E580..0x3E609).
   cnt[t] is the local uint8[160] at [ebp-0xD4], read signed (dword [t + ebp - 0xD7] sar 24). */
static void overview_ships(int32_t k, int8_t *cnt)
{
    int32_t s, t, x, y, d;                                          /* [ebp-0x20], [ebp-0x1C], [ebp-0x28], [ebp-0x24] */

    for (s = 0; s < 4; s++) {                                       /* 0x3E49A, 0x3E4A5 cmp 4; jge */
        if (D16(nat(k, 0x45 + 2u * (uint32_t)s)) == 0)              /* 0x3E4B5 mov bx,[199k + 2s + 0xF1511]; test bx,bx */
            continue;
        t = (int32_t)(D16(nat(k, 0x45 + 2u * (uint32_t)s)) & 0xFF) - 1;   /* 0x3E4CC; xor ah,ah; cwde; dec */
        d = 2 * (int32_t)cnt[t];                                    /* 0x3E4EB..0x3E4F8 */
        x = (int32_t)S16((uint32_t)(0xEE770 + 2 * t)) + 8 + d;      /* 0x3E4DE dword [2t + 0xEE76E] sar 16; 0x3E4FC */
        y = (int32_t)S16((uint32_t)(0xEE8B0 + 2 * t)) + 6 + d;      /* 0x3E502 dword [2t + 0xEE8AE] sar 16; 0x3E511 */
        f3AF8C(0xF4D5Cu + 8u * (uint32_t)k, x, y);                  /* 0x3E525 */
        cnt[t]++;                                                   /* 0x3E52D inc byte [t + ebp - 0xD4] */
    }
}

/* One token stack of nation k + 1 in area record t: the loop body 0x3E66C..0x3E74C (and its copy 0x3E795..0x3E875,
   whose sprintf uses the second "%d" at 0xB3F0E). */
static void overview_tokens(int32_t t, int32_t k, int32_t *num, uint32_t fmt)
{
    int32_t x, y;                                                   /* [ebp-0x28], [ebp-0x24] */
    char *msg = (char *)DP(0xF5500u);

    x = (int32_t)S16((uint32_t)(0xEE270 + 2 * (t - 1))) + 8 + 2 * *num;   /* 0x3E66F dword [2t + 0xEE26C] sar 16; 0x3E683 */
    y = (int32_t)S16((uint32_t)(0xEE3B0 + 2 * (t - 1))) + 6 + 2 * *num;   /* 0x3E689 dword [2t + 0xEE3AC] sar 16; 0x3E69D */
    f3AF8C(0xF4E3Cu + 8u * (uint32_t)k + 8u, x, y);                 /* 0x3E6B4 */
    snprintf(msg, 0x1F4, DSTR(fmt),                                 /* 0x3E6DF sprintf(0xF5500, "%d", ...) */
             (int)S8(GAME_AREA + 69u * (uint32_t)t + 0x0F + (uint32_t)k));   /* 0x3E6CB dword [69t + k + 0xE31C4] sar 24 */
    fg_setcolor(5);                                                 /* 0x3E6E9 */
    f3A0B0(x + 1, y + 2, 0x0B, msg);                                /* 0x3E705 */
    if (k == 2)                                                     /* 0x3E70D cmp ecx,2; jne */
        f3A0B0(x + 1, y + 1, 0x0B, msg);                            /* 0x3E724 */
    fg_setcolor(0x0F);                                              /* 0x3E72B */
    f3A0B0(x, y + 1, 0x0B, msg);                                    /* 0x3E744 */
    (*num)++;                                                       /* 0x3E74C inc dword [ebp-0x2c] */
}

void f3E374(int P)
{
    int32_t t, k, num;                                              /* [ebp-0x34] / [ebp-0x18], [ebp-0x34], [ebp-0x2C] */
    int32_t own, last;
    /* [ebp-0xD4]: the ship counters, uint8[160]. Port guard (G): one scratch byte below and 95 above, for the
       unchecked territory -1..254 (file comment); the original writes stack bytes there. */
    int8_t cnt_mem[1 + 0xFF];
    int8_t *cnt = cnt_mem + 1;

    fg_vbopen(S32(BACKBUF));                                        /* 0x3E396 */
    fg_move(8, 6);                                                  /* 0x3E3A2 */
    cat_show(DSTR(D32(0xC1A38u + 4u * D32(0xF40F0u))), 3,           /* 0x3E3B8 [4*[0xF40F0] + 0xC1A38]; 0x3E3C0 */
             S32(0xF40D4u));                                        /*   CT1 */
    if (S8(0xC1A34u) > 1) {                                         /* 0x3E3C5 dword [0xC1A31] sar 24; cmp 1; jle 0x3E887 */
        f39F90(0);                                                  /* 0x3E3DD */

        /* cities */
        for (t = 0; t < 0x9D; t++)                                  /* 0x3E3E2, 0x3E3ED cmp 0x9D; jge */
            if (D8(GAME_TSTATE + 3u * (uint32_t)t + 2) & 1)         /* 0x3E402 mov dl,[3t + 0xE2C1A]; test dl,1 */
                f3AF8C(0xF4DCCu + 8u * D8(GAME_TSTATE + 3u * (uint32_t)t),   /* 0x3E439 byte [3t + 0xE2C18]; 0x3E44C */
                       (int32_t)S16((uint32_t)(0xEE4F0 + 2 * t)) + 8,        /* 0x3E420 dword [2t + 0xEE4EE] sar 16 */
                       (int32_t)S16((uint32_t)(0xEE630 + 2 * t)) + 6);       /* 0x3E410 dword [2t + 0xEE62E] sar 16 */

        /* ships: every nation but the viewer's, then the viewer's on top */
        memset(cnt_mem, 0, sizeof cnt_mem);                         /* port: the scratch bytes */
        memset(cnt, 0, 0xA0);                                       /* 0x3E460 memset([ebp-0xD4], 0, 0xA0) */
        if (g_original) {
            /* BUG(1995) 1: k is a 1-based nation in these loops, but the viewer is compared as P - 1 (as in the 0-based
               token loops below), and the second loop stops at 8 (0x3E548 cmp 9; jge). */
            own = P - 1;                                            /* 0x3E48F, 0x3E552 dec eax */
            last = 9;                                               /* 0x3E548 */
        } else {
            own = P;
            last = 0x0B;
        }
        for (k = 1; k < 0x0B; k++) {                                /* 0x3E465, 0x3E474 cmp 0xB; jge */
            if (D8(nat(k, 0x9A)) == 0)                              /* 0x3E485 mov dh,[199k + 0xF1566]; test dh,dh */
                continue;
            if (own == k)                                           /* 0x3E493 cmp eax,[ebp-0x34]; je */
                continue;
            overview_ships(k, cnt);
        }
        for (k = 1; k < last; k++) {                                /* 0x3E539, 0x3E548 cmp 9; jge */
            if (own != k)                                           /* 0x3E559 cmp eax,ebx; jne */
                continue;
            if (D8(nat(k, 0x9A)) == 0)                              /* 0x3E564 cmp byte [199k + 0xF1566],0; je */
                continue;
            overview_ships(k, cnt);                                 /* 0x3E56D..0x3E609 */
        }

        /* token stacks with their counts, the viewer's last */
        for (t = 1; t <= 0x9D; t++) {                               /* 0x3E60E, 0x3E61D cmp 0x9D; jg */
            num = 0;                                                /* 0x3E62C */
            for (k = 0; k < 0x0D; k++) {                            /* 0x3E62F, 0x3E63A cmp 0xD; jge */
                if (S8(GAME_AREA + 69u * (uint32_t)t + 0x0F + (uint32_t)k) <= 0)   /* 0x3E656 dword [69t + k + 0xE31C4] sar 24; jle */
                    continue;
                if (P - 1 == k)                                     /* 0x3E666 dec eax; cmp; je */
                    continue;
                overview_tokens(t, k, &num, 0xB3F0Bu);              /* 0x3E66C..0x3E74F */
            }
            for (k = 0; k < 0x0D; k++) {                            /* 0x3E756, 0x3E761 cmp 0xD; jge */
                if (P - 1 != k)                                     /* 0x3E76E dec eax; cmp eax,ebx; jne */
                    continue;
                if (S8(GAME_AREA + 69u * (uint32_t)t + 0x0F + (uint32_t)k) <= 0)   /* 0x3E788 sar 24; test; jle */
                    continue;
                overview_tokens(t, k, &num, 0xB3F0Eu);              /* 0x3E795..0x3E878 */
            }
        }
        f39F90(1);                                                  /* 0x3E882 */
    }
    fg_vbclose();                                                   /* 0x3E887 */
    f3CC14();                                                       /* 0x3E88C the back buffer to the screen */
}

/* ---- 0x3A990: erase the message strip --------------------------------------------------------------------------- */

void f3A990(void)
{
    int32_t x, w;

    f3CD6C(S8(0xEE168u));                                           /* 0x3A9A0 dword [0xEE165] sar 24; 0x3A9A8 */
    x = (int32_t)S16(0xBB3FAu);                                     /* 0x3A9BB dword [0xBB3F8] sar 16 */
    w = (int32_t)S16(0xBB3FEu);                                     /* 0x3A9C8 dword [0xBB3FC] sar 16 */
    fg_vbcopy(x, x + w - 1, 0x14C, 0x1C6, x, 0x1C6,                 /* 0x3A9E1: rows 332..454, lower-left (x, 454) */
              S32(BGBUF), S32(BACKBUF));
}

/* ---- 0x3AFF4: mirrored sprite ----------------------------------------------------------------------------------- */

void f3AFF4(uint32_t spr, int x, int y)
{
    const uint8_t *pixels = ui_sprite_pixels(spr);                  /* 0x3B048 mov ebx,[eax]: the pixel block */

    /* Port guard (G), as f3AF8C: a record whose block was never captured draws nothing. */
    if (pixels == NULL)
        return;
    fg_move(x, (int)D16(spr + UI_SPR_H) + y - 1);                   /* 0x3B011..0x3B023: word h & 0xFFFF, + y, dec */
    fg_flpimage(pixels,                                             /* 0x3B04B */
                (int)D16(spr + UI_SPR_W),                           /* 0x3B03B word [spr+4] & 0xFFFF */
                (int)D16(spr + UI_SPR_H));                          /* 0x3B02E word [spr+6] & 0xFFFF */
}

/* ---- 0x783B8: player button labels ------------------------------------------------------------------------------ */

void f783B8(uint32_t dlg, int base)
{
    int32_t p, n;                                                   /* [ebp-0x4], ebx / edx */
    uint32_t rec;

    for (p = 0; p < S32(UI_PLAYERS_A); p++) {                       /* 0x783DC cmp [0xF40EC]; jge */
        n = (int32_t)S16(0xF06B0u + 2u * (uint32_t)p);              /* 0x783EA dword [2p + 0xF06AE] sar 16 */
        rec = dlg + 20u * (uint32_t)(base + p);                     /* 0x783F4..0x78409: dlg + 20(base + p) */
        setD32(rec + 0x0A, D32(0xC768Cu + 4u * (uint32_t)n));       /* 0x78412: record +0x08, the label */
        setD8(rec + 0x0E, 0);                                       /* 0x7842A: record +0x0C, the label colour */
        n = (int32_t)S16(0xF06B0u + 2u * (uint32_t)p);              /* 0x78431 dword [2p + 0xF06AE] sar 16 */
        setD8((uint32_t)(0xF06C0 + n), (uint8_t)p);                 /* 0x7843E mov [edx + 0xF06C0], al */
    }
}

/* ---- 0x7853C: append a dialog's records to 0xF596C -------------------------------------------------------------- */

void f7853C(uint32_t dlg)
{
    int32_t first, end, i;                                          /* [ebp-0x8], [ebp-0x4], [ebp-0xC] */

    first = (int32_t)S16(PANEL_DLG);                                /* 0x78552 dword [0xF596A] sar 16 */
    end = first + (int32_t)S16(dlg);                                /* 0x78560 movsx word [dlg]; 0x78566 */
    for (i = first; i < end; i++)                                   /* 0x78579 cmp; jge */
        memmove(DP(PANEL_DLG + DLG_REC(i)),                         /* 0x785A7 lea edi, [20i + 0xF596E] */
                DP(dlg + DLG_REC(i - first)), BTN_SIZE);            /* 0x78593..0x785A4; 0x785AE 5 x movsd */
    setD16(PANEL_DLG, (uint16_t)(D16(PANEL_DLG) + D16(dlg)));      /* 0x785B8..0x785C4 16-bit add */
}

/* ---- 0x78664: remove a dialog's records from 0xF596C ------------------------------------------------------------ */

void f78664(uint32_t dlg)
{
    int32_t found, i, from, rest;                                   /* [ebp-0xC], [ebp-0x10], [ebp-0x8], [ebp-0x4] */

    found = -1;                                                     /* 0x7867C */
    for (i = 0; (int32_t)S16(PANEL_DLG) > i; i++)                   /* 0x78690 dword [0xF596A] sar 16; cmp; jle */
        if (D16(PANEL_DLG + DLG_REC(i) + BTN_ID) == D16(dlg + DLG_REC(0) + BTN_ID)) {  /* 0x786A9 [20i + 0xF597C]; 0x786B4 [dlg + 0x10]; cmp dx,bx */
            found = i;                                              /* 0x786C0, then out of the loop */
            break;
        }
    if (found == -1)                                                /* 0x786C3 */
        return;
    from = found + (int32_t)S16(dlg);                               /* 0x786D0 movsx word [dlg]; 0x786D6 */
    rest = (int32_t)S16(PANEL_DLG) - from;                          /* 0x786DB..0x786E6 */
    for (i = 0; i < rest; i++)                                      /* 0x786F8 cmp; jge */
        memmove(DP(PANEL_DLG + DLG_REC(found + i)),                 /* 0x78727 lea edi, [20(found + i) + 0xF596E] */
                DP(PANEL_DLG + DLG_REC(from + i)), BTN_SIZE);       /* 0x78721 lea esi, [20(from + i) + 0xF596E]; 5 x movsd */
    setD16(PANEL_DLG, (uint16_t)(D16(PANEL_DLG) - D16(dlg)));      /* 0x78738..0x78744 16-bit sub */
}
