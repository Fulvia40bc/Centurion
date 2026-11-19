#!/usr/bin/env python3
"""Print a dialog's button rectangles and centres from a data-object dump (docs/M6_PLAN.md: click scripts name
buttons by id, not by pixel).

Usage: python -I dlgxy.py DUMP.bin DLG [ID]

DUMP.bin: the data object, link base 0xB0000, 0x72C00 bytes (DOSBox-X: MEMDUMPBIN 0188:2A9000 72C00; the port:
--dump-ds). DLG: the dialog's link address, hex (0x...) or decimal. The dialog (D, ui.h): int16 count at +0,
then count records of 20 bytes at +2 + 20i - int16 x +0, y +2, w +4, h +6, id +0x0E, enabled byte +0x10.

Prints one line per record: id, rectangle, centre (x + w/2, y + h/2 with C integer division), enabled byte.
With ID (hex or decimal, compared as the game's 16-bit compare): only the first record with that id, its centre
printed as "x,y". Exit 1 on a dump of the wrong size, a DLG outside the data object, a count running past the
dump, or an unknown ID.
"""
import struct
import sys

BASE, SIZE = 0xB0000, 0x72C00


def main():
    if len(sys.argv) < 3 or len(sys.argv) > 4:
        sys.exit(__doc__)
    try:
        dump = open(sys.argv[1], 'rb').read()
    except OSError as e:
        sys.exit(f'dlgxy: {e}')
    if len(dump) != SIZE:
        sys.exit(f'dlgxy: {sys.argv[1]} is {len(dump)} bytes, expected {SIZE:#X}')
    dlg = int(sys.argv[2], 0)
    off = dlg - BASE
    if not 0 <= off <= len(dump) - 2:
        sys.exit(f'dlgxy: DLG {sys.argv[2]} ({dlg:#x}) is outside the data object')
    count = struct.unpack_from('<h', dump, off)[0]
    if count < 0 or off + 2 + 20 * count > len(dump):
        sys.exit(f'dlgxy: count {count} at {dlg:#x} runs past the dump')
    want = int(sys.argv[3], 0) & 0xFFFF if len(sys.argv) == 4 else None
    for i in range(count):
        rec = off + 2 + 20 * i
        x, y, w, h = struct.unpack_from('<hhhh', dump, rec)
        bid = struct.unpack_from('<h', dump, rec + 0x0E)[0] & 0xFFFF
        en = dump[rec + 0x10]
        cx, cy = x + w // 2, y + h // 2
        if want is not None:
            if bid == want:
                print(f'{cx},{cy}')
                return 0
        else:
            print(f'id={bid:#06x} rect=({x},{y},{w},{h}) centre={cx},{cy} enabled={en}')
    if want is not None:
        sys.exit(f'dlgxy: no button with id {want:#x} in DLG {dlg:#x}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
