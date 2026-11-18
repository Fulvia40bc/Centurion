"""Check an indexed PNG from plat_save_png against the source GIF, independently of the port's code.
Usage: pngcheck.py OUT.png GIFFILE   (the GIF is 640x480 at 0,0 with a global palette)"""
import struct, sys, zlib, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gifpy  # noqa: E402  (its decode() is the independent LZW decoder from the Task 2 check)

png = open(sys.argv[1], 'rb').read()
assert png[:8] == b'\x89PNG\r\n\x1a\n'
p, chunks = 8, {}
while p < len(png):
    n = struct.unpack_from('>I', png, p)[0]; t = png[p + 4:p + 8]; d = png[p + 8:p + 8 + n]
    crc = struct.unpack_from('>I', png, p + 8 + n)[0]
    assert zlib.crc32(t + d) == crc, 'CRC ' + t.decode()
    chunks[t] = d; p += 12 + n
w, h, depth, ctype = struct.unpack_from('>IIBB', chunks[b'IHDR'])
assert (w, h, depth, ctype) == (640, 480, 8, 3), (w, h, depth, ctype)
raw = zlib.decompress(chunks[b'IDAT'])   # checks the zlib stream and its Adler-32
assert len(raw) == h * (w + 1) and all(raw[y * (w + 1)] == 0 for y in range(h))
pix = b''.join(raw[y * (w + 1) + 1:(y + 1) * (w + 1)] for y in range(h))
g = open(sys.argv[2], 'rb').read()
gw, gh, gpix, stop = gifpy.decode(g)
assert (gw, gh) == (640, 480), (gw, gh)
ncol = 1 << ((g[10] & 7) + 1)
gpal = g[13:13 + 3 * ncol]
diff = sum(1 for a, b in zip(pix, gpix[:w * h]) if a != b)
print('png', w, 'x', h, 'indexed; gif decode:', stop, '; pixels differing:', diff,
      '; palette entries', ncol, 'identical:', chunks[b'PLTE'][:3 * ncol] == gpal,
      '; rest of palette black:', set(chunks[b'PLTE'][3 * ncol:]) <= {0})
