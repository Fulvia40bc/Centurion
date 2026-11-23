/* The title menu of AC_MAIN.EXE (0x4F184, 0x4FA54), the part of the start-up (main, 0x10010..0x111A2) the menu
   needs, and main's new-game part around the setup screens (0x111DC..0x113FE, title_newgame). Literal translations; the address of each instruction a line comes from is in its comment.
   D = read in the code, G = inference. Prototypes and the shared layouts: ui.h.

   Play-by-email is dropped (Erik's ruling, OVERVIEW §7.5): the menu's PBEM file checks and buttons
   (0x4F23E..0x4F5D6) and its PBEM start (0x4F84B..0x4FA41) are left out, see title_menu. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui.h"
#include "fg.h"
#include "snd.h"

/* Defined in ui_screen.c, not yet in ui.h: the port's home of the heap pointer [0xF522C] (fg_imagebuf's buffer) and
   0x78508 (empty the keyboard buffer). */
extern uint8_t *ui_heap_F522C;
void f78508(void);

/* Heap blocks the original keeps in 32-bit data fields (G: the port's convention until ui.h gets one; the data fields
   themselves stay 0): [0xF52BC] Page1's memory, [0xF52B8] the map buffer's, [0xF52C0] Page2's, and the FGF data of the
   three fonts [0xF8B04 + 4*i] (fgf_define keeps pointers into it, so it lives as long as the program). */
static uint8_t *s_heap_F52BC, *s_heap_F52B8, *s_heap_F52C0;
static uint8_t *s_heap_F8B04[3];

/* The game folder title_boot was given (port: title_menu_again puts it in front of pal.pal, as title_boot did). */
static char s_gamedir[0x200];

/* Port entry point, not yet in ui.h (Claude moves it there): main 0x11168..0x111A2, see below. */

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
   starts as with /NT, see below), the CD check (0x10572..0x1061A), 0x4FADC ([0xBB3EE]), the
   banner delay 0x78A20 (0x1078D), the debug log 0x78B0C/0x78B24 (no-ops), reading aciv.cfg (0x107A4..0x10826), clearing
   0xF5240/0xF5268/0xF5290 (0x10826), 0x7DD37, the DOS video mode (0x7E044 -> [0xF6058]) and the chipset (0x7E121),
   fg_testmode (0x109BC), the 1.6 MB intro trans_buffer (0x10AAB..0x10B0A, freed again at 0x10F98), all of the sound (0x10B4D..0x10E7F; the
   port has its own), the intro (0x12DF4 at 0x10EA8, 0x1330C and the fade to black 0x3CEB8 at 0x1101D..0x11022), the
   setjmp at 0x11031 and the continuous-test set-up (0x1103B..0x11150).
   File names: the original opens everything relative to its working directory; title_boot puts gamedir in front of the
   catalogs and pal.pal. The menu's savgame%x.dat and the quit's aciv.cfg stay relative, as in the original (G: the port
   runs in the game folder). The caller has opened the platform layer (platform.h) before. */
/* 0x10BEE..0x10E7F, the data side of the sound start-up (the sound itself: snd.c / snd_ail.c, started by
   snd_startup at AIL_startup's place): MDI.INI's DRIVER picks the music set (OVERVIEW §2), whose four file names go
   to 0xCAB1C/0xCAB2C/0xCAB3C/0xCAB4C; when the driver installs, [0xF625D] = 1 (music on) and [0xF6284] = a sequence
   handle. G: the port treats a readable MDI.INI with a DRIVER line as "installed" (AIL_install_MDI_INI 0x8547D
   succeeded in DOSBox-X with SBPRO2.MDI). The AIL handles [0xF6270..0xF6284] differ from a DOSBox-X dump by design:
   the drivers 1, the samples 1, 2, 3 and the sequence 1 (ail.c's numbering; 1 each with no backend). */
/* stricmp 0x877AD: ASCII case folding of A..Z only */
static int ui_stricmp(const char *a, const char *b)
{
    for (;; a++, b++) {
        int x = (unsigned char)*a, y = (unsigned char)*b;
        if (x >= 'A' && x <= 'Z') x += 0x20;
        if (y >= 'A' && y <= 'Z') y += 0x20;
        if (x != y || y == 0)
            return x - y;
    }
}

static void boot_music_set(void);

static void boot_sound(void)
{
    if (D8(0xBC880u) == 0)                            /* 0x10B4D sound wanted (1 in the shipped data; /NS clears it) */
        return;                                       /* -> 0x10E7F */
    snd_startup();                                    /* 0x10B60 AIL_startup (snd.c: the port's sound, 22 Nov) */
    setD8(0xF625Bu, 1);                               /* 0x10B65 after AIL_startup: the sound system is up */
    {   /* 0x10B7F AIL_install_DIG_INI(&[0xF6274]): G, the port treats a readable DIG.INI with a DRIVER line as installed */
        char line[256], key[32], val[128];
        int ok = 0;
        FILE *f = fopen("DIG.INI", "r");
        if (f != NULL) {
            while (fgets(line, sizeof line, f) != NULL)
                if (sscanf(line, "%31s %127s", key, val) == 2 && strcmp(key, "DRIVER") == 0)
                    ok = 1;
            fclose(f);
        }
        if (ok) {
            setD32(0xF6274u, 1);                      /* the driver handle (an AIL pointer: differs by design) */
            setD8(0xF625Cu, 1);                       /* 0x10B99 digital sound on */
            setD32(0xF6278u, (uint32_t)snd_allocate_sample());   /* 0x10BA8, 0x10BC5, 0x10BE1: three sample handles, */
            setD32(0xF627Cu, (uint32_t)snd_allocate_sample());   /* each AIL_init_sample'd (snd_ail.c): 1, 2, 3 */
            setD32(0xF6280u, (uint32_t)snd_allocate_sample());   /* (1, 1, 1 with no backend: the unit tests) */
        }
    }
    boot_music_set();                                 /* 0x10BEE..0x10E7F */
}

static void boot_music_set(void)
{
    static const uint32_t dst[4] = { 0xCAB1Cu, 0xCAB2Cu, 0xCAB3Cu, 0xCAB4Cu };
    static const uint32_t gset[4] = { 0xB0A5Cu, 0xB0A68u, 0xB0A75u, 0xB0A82u };   /* 0x10C13: no MDI.INI */
    static const uint32_t fset[4] = { 0xB0AA6u, 0xB0AB2u, 0xB0ABFu, 0xB0ACCu };   /* 0x10CB1: adlib, sblaster */
    static const uint32_t sset[4] = { 0xB0AEFu, 0xB0AFBu, 0xB0B08u, 0xB0B15u };   /* 0x10D4F: sbpro1, sbpro2 */
    static const uint32_t g2set[4] = { 0xB0B22u, 0xB0B2Eu, 0xB0B3Bu, 0xB0B48u };  /* 0x10DC2: any other driver */
    const uint32_t *set = gset;
    char line[256], driver[128] = "";
    FILE *f = fopen(DSTR(0xB0A54u), "r");             /* 0x10C07 AIL_API_read_INI("MDI.INI") */
    int i;

    if (f != NULL) {
        while (fgets(line, sizeof line, f) != NULL) {
            char key[32];
            if (sscanf(line, "%31s %127s", key, driver) == 2 && strcmp(key, "DRIVER") == 0)
                break;
            driver[0] = 0;
        }
        fclose(f);
    }
    if (driver[0] != 0) {                             /* 0x10C11 the INI read succeeded */
        if (ui_stricmp(driver, DSTR(0xB0A8Fu)) == 0 || ui_stricmp(driver, DSTR(0xB0A99u)) == 0)
            set = fset;                               /* 0x10C94 / 0x10CA8 "adlib.mdi", "sblaster.mdi" */
        else if (ui_stricmp(driver, DSTR(0xB0AD9u)) == 0 || ui_stricmp(driver, DSTR(0xB0AE4u)) == 0)
            set = sset;                               /* 0x10D32 / 0x10D46 "sbpro1.mdi", "sbpro2.mdi" */
        else
            set = g2set;
    }
    for (i = 0; i < 4; i++)                           /* the inline strcpy loops into the four name buffers */
        strcpy((char *)DP(dst[i]), DSTR(set[i]));
    if (driver[0] != 0) {                             /* 0x10E57 AIL_install_MDI_INI(&[0xF6270]) */
        setD32(0xF6270u, 1);
        setD8(0xF625Du, 1);                           /* 0x10E6C music on */
        setD32(0xF6284u, (uint32_t)snd_allocate_sequence());   /* 0x10E7A AIL_allocate_sequence_handle */
    }
}

int title_boot(const char *gamedir)
{
    char path[0x200];
    FILE *f;
    int r;

    r = state_init(gamedir);                          /* the data object, as DOS/4GW loaded it */
    if (r != 0)
        return r;
    f60A80();                                         /* run by _cstart_ before main: Watcom's initializer table
                                                         (XI entry at 0xE15EA, priority 0x40); the table's other
                                                         eight routines are library start-ups the port replaces */
    /* 0x10035: f78B44 asks DPMI for free-memory information (int 31h AX=0500h) into 0x1125A8..0x1125D7: machine
       specific, left out (differs from a DOSBox-X dump by design). */
    f7BB14();                                         /* 0x10577: the CRC-32 table (the CD check around it is skipped) */

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
    /* 0x10752..0x1075C: sprintf(0xF549C, "volcano.wav"): the sound the AUDIO/VISUALS volume arrows play (menu2.c) */
    snprintf((char *)DP(0xF549Cu), 0x20, "%s", DSTR(0xB0828u));

    /* 0x107A4..0x10826: the settings file aciv.cfg (6 bytes) into 0xF097C, if it is there (cwd = the game folder) */
    f = fopen(DSTR(0xB0849u), "rb");                  /* 0x107A9 fopen("aciv.cfg") -> [0xF52D8] (FILE *: port-side) */
    if (f != NULL) {                                  /* 0x107B3 */
        uint16_t w7c, w80;
        size_t got = fread(DP(0xF097Cu), 6, 1, f);    /* 0x107C8 fread(0xF097C, 6, 1, fp) */
        (void)got;                                    /* the original does not check it either */
        fclose(f);                                    /* 0x107D2 */
        w80 = D16(0xF0980u);                          /* 0x107D7 */
        w7c = D16(0xF097Cu);                          /* 0x107E6 */
        /* 0x107DD..0x107E4: ax = (word << 8) >> 13 in 16 bits = low byte of 0xF0980 >> 5 */
        setD16(0xF097Cu, (uint16_t)(w7c & 0xFFDFu)); /* 0x107FB and al,0xDF; 0x10803 */
        setD16(0xF0982u, (uint16_t)((w7c >> 9) + (((w80 & 0xFFu) >> 5) << 7)));   /* 0x107F4..0x10813 */
        if (S16(0xF40AEu) == 1)                       /* 0x10809 dword [0xF40AC] sar 16 == 1 */
            setD8(0xF097Cu, (uint8_t)(D8(0xF097Cu) & 0xFAu));   /* 0x1081F */
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
    f118F8();                                         /* 0x10B3E: the sprites from maped.gif */
    boot_sound();                                     /* 0x10B4D..0x10E7F (data side only; the port plays sound itself) */

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
    snprintf(s_gamedir, sizeof s_gamedir, "%s", gamedir);   /* for title_menu_again (port) */
    title_menu_again();                               /* 0x11168 */
    return 0;
}

/* main 0x11168..0x111A2: what main does before each call of the menu 0x4F184. Start-up falls into it (title_boot),
   and the new game's CANCEL (0x113E6) and the restore dialog's cancel (0x115A3) jump back to it (D). The caller then
   calls title_menu (0x111A2).
   0x11198: if [0xF40BA] != 0 (the continuous-test player count, set at 0x11110 from [0xF40C8]; the port skips that
   set-up, so it stays 0) the original skips the menu and goes 0x111A9..0x111D7: flags 0xE1624 |= 0x04, &= 0x7F, the
   screen erased (0x81312) with the pointer hidden, fade in to pal.pal (0x3CFE4). Left out with the continuous test. */
void title_menu_again(void)
{
    char path[0x200];

    if (S16(0xF40BEu) != 0) {                         /* 0x11168: titles skipped */
        snprintf(path, sizeof path, "%s/%s", s_gamedir, DSTR(0xB0C87u));
        fg_gifpal(path, DP(0xF5D58u));                /* 0x1117C fg_gifpal("pal.pal", 0xF5D58) */
        fg_setdacs(0, 0x100, DP(0xF5D58u));           /* 0x11190 */
    }
}

/* main 0x111DC..0x113FE, the new game (bit 0x04 of 0xE1624, set by the menu at 0x4F7D7): the defaults of the setup
   data, then the loop 0x113CD of the setup screen 0x4DD50 and the variants screen 0x4EAE0. D unless marked.
   Returns 0 where main jumps back to the menu 0x11168 (the caller then runs title_menu_again and title_menu), 1 where
   it goes on to 0x113FE. Not yet ported from 0x113FE on: 0x12DA4 (clears byte [0xF40FE], word [0xF5478], dword
   [0xEDF58], and 20 bytes at 0xF6E76 and 100 bytes at 0xF6E12 through 0x7D5D0, G: memset), then bit 0x80 of 0xE1624
   = PBEM (0x11410..0x114CA: 0x128F0, 0x30D58, the "Starting this new PBEM game will remove ..." box 0x795A8, the
   "Preparing initial PBEM game files" screen), dropped: the menu clears 0x80 with 0x04 (0x4F7CC), so the port always
   takes 0x114CF ("Loading game data and maps" on background 8) and 0x115B0 into the game.
   [0xF40BA] and [0xF40C6] / [0xF40C8] are the continuous test's (set at 0x11085..0x11110, not ported): 0 in the port,
   but their branches are kept, as they are plain data. */
int title_newgame(void)
{
    uint8_t ch = D8(0xE1624u);                        /* 0x111DC mov ch, byte [0xE1624] */
    uint16_t ax;
    uint8_t al;
    int32_t r;                                        /* [ebp-0x2C] */

    if ((ch & 0x04) == 0)                             /* 0x111E2 je 0x11557: the restore path (G: never here, the */
        return 0;                                     /* port calls this only after START A NEW GAME) */

    if (S16(0xF40BAu) == 0) {                         /* 0x111EB word [0xF40BA]; 0x111F5 jne 0x1125E */
        ax = (uint16_t)(D16(0xE1624u) & 0xFFFDu);     /* 0x111FD, 0x11206 and al,-3 */
        if (S16(0xF40C6u) == 0) {                     /* 0x111F7 dword [0xF40C4] sar 16; 0x1120A */
            setD8(UI_SETUP_HUMANS, 3);                /* 0x1120C: 3 humans */
            setD16(0xE1624u, ax);                     /* 0x11215: flag 0x02 off */
            setD8(UI_SETUP_COMPUTERS, 0);             /* 0x1121B: 0 computers (dh = 0) */
        } else {                                      /* 0x11226: the continuous test's players */
            setD8(UI_SETUP_COMPUTERS, D8(0xF40C8u));  /* 0x1122C */
            setD8(UI_SETUP_HUMANS, 1);                /* 0x1123D */
            if (S16(0xF40C8u) + 1 > 3)                /* 0x11232 dword [0xF40C6] sar 16; 0x11243 inc; jle */
                setD8(0xE1624u, (uint8_t)(ch | 0x02));   /* 0x1124E */
            else
                setD16(0xE1624u, ax);                 /* 0x11256 */
        }
    } else {                                          /* 0x1125E: the continuous test, no setup screen */
        int32_t total;
        setD8(UI_SETUP_COMPUTERS, D8(0xF40BAu));      /* 0x11263 */
        setD8(UI_SETUP_HUMANS, 0);                    /* 0x11272 */
        ch &= 0xFD;                                   /* 0x11283 and ch,-3 */
        setD8(0xE1624u, ch);                          /* 0x11288 */
        total = S8(UI_SETUP_HUMANS) + S8(UI_SETUP_COMPUTERS);   /* 0x11299..0x112AC dword [0xE161D] / [0xE161E] sar 24 */
        setD16(0xE1624u, (uint16_t)(D16(0xE1624u) | (S16(0xF40BAu) != 3 ? 0x02u : 0u)));   /* 0x11278 setne, 0x112AE */
        setD32(UI_PLAYERS_A, (uint32_t)total);        /* 0x112B5 */
        setD32(UI_PLAYERS_B, (uint32_t)total);        /* 0x112BB */
    }

    /* 0x112C1: the computers' AI level, bits 10..11 of word [0xE1624] (the cleared value is also stored first at
       0x112D4 / 0x112F1; nothing reads it in between) */
    ax = (uint16_t)(D16(0xE1624u) & 0xF3FFu);         /* 0x112C1, 0x112CD and ah,-0xD */
    if (S32(0xF5884u) == 0)                           /* 0x112D0 */
        ax |= 0x0400u;                                /* 0x112DA or ah,4: 1 "Normal" */
    else                                              /* 0x112DF: ([0xF5884] - 10) & 3; only bits 10..11 reach the word */
        ax |= (uint16_t)(((D32(0xF5884u) - 0xAu) & 3u) << 10);
    setD16(0xE1624u, ax);                             /* 0x112F9 */

    al = (uint8_t)(D8(UI_SETUP_TRADE) & 0xF0);        /* 0x112FF, 0x1130A: "# of Trading Sessions" */
    if (S32(0xF5888u) == 0)                           /* 0x1130C */
        al |= 0x02;                                   /* 0x11315 */
    else
        al |= (uint8_t)(D8(0xF5888u) & 0x0F);         /* 0x11319 */
    setD8(UI_SETUP_TRADE, al);                        /* 0x11329 */
    al = (uint8_t)(D8(UI_SETUP_TRADE) & 0x0F);        /* 0x1132E, 0x11339: "Minimum Goods Before Trading Allowed" */
    if (S32(0xF588Cu) == 0)                           /* 0x1133B */
        al |= 0x60;                                   /* 0x11344 */
    else
        al |= (uint8_t)((D8(0xF588Cu) & 0x0F) << 4);  /* 0x11348..0x11359 */
    setD8(UI_SETUP_TRADE, al);                        /* 0x1135B */

    if (S32(0xF5890u) == 0)                           /* 0x11360 */
        setD8(0xE1624u, (uint8_t)(D8(0xE1624u) & 0xEF));   /* 0x11369 and -0x11 */
    else
        setD8(0xE1624u, (uint8_t)(D8(0xE1624u) | 0x10));   /* 0x11372 */
    if (S32(0xF5894u) == 0)                           /* 0x11379 */
        setD8(0xE1624u, (uint8_t)(D8(0xE1624u) & 0xDF));   /* 0x11382 and -0x21 */
    else
        setD8(0xE1624u, (uint8_t)(D8(0xE1624u) | 0x20));   /* 0x1138B */
    if (S32(0xF5898u) == 0)                           /* 0x11392 */
        setD8(0xE1624u, (uint8_t)(D8(0xE1624u) & 0xBF));   /* 0x1139B and -0x41 */
    else
        setD8(0xE1624u, (uint8_t)(D8(0xE1624u) | 0x40));   /* 0x113A4 */
    if (S32(0xF589Cu) == 0)                           /* 0x113AB */
        setD8(0xE1625u, (uint8_t)(D8(0xE1625u) & 0xDF));   /* 0x113B4 and -0x21 */
    else
        setD8(0xE1625u, (uint8_t)(D8(0xE1625u) | 0x20));   /* 0x113BD */
    setD16(0xE1624u, (uint16_t)(D16(0xE1624u) & 0xEDF7u));   /* 0x113C4: 0xE1624 0x08, 0xE1625 0x02 and 0x10 off */

    for (;;) {                                        /* 0x113CD */
        if (S16(0xF40BAu) != 0)                       /* 0x113D5 jne 0x113FE */
            break;
        r = (int16_t)f4DD50();                        /* 0x113D7; 0x113DC cwde */
        if (r == -0xF)                                /* 0x113E3: CANCEL */
            return 0;                                 /* 0x113E6 je 0x11168 */
        if (r != -0x82)                               /* 0x113EF: START GAME (-1) */
            break;                                    /* 0x113F5 jne 0x113FE */
        f4EAE0();                                     /* 0x113F7: VARIANTS */
    }                                                 /* 0x113FC jmp 0x113CD */
    return 1;                                         /* 0x113FE: on into the game (not ported yet) */
}

/* 0x118F8: draw maped.gif from CT2 into Page1 and cut the 12 sprites the UI uses out of it (f3AE90: record, x, y, w,
   h). 0xF51CC / 0xF51D4 are the variants' tick boxes (set / clear). */
void f118F8(void)
{
    static const struct { uint32_t spr; int x, y, w, h; } cuts[12] = {
        { 0xF51ACu, 0xE6, 0x36, 0xD, 0x13 },   /* 0x1194F */
        { 0xF51B4u, 0xE6, 0x1D, 0xD, 0x13 },   /* 0x1196A */
        { 0xF51D4u, 0x40, 0x4C, 0xE, 0xE },    /* 0x11985 */
        { 0xF51CCu, 0x40, 0x3A, 0xE, 0xE },    /* 0x119A0 */
        { 0xF51E4u, 0xCE, 0x2D, 0x9, 0x9 },    /* 0x119BB */
        { 0xF51DCu, 0xCE, 0x3B, 0x9, 0x9 },    /* 0x119D6 */
        { 0xF51ECu, 0x61, 0x3A, 0xB, 0xB },    /* 0x119F1 */
        { 0xF51F4u, 0x61, 0x4D, 0xB, 0xB },    /* 0x11A0C */
        { 0xF51FCu, 0x61, 0x65, 0xB, 0xB },    /* 0x11A27 */
        { 0xF5204u, 0x61, 0x78, 0xB, 0xB },    /* 0x11A42 */
        { 0xF51BCu, 0x19, 0x23, 0x5, 0xA },    /* 0x11A5D */
        { 0xF51C4u, 0x2A, 0x23, 0x5, 0xA },    /* 0x11A78 */
    };
    int i;

    fg_vbopen((int)D32(0xF52C8u));               /* 0x1190F Page1 */
    fg_move(0, 0);                               /* 0x1191B */
    cat_show(DSTR(0xB0D69u), 3, (int)D32(0xF40D8u));   /* 0x11931 "maped.gif", flags 3, CT2 */
    for (i = 0; i < 12; i++)
        f3AE90(cuts[i].spr, cuts[i].x, cuts[i].y, cuts[i].w, cuts[i].h);
    fg_vbclose();                                /* 0x11A7D */
}

/* 0x60A80: an initializer (Watcom XI table, run before main): the running sums of the seven dwords at 0xC7618 into
   0x10F2F4..0x10F308 (G: thresholds read at 0x63893..0x63939). */
void f60A80(void)
{
    uint32_t a = D32(0xC7618u), d;

    setD32(0x10F2F4u, a);                             /* 0x60A99 */
    a += D32(0xC761Cu);
    setD32(0x10F2F8u, a);                             /* 0x60AA0 */
    d = a + D32(0xC7620u);
    setD32(0x10F2FCu, d);                             /* 0x60AB3 */
    d += D32(0xC7624u);
    setD32(0x10F300u, d);                             /* 0x60AC0 */
    d += D32(0xC7628u);
    setD32(0x10F304u, d);                             /* 0x60ACD */
    d += D32(0xC762Cu);
    setD32(0x10F308u, d);                             /* 0x60AD5 */
}

/* 0x7BB14: the CRC-32 table (reflected polynomial 0xEDB88320) into the 256 dwords at 0x11260C, for the file checks
   0x7BB84 / 0x7BBF0. */
void f7BB14(void)
{
    int32_t i, k;

    for (i = 0; i <= 0xFF; i++) {                     /* 0x7BB65 */
        uint32_t c = (uint32_t)i;                     /* 0x7BB71 */
        for (k = 8; k > 0; k--)                       /* 0x7BB74, 0x7BB35 */
            c = (c & 1) ? (c >> 1) ^ 0xEDB88320u : c >> 1;   /* 0x7BB3E..0x7BB4D, 0x7BB2C */
        setD32(0x11260Cu + 4u * (uint32_t)i, c);      /* 0x7BB58 */
    }
}
