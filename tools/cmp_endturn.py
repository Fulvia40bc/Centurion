"""Compare the port's End Turn with the real game's.

usage: python cmp_endturn.py mem_X_in.bin mem_X_out.bin   (run from the game folder, which holds OVL0.OVL)
mem_X_in/out = 640K DOSBox snapshots at End Turn entry (0823:09BA) and exit (0823:0D0D), from hunt/turn.ps1.
Runs `centurion --end-turn mem_X_in.bin port_ds.bin` and compares the DS images: every save block, plus the state a
save leaves out (dice DS:00C8-00CB, war chance DS:3D9C, Rome's garrison pseudo-legion DS:1A08-1A1B).
"""
import subprocess, sys, os, tempfile

DS = 0x16860
EXE = r'C:\work\Centurion-native\centurion.exe'


def ds_of(path):
    with open(path, 'rb') as f:   # a 640K snapshot, or a bare 64K DS image
        if os.path.getsize(path) != 0x10000:
            f.seek(DS)
        return f.read(0x10000)


def main():
    pin, pout = sys.argv[1], sys.argv[2]
    port_path = os.path.join(tempfile.gettempdir(), 'port_ds.bin')
    r = subprocess.run([EXE, '--end-turn', pin, port_path], capture_output=True, text=True)
    sys.stdout.write(r.stderr)
    port, real, before = open(port_path, 'rb').read(), ds_of(pout), ds_of(pin)
    w = lambda a: before[a] | before[a + 1] << 8
    ranges = [(w(0x5BDE + 4 * i), w(0x5BE0 + 4 * i)) for i in range(24)]
    ranges += [(0x00C8, 4), (0x3D9C, 1), (0x1A08, 20)]
    bad = changed = 0
    for a, n in ranges:
        for x in range(a, a + n):
            if real[x] != before[x]:
                changed += 1
            if port[x] != real[x]:
                bad += 1
                if bad <= 40:
                    print('  DS:%04X  before %3d  game %3d  port %3d' % (x, before[x], real[x], port[x]))
    print('turn %d -> %d: %d bytes changed by the game, %d differ in the port'
          % (before[0x634C] | before[0x634D] << 8, real[0x634C] | real[0x634D] << 8, changed, bad))


main()
