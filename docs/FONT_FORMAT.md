# Font format (verified)

Verified on 2026-11-06: re-rendering the line "The people are Rebellious" from the province screen
(work buffer capture, segment 2687) with the data below reproduces all 235 text pixels, with 0
extra and 0 missing (the only unexplained pixels are the box's border line at x=312).

Found statically by cutting glyphs out of the screen capture and searching memory. Not yet found:
the drawing routine itself (not needed for the format).

## Location

One contiguous block in the data segment (DS = 1686), loaded from `OVL0.OVL`
(DS offset = OVL0 file offset − 0xB400). Covers characters 0x20–0x7F (96 glyphs).

| DS offset | OVL0.OVL offset | Contents |
|---|---|---|
| `9C9C` | `0x1509C` | `width[96]`: glyph width in pixels |
| `9CFC` | `0x150FC` | `yoff[96]`: vertical offset (1 normally, 2 for p q y, 3 for g) |
| `9D5C` | `0x1515C` | `glyph[96][8]`: 8 rows, 1 byte per row, leftmost pixel = bit 7 |

The 12 bytes before (`DS:9C90`) are `97 92 8d 93 e8 95 c0 95 79 95 f5 95`, which look like near pointers;
they may be a font/table directory (UNVERIFIED).

## Drawing

```
x = start_x
for ch in text:
    c = ord(ch) - 0x20
    for r in 0..7:
        bits = glyph[c][r]
        for i in 0..7:
            if bits & (0x80 >> i): pixel(x + i, base_y + yoff[c] + r) = colour
    x += width[c] + 1          # 1 pixel letter spacing (verified)
```

Only the ink pixels are drawn (the background is left untouched). The colour comes from the caller
(0 = black in the province text box). Capital height is 7 rows; with descenders, glyphs reach
`base_y + 9`.

## Not yet known

- Whether there are other fonts (e.g. a large title font). The pointer-like bytes at `DS:9C90` hint at a directory.
- How line breaks, colours and clipping are handled: that's in the drawing routine, which a runtime
  capture can locate if needed.
