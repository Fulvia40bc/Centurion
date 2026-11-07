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

## Stage 10: runtime overlay image captured and disassembled
- `disasm/OVL_RUNTIME.BIN` (98,304 bytes, from DOSBox-X MEMDUMPBIN 3682:0):
  starts with the jmp-stub table (E9 E5 03 -> 03E8, E9 FD 03 -> 0403, ...),
  matching the debugger exactly. The on-disk OVL0.OVL is a packed/encoded
  form of this; all further analysis uses the runtime image.
- Dispatcher at 0x03E8: `mov bl,ah; shl bl,1; call near [bx+0x03DE]; retf`.
  AH table at 0x03DE: AH0->0x4A9 (command-stream interpreter), AH1->0x403,
  AH2->0x470, AH3->0x483, AH4->0x47B.
- `disasm/OVL_RUNTIME.asm`: recursive disasm of the runtime image (70 funcs
  from entry seeds). VRAM segment is passed in from the EXE (global), not
  hardcoded in the overlay.

## Stage 11: runtime overlay analyzed
- `disasm/OVL_RUNTIME.BIN` = authoritative overlay image (from DOSBox-X dump).
- `disasm/OVL_RUNTIME.funcs.asm` (recursive disasm from the 5 AH entries,
  70 functions) and `disasm/OVL_RUNTIME.linear.asm` (full linear sweep).
- AH=0 handler at 0x4A9 is a byte-command interpreter (reads script via es:si);
  image display is one of its commands -> the CBM blit is reached through it.

## Stage 12: overlay command interpreter (runtime image, AH=0 handler at 0x4A9)
- Reads a byte stream via es:si; command = high nibble of byte (al & 0xF0):
  - 0xE0 -> reads 16-bit resource id into [0x3D5], calls 0x7A3 per matching
    slot (0x7A3 programs AdLib regs via ports 0x388/0x389 -> music command)
  - 0x90 -> 0x5E2 (slot activate), 0x80 -> 0x6AB (slot deactivate),
    0xC0 -> end, 0xFC -> 0x5CC (return/reset), else skip 2 bytes
  - 9-slot state table at 0x35C (slot id at +0x35C, active flag +0x365, ...)
- Image display is one of the command handlers calling into the blit routine.

## Stage 13: AH dispatch table fully enumerated - all 5 entries are the AdLib sound engine
- AH=0 -> 0x4A9 (music command interpreter), AH=1 -> 0x403 (init script),
  AH=2 -> 0x470 (stop), AH=3 -> 0x483 (silence 9 channels, ports 0x388/0x389),
  AH=4 -> 0x47B (enable). Table entries beyond AH=4 are code bytes, not entries.
- => Image/game logic is NOT reached via int 0x64 AH dispatch; it is called
  from the EXE through other far pointers. Next: find the image blit by
  watching VRAM (es: writes) at runtime, or follow EXE->overlay calls.

## Stage 14: EXE overlay far-call sites enumerated
- EXE calls into the overlay via far-pointer globals (lcall [disp16]):
  [0xC4] (int 0x64 sound dispatch), plus [0xB6] @0x07FC, [0xB8] @0x0800,
  [0x684] @0x1C5A/0x1D00, [0x1E06] @0x1D04, [0xF46] @0x2057, [0xF40] @0x2123,
  [0xF3E] @0x2151, [0x15DA] @0x33EB, [0xA06] @0x3F40, [0x8EC] @0x6332.
- Each global is set to an overlay far pointer at load; the image-display
  entry is one of these. EXE regions around these sites misalign in linear
  sweeps, so mapping global->overlay function is fastest at runtime
  (break at each lcall, read the pointer, subtract overlay base segment).

## Stage 15: runtime data-segment dump analysis (ds_dump.bin)
- Only ONE overlay far pointer exists in the EXE data segment: ds:0xC4 =
  3682:0000. The other scanned "lcall [disp]" byte hits were mid-instruction
  false positives (stricter scan with segment-override prefixes confirms only
  [ds:0xC4] and one [ds:0xA06] are real, and 0xA06 is text).
- => ALL EXE->overlay calls go through [0xC4] (int 0x64). The AH table at
  overlay 0x03DE has 256 entries (0x3DE..0x5DE); AH 0-4 are AdLib sound, the
  rest are game functions (176 point at non-text code).
- `disasm/OVL_RUNTIME.ah.asm`: recursive disasm seeded from all 256 AH
  entries (558 functions). 300 functions write via es:[di] (VRAM blits among
  them). Next: identify which AH entry is the image-display blit.

## Stage 16 (SUPERSEDED by Stage 16-corrected note above; see below): graphics ARE direct VRAM writes
- CORRECTION: the earlier note claimed no 0xA000 exists in either binary.
  Wrong — the DOSBox-X register dumps show ES=A000 while the game draws, and
  the screen dump (screen_map.bin) is a coherent mode-13h framebuffer. The
  0xA000 scan missed it because the VRAM segment is loaded from a global /
  computed, never as a literal immediate. int 0x10 is used only for mode set /
  palette / cursor. Port strategy: emulate the direct-VRAM framebuffer
  (mode 13h, 320x200x256 at A000:0000), NOT a BIOS int 0x10 pixel layer.

## Stage 17: EXE int 0x10 handler map
- 0x8102: AH=0x12 BL=0x10 AL=0x82 (set DAC palette block)
- 0x8496: AH=0 AL=9 then AH=0 AL=3 (mode set: mode 9 -> text mode 3)
- 0x84a6: AH=0x0F (get video mode)
- 0x84b5: AH=0 mode set + vsync via port 0x3DA
- 0x970a: pixel/cursor write via AH from args (function misaligns in linear
  sweep; needs entry-seeded disasm)
- 0x9723: AH=0x10 AL=2 (set cursor shape/position)
- Conclusion: all video via BIOS int 0x10; port emulates int 0x10 into SDL2.

## Stage 18: decode-routine hunt status
- Static call-chasing into the LZSS unpacker (EXE 0x01DE-0x0287) fails: EXE
  regions misalign in linear sweeps, byte-scan call targets are false positives.
- Efficient path is runtime: in DOSBox-X, break at the unpacker return when a
  .CBM is loaded and dump the decoded buffer (MEMDUMPBIN), which gives the
  exact decoded image bytes -> then match against the extracted .CBM to
  derive the pixel format directly.

## Stage 19: CBM pixel-format sweep exhausted
- Full LZSS parameter sweep on map.cbm (header 0-8, winit 0xFEE/0, prefill
  0x20/0, flag polarity, bit order, length base): best run-metric 0.42
  (real images >0.6) -> the .CBM payload is NOT the EXE LZSS; the overlay has
  its own image decoder. Definitive fix: dump the DECODED buffer at runtime
  (break at unpacker ret 0x0287 while a .CBM loads, MEMDUMPBIN the output),
  then diff against the raw .CBM to derive the format.

## Stage 20: screen-dump alignment tooling ready
- No 768-byte 6-bit palette run inside any .CBM -> palette is the shared
  CMAP.BIN (750 bytes), not embedded per image.
- `tools/align_screen.py`: renders a 0xFA00 VGA screen dump (320x200 mode 13h)
  to PNG and extracts image-sized regions (y offsets 0/20/40 to skip UI) as
  raw pixels for diffing against the raw .CBM payload.
- Waiting on: tools/dosbox/results/screen_map.bin (+ .txt mode note) pushed
  via collect.bat, then run align_screen.py to line pixels up with map.cbm.

## Stage 21: ground-truth map pixels captured
- screen_map.bin (320x200 mode 13h) rendered: left 160x200 region (0,0) IS the
  map image (verified visually). Saved as tools/dosbox/results/map_decoded_160x200.raw
  = the exact decoded map.cbm pixels (known plaintext for the CBM codec).
- LZSS-with-known-window simulation against this plaintext: no match -> CBM
  codec is custom (not the EXE LZSS). Next: analyze payload vs plaintext
  structure directly (payload head 06 0a 00 7d 00 00 f9 88... vs 73x0x6f...).

## Stage 22: CBM codec known-plaintext analysis
- Known plaintext: map_decoded_160x200.raw (32000 px) vs map.cbm payload
  (23046 bytes). Tried: LZSS 8/16-bit flags x all polarities x window inits,
  XOR-key sweeps on payload and output, RLE pair parses. Literals never match
  raw plaintext (24/12828) -> values are transformed or the format is
  bit-packed/custom. No 0xfff window mask in the overlay's AH-reachable code.
- Conclusion: CBM codec is a custom bit-level scheme; fastest resolution is
  dumping the decoder's output buffer at runtime (break in overlay 3682 at
  the decode call, MEMDUMPBIN es:di after ret, use 0xFA00 length).

## Stage 23: first visible milestone - map on screen in SDL2 port
- CMAP.BIN = 750 bytes = 250 entries x 3 bytes of 6-bit VGA DAC values
  (only 1 byte >63); map to 8-bit with (v & 0x3F) << 2. Verified visually:
  blue sea, colored provinces, UI at top.
- port/src/game.c now loads cmap.bin + screen_map.bin (320x200 mode-13h
  framebuffer) and presents it via platform_present. Port syntax-checks clean.
- Next: swap in live map.cbm decode once the codec is read from the overlay.

## Stage 24: text/fonts + AdLib music data (status)
- Strings are plain NUL-terminated text in the data segment (confirmed in
  ds_dump: "Rebellion in ...", "Roman yoke!", etc.).
- No BIOS text calls (int 10 AH=09/0A/0E/13) in either binary -> text is drawn
  with a custom glyph blit to VRAM; font not yet located (no classic 8x8 'A'
  pattern in any asset or binary; likely 8x16 or custom). Next: find the
  string->glyph blit in the runtime overlay (not reachable from AH seeds).
- AdLib music: CENT.DAT members ADLIB.LZW (2430 B decoded), SPEAKER.LZW
  (742 B), TANDY.LZW (1711 B). Format: event stream of 0xE9 <reg> <val>
  AdLib register writes separated by 16-bit LE delay values -> feeds the
  9-channel slot player (overlay AH=0 interpreter at 0x4A9).

## Stage 25: extractor alignment bug fixed - CBM codec CRACKED
- BUG (fixed in tools/extract_data.py and port/src/assets.c): DIR offsets are
  ABSOLUTE file offsets (they include the 4-byte CA CA D0 D0 signature); the
  extractors wrongly added 4. All earlier CBM/codec analysis (Stages 19/22)
  used data shifted by 4 bytes and is invalid.
- .LZW members start with a uint32 uncompressed size, then the LZSS stream;
  decode from member+4 and truncate to the declared size (old code read 4
  bytes of the next member at the end).
- CMAP.BIN correctly starts 00 00 00 00 00 2A 00 2A... (standard EGA colors);
  250 clean 6-bit entries; map as (v << 2), no mask.
- CBM format (0xC0): 14-byte header [u32 width=320, u16 pad, u8 bytes_per_row=160,
  u8 rows=200, u16 ?, u32 unpacked_size=0x7D00] + LZSS stream -> 4-bit packed
  pixels (hi nibble first); screen palette index = nibble + 0x68 (16 colors).
  Verified: decoded map renders as a coherent map; C and Python decoders
  byte-identical (64000 bytes).
- port/src/game.c now live-decodes map.cbm from MMAP.DAT into the framebuffer.
- Runtime note from DOSBox-X: full screens are composed in an off-screen
  buffer at segment 2687 and copied to VRAM by the loop at 0823:95DB.

## Stage 26: CBM LUT applied in the C port
- assets.c: assets_decode_cbm now parses the 14-byte header properly and maps
  nibbles through the 16-entry LUT (header byte 9) loaded from OVL0.OVL
  offset 0x114C0 via assets_load_luts(). Verified byte-identical to the
  RAM-verified tools/cbm_decode.py (64000/64000). port/screen_map.bin removed;
  the port now live-decodes MAP.CBM from MMAP.DAT.
- Next: text rendering (glyph blit in overlay), then an any-image viewer.

## Stage 28: text rendering implemented in the port
- Font per docs/FONT_FORMAT.md (VERIFIED by Claude vs game RAM): OVL0.OVL file
  offset 0x1509C = width[96], yoff[96], glyph[96][8]; chars 0x20-0x7F, 8 rows,
  bit7 = leftmost, 1px spacing, ink-only. port: assets_load_font + draw_text;
  verified rendering "The people are Rebellious"; C glyph bytes match file.

## Stage 29: main-loop mapping (status)
- EXE entry CS:IP 0x0A78:0x0322 (linear 0xAAC2; clean code from 0xAAC3:
  mov ax,0x8EDA; neg ax; call 0xACFD = C startup).
- EXE int 0x64 call sites: 0x0024 (timer hook), 0x1C40, 0xADCC. Overlay
  dispatch = AH table at runtime-ovl 0x03DE; AH 0-4 = AdLib sound engine.
- Input (int 0x33 mouse) functions cluster at runtime-ovl 0x88F4-0x8A00.
- Frame path (VERIFIED by Claude): compose in work buffer seg 2687,
  blit to VRAM A000 by loop at EXE 0823:95DB. Rect fill 0823:7F96-80D4,
  cursor draw 0823:1BE6, mouse pos at DS:00B6/00B8.
- Remaining: identify the game-logic AH entry (the EXE main loop's int 0x64
  argument) and decompile that function from the runtime overlay.

## Stage 30: EXE ground truth = runtime image (corrects corrupt unpacker)
- tools/CENTURIO.unpacked.bin (unlz09.py) is CORRUPT (5.2% chunk match vs
  runtime; wrong LZ back-reference model). Stages 5c/14/29 derived from it are
  UNVERIFIED/invalid. Ground truth: tools/dosbox/results/exe_runtime_0823.bin
  (origin 0823:0000, fixups applied). NOTE: the .LZW asset decoder is a
  different (8-bit-flag) LZSS variant and IS correct (RAM-verified).
- EXE re-analysis now runs from exe_runtime_0823.bin.

## Stage 31: EXE re-analyzed from runtime image; main-loop architecture
- disasm/EXE_RUNTIME.bin/.asm = clean EXE (1,596 funcs from entry 0x322).
  Old CENTURIO.unpacked.bin + CENTURIO.asm removed (corrupt).
- int 0x64 appears ONLY in the timer ISR at 0x0000 (sound tick every 4th tick,
  AH=0..4 + lcall [0xC4]). The game loop lives in the OVERLAY, entered via
  [0xC4]; EXE hooks: int 0x08 timer (0x0000), int 0x09 keyboard ISR at 0x042F
  (scancodes, arrows 47h-51h, then lcall [0x9E] = overlay key callback).
- Video: int 0x10 AH=0x12 (palette, funcs 0x80D6/0x80F3), AH=0x0F (get mode).
- Overlay game calls from EXE: wrapper func_0x072E (lcall [0xC4] AH=1),
  called from 0x5D29/0x5D43.

## Stage 32: game-loop region located in EXE
- EXE main game code region ~0x49xx-0x62xx: func_0x49CE (menu/item loop:
  iterates si/di over a table, calls 0x22EA per item), func_0x62C2 (68 insns)
  calls the overlay wrapper 0x5D29, which dispatches overlay AH=1 via 0x072E.
- Architecture: EXE owns main loop + video/IO hooks (int 08/09 ISRs,
  int 0x10/0x21), overlay owns game logic/sound, entered via [0xC4] and
  keyboard callback [0x9E].
- Next: decompile func_0x62C2 and func_0x49CE region as the EXE main loop;
  the overlay game state machine is reached via AH dispatch table 0x03DE.

## Stage 33: frame blit + main-loop callees mapped (clean EXE)
- VRAM segment stored in global [0x9ABA] (=0xA000); blit helper func_0x689F
  (rep movsw 9 words, then call 0x494 with [0xC1DE] and [0x9ABA]).
- Screen-build func_0x62C2 (68 insns): sequence of UI calls (0x5AC2, 0x7CB2,
  0x5D29 overlay, 0x8E62 window, 0x7E74 rect, 0x5AE4) - a menu screen builder.
- Menu item loop func_0x49CE: iterates table si/di (di to 0x55C5), calls
  0x22EA per item, then 0x22E2/0x4A24/0x4AA6/0x2AF8/0x423A/0x53BC/0x680F/0x6973.
- Main loop: EXE-driven, overlay does game logic via AH dispatch.

## Stage 35: menu screen builder identified
- func_0x6589 (188 insns) = full menu screen builder: rect 0x7E74, window
  0x8E62, item list 0x950E, text 0x8BE4, buttons 0x6973/0x32E2, overlay call
  via 0x5D29. UI primitives: 0x7E74=rect fill, 0x8E62=window, 0x5AC2=begin,
  0x7CB2=panel, 0x5AE4=list, blit 0x0494/0x689F.
- Port now renders via offscreen work buffer -> blit (matches original).

## Stage 36: decompile tooling + UI map
- tools/pseudo.py + docs/DECOMP_UI.txt: per-function call/global maps for the
  UI primitives and screen builders (0x7E74 rect, 0x8E62 window, 0x8BE4 text,
  0x950E list, 0x6973 button, 0x0494 blit, 0x6589 menu screen, 0x62C2, 0x49CE).
- Overlay game-logic entry AH still needs one runtime read ([0xC4] call at
  game start) - handoff request filed previously.

## Stage 37: UI primitives in the port
- port/src/ui.c: ui_rect (0x7E74), ui_window (0x8E62), ui_text (font+0x8BE4),
  ui_blit (0x0494). Viewer draws a top bar + image-name label per image.

## Stage 38: handoff review corrections (Claude)
- SUPERSEDED: Stage 16's original "all video via BIOS int 0x10" claim and
  Stage 19/22 "nibble + 0x68" claim - see corrected Stage 16 note and
  docs/CBM_FORMAT.md (LUT via header byte 9).
- [0xC4] = 3682:0000 is the ADLIB SOUND DRIVER, not game logic (VERIFIED).
  MAINLOOP.md corrected.

## Stage 39: game-session captures consumed
- Clean EXE disasm now covers ALL 432 traced 0823 addresses < 0xB0E0 as instruction
  starts (timer ISR at 0x0000 included; rdisasm2 seed-range bug fixed).
- OVL0 is a section-loaded overlay: swap area 1331:0000 (= 0823:B0E0); race
  section = OVL0 file 0x051A0-0x08540 -> disasm/OVL_RACE.clean.asm (621 funcs,
  93.8% of section). Resident part OVL0 0x0000-0x09A0 -> disasm/OVL_RESIDENT.clean.asm;
  its func_0x03BC is the section loader (20-byte section entries, type nibble at
  entry+1 dispatches via table at 0x642E).
- CBM table-0 = keep-current-table now VERIFIED (GALLERY2 with table 3: 90.4%).

## Stage 40: Rome province screen in port (first real screen)
- `--dump-frame out.bin province_rome` composes the screen: 116 background,
  ROME.CBM strip at (0,0) (LUT 19, pixel-perfect vs capture), two windowed
  panels with the game's border style, game font text. Diff vs capture: 10.5%,
  strip 0%. Remaining: text glyph positions in the panels.

## Stage 41: sprite sheets (HDR/DAT) in port; crowd portrait drawn
- assets_load_sheet + assets_blit_sprite implement the xxxHDR/xxxDAT sheet format
  (VERIFIED on INIT2: 14-byte entries, u32 offset into raw-nibble DAT, byte8 =
  transparent nibble). Province screen draws INIT2 sprite 0 at (255,104) LUT 22.
- Diff vs capture now 6.7%. OVL0 map/province section disassembled:
  disasm/OVL_MAP.clean.asm (832 funcs, 95.3%).

## Stage 43: generic data-driven province screen
- draw_province() driven by game data: menu items from DS:573C records (12-byte, read from
  OVL0 at runtime), mood from func_0x6c26 index -> INIT2 sprite, status strings composed per
  func_0x6f0c, title centred at (160,66). --dump-frame out.bin province_rome|province_narbonensis.
- Rome: 707 px (1.1%). Narbonensis: menu+status ~700 px (bevels), strip asset still unidentified.

## Stage 44: layered province strips (VERIFIED 20160/20160 on Narbonensis)
- Non-city strips = sky fill (23) rows 0-30 + NEGBG?.CBM at y=19 + NEGFGX-style fg at y=31,
  LUT 1, byte8 (8) transparent. Port: blit_cbm_at_trans. Narb now 964 px (1.5%); Rome 707 px.
