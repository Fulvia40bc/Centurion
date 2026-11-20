# cal_edit.py IN OUT "n:id[:giver] ..." : a version-0x38 save at the calamity phase (phase byte 15) with the given
# calamity cards added to nation n's hand (+0x13 count, +0x14 ids, +0x1C givers; giver 0xFF = drawn).
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))   # python -I leaves the script's folder out
import struct
from hsave import crc32

src, dst, spec = sys.argv[1], sys.argv[2], sys.argv[3]
d = bytearray(open(src, 'rb').read())
assert crc32(d[0x4B:-4]) == struct.unpack_from('<I', d, len(d) - 4)[0]
PHASE = 0x395D
assert d[PHASE] in (12, 15), d[PHASE]
d[PHASE] = 15
for item in spec.split():
    f = item.split(':')
    n, cid = int(f[0]), int(f[1], 16)
    giver = int(f[2]) if len(f) > 2 else 0xFF
    b = 0x55 + 199 * n
    c = d[b + 0x13]
    assert c < 8
    d[b + 0x14 + c] = cid
    d[b + 0x1C + c] = giver
    d[b + 0x13] = c + 1
struct.pack_into('<I', d, len(d) - 4, crc32(d[0x4B:-4]))
open(dst, 'wb').write(d)
print('written', dst)
