# Land battle reference images (Claude, 2026-11-09)

64 KB DS images (DS = 1686) from DOSBox-X on the copy C:\work\cent_play, Erik's game "erik", turn 10 (265 BC), difficulty Galley Slave (DS:048E = 0).

- `b10_form.bin`: the Choose Formation screen of Macedonia's attack on Rome's garrison (the battle had already been entered, so setup
  and its dice are done). Unit table at DS:[C292] = F5CE, 29 records of 18 bytes: Rome 2 cavalry (150, 117), 11 x 500 + 17 infantry,
  general (5); Macedonia 4 x 150 cavalry, 8 x 500 infantry, general (5).
- `b10_exit.bin`: at the battle function's exit (0823:E212). Choices: **Balanced Army**, **Frontal Assault**, no orders during the battle.
  Report: "A Glorious Victory!" Philip of Macedon used Frontal Assault against Albertus Discreditus. Rome 1583 casualties / 4201
  survivors, Macedonia 2610 / 1990. Garrison (DS:1A08) troops 5517/267 -> 4201/0, experience byte +0x0B 75 -> 80; army record 1
  strengths 600/4000 -> 137/1853 and +8 active -> 0.

## b11: Sarmatia (rebel army 7, Malodor the Impaler) attacks legion I Ital (record 0) in zone 0, turn 11 (263 BC), with orders
Played by Erik and Claude, recorded by C:\DOSBox-X\hunt\watch.ps1 (snapshots at every breakpoint).
- `b11_entry.bin` at the battle's entry (0823:DF24, args read from the stack: army 7, legion 0, zone 0); `b11_exit.bin` at 0823:E212.
- Choices: **Strong Right** (2), **Stand Fast** (4); the enemy rolled Massed Army (4) + Sweep Right (1).
- Orders session 1 (Erik) after tick 0: cohorts 1, 2, 3 forward to (22,18), (24,26), (20,26), the general to (24,31)
  (`b11_orders1_open/done.bin` = when the orders mode opened / closed; tick = DF24's [bp-6] on the stack).
- Orders session 2 (Claude) after tick 245: Melee, "Release The Legion!" (`b11_orders2_open/done.bin`).
- Result: A Glorious Victory, Rome 1326 casualties / 2874 survivors, Sarmatia 1734 / 2166.
Check: `python tools/cmp_battle.py --entry b11_entry.bin b11_exit.bin 7 0 0 2 4 "0:u1 t22,18 e u2 t24,26 e u3 t20,26 e u10 t24,31 e;245:M"`
(identical), and End Turn 11: `cmp_endturn.py t11_in.bin t11_out.bin "2,4" "<same orders>"` (0 bytes off).

## Battle pictures
- `b11_frame_tick0.bin`, `b11_frame_tick245.bin`: the game's frame buffer (320x200 palette indices, from the 640K snapshots at
  linear 0x26870; the clean field is at 0x39420) when the orders mode opened; DS at the same moments = b11_orders1_open.bin /
  b11_orders2_open.bin. `python tools/cmp_battle_frame.py b11_orders1_open.bin 0 b11_frame_tick0.bin` -> 0 differing pixels
  (same for tick 245). port/src/battle_gfx.c.
- **Run these checks with the working folder C:\work\cent_play** (the port reads the game's files from the working folder)
  and give the result files with their full paths. From this results folder `cmp_battle_frame.py` reports 4630 differing
  pixels for tick 0 with every build (a false failure), and `cmp_battle.py b10_form.bin ...` prints "TACTICS.LZW not
  found (run in the game folder)" while still saying "identical" (V, 2026-11-12).
- The report: `centurion --report-shot endturn/t9_out.bin 12 1 641 3146 5659 1534 ../scr_battle_report.bin out.bin` draws
  Carthage's report over the captured screen: 0 differing pixels against scr_battle_report.bin.
