/* The title menu of AC_MAIN.EXE (0x4F184, 0x4FA54) and the part of the start-up (main, 0x10010..0x111A2) the menu
   needs. Literal translations; the address of each instruction a line comes from is in its comment.
   D = read in the code, G = inference. Prototypes and the shared layouts: ui.h.

   Play-by-email is dropped (Erik's ruling, OVERVIEW §7.5): the menu's PBEM file checks and buttons
   (0x4F23E..0x4F5D6) and its PBEM start (0x4F84B..0x4FA41) are left out, see title_menu. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui.h"
#include "fg.h"

/* Defined in ui_screen.c, not yet in ui.h: the port's home of the heap pointer [0xF522C] (fg_imagebuf's buffer) and
   0x78508 (empty the keyboard buffer). */
extern uint8_t *ui_heap_F522C;
void f78508(void);

/* Heap blocks the original keeps in 32-bit data fields (G: the port's convention until ui.h gets one; the data fields
   themselves stay 0): [0xF52BC] Page1's memory, [0xF52B8] the map buffer's, [0xF52C0] Page2's, and the FGF data of the
   three fonts [0xF8B04 + 4*i] (fgf_define keeps pointers into it, so it lives as long as the program). */
static uint8_t *s_heap_F52BC, *s_heap_F52B8, *s_heap_F52C0;
static uint8_t *s_heap_F8B04[3];

/* 0x4FA54: title button id -> menu code (16-bit unsigned compares, D) */
int f4FA54(int id)
{
    int32_t code = -100;                              /* 0x4FA67 [ebp-8] = 0xFFFFFF9C */
    uint16_t ax = (uint16_t)id;                       /* 0x4FA71 cmp ax,... (jb/jbe: unsigned) */

    if (ax == 0xAC)                                   /* 0x4FA77 jbe 0x4FA89 */
        code = -0x45;                                 /* START A NEW GAME */
    else if (ax > 0xAC) {
        if (ax == 0xAD)                               /* 0x4FA7D jb 0x4FA9B (0xAC < ax < 0xAE) */
            code = -0x47;                             /* RESUME PBEM (button dropped in the port) */
        else if (ax == 0xAE)                          /* 0x4FA7F jbe 0x4FAA4 */
            code = -0x5C;                             /* START NEW PBEM (dropped) */
        else if (ax == 0xAF)                          /* 0x4FA85 */
            code = -0x5D;                             /* HOST PBEM (dropped) */
    } else {                                          /* 0x4FAC5 */
        if (ax == 0x50)                               /* 0x4FACB jbe 0x4FA92 */
            code = -0x46;                             /* RESTORE A SAVED GAME */
        else if (ax == 0x57) {                        /* 0x4FAD1 je 0x4FAB6: QUIT TO DOS */
            setD8(0xF5500u, 0);                       /* 0x4FAB8: nothing to print */
            f44CAC();                                 /* 0x4FABE */
        }
    }
    return code;                                      /* 0x4FAD3 */
}

/* 0x4F184 as the port calls it: the title menu; returns the code that ended it (port design, G):
     -0x45  START A NEW GAME: flags 0xE1624 = (flags & 0x7B) | 0x04 (0x4F7C6, D)
     -0x46  RESTORE A SAVED GAME: flags &= 0x7B (0x4F7ED, D). The menu itself does no more: the original's caller then
            sees bit 0x04 clear and runs the restore (main 0x11557: chariot.gif, the restore dialog 0x5FF28, which
            comes back to the menu at 0x11168 when it returns -0xF). Not translated yet: the port's caller gets -0x46.
     -0x47, -0x5C, -0x5D  the PBEM codes: dropped (they need the PBEM buttons or the PBEM preset of [0xF408A], which
            the port never makes); returned unchanged, with the flags untouched, should one ever arrive.
   QUIT TO DOS ends the program in f4FA54 (f44CAC). */
int title_menu(void)
{
    char name[0x14];                                  /* [ebp-0x4C] */
    char vers[0x64];                                  /* [ebp-0x110] */
    int32_t mode = 1;                                 /* [ebp-0x34] = 1 (0x4F1A1) */
    int32_t code;                                     /* [ebp-0x24] */
    int32_t i;                                        /* [ebp-0x38] */

    /* 0x4F19E [ebp-0x30] = 0: read only by the PBEM *.hst check (0x4F4F0), dropped. */
    if (S16(0xF408Au) != 0)                           /* 0x4F1A8 dword [0xF4088] sar 16 */
        mode = S16(0xF408Au);                         /* 0x4F1BC: a preset outcome skips the menu (PBEM only, G) */

    /* 0x4F1BF (the PBEM host path 0x4F834 comes back here; dropped): the working copy of the title dialog */
    memcpy(DP(UI_DIALOG_TITLE), DP(UI_DIALOG_TITLE_TEMPLATE), 1002);   /* 0x4F1D3 rep movsd 0xFA dwords; movsw */
    f3C3D0(UI_DIALOG_TITLE, 0x50);                    /* 0x4F1DE: RESTORE A SAVED GAME disabled ... */
    for (i = 0; i < 0xC; i++) {                       /* 0x4F1EE */
        FILE *f;
        snprintf(name, sizeof name, DSTR(0xB48B3u), (unsigned)i);   /* 0x4F201 sprintf "savgame%x.dat" (0x7DB3D) */
        f = fopen(name, DSTR(0xB48C1u));              /* 0x4F211 fopen(name, "rb") (0x7D8C8) */
        if (f != NULL) {                              /* 0x4F21C */
            fclose(f);                                /* 0x4F223 (0x7D95E) */
            f3C314(UI_DIALOG_TITLE, 0x50);            /* 0x4F232: ... unless a save game savgame0..b.dat exists */
            break;
        }
    }

    /* 0x4F237..0x4F5D6, PBEM, dropped: [ebp-0x20] = 0xDC (the y of the PBEM buttons); if word [0xF40C0] == 0, a
       *.1st file (0x4F24C..0x4F395) appends 0xC3868 and "START NEW %s PBEM GAME" (id 0xAE), a *.pl? file
       (0x4F399..0x4F4E6) appends 0xC347C and "RESUME %s PBEM GAME" (id 0xAD), none clears 0xEE220 (0x4F4E8); a
       *.hst file (0x4F4F0..0x4F5D6) appends 0xC3C54 (id 0xAF). Each also loads the file's header (0x5F05C) and
       quits on a corrupted or invalid one. */

    if (mode == 1) {                                  /* 0x4F5DA */
        f3CD6C(7);                                    /* 0x4F5E9: partha.gif into [0xF52CC] */
        fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(0xF52CCu), S32(0xF52C8u));   /* 0x4F611 -> the back buffer */
        fg_vbopen(S32(0xF52C8u));                     /* 0x4F620 */
        f3A18C(0, 0x32, 0x280, 0x45, DSTR(0xB49A9u)); /* 0x4F63E "A D V A N C E D   C I V I L I Z A T I O N ~" */
        f39F90(1);                                    /* 0x4F648 */
        fg_setcolor(5);                               /* 0x4F64F */
        snprintf(vers, sizeof vers, DSTR(0xB49D5u), DSTR(0xBC8E4u));   /* 0x4F66E "Version %s         ", "1.11B1" */
        f3A044(6, 0x1CD, vers);                       /* 0x4F680: shadow */
        fg_setcolor(0xF);                             /* 0x4F687 */
        f3A044(5, 0x1CC, vers);                       /* 0x4F69F */
        fg_vbclose();                                 /* 0x4F6A4 */
        f3B05C(UI_DIALOG_TITLE);                      /* 0x4F6AE: the buttons */
        f3CC14();                                     /* 0x4F6B3: on the screen */
        f3CFE4(DSTR(0xB49E9u));                       /* 0x4F6BD: fade in to pal.pal if faded out */
    }

    for (;;) {                                        /* 0x4F6C2 */
        if (mode == 1) {                              /* 0x4F6C5 */
            f775F0();                                 /* 0x4F6CE: no button down */
            while (S32(0xE1630u) == 0) {              /* 0x4F6D3: wait for a click (either button) */
                int x, y, b;
                fg_mousepos(&x, &y, &b);              /* 0x4F6EB fg_mousepos(0xE1628, 0xE162C, 0xE1630) */
                setD32(0xE1628u, (uint32_t)x);
                setD32(0xE162Cu, (uint32_t)y);
                setD32(0xE1630u, (uint32_t)b);
            }
            code = -100;                              /* 0x4F6F5 */
            for (i = 0; i < S16(UI_DIALOG_TITLE + DLG_COUNT); i++) {   /* 0x4F709 dword [0x1082DE] sar 16 */
                uint32_t rec = UI_DIALOG_TITLE + DLG_REC(i);           /* 0x4F71D: 0x1082E2 + 20*i */
                int32_t x, y;
                if (S8(rec + BTN_ENABLED) != 1)       /* 0x4F736 dword [rec+0xD] sar 24 */
                    continue;
                x = S16(rec + BTN_X);                 /* 0x4F74A */
                if (x > S32(0xE1628u))                /* 0x4F74D jg */
                    continue;
                if (x + S16(rec + BTN_W) <= S32(0xE1628u))   /* 0x4F754 dword [rec+2] sar 16; 0x4F75C jle */
                    continue;
                y = S16(rec + BTN_Y);                 /* 0x4F763 dword [rec] sar 16 */
                if (y > S32(0xE162Cu))                /* 0x4F76E jg */
                    continue;
                if (y + S16(rec + BTN_H) <= S32(0xE162Cu))   /* 0x4F775 dword [rec+4] sar 16; 0x4F783 jle */
                    continue;
                f3C5D4(UI_DIALOG_TITLE, S16(rec + BTN_ID));  /* 0x4F799: the press, until the button is let go */
                code = f4FA54(S16(rec + BTN_ID));     /* 0x4F7A7 */
                break;                                /* 0x4F7AC jmp 0x4F7B1 */
            }
        } else
            code = mode;                              /* 0x4F7AE */

        if (code == -100)                             /* 0x4F7B4: no button hit */
            continue;
        if (code == -0x45) {                          /* 0x4F7C1 */
            setD8(0xE1624u, (uint8_t)(D8(0xE1624u) & 0x7B));   /* 0x4F7CC */
            setD8(0xE1624u, (uint8_t)(D8(0xE1624u) | 0x04));   /* 0x4F7D7: new game */
            return code;                              /* 0x4F7E0 */
        }
        if (code == -0x46) {                          /* 0x4F7E8 */
            setD8(0xE1624u, (uint8_t)(D8(0xE1624u) & 0x7B));   /* 0x4F7ED: neither new game nor PBEM = restore */
            return code;                              /* 0x4F7F4 */
        }
        if (code == -0x5D)                            /* 0x4F801: PBEM host (0x50EEC, 0x5195C), dropped */
            return code;
        if (code == -0x47 || code == -0x5C)           /* 0x4F83C, 0x4F841: PBEM resume / start (0x4F84B..0x4FA41), dropped */
            return code;
        /* any other code: back to 0x4F6C2 (0x4F845 jne) */
    }
}

/* 0x4F184: the title menu; its caller (0x111A2) ignores eax: the outcome is in the flags byte 0xE1624 */
void f4F184(void)
{
    (void)title_menu();
}

/* 0x39E50: load font i (0..2) from ACIV.CT1 and define it; errors go through f3A780 (message, wait for a key) */
static void f39E50(int32_t i)
{
    char msg[0x40];                                   /* [ebp-0x54] */
    const char *fname = DSTR(D32(0xC1948u + 4u * (uint32_t)i));   /* 0xC1948[i]: block05 / cognac10 / cognac19.fgf */
    long size;                                        /* [ebp-0x18] */

    size = cat_size(fname, S32(0xF40D4u));            /* 0x39E7A cat_size(name, [0xF40D4] = CT1) */
    if (size < 0) {                                   /* 0x39E85 jge */
        snprintf(msg, sizeof msg, DSTR(0xB3B28u), fname);   /* "Can't find font file: %s\n" */
        f3A780(msg);                                  /* 0x39F34, 0x39F3F */
        return;
    }
    s_heap_F8B04[i] = malloc((size_t)size);           /* 0x39EA3 (0x822B5 = malloc), 0x39EB1 [0xF8B04 + 4*i] */
    if (cat_load(fname, s_heap_F8B04[i], S32(0xF40D4u)) != 0) {   /* 0x39ECF, 0x39EE3 */
        snprintf(msg, sizeof msg, DSTR(0xB3B42u), fname);   /* "Can't load font file: %s\n" */
        f3A780(msg);
        return;
    }
    setD16(UI_FONT_NUM + 2u * (uint32_t)i, (uint16_t)fgf_define(s_heap_F8B04[i]));   /* 0x39EFC, 0x39F07 */
    if (S16(UI_FONT_NUM + 2u * (uint32_t)i) == 0) {   /* 0x39F12 dword [2*i+0xF8B2A] sar 16 */
        snprintf(msg, sizeof msg, DSTR(0xB3B5Cu), fname);   /* "Can't define font: %s\n" */
        f3A780(msg);
    }
}

/* 0x788D4: show the mouse pointer: visibility stack top = 1, fg_mousevis(1) */
static void f788D4(void)
{
    setD8(0x112A0Cu, 1);                              /* 0x788E8 */
    fg_mousevis(1);                                   /* 0x788EE */
}

/* Copy a message into [0xF5500] and quit through f44CAC, as the start-up's error exits do (0x10A50.., inline strcpy). */
static _Noreturn void boot_fail(uint32_t text)
{
    snprintf((char *)DP(0xF5500u), 0x1F4, "%s", DSTR(text));   /* 0xF5500: 500 bytes up to 0xF56F4 (G) */
    f44CAC();
}

/* Allocate a 640x480-style buffer and define it as a virtual buffer, as 0x10A3D..0x10A92 (Page1) does. */
static void boot_vbuffer(uint8_t **heap, uint32_t size, int w, int h, uint32_t handle_at, uint32_t nomem)
{
    *heap = malloc(size);                             /* 0x822B5 = malloc */
    if (*heap == NULL)
        boot_fail(nomem);                             /* "Advanced Civ. needs more Extended memory" */
    setD32(handle_at, (uint32_t)fg_vbdefine(*heap, w, h));   /* 0x82398 fg_vbdefine(buffer, width, height) */
}

/* The start-up (main 0x10010) up to the menu, as far as the menu needs it. G: the choice of steps; each step D at its
   address. Skipped: the DOS banner and the XMS check (0x1003F..0x100F3), the command line (0x100F3..0x10572; the port
   starts as with /NT, see below), the CD check (0x10572..0x1061A), "volcano.wav" (0x10752), 0x4FADC ([0xBB3EE]), the
   banner delay 0x78A20 (0x1078D), the debug log 0x78B0C/0x78B24 (no-ops), reading aciv.cfg (0x107A4..0x10826), clearing
   0xF5240/0xF5268/0xF5290 (0x10826), 0x7DD37, the DOS video mode (0x7E044 -> [0xF6058]) and the chipset (0x7E121),
   fg_testmode (0x109BC), the 1.6 MB intro trans_buffer (0x10AAB..0x10B0A, freed again at 0x10F98), the opening sprites
   (0x118F8 at 0x10B3E: maped.gif cut into sprites; the menu overwrites Page1), all of the sound (0x10B4D..0x10E7F; the
   port has its own), the intro (0x12DF4 at 0x10EA8, 0x1330C and the fade to black 0x3CEB8 at 0x1101D..0x11022), the
   setjmp at 0x11031 and the continuous-test set-up (0x1103B..0x11150).
   File names: the original opens everything relative to its working directory; title_boot puts gamedir in front of the
   catalogs and pal.pal. The menu's savgame%x.dat and the quit's aciv.cfg stay relative, as in the original (G: the port
   runs in the game folder). The caller has opened the platform layer (platform.h) before. */
int title_boot(const char *gamedir)
{
    char path[0x200];
    FILE *f;
    int r;

    r = state_init(gamedir);                          /* the data object, as DOS/4GW loaded it */
    if (r != 0)
        return r;

    /* The port has no intro yet, so it starts as the original does with /NT ("Titles will be skipped.", 0x102AC):
       the palette is set directly at 0x11177 and the menu does not fade in ([0xF8E40] stays 0). */
    setD16(0xF40BEu, 1);                              /* 0x102AC */

    /* 0x10629..0x1074F: where the animations are. getcwd (0x10633) -> gamedir. Prefixes: [0xF5758] for CT4 and the
       animations, [0xF57BC] for CT5, [0xF5820] (G: the sounds); 100 bytes each. */
    snprintf(path, sizeof path, "%s/%s", gamedir, DSTR(0xB07E0u));
    f = fopen(path, DSTR(0xB07DDu));                  /* 0x10642 fopen("anims\aciv.ct4", "rb") */
    if (f != NULL) {                                  /* 0x1064C */
        snprintf((char *)DP(0xF5758u), 0x64, DSTR(0xB07EFu), gamedir);   /* 0x1065F "%s\ANIMS\" */
        snprintf((char *)DP(0xF57BCu), 0x64, DSTR(0xB07F9u), gamedir);   /* 0x10676 "%s\" */
        snprintf((char *)DP(0xF5820u), 0x64, DSTR(0xB07FDu), gamedir);   /* 0x1068D */
        fclose(f);                                    /* 0x1069A */
    } else {
        snprintf(path, sizeof path, "%s/%s", gamedir, DSTR(0xB0804u));
        f = fopen(path, DSTR(0xB0801u));              /* 0x106AE fopen("aciv.ct4", "rb") */
        if (f != NULL) {                              /* 0x106B8 */
            snprintf((char *)DP(0xF5758u), 0x64, DSTR(0xB080Du), gamedir);   /* 0x106CB "%s\" */
            snprintf((char *)DP(0xF57BCu), 0x64, DSTR(0xB0811u), gamedir);
            snprintf((char *)DP(0xF5820u), 0x64, DSTR(0xB0815u), gamedir);
            fclose(f);                                /* 0x10706 */
        } else {                                      /* 0x1070D: the CD drive [0xF40CC] (empty: the check is skipped) */
            snprintf((char *)DP(0xF5758u), 0x64, DSTR(0xB0819u), DSTR(0xF40CCu));   /* "%s:\" */
            snprintf((char *)DP(0xF57BCu), 0x64, DSTR(0xB081Eu), DSTR(0xF40CCu));
            snprintf((char *)DP(0xF5820u), 0x64, DSTR(0xB0823u), DSTR(0xF40CCu));
        }
    }

    /* 0x10865..0x1097F: the five catalogs; a failure quits with a message */
    snprintf(path, sizeof path, "%s/%s", gamedir, DSTR(0xB0852u));
    setD32(0xF40D4u, (uint32_t)cat_open_file(path));  /* 0x1086F cat_open("aciv.ct1") -> [0xF40D4] */
    if (D32(0xF40D4u) == 0)
        boot_fail(0xB085Bu);                          /* 0x10880 "Can't open catalog file- aciv.ct1\n" */
    snprintf(path, sizeof path, "%s/%s", gamedir, DSTR(0xB087Eu));
    setD32(0xF40D8u, (uint32_t)cat_open_file(path));  /* 0x1089C "aciv.ct2" -> [0xF40D8] */
    if (D32(0xF40D8u) == 0)
        boot_fail(0xB0887u);
    snprintf(path, sizeof path, "%s/%s", gamedir, DSTR(0xB08AAu));
    setD32(0xF40DCu, (uint32_t)cat_open_file(path));  /* 0x108C9 "aciv.ct3" -> [0xF40DC] */
    if (D32(0xF40DCu) == 0)
        boot_fail(0xB08B3u);
    snprintf(path, sizeof path, DSTR(0xB08D6u), DSTR(0xF5758u));   /* 0x10902 sprintf([ebp-0x9C], "%saciv.ct4") */
    setD32(0xF40E0u, (uint32_t)cat_open_file(path));  /* 0x10911 -> [0xF40E0] */
    if (D32(0xF40E0u) == 0) {                         /* 0x1091E */
        snprintf((char *)DP(0xF5500u), 0x1F4, DSTR(0xB08E1u), path);   /* 0x10933 "Can't open catalog file- %s\n" */
        f44CAC();                                     /* 0x1093B */
    }
    snprintf((char *)DP(0xF5500u), 0x1F4, DSTR(0xB08FEu), DSTR(0xF57BCu));   /* 0x1094F sprintf(0xF5500, "%saciv.ct5") */
    setD32(0xF40E4u, (uint32_t)cat_open_file(DSTR(0xF5500u)));   /* 0x1095C -> [0xF40E4] */
    if (D32(0xF40E4u) == 0)
        boot_fail(0xB0909u);                          /* 0x1096D "Can't open catalog file- aciv.ct5\n" */

    /* 0x109C9..0x10A00: video mode 25 (640x480x256): fg_setmode (0x109F3), fg_vbinit (0x812F4) and the screen erase
       (0x81312) are fg_init in the port (G) */
    setD32(0xF40D0u, 0x19);                           /* 0x109C9 */
    fg_init();
    if (fg_mouseini() == -1)                          /* 0x10A05 fg_mouseini (0x814BD); 0x10A0A cmp eax,-1 */
        boot_fail(0xB098Du);                          /* 0x10A1D "Could not initialize your mouse." (after the mode restore 0x10A15) */
    setD8(0xF625Au, 1);                               /* 0x10A36: the mouse is there */

    boot_vbuffer(&s_heap_F52BC, 0x4B000, 0x280, 0x1E0, 0xF52C8u, 0xB09AEu);   /* 0x10A3D..0x10A92 Page1 -> [0xF52C8] */

    f39E50(0);                                        /* 0x10B11: block05.fgf */
    f39E50(1);                                        /* 0x10B1B: cognac10.fgf */
    f39E50(2);                                        /* 0x10B2A: cognac19.fgf */
    f39F90(1);                                        /* 0x10B39 */

    /* 0x10EAD..0x10F02: the map buffer, 1716x568 -> [0xF52C4]; the menu does not use it, but defining it here keeps
       the handles as in the original (Page2 = handle 2, G: fg_vbdefine takes the first free slot, 0x823A8) */
    boot_vbuffer(&s_heap_F52B8, 0xEDF60, 0x6B4, 0x238, 0xF52C4u, 0xB0B70u);
    boot_vbuffer(&s_heap_F52C0, 0x4B000, 0x280, 0x1E0, 0xF52CCu, 0xB0BB6u);   /* 0x10F1B..0x10F70 Page2 -> [0xF52CC] */

    ui_heap_F522C = malloc(0xFF00);                   /* 0x10FA4 the small trans_buffer -> [0xF522C] */
    if (ui_heap_F522C == NULL)
        boot_fail(0xB0C04u);                          /* 0x10FB7 */
    f8240F(ui_heap_F522C, 0xFF00);                    /* 0x10FEC fg_imagebuf([0xF522C], 0xFF00) */

    f78508();                                         /* 0x11027: empty the keyboard buffer */
    f788D4();                                         /* 0x1102C: the pointer on */

    if (S16(0xF40C0u) == 0)                           /* 0x11155 */
        setD16(0xF408Au, 0);                          /* 0x11161: no preset menu outcome */
    if (S16(0xF40BEu) != 0) {                         /* 0x11168: titles skipped */
        snprintf(path, sizeof path, "%s/%s", gamedir, DSTR(0xB0C87u));
        fg_gifpal(path, DP(0xF5D58u));                /* 0x1117C fg_gifpal("pal.pal", 0xF5D58) */
        fg_setdacs(0, 0x100, DP(0xF5D58u));           /* 0x11190 */
    }
    /* 0x11198: [0xF40BA] (the self-running demo /SRD) is 0, so the original calls the menu next (0x111A2). */
    return 0;
}
