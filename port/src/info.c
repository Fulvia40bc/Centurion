/* Module INFO of docs/MAP_PLAN.md: the information screens of AC_MAIN.EXE behind the panel buttons CENSUS (dispatcher
   id 4), TOOLS (6), GOODS (7) and the trade screens' REVIEW (0x103):
     0x13E50 the census, 0x19C10 Civilization Tools, 0x1A124 the advance pop-up, 0x76A9C Goods Held, 0x76798 "Select
     Nation To Show", 0x71F2C / 0x71FB0 a hand face down / up, 0x76F14 cards held, 0x194E4 the Mining bonus list,
     0x195D8 the value of a hand, 0x763FC / 0x764A8 Trading Wrapup, and the dispatcher cases 4, 6, 7, 0x103 (dcase_*).
   Literal translations; the address of each instruction a line comes from is in its comment. D = read in the code,
   G = inference. Prototypes: info.h.

   Watcom runtime -> standard C:
     0x8C897 itoa(value, buf, 10) -> snprintf "%d" into a buffer of the original's size (LIBRARY_CALLS.md: radix 10
             writes '-' and the magnitude, as "%d")
     0x7DB3D sprintf(buf, fmt) with a data format without conversions ("Goods Held", "Civilization Tools") ->
             snprintf(buf, size, "%s", fmt) (the same bytes; the format is not passed as a format, -Wformat-security)
     0x7D5D0 memset -> memset; the inline strcpy (0x19E63) -> a bounded copy; repne scasb (strlen) -> strlen
   __CHK dropped. Library calls through fg.h in the original argument order.

   fg_makegif / fg_showgif "junk" and "junk2" (port choice, MAP_PLAN.md risk 7): the original saves a screen rectangle
   to a GIF file in the current folder with fg_makegif (0x8CE50, not in fg.c) and reads it back with fg_showgif(name, 3)
   (at the cursor, palette kept) at the same top-left. The files are only written to be read back. The port keeps the
   pixels in the shared slots of junk.c instead: the same rectangle of the active surface is read with fg_getpixel and
   written back with fg_point at the cursor, the colour put back afterwards (fg_showgif leaves the colour and the
   cursor alone; so does junk.c). The screen ends up identical: the GIF is lossless and keeps the palette. Differences,
   all G and outside these call sites' use: fg_getpixel / fg_point clip to the clip region (fg_showgif clips only
   right and bottom to the surface); every call site here runs with the full-surface clip (map.c restores it after
   drawing the map). No junk / junk2 file is left on disk (the original leaves both in the game folder).

   Data (D, read in the code at the addresses given):
     nation n at 0xF14CC + 199n (game.h): +0x01..+0x12 int8 trade cards per commodity, +0x13 int8 calamity cards held,
       +0x14.. int8 their ids, +0x24..+0x3B int8 advances (index 0..23; +0x29 = advance 5, Mining), +0x35 int8 (G: a
       marker of the census, boxed), +0x98 int8 tokens on the map, +0x9A int8 0 = not in play, 3 = human, 2 = computer
     0xC768C uint32[] -> nation names; 0xC7DC4 uint32[24] -> advance names, mixed case; 0xC7E24 uint32[24] -> upper
       case; 0xC7E9C uint32[24] -> description texts (0x8F / 0x8E colour codes); 0xBD480 int16[24] advance costs
     0xF6E12 int16[10][5] (read as dword [10n + 2j + 0xF6E10] sar 16) and 0xF6E76 int16[10] its counts (dword
       [2n + 0xF6E74] sar 16): per nation a list of advance indices (G: bought this turn). An owned advance in the list
       is drawn in colour 15, an owned one not in it in 14 (0x19EED; INVENTORY_2EFA0.json has the two colours swapped)
     0xBDD3C uint8[6] {0, 4, 12, 11, 16, 18}: the commodities Mining raises (G: Iron, Bronze, Silver, Gems, Gold, after
       the advance's text "Iron, Silver, Bronze, Gems or Gold")
     Mining list: 0xF68B0 int32 (cleared), 0xF68B4 int32 count, 0xF68B8 uint8[6] commodity, 0xF68C0 uint8[] its index
       in 0xBDD3C, 0xF6754 uint8 flag; 0xF6740, 0xF6744 int32 (G: both 0 when the bonus may apply)
     0x11254C int32 big card step, 0x112550 int32 small card step (written by f71F2C / f71FB0)
     trades (0x110F74): uint8 count, A's cards int8[32][5] at 0x110F75, B's int8[32][5] at 0x111015, A's nation int8[32]
       at 0x1110B5, B's at 0x1110D5; int16 [0x112544] the scroll row (shared with the trade screens, G)
     sprite records (ui.h, 8 bytes; +0 a ui_sprite_put handle): 0xF4ACC + 8n tokens (w at 0xF4AD0 + 8n, h at 0xF4AD2 +
       8n, read as words), 0xF4C1C + 8i cardtiny i (1..33), 0xF4D14 cardtiny 31 (the card back, G), 0xF5214 c_invld
     dialogs: 0xBB800 (CONTINUE, 265,450 110x19 in obj2), 0xBBBEC (CONTINUE without label), 0xBC3C4 (8 records, ids
       0x12..0x19, dispatcher codes -0x36..-0x3D), 0xBD940 (rows 0xB2..0xB9 at (61, 36 + 49k) 540x48, codes -0x50..
       -0x57; CONTINUE 265,450), 0xCA5A8 (only CONTINUE, 265,430)
     [0xF8E78] the current dialog, [0xF52C8] back buffer, [0xF52CC] background buffer, int8 [0xF52D0] background in
       it, int8 [0xEE168] the panel's background number, int8 [0xE1620] humans, [0xE1622] (PBEM: the viewer), [0xE1624]
       flags (0x80 PBEM), int32 [0xF40F8] the current nation, int16 [0xF40C2] (all hands face up), int16 [0xF40BA]
       continuous test, uint8 [0xF4102] the phase, int8 [0xF40FE] (PBEM data invalid), 0xE1628 / 0xE162C mouse x, y

   Heap pointers in data fields: none written. The sprite records' +0 handles are read through f3AF8C; [0xF52C8] /
   [0xF52CC] are Fastgraph buffer handles.

   1995 bugs (fixed unless g_original):
     - 0x1A0B9: a click in the bottom 4 pixels of a row of Civilization Tools (the button is 48 high, the grid's four
       name rows 44) gives grid row 4: the pop-up shows the next column's first advance, and in the last column index
       24, past the tables: 0xC7E9C[24] is the dword 0x0B86ED00, not a text (D: obj2), so the original reads a wild
       pointer (G: a crash or garbage). The same happens when the press on a row is dragged off it before the release:
       f775F0 (in f3C5D4) leaves the release position in 0xE1628 / 0xE162C, so ri is -38..40 and c 0..6 (D). Fixed:
       ri clamped to 0..3 and c to 0..5 (row 4 counts as row 3). Grades D (arithmetic and tables), G (effect).
     - 0x3FC75: REVIEW saves rows 0..0x1DE of the screen (fg_makegif(0, 0x27F, 0, 0x1DE)) and later shows them over the
       background buffer, so the bottom row 479 keeps tradwrap.gif's row instead of the trade screen's. Fixed: 0x1DF.
       D (the arguments), G (visible only where the two pictures' last rows differ).
   Bugs and oddities kept (D unless marked):
     - 0x13ED8 / 0x14106: the census histograms are indexed with tokens - 1. A nation in play with 0 tokens on the map
       counts in the byte below the first histogram ([ebp-0x69], never cleared: G stack garbage in the original) and the
       byte below the second (ebp-0xA1, below esp: G clobbered by later calls), and is drawn in cell -1: x 0x36 - 49 =
       5, y 0x42, left of the board. The port gives both bytes a scratch slot that starts at 0 (G: what the original
       does when the stack bytes happen to be 0); the original's garbage cannot be reproduced, so there is no
       g_original branch. The drawing position is kept.
     - 0x13E50 leaves int8 [0xF52D0] = 5 after drawing the tokens into the background buffer, so a census shown again
       before another background is loaded starts from the old drawing (same state: the same pixels; G harmless).
     - 0x19C82: with PBEM in phase 0x12 / 0x13 the "This data is invalid..." line is printed twice (two copies of the
       string) on the screen, which the background buffer then covers (f3CC68): it never shows. PBEM is dropped
       (Erik's ruling); translated as is.
     - 0x763FC handles -0x2C / -0x2B (NEXT / PREV PAGE, ids 0x86 / 0x85, which the trade screens' dialogs have:
       0xC9A72, 0xC9A86, 0xCA25E, 0xCA272), but its dialog 0xCA5A8 holds only CONTINUE: Trading Wrapup shows the first
       6 trades only (MAP_PLAN.md risk 11). A fix needs new buttons in a dialog that has no room for them: not done,
       for Erik's ruling.
     - 0x767F0: the box width is capped by "W - 20 > 350 -> W = 350" (a box of 371 shrinks to 350, 370 stays); the title
       is fixed and W is 200, so it never applies.
     - The REVIEW case leaves [0xF8E78] = 0xCA5A8 (f3B05C in f764A8) when it returns into the trade screen's loop. */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "fg.h"
#include "game.h"
#include "info.h"
#include "junk.h"
#include "state.h"
#include "ui.h"

#define BACKBUF   0xF52C8u   /* int32 virtual-buffer handle of the back buffer */
#define BGBUF     0xF52CCu   /* int32 handle of the background-picture buffer */
#define BGPIC     0xF52D0u   /* int8 background now in [0xF52CC] */
#define CURDLG    0xF8E78u   /* uint32 the current dialog */
#define MOUSE_X   0xE1628u
#define MOUSE_Y   0xE162Cu
#define TOKEN_SPR 0xF4ACCu   /* sprite records [14], 8 bytes: +4 w, +6 h */
#define CARD_SPR  0xF4C1Cu   /* cardtiny sprite records [34] */
#define BIG_STEP  0x11254Cu
#define SMALL_STEP 0x112550u

/* nation n's record field */
static uint32_t nat(int32_t n, uint32_t off)
{
    return GAME_NATION + 199u * (uint32_t)n + off;
}

/* ---- 0x76F14: cards held ---------------------------------------------------------------------------------------- */

int f76F14(int n)
{
    int32_t sum = 0;                                                /* [ebp-0x8] */
    int32_t i;                                                      /* [ebp-0x4] */

    for (i = 1; i <= 0x12; i++)                                     /* 0x76F2D, 0x76F43 cmp 0x12; jg */
        sum += S8(nat(n, (uint32_t)i));                             /* 0x76F4C dword [199n + i + 0xF14C9] sar 24 */
    sum += S8(nat(n, 0x13));                                        /* 0x76F5A dword [199n + 0xF14DC] sar 24 */
    return sum;
}

/* ---- 0x71F2C: a hand face down ---------------------------------------------------------------------------------- */

void f71F2C(int x, int y, int w, int n)
{
    int32_t k, i;                                                   /* [ebp-0xc], [ebp-0x10] */

    setD32(BIG_STEP, 0x28);                                         /* 0x71F4A */
    k = f76F14(n);                                                  /* 0x71F57 */
    while (S32(BIG_STEP) * k > w)                                   /* 0x71F5F imul; cmp; jle (signed) */
        setD32(BIG_STEP, (uint32_t)(S32(BIG_STEP) - 2));            /* 0x71F6D */
    for (i = 0; i < k; i++)                                         /* 0x71F76, 0x71F89 jge */
        f3AF8C(0xF4D14u, x + i * S32(BIG_STEP), y);                 /* 0x71FA2 */
}

/* ---- 0x71FB0: a hand face up ------------------------------------------------------------------------------------ */

void f71FB0(int x, int y, int w, int n)
{
    uint32_t rec;                                                   /* [ebp-0xc] */
    int32_t total;                                                  /* [ebp-0x14] */
    int32_t i, k, c;                                                /* [ebp-0x1c], [ebp-0x18] */
    int32_t cx;                                                     /* [ebp-0x10] */

    setD32(BIG_STEP, 0x28);                                         /* 0x71FD8 */
    setD32(SMALL_STEP, 0x0A);                                       /* 0x71FDE */
    rec = nat(n, 0);                                                /* 0x71FF2 */
    do {
        total = 0;                                                  /* 0x71FF7 */
        for (i = 1; i <= 0x12; i++) {                               /* 0x71FFA, 0x72036 cmp 0x12; jg */
            if (D8(rec + (uint32_t)i) == 0)                         /* 0x7203F mov dl,[..]; test dl,dl; je */
                continue;
            if (S8(rec + (uint32_t)i) == 1)                         /* 0x72053 sar 24; cmp 1; jne */
                total += S32(BIG_STEP);                             /* 0x72061, 0x72026 */
            else
                total += (S8(rec + (uint32_t)i) - 1) * S32(SMALL_STEP) + S32(BIG_STEP);   /* 0x7201C..0x72026 */
        }
        total += S8(rec + 0x13) * S32(BIG_STEP);                    /* 0x72068 dword [199n + 0xF14DC] sar 24; imul */
        if (total > w) {                                            /* 0x72080 cmp; jle (signed) */
            setD32(BIG_STEP, (uint32_t)(S32(BIG_STEP) - 2));        /* 0x72085 lea edi,[ebx-2]; 0x7208D */
            if (S32(SMALL_STEP) > 3)                                /* 0x72093 cmp eax,3; jle */
                setD32(SMALL_STEP, (uint32_t)(S32(SMALL_STEP) - 1));   /* 0x7209B */
        }
    } while (total > w);                                            /* 0x720A7 cmp; jg 0x71FF5 */

    cx = x;                                                         /* 0x720B2 */
    for (i = 1; i <= 0x12; i++) {                                   /* 0x720B5, 0x7211A cmp 0x12; jg */
        if (S8(rec + (uint32_t)i) == 1) {                           /* 0x72126 movsx; cmp 1; jne */
            f3AF8C(CARD_SPR + 8u * (uint32_t)i, cx, y);             /* 0x7213F */
            cx += S32(BIG_STEP);                                    /* 0x72144, 0x72111 */
            continue;
        }
        if (S8(rec + (uint32_t)i) == 0)                             /* 0x720C4 movsx; test eax,eax; je */
            continue;
        for (k = 0; S8(rec + (uint32_t)i) > k; k++) {               /* 0x720CD, 0x720E1 cmp eax,k; jle */
            f3AF8C(CARD_SPR + 8u * (uint32_t)i, cx, y);             /* 0x720F7 */
            cx += S32(SMALL_STEP);                                  /* 0x720FC */
        }
        cx += S32(BIG_STEP) - S32(SMALL_STEP);                      /* 0x72106..0x72111 */
    }
    if (D8(rec + 0x13) != 0)                                        /* 0x7214E cmp byte [rec+0x13],0; je */
        for (k = 0; S8(rec + 0x13) > k; k++) {                      /* 0x72156, 0x72161 dword [rec+0x10] sar 24; jle */
            c = S8(rec + 0x14 + (uint32_t)k);                       /* 0x7217B dword [rec + k + 0x11] sar 24 */
            f3AF8C(CARD_SPR + 8u * (uint32_t)c, cx, y);             /* 0x72189 */
            cx += S32(BIG_STEP);                                    /* 0x7218E */
        }
}

/* ---- 0x194E4: the Mining bonus list ----------------------------------------------------------------------------- */

void f194E4(int n)
{
    uint32_t rec = nat(n, 0);                                       /* [ebp-0x14] (0x19509) */
    int32_t i, cnt;                                                 /* [ebp-0x18] */

    if (D8(rec + 0x29) == 0 || S32(0xF6740u) != 0 || S32(0xF6744u) != 0) {   /* 0x1950F je, 0x1951C jne, 0x19525 je */
        memset(DP(0xF68B8u), 0, 6);                                 /* 0x19533 */
        setD32(0xF68B0u, 0);                                        /* 0x1953C */
        setD8(0xF6754u, 0);                                         /* 0x19542 */
        return;
    }
    memset(DP(0xF68B8u), 0, 6);                                     /* 0x1955E */
    setD32(0xF68B4u, 1);                                            /* 0x19565 */
    setD8(0xF6754u, 1);                                             /* 0x1956B */
    for (i = 1; i < 6; i++) {                                       /* 0x19571, 0x1957C cmp 6; jge */
        /* 0x1958C dword [i + 0xBDD39] sar 24: the signed byte 0xBDD3C[i]; 0x19595 mov bh,[199n + that + 0xF14CC] */
        if (D8(rec + (uint32_t)(int32_t)S8(0xBDD3Cu + (uint32_t)i)) == 0)
            continue;
        cnt = S32(0xF68B4u);                                        /* 0x195A0 */
        setD8(0xF68C0u + (uint32_t)cnt, (uint8_t)i);                /* 0x195A9 */
        cnt++;                                                      /* 0x195B2 */
        setD32(0xF68B4u, (uint32_t)cnt);                            /* 0x195B9 */
        setD8(0xF68B7u + (uint32_t)cnt, D8(0xBDD3Cu + (uint32_t)i));   /* 0x195B3, 0x195BF: 0xF68B8[old count] */
    }
    setD32(0xF68B0u, 0);                                            /* 0x195C9 */
}

/* ---- 0x195D8: the value of a hand ------------------------------------------------------------------------------- */

int f195D8(int n)
{
    uint32_t rec = nat(n, 0);                                       /* [ebp-0x4] */
    int32_t total = 0, best = 0;                                    /* [ebp-0x10], [ebp-0xc] */
    int32_t i, j, c;                                                /* [ebp-0x14], [ebp-0x8] */

    if (D8(0xF6754u) == 0) {                                        /* 0x19607 cmp byte [0xF6754],0; jne */
        for (i = 1; i <= 0x12; i++) {                               /* 0x19610, 0x1961F cmp 0x12; jg */
            c = S8(rec + (uint32_t)i);                              /* 0x1962B movsx */
            total += c * c * ((i + 1) >> 1);                        /* 0x19637..0x19643 */
        }
        return total;                                               /* 0x19648 */
    }
    for (j = 0; j < S32(0xF68B4u); j++) {                           /* 0x19650, 0x196E0 cmp [0xF68B4]; jge */
        total = 0;                                                  /* 0x196EA */
        for (i = 1; i <= 0x12; i++) {                               /* 0x196ED, 0x19681 cmp 0x12; jg */
            c = S8(rec + (uint32_t)i);
            /* 0x1968D test dl,dl; 0x19696 dword [j + 0xF68B5] sar 24 = int8 0xF68B8[j]; cmp i */
            if (D8(rec + (uint32_t)i) != 0 && S8(0xF68B8u + (uint32_t)j) == i)
                total += (c + 1) * (c + 1) * ((i + 1) >> 1);        /* 0x196AA..0x196C4 */
            else
                total += c * c * ((i + 1) >> 1);                    /* 0x1965A..0x19678 */
        }
        if (total > best)                                           /* 0x196CC cmp; jle (signed) */
            best = total;                                           /* 0x196D4 */
    }
    return best;                                                    /* 0x196F6 */
}

/* ---- 0x13E50: the census ---------------------------------------------------------------------------------------- */

void f13E50(void)
{
    /* [ebp-0x68] and [ebp-0xA0]: uint8[0x37] each, indexed tokens - 1. Port guard (G): each array is placed in 256
       scratch bytes that cover every index an int8 count can give (-129..126), cleared, so index -1 (0 tokens, file
       comment) and impossible counts stay inside the port's memory; the original reads and writes stack bytes there. */
    int8_t hist_mem[256], seen_mem[256];
    int8_t *hist = hist_mem + 129, *seen = seen_mem + 129;
    int32_t n;                                                      /* [ebp-0x2c] */
    int32_t q;                                                      /* [ebp-0x18] tokens - 1 */
    int32_t dx, dy;                                                 /* [ebp-0x24], [ebp-0x20] */
    int32_t step;                                                   /* [ebp-0x1c] */
    int32_t k, s, x, y, i, r;

    f3CD6C(5);                                                      /* 0x13E6D census.gif */
    fg_vbopen(S32(BGBUF));                                          /* 0x13E79 */
    memset(hist_mem, 0, sizeof hist_mem);                           /* port: the scratch bytes (G) */
    memset(seen_mem, 0, sizeof seen_mem);
    memset(hist, 0, 0x37);                                          /* 0x13E8B */
    memset(seen, 0, 0x37);                                          /* 0x13E9D */
    for (n = 1; n <= 9; n++) {                                      /* 0x13EA2, 0x13EB1 cmp 9; jg */
        if (D8(nat(n, 0x9A)) == 0)                                  /* 0x13EBE mov dl,[199n + 0xF1566]; test; je */
            continue;
        hist[S8(nat(n, 0x98)) - 1]++;                               /* 0x13ECF dword [199n + 0xF1561] sar 24; 0x13ED8 inc byte [eax + ebp - 0x69] */
    }

    for (n = 1; n <= 9; n++) {                                      /* 0x13EDE, 0x14113 cmp 9; jg */
        dx = 0;                                                     /* 0x1411B */
        dy = 0;                                                     /* 0x1411E */
        if (D8(nat(n, 0x9A)) == 0)                                  /* 0x14128 mov bl,[..]; test; je */
            continue;
        q = S8(nat(n, 0x98)) - 1;                                   /* 0x14139 sar 24; dec */
        k = hist[q];                                                /* 0x14149 dword [q + ebp - 0x6b] sar 24 */
        if (k == 1) {                                               /* 0x14150 cmp 1; jne 0x13EEA */
            dx = 10;                                                /* 0x1415E */
            dy = 10;                                                /* 0x14161 */
            step = k;                                               /* 0x13EF9 (eax = 1) */
        } else {
            step = 0x13 / (k - 1);                                  /* 0x13EEA lea ebx,[eax-1]; idiv (signed) */
        }
        s = seen[q];                                                /* 0x13F23 dword [q + ebp - 0xa3] sar 24 */
        y = 47 * (q / 11) + dy + 0x42 + step * s;                   /* 0x13EFC..0x13F38 */
        x = 49 * (q % 11) + dx + 0x36 + step * s;                   /* 0x13F3A..0x13F61 (remainder of the idiv) */
        f3AF8C(TOKEN_SPR + 8u * (uint32_t)n, x, y);                 /* 0x13F70 */
        if (D8(nat(n, 0x35)) != 0) {                                /* 0x13F7C cmp byte [199n + 0xF1501],0; je */
            fg_setcolor(5);                                         /* 0x13F8B */
            fg_move(x, y);                                          /* 0x13FF0 */
            fg_box(x, x + 0x17, y, y + 0x16);                       /* 0x140AD */
            fg_setcolor(0x0F);                                      /* 0x140B7 */
            fg_dashrel(0x17, 0, 0xDDDD);                            /* 0x140C8 */
            fg_dashrel(0, 0x16, 0xDDDD);                            /* 0x140D9 */
            fg_dashrel(-0x17, 0, 0xDDDD);                           /* 0x140EA */
            fg_dashrel(0, -0x16, 0xDDDD);                           /* 0x140FB */
        }
        seen[q]++;                                                  /* 0x14106 inc byte [q + ebp - 0xa0] */
    }

    if (S8(0xF40FEu) == 1) {                                        /* 0x14169 dword [0xF40FB] sar 24; cmp 1; jne */
        f3AF8C(GAME_SPR_C_INVLD0, 0x4F, 0x1BE);                     /* 0x14185 */
        fg_setcolor(5);                                             /* 0x1418C */
        f3A0B0(0, 0x1CC, 0x280, DSTR(0xC7A5Cu));                    /* 0x141A5 "This data is invalid ... PBEM consolidation" */
    }
    f3BB48(0xBB800u, 0x2A, 0x108);                                  /* 0x141B9 CONTINUE x 264 */
    f3BC64(0xBB800u, 0x2A, 0x18E);                                  /* 0x141CD y 398 */
    f3BD80(0xBB800u, 0x2A, 0x6F);                                   /* 0x141E1 w 111 */
    f3BE10(0xBB800u, 0x2A, 0x17);                                   /* 0x141F7 h 23 */
    for (i = 0; S16(0xBB800u) > i; i++)                             /* 0x14207 dword [0xBB7FE] sar 16; jle */
        f3B4E0(0xBB802u + 20u * (uint32_t)i);                       /* 0x14230 */
    fg_vbclose();                                                   /* 0x14237 */
    f3CC68();                                                       /* 0x1423C */
    do
        r = f78BA4(0xBB800u, 0);                                    /* 0x14248 */
    while (r == -100 || r != -1);                                   /* 0x14253, 0x1425B */
    f3BD80(0xBB800u, 0x2A, 0x6E);                                   /* 0x1426F w 110 */
    f3BE10(0xBB800u, 0x2A, 0x13);                                   /* 0x14283 h 19 */
}

/* ---- 0x1A124: the advance description pop-up ------------------------------------------------------------------- */

void f1A124(int idx)
{
    int32_t maxw;                                                   /* [ebp-0x24] */
    int32_t H;                                                      /* [ebp-0x20] */
    int32_t X, Y;                                                   /* [ebp-0x34], [ebp-0x30] */
    int32_t bx;                                                     /* [ebp-0x2c] */
    int32_t tw;                                                     /* [ebp-0x1c] */
    uint32_t saved;                                                 /* [ebp-0x18] */
    uint32_t name_a = D32(0xC7DC4u + 4u * (uint32_t)idx);          /* 0x1A1F7 [4*idx + 0xC7DC4] */
    uint32_t text_a = D32(0xC7E9Cu + 4u * (uint32_t)idx);          /* 0x1A14A [4*idx + 0xC7E9C] */
    const char *name, *text;
    int32_t r;

    /* Port guard (G): indices outside 0..23 (reachable only with g_original, see f19C10: -38..64, whose table reads
       stay inside g_ds) give entries such as 0xC7E9C[24] = 0x0B86ED00, no address in the data object; the port then
       uses an empty text instead of reading outside g_ds. */
    name = ds_valid(name_a, 1) ? DSTR(name_a) : "";
    text = ds_valid(text_a, 1) ? DSTR(text_a) : "";

    H = f3A42C(0, 0, 0xC8, 0, text, &maxw, 0);                      /* 0x1A158 measure */
    H += 0x41;                                                      /* 0x1A160 */
    X = 0xD2;                                                       /* 0x1A164 */
    Y = (0x1E0 - H) / 2;                                            /* 0x1A16B..0x1A17E signed */
    bx = X + 0x37;                                                  /* 0x1A184 */
    f78884();                                                       /* 0x1A18A */
    junk_makegif(X, X + 0xDB, Y, Y + H - 1, DSTR(0xB1686u));        /* 0x1A1AD fg_makegif(.., "junk2") (port: junk.c) */
    f788AC();                                                       /* 0x1A1B5 */
    saved = D32(CURDLG);                                            /* 0x1A1BA */
    f3C940(saved);                                                  /* 0x1A1C2 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x1A1CD */
    f3C9D0(X, Y, 0xDC, H, 3);                                       /* 0x1A1E5 */
    fg_setcolor(0x0F);                                              /* 0x1A1EC */
    f3A0B0(X, Y + 0x0A, 0xDC, name);                                /* 0x1A20C */
    tw = fgf_width(name, (int)strlen(name));                        /* 0x1A220 repne scasb; 0x1A231 */
    fg_move((0xDC - tw) / 2 + X - 2, Y + 0x15);                     /* 0x1A25B */
    fg_draw((0xDC - tw) / 2 + X + 2 + tw, Y + 0x15);                /* 0x1A285 the underline */
    fg_setcolor(5);                                                 /* 0x1A28F */
    f3A42C(X + 0x0A, Y + 0x1E, 0xC8, 0, text, &maxw, 1);            /* 0x1A2BB */
    fg_vbclose();                                                   /* 0x1A2C0 */
    f3BB48(0xBBBECu, 0x2A, bx);                                     /* 0x1A2D2 */
    f3BC64(0xBBBECu, 0x2A, Y + H - 0x1D);                           /* 0x1A2EC */
    f3C284(0xBBBECu, 0x2A, 0xB168Cu);                               /* 0x1A300 "CONTINUE" */
    f3B05C(0xBBBECu);                                               /* 0x1A30A */
    f78884();                                                       /* 0x1A30F */
    fg_vbpaste(X, X + 0xDB, Y, Y + H - 1, X, Y + H - 1);            /* 0x1A339 */
    f788AC();                                                       /* 0x1A341 */
    do
        r = f78BA4(0xBBBECu, 0);                                    /* 0x1A34D */
    while (r != -1);                                                /* 0x1A358 */
    setD32(CURDLG, saved);                                          /* 0x1A360 */
    f3C8C4(saved);                                                  /* 0x1A365 */
    f78884();                                                       /* 0x1A36A */
    fg_move(X, Y);                                                  /* 0x1A377 */
    junk_showgif(DSTR(0xB1695u), 3);                                /* 0x1A386 fg_showgif("junk2", 3) (port: junk.c) */
    f788AC();                                                       /* 0x1A38E */
}

/* ---- 0x19C10: Civilization Tools -------------------------------------------------------------------------------- */

void f19C10(void)
{
    char buf[0x64];                                                 /* [ebp-0xc8] */
    int16_t rows[9];                                                /* [ebp-0x64]: the nation of each row; never read */
    uint32_t saved;                                                 /* [ebp-0x18] */
    int32_t row, n;                                                 /* [ebp-0x44], [ebp-0x48] */
    int32_t total;                                                  /* [ebp-0x30] */
    int32_t i, c, ri, idx, j;                                       /* [ebp-0x40], [ebp-0x1c], [ebp-0x3c] */
    int32_t flag;                                                   /* [ebp-0x2c] */
    int32_t r, k, mx, my;                                           /* [ebp-0x4c], [ebp-0x28], [ebp-0x38], [ebp-0x34] */
    const char *s;

    saved = D32(CURDLG);                                            /* 0x19C28 */
    f78884();                                                       /* 0x19C30 */
    junk_makegif(0, 0x27F, 0, 0x1DF, DSTR(0xB161Fu));               /* 0x19C48 fg_makegif(.., "junk") (port: junk.c) */
    f788AC();                                                       /* 0x19C50 */
    for (;;) {
        f3CD6C(0x14);                                               /* 0x19C5A */
        setD8(BGPIC, 0);                                            /* 0x19C67 */
        if ((D8(UI_SETUP_FLAGS) & 0x80) &&                          /* 0x19C6D test dl,0x80 */
            (D8(0xF4102u) == 0x12 || D8(0xF4102u) == 0x13)) {       /* 0x19C78, 0x19C7D */
            fg_setcolor(5);                                         /* 0x19C84 */
            f39F90(1);                                              /* 0x19CA0 */
            f3A0B0(0, 0x1B3, 0x280, DSTR(0xB1624u));                /* 0x19CA7 "This data is invalid ..." */
            f3A0B0(0, 0x1B3, 0x280, DSTR(0xC7A5Cu));                /* 0x19CBD the same text, second copy */
        }
        fg_vbopen(S32(BGBUF));                                      /* 0x19CC9 */
        f39F90(2);                                                  /* 0x19CD6 */
        snprintf(buf, sizeof buf, "%s", DSTR(0xB166Eu));            /* 0x19CEC sprintf(buf, "Civilization Tools") */
        f3A18C(0, 6, 0x280, 0x47, buf);                             /* 0x19D07 (ecx 0x47 kept over sprintf) */
        row = 0;                                                    /* 0x19D0E */
        for (n = 1; n <= 9; n++) {                                  /* 0x19D11, 0x19F65 cmp 9; jg */
            if (D8(nat(n, 0x9A)) == 0)                              /* 0x19F72 mov bh,[199n + 0xF1566]; test; je */
                continue;
            rows[row] = (int16_t)n;                                 /* 0x19F82 mov [ebp + 2*row - 0x64], ax */
            f3AF8C(TOKEN_SPR + 8u * (uint32_t)n,                    /* 0x19FCA */
                   (0x33 - (int32_t)D16(0xF4AD0u + 8u * (uint32_t)n)) / 2 + 5,   /* 0x19F9E word; signed /2; + 5 */
                   49 * row + 0x2B);
            fg_setcolor(S32(0xF40F8u) == n ? 0x0F : 5);             /* 0x19FD4 cmp; 0x19D1D / 0x19FDD */
            f3A0B0(5, 49 * row + 0x2B + (int32_t)D16(0xF4AD2u + 8u * (uint32_t)n) + 2, 0x33,   /* 0x19D61 */
                   DSTR(D32(0xC768Cu + 4u * (uint32_t)n)));
            total = 0;                                              /* 0x19D68 */
            for (i = 0; i <= 0x17; i++)                             /* 0x19D6B, 0x19D76 cmp 0x17; jg */
                if (D8(nat(n, 0x24 + (uint32_t)i)) != 0)            /* 0x19D86 mov cl,[199n + i + 0xF14F0]; test */
                    total += S16(0xBD480u + 2u * (uint32_t)i);      /* 0x19D93 dword [2i + 0xBD47E] sar 16 */
            fg_setcolor(5);                                         /* 0x19DA4 */
            snprintf(buf, sizeof buf, "%d", total);                 /* 0x19DBA itoa(total, buf, 10) */
            f3A0B0(0x258, 49 * row + 0x35, 0x28, buf);              /* 0x19DE5 */
            for (c = 0; c < 6; c++)                                 /* 0x19DEA, 0x19DED / 0x19E04 cmp 6; jge */
                for (ri = 0; ri < 4; ri++) {                        /* 0x19DF9, 0x19E1B cmp 4; jge */
                    idx = 4 * c + ri;                               /* 0x19E28..0x19E2E */
                    if (D8(nat(n, 0x24 + (uint32_t)idx)) != 0)      /* 0x19E3A mov ch,[199n + idx + 0xF14F0]; test */
                        s = DSTR(D32(0xC7E24u + 4u * (uint32_t)idx));   /* 0x19E4E upper case */
                    else
                        s = DSTR(D32(0xC7DC4u + 4u * (uint32_t)idx));   /* 0x19E5C mixed case */
                    strncpy(buf, s, sizeof buf - 1);                /* 0x19E63..0x19E79 inline strcpy */
                    buf[sizeof buf - 1] = '\0';
                    if (D8(nat(n, 0x24 + (uint32_t)idx)) == 0) {    /* 0x19E8C cmp byte [..],0; jne */
                        fg_setcolor(5);                             /* 0x19E96 push 5 */
                    } else {
                        flag = 1;                                   /* 0x19E9A */
                        for (j = 0; S16(0xF6E76u + 2u * (uint32_t)n) > j; j++)   /* 0x19EB1 dword [2n + 0xF6E74] sar 16; jle */
                            if (idx == S16(0xF6E12u + 10u * (uint32_t)n + 2u * (uint32_t)j)) {   /* 0x19ED1 dword [10n + 2j + 0xF6E10] sar 16 */
                                flag = 0;                           /* 0x19EEA */
                                break;
                            }
                        fg_setcolor(flag != 0 ? 0x0E : 0x0F);       /* 0x19EED je; 0x19EF3 / 0x19EF7 */
                    }
                    f3A0B0(90 * c + 0x3D, 49 * row + 0x24 + 11 * ri + 1, 0x5A, buf);   /* 0x19F4F */
                }
            row++;                                                  /* 0x19F5C */
        }
        (void)rows;
        for (i = 0; S16(0xBD940u) > i; i++)                         /* 0x19FF1 dword [0xBD93E] sar 16; jle */
            f3B4E0(0xBD942u + 20u * (uint32_t)i);                   /* 0x1A01A */
        fg_vbclose();                                               /* 0x1A026 */
        setD32(CURDLG, 0xBD940u);                                   /* 0x1A02B */
        f3CC68();                                                   /* 0x1A031 */
        for (;;) {
            r = f78BA4(0xBD940u, 0);                                /* 0x1A03D */
            if (r == -100)                                          /* 0x1A048 */
                continue;
            if (r <= -0x50 && r >= -0x57)                           /* 0x1A050 jg, 0x1A059 jl */
                break;
            if (r == -1) {                                          /* 0x1A0DA cmp -1; jne 0x1A036 */
                fg_vbopen(S32(BACKBUF));                            /* 0x1A0EA */
                fg_move(0, 0);                                      /* 0x1A0F6 */
                junk_showgif(DSTR(0xB1681u), 3);                    /* 0x1A105 fg_showgif("junk", 3) (port: junk.c) */
                f3CC14();                                           /* 0x1A10D */
                setD32(CURDLG, saved);                              /* 0x1A115 */
                return;
            }
        }
        k = -0x50 - r;                                              /* 0x1A067 */
        mx = S32(MOUSE_X) - 0x3D;                                   /* 0x1A071 */
        my = S32(MOUSE_Y) - (49 * k + 0x24);                        /* 0x1A07A..0x1A091 */
        ri = my / 11;                                               /* 0x1A0A4 idiv (signed) */
        c = mx / 0x5A;                                              /* 0x1A0B7 idiv (signed) */
        if (!g_original) {
            /* BUG(1995): the row button is 48 high, the four name rows 44: its bottom 4 pixels gave row 4, the next
               column's first advance or, in the last column, index 24 past the tables (a wild text pointer,
               0xC7E9C[24] = 0x0B86ED00). Also, f78BA4 picks the button at the press but f3C5D4's f775F0 leaves the
               RELEASE position in 0xE1628 / 0xE162C: a press on a row dragged off it gives ri -38..40 and c up to 6
               (x >= 601), indices -38..64 into the 24-entry tables. Fixed: the position is kept inside the clicked
               row's grid (the strip counts as the last row). */
            if (ri > 3)
                ri = 3;
            if (ri < 0)
                ri = 0;
            if (c > 5)
                c = 5;
        }
        idx = 4 * c + ri;                                           /* 0x1A0BC..0x1A0C5 */
        f1A124(idx);                                                /* 0x1A0CD */
    }                                                               /* 0x1A0D2 jmp 0x19C55: redraw */
}

/* ---- 0x76798: "Select Nation To Show" --------------------------------------------------------------------------- */

int f76798(void)
{
    const char *title = DSTR(0xCA9BCu);                             /* [ebp-0x1c] "Select Nation To Show" */
    int32_t tw;                                                     /* [ebp-0x38] */
    int32_t W;                                                      /* [ebp-0x34] */
    int32_t cnt;                                                    /* [ebp-0x24] */
    int32_t list[9] = {0};                                          /* [ebp-0x6c] (port: cleared, G) */
    int32_t n, i;                                                   /* [ebp-0x44] */
    int32_t maxw;                                                   /* [ebp-0x2c] */
    int32_t H;                                                      /* [ebp-0x28] */
    int32_t X, Y;                                                   /* [ebp-0x40], [ebp-0x3c] */
    int32_t bx;                                                     /* [ebp-0x20] */
    uint32_t saved;                                                 /* [ebp-0x18] */
    int32_t r;                                                      /* [ebp-0x30] */

    tw = fgf_width(title, (int)strlen(title)) + 0x0A;               /* 0x767BC repne scasb; 0x767C6; 0x767CE */
    W = tw > 0xC8 ? tw : 0xC8;                                      /* 0x767D4 cmp 0xC8; jle (signed) */
    if (W - 0x14 > 0x15E)                                           /* 0x767ED sub 0x14; cmp 0x15E; jle */
        W = 0x15E;                                                  /* 0x767F7 (kept: never applies, file comment) */
    cnt = 0;                                                        /* 0x76800 */
    for (n = 1; n <= 9; n++)                                        /* 0x76803, 0x76812 cmp 9; jg */
        if (S8(nat(n, 0x9A)) == 3)                                  /* 0x7681F dword [199n + 0xF1563] sar 24; cmp 3 */
            list[cnt++] = n;                                        /* 0x76833 mov [ebp + 4*cnt - 0x6c], n */

    H = f3A42C(0, 0, W - 0x14, 0, title, &maxw, 0);                 /* 0x76855 measure */
    H += 20 * (cnt - 1) + 0x2D;                                     /* 0x7685D..0x76870 */
    X = (0x280 - W) / 2;                                            /* 0x76873..0x76886 signed */
    Y = (0x1E0 - H) / 2;                                            /* 0x76889..0x7689C */
    bx = X + (W - 0x6E) / 2;                                        /* 0x7689F..0x768B3 */
    f78884();                                                       /* 0x768B6 */
    junk_makegif(X, X + W - 1, Y, Y + H - 1, DSTR(0xB935Fu));       /* 0x768D8 fg_makegif(.., "junk") (port: junk.c) */
    f788AC();                                                       /* 0x768E0 */
    saved = D32(CURDLG);                                            /* 0x768E5 */
    if (saved != 0)                                                 /* 0x768ED test; je */
        f3C940(saved);                                              /* 0x768F1 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x768FD */
    f3C9D0(X, Y, W, H, 3);                                          /* 0x76913 */
    fg_setcolor(5);                                                 /* 0x7691A */
    f3A42C(X + 0x0A, Y + 0x0A, W - 0x14, 2, title, &maxw, 1);       /* 0x76945 centred */
    for (i = 0; i < cnt; i++) {                                     /* 0x7694A, 0x76958 cmp; jge */
        f3BB48(0xBC3C4u, (int16_t)(i + 0x12), bx);                  /* 0x76970 movsx edx, ax */
        f3BC64(0xBC3C4u, (int16_t)(i + 0x12), Y + H - 0x1D - 20 * (cnt - 1 - i));   /* 0x76975..0x769A3 */
        f3C284(0xBC3C4u, (int16_t)(i + 0x12), D32(0xC768Cu + 4u * (uint32_t)list[i]));   /* 0x769C6 */
    }
    setD16(0xBC3C4u, (uint16_t)cnt);                                /* 0x769D0 mov word [0xBC3C4], ax */
    fg_vbclose();                                                   /* 0x769D6 */
    f3B05C(0xBC3C4u);                                               /* 0x769E0 */
    f78884();                                                       /* 0x769E5 */
    fg_vbpaste(X, X + W - 1, Y, Y + H - 1, X, Y + H - 1);           /* 0x76A0E */
    f788AC();                                                       /* 0x76A16 */
    do
        r = f78BA4(0xBC3C4u, 0);                                    /* 0x76A22 */
    while (r == -100 || r > -0x36 || r < -0x3D);                    /* 0x76A2D, 0x76A35 jg, 0x76A3E jl */
    r = list[-0x36 - r];                                            /* 0x76A40..0x76A4C */
    setD32(CURDLG, saved);                                          /* 0x76A52 */
    if (saved != 0)                                                 /* 0x76A57 */
        f3C8C4(saved);                                              /* 0x76A5B */
    f78884();                                                       /* 0x76A60 */
    fg_move(X, Y);                                                  /* 0x76A6D */
    junk_showgif(DSTR(0xB9364u), 3);                                /* 0x76A7C fg_showgif("junk", 3) (port: junk.c) */
    f3CCBC();                                                       /* 0x76A84 */
    f788AC();                                                       /* 0x76A89 */
    return r;                                                       /* 0x76A8E */
}

/* ---- 0x76A9C: Goods Held ---------------------------------------------------------------------------------------- */

/* one face-up row: 0x76BDE..0x76C7C (and its copy 0x76D5D..0x76DAD, which passes the second "POINTS", 0xB9374) */
static void goods_face_up(int32_t n, int32_t row, uint32_t points)
{
    char buf[0x64];                                                 /* [ebp-0x88] */

    f71FB0(0x37, 49 * row + 0x24, 0x1CC, n);                        /* 0x76BFF / 0x76D7E */
    fg_setcolor(0x0E);                                              /* 0x76C06 */
    f39F90(1);                                                      /* 0x76C13 */
    f3A0B0(0x249, 49 * row + 0x2D, 0x32, DSTR(points));             /* 0x76C3B "POINTS" */
    snprintf(buf, sizeof buf, "%d", f195D8(n));                     /* 0x76C4E, 0x76C53 itoa(.., 10) */
    f3A0B0(0x249, 49 * row + 0x41, 0x32, buf);                      /* 0x76C7C */
}

void f76A9C(void)
{
    char buf[0x64];                                                 /* [ebp-0x88] */
    int32_t chosen;                                                 /* [ebp-0x1c] */
    int32_t n, row, i, r;                                           /* [ebp-0x20], [ebp-0x24], [ebp-0x14] */

    f3CCBC();                                                       /* 0x76AB0 */
    f3CD6C(0x13);                                                   /* 0x76ABA goodheld.gif */
    setD8(BGPIC, 0);                                                /* 0x76AC8 */
    fg_vbopen(S32(BGBUF));                                          /* 0x76ACE */
    f39F90(2);                                                      /* 0x76ADB */
    fg_setcolor(4);                                                 /* 0x76AE2 */
    snprintf(buf, sizeof buf, "%s", DSTR(0xB9369u));                /* 0x76AFB sprintf(buf, "Goods Held") */
    f3A18C(0, 6, 0x280, 0x23, buf);                                 /* 0x76B16 (ecx 0x23 kept over sprintf) */
    f39F90(1);                                                      /* 0x76B25 */
    chosen = -1;                                                    /* 0x76B2A (ebx kept over 0x39F90) */
    fg_vbclose();                                                   /* 0x76B2D */
    if (S8(UI_SETUP_HUMANS) == 1) {                                 /* 0x76B32 dword [0xE161D] sar 24; cmp 1; jne */
        for (n = 1; n <= 9; n++)                                    /* 0x76B3F, 0x76B4A cmp 9; jg 0x76B93 */
            if (S8(nat(n, 0x9A)) == 3) {                            /* 0x76B57 dword [199n + 0xF1563] sar 24 */
                chosen = n;                                         /* 0x76B65, 0x76B90 */
                break;
            }
    } else if (D8(UI_SETUP_FLAGS) & 0x80) {                         /* 0x76B6A test dl,0x80 */
        chosen = S8(0xE1622u);                                      /* 0x76B75 dword [0xE161F] sar 24 */
    } else if (S8(UI_SETUP_HUMANS) > 1) {                           /* 0x76B7F cmp eax,1; jle */
        chosen = f76798();                                          /* 0x76B84 */
    } else {
        chosen = S32(0xF40F8u);                                     /* 0x76B8B */
    }

    fg_vbopen(S32(BGBUF));                                          /* 0x76B9A */
    row = 0;                                                        /* 0x76BA4 */
    for (n = 1; n <= 9; n++) {                                      /* 0x76BA7, 0x76C8D cmp 9; jg */
        if (D8(nat(n, 0x9A)) == 0)                                  /* 0x76C9E mov dh,[199n + 0xF1566]; test; je */
            continue;
        f3AF8C(TOKEN_SPR + 8u * (uint32_t)n,                        /* 0x76CED */
               (0x35 - (int32_t)D16(0xF4AD0u + 8u * (uint32_t)n)) / 2,   /* 0x76CBF word; signed /2 */
               49 * row + 0x2B);
        fg_setcolor(0x0E);                                          /* 0x76CF4 */
        f3A0B0(0, 49 * row + 0x2B + (int32_t)D16(0xF4AD2u + 8u * (uint32_t)n) + 2, 0x35,   /* 0x76D34 */
               DSTR(D32(0xC768Cu + 4u * (uint32_t)n)));
        f194E4(n);                                                  /* 0x76D3C */
        if (S16(0xF40C2u) != 0)                                     /* 0x76D41 cmp word [0xF40C2],0; jne 0x76BDE */
            goods_face_up(n, row, 0xB937Bu);
        else if (n == chosen)                                       /* 0x76D55 cmp; jne 0x76BB3 */
            goods_face_up(n, row, 0xB9374u);
        else
            f71F2C(0x37, 49 * row + 0x24, 0x1CC, n);                /* 0x76BD4 */
        row++;                                                      /* 0x76C84 */
    }
    f3BB48(0xBB800u, 0x2A, 0x109);                                  /* 0x76DC1 CONTINUE x 265 */
    f3BC64(0xBB800u, 0x2A, 0x1C2);                                  /* 0x76DD7 y 450 */
    for (i = 0; S16(0xBB800u) > i; i++)                             /* 0x76DE7 dword [0xBB7FE] sar 16; jle */
        f3B4E0(0xBB802u + 20u * (uint32_t)i);                       /* 0x76E10 */
    fg_vbclose();                                                   /* 0x76E17 */
    f3CC68();                                                       /* 0x76E1C */
    do
        r = f78BA4(0xBB800u, 0);                                    /* 0x76E28 */
    while (r == -100 || r != -1);                                   /* 0x76E33, 0x76E3B */
    f3CC14();                                                       /* 0x76E40 */
}

/* ---- 0x764A8 / 0x763FC: Trading Wrapup -------------------------------------------------------------------------- */

void f764A8(void)
{
    int32_t t, y, j, c, a, b;                                       /* [ebp-0x24], [ebp-0x20], [ebp-0x2c], [ebp-0x28] */
    int32_t scroll;

    /* [ebp-0x18] = 0 (0x764BF) and the card counter [ebp-0x1c] (0x76627, 0x7670E, incremented per card) are never
       read: left out. */
    setD8(0xEE168u, 0x10);                                          /* 0x764C2 */
    f3CD6C(0x10);                                                   /* 0x764CE tradwrap.gif */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BGBUF), S32(BACKBUF));   /* 0x764F3 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x76502 */
    f3A18C(0, 6, 0x280, 0xB0, DSTR(0xB934Bu));                      /* 0x76520 "Trading Wrapup" */
    for (t = S16(0x112544u);                                        /* 0x76525 dword [0x112542] sar 16 */
         (scroll = S16(0x112544u), scroll + 6 > t) &&               /* 0x76538..0x76546 jle */
         S8(0x110F74u) > t;                                         /* 0x7654C dword [0x110F71] sar 24; jle */
         t++) {
        y = 60 * (t - scroll) + 0x32;                               /* 0x76561..0x76573 */
        fg_setcolor(5);                                             /* 0x7657B */
        f39F90(2);                                                  /* 0x76588 */
        f3A0B0(0, y + 0x0F, 0x280, DSTR(0xB935Au));                 /* 0x7659F "<-->" */
        f39F90(1);                                                  /* 0x765A9 */
        a = S8(0x1110B5u + (uint32_t)t);                            /* 0x765B7 dword [t + 0x1110B2] sar 24 */
        f3AF8C(TOKEN_SPR + 8u * (uint32_t)a,                        /* 0x765E9 */
               (0x35 - (int32_t)D16(0xF4AD0u + 8u * (uint32_t)a)) / 2 + 0xF5, y + 5);
        f3A0B0(0xF5, y + 5 + (int32_t)D16(0xF4AD2u + 8u * (uint32_t)a) + 2, 0x35,   /* 0x76622 */
               DSTR(D32(0xC768Cu + 4u * (uint32_t)a)));
        for (j = 0; j < 3; j++) {                                   /* 0x7662A, 0x76635 cmp 3; jge */
            c = S8(0x110F75u + 5u * (uint32_t)t + (uint32_t)j);     /* 0x7664A dword [5t + j + 0x110F72] sar 24 */
            if (c == 0)                                             /* 0x76659 test; je */
                continue;
            f3AF8C(CARD_SPR + 8u * (uint32_t)c, 0xCB - 39 * j, y);  /* 0x76689 */
        }
        b = S8(0x1110D5u + (uint32_t)t);                            /* 0x76699 dword [t + 0x1110D2] sar 24 */
        f3AF8C(TOKEN_SPR + 8u * (uint32_t)b,                        /* 0x766CD */
               (0x35 - (int32_t)D16(0xF4AD0u + 8u * (uint32_t)b)) / 2 + 0x154, y + 5);
        f3A0B0(0x154, y + 5 + (int32_t)D16(0xF4AD2u + 8u * (uint32_t)b) + 2, 0x35,   /* 0x76709 */
               DSTR(D32(0xC768Cu + 4u * (uint32_t)b)));
        for (j = 0; j < 5; j++) {                                   /* 0x76711, 0x7671C cmp 5; jge */
            c = S8(0x111015u + 5u * (uint32_t)t + (uint32_t)j);     /* 0x76735 dword [5t + j + 0x111012] sar 24 */
            if (c == 0)                                             /* 0x76744 test; je */
                continue;
            f3AF8C(CARD_SPR + 8u * (uint32_t)c, 0x195 + 39 * j, y); /* 0x76773 */
        }
    }
    fg_vbclose();                                                   /* 0x7677A */
    f3B05C(0xCA5A8u);                                               /* 0x76784 */
    f3CC14();                                                       /* 0x76789 */
}

void f763FC(void)
{
    int32_t r;                                                      /* [ebp-0x14] */

    if (S8(0x110F74u) == 0)                                         /* 0x76410 dword [0x110F71] sar 24; test; je */
        return;
    setD16(0x112544u, 0);                                           /* 0x76422 */
    for (;;) {
        f764A8();                                                   /* 0x76429 */
        if (f7BA64() == 0) {                                        /* 0x7642E test; jne */
            r = f78BA4(0xCA5A8u, 0);                                /* 0x7643E */
        } else {
            f78A20(S16(0xF40BAu) == 0 ? 0xFA : 0x3E8);              /* 0x76448..0x76463 */
            r = -1;                                                 /* 0x7645E edx = -1, kept over 0x78A20 */
        }
        if (r == -100)                                              /* 0x7646B */
            continue;
        if (r == -0x2C) {                                           /* 0x76474 */
            setD16(0x112544u, (uint16_t)(D16(0x112544u) + 6));      /* 0x76479 add word, 6 */
            continue;
        }
        if (r == -0x2B) {                                           /* 0x76486 */
            setD16(0x112544u, (uint16_t)(D16(0x112544u) - 6));      /* 0x7648B sub word, 6 */
            continue;
        }
        if (r == -1)                                                /* 0x76498 cmp -1; jne 0x76429 */
            return;
    }
}

/* ---- dispatcher cases (0x3FB94, jump table 0x3F780) ------------------------------------------------------------- */

/* id 4 -> 0x3FBF1 CENSUS */
int dcase_4(int code)
{
    f3CCBC();                                                       /* 0x3FBF1 */
    f13E50();                                                       /* 0x3FBF6 */
    f3CC14();                                                       /* 0x3FBFB */
    return code;                                                    /* 0x3FC00 mov eax,[ebp-0x3c] */
}

/* id 6 -> 0x3FC27 TOOLS */
int dcase_6(int code)
{
    f19C10();                                                       /* 0x3FC27 */
    return code;                                                    /* 0x3FC2C */
}

/* id 7 -> 0x3FC39 GOODS */
int dcase_7(int code)
{
    f76A9C();                                                       /* 0x3FC39 */
    return code;                                                    /* 0x3FC3E */
}

/* id 0x103 -> 0x3FC5D REVIEW (Trading Wrapup from the trade screens) */
int dcase_103(int code)
{
    uint8_t pic;                                                    /* [ebp-0x18] (byte) */

    f78884();                                                       /* 0x3FC5D */
    if (g_original)
        junk_makegif(0, 0x27F, 0, 0x1DE, DSTR(0xB400Cu));           /* 0x3FC75 fg_makegif(0, 0x27F, 0, 0x1DE, "junk") (port: junk.c) */
    else
        junk_makegif(0, 0x27F, 0, 0x1DF, DSTR(0xB400Cu));  /* BUG(1995): the original saves rows 0..478 only, so row 479 keeps
                                                       tradwrap.gif's row when the screen is put back. Fixed: 0..479. */
    f788AC();                                                       /* 0x3FC7D */
    pic = D8(0xEE168u);                                             /* 0x3FC82 */
    f763FC();                                                       /* 0x3FC8A */
    fg_vbopen(S32(BGBUF));                                          /* 0x3FC96 (no fg_vbclose: f3CC68 closes) */
    fg_move(0, 0);                                                  /* 0x3FCA2 */
    junk_showgif(DSTR(0xB4011u), 3);                                /* 0x3FCB1 fg_showgif("junk", 3) (port: junk.c) */
    f3CC68();                                                       /* 0x3FCB9 */
    f3CCBC();                                                       /* 0x3FCBE */
    setD8(BGPIC, 0);                                                /* 0x3FCC5 */
    setD8(0xEE168u, pic);                                           /* 0x3FCCE */
    return code;                                                    /* 0x3FCD3 */
}
