# Reference saves (Claude, 2026-11-08, DOSBox-X on a copy of the game folder)

| File | What |
|---|---|
| fresh_275BC.csv | New game, saved at once (slot 5, "fresh"). Same state as `scr_newgame_map.bin`: `--dump-frame out.bin map --save fresh_275BC.csv` = 583 px off (outline, legion, cursor, ship details). |
| erik_275BC.csv | Erik's save "erik" (Narbonensis conquered, legion there; turn 0). |
| erik_turn2_274BC.csv | erik_275BC.csv after one End Turn in the real game (2026-11-09, slot 7 "erik2"). |
| turn2_274BC.csv | The same game after one End Turn, nothing else done (slot 6, "turn2"). Screenshot: turn2_274BC_map.png (year button shows 274 BC). |

## fresh → turn2 (42 bytes differ, 5 of them the name)
- **Turn counter: CSV 0x5FE, 0 → 1.** No ±275/274 anywhere in the save, so the year is derived: year = 275 BC − turn (one step observed; UNVERIFIED beyond that). Neighbours 0x5F9 1→6, 0x5FA 20→28, 0x6AA 0→108: unknown.
- **Every province record +0x19: 0 → 1** (CSV 0x55, 0x75, …, 0x395 = records 0-28 at 0x3C + 32·i). A per-province counter (turns held? ticks since an event?). UNVERIFIED meaning.
- Rome (record 0): manpower +0x04 5000 → 5175 (0x1388 → 0x1437); +0x08 0 → 32, +0x0E 10 → 5, +0x10 0 → 1.
- CSV 0x3A3: 75 → 77 (in the save's second block; unknown).
- Fleet array (CSV 0x48E) unchanged: zones 30, 27, 28.

## End Turn check (port/src/endturn.c, 2026-11-09)
`centurion --end-turn A.csv out.csv` must equal the real game's next-turn save in every game-state byte (bytes 0x04-0x33, the save name, excepted):
- fresh_275BC.csv -> turn2_274BC.csv: 0 bytes differ (42 changed by the game).
- erik_275BC.csv -> erik_turn2_274BC.csv: **0 bytes differ (44 changed by the game)**. This was a held-out test: made after the translation was written.
Both are turn 0 -> 1. From turn 5 on End Turn uses random events (not ported yet), so later turns need the RNG first.
