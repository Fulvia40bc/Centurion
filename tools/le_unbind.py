#!/usr/bin/env python3
"""Unbind a DOS/4GW-bound Watcom LE executable into flat, relocated object images.

Usage: le_unbind.py AC_MAIN.EXE OUTDIR

AC_MAIN.EXE = DOS/16M loader + DOS/4GW Professional kernel (bound) + an MZ stub whose
e_lfanew points at the LE header of the game itself. This tool finds that LE header, reads
the object table, assembles each object's pages at its link-time base address, applies the
internal fixups (32-bit offsets and self-relative), and writes:

  OUTDIR/obj<N>.bin      the object's bytes, loaded at its base (size = virtual size, zero-filled)
  OUTDIR/le_map.txt      header summary, objects, entry point, fixup counts by type
  OUTDIR/fixups.txt      every fixup site: source address -> target address (lets a disassembler
                         tell pointers from numbers, and seeds code/data discovery)

Only internal-reference fixups are expected (a flat Watcom program imports nothing); any other
kind is reported, not silently skipped.
"""
import os, struct, sys, collections

def u8(d, o): return d[o]
def u16(d, o): return struct.unpack_from('<H', d, o)[0]
def u32(d, o): return struct.unpack_from('<I', d, o)[0]

def find_le(d):
    """Return the file offset of the LE header (the game's, not the extender's)."""
    o = 0
    best = None
    while True:
        o = d.find(b'MZ', o)
        if o < 0 or o + 0x40 > len(d): break
        lf = u32(d, o + 0x3C)
        if o + lf + 2 <= len(d) and d[o + lf:o + lf + 2] == b'LE':
            best = (o, o + lf)
        o += 2
    if best is None: sys.exit('no MZ+LE pair found')
    return best

def main():
    src, out = sys.argv[1], sys.argv[2]
    d = open(src, 'rb').read()
    mz, le = find_le(d)
    os.makedirs(out, exist_ok=True)
    h = lambda off: u32(d, le + off)
    npages, eip_obj, eip, esp_obj, esp = h(0x14), h(0x18), h(0x1C), h(0x20), h(0x24)
    page_size, last_page = h(0x28), h(0x2C)
    obj_tab, nobj, page_map = h(0x40), h(0x44), h(0x48)
    fix_page_tab, fix_rec_tab = h(0x68), h(0x6C)
    nimports = h(0x74)
    data_pages = h(0x80)
    # data_pages is relative to the start of the file for a plain LE; for a bound DOS/4GW
    # program it is relative to the start of the stub MZ. Pick whichever lands inside the file.
    data_base = data_pages if data_pages + (npages - 1) * page_size + last_page <= len(d) - 0 and data_pages > le else mz + data_pages
    log = []
    p = log.append
    p(f'file {src} ({len(d)} bytes)')
    p(f'stub MZ at 0x{mz:X}, LE header at 0x{le:X}')
    p(f'pages {npages} x 0x{page_size:X} (last 0x{last_page:X}), data pages at 0x{data_base:X} (raw field 0x{data_pages:X})')
    p(f'entry obj {eip_obj}:0x{eip:X}  stack obj {esp_obj}:0x{esp:X}  imports {nimports}')
    objs = []
    for i in range(nobj):
        e = le + obj_tab + 24 * i
        vsize, base, flags, pidx, pcnt = struct.unpack_from('<IIIII', d, e)
        objs.append(dict(n=i + 1, vsize=vsize, base=base, flags=flags, pidx=pidx, pcnt=pcnt))
        kind = 'code' if flags & 4 else 'data'
        p(f'object {i+1}: base 0x{base:08X} vsize 0x{vsize:X} flags 0x{flags:04X} ({kind}{", 32-bit" if flags & 0x2000 else ""}) pages {pidx}..{pidx+pcnt-1}')
    def page_file_off(pn):  # pn 1-based object-page-map index
        e = le + page_map + 4 * (pn - 1)
        num = (d[e] << 16) | (d[e + 1] << 8) | d[e + 2]
        fl = d[e + 3]
        return data_base + (num - 1) * page_size, fl
    images = {}
    for ob in objs:
        img = bytearray(ob['vsize'] + page_size)
        for k in range(ob['pcnt']):
            pn = ob['pidx'] + k
            off, fl = page_file_off(pn)
            if fl not in (0,): p(f'  page {pn} flags 0x{fl:02X} (not a plain page)')
            ln = last_page if pn == npages else page_size
            img[k * page_size:k * page_size + ln] = d[off:off + ln]
        images[ob['n']] = img
    page_obj = {}
    for ob in objs:
        for k in range(ob['pcnt']): page_obj[ob['pidx'] + k] = (ob, k)
    counts = collections.Counter()
    sites = []
    for pn in range(1, npages + 1):
        a = le + fix_page_tab + 4 * (pn - 1)
        start, end = u32(d, a), u32(d, a + 4)
        r = le + fix_rec_tab + start
        rend = le + fix_rec_tab + end
        ob, k = page_obj[pn]
        img = images[ob['n']]
        while r < rend:
            stype, flags = d[r], d[r + 1]; r += 2
            if stype & 0x20:
                cnt = d[r]; r += 1; srcoffs = None
            else:
                srcoffs = [struct.unpack_from('<h', d, r)[0]]; r += 2
            if flags & 3 != 0:
                sys.exit(f'page {pn}: non-internal fixup flags 0x{flags:02X} (imports not expected)')
            if flags & 0x40: tobj = u16(d, r); r += 2
            else: tobj = d[r]; r += 1
            t = stype & 0x0F
            if t == 2:
                toff = None
            elif flags & 0x10:
                toff = u32(d, r); r += 4
            else:
                toff = u16(d, r); r += 2
            if srcoffs is None:
                srcoffs = [struct.unpack_from('<h', d, r + 2 * j)[0] for j in range(cnt)]; r += 2 * cnt
            tgt = None if toff is None else objs[tobj - 1]['base'] + toff
            for so in srcoffs:
                pos = k * page_size + so
                saddr = ob['base'] + pos
                counts[t] += 1
                if t == 7:      # 32-bit offset
                    if 0 <= pos <= len(img) - 4: struct.pack_into('<I', img, pos, tgt)
                    sites.append((saddr, tgt, 'off32'))
                elif t == 8:    # 32-bit self-relative
                    if 0 <= pos <= len(img) - 4: struct.pack_into('<i', img, pos, tgt - (saddr + 4))
                    sites.append((saddr, tgt, 'rel32'))
                elif t == 2:    # 16-bit selector: flat model, leave the bytes, note the object
                    sites.append((saddr, tobj, 'sel16'))
                elif t == 5:    # 16-bit offset
                    if 0 <= pos <= len(img) - 2: struct.pack_into('<H', img, pos, tgt & 0xFFFF)
                    sites.append((saddr, tgt, 'off16'))
                else:
                    sites.append((saddr, tgt, f'type{t}'))
    for ob in objs:
        open(os.path.join(out, f"obj{ob['n']}.bin"), 'wb').write(images[ob['n']][:ob['vsize']])
    names = {2: 'sel16', 5: 'off16', 7: 'off32', 8: 'rel32'}
    p('fixups: ' + ', '.join(f'{names.get(t, "type%d" % t)} {c}' for t, c in sorted(counts.items())))
    p(f"entry point: 0x{objs[eip_obj-1]['base'] + eip:08X}")
    open(os.path.join(out, 'le_map.txt'), 'w').write('\n'.join(log) + '\n')
    with open(os.path.join(out, 'fixups.txt'), 'w') as f:
        for s, t, k in sorted(sites):
            f.write(f'{s:08X} {k} {t:08X}\n' if k != 'sel16' else f'{s:08X} {k} obj{t}\n')
    print('\n'.join(log))

if __name__ == '__main__':
    main()
