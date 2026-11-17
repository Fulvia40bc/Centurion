#!/usr/bin/env python3
"""Reference model of the AdLib music driver ADLIB.LZW (Claude, 2026-11-17; docs/MUSIC_NOTES.md, disasm/ADLIB_DRV.asm).

A literal Python model of the driver's code working on a copy of the driver image, so its variables sit at their real
offsets. It prints the OPL2 register writes per driver tick, for checking the port's C sequencer
(centurion --music-wav SONG OUT.txt SECONDS writes the same log), and checks itself against the DOSBox RAM dumps.

usage (from the game folder, which holds the .DAT/.DIR archives):
  python adlib_regs.py SONG SECONDS            SONG = 0..16 or a name (cn9a); log lines "tick reg val" (hex) on stdout
  python adlib_regs.py --check-dumps           the driver state in tools/dosbox/results/mem_*_640k.bin vs the model
"""
import os, struct, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from extract_data import parse_dir, lzss_decompress

# DS:3EF0 song table {name, archive}; archive numbers of 0F7B (DS:029E): 0 MRACE 2 BATL 3 CENT 5 MMAP 6 MGLAD 7 MNEGO 8 MNAVAL
SONGS = [('cn1', 'CENT'), ('cn2', 'MMAP'), ('cn3a', 'MRACE'), ('cn3b', 'MRACE'), ('cn4a', 'MGLAD'), ('cn4b', 'MGLAD'),
         ('cn4c', 'MGLAD'), ('cn4d', 'MGLAD'), ('cn6', 'MNEGO'), ('cn7a', 'BATL'), ('cn7b', 'BATL'), ('cn8a', 'BATL'),
         ('cn8b', 'BATL'), ('cn9a', 'MNAVAL'), ('cn9b', 'MNAVAL'), ('cn9c', 'MNAVAL'), ('cn9d', 'MNAVAL')]

def member(arch, name):
    for n, off, ln, fmt in parse_dir(arch + '.DIR'):
        if n.lower() == name.lower() + '.lzw':
            raw = open(arch + '.DAT', 'rb').read()[off:off + ln]
            return lzss_decompress(raw[4:])[:struct.unpack_from('<I', raw, 0)[0]]
    raise SystemExit('%s.lzw not in %s' % (name, arch))


class Driver:
    def __init__(self, image, song, log=None):
        self.m = bytearray(image)          # the driver segment
        self.s = song                      # the song segment (ES)
        self.log = log                     # log(reg, val)

    def b(self, a): return self.m[a]
    def sb(self, a, v): self.m[a] = v & 0xFF
    def w(self, a): return self.m[a] | self.m[a + 1] << 8
    def sw(self, a, v): self.m[a] = v & 0xFF; self.m[a + 1] = (v >> 8) & 0xFF
    def out(self, reg, val):
        if self.log: self.log(reg, val & 0xFF)

    def reset(self):                                       # 0x929
        self.out(0x01, 0x20); self.out(0x04, 0xE0)
        for si in range(0x64, -1, -1): self.out(self.b(0xD6 + si), 0)
        for si in range(0x11, -1, -1): self.out(self.b(0x13B + si), 0x3F)

    def start(self):                                       # AH=1 0x403
        st = self.s[0] | self.s[1] << 8
        self.sw(0x3C8, st); self.sw(0x3CA, st)
        for v in range(9): self.sb(0x35C + v, self.s[0x22 + v])
        self.reset()
        for v in range(9): self.sb(0x365 + v, 0); self.sb(0x36E + v, 0); self.sw(0x3B6 + 2 * v, 0)
        for v in range(9):
            if self.b(0x35C + v) == 9: self.patch(self.b(0x392 + v), v)
        self.sb(0x3CE, 0); self.sb(0x3D8, 0); self.sb(0x3D1, 1)

    def stop(self): self.sb(0x3D1, 0); self.reset()       # AH=2 0x470
    def enable(self): self.sb(0x3DD, 1)                    # AH=4 0x47B
    def disable(self):                                     # AH=3 0x483
        self.sb(0x3DD, 0)
        for v in range(9): self.out(0xB0 + v, 0)

    def patch(self, prog, v):                              # 0x7FB
        p = self.w(0x167 + ((self.b(0x14F + prog) << 1) & 0xFF))
        mo, co = self.b(0x34A + v), self.b(0x353 + v)
        for k, r in enumerate((0x20 + mo, 0x40 + mo, 0x60 + mo, 0x80 + mo, 0x20 + co, 0x40 + co, 0x60 + co, 0x80 + co,
                               0xC0 + v)):
            self.out(r, self.b(p + k))
        self.sb(0x39B + v, self.b(p + 5)); self.sb(0x377 + v, self.b(p + 9))
        self.out(0xE0 + mo, self.b(p + 10)); self.out(0xE0 + co, self.b(p + 11))

    def freq(self, idx): return self.w(0x14 + ((idx << 1) & 0xFF))   # shl bl: 8 bits

    def voice(self, v, bx, vel):                           # 0x73F
        if self.b(0x3DD) != 1: self.sb(0x3D2, 0)
        if self.b(0x3D2): self.out(0x40 + self.b(0x353 + v), self.b(0x39B + v) - (vel >> 2))
        self.out(0xA0 + v, bx); self.sb(0x3A4 + v, bx)
        b0 = (bx >> 8) | self.b(0x3D2)
        self.out(0xB0 + v, b0); self.sb(0x3AD + v, b0)

    def bend(self, v):                                     # 0x7A3 (no song bends)
        ah = self.b(0x3AD + v); self.sb(0x3D7, ah & 0xFC)
        r = ((((ah & 3) << 8 | self.b(0x3A4 + v)) * self.w(0x3D5)) >> 13) & 0xFFFF
        self.out(0xA0 + v, r); self.out(0xB0 + v, (r >> 8) | self.b(0x3D7))

    def tick(self):                                        # AH=0 0x4A9
        if self.b(0x3D1) != 1: return
        s, si = self.s, self.w(0x3C8)
        for v in range(9):
            if self.b(0x365 + v) == 1: self.sw(0x3B6 + 2 * v, self.w(0x3B6 + 2 * v) + 1)
        if self.b(0x3CE):
            self.sb(0x3CE, self.b(0x3CE) - 1); return
        while True:
            al = s[si]; si += 1
            if al < 0x80:                                  # running status
                si -= 1; kind = self.b(0x3CC)
                if self.b(0x3CF):
                    si += 2 if self.b(0x3D0) else 1; kind = None
            elif al == 0xFC:
                if s[si] == 0x80: si = self.w(0x3CA); continue
                self.sb(0x3D1, 0); return
            else:
                self.sb(0x3CD, al & 15); self.sb(0x3CC, al & 0xF0); kind = al & 0xF0
            ch = self.b(0x3CD)
            if kind == 0xE0:
                self.sb(0x3D0, 1)
                dl, dh = s[si], s[si + 1]; si += 2
                self.sw(0x3D5, (dl | (dh & 1) << 7) | (dh >> 1) << 8)
                for v in range(9):
                    if ch == self.b(0x35C + v) and self.b(0x365 + v) == 1: self.bend(v)
            elif kind is not None:
                self.sb(0x3CF, 0); self.sb(0x3D0, 1)
                if kind == 0xC0:
                    self.sb(0x3D0, 0); prog = s[si]; si += 1
                    for v in range(9):
                        if ch != 9 and ch == self.b(0x35C + v): self.patch(prog, v)
                elif kind == 0x90 and s[si + 1]:
                    note, vel = s[si], s[si + 1]; si += 2
                    if ch == 9:
                        for v in range(9):
                            if ch == self.b(0x35C + v) and note == self.b(0x380 + v):
                                self.sb(0x3D2, 0x20); self.sb(0x365 + v, 1); self.sb(0x36E + v, note)
                                self.voice(v, self.freq(self.b(0x389 + v)), vel); break
                    else:
                        for v in range(9):
                            if ch == self.b(0x35C + v) and self.b(0x365 + v) != 1: break
                        else:
                            dx, v = 0, 0xFFFF
                            for k in range(9):
                                if ch == self.b(0x35C + k) and dx < self.w(0x3B6 + 2 * k): dx, v = self.w(0x3B6 + 2 * k), k
                            self.sw(0x3D3, v)
                            if v != 0xFFFF:
                                self.sb(0x3D2, 0); self.voice(v, 0, vel); self.sw(0x3B6 + 2 * v, 0)
                        if v != 0xFFFF:
                            self.sb(0x3D2, 0x20); self.sb(0x365 + v, 1); self.sb(0x36E + v, note)
                            self.voice(v, self.freq(note + self.b(0x377 + v)), vel)
                elif kind in (0x80, 0x90):
                    note = s[si]; si += 2
                    for v in range(9):
                        if ch == self.b(0x35C + v) and self.b(0x36E + v) == note:
                            self.sb(0x3D2, 0); self.sb(0x365 + v, 0)
                            n = self.b(0x389 + v) if ch == 9 else note
                            self.sb(0x36E + v, 0); self.sw(0x3B6 + 2 * v, 0)
                            self.voice(v, self.freq(n + self.b(0x377 + v)), 0); break
                else:
                    si += 2; self.sb(0x3D0, 1); self.sb(0x3CF, 1)
            dl = s[si]; si += 1
            if dl:
                self.sb(0x3CE, dl - 1); break
        self.sw(0x3C8, si)


def check_dumps():
    res = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'dosbox', 'results')
    image = member('CENT', 'adlib')
    for dump in ('mem_race_640k.bin', 'mem_map_gamemenu_640k.bin', 'mem_narbonensis_province_640k.bin'):
        m = open(os.path.join(res, dump), 'rb').read()
        ds = m[0x16860:0x26860]
        live = m[(ds[0xC6] | ds[0xC7] << 8) * 16:][:len(image)]
        n = ds[0x3EEE] if ds[0x3EEE] != 0xFF else 1          # stopped: the map song (slot 0 holds song 1)
        song = member(SONGS[n][1], SONGS[n][0])
        seg = live[0x12] | live[0x13] << 8
        print('%s: driver code == ADLIB.LZW: %s; song %d %s in RAM == file: %s' % (
            dump, live[0x3E8:] == image[0x3E8:], n, SONGS[n][0], m[seg * 16:seg * 16 + len(song)] == song))
        d = Driver(image, song); d.start()
        for t in range(1, 20000):
            d.tick()
            if d.w(0x3C8) == (live[0x3C8] | live[0x3C9] << 8) and d.b(0x3CE) == live[0x3CE]:
                diff = [a for a in range(0x35C, 0x3D8) if a != 0x3D1 and d.m[a] != live[a]]
                print('  first match at tick %d: driver bytes 35C..3D7 differing (3D1 aside): %s' % (
                    t, ' '.join('%03X' % a for a in diff) or 'none'))
                break


def main():
    if sys.argv[1:2] == ['--check-dumps']: return check_dumps()
    song = sys.argv[1]
    n = int(song) if song.isdigit() else [x[0] for x in SONGS].index(song.lower())
    ticks = int(float(sys.argv[2]) * 1193182 / 16383)
    t = [0]
    d = Driver(member('CENT', 'adlib'), member(SONGS[n][1], SONGS[n][0]),
               lambda r, v: sys.stdout.write('%d %02X %02X\n' % (t[0], r, v)))
    d.start()
    for t[0] in range(1, ticks + 1):
        d.tick()


if __name__ == '__main__':
    main()
