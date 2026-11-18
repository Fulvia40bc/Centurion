"""Independent FLC decoder (written from the Autodesk format notes, not from flc.c).
Usage: flcpy.py DIR NAME...  -> same text as flc_test: header line, then 'index crc(pix) crc(pal)' per frame + ring."""
import sys, struct, zlib

def s8(v): return v - 256 if v > 127 else v

def play(b):
    size, magic, frames, w, h, depth, flags, speed = struct.unpack_from('<IHHHHHHI', b, 0)
    assert magic == 0xAF12 and depth == 8, (hex(magic), depth)
    oframe1 = struct.unpack_from('<I', b, 80)[0] or 128
    pix = bytearray(w * h); pal = bytearray(768)
    out = []
    p = oframe1
    for idx in range(frames + 1):
        fsize, ftype, nchunks = struct.unpack_from('<IHH', b, p)
        while ftype != 0xF1FA:          # prefix chunks
            p += fsize; fsize, ftype, nchunks = struct.unpack_from('<IHH', b, p)
        q = p + 16
        for _ in range(nchunks):
            csize, ctype = struct.unpack_from('<IH', b, q); d = q + 6
            if ctype == 4:              # COLOR_256
                npk = struct.unpack_from('<H', b, d)[0]; d += 2; i = 0
                for _ in range(npk):
                    i += b[d]; n = b[d + 1] or 256; d += 2
                    pal[3 * i:3 * (i + n)] = b[d:d + 3 * n]; d += 3 * n; i += n
            elif ctype == 15:           # BYTE_RUN
                for y in range(h):
                    d += 1; x = 0; row = y * w
                    while x < w:
                        c = s8(b[d]); d += 1
                        if c > 0: pix[row + x:row + x + c] = bytes([b[d]]) * c; d += 1; x += c
                        else: c = -c; pix[row + x:row + x + c] = b[d:d + c]; d += c; x += c
            elif ctype == 7:            # DELTA_FLC
                lines = struct.unpack_from('<H', b, d)[0]; d += 2; y = 0
                for _ in range(lines):
                    while True:
                        op = struct.unpack_from('<H', b, d)[0]; d += 2
                        if op & 0xC000 == 0xC000: y += 0x10000 - op
                        elif op & 0xC000 == 0x8000: pix[y * w + w - 1] = op & 0xFF
                        else: npk = op; break
                    x = 0; row = y * w
                    for _ in range(npk):
                        x += b[d]; c = s8(b[d + 1]); d += 2
                        if c > 0: pix[row + x:row + x + 2 * c] = b[d:d + 2 * c]; d += 2 * c; x += 2 * c
                        else: c = -c; pix[row + x:row + x + 2 * c] = b[d:d + 2] * c; d += 2; x += 2 * c
                    y += 1
            elif ctype == 18: pass      # PSTAMP
            else: raise SystemExit('unhandled chunk %d' % ctype)
            q += csize
        p += fsize
        out.append((idx if idx < frames else 0, zlib.crc32(pix), zlib.crc32(pal)))
        if idx == frames - 1: ring_at = p
    return w, h, frames, speed, out

for n in sys.argv[2:]:
    w, h, fr, sp, out = play(open(f'{sys.argv[1]}/{n}', 'rb').read())
    print(n, w, h, fr, sp)
    for i, a, c in out: print('%u %08X %08X' % (i, a, c))
