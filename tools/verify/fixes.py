"""fixes.py - inventory and lint of the port's "1995 bug" notes (Task 20).

Usage: python -I fixes.py check [SRCDIR]            lint: findings, exit 0 none / 1 any
       python -I fixes.py list [SRCDIR] [--json]   one record per item
       python -I fixes.py check-doc [SRCDIR] [FIXES.md]

Most port/src/*.c files start with a comment listing the original's bugs found in their routines ("1995 bugs,
fixed unless g_original ..."); the code marks the fixes with `BUG(1995)` in comments next to the changed lines.
This tool reads those notes (it does not edit port/src): `check` reports where notes and code disagree, `list` is
the inventory docs/FIXES.md is written from, `check-doc` checks FIXES.md against `list`.

An inventory with a lint, not a strict parser: when in doubt it reports the place (kind 5) instead of guessing.

Finding kinds (check), one line each, grouped by kind, then counts:
  1  an item no marker cites (labelled, by trailing citation, or by address; a file's only item is cited by any
     marker in that file); items from the "fixed in both modes" lists count too
  2  a marker whose label is not an item of that file
  3  a g_original use in code (comments/strings stripped; not the definition in state.c nor the option in main.c)
     with no marker in the same function and none within 15 lines, and no "port guard" / "text slip fixed" /
     TEXT_SWEEP comment on the same line or the one before (a function body = a "{" at column 0 through the next
     "}" at column 0)
  4  a "none" file that has markers or code uses of g_original
  5  what it could not read (file:line each)
"""
import argparse
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_SRC = os.path.join(HERE, '..', '..', 'port', 'src')
DEFAULT_DOC = os.path.join(HERE, '..', '..', 'docs', 'FIXES.md')

BUG = 'BUG(1995)'
ADDR_RE = re.compile(r'\b-?0x[0-9A-Fa-f]{4,6}\b')
NONE_RES = [re.compile(r'1995\s*bugs?\b[^\n]{0,60}?\bnone\b', re.I),
            re.compile(r'\bno 1995 bug', re.I),
            re.compile(r'\bno 1995 fix', re.I),
            re.compile(r'no routine here has a 1995 fix', re.I)]
HEADING_RE = re.compile(r'1995\s*bug', re.I)
LIST_END_WORDS = ('Kept', 'Oddit', 'Noted', 'Guards', 'Port guard', 'Text', 'Easter', 'Bugs and oddities kept')
BOTH_MODES_INTROS = (re.compile(r'fixed in both modes', re.I),
                     re.compile(r'stack words read before they are written', re.I),
                     re.compile(r'reads of stack garbage', re.I))
TEXT_INTRO_RE = re.compile(r'^\s*Text\b', re.I)
LABEL_RE = r'([1-9][0-9]*[a-z]?|[A-Z])(?![0-9A-Fa-f])'
LABEL_AFTER_RE = re.compile(r'^\s*:?\s*(?:bug\s+|item\s+|fix\s+)?' + LABEL_RE +
                            r'(?:\s+and\s+(?:bug\s+|item\s+)?' + LABEL_RE + r')?')
LABEL_BEFORE_RE = re.compile(r'(?<![0-9A-Fa-f])(?:item\s+|bug\s+|fix\s+)?' + LABEL_RE + r'\s*\(\s*' + re.escape(BUG))
TRAIL_CITE_RE = re.compile(r'\((?:the\s+)?file comment(?:,\s*(?:bug\s+)?' + LABEL_RE + r')?\)\s*\*/\s*$')
BARE_CITE_RE = re.compile(r'\(' + LABEL_RE + r'\)\s*\*/\s*$')
REFERENCE_RE = re.compile(r'(\bsee\b|\bguard of\b|\bthe\b[^.;]*\bguard of\b)\s*$', re.I)
ITEM_NUM_RE = re.compile(r'^(\d+)[.)]\s+')
ITEM_CAP_RE = re.compile(r'^(?:' + re.escape(BUG) + r'\s*)?([A-Z])(?:[.:]|\s+\()')
ITEM_SUB_RE = re.compile(r'^([a-z])[.)]\s+')
ITEM_DASH_RE = re.compile(r'^-\s+')
GRADE_RE = re.compile(r'\b([DG])\b')


class Item:
    def __init__(self, fname, line, list_name, label, mode, text):
        self.file = fname
        self.line = line
        self.list = list_name
        self.label = label          # None = unlabelled
        self.mode = mode            # 'fixed unless --original' / 'fixed in both modes' / 'text'
        self.text = text
        self.addrs = []
        self.markers = []           # (line, style)
        self.code = None
        self.effect = None

    def key(self):
        return (self.file, self.label if self.label is not None else f'@{self.addrs[0] if self.addrs else self.line}')


class Marker:
    def __init__(self, fname, line, labels, text_fix, cite):
        self.file = fname
        self.line = line
        self.labels = labels        # set, may be empty
        self.text_fix = text_fix
        self.cite = cite            # trailing citation label or None
        self.unreadable = False


def strip_code(text):
    """Per-line code with comments and string/char literals blanked out (state carried across lines)."""
    out = []
    incomment = False
    instring = False
    instr = False
    esc = False
    for line in text.split('\n'):
        res = []
        i = 0
        while i < len(line):
            c = line[i]
            if incomment:
                if c == '*' and i + 1 < len(line) and line[i + 1] == '/':
                    incomment = False
                    i += 2
                    continue
                i += 1
                continue
            if instring or instr:
                if esc:
                    esc = False
                elif c == '\\':
                    esc = True
                elif instring and c == '"':
                    instring = False
                elif instr and c == "'":
                    instr = False
                i += 1
                continue
            if c == '/' and i + 1 < len(line) and line[i + 1] == '*':
                incomment = True
                i += 2
                continue
            if c == '/' and i + 1 < len(line) and line[i + 1] == '/':
                break
            if c == '"':
                instring = True
                i += 1
                continue
            if c == "'":
                instr = True
                i += 1
                continue
            res.append(c)
            i += 1
        out.append(''.join(res))
    return out


def find_first_comment(lines):
    """(start, end) 1-based inclusive of the file's first /* ... */ block; (0, -1) if none."""
    if not lines or not lines[0].lstrip().startswith('/*'):
        return (0, -1)
    if '*/' in lines[0]:
        return (1, 1)
    for i in range(1, len(lines)):
        if '*/' in lines[i]:
            return (1, i + 1)
    return (1, len(lines))          # unterminated: unparseable, reported as kind 5


def item_addresses(text):
    out = []
    for m in ADDR_RE.finditer(text):
        a = m.group(0)
        rest = text[m.end():]
        if a.startswith('-'):
            continue
        if re.match(r'\s*bytes\b', rest, re.I):
            continue
        out.append(a.lower())
    return out


def parse_grades(item):
    """Code grade = the D/G next to 'code' (or the first of an X / Y pair); effect grade = the one next to
    'effect'/'reach' (or the second of the pair). A missing grade is not a finding."""
    text = item.text
    toks = [(m.start(), m.group(1)) for m in GRADE_RE.finditer(text)]
    code = effect = None
    m = re.search(r'\bcode\b', text)
    if m:
        before = [t for t in toks if t[0] < m.start()]
        if before:
            code = before[-1][1]
    for w in ('effect', 'reach'):
        m = re.search(r'\b' + w + r'\b', text)
        if m:
            before = [t for t in toks if t[0] < m.start()]
            if before:
                effect = before[-1][1]
                break
    if code is None and toks:
        code = toks[0][1]
    if effect is None:
        m = re.search(r'\(([DG])\s*/\s*([DG])\)', text)
        if m:
            effect = m.group(2)
        elif len(toks) >= 2:
            effect = toks[1][1]
    item.code, item.effect = code, effect


def parse_head_lists(fname, lines, head_start, head_end, unread):
    """The '1995 bug' heading list plus the extra lists (both-modes, text). Returns (items, none_file, heading_line)."""
    head = lines[head_start - 1:head_end]          # 0-based slice of the first comment
    head_lineno = lambda i: head_start + i
    text = '\n'.join(head)
    none_file = any(r.search(text) for r in NONE_RES)
    items = []
    heading_line = None
    hidx = None
    for i, l in enumerate(head):
        if HEADING_RE.search(l):
            hidx = i
            heading_line = head_lineno(i)
            break
    if hidx is not None and not none_file:
        # the heading runs until a line ending in ':' or the first item line
        h_indent = len(head[hidx]) - len(head[hidx].lstrip())
        j = hidx + 1
        while j < len(head):
            l = head[j]
            s = l.strip()
            if ITEM_NUM_RE.match(s) or ITEM_CAP_RE.match(s) or ITEM_DASH_RE.match(s):
                break                      # an item line: the heading ends before it
            if l.rstrip().endswith(':'):
                j += 1
                break
            j += 1
        # the list: from j, items at the indent of the first item line
        list_indent = None
        k = j
        parent = None
        cur = None
        while k < len(head):
            l = head[k]
            s = l.strip()
            indent = len(l) - len(l.lstrip())
            if not s:
                k += 1
                continue
            if any(s.startswith(w) for w in LIST_END_WORDS):
                break
            if list_indent is not None and indent <= h_indent and not (ITEM_NUM_RE.match(s) or ITEM_CAP_RE.match(s) or ITEM_DASH_RE.match(s)):
                break                        # a line at the heading's indent that starts no item ends the list
            m = ITEM_NUM_RE.match(s)
            if m and (list_indent is None or indent <= list_indent + 2):
                if list_indent is None:
                    list_indent = indent
                parent = m.group(1)
                label = parent
                mode = 'fixed unless --original'
                it = Item(fname, head_lineno(k), 'heading', label, mode, s)
                items.append(it)
                cur = it
                k += 1
                continue
            m = ITEM_CAP_RE.match(s)
            if m and (list_indent is None or indent == list_indent):
                if list_indent is None:
                    list_indent = indent
                label = m.group(1)
                it = Item(fname, head_lineno(k), 'heading', label, 'fixed unless --original', s)
                items.append(it)
                cur = it
                parent = None
                k += 1
                continue
            m = ITEM_SUB_RE.match(s)
            if m and parent is not None and indent > list_indent:
                label = parent + m.group(1)
                it = Item(fname, head_lineno(k), 'heading', label, 'fixed unless --original', s)
                items.append(it)
                cur = it
                k += 1
                continue
            if ITEM_DASH_RE.match(s) and (list_indent is None or indent >= list_indent):
                if list_indent is None:
                    list_indent = indent
                if BUG in s:
                    it = Item(fname, head_lineno(k), 'heading', None, 'fixed unless --original', s)
                    items.append(it)
                    cur = it
                    parent = None
                    k += 1
                    continue
                k += 1
                continue
            if cur is not None and indent > (list_indent or 0):
                cur.text += ' ' + s
                k += 1
                continue
            if list_indent is None:
                k += 1
                continue
            break
        for it in items:
            it.addrs = item_addresses(it.text)
            parse_grades(it)
    # extra list 1: both-modes / stack-garbage numbered lists anywhere in the first comment
    for i, l in enumerate(head):
        if any(r.search(l) for r in BOTH_MODES_INTROS):
            list_name = 'both modes'
            # the intro can wrap; the numbered list starts at the first item line within 8 lines
            k = i + 1
            while k < len(head) and k < i + 9 and not ITEM_NUM_RE.match(head[k].strip()):
                k += 1
            parent = None
            cur = None
            while k < len(head):
                s = head[k].strip()
                if not s:
                    k += 1
                    continue
                m = ITEM_NUM_RE.match(s)
                if m:
                    parent = m.group(1)
                    it = Item(fname, head_lineno(k), list_name, parent, 'fixed in both modes', s)
                    items.append(it)
                    cur = it
                    k += 1
                    continue
                m = ITEM_SUB_RE.match(s)
                if m and parent is not None:
                    it = Item(fname, head_lineno(k), list_name, parent + m.group(1), 'fixed in both modes', s)
                    items.append(it)
                    cur = it
                    k += 1
                    continue
                if cur is not None and (len(head[k]) - len(head[k].lstrip())) > (len(l) - len(l.lstrip())):
                    cur.text += ' ' + s
                    k += 1
                    continue
                break
            for it in items:
                if it.list == list_name:
                    it.addrs = item_addresses(it.text)
                    parse_grades(it)
            break
    # extra list 2: Text paragraphs that say "fixed"
    for i, l in enumerate(head):
        if TEXT_INTRO_RE.match(l) and re.search(r'\bfixed\b', '\n'.join(head[i:i + 6]), re.I):
            # gather the paragraph
            para = [l.strip()]
            k = i + 1
            while k < len(head) and head[k].strip() and (len(head[k]) - len(head[k].lstrip())) > 2:
                para.append(head[k].strip())
                k += 1
            it = Item(fname, head_lineno(i), 'text', None, 'text', ' '.join(para))
            items.append(it)
            it.addrs = item_addresses(it.text)
            parse_grades(it)
    return items, none_file, heading_line


def parse_markers(fname, lines, head_end, unread):
    markers = []
    for i in range(head_end, len(lines)):          # 0-based; head_end is 1-based inclusive -> first code line index = head_end
        line = lines[i]
        if BUG not in line:
            continue
        lineno = i + 1
        pos = line.index(BUG)
        prefix = line[:pos]
        if REFERENCE_RE.search(prefix):
            continue                                # "see BUG(1995) below" etc.
        rest = line[pos + len(BUG):]
        labels = set()
        m = LABEL_AFTER_RE.match(rest)
        if m:
            labels.add(m.group(1))
            if m.group(2):
                labels.add(m.group(2))
        mb = LABEL_BEFORE_RE.search(line)
        if mb:
            labels.add(mb.group(1))
        text_fix = bool(re.search(r'\(\s*text\s*\)|\btext\b', rest[:20], re.I)) and not labels
        cite = None
        mc = TRAIL_CITE_RE.search(line)
        if mc:
            cite = mc.group(1)
        else:
            mc = BARE_CITE_RE.search(line)
            if mc:
                cite = mc.group(1)
        if not labels and not text_fix and not cite and not ADDR_RE.search(rest) and re.match(r'^\s*:?\s*[0-9@#?$%&*]', rest):
            mk = Marker(fname, lineno, labels, text_fix, cite)
            mk.unreadable = True
            unread.append((fname, lineno, 'unreadable label after BUG(1995)'))
            markers.append(mk)
            continue
        markers.append(Marker(fname, lineno, labels, text_fix, cite))
    return markers


def functions_map(lines):
    """line (1-based) -> function start line, from '{' at column 0 through the next '}' at column 0."""
    fstart = None
    fmap = {}
    for i, l in enumerate(lines):
        if l.startswith('{'):
            fstart = i + 1
        elif l.startswith('}') and fstart is not None:
            for j in range(fstart, i + 1):
                fmap[j] = fstart
            fstart = None
    if fstart is not None:
        for j in range(fstart, len(lines) + 1):
            fmap[j] = fstart
    return fmap


def g_original_uses(fname, code_lines):
    uses = []
    for i, code in enumerate(code_lines):
        if not re.search(r'\bg_original\b', code):
            continue
        if re.search(r'\bint\s+g_original\s*;', code):
            continue                              # the definition (state.c)
        if fname == 'main.c' and re.search(r'\bg_original\s*=\s*1\s*;', code):
            continue                              # the --original option assignment
        uses.append(i + 1)
    return uses


def parse_file(path):
    fname = os.path.basename(path)
    raw = open(path, encoding='utf-8', errors='replace').read()
    lines = raw.split('\n')
    code_lines = strip_code(raw)
    unread = []
    hs, he = find_first_comment(lines)
    if he == len(lines) and '*/' not in lines[-1]:
        unread.append((fname, 1, 'unterminated first comment'))
    items, none_file, heading_line = parse_head_lists(fname, lines, hs, he, unread)
    markers = parse_markers(fname, lines, he, unread)
    # extra list 3: in a file with no heading, each unlabelled marker is an item in place
    if heading_line is None:
        for m in markers:
            if not m.labels and not m.text_fix:
                it = Item(fname, m.line, 'code', None, 'fixed unless --original', lines[m.line - 1].strip())
                it.addrs = item_addresses(it.text)
                parse_grades(it)
                it.markers.append((m.line, 'in place'))
                items.append(it)
    fmap = functions_map(lines)
    guses = g_original_uses(fname, code_lines)
    return {'file': fname, 'lines': lines, 'code': code_lines, 'head': (hs, he), 'items': items,
            'none': none_file, 'heading': heading_line, 'markers': markers, 'fmap': fmap, 'guses': guses,
            'unread': unread}


def attach_markers(f):
    """Attach markers to items by label, trailing citation, address, or the only-item rule. Returns findings."""
    findings = []
    items = f['items']
    markers = f['markers']
    labels_of = {it.label for it in items if it.label is not None}
    cited = set()
    for m in markers:
        cited |= m.labels
        if m.cite:
            cited.add(m.cite)

    def add_marker(it, m, style):
        if not any(l[0] == m.line for l in it.markers):
            it.markers.append((m.line, style))

    # address-cited: a marker line sharing an address found in exactly one item
    for m in markers:
        addrs = set(a.lower() for a in item_addresses(f['lines'][m.line - 1]))
        if not addrs:
            continue
        owners = [it for it in items if addrs & set(it.addrs)]
        if len(owners) == 1:
            add_marker(owners[0], m, 'address')
            if owners[0].label:
                cited.add(owners[0].label)
                m.labels.add(owners[0].label)
    for m in markers:
        for lab in m.labels:
            if lab in labels_of:
                for it in items:
                    if it.label == lab:
                        add_marker(it, m, 'label')
        if m.cite and m.cite in labels_of:
            for it in items:
                if it.label == m.cite:
                    add_marker(it, m, 'citation')
    # the only-item rule
    if len(items) == 1 and markers:
        for m in markers:
            add_marker(items[0], m, 'only item')
    # text markers cite the file's single text item
    text_items = [it for it in items if it.mode == 'text']
    if len(text_items) == 1:
        for m in markers:
            if m.text_fix:
                add_marker(text_items[0], m, 'text')
    # unlabelled markers in multi-item files: address match done above; else kind 5
    if len(items) > 1:
        for m in markers:
            if not m.labels and not m.cite and not m.text_fix and not m.unreadable:
                if not any(ml[0] == m.line for it in items for ml in it.markers):
                    findings.append((f['file'], m.line, 5, 'unlabelled marker in a multi-item file'))
    return findings


def run_check(srcdir):
    findings = []
    files = []
    for name in sorted(os.listdir(srcdir)):
        if name.endswith('.c'):
            files.append(parse_file(os.path.join(srcdir, name)))
    for f in files:
        findings.extend(attach_markers(f))
        # kind 5: unreadable things
        for (fn, ln, what) in f['unread']:
            findings.append((fn, ln, 5, what))
        labels_of = {it.label for it in f['items'] if it.label is not None}
        # kind 1: items no marker cites
        for it in f['items']:
            if len(f['items']) == 1 and f['markers']:
                continue
            if it.label is not None and it.label in {l for m in f['markers'] for l in m.labels} | {m.cite for m in f['markers'] if m.cite}:
                continue
            if any(ml[0] and True for ml in it.markers):
                continue
            findings.append((f['file'], it.line, 1,
                             f'item {it.list}/{it.label if it.label is not None else "unlabelled"} not cited by any marker'))
        # kind 2: marker labels that are not items of this file
        for m in f['markers']:
            for lab in m.labels:
                if lab not in labels_of:
                    findings.append((f['file'], m.line, 2, f'marker label {lab} is not an item of this file'))
            if m.cite and m.cite not in labels_of:
                findings.append((f['file'], m.line, 2, f'citation {m.cite} is not an item of this file'))
        # kind 3: g_original use in code without a marker nearby
        marker_lines = {m.line for m in f['markers']}
        kind3 = set()
        for ln in f['guses']:
            fn = f['fmap'].get(ln)
            func_lines = set()
            if fn is not None:
                # function body = { at col 0 through next } at col 0
                start = fn
                end = fn
                for j in range(fn - 1, len(f['lines'])):
                    if f['lines'][j].startswith('}'):
                        end = j + 1
                        break
                func_lines = set(range(start, end + 1))
            near = any(abs(ml - ln) <= 15 for ml in marker_lines) or any(ml in func_lines for ml in marker_lines)
            guard = any(w in f['lines'][ln - 1] for w in ('port guard', 'text slip fixed', 'TEXT_SWEEP')) or \
                    any(w in f['lines'][ln - 2] for w in ('port guard', 'text slip fixed', 'TEXT_SWEEP')) if ln >= 2 else False
            if not near and not guard:
                findings.append((f['file'], ln, 3, 'g_original use in code with no marker nearby'))
                kind3.add(ln)
        # kind 4: a "none" file with markers that belong to no list, or unguarded g_original uses
        if f['none']:
            attached = {ml[0] for it in f['items'] for ml in it.markers}
            orphan_markers = [m for m in f['markers'] if m.line not in attached]
            if orphan_markers or kind3:
                findings.append((f['file'], f['heading'] or 1, 4,
                                 f'"none" file has {len(orphan_markers)} unattached marker(s) or {len(kind3)} unguarded g_original use(s)'))
    # no-list files: each unlabelled marker becomes an item in place (for list), here just noted
    by_kind = {}
    for (fn, ln, kind, text) in findings:
        by_kind.setdefault(kind, []).append((fn, ln, text))
    for kind in sorted(by_kind):
        for (fn, ln, text) in by_kind[kind]:
            print(f'{fn}:{ln}: kind {kind}: {text}')
    print('counts:', ', '.join(f'kind {k}: {len(v)}' for k, v in sorted(by_kind.items())) or 'none')
    return 1 if findings else 0


def collect_items(srcdir):
    """All items (plus marker-only records in files with no list) as dicts."""
    recs = []
    for name in sorted(os.listdir(srcdir)):
        if not name.endswith('.c'):
            continue
        f = parse_file(os.path.join(srcdir, name))
        attach_markers(f)
        if f['items']:
            for it in f['items']:
                recs.append({'file': it.file, 'list': it.list, 'label': it.label, 'mode': it.mode,
                             'code': it.code, 'effect': it.effect, 'addrs': it.addrs,
                             'text': re.sub(r'\s+', ' ', it.text).strip(),
                             'markers': [f'{it.file}:{ln}' for (ln, _s) in it.markers]})
        else:
            for m in f['markers']:
                lab = sorted(m.labels)[0] if m.labels else None
                recs.append({'file': f['file'], 'list': 'code', 'label': lab,
                             'mode': 'text' if m.text_fix else 'fixed unless --original',
                             'code': None, 'effect': None,
                             'addrs': item_addresses(f['lines'][m.line - 1]),
                             'text': re.sub(r'\s+', ' ', f['lines'][m.line - 1].strip()),
                             'markers': [f'{f["file"]}:{m.line}']})
    return recs


def run_list(srcdir, as_json):
    recs = collect_items(srcdir)
    if as_json:
        print(json.dumps(recs, indent=1))
        return 0
    for r in recs:
        lab = r['label'] if r['label'] is not None else '-'
        addr = r['addrs'][0] if r['addrs'] else '-'
        print(f'{r["file"]}\t{r["list"]}\t{lab}\t{r["mode"]}\tcode={r["code"] or "?"}\teffect={r["effect"] or "?"}\t{addr}\t'
              f'markers={",".join(r["markers"])}\t{re.sub(r"chr(10)", " ", r["text"])[:160]}')
    return 0


ROW_RE = re.compile(r'([a-z0-9_]+\.c)\s+([0-9]+[a-z]?|[A-Z]|@[0-9A-Fa-fx]+)\s+(0x[0-9A-Fa-f]{4,6})')


def run_check_doc(srcdir, docpath):
    recs = collect_items(srcdir)
    known = {(r['file'], r['label'] if r['label'] is not None else '@' + (r['addrs'][0][2:] if r['addrs'] else '0')) for r in recs}
    text = open(docpath, encoding='utf-8').read() if os.path.exists(docpath) else ''
    doc_keys = set()
    for line in text.split('\n'):
        for m in ROW_RE.finditer(line):
            doc_keys.add((m.group(1), m.group(2)))
    missing = sorted(k for k in known if k not in doc_keys)
    extra = sorted(k for k in doc_keys if k not in known)
    for k in missing:
        print(f'not in FIXES.md: {k[0]} {k[1]}')
    for k in extra:
        print(f'in FIXES.md but not in list: {k[0]} {k[1]}')
    print(f'{len(known)} items, {len(doc_keys)} rows: {len(missing)} missing, {len(extra)} extra')
    return 1 if (missing or extra) else 0


def main():
    ap = argparse.ArgumentParser(description='inventory and lint of the port\'s 1995-bug notes')
    sub = ap.add_subparsers(dest='cmd')
    for name in ('check', 'list', 'check-doc'):
        sp = sub.add_parser(name)
        sp.add_argument('srcdir', nargs='?', default=DEFAULT_SRC)
        if name == 'list':
            sp.add_argument('--json', action='store_true')
        if name == 'check-doc':
            sp.add_argument('doc', nargs='?', default=DEFAULT_DOC)
    a = ap.parse_args()
    if not a.cmd:
        ap.error('need a command: check, list or check-doc')
    if not os.path.isdir(a.srcdir):
        print(f'fixes: {a.srcdir} is not a folder', file=sys.stderr)
        return 2
    if a.cmd == 'check':
        return run_check(a.srcdir)
    if a.cmd == 'list':
        return run_list(a.srcdir, a.json)
    return run_check_doc(a.srcdir, a.doc)


if __name__ == '__main__':
    sys.exit(main())
