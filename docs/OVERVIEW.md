# Advanced Civilization: program map and porting plan

Stage 1 report (2026-11-17). Claims are marked **D** decoded from the bytes, **V** verified by a run of the
original, **G** a guess still to be checked. Nothing here is V yet: the original has not been run.

## 1. The files

Erik's installed copy: `C:\Users\erikk\Desktop\adcivi` (67 files, 21.9 MB, never touched). Work copy:
`C:\work\advciv_play` (identical; SHA-256 prefixes in `docs/ORIGINAL_FILES.sha256`). All tools read the copy.

It is a full hard-disk install of the CD version (animations and sounds local, no `ANIMS\` on a CD needed).
`ADVCIVIL.BAT` / `ADVCIVV1.BAT` from the install are not in the folder (they are still inside `ACIV.SHR`).

| File(s) | What it is | |
|---|---|---|
| `AC_START.EXE` (6 KB) | Watcom C/C++16 real-mode launcher (memory check, then runs AC_MAIN) | D (G: what it checks) |
| `AC_MAIN.EXE` (1.2 MB) | the game: DOS/16M loader + DOS/4GW Professional kernel + the 32-bit LE program (§2) | D |
| `ACIV.CT1` .. `CT5` | catalog archives, 32 entries each (§4) | D |
| `P2`, `P3`, `P4` | GIF87a 640x480 title pictures (intro slideshow after the FLCs) | D |
| `AH_LOGO.FLC`, `ADVCIVIL.FLC` | Autodesk FLC 640x480 intro animations | D |
| `PAL.PAL` | a GIF87a (1716x568) that the game reads its palette from | D (G: palette only) |
| `MAPDATAZ.DAT`, `BORDERS1/2.DAT` | map geometry (territory outlines / borders) | G |
| `LANDADJ`, `COASTADJ`, `SEAADJ.DAT` | adjacency tables (land, coast, sea) | G |
| `TERRFLAG.DAT` | territory flags; versioned ("terrflag.dat is wrong version. It is %d and should be %d") | D (header G) |
| `3.RL` .. `8.RL` | per player count: `u32 n`, 0x2084 bytes, 0x5644 bytes, n bytes (loader at 0x3F409); precomputed tables | D layout, G purpose (AI / map analysis) |
| `SAVGAMEA.DAT`, `savgameb.dat` | the game's own autosaves, 44,062 bytes each: `savgamea.dat` "AutoSaved after SHIP MAINTENANCE - R4" (written after every phase), `savgameb.dat` "AutoSaved at end of Round 3" (written at every round end). Both were already in the folder Erik copied (every file there has the same copy time, 2026-11-17 14:52). They are from an earlier session on the machine this install came from; Erik made no save. | D |
| `ACIV.CFG` (6 bytes) | settings | G |
| `ACIV.SHR` | the installer's compressed archive of the install files (DOS dates 1995-10-04); redundant | D |
| `*.MDI`, `*.DIG`, `MDI.INI`, `DIG.INI`, `AILDRVR.LST`, `SETSOUND.EXE` | Miles AIL 3.03 sound drivers and setup; Erik's setup: SB Pro 2 FM (SBPRO2.MDI) + SB16 digital | D |
| `EHGM2FM2.AD` / `.OPL` | FM timbre bank for the AdLib/OPL drivers (identical files) | D |
| `README.TXT` | v1.1 notes and the bug-fix history of 1.01 / 1.1 | D |
| `TDUJAM.CNF`, `JAMMER.DIG` | 1 byte / a "Jammer" digital driver (G: a sound card) | G |

**Version.** README says 1.1; the EXE's banner says **"Version 1.11 Beta 1"** (DS 0xB00A7). The port follows this EXE.

## 2. AC_MAIN.EXE

| Part | File offset | |
|---|---|---|
| DOS/16M loader + DOS/4GW Professional kernel ("Copyright (c) Rational Systems, Inc. 1990-1994") | 0 .. 0x352A3 | D |
| MZ stub of the game, `e_lfanew` → LE header | 0x352A4, LE at 0x37CF4 | D |
| LE data pages (201 pages of 4 KB, last 0x61D) | 0x67CA4 .. end of file exactly | D |

LE objects (unbound by `tools/le_unbind.py`; link-time bases, DOS/4GW relocates them at load time):

| Object | Base | Size | |
|---|---|---|---|
| 1 code, 32-bit | 0x00010000 | 0x960D1 (600 KB) | D |
| 2 data, 32-bit (incl. BSS and stack) | 0x000B0000 | 0x72C00 (459 KB; 200 KB initialised) | D |

Entry `0x8BF54` = Watcom `_cstart_` (jumps over "WATCOM C/C++32 Run-Time system"). 22,314 fixups, all
internal 32-bit offsets, no imports: one flat program. **Compiler: Watcom C/C++32 (1994), C++** (the module
name `ah_civl.cpp` appears; exception-handling runtime is linked). No symbols, but many routines print their
own names in traces or errors ("NationLosesUnits", "DrawTradingCards", "DrawInitialDrawingScreen", "PBEMViewer"),
which will name them.

Code regions, by the strings each region references (G for the boundaries, refined in M0):

| Linear range | Contents |
|---|---|
| 0x10000-0x13FFF | `main`: command line, memory/XMS checks, catalogs, sound init, intro, palette (ah_civl.cpp) |
| 0x14000-0x1BFFF | map / start positions per player count (the `.bpm` names), civilization-advance purchase |
| 0x1C000-0x2FFFF | calamities: barbarians, volcano/earthquake, treachery, civil disorder, iconoclasm, piracy, famine, civil war, flood, epidemic, superstition, slave revolt |
| 0x30000-0x37FFF | PBEM user info, nation tables |
| 0x38000-0x3FFFF | UI kit (fonts, buttons, text), map data loading (`mapdataz`, borders, adjacency, `N.RL`) |
| 0x40000-0x4FFFF | phase menus, help screens, scoring, start of game, movement, save/load |
| 0x50000-0x67FFF | PBEM files, trade sessions, ship movement, conflict, city building, ship maintenance, tax |
| 0x68000-0x77FFF | trade cards and the trading screens |
| 0x78000-0x7BFFF | animations (FLC + WAV) |
| 0x7C000-0x7FFFF | Watcom C runtime (stack check `__CHK` D at 0x7D4AC, called from 633 functions) |
| 0x80000-0x87FFF | Miles AIL 3.03 (with its debug trace "AIL_install_driver(0x%X,%u)" …) |
| 0x87000-0x9FFFF | Fastgraph 4.04 (+ Fastgraph/Image 2.04: GIF/FLC, Fastgraph/Fonts 2.04), C++ runtime, DOS/4GW glue |

So about **440 KB of game code (≈620 functions)** to translate, roughly three times Centurion. The ≈170 KB of
libraries are not translated but replaced (C library, a small 8-bit rasterizer, our own GIF/FLC/XMI code).

**Recursive disassembly (M0, done 2026-11-17).** `tools/rdisasm.py` (run by `tools/disasm.ps1`) follows the
code from the entry point, every direct call and jump, address-taken code (fixups) and the jump tables, and
writes `disasm/AC_MAIN.asm` (labelled listing, not committed), `disasm/functions.txt` and `disasm/coverage.txt`:
- **1,485 functions**, 179,447 instructions; 95.5 % of the code object decoded, 1,761 jump-table entries;
  0 overlapping instructions, every call target decoded. The rest (20 KB in small gaps) is alignment padding,
  data and dead code without a recognisable prologue.
- Below 0x7C000 (the game, ah_civl.cpp): **627 functions, 444 KB** (V for the ≈620 estimate above), of which
  20 are reached by nothing (dead code). From 0x7C000 on (libraries): 858 functions, 170 KB, 44 unreached
  (mostly AIL API the game never calls).
- 106 code addresses taken as immediates proved to be data (jump-table bases, the "con" string of the Watcom
  start-up): rejected because they decode into instructions a compiler never emits.
- Names: 70 AIL functions from AIL's own trace strings (D), the Watcom start-up and stack check (D), and
  8 game functions from the identifier strings they pass (G: e.g. `NationLosesUnits`, `DrawTradingCards`,
  `DoShipContruction` (sic)). Hand-kept names go in `disasm/names.txt`.
- Watcom register calling convention throughout (first arguments in eax, edx, ebx, ecx); the game was built
  with stack checking on (`push framesize; call __CHK` at the top of 633 functions).

## 3. Platform

- **Video**: 640x480x256 SVGA through Fastgraph 4.04 (chipset autodetect, `/V1` forces VESA). The port draws
  into a 640x480 8-bit buffer + 256-colour palette and reimplements only the Fastgraph calls the game uses.
- **Sound**: Miles AIL 3.03. Music = 12 XMIDI tracks in `ACIV.CT5` by Eric Heberling (credits), three
  arrangements of the civil / map / trade / calamity themes.
  - (D, 0x10BEE) At start-up the game reads `MDI.INI` and picks the set by the driver file name (case-insensitive
    compare 0x877AD):
    - `ADLIB.MDI` or `SBLASTER.MDI` (mono FM, 9 voices) → `f*`
    - `SBPRO1.MDI` or `SBPRO2.MDI` (Sound Blaster Pro, stereo FM) → `s*`
    - any other driver, or no `MDI.INI` → `g*` (General MIDI)
  - So `s*` is the stereo-FM arrangement, not a Sound Canvas one (an earlier guess here, now corrected). Erik's
    setup (SBPRO2.MDI) played the `s*` set.
  - All twelve files differ in size: three distinct arrangements.
  - Other FM drivers (`OPL3.MDI`, `ESFM.MDI`, `ADLIBG.MDI`) get the GM set. Their notes go through the FM timbre bank
    `EHGM2FM2`; the "EH" is probably Heberling's initials (G).
  - The game's own Preferences screen (strings at 0xB4547) has Sound effects + Volume, Music + Volume and Animations,
    saved in `aciv.cfg`. There is no in-game choice of music set; that came from SETSOUND.EXE's driver choice.
  - Digital = 17 WAVs in `ACIV.CT5` (calamities, market place, Zeus, `KABOOM3.WAV`).
- **Input**: mouse + keyboard. **Files**: catalogs, saves, PBEM files (`*.hst`, `pbem_out\`, `PREP????.ZIP`
  via pkzip).
- **Command line** (D): `/?` `/NS` `/NT` `/NZ` `/SRD[3-8]` `/VT` `/Vx`, and the undocumented **`/C[n]`** (§6).

## 4. Data formats

`ACIV.CTn` (D, corrected 2026-11-18): a **chain of directory blocks**.
- Each block is 32 entries of 24 bytes, `{char name[12]; u32 offset; u32 length; u32 kind}`, then a link entry whose
  name is twelve 0xFF bytes and whose offset is the next block (0 = the last).
- The first block is at offset 0. Unused entries have an empty name.
- Kinds: 2 = GIF, 6 = FLC, 7 = FGF font, 0 = other.
- Until 2026-11-18 only the first block was read (Task 1's spec said "count = first offset / 24"). That hid 76 files (139 named in the first blocks):
  CT1's second block and CT3's blocks 2-4.
- With the chain, all five catalogs account for every byte up to the end of the file: 215 files.
- `tools/ct_list.py` lists and extracts.

| Archive | Contents |
|---|---|
| CT1 | 37 GIFs: map zoom levels (`ZOOM3..ZOOM78`, by player count), BRPC, Zeus, nation picture cards (`CH_*`, `CV_*`), `AH_LOGO.GIF` and `ADVCIVIL.GIF` (never named by the game, §6); the three fonts `BLOCK05.FGF`, `COGNAC10.FGF`, `COGNAC19.FGF` |
| CT2 | 32 GIFs: screens (trade, AST, census, buy civ, victory, map & palette, tokens, minimap, card art) |
| CT3 | 34 help texts `*.HLP` (one per phase/calamity) and 66 `*.BPM` opening maps for the computer players (§6, item 12) |
| CT4 | 14 FLC calamity animations (+ `VOLCANO.FLC`); the other 18 entries of its block are unused |
| CT5 | 12 XMI + 17 WAV |

GIF/FLC/WAV/XMIDI are standard formats; the HLP files are plain text with layout spaces. Still to decode:
the map files, `TERRFLAG.DAT`, `N.RL` contents, the `.BPM` values, the save layout, `ACIV.CFG`.

**Fonts (D, 2026-11-18).** The game defines three fonts from CT1 at start-up (0x39E50, names at 0xC1948):
- 0 = `block05.fgf`: 5 pixels high, small capitals.
- 1 = `cognac10.fgf`: 11 high, baseline 2.
- 2 = `cognac19.fgf`: 22 high, baseline 4.

The FGF format, from Fastgraph/Fonts 2.04's define routine (0x8DFEE):
- `u8 bytes_per_row, u8 height`, then `u8 width[96]`, `u8 rows[96]`, then 96 cells of bytes_per_row × height
  bytes, for characters 32..127.
- A glyph is `rows[c]` rows of `(width[c]+7)/8` bytes, top row first, most significant bit leftmost.
- The loader turns each glyph bottom row first, as Fastgraph draws bitmaps from their lower-left corner.
- The baseline is the number of blank rows under 'A'.

Print (0x8E253):
- Starts at the graphics cursor, which marks the baseline; glyph bottoms are `baseline` rows lower.
- Each character advances by its width + the spacing (0x8E244; the game uses 0 and 2).
- 0x7F + colour changes the colour mid-string. 0x7F + `_` starts and ends an underline 2 rows below the baseline.
- Horizontal and vertical justification exist but stay at "none" (no writes to 0xE0A80/81).

**Palette (D, V, 2026-11-18).** The game's palette is `PAL.PAL`'s global colour table, each byte shifted right by 2
into the VGA DAC's 6 bits.
- Fastgraph/Image's `fg_gifpal` (0x878F7: `lodsw; shr ax,2; and al,3Fh`) is called at start-up (0x11177), then
  `fg_setdacs(0, 256, …)`.
- The game's fade-in (0x3CFE4) does the same from black in 64 steps. Each step: `acc += target+1` while
  `acc < 64·target`, DAC = `acc/64 − 1`, floored at 0. At the end it writes the exact target.
- V: the fade's working buffer at 0xF5D58 in the DOSBox-X dumps equals `max(0, (x>>2)−1)` for all 768 bytes, the
  last fade step; the final palette is written from 0xF8B40.
- PAL.PAL's 1716x568 picture is never displayed for the palette (only `fg_gifpal` reads it). Whether another
  screen uses the picture: G, to check.
- Port: `port/src/palette.c`, GIF → DAC (>>2) → screen RGB (`v<<2 | v>>4`, as DOSBox-X). `plat_demo` now shows
  P2-P4 through it: 245 of P2's 256 colours change slightly, as on the real VGA.

**M2 against the original (V, DOSBox-X raw captures, 2026-11-18; scripts in `tools/verify/`):**
- **Pictures:** the intro's three stills are P2, P3, P4, each 307,200 of 307,200 pixels by palette index.
  - The port's headless P2 frame (`plat_demo`) equals the capture in every pixel's index and RGB.
- **Palette:** the main menu's palette equals PAL.PAL >> 2 (widened `v<<2|v>>4`) in all 256 entries.
- **Main menu:**
  - The background is `PARTHA.GIF` (ACIV.CT2). It differs only inside the game's overlays: the 21 title letters
    (y 48-67), the three buttons (x 212-427), the pointer, and the version text.
  - "START A NEW GAME" (x 267, baseline 152), "RESTORE A SAVED GAME" (255, 192), "QUIT TO DOS" (286, 412): COGNAC10,
    spacing 0, colour 67.
  - "Version 1.11B1" (5, 468): COGNAC10, colour 15.
  - Every one is an exact match: all glyph pixels, no stray pixel of the text colour in its box.
- **Credits:** "Music" (290, 191) and "Bill Levay" (271, 311): COGNAC19, spacing 0, colour 13, exact.
  - The credits underline is drawn by the credits code, not the font's 0x7F `_`: 6 rows under the baseline, from
    2 pixels before the text to 1 past its end (row 197, x 288-349 under "Music").
- BLOCK05 has not appeared on these screens yet.

Ported: `port/src/fgfont.c`. `port/test/font_test.c` renders every glyph upright, with descenders below the
baseline. A pixel check against a DOSBox-X capture follows with the first screen that has text.

## 5. Porting plan

Same method as Centurion: decode each routine, translate literally into C11 with the EXE address in a comment,
verify against the original in DOSBox-X. Each module lands with its checks; 1990s bugs fixed by default,
`--original` keeps them; headless hooks so every check runs without a window.

| # | Module | Verified by |
|---|---|---|
| M0 | Tooling: recursive disassembler (entry + calls + fixups; jump tables), function list named from the trace strings; DOSBox-X profile for the game (S3 SVGA, XMS, DOS/4GW); protected-mode memory dumps (find the load base of object 2 by its first bytes, then dump it linearly); frame capture at 640x480 | listing covers every call target with 0 overlaps; a dump of the data object matches `obj2.bin`'s initialised bytes at start-up |
| M1 | Platform layer: **SDL3** (window with scaling, keyboard/mouse/pen/touch, audio stream mixing FluidSynth, the FM synth and the WAVs) behind a `platform.h` like Centurion's; headless `--dump-frame`. Builds for ARM64 and x86_64 Windows with llvm-mingw (SDL3 3.4.20 built from source by `tools/build_deps.ps1`: the official mingw package has no ARM64 build); Mac and Linux possible later | builds both; headless run writes frames |
| M2 | Assets: catalog reader, GIF decoder, palette from PAL.PAL, FLC player, HLP text, fonts | each title/screen GIF pixel-for-pixel vs a DOSBox-X capture; FLC frames likewise |
| M3 | Fastgraph subset: identify each fg_ routine the game calls (≈80 G), implement those | frame diffs of the first screens = 0 px |
| M4 | UI kit (0x38000-0x3FFFF) + the map: zoom levels, borders, tokens, cities | map screen of the R4 autosave in the originals = 0 px |
| M5 | Game state + save/load (44 KB layout) | load → save round trip byte-exact; DOSBox save of the same state = the port's |
| M6 | Phases in turn order, human side: tax, population expansion, census, ships, movement, conflict, cities, surplus, trade cards, trade, calamities, civ advances, AST | per phase: the game autosaves after every phase ("AutoSaved after SHIP MAINTENANCE - R4"), so load the autosave before the phase in both, same random state, compare the next autosave byte-for-byte |
| M7 | AI (the largest part, spread through the phases) | the self-running demo `/SRD` and the hidden continuous test `/C`: whole AI-only games, compared autosave by autosave against DOSBox-X from the same seed |
| M8 | Animations and sound: calamity FLCs, WAVs, XMI sequencer + synth (§7, question 2) | FLC frames vs captures; music by ear (Erik) |
| M9 | Hot seat (PBEM dropped, question 5) | a hot-seat game with two human players, run side by side with DOSBox-X from the same save |
| M10 | Window polish as Centurion: resizable 4:3, Alt+Enter / F11, no console | Erik |

**The random-number generator (M0, settled 2026-11-17).** The game's own `rand` at 0x78AE4 (D):
`seed = seed * 22695477 + 1` (0x015A4E35), returns `(seed >> 16) & 0x7FFF`; the seed is the dword at 0xCAC5C
(1 at start-up, V). Only five places touch it (D):
- a **new game** seeds it from the clock (0x117DA): `_dos_gettime` (0x87E29, int 21h AH=2Ch) into 0xF422C, then
  seed = hundredths << 24 | hours << 16 | seconds << 8 | minutes; this runs only when bit 0x04 of the flags at
  0xE1624 ("new game") is set, and the restore dialog (0x5FF28) clears that bit when a game is loaded;
- **save** writes it (0x5EB7B, 4 bytes) and **load** reads it back (0x5F27C): it is in the save file, at offset
  **0x3D0C** of `SAVGAMEA.DAT` (V: 0xDE9B76A2 in the R4 autosave in the originals = the seed in memory right after loading);
- 0x55963 / 0x55B28 save it and put it back around a block in the movement code (G: a trial run that must not
  use up random numbers); 0x4FE86 copies it to 0xF40F4 (G: the seed a game or PBEM round started with).

**Reproducible (V, DOSBox-X 2026-11-17):** the R4 autosave in the originals loaded in two fresh DOSBox-X sessions and left to
run to the same point (the AI players' ship construction, then Africa, a human player, is asked to choose):
seed 0xA7BB1DC8 in both, and the whole data object identical except 228 bytes of timers, mouse and sound-driver
state and the AI think-time counters (0x10F2C0..: `clock()` before and after each AI move, G). So M6/M7 can run
the original and the port from the same autosave with the same input and compare autosaves byte for byte.

**DOSBox-X (M0, 2026-11-17):** profile `tools/dosbox/advciv.conf` (S3 SVGA, 32 MB, SB16 at 220h with OPL3,
normal core so breakpoints fire) runs the run copy `C:\work\advciv_run`; `tools/dosbox/drive.ps1` drives the
debugger console without focus and the game window (screenshots, clicks, keys) with it. DOS/4GW loads the code
object at **0x212000** (link 0x10000, delta +0x202000) and the data object at **0x2A9000** (link 0xB0000, delta
+0x1F9000), selectors 0180 (code) and 0188 (data, flat). So a link address A in the code is `BP 0180:A+202000`,
and the data object is `MEMDUMPBIN 0188:2A9000 72C00 file`. `tools/dosbox/find_objects.py` finds both
objects in a flat dump and compares them with `le_unbind`'s images (V, at the first DOS time call in start-up):
code identical except 2 bytes in `_cstart_`, the 84,936 relocated bytes identical after rebasing; data identical
except 200 bytes the C start-up has already set (stack limits, the debugger flag).

**M1 platform layer (2026-11-18, first version):**
- `port/src/platform.h`, the SDL3 backend `plat_sdl3.c`, and `png_write.c`.
- The window:
  - The largest whole multiple of 640x480 within 85% of the desktop.
  - Letterboxed 4:3 at any window size, scaled with SDL's pixel-art sampling.
  - Alt+Enter or F11 for full screen.
- Input: events in order.
  - Keys carry the PC set-1 scan code and the US-layout ASCII code, as INT 16h gave them (G until the game's key
    reader is decoded).
  - Mouse in game pixels. Pen and touch arrive as the mouse.
- Sound: one 44.1 kHz stereo stream with a fill callback and a lock.
- `PLAT_HEADLESS` opens nothing. `plat_save_png` writes the last frame as an 8-bit indexed PNG, so palette indices
  can be compared exactly with DOSBox-X captures (M2).
- `port/test/plat_demo.c`: P2-P4 with the civil theme playing. Built by `tools/build_port.ps1` into
  `C:\work\advciv_port\<arch>`, one folder per CPU.
- Check (V, headless, ARM64 and x64):
  - P2's PNG is identical on both CPUs.
  - An independent Python check finds valid CRCs and zlib stream, 0 of 307,200 pixels different from a separate
    GIF decode, and the palette identical.
- The windowed run is Erik's to try (Claude doesn't open windows unasked).

**M3 + first slice of M4: the title menu (2026-11-18).**
- `docs/LIBRARY_CALLS.md`: the 63 library routines the game calls, each identified by one agent and checked by a
  second that tried to refute it: 50 confirmed, 13 corrected in detail, 0 refuted.
- The port loads the data object from `AC_MAIN.EXE` at run time (`le_image.c`, identical to `le_unbind.py`'s
  output). Translated code reads and writes the game's globals at their original addresses (`state.h`), as Centurion
  does.
- `fg.c` reimplements that Fastgraph subset for mode 25 (640x480x256), from the reference.
- Translated, each function with its address:
  - the text routines (`ui_text.c`, 0x39F90-0x3AE3C);
  - the buttons and dialogs (`ui_dialog.c`, 0x3A7B0-0x3C9D0, 0x785D4);
  - screen, mouse and palette fade (`ui_screen.c`);
  - the start-up steps the menu needs and the menu itself (`title.c`, 0x10010.. and 0x4F184).
  The play-by-email branches are left out (Erik's ruling).
- Each file had a translator and an independent reviewer. The reviewers fixed three details in `fg.c` (the
  mouse-visibility byte, `cat_show`'s type byte and flags mask) and found no errors in the game code.
- **Check (V):** `advciv.exe GAMEDIR --headless-frame OUT.png --pointer 320 240` renders the main menu identical to
  the DOSBox-X capture: **0 of 307,200 pixels differ**, Fastgraph's own pointer included, on ARM64 and x64. Palette
  identical in all 86 colours used.
  - The first run differed only in "RESTORE A SAVED GAME": the menu enables it only if a `savgame?.dat` exists in
    the current folder. The port now changes into the game folder, as the original runs there.
- **BUG(1995), fixed by default:** the word-wrap routine 0x3A42C drops the first letter of a word that has to be
  broken at the box edge (e.g. "Mediterranean-" in a narrow box prints "iterranean-"). `--original` keeps it.
- Not yet translated: restore (main 0x11557 → the restore dialog 0x5FF28), quit's clean-up beyond the exit, the
  font release 0x8E0F8.

**M4, the new-game screens (2026-11-18).**
- Translated and reviewed in a second workflow:
  - "New Game Options" (`setup.c`: 0x4DD50, 0x4E2D8) and "Variants" (`variants.c`: 0x4EAE0, 0x4EEC8);
  - the button helpers (`ui_widgets.c`);
  - main's new-game loop (`title.c`: 0x111DC..0x113FE, and 0x11168 for going back to the menu).
- The sprites: start-up 0x118F8 cuts 12 sprites out of `maped.gif` (0x3AE90, `fg_getimage`; `fg_unpack` is a copy in
  mode 25). They include the two tick boxes.
- **Check (V, DOSBox-X raw captures on the run copy, after clicking START A NEW GAME and then VARIANTS):**
  - The setup screen equals the capture except the play-by-email row: its label and box, 551 pixels, left out by
    Erik's ruling. Fastgraph's pointer at (321,151) matches too.
  - The Variants screen equals the capture in all 307,200 pixels, pointer at (521,429) included.
  - Same on ARM64 and x64. The defaults agree with the original: 3 humans, 0 computers, 2 trading sessions, minimum
    goods 6, Normal AI (greyed with no computer players).
- `advciv.exe … --headless-frame OUT.png --click X Y …` scripts clicks without a window.
  - Each click is 2 input polls released, 2 pressed, then released (the game waits for a release before each click).
  - The frame is saved 4 polls after the last click.
- Note: in DOSBox-X, `drive.ps1 click X Y` lands the emulated mouse at about (X+1, Y+1).

**M4, the new-game loading step (2026-11-18).** Main 0x114CF..0x1184A, translated and reviewed in a third workflow:
- `load.c`: terrflag.dat, mapdataz.dat, borders1/2.dat, the area and territory tables.
- `gfxload.c`: the 273 game sprites, the adjacency lists and the land matrix, N.RL, greying the territories not in play.
- `newgame.c`: card stacks dealt and shuffled with `game_rand`, nation records, the music data, and `game_load`.
- The start-up parts the comparison showed missing, in `title.c`:
  - aciv.cfg (0x107A4);
  - the sound start-up's data side (0x10B4D..0x10E7F: flags 0xF625B/0xF625C/0xF625D and the music-set names);
  - the CRC-32 table (0x7BB14 at 0x10577);
  - the Watcom initializer 0x60A80, which runs before main from the XI table at 0xE15EA (the table's other eight
    entries are library start-ups).
- Heap pointers the original keeps in 32-bit fields hold port handles (`load_heap_*`, `ui_sprite_*`).

**Check (V, 2026-11-18):** the original was stopped in DOSBox-X at main 0x1184F, the call into the game loop, after
START A NEW GAME and START GAME.
- How: break on int 21h AH=2Ch, then set `BP 0180:21384F` while in protected mode. A breakpoint set in real mode or
  before DOS/4GW starts resolves `0180:` as a real-mode segment and never fires.
- Its data object was dumped and the port run with the same initial seed (`--seed 5F0F0B18`, from the clock bytes
  at 0xF422C) to the same point (`--dump-ds`).
- `tools/verify/dscompare.py` turns DOS/4GW's relocated pointers back into link addresses: the 1,068 fixup sites,
  plus 173 run-time pointers.
- Result: the final random seed is identical (0xA941EB2A, after all the shuffles). **0 unexplained bytes of
  470,016**. The 44,533 differing bytes all fall in the 30 by-design ranges of `tools/verify/by_design.txt`, each
  with its reason:
  - library state (AIL, Fastgraph, Watcom runtime), which game code does not write;
  - port heap handles, paths, the clock, the mouse;
  - the fade buffers (the port starts as with /NT);
  - the stack.
- `tools/verify/dsclassify.py` sorts differences by whether game or library code references them.

**M4, module N1: the nation choice for human players (2026-11-18 evening).** `choose.c` (0x2EFA0, 0x2F9F8, 0x2FC04,
0x300E0) and `ui_kit2.c` (0x3AB7C, 0x3A9F4, 0x3AF54, 0x79A9C, 0x78A20, 0x3C940, 0x3C7A0), translated and reviewed in a
workflow (plan: docs/MAP_PLAN.md; inventory of the 115 functions under 0x2EFA0: docs/INVENTORY_2EFA0.json). The AST
button (0x136B0) and the computer nations (0x302C4) stop with `not_ported()` until N2/N3.
- **Check (V):** setup defaults (3 humans). The clicks: START A NEW GAME, START GAME; SELECT; right arrow, left
  arrow; SELECT; SELECT.
  - Frames F1 ("Player One choose your nation"), F4 ("Player Two"), F2b/F3b (after the arrows) and F5
    ("Player Three") equal the DOSBox-X captures: **0 px each**, Fastgraph's pointer at (320,240) included, on ARM64
    and x64.
  - Memory at main 0x11854 (the return of 0x2EFA0), same seed (`--seed 3D0F2338`): **0 unexplained bytes**; the
    final seed is identical.
- **Clicks without focus (`tools/dosbox/pokeclick.sh`):** the desktop was locked that evening. The script breaks in
  through DOSBox-X's menu ("Start DOSBox-X Debugger"), stops in protected mode on the loop's mouse call
  (`BPINT 33 00 03`: AH then AL), then at the loop's `cmp [0xE1630],0`, and writes x, y and button into
  0xE1628..0xE1633 (`SM`, every byte with a leading 0: the debugger reads "AC" as the alignment flag).
  - Raw screenshots need the emulation running for a frame: take them after F5.
  - Posting window messages to DOSBox-X does nothing: SDL ignores them.
- The next screen, captured for module START: "AFRICA place your token in a starting territory" on the zoomed map
  with the full game panel.

**M4, module N2 (2026-11-18 evening):**
- `ast.c`: the AST chart 0x136B0 in its three modes, and the scoring helpers 0x147C4, 0x1486C, 0x17760, 0x6DD98,
  0x7BA64.
- `ui_kit3.c`: the generic click wait 0x78BA4 and 12 button/dialog helpers.
- `dispatch.c`: the button dispatcher 0x3FB94, all 261 jump-table ids. 210 are translated (29 default, 156
  constant codes, 24 that store [0xF6758], QUIT, and id 3, the AST chart). The other 51 stop with `not_ported(case
  address)`: the work list of the later modules, with each case's target in the file.
- 0x3CC68 and 0x3CCBC (show the background buffer; screen to back buffer) added by hand to `ui_screen.c`.
- **Check (V):** setup defaults; clicks START A NEW GAME, START GAME, AST, CONTINUE, SELECT, AST, CONTINUE, SELECT,
  SELECT.
  - Frames F5 (the AST chart from the nation choice), F6 (back at the choice) and F7 (AST after one SELECT): 0 px
    each, pointer included (ARM64; F7 also on x64).
  - Memory at main 0x11854 (`--seed 0B102E0E`): 0 unexplained bytes. This includes the CONTINUE button 0x3BB48 moved
    in dialog 0xBB800.

**M4, module N3: the computer nations (2026-11-18 night).** `nations.c`: 0x302C4 (which nations the computer
takes, by player count 3..8: fixed orders, pairs and triples drawn with `game_rand`, five jump tables) and 0x30174
(one nation to the computer: token, name box, 750 ms wait). The nation choice now runs to its end for every setup.
- **BUG(1995), fixed by default (`--original` keeps it), V in DOSBox-X:** the computer can get more nations than
  the setup asked for.
  - A: after taking a nation the code counts down what is left to assign, except in nine places where it counts
    *up*: Africa (all six player counts), Iberia (4 and 5 players), Asia (5 players). With 0 or 1 human players
    every setup still comes out right, which is likely why it went unnoticed; with more humans, some choices give
    the computer one or two extra nations.
  - B: with 7 players, two single takes after Africa/Egypt/Iberia are not guarded at all.
  - The full table (every setup, every human choice) is in `nations.c`'s header, from the translator's and the
    reviewer's independent models.
  - Confirmed in the original, as predicted:
    - 3 players, 2 humans (IBERIA, ILLYRIA) + 1 computer: the computer gets AFRICA *and* CRETE;
    - 4 players, 3 humans (ILLYRIA, THRACE, CRETE) + 1 computer: AFRICA and IBERIA;
    - 7 players, 4 humans (ILLYRIA, ASIA, ASSYRIA, BABYLON) + 3 computers: AFRICA, IBERIA, CRETE and EGYPT.
  - The fix changes no `game_rand` call, so the seed is the same in both modes. The fixed port differs from the
    original by exactly that one nation byte.
- **Check (V):** ten setups run in DOSBox-X to main 0x11854 (`tools/dosbox/run_setup.sh`, per-human choices with
  `/`):
  - 2H+1C (twice), 1H+2C (twice), 2H+2C (twice), 2H+3C, 2H+4C, 3H+4C, 2H+6C: these reach all five jump tables;
  - plus the three bug setups (port with `--original`).
  - The port with the same seed and clicks (`tools/verify/portcmp.sh`): every frame at the stop **0 px**, every
    data object **0 unexplained bytes**, every final seed identical.

**M4, module S1: save and restore (2026-11-18 night).** Translated and reviewed in two halves:
- `save.c`: RESTORE GAME 0x5FF28, SAVE GAME 0x5E7D0, the slot dialog 0x601D8, the one-line text input 0x60694, the
  version check 0x6016C, 0x3CD10, and main's restore path 0x11557..0x115A9 (`title_restore`, wired into main).
- `saveload.c`: the save reader 0x5F05C (73 freads in file order, the 61→69-byte area conversion of old versions),
  the CRC check 0x7BBF0 / 0x7BB84 (on the table 0x7BB14 builds), the AI's random table 0x7BCD4, the trade-offer
  table 0x726DC.
- The original runs `system("copy ...")` for save and for restoring an autosave; the port copies the file in C.
- **BUG(1995) fixes (`--original` keeps each):**
  - The restore loads the file into the game *before* the CRC and version checks, so a rejected ("corrupted",
    "earlier format") save still overwrites the game in progress. Fixed: check first, load only an accepted file.
  - The "Restoring" screen measures the description in the wrong font, so the fallback for long descriptions never
    runs and one of 50 characters runs off the screen.
  - The text input clears the click flag and then tests it, so CANCEL cannot be clicked while a description is
    typed (only Esc works).
  - Esc in the text input leaves the back buffer as the drawing surface.
  - Two only for save versions the game then rejects anyway (a 22-byte tag read for versions before 0x28; a stale
    AI random table for versions 0x2D/0x2E).
- Checked by the translator (V): both autosaves of the run copy pass the CRC (0); the reader consumes 44,058 of
  their 44,062 bytes (the last 4 are the CRC) and sets the seeds 0xDE9B76A2 (R4) and 0x3AA893EE (end of round 3).
- **Check (V):** title menu → RESTORE A SAVED GAME in DOSBox-X and the port (`tools/dosbox/run_restore.sh`):
  - the slot dialog (12 slots, the two autosaves' descriptions): **0 px**;
  - slot 'a', the "Restoring" screen (chariot, the quoted description): **0 px**;
  - slots 'a' (R4 autosave) and 'b' (end of round 3): the data object at main 0x115B0 (after the return of 0x5FF28
    and the flag clear at 0x115A9): **0 unexplained bytes** each, seeds 0xDE9B76A2 and 0x3AA893EE as in the
    original. Restoring 'b' copies savgameb.dat over savgamea.dat in both, the same bytes.
  - Noted for later modules (D, the run copy's files): the original, let run on past 0x115B0 after restoring 'b',
    rewrote savgameb.dat with flag byte 0xE1624 = 02 instead of 06 (and a new CRC). That is game code after the
    restore (not ported yet); the run copy's saves were put back from `advciv_play` afterwards.
- `--click` scripts reach the restore too: `advciv GAMEDIR --click 320 186 --click 320 354 --dump-ds OUT.bin`.
- The restore then goes on through main 0x115B0..0x1177C, shared with a new game (`game_continue`), to 0x1188C:
  0 unexplained bytes against DOSBox-X there too.

**M4, module MAP1: the map window (2026-11-18 night).**
- `map.c`: the redraw 0x3D52C (scroll clamp, the map picture, borders, markers, cities, tokens and ships, the
  mini-map with its viewport box), borders 0x3D18C, tokens 0x3DAD8, ships 0x3DE40, the number sprites, 0x3CBBC and
  the two labels.
- `mapdata.c`: the scroll in pixels, the home view 0x3D9FC, centring 0x3DA60, the highlight setters, the territory
  aliases 157..159 (a second piece of land for three territories), the hit test 0x3D2A4 (now used by the click
  wait) and 0x3F558.
- **BUG(1995), fixed by default:** 0x3F558 frees the N.RL block but zeroes the wrong field, leaving the pointer to
  the freed block (harmless in the original as long as the block is reloaded before a second free).
- **Checks (V):**
  - `port/test/hit_test.c`: the hit test against an independent reading of mapdataz.dat, 14 map variants × 9
    scrolls × every screen point: 28,166,400 points, all agree.
  - Frame: the R4 autosave restored in both; in DOSBox-X the original runs on to its game screen
    (`tools/dosbox/run_restored_screen.sh`: Africa's ship construction, Sabrata highlighted). The port copies the
    map's plain state from that dump (`--map-state`: scroll, highlights, territory, area, city and nation records,
    player, phase, detail level; no heap handles) and redraws. Map window **0 px** (199,056 px, pointer included),
    mini-map **0 px** (`tools/verify/rectdiff.py`). The 1-px ring around the mini-map comes from the game panel,
    not ported yet.
  - The dispatcher's map cases are translated (scroll arrows 0x1B..0x22, mini-map click 0x23, TOGGLE UNITS 0x24).
    In DOSBox-X (`tools/dosbox/run_map_states.sh`) the original was clicked through eight presses from the restored
    R4 screen: TOGGLE UNITS (3 → 0 → 3), and with the detail level poked to 1 and to 2 (TOGGLE UNITS only swaps 3
    and 0; 1 and 2 come from the toggles box): right, down (clamped at row 7), mini-map, left, up, up, TOGGLE UNITS.
    The port replays each from the dump before it (`--map-state BEFORE.bin --press ID`): every frame 0 px in the map
    window and the mini-map, scroll and detail level identical. So all four detail levels, odd and even rows (the
    35-px half steps), both clamps and the mini-map arithmetic are covered.
  - Note for test scripts: in the original the dispatcher's mini-map case reads the mouse position again after the
    click wait, so a poked click there uses the real pointer (320,240 in these runs; the port replays it with
    `--mouse 320 240`).
  - Not covered yet: the movement phase's drawing (tokens under ships in sea mode, the ship values), which needs a
    save in MOVEMENT (module MOVE).

**M4, module PANEL (2026-11-18 night).**
- `panel.c`: the nation status panel 0x459E4, the ZOOM overview 0x3E374, the message-strip eraser 0x3A990, the
  mirrored sprite 0x3AFF4, the player-button labels 0x783B8, and appending/removing dialog records 0x7853C/0x78664.
  The dispatcher's player buttons 9..0x10, ZOOM 0x25 and DETAIL's zoomed branch are translated.
- `shippanel.c`: the ship panel 0x5D14C with its labels 0x5DDF4, the territory refresh 0x426D0 and the count
  buttons 0x572CC (used in MOVEMENT; their frame check comes with MOVE).
- **BUG(1995), fixed by default:** the overview draws the viewer's own ships on top of shared stacks, but its two
  ship loops compare with the nation *before* the viewer's (P-1), and the second loop stops at nation 8. So nation
  P-1's ships end up on top, and Egypt's never do. It only changes which 2-px stack lies on top.
- Oddities kept (D): the overview prints Illyria's token counts with one extra shadow copy (as MAP1's ship
  numbers do for Iberia); the status panel reads the ships count unsigned and the others signed.
- **Check (V):** from the restored R4 screen the original was clicked through the eight player buttons and ZOOM
  (`run_map_states.sh`); the port replays each from the dump before it:
  - all eight nations: status panel and name box, and the map window at that nation's home view: **0 px** each;
  - the ZOOM overview (Africa's): **0 px** in the map window, with and without `--original` (no shared ship
    stack involves Africa or the nation before it here).
- Oddity (D): in the movement phase the number on a ship stack gets one extra shadow copy (colour 5), only when the
  current player is nation 2 (Iberia, 0x3E24A); G: so it stays readable on that nation's colour.

**M4, module START: the starting territories (2026-11-18 night).** A new game in the port now runs on through main
0x11854..0x11884 to 0x1188C (`--through-start`).
- `start.c`:
  - 0x6B9AC: one coin flip per computer nation;
  - 0x454D0: the "%s place your token in a starting territory." screen, with its own click loop;
  - 0x3D434: a map-screen click;
  - 0x45F88: a computer's pick from its opening map;
  - 0x3A924 and 0x15FE0 (loading a .BPM's three tables).
- `aistart.c`: 0x14C6C, which opening map (`.BPM` in ACIV.CT3) each nation gets, by player count and by which
  neighbours are in play. 58 maps, 13 jump tables. A second interpreter written by the reviewer agrees on all
  135,212 cases.
- **BUG(1995), fixed by default:** a copied test. For a human Illyria in a 4-player game, the choice between ILLY40
  ("non-iberia") and ILLY41 tests Illyria's own byte (always set) where Africa's belongs. It only matters in a
  4-human Iberia/Illyria/Thrace/Crete game.
- **Script steps for the port:**
  - `--wait N`: N input polls with no click (the nation choice's closing waits poll too);
  - `--settle N`: polls before the frame is saved.
- **Check (V)**, `tools/dosbox/run_start.sh` against the port with the same seed and clicks:
  - 2 humans (Africa, Italy) + 1 computer (Crete):
    - both placement screens **0 px**;
    - memory at main 0x1188C **0 unexplained bytes**, the same final seed;
    - the last frame's map window and mini-map 0 px. The rest of that frame is not comparable: DOSBox-X's raw
      shot comes after the original has run on into its game loop, which redraws the panel and hides the pointer.
  - 2 humans (Africa, Iberia) + 4 computers (Thrace, Crete, Asia, Egypt; 6 players): the same results. The four
    computers pick areas 130, 86, 4 and 129 from their opening maps, as in the original, and the final seed is
    identical.
  - The original autosaves once it runs on past the stop; `run_start.sh` keeps and restores the run copy's saves.

**M4, modules INFO, MENU1, MENU2: the information screens and the menus (2026-11-18 night).**
Translated in one workflow (three translators, three reviewers):
- `info.c`: census 0x13E50, Civilization Tools 0x19C10 with the advance pop-up 0x1A124, Goods Held 0x76A9C with
  "Select Nation To Show" 0x76798 and the hands, the Mining list and hand value, Trading Wrapup 0x763FC/0x764A8.
- `menu1.c`: the options menu 0x42754, HELP 0x4290C with its file/title table 0x42CEC, TOGGLES 0x436FC.
- `menu2.c`: AUDIO/VISUALS 0x43E88, the Scoring History graph 0x44D5C, the full-map view 0x44A34, the WAV loader
  0x7B504.
- The original writes the screen to a GIF file "junk" (or "junk2") only to read it back; the port keeps those
  pixels in memory (three copies of the helper for now, to merge later). No junk files are left in the game folder.
- Sound: the Miles calls in 0x7B504 and 0x43E88 are port no-ops for now; every data write of the original is kept.
- title.c now writes "volcano.wav" into 0xF549C at start-up, as main 0x10752 does. It is the sound the volume arrows
  play, not the intro's, so the range left by_design.txt.
- **BUG(1995) fixes (`--original` keeps each):**
  - **City Support help.** The City Support phases show Ship Construction's help, because a copied case kept its
    strings. CITYSUP.HLP (416 bytes, the city-support rules) is in ACIV.CT3, but the game never names it. The port
    shows it, under the title "City Support" (port text, for Erik's ruling: TEXT_SWEEP.md).
  - **Civilization Tools clicks.** A click in the 4-px strip under a row, or one dragged off a row before release,
    gives an index past the 24-entry advance tables, so the description pop-up reads a wild pointer. The port keeps
    the index inside the clicked row's grid.
  - **Scoring History.** It stores this round's score without the column clamp the AST phase uses. From round 45 it
    writes into the next nation's row, and for Egypt over a heap pointer.
  - **The "R O U N D S" label** is drawn in the grid's grey with 37..43 rounds and an odd count.
  - **Trading Wrapup** saves rows 0..478 of the screen, leaving the background's last row.
- Kept, for Erik (D): Trading Wrapup cannot scroll. Its dialog has only CONTINUE, so only the first 6 trades are
  ever shown.
- **Check (V):** each screen pressed in DOSBox-X from the restored R4 screen (`run_map_states.sh`) and replayed in
  the port (`--map-state BEFORE --press ID`; the port now stops when the pressed button's screen waits for input):
  - CENSUS, CIVILIZATION TOOLS, GOODS HELD, SCORING HISTORY: **0 px over the whole frame**;
  - the options menu, AUDIO/VISUALS, TOGGLES and HELP ("Ship Construction", 825 bytes of help text): **0 px** in
    their boxes. Their rounded corners show the screen underneath, which the port's test hook does not redraw.
  - Not yet checked: clicks inside these screens (a toggle and its aciv.cfg write, the advance pop-up, the volume
    arrows), Trading Wrapup (needs a save after TRADE) and the full-map view (button 8, AST phase only).

**M4, module MOVE: the movement primitives and buttons (2026-11-18 night).**
- `move.c`: the eight primitives and the dispatcher's movement cases:
  - primitives: tokens off/onto a territory, the ship move, a nation leaving a territory, ship selection and two
    ship tests;
  - cases: MOVE TOKENS 0x26, MOVE SHIPS 0x27, UNDO MOVE 0x28, the count buttons 0xC4..0xC8, and EMBARK, DISEMBARK,
    DISEMBARK ALL, NEXT SHIP on both coasts.
- **BUG(1995), fixed by default:** NEXT SHIP writes its play-by-email move-log record even in a normal game. There
  the log pointer is 0, so each press writes two bytes into the first kilobytes of DOS memory (the interrupt vector
  table, then the BIOS data area); by the 4th press it hits INT 9's (the keyboard's) vector. G: a long session
  with many NEXT SHIP presses could crash the machine. Every other ship action tests for PBEM first.
- `port/test/move_test.c`: 38 hand vectors, 200,000 random operations against a second implementation, and 29
  dispatcher cases. Five deliberately broken copies of move.c were all caught.
- **A save in the MOVEMENT phase, from the original itself:** restore R4, click FINISHED; the original autosaves
  "AutoSaved after SHIP CONSTRUCTION - R4". It is installed in the run copy as savgame0.dat (slot 0).
- `tools/dosbox/run_steps.sh` plays clicks in any input loop (here f78BA4's 0x78BD3 for the computers' "Movement
  for X complete" screens, then the human movement loop 0x55FFD) and dumps the state at every stop. The computers'
  moves replay identically in every run.
- **Check (V):** in DOSBox-X, Africa (the human, moving last):
  - land: TUNISIA, count 2, THAPSUS (2 tokens move), UNDO MOVE;
  - ship (one ship poked in at SABRATA, as Africa has none in this save): MOVE SHIPS, SABRATA, EMBARK, DISEMBARK,
    THAPSUS (the ship sails), UNDO MOVE.
  - The port replays each dispatcher button from the dump before it (`--map-state`, which now also copies the
    movement state and the panel dialog, label pointers turned back into link addresses). It is compared with the
    dump after it over the area, nation, movement, highlight and dialog state (`tools/verify/rangecmp.py`).
  - count 2, UNDO (land), EMBARK, DISEMBARK, UNDO (ship): **0 differing bytes**; both UNDOs redraw the map window
    at **0 px**.
  - MOVE SHIPS: 1 byte differs, Tunisia's ship-box colour. The case returns -2 and the movement phase's own loop
    (not ported yet) then highlights the selection.
  - The map clicks themselves go through the movement phase (0x55xxx, the game loop), which comes with M6.

**M4, module STACK: the World State Stack, in-game SAVE / RESTORE / NEW GAME, REPLAY (2026-11-18 night).**
- `stack.c`:
  - the stack: 0x7A8D0 allocate, 0x7A940 push, 0x7AC60 pop, 0x7ABD0 free;
  - 0x121A0, which frees the game sprites;
  - the dispatcher's SAVE GAME 0x4F, RESTORE GAME 0x50, START NEW GAME 0x51 and REPLAY 0x101.
  - With these, every dispatcher case is translated except SHOW PBEM, which is dropped by ruling.
- **main's two setjmps are real now.** NEW GAME's longjmp lands on the title menu (main 0x11036 → 0x1103B → 0x11155
  → the menu; not on the setup screen as MAP_PLAN guessed). RESTORE's longjmp lands at 0x115D3, after the tables
  and sprites; game_continue is split there (game_continue_load / game_continue_rest), and that path takes the
  restored-game branch at 0x1177C. In a `--press` test the port stops at the jump instead.
- **BUG(1995) fixes (`--original` keeps each):**
  - START NEW GAME frees 28 kinds of game sprites but not 15 token sprites, which the next start cuts again (about
    2 KB lost per in-game new game).
  - REPLAY copies its post-movement snapshot one record off (records 0..159 and nations 0..10, where the stack
    saves 1..160 and 1..11). So area record 160 and nation 11 are never shown as they were after movement, and
    records 0 keep the snapshot's bytes afterwards.
  - Text: "World State Stack has popped once to often." → "too often".
- Noted (D, main's code, for later): every in-game RESTORE cuts the minimap sprite again without freeing the old one.
- **Check (V), REPLAY** on the MOVEMENT save (the original keeps a round-3 snapshot there): the original was clicked
  REPLAY from its first movement screen. The port replayed it from that state with slot 0 restored.
  - "Map shows Round 3 post movement state.": map window and message box **0 px**;
  - the area, territory-state and nation records, the replay flag, the message area and the button label equal
    the original's (0 differing bytes; in both modes, because records 0/160 and nation 11 don't differ here).
  - The stack depth is 1 in the original and 0 in the port. The movement phase had pushed one level before REPLAY
    (for its RESTART), which the test hook does not copy.
  - Not yet run against the original: SAVE GAME's file (its description is typed: needs keyboard input in both,
    MAP_PLAN risk 4), and the in-game RESTORE and NEW GAME jumps (they need the game loop, M6, to land somewhere
    the port can compare).

**M6 begins (2026-11-19): the save writer, the helpers, the game loop.** Plan: docs/M6_PLAN.md, inventory:
docs/INVENTORY_M6.json (383 functions, 309 KB).
- `savewrite.c` (Mistral, Task 12; one fix by Claude): the save writer 0x5E9B0 and the autosave 0x5E8EC. The port's
  autosave reproduces five autosaves the original wrote, byte for byte.
- Helpers, each with unit tests against an independent implementation:
  - `rules.c` (16 shared rule helpers);
  - `kit.c` (message boxes, YES/NO, button-by-id);
  - `wqsort.c` (Watcom's own qsort, literal: its order of equal keys is not stable, and the AI depends on it);
  - `mstack.c` (the move stacks and snapshots);
  - `route.c` (ship routes over N.RL).
- `loop.c`: the game loop 0x54824 with all 21 phase cases, the restored game's set-up 0x14AA4, the end-of-round and
  end-of-game saves, the elimination check and the end screens, the AI's score matrix 0x7C960.
  - Each phase routine not ported yet stops the port with its address. `--play` runs a restored or new game into
    the loop; `not_ported` then dumps the data object and frame (`--dump-ds`, `--headless-frame`).
- **Check (V):** R4 restored in both:
  - at the loop's entry (port `--stop-at-loop`, original BP 0x54824): **0 unexplained bytes**;
  - at phase 4's first routine 0x60AE0 (port `--play`, original BP 0x549D5): **0 unexplained bytes**, after the
    score matrix, the round label and the phase music.

**M6, the first phases (2026-11-19): TAX, POP, CENSUS, SHIP.** Translated in one workflow, a reviewer each, with
unit tests against independent models (tax 18,925 checks, and deliberately broken copies caught; pop, census, ship
likewise). The computers' parts (M7) stop with `not_ported`.
- `tax.c` TAXATION 0x66050 with the human Coinage screen, tax revolts and the human's city removal; `pop.c`
  POPULATION EXPANSION 0x673A8 with the manual screen 0x67758; `census.c` CENSUS 0x13D44 (Watcom's qsort, its
  comparator); `ship.c` SHIP MAINTENANCE 0x60BD8 and CONSTRUCTION 0x61940 with their human screens.
- **BUG(1995) fixes (`--original` keeps each):** "Democrary" (text); a tax revolt highlights the wrong nation's
  player button; removing the last city in a revolt writes into a heap pointer and shows "PIRACY" as the territory;
  POP's DISTRIBUTE ALL can loop for ever; maintaining a ship on a second coast by levy leaves the land territory
  marked; two latent errors in the computers' extra ships.
- **Check (V): round 1 of a new 3-human game, played by the port itself from the title menu** (`--play`; the same
  setup, seed and token clicks in DOSBox-X, `tools/dosbox/run_start.sh` with `STOP=0x54A51`):
  - TAXATION, POPULATION EXPANSION, CENSUS, SHIP MAINTENANCE, SHIP CONSTRUCTION run on their own;
  - the port's autosave "AutoSaved after SHIP CONSTRUCTION - R1" is **byte-identical** to the original's
    (44,062 bytes, the CRC included);
  - the data object at the call of the movement phase: **0 unexplained bytes**.
  - The R4 game (seven computers) stops at the computers' ship planning 0x62840 (M7).
- **All-human saves** (Mistral's `tools/verify/hsave.py`): the R4 autosave with every computer turned human is
  accepted by the original (Iberia gets the human ship-construction screen). The port restores it (slot 1 in the run
  copy, savgame1.dat) and with `--play --stop-at-wait` stops at the same wait: whole frame **0 px**, data object **0
  unexplained bytes**. This is how the human side of every later phase gets checked before the AI is ported.

Pace: one module at a time, each with its own commit; ultracode workflows for the large translation phases
(M4, M6, M7), split by code region with an adversarial check of each translated routine.

## 6. Easter eggs, jokes and oddities so far

1. **Hidden switch `/C[n]`: "Continuous testing has been enabled."** It asks "Append to ctest.txt? (y/n)" and
   accepts a number checked against a range ("Invalid argument %s. Must be in range 0-%d."). Not in `/?` or
   the README. G: the developers' AI-vs-AI soak test with a log (parser at 0x10378).
2. The EXE calls itself **"Version 1.11 Beta 1"**; the README documents 1.1.
3. Piracy with no room left: *"The management of Skull & Crossbones, Inc. regret to inform you that they are
   unable to add new cities to their portfolio."*
4. The **Barbarian Ravaging and Plundering Committee** (BRPC): *"The BRPC is deadlocked. They decide to settle
   in %s."*, and *"…has been wiped out by the Barbarians. Satisfied with a job well done they settle in %s."*
5. *"Zeus takes pity upon %s and discards …"* with its own Zeus picture, animation and sound.
6. **Two stills the game never shows.** `ACIV.CT1`'s second directory block lists a still **Avalon Hill logo**
   (sphere on a star field) and the **box-art title screen** as 640x480 GIFs (`AH_LOGO.GIF`, `ADVCIVIL.GIF`). The
   game plays `ah_logo.flc` and `advcivil.flc` instead and never names the GIFs.
   - Corrected 2026-11-18: an earlier note called these "past the end of the catalog". They are in the directory,
     in a second block the first reader missed.
   - The same block holds three nation cards the game does use, and the fonts.
7. Developer leftovers in the strings: memory statistics ("Memory Headroom: %s", "Max Memory used including
   .map analysis"), "junk" / "junk2" placeholders, "Move Stack has popped once to often."
8. README humour (not in the game): the "lucky ones whose machine confounds the detection code", and the PBEM
   rename error that says *"refer to the manual"* while "of course there is no such reference in the manual".
9. **The credits** (after the intro): Don Greenwood, the board game's developer and the game's producer, is
   credited for **"AI Design Nemesis"**, right after Jim Synoski's "Program and AI Design"; and the last card
   reads **"Original Boardgame Design (Thank You!)"** for Francis Tresham and Bruce Harper.
10. A developer's typo kept as an internal name: the ship-building routine tags its memory "DoShipContruction"
    (never shown to the player).

11. `KABOOM3.WAV` still carries its sound editor's signature: a `LIST/INFO/ISFT` chunk "GoldWave v2.10 (C) Chris Craig". `BRPC.WAV` has empty `alia` and `mark` chunks from another editor. A trace of how the sounds were made, not a joke.

12. **The computer players' opening maps carry their makers' notes.** The 66 `.BPM` files in `ACIV.CT3` (523
    bytes each, one per nation and player-count variant, e.g. `AFRC30`, `ILLY42`) start with a text label:
    - "Base map by Jim - 7/29/95" and "Base map by Jim - 8/2": Jim Synoski, the programmer and AI designer, dating
      his work two months before release.
    - Matchups: "Italy vs Africa vs Crete", "Thrace vs Africa vs Iberia/Italy", "illy vs italy vs thrc".
    - "non-iberia", "Alt base map".
    The values that follow (G) are per-territory priorities for where to expand, 0xFF = not used.

13. **Zeus borrows the iconoclasts' sound and picture.** `ZEUS.WAV` and `ICONCLSM.WAV` (Iconoclasm and Heresy) hold
    the same sample data, 132,352 samples at 22,050 Hz, and `ZEUS.FLC` is byte-identical to `ICONCLSM.FLC` (329,936
    bytes): Zeus's pity looks and sounds exactly like the temples being torn down.

14. **The opening maps keep their editing history.** The map editor wrote each `.BPM` label into a 41-byte buffer
    without clearing it, so older, longer labels show through after the NUL:
    - ASSY71 and ASSY81 read "vs Asia", then **"...edit this!!!"**: a reminder to the developer that nobody acted
      on, shipped in the game. ILLY30 keeps "!!" and "layers" from the same default text.
    - The "Base map by Jim - 8/2" labels were typed over "7/29/95", so "/95" still shows behind them.
    - ILLY32 was once "illy vs italy vs thrc or afrc"; AFRC40 ("non-iberia") was once an Italy-vs-Illyria map.
    - 8 of the 66 opening maps are never used: IBER60/70/80 (Jim's base maps; the IBER61/71/81 variants are
      used instead), ILLY30, ITAL31, THRC30, THRC72, THRC73.
    - Two labels don't fit their use (G, renaming during development): ILLY32 "illy vs italy vs thrc" is read
      only when Thrace is *not* in play, and ITAL32 "Italy vs Africa vs Thrace" also serves Italy + Illyria +
      Thrace, the matchup of the unused ITAL31.
    - A dead developer switch: per-nation opening-map overrides (names at 0xF6064, flags at 0xF60E8) are still
      honoured by four routines, but main clears them and nothing sets them. With the loose-file fallback when a
      name is missing from ACIV.CT3, it looks like the developers' way to try edited maps without rebuilding the
      catalogue.
15. Clicking outside your own starting territories rings the PC speaker bell (`printf("\a")`).
16. **Turning the volume up erupts a volcano.** Each click on the sound-effects volume arrows (AUDIO/VISUALS) plays
    `volcano.wav` as the test sound.
17. **The help texts tell stories.** Trading is "a crowded bazaar before the first millennium" where you shout "I
    have Salt and need Grain. Who has Grain for Salt?"; making offers is "baiting several hooks at once. Remember,
    you are only allowed to land the first fish to bite - not necessarily the biggest"; accepting opens "The moment
    of truth has arrived." The PBEM-only help VIEWMOVE sells blind simultaneous moves as "calculated gambles".
18. **A help page the game never shows**: CITYSUP.HLP (City Support), because of the copied help case (bug above).
19. **A dead "show all hands" switch.** int16 [0xF40C2] is never set (BSS, 0), but Goods Held, three trade screens
    and the save-version check still test it: set, Goods Held would show every nation's hand face up with its
    points. G: a developers' debug flag, like the opening-map override of egg 14.
20. Two identical copies of the PBEM line "This data is invalid and will remain so until the next PBEM
    consolidation" are printed on top of each other in Civilization Tools, then painted over: never visible.
21. A colour-blind accommodation (G): the toggle "Alternate Tool Acquisition Colors" makes buyable advances blue
    instead of green.

Text slips found in passing (to fix by default, `--original` keeps them): "converts %s by **Montheism**!" (twice),
"popped once **to** often". A full text sweep is part of M4/M6.

### Lanes (proposal, if Mistral joins)

Mistral gets self-contained tasks with a written spec and a check that runs without DOSBox; Claude keeps
everything that needs the disassembly or DOSBox-X runs, and verifies Mistral's work before it lands (the
Centurion `docs/HANDOFF.md` protocol, with a `docs/HANDOFF.md` here).

| Mistral (spec'd, standard formats, no EXE knowledge needed) | Claude |
|---|---|
| C catalog reader (`ACIV.CTn`, spec in §4) | M0 tooling, the disassembly, DOSBox-X captures |
| C GIF87a decoder into an 8-bit buffer + palette (check: all 64 catalog GIFs + P2-P4 decode; pixel diff vs a reference decoder) | Fastgraph subset (which calls, exact pixel rules) |
| C FLC player (check: frame count / CRC per frame vs a reference) | UI kit, map, save layout, phases, AI |
| C XMIDI parser → timed MIDI event list (check: event dump vs a reference XMI→MID conversion) | the sound synth choice and the AIL timing |
| WAV loader + mixer in the platform layer | checking every Mistral task against the original |
| Text sweep: list every game string with spelling/grammar slips for Erik's rulings | which slips are bugs (by the game's own texts) |

## 7. Questions for Erik

1. **GitHub**: one repo for the port, which Mistral can share (e.g. `Fulvia40bc/AdvCiv`, private)? Nothing is
   pushed yet. If Mistral joins, the lanes above apply.
2. **Music**: RULED 2026-11-18. All three sets, chosen in the port by a setting (the original chose by sound
   driver, see §2).
   - Choices: FM (`f*`, OPL2), stereo FM (`s*`, OPL3 as SB Pro 2), General MIDI (`g*`, Windows' built-in synth).
   - Default: stereo FM, the set Erik's setup played.
   - The choice is saved in the port's own settings file, not in `aciv.cfg`, so `aciv.cfg` stays as the original
     writes it.
   - The setting appears as one extra line next to Music on the game's own Preferences screen (Erik's "little
     setsound layer"), plus a command-line switch.
   - General MIDI plays through a synth built into the port, not Windows' own; the player never has to set up a synth.
     - Synth: **FluidSynth** (LGPL-2.1; linked as a DLL to keep within the LGPL).
       - It has reverb and chorus: the `g*` songs set both sends (controllers 91, 93). They also use GS bank select
         (0/32), NRPN (98/99/6) and controller 70 (D, the event dumps).
       - BUILT 2026-11-18: FluidSynth 2.6.2 (no GLib since 2.5, osal cpp11, every audio/MIDI driver off) as
         `libfluidsynth-3.dll` for ARM64 and x64, by `tools/build_deps.ps1`. It needs llvm-mingw's `libc++.dll`
         and `libunwind.dll` next to it.
       - `port/src/gm_synth.c` connects it to the sequencer: GS bank select, gain 0.6 (the loudest song, GCIVIL2,
         peaks at -3.4 dBFS).
       - `port/test/gm_render.c` renders a song to WAV, about 100x real time on the Surface.
       - ARM64 and x64 renders differ by at most 1 LSB (float rounding), so tests compare MIDI events, not
         audio.
     - SoundFont: **GeneralUser GS** 2.0.x (S. Christian Collins). Its license allows use in software projects,
       private or commercial, and modifying the bank or its packaging. It needs a synth with full SoundFont 2.01
       modulators; FluidSynth has them, TinySoundFont doesn't fully.
       - Not in git: `tools/build_deps.ps1` fetches it at a pinned commit and SHA-256.
       - A player's own `.sf2` in the game folder takes precedence.
   - Order: General MIDI first (sequencer = Mistral Task 7, FluidSynth glue = Claude), then FM. The FM part is one
     driver logic for `f*` (OPL2) and `s*` (OPL3 stereo).
   - (D) Every song has one infinite loop:
     - FOR (controller 116, count 0 = forever) → NEXT (117, value 127).
     - Civil: intro 0-6564 ticks (55 s), loop 6564-14645 (67 s). Calamity: loops from tick 0.
     - One callback trigger (119) just before the NEXT; what the game does with it: G, to decode.
     - SCIVIL2 has 4 notes still sounding at the NEXT. A sequencer must keep their note-offs across the jump.
3. **Platform layer**: RULED 2026-11-18: SDL, because the port must run on Intel machines as well as ARM, and battery
   use doesn't matter for this game.
   - SDL3: SDL2, Centurion's, is in maintenance.
   - Built from source for ARM64 so the Surface runs it natively. Centurion's SDL2 build is x64 under emulation
     there, because the SDL2 mingw package has no ARM64 libraries.
4. **Running the original**: verification needs DOSBox-X runs of the game in a window. May I use the screen for
   those, and when?
5. **PBEM**: RULED 2026-11-18: dropped. Erik plays hot seat (several human players at one PC), and hot seat stays.
   - The new-game screen keeps "Number of Human Players" and "Number of Computer Players".
   - The "Play-by-email" choice and the PBEM menus are left out, and so is pkzip.
   - The Military advance's text says "PBEM only: removes one opposing unit before normal conflict resolution"
     (data 0xB8063). RULED 2026-11-18: the sentence and the rule are left out (`--original` keeps the text).
