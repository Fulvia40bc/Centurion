#!/usr/bin/env python3
"""Naive pseudocode annotator for the EXE_RUNTIME disassembly.

Usage: pseudo.py <func_hex> [more funcs...]
Reads disasm/EXE_RUNTIME.asm, prints per-function: args (from trailing pushes
before calls are hard, so we just list calls/globals/loops), calls, globals.
"""
import re, sys

def load(path):
    funcs = {}
    cur = None
    for ln in open(path):
        m = re.match(r'func_0x([0-9a-f]+):', ln)
        if m:
            cur = int(m.group(1), 16)
            funcs[cur] = []
        elif cur is not None and ln.startswith('  0x'):
            funcs[cur].append(ln.strip())
    return funcs

def summarize(addr, body):
    calls = sorted(set(re.findall(r'call 0x([0-9a-f]+)', '\n'.join(body))))
    globals_ = sorted(set(re.findall(r'\[0x([0-9a-f]+)\]', '\n'.join(body))))
    loops = [l for l in body if re.search(r'j\w+ 0x[0-9a-f]+', l)]
    back = [l for l in loops if re.match(r'0x([0-9a-f]+): j\w+ 0x([0-9a-f]+)', l)
            and int(re.match(r'0x([0-9a-f]+): j\w+ 0x([0-9a-f]+)', l).group(2), 16)
            < int(re.match(r'0x([0-9a-f]+): j\w+ 0x([0-9a-f]+)', l).group(1), 16)]
    print(f'func_0x{addr:04x}: {len(body)} insns, {len(calls)} calls, {len(back)} back-edges')
    print(f'  calls: {" ".join(calls)}')
    print(f'  globals: {" ".join("0x"+g for g in globals_)}')
    if back:
        print(f'  loops at: {"; ".join(back[:4])}')

if __name__ == '__main__':
    funcs = load('disasm/EXE_RUNTIME.asm')
    for a in sys.argv[1:]:
        addr = int(a, 16)
        if addr in funcs:
            summarize(addr, funcs[addr])
        else:
            print(f'0x{addr:x}: not a function start')
