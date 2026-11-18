/* The dialog and button routines of AC_MAIN.EXE's UI kit (0x3A7B0, 0x3B05C..0x3C9D0, 0x785D4), translated literally.
   Dialogs and button records stay in the data object (link addresses, layouts in ui.h); grades: D = read in the code
   at the address given, G = inference. */

#include <string.h>

#include "fg.h"
#include "state.h"
#include "ui.h"

#define UI_MSGBOX_X 0xBB3FAu   /* int16 x of the message-box area ([0xBB3F8] sar 16, D 0x3A803) */
#define UI_MSGBOX_Y 0xBB3FCu   /* int16 y ([0xBB3FA] sar 16, D 0x3A82B) */
#define UI_MSGBOX_W 0xBB3FEu   /* int16 w ([0xBB3FC] sar 16, D 0x3A7CF) */
#define UI_MSGBOX_H 0xBB400u   /* int16 h ([0xBB3FE] sar 16, D 0x3A811) */
#define UI_BACKBUF  0xF52C8u   /* int32 virtual-buffer handle of the back buffer (D 0x3B072) */
#define UI_LABEL_DY_UP   0xC1954u  /* int16[3] label y offset per font, button up (-1, 0, 0) (D 0x3B1FB, 0x3B548) */
#define UI_LABEL_DY_DOWN 0xC1968u  /* int16[3] label y offset per font, button pressed (-2, 0, 0) (D 0x3B401, 0x3B55B) */

/* The three button painters copy the label into a stack buffer before printing it (the strcpy loop at 0x3B176,
   0x3B37A, 0x3B5EB). Each frame has 152 bytes for it (ebp-0xB8..ebp-0x20 at 0x3B16C, D). The port bounds the copy
   at that size; no label in the dialog templates comes near it (D for 0xC3090/0xC3868/0xC347C/0xC3C54; G for the
   labels built at run time). */
#define UI_LABEL_BUF 152

static void copy_label(char *buf, uint32_t label)
{
    const char *s = DSTR(label);
    size_t i;

    for (i = 0; i + 1 < UI_LABEL_BUF && s[i] != 0; i++)
        buf[i] = s[i];
    buf[i] = 0;
}

/* 0x3A7B0: message box: text wrapped and centred in the area at 0xBB3FA, fill 13, frames 4/14, text colour 5 */
void f3A7B0(uint32_t text)
{
    int32_t maxw;    /* [ebp-0x20]: widest line + 1, from the measuring pass */
    int32_t maxw2;   /* [ebp-0x18]: the drawing pass's, unused */
    int hh, bx, by;

    /* 0x3A7C6..0x3A7E2: measure only (draw 0), from (0, 0), width w-10, align 0 */
    hh = f3A42C(0, 0, S16(UI_MSGBOX_W) - 10, 0, DSTR(text), &maxw, 0);           /* [ebp-0x1C] = text height */
    bx = S16(UI_MSGBOX_X) + (S16(UI_MSGBOX_W) - (maxw + 10)) / 2;                /* 0x3A7EA..0x3A80E: signed /2 */
    by = S16(UI_MSGBOX_Y) + (S16(UI_MSGBOX_H) - (hh + 10)) / 2;                  /* 0x3A811..0x3A838 */

    fg_setcolor(13);                                                            /* 0x3A836 */
    fg_rect(bx, bx + maxw + 9, by, by + hh + 9);                                /* 0x3A85F */
    fg_setcolor(4);                                                             /* 0x3A867 */
    fg_box(bx, bx + maxw + 9, by, by + hh + 9);                                 /* 0x3A88D */
    fg_box(bx + 2, bx + maxw + 7, by + 2, by + hh + 7);                         /* 0x3A8B7 */
    fg_setcolor(14);                                                            /* 0x3A8BF */
    fg_box(bx + 1, bx + maxw + 8, by + 1, by + hh + 8);                         /* 0x3A8E7 */
    fg_setcolor(5);                                                             /* 0x3A8EF */
    /* 0x3A8F9..0x3A914: draw (1) at (bx+5, by+5), width = maxw (the measured width, not w-10), align 0 */
    f3A42C(bx + 5, by + 5, maxw, 0, DSTR(text), &maxw2, 1);
}

/* 0x3B05C: draw every button of dlg into the back buffer, remember dlg at 0xF8E78 */
void f3B05C(uint32_t dlg)
{
    int32_t i;

    fg_vbopen(S32(UI_BACKBUF));                                                 /* 0x3B079 */
    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                                  /* 0x3B091: movsx word [dlg]; jle */
        f3B4E0(dlg + DLG_REC(i));                                               /* 0x3B099..0x3B0B6 */
    fg_vbclose();                                                               /* 0x3B0BD */
    setD32(UI_CUR_DIALOG, dlg);                                                 /* 0x3B0C5 */
}

/* 0x3B0D4: draw button rec up (on the active surface) and mark it up */
void f3B0D4(uint32_t rec)
{
    char buf[UI_LABEL_BUF];   /* [ebp-0xB8] */
    int savefont;             /* [ebp-0x20] */
    int x, y, w, h, color;

    f78884();                                                                   /* 0x3B0EF: hide the pointer */
    x = S16(rec + BTN_X);
    y = S16(rec + BTN_Y);
    w = S16(rec + BTN_W);
    h = S16(rec + BTN_H);
    if (S8(rec + BTN_STYLE) == 1)                                               /* 0x3B0F7: [rec+0xE] sar 24 */
        f3C9D0(x, y, w, h, 3);                                                  /* 0x3B124: fill + raised */

    if (strlen(DSTR(D32(rec + BTN_LABEL))) != 0                                 /* 0x3B129: repne scasb */
        && S8(rec + BTN_STYLE) != 2) {                                          /* 0x3B14A */
        savefont = f3A02C();                                                    /* 0x3B153 */
        f39F90(S8(rec + BTN_FONT));                                             /* 0x3B164: [rec+0x10] sar 24 */
        copy_label(buf, D32(rec + BTN_LABEL));                                  /* 0x3B169..0x3B18E */

        color = S8(rec + BTN_COLOR);                                            /* 0x3B192: [rec+9] sar 24 */
        if (color == 0) {
            /* shadow, one pixel right and down: 0x20 enabled, 3 disabled (0x3B1A0..0x3B1BB) */
            fg_setcolor(S8(rec + BTN_ENABLED) == 1 ? 0x20 : 3);
            f3A0B0(x + 1,                                                       /* 0x3B20A */
                   y + 1 + (h - S32(UI_FONT_HEIGHT)) / 2                        /* 0x3B1CC..0x3B1EF: signed /2 */
                       + S16(UI_LABEL_DY_UP + 2u * (uint32_t)S8(rec + BTN_FONT)),   /* 0x3B1FB: [2*font+0xC1952] sar 16 */
                   w, buf);                                                     /* 0x3B214 */
        }

        /* the label: default colours 0x43 enabled / 0x17 disabled; an own colour when enabled, else 0x17 (0x3B219..0x3B25B) */
        if (S8(rec + BTN_COLOR) == 0)
            color = S8(rec + BTN_ENABLED) == 1 ? 0x43 : 0x17;
        else
            color = S8(rec + BTN_ENABLED) == 1 ? S8(rec + BTN_COLOR) : 0x17;
        fg_setcolor(color);                                                     /* 0x3B25C */
        f3A0B0(x,                                                               /* 0x3B2AF */
               y + (h - S32(UI_FONT_HEIGHT)) / 2                                /* 0x3B264..0x3B28F */
                   + S16(UI_LABEL_DY_UP + 2u * (uint32_t)S8(rec + BTN_FONT)),   /* 0x3B29A */
               w, buf);                                                         /* 0x3B2B2 */
        f39F90(savefont);                                                       /* 0x3B2BA */
    }
    setD8(rec + BTN_PRESSED, 0);                                                /* 0x3B2C2 */
    f788AC();                                                                   /* 0x3B2C6: restore the pointer */
}

/* 0x3B2D8: draw button rec pressed (on the active surface) and mark it pressed */
void f3B2D8(uint32_t rec)
{
    char buf[UI_LABEL_BUF];   /* [ebp-0xB4] */
    int savefont;             /* [ebp-0x1C] */
    int x, y, w, h, color;

    f78884();                                                                   /* 0x3B2F3 */
    x = S16(rec + BTN_X);
    y = S16(rec + BTN_Y);
    w = S16(rec + BTN_W);
    h = S16(rec + BTN_H);
    if (S8(rec + BTN_STYLE) == 1)                                               /* 0x3B301 */
        f3C9D0(x, y, w, h, 5);                                                  /* 0x3B328: fill + sunken */

    if (strlen(DSTR(D32(rec + BTN_LABEL))) != 0                                 /* 0x3B32D */
        && S8(rec + BTN_STYLE) != 2) {                                          /* 0x3B34E */
        savefont = f3A02C();                                                    /* 0x3B357 */
        f39F90(S8(rec + BTN_FONT));                                             /* 0x3B368 */
        copy_label(buf, D32(rec + BTN_LABEL));                                  /* 0x3B36D..0x3B392 */

        color = S8(rec + BTN_COLOR);                                            /* 0x3B396 */
        if (color == 0) {
            fg_setcolor(S8(rec + BTN_ENABLED) == 1 ? 0x20 : 3);                 /* 0x3B3A4..0x3B3BF */
            f3A0B0(x + 2,                                                       /* 0x3B419: shadow two pixels right */
                   y + 2 + (h - S32(UI_FONT_HEIGHT)) / 2                        /* 0x3B3C7..0x3B3F5 */
                       + S16(UI_LABEL_DY_DOWN + 2u * (uint32_t)S8(rec + BTN_FONT)), /* 0x3B401: [2*font+0xC1966] sar 16 */
                   w, buf);                                                     /* 0x3B41C */
        }

        if (S8(rec + BTN_COLOR) == 0)                                           /* 0x3B421..0x3B462 */
            color = S8(rec + BTN_ENABLED) == 1 ? 0x43 : 0x17;
        else
            color = S8(rec + BTN_ENABLED) == 1 ? S8(rec + BTN_COLOR) : 0x17;
        fg_setcolor(color);                                                     /* 0x3B463 */
        f3A0B0(x + 1,                                                           /* 0x3B4BB: label one pixel right */
               y + 1 + (h - S32(UI_FONT_HEIGHT)) / 2                            /* 0x3B46B..0x3B497 */
                   + S16(UI_LABEL_DY_DOWN + 2u * (uint32_t)S8(rec + BTN_FONT)), /* 0x3B4A3 */
               w, buf);                                                         /* 0x3B4BC */
        f39F90(savefont);                                                       /* 0x3B4C4 */
    }
    setD8(rec + BTN_PRESSED, 1);                                                /* 0x3B4CC */
    f788AC();                                                                   /* 0x3B4D0 */
}

/* 0x3B4E0: draw button rec in the state rec+0x12 holds, on the active surface (the state is not changed) */
void f3B4E0(uint32_t rec)
{
    char buf[UI_LABEL_BUF];   /* [ebp-0xC0] */
    int flags;                /* [ebp-0x18]: frame flags */
    int off;                  /* [ebp-0x20]: 1 when pressed, the label's x and y shift */
    int dy;                   /* [ebp-0x1C]: per-font label y offset */
    int savefont;             /* [ebp-0x28] */
    int x, y, w, h, color;

    f78884();                                                                   /* 0x3B4FB */
    flags = S8(rec + BTN_PRESSED) == 0 ? 3 : 5;                                 /* 0x3B503: [rec+0xF] sar 24 */
    off = S8(rec + BTN_PRESSED) != 0;                                           /* 0x3B527: setne */
    if (S8(rec + BTN_PRESSED) == 0)                                             /* 0x3B53D */
        dy = S16(UI_LABEL_DY_UP + 2u * (uint32_t)S8(rec + BTN_FONT));           /* 0x3B548: word [2*font+0xC1954], cwde */
    else
        dy = S16(UI_LABEL_DY_DOWN + 2u * (uint32_t)S8(rec + BTN_FONT));         /* 0x3B55B: word [2*font+0xC1968] */

    x = S16(rec + BTN_X);
    y = S16(rec + BTN_Y);
    w = S16(rec + BTN_W);
    h = S16(rec + BTN_H);
    if (S8(rec + BTN_STYLE) == 1)                                               /* 0x3B570 */
        f3C9D0(x, y, w, h, flags);                                              /* 0x3B599 */

    if (strlen(DSTR(D32(rec + BTN_LABEL))) != 0                                 /* 0x3B59E */
        && S8(rec + BTN_STYLE) != 2) {                                          /* 0x3B5BF */
        savefont = f3A02C();                                                    /* 0x3B5C8 */
        f39F90(S8(rec + BTN_FONT));                                             /* 0x3B5D9 */
        copy_label(buf, D32(rec + BTN_LABEL));                                  /* 0x3B5DE..0x3B603 */

        color = S8(rec + BTN_COLOR);                                            /* 0x3B607 */
        if (color == 0) {
            fg_setcolor(S8(rec + BTN_ENABLED) == 1 ? 0x20 : 3);                 /* 0x3B611..0x3B62C */
            f3A0B0(x + 1 + off,                                                 /* 0x3B66F..0x3B67C */
                   dy + (h - S32(UI_FONT_HEIGHT)) / 2 + off + (y + 1),          /* 0x3B640..0x3B66A */
                   w, buf);                                                     /* 0x3B67E */
        }

        if (S8(rec + BTN_COLOR) == 0)                                           /* 0x3B683..0x3B6BF */
            color = S8(rec + BTN_ENABLED) == 1 ? 0x43 : 0x17;
        else
            color = S8(rec + BTN_ENABLED) == 1 ? S8(rec + BTN_COLOR) : 0x17;
        fg_setcolor(color);                                                     /* 0x3B6C0 */
        f3A0B0(x + off,                                                         /* 0x3B6FD..0x3B709 */
               y + off + (h - S32(UI_FONT_HEIGHT)) / 2 + dy,                    /* 0x3B6D1..0x3B6F7 */
               w, buf);                                                         /* 0x3B70B */
        f39F90(savefont);                                                       /* 0x3B713 */
    }
    f788AC();                                                                   /* 0x3B718 */
}

/* 0x3BC64: set the y of the button with this id (the first match; "Button not found" box if none) */
void f3BC64(uint32_t dlg, int id, int y)
{
    int32_t i, found = -1;   /* [ebp-0xC], [ebp-0x8] */

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                                  /* 0x3BC94 */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {                   /* 0x3BCAE: cmp ax,dx (16 bits) */
            found = i;                                                          /* 0x3BCBD, then out of the loop (D) */
            break;
        }
    if (found == -1) {                                                          /* 0x3BCC0 */
        f3A7B0(0xB3CADu);                                                       /* 0x3BCCB: "Button not found" */
        return;
    }
    setD16(dlg + DLG_REC(found) + BTN_Y, (uint16_t)y);                          /* 0x3BCEB: word [dlg+20*i+4] */
}

/* 0x3C314: enable the button with this id and redraw it in the back buffer */
void f3C314(uint32_t dlg, int id)
{
    int32_t i, found = -1;   /* [ebp-0x14], [ebp-0x10] */

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                                  /* 0x3C343 */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {                   /* 0x3C364 */
            found = i;                                                          /* 0x3C36C: first match */
            break;
        }
    if (found == -1) {                                                          /* 0x3C36F */
        f3A7B0(0xB3D79u);                                                       /* 0x3C37A: "Button not found" */
        return;
    }
    fg_vbopen(S32(UI_BACKBUF));                                                 /* 0x3C388 */
    setD8(dlg + DLG_REC(found) + BTN_ENABLED, 1);                               /* 0x3C3A2: byte [dlg+20*i+0x12] */
    f3B4E0(dlg + DLG_REC(found));                                               /* 0x3C3BD */
    fg_vbclose();                                                               /* 0x3C3C2 */
}

/* 0x3C3D0: disable the button with this id and redraw it in the back buffer */
void f3C3D0(uint32_t dlg, int id)
{
    int32_t i, found = -1;   /* [ebp-0x14], [ebp-0x10] */

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                                  /* 0x3C3FF */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {                   /* 0x3C420 */
            found = i;                                                          /* 0x3C428: first match */
            break;
        }
    if (found == -1) {                                                          /* 0x3C42B */
        f3A7B0(0xB3D8Au);                                                       /* 0x3C436: "Button not found" */
        return;
    }
    fg_vbopen(S32(UI_BACKBUF));                                                 /* 0x3C444 */
    setD8(dlg + DLG_REC(found) + BTN_ENABLED, 0);                               /* 0x3C45E */
    f3B4E0(dlg + DLG_REC(found));                                               /* 0x3C479 */
    fg_vbclose();                                                               /* 0x3C47E */
}

/* 0x3C5D4: click feedback on the active surface: pressed, wait for the release, up (unless rec+0x0D == 1) */
void f3C5D4(uint32_t dlg, int id)
{
    int32_t i, found = -1;   /* [ebp-0x10], [ebp-0xC] */

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                                  /* 0x3C602 */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {                   /* 0x3C623 */
            found = i;                                                          /* 0x3C62B: first match */
            break;
        }
    if (found == -1) {                                                          /* 0x3C62E */
        f3A7B0(0xB3DBDu);                                                       /* 0x3C639: "Button not found" */
        return;
    }
    if (S8(dlg + DLG_REC(found) + BTN_NOFLASH) == 1)                            /* 0x3C652: [dlg+20*i+0xC] sar 24 */
        return;
    f3B2D8(dlg + DLG_REC(found));                                               /* 0x3C674 */
    f775F0();                                                                   /* 0x3C679: wait for the release */
    f3B0D4(dlg + DLG_REC(found));                                               /* 0x3C695 */
}

/* 0x3C9D0: 3-D frame of (x, y, w, h): flags 1 fill, 2 raised, 4 sunken */
void f3C9D0(int x, int y, int w, int h, int flags)
{
    int lower;   /* the colour of the bottom/right edges (pushed at 0x3CAA5 / 0x3CB2D) */

    if (flags & 1) {                                                            /* 0x3C9F1: test dl,1 */
        fg_setcolor(13);                                                        /* 0x3C9F8 */
        fg_rect(x + 1, x + 1 + w - 3, y + 1, y + 1 + h - 3);                    /* 0x3CA20 */
    }
    if (flags & 2) {                                                            /* 0x3CA2B: test si,2 */
        fg_setcolor(14);                                                        /* 0x3CA34: light top/left */
        lower = 0x36;                                                           /* 0x3CAA5 */
    } else if (flags & 4) {                                                     /* 0x3CAAF: test di,4 */
        fg_setcolor(0x36);                                                      /* 0x3CABC: dark top/left */
        lower = 14;                                                             /* 0x3CB2D */
    } else
        return;                                                                 /* 0x3CAB4 */

    /* top/left edge with cut corners (0x3CA4B..0x3CA9D, the same at 0x3CAD3..0x3CB25) */
    fg_move(x + 1, y + h - 2);
    fg_draw(x, y + h - 3);
    fg_draw(x, y + 2);
    fg_draw(x + 2, y);
    fg_draw(x + w - 3, y);

    fg_setcolor(lower);                                                         /* 0x3CB2F */
    /* bottom/right edge (0x3CB46..0x3CBA8) */
    fg_move(x + w - 2, y + 1);
    fg_draw(x + w - 1, y + 2);
    fg_draw(x + w - 1, y + h - 3);
    fg_draw(x + w - 3, y + h - 1);
    fg_draw(x + 2, y + h - 1);
}

/* 0x785D4: append src's button records after dst's and add src's count to dst's */
void f785D4(uint32_t dst, uint32_t src)
{
    int32_t n0 = S16(dst + DLG_COUNT);                                          /* 0x785EF: [ebp-0x8] */
    int32_t end = n0 + S16(src + DLG_COUNT);                                    /* 0x785F8..0x78600: [ebp-0x4] */
    int32_t i;

    for (i = n0; i < end; i++)                                                  /* 0x78614: jge */
        memmove(DP(dst + DLG_REC(i)), DP(src + DLG_REC(i - n0)), BTN_SIZE);     /* 0x78648: 5 x movsd */
    setD16(dst + DLG_COUNT, (uint16_t)(D16(dst + DLG_COUNT) + D16(src + DLG_COUNT)));  /* 0x78658: 16-bit add */
}
