"""Tests for fixes.py on synthetic C snippets only (no port code copied). Run: python -I fixes_test.py

Covers, per the TASK 20 spec: each heading style (wrapped, "none" with bullets, mid-line "none"), each item
label style (1. / a. sub-item / A: / A (G) / - BUG(1995) bullets), a "fixed in both modes" list and a Text
paragraph, each marker style (label, colon-label, "bug N", two labels, trailing "(file comment, N)", bare "(N)",
label before the marker, references), BUG(1995) inside the first comment (not a marker), g_original in a comment
and a string (not counted), the definition and the option assignment (not counted), a port-guard line, an uncited
item (kind 1), a stray marker (kind 2), an unguarded g_original use (kind 3), an unplaceable unlabelled marker and
an unreadable label (kind 5), and check-doc coverage of a FIXES.md.
"""
import contextlib
import io
import os
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import fixes

fails = 0


def check(cond, msg):
    global fails
    if not cond:
        fails += 1
        print('FAIL', msg)


CLEAN = r'''/* Test file CLEAN.
   1995 bugs, fixed unless g_original (grades D code / G effect):
     1. 0x1234 does X (D code, G effect: rare). Continuation line.
        a. 0x2345 sub-item of 1.
     2. 0x3456 does Y (D / G).
   Kept as the original:
     - not a bug, just noted.
*/
#include <stdint.h>
int g_original;
int f(void) {
    int r = g_original ? 1 : 0;   /* BUG(1995) 1 */
    int s = g_original ? 2 : 0;   /* BUG(1995): 2 */
    int t = g_original ? 3 : 0;   /* BUG(1995) bug 1 and 2 */
    int u = g_original ? 4 : 0;   /* stale-word item 1a (BUG(1995)) */
    int v = g_original ? 5 : 0;   /* see BUG(1995) below, a reference */
    return r + s + t + u + v;
}
/* BUG(1995) 2 (file comment) */
int g(void) { return g_original ? 6 : 0; }   /* (BUG(1995) 1) bare before marker not a citation; see above */
int h(void) { return g_original ? 7 : 0; }   /* BUG(1995) 1 (file comment, 1) */
'''

NONEFILE = r'''/* Test NONEFILE.
   1995 bugs: none found (no g_original branches). Noted, not changed:
     - 0x9999 an oddity, kept.
*/
int f(void) { return 1; }
'''

BOTHMODES = r'''/* Test BOTHMODES.
   Stack words read before they are written (fixed in both modes, a port cannot reproduce them):
     1. 0x4444 the word is read before written. Fixed: starts at 0.
     2. 0x5555 the second stale word.
*/
int f(void) {
    return g_original ? 0 : 1;   /* BUG(1995) item 1: both modes */
}
'''

TEXTFILE = r'''/* Test TEXTFILE.
   1995 bug fixed (D):
     1. 0x6666 see below.
   Text slips fixed (TEXT_SWEEP.md): "TIGRUS" -> "TIGRIS" in one message.
*/
int f(void) {
    int a = g_original ? 0 : 1;   /* BUG(1995) 1 */
    int b = g_original ? 0 : 1;   /* BUG(1995) (text) */
    return a + b;
}
'''

NOHEAD = r'''/* Test NOHEAD: no 1995 heading in this first comment. */
int f(void) {
    return g_original ? 0 : 1;   /* BUG(1995): the counter reads stack garbage */
}
'''

FINDINGS = r'''/* Test FINDINGS. 1995 bugs, fixed unless g_original:
     1. 0x7777 cited below (D code, D effect).
     2. 0x8888 never cited by any marker (D code, G effect).
*/
int g_original;
int f(void) {
    int a = g_original ? 1 : 0;   /* BUG(1995) 1 */
    int b = g_original ? 2 : 0;   /* BUG(1995) 9: stray label */
    return a + b;
}
int g(void) {
    return g_original ? 4 : 0;   /* BUG(1995): unlabelled, no address, multi-item file */
}
/* BUG(1995): ??? */




















int far(void) {
    return g_original ? 3 : 0;   /* no marker in this function, none within 15 lines */
}
'''

MAIN = 'int g_original;\nint main(void) { g_original = 1; return 0; }\n'   # the option assignment: not counted

STATE = 'int g_original;\n'                                                # the definition: not counted

STRINGS = r'''int f(void) {
    const char *s = "g_original in a string";
    /* g_original in a comment */
    if (!g_original)   /* port guard: the original's path cannot be reproduced */
        return 1;
    return 0;
}
'''


def write(tmp, name, src):
    with open(os.path.join(tmp, name), 'w') as f:
        f.write(src)


def run_check(srcdir):
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        rc = fixes.run_check(srcdir)
    return rc, buf.getvalue()


def main():
    tmp = tempfile.mkdtemp(prefix='fixes_test_')
    write(tmp, 'clean.c', CLEAN)
    write(tmp, 'nonefile.c', NONEFILE)
    write(tmp, 'bothmodes.c', BOTHMODES)
    write(tmp, 'textfile.c', TEXTFILE)
    write(tmp, 'nohead.c', NOHEAD)
    write(tmp, 'findings.c', FINDINGS)
    write(tmp, 'main.c', MAIN)
    write(tmp, 'state.c', STATE)
    write(tmp, 'strings.c', STRINGS)

    rc, out = run_check(tmp)
    check(not any(l.startswith('clean.c:') for l in out.splitlines()), f'clean.c findings: {out}')
    check(not any(l.startswith('nonefile.c:') for l in out.splitlines()), f'nonefile findings: {out}')
    check(not any(l.startswith('nohead.c:') for l in out.splitlines()), f'nohead findings: {out}')
    check(not any(l.startswith('main.c:') or l.startswith('state.c:') or l.startswith('strings.c:') for l in out.splitlines()),
          f'main/state/strings findings: {out}')
    k1 = [l for l in out.splitlines() if 'kind 1' in l and not l.startswith('counts:')]
    check(sorted(l.split('kind 1: ', 1)[1] for l in k1) ==
          ['item both modes/2 not cited by any marker', 'item heading/2 not cited by any marker'], f'kind 1: {k1}')
    k2 = [l for l in out.splitlines() if 'kind 2' in l]
    check(any('findings.c' in l and 'label 9' in l for l in k2), f'kind 2: {k2}')
    k3 = [l for l in out.splitlines() if 'kind 3' in l]
    check(any('findings.c' in l and 'no marker nearby' in l for l in k3), f'kind 3: {k3}')
    k5 = [l for l in out.splitlines() if 'kind 5' in l]
    check(any('findings.c' in l and 'unlabelled marker' in l for l in k5), f'kind 5 marker: {k5}')
    check(any('findings.c' in l and 'unreadable label' in l for l in k5), f'kind 5 label: {k5}')
    check(rc == 1, f'check exit {rc}')

    # list: items and their markers
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        fixes.run_list(tmp, False)
    lst = buf.getvalue()
    rows = [l.split('\t') for l in lst.splitlines()]
    by = {(r[0], r[2]): r for r in rows}
    check(('clean.c', '1') in by and by[('clean.c', '1')][6].startswith('0x1234'), f'clean item 1: {by.get(("clean.c", "1"))}')
    check(len([m for m in by[('clean.c', '1')][7].split('=', 1)[1].split(',') if m]) >= 3,
          f'clean item 1 markers: {by[("clean.c", "1")][7]}')
    check(('clean.c', '1a') in by, 'clean sub-item 1a')
    check(('bothmodes.c', '1') in by and by[('bothmodes.c', '1')][3] == 'fixed in both modes', 'both modes item')
    check(any(r[0] == 'textfile.c' and r[1] == 'text' for r in rows), f'text item: {rows}')
    check(any(r[0] == 'nohead.c' and r[1] == 'code' and r[2] == '-' for r in rows), f'nohead in-place item: {rows}')
    check(by[('clean.c', '1')][4] == 'code=D' and by[('clean.c', '1')][5] == 'effect=G', f'grades: {by[("clean.c", "1")]}')

    # list --json
    import json
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        fixes.run_list(tmp, True)
    recs = json.loads(buf.getvalue())
    check(isinstance(recs, list) and any(r['file'] == 'clean.c' and r['label'] == '2' for r in recs), 'json list')

    # check-doc: a FIXES.md covering every item passes; a missing row fails
    doc = os.path.join(tmp, 'FIXES.md')
    lines = ['# FIXES', '']
    for r in recs:
        lab = r['label'] if r['label'] is not None else '@' + (r['addrs'][0][2:] if r['addrs'] else '0')
        addr = r['addrs'][0] if r['addrs'] else '0x000000'
        lines.append(f'| row | {r["file"]} {lab} {addr} |')
    open(doc, 'w').write('\n'.join(lines) + '\n')
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        rc = fixes.run_check_doc(tmp, doc)
    check(rc == 0, f'check-doc full: {rc} {buf.getvalue()}')
    open(doc, 'w').write('\n'.join(lines[:-2]) + '\n')   # drop the last row
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        rc = fixes.run_check_doc(tmp, doc)
    check(rc == 1 and 'not in FIXES.md' in buf.getvalue(), f'check-doc missing: {rc} {buf.getvalue()}')

    if fails == 0:
        print('fixes_test: all checks pass')
    else:
        print(f'fixes_test: {fails} FAILURES')
    return 1 if fails else 0


if __name__ == '__main__':
    sys.exit(main())
