# Game logic notes (Claude, from disasm/EXE_RUNTIME.clean.asm)

Status labels as in HANDOFF.md: PORTED = translated and checked against the real game; DECODED = read, not ported.

## End Turn: EXE func_0x09BA (PORTED, port/src/endturn.c)
Checked against two real next-turn saves (fresh game, and Erik's held-out save): 0 differing bytes. See tools/dosbox/results/saves/README.md.
Not yet ported inside it: the rest of the revolt handler 0x0D13 (0x0D98-0x0EDF, legions in a revolting province) and the two turn-5+ calls below.

## Dice: EXE 0x076B (DECODED)
```
roll(n): s1 = s1*0x7ABD + 0x1B0F; s2 = s2*0x1B0F + 0x7ABD;   (DS:00C8, DS:00CA, 16-bit)
         return ((s1 ^ s2) * n) >> 16                          (high word of the 16x16 product)
seed (0x075A): s1:s2 = BIOS ticks since midnight (int 1Ah AH=0: CX -> s1, DX -> s2)
```
DS:00C8/00CA are **not** in the save block table, so a save doesn't determine later turns. Test plan for turn 5+: in DOSBox, breakpoint
at End Turn entry (BP 0823:09BA), dump DS (snap.ps1) including C8/CA, let the turn run, save; then run the port's end_turn from that DS
image and compare the save. (The port seeds from the clock in play, like the original.)

## Turn 5+ events (DECODED outline, not ported)
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

## Province commands (EXE menu switch at 0x6D76)
0 Legions 0x4065 (~330 instr, 20 subroutines; DECODED outline only) | 1 Plunder inline 0x6DB2 | 2 Tribute 0x7CE0 (DECODED, spec in HANDOFF) |
3 Hold Games 0x3342 | 4 Build Fleet 0x2458 | 5 See Province List 0x69CC (layout DECODED) | 6 Continue.
Tribute Policy (Rome only) 0x7D42 (DECODED).

## Data
- Save = 24 DS blocks (table DS:5BDE), see SAVE_FORMAT.md. Province records 27 x 32 from DS:5270; legion records 12 x 20 from DS:1918;
  fleets 6 x 12 from DS:1114. Zone names DS:5222; map anchors DS:1AB8; outline seeds DS:3C52; adjacency DS:50EC (8 per zone, 0xFF = none).
- Menus: records {x0, y0, x1, y1, 0, items, title, count, 0xFF, 0xFFFF} + items {text, hotkey, x0, y0, x1, y1}; province DS:5790, Tribute DS:6054,
  Policy DS:60AE.
