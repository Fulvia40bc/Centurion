"""flcframes.py FLC SHOT_PREFIX COUNT [X Y]: fg_showflic's frames in DOSBox-X raw captures against the independent
decoder flcpy.py. SHOT_PREFIX_j.png (j = 1..COUNT, from tools/dosbox/flcshots.sh) is the screen at fg_showflic's key
check after its j-th frame; the file's first frame is the 0xF100 prefix, so shot j >= 2 must show flcpy's frame j - 2
in the W x H window at (X, Y) (default (210, 90), where the game centres every calamity FLC), and its palette (the
COLOR_256 bytes >> 2, widened as the VGA DAC: v << 2 | v >> 4) in the capture's PLTE. Shot 1 is only reported.
Prints one line per shot: differing window pixels and palette entries. Exit 1 if any shot j >= 2 differs."""
import os, struct, sys
here = os.path.dirname(os.path.abspath(__file__))
exec(open(os.path.join(here, 'fontfind.py')).read().split('def load_font')[0])
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

flc, pre, count = sys.argv[1], sys.argv[2], int(sys.argv[3])
X0, Y0 = (int(sys.argv[4]), int(sys.argv[5])) if len(sys.argv) > 5 else (210, 90)
w, h, frames, speed, out = fl.play(open(flc, 'rb').read())
bad = 0
for j in range(1, count + 1):
    W, H, img = png_indices(f'{pre}_{j}.png')
    cp = plte(f'{pre}_{j}.png')
    if j == 1:
        print(f'shot 1 (prefix frame): not compared'); continue
    k = j - 2
    if k >= len(out):
        print(f'shot {j}: no frame {k} in the file'); bad += 1; continue
    pix, pal = out[k][1], out[k][2]
    dp = sum(1 for y in range(h) for x in range(w) if img[(Y0 + y) * W + X0 + x] != pix[y * w + x])
    dpal = 0
    for i in range(256):
        for c in range(3):
            v = pal[3 * i + c] >> 2
            if cp[3 * i + c] != ((v << 2) | (v >> 4)): dpal += 1; break
    print(f'shot {j} = frame {k}: {dp} px, {dpal} palette entries differ')
    if dp or dpal: bad += 1
print(f'{count - 1} frames compared, {bad} differ')
sys.exit(1 if bad else 0)
