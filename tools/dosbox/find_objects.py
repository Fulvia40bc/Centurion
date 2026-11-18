#!/usr/bin/env python3
"""Locate AC_MAIN.EXE's two LE objects in a DOSBox-X flat-memory dump and compare them with le_unbind's images.

DOS/4GW loads each object at its own address and patches every fixup for it, so a relocated dword in memory is
link-time value + (runtime base of the target object - link base). This tool finds the bases by the banners
(Watcom's run-time banner in the code object, the game's title banner in the data object), then compares every
byte: equal, equal after rebasing a fixup, or different (data the program has changed since start-up).

Usage: find_objects.py DUMP LE_DIR      (DUMP = MEMDUMPBIN of selector 0188 from offset 0, i.e. flat memory)
"""
import struct, sys

CODE_LINK, DATA_LINK = 0x10000, 0xB0000
CODE_KEY = (0x8BF56, b'WATCOM C/C++32 Run-Time system')
DATA_KEY = (0xB0071, b'Advanced Civilization (R)')


def find_base(mem, image, link, key):
    addr, text = key
    hits = []
    p = mem.find(text)
    while p >= 0:
        hits.append(p - (addr - link))
        p = mem.find(text, p + 1)
    return hits


def compare(mem, image, base, link, fix, bases):
    same = rebased = diff = 0
    runs = []
    skip = set()
    for s, t in fix.items():
        if link <= s < link + len(image):
            o = s - link
            tl = CODE_LINK if t < DATA_LINK else DATA_LINK
            want = (t - tl + bases[tl]) & 0xFFFFFFFF
            got = struct.unpack_from('<I', mem, base + o)[0]
            if got == want:
                rebased += 4
            else:
                diff += 4
                runs.append(o)
            skip.update(range(o, o + 4))
    for o in range(len(image)):
        if o in skip:
            continue
        if mem[base + o] == image[o]:
            same += 1
        else:
            diff += 1
            runs.append(o)
    return same, rebased, diff, sorted(runs)


def summarize(runs, link, limit=25):
    out, i = [], 0
    while i < len(runs) and len(out) < limit:
        j = i
        while j + 1 < len(runs) and runs[j + 1] - runs[j] <= 4:
            j += 1
        out.append(f'{link + runs[i]:08X}+{runs[j] - runs[i] + 1}')
        i = j + 1
    return out


def main():
    mem = open(sys.argv[1], 'rb').read()
    le = sys.argv[2]
    code = open(f'{le}/obj1.bin', 'rb').read()
    data = open(f'{le}/obj2.bin', 'rb').read()
    fix = {}
    for line in open(f'{le}/fixups.txt'):
        f = line.split()
        fix[int(f[0], 16)] = int(f[2], 16)
    cb = find_base(mem, code, CODE_LINK, CODE_KEY)
    db = find_base(mem, data, DATA_LINK, DATA_KEY)
    print(f'code object candidates: {[hex(x) for x in cb]}')
    print(f'data object candidates: {[hex(x) for x in db]}')
    if len(cb) != 1 or len(db) != 1:
        sys.exit('need exactly one of each')
    bases = {CODE_LINK: cb[0], DATA_LINK: db[0]}
    print(f'code object at {cb[0]:#x} (link {CODE_LINK:#x}, delta {cb[0] - CODE_LINK:+#x}); '
          f'data object at {db[0]:#x} (link {DATA_LINK:#x}, delta {db[0] - DATA_LINK:+#x})')
    for name, img, link in (('code', code, CODE_LINK), ('data', data, DATA_LINK)):
        n = min(len(img), len(mem) - bases[link])
        s, r, d, runs = compare(mem, img[:n], bases[link], link, fix, bases)
        print(f'{name}: {n} bytes compared: {s} identical, {r} relocated fixup bytes identical after rebasing, '
              f'{d} different')
        if d:
            print('   differing ranges (link address+len): ' + ' '.join(summarize(runs, link)))


if __name__ == '__main__':
    main()
