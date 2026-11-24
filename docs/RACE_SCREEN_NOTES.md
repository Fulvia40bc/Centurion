# Chariot race and victory sequence: the screens, the input and the timing (OVL0 section 4)

Claude, 2026-11-10. Scope: what section 4 draws, when, where and into which buffer; its texts, sprites, fonts, palette,
music, input and timing. Source: `disasm/OVL_RACE.asm` (addresses = 0823 offsets), index `docs/RACE_INDEX.md` (function
roles, DS map, call graph), EXE helpers in `disasm/EXE_RUNTIME.clean.asm` / `EXE_RUNTIME.bin`, the static DS (OVL0 file
0xB400 = DS:0000) and the DOSBox race snapshot `tools/dosbox/results/mem_race_640k.bin` (DS at linear 0x16860) with its
screen `scr_chariot_race.bin` (both taken during the pre-race parade, 2026-11-06). Data formats: `docs/RACE_DATA_NOTES.md`.
The race logic (AI, bumps, results) belongs to the index and a logic note; it appears here only where the screen
depends on it.

Marks: **D** decoded from the code or dumped from data; **V** verified (by an exact fit to the DOSBox capture or the
snapshot, or by a render); **G** guess. Nothing of section 4 has been run in the port. The V marks come from python
models in the scratch folder (`racedata\scripts`, listed in RACE_DATA_NOTES section 0).

Calls are Pascal: the first argument pushed is the highest `[bp+N]`, the callee pops. Below, arguments are written in
push order = source order: `7E74(x0, y0, x1, y1)` fill in [9AFA], `3134(x0, y0, x1, y1, fill)` bevelled box,
`8E62(desc, x, y)` sprite/CBM at (x - xhot, y - yhot), `8BE4(str, cx, y)` centred text, `899C(str, x, y)` left text.

## 0. Summary for the implementer

1. Five screens: the **pre-race screen** (circus picture on top, racer cards, the purchase / prepare / bet / skulduggery
   menus), the **gallery parade** (a chariot drives into the stands' arena, until a key), the **race screen** (minimap
   in the circus picture, a vertically scrolling track view, a dashboard), the **crash scene** (stretcher bearers,
   inside the track view) and the **winner's parade**; the victory sequence adds two **statistics boxes** over the map
   and the **triumph** (10 chariots in the gallery). D; the parade frame is V (section 4.2).
2. One logic-and-draw pass every **7 timer ticks** (72.8 Hz timer, 96.2 ms, 10.4 passes/s) in the race, the parades and
   the crash scene: `00F8` (slot 0 = now) at the top, busy-wait `01AE() >= 7` at the bottom. The race is therefore
   tick-locked: one race tick = one pass, whatever the CPU speed (as long as a pass fits in 7 ticks). D
3. **Drawing routines change game state and roll dice** (section 9): B11C (track speckles) rolls 2 dice per respawn and
   is called from the race tick; BD0A advances the sprite animations that the racer state machine waits for; the
   gallery parade rolls rng(25) on every frame until the player presses a key, the winner's parade rng(15) for 34 frames,
   the triumph 10 x rng(3); the crash scene rng(2). A headless run must run the same drawing calls on the same passes. D
4. Buffers (VGA, [9B26] = 2): during the race the view is composed in the work buffer and copied to the screen by
   rectangles (6973), the minimap dots and the dashboard are drawn **directly on the screen**; the parades compose whole
   frames in the work buffer and present them with 6887. No palette writes anywhere in section 4. D
5. Two fonts: the normal font (DS:A05C) everywhere, the small 6-px font (DS:A3B4) only for the dashboard letters
   "S", "H", "C". D

## 1. The sequence of screens

### 1.1 The race (entry D08A, EXE 0x349D "Racing Chariots") (D)

| step | code | what the player sees | buffer / target |
|---|---|---|---|
| 1 | D08A `B(C350) = -1`, B0E0 | nothing (10 speckles rolled: 30 dice) | - |
| 2 | C2DE c2f1..c34c | sheets race/laps/med (BC15), gallery1/raceest/race2 (BDA4), circus.cbm loaded; songs 3 and 2 loaded | - |
| 3 | c32d, c373 | **circus.cbm** drawn at (0,0) (rows 0..60, over the Rome screen); rows 61..199 filled with colour 0x11 (VGA) / 7 | screen (G: the caller's target) |
| 4 | c3b1 | three opponent cards (CDCE 0..2), section 3.1 | screen |
| 5 | c3c8 | menu DS:88E6 "Race what type of Chariot?" (mode 2: 3134 box) at (6,117)-(156,197) | screen |
| 6 | c41f..c42d | after a purchase: fill (6,89)-(157,199) colour 0x11, the player's card (CDCE 3) at (6,117), then message_box(DS:3E02 "") = a bare "Click to continue" box (G: a pause to look at the card) | screen |
| 7 | c455..c46d | circus redrawn at (0,0), rows 61..199 refilled (cards gone) | screen |
| 8 | c530..c5b4 | prepare loop: each pass 3512 panel frame + "Rank/Talents" (right half, (164,79)..), 6F0C(0) the Rome info panel, 44E5 treasury minus bet at (211,167), menu DS:88F8 "How Shall We Prepare?" (mode 1: 30E6 frame) at (6,79); sub-menus Skulduggery (DS:891C) and Bet (DS:890A) at the same place; message boxes | screen |
| 9 | c5c1 BE10 | **gallery parade** (section 4.1), song 2 (cn3a); any key or button ends it | work -> screen (6887) |
| 10 | c5cc..c6b9 | **race screen**: circus at (0,0), rows 61..199 colour 0x11, dashboard (section 5.3), gauges, lap dolphins | screen |
| 11 | c6c8, c6d3 | 691B(0,0,319,199) screen -> work (the work buffer now holds the race background); mouse warped to (160,100) | - |
| 12 | c6dd..c6fb | path built (CBE6); work buffer view (0,61)-(319,170) filled 0x11; target = screen | work |
| 13 | D08A d0b5..d0c2 | song 3 (cn3b) starts; `011F(0x12)`: 18-tick pause (0.25 s) | - |
| 14 | D08A loop | the race, one pass per 7 ticks (section 6); the player's crash scene (D738) inside a pass | work -> screen rects |
| 15 | d14e..d16e | music stopped, songs 2 and 3 unloaded | - |
| 16 | C9B6 | result message box(es) over the last race frame; after a win: the **winner's parade** BF6A(0) (song 2 reloaded) | screen; parade work -> screen |
| 17 | D08A d174 | gallery pictures freed; return to the EXE, which redraws the Rome screen (rome_games returns -1, GAMELOOP_NOTES 4) | - |

Cancel ("Exit" + "Are You Sure / You Don't Want To Race?" Yes): C70F frees everything and unloads songs 2 and 3,
C2DE returns 0; the screen stays as it is until the Rome screen is redrawn by the caller. D

### 1.2 The victory sequence (entry DF58, EXE 0x09A4, rank Caesar) (D)

| step | code | what the player sees | buffer / target |
|---|---|---|---|
| 1 | DF58 | music_load(0) (cn1, archive 3 CENT); 1D19 (G: mouse cursor hidden) | - |
| 2 | DF74 | box 1 "THE MAKING OF A CONQUEROR ..." over the current (map) screen, wait for a key/click, screen restored | screen (691B/6973 save/restore) |
| 3 | DF74 | box 2, the epilogue DS:8BD8 ("The known world lies at your feet. ..."), same handling | screen |
| 4 | BDA4, BF6A(1) | the **triumph**: 10 chariots in the gallery, song 0 (cn1); ends after 76 frames of the last stage or a key | work -> screen (6887) |
| 5 | C2C2, B0CE | pictures freed; thunk B0CE = section 1 B9A1, the credits (song 0 keeps playing: G, nothing stops it) | - |

## 2. Buffers and helpers (D; VGA path)

Same EXE helpers as the naval battle (NAVAL_SCREEN_NOTES section 2; descriptors 9AB6 screen, C1DA work, C0D0 current;
their clip rects at +0x0A..+0x10: 9AC0.., C1E4.., C0DA..). Section 4 uses: 67E4 (target = work), 680F (target = screen),
6837 (flip if needed, target = screen), 6887 (flip: whole work buffer -> screen, target = work), 691B / 6973 (rect
screen -> work / work -> screen), 7E74 fill, 82DE putpixel, 979C line, 8E62 sprite/CBM, 8790 blit of a cached picture
(slot 3 = raceest.cbm, descriptor DS:C0B6, at the point DS:840E = (0,130)), 899C / 8BE4 text, 3134 box, 59D4 message box,
5C20 yes/no box, 5614 / 54C8 / 585B menus.

**3233(x0, y0, x1, y1, colour)** (EXE 0x3233, decoded here): `if (colour != -1) { [9AFA] = colour; 7E74(x0,y0,x1,y1); }`
then colour 8: lines (x0,y0)-(x0,y1), (x0+1,y0+1)-(x0+1,y1-1), (x1,y0)-(x1,y1), (x1-1,y0+1)-(x1-1,y1-1); colour 0:
hlines y1 (x0..x1) and y1-1 (x0+1..x1-1); colour 15: hlines y0 (x0..x1) and y0+1 (x0+1..x1-1). = 3134 without the inner
line at +3. D

**Clip rects.** DS:83D4 = (0,61,319,170) (the track view), DS:83DC = (0,0,319,199). B190 copies 83D4 into the current
clip C0DA for the speckles and the chariots and 83DC back before the finishing line and at the end of the tick. D738
writes 83D4 into all three clips (C0DA, C1E4, 9AC0) and 83DC into all three at its end. D

**Video modes.** [9B26]: 2 = VGA (the port), 6 = a mode with its own double buffering in the race (D11E/D133: D3C2(0)
before the wait, 6887 after it; B1D6: the old minimap dots are erased with putpixel colour 7 on the screen; D158: 6837
at the end). DE64 dispatches on DS:8BB0[[9B26]] = DEBA (mode 0: nothing), DEC4 (2: VGA, 8-bit), DEFE (4: EGA planar,
graphics-controller ports 3CE), DEDB (6: 4-bit packed, 2 pixels per byte; G: Tandy/PCjr). Non-VGA colours: view 7
instead of 0x11, dashboard frame filled with [6346] instead of 0. Only the VGA path is described below. D

## 3. The pre-race screen (C2DE) (D; card layout V by a render)

### 3.1 Racer cards CDCE(i)

Card origin (x, y) = DS:8A8A[i] = (6,6) Red, (213,6) Green, (213,117) Blue, (6,117) the player.
```c
3134(x, y, x + 100, y + 80, [6346] /* 26 */);
lh = [9B5C] + [9B5D] + 6;                                    /* 17 with the normal font */
[9B69] = DS:87DE[i];                                         /* 4 red, 10 light green, 1 blue, 15 white */
8BE4(DS:5222[home province], x + 50, y + 6);                 /* "Italia" for the player (C2FB = 0) */
if (i != 3) 8BE4(DS:8804[strategy], x + 50, y + lh);         /* "Aggressive" / "Steady" / "Cautious" */
8BE4(DS:87FE[type], x + 50, y + 80 - lh);                    /* "Light Chariot" ... */
8E62(race sheet + 14 * DS:87F6[type], x + 33, y + 36);       /* the chariot body seen from above: frame 28/24/20 */
```
The widest province name, "Mesopotamia", is 57 px: every name fits the 100-px card (V, width table). Render with the
snapshot's field: `png\prerace_cards_model.png` (G for the exact pixels, nothing to compare with).

### 3.2 Layering of the pre-race screen

- The opponents' cards are drawn once (step 4). The purchase menu (6,117)-(156,197) sits where the player's card will
  be. After a purchase, (6,89)-(157,199) is cleared and the player's card is shown under an empty "Click to continue"
  box (DS:3E02 = ""), then step 7 redraws the circus (opaque: no transparent nibble in circus.cbm) and clears rows
  61..199: all four cards vanish, and the prepare screen shows only the circus strip, the menu at (6,79) and the Rome
  panel. The player's card is therefore seen only under that empty box (G: this is what the empty box is for). D
- The prepare loop redraws `3512` (frame 30E6 at (164,79): fill (168,82)-(310,192) in [6346] + the four MMENU corner
  sprites; "Rank: <rank>\nTalents: <treasury>" at (171,156)) and `6F0C(0)` (Rome's panel: "The people are ..." centred
  at (239,86), status lines at (171,106), the crowd sprite at (280,131), "<difficulty> level" at (239,180)) on every
  pass; port: province.c panel_frame3512 / province_panel6f0c. During the skulduggery menu [634A] temporarily holds
  treasury minus bet (C730 c743), so "Talents:" shows the money left after the bet. D
- Bet text (C88E): 44E5(AABC) (treasury minus bet at (211,167); it leaves [9AFA] = 26 and [9B69] = 0), fill
  (10,150)-(145,180) in 26, then `8BE4("Your Bet: <bet> Talents\nOdds: <odds> to 1", 78, 150)` in black (lines at y
  150 and 161), inside the menu frame below the four items. D

### 3.3 The menus (data in RACE_INDEX section 10)

| menu | box | drawn with | title |
|---|---|---|---|
| DS:88E6 | (6,117)-(156,197) | 5614(rec, 2): 3134 box | "Race what type of Chariot?" |
| DS:88F8 | (6,79)-(156,197) | 5614(rec, 1): frame 30E6 | "How Shall We Prepare?" |
| DS:891C | (6,79)-(156,197) | 5614(rec, 1) once, then 54C8 per pass | "What Skulduggery?" |
| DS:890A | (6,79)-(156,197) | 5614(rec, 1) once | "What Will You Bet?" |

After every choice 585B(rec, item, 0) re-marks the item (keyboard highlight). Message and yes/no boxes are the standard
59D4 / 5C20 at the screen centre (TEXTBOX_NOTES). D

## 4. The gallery parades

Background for all of them (D; V by the capture fit below):
```c
[9AFA] = 0; 7E74(0, 0, 319, 199);                 /* black */
8E62(&gallery1 /* DS:C356 */, 0, 0);              /* 320x130, LUT 3, nibble 0 transparent (LUT3[0] = 0: no visible effect) */
8790(&slot3 /* C0B6, raceest.cbm */, DS:840E /* (0,130) */, C0D0);   /* 320x70 strip, LUT 3, opaque */
691B(0, 0, 319, 199); 6887(); 35DB();             /* screen -> work, present, wait for the buttons up */
```
Per frame (each in the work buffer, then `while (01AE() < 7);` and 6887):
```c
8790(&slot3, (0,130), C0D0);                      /* erase the lower strip */
C299();                                           /* brazier: race2 frame W(8418) at (292,35); W(8418) = (W(8418)+1) % 5 */
C23D(type, x, frame);                             /* one chariot */
```
`C23D(type, x, f)`: race2 frame 5+type (the chariot with its driver, side view) at (x+50, 142), race2 frame 8+f (the
four white horses) at (x, 130), race2 frame 14+(f & 1) (the near wheel) at (x+67, 158), in that order. The rows 0..129
are never redrawn: the brazier frames overwrite their 11x29 box completely except 4 transparent pixels of frame 0
(they keep frame 4's pixels). D

**V (exact fit):** the capture `scr_chariot_race.bin` equals this model with brazier frame 1 (snapshot W(8418) = 2) and
`C23D(0, 54, 4)` (BE10 phase 2, x = 62 - 8) in 63,999 of 64,000 pixels; the odd pixel is (319,199): capture 0x36
(LUT 3 nibble 12), our LZSS decode of RACEEST ends in nibble 1 (G: the decoder's last byte; MAP.CBM also matched
63,999/64,000 in CBM_FORMAT.md). The snapshot's work buffer equals the screen except 1,170 pixels (the next frame being
composed). `png\parade_model.png`, `png\parade_diff.png`.

### 4.1 BE10: the parade before the race (song 2 = cn3a)

- Phase 1: x = 331 - 6k for frame k = 1, 2, ...; f = 0, 1, 2, 3, 0, ... (galloping horses 8..11); type **0** for
  everybody (the plain chariot, race2 frame 5, whatever chariot was bought); until x <= 56 after a frame: 46 frames
  (4.4 s). D
- Phase 2 (only if phase 1 was not interrupted): the chariot stands at x = s - 8, s starting at 62 (x 54); f = 4 + t
  (horses 12/13, standing and prancing), t toggled on `rng_roll(25) == 0`, which also moves s one step toward 62
  (`s <= 62 ? s++ : s--`, so x 54 <-> 55). **Runs until a key (03AA != 0) or a mouse button ([B2] != 0)**: one
  rng_roll(25) per frame for as long as the player watches. D
- A key or button in phase 1 skips phase 2. End: 6837, music_stop, music_unload(2), key_flush, 35DB if a button is
  held. D

### 4.2 BF6A(mode): the winner's parade (mode 0, after "You Win The Race!") and the triumph (mode 1)

```c
n = mode ? 10 : 4;  w = mode ? 9 : 3;                         /* the winner is the last chariot: the player */
music_play(mode ? 0 : 2);                                      /* cn1 / cn3a (5D29 reloads an unloaded song: 5D73 first) */
type[i] = mode ? rng_roll(3) : B(C2C6 + 18*i);  (10 dice in mode 1, none in mode 0)
x[i] = 331 + 90*i; on[i] = 1; f[i] = 0;  x[w] += 45;  type[w] = 0;   /* the player always in the plain chariot */
background (above);
phase 1: per frame: for each i with on[i]: x[i] -= 6; C23D(type[i], x[i], f[i]);
             if (mode && i == w) 8E62(race2 + 14*20, x[i] + 50, 142);      /* the red figure, frame 20 */
             if (x[i] <= -90) on[i] = 0;  f[i] = (f[i] + 1) & 3;
         until x[w] <= 56 or a key/button (then phase 2 is skipped)
phase 2: f[w] = 0; stage = 0; count = 0;
         per frame: C23D(stage < 4 ? 11 + stage : 0, x[w], f[w] + 4);     /* chariot frames 16..19 = arms raised, then 5 */
             if (mode) 8E62(race2 + 14*(stage < 4 ? 21 + stage : 20), x[w] + 50, 142);
             flip; count++; if (count > DS:8412[stage]) stage++;          /* thresholds 0, 1, 32, 33, 34 */
             if (stage <= 4 && rng_roll(15) == 0) { f[w] ^= 1; x[w] += (x[w] <= 56) ? 1 : -1; }
         until a key/button or count > 75
mode 0: music_stop, music_unload(2); key_flush; 35DB if held; 6837.
```
Timing (D): mode 0 phase 1 = 99 frames (9.5 s), mode 1 = 189 frames (18.2 s); phase 2 = 76 frames (7.3 s): chariot
frame 16 (1 frame), 17 (1), 18 (31 frames, arms up), 19 (1), then the plain frame 5 from the 35th frame on (stage 4
and 5); in mode 1 the red figure follows with frames 21, 22, 23, 24, then 20. Dice (D): rng_roll(15)
on the first 34 phase-2 frames (stage <= 4 after the increment), none afterwards; a key ends the loop and the dice.
The ten triumph chariots (mode 1) get random types 0..2 (plain, gold, red side views: race2 frames 5, 6, 7), the
winner's chariot always type 0. Renders: `png\triumph_phase1_model.png`, `png\triumph_phase2_model.png` (G, models).

## 5. The race screen

### 5.1 Layout (D)

| area | rows | content |
|---|---|---|
| circus strip | 0..60 | circus.cbm (320x61, LUT 4; its transparent nibble 13 does not occur: V by count), with the minimap |
| minimap | rect (104,22)-(275,39) | the 368-point path drawn as dots on the circus picture (section 5.4) |
| track view | 61..170 | fill 0x11 (VGA; RGB 160,112,96) + speckles + the chariots, clipped to (0,61,319,170) |
| dashboard | 171..199 | 3134 frame (1,171)-(318,198) filled 0 (column 0, column 319 and row 199 keep 0x11) |

Dashboard (C5E7..C6B9), drawn once on the screen, then copied to the work buffer by 691B:

| item | position | colours |
|---|---|---|
| speed box | 3233 (11,180)-(127,189) | fill 26 |
| horses box | 3233 (196,176)-(312,184) | fill 26 |
| chariot box | 3233 (196,185)-(312,193) | fill 26 |
| "S" | 899C at (7,181), small font | 15 |
| "H" | (191,177), small font | 15 |
| "C" | (191,186), small font | 15 |
| lap counter | laps frame max(0, C350) at (134,185) (frame 0 covers (130,181)-(188,191)) | LUT 4 |

The letters use the small font (`rep movsw` 5 words DS:A3B4 -> 9B56 at C65F, back to A05C at C6A1). Model render:
`png\race_screen_model.png` (G for pixels, D for the coordinates).

### 5.2 Gauges (D)

All drawn on the screen (the target after B190 is the screen), each skipped while the player's previous state C301 == 5
(crashed), so they freeze then.

| routine | bar | fill | rest |
|---|---|---|---|
| D400 speed | (14,183)-(14+s,186), s = 110*speed/130 (C2FE, unsigned) | colour 2 | (14+s,183)-(124,186) colour 7 |
| D453 needle (called by D400) | line (n,183)-(n,186), n = 13 + 110*4*(min(abs(C340) >> 4, 15) + 15) / 130 | colour 4 | - |
| D49A horses | (199,179)-(199+h,181), h = 110*horses/60 (C304, signed) | colour 4 | (199+h,179)-(309,181) colour 7 |
| D4EC chariot | (199,188)-(199+c,190), c = 110*strength/20 (C303, signed) | colour 1 | (199+c,188)-(309,190) colour 7 |

The needle: C340 is the player's lane x (C334 + 4*3); `(min(|x| >> 4, 15) + 15) * 4` is the same speed limit that D17A
uses for the bend crash test (speed above it in a bend -> `rng_roll(15) < excess` wrecks the chariot). So the red mark
is the safe cornering speed of the current lane, 60..120 (x 63..114): outer lanes allow more. D (formula), G (the
designers' intent). Bar maxima reachable: speed <= 109 (top 100 + physician 0..9) -> 92 px; horses <= 59 -> 108 px;
chariot <= 20 -> 110 px. D

Redraw (D3C2(all), every pass after the 7-tick wait): D400 always; D49A if C330; D4EC if C36C; D392 if C332; then,
with all = -1, C36C = C330 = C32E = C332 = 0. C32E (set when a speed changes) is never read: the speed gauge is redrawn
every pass anyway. D

### 5.3 The lap counter: three golden dolphins (D; V by render)

LAPS frame 0 is a bar with three golden dolphins lying head-first; frames 1, 2, 3 (hotspots -11, -20, -28) are the
first, second and third dolphin tipped nose-down, drawn over frame 0 at the same origin, so after three laps all three
point down (`png\laps_sequence_x6.png`). C350 (the leader's lap, -1 at the start) is incremented when any racer starts
a lap beyond it; D392 then draws frame C350 once (C332 dirty), so the tipped dolphins accumulate on the screen.
The Circus Maximus had exactly such a lap counter (Agrippa's dolphins, with the eggs): a period detail (G for intent).

### 5.4 The minimap (D; path V by an exact fit)

The path (CBE6) is 368 points rasterised from the polyline DS:896A..8A89 (72 points, 18 pen-up markers): an oval around
the spina, x 104..275, y 25..39; index 0 = (120,25) is the finish line, the start index is 220 = (260,25) (C30A.. = 220).
The simulation writes exactly 368 points (V). Every pen-up marker writes two (-1,-1) points: 18 gaps of 2 pixels along
the top straight (y 25, x 123..266), where the line passes the spina's monuments in circus.cbm; a dot at (-1,-1) is
clipped, so the racers' dots vanish there (D; G that the gaps were meant to hide them behind the monuments, by eye they
line up: `png\circus_spina_x6.png`). 30 points are duplicates (each polyline segment repeats its start point): a dot
pauses one step at each vertex (D, harmless).

Per pass (B3B0..B421, VGA): 6973(104,22,275,39) restores the minimap rect from the work buffer (the clean circus +
finish line), then for each racer i = 0..3 still running and with laps != 3, putpixel(path[C30C+4i]) in DS:87DE[i]
(4, 10, 1, 15) on the screen. Finished and retired racers lose their dot. D

**Finish line:** when C350 becomes 2 (the leader starts the last lap), B2DA..B30B draws `979C(120, 22, 120, 28)` in
colour 15 (clip 83DC) into the work buffer; it stays (the work buffer's minimap is never cleared). D

### 5.5 The track view (D)

Each pass B190 (target = work buffer):
1. `[9AFA] = 0x11; 7E74(0, 61, 319, 170)`.
2. C0DA = 83D4; B11C(-1): speckles scroll and draw (5.6).
3. For racer 0, 1, 2, 3 (player last, so on top): move, laps, BAC5 (screen y), B6CB (crash state), B460 (state
   machine, then BD0A draws the racer's sprite tree at (C334+4i, C336+4i)).
4. C0DA = 83DC; 680F; 6973(0,61,319,170) and 6973(104,22,275,39) to the screen; dots (5.4).

Screen positions: x = lane C334+4i, 0..280 (steps of 2: B84E +2 capped 280, B886 -2 floored 0; start lanes
70*((r + i) & 3) with r = rng_roll(4) at C474); y = C336+4i: the player is always at 100; another racer aims at
100 + 8*(d), d = player's path index - racer's path index (wrapped to -184..184), moves at most 4 px per pass toward it
when |d| <= 24, jumps otherwise, and is drawn only while -22 <= y <= 170 (BAC5). The horses are at the top of a tree,
so chariots run up the screen and the track scrolls down. D

### 5.6 Speckles (B0E0, B11C, DE64) (D)

10 records DS:AA78 + 6k: x, y, 16-bit pattern. Init (B0E0): x = 5 + 4*rng(77), y = 45 + rng(109), pattern =
rng(65535) (3 dice each). Per pass B11C(-1): `acc = [83EA] + speed (C2FE); step = acc >> 4; [83EA] = acc & 15` (4.4
fixed point, speed/16 px per pass), then for each speckle DE64(x, y, pattern), y += step, and if y > 170: x =
5 + 4*rng(77), y = 45, pattern = rng(65535) (2 dice). B11C(0) (crash scene) draws without moving.
DE64(x, y, pattern), VGA path DEC4: for every set bit, MSB first, from row y downward, the word 0x5652 is written at
(x, row): pixel x colour 0x52 (RGB 224,160,128), pixel x+1 colour 0x56 (112,48,32), a 2-pixel light/dark dash;
rows outside the clip C0DC..C0E0 are shifted out / masked (DS:8BB8 masks), a pattern starting 16+ rows above 61 or at
row >= 170 draws nothing. So the track surface is 10 columns of dashes scrolling down with the player's speed.

### 5.7 The sprite trees (BC15, BCA0, BBAD, BA28, BD0A) (D; numbering V on the snapshot)

Each racer has a tree of 12 nodes of 16 bytes (racer 0 at DS:808C..814B, 1 at 814C..820B, 2 at 820C..82CB, 3 at
82CC..838B; roots DS:83CC = 813C, 81FC, 82BC, 837C, the last node of each block). Node: +0 next sibling, +2 first child, +4 frame descriptor
(sheet + 14*first frame), +6 offset table (s8 dx, dy per frame), +8/+9 s8 dx/dy, +0A active (word), +0C frame count,
+0D current frame, +0E tick, +0F ticks per frame.
BD0A(node, x, y): for each active sibling: draw frame +0D at (x + dx + otab[f].dx, y + dy + otab[f].dy), then
`if (++tick > per) { tick = 0; if (++frame >= count) frame = 0; }`, then the children at that point. Inactive nodes
hide their subtree and do not animate.
BC15/BCA0 number the nodes in pre-order with the frame lists DS:838C (opponents) / 8398 (player) and the active flags
DS:83A4; BBAD then sets the chariot node and the crash node by type. **V:** the 40 numbered descriptors and the 8
type-dependent ones of all four trees in the snapshot equal `race sheet + 14 * frame` from these tables.

| node (pre-order) | frames opp / player | per | offsets | active at start | picture (V by render) |
|---|---|---|---|---|---|
| 0 root | 6..12 / 13..19 | 1 | (0,0) | yes | the four horses from above (brown / white) |
| 1 chariot | 83C4[type] = 28 / 24 / 20, 4 frames | 1 | DS:8030 (2..6, 29) | yes | chariot body: light (plain) / medium (silver) / heavy (red) |
| 2 driver (child of 1) | 0..2 / 3..5 | 2 | (12,-2) | yes | driver holding the reins |
| 3 driver | 59..61 / 68..70 | 2 | (12,-2) | no | driver whipping (with child 4) |
| 4 whip (child of 3) | 77..79 | 2 | DS:807A | yes | the whip |
| 5 driver | 53..55 / 62..64 | 2 | (12,-2) | no | driver lashing (child 6) |
| 6 whip | 71..73 | 2 | DS:8080 | yes | |
| 7 driver | 56..58 / 65..67 | 2 | (12,-2) | no | driver lashing the other side (child 8) |
| 8 whip | 74..76 | 2 | DS:8086 | yes | |
| 9 crash chariot | 83C8[type] = 46 / 39 / 32, 7 frames | 1 | 83E4[type] = DS:8054 / 8046 / 8038 | no | the chariot breaking up, wheel and driver flying |
| 10 (child of 9) | 84, 85 / 90, 91 | 2 | (0,0) | yes | the driver standing |
| 11 | 80..85 / 86..91 | 2 | DS:806A (9, 29) | no | the driver lying / dragged (G) |

BA28(i, k) shows driver pose k (0 reins, 1 whipping the horses, 2 and 3 lashing sideways: state 1 / 2 / 3 of the state
machine) by deactivating the four pose nodes and activating the k-th; it does nothing when the previous state is 4..7
or 11 (its "deactivate all" loop at BA71 is unreachable). Crash (state 5, B6CB / B686): node 9 off, node 11 on; the
pose node stays on, so the driver stays drawn in the chariot while the figure of node 11 lies beside it (D; G whether
the artists meant node 11 as the same driver). Wreck (state 11, B5F1): node 1 off (the chariot and its driver vanish),
node 9 on. Poses side by side: `png\player_tree_poses.png`.

The state machine waits for the animations: B51D..B564 skip the state handling while the chariot node or its next
sibling is active and not at frame 0. So BD0A's frame counters are game state (section 9). D

### 5.8 The crash scene D738 (the player's crash) (D; assembly V by render)

Called from B6CB when the player's crash counter reaches 24 passes and no opponent is visible (AAB4 == 0); all racers'
running flags are cleared first (so the race ends after it, "You Failed To Finish") and node 11 of the player is hidden.
```c
music_stop();                                        /* the race music does not come back */
v = rng_roll(2);                                     /* B: v == 0, A: v != 0 */
A: offs = DS:8B3A, list = DS:8B04 (28 frames: 0..20, 6, 4, 5, 2, 3, 0, 1), base3 = 0,  stays = 1
B: offs = DS:8B72, list = DS:8B20 (26 frames: 0..16, 21, 22, 23, 24, 27, 28, 25, 26, 25), base3 = 25, stays = 0
x0 = C340 (lane), y0 = C342 (screen y); bx = x0 + 9; by = y0 + 29; step = 8*speed/75;
clips C0DA, C1E4, 9AC0 = (0,61,319,170); key_flush();
n1 = (170 - by) / 2 + 15;  sx = bx - 2*n1 - 19;  sy = by + 2*(n1 + 6);
phase 1, n1 frames: 67E4; DA2E (view cleared, speckles drawn, not moved);
         if (ty >= -22) BD0A(player tree, x0, ty -= step);     /* the empty chariot drives off upward; ty starts at y0 */
         8E62(med + 14*29, bx + 14, by + 6);                   /* the driver standing */
         8E62(med + 14*(i & 1), sx, sy); sx += 2; sy -= 2;     /* two bearers bring a stretcher diagonally */
phase 2, list frames i: same clear; if (i <= 7) driver at (bx+14, by+6); if (A && i > 19) driver at (bx-9, by+7);
         8E62(med + 14*list[i], bx + offs[i].dx, by + offs[i].dy);
sx += 27; sy -= 18; if (B) { sx += 9; sy -= 2; }
phase 3, until sx >= 319 or sy <= 21: clear; if (A) driver at (bx-9, by+7);
         8E62(med + 14*((i & 1) + base3), sx, sy); sx += 2; sy -= 2;        /* carried off to the upper right */
each frame: Esc (key 0x1B) ends the phase (and the next ones); 7-tick wait; 680F; 6973(0,61,319,170)
end: 680F; DA2E on the screen; all three clips = (0,0,319,199); key_flush.
```
Variant A: the bearers lift the driver, he gets up again, they carry the **empty** stretcher away and the driver stays
standing; variant B: the driver is carried off on the stretcher. Each about 110 frames (10.7..11 s). The stretcher
bears a **red cross** (MED frames 0..28): an anachronism (G: deliberate joke). Strips: `png\crash_A.png`,
`png\crash_B.png` (models; V by eye that the scripted offsets bring stretcher and driver together: the phase-1 end
point (bx - 19, by + 12) meets phase-2 frame 0 at (bx - 19, by + 11)).

## 6. Timing per pass (D)

Race pass (D08A d0c7..d141), VGA:
```
00F8()                             slot 0 = now
DABA()                             opponents' driving (dice)
CECC()                             input (section 7)
D17A()                             bend crash test, bumps (dice)
B190()                             race tick C322++, view (work), rects to the screen, dots (dice: speckles, B460, crash)
horse recovery: racer i: if C324[i] + 150 <= C322: horses = add_cap(horses, 1, DS:87E6[type]); C324[i] = C322
while (01AE() < 7);                7 ticks = 96.2 ms
D3C2(-1)                           gauges, lap dolphins on the screen
```
So one race tick = 7 timer ticks; horses recover one point per 150 race ticks (14.4 s). Before the first pass:
music_play(3), 00F8, `011F(18)` (0.25 s). The parades and the crash scene use the same 7-tick frame. The message boxes
and menus wait for input (35F9 / 54C8) without a time limit.

For the port: a pass per 7 ticks of a virtual clock (window: the real 72.8 Hz clock; headless: +7 at once) makes a race
a pure function of the seed and the input per pass. G (proposal, as for the naval battle).

## 7. Input (D)

**Race (CECC, once per pass).** `k = 03AA(); 04ED();` (one key read, the rest flushed). Nothing when the player is not
running, or C301 (previous state) == 11, or C302 (state) == 11. With a mouse ([BA] != 0): bits DS:8A9E = 0; 0644;
d = last - now for x and y; if the pointer is outside x 20..300 or y 20..180 it is warped to (160,100) and last =
(160,100) (the deltas are taken before the warp); the larger |d| wins if > 2: moved right -> bit 1, left -> bit 2,
down -> bit 8, up -> bit 4. Buttons: both -> s = 1, right only -> 3, left only -> 2. last = now. Without a mouse and
without a key this pass, 8A9E = 0 (so without a mouse the bits persist while keys come in). Joystick/keypad byte
[A2] (G: the int 9 arrow mapping): if A2 & 8: bit 4, and if C301 == 0: s = 3 if A2 & 1, else 2 if A2 & 2, else 1;
else A2 & 4 -> bit 8, and A2 & 2 -> bit 2, else A2 & 1 -> bit 1 (RACE_LOGIC_NOTES reads A2 as 8 up, 4 down,
2 left, 1 right). Then 04ED again and the state byte C302 (= racer 3 +0x0E):
bit 2 -> 9 (lane x - 2), else bit 1 -> 10 (lane x + 2), else bit 8 -> 8 (slow down); finally s != 0 -> C302 = s
(1 whip the horses, 2 / 3 lash a neighbour). Bit 4 (mouse up) maps to nothing. The ASCII value of the key is never
looked at.

**Parades (BE10, BF6A):** any key with a code (03AA) or any held button ([B2] != 0, polled by 0644) ends the current
phase and skips the next. **Crash scene:** only Esc ends a phase; other keys are read and dropped. **Boxes and menus:**
the EXE's 35F9 and 54C8 (TEXTBOX_NOTES, MENU_NOTES). The mouse is warped to (160,100) when the race screen is ready
(C6D3).

## 8. Fonts, palette and music (D)

- Fonts: normal (DS:A05C: 8 rows, gap 3) for cards, bet text, statistics; small (DS:A3B4: 6 rows) only for "S", "H",
  "C". Text colours: cards DS:87DE[i]; dashboard letters 15; bet text 0; statistics 0 and 15 (click line).
- Palette: no DAC write in section 4 (no call of 01B6, no `out` except DE64's EGA graphics controller). Every picture
  and sprite goes through LUTs into the CMAP palette: circus and LAPS LUT 4, gallery1 / raceest / race2 LUT 3 (frames
  6, 7 LUT 5, frames 20..24 LUT 30), RACE and MED LUT 5 (RACE_DATA_NOTES section 2).
- Music (song numbers DS:3EF0, RACE_INDEX section 9):

| where | call | song |
|---|---|---|
| C2DE c343 | music_load(3), music_load(2) | cn3b (race), cn3a (parades), archive 0 MRACE |
| BE10 | music_play(2) ... music_stop, music_unload(2) | cn3a during the gallery parade |
| D08A d0b8 | music_play(3) | cn3b during the race (5D29 reloads if needed; no restart if already playing) |
| D738 | music_stop | the race music stops at the crash and stays off |
| D08A d162 | music_stop, music_unload(2), music_unload(3) | |
| BF6A(0) | music_play(2) (reloaded by 5D73 inside 5D29) ... music_stop, music_unload(2) | cn3a for the winner's parade |
| C70F (cancel) | music_unload(2), music_unload(3) | |
| DF58 / BF6A(1) | music_load(0), music_play(0), never stopped | cn1 for the triumph, into the credits (G) |

The map/Rome music is not stopped by section 4: it plays on through the pre-race menus until BE10 starts cn3a (G:
whatever was playing).

## 9. Drawing with side effects (the house rule for headless runs) (D)

| routine | called from | side effect |
|---|---|---|
| B0E0 | D08A | 30 dice (speckles), before C2DE |
| B11C | B190 (every pass), DA2E | moves the speckles (DS:AA78..AAB3, 83EA); 2 dice per speckle passing row 170 |
| B190 | D08A | the race tick itself: positions, laps, C350, C320/C32C (race end), running flags; draws the view |
| BD0A | B460, D738 | advances frame/tick of every active node; B460's state machine waits on these frames |
| B460 / B6CB | B190 | racer state machine with dice (rng(10), rng(2)), then BD0A |
| D738 | B6CB | rng_roll(2); its length depends on Esc |
| D392 / D3C2 | D08A, C2DE | clear the dirty flags C330, C332, C36C, C32E |
| C299 | BE10, BF6A | W(8418) brazier frame (persists between calls) |
| BE10 phase 2 | C2DE | rng_roll(25) per frame **until the player presses a key** |
| BF6A | C9B6, DF58 | mode 1: 10 x rng_roll(3); phase 2: rng_roll(15) on up to 34 frames; a key cuts them |
| C2DE c474 | after the purchase, before the prepare menu | rng_roll(4) for the start lanes |
| C2DE c38c (DA5A) | first race of a turn | 4 x 4 dice for the field, before any card is drawn |

So the number of dice consumed before the race starts depends on how many parade frames the player watched; a headless
script must reproduce the frame count of BE10 phase 2 (or the key timing) to get the original's race.

## 10. The victory statistics boxes (DF74) (D)

Text in DS:AC60 (RACE_INDEX section 10). Box: `8C9F(AC60, minw 0, lines0 1, 159, 99, r)`, grown by 7 on each side,
691B save, `3134(r, [6346])`, text in colour 0 centred at ((X0+X1)/2, Y0+7), then the click string W(3E40) ("Click to
continue" with a mouse, "Hit any key to continue" without) in colour 15 centred at ((X0+X1)/2, **Y1 - LH - 5** =
Y1 - 16): one pixel higher than 59D4's Y1 - 15, and the click line's width is not counted (minw 0). 35F9, 6973
restore. The epilogue box DS:8BD8 is drawn the same way (5 lines, 234 px: box (35,59)-(283,139)). With typical values
the statistics box has 13 lines (one empty after "sunk." and "winning N.") and is 290..300 px wide: (2..7, 15)-
(311..316, 183) (D, widths from the font table); seven-digit totals in both "legionnaires ... enemy soldiers" numbers
would push X0 below 0 (G: not reachable in normal play).

## 11. Open points

1. The (319,199) pixel of RACEEST (section 4): our LZSS decoder vs the game's. G.
2. What [A2] bits mean exactly (int 9 arrow mapping, joystick): read the EXE's keyboard ISR. G.
3. The race screen, crash scene and statistics boxes have not been captured in DOSBox; their coordinates are D only.
   A capture of a race pass and of the dashboard would make sections 5 and 10 V.
