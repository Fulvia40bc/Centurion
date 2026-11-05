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

def lzw_decompress(data, bits_msb=True, clear_code=256, first=9, maxbits=12):
    """Generic LZW. Tries to auto-detect bit order."""
    out = bytearray()
    table = None
    bitpos = 0
    nbits = first
    prev = None

    def read_code():
        nonlocal bitpos, nbits
        if bitpos + nbits > len(data) * 8:
            return None
        if bits_msb:
            byte = bitpos // 8
            bit = bitpos % 8
            val = 0
            for i in range(nbits):
                b = (data[(bitpos + i) // 8] >> (7 - (bitpos + i) % 8)) & 1
                val = (val << 1) | b
            bitpos += nbits
            return val
        else:
            val = 0
            for i in range(nbits):
                b = (data[(bitpos + i) // 8] >> ((bitpos + i) % 8)) & 1
                val |= b << i
            bitpos += nbits
            return val

    table = {i: bytes([i]) for i in range(256)}
    next_code = clear_code + 2 if clear_code else 258
    while True:
        code = read_code()
        if code is None:
            break
        if clear_code and code == clear_code:
            table = {i: bytes([i]) for i in range(256)}
            next_code = clear_code + 2
            nbits = first
            prev = None
            continue
        if code in table:
            entry = table[code]
        elif prev is not None and code == next_code:
            entry = prev + prev[:1]
        else:
            break
        out += entry
        if prev is not None:
            table[next_code] = prev + entry[:1]
            next_code += 1
            if next_code == (1 << nbits) + 1 and nbits < maxbits:
                nbits += 1
        prev = entry
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
        entries = parse_dir(os.path.join(src_dir, dirfile))
        print(f'{dirfile}: {len(entries)} entries')
        for name, offset, length, fmt in entries:
            raw = blob[4+offset:4+offset+length]
            if fmt == 0x80:
                for msb in (True, False):
                    dec = lzw_decompress(raw, bits_msb=msb)
                    if len(dec) > len(raw) and all(32 <= b < 127 or b in (9,10,13,0) for b in dec[:200]):
                        raw = dec
                        break
                else:
                    dec = lzw_decompress(raw, bits_msb=True)
                    if len(dec) > 0:
                        raw = dec
            out = os.path.join(out_dir, base + '__' + name.lower())
            open(out, 'wb').write(raw)
            print(f'  {name:14s} fmt=0x{fmt:02x} {length:6d} -> {len(raw):6d} bytes')

if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
