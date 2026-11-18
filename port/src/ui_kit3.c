/* UI-kit routines of AC_MAIN.EXE for module N2 of docs/MAP_PLAN.md (the AST chart and the button dispatcher): the
   generic click wait 0x78BA4, the button-by-id setters 0x3BB48 (x), 0x3BD80 (w), 0x3BE10 (h), 0x3C0D8 (style),
   0x3C284 (label), the by-id painters in the back buffer 0x3B7CC (pressed) and 0x3B870 (up), the pressed-state reader
   0x3B9A4, the dialog disable / restore pair 0x3C828 / 0x3C8C4, and the text helpers 0x3A148 (a number centred) and
   0x3A3A0 (right-aligned). Literal translations; the address of each instruction a line comes from is in its comment.
   Prototypes and the data layouts: game.h (section "AST chart, button kit, ...") and ui.h (dialog and button records).
   Grades: D = read in the code at the address given, G = inference.

   The by-id routines all search the same way (D, e.g. 0x3BB61..0x3BBA1): found = -1; for i < int16 [dlg] (movsx,
   signed jle): if word [dlg + 20i + 0x10] (record i's +0x0E, the id) == (uint16)id (cmp ax,dx), found = i and the
   code falls out of the loop (the first match wins). Then found == -1 -> the "Button not found" box (f3A7B0, each
   routine with its own copy of the string). They address record i's fields as dlg + 20i + (field + 2). Each routine
   keeps its own copy of the loop here too.

   Watcom C runtime -> standard C in this file:
     0x8C897 itoa(value, buf, 10) -> snprintf "%d" into a buffer of the original's size (20 bytes at [ebp-0x24],
             0x3A16C); for radix 10 Watcom writes '-' and the magnitude as unsigned (LIBRARY_CALLS.md), as "%d" does
     repne scasb / not / dec (inline strlen) -> strlen
   __CHK dropped. Library calls through fg.h in the original argument order.

   Heap pointers in data fields: none written here. f3C284 stores a text LINK ADDRESS in a button record's +0x08
   (game.h: two callers pass stack buffers, 0x516EE and 0x54128, which their translators must place at link
   addresses). f78BA4 reads the mouse fields 0xE1628/0xE162C/0xE1630 (int32) and the choice list 0xF7C9C/0xF7CA0.

   The map hit test 0x3D2A4 (reached from f78BA4 only with map == 1 and a click at y <= 0x145 that hit no enabled
   button) is mapdata.c's f3D2A4 (module MAP1).

   1995 bugs: none found in these routines (no g_original branches). */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "fg.h"
#include "game.h"
#include "mapdata.h"
#include "state.h"
#include "ui.h"

#define UI_BACKBUF      0xF52C8u   /* int32 virtual-buffer handle of the back buffer (D 0x3B839) */
#define UI_MOUSE_X      0xE1628u   /* int32 mouse x from fg_mousepos (D 0x78C43) */
#define UI_MOUSE_Y      0xE162Cu   /* int32 mouse y (D 0x78C64) */
#define UI_MOUSE_B      0xE1630u   /* int32 mouse buttons (D 0x78BD3) */
#define UI_CHOICE_COUNT 0xF7C9Cu   /* int32 count of the choice list (D 0x78CDA) */
#define UI_CHOICE_LIST  0xF7CA0u   /* int32[9] the choice list (D 0x78CE8) */
/* int8[50] 0xF8E44: ui.h calls it UI_SAVED_PRESSED, but 0x3C828 / 0x3C8C4 save and restore byte [dlg + 20i + 0x12],
   which is record i's +0x10, the ENABLED byte (D 0x3C877, 0x3C912), as in 0x3C940 / 0x3C7A0 (ui_kit2.c). */
#define UI_SAVED_ENABLED UI_SAVED_PRESSED

/* ---- Button setters (no redraw) ------------------------------------------------------------------------------- */

/* 0x3BB48: x (int16 rec+0x00) of the button id */
void f3BB48(uint32_t dlg, int id, int x)
{
    int32_t i, found = -1;

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                    /* 0x3BB61..0x3BBA1: movsx word [dlg]; jle */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {     /* cmp ax,dx (16 bits); jne */
            found = i;                                            /* 0x3BBA1, then out of the loop */
            break;
        }

    if (found == -1) {                                            /* 0x3BBA4 cmp [ebp-8],-1; jne */
        f3A7B0(0xB3C8Bu);                                         /* 0x3BBAF: "Button not found" */
        return;
    }
    setD16(dlg + DLG_REC(found) + BTN_X, (uint16_t)x);            /* 0x3BBCF word [dlg+20i+2] = ax */
}

/* 0x3BD80: w (int16 rec+0x04) of the button id */
void f3BD80(uint32_t dlg, int id, int w)
{
    int32_t i, found = -1;

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                    /* 0x3BD99..0x3BDD9: movsx word [dlg]; jle */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {     /* cmp ax,dx (16 bits); jne */
            found = i;                                            /* 0x3BDD9, then out of the loop */
            break;
        }

    if (found == -1) {                                            /* 0x3BDDC */
        f3A7B0(0xB3CCFu);                                         /* 0x3BDE7: "Button not found" */
        return;
    }
    setD16(dlg + DLG_REC(found) + BTN_W, (uint16_t)w);            /* 0x3BE07 word [dlg+20i+6] = ax */
}

/* 0x3BE10: h (int16 rec+0x06) of the button id */
void f3BE10(uint32_t dlg, int id, int h)
{
    int32_t i, found = -1;

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                    /* 0x3BE29..0x3BE69: movsx word [dlg]; jle */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {     /* cmp ax,dx (16 bits); jne */
            found = i;                                            /* 0x3BE69, then out of the loop */
            break;
        }

    if (found == -1) {                                            /* 0x3BE6C */
        f3A7B0(0xB3CE0u);                                         /* 0x3BE77: "Button not found" */
        return;
    }
    setD16(dlg + DLG_REC(found) + BTN_H, (uint16_t)h);            /* 0x3BE97 word [dlg+20i+8] = ax */
}

/* 0x3C0D8: style byte (rec+0x11) of the button id */
void f3C0D8(uint32_t dlg, int id, int style)
{
    int32_t i, found = -1;

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                    /* 0x3C0F1..0x3C131: movsx word [dlg]; jle */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {     /* cmp ax,dx (16 bits); jne */
            found = i;                                            /* 0x3C131, then out of the loop */
            break;
        }

    if (found == -1) {                                            /* 0x3C134 */
        f3A7B0(0xB3D35u);                                         /* 0x3C13F: "Button not found" */
        return;
    }
    setD8(dlg + DLG_REC(found) + BTN_STYLE, (uint8_t)style);      /* 0x3C15C mov al,[ebp-0x10]; 0x3C15F byte [dlg+20i+0x13] */
}

/* 0x3C284: label (uint32 rec+0x08, a link address) of the button id */
void f3C284(uint32_t dlg, int id, uint32_t label)
{
    int32_t i, found = -1;

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                    /* 0x3C29D..0x3C2DD: movsx word [dlg]; jle */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {     /* cmp ax,dx (16 bits); jne */
            found = i;                                            /* 0x3C2DD, then out of the loop */
            break;
        }

    if (found == -1) {                                            /* 0x3C2E0 */
        f3A7B0(0xB3D68u);                                         /* 0x3C2EB: "Button not found" */
        return;
    }
    setD32(dlg + DLG_REC(found) + BTN_LABEL, label);              /* 0x3C30B dword [dlg+20i+0xA] = eax */
}

/* ---- Button painters by id, in the back buffer, and the pressed-state reader ---------------------------------- */

/* 0x3B7CC: draw the button id pressed (f3B2D8 also sets rec+0x12 = 1) in the back buffer */
void f3B7CC(uint32_t dlg, int id)
{
    int32_t i, found = -1;

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                    /* 0x3B7E4..0x3B824: movsx word [dlg]; jle */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {     /* cmp ax,dx (16 bits); jne */
            found = i;                                            /* 0x3B824, then out of the loop */
            break;
        }

    if (found == -1) {                                            /* 0x3B827 */
        f3A7B0(0xB3C25u);                                         /* 0x3B832: "Button not found" */
        return;                                                   /* 0x3B837 jmp 0x3B869 */
    }
    fg_vbopen(S32(UI_BACKBUF));                                   /* 0x3B840 */
    f3B2D8(dlg + DLG_REC(found));                                 /* 0x3B85F: dlg + 2 + 20i */
    fg_vbclose();                                                 /* 0x3B864 */
}

/* 0x3B870: draw the button id up (f3B0D4 also sets rec+0x12 = 0) in the back buffer */
void f3B870(uint32_t dlg, int id)
{
    int32_t i, found = -1;

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                    /* 0x3B888..0x3B8C8: movsx word [dlg]; jle */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {     /* cmp ax,dx (16 bits); jne */
            found = i;                                            /* 0x3B8C8, then out of the loop */
            break;
        }

    if (found == -1) {                                            /* 0x3B8CB */
        f3A7B0(0xB3C36u);                                         /* 0x3B8D6: "Button not found" */
        return;                                                   /* 0x3B8DB jmp 0x3B90D */
    }
    fg_vbopen(S32(UI_BACKBUF));                                   /* 0x3B8E4 */
    f3B0D4(dlg + DLG_REC(found));                                 /* 0x3B903 */
    fg_vbclose();                                                 /* 0x3B908 */
}

/* 0x3B9A4: the pressed byte (rec+0x12) of the button id, sign-extended; 0 with the "Button not found" box if none */
int f3B9A4(uint32_t dlg, int id)
{
    int32_t i, found = -1;

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                    /* 0x3B9BB..0x3B9FB: movsx word [dlg]; jle */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {     /* cmp ax,dx (16 bits); jne */
            found = i;                                            /* 0x3B9FB, then out of the loop */
            break;
        }

    if (found == -1) {                                            /* 0x3B9FE */
        f3A7B0(0xB3C58u);                                         /* 0x3BA09: "Button not found" */
        return 0;                                                 /* 0x3BA0E xor eax,eax */
    }
    return S8(dlg + DLG_REC(found) + BTN_PRESSED);                /* 0x3BA24 dword [dlg+20i+0x11] sar 24 */
}

/* ---- Dialog disable / restore --------------------------------------------------------------------------------- */

/* 0x3C828: disable every button of dlg in the back buffer, saving each enabled byte (rec+0x10) to 0xF8E44[i] and
   redrawing the record (no pointer hiding; the back-buffer twin of f3C940) */
void f3C828(uint32_t dlg)
{
    int32_t i;   /* [ebp-0x10] */

    fg_vbopen(S32(UI_BACKBUF));                                   /* 0x3C845 */
    for (i = 0; S16(dlg + DLG_COUNT) > i; i++) {                  /* 0x3C85D movsx word [dlg]; 0x3C863 jle */
        /* 0x3C877..0x3C87D: byte [i + 0xF8E44] = byte [dlg + 20i + 0x12] (record i's +0x10) */
        setD8(UI_SAVED_ENABLED + (uint32_t)i, D8(dlg + DLG_REC(i) + BTN_ENABLED));
        setD8(dlg + DLG_REC(i) + BTN_ENABLED, 0);                 /* 0x3C895 */
        f3B4E0(dlg + DLG_REC(i));                                 /* 0x3C8B0: dlg + 2 + 20i */
    }
    fg_vbclose();                                                 /* 0x3C8B7 */
}

/* 0x3C8C4: undo f3C828 / f3C940 on the active surface: restore each enabled byte from 0xF8E44[i] and redraw the
   record, with the mouse pointer hidden (no fg_vbopen, unlike f3C7A0) */
void f3C8C4(uint32_t dlg)
{
    int32_t i;   /* [ebp-0x8] */

    f78884();                                                     /* 0x3C8DA: hide the pointer */
    for (i = 0; S16(dlg + DLG_COUNT) > i; i++) {                  /* 0x3C8DF i = edx (0); 0x3C8ED movsx; 0x3C8F3 jle */
        /* 0x3C90C..0x3C912: byte [dlg + 20i + 0x12] = byte [i + 0xF8E44] */
        setD8(dlg + DLG_REC(i) + BTN_ENABLED, D8(UI_SAVED_ENABLED + (uint32_t)i));
        f3B4E0(dlg + DLG_REC(i));                                 /* 0x3C92C */
    }
    f788AC();                                                     /* 0x3C933: restore the pointer */
}

/* ---- Text ----------------------------------------------------------------------------------------------------- */

/* 0x3A148: print value in decimal, centred in [x, x+w) with its top at y */
void f3A148(int x, int y, int w, int value)
{
    char buf[0x14];   /* [ebp-0x24] .. [ebp-0x10]: 20 bytes */

    snprintf(buf, sizeof buf, "%d", value);                       /* 0x3A16F itoa(ecx value, buf, 10) */
    f3A0B0(x, y, w, buf);                                         /* 0x3A180 */
}

/* 0x3A3A0: print s right-aligned with its right edge at x and its top at y */
void f3A3A0(int x, int y, const char *s)
{
    int32_t width;   /* [ebp-0xC] */

    if (strlen(s) <= 0x80) {                                      /* 0x3A3C8 cmp ecx,0x80; ja (unsigned) */
        width = fgf_width(s, (int)strlen(s));                     /* 0x3A3E2: 0x8A7DD */
        /* 0x3A3ED..0x3A400: fg_move(x - width, (dword [0xF8AFE] sar 16 = int16 [0xF8B00]) + y) */
        fg_move(x - width, S16(UI_FONT_ASCENT) + y);
        f3AD08(s);                                                /* 0x3A40B */
    } else {
        f3A044(x, y, DSTR(0xB3BAAu));                             /* 0x3A41D: "String too long" */
    }
}

/* ---- The generic click wait ----------------------------------------------------------------------------------- */

/* 0x3D2A4, the map hit test (mapdata.c, module MAP1), at the call site 0x78CC2 */
static int map_hit_3D2A4(int x, int y)
{
    return f3D2A4(x, y);
}

/* 0x78BA4: wait for a click; on the first enabled button of dlg under the pointer flash it (f3C5D4) and return
   f3FB94(its id). With map == 1 and no button hit, a click on the map (y <= 0x145) returns the index of the clicked
   territory in the choice list 0xF7CA0, if it is there. Otherwise -100. */
int f78BA4(uint32_t dlg, int map)
{
    int32_t onmap;   /* [ebp-0x14] */
    int32_t code;    /* [ebp-0x20] */
    int32_t i;       /* [ebp-0x24] */
    uint32_t rec;    /* [ebp-0x1C] */
    int32_t t;       /* [ebp-0x18] */

    onmap = 0;                                                    /* 0x78BC0 */
    if (map == 1)                                                 /* 0x78BC6 cmp ebx,1; jne */
        onmap = map;                                              /* 0x78BCB */
    f775F0();                                                     /* 0x78BCE: wait until no button is down */
    while (S32(UI_MOUSE_B) == 0) {                                /* 0x78BD3 cmp dword [0xE1630],0; jne */
        int mx, my, mb;
        fg_mousepos(&mx, &my, &mb);                               /* 0x78BEB fg_mousepos(0xE1628, 0xE162C, 0xE1630) */
        setD32(UI_MOUSE_X, (uint32_t)mx);
        setD32(UI_MOUSE_Y, (uint32_t)my);
        setD32(UI_MOUSE_B, (uint32_t)mb);
    }

    code = -100;                                                  /* 0x78BF5 mov [ebp-0x20],0xFFFFFF9C */
    for (i = 0; S16(dlg + DLG_COUNT) > i; i++) {                  /* 0x78C0C movsx word [dlg]; 0x78C12 jle */
        rec = dlg + DLG_REC(i);                                   /* 0x78C1B..0x78C2F: dlg + 2 + 20i */
        if (S8(rec + BTN_ENABLED) != 1)                           /* 0x78C35 dword [rec+0xD] sar 24; cmp 1; jne */
            continue;
        /* 0x78C43..0x78C7E, signed compares: x <= mx < x + w, y <= my < y + h */
        if (S16(rec + BTN_X) > S32(UI_MOUSE_X))                                   /* 0x78C4C jg */
            continue;
        if (S16(rec + BTN_X) + S16(rec + BTN_W) <= S32(UI_MOUSE_X))               /* 0x78C53 [rec+2] sar 16; 0x78C5D jle */
            continue;
        if (S16(rec + BTN_Y) > S32(UI_MOUSE_Y))                                   /* 0x78C62 [rec] sar 16; 0x78C6F jg */
            continue;
        if (S16(rec + BTN_Y) + S16(rec + BTN_H) <= S32(UI_MOUSE_Y))               /* 0x78C74 [rec+4] sar 16; 0x78C7E jle */
            continue;
        f3C5D4(dlg, S16(rec + BTN_ID));                           /* 0x78C83 [rec+0xC] sar 16; 0x78C8C */
        code = f3FB94(S16(rec + BTN_ID));                         /* 0x78C94..0x78C9F */
        onmap = 0;                                                /* 0x78CA4 */
        break;                                                    /* falls into 0x78CA7: out of the loop */
    }

    if (onmap != 0 && S32(UI_MOUSE_Y) <= 0x145) {                 /* 0x78CA7 je; 0x78CB3 cmp esi,0x145; jg (signed) */
        t = map_hit_3D2A4(S32(UI_MOUSE_X), S32(UI_MOUSE_Y));      /* 0x78CC2 f3D2A4([0xE1628], esi) */
        for (i = 0; i < S32(UI_CHOICE_COUNT); i++)                /* 0x78CDA cmp [0xF7C9C]; jge (signed) */
            if (t == S32(UI_CHOICE_LIST + 4u * (uint32_t)i)) {    /* 0x78CE8 [4*i + 0xF7CA0]; 0x78CEF cmp; jne */
                code = i;                                         /* 0x78CF6, then out of the loop (0x78CF9) */
                break;
            }
    }
    return code;                                                  /* 0x78CF9 */
}
