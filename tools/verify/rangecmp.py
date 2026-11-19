"""rangecmp.py A.bin B.bin RANGE...: compare two data-object dumps (link base 0xB0000) over the given ranges only.
RANGE is ADDR:LEN (hex address, length hex or decimal), or dlg:ADDR for a dialog (int16 count, then 20-byte records)
compared field by field except the label pointer at +0x08 (a run-time address in a DOSBox-X dump, a link address in
the port's). Prints each differing byte as address, A, B; exits 1 if any differ."""
import sys

BASE = 0xB0000
a = open(sys.argv[1], 'rb').read()
b = open(sys.argv[2], 'rb').read()
diffs = []
for spec in sys.argv[3:]:
    kind, _, rest = spec.partition(':')
    if kind == 'dlg':
        addr = int(rest, 16)
        n = a[addr - BASE] | a[addr - BASE + 1] << 8
        nb = b[addr - BASE] | b[addr - BASE + 1] << 8
        if n != nb:
            diffs.append((addr, n, nb, 'record count'))
        for r in range(min(n, nb)):
            rec = addr + 2 + 20 * r
            for off in list(range(0, 8)) + list(range(12, 20)):
                x, y = a[rec + off - BASE], b[rec + off - BASE]
                if x != y:
                    diffs.append((rec + off, x, y, 'record %d +0x%X' % (r, off)))
        print('%-16s %d records' % (spec, n))
    else:
        addr = int(kind, 16)
        length = int(rest, 0)
        d = [(addr + i, a[addr + i - BASE], b[addr + i - BASE], '') for i in range(length)
             if a[addr + i - BASE] != b[addr + i - BASE]]
        diffs += d
        print('%-16s %d bytes, %d differ' % (spec, length, len(d)))
for addr, x, y, what in diffs[:40]:
    print('  0x%06X  %02X  %02X  %s' % (addr, x, y, what))
print('%d differing bytes' % len(diffs))
sys.exit(1 if diffs else 0)
