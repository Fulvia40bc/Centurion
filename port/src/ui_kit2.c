/* UI-kit routines of AC_MAIN.EXE used by the nation choice (module N1 of docs/MAP_PLAN.md): the message boxes in the
   back buffer 0x3A9F4 / 0x3AB7C, the sprite release 0x3AF54, the list-index clamp 0x79A9C, the delay 0x78A20 and the
   dialog disable / re-enable pair 0x3C940 / 0x3C7A0. Literal translations; the address of each instruction a line
   comes from is in its comment. Prototypes and the data layouts: game.h (N1 section) and ui.h.
   Grades: D = read in the code at the address given, G = inference.

   Watcom C runtime -> standard C in this file:
     0x877E6 free   -> free() (stdlib.h), on the block ui_sprite_pixels() gives for the record
     0x8A080 clock  -> clock_8A080() below: a port clock in Watcom's units (hundredths of a second) that advances only
                       in the wait of f78A20, one 18.2 Hz BIOS tick per fg_waitfor(1) (G, port design; see there) */

#include <stdint.h>
#include <stdlib.h>

#include "fg.h"
#include "game.h"
#include "state.h"
#include "ui.h"

#define UI_MSGBOX_X 0xBB3FAu   /* int16 x of the message-box area ([0xBB3F8] sar 16, D 0x3AA56) */
#define UI_MSGBOX_Y 0xBB3FCu   /* int16 y ([0xBB3FA] sar 16, D 0x3AA7E) */
#define UI_MSGBOX_W 0xBB3FEu   /* int16 w ([0xBB3FC] sar 16, D 0x3AA13) */
#define UI_MSGBOX_H 0xBB400u   /* int16 h ([0xBB3FE] sar 16, D 0x3AA64) */
#define UI_BACKBUF  0xF52C8u   /* int32 virtual-buffer handle of the back buffer (D 0x3AA2E) */
#define UI_DELAY_DIV 0xBB3F4u  /* int32: 1 (obj2), 10 with the /C switch (0x10494, "Continuous testing") (D 0x78A43) */
#define UI_CHOICE_COUNT 0xF7C9Cu   /* int32 count of the choice list 0xF7CA0 (D 0x79AB3) */
/* int8[50] 0xF8E44: ui.h calls it UI_SAVED_PRESSED, but 0x3C940 / 0x3C7A0 save and restore byte [dlg + 20i + 0x12],
   which is record i's +0x10, the ENABLED byte (D 0x3C983, 0x3C7FA). */
#define UI_SAVED_ENABLED UI_SAVED_PRESSED

/* 0x3A9F4: message box sized to its text, centred in the area at 0xBB3FA, text left-aligned, in the back buffer.
   The same as f3A7B0 (ui_dialog.c) except for the back buffer and the text argument (a C pointer: callers pass data
   strings and stack buffers). */
void f3A9F4(const char *text)
{
    int32_t maxw;    /* [ebp-0x20]: widest line + 1, from the measuring pass */
    int32_t maxw2;   /* [ebp-0x18]: the drawing pass's, unused */
    int32_t th;      /* [ebp-0x1C]: text height (the y under the last line, from y = 0) */
    int32_t bx;      /* [ebp-0x14] */
    int32_t by;      /* [ebp-0x10] */

    /* 0x3AA0A..0x3AA26: measure only (draw 0), from (0, 0), width w - 10, align 0 (xor ecx,ecx) */
    th = f3A42C(0, 0, S16(UI_MSGBOX_W) - 10, 0, text, &maxw, 0);
    fg_vbopen(S32(UI_BACKBUF));                                                 /* 0x3AA35 */
    bx = S16(UI_MSGBOX_X) + (S16(UI_MSGBOX_W) - (maxw + 10)) / 2;               /* 0x3AA3A..0x3AA61: signed /2 */
    by = S16(UI_MSGBOX_Y) + (S16(UI_MSGBOX_H) - (th + 10)) / 2;                 /* 0x3AA64..0x3AA8B */

    fg_setcolor(13);                                                            /* 0x3AA8E */
    fg_rect(bx, bx + maxw + 9, by, by + th + 9);                                /* 0x3AAB2 */
    fg_setcolor(4);                                                             /* 0x3AABC */
    fg_box(bx, bx + maxw + 9, by, by + th + 9);                                 /* 0x3AAE0 */
    fg_box(bx + 2, bx + maxw + 7, by + 2, by + th + 7);                         /* 0x3AB0A */
    fg_setcolor(14);                                                            /* 0x3AB14 */
    fg_box(bx + 1, bx + maxw + 8, by + 1, by + th + 8);                         /* 0x3AB3A */
    fg_setcolor(5);                                                             /* 0x3AB44 */
    /* 0x3AB4C..0x3AB67: draw (1) at (bx+5, by+5), width = maxw (the measured width), align 0 */
    f3A42C(bx + 5, by + 5, maxw, 0, text, &maxw2, 1);
    fg_vbclose();                                                               /* 0x3AB6C */
}

/* 0x3AB7C: as f3A9F4 with the text centred (align 2, mov ecx,2 at 0x3AB9B and 0x3ACED) */
void f3AB7C(const char *text)
{
    int32_t maxw;    /* [ebp-0x20] */
    int32_t maxw2;   /* [ebp-0x18], unused */
    int32_t th;      /* [ebp-0x1C] */
    int32_t bx;      /* [ebp-0x14] */
    int32_t by;      /* [ebp-0x10] */

    th = f3A42C(0, 0, S16(UI_MSGBOX_W) - 10, 2, text, &maxw, 0);               /* 0x3AB92..0x3ABB1 */
    fg_vbopen(S32(UI_BACKBUF));                                                 /* 0x3ABC0 */
    bx = S16(UI_MSGBOX_X) + (S16(UI_MSGBOX_W) - (maxw + 10)) / 2;               /* 0x3ABC5..0x3ABEC */
    by = S16(UI_MSGBOX_Y) + (S16(UI_MSGBOX_H) - (th + 10)) / 2;                 /* 0x3ABEF..0x3AC16 */

    fg_setcolor(13);                                                            /* 0x3AC19 */
    fg_rect(bx, bx + maxw + 9, by, by + th + 9);                                /* 0x3AC3D */
    fg_setcolor(4);                                                             /* 0x3AC47 */
    fg_box(bx, bx + maxw + 9, by, by + th + 9);                                 /* 0x3AC6B */
    fg_box(bx + 2, bx + maxw + 7, by + 2, by + th + 7);                         /* 0x3AC95 */
    fg_setcolor(14);                                                            /* 0x3AC9F */
    fg_box(bx + 1, bx + maxw + 8, by + 1, by + th + 8);                         /* 0x3ACC5 */
    fg_setcolor(5);                                                             /* 0x3ACCF */
    f3A42C(bx + 5, by + 5, maxw, 2, text, &maxw2, 1);                          /* 0x3ACD7..0x3ACF5 */
    fg_vbclose();                                                               /* 0x3ACFA */
}

/* 0x3AF54: release sprite record spr's pixel block if it has a width.
   Port (G): the record's +0 holds a ui_sprite_put handle instead of the heap pointer (ui_widgets.c). The block is
   freed, the handle's slot emptied and +0 cleared, so that a later ui_sprite_put on this record cannot reuse a slot
   that another record has taken in the meantime. The original leaves the dangling pointer in +0; every caller's
   record lies in a by-design range (0xF48A4..0xF522F, or 0x2EFA0's local record). */
void f3AF54(uint32_t spr)
{
    if (D16(spr + UI_SPR_W) != 0) {                   /* 0x3AF6A..0x3AF73: word [spr+4] & 0xFFFF; je */
        free(ui_sprite_pixels(spr));                  /* 0x3AF78..0x3AF7A: free([spr]) */
        ui_sprite_put(spr, NULL);                     /* port: empty the handle's slot */
        setD32(spr + UI_SPR_DATA, 0);                 /* port: no handle (also undoes a slot ui_sprite_put picked
                                                         for a record that had none) */
        setD16(spr + UI_SPR_W, 0);                    /* 0x3AF82: word [spr+4] = 0 */
    }
}

/* 0x79A9C: clamp *p (a caller's stack int32) to 0 .. [0xF7C9C] - 1. Prologue push 0x10 / __CHK / push edx / push ebp
   (D); the result is in *p only. */
void f79A9C(int32_t *p)
{
    int32_t count = S32(UI_CHOICE_COUNT);             /* 0x79AB3 */

    if (count <= *p)                                  /* 0x79AB9 cmp edx,[eax]; jg 0x79AC5 (signed) */
        *p = (int32_t)((uint32_t)count - 1u);         /* 0x79ABD lea eax,[edx-1] (wraps as on the 386) */
    if (*p < 0)                                       /* 0x79AC8 cmp dword [eax],0; jge (signed) */
        *p = 0;                                       /* 0x79AD0 */
}

/* The port's 0x8A080 clock (G, port design). Watcom's clock() gives hundredths of a second since start-up, read from
   the DOS time, so it moves in steps of one BIOS tick (1/18.2 s = 1000/182 hundredths; docs/LIBRARY_CALLS.md). The
   original busy-waits on it; the port waits instead with fg_waitfor(1), one tick per call, and counts the ticks, so
   this clock advances by exactly the waits made: in real time when a window is open (fg_waitfor sleeps and keeps the
   window serviced), and at once when headless (fg_set_headless: fg_waitfor returns without sleeping), so a test run
   neither sleeps nor spins and ends the wait after the same number of DOS clock steps as the original. Only f78A20
   reads it. The midnight wrap of Watcom's clock (8,640,000) is not reproduced. */
static uint32_t s_bios_ticks;   /* ticks waited so far */

static uint32_t clock_8A080(void)
{
    return (uint32_t)((uint64_t)s_bios_ticks * 1000u / 182u);
}

/* port: the clock above for the routines that read 0x8A080 themselves (game.h) */
uint32_t wat_clock(void)
{
    return clock_8A080();
}

/* 0x78A20: wait t milliseconds (Watcom's hundredths: (t / [0xBB3F4]) / 10; [0xBB3F4] = 1, or 10 with /C) */
void f78A20(int t)
{
    int32_t start;   /* [ebp-0xC] */

    start = (int32_t)clock_8A080();                                             /* 0x78A35 */
    /* 0x78A3D..0x78A66: target = start + (t / [0xBB3F4]) / 10 (two signed idivs, 0x78A4C, 0x78A58; 32-bit add),
       loop while target > clock() (unsigned: ja). The body is the port's: one tick of waiting (see clock_8A080). */
    while ((uint32_t)start + (uint32_t)((int32_t)t / S32(UI_DELAY_DIV) / 10) > clock_8A080()) {
        fg_waitfor(1);
        s_bios_ticks++;
    }
}

/* 0x3C940: disable every button of dlg, saving each enabled byte (record +0x10) to 0xF8E44[i], and redraw them on the
   active surface, with the mouse pointer hidden */
void f3C940(uint32_t dlg)
{
    int32_t i;   /* [ebp-0x8] */

    f78884();                                                                   /* 0x3C956: hide the pointer */
    for (i = 0; S16(dlg + DLG_COUNT) > i; i++) {                                /* 0x3C969 movsx word [dlg]; jle */
        /* 0x3C983..0x3C989: byte [i + 0xF8E44] = byte [dlg + 20i + 0x12] (record i's +0x10) */
        setD8(UI_SAVED_ENABLED + (uint32_t)i, D8(dlg + DLG_REC(i) + BTN_ENABLED));
        setD8(dlg + DLG_REC(i) + BTN_ENABLED, 0);                               /* 0x3C9A1 */
        f3B4E0(dlg + DLG_REC(i));                                               /* 0x3C9BC: dlg + 2 + 20i */
    }
    f788AC();                                                                   /* 0x3C9C3: restore the pointer */
}

/* 0x3C7A0: undo f3C940: restore each button's enabled byte from 0xF8E44[i] and redraw it in the back buffer
   (no pointer hiding here, D) */
void f3C7A0(uint32_t dlg)
{
    int32_t i;   /* [ebp-0x10] */

    fg_vbopen(S32(UI_BACKBUF));                                                 /* 0x3C7BD */
    for (i = 0; S16(dlg + DLG_COUNT) > i; i++) {                                /* 0x3C7D5 movsx word [dlg]; jle */
        /* 0x3C7F4..0x3C7FA: byte [dlg + 20i + 0x12] = byte [i + 0xF8E44] */
        setD8(dlg + DLG_REC(i) + BTN_ENABLED, D8(UI_SAVED_ENABLED + (uint32_t)i));
        f3B4E0(dlg + DLG_REC(i));                                               /* 0x3C814 */
    }
    fg_vbclose();                                                               /* 0x3C81B */
}
