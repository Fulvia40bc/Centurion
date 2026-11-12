# Gladiator show: the screens, the input and the timing (OVL0 section 3)

Claude, 2026-11-10. What the gladiator show draws, when, where and into which buffer; its texts, sprites, music, input
and timing. Source: `disasm/OVL_GLAD.asm` (addresses = 0823 offsets), the EXE helpers in `disasm/EXE_RUNTIME.bin`
(disassembled with the scratch `glad/odis.py`), the static DS image (OVL0.OVL 0xB400 = DS:0000) and the runtime image
`tools/dosbox/results/mem_map_gamemenu_640k.bin` (DS at linear 0x16860). The gladiator block DS:7B06-0x801F is
identical in both images (0 bytes differ). Function index and DS map: `docs/GLAD_INDEX.md`; data files:
`docs/GLAD_DATA_NOTES.md`. The duel's rules (damage, AI, scoring) belong to the logic notes and appear here only where
the screen or the input depends on them.

Marks: **D** = decoded from the code or dumped from the data; **V** = verified by a render, an exact fit or a capture;
**G** = guess. Nothing of section 3 has been run in DOSBox yet. The V marks below come from Python renders of the
decoded drawing rules with the real data (scratch `gladscr\comp.py`, PNGs in `gladscr\png\`): every piece lands where
it should (portraits on their plinths, feet on the sand, the kill stab on the body, the thumbs box under the crowd
meter), which checks coordinates and formats by eye, not pixel by pixel against the game.

Calls are Pascal: the first argument pushed is the first parameter; below, arguments are in push order. P = the
player's fighter (struct DS:AA52, the right-hand man), O = the opponent (DS:AA6A, the left-hand one).

## 0. Summary for the implementer

1. Four screens, all 320x200, no scrolling, no palette change: the COLOSEUM title picture, the card grid (selection,
   training menu, VERSUS), the arena (fight, walk-over, thumbs vote) and the closing "Click to continue". D
2. Buffers: pictures are composed in the work buffer (67E4) and copied to the screen with 6973; cards, texts, health
   bars, the crowd meter and the thumbs box are drawn **straight onto the screen** (680F). During the fight only rows
   126..185 are copied each frame (C1A4); everything else stays as it was drawn once. There is no 6887 page flip. D
3. The fight runs in frames of exactly 4 timer ticks (busy-wait after the drawing) = 54.9 ms = 18.2 fps; each fighter
   advances one animation state every 6 frames (both counters in phase). Unlike the naval duel, the number of dice per
   frame does not depend on the CPU speed. D
4. Drawing and game state are interleaved: the crowd routine BB3B (crowd meter + music) is called from the exchange test
   and from the vote; the frame loop rolls rng(2) whenever the fighters would collide; B38B rolls rng(10) between card
   draws. A headless run must call the same routines in the same order (section 9). D
5. Two serious input slips in the card pick B4DA (section 6.2): **Esc never cancels the pick, and a click that misses
   every card hangs the game** (the loop then hit-tests a stale mouse position forever). D (not yet seen in DOSBox)

## 1. The sequence of screens (D)

| step | code | what the player sees | target |
|---|---|---|---|
| 1 | B0E7 | archive 6 (MGLAD) opened; ARENABTM.CBM into picture slot 4 (13A8). Open failure: message "Can't Load Gladiator Graphics" (box 88,81-230,117), return | - |
| 2 | B108..B154 | COLOSEUM.CBM: work buffer filled black, the picture drawn with 8E62 at (0, 0), freed (86A0), whole work buffer copied to the screen. Skipped if the member or slot 4 failed to load | work -> screen |
| 3 | B157..B1A6 | sheets "show" and "arena" loaded; cursor push/hide (1D55(0)); text colour 15; map song 1 unloaded, songs 4..7 loaded, song 4 (cn4a) starts | - |
| 4 | B1A9..B1C0 | the title stays until 432 ticks (5.9 s) or a key (a mouse click does not count, section 6.1) | - |
| 5 | B1C2, B38B | song 7 (cn4d); the card grid with 6 free gladiators facing right; "Select a Gladiator\nfor You to Control"; pick + "How Much Training?" | work -> screen, then screen |
| 6 | B38B | the grid again, the 6 cards facing left (the picked card now shows a beast); "Select a Gladiator\nTo Fight Against"; pick + training | same |
| 7 | B205..B23B | song 5 (cn4b); empty grid, "VERSUS" at (160, 125), the player's card in slot 5 (bottom right, facing right) and the opponent's in slot 3 (bottom left, facing left) | work -> screen, then screen |
| 8 | B23E..B298 | "show" freed; the two fighter sheets loaded; archive closed; song 4 (cn4a); **an empty message box** = a bare "Click to continue" box (116,87)-(202,111) over the VERSUS screen | screen |
| 9 | B29B..B2DC | the arena: work filled black, arena frame 0 at (0, 0), ARENABTM at (0, 126), whole work buffer to the screen | work -> screen |
| 10 | B2DF..B2F0 | C288 places the fighters, C1A4 draws them; song 5 (cn4b) | work rows 126..185 -> screen |
| 11 | B7FE | the fight: health bars, crowd meter box, the frame loop (section 5) | screen + band |
| 12 | BDEA | the outcome: walk-over animation, thumbs box + "Thumbs Up or Down?" + vote, kill frame (section 4.9) | screen + band |
| 13 | B324..B346 | music stopped, songs 4..7 unloaded; **an empty message box** ("Click to continue") over the final arena picture | screen |
| 14 | B349..B37A | both records' status = 0, the arena and fighter sheets freed, cursor pop, return to Rome Games (which redraws) | - |

Cancelled selection (B38B returns 0: the treasury check, "cannot afford a second", or Esc in a training menu): songs
4..7 unloaded, "show"/"arena" freed, archive closed, cursor pop, music stopped, return (steps 7..14 skipped). D

## 2. Buffers and EXE drawing helpers (D; VGA [9B26] = 2)

Descriptors: DS:9AB6 screen (A000), DS:C1DA work buffer (2687:0000 at run time), DS:C0D0 = the current target.
Section 3 never changes the clip words (C0DA..C0E0 stay 0,0,319,199). Helpers used (same as NAVAL_SCREEN_NOTES
section 2 unless noted):

| EXE | use here |
|---|---|
| 67E4 / 680F | target = work buffer / screen |
| 6973(x0,y0,x1,y1) | copy work -> screen: full screen (B154, B2DC, B6E7) or rows 126..185 (C257, `6973(0, 126, 319, 185)`) |
| 7E74(x0,y0,x1,y1) | filled rect in [9AFA], inclusive, corners sorted (C302 and C149 rely on the sorting) |
| 7ECA(x0,y,x1) | hline in [9AFA] (the card grid) |
| 8E62(desc,x,y) | sprite at (x - xhot, y - yhot), transparent nibble skipped, LUT, clipped to the target's rect |
| 8790(desc,point,target) | opaque blit of a picture slot (rep movsb rows, the bytes already expanded through the CBM's LUT by 13A8); point DS:7BC8 = {x 0, y 126} |
| 86A0(desc) | free a CBM descriptor (the index's "blit (86A0)" is wrong: it is the free, as in nego/province) |
| 3233(x0,y0,x1,y1,c) | bevelled box: fill c (skipped if -1); colour 8 verticals x0 (y0..y1), x0+1 (y0+1..y1-1), x1, x1-1; colour 0 hlines y1 (x0..x1), y1-1 (x0+1..x1-1); colour 15 hlines y0, y0+1 (inner). = 3134 without the inner groove (decoded at EXE 3233..32DF) |
| 59D4(s) | message box (TEXTBOX_NOTES 3, V on 3 captures): saves the screen rect to the work buffer (691B), box 3134 fill 26, text black, "Click to continue" white, waits for click/key, restores, leaves [9B69] = 15 |
| 8BE4(s,cx,y) | centred text, lines 11 px apart, x = cx - (8C58(line) >> 1) |
| 5614/54C8/585B | menu draw (mode 1) / run / highlight (MENU_NOTES) |

Which picture lives where:
- **Work buffer:** the title picture; the card grid without cards (B67C); the arena (frame 0 + ARENABTM); then each fight
  frame repaints rows 126..199 (8790 of ARENABTM) and draws both fighters (C1A4). Rows 0..125 of the work buffer keep
  arena frame 0 for the whole fight. Message boxes also write the screen rect under them into the work buffer (691B).
- **Screen only:** the cards, "VERSUS", the training menu, the crowd meter box and bar (rows 10..20), the health bars
  (rows 188..193), the thumbs box. The per-frame copy covers rows 126..185 only, so none of these is erased by the
  fight. The fighters fit that band: the tallest frame (man frame 3, yhot 47, 48 rows) spans rows 128..175 at y 175;
  the lowest (fallen man, frame 10: yhot 13, 24 rows) ends at row 185. D for the arithmetic, V by the renders.

## 3. Timing (D; tick = DS:42/44, 72.8 Hz, 13.73 ms)

| slot | where | rule | time |
|---|---|---|---|
| 0 | B1A9 title | `00F8` reset, loop while `01AE() < 0x1B0` and no key and [B2] == 0 | 432 ticks = 5.93 s at most |
| 0 | B9E3..BAD9 fight frame | reset just before the move/draw part, busy-wait `01AE() >= 4` after it | 4 ticks = 54.9 ms per frame |
| 1 | B883, BC42, BAF2 boredom | `timer_start(1)` before the loop, after every exchange (BBC8) and after every boredom tick; `0183(1) > 0xD8` -> BB3B(-1) | > 216 ticks: the 54th frame after an exchange, the 55th after a boredom tick |
| 0 | BF17..BF97 walk-over | reset at the top, draw, busy-wait `01AE() >= 4` | 4 ticks per step, states advance every frame |
| - | C004 vote | no wait: poll mouse and [A2] as fast as the CPU goes (no dice in it); the timer ISR keeps counting. Port: one tick (nv_tick) and one dropped key read per pass, so script "wait" / "key" lines run there | - |

Frame counters in the fight (locals of B7FE, all decremented once per pass):
- animation: P `[bp-0xA]`, O `[bp-0xC]`, both start at 6 and are reset to 6 when they reach 0 -> **one step every 6
  frames (24 ticks, 0.33 s), P and O always on the same frame** (P's step is processed first);
- input: P `[bp-8]` starts 0 -> P's command is read on the first pass and then every `(4 - P.training) * 4` frames
  (16/12/8/4 frames for Trainee/Slave/Veteran/Master); AI `[bp-6]` every `(4 - O.training) * 2` frames (8/6/4/2), gated
  further by the pace accumulator DS:800D (BC4C);
- a command changes the state at once but does not reset the animation counter, so a commanded state is shown for
  1..6 frames, depending on the phase.

On a machine fast enough to finish a pass in under a tick, a pass is exactly 4 ticks: the slot-0 reset and the end of
the wait fall in the same tick. The port: one fight pass = 4 virtual ticks, the busy-waits advance the virtual clock
(as proposed for the naval battle). G for the choice, D for the facts.

## 4. The screens, piece by piece

### 4.1 Title (B108..B1C0) D, V by render (`c1_title.png`)
`67E4; [9AFA] = 0; 7E74(0,0,319,199); 8E62(&coloseum, 0, 0); 680F; 86A0(&coloseum); 6973(0,0,319,199)`. COLOSEUM.CBM is
320x200, hotspot (0,0), transparent nibble 0 (LUT 20 maps 0 to colour 0, so the black fill shows through exactly where
the picture is black anyway). The picture: the Colosseum seen from above, full of spectators, crowds on the square.

### 4.2 Card grid B67C (no frame args) D, V by render (`c2_grid_empty.png`)
Into the work buffer: `[9AFA] = 1; 7E74(0,0,319,199)` (dark blue), then 16 full-width hlines from DS:7BDA {colour word,
y byte, pad}: colour 8 at y 0, 95, 100, 195; 0 at y 1, 94, 101, 194; 7 at y 96, 99, 196, 199; 15 at y 97, 98, 197, 198
(a bevelled shelf under each row of cards), then 14 sprites of sheet "arena" from DS:7C1A {x, y, frame} (frame k =
`[C2B4] + 14k`, hotspot 0,0):

| frame | what | positions (x, y) |
|---|---|---|
| 1 (25x92) | a column | (-11, 2) (94, 2) (201, 2) (307, 2) (-11, 102) (94, 102) (201, 102) (307, 102); x -11 and 307 are clipped at the screen edges |
| 2 (76x8) | a plinth | (16, 86) (122, 86) (228, 86) (16, 186) (122, 186) (228, 186) |

Then `680F; 6973(0,0,319,199)`. Target = screen afterwards.

### 4.3 A card B6EC(pos, record, facing) D, V by render (`c3_pick1.png`, `c5_pick2.png`, `c7*_versus*.png`)
`cx = 56 + 106*(pos % 3)`, `yb = 87 + 100*(pos / 3)` (pos 0..5, row-major). On the current target (= screen):
- name: strcpy(stack, name ptr DS:7E3E+10r) (names carry a trailing space); `if style != 4 && generation != 1` strcat
  roman_numeral(generation) (6738 -> DS:C072); `8BE4(name, cx, yb - 80)` = y 7 / 107. The trailing space counts in
  8C58, so a first-generation name sits 2 px left of the column centre (D, cosmetic).
- `if wins != 0`: "Bouts Won" (DS:7DAD) at (cx, yb - 70), roman_numeral(wins) at (cx, yb - 60). Losses are never shown.
- portrait, sheet "show": facing != 0: frame = body (0 man, 1 wimp: the right-hand fighters, facing left), x = cx - 10;
  facing 0: frame = body + 2 (2 man, 3 wimp, 4..6 the cats, facing right), x = cx + 10, beasts x + 15 more;
  `8E62(frame, x, yb)`: feet on the plinth (V by render).
- text colour: [9B69] = 15 (set at B17F; message boxes and the menu title leave it at 15). D

Card click rectangles (B4DA): `x0 = 7 + 106*(i % 3)`, `y0 = 2 + 100*(i / 3)`, `x1 = x0 + 91`, `y1 = y0 + 91`, inclusive.
Gaps (not on any card): x 0..6, 99..112, 205..218, 311..319 and y 0..1, 94..101, 194..199.

"Facing right" in the code (B6EC facing = -1) shows the R sprites, which look to the LEFT: the player's man stands on
the right of the arena and faces his opponent. The first grid therefore shows men looking left, the second (opponent)
grid men looking right (V by render).

### 4.4 Selection messages (B38B) D; boxes by the verified 59D4 rule
| DS | text | box |
|---|---|---|
| 7CC2 | "You Cannot Afford\nTwo Gladiators." (treasury < 20) | 111,76-207,122 |
| 7CE4 | "Select a Gladiator\nfor You to Control" | 111,76-207,122 |
| 7D0A | "You Cannot Afford a Second Gladiator.\nYour Talents for the First are Refunded." | 60,76-258,122 |
| 7D59 | "Select a Gladiator\nTo Fight Against" | 114,76-204,122 |
| 7D7D | "You Cannot Afford Such\na Well-Trained Gladiator" | 99,76-219,122 |
| 7BC6 / 7BC7 | "" (two empty strings: the pauses at B294 and B342) | 116,87-202,111 |
| 800E | "Thumbs Up or Down?" | 105,81-213,117 |
| 7BA1 | "Can't Load Gladiator Graphics" | 88,81-230,117 |

Order in B38B: the first grid is drawn (B67C), then for each of the 6 slots: rng(10), probe for a free record, mark it
3, draw its card (so dice and card draws alternate). Then the message box, then B4DA. After the first pick: cursor
hidden, B67C, the 6 cards facing left (lineup slot of the pick now = beast 10 + rng(3)), cursor shown, then either the
refund message or "Select a Gladiator\nTo Fight Against" and the second B4DA.

### 4.5 The training menu (B59F..B66D) D, V by render (`c4_menu_man.png`, `c6_menu_beast.png`)
B4DA builds 4 item texts in its own stack frame (25 bytes each: label + " (n0 Talents)", n = 1..4 patched into DS:7B6A)
and stores their stack addresses in the item records DS:7C80/7C8C/7C98/7CA4, then `5614(7CB0, 1)` on the screen:
- mode 1 frame 30E6 at (91, 79): fill [6346] (95,82)-(237,192), MMENU sprites 0..3 at (91,79) (the parchment frame);
- items colour 0 at (91 + 6 + 11, 79 + 23 + 10i) = (108, 102/112/122/132);
- ornament (colour 8): lines x 99, 101, 103 from y 102 to 141, caps x 98..104 at y 102 and y 141;
- title "How Much Training?" colour 15 centred on x 166 at y 85 (W 90 -> x 121).
Labels: men DS:7C6E "Trainee", "Slave", "Veteran", "Master"; a beast (style 4) DS:7C76 "Fat", "Well-Fed", "Hungry",
"Ravenous". The menu is not erased by B4DA: the next full redraw (B67C) covers it.

### 4.6 VERSUS (B205..B23B) D, V by render (`c7_versus.png`, `c7b_versus_beast.png`)
B67C (empty grid), `8BE4("VERSUS", 160, 125)` (W 31 -> x 145, in the middle card of the bottom row), B6EC(5, player, -1),
B6EC(3, opponent, 0). The top row stays empty.

### 4.7 The arena (B29B..B2F0, C288, C1A4) D, V by render (`c9_arena_start.png`, `c9b_arena_wimp_cat.png`)
- background: `67E4; [9AFA] = 0; 7E74(0,0,319,199); 8E62(arena frame 0, 0, 0)` (320x126: stands, crowd, the
  imperial box, the gate) `; 8790(ARENABTM, (0,126))` (320x74: sand, the base of the wall, two dark bands at the bottom)
  `; 680F; 6973(0,0,319,199)`.
- start: P at (192, 175), O at (127, 175) (C288), both in state 0.
- **C1A4 (every fight frame and the walk-over):** `67E4; 8790(ARENABTM, (0,126))`; frames f = movetab[state].frame;
  if `O.state != O.dying`: O sprite (sheet [C2B8]) then P (sheet [C2B6]) on top; else P first and **O on top**. So the
  opponent is under the player except when he lies in his dying state (then the body covers the winner's feet). Then
  `680F; 6973(0, 126, 319, 185)`. Pure drawing: no DS write, no dice. D (the index's "the fallen one first
  (underneath)" is wrong: the fallen opponent is drawn last).
- Clipping: P may step back up to x 305 and O down to x 15: parts of the sprites leave the screen and are clipped by
  8E62 at x 0/319 (D from the move rules: P steps back only if x < 290, O guards instead below x 30).

### 4.8 Health bars and crowd meter (B872..B8B3, C25F, C302, C149) D, V by render
- C25F(side): `[9AFA] = 8` (dead store), then `C302(health, side)` with side 0 = O.health, 1 = P.health.
- **C302(health, side):** `h = max(health, 0); w = (h << 7) / 50` (so 10 health = 25 px, the maximum 37 = 94 px); grey =
  8 (EGA) or **0x36** on VGA. Side 0 (opponent, left): red (colour 4) fill (0,188)-(w,193), then grey (w,188)-(128,193).
  Side 1 (player, right): red (319-w,188)-(319,193), then grey (191,188)-(319-w,193). The grey overwrites column w (left)
  or 319-w (right), so exactly w red pixels show, and none at health 0. Drawn on the screen at fight start (P then O)
  and after each fighter's exchange test (C25F(0) after P attacks, C25F(1) after O attacks), every 6 frames.
- **Crowd box:** `3233(79, 10, 234, 20, [6346] = 26)` once at fight start (screen). Inside: x 81..232, y 12..18.
- **C149(v):** `w = 145 * v / 30` clamped 0..145; white (15) fill (82,13)-(82+w,17), then [6346] fill (82+w,13)-(221,17).
  Redrawn by every BB3B call and once in BDEA. **Slip (D, cosmetic):** the rest fill ends at x 221 although the bar can
  reach 82 + 145 = 227; for v >= 29 the sorted second fill paints 221..82+w grey again, so the white bar stops at x 220
  (139 px): crowd 29 and 30 look the same, and x 221..232 of the box never turns white. Fix (not under --original):
  fill the rest to 227 (or scale by 139). Values: v 26 -> 125 px, 27 -> 130, 28 -> 135, 29/30 -> 139 visible.
- Start value: crowd = P.training + O.training + 2 (2..8) -> 9..38 px.

### 4.9 The end of the fight (BDEA) D, V by render (`c10_thumbs.png`, `c10b_thumbs_msg.png`, `c11_thumbs_down_kill.png`)
Only when a man beat a man with exactly one survivor (both dead, a dead beast, or a winning beast: no walk, no vote):
1. **Walk-over loop (BF17..BFD5), one step per 4 ticks:** the loser's state = his dying state (O: [AA73], P: [AA5B]) at
   every step; the winner, if his state is idle (flags bit 0), is set to state 1 (step in), then moved by
   movetab[state].dx; C1A4; wait 4 ticks; if `O.x > P.x` (the winner has walked onto/past the body) stop; else both
   states advance (next). The winner cycles states 1 -> 12 -> 15 (man: frames 1, 0, 0, 16 px per 3 steps = 12 ticks)
   or 1 -> 11 -> 13 (wimp, 8 px). The walk runs one state per frame, 6 times faster than the fight.
2. Thumbs box: arena frame 3 (156x62, hotspot 0,0) at (79, 20) -> covers x 79..234, y 20..81 (the left half a blue
   panel with a thumb up, the right half a red panel with a thumb down), on the screen, over the crowd box's bottom
   row. Then C149(crowd) again; cursor shown if a mouse is present (1CCE); message "Thumbs Up or Down?" (box
   105,81-213,117, dismissed by a click or key).
3. Vote loop (C004): section 6.5.
4. Thumbs down: the winner's state = 8 (the kill: man frame 5, a downward stab; wimp frame 4), the loser's state = his
   dying state, **one** C1A4 (a single still frame, no animation). Thumbs up: nothing is redrawn.
5. BB3B(+5 or -5) (crowd bar, maybe music) and score_add (logic notes).

### 4.10 After the show
The final empty message box (B342) shows over the last arena picture. Rome Games' caller redraws its own screen. 13A8
cleared the "loaded" flag of every other picture slot (0..3: map.cbm, bb1, bb2, raceest) when it loaded ARENABTM
(EXE 13CE..13EE, unless [9B26] == 4), so the map picture is reloaded from disk by the next map draw (D for the flags;
the reload is the map code's). The port keeps its map image separately and only needs to redraw. G

## 5. The fight loop B7FE, frame order (D)

Before the loop: crowd counters 0, mood 1, health = 10 for both; P.health += rng(10) per P.training, then O (dice:
P first); C25F(1), C25F(0); timer_start(1); crowd = Pt + Ot + 2; 3233 box; C149. Then per pass:

| step | code | what |
|---|---|---|
| 1 | B8B9 | key_read + key_flush (result discarded: no key does anything in the fight) |
| 2 | B8BF | if P.state or O.state == FF: end flag |
| 3 | B8D2 | P input countdown <= 0: reload; if P idle: P.health <= 0 -> state 9 (fall), else the [A2] command (6.4); pending-move flag P |
| 4 | B997 | P input and P animation counters -1 |
| 5 | B99D | AI countdown <= 0: reload; if O idle: BC4C (dice); pending-move flag O |
| 6 | B9D4 | AI and O animation counters -1 |
| 7 | B9DA | end flag set: leave the loop **here, before any move or drawing** (so a state FF is never drawn) |
| 8 | B9E3 | timer0_reset |
| 9 | B9E6..BA5D | moves: new P.x = P.x + dx(P.state) if P's flag, new O.x likewise (flags cleared). If new O.x < new P.x both are taken; else **rng(2)**: 0 -> only P moves (if O.x < new P.x), 1 -> only O moves (if P.x > new O.x). O.x < P.x always holds |
| 10 | BA61 | C1A4 (work buffer, rows 126..185 to the screen) |
| 11 | BA64 | P animation counter == 0: BBC8(P attacks O) [hit/block, BB3B(+1), timer_start(1)], C25F(0), P.state = next, pending flag, counter 6 |
| 12 | BA9B | O animation counter == 0: BBC8(O attacks P), C25F(1), O.state = next, pending flag, counter 6 |
| 13 | BAD3 | busy-wait until 4 ticks since step 8 |
| 14 | BADB | if more than 216 ticks since timer 1: BB3B(-1), timer_start(1) |
| 15 | BAF5 | if a state is FF and both states are 0 or FF: end flag |
| 16 | BB24 | loop; at the end key_flush, music_stop |

So the frame on the screen always shows the states as they were before the exchange test of that frame; the test
uses the state that was shown, and the new state appears (and moves) on the next frame. A fall runs 9 -> 13 -> 14 -> FF
(man) and ends the loop 6 frames after state 14 was entered at the earliest. When the loop ends at step 7 the AI and
the command check of that last pass may still read movetab[FF] (P wimp flags FF and O wimp flags 0x3B have bit 0 set
in the bytes past the tables: the dead wimp is set to state 9 again); BDEA overwrites that state, so it is harmless.
D. (This corrects the index's guess 5: no FF frame is ever moved or drawn.)

## 6. Input

### 6.1 Title (B1AC..B1C0) D
Ends on `01AE() >= 432`, on `03AA` returning a key (03AA loops over keys whose ASCII is 0, so arrows and function keys
are eaten without ending the wait; Ins -> 0x10 counts; Ctrl-S counts as nothing), or on `[B2] != 0`. Nothing polls the
mouse in this loop ([B2] is written only by 0644/06D4/3603, and the INT 8 handler does not poll; no INT 33h user
handler is installed: the EXE uses functions 3..8 only). [B2] is the value of the last poll: 0, since Rome Games' menu
(54C8) waits for the release before it returns. **1990 bug: a mouse click does not end the title; only a key or the
5.9 s timeout do.** D for the code, G that [B2] is always 0 at entry. Fix: call 0644 in the loop (not under --original).

### 6.2 Card pick B4DA D
```c
picked = 0; armed = 0;                       /* [bp-2], [bp-8] */
cursor_show();
for (;;) {
    if (picked) break;
    while (!armed) {
        if (359B() == 0) continue;           /* a KEY: returns 0, [C07F] = key -> ignored */
        if ([C07F] == 0x1B) return_cancel(); /* button: 359B set [C07F] = 0 -> never true */
        armed = -1;
    }
    for (i = 0; i < 6; i++) if (in_rect(mouse_x, mouse_y, card_rect(i))) { pick(i); picked = -1; break; }
    35DB();                                  /* wait for release: polls only while [B2] != 0 */
}
```
- **Esc cannot cancel the pick (D):** the Esc test sits on the button branch, where [C07F] is always 0. Keys do nothing on
  the grid. Without a mouse, Enter/'+' (right) and Space/Ins (left) act as buttons at the keyboard-moved cursor (3603),
  so the keyboard player picks the same way. The only way out of the selection is Esc in a training menu.
- **A click that misses every card hangs the game (D, not yet reproduced):** `armed` is never cleared, so after a miss the
  loop hit-tests again at once; 35DB returns immediately once [B2] is 0 and then polls nothing, so the mouse position is
  never updated and no new click is waited for: an endless loop with a frozen cursor. A release over a card (dragging
  onto a card while holding the button) still picks that card. Clicking on a column, the shelf between the rows or the
  screen border (the gaps in 4.3) hangs. Fix (default, not under --original): `armed = 0` after a miss, and test Esc on
  the key branch (`if (359B() == 0) { if ([C07F] == 0x1B) cancel; continue; }`).
- Disagreement: `docs/GLAD_LOGIC_NOTES.md` section 3 (written in parallel) reads "any key other than Esc acts as a click
  at the cursor; Esc cancels only before the first click". 359B returns 0 for every key (with the key in [C07F]) and
  1/2 for a button (with [C07F] = 0), and B501 is `or al, al / je B514`, so a key loops back to the wait and the Esc
  compare at B505 only runs after a button. The reading above follows the bytes. A DOSBox test settles it: press Esc,
  then a letter, then click a column.
- On a hit: record status 1, the lineup slot becomes beast 10 + rng(3) (one die per pick), release wait, cursor hidden,
  the training menu (4.5) and `54C8(7CB0)`: item c -> if `10*(c+1) > treasury`: `585B(7CB0, c, 0)` (item text back to
  colour 0) and "You Cannot Afford Such\na Well-Trained Gladiator", ask again; else training = c, treasury -= 10*(c+1).
  Esc (0xFF) -> return 0xFF. Mouse clicks on an item or hotkeys '1'..'4' (MENU_NOTES section 4).

### 6.3 Message boxes D
Any click or key (35F9: release, 359B, release). The vote box and the two pauses are ordinary 59D4 boxes.

### 6.4 The fight D
Only the held arrow bits [A2] (INT 9 hook: Up 8, Down 4, Left 2, Right 1; keypad Home 0x0A, End 0x06, PgUp 0x09, PgDn
0x05, 5 -> 0x80), read when P's input countdown fires and P is idle (states 0, 5, 6, 7, 8):

| [A2] | keys | P state |
|---|---|---|
| 1 | Right | 2 = step back (+15 man / +7 wimp), only if P.x < 290 (else unchanged) |
| 2 | Left | 1 = step in (then 12: -16 / wimp 11: -8), only if P.x > 30 |
| 4 | Down | 5 = low guard |
| 6 | Down+Left, keypad End | 4 = low attack |
| 8 | Up | from state 0 only: 7, and if rng(2) != 0 then 6 (two high-guard poses); from a guard: unchanged |
| 10 | Up+Left, keypad Home | 3 = high attack (3 -> 10 -> 11) |
| anything else (none, 3, 5, 7, 9, 0x80, ...) | | 0 = stand |

The keyboard buffer is read and flushed every pass, so typed keys (Esc included) do nothing; the mouse is ignored.
Releasing all keys drops a guard at the next input check (state 0).

### 6.5 The vote (C004..C071) D
After the "Thumbs Up or Down?" box is dismissed, loop: if a mouse is present: 0644, 1BE6 (cursor follows), and if a
button is down: inside DS:7EC0 (79,20)-(156,81) -> up; inside DS:7EC8 (157,20)-(234,81) -> down. Then [A2] == 8 (Up
alone) -> up; [A2] == 4 (Down alone) -> down. Loop until one is chosen; then key_flush. No dice, no timer. The cursor
stays visible until the final message box hides it (1D55 inside 59D4).

## 7. Music (D; 5D29 restarts nothing if the song is already playing, [3EEE] = current song, 5D20 sets it to FF)

| when | call | song |
|---|---|---|
| B188 | 5E1A(1) unload the map music (cn2); it is not stopped first | - |
| B18B..B1A0 | 5D73(4), (5), (6), (7): all four music slots | cn4a..cn4d (MGLAD) |
| B1A6 | 5D29(4) title | cn4a |
| B1C5 | 5D29(7) selection | cn4d |
| B208 | 5D29(5) VERSUS | cn4b |
| B291 | 5D29(4) the pause before the arena | cn4a |
| B2F0 | 5D29(5) arena, fight start (= mood 1) | cn4b |
| BBBF (BB3B) | 5D29(DS:800A[mood]) when the mood changes: mood 0 (bored) 7 cn4d, 1 -> 5 cn4b, 2 (excited) -> 6 cn4c | |
| BB30 | 5D20 at the end of the fight | stop |
| BDEA | the vote's BB3B(+-5) can start a mood song again for a moment | |
| B327..B33F | 5D20, 5E1A(4..7) | stop, unload |
| B1CF..B1FF (cancel) | 5E1A(4..7), then 5D20 | |

BB3B plays a song after 4 consecutive positive calls (exchanges or a +5 vote: mood + 1, max 2) or 4 consecutive
negative calls (boredom ticks or a -5 vote: mood - 1, min 0); the run counters persist from the fight into the vote.
The map music is not reloaded here (the caller's next map draw plays it, as after the naval battle). G for that.

## 8. Fonts and palettes (D)
- One font: the standard 8-px font (DS:9B56.. = 9C9C/9CFC/9D5C, height 8, gap 3, spacing 1 in the runtime image); the
  section never writes 9B56..9B5F. Text colours: 15 (names, "VERSUS", "Bouts Won", the menu title, "Click to
  continue"), 0 (menu items, message text).
- No palette writes, no DAC access, no palette cycling: every colour goes through the CBM/sprite LUTs into the fixed
  250-colour CMAP. VGA-only colour choice: the grey of the health bars, 0x36 when [9B26] == 2, else 8. The box fill
  [6346] is 26 at run time.

## 9. Drawing routines with side effects; dice; DS writes for compare tools (D)

Pure drawing (no DS write except the target/fill words, no dice): B67C, C149, C1A4, C25F, C302.
- **B6EC** writes DS:C072 (roman_numeral buffer) whenever it prints a numeral.
- **BB3B** (called from BBC8, the boredom check and the vote) changes the crowd meter, the run counters and the mood, draws
  the bar and may change the music. It must run headless exactly where the original calls it.
- **B38B** interleaves rng(10) with the card draws (6 rolls, one per slot).
- **B4DA** rolls rng(3) on a hit (before the menu), patches DS:7B6A (" (20 Talents)" ends as " (40 Talents)" after every
  menu) and writes stack addresses into DS:7C80/7C8C/7C98/7CA4 (the item text pointers: `bp - 0x78 + 0x19*i` of B4DA's
  frame; a DS compare must either model SP or mask these 4 words).
- **B7FE** rolls: P health (P.training dice), O health, rng(2) for Up from state 0, rng(2) on every blocked move; BC4C's
  AI dice (paced by DS:800D, which is never reset: it carries over between shows and is part of the DS).
- Message boxes write [9B69] = 15 and the work buffer under the box (691B).
- Sheet handles C2B4..C2BA are heap pointers (mask them in DS compares, as for the naval sheets).
- 13A8 writes the picture-slot flags DS:0254+8n and the descriptor DS:C0BE.

## 10. 1990 slips found on the screen/input side (fix by default, keep under --original)
1. **Card pick hangs after a missed click** (6.2). D. Severity high.
2. **Esc on the card grid does nothing** although the code tests for it (6.2). D.
3. **The title ignores mouse clicks** (6.1). D (G for [B2] = 0 at entry).
4. **Crowd meter tops out at x 220** (4.8). D, cosmetic.
5. (G, cosmetic) After a thumbs-down with the keyboard the cursor's save-under can still hold pixels from before the kill
   frame; if the cursor sits in rows 126..185, hiding it (final message box) would paint a cursor-sized patch of the
   old picture. Only reachable when the cursor is low on the screen while voting with the arrows. Not worth a fix
   unless seen.

Corrections to `docs/GLAD_INDEX.md` (not edited here): 86A0 is "free CBM descriptor"; B4DA has no working Esc and no
"other key uses the mouse position" path; C1A4 draws the fallen opponent last (on top); the index's guess 5 (an FF
frame drawn) does not happen (section 5); 3233 = 3134 without the inner groove.

## 11. Easter eggs and jokes on the screen side
- No hidden text in the section or its DS block: every string in DS:7B06-801F is used (scan of the block). The MGLAD
  pictures were checked by eye at 4x-8x (the stand plaques, the imperial box, the gate): no hidden picture or text found.
- The gate in the arena picture holds three dark figures (the next fighters waiting?), and the imperial box has three
  spectators, one in red. G for the intent.
- "VERSUS" in the empty middle card between the two portraits; a beast opponent appears in the VERSUS card as a
  full big cat (Black Panther, Wild Lioness, Snow Leopard).
- The menu prices a beast by hunger: "Fat" 10 ... "Ravenous" 40 Talents (the menu for a man offers "Slave" as the
  second grade).
- The thumbs box is a pair of painted hands, thumb up on blue and thumb down on red.

## 12. Files
- Renders and scripts: `C:\Users\erikk\AppData\Local\Temp\claude\bfb5522be8ee045d\c4bf0f5f-57f9-4ea6-80b3-e1aa05bf0592\scratchpad\gladscr\`
  (`gfx.py` model of 7E74/7ECA/8E62/8790/3233/3134/8BE4/899C/59D4/6738, `comp.py` screens, `sheets.py` contact sheets
  and checks, `crop.py` zooms, `ds.py` DS tables; PNGs in `png\`).
