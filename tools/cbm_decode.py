#!/usr/bin/env python3
"""Reference decoder for Centurion .CBM images (verified against game RAM).

usage: cbm_decode.py GAME_DIR NAME.CBM out.png

Format (see docs/CBM_FORMAT.md):
  14-byte header, little-endian:
    u16 width_px, s16 x_offset, s16 y_offset, u8 bytes_per_row, u8 rows,
    u8 transparent_nibble (probably), u8 lut_index, u16 unpacked_size, u16 0
  then the game's LZSS stream (same as .LZW members) -> unpacked_size bytes of
  4-bit pixels, high nibble first. Each nibble is mapped to a palette index via
  a 16-entry lookup table: OVL0.OVL offset 0x114C0 + 16*lut_index
  (= DS:60C0 at runtime). Palette: CMAP.BIN (250 x 6-bit RGB) in CENT.DAT.
"""
import os, struct, sys, zlib, importlib.util

HERE = os.path.dirname(os.path.abspath(__file__))
_spec = importlib.util.spec_from_file_location('extract_data', os.path.join(HERE, 'extract_data.py'))
ex = importlib.util.module_from_spec(_spec); _spec.loader.exec_module(ex)

LUT_TABLE_OFFSET = 0x114C0   # in OVL0.OVL


def find_member(game_dir, name):
    for f in os.listdir(game_dir):
        if f.upper().endswith('.DIR'):
            blob = open(os.path.join(game_dir, f[:-4] + '.DAT'), 'rb').read()
            for n, off, ln, fmt in ex.parse_dir(os.path.join(game_dir, f)):
                if n.upper() == name.upper():
                    return blob[off:off + ln]
    raise KeyError(name)


def decode_cbm(raw, ovl0):
    """Return (width, height, x_off, y_off, transparent, palette_indices)."""
    w, xo, yo, bpr, rows, trans, lut, size = struct.unpack_from('<HhhBBBBH', raw, 0)
    packed = ex.lzss_decompress(raw[14:])[:size]
    table = ovl0[LUT_TABLE_OFFSET + 16 * lut: LUT_TABLE_OFFSET + 16 * lut + 16]
    px = bytearray()
    for b in packed:
        px.append(table[b >> 4]); px.append(table[b & 15])
    return bpr * 2, rows, xo, yo, trans, bytes(px)


def write_png(path, w, h, px, cmap):
    pal = bytearray()
    for i in range(256):
        rgb = cmap[i * 3:i * 3 + 3] if i * 3 + 3 <= len(cmap) else b'\0\0\0'
        pal += bytes(v << 2 for v in rgb)
    raw = b''.join(b'\0' + px[y * w:(y + 1) * w] for y in range(h))
    ch = lambda t, d: struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d))
    open(path, 'wb').write(b'\x89PNG\r\n\x1a\n' + ch(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 3, 0, 0, 0))
                           + ch(b'PLTE', bytes(pal)) + ch(b'IDAT', zlib.compress(raw)) + ch(b'IEND', b''))


if __name__ == '__main__':
    game, name, out = sys.argv[1:4]
    ovl0 = open(os.path.join(game, 'OVL0.OVL'), 'rb').read()
    w, h, xo, yo, trans, px = decode_cbm(find_member(game, name), ovl0)
    write_png(out, w, h, px, find_member(game, 'CMAP.BIN'))
    print(f'{name}: {w}x{h} offset ({xo},{yo}) -> {out}')
