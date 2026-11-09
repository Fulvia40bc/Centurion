# Naval battle data (MNAVAL archive 8, NAVL archive 9)

Claude, 2026-11-09. The data side of the naval battle, overlay section 5 (`disasm/OVL_NAVAL.asm`, index in
[NAVAL_INDEX.md](NAVAL_INDEX.md)). Marks: **D** decoded from the disassembly or dumped from the data, **V** verified by an
exact fit or by drawing it, **G** guess. Nothing was run in the game or in DOSBox.

Work files (scripts, extracted members, PNGs) are in
`C:\Users\erikk\AppData\Local\Temp\claude\bfb5522be8ee045d\c4bf0f5f-57f9-4ea6-80b3-e1aa05bf0592\scratchpad\fleet\data\`:
`raw\` (all members unpacked with `tools/extract_data.py`), `png\` (contact sheets, scale 3, frame number in yellow,
hotspot as a magenta pixel), `hdr.py` (header dump), `render.py` (sheets, intro composite, collision overlay),
`ds.py` (static DS dump), `scanarc.py` (who opens archives 8/9).

## 1. The archives (D, dumped)

The archive table is at DS:029E. Entry 8 is MNAVAL (its name is stored obfuscated at DS:6406) and entry 9 is DS:021B "navl".
Archive 8 = MNAVAL is confirmed by the music table: songs 13..16 = cn9a..cn9d are in archive 8 (DS:3EF0), and only
MNAVAL has CN9*. Every member has format 0x80 (u32 unpacked size, then LZSS). Nothing in either archive is a CBM.

| archive | member | packed -> unpacked | what | loaded by |
|---|---|---|---|---|
| 9 NAVL | GALLEYS.LZW | 12828 -> 64000 | title picture, raw 320x200 8-bit pixels (sec. 3) | BF50 (VGA only) |
| 9 NAVL | OARS1HDR/DAT | 605->784 / 10362->44103 | 56 sprites: oars + wake, headings 0..7 | BF50 -> [C3A6] |
| 9 NAVL | OARS2HDR/DAT | 609->784 / 10297->43945 | 56 sprites: oars + wake, headings 8..15 | BF50 -> [C3A8] |
| 8 MNAVAL | SMLHDR/DAT | 199->224 / 3022->5555 | 16 sprites: Trireme, 16 headings | BF50 -> [C3A2]/[C3A4] |
| 8 MNAVAL | MEDHDR/DAT | 181->224 / 4046->6990 | 16 sprites: Quinquereme | same |
| 8 MNAVAL | LRGHDR/DAT | 193->224 / 4539->7731 | 16 sprites: "Galleon" | same |
| 8 MNAVAL | BOMBSHDR/DAT | 588->924 / 1399->4003 | 66 sprites: fire pots, arrows, fire, splash, corvus | BF50 -> [C3AE] |
| 8 MNAVAL | SMOKHDR/DAT | 50->42 / 2508->4527 | 3 sprites: melee cloud | BF50 -> [C3AC] |
| 8 MNAVAL | FLAGHDR/DAT | 53->56 / 214->600 | 4 sprites: waving pennant (intro) | BF50 -> [C3BC], freed in C105 |
| 8 MNAVAL | COLLIDE.LZW | 543 -> 816 | collision circles + bow points (sec. 5) | BF50 (11C4) -> C3B0..C3B8 |
| 8 MNAVAL | SSINK/MSINK/LSINK HDR/DAT | see sec. 4.6 | 22 / 27 / 24 sprites: a flagship sinking | D60C |
| 8 MNAVAL | SHARKHDR/DAT | 81->112 / 36->52 | 8 sprites: the shark | D60C -> [9956] |
| 8 MNAVAL | CN9A..CN9D.LZW | 445->550, 1134->1666, 1208->1842, 1426->2577 | music 13..16: the rowing drum | BF50 (5D73) |

- **There is no "2GALLEYS"**: NAVL's first member is `GALLEYS.LZW`.
- **Who opens NAVL:** only section 5 opens it, at BF58 (`mov ax,9 / push ax / ... call 0F7B`). A byte scan of the EXE and
  all five overlay sections for `mov ax,8|9; push ax; ... call 0F7B` finds exactly three hits, all in section 5: BF58
  (archive 9), BFAC (8) and D630 (8). (D for the scan; an open coded another way would be missed.)
- **EGA path, missing file (D):** BF50 loads "galleys.lzw" from NAVL only when [9B26] == 2 (VGA). Otherwise it loads
  "galleys.cbm" from archive 8 (1273 / 8E62 / 86A0), and **MNAVAL has no GALLEYS.CBM**. NAVL is VGA-only data, like
  "batlvga" (archive 1). An EGA game would show no title picture. This does not matter for the port, which is VGA only.

## 2. Sprite sheets: format exactly as the code reads them (D; V by the fit below)

They use the same loader and drawer as the land battle (battle_gfx.c, VERIFIED pixel-exact there):
`163B(name, &arch, remap)` loads `<name>HDR.LZW` and `<name>DAT.LZW` (suffixes DS:0284 / DS:028C), and
`8E62(desc, x, y)` draws one frame.

- **HDR:** 14-byte entries `<HhhBBBBI`: w, xhot, yhot, bytes per row, rows, transparent nibble, LUT index, offset.
  The offset counts from **2 bytes before** the unpacked DAT, so entry 0 has offset 2.
- **Checks on every naval sheet (V):** every HDR length is an exact multiple of 14. The frames are contiguous: each
  offset equals the previous offset plus bpr*rows. The last frame ends exactly at the DAT length.
- **Remap:** every naval call passes remap 0 (no nibble remap, unlike the land battle's enemy colours).
- **Frame address:** frame k's descriptor is `sheet + 14*k`. The code always builds it as `14*frame + [handle]`.
- **Drawing:** the frame goes to (x - xhot, y - yhot). Pixels are 4-bit, high nibble first. The transparent nibble is
  skipped, and every other nibble goes through LUT[lut] (OVL0 0x114C0 + 16*lut, DS:60C0).
- **LUT 0 = identity**, which means the 16 EGA colours (CMAP 0..15). BOMBS uses it.

| sheet | n | LUT | transparent | LUT entries (hex) |
|---|---|---|---|---|
| SML/MED/LRG, S/M/LSINK, SHARK | 16/16/16, 22/27/24, 8 | 28 | 0 | 00 00 32 ED 48 EE EF 17 44 45 46 F0 68 1A 19 6A |
| OARS1/OARS2 | 56/56 | 32 | 13 | 00 01 02 4E 04 05 56 07 08 09 0A 7B 00 F9 0E 96 |
| BOMBS | 66 | 0 | 13 | identity (EGA colours) |
| SMOK | 3 | 31 | 0 | 00 32x8 F8 1C 68 1A 19 1E 6A (only nibbles 0, 9..15 occur) |
| FLAG | 4 | 15 | 3 | 81 42 82 83 84 67 85 BF 86 87 7F 88 89 8A 8B 8C (only nibbles 0, 1 occur: reds 81/42) |

Real palette: `C:\work\Centurion-native\cmap.bin` (250 x 3, 6-bit). Full header dumps: `python -I hdr.py raw`.

## 3. GALLEYS.LZW and the intro (BF50, C105, C1BB) (D; V by drawing)

**Loading the picture.** GALLEYS.LZW is a raw 320x200 picture with one byte per pixel: a palette index, with no
header and no palette of its own. It uses CMAP colours 0..140, mostly 127/128 (sky) and 130..140.
- **Load:** `0F0C([C1DC], [C1DE], &arch, "galleys.lzw")` unpacks it straight into the far buffer [C1DE]:[C1DC], which is
  the work-buffer descriptor C1DA (+2 offset, +4 segment).
- **Show:** `6973(0, 0, 319, 199)` copies the work buffer to the screen.
- **Picture:** `png\galleys.png` shows a big galley with a red-eagle sail at the left and smaller galleys on the horizon.

**C105, the intro loop.** It runs until a key or a mouse button ([B2]). Each pass when more than 3 ticks have gone by:
1. `fill(38, 2, 68, 15)` in colour [9AFA] = 0x80 (VGA; 0x0B on EGA), the sky behind the pennant;
2. FLAG frame `si` (0..3 cycling) at (38, 5);
3. 6887 flip.

The fit is checked: `png\intro_flag0..3.png` draw the pennant on top of the mast in GALLEYS (V by eye). When the loop
ends, the FLAG sheet is freed (19A0).

**C1BB, palette cycling (VGA only, called every pass of C105).** Every 4 BIOS ticks (DS:9812 = the last time):
- **Phase A** (DS:9816, 0..5): colours DS:9762 = {8C, 8B, 8A, 89, 88, 87} get the 6 RGB triples at DS:9778.
  Colour 9762[(phaseA + i) % 6] = triple i.
- **Phase B** (DS:9817, 0..4): colours DS:976E = {84, 67, 85, BF, 86} get the 5 triples at DS:979C, in the same way.
- **The triples (6-bit, words r,g,b):**
  - A: (28,36,40) (24,32,32) (20,28,28) (16,24,28) (20,28,32) (32,36,36);
  - B: (20,20,24) (20,24,28) (24,28,32) (20,24,28) (12,20,24).
- **The effect:** these are GALLEYS' sea colours (130..140, 103), so the sea shimmers while the title is up.
- **Same values as CMAP (V):** at phase 0 the A triples equal CMAP 8C..87 exactly, and B equals CMAP 84, 67, 85, 86.
  The exception is BF: the table holds (20,24,28) where CMAP has (16,20,24).
- **C18C, never called** (no reference anywhere, see the index): it writes the triples back at phase 0, i.e. it
  restores the palette. Without it, colours 84..8C, 67 and BF stay rotated by whatever phase the intro ended in.
  Whether a later palette load resets them is an open question (sec. 9).
- **Port:** the port must cycle these 11 entries during the intro (VGA path) and leave them rotated as the original
  does.

## 4. The sprites, frame by frame (pictures: `png\*.png`)

### 4.1 Ships SML / MED / LRG (D)
- **Frames:** 16 frames per sheet = 16 headings. Frame = heading C388[side]: 0 = bow north, then clockwise in 22.5 degree
  steps (4 = east, 8 = south, 12 = west).
- **Bow direction (V):** the COLLIDE bow points (sec. 5) lie on the drawn bow for all 48 frames. See
  `png\collide_sml/med/lrg.png`.
- **Sheet per ship type:** DS:980C = {97C0 "sml", 97C4 "med", 97C8 "lrg"}, indexed by the flagship slot.
- **Names at the ship choice (C274, "Choose a flag ship:", DS:96F0):** 96D4 "Trireme" = slot 0 = sml,
  96DC "Quinquereme" = slot 1 = med, 96E8 "Galleon" = slot 2 = lrg.
- **Slots:** C37A = enemy (side 0), C37B = Roman (side 1).
- **Loading:** BF50 loads the enemy sheet into [C3A2]. The Roman sheet goes into [C3A4], and when both slots are equal
  [C3A4] reuses [C3A2].
- **Drawing:** frame k = `[C3A2 + 2*side] + 14*C388[side]`, drawn at the ship's position (C372 + 4*side, C374 + 4*side).
  Side 0 is the enemy at C372/C374, side 1 the Roman at C376/C378. The code that draws them: B8E3, B96A, CF7F (bounding
  box), D97C.
- **Speed table DS:9818 (5 per slot, by oar level C38C):**
  - Trireme 0 2 3 5 6;
  - Quinquereme 0 2 3 4 5;
  - Galleon 0 1 2 3 4.

  Level names DS:9830.. "STOPPED", "CRUISING SPEED", "BATTLE SPEED", "ATTACK SPEED", "RAMMING SPEED".

### 4.2 Oars OARS1 / OARS2 (D)
- **Frame:** `frame = DS:93E6[heading] + C390[side]` (the stroke phase). DS:93E6 = 0, 7, 14, ..., 105, i.e. 7 stroke
  frames per heading.
- **Sheet:** heading < 8 uses OARS1 (`14*frame + [C3A6]`). Heading >= 8 uses OARS2 (`14*frame + [C3A8] - 0x310`;
  0x310 = 56*14, so it is frame - 56).
- **Pictures:** each frame is two banks of oars (brown) plus the white/blue wake, hotspot on the hull centre.
- **One size for all ships:** the oar sprites are the same for all three ship sizes.
- **Order:** the oars are drawn first and the ship on top (B8E3/B96A).

### 4.3 BOMBS (66 frames, LUT 0 = EGA colours, transparent 13) (D for the users, G for the descriptions)

| frames | what | user |
|---|---|---|
| 0..15, 16..31 | fire pot in flight, 16 directions x 2 flame phases | CCDE: `98BE[side] + C3CC[side]`; 98BE toggles 0/16 (`xor 0x10`, CB5B) |
| 32..47 | arrow volley, 16 directions | CCDE: `C3CE + 0x20` at (C3D8, C3DA); D11B clips it (`+0x1C0` = 32*14) |
| 48..50 | flames on a burning deck | CD4B: `0x30 + 98E0[side]`, 98E0 cycling 0..2 |
| 51..57 | splash of a missed shot | CB11: 98BE := 0x33 (CC3A), then +1 per step until > 0x39 |
| 58..65 | corvus (boarding bridge), 8 compass directions N, NE, E, ... NW; hotspot at the foot | BA7A: `0x3A + 95D4[h]` when (h - C370) & 15 is 1..7, `0x3A + 95E4[h]` when 9..15 |

- **Burning (D):** a fire-pot hit sets C3A0[target] = rng_roll(50) + 10 (CC26). CD4B decrements it on every draw, and
  each time its low nibble reaches 0 the target's flagship strength C38E[side] loses 1. Side 0 = enemy C38E, side 1 =
  Roman C38F.
- **Corvus:**
  - DS:95D4 = 07 07 00 00 01 01 02 02 03 03 04 04 05 05 06 06;
  - DS:95E4 = 01 02 02 03 03 04 04 05 05 06 06 07 07 00 00 01;
  - base offsets DS:95F4/9624 and 9654/9684, each 48 bytes = 3 sizes x 16 headings;
  - tip offsets per direction: DS:96B4 dx = 0, 10, 16, 10, 0, -10, -16, -10 and DS:96BC dy = -14, -10, 0, 10, 14, 10, 0, -10.
  - D070 then tests whether the tip lies in the other ship (sec. 5).

### 4.4 SMOK (3 frames, 54x45 .. 65x54) (D for the use)
- **Picture:** a dust/smoke cloud with holes (nibble 0) in the middle, so the two locked ships show through.
- **Use:** BB7E (boarding) and D3F2 fill the sea (0, 0, 319, 185) with colour 9, draw both ships (B8E3 1, B8E3 0), then
  draw SMOK frame **rng_roll(3)** on top. This repeats every animation step (BC88, D5B6).
- **Dice:** these rng_roll(3) calls sit in timed loops, so the port must make exactly as many calls as the original
  (one per step).

### 4.5 FLAG: section 3.

### 4.6 Sinking: SSINK (22) / MSINK (27) / LSINK (24) (D)
- **Sheet name:** D60C patches the first letter of DS:992C "xsink" (pointer DS:9950) with DS:9952 = 's', 'm', 'l'
  [slot].
- **Who is loaded:** arg bit 0 loads the **Roman** sheet from C37B. On failure: message "Failed to Load Human Sinking"
  (DS:9978). Arg bit 1 loads the **enemy** sheet from C37A ("Failed to Load Computer Sinking", DS:9995). If both sank and
  C37A == C37B, the enemy shares the Roman sheet.
- **Bits (D):** these two strings confirm the bits: **bit 0 = the Roman flagship, bit 1 = the enemy flagship**. This
  matches B12A's C38F/C38E tests.
- **Frame counts:** DS:994A = 22, 27, 24 words per slot.
- **Erase rectangles:** DS:9932 holds 8-byte rectangles per slot, relative to the ship position:
  - sml (-28, -19, 19, 17);
  - med (-27, -21, 21, 20);
  - lrg (-28, -18, 25, 18).

  y1 is clamped to 185 (D919).
- **Animation:** each step waits for 14 ticks (0x0E), fills the rectangle with colour 9 and draws frame si at the ship
  position: Roman (C376, C378), enemy (C372, C374).
- **Pictures:** the galley lists and goes down in a burst of spray; the last frames are only bubbles and a single pixel
  (MSINK 26 is 1x1).

### 4.7 SHARK (8 tiny frames, 2x1 .. 8x3) (D)
- **Pictures:** 0-1 surfacing, 2-4 a black fin cutting the water, 5-7 diving.
- **Start:** D60C starts one shark per sinking ship, just below the wreck's rectangle: x = x0, y = y1 + 4, or y0 - 2 when
  y1 + 4 >= 185. The state is DS:AAC6/AAC7 (frame), AAC8/AACA (x) and AACC/AACE (y), with index 0 = Roman wreck and
  1 = enemy. Start frames are 0 (Roman) and 1 (enemy).
- **D89B(i), each step while the shark is outside the other ship's rectangle:**
  1. fill (x-5, y-3, x+4, y+2) with colour 9;
  2. x += 1;
  3. draw frame AAC6[i] at (x, y);
  4. frame += 1;
  5. at frame 5: `rng_roll(10)`, and if it is non-zero (9 in 10) go back to frame 2 (keep swimming);
  6. at frame 8: frame 0.

  So the fin circles the wreck and now and then dives.
- **Dice:** this rng_roll(10) is game dice inside a timed animation loop. The number of calls is fixed by the sinking
  frame counts, not by real time, because the loop runs one step per 14 ticks until both animations finish.

## 5. COLLIDE.LZW: the ship hit model (D; V by the overlay)

816 bytes, cut by BF50 with the sizes at DS:9714 = F0 F0 F0 30 30 into five tables. Their pointers are stored at
DS:970A -> C3B0, C3B2, C3B4, C3B6, C3B8. All bytes are signed except r.

| table | bytes | index | meaning |
|---|---|---|---|
| C3B0 dx | 240 | 80*size + 5*heading + k | centre of circle k (0..4) relative to the ship position |
| C3B2 dy | 240 | same | |
| C3B4 r | 240 | same | radius: sml 3..6, med 3..7, lrg 3..8 |
| C3B6 bow dx | 48 | 16*size + heading | ram point (bow tip) |
| C3B8 bow dy | 48 | same | |

**Index tables:** DS:98E2[h] = 5*h, DS:98F2[size] = 0, 80, 160.

**Bow points (dx, dy) by heading 0..15:**
- sml: dx 0 8 15 20 22 21 16 8 0 -9 -15 -21 -22 -22 -15 -9, dy -20 -20 -15 -9 0 8 15 20 22 21 16 9 0 -10 -15 -21;
- lrg: reaches 25.

So heading 0 = north and the headings go clockwise. The overlay `png\collide_*.png` (green circles, red bow) fits the hull
of every frame (V by eye).

**Users:**
- **CE7E** (ship-ship contact): circles_touch 05ED on 5x5 circle pairs.
- **D070(side, x, y)** (point in a ship): first the sprite bounding box C3F0+8*side (set by CF7F from the frame
  descriptor), then point_in_circle 05B0 against the 5 circles.
- **CFDD (ramming):** tests the attacker's (C384) bow point in the other ship. If that misses, it tests the other ship's
  bow in the attacker, and on a hit the rammer becomes the other side (C384 := other).
- **D11B:** arrows against the circles.
- **BA7A:** the corvus tip, via D070.

## 6. Music CN9A..CN9D (D for the use; the song format is the sound overlay's, not decoded)

Music table DS:3EF0 ({name, archive}): 13 cn9a, 14 cn9b, 15 cn9c, 16 cn9d, all archive 8.
- **Loading:** BF50 loads all four (5D73 of DS:9705..9708 = 0D 0E 0F 10) and starts 13.
- **Playing:** C422 sets the Roman ship's oar level. Level 0 (STOPPED) calls music_stop. Levels 1..4 play
  DS:9704[level] = cn9a (cruising) .. cn9d (ramming speed). **The music is the rowers' drum and speeds up with the
  stroke.**
- **Other plays:** B12A plays 16 (cn9d) just before the boarding fight BB7E (B388), and 13 before the "re-engage"
  question (B3BA, B41C).
- **Leaving:** BD8A unloads song 1 (the map music) when the battle starts. C34F stops the music and unloads 13..16.
- **Data:** every CN9 starts `7F 00 00 ...`, the same header as CN6 (NEGO_DATA_NOTES sec. 6).
- **Port:** the port only needs the DS bookkeeping of map.c's music_load_5D73 / music_play_5D29.

## 7. Erik's report: what the data side shows (D)

- **Which bit is which:** the sinking loader settles the meaning of the flags. **Bit 0 = Roman flagship** ("Human
  Sinking", slot C37B, drawn at C376/C378). **Bit 1 = enemy flagship** ("Computer Sinking", C37A, C372/C374).
- **Fire damage:** CD4B lowers C38E for side 0 and C38F for side 1. So C38E = enemy and C38F = Roman, as NAVAL_INDEX
  assumed.
- **The text Erik saw is not a bit mix-up:** "witless commander ... his more competent mates have swept the seas clean
  of Roman vessels" (DS:9220) is the planned "lost" text for "you sank their flagship but lost the fleet battle". The
  1990 writers expected this outcome and joked about it.
- **Why he lost:** that is decided in the fleet battle round DA50/DD2E/DE51, which is not data (the logic agent's part).
  Nothing in the naval data files counts ships or decides the outcome.
- **"In spite of their admiral's bath, your fleet prevails" (DS:9104):** B59F shows it for bit 0 = the ROMAN flagship
  sank. "In spite of" fits a Roman loss, so the mapping is consistent and "their" is most likely a slip for "your" (G).
  It is a wording slip, not a logic bug. Suggestion: keep the text and mention it; at most change "their" to "your"
  outside `--original`.

## 8. 1990 bug found in D60C (D): the second shark has no start position

- **Where:** at D6CE, when both flagships sink (arg = 3) **and both are the same type** (C37A == C37B), the enemy reuses
  the Roman sinking sheet (`[bp-6] = [bp-8]`) and jumps straight to D735.
- **What it skips:** D70F..D730 are skipped. They set the enemy shark's x AACA, y AACE and frame AAC7 (= 1); no other
  code writes them (grep).
- **Effect:** the enemy wreck's shark then starts wherever the previous naval battle's enemy shark stopped. On the first
  such sinking of a session it starts at (0, 0), from the static DS. It swims from there, top left of the screen, and its
  erase box (x-5, y-3, ..) reaches above row 0.
- **Visual only:** AAC7 is always left at 0..7 after a previous run, so the frame stays valid. No rng difference: D89B(1)
  is still called, and it rolls only on frame 5.
- **What triggers it:** the same both-sink case gives "together forever in a deadly embrace".
- **Fix (default):** set AACA/AACE/AAC7 as at D714..D730 in this case too. **--original:** keep the stale values.
- **One point that changes the dice:** whether D89B(1) is called each step depends on the stale position: it is called
  only while the shark is outside the Roman rectangle. When the shark is called, it rolls the dice at frame 5. So the fix
  can change the number of rng_roll(10) calls compared with the original. Under the fix the first call is at frame 1,
  as for a fresh enemy shark.

## 9. Open questions
- **Shark state in other code:** is AAC6..AACF used anywhere else (save file, other overlays)? It is not found in the
  EXE, battle, race or map listings.
- **Rotated palette:** after the intro, CMAP 84..8C, 67 and BF stay rotated (C18C is dead code). Does anything reload
  the CMAP before the map is drawn again, and do any map/province colours use those entries?
- **Corvus offsets:** the exact meaning of the corvus base offset tables 95F4/9624/9654/9684 (x/y per size x heading for
  the two sides).
- **Speed level names:** where the speed level names DS:9830.. are drawn (C71B panel?).
- **CN9 song format:** sound overlay.

## 10. Easter eggs and jokes (report to Erik)
- **The shark:** after a flagship sinks, a little black fin circles the wreck, with a 1-in-10 chance per loop to dive.
  It is a separate 8-frame sprite (SHARK) with its own random swim/dive state machine.
- **"RAMMING SPEED":** the top oar level (G: a nod to Ben-Hur, 1959). The drum music (CN9A..D) speeds up with the oar
  level.
- **"Galleon":** the third Roman ship type is a galleon, a 16th-century ship, next to the Trireme and Quinquereme. Keep
  it.
- **Boarding texts (DS:9406..95A9; pointer table DS:96C4 = 9406, 9455, 947E, 94DA, 9531, used at BCF6; they are not
  in NAVAL_INDEX's string list):**
  - "Your sailors cheer lustily as they swing across and board the enemy flagship.";
  - "the scurrilous enemy scuttles your flagship";
  - "your axemen chop gaping holes in the enemy vessel, allowing Neptune to claim the hulk";
  - "the two flagships sink together, bound eternally in Neptune's playground".
- **Already listed by the disassembler:** "witless commander ... his more competent mates", "their admiral's bath",
  "sent to Neptune's locker without a sword", "together forever in a deadly embrace".
- **Smoke:** the melee cloud has see-through holes, so the brawling ships show through the dust.

## 11. For the implementer
1. **Loading the sheets:** reuse battle_gfx.c's `load_sheet` and `blit`: same HDR, offset - 2, hotspot and LUT rules,
   no remap. Load OARS1/OARS2/GALLEYS from NAVL.DIR/DAT and the rest from MNAVAL.DIR/DAT. The port needs both archives
   (assets_load) and the LUTs (assets_load_luts).
2. **Descriptor pointers:** keep [C3A2/C3A4/C3A6/C3A8/C3AC/C3AE/C3BC/9956] as "sheet handles". The original computes
   `handle + 14*frame` (and `- 0x310` for OARS2), so in the port a handle is just (sheet, base frame).
3. **Collision:** the COLLIDE pointers C3B0..C3B8 are offsets into a far buffer in the original. In the port keep the
   816 bytes in a static array and index it with the same formulas.
4. **Dice inside animations:** rng_roll(3) per smoke step (BB7E/D3F2) and rng_roll(10) in the shark (D89B). Also
   rng_roll(50) on a fire-pot hit.
5. **Sea colour:** colour 9 (CMAP 9 = (21,21,63)). The arena is (0,0)-(319,185), with the panel below (C71B).
6. **Intro:** the title is GALLEYS plus the FLAG at (38,5) with the 0x80 sky fill and the palette cycling of sec. 3.
   The palette cycling is real-time cosmetic: drive it from the frame timer in the headless --script mode.
7. **Bugs:** the sec. 8 shark bug (fix by default, `--original` keeps it) and the possible "their" text slip (sec. 7).
