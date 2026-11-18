/* The "Variants" screen of the new-game setup in AC_MAIN.EXE: 0x4EAE0 (the screen and its click loop) and 0x4EEC8
   (one click on a row). Literal translations; the address of each instruction a line comes from is in its comment.
   D = read in the code, G = inference. Prototypes and the shared layouts (dialog 0xC2CA4, the flags 0xE1624/0xE1625,
   the box sprites 0xF51CC/0xF51D4): ui.h. No Watcom runtime routine is called here.

   Play-by-email is dropped (Erik's ruling, OVERVIEW §7.5): f4EAE0's branch that enables the PBEM-only row 0x104
   when flag 0x80 is set (0x4EBBC..0x4EBCB) is left out, see there. The row's click case in f4EEC8 (0x4F066..0x4F0C9)
   is not PBEM code and is kept, although in the port the row stays disabled and so can never be hit. */

#include "ui.h"
#include "fg.h"

/* 0x4EEC8: one click on the variants dialog's button id (16-bit unsigned compares, D 0x4EEF7..0x4F16E). The rows
   toggle their flag bit and draw the matching box sprite into the back buffer; the screen shows it at the next
   f3CC14 of f4EAE0's loop (0x4EDCB). Returns -1 for CONTINUE (0x2A), else -100. */
int f4EEC8(int id)
{
    int32_t code = -100;                              /* 0x4EEDE [ebp-0x14] = 0xFFFFFF9C */
    uint16_t ax = (uint16_t)id;                       /* 0x4EEF4: [ebp-0x10] = eax; cmp ax,... (jb/jbe: unsigned) */

    fg_vbopen(S32(0xF52C8u));                         /* 0x4EEEC: the back buffer */

    if (ax == 0x2A)                                   /* 0x4F162 cmp ax,0x2A; 0x4F168 jbe 0x4F131: CONTINUE */
        code = -1;                                    /* 0x4F131 */
    else if (ax == 0x5A) {                            /* 0x4F16E je 0x4EF31: "Allow more than 2 calamities per victim" */
        if ((D8(UI_SETUP_FLAGS) & 0x10) == 0) {       /* 0x4EF31 word [0xE1624] shl 0xB, shr ax,0xF: bit 4 */
            setD8(UI_SETUP_FLAGS, (uint8_t)(D8(UI_SETUP_FLAGS) | 0x10));    /* 0x4EF50 or ah,0x10; 0x4EF58 */
            f3AF8C(UI_SPR_CHECK_ON, 0x58, 0x66);      /* 0x4EF83 */
        } else {
            setD8(UI_SETUP_FLAGS, (uint8_t)(D8(UI_SETUP_FLAGS) & ~0x10));   /* 0x4EF70 and dl,-0x11; 0x4EF78 */
            f3AF8C(UI_SPR_CHECK_OFF, 0x58, 0x66);     /* 0x4EF83 */
        }
    } else if (ax == 0x5C) {                          /* 0x4EF01 jbe 0x4EF98: "Allow purchase from any stack" */
        if ((D8(UI_SETUP_FLAGS) & 0x20) == 0) {       /* 0x4EF98 shl 0xA, shr ax,0xF: bit 5 */
            setD8(UI_SETUP_FLAGS, (uint8_t)(D8(UI_SETUP_FLAGS) | 0x20));    /* 0x4EFB7 or dh,0x20; 0x4EFBF */
            f3AF8C(UI_SPR_CHECK_ON, 0x58, 0x7F);      /* 0x4EFEA */
        } else {
            setD8(UI_SETUP_FLAGS, (uint8_t)(D8(UI_SETUP_FLAGS) & ~0x20));   /* 0x4EFD7 and bl,-0x21; 0x4EFDF */
            f3AF8C(UI_SPR_CHECK_OFF, 0x58, 0x7F);     /* 0x4EFEA */
        }
    } else if (ax == 0x5D) {                          /* 0x4F148 cmp ax,0x5D; je 0x4EFFF: "Draw 2 goods per city" */
        if ((D8(UI_SETUP_FLAGS) & 0x40) == 0) {       /* 0x4EFFF shl 9, shr ax,0xF: bit 6 */
            setD8(UI_SETUP_FLAGS, (uint8_t)(D8(UI_SETUP_FLAGS) | 0x40));    /* 0x4F01E or bh,0x40; 0x4F026 */
            f3AF8C(UI_SPR_CHECK_ON, 0x58, 0x98);      /* 0x4F051 */
        } else {
            setD8(UI_SETUP_FLAGS, (uint8_t)(D8(UI_SETUP_FLAGS) & ~0x40));   /* 0x4F043 and cl,-0x41; 0x4F04B */
            f3AF8C(UI_SPR_CHECK_OFF, 0x58, 0x98);     /* 0x4F051 */
        }
    } else if (ax == 0x104) {                         /* 0x4EF11 jbe 0x4F066: "Another trade round needs only 2 players" */
        if ((D8(UI_SETUP_FLAGS2) & 0x10) == 0) {      /* 0x4F066 shl 3, shr ax,0xF: word bit 12 = 0xE1625 bit 4 */
            setD8(UI_SETUP_FLAGS2, (uint8_t)(D8(UI_SETUP_FLAGS2) | 0x10));  /* 0x4F08A or ch,0x10; 0x4F092 */
            f3AF8C(UI_SPR_CHECK_ON, 0x58, 0xB1);      /* 0x4F0B5 */
        } else {
            setD8(UI_SETUP_FLAGS2, (uint8_t)(D8(UI_SETUP_FLAGS2) & ~0x10)); /* 0x4F0A4 and al,-0x11; 0x4F0AB */
            f3AF8C(UI_SPR_CHECK_OFF, 0x58, 0xB1);     /* 0x4F0B5 */
        }
    } else if (ax == 0x105) {                         /* 0x4EF1B je 0x4F0CA: "Real Goods in trade are known" */
        if ((D8(UI_SETUP_FLAGS2) & 0x20) == 0) {      /* 0x4F0CA shl 2, shr ax,0xF: word bit 13 = 0xE1625 bit 5 */
            setD8(UI_SETUP_FLAGS2, (uint8_t)(D8(UI_SETUP_FLAGS2) | 0x20));  /* 0x4F0E9 or ah,0x20; 0x4F0F1 */
            f3AF8C(UI_SPR_CHECK_ON, 0x58, 0xCA);      /* 0x4F11C */
        } else {
            setD8(UI_SETUP_FLAGS2, (uint8_t)(D8(UI_SETUP_FLAGS2) & ~0x20)); /* 0x4F109 and dl,-0x21; 0x4F111 */
            f3AF8C(UI_SPR_CHECK_OFF, 0x58, 0xCA);     /* 0x4F11C */
        }
    }
    /* any other id: nothing (0x4EF21, 0x4F152, 0x4F174) */

    fg_vbclose();                                     /* every exit: 0x4EF21, 0x4EF88, ..., 0x4F174 */
    return code;
}

/* 0x4EAE0: the "Variants" screen (D). Its caller (main 0x113F7) ignores eax. */
void f4EAE0(void)
{
    int32_t code;                                     /* [ebp-0x18] */
    int32_t i;                                        /* [ebp-0x1C] */

    f3CD6C(7);                                        /* 0x4EAF9: partha.gif into [0xF52CC] */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(0xF52CCu), S32(0xF52C8u));   /* 0x4EB21 -> the back buffer */
    fg_vbopen(S32(0xF52C8u));                         /* 0x4EB30 */
    f3A18C(0, 0x28, 0x280, 0x45, DSTR(0xB4807u));     /* 0x4EB4E "Variants" */

    /* "Draw 2 goods per city" only with at most 4 players (humans + computers) */
    if (S8(UI_SETUP_HUMANS) + S8(UI_SETUP_COMPUTERS) > 4) {   /* 0x4EB53 dword [0xE161E]/[0xE161D] sar 24; 0x4EB69 jle */
        f3BFBC(UI_DIALOG_VARIANTS, 0x5D, 0);          /* 0x4EB77 */
        setD8(UI_SETUP_FLAGS, (uint8_t)(D8(UI_SETUP_FLAGS) & ~0x40));   /* 0x4EB7C and byte [0xE1624],-0x41 */
    } else
        f3BFBC(UI_DIALOG_VARIANTS, 0x5D, 1);          /* 0x4EB94 */

    /* "Another trade round needs only 2 players" only with play-by-email (flag 0x80, 0x4EB99). PBEM is dropped and
       the port never sets 0x80 (title_menu clears it, 0x4F7CC), so only the non-PBEM side is kept; the PBEM side
       f3BFBC(0xC2CA4, 0x104, 1) at 0x4EBBC..0x4EBCB is left out. */
    f3BFBC(UI_DIALOG_VARIANTS, 0x104, 0);             /* 0x4EBAE */
    setD8(UI_SETUP_FLAGS2, (uint8_t)(D8(UI_SETUP_FLAGS2) & ~0x10));   /* 0x4EBB3 and byte [0xE1625],-0x11 */

    f39F90(2);                                        /* 0x4EBDA */
    fg_setcolor(f3BF30(UI_DIALOG_VARIANTS, 0x5C) == 0 ? 7 : 0xF);     /* 0x4EBE4 (edx still 0x5C); 0x4EBFA */
    f3A044(0x7D, 0x7D, DSTR(0xB4810u));               /* 0x4EC0E "Allow purchase from any stack" */
    fg_setcolor(f3BF30(UI_DIALOG_VARIANTS, 0x5D) == 0 ? 7 : 0xF);     /* 0x4EC1D; 0x4EC33 */
    f3A044(0x7D, 0x96, DSTR(0xB482Eu));               /* 0x4EC4A "Draw 2 goods per city" */
    fg_setcolor(0xF);                                 /* 0x4EC51 */
    f3A044(0x7D, 0x64, DSTR(0xB4844u));               /* 0x4EC68 "Allow more than 2 calamities per victim" */
    fg_setcolor(f3BF30(UI_DIALOG_VARIANTS, 0x104) == 0 ? 7 : 0xF);    /* 0x4EC77; 0x4EC8D */
    f3A044(0x7D, 0xAF, DSTR(0xB486Cu));               /* 0x4ECA4 "Another trade round needs only 2 players" */
    fg_setcolor(0xF);                                 /* 0x4ECAB */
    f3A044(0x7D, 0xC8, DSTR(0xB4895u));               /* 0x4ECC2 "Real Goods in trade are known" */

    /* the boxes: ticked when the flag is set */
    f3AF8C((D8(UI_SETUP_FLAGS) & 0x10) ? UI_SPR_CHECK_ON : UI_SPR_CHECK_OFF, 0x58, 0x66);   /* 0x4ECC7; 0x4ECF0 */
    f3AF8C((D8(UI_SETUP_FLAGS) & 0x20) ? UI_SPR_CHECK_ON : UI_SPR_CHECK_OFF, 0x58, 0x7F);   /* 0x4ECF5; 0x4ED1E */
    f3AF8C((D8(UI_SETUP_FLAGS) & 0x40) ? UI_SPR_CHECK_ON : UI_SPR_CHECK_OFF, 0x58, 0x98);   /* 0x4ED23; 0x4ED4C */
    f3AF8C((D8(UI_SETUP_FLAGS2) & 0x10) ? UI_SPR_CHECK_ON : UI_SPR_CHECK_OFF, 0x58, 0xB1);  /* 0x4ED51; 0x4ED7A */
    f3AF8C((D8(UI_SETUP_FLAGS2) & 0x20) ? UI_SPR_CHECK_ON : UI_SPR_CHECK_OFF, 0x58, 0xCA);  /* 0x4ED7F; 0x4EDA8 */

    f3B05C(UI_DIALOG_VARIANTS);                       /* 0x4EDB2: the buttons (opens and closes the back buffer itself) */
    f39F90(2);                                        /* 0x4EDBC */
    fg_vbclose();                                     /* 0x4EDC1 */
    f3CC14();                                         /* 0x4EDC6: on the screen */

    do {
        f3CC14();                                     /* 0x4EDCB: the boxes a click redrew */
        f775F0();                                     /* 0x4EDD0: no button down */
        while (S32(0xE1630u) == 0) {                  /* 0x4EDD5: wait for a click (either button) */
            int x, y, b;
            fg_mousepos(&x, &y, &b);                  /* 0x4EDED fg_mousepos(0xE1628, 0xE162C, 0xE1630) */
            setD32(0xE1628u, (uint32_t)x);
            setD32(0xE162Cu, (uint32_t)y);
            setD32(0xE1630u, (uint32_t)b);
        }
        code = -100;                                  /* 0x4EDF7 */
        for (i = 0; i < S16(UI_DIALOG_VARIANTS + DLG_COUNT); i++) {   /* 0x4EE0B dword [0xC2CA2] sar 16 */
            uint32_t rec = UI_DIALOG_VARIANTS + DLG_REC(i);           /* 0x4EE1C: 0xC2CA6 + 20*i */
            int32_t x, y;
            if (S8(rec + BTN_ENABLED) != 1)           /* 0x4EE38 dword [rec+0xD] sar 24 */
                continue;
            x = S16(rec + BTN_X);                     /* 0x4EE4C movsx word [rec] */
            if (x > S32(0xE1628u))                    /* 0x4EE51 jg */
                continue;
            if (x + S16(rec + BTN_W) <= S32(0xE1628u))   /* 0x4EE56 dword [rec+2] sar 16; 0x4EE60 jle */
                continue;
            y = S16(rec + BTN_Y);                     /* 0x4EE65 dword [rec] sar 16 */
            if (y > S32(0xE162Cu))                    /* 0x4EE72 jg */
                continue;
            if (y + S16(rec + BTN_H) <= S32(0xE162Cu))   /* 0x4EE77 dword [rec+4] sar 16; 0x4EE81 jle */
                continue;
            f3C5D4(UI_DIALOG_VARIANTS, S16(rec + BTN_ID));   /* 0x4EE91: the press, until the button is let go */
            code = f4EEC8(S16(rec + BTN_ID));         /* 0x4EE9F */
            break;                                    /* falls through to 0x4EEA7 */
        }
    } while (code == -100 || code != -1);             /* 0x4EEA7 je 0x4EDCB; 0x4EEB7 jne 0x4EDCB */
}
