"""dsclassify.py DOSBOX_DS.bin PORT_DS.bin
After dscompare's relocation undo, sort the differing bytes by who touches them: each data address referenced by an
instruction of the GAME code (< 0x87000) or only by LIBRARY code (Fastgraph, AIL, Watcom runtime, >= 0x87000), as
direct memory operands or immediates in AC_MAIN.asm. A differing run is "game" if a game reference falls inside it
or within 64 bytes before it (a table base); "library" if only library references do; "none" if neither (stack,
buffers reached through pointers)."""
import bisect, os, re, struct, sys

here = os.path.dirname(os.path.abspath(__file__))
src = open(os.path.join(here, 'dscompare.py')).read()
# reuse dscompare's relocation undo: run its top part on our arguments
ns = {'__name__': 'dscompare_lib', '__file__': os.path.join(here, 'dscompare.py')}
sys_argv = sys.argv[:]
exec(src.split('design = []')[0], ns)
a, b, n = ns['a'], ns['b'], ns['n']
DATA_LINK = ns['DATA_LINK']
for o in range(0, n - 3):   # run-time pointers, as dscompare
    if a[o:o + 4] != b[o:o + 4]:
        va, vb = struct.unpack_from('<I', a, o)[0], struct.unpack_from('<I', b, o)[0]
        if va - vb in (ns['DD'], ns['CD']):
            a[o:o + 4] = b[o:o + 4]

game, lib = set(), set()
pat = re.compile(r'0x([0-9a-f]{5,6})\b')
for line in open(os.path.join(here, '..', '..', 'disasm', 'AC_MAIN.asm')):
    m = re.match(r'^  ([0-9A-F]{8})\s+(\S+)\s*(.*)$', line)
    if not m or m.group(2).startswith(('call', 'j', 'db', 'dd', 'loop')):
        continue
    ins = int(m.group(1), 16)
    for v in pat.findall(m.group(3).split(';')[0]):
        v = int(v, 16)
        if DATA_LINK <= v < DATA_LINK + n:
            (game if ins < 0x87000 else lib).add(v)
gl, ll = sorted(game), sorted(lib)

def near(sorted_refs, s, e, before=64):
    i = bisect.bisect_left(sorted_refs, s - before)
    return i < len(sorted_refs) and sorted_refs[i] <= e

diff = [i for i in range(n) if a[i] != b[i]]
runs = []
for i in diff:
    if runs and i - runs[-1][1] <= 4:
        runs[-1][1] = i
    else:
        runs.append([i, i])
cls = {'game': [], 'library': [], 'none': []}
for s, e in runs:
    S, E = s + DATA_LINK, e + DATA_LINK
    if near(gl, S, E):
        cls['game'].append((S, E))
    elif near(ll, S, E):
        cls['library'].append((S, E))
    else:
        cls['none'].append((S, E))
for k, v in cls.items():
    print(f'{k:8}: {len(v):4d} runs, {sum(E - S + 1 for S, E in v):6d} bytes')
for k in ('game', 'none'):
    print(f'--- {k} runs:')
    for S, E in cls[k]:
        o = S - DATA_LINK
        print(f'  {S:06X}..{E:06X} ({E - S + 1:4d})  dosbox {bytes(a[o:o + min(16, E - S + 1)]).hex()}  '
              f'port {b[o:o + min(16, E - S + 1)].hex()}')
