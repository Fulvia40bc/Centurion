# .CBM image format (verified)

Verified on 2026-11-06 against images captured from the running game (DOSBox-X debugger):

| Image | Compared against | Result |
|---|---|---|
| MAP.CBM (320×200, LUT 10) | clean map buffer in RAM, segment 3942 | 63,999 / 64,000 pixels identical |
| ROME.CBM (320×63, LUT 19) | province-screen work buffer, segment 2687, rows 0–62 | 20,160 / 20,160 identical |

Reference decoder: `tools/cbm_decode.py`.

## Layout

```
off size  field
 0  u16   width in pixels          (320; 96 for LEADER*.CBM; 313 for TITLE.CBM)
 2  s16   x offset / hotspot       (0 for full screens; -4 for TITLE)
 4  s16   y offset / hotspot       (-19 for NEGBG*, -10 for TITLE)
 6  u8    bytes per row            (= width/2 rounded up: 160, 48, 157)
 7  u8    rows                     (200, 66, 51, ...)
 8  u8    transparent nibble (probably; 08 for leader portraits, 0F for NEGFG*) - unconfirmed
 9  u8    colour lookup table index
10  u16   unpacked size            (= bytes_per_row * rows)
12  u16   0
14  ...   LZSS stream (same codec as the .LZW members: 4 KB window prefilled 0x20,
          start 0xFEE, flag byte LSB-first, 1 = literal)
```

The unpacked data is 4-bit pixels, high nibble first. **Each nibble goes through a 16-entry colour
lookup table**; it is *not* `nibble + 0x68`. That rule happens to be close for MAP.CBM, but it is wrong: e.g. 6→0x68, 15→0x69,
7→0x6A, 1→0x6C, 8→0x6F, 0→0x00, 2→0x10.

- LUT table: `OVL0.OVL` file offset `0x114C0`, 16 bytes per entry; at runtime `DS:60C0` (DS = 1686).
- Entry 0 is the identity (`00 01 … 0F`). MAP uses entry 10 (`00 6C 10 6D 6E 00 68 6A 6F 70 71 72 73 74 75 69`).
- Palette: CMAP.BIN in CENT.DAT, 250 entries × 3 bytes of 6-bit VGA DAC values (`v << 2`).

## Runtime notes (from the debugger captures)

- Screens are composed in a 320×200 work buffer at segment 2687 and copied to VRAM by the loop at `0823:95DB`.
- The decoded map is kept in its own buffer at segment 3942, used to repaint the map behind closed windows.
- `0823:7F96–80D4` is a rectangle fill (solid / XOR highlight), with EGA (port 3CEh), 4-bit and 8-bit paths.
- `0823:1BE6` draws the mouse cursor; `DS:00B6/00B8` hold the mouse x/y.
