"""Continue a captured naval battle in the port from one DOSBox breakpoint to the next and compare (Claude, 2026-11-10).

usage (from the game folder):
  python cmp_naval_resume.py [--exe EXE] [--original] SNAP VGA FROM UNTIL BP SP REF [SCRIPT]
SNAP, VGA = the snapshot (DS = 1686, work buffer 2687:0000) and screen at breakpoint FROM; BP / SP = from its .txt (hex);
REF = the snapshot at breakpoint UNTIL (its screen = REF with _vga before .bin); SCRIPT = input for the port (screen.c
script lines: the flagship menu click, Esc, waits). Runs centurion --naval-resume (naval.c naval_test_resume) and compares
the DS image byte by byte and the screen pixel by pixel (palette indexes).
DS bytes the port does not keep as the EXE does are listed per class and do not count; any other byte counts. The clock
class's bytes (the tick DS:42 and the timer slots 0..15 at DS:46 + 4n, NAVAL_INDEX nv_t_start_00D7) are printed one by
one, so the reader sees which slots ended as the game's.
Then the run is repeated with the snapshot's work buffer and the input screen filled with FD and with FE: the pixels that
come out the same in both were drawn by the port itself; the others are the input picture (the DOSBox capture) passed
through, and their match with the game's screen says nothing about the port.
"""
import subprocess, sys, os, tempfile
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cmp_naval_frame import groups, EXE, filled_inputs, drawn_count

# (first, last, class): DS ranges whose contents are not the game's state in the port (each checked by hand, see
# tools/dosbox/results/naval/README.md)
CLASSES = [
    (0x0042, 0x0085, 'clock: tick count DS:42 and the 16 timer slots (time a box waited for a click; DOSBox frames slower than the wait)'),
    (0x0086, 0x00C7, 'interrupt handlers: timer divider [8A], keyboard [A2..A5], mouse [B2..B8] (input)'),
    (0x0680, 0x068F, 'mouse cursor (1BE6/1D55/1D8A, not ported: the system pointer)'),
    (0xA674, 0xA691, 'mouse cursor visibility stack (1D55 pushes [686] to A674 + [68A], 1D8A pops; depth < 30; not ported)'),
    (0x9812, 0x9817, 'intro water-colour phase (C1BB: time)'),
    (0x9AFA, 0x9AFB, 'draw colour left by the EXE menu / message box (screen.c draws them without DS)'),
    (0x9B48, 0x9B50, 'state of the EXE menu / message box drawing'),
    (0x9B69, 0x9B69, 'ink left by the EXE menu / message box'),
    (0xA3C0, 0xA3CF, 'memory allocator'),
    (0xA6B0, 0xA6BF, 'music slot handles: song segments (the port writes 1)'),
    (0xAC60, 0xBA5F, 'archive decode buffer (also the message text buffer AC60: compared by eye when no load came between)'),
    (0xC04E, 0xC05F, 'archive loader scratch'),
    (0xC0CE, 0xC0CF, 'sprite sheet loader: descriptor count'),
    (0xC3A2, 0xC3BD, 'sheet handles / COLLIDE pointers (far pointers; the port keeps its own handles)'),
    (0x9956, 0x9957, 'shark sheet handle (far pointer)'),
    (0xCA00, 0xCDFF, 'the stack below SP and the callers\' frames (no DOS stack in the port; cursor save area CC9E..CCB9)'),
    (0xCF00, 0xFFFF, 'heap: sheets, archive directories'),
]


def ds_of(path):
    d = open(path, 'rb').read()
    return d if len(d) == 0x10000 else d[0x16860:0x26860]


def resume(exe, orig, snap, vga, args, ods, ovga):
    cmd = [exe] + (['--original'] if orig else []) + ['--naval-resume', snap, vga] + args[:4] + [ods, ovga] + args[5:6]
    return subprocess.run(cmd, capture_output=True, text=True)


def main():
    argv = sys.argv[1:]
    exe, orig = EXE, False
    if '--exe' in argv:
        i = argv.index('--exe'); exe = argv[i + 1]; del argv[i:i + 2]
    if '--original' in argv:
        argv.remove('--original'); orig = True
    snap, vga, ref = argv[0], argv[1], argv[6]
    args = argv[2:6] + [ref] + argv[7:8]                  # FROM UNTIL BP SP REF [SCRIPT]
    tmp = tempfile.gettempdir()
    ods, ovga = os.path.join(tmp, 'naval_resume_ds.bin'), os.path.join(tmp, 'naval_resume_vga.bin')
    r = resume(exe, orig, snap, vga, args, ods, ovga)
    sys.stdout.write(r.stderr)
    if r.returncode != 0:
        print('centurion failed (%d)' % r.returncode)
        return 1
    a, b = ds_of(ods), ds_of(ref)
    per, other = {}, []
    for i in range(0x10000):
        if a[i] == b[i]:
            continue
        for lo, hi, what in CLASSES:
            if lo <= i <= hi:
                per.setdefault(what, []).append(i)
                break
        else:
            other.append(i)
    for what, l in per.items():
        print('  %4d bytes  %s' % (len(l), what))
    for w in sorted({i & ~1 for i in per.get(CLASSES[0][2], [])}):
        name = 'tick' if w < 0x46 else 'slot %d %s' % ((w - 0x46) // 4, 'lo' if (w - 0x46) % 4 == 0 else 'hi')
        print('      DS %04X %-10s port %04X  game %04X' % (w, name, a[w] | a[w + 1] << 8, b[w] | b[w + 1] << 8))
    i = 0
    while i < len(other):
        j = i
        while j + 1 < len(other) and other[j + 1] == other[j] + 1:
            j += 1
        lo, hi = other[i], other[j]
        print('  DS %04X..%04X  port %s  game %s' % (lo, hi, a[lo:hi + 1].hex(), b[lo:hi + 1].hex()))
        i = j + 1
    print('DS: %d game-state bytes differ' % len(other))
    rv = ref[:-4] + '_vga.bin'
    game = open(rv, 'rb').read()[:64000]
    n, gs = groups(open(ovga, 'rb').read(), game)
    print('screen: %d differing pixels' % n)
    for g in gs[:20]:
        print('  %5d px  (%d,%d)-(%d,%d)' % g)

    def produce(colour):
        s2, v2 = filled_inputs(snap, vga, colour, 'resume')
        return open(ovga, 'rb').read() if resume(exe, orig, s2, v2, args, ods, ovga).returncode == 0 else None
    drawn_count(produce, game)
    return 0 if not other else 1


if __name__ == '__main__':
    sys.exit(main())
