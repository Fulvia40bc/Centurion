# Reference saves (Claude, 2026-11-08, DOSBox-X on a copy of the game folder)

| File | What |
|---|---|
| fresh_275BC.csv | New game, saved at once (slot 5, "fresh"). Same state as `scr_newgame_map.bin`: `--dump-frame out.bin map --save fresh_275BC.csv` = 583 px off (outline, legion, cursor, ship details). |
| turn2_274BC.csv | The same game after one End Turn, nothing else done (slot 6, "turn2"). Screenshot: turn2_274BC_map.png (year button shows 274 BC). |

## fresh → turn2 (42 bytes differ, 5 of them the name)
- **Turn counter: CSV 0x5FE, 0 → 1.** No ±275/274 anywhere in the save, so the year is derived: year = 275 BC − turn (one step observed; UNVERIFIED beyond that). Neighbours 0x5F9 1→6, 0x5FA 20→28, 0x6AA 0→108: unknown.
- **Every province record +0x19: 0 → 1** (CSV 0x55, 0x75, …, 0x395 = records 0-28 at 0x3C + 32·i). A per-province counter (turns held? ticks since an event?). UNVERIFIED meaning.
- Rome (record 0): manpower +0x04 5000 → 5175 (0x1388 → 0x1437); +0x08 0 → 32, +0x0E 10 → 5, +0x10 0 → 1.
- CSV 0x3A3: 75 → 77 (in the save's second block; unknown).
- Fleet array (CSV 0x48E) unchanged: zones 30, 27, 28.
