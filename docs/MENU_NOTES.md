# Menu engine (EXE 0x5614 draw, 0x54C8 run)

Written 2026-11-09. All addresses are 0823 code offsets (EXE) or DS (1686) offsets. "Pascal" = callee pops; the first pushed
argument is the leftmost one below.

Labels: **VERIFIED** = read in the code and reproduced pixel by pixel against a capture with a Python renderer;
**DECODED** = read in the code only; **GUESS** = inferred.

## 0. Short answer to the known port gaps

| Gap in `ui_menu_draw` | Cause in the original | Status |
|---|---|---|
| Item text starts 11 px too far left ('Balanced Army' ink should span x 19..81) | 0x5614 draws each item at `X0 + item.x0 + 11`, `Y0 + item.y0`. Same font, same 1 px spacing: drawn at x 8 it spans 8..70, which is where the port's right edge of 70 comes from. **There is only one font.** The font variables DS:9B56..9B5F are identical in all 33 memory snapshots. DS:9C8C..9C9B is a per-video-mode jump table for line drawing, not a font directory (it corrects FONT_FORMAT.md). | VERIFIED |
| Title 2 px too high | 0x56B6 draws the title at `y = Y0 + 6` (port: +4), centred on `cx = (X0+X1)/2` with `x = cx - (W >> 1)`, where W is the 0x8C58 width that **includes the trailing 1 px spacing** (Σ(width+1)). | VERIFIED |
| Missing column ornament | 0x56B6, colour 8: cap row `x sx+1..sx+7` at the first item's top, three vertical lines at `sx+2, sx+4, sx+6`, base row at the last item's bottom; `sx = X0 + item0.x0`. | VERIFIED |
| Highlight | The port inverts 0↔26 under the hovered item. The original has **no hover highlight** when a mouse is present. 0x585B only redraws the chosen item's text in colour 15 just before 0x54C8 returns. | DECODED |

Verification result: the Python renderer (section 9) reproduces the **whole box** of `scr_battle_formation.bin` (record 73B6, box
(2,2)-(106,82), 8,505 px) and `scr_game_control_menu.bin` (record 05A6, box (81,79)-(251,197), 20,349 px) with **0 differing
pixels**. Overlaying items, ornament and title (mode 0) on `scr_rome_province.bin` (record **5A30**) and
`scr_narbonensis_province.bin` (record 5790) changes **0 pixels**. That covers 1,143 and 1,088 drawn pixels respectively; the
province frame is the mode 1 sprite frame and is not reproduced (section 3.2).

## 1. Record formats (VERIFIED)

```
menu record (18 bytes, DS):
 +00 s16 X0, +02 s16 Y0, +04 s16 X1, +06 s16 Y1   box, inclusive screen coords
 +08 u16 colour     item text colour (0 in every record); the title uses colour ^ 0x0F (=15). Word written to DS:9B69 (see 6.1)
 +0A u16 items      DS pointer to item[count]
 +0C u16 title      DS pointer to a NUL-terminated string (LF allowed); 0 / "" = no title
 +0E u8  count
 +0F u8  single     !=0 (0xFF): one ornament spans all items; 0: one ornament per item (only 11CA OK/Cancel)
 +10 u16 enabled    0xFFFF = active (ornament + title drawn). 0x55EA sets it to FFFF, 0x55FF sets it to 0 (greyed)
item (12 bytes):
 +00 u16 text       DS string pointer (may contain LF); the game rewrites it at run time for the tactic menu (3E02 = "")
 +02 u8  hotkey     ASCII, compared after toupper()
 +03 u8  disabled   !=0: text drawn in colour 6 (0x5614/0x585B). Still selectable by click and hotkey (no check in 552B/5593)
 +04 s16 x0, y0, x1, y1   inclusive rect RELATIVE to (X0,Y0); used for the hit test and for text/ornament placement
```
All stock records use item rects `(6, 23+10i, 138, 33+10i)`, so items are 10 px apart. x1 = 138 is wider than the
104-px battle box, but the hit test checks the box first (552B).

### 1.1 Catalogue (static OVL0 data; a scan for the record shape)

| DS | box | title | items (hotkey) |
|---|---|---|---|
| 05A6 | 81,79-251,197 | Game Control Menu | Save Game... S, Load Game... L, New Game N, About Centurion... A, Difficulty Levels... D, Quit to DOS Q, Continue Playing C |
| 05B8 | same | Difficulty Level Menu | Galley Slave G, Legionnaire L, Senator S, Emperor E, Fine Tuning F, Continue C |
| 05CA | same | Fine Tuning Menu | Land Battle L, Racing Chariots R, Gladiatorial Combat G, Naval Battle N, Continue C |
| 05DC | same | (none) | Galley Slave, Legionnaire, Senator, Emperor, Continue |
| 11CA | 84,75-234,131 | (none) | OK O, Cancel C; **single = 0** (per-item ornaments) |
| 11DC | 6,79-156,197 | What type of ships? | Triremes:\n.. T, Quinqueremes:\n.. Q, Galleons:\n.. G, Continue C (two-line items) |
| 15B0 | 6,79-156,197 | Provincal Games [sic] | Bestiarii (15 Talents) B, Gladiator Show (30) G, Racing Chariots (40) R, Extravaganza (50) E, Continue C |
| 15C2 | 6,79-156,197 | What Kind of Games? | Racing Chariots R, Gladiator Show G, Continue C |
| 1B7E | 2,0-317,60 | (none) | "" 1, "" 2, "" 3; **enabled = 0** in static data (0x2CE1 enables it) |
| 1C0A | 6,79-156,197 | What is your command? | Raise Legion R, Strengthen Legion S, Move Legion M, Upgrade Legion U, Legion List L, Continue C |
| 1C1C | 6,79-156,197 | What type of Legion? | Infantry Legion: 20 I, Cavalry Legion: 30 C, Consular Army: 60 A, None N |
| 4080 | 8,151-103,197 | (none) | 3 run-time items, hotkey 0 |
| 5790 | 6,79-156,197 | What is your command? | Legions L, Plunder P, Tribute T, Hold Games H, Build Fleet B, See Province List S, Continue C |
| **5A30** | 6,79-156,197 | What is your command? | **Rome's menu**: Legions L, Tribute T, Hold Games H, Build Fleet B, Tribute Policy P, See Province List S, Continue C (VERIFIED on scr_rome_province: a separate record, not a run-time patch of 5790) |
| 5BCC | 30,40-290,159 | (none) | "1 -".."9 -" 1..9 |
| 6054 / 60AE | 6,79-156,197 | What level of tribute? / What tribute policy? | Exempt this province / Exempt all provinces E, Tolerable T, Irritating I, Oppressive O, Bleed Them Dry B, Continue C |
| 73B6 | 2,2-106,82 | Choose Formation | Balanced Army B, Wedge W, Strong Right R, Strong Left L |
| 73C8 | 2,2-106,82 | Choose Tactic | 4 run-time names 1..4, Stand Fast 5 |
| 7CB0 | 91,79-241,197 | How Much Training? | 4 run-time items 1..4 |
| 88E6 | 6,117-156,197 | Race what type of Chariot? | Light Chariot: 5 L, Medium Chariot: 10 M, Heavy Chariot: 20 H, Exit E |
| 88F8 / 890A / 891C | 6,79-156,197 | How Shall We Prepare? / What Will You Bet? / What Skulduggery? | Race Now R, Skulduggery S, Bet B, Exit E / Bet 10 More +, Bet 10 Less -, Bet The Limit L, Done D / Bribe Opponent: 10 B, Hire Physician: 30 H, Invoke Gods: 50 I, Done D |
| 9750 | 110,70-210,118 | Choose a flag ship: | 3 run-time items, hotkey 0 |

## 2. Globals used

| DS | meaning |
|---|---|
| 00BA | mouse present (FFFF in every snapshot). Chooses the mouse or keyboard path everywhere below |
| 00B2 / 00B6 / 00B8 | mouse buttons (bit0 left, bit1 right) / x / y from INT 33h fn 3 (0x0644). Ranges are set in screen pixels with fn 7/8 (0x06E4/0x0709, e.g. x ≤ 0x137, y ≤ 0xB9), so x,y are 320x200 coords (DECODED) |
| C07F | last key from 0x359B (0 after a mouse click) |
| 00A2 / A3 / A4 / A5 | INT 9 hook (0x03FF) direction state: A2 = held direction bits (8 up, 4 down, 2 left, 1 right), A3 = bits of the last key, A4 = BIOS shift flags 40:17, A5 = repeat counter (0 on the first make, FF after break). Keypad map DS:00A6 for scans 47..51: `0A 08 09 04 02 80 01 01 06 04 05` (Home=up+left, Up=8, PgUp=up+right, Left=2, 5=0x80, Right=1, End=down+left, Down=4, PgDn=down+right). Keypad - and + are ignored unless [9AB2]==6 (then ` and \ act as Up/Left) |
| 6346 | box fill colour (26 at run time) |
| 9AFA | current fill/line colour for 7E74 / 7ECA / 925C |
| 9B69 (word) | text colour: low nibble = ink, bits 4..7 = background, **byte 9B6A bit 7 = opaque** (fill the line's background first). Menus always write a word with high byte 0, so text is transparent |
| 9B56/58/5A | font: width table 9C9C, yoff table 9CFC, glyph table 9D5C (FONT_FORMAT.md) |
| 9B5C/5D/5E/5F | glyph height 8, line gap 3 (line pitch 11), letter spacing 1, glyph-size shift 0 |
| 0042/0044 | tick counter, incremented by the INT 8 hook at 0x0000, which calls the old handler (int 64h) every 4th tick, so it runs at about 72.8 Hz (GUESS). 0x00F8 resets timer slot 0; 0x01AE returns the ticks since then |

## 3. Draw: `0x5614 menu_draw(rec, mode)` (Pascal: push rec, push mode)

```c
void menu_draw(Menu *m, int mode) {                 /* 5614 */
    mouse_push(0);                                  /* 1D55(0): save cursor state, hide (1D19) */
    if (mode == 2) box3134(m->X0, m->Y0, m->X1, m->Y1, [6346] /*26*/);   /* bevelled box */
    else if (mode == 1) scroll30E6(m->X0, m->Y0);   /* province-panel sprite frame */
    /* mode 0: draw over whatever is there */
    for (i = 0; i < m->count; i++) {
        Item *it = &m->items[i];
        word[9B69] = it->disabled ? 6 : m->colour;
        text899C(it->text, m->X0 + it->x0 + 11, m->Y0 + it->y0);
    }
    if (m->enabled) ornament_and_title56B6(m);     /* title is ONLY drawn here */
    mouse_pop();                                    /* 1D8A */
}
```
Callers: the battle passes mode 2 (BF18 for 73B6, C0AC for 73C8). The province menus use mode 1. The text and ornament
geometry is VERIFIED in modes 1 and 2.

### 3.1 Box `0x3134(x0,y0,x1,y1,fill)`: all inclusive, colours exact (VERIFIED, = port `ui_frame(x0,y0,x1-x0+1,y1-y0+1)`)

```
if (fill != -1) rect(x0,y0,x1,y1, fill)
colour 8:  vline x0 (y0..y1), vline x0+1 (y0+1..y1-1), vline x1 (y0..y1), vline x1-1 (y0+1..y1-1)
           vline x0+3 (y0+3..y1-3), hline y1-3 (x0+3..x1-3), vline x1-3 (y0+3..y1-3), hline y0+3 (x0+3..x1-3)
colour 0:  hline y1 (x0..x1), hline y1-1 (x0+1..x1-1)
colour 15: hline y0 (x0..x1), hline y0+1 (x0+1..x1-1)
```
Order matters at the corners: (x0,y0+1) and (x1,y0+1) stay 8, (x0,y1-1) and (x1,y1-1) are 0 at x0+1..x1-1 but 8 at x0 and x1.
The capture shows (2,3)=8, (2,81)=8, (3,81)=0.

### 3.2 Mode 1 frame `0x30E6(x0,y0)` (DECODED)
`[9AFA]=[6346]; rect(x0+4, y0+3, x0+0x92, y0+0x71)` (fixed 143x111 fill, independent of X1/Y1), then four sprites
`8E62(spr, x0, y0)` with `spr = [C0F6] + 0, 14, 28, 42` (14-byte sprite descriptors; DS:CE0E.. at run time). Only for the
150x118 province-panel boxes. Not rendered here.

### 3.3 Ornament and title `0x56B6(rec)` (VERIFIED for single != 0 with a mouse)

```c
[9AFA] = 8;
for (i = 0; i < count; i++) {                       /* per-item pass */
    sx = X0 + it[i].x0;  yt = Y0 + it[i].y0;  yb = Y0 + it[i].y1 - 1;
    if (![00BA])        sprite8E62([C1D6], sx, yt + 1);          /* keyboard mode: a bullet sprite per item (DECODED) */
    else if (!single) { for k in 2,4,6: line(sx+k, yt, sx+k, yb);  /* per-item ornament (DECODED; record 11CA) */
                        line(sx+1, yt, sx+7, yt); line(sx+1, yb, sx+7, yb); }
}
if (single && [00BA]) {                             /* one ornament for the column */
    sx = X0 + it[0].x0;  yt = Y0 + it[0].y0;  yb = Y0 + it[count-1].y1 - 1;
    for k in 2,4,6: line(sx+k, yt, sx+k, yb);       /* colour 8 */
    line(sx+1, yt, sx+7, yt);  line(sx+1, yb, sx+7, yb);
}
word[9B69] = colour ^ 0x0F;                         /* 15 */
text_centered8BE4(title, (X0 + X1) / 2, Y0 + 6);
```
Battle 73B6: sx = 8, cap row y 25 x 9..15, lines x 10/12/14 from y 25 to y 64, base row y 64 x 9..15. The cap and base rows
cover the line ends, so there are 40 rows. Game Control 05A6: sx = 87, y 102..171. Title: "Choose Formation" W = 78,
x = 54 - 39 = 15, y = 8 (cap top 9). "Game Control Menu" W = 85, x = 166 - 42 = 124, y = 85.

### 3.4 Disable / enable (DECODED)
- `0x55FF(rec)`: `enabled = 0`, then **0x57F7**: `[9AFA]=[6346]`. If single, it erases the ornament with `rect(sx, yt, sx+8, yb)` (yt/yb
  as above). Then it redraws the title in colour 6 (`word[9B69]=6`, same position). Items are not redrawn. It is used to grey a
  menu while a sub-action runs (e.g. 0x25C0 for 11DC, 0x2D25 for 1B7E).
- `0x55EA(rec)`: `enabled = 0xFFFF`, then 0x56B6 (redraw the ornament and the title in colour 15).

### 3.5 Highlight `0x585B(rec, index, on)` (Pascal: push rec, push index, push on) (DECODED)

```c
if (index == 0xFF) return;
mouse_push(0);  [9AFA] = [6346];
word[9B69] = it->disabled ? 6 : colour;
if (on) { [9AFA] = 8; word[9B69] = 15; }
text899C(it->text, X0 + it->x0 + 11, Y0 + it->y0);  /* ink only: recolours the same glyph pixels */
if (![00BA]) rect(sx+2, yt+3, sx+6, yt+5);         /* keyboard mode only: 5x3 marker in the bullet, colour 8 on / 26 off */
mouse_pop();
```
With a mouse, "highlighted" means the item text is in colour 15 and nothing else changes. Off means it is redrawn in colour 0
(or 6 if the item is disabled).

## 4. Run: `0x54C8 menu_run(rec, preselect)` → index, or 0xFF on Esc (DECODED)

```c
int menu_run(Menu *m, int pre) {                    /* 54C8 */
    if (![00BA]) return menu_run_keys5901(m, pre);  /* no mouse */
    mouse_push(1);                                  /* 1D55(1): save cursor state, show (1CCE) */
    wait_release35DB();                             /* until [B2]==0, cursor keeps moving (1BE6) */
    int r = 0x100;
    do {
        if (wait_input359B() == 1) r = hit552B(&mouse_xy /*DS:B6*/, m);   /* left button */
        else                       r = hotkey5593(m, [C07F]);           /* key, or right button with [C07F]=0 */
    } while (r == 0x100);
    highlight585B(m, r, ON);                        /* no-op for 0xFF */
    wait_release35DB();
    mouse_pop();
    return r;
}
```
- `0x359B` wait: loop { poll mouse 0644; move cursor 1BE6(x,y); key = 3603() } until a button or a key. Button: `[C07F]=0`,
  return 1 (left) / 2 (right). Key: `[C07F]=key`, return 0. It returns at once while a button is still held.
- `0x552B(xy, rec)`: if the point is not in the box (8238, inclusive), return 0x100. Otherwise convert to box-relative coords and
  return the first i with the point in `(x0..x1, y0..y1)` inclusive; if none, return 0x100. Adjacent items share their border
  row (item i y1 = item i+1 y0), and the first match wins, so the shared row belongs to the upper item.
- `0x5593(rec, key)`: `k = toupper(key)` (9DDC, ctype bit 2 at DS:A435); return the first i with `item.hotkey == k`. If none:
  `k == 0x1B` → 0xFF, else 0x100. The disabled flag is not checked. **Quirk**: a right click passes key 0 and so selects the
  first item whose hotkey is 0 (records 4080 and 9750 have hotkey 0).
- Clicks outside the box or on gaps are ignored. Esc is the only way to get 0xFF. Arrows do nothing in mouse mode: 3603 returns
  the raw key before its arrow handling.
- `preselect` is ignored in mouse mode.

### 4.1 Keyboard-only path `0x5901(rec, sel)` (DECODED, never runs in DOSBox because the mouse is present)

```c
highlight(m, sel, ON);
for (;;) {
    if ([A2] && ticks_since_slot0() >= 10) {        /* 01AE; about 0.14 s auto-repeat */
        reset_slot0();                               /* 00F8 */
        highlight(m, sel, OFF);
        if ([A2] == 8 || [A2] == 2) sel = (sel == 0 ? count : sel) - 1;          /* Up / Left: previous, wraps */
        if ([A2] == 4 || [A2] == 1) { if (++sel == count) sel = 0; }             /* Down / Right: next, wraps */
        highlight(m, sel, ON);
        if ([A5] == 0) [A2] ^= [A3];                 /* first make: one step only; typematic repeats keep moving */
    }
    k = key03AA();
    if (k == 0x0D || k == 0x10 || k == 0x20 || k == 0x2B) return sel;   /* Enter, Ins (3AA maps scan 52 → 0x10), Space, + */
    if (k == 0) continue;
    r = hotkey5593(m, k);
    if (r != 0x100) { highlight(m, sel, OFF); highlight(m, r, ON); return r; }  /* 0xFF (Esc) highlights nothing */
}
```
Diagonals (Home/PgUp/End/PgDn) do not match any of the four values, so they do nothing.
`0x03AA` reads a key: it returns the ASCII code, or 0 if no key is waiting. Extended keys (arrows) read as 0 here, because
INT 9 handles them. Scan 52 (Ins) and 9B both map to 0x10. Ctrl-S (0x13) is swallowed. The INT 9 hook toggles **DS:3EE9**
(music on/off, played by the INT 8 hook) on **Ctrl+S** (scan 1F with Ctrl).
In the keyboard path the game's other screens use `0x3603`: there Enter or + set [B2] = 2 (right button) and Ins or Space set
[B2] = 1 (left button), and the arrows move a virtual cursor by 8 px (1 px with Ctrl or Scroll Lock, [A4] & 0x14).

## 5. Return values used by callers
Index 0..count-1, or 0xFF on Esc. The battle (BF9F/C0B6) passes preselect 0 and treats 0xFF as cancel (BATTLE_NOTES).
0x100 is internal only.

## 6. Text helpers (DECODED; transparent mode VERIFIED by the renders)

### 6.1 `0x899C text(str, x, y)` (Pascal: push str, push x, push y)
```c
if (opaque = [9B6A] & 0x80) and *str:              /* never used by menus */
    per line: rect(x-1, y + yoff[0]-2, x + W(line), y + 11 - 1) in colour (([9B69] & 0xF0) >> 4)  /* yoff[0] = 1 */
cx = x;
for (c in str) {
    if (c == '\n') { cx = x; y += [9B5C] + [9B5D] /*11*/; continue; }
    if (c < 0x20) continue;
    glyph8D56(c, cx, y);   /* ink pixels of glyph[c-0x20] at (cx+i, y + yoff[c-0x20] + r), colour [9B69] & 0x0F */
    cx += width[c-0x20] + [9B5E] /*1*/;
}
```
0x8D56 builds a 4-bpp sprite at DS:9B48 and blits it with 0x8E62. The glyph converter is picked by `[9B26]` (=2 at run time)
from the table DS:9B60. Only ink is drawn (FONT_FORMAT.md), confirmed again by the 0-difference renders.

### 6.2 Width `0x8C58(str)` = Σ over chars ≥ 0x20 up to NUL/LF of `(width + spacing)`. **It includes the trailing spacing.**

### 6.3 `0x8BE4 text_centered(str, cx, y)`: for each LF-separated line, `text(line, cx - (W(line) >> 1), y); y += 11`.
Other variants: 8C3A right-aligns (`x - W`); 8B9D/8B2C/8AF2/8B66 erase a background rect first; 8A9E/8AC8 print numbers.

## 7. Other primitives
`0x7E74 rect(x0,y0,x1,y1)` is an inclusive fill in [9AFA], with endpoints swapped if reversed and y clipped to [C0DC]..[C0E0].
`0x7ECA hline(x0,y,x1)`. `0x925C line(x0,y0,x1,y1)` in [9AFA], dispatched per video mode through DS:9C8C+[9B26].
`0x8238 in_rect(x,y,rect*)` is inclusive and returns FFFF/0. `0x1D55(n)` / `0x1D8A` push and pop the cursor visibility
(stack at DS:A674, depth ≤ 30). The names "show" and "hide" for 1CCE/1D19 are a GUESS.

## 8. What the port should do (spec)
1. Store the record's colour, single, enabled and per-item disabled fields as well.
2. Draw: frame (mode 2 = `ui_frame`), then each item at `(X0+x0+11, Y0+y0)` in colour 0 (6 if disabled), using `ui_text`
   (same font). If enabled: the ornament in colour 8 as in section 3.3, then the title in colour 15 at
   `x = (X0+X1)/2 - (Σ(width+1) >> 1)`, `y = Y0+6`. `ui_text_width` drops the trailing pixel, so use `ui_text_width(t)+1` before
   halving. With no enabled flag, draw no title.
3. Run: no hover effect. A left click inside the box on an item rect returns its index. A hotkey (case-insensitive) returns its
   index. Esc returns 0xFF. Everything else is ignored. On a hit, redraw that item's text in colour 15, wait for the button to be
   released, and return. Arrow keys do nothing while a mouse exists.

## 9. Verification method
Script: `C:\Users\erikk\AppData\Local\Temp\claude\bfb5522be8ee045d\c4bf0f5f-57f9-4ea6-80b3-e1aa05bf0592\scratchpad\wf\menu_render.py`
(scratch, not in the repo). It reads the record, strings and font from `OVL0.OVL` (DS = file offset − 0xB400; font at
DS:9C9C/9CFC/9D5C, the same bytes `assets_load_font` reads at 0x1509C) and implements sections 3.1, 3.3 and 6.1–6.3 literally.
Then it compares every pixel of the box against the VRAM capture:
- `scr_battle_formation.bin`, 73B6, (2,2)-(106,82): 8,505 px, **0 differ**. The mouse cursor at about 160,100 is outside.
- `scr_game_control_menu.bin`, 05A6, (81,79)-(251,197): 20,349 px, **0 differ**.
- `scr_rome_province.bin` 5A30 and `scr_narbonensis_province.bin` 5790, mode 0 overlay (the frame is mode 1): 0 pixels changed.

## 10. Open points
- The mode 1 sprite frame (30E6, sprites at [C0F6]) and the keyboard-mode bullet sprite [C1D6] are not rendered or verified.
- The highlight (585B) and the disabled colour 6 are not checked against a capture: no capture shows a selected or greyed menu.
- The 72.8 Hz tick rate and the cursor show/hide naming are GUESSes.

## Corrections (verifier)

Checked 2026-11-09 against the code (seen.pkl / EXE_RUNTIME.bin) and with an independent renderer
(`scratchpad\wf\vmenu.py` + `vcmp.py`, written from the code, not from section 9's script). The drawing claims hold:
73B6 on `scr_battle_formation.bin` 8,505 px / 0 differ; 05A6 on `scr_game_control_menu.bin` 20,349 px / 0 differ
(title moved up 2 px gives 196 differ, so the test is sensitive); mode 0 overlays 5A30 on Rome and 5790 on Narbonensis 0 differ,
and the cross-check (5790 on Rome, 5A30 on Narbonensis) gives 241 / 296 differ, so 5A30 really is Rome's record.
The corrections below are about things the notes leave out or overstate.

1. **Callers pre-highlight the current setting (missing from sections 0, 4 and 8).** The statement that 0x585B is used "only
   ... just before 0x54C8 returns" is wrong for several callers, which call `585B(rec, cur, ON)` themselves before `54C8`:
   0x1A31 (05DC, current level from `[di+48E]`), 0x1B63 (05B8), 0x7D21 (6054, only if `[BA]!=0`), 0x7D76 (60AE, only if
   `[BA]!=0`). So with a mouse the current choice is shown in colour 15 while the menu waits. 0x54C8 never turns that item
   off: after a click on another item both texts are colour 15 until the screen is redrawn. 0x6EBE calls `585B(5790, 1, OFF)`
   (redraws item 1 "Plunder" in colour 0). The port's `ui_menu_draw`/`run` needs a "highlight item i" call that callers use. (DECODED)
2. **Held button = drag-select (missing from section 4).** 0x359B returns 1 at once while `[B2]&1` is set and skips the cursor
   redraw 0x1BE6 (0x359E jne 0x35BF). 0x54C8 loops on 0x100, so if the button goes down outside the items and is held, the
   hit test re-runs on every poll with the live `[B6]/[B8]`; dragging onto an item selects it without a release, and the
   cursor sprite does not follow while held. The 0x35DB at 0x54DE only waits for buttons held on entry. (DECODED)
3. **Item rects are not uniform.** Section 1 says all stock records use `(6, 23+10i, 138, 33+10i)`. Exceptions (static data):
   11CA OK `(40,28,72,37)`, Cancel `(76,28,119,37)`; 9750 `(7,18+13i,93,28+13i)`; 4080 `(7,4+13i,93,14+13i)`; 1B7E
   `(6,24+10i,309,34+10i)`. Read the rects from the record, do not assume the 10 px pitch. (VERIFIED by reading OVL0)
4. **Missing record DS:3E5A** (Yes / No, hotkeys 'Y' 'N', items at DS:3E42, count 2, single 0, enabled FFFF, title 0): box and
   item rects are all 0 in the static data and are filled at run time; drawn with mode 2 at 0x5CC8, run at 0x5CF3 with
   preselect 1 (ignored with a mouse). The catalogue scan missed it, probably because the box is 0. (DECODED)
5. **0x03AA detail.** It does not return 0 for an extended key: at 0x3CC `or al,al; je 0x3AA` it discards keys whose ASCII is 0
   (arrows, F-keys) and Ctrl-S (0x13 → 0) and reads again; it returns 0 only when the BIOS buffer is empty. Ins (scan 52) and
   scan 9B become 0x10 before that check. (DECODED)
6. **Keypad - / + with `[9AB2]==6`.** The INT 9 hook (0x431..0x44C) skips scans 4A/4E only when `[9AB2]!=6`. When it is 6, the backquote key
   (scan 29) → Up and backslash (scan 2B) → Left as stated, but keypad - and + are then *not* skipped and map through DS:00A6 to 04 (Down) and
   01 (Right). (DECODED)
7. Static `[6346]` is 7 in OVL0; 26 is the run-time value (all 33 snapshots and t10_in.bin). A port that reads the fill from
   the static image must hard-code 26. Same for the font pointers DS:9B56..9B5F, which are 0 statically and
   `9C9C FC9C 5C9D 08 03 01 00` at run time. (VERIFIED)

## Port status (2026-11-09, port/src/ui.c)

Ported literally (mouse paths only): `ui_menu_load` (record colour, single, enabled, per-item hotkey byte + disabled byte;
title pointer 0 = no title), `ui_menu_draw_mode` = 0x5614 (mode 2 box 0x3134 = `ui_box`, mode 0 nothing, mode 1 only the
0x30E6 fill rect, not its sprites), `menu_ornament_title` = 0x56B6 (single and per-item ornaments, title in colour ^ 0x0F via
8BE4 at ((X0+X1)/2, Y0+6)), `ui_menu_highlight` = 0x585B, `ui_menu_enable` / `ui_menu_disable` = 0x55EA / 0x55FF+0x57F7,
`ui_menu_hit` = 0x552B (signed inclusive 0x8238, box first, first item wins), `ui_menu_hotkey` = 0x5593 (toupper, Esc -> -1),
`ui_menu_run` = one poll of 0x54C8 (-2 = nothing yet; `*last_click` != 0 = 0x35DB wait for release; after that a held
left button hit-tests on every poll = drag-select, correction 2; a non-left button -> hotkey 0; keys -> hotkey only in a
poll with no button down, as 0x359B reaches 0x3603 only when [B2] == 0; a key pressed during a hold is read in the first
button-free poll only if it is still down, where the original keeps it in the BIOS buffer). Items, highlight and title
write the whole colour word to [9B69] (0x5673 / 0x5898 / 0x57CF `xor al, 0Fh`) and draw with `ui_text_attr` /
`ui_text_block_attr`, so bit 15 gives 899C's opaque band (every static record has colour 0, so no capture shows it;
harness: record 73B6 with colour 0x8030 paints 2,604 band px in colour 3). The caller
draws 0x54C8's final `585B(r, ON)` with `ui_menu_highlight`. `ui_menu_draw(fb, m, pre)` = mode 2 + `585B(pre, ON)` when
pre >= 0 (the callers 0x1A31 / 0x1B63 / 0x7D21 / 0x7D76 pre-highlight, correction 1). Keyboard-only paths (5901, bullet sprite
[C1D6], 5x3 marker) are not ported.

Checks with the port's own ui.c (harness `wf2\ui\harness\test_ui.c` + `verify.py` in the scratchpad): 73B6 on
scr_battle_formation.bin box (2,2)-(106,82) 8,505 px / 0 differ; 05A6 on scr_game_control_menu.bin (81,79)-(251,197) 20,349 px /
0 differ; mode 0 overlays 5A30 on Rome 1,143 px drawn / 0 changed and 5790 on Narbonensis 1,088 / 0 changed (cross-check 241 /
296 changed). `--dump-frame game_menu` went from 1,772 to 105 differing pixels. 54C8 poll checks on 73B6: (8,25) -> 0, shared
row y 35 -> 0 (upper item), (106,30) -> 0, (107,30) -> none, gap (20,70) -> none, held from entry -> none until released, then
held drag onto (20,47) -> 2, 'w' -> 1, 'L' -> 3, Esc -> -1; left held at (200,100) + 'w' -> none, waiting for release +
'w' -> none, released + 'w' in the same poll -> 1, right held + 'w' -> none.

For the callers (not owned here): `ui_menu_draw(fb, m, pre)` draws 585B ON for pre >= 0, so battle_ui.c `menu_hook`
(which passes the item under the mouse) now shows a hover highlight the original does not have: it should pass -1, or the
current setting where a caller in the EXE pre-highlights. game.c: an outside click now returns -2 (0x552B, the menu stays
open), and `*last_click` must be left to `ui_menu_run` (game.c sets `g_last_click` = left-held every frame, which turns
every press held across frames into "held from before" and disables the drag-select).

Not changed: `ui_text_centered` keeps the port's older rule (cx - ui_text_width/2), because game.c `draw_map_buttons` is
tuned to it (with the 8BE4 rule `screen map` goes 292 -> 406 px). The original 0x4AA6 draws "End Turn" left-aligned with 899C
at (X0+5, Y0+4); game.c should do that and 8BE4 callers should use `ui_text_block`.
