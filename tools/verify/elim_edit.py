# elim_edit.py IN OUT PHASE "n n ..." : the given nations lose every token and city (so f22B00 eliminates them):
# per occupied territory (+0x60/+0x61) its tokens go back to stock and its presence is cleared (area +1 - 1, area
# +0x01+n = 0, area +0x0E+n = 0, as f581C4); per city (+0x51/+0x52) the territory state is cleared (owner 0, round
# 0xFF as cal1.c 0x2122A, bit 0x01 off) and the city count is 0. Then the phase byte is set and the CRC rewritten.
import os, struct, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))   # python -I leaves the script's folder out
from hsave import crc32

NAT = lambda n: 0x55 + 199 * n
AREA = lambda t: 0xB37 + 69 * (t + 1)
TST = lambda t: 0x377D + 3 * t

d = bytearray(open(sys.argv[1], 'rb').read())
assert crc32(d[0x4B:-4]) == struct.unpack_from('<I', d, len(d) - 4)[0]
for n in map(int, sys.argv[4].split()):
    b = NAT(n)
    for j in range(d[b + 0x60]):
        t = d[b + 0x61 + j] - 1
        k = d[AREA(t) + 0x0E + n]
        d[AREA(t) + 0x0E + n] = 0
        d[b + 0x99] += k
        d[b + 0x98] -= k
        if d[AREA(t) + 1 + n]:
            d[AREA(t) + 1] -= 1
            d[AREA(t) + 1 + n] = 0
    d[b + 0x60] = 0
    for j in range(d[b + 0x51]):
        t = d[b + 0x52 + j] - 1
        d[TST(t)] = 0
        d[TST(t) + 1] = 0xFF
        d[TST(t) + 2] &= 0xFE
        d[b + 0x52 + j] = 0
    d[b + 0x51] = 0
    assert d[b + 0x98] == 0, (n, d[b + 0x98])
    print(f'nation {n}: no tokens, no cities; stock {d[b + 0x99]}')
d[0x395D] = int(sys.argv[3])
struct.pack_into('<I', d, len(d) - 4, crc32(d[0x4B:-4]))
open(sys.argv[2], 'wb').write(d)
