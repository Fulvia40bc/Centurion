# Game logic notes (Claude, from disasm/EXE_RUNTIME.clean.asm)

Status labels as in HANDOFF.md: PORTED = translated and checked against the real game; DECODED = read, not ported.

## End Turn: EXE func_0x09BA (PORTED, port/src/endturn.c)
Checked against two real next-turn saves (fresh game, and Erik's held-out save): 0 differing bytes. See tools/dosbox/results/saves/README.md.
Not yet ported inside it: the rest of the revolt handler 0x0D13 (0x0D98-0x0EDF, legions in a revolting province) and the battles that
enemy armies start (below).

**Turn 5+ (2026-11-09): enemy armies 0x4C24 (port/src/enemy.c) and random events 0x1E98 (port/src/events.c) PORTED.** Checked against
eight real End Turns in a row (turns 1-9 of Erik's game, with a random event, Carthage and Macedonia declaring war and sailing toward
Italy): **0 differing bytes** in every save block plus dice, war chance and Rome's garrison record. The DS images at End Turn entry and
exit are in tools/dosbox/results/endturn (tN_in.bin / tN_out.bin); rerun with
`python tools/cmp_endturn.py tools/dosbox/results/endturn/tN_in.bin tools/dosbox/results/endturn/tN_out.bin` from the game folder.
Turns 9 and 10: the port predicted "The Marauding Army of Carthago / Macedonia Meets the Roman Garrison", and the real game showed exactly
that, then fought the battle (BATTLE, below), which the port stops at.

## Land battle (PORTED 2026-11-09, port/src/battle.c; map in docs/BATTLE_NOTES.md)
OVL0 section 2, entry DF24(army or 0xFF, legion or 12 = garrison, zone), returns 0xFFFF when Rome wins. The simulation, setup,
result, write-back and the **orders** are ported (graphics and the orders-mode screen/input are not): battle_order_legion_melee
("Release The Legion!", D21E), battle_order_legion_retreat ("Run Away!
Run Away!", D299), and per cohort (D33A) battle_cmd_begin
(refused for enemies, routed units and units outside the general's voice, D8AD) + target (drag, CBEE: one straight leg along the
dominant axis) / hold (clears the route but not an attack) / melee ("Release Cohort n", D7E8) / retreat ("Run Away!", D77C) / end.
They run from battle_orders_hook after the tick's end_of_tick (where D0F2 polls). Replays: `centurion --orders "T:ops;..."`.
Checked with a real battle with orders (battle/README.md, b11): identical, and End Turn 11 0 bytes off.
Wired into End Turn (enemy.c, 0x501E legions in the zone, 0x508D garrison with EXE 0x4733 assign_commander and the 0x50E3 copy of
Rome's troops), with the player's choices from `--choices "F,T;..."` in headless runs.
Checks (all byte-identical to DOSBox): Macedonia's battle from the formation menu and from the entry (tools/cmp_battle.py,
tools/dosbox/results/battle); End Turn 9 (Carthage vs the garrison, Balanced Army + Scipio's Defense: Rome 641 / Carthage 3146
casualties) and 10 (Macedonia, Balanced Army + Frontal Assault) including the battles: `cmp_endturn.py t9_in t9_out "0,1"`,
`... t10_in t10_out "0,0"`. Hannibal used Outflank, Philip Frontal Assault.
- 0xB0C2 = the naval battle (overlay 3, args (6, fleet)); result bit 2 = stop, bit 4 = the fleet retreats (0x5244). NOT YET PORTED.

## Dice: EXE 0x076B (PORTED, enemy.c)
```
roll(n): s1 = s1*0x7ABD + 0x1B0F; s2 = s2*0x1B0F + 0x7ABD;   (DS:00C8, DS:00CA, 16-bit)
         return ((s1 ^ s2) * n) >> 16                          (high word of the 16x16 product)
seed (0x075A): s1:s2 = BIOS ticks since midnight (int 1Ah AH=0: CX -> s1, DX -> s2)
```
DS:00C8/00CA are **not** in the save block table, so a save doesn't determine later turns. Test plan for turn 5+: in DOSBox, breakpoint
at End Turn entry (BP 0823:09BA), dump DS (snap.ps1) including C8/CA, let the turn run, save; then run the port's end_turn from that DS
image and compare the save. (The port seeds from the clock in play, like the original.)

## Random events 0x1E98 (PORTED, events.c)
First 0x21A4, the crises: Rome is bored (DS:0ED8, chance DS:0ED4 +5 per turn) or a governor wants games (DS:0EDA, province DS:0ED6,
chance DS:0ED5, starts at -20); either one blocks the events. Otherwise 20% per turn: one of 19 events (DS:0F02 {kind, score, talents};
kind 0 = once per game, 1 = repeatable). 0/1 storms sink 10-19% / 0-9% of the ships of each Roman fleet in the Mediterranean (0x3A32
then trims a legion aboard); 3 disease takes a tenth of the troops in the zone of one legion. **Bug in the original, fixed in the port (Erik, 2026-11-09):** it
counts the active legions (0x1F88) but then rolls with n = 0 (0x1FDE `sub ax,ax; push ax; call 0x76b`), and its pick loop (0x1FEE)
never skips empty records, so the disease always strikes **legion record 0**. The port picks the k-th active legion with
k = roll(count) (still one roll, so the dice stay in step); `centurion --original ...` keeps the 1990 behaviour, and
cmp_endturn.py uses it. Test: 16 outbreaks with four legions hit records 0/2/5/9 four times each (original: record 0 every time); the rest are texts with a score and/or talents change (-128 = half the tribute, 127 = all of it).
Original quirks kept as they are: an undefended province taken by an army raises DS:634E by a quarter (the status is read after it was
zeroed); places that read uninitialised locals are logged and skipped (enemy.c).

## Turn 5+ events (DECODED outline; now PORTED, see above)
- **0x4C24** (called by End Turn, returns early if turn < 5):
  - for each active record in the 14-byte table DS:C11A..C1B4 (enemy powers/armies): `call 0x4F27` (army AI, below);
  - war chance DS:3D9C = min(+10, 50); if roll(100) < it: `call 0x4DB5`, reset to 10;
    else n = number of Roman provinces; if table DS:3D52[n/5] > roll(100): `call 0x4E2D`;
  - armies DS:C114.. (14-byte records, +0 province, +2/+4 strengths, +6 active): strengths grow toward the province's DS:526E/526C values
    by +20/+100 per turn;
  - enemy fleets (fleet records 3-5, DS:113A..): strength grows toward caps (DS:3D90/3D96 tables, max 8000).
- **0x4DB5**: rolls a war/event (tables DS:3D2E, 3D58, 3D60), may call 0x4E2D, then 0x4EC1 (spawn an army).
- **0x4E2D**: picks a power from DS:C17C..C1B4 by dice, needs a non-Roman province (status 0), then 0x4EC1.
- **0x4F27**: army AI. Moves through the adjacency table DS:50EC by dice; on entering a Roman province: message
  "The Marauding Army of <name>\nMeets the Roman Garrison" (DS:3D9D / 3DB4) and **0x4733 = garrison battle** (dice-driven, auto-resolved?),
  plus 0x4BAC, 0x1E21, 0xB06E (to read).
- **0x1E98** (called by End Turn when turn > 4): random events; skipped if DS:0ED8 or 0EDA is set, otherwise 20% per turn: picks one of
  19 events (table DS:0F02, 3 bytes each; DS:0EDC marks used ones) and dispatches by index (0x1F12 switch).

So turn 5+ = dice + enemy AI + garrison battles + random events: port it together with the battle code.

### Enemy army records (DECODED 2026-11-09 from 0x4C24/0x4EC1/0x4F27)
Record i (0..10) at R = DS:C112 + 14*i: R+0 byte mode (1 = wanders at random, 0 = only into weaker zones), R+1 byte current zone,
R+2 word home zone (name for "The Marauding Army of <name>", strength caps), R+4 word strength (grows +20/turn to home's DS:526E),
R+6 word strength (grows +100/turn to home's DS:526C), R+8 word active (0xFFFF), R+0x0A..0x0C bytes copied from home +0x0C/+0x0B/+0x0A.
Records 0-6 = fixed powers spawned by 0x4DB5 (zone DS:3D58[i], home DS:3D60[i]; which one by the cumulative table DS:3D2E, 7 bytes per
era, era = min((turn-5)/8, 4)); 7-10 = rebels spawned by 0x4E2D in a random non-Roman province (mode 1 if roll(10) <= 4).
Army entering a zone with Roman legions: 0x4BAC + 0x1E21 + **0xB06E** (likely the battle module); entering Rome's garrison: message
DS:3D9D + name + DS:3DB4, then 0x4733 (garrison battle, 12) and 0xB06E. Enemy fleets 3-5 also grow here (cap 8000, DS:3D90/3D96).

## Difficulty
DS:048E..0491 (a save block) = difficulty per module: [0] land battle, [1] chariot race, [2] gladiators, [3] naval battle;
0 Galley Slave, 1 Legionnaire, 2 Senator, 3 Emperor (names via DS:0484). The menu shows "Fine Tuning" when they differ (0x702A).
The land battle reads [0x48E] at 0xB354 and 0xC178.

## Overlay sections (EXE table at 0xAFFA, 16-byte entries {load seg 1331, end seg, 0x75, file para, size para})
Thunks at EXE 0xB06E.. are `lcall 129B:02EC; dw section; jmp 0823:entry`. Section n = entry n-1: 1 = file 0x0000-0x09A0,
2 = 0x09A0-0x3EF0 **land battle** (disasm/OVL_BATTLE.asm, entry 0xDF24), 3 = 0x3EF0-0x51A0 (naval, entry 0xB0E0), 4 = 0x51A0-0x8550,
5 = 0x8550-0xB3F0; then DS from file 0xB3F0. All load at 1331:0000 = 0823:B0E0.

## Province commands (EXE menu switch at 0x6D76)
0 Legions 0x4065 (~330 instr, 20 subroutines; DECODED outline only) | 1 Plunder inline 0x6DB2 | 2 Tribute 0x7CE0 (DECODED, spec in HANDOFF) |
3 Hold Games 0x3342 | 4 Build Fleet 0x2458 | 5 See Province List 0x69CC (layout DECODED) | 6 Continue.
Tribute Policy (Rome only) 0x7D42 (DECODED).

## Easter egg: the indomitable village (DECODED, not yet seen live)
Map click (0x0839) → hit test 0x6C67(x = [B6], y = [B8]): per-row run-length zone table, row pointers at DS:3A96 ({zone, run length} pairs).
Code 0xFE marks 6 pixels on the north coast of Armorica: (50,74), (51,75), (52..55,76). Clicking there shows message DS:[57BC] → DS:5676
("Gaul was divided into three parts... No, four parts. One indomitable village still held out against the Roman invaders.") via 0x59D4,
then returns 0xFF (no zone). Marked image: tools/dosbox/results/asterix_village_marked.png. The port must keep it.

## Data
- Save = 24 DS blocks (table DS:5BDE), see SAVE_FORMAT.md. Province records 27 x 32 from DS:5270; legion records 12 x 20 from DS:1918;
  fleets 6 x 12 from DS:1114. Zone names DS:5222; map anchors DS:1AB8; outline seeds DS:3C52; adjacency DS:50EC (8 per zone, 0xFF = none).
- Menus: records {x0, y0, x1, y1, 0, items, title, count, 0xFF, 0xFFFF} + items {text, hotkey, x0, y0, x1, y1}; province DS:5790, Tribute DS:6054,
  Policy DS:60AE.
