#!/usr/bin/env python3
"""Align a DOSBox-X VGA screen dump with a raw .CBM to derive the CBM format.

Usage: align_screen.py <screen.bin> <image.cbm> <out_prefix> [palette.bin]
- screen.bin: 0xFA00 bytes from A000:0000 (320x200x256, mode 13h)
- image.cbm: raw extracted .CBM (4-byte header 00 00 W H + payload)
Renders the screen and the image region to PNG for visual comparison, and
writes the extracted region as raw pixels for diffing against the CBM payload.
"""
import struct, sys, zlib

def png_write(path, w, h, rgb):
    def chunk(t, d):
        return struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d) & 0xffffffff)
    raw = b''.join(b'\x00' + rgb[y*w*3:(y+1)*w*3] for y in range(h))
    open(path, 'wb').write(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0))
                           + chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b''))

def main():
    screen = open(sys.argv[1], 'rb').read()
    cbm = open(sys.argv[2], 'rb').read()
    prefix = sys.argv[3]
    pal = open(sys.argv[4], 'rb').read() if len(sys.argv) > 4 else None
    W, H = 320, 200
    iw, ih = cbm[2], cbm[3]
    print(f'screen {len(screen)} bytes ({W}x{H}), cbm image {iw}x{ih}, cbm file {len(cbm)} bytes')
    def to_rgb(px):
        out = bytearray()
        for p in px:
            if pal and p * 3 + 2 < len(pal):
                out += bytes(((pal[p*3] & 0x3f) << 2, (pal[p*3+1] & 0x3f) << 2, (pal[p*3+2] & 0x3f) << 2))
            else:
                out += bytes([p, p, p])
        return bytes(out)
    png_write(prefix + '_screen.png', W, H, to_rgb(screen[:W*H]))
    # extract the image-sized region starting at a few candidate origins (skip UI at top)
    for oy in (0, 20, 40):
        reg = bytearray()
        for y in range(ih):
            row = (oy + y) * W
            reg += screen[row:row + iw]
        png_write(f'{prefix}_region_y{oy}.png', iw, ih, to_rgb(bytes(reg)))
        open(f'{prefix}_region_y{oy}.raw', 'wb').write(bytes(reg))
    print(f'wrote {prefix}_screen.png and region PNGs/raw for y offsets 0/20/40')
    print('next: diff region raw vs cbm payload to derive the pixel encoding')

if __name__ == '__main__':
    main()
