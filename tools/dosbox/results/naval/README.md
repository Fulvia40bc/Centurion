# Naval battle reference captures (Claude, 2026-11-10)

Two naval battles Erik played in DOSBox-X on the copy C:\work\cent_play (difficulty Galley Slave, DS:0491 = 0) on
10 Nov, recorded by stopping the game at breakpoints (C:\DOSBox-X\hunt\battle1, battle2). Here, for each stop:

- `bN_nNNN_ADDR.bin`: the 640K memory snapshot cut after the work buffer (its first 0x36270 bytes: DS = 1686 at
  linear 0x16860, SS = DS, and the work buffer 2687:0000 at 0x26870). `state_load_mem` and the test hooks read it as
  they read a whole snapshot.
- `bN_nNNN_ADDR_vga.bin`: the VGA screen A000:0000 (64000 bytes, mode 13h palette indexes, no palette: compare indexes).
- `bN_nNNN_ADDR.txt`: the debugger's text at the stop (registers: BP and SP for `--naval-resume`).
- ADDR = the breakpoint, 0823:ADDR: B09E = B0E0's thunk (args f = [SP+2], zone = [SP+4]), 2C27 = right after B0E0
  returned to fleet_move 2B40 (EAX = B0E0's result), BEAE / BB7E / D3F2 / D60C / B59F = those routines' entries.

| file | what |
|---|---|
| b1_n002_B09E, b1_n003_2C27 | battle 1's first try: fleet 0 into zone 30, Erik pressed Esc at "Choose a flag ship:"; B0E0 returned 0 |
| b1_n004_B09E | fleet 0 (45/20/15, no legion) into zone 30 against Carthage's fleet 3 (50 quinqueremes, army 11 of 5000 aboard) |
| b1_n005_2C27 | after the battle: won (EAX 1), Carthage 0 ships, Rome 38/16/13; the screen = the last sinking frame (Roman MSINK, shark) with "0 SHIPS 67 / 19 MEN 0" |
| b2_n001_B09E, b2_n002_2C27 | a move of fleet 0 into zone 29, no enemy there: B0E0 returned 1 |
| b2_n003_B09E | fleet 0 (38/16/13) into zone 27 against Aegyptus's fleet 4 (100 galleons, army 12 of 5520) |
| b2_n005_BEAE | BEAE's entry: the set-up done (Erik chose the trireme, C37B = 0; the enemy has a galleon, C37A = 2); the screen = the intro's last frame |
| b2_n006_BB7E | BB7E's entry: a head-on ram (bits 3); the Roman escort all sunk by the fleet rounds during the duel (C404 = 0), Roman crew 0; the screen = the last duel frame |
| b2_n007_D3F2 | D3F2's entry (bits 1: the Roman flagship scuttled); the screen = the last boarding frame + b479's panel |
| b2_n008_D60C | D60C's entry; the screen = D3F2's last frame |
| b2_n009_B59F | B59F's entry (outcome 1 lost, mode 0, flags 1); the screen = D60C's last frame + DC8E |
| b2_n010_2C27 | after the battle: lost (EAX 0) |
| b2_flagship_trireme.txt, b1_flagship_quinquereme.txt, b1_flagship_esc.txt | port input scripts (screen.c) for the flagship menu: a click on item 0 / item 1, Esc |
| b2_flagship_trireme_intro.txt | the trireme click, then "wait 18": the port's intro C105 runs 18 ticks before the script ends it, so it ends on pennant frame 0 as the game's did (any wait 17..20 does; 0..4 none, 5..8 frame 1, 9..12 frame 2, 13..16 frame 3, 21..24 frame 1 again) |

Not kept: battle 1 n001 (the boot stop) and battle 2 n004 (a broken dump: Erik's manual Alt+Pause, segment 1 is a copy).

## Checks (run from the game folder C:\work\cent_play; `--exe` defaults to C:\work\Centurion-native\centurion.exe)

Both tools also run the port twice more with the snapshot's work buffer and the input screen filled with FD and with FE.
A pixel that comes out the same in both runs was drawn by the port itself ("drawn" below, whatever colour it drew); the
others are the input picture, i.e. the DOSBox capture passed through, and their match with the game's screen says
nothing about the port. The tools print both counts and the box of the input pixels.

Pictures, `tools/cmp_naval_frame.py` (centurion --naval-frame; naval.c naval_test_frame):

| command (N = tools/dosbox/results/naval) | result |
|---|---|
| `cmp_naval_frame.py --original N/b2_n006_BB7E.bin N/b2_n006_BB7E_vga.bin duel 4 0` | 0 differing pixels; drawn 62023, 0 of them differ; from the input 1977, all in rows 186..199 (the panel background, the black line and "SHIPS", drawn only by BEAE) |
| `cmp_naval_frame.py --original N/b2_n005_BEAE.bin N/b2_n005_BEAE_vga.bin intro 0` | 0 differing pixels, drawn 64000 (pennant frame 0; frames 1 / 2 / 3 / none: 104 / 152 / 134 / 78). This mode re-types C105's loop body (the 0x80 erase of (38,2)-(68,15), FLAG frame K at (38,5)) and K is the one that fits; the port's own C105 is checked by the set-up resume with b2_flagship_trireme_intro.txt below |
| `cmp_naval_frame.py --original N/b1_n005_2C27.bin N/b1_n005_2C27_vga.bin after 1 3 0` | 93 differing pixels, all at (160,109)-(170,124): the EXE's mouse cursor (the port shows the system pointer); drawn 64000. D60C runs here on the dice left at the END of the battle (after the real D60C and all the DA50 rounds), so this checks D60C's sprites and places, and its dice only by chance: the last shark frame hardly depends on them (other dice in DS:00C8..CB give 93 px too, now and then 100). The shark dice are checked by battle 2's D60C resume below |
| `cmp_naval_frame.py --original N/b2_n006_BB7E.bin N/b2_n006_BB7E_vga.bin check` | CF7F rects, C641 bearing 2/10, CFDD, CE7E touching: the same; C4F8 distance 25 against the game's 32, because the game computed it at the top of the last pass, before both ships moved (32 = C558's rounding for one of the pre-move positions; NAVAL_SCREEN_NOTES 18). The tool says "DIFFERS" for that reason. |

Resumed battle, `tools/cmp_naval_resume.py` (centurion --naval-resume; naval.c naval_test_resume): the port continues
from one stop to the next with the snapshot's dice and timer, then the DS is compared byte by byte and the screen pixel
by pixel. Bytes the port does not keep as the EXE does are counted per class and listed by the tool (clock, interrupt
handlers, mouse cursor and its visibility stack, allocator, song segments, the loader's buffers, sheet pointers, the
stack, the heap); the clock class's words (the tick DS:42, timer slot n at DS:46 + 4n) are printed one by one.

| command (all `cmp_naval_resume.py --original`) | DS game-state bytes off | clock words off (port / game) | screen | drawn by the port |
|---|---|---|---|---|
| `N/b2_n003_B09E.bin N/b2_n003_B09E_vga.bin B09E BEAE CBB2 CBC4 N/b2_n005_BEAE.bin N/b2_flagship_trireme_intro.txt` | 0 (9AFA = 80 as the game's: the intro's erase colour) | tick 6F3B / 713B, slot 0 6F39 / 7136 (the game showed the intro some 530 ticks, the port 18) | 0 px | 64000 |
| the same with `N/b2_flagship_trireme.txt` (the click only) | 0 (9AFA 0F, the game's 80) | tick, slot 0 | 78 px: the pennant (the script ends the intro before C105 draws its first frame) | 64000 |
| `N/b2_n006_BB7E.bin N/b2_n006_BB7E_vga.bin BB7E D3F2 CBB2 CB8E N/b2_n007_D3F2.bin` | 0 (dice, crews, courage, the text in AC60) | tick only, 8AE1 / 8BF8 (the game waited for Erik's click on the box); slots 2 and 8 exact | 0 px | 61711 (from the input 2289, rows 186..199) |
| `N/b2_n007_D3F2.bin N/b2_n007_D3F2_vga.bin D3F2 D60C CBB2 CB92 N/b2_n008_D60C.bin` | 0 (the smoke dice) | tick 8CF4 / 8CF9 | 0 px | 59520 (from the input 4480: the whole panel strip, rows 186..199) |
| `N/b2_n008_D60C.bin N/b2_n008_D60C_vga.bin D60C B59F CBB2 CB92 N/b2_n009_B59F.bin` | 0 (the shark dice, DA50) | tick 8E3B / 8E46, slot 0 8E2D / 8E36 | 0 px | **2332 only**: D60C restores its rects from the work buffer, which here is the capture's (from the input 61668) |
| `N/b2_n009_B59F.bin N/b2_n009_B59F_vga.bin B59F 2C27 CBB2 CB8E N/b2_n010_2C27.bin` | 0; B0E0 returns 0 as EAX | tick 8E46 / 9276 (the box waited for a click) | 93 px at (148,92)-(158,107): the mouse cursor | **0**: the result box is drawn and taken down again, the screen is the input |
| `N/b2_n001_B09E.bin N/b2_n001_B09E_vga.bin B09E 2C27 CBB2 CBC4 N/b2_n002_2C27.bin` | 0; returns 1 | none | 0 px | **0** (no battle: nothing drawn) |
| `N/b1_n002_B09E.bin N/b1_n002_B09E_vga.bin B09E 2C27 CBB2 CBC4 N/b1_n003_2C27.bin N/b1_flagship_esc.txt` | 0; returns 0 | tick 1536 / 1538 | 0 px | 6161 (from the input 57839) |

The real checks of the D60C and result-box screens are the chained runs from BB7E's entry over the stops in between
(BB7E's input screen is the last duel frame, which the duel check above shows the port draws itself):

| command (all `cmp_naval_resume.py --original`) | DS game-state bytes off | clock words off (port / game) | screen | drawn by the port |
|---|---|---|---|---|
| `N/b2_n006_BB7E.bin N/b2_n006_BB7E_vga.bin BB7E B59F CBB2 CB8E N/b2_n009_B59F.bin` | 0 | tick 8D1F / 8E46, slot 0 8D11 / 8E36 | 0 px | 62023 (from the input 1977, rows 186..199: BEAE's panel background) |
| `N/b2_n006_BB7E.bin N/b2_n006_BB7E_vga.bin BB7E 2C27 CBB2 CB8E N/b2_n010_2C27.bin` | 0; returns 0 | tick 8D1F / 9276, slot 0 8D11 / 8E36 | 93 px: the mouse cursor | 62023, 93 of them differ (the cursor) |
| `N/b2_n007_D3F2.bin N/b2_n007_D3F2_vga.bin D3F2 B59F CBB2 CB92 N/b2_n009_B59F.bin` | 0 | tick 8E36 / 8E46, slot 0 8E28 / 8E36 | 0 px | 59884 (from the input 4116, rows 186..199) |

Dice (DS:00C8..CB) end exactly as the game's in every stage, so each stage ran the game's number of frames (D3F2's
smoke dice and D60C's shark dice are rolled per frame). The tick and timer slot 0 are off by a few ticks in the set-up,
D3F2 and D60C stages (G: DOSBox ran the game's frames slower than 72.8 Hz); in BB7E only the tick differs (the game
waited for the click) and slots 2 and 8 are exact.

The same results without `--original` (no 1990 bug the port fixes is on these paths). Battle 1's set-up from
b1_n004_B09E (`... B09E BEAE CBB2 CBC4 OUT_DS OUT_VGA N/b1_flagship_quinquereme.txt`) gives courage 80 / 80, the values
the end state implies (Carthage's army 85 - 5, Rome 75 + 5, then the boarding's +10 to Carthage: 90 / 80). Battle 1's
write-back was checked by hand (NAVAL_LOGIC_NOTES 17); the duel passes of both battles cannot be replayed.
