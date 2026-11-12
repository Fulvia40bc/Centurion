# Gladiator show data (MGLAD, archive 6)

Claude, 2026-11-10. The data side of the gladiator show, overlay section 3 (`disasm/OVL_GLAD.asm`, index in
[GLAD_INDEX.md](GLAD_INDEX.md), screens in [GLAD_SCREEN_NOTES.md](GLAD_SCREEN_NOTES.md)). Marks: **D** decoded from the
disassembly or dumped from the data, **V** verified by an exact fit, an exact comparison or by drawing it, **G** guess.
Nothing was run in the game or in DOSBox.

Work files in `C:\Users\erikk\AppData\Local\Temp\claude\bfb5522be8ee045d\c4bf0f5f-57f9-4ea6-80b3-e1aa05bf0592\scratchpad\gladscr\`:
`in\` (copies of MGLAD.DAT/DIR), `raw\` (all members unpacked with `tools/extract_data.py`), `raw2\` (CENT, for the
MMENU frame), `png\` (contact sheets `sheet_*.png` at scale 2-3 with the frame number in yellow and the hotspot as a
magenta pixel; `coloseum.png`, `arenabtm.png`; composite screens `c*.png`, fight poses `f_*.png`, zooms `z_*.png`),
`gfx.py` (drawing model), `sheets.py` (sheets + checks), `comp.py` (screens), `ds.py` (DS tables). The naval
`fleet\data\hdr.py` dumps the headers (`python -I hdr.py raw`).

## 1. The archive (D, dumped; V for the coverage)

Archive 6 = MGLAD: DS:029E[6] = DS:63FA (name stored XOR "^ENNY", like MRACE), and the music table DS:3EF0 puts songs
4..7 (cn4a..cn4d) in archive 6. Section 3 opens it once (B0EF) and closes it at B28B, after the fighter sheets are
loaded (or at B1F9 on a cancel). 24 members; the DIR entries tile MGLAD.DAT from offset 4 to its end (72758) with no gap
and no overlap, so there is no hidden data between members (V). Every member is used (V against the load calls).

| member | fmt | packed -> unpacked | what | loaded by |
|---|---|---|---|---|
| COLOSEUM.CBM | C0 | 20120, CBM 320x200, LUT 20, tr 0 | title picture | B114 `1273("coloseum.cbm")` |
| ARENABTM.CBM | C0 | 5788, CBM 320x74, LUT 29, tr 0 | lower arena strip (sand, wall base) | B103 `13A8(4)` (picture slot 4, name DS:01F0) |
| ARENAHDR/DAT | 80 | 56->56 / 12030->26496 | 4 sprites: arena top, column, plinth, thumbs box | B173 `163B("arena")` -> [C2B4] |
| SHOWHDR/DAT | 80 | 85->98 / 3781->8068 | 7 card portraits | B162 `163B("show")` -> [C2BA] |
| RMANHDR/DAT | 80 | 171->168 / 5145->13390 | 12 sprites: the player's man | B260 `163B(DS:7BCC[body])` -> [C2B6] |
| RWIMPHDR/DAT | 80 | 148->140 / 3456->7907 | 10 sprites: the player's wimp | same |
| LMANHDR/DAT | 80 | 169->168 / 5070->13390 | 12 sprites: the opponent man | B281 `163B(DS:7BD0[body])` -> [C2B8] |
| LWIMPHDR/DAT | 80 | 145->140 / 3464->7907 | 10 sprites: the opponent wimp | same |
| LCAT0HDR/DAT | 80 | 76->70 / 1778->5994 | 5 sprites: Black Panther | same (body 2) |
| LCAT1HDR/DAT | 80 | 76->70 / 2936->5994 | 5 sprites: Wild Lioness | same (body 3) |
| LCAT2HDR/DAT | 80 | 76->70 / 2277->5994 | 5 sprites: Snow Leopard | same (body 4) |
| CN4A..CN4D.LZW | 80 | 774->1066, 2326->4938, 1555->3470, 1252->2471 | music 4..7 | B18B..B1A0 `5D73(4..7)` |

There are no RCAT sheets: a beast can only be the opponent (GLAD_INDEX 6). The EGA path does not differ here: the same
members are loaded for every video mode (no "vga" variant, unlike NAVL); only the health-bar grey depends on [9B26]. D

## 2. Sprite sheets (D; V by the checks and the renders)

Same format, loader (163B) and drawer (8E62) as the land and naval battles (NAVAL_DATA_NOTES 2): HDR = 14-byte entries
`<HhhBBBBI` (w, xhot, yhot, bytes per row, rows, transparent nibble, LUT, offset from 2 bytes before the DAT); 4-bit
pixels, high nibble first; drawn at (x - xhot, y - yhot); remap 0 in every call. **Checks on all 9 sheets (V):** every
HDR length is a multiple of 14; the frames are contiguous (each offset = previous offset + bpr*rows); the last frame
ends exactly at the DAT length.

LUTs used (OVL0 0x114C0 + 16*n = DS:60C0 + 16*n):

| LUT | entries (hex) | used by |
|---|---|---|
| 3 | 00 2c 2d 22 2e 2f 30 31 32 33 34 35 36 37 38 39 | LCAT2 frames 0..2, SHOW 6 |
| 12 | 00 2c 2d 22 2e 78 79 7a 32 33 34 35 36 37 38 39 | every fighter frame except LCAT2 0..2; SHOW 0..5 |
| 13 | 00 2c 2d 22 2e 78 79 7b 00 33 34 35 36 37 38 39 | ARENA 3 (thumbs box) |
| 20 | 00 68 1a 19 b9 ba bb bc 24 bd 25 be 26 27 28 29 | COLOSEUM.CBM |
| 21 | 00 c0 02 03 04 05 06 c1 c2 09 0a 0b 0c 00 0e c3 | ARENA 1, 2 (column, plinth) |
| 29 | 00 2c 2d 22 f1 78 19 f2 32 33 34 35 36 37 38 7a | ARENA 0 and ARENABTM.CBM (the two halves of the arena share it) |

### 2.1 ARENA (4 frames, hotspot 0,0 everywhere) V by render
| k | size | tr | LUT | content | drawn |
|---|---|---|---|---|---|
| 0 | 320x126 | 0 | 29 | the stands: crowd in three tiers, the imperial box between two columns with braziers, the gate | (0, 0) at B2BB |
| 1 | 25x92 | 1 | 21 | a Corinthian column | 8 times by B67C (DS:7C1A) |
| 2 | 76x8 | 13 | 21 | a stone plinth | 6 times by B67C |
| 3 | 156x62 | 8 | 13 | thumbs box: thumb up on blue (left half), thumb down on red (right half) | (79, 20) at BFE7 |

ARENA 0 (rows 0..125) and ARENABTM (rows 126..199) join without a seam (V by render, `c9_arena_start.png`).

### 2.2 SHOW (7 frames): exact copies (V)
SHOW frame k is byte-for-byte the frame 0 of another sheet, header included: 0 = RMAN 0, 1 = RWIMP 0, 2 = LMAN 0,
3 = LWIMP 0, 4 = LCAT0 0, 5 = LCAT1 0, 6 = LCAT2 0 (V, `sheets.py`). Index: facing right (code facing -1) = body,
facing left (0) = body + 2.

### 2.3 The fighters (V by render; frame meanings D from the move tables + by eye)
**RMAN = LMAN mirrored, RWIMP = LWIMP mirrored, pixel for pixel (V)**; only the hotspots are not exact mirrors: the L
xhot is 2..5 larger than `w - 1 - xhot` of the R frame (man frames 0, 2..8 by 4, 1 and 10 by 3, 9 by 2, 11 by 5; wimp
frames 0, 1, 5, 6, 7 by 4, 4 by 3, 2, 3, 8, 9 by 5), so an L sprite is drawn 2..5 px further left than a true mirror
would be. R sprites look LEFT (the player stands on
the right), L sprites look RIGHT. Man: transparent 0, wimp: 8, LUT 12, all frames.

| frame | man (12) w x rows, hot R / L | wimp (10) | used by states (man / wimp) | pose |
|---|---|---|---|---|
| 0 | 59x47, (14,46) / (48,46) | 49x41 | 0, 12, 15 / 0, 11, 13 | stand, shield forward |
| 1 | 59x47 | 48x41 | 1, 2 / 1, 2 | step |
| 2 | 53x47 | 50x37 | 3, 4 / 3, 4 | sword low, lunge (low attack; first frame of the high attack) |
| 3 | 58x48 | 55x41 | 10 / 10 | man: sword level at shoulder height; wimp: sword raised high |
| 4 | 55x47 | 47x32 | 11 (man) / 8 (wimp: kill) | man: sword raised high; wimp: downward stab |
| 5 | 52x41 | 37x37 | 8 (man: kill) / 5 | man: downward stab; wimp: kneeling low guard |
| 6 | 53x43 | 25x40 | 5 / 6 | man: kneeling low guard; wimp: high guard |
| 7 | 39x47 | 40x41 | 6 / 7 | high guards |
| 8 | 49x47 | 44x30 | 7 / 9 | man: high guard; wimp: falling |
| 9 | 44x43 | 43x17 | 9 / 12 | man: falling; wimp: lying dead |
| 10 | 57x24 | - | 13 | man: falling to the ground |
| 11 | 57x21 | - | 14 (dying state) | man: lying dead, blood |

Cats (LCAT0..2, 5 frames each, LUT 12 except LCAT2 0..2 = LUT 3): 0 stand (82x26, hot 65,24), 1 step (82x26), 2 strike
with a raised paw (96x30), 3 hurt/falling (84x29, blood), 4 dead (86x28, blood). Transparent nibble: LCAT0 and LCAT2 use 9
for frames 0..2 and 8 for 3..4; LCAT1 uses 8 throughout. The three cats are separate paintings, not palette swaps (the
nibbles differ in 1090..2849 positions per frame, V). LCAT2's change from LUT 3 to LUT 12 in frames 3..4 is deliberate:
those frames use nibble 6, which LUT 12 maps to the blood red (V by render: the white leopard bleeds).

All frames of every sheet are used by some state (V against the move tables below). No unused frame.

### 2.4 Move tables (D, dumped; identical in the static and runtime images)
4-byte states {dx (signed), next, frame, flags}; flags bit 0 = idle (accepts a command), bits 1-2 = attack zone (2 low,
4 high), bits 3-4 = guard (8 -> low 2, 0x10 -> high 4 after `>> 2`).

```
P man  7ED8: 0(0,0,0,01) 1(0,12,1,00) 2(15,15,1,00) 3(0,10,2,00) 4(0,15,2,02) 5(0,5,6,09) 6(0,6,7,11) 7(0,7,8,11)
             8(0,8,5,01) 9(0,13,9,00) 10(0,11,3,00) 11(0,15,4,04) 12(-16,15,0,00) 13(0,14,10,00) 14(0,FF,11,00) 15(0,0,0,00)
P wimp 7F18: 0(0,0,0,01) 1(0,11,1,00) 2(7,13,1,00) 3(0,10,2,00) 4(0,13,2,02) 5(0,5,5,09) 6(0,6,6,11) 7(0,7,7,11)
             8(0,8,4,01) 9(0,12,8,00) 10(0,13,3,04) 11(-8,13,0,00) 12(0,FF,9,00) 13(0,0,0,00)
O man  7F50: as P man with dx negated, except 10(0,11,3,04) 11(0,15,4,00)
O wimp 7F90: P wimp with dx negated
O cats 7FC8: 0(0,0,0,01) 1(0,5,1,00) 2(-20,7,1,00) 3(0,7,2,02) 4(0,6,3,00) 5(20,7,0,00) 6(0,FF,4,00) 7(0,0,0,00)
```
Pointers DS:7FE8 = {7ED8, 7F18} (player by body), DS:7FEC = {7F50, 7F90, 7FC8, 7FC8, 7FC8}; dying states DS:7ED0 =
{14, 12} (player), DS:7ED2 = {14, 12, 6, 6, 6} (opponent). A cat's only attack is a LOW one (state 3, zone 2), and a cat
has no guard. D

**Data slip (G, strong): the opponent man's high attack lands one step early.** The high attack is 3 -> 10 -> 11
(frames 2 -> 3 -> 4). The hit test runs when a state ends and uses that state's flags. Hit flag 04 sits on state 11
(frame 4, sword raised high) for the player's man, but on state 10 (frame 3, sword still level) for the opponent's man.
For the wimps (both sides) the hit flag is on the frame where the sword is raised high (wimp frame 3). The man has
one extra pose (frame 3, level) that the wimp does not have, and the player's table puts the hit on the raised pose
like the wimps do. So the O man table is the odd one: its high attack connects 6 frames (24 ticks, 0.33 s) sooner than
any other fighter's and before the sword is up. Fix proposal (not under --original): O man 10 = (0,11,3,00),
11 = (0,15,4,04), i.e. the same as the P table. V that the poses are as described (`f_Pman_state10.png`,
`f_Pman_state11.png`, `sheet_rman.png`, `sheet_rwimp.png`); G that it is a slip and not a deliberate "faster AI".

The bytes 0x3FC past each table (state FF, DS:82D4/8314/834C/838C/83C4: 00 00 00 00 / 00 00 FF FF / 3C 83 00 00 /
06 14 00 3B / 1C 18 14 00, same in both images) are other data. They are read only for the flags test of the last pass
(see GLAD_SCREEN_NOTES 5); never moved or drawn. D

### 2.5 Fighter hotspots and the arena (V by render)
Every fighter is drawn at y 175: the feet stand on the sand of ARENABTM (rows 126..199); P starts at x 192, O at 127.
The kill pose (state 8) after the walk-over puts the stab on the body (`c11_thumbs_down_kill.png`).

## 3. The CBM pictures (D; V by render)
| member | header (w, xo, yo, bpr, rows, tr, LUT, size) | nibbles used | drawn by |
|---|---|---|---|
| COLOSEUM.CBM | 320, 0, 0, 160, 200, 0, 20, 32000 | all 16 | 1273 + 8E62 at (0,0), transparent nibble 0 over a black fill (LUT 20[0] = 0: same result), then 86A0 frees it |
| ARENABTM.CBM | 320, 0, 0, 160, 74, 0, 29, 11840 | 0-4, 6, 10-15 | 13A8 into picture slot 4 (descriptor DS:C0BE), 8790 opaque at (0, 126) |

Both unpack to exactly their size (V). COLOSEUM: the Colosseum from above, packed with spectators, a temple front and
gardens around, flags at the main entrance (`coloseum.png`). The name is spelt "coloseum" (one s) in DS:7B89 and in
the archive.

## 4. The DS data of the show (D; dumped, identical static/runtime)

| DS | content |
|---|---|
| 7B06..7B28 | sheet names "rman", "rwimp", "lman", "lwimp", "lcat0", "lcat1", "lcat2" (pointers 7BCC = player {rman, rwimp}, 7BD0 = opponent {lman, lwimp, lcat0..2}) |
| 7B2E..7B5F | "Trainee", "Slave", "Veteran", "Master" (7C6E) / "Fat", "Well-Fed", "Hungry", "Ravenous" (7C76) |
| 7B68 | " (20 Talents)", digit at 7B6A patched to '1'..'4' (pointer 7C7E) |
| 7B76 | "How Much Training?" |
| 7B89, 7B96, 7B9B | "coloseum.cbm", "show", "arena" |
| 7BA1, 7BBF, 7BC6, 7BC7 | "Can't Load Gladiator Graphics", "VERSUS", "", "" |
| 7BC8 | point {0, 126} for 8790 |
| 7BDA..7C19 | 16 hlines {colour word, y byte, 0}: see GLAD_SCREEN_NOTES 4.2 |
| 7C1A..7C6D | 14 {x, y, frame} for the columns and plinths |
| 7C80..7CAF | 4 menu items {text (run time), hotkey '1'..'4', 0, rect (6, 23+10i)-(138, 33+10i)} |
| 7CB0 | menu record: box (91,79)-(241,197), colour 0, items 7C80, title 7B76, 4 items, single FF, enabled FFFF |
| 7CC2..7DAD | the selection messages and "Bouts Won" |
| 7DB8..7E2F | names (with a trailing space): Flavius, Marcus, Brutus, Marius, Tillius, Cassius, Flaminius, Spartacus, Julius, Publius, Black Panther, Wild Lioness, Snow Leopard |
| 7E3E..7EBF | 13 gladiator records of 10 bytes (GLAD_INDEX 9; records 0..9 are saved, 10..12 not) |
| 7EC0, 7EC8 | thumbs rects (79,20)-(156,81), (157,20)-(234,81): the two halves of ARENA frame 3 exactly (V by render) |
| 7ED0..7FE7 | dying states and move tables (2.4) |
| 7FF6 / 8000 | AI close-range moves {6,6,7,7,5,5,4,3,2,2} / {6,7,5,4,3,1,2,5,4,3} |
| 800A | music by mood {7, 5, 6}; 800D AI pace accumulator (0) |
| 800E | "Thumbs Up or Down?" |

Every string in DS:7B06-0x801F is referenced by the section (scan of the block, V); no unused text.

## 5. Music CN4A..CN4D (D for the use; the song format is the sound driver's, not decoded)
All four start `7F 00 00 01 00 00 01 01 00 01 ...`, the same header family as CN6/CN9. 4 cn4a = title, the pause before
the arena; 5 cn4b = VERSUS, fight start, crowd mood 1; 6 cn4c = crowd mood 2 (excited); 7 cn4d = selection, crowd mood 0
(bored). The show uses all four music slots, so it unloads the map song 1 first. Details: GLAD_SCREEN_NOTES 7.

## 6. Findings
1990 data slips (fix by default, keep under --original):
1. O man move table: hit flag on state 10 instead of 11 (2.4). G (strong), cosmetic-to-gameplay (timing of the AI's high
   attack).
2. Crowd meter rest fill ends at 221 instead of 227 (GLAD_SCREEN_NOTES 4.8). D, cosmetic.
Not slips (checked): LCAT2's LUT change in frames 3..4 (blood); SHOW being copies; R/L sheets being mirrors.

Easter eggs and jokes (data side; all kept):
- The two body types are literally named **"man" and "wimp"** in the sheet names (RMAN, RWIMP, LMAN, LWIMP); the wimp
  is smaller, steps 7-8 px instead of 15-16, has no "level sword" pose, and Spartacus starts as one.
- The roster puts **Julius** among **Brutus, Cassius and Tillius** (Cimber), Caesar's assassins, plus **Spartacus** and a
  **Flavius** (G for the intent). The beasts are a **Black Panther**, a **Wild Lioness** and a **Snow Leopard**; the white
  leopard bleeds red in its last two frames.
- Beasts are "trained" by hunger: **Fat, Well-Fed, Hungry, Ravenous**, 10..40 talents; the men's second grade is
  **"Slave"**.
- The thumbs box is the only "UI" sprite: two painted hands.
- Dead gladiators come back as namesakes with a numeral ("Flavius II").
- No hidden pictures, unused frames or unused members: every member, every frame and every string is used, and the
  archive has no gaps (V). Zooms of the stands, the imperial box and the gate (`z_*.png`) show nothing hidden.
