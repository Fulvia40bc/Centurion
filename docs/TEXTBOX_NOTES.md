# Text and message boxes (EXE 59D4, 8C9F, 8BE4, 899C, 8C58, 3134, ...)

Status words: **VERIFIED** = read in the code and, for drawing, matched against a capture pixel by pixel with a
python script; **DECODED** = read in the code only; **GUESS** = inferred.
All addresses are 0823 offsets (EXE code segment), DS = 1686. Calls are Pascal style: the first argument pushed is
the highest `[bp+N]`, the callee pops (`ret 2*nargs`). Below, arguments are written in push order.

## 1. Font globals (DECODED; values read from the runtime DS in t10_in.bin and mem_b10_exit.bin)

| DS | value | meaning |
|---|---|---|
| 9B56 | 9C9C | glyph width table, indexed by `c - 0x20` (e.g. ' ' = 3, 'A' = 4, 'M' = 7, 'i' = 1) |
| 9B58 | 9CFC | per-glyph y offset table, indexed by `c - 0x20` (1 for most, 0/2/3 for some) |
| 9B5A | 9D5C | glyph bitmaps: `c` at `9D5C + ((c-0x20) << [9B5F]) * [9B5C]`, one byte per row, bit 7 = left |
| 9B5C | 8 | glyph height |
| 9B5D | 3 | line gap -> line height `LH = 9B5C + 9B5D = 11` |
| 9B5E | 1 | spacing added after every glyph |
| 9B5F | 0 | shift for the glyph index |
| 9B69 | word | text colour: low nibble = foreground (only 0..15 possible), high nibble = background colour for 899C's background mode; high byte & 0x3F = blit flags |
| 9B6A | byte | bit 7 set = 899C paints a background band behind each line |
| 9AFA | word | current fill/line colour for 7E74 / 925C / 7ECA |
| 6346 | 0x1A = 26 | the standard box fill colour passed to 3134 |
| 3E40 | ptr | run time (with a mouse) 3E2E "Click to continue"; static OVL0 3E16 "Hit any key to continue". Only read in the EXE listing; written once by the resident overlay start-up (OVL0 file 0x0105: `if ([00BA]) { if (mouse_init()) [3E40] = 3E2E; else [00BA] = 0; }`). (3E42 is not a string pointer: it is the Yes/No item list of record 3E5A, first item text 3E0E "Yes", MENU_NOTES correction 4.) |
| 3E02 | "" | an empty string, used by the race overlay for a bare "Click to continue" box |

The port's `font_glyph()`/`ui_text()` reproduce 8D56 exactly (the battle report proves it). Port relation:
`ui_text_width(s) == 8C58(s) - 1` for a non-empty line, both 0 for an empty one.

## 2. The primitives

### 8C58 width(str) - DECODED, VERIFIED through the captures below
```c
int w8c58(const char *s) {               /* stops at NUL or '\n'; chars < 0x20 add nothing */
    int t = 0;
    for (; s && *s && *s != '\n'; s++)
        if ((unsigned char)*s >= 0x20) t += width[*s - 0x20] + 1 /* [9B5E] */;
    return t;                             /* = advance width INCLUDING the trailing spacing column */
}
```
"Click to continue" = 72, "Release The Legion!" = 86, "Run Away!" = 45.

### 8D56 glyph(ch, x, y) - DECODED, VERIFIED
`y += yoff[ch-0x20]`; 8 rows; set bits are painted in colour `[9B69] & 0x0F`, clear bits are transparent (the
VGA path is `jmp [9B60 + [9B26]]` with [9B26] = 2 -> 8DD1, which expands the glyph and calls a masked blit).
Width 0 glyphs draw nothing.

### 899C text(str, x, y) - DECODED (no-background path VERIFIED)
```c
void t899c(const char *s, int x, int y) {
    if ((B[0x9B6A] & 0x80) && *s) {            /* background mode (not used by the boxes here) */
        save = [9AFA]; [9AFA] = ([9B69] & 0xF0) >> 4;
        for each line l (a leading '\n' skipped):  /* yy starts at y, += LH per line */
            7E74(x - 1, yy + yoff[0] - 2, x + w8c58(l), yy + LH - 1);   /* yoff[0] = [[9B58]] = 1 */
        [9AFA] = save;
    }
    int sx = x;
    for (; *s; s++) {
        if (*s == '\n') { sx = x; y += LH; }
        else if (*s >= 0x20) { 8D56(*s, sx, y); sx += width[*s-0x20] + 1; }
    }
}
```
So multi-line text through 899C is LEFT aligned at x.

### 8BE4 centred_text(str, cx, y) - DECODED, VERIFIED
```c
void t8be4(char *s, int cx, int y) {            /* each '\n' line centred on cx */
    while (s) {
        char *nl = strchr(s, '\n'); if (nl) *nl = 0;
        899C(s, cx - (w8c58(s) >> 1), y);       /* shr: unsigned halving, floor */
        if (nl) *nl++ = '\n';
        y += LH;                                /* 11 */
        s = nl;
    }
}
```
Port: line x = `cx - (ui_text_width(l) + 1) / 2` (empty line: nothing drawn, y still advances).

### 8C3A right_text(str, xr, y) - DECODED
`899C(str, xr - w8c58(str), y)`: the advance ends at xr, so the last ink column is <= xr - 2.

### 8A9E / 8AC8 numbers(lo, hi, x, y) - DECODED, VERIFIED (battle report)
Push order: hi word, lo word, x, y. Both `ltoa(hi:lo, buf, 10)` (A022) then 8A9E -> `899C(buf, x, y)` (left
aligned), 8AC8 -> `8C3A(buf, x, y)` (right aligned at x). Port: `ui_text(x - ui_text_width(s) - 1, y)` for 8AC8.

### 8AF2 number_field(hi, lo, x, y, w) - DECODED
`7E74(x, y + yoff[0], x + w, y + yoff[0] + 8)` in the current [9AFA] colour (clears the field), then 8A9E(hi, lo, x, y).

### 8C9F text_rect(str, minw, lines0, cx, cy, out*) - DECODED, VERIFIED (4 captures)
```c
int *r8c9f(const char *s, int minw, int lines0, int cx, int cy, int out[4]) {
    int n = lines0, mw = minw;                     /* signed compares */
    if (strlen(s)) {
        const char *p = s;
        do {
            if (*p == '\n') p++;                   /* also skips a LEADING '\n' (quirk, see 59D4) */
            if (w8c58(p) > mw) mw = w8c58(p);
            n++;
            p = strchr(p, '\n');
        } while (p);                               /* a trailing '\n' counts one more (empty) line */
    }
    int h = (n * LH) / 2, hw = mw / 2;             /* cwd/sub/sar: C division, toward 0 */
    out[0] = cx - hw; out[1] = cy - h; out[2] = cx + hw; out[3] = cy + h;   /* x0, y0, x1, y1 */
    return out;
}
```

### 3134 box(x0, y0, x1, y1, fill) - DECODED, VERIFIED (= port ui_frame)
Corners inclusive. `fill == -1` -> no fill.
```c
if (fill != -1) { [9AFA] = fill; 7E74(x0, y0, x1, y1); }     /* filled rect */
[9AFA] = 8:  vline(x0, y0..y1); vline(x0+1, y0+1..y1-1); vline(x1, y0..y1); vline(x1-1, y0+1..y1-1);
             vline(x0+3, y0+3..y1-3); hline(x0+3..x1-3, y1-3); vline(x1-3, y0+3..y1-3); hline(x0+3..x1-3, y0+3);
[9AFA] = 0:  hline(x0..x1, y1); hline(x0+1..x1-1, y1-1);
[9AFA] = 15: hline(x0..x1, y0); hline(x0+1..x1-1, y0+1);
```
925C(xa, ya, xb, yb) = line, 7ECA(xa, y, xb) = horizontal line, both inclusive (VERIFIED by the pixel match).
Because 15 is drawn last, (x0, y0+1) stays 8 and (x1, y0+1) stays 8: the "mitre" the port draws.
Port equivalent: `ui_frame(fb, x0, y0, x1 - x0 + 1, y1 - y0 + 1)` with fill 26. 7E74 inclusiveness itself is a
GUESS (its edge pixels are always overdrawn here).

### 691B save(x0, y0, x1, y1) / 6973 restore(x0, y0, x1, y1) - DECODED
Mouse hidden (1D55(0)) around both. VGA ([9B26] != 6): 691B = `950E(src = 9AB6 (VRAM A000), dst = C1DA (the
320x200 work buffer at segment 2687 = linear 0x26870), &rect, x0, y0)`, 6973 = the reverse. The work buffer is
used as the save area at the SAME coordinates, so a message box overwrites that rectangle of the work buffer with
whatever was on the screen.

### 35F9 wait_click - DECODED
`35DB(); 359B(); 35DB();` = wait until no button is down, wait for a click or key, wait for release.
359B: returns 1 (left) / 2 (right) when [B2] has a button; keys (03AA via 3603): Enter (0x0D) and '+' act as the
right button, Space (0x20) and 0x10 as the left button, any other key returns 0 with the key in [C07F]; the
joystick / arrow movement of the cursor is handled in 3603 (3653..36DD). So any key or button closes the box.

## 3. 59D4 message_box(str) - DECODED, VERIFIED on 3 captures

```c
void message_box(char *s) {                 /* 59D4, ret 2 */
    if (!s) return;                          /* NULL pointer only; "" still shows a box */
    1D55(0);                                 /* hide mouse */
    int r[4]; 8C9F(s, w8c58("Click to continue") /* 72 */, 1, 159, 99, r);
    int X0 = r[0] - 7, Y0 = r[1] - 7, X1 = r[2] + 7, Y1 = r[3] + 7;
    691B(X0, Y0, X1, Y1);                    /* save screen under the box */
    3134(X0, Y0, X1, Y1, [6346] /* 26 */);
    [9B69] = 0;  8BE4(s, (X0 + X1) / 2 /* = 159 */, Y0 + 7);
    [9B69] = 15; 8BE4("Click to continue", 159, Y1 - 15);
    35F9();                                  /* wait release, click/key, release */
    6973(X0, Y0, X1, Y1);                    /* put the screen back */
    1D8A();                                  /* mouse back */
}                                            /* side effect: [9B69] stays 15 */
```

Closed form (port terms; `W(l) = ui_text_width(l) + 1` for a non-empty line, 0 for empty):
```
k  = number of '\n'-separated lines of s (0 for ""), n = k + 1        (the +1 is the click line)
M  = max(72, W(line_i))
X0 = 159 - M/2 - 7      X1 = 159 + M/2 + 7      (M/2 floors)
Y0 = 99 - (11n)/2 - 7   Y1 = 99 + (11n)/2 + 7   ((11n)/2 floors)
ui_frame(X0, Y0, X1 - X0 + 1, Y1 - Y0 + 1)          fill 26, borders 15 / 8 / 0
line i : ui_text(159 - W(line_i)/2, Y0 + 7 + 11*i, line_i, 0)                (black)
click  : ui_text(123, Y1 - 15, "Click to continue", 15)                       (white, x = 159 - 36)
```
The click line sits 3 px (n even) or 2 px (n odd) below where text line k would be: `Y1 - 15 = Y0 + 11n - 1`
for even 11n, `Y0 + 11n - 2` for odd. The box is shown until a click/key and then erased (screen restored);
nothing stays behind.

Quirk (DECODED): a string that starts with '\n' is measured without the leading empty line (8C9F skips it) but
drawn with it (8BE4 draws an empty first line), so its text would sit one line lower than the box was sized for.
None of the strings found below starts with '\n' (DS:3D9C is a byte variable that happens to hold 0x0A; the
Marauding message starts at 3D9D).

### Captures (VERIFIED, pixel by pixel)
The DOSBox screenshots C:\DOSBox-X\hunt\shots\*.png (640x400 RGBA, 2x) were turned back into palette indices
(nearest CMAP colour with the 6-bit -> 8-bit scale `(v*255+31)/63`; with `v<<2` a few pixels of colour 26 land on
229 = (40,36,33), a rounding artefact of the screenshot only). The python model of 8C58/8C9F/3134/8BE4/8D56/59D4
(font tables read from the runtime DS) drawn over the capture gives 0 differing pixels on the whole screen, and
drawn on a blank canvas it matches every pixel of the box area:

| shot | text | n | box (X0,Y0,X1,Y1) | lines (x,y) | click (x,y) | box px matched |
|---|---|---|---|---|---|---|
| b1.png (map, 266 BC) | "Albertus Discreditus is assigned\nto command Rome" (W 140, 81) | 3 | 82,76,236,122 | (89,83) (119,94) | (123,107) | 7285 / 7285 |
| p8.png (map, 275 BC) | "Gaul" (W 17) | 2 | 116,81,202,117 | (151,88) | (123,102) | 3219 / 3219 |
| s08.png (chariot race) | "" (DS:3E02, race overlay 0823:C429 = OVL4 file off 0x1349) | 1 | 116,87,202,111 | - | (123,96) | 2175 / 2175 |

For s08 the one-pixel ring around the box is uniformly colour 17 (the race background), so the box extent is exact.
Scripts: scratchpad `wf/tb.py` (model), `wf/v59.py` (check), `wf/png2idx.py` (PNG -> indices), `wf/scan.py`
(finds bevelled boxes in any 64000-byte frame).

### 'Release The Legion!' (no picture)
No capture shows it. The 640K snapshots cannot: VRAM (A000) is outside them, and the box is drawn on VRAM only.
mem_w005_D14A and mem_w006_D170 have identical work buffers at 0x26870 (0 bytes differ): D162 saved the whole
screen there (691B(0,0,319,199)); 59D4's own 691B then copied the VRAM under its box (identical to that save, as
nothing was drawn there) to the same place, and 6973 put it back. w006 does prove the message ran: DS:AC60 =
"Release The Legion!" (built at D21E from [74B6] "Release" + [74B8] " The Legion!"). The stack slots of 59D4's
rect (CB00..CB0F) were overwritten by later calls. Predicted (same rules as the verified shots):

| message | caller | n | M | box | lines | click |
|---|---|---|---|---|---|---|
| "Release The Legion!" | battle D21E -> D23C | 2 | 86 | 109,81,209,117 | (116,88) | (123,102) |
| "Run Away!\nRun Away!" (DS:74D4) | battle D299 | 3 | 72 | 116,76,202,122 | (137,83) (137,94) | (123,107) |
| "The Marauding Army of Macedonia\nMeets the Roman Garrison" | EXE 508D..50C1 | 3 | 155 | 75,76,243,122 | (82,83) (100,94) | (123,107) |
| "Run Away!" (DS:74E8) | battle D7A3 | 2 | 72 | 116,81,202,117 | (137,88) | (123,102) |

(The 3-line rows are the same geometry as the verified b1/p8 cases; only the strings are unverified.)

## 4. The battle report (OVL_BATTLE E48B..E58B) explained - VERIFIED

```c
strcpy(AC60, [7ADC]); for (si = 7ADE; si < 7B06; si += 2) strcat(AC60, *si);   /* title + text, 20 pieces */
8C9F(AC60, w8c58([7AD4]), 0, 159, 99, r);            /* no extra click line (lines0 = 0) */
X0 = r0 - 7; Y0 = r1 - 7; X1 = r2 + 7; Y1 = r3 + 7;
5D29([C2A6]);                                        /* (not a drawing call for the box; DECODED only) */
[9B69] = 0; 3134(X0, Y0, X1, Y1, [6346] /* 26 */);
8BE4(AC60, (X0 + X1) / 2, Y0 + 7);
8AC8(Rome casualties, 0x78, 0x8A); 8A9E(enemy casualties, 0xC4, 0x8A);
8AC8(Rome survivors,  0x78, 0x95); 8A9E(enemy survivors,  0xC4, 0x95);
35F9();                                              /* no 691B / 6973: the box stays on the screen */
```
minw = 8C58 of the first line of the string [7AD4] points to (e.g. "an" from "an\nignominious defeat for
Rome\n\n"): it never beats the widest report line. No "Click to continue".

Check: scr_battle_report.bin with the text "A Glorious Victory!\n\nThe legionnaires of Rome have met the army of
Carthago\non the field of battle.  Hannibal's army used the\nOutflank tactic against the Scipio's Defense
of\nAlbertus Discreditus and the outcome was a\nglorious victory for Rome!\n\nRome  vs  Carthago\nCasualties\nSurvivors"
(11 lines, widest W = 260) and the numbers 641 / 3146 / 5659 / 1534: rect (29,39,289,159), box (22,32,296,166),
**0 differing pixels on the whole screen, 37125 / 37125 box pixels** (`wf/vr2.py`).

Why port/src/battle_ui.c battle_report_draw matches: with `pw = ui_text_width` (W = pw + 1) it uses
`h = 11n - 3, top = 99 - h/2, left = 159 - (pw+1)/2`, box `(left-7, top-8, pw+16, h+17)`, lines at
`left + (pw - w + 1)/2, top - 1 + 11i`. That equals the code rule only when n is odd AND W is even. Checked
for n = 1..13, pw = 5..299:
- n even: box top and every text line 1 px too low (the code's `(11n)/2` vs the port's `(11n-3)/2`).
- W odd: box right edge 1 px too far right, and lines whose own W is odd 1 px too far right.
The capture had n = 11, W = 260, so it could not show this. Exact form for the port:
`X0 = 159 - W/2 - 7, X1 = 159 + W/2 + 7, Y0 = 99 - (11n)/2 - 7, Y1 = 99 + (11n)/2 + 7`, `ui_frame(X0, Y0,
X1-X0+1, Y1-Y0+1)`, line i at `(159 - W_i/2, Y0 + 7 + 11i)` (W = max over lines of W_i, n = line count
including empty ones and one extra line for a trailing '\n').

## 5. Message builders that end in 59D4 (DECODED)

- **5A9A(a, b)**: `strcpy(AC60, a); strcat(AC60, b); 59D4(AC60)`. Callers: 10B0 1164 121A 130B 1347 14FD 15CD 16C9 16E9 1924.
- **4BAC(army, zone, num, name)**: `[3D7C] = 5222[home zone of army (word C114 + 14*army)]; [3D80] = 5222[zone];
  6738(num) -> C072 (Roman numeral: 4F82 hundreds, 4F6E tens, 4F5A units; > 999 -> "OOPS"); [3D86] = name;
  [3D8A] = zone >= 0x1B ? [3D8E] "rolling waves of destiny." : [3D8C] "field of battle.";`
  AC60 = [3D7A] "An army from " + 3D7C.. 3D8A pieces = `"An army from <home>\ninvades <zone>\n<numeral><name> meets
  the enemy on\nthe <field of battle.|rolling waves of destiny.>"`, then 59D4(AC60) (4 lines -> n = 5, box
  Y 65..133). Callers 4B11 (fleets: name from 1100[] "Fleet I"...), 4B78, 5049 (legions: name = record - 0xD), 520E; each is followed by 1E21.
- **508D..50C1**: `"The Marauding Army of " (3D9D) + 5222[home] + "\nMeets the Roman Garrison" (3DB4)` -> 59D4.
- **Battle**: D21E "Release The Legion!", D299 "Run Away!\nRun Away!", D7A3 "Run Away!" (74E8), D84D
  `"Release" + " Cohort" + " " + roman(n)` (74B6 + 74BA + 74F2 + 6738).
- **Race overlay** (OVL4, call target shown as 0xffffa8f4 in OVL_RACE.clean.asm = 59D4): off 1349 with DS:3E02 ""
  (bare click box, VERIFIED s08), 16C4 (DS:8930), 173E [8820], 176E [8822], 19DE table at DS:880C, 1ABC [8816].
- Other EXE callers of 59D4: 098E 0E09 0EDC 1058 1061 1E7C 1E85 2199 2216 250C 2D18 2D4C 33EF 344C 348E 3C5A 3C9C
  4054 40D5 4129 43BB 44C4 4591 480B 4C1A 50C1 52F8 5AB9 5FF3 6CA6 6E8E 6EB0 72D3 74CD 7525 770B 777A 7B4D.

## 6. Not covered / open

- s10.png shows a different two-button question box ("You Have Not Bet / Race Anyway? / Yes No", box at
  x 113..205, top y 76) and s02.png the copy-protection question with an input bar: other routines (69CC is a
  candidate, it reads 9B5C/9B5D like 8C9F), not decoded here.
- 5AE4/5AC2: the scrolling message log (lines appended at x 0x78, y from [C1D8], scrolls 0x56..0x131 with 950E
  when past y 0xBE); 5AC2(n) adds n empty lines (DS:3E02). Only skimmed.
- 925C, 7ECA, 7E74 internals (only their effect is verified).
- What the port should change (not done, port/src untouched): battle_ui.c `msg_hook`/`text_box` use
  `w = wmax + 24, h = rows*11 + 16` centred by (320 - w)/2 and draw "Click to continue" in black as an ordinary
  line; the real 59D4 rule is section 3 (click line white at Y1 - 15, min width 72, centre (159, 99)).
  battle_report_draw should take the exact form in section 4.

## Corrections (verifier)

Independent re-check (own model written from the disassembly, own PNG decoder comparing RGB against CMAP, not
using tb.py/v59.py/vr2.py/png2idx.py): scratchpad `wf/vv/model.py`, `wf/vv/check59.py`, `wf/vv/checkrep.py`,
`wf/vv/portcmp.py`, `wf/vv/pred.py`. Confirmed: 59D4 geometry/colours on b1, p8, s08 (0 mismatches over 7285 /
3219 / 2175 box pixels; shifting the model by 1 px in any direction gives 375..1705 mismatches, so the placement is
exact), the battle report (0 diffs over the whole scr_battle_report.bin, 37125/37125 box pixels; swapping the
numbers gives 234 mismatches, so the 8AC8/8A9E positions are pinned), 3134 order 3134..3230, 8C9F rounding
(8D0C/8D27 cwd; sub; sar = toward 0), 8BE4 `shr` at 8C06, 691B/6973 surfaces ([9AB6] seg A000, [C1DA] seg 2687,
stride 0x140), the 'Release The Legion!' evidence (DS:AC60 in w006, work buffer 0 bytes different) and the
predicted table in section 3. Corrections:

1. **Section 4, port mismatch parity is wrong for the lines.** With `W` = widest 8C58 width odd, the lines that
   land 1 px too far right are those whose own 8C58 width `W_i` is **even** (port `ui_text_width` odd), not odd.
   Proof: port x = `159 - W/2 + (W - W_i + 1)/2`; W odd, W_i even -> `160 - W_i/2` vs code `159 - (W_i >> 1)`
   (8C06); W odd, W_i odd -> equal. Brute force n = 1..13, W = 6..300, W_i = 2..W (`wf/vv/portcmp.py`):
   line dx = +1 only for (W odd, W_i even); box deltas (x0,y0,x1,y1) = (0,0,0,0) n odd/W even, (0,0,+1,0)
   n odd/W odd, (0,+1,0,0) n even/W even, (0,+1,+1,0) n even/W odd; text dy = +1 for every line when n is even.
   (So for even n the box bottom is right and the box is 1 px short at the top.) The "exact form" given in
   section 4 is correct and is what the port should use.
2. **35F9 / 359B: not every key closes the box** (DECODED, 03AA..03D2, 3603..364A). 03AA reads INT 16h AH=0 and
   loops back (03CC `je 03AA`) on AL = 0, so arrow keys, F-keys and other AL=0 extended keys are swallowed and do
   NOT close a box; Ctrl-S (AL = 0x13) is zeroed at 03C6 and also ignored. Insert (scan 0x52, 03B8) and scan 0x9B
   (03BF) are turned into 0x10 = left button. The Enter/'+' -> right, Space/0x10 -> left mapping in 3603 is
   skipped when word [00BA] != 0 (361B jumps to 36E4): the key is then returned raw (still closes the box, key in
   [C07F]). Keyboard cursor movement comes from the flags in [00A2] (3649..36E0), not from 03AA.
3. **8C58 / 899C character test is signed** (8C75 `cmp byte [si],0x20; jl`, 8A5D same, widths indexed after
   `cbw`): bytes 0x80..0xFF add no width and draw nothing. The pseudo-C `(unsigned char)*s >= 0x20` should read
   `(signed char)*s >= 0x20`.
4. **Section 4 omits a data effect** (DECODED, OVL_BATTLE E575..E587): before 35F9 the report adds the Rome
   casualties ([bp-0E], drawn by 8AC8 at x 0x78) as a dword to DS:C086/C088 and the enemy casualties ([bp-10],
   drawn by 8A9E at x 0xC4) to DS:C082/C084. A port that replaces E48B..E58B with battle_report_draw must keep
   these two running totals.
5. Minor wording: in "8A9E / 8AC8 numbers(lo, hi, x, y)" the parameter list is not the push order; the code
   (8AAC/8AAF: A022 gets [bp+8] as the low word, [bp+0A] as the high word) agrees with the sentence after it:
   push hi, lo, x, y.

## Port status (2026-11-09, port/src/ui.c)

Ported literally: `ui_text` / `ui_text_attr` = 0x899C ('\n' -> x back, y += 11; chars < 0x20 and 0x80..0xFF skipped;
`ui_text_attr` also has the opaque band mode of word [9B69] bit 15), `ui_text_advance` = 0x8C58, `ui_text_block` = 0x8BE4,
`ui_text_rect` = 0x8C9F (C division toward 0), `ui_box` = 0x3134 (fill -1 = none; `ui_frame` = fill 26),
`ui_text_box(fb, s, minw, lines0)` = the shared body of 59D4 and the report (rect at (159, 99) grown by 7, box, 8BE4 black at
((X0+X1)/2, Y0+7)), `ui_message_draw` = the drawing of 0x59D4 (click line `ui_click_text()` = the string at DS:[3E40],
read through `W(0x3E40)` after `ui_mouse_init()`, the resident overlay's start-up branch above; the port always has a mouse,
so a static image (3E16 "Hit any key to continue") reads 3E2E "Click to continue" like the run-time one; `ui_mouse_init` is
idempotent and may also be called once by main.c after state_init) in white via 8BE4 at (159, Y1-15); it returns the box so
the caller can do 691B/6973 with `ui_copy_rect`. `ui_text_block_attr` = 8BE4 with the whole word [9B69] (bit 15 band), as
899C gets it. `ui_text_width` is unchanged (= 8C58 - 1, no '\n' stop); `ui_text_centered` is NOT 8BE4 (see MENU_NOTES port
status).

Checks with the port's ui.c (harness in the scratchpad `wf2\ui\harness`, PNG decoded by its own reader, RGB compared against
CMAP with tolerance 5 as the verifier did): b1 box (82,76,236,122) 7,285 px / 0 differ, p8 (116,81,202,117) 3,219 / 0, s08
(116,87,202,111) 2,175 / 0 (1 px shifts give 375..1,705). `ui_text_box(report, 8C58("an"), 0)` + the four numbers over
scr_battle_report.bin: box (22,32,296,166), 0 px differ on the whole screen. Not yet used by battle_ui.c (msg_hook / text_box /
battle_report_draw are still the approximate versions; that file is not owned here).
