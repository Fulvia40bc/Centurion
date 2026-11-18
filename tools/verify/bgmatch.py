"""bgmatch.py CAPTURE.png GIF... : per 640x480 GIF, how many capture pixels have the same palette index, and whether
the capture's palette (rPAL = the raw 6-bit DAC, else PLTE) equals the GIF's colour table >> 2."""
import os, struct, sys, zlib
here = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, here)
src = open(os.path.join(here, 'fontfind.py')).read()
exec(src.split('def load_font')[0])
gifpy = type(sys)('gifpy')   # only its decode(): the module runs a command line when imported
exec(open(os.path.join(here, 'gifpy.py')).read().split('def bmp_pixels')[0], gifpy.__dict__)

def chunks(path):
    b = open(path, 'rb').read(); q = 8; ch = {}
    while q < len(b):
        n = struct.unpack_from('>I', b, q)[0]; ch[b[q + 4:q + 8]] = ch.get(b[q + 4:q + 8], b'') + b[q + 8:q + 8 + n]; q += 12 + n
    return ch

cap = sys.argv[1]
W, H, img = png_indices(cap)
ch = chunks(cap)
rpal = ch.get(b'rPAL'); plte = ch[b'PLTE']
print('capture palette: rPAL', len(rpal) if rpal else None, 'bytes; first entries', (rpal or plte)[:12].hex())
for g in sys.argv[2:]:
    b = open(g, 'rb').read()
    try:
        w, h, pix, stop = gifpy.decode(b)
    except SystemExit as e:
        print(os.path.basename(g), 'skipped:', e); continue
    if (w, h) != (640, 480):
        continue
    same = sum(1 for a, c in zip(img, pix) if a == c)
    ncol = 1 << ((b[10] & 7) + 1)
    gp = b[13:13 + 3 * ncol]
    dac = bytes(x >> 2 for x in gp)
    pal_ok = rpal is not None and rpal[:len(dac)] == dac
    print(f'{os.path.basename(g):14} same index {same:6d} / {W * H} ({100 * same / (W * H):5.1f}%)  palette == GIF>>2: {pal_ok}')
