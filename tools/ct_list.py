#!/usr/bin/env python3
"""List (and optionally extract) the ACIV.CT1..CT5 archives.

Usage: ct_list.py ARCHIVE [OUTDIR]

Directory format (D, read from the files): entries of 24 bytes at offset 0,
  char name[12] (NUL padded), u32 offset, u32 length, u32 kind
The first entry's offset is the directory size, so count = first_offset / 24.
kind seen so far: 0 (HLP, XMI), 2 (GIF), 6 (FLC); meaning G.
"""
import os, struct, sys, collections

def read_dir(d):
    first = struct.unpack_from('<I', d, 12)[0]
    n = first // 24
    out = []
    for i in range(n):
        name, off, ln, kind = struct.unpack_from('<12sIII', d, 24 * i)
        out.append((name.split(b'\0')[0].decode('latin1'), off, ln, kind))
    return out

def main():
    d = open(sys.argv[1], 'rb').read()
    ents = read_dir(d)
    end = max(o + l for _, o, l, _ in ents)
    kinds = collections.Counter((os.path.splitext(n)[1], k) for n, _, _, k in ents)
    print(f'{sys.argv[1]}: {len(ents)} entries, data ends 0x{end:X} of 0x{len(d):X}')
    print('  by extension/kind: ' + ', '.join(f'{e or "-"}/{k} x{c}' for (e, k), c in sorted(kinds.items())))
    for n, o, l, k in ents:
        magic = d[o:o + 6]
        print(f'  {n:<13} @{o:08X} {l:8d}  kind {k}  {magic!r}')
    if len(sys.argv) > 2:
        os.makedirs(sys.argv[2], exist_ok=True)
        for n, o, l, k in ents:
            if l == 0:  # the directory's last entry is an empty end marker (name 0xFF...)
                continue
            open(os.path.join(sys.argv[2], n), 'wb').write(d[o:o + l])

if __name__ == '__main__':
    main()
