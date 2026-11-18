"""dscompare.py DOSBOX_DS.bin PORT_DS.bin [DIFF_BY_DESIGN.txt]
Compare a DOSBox-X dump of the data object (MEMDUMPBIN 0188:2A9000 72C00) with the port's (--dump-ds).

DOS/4GW relocates every pointer of the data object to its run-time address (data +0x1F9000, code +0x202000); the port
keeps link addresses. So at each fixup site of the data object (disasm/le/fixups.txt) the DOSBox-X dword is turned
back into a link address before comparing. Pointers the game computes at run time and stores elsewhere are found the
same way afterwards: a 4-byte difference whose DOSBox value minus the port value is exactly one of the two deltas.

DIFF_BY_DESIGN.txt: lines "ADDR LEN reason" (hex address, decimal or 0x length): ranges expected to differ (heap
handles, timers, mouse and sound state); listed separately, not counted as errors."""
import os, struct, sys

DATA_LINK, DATA_RUN, CODE_LINK, CODE_RUN = 0xB0000, 0x2A9000, 0x10000, 0x212000
DD, CD = DATA_RUN - DATA_LINK, CODE_RUN - CODE_LINK
here = os.path.dirname(os.path.abspath(__file__))
fix = os.path.join(here, '..', '..', 'disasm', 'le', 'fixups.txt')

a = bytearray(open(sys.argv[1], 'rb').read())
b = open(sys.argv[2], 'rb').read()
assert len(a) == len(b), (len(a), len(b))
n = len(a)
sites = []
for line in open(fix):
    f = line.split()
    s, t = int(f[0], 16), int(f[2], 16) if f[1] == 'off32' else None
    if t is not None and DATA_LINK <= s < DATA_LINK + n - 3:
        sites.append(s)
undone = 0
for s in sites:
    o = s - DATA_LINK
    v = struct.unpack_from('<I', a, o)[0]
    for lo, hi, delta in ((DATA_RUN, DATA_RUN + n, DD), (CODE_RUN, CODE_RUN + 0x960D1, CD)):
        if lo <= v < hi:
            struct.pack_into('<I', a, o, v - delta)
            undone += 1
            break
design = []
if len(sys.argv) > 3:
    for line in open(sys.argv[3]):
        f = line.split(None, 2)
        if len(f) >= 2 and not line.startswith('#'):
            design.append((int(f[0], 16), int(f[1], 0), f[2].strip() if len(f) > 2 else ''))
# run-time pointers outside the fixup sites
computed = 0
for o in range(0, n - 3):
    if a[o:o + 4] != b[o:o + 4]:
        va, vb = struct.unpack_from('<I', a, o)[0], struct.unpack_from('<I', b, o)[0]
        if va - vb in (DD, CD) and (DATA_LINK <= vb < DATA_LINK + n or CODE_LINK <= vb < CODE_LINK + 0x960D1):
            a[o:o + 4] = b[o:o + 4]
            computed += 1
diff = [i for i in range(n) if a[i] != b[i]]
def why(i):
    addr = i + DATA_LINK
    for s, l, r in design:
        if s <= addr < s + l:
            return r
    return None
runs = []
for i in diff:
    if runs and i - runs[-1][1] <= 4 and why(i) == runs[-1][2]:
        runs[-1][1] = i
    else:
        runs.append([i, i, why(i)])
unexpected = [r for r in runs if r[2] is None]
print(f'{len(sites)} fixup sites in the data object, {undone} relocated pointers turned back, {computed} run-time '
      f'pointers found outside them')
print(f'{len(diff)} bytes differ after that, in {len(runs)} runs: {len(runs) - len(unexpected)} by design, '
      f'{len(unexpected)} unexplained ({sum(e - s + 1 for s, e, _ in unexpected)} bytes)')
for s, e, r in runs:
    if r is None:
        print(f'  {s + DATA_LINK:06X}..{e + DATA_LINK:06X} ({e - s + 1:3d})  dosbox {bytes(a[s:e + 1])[:16].hex()}  '
              f'port {b[s:e + 1][:16].hex()}')
by = {}
for s, e, r in runs:
    if r is not None:
        by.setdefault(r, 0)
        by[r] += e - s + 1
for r, c in by.items():
    print(f'  by design: {c:6d} bytes  {r}')
