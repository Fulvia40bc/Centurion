"""rectdiff.py A.png B.png X0 X1 Y0 Y1 [...]: differing pixels of two 640x480 indexed frames inside the given
rectangles only (inclusive bounds, as Fastgraph gives them), with the first few positions and colour pairs."""
import os, sys
here = os.path.dirname(os.path.abspath(__file__))
exec(open(os.path.join(here, 'fontfind.py')).read().split('def load_font')[0])
W, H, a = png_indices(sys.argv[1])
_, _, b = png_indices(sys.argv[2])
r = [int(v, 0) for v in sys.argv[3:]]
total = 0
for k in range(0, len(r), 4):
    x0, x1, y0, y1 = r[k:k + 4]
    d = [(x, y, a[y * W + x], b[y * W + x]) for y in range(y0, y1 + 1) for x in range(x0, x1 + 1) if a[y * W + x] != b[y * W + x]]
    total += len(d)
    print('x %d..%d y %d..%d: %d of %d px differ' % (x0, x1, y0, y1, len(d), (x1 - x0 + 1) * (y1 - y0 + 1)), d[:8])
sys.exit(1 if total else 0)
