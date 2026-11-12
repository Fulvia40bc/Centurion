# Chariot race data (MRACE, archive 0) and the race tables in DS

Claude, 2026-11-10. The data side of OVL0 section 4 (chariot race + victory sequence): `disasm/OVL_RACE.asm`, index
[RACE_INDEX.md](RACE_INDEX.md), screens [RACE_SCREEN_NOTES.md](RACE_SCREEN_NOTES.md). Marks: **D** decoded from the
disassembly or dumped from the data, **V** verified by an exact fit (to the DOSBox snapshot/capture of 2026-11-06 or by
a size/offset identity) or by drawing it, **G** guess. Nothing was run in the port.

## 0. Work files

`C:\Users\erikk\AppData\Local\Temp\claude\bfb5522be8ee045d\c4bf0f5f-57f9-4ea6-80b3-e1aa05bf0592\scratchpad\racedata\`:
`raw\` (MRACE unpacked with `tools/extract_data.py`), `png\` (renders), `scripts\`:
`rlib.py` (LZSS, sheets, CBM, LUTs, PNG), `ds.py` (DS dump, static or snapshot), `parade_fit.py` (the capture fit),
`path.py` (CBE6 simulation, minimap), `race_screen.py` (race screen model, sprite-tree checks against the snapshot),
`cards.py` (pre-race cards), `crash.py` (crash scene strips). Sheet contact sheets were drawn with
`..\fleet\data\render.py` (scale 3, frame number in yellow, hotspot = magenta pixel).
Run with `python -I scripts\<name>.py` from `racedata\`.

## 1. The archive (D, dumped)

Archive 0 of 0F7B = "mrace" (DS:029E[0], name stored obfuscated, RACE_INDEX 12). Opened three times per race (BC15,
BDA4, C2DE) and once in the victory sequence (BDA4). DIR entries: 20 bytes (u32 offset, u16 length, 13-byte name,
format byte).

| member | format | packed -> unpacked | what | loaded by |
|---|---|---|---|---|
| CIRCUS.CBM | 0xC0 CBM | 5171 | 320x61, LUT 4: the Circus Maximus from above, the minimap's background | C2DE 1273 into a local descriptor, 8E62 at (0,0) twice, freed at C6DA |
| GALLERY1.CBM | 0xC0 CBM | 9410 | 320x130, LUT 3: the stands with seven statues and a brazier | BDA4 1273 -> DS:C356, 8E62 at (0,0) |
| RACEEST.CBM | 0xC0 CBM | 3338 | 320x70, LUT 3: the arena floor below the stands | BDA4 13A8(3): picture slot 3 (descriptor C0B6), 8790 at (0,130) |
| GALLERY2.CBM | 0xC0 CBM | 3338 | **unused**: byte-identical to RACEEST except header byte 9 (LUT 0 instead of 3) | nobody (section 7) |
| RACEHDR / RACEDAT.LZW | 0x80 | 750 -> 1288 / 10125 -> 22089 | 92 sprites: the race seen from above | BC15 163B("race") -> C2BC |
| LAPSHDR / LAPSDAT.LZW | 0x80 | 55 -> 56 / 375 -> 638 | 4 sprites: the dolphin lap counter | BC15 163B("laps") -> C306 |
| MEDHDR / MEDDAT.LZW | 0x80 | 260 -> 420 / 4674 -> 10644 | 30 sprites: the stretcher bearers ("med") | BC15 163B("med") -> C352 |
| RACE2HDR / RACE2DAT.LZW | 0x80 | 230 -> 350 / 7467 -> 14237 | 25 sprites: the parade, side view | BDA4 163B("race2") -> C354 |
| CN3A.LZW | 0x80 | 967 -> 1386 | song 2: parades | C2DE 5D73(2), BE10/BF6A 5D29(2) |
| CN3B.LZW | 0x80 | 3145 -> 6469 | song 3: the race | C2DE 5D73(3), D08A 5D29(3) |

All sheet loads pass remap 0 (no nibble remap). Song 0 of the victory sequence (cn1) is in archive 3 (CENT), not here.
File-name strings in OVL0 (DS): "race" 83EC, "laps" 83F1, "med" 83F6, "gallery1.cbm" 83FA, "race2" 8407, "circus.cbm"
87AF; "raceest.cbm" is picture slot 3 of DS:0250. No string "gallery2" exists in OVL0.OVL or CENTURIO.EXE (byte search;
the only "GALLERY2" in the race snapshot is the in-memory copy of MRACE.DIR at DS:CF9E). D

## 2. Sprite sheets (format V: same loader as the naval and land battles)

Format (NAVAL_DATA_NOTES 2): HDR = 14-byte entries `<HhhBBBBI` (w, xhot, yhot, bytes per row, rows, transparent nibble,
LUT, offset from 2 bytes before the DAT); 4-bit pixels, high nibble first; frame k = `sheet + 14*k`; drawn at
(x - xhot, y - yhot), transparent nibble skipped, other nibbles through LUT[lut] (OVL0 0x114C0 + 16*lut = DS:60C0).
Checks (V): every HDR length is a multiple of 14; every sheet is contiguous (offset[k] = offset[k-1] + bpr*rows), starts
at 2 and ends exactly at the DAT length; bytes per row = (w+1)/2 everywhere.

| sheet | frames | LUT | transparent | used LUT entries (hex) |
|---|---|---|---|---|
| RACE | 92 | 5 | 0 | 00 00 43 44 45 46 47 48 49 4A 26 4B 4C 2B 4D 4E |
| MED | 30 | 5 | 0 | (same) |
| LAPS | 4 | 4 | 14 | 00 3A 35 3B 3C 37 3D 38 1F 39 3E 3F 40 41 42 7C |
| RACE2 0..5, 8..19 | 18 | 3 | 1 (0..4, 8..15), 8 (5, 16..19) | 00 2C 2D 22 2E 2F 30 31 32 33 34 35 36 37 38 39 |
| RACE2 6, 7 | 2 | 5 | 0 | LUT 5 |
| RACE2 20..24 | 5 | 30 | 6 | 00 2C 2D 22 2E 2F 00 31 32 33 34 F3 F4 F5 F6 F7 |

Palette: `C:\work\Centurion-native\cmap.bin` (CMAP.BIN, 6-bit), unchanged by section 4.

### 2.1 RACE (from above; RACE_SCREEN_NOTES 5.7 for how they are assembled) (D; pictures V by render)

| frames | size | picture | used by node |
|---|---|---|---|
| 0..2 / 3..5 | 12x13 | driver holding the reins, brown / blue (player) | pose 0 |
| 6..12 / 13..19 | 40x29..31 | four horses galloping, 7 frames, brown / white with blue harness (player) | root |
| 20..23, 24..27, 28..31 | 33x17, 28x15, 25x13 | chariot body: heavy (red rim), medium (silver), light (plain), 4 frames each | chariot (83C4[type] = 28, 24, 20 for type 0, 1, 2) and the cards (87F6) |
| 32..38, 39..45, 46..52 | up to 29x55 | the chariot breaking up, wheel and driver thrown (heavy, medium, light) | crash node (83C8[type] = 46, 39, 32) |
| 53..55, 56..58, 59..61 / 62..64, 65..67, 68..70 | 8..10 x 9..12 | driver lashing (two sides) and whipping | poses 2, 3, 1 |
| 71..73, 74..76, 77..79 | | whips (black lines) | whip children |
| 80..83 / 86..89 | 15..19 x 16..17 | driver lying (G: dragged by the reins) | node 11 |
| 84, 85 / 90, 91 | 7x19, 11x18 | driver standing | node 10 (and the end of node 11's cycle) |

Every frame is used (D: the frame lists DS:838C/8398 plus 83C4/83C8/87F6 cover 0..91). Contact sheet:
`png\MRACE__race_x3.png`.

### 2.2 RACE2 (side view, parades) (D; V by the capture fit for frames 1, 5, 12, 14)

| frames | size, hotspot | picture | drawn at |
|---|---|---|---|
| 0..4 | 11x29 | the brazier's flame (frame 0 has 4 transparent pixels) | (292,35), cycling (C299) |
| 5, 6, 7 | 40..43x31 | chariot with a blue-clad driver: plain, gold, red (type 0, 1, 2) | (x+50, 142) |
| 8..11 | 58..64x44..47, hot (-1..-3, -2..-6) | four white horses galloping | (x, 130) |
| 12, 13 | 52x45, 55x47 | the horses standing / prancing | (x, 130) |
| 14, 15 | 17x15 | the near wheel, two rotation frames | (x+67, 158) |
| 16..19 | 39x31 | the plain chariot, driver raising his arms (winner) | (x+50, 142) |
| 20..24 | 14x18, hot (-18/-19, 0) | a figure in red; 21..24 move like 16..19 | (x+50, 142), triumph only |

All 25 used. G for frames 20..24: either the triumphator in the red/purple toga picta, or the slave who held the
wreath over him ("memento mori"); they are drawn over the driver of frames 5 / 16..19. `png\MRACE__race2_x3.png`.

### 2.3 LAPS (V by render)

Frame 0: 59x11, hot (4,4): a bar with three golden dolphins (lying); frames 1, 2, 3 (16x14, 15x14, 12x14; hot (-11,8),
(-20,8), (-28,8)): dolphin 1, 2, 3 tipped nose-down, each exactly over its lying twin when drawn at the same point
(134,185). `png\laps_sequence_x6.png`. The real Circus Maximus counted laps with seven dolphins (and seven eggs).

### 2.4 MED (V by render)

30 frames, all hot (0,0): 0..6 two bearers with an empty stretcher marked with a **red cross**, turning from the
diagonal to upright; 7..20 the injured driver (blue tunic, red trim) beside the stretcher, examined, lifted, getting up;
21..24 the driver laid on the stretcher; 25..28 the stretcher with the driver, carried diagonally; 29 the driver
standing alone (10x17). Order lists DS:8B04 / 8B20 (section 5). All 30 used. `png\MRACE__med_x3.png`,
`png\med_zoom.png`.

## 3. Pictures (CBM; format CBM_FORMAT.md) (V by render and fit)

| member | header (w, x, y, bpr, rows, transparent, LUT, size) | notes |
|---|---|---|
| CIRCUS | 320, 0, 0, 160, 61, 13, 4, 9760 | no nibble 13 occurs (V), so drawing it with 8E62 is opaque |
| GALLERY1 | 320, 0, 0, 160, 130, 0, 3, 20800 | 9,873 nibble-0 pixels; LUT3[0] = colour 0 = the black fill below, so the transparency is invisible |
| RACEEST | 320, 0, 0, 160, 70, 6, 3, 11200 | no nibble 6 occurs; blitted opaque by 8790 |
| GALLERY2 | 320, 0, 0, 160, 70, 6, **0**, 11200 | RACEEST's bytes, LUT 0 |

V: gallery1 at (0,0) + raceest at (0,130) + race2 sprites reproduce the DOSBox capture in 63,999/64,000 pixels
(RACE_SCREEN_NOTES 4). Pictures: `png\circus.png`, `png\gallery1_x3.png`, `png\raceest.png`, `png\gallery2_lut0.png`
(GALLERY2 with the identity table: noise in EGA colours, so it is not an EGA version like BB1/BB2).
The gallery statues (G, by eye): a discus thrower (Myron's Discobolus), a Hercules leaning on his club (Farnese type), a
Venus, a goddess or empress with a spear in an aedicula, a cloaked nude (Apollo Belvedere type), a togate figure, then
the brazier.

## 4. The race path (D; V by exact fit)

Polyline DS:896A..8A89: 72 (x, y) words, (-1, -1) = pen up, 18 of them. CBE6 rasterises it into calloc(368, 4) at
C308: horizontal/vertical runs one pixel per point (each run repeats its start point), single steps one point, every
pen-up two (-1,-1) points; at a pen-up the next point is read from [bx+4]/[bx+6], which for the final marker is
DS:8A8A/8A8C (the first card position): harmless, the loop ends. **The simulation writes exactly 368 points** (V): the
buffer size is the path length. Index 0 = (120,25) = finish line, 220 = (260,25) = start (C30A.. = 220), the oval runs
left along the top straight, round the left bend (x 104), right along y 39, round the right bend (x 275) and back left
along y 25 with 18 two-pixel gaps (x 123..266). Positions: racer +0 / 75 = path index (B190: `pos / 0x4B`); a lap =
368*75 = 0x6BD0 position units, subtracted at the wrap. `path.txt`, `png\circus_path.png`.

## 5. DS tables used for drawing (D, static DS = runtime DS in the snapshot unless noted)

| DS | content |
|---|---|
| 8022..808B | sprite-tree offset tables (s8 dx, dy per frame): 8022 root (0,0)x7; 8030 chariot (2,29) (4,29) (6,29) (4,29); 8038 / 8046 / 8054 crash, type 2 / 1 / 0 (x 4..12, y 29); 8062 poses (12,-2)x3; 806A node 11 (9,29)x6; 8076 node 10 (0,0)x2; 807A whip of pose 1 (11,-4) (8,-7) (6,-38); 8080 pose 2 (10,-2) (5,-7) (-20,-23); 8086 pose 3 (10,1) (3,3) (10,-20) |
| 808C..838B | the four sprite trees, 12 nodes of 16 bytes each, racer i at 808C + 0xC0*i (RACE_SCREEN_NOTES 5.7); the static image has the links, offset tables, counts and flags, BC15 fills in the descriptors |
| 838C / 8398 | frame lists in BCA0 pre-order, opponents 06 14 00 3B 4D 35 47 38 4A 20 54 50, player 0D 14 03 44 4D 3E 47 41 4A 20 5A 56 |
| 83A4 | active flags by pre-order number: FFFF FFFF FFFF 0 FFFF 0 FFFF 0 FFFF 0 FFFF 0 |
| 83BC | per racer: list pointer 838C x3, 8398 |
| 83C4 / 83C8 | per type: chariot body first frame 28 24 20 / crash first frame 46 39 32 |
| 83CC | tree roots 813C 81FC 82BC 837C |
| 83D2 | 837C: the player's tree (crash scene) |
| 83D4 / 83DC | clip rects (0,61,319,170) / (0,0,319,199) |
| 83E4 | per type: crash offset table 8054 8046 8038 |
| 840E | point (0,130) for 8790 |
| 8412 | parade stage thresholds 0, 1, 32, 33, 34 (bytes) |
| 8418 | brazier frame (runtime 2 in the snapshot) |
| 87D8 / 87DA / 87DC | "S", "H", "C" |
| 87DE | racer colours 4, 10, 1, 15 (cards, minimap dots) |
| 87F6 | card sprite per type 28 24 20 (= 83C4) |
| 896A..8A89 | path polyline (section 4) |
| 8A8A | card origins (6,6) (213,6) (213,117) (6,117) |
| 8AB4..8AF3 | neighbour/collision rectangles for D5E1 (logic) |
| 8B04 / 8B20 | crash-scene frame lists A (28) / B (26) |
| 8B3A / 8B72 | crash-scene offsets A / B (s8 pairs relative to (lane+9, y+29)): both start (-19,11) (-17,9) (-10,9) (-9,5) (0,4) (1,2) (1,-1) (1,-1) (2,-2) (4,-5) (4,-2) (4,2) (4,-1) x4 (1,-1); then A (-6,-2) (-7,-4) (-9,-2) (-9,-1) (1,-1) (0,-2) (1,-4) (1,-4) (3,-8) (7,-4) (8,-7), B (3,-2) (1,-4) (-1,-2) (0,-1) (4,1) (4,-4) (7,0) (9,-3) (15,-5) |
| 8BB0 | DE64 video-mode code pointers DEBA DEC4 DEFE DEDB |
| 8BB8 | DE64 bottom-clip masks 8000 C000 ... FFFF |
| 8BD8 | epilogue text; 8CD0..8EAC statistics fragments (RACE_INDEX 10) |
| A05C / A3B4 | the normal / small font blocks copied to 9B56 |
| AA78..AAB3 | 10 speckles (x, y, pattern), runtime only |

## 6. What the snapshot shows (V, values read from mem_race_640k.bin)

At the capture (BE10 phase 2): race sheets at DS:D062 (race), D5A6 (med), D74C (race2) [C2BC/C352/C354]; gallery1
descriptor C356 = (320, 0, 0, bpr 160, rows 130, tr 0, LUT 3, +0A = 2, segment 5151); C364 = loaded; bet 0; treasury 15;
odds 1/2/3; skill base 95 (AAC2 = 95 95 90 96); difficulties 0 0 0 0; racers: Red heavy from Dalmatia "Cautious",
Green heavy Hispania "Steady", Blue light Macedonia "Cautious", the player light; all at position 0x4074 with lanes
140, 210, 0, 70 (start offset rng(4) = 2), y 100; the 10 speckles mid-screen; mouse at (144,110).

## 7. Unused data, jokes and hidden things

1. **GALLERY2.CBM is unused** (D: no name string anywhere; V: byte-identical to RACEEST except the LUT byte 0). Drawn
   with LUT 0 it is noise, so it is a leftover copy, not an EGA variant.
2. **The stretcher has a red cross** (V by render): Red Cross emblem in ancient Rome, 1,900 years early (G: a joke).
   The crash scene has two endings by rng(2): the driver carried off, or the driver waving the medics away and staying
   on his feet while they leave with the empty stretcher.
3. **Agrippa's dolphins** as the lap counter (V by render): three golden dolphins tip over one per lap.
4. **Minimap dots hide behind the spina**: the path's 18 two-pixel gaps on the top straight (D; G that they line up
   with the monuments of circus.cbm on purpose, they do by eye).
5. **The winner raises both arms** for three seconds (RACE2 16..19) and, in the triumph, a figure in red joins him
   (RACE2 20..24, G: toga picta or the memento-mori slave).
6. The gallery's statues are recognisable classics (G: Discobolus, Farnese Hercules, Venus, Apollo Belvedere type).
7. Texts already listed in RACE_INDEX 10: the physician's herbs "'guaranteed' to make your horses win.  You doubt that
   this will work", the "squinty-eyed attendant" of the Vestal Virgins and her "small donation of 50 talents", "Utter,
   Ignominious Failure", the bribed driver "paid 10 talents to let you win" (the bribe lowers his top speed by
   rng(10) = 0..9 only), the epilogue "like those of Alexander ... nothing is forever".
8. No unused sprite frame: all 92 + 25 + 4 + 30 frames are referenced (D).
9. MRACE.DIR name fields keep leftover bytes after the NUL ("CN3B.LZW\0ZW", "MEDDAT.LZW\0W", in the snapshot's copy):
   the authoring tool did not clear the 13-byte field (D; no hidden name).

## 8. 1990 slips seen in the data/screens (candidates; none fixed, nothing in port/src)

1. Crash state 5 keeps the driver's pose node active, so the driver is drawn in the chariot and the node-11 figure
   lies beside it (D; G whether visible as a slip: the node-11 frames may show the same driver). Fix candidate only
   after a capture of a crash.
2. BA28's "deactivate all poses" loop (BA71..BA8A) is unreachable (D, harmless).
3. The victory boxes put the click line 1 px higher than 59D4 does (Y1 - 16, D): a difference to keep, not a slip.
4. The gauges are not redrawn while the player is crashed (D): they keep the last values until the crash scene; fine.
