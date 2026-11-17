# Remaining pixel differences against the DOSBox captures

**Status 2026-11-17: section 5 is in the tree** (game.c's hand-built province screen deleted, province.c province_dump,
the 13A8 last pixel under `--original`) and the cursor is ported (CURSOR_NOTES.md). Now (V): province_rome 93 px (the
capture's arrow at (140,125); a .CSV holds no cursor), province_narbonensis with snap_back1.bin **0 px** (the
snapshot's arrow drawn by the port), province24 with erik_275BC.csv 85 px; map / game_menu 84 (83 with `--original`);
screen_map 90 / 89; naval `after` and the resumes to 2C27 0 px.

Claude, 2026-11-17. Every whole-frame comparison between the port and a DOSBox capture that still reports differing pixels,
with the cause read from the code (EXE segment 0823, `disasm/EXE_RUNTIME.clean.asm`) and the fix.
Labels: **D** = decoded from the code, **V** = verified by a run against a capture, **G** = guess.
Side exe for every number below: `C:\work\Centurion-native\centurion-framediff.exe`, built from the tree at c0505d0 with
0 warnings. Prototype exe (the patch in section 5): `centurion-framediff-proto.exe`. Runs from C:\work\cent_play.

## 1. Summary

| check | today | cause | after the patch |
|---|---|---|---|
| `--dump-frame province_rome` vs scr_rome_province.bin | 707 px | the dump draws game.c's old **hand-built** screen, not province.c (section 2) | 93 px = the cursor |
| `--dump-frame province_narbonensis` vs scr_narbonensis_province.bin | 763 px | the same | 85 px = the cursor (with the dice of section 2.4) |
| real path: `--play --load fresh_275BC.csv`, click Rome (125,114), shot | 93 px | the mouse cursor only | 93 |
| real path: `--play --load` Narbonensis snapshot stepped back 1 roll, click (83,109), shot | 85 px | the mouse cursor only | 85 |
| `--dump-frame map --save fresh_275BC.csv` vs scr_newgame_map.bin | 84 px | cursor 83 + **(319,199)**: a 1990 bug of the picture loader (section 3) | 84 by default, **83** with `--original` |
| `--dump-frame game_menu --save fresh_275BC.csv` vs scr_game_control_menu.bin | 84 px | cursor 83 + (319,199) | 84 / **83** with `--original` |
| real path map / Game Control (script: shot, click 295 10, shot) | 84 / 84 | the same | 84 / 83 with `--original` |
| `--dump-frame map` vs screen_map.bin (2026-11-06) | 90 px | cursor 89 (clipped at the bottom) + (319,199) | 89 with `--original` |
| race parade (scr_chariot_race.bin) | (319,199): port 0x2C, game 0x36 | section 3 | 0x36 with `--original` (V) |
| cmp_naval_resume B59F/BB7E -> 2C27, cmp_naval_frame `after` | 93 px | the cursor | 93 |
| cmp_naval_resume B09E -> BEAE with b2_flagship_trireme.txt | 78 px | the script ends the intro before its first pennant frame (README) | 78 (0 with ..._intro.txt) |
| cmp_battle_frame tick 0 / 245, --report-shot, cmp_naval_frame duel / intro, the other naval resumes | 0 px | | 0 |

So the province screens' "Mode 1" frame sprites are already pixel-exact in the game (province.c, `0x30E6`): the 707 / 763 came
from a test path that never runs them. Apart from the cursor, the only real difference left on any captured screen is the
last pixel of the 13A8 pictures, a 1990 bug the port already fixes (its `--original` copy is missing).

The numbers in HANDOFF.md / STAGE4.md / GAMELOOP_NOTES.md section 7 (map 292, game_menu 2,104 / 2,254, Rome 707, Narbonensis 763)
are stale: map and game_menu are 84 today (V).

## 2. The province screens: 707 / 763 px

### 2.1 What draws what
- `--dump-frame out province_rome|province_narbonensis` (main.c -> game.c `game_dump_frame`) calls game.c `draw_province()`, the
  Stage 4x reconstruction: a flat `box()` of `ui_rect` stripes (colours 25/26/27/28/30/0) in place of the frame sprites, the title by
  `ui_text_centered(160, 66)`, three ornament lines without caps, the crowd at a measured (255,104), and literal province values
  (`province_t narb = {"Narbonensis", ..., 5279, 20, 0}`). Nothing in the game loop calls it.
- The game (`--play`) draws the screens with province.c, the literal translation: `0x6CB8` / `0x7366` -> `0x708A` strips,
  `0x7E74` fill, `0x8BE4` title at (0xA0, 0x41), `0x3512` (frame + "Rank / Talents"), `0x6F0C` panel, `0x6973` copy, then
  `0x5614(rec, 1)` = `0x30E6` frame + items, ornament and title.

### 2.2 The Mode 1 frame (D, V)
`0x30E6(x = [bp+6], y = [bp+4])` (`ret 4`): `[9AFA] = [6346]`; `7E74(x+4, y+3, x+0x92, y+0x71)`; then `8E62([C0F6] + k, x, y)` for
k = 0, 0x0E, 0x1C, 0x2A (0x3115..0x312B), i.e. the four 14-byte descriptors of the MMENU sheet (CENT archive, `MMENUHDR.LZW` /
`MMENUDAT.LZW`), each at (x - xhot, y - yhot), nibble 9 transparent, colour table 26:

| entry | size | hotspot | lands at (frame at x, y) | part |
|---|---|---|---|---|
| 0 | 151x6 | (0, -113) | (x, y+113) | bottom edge |
| 1 | 6x109 | (0, -4) | (x, y+4) | left edge |
| 2 | 6x109 | (-145, -4) | (x+145, y+4) | right edge |
| 3 | 151x4 | (0, 0) | (x, y) | top edge |

Callers: `0x5614` mode 1 at the menu record's (x0, y0) = (6, 79) (records 5790 / 5A30 / 6054 / 60AE), `0x3512` at (164, 79).
province.c `frame30e6` / `sprite8e62` do exactly this, and the screens they produce differ from both captures **only in the
cursor** (V, table in 1).

### 2.3 Where the old dump's 707 / 763 come from (V: old dump vs capture, split by region; every pixel outside the cursor is
one the real path draws right)

| region | Rome | Narbonensis | why the old dump differs |
|---|---|---|---|
| left frame (6,79)-(156,197): MMENU 3 / 1 / 2 / 0 | 131 / 71 / 33 / 27 | the same | flat stripes instead of the sprites' texture (mostly 26 <-> 27 <-> 28 swaps, e.g. row 81 and the top-right corner (151..156, 79..97)); a few corner pixels are the sprites' nibble-9 holes, where the capture keeps the 0x6344 fill (116) |
| right frame (164,79)-(314,197) | 62 / 71 / 63 / 27 | 62 / 71 / 60 / 18 | the same |
| title, row 0x41 | 58 | 134 | `8BE4(0xA0, 0x41)`: y 65, x = 160 - (sum(w+1) >> 1); the old dump draws at y 66 with its own centring |
| crowd (251..254, 114..139) | 57 | 57 | `0x6F0C`: `8E62([C28A] + 14 * mood, 0x118, 0x83)` = (280 - 29, 131 - 27) = (251, 104); the old dump puts the top-left at (255,104). The 30 "border over the sprite" pixels at x 309..311 reported on 2026-11-07 are the MMENU 2 edge, not an overlap: the sprite ends at x 307 |
| ornament caps (13..19, 100..102) and (13..19, 171) | 14 | 14 | `0x56B6` single ornament: lines x sx+2/4/6 from yt to yb plus the cap rows (sx+1..sx+7) at yt and yb; the old dump draws the three lines only |
| cursor | 93 | 85 | section 4 |
| **total** | **707** | **763** | |

### 2.4 Narbonensis and the dice (D, V)
`0x70BA(z)` for z != 0 rolls `si = rng_roll(100)` (`0x076B`) and draws a cloud picture `skyN.cbm` when `B(58A6 + z) <= si`. The
snapshot `mem_narbonensis_province_640k.bin` was cut on the screen, after that roll: its dice DS:00C8/CA = 07E1 / 1F35 give roll 76
>= 30 (`B(58A6 + 24)`), so a run from it draws clouds (2,619 px in rows 0..15). The generator is invertible (s1' = 0x7ABD s1 +
0x1B0F, s2' = 0x1B0F s2 + 0x7ABD, both multipliers odd); one step back gives roll 9 < 30, no clouds, as captured. With DS:00C8/CA
set to that state (`snap_back1.bin` in the scratch folder) the port's screen is the capture but for the cursor. The check needs
that one-step-back state; a save gives the dice of its own run (erik_275BC.csv happens to roll under 30 too).

## 3. The last pixel of every 13A8 picture (1990 bug; D, V)

`0x13A8(arch, n)` -> `0x13F8` loads picture slot n (DS:0250 + 8n: name, flags, descriptor) in VGA mode (`[9B26] = 2`) like this:
- `[bp-0x12]` = the packed size S (bytes/row x rows); the buffer is 2S bytes in segment `[C0C6]` (= 3942 in the snapshots);
- `[bp-0x16] = S - 1` (0x151C..0x1520): the packed bytes are read / unpacked (`0x1DC`) to offset **S - 1** of that buffer, and the
  header's +0x0A/+0x0C (`[bp-0x1A]` / `[bp-0x18]`) are set to that offset and segment;
- `0x88A7(desc, hdr)` then expands them **in place**, forwards: for each byte at `ds:bx` it writes `t[byte >> 4]` (0x8906..0x890F,
  `stosb` to `es:di`, di from 0) and then **reads `[bx]` again** for the low nibble (0x8910, `and ax, 0xF`).

For byte k, di = 2k and bx = S - 1 + k: they meet only at the last byte (k = S - 1), where the first `stosb` overwrites the source
byte with the colour of its high nibble before 0x8910 re-reads it. So the last pixel is `t[t[hi] & 15]` instead of `t[lo]`.

| picture (slot) | last byte | correct `t[lo]` | 1990 `t[t[hi] & 15]` | evidence |
|---|---|---|---|---|
| map.cbm (0), LUT 10 | 88 | 111 (6F) | 105 (69) | V: scr_newgame_map, scr_game_control_menu, screen_map.bin and the picture buffer 3942:FFFF in mem_map_gamemenu_640k.bin all hold 105 |
| raceest.cbm (3), LUT 3 | 11 | 44 (2C) | 54 (36) | V: scr_chariot_race.bin (319,199) = 36; the prototype's `--original` parade shot ends 45,44,44,54 like the capture |
| arenabtm.cbm (4), LUT 29 | DD | 55 | 242 | D only (no capture of the arena) |
| bb1.cbm / bb2.cbm (1, 2) | | | | EGA names; the VGA battle draws its field another way (battle_gfx.c, 0 px) |

This settles RACE_SCREEN_NOTES.md open point 1 ("(319,199) of RACEEST: our LZSS decoder vs the game's", G) and the 63,999 / 64,000
of CBM_FORMAT.md: the LZSS decoder is right, the expander is not. The port decodes the picture correctly, so the fix is already
the default; what is missing is the `--original` copy (the port rule), listed in the headers of map.c, race.c and glad.c.

## 4. The mouse cursor (not drawn by the port)

All other remaining pixels are the EXE's own software arrow (decoded separately in docs/CURSOR_NOTES.md: INIT sheet entry 0, drawn
by the game, not by the driver; the port's screen.c and map.c headers say it is not ported). Shape (V, cut from
scr_rome_province.bin): 11x16, colour 15 inside, 0 outline, 93 pixels, top-left at the pointer ([B6], [B8] = (296,10) in
mem_map_gamemenu, (310,185) in mem_narbonensis; the captures' arrows start there). The count is 93 minus the pixels the screen
already has in the arrow's colour, minus what is clipped: map 83 (10 already 0/15), Game Control 83 (10), Narbonensis 85 (87 on
screen, 2 already), screen_map.bin 89. In scr_newgame_map.bin the arrow starts at (124,114), one pixel left of Rome's anchor
(125,114) that `0x494A(0)` warps to via 0x066A (CURSOR_NOTES.md: the warp reads back with x rounded down to even).
Once the port draws the cursor, every check in section 1 should read 0 (with `--original` for the 13A8 pixel).

## 5. The fix (prototype; patch file in the scratch folder, `finish/framediff/prototype.patch`)

1. **province.c**: move 0x6CB8's drawing (0x6CC6..0x6D2F: `708A`, 7E74, 8BE4 title, 3512, 6F0C, 6973) into
   `static void province_draw6cb8(uint8_t z)`, called by `province_screen`; add
   ```c
   void province_dump(int zi) {   /* 0x7366 / 0x6CB8 up to their first 54C8 */
       uint8_t z = (uint8_t)zi;
       if (z == 0) { rome_draw7411(); menu_draw5614(0x5A30, 1); return; }
       province_draw6cb8(z);
       B(W(0x579A) + 0x33) = coastal2a07(z) == 0xFF ? 0xFF : 0;   /* 0x6D32 */
       menu_draw5614(0x5790, 1);
   }
   ```
   and declare it in screen.h.
2. **game.c** `game_dump_frame`: the province branch becomes state_init, `state_load_mem` for a `.bin` / `state_load` for a save,
   `screen_open(1)`, `map_startup()`, `province_dump(z)` (z = 24 for "province_narbonensis", else `atoi(screen + 8)`, so
   "province_rome" = 0 and "provinceN" = N), `memcpy(g_work, g_scr)`. Delete the hand-built screen: `box`, `province_t`,
   `ds_str`, `ds_table_str`, `draw_province`, `prov_name`, `load_province_csv`, `draw_province_rome`, `blit_cbm_at`,
   `blit_cbm_at_trans` (the prototype fences them with `#if 0`; the build stays at 0 warnings either way).
3. **The 13A8 last pixel under `--original`** (and a header line in map.c, race.c (bug 14) and glad.c (bug 8)):
   - map.c `map_image_load`, after the expansion: `if (g_original_bugs) g_mapimg[SCREEN_W * SCREEN_H - 1] = t[t[px[w * h - 2] & 0xF] & 0xF];`
   - glad_gfx.c `gl_picture_13A8`: compute `last = t[t[px[w*h-2] & 15] & 15]` before the in-place LUT pass, then
     `if (g_original_bugs) px[w * h - 1] = last;`
   - race_gfx.c `rc_picture_13A8`: after `pic_load`, `if (g_original_bugs) p->px[n - 1] = B(0x60C0 + 16 * p->lut + (p->px[n - 2] & 15)) & 15;`
     (the nibble that `rc_blit_8790` maps to `t[t[hi] & 15]`).

Results with the prototype (V): `--dump-frame province_rome --save fresh_275BC.csv` 93 px (cursor); `province_narbonensis --save
snap_back1.bin` 85 (cursor), `province24 --save erik_275BC.csv` 85; `--original --dump-frame map` 83 and `game_menu` 83 (cursor
only; 84 without `--original`); `--original` race parade (319,199) = 0x36 as captured. Regression set unchanged with the
prototype: cmp_endturn t1..t11 0 bytes, cmp_battle b11 / b10 identical, cmp_battle_frame tick 0 / 245 0 px, zone-check 37/37,
naval duel 0 px, naval resume BB7E -> 2C27 0 DS bytes / 93 px (cursor).

## 6. Reproduce

```
cd C:\work\cent_play
centurion-framediff.exe --dump-frame OUT province_rome --save C:/work/Centurion/tools/dosbox/results/saves/fresh_275BC.csv   (707)
centurion-framediff.exe --play --seed 1 --load .../saves/fresh_275BC.csv --script rome.txt SHOTDIR   (rome.txt: "click 125 114", "shot rome", "end"; 93)
centurion-framediff.exe --play --load snap_back1.bin --script narb.txt SHOTDIR   (narb.txt: "click 83 109", "shot narb", "end"; 85)
```
`snap_back1.bin` = mem_narbonensis_province_640k.bin with the words at linear 0x16860 + 0xC8 / 0xCA set to one generator step back
(section 2.4). The diff script `fd.py` (counts, regions, a side-by-side PNG) is in the same scratch folder.
