"""fake_advciv.py - a stand-in for advciv.exe in demo_regress_test.py (Task 19).

It parses the same command line the port takes for a headless demo run:

    fake_advciv.py GAMEDIR --srd 8 --seed 0x28132309 --play --dump-every 0x55258 PREFIX \
                  --dump-ds FINAL --stop-at 0x14294 1

It does what the port does to the game folder (os.chdir(GAMEDIR) first; exit 1 if GAMEDIR is relative): appends
one byte to ACIV.CFG, writes SAVGAMEA.DAT, creates savgameb.dat (lower case). Then it writes N end-of-round dumps
PREFIX1.bin..PREFIXN.bin and FINAL, each 470,016 bytes, derived from the seed, the round and the bytes ACIV.CFG had
at the start of the run (so a missed restore changes the dumps), with the real time of day at file offset
0xF422C - 0xB0000. Finally it prints the stop line on stderr and exits 0.

Misbehaviour comes from environment variables, applied only when FAKE_ONLY is unset or equals this file's own name
(so a test can copy the fake to fake_b.py and break only that copy):
  FAKE_DIFF_ROUND=k   change one byte in dump k (a round number, or "final")
  FAKE_ROUNDS=n       write n round dumps instead of the default 5
  FAKE_EXIT=3         exit with that code (after the game-folder side effects, before any dump)
  FAKE_HANG=1         sleep forever (the runner's timeout kills it)
  FAKE_NO_WRITE=k     print "advciv: cannot write ..." for round k and skip that dump (a gap)
"""
import hashlib
import os
import struct
import sys
import time

DS_SIZE = 470016
CLOCK_OFF = 0xF422C - 0xB0000          # file offset of the time-of-day dword in a data-object dump
DEFAULT_ROUNDS = 5
DIFF_OFF = 0x100                        # where FAKE_DIFF_ROUND flips a byte


def fake_on():
    fo = os.environ.get('FAKE_ONLY')
    return fo is None or fo == os.path.basename(os.path.abspath(__file__))


def env(name, default=None):
    v = os.environ.get(name)
    return v if (v not in (None, '') and fake_on()) else default


def dump_bytes(seed, k, cfg):
    h = hashlib.sha256(f'0x{seed:08X}|{k}|'.encode() + cfg).digest()
    buf = bytearray((h * (DS_SIZE // 32 + 1))[:DS_SIZE])
    struct.pack_into('<I', buf, CLOCK_OFF, int(time.time()) & 0xFFFFFFFF)
    return buf


def main():
    args = sys.argv[1:]
    if not args:
        print('usage: fake_advciv.py GAMEDIR [--srd N] [--seed HEX] [--play] --dump-every ADDR PREFIX '
              '--dump-ds FILE [--stop-at ADDR N]', file=sys.stderr)
        return 2
    gamedir = args[0]
    if not os.path.isabs(gamedir):
        print('fake_advciv: GAMEDIR must be absolute', file=sys.stderr)
        return 1
    seed, prefix, final = None, None, None
    i = 1
    while i < len(args):
        a = args[i]
        if a == '--seed':
            seed = int(args[i + 1], 16)
            i += 2
        elif a == '--dump-every':
            prefix = args[i + 2]
            i += 3
        elif a == '--dump-ds':
            final = args[i + 1]
            i += 2
        elif a in ('--srd', '--stop-at'):
            i += 3 if a == '--stop-at' else 2
        else:
            i += 1
    if seed is None or prefix is None or final is None:
        print('fake_advciv: need --seed, --dump-every ADDR PREFIX and --dump-ds FILE', file=sys.stderr)
        return 2

    os.chdir(gamedir)
    # the game-folder side effects, before any misbehaviour
    cfg_path = 'ACIV.CFG'
    cfg = open(cfg_path, 'rb').read() if os.path.exists(cfg_path) else b''
    with open(cfg_path, 'ab') as f:
        f.write(b'\x00')
    open('SAVGAMEA.DAT', 'wb').write(b'fake autosave\n')
    open('savgameb.dat', 'wb').write(b'fake round-end save\n')      # lower case, like the port

    if env('FAKE_EXIT'):
        return int(env('FAKE_EXIT'))
    if env('FAKE_HANG'):
        time.sleep(3600)
        return 0
    rounds = int(env('FAKE_ROUNDS', str(DEFAULT_ROUNDS)))
    diff_round = env('FAKE_DIFF_ROUND')
    no_write = env('FAKE_NO_WRITE')

    os.makedirs(os.path.dirname(prefix) or '.', exist_ok=True)
    os.makedirs(os.path.dirname(final) or '.', exist_ok=True)
    for k in range(1, rounds + 1):
        if no_write is not None and str(k) == no_write:
            print(f'advciv: cannot write {prefix}{k}.bin', file=sys.stderr)
            continue
        buf = dump_bytes(seed, k, cfg)
        if diff_round == str(k):
            buf[DIFF_OFF] ^= 0xFF
        with open(f'{prefix}{k}.bin', 'wb') as f:
            f.write(buf)
    buf = dump_bytes(seed, 'final', cfg)
    if diff_round == 'final':
        buf[DIFF_OFF] ^= 0xFF
    with open(final, 'wb') as f:
        f.write(buf)
    print('advciv: stop at 0x14294 #1', file=sys.stderr)
    return 0


if __name__ == '__main__':
    sys.exit(main())
