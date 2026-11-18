/* Save and restore dialogs of AC_MAIN.EXE (module S1 of docs/MAP_PLAN.md, the dialog half): 0x5FF28 (RESTORE GAME),
   0x5E7D0 (SAVE GAME), 0x601D8 (the slot dialog), 0x60694 (one-line text input), 0x6016C (save-version check) and
   0x3CD10 (screen grab into the background buffer), plus the port's title_restore (main 0x11557..0x115A9).
   Translations in save.c; the loading half (0x5F05C, 0x7BBF0, 0x7BB84, 0x7BCD4, 0x726DC) is declared in saveload.h. */

#ifndef SAVE_H
#define SAVE_H

#include <stdint.h>

int  f5FF28(void);                     /* D: RESTORE GAME (title 0x1159B, dispatcher 0x3FEAA): f3CD10, [0xF52D0] = 0, slot = f601D8(2, desc), -15 = cancel; loads savgame%x.dat (f5F05C), [0xF40EC] = humans + computers, PBEM name 0x109E84 -> 0xEE20C, f726DC in phase 14, CRC (f7BBF0, version >= 0x1E) and version (f6016C) checks; unless slot 10 copies the file over savgamea.dat; the "Restoring" screen (background 8). Returns the slot or -15 */
void f5E7D0(void);                     /* D: SAVE GAME (dispatcher 0x3FDBA, eax unused): f3CD10, [0xF52D0] = 0, slot = f601D8(1, desc); unless -15 copies savgamea.dat to savgame<slot>.dat and patches its header ("ADVANCED CIVILIZATION" from version 0x28, then the 51-byte description); "Save failed..." (f3A9F4) if the copy fails; f3CC68, f3CCBC */
int  f601D8(int mode, char *desc);     /* D: the slot dialog: mode 1 save (10 slots, box 349 high), 2 restore (12 slots incl. 'a'/'b', box 409 high); dialog 0x10A174 from the templates 0xC6628 (+ 0xC6A12 in mode 2), slot descriptions char[12][61] at 0x109E98; save mode edits the description (f60694); copies it to desc; returns the slot (0..11) or -15 */
int  f60694(int x, int y, int w, int maxlen, char *buf, uint32_t align, uint32_t dlg);
                                       /* D: one-line text input (the last three on the stack, ret 0xC): starts as Ctrl-X (clear); Enter ends, Esc -15, Backspace, 0x20..0x7E while len < maxlen and the text fits w; draws buf + "_" in colour 15 on fill 13, align 0 left, 1 right, 2 centred; a click on an enabled button of dlg goes through f3C5D4 + f3FB94 (-15 ends). Returns -100 or the last button code on Enter, -15 */
int  f6016C(int version);              /* D: save-version check: with PBEM (0xE1624 & 0x80) or int16 [0xF40C2] != 0 versions below 0x2C, otherwise below 0x35, are rejected ("That save game is an earlier format...", f3A7B0, getch) -> 0; else 1 */
void f3CD10(void);                     /* D: grab the screen into the background buffer [0xF52CC] (as f3CCBC does for the back buffer), then int8 [0xF52D0] = 0 */

int  title_restore(void);              /* port: main 0x11557..0x115A9, the title menu's RESTORE (code -0x46): background 8 shown, f5FF28; 1 = restored ([0xE1624] &= ~4, main goes on at 0x115B0), 0 = cancelled (back to the title menu, 0x11168) */

#endif
