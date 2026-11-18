"""Is the main menu's background a piece of PAL.PAL (1716x568)? Search PAL.PAL for capture rows."""
import os, sys
here = os.path.dirname(os.path.abspath(__file__))
exec(open(os.path.join(here, 'fontfind.py')).read().split('def load_font')[0])
g = type(sys)('gifpy')
exec(open(os.path.join(here, 'gifpy.py')).read().split('def bmp_pixels')[0], g.__dict__)
W, H, cap = png_indices(sys.argv[1])
pw, ph, pal, stop = g.decode(open('C:/work/advciv_play/PAL.PAL', 'rb').read())
print('PAL.PAL', pw, ph, stop, len(pal))
for (cx, cy) in ((10, 300), (600, 250), (100, 440)):
    probe = cap[cy * W + cx:cy * W + cx + 24]
    hits = []
    for y in range(ph):
        row = pal[y * pw:(y + 1) * pw]
        i = row.find(probe)
        while i >= 0:
            hits.append((i - cx, y - cy)); i = row.find(probe, i + 1)
    print(f'probe at capture ({cx},{cy}): offsets in PAL.PAL', hits[:5])
