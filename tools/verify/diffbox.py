"""diffbox.py CAPTURE.png GIF: group the pixels where the capture differs from the GIF into rectangles
(rows of differing pixels merged when they touch) and list them."""
import os, sys
here = os.path.dirname(os.path.abspath(__file__))
exec(open(os.path.join(here, 'fontfind.py')).read().split('def load_font')[0])
g = type(sys)('gifpy')
exec(open(os.path.join(here, 'gifpy.py')).read().split('def bmp_pixels')[0], g.__dict__)
W, H, cap = png_indices(sys.argv[1])
gw, gh, pix, stop = g.decode(open(sys.argv[2], 'rb').read())
d = [[cap[y * W + x] != pix[y * W + x] for x in range(W)] for y in range(H)]
# 8-connected components with a 3-pixel dilation, by flood fill on a coarse grid of 4x4 cells
C = 4
cells = {(x // C, y // C) for y in range(H) for x in range(W) if d[y][x]}
seen = set(); boxes = []
for c in cells:
    if c in seen: continue
    stack = [c]; seen.add(c); xs = []; ys = []
    while stack:
        cx, cy = stack.pop(); xs.append(cx); ys.append(cy)
        for ox in (-1, 0, 1):
            for oy in (-1, 0, 1):
                n = (cx + ox, cy + oy)
                if n in cells and n not in seen:
                    seen.add(n); stack.append(n)
    boxes.append((min(xs) * C, min(ys) * C, max(xs) * C + C - 1, max(ys) * C + C - 1))
total = sum(sum(r) for r in d)
print(f'{total} differing pixels in {len(boxes)} regions:')
for b in sorted(boxes, key=lambda b: (b[1], b[0])):
    n = sum(d[y][x] for y in range(b[1], b[3] + 1) for x in range(b[0], b[2] + 1))
    print(f'  x {b[0]:3d}..{b[2]:3d}  y {b[1]:3d}..{b[3]:3d}  ({n} pixels)')
