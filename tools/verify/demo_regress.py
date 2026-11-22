"""demo_regress.py - regression and two-CPU check of whole demo games (Task 19, M7).

Whole computer-only games (the /SRD8 demo) are the reference check of the port: the data object (470,016 bytes,
link 0xB0000 at file offset 0) is dumped at every end-of-round save and at the game's end. This script plays the
games headless, snapshots the game folder's save/cfg files around every run, masks the volatile ranges
(port_volatile.txt) and compares round by round.

Usage: python -I demo_regress.py record EXE GAMEDIR REFDIR [--seeds S1,S2,...] [--force]
       python -I demo_regress.py check  EXE GAMEDIR REFDIR [--seeds S1,S2,...]
       python -I demo_regress.py xarch  EXE_A EXE_B GAMEDIR [--seeds S1,S2,...]
       options: --mask FILE (default port_volatile.txt next to this script), --timeout S (default 600),
                --keep DIR (keep the new dumps there, one subfolder per seed and program)

  record  keeps the dumps of each seed as REFDIR\\0x<SEED>\\r_<k>.bin and final.bin plus REFDIR\\manifest.json
          (per seed: round count, SHA-256 of each masked dump, the exe's path and SHA-256, the mask file, the
          date). An existing seed folder is refused without --force.
  check   plays each seed (default: every seed in the manifest) and compares with the reference round by round.
  xarch   plays each seed with both programs and compares them (no reference).

An EXE path ending in .py is run as [sys.executable, '-I', path, ...] (tools/verify/fake_advciv.py in the tests).
A run fails on: any exit code other than 0, no stop line on stderr, a stderr line starting "advciv: cannot write",
or dumps that are not exactly r_1..N.bin (no gaps) plus final.bin, each 470,016 bytes.

Exit: the highest code seen - 0 all equal, 1 a difference, 2 a run failed (the port's stderr, last 20 lines).
Refusals (no ACIV.CT1 in GAMEDIR, EXE missing, existing seed folder without --force) and usage errors exit 4.
"""
import argparse
import datetime
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
DS_SIZE = 470016
LINK_BASE = 0xB0000
DEFAULT_MASK = os.path.join(HERE, 'port_volatile.txt')
DEFAULT_SEEDS = ['0x28132309', '0x42120E34', '0x12345678']
STOP_ADDR = '0x14294'
STOP_LINE = f'advciv: stop at {STOP_ADDR} #1'
DUMP_PREFIX = 'r_'
GAMEDIR_MARKER = 'ACIV.CT1'
SAVE_RE = re.compile(r'(?i)^(savgame.*\.dat|aciv\.cfg)$')


class AP(argparse.ArgumentParser):
    def error(self, message):
        print(f'demo_regress: {message}', file=sys.stderr)
        sys.exit(4)


def refuse(msg):
    print(f'demo_regress: {msg}', file=sys.stderr)
    sys.exit(4)


def seed_name(seed):
    return f'0x{seed:08X}'


def parse_seeds(s):
    out = []
    for part in s.split(','):
        part = part.strip()
        if not part:
            continue
        try:
            out.append(int(part, 16))
        except ValueError:
            refuse(f'bad seed {part!r} (hex, with or without 0x)')
    if not out:
        refuse('--seeds: no seeds')
    return out


def read_mask(path):
    """ADDR LEN reason lines (hex link address, decimal or 0x length, # comments), as dscompare.py reads them."""
    if not os.path.exists(path):
        refuse(f'mask file {path} not found')
    ranges = []
    for line in open(path):
        line = line.split('#', 1)[0]
        f = line.split()
        if len(f) >= 2:
            try:
                addr, ln = int(f[0], 16), int(f[1], 0)
            except ValueError:
                refuse(f'bad mask line in {path}: {line!r}')
            if LINK_BASE <= addr and addr + ln <= LINK_BASE + DS_SIZE:
                ranges.append((addr - LINK_BASE, ln))          # file offsets in the dump
    return ranges


def masked(buf, ranges):
    b = bytearray(buf)
    for off, ln in ranges:
        b[off:off + ln] = bytes(ln)
    return bytes(b)


def sha256_file(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(1 << 20), b''):
            h.update(chunk)
    return h.hexdigest()


def snapshot(gamedir):
    snap = {}
    for name in os.listdir(gamedir):
        if SAVE_RE.match(name):
            with open(os.path.join(gamedir, name), 'rb') as f:
                snap[name] = f.read()
    return snap


def restore(gamedir, snap):
    """Rewrite the snapshotted files and delete any other save/cfg file a run created."""
    for name in os.listdir(gamedir):
        if SAVE_RE.match(name) and name not in snap:
            os.remove(os.path.join(gamedir, name))
    for name, data in snap.items():
        with open(os.path.join(gamedir, name), 'wb') as f:
            f.write(data)


class RunFailure(Exception):
    def __init__(self, msg, err=''):
        super().__init__(msg)
        self.err = err


def run_port(exe, gamedir, workdir, seed, timeout):
    """One headless run. Returns (dumps, stderr) with dumps {1: bytes, ..., 'final': bytes}; raises RunFailure."""
    os.makedirs(workdir, exist_ok=True)
    for f in os.listdir(workdir):
        if f.endswith('.bin'):
            os.remove(os.path.join(workdir, f))
    prefix = os.path.join(workdir, DUMP_PREFIX)
    final = os.path.join(workdir, 'final.bin')
    cmd = [exe, gamedir, '--srd', '8', '--seed', seed_name(seed), '--play',
           '--dump-every', '0x55258', prefix, '--dump-ds', final, '--stop-at', STOP_ADDR, '1']
    if exe.lower().endswith('.py'):
        cmd = [sys.executable, '-I', exe] + cmd[1:]
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    try:
        out, err = p.communicate(timeout=timeout)
    except subprocess.TimeoutExpired:
        p.kill()
        out, err = p.communicate()
        raise RunFailure(f'timeout after {timeout} s', err)
    except BaseException:
        p.kill()                       # Ctrl+C too: kill and wait before the caller's finally restores the folder
        p.wait()
        raise
    if p.returncode != 0:
        raise RunFailure(f'exit {p.returncode}', err)
    if STOP_LINE not in err:
        raise RunFailure(f'no stop line ({STOP_LINE!r}) on stderr', err)
    for line in err.splitlines():
        if line.startswith('advciv: cannot write'):
            raise RunFailure(line, err)
    names = sorted((int(m.group(1)) for f in os.listdir(workdir)
                    for m in [re.fullmatch(r'r_(\d+)\.bin', f)] if m))
    if names and names != list(range(1, len(names) + 1)):
        raise RunFailure(f'gap in the dumps: {sorted(set(range(1, names[-1] + 1)) - set(names))}', err)
    if not names:
        raise RunFailure('no round dumps', err)
    if not os.path.exists(final):
        raise RunFailure('final.bin missing', err)
    dumps = {}
    for k in names:
        path = f'{prefix}{k}.bin'
        data = open(path, 'rb').read()
        if len(data) != DS_SIZE:
            raise RunFailure(f'r_{k}.bin is {len(data)} bytes, expected {DS_SIZE}', err)
        dumps[k] = data
    data = open(final, 'rb').read()
    if len(data) != DS_SIZE:
        raise RunFailure(f'final.bin is {len(data)} bytes, expected {DS_SIZE}', err)
    dumps['final'] = data
    return dumps, err


def diff_runs(a, b, ranges):
    """Differing byte runs, skipping masked offsets; runs closer than 4 bytes apart are merged."""
    skip = bytearray(len(a))
    for off, ln in ranges:
        for i in range(off, min(off + ln, len(a))):
            skip[i] = 1
    diff = [i for i in range(len(a)) if a[i] != b[i] and not skip[i]]
    runs = []
    for i in diff:
        if runs and i - runs[-1][1] <= 4:
            runs[-1][1] = i
        else:
            runs.append([i, i])
    return runs


def report_runs(label, a, b, ranges, max_runs=12):
    runs = diff_runs(a, b, ranges)
    print(f'  {label}: {len(runs)} differing byte run(s)')
    for s, e in runs[:max_runs]:
        print(f'    {s + LINK_BASE:06X}..{e + LINK_BASE:06X} ({e - s + 1:3d})  '
              f'ref {a[s:e + 1][:16].hex()}  new {b[s:e + 1][:16].hex()}')
    if len(runs) > max_runs:
        print(f'    ... {len(runs) - max_runs} more run(s)')
    return runs


def ordered_keys(dumps):
    rounds = sorted(k for k in dumps if k != 'final')
    return rounds + (['final'] if 'final' in dumps else [])


def play(exe, gamedir, snap, seed, args, tag, subname=None):
    """Restore the game folder, run once into a work folder (temp, or --keep), restore again on failure too."""
    restore(gamedir, snap)
    if args.keep:
        if subname is None:
            subname = re.sub(r'[^\w.-]', '_', os.path.basename(exe))
        workdir = os.path.join(os.path.abspath(args.keep), seed_name(seed), subname)
    else:
        workdir = tempfile.mkdtemp(prefix='demo_regress_')
    try:
        dumps, err = run_port(exe, gamedir, workdir, seed, args.timeout)
        return dumps, err, workdir, None
    except RunFailure as e:
        return None, e, workdir, str(e)


def run_failed(seed, reason, err):
    print(f'seed {seed_name(seed)}: run failed: {reason}')
    text = getattr(err, 'err', None) or (err.stderr if hasattr(err, 'stderr') else str(err))
    for l in text.splitlines()[-20:]:
        print(f'  | {l}')
    return 2


def common_args(ap):
    ap.add_argument('--seeds', default=None, help='comma-separated hex seeds')
    ap.add_argument('--mask', default=DEFAULT_MASK, help='volatile-ranges file (ADDR LEN reason)')
    ap.add_argument('--timeout', type=int, default=600, help='per-run timeout in seconds')
    ap.add_argument('--keep', default=None, help='keep the new dumps in this folder (one subfolder per seed and program)')


def check_exe(exe):
    if not os.path.isfile(exe):
        refuse(f'exe {exe} is not a file')


def check_gamedir(gamedir):
    if not os.path.isdir(gamedir):
        refuse(f'GAMEDIR {gamedir} is not a folder')
    if not os.path.exists(os.path.join(gamedir, GAMEDIR_MARKER)):
        refuse(f'no {GAMEDIR_MARKER} in {gamedir}')


def load_manifest(refdir):
    path = os.path.join(refdir, 'manifest.json')
    if not os.path.exists(path):
        refuse(f'no manifest.json in {refdir} (run record first)')
    try:
        return json.load(open(path))
    except (OSError, ValueError) as e:
        refuse(f'bad manifest.json in {refdir}: {e}')


def cmd_record(args):
    exe = os.path.abspath(args.exe)
    gamedir = os.path.abspath(args.gamedir)
    refdir = os.path.abspath(args.refdir)
    check_exe(exe)
    check_gamedir(gamedir)
    os.makedirs(refdir, exist_ok=True)
    seeds = parse_seeds(args.seeds) if args.seeds else [int(s, 16) for s in DEFAULT_SEEDS]
    ranges = read_mask(args.mask)
    manifest_path = os.path.join(refdir, 'manifest.json')
    manifest = json.load(open(manifest_path)) if os.path.exists(manifest_path) else {'seeds': {}}
    manifest.setdefault('seeds', {})
    snap = snapshot(gamedir)
    code = 0
    try:
        for seed in seeds:
            name = seed_name(seed)
            sdir = os.path.join(refdir, name)
            if os.path.isdir(sdir) and not args.force:
                refuse(f'{sdir} exists (use --force to overwrite)')
            dumps, err, workdir, fail = play(exe, gamedir, snap, seed, args, 'record')
            if fail is not None:
                code = max(code, run_failed(seed, fail, err))
                if not args.keep:
                    shutil.rmtree(workdir, ignore_errors=True)
                continue
            # write the new seed folder under a temporary name, then swap it in: a failure anywhere below
            # leaves the old reference and the manifest intact
            tmp_new = os.path.join(refdir, f'.{name}.tmp{os.getpid()}')
            shutil.rmtree(tmp_new, ignore_errors=True)
            os.makedirs(tmp_new)
            try:
                entry = {'rounds': len([k for k in dumps if k != 'final']),
                         'dumps': {}, 'exe': exe, 'exe_sha256': sha256_file(exe) if not exe.endswith('.py') else '',
                         'mask': os.path.basename(args.mask), 'date': datetime.date.today().isoformat()}
                for k in ordered_keys(dumps):
                    fn = f'r_{k}.bin' if k != 'final' else 'final.bin'
                    with open(os.path.join(tmp_new, fn), 'wb') as f:
                        f.write(dumps[k])
                    entry['dumps'][fn] = hashlib.sha256(masked(dumps[k], ranges)).hexdigest()
            except BaseException:
                shutil.rmtree(tmp_new, ignore_errors=True)
                raise
            old = None
            if os.path.isdir(sdir):
                old = os.path.join(refdir, f'.{name}.old{os.getpid()}')
                os.rename(sdir, old)
            os.rename(tmp_new, sdir)
            if old is not None:
                shutil.rmtree(old, ignore_errors=True)
            manifest['seeds'][name] = entry
            tmp_man = manifest_path + f'.tmp{os.getpid()}'
            with open(tmp_man, 'w') as f:
                json.dump(manifest, f, indent=1, sort_keys=True)
            os.replace(tmp_man, manifest_path)
            print(f'seed {name}: {entry["rounds"]} rounds and the end, recorded in {sdir}')
            if not args.keep:
                shutil.rmtree(workdir, ignore_errors=True)
    finally:
        restore(gamedir, snap)
    return code


def load_ref(refdir, seed, manifest, ranges):
    """The reference for one seed, validated against manifest.json: exactly the listed r_1..N.bin + final.bin,
    each 470,016 bytes and with the manifest's masked SHA-256; otherwise refuse (exit 4) naming the file."""
    name = seed_name(seed)
    sdir = os.path.join(refdir, name)
    entry = manifest['seeds'].get(name) if isinstance(manifest.get('seeds'), dict) else None
    if entry is None or not isinstance(entry.get('dumps'), dict):
        refuse(f'no reference for seed {name} in {refdir} (record it first, or pass --seeds)')
    want = entry['dumps']
    # validate with the mask the reference was recorded with (the manifest's mask file), not the current --mask
    ref_ranges = ranges
    mname = entry.get('mask')
    for cand in (mname, os.path.join(HERE, os.path.basename(mname or ''))):
        if cand and os.path.exists(cand):
            ref_ranges = read_mask(cand)
            break
    have = {}
    if os.path.isdir(sdir):
        for f in os.listdir(sdir):
            if f == 'final.bin' or re.fullmatch(r'r_\d+\.bin', f):
                have[f] = os.path.join(sdir, f)
    if set(have) != set(want):
        refuse(f'reference damaged: {sdir} holds {sorted(have)}, manifest.json lists {sorted(want)}')
    dumps = {}
    for fn in sorted(want, key=lambda f: (f == 'final.bin', int(f[2:-4]) if f != 'final.bin' else 0)):
        path = have[fn]
        data = open(path, 'rb').read()
        if len(data) != DS_SIZE:
            refuse(f'reference damaged: {path} is {len(data)} bytes, expected {DS_SIZE}')
        if hashlib.sha256(masked(data, ref_ranges)).hexdigest() != want[fn]:
            refuse(f'reference damaged: {path} masked SHA-256 does not match manifest.json')
        dumps['final' if fn == 'final.bin' else int(fn[2:-4])] = data
    return dumps


def compare_seed(seed, ref, new, ranges, what):
    """Print the per-seed lines; return 0 (equal) or 1 (difference). A different round count does not hide the
    first difference: the common rounds are compared first, then final.bin."""
    rrounds = len([k for k in ref if k != 'final'])
    nrounds = len([k for k in new if k != 'final'])
    common = min(rrounds, nrounds)
    first = None
    for k in range(1, common + 1):
        if k in ref and k in new:
            runs = diff_runs(ref[k], new[k], ranges)
            if runs:
                first = (k, runs)
                break
    if rrounds != nrounds:
        if first:
            k = first[0]
            print(f'seed {seed_name(seed)}: first difference in round {k}')
            report_runs(f'round {k}', ref[k], new[k], ranges)
        else:
            print(f'seed {seed_name(seed)}: the common rounds 1..{common} are all equal')
        print(f'seed {seed_name(seed)}: the game ended after {rrounds} rounds in the {what}, {nrounds} here')
        if 'final' in ref and 'final' in new:
            runs = diff_runs(ref['final'], new['final'], ranges)
            if runs:
                print(f'seed {seed_name(seed)}: first difference in final')
                report_runs('final', ref['final'], new['final'], ranges)
        return 1
    if first:
        k = first[0]
        print(f'seed {seed_name(seed)}: first difference in round {k}')
        report_runs(f'round {k}', ref[k], new[k], ranges)
        return 1
    if 'final' in ref and 'final' in new:
        runs = diff_runs(ref['final'], new['final'], ranges)
        if runs:
            print(f'seed {seed_name(seed)}: first difference in final')
            report_runs('final', ref['final'], new['final'], ranges)
            return 1
    print(f'seed {seed_name(seed)}: {rrounds} rounds and the end, all equal')
    return 0


def cmd_check(args):
    exe = os.path.abspath(args.exe)
    gamedir = os.path.abspath(args.gamedir)
    refdir = os.path.abspath(args.refdir)
    check_exe(exe)
    check_gamedir(gamedir)
    ranges = read_mask(args.mask)
    manifest = load_manifest(refdir)
    if args.seeds:
        seeds = parse_seeds(args.seeds)
    else:
        seeds = sorted((int(s, 16) for s in manifest['seeds']), key=lambda s: f'0x{s:08X}')
    snap = snapshot(gamedir)
    code = 0
    try:
        for seed in seeds:
            ref = load_ref(refdir, seed, manifest, ranges)
            dumps, err, workdir, fail = play(exe, gamedir, snap, seed, args, 'check')
            if fail is not None:
                code = max(code, run_failed(seed, fail, err))
            else:
                code = max(code, compare_seed(seed, ref, dumps, ranges, 'reference'))
            if not args.keep:
                shutil.rmtree(workdir, ignore_errors=True)
    finally:
        restore(gamedir, snap)
    return code


def cmd_xarch(args):
    exea = os.path.abspath(args.exe_a)
    exeb = os.path.abspath(args.exe_b)
    gamedir = os.path.abspath(args.gamedir)
    check_exe(exea)
    check_exe(exeb)
    check_gamedir(gamedir)
    seeds = parse_seeds(args.seeds) if args.seeds else [int(s, 16) for s in DEFAULT_SEEDS]
    ranges = read_mask(args.mask)
    snap = snapshot(gamedir)
    code = 0
    try:
        for seed in seeds:
            da, ea, wa, fa = play(exea, gamedir, snap, seed, args, 'xarch', subname='a')
            if fa is not None:
                code = max(code, run_failed(seed, f'{os.path.basename(exea)}: {fa}', ea))
                if not args.keep:
                    shutil.rmtree(wa, ignore_errors=True)
                continue
            db, eb, wb, fb = play(exeb, gamedir, snap, seed, args, 'xarch', subname='b')
            if fb is not None:
                code = max(code, run_failed(seed, f'{os.path.basename(exeb)}: {fb}', eb))
            else:
                code = max(code, compare_seed(seed, da, db, ranges, 'other build'))
            if not args.keep:
                shutil.rmtree(wa, ignore_errors=True)
                shutil.rmtree(wb, ignore_errors=True)
    finally:
        restore(gamedir, snap)
    return code


def main():
    ap = AP(description='regression and two-CPU check of whole demo games (record / check / xarch)')
    sub = ap.add_subparsers(dest='cmd')
    for name, fn, extra in (('record', cmd_record, [('exe',), ('gamedir',), ('refdir',)]),
                            ('check', cmd_check, [('exe',), ('gamedir',), ('refdir',)]),
                            ('xarch', cmd_xarch, [('exe_a',), ('exe_b',), ('gamedir',)])):
        sp = sub.add_parser(name)
        for (a,) in extra:
            sp.add_argument(a)
        common_args(sp)
        if name == 'record':
            sp.add_argument('--force', action='store_true', help='overwrite an existing seed folder')
        sp.set_defaults(func=fn)
    a = ap.parse_args()
    if not getattr(a, 'cmd', None):
        ap.error('need a command: record, check or xarch')
    try:
        return a.func(a)
    except SystemExit:
        raise
    except BaseException:
        import traceback
        traceback.print_exc()          # unexpected errors exit 2 with the full traceback (Erik wants them shown)
        return 2


if __name__ == '__main__':
    sys.exit(main())
