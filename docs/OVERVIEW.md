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

13. **Zeus borrows the iconoclasts' sound.** `ZEUS.WAV` and `ICONCLSM.WAV` (Iconoclasm and Heresy) hold the same
    sample data, 132,352 samples at 22,050 Hz: Zeus's pity sounds exactly like the temples being torn down.

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
