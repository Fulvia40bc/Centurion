"""diffpng.py A.png B.png: where two 640x480 indexed frames differ, grouped into rectangles, with the colours."""
import collections, os, sys
here = os.path.dirname(os.path.abspath(__file__))
exec(open(os.path.join(here, 'fontfind.py')).read().split('def load_font')[0])
W, H, a = png_indices(sys.argv[1])
_, _, b = png_indices(sys.argv[2])
d = [[a[y * W + x] != b[y * W + x] for x in range(W)] for y in range(H)]
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
print(sum(map(sum, d)), 'differing pixels in', len(boxes), 'regions')
for x0, y0, x1, y1 in sorted(boxes, key=lambda b: (b[1], b[0])):
    pairs = collections.Counter((a[y * W + x], b[y * W + x]) for y in range(y0, y1 + 1) for x in range(x0, x1 + 1) if d[y][x])
    print(f'  x {x0:3d}..{x1:3d} y {y0:3d}..{y1:3d}: {sum(pairs.values()):4d} px, (A,B) colours {pairs.most_common(4)}')
