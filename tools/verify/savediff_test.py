"""Tests for savediff.py, all on synthetic saves (no game data). Run: python -I savediff_test.py

Builds a 44,062-byte version-0x38 save with a checking CRC (as TASK 14's synthetic saves), then checks:
self-compare; every single-byte change 2..0xAC19 (one line each, naming the element that holds the byte);
two changes in one record; an s16 high-byte change; a description change; the 44,066 end-game shape; the
refusals (version 0x37, size 44,061, PBEM bit) and a bad CRC (reported, not refused); --ignore/--max/--layout;
and the three save editors (cal_edit.py, city_edit.py, elim_edit.py) run as subprocesses on a synthetic save
that meets their asserts: savediff must give exactly one line per element holding a byte where IN and OUT
differ, and every such element must be one the editor names.
"""
import os
import struct
import subprocess
import sys
import tempfile
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)                       # python -I leaves the script's folder out
import savediff
from hsave import crc32

SIZE, SIZE_END, CRC_OFF = savediff.SIZE, savediff.SIZE_END, savediff.CRC_OFF
NAT = lambda n: 0x55 + 199 * n
AREA = lambda t: 0xB37 + 69 * (t + 1)
TST = lambda t: 0x377D + 3 * t

fails = 0


def check(cond, msg):
    global fails
    if not cond:
        fails += 1
        print('FAIL', msg)


def fast_crc(data):
    # == hsave.crc32 (reflected 0xEDB88320, start 0, no final inversion), verified against it in test_crc
    return (zlib.crc32(data, 0xFFFFFFFF) ^ 0xFFFFFFFF) & 0xFFFFFFFF


def fix_crc(d):
    struct.pack_into('<I', d, len(d) - 4, fast_crc(bytes(d[0x4B:-4])))
    return d


def make_synthetic():
    """A 44,062-byte 0x38 save with a checking CRC and a consistent little game state:
    nations 1..3 in play (type 3); nation 1 occupies t 10, 11, 12 with 2, 3, 4 tokens (sum = +0x98 = 9);
    nations 2 and 3 occupy t 20, 21 (1, 2 tokens) and t 30 (5 tokens); all tstate city bits clear."""
    d = bytearray(SIZE)
    struct.pack_into('<H', d, 0, 0x38)
    d[2:2 + 22] = b'Advanced Civilization\0'          # 21 chars + NUL = exactly 22 bytes
    d[0x18:0x18 + 51] = b'synthetic test save\0' + bytes(51 - 20)
    for i in range(0x4B, SIZE - 4):            # deterministic pattern body
        d[i] = (i * 31 + 7) & 0xFF
    d[0x4F:0x55] = bytes([3, 0, 0, 0, 0, 0])  # setup: 3 humans, 0 computers, flags 0 (PBEM bit clear)
    d[0x395D] = 12                             # phase 12 DECLARE OFFERS (cal_edit accepts 12 or 15)
    struct.pack_into('<h', d, 0x395E, 4)       # round 4
    struct.pack_into('<I', d, 0x3D0C, 0xDE9B76A2)
    d[0x4535:0x4535 + 11] = b'1.11B1' + bytes(5)
    for n in (1, 2, 3):
        d[NAT(n) + 0x9A] = 3                   # player_type human
        d[NAT(n) + 0x13] = 0                   # calamity_count (cal_edit asserts < 8)
        d[NAT(n) + 0x51] = 0                   # cities (city_edit asserts < 9)
    # nation 1: occupied t 10, 11, 12 with 2, 3, 4 tokens
    d[NAT(1) + 0x98] = 9
    d[NAT(1) + 0x99] = 40
    d[NAT(1) + 0x60] = 3
    for j, t in enumerate((10, 11, 12)):
        d[NAT(1) + 0x61 + j] = t + 1
        d[AREA(t) + 0x0F] = (2, 3, 4)[j]       # area tokens[1] at +0x0E+1
        d[AREA(t) + 0x01] = 1                  # nations_present
        d[AREA(t) + 0x02] = 1                  # present[1] at +0x01+1
        d[TST(t) + 2] = 0                      # tstate flags: city bit clear (city_edit asserts)
    # nation 2: t 20 (1 token), t 21 (2 tokens); nation 3: t 30 (5 tokens)
    d[NAT(2) + 0x98] = 3
    d[NAT(2) + 0x60] = 2
    d[NAT(2) + 0x61] = 21
    d[NAT(2) + 0x62] = 22
    d[AREA(20) + 0x10] = 1
    d[AREA(20) + 0x01] = 1
    d[AREA(20) + 0x03] = 2                     # present[2] at +0x01+2
    d[AREA(21) + 0x10] = 2
    d[AREA(21) + 0x01] = 1
    d[AREA(21) + 0x03] = 2
    d[NAT(3) + 0x98] = 5
    d[NAT(3) + 0x60] = 1
    d[NAT(3) + 0x61] = 31
    d[AREA(30) + 0x11] = 5                     # tokens[3] at +0x0E+3
    d[AREA(30) + 0x01] = 1
    d[AREA(30) + 0x04] = 3                     # present[3] at +0x01+3
    return fix_crc(d)


def run_cli(args, **kw):
    return subprocess.run([sys.executable, '-I', os.path.join(HERE, 'savediff.py')] + args,
                          capture_output=True, text=True, **kw)


def diff_lines_of(a, b):
    return savediff.diff_lines(bytes(a[:CRC_OFF]), bytes(b[:CRC_OFF]), ())


def test_crc_identity():
    import random
    random.seed(7)
    for n in (0, 1, 100, 44058):
        d = bytes(random.randrange(256) for _ in range(n))
        check(crc32(d) == fast_crc(d), f'crc identity fails at {n}')


def test_self(tmp):
    base = make_synthetic()
    p = os.path.join(tmp, 's.dat')
    open(p, 'wb').write(base)
    r = run_cli([p, p])
    check(r.returncode == 0, f'self: exit {r.returncode} ({r.stderr.strip()})')
    check('CRC ok' in r.stdout and "desc 'synthetic test save'" in r.stdout and 'phase 12 DECLARE OFFERS' in r.stdout
          and 'round 4' in r.stdout, f'self: header {r.stdout!r}')
    check(' -> ' not in r.stdout, f'self: diff lines {r.stdout!r}')
    check(diff_lines_of(base, base) == [], 'self: diff_lines nonempty')


def test_single_byte_sweep():
    base = make_synthetic()
    a = bytes(base[:CRC_OFF])
    n_checked = 0
    for off in range(2, CRC_OFF):
        b = bytearray(base)
        b[off] ^= (0xFE if off == 0x53 else 0xFF)   # at 0x53 keep the PBEM bit clear
        fix_crc(b)
        lines = savediff.diff_lines(a, bytes(b[:CRC_OFF]), ())
        e = savediff.OFF2ELEM[off]
        eoff, esize, etyp, elabel = savediff.ELEMS[e][0], savediff.ELEMS[e][1], savediff.ELEMS[e][2], savediff.ELEMS[e][3]
        if len(lines) != 1 or not lines[0].startswith(f'0x{eoff:04X}  ') or elabel not in lines[0]:
            check(False, f'offset {off:#x}: {len(lines)} lines, first={lines[:1]}, element={elabel!r} at {eoff:#x}')
            if fails > 30:
                break
        n_checked += 1
    check(n_checked == CRC_OFF - 2, f'sweep covered {n_checked} of {CRC_OFF - 2}')
    # 0x53 with XOR 0xFE gives one line, "setup +4"
    b = bytearray(base)
    b[0x53] ^= 0xFE
    fix_crc(b)
    lines = savediff.diff_lines(a, bytes(b[:CRC_OFF]), ())
    check(len(lines) == 1 and 'setup +4' in lines[0], f'0x53 XOR 0xFE: {lines}')


def test_multi_and_types():
    base = make_synthetic()
    a = bytes(base[:CRC_OFF])
    # two changes in one record -> two lines
    b = bytearray(base)
    b[NAT(5) + 0x98] ^= 1
    b[NAT(5) + 0x99] ^= 1
    fix_crc(b)
    lines = savediff.diff_lines(a, bytes(b[:CRC_OFF]), ())
    check(len(lines) == 2 and all('nation 5' in l for l in lines), f'two changes: {lines}')
    check(any('tokens_on_board' in l for l in lines) and any('tokens_in_stock' in l for l in lines),
          f'two changes labels: {lines}')
    # an s16 change in its high byte -> one line with the right signed value
    b = bytearray(base)
    struct.pack_into('<h', b, 0x395F - 1, -2)      # round s16 at 0x395E, high byte at 0x395F
    fix_crc(b)
    lines = savediff.diff_lines(a, bytes(b[:CRC_OFF]), ())
    check(lines == ['0x395E  round  4 -> -2'], f's16 high byte: {lines}')
    # a change inside the description -> one description line
    b = bytearray(base)
    b[0x18 + 5] ^= 0xFF
    fix_crc(b)
    lines = savediff.diff_lines(a, bytes(b[:CRC_OFF]), ())
    check(len(lines) == 1 and lines[0].startswith('0x0018  description  ') and ' -> ' in lines[0], f'desc: {lines}')
    # a change after the first NUL of the description still shows (full bytes compared)
    b = bytearray(base)
    b[0x18 + 30] ^= 0xFF
    fix_crc(b)
    lines = savediff.diff_lines(a, bytes(b[:CRC_OFF]), ())
    check(len(lines) == 1 and 'description' in lines[0] and '\\x' in lines[0], f'desc past NUL: {lines}')


def test_endgame(tmp):
    base = make_synthetic()
    end = bytes(base) + struct.pack('<I', fast_crc(bytes(base[0x4B:])))   # second CRC over 0x4B..end-4
    check(len(end) == SIZE_END, f'end-game size {len(end)}')
    pa, pb = os.path.join(tmp, 'a.dat'), os.path.join(tmp, 'e.dat')
    open(pa, 'wb').write(base)
    open(pb, 'wb').write(end)
    r = run_cli([pa, pb])
    check(r.returncode == 0, f'endgame: exit {r.returncode} ({r.stderr.strip()})')
    check(' -> ' not in r.stdout, f'endgame: diff lines {r.stdout!r}')
    check('end-game file: first CRC at 0xAC1A' in r.stdout, f'endgame: note missing in {r.stdout!r}')
    check(r.stdout.count('CRC ok') == 2, f'endgame: CRC lines {r.stdout!r}')
    # and the end-game file loads with CRC ok on its own
    d, ok = savediff.load(pb)
    check(ok and len(d) == SIZE_END, 'endgame: load/crc')


def test_refusals(tmp):
    base = make_synthetic()
    p = os.path.join(tmp, 's.dat')
    open(p, 'wb').write(base)

    b = bytearray(base)
    struct.pack_into('<H', b, 0, 0x37)
    fix_crc(b)
    q = os.path.join(tmp, 'v37.dat')
    open(q, 'wb').write(b)
    r = run_cli([q, p])
    check(r.returncode == 2 and 'v37.dat' in (r.stderr + r.stdout), f'version refusal: {r.returncode} {r.stderr!r}')

    q = os.path.join(tmp, 'short.dat')
    open(q, 'wb').write(base[:44061])
    r = run_cli([q, p])
    check(r.returncode == 2 and 'short.dat' in (r.stderr + r.stdout), f'size refusal: {r.returncode} {r.stderr!r}')

    b = bytearray(base)
    b[0x53] |= 0x01
    fix_crc(b)
    q = os.path.join(tmp, 'pbem.dat')
    open(q, 'wb').write(b)
    r = run_cli([q, p])
    check(r.returncode == 2 and 'pbem.dat' in (r.stderr + r.stdout) and 'PBEM' in (r.stderr + r.stdout),
          f'PBEM refusal: {r.returncode} {r.stderr!r}')

    # a bad CRC is reported, not refused; the comparison still runs
    b = bytearray(base)
    b[0x3D0C] ^= 0xFF
    q = os.path.join(tmp, 'badcrc.dat')
    open(q, 'wb').write(b)
    r = run_cli([p, q])
    check(r.returncode == 1, f'bad CRC: exit {r.returncode}')
    check('CRC bad' in r.stdout and 'CRC ok' in r.stdout, f'bad CRC header: {r.stdout!r}')
    check(any(l.startswith('0x3D0C  seed') for l in r.stdout.splitlines()), f'bad CRC still diffs: {r.stdout!r}')


def test_ignore_max_layout(tmp):
    base = make_synthetic()
    b = bytearray(base)
    b[0x18 + 3] ^= 0xFF                 # description
    struct.pack_into('<I', b, 0x3D0C, 0xA7BB1DC8)   # seed
    b[0x395D] = 15                      # phase
    fix_crc(b)
    pa, pb = os.path.join(tmp, 'a.dat'), os.path.join(tmp, 'b.dat')
    open(pa, 'wb').write(base)
    open(pb, 'wb').write(b)
    r = run_cli([pa, pb])
    lines = [l for l in r.stdout.splitlines() if ' -> ' in l]
    check(r.returncode == 1 and len(lines) == 3, f'three diffs: exit {r.returncode} lines {lines}')
    check(any('description' in l for l in lines) and any(l.startswith('0x3D0C  seed  0xDE9B76A2 -> 0xA7BB1DC8') for l in lines)
          and any(l.startswith('0x395D  phase  12 DECLARE OFFERS -> 15 CALAMITIES') for l in lines), f'diff lines {lines}')
    r = run_cli([pa, pb, '--ignore', 'description'])
    lines = [l for l in r.stdout.splitlines() if ' -> ' in l]
    check(r.returncode == 1 and len(lines) == 2 and not any('description' in l for l in lines), f'--ignore: {lines}')
    r = run_cli([pa, pb, '--ignore', 'description', '--ignore', 'seed', '--ignore', 'phase'])
    check(r.returncode == 0 and ' -> ' not in r.stdout, f'--ignore all: exit {r.returncode} {r.stdout!r}')
    r = run_cli([pa, pb, '--max', '1'])
    lines = [l for l in r.stdout.splitlines() if ' -> ' in l]
    check(r.returncode == 1 and len(lines) == 1 and '2 more differences' in r.stdout, f'--max: {r.stdout!r}')
    r = run_cli([pa, pb, '--ignore', 'nosuchfield'])
    check(r.returncode == 2, f'--ignore unknown: {r.returncode}')
    # --layout
    r = run_cli(['--layout'])
    flines = [l for l in r.stdout.splitlines() if l.startswith('0x')]
    check(r.returncode == 0 and len(flines) == len(savediff.FIELDS), f'--layout: {len(flines)} fields')
    r = run_cli(['--layout', pa])
    elines = [l for l in r.stdout.splitlines() if l.startswith('    0x')]
    check(len(elines) == len(savediff.ELEMS), f'--layout file: {len(elines)} element lines vs {len(savediff.ELEMS)}')
    check(any('0x3D0C  seed = 0xDE9B76A2' in l for l in elines), '--layout file: seed value line missing')
    check(any('nation 5 CRETE +0x98 tokens_on_board' in l for l in elines), '--layout file: nation element label missing')


# ---- the three save editors -----------------------------------------------------------------------------------

CAL_ALLOWED = {'phase'}                      # plus nation +0x13/+0x14..0x1B/+0x1C..0x23
CITY_NATION = {0x51} | set(range(0x52, 0x5C)) | {0x5C} | {0x60} | set(range(0x61, 0x98)) | {0x98, 0x99}
CITY_AREA = {0x01} | set(range(0x02, 0x0F)) | set(range(0x0F, 0x1C))
ELIM_NATION = {0x51} | set(range(0x52, 0x5C)) | {0x60, 0x98, 0x99}


def suboff(label):
    if ' +0x' in label:
        return int(label.split(' +0x')[1][:2], 16)
    return None


def check_editor(tmp, name, argv, allowed):
    """Run an editor as a subprocess; savediff IN OUT must give exactly one line per element holding a
    byte where IN and OUT differ, and each such element must be allowed."""
    src = os.path.join(tmp, f'in_{name}.dat')
    dst = os.path.join(tmp, f'out_{name}.dat')
    open(src, 'wb').write(make_synthetic())
    r = subprocess.run([sys.executable, '-I', os.path.join(HERE, argv[0])] + [src, dst] + argv[1:],
                       capture_output=True, text=True)
    check(r.returncode == 0, f'{name}: editor exit {r.returncode}: {r.stderr.strip()}')
    if r.returncode != 0:
        return
    a = open(src, 'rb').read()
    b = open(dst, 'rb').read()
    check(len(b) == SIZE, f'{name}: output size {len(b)}')
    d, ok = savediff.load(dst)
    check(ok, f'{name}: output CRC does not check')
    changed = {savediff.OFF2ELEM[o] for o in range(CRC_OFF) if a[o] != b[o]}
    lines = savediff.diff_lines(a[:CRC_OFF], b[:CRC_OFF], ())
    line_elems = {savediff.OFF2ELEM[int(l.split()[0], 16)] for l in lines}
    check(line_elems == changed, f'{name}: lines cover {len(line_elems)} elements, byte compare {len(changed)}')
    check(len(lines) == len(changed), f'{name}: {len(lines)} lines vs {len(changed)} changed elements')
    for e in changed:
        off, size, typ, label, field, hexed = savediff.ELEMS[e]
        okelt = field in allowed and (allowed[field] is None or suboff(label) in allowed[field])
        check(okelt, f'{name}: unexpected element {label!r} (field {field})')
    # and via the CLI
    r = run_cli([src, dst])
    check(r.returncode == 1 and len([l for l in r.stdout.splitlines() if ' -> ' in l]) == len(changed),
          f'{name}: CLI exit {r.returncode}')


def test_editors(tmp):
    check_editor(tmp, 'cal_edit', ['cal_edit.py', '1:13 2:15'],
                 {'phase': None, 'nation': {0x13} | set(range(0x14, 0x1C)) | set(range(0x1C, 0x24))})
    check_editor(tmp, 'city_edit', ['city_edit.py', '1:11', 'phase=20'],
                 {'phase': None, 'nation': CITY_NATION, 'area': CITY_AREA, 'tstate': {0x00, 0x01, 0x02}})
    check_editor(tmp, 'elim_edit', ['elim_edit.py', '20', '2 3'],
                 {'phase': None, 'nation': ELIM_NATION, 'area': CITY_AREA, 'tstate': {0x00, 0x01, 0x02}})


def main():
    test_crc_identity()
    with tempfile.TemporaryDirectory() as tmp:
        test_self(tmp)
        test_single_byte_sweep()
        test_multi_and_types()
        test_endgame(tmp)
        test_refusals(tmp)
        test_ignore_max_layout(tmp)
        test_editors(tmp)
    if fails == 0:
        print(f'savediff_test: all checks pass ({len(savediff.ELEMS)} elements, {len(savediff.FIELDS)} fields)')
    else:
        print(f'savediff_test: {fails} FAILURES')
    return 1 if fails else 0


if __name__ == '__main__':
    sys.exit(main())
