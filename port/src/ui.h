#ifndef UI_H
#define UI_H

/* The UI kit of AC_MAIN.EXE (0x39F90..0x3D170, plus the mouse-visibility stack at 0x78884.., the dialog append
   0x785D4, the title menu 0x4F184 and the quit routine 0x44CAC): prototypes shared by the translations.

   Conventions (the contract between the translated files):
   - fXXXXX = the original function at 0xXXXXX, Watcom register arguments in the order eax, edx, ebx, ecx, then the
     stack slots [ebp+8], [ebp+0xC], ... A function is void when no caller uses eax after the call (all call sites
     checked); otherwise it returns int.
   - Dialogs and button records live in the data object and are passed as uint32_t link addresses (every call site
     passes an immediate address such as 0x1082E0 or 0xF596C, D). Read their fields with state.h's accessors and
     the offsets below.
   - Text arguments are const char *: the same routines receive both strings in the data (immediates such as
     0xB3B8A, pass DSTR(a)) and caller stack buffers (lea reg,[ebp-..], D at the call sites). f3A7B0 is the
     exception: all 29 callers pass a data string, so it takes the link address.
   - Grades: D = read in the code at the address given; G = inference. */

#include <stdint.h>
#include "state.h"

/* ---- Shared structures (offsets in the data object) ------------------------------------------------------------

   Dialog object (button list). The title menu copies the template at 0xC3090 to 0x1082E0 (0x4F1BF: rep movsd 0xFA
   dwords + movsw = 1002 bytes = 2 + 50 records, D) and appends 0xC3868 / 0xC347C / 0xC3C54 to it with f785D4.
     +0x00  int16  count of records (movsx word [dlg], D 0x3B091)
     +0x02  records of 20 bytes: record i at dlg + 2 + 20*i (lea [4*i+i]; shl 2; +2, D 0x3B099)
   The by-id routines (0x3B728..0x3C5D4) address record i as dlg + 20*i + off+2; the offsets below are from the
   record's start.

   Button record (20 bytes):
     +0x00  int16  x, left (movsx word [rec], D 0x3B121)
     +0x02  int16  y, top ([rec] sar 16, D 0x3B117)
     +0x04  int16  w, width ([rec+2] sar 16, D 0x3B10E; the title's hit test x < rec.x + w, D 0x4F754)
     +0x06  int16  h, height ([rec+4] sar 16, D 0x3B105)
     +0x08  uint32 label: link address of the text; empty = no label (strlen at 0x3B129; set by 0x3C284, D)
     +0x0C  int8   label colour; 0 = the default colours with a shadow (D 0x3B192..0x3B255; set by 0x3BEA0)
     +0x0D  int8   1 = no press flash in f3C5D4 (D 0x3C652; meaning of the flag G; set by 0x3C1F4, read by 0x3C168)
     +0x0E  int16  button id (cmp ax,dx on word [dlg+20*i+0x10], D 0x3B771; the title reads it as [rec+0xC] sar 16)
     +0x10  int8   enabled: 1 = label in colour 0x43 with shadow 0x20, else 0x17 with shadow 3 (D 0x3B1A0,
                   0x3B22C); the title's hit test requires 1 (D 0x4F736); f3C314 sets 1, f3C3D0 sets 0
     +0x11  int8   style: 1 = 3-D frame by f3C9D0, 2 = no label (D 0x3B0FD, 0x3B14A)
     +0x12  int8   pressed: 0 up (frame flags 3), else down (flags 5, label 1 pixel right and down) (D 0x3B503..)
     +0x13  int8   font of the label (f39F90 index; D 0x3B15E) and index of the label offset tables below
   Signed bytes: every byte field is read as dword [rec+k-3] sar 24 (D). */
#define DLG_COUNT  0x00
#define DLG_REC(i) (2u + 20u * (uint32_t)(i))
#define BTN_SIZE   20
#define BTN_X      0x00
#define BTN_Y      0x02
#define BTN_W      0x04
#define BTN_H      0x06
#define BTN_LABEL  0x08
#define BTN_COLOR  0x0C
#define BTN_NOFLASH 0x0D
#define BTN_ID     0x0E
#define BTN_ENABLED 0x10
#define BTN_STYLE  0x11
#define BTN_PRESSED 0x12
#define BTN_FONT   0x13

#define UI_DIALOG_TITLE_TEMPLATE 0xC3090u  /* QUIT TO DOS 0x57, START A NEW GAME 0xAC, RESTORE A SAVED GAME 0x50 (D) */
#define UI_DIALOG_TITLE          0x1082E0u /* the working copy, 1002 bytes (D 0x4F1C4) */
#define UI_CUR_DIALOG            0xF8E78u  /* uint32: the dialog f3B05C drew last (D 0x3B0C5) */
#define UI_SAVED_PRESSED         0xF8E44u  /* int8[50]: pressed states saved by 0x3C828/0x3C940, restored by 0x3C7A0/0x3C8C4 (D) */

/* Fonts (D, 0x39E50 and 0x39F90):
     0xF8B04  uint32[3]  the FGF file data of fonts 0..2, heap blocks malloc'd at 0x39EA3 (pointers outside the data
                         object; the port keeps them in its own table, G)
     0xF8B2C  int16[3]   Fastgraph font number from fgf_define for fonts 0..2 (mov word [2*i+0xF8B2C], D 0x39F07);
                         0 = not defined ("Font %d is not loaded"). 0xF8B2A is not a separate variable: the code
                         reads this array as dword [2*i+0xF8B2A] sar 16 (D 0x39FA9), the word at 0xF8B2C+2*i.
     0xC1948  uint32[3]  file names: block05.fgf, cognac10.fgf, cognac19.fgf (then 0xFFFF)
   The current font (set by f39F90):
     0xF8AF8  int32      height = fgf_height("Mg", 2) (the string at 0xC1940; D 0x39FDD)
     0xF8AFC  int16      baseline = fgf_baseline() (D 0x39FF8)
     0xF8AFE  int16      current font index 0..2 (D 0x39FD7; returned by f3A02C)
     0xF8B00  int16      height - baseline: added to a text's top y to get the baseline (D 0x39FFE, 0x3A074)
   Text: line step = height + 2 (D 0x3A708). 0xC1944: a 1-byte string buffer for f3A748.
   Button label y offset per font (int16, D 0x3B548/0x3B55B): 0xC1954 + 2*font when up (-1, 0, 0),
   0xC1968 + 2*font when pressed (-2, 0, 0). */
#define UI_FONT_DATA   0xF8B04u
#define UI_FONT_NUM    0xF8B2Cu
#define UI_FONT_HEIGHT 0xF8AF8u
#define UI_FONT_BASE   0xF8AFCu
#define UI_FONT_CUR    0xF8AFEu
#define UI_FONT_ASCENT 0xF8B00u

/* Screens (D): [0xF52C8] = virtual-buffer handle of the back buffer (a full-screen copy that the dialogs draw into),
   [0xF52CC] = handle of the background-picture buffer, int8 [0xF52D0] = background now in it (f3CD6C),
   0xC197C = uint32[] picture names (0 "", 1 plainbck.gif .. 7 partha.gif .. 11 traddrw8.gif),
   [0xF522C] = the fg_imagebuf buffer (heap), int8 [0xF8E40] = 1 while the palette is faded to black (0x3CEB8 sets,
   f3CFE4 clears). Message-box area (f3A7B0): int16 x 0xBB3FA, y 0xBB3FC, w 0xBB3FE, h 0xBB400 (D 0x3A7CF..).
   Mouse visibility stack (D 0x7895C/0x789A4): int8[15] at 0x112A0C, top first. Mouse state from fg_mousepos:
   int32 x 0xE1628, y 0xE162C, buttons 0xE1630. */

/* ---- Text ----------------------------------------------------------------------------------------------------- */

void f39F90(int font);                 /* D: select font 0..2: fgf_select, sets 0xF8AF8..0xF8B00; prints "Font %d is not loaded" through f3A780 if its number is 0 */
int  f3A02C(void);                     /* D: the current font index (int16 at 0xF8AFE, sign-extended) */
void f3A044(int x, int y, const char *s);          /* D: print s with its top at (x, y): fg_move(x, y + [0xF8B00]), f3AD08; over 128 chars prints "String too long" instead */
void f3A0B0(int x, int y, int w, const char *s);   /* D: print s centred in [x, x+w): x + (w - fgf_width)/2 (signed /2), top at y; same length check */
void f3A18C(int x, int y, int w, int color, const char *s); /* D: title text: font 2, spacing 2, s centred (f3A0B0) 4 times in color+4 at (+1,-1),(+1,+1),(+1,0),(0,+1), 4 times in color+8 at (-1,-1),(0,-1),(-1,0),(-1,+1), once in color at (x,y); then font 1, spacing 0. s is the stack argument (ret 4) */
int  f3A42C(int x, int y, int w, uint32_t align, const char *text, int32_t *maxw, int draw);
                                       /* D: word-wrap text into lines of width w from (x, y); align (ecx, unsigned compares) 0 left, 1 right, 2 centred, other: no drawing; draw (stack 3) 0 = measure only; *maxw (stack 2, caller stack) = widest line + 1; returns the y under the last line, or -1 when y + baseline - 1 >= 480 at a line (D 0x3A622). Chars 0x14 (adds 2 to y) and 0x0D are skipped, bytes >= 0x80 are colour codes (f3AD08). Stack args: text, maxw, draw (ret 0xC) */
int  f3A748(uint8_t c);                /* D: width of the one character c in the current font (writes it to 0xC1944, fgf_width(0xC1944, 1)) */
void f3A780(const char *msg);          /* D: printf("%s", msg) (0x7D555 = printf, D) then getch (0x7D5A6: int 21h AH=8, D): a debug message that waits for a key */
void f3AD08(const char *s);            /* D: fgf_print s at the graphics cursor; a byte >= 0x80 sets the colour to byte & 0x7F (runs printed in pieces through a 200-byte stack buffer) */
int  f3AE3C(const char *s, int len);   /* D: 1 if any of the first len bytes of s is >= 0x80 (a colour code), else 0 */

/* ---- Dialogs and buttons ------------------------------------------------------------------------------------- */

void f3A7B0(uint32_t text);            /* D: message box: wraps text (f3A42C, width word[0xBB3FE]-10), centres it in the area at 0xBB3FA, draws fill 13, frames 4 and 14, text colour 5, on the active surface. Text is a data link address (all 29 callers, D) */
void f3B05C(uint32_t dlg);             /* D: draw every button of dlg (f3B4E0) into the back buffer [0xF52C8] (fg_vbopen/vbclose) and store dlg at 0xF8E78 */
void f3B0D4(uint32_t rec);             /* D: draw button rec up on the active surface (frame flags 3 if style 1; label at x, shadow at +1,+1) and set rec+0x12 = 0; mouse hidden around it */
void f3B2D8(uint32_t rec);             /* D: draw button rec pressed on the active surface (frame flags 5; label at +1,+1, shadow at +2,+2) and set rec+0x12 = 1 */
void f3B4E0(uint32_t rec);             /* D: draw button rec as its rec+0x12 state says (flags 3 or 5, label offset 0 or 1 plus the per-font table), on the active surface; mouse hidden around it */
void f3BC64(uint32_t dlg, int id, int y);   /* D: set the y (int16 at rec+2) of the button with this id (16-bit compare; the first match wins: the store at 0x3BCBA falls out of the loop at 0x3BCC0); "Button not found" box (f3A7B0) if none */
void f3C314(uint32_t dlg, int id);     /* D: enable the button id (rec+0x10 = 1) and redraw it in the back buffer [0xF52C8]; "Button not found" if none */
void f3C3D0(uint32_t dlg, int id);     /* D: disable the button id (rec+0x10 = 0) and redraw it in the back buffer */
void f3C5D4(uint32_t dlg, int id);     /* D: click feedback: unless rec+0x0D == 1, draw it pressed (f3B2D8), wait for the mouse button's release (f775F0), draw it up (f3B0D4) */
void f3C9D0(int x, int y, int w, int h, int flags); /* D: 3-D frame: flags&1 fills the inside (colour 13); &2 raised (light 14 top/left, dark 0x36 bottom/right), else &4 sunken (swapped). flags is the stack argument (ret 4) */
void f785D4(uint32_t dst, uint32_t src);   /* D: append src's button records to dst (20-byte copies after dst's last) and add src's count to dst's count */

/* ---- Screen, mouse, menu ------------------------------------------------------------------------------------- */

void f3CC14(void);                     /* D: show the back buffer: fg_vbopen([0xF52C8]), mouse hidden, fg_vbpaste(0, 639, 0, 479, 0, 479), mouse restored, fg_vbclose */
void f3CD6C(int8_t pic);               /* D: load background picture pic (name 0xC197C[pic]) into buffer [0xF52CC] with cat_show(name, 3, [0xF40D8] = ACIV.CT2) after fg_imagebuf([0xF522C], 0xFF00), unless it is already there (int8 [0xF52D0]) */
void f3CFE4(const char *gifname);      /* D: if int8 [0xF8E40] == 1 (faded out): fade in from black to gifname's palette in 64 steps (OVERVIEW §4), write the exact target from 0xF8B40, clear 0xF8E40; else nothing */
void f775F0(void);                     /* D: wait until no mouse button is down (fg_mousepos into 0xE1628/0xE162C/0xE1630) */
void f78884(void);                     /* D: hide the mouse pointer, pushing its state (f7895C, fg_mousevis(0)) */
void f788AC(void);                     /* D: restore the pointer state from before the matching f78884 (fg_mousevis(f789A4())) */
void f7895C(void);                     /* D: push the visibility stack at 0x112A0C (15 bytes, shift up), new top = 0 */
int  f789A4(void);                     /* D: pop the visibility stack; returns the new top (int8, sign-extended) */
int  f8240F(uint8_t *buffer, uint32_t size); /* D: Fastgraph/Image fg_imagebuf (cdecl): [0xD668A] = buffer, [0xD6690] = size; size 0 = the internal 4096 bytes at 0xD6694; returns 0. The buffer is heap memory (the game passes [0xF522C]), hence a C pointer */
int  f4FA54(int id);                   /* D: title button id -> menu code (16-bit unsigned compares): 0xAC -> -0x45 new game, 0x50 -> -0x46 restore, 0xAD -> -0x47 resume PBEM, 0xAE -> -0x5C new PBEM, 0xAF -> -0x5D host PBEM, 0x57 -> quit (f44CAC, [0xF5500] = ""), else -100 */
void f4F184(void);                     /* D: the title menu (partha.gif, "A D V A N C E D   C I V I L I Z A T I O N", the buttons, version); its caller 0x111A2 ignores eax: the outcome is bit 0x04 (new game) / 0x80 (PBEM) of the flags byte 0xE1624 */
_Noreturn void f44CAC(void);           /* D: quit to DOS: writes aciv.cfg (6 bytes from 0xF097C) if [0xF40E8] == 1, fg_mousefin, fg_setvpage(0), fg_setmode([0xF6058]) (the start-up mode, G), prints [0xF5500] + "\n", AIL_shutdown, 0x78508, exit(0) (0x7D578) */

/* ---- Port entry points ---------------------------------------------------------------------------------------- */

int title_boot(const char *gamedir);   /* the part of start-up (0x10010) the menu needs: state_init, the catalogs (0x1086F..: [0xF40D4] CT1, [0xF40D8] CT2, ...), the video mode and mouse, the back buffer [0xF52C8], the imagebuf [0xF522C], the three fonts (0x39E50 at 0x10B11..0x10B2A), the background buffer [0xF52CC], the fade to black (0x3CEB8 at 0x11022) (G: the list of steps; addresses D). 0 ok */
int title_menu(void);                  /* 0x4F184: runs f4F184 and returns the code that ended it (G, port design): -0x45 new game, -0x46 restore, -0x47 / -0x5C PBEM resume / new (D: the codes and the flag updates at 0x4F7C6..0x4FA41); QUIT exits through f44CAC */

#endif
