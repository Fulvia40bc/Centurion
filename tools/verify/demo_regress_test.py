"""Tests for demo_regress.py with the fake port (no game data, no real advciv.exe). Run: python -I demo_regress_test.py

Uses a synthetic game folder in a temp dir (an empty ACIV.CT1, an ACIV.CFG and one SAVGAMEA.DAT) and
tools/verify/fake_advciv.py as the EXE (a .py path is run as [sys.executable, '-I', path, ...]).
"""
import os
import shutil
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
DR = os.path.join(HERE, 'demo_regress.py')
FAKE = os.path.join(HERE, 'fake_advciv.py')

CFG0 = b'fake cfg bytes\n'
SAVE0 = b'original autosave\n'

fails = 0


def check(cond, msg):
    global fails
    if not cond:
        fails += 1
        print('FAIL', msg)


def run(args, env_extra=None, cwd=None):
    env = dict(os.environ)
    if env_extra:
        env.update(env_extra)
    return subprocess.run([sys.executable, '-I', DR] + args, capture_output=True, text=True, env=env, cwd=cwd)


def make_gamedir(tmp):
    g = os.path.join(tmp, 'game')
    os.makedirs(g)
    open(os.path.join(g, 'ACIV.CT1'), 'wb').write(b'')
    open(os.path.join(g, 'ACIV.CFG'), 'wb').write(CFG0)
    open(os.path.join(g, 'SAVGAMEA.DAT'), 'wb').write(SAVE0)
    return g


def game_state_ok(g):
    """ACIV.CFG and SAVGAMEA.DAT have their starting bytes and savgameb.dat is gone."""
    ok = True
    if open(os.path.join(g, 'ACIV.CFG'), 'rb').read() != CFG0:
        ok = False
    if open(os.path.join(g, 'SAVGAMEA.DAT'), 'rb').read() != SAVE0:
        ok = False
    if os.path.exists(os.path.join(g, 'savgameb.dat')):
        ok = False
    if os.path.exists(os.path.join(g, 'SAVGAMEB.DAT')):
        ok = False
    return ok


def main():
    tmp = tempfile.mkdtemp(prefix='demo_regress_test_')
    try:
        g = make_gamedir(tmp)
        ref = os.path.join(tmp, 'ref')
        fake_b = os.path.join(tmp, 'fake_b.py')
        shutil.copy(FAKE, fake_b)
        empty_mask = os.path.join(tmp, 'empty_mask.txt')
        open(empty_mask, 'w').write('# nothing\n')

        # 1. record two seeds, then check = equal (fails if the game folder is restored only at the end:
        #    the fake's dumps derive from ACIV.CFG's bytes at run start)
        r = run(['record', FAKE, g, ref, '--seeds', '0x28132309,0x42120E34'])
        check(r.returncode == 0, f'record: exit {r.returncode} {r.stderr}')
        check(os.path.isdir(os.path.join(ref, '0x28132309')) and os.path.exists(os.path.join(ref, 'manifest.json')),
              'record: seed folder / manifest missing')
        check(game_state_ok(g), 'record left the game folder dirty')
        r = run(['check', FAKE, g, ref])
        check(r.returncode == 0, f'check: exit {r.returncode} {r.stdout} {r.stderr}')
        check('seed 0x28132309: 5 rounds and the end, all equal' in r.stdout
              and 'seed 0x42120E34: 5 rounds and the end, all equal' in r.stdout, f'check lines: {r.stdout}')
        check(game_state_ok(g), 'check left the game folder dirty')

        # 2. the clock difference is masked: two checks a second apart are equal with the default mask...
        time.sleep(1.1)
        r = run(['check', FAKE, g, ref, '--seeds', '0x28132309'])
        check(r.returncode == 0 and 'all equal' in r.stdout, f'masked clock: exit {r.returncode} {r.stdout}')
        # ...but not with an empty mask (the difference is at 0xF422C)
        time.sleep(1.1)
        r = run(['check', FAKE, g, ref, '--seeds', '0x28132309', '--mask', empty_mask])
        check(r.returncode == 1 and 'F422C' in r.stdout, f'unmasked clock: exit {r.returncode} {r.stdout}')
        check(game_state_ok(g), 'clock cases left the game folder dirty')

        # 3. a one-byte difference is found at the right round and address (0x100 + 0xB0000 = 0xB0100), also in final
        r = run(['check', FAKE, g, ref, '--seeds', '0x28132309'], {'FAKE_DIFF_ROUND': '3'})
        check(r.returncode == 1 and 'first difference in round 3' in r.stdout and 'B0100' in r.stdout,
              f'diff round 3: exit {r.returncode} {r.stdout}')
        r = run(['check', FAKE, g, ref, '--seeds', '0x28132309'], {'FAKE_DIFF_ROUND': 'final'})
        check(r.returncode == 1 and 'first difference in final' in r.stdout and 'B0100' in r.stdout,
              f'diff final: exit {r.returncode} {r.stdout}')
        check(game_state_ok(g), 'diff cases left the game folder dirty')

        # 4. a shorter game is reported
        r = run(['check', FAKE, g, ref, '--seeds', '0x28132309'], {'FAKE_ROUNDS': '3'})
        check(r.returncode == 1 and 'ended after 5 rounds in the reference, 3 here' in r.stdout,
              f'shorter game: exit {r.returncode} {r.stdout}')
        check(game_state_ok(g), 'rounds case left the game folder dirty')

        # 5. exit 3, a hang and FAKE_NO_WRITE are failures (exit 2)
        r = run(['check', FAKE, g, ref, '--seeds', '0x28132309'], {'FAKE_EXIT': '3'})
        check(r.returncode == 2 and 'run failed' in r.stdout and 'exit 3' in r.stdout,
              f'exit 3: exit {r.returncode} {r.stdout}')
        r = run(['check', FAKE, g, ref, '--seeds', '0x28132309', '--timeout', '3'], {'FAKE_HANG': '1'})
        check(r.returncode == 2 and 'timeout' in r.stdout, f'hang: exit {r.returncode} {r.stdout}')
        r = run(['check', FAKE, g, ref, '--seeds', '0x28132309'], {'FAKE_NO_WRITE': '2'})
        check(r.returncode == 2 and 'cannot write' in r.stdout, f'no_write: exit {r.returncode} {r.stdout}')
        check(game_state_ok(g), 'failure cases left the game folder dirty')

        # 6. seeds 42120e34 and 0x42120E34 find the same reference
        r = run(['check', FAKE, g, ref, '--seeds', '42120e34'])
        check(r.returncode == 0 and 'seed 0x42120E34: 5 rounds and the end, all equal' in r.stdout,
              f'seed spelling: exit {r.returncode} {r.stdout}')

        # 7. xarch: fake vs an unmodified copy = equal; fake vs fake_b with FAKE_ONLY=fake_b.py FAKE_DIFF_ROUND=5 = round 5
        r = run(['xarch', FAKE, FAKE, g, '--seeds', '0x28132309'])
        check(r.returncode == 0 and 'all equal' in r.stdout, f'xarch same: exit {r.returncode} {r.stdout}')
        r = run(['xarch', FAKE, fake_b, g, '--seeds', '0x28132309'],
                {'FAKE_ONLY': 'fake_b.py', 'FAKE_DIFF_ROUND': '5'})
        check(r.returncode == 1 and 'first difference in round 5' in r.stdout,
              f'xarch diff: exit {r.returncode} {r.stdout}')
        check(game_state_ok(g), 'xarch cases left the game folder dirty')

        # 8. refusals exit 4: no ACIV.CT1, missing EXE, existing seed folder without --force, usage error
        nogame = os.path.join(tmp, 'nogame')
        os.makedirs(nogame)
        r = run(['check', FAKE, nogame, ref])
        check(r.returncode == 4, f'no ACIV.CT1: exit {r.returncode}')
        r = run(['check', os.path.join(tmp, 'nope.exe'), g, ref])
        check(r.returncode == 4, f'missing exe: exit {r.returncode}')
        r = run(['record', FAKE, g, ref, '--seeds', '0x28132309'])
        check(r.returncode == 4, f'existing seed folder: exit {r.returncode}')
        r = run(['record', FAKE, g, ref, '--seeds', '0x28132309', '--force'])
        check(r.returncode == 0, f'--force: exit {r.returncode} {r.stderr}')
        r = run([])
        check(r.returncode == 4, f'usage error: exit {r.returncode}')
        r = run(['frobnicate', FAKE, g, ref])
        check(r.returncode == 4, f'unknown command: exit {r.returncode}')
        check(game_state_ok(g), 'refusal cases left the game folder dirty')
    finally:
        shutil.rmtree(tmp, ignore_errors=True)
    if fails == 0:
        print('demo_regress_test: all checks pass')
    else:
        print(f'demo_regress_test: {fails} FAILURES')
    return 1 if fails else 0


if __name__ == '__main__':
    sys.exit(main())
