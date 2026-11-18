"""Independent reference XMIDI sequencer (from the spec text only).
Usage: python -I refseq.py SRCDIR OUTDIR NAME.XMI..."""
import sys, struct, os
from collections import Counter

SECONDS = 300
T_END = SECONDS * 120

def find(b, want):
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

def parse(b, notes):
    """events in file order: dict(tick, st, d=[bytes], dur, kind)"""
    e = find(b, (b'EVNT',))[b'EVNT']; p = 0; tick = 0; ev = []
    while p < len(e):
        c = e[p]
        if c < 0x80: tick += c; p += 1; continue
        hi = c & 0xF0
        if hi == 0x90:
            k, v = e[p + 1], e[p + 2]; dur, p = vlq(e, p + 3)
            if v == 0: notes['note-on vel 0'] += 1
            if dur == 0: notes['note-on dur 0'] += 1
            ev.append(('on', tick, c, k, v, dur))
        elif hi in (0xC0, 0xD0):
            ev.append(('ch', tick, bytes([c, e[p + 1]]))); p += 2
        elif hi < 0xF0:
            if hi == 0x80: notes['explicit 8n note-off in stream'] += 1
            if hi == 0xB0 and 110 <= e[p + 1] <= 120:
                notes['ctl %d' % e[p + 1]] += 1
            ev.append(('ch', tick, bytes([c, e[p + 1], e[p + 2]]))); p += 3
        elif c in (0xF0, 0xF7):
            notes['sysex %02X' % c] += 1
            n, p = vlq(e, p + 1); p += n
        elif c == 0xFF:
            typ = e[p + 1]; n, p = vlq(e, p + 2); p += n
            if typ == 0x2F:
                ev.append(('end', tick)); break
            notes['meta FF %02X' % typ] += 1
        else:
            raise SystemExit('bad status %02X' % c)
    if p < len(e): notes['bytes after FF 2F: %d' % (len(e) - p)] += 1
    if not ev or ev[-1][0] != 'end': notes['no FF 2F'] += 1
    return ev

def run(ev, notes):
    out = []
    pending = {}            # (ch,key) -> due
    loops = []              # [for_index, count]
    shift = 0; i = 0; ended = False
    n_on = n_off = n_loops = n_trig = 0
    jumps_no_progress = 0; last_jump_t = None
    def emit(t, bs): out.append((t, bs))
    while True:
        te = None
        if not ended and i < len(ev):
            te = ev[i][1] + shift
        po = min(((d, ck) for ck, d in pending.items()), default=None)
        if po is not None and (te is None or po[0] <= te):
            if po[0] > T_END: break
            d, (ch, k) = po
            del pending[(ch, k)]
            emit(d, bytes([0x80 | ch, k, 0])); n_off += 1
            continue
        if te is None or te > T_END: break
        x = ev[i]; t = te
        kind = x[0]
        if kind == 'end':
            ended = True; continue
        if kind == 'on':
            _, _, c, k, v, dur = x; ch = c & 15
            if v == 0:
                notes['vel-0 note-on reached (emitted as-is, no note-off scheduled)'] += 1
                emit(t, bytes([c, k, v])); i += 1; continue
            if (ch, k) in pending:
                del pending[(ch, k)]
                emit(t, bytes([0x80 | ch, k, 0])); n_off += 1
            emit(t, bytes([c, k, v])); n_on += 1
            pending[(ch, k)] = t + dur
            i += 1; continue
        bs = x[2]
        if bs[0] & 0xF0 == 0xB0 and 110 <= bs[1] <= 120:
            cn, val = bs[1], bs[2]
            if cn == 116:
                if len(loops) < 4: loops.append([i, val])
                else: notes['5th nested FOR ignored'] += 1
                i += 1; continue
            if cn == 117:
                if val >= 64:
                    if loops:
                        L = loops[-1]
                        jump = False
                        if L[1] == 0: jump = True
                        else:
                            L[1] -= 1
                            jump = L[1] > 0
                        if jump:
                            shift = t - ev[L[0]][1]
                            i = L[0] + 1; n_loops += 1
                            if last_jump_t == t:
                                jumps_no_progress += 1
                                if jumps_no_progress > 100000:
                                    notes['ABORT: zero-length loop'] += 1; break
                            else:
                                jumps_no_progress = 0; last_jump_t = t
                            continue
                        loops.pop()
                    else:
                        notes['NEXT without open loop'] += 1
                else:
                    if loops: loops.pop()
                    else: notes['BREAK without open loop'] += 1
                i += 1; continue
            if cn == 119: n_trig += 1
            i += 1; continue
        emit(t, bs); i += 1
    for (ch, k) in sorted(pending):
        emit(T_END, bytes([0x80 | ch, k, 0])); n_off += 1
    return out, (n_on, n_off, n_loops, n_trig)

src, dst = sys.argv[1], sys.argv[2]
for name in sys.argv[3:]:
    notes = Counter()
    ev = parse(open(os.path.join(src, name), 'rb').read(), notes)
    out, (a, b_, c, d) = run(ev, notes)
    summ = '# notes_on %d notes_off %d loops %d triggers %d' % (a, b_, c, d)
    base = name.rsplit('.', 1)[0]
    with open(os.path.join(dst, base + '.txt'), 'w', newline='\n') as f:
        for t, bs in out: f.write('%d %s\n' % (t, ' '.join('%02X' % x for x in bs)))
        f.write(summ + '\n')
    print(base, summ, '|', '; '.join('%s x%d' % kv for kv in sorted(notes.items())))
