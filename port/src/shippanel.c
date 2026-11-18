/* The ship panel of AC_MAIN.EXE, half of module PANEL (docs/MAP_PLAN.md): 0x5D14C (the ship panel of nation n at
   territory index i), 0x5DDF4 (its two coast labels), 0x426D0 (refresh one territory after a movement action) and
   0x572CC (enable the count buttons 0xC4..0xC8). Literal translations; the address of each instruction a line comes
   from is in its comment. D = read in the code, G = inference. Prototypes: shippanel.h. The mirrored sprite 0x3AFF4 is
   in the other half of PANEL (panel.h).

   Watcom runtime -> standard C: 0x8C897 itoa(value, buf, 10) -> snprintf "%d" into the same stack buffer (bounded; the
   original's buffer is at [ebp-0x5c], below the locals at [ebp-0x3c], so at most 32 bytes: 16 here, the value is
   1..256). __CHK dropped.

   Data (offsets in the records, D at the addresses given; meanings G unless said):
     nation n at 0xF14CC + 199n (rec):
       +0x45 + 2k  int16, k 0..3: ship k's territory + 1 in the low byte (word load, xor ah,ah; cwde; dec: 0x5D1B3)
       +0x41 + s   uint8 (byte load, and 0xff: 0x5D46D): ship s's movement left (G)
       +0x4D + s   int8 (dword [rec+s+0x4A] sar 24: 0x5D441): tokens on ship s (< 5 to embark; up to 5 drawn)
     area record r + 1 at 0xE31B8 + 69(r+1) (the code adds n - 1 to the address, so the fields read as below):
       +0x24 + n   int8 (dword [.. + 0xE31DA] sar 24 with n - 1 added, 0x5D48C; dword [.. + 0xE31D9] with n added,
                   0x42706 / 0x57300): nation n's tokens there that may still move (G)
       +0x2D + n   uint8 (byte [.. + 0xE31E6] with n - 1 added, 0x5D433): nation n's selected ship index s there
     0xC803C  uint8[160]  canonical territory (mapdata.h); f3E89C gives the twin (alias) 0x9D/0x9E/0x9F of 0x39/0x0D/0x87
     byte [0xF4102] the phase (5 = MOVEMENT); int32 [0xF40F8] the current nation; int16 [0x109E14] the selected group
     of the panel (0 = territory t, 1 = its twin; read as dword [0x109E12] sar 16, 0x5D3DE); int16 [0xF09E0] (G: the
     selected count, 0x572CC writes 1)
     0xC49B8 / 0xC49CC  int32[5] x / y of the token sprites on one ship (299,313,299,313,327 / 377,377,392,392,384)
     0xC49E0 / 0xC49F4  int32[5] x / y of the token boxes of group 0 (288,296,288,296,304 / 384,384,391,391,387)
     0xC4A08 / 0xC4A1C  int32[5] x / y of the token boxes of group 1 (333,341,333,341,349 / 384,384,391,391,387)
     0xC4A30  int8[14]  box colour by nation (read sign-extended, dword [0xC4A2D + n] sar 24: 0xEA and 0xB3 for
                        nations 1 and 9 reach fg_setcolor negative; Fastgraph keeps the low byte)
   Sprite records (ui.h; game.h's list from 0x11A8C): 0xF518C / 0xF5194 the ship (54 x 28) with / without ships there,
   0xF519C the small ship (27 x 13; drawn plain and mirrored for two groups), 0xF4E3C + 8n the small token (11 x 10).
   Strings (data, D): 0xB5E16 "EMBARK", 0xB5E1D "DISEMBARK", 0xB5E27 "DISEMBARK ALL", 0xB5E35 "NEXT SHIP", 0xB5E3F
   "EMBRK", 0xB5E45 "DSMBRK", 0xB5E4C "DSMBRK-A", 0xB5E55 "NXT SHP", 0xB5E62 "WEST" / 0xB5E5D "EAST", 0xB5E6D "SOUTH" /
   0xB5E67 "NORTH", 0xB5E79 "NORTH" / 0xB5E73 "SOUTH".

   1995 bugs: none fixed here (no g_original branches). Oddities, kept (D):
   - With fewer than two groups, the EMBARK / DISEMBARK enables read the selected ship from the area record of the index
     i the caller passed (0x5D420, 0x5D4C8 [ebp-0x18]), while the panel draws the ship of t or of its twin (0x5D9BB
     [ebp-0x1c], 0x5DA77 [ebp-0x20]). They differ only when i is one half of a two-coast territory and the ships are
     on the other half. Review (D, dispatcher case 0x2B at 0x40F80): EMBARK acts on the ship selected in the record of
     i' = f3E8FC([0x109E10]) (0x40FA7, byte +0x2D+n at 0x40FC4, its load +0x4D incremented at 0x40FDA) and redraws
     the panel with f426D0(i') (0x40FE4), so after an action the enables test the same record as the action; f3E8FC
     maps 0x39/0x0D/0x87 to the twin when int16 [0x109E14] == 1 and returns a twin index 0x9D..0x9F unchanged. A
     mismatch remains only against the drawn ship (i' = twin while the ships are on t), and for callers that pass i
     unmapped (G; the other callers are in MOVE, not checked). Not fixed.
   - With two groups, EMBARK of the second group (0x2F) tests the land tokens of t, not of the twin (0x5D858): right,
     as tokens stand on the canonical territory only (map.c: tokens are drawn for t < 157).
   - The twin's ships are counted even for n == 0 (only the first loop tests n, 0x5D194).
   - The first group's token boxes set the colour once more before their loop (0x5DBAD), the second group's do not:
     no visible difference (each box sets it again). */

#include <stdio.h>
#include "state.h"
#include "game.h"
#include "fg.h"
#include "map.h"
#include "mapdata.h"
#include "panel.h"
#include "shippanel.h"

#define SP_DLG 0xF596Cu   /* the panel dialog (D: every call here passes the immediate 0xF596C) */

/* nation rec's ship byte: territory + 1 of ship k (word [rec + 2k + 0x45], low byte, 0x5D1B3..0x5D1BA), minus 1 */
static int32_t ship_terr(uint32_t rec, int32_t k)
{
    return (int32_t)D8(rec + 2u * (uint32_t)k + 0x45u) - 1;
}

/* area record r + 1, uint8 +0x2D + n: the selected ship (byte [69(r+1) + (n-1) + 0xE31E6], and 0xff) */
static int32_t area_sel(int32_t r, int32_t n)
{
    return (int32_t)D8(GAME_AREA + 69u * (uint32_t)(r + 1) + (uint32_t)(n - 1) + 0x2Eu);
}

/* area record r + 1, int8 +0x24 + n: tokens that may still move (dword [69(r+1) + (n-1) + 0xE31DA] sar 24) */
static int32_t area_tok(int32_t r, int32_t n)
{
    return (int32_t)S8(GAME_AREA + 69u * (uint32_t)(r + 1) + (uint32_t)(n - 1) + 0x25u);
}

/* nation rec, int8 +0x4D + s (dword [rec + s + 0x4A] sar 24) */
static int32_t ship_load(uint32_t rec, int32_t s)
{
    return (int32_t)S8(rec + (uint32_t)s + 0x4Du);
}

/* nation rec, uint8 +0x41 + s (byte [rec + s + 0x41], and 0xff) */
static int32_t ship_moves(uint32_t rec, int32_t s)
{
    return (int32_t)D8(rec + (uint32_t)s + 0x41u);
}

/* ---- 0x5D14C: the ship panel ----------------------------------------------------------------------------------- */

void f5D14C(int n, int i)
{
    uint32_t rec;                                               /* [ebp-0x14] */
    int32_t t;                                                  /* [ebp-0x1c] */
    int32_t twin;                                               /* [ebp-0x20] */
    int32_t cnt[2];                                             /* [ebp-0x3c], [ebp-0x38] */
    int32_t groups;                                             /* [ebp-0x28] */
    int32_t k;                                                  /* [ebp-0x2c] */
    int32_t tokens;                                             /* [ebp-0x30] */
    int32_t sel;
    char buf[16];                                               /* [ebp-0x5c] */

    rec = GAME_NATION + 199u * (uint32_t)n;                     /* 0x5D166 imul 0xC7; add 0xF14CC */
    t = (int32_t)D8(0xC803Cu + (uint32_t)i);                    /* 0x5D183 */
    cnt[0] = 0;                                                 /* 0x5D18E */
    cnt[1] = 0;                                                 /* 0x5D191 */
    if (n != 0)                                                 /* 0x5D194 */
        for (k = 0; k < 4; k++)                                 /* 0x5D1A5 */
            if (ship_terr(rec, k) == t)                         /* 0x5D1BE */
                cnt[0]++;                                       /* 0x5D1C5 */
    twin = -1;                                                  /* 0x5D1CA */
    if (f3E89C(t) != t) {                                       /* 0x5D1D4 */
        twin = f3E89C(t);                                       /* 0x5D1E1 */
        for (k = 0; k < 4; k++)                                 /* 0x5D1F6 */
            if (ship_terr(rec, k) == twin)                      /* 0x5D20F */
                cnt[1]++;                                       /* 0x5D216 */
    }
    groups = 0;                                                 /* 0x5D21D */
    for (k = 0; k < 2; k++)                                     /* 0x5D22B */
        if (cnt[k] != 0)                                        /* 0x5D238 */
            groups++;                                           /* 0x5D23F */

    setD16(0x109E14u, 0);                                       /* 0x5D24C */
    if (D8(0xF4102u) == 5) {                                    /* 0x5D253 cmp ah,5 */
        if (groups < 2) {                                       /* 0x5D25F */
            /* one group (or none): long labels on 0x2B..0x2E, the second group's buttons hidden */
            if (groups == 1)                                    /* 0x5D26B */
                setD16(0x109E14u, (uint16_t)(cnt[0] == 0));     /* 0x5D270 sete; 0x5D27C */
            f3BD80(SP_DLG, 0x2B, 0x6E);                         /* 0x5D291 */
            f3BD80(SP_DLG, 0x2C, 0x6E);                         /* 0x5D2A5 */
            f3BD80(SP_DLG, 0x2D, 0x6E);                         /* 0x5D2B9 */
            f3BD80(SP_DLG, 0x2E, 0x6E);                         /* 0x5D2CD */
            f3BB48(SP_DLG, 0x2D, 0x167);                        /* 0x5D2E1 */
            f3BB48(SP_DLG, 0x2E, 0x167);                        /* 0x5D2F5 */
            f3C284(SP_DLG, 0x2B, 0xB5E16u);                     /* 0x5D309 "EMBARK" */
            f3C284(SP_DLG, 0x2C, 0xB5E1Du);                     /* 0x5D31D "DISEMBARK" */
            f3C284(SP_DLG, 0x2D, 0xB5E27u);                     /* 0x5D331 "DISEMBARK ALL" */
            f3C284(SP_DLG, 0x2E, 0xB5E35u);                     /* 0x5D345 "NEXT SHIP" */
            f3C0D8(SP_DLG, 0x2F, 2);                            /* 0x5D359 */
            f3C0D8(SP_DLG, 0x30, 2);                            /* 0x5D36D */
            f3C0D8(SP_DLG, 0x31, 2);                            /* 0x5D381 */
            f3C0D8(SP_DLG, 0x32, 2);                            /* 0x5D395 */
            f3BFBC(SP_DLG, 0x2F, 0);                            /* 0x5D3A6 */
            f3BFBC(SP_DLG, 0x30, 0);                            /* 0x5D3B7 */
            f3BFBC(SP_DLG, 0x31, 0);                            /* 0x5D3C8 */
            f3BFBC(SP_DLG, 0x32, 0);                            /* 0x5D3D9 */

            sel = (int32_t)S16(0x109E14u);                      /* 0x5D3DE dword [0x109E12] sar 16 (0 or 1, set above) */
            if (cnt[sel] > 1)                                   /* 0x5D3E6 */
                f3C314(SP_DLG, 0x2E);                           /* 0x5D3F7 NEXT SHIP */
            else
                f3C3D0(SP_DLG, 0x2E);                           /* 0x5D408 */

            sel = (int32_t)S16(0x109E14u);                      /* 0x5D40D */
            if (cnt[sel] != 0                                   /* 0x5D415 */
                && ship_load(rec, area_sel(i, n)) < 5           /* 0x5D420..0x5D447 (record of i: see the header) */
                && ship_moves(rec, area_sel(i, n)) > 0          /* 0x5D44C..0x5D475 */
                && area_tok(t, n) > 0)                          /* 0x5D479..0x5D495 */
                f3C314(SP_DLG, 0x2B);                           /* 0x5D4A3 EMBARK */
            else
                f3C3D0(SP_DLG, 0x2B);                           /* 0x5D4B4 */

            sel = (int32_t)S16(0x109E14u);                      /* 0x5D4B9 */
            if (cnt[sel] != 0                                   /* 0x5D4C1 */
                && ship_load(rec, area_sel(i, n)) > 0) {        /* 0x5D4C8..0x5D4EF */
                f3C314(SP_DLG, 0x2C);                           /* 0x5D4FD DISEMBARK */
                f3C314(SP_DLG, 0x2D);                           /* 0x5D50C DISEMBARK ALL */
            } else {
                f3C3D0(SP_DLG, 0x2C);                           /* 0x5D520 */
                f3C3D0(SP_DLG, 0x2D);                           /* 0x5D52A -> 0x5D902 */
            }
        } else {
            /* two groups: short labels, 0x2B..0x2E for t, 0x2F..0x32 for the twin */
            f3BD80(SP_DLG, 0x2B, 0x37);                         /* 0x5D53E */
            f3BD80(SP_DLG, 0x2C, 0x37);                         /* 0x5D552 */
            f3BD80(SP_DLG, 0x2D, 0x37);                         /* 0x5D566 */
            f3BD80(SP_DLG, 0x2E, 0x37);                         /* 0x5D57A */
            f3BB48(SP_DLG, 0x2D, 0xDE);                         /* 0x5D58E */
            f3BB48(SP_DLG, 0x2E, 0xDE);                         /* 0x5D5A2 */
            f3C284(SP_DLG, 0x2B, 0xB5E3Fu);                     /* 0x5D5B6 "EMBRK" */
            f3C284(SP_DLG, 0x2C, 0xB5E45u);                     /* 0x5D5CA "DSMBRK" */
            f3C284(SP_DLG, 0x2D, 0xB5E4Cu);                     /* 0x5D5DE "DSMBRK-A" */
            f3C284(SP_DLG, 0x2E, 0xB5E55u);                     /* 0x5D5F2 "NXT SHP" */
            f3C0D8(SP_DLG, 0x2F, 1);                            /* 0x5D606 */
            f3C0D8(SP_DLG, 0x30, 1);                            /* 0x5D61A */
            f3C0D8(SP_DLG, 0x31, 1);                            /* 0x5D62E */
            f3C0D8(SP_DLG, 0x32, 1);                            /* 0x5D642 */
            f3BFBC(SP_DLG, 0x2F, 1);                            /* 0x5D656 */
            f3BFBC(SP_DLG, 0x30, 1);                            /* 0x5D66A */
            f3BFBC(SP_DLG, 0x31, 1);                            /* 0x5D67E */
            f3BFBC(SP_DLG, 0x32, 1);                            /* 0x5D692 */

            if (cnt[0] > 1)                                     /* 0x5D697 */
                f3C314(SP_DLG, 0x2E);                           /* 0x5D6A7 */
            else
                f3C3D0(SP_DLG, 0x2E);                           /* 0x5D6B8 */

            if (cnt[0] != 0                                     /* 0x5D6BD */
                && ship_load(rec, area_sel(t, n)) < 5           /* 0x5D6C7..0x5D6EE */
                && ship_moves(rec, area_sel(t, n)) > 0          /* 0x5D6F3..0x5D71C */
                && area_tok(t, n) > 0)                          /* 0x5D720..0x5D73C */
                f3C314(SP_DLG, 0x2B);                           /* 0x5D74A */
            else
                f3C3D0(SP_DLG, 0x2B);                           /* 0x5D75B */

            if (cnt[0] != 0                                     /* 0x5D760 */
                && ship_load(rec, area_sel(t, n)) > 0) {        /* 0x5D766..0x5D78D */
                f3C314(SP_DLG, 0x2C);                           /* 0x5D79B */
                f3C314(SP_DLG, 0x2D);                           /* 0x5D7AA */
            } else {
                f3C3D0(SP_DLG, 0x2C);                           /* 0x5D7BB */
                f3C3D0(SP_DLG, 0x2D);                           /* 0x5D7CA */
            }

            if (cnt[1] > 1)                                     /* 0x5D7CF */
                f3C314(SP_DLG, 0x32);                           /* 0x5D7DF */
            else
                f3C3D0(SP_DLG, 0x32);                           /* 0x5D7F0 */

            if (cnt[1] != 0                                     /* 0x5D7F5 */
                && ship_load(rec, area_sel(twin, n)) < 5        /* 0x5D7FF..0x5D826 */
                && ship_moves(rec, area_sel(twin, n)) > 0       /* 0x5D82B..0x5D854 */
                && area_tok(t, n) > 0)                          /* 0x5D858..0x5D874 (t, not the twin) */
                f3C314(SP_DLG, 0x2F);                           /* 0x5D882 */
            else
                f3C3D0(SP_DLG, 0x2F);                           /* 0x5D893 */

            if (cnt[1] != 0                                     /* 0x5D898 */
                && ship_load(rec, area_sel(twin, n)) > 0) {     /* 0x5D89E..0x5D8C5 */
                f3C314(SP_DLG, 0x30);                           /* 0x5D8D3 */
                f3C314(SP_DLG, 0x31);                           /* 0x5D8E2 */
            } else {
                f3C3D0(SP_DLG, 0x30);                           /* 0x5D8F3 */
                f3C3D0(SP_DLG, 0x31);                           /* 0x5D902 */
            }
        }
    }

    /* every phase: the panel rectangle in the back buffer */
    fg_vbopen(S32(0xF52C8u));                                   /* 0x5D90E */
    fg_vbcopy(0x116, 0x165, 0x172, 0x19B, 0x116, 0x19B,         /* 0x5D942: [0xF52CC] -> [0xF52C8] */
              S32(0xF52CCu), S32(0xF52C8u));
    if (groups < 2) {                                           /* 0x5D94A */
        if (groups != 0)                                        /* 0x5D953 */
            f3AF8C(0xF518Cu, 0x122, 0x176);                     /* 0x5D977 */
        else
            f3AF8C(0xF5194u, 0x122, 0x176);                     /* 0x5D977 */
    } else {
        f3AF8C(0xF519Cu, 0x11B, 0x17F);                         /* 0x5D98D */
        f3AFF4(0xF519Cu, 0x146, 0x17F);                         /* 0x5D9A1 mirrored */
    }

    if (groups < 2) {                                           /* 0x5D9A6 */
        if (cnt[0] != 0) {                                      /* 0x5D9B3 */
            snprintf(buf, sizeof buf, "%d", (int)(area_sel(t, n) + 1));   /* 0x5D9E2 itoa(.., 10) */
            fg_setcolor(5);                                     /* 0x5D9E9 */
            f3A044(0x15B, 0x172, buf);                          /* 0x5D9FE (current font) */
            tokens = ship_load(rec, area_sel(t, n));            /* 0x5DA03..0x5DA2A */
            for (k = 0; k < tokens; k++)                        /* 0x5DA3A */
                f3AF8C(0xF4E3Cu + 8u * (uint32_t)n,             /* 0x5DA65 */
                       S32(0xC49B8u + 4u * (uint32_t)k),        /* 0x5DA53 */
                       S32(0xC49CCu + 4u * (uint32_t)k));       /* 0x5DA49 */
        } else if (cnt[1] != 0) {                               /* 0x5DA6F */
            snprintf(buf, sizeof buf, "%d", (int)(area_sel(twin, n) + 1));    /* 0x5DA9E */
            fg_setcolor(5);                                     /* 0x5DAA5 */
            f3A044(0x15B, 0x172, buf);                          /* 0x5DABA */
            tokens = ship_load(rec, area_sel(twin, n));         /* 0x5DABF..0x5DAE6 */
            for (k = 0; k < tokens; k++)                        /* 0x5DAF6 */
                f3AF8C(0xF4E3Cu + 8u * (uint32_t)n,             /* 0x5DB21 */
                       S32(0xC49B8u + 4u * (uint32_t)k),        /* 0x5DB0F */
                       S32(0xC49CCu + 4u * (uint32_t)k));       /* 0x5DB05 */
        }
        fg_vbclose();                                           /* 0x5DDE5 */
        return;
    }

    /* two groups: numbers in font 0, the tokens as boxes in the nation's colour, the coast names */
    f39F90(0);                                                  /* 0x5DB2A */
    snprintf(buf, sizeof buf, "%d", (int)(area_sel(t, n) + 1)); /* 0x5DB56 */
    fg_setcolor(5);                                             /* 0x5DB5D */
    f3A044(0x135, 0x17E, buf);                                  /* 0x5DB72 */
    tokens = ship_load(rec, area_sel(t, n));                    /* 0x5DB77..0x5DB9E */
    fg_setcolor((int)S8(0xC4A30u + (uint32_t)n));               /* 0x5DBB0 dword [n + 0xC4A2D] sar 24 */
    for (k = 0; k < tokens; k++) {                              /* 0x5DBC3 */
        fg_setcolor((int)S8(0xC4A30u + (uint32_t)n));           /* 0x5DBDC */
        fg_rect(S32(0xC49E0u + 4u * (uint32_t)k),               /* 0x5DC16 */
                S32(0xC49E0u + 4u * (uint32_t)k) + 4,
                S32(0xC49F4u + 4u * (uint32_t)k),
                S32(0xC49F4u + 4u * (uint32_t)k) + 3);
        fg_setcolor(5);                                         /* 0x5DC20 */
        fg_box(S32(0xC49E0u + 4u * (uint32_t)k),                /* 0x5DC5A */
               S32(0xC49E0u + 4u * (uint32_t)k) + 4,
               S32(0xC49F4u + 4u * (uint32_t)k),
               S32(0xC49F4u + 4u * (uint32_t)k) + 3);
    }
    snprintf(buf, sizeof buf, "%d", (int)(area_sel(twin, n) + 1));    /* 0x5DC8E */
    fg_setcolor(5);                                             /* 0x5DC95 */
    f3A044(0x15F, 0x17E, buf);                                  /* 0x5DCAA */
    tokens = ship_load(rec, area_sel(twin, n));                 /* 0x5DCAF..0x5DCD6 */
    for (k = 0; k < tokens; k++) {                              /* 0x5DCE6 */
        fg_setcolor((int)S8(0xC4A30u + (uint32_t)n));           /* 0x5DCFF */
        fg_rect(S32(0xC4A08u + 4u * (uint32_t)k),               /* 0x5DD39 */
                S32(0xC4A08u + 4u * (uint32_t)k) + 4,
                S32(0xC4A1Cu + 4u * (uint32_t)k),
                S32(0xC4A1Cu + 4u * (uint32_t)k) + 3);
        fg_setcolor(5);                                         /* 0x5DD43 */
        fg_box(S32(0xC4A08u + 4u * (uint32_t)k),                /* 0x5DD7D */
               S32(0xC4A08u + 4u * (uint32_t)k) + 4,
               S32(0xC4A1Cu + 4u * (uint32_t)k),
               S32(0xC4A1Cu + 4u * (uint32_t)k) + 3);
    }
    f39F90(1);                                                  /* 0x5DD8F */
    if (t == 0x39)                                              /* 0x5DD94 */
        f5DDF4(DSTR(0xB5E62u), DSTR(0xB5E5Du));                 /* 0x5DDE0 "WEST", "EAST" */
    else if (t == 0x87)                                         /* 0x5DDA9 */
        f5DDF4(DSTR(0xB5E6Du), DSTR(0xB5E67u));                 /* 0x5DDBB "SOUTH", "NORTH" */
    else if (t == 0x0D)                                         /* 0x5DDD1 */
        f5DDF4(DSTR(0xB5E79u), DSTR(0xB5E73u));                 /* 0x5DDE0 "NORTH", "SOUTH" */
    fg_vbclose();                                               /* 0x5DDC0 / 0x5DDE5 */
}

/* ---- 0x5DDF4: the two coast labels ------------------------------------------------------------------------------ */

void f5DDF4(const char *left, const char *right)
{
    f39F90(0);                                                  /* 0x5DE0E */
    fg_setcolor(5);                                             /* 0x5DE15 */
    f3A0B0(0x118, 0x18E, 0x21, left);                           /* 0x5DE2F */
    f3A0B0(0x143, 0x18E, 0x21, right);                          /* 0x5DE46 */
    f39F90(1);                                                  /* 0x5DE50 */
}

/* ---- 0x426D0: refresh one territory after a movement action ---------------------------------------------------- */

void f426D0(int i)
{
    int32_t t = (int32_t)D8(0xC803Cu + (uint32_t)i);            /* 0x426E7 */
    int32_t n = S32(0xF40F8u);                                  /* 0x42701 */
    int color;

    /* 0x42706 dword [69(t+1) + n + 0xE31D9] sar 24: area record t + 1, int8 +0x24 + n */
    if ((int32_t)S8(GAME_AREA + 69u * (uint32_t)(t + 1) + (uint32_t)n + 0x24u) == 0)   /* 0x42710 */
        color = 5;                                              /* 0x42714 */
    else
        color = 0x0F;                                           /* 0x4271B */
    f3F698((int)D8(0xC803Cu + (uint32_t)i), color);             /* 0x4272E */
    f5D14C((int)S32(0xF40F8u), i);                              /* 0x4273B */
    f3D52C(0, 0);                                               /* 0x42744 */
    f3CBBC();                                                   /* 0x42749 */
}

/* ---- 0x572CC: the count buttons 0xC4..0xC8 ---------------------------------------------------------------------- */

void f572CC(int t)
{
    setD16(0xF09E0u, 1);                                        /* 0x572E2 */
    /* each: dword [69(t+1) + [0xF40F8] + 0xE31D9] sar 24 = area record t + 1, int8 +0x24 + n */
    f3BFBC(SP_DLG, 0xC4, area_tok(t, S32(0xF40F8u)) >= 1 ? 1 : 0);   /* 0x5730A jl; 0x5732C */
    f3BFBC(SP_DLG, 0xC5, area_tok(t, S32(0xF40F8u)) >= 2 ? 1 : 0);   /* 0x57350; 0x57372 */
    f3BFBC(SP_DLG, 0xC6, area_tok(t, S32(0xF40F8u)) >= 3 ? 1 : 0);   /* 0x57396; 0x573B8 */
    f3BFBC(SP_DLG, 0xC7, area_tok(t, S32(0xF40F8u)) >= 4 ? 1 : 0);   /* 0x573DC; 0x573FE */
    f3BFBC(SP_DLG, 0xC8, area_tok(t, S32(0xF40F8u)) >= 5 ? 1 : 0);   /* 0x57422; 0x57444 */
    fg_vbopen(S32(0xF52C8u));                                   /* 0x57450 */
    f3B7CC(SP_DLG, 0xC4);                                       /* 0x57462 pressed */
    f3B870(SP_DLG, 0xC5);                                       /* 0x57471 up */
    f3B870(SP_DLG, 0xC6);                                       /* 0x57480 */
    f3B870(SP_DLG, 0xC7);                                       /* 0x5748F */
    f3B870(SP_DLG, 0xC8);                                       /* 0x5749E */
    fg_vbclose();                                               /* 0x574A3 */
}
