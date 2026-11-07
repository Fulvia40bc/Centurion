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
 8  u8    transparent nibble WHEN the image is drawn as an overlay. MAP.CBM has 06 and the game draws all
          6,172 nibble-6 pixels (VERIFIED vs RAM): full-screen backdrops are drawn opaque. But the province strip
          layers NEGBG* (byte8 = 8) over a sky fill, and nibble 8 IS transparent there (VERIFIED, see below), as is
          nibble 0 for the INIT2 sprites. Values seen: 00 02 06 07 08 09 0D 0F.
 9  u8    colour lookup table index; 0 = keep the current table (see below)
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
- **Table index 0 = don't change the colours** (only GALLERY2, BB1, BB2):
  - **BB1 / BB2 are the EGA versions of BATTLENW / DESERT**: same layout (trees, rocks), dithered for 16 colours, and correct with
    the identity table (= the standard EGA palette, CMAP entries 0–15). A VGA game never draws them.
  - **GALLERY2 has exactly RACEEST's pixels** (22,400/22,400 nibbles) and is drawn on the race screen, where table 3 is already set,
    so "keep the current table" gives the right colours.
  - Both fit "0 = leave the current table alone": in VGA mode the screen's table stays; in EGA mode it is always identity.
    VERIFIED 2026-11-06: in the chariot race the strip at rows 130-199 matches GALLERY2 with table 3 on 90.4% of pixels (rest = chariot sprite), vs 2% with table 0.
- **x/y header fields are hotspot offsets**, relative to where the game draws the image, not screen positions:
  WATER.CBM (100×47 sprite) has x = −220; NEGBG* have y = −19 (or −20/−21/−15); TITLE has (−4, −10). Their exact use is unknown.
- **Row stride:** each row is `bytes_per_row` bytes (2 nibbles each); for odd widths (TITLE.CBM: 313 px, 157 bytes) the last
  nibble of each row is padding. Crop to `width`.
- All 82 CBMs checked visually on a contact sheet with these rules (2026-11-06).

## Layered province strips (VERIFIED 2026-11-07, Narbonensis: 20,160 / 20,160 pixels)

Non-city provinces don't have their own strip image. Rows 0-62 are composed from layers (all in MNEGO.DAT, LUT 1):
1. rows 0-30 filled with colour **23** (sky);
2. **NEGBG?.CBM** (A-K, 10-16 rows: mountains) drawn at y = -hotspot_y (NEGBGA: y = 19), **nibble 8 (= byte8) transparent**;
3. **NEGFG?.CBM** (A-Z, 32 rows: foreground) at y = 31.
Narbonensis = NEGBGA (MNEGO index 31) + NEGFGX (index 65). The file names aren't stored as strings in the EXE or OVL0, so the
game picks them by archive index; where the per-province choice lives is still UNVERIFIED (probably the province record).
SKY0-3 are not used here (the sky is a plain fill). Cities like Rome use a full 63-row strip (ROME.CBM, MNEGO index 68).

## Runtime notes (from the debugger captures)

- Screens are composed in a 320×200 work buffer at segment 2687 and copied to VRAM by the loop at `0823:95DB`.
- The decoded map is kept in its own buffer at segment 3942, used to repaint the map behind closed windows.
- `0823:7F96–80D4` is a rectangle fill (solid / XOR highlight), with EGA (port 3CEh), 4-bit and 8-bit paths.
- `0823:1BE6` draws the mouse cursor; `DS:00B6/00B8` hold the mouse x/y.
