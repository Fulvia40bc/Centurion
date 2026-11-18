/* ui_text.c: the text routines of the UI kit, AC_MAIN.EXE 0x39F90..0x3AE8B (font selection, printing, centring,
   the title lettering, word wrap, the colour-code printer). Literal translations; prototypes and the data layouts in
   ui.h. Grades: D = read in the code at the address given; G = inference.

   Runtime mappings used here (Watcom C runtime -> standard C):
     0x7DB3D sprintf -> snprintf with the size of the original stack buffer (D: body calls __prtf 0x94C08, then
             stores the terminating 0 at buf[n])
     0x7D555 printf  -> printf + fflush(stdout) (D: vfprintf 0x9480D on the FILE at 0xD5D56 = stdout)
     0x7D5A6 getch   -> getch_7D5A6 below: the ungetch slot dword 0xD5D1C (D), else int 21h AH=8 = wait for a key from
             the keyboard without echo (D); in the port the keyboard is the window's, read through fg_intkey (G)
     repne scasb / not / dec (inline strlen) -> strlen */

#include <stdio.h>
#include <string.h>

#include "state.h"
#include "fg.h"
#include "ui.h"

/* 0x7D5A6: Watcom getch. A character put back by ungetch (dword 0xD5D1C, cleared on read, D 0x7D5A9..0x7D5AE) comes
   first; otherwise wait for a key (int 21h AH=8, D 0x7D5B8). DOS returns 0 for an extended key and its scan code on the
   next call: the port keeps that scan code in the same slot (G: the port's way to give the second byte). */
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
            setD32(0xD5D1Cu, aux);            /* extended key: 0 now, the scan code next time (G) */
            return 0;
        }
        fg_waitfor(1);                        /* port: no busy loop while waiting (one 18.2 Hz tick) */
    }
}

/* 0x39F90: select font 0..2 and set the current-font globals */
void f39F90(int font)
{
    char msg[0x34];                           /* [ebp-0x44] .. [ebp-0x10] */
    int baseline;

    /* 0x39FA9: dword [2*font+0xF8B2A] sar 16 = the int16 at 0xF8B2C+2*font, the Fastgraph font number */
    if (S16(UI_FONT_NUM + 2u * (uint32_t)font) != 0) {
        fgf_select(S16(UI_FONT_NUM + 2u * (uint32_t)font));     /* 0x39FC5: 0x8E188 */
        setD16(UI_FONT_CUR, (uint16_t)font);                    /* 0x39FD7: word [0xF8AFE] = ax */
        setD32(UI_FONT_HEIGHT, (uint32_t)fgf_height(DSTR(0xC1940u), 2));   /* 0x39FDD: height of "Mg" */
        baseline = fgf_baseline();                              /* 0x39FEA: 0x8E23E */
        /* 0x39FEF: dx = word [0xF8AF8]; sub edx, eax; word [0xF8B00] = dx (16-bit result) */
        setD16(UI_FONT_ASCENT, (uint16_t)(D16(UI_FONT_HEIGHT) - (uint32_t)baseline));
        setD16(UI_FONT_BASE, (uint16_t)baseline);               /* 0x39FF8 */
    } else {
        snprintf(msg, sizeof msg, DSTR(0xB3B73u), font);        /* 0x3A014: sprintf "Font %d is not loaded\n" */
        f3A780(msg);                                            /* 0x3A01F */
    }
}

/* 0x3A02C: the current font index */
int f3A02C(void)
{
    return S16(UI_FONT_CUR);                  /* 0x3A039: dword [0xF8AFC] sar 16 = int16 [0xF8AFE] */
}

/* 0x3A044: print s with its top at (x, y) */
void f3A044(int x, int y, const char *s)
{
    if (strlen(s) <= 0x80) {                  /* 0x3A06C: cmp ecx, 0x80; ja (unsigned) */
        fg_move(x, S16(UI_FONT_ASCENT) + y);  /* 0x3A074: dword [0xF8AFE] sar 16 = int16 [0xF8B00] */
        f3AD08(s);                            /* 0x3A08F */
    } else {
        f3A044(x, y, DSTR(0xB3B8Au));         /* 0x3A096: "String too long" */
    }
}

/* 0x3A0B0: print s centred in [x, x+w), top at y */
void f3A0B0(int x, int y, int w, const char *s)
{
    int width;

    if (strlen(s) <= 0x80) {                  /* 0x3A0DA: unsigned ja */
        width = fgf_width(s, (int)strlen(s)); /* 0x3A0F4: 0x8A7DD */
        /* 0x3A10A: edx = w - width; (edx - (edx sar 31)) sar 1 = signed division by 2, truncating toward 0 */
        fg_move((w - width) / 2 + x, S16(UI_FONT_ASCENT) + y);  /* 0x3A11E */
        f3AD08(s);                            /* 0x3A129 */
    } else {
        f3A044(x, y, DSTR(0xB3B9Au));         /* 0x3A130: "String too long" (a second copy of the string) */
    }
}

/* 0x3A18C: the title lettering: font 2, spacing 2, a shadow in color+4 and color+8, then the text in color */
void f3A18C(int x, int y, int w, int color, const char *s)
{
    f39F90(2);                                /* 0x3A1AD */
    fgf_spacing(2);                           /* 0x3A1B4: 0x8E244 */
    fg_setcolor(color + 4);                   /* 0x3A1C3 */
    f3A0B0(x + 1, y - 1, w, s);               /* 0x3A1D9 */
    f3A0B0(x + 1, y + 1, w, s);               /* 0x3A1EC */
    f3A0B0(x + 1, y, w, s);                   /* 0x3A1FE */
    f3A0B0(x, y + 1, w, s);                   /* 0x3A210 */
    fg_setcolor(color + 8);                   /* 0x3A21C */
    f3A0B0(x - 1, y - 1, w, s);               /* 0x3A232 */
    f3A0B0(x, y - 1, w, s);                   /* 0x3A244 */
    f3A0B0(x - 1, y, w, s);                   /* 0x3A256 */
    f3A0B0(x - 1, y + 1, w, s);               /* 0x3A269 */
    fg_setcolor(color);                       /* 0x3A272 */
    f3A0B0(x, y, w, s);                       /* 0x3A286 */
    f39F90(1);                                /* 0x3A290 */
    fgf_spacing(0);                           /* 0x3A297 */
}

/* 0x3A42C: word-wrap text into lines of width w from (x, y); align 0 left, 1 right, 2 centred; draw 0 = measure only.
   *maxw = the widest line + 1; returns the y under the last line, or -1 when a line's y + baseline - 1 >= 480. */
int f3A42C(int x, int y, int w, uint32_t align, const char *text, int32_t *maxw, int draw)
{
    char line[0x98];                          /* [ebp-0xE8] .. [ebp-0x50]: 152 bytes */
    const char *linestart, *p, *q;            /* [ebp-0x3C], [ebp-0x38], [ebp-0x34] */
    int n, width, empty, i, k, len, lw;       /* [ebp-0x28], [ebp-0x2C], [ebp-0x30], [ebp-0x1C], [ebp-0x18],
                                                 [ebp-0x14], [ebp-0x20] / [ebp-0x24] */
    uint8_t more, nonblank;                   /* [ebp-0xC], [ebp-0x10] */
    int back;

    *maxw = 0;                                /* 0x3A456 */
    linestart = text;                         /* 0x3A45F */
    p = linestart;                            /* 0x3A465 */
    more = 1;                                 /* 0x3A468 */

    while (more != 0) {                       /* 0x3A46C */
        n = 0;                                /* 0x3A478 */
        width = 0;                            /* 0x3A47E */
        empty = 0;                            /* 0x3A481 */

        /* 0x3A484: take characters while the line is narrower than w */
        while (width < w) {                   /* 0x3A487: signed jge */
            int c = (signed char)*p;          /* 0x3A48F: movsx */
            if (c == 0 || c == 0x0A)          /* 0x3A492, 0x3A496 */
                break;
            if (*p != 0x14 && c != 0x0D && (uint8_t)*p < 0x80) {   /* 0x3A4A0, 0x3A4A5, 0x3A4B5 (and 0xFF; jge) */
                width += f3A748((uint8_t)c);  /* 0x3A4BD: al = the character */
            } else if (*p == 0x14) {          /* 0x3A4CC */
                y += 2;                       /* 0x3A4D1: 0x14 moves the text 2 pixels down */
            }
            n++;                              /* 0x3A4D8 */
            p++;                              /* 0x3A4DE */
        }
        if (n == 0)                           /* 0x3A4E3 */
            empty = 1;                        /* 0x3A4E9: an empty line (two newlines, or the end) */

        /* 0x3A4F0: too wide: step back to a space, hyphen, newline or the end of the text, but not before the line's
           start. The step back comes first (0x3A4F8) when the line is too wide; then the test (0x3A4FE). */
        back = (width >= w);                  /* 0x3A4F6: jl skips the first step */
        for (;;) {
            if (back) {
                p--;                          /* 0x3A4F8 */
                n--;                          /* 0x3A4FB */
            }
            back = 1;
            if (*p == ' ' || *p == 0 || *p == 0x0A || *p == '-')   /* 0x3A501, 0x3A50E, 0x3A510, 0x3A518 */
                break;
            if (p <= text)                    /* 0x3A520: unsigned jbe */
                break;
            if (!(p > linestart))             /* 0x3A528: unsigned ja loops */
                break;
        }

        /* 0x3A52D: if only spaces follow up to the newline or the end, skip them */
        nonblank = 0;                         /* 0x3A52F */
        q = p;                                /* 0x3A535 */
        while ((signed char)*q != 0x0A && *q != 0) {   /* 0x3A53E, 0x3A543 */
            if (*q != ' ') {                  /* 0x3A54C */
                nonblank = 1;                 /* 0x3A551 */
                break;
            }
            q++;                              /* 0x3A55A */
        }
        if (nonblank == 0)                    /* 0x3A564 */
            p = q;                            /* 0x3A56B */
        if (*p == '-')                        /* 0x3A571 */
            n++;                              /* 0x3A579: the hyphen stays on this line */
        if (*p == 0)                          /* 0x3A584 */
            more = 0;                         /* 0x3A588: the last line */

        if (n > 0 || empty != 0) {            /* 0x3A58B: signed jg; 0x3A591 */
            /* 0x3A59B: copy the line without the 0x14 and 0x0D bytes (colour codes stay) */
            k = 0;                            /* 0x3A59D */
            for (i = 0; i < n; i++) {         /* 0x3A5A0, 0x3A5AE: signed jge */
                if (linestart[i] == 0x14)     /* 0x3A5BB */
                    continue;
                if ((signed char)linestart[i] == 0x0D)  /* 0x3A5C9 */
                    continue;
                /* Port bound: the original's 152-byte stack buffer would overflow past 151 bytes (only a line of
                   zero-width bytes can be that long; no game text is known to reach it, G). The copy stops there. */
                if (k >= (int)sizeof line - 1)
                    break;
                line[k] = linestart[i];       /* 0x3A5D9 */
                k++;                          /* 0x3A5E3 */
            }
            line[k] = 0;                      /* 0x3A5ED */
            len = k;                          /* 0x3A5F7 */
            lw = fgf_width(line, len);        /* 0x3A605 */
            if (lw > *maxw)                   /* 0x3A616: signed jle */
                *maxw = lw;                   /* 0x3A620 */

            /* 0x3A622: dword [0xF8AFA] sar 16 = int16 [0xF8AFC], the baseline (not the height) */
            if (S16(UI_FONT_BASE) + y - 1 >= 0x1E0)    /* 0x3A62E: signed jge */
                return -1;                    /* 0x3A701: *maxw is not incremented on this path */

            if (draw != 0) {                  /* 0x3A63C */
                if (align < 1) {              /* 0x3A647: unsigned jb */
                    /* 0x3A6EC: test eax, eax is always true here (align == 0); its else branch (0x3A6F4, the
                       line step then 0x3A727) is dead code */
                    f3A044(x, y, line);       /* 0x3A65C */
                } else if (align == 1) {      /* 0x3A650: jbe */
                    lw = fgf_width(line, len);            /* 0x3A6BB */
                    f3A044(x + w - lw, y, line);          /* 0x3A6DA: right-aligned */
                    /* 0x3A6DF: the line step is done here and the loop continues at 0x3A727, the same as 0x3A708 */
                } else if (align == 2) {      /* 0x3A655 */
                    lw = fgf_width(line, len);            /* 0x3A67D */
                    /* 0x3A691: (w - lw) / 2, signed, truncating toward 0 */
                    f3A044((w - lw) / 2 + x, y, line);    /* 0x3A6A9: centred */
                }
                /* other align values: nothing is drawn (0x3A657) */
            }
            y += S32(UI_FONT_HEIGHT) + 2;     /* 0x3A708: line step = height + 2 */
            p++;                              /* 0x3A713: past the space, hyphen or newline */
            linestart = p;                    /* 0x3A719 */
        } else {
            /* 0x3A721: nothing fits: the line's first word is w or more pixels wide (the step back at 0x3A4F8 comes
               before the test, so a break right after a word that ends exactly at w is not seen either: "abc def"
               with width("abc") == w lands here, D 0x3A4F0..0x3A52B). The box widens by one pixel. */
            w++;                              /* 0x3A724 */
            /* BUG(1995): the shared tail 0x3A727 then steps past the line's first byte, which here is the first
               letter of the too-long word, not a separator: each pass drops one letter of the word until the rest
               fits in the widened box. Fixed: when that byte is a letter (not a space, newline or the end), try the
               same line again with the wider box; it terminates, as w grows by one each pass until the word fits.
               A leading space is still skipped as the original does. */
            if (!g_original && *p != ' ' && *p != 0x0A && *p != 0) {
                linestart = p;
                continue;
            }
            p++;                              /* 0x3A727 */
            linestart = p;                    /* 0x3A72D */
        }
    }
    (*maxw)++;                                /* 0x3A738 */
    return y;                                 /* 0x3A73A */
}

/* 0x3A748: width of the one character c in the current font */
int f3A748(uint8_t c)
{
    setD8(0xC1944u, c);                       /* 0x3A768: the 1-byte string at 0xC1944 (0 follows it) */
    return fgf_width(DSTR(0xC1944u), 1);      /* 0x3A76D: 0x8A7DD */
}

/* 0x3A780: debug message: print msg on the console and wait for a key */
void f3A780(const char *msg)
{
    printf(DSTR(0xB3BBAu), msg);              /* 0x3A79C: printf("%s", msg), 0x7D555 */
    fflush(stdout);                           /* port: the message must show before the wait */
    (void)getch_7D5A6();                      /* 0x3A7A4: 0x7D5A6 */
}

/* 0x3AD08: print s at the graphics cursor; a byte >= 0x80 sets the colour to byte & 0x7F */
void f3AD08(const char *s)
{
    char buf[0xC8];                           /* [ebp-0xEC] .. [ebp-0x24]: 200 bytes */
    const char *src = s;                      /* [ebp-0x20] */
    char *dst = buf;                          /* [ebp-0x1C] */
    int len, i;                               /* [ebp-0x14], [ebp-0x18] */

    len = (int)strlen(s);                     /* 0x3AD3E */
    if (f3AE3C(s, len) == 0) {                /* 0x3AD47 */
        fgf_print(s, len);                    /* 0x3AE29: no colour codes: the whole string at once (0x8E253) */
        return;
    }
    for (i = 0; i < len; i++) {               /* 0x3AD5E, 0x3ADCA: signed jge */
        if ((uint8_t)*src >= 0x80) {          /* 0x3ADD9: and 0xFF; jge */
            *dst = 0;                         /* 0x3AD6C */
            if (strlen(buf) != 0)             /* 0x3AD79 */
                fgf_print(buf, (int)strlen(buf));          /* 0x3AD95: the run before the colour code */
            dst = buf;                        /* 0x3ADA3 */
            fg_setcolor((uint8_t)*src & 0x7F);/* 0x3ADB3: and al, 0x7F */
            src++;                            /* 0x3ADBE */
        } else {
            /* Port bound: the original's 200-byte stack buffer would overflow on a run of more than 199 bytes
               between colour codes. The port prints the run so far and goes on; fgf_print continues at the cursor,
               so the output is the same (unless a 0x7F escape pair is split exactly there, G). */
            if (dst - buf >= (int)sizeof buf - 1) {
                *dst = 0;
                fgf_print(buf, (int)strlen(buf));
                dst = buf;
            }
            *dst = *src;                      /* 0x3ADE8 */
            src++;                            /* 0x3ADED */
            dst++;                            /* 0x3ADF3 */
        }
    }
    *dst = 0;                                 /* 0x3AE01 */
    if (strlen(buf) != 0)                     /* 0x3AE0E */
        fgf_print(buf, (int)strlen(buf));     /* 0x3AE2A: the last run */
}

/* 0x3AE3C: 1 if any of the first len bytes of s is a colour code (>= 0x80), else 0 */
int f3AE3C(const char *s, int len)
{
    int found = 0;                            /* 0x3AE54: [ebp-0x4] */
    int i;

    for (i = 0; i < len; i++) {               /* 0x3AE57, 0x3AE65: signed jge */
        if ((uint8_t)s[i] >= 0x80) {          /* 0x3AE77: and 0xFF; jl continues */
            found = 1;                        /* 0x3AE7E */
            break;
        }
    }
    return found;                             /* 0x3AE85 */
}
