"""Find strings in a DOSBox-X raw capture by rendering them with the game's FGF fonts (Python, written from the
format notes in OVERVIEW §4, independent of fgfont.c). A match = every glyph pixel has one colour C, and no other
pixel inside the text's box (from the top of the tallest cell to the glyph bottoms, x from start to end) has C.
Usage: fontfind.py CAPTURE.png FONTDIR "string" ["string" ...]"""
import struct, sys, zlib, os

def png_indices(path):
    b = open(path, 'rb').read(); q = 8; idat = b''; ihdr = None
    while q < len(b):
        n = struct.unpack_from('>I', b, q)[0]; t = b[q + 4:q + 8]; d = b[q + 8:q + 8 + n]
        if t == b'IHDR': ihdr = d
        if t == b'IDAT': idat += d
        q += 12 + n
    w, h, bd, ct = struct.unpack_from('>IIBB', ihdr)
    assert bd == 8 and ct == 3
    raw = zlib.decompress(idat); out = bytearray(); prev = bytearray(w); stride = w + 1
    for y in range(h):
        f = raw[y * stride]; line = bytearray(raw[y * stride + 1:(y + 1) * stride])
        for x in range(w):
            a = line[x - 1] if x else 0; up = prev[x]; c = prev[x - 1] if x else 0
            if f == 1: line[x] = (line[x] + a) & 255
            elif f == 2: line[x] = (line[x] + up) & 255
            elif f == 3: line[x] = (line[x] + ((a + up) >> 1)) & 255
            elif f == 4:
                p = a + up - c; pa, pb, pc = abs(p - a), abs(p - up), abs(p - c)
                line[x] = (line[x] + (a if pa <= pb and pa <= pc else up if pb <= pc else c)) & 255
        out += line; prev = line
    return w, h, bytes(out)

def load_font(path):
    f = open(path, 'rb').read(); bpr, h = f[0], f[1]; cell = bpr * h
    w = f[2:98]; rows = f[98:194]; cells = f[194:]
    a = 33; ab = (w[a] + 7) >> 3; g = cells[a * cell:]; base = 0
    for r in range(rows[a] - 1, -1, -1):
        if any(g[r * ab:(r + 1) * ab]): break
        base += 1
    return dict(h=h, w=w, rows=rows, cells=cells, cell=cell, base=base)

def mask(font, s, spacing):
    """pixels (dx, dy) relative to (x0, baseline y); dy = 0 is the baseline row"""
    px = []; x = 0
    for ch in s.encode('latin1'):
        c = ch - 32; w = font['w'][c]; rows = font['rows'][c]; cb = (w + 7) >> 3; g = font['cells'][c * font['cell']:]
        for r in range(rows):
            for b in range(cb * 8):
                if g[r * cb + (b >> 3)] & (0x80 >> (b & 7)):
                    px.append((x + b, font['base'] - (rows - 1 - r)))
        x += w + spacing
    return px, x

def find(W, H, img, px, adv, font):
    top = font['base'] - font['h'] + 1; bot = font['base']
    hits = []
    dx0, dy0 = px[0]
    dx1, dy1 = px[len(px) // 2]
    rest = px[1:]
    for y in range(-top, H - bot):
        r0 = (y + dy0) * W + dx0; r1 = (y + dy1) * W + dx1
        for x in range(0, W - adv + 1):
            c = img[r0 + x]
            if c in SKIP or img[r1 + x] != c:
                continue
            ok = True
            for dx, dy in rest:
                if img[(y + dy) * W + x + dx] != c:
                    ok = False; break
            if not ok:
                continue
            s = set(px)
            stray = sum(1 for yy in range(top, bot + 1) for xx in range(0, adv)
                        if (xx, yy) not in s and img[(y + yy) * W + x + xx] == c)
            if stray == 0:
                hits.append((x, y, c))
    return hits

W, H, img = png_indices(sys.argv[1])
from collections import Counter
SKIP = {c for c, _ in Counter(img).most_common(1)}   # the background colour: text is never drawn in it here
print('skipping background colour', SKIP)
fonts = {n: load_font(os.path.join(sys.argv[2], n)) for n in ('BLOCK05.FGF', 'COGNAC10.FGF', 'COGNAC19.FGF')}
for s in sys.argv[3:]:
    for n, f in fonts.items():
        for sp in (0, 1, 2):
            px, adv = mask(f, s, sp)
            if not px: continue
            for x, y, c in find(W, H, img, px, adv, f):
                print(f'{s!r}: {n} spacing {sp} at x={x} baseline y={y} colour {c} (advance {adv}, {len(px)} pixels)')
