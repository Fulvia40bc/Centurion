"""Independent implementation of the TASK 10 mixer rules (docs/HANDOFF.md), written from the spec, not from mixer.c.
Usage: mixpy.py ACIV.CT5 OUT.wav -> the same 6-second scene as port/test/mixer_test.c; prints 'frames sumL sumR clipped'."""
import os, struct, sys
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from ct_list import read_dir  # noqa: E402

d = open(sys.argv[1], 'rb').read()
ent = {n: (o, l) for n, o, l, k in read_dir(d)}

def wav(name):
    o, l = ent[name]
    b = d[o:o + l]
    p, fmt, data = 12, None, None
    while p + 8 <= len(b):
        cid, n = b[p:p + 4], struct.unpack_from('<I', b, p + 4)[0]
        if cid == b'fmt ' and fmt is None: fmt = struct.unpack_from('<HHIIHH', b, p + 8)
        if cid == b'data' and data is None: data = b[p + 8:p + 8 + n]
        p += 8 + n + (n & 1)
    tag, ch, rate, _, align, bits = fmt
    frames = len(data) // align
    return rate, ch, frames, struct.unpack_from('<%dh' % (frames * ch), data)

def i32(x):
    return ((x + 0x80000000) & 0xFFFFFFFF) - 0x80000000

def tdiv(a, b):   # C division, truncating toward zero
    q = abs(a) // abs(b)
    return q if (a >= 0) == (b >= 0) else -q

def gains(pan):
    gl = 128 if pan <= 64 else ((127 - pan) * 128) // 63
    gr = 128 if pan >= 64 else (pan * 128) // 64
    return gl, gr

scene = [(0, 'MARKETPL.WAV', 127, 64), (22050, 'KABOOM3.WAV', 100, 0), (44100, 'BRPC.WAV', 90, 127),
         (88200, 'MARKETPL.WAV', 127, 32)]
sounds = {n: wav(n) for _, n, _, _ in scene}
TOTAL = 6 * 44100
voices = []
out = []
clipped = 0
for t in range(TOTAL):
    for start, n, vol, pan in scene:
        if start == t:
            rate, ch, frames, s = sounds[n]
            voices.append(dict(rate=rate, ch=ch, frames=frames, s=s, vol=vol, pan=pan, p=0,
                               step=(rate << 32) // 44100, live=True))
    L = R = 0
    for v in voices:
        if not v['live']:
            continue
        i = v['p'] >> 32
        if i >= v['frames']:
            v['live'] = False
            continue
        f = (v['p'] >> 16) & 0xFFFF
        vals = []
        for c in range(v['ch']):
            a = v['s'][i * v['ch'] + c]
            nb = v['s'][(i + 1) * v['ch'] + c] if i + 1 < v['frames'] else a
            vals.append(a + (i32((nb - a) * f) >> 16))
        if v['ch'] == 1:
            gl, gr = gains(v['pan'])
            L += tdiv(vals[0] * v['vol'] * gl, 127 * 128)
            R += tdiv(vals[0] * v['vol'] * gr, 127 * 128)
        else:
            L += tdiv(vals[0] * v['vol'] * 128, 127 * 128)
            R += tdiv(vals[1] * v['vol'] * 128, 127 * 128)
        v['p'] += v['step']
    for x in (L, R):
        if x > 32767 or x < -32768:
            clipped += 1
        out.append(max(-32768, min(32767, x)))
pcm = struct.pack('<%dh' % len(out), *out)
hdr = b'RIFF' + struct.pack('<I', 36 + len(pcm)) + b'WAVEfmt ' + struct.pack('<IHHIIHH', 16, 1, 2, 44100, 44100 * 4, 4, 16) \
      + b'data' + struct.pack('<I', len(pcm))
open(sys.argv[2], 'wb').write(hdr + pcm)
print(TOTAL, sum(out[0::2]), sum(out[1::2]), clipped)
