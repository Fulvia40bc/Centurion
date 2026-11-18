/* The nation choice of AC_MAIN.EXE, human players (module N1 of docs/MAP_PLAN.md): the screen 0x2EFA0 and the three
   routines only it calls, 0x2F9F8 (load the pictures), 0x2FC04 (picture names and card rectangles for the player
   count) and 0x300E0 (the dialog's button id -> code), plus the port's not_ported(). Literal translations; the address
   of each instruction a line comes from is in its comment. D = read in the code, G = inference. Prototypes and the
   data layouts: game.h (section "Nation choice, human players") and ui.h. The other N1 helpers (0x3AB7C, 0x3A9F4,
   0x3AF54, 0x79A9C, 0x78A20, 0x3C940, 0x3C7A0) live in their own files.

   Main calls f2EFA0 at 0x1184F and it returns to 0x11854, the data-compare point (MAP_PLAN.md N1).

   Watcom runtime -> standard C: 0x7DB3D sprintf -> snprintf (bounded to the original's stack buffers: 100 bytes at
   [ebp-0x98] in 0x2EFA0, 52 bytes at [ebp-0x4c] in 0x2F9F8); 0x822B5 malloc -> load_heap_alloc and 0x877E6 free ->
   load_heap_free for the data fields below (game.h). __CHK dropped. Catalog calls through fg.h (cat_size 0xA24A7,
   cat_load 0xA25B3), in the original argument order.

   Heap pointers the original keeps in 32-bit data fields: the port stores load.c handles there (G: port design, as
   gfxload.c; a DOSBox-X dump differs in these by design while they are live, and all are 0 again at 0x11854):
     0xF7C00            the zoom map GIF (malloc 0x2FA34; free 0x2F93E)
     0xF7C08[9]         the candidates' ch_ GIFs (malloc 0x2FB25; free 0x2F7C7, 0x2F972)
     0xF7C40[9]         the candidates' cv_ GIFs (malloc 0x2FBC6; free 0x2F7D6, 0x2F9B0)
     +0 of the local sprite record [ebp-0x34] of 0x2EFA0 (f3AE90 0x2F05B, f3AF8C 0x2F29F, f3AF54 0x2F932): the ui.h
                        routines take link addresses, so the port keeps the record at CHOOSE_SAVE_SPR, in the stack range
                        0x121C00..0x122BFF that tools/verify/by_design.txt excludes (G: port design), with +0 cleared
                        before f3AE90 so a stale handle cannot make ui_sprite_put reuse another record's slot.
   [0xF522C] is read only (0x2F91B, 0x30116): ui_screen.c's ui_heap_F522C.

   Pictures from memory (fg_showgif(an empty string, 7): 4 = from the fg_imagebuf buffer, 2 = at the cursor, 1 = keep
   the palette). ui.h's f8240F only writes Fastgraph/Image's fields in the data object; fg.c's fg_showgif reads the
   buffer fg.h's fg_imagebuf was given. So every call of 0x8240F here goes through imagebuf() below, which does both.
   At 0x2F91B and 0x30116 the buffer is ui_heap_F522C, 0xFF00 again, so fg.c keeps no pointer to a freed card block.

   Not ported yet: 0x136B0 (the AST chart, 0x3012F) and 0x302C4 (the computer nations, 0x2F9DE): not_ported(addr).

   1995 bugs: none found in these four routines (no g_original branches). */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "fg.h"
#include "platform.h"

/* Defined in ui_screen.c, not yet in ui.h (title.c declares it the same way): the port's home of [0xF522C]. */
extern uint8_t *ui_heap_F522C;

/* Port (G): the link address that stands for 0x2EFA0's local sprite record [ebp-0x34] (8 bytes). */
#define CHOOSE_SAVE_SPR 0x121C00u

/* ---- Port helpers ------------------------------------------------------------------------------------------------ */

/* not_ported: a call into code that is not translated yet. Prints the original's address and ends the program. */
_Noreturn void not_ported(uint32_t addr)
{
    fprintf(stderr, "advciv: 0x%X is not ported yet\n", (unsigned)addr);
    plat_shutdown();
    exit(3);
}

/* 0x8240F fg_imagebuf(block, size) where block is a data field's heap handle: f8240F for Fastgraph/Image's fields in
   the data object, fg_imagebuf for the buffer the port's fg_showgif reads. Port guard (G): a handle with no block
   behind it (malloc failed) gets a 1-byte buffer, so fg_showgif draws nothing (fg.c returns 2) instead of reading
   through a null pointer as the original would. */
static void imagebuf(uint32_t block, uint32_t size)
{
    static uint8_t none[1];
    uint8_t *p = load_heap_ptr(block, size);

    f8240F(p, size);
    if (p == NULL && size != 0)
        fg_imagebuf(none, 1);
    else
        fg_imagebuf(p, size);
}

/* Nation n's card rectangle, int16 {x, y, w, h} at 0xC14E4 + 8n (the code reads dword [8n + 0xC14E2], [+0xC14E4],
   [+0xC14E6], [+0xC14E8] sar 16) */
static int32_t card_x(int32_t n) { return S16((uint32_t)(0xC14E4 + 8 * n)); }
static int32_t card_y(int32_t n) { return S16((uint32_t)(0xC14E6 + 8 * n)); }
static int32_t card_w(int32_t n) { return S16((uint32_t)(0xC14E8 + 8 * n)); }
static int32_t card_h(int32_t n) { return S16((uint32_t)(0xC14EA + 8 * n)); }

/* The candidate at index i of the choice list: int32 0xF7CA0[i] */
static int32_t cand(int32_t i) { return S32((uint32_t)(0xF7CA0 + 4 * i)); }

/* ---- 0x2EFA0: the nation choice --------------------------------------------------------------------------------- */

/* 0x2EFA0: the "Player %s choose your nation" screen (choose.gif) on the zoom map. Called from main 0x1184F. */
void f2EFA0(void)
{
    int32_t i;                  /* [ebp-0x2c] */
    int32_t p;                  /* [ebp-0x20] the player choosing, 1-based */
    int32_t idx;                /* [ebp-0x28] the candidate shown (passed by address to f79A9C) */
    int32_t choosing;           /* [ebp-0x18] 0 once this player has chosen */
    int32_t code;               /* [ebp-0x24] */
    uint32_t rec;               /* [ebp-0x1c] */
    int32_t n, x, y, humans;
    char text[100];             /* [ebp-0x98] .. [ebp-0x35] */

    setD16(0xBB3FAu, 0xA9);                                     /* 0x2EFD1 message area x 169 */
    setD16(0xBB3FCu, 0x154);                                    /* 0x2EFD8 y 340 */
    setD16(0xBB3FEu, 0x12C);                                    /* 0x2EFDF w 300 */
    setD16(0xBB400u, 0x23);                                     /* 0x2EFE6 h 35 */
    f3CD6C(6);                                                  /* 0x2EFED choose.gif into [0xF52CC] */
    setD8(0xEE168u, 6);                                         /* 0x2F017 */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(0xF52CCu), S32(0xF52C8u));   /* 0x2F01D background -> back buffer */

    fg_vbopen(S32(0xF52CCu));                                   /* 0x2F02C */
    setD32(CHOOSE_SAVE_SPR + UI_SPR_DATA, 0);                   /* port: no stale handle in the record (see above) */
    f3AE90(CHOOSE_SAVE_SPR, S16(0xBB3FAu), S16(0xBB3FCu),       /* 0x2F05B: dword [0xBB3F8], [0xBB3FA], [0xBB3FC], */
           S16(0xBB3FEu), S16(0xBB400u));                       /*          [0xBB3FE] sar 16: x, y, w, h (h pushed) */
    fg_vbclose();                                               /* 0x2F060 */

    fg_vbopen(S32(0xF52C8u));                                   /* 0x2F06C */
    if (S8(0xE1620u) == 0)                                      /* 0x2F071 dword [0xE161D] sar 24; test; jne */
        f3C9D0(0x109, 0x185, 0x6E, 0x13, 5);                    /* 0x2F096 the name box (only with no humans, D) */

    /* The list index v (0x2F09B..0x2F10B). The bit is bit 0x02 of word [0xE1624]: shl eax,0xE; shr ax,0xF; the
       compares use eax & 0xFFFF. */
    setD32(0xF7C78u, (uint32_t)(S32(0xF40F0u) - 2));            /* 0x2F0A3 */
    {
        uint32_t bit = ((uint32_t)D16(0xE1624u) >> 1) & 1u;     /* 0x2F0A8..0x2F0B7 */
        int32_t players = S32(0xF40F0u);                        /* 0x2F0B1 edi */

        if (players >= 6) {                                     /* 0x2F0BB cmp edi,6; jl */
            setD32(0xF7C78u, (uint32_t)(S32(0xF7C78u) + 2));    /* 0x2F0CE */
            if (bit == 1 && players == 6)                       /* 0x2F0D4, 0x2F0D9 */
                setD32(0xF7C78u, 9);                            /* 0x2F0DE */
        } else if (players >= 4) {                              /* 0x2F0EA cmp edi,4; jl */
            if (bit == 0)                                       /* 0x2F0EF and eax,0xFFFF; jne */
                setD32(0xF7C78u, D32(0xF7C78u) + 1);            /* 0x2F0F6 inc */
            if (S32(0xF40F0u) == 5)                             /* 0x2F0FC */
                setD32(0xF7C78u, D32(0xF7C78u) + 1);            /* 0x2F105 inc */
        }
    }
    /* 0x2F111..0x2F12B: record v of the ten 14-byte lists at 0xC0794 (lea [8v]; sub v; [2*eax + 0xC0794]) -> 0xF7B00,
       3 movsd + movsw */
    memmove(DP(0xF7B00u), DP((uint32_t)(0xC0794 + 14 * S32(0xF7C78u))), 14);
    f2FC04();                                                   /* 0x2F12D */
    for (i = 0; S32(0xF7B00u) - 1 > i; i++)                     /* 0x2F132, 0x2F13D dec; cmp; jle */
        setD8((uint32_t)(0xF7B04 + i), D8((uint32_t)(0xF7B05 + i)));   /* 0x2F14B, 0x2F154: drop the leading 0 */
    setD32(0xF7B00u, (uint32_t)(S32(0xF7B00u) - 1));            /* 0x2F16C dec, 0x2F172 */
    f3AF8C(0xF51ACu, 0xFB, 0x185);                              /* 0x2F178 the left arrow's picture */
    f3AF8C(0xF51B4u, 0x178, 0x185);                             /* 0x2F18C the right arrow's picture */
    fg_vbclose();                                               /* 0x2F191 */
    fg_setcolor(5);                                             /* 0x2F198 */
    f3B05C(0xC08D0u);                                           /* 0x2F1A5 the dialog: arrows, SELECT, AST */

    setD32(0xF7C9Cu, 0);                                        /* 0x2F1AC */
    for (i = 0; i < S32(0xF7B00u); i++) {                       /* 0x2F1B1, 0x2F1BF cmp; jge */
        int32_t c = S32(0xF7C9Cu) + 1;                          /* 0x2F1D0 inc */
        setD32((uint32_t)(0xF7C9C + 4 * c), (uint32_t)S8((uint32_t)(0xF7B04 + i)));   /* 0x2F1CA dword [i+0xF7B01] sar 24; 0x2F1D9 */
        setD32(0xF7C9Cu, (uint32_t)c);                          /* 0x2F1E0 */
    }

    f2F9F8();                                                   /* 0x2F1E7 the map and the cards */
    fg_settrans(0, 1);                                          /* 0x2F1F5 */
    p = 1;                                                      /* 0x2F1FD */
    idx = 0;                                                    /* 0x2F202 */
    fg_vbopen(S32(0xF52C8u));                                   /* 0x2F20B */
    fg_move(8, 6);                                              /* 0x2F217 */
    imagebuf(D32(0xF7C00u), D16(0xF7C04u));                     /* 0x2F22F fg_imagebuf([0xF7C00], word [0xF7C04]) */
    fg_showgif(DSTR(0xB378Cu), 7);                              /* 0x2F23E "" from memory, at the cursor, palette kept */
    fg_vbclose();                                               /* 0x2F246 */

    for (;;) {                                                  /* 0x2F24B */
        humans = S8(0xE1620u);                                  /* 0x2F24B dword [0xE161D] sar 24 */
        if (humans < p)                                         /* 0x2F253 cmp; jl */
            break;
        snprintf(text, sizeof text, DSTR(0xB378Du),             /* 0x2F273 sprintf "Player %s choose your nation" */
                 DSTR(D32((uint32_t)(0xC08AC + 4 * p))));       /* 0x2F25F "Zero".."Eight" */
        fg_vbopen(S32(0xF52C8u));                               /* 0x2F282 */
        f3AF8C(CHOOSE_SAVE_SPR, S16(0xBB3FAu), S16(0xBB3FCu));  /* 0x2F29F the saved message area back */
        fg_vbclose();                                           /* 0x2F2A4 */
        f3AB7C(text);                                           /* 0x2F2B4 */
        choosing = 1;                                           /* 0x2F2B9 */

        while (choosing != 0) {                                 /* 0x2F2BC cmp 0; je 0x2F24B */
            /* The ch_ card of the candidate idx into the background buffer, then copied with colour 0 transparent
               into the back buffer */
            fg_vbopen(S32(0xF52CCu));                           /* 0x2F2C9 */
            n = cand(idx);
            fg_move(card_x(n) + 8, card_y(n) + 6);              /* 0x2F2D1..0x2F301 */
            imagebuf(D32((uint32_t)(0xF7C08 + 4 * idx)),        /* 0x2F325 fg_imagebuf(0xF7C08[idx], */
                     D16((uint32_t)(0xF7C2C + 2 * idx)));       /*         word 0xF7C2C[idx]) */
            fg_showgif(DSTR(0xB37AAu), 7);                      /* 0x2F334 */
            fg_vbclose();                                       /* 0x2F33C */
            setD8(0xF52D0u, 0);                                 /* 0x2F34F the background is no longer choose.gif alone */
            n = cand(idx);
            fg_vbtccopy(card_x(n) + 8, card_x(n) + 8 + card_w(n) - 1,       /* 0x2F42C: minx, maxx, */
                        card_y(n) + 6, card_y(n) + 6 + card_h(n) - 1,       /*   miny, maxy, */
                        card_x(n) + 8, card_y(n) + 6 + card_h(n) - 1,       /*   x, y (lower-left), */
                        S32(0xF52CCu), S32(0xF52C8u));                      /*   source 0x2F349, dest 0x2F341 */

            fg_vbopen(S32(0xF52C8u));                           /* 0x2F43B */
            for (i = 1; i <= 9; i++)                            /* 0x2F448, 0x2F453 cmp 9; jg */
                if (D8((uint32_t)(i * 0xC7 + 0xF1566)) != 0)    /* 0x2F459 imul 0xC7; 0x2F460 byte +0x9A; test dh,dh */
                    f3AF8C((uint32_t)(i * 8 + 0xF4A5C),         /* 0x2F48F the 35x35 token of a nation taken */
                           S16((uint32_t)(0xC076C + 2 * i)),    /* 0x2F47A dword [2i + 0xC076A] sar 16 */
                           S16((uint32_t)(0xC0780 + 2 * i)));   /* 0x2F46D dword [2i + 0xC077E] sar 16 */
            f3C9D0(0x109, 0x185, 0x6E, 0x13, 5);                /* 0x2F4AC the sunken name box */
            fg_setcolor(5);                                     /* 0x2F4B3 */
            f3A0B0(0x109, 0x189, 0x6E, DSTR(D32((uint32_t)(0xC768C + 4 * cand(idx)))));   /* 0x2F4DB the nation's name */
            fg_vbclose();                                       /* 0x2F4E0 */
            f3CC14();                                           /* 0x2F4E5 */
            f775F0();                                           /* 0x2F4EA no button down */
            while (S32(0xE1630u) == 0) {                        /* 0x2F4EF cmp dword [0xE1630],0; jne */
                int mx, my, mb;
                fg_mousepos(&mx, &my, &mb);                     /* 0x2F507 fg_mousepos(0xE1628, 0xE162C, 0xE1630) */
                setD32(0xE1628u, (uint32_t)mx);
                setD32(0xE162Cu, (uint32_t)my);
                setD32(0xE1630u, (uint32_t)mb);
            }

            /* Hit test of the dialog 0xC08D0 by hand (as the setup screen does) */
            code = -100;                                        /* 0x2F511 */
            for (i = 0; S16(0xC08D0u) > i; i++) {               /* 0x2F525 dword [0xC08CE] sar 16; cmp; jle */
                rec = (uint32_t)(0xC08D2 + 20 * i);             /* 0x2F539..0x2F54C */
                if (S8(rec + BTN_ENABLED) != 1)                 /* 0x2F552 dword [rec+0xD] sar 24; cmp 1; jne */
                    continue;
                x = S16(rec + BTN_X);                           /* 0x2F566 movsx word [rec] */
                if (x > S32(0xE1628u))                          /* 0x2F569 cmp eax,ecx; jg */
                    continue;
                if (x + S16(rec + BTN_W) <= S32(0xE1628u))      /* 0x2F570 dword [rec+2] sar 16; 0x2F578 jle */
                    continue;
                y = S16(rec + BTN_Y);                           /* 0x2F57F dword [rec] sar 16 */
                if (y > S32(0xE162Cu))                          /* 0x2F58A cmp eax,edi; jg */
                    continue;
                if (y + S16(rec + BTN_H) <= S32(0xE162Cu))      /* 0x2F591 dword [rec+4] sar 16; 0x2F599 jle */
                    continue;
                f3C5D4(0xC08D0u, S16(rec + BTN_ID));            /* 0x2F5AB dword [rec+0xC] sar 16 */
                code = f300E0(S16(rec + BTN_ID));               /* 0x2F5B9, 0x2F5BE */
                break;                                          /* falls to 0x2F5C1 */
            }

            if (code == -100)                                   /* 0x2F5C1 cmp -0x64; je 0x2F2BC */
                continue;
            if ((uint32_t)code < (uint32_t)-10) {               /* 0x2F5CE cmp eax,-0xA; jb 0x2F88F */
                if (code != -16)                                /* 0x2F88F cmp eax,-0x10; je 0x2F713 */
                    continue;                                   /* 0x2F898 */

                /* SELECT (0x2F713): player p takes the candidate idx */
                setD8((uint32_t)(0xF40E4 + (p << 5)), (uint8_t)cand(idx));   /* 0x2F71F shl edx,5; 0x2F72A byte */
                fg_vbopen(S32(0xF52C8u));                       /* 0x2F730 */
                n = cand(idx);
                fg_move(card_x(n) + 8, card_y(n) + 6);          /* 0x2F768 */
                imagebuf(D32((uint32_t)(0xF7C40 + 4 * idx)),    /* 0x2F78C fg_imagebuf(0xF7C40[idx], */
                         D16((uint32_t)(0xF7C64 + 2 * idx)));   /*         word 0xF7C64[idx]) */
                fg_showgif(DSTR(0xB37ADu), 7);                  /* 0x2F79B the cv_ card back */
                fg_vbclose();                                   /* 0x2F7A3 */
                setD8((uint32_t)(cand(idx) * 0xC7 + 0xF1566), 3);   /* 0x2F7AB imul; 0x2F7B6 nation +0x9A = 3 (human) */
                load_heap_free(D32((uint32_t)(0xF7C08 + 4 * idx)));   /* 0x2F7C7 free(0xF7C08[idx]) */
                load_heap_free(D32((uint32_t)(0xF7C40 + 4 * idx)));   /* 0x2F7D6 free(0xF7C40[idx]) */
                p++;                                            /* 0x2F7DE */
                for (i = idx; S32(0xF7C9Cu) - 1 > i; i++) {     /* 0x2F7E4, 0x2F7EF dec; cmp; jle */
                    setD32((uint32_t)(0xF7CA0 + 4 * i), D32((uint32_t)(0xF7CA4 + 4 * i)));   /* 0x2F807 */
                    setD32((uint32_t)(0xF7C08 + 4 * i), D32((uint32_t)(0xF7C0C + 4 * i)));   /* 0x2F81B */
                    setD16((uint32_t)(0xF7C2C + 2 * i), D16((uint32_t)(0xF7C2E + 2 * i)));   /* 0x2F830 */
                    setD32((uint32_t)(0xF7C40 + 4 * i), D32((uint32_t)(0xF7C44 + 4 * i)));   /* 0x2F845 */
                    setD16((uint32_t)(0xF7C64 + 2 * i), D16((uint32_t)(0xF7C66 + 2 * i)));   /* 0x2F85A */
                }
                setD32(0xF7C9Cu, (uint32_t)(S32(0xF7C9Cu) - 1));   /* 0x2F864 dec */
                f79A9C(&idx);                                   /* 0x2F86D clamp idx to 0..count-1 */
                setD32((uint32_t)(0xF7C08 + 4 * S32(0xF7C9Cu)), 0);   /* 0x2F879 */
                setD32((uint32_t)(0xF7C40 + 4 * S32(0xF7C9Cu)), 0);   /* 0x2F880 */
                choosing = 0;                                   /* 0x2F887 */
                continue;                                       /* 0x2F88A */
            }
            if (code == -10) {                                  /* 0x2F5D7 jbe 0x2F67A: the right arrow */
                if (S32(0xF7C9Cu) - 1 <= idx)                   /* 0x2F67F dec; cmp; jle */
                    continue;
                fg_vbopen(S32(0xF52C8u));                       /* 0x2F690 */
                n = cand(idx);
                fg_move(card_x(n) + 8, card_y(n) + 6);          /* 0x2F6C8 */
                imagebuf(D32((uint32_t)(0xF7C40 + 4 * idx)),    /* 0x2F6EC */
                         D16((uint32_t)(0xF7C64 + 2 * idx)));
                fg_showgif(DSTR(0xB37ACu), 7);                  /* 0x2F6FB the cv_ card back */
                fg_vbclose();                                   /* 0x2F703 */
                idx++;                                          /* 0x2F70B */
                continue;
            }
            if (code != -9)                                     /* 0x2F5DD cmp eax,-9; jne 0x2F2BC */
                continue;
            /* the left arrow */
            if (idx <= 0)                                       /* 0x2F5E6 cmp 0; jle */
                continue;
            fg_vbopen(S32(0xF52C8u));                           /* 0x2F5F7 */
            n = cand(idx);
            fg_move(card_x(n) + 8, card_y(n) + 6);              /* 0x2F62F */
            imagebuf(D32((uint32_t)(0xF7C40 + 4 * idx)),        /* 0x2F653 */
                     D16((uint32_t)(0xF7C64 + 2 * idx)));
            fg_showgif(DSTR(0xB37ABu), 7);                      /* 0x2F662 the cv_ card back */
            fg_vbclose();                                       /* 0x2F66A */
            idx--;                                              /* 0x2F672 */
        }
    }

    if (humans > 0) {                                           /* 0x2F89D test eax,eax; jle (eax from 0x2F250) */
        fg_vbopen(S32(0xF52C8u));                               /* 0x2F8B1 */
        for (i = 1; i <= 9; i++)                                /* 0x2F8B9, 0x2F8C4 cmp 9; jg */
            if (D8((uint32_t)(i * 0xC7 + 0xF1566)) != 0)        /* 0x2F8CA, 0x2F8D1 */
                f3AF8C((uint32_t)(i * 8 + 0xF4A5C),             /* 0x2F900 */
                       S16((uint32_t)(0xC076C + 2 * i)),        /* 0x2F8EB */
                       S16((uint32_t)(0xC0780 + 2 * i)));       /* 0x2F8DE */
        fg_vbclose();                                           /* 0x2F907 */
        f3CC14();                                               /* 0x2F90C */
        f78A20(0x2EE);                                          /* 0x2F916 wait 750 ms */
    }

    /* Clean-up (0x2F91B) */
    f8240F(ui_heap_F522C, 0xFF00);                              /* 0x2F927 fg_imagebuf([0xF522C], 0xFF00) */
    fg_imagebuf(ui_heap_F522C, 0xFF00);                         /* port: fg.c's buffer too (see the file comment) */
    f3AF54(CHOOSE_SAVE_SPR);                                    /* 0x2F932 */
    load_heap_free(D32(0xF7C00u));                              /* 0x2F93E free([0xF7C00]) */
    setD32(0xF7C00u, 0);                                        /* 0x2F943 */
    for (i = 0; i < 9; i++)                                     /* 0x2F949, 0x2F954 cmp 9; jge */
        if (D32((uint32_t)(0xF7C08 + 4 * i)) != 0) {            /* 0x2F95D test; je */
            load_heap_free(D32((uint32_t)(0xF7C08 + 4 * i)));   /* 0x2F972 */
            setD32((uint32_t)(0xF7C08 + 4 * i), 0);             /* 0x2F97C */
        }
    for (i = 0; i < 9; i++)                                     /* 0x2F987, 0x2F992 */
        if (D32((uint32_t)(0xF7C40 + 4 * i)) != 0) {            /* 0x2F99B */
            load_heap_free(D32((uint32_t)(0xF7C40 + 4 * i)));   /* 0x2F9B0 */
            setD32((uint32_t)(0xF7C40 + 4 * i), 0);             /* 0x2F9BA */
        }

    if (S8(0xE1621u) > 0) {                                     /* 0x2F9C3 dword [0xE161E] sar 24; test; jle */
        f3CC14();                                               /* 0x2F9CF */
        f3C940(0xC08D0u);                                       /* 0x2F9D9 */
        not_ported(0x302C4u);                                   /* 0x2F9DE call 0x302C4: the computer nations */
        f3C7A0(0xC08D0u);                                       /* 0x2F9E8 (reached once 0x302C4 is ported) */
    }
}

/* ---- 0x2F9F8: the map and the cards from ACIV.CT1 --------------------------------------------------------------- */

/* 0x2F9F8: load the zoom map for [0xF40F0] players into [0xF7C00] (size word 0xF7C04), zero the card slots, then each
   candidate's "<ch_ name>.gif" into 0xF7C08[i] and "<cv_ name>.gif" into 0xF7C40[i] (sizes 0xF7C2C[i], 0xF7C64[i]).
   Each size is cat_size's low word, and malloc gets that word (and eax,0xFFFF). One caller, 0x2EFA0 (0x2F1E7). */
void f2F9F8(void)
{
    int32_t i;                  /* [ebp-0x18] */
    int32_t n;
    long size;
    uint32_t h;
    uint8_t *buf;
    char name[0x34];            /* [ebp-0x4c] .. [ebp-0x19] */

    size = cat_size(DSTR(D32((uint32_t)(0xC1A38 + 4 * S32(0xF40F0u)))), S32(0xF40D4u));   /* 0x2FA21 zoomN.gif, CT1 */
    setD16(0xF7C04u, (uint16_t)size);                           /* 0x2FA29 mov word [0xF7C04], ax */
    h = load_heap_alloc((uint32_t)size & 0xFFFFu);              /* 0x2FA2F and eax,0xFFFF; 0x2FA34 malloc */
    setD32(0xF7C00u, h);                                        /* 0x2FA40 */
    /* Port guard (G): cat_load writes the entry's whole length, so it gets the block only when that length fits (a
       failed malloc, or a cat_size error -1/-2, would make the original write through NULL or past the block; the
       shipped CT1 entries all fit in 16 bits). */
    buf = load_heap_ptr(h, (uint32_t)size);
    if (buf != NULL)
        cat_load(DSTR(D32((uint32_t)(0xC1A38 + 4 * S32(0xF40F0u)))), buf, S32(0xF40D4u));   /* 0x2FA55 */

    for (i = 0; i < 9; i++) {                                   /* 0x2FA5D, 0x2FA68 cmp 9; jge */
        setD32((uint32_t)(0xF7C08 + 4 * i), 0);                 /* 0x2FA73 */
        setD16((uint32_t)(0xF7C2C + 2 * i), 0);                 /* 0x2FA7F */
    }
    for (i = 0; i < 9; i++) {                                   /* 0x2FA8B, 0x2FA96 */
        setD32((uint32_t)(0xF7C40 + 4 * i), 0);                 /* 0x2FAA1 */
        setD16((uint32_t)(0xF7C64 + 2 * i), 0);                 /* 0x2FAAD */
    }

    for (i = 0; i < S32(0xF7B00u); i++) {                       /* 0x2FAB9, 0x2FAC7 cmp; jge */
        n = S8((uint32_t)(0xF7B04 + i));                        /* 0x2FAD6 dword [i + 0xF7B01] sar 24 */
        snprintf(name, sizeof name, DSTR(0xB37AEu), DSTR(D32((uint32_t)(0xC1494 + 4 * n))));   /* 0x2FAF0 "%s.gif" */
        size = cat_size(name, S32(0xF40D4u));                   /* 0x2FB02 */
        setD16((uint32_t)(0xF7C2C + 2 * i), (uint16_t)size);    /* 0x2FB0D */
        h = load_heap_alloc(D16((uint32_t)(0xF7C2C + 2 * i)));  /* 0x2FB18 word; 0x2FB25 malloc */
        setD32((uint32_t)(0xF7C08 + 4 * i), h);                 /* 0x2FB2D */
        buf = load_heap_ptr(D32((uint32_t)(0xF7C08 + 4 * i)), (uint32_t)size);   /* port guard, as above */
        if (buf != NULL)
            cat_load(name, buf, S32(0xF40D4u));                 /* 0x2FB4A */
    }

    for (i = 0; i < S32(0xF7B00u); i++) {                       /* 0x2FB59, 0x2FB67 */
        n = S8((uint32_t)(0xF7B04 + i));                        /* 0x2FB76 */
        snprintf(name, sizeof name, DSTR(0xB37B5u), DSTR(D32((uint32_t)(0xC14BC + 4 * n))));   /* 0x2FB90 "%s.gif" */
        size = cat_size(name, S32(0xF40D4u));                   /* 0x2FBA3 */
        setD16((uint32_t)(0xF7C64 + 2 * i), (uint16_t)size);    /* 0x2FBAE */
        h = load_heap_alloc(D16((uint32_t)(0xF7C64 + 2 * i)));  /* 0x2FBB9; 0x2FBC6 malloc */
        setD32((uint32_t)(0xF7C40 + 4 * i), h);                 /* 0x2FBD4 */
        buf = load_heap_ptr(D32((uint32_t)(0xF7C40 + 4 * i)), (uint32_t)size);   /* port guard, as above */
        if (buf != NULL)
            cat_load(name, buf, S32(0xF40D4u));                 /* 0x2FBEB */
    }
}

/* ---- 0x2FC04: picture names and card rectangles for the player count ------------------------------------------- */

/* The inline strcpy loop of 0x2FC04 (two bytes a step, stopping at the 0, which is copied): src (a literal in the
   data) into the literal the pointer at field points to, both in g_ds. Each new name fits the one it replaces (D,
   obj2: the literals 0xB39CC.. and 0xB3A1B.. are as long as the longest name written into them). */
static void copy_name(uint32_t field, uint32_t src)
{
    uint8_t *d = DP(D32(field));
    const uint8_t *s = DP(src);
    uint8_t c;

    do {
        c = *s++;
        *d++ = c;
    } while (c != 0);
}

/* 0x2FC04: the ch_/cv_ picture names (by strcpy into the literals behind 0xC1494[n] / 0xC14BC[n]) and the card
   rectangles of nations 1, 2, 7 and 9 (int16 x, y, w, h at 0xC14E4 + 8n) for [0xF40F0] players. One caller, 0x2EFA0
   (0x2F12D). The stores are in the original's order. */
void f2FC04(void)
{
    if (S32(0xF40F0u) == 3) {                                   /* 0x2FC16 cmp 3; jne 0x2FD65 */
        copy_name(0xC1498u, 0xB37BCu);                          /* 0x2FC39 "ch_af3" -> ch_ slot 1 */
        copy_name(0xC14C0u, 0xB37C3u);                          /* 0x2FC63 "cv_af3" -> cv_ slot 1 */
        setD16(0xC14ECu, 0x8F);                                 /* 0x2FC87 Africa x 143 */
        setD16(0xC14EEu, 0xC2);                                 /* 0x2FC8E y 194 */
        setD16(0xC14F2u, 0x7D);                                 /* 0x2FC95 h 125 */
        setD16(0xC14F0u, 0xBA);                                 /* 0x2FCA1 w 186 */
        copy_name(0xC149Cu, 0xB37CAu);                          /* 0x2FCA9 "ch_italy" -> ch_ slot 2 */
        copy_name(0xC14C4u, 0xB37D3u);                          /* 0x2FCCE "cv_italy" -> cv_ slot 2 */
        copy_name(0xC14C8u, 0xB37DCu);                          /* 0x2FCF8 "cv_ill3" -> cv_ slot 3 */
        copy_name(0xC14D0u, 0xB37E4u);                          /* 0x2FD22 "cv_cret3" -> cv_ slot 5 */
        setD16(0xC14F8u, 0x73);                                 /* 0x2FD45 nation 2 w 115 */
        setD16(0xC14FAu, 0x5C);                                 /* 0x2FD4C h 92 */
        setD16(0xC14F4u, 0x93);                                 /* 0x2FD53 x 147 */
        setD16(0xC14F6u, 0x17);                                 /* 0x2FD5A y 23 */
    } else {
        copy_name(0xC1498u, 0xB37EDu);                          /* 0x2FD71 "ch_af48" */
        copy_name(0xC14C0u, 0xB37F5u);                          /* 0x2FD9B "cv_af48" */
        setD16(0xC14ECu, 0x79);                                 /* 0x2FDC3 Africa x 121 */
        setD16(0xC14EEu, 0xC4);                                 /* 0x2FDCA y 196 */
        setD16(0xC14F0u, 0xD0);                                 /* 0x2FDD1 w 208 */
        setD16(0xC14F2u, 0x7B);                                 /* 0x2FDE3 h 123 */
        copy_name(0xC149Cu, 0xB37FDu);                          /* 0x2FDEA "ch_ibera" */
        copy_name(0xC14C4u, 0xB3806u);                          /* 0x2FE14 "cv_ibera" */
        copy_name(0xC14C8u, 0xB380Fu);                          /* 0x2FE3B "cv_illy" */
        copy_name(0xC14D0u, 0xB3817u);                          /* 0x2FE65 "cv_crete" */
        setD16(0xC14F4u, 0);                                    /* 0x2FE83 Iberia x 0 */
        setD16(0xC14F6u, 0x5B);                                 /* 0x2FE8A y 91 */
        setD16(0xC14F8u, 0x41);                                 /* 0x2FE91 w 65 */
        setD16(0xC14FAu, 0x87);                                 /* 0x2FE98 h 135 */
    }

    if (S32(0xF40F0u) == 4)                                     /* 0x2FE9F cmp 4; jne */
        copy_name(0xC14D0u, 0xB3820u);                          /* 0x2FEB4 "cv_cret3" -> cv_ slot 5 */

    if (S32(0xF40F0u) == 5 || S32(0xF40F0u) == 6) {             /* 0x2FED3, 0x2FED8 */
        copy_name(0xC14B0u, 0xB3829u);                          /* 0x2FEEE "ch_ass56" -> ch_ slot 7 */
        copy_name(0xC14D8u, 0xB3832u);                          /* 0x2FF18 "cv_ass56" -> cv_ slot 7 */
        setD16(0xC1520u, 0x37);                                 /* 0x2FF36 Assyria w 55 */
        setD16(0xC1522u, 0x24);                                 /* 0x2FF3D h 36 */
        setD16(0xC151Cu, 0x1CB);                                /* 0x2FF46 x 459 */
        setD16(0xC151Eu, 0);                                    /* 0x2FF4D y 0 */
    } else {
        copy_name(0xC14B0u, 0xB383Bu);                          /* 0x2FF61 "ch_ass78" */
        copy_name(0xC14D8u, 0xB3844u);                          /* 0x2FF8B "cv_ass78" */
        setD16(0xC151Cu, 0x1CB);                                /* 0x2FFB0 x 459 */
        setD16(0xC151Eu, 0);                                    /* 0x2FFB7 y 0 */
        setD16(0xC1520u, 0x89);                                 /* 0x2FFBE w 137 */
        setD16(0xC1522u, 0x24);                                 /* 0x2FFC5 h 36 */
    }

    if (S32(0xF40F0u) == 6) {                                   /* 0x2FFCB cmp 6; jne 0x3005C */
        copy_name(0xC14B8u, 0xB384Du);                          /* 0x2FFEE "ch_egy6" -> ch_ slot 9 */
        copy_name(0xC14E0u, 0xB3855u);                          /* 0x30018 "cv_egy6" -> cv_ slot 9 */
        setD16(0xC152Cu, 0x13B);                                /* 0x30036 Egypt x 315 */
        setD16(0xC152Eu, 0x105);                                /* 0x3003D y 261 */
        setD16(0xC1530u, 0xC8);                                 /* 0x30044 w 200 */
        setD16(0xC1532u, 0x3A);                                 /* 0x3004B h 58 */
    } else {
        copy_name(0xC14B8u, 0xB385Du);                          /* 0x3006D "ch_egy78" */
        copy_name(0xC14E0u, 0xB3866u);                          /* 0x30097 "cv_egy78" */
        setD16(0xC1530u, 0xF9);                                 /* 0x300BA w 249 */
        setD16(0xC1532u, 0x3A);                                 /* 0x300C1 h 58 */
        setD16(0xC152Cu, 0x13B);                                /* 0x300C8 x 315 */
        setD16(0xC152Eu, 0x105);                                /* 0x300CF y 261 */
    }
}

/* ---- 0x300E0: the choice dialog's button id -> code ------------------------------------------------------------- */

/* 0x300E0: 16-bit unsigned compares of ax: 0x39 -> -9 (left arrow), 0x3A -> -10 (right arrow), 0x58 -> -16 (SELECT);
   3 (AST): the AST chart in mode 1, then the back buffer and choose.gif again, -100; anything else -100. Its caller
   0x2EFA0 passes the record's int16 id (sar 16) and uses eax. */
int f300E0(int id)
{
    uint16_t ax = (uint16_t)id;                                 /* [ebp-0x10] = eax; 0x300FD mov eax; cmp ax */
    int32_t code = -100;                                        /* 0x300F6 [ebp-0x14] = -0x64 */

    if (ax < 0x39) {                                            /* 0x30100 cmp ax,0x39; jb 0x30160 */
        if (ax == 3) {                                          /* 0x30160 cmp ax,3; je 0x30116 */
            f8240F(ui_heap_F522C, 0xFF00);                      /* 0x30122 fg_imagebuf([0xF522C], 0xFF00) */
            fg_imagebuf(ui_heap_F522C, 0xFF00);                 /* port: fg.c's buffer too */
            f136B0(1);                                          /* 0x3012F the AST chart, mode 1 (ast.c); eax unused */
            f3CC14();                                           /* 0x30134 (reached once 0x136B0 is ported) */
            f3CD6C(6);                                          /* 0x3013E choose.gif */
        }
    } else if (ax == 0x39) {                                    /* 0x30106 jbe 0x30145 */
        code = -9;                                              /* 0x30145 */
    } else if (ax <= 0x3A) {                                    /* 0x30108 cmp ax,0x3A; jbe 0x3014E */
        code = -10;                                             /* 0x3014E */
    } else if (ax == 0x58) {                                    /* 0x3010E cmp ax,0x58; je 0x30157 */
        code = -16;                                             /* 0x30157 */
    }
    return code;                                                /* 0x30166 */
}
