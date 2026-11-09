# Negotiation dialogue logic (EXE 0x5F82 from 0x602E, helpers, DS tables)

Claude, 2026-11-09. Scope: the dialogue loop of `enter_foreign` 0x5F82 (0x602E..0x64FE) with the helpers it calls, every DS table
it reads (dumped from the static DS = OVL0.OVL file 0xB400), the TEXTx.LZW format, the Cleopatra branch, the dice order, and a
check of the first pass in LEGIONS_NOTES.md ("5F82 enter_foreign"). The screen set-up 0x6570 (portrait, frame, scout report 7B56)
is only summarised where the dialogue depends on it.

Marks: **D** decoded from the disassembly (fresh `llvm-objdump` of EXE_RUNTIME.bin, scratch `wf/legions/d.sh`), **V** verified by
running a script against the game data (static DS / MNEGO archive; no game run, no DOSBox), **G** guess.

## 0. Conventions

- Every helper here that ends in `ret N` is **Pascal**: arguments pushed left to right, so the first pushed is the first
  parameter. I write calls in push order: `5AE4(text, colour, prefix)` = `push text; push colour; push prefix`. (The C-runtime
  helpers 9DF2 strcat, 9E32 strcpy, 9EEE strncpy, 9FF8 strchr, A0A8 strtok are cdecl, written in C order.) **D**
- 5F82 is itself Pascal (`ret 4`): pushed `z` then `L`, so `[bp+6]` = z (zone/province 1..26), `[bp+4]` = L (legion record
  index). The first pass's name `enter_foreign(L, z)` is kept.
- `P` = province record `0x5270 + 32*z`; `rec` = legion record `0x1918 + 20*L`.
- Locals of 5F82 (bp-relative): `-0x22` z (word), `-0x20` z*32, `-0x1E..-0x17` `buf` (the text file: +0 offset, +2 segment),
  `-0x16` `A` (word, 455A[z]), `-0x14` `Bt` (word, 42EC[z]), `-0x12..-0x10` `code[3]`, `-0x0E` saved si, `-0x0C` `result`
  (word), `-0x0A` `state` (byte), `-9` `weak` (byte), `-8` `c` (byte), `-7` `mood` (byte), `-6` `sel` (word), `-3` `code`
  (byte), `-2` item pointer (word, only in the menu-building loop), `-1` `x` = reply kind (byte; overlaps the high byte of the
  item pointer, rewritten before use). Register `si` is the loop's "end" flag. **D**
- Timer: ISR at 72.8 Hz (BATTLE_NOTES). `0x48` = 72 ticks = 0.99 s, `0x90` = 144 = 1.98 s, `0x0E` = 0.19 s, `9` = 0.12 s.

## 1. The dialogue loop, literal (0x602E..0x64FE) - D

```c
/* 0x602E: (the rebel branch 0x5F82..0x602B is in LEGIONS_NOTES and already ported) */
cursor_push_1D55(0);
if (!nego_screen_6570(z, &buf)) { cursor_pop_1D8A(); return 0; }   /* 6570 = Pascal (z, &buf); 0 = MNEGO could not be opened */
/* 0x604B */
si = 0; result = 0; state = 0; weak = 0;
mood = (uint8_t)B(0x634E) / 34;                       /* 8-bit div: 0..33 -> 0, 34..67 -> 1, 68..101 -> 2 (102+ -> 3.., see 9.1) */
A  = W(0x455A + 2*z);                                  /* reply -> next state table (section 5.6) */
Bt = W(0x42EC + 2*z);                                  /* state -> 3 offered reply codes (section 5.5) */
c  = B(0x4322 + z);                                    /* row width of A */
if ((int8_t)B(P+0x0E) > 75 && mood != 0) mood--;      /* signed compare (jle) */
if ((uint16_t)(W(rec+6) + W(rec+8)) < (uint16_t)(W(P+0) + W(P+2)))   /* legion inf+cav < province troops, unsigned */
    weak = c;
if (!(B(0x4092 + z) > B(0x5971)) && !(B(0x40AE + z) > B(0x634E)))   /* rank >= min rank AND [634E] >= min, unsigned */
    state = 1;
/* 0x64D9 */
while (si == 0) {
  /* 0x60D8 */
  saved_si = si;                                       /* [bp-0xE] = 0 */
  text_4B8(&buf, state, 0xAC60);                       /* copy text #state to DS:AC60 */
  say_5AE4(variant_6501(0xAC60), 15, '"');             /* DICE: roll(3). province speaks, white, in quotes */
  B(0x408E) = 3;                                       /* menu 4080 item count */
  for (di = 0, p = 0x405C; p < 0x4080; p += 12, di++) {
    code = B(Bt + state*3 + di);  code_[di] = code;
    if (code == 0x3D) B(0x408E) = 2;                   /* 0x3D = hidden third item */
    k = B(0x40E6 + code);
    setW(p, W(0x4028 + 2*k));                          /* item text = reply name */
    B(p + 2) = B(0x404A + k);                          /* item hotkey */
  }
  /* 0x6159 draw the menu */
  target_back_67E4();                                  /* draw into the work buffer (C0D0 := C1DA), Tandy: page reg */
  W(0x9AFA) = W(0x6346);  fill_7E74(10, 151, 103, 197);          /* clear the inside of the menu frame */
  for (si = 0; si < 0x38; si += 14)                    /* tab sheet images 4..7 = the menu frame */
    sprite_8E62(W(0xC1EE) + 0x38 + si, W(0x4080), W(0x4082));   /* at (8, 151) */
  menu_draw_5614(0x4080, 0);
  target_screen_680F();
  restore_6973(8, 151, 110, 198);                      /* work buffer -> screen */
  do { sel = 0; sel = menu_run_54C8(0x4080, 0); } while (sel == 0xFF);   /* Esc / outside click ignored */
  /* 0x61CA */
  si = sel;  code = code_[sel];  x = B(0x40E6 + code);
  say_5AE4(W(0x4C92 + 2*code), 9, '"');                /* the player's line, colour 9, in quotes */
  if (x <= 1) goto leave_647E;                         /* unsigned: 0 Withdraw, 1 Go to War */
  /* 0x61F4 */
  delay_143(0x48);
  if (x >= 14)                                         /* Low / Medium / High tribute */
    state = (B(0x40CA + z) > (uint8_t)(x - 14)) ? 2 : 3;
  else
    state = B(A + (uint16_t)(c * mood) * 2 + (uint8_t)(B(0x4124 + code) + weak));   /* 8-bit add, then 16-bit */
  if (state == 2) {
    /* 0x623C: alliance */
    B(P+0x12) = B(P+0x13) = (uint8_t)sel + 1;          /* tribute level = index of the Low/Medium/High item + 1 */
    B(P+0x15) = 2;                                     /* status Ally */
    setW(0xC106, W(0xC106) + 1);                       /* statistics: alliances */
    B(P+0x19) = 0;
    setW(P+4, W(P+4) + W(P+2));  setW(P+4, W(P+4) + W(P+0));   /* 16-bit wrap */
    if (W(P+6) < W(P+4)) setW(P+4, W(P+6));            /* the province's troops join its manpower pool, capped */
    text_4B8(&buf, state, 0xAC60);
    say_5AE4(variant_6501(0xAC60), 15, '"');           /* DICE: roll(3) */
    if (z != 2) {
      blank_lines_5AC2(1);
      click_to_continue_7CB2();
      free_text_198F(&buf);
      si = 0xFFFF;  result = 0xFFFF;
      goto check_6348;                                 /* -> 0x64D9 -> exit */
    }
    /* 0x62C2: Aegyptus - Cleopatra */
    blank_lines_5AC2(1);
    click_to_continue_7CB2();
    music_play_5D29(8);                                /* "cn6" from archive 7 (MNEGO CN6.LZW) */
    sprite_8E62(W(0xC1EC) + 0x1C, 0, 0);               /* love2[2]: the barge on the Nile, 320x63, top band */
    W(0x9AFA) = W(0x6344);  fill_7E74(8, 151, 107, 198);         /* erase the menu area (panel colour) */
    say_5AE4(0x4DD4, 0, 0);                            /* narration, colour 0, no quotes: the five-day feast */
    blank_lines_5AC2(1);
    say_5AE4(0x4E28, 0, 0);                            /* the bedchamber on her barge */
    blank_lines_5AC2(1);
    click_to_continue_7CB2();
    sprite_8E62(W(0xC1EC) + 0x0E, 0, 0);               /* love2[1]: Cleopatra on her couch, 319x63 */
    sprite_8E62(W(0xC1EC), 8, 0x4F);                   /* love2[0]: Cleopatra close-up over the leader portrait */
    state = 0x0F;
  }
  /* 0x6345 */
  si = saved_si;                                       /* = 0 */
check_6348:
  if (z == 2) switch (state) {                         /* computed as state - 0x11 */
    case 0x11: case 0x12: case 0x13:                   /* 0x636C: she calls the guards */
      text_4B8(&buf, state, 0xAC60);
      say_5AE4(variant_6501(0xAC60), 15, '"');         /* DICE: roll(3) */
      blank_lines_5AC2(1);
      delay_143(0x90);
      say_5AE4(0x4D0C, 0, 0);                          /* "...but the alliance remains intact." */
      click_to_continue_7CB2();
      goto end_6458;
    case 0x15:                                         /* 0x63B0: "Then I offer you this gift..." accepted */
      blank_lines_5AC2(2);
      W(0x9AFA) = W(0x6344);  fill_7E74(8, 151, 107, 198);
      love_anim_4852();                                /* bedchamber animation, ~6.4 s, section 2.6 */
      delay_143(0x90);
      W(0x9AFA) = W(0x6344);  fill_7E74(8, 79, 105, 146);        /* erase the portrait area */
      sprite_8E62(W(0xC1EC) + 0x1C, 0, 0);             /* love2[2]: the barge again */
      W(0x9B69) = 4;  text_centred_8BE4(W(0x3E40), 212, 184);    /* "Click to continue" colour 4 (y 184; 7CB2 uses 183) */
      save_691B(0, 0, 319, 199);                       /* whole screen -> work buffer */
      mouse_poll_644();
      goto flash_644B;                                 /* one flash at once */
      for (;;) {                                       /* 0x642A */
        if (key_3AA()) break;                          /* any key (al != 0) */
        mouse_poll_644();
        if (elapsed_1AE() > 0x48) {                    /* timer 0 */
          timer_start_F8();
          if (roll(12) < 2)                            /* DICE: roll(12), only once per > 72 ticks */
flash_644B: flash_654B();                              /* love2[3] over the barge for 9 ticks */
        }
        if (W(0x00B2) != 0) break;                     /* mouse button */
      }
      /* 0x6455 */
      page_6837();
      goto end_6458;
    case 0x16:                                         /* "My queen, I must go." */
end_6458:
      free_text_198F(&buf);
      free_sheet_19A0(W(0xC10E));                      /* love1 */
      free_sheet_19A0(W(0xC1EC));                      /* love2 */
      si = 0xFFFF;  result = 0xFFFF;                   /* counts as a won province, see 3 */
      music_stop_5D20();
      music_unload_5E1A(8);
      break;
    default: break;                                    /* 0x0F, 0x10, 0x14 and all states < 0x11: talk on */
  }
  continue;                                            /* 0x64D9: loop while si == 0 */

leave_647E:
  free_text_198F(&buf);
  if (z == 2) { free_sheet_19A0(W(0xC10E)); free_sheet_19A0(W(0xC1EC)); }
  si = 0xFFFF;
  if (x == 0) { delay_143(0x48); continue; }          /* Withdraw: result stays 0 */
  disk_check_1E21();                                   /* Go to War */
  result = battle_B06E(z, L, 0xFF);                    /* push z, L, 0xFF; army 0xFF = the province's own troops */
  if (result) { B(P+0x15) = 1; B(P+0x19) = 0; B(P+0x12) = B(0x5FC0); }   /* Occupied, tribute policy */
}
/* 0x64E0 */
if (result) province_won_53F8(z);
cursor_pop_1D8A();
free_sheet_19A0(W(0xC1EE));                            /* tab */
return result;
```
Notes on the flow (D):
- `result` is the return value: 0 = Withdraw / battle lost; 0xFFFF = battle won, alliance (any province), or any Cleopatra
  ending. Every nonzero result runs 53F8 here, and the caller 382A then scores it (+30 Aegyptus / +15 Carthago / +5, LEGIONS_NOTES)
  and moves the legion in: **an alliance scores exactly like a conquest.**
- The Aegyptus alliance itself does not set `result`; it is set by whichever Cleopatra ending follows (all of them set it).
  No Cleopatra state offers Withdraw or Go to War, so every alliance with Aegyptus ends in 6458 with result 0xFFFF.
- The text buffer is kept across the loop and freed exactly once on every exit. "tab" ([C1EE]) is freed at 0x64EF only
  (not on the 6570-failure exit). love1/love2 ([C10E]/[C1EC]) are freed on Withdraw/War (Aegyptus) or at 6458.
- Withdraw: player line, then (no 0x48 delay before) free, then `delay_143(0x48)`. Go to War: player line, free, 1E21, battle,
  no delay. Every other reply: `delay_143(0x48)` before the province's next line.
- The B(0x408E) count is reset to 3 for every menu and dropped to 2 when any of the three codes is 0x3D. In all reachable rows
  0x3D is only ever the third code (V, section 6).

## 2. Helpers - D

### 2.1 04B8 text_4B8(&buf, n, dst)   (Pascal, `ret 6`, asm)
```c
src = MK_FP(W(buf+2), W(buf+0));              /* the loaded TEXTx.LZW (unpacked) */
off0 = ((uint16_t far *)src)[n]; off1 = ((uint16_t far *)src)[n+1];
memcpy(DS:dst, src + off0, (uint16_t)(off1 - off0));  /* rep movsb; copies the text's own trailing NUL */
```
No bounds check (see 9.1). TEXTx.LZW unpacked = `u16 off[count+1]` (off[0] = 2*(count+1)) then NUL-terminated texts; off[count]
= file length (V: all 26 files, every text ends with its NUL, no tail bytes).

### 2.2 6501 variant_6501(s)   (Pascal)
```c
n = (uint8_t)(roll(3) + 1);                  /* DICE: roll(3) -> 1..3 */
for (i = 0; i < n; i++) {
  last = s;
  t = strchr(s, '~');
  if (t) { *t = 0; s = t + 1; }
}
return last;
```
Texts hold 1..3 variants separated by '~' (V: 50 texts have 3 variants, 4 have 2, the rest 1). With 2 variants the second one is
picked with probability 2/3 (roll 1 and 2 both end on it). The roll happens even for a single-variant text. 6501 = "0x651B /
0x653A / 0x653B" in the clean listing (labels inside this one function); 654B is a different routine (2.7).

### 2.3 5AE4 say(text, colour, prefix)   (Pascal) - the scrolling text log
```c
char buf[0x102+], line[0x42];  int done = 0, w = 0;
di = (W(0x9B5C) & 0xFF) + B(0x9B5D) - 1;      /* line step: 8 + 3 - 1 = 10 at run time */
W(0x9B69) = colour;                           /* stays set after return */
setW(0x3E76, di + 0x56);                      /* scroll source rect 3E74 = {120, 86+di, 305, 190} */
if (prefix) { buf[0] = prefix; B(0x3E72) = prefix;            /* 3E72 is a 1-char string (static ' ', then '"') */
              strncpy(buf+1, text, 0xFE); strcat(buf, (char*)0x3E72); }   /* -> "text" */
else          strncpy(buf, text, 0x100);
s = strtok(buf, " ");  line[0] = 0;           /* 3E6C/3E6E/3E70 are all " " */
while (!done) {
  if (s == NULL || width_8C58(s) + w > 0xBA) {            /* flush the line (186 px) */
    if (W(0xC1D8) + di > 0xBE) {                          /* past y 190: scroll the log up by di */
      copy_950E(C0D0, C0D0, 0x3E74, 120, 86);             /* rect (120,96)-(305,190) -> (120,86), current target */
      setW(0xC1D8, W(0xC1D8) - di);
      W(0x9AFA) = W(0x6346); fill_7E74(120, W(0xC1D8), 305, W(0xC1D8) + di);
    }
    text_899C(line, 120, W(0xC1D8));                      /* left-aligned at x 120 */
    setW(0xC1D8, W(0xC1D8) + di);
    line[0] = 0;
  }
  if (s == NULL) { done = 0xFFFF; continue; }
  strcat(line, s); strcat(line, " ");
  w = width_8C58(line);
  s = strtok(NULL, " ");
}
```
- The log is the frame drawn from "tab" images 0..3 at (113,79)..(312,198); text x 120, lines every 10 px from [C1D8]. [C1D8] =
  0x56 (86) is set by the scout report 7B56 inside 6570 (the only other writer of C1D8 besides 5AE4).
- Only ' ' delimits words, so runs of spaces ("Roman.  The") collapse to one; there are no '\n' in the texts (V).
- `width_8C58(NULL)` is evaluated once at the end (reads DS:0); its value is unused. A port can skip it.
- 5AC2(n) (Pascal): `for (i = 0; i < n; i++) say_5AE4(0x3E02 /* "" */, 0, 0);` = n blank lines (each may scroll); it sets
  [9B69] = 0.

### 2.4 7CB2 click_to_continue
`W(0x9B69) = 4; 8BE4(W(0x3E40), 212, 183); 35F9() /* release, click or key, release */; W(0x9B69) = W(0x6348);
8BE4(W(0x3E40), 212, 183);` - draws [3E40] ("Click to continue", or "Hit any key to continue" without a mouse, see LEGIONS_NOTES
correction 1) centred at x 212 in colour 4, waits, then draws it again in [6348] (VGA: 0x10C) to remove it. G: the low byte 12
is the colour that 8BE4 uses; the erase colour should be checked against a capture.

### 2.5 0143 delay(n), 00F8, 0183/01AE, 0644, 03AA
- `F8()` = `D7(0)`: timer 0 start = tick counter [42]/[44]. `1AE()` = `183(0)`: ticks since timer 0 (0xFFFF if > 65535).
- `delay_143(n)`: `F8(); target = start + n; while (now < target) { 644(); if (W(0xB2)) break; if (3603()) break; }` - it ends
  early on a mouse button or a key (3603 sets [B2] for Enter/Esc etc. in keyboard mode; its mouse-mode part 36E4 not decoded).
  **The delays are interruptible: a click skips them.** It also restarts timer 0.
- `644()`: `W(0xB2) = W(0xBA); if (W(0xBA)) { int 33h fn 3: [B2] = buttons, [B6] = x, [B8] = y; }`.
- `3AA()`: BIOS key check; returns al = 0 when no key, else reads the key (al = ASCII, mapped for a few scan codes).

### 2.6 4852 love_anim (Cleopatra's bedchamber), 48EF step
```c
if (W(0xC10E) == 0) return;                   /* love1 not loaded (non-Aegyptus) */
target_back_67E4();
sprite_8E62(W(0xC10E) + 0x00, 0, 0);         /* love1[0] 224x66: hieroglyph cartouche + the couch */
sprite_8E62(W(0xC10E) + 0x0E, 0xE0, 0);      /* love1[1] 96x66: right panel (door, second cartouche) */
sprite_8E62(W(0xC10E) + 0x1C, B(0x3C26), 0); /* love1[2] 44x63: the Roman in the doorway at x 229 */
target_screen_680F();  restore_6973(0, 0, 319, 63);
delay_143(0x48);
for (si = 0, di = 0; si < 0xB6; si += 14, di++) {    /* 13 frames */
  timer_start_F8();
  sprite_8E62(W(0xC10E) + 0x2A + si, 0x55, 3);        /* love1[3..15] straight onto the screen */
  step_48EF(di);
}
for (si = 13; si < 28; si++) { timer_start_F8(); step_48EF(si); }
/* 48EF(i): */
if (B(0x3C26 + i)) { target_back_67E4(); sprite_8E62(W(0xC10E)+0x0E, 0xE0, 0);
                     sprite_8E62(W(0xC10E)+0x1C, B(0x3C26+i), 0); target_screen_680F(); restore_6973(0xE0, 0, 319, 63); }
while (elapsed_1AE() < 0x0E) ;                /* 14 ticks per step, NOT interruptible */
```
3C26 (28 bytes) = x of love1[2] per step: `229,0,0,0,0,0,230,231,233,0,0,0,0,234,235,236,0,0,0,0,0,0,238,240,242,243,0,244`
(0 = do not move). Total: 72 + 28*14 = 464 ticks = 6.4 s; then 5F82 waits another 0x90. Sprite positions use the sheet's own
hot spots (love1[3..15] have hot x 0..-49). G (from greyscale renders, palette not applied): frames 3..15 show the Roman
crossing the room to the couch while the figure in the right-hand doorway slides right.

### 2.7 654B flash
`sprite_8E62(W(0xC1EC) + 0x2A, 0, 0); delay_143(9); restore_6973(0, 0, 319, 63);` - love2[3] (238x23, hot (-51,-17), so at
(51,17)) over the barge for 9 ticks (cut short by a click, which then also ends the loop), then the barge from the 691B backup.
G: in greyscale it adds bright jagged shapes around the cabin and in the sky (lightning or fireworks?); check with the palette.

### 2.8 Music: 5D29 play(n), 5D73 load(n), 5D20 stop, 5E1A unload(n)
Slots `A6B0 + 4*k`, k < 4: `{u16 ptr, u8 used, u8 id}`.
- 5D73(n): if no used slot has id n: take the first unused slot, `ptr = 5EB5(n)` (load `name[n] + ".lzw"` from archive
  `arch[n]`, table 3EF0 `{u16 name, u16 archive}`; returns 0 when [3EEC] (sound on) == 0 and n < 0x11), and if ptr: used = 0xFF,
  id = n.
- 5D29(n): `5D73(n); if (B(0x3EEE) != n) for each slot k < 4 with id == n: { 072E(ptr) /* start */; B(0x3EEE) = n; }`.
- 5D20: `B(0x3EEE) = 0xFF; 074B()` (driver stop; ported in legions.c as LEV_MUSIC_STOP).
- 5E1A(n): the used slot with id n: `868C(ptr)` (free), slot = {0, 0, 0xFF}, `B(0x3EEE) = 0xFF`.
- Music 8 = "cn6", archive 7 = **MNEGO CN6.LZW** (V, table 3EF0). The love theme starts at the alliance text and stops at 6458.
- Port (review fix, 9 Nov): 5D73's DS part is done in map.c `music_load_5D73` (called by `music_play_5D29`, shared by map.c
  and nego.c). With the sound on ([3EEC] != 0, as in every DOSBox capture: FFFF) "cn6" takes the first free slot
  (in-game: slot 1) as {1, FF, 8} (handle 1 stands for the DOS segment), and 5E1A leaves it {0, 0, FF}: after any Cleopatra
  ending [A6B7] = FF as in the original (before the fix the port left 00). With the sound off ([3EEC] = 0, OVL0) 5EB5
  returns 0 and only the handle word 0 is written, in both.
- 5D73 detail (D): the id is written through slot index [bp-2], the loaded flag through [bp-1]; on the only path that
  writes them both hold the same free slot, so this is not a bug.

### 2.9 Others
- 1E21 disk_check: loops `while (!file_exists([6AA]))` showing 6AC / 6DB "insert disk" boxes (floppy installs). No-op in the
  port (as in legions.c). It is called before the battle only.
- 198F(&buf): `free(W(buf))` (74B7). 19A0(p): `if (p) { 86A0(p) /* free the sheet pixels */; free(p); }`.
- 67E4 target_back: unless [9B26] == 6 (Tandy), copy 9 words C1DA -> C0D0 (the current draw target := work buffer); Tandy:
  `A13A(0x3DF, 0xE6)` (page register). 680F target_screen: copy 9AB6 -> C0D0; Tandy: `A13A(0x3DF, 0xF6)`. 6837: page flip
  for Tandy ([4F96]), then target = screen.
- 691B / 6973 = save screen -> work / restore work -> screen of a rect (BATTLE_UI_NOTES).

## 3. Variables used (meaning)

| DS | meaning | source |
|---|---|---|
| 5971 b | rank 0..8: Officer, Centurion, Tribune, Legatus, General, Praetor, Consul, Proconsul, Caesar (names W(5972+2r)) | D (V names) |
| 634E b | a 0-100 "standing" level (G: Rome's reputation / the people's favour): +5 per End Turn capped at 100; plunder halves or quarters it; a province throwing off the yoke cuts it; raising a tribute level above the agreed one costs 15; > 75 gives +1 score per turn; **an undefended province taken by an army multiplies it by 1.25 with no cap** (enemy.c 0x5175) | D (endturn.c/enemy.c/GAMELOOP) |
| P+0x00 / +0x02 w | the province's own infantry / cavalry (the army 0xFF fights with them) | D |
| P+0x04 / +0x06 w | manpower pool / its cap | D |
| P+0x0E b | unrest (name from province.c; signed byte; games lower it, plunder raises it) | D use, G name |
| P+0x12 b | tribute level (0..4); set to 1..3 by an alliance, to [5FC0] by a conquest | D |
| P+0x13 b | the tribute level agreed in the treaty: End Turn charges [634E] -15 when +0x12 > +0x13 (endturn.c 0x0BBA) | D |
| P+0x15 b | status: 0 independent, 1 Occupied, 2 Ally, 3 Colony, 4 Citizens | D |
| P+0x17 b | rebellion flag (rebel branch) ; P+0x19 b: turns-since counter, zeroed here | D |
| rec+6 / +8 w | the legion's infantry / cavalry | D |
| 5FC0 b | tribute policy for conquered provinces | D |
| C106 w | statistics: number of alliances | D |
| C1EE w | "tab" sheet (8 images: log frame 0..3, menu frame 4..7) | D/V |
| C10E / C1EC w | love1 (16 images) / love2 (4 images) sheets, Aegyptus only (6570) | D/V |
| C1D8 w | y of the next log line | D |
| 3E40 w | "Click to continue" / "Hit any key to continue" | D |
| 6344 / 6346 / 6348 w | VGA run-time 0x74 panel, 0x1A box, 0x10C text-erase colour (static 3/7/7) | V (runtime image) |

So: `mood` = [634E]/34 is the province's view of Rome (0 bad, 1, 2 good), one step worse when its unrest > 75;
`weak` selects the "your legion is smaller than our army" row; `state 1` needs both the rank and the standing.

## 4. Dice order (076B) inside the dialogue - D
1. 6501 roll(3) for every province text: the opening (state 0/1), each later state text, the alliance text, the Cleopatra
   guard text (0x11..0x13). Narration (4D0C/4DD4/4E28) and player lines roll nothing.
2. 0x6443 roll(12) in the Cleopatra end loop, only after more than 72 ticks since the last timer start, once per such
   period; < 2 shows the flash. Timing dependent: the number of rolls depends on how long the player waits.
3. Go to War: B06E's dice (BATTLE_NOTES) after 1E21.
No other dice: the reply's effect is table-driven.

## 5. DS tables (static DS, OVL0.OVL 0xB400) - V (dumped by `wf/nego/ds.py`, `reach.py`)

### 5.1 Replies 4028 (17 string pointers) and hotkeys 404A (17 bytes)
| k | 4028[k] | key | | k | 4028[k] | key |
|---|---|---|---|---|---|---|
| 0 | 3F4A Withdraw | W | | 9 | 3FA4 Gentle | G |
| 1 | 3F53 Go to War | G | | 10 | 3FAB Pushy | P |
| 2 | 3F5D Ultimatum | U | | 11 | 3FB1 Rude | R |
| 3 | 3F67 Offer Alliance | O | | 12 | 3FB6 Approach | A |
| 4 | 3F76 Aggressive | A | | 13 | 3FBF Leave | L |
| 5 | 3F81 Friendly | F | | 14 | 3FC5 Low | L |
| 6 | 3F8A Diplomatic | D | | 15 | 3FC9 Medium | M |
| 7 | 3F95 Warm | W | | 16 | 3FD0 High | H |
| 8 | 3F9A Sarcastic | S | | | | |

k = 0 and 1 end the talk; k >= 14 is the tribute answer. No reachable menu has two items with the same hotkey (V).

### 5.2 Menu record 4080 and its items 405C
4080: `08 00 97 00 67 00 C5 00 | 00 00 | 5C 40 | 02 3E | 03 | FF | FF FF` = box (8,151)-(103,197), colour 0, items 405C,
title 3E02 "", count 3 (byte 408E, rewritten each turn), single ornament 0xFF, +0x10 = 0xFFFF.
Items (12 bytes) 405C / 4068 / 4074: `{text 3E02, key 0, 0, x0 7, y0 4|0x11|0x1E, x1 0x5D, y1 0x0E|0x1B|0x28}`; the loop
rewrites only text and key. Item rects are 13 px apart (not the stock 10). 6570 confines the mouse to x 8..103, y 151..197
(06E4/0709) and puts it at (88, 171) (066A) for the whole negotiation.

### 5.3 Per-province parameters 4092 (min rank), 40AE (min [634E]), 40CA (tribute tolerance), 42EC/4322/455A
| z | province | text | 4092 rank | 40AE | 40CA | B 42EC | c 4322 | A 455A |
|---|---|---|---|---|---|---|---|---|
| 1 | Carthago | TEXTA | 4 General | 70 | 1 | 4226 | 12 | 442E |
| 2 | Aegyptus | TEXTB | 6 Consul | 80 | 2 | 4162 | 19 | 433E |
| 3 | Macedonia | TEXTC | 4 General | 60 | 2 | 42A0 | 11 | 44E2 |
| 4 | Gaul | TEXTD | 2 Tribune | 40 | 1 | 41A2 | 10 | 43B0 |
| 5 | Parthia | TEXTE | 4 General | 70 | 1 | 4226 | 12 | 442E |
| 6 | Syria | TEXTF | 4 General | 50 | 1 | 424E | 10 | 4476 |
| 7 | Thracia | TEXTG | 2 Tribune | 60 | 2 | 42A0 | 11 | 44E2 |
| 8 | Pontus | TEXTH | 3 Legatus | 40 | 2 | 424E | 10 | 4476 |
| 9 | Asia | TEXTI | 2 Tribune | 30 | 3 | 424E | 10 | 4476 |
| 10 | Cilicia | TEXTJ | 4 General | 10 | 1 | 4278 | 8 | 44B2 |
| 11 | Alpes | TEXTK | 0 Officer | 10 | 1 | 41A2 | 10 | 43B0 |
| 12 | Germania | TEXTL | 2 Tribune | 30 | 1 | 41D0 | 10 | 43B0 |
| 13 | Dalmatia | TEXTM | 1 Centurion | 20 | 2 | 41FE | 11 | 43EC |
| 14 | Dacia | TEXTN | 2 Tribune | 30 | 1 | 4226 | 12 | 442E |
| 15 | Hispania | TEXTO | 2 Tribune | 30 | 3 | 42C8 | 9 | 4524 |
| 16 | Mauretania | TEXTP | 2 Tribune | 40 | 2 | 42C8 | 9 | 4524 |
| 17 | Cyrenaica | TEXTQ | 2 Tribune | 40 | 2 | 4278 | 8 | 44B2 |
| 18 | Arabia | TEXTR | 1 Centurion | 10 | 1 | 42C8 | 9 | 4524 |
| 19 | Mesopotamia | TEXTS | 3 Legatus | 30 | 2 | 42A0 | 11 | 44E2 |
| 20 | Armenia | TEXTT | 1 Centurion | 20 | 1 | 42A0 | 11 | 44E2 |
| 21 | Sicilia | TEXTU | 1 Centurion | 40 | 1 | 42C8 | 9 | 4524 |
| 22 | Sardinia | TEXTV | 2 Tribune | 40 | 1 | 424E | 10 | 4476 |
| 23 | Britannia | TEXTW | 4 General | 10 | 1 | 41FE | 11 | 43EC |
| 24 | Narbonensis | TEXTX | 0 Officer | 10 | 1 | 4226 | 12 | 442E |
| 25 | Scythia | TEXTY | 1 Centurion | 10 | **0** | 41FE | 11 | 43EC |
| 26 | Sarmatia | TEXTZ | 1 Centurion | 10 | 1 | 4226 | 12 | 442E |

Entry 0 (Italia) is 0 / null in all tables. The text letter is `'@' + z` (6570 writes `z + 0x40` into "textA.lzw" at 4EE8 and
"leaderA.cbm" at 4EF4). 40CA = how many tribute levels the province accepts: Low needs >= 1, Medium >= 2, High >= 3.
Number of texts per file = number of B rows for every province (V).

### 5.4 Reply codes: 40E6 (code -> reply k), 4124 (code -> column of A), 4C92 (code -> the player's line)
| code | k | col | 4C92 line |
|---|---|---|---|
| 00 | 0xFF | 0 | "" (never offered) |
| 01 | 2 Ultimatum | 0 | Surrender or die! |
| 02 | 5 Friendly | 1 | We come in friendship. |
| 03 | 6 Diplomatic | 2 | Your courage and wisdom are legendary. |
| 04 | 1 Go to War | 0 | Then we are at war. |
| 05 | 0 Withdraw | 0 | We leave you in peace. |
| 06 | 3 Offer Alliance | 3 | We offer an alliance. |
| 07 | 4 Aggressive | 4 | Submit to the power of Rome! |
| 08 | 5 Friendly | 5 | We should trust each other, my friend. |
| 09 | 6 Diplomatic | 6 | Our terms are extremely generous. |
| 0A | 3 Offer Alliance | 7 | We want you as allies. |
| 0B | 4 Aggressive | 8 | Lay down your weapons. |
| 0C | 1 Go to War | 0 | Die, barbarian pig! |
| 0D | 6 Diplomatic | 9 | We seek to negotiate with you. |
| 0E | 14 Low | 0 | The lowest in the Republic. |
| 0F | 15 Medium | 0 | Our demands will not be unreasonable. |
| 10 | 16 High | 0 | We require tribute befitting the grandeur of Rome. |
| 11 | 3 Offer Alliance | 0 | We come in peace and seek an ally. |
| 12 | 6 Diplomatic | 1 | Hear me, wise and honorable chieftain. |
| 13 | 4 Aggressive | 2 | Your choice is simple. Surrender with honor or die in disgrace. |
| 14 | 5 Friendly | 3 | I am here to make peace. |
| 15 | 6 Diplomatic | 5 | Forgive our arrogance, honorable chieftain. |
| 16 | 3 Offer Alliance | 6 | Join us and the armies of Rome will protect you. |
| 17 | 6 Diplomatic | 7 | We will try to earn your trust. |
| 18 | 4 Aggressive | 8 | That is the choice we offer, and we will not make it again. |
| 19 | 3 Offer Alliance | 9 | Then we offer an alliance, and the promise of protection. |
| 1A | 2 Ultimatum | 10 | Submit now, or be destroyed. |
| 1B | 5 Friendly | 1 | Rome can be a powerful friend. |
| 1C | 6 Diplomatic | 2 | Great queen, we merely wish to negotiate. |
| 1D | 4 Aggressive | 3 | Capitulate now, and there will be no bloodshed. |
| 1E | 5 Friendly | 4 | Protection, great queen.  Aegyptus has weakened, and Rome is great. |
| 1F | 6 Diplomatic | 5 | Rome desires only peace between our great nations. |
| 20 | 4 Aggressive | 6 | Take care! Arrogant queens are easily crushed! |
| 21 | 6 Diplomatic | 7 | Indeed, we are humbled by your presence. |
| 22 | 3 Offer Alliance | 8 | I can offer you an alliance with Rome. |
| 23 | 3 Offer Alliance | 9 | We offer an alliance, and promise protection in exchange for tribute. |
| 24 | 5 Friendly | 10 | We should trust each other, great queen. |
| 25 | 5 Friendly | 2 | I am here to make peace. |
| 26 | 6 Diplomatic | 4 | We will try to earn your trust. |
| 27 | 4 Aggressive | 6 | Capitulate now, and there will be no bloodshed. |
| 28 | 6 Diplomatic | 11 | Rome desires only peace between our great nations. |
| 29 | 4 Aggressive | 3 | Your choice is simple. Surrender with honor or die in disgrace. |
| 2A | 5 Friendly | 8 | Rome can be a powerful friend. |
| 2B | 6 Diplomatic | 0 | Rome desires only peace between our great nations. |
| 2C | 4 Aggressive | 2 | Lay down your weapons. |
| 2D | 6 Diplomatic | 3 | We seek to negotiate with you. |
| 2E | 6 Diplomatic | 4 | Hear me, wise and honorable chieftain. |
| 2F | 5 Friendly | 7 | Rome can be a powerful friend. |
| 30 | 6 Diplomatic | 1 | Your courage and wisdom are legendary. |
| 31 | 3 Offer Alliance | 4 | Then we offer an alliance, and the promise of protection. |
| 32 | 3 Offer Alliance | 0 | We offer an alliance. |
| 33 | 4 Aggressive | 5 | Submit to the power of Rome! |
| 34 | 3 Offer Alliance | 7 | We offer an alliance, and promise protection in exchange for tribute. |
| 35 | 7 Warm | 11 | My feelings for you are genuine.  Your beauty, Cleopatra, is unsurpassed throughout the world. |
| 36 | 8 Sarcastic | 12 | How you flatter yourself, great Queen. |
| 37 | 4 Aggressive | 13 | You are the only conquest I seek, and I never lose a battle. |
| 38 | 9 Gentle | 14 | My queen, hunger is a simple need, while love is a great gift. |
| 39 | 10 Pushy | 15 | Romans take what is theirs.  It is that simple. |
| 3A | 11 Rude | 16 | I grow impatient with you, Cleopatra. |
| 3B | 12 Approach | 17 | Then I offer you this gift... |
| 3C | 13 Leave | 18 | My queen, I must go. |
| 3D | 0 Withdraw | 0 | (4C92[0x3D] would be the word at 4D0C = text bytes; never selectable, count 2) |

The column (4124) of a reply means nothing across provinces: it is just the index into that province's A row.

### 5.5 State -> offered codes (B, 3 bytes per state; pad byte at the end of most tables)
```
4162 Aegyptus:  s0 04/05/3D s1 1B/1C/1D s2 00/00/00 s3 04/05/3D s4 1E/1F/22 s5 1F/1D/01 s6 20/21/22 s7 04/05/3D s8 24/1F/23
                s9 04/05/3D s10 04/05/3D s11 04/05/3D s12 04/05/3D s13 0E/0F/10 s14 04/05/3D s15 35/36/37 s16 38/39/3A
                s17 00/00/00 s18 00/00/00 s19 00/00/00 s20 3B/3C/3D
41A2 Gaul, Alpes:   s0 04/05/01 s1 08/0D/0B s2 00 s3 04/05/3D s4 03/08/0A s5 0C/07/01 s6 01/04/05 s7 04/05/3D s8 07/05/06
                s9 01/07/09 s10 04/05/3D s11 04/05/3D s12 0C/05/01 s13 0E/0F/10 s14 02/07/0A
41D0 Germania:  as 41A2 except s8 08/07/06
41FE Dalmatia, Britannia, Scythia: s0 1A/04/05 s1 11/12/13 s2 00 s3 04/05/3D s4 17/04/05 s5 14/15/16 s6 14/15/16 s7 04/05/3D
                s8 1A/07/16 s9 1A/18/19 s10 04/05/3D s11 04/05/3D s12 0E/0F/10
4226 Carthago, Parthia, Dacia, Narbonensis, Sarmatia: s0 04/05/3D s1 02/28/0B s2 00 s3 25/26/0A s4 0D/27/06 s5 1A/04/05
                s6 27/01/04 s7 25/0D/0A s8..s11 04/05/3D s12 08/26/06            (no tribute row!)
424E Syria, Pontus, Asia, Sardinia: s0 04/05/3D s1 02/03/07 s2 00 s3 04/05/3D s4 08/2B/16 s5 04/05/3D s6 04/05/3D
                s7 0E/0F/10 s8 04/05/3D s9 0D/29/0A s10..s12 04/05/3D s13 2A/0D/0A   (no pad)
4278 Cilicia, Cyrenaica: s0 04/05/3D s1 02/2D/2C s2 00 s3..s5 04/05/3D s6 2F/2E/16 s7 04/05/3D s8 0E/0F/10 s9 04/05/3D
                s10 04/05/3D s11 2F/2D/16 s12 08/2E/11
42A0 Macedonia, Thracia, Mesopotamia, Armenia: s0 04/05/3D s1 08/30/13 s2 00 s3 04/05/3D s4 2B/0B/31 s5 0D/0B/16 s6 04/05/3D
                s7 04/05/3D s8 0E/0F/10 s9 1A/04/05 s10 04/05/3D s11 14/0D/0A s12 04/05/3D
42C8 Hispania, Mauretania, Arabia, Sicilia: s0 04/05/3D s1 02/2E/27 s2 00 s3..s5 04/05/3D s6 2D/27/34 s7 04/05/3D s8 04/05/3D
                s9 0E/0F/10 s10 03/33/32 s11 03/0B/32   (no pad)
```
"04/05/3D" = the 2-item end menu Go to War / Withdraw. State 0 is "refuses to talk", 1 the opening, 2 the alliance (row never
shown: the talk ends, or Cleopatra), the row 0E/0F/10 asks "what tribute?", 3 is "tribute refused" in every table that has a
tribute row (in 4226 state 3 is an ordinary state). The other numbers mean different things per table.

### 5.6 Reply -> next state (A, 6 rows of c bytes: row = mood*2 + (weak ? 1 : 0), column = 4124[code])
```
433E Aegyptus (c 19)  col: 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18
  mood0 strong          14  5  5  7  8 14  9 10 11 12 14 16 17 18 20 19 19 21 22
  mood0 weak            14  5  5 14  8 14 14 10 11 12 14 16 17 18 20 19 19 21 22
  mood1 strong / weak   = mood0 strong / weak
  mood2 strong          14  4  6  7  8 14  9 10 11 13 14 16 17 18 20 19 19 21 22
  mood2 weak            14  4  6 14  8 14 14 10 11 12 14 16 17 18 20 19 19 21 22
43B0 Gaul/Alpes/Germania (c 10)
  m0 s  7 5 12 3 10 12 3 5 14 12 | m0 w  7 5 12 3 11 12 3 5 14 12 | m1 s  7 4 8 3 10 9 3 5 14 12
  m1 w  7 5 12 6 11 12 3 5 14 12 | m2 s  7 4 8 13 10 9 3 5 14 12  | m2 w  7 4 8 6 11 9 3 5 14 9
43EC Dalmatia/Britannia/Scythia (c 11)
  m0 s  4 7 8 11 11 11 10 11 11 10 11 | m0 w  4 7 8 11 3 11 10 11 11 10 11 | m1 = m0
  m2 s  5 6 9 11 11 11 10 11 11 12 11 | m2 w  5 6 9 11 3 11 10 11 11 10 11
442E Carthago/Parthia/Dacia/Narbonensis/Sarmatia (c 12)
  m0 s  9 12 10 8 7 7 10 11 6 11 9 3 | m0 w  9 12 10 8 7 7 10 11 5 11 9 3
  m1 s  9 12 10 8 7 7 10 11 6 11 9 4 | m1 w  9 12 10 8 7 7 10 11 5 11 9 4 | m2 = m1
4476 Syria/Pontus/Asia/Sardinia (c 10)
  m0 s  6 12 9 11 10 6 8 5 4 6 | m0 w  6 13 9 11 10 6 8 5 4 6 | m1 = m0 | m2 s  6 12 9 11 10 6 7 5 4 6 | m2 w  6 13 9 11 10 6 7 5 4 6
44B2 Cilicia/Cyrenaica (c 8)
  m0 s/w  8 12 5 4 7 6 10 9 | m1 s/w, m2 s/w  8 11 5 4 7 6 10 9
44E2 Macedonia/Thracia/Mesopotamia/Armenia (c 11)
  m0 s  5 12 7 12 6 11 8 12 12 12 10 | m0 w  5 12 7 12 6 11 9 12 12 12 10 | m1 = m0
  m2 s  5 4 7 12 6 11 8 12 12 12 10  | m2 w  5 4 7 12 6 11 9 12 12 12 10
4524 Hispania/Mauretania/Arabia/Sicilia (c 9)
  m0 s  5 10 5 8 11 5 4 9 7 | m0 w  same | m1 s  5 10 5 8 11 5 4 9 6 | m1 w  5 10 5 8 11 5 4 9 7
  m2 s  5 10 5 8 11 5 4 9 6 | m2 w  5 10 5 8 11 5 4 9 7
```
Each A table is exactly 6*c bytes and they are contiguous (433E, 43B0, 43EC, 442E, 4476, 44B2, 44E2, 4524, then the pointer
table 455A) (V). No A entry is 2: **an alliance only ever comes from the tribute menu.** So `sel + 1` in 623C is always the
tribute level (Low 1, Medium 2, High 3).

### 5.7 Fixed texts and file names
- 4D0C (narration, after the guards): "Cleopatra calls the guards, and you quickly turn to leave, wishing to avoid the humiliation
  of being escorted to the door.  The audience has not turned out very well, but the alliance remains intact."
- 4DD4: "Cleopatra entertains the Romans in her palace, and the celebration lasts five days."
- 4E28: "Infatuated with the Aegyptian queen, you tarry in Alexandria until finally you gain entrance to her bedchamber on her
  barge. Cleopatra is as sharp-tongued as ever."
- 4ED4 "love1", 4EDA "love2", 4EE0 "tab", 4EE4 "textA.lzw", 4EEE "leaderA.cbm" (letters patched with '@'+z), 3E02 "",
  3E6C/3E6E/3E70/3E72 " " (3E72 becomes the quote char), 3E74 = {120, 86, 305, 190} (+2 rewritten to 86 + line step).
- 3EF0 music table {name, archive}: 0 cn1/3, 1 cn2/5, 2 cn3a/0, 3 cn3b/0, 4..7 cn4a..d/6, **8 cn6/7**, 9..10 cn7a,b/2,
  11..12 cn8a,b/2, 13..16 cn9a..d/8; suffix 3F44 ".lzw".
- Sheets (14-byte entries `<HhhBBBBI` w, xhot, yhot, bpr, h, trans, lut, off): tab 8 images (0 bottom 200x6, 1 left 5x110,
  2 right 6x110 hot -194, 3 top 200x4; 4..7 the menu frame 98x5 / 5x39 / 8x39 / 98x4), love1 16 images (0 224x66, 1 96x66,
  2 44x63, 3..15 the animation, lut 7), love2 4 images (0 96x66 portrait lut 9, 1 319x63 couch lut 9, 2 320x63 barge lut 8,
  3 238x23 flash lut 8).

## 6. Reachability (V, `wf/nego/reach.py`: walks every menu row, every mood and weak/strong)
- Every reachable state is below the province's text count; every text is reachable except: state 2 (alliance) in the five
  4226 provinces Carthago, Parthia, Dacia, Narbonensis, Sarmatia (no tribute row; their text 2 is a single space) and in
  **Scythia** (40CA = 0, see 9.2); state 3 (tribute refused) in Asia and Hispania (40CA = 3 accepts everything).
- Aegyptus: s0 refuses; s1 -> s4..s14 by reply; s13 = tribute -> 2 (Low, Medium) or 3 (High). From 2 -> 0x0F (via 62C2):
  Warm -> 0x10, Sarcastic -> 0x11, Aggressive -> 0x12; 0x10: Gentle -> 0x14, Pushy / Rude -> 0x13; 0x14: Approach -> 0x15
  (bedchamber animation), Leave -> 0x16 (no text: 6458 at once). 0x11..0x13 end with the guards (4D0C). TEXTB has 21 texts
  (0..0x14); 0x15 / 0x16 never reach 4B8. Mood and weak do not change the Cleopatra part.
- Hotkeys are unique in every reachable menu; 0x3D is always the third code.

## 7. The first pass (LEGIONS_NOTES 5F82) checked
Correct: the variable set-up (mood, A, B, c, weak, state 1 rule), the menu building incl. 0x3D -> 2 items, the cannot-Esc menu
loop, `x = 40E6[code]`, the player line 4C92 in colour 9, the tribute rule `40CA[z] > x - 14 ? 2 : 3`, the A index
`mood*2*c + 4124[code] + weak`, the alliance writes, the Cleopatra states 0x11..0x13 / 0x15 / 0x16, `result -> 53F8`.
Corrections and additions:
1. Every province line is said with prefix '"' (5AE4 third argument 0x22): **lines are wrapped in double quotes**; narration
   (4D0C, 4DD4, 4E28, blank lines) has none. Player lines are quoted too.
2. Withdraw: the delay comes after the frees; Go to War has no delay; for Aegyptus love1/love2 are freed first.
3. Alliance (non-Aegyptus) also does `5AC2(1)`, Click to continue (7CB2) and frees the text before ending (first pass: only
   `end = result = -1`).
4. Cleopatra (62C2..6341): Click to continue, then music 8 (cn6), the barge picture, menu area cleared, 4DD4, blank, 4E28,
   blank, Click to continue, the couch picture + Cleopatra portrait, state 0x0F (the first pass had the texts and 0x0F only).
5. 0x11..0x13: the state text (roll(3)), blank line, 0x90 delay, 4D0C, Click to continue.
6. 0x15: the roll(12) is made only when more than 72 ticks passed since the last restart (and the timer restarts before the
   roll), and one flash is shown at once before the loop; the loop ends on a key or a mouse button. Then 6837.
7. All Cleopatra ends: also music stop 5D20 and unload 5E1A(8).
8. Exit: `19A0([C1EE])` (free "tab") after the cursor pop - missing in the first pass. The 6570-failure exit does not free it.
9. The alliance's `P.w04 = min(...)` is two 16-bit adds (wrap) then the cap - same as the first pass unless it wraps.
10. GAMELOOP_NOTES "0x62C2 ... a menu screen builder" (STAGE4 / MAINLOOP.md "ui_begin", "window", "list") is the Cleopatra
    feast scene: 5AC2 = blank log lines, 8E62 = sprite, 5AE4 = log text, 7CB2 = Click to continue.
11. LEGIONS_NOTES 4 dice table: "6501 roll(3) every text the province speaks" is right (also the alliance and guard texts);
    "6443 roll(12) loop while waiting for a click" -> only every > 72 ticks.

## 8. What the implementer must know
- Read the tables straight from `g_ds` (all static, identical in the runtime image: only 3E40/3EE9/3EEA/3EEC/3EED differ in
  3C00..4F00, V). Keep byte arithmetic: `(uint8_t)(4124[code] + weak)`, 8-bit `div` for mood, signed `> 75` for unrest.
- The 3 dice sites must be called in the same order; the roll(12) count depends on wall time, so a headless `--script` run
  should drive the end loop by a scripted click (as the original, a click at once means: one flash, no roll).
- Delays 143 are cut short by a click/key (and the click is then still "down" for the next wait); 48EF's 14-tick steps are not.
- 5AE4 needs strtok semantics on ' ' only, width 186 px, line step from the font ([9B5C]+[9B5D]-1 = 10), scroll by copying
  (120,96)-(305,190) up by 10 and clearing the last line with [6346]. [9B69] keeps the last colour.
- The text log y [C1D8] is shared with the scout report (7B56, inside 6570): the dialogue continues below it.
- 0x16 (Leave) and 0x15 have no TEXTB entry; never call 4B8 for them.
- A nonzero result (won battle, alliance, any Cleopatra end) runs 53F8 here and the caller scores and moves the legion.
- Music: needs a "play cn6" / "stop" / "unload" hook (5D29/5D20/5E1A); legions.c already has the stop event.

## 9. Quirks and 1990 bugs
1. **mood out of the table (1990 bug).** [634E] can exceed 100: a province taken undefended by an enemy army multiplies it by
   1.25 with no cap (enemy.c 0x5175), and End Turn's +5/cap 100 runs before that, so it stays > 100 for the player's next turn
   (one fall: up to 125; two in one turn: 156). [634E] >= 102 gives mood 3 (>= 136: 4..7), and the A read goes past the 6-row
   table into the next table / the 455A pointer table. V (reach.py): with mood 3 Hispania/Mauretania/Arabia/Sicilia get states
   0..226, Macedonia group at mood >= 5, Carthago group gets 13 (one past the last text). 4B8 then copies `off[n+1] - off[n]`
   bytes from beyond the text index (garbage length, likely a huge `rep movsb` over DS in the original: crash or corrupt text).
   G for the DOS effect. Suggested fix (Erik's rule: fix, `--original` keeps): clamp mood to 2; under `--original` read A from
   g_ds as is but bounds-check 4B8 / the B row in the port (log the out-of-range state and end the talk like Withdraw), since an
   exact replay of the memory overwrite is not possible. Not ported yet; for Erik to decide.
2. **Scythia can never become an ally** although TEXTY has three alliance variants: 40CA[25] = 0 refuses even Low tribute.
   Possibly a data slip (all other provinces with a tribute row have >= 1). The alliance text itself says "I cannot guarantee
   what the Scythians will do". G whether intentional; it is data, so the port shows it as is unless Erik wants it changed.
3. Carthago, Parthia, Dacia, Narbonensis and Sarmatia cannot be allied at all (no tribute row; text 2 is " "). Intentional (the
   texts say so: "Hannibal does not surrender").
4. 2-variant texts pick the second variant 2/3 of the time (6501).
5. Hiring the province's troops: the alliance adds the province's own infantry and cavalry to its manpower pool (capped) but
   does not zero +0x00/+0x02.
6. The mouse stays confined to the menu box during the whole talk, including the Cleopatra ending (6570 sets it; 5F82 never
   resets it; the caller's map redraw does).
7. Click to continue in 0x15 is drawn at y 184 and never erased (7CB2 uses y 183 and erases).

## 10. Easter eggs and jokes (all kept as data)
- **Cleopatra** (Aegyptus, needs rank Consul and [634E] >= 80, then a tribute alliance at Low or Medium): five-day feast,
  bedchamber on her barge, her own love music (cn6), three pictures and an animation (love1/love2), a randomly flashing
  overlay on the barge, a romance mini-dialogue (Warm / Sarcastic / Aggressive -> Gentle / Pushy / Rude -> Approach / Leave)
  with "GUARDS!!!" endings ("Perhaps you are about to have a new experience, Roman.") and the narration "The audience has not
  turned out very well, but the alliance remains intact."
- Scythia (TEXTY) speaks like Yoda ("Hear ye I will, trust ye I will not", "Powerful are the Scythians, but honorable they are
  not") - and its own alliance is unreachable (9.2).
- Narbonensis (TEXTX) talks caveman ("UH-LIE-ANCE.  THIS WORD I NOT LIKE.", "YOU TRIBE WEAR CLOTHES.  NOT RIGHT!").
- Germania (TEXTL): "ROMAN SMELL LIKE ELEPHANT." / "ROMAN LOOK LIKE WARTHOG."; Gaul (TEXTD) shouts in capitals
  ("PTOOEE!!", "GAUL SURRENDER TO ROMANS? NEVER!" - Asterix flavour, cf. the Asterix village egg in the map).
- Sicilia (TEXTU): Godfather quotes - "an offer ... we cannot refuse", "You will sleep with the fishes!".
- Mesopotamia (TEXTS): "So let it be written." (The Ten Commandments); "The heirs of Hammurabi shall slaughter you like sheep."
- Armenia (TEXTT): "Might Romans be part Armenian?" and neighbour digs ("You extort like a Cilician", "graceless as a
  Syrian's", "worse than a Greek", "You lie better than even a Parthian").
- Cilicia / Cyrenaica pirates: "Pirates and Romans belong together!", "Who is the true pirate here?", "Roman shark".
- Dacia's rank-too-low refusals: "We won't deal with women.", "They send a eunuch to parlay with warriors."
- Carthago: Hannibal - "I beat your best generals, and now I am sent a subaltern?"; Macedonia quotes Euripides.
- Original typos kept as data: "suprises" (TEXTH 2), "Begin the the attack!" (TEXTE 11), "parlay" (TEXTN 0), missing full stop
  "The alliance is made" (TEXTU 2), "Provincal Games" elsewhere.
- Pictures: hieroglyph cartouches in love1[0] / love1[1] (not deciphered, G: decorative).

## 11. Open questions
- 3603 (key -> click mapping) in mouse mode (36E4) and the exact keys that end delay_143 / the Cleopatra loop: not decoded.
- 8BE4's use of the high byte of [9B69] (0x10C) when 7CB2 erases "Click to continue": check against a capture.
- The love2[3] flash content and love1 animation: only seen in greyscale (palette LUTs 7/8/9 not applied).
- How the DOS original behaves with mood >= 3 (9.1): only predicted from the code.
- The meaning of [634E]'s name in the game's own UI (if it is shown anywhere): G "reputation".

## 12. Scratch scripts (not in the repo)
`C:\Users\erikk\AppData\Local\Temp\claude\bfb5522be8ee045d\c4bf0f5f-57f9-4ea6-80b3-e1aa05bf0592\scratchpad\wf\nego\`:
`ex.py` (unpacks MNEGO to `out\`), `ds.py` (static DS reader), `reach.py` (state walk), `texts.txt` (all 26 text files),
`sheet.py` / `up.py` / `comp.py` / `comp2.py` (greyscale renders of tab/love1/love2, `bed_*.png`, `barge_*.png`).
