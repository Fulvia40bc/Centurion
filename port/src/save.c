/* Save and restore dialogs of AC_MAIN.EXE (module S1 of docs/MAP_PLAN.md, the dialog half): 0x5FF28 (RESTORE GAME),
   0x5E7D0 (SAVE GAME), 0x601D8 (the slot dialog), 0x60694 (one-line text input), 0x6016C (save-version check),
   0x3CD10 (screen grab into the background buffer), and the port's title_restore (main 0x11557..0x115A9). Literal
   translations; the address of each instruction a line comes from is in its comment. D = read in the code,
   G = inference. Prototypes: save.h; the loading half (0x5F05C, 0x7BBF0, 0x726DC): saveload.h.

   Save files (D): savgame0.dat .. savgame9.dat are the player's slots, savgamea.dat (slot 10) the autosave written
   after every phase, savgameb.dat (slot 11) the one written at every round end. The header is int16 version, then
   from version 0x28 the 22 bytes "ADVANCED CIVILIZATION\0", then a 51-byte description (0x4B bytes, or 0x35 before
   0x28). SAVE does not write the live state: it copies the autosave savgamea.dat to the slot and patches the header.
   RESTORE loads the slot and, unless it is slot 10 itself, copies it over savgamea.dat.

   Data used (D): dialog working copy 0x10A174 (1002 bytes = count + 50 records) from the templates 0xC6628 (11
   records: slots 0x42..0x4B at x 170, y 115 + 25i, 300 x 19, labels 0x109E98 + 61i; CANCEL 0x4E) and 0xC6A12 (word 0,
   then a dialog of 3 records: 0x4C, 0x4D, CANCEL); slot descriptions char[12][61] at 0x109E98 (the labels of the
   slot buttons; 12 * 61 = 732 bytes end exactly at 0x10A174. The inventory's "51i" is wrong: the stride is
   61, D 0x602E3..0x602F0 i<<4 - i, <<2, + i); [0xF8E78] the current dialog (saved and put back by 0x601D8);
   [0xF52C8] back buffer, [0xF52CC] background buffer, int8 [0xF52D0] the picture in it; [0xF8AF8] font height;
   0xE1628/0xE162C/0xE1630 mouse x, y, buttons.

   Runtime calls (Watcom C runtime -> standard C): 0x7DB3D sprintf -> snprintf bounded to the original stack buffer,
   0x7D8C8 fopen, 0x7DB6D fread, 0x912D5 fwrite, 0x91D94 fseek, 0x7D95E fclose (names and modes as the original,
   relative to the current folder: the port runs in the game folder). 0x7D4EC system: NOT called; the two DOS copy
   commands it ran are done in C by copy_file below. 0x7D5A6 getch: getch_7D5A6 below, a copy of ui_text.c's static
   one (it is not exported in ui.h; the same body: the ungetch slot 0xD5D1C, else wait for a key through fg_intkey).
   repne scasb / not / dec -> strlen; the inline byte-pair copy loops -> strcpy / strcat. __CHK dropped. The debug
   stubs 0x78B0C / 0x78B24 are not called by these six routines.

   Heap pointers in 32-bit data fields: none here (0x5F05C, in saveload.c, re-reads the heap lists 0xF0984[1..11]).

   1995 bugs fixed (the original's behaviour kept under g_original, each marked BUG(1995) below):
     1. 0x5FF28 loads the save into the game globals (0x5FF9C) BEFORE the CRC and version checks (0x60005..0x6003E):
        a rejected file ("corrupted" / "earlier format") returns -15 as if cancelled, but the state is already
        overwritten (D). In-game (0x3FEAA, -15 -> 0x3FD35) the game goes on with the rejected file's state (G: the
        consequence). Fix: read the version, run both checks, and load only if they pass. For an accepted file the
        calls (and the random numbers of f726DC) are the same and in the same order.
     2. 0x5FF28 chooses the "Restoring" font by measuring the description in the CURRENT font (font 1, left by the
        slot dialog 0x60576) against 635 px, then prints it in font 2 (D 0x600F3..0x6010E). A description narrower
        than 635 px in font 1 (all of them: 50 characters) is printed in font 2, up to about twice as wide, and runs
        off both edges of the screen; the font-1 fallback never happens (G: the intent is to measure in font 2). Fix:
        select font 2 before measuring.
     3. 0x60694 clears [0xE1630] (0x607FB) and tests it after fg_intkey (0x6080D), but nothing reads the mouse in
        between: the click branch (0x6081E..0x60A42) is dead (D: the only stores to 0xE1630 in the program are
        0x16798, 0x167DB, 0x29BFC, 0x29C76 and 0x607FB; fg_mousepos writes it only when called). So CANCEL cannot be
        clicked while typing a description (Esc works). Fix: fg_mousepos into 0xE1628/0xE162C/0xE1630 after
        fg_intkey (G: the intended read, as f78BA4 does).
     4. 0x60694 on Esc leaves through 0x60708 after fg_vbopen([0xF52C8]) (0x606E4) without fg_vbclose (D): the back
        buffer stays the active surface, so a caller drawing next draws into it instead of the screen (G: the
        callers 0x315B7 / 0x3209F draw after it). Fix: fg_vbclose before leaving.
   Port guards (not 1995 behaviour that can be kept): a slot file that does not open in 0x5FF28 (deleted after the
   dialog listed it) or 0x5E7D0 would go to fread/f5F05C with a NULL FILE in the original; the port treats it as
   cancelled / skips the header patch.

   By design: the 51 description bytes 0x5E7D0 writes come from a 64-byte stack buffer filled only up to its
   terminator by 0x601D8's strcpy; the original writes stack leftovers after the terminator, the port zeros. And
   the original's system() result: COMMAND.COM's COPY sets no exit code (G), so "Save failed" appeared only when
   COMMAND.COM itself could not run; the port's copy reports a failed read or write, so the message appears when the
   copy really fails (savgamea.dat missing, disk full). */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "fg.h"
#include "save.h"
#include "saveload.h"

/* Defined in ui_screen.c, not in ui.h (ast.c, title.c and ui_screen.c declare it the same way): 0x78508 empties the
   keyboard buffer (fg_intkey until key 0). */
void f78508(void);

/* ---- Runtime replacements ---------------------------------------------------------------------------------------- */

/* 0x7D5A6: Watcom getch, as ui_text.c's getch_7D5A6 (static there). A character put back by ungetch (dword 0xD5D1C,
   cleared on read, D 0x7D5A9..0x7D5AE) comes first; otherwise wait for a key (int 21h AH=8, D 0x7D5B8). */
static int getch_7D5A6(void)
{
    unsigned char key, aux;
    uint32_t pending = D32(0xD5D1Cu);         /* 0x7D5A9 */
    setD32(0xD5D1Cu, 0);                      /* 0x7D5AE */
    if (pending != 0)                         /* 0x7D5B4 */
        return (int)pending;
    for (;;) {                                /* 0x7D5B8: int 21h AH=8 waits for a key */
        fg_intkey(&key, &aux);
        if (key != 0)
            return key;                       /* 0x7D5BC: and eax, 0xFF */
        if (aux != 0) {
            setD32(0xD5D1Cu, aux);            /* extended key: 0 now, the scan code next time (G, as ui_text.c) */
            return 0;
        }
        fg_waitfor(1);                        /* port: no busy loop while waiting (one 18.2 Hz tick) */
    }
}

/* system("copy SRC DST >NUL") (0x7D4EC) done in C: read the whole file, write it (DOS COPY of one file is a binary
   copy). Returns 0 on success, -1 if SRC cannot be read or DST cannot be written in full (the port's stand-in for a
   non-zero system() result). */
static int copy_file(const char *src, const char *dst)
{
    FILE *in, *out;
    uint8_t *data;
    long n;
    int rc = -1;

    in = fopen(src, "rb");
    if (in == NULL)
        return -1;
    if (fseek(in, 0, SEEK_END) != 0 || (n = ftell(in)) < 0 || fseek(in, 0, SEEK_SET) != 0) {
        fclose(in);
        return -1;
    }
    data = malloc(n > 0 ? (size_t)n : 1u);
    if (data == NULL || fread(data, 1, (size_t)n, in) != (size_t)n) {
        free(data);
        fclose(in);
        return -1;
    }
    fclose(in);
    out = fopen(dst, "wb");
    if (out != NULL) {
        if (fwrite(data, 1, (size_t)n, out) == (size_t)n)
            rc = 0;
        if (fclose(out) != 0)
            rc = -1;
    }
    free(data);
    return rc;
}

/* fread(&local, 1, 2, fp) into a little-endian int16 local: bytes fread does not deliver leave the local as it was
   (the original's locals keep their earlier contents; the port's start at the value given by the caller). */
static void fread16(int16_t *v, FILE *fp)
{
    uint8_t b[2] = { (uint8_t)*v, (uint8_t)((uint16_t)*v >> 8) };

    (void)fread(b, 1, 2, fp);
    *v = (int16_t)(uint16_t)(b[0] | b[1] << 8);
}

/* ---- 0x3CD10: grab the screen into the background buffer --------------------------------------------------------- */

void f3CD10(void)
{
    f78884();                                         /* 0x3CD20 mouse off */
    fg_vbopen(S32(0xF52CCu));                         /* 0x3CD2C */
    fg_vbcut(0, 0x27F, 0, 0x1DF, 0, 0x1DF);           /* 0x3CD49 */
    fg_vbclose();                                     /* 0x3CD51 */
    f788AC();                                         /* 0x3CD56 mouse on */
    setD8(0xF52D0u, 0);                               /* 0x3CD5B xor ah,ah; mov [0xF52D0],ah: f3CD6C reloads next time */
}

/* ---- 0x6016C: save-version check ------------------------------------------------------------------------------- */

int f6016C(int version)
{
    int32_t ok = 1;                                   /* 0x60180 [ebp-0x8] */

    /* 0x60187: with PBEM or [0xF40C2] != 0 the limit is 0x2C (0x6019A jl reject) ... */
    if (((D8(0xE1624u) & 0x80) != 0 || S16(0xF40C2u) != 0) && version < 0x2C)
        goto reject;
    /* 0x601A0: ... otherwise 0x35 (0x601B3 cmp 0x35; jge accept) */
    if ((D8(0xE1624u) & 0x80) == 0 && S16(0xF40C2u) == 0 && version < 0x35)
        goto reject;
    return ok;                                        /* 0x601CD */
reject:
    ok = 0;                                           /* 0x601B9 */
    f3A7B0(0xB60CAu);                                 /* 0x601C3 "That save game is an earlier format that is no longer valid. Press a key." */
    (void)getch_7D5A6();                              /* 0x601C8 */
    return ok;
}

/* ---- 0x60694: one-line text input ------------------------------------------------------------------------------ */

/* Arguments: eax x, edx y, ebx w, ecx maxlen, stack buf ([ebp+8]), align ([ebp+0xC]), dlg ([ebp+0x10]); ret 0xC.
   buf is in the data object at every call site (0x109E98 + 61i here; 0xF4104 + .. and 0xF7B10 + .. at 0x315B7 and
   0x3209F), passed as DP(addr). */
int f60694(int x, int y, int w, int maxlen, char *buf, uint32_t align, uint32_t dlg)
{
    unsigned char key;                                /* [ebp-0x10] */
    unsigned char aux;                                /* [ebp-0xC] */
    int32_t len;                                      /* [ebp-0x24] */
    int32_t result;                                   /* [ebp-0x1C] */
    int32_t i;                                        /* [ebp-0x18] */
    uint32_t rec;                                     /* [ebp-0x14] */
    char line[0x64];                                  /* [ebp-0x94] buf + "_" */

    len = 0;                                          /* 0x606B7 */
    result = -100;                                    /* 0x606BA */
    f78884();                                         /* 0x606C1 mouse off */
    f78508();                                         /* 0x606C6 empty the keyboard buffer */
    key = 0x18;                                       /* 0x606CB: start as if Ctrl-X were pressed */

    while (key != 0x0D) {                             /* 0x606CF movzx; cmp 0xD; je 0x60A68 */
        fg_vbopen(S32(0xF52C8u));                     /* 0x606E4 */
        /* 0x606EC: unsigned compares on the key */
        if (key == 0x18) {                            /* 0x6070D: Ctrl-X clears the field */
            len = 0;
            buf[0] = 0;                               /* 0x60717 */
            fg_setcolor(13);                          /* 0x6071A */
            fg_rect(x, x + w - 1, y, y + S32(0xF8AF8u) + 1);    /* 0x6073D */
        } else if (key == 0x1B) {                     /* 0x606F9 cmp 0x1B; jne */
            result = -15;                             /* 0x60701 */
            if (!g_original)
                fg_vbclose();   /* BUG(1995) 4: the original leaves (0x60708) with the back buffer still open */
            break;                                    /* 0x60708 jmp 0x60A68 */
        } else if (key == 0x08) {                     /* 0x6088B: Backspace */
            if (len > 0) {                            /* 0x6082F cmp 0; jle 0x60745 */
                len--;                                /* 0x60839 */
                buf[len] = 0;                         /* 0x60844 */
                fg_setcolor(13);                      /* 0x60847 */
                fg_rect(x, x + w - 1, y, y + S32(0xF8AF8u) + 1);    /* 0x6086A */
            }
        } else if (key >= 0x20 && key <= 0x7E         /* 0x6088F jl, 0x608A2 jg 0x60745 */
                   && len < maxlen                    /* 0x608AE cmp len, maxlen; jge 0x60745 */
                   && fgf_width(buf, (int)strlen(buf)) <= w) {      /* 0x608C9; 0x608D1 jg 0x60745 */
            buf[len++] = (char)key;                   /* 0x608DA..0x608E8 */
            buf[len] = 0;                             /* 0x608F0 */
            if (fgf_width(buf, (int)strlen(buf)) > w) {             /* 0x60905; 0x60912 jg 0x60877 */
                len--;                                /* 0x60877..0x6087A */
                buf[len] = 0;                         /* 0x60883 */
            } else {
                fg_setcolor(13);                      /* 0x6091A */
                fg_rect(x, x + w - 1, y, y + S32(0xF8AF8u) + 1);    /* 0x6093D */
            }
        }

        /* 0x60745: draw buf + "_" */
        strcpy(line, buf);                            /* 0x6074F */
        strcat(line, DSTR(0xB6144u));                 /* 0x6077C "_" */
        fg_setcolor(15);                              /* 0x60797 */
        if (align < 1) {                              /* 0x607A2 jb (unsigned) */
            if (align == 0)                           /* 0x60993 */
                f3A044(x, y, line);                   /* 0x6094A..0x60956 left */
        } else if (align == 1) {                      /* 0x607AB jbe */
            f3A3A0(x + w - 1, y, line);               /* 0x60979..0x60989 right edge at x + w - 1 */
        } else if (align == 2) {                      /* 0x607B4 */
            f3A0B0(x, y, w, line);                    /* 0x60960..0x6096F centred */
        }
        fg_vbclose();                                 /* 0x607BA */
        f78884();                                     /* 0x607BF */
        fg_vbpaste(x, x + w - 1, y, y + S32(0xF8AF8u) + 1, x, y + S32(0xF8AF8u) + 1);   /* 0x607EC */
        f788AC();                                     /* 0x607F4 */

        /* 0x607F9: poll the keyboard (and the mouse) until a key comes or a button ends the edit */
        for (;;) {
            setD32(0xE1630u, 0);                      /* 0x607FB */
            fg_intkey(&key, &aux);                    /* 0x60808 fg_intkey([ebp-0x10], [ebp-0xC]) */
            if (!g_original) {
                /* BUG(1995) 3: nothing reads the mouse here in the original, so [0xE1630] is still 0 at 0x6080D and
                   the click branch below never runs. The port reads it as f78BA4 does. */
                int mx, my, mb;
                fg_mousepos(&mx, &my, &mb);
                setD32(0xE1628u, (uint32_t)mx);
                setD32(0xE162Cu, (uint32_t)my);
                setD32(0xE1630u, (uint32_t)mb);
            }
            if (D32(0xE1630u) != 0) {                 /* 0x60816 test edx,edx; je 0x60A58 */
                result = -100;                        /* 0x6081E */
                for (i = 0; S16(dlg + DLG_COUNT) > i; i++) {        /* 0x609A2 movsx word [dlg]; jle 0x60A45 */
                    rec = dlg + DLG_REC(i);           /* 0x609B1..0x609C8 dlg + 2 + 20i */
                    if (S8(rec + BTN_ENABLED) != 1)   /* 0x609CE [rec+0xD] sar 24; cmp 1; jne */
                        continue;
                    /* 0x609D9..0x60A1D, signed: x <= mx < x + w, y <= my < y + h */
                    if (S16(rec + BTN_X) > S32(0xE1628u))                       /* 0x609E7 jg */
                        continue;
                    if (S16(rec + BTN_X) + S16(rec + BTN_W) <= S32(0xE1628u))   /* 0x609EC [rec+2] sar 16; 0x609FC jle */
                        continue;
                    if (S16(rec + BTN_Y) > S32(0xE162Cu))                       /* 0x60A01 [rec] sar 16; 0x60A0E jg */
                        continue;
                    if (S16(rec + BTN_Y) + S16(rec + BTN_H) <= S32(0xE162Cu))   /* 0x60A13 [rec+4] sar 16; 0x60A1D jle */
                        continue;
                    f3C5D4(dlg, S16(rec + BTN_ID));   /* 0x60A23 [rec+0xC] sar 16; 0x60A2F */
                    result = f3FB94(S16(rec + BTN_ID));                         /* 0x60A3D */
                    break;
                }
                if (result == -15) {                  /* 0x60A45 cmp -0xF; jne 0x607F9 */
                    key = 0x0D;                       /* 0x60A4F: end the edit with -15 */
                    break;                            /* 0x60A53 jmp 0x606CF */
                }
                continue;                             /* 0x60A49 jne 0x607F9: no redraw */
            }
            if (key != 0)                             /* 0x60A58 test dh,dh; je 0x607F9 */
                break;                                /* 0x60A63 jmp 0x606CF */
        }
    }

    f788AC();                                         /* 0x60A68 */
    return result;                                    /* 0x60A6D */
}

/* ---- 0x601D8: the Save Game / Restore Game slot dialog ------------------------------------------------------------ */

/* Arguments: eax mode (1 save, 2 restore), edx desc (a caller's 64-byte stack buffer, [ebp-0x70] / [ebp-0x78]). */
int f601D8(int mode, char *desc)
{
    int32_t result;                                   /* [ebp-0x2C] */
    uint32_t saved_dlg;                               /* [ebp-0x1C] */
    int32_t nslots;                                   /* [ebp-0x24] */
    int32_t h;                                        /* [ebp-0x28] */
    int32_t top;                                      /* [ebp-0x20] */
    int32_t base, end, i;                             /* [ebp-0x38], [ebp-0x34], [ebp-0x30] */
    int32_t r;                                        /* [ebp-0x3C] */
    int32_t lower;                                    /* ebx: 10 px lower for the autosave slots */
    FILE *fp;                                         /* [ebp-0x18] */
    int16_t version = 0;                              /* [ebp-0x14] (port: 0 instead of the stack's leftover) */
    char name[0x14];                                  /* [ebp-0x58] .. [ebp-0x44] */
    char tag[0x18];                                   /* [ebp-0x70] the 22-byte "ADVANCED CIVILIZATION" tag */

    result = -15;                                     /* 0x601F2 */
    saved_dlg = D32(UI_CUR_DIALOG);                   /* 0x601F9 [0xF8E78] */
    nslots = mode == 2 ? 0xC : 0xA;                   /* 0x60201 */
    h = mode == 1 ? 0x15D : 0x199;                    /* 0x60216: 349 or 409 */
    top = (0x1E0 - h) / 2;                            /* 0x6022B..0x6023C signed /2 */
    memcpy(DP(0x10A174u), DP(0xC6628u), 1002);        /* 0x60241 rep movsd 0xFA + movsw */

    if (mode == 2) {                                  /* 0x60254 */
        /* append 0xC6A12's records over the old last record (CANCEL) */
        base = S16(0x10A174u) - 1;                    /* 0x6025E dword [0x10A172] sar 16; dec */
        end = base + S16(0xC6A14u);                   /* 0x6026A dword [0xC6A12] sar 16 */
        for (i = base; i < end; i++)                  /* 0x60288 */
            memcpy(DP(0x10A176u + 20u * (uint32_t)i), DP(0xC6A16u + 20u * (uint32_t)(i - base)), 20);   /* 0x602BB 5 movsd */
        setD16(0x10A174u, (uint16_t)(D16(0x10A174u) + D16(0xC6A14u) - 1u));    /* 0x602C2..0x602D2 16-bit */
    }

    for (i = 0; i < nslots; i++) {                    /* 0x602D9, 0x60412 */
        uint32_t slotdesc = 0x109E98u + 61u * (uint32_t)i;          /* 61i: i<<4 - i, <<2, + i */

        snprintf(name, sizeof name, DSTR(0xB6114u), (unsigned)i);  /* 0x6042B "savgame%x.dat" */
        fp = fopen(name, DSTR(0xB6122u));             /* 0x6043B "rb" */
        if (fp != NULL) {                             /* 0x60446 test; je 0x602E3 */
            fread16(&version, fp);                    /* 0x6045E fread([ebp-0x14], 1, 2, fp) */
            if (version >= 0x28)                      /* 0x60463 movsx; cmp 0x28; jl */
                (void)fread(tag, 1, 0x16, fp);        /* 0x6047C */
            (void)fread(DP(slotdesc), 0x33, 1, fp);   /* 0x604A2 fread(desc, 51, 1, fp): edx 0x33, ebx 1 */
            fclose(fp);                               /* 0x604AA */
            f3C314(0x10A174u, (int16_t)(i + 0x42));   /* 0x604BF enable */
        } else if (mode == 1) {                       /* 0x602F5 cmp ebx,1; jne */
            strcpy((char *)DP(slotdesc), DSTR(0xB6125u));           /* 0x60307 "Empty" */
            f3C314(0x10A174u, (int16_t)(i + 0x42));   /* 0x60330 enable */
        } else {
            setD8(slotdesc, 0);                       /* 0x6034E */
            while (strlen(DSTR(slotdesc)) < 0x32)     /* 0x60374 cmp ecx,0x32; jae (unsigned) */
                strcat((char *)DP(slotdesc), DSTR(0xB612Bu));       /* 0x6039D " " */
            f3C3D0(0x10A174u, (int16_t)(i + 0x42));   /* 0x603C8 disable */
        }
        lower = i >= 0xA ? 0xA : 0;                   /* 0x60335 / 0x603CD / 0x604C4 */
        f3BC64(0x10A174u, (int16_t)(i + 0x42), 25 * i + top + 0x32 + lower);   /* 0x603DC..0x60407 */
    }

    lower = nslots > 0xA ? 0xA : 0;                   /* 0x604D8 jle */
    f3BC64(0x10A174u, 0x4E, 25 * nslots + top + 0x32 + lower);      /* 0x604E7..0x6050C CANCEL */

    fg_vbopen(S32(0xF52C8u));                         /* 0x60517 */
    f3C9D0(0x91, top, 0x15E, h, 3);                   /* 0x60531 */
    f39F90(2);                                        /* 0x6053B */
    fg_setcolor(5);                                   /* 0x60542 */
    f3A0B0(0x91, top + 0xA, 0x15E, DSTR(mode == 1 ? 0xB612Du : 0xB6137u));   /* 0x6056C "Save Game" / "Restore Game" */
    f39F90(1);                                        /* 0x60576 */
    fg_setcolor(0xF);                                 /* 0x6057D */
    f3B05C(0x10A174u);                                /* 0x6058A (its own vbopen/vbclose of [0xF52C8]) */
    fg_vbclose();                                     /* 0x6058F */
    f78884();                                         /* 0x60594 */
    fg_vbpaste(0x91, 0x1EE, top, top + h - 1, 0x91, top + h - 1);   /* 0x605BC */
    f788AC();                                         /* 0x605C4 */

    do
        r = f78BA4(0x10A174u, 0);                     /* 0x605D0 */
    while (r == -100);                                /* 0x605DB cmp -0x64; je */

    if (r != -15) {                                   /* 0x605E3 */
        result = r;                                   /* 0x605EB */
        if (mode == 1)                                /* 0x605F1: edit the description in place */
            r = f60694(0xAF, 25 * result + 0x77, 0x122, 0x32,
                       (char *)DP(0x109E98u + 61u * (uint32_t)result), 2, 0x10A174u);   /* 0x60635 */
    }
    if (r != -15)                                     /* 0x6063D */
        strcpy(desc, DSTR(0x109E98u + 61u * (uint32_t)result));    /* 0x6065D */
    else
        result = r;                                   /* 0x60678 */
    setD32(UI_CUR_DIALOG, saved_dlg);                 /* 0x60681 */
    return result;                                    /* 0x60686 */
}

/* ---- 0x5E7D0: SAVE GAME -------------------------------------------------------------------------------------------- */

void f5E7D0(void)
{
    int16_t version = 0x38;                           /* [ebp-0x10] dword 0x38 (0x5E7E6), read as a word */
    int32_t slot;                                     /* [ebp-0x1C] */
    int32_t rc;                                       /* [ebp-0x18] */
    FILE *fp;                                         /* [ebp-0x14] */
    char name[0x14];                                  /* [ebp-0x30] */
    char desc[0x40] = { 0 };                          /* [ebp-0x70] (port: zeroed; see "By design" above) */

    slot = -15;                                       /* 0x5E7ED */
    f3CD10();                                         /* 0x5E7F4 */
    setD8(0xF52D0u, 0);                               /* 0x5E7FE */
    slot = f601D8(1, desc);                           /* 0x5E809 */
    if (slot != -15) {                                /* 0x5E811 */
        snprintf(name, sizeof name, DSTR(0xB5F70u), slot);         /* 0x5E828 "savgame%d.dat" */
        /* 0x5E840..0x5E84E: sprintf(cmd [ebp-0xD4], "copy savgamea.dat %s >NUL", name); rc = system(cmd).
           Port: the copy in C (no system()). */
        rc = copy_file("savgamea.dat", name);
        if (rc != 0) {                                /* 0x5E859 */
            f3A9F4(DSTR(0xB5F98u));                   /* 0x5E862 "Save failed. Probably due to insufficient disk space." */
        } else {
            fp = fopen(name, DSTR(0xB5FCEu));         /* 0x5E871 "r+b" */
            if (fp != NULL) {                         /* port guard: the original goes on with a NULL FILE */
                fread16(&version, fp);                /* 0x5E889 fread([ebp-0x10], 2, 1, fp) (fread16 asks 1 x 2: the same bytes) */
                fseek(fp, 0, SEEK_CUR);               /* 0x5E898: between the read and the write */
                if (version >= 0x28)                  /* 0x5E89D movsx; cmp 0x28; jl */
                    (void)fwrite(DSTR(0xB5FD2u), 1, 0x16, fp);      /* 0x5E8B8 "ADVANCED CIVILIZATION" + 0 */
                (void)fwrite(desc, 0x33, 1, fp);      /* 0x5E8CD fwrite(desc, 51, 1, fp): edx 0x33, ebx 1 */
                fclose(fp);                           /* 0x5E8D5 */
            }
        }
    }
    f3CC68();                                         /* 0x5E8DA */
    f3CCBC();                                         /* 0x5E8DF */
}

/* ---- 0x5FF28: RESTORE GAME --------------------------------------------------------------------------------------- */

/* 0x5FF96..0x60000: load the open file into the game globals, close it, players, PBEM name, trade offers */
static void restore_load(FILE *fp, int16_t *version, char *desc)
{
    f5F05C(version, desc, fp);                        /* 0x5FF9C (eax version, edx desc, ebx fp) */
    fclose(fp);                                       /* 0x5FFA4 */
    setD32(0xF40ECu, (uint32_t)(S8(0xE1620u) + S8(0xE1621u)));     /* 0x5FFA9..0x5FFBC sar 24 twice, add */
}

/* 0x5FFC8..0x60000 (after result = slot): the PBEM name and, in ACCEPT TRADES, the trade offers */
static void restore_after(void)
{
    if ((D8(0xE1624u) & 0x80) != 0)                   /* 0x5FFCE test dl,0x80 */
        strcpy((char *)DP(0xEE20Cu), DSTR(0x109E84u));             /* 0x5FFDE */
    if (D8(0xF4102u) == 0xE)                          /* 0x5FFF7 phase 14 */
        f726DC();                                     /* 0x60000 */
}

/* 0x60005..0x6003E: the CRC and version checks; returns result, or -15 if the file is rejected */
static int32_t restore_check(int32_t result, int16_t version, const char *name)
{
    if (version >= 0x1E && f7BBF0(name) != 0) {       /* 0x60009 jl; 0x60011; 0x60016 test eax,eax */
        f3A7B0(0xB605Eu);                             /* 0x60024 "Save game has been corrupted and is no longer valid. Press a key." */
        (void)getch_7D5A6();                          /* 0x60029 */
        result = -15;                                 /* 0x6002E (ecx = -15, kept across both calls) */
    }
    if (f6016C(version) == 0)                         /* 0x60035 (both checks run: two messages are possible) */
        result = -15;                                 /* 0x6003E */
    return result;
}

int f5FF28(void)
{
    int16_t version = 0;                              /* [ebp-0x18] */
    FILE *fp;                                         /* [ebp-0x1C] */
    int32_t slot;                                     /* [ebp-0x20] */
    int32_t result;                                   /* [ebp-0x24] */
    char name[0x14];                                  /* [ebp-0x38] */
    char desc[0x40] = { 0 };                          /* [ebp-0x78] */
    char quoted[0x64];                                /* [ebp-0xDC] */

    result = -15;                                     /* 0x5FF40 */
    f3CD10();                                         /* 0x5FF47 */
    setD8(0xF52D0u, 0);                               /* 0x5FF51 */
    slot = f601D8(2, desc);                           /* 0x5FF5C */
    if (slot != -15) {                                /* 0x5FF64 je 0x6014F */
        snprintf(name, sizeof name, DSTR(0xB604Du), (unsigned)slot);   /* 0x5FF7B "savgame%x.dat" */
        fp = fopen(name, DSTR(0xB605Bu));             /* 0x5FF8B "rb" */
        if (fp != NULL) {                             /* port guard: the original hands a NULL FILE to f5F05C */
            if (g_original) {
                /* BUG(1995) 1: the original loads first, then checks; a rejected file has already replaced the
                   game state when -15 is returned. */
                restore_load(fp, &version, desc);
                result = slot;                        /* 0x5FFC5 */
                restore_after();
                result = restore_check(result, version, name);
            } else {
                /* fixed: the version word first (the file's first two bytes, as f5F05C reads it), then both checks,
                   then the load only for an accepted file */
                fread16(&version, fp);
                fseek(fp, 0, SEEK_SET);
                result = restore_check(slot, version, name);
                if (result != -15) {
                    restore_load(fp, &version, desc);
                    restore_after();
                } else {
                    fclose(fp);
                }
            }
        }
        if (result != -15) {                          /* 0x60045 je 0x6014F */
            if (slot != 0xA)                          /* 0x60052: not the autosave itself */
                /* 0x6005B..0x60075: sprintf(cmd [ebp-0x140], "copy %s savgamea.dat >NUL ", name); system(cmd), the
                   result ignored. Port: the copy in C (no system()). */
                (void)copy_file(name, "savgamea.dat");

            /* the "Restoring" screen */
            f78884();                                 /* 0x6007A */
            f3CD6C(8);                                /* 0x60084 background 8 into [0xF52CC] */
            fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(0xF52CCu), S32(0xF52C8u));   /* 0x600AC */
            f3CC14();                                 /* 0x600B4 show the back buffer */
            fg_setcolor(5);                           /* 0x600BB */
            snprintf(quoted, sizeof quoted, DSTR(0xB60BBu), desc);  /* 0x600D3 "\"%s\"" */
            if (!g_original)
                f39F90(2);      /* BUG(1995) 2: the original measures in the current font (1), then prints in font 2 */
            f39F90(fgf_width(quoted, (int)strlen(quoted)) < 0x27B ? 2 : 1);   /* 0x600F3; 0x600FB cmp 0x27B; jge */
            f3A0B0(0, 0xCD, 0x280, DSTR(0xB60C0u));   /* 0x60124 "Restoring" */
            f3A0B0(0, 0xE6, 0x280, quoted);           /* 0x6013B */
            f39F90(1);                                /* 0x60145 */
            f788AC();                                 /* 0x6014A */
        }
    }
    if (result == -15) {                              /* 0x6014F */
        f3CC68();                                     /* 0x60155 */
        f3CCBC();                                     /* 0x6015A */
    }
    return result;                                    /* 0x6015F */
}

/* ---- main 0x11557..0x115A9: the title menu's RESTORE (port: called by main.c on code -0x46) --------------------- */

/* main reaches 0x11561 when bits 0x04 (new game) and 0x80 (PBEM) of [0xE1624] are clear (0x11557, 0x1155C). Returns
   1 = restored (main goes on at 0x115B0: f12394, f3D170, f11A8C, ...), 0 = cancelled (0x115A3 je 0x11168: back to the
   title menu). */
int title_restore(void)
{
    f3CD6C(8);                                        /* 0x11566 background 8 into [0xF52CC] */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(0xF52CCu), S32(0xF52C8u));   /* 0x1158E -> back buffer */
    f3CC14();                                         /* 0x11596 show it */
    if (f5FF28() == -15)                              /* 0x1159B; 0x115A0 cmp -0xF; je 0x11168 */
        return 0;
    setD8(0xE1624u, (uint8_t)(D8(0xE1624u) & 0xFB));  /* 0x115A9 and byte [0xE1624], -5: no longer a new game */
    return 1;
}
