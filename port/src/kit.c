/* Module KIT of docs/M6_PLAN.md: message boxes, YES/NO, button-by-id helpers, map-box clears.
     0x79270 the game-screen message, 0x795A8 the YES/NO question, 0x7983C the CONTINUE message box, 0x7844C the
     player-button highlight; by button id: 0x3B728, 0x3B914, 0x3BA30, 0x3BABC, 0x3BBD8, 0x3BCF4, 0x3BEA0, 0x3C1F4,
     0x3C48C, 0x3C530; whole dialog: 0x3C6A0, 0x3C720; 0x3F6E4 / 0x3F758 (token / ship box colours cleared), 0x448EC
     (a rectangle recoloured through 0xC2720).
   Literal translations; the address of each instruction a line comes from is in its comment. Prototypes: kit.h.
   Grades: D = read in the code at the address given, G = inference. Watcom's qsort 0x8C331 is wqsort.c.

   The by-id routines all search the same way (D, e.g. 0x3B740..0x3B780): found = -1; for i < int16 [dlg] (movsx,
   signed jle): if word [dlg + 20i + 0x10] (record i's +0x0E, the id) == (uint16)id (cmp ax,dx), found = i and the code
   falls out of the loop (the first match wins). found == -1 -> the "Button not found" box (f3A7B0), each routine with
   its own copy of the string (0xB3C14, 0xB3C47, 0xB3C69, 0xB3C7A, 0xB3C9C, 0xB3CBE, 0xB3CF1, 0xB3D57, 0xB3D9B,
   0xB3DAC). Each routine keeps its own copy of the loop here, as ui_kit3.c does. They address record i's fields as
   dlg + 20i + (field + 2).

   Watcom C runtime -> standard C:
     0x7DB3D sprintf(buf, "zeus.wav") (a data format without conversions) -> snprintf(buf, size, "%s", ...)
     0x7D5D0 memset -> memset; repne scasb / not / dec (inline strlen) -> strlen
   __CHK dropped. Library calls through fg.h in the original argument order.

   fg_makegif / fg_showgif "junk" (port choice, as info.c and menu1.c): 0x795A8 and 0x7983C save the screen under the
   box with fg_makegif(x, x + w - 1, y, y + h - 1, "junk") (0x8CE50, not in fg.c) and put it back with fg_move(x, y),
   fg_showgif("junk", 3) on the same surface (no virtual buffer is open at either call: the box's fg_vbopen is closed
   before, D). The port keeps the rectangle in the shared slot of junk.c (the same method as info.c's: fg_getpixel /
   fg_point on the active surface, colour and cursor left alone). The pixels end up the same (GIF87a is lossless and
   the palette is kept). No file "junk" is left in the game folder. The original shares ONE file "junk" among 0x795A8,
   0x7983C, 0x19C10, 0x4290C, 0x76798, 0x3FC75, 0x199AE... exactly as the port now shares the one slot; no nesting is
   known (0x1A124 uses "junk2" for exactly that reason).

   Data (D, read in the code at the addresses given):
     int16 0xBB3FA / 0xBB3FC / 0xBB3FE / 0xBB400 the message area x, y, w, h (0x79291 reads them as dword [a-2] sar 16)
     0xF596C the map screen's panel dialog; 0xBB800 CONTINUE (one record, id 0x2A); 0xBBFD8 YES (0xCA) / NO (0xCB);
       0xBBBEC CONTINUE (0x2A, label set here)
     [0xF8E78] the current dialog (f3B05C stores it); [0xF52C8] back buffer, [0xF52CC] background buffer; int8
       [0xEE168] the panel's background number; uint8 [0xF097C] aciv.cfg byte 0 (bit 0x04 animations); int32 [0xF783C]
       the calamity; int16 [0xF40BA] the self-running demo's player count; int32 [0xF40EC] players; int16 0xF06B0[p]
       the nation of player p (read as dword [2p + 0xF06AE] sar 16); int32 [0xF40F8] the current nation
     0xFFC00 / 0xFFCA0 uint8[160] token / ship box colours (mapdata.c's f3F698 / f3F70C); 0xC2720 uint8[256] a colour map
   Heap pointers in data fields: none.

   Module ANIM (anim.c, anim.h): 0x79270 with anim != 0 calls f7A0A0 (a calamity animation, animations on), f79ADC
     (Zeus, -298, animations on) or f79C70 (the BRPC picture, -299, on or off).

   1995 bugs: none that needed a g_original branch. Port guard (no g_original branch: the original's path cannot be
   reproduced):
     - 0x7983C passes [0xF8E78] to f3C940 (0x79930) and f3C8C4 (0x79A5F) without the null test 0x795A8 makes
       (0x796A9, 0x797F6). With no current dialog the original reads a count from linear address 0 (the real-mode
       interrupt table under DOS/4GW) and clears bytes there and past 0xF8E44[50] (D: the code; G: the effect). The port
       skips both calls when [0xF8E78] is 0. G: unreachable in play (its callers 0x54824 and 0x6D178 run with the panel
       dialog drawn).
   Oddities kept (D):
     - Box width (0x795A8 / 0x7983C, as 0x76798 in info.c): W = max(text width + 10, 180), then "W - 20 > 350 -> W = 350":
       a box of 371 or more shrinks to 350, while 351..370 stay. G: 370 was meant. Texts wider than 360 px get the
       narrower box (and wrap into more lines; the height follows the wrap, so nothing is cut).
     - The text is wrapped to W - 20 = max(text width - 10, 160) (W = max(text width + 10, 180)), and f3A42C breaks a
       line once its width reaches w, so a one-line text of 160 px or more always wraps its last word (D for the
       arithmetic; G for the exact threshold, as f3A42C sums f3A748's widths, not fgf_width's; deliberate or not, the
       frame is the original's).
     - 0x79270 with wait -560 still calls f78A20(-560), which returns at once (game.h), and f3C828 / f3C7A0 on the
       panel dialog.
     - 0x795A8 has no unattended check: a self-running demo would wait for a click (G: its callers ask humans only). */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "anim.h"
#include "fg.h"
#include "game.h"
#include "junk.h"
#include "kit.h"
#include "map.h"
#include "mapdata.h"
#include "panel.h"
#include "state.h"
#include "ui.h"

#define BACKBUF   0xF52C8u   /* int32 virtual-buffer handle of the back buffer */
#define BGBUF     0xF52CCu   /* int32 handle of the background-picture buffer */
#define PANELBG   0xEE168u   /* int8 the panel's background number */
#define CURDLG    0xF8E78u   /* uint32 the current dialog */
#define AREA_Y    0xBB3FCu   /* int16 message area y */
#define AREA_H    0xBB400u   /* int16 message area h */
#define PANEL_DLG 0xF596Cu   /* the map screen's panel dialog */
#define CONT_DLG  0xBB800u   /* CONTINUE (id 0x2A) appended to the panel dialog */
#define YESNO_DLG 0xBBFD8u   /* YES 0xCA / NO 0xCB */
#define MSG_DLG   0xBBBECu   /* CONTINUE 0x2A of the message box */
#define CFG0      0xF097Cu   /* uint8 aciv.cfg byte 0: bit 0x04 animations */
#define CALAMITY  0xF783Cu   /* int32 the calamity */
#define SRD       0xF40BAu   /* int16 self-running demo player count */
#define PLAYERS   0xF40ECu   /* int32 players */
#define PLAYER_NAT 0xF06B0u  /* int16[] nation of player p */
#define CUR_NAT   0xF40F8u   /* int32 the current nation */

/* ---- 0x79270: the game-screen message --------------------------------------------------------------------------- */

void f79270(int wait, int n, int t, const char *text, int anim)
{
    int32_t h0, y0;                                                 /* [ebp-0x18], [ebp-0x14] */
    int32_t r;                                                      /* [ebp-0x10] */
    char buf[0x80];                                                 /* [ebp-0x88] */

    h0 = S16(AREA_H);                                               /* 0x79291 dword [0xBB3FE] sar 16 */
    y0 = S16(AREA_Y);                                               /* 0x7929C dword [0xBB3FA] sar 16 */
    setD16(AREA_Y, 0x14C);                                          /* 0x792B1 */
    setD16(AREA_H, 0x7B);                                           /* 0x792B8 */
    if (wait != -560) {                                             /* 0x792BF cmp 0xFFFFFDD0; je */
        f3CD6C(3);                                                  /* 0x792CD */
        setD8(PANELBG, 3);                                          /* 0x792F7 mov byte [0xEE168], ah (after the call) */
        fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BGBUF), S32(BACKBUF));   /* 0x792FD */
        f7876C();                                                   /* 0x79305 */
    } else
        f3A990();                                                   /* 0x7930C */
    if (t != -300 && wait != -560)                                  /* 0x79311 cmp 0xFFFFFED4; 0x7931D */
        f459E4(n);                                                  /* 0x79328 */
    if (wait != -500)                                               /* 0x7932D cmp 0xFFFFFE0C; je */
        f3C828(PANEL_DLG);                                          /* 0x7933B */
    if (wait == -500) {                                             /* 0x79340 jne */
        f7853C(CONT_DLG);                                           /* 0x79358 (edx 0x2A, ebx 0x109 loaded for the next calls) */
        f3BB48(PANEL_DLG, 0x2A, 0x109);                             /* 0x79362 (edx, ebx kept across f7853C) */
        f3BC64(PANEL_DLG, 0x2A, 0x1AC);                             /* 0x79376 */
    }
    if (wait != -560)                                               /* 0x7937B */
        f3B05C(PANEL_DLG);                                          /* 0x79389 */
    if (strlen(text) > 0)                                           /* 0x79396 repne scasb; 0x7939B test; jbe */
        f3A9F4(text);                                               /* 0x793A2 */

    if (t == -200) {                                                /* 0x793A7 cmp 0xFFFFFF38; jne */
        if (wait != -560) {                                         /* 0x793B3 */
            f3F670();                                               /* 0x793BF */
            f3D9FC(n);                                              /* 0x793C7 */
        }
    } else if (t == -300 || t == -250) {                            /* 0x793D1, 0x793D9 cmp 0xFFFFFF06 */
        if (wait != -560)                                           /* 0x793E5 */
            f3D52C(0, 0);                                           /* 0x793F1 */
    } else if (wait != -560) {                                      /* 0x793FB */
        if (t >= 0)                                                 /* 0x79406 test; jl */
            f3F670();                                               /* 0x7940A */
        else
            t = t * -1;                                             /* 0x79414 imul eax, esi, -1 */
        f3F624(t, 0x0F);                                            /* 0x79422 */
        f3DA60(t);                                                  /* 0x7942A */
    }
    f3CBBC();                                                       /* 0x7942F */

    if (anim != 0) {                                                /* 0x79434 cmp [ebp+8], 0; je */
        if (D8(CFG0) & 0x04) {                                      /* 0x7943E test dl, 4; je */
            if (anim != -299 && anim != -298) {                     /* 0x79450 cmp 0xFFFFFED5, 0x79457 0xFFFFFED6 */
                f7A0A0(S32(CALAMITY));                              /* 0x79465 f7A0A0([0xF783C]) (ANIM) */
            } else if (anim == -298) {                              /* 0x79472 */
                snprintf(buf, sizeof buf, "%s", DSTR(0xB9558u));    /* 0x79486 sprintf(buf, "zeus.wav") */
                f79ADC(DSTR(0xB9561u), buf);                        /* 0x79499 f79ADC("zeus.flc", buf) (ANIM) */
            } else if (anim == -299) {                              /* 0x794A3 */
                buf[0] = 0;                                         /* 0x794AD */
                f79C70(DSTR(0xB956Au), buf);                        /* 0x794BE f79C70("brpc.gif", buf = "") (ANIM) */
            }
        } else if (anim != 1 && anim != 2) {                        /* 0x794C8 cmp 1, 0x794D0 cmp 2 */
            if (anim == -299)                                       /* 0x794D8 */
                f79C70(DSTR(0xB9574u), DSTR(0xB9573u));             /* 0x794E9 f79C70("brpc.gif", "") (ANIM) */
            /* 0x794F3: cmp 0xFFFFFED6 with nothing after it: -298 shows nothing with animations off */
        }
    }

    if (wait == -550)                                               /* 0x794F9 cmp 0xFFFFFDDA; jne */
        f3C7A0(PANEL_DLG);                                          /* 0x79507 */
    else if (wait != -500) {                                        /* 0x79514 */
        f78A20(wait);                                               /* 0x7951F */
        f3C7A0(PANEL_DLG);                                          /* 0x79529 */
    } else {                                                        /* 0x79533 cmp esi, ecx: always equal here */
        do {
            if (f7BA64() == 0)                                      /* 0x79537 test; jne */
                r = f78BA4(PANEL_DLG, 0);                           /* 0x79547 */
            else {
                f78A20(S16(SRD) == 0 ? 0xFA : 0x3E8);               /* 0x79551 cmp word [0xF40BA], 0; 0x7956C */
                r = -1;                                             /* 0x79567 mov edi, -1; 0x79571 */
            }
        } while (r == -100 || r != -1);                             /* 0x79574 cmp -0x64; je; 0x7957D cmp -1; jne */
        f78664(CONT_DLG);                                           /* 0x79587 */
    }
    setD16(AREA_H, (uint16_t)h0);                                   /* 0x7958F */
    setD16(AREA_Y, (uint16_t)y0);                                   /* 0x79598 */
}

/* ---- 0x795A8: the YES/NO question ------------------------------------------------------------------------------- */

int f795A8(const char *text)
{
    int32_t tw, W, H, X, Y, bx;                                     /* [ebp-0x2C], [ebp-0x28], [ebp-0x1C], [ebp-0x38], [ebp-0x34], [ebp-0x30] */
    int32_t maxw;                                                   /* [ebp-0x20] */
    int32_t r;                                                      /* [ebp-0x24] */
    uint32_t saved;                                                 /* [ebp-0x18] */

    tw = fgf_width(text, (int)strlen(text)) + 0x0A;                 /* 0x795C8 repne scasb; 0x795D2; 0x795DA */
    W = tw > 0xB4 ? tw : 0xB4;                                      /* 0x795E0 cmp 0xB4; jle (signed) */
    if (W - 0x14 > 0x15E)                                           /* 0x795F9 sub 0x14; cmp 0x15E; jle */
        W = 0x15E;                                                  /* 0x79603 (kept: file comment) */
    H = f3A42C(0, 0, W - 0x14, 0, text, &maxw, 0);                  /* 0x79620 measure */
    H += 0x41;                                                      /* 0x79628 */
    X = (0x280 - W) / 2;                                            /* 0x7962C..0x7963D sar (signed /2) */
    Y = (0x1E0 - H) / 2;                                            /* 0x79642..0x79653 */
    bx = X + (W - 0xA0) / 2;                                        /* 0x79658..0x7966D */
    f78884();                                                       /* 0x79672 */
    junk_makegif(X, X + W - 1, Y, Y + H - 1, DSTR(0xB957Du));       /* 0x79694 fg_makegif(.., "junk" 0xB957D) (port: junk.c) */
    f788AC();                                                       /* 0x7969C */
    saved = D32(CURDLG);                                            /* 0x796A1 */
    if (saved != 0)                                                 /* 0x796A9 test; je */
        f3C940(saved);                                              /* 0x796AD */
    fg_vbopen(S32(BACKBUF));                                        /* 0x796B9 */
    f3C9D0(X, Y, W, H, 3);                                          /* 0x796CF */
    fg_setcolor(5);                                                 /* 0x796D6 */
    f3A42C(X + 0x0A, Y + 0x0A, W - 0x14, 2, text, &maxw, 1);        /* 0x796FF centred */
    f3BB48(YESNO_DLG, 0xCA, bx);                                    /* 0x79711 */
    f3BC64(YESNO_DLG, 0xCA, Y + H - 0x1D);                          /* 0x7972B */
    f3C284(YESNO_DLG, 0xCA, 0xB9582u);                              /* 0x7973F "YES" */
    f3BB48(YESNO_DLG, 0xCB, bx);                                    /* 0x79751 */
    f3BC64(YESNO_DLG, 0xCB, Y + H - 0x31);                          /* 0x7976B (NO above YES) */
    f3C284(YESNO_DLG, 0xCB, 0xB9586u);                              /* 0x7977F "NO" */
    f3B05C(YESNO_DLG);                                              /* 0x79789 */
    fg_vbclose();                                                   /* 0x7978E */
    f78884();                                                       /* 0x79793 */
    fg_vbpaste(X, X + W - 1, Y, Y + H - 1, X, Y + H - 1);           /* 0x797BC */
    f788AC();                                                       /* 0x797C4 */
    do
        r = f78BA4(YESNO_DLG, 0);                                   /* 0x797D0 */
    while (r == -100 || (r != -0x61 && r != -0x62));                /* 0x797DB, 0x797E3, 0x797E8 */
    setD32(CURDLG, saved);                                          /* 0x797F1 */
    if (saved != 0)                                                 /* 0x797F6 */
        f3C8C4(saved);                                              /* 0x797FA */
    f78884();                                                       /* 0x797FF */
    fg_move(X, Y);                                                  /* 0x7980C */
    junk_showgif(DSTR(0xB9589u), 3);                                /* 0x7981B fg_showgif("junk" 0xB9589, 3) (port: junk.c) */
    f3CCBC();                                                       /* 0x79823 */
    f788AC();                                                       /* 0x79828 */
    return r;                                                       /* 0x7982D */
}

/* ---- 0x7983C: the CONTINUE message box -------------------------------------------------------------------------- */

void f7983C(const char *text)
{
    int32_t tw, W, H, X, Y, bx;                                     /* [ebp-0x2C], [ebp-0x28], [ebp-0x1C], [ebp-0x38], [ebp-0x34], [ebp-0x30] */
    int32_t maxw;                                                   /* [ebp-0x20] */
    int32_t r;                                                      /* [ebp-0x24] */
    uint32_t saved;                                                 /* [ebp-0x18] */

    tw = fgf_width(text, (int)strlen(text)) + 0x0A;                 /* 0x7985C; 0x79866; 0x7986E */
    W = tw > 0xB4 ? tw : 0xB4;                                      /* 0x79874 cmp 0xB4; jle */
    if (W - 0x14 > 0x15E)                                           /* 0x7988D */
        W = 0x15E;                                                  /* 0x79897 (kept) */
    H = f3A42C(0, 0, W - 0x14, 0, text, &maxw, 0);                  /* 0x798B4 measure */
    H += 0x2D;                                                      /* 0x798BC */
    X = (0x280 - W) / 2;                                            /* 0x798C0..0x798D1 */
    Y = (0x1E0 - H) / 2;                                            /* 0x798D6..0x798E7 */
    bx = X + (W - 0x6E) / 2;                                        /* 0x798EC..0x79900 */
    junk_makegif(X, X + W - 1, Y, Y + H - 1, DSTR(0xB958Eu));       /* 0x79920 fg_makegif(.., "junk" 0xB958E) (port: junk.c): no f78884 here */
    saved = D32(CURDLG);                                            /* 0x79925 */
    if (saved != 0)                                                 /* port guard: the original has no test (file comment) */
        f3C940(saved);                                              /* 0x79930 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x7993C */
    f3C9D0(X, Y, W, H, 3);                                          /* 0x79952 */
    fg_setcolor(5);                                                 /* 0x79959 */
    f3A42C(X + 0x0A, Y + 0x0A, W - 0x14, 2, text, &maxw, 1);        /* 0x79982 centred */
    f3BB48(MSG_DLG, 0x2A, bx);                                      /* 0x79994 */
    f3BC64(MSG_DLG, 0x2A, Y + H - 0x1D);                            /* 0x799AE */
    f3C284(MSG_DLG, 0x2A, 0xB9593u);                                /* 0x799C2 "CONTINUE" */
    f3B05C(MSG_DLG);                                                /* 0x799CC */
    fg_vbclose();                                                   /* 0x799D1 */
    f78884();                                                       /* 0x799D6 */
    fg_vbpaste(X, X + W - 1, Y, Y + H - 1, X, Y + H - 1);           /* 0x799FF */
    f788AC();                                                       /* 0x79A07 */
    do {
        if (f7BA64() == 0)                                          /* 0x79A0C test; jne */
            r = f78BA4(MSG_DLG, 0);                                 /* 0x79A1C */
        else {
            f78A20(S16(SRD) == 0 ? 0xFA : 0x3E8);                   /* 0x79A26 cmp word [0xF40BA], 0; 0x79A41 */
            r = -1;                                                 /* 0x79A3C mov ecx, -1; 0x79A46 */
        }
    } while (r == -100 || r != -1);                                 /* 0x79A49, 0x79A52 */
    setD32(CURDLG, saved);                                          /* 0x79A5A */
    if (saved != 0)                                                 /* port guard, as above */
        f3C8C4(saved);                                              /* 0x79A5F */
    f78884();                                                       /* 0x79A64 */
    fg_move(X, Y);                                                  /* 0x79A71 */
    junk_showgif(DSTR(0xB959Cu), 3);                                /* 0x79A80 fg_showgif("junk" 0xB959C, 3) (port: junk.c) */
    f3CCBC();                                                       /* 0x79A88 */
    f788AC();                                                       /* 0x79A8D */
}

/* ---- 0x7844C: the player buttons' highlight --------------------------------------------------------------------- */

void f7844C(uint32_t dlg, int base)
{
    int32_t p;                                                      /* [ebp-0xC] */
    uint32_t rec;

    fg_vbopen(S32(BACKBUF));                                        /* 0x7846B */
    for (p = 0; p < S32(PLAYERS); p++) {                            /* 0x78473, 0x784A6 cmp [0xF40EC]; jge */
        rec = dlg + 20u * (uint32_t)(base + p);                     /* 0x784BB..0x784D6 */
        if ((int32_t)S16(PLAYER_NAT + 2u * (uint32_t)p) == S32(CUR_NAT))   /* 0x784B1 dword [2p + 0xF06AE] sar 16; 0x784D8 */
            setD8(rec + 0x0E, 0x0F);                                /* 0x7847A byte [dlg + 20k + 0xE]: record k's +0x0C */
        else
            setD8(rec + 0x0E, 0x00);                                /* 0x784DC */
        f3B4E0(rec + 2u);                                           /* 0x78496 add eax, edx (dlg + 2); 0x78498 */
    }
    fg_vbclose();                                                   /* 0x784FA */
}

/* ---- By button id ----------------------------------------------------------------------------------------------- */

/* 0x3B728: redraw the button id in the back buffer */
void f3B728(uint32_t dlg, int id)
{
    int32_t i, found = -1;                                          /* [ebp-0x14], [ebp-0x10] */

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                      /* 0x3B754 movsx word [dlg]; jle */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {       /* 0x3B771 mov ax, [dlg + 20i + 0x10]; cmp ax,dx */
            found = i;                                              /* 0x3B780 */
            break;
        }
    if (found == -1) {                                              /* 0x3B783 */
        f3A7B0(0xB3C14u);                                           /* 0x3B78E "Button not found" */
        return;
    }
    fg_vbopen(S32(BACKBUF));                                        /* 0x3B79C */
    f3B4E0(dlg + DLG_REC(found));                                   /* 0x3B7BB */
    fg_vbclose();                                                   /* 0x3B7C0 */
}

/* 0x3B914: the pressed byte rec+0x12 */
void f3B914(uint32_t dlg, int id, int pressed)
{
    int32_t i, found = -1;                                          /* [ebp-0xC], [ebp-0x8] */

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                      /* 0x3B941 */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {       /* 0x3B95E */
            found = i;                                              /* 0x3B96D */
            break;
        }
    if (found == -1) {                                              /* 0x3B970 */
        f3A7B0(0xB3C47u);                                           /* 0x3B97B */
        return;
    }
    setD8(dlg + DLG_REC(found) + BTN_PRESSED, (uint8_t)pressed);    /* 0x3B99B mov byte [dlg + 20i + 0x14], bl */
}

/* 0x3BA30: the width (ax only, D: callers movsx the word) */
int f3BA30(uint32_t dlg, int id)
{
    int32_t i, found = -1;                                          /* [ebp-0x10], [ebp-0xC] */

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                      /* 0x3BA5B */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {       /* 0x3BA78 */
            found = i;                                              /* 0x3BA87 */
            break;
        }
    if (found == -1) {                                              /* 0x3BA8A */
        f3A7B0(0xB3C69u);                                           /* 0x3BA95 */
        return 0;                                                   /* 0x3BA9A xor eax, eax */
    }
    return S16(dlg + DLG_REC(found) + BTN_W);                       /* 0x3BAB0 mov ax, [dlg + 20i + 6] */
}

/* 0x3BABC: the height */
int f3BABC(uint32_t dlg, int id)
{
    int32_t i, found = -1;                                          /* [ebp-0x10], [ebp-0xC] */

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                      /* 0x3BAE7 */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {       /* 0x3BB04 */
            found = i;                                              /* 0x3BB13 */
            break;
        }
    if (found == -1) {                                              /* 0x3BB16 */
        f3A7B0(0xB3C7Au);                                           /* 0x3BB21 */
        return 0;                                                   /* 0x3BB26 */
    }
    return S16(dlg + DLG_REC(found) + BTN_H);                       /* 0x3BB3C mov ax, [dlg + 20i + 8] */
}

/* 0x3BBD8: x */
int f3BBD8(uint32_t dlg, int id)
{
    int32_t i, found = -1;                                          /* [ebp-0x10], [ebp-0xC] */

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                      /* 0x3BC03 */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {       /* 0x3BC20 */
            found = i;                                              /* 0x3BC2F */
            break;
        }
    if (found == -1) {                                              /* 0x3BC32 */
        f3A7B0(0xB3C9Cu);                                           /* 0x3BC3D */
        return 0;                                                   /* 0x3BC42 */
    }
    return S16(dlg + DLG_REC(found) + BTN_X);                       /* 0x3BC58 mov ax, [dlg + 20i + 2] */
}

/* 0x3BCF4: y */
int f3BCF4(uint32_t dlg, int id)
{
    int32_t i, found = -1;                                          /* [ebp-0x10], [ebp-0xC] */

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                      /* 0x3BD1F */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {       /* 0x3BD3C */
            found = i;                                              /* 0x3BD4B */
            break;
        }
    if (found == -1) {                                              /* 0x3BD4E */
        f3A7B0(0xB3CBEu);                                           /* 0x3BD59 */
        return 0;                                                   /* 0x3BD5E */
    }
    return S16(dlg + DLG_REC(found) + BTN_Y);                       /* 0x3BD74 mov ax, [dlg + 20i + 4] */
}

/* 0x3BEA0: the label colour byte rec+0x0C */
void f3BEA0(uint32_t dlg, int id, int color)
{
    int32_t i, found = -1;                                          /* [ebp-0xC], [ebp-0x8] */

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                      /* 0x3BECD */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {       /* 0x3BEEA */
            found = i;                                              /* 0x3BEF9 */
            break;
        }
    if (found == -1) {                                              /* 0x3BEFC */
        f3A7B0(0xB3CF1u);                                           /* 0x3BF07 */
        return;
    }
    setD8(dlg + DLG_REC(found) + BTN_COLOR, (uint8_t)color);        /* 0x3BF27 mov byte [dlg + 20i + 0xE], bl */
}

/* 0x3C1F4: the no-flash byte rec+0x0D */
void f3C1F4(uint32_t dlg, int id, int noflash)
{
    int32_t i, found = -1;                                          /* [ebp-0xC], [ebp-0x8] */

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                      /* 0x3C221 */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {       /* 0x3C23E */
            found = i;                                              /* 0x3C24D */
            break;
        }
    if (found == -1) {                                              /* 0x3C250 */
        f3A7B0(0xB3D57u);                                           /* 0x3C25B */
        return;
    }
    setD8(dlg + DLG_REC(found) + BTN_NOFLASH, (uint8_t)noflash);    /* 0x3C27B mov byte [dlg + 20i + 0xF], bl */
}

/* 0x3C48C: enable the button id and redraw it on the active surface */
void f3C48C(uint32_t dlg, int id)
{
    int32_t i, found = -1;                                          /* [ebp-0x10], [ebp-0xC] */

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                      /* 0x3C4B7 */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {       /* 0x3C4D4 */
            found = i;                                              /* 0x3C4E3 */
            break;
        }
    if (found == -1) {                                              /* 0x3C4E6 */
        f3A7B0(0xB3D9Bu);                                           /* 0x3C4F1 */
        return;
    }
    setD8(dlg + DLG_REC(found) + BTN_ENABLED, 1);                   /* 0x3C50A mov byte [dlg + 20i + 0x12], 1 */
    f3B4E0(dlg + DLG_REC(found));                                   /* 0x3C525 */
}

/* 0x3C530: disable the button id and redraw it on the active surface */
void f3C530(uint32_t dlg, int id)
{
    int32_t i, found = -1;                                          /* [ebp-0x10], [ebp-0xC] */

    for (i = 0; S16(dlg + DLG_COUNT) > i; i++)                      /* 0x3C55B */
        if (D16(dlg + DLG_REC(i) + BTN_ID) == (uint16_t)id) {       /* 0x3C578 */
            found = i;                                              /* 0x3C587 */
            break;
        }
    if (found == -1) {                                              /* 0x3C58A */
        f3A7B0(0xB3DACu);                                           /* 0x3C595 */
        return;
    }
    setD8(dlg + DLG_REC(found) + BTN_ENABLED, 0);                   /* 0x3C5AE mov byte [dlg + 20i + 0x12], 0 */
    f3B4E0(dlg + DLG_REC(found));                                   /* 0x3C5C9 */
}

/* ---- Whole dialog ----------------------------------------------------------------------------------------------- */

/* 0x3C6A0: enable every button in the back buffer */
void f3C6A0(uint32_t dlg)
{
    int32_t i;                                                      /* [ebp-0x10] */

    fg_vbopen(S32(BACKBUF));                                        /* 0x3C6BD */
    for (i = 0; S16(dlg + DLG_COUNT) > i; i++) {                    /* 0x3C6D2 movsx; jle */
        setD8(dlg + DLG_REC(i) + BTN_ENABLED, 1);                   /* 0x3C6EF */
        f3B4E0(dlg + DLG_REC(i));                                   /* 0x3C70A */
    }
    fg_vbclose();                                                   /* 0x3C711 */
}

/* 0x3C720: disable every button in the back buffer */
void f3C720(uint32_t dlg)
{
    int32_t i;                                                      /* [ebp-0x10] */

    fg_vbopen(S32(BACKBUF));                                        /* 0x3C73D */
    for (i = 0; S16(dlg + DLG_COUNT) > i; i++) {                    /* 0x3C752 */
        setD8(dlg + DLG_REC(i) + BTN_ENABLED, 0);                   /* 0x3C76F */
        f3B4E0(dlg + DLG_REC(i));                                   /* 0x3C78A */
    }
    fg_vbclose();                                                   /* 0x3C791 */
}

/* ---- Map boxes and the recolour ---------------------------------------------------------------------------------- */

/* 0x3F6E4: every token box colour cleared */
void f3F6E4(void)
{
    memset(DP(0xFFC00u), 0, 0xA0);                                  /* 0x3F6FF memset(0xFFC00, 0, 0xA0) */
}

/* 0x3F758: every ship box colour cleared */
void f3F758(void)
{
    memset(DP(0xFFCA0u), 0, 0xA0);                                  /* 0x3F773 memset(0xFFCA0, 0, 0xA0) */
}

/* 0x448EC: recolour the rectangle (x, y, w, h) on the active surface through 0xC2720, column by column */
void f448EC(int x, int y, int w, int h)
{
    int32_t i, j;                                                   /* [ebp-0x8], [ebp-0x4] */

    for (i = x; x + w > i; i++)                                     /* 0x44916 cmp; jle (signed) */
        for (j = y; y + h > j; j++) {                               /* 0x4492F */
            fg_setcolor(D8(0xC2720u + (uint32_t)fg_getpixel(i, j)));   /* 0x44942, 0x4494A mov al, [eax + 0xC2720] */
            fg_point(i, j);                                         /* 0x44966 */
        }
}
