"""Check the port's naval-battle pictures against the game's screen (Claude, 2026-11-10).

usage (from the game folder):
  python cmp_naval_frame.py [--exe EXE] [--original] SNAP VGA MODE [A B C] [--ref REF_VGA]
SNAP = a DOSBox snapshot of tools/dosbox/results/naval (DS = 1686, work buffer 2687:0000), VGA = the screen saved with it.
MODE (centurion --naval-frame, naval.c naval_test_frame):
  duel E R      B12A's frame from the snapshot's state (into the work buffer)
  intro K       GALLEYS.LZW with pennant frame K (-1 = none)
  after BITS E R   the screen a finished battle leaves (BEAE panel, end of D3F2, D60C, DC8E) from the final state
  check         no picture: C4F8 / C641 / CF7F / CFDD / CE7E against the snapshot's values
Prints the number of differing pixels (palette indexes) and the bounding box of each connected group (8-neighbour).
Each picture mode is then run twice more with the snapshot's work buffer and the input screen filled with FD and with FE
(drawn_count): a pixel the port drew itself is the same in both runs (whatever colour it drew, FD and FE included); the
others come from the input picture, i.e. from the DOSBox capture, and say nothing about the port.
"""
import subprocess, sys, os, tempfile

EXE = r'C:\work\Centurion-native\centurion.exe'


def groups(a, b):
    pts = {i for i in range(64000) if a[i] != b[i]}
    seen, out = set(), []
    for p in sorted(pts):
        if p in seen:
            continue
        st, n = [p], 0
        seen.add(p)
        x0 = x1 = p % 320
        y0 = y1 = p // 320
        while st:
            q = st.pop()
            n += 1
            x, y = q % 320, q // 320
            x0, x1, y0, y1 = min(x0, x), max(x1, x), min(y0, y), max(y1, y)
            for dx in (-1, 0, 1):
                for dy in (-1, 0, 1):
                    nx, ny = x + dx, y + dy
                    r = ny * 320 + nx
                    if 0 <= nx < 320 and 0 <= ny < 200 and r in pts and r not in seen:
                        seen.add(r)
                        st.append(r)
        out.append((n, x0, y0, x1, y1))
    return len(pts), out


WRK = 0x26870          # the work buffer 2687:0000 in a snapshot (64000 bytes)


def filled_inputs(snap, vga, colour, tag):
    """copies of SNAP and VGA with the work buffer and the screen filled with COLOUR (in the temp folder)"""
    tmp = tempfile.gettempdir()
    d = bytearray(open(snap, 'rb').read())
    d[WRK:WRK + 64000] = bytes([colour]) * 64000
    s2, v2 = os.path.join(tmp, 'naval_%s_%02X.bin' % (tag, colour)), os.path.join(tmp, 'naval_%s_%02X_vga.bin' % (tag, colour))
    open(s2, 'wb').write(d)
    open(v2, 'wb').write(bytes([colour]) * 64000)
    return s2, v2


def drawn_count(produce, ref):
    """produce(colour) -> the port's 64000-pixel picture with both input pictures filled with colour; prints how many
    pixels the port drew itself (the same with FD and FE under them) and how many of those differ from REF"""
    a, b = produce(0xFD), produce(0xFE)
    if a is None or b is None:
        print('drawn count: centurion failed')
        return
    drawn = [i for i in range(64000) if a[i] == b[i]]
    print('the port draws %d pixels itself, %d of them differ; %d come from the input picture (the capture)'
          % (len(drawn), sum(1 for i in drawn if a[i] != ref[i]), 64000 - len(drawn)))
    rest = [i for i in range(64000) if a[i] != b[i]]
    if rest:
        print('  from the input: (%d,%d)-(%d,%d)' % (min(i % 320 for i in rest), rest[0] // 320,
                                                     max(i % 320 for i in rest), rest[-1] // 320))


def run(exe, orig, args, out):
    cmd = [exe] + (['--original'] if orig else []) + ['--naval-frame', args[0], args[1], out] + args[2:]
    r = subprocess.run(cmd, capture_output=True, text=True)
    sys.stdout.write(r.stderr)
    return r.returncode


def main():
    argv = sys.argv[1:]
    exe, orig, ref = EXE, False, None
    if '--exe' in argv:
        i = argv.index('--exe'); exe = argv[i + 1]; del argv[i:i + 2]
    if '--original' in argv:
        argv.remove('--original'); orig = True
    if '--ref' in argv:
        i = argv.index('--ref'); ref = argv[i + 1]; del argv[i:i + 2]
    out = os.path.join(tempfile.gettempdir(), 'naval_frame.bin')
    rc = run(exe, orig, argv, out)
    if argv[2] == 'check':
        print('geometry %s' % ('same as the game' if rc == 0 else 'DIFFERS'))
        return rc
    if rc != 0:
        print('centurion failed (%d)' % rc)
        return 1
    a = open(out, 'rb').read()
    b = open(ref or argv[1], 'rb').read()[:64000]
    n, gs = groups(a, b)
    print('%d differing pixels' % n)
    for g in gs[:40]:
        print('  %5d px  (%d,%d)-(%d,%d)' % g)
    def produce(colour):
        s2, v2 = filled_inputs(argv[0], argv[1], colour, 'frame')
        return open(out, 'rb').read() if run(exe, orig, [s2, v2] + argv[2:], out) == 0 else None
    drawn_count(produce, b)
    return 0 if n == 0 else 1


if __name__ == '__main__':
    sys.exit(main())
