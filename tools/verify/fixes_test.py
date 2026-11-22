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

# ---- TASK 20b: one synthetic case per fix, each fails without the fix ----

# fix 1: a numbered item whose line ends ':' and only introduces lettered sub-items is a parent, not an item
PARENT = r'''/* Test PARENT.
   1995 bugs, fixed unless g_original:
     1. 0x1111 cited below (D code, G effect).
     3. The runs have no bound (D / G):
        a. 0x2222 sub a.
        b. 0x3333 sub b.
*/
int g_original;
int f(void) { return g_original ? 1 : 0; }   /* BUG(1995) 3a */
int g(void) { return g_original ? 2 : 0; }   /* BUG(1995) 3b */
int h(void) { return g_original ? 3 : 0; }   /* BUG(1995) 1 */
'''

# fix 2: keys are (file, list, label); a label in several lists is kind 5 unless the marker names the list
KEYS = r'''/* Test KEYS.
   1995 bugs, fixed unless g_original:
     1. 0x4444 heading one (D code, G effect).
     2. 0x4555 heading two (D code, G effect).
   Stack words read before they are written, fixed in both modes:
     1. 0x5555 stale one, fixed in both modes.
*/
int g_original;
int f(void) { return g_original ? 1 : 0; }   /* BUG(1995) 1: ambiguous, names no list */
int g(void) { return g_original ? 2 : 0; }   /* BUG(1995) both modes 1 */
int h(void) { return g_original ? 3 : 0; }   /* BUG(1995) 2 */
'''

# fix 3: a Text paragraph ends at a new same-indent paragraph starting with a section word
SECT = r'''/* Test SECT.
   1995 bugs, fixed unless g_original:
     1. 0x1212 one (D code, G effect).
   Text slips fixed (TEXT_SWEEP.md): "X" -> "Y" (0xB123).
   Oddities kept (D):
     - 0x9999 an oddity, not a fix.
*/
int g_original;
int f(void) { return g_original ? 1 : 0; }   /* BUG(1995) 1 */
int g(void) { return g_original ? 2 : 0; }   /* BUG(1995) (text) */
'''

# fix 4: a note at the item indent after a both-modes item's continuation is not part of the item
CONT = r'''/* Test CONT.
   1995 bugs, fixed unless g_original:
     5. 0x1313 five (D code, G effect).
   Stack words read before they are written, fixed in both modes:
     1. 0x1414 stale one, fixed in both modes; its word is read
        before it is written (0x1515).
     f6B0EC's note at the item indent (0x1616) is not part of item 1.
*/
int g_original;
int f(void) { return g_original ? 1 : 0; }   /* BUG(1995) 5 */
int g(void) { return g_original ? 2 : 0; }   /* BUG(1995) both modes 1 */
'''

# fix 5: citations are read anywhere in the marker's comment, across lines
CITE = r'''/* Test CITE.
   1995 bugs, fixed unless g_original:
     1. 0x1717 one (D code, G effect).
     2. 0x1818 two (D code, G effect).
*/
int g_original;
int f(void) {
    return g_original ? 1 : 0;   /* BUG(1995): the first one
                                    (the file
                                    comment, 1) and more text */
}
int g(void) {
    return g_original ? 2 : 0;   /* BUG(1995): the second (file comment, 2); trailing words */
}
'''

# fix 6: every "- " bullet of a heading list is an item (keyed @<first address> without a label)
DASH = r'''/* Test DASH.
   1995 bugs, fixed unless g_original:
     - 0x1919 dash one (D code, G effect).
     - 0x1A1A dash two (D code, G effect).
   Kept as the original:
     - 0x1B1B kept, not an item.
*/
int g_original;
int f(void) { return g_original ? 1 : 0; }   /* BUG(1995): dash one (0x1919) */
int g(void) { return g_original ? 2 : 0; }   /* BUG(1995): dash two (0x1A1A) */
'''

# fix 6, negative: deeper dash bullets under a non-item line are not items (aiplan.c's shape)
DASH2 = r'''/* Test DASH2. 1995 bugs (graded code / effect). Prose:
     1./2. a combined label line, not an item:
        - 0x1C1C deep dash one.
        - 0x1D1D deep dash two.
   Kept as the original:
     - 0x1E1E kept.
*/
int g_original;
int f(void) { return g_original ? 1 : 0; }   /* BUG(1995): nothing, no items in this file */
'''

# fix 7: "BUG(1995) (text):" links to the file's one text item, whichever list holds it
TXTM = r'''/* Test TXTM.
   1995 bugs, fixed unless g_original:
     1. 0x1F1F one (D code, G effect).
     2. Text, 0xBEEF (Erik's ruling): "X" -> "Y".
*/
int g_original;
int f(void) { return g_original ? 1 : 0; }   /* BUG(1995) 1 */
int g(void) { return g_original ? 2 : 0; }   /* BUG(1995) (text) */
'''

# fix 7, several text items: kind 5
TXTM2 = r'''/* Test TXTM2.
   1995 bugs, fixed unless g_original:
     1. 0x2020 one (D code, G effect).
     2. Text, 0xB111 (ruling): "A" -> "B".
     3. Text, 0xB222 (ruling): "C" -> "D".
*/
int g_original;
int f(void) { return g_original ? 1 : 0; }   /* BUG(1995) 1 */
int g(void) { return g_original ? 2 : 0; }   /* BUG(1995) (text) */
'''

# fix 8: kind-3 guard words count on the next line too, and in the plural
GRD = r'''/* Test GRD.
   1995 bugs, fixed unless g_original:
     1. 0x2121 one (D code, G effect).
*/
int g_original;
int f(void) { return g_original ? 1 : 0; }   /* BUG(1995) 1 */














int far(void) {
    return g_original ? 3 : 0;
    /* text slips fixed (TEXT_SWEEP.md) */
}
'''

# fix 9: "none" followed by beyond / other than / except is not "none"
NONEB = r'''/* Test NONEB. 1995 bugs: none fixed beyond the garbage read above.
     1. 0x2222 one (D code, G effect).
*/
int g_original;
int f(void) { return g_original ? 1 : 0; }   /* BUG(1995) 1 */
'''

T20B = {'parent.c': PARENT, 'keys.c': KEYS, 'sect.c': SECT, 'cont.c': CONT, 'cite.c': CITE,
        'dash.c': DASH, 'dash2.c': DASH2, 'txtm.c': TXTM, 'txtm2.c': TXTM2, 'grd.c': GRD, 'noneb.c': NONEB}

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

    # ---- TASK 20b cases ----
    for name, src in T20B.items():
        write(tmp, name, src)
    rc, out = run_check(tmp)
    lines = out.splitlines()

    def findings_for(name):
        return [l for l in lines if l.startswith(name + ':') and not l.startswith('counts:')]

    # fix 1: parent.c is clean and its list has 1, 3a, 3b but not 3
    check(not findings_for('parent.c'), f'parent.c findings: {findings_for("parent.c")}')
    # fix 2: keys.c reports the ambiguous label (kind 5) and the uncited heading item 1 (kind 1)
    kf = findings_for('keys.c')
    check(any('kind 5' in l and 'several lists' in l and 'label 1' in l for l in kf), f'keys.c kind 5: {kf}')
    check(any('kind 1' in l and 'heading/1' in l for l in kf), f'keys.c kind 1: {kf}')
    check(not any('kind 1' in l and 'both modes/1' in l for l in kf), f'keys.c both modes/1 should be cited via the hint: {kf}')
    # fix 3: sect.c is clean
    check(not findings_for('sect.c'), f'sect.c findings: {findings_for("sect.c")}')
    # fix 4: cont.c is clean
    check(not findings_for('cont.c'), f'cont.c findings: {findings_for("cont.c")}')
    # fix 5: cite.c is clean (both citations read mid-comment / across lines)
    check(not findings_for('cite.c'), f'cite.c findings: {findings_for("cite.c")}')
    # fix 6: dash.c is clean; dash2.c has no items and no findings
    check(not findings_for('dash.c'), f'dash.c findings: {findings_for("dash.c")}')
    check(not findings_for('dash2.c'), f'dash2.c findings: {findings_for("dash2.c")}')
    # fix 7: txm.c is clean; txm2.c reports several text items (kind 5)
    check(not findings_for('txtm.c'), f'txtm.c findings: {findings_for("txtm.c")}')
    check(any('kind 5' in l and 'several text items' in l for l in findings_for('txtm2.c')), f'txtm2.c kind 5: {findings_for("txtm2.c")}')
    # fix 8: grd.c has no kind 3 (the "text slips fixed" is on the next line)
    check(not any('kind 3' in l for l in findings_for('grd.c')), f'grd.c kind 3: {findings_for("grd.c")}')
    # fix 9: noneb.c is clean and its item is read (without the fix: kind 2, label 1 not an item)
    check(not findings_for('noneb.c'), f'noneb.c findings: {findings_for("noneb.c")}')

    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        fixes.run_list(tmp, True)
    recs = json.loads(buf.getvalue())
    by = {(r['file'], r['list'], r['label']): r for r in recs}
    # fix 1: no item 3 in parent.c; sub-items 3a/3b present
    check(('parent.c', 'heading', '3') not in by, f'parent.c must not have item 3: {sorted(k for k in by if k[0] == "parent.c")}')
    check(('parent.c', 'heading', '3a') in by and ('parent.c', 'heading', '3b') in by, 'parent.c sub-items 3a/3b')
    # fix 3: the text item of sect.c has only its own address
    check(by[('sect.c', 'text', None)]['addrs'] == ['0xb123'], f'sect.c text addrs: {by[("sect.c", "text", None)]["addrs"]}')
    # fix 4: the both-modes item of cont.c stops before the note at the item indent
    check(by[('cont.c', 'both modes', '1')]['addrs'] == ['0x1414', '0x1515'],
          f'cont.c both-modes addrs: {by[("cont.c", "both modes", "1")]["addrs"]}')
    # fix 6: dash bullets are items keyed @<first address>; dash2.c's deep dashes are not list items
    dash_items = [r for r in recs if r['file'] == 'dash.c' and r['list'] == 'heading']
    check(sorted(r['addrs'][0] for r in dash_items) == ['0x1919', '0x1a1a'] and all(r['label'] is None for r in dash_items),
          f'dash.c items: {dash_items}')
    check(not any(r['file'] == 'dash2.c' and r['list'] in ('heading', 'both modes', 'text') for r in recs),
          f'dash2.c must have no list items: {[r for r in recs if r["file"] == "dash2.c"]}')
    # fix 7: the (text) marker of txm.c is attached to the numbered "Text" item
    check(by[('txtm.c', 'heading', '2')]['markers'] != [], f'txtm.c item 2 markers: {by[("txtm.c", "heading", "2")]}')
    # fix 9: noneb.c's heading list is parsed (item 1 exists)
    check(('noneb.c', 'heading', '1') in by, f'noneb.c items: {[r for r in recs if r["file"] == "noneb.c"]}')

    if fails == 0:
        print('fixes_test: all checks pass')
    else:
        print(f'fixes_test: {fails} FAILURES')
    return 1 if fails else 0


if __name__ == '__main__':
    sys.exit(main())
