#!/usr/bin/env python3
"""Recursive-descent 16-bit disassembler for DOS real-mode binaries.

Usage: rdisasm.py <binary> <out.asm>
Seeds: offset 0 + every direct call target found in a linear capstone pass.
Follows call/jmp/conditional-jmp targets recursively. Emits one annotated
listing with function boundaries marked.
"""
import sys
from capstone import Cs, CS_ARCH_X86, CS_MODE_16
from capstone.x86 import X86_OP_IMM, X86_OP_MEM

md = Cs(CS_ARCH_X86, CS_MODE_16)
md.detail = True

def collect_call_targets(code):
    targets = set()
    for ins in md.disasm(code, 0):
        if ins.mnemonic in ('call', 'jmp') and ins.op_str.startswith('0x'):
            try:
                targets.add(int(ins.op_str, 16))
            except ValueError:
                pass
        elif ins.mnemonic == 'call' and '0x' in ins.op_str:
            pass
    # also near call rel16 via operands
    for ins in md.disasm(code, 0):
        if ins.mnemonic in ('call', 'jmp', 'je', 'jne', 'ja', 'jb', 'jae', 'jbe',
                           'jg', 'jl', 'jge', 'jle', 'js', 'jns', 'jo', 'jno',
                           'jp', 'jnp', 'jc', 'jnc', 'loop', 'loope', 'loopne'):
            for op in ins.operands:
                if op.type == X86_OP_IMM:
                    targets.add(op.imm & 0xFFFF)
    return targets

def disasm_function(code, start, max_len=0x8000):
    """Disassemble from start until ret/iret/uncond jmp out; return (end, lines, targets)."""
    lines = []
    targets = []
    addr = start
    seen = set()
    while addr < len(code) and addr - start < max_len:
        if addr in seen:
            break
        seen.add(addr)
        ins = next(md.disasm(code[addr:addr+16], addr, count=1), None)
        if ins is None:
            lines.append((addr, code[addr], 'db 0x%02x' % code[addr]))
            addr += 1
            continue
        lines.append((addr, None, f'{ins.mnemonic} {ins.op_str}'))
        for op in ins.operands:
            if op.type == X86_OP_IMM and ins.mnemonic != 'ret':
                t = op.imm & 0xFFFF
                if ins.mnemonic in ('call', 'jmp') or ins.mnemonic.startswith('j') or ins.mnemonic.startswith('loop'):
                    targets.append(t)
        if ins.mnemonic in ('ret', 'retf', 'iret', 'hlt'):
            addr += ins.size
            break
        if ins.mnemonic == 'jmp':
            addr += ins.size
            break
        addr += ins.size
    return addr, lines, targets

def main():
    code = open(sys.argv[1], 'rb').read()
    seeds = {0} | collect_call_targets(code)
    seeds = {s for s in seeds if 0 <= s < len(code)}
    print(f'{len(seeds)} seed addresses', file=sys.stderr)
    functions = {}
    work = sorted(seeds)
    while work:
        start = work.pop()
        if start in functions:
            continue
        end, lines, targets = disasm_function(code, start)
        functions[start] = (end, lines)
        for t in targets:
            if 0 <= t < len(code) and t not in functions:
                work.append(t)
    with open(sys.argv[2], 'w') as f:
        for start in sorted(functions):
            end, lines = functions[start]
            f.write(f'\nfunc_0x{start:04x}: ; ends ~0x{end:04x}, {len(lines)} insns\n')
            for addr, raw, text in lines:
                f.write(f'  0x{addr:04x}: {text}\n')
    print(f'{len(functions)} functions -> {sys.argv[2]}', file=sys.stderr)

if __name__ == '__main__':
    main()
