# Game loop notes: the map, its commands, Game Control, saving and the end of the game

Claude, 2026-11-09. From the runtime EXE image (disasm/EXE_RUNTIME.bin, fresh llvm-objdump listing, because the clean listing and
seen.pkl lack 0x079B-0x07EE), OVL0 sections 1/3/4/5 disassembled at 0823:B0E0, the static DS in OVL0 (file 0xB400 = DS:0000),
the runtime DS image tools/dosbox/results/endturn/t10_in.bin, and the screen captures.
Labels: **VERIFIED** = read in the code and checked against data or a capture by a script (the script is listed);
**DECODED** = read in the code only; **GUESS** = inference.
The checks: `python <scratch>/wf/gameloop/checks.py` from C:\work\Centurion (scratch = C:\Users\erikk\AppData\Local\Temp\claude\
bfb5522be8ee045d\c4bf0f5f-57f9-4ea6-80b3-e1aa05bf0592\scratchpad). Copy it to tools/ when this is committed.

## 0. Corrections to earlier docs

- **docs/MAINLOOP.md is wrong about both functions it names.** 0x49CE is not a menu loop: it is the flood-fill loop *inside* the map
  builder 0x494A (no caller jumps to 0x49CE). 0x62C2 is not a screen builder entry: it is in the middle of **0x5F82** (a legion enters a
  non-Roman province), in the part that draws the negotiation screen (music 8, MNEGO archive via 0x6570). DECODED (no callers of either
  address; 0x5F82 runs to `ret 4` at 0x64FE with no prologue in between).
- **Overlay sections (LOGIC_NOTES and the task text say section 3 = naval): section 3 is the gladiators, section 5 is the naval battle.**
  Section 3 code loads DS:7BA1 "Can't Load Gladiator Graphics"; section 5 loads DS:9975/9995 "Failed to Load Human/Computer Sinking";
  section 4 holds the chariot race and the victory sequence (it loads DS:8CD0 "THE MAKING OF A CONQUEROR"). DECODED.
- Thunk table (EXE 0xB06E.., 12 bytes each: `lcall 129B:02EC; dw section; jmp 0823:entry`), all DECODED from the bytes:

| thunk | section | entry | what | called from |
|---|---|---|---|---|
| B06E | 2 | DF24 | land battle (ported, battle.c) | 0E1C, 4B85, 5056, 50FC, 6006, 64BB |
| B07A | 1 | B18A | init: hardware, **copy-protection question B466** (fail → exit(2)), intro B694, sprites, enemy fleet records B397(3..5) | 07C2 |
| B086 | 1 | B0E0 | command-line parsing (mouse off, video mode, sound) | 07B4 |
| B092 | 4 | DF58 | **victory sequence** (summary text, BDA4, BF6A(1), C2C2, then credits B0CE) | 09A4 |
| B09E | 5 | B0E0 | fleet enters a sea zone: fights each enemy fleet 3..5 there (B12A); returns 1 = way clear | 2C24 |
| B0AA | 4 | D08A | chariot race | 349D |
| B0B6 | 3 | B0E0 | gladiator show | 34F5 |
| B0C2 | 5 | B12A | naval battle B12A(enemy fleet record, Roman fleet) | 4B22, 5221 |
| B0CE | 1 | B9A1 | credits (pairs from DS:6A6E) | section 4 DF70 |

- The four "module test entries" in the task (Land Battle / Racing Chariots / Gladiatorial Combat / Naval Battle) are **not test entries**:
  they are the **Fine Tuning Menu** DS:05CA, which sets the per-module difficulty DS:048E[0..3]. DECODED.
- The Rome menu is **not** the provincial menu with one item swapped: Rome (zone 0) has its own screen 0x7366 and its own menu record
  **DS:5A30** ("Legions, Tribute, Hold Games, Build Fleet, Tribute Policy, See Province List, Continue"). VERIFIED (check 5: the 7 items
  rendered from DS:5A30 at the engine's positions match scr_rome_province: 0 extra px, 18 of 733 missing = the mouse cursor at x 140-143,
  y 125-137). The port's "swap_plunder" exception in game.c should become "zone 0 uses DS:5A30".

## 1. Program start: EXE 0x079D `main(argc, argv)` (DECODED)

```c
void main(int argc, char **argv) {             /* 0x079D */
    int quit = 0;                               /* di */
    dice_seed();                                /* 0x075A: s1:s2 = BIOS ticks */
    video_init();                               /* 0x80D6 */
    ovl_B086(argv, argc);                       /* section 1 B0E0: command line */
    B(0x9AF8) = 0xFF;
    mem_gfx_init();                             /* 0x7DAA ("Remove TSRs: can't allocate high mem", "No mem for char_BM") */
    detect_drives();                            /* 0x1E02: [0xC0E4] drive, [0x698]/[0x696] two-drive flags */
    if (!ovl_B07A()) exit_game(1);              /* section 1 B18A: init + copy protection + intro + B397(3..5) */
    W(0x9AFA) = 0;                              /* draw colour */
    fill_rect(0, 0, 319, 199);                  /* 0x7E74(0xC7, 0x13F, 0, 0) */
    map_build(0);                               /* 0x494A(0): map screen, mouse warped to Italia's anchor */
    cursor_show();                              /* 0x1CCE */
    goto check;                                 /* 0x09A7 */
loop: ... (section 2) ...
check:
    if (!quit) goto loop;                       /* 0x09A7 */
    exit_game(0);                               /* 0x0EE7: sound off, ISRs back, video mode back, DOS exit */
}
```
**The game starts on the static DS image: there is no New Game step at startup.** VERIFIED (check 2): OVL0's static DS plus the
resident init B397(3), B397(4), B397(5) (enemy fleet records 3-5 → army records 11-13 at DS:C112+14*(i+8), fleet +3 = i+8, +4 =
strength/count) gives fresh_275BC.csv in all 1780 bytes except the description (CSV 0x04-0x21). B397 in pseudo-C:
```c
void fleet_army_init(int i) {                   /* section 1 B397, i = 3..5 */
    uint8_t *f = &B(0x1114 + 12*i); int a = 0xC112 + 14*(i+8), home = f[1];
    f[3] = i + 8; W(a+2) = home; B(a+1) = f[0]; B(a+0x0A) = 0; W(a+4) = 0;      /* W(0xC186+14i) = rec+4 */
    B(a+0x0B) = B(0x5270 + 32*home + 0x0B); B(a+0x0C) = B(0x5270 + 32*home + 0x0A); W(a+8) = 0xFFFF;
    for (j = 0; j < 3; j++) { cnt = W(0x110E + 12*i + 2*j);                   /* ship counts: 3 words per fleet at DS:110E+12f */
        if (cnt) { v = min((uint16_t)(B(0x8EDE + j) * cnt), 8000); W(a+6) = v; f[4] = v / cnt; } }
}
```
Copy protection (section 1 B466, PLACES.LZW, "What is the <Northern/...> <Town/...> in the province of <name>?", 3 tries): the port skips it
(Erik's decision, HANDOFF 2026-11-08).

## 2. The map loop (EXE 0x07EF-0x09B9, in main) (DECODED; pieces VERIFIED where marked)

**Nothing runs per frame on the map.** The loop blocks in 0x359B until a click or a key; the music plays from the int 08 ISR (0x0000,
`lcall [0xC4]` = ADLIB driver). There is no animation, no idle tick, no timer.

```c
int wait_input(void) {                          /* 0x359B: 1 = left, 2 = right, 0 = key in B(0xC07F) */
    mouse_poll();                               /* 0x0644: [0xB6] x, [0xB8] y, [0xB2] buttons */
    while (W(0xB2) == 0) {
        mouse_poll(); cursor_move(W(0xB6), W(0xB8));   /* 0x1BE6: restore under, save, draw sprite [0x684] */
        if ((k = key_poll()) != 0) { B(0xC07F) = k; return 0; }   /* 0x3603 (also keyboard mouse when no mouse, below) */
    }
    B(0xC07F) = 0; return (W(0xB2) & 1) ? 1 : (W(0xB2) & 2) ? 2 : 0;
}

loop:                                           /* 0x07EF */
    btn = wait_input();
    if (btn == 0) goto keys;
    if (in_rect(DS:3C42, x, y)) { end_turn_button(); goto after; }        /* 0x8238 point-in-rect; End Turn (227,2)-(275,18) */
    if (in_rect(DS:3C4A, x, y)) {                                          /* year button (277,2)-(317,18) */
        music_stop(); quit = game_control_menu(); map_build(0xFF); goto after; }   /* 0x5D20, 0x1A8B, 0x494A */
    z = zone_at(x, y);                                                     /* 0x6C67 */
    if (z < 27) {                                                          /* a province */
        if (B(0x5270 + 32*z + 0x15) != 0) {                                /* Roman (status 1..4) */
            B(0xC080) = z;
            if (btn != 1) goto right;
            music_stop();                                                  /* 0x5D20 */
            if (z == 0) rome_screen(); else province_screen(z);            /* 0x7366 / 0x6CB8 */
        } else zone_info(z);                                               /* 0x720B (left or right click) */
        goto after;
    }
    if (z == 0xFF) goto after;                                             /* no zone, or the Asterix village (handled in 0x6C67) */
    f = 0xFF; for (i = 0; i < 6; i++)                                      /* sea zone: LAST fleet record in z wins */
        if (B(0x1114+12*i) == z && B(0x1114+12*i+2) != 0xFF) f = i;
    if (f == 0xFF) { zone_info(z); goto after; }
    if (btn != 1) goto right;
    if (f >= 3) { zone_info(z); goto after; }                              /* an enemy fleet (3-5) is listed last */
    f = select_fleet(z);                                                   /* 0x2C6F "Select Fleet" (skips if only one) */
    restore_popup();                                                       /* 0x3F23 */
    if (f < 3) { fleet_info(f); fleet_move(f); }                           /* 0x2F5A, 0x2B40 */
    goto after;
right:                                                                     /* 0x08EF: right button = legions in that zone */
    if (btn == 2) { l = select_legion(z, 0); restore_popup();              /* 0x3BAF "Select Legion", 0x3F23 */
        if (l < 12) { legion_info(l); legion_move(l); } }                  /* 0x3EA3, 0x382A */
    goto after;
keys:                                                                      /* 0x091B, key = toupper(B(0xC07F)) via 0x9DDC */
    switch (key) {
    case 0x1B: /* Esc */ goto game_control;          /* 0x0828, same as the year button */
    case 'E': end_turn_button(); break;              /* 0x080F */
    case 'F': fleet_list(); break;                   /* 0x2FD6 "Complete Fleet List -- " */
    case 'L': legion_list(); break;                  /* 0x4280 "Complete Legion List -- " */
    case 'M': map_build(0xFF); break;                /* 0x0830: redraw */
    case 'P': province_list(); break;                /* 0x69CC */
    case 'Q': if (yes_no(DS:0172 "Really Quit?")) quit = -1; break;   /* 0x5C20 */
    }
after:                                                                     /* 0x096E */
    for (k = 0, any = 0; k < 27; k++) if (B(0x5285 + 32*k)) any = 1;     /* any province with status != 0 */
    if (!any) { msgbox(DS:017F "Sic Transit Gloria Mundi\nThe Roman Empire Has Fallen"); quit = -1; }   /* 0x59D4 */
    wait_release();                                                        /* 0x35DB */
    if (B(0x5971) == 8) { quit = -1; disk_check(); ovl_B092(); }           /* rank Caesar: victory sequence */
    goto check;
```
Where `end_turn_button()` = `end_turn()` 0x09BA (PORTED), which ends with rank 0x72DE, `turn++`, cursor pop, `map_build(0xFF)`, then
treasury and DS:C092 bookkeeping and the turn > 4 events (see LOGIC_NOTES). The map is redrawn **inside** End Turn, so the year button
shows the new year before the random events' messages.

Keyboard mouse (0x3603, only when DS:00BA = 0, no mouse): Enter (0x0D) or '+' = right button, Space or 0x10 = left button; arrow keys
(int 09 ISR state DS:00A2) move the pointer 8 px, 1 px with Shift (DS:00A4 & 0x14), clamped to [0xBC..0xC0]x[0xBE..0xC2]. DECODED.
The map builder sets the mouse range to x 0..311, y 0..185 (0x06E4(0x137,0), 0x0709(0xB9,0)) and, with an argument z != 0xFF,
**warps the pointer to the zone anchor DS:1AB8[z]** (0x066A), which is why the pointer sits on the province you just left. DECODED.

### Zone hit test 0x6C67(y, x) (VERIFIED)
```c
uint8_t zone_at(int x, int y) {                 /* args pushed x = [0xB6], y = [0xB8] */
    const uint8_t *row = &B(W(0x3A96 + 2*y));   /* run-length pairs {zone, length} */
    int dx = 0, i = 0; uint8_t z;
    while (dx < 319) { z = row[i]; dx += row[i+1]; i += 2; if (x < dx) break; }
    if (z == 0xFE) { msgbox(W(0x57BC) /* = DS:5676 "Gaul was divided into three parts... indomitable village" */); z = 0xFF; }
    return z;                                   /* 0-26 provinces, 27-36 seas, 0xFF none */
}
```
Check 3: every zone anchor DS:1AB8[z] hits its own zone (37/37); 0xFE covers exactly (50,74), (51,75), (52..55,76) on Armorica's coast.
2,999 pixels are 0xFF. **The port must use this table instead of game.c's two hand-made rectangles (g_regions).**

### Map builder 0x494A(z) (DECODED; the port's map frame is 292 px off the capture, all of it legion icon + cursor per HANDOFF)
```c
void map_build(uint8_t z) {
    cursor_push(0);                                         /* 0x1D55(0) hide */
    if (!W(0x254)) load MAP image from archive 5 (MMAP) via 0x13A8;
    mouse_range(0..311, 0..185); if (z != 0xFF) mouse_warp(anchor[z]);
    select_work_buffer();                                   /* 0x67E4 */
    blit(map image [0xC09E] at DS:56FE);                    /* 0x8790 */
    if (span_stack_alloc())                                 /* 0x22CC (500 entries) */
        for (k = 0; k < 27; k++) if (B(0x5285+32*k)) flood_fill(9, seed DS:3C52[k]);   /* 0x22EA; 0x22E2 frees */
    year_button(W(0x634C));                                 /* 0x4A24 */
    end_turn_button_draw();                                 /* 0x4AA6 */
    fleet_icons();                                          /* 0x2AF8: records 0-2 sprite [0xC0F0], 3-5 [0xC0F4] */
    legion_icons();                                         /* 0x423A */
    army_icons();                                           /* 0x53BC: each active army record (C112.., +8 != 0) at anchor[+1],
                                                               sprite [0xC110] on land, [0xC0F4] at sea  -- NOT in the port */
    present(); copy_rect_to_screen(0, 0, 319, 199);         /* 0x680F, 0x6973 */
    music(1);                                               /* 0x5D29(1) map music */
    cursor_pop();                                           /* 0x1D8A */
}
```
Buttons (VERIFIED, check 4 against scr_newgame_map, 0 missing / 0 extra in the button interiors):
- End Turn: box 0x3134(colour W(0x6346), DS:3C42 = (227,2)-(275,18)), "End Turn" (DS:3CC6) left-aligned via 0x899C at (x0+5, y0+4) =
  (232,6), colour 15 (glyph rows start at y + yoff, so ink from y 7).
- Year: box DS:3C4A = (277,2)-(317,18); `v = 275 - turn`; text = itoa(|v|) + (v > 0 ? " BC" (DS:3CBE) : " AD" (DS:3CC2)), centred by
  0x8BE4 at x = (x0+x1)/2 = 297, y = y0+4, colour 15. "275 BC" lands at x 283. **Quirk: turn 275 shows "0 AD".**
- 0x8BE4 centring rule (DECODED): `x = cx - (sum over chars of (width + DS:9B5E spacing)) / 2`, i.e. the trailing spacing counts. The
  port's ui_text_centered uses `(sum - 1) / 2`, which is 1 px to the right whenever the summed advance is even. Fix it before porting more
  centred texts (both agree for "275 BC": advance 29).

## 3. Game Control Menu 0x1A8B (year button or Esc) (DECODED; menu text VERIFIED)

Record DS:05A6, box (81,79)-(251,197), title "Game Control Menu", 7 items. Item text VERIFIED (check 5: 871 px, 0 missing, 0 extra on
scr_game_control_menu) at `(menu.x0 + item.x0 + 11, menu.y0 + item.y0)` in colour menu+8 (= 0); the engine draws an item whose hotkey high
byte != 0 in colour 6 instead (DECODED, 0x5665).
```c
int game_control_menu(void) {                   /* returns -1 = quit */
    int quit = 0; cursor_push(0); save_rect(0,0,319,199);           /* 0x691B */
    menu_draw(DS:05A6, 2); if (mouse) cursor_show();                /* 0x5614(2 = frame 0x3134 + items) */
    switch (menu_run(DS:05A6, 0)) {                                 /* 0x54C8: index, 0xFF = Esc */
    case 0: save_game(); break;                                     /* 0x762D */
    case 1: load_game(); break;                                     /* 0x76C2 */
    case 2: new_game(); break;                                      /* 0x752E */
    case 3: copy_rect_to_screen(menu box);                          /* About */
            strcpy(buf AC60, DS:05EE "CENTURION\nDefender of Rome\nby Kellyn Beck and Bits of Magic\n\nCopyright..."); 
            strcat(buf, DS:046E "Version 1.00A 5/25/90"); msgbox(buf); break;
    case 4: difficulty_menu(); break;                               /* inline 0x1B32 */
    case 5: if (yes_no(DS:0672 "Really Quit?")) quit = -1; break;
    default: break;                                                 /* 6 Continue Playing, 0xFF Esc */
    }
    cursor_pop(); return quit;                  /* the caller redraws the map with 0x494A(0xFF) */
}
void difficulty_menu(void) {                    /* DS:05B8 "Difficulty Level Menu": Galley Slave, Legionnaire, Senator, Emperor, Fine Tuning, Continue */
    menu_draw(DS:05B8, 2);
    for (;;) {
        sel = B(0x48E); for (i = 1; i < 4; i++) if (B(0x48E+i) != B(0x48E)) sel = 4;   /* mixed levels -> "Fine Tuning" preselected */
        menu_preselect(DS:05B8, sel, -1);       /* 0x585B */
        c = menu_run(DS:05B8, sel);
        if (c == 4) { fine_tuning(); menu_draw(DS:05B8, 2); continue; }
        if (c < 4) for (i = 0; i < 4; i++) B(0x48E+i) = c;
        return;                                 /* 0..3, 5 Continue, 0xFF */
    }
}
void fine_tuning(void) {                        /* 0x19BC, DS:05CA "Fine Tuning Menu": Land Battle, Racing Chariots, Gladiatorial Combat, Naval Battle, Continue */
    menu_draw(DS:05CA, 2); show_levels();       /* 0x1A5E: name DS:0484[B(0x48E+i)] at (195, 102 + 10*i), colour 15, via 0x899C */
    for (m = 0;;) {
        m = menu_run(DS:05CA, m); if (m == 4 || m == 0xFF) return;
        strcpy(A64C, item_text(DS:05CA, m)); strcat(A64C, DS:0426 " Difficulty Menu");   /* title of DS:05DC */
        menu_draw(DS:05DC, 2); menu_preselect(DS:05DC, B(0x48E+m), -1);
        c = menu_run(DS:05DC, B(0x48E+m)); if (c < 4) B(0x48E+m) = c;
        menu_draw(DS:05CA, 2); show_levels();
    }
}
```
DS:048E[0] land battle, [1] chariot race, [2] gladiators, [3] naval (LOGIC_NOTES). Saved (block DS:048E, 4 bytes).

### Save / Load / New Game (DECODED; New Game data VERIFIED)
- Slots: menu record **DS:5BCC**, box (30,40)-(290,159), 9 items "1 -".."9 -" (hotkeys '1'..'9'), title pointer DS:5BD8 set to
  DS:5B18 "Select a Save Position" (save) or DS:5B4A "Select a Game to Load" (load). Slot s (0-8) = file **"s.csv"** (DS:5B12, byte 0 =
  '0'+s), so menu slot "1" = 0.CSV, Erik's "erik" save 3.CSV = slot "4". The list (0x7996("*.csv") + 0x7A54) reads each file's header,
  accepts it only if the first 4 bytes equal "10A" (DS:5B0E, strcmp), and copies its 30-byte description to DS:A6C0 + 49*s; the 9
  descriptions are drawn at (65, 63 + 10*s) with 0x899C, colour 0.
- **Save 0x762D:** disk prompt 0x7B1E(DS:5C4A "Please Insert Your Save Game Diskette In Drive ") (only matters on floppies), s = 0x7784();
  if s != 0xFF: 0x7852 asks "Enter a Game Description" (DS:5B30; empty → "Untitled" DS:5CDB) into DS:A8F0 (cleared 0x30 bytes);
  create "s.csv" (0xA1B9), pack (0x75A3: the 24 {addr,len} blocks of DS:5BDE into DS:AC60, 1780 bytes), write (0x9EC5), close; errors
  DS:5C51 "Save Failed\nCould not write file" / DS:5C72 "...create file". Then 0x7B1E(DS:5C4C "Please Replace Game Disk").
  = port state_save (state.c, round trip byte-identical, LOGIC_NOTES).
- **Load 0x76C2:** s = 0x78F0(); empty description → DS:5C95 "No saved game\nat that position"; else 0x7737("s.csv"): open, read 1780
  bytes to AC60, close, unpack 0x75E8 (scatter the blocks); open error DS:5CB4. No other state is reset (DS:00C8/CA dice keep running).
- **New Game 0x752E:** opens archive 3 (CENT), reads member NEWGAME.LZW through 0xF0C into a buffer of 1781 bytes, copies it to AC60 and
  unpacks it with 0x75E8. **VERIFIED (check 1): NEWGAME.LZW = u32 1780 + LZSS (tools/extract_data.lzss_decompress), and the result equals
  fresh_275BC.csv in every byte except the description ("New Game" vs "fresh").** So New Game and the startup state are the same game.
- After any of them main redraws with 0x494A(0xFF). Archive numbers for 0xF7B (names at DS:029E, obfuscated in the static DS, read from
  t10_in.bin): 0 mrace, 1 batlvga, 2 batl, 3 cent, 4 minit, 5 mmap, 6 mglad, 7 mnego.

## 4. Province screens

### Provincial screen 0x6CB8(z) (z = 1..26, Roman) (DECODED; menu VERIFIED, check 5: DS:5790 items 660 px, 0 missing, 0 extra on scr_narbonensis_province)
```c
void province_screen(int z) {
    int leave = 0, redraw_menu = -1, sel = 0;
    wait_release(); cursor_push(0);
    strips_load(z);                                 /* 0x708A: background / foreground strips */
    W(0x9AFA) = W(0x6344); fill_rect(0, 65, 319, 199);   /* 0x7E74(0xC7,0x13F,0x41,0) */
    W(0x9B69) = 0; text_centred(160, 65, name DS:5222[z]);   /* 0x8BE4 */
    panel_frame(); province_panel(z);               /* 0x3512, 0x6F0C */
    present(); copy_rect_to_screen(0,0,319,199);
    while (!leave) {
        if (redraw_menu) {                          /* 0x6D32 */
            item4_hotkey_hi = (coastal_sea(z) == 0xFF) ? 0xFF : 0;   /* 0x2A07: B(0x50EC+8z) is a sea zone (27..36) ?
                                                       it : 0xFF.  Written to byte [W(0x579A) + 0x33] = DS:576F = Build Fleet hotkey hi */
            menu_draw(DS:5790, 1); redraw_menu = 0; /* 1 = items only (the frame 0x30E6 instead of 0x3134) */
        }
        sel = menu_run(DS:5790, sel);
        switch (sel) {
        case 0:    leave = legions_cmd(); redraw_menu = -1; break;    /* 0x4065, returns != 0 when a legion left */
        case 1:    plunder(z); break;                                  /* inline 0x6DB2, below */
        case 2:    tribute_menu(z); redraw_menu = -1; break;           /* 0x7CE0 */
        case 3:    provincial_games(z); redraw_menu = -1; break;       /* 0x3342 */
        case 4:    build_fleet(z); redraw_menu = -1; break;            /* 0x2458 (handles "There is no ocean here" itself) */
        case 5:    province_list(); menu_preselect(DS:5790, 5, 0); break;   /* 0x69CC */
        case 6: case 0xFF: leave = -1; break;                          /* Continue, Esc */
        }
    }
    map_build(z); cursor_pop();                     /* back on the map, pointer warped to z */
}
```
The greyed item is only drawn grey: 0x54C8 still accepts it (mouse 0x552B and hotkey 0x5593 don't look at the high byte), so Build Fleet
in an inland province runs 0x2458, which prints DS:1274 "There is no ocean here". DECODED. Coastal by that rule (first neighbour is a sea):
all provinces except 5 Parthia, 11 Alpes, 18 Arabia, 19 Mesopotamia (data, DS:50EC).

**Plunder (inline 0x6DB2), full pseudo-C (DECODED):**
```c
for (l = 0; l < 12; l++) if (B(0x1918+20*l+0x0D) != 0xFF && B(0x1918+20*l+0x0C) == z) break;
if (l == 12) { msgbox(DS:57CC "You must have a legion\nthere to plunder"); menu_preselect(DS:5790, 1, 0); continue; }
menu_highlight(DS:5790);                                   /* 0x55FF */
if (yes_no(DS:57BE "Are you sure?")) {
    P = 0x5270 + 32*z;
    if (B(P+0x15) == 1) { B(0x634E) >>= 1; B(P+0x0E) = min(SB(P+0x0E), 50) * 2; }   /* Occupied */
    else                { B(0x634E) >>= 2; B(P+0x0E) = min(SB(P+0x0E), 25) * 4; }
    t = B(P+0x14);  W(0x634A) += t;  DS:C09A (u32) += t;    /* treasury, plunder total */
    if (t == 0) msgbox(DS:5834 "There's nothing\nto plunder");
    else { B(P+0x0F) >>= 1; B(P+0x14) = 0;
           msgbox("You plunder the province for " (DS:57F4) + itoa(t) + "\ntalents.  The natives are upset." (DS:5812)); }
    panel_frame(); province_panel(z);
}
menu_unhighlight(DS:5790);                                 /* 0x55EA */
menu_preselect(DS:5790, 1, 0);
```
(min() is signed: `cmp al, 0x32; jle`.) Only the first legion found counts; it is not marked as having acted.

**Provincial games 0x3342(z) (DECODED):** costs DS:15D4 = {15, 30, 40, 50}; menu DS:15B0 "Provincal Games" (Bestiarii, Gladiator Show,
Racing Chariots, Extravaganza, Continue).
```c
if (W(0x634A) < 15) { msgbox(W(0x15D8) "You don't have enough Talents..."); return; }
u = SB(P+0x0E);  c = menu_run(DS:15B0, 0);  if (c > 3) return;
menu_preselect(DS:15B0, c, 0);
if (cost[c] > W(0x634A)) { msgbox(W(0x15D8)); return; }
switch (c) { case 0: u -= 10; break; case 1: u = u*3/4; break; case 2: u = u/2; break; case 3: u = u/4; break; }   /* signed idiv */
W(0x634A) -= cost[c]; msgbox(W(0x15DA) "The Roman governor stages a sp..."); B(P+0x0E) = max(u, 1);
W(0xC100)++;                                   /* games held (victory statistics) */
if (SB(0xED5) > 10) B(0xED5) = 10;              /* the "governor wants games" crisis chance */
if (z == B(0xED6)) { W(0xEDA) = 0; B(0xED6) = 0; }   /* that crisis is satisfied */
panel_frame(); province_panel(z);              /* then returns: one show per command */
```
No module is run here: the provincial games are pure bookkeeping.

### Rome screen 0x7366 (zone 0) (DECODED; menu VERIFIED, see section 0)
```c
void rome_screen(void) {
    int leave = 0, first = -1, sel = 0; cursor_push(0); wait_release();
draw: strips_load(0); fill_rect(0,65,319,199) in W(0x6344); rome_extra();          /* 0x7411: 0x708A(0), 0x7E74, 0x71BB */
      W(0x9B69) = 0; text_centred(160, 65, DS:5A42 "ROME"); panel_frame(); province_panel(0); present();
    while (!leave) {
        if (first) menu_draw(DS:5A30, 1); first = -1;
        switch (sel = menu_run(DS:5A30, sel)) {
        case 0: leave = legions_cmd(); break;              /* 0x4065 */
        case 1: tribute_menu(0); break;                    /* 0x7CE0 */
        case 2: if (rome_games()) goto draw; break;        /* 0x3457: the module ran, redraw the whole screen */
        case 3: build_fleet(0); break;                     /* 0x2458 */
        case 4: tribute_policy(); break;                   /* 0x7D42 */
        case 5: province_list(); break;                    /* 0x69CC */
        case 6: case 0xFF: leave = -1; first = 0; break;
        }
    }
    map_build(0); cursor_pop();
}
int rome_games(void) {                         /* 0x3457, menu DS:15C2 "What Kind of Games?": Racing Chariots, Gladiator Show, Continue */
    c = menu_run(DS:15C2, 0);
    if (c == 0) {                              /* chariots */
        if (W(0x154E) == W(0x634C)) msgbox(DS:15DC "Only One Race Per Turn");
        else if (W(0x634A) >= 5) { disk_check(); ovl_B0AA(); return -1; }        /* section 4 D08A */
        else msgbox(DS:15F3 "You cannot afford to\nenter a chariot race.");
    } else if (c == 1) {                       /* gladiators */
        if (W(0x154A) == 0) {                  /* no amphitheatre */
            if (W(0x634A) < 50) { msgbox(DS:161E "You need an amphitheatre\nand cannot afford one."); goto out; }
            menu_highlight(DS:15C2);
            if (!yes_no(DS:164E "You need an amphitheatre.\nSpend 50 Talents?")) { menu_unhighlight(DS:15C2); goto out; }
            W(0x634A) -= 50; W(0x15D2) = W(0x154A) = 0xFFFF;
        } else if (W(0x154C) == W(0x634C)) { msgbox(DS:167A "Only One Show Per Turn"); goto out; }
        disk_check(); ovl_B0B6(); return -1;   /* section 3 B0E0 */
    }
out: menu_preselect(DS:15C2, c, 0); return 0;
}
```
(DS:154A/154C/154E are saved blocks: amphitheatre flag, last show turn, last race turn — GUESS for 154E being written by the race module.)

### Other screens reached from the map (DECODED outline)
- **zone_info 0x720B(z)**: one message box: "Marauding Army\nof " (DS:58E3) + name of the first active army record in z + "\nin " (DS:58F6),
  else for a sea zone the first enemy fleet 3-5 there: fleet name DS:1100[f] + "\nin " (DS:58FB); then the zone name DS:5222[z].
- **Legions 0x4065** (from either screen): 0x4710 menu; raising legions (limit by rank DS:595E[rank] = {1,2,3,4,6,8,10,12,12}, "You
  cannot command any more legions at this rank" DS:1CBA, "All legions have been raised" DS:1C9C), Cavalry (rank >= 2), Consular Army
  (rank >= 6), "Local Legion List" (DS:1D58 via 0x3CB0), select 0x3BAF, then 0x3EA3 info + **0x382A move**; it calls 0x494A itself and
  returns != 0 so the province screen closes. NOT PORTED (game.c has a placeholder).
- **Legion move 0x382A(l)**: markers at the adjacent zones (DS:50EC), click a target; sea zone → board a fleet (0x3AE4 "N Legionnaires must
  be left behind.\nBoard the Legion?"); on entering z: legion +0x0E = 1 (acted), 0x4B3F(l, z) fights every army record in z (B06E); if
  it survives and z is a non-Roman province: **0x5F82(l, z)** (below); a won conquest scores +30 for Aegyptus (z = 2), +15 for Carthago,
  Macedonia, Gaul, Parthia (1, 3, 4, 5), +5 otherwise (0x7330); the legion's zone becomes z.
- **0x5F82(l, z)**: if province +0x17 != 0 (a revolting province; GUESS from the text) → message legion name + DS:3FD5 " arrives in " +
  name + DS:3FE2 "\nto quell the rebellion, and m..." then battle B06E(0xFF, l, z); a win sets +0x15 = 1 (Occupied), +0x19 = 0, +0x12 =
  the tribute policy B(0x5FC0), and 0x53F8(z) frees enemy fleets there. Otherwise → the **negotiation screen** (archive 7 MNEGO, 0x6570,
  the code around 0x62C2; uses rank vs DS:4092[...] and B(0x634E) vs DS:40AE[...]); a failed negotiation can still end in a battle (0x64BB).
  NOT PORTED; this is the biggest missing piece after Legions.
- **Fleets**: select 0x2C6F ("Select Fleet" DS:1390, "Fleet has already\nacted this turn"), info 0x2F5A, **move 0x2B40(f)**: markers at the
  adjacent sea zones, click one: fleet +2 = 1 (acted), disk check, 0x4ADA(f, z) fights every army record at sea in z (0x531F(army) then naval battle
  B0C2 = section 5 B12A(6, f); B12A takes (enemy fleet record, Roman fleet), so record 6 is the army made into a fleet: GUESS; result bit 2 = stop), then B09E(f, z) fights enemy fleets 3-5 there; if it returns 1 the fleet's zone (and its legion's, fleet +3) becomes z.
  **Build Fleet 0x2458(z)**: "There is no ocean here", fleets per rank DS:5968[rank] = {0,0,1,1,2,2,3,3,0} ("You cannot command any fleets
  at this rank" DS:1218), "The enemy fleet\nprevents any shipbuilding" (0x2687), purchase screen 0x26D0/0x2852 ("Cost:", "Available
  talents:", "Enter to Buy ESC to Cancel"). NOT PORTED. Naval battle (section 5) NOT PORTED.
- **Lists**: 0x69CC province list (layout DECODED in HANDOFF; placeholder in the port), 0x4280 legion list, 0x2FD6 fleet list (wait 0x35F9).
- **Tribute 0x7CE0 / Tribute Policy 0x7D42**: DECODED in HANDOFF; the port has Tribute approximately (writes its own g_prov_level, not DS).

## 5. Score, rank and the end of the game (DECODED; data from the static DS)

```c
void score_add(int n) {                         /* 0x7330 (PORTED, endturn.c) */
    W(0x594A) = n >= 0 ? min(W(0x594A) + n, 256) : max(W(0x594A) - |n|, 0); }
void rank_update(void) {                        /* 0x72DE, only called from End Turn (0x0CD2) (PORTED) */
    for (r = 0; r < 8 && SW(0x594E + 2*r) < score; r++) ;   /* thresholds {10,30,60,90,120,150,190,255} */
    if (B(0x5971) < r) B(0x5971) = r;                       /* rank never drops */
    if (B(0x5971) == 8 && any province has status 0) B(0x5971) = 7;
}
```
Rank names DS:5972[r]: Officer, Centurion, Tribune, Legatus, General, Praetor, Consul, Proconsul, **Caesar (8)**.
- **Victory**: rank 8 = score 256 (max) **and** all 27 provinces Roman, seen by the main loop after End Turn → disk check, section 4 DF58:
  music off (0x5D73(0)), cursor hidden, DF74 builds "THE MAKING OF A CONQUEROR\nIn becoming Caesar, you successfully negotiated N
  alliance(s), won N battles and lost N battles, in which N legionnaires were killed and N enemy soldiers were slain. You won N fleet
  battles..." from the statistics block DS:C0F8-C107 (C106 alliances, C0FA battles won, C0F8 battles fought, ...; a saved block), then
  BDA4, BF6A(1), C2C2 (GUESS: the triumph pictures/animation), then the credits (B9A1). Then exit(0).
- **Defeat**: after any command, no province with status != 0 → "Sic Transit Gloria Mundi\nThe Roman Empire Has Fallen" (DS:017F) → exit(0).
  There is no turn limit and no other game-over test in the map loop.
- **Quit**: Q key or Game Control "Quit to DOS", each with "Really Quit?" → exit(0). No autosave.

## 6. Every screen / command, entry and port status

| screen / command | entry | port today |
|---|---|---|
| startup state | static DS + section 1 B397(3..5) | state_init (static DS) — needs B397 (or load NEWGAME.LZW) |
| copy protection | section 1 B466 | skipped (decision) |
| intro / title | section 1 B694 | no |
| map loop + dispatch | 0x07EF-0x09B9 | game.c prototype (2 hard-coded click boxes, own turn counter, no DS) |
| zone hit test + Asterix village | 0x6C67 | no (table VERIFIED here) |
| map builder | 0x494A | game.c draw_map_full, from the CSV file, not g_ds; enemy army icons 0x53BC missing |
| End Turn button | 0x09BA | logic PORTED (endturn.c/enemy.c/events.c/battle.c); not wired to the map loop (game.c does g_turn++) |
| year button | 0x4A24 | yes (draw), "AD" branch not checked |
| Game Control Menu | 0x1A8B, DS:05A6 | menu drawn (VERIFIED text); actions no |
| Save / Load | 0x762D / 0x76C2, slots DS:5BCC | state.c save/load (byte-exact); the slot screens and "10A" check no |
| New Game | 0x752E (NEWGAME.LZW) | no |
| About | inline 0x1AFB | no |
| Difficulty / Fine Tuning | 0x1B32 / 0x19BC | no |
| Quit (Q / menu) | 0x0960 / 0x1BC9 | window close only |
| Rome screen | 0x7366, DS:5A30 | approximate (swap_plunder exception) |
| provincial screen | 0x6CB8, DS:5790 | approximate |
| foreign zone info | 0x720B | no |
| Legions | 0x4065 (+0x382A move) | placeholder |
| Plunder | inline 0x6DB2 | no |
| Tribute / Tribute Policy | 0x7CE0 / 0x7D42 | Tribute approximate (not in g_ds), Policy no |
| Hold Games (province) | 0x3342 | no |
| Hold Games (Rome) | 0x3457 → B0AA race / B0B6 gladiators | no (modules not ported) |
| Build Fleet | 0x2458 | no |
| See Province List / P | 0x69CC | placeholder |
| fleet click: select/info/move | 0x2C6F / 0x2F5A / 0x2B40 | no |
| right click: legion select/info/move | 0x3BAF / 0x3EA3 / 0x382A | no |
| F / L lists | 0x2FD6 / 0x4280 | no |
| conquest battle / negotiation | 0x5F82 (B06E / MNEGO 0x6570) | battle PORTED; negotiation no |
| enemy army battles in End Turn | 0x4B3F, 0x501E, 0x508D | PORTED |
| naval battle | section 5 (B0C2, B09E) | no |
| chariot race / gladiators | section 4 D08A / section 3 B0E0 | no |
| victory sequence + credits | section 4 DF58, section 1 B9A1 | no |
| "Sic Transit" defeat | 0x0986 | no |

## 7. Porting plan (order, dependencies, checks)

1. **One game state: g_ds.** Make game.c read everything from g_ds (state_init + state_load) instead of re-parsing CSV files (draw_map_full,
   load_province_csv, init_legions, load_turn, g_prov_level). Startup = state_init, then B397(3..5) (pseudo-C in section 1) — or simply
   unpack NEWGAME.LZW. *Check:* checks.py 1/2 (already 0 bytes); in C: build the start state both ways and compare with fresh_275BC.csv
   (0 bytes outside 0x04-0x21); `--dump-frame map` must stay at 292 px.
2. **Map loop skeleton = section 2 pseudo-C**, with the real hit test 0x6C67 (incl. the 0xFE message), left/right buttons, the key table,
   the "Sic Transit" and rank-8 tests after every command, and exit. Input: the port's SDL events feed the same wait_input() contract
   (1 = left, 2 = right, 0 = key). *Check:* checks.py 3 in C (37/37 anchors, 6 Asterix pixels), and a scripted headless run (click
   list) that prints the dispatched handler per click.
3. **End Turn wired**: button + 'E' → end_turn() (already byte-exact), battles through battle_play, messages through the message box
   (59D4), map_build(0xFF) at the point where 0x09BA calls it, then events. *Check:* the existing cmp_endturn runs stay at 0 bytes;
   `--dump-frame map` after one End Turn vs turn2_274BC_map.png (year 274 BC).
4. **Game Control menu**: Save/Load slot screens (DS:5BCC, "10A" check, s.csv naming, description input), New Game (NEWGAME.LZW),
   About, Difficulty/Fine Tuning, Quit. *Check:* save from the port after loading 3.CSV = 3.CSV except nothing (round trip); New Game
   state = fresh_275BC.csv; menu text pixels as check 5; a DOSBox capture of "Select a Save Position" would verify the slot screen.
5. **Province screens on DS**: 0x6CB8 / 0x7366 with their own records (drop swap_plunder), Build Fleet greying (0x2A07), then the pure-logic
   commands in this order: Plunder, provincial Games 0x3342, Tribute 0x7CE0 / Policy 0x7D42 writing DS, Province List 0x69CC, zone_info
   0x720B. *Check (logic):* DOSBox DS dump before/after one command (same method as End Turn: BP at the command entry and at its
   return), compare all saved blocks — 0 bytes. *Check (screens):* the province captures (Rome 707 / Narbonensis 763 px today).
6. **Legions 0x4065 + move 0x382A + 0x4B3F + conquest 0x5F82** (battle already ported; the negotiation screen 0x6570.. is new and needs
   MNEGO graphics and captures). *Check:* DOSBox before/after a move into an enemy army's zone and into a foreign province (battle choices
   via --choices), 0 bytes.
7. **Fleets**: select/move 0x2C6F/0x2B40, Build Fleet 0x2458, then the naval battle (section 5) — a project of its own like the land battle.
8. **Rome's games**: race (section 4 D08A) and gladiators (section 3) are separate modules; until ported, the port can say "not ported" and
   leave DS untouched (the original's bookkeeping lives inside the modules: GUESS).
9. **End of game**: Sic Transit (trivial), victory text DF74 (strings DS:8CD0..; statistics block C0F8), credits (DS:6A6E pairs, B9A1/B9D3).

Dependencies: 1 → 2 → 3/4/5 (independent of each other); 6 needs 5's screens and the message box; 7 needs 6's move framework;
8/9 independent. Keep `--original` for byte-exact comparisons as for End Turn.

## Open points
- What the negotiation screen (0x6570 and 0x6043-0x64FE) decides and how (rank table DS:4092, mood table DS:40AE): not read in detail.
- The meaning of province +0x17 (taken as "revolting" from the battle message text) and of DS:154E (last race turn) are GUESSES.
- BDA4 / BF6A / C2C2 in the victory sequence were not read.
- The slot screen layout (title position (160,46) via 0x8BE4, descriptions at (65, 63+10s)) is DECODED only; no capture of it exists.
- The year "AD" branch and 0x8BE4's centring for even advances are DECODED only (no capture at turn >= 275 or of such a text).

## Corrections (verifier)

Verifier, 2026-11-09. Own listing of disasm/EXE_RUNTIME.bin and OVL0 section 1 (llvm-objdump), own scripts in
`<scratch>/wf/ver_gl/` (btn.py, menus.py; the hit-test, B397 and NEWGAME checks were run inline). Re-checked and **confirmed**: the map
loop 0x07EF-0x09B9 (every branch and key of section 2), 0x6C67 (37/37 anchors, the same 6 Asterix pixels, 2,999 0xFF pixels, static
and t10 DS alike), the End Turn / year text (own font renderer from the t10 font pointers DS:9B56/58/5A, height 8, spacing B(0x9B5E)=1:
"End Turn" 81 px at (232,6) and "275 BC" 64 px at x 283, both colour 15, 0 missing / 0 extra in the interiors; x +-1 or +-2 gives 43-57
missing), the 0x8BE4 rule `x = cx - (sum(w+1) >> 1)` (0x8C58 counts the trailing spacing), the 0x5614 item rule (Game Control 871 px,
Provincial 660, Rome 715, each 0 missing / 0 extra; the notes' "733 / 18 missing" counts capture pixels and the 18 are the cursor),
static DS + B397(3..5) = fresh_275BC.csv (5 differing bytes, CSV 0x04-0x08, the description), NEWGAME.LZW = fresh (8 bytes, 0x04-0x0B),
the thunk table, rank 0x72DE / score 0x7330, Plunder arithmetic, 0x3342, 0x3457, 0x7366, and the Game Control jump table at 0x1ADD
(0x1AE9 save, 0x1AEF load, 0x1AF5 new, 0x1AFB about, 0x1B32 difficulty, 0x1BC9 quit). What is wrong or missing:

1. **Fleet limit table DS:5968 is {0,0,1,1,2,2,3,3,3}, not {...,3,0}** (static DS and t10_in.bin alike; read by 0x2458 at 0x2518/0x254D).
   Caesar (rank 8) may command 3 fleets. That matters only for a loaded save at rank 8, but the port should copy the table, not the note.
2. **Slot list 0x7A54 reads a 48-byte (0x30) description, not 30 bytes**: `read(fd, DS:A6C0 + 49*s, 0x30)` at 0x7AD2; a failed read
   sets the slot to "" (DS:5CEB). The "10A" test is `strcmp(DS:5B0E, DS:A922)` on the first 4 bytes read to A922 (0x7A9C-0x7AB6), and the
   slot is `name[0] - '0'`, kept only if 0..8 (0x7ABF-0x7ACC). **Quirk:** the clear loop at 0x7A5E-0x7A75 runs 9 times but indexes
   with the uninitialised local [bp-2] instead of the counter si, so it clears at most one (random) slot. A slot whose file has gone
   keeps its old description in DS:A6C0, and Load would then try to open a missing file (error DS:5CB4). DECODED. In the port: clear all 9.
3. **Plunder's "You must have a legion there to plunder" depends on a flag that is set once per screen visit.** The test at 0x6EA6 is
   `if (W(bp-8) == 0) msgbox(DS:57CC)`, where bp-8 is zeroed only at the entry of 0x6CB8 (0x6CE2) and set to -1 after the first
   yes/no (0x6EA1, whether the answer was yes or no). After one Plunder attempt in a visit, a later Plunder with no legion in z does
   nothing and only re-selects item 1. Rarely reachable (a legion that leaves closes the screen), but the pseudo-C should be:
   ```c
   if (l == 12) { if (!asked) msgbox(DS:57CC); }  else { ...yes/no as given...; asked = -1; }
   menu_preselect(DS:5790, 1, 0);                 /* asked = 0 at 0x6CE2, once per province_screen() */
   ```
4. **Drawing calls left out of the pseudo-C** (matter for frame checks): provincial games 0x3342 draws its menu first,
   `menu_draw(DS:15B0, 1)` (0x337E-0x3386), before `menu_run(DS:15B0, 0)`; rome_games 0x3457 starts with `cursor_push(0)` and
   `menu_draw(DS:15C2, 1)` (0x345B-0x3469) and ends with `cursor_pop()` 0x1D8A (0x350A) on every path. In rome_screen 0x7366 the
   "first" flag is set to -1 before every menu_run (0x7396), so DS:5A30 is redrawn after every command (the pseudo-C is equivalent, but
   read it as "always redraw").
5. Small points: rank_update's "rank never drops" is true except that the same call can lower 8 to 7 (0x730D-0x7327, as the pseudo-C
   shows). The main loop's rank-8 test (0x0997) runs after **every** command, not only after End Turn (it can only become 8 inside End
   Turn, since 0x72DE has one caller, 0x0CD2, but a loaded save at rank 8 triggers the victory at once on the next click). Init also writes
   W(0x6344)=0x74, W(0x6346)=0x1A, W(0x6348)=0x10C (section 1 B226-B232, only when W(0x9B26) == 2, the display mode; GUESS: VGA). They are the panel, button and text colours
   (the static DS has 3 and 7 for 0x6344/0x6346; t10_in.bin has 116 and 26), so the port must take them from a runtime DS, not the static one. They are not in the saved blocks.
