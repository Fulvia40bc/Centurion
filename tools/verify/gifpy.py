"""Third, independent GIF decode (plain Python LZW) to arbitrate port vs GDI+.
Usage: gifpy.py SRC_DIR BMP_DIR NAME...  -> per name: python-vs-port pixel diffs, and where they are."""
import sys, struct, zlib

def decode(b):
    p = 13
    if b[10] & 0x80: p += 3 * (1 << ((b[10] & 7) + 1))
    while b[p] == 0x21:
        p += 2
        while b[p]: p += b[p] + 1
        p += 1
    assert b[p] == 0x2C
    l, t, w, h = struct.unpack_from('<HHHH', b, p + 1); fl = b[p + 9]; p += 10
    if fl & 0x80: p += 3 * (1 << ((fl & 7) + 1))
    mcs = b[p]; p += 1
    data = bytearray()
    while b[p]: data += b[p + 1:p + 1 + b[p]]; p += b[p] + 1
    clear, eoi = 1 << mcs, (1 << mcs) + 1
    out = bytearray()
    bitpos = 0
    def reset():
        return [bytes([i]) for i in range(clear)] + [b'', b''], mcs + 1
    table, size = reset()
    prev = None
    nbits = len(data) * 8
    stop = 'ran out of data'
    while bitpos + size <= nbits:
        v = int.from_bytes(data[bitpos >> 3:(bitpos >> 3) + 3].ljust(3, b'\0'), 'little') >> (bitpos & 7) & ((1 << size) - 1)
        bitpos += size
        if v == clear:
            table, size = reset(); prev = None; continue
        if v == eoi: stop = 'EOI'; break
        if prev is None:
            e = table[v]
        elif v < len(table):
            e = table[v]
            if len(table) < 4096: table.append(prev + e[:1])
        elif v == len(table):
            e = prev + prev[:1]
            if len(table) < 4096: table.append(e)
        else:
            stop = 'bad code %d at pixel %d' % (v, len(out)); break
        out += e; prev = e
        if len(table) == (1 << size) and size < 12: size += 1
    if fl & 0x40: raise SystemExit('interlaced: not handled here')
    return w, h, bytes(out), stop

def bmp_pixels(path):
    b = open(path, 'rb').read()
    off, w, h = struct.unpack_from('<I', b, 10)[0], *struct.unpack_from('<ii', b, 18)
    stride = (w + 3) & ~3
    rows = [b[off + y * stride: off + y * stride + w] for y in range(abs(h))]
    if h > 0: rows.reverse()
    return w, abs(h), b''.join(rows)

src, bmp = sys.argv[1], sys.argv[2]
for n in sys.argv[3:]:
    w, h, px, stop = decode(open(f'{src}/{n}', 'rb').read())
    _, _, mine = bmp_pixels(f'{bmp}/{n}.bmp')
    n_px = w * h
    print(f'{n}: {w}x{h}, python decoded {len(px)} px ({stop}), crc of first w*h %08X' % (zlib.crc32(px[:n_px].ljust(n_px, b"\0"))))
    d = [i for i in range(min(n_px, len(px))) if px[i] != mine[i]]
    print(f'   python vs port: {len(d)} px differ' + (f', first at row {d[0]//w} col {d[0]%w}, last row {d[-1]//w}' if d else ''))
