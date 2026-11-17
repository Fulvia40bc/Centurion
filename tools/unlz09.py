#!/usr/bin/env python3
# LZEXE v0.90 ("LZ09") unpacker, adapted from samrussell/unpacklzexe (LZ91).
import struct, sys

def parseheader(h):
    f = struct.unpack('<14H', h[:0x1C])
    return dict(zip(
        ['sig','partpage','pagecnt','relocnt','hdrsize','minalloc','maxalloc',
         'initss','initsp','chksum','initip','initcs','tabloff','overlayno'], f))

def unpack_data(indata):
    out = bytearray()
    si = 0
    dx = 0x10
    bp = struct.unpack('<H', indata[si:si+2])[0]; si += 2
    while True:
        bit = bp & 1; bp >>= 1; dx -= 1
        if dx == 0:
            bp = struct.unpack('<H', indata[si:si+2])[0]; si += 2; dx = 0x10
        if bit == 1:
            out.append(indata[si]); si += 1
            continue
        cx = 0
        bit = bp & 1; bp >>= 1; dx -= 1
        if dx == 0:
            bp = struct.unpack('<H', indata[si:si+2])[0]; si += 2; dx = 0x10
        if bit == 0:
            for _ in range(2):
                bit = bp & 1; bp >>= 1; dx -= 1
                if dx == 0:
                    bp = struct.unpack('<H', indata[si:si+2])[0]; si += 2; dx = 0x10
                cx = ((cx << 1) + bit) & 0xFFFF
            cx += 2
            tb = indata[si]; si += 1
            bx = tb - 0x100
            for _ in range(cx):
                out.append(out[bx]); bx += 1
            continue
        ax = struct.unpack('<H', indata[si:si+2])[0]; si += 2
        bx = ax
        bh = (bx >> 8) >> 3
        bx = (bh << 8) + (bx & 0xff)
        bx -= 0x2000
        ah = (ax >> 8) & 0x7
        if ah != 0:
            for _ in range(ah + 2):
                out.append(out[bx]); bx += 1
            continue
        al = indata[si]; si += 1
        if al == 0:
            break
        if al != 1:
            for _ in range(al + 1):
                out.append(out[bx]); bx += 1
            continue
        # al == 1: segment realign signal, ignore in linear output
    return bytes(out)

def unpack_reloc(relocdata):
    out = bytearray()
    si = 0; dx = 0; di = 0
    while True:
        al = relocdata[si]; si += 1
        if al == 0:
            ax = struct.unpack('<H', relocdata[si:si+2])[0]; si += 2
            if ax == 0:
                dx = (dx + 0xfff) % 0x10000
                continue
            if ax == 1:
                break
        else:
            ax = al
        di += ax
        es = (dx + (di >> 4)) & 0xFFFF
        out += struct.pack('<HH', di & 0xf, es)
        dx = es
    return bytes(out)

def main(infile, outfile, reloc_off=0x158):
    data = open(infile, 'rb').read()
    hd = parseheader(data[:0x1C])
    assert data[0x1C:0x20] == b'LZ09', data[0x1C:0x20]
    loaderoffset = hd['initcs'] * 0x10 + hd['hdrsize'] * 0x10
    exeip, execs, exesp, exess = struct.unpack('<4H', data[loaderoffset:loaderoffset+8])
    packed = data[0x20:loaderoffset]
    packedreloc = data[loaderoffset+reloc_off:]
    print(f"loader at file 0x{loaderoffset:X}, orig CS:IP={execs:04X}:{exeip:04X} SS:SP={exess:04X}:{exesp:04X}")
    print(f"packed data: {len(packed)} bytes, packed reloc: {len(packedreloc)} bytes")
    img = unpack_data(packed)
    try:
        reloc = unpack_reloc(packedreloc)
    except Exception as e:
        print('reloc failed:', e); reloc = b''
    print(f"unpacked image: {len(img)} bytes (0x{len(img):X}), relocs: {len(reloc)//4}")
    relocnt = len(reloc)//4
    headersize = 0x1C + relocnt*4
    pad = (0x10 - headersize % 0x10) % 0x10
    headersize += pad
    filesize = headersize + len(img)
    nh = dict(hd)
    nh.update(sig=0x5A4D, partpage=filesize % 0x200,
              pagecnt=(filesize + 0x1FF)//0x200, relocnt=relocnt, hdrsize=headersize//0x10,
              minalloc=(hd['minalloc'] - ((len(img)-len(packed))//0x10)) & 0xFFFF,
              maxalloc=0xFFFF, initss=exess, initsp=exesp, chksum=0,
              initip=exeip, initcs=execs, tabloff=0x1C, overlayno=0)
    hdr = struct.pack('<14H', nh['sig'], nh['partpage'], nh['pagecnt'], nh['relocnt'],
                      nh['hdrsize'], nh['minalloc'], nh['maxalloc'], nh['initss'],
                      nh['initsp'], nh['chksum'], nh['initip'], nh['initcs'],
                      nh['tabloff'], nh['overlayno'])
    open(outfile, 'wb').write(hdr + reloc + b'\x00'*pad + img)
    print(f"wrote {outfile}: {filesize} bytes, header {headersize} bytes")

if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2], int(sys.argv[3], 0) if len(sys.argv) > 3 else 0x158)
