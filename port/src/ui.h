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

/* ---- Sprites ---------------------------------------------------------------------------------------------------

   Sprite record (8 bytes in the data object, passed as its link address; made by 0x3AE90(spr, x, y, w, h), which
   captures the w x h block at top-left (x, y) of the active surface with fg_getimage and keeps it with fg_unpack):
     +0x00  uint32  the pixel block, a heap block from 0x822B5 (tagged_alloc?, 0x3AEB3); outside the data object, so
                    the port keeps these blocks in its own table, as with the font data (G)
     +0x04  uint16  w (mov word [spr+4], D 0x3AEEF; read as word & 0xFFFF, D 0x3AFD3)
     +0x06  uint16  h (D 0x3AEF9, 0x3AFC6)
   The start-up routine 0x118F8 (called at 0x10B3E) shows maped.gif and captures 0xF51AC, 0xF51B4, 0xF51D4, 0xF51CC,
   0xF51E4, 0xF51DC, ... from it (D 0x1192C..0x119D6). The setup screens use two 14 x 14 of them:
     0xF51CC  captured at (0x40, 0x3A): drawn when a flag is set (the ticked box, G: the look)
     0xF51D4  captured at (0x40, 0x4C): drawn when a flag is clear (the empty box, G)
   title_boot does not run 0x118F8 yet; the setup screens need at least these two. */
#define UI_SPR_DATA 0x00
#define UI_SPR_W    0x04
#define UI_SPR_H    0x06
#define UI_SPR_CHECK_ON  0xF51CCu
#define UI_SPR_CHECK_OFF 0xF51D4u

void f3AE90(uint32_t spr, int x, int y, int w, int h);   /* D: cut sprite spr (w x h, top-left at x,y) from the active surface: block via ui_sprite_put, [spr+4]=w, [spr+6]=h, fg_getimage (fg_unpack is a copy in mode 25) */
void f60A80(void);                     /* D: initializer (XI 0xE15EA): running sums of 0xC7618[7] into 0x10F2F4.. */
void f7BB14(void);                     /* D: CRC-32 table (0xEDB88320) into 0x11260C[256] (start-up 0x10577) */
void f118F8(void);                     /* D: start-up 0x10B3E: maped.gif (CT2) into Page1, cut into the 12 sprites 0xF51AC..0xF5204 */
void f3AF8C(uint32_t spr, int x, int y);   /* D: draw sprite spr with its top-left at (x, y): fg_move(x, y + h - 1), fg_clpimage(data, w, h) (colour 0 transparent), on the active surface. Void: eax is fg_clpimage's result and no caller reads it (all 233 call sites scanned; a few are tail calls in small wrappers such as the one ending at 0x3E9DB, whose own callers were not checked, G) */

/* ---- Dialog helpers by button id (the search of f3C314: 16-bit compare of the id, the first match wins) -------- */

int  f3BF30(uint32_t dlg, int id);     /* D: the enabled byte (rec+0x10, sign-extended: dword [dlg+20*i+0xF] sar 24, 0x3BFB0) of the button id; if none, the "Button not found" box (0xB3D02, f3A7B0) and 0 */
void f3BFBC(uint32_t dlg, int id, int enabled); /* D: rec+0x10 = (int8)enabled (bl, 0x3C043) of the button id, without redrawing (unlike f3C314/f3C3D0); if none, the "Button not found" box (0xB3D13) */

/* ---- New-game setup ("New Game Options", 0x4DD50) and "Variants" (0x4EAE0) -------------------------------------

   Setup data (D: the reads and writes in 0x4DD50..0x4F183 and main 0x111EB..0x113C4):
     0xE1620  int8   human players (read as dword [0xE161D] sar 24). Buttons 0x5F '-' (only if > 0 and the total
                     > 3, 0x4E582) and 0x60 '+' (only if < 8 and the total < 8, 0x4EA6B)
     0xE1621  int8   computer players (dword [0xE161E] sar 24). Buttons 0x61 '-' (> 0, total > 3, 0x4E5C1) and
                     0x62 '+' (< 8, total < 8, 0x4E692). With 0 computers the AI-level buttons 0x67/0x68 are
                     disabled and the AI-level text is drawn in colour 7 (else 0xF)
     0xE1622  -      not touched by these screens
     0xE1623  uint8  low nibble: "# of Trading Sessions" 1..4 (buttons 0x63 '-' if > 1, 0x64 '+' if < 4);
                     high nibble: "Minimum Goods Before Trading Allowed" 3..7 (0x65 '-' if > 3, 0x66 '+' if < 7).
                     Defaults in main: 2 (0x11315) and 6 (0x11344), or the settings at 0xF5888 / 0xF588C
     0xE1624  uint8  flags: 0x01 cleared by START GAME (0x4E29D; meaning G); 0x02 set by START GAME when the total
                     > 3, else cleared (0x4E2AC; G: the larger board); 0x04 new game (title, OVERVIEW §5); 0x08
                     cleared by main 0x113C4 (G); 0x10 variant "Allow more than 2 calamities per victim"; 0x20
                     variant "Allow purchase from any stack"; 0x40 variant "Draw 2 goods per city" (forced off with
                     more than 4 players: 0x4E290, 0x4EB7C); 0x80 play-by-email
     0xE1625  uint8  bits 2..3 (word [0xE1624] bits 10..11): computer AI level 0 "Stronger", 1 "Normal" (main's
                     default, 0x112DA), 2 "Weaker"; names at 0xC4040 (uint32[3]: 0xB4BC5, 0xB4BDA, 0xB4BED), drawn
                     at (0x7D, 0xE1). Button 0x67 '-' raises the index (weaker, if < 2, 0x4E825), 0x68 '+' lowers it
                     (stronger, if > 0, 0x4E764); both repaint (0x7D..0x275, 0xE1..0xF7) from [0xF52CC] first.
                     0x10 variant "Another trade round needs only 2 players" (offered only with PBEM: cleared at
                     0x4EBB3 otherwise, and by main 0x113C4); 0x20 variant "Real Goods in trade are known"; 0x02 cleared by
                     main 0x113C4 (word &= 0xEDF7, G)
     0xF40EC, 0xF40F0  int32  both = humans + computers, written by START GAME (0x4E281) and main 0x112B5
     0xF5884, 0xF5888, 0xF588C, 0xF5890, 0xF5894, 0xF5898, 0xF589C  int32  settings main turns into the defaults
                     above (AI level = [0xF5884] - 10, sessions, minimum goods, then the four variant flags 0x10,
                     0x20, 0x40 and 0xE1625 0x20; 0 = the built-in default / off, D 0x112C7..0x113BD). Where they
                     are written: no direct store found (G: a settings table)
   Setup dialog 0xC28B8 (14 records, D obj2): 0x59 the PBEM row (88,152,200,14, style 2), 0x5F/0x60, 0x61/0x62,
   0x63/0x64, 0x65/0x66, 0x67/0x68 the '-'/'+' pairs (12 x 11), 0x4E CANCEL (48,420), 0x2A START GAME (246,420),
   0x5B VARIANTS (445,420). Numbers in sunken boxes f3C9D0(0x56, y, 0x12, 0x13, 5) at y 0x63, 0x7C, 0xAE, 0xC7,
   colour 5 (the computers' count 0x17 when button 0x62 is disabled, 0x4E072).
   Variants dialog 0xC2CA4 (6 records): rows 0x5A (y 102), 0x5C (127), 0x5D (152), 0x104 (175), 0x105 (200), each
   (88, y, 500, 14) style 2, with the box sprite at (0x58, y) and the text at (0x7D, y - 2); 0x2A CONTINUE
   (245,420). The rows 0x5C, 0x5D, 0x104 have their text in colour 7 when
   the button is disabled, else 0xF; 0x5A and 0x105 always 0xF (0x4EBDF..0x4ECC2).
   Play-by-email (dropped from the port): the PBEM row of the setup screen (label "Play-by-email" 0x4DE96, box
   0x4DEDF, button 0x59 -> 0x4E355..0x4E581, which disables 0x61, 0x62, 0x67, 0x68 and the variant 0x5C, sets
   3 humans if fewer and 0 computers), and the variant 0x104 that only PBEM enables (0x4EB99..0x4EBCB). */
#define UI_DIALOG_SETUP    0xC28B8u
#define UI_DIALOG_VARIANTS 0xC2CA4u
#define UI_SETUP_HUMANS    0xE1620u
#define UI_SETUP_COMPUTERS 0xE1621u
#define UI_SETUP_TRADE     0xE1623u
#define UI_SETUP_FLAGS     0xE1624u
#define UI_SETUP_FLAGS2    0xE1625u
#define UI_SETUP_AI_NAMES  0xC4040u
#define UI_PLAYERS_A       0xF40ECu
#define UI_PLAYERS_B       0xF40F0u

int  f4DD50(void);                     /* D: the setup screen (f3CD6C(7) = partha.gif, title "New Game Options"); loops until f4E2D8 returns other than -100: -1 START GAME (then [0xF40EC] = [0xF40F0] = total and the flag updates 0x4E26E..0x4E2C3), -0xF CANCEL, -0x82 VARIANTS. Its caller sign-extends ax (cwde, 0x113DC) */
int  f4E2D8(int id);                   /* D: one click on the setup dialog's button id (16-bit unsigned compares): the counter and AI-level changes listed above; returns -1 for START GAME (0x2A), -0xF for CANCEL (0x4E), -0x82 for VARIANTS (0x5B), else -100 */
void f4EAE0(void);                     /* D: the variants screen (f3CD6C(7), title "Variants"): enables/disables 0x5D (total <= 4) and 0x104 (PBEM) with f3BFBC, draws the rows, then loops until f4EEC8 returns -1 (CONTINUE). Its caller 0x113F7 ignores eax */
int  f4EEC8(int id);                   /* D: one click on the variants dialog: 0x5A, 0x5C, 0x5D, 0x104, 0x105 toggle their flag and redraw the box sprite (inside fg_vbopen/vbclose of the back buffer); returns -1 for CONTINUE (0x2A), else -100 */

void title_menu_again(void);           /* main 0x11168..0x111A2: what main re-does before showing the menu again */
int ui_sprite_put(uint32_t spr, uint8_t *pixels);   /* port: keep the pixels of sprite record spr (its +0 dword holds a
                                                       handle, as a 64-bit pointer does not fit); 0 ok, -1 table full */
uint8_t *ui_sprite_pixels(uint32_t spr);            /* port: the pixel block of sprite record spr, or NULL */
int title_newgame(void);               /* main 0x111DC..0x113FE: the new-game defaults and the loop 0x113CD (0x4DD50, 0x4EAE0), without PBEM: returns 1 when the player started a game, 0 when they went back to the title menu (0x11168) */

#endif
