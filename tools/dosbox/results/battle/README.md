# Land battle reference images (Claude, 2026-11-09)

64 KB DS images (DS = 1686) from DOSBox-X on the copy C:\work\cent_play, Erik's game "erik", turn 10 (265 BC), difficulty Galley Slave (DS:048E = 0).

- `b10_form.bin`: the Choose Formation screen of Macedonia's attack on Rome's garrison (the battle had already been entered, so setup
  and its dice are done). Unit table at DS:[C292] = F5CE, 29 records of 18 bytes: Rome 2 cavalry (150, 117), 11 x 500 + 17 infantry,
  general (5); Macedonia 4 x 150 cavalry, 8 x 500 infantry, general (5).
- `b10_exit.bin`: at the battle function's exit (0823:E212). Choices: **Balanced Army**, **Frontal Assault**, no orders during the battle.
  Report: "A Glorious Victory!" Philip of Macedon used Frontal Assault against Albertus Discreditus. Rome 1583 casualties / 4201
  survivors, Macedonia 2610 / 1990. Garrison (DS:1A08) troops 5517/267 -> 4201/0, experience byte +0x0B 75 -> 80; army record 1
  strengths 600/4000 -> 137/1853 and +8 active -> 0.
