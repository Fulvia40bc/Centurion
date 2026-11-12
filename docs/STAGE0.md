# Stage 0 Report — Centurion: Defender of Rome (DOS, 1991) reverse-engineering recon

## Input artifacts (from repo root)
- `CENTURIO.EXE` — 32,413 bytes, MS-DOS MZ executable, **LZEXE v0.90** packed (signature `LZ09` at offset 0x1C)
- `OVL0.OVL` — 96,272 bytes, overlay file (bulk of game code lives here)
- `*.DAT` / `*.DIR` — game data (CENT, BATL, MGLAD, MINIT, MMAP, MNAVAL, MNEGO, MRACE, NAVL)
- `0.CSV`–`2.CSV` — binary data tables (despite extension)

## Unpacking
- Tool: `tools/unlz09.py` (LZEXE 0.90 unpacker, adapted from samrussell/unpacklzexe)
- Output: `tools/CENTURIO.unpacked.bin` — **45,296-byte (0xB0F0) 16-bit real-mode load image**
- Validation: clean strings — `Copyright (C) 1984, 1985, 1986 by Phoenix Software Associates Ltd.`, `Overlay Loader: stack underflow`, `dos 2.0 required`, `PLINK` (linked with PLINK, Phoenix Software Associates' own linker)

## EXE structure (unpacked)
- MZ header: entry **CS:IP = 0x0A78:0x0322** (linear 0xAAC2), SS:SP = 0x1AA4:0x0800
- 16-bit real-mode x86 code, single load image ~45 KB; most game logic is in `OVL0.OVL` (96 KB overlay, loaded at runtime by the "Overlay Loader")
- Full linear disassembly: `disasm/CENTURIO.asm` (objdump `-b binary -m i8086`, 20,303 lines)

## Platform touchpoints found in main EXE
| Category | Sites | Notes |
|---|---|---|
| BIOS video `int 0x10` | 7 | mode set, palette, text/graphics output |
| DOS `int 0x21` | 7 | file I/O (loads .DAT/.OVL), AH=09h print |
| Mouse `int 0x33` | 2 | init / read status |
| Keyboard `int 0x16` | 2 | key polling |
| Indirect port I/O via DX | 292 | VGA/PIC/timer/Sound Blaster-style register access (`in (%dx)` / `out (%dx)`) |

## Implications for the port
- This is **decompilation + reimplementation**, not a source port — no C/ASM source exists.
- 16-bit real-mode, PLINK-linked, overlay-based: the port must be built **module by module**, starting with the main EXE's shell (loader, main loop, video init) and then `OVL0.OVL`.
- Replacement strategy: BIOS/DOS interrupts → SDL2/Win32 backend; direct VGA port writes → SDL2 renderer; fixed-timestep loop replaces DOS tick timing; `.DAT`/`.DIR` files reused as-is.
- Behavioral reference: original under DOSBox.

## Suggested next stages
1. Disassemble + annotate `OVL0.OVL`; build a function map of both binaries.
2. Identify main loop, video mode (likely VGA 320x200x256 or MCGA), audio (AdLib/Sound Blaster/PC speaker), input.
3. Scaffold `port/` with an SDL2 backend (window, framebuffer, input, timing) and a stub main that loads data files.
