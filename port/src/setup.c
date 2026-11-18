/* The "New Game Options" screen of AC_MAIN.EXE (0x4DD50) and its click handler (0x4E2D8). Literal translations; the
   address of each instruction a line comes from is in its comment. D = read in the code, G = inference. Prototypes,
   the setup data and the dialog layouts: ui.h ("New-game setup").

   Play-by-email is dropped (Erik's ruling, OVERVIEW §7.5): the screen's "Play-by-email" label (0x4DE96..0x4DEA8) and
   its box sprite (0x4DEDF..0x4DF08) are not drawn, and a click on the PBEM row (button 0x59, the toggle at
   0x4E355..0x4E581) changes nothing and returns -100, as an unknown id does. The row's button record stays in the
   template (style 2: no frame, no label), so the press feedback f3C5D4 draws nothing visible there (G from f3B0D4 /
   f3B2D8, which draw a style-2 button as nothing). */

#include <stdio.h>
#include "ui.h"
#include "fg.h"

/* The AI-level name: word [0xE1624] shl 4, shr 14 = bits 10..11 (0xE1625 bits 2..3), names at 0xC4040 (D). */
static uint32_t ai_level(void)
{
    return (uint32_t)(((uint16_t)(D16(UI_SETUP_FLAGS) << 4)) >> 14);   /* shl eax,4; shr ax,0xE; and eax,0xFFFF */
}

static const char *ai_name(void)
{
    return DSTR(D32(UI_SETUP_AI_NAMES + 4u * ai_level()));            /* dword [4*eax+0xC4040] */
}

/* Watcom itoa(value, buf, 10) (0x8C897, D) -> snprintf "%d" */
static void itoa10(int v, char *buf, size_t size)
{
    snprintf(buf, size, "%d", v);
}

/* 0x4DD50: the setup screen; returns -1 START GAME, -0xF CANCEL, -0x82 VARIANTS */
int f4DD50(void)
{
    char buf[0x64];          /* [ebp-0x84] */
    int32_t code;            /* [ebp-0x18] */
    int32_t i;               /* [ebp-0x20] */
    int32_t total;

    f3CD6C(7);                                                                  /* 0x4DD6A: partha.gif into [0xF52CC] */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(0xF52CCu), S32(0xF52C8u));      /* 0x4DD92 -> the back buffer */
    fg_vbopen(S32(0xF52C8u));                                                   /* 0x4DDA1 */
    f3A18C(0, 0x28, 0x280, 0x45, DSTR(0xB4744u));                               /* 0x4DDBF "New Game Options" */
    fg_setcolor(0xF);                                                           /* 0x4DDC6 */
    f39F90(2);                                                                  /* 0x4DDDD */
    f3A044(0x7D, 0x64, DSTR(0xB4755u));                                         /* 0x4DDE7 "Number of Human Players" */
    f3A044(0x7D, 0x7D, DSTR(0xB476Du));                                         /* 0x4DDF8 "Number of Computer Players" */
    fg_vbclose();                                                               /* 0x4DDFD */

    if (S8(UI_SETUP_COMPUTERS) == 0) {                                          /* 0x4DE02 dword [0xE161E] sar 24 */
        f3C3D0(UI_DIALOG_SETUP, 0x68);                                          /* 0x4DE18: AI level buttons off */
        f3C3D0(UI_DIALOG_SETUP, 0x67);                                          /* 0x4DE27 */
        fg_setcolor(7);                                                         /* 0x4DE2C push 7 */
    } else {
        f3C314(UI_DIALOG_SETUP, 0x68);                                          /* 0x4DE3A */
        f3C314(UI_DIALOG_SETUP, 0x67);                                          /* 0x4DE49 */
        fg_setcolor(0xF);                                                       /* 0x4DE4E push 0xF */
    }                                                                           /* 0x4DE50 fg_setcolor */
    fg_vbopen(S32(0xF52C8u));                                                   /* 0x4DE5F */
    f3A044(0x7D, 0xE1, ai_name());                                              /* 0x4DE8A: the AI level */
    fg_setcolor(0xF);                                                           /* 0x4DE91 */
    /* 0x4DE96..0x4DEA8 "Play-by-email" at (0x7D, 0x96): PBEM, dropped */
    fg_setcolor(0xF);                                                           /* 0x4DEAF */
    f3A044(0x7D, 0xAF, DSTR(0xB4796u));                                         /* 0x4DEC6 "# of Trading Sessions" */
    f3A044(0x7D, 0xC8, DSTR(0xB47ACu));                                         /* 0x4DEDA "Minimum Goods Before Trading Allowed" */
    /* 0x4DEDF..0x4DF08 the PBEM box: f3AF8C(flags & 0x80 ? 0xF51CC : 0xF51D4, 0x58, 0x98): PBEM, dropped */
    f3C9D0(0x56, 0x63, 0x12, 0x13, 5);                                          /* 0x4DF23: humans' box */
    f3C9D0(0x56, 0x7C, 0x12, 0x13, 5);                                          /* 0x4DF3E: computers' box */
    fg_setcolor(5);                                                             /* 0x4DF45 */
    itoa10(S8(UI_SETUP_HUMANS), buf, sizeof buf);                               /* 0x4DF66 itoa(dword [0xE161D] sar 24) */
    f3A0B0(0x56, 0x64, 0x12, buf);                                              /* 0x4DF7A (the loop draws at 0x57) */
    itoa10(S8(UI_SETUP_COMPUTERS), buf, sizeof buf);                            /* 0x4DF98 */
    f3A0B0(0x56, 0x7D, 0x12, buf);                                              /* 0x4DFAC */
    f3B05C(UI_DIALOG_SETUP);                                                    /* 0x4DFB6: the buttons (it closes the buffer) */
    f39F90(2);                                                                  /* 0x4DFC0 */
    fg_vbclose();                                                               /* 0x4DFC5: nothing open any more */
    f3CC14();                                                                   /* 0x4DFCA: on the screen */

    for (;;) {                                                                  /* 0x4DFCF */
        fg_vbopen(S32(0xF52C8u));                                               /* 0x4DFD6 */
        f3C9D0(0x56, 0x63, 0x12, 0x13, 5);                                      /* 0x4DFF4 */
        itoa10(S8(UI_SETUP_HUMANS), buf, sizeof buf);                           /* 0x4E00C */
        fg_setcolor(5);                                                         /* 0x4E013 */
        f3A0B0(0x57, 0x64, 0x12, buf);                                          /* 0x4E030 */
        f3C9D0(0x56, 0x7C, 0x12, 0x13, 5);                                      /* 0x4E04B */
        itoa10(S8(UI_SETUP_COMPUTERS), buf, sizeof buf);                        /* 0x4E063 */
        fg_setcolor(f3BF30(UI_DIALOG_SETUP, 0x62) == 1 ? 5 : 0x17);             /* 0x4E072..0x4E089: grey while '+' is off (PBEM only) */
        f3A0B0(0x57, 0x7D, 0x12, buf);                                          /* 0x4E0A6 */
        f3C9D0(0x56, 0xAE, 0x12, 0x13, 5);                                      /* 0x4E0C1 */
        itoa10(D8(UI_SETUP_TRADE) & 0xF, buf, sizeof buf);                      /* 0x4E0DD: trading sessions */
        fg_setcolor(5);                                                         /* 0x4E0E4 */
        f3A0B0(0x57, 0xAF, 0x12, buf);                                          /* 0x4E101 */
        f3C9D0(0x56, 0xC7, 0x12, 0x13, 5);                                      /* 0x4E11C */
        itoa10(D8(UI_SETUP_TRADE) >> 4, buf, sizeof buf);                       /* 0x4E139: minimum goods */
        fg_setcolor(5);                                                         /* 0x4E140 */
        f3A0B0(0x57, 0xC8, 0x12, buf);                                          /* 0x4E15D */
        fg_vbclose();                                                           /* 0x4E162 */
        f3CC14();                                                               /* 0x4E167 */
        f775F0();                                                               /* 0x4E16C: no button down */
        while (S32(0xE1630u) == 0) {                                            /* 0x4E171: wait for a click */
            int x, y, b;
            fg_mousepos(&x, &y, &b);                                            /* 0x4E189 fg_mousepos(0xE1628, 0xE162C, 0xE1630) */
            setD32(0xE1628u, (uint32_t)x);
            setD32(0xE162Cu, (uint32_t)y);
            setD32(0xE1630u, (uint32_t)b);
        }

        code = -100;                                                            /* 0x4E193 */
        for (i = 0; i < S16(UI_DIALOG_SETUP + DLG_COUNT); i++) {                /* 0x4E1A7 dword [0xC28B6] sar 16 */
            uint32_t rec = UI_DIALOG_SETUP + DLG_REC(i);                        /* 0x4E1C4: 0xC28BA + 20*i */
            int32_t x, y;
            if (S8(rec + BTN_ENABLED) != 1)                                     /* 0x4E1D4 dword [rec+0xD] sar 24 */
                continue;
            x = S16(rec + BTN_X);                                               /* 0x4E1E8 */
            if (x > S32(0xE1628u))                                              /* 0x4E1ED jg */
                continue;
            if (x + S16(rec + BTN_W) <= S32(0xE1628u))                          /* 0x4E1F2..0x4E1FC jle */
                continue;
            y = S16(rec + BTN_Y);                                               /* 0x4E201 dword [rec] sar 16 */
            if (y > S32(0xE162Cu))                                              /* 0x4E20E jg */
                continue;
            if (y + S16(rec + BTN_H) <= S32(0xE162Cu))                          /* 0x4E213..0x4E21D jle */
                continue;
            f3C5D4(UI_DIALOG_SETUP, S16(rec + BTN_ID));                         /* 0x4E22D: the press, until let go */
            code = f4E2D8(S16(rec + BTN_ID));                                   /* 0x4E23B */
            break;                                                              /* 0x4E240 falls to 0x4E243 */
        }

        if (code == -100)                                                       /* 0x4E243 */
            continue;
        if (code == -0xF || code == -0x82)                                      /* 0x4E250, 0x4E259: CANCEL, VARIANTS */
            return code;
        if (code != -1)                                                         /* 0x4E265 */
            continue;

        /* START GAME (0x4E26E) */
        total = S8(UI_SETUP_HUMANS) + S8(UI_SETUP_COMPUTERS);                   /* 0x4E26E..0x4E27F */
        setD32(UI_PLAYERS_A, (uint32_t)total);                                  /* 0x4E281 */
        setD32(UI_PLAYERS_B, (uint32_t)total);                                  /* 0x4E286 */
        if (total > 4)                                                          /* 0x4E28B jle */
            setD8(UI_SETUP_FLAGS, (uint8_t)(D8(UI_SETUP_FLAGS) & 0xBF));        /* 0x4E290: no "Draw 2 goods per city" */
        setD8(UI_SETUP_FLAGS, (uint8_t)(D8(UI_SETUP_FLAGS) & 0xFE));            /* 0x4E297..0x4E2A6 */
        if (S32(UI_PLAYERS_B) > 3)                                              /* 0x4E2A0, 0x4E2AC jle */
            setD8(UI_SETUP_FLAGS, (uint8_t)(D8(UI_SETUP_FLAGS) | 0x02));        /* 0x4E2B6 */
        else
            setD8(UI_SETUP_FLAGS, (uint8_t)(D8(UI_SETUP_FLAGS) & 0xFD));        /* 0x4E2C3 */
        return code;                                                            /* 0x4E2C9 */
    }
}

/* 0x4E5F3..0x4E685 and 0x4E6C5..0x4E757, the same block after a change of the computers' count: the AI-level buttons
   only with computers, and the AI level redrawn in the back buffer in colour 0xF, or 7 without computers. */
static void computers_changed(void)
{
    if (S8(UI_SETUP_COMPUTERS) == 0) {                                          /* 0x4E5F3 / 0x4E6C5 */
        f3C3D0(UI_DIALOG_SETUP, 0x68);                                          /* 0x4E609 / 0x4E6DB */
        f3C3D0(UI_DIALOG_SETUP, 0x67);                                          /* 0x4E618 / 0x4E6EA */
        fg_setcolor(7);                                                         /* 0x4E61D / 0x4E6EF push 7 */
    } else {
        f3C314(UI_DIALOG_SETUP, 0x68);                                          /* 0x4E62B / 0x4E6FD */
        f3C314(UI_DIALOG_SETUP, 0x67);                                          /* 0x4E63A / 0x4E70C */
        fg_setcolor(0xF);                                                       /* 0x4E63F / 0x4E711 push 0xF */
    }                                                                           /* 0x4E641 / 0x4E713 fg_setcolor */
    fg_vbopen(S32(0xF52C8u));                                                   /* 0x4E650 / 0x4E722 */
    f3A044(0x7D, 0xE1, ai_name());                                              /* 0x4E67B / 0x4E74D */
    fg_vbclose();                                                               /* 0x4E680 / 0x4E752 */
}

/* 0x4E764..0x4E813 and 0x4E825..0x4E8D6: store a new AI level (bits 10..11 of word [0xE1624]) and repaint its text:
   the background strip (0x7D..0x275, 0xE1..0xF7) from [0xF52CC], then the name in colour 0xF. */
static void ai_level_set(uint32_t level)
{
    setD8(UI_SETUP_FLAGS2, (uint8_t)(D8(UI_SETUP_FLAGS2) & 0xF3));              /* 0x4E78A / 0x4E84C: and -0xD */
    setD16(UI_SETUP_FLAGS, (uint16_t)(D16(UI_SETUP_FLAGS) | (level & 3u) << 10));   /* 0x4E790..0x4E7A0 / 0x4E852..0x4E862 */
    fg_setcolor(0xF);                                                           /* 0x4E7A7 / 0x4E869 */
    fg_vbcopy(0x7D, 0x275, 0xE1, 0xF7, 0x7D, 0xF7, S32(0xF52CCu), S32(0xF52C8u));   /* 0x4E7D4 / 0x4E897 */
    fg_vbopen(S32(0xF52C8u));                                                   /* 0x4E7E3 / 0x4E8A6 */
    f3A044(0x7D, 0xE1, ai_name());                                              /* 0x4E80E / 0x4E8D1 */
    fg_vbclose();                                                               /* 0x4E813 / 0x4E8D6 */
}

/* 0x4E2D8: one click on the setup dialog's button id (16-bit unsigned compares); returns -1 START GAME, -0xF CANCEL,
   -0x82 VARIANTS, else -100. The dispatch tree (0x4E2FC..0x4E342, 0x4E9F8, 0x4EA53, 0x4EAA3, 0x4EABA) is written as
   one test per id; every id lands where the tree sends it. */
int f4E2D8(int id)
{
    int32_t code = -100;                                                        /* 0x4E2F0 [ebp-0x1C] = 0xFFFFFF9C */
    uint16_t ax = (uint16_t)id;                                                 /* 0x4E2F7 */
    uint8_t lo, hi;

    switch (ax) {
    case 0x2A:                                                                  /* 0x4EAC0 jbe 0x4E9E4: START GAME */
        code = -1;                                                              /* 0x4E9E4 */
        break;
    case 0x4E:                                                                  /* 0x4EACA je 0x4E9D0: CANCEL */
        code = -0xF;                                                            /* 0x4E9D0 */
        break;
    case 0x59:                                                                  /* 0x4EA59 jbe 0x4E355: the PBEM toggle */
        /* 0x4E355..0x4E581, PBEM, dropped: on, it sets flag 0x80, disables 0x62, 0x61, 0x67, 0x68 and the variant
           0x5C (0xC2CA4), draws the ticked box and greys the computers' text, clears flag 0x20, raises the humans to
           3 if fewer and sets 0 computers; off, it clears 0x80, enables 0x62, 0x61, 0x5C and (with computers)
           0x67/0x68, and redraws the empty box and both texts. Returns -100 either way. */
        break;
    case 0x5B:                                                                  /* 0x4EAA7 je 0x4E9BC: VARIANTS */
        code = -0x82;                                                           /* 0x4E9BC */
        break;
    case 0x5F:                                                                  /* 0x4EA65 jbe 0x4E582: humans '-' */
        if (S8(UI_SETUP_HUMANS) > 0                                             /* 0x4E58C jle */
            && S8(UI_SETUP_COMPUTERS) + S8(UI_SETUP_HUMANS) > 3)                /* 0x4E5A8 jle */
            setD8(UI_SETUP_HUMANS, (uint8_t)(D8(UI_SETUP_HUMANS) - 1));         /* 0x4E5AE dec byte */
        break;
    case 0x60:                                                                  /* 0x4EA6B: humans '+' */
        if (S8(UI_SETUP_HUMANS) < 8                                             /* 0x4EA76 jge */
            && S8(UI_SETUP_COMPUTERS) + S8(UI_SETUP_HUMANS) < 8)                /* 0x4EA8E jge */
            setD8(UI_SETUP_HUMANS, (uint8_t)(D8(UI_SETUP_HUMANS) + 1));         /* 0x4EA90 inc byte */
        break;
    case 0x61:                                                                  /* 0x4E306 jbe 0x4E5C1: computers '-' */
        if (S8(UI_SETUP_COMPUTERS) > 0                                          /* 0x4E5CB jle */
            && S8(UI_SETUP_COMPUTERS) + S8(UI_SETUP_HUMANS) > 3) {              /* 0x4E5E7 jle */
            setD8(UI_SETUP_COMPUTERS, (uint8_t)(D8(UI_SETUP_COMPUTERS) - 1));   /* 0x4E5ED dec byte */
            computers_changed();
        }
        break;
    case 0x62:                                                                  /* 0x4E9FC jb 0x4E692: computers '+' */
        if (S8(UI_SETUP_COMPUTERS) < 8                                          /* 0x4E69D jge */
            && S8(UI_SETUP_COMPUTERS) + S8(UI_SETUP_HUMANS) < 8) {              /* 0x4E6B9 jge */
            setD8(UI_SETUP_COMPUTERS, (uint8_t)(D8(UI_SETUP_COMPUTERS) + 1));   /* 0x4E6BF inc byte */
            computers_changed();
        }
        break;
    case 0x63:                                                                  /* 0x4EA0F jbe 0x4E8E8: sessions '-' */
        lo = (uint8_t)(D8(UI_SETUP_TRADE) & 0xF);                               /* 0x4EA02 dl */
        if (lo > 1) {                                                           /* 0x4E8EC jle */
            setD8(UI_SETUP_TRADE, (uint8_t)(D8(UI_SETUP_TRADE) & 0xF0));        /* 0x4E907 */
            setD8(UI_SETUP_TRADE, (uint8_t)(D8(UI_SETUP_TRADE) | ((lo - 1) & 0xF)));   /* 0x4E913 */
        }
        break;
    case 0x64:                                                                  /* 0x4EA15: sessions '+' */
        lo = (uint8_t)(D8(UI_SETUP_TRADE) & 0xF);                               /* 0x4EA02 dl */
        if (lo < 4) {                                                           /* 0x4EA1C jge */
            setD8(UI_SETUP_TRADE, (uint8_t)(D8(UI_SETUP_TRADE) & 0xF0));        /* 0x4EA34 */
            setD8(UI_SETUP_TRADE, (uint8_t)(D8(UI_SETUP_TRADE) | ((lo + 1) & 0xF)));   /* 0x4EA40 */
        }
        break;
    case 0x65:                                                                  /* 0x4E316 jbe 0x4E926: minimum goods '-' */
        hi = (uint8_t)(D8(UI_SETUP_TRADE) >> 4);                                /* 0x4E92B */
        if (hi > 3) {                                                           /* 0x4E936 jle */
            setD8(UI_SETUP_TRADE, (uint8_t)(D8(UI_SETUP_TRADE) & 0xF));         /* 0x4E951 */
            setD8(UI_SETUP_TRADE, (uint8_t)(D8(UI_SETUP_TRADE) | ((hi - 1) & 0xF) << 4));   /* 0x4E95E */
        }
        break;
    case 0x66:                                                                  /* 0x4E320 jb 0x4E971: minimum goods '+' */
        hi = (uint8_t)(D8(UI_SETUP_TRADE) >> 4);                                /* 0x4E976 */
        if (hi < 7) {                                                           /* 0x4E981 jge */
            setD8(UI_SETUP_TRADE, (uint8_t)(D8(UI_SETUP_TRADE) & 0xF));         /* 0x4E9A1 */
            setD8(UI_SETUP_TRADE, (uint8_t)(D8(UI_SETUP_TRADE) | ((hi + 1) & 0xF) << 4));   /* 0x4E9A9 */
        }
        break;
    case 0x67:                                                                  /* 0x4E338 jbe 0x4E825: AI '-' = weaker */
        if (ai_level() < 2)                                                     /* 0x4E82D jge */
            ai_level_set(ai_level() + 1);                                       /* 0x4E846 inc eax */
        break;
    case 0x68:                                                                  /* 0x4E342 je 0x4E764: AI '+' = stronger */
        if (ai_level() > 0)                                                     /* 0x4E76B jle */
            ai_level_set(ai_level() - 1);                                       /* 0x4E784 dec eax */
        break;
    default:                                                                    /* 0x4E348, 0x4EAAD, 0x4EAD0 */
        break;
    }
    return code;                                                                /* [ebp-0x1C] */
}
