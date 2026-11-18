"""cmpframe.py A.png B.png: two 640x480 indexed PNGs: same palette indices? same displayed RGB per pixel?"""
import os, struct, sys
here = os.path.dirname(os.path.abspath(__file__))
exec(open(os.path.join(here, 'fontfind.py')).read().split('def load_font')[0])

def plte(path):
    b = open(path, 'rb').read(); q = 8
    while q < len(b):
        n = struct.unpack_from('>I', b, q)[0]
        if b[q + 4:q + 8] == b'PLTE': return b[q + 8:q + 8 + n]
        q += 12 + n

wa, ha, a = png_indices(sys.argv[1]); wb, hb, b = png_indices(sys.argv[2])
pa, pb = plte(sys.argv[1]), plte(sys.argv[2])
idx = sum(1 for x, y in zip(a, b) if x == y)
rgb = sum(1 for x, y in zip(a, b) if pa[3 * x:3 * x + 3] == pb[3 * y:3 * y + 3])
used = sorted(set(a))
print(f'{os.path.basename(sys.argv[1])} vs {os.path.basename(sys.argv[2])}: same index {idx}/{wa * ha}, '
      f'same RGB {rgb}/{wa * ha}; palette entries used {len(used)}, '
      f'of those identical: {sum(1 for i in used if pa[3 * i:3 * i + 3] == pb[3 * i:3 * i + 3])}')
