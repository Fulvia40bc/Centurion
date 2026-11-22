# Independent checkers

Plain Python (no packages), written from the format notes, **not** from the port's C code, so they can check it.
Run with `python -I`. None of them contain game data; they read the game copy or captures given on the command line.

| Script | Checks |
|---|---|
| `gifpy.py` | GIF87a decoder (Task 2/4 checks); its `decode()` is reused by the others |
| `flcpy.py` | FLC player (Task 5 check); `play()` reused by `flcmatch.py` |
| `xmipy.py` | XMIDI reader (Task 6 check) |
| `refseq.py` | XMIDI sequencer written by an agent that never saw `xmiseq.c` (Task 7 check) |
| `mid2txt.py` | reads `xmiseq_test`'s .mid back to `tick bytes` lines |
| `pngcheck.py` | a `plat_save_png` frame vs a GIF: CRCs, zlib, pixels, palette |
| `cmpframe.py A.png B.png` | two indexed frames: same palette index and same RGB, pixel for pixel |
| `bgmatch.py CAP.png GIF...` | which 640x480 GIF a capture shows, by index agreement |
| `flcmatch.py CAP.png FLC` | the same against every frame of an FLC, plus palette vs PAL.PAL >> 2 |
| `diffbox.py CAP.png GIF` | where a capture differs from a GIF, grouped into rectangles (the game's overlays) |
| `fontfind.py CAP.png FONTDIR "text"...` | finds text in a capture by rendering it with the FGF fonts (exact match, no stray pixels) |
| `rows.py CAP.png COLOUR X0 X1 Y0 Y1` | per row, the span of one colour (underlines, lines) |
| `palcrop.py CAP.png` | searches PAL.PAL's picture for pieces of a capture |
| `hsave.py IN.DAT OUT.DAT [--desc TEXT]` | all-human saves (Task 14): player types 2 -> 3 at nation +0x9A, human/computer counts at 0x4F, optional description, CRC recomputed; refuses non-0x38 / wrong-size / bad-CRC files |
| `dlgxy.py DUMP.bin DLG [ID]` | a dialog's button rectangles and centres from a data-object dump (Task 14); with ID, only that button's centre "x,y" |
| `flcframes.py FLC SHOT_PREFIX COUNT [X Y]` | `tools/dosbox/flcshots.sh` captures of fg_showflic, frame by frame against `flcpy.py` (the 0xF100 prefix frame counted), pixels and palette |
| `savediff.py A.DAT B.DAT [--ignore NAME]... [--max N]` | names each differing element of two version-0x38 saves (Task 18): field, record, sub-field, value `->` value; `--layout [A.DAT]` prints the field table (with a file, each element's value); `savediff_test.py` runs its synthetic-file tests (single-byte sweep over the whole file, refusals, end-game 44,066 shape, the three save editors) |
| `demo_regress.py record EXE GAMEDIR REFDIR [--seeds ...]` / `check EXE GAMEDIR REFDIR` / `xarch EXE_A EXE_B GAMEDIR` | whole demo games (Task 19): record the per-round data-object dumps per seed as the reference, check new runs against them round by round (volatile ranges masked by `port_volatile.txt`), or compare two builds directly; the game folder's save/cfg files are snapshotted and restored around every run. `fake_advciv.py` is the test stand-in for the exe, `demo_regress_test.py` the tests |

Save editors (test positions for DOSBox-X / port runs; they follow the port's reading of the save format, so they
make inputs, not checks). Each verifies the CRC first and rewrites it:

| Script | Makes |
|---|---|
| `cal_edit.py IN OUT "n:id[:giver] ..."` | the calamity phase (phase byte 15) with calamity cards added to hands (giver = the trader's nation, default 0xFF drawn) |
| `city_edit.py IN OUT "n:t ..." [ast=n:pos] [phase=P]` | cities built as CITY CONSTRUCTION's human path does (tokens back to stock, presence and occupied list as f581C4); `--show` lists each nation's territories |
| `elim_edit.py IN OUT PHASE "n ..."` | nations stripped of every token and city, so the elimination check (f22B00) removes them |

The 19 Nov runs, all from `C:\work\advciv_cmp\savgame3_tradestart.dat` (the verified round-4 trade start):
`cal_edit "1:13 2:15 3:17 4:19 5:1B 7:1A"` (the calamity phase, 183 clicks); `city_edit` with 21 cities, then
`cal_edit "2:14:3 3:16 4:18 5:1C 7:1D 8:1E:9 9:14"` (the tradeable calamities and city support, 27 clicks);
`city_edit "1:101 1:103 1:99 1:98" ast=1:15 phase=20` (an AST win); `elim_edit 20 "2 3 4 5 7 8 9"` (a conquest).
Slots 4, 7, 8, 9 of `C:\work\advciv_run`. The DOSBox-X step scripts (`run_steps.sh` "ADDR:X,Y ..."; the port takes
the same X,Y as `--wait 200 --click X Y`) are in `tools/dosbox/runs/`. Every slot file must be the same in both runs:
the restore menu reads all their descriptions into the data object.

Captures come from DOSBox-X's **Take raw screenshot** (`tools/dosbox/drive.ps1 menu "Take raw screenshot"`): the
emulated 640x480 frame as an indexed PNG with the palette widened to 8 bits (`v << 2 | v >> 4`), before any window
scaling. Some scripts reuse another's functions by reading its source up to a marker; keep those markers
(`def load_font`, `def bmp_pixels`, `for n in sys.argv[2:]:`, `W, H, img = png_indices`) when editing.
