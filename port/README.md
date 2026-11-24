# Centurion: Defender of Rome — native port (work in progress)

Reimplementation of the 1991 DOS game (reverse-engineered from
`CENTURIO.EXE` / `OVL0.OVL`, see `docs/STAGE0.md` and `docs/STAGE1.md`)
running natively on Windows/macOS/Linux via SDL2 — no DOS emulator needed.

## Build
```
# install SDL2 dev package, e.g.:
#   Debian/Ubuntu: sudo apt install libsdl2-dev
#   Windows (vcpkg): vcpkg install sdl2
#   macOS: brew install sdl2
make
./centurion
```

## Architecture
- `src/platform.h` — abstraction layer over video / audio / input / timing
  (replaces the original's direct mode-13h VRAM framebuffer, BIOS/DOS
  interrupts, PIT speaker and DOS tick timing)
- `src/sdl2_platform.c` — SDL2 backend: 320x200x256 window (2x scaled), RGB palette
  texture, keyboard/mouse, square-wave audio, fixed-timestep friendly timer
- `src/game.c` — game logic (currently a stub; will be filled in module by module
  from the annotated disassembly in `disasm/`)
- `src/main.c` — entry point, fixed-timestep loop at 60 FPS

## Status
Images (82/82 CBMs), text, assets, and UI primitives work; game logic
decompile in progress. See PR #1 and docs/STAGE4.md.
