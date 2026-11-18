#!/usr/bin/env python3
"""List (and optionally extract) the ACIV.CT1..CT5 archives.

Usage: ct_list.py ARCHIVE [OUTDIR]

Directory format (D, read from the files, 2026-11-18): a chain of directory blocks. Each block is 32 entries of
24 bytes, char name[12] (NUL padded), u32 offset, u32 length, u32 kind, then a link entry whose name is twelve 0xFF
bytes and whose offset is the file offset of the next block (0 = last block). The first block is at offset 0.
Unused entries have an empty name. CT1 has 2 blocks (the second holds AH_LOGO.GIF, ADVCIVIL.GIF and the three .FGF
fonts), CT3 has 4 (help texts and the .BPM opening maps), CT2, CT4 and CT5 have 1.
kind seen so far: 0 (HLP, XMI, WAV, BPM), 2 (GIF), 6 (FLC), 7 (FGF font); meaning G.
"""
import os, struct, sys, collections

def read_dir(d):
    out, base, seen = [], 0, set()
    while base not in seen:
        seen.add(base)
        for i in range(33):
            name, off, ln, kind = struct.unpack_from('<12sIII', d, base + 24 * i)
            if name == b'\xff' * 12:
                break
            n = name.split(b'\0')[0].decode('latin1')
            if n:
                out.append((n, off, ln, kind))
        else:
            raise SystemExit(f'no link entry in the block at 0x{base:X}')
        if off == 0:
            break
        base = off
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
            open(os.path.join(sys.argv[2], n), 'wb').write(d[o:o + l])

if __name__ == '__main__':
    main()
