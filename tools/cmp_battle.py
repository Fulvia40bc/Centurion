"""Check the port's land battle against a real one (Claude, 2026-11-09).

usage (from the game folder, which holds BATL.DAT):
  python cmp_battle.py FORM.bin EXIT.bin army legion zone formation tactic
FORM = DS image at the Choose Formation menu, EXIT = DS image at the battle's exit (0823:E212), both from DOSBox.
Two runs:
  1. --battle-from-formation from FORM (setup already done by the game);
  2. --battle (the whole battle from the entry): FORM with the dice rewound by one roll (the enemy formation roll; the LCGs
     are invertible) and the battle counter C0F8 put back, so the port also builds the field, the units and rolls the formation.
Or: python cmp_battle.py --entry ENTRY.bin EXIT.bin army legion zone formation tactic ["T:orders;..."]
  ENTRY = DS image at the battle's entry (0823:DF24); the orders (see centurion --orders) are replayed at their ticks.
  The unit table and C296 pointers are heap addresses: they may differ and are not compared.
Each compares the dice, the legion records, commanders, armies, provinces, counters, battle globals and the unit table.
"""
import subprocess, sys, os, tempfile
EXE = r'C:\work\Centurion-native\centurion.exe'
entry_mode = sys.argv[1] == '--entry'
if entry_mode: sys.argv.pop(1)
form, exitp = sys.argv[1], sys.argv[2]
args = sys.argv[3:8]
orders = ['--orders', sys.argv[8]] if len(sys.argv) > 8 else []
w = lambda d, x: d[x] | d[x + 1] << 8
real = open(exitp, 'rb').read()
HEAP = {0xC292, 0xC293, 0xC296, 0xC297}   # heap addresses of the unit table / strengths (not game state)
CHECK = {'dice': (0xC8, 4), 'legions': (0x1918, 13 * 20), 'commanders': (0x1A1C, 144), 'armies': (0xC112, 196),
         'provinces': (0x5270, 864), 'counters C0F8': (0xC0F8, 4), 'casualty totals': (0xC082, 8),
         'battle globals': (0xC28C, 0x28), 'AA4A-AA4F': (0xAA4A, 6)}

def compare(port, label):
    bad = 0
    for k, (s, n) in CHECK.items():
        d = [x for x in range(s, s + n) if port[x] != real[x] and x not in HEAP]
        bad += len(d)
        if d: print('  %-16s %d bytes differ: %s' % (k, len(d), ' '.join('%04X' % x for x in d[:10])))
    ur, up = w(real, 0xC292), w(port, 0xC292)
    n = w(real, 0xC294) * 18
    d = sum(1 for i in range(n) if real[ur + i] != port[up + i])
    bad += d
    if d: print('  unit table %d bytes differ' % d)
    print('%s: %s' % (label, 'identical' if not bad else '%d bytes differ' % bad))

tmp = tempfile.gettempdir()
out = os.path.join(tmp, 'battle_port.bin')
if entry_mode:   # FORM is a DS image at the battle's entry (0823:DF24): one run, the whole battle, with the replayed orders
    r = subprocess.run([EXE, '--original'] + orders + ['--battle', form, out] + args, capture_output=True, text=True)
    sys.stdout.write(r.stderr); compare(open(out, 'rb').read(), 'whole battle from the entry'); sys.exit(0)
r = subprocess.run([EXE, '--original', '--battle-from-formation', form, out] + args, capture_output=True, text=True)
sys.stdout.write(r.stderr); compare(open(out, 'rb').read(), 'from the formation menu')

d = bytearray(open(form, 'rb').read())
inv = pow(0x7ABD, -1, 0x10000), pow(0x1B0F, -1, 0x10000)
s1, s2 = w(d, 0xC8), w(d, 0xCA)
s1 = ((s1 - 0x1B0F) * inv[0]) & 0xFFFF; s2 = ((s2 - 0x7ABD) * inv[1]) & 0xFFFF
d[0xC8:0xCC] = bytes([s1 & 255, s1 >> 8, s2 & 255, s2 >> 8])
c = w(d, 0xC0F8) - 1; d[0xC0F8:0xC0FA] = bytes([c & 255, c >> 8])
entry = os.path.join(tmp, 'battle_entry.bin'); open(entry, 'wb').write(d)
r = subprocess.run([EXE, '--original', '--battle', entry, out] + args, capture_output=True, text=True)
sys.stdout.write(r.stderr.splitlines()[-1] + '\n'); compare(open(out, 'rb').read(), 'whole battle from the entry')
