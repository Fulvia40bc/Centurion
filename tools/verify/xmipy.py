"""Independent XMIDI reader (from the XMIDI format notes, not from xmi.c).
Usage: xmipy.py DIR NAME...       -> 'name events notes last_tick timb_count' per file
       xmipy.py DIR --dump NAME   -> 'tick status d1 d2' lines (note-offs before note-ons at equal ticks)"""
import sys, struct

def find(b, want):
    """depth-first scan for chunk ids, ignoring container sizes (some files undersize FORM XDIR)."""
    found = {}
    p = 0
    while p + 8 <= len(b):
        cid = b[p:p + 4]; n = struct.unpack_from('>I', b, p + 4)[0]
        if cid in (b'FORM', b'CAT ', b'LIST'): p += 12; continue
        if cid in want and cid not in found: found[cid] = b[p + 8:p + 8 + n]
        p += 8 + n + (n & 1)
    return found

def vlq(b, p):
    v = 0
    while True:
        c = b[p]; p += 1; v = (v << 7) | (c & 0x7F)
        if not c & 0x80: return v, p

def parse(b):
    f = find(b, (b'TIMB', b'EVNT'))
    t = f.get(b'TIMB', b'')
    ntimb = min(struct.unpack_from('<H', t, 0)[0], 64) if t else 0
    e = f[b'EVNT']; p = 0; tick = 0; ev = []; seq = 0
    while p < len(e):
        c = e[p]
        if c < 0x80: tick += c; p += 1; continue
        hi = c & 0xF0
        if hi == 0x90:
            k, v = e[p + 1], e[p + 2]; dur, p = vlq(e, p + 3)
            ev.append((tick, 1, seq, c, k, v)); seq += 1
            ev.append((tick + dur, 0, seq, 0x80 | (c & 15), k, 0)); seq += 1
        elif hi in (0xC0, 0xD0):
            ev.append((tick, 1, seq, c, e[p + 1], 0)); seq += 1; p += 2
        elif hi < 0xF0:
            ev.append((tick, 1, seq, c, e[p + 1], e[p + 2])); seq += 1; p += 3
        elif c in (0xF0, 0xF7):
            n, p = vlq(e, p + 1); p += n
        elif c == 0xFF:
            typ = e[p + 1]; n, p = vlq(e, p + 2); p += n
            if typ == 0x2F: ev.append((tick, 1, seq, 0xFF, 0x2F, 0)); break
        else: raise SystemExit('bad status %02X' % c)
    ev.sort(key=lambda x: (x[0], x[1], x[2]))
    return ev, ntimb

d = sys.argv[1]
if sys.argv[2] == '--dump':
    ev, _ = parse(open(f'{d}/{sys.argv[3]}', 'rb').read())
    for x in ev: print(x[0], '%02X %02X %02X' % x[3:])
else:
    for n in sys.argv[2:]:
        ev, nt = parse(open(f'{d}/{n}', 'rb').read())
        notes = sum(1 for x in ev if x[3] & 0xF0 == 0x90)
        print(n, len(ev), notes, ev[-1][0], nt)
