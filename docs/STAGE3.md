# Stage 3 Report — game data pipeline

## .DAT/.DIR container format (fully decoded)
- `.DAT` file: 4-byte signature `CA CA D0 D0`, then concatenated member blobs.
- `.DIR` file: 20-byte entries — `int32 offset`, `uint16 length`, `char name[13]`
  (NUL-padded), `uint8 format` where `0x00` = raw (`.BIN`), `0x80` = LZW-compressed
  (`.LZW`), `0xC0` = CBM image (`.CBM`).
- Reference: PeterSwinkels' Centurion-Data-Extractor confirms the same layout.
- Tool: `tools/extract_data.py` — extracts every member of every `.DAT` pair.
  Verified: all 9 `.DIR` files parse cleanly; all `.CBM` images extract byte-exact.

## Assets recovered
- ~21 `.CBM` images (title/map/world/battle/arena/race/circus/colosseum screens)
- `CMAP.BIN` (750 bytes = 250-entry 6-bit RGB palette)
- dozens of `.LZW` data blobs (menus, negotiations, places, tactics, audio tables:
  `ADLIB.LZW`, `SPEAKER.LZW`, `TANDY.LZW` — confirming AdLib/Speaker/Tandy audio)

## .CBM image format (partially decoded)
- 4-byte header `00 00 W H` (e.g. `00 00 A0 C8` = 160x200), then an RLE stream.
- Best-guess (count, value) pair decoding yields ~W*H pixels but wrong 2D layout —
  exact run encoding still to be confirmed from the decoder in `disasm/OVL0.asm`.
- Tool: `tools/cbm2png.py` renders a best-effort PNG (pure stdlib).

## .LZW compression (not yet decoded)
- Custom LZW variant; standard decoders fail on it (also reported on encode.su —
  nobody has cracked it with off-the-shelf tools). The game ships `unlzss.exe`.
- Next step: locate the unpacker routine in the EXE disassembly (the DOS int 0x21
  file-read path) and port it.

## Port scaffold
- `port/` contains an SDL2 backend (320x200x256 framebuffer + palette, input,
  square-wave audio, fixed 60 Hz timestep) with a stub game loop.
  Syntax-checked with gcc -Wall -Wextra.
