# Negotiation screen (EXE 6570 and what it calls; Cleopatra scenes 4852)

Written 2026-11-09 by Claude (workflow agent "screen"). All addresses are EXE segment 0823 offsets or DS (1686) offsets.
Calls are Pascal (callee pops); arguments are listed **in push order** (first pushed = leftmost), which is also the
order the port's helpers use (`fill7e74(x0, y0, x1, y1)`, `8E62(desc, x, y)`, `8BE4(str, cx, y)`, `899C(str, x, y)`).
Labels: **D** = decoded from the disassembly (fresh llvm-objdump of `EXE_RUNTIME.bin`), **V** = verified by running
or comparing, **G** = guess. There is no capture of this screen (tools/dosbox/results has none), so the drawing is D;
the sprite geometry is V only in the sense stated in section 9.

Run-time values used below: [6344] = 0x74 (screen background, RGB 208,176,144), [6346] = 0x1A (box fill, 160,144,128),
[6348] = 0x010C (the "erase" text colour, see 2.3), [9B26] = 2 (VGA), [9B5C] = 8 / [9B5D] = 3 (font), [3E40] = DS:3E2E
"Click to continue" (with a mouse; DS:3E16 "Hit any key to continue" without, LEGIONS_NOTES correction 1). Static OVL0 has
other values for 6344/6346/6348 (3/7/7): **hard-code the run-time ones** (D, both images dumped).

The label split in the task (0x6570, 0x6589, 0x6608, 0x6646, 0x66BB) is one function: 6589/6608/6646/66BB are jump
targets inside 6570 (`func_` labels in EXE_RUNTIME.clean.asm are only cut points) (D).

## 0. Short answer

- 6570 `nego_screen(z, &textbuf)` returns -1 (ok) or 0 (archive 7 could not be opened). It opens MNEGO, composes the
  province strip and the leader portrait in the **work buffer**, copies the strip to the **screen**, draws everything
  else straight on the screen, shows the **scout report** (7B56, with a "Click to continue" wait), then copies the
  portrait to the screen, draws its drop shadow and confines the mouse to the reply-menu box.
- The reply menu DS:4080 is drawn **each round** of the dialogue in the work buffer (box fill, TAB frame sprites 4..7,
  `5614(4080, 0)`) and copied to the screen.
- All speech goes through **5AE4 `say(str, colour, quote)`**, a scrolling word-wrapped log in the right box (x 120,
  lines 10 px apart from y 86, scrolls when a line would pass y 190).
- No teardown in 5F82: it frees the buffers and pops the cursor; the caller 382A redraws the map with 494A.
- File names use **'@' + z**, not 'A' + z: zone 1 Carthago = LEADERA / TEXTA / NEGFGA, zone 2 Aegyptus = B, ...,
  zone 26 Sarmatia = Z (D; V by content: SCOUT.LZW string 2 is Alexandria/Cleopatra, LEADERB is the Egyptian queen).

## 1. Files and buffers (D)

| what | name (DS) | loader | where it goes |
|---|---|---|---|
| archive | index 7 = MNEGO.DIR/.DAT (`0F7B(7, &arch)`, name table DS:029E) | 0F7B | local `arch` (8 bytes) in 6570's frame, closed by `11A9(&arch)` at the end of 6570 |
| province strip | ROME.CBM / SKY0..3 / NEGBG? / NEGFG? (DS:5881, 58DA, 58CE, 58C2) | 70BA (ported: province.c `strips_draw70ba`) | work buffer rows 0..64 |
| portrait | "leaderA.cbm" DS:4EEE, letter at DS:4EF4 := '@'+z | 1273 into a 14-byte CBM descriptor in 6570's frame (bp-1E), drawn by 8E62, freed by 86A0 at once | work buffer (8,79)-(103,144) |
| leader texts | "textA.lzw" DS:4EE4, letter at DS:4EE8 := '@'+z | `11C4(name, &arch, buf)` into **the caller's** 8-byte buffer (5F82 bp-1E): +0 near data pointer, +2 segment ([C0C8] = DS), +4 length, +6 dir entry | kept until 5F82 frees it (`198F(&buf)`) |
| reply/box frame | "tab" DS:4EE0 -> "tabHDR.LZW" + "tabDAT.LZW" (suffixes DS:0284 "HDR.LZW", DS:028C "DAT.LZW") | `163B(name, &arch, 0)` | **[C1EE]** = DS pointer to 8 descriptors; freed at 5F82 64EF `19A0([C1EE])` |
| Cleopatra sheets (z == 2 only) | "love1" DS:4ED4, "love2" DS:4EDA | `163B(name, &arch, 0)` | **[C10E]** = love1 (16 descriptors), **[C1EC]** = love2 (4 descriptors); freed by `19A0` at 5F82 645F/6466 or 648F/6496 |
| scout report | "scout.lzw" DS:5EA1 | `11C4` inside 7B56 into a local buffer, freed at the end of 7B56 | - |
| music | "cn6" + ".lzw" (DS:3EA2, 3F44), music #8 of table DS:3EF0 {name, archive}: archive 7 | 5D29(8) -> 5D73 -> 5EB5 (only if [3EEC] != 0 or n >= 0x11) | the Cleopatra love theme, started at 62CF, stopped/freed at 6473/6479 |

- **11C4 / 4B8 text format** (D, V on all 26 TEXT files and SCOUT): u16 offset table, `n = off[0] / 2` entries, the last
  one = file length; string i = bytes off[i]..off[i+1] **including its NUL**. `04B8(buf, i, dst)` copies exactly
  off[i+1]-off[i] bytes (rep movsb, adds no NUL). No string contains LF; the longest '~'-variant is 112 bytes; the longest
  player line (4C92 table) 94 bytes (V).
- **163B sprite sheet** (D): HDR = 14-byte descriptors `{u16 w, s16 xhot, s16 yhot, u8 bytes/row, u8 rows, u8 transparent
  nibble, u8 LUT, u16 offset, u16 segment}`; 16FB counts them (len / 14) into **[C0CE]**; 17B4 unpacks the DAT into a far
  block (863E), offset field = position + 2 in that block, and writes the block's segment into each descriptor's +0x0C.
  The third argument (0 here) would re-map colours through 85D2; 0 = keep each descriptor's own LUT. Same format as
  province.c `sheet_load` (MMENU/INIT), which can load TAB/LOVE1/LOVE2 unchanged. V: in all three sheets the offsets tile
  the DAT exactly (offset - 2 + bytes/row * rows = next offset - 2; the last one ends at the DAT length).
- **1273 CBM** (D): reads the 10-byte CBM header into the descriptor, sets +0x0A = 2 and +0x0C = segment, so a CBM is a
  sprite descriptor too (hotspot = header x/y, transparent nibble = header byte 8, LUT = byte 9).

### 1.1 Sheet contents (D: headers dumped; pictures rendered with CMAP + LUTs, see section 9)
TAB (8 sprites, all LUT 26, transparent 9):

| # | w x h | hotspot | drawn at | covers | role |
|---|---|---|---|---|---|
| 0 | 200x6 | (0,-114) | (113,79) | x 113..312, y 193..198 | speech box bottom |
| 1 | 5x110 | (0,-4) | (113,79) | x 113..117, y 83..192 | left |
| 2 | 6x110 | (-194,-4) | (113,79) | x 307..312, y 83..192 | right |
| 3 | 200x4 | (0,0) | (113,79) | x 113..312, y 79..82 | top |
| 4 | 98x5 | (0,-43) | (8,151) | x 8..105, y 194..198 | menu box bottom |
| 5 | 5x39 | (0,-4) | (8,151) | x 8..12, y 155..193 | left |
| 6 | 8x39 | (-90,-4) | (8,151) | x 98..105, y 155..193 | right |
| 7 | 98x4 | (0,0) | (8,151) | x 8..105, y 151..154 | top |

LOVE1 (16 sprites, LUT 7, transparent 0): 0 = 224x66 left part of the bedroom scene (cartouche + couch with Cleopatra),
1 = 96x66 right panel (a man seen from behind, cartouche), 2 = 44x63 the man alone (moves in the right panel),
3..15 = the 13 frames of a Roman walking to the couch (left part). LOVE2 (4 sprites): 0 = 96x66 second Cleopatra portrait
(LUT 9, transparent 15, fully opaque), 1 = 319x63 bedchamber picture (LUT 9, opaque), 2 = 320x63 Cleopatra's barge on the
Nile with pyramids (LUT 8, transparent 8 but no pixel uses it), 3 = 238x23, hotspot (-51,-17) = the barge area with white
forked lightning (LUT 8, transparent 15, 44 transparent px).
LEADER?.CBM: 96x66, hotspot (0,0), transparent nibble 8, LUT 1 (many have "holes": LEADERB 2,948 of 6,336 pixels are
nibble 8, LEADERX 1,659).

## 2. 6570 nego_screen(z, &buf) - drawing order (D)

W = work buffer (67E4 makes it the target: C0D0 := C1DA), S = screen (680F: C0D0 := 9AB6). `copy(rect) W->S` is 6973.
```c
if (!open_archive_0F7B(7, &arch)) return 0;
67E4();                                              /* target W */
[9AFA] = [6344]; fill7E74(0, 65, 319, 199);          /* W */
strips_70BA(&arch, z);                               /* W rows 0..64: sky 0x17 rows 0..31, [roll(100) >= 58A6[z]: SKY<roll(4)>],
                                                        NEGBG<'A'+588A[z]>, NEGFG<'@'+z> at y 31, 71BB lines -- province.c   */
950E(src=C1DA, dst=C1DA, &{159, 0, 254, 65}, dx=9, dy=80);   /* W: copy (159,0)-(254,65) to (9,80)-(104,145)              */
[4EF4] = '@' + z;
if (load_cbm_1273("leaderX.cbm", &arch, &cbm)) { 8E62(&cbm, 8, 79); free_86A0(&cbm); }   /* W, nibble 8 transparent  */
[4EE8] = '@' + z;  load_11C4("textX.lzw", &arch, buf);        /* result not checked */
if (z == 2) { [C10E] = sheet_163B("love1", &arch, 0); [C1EC] = sheet_163B("love2", &arch, 0); }
[C1EE] = sheet_163B("tab", &arch, 0);
680F();                                              /* target S */
copy 6973(0, 0, 319, 64);                            /* W -> S: strip rows 0..64 */
[9AFA] = [6344]; fill7E74(0, 65, 319, 199);          /* S */
lines_71BB();                                        /* S (same pixels as in W) */
[9B69] = 0; 8BE4(zone_name[5222 + 2z], 160, 65);     /* S, centred, colour 0 */
[9AFA] = [6346]; fill7E74(117, 82, 307, 192);        /* S: speech box interior, colour 0x1A */
for (k = 0; k < 4; k++) 8E62([C1EE] + 14k, 113, 79);   /* S: TAB 0..3 = the speech box frame (113,79)-(312,198) */
scout_7B56(&arch, z);                                /* S: scout report + "Click to continue" (section 4) */
copy 6973(8, 79, 103, 144);                          /* W -> S: the portrait (only now visible) */
shadow_32E2(8, 79, 103, 144);                        /* S */
mouse_warp_066A(88, 171); range_x_06E4(8, 103); range_y_0709(151, 197);
close_11A9(&arch);
return -1;
```
Details:
- The portrait background: the 96x66 slice of the province's own strip at x 159..254 is pasted at (9,80), **one pixel
  right and down of the portrait** (8,79). Where the portrait is transparent (nibble 8), the landscape shows, shifted by
  (+1,+1). Column x 8 and row y 79 of the window come from the W fill 0x74; rows 143/144 of the window are the strip's
  71BB lines (row 63 black, row 64 colour 8) moved down by 80 (D; rendered for Aegyptus and Narbonensis, section 9).
- The portrait window copied to S is (8,79)-(103,144) = exactly the 96x66 portrait.
- **32E2 shadow(x0, y0, x1, y1)** (D): `[9AFA]=0; hline7ECA(x0+1, y1+1, x1+1); line925C(x1+1, y0+1, x1+1, y1+1);
  [9AFA]=8; hline(x0+2, y1+2, x1+2); line(x1+2, y0+2, x1+2, y1+2);` -> black row 145 x 9..104 and column 104 y 80..145,
  grey (8) row 146 x 10..105 and column 105 y 81..146.
- **Mouse range** (D): 06E4 writes [BC]/[C0] (x min/max), 0709 writes [BE]/[C2] (y min/max) and INT 33h fn 7/8. The
  pointer is confined to the reply-menu box (8..103 x 151..197) for the rest of the negotiation; 066A puts it at
  (88,171). The port's poll0644 already clamps to these DS words, so writing them is enough. Nothing restores the range:
  494A (map) sets 0..311 x 0..185 again. 7B56's "Click to continue" happens **before** the range is set (old range).
- Order of the reply menu: it does not exist yet; the lower-left box stays colour 0x74 until the first dialogue round.
- Dice consumed by 6570: 70BA's roll(100), then roll(4) only if a cloud picture is drawn (z != 0). They come **before**
  the dialogue's first roll(3) (6501) - keep that order for byte-exact runs (D; province.c strips_draw70ba does the same).
- Archive: everything is loaded while the archive is open; the port's assets.c holds **one** archive at a time, and
  province.c's mmenu()/init1()/init2() loaders switch it. Load all MNEGO members (or re-`assets_load` MNEGO) before
  touching other sheets.

### 2.1 Reuse of the ported pieces
- `strips_draw70ba(z)` (province.c) is 70BA for VGA, drawing into `g_fb`; set `g_fb = g_wrk` and load MNEGO first
  (exactly what `strips_load708a` does). It already writes the letters into DS:58D3/58C7/58DD as the original.
- `strip_lines71bb()` = 71BB (into `g_fb`); call it again with `g_fb = g_scr` after the S fill.
- `cbm_draw(name, x, y)` = 1273 + 8E62 + 86A0 (header hotspot, byte 8 transparent, LUT byte 9): use it for the portrait.
- `sheet_load` + `sprite8e62` handle TAB / LOVE1 / LOVE2 (same 14-byte HDR format).
- `fill7e74`, `hline7eca`, `text8be4`, `text899c`, `restore6973` (= 6973 W->S), `save691b` (= 691B S->W) exist in
  province.c (static); 925C vertical lines can be `fill7e74(x, y0, x, y1)`.
- 950E with src = dst = W and a different destination point is not ported: it is a plain inclusive rect copy
  (VGA path 95C0: source (x0..x1, y0..y1), destination top-left dx = 4th pushed argument ([bp+6]), dy = 5th ([bp+4]);
  rows copied top to bottom with rep movsb, so an upward scroll in the same buffer is safe).

### 2.2 Screen map after 6570 (D)
| area | content |
|---|---|
| rows 0..62 | strip (sky / clouds / NEGBG / NEGFG, or ROME) |
| row 63 | x 1..319 colour 0, x 0 colour 0x74; row 64 x 2..319 colour 8, x 0..1 colour 0x74 |
| rows 65..199 | 0x74 background |
| y 65 (glyph rows about 66..73) | zone name, colour 0, centred on x 160 |
| (8,79)-(103,144) + shadow to (105,146) | leader portrait |
| (113,79)-(312,198) | speech box: TAB 0..3 frame, interior (117,82)-(307,192) colour 0x1A |
| (8,151)-(105,198) | reply menu box (from the first dialogue round on) |

### 2.3 Text colour word [9B69] (D: 8D56)
899C/8BE4 build each glyph as a 4-bpp sprite at DS:9B48 and blit it with 8E62: **ink nibble = [9B69] & 0x0F,
sprite LUT = ([9B69] >> 8) & 0x3F**, transparent nibble = [9B85] & 0x0F (complemented if equal to the ink), bit 15 =
opaque band (899C). So the drawn colour is `LUT[(attr >> 8) & 0x3F][attr & 0x0F]`. With [6348] = 0x010C this is
LUT 1 entry 12 = **0x1A**, the speech box fill: 7CB2 erases its "Click to continue" by redrawing it in the box colour.
**Port gap**: ui.c `ui_text_attr` uses `attr & 0x0F` and ignores the LUT byte; it would draw colour 12 (bright red)
instead of erasing. Either add the LUT lookup (LUT 0 is the identity, so every existing caller is unchanged) or pass 0x1A.

## 3. 5AE4 say(str, colour, quote) and 5AC2 blank(n) - the speech log (D)

```c
void say_5AE4(char *s, int colour, char quote) {          /* push s, colour, quote; ret 6 */
    char buf[0x146], line[0x40];  int done = 0, w = 0;
    int pitch = [9B5C] + [9B5D] - 1;                      /* 8 + 3 - 1 = 10 (one less than the 11 px font pitch) */
    [9B69] = colour;
    [3E76] = pitch + 0x56;                                 /* scroll rect DS:3E74 = {120, 96, 305, 190} */
    if (quote) { buf[0] = quote; [3E72] = quote;           /* DS:3E72 becomes the 1-char string quote */
                 strncpy(buf + 1, s, 0xFE); strcat(buf, (char*)0x3E72); }   /* '"' + s + '"' */
    else         strncpy(buf, s, 0x100);
    char *t = strtok(buf, " ");                            /* A0A8 = strtok (static at DS:A42E), DS:3E6C " " */
    line[0] = 0;
    while (!done) {
        if (8C58(t) + w > 186 || t == NULL) {             /* 8C58(NULL) = 0; w = 8C58(line) incl. its trailing ' ' */
            if ([C1D8] + pitch > 190) {                    /* scroll the log up by one line, on the current target */
                950E(src=C0D0, dst=C0D0, &rect3E74 /*(120,96)-(305,190)*/, dx=120, dy=86);
                [C1D8] -= pitch;
                [9AFA] = [6346]; fill7E74(120, [C1D8], 305, [C1D8] + pitch);
            }
            899C(line, 120, [C1D8]);                       /* colour [9B69], transparent */
            [C1D8] += pitch;  line[0] = 0;
        }
        if (t == NULL) done = -1;
        else { strcat(line, t); strcat(line, " "); w = 8C58(line); t = strtok(NULL, " "); }
    }
}
void blank_5AC2(int n) { for (i = 0; i < n; i++) say_5AE4("" /*DS:3E02*/, 0, 0); }   /* each = one empty line */
```
- [C1D8] = next line's y; 7B56 sets it to 86 (0x56) at the start of every negotiation. Lines are drawn at 86, 96, ...,
  176; the 11th line scrolls: rows 96..190 move to 86..180, (120,176)-(305,186) is filled 0x1A, the line is drawn at 176,
  [C1D8] = 186. From then on every new line scrolls and lands at y 176.
- Wrap rule: a word goes on the current line while `W(line incl. trailing space) + W(word) <= 186` (8C58 widths include
  the 1 px letter spacing). Lines are drawn with their trailing space. Runs of spaces collapse (strtok); a word wider than
  186 px gets its own line (after flushing the current one, possibly an empty line). The text is never hyphenated.
- Colours used: leader speech 15 (white) with quote '"' (5F82 60FB, 629F, 638C); the player's reply 9 (palette
  84,84,252) with quotes (61E8); narration (scout report, Cleopatra texts 4DD4/4E28/4D0C) colour 0 without quotes.
- The log is drawn on the **current target**; in the negotiation that is always S (5F82 switches back to S with 680F
  before any say). The log is never cleared during a negotiation, it only scrolls.
- `line` is 64 bytes; 186 px of the narrowest glyphs could exceed it in theory (G: no text does).

## 4. 7B56 scout report and 7CB2 click wait (D)

```c
void scout_7B56(arch, z) {                         /* push z, &arch */
    [C1D8] = 86;
    if (!load_11C4("scout.lzw", &arch, &sb)) return;
    04B8(&sb, z, AC60);  say(AC60, 0, 0);          /* SCOUT.LZW string z (string 0 = "" for Italia) */
    blank_5AC2(1);
    AC60 = "The wealth of " + zone_name[z] + " is " + 5F52[P.b14 / 40]
         + ", the army is " + 5F5A[(P.w02 * 3 + P.w00) / 2500]          /* 16-bit sum, unsigned divide */
         + " and the soldiers are known for their " + 5F62[P.b0A / 25];
    say(AC60, 0, 0);
    click_7CB2();
    blank_5AC2(2);
    AC60 = leader_name[5F1C + 2z] + ", the leader of " + zone_name[z] + ", offers a greeting:";
    say(AC60, 0, 0);
    free_198F(&sb);
}
void click_7CB2(void) {
    [9B69] = 4;      8BE4([3E40], 212, 183);       /* "Click to continue", palette 4 (168,0,0), centred on x 212 */
    wait_click_35F9();
    [9B69] = [6348]; 8BE4([3E40], 212, 183);       /* 0x010C = LUT 1[12] = 0x1A: erased (only its ink pixels) */
}
```
P = province record 5270 + 32z (P.b14 = +0x14, P.w00/+0x02 the troops, P.b0A courage). Tables (DS):
5F52 wealth {small, unremarkable, immense, legendary}; 5F5A army {insignificant, ordinary, strong, huge};
5F62 courage {"tendency to panic.", "lack of courage.", "tenacity.", "fierce courage."}; 5F1C leader names (section 8).
- **1990 bug (D)**: the tables have 4 entries but the indices are not clamped. Aegyptus starts with +0x14 = 200 (static
  and run-time images): 200 / 40 = 5 reads 5F52[5] = 5F5A[1], so the scout says **"The wealth of Aegyptus is ordinary"**
  although it is the richest province (Parthia 100 -> "immense"). Index 4 would read "insignificant". An army value
  >= 10000 reads 5F62[0..] ("the army is tendency to panic."); courage >= 100 reads the word 0x7845 (bytes of the next
  string "Exempt...") and prints a string of control bytes (1E 1E 14 14), i.e. nothing visible. Start values: wealth
  0..200, army max 8000 (Germania), courage max 90, so only the Aegyptus case happens in a new game (G whether these
  fields change later). Port suggestion: clamp each index to 3 in the default mode, keep the overflow with --original.
- The "Click to continue" at y 183 has its ink on rows 184..191; a log line at y 176 has descenders reaching row 184, and
  the erase redraw (colour 0x1A, same glyph pixels) can cut such descender pixels (G: depends on the glyph shapes).
  Literal drawing reproduces it.
- The 7CB2 wait happens while the portrait window is still plain 0x74 and the mouse range is the caller's.

## 5. Reply menu DS:4080 (D; the dialogue agent owns 5F82's logic)

Record (static and run-time identical): box (8,151)-(103,197), colour 0, items DS:405C, title DS:3E02 (""), count 3
(5F82 sets [408E] = 3, or 2 when one of the three codes is 0x3D), single 0xFF, enabled 0xFFFF. Items (12 bytes):
text and hotkey written by 5F82 from 4028[x] / 404A[x] (x = 40E6[code]), disabled 0, rects relative to (8,151):
(7,4)-(93,14), (7,17)-(93,27), (7,30)-(93,40).
Each round (5F82 6159..61B0):
```c
67E4();                                               /* W */
[9AFA] = [6346]; fill7E74(10, 151, 103, 197);          /* W */
for (k = 0; k < 4; k++) 8E62([C1EE] + 0x38 + 14k, [4080] /*8*/, [4082] /*151*/);   /* TAB 4..7: frame (8,151)-(105,198) */
menu_draw_5614(0x4080, 0);                            /* mode 0: no box; items + ornament + (empty) title */
680F();  copy 6973(8, 151, 110, 198);                 /* W -> S */
do sel = menu_run_54C8(0x4080, 0); while (sel == 0xFF);   /* Esc is ignored */
```
Pixels (MENU_NOTES 3/3.3): items at (8+7+11, 151+y0) = x 26, y 155 / 168 / 181, colour 0. Ornament colour 8:
sx = 15, lines x 17, 19, 21 from y 155 to yb, caps x 16..22 at y 155 and at yb, yb = 151 + last.y1 - 1 = 190 (3 items)
or 177 (2 items). Title "" draws nothing. Hit rects (absolute, inclusive): (15..101, 155..165), (15..101, 168..178),
(15..101, 181..191). 54C8 redraws the chosen item in colour 15 on S (585B) and waits for the release. The copy also
takes x 106..110 from W (colour 0x74 there, same as S). A third item hidden by count 2 keeps its text in the record but
is neither drawn nor hit-tested. Duplicate hotkeys exist (W Withdraw/Warm, G Go to War/Gentle, A Aggressive/Approach,
L Leave/Low): 5593 returns the first match among the shown items.
Next, `say(4C92[code], 9, '"')` logs the player's line. Delays: `0143(0x48)` after a non-final choice and after
Withdraw (72 ticks).

## 6. Buffer switching (D)

| fn | what (VGA, [9B26] = 2) |
|---|---|
| 67E4 | target = work: copy the 18-byte descriptor C1DA -> C0D0 (Tandy [9B26] = 6: port 3DF := E6) |
| 680F | target = screen: copy 9AB6 -> C0D0 (mode 6: 3DF := F6) |
| 691B(x0,y0,x1,y1) | cursor push(0); 950E(src 9AB6 screen, dst C1DA work, &rect, x0, y0); cursor pop = save S -> W, same place |
| 6973(x0,y0,x1,y1) | the same with src/dst swapped = W -> S |
| 6837() | VGA with [4F96] = 0: 6887 ([4F96] ^= 1, C0D0 := C1DA, `0494(seg 9ABA = A000, seg C1DE = 2687)` = copy all 64,000 bytes W -> S), then [4F96] := 0 and C0D0 := 9AB6: **present the whole work buffer and make the screen the target**. Returns 0 if [4F96] was 0, else 1 |

Descriptors (run-time): 9AB6 = {rows AAD0, base 0, seg A000, stride 320, h 200, clip 0,0,319,199}; C1DA = the same with
seg 2687 (the work buffer). 950E VGA path: plain inclusive rect copy, destination (dx, dy) = (4th, 5th pushed argument).
Port: `g_fb` = target; 691B = `save691b`, 6973 = `restore6973`, 6837 = `memcpy(g_scr, g_wrk, 64000); g_fb = g_scr`.

## 7. End of the negotiation (D)
5F82 never redraws the screen itself. On every exit it frees the text buffer (198F), love1/love2 when z == 2 (19A0, at
645F/6466 or 648F/6496; the pointers [C10E]/[C1EC] are not cleared), the TAB sheet (64EF `19A0([C1EE])`), and pops the
cursor (1D8A). Exits:
- Withdraw: say the line, delay 72 ticks (skippable), return 0; the screen stays as it is until the caller redraws.
- Go to War: `1E21` (disk prompt), battle B06E draws its own screens; result as returned.
- Alliance (z != 2): leader's alliance text (15, quotes), blank line, 7CB2 click, return -1.
- Cleopatra endings: section 8.
After `province_won 53F8` (if the result is nonzero), 382A (39BA) continues; at 3A24 it calls **494A(dest zone)**, the
map screen, which sets the mouse range back to 0..311 x 0..185 and warps the pointer to the zone anchor (1AB8/1ABA).
The music started by the feast is stopped only on the 6458 path (5D20 stop, 5E1A(8) free); the Withdraw/War paths after
state 15 would leave it playing, but the Aegyptus menus of states 15/16/20 contain no Withdraw/War (G, B table DS:4162).

## 8. Cleopatra (z == 2) drawing (D) - the Easter egg

### 8.1 Alliance (state 2) at 62C2
```c
say(variant(text(buf, 2)), 15, '"');        /* generic alliance code before 62A2 */
blank(1); click_7CB2();
music_5D29(8);                              /* cn6.lzw from MNEGO */
8E62([C1EC] + 28, 0, 0);                    /* S: LOVE2 #2, the barge, 320x63 over rows 0..62 */
[9AFA] = [6344]; fill7E74(8, 151, 107, 198);   /* S: reply menu erased */
say(DS:4DD4 "Cleopatra entertains the Romans in her palace, and the celebration lasts five days.", 0, 0);
blank(1);
say(DS:4E28 "Infatuated with the Aegyptian queen, ... Cleopatra is as sharp-tongued as ever.", 0, 0);
blank(1); click_7CB2();
8E62([C1EC] + 14, 0, 0);                    /* S: LOVE2 #1 bedchamber, 319x63: column 319 keeps the barge pixel */
8E62([C1EC] + 0, 8, 79);                    /* S: LOVE2 #0, second portrait over the first (opaque); shadow stays */
state = 15;                                 /* dialogue goes on (the menu is redrawn next round) */
```
### 8.2 States 0x11..0x13 (6348/636C): the guards
`say(variant(text(buf, state)), 15, '"'); blank(1); delay_0143(144); say(DS:4D0C "Cleopatra calls the guards, ...
the alliance remains intact.", 0, 0); click_7CB2();` then 6458 (free, music stop, result -1).
### 8.3 State 0x15 (63B0): the love scene
```c
blank(2);
[9AFA] = [6344]; fill7E74(8, 151, 107, 198);   /* S: menu erased */
love_4852();                                    /* 8.4 */
delay_0143(144);
[9AFA] = [6344]; fill7E74(8, 79, 105, 146);     /* S: portrait and its shadow erased */
8E62([C1EC] + 28, 0, 0);                        /* S: the barge again */
[9B69] = 4; 8BE4([3E40], 212, 184);             /* S: "Click to continue", colour 4, y 184 (7CB2 uses 183) */
save_691B(0, 0, 319, 199);                      /* S -> W */
poll_0644();
goto flash;                                     /* one flash always happens first */
for (;;) {
    if ([B2]) break;                            /* any button held */
    if (key_03AA()) break;                      /* any key (consumed) */
    poll_0644();
    if (ticks_01AE() > 72) {                    /* slot 0 */
        reset_00F8();
        if (roll_076B(12) < 2) flash: flash_654B();
    }
}
present_6837();                                 /* W -> S whole, target S (no visible change) */
/* 6458: free text, love1, love2; result -1; stop music 5D20; free music 5E1A(8) */
```
`flash_654B()`: `8E62([C1EC] + 42, 0, 0)` (LOVE2 #3: lightning over the barge, at (51,17)-(288,39)), `delay_0143(9)`,
`copy 6973(0, 0, 319, 63)` (W -> S: the saved picture back). There is no release wait: the button is still down when
5F82 returns. **Dice**: the number of roll(12) calls depends on real time (one per 72+ ticks while waiting), so a
byte-exact replay needs the same timing; a headless port must define it (e.g. a virtual clock advanced per poll).
The 0143 delay inside the flash consumes a key or ends on a held button (see 8.5) without ending the loop.

### 8.4 4852 love animation (rows 0..63)
```c
if (![C10E]) return;
67E4();                                                     /* W */
8E62([C10E] + 0, 0, 0);  8E62([C10E] + 14, 224, 0);  8E62([C10E] + 28, [3C26], 0);   /* LOVE1 0, 1, 2 at x 229 */
680F(); copy 6973(0, 0, 319, 63);                           /* S */
delay_0143(72);
for (i = 0; i < 13; i++) { reset_00F8(); 8E62([C10E] + 42 + 14i, 85, 3); frame_48EF(i); }   /* S, not erased */
for (i = 13; i < 28; i++) { reset_00F8(); frame_48EF(i); }
void frame_48EF(i) {
    if (B(3C26 + i)) { 67E4(); 8E62([C10E] + 14, 224, 0); 8E62([C10E] + 28, B(3C26 + i), 0);
                       680F(); copy 6973(224, 0, 319, 63); }
    while (ticks_01AE() < 14) ;                             /* 14 ticks per step, not skippable */
}
```
DS:3C26 (28 bytes) = x of LOVE1 #2 per step: `E5 00 00 00 00 00 E6 E7 E9 00 00 00 00 EA EB EC 00 00 00 00 00 00 EE F0
F2 F3 00 F4` (0 = no change): the man in the right panel slides from x 229 to 244.
Left frames (LOVE1 #3..#15, transparent 0, drawn on S over each other at (85,3) - hotspot): top-left x/y =
#3 (85,25) 6x25, #4 (85,2) 19x48, #5 (85,2) 19x47, #6 (85,2) 20x47, #7 (91,2) 23x47, #8 (97,3) 24x46, #9 (98,3) 29x46,
#10 (114,3) 15x36, #11 (115,3) 20x36, #12 (118,2) 29x37, #13 (122,2) 73x39, #14 (115,2) 80x48, #15 (134,2) 51x43.
LOVE1 #0 is 66 rows: rows 64/65 land in W only (the copies stop at row 63).
Timing: 72 ticks (skippable) + 28 x 14 ticks = 392 ticks, about 1.0 s + 5.4 s.

### 8.5 Timer and delays (D)
INT 8 hook at EXE 0000 increments the dword [42]/[44]; the PIT channel 0 is reloaded with 0x3FFF (EXE 009B..00A6:
1193182 / 16383 = **72.83 Hz**, 13.73 ms per tick; restored to 0xFFFF on exit), the old handler is chained every 4th
tick. 00F8 = reset slot 0 ([46]/[48] := now, via 00D7/00FF); 01AE = ticks since slot 0 (0xFFFF if the high word
differs). **0143(n)**: reset slot 0, then wait until n ticks passed **or** a button is down ([B2] after 0644) **or**
3603 returns a key (with a mouse = 03AA, the key is consumed). So 72 = 0.99 s, 144 = 1.98 s, 9 = 0.12 s.
Port: there is no tick source yet (screen.c); 0143/01AE/00F8 need one (window: SDL ticks x 72.83 / 1000; headless:
a virtual clock), and scripted keys are eaten by these delays exactly as in the original.

## 9. Verification done (V) and method
Scratch (not in the repo): `C:\Users\erikk\AppData\Local\Temp\claude\bfb5522be8ee045d\c4bf0f5f-57f9-4ea6-80b3-e1aa05bf0592\scratchpad\wf\nego_screen\`
`mnego.py` (DIR/DAT + LZSS), `render.py` (CMAP palette, OVL0 LUTs at 0x114C0, sprite blit x - xhot / y - yhot,
transparent nibble), `scr.py Z [cloud]` (the composed screen of section 2), `love.py` (section 8), `d.sh` (objdump).
- V: the TAB sprites tile exactly with the coordinates of 6589/6159 (speech frame (113,79)-(312,198) around the
  fill (117,82)-(307,192); menu frame (8,151)-(105,198) around the fill (10,151)-(103,197)): confirms the hotspot sign
  and the push order of 8E62 (also read at 8EA4: x0 = [bp+6] - desc+2, y0 = [bp+4] - desc+4).
- V: 63E0's erase rect (8,79)-(105,146) is exactly portrait + 32E2 shadow.
- V: sheet offsets tile the DAT files; text tables end at the file length; strings NUL-terminated.
- Rendered pictures only (no capture to compare): nego_2.png (Aegyptus), nego_24.png (Narbonensis), love_*.png.

## 10. Open questions
- G: whether P+0x14 / troops / courage can grow past the table ranges later in a game (section 4 overflow).
- G: interpretation of the pictures (section 11); Erik should look at love2_1_0.png and love2_2flash.png.
- Not decoded here: 5F82's dialogue logic and tables (the dialogue task), 6501 beyond "roll(3)+1 picks the n-th
  '~'-separated variant (9FF8 = strchr), the last one if there are fewer", 53F8, B06E.
- The keyboard-only paths ([00BA] = 0: bullet sprites, 5901) and the EGA/Tandy branches of 67E4/680F/6887 are not
  covered (the port has a mouse and VGA).

## 11. Easter eggs and jokes (for Erik)
1. **Cleopatra** (known): alliance with Aegyptus leads to the feast, the bedchamber and the love scene (section 8),
   with its own music CN6.LZW.
2. **The bedchamber picture** (LOVE2 #1): Cleopatra reclines on her couch pulling a face, her mouth/tongue pushed out in
   a long grimace - a visual pun on the text's "Cleopatra is as sharp-tongued as ever" (G: my reading of the render).
3. **The love scene itself is never shown**: after the walking animation, the screen cuts to her barge on the Nile, and
   while "Click to continue" waits, white lightning bolts flash over the barge at random (one flash at once, then a 2-in-12
   chance about every second) (D for the mechanics, G for the joke).
4. **Leader names** (DS:5F1C, shown in the scout report "X, the leader of Y, offers a greeting:"): "Thorax of Thrace"
   (Thracia), "Absolna the Rotten" (Cilicia), "Mamud the Terrible" (Arabia), "Malodor the Impaler" (Sarmatia; malodor =
   stench), and the unhistorical "Horaxus" (Mesopotamia), "Bertonus" (Armenia), "Darminus" (Sardinia), possibly staff
   names in Latin dress (G). Italia's leader is "Cinna", never shown (Rome is never negotiated; SCOUT string 0 is empty
   and there is no LEADER@.CBM).
5. **Scout reports** (SCOUT.LZW): Cilicia "...the scourge of Mare Nostrum. The scouts' advice is simple: SQUASH THEM!";
   Alpes "your scouts cry out the dreaded word: BARBARIANS!".
6. **1990 bug**: "The wealth of Aegyptus is ordinary" (section 4).
