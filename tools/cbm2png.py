#!/usr/bin/env python3
"""Best-effort renderer for Centurion .CBM images -> PNG (pure stdlib).

Known so far: 4-byte header (00 00, W, H) — e.g. 160x200 — followed by an
RLE stream. The exact run encoding is still being reverse-engineered from
disasm/OVL0.asm; current best guess is (count, value) byte pairs, which yields
the right pixel count (~W*H) but not yet correct 2D structure.
Usage: cbm2png.py in.cbm out.png [palette.bin]
"""
import struct, sys, zlib

def png_write(path, w, h, rgb):
    def chunk(t, d):
        return struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d) & 0xffffffff)
    raw = b''.join(b'\x00' + rgb[y*w*3:(y+1)*w*3] for y in range(h))
    png = (b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0))
           + chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b''))
    open(path, 'wb').write(png)

def decode(data):
    w, h = data[2], data[3]
    out = bytearray()
    i = 4
    while i + 1 < len(data) and len(out) < w * h:
        c, v = data[i], data[i+1]
        i += 2
        out += bytes([v]) * c
    return w, h, bytes(out[:w*h])

def main():
    data = open(sys.argv[1], 'rb').read()
    w, h, px = decode(data)
    pal = open(sys.argv[3], 'rb').read() if len(sys.argv) > 3 else None
    rgb = bytearray()
    for p in px:
        if pal and p * 3 + 2 < len(pal):
            rgb += bytes(((pal[p*3] & 0x3f) << 2, (pal[p*3+1] & 0x3f) << 2, (pal[p*3+2] & 0x3f) << 2))
        else:
            rgb += bytes([p, p, p])
    png_write(sys.argv[2], w, h, bytes(rgb))
    print(f'{sys.argv[1]}: {w}x{h}, {len(px)} px -> {sys.argv[2]}')

if __name__ == '__main__':
    main()
