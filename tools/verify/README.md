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

Captures come from DOSBox-X's **Take raw screenshot** (`tools/dosbox/drive.ps1 menu "Take raw screenshot"`): the
emulated 640x480 frame as an indexed PNG with the palette widened to 8 bits (`v << 2 | v >> 4`), before any window
scaling. Some scripts reuse another's functions by reading its source up to a marker; keep those markers
(`def load_font`, `def bmp_pixels`, `for n in sys.argv[2:]:`, `W, H, img = png_indices`) when editing.
