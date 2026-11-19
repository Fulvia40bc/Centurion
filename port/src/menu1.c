/* Module MENU1 of docs/MAP_PLAN.md: the in-game options menu 0x42754 (the panel's SYSTEM button, dispatcher id 5),
   the HELP screen 0x4290C and its file/title picker 0x42CEC, the TOGGLES screen 0x436FC, and the bodies of the
   dispatcher 0x3FB94's cases 5, 0x52, 0x53, 0x57, 0x41, 0x6D, 0x6E, 0xE3..0xE9 as dcase_<id>() (dispatch.c is not
   edited here; Claude wires them in). Literal translations; the address of the instruction a line comes from is in
   its comment. D = read in the code, G = inference. Prototypes: menu1.h.

   Watcom runtime -> standard C:
     0x7DB3D sprintf -> snprintf bounded to the original's 100-byte stack buffer ([ebp-0xE0] .. [ebp-0x7D])
     0x822B5 malloc, 0x877E6 free -> malloc / free (the help text's block lives only in a local, no data field)
     0x7D8C8 fopen, 0x912D5 fwrite(buf eax, size edx, n ebx, fp ecx), 0x7D95E fclose -> stdio, as ui_screen.c's f44CAC
     inline strcpy loops (two bytes per turn) -> strcpy
     repne scasb (strlen) -> strlen
   __CHK dropped. Library calls through fg.h in the original argument order (Fastgraph pushes right to left).

   The 'junk' GIF (port design, as MAP_PLAN.md risk 7 proposes): 0x4290C saves the screen under the help box with
   fg_makegif(x, x + w - 1, y, y + h - 1, "junk") (0x42A8C) and puts it back with fg_move(x, y), fg_showgif("junk", 3)
   (0x42C2D: flag 2 = at the cursor, flag 1 = keep the palette) into the back buffer, which f3CC14 then shows. The
   file is written only to be read back. The port keeps the rectangle in memory instead (junk_save / junk_show below):
   the pixels come from the screen bytes (at 0x42A8C no virtual buffer is open: f3CC68 and f3CCBC in the dispatcher
   case close theirs, and nothing between opens one, D) and go back with fg_vbcopy into the back buffer at the same
   place, opaque and unclipped as fg_showgif writes them. GIF87a with 256 colours is lossless and the palette is kept,
   so the back buffer and the screen end up identical. Differences: no file "junk" is left in the game folder, and
   the port uses one Fastgraph virtual-buffer handle of its own for the copy (the game defines its three buffers at
   start-up only: fg_vbdefine 0x82398 is called at 0x10A8A, 0x10EFA, 0x10F68 and nowhere else, D).
     G, to check in DOSBox-X: fg_makegif at 0x42A8C is not bracketed by 0x78884/0x788AC (0x19C48 is), so in the
     original Fastgraph's SVGA pointer, if drawn into video memory, would be saved into "junk" and restored into the
     back buffer. Nothing of it can stay visible: the options menu's own f3CC68 (0x428EF) repaints the whole screen
     from the background buffer as soon as HELP returns -1.

   Data (D):
     0xF4102 uint8 phase (0..21, names at 0xCA9F4: TAXATION .. SELECT START); 0xFFD40 int32 the tax state (2 = tax
     revolt, -50 = tax rate change); int8 0x108804 (read as dword [0x108801] sar 24) 1 = token movement, else ships;
     uint8 0xEE168 (0x0C = counter offers); int32 0xF783C the calamity, 18..30 (0x431A0 sub edi, 0x12; table 0x42C60)
     0xF097C..0xF0981 the aciv.cfg image (6 bytes); 0xF8E78 the current dialog; [0xF52C8] back buffer, [0xF52CC]
     background buffer, int8 [0xF52D0] background id; [0xF40DC] ACIV.CT3's catalog handle
     dialogs (obj2): 0xC1A5C options (10 records: 0x4F SAVE GAME (200,90), 0x50 RESTORE GAME (200,115), 0x51 START
     NEW GAME (200,140), 0x56 SCORING (200,165), 0xE2 AUDIO/VISUALS (330,90), 0x52 TOGGLES (330,115), 0x53 HELP
     (330,140), 0x57 QUIT TO DOS (330,165), 0x2A CANCEL (265,190), 0x107 SHOW PBEM (200,190, disabled, style 2); all
     110 x 19); 0xBBBEC help (one record, 0x2A, 110 x 19, no label until 0x42BA6); 0xC1E48 toggles (rows 0x41, 0x6D,
     0x6E, 0xE3..0xE9 at (220, 91 + 20i), 200 x 9, style 2; 0x2A CONTINUE (265,290))
   Heap pointers in data fields: none.

   1995 bugs (fixed unless g_original):
     - 0x42CEC gives the two CITY SUPPORT phases (10 and 17) the Ship Construction help: both jump-table entries go to
       0x42FEE, which copies "shipcnst.hlp" / "Ship Construction" from a second pair of strings (0xB41AE, 0xB41BB;
       phase 4 has its own at 0xB4104 / 0xB4111). ACIV.CT3 holds CITYSUP.HLP (416 bytes, about city support), which
       no string of the game names (D). G: 0x42FEE is a copy of phase 4's case whose strings were never changed. The
       fix shows citysup.hlp with the title "City Support" (port text; the original has no such title string).
   Oddities kept or guarded (D):
     - 0x42CEC leaves both buffers untouched in phases 2 (CENSUS) and 9 (REMOVE SURPLUS), in TAXATION when the tax
       state is neither 2 nor -50, for a calamity outside 18..30 and for phases above 21. 0x4290C passes them
       uninitialised (stack) to cat_size and prints the title from them: garbage. The port starts both as empty
       strings (port guard; the garbage cannot be reproduced), so HELP there shows "Help file '' not found." under an
       empty title. G: whether HELP can be reached in those states.
     - 0x4290C uses the text pointer [ebp-0x1C] uninitialised when cat_size returns neither -2 nor a positive size
       (0 for an empty entry, other negative errors). The port points it at an empty string (port guard; G: never
       with the shipped ACIV.CT3, whose help entries are 388..2747 bytes).
     - Clicking TOGGLES, HELP (or SAVE, RESTORE, SCORING, AUDIO/VISUALS) ends the options menu too: those cases return
       -1, the menu's exit code (0x428E2).
     - 0x436FC writes aciv.cfg whenever a row was clicked, even if a second click put the bit back. */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fg.h"
#include "game.h"
#include "save.h"
#include "menu1.h"

/* ---- port: the "junk" GIF kept in memory ------------------------------------------------------------------------ */

static uint8_t s_junk[640 * 480];               /* the rectangle at its screen position */
static int s_junk_vb = -1;                      /* Fastgraph handle of s_junk, defined on first use */

/* fg_makegif(minx, maxx, miny, maxy, "junk") of the screen (no virtual buffer open) */
static void junk_save(int minx, int maxx, int miny, int maxy)
{
    const uint8_t *scr = fg_screen();
    int r;

    if (s_junk_vb < 0)
        s_junk_vb = fg_vbdefine(s_junk, 640, 480);
    for (r = miny; r <= maxy; r++)
        memcpy(s_junk + r * 640 + minx, scr + r * 640 + minx, (size_t)(maxx - minx + 1));
}

/* fg_showgif("junk", 3) with the cursor at (minx, miny) in the back buffer [0xF52C8]: the saved rectangle, opaque */
static void junk_show(int minx, int maxx, int miny, int maxy)
{
    if (s_junk_vb >= 0)
        fg_vbcopy(minx, maxx, miny, maxy, minx, maxy, s_junk_vb, S32(0xF52C8u));
}

/* ---- 0x42754: the options menu ------------------------------------------------------------------------------------ */

void f42754(void)
{
    uint32_t saved;                             /* [ebp-0x10] */
    int r;                                      /* [ebp-0x14] */
    int style;                                  /* ebx at 0x4280F */

    f3C940(D32(0xF8E78u));                      /* 0x4276C disable the current dialog, saving its enabled bytes */
    f3CD10();                                   /* 0x42771 screen -> background buffer [0xF52CC] */
    setD8(0xF52D0u, 0);                         /* 0x4277F xor ah, ah; mov [0xF52D0], ah */
    fg_vbopen(S32(0xF52C8u));                   /* 0x42785 */
    f3C9D0(0xBE, 0x50, 0x104, 0x8C, 3);         /* 0x427A3 the menu's frame, filled and raised */
    fg_vbclose();                               /* 0x427A8 */
    if (D8(0xE1624u) & 0x80) {                  /* 0x427AD test byte [0xE1624], 0x80: play-by-email */
        f3BB48(0xC1A5Cu, 0x2A, 0x14A);          /* 0x427C5 CANCEL to x 330 */
        f3BFBC(0xC1A5Cu, 0x107, 1);             /* 0x427D9 SHOW PBEM enabled */
        style = 1;                              /* 0x427DE */
    } else {
        f3BB48(0xC1A5Cu, 0x2A, 0x109);          /* 0x427F4 CANCEL to x 265 */
        f3BFBC(0xC1A5Cu, 0x107, 0);             /* 0x42805 SHOW PBEM disabled */
        style = 2;                              /* 0x4280A no frame, no label */
    }
    f3C0D8(0xC1A5Cu, 0x107, style);             /* 0x42819 */
    if (!(D8(0xE1624u) & 0x80) && f3BF30(0xC1A5Cu, 0x4F) != 0) {   /* 0x4281E, 0x42831 */
        f3BFBC(0xC1A5Cu, 0x4F, 1);              /* 0x42849 SAVE GAME */
        f3BFBC(0xC1A5Cu, 0x50, 1);              /* 0x4287C RESTORE GAME */
    } else {
        f3BFBC(0xC1A5Cu, 0x4F, 0);              /* 0x4286B */
        f3BFBC(0xC1A5Cu, 0x50, 0);              /* 0x4287C */
    }
    saved = D32(0xF8E78u);                      /* 0x42881 */
    f3B05C(0xC1A5Cu);                           /* 0x4288E draw the buttons into the back buffer */
    setD32(0xF8E78u, saved);                    /* 0x42896 */
    f78884();                                   /* 0x4289B */
    fg_vbpaste(0xBE, 0x1C1, 0x50, 0xDB, 0xBE, 0xDB);   /* 0x428BB */
    f788AC();                                   /* 0x428C3 */
    do {
        r = f78BA4(0xC1A5Cu, 0);                /* 0x428CF */
    } while (r == -100 || r != -1);             /* 0x428DA cmp -0x64; 0x428E2 cmp -1 */
    setD32(0xF8E78u, saved);                    /* 0x428EA */
    f3CC68();                                   /* 0x428EF background buffer -> screen */
    f3C8C4(D32(0xF8E78u));                      /* 0x428F9 the dialog's enabled bytes back, redrawn on the screen */
    f3CCBC();                                   /* 0x428FE screen -> back buffer */
}

/* ---- 0x4290C: HELP -------------------------------------------------------------------------------------------- */

/* Port: the obvious errors in the help files, fixed by default (Erik's ruling of 19 Nov 2026: the help texts stay
   as written, only obvious errors are corrected; --original shows the files unchanged; docs/TEXT_SWEEP.md). Each
   phrase occurs once in its file (checked on ACIV.CT3); the replacement may be up to HELP_FIX_ROOM bytes longer. */
enum { HELP_FIX_ROOM = 8 };
static const struct { const char *file, *from, *to; } HELP_FIXES[] = {
    { "buyciv.hlp",   "Tool Acquistion Color", "Tool Acquisition Color" },
    { "tradwant.hlp", "Grain for Salt?\".",     "Grain for Salt?\"" },
    { "tradoffr.hlp", "OFFER button , only",   "OFFER button, only" },
    { "civilwar.hlp", "replaced by a F (",     "replaced by an F (" },
    { "superstn.hlp", "limits the affect to",  "limits the effect to" },
};

void help_fix(const char *name, char *text)
{
    size_t i;

    for (i = 0; i < sizeof HELP_FIXES / sizeof HELP_FIXES[0]; i++) {
        const char *f = HELP_FIXES[i].file, *n = name;
        char *at;
        size_t lf, lt;

        while (*f != 0 && (*n | 0x20) == *f)  /* the catalogue lookup ignores case */
            f++, n++;
        if (*f != 0 || *n != 0)
            continue;
        at = strstr(text, HELP_FIXES[i].from);
        if (at == NULL)
            continue;
        lf = strlen(HELP_FIXES[i].from);
        lt = strlen(HELP_FIXES[i].to);
        memmove(at + lt, at + lf, strlen(at + lf) + 1);
        memcpy(at, HELP_FIXES[i].to, lt);
    }
}

void f4290C(void)
{
    char name[0x24] = "";                       /* [ebp-0x54] (port guard: see the file comment) */
    char title[0x28] = "";                      /* [ebp-0x7C] */
    char msg[0x64];                             /* [ebp-0xE0] */
    char *text = (char *)"";                    /* [ebp-0x1C] (port guard) */
    long size;                                  /* [ebp-0x30] */
    int32_t maxw;                               /* [ebp-0x2C] */
    int32_t maxw2;                              /* [ebp-0x3C] */
    int w, h, x, y, bx, tw, r;                  /* [ebp-0x34], [ebp-0x38], [ebp-0x18], [ebp-0x40], [ebp-0x44], [ebp-0x24], [ebp-0x20] */
    uint32_t saved;                             /* [ebp-0x28] */

    f42CEC(name, title);                        /* 0x4292A */
    size = cat_size(name, S32(0xF40DCu));       /* 0x4293A ACIV.CT3 */
    if (size == -2) {                           /* 0x42945 */
        size = 0;                               /* 0x4294D */
        snprintf(msg, sizeof msg, DSTR(0xB405Bu), name);   /* 0x42960 "Help file '%s' not found." */
        text = msg;                             /* 0x4296E */
    }
    if (size > 0) {                             /* 0x42971 cmp [ebp-0x30], 0; jle */
        text = malloc((size_t)size + 1 + HELP_FIX_ROOM);   /* 0x4297B (port: room for the text fixes) */
        cat_load(name, (uint8_t *)text, S32(0xF40DCu));    /* 0x42992 */
        text[size] = 0;                         /* 0x429A0 */
        if (!g_original)
            help_fix(name, text);               /* port: Erik's text ruling of 19 Nov */
    }

    w = 0xFF;                                   /* 0x429A3 */
    do {
        w += 0x1E;                              /* 0x429AD */
        h = f3A42C(0, 0, w - 0x14, 0, text, &maxw, 0);     /* 0x429C9 measure only */
        if (h != -1)                            /* 0x429D4 */
            h += 0x41;                          /* 0x429D9 */
        if (w > 0x249)                          /* 0x429DD */
            break;
    } while (h == -1 || 2 * w / 3 < h || h > 0x1CC);       /* 0x429E9, 0x429FF (signed idiv 3), 0x42A04 */
    if (h > 0x1CC || h == -1)                   /* 0x42A0D, 0x42A16 */
        h = 0x1CC;                              /* 0x42A1C */
    w = maxw + 0x14;                            /* 0x42A23 */
    x = (0x280 - w) / 2;                        /* 0x42A2C (cdq-style rounding toward 0) */
    y = (0x1E0 - h) / 2;                        /* 0x42A42 */
    bx = x + (w - 0x6E) / 2;                    /* 0x42A58 CONTINUE centred */

    junk_save(x, x + w - 1, y, y + h - 1);      /* 0x42A8C fg_makegif(x, x + w - 1, y, y + h - 1, "junk") (port: memory) */
    saved = D32(0xF8E78u);                      /* 0x42A91 */
    fg_vbopen(S32(0xF52C8u));                   /* 0x42AA2 */
    f3C9D0(x, y, w, h, 3);                      /* 0x42AB8 */
    fg_setcolor(5);                             /* 0x42ABF */
    f3A0B0(x, y + 0xA, w, title);               /* 0x42AD9 the title centred */
    tw = fgf_width(title, (int)strlen(title));  /* 0x42AED */
    fg_move((w - tw) / 2 + x - 2, y + 0x14);    /* 0x42B15 */
    fg_draw((w - tw) / 2 + x + 1 + tw, y + 0x14);          /* 0x42B3B the underline */
    f3A42C(x + 0xA, y + 0x1E, w - 0x14, 0, text, &maxw2, 1);   /* 0x42B61 the text */
    fg_vbclose();                               /* 0x42B66 */
    f3BB48(0xBBBECu, 0x2A, bx);                 /* 0x42B78 */
    f3BC64(0xBBBECu, 0x2A, y + h - 0x1D);       /* 0x42B92 */
    f3C284(0xBBBECu, 0x2A, 0xB407Au);           /* 0x42BA6 "CONTINUE" */
    f3B05C(0xBBBECu);                           /* 0x42BB0 */
    f78884();                                   /* 0x42BB5 */
    fg_vbpaste(x, x + w - 1, y, y + h - 1, x, y + h - 1);  /* 0x42BDE */
    f788AC();                                   /* 0x42BE6 */
    do {
        r = f78BA4(0xBBBECu, 0);                /* 0x42BF2 */
    } while (r != -1);                          /* 0x42BFD */
    f78884();                                   /* 0x42C02 */
    fg_vbopen(S32(0xF52C8u));                   /* 0x42C0E */
    fg_move(x, y);                              /* 0x42C1E */
    junk_show(x, x + w - 1, y, y + h - 1);      /* 0x42C2D fg_showgif("junk", 3) (port: memory) */
    f3CC14();                                   /* 0x42C35 back buffer -> screen */
    f788AC();                                   /* 0x42C3A */
    setD32(0xF8E78u, saved);                    /* 0x42C42 */
    if (size > 0)                               /* 0x42C47 */
        free(text);                             /* 0x42C50 */
}

/* ---- 0x42CEC: the help file and title for the phase -------------------------------------------------------------- */

static void pick(char *name, char *title, uint32_t n, uint32_t t)
{
    strcpy(name, DSTR(n));
    strcpy(title, DSTR(t));
}

void f42CEC(char *name, char *title)
{
    switch (D8(0xF4102u)) {                     /* 0x42D04 cmp al, 0x15; ja 0x436F3; jmp cs:[4*phase + 0x42C94] */
    case 0:                                     /* -> 0x42D1C TAXATION */
        if (S32(0xFFD40u) == 2)                 /* 0x42D22 */
            pick(name, title, 0xB4088u, 0xB4095u);      /* "taxrevlt.hlp", "Tax Revolt" */
        else if (S32(0xFFD40u) == -0x32)        /* 0x42D53 */
            pick(name, title, 0xB40A0u, 0xB40ACu);      /* "taxrate.hlp", "Tax Rate Changes" */
        return;
    case 1:                                     /* -> 0x42DA7 */
        pick(name, title, 0xB40BDu, 0xB40CAu);  /* "popexpan.hlp", "Manual Population Expansion" */
        return;
    case 3:                                     /* -> 0x42DF2 */
        pick(name, title, 0xB40E6u, 0xB40F3u);  /* "shipmain.hlp", "Ship Maintenance" */
        return;
    case 4:                                     /* -> 0x42E3D */
        pick(name, title, 0xB4104u, 0xB4111u);  /* "shipcnst.hlp", "Ship Construction" */
        return;
    case 5:                                     /* -> 0x42E88 */
        if (S8(0x108804u) == 1)                 /* 0x42E88 mov edi, [0x108801]; sar edi, 0x18; cmp edi, 1 */
            pick(name, title, 0xB4123u, 0xB4130u);      /* "movement.hlp", "Token Movement" */
        else
            pick(name, title, 0xB413Fu, 0xB414Cu);      /* 0x42EC2 "moveship.hlp", "Ship Movement" */
        return;
    case 6:                                     /* -> 0x42F0D */
        pick(name, title, 0xB415Au, 0xB4167u);  /* "viewmove.hlp", "Movement Playback" */
        return;
    case 7:                                     /* -> 0x42F58 */
        pick(name, title, 0xB4179u, 0xB4186u);  /* "conflict.hlp", "Conflict" */
        return;
    case 8:                                     /* -> 0x42FA3 */
        pick(name, title, 0xB418Fu, 0xB419Cu);  /* "citycnst.hlp", "City Construction" */
        return;
    case 10:                                    /* -> 0x42FEE (CITY SUPPORT) */
    case 17:                                    /* -> 0x42FEE (CITY SUPPORT) */
        if (g_original) {
            /* BUG(1995): the City Support phases show the Ship Construction help; CITYSUP.HLP is never shown */
            pick(name, title, 0xB41AEu, 0xB41BBu);      /* "shipcnst.hlp", "Ship Construction" */
        } else {
            strcpy(name, "citysup.hlp");        /* port text: the catalogue's CITYSUP.HLP */
            strcpy(title, "City Support");      /* port text, after the phase name CITY SUPPORT (0xCA9F4[10]) */
        }
        return;
    case 11:                                    /* -> 0x43039 */
        pick(name, title, 0xB41CDu, 0xB41DAu);  /* "acqrgood.hlp", "Acquire Trade Goods" */
        return;
    case 12:                                    /* -> 0x43084 */
        pick(name, title, 0xB41EEu, 0xB41FBu);  /* "tradwant.hlp", "Declare Initial Trade Offers" */
        return;
    case 13:                                    /* -> 0x430CF */
        if (D8(0xEE168u) == 0x0C)               /* 0x430CF cmp byte [0xEE168], 0xC */
            pick(name, title, 0xB4218u, 0xB4225u);      /* "tradoffr.hlp", "Make Counter Offers" */
        else
            pick(name, title, 0xB4239u, 0xB4246u);      /* 0x43104 "tradresp.hlp", "Respond To Offer" */
        return;
    case 14:                                    /* -> 0x4314F */
        pick(name, title, 0xB4257u, 0xB4264u);  /* "tradacpt.hlp", "Accept Trades" */
        return;
    case 15:                                    /* -> 0x4319A */
        switch (D32(0xF783Cu) - 0x12u) {        /* 0x431A0 sub edi, 0x12; cmp edi, 0xC; ja 0x436F3 (unsigned) */
        case 0:  pick(name, title, 0xB4272u, 0xB427Fu); return;  /* 0x431B4 "earthquk.hlp", "Earthquake" */
        case 1:  pick(name, title, 0xB428Au, 0xB4296u); return;  /* 0x431FF "volcano.hlp", "Volcano" */
        case 2:  pick(name, title, 0xB429Eu, 0xB42ABu); return;  /* 0x4324A "treachry.hlp", "Treachery" */
        case 3:  pick(name, title, 0xB42B5u, 0xB42C0u); return;  /* 0x43295 "famine.hlp", "Famine" */
        case 4:  pick(name, title, 0xB42C7u, 0xB42D4u); return;  /* 0x432E0 "superstn.hlp", "Superstition" */
        case 5:  pick(name, title, 0xB42E1u, 0xB42EEu); return;  /* 0x4332B "civilwar.hlp", "Civil War" */
        case 6:  pick(name, title, 0xB42F8u, 0xB4305u); return;  /* 0x43376 "slavervt.hlp", "Slave Revolt" */
        case 7:  pick(name, title, 0xB4312u, 0xB431Cu); return;  /* 0x433C1 "flood.hlp", "Flood" */
        case 8:  pick(name, title, 0xB4322u, 0xB432Fu); return;  /* 0x4340C "barbaran.hlp", "Barbarian Hordes" */
        case 9:  pick(name, title, 0xB4340u, 0xB434Du); return;  /* 0x43457 "epidemic.hlp", "Epidemic" */
        case 10: pick(name, title, 0xB4356u, 0xB4363u); return;  /* 0x434A2 "civildis.hlp", "Civil Disorder" */
        case 11: pick(name, title, 0xB4372u, 0xB437Fu); return;  /* 0x434ED "iconclsm.hlp", "Iconoclasm & Heresy" */
        case 12: pick(name, title, 0xB4393u, 0xB439Eu); return;  /* 0x43538 "piracy.hlp", "Piracy" */
        default: return;                        /* 0x436F3 */
        }
    case 16:                                    /* -> 0x43583 */
        pick(name, title, 0xB43A5u, 0xB43B2u);  /* "monocnvt.hlp", "Monotheism Conversion" */
        return;
    case 18:                                    /* -> 0x435CE */
        pick(name, title, 0xB43C8u, 0xB43D3u);  /* "buyciv.hlp", "Acquire Civilization Tools" */
        return;
    case 19:                                    /* -> 0x43619 */
        pick(name, title, 0xB43EEu, 0xB43F9u);  /* "excess.hlp", "Remove Excess Goods" */
        return;
    case 20:                                    /* -> 0x43664 */
        pick(name, title, 0xB440Du, 0xB4419u);  /* "showast.hlp", "AST Chart" */
        return;
    case 21:                                    /* -> 0x436AF */
        pick(name, title, 0xB4423u, 0xB442Du);  /* "start.hlp", "Select Starting Territory" */
        return;
    default:                                    /* 2, 9 (-> 0x436F3) and > 21: nothing */
        return;
    }
}

/* ---- 0x436FC: TOGGLES ----------------------------------------------------------------------------------------- */

/* one row's flip: the bit of the aciv.cfg byte at a, its box at (0xDC, y) on the screen (mouse hidden by the caller) */
static void flip(uint32_t a, uint8_t bit, int y)
{
    uint8_t v = D8(a);

    if (v & bit) {
        setD8(a, (uint8_t)(v & ~bit));
        f3AF8C(0xF51E4u, 0xDC, y);              /* clear box */
    } else {
        setD8(a, (uint8_t)(v | bit));
        f3AF8C(0xF51DCu, 0xDC, y);              /* ticked box */
    }
}

void f436FC(void)
{
    int changed = 0;                            /* [ebp-0x20] (0x43713) */
    uint32_t saved;                             /* [ebp-0x1C] */
    int r;                                      /* [ebp-0x24] */
    FILE *fp;                                   /* [ebp-0x18] */

    fg_vbopen(S32(0xF52C8u));                   /* 0x4371D */
    f3C9D0(0xD2, 0x50, 0xDC, 0xF0, 3);          /* 0x4373B */
    fg_setcolor(5);                             /* 0x43742 */
    f3A044(0xEB, 0x5A, DSTR(0xB4447u));         /* 0x43759 "Restart Confirmations" */
    f3A044(0xEB, 0x6E, DSTR(0xB445Du));         /* 0x4376D "Movement Finished Confirmation" */
    f3A044(0xEB, 0x82, DSTR(0xB447Cu));         /* 0x43781 "Show AI Movement" */
    f3A044(0xEB, 0x96, DSTR(0xB448Du));         /* 0x43795 "Speedup AI Movement" */
    f3A044(0xEB, 0xAA, DSTR(0xB44A1u));         /* 0x437A9 "Ship Maintenance AI Pause" */
    f3A044(0xEB, 0xBE, DSTR(0xB44BBu));         /* 0x437BD "Ship Construction AI Pause" */
    f3A044(0xEB, 0xD2, DSTR(0xB44D6u));         /* 0x437D1 "City Construction AI Pause" */
    f3A044(0xEB, 0xE6, DSTR(0xB44F1u));         /* 0x437E5 "Calamity AI Pause" */
    f3A044(0xEB, 0xFA, DSTR(0xB4503u));         /* 0x437F9 "City Support AI Pause" */
    f3A044(0xEB, 0x10E, DSTR(0xB4519u));        /* 0x4380D "Alternate Tool Acquisition Colors" */
    f3AF8C((D8(0xF097Cu) & 0x08) ? 0xF51DCu : 0xF51E4u, 0xDC, 0x5B);  /* 0x43812 .. 0x4383B */
    f3AF8C((D8(0xF097Cu) & 0x10) ? 0xF51DCu : 0xF51E4u, 0xDC, 0x6F);  /* 0x43840 .. 0x43869 */
    f3AF8C((D8(0xF097Cu) & 0x40) ? 0xF51DCu : 0xF51E4u, 0xDC, 0x83);  /* 0x4386E .. 0x43897 */
    f3AF8C((D8(0xF097Du) & 0x01) ? 0xF51DCu : 0xF51E4u, 0xDC, 0x97);  /* 0x4389C .. 0x438C5 */
    f3AF8C((D8(0xF097Fu) & 0x40) ? 0xF51DCu : 0xF51E4u, 0xDC, 0xAB);  /* 0x438CA .. 0x438F3 */
    f3AF8C((D8(0xF097Fu) & 0x80) ? 0xF51DCu : 0xF51E4u, 0xDC, 0xBF);  /* 0x438F8 .. 0x43921 */
    f3AF8C((D8(0xF0980u) & 0x01) ? 0xF51DCu : 0xF51E4u, 0xDC, 0xD3);  /* 0x43926 .. 0x4394F */
    f3AF8C((D8(0xF0980u) & 0x04) ? 0xF51DCu : 0xF51E4u, 0xDC, 0xE7);  /* 0x43954 .. 0x4397D */
    f3AF8C((D8(0xF0980u) & 0x10) ? 0xF51DCu : 0xF51E4u, 0xDC, 0xFB);  /* 0x43982 .. 0x439AB */
    f3AF8C((D8(0xF0981u) & 0x01) ? 0xF51DCu : 0xF51E4u, 0xDC, 0x10F); /* 0x439B0 .. 0x439D9 */
    fg_vbclose();                               /* 0x439DE */
    saved = D32(0xF8E78u);                      /* 0x439E3 */
    f3B05C(0xC1E48u);                           /* 0x439F0 */
    setD32(0xF8E78u, saved);                    /* 0x439F8 */
    f78884();                                   /* 0x439FD */
    fg_vbpaste(0xD2, 0x1AD, 0x50, 0x13F, 0xD2, 0x13F);     /* 0x43A1D */
    f788AC();                                   /* 0x43A25 */
    for (;;) {
        r = f78BA4(0xC1E48u, 0);                /* 0x43A2A .. 0x43A31 */
        if (r == -100)                          /* 0x43A3C */
            continue;
        if (r == -1)                            /* 0x43A44 je 0x43E3E */
            break;
        /* each row: changed = 1 (0x43A55 ..), f78884 (0x43A5C ..), the bit flipped and its box drawn, then 0x43A25 */
        if (r == -0x63) {                       /* 0x43A50 */
            changed = 1;
            f78884();
            flip(0xF097Cu, 0x08, 0x5B);         /* 0x43A61 Restart Confirmations */
        } else if (r == -0x7F) {                /* 0x43AA5 */
            changed = 1;
            f78884();
            flip(0xF097Cu, 0x10, 0x6F);         /* 0x43AB6 Movement Finished Confirmation */
        } else if (r == -0x80) {                /* 0x43B0A */
            changed = 1;
            f78884();
            flip(0xF097Cu, 0x40, 0x83);         /* 0x43B1B Show AI Movement */
        } else if (r == -0x87) {                /* 0x43B64 */
            changed = 1;
            f78884();
            flip(0xF097Du, 0x01, 0x97);         /* 0x43B78 word [0xF097C] shl 7, shr ax 15 = bit 0 of 0xF097D */
        } else if (r == -0x88) {                /* 0x43BD2 */
            changed = 1;
            f78884();
            flip(0xF097Fu, 0x40, 0xAB);         /* 0x43BE6 Ship Maintenance AI Pause */
        } else if (r == -0x89) {                /* 0x43C3A */
            changed = 1;
            f78884();
            flip(0xF097Fu, 0x80, 0xBF);         /* 0x43C4E Ship Construction AI Pause */
        } else if (r == -0x8A) {                /* 0x43C97 */
            changed = 1;
            f78884();
            flip(0xF0980u, 0x01, 0xD3);         /* 0x43CAB City Construction AI Pause */
        } else if (r == -0x8B) {                /* 0x43D01 */
            changed = 1;
            f78884();
            flip(0xF0980u, 0x04, 0xE7);         /* 0x43D15 Calamity AI Pause */
        } else if (r == -0x8C) {                /* 0x43D69 */
            changed = 1;
            f78884();
            flip(0xF0980u, 0x10, 0xFB);         /* 0x43D7D City Support AI Pause */
        } else if (r == -0xAC) {                /* 0x43DD3 */
            changed = 1;
            f78884();
            flip(0xF0981u, 0x01, 0x10F);        /* 0x43DEB Alternate Tool Acquisition Colors */
        } else {
            continue;                           /* 0x43DD9 jne 0x43A2A: any other code, without f788AC */
        }
        f788AC();                               /* 0x43A25 */
    }
    if (changed == 1) {                         /* 0x43E3E */
        fp = fopen(DSTR(0xB453Eu), DSTR(0xB453Bu));        /* 0x43E4E fopen("aciv.cfg", "wb") */
        if (fp != NULL) {                       /* 0x43E59 */
            fwrite(DP(0xF097Cu), 6, 1, fp);     /* 0x43E6F */
            fclose(fp);                         /* 0x43E77 */
        }
    }
}

/* ---- the dispatcher 0x3FB94's cases (entry id - 3 of the table 0x3F780) ------------------------------------------- */

int dcase_5(int code)                           /* -> 0x3FC0D */
{
    (void)code;
    code = -100;                                /* 0x3FC0D mov ebx, 0xFFFFFF9C (kept by 0x42754's push/pop ebx) */
    f42754();                                   /* 0x3FC12 the options menu */
    return code;                                /* 0x3FC17 mov [ebp-0x3C], ebx; 0x3FC1A mov eax, [ebp-0x3C] */
}

int dcase_52(int code)                          /* -> 0x3FD26 */
{
    f3CC68();                                   /* 0x3FD26 the background (the screen before the menu) */
    f3CCBC();                                   /* 0x3FD2B screen -> back buffer */
    f436FC();                                   /* 0x3FD30 TOGGLES */
    code = -1;                                  /* 0x3FD35 mov [ebp-0x3C], 0xFFFFFFFF */
    return code;
}

int dcase_53(int code)                          /* -> 0x3FD6D */
{
    f3CC68();                                   /* 0x3FD6D */
    f3CCBC();                                   /* 0x3FD72 */
    code = -1;                                  /* 0x3FD77 mov ebx, 0xFFFFFFFF (kept by 0x4290C's push/pop ebx) */
    f4290C();                                   /* 0x3FD7C HELP */
    return code;                                /* 0x3FD81 mov [ebp-0x3C], ebx */
}

int dcase_57(int code)                          /* -> 0x3FD91 */
{
    (void)code;
    setD8(0xF5500u, 0);                         /* 0x3FD91 xor dh, dh; 0x3FD93 mov [0xF5500], dh: no quit message */
    f44CAC();                                   /* 0x3FD99 quit to DOS; does not return */
}

int dcase_41(int code) { (void)code; return -0x63; }   /* 0x41997 mov [ebp-0x3C], 0xFFFFFF9D */
int dcase_6D(int code) { (void)code; return -0x7F; }   /* 0x41A23 mov [ebp-0x3C], 0xFFFFFF81 */
int dcase_6E(int code) { (void)code; return -0x80; }   /* 0x41A37 mov [ebp-0x3C], 0xFFFFFF80 */
int dcase_E3(int code) { (void)code; return -0x87; }   /* 0x4248A mov [ebp-0x3C], 0xFFFFFF79 */
int dcase_E4(int code) { (void)code; return -0x88; }   /* 0x4249E mov [ebp-0x3C], 0xFFFFFF78 */
int dcase_E5(int code) { (void)code; return -0x89; }   /* 0x424B2 mov [ebp-0x3C], 0xFFFFFF77 */
int dcase_E6(int code) { (void)code; return -0x8A; }   /* 0x424C6 mov [ebp-0x3C], 0xFFFFFF76 */
int dcase_E7(int code) { (void)code; return -0x8B; }   /* 0x424DA mov [ebp-0x3C], 0xFFFFFF75 */
int dcase_E8(int code) { (void)code; return -0x8C; }   /* 0x424EE mov [ebp-0x3C], 0xFFFFFF74 */
int dcase_E9(int code) { (void)code; return -0xAC; }   /* 0x42502 mov [ebp-0x3C], 0xFFFFFF54 */
