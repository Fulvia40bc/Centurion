#!/usr/bin/env python3
"""Extract assets from Centurion: Defender of Rome .DAT/.DIR pairs.

DIR entry (20 bytes): int32 offset, uint16 length, char name[13] (NUL-padded),
uint8 format (0x00 raw, 0x80 LZW-compressed, 0xC0 CBM image).
DAT file: 4-byte signature CA CA D0 D0, then concatenated member data.
"""
import os, struct, sys

DAT_SIG = b'\xca\xca\xd0\xd0'

def parse_dir(path):
    data = open(path, 'rb').read()
    entries = []
    off = 0
    while off + 20 <= len(data):
        offset, length = struct.unpack_from('<IH', data, off)
        name = data[off+6:off+19].split(b'\x00')[0].decode('ascii', 'replace')
        fmt = data[off+19]
        entries.append((name, offset, length, fmt))
        off += 20
    return entries

def lzss_decompress(data, winit=0xFEE, prefill=0x20):
    """LZSS as used by the game's 'unlzss' (despite the .LZW extension).

    Reverse-engineered from the unpacker at 0x01DE-0x0287 in the unpacked
    CENTURIO.EXE: 4096-byte circular window prefilled with 0x20, write position
    starts at 0xFEE. Stream = flag byte (8 flags, LSB first; 1 = literal byte,
    0 = match) followed by items; match = 2 bytes: low 8 bits of window
    position in byte 1, high 4 bits in byte 2's high nibble, length =
    byte 2's low nibble + 3.
    """
    window = bytearray([prefill] * 4096)
    wpos = winit
    out = bytearray()
    i = 0
    while i < len(data):
        flags = data[i]
        i += 1
        for bit in range(8):
            if i >= len(data):
                break
            if flags & (1 << bit):
                b = data[i]
                i += 1
                out.append(b)
                window[wpos] = b
                wpos = (wpos + 1) & 0xFFF
            else:
                if i + 1 >= len(data):
                    break
                b1, b2 = data[i], data[i + 1]
                i += 2
                pos = (b1 | ((b2 & 0xF0) << 4)) & 0xFFF
                ln = (b2 & 0x0F) + 3
                for k in range(ln):
                    b = window[(pos + k) & 0xFFF]
                    out.append(b)
                    window[wpos] = b
                    wpos = (wpos + 1) & 0xFFF
    return bytes(out)

def main(src_dir, out_dir):
    os.makedirs(out_dir, exist_ok=True)
    for dirfile in sorted(f for f in os.listdir(src_dir) if f.lower().endswith('.dir')):
        base = dirfile[:-4]
        dat = os.path.join(src_dir, base + '.DAT')
        if not os.path.exists(dat):
            continue
        blob = open(dat, 'rb').read()
        assert blob[:4] == DAT_SIG, f'{dat}: bad signature {blob[:4].hex()}'
        # DIR offsets are absolute file offsets (they include the 4-byte
        # signature): the first entry starts at offset 4 == right after sig.
        entries = parse_dir(os.path.join(src_dir, dirfile))
        print(f'{dirfile}: {len(entries)} entries')
        for name, offset, length, fmt in entries:
            raw = blob[offset:offset+length]
            if fmt == 0x80:
                # .LZW members start with a uint32 uncompressed size, then the
                # LZSS stream; decode and truncate to the declared size.
                usize = struct.unpack_from('<I', raw, 0)[0]
                raw = lzss_decompress(raw[4:])[:usize]
            out = os.path.join(out_dir, base + '__' + name.lower())
            open(out, 'wb').write(raw)
            print(f'  {name:14s} fmt=0x{fmt:02x} {length:6d} -> {len(raw):6d} bytes')

if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
