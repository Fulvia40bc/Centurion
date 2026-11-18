"""rows.py CAPTURE.png COLOUR X0 X1 Y0 Y1: per row, the span of pixels in COLOUR within the box"""
import os, sys
src = open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'fontfind.py')).read()
exec(src.split('W, H, img = png_indices')[0])
W, H, img = png_indices(sys.argv[1])
c, x0, x1, y0, y1 = map(int, sys.argv[2:7])
for y in range(y0, y1 + 1):
    xs = [x for x in range(x0, x1 + 1) if img[y * W + x] == c]
    if xs:
        print('row', y, len(xs), 'pixels, from', min(xs), 'to', max(xs))
