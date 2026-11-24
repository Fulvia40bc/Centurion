"""Pack PNG renders into a Windows .ico (PNG-compressed entries, Vista and later).

    python tools/make_icon.py port/centurion.ico icon_256.png icon_48.png icon_32.png icon_24.png icon_16.png

Each PNG must be square, 1..256 px, 32-bit RGBA. Standard library only.
"""
import struct
import sys


def png_info(data):
    if data[:8] != b'\x89PNG\r\n\x1a\n' or data[12:16] != b'IHDR':
        raise SystemExit('not a PNG')
    w, h, depth, ctype = struct.unpack('>IIBB', data[16:26])
    if w != h or not 1 <= w <= 256:
        raise SystemExit(f'PNG is {w}x{h}: an icon entry must be square, at most 256')
    if (depth, ctype) != (8, 6):
        raise SystemExit(f'PNG is depth {depth} colour type {ctype}: needs 8-bit RGBA (type 6)')
    return w


def main(out, pngs):
    blobs = [open(p, 'rb').read() for p in pngs]
    sizes = [png_info(b) for b in blobs]
    order = sorted(range(len(blobs)), key=lambda i: -sizes[i])
    offset = 6 + 16 * len(blobs)
    head = struct.pack('<HHH', 0, 1, len(blobs))
    dirs, body = b'', b''
    for i in order:
        s = sizes[i] % 256          # 256 is written as 0
        dirs += struct.pack('<BBBBHHII', s, s, 0, 0, 1, 32, len(blobs[i]), offset + len(body))
        body += blobs[i]
    with open(out, 'wb') as f:
        f.write(head + dirs + body)
    print(f'{out}: {", ".join(str(sizes[i]) for i in order)} px, {len(head + dirs + body)} bytes')


if __name__ == '__main__':
    if len(sys.argv) < 3:
        raise SystemExit(__doc__)
    main(sys.argv[1], sys.argv[2:])
