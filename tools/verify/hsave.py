#!/usr/bin/env python3
"""Make every nation in a save file a human player (docs/M6_PLAN.md "All-human saves").

Usage: python -I hsave.py IN.DAT OUT.DAT [--desc TEXT]

Save layout (D, port/src/saveload.c's f5F05C and port/src/savewrite.c's f5E9B0): int16 version at 0, the 22-byte
"Advanced Civilization" tag at 2, the 51-byte description at 0x18, the body from 0x4B. The first body ranges: 4
bytes at 0x4B (link 0xF40F4), then the 6 bytes at 0x4F from link 0xE1620 (byte 0 = number of humans, byte 1 = number
of computers), then the 14 nation records of 199 bytes at 0x55 (link 0xF14CC + 199n; byte +0x9A of record n is the
player type: 0 not in play, 2 computer, 3 human). The last 4 bytes are the CRC-32 (reflected polynomial 0xEDB88320,
start value 0, no final inversion, table as title.c's f7BB14, i.e. f7BB84) over the file from 0x4B up to (not
including) the last 4 bytes, stored little-endian - so a correct file checks as CRC 0 over 0x4B..end.

Every nation whose player type is 2 becomes 3; humans += that count, computers = 0. --desc TEXT replaces the
description (UTF-8, truncated to 50 bytes, NUL-padded to 51; byte 50 stays NUL, as the writer's desc[50] = 0). The
CRC is recomputed. A file that is not 44,062 bytes, not version 0x38, or whose CRC does not check is refused
(exit 1 with a message). Prints the nations changed.
"""
import argparse
import struct
import sys

VERSION, SIZE = 0x38, 44062
DESC_OFF, DESC_LEN, BODY_OFF = 0x18, 51, 0x4B
CFG_OFF = 0x4F                                  # link 0xE1620: byte 0 humans, byte 1 computers
NAT_OFF, NAT_SIZE, NAT_N, TYPE_OFF = 0x55, 199, 14, 0x9A   # link 0xF14CC + 199n, player type at +0x9A


def crc_table():
    t = []
    for i in range(256):
        c = i
        for _ in range(8):
            c = (c >> 1) ^ 0xEDB88320 if c & 1 else c >> 1
        t.append(c)
    return t


TABLE = crc_table()


def crc32(data):
    crc = 0
    for b in data:
        crc = (crc >> 8) ^ TABLE[(crc ^ b) & 0xFF]
    return crc


def die(msg):
    print(f'hsave: {msg}', file=sys.stderr)
    sys.exit(1)


def main():
    ap = argparse.ArgumentParser(description='make every nation in a version-0x38 save a human player')
    ap.add_argument('infile')
    ap.add_argument('outfile')
    ap.add_argument('--desc', help='replace the 51-byte description (NUL-padded)')
    a = ap.parse_args()

    try:
        d = bytearray(open(a.infile, 'rb').read())
    except OSError as e:
        die(str(e))
    if len(d) != SIZE:
        die(f'{a.infile} is {len(d)} bytes, expected {SIZE}')
    if struct.unpack_from('<H', d, 0)[0] != VERSION:
        die(f'{a.infile} is version {struct.unpack_from("<H", d, 0)[0]:#x}, expected {VERSION:#x}')
    stored = struct.unpack_from('<I', d, len(d) - 4)[0]
    if crc32(d[BODY_OFF:len(d) - 4]) != stored:
        die(f'{a.infile}: CRC does not check (stored {stored:#010x})')

    changed = []
    for n in range(NAT_N):
        o = NAT_OFF + NAT_SIZE * n + TYPE_OFF
        if d[o] == 2:
            d[o] = 3
            changed.append(n)
    h0, c0 = d[CFG_OFF], d[CFG_OFF + 1]
    d[CFG_OFF] = (h0 + len(changed)) & 0xFF
    d[CFG_OFF + 1] = 0
    if a.desc is not None:
        b = a.desc.encode('utf-8')[:DESC_LEN - 1]
        d[DESC_OFF:DESC_OFF + DESC_LEN] = b + bytes(DESC_LEN - len(b))

    crc = crc32(d[BODY_OFF:len(d) - 4])
    struct.pack_into('<I', d, len(d) - 4, crc)
    open(a.outfile, 'wb').write(d)
    for n in changed:
        print(f'nation {n}: 2 -> 3')
    print(f'humans {h0} -> {d[CFG_OFF]}, computers {c0} -> 0; wrote {a.outfile} ({SIZE} bytes, crc {crc:#010x})')


if __name__ == '__main__':
    main()
