# End Turn reference images (Claude, 2026-11-09)

DOSBox-X on the copy of the game folder (C:\work\cent_play), Erik's game "erik" from 274 BC on. Breakpoints at End Turn entry
(0823:09BA) and exit (0823:0D0D); at each one the whole data segment (64 KB, DS = 1686) was saved:
`tN_in.bin` = before End Turn number N, `tN_out.bin` = after it (C:\DOSBox-X\hunt\turn.ps1 / turns.ps1).

Check: from the game folder, `python <repo>/tools/cmp_endturn.py tN_in.bin tN_out.bin`. It runs `centurion --end-turn tN_in.bin`
and compares every save block plus the dice (DS:00C8), war chance (DS:3D9C) and Rome's garrison record (DS:1A08).

| N | year | what happened | port |
|---|---|---|---|
| 1-3 | 274-272 BC | quiet turns | 0 bytes off |
| 4 | 271 BC | first random event: "You take a senator's daughter to the catacombs..." | 0 bytes off |
| 5 | 270 BC | enemy logic active from now on | 0 bytes off |
| 6 | 269 BC | Carthage declares war (army appears in Carthago) | 0 bytes off |
| 7 | 268 BC | Carthage's army sails into Mare Nostrum | 0 bytes off |
| 8 | 267 BC | into Mare Adriaticum; Macedonia declares war | 0 bytes off |
| 9 | 266 BC | Carthage lands at Rome: garrison battle (fought: Scipio's Defense, a glorious victory) | 0 bytes off with `--choices "0,1"` (battle ported) |
| 10 | 265 BC | Macedonia meets the garrison; battle fought (Balanced Army + Frontal Assault vs Frontal Assault: victory, Rome 1583 lost / 4201 left, Macedonia 2610 / 1990); then "The people of Rome are bored" | 0 bytes off with `--choices "0,0"` |
