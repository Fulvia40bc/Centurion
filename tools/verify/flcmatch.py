"""flcmatch.py CAPTURE.png FLC: for each frame of the FLC (independent decoder flcpy.py), how many capture pixels have
the same index; for the best frame, whether its palette (6-bit FLC COLOR_64 / 8-bit COLOR_256, widened as the VGA)
equals the capture's, and whether PAL.PAL >> 2 does."""
import os, struct, sys, zlib
here = os.path.dirname(os.path.abspath(__file__))
src = open(os.path.join(here, 'fontfind.py')).read()
exec(src.split('def load_font')[0])
fl = type(sys)('flcpy')
code = open(os.path.join(here, 'flcpy.py')).read().split('for n in sys.argv[2:]:')[0]
code = code.replace('out.append((idx if idx < frames else 0, zlib.crc32(pix), zlib.crc32(pal)))',
                    'out.append((idx, bytes(pix), bytes(pal)))')
exec(code, fl.__dict__)

def plte(path):
    b = open(path, 'rb').read(); q = 8
    while q < len(b):
        n = struct.unpack_from('>I', b, q)[0]
        if b[q + 4:q + 8] == b'PLTE': return b[q + 8:q + 8 + n]
        q += 12 + n

W, H, img = png_indices(sys.argv[1])
cp = plte(sys.argv[1])
w, h, frames, speed, out = fl.play(open(sys.argv[2], 'rb').read())
best = max(out, key=lambda o: sum(1 for a, c in zip(img, o[1]) if a == c))
same = sum(1 for a, c in zip(img, best[1]) if a == c)
print(f'{frames} frames; best frame {best[0]}: {same} / {W * H} pixels same index ({100 * same / (W * H):.1f}%)')
widen = lambda v: (v << 2) | (v >> 4)
pal = best[2]
print('  FLC palette max value', max(pal))
p6 = bytes(widen(v & 63) for v in pal) if max(pal) < 64 else bytes(widen(v >> 2) for v in pal)
print('  FLC palette (through the 6-bit DAC) == capture palette:', p6 == cp,
      '| entries differing', sum(1 for i in range(256) if p6[3 * i:3 * i + 3] != cp[3 * i:3 * i + 3]))
g = open('C:/work/advciv_play/PAL.PAL', 'rb').read()[13:13 + 768]
pp = bytes(widen(v >> 2) for v in g)
print('  PAL.PAL >> 2 (widened) == capture palette:', pp == cp,
      '| entries differing', sum(1 for i in range(256) if pp[3 * i:3 * i + 3] != cp[3 * i:3 * i + 3]))
diff = [(i % W, i // W) for i, (a, c) in enumerate(zip(img, best[1])) if a != c]
if diff:
    xs = [d[0] for d in diff]; ys = [d[1] for d in diff]
    print(f'  differing pixels: {len(diff)}, box x {min(xs)}..{max(xs)}, y {min(ys)}..{max(ys)}')
