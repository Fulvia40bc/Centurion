"""Independent WAV reader (from the RIFF/WAVE notes, not from wav.c), for the Task 8 check.
Usage: wavpy.py ACIV.CT5 -> 'name rate channels frames sum' per .WAV in catalog order, same as wav_test,
where sum = 32-bit wrapping sum of the 16-bit samples as unsigned."""
import os, struct, sys
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from ct_list import read_dir  # noqa: E402  (tools/ct_list.py: the chained catalog directory)

d = open(sys.argv[1], 'rb').read()
data_of = {}
for name, off, ln, kind in read_dir(d):
    if not name.upper().endswith('.WAV'):
        continue
    b = d[off:off + ln]
    assert b[:4] == b'RIFF' and b[8:12] == b'WAVE'
    p, fmt, data = 12, None, None
    while p + 8 <= len(b):
        cid, n = b[p:p + 4], struct.unpack_from('<I', b, p + 4)[0]
        if cid == b'fmt ' and fmt is None: fmt = struct.unpack_from('<HHIIHH', b, p + 8)
        if cid == b'data' and data is None: data = b[p + 8:p + 8 + n]
        p += 8 + n + (n & 1)
    tag, ch, rate, _, align, bits = fmt
    assert tag == 1 and bits == 16 and align == 2 * ch
    frames = len(data) // align
    samples = struct.unpack_from('<%dh' % (frames * ch), data)
    print(name, rate, ch, frames, sum(s & 0xFFFF for s in samples) & 0xFFFFFFFF)
    data_of[name] = data[:frames * align]
same = {}
for n, x in data_of.items():
    same.setdefault(x, []).append(n)
for names in same.values():
    if len(names) > 1:
        print('# identical sample data:', ' '.join(names))
