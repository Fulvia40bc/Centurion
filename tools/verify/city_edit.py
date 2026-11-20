# city_edit.py IN OUT "n:t ..." [ast=n:pos] [phase=P] | IN --show N,N.. : build cities in a version-0x38 save the way CITY CONSTRUCTION's human
# path does (city.c 0x32365..0x3244B): territory state (owner n, round, bit 0x01), city list +0x52 (t + 1), +0x51 + 1,
# all of n's tokens in t back to stock (+0x99 += k, +0x98 -= k, area +0x0E+n = 0), then f581C4(t, n) (move.c): the
# presence flag area +0x01+n cleared, area +1 (nations there) - 1, t + 1 dropped from the occupied list +0x60/+0x61.
# With --show: each in-play nation's territories with their token counts and how many nations share each.
import os, struct, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))   # python -I leaves the script's folder out
from hsave import crc32

NAT = lambda n: 0x55 + 199 * n
AREA = lambda t: 0xB37 + 69 * (t + 1)
TST = lambda t: 0x377D + 3 * t
ROUND_OFF = 0x395D + 1          # int16 round right after the phase byte

d = bytearray(open(sys.argv[1], 'rb').read())
assert crc32(d[0x4B:-4]) == struct.unpack_from('<I', d, len(d) - 4)[0]

if sys.argv[2] == '--show':
    for n in map(int, sys.argv[3].split(',')):
        b = NAT(n)
        occ = [d[b + 0x61 + j] - 1 for j in range(d[b + 0x60])]
        print(n, 'cities', d[b + 0x51], 'tokens', d[b + 0x98], 'stock', d[b + 0x99],
              [(t, d[AREA(t) + 0x0E + n], d[AREA(t) + 1], d[AREA(t) + 1 + n]) for t in occ])
    sys.exit(0)

rnd = struct.unpack_from('<H', d, ROUND_OFF)[0]
for item in sys.argv[4:]:                           # ast=n:pos (AST square +0x5C), phase=P (the phase byte)
    key, val = item.split('=')
    if key == 'ast':
        n, pos = map(int, val.split(':'))
        d[NAT(n) + 0x5C] = pos
    elif key == 'phase':
        d[0x395D] = int(val)
    else:
        sys.exit(f'unknown option {item}')
for item in sys.argv[3].split():
    n, t = map(int, item.split(':'))
    b = NAT(n)
    assert d[b + 0x51] < 9 and d[TST(t) + 2] & 1 == 0, (n, t)
    k = d[AREA(t) + 0x0E + n]
    d[TST(t) + 2] |= 1
    d[TST(t)] = n
    d[TST(t) + 1] = rnd & 0xFF
    d[b + 0x52 + d[b + 0x51]] = t + 1
    d[b + 0x51] += 1
    d[AREA(t) + 0x0E + n] = 0
    d[b + 0x99] += k
    d[b + 0x98] -= k
    if d[AREA(t) + 1 + n] != 0:                     # f581C4: tokens are 0 now
        d[AREA(t) + 1] -= 1
        d[AREA(t) + 1 + n] = 0
        cnt = d[b + 0x60]
        lst = [d[b + 0x61 + j] for j in range(cnt)]
        keep = [v for v in lst if v != t + 1]
        # 0x58286: entries shift down; the last slot keeps its old byte (no clear), as the original
        for j in range(len(keep)):
            d[b + 0x61 + j] = keep[j]
        d[b + 0x60] = len(keep)
    print(f'nation {n}: city on {t} ({k} tokens back), round {rnd}')
struct.pack_into('<I', d, len(d) - 4, crc32(d[0x4B:-4]))
open(sys.argv[2], 'wb').write(d)
