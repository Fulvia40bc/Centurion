#!/usr/bin/env python3
"""Recursive disassembly of AC_MAIN.EXE's code object (output of le_unbind.py).

No disassembler library is assumed: llvm-objdump does the decoding. It decodes linearly, but it restarts at every
symbol, so this tool writes an ELF of the code object with a symbol at every address proven to be code, runs
llvm-objdump, follows the control flow from the proven starts, adds the targets it finds, and repeats until nothing
new turns up.

Proven code: the entry point; targets of direct call/jmp/jcc/loop from reachable code; code addresses taken as
immediates in reachable code (function pointers); code addresses stored in the data object (data->code fixups);
entries of jump tables (code->code fixups lying outside every reachable instruction, i.e. in data inside the code
object). Calls are assumed to return.

Usage: rdisasm.py LE_DIR OUT_DIR [TOOLCHAIN_BIN]
  LE_DIR   holds obj1.bin, obj2.bin, fixups.txt (from le_unbind.py)
  OUT_DIR  receives AC_MAIN.asm (labelled listing, reachable code only, data shown as bytes),
           functions.txt (one line per function) and coverage.txt (what was decoded, what was not)
"""
import os, re, struct, subprocess, sys
from collections import defaultdict

CODE_BASE, DATA_BASE = 0x10000, 0xB0000
ENTRY = 0x8BF54
TERMINATORS = {'jmp', 'ret', 'retf', 'iret', 'iretd', 'hlt', 'ud2', 'ljmp'}
# instructions Watcom never emits and hand-written library code here has no use for: when an address-taken target
# decodes into one of these before its first terminator, the target is data (text, a table) and not a function
JUNK = {'<unknown>', '(bad)', 'arpl', 'bound', 'into', 'das', 'daa', 'aaa', 'aas', 'aam', 'aad', 'salc',
        'wbinvd', 'sldt', 'str', 'insb', 'insw', 'insd', 'outsb', 'outsw', 'outsd', 'hlt',
        'int1', 'icebp', 'lock', 'addr16', 'data16'}
BRANCH = re.compile(r'^(j[a-z]+|loop[a-z]*|call)$')
INSN = re.compile(r'^\s*([0-9a-f]+):\s+(\S+)(?:\s+(.*))?$')
HEX = re.compile(r'0x[0-9a-f]+')


def write_elf(path, code, syms):
    """ELF32 i386 relocatable with one .text section at CODE_BASE and a FUNC symbol per address."""
    strtab = bytearray(b'\0')
    symtab = bytearray(16)
    for a in sorted(syms):
        name_off = len(strtab)
        strtab += b'L_%08x\0' % a
        symtab += struct.pack('<IIIBBH', name_off, a - CODE_BASE, 0, 0x12, 0, 1)   # ET_REL: section offset
    shstr = b'\0.text\0.symtab\0.strtab\0.shstrtab\0'
    off_text = 0x40
    off_sym = (off_text + len(code) + 3) & ~3
    off_str = off_sym + len(symtab)
    off_shs = off_str + len(strtab)
    off_sh = (off_shs + len(shstr) + 3) & ~3
    sh = [bytes(40),
          struct.pack('<10I', 1, 1, 6, CODE_BASE, off_text, len(code), 0, 0, 16, 0),
          struct.pack('<10I', 7, 2, 0, 0, off_sym, len(symtab), 3, 1, 4, 16),
          struct.pack('<10I', 15, 3, 0, 0, off_str, len(strtab), 0, 0, 1, 0),
          struct.pack('<10I', 23, 3, 0, 0, off_shs, len(shstr), 0, 0, 1, 0)]
    hdr = b'\x7fELF\x01\x01\x01' + bytes(9) + struct.pack('<HHIIIIIHHHHHH', 1, 3, 1, 0, 0, off_sh, 0, 52, 0, 0, 40,
                                                          len(sh), 4)
    out = bytearray(off_sh + 40 * len(sh))
    out[0:len(hdr)] = hdr
    out[off_text:off_text + len(code)] = code
    out[off_sym:off_sym + len(symtab)] = symtab
    out[off_str:off_str + len(strtab)] = strtab
    out[off_shs:off_shs + len(shstr)] = shstr
    for i, s in enumerate(sh):
        out[off_sh + 40 * i:off_sh + 40 * (i + 1)] = s
    open(path, 'wb').write(out)


def objdump(tool, elf):
    """-> {addr: (mnemonic, operands, next_addr)} for every decoded instruction."""
    # -z: show zero bytes as instructions instead of "...", so every instruction's length is next_addr - addr
    r = subprocess.run([tool, '-d', '-z', '--no-show-raw-insn', '-M', 'intel', elf], capture_output=True, text=True,
                       check=True)
    insns = {}
    prev = None
    for line in r.stdout.splitlines():
        if line.endswith('>:'):            # symbol header: a new linear run starts here
            if prev is not None:
                insns[prev][2] = int(line.split()[0], 16)
            prev = None
            continue
        m = INSN.match(line)
        if not m:
            continue
        a = int(m.group(1), 16)
        ops = (m.group(3) or '').split(' <')[0].strip()
        mn = m.group(2)
        # objdump prints prefixes as separate mnemonics ("rep", "lock", "cs"...) on their own line only for orphans
        insns[a] = [mn, ops, None]
        if prev is not None:
            insns[prev][2] = a
        prev = a
    return insns


def main():
    le_dir, out_dir = sys.argv[1], sys.argv[2]
    tool = os.path.join(sys.argv[3] if len(sys.argv) > 3 else
                        r'C:\work\toolchain\llvm-mingw-20261104-ucrt-aarch64\bin', 'llvm-objdump.exe')
    code = open(os.path.join(le_dir, 'obj1.bin'), 'rb').read()
    code_end = CODE_BASE + len(code)
    fix = {}
    for line in open(os.path.join(le_dir, 'fixups.txt')):
        f = line.split()
        if len(f) >= 3 and f[1] == 'off32':
            fix[int(f[0], 16)] = int(f[2], 16)
    is_code = lambda t: CODE_BASE <= t < code_end
    code_fix = sorted(s for s, t in fix.items() if is_code(s) and is_code(t))   # code->code fixups
    strong = {ENTRY}                                            # entry + direct call targets: always code
    cands = {t for s, t in fix.items() if not is_code(s) and is_code(t)}   # address taken: code unless junk
    ucands = set()                                              # unreached starts found by their prologue
    label_seeds = set()
    elf = os.path.join(out_dir, 'rdisasm.elf')
    os.makedirs(out_dir, exist_ok=True)

    def junk(c, insns):
        """True if an address-taken target is data: it holds a relocated code pointer itself (a jump table), or a
        linear decode from it hits instructions compilers never emit (text, zero runs, tables)."""
        if is_code(fix.get(c, 0)):
            return True
        a, zeros = c, 0
        for _ in range(400):
            if a not in insns:
                return True
            mn, ops, nxt = insns[a]
            if mn in JUNK or nxt is None:
                return True
            zeros = zeros + 1 if (mn, ops) == ('add', 'byte ptr [eax], al') else 0
            if zeros >= 2:
                return True
            if mn in TERMINATORS:
                return False
            a = nxt
        return False

    phase = 1
    tlabels = set()
    for rnd in range(1, 80):
        syms = strong | cands | label_seeds
        write_elf(elf, code, syms)
        insns = objdump(tool, elf)
        rejected = {c for c in cands - strong if junk(c, insns)}
        # case labels from the last round's jump tables; re-derived every round, since a fixup that looked like a
        # table entry may turn out to sit inside an instruction reached later
        troots = {t for t in tlabels if not junk(t, insns)}
        reach = {}                                              # addr -> length
        calls, jumps, imm_code, misaligned = set(), set(), set(), set()
        work = sorted(strong | (cands - rejected) | troots)
        seen = set()
        while work:
            a = work.pop()
            while a not in seen:
                if a not in insns:
                    misaligned.add(a)
                    break
                seen.add(a)
                mn, ops, nxt = insns[a]
                if nxt is None or mn == '<unknown>' or mn == '(bad)':
                    break
                reach[a] = nxt - a
                # fixups inside this instruction: immediate code addresses are function pointers
                for s in range(a, nxt):
                    t = fix.get(s)
                    if t is not None and is_code(t) and 'ptr [' not in ops:
                        imm_code.add(t)
                if BRANCH.match(mn) and HEX.match(ops):
                    t = int(HEX.match(ops).group(0), 16)
                    if is_code(t):
                        (calls if mn == 'call' else jumps).add(t)
                        if mn != 'call':
                            work.append(t)
                if mn in TERMINATORS:
                    break
                a = nxt
        # jump-table entries: code->code fixups whose 4 bytes lie outside every reachable instruction
        covered = bytearray(len(code))
        for a, n in reach.items():
            covered[a - CODE_BASE:a - CODE_BASE + n] = b'\1' * n
        tables = {s for s in code_fix if not any(covered[s - CODE_BASE:s - CODE_BASE + 4])}
        table_targets = {fix[s] for s in tables if is_code(fix[s])}
        new_strong = calls - strong
        new_cands = imm_code - cands - strong - calls
        new_labels = (jumps | table_targets) - strong - cands - label_seeds - new_strong - new_cands
        # a jump target already decoded inside a run needs no symbol; one objdump did not land on does
        new_labels = {t for t in new_labels if t not in insns}
        tchanged = table_targets != tlabels
        tlabels = table_targets
        print(f'round {rnd}: {len(reach)} insns reachable, {len(strong)} called, {len(cands - rejected)} address-'
              f'taken ({len(rejected)} rejected as data), {len(troots)} case labels, +{len(new_strong)} called, '
              f'+{len(new_cands)} taken, +{len(new_labels)} labels, {len(misaligned)} misaligned')
        if not new_strong and not new_cands and not new_labels and not tchanged:
            if phase == 2:
                break
            # phase 2: functions nothing reaches (unused library API, dead code), found by a prologue at a gap
            phase = 2
            ucands = gap_starts(code, reach, tables) - strong - cands
            print(f'phase 2: {len(ucands)} unreached starts with a prologue')
            cands |= ucands
            continue
        strong |= new_strong
        cands |= new_cands
        label_seeds |= new_labels

    # ---- results ----------------------------------------------------------------------------------------------
    ucands -= rejected
    funcs = sorted(strong | calls | (cands - rejected))
    callers = defaultdict(set)
    insn_func = {}
    fi = 0
    for a in sorted(reach):
        while fi + 1 < len(funcs) and funcs[fi + 1] <= a:
            fi += 1
        insn_func[a] = funcs[fi] if funcs and funcs[fi] <= a else None
    for a, n in reach.items():
        mn, ops, _ = insns[a]
        if mn == 'call' and HEX.match(ops):
            t = int(HEX.match(ops).group(0), 16)
            callers[t].add(insn_func[a])
    data_refs = defaultdict(list)                               # function -> data addresses it references
    for a, n in reach.items():
        for s in range(a, a + n):
            t = fix.get(s)
            if t is not None and not is_code(t):
                data_refs[insn_func[a]].append(t)

    strings = load_strings(os.path.join(le_dir, 'obj2.bin'))
    names = auto_names(funcs, reach, insns, fix, strings, insn_func, is_code)
    names.update(load_names(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'disasm', 'names.txt')))
    with open(os.path.join(out_dir, 'functions.txt'), 'w', newline='\n') as f:
        f.write('# Functions of AC_MAIN.EXE found by tools/rdisasm.py (recursive descent from the entry point).\n'
                '# addr     bytes  insns calls-in  how    name (grade)                    first strings referenced\n'
                '# how: e = entry, c = called directly, p = address taken (function pointer),\n'
                '#      u = reached by nothing, found by its prologue in a gap (unused library API or dead code)\n'
                '# grade: D = the binary says so (own trace string, known library code), G = guess (it passes this\n'
                '#        identifier string in eax, Watcom\'s first argument), from disasm/names.txt or automatic\n')
        bounds = funcs + [code_end]
        for i, fa in enumerate(funcs):
            fb = bounds[i + 1]
            n_ins = sum(1 for a in reach if fa <= a < fb)
            how = ('e' if fa == ENTRY else '') + ('c' if fa in calls else '') + \
                  ('u' if fa in ucands else 'p' if fa in cands else '')
            strs = []
            for t in data_refs.get(fa, []):
                s = strings.get(t)
                if s and s not in strs:
                    strs.append(s)
            hint = ' | '.join(x[:40] for x in strs[:3])
            nm = f'{names[fa][0]} ({names[fa][1]})' if fa in names else '-'
            f.write(f'{fa:08X} {fb - fa:6d} {n_ins:6d} {len(callers.get(fa, ())):6d}  {how:<6} {nm:<31} {hint}\n')

    # coverage: decoded bytes, jump tables, and the gaps (data or unreached code)
    tbl_bytes = len(tables) * 4
    dec = sum(covered)
    gaps = []
    a = 0
    cov = bytearray(covered)
    for s in tables:
        cov[s - CODE_BASE:s - CODE_BASE + 4] = b'\2' * 4
    while a < len(code):
        if cov[a]:
            a += 1
            continue
        b = a
        while b < len(code) and not cov[b]:
            b += 1
        gaps.append((a + CODE_BASE, b - a))
        a = b
    with open(os.path.join(out_dir, 'coverage.txt'), 'w', newline='\n') as f:
        f.write(f'code object {len(code)} bytes: {dec} decoded as reachable instructions ({100 * dec / len(code):.1f}%), '
                f'{tbl_bytes} in {len(tables)} jump-table entries, {len(code) - dec - tbl_bytes} in {len(gaps)} gaps\n')
        ra = sorted(reach)
        overlaps = sum(1 for x, y in zip(ra, ra[1:]) if x + reach[x] > y)
        unseen_calls = len(calls - set(reach))
        f.write(f'{len(funcs)} functions, {len(reach)} instructions; {overlaps} overlapping instructions; '
                f'{unseen_calls} call targets not decoded\n')
        f.write('# address-taken code addresses rejected as data (tables, strings in the code object):\n')
        for r in sorted(rejected):
            o = r - CODE_BASE
            f.write(f'{r:08X}  {code[o:o + 16].hex(" ")}  {printable(code[o:o + 24])}\n')
        f.write('# gaps of 16+ bytes (address, length, first bytes):\n')
        for g, n in gaps:
            if n >= 16:
                o = g - CODE_BASE
                f.write(f'{g:08X} {n:7d}  {code[o:o + 16].hex(" ")}  {printable(code[o:o + 40])}\n')

    # labelled listing
    labels = set(label_seeds) | jumps | table_targets
    with open(os.path.join(out_dir, 'AC_MAIN.asm'), 'w', newline='\n') as f:
        f.write('; AC_MAIN.EXE code object, recursive disassembly (tools/rdisasm.py). Addresses are linear.\n'
                '; Data inside the code object (banners, jump tables, unreached bytes) is shown as db/dd.\n')
        fset = set(funcs)
        a = CODE_BASE
        while a < code_end:
            if a in fset:
                f.write(f'\nfn_{a:08X}:' + (f'  ; {names[a][0]} ({names[a][1]})' if a in names else '') + '\n')
            elif a in labels:
                f.write(f'L_{a:08X}:\n')
            if a in reach:
                mn, ops, nxt = insns[a]
                note = ''
                for s in range(a, nxt):
                    t = fix.get(s)
                    if t is not None and not is_code(t) and t in strings:
                        note = f'  ; "{strings[t][:60]}"'
                        break
                if mn == 'call' and HEX.match(ops) and int(HEX.match(ops).group(0), 16) in names:
                    note = f'  ; {names[int(HEX.match(ops).group(0), 16)][0]}' + note
                f.write(f'  {a:08X}  {mn:<7} {ops}{note}\n')
                a = nxt
            elif a in tables:
                f.write(f'  {a:08X}  dd      0x{fix[a]:08X}\n')
                a += 4
            else:
                b = a + 1
                while b < code_end and b not in reach and b not in tables and b not in fset and b not in labels \
                        and b - a < 16:
                    b += 1
                o = a - CODE_BASE
                f.write(f'  {a:08X}  db      {code[o:b - CODE_BASE].hex(" ")}  ; {printable(code[o:b - CODE_BASE])}\n')
                a = b
    os.remove(elf)
    print('misaligned (first 15):', ' '.join(f'{x:08X}' for x in sorted(misaligned)[:15]))
    print(f'{len(funcs)} functions, {dec} of {len(code)} bytes decoded; see {out_dir}')


AIL_TRACE = re.compile(r'^(AIL_\w+)\(')
IDENT = re.compile(r'^[A-Z][a-z]+(?:[A-Z][a-z0-9]*)+$')


def auto_names(funcs, reach, insns, fix, strings, insn_func, is_code):
    """addr -> (name, grade, evidence). AIL API functions print their own name when AIL_DEBUG is set (D); game
    functions pass their own name as an identifier string in eax, Watcom's first argument (G)."""
    ail, ident = defaultdict(set), defaultdict(set)
    for a, n in reach.items():
        mn, ops, _ = insns[a]
        for s in range(a, a + n):
            t = fix.get(s)
            if t is None or is_code(t) or t not in strings:
                continue
            m = AIL_TRACE.match(strings[t])
            if m:
                ail[insn_func[a]].add(m.group(1))
            elif IDENT.match(strings[t]) and mn == 'mov' and ops.startswith('eax,'):
                ident[insn_func[a]].add(strings[t])
    out = {}
    for fa, s in ident.items():
        if fa is not None and len(s) == 1:
            out[fa] = (s.pop(), 'G', 'passes its name in eax')
    for fa, s in ail.items():
        if fa is not None and len(s) == 1:
            out[fa] = (s.pop(), 'D', 'own AIL trace string')
    return out


def load_names(path):
    """disasm/names.txt: 'ADDR NAME GRADE  # evidence' per line; overrides the automatic names."""
    out = {}
    if os.path.exists(path):
        for line in open(path, encoding='utf-8'):
            f = line.split('#')[0].split()
            if len(f) >= 3:
                out[int(f[0], 16)] = (f[1], f[2], line.split('#', 1)[1].strip() if '#' in line else '')
    return out


PADDING = (b'\x8d\x80\x00\x00\x00\x00', b'\x8d\x76\x00', b'\x8d\x40\x00', b'\x8b\xc0', b'\x89\xc0', b'\x90', b'\x00')


def gap_starts(code, reach, tables):
    """Start of each gap in the decoded code, after Watcom's alignment padding, if it opens like a function:
    push reg (50-57), push imm32 (68: the __CHK stack-size prologue), or sub esp."""
    cov = bytearray(len(code))
    for a, n in reach.items():
        cov[a - CODE_BASE:a - CODE_BASE + n] = b'\1' * n
    for s in tables:
        cov[s - CODE_BASE:s - CODE_BASE + 4] = b'\1' * 4
    out = set()
    o = 0
    while o < len(code):
        if cov[o]:
            o += 1
            continue
        e = o
        while e < len(code) and not cov[e]:
            e += 1
        p = o
        while p < e:
            for pad in PADDING:
                if code[p:p + len(pad)] == pad:
                    p += len(pad)
                    break
            else:
                break
        if e - p >= 3 and (0x50 <= code[p] <= 0x57 or code[p] == 0x68 or code[p:p + 2] == b'\x83\xec'):
            out.add(p + CODE_BASE)
        o = e
    return out


def printable(b):
    return ''.join(chr(c) if 32 <= c < 127 else '.' for c in b)


def load_strings(path):
    """data address -> text for every NUL-terminated printable run of 3+ chars in the data object."""
    d = open(path, 'rb').read()
    out = {}
    i = 0
    while i < len(d):
        j = i
        while j < len(d) and (32 <= d[j] < 127 or d[j] in (9, 10, 13)):
            j += 1
        if j - i >= 3 and j < len(d) and d[j] == 0:
            out[DATA_BASE + i] = d[i:j].decode('latin-1').replace('\n', '\\n').replace('\r', '\\r')
        i = j + 1
    return out


if __name__ == '__main__':
    main()
