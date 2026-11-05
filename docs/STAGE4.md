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

## Stage 5c: EXE disassembly from true entry (CS:IP 0x0A78:0x0322 -> linear 0xAAC2)
- `disasm/CENTURIO.funcs3.asm`: 121 functions recovered by recursive descent
  from the real entry point (vs 44 from call-target seeding).
- Located: filename buffer at 0xB060 ("CENTURIO.EXE"/"OVL0.OVL" strings at
  0xB059/0xB065), file-read via int 0x21 AH=3Fh around 0x02C5, LZSS unpacker
  at 0x01DE-0x0287, overlay-manager strings at 0xAEE5 ("Overlay Loader...").
- Load-base question still open: no constant file->memory shift fits the OVL
  dispatch table; needs the overlay manager's header parse read directly.
  Next: find the code referencing 0xAEE5 (misaligned in current listing) and
  read the OVL header handling.

## Stage 5d: overlay dispatch mechanism pinned
- EXE hooks **int 0x64** as the overlay-call primitive: handler at ~0x0000/0xAD8D
  does `lcall [0xc4]` (far pointer = overlay segment:entry, set at load time).
  EXE code calls overlays via `int 0x64` with AH = function index.
- **Load base = 0 (file offset == memory offset)**: OVL0.OVL offset 0 is a clean
  `push bp; mov bp,sp` prologue, and dispatch-table values are all < file size.
  No constant shift fits the table in either direction (<=9/600 prologue hits).
  The earlier "call 0xe37c lands in data" anomaly is a disassembly-alignment
  artifact, not a base offset.
- Asset-load chain in overlay: func_0x93c0 opens a member, and when global
  [0x9b26]==2 passes VRAM seg [0xc1de]=0xA000 to the decode/blit at 0xe37c.
  Next: disassemble 0xe37c from a properly aligned seed to read the CBM decoder.

## Stage 6: runtime asset pipeline in the port (C)
- `port/src/assets.c` + `assets.h`: C implementation of the .DIR/.DAT container
  parse and the LZSS decoder, wired into `game_init` as a smoke test.
- Verified: C decoder output byte-identical to the Python reference decoder
  (`cmp` clean on newgame.lzw, 1791 bytes). Whole port syntax-checks with
  `gcc -Wall -Wextra`.
- Ghidra 12.2 headless (JDK 25) was set up for 16-bit real-mode decompilation;
  its segmented x86-16 output is usable but the capstone recursive disasm
  (121 EXE / 4503 OVL functions) remains the primary reference.

## Stage 6b: overlay alignment investigation
- Prologue-seeded (55 8b ec) recursive disasm: `disasm/OVL0.prologue.asm`
  (3,324 funcs), `disasm/CENTURIO.prologue.asm` (123 funcs, EXE is PLINK-linked).
- EXE func 0x01EA confirmed as the LZSS unpacker (window 0xB060, prefill 0x2020,
  wpos 0xFEE, mask 0xFFF) — matches the Python/C decoders.
- Overlay image-load call sites: func_0x05B4 -> call 0x5E2C and func_0x93C0 ->
  call 0xE37C, both passing VRAM seg [0xC1DE]=0xA000 when [0x9B26]==2.
  Both targets disassemble as garbage at H=0 and no constant header shift H
  fixes them -> the callers' apparent alignment is coincidental; true overlay
  function boundaries still unresolved. Next: read the EXE's AH=3D open caller
  (overlay manager) to get the real OVL entry offset.

## Stage 7: Ghidra analysis pass for the [0xC4] writer (result: inconclusive)
- Full Ghidra 12.2 headless analysis (JDK 25) on a reconstructed MZ of the
  unpacked EXE, entry marked at 0xAAC2. Scripts in tools/ghidra/.
- Ghidra's x86-16 real-mode analyzer does NOT create data references for the
  DS-relative accesses (`lcall [0xC4]` with DS=0xE63, string refs), so
  reference search found nothing. Byte-level scans of the EXE found no direct
  store to [0xC4]/[0xC6] -> the overlay manager writes the dispatch far pointer
  via a register-indirect store in a region that misaligns linear sweeps.
- Dispatch mechanism is nonetheless confirmed: int 0x64 handler does
  `lcall [0xC4]` with AH = overlay function index (0=idle, 3/4 from timer).

## Stage 8: runtime ground truth (DOSBox-X debugger)
- `bpint 64` caught the game at an `int 0x64`; with DS=1686, the dispatch far
  pointer `[0xC4]` = **3682:0000** (bytes `00 00 82 36` at 1686:00C4).
- => OVL0.OVL is loaded at a runtime segment (0x3682 in that session) with
  entry offset 0x0000, and the int 0x64 handler enters the overlay at OVL
  offset 0 with AH = function index.

## Stage 9: runtime overlay structure (DOSBox-X)
- At the entry breakpoint, CS=3682; the dispatcher at 3682:03E8 does
  `mov bl,ah; shl bl,1; call near [bx+03DE]` -> AH indexes a word table
  at overlay offset 0x03DE (near-call offsets). Jump stubs (E9 rel16) at
  3682:0003+ (jmp 0403/0470...).
- Runtime bytes at 3682:0003 (E9 FD 03...) do NOT appear in OVL0.OVL on
  disk -> the in-memory overlay image differs from the file (transformed or
  relocated at load). Static analysis must use a runtime memory dump.
- Next: MEMDUMPBIN 3682:0 <len> ovl.bin in DOSBox-X, analyze that image.
