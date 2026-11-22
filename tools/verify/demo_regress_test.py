"""Tests for demo_regress.py with the fake port (no game data, no real advciv.exe). Run: python -I demo_regress_test.py

Uses a synthetic game folder in a temp dir (an empty ACIV.CT1, an ACIV.CFG and one SAVGAMEA.DAT) and
tools/verify/fake_advciv.py as the EXE (a .py path is run as [sys.executable, '-I', path, ...]).
"""
import hashlib
import json
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
import demo_regress as DRMOD

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
        # 19b #9: check the second seed alone - record and check ran the seeds in the same order, so this only
        # passes if the game folder is restored before every run, not just at the end
        r = run(['check', FAKE, g, ref, '--seeds', '0x42120E34'])
        check(r.returncode == 0 and 'seed 0x42120E34: 5 rounds and the end, all equal' in r.stdout,
              f'check second seed alone: exit {r.returncode} {r.stdout} {r.stderr}')
        check(game_state_ok(g), 'second-seed check left the game folder dirty')

        # 19b #12: the manifest's masked SHA-256 equals the SHA-256 of the masked dump file
        man = json.load(open(os.path.join(ref, 'manifest.json')))
        entry = man['seeds']['0x28132309']
        ranges = DRMOD.read_mask(DRMOD.DEFAULT_MASK)
        raw = open(os.path.join(ref, '0x28132309', 'r_1.bin'), 'rb').read()
        check(hashlib.sha256(DRMOD.masked(raw, ranges)).hexdigest() == entry['dumps']['r_1.bin'],
              'manifest masked hash mismatch')

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
        # 19b #2: a different round count must not hide the first difference (round 2 differs, 3 rounds here)
        r = run(['check', FAKE, g, ref, '--seeds', '0x28132309'], {'FAKE_ROUNDS': '3', 'FAKE_DIFF_ROUND': '2'})
        check(r.returncode == 1 and 'first difference in round 2' in r.stdout
              and 'ended after 5 rounds in the reference, 3 here' in r.stdout,
              f'count hides difference: exit {r.returncode} {r.stdout}')
        # 19b #10: a divergence from round K on is reported at round K, not at the last differing round
        r = run(['check', FAKE, g, ref, '--seeds', '0x28132309'], {'FAKE_DIFF_FROM': '3'})
        check(r.returncode == 1 and 'first difference in round 3' in r.stdout
              and 'round 4' not in r.stdout.split('first difference')[1][:40],
              f'diff-from: exit {r.returncode} {r.stdout}')
        # 19b #12: several runs in one dump - merged (gaps < 4), capped at 12 printed, run's own bytes shown
        r = run(['check', FAKE, g, ref, '--seeds', '0x28132309'], {'FAKE_MULTI_DIFF': '1'})
        check(r.returncode == 1 and '16 differing byte run(s)' in r.stdout and '0B0100..0B0105 (  6)' in r.stdout
              and '... 4 more run(s)' in r.stdout, f'multi-diff: exit {r.returncode} {r.stdout}')
        # the run's own bytes only, first 16 (here 6): not 16 bytes from the run start
        check('ref a6d7b197be63  new 59d74e97be9c' in r.stdout, f'multi-diff values: {r.stdout}')
        # 19b #11: each run failure rule alone trips exit 2
        r = run(['check', FAKE, g, ref, '--seeds', '0x28132309'], {'FAKE_CANNOT_WRITE_LINE': '1'})
        check(r.returncode == 2 and 'cannot write' in r.stdout, f'cannot-write-line only: exit {r.returncode} {r.stdout}')
        r = run(['check', FAKE, g, ref, '--seeds', '0x28132309'], {'FAKE_GAP': '2'})
        check(r.returncode == 2 and 'gap' in r.stdout, f'gap only: exit {r.returncode} {r.stdout}')
        r = run(['check', FAKE, g, ref, '--seeds', '0x28132309'], {'FAKE_SHORT': '2'})
        check(r.returncode == 2 and '470015 bytes' in r.stdout, f'short dump only: exit {r.returncode} {r.stdout}')
        check(game_state_ok(g), 'rule cases left the game folder dirty')
        # 19b #1: a damaged reference is refused with exit 4, naming the file
        rp = os.path.join(ref, '0x28132309', 'r_1.bin')
        orig = open(rp, 'rb').read()
        with open(rp, 'r+b') as f:
            f.seek(0x200)
            f.write(bytes([orig[0x200] ^ 0xFF]))
        r = run(['check', FAKE, g, ref, '--seeds', '0x28132309'])
        check(r.returncode == 4 and 'reference damaged' in (r.stdout + r.stderr) and 'r_1.bin' in (r.stdout + r.stderr),
              f'damaged ref: exit {r.returncode} {r.stdout} {r.stderr}')
        with open(rp, 'wb') as f:
            f.write(orig)
        os.remove(os.path.join(ref, '0x28132309', 'final.bin'))
        r = run(['check', FAKE, g, ref, '--seeds', '0x28132309'])
        check(r.returncode == 4 and 'reference damaged' in (r.stdout + r.stderr),
              f'missing final in ref: exit {r.returncode} {r.stdout} {r.stderr}')
        # put the reference back
        r = run(['record', FAKE, g, ref, '--seeds', '0x28132309', '--force'])
        check(r.returncode == 0, f're-record: exit {r.returncode} {r.stderr}')
        r = run(['check', FAKE, g, ref, '--seeds', '0x28132309'])
        check(r.returncode == 0, f'check after re-record: exit {r.returncode} {r.stdout}')

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

        # 19b #4/#5: --keep is made absolute (the port chdirs into GAMEDIR); xarch keeps a/ and b/ subfolders
        r = run(['check', FAKE, g, ref, '--seeds', '0x28132309', '--keep', 'kept'], cwd=tmp)
        check(r.returncode == 0 and os.path.exists(os.path.join(tmp, 'kept', '0x28132309', os.path.basename(FAKE), 'r_1.bin')),
              f'--keep relative: exit {r.returncode} {r.stdout} {r.stderr}')
        check(game_state_ok(g), '--keep case left the game folder dirty')
        r = run(['xarch', FAKE, fake_b, g, '--seeds', '0x28132309', '--keep', 'keptx'], cwd=tmp)
        check(r.returncode == 0 and os.path.isdir(os.path.join(tmp, 'keptx', '0x28132309', 'a'))
              and os.path.isdir(os.path.join(tmp, 'keptx', '0x28132309', 'b')),
              f'xarch --keep a/b: exit {r.returncode} {r.stdout} {r.stderr}')

        # 19b #6: refusals exit 4 (bad EXE not a file, bad mask line, corrupt manifest); unexpected error exits 2
        # with the traceback
        r = run(['check', g, g, ref])
        check(r.returncode == 4, f'exe is a folder: exit {r.returncode}')
        badmask = os.path.join(tmp, 'badmask.txt')
        open(badmask, 'w').write('ZZZ 4 nope\n')
        r = run(['check', FAKE, g, ref, '--mask', badmask])
        check(r.returncode == 4 and 'bad mask line' in (r.stdout + r.stderr), f'bad mask: exit {r.returncode}')
        ref2 = os.path.join(tmp, 'ref2')
        shutil.copytree(ref, ref2)
        open(os.path.join(ref2, 'manifest.json'), 'w').write('{not json')
        r = run(['check', FAKE, g, ref2])
        check(r.returncode == 4 and 'bad manifest' in (r.stdout + r.stderr), f'corrupt manifest: exit {r.returncode}')
        afile = os.path.join(tmp, 'afile')
        open(afile, 'w').write('x')
        r = run(['record', FAKE, g, afile])
        check(r.returncode == 2 and 'Traceback' in r.stderr, f'unexpected error: exit {r.returncode} {r.stderr[:80]}')

        # 19b #3: record --force is atomic - a failure while writing the new folder leaves the old reference
        # and the manifest intact
        import builtins
        from unittest import mock
        real_open = builtins.open
        ncalls = {'n': 0}

        def flaky_open(path, *a, **k):
            if '.tmp' in str(path) and str(path).endswith('.bin'):
                ncalls['n'] += 1
                if ncalls['n'] == 3:
                    raise OSError('simulated disk full')
            return real_open(path, *a, **k)

        old_argv = sys.argv
        sys.argv = ['demo_regress.py', 'record', FAKE, g, ref, '--seeds', '0x28132309', '--force']
        try:
            with mock.patch('builtins.open', flaky_open):
                try:
                    rc = DRMOD.main()
                except SystemExit as e:
                    rc = e.code
        finally:
            sys.argv = old_argv
        check(rc == 2, f'atomic record failure: rc {rc}')
        man = json.load(open(os.path.join(ref, 'manifest.json')))
        e = man['seeds']['0x28132309']
        raw = open(os.path.join(ref, '0x28132309', 'r_1.bin'), 'rb').read()
        rr = DRMOD.read_mask(os.path.join(HERE, 'port_volatile.txt'))
        check(hashlib.sha256(DRMOD.masked(raw, rr)).hexdigest() == e['dumps']['r_1.bin'],
              'old reference intact after failed --force')
        check(not any(f.startswith('.0x') for f in os.listdir(ref)), 'temp seed folder left behind')

        # TASK 21: a failed swap of a re-recorded seed folder leaves no leftover - the new recording's
        # .<seed>.tmp<pid> folder is removed, an already-renamed .<seed>.old<pid> is renamed back, the manifest
        # stays as it was, exit 2 with the traceback
        def run_record_force():
            old_argv = sys.argv
            sys.argv = ['demo_regress.py', 'record', FAKE, g, ref, '--seeds', '0x28132309', '--force']
            try:
                try:
                    return DRMOD.main()
                except SystemExit as e:
                    return e.code
            finally:
                sys.argv = old_argv

        def refdir_state():
            st = {}
            for fn in sorted(os.listdir(ref)):
                p = os.path.join(ref, fn)
                if os.path.isfile(p):
                    st[fn] = open(p, 'rb').read()
                else:
                    st[fn] = {f: open(os.path.join(p, f), 'rb').read() for f in sorted(os.listdir(p))}
            return st

        state_before = refdir_state()
        sdir_path = os.path.join(ref, '0x28132309')
        real_rename = os.rename

        # (a) the first rename (old seed folder -> .old<pid>) fails
        def fail_old_rename(src, dst):
            if src == sdir_path:
                raise OSError(5, 'simulated WinError 5: a file of the old folder is held open')
            return real_rename(src, dst)

        with mock.patch('os.rename', fail_old_rename):
            rc = run_record_force()
        check(rc == 2, f'swap failure, old rename: rc {rc}')
        check(refdir_state() == state_before, 'swap failure, old rename: REFDIR unchanged (old seed folder, old manifest, no .tmp/.old)')
        check(game_state_ok(g), 'swap failure, old rename: game folder restored')

        # (b) the second rename (tmp_new -> seed folder) fails after the old folder was renamed away
        def fail_new_rename(src, dst):
            if dst == sdir_path and '.tmp' in src:
                raise OSError(5, 'simulated WinError 5: a file of the new recording is held open')
            return real_rename(src, dst)

        with mock.patch('os.rename', fail_new_rename):
            rc = run_record_force()
        check(rc == 2, f'swap failure, new rename: rc {rc}')
        check(refdir_state() == state_before, 'swap failure, new rename: REFDIR holds exactly the old seed folder (same bytes) and the old manifest')
        check(game_state_ok(g), 'swap failure, new rename: game folder restored')

        # 19b #7: on any exception while the port runs (Ctrl+C), the child is killed and waited for
        class FakeP:
            def __init__(self):
                self.killed = False
                self.waited = False

            def communicate(self, timeout=None):
                raise KeyboardInterrupt

            def kill(self):
                self.killed = True

            def wait(self):
                self.waited = True
                return 0

        fp = FakeP()
        with mock.patch.object(DRMOD.subprocess, 'Popen', lambda *a, **k: fp):
            try:
                DRMOD.run_port(FAKE, g, os.path.join(tmp, 'wp'), 0x28132309, 600)
                check(False, 'KeyboardInterrupt not propagated')
            except KeyboardInterrupt:
                pass
        check(fp.killed and fp.waited, 'child killed and waited on KeyboardInterrupt')

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
