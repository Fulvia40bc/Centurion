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

## CBM follow-up (sweep results)
Exhaustive parameter sweep (header offset 0-8, flag polarity, bit order, window
init 0xFEE/0, prefill 0x20/0x00, length base 2/3, absolute/relative position,
delta/XOR/bit transforms) never exceeds ~0.42 neighbor-smoothness on decoded
pixels -> the .CBM payload is NOT the EXE's LZSS. The overlay contains its own
image decoder. Next step: recursive-descent disassembly of OVL0.OVL from its entry
table to locate the image loader instead of grepping the linear sweep.

## Stage 5 tooling: recursive-descent disassembler
- `tools/rdisasm.py` (Capstone, 16-bit mode): seeds from offset 0 + all direct
  call/jump targets, follows control flow recursively.
  - `disasm/OVL0.funcs.asm`: 3,318 recovered functions in the overlay
  - `disasm/CENTURIO.funcs.asm`: 44 functions in the main EXE
- Located the asset-load path: `func_0x93c0` in the overlay opens a file
  (via 0xe3eb) and, when global [0x9b26]==2, passes the VRAM segment
  (global [0xc1de] = 0xa000) to the decode/blit routine at 0xe37c.
- The 0xe37c region currently disassembles as garbage under linear-sweep
  alignment — needs real function-boundary recovery seeded from the EXE's
  overlay dispatch table (next step).

## Stage 5b: overlay entry seeds + refined disassembly
- Extracted ~3,094 candidate entry offsets from the overlay's dispatch/jump
  table region (0xee00-0x10800) -> `disasm/OVL0.entry_seeds.txt`
- `tools/rdisasm.py` now accepts a seed file (argv[3]); re-ran:
  `disasm/OVL0.funcs2.asm` = 4,503 functions (up from 3,318)
- Open question: a `call 0xe37c` from the asset-load func (0x93c0) lands in a
  data-looking region at file 0xe370+, so either the overlay file has a
  load-base offset vs. memory, or that call site is misaligned. Next: read the
  EXE's overlay manager (string "OVL0.OVL" in EXE) to get the true load base,
  then re-align and read the image loader.
