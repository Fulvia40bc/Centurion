"""Check the port's battle drawing against the game's own frame buffer (Claude, 2026-11-09).

usage (from the game folder): python cmp_battle_frame.py DS.bin ZONE FRAME.bin
DS = DS image taken during a battle (e.g. battle/b11_orders1_open.bin, at 0823:D14A when the orders mode opened, so the
sprite list of the tick is complete); FRAME = the game's frame buffer at that moment (640K snapshot offset 0x26870).
Runs centurion --battle-frame and counts differing pixels (expected 0).
"""
import subprocess, sys, os, tempfile
EXE = r'C:\work\Centurion-native\centurion.exe'
out = os.path.join(tempfile.gettempdir(), 'battle_frame.bin')
r = subprocess.run([EXE, '--battle-frame', sys.argv[1], sys.argv[2], out], capture_output=True, text=True)
sys.stdout.write(r.stderr)
a, b = open(out, 'rb').read(), open(sys.argv[3], 'rb').read()
print('%d differing pixels' % sum(1 for i in range(64000) if a[i] != b[i]))
