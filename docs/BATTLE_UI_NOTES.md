# Land battle: the orders-mode screen (D0F2 .. DC72)

Scope: what the orders mode draws and where, in `disasm/OVL_BATTLE.asm` (addresses = 0823 offsets), plus the EXE
helpers it calls (EXE code segment 0823, `disasm/EXE_RUNTIME.bin`). Logic of the orders themselves is in
BATTLE_NOTES.md section 5; this file is the drawing/paging spec.

Marks: **VERIFIED** = read in code and checked pixel by pixel against a capture with a python script;
**DECODED** = read in code only (no capture shows it); **GUESS** = inferred.

Captures used: `C:\DOSBox-X\hunt\mem_w00N_*.bin` (640K; DS at linear 0x16860, work buffer seg 2687 = linear 0x26870,
clean battle field at 0x39420). w003/w005 = at D14A (orders mode about to open), w004/w006 = at D170 (orders loop
just left, before the closing restore). Session 1 (w003->w004): Erik dragged units 1, 2, 3 and the general 10
(their +4 became FFFF, +6 = CA / C2 / C2 / 84). Session 2 (w005->w006): a global order, no unit clicked.

## 0. Drawing targets ("pages") - VERIFIED (code + snapshots)

DS descriptors, 9 words: {AAD0, 0, segment, 320, 200, clip x0 0, y0 0, x1 319, y1 199}:
- DS:9AB6 = VGA screen (seg A000); DS:C1DA = work buffer (seg 2687, linear 0x26870); DS:C0D0 = current target
  (copied from one of them). [9B26] = video mode (2 = VGA in the snapshots; the EGA path is mode 6).
- [4F96] = 1 while drawing into the work buffer (normal battle), 0 while drawing straight onto the screen.

| EXE | does (VGA path) |
|---|---|
| 6887 | flip: `[4F96] ^= 1`; target = work (C0D0 = C1DA); `0494(src=[C1DE]=2687, dst=[9ABA]=A000)` = copy the whole work buffer to the screen (rep movsw 0x7D00). Returns [4F96]!=0. |
| 6837 | "draw on the screen": `old = ([4F96]!=0); if ([4F96]==0) 6887(); [4F96]=0; C0D0 = 9AB6 (screen)`; returns old. |
| 691B(x1,y1,x2,y2) | cursor push/hide (1D55(0)); `950E(x1,y1,&rect, dst=C1DA, src=9AB6)` = copy rect **screen -> work buffer** (save); 1D8A. |
| 6973(x1,y1,x2,y2) | same, **work buffer -> screen** (restore). |
| 950E(x,y,&rect,dst,src) | rect blit between two descriptors (VGA: row memcpy). |
| BB7E (overlay) | `8790(C0A6 + 8*[A926], 56FE, C0D0)` = blit the clean field image onto the current target. |
| BA72 (overlay) | draw all [A928] unit sprites (records AA02..) onto the current target (8E62). |

So in orders mode **everything is drawn directly on the VGA screen**, and the work buffer is used as the backup of
the clean picture (field + sprites): `691B(0,0,319,199)` saves it, `6973(0,0,319,199)` erases any overlay.

Evidence: w003 vs w004 work buffers differ in 3617 pixels, all inside the button strip (x 124..317, y 2..20), 0
elsewhere; the w004 buttons match the rendering below exactly. w005 vs w006: work buffers identical (0 px) because
session 2 never called 691B with the buttons on screen. At w004 C0D0 = 9AB6 (target = screen), [4F96] = 0.

## 1. D0F2 - per-tick trigger, opening and closing (DECODED; paging VERIFIED as above)

```c
void D0F2(void) {
  int trig = 0, done = 0; uint8_t k;
  if ([BA]) { 0692(0); if ([B4]) trig = 1; 0692(1); if ([B4]) trig = 1; }   /* button-press counts */
  k = 03AA(); if (k) { if (k==' '||k==0x10||k=='+'||k=='\r') trig = 1; [C07F] = k; }
  if (trig) {
    04ED();                         /* flush keyboard */
    6837();                         /* target = screen (work buffer shown first if needed) */
    BB7E();                         /* clean field -> screen */
    BA72();                         /* unit sprites -> screen */
    691B(0,0,319,199);              /* backup: screen -> work buffer */
    while (!done) done = D198();    /* 0 = stay in the menu */
    6973(0,0,319,199);              /* restore: work -> screen (erases buttons/paths/panels) */
    6887();                         /* [4F96]=1, target = work, work -> screen */
    0692(1); 0692(0);               /* clear press counters */
  }
  1D19();                           /* hide the mouse cursor (every tick) */
}
```
Quirk (VERIFIED by w003/w004): D9EF and D33A call `691B(0,0,319,199)` while the 4 buttons are on screen, so from
then on the backup contains the buttons; the closing restore and flip therefore leave the buttons visible until the
next battle tick redraws the work buffer (BB7E+BA72+flip, 0.165 s). Harmless; a port can simply redraw.

## 2. D198 - the menu loop (DECODED)

```c
int D198(void) {
  int got = 0, ret = 0;
  0644();                               /* poll mouse */
  6973(0,0,319,199);                    /* erase everything drawn since the backup */
  D92D();                               /* the 4 buttons */
  1CCE();                               /* show cursor */
  while (!got) {
    if (359B() != 0) { [C07F] = 0; got = 1; }          /* mouse click (1 left / 2 right) */
    else if ([C07F] == 0x1B) { [C07F] = 0; return -1; } /* Esc */
    else { [C07F] = toupper([C07F]); if (strchr("MRDS", [C07F])) got = 1; }
  }
  switch (D978()) {
    case 0: D9EF(); break;                               /* S / Select Roman Unit: show all orders, stay */
    case 1: ret = -1; break;                             /* D / Done */
    case 2: msg "Release The Legion!" + global melee; ret = -1; break;  (d21e, see BATTLE_NOTES 5)
    case 3: msg "Run Away!\nRun Away!" + global retreat; ret = -1; break; (d299)
    case 4: D33A(); break;                               /* click on the field (return value ignored) */
  }
  35DB();                                                /* wait until the buttons are released */
  return ret;                                            /* returns di: 0 or -1 */
}
```
Messages: 9E32/9DF2 build `"Release"` (DS:7468 via [74B6]) + `" The Legion!"` (DS:7470 via [74B8]) in DS:AC60 and
show it with 59D4; retreat shows DS:74D4 `"Run Away!\nRun Away!"`.

## 3. D92D - the 4 buttons - **VERIFIED pixel-exact**

```c
void D92D(void) {
  1D55(0);                         /* hide cursor */
  [9B69] = 15;                     /* text colour: white */
  for (i = 0; i < 4; i++) {
    r = (int16_t*)(DS + 0x748E + 8*i);           /* x1, y1, x2, y2 (inclusive) */
    3134(r[0], r[1], r[2], r[3], [6346]);        /* [6346] = 26: bevelled box with fill 26 */
    899C(*(char**)(DS + 0x74AE + 2*i), r[0] + 5, r[1] + 5);   /* left-aligned, NOT centred */
  }
  1D8A();
}
```
| i | label (ptr at 74AE) | box x1,y1 - x2,y2 | text origin | hotkey |
|---|---|---|---|---|
| 0 | "Select Roman Unit" (DS:7443) | 124,2 - 209,20 | 129,7 | S |
| 1 | "Done" (DS:7455) | 289,2 - 317,20 | 294,7 | D |
| 2 | "Melee" (DS:745A) | 211,2 - 244,20 | 216,7 | M |
| 3 | "Retreat" (DS:7460) | 246,2 - 287,20 | 251,7 | R |

On screen left to right: Select Roman Unit, Melee, Retreat, Done (rects are stored in index order above).
- 3134(x1,y1,x2,y2,fill) == `ui_frame(fb, x1, y1, x2-x1+1, y2-y1+1)` (fill 26, top 2 rows 15, bottom 2 rows 0,
  sides 8, inner line of 8 three pixels in).
- 899C(str,x,y): per char `c`: glyph rows drawn at (x, y + yoff[c] + r), only ink pixels (background untouched),
  colour = [9B69] & 15; x += width[c] + [9B5E](=1); '\n' -> x = start, y += [9B5C]+[9B5D] = 8+3 = 11. Font tables
  DS:9C9C/9CFC/9D5C (pointers [9B56]/[9B58]/[9B5A]). This is exactly `ui_text(fb, x, y, s, colour)`.
  If ([9B6A] & 0x80) it first fills a background rect in colour ([9B69]>>4)&15 (not used here: [9B69] = 0x000F).
- Verification: rendered the four boxes with the ui_frame algorithm and the labels with the OVL0 font (colour 15) onto
  the clean field (linear 0x39420) and compared with the work buffer of mem_w004_D170 over x 120..319, y 0..22:
  4600 pixels, **0 mismatches**. (Script: build `img = clean field`; for i in 0..3 draw box + label as above; compare.)
- Palette (CMAP.BIN, 8-bit): 0 = 0,0,0; 8 = 84,84,84; 15 = 252,252,252; 26 = 160,144,128.

## 4. D978 - key / click to button (DECODED)

```c
int D978(void) {
  switch (toupper([C07F])) { case 'S': return 0; case 'D': return 1; case 'M': return 2; case 'R': return 3; }
  if ([BA] && [B2] == 0) return 4;                   /* mouse present, no button down -> "field" */
  for (i = 0; i < 4; i++)                             /* with no mouse ([BA]=0) this tests the stale [B6],[B8] */
    if (8238([B6], [B8], DS + 0x748E + 8*i)) return i; /* x1<=x<=x2 && y1<=y<=y2, inclusive (EXE 8238) */
  return 4;
}
```

## 5. D9EF - show every Roman unit's orders (DECODED)

```c
691B(0,0,319,199);                          /* backup = field + sprites + buttons (see quirk) */
for (i = 1; i < [C294]; i++) if (U[i].side == 0) DA35(i);   /* DA35 skips state 4 */
35F9();                                     /* wait release, wait click or key, wait release */
6973(0,0,319,199);                          /* erase the paths */
```
The buttons stay on screen while the paths are shown; D198 then redraws them anyway.

## 6. DA35(i) - a unit's order path (DECODED, not in any capture)

Colour [9AFA] = **1** (0,0,168 dark blue); every plotted point is a **3x3 dot** (EXE 0524), so the lines are 3 px thick.
```c
void DA35(int i) {
  U *u = &U[i];
  if (u->state == 4) return;
  [9AFA] = 1; 1D55(0);
  type = u->type; fac = u->facing (+0x0E);
  P[0..3] = {-1,-1};                         /* screen points, x and y words */
  cell = {u->x, u->y};                       /* s8, s8 */
  P[0] = DBEB(cell, fac, type);
  for (k = 0; k < 3; k++) {
    o = u->ord[k];                           /* +6, +7, +8 */
    if (o == 0x80) { dot0524(P[0].x, P[0].y); continue; }   /* always at P[0]; P[k+1] stays -1 */
    fac = o >> 6 & 3; n = o & 0x3F;          /* the leg direction is used as the facing from here on */
    switch (fac) { case 0: cell.x += n; break; case 1: cell.y += n; break;
                   case 2: cell.x -= n; break; case 3: cell.y -= n; break; }   /* byte arithmetic */
    P[k+1] = DBEB(cell, fac, type);
  }
  for (k = 1; k <= 3; k++)
    if (P[k].x != -1 && P[k].y != -1 && P[k-1].x != -1 && P[k-1].y != -1)
      line97B6(P[k-1].x, P[k-1].y, P[k].x, P[k].y, dot0524);
  if (type == 3) DB8A(i);                    /* the general: voice circle */
  1D8A();
}
```
Notes: a 0x80 leg does not stop the walk (later legs still move from the unchanged cell; their first line is skipped
because P[k] = -1). A leg 0x00 (dir 0, length 0) is a real point (footprint re-centred with facing 0).
Port: battle_ui.c draw_path is now DA35 literally (see "Port status" below).

### 6.1 DBEB(&out, &cell, facing, type) - footprint centre on screen (DECODED)
```c
a = DS[6AE8 + type]; b = DS[6AEC + type];          /* {2,2,2,1}, {2,3,2,2} */
if (facing == 0 || facing == 2) swap(a, b);
p1 = C7B0(cell); p2 = C7B0((s8)(cell.x + a), (s8)(cell.y + b));
out.x = (p1.x + p2.x) / 2;          /* C division (truncates toward 0) */
out.y = (p1.y + p2.y - 8) / 2;      /* 4 px above the footprint's middle */
```
C7B0 (grid -> screen): `sx = (x - y)*8 + 0xA4; sy = (x + y)*4 - 0x4A` (x, y signed bytes).
C752 (screen -> grid, used for clicks): `a = 2*(my-2) + (mx-4); gx = a/16 (trunc); b = 2*(my-2) - (mx-4);
if (b < 0) b -= 15; gy = b/16 (trunc) + 0x14` (the port's screen_to_grid matches).

### 6.2 EXE 0524(x, y) - 3x3 dot (DECODED)
`for (yy = y-1; yy <= y+1; yy++) hspan7ECA(x-1, yy, x+1);` in colour [9AFA], clipped to the target's clip rect
(0..319, 0..199).

### 6.3 EXE 97B6(x1, y1, x2, y2, plot) - line (DECODED)
Bresenham with the callback; both end points plotted (n+1 points):
```c
sx = sign(x2-x1); dx = |x2-x1|; sy = sign(y2-y1); dy = |y2-y1|; x = x1; y = y1;
if (dx == 0)      { for (c = dy; c; c--) { plot(x,y); y += sy; } }
else if (dy == 0) { for (c = dx; c; c--) { plot(x,y); x += sx; } }
else if (dx > dy) { i1 = 2*dy; e = 2*dy - dx; i2 = 2*dy - 2*dx;
                    for (c = dx; c; c--) { plot(x,y); x += sx; if (e < 0) e += i1; else { y += sy; e += i2; } } }
else              { i1 = 2*dx; e = 2*dx - dy; i2 = 2*dx - 2*dy;           /* dx == dy lands here */
                    for (c = dy; c; c--) { plot(x,y); y += sy; if (e < 0) e += i1; else { x += sx; e += i2; } } }
plot(x, y);
```

## 7. DB8A / DBBF - the general's voice circle (DECODED)

`DB8A(i)`: `98BC(cx = U[i].x, cy = U[i].y, r = cmdr[C28C].voice (DS:1A1F + 6*[C28C]), cb = DBBF)`.
The circle is computed **in grid cells**, and each cell is mapped to the screen: `DBBF(x, y)`:
`(sx, sy) = C7B0((s8)x, (s8)y); dot0524(sx, sy)` - colour still 1 from DA35. No footprint offset, no -8.
So it is a dotted isometric ellipse (one 3x3 dot per cell on the circle; dots 16 px apart horizontally at most).
Drawn only when DA35 runs for the general (D9EF, or the general selected in D33A).

EXE 98BC -> 98DA(cx, cy, r, octant=993E):
```c
x = 0; y = r; d = 3 - 2*r;
do {
  octant(x, y);
  if (d < 0) { d += 4*x + 5; x++; }          /* note 5, not the textbook 6 (bytes 83 C2 05 at EXE 9907) */
  else       { d += 4*(x - y) + 10; y--; x++; }
} while (x < y);
if (x == y) octant(x, y);
```
993E(a, b): collects points, then calls cb on them in **reverse** order of collection:
```c
for (pass = 0; pass < 2; pass++) {
  add(cx+a, cy+b);
  if (b) { add(cx+a, cy-b); if (a) { add(cx-a, cy-b); add(cx-a, cy+b); } }
  else if (a) add(cx-a, cy-b);
  swap(a, b);
}
for (k = n-1; k >= 0; k--) cb(px[k], py[k]);  /* duplicates when a == b are plotted twice (same result) */
```
(Order only matters for overdraw, which is a single colour here.) r = 5 gives 28 distinct cells.

## 8. DC72(i) - unit info panel (DECODED, not in any capture)

Common: `(sx, sy) = C7B0(U[i].x, U[i].y)`; side of the screen chosen by sx: **sx < 0x9F (159) -> panel on the right,
else on the left.** Text colour [9B69] set per line; box fill [6346] = 26; boxes via 3134 (= ui_frame, sec. 3).
Returns at once if U[i].men == 0.

### 8.1 Cohorts / enemy units (type 0..2)
```c
x1 = (sx < 159) ? 247 : 2;  y1 = 24;  x2 = x1 + 70;  y2 = 64;     /* box 71 x 41: (2,24)-(72,64) or (247,24)-(317,64) */
3134(x1, 24, x2, 64, 26);
buf = Roman(i) /* 6738, of the UNIT INDEX */ + (U[i].side == 1 ? " Unit" /* DS:751A */ : " Cohort" /* [74BA] -> DS:747D */);
[9B69] = 15; 899C(buf, x1+5, 29);                                    /* line 1 */
buf = itoa(U[i].men, 10) + type_name[type];                           /* 753C: " Cavalry", " Elephants", " Infantry" */
899C(buf, x1+5, 40);                                                  /* line 2, colour still 15, e.g. "500 Infantry" */
if (U[i].state == 3) { [9B69] = 4; 899C("Panicked" /* DS:7520 */, x1+5, 51); }
else { m = (s8)U[i].morale / 25;   /* signed idiv, truncates */
       [9B69] = DS16[7544 + 2*m]; 899C(DS16[7486 + 2*m], x1+5, 51); }
```
Line y = 29 + 11*k (k = 0,1,2), text x = x1 + 5 (7 or 252).
| morale | m | word | colour (RGB) |
|---|---|---|---|
| 0..24 | 0 | "Panicky" (DS:742A) | 4 (168,0,0) |
| 25..49 | 1 | "Weak" (DS:7432) | 12 (252,84,84) |
| 50..74 | 2 | "Good" (DS:7437) | 15 (252,252,252) |
| 75..99 | 3 | "Fierce" (DS:743C) | 9 (84,84,252) |
Morale >= 100 would read past both tables (7544[4] = 0x7453 = the "Standard" string, 7486[4] = 0x007C); morale < 0
reads before them. Snapshot values are 99 (Romans) and 80 (enemy); GUESS: morale never leaves 0..99 here.
The raw +0x0C is shown (not B7BE's charisma-boosted value).

### 8.2 Generals (type 3)
```c
if (U[i].side == 1) buf = "General";                                  /* [7542] -> DS:7512 */
else buf = cmdr.name /* [1A1C+6k] */ + "\nCharisma:" /* DS:7529 */ + itoa(cmdr.charisma /* 1A1E+6k */)
           + "\nVoice:" /* DS:7534 */ + itoa(cmdr.voice /* 1A1F+6k */);   /* k = [C28C]; no space after ':' */
/* 8C9F(&r, buf, cx=0, cy=0): W = max over lines of 8C58(line) (= sum(width+1), i.e. ui_text_width+1),
   L = number of lines; r = {-(W/2), -(L*11/2), W/2, L*11/2} */
w = 2*(W/2) + 14;  h = 2*((L*11)/2) + 14;
if (sx < 159) { x2 = 317; x1 = 317 - w; } else { x1 = 2; x2 = w + 2; }
y1 = 24; y2 = 24 + h;                                                 /* 3 lines: h = 46; "General": h = 24 */
3134(x1, 24, x2, y2, 26);
[9B69] = 15; 8BE4(buf, (x1 + x2)/2, 31);
```
8BE4(str, cx, y) (centred text): for each '\n'-separated line: `899C(line, cx - (8C58(line) >> 1), y); y += 11`.
Note 8C58 includes the trailing 1-px gap, so it is `cx - (ui_text_width(line)+1)/2`, not the port's
`ui_text_centered` (`cx - ui_text_width/2`): they differ by one pixel when the width is odd.
Example (w004: Scipio Africanus, charisma 10, voice 18, general at grid 28,31 -> sx 140 -> right side):
widths 70/53/36, box (233,24)-(317,70), centre 275, lines at x 240 / 249 / 257, y 31 / 42 / 53.

Port: battle_ui.c draw_panel is now DC72 literally (see "Port status" below).

## 9. D33A - a click on the field: what is drawn when (DECODED)

```c
cell = C752([B6],[B8]); t = grid[cell.x*48 + cell.y]; if (!t) t = D2DD(&cell);   /* 12 neighbours DS:74BC, no bounds check */
if (t == 0 || t == 0xFF || t == 0xFE) return;
if (U[t].side != 0) { DC72(t); wait(); return; }       /* enemy: panel on top of the buttons, no backup taken;
                                                          wait = 35DB (mouse) or 358A -> [C07F] (no mouse) */
691B(0,0,319,199);                                      /* backup WITH the buttons */
DC72(t); DA35(t); inv = D8AD(t); 35DB();                /* panel + path (+ circle if general) */
if (U[t].state == 3 || !inv) { wait(); 6973(0,0,319,199); return; }
/* mouse: wait for the next click/key (as D198). Esc -> return. Click on the same unit (cell or D2DD) with no key:
   drag loop while [B2]: 0644; 1BE6(x,y) (move cursor); cell = C752; if cell changed:
       6973(full)  -> panel disappears, buttons stay (they are in the backup)
       U[t].ord = {CBEE(t, cell), 0x80, 0x80}; DA35(t);       (path only)
   after release: if changed { +4 = FFFF; state = 1 }  6973(full); return.
   Anything else (other cell, button, key): 6973(full); then D978(): 2 -> D7E8(t, keep) "Release Cohort <n>",
   3 -> D77C(t, keep) "Run Away!", others -> just return.
   keyboard (no mouse): every 9 timer ticks a target-cursor step -> 6973(full), CBEE, DA35; Space -> ord = 80 80 80,
   6973, DA35 (dot only); 'M'/'R' -> 6973 then D7E8/D77C; Enter/Esc/'+'/Insert -> end. */
```
D7E8 message: `"Release"` + `" Cohort"` ([74BA]) + `" "` (DS:74F2) + Roman(t) -> 59D4; the general (type 3) gets no
message and no order. D77C message: `"Run Away!"` (DS:74E8).
After D33A returns, D198 returns 0 and D0F2 calls D198 again: 6973 (erase) + buttons.

## 10. Strings and tables used (DS, values read from the w004 snapshot)

748E rects (sec. 3); 74AE label ptrs 7443/7455/745A/7460; 74B6 -> "Release", 74B8 -> " The Legion!", 74BA -> " Cohort";
74BC D2DD offsets (dx,dy): (1,1)(0,1)(-1,1)(-1,0)(-1,-1)(0,-1)(1,-1)(1,0)(2,1)(1,2)(0,2)(2,0);
74D4 "Run Away!\nRun Away!"; 74E8 "Run Away!"; 74F2 " "; 753C type names ptrs 74F4 " Cavalry" / 74FD " Elephants" /
7508 " Infantry" / 7512 "General"; 751A " Unit"; 7520 "Panicked"; 7529 "\nCharisma:"; 7534 "\nVoice:";
7542 -> "General"; 7486 morale words; 7544 morale colours {4,12,15,9}; 6346 = 26 (box fill); 6AE8/6AEC footprints.
Commander record DS:1A1C + 6k: +0 name ptr, +2 charisma, +3 voice, +4 status (D8AD requires 0).

## 11. Test vectors from mem_w004_D170 (computed with the decoded DA35/DBEB, DECODED - no capture of paths)

| unit | grid x,y | type, facing | +6 | P0 | P1 |
|---|---|---|---|---|---|
| 1 | 22,28 | 2, 3 | CA (y-10) | 116,130 | 196,90 |
| 2 | 24,28 | 2, 3 | C2 (y-2) | 132,138 | 148,130 |
| 3 | 20,28 | 2, 3 | C2 | 100,122 | 116,114 |
| 4 | 26,28 | 2, 3 | 80 | 148,146 (dot) | - |
| 10 (general) | 28,31 | 3, 3 | 84 (x-4) | 136,164 | 112,148 |
General voice 18: circle around grid (28,31), dots at C7B0 of each circle cell (most fall off screen at the bottom).

## Port status (port/src/battle_ui.c, 2026-11-09)

Translated from the code (each function carries its address): D92D draw_buttons, DA35 draw_path (+ DBEB, EXE 0524 dot,
97B6 line), DB8A/DBBF + EXE 98BC/98DA/993E voice circle, DC72 draw_panel, D978, D9EF, D33A, D2DD, D198, D0F2's
triggered branch (orders_mode), EXE 59D4 message box, E48B..E58B report, with local copies of 3134 / 899C / 8BE4 / 8C58 /
8C9F / 8D56 (no dependence on ui.c for these). The screen and the work buffer are two buffers (g_fb / g_work), 691B / 6973
copy rects between them, so what is drawn / erased when follows the code (panel gone on the first drag move, enemy panel
only while the button is held, Select = every Roman path + the general's circle, buttons kept in the backup).
Input: the mouse paths ([00BA] != 0) only, since the port always has a mouse; the keyboard-only paths of D33A (target
cursor every 9 timer ticks, Space = hold) are not ported.
D0F2's trigger (tick_hook): a left OR right press (0692(0), 0692(1) = INT 33h fn 5 press counts; the port: a button bit
new since the last poll, bit 1 left / bit 4 right) or 03AA's key in {Space, 10h, '+', Enter}. Not exact: Insert (03AA
turns scan 52h into 10h) cannot reach the port, because sdl2_platform.c only records keysyms < 256; and SDL gives the
keysym of the unshifted key with no modifier state, so the US '+' (Shift + '=') arrives as '=': '=' is accepted for it,
which also lets a plain '=' open the orders mode (the original does not).
DS values the start-up sets (a DS built by state_init from the static OVL0 image has the old ones): the click line
[3E40] (static 3E16 "Hit any key to continue", with a mouse 3E2E "Click to continue") and the box fill [6346] (static 7,
1Ah from the resident overlay's VGA branch 0123..014C). battle_ui.c uses 3E2E and 1Ah directly (the port is always
VGA with a mouse), so a battle from a static-image DS draws the same boxes as one from a DOSBox snapshot (harness:
59D4 "Gaul" from both DSes, 0 px differ).

Checks (scratch harness that includes battle_ui.c):
- D92D over the w003 work buffer vs the w004 work buffer: **0 differing pixels** (whole screen). VERIFIED.
- 59D4 drawn by the port for "Gaul" / "" / "Albertus Discreditus is assigned\nto command Rome" vs p8.png / s08.png /
  b1.png: 0 mismatches over 3219 / 2175 / 7285 box pixels. VERIFIED. Box rects of all rows of TEXTBOX_NOTES sec. 3 match.
- Report (now the general 8C9F/8BE4 rule): `--report-shot` 0 px against scr_battle_report.bin. VERIFIED.
- DA35/DBEB on the w004 units: every vector of section 11 reproduced; the general's panel (Scipio) box (233,24)-(317,70).
  DECODED (no capture shows paths, panels or the circle).
- Scripted D198 -> D33A session on w003 (fake platform): click cohort 1 = panel + dot, second click + drag = panel
  erased and path drawn, release = +4 FFFF / state 1 and everything erased, 'D' = done.
- Re-run after the reviewer fixes (2026-11-09, harness including battle_ui.c): buttons 0 px; section 11 vectors all
  reproduced; Scipio panel box (233,24)-(317,70); voice circle r 5 = 28 dots; 59D4 boxes "Gaul" (116,81,202,117),
  "Run Away!\nRun Away!" (116,76,202,122), "" (116,87,202,111), "Release The Legion!" (109,81,209,117) = the
  TEXTBOX_NOTES table, screen fully restored after each; report boxes for n 2 / 3 equal the closed form; headless
  --battle-shots of b11 identical (10 frames, 0 px) to the version before the fixes.

## Corrections (verifier)

Independent re-check (2026-11-09) with a fresh llvm-objdump listing of OVL0.OVL section 2 (D0F2 is mis-synced in
disasm/OVL_BATTLE.asm: it decodes from D0F1 as `add [di-0x75],dl`; a listing started at D0F2 gives `push bp; mov bp,sp`)
and my own python renderer (3134 decoded from EXE 3134..3230, glyphs from DS:[9B56]/[9B58]/[9B5A], height [9B5C]=8,
1 byte/row since [9B5F]=0, MSB = leftmost). Everything in sections 0-11 that I checked holds; only small additions:

1. **Buttons re-verified, whole screen.** Starting from the w003 work buffer (field + sprites before session 1) and
   drawing the 4 boxes + labels gives the w004 work buffer with **0 mismatches over all 64000 px** (not only the strip);
   w003 vs w004 differ in 3617 px, bbox x 124..317, y 2..20. 3134 pixel order (fill 26 via 7E74; colour 8 verticals
   x1/x2 full height, x1+1/x2-1 at y1+1..y2-1, inner rect at 3 px; colour 0 rows y2 (x1..x2) and y2-1 (x1+1..x2-1);
   colour 15 rows y1 (x1..x2) and y1+1 (x1+1..x2-1)) equals port ui_frame(x1,y1,x2-x1+1,y2-y1+1) on 300 random boxes.
   Also: the w005 work buffer differs from w004 in 3609 px of the strip (buttons gone by the next tick), supporting the
   section 1 quirk.
2. **"899C is exactly ui_text" holds for single-line strings only.** 899C handles '\n' (x = start x, y += [9B5C]+[9B5D]
   = 11, EXE 8A4A-8A5B); port ui.c ui_text has no '\n' case (font_glyph returns NULL for c < 0x20, so it is skipped
   without a line feed). No impact on this screen (all 899C calls here are single-line; 8BE4 splits lines itself), but
   a port must not pass "\n" strings to ui_text expecting 899C behaviour.
3. **(RESOLVED, 2026-11-09)** ~~Port draw_path difference, more precisely: battle_ui.c:179 stops at `o == 0x80 ||
   !(o & 0x3F)`, i.e. also at zero-length legs (0x00/0x40/0x80/0xC0 all stop it), whereas DA35 treats only 0x80
   specially and keeps walking.~~ That code is gone: draw_path is now DA35 literally (only 0x80 is special, the walk
   goes on, a 0x00 leg is a real point; section 6 and "Port status").
4. **D33A return value**: the early exits (t == 0/FF/FE, D379/D388 -> D773) return the uninitialised local [bp-6];
   harmless because D198 (D212) ignores it, as the notes say.
5. **Video modes** (sec. 0, minor): 6887 switches on [9B26] = 2 (VGA: copy C1DA -> C0D0, 0494 work->screen),
   4 (84BB + 0388 page select, C0D0 = C1DA or 9AB6 by [4F96]) and 6 (port 3DF write E6/F4). 6837/691B/6973 only
   special-case 6. Which adapter is 4 vs 6 is not established here (GUESS in the notes).

Checked and confirmed (DECODED): D0F2 sequence and key set {20,10,2B,0D}; D198 (Esc returns -1 without 35DB);
D978 key map S/D/M/R -> 0/1/2/3 and `[BA] && [B2]==0 -> 4`; D9EF; 691B = screen->work and 6973 = work->screen
(950E: [bp+0A] = destination descriptor, [bp+0C] = source); 0494 = work->screen (ES = [bp+4] = A000);
DA35 incl. the 4-point array, 0x80 handling and the persistent facing; DBEB tables {2,2,2,1}/{2,3,2,2}, swap for
facing 0/2, truncating /2 and the -8; C7B0; C752; 0524 3x3 dot; 97B6 (dx == dy takes the y-major branch, e == 0
steps the minor axis); 98DA (d += 4x+5 / 4(x-y)+10) and 993E (r = 5 -> 28 cells, r = 18 -> 104); DC72 box
(2|247, 24)-(+70, 64), lines y 29/40/51, morale idiv 25 with colours {4,12,15,9}; general box via 8C9F (+-7) and
8BE4 at y 31; Scipio example widths 70/53/36 -> box (233,24)-(317,70), line x 240/249/257; all section 11 vectors
(recomputed from the w004 unit table at DS:D84A, 24 units). Palette 1/4/8/9/12/15/26 confirmed from CMAP.BIN.
No capture shows paths, panels or the voice circle (hunt/shots/*.png checked), so those stay DECODED.
