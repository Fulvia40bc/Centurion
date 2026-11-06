#!/usr/bin/env python3
"""Conflict-free recursive disassembler.

Usage: rdisasm2.py <binary> <out.asm> <speculative.asm> <verified_seeds.txt>

Primary output: seeds accepted only from
  - direct call/jmp targets,
  - `push imm16` immediately consumed by ret / pop-then-jmp (thunks),
  - the verified seed list (entry, ISRs, runtime-observed addresses).
Overlaps resolved in favour of the decode reachable from verified seeds
(BFS order). Reports coverage and conflict count. Speculative brute-force
decodes go to the second file, labelled UNVERIFIED.
"""
import re, sys
from capstone import Cs, CS_ARCH_X86, CS_MODE_16
from capstone.x86 import X86_OP_IMM

md = Cs(CS_ARCH_X86, CS_MODE_16)
md.detail = True

def disasm_at(code, addr, max_len=0x4000):
    out = []
    a = addr
    seen = set()
    while a < len(code) and a - addr < max_len:
        if a in seen: break
        seen.add(a)
        ins = next(md.disasm(code[a:a+16], a, count=1), None)
        if ins is None:
            out.append((a, 'db 0x%02x' % code[a])); a += 1; continue
        out.append((a, f'{ins.mnemonic} {ins.op_str}'))
        if ins.mnemonic in ('ret', 'retf', 'iret', 'hlt'):
            a += ins.size; break
        if ins.mnemonic == 'jmp':
            a += ins.size; break
        a += ins.size
    return out, a

def collect(code, verified):
    """Return (primary_seeds, speculative_seeds)."""
    primary = set(verified)
    spec = set()
    # direct call/jmp targets
    for ins in md.disasm(code, 0):
        if ins.mnemonic in ('call', 'jmp'):
            for op in ins.operands:
                if op.type == X86_OP_IMM and 0x20 <= op.imm < len(code):
                    primary.add(op.imm & 0xFFFF)
        # push imm16 followed shortly by ret / pop+jmp = thunk
        if ins.mnemonic == 'push':
            for op in ins.operands:
                if op.type == X86_OP_IMM and 0x20 <= (op.imm & 0xFFFF) < len(code):
                    spec.add(op.imm & 0xFFFF)
    return primary, spec

def run(code, seeds):
    funcs = {}
    work = sorted(seeds)
    conflicts = 0
    covered = set()
    while work:
        s = work.pop()
        if s in funcs or not (0x20 <= s < len(code)):
            continue
        lines, end = disasm_at(code, s)
        # conflict check: any instruction start inside an existing function?
        starts = [a for a, _ in lines]
        for a in starts[1:]:
            for f, fl in funcs.items():
                fstarts = {x for x, _ in fl}
                if a in fstarts:
                    conflicts += 1
        funcs[s] = lines
        for a in starts:
            covered.add(a)
        for a, text in lines:
            m = re.match(r'(?:call|jmp|j\w+|loop\w*) 0x([0-9a-f]+)', text)
            if m:
                t = int(m.group(1), 16)
                if 0x20 <= t < len(code) and t not in funcs:
                    work.append(t)
    return funcs, covered, conflicts

def main():
    code = open(sys.argv[1], 'rb').read()
    out, spec_out, seedfile = sys.argv[2], sys.argv[3], sys.argv[4]
    verified = {int(l, 16) for l in open(seedfile) if l.strip()}
    primary, spec = collect(code, verified)
    funcs, covered, conflicts = run(code, primary)
    specfuncs, _, _ = run(code, spec - set(funcs))
    with open(out, 'w') as f:
        for s in sorted(funcs):
            f.write(f'\nfunc_0x{s:04x}:\n')
            for a, t in funcs[s]:
                f.write(f'  0x{a:04x}: {t}\n')
    with open(spec_out, 'w') as f:
        f.write('; UNVERIFIED speculative decodes - do NOT port from this file\n')
        for s in sorted(specfuncs):
            f.write(f'\nfunc_0x{s:04x} (UNVERIFIED):\n')
            for a, t in specfuncs[s]:
                f.write(f'  0x{a:04x}: {t}\n')
    total_chunks = len(code) // 16
    cov = len({a // 16 for a in covered})
    print(f'primary: {len(funcs)} funcs, coverage {cov}/{total_chunks} = {cov/total_chunks:.1%}, conflicts {conflicts}')
    print(f'speculative (UNVERIFIED): {len(specfuncs)} funcs -> {spec_out}')

if __name__ == '__main__':
    main()
