# Stage 4 Report — the ".LZW" compression cracked (it's LZSS)

## Finding
Despite the `.LZW` extension, the game's compression is **LZSS**, matching the
`unlzss.exe` the original ships with. Located the unpacker in the unpacked
`CENTURIO.EXE` at linear `0x01DE–0x0287` (called after `int 0x21` file reads).

## Format (reverse-engineered from the disassembly + validated on assets)
- 4096-byte circular window, prefilled with `0x20`, write position starts at `0xFEE`
- Stream: flag byte, then 8 items; flags LSB-first; flag `1` = literal byte,
  flag `0` = match
- Match = 2 bytes: `b1 b2` → window position = `(b1 | ((b2 & 0xF0) << 4)) & 0xFFF`,
  length = `(b2 & 0x0F) + 3` (3–18)
- Both literals and match output are written into the window (standard LZSS)

## Validation
- `tools/extract_data.py` now decodes every `0x80` member; all 130 `.LZW` files
  across the 9 archives decompress to structured data
- Text content recovered verbatim, e.g. `CENT__newgame.lzw` → `"10A.New Game..."`
- `CENT__adlib.lzw` → 2,430 bytes of clean AdLib register-write streams
- `NAVL__galleys.lzw` → 64 KB (galley battle graphics data)

## Updated tools
- `tools/extract_data.py` — full pipeline: `.DIR` parse → `.DAT` slice →
  LZSS decode (`0x80`) / passthrough (`0x00`, `0xC0`)
- `tools/cbm2png.py` — best-effort `.CBM` → PNG renderer (RLE scheme still
  being confirmed; see STAGE3)

## CBM images — status update
`.CBM` = 4-byte header (`00 00 W H`, e.g. 160x200) + an LZSS-like stream using the
same 4 KB window scheme; decodes to ~W*H pixels but pixel-exact layout not yet
confirmed (best smoothness 0.40 over parameter sweep — likely an additional
transform). Rigorous next step: read the actual CBM loader in `disasm/OVL0.asm`.
