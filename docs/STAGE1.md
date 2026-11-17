# Stage 1 Report — OVL0.OVL disassembly + function map

## OVL0.OVL
- 96,272 bytes, raw 16-bit real-mode code (starts directly with a function prologue `push bp; mov bp,sp` — no file header; loaded by the EXE's overlay manager)
- Disassembly: `disasm/OVL0.asm` (objdump `-m i8086`, 37,818 lines)
- **Toolchain: Microsoft C run-time** — string `MS Run-Time Library - Copyright (c) 1990, Microsoft Corp` (the main EXE is PLINK-linked; overlay is MSC-compiled)
- References bitmap assets: `title.cbm`, `map.cbm`, `world.cbm`, `bb1.cbm`, `bb2.cbm`, `battlenw.cbm`, `arenabtm.cbm`, `raceest.cbm`, `circus.cbm`, `coloseum.cbm`, `galleys.cbm`, `gallery1.cbm`, `desert.cbm`, `water.cbm`, `sky0.cbm`, `leaderA.cbm`, `negbgA.cbm`, `negfgA.cbm`, `rome.cbm`, `romulus.cbm` — plus `ovl0.ovl` self-reference and `.DAT`/`.DIR`
- Plus game text: "Rebellion in ... meets the rebel army on the field of battle.", "Really Quit?", "Sic Transit Gloria Mundi", etc.

## Platform touchpoints (combined)
| Category | Where | Notes |
|---|---|---|
| Video `int 0x10` | 7 sites in EXE | AH=0Fh (get mode), AH=00/AL=03 (text 80x25), AH=12h, AH=10h AL=02 (cursor); graphics likely via direct port writes in OVL |
| DOS `int 0x21` | 7 sites in EXE | file open/read of .DAT/.OVL/.CBM, AH=09h print |
| Mouse `int 0x33` | 2 sites in EXE | init + status |
| Keyboard `int 0x16` | 2 sites in EXE | polling |
| Indirect port I/O via DX | 292 (EXE) + 1,812 (OVL) = ~2,100 | VGA sequencer/CRTC/GC/DC register programming → renderer is direct-hardware in OVL |
| PC speaker / PIT | ports 0x40–0x43, 0x61 in both | square-wave audio via PIT channel 2 |
| Sound Blaster | port 0x220 (1 ref in OVL) | optional digital audio |
| No AdLib (0x388/0x389), no int 0x1A | — | timing via PIT ports, not BIOS tick |

## Function map (hot call targets)
- EXE (`disasm/CENTURIO.asm`, 745 call sites): `0x01d8a` x13, `0x035db` x9, `0x03f23`/`0x01e21` x7 — candidate core: overlay-call dispatcher, file loader, main logic
- OVL (`disasm/OVL0.asm`, 1,347 call sites): `0x03eb2` x45, `0x0a82b` x35, `0x0dbdb` x23, `0x02f22` x21 — candidate blit/draw/memory helpers (renderer core)

## Implications
- Renderer = direct VGA register programming in the overlay → port strategy: replace with SDL2 software framebuffer (mode 13h-style 320x200x256 palette) rather than trying to emulate register writes
- Audio = PIT-driven PC speaker (+ optional SB at 0x220) → SDL2 audio with square-wave synthesis
- Input = BIOS int 0x16/0x33 → SDL2 keyboard/mouse
- Timing = PIT port reads → fixed-timestep loop (e.g., 60 Hz)
