"""Standard MIDI file (type 0) -> 'tick HH HH [HH]' lines, channel messages only, in file order.
Checks the header (type 0, 1 track, division 120), the tempo event (1,000,000 us/quarter at tick 0) and end of track.
Usage: mid2txt.py IN.mid OUT.txt"""
import struct, sys

def vlq(b, p):
    v = 0
    while True:
        c = b[p]; p += 1; v = (v << 7) | (c & 0x7F)
        if not c & 0x80:
            return v, p

b = open(sys.argv[1], 'rb').read()
assert b[:4] == b'MThd' and struct.unpack_from('>IHHH', b, 4) == (6, 0, 1, 120), 'header'
assert b[14:18] == b'MTrk'
n = struct.unpack_from('>I', b, 18)[0]
t = b[22:22 + n]
assert len(t) == n, 'track length'
p = tick = 0; run = None; out = []; tempo = eot = None
while p < len(t):
    d, p = vlq(t, p); tick += d
    c = t[p]
    if c == 0xFF:
        typ = t[p + 1]; ln, q = vlq(t, p + 2); data = t[q:q + ln]; p = q + ln
        if typ == 0x51: tempo = (tick, int.from_bytes(data, 'big'))
        if typ == 0x2F: eot = tick; break
        continue
    if c in (0xF0, 0xF7):
        ln, q = vlq(t, p + 1); p = q + ln; continue
    if c & 0x80:
        run = c; p += 1
    elif run is None:
        raise SystemExit('running status without status')
    k = 1 if run & 0xF0 in (0xC0, 0xD0) else 2
    out.append('%d %s' % (tick, ' '.join('%02X' % x for x in (run,) + tuple(t[p:p + k]))))
    p += k
assert tempo == (0, 1000000), tempo
assert eot is not None and p == len(t), 'end of track'
open(sys.argv[2], 'w', newline='\n').write('\n'.join(out) + '\n')
print(sys.argv[1], len(out), 'messages, end of track at', eot)
