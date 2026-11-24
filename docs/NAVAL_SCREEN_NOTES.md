# Naval battle: the screen, the input and the timing (OVL0 section 5)

Scope: what the naval battle draws, when, where and into which buffer; its texts, sprites, music, input and timing.
Source: `disasm/OVL_NAVAL.asm` (addresses = 0823 offsets), the EXE helpers in `disasm/EXE_RUNTIME.clean.asm` /
`EXE_RUNTIME.bin`, DS values read from the static OVL0 image (file 0xB400 = DS:0000) and the runtime image
`tools/dosbox/results/mem_map_gamemenu_640k.bin` (DS at linear 0x16860). All the naval tables below are identical in
both images, except [6346] (box fill: 7 static, 26 runtime/VGA), [9B26] (0 static, 2 = VGA runtime) and the font block
9B56..9B5F (0 static). Function index and DS map: `docs/NAVAL_INDEX.md`. Logic of the duel and the fleet battle
belongs to the other notes; it appears here only where the screen depends on it.

Marks: **D** = decoded from the code; **V** = verified by running or comparing; **G** = guess. Since 10 Nov the port's
pictures are compared with two real battles captured in DOSBox-X (tools/dosbox/results/naval, section 18): every
pixel the port draws on the captured screens matches (the EXE's mouse cursor aside), so the coordinates of the duel frame, the panel, the intro, the boarding, the
sinking and the sharks are **V** (section 18 lists what was and was not on the captured screens).

Calls are Pascal: the first argument pushed is the highest `[bp+N]`, the callee pops. Below, arguments are written in
push order. "side" 0 = the enemy flagship, 1 = the Roman flagship (the per-side tables are pairs: C388/C389 heading,
C38E/C38F strength, C390/C391 oar frame, C372/C374 = enemy x/y, C376/C378 = Roman x/y, C37E/C380 = men, ...).

## 0. Summary for the implementer

1. One screen, 320x200: the sea (fill colour 9, rows 0..185), the two flagships (oars sprite + hull sprite + flames),
   projectiles (fire-pots, arrow volleys, splashes, the Roman corvus), and a status panel in rows 186..199 drawn in a
   second, small 6-px font (DS:A3B4). No background picture, no scrolling: the field is the screen (ships kept inside
   x 15..304, y 15..170). D
2. Double buffering: each frame is drawn into the work buffer and copied whole to the screen (6887), at most every
   3 timer ticks (72.8 Hz timer) = 24.3 fps. D
3. **Drawing routines have game side effects and roll dice** (C71B zeroes the men of a flagship at strength <= 0;
   CD4B burns strength; BA7A runs the corvus countdown and its hit test; smoke and shark animations call rng_roll). The
   headless mode must run the same drawing calls (into off-screen buffers) at the same ticks. D
4. The main loop runs its logic once per pass, as fast as the CPU allows; B6A0 rolls the dice on every pass. The
   original's dice sequence therefore depends on the CPU speed. A port must fix the pass rate: one pass per timer tick
   (section 3.3). D for the fact, G for the choice.
5. A 1990 bug found in the drawing: the flames routine CD4B can turn a sunk flagship's strength 0 into -1 during the
   sinking animation, and then the fleet battle is skipped (section 9.1). D

## 1. The sequence of screens (D)

| step | code | what the player sees | buffer / target |
|---|---|---|---|
| 1 | B13D `1D55(0)` | mouse cursor hidden for the whole battle (popped at B592 `1D8A`) | - |
| 2 | BD8A: 5D20, 5E1A(1) | map music stopped, map music (cn2) unloaded | - |
| 3 | C274 | menu "Choose a flag ship:" over the map, only if the Roman fleet has ships of 2 or 3 types (else no menu) | screen |
| 4 | BF50 | GALLEYS.LZW (NAVL) loaded into the work buffer and copied to the screen; sheets and music loaded; cn9a starts | work -> screen |
| 5 | C105 | the intro: the galley picture with a waving pennant and cycling water colours, until a key or a mouse button | work -> screen |
| 6 | BE0F + BEAE | the battle screen: sea, panel, both ships at their start positions | screen, then saved to work |
| 7 | B248..B417 | the duel, one frame every >= 3 ticks; boarding (BB7E) / re-engage prompt / retreat | work -> screen |
| 8 | B419..B48D | flagship sinking: D3F2 (the survivor backs off, the wreck turns broadside, smoke), D60C (sinking frames, sharks) | work -> screen |
| 9 | B49E..B4C3 | the fleet battle: no picture; only the panel's SHIPS numbers are reprinted at the end (DC8E) | screen |
| 10 | B59F | the result message box ("<Roman fleet> met the <enemy fleet> on the gently rolling waves of <sea>." ...) | screen |
| 11 | C34F, B592 | naval music stopped and unloaded; cursor popped; return to the caller (which redraws its own screen) | - |

Returning 0 (no battle): Esc in the flagship menu (C274 -> FF -> BD8A returns 0 -> B14F jumps to the end), or BF50
failing to load. The map music is already unloaded then (step 2); the caller's next map_build plays it again (5D29
loads first).

## 2. Buffers and EXE drawing helpers (D; VGA path [9B26] = 2)

Descriptors (9 words, BATTLE_UI_NOTES section 0): DS:9AB6 = screen (A000), DS:C1DA = work buffer, DS:C0D0 = current
target (a copy of one of them). Word +0x10 of each is the clip y1: **9AC6 (screen), C1EA (work), C0E0 (current)**.
The naval code writes all three at once: 0xB9 = 185 while the battle field is drawn (B23C, C837, DD1B), 0xC7 = 199
while the panel or a message box is drawn (C71C, DCD7, B649).

| EXE | VGA behaviour |
|---|---|
| 67B6 | `if ([4F96] != 1 && [9B26] != 2) 6973(0,0,319,199); else 691B(0,0,319,199);` -> on VGA always **691B: copy the screen into the work buffer** (bytes at 67B6: `83 3E 96 4F 01 74 17 83 3E 26 9B 02 74 10 ... E8 A0 01 C3` / `... E8 38 01 C3`). The index's guess "restore" is the other branch. |
| 67E4 | target = work buffer (C0D0 = C1DA); mode 6 only: port 3DF |
| 680F | target = screen (C0D0 = 9AB6) |
| 6837 | `if ([4F96] == 0) 6887(); [4F96] = 0; target = screen` |
| 6887 | `[4F96] ^= 1; target = work; copy the whole work buffer to the screen (0494, 0x7D00 words)` = present a frame |
| 691B(x0,y0,x1,y1) / 6973(...) | rect screen -> work (save) / work -> screen (restore) |
| 7E74(x0,y0,x1,y1) | filled rect in [9AFA], corners sorted, y clipped to [C0DC]..[C0E0] (when [9B24] != 0), x to [C0DA]..[C0DE] |
| 7ECA(x0,y,x1) | horizontal line in [9AFA]; nothing if y is outside [C0DC]..[C0E0] |
| 8E62(desc,x,y) | sprite at (x - xhot, y - yhot), size w x rows, **clipped to the target's clip rect** C0DA..C0E0 (via 817C); 4-bit pixels, transparent nibble skipped, colour = DS:60C0 + 16*lut + nibble. nego.c's sprite_8E62 clips to the screen only: the naval port must clip to the target rect (ships near y 185 are cut at the panel). |
| 817C(out,a,b) | rect intersection a ∩ b (out = 0 -> DS:9AE8); returns 0 if empty |
| 81EA(out,a,b) | rect union (min x0/y0, max x1/y1) |
| 8238(x,y,rect) | point in rect, inclusive -> FFFF / 0 |
| 899C(s,x,y) | text in [9B69] with the font block 9B56..9B5F; glyphs go through the sprite blitter, so they are clipped too (gamectl.c note) |
| 8BE4(s,cx,y) | centred: every '\n' line at `cx - (8C58(line) >> 1)`, lines 9B5C+9B5D apart |
| 8C58(s) | width = sum(width[c-0x20] + [9B5E]) up to NUL or '\n' (includes the trailing spacing) |
| 8C3A(s,x,y) | right-aligned: `899C(s, x - 8C58(s), y)` |
| 8A9E(long v,x,y) / 8AC8(long v,x,y) | ltoa(v, 10) then 899C (left) / 8C3A (right-aligned at x) |
| 8AF2(long v,x,y,w) | `7E74(x, y+Y0, x+w, y+Y0+[9B5C])` then 8A9E(v,x,y); Y0 = byte [[9B58]] = yoff of ' ' |
| 8B2C(long v,x,y,w) | `7E74(x-w, y+Y0, x, y+Y0+[9B5C])` then 8AC8(v,x,y) (bytes at 8B2C decoded by hand: `8B 46 08 2B 46 04 50 ...`) |
| 01B6(i,r,g,b) | VGA DAC: out 3C8 i; out 3C9 r, g, b (6-bit) |
| 84BB | wait until bit 3 of port 3DA goes 1 then 0 (end of a vertical retrace) |
| 0F0C(name,&arch,seg,off) | load an LZSS (format 0x80) member, unpacked, into seg:off; FFFF = ok |
| 163B(name,&arch,0) / 19A0(sheet) | load / free a sprite sheet nameHDR.LZW + nameDAT.LZW (14-byte descriptors) |

### 2.1 The two fonts (D, values V from both DS images)

The 10-byte block DS:9B56..9B5F (width table, yoff table, glyph table, height, line gap, spacing, shift) is replaced
by `rep movsw` from one of two images:

| image | 9B56 widths | 9B58 yoff | 9B5A glyphs | height | gap | spacing | used by |
|---|---|---|---|---|---|---|---|
| DS:A05C | 9C9C | 9CFC | 9D5C | 8 | 3 | 1 | the normal font: set before every message box (B3DB, B655, BCDF) |
| DS:A3B4 | A0B4 | A114 | A174 | **6** | 2 | 1 | the **naval panel font**: set by BEAE (BEFD) and after the boarding box (BD2C) |

So MENU_NOTES' "there is only one font" holds for the rest of the game only: OVL0 data at file 0x6725 (another
overlay section) also points at A3B4 (G: a second user). The port's font_glyph (assets.c) is the big font only; the
small font is in the static DS (glyph c at A174 + 6*(c-0x20), one byte per row, bit 7 left; ink rows 0..4, row 5
blank; yoff 1 for most, 0 for ' ', '&' and 'i', 2 for 'g', 'p', 'q', 'y'). Rendering "SHIPS MEN 0123456789" from these tables
gives clean 5-px capitals (V, render only). Widths (index c-0x20): ' ' 1, '0' 4, '1'-'3' 3, '4' 4, '5' 3, '6' 4,
'7' 3, '8' '9' 4, 'A'-'H' 4, 'I' 3, 'M' 'N' 5, 'S' 3, 'W' 5.
Text widths 8C58 with the small font: "SHIPS" 22, "MEN" 17, "STOPPED" 33, "CRUISING SPEED" 64, "BATTLE SPEED" 53,
"ATTACK SPEED" 54, "RAMMING SPEED" 63.

After the battle the font stays A05C (B59F sets it before the result box), so the rest of the game is unaffected.

## 3. Time

### 3.1 Timer (D, from BATTLE_NOTES + the EXE bytes)

Tick counter DS:42 (low) / 44 (high), incremented by the INT 8 hook at 4 x 18.2 = 72.8 Hz (1 tick = 13.73 ms).
16 timer slots DS:46+4n (n masked to 0..15):
- `00D7(n)`: slot n = now. `00F8` = 00D7(0). `00FF`: **all 16 slots = now** (called by BEAE).
- `0183(n)`: now - slot n, 0xFFFF if the high words differ (so a slot started before a 16-bit wrap of the low word
  reads "very long ago" once; harmless). `01AE` = 0183(0).
- `011F(n)`: busy wait until now >= slot 0 + n (slot 0 not restarted).

Slots used by the naval battle (D):

| slot | user | rule | interval |
|---|---|---|---|
| 0 | B12A render gate | `01AE() > 2` -> reset, draw a frame | every 3 ticks = 41.2 ms (24.3 fps) |
| 0 | C105 intro pennant | `01AE() > 3` -> reset, next pennant frame | 4 ticks = 54.9 ms |
| 0 | D3F2 / D60C sinking frames | reset, draw, then busy-wait `01AE() >= 14` | 14 ticks = 192 ms (5.2 fps) |
| 0 | BB7E boarding start | `011F(0x24)` | 36 ticks = 0.49 s pause |
| 1 / 9 | CB11 fire-pot step (side 0 / 1) | `> 6` | 7 ticks |
| 1 / 9 | C9A3 arrow step (same slots!) | `> 8` | 9 ticks |
| 2 / 10 | B7FA move step | `> 93FE[gear]` | 22,22,18,18,13,13,9 ticks for gear 0..6 |
| 2 | BB7E boarding frames (side 0's move slot reused) | `> 8` | 9 ticks = 124 ms |
| 3 / 11 | C3D3 turn one heading step (22.5 degrees) | `> 9828[type]` | 73 / 109 / 145 ticks (Trireme / Quinquereme / Galleon) |
| 4 / 12 | C422 speed level change | `> 982C[type]` | 73 / 109 / 145 ticks |
| 5 / 13 | C846 fire-pot reload | `> 0x90` | 145 ticks = 1.99 s |
| 8 | fleet battle round DA50 during the duel (B2F4) | `> [C386]` | first 216 ticks (2.97 s, BE0F), then `(rng(9)+2)*72` = 144..720 ticks (2.0..9.9 s) |
| 8 | BB7E boarding men rounds | `> [C386]` | first 72 ticks (BBB8), then `rng(108)` ticks |
| - | C1BB water colour cycling | own long word DS:9812/9814: proceeds when `now > last + 4` | 5 ticks = 68.7 ms |

Slots 1/9 are shared by the fire-pot and the arrows of the same side, but they never run together: C846 does not
launch a pot while that side's arrows fly (`C3BA[s] != 0`) and C92B does not launch arrows while a pot flies or
splashes (`C392[s] != 0`). Harmless (D).

### 3.2 Display pacing (D)

The duel loop (B248) has no wait: it runs its logic every pass and draws only when slot 0 shows > 2 ticks. The other
loops are busy-waits on slot 0 or 2 (the intro also polls the mouse and the keyboard every pass). Every frame is
presented by 6887 (whole work buffer to the screen), except the first battle frame (BEAE draws on the screen) and the
message boxes (drawn on the screen).

### 3.3 The pass rate and the dice (D for the facts, G for the proposal)

Per pass of the duel loop, outside the tick gates: C4F8 (distance), C641 (bearing), C366 (input: one key read),
**B6A0 (enemy AI: rng_roll(3) or rng_roll(2) every pass, b7cf / b7da; rng_roll(16) when the bearing is dead astern;
another rng_roll(3) while the Roman fire-pot flies)**, D242, CF7F, CFDD, CD96 (collision, rolls on contact), C92B.
So on a fast machine many passes, and many dice, fall between two ticks; the original is not reproducible across CPU
speeds. Everything else is tick-gated (above), so any machine fast enough for one pass per tick sees the same events
at the same ticks; only the number of dice per tick differs.

Proposal for the port (both window and headless): **one duel pass per timer tick**: pass, then advance the virtual
clock [42]/[44] by 1 (window: wait until the real 72.8 Hz clock reaches it). The tick gates then fire exactly as on a
fast PC, and a run is fully determined by the RNG seed and the input per tick. The busy-waits (sinking 14 ticks,
boarding 9 / 36 ticks, intro 4 / 5 ticks) advance the virtual clock to the threshold at once in headless mode. The
render gate stays "slot 0 > 2" (a frame every 3 passes), which matters: burning (CD4B), the corvus countdown (BA7A)
and the speed message (C37C) count frames, not ticks.

## 4. Set-up screens

### 4.1 C274: "Choose a flag ship:" (D; strings V from DS)

```c
int C274(fleet) {                              /* returns the ship type 0..2, or FF = cancel */
    n = 0;
    for (t = 0; t < 3; t++) if (W(0x110E + 12*fleet + 2*t)) {
        type_of[n] = t;
        item[n].text = W(0x9722 + 2*t);        /* 96D4 "Trireme", 96DC "Quinquereme", 96E8 "Galleon" */
        item[n].hotkey = B(0x9728 + t);        /* 'T', 'Q', 'G' */
        n++;                                    /* items = DS:972C + 12*k */
    }
    B(0x975E) = n;                              /* menu count */
    k = 0;
    if (n > 1) {
        W(0x9756) = (n == 2) ? 0x76 : 0x82;     /* box y1 */
        5614(0x9750, 2);                         /* draw, mode 2 = bevelled box */
        k = 54C8(0x9750, 0);                     /* run; FF = Esc */
        if (k == 0xFF) return 0xFF;
    }
    return type_of[k];
}
```
Menu record DS:9750 (MENU_NOTES section 1): box (110,70)-(210,118) for 2 items, (110,70)-(210,130) for 3; colour 0;
items DS:972C; title DS:96F0 "Choose a flag ship:"; single = FF; enabled = FFFF. Item rects (relative): (7,18)-(93,28),
(7,31)-(93,41), (7,44)-(93,54) -> item text at (128, 88 / 101 / 114). Drawn on the map screen (the target is the
screen there). With one ship type there is no menu and no way to cancel.

### 4.2 BF50: loading (D)

```
archive 9 (NAVL):   VGA: 0F0C("galleys.lzw" DS:97CC, &arch, [C1DE], [C1DC])   -> the work buffer (64000 bytes, V)
                    C3A6 = 163B("oars1")  C3A8 = 163B("oars2")
archive 8 (MNAVAL): non-VGA: 1273("galleys.cbm") into C356, 67E4, 8E62(C356,0,0), 680F, 86A0(C356)
                    6973(0,0,319,199)                                  <- the picture appears on the screen
                    C3AE = "bombs", C3AC = "smok",
                    C3A2 = sheet 980C[C37A] (enemy: "sml"/"med"/"lrg"), C3A4 = sheet 980C[C37B] (Roman; = C3A2 if same type)
                    "collide.lzw" (816 bytes, V) via 11C4: pointers C3B0..C3B8 = buffer + 0, 240, 480, 720, 768
                    C3BC = "flag"
music: 5D73(9704[1..4]) = 0x0D 0x0E 0x0F 0x10 (cn9a..cn9d, MNAVAL), then 5D29(0x0D)
```
MNAVAL has no GALLEYS.CBM (V, directory listing), so the non-VGA branch would show no intro picture (G: unreachable
in the port, which is VGA only).

### 4.3 C105: the intro (D)

```c
void C105(void) {
    si = 0; col = (VGA) ? 0x80 : 0x0B;
    00F8();
    for (;;) {
        6887();                                   /* present (first time: the bare picture); target = work */
        do {
            0644();                               /* mouse */
            if (03AA()) goto out;                 /* any key with an ASCII code (arrows etc. are swallowed) */
            if (W(0xB2)) goto out;                /* a mouse button HELD */
            if (VGA) C1BB();                      /* water colours */
        } while (01AE() <= 3);
        00F8();
        W(0x9AFA) = col; 7E74(38, 2, 68, 15);     /* erase the pennant: colour 0x80 = the sky */
        if (++si > 3) si = 0;
        8E62(C3BC + 14*si, 38, 5);                /* FLAG frame 1, 2, 3, 0, 1, ... */
    }
out:
    67B6();                                       /* VGA: screen -> work */
    6837();                                       /* target = screen */
    19A0(C3BC);                                   /* free the flag sheet */
}
```
FLAG frames (V, sheet): 30x12, 28x11, 28x10, 27x9, xhot 0, yhot 1/0/3/3, lut 15, transparent nibble 3. LUT 15 =
`81 42 82 83 84 67 85 BF 86 87 7F 88 89 8A 8B 8C`: the pennant uses the cycled colours too.

C1BB (water cycling, VGA only), every > 4 ticks (own clock DS:9812):
```c
if (now <= last + 4) return;
B(0x9816) = (B(0x9816) + 1) % 6;  B(0x9817) = (B(0x9817) + 1) % 5;   /* written as inc + "if > 5 -> 0" / "> 4 -> 0" */
84BB();
for (i = 0; i < 6; i++) 01B6(W(0x9762 + 2*((B(0x9816)+i) % 6)), RGB(0x9778 + 6*i));  /* colours 8C 8B 8A 89 88 87 */
for (i = 0; i < 5; i++) 01B6(W(0x976E + 2*((B(0x9817)+i) % 5)), RGB(0x979C + 6*i));  /* colours 84 67 85 BF 86 */
last = now;
```
RGB lists (6-bit): 9778 = (28,36,40) (24,32,32) (20,28,28) (16,24,28) (20,28,32) (32,36,36) = CMAP.BIN's 8C..87;
979C = (20,20,24) (20,24,28) (24,28,32) (20,24,28) (12,20,24) (CMAP has BF = (16,20,24), so the cycle also changes
that colour's value) (V, values).

C18C, the routine that would put the 11 colours back (`01B6(9762[i], 9778[i])` for i 0..10), has **no caller** (D,
NAVAL_INDEX). After the battle the DAC keeps the last rotation phase of colours 67, 84..8C and BF. LUT 8 (67, BF) and
LUT 25 (8B) also use them (V, LUT scan), so any picture drawn later through those LUTs, or with those palette indices,
shows slightly permuted blue-greys until something reloads the palette (G: not checked whether the map or the caller
reloads CMAP; a 1990 cosmetic bug candidate). Port: keep a "DAC" palette that C1BB modifies, then either restore
(fix) or keep (--original).

### 4.4 BE0F and BEAE: the start of a duel (D)

BE0F (also on "re-engage: Yes"): enemy at (40,40) heading 4 (east), Roman at (280,160) heading 12 (west) (DS:971E /
971A); oar frames 6; speed level 1 (cruising) both; gear 9818[5*type+1]; front ship C384 = 1 (Roman); C386 = 216;
all projectiles, corvus counters, fires off (C3A0/C3A1 = 0), C37C = 0. Strength and men are NOT reset.
Headings: 0 = north (up), 4 = east, 8 = south, 12 = west (sin table 987A, cos 989A, x += sin, y += cos; V by the sheet
render: hull frame h has its bow that way).

```c
void BEAE(enemy, roman) {               /* draws on the current target = the screen */
    00FF();                              /* all timer slots = now */
    W(0x9AFA) = 9;   7E74(0, 0, 319, 185);        /* sea */
    W(0x9AFA) = 0;   7ECA(0, 186, 319);           /* black line */
    W(0x9AFA) = W(0x6346); 7E74(0, 187, 319, 199);  /* panel, 26 */
    W(0x9B69) = 0;  font = A3B4 (small);
    8BE4("SHIPS" DS:97BA, 159, 187);             /* x 148 */
    C71B();  DC8E(enemy, roman);                  /* bars + MEN, ship totals (both set clip 199, leave 185) */
    B8E3(1); B8E3(0);                             /* Roman ship, then the enemy ship on top */
    691B(0, 0, 319, 199);                         /* screen -> work */
    6887();                                       /* present; target = work */
}
```
Quirk (D): on the re-engage call the clip y1 is already 185, so the black line, the panel fill and "SHIPS" are
clipped away; the panel left on the screen from before survives, so nothing visible changes.

## 5. The duel loop B248 (D)

```c
for (;;) {                                         /* di = 0 while the duel goes on */
    if (di) goto outcome;
    C382 = C4F8(enemy pos, Roman pos);             /* distance */
    C371 = C641(&C372, &C376);  C370 = (C371 - 8) & 15;   /* bearing */
    key = C366();                                  /* 4 = 'r' (retreat) */
    B6A0();                                        /* enemy AI (dice every pass) */
    C3D3(1); C3D3(0); C422(1); C422(0);            /* turn, speed (music on Roman speed change) */
    B7FA(1); B7FA(0);                              /* move + oar frame */
    if (!mode) mode = D242();                      /* keep inside x 15..304, y 15..170; 4 = Roman at the edge, 8 = enemy */
    CF7F(1); CF7F(0); CFDD();                      /* hull rects; which ship is in front (C384) */
    si = CD96();                                   /* ram: 1 = enemy rammed... (95D2 bits), boarding trigger */
    CB11(1); CB11(0); C92B(1); C92B(0); C9A3(1); C9A3(0);   /* fire-pots, arrows */
    if (0183(8) > C386) { 00D7(8); DA50(enemy, roman); C386 = (rng_roll(9) + 2) * 72; }   /* fleets, unseen */
    if (01AE() > 2) {                               /* ---- a frame (into the work buffer) ---- */
        00F8();
        W(0x9AFA) = 9; 7E74(0, 0, 319, 185);       /* 1. sea */
        B96A();                                    /* 2. both ships */
        si |= BA7A(1); si |= BA7A(0);              /* 3. corvus (Roman only in practice), hit -> boarding */
        CCDE(1); CCDE(0);                          /* 4. fire-pot / splash, arrow volley */
        C71B();                                    /* 5. panel: bars, MEN or the speed word */
        DC8E(enemy, roman);                        /* 6. panel: ship totals */
        6887();                                    /* 7. present */
    }
    if (C38F == 0 || C38E == 0) { di = -1; continue; }          /* a flagship sank */
    if (si) { 5D29(0x10); BB7E(si, enemy, roman); di = -1; continue; }   /* boarding (cn9d) */
    if (key) { di = -1; mode = key /* 4 */; continue; }         /* retreat */
    if (mode & 4) { re-engage prompt (5.6) }
    else if (mode & 8) di = -1;                                  /* the enemy admiral flees */
}
```

### 5.1 B96A: the two ships (D; layering V by render)

b = C384 ^ 1 (the ship behind), f = C384 (the ship in front). Order:
1. oars(b), 2. oars(f), 3. hull(b), 4. flames(b) if C3A0[b], 5. hull(f), 6. flames(f) if C3A0[f].

- oars(s): frame k = 93E6[h] + C390[s] = 7*h + oar frame (0..6); h < 8: sheet oars1 frame k; h >= 8: oars2 frame
  k - 56 (`C3A8 + 14*k - 0x310`). Drawn at (C372[s*2], C374[s*2]) = the ship's (x, y).
- hull(s): frame h of the side's ship sheet (C3A2[s]: side 0 = 980C[C37A], side 1 = 980C[C37B]) at (x, y).
- flames: CD4B(s) (5.4).

B8E3(s) (used by BEAE, BB7E, D3F2) draws one complete ship: oars(s), hull(s), flames(s).
Render check (V, render only): with this indexing, hull and oars line up for all 16 headings of SML/MED/LRG.

The oar frame C390[s] advances (0..6, wraps) once per move step (B7FA) while the gear is not 0; a stopped ship keeps
its frame. D3F2 sets 6 for the survivor and 3 for the wreck.

C384 (CFDD, every pass): f's "bow point" = pos[f] + (C3B6[16*type+h], C3B8[16*type+h]) (COLLIDE.LZW tables); if it
is not inside b's hull circles (D070) and b's point is inside f's, C384 = b. So the ship whose bow overlaps the other
is drawn behind it.

### 5.2 BA7A(s): the corvus (D; the sprite looks like a gangplank, V render)

```c
if (!C3AA[s]) return 0;   C3AA[s]--;               /* 20 frames after Enter / '+' (C366: C3AB = 0x14) */
h = C388[s]; k = 16*C37A[s] + h; rel = (h - C370[s]) & 15;   /* C371 = C641 = bearing Rome -> enemy, C370 = C371 - 8 */
if (rel in 1..7)  { f = 0x3A + 95D4[h]; x = pos.x + 95F4[k]; y = pos.y + 9624[k]; }
else if (rel in 9..15) { f = 0x3A + 95E4[h]; x = pos.x + 9654[k]; y = pos.y + 9684[k]; }
else { C3AA[s] = 0; return 0; }                    /* enemy dead ahead / astern: corvus withdrawn */
8E62(bombs + 14*f, x, y);
hit = D070(x + 96B4[f-0x3A], y + 96BC[f-0x3A], s ^ 1);   /* tip: (0,-14) (10,-10) (16,0) (10,10) (0,14) (-10,10) (-16,0) (-10,-10) */
return hit ? 95D2[s] : 0;                          /* 95D2 = {1, 2}: Roman corvus -> 2 */
```
Enemy corvus: B6A0 calls BA7A(0) (b7f0) but C3AA[0] is only ever written 0 (BE0F) or decremented, so the enemy never
shows a corvus (D). Perhaps on purpose (the corvus was the Roman weapon); recommend leaving it.

### 5.3 CCDE(s): projectiles (D)

```c
if (C392[s]) {                                     /* fire-pot */
    f = 98BE[s]; if (C392[s] != 1) f += C3CC[s];   /* in flight: 98BE alternates 0 / 0x10 -> frames h or 16+h */
    8E62(bombs + 14*f, C3D0[2s], C3D2[2s]);         /* C392 == 1: splash frames 0x33..0x39 (51..57) */
}
if (C3BA[s]) 8E62(bombs + 14*(0x20 + C3CE[s]), C3D8[2s], C3DA[2s]);   /* arrow volley, frames 32..47 by direction */
```
Fire-pot flight: one step per 7 ticks of 15 px along the ship's heading at launch; range = distance minus a lead
term, at least 90 px (C846); leaves the field (x <= 0, x >= 319, y <= 0, y >= 185) -> gone. At the end of the range
it walks the last pixels with a hit test: hit -> no splash, the target burns (C3A0 = rng(50) + 10) and loses strength
98C0/98C4[difficulty] and men; miss -> splash 98BE = 0x33..0x39, one frame per 7 ticks.
Arrows: launched automatically by both sides when distance < 90 and the own men > 0 (C92B), 6 px per 9 ticks toward
the bearing at launch, range max(distance - 10, 7); hit test once the range is used up, then one more step on
screen.

### 5.4 CD4B(s): flames on a ship (D, **side effects**)

```c
C3A0[s]--;                                       /* burn counter (10..59 from a fire-pot hit) */
if ((C3A0[s] & 15) == 0) C38E[s]--;              /* strength -1 at every multiple of 16 (also at 0) */
98E0[s] = (98E0[s] + 1) % 3;
8E62(bombs + 14*(0x30 + 98E0[s]), x, y);         /* flame frames 48..50 at the ship's centre */
```
Called once per frame per burning ship (from B96A / B8E3), so the burn damage is paced by the frame rate.

### 5.5 C71B: the panel, bars and men (D; side effects)

```c
clip y1 = 199 (C0E0, C1EA, 9AC6);
if ((s8)C38E <= 0) { C38E = 0; C37E = 0; }      /* side effect: a sunk enemy flagship loses its men */
w = (0x73 * (s8)C38E) / 60;                      /* signed byte imul, idiv: 115 px = strength 60 */
W(0x9AFA) = 4;  7E74(0, 190, w, 196);            /* enemy bar, red (colour 4 = 42,0,0) from the left */
W(0x9AFA) = [6346]; 7E74(w, 190, 115, 196);      /* rest grey 26 (also repaints column w) */
if ((s8)C38F <= 0) { C38F = 0; C380 = 0; }
w = (0x73 * (s8)C38F) / 60;
W(0x9AFA) = 4;  7E74(319, 190, 319 - w, 196);    /* Roman bar, red from the right */
W(0x9AFA) = [6346]; 7E74(319 - w, 190, 204, 196);
7E74(119, 193, 199, 199);                        /* centre text area, 26 */
W(0x9B69) = 0;
if (C37C == 0) {
    8BE4("MEN" DS:98BA, 159, 193);               /* x 151 */
    8A9E(C380, 174, 193);                        /* Roman men, left-aligned */
    8AC8(C37E, 144, 193);                        /* enemy men, right-aligned at 144 */
} else { C37C--; 8BE4(9870[C38D], 159, 193); }   /* the Roman speed word for 10 frames */
clip y1 = 185;
```
So the red bars are w columns wide: x 0..w-1 (enemy) and 320-w..319 (Roman). Full strengths 8EE2 = 30 / 45 / 60
(Trireme / Quinquereme / Galleon) -> 57 / 86 / 115 px. Men 8EDE = 50 / 100 / 200 (the Roman's halved when no legion
is aboard, B20E).
Speed words 9870[level]: 9830 "STOPPED", 9838 "CRUISING SPEED", 9847 "BATTLE SPEED", 9854 "ATTACK SPEED",
9861 "RAMMING SPEED" (x 143 / 127 / 133 / 132 / 128). C37C = 10 is set by C422 only when the Roman level becomes
1..4, so "STOPPED" shows only if the ship stops within 10 frames of a change (D).

### 5.6 DC8E(enemy, roman): ship totals (D)

C404 = sum of the Roman fleet's 3 ship words, C406 = the enemy's (the flagships are not in them during the duel:
B174 / B1DE took one ship of the flagship type out; B44A / B45F put a surviving flagship back).
```
clip 199; W(9AFA) = [6346]; W(9B69) = 0;
8AF2(C404, 174, 187, 25):  erase (174,187)-(199,193), Roman total left-aligned at x 174
8B2C(C406, 144, 187, 25):  erase (119,187)-(144,193), enemy total right-aligned at 144
clip 185
```
(Y0 = yoff of ' ' in the small font = 0, height 6.) The numbers change on screen whenever a background fleet round
(DA50, every 2..10 s) sinks ships: the player can watch the fleets fight during the duel.

### 5.7 Panel layout, VGA (D; reference render in section 13)

```
y 186        black line, full width (BEAE only)
y 187..199   grey 26
y 187        enemy ships (right-aligned to x 144) | "SHIPS" (x 148) | Roman ships (from x 174)
y 190..196   enemy strength bar x 0..115 (red from the left) | Roman bar x 204..319 (red from the right)
y 193        enemy men (right-aligned to 144) | "MEN" (x 151) or the speed word | Roman men (from 174)
```
Left = enemy, right = Rome. Text colour 0, small font.

### 5.8 The re-engage prompt (b3b4, mode & 4: the Roman ship ran into the edge pointing outward) (D)

```
5D29(0x0D); 67B6(); 6837();                       /* screen = work; draw on the screen */
W(9AFA) = 9; 7E74(0, 0, 319, 185);                /* empty sea (clip 185) */
font = A05C; 5C20("The tides of battle sweep\nyour flag ships apart.\nDo you wish to re-engage?" DS:935A)
Yes: mode = 0; BE0F(); BEAE(enemy, roman); continue   /* ships back at their start positions, damage kept */
No:  di = -1  (outcome with mode 4: no fleet battle if both flagships float -> "You retreat to safety ...")
```
The No path leaves font A05C set (nothing between b3db and b479 sets A3B4), so b479's C71B prints the MEN line in the
8-px box font: 1990 bug, 9.2.
Mode 8 (the enemy ran into the edge outward): no prompt, the duel ends; the outcome sets C38E = 0 (b499) and the
texts say the admiral fled.

## 6. Boarding: BB7E(si, enemy, roman) (D)

Trigger: a ram (CD96) or a corvus hit (BA7A) while both flagships float. si bit 1 = the enemy rammed, bit 2 = Rome
rammed / the Roman corvus hit (95D2 = {1, 2}).
```c
04ED();  C37C = 0;  011F(36);                     /* key flush, 0.49 s pause (frozen picture) */
if (si & 2) C36E += 10;  if (si & 1) C36F += 10; /* the boarded side gets it: 1990 bug 13.2 (section 18) */
C390 = C391 = 6;  C386 = 72;  cx = (C376 + C372) / 2;  cy = (C378 + C374) / 2;   /* C division (toward 0) */
frames = 20;
while ((C37E && C380) || frames) {
    while (0183(2) <= 8) if (!((C37E && C380) || frames)) goto done;   /* 9-tick frames */
    00D7(2);  if (frames) frames--;
    if (0183(8) > C386) { 00D7(8); C386 = rng_roll(108);
        e = BD41(0, C36E); r = BD41(1, C36F);       /* per man: rng(100) < q && rng(100) < 25 */
        C380 = max(C380 - e, 0); C37E = max(C37E - r, 0); }
    W(9AFA) = 9; 7E74(0, 0, 319, 185);
    B8E3(1); B8E3(0);                             /* Roman, then enemy on top; oars at rest (frame 6) */
    8E62(smok + 14*rng_roll(3), cx, cy);          /* smoke between the ships (SMOK 54x45, 60x51, 65x54) */
    C71B(); 6887();                               /* MEN numbers fall */
}
done:
if (!C37E) { C38E = 0; [96D2] = [96CA]; }
if (!C380) { C38F = 0; [96D2] = C38E ? [96C8] : [96CC]; }
67B6(); 6837(); font = A05C;
AC60 = ((si & 2) ? [96C4] : [96C6]) + [96D0] + [96D2];   /* [96CE] = the first part, unused */
59D4(AC60);  font = A3B4;
```
Texts: 9406 "Your sailors cheer lustily as they swing\nacross and board the enemy flagship.\n" / 9455 "The enemy swarms
across the gangplanks.\n"; then 95A9 "After fierce combat aboard both ships,\n"; then 94DA "your axemen chop gaping
holes in the enemy\nvessel, allowing Neptune to claim the hulk." / 947E "the last of your men are thrown overboard,\nand
the scurrilous enemy scuttles your flagship." / 9531 "they are each too damaged to stay afloat,\nand the two flagships
sink together, bound\neternally in Neptune's playground."
The box is drawn with the clip y1 still 185 (BB7E does not set 199); 4-line boxes end well above 185 (TEXTBOX_NOTES),
so nothing is cut (D/G). After the box: di = -1 -> the outcome.

## 7. The end of the duel (D)

```
b419: di = 0; 5D29(0x0D);                                /* cn9a */
      bits: 1 = Roman flagship C38F == 0, 2 = enemy C38E == 0 (counters C08E / C08A += 1)
      put a surviving flagship back into its fleet's count (b44a, b45f)
      C37C = 0; C71B();                                  /* panel, clamps <= 0 to 0 */
      67B6();                                            /* screen -> work */
      D3F2(bits); C312(); D60C(bits);                    /* sinking; C312 frees ships, oars, bombs, smoke, collide */
      67B6(); 6837();                                    /* target = screen */
      if (mode == 8) C38E = 0;
      if (C38F == 0 || C38E == 0) { while (!DA50(enemy, roman)); DC8E(enemy, roman); }   /* invisible fleet battle */
      outcome from the two fleets' totals; B59F(...)     /* message box */
      C34F(): 5D20, 5E1A(0x0D..0x10)
```

### 7.1 D919(s) and D97C(s): rectangles (D)

- D919(s): the "sink area" = pos + DS:9932[4*type] offsets, y1 clamped to <= 185:
  Trireme (-28,-19)-(+19,+17), Quinquereme (-27,-21)-(+21,+20), Galleon (-28,-18)-(+25,+18).
- D97C(s): union (81EA) of the hull frame rect and the oar frame 6 rect, each `(x-xhot, y-yhot)-(x-xhot+w, y-yhot+rows)`
  (one pixel wider and taller than the sprite; literal).

### 7.2 D3F2(bits): the survivor backs off, the wreck turns broadside (D)

Nothing for bits 0. Bits 3: 67E4; sea (0,0,319,185) in the work buffer; return. Otherwise s = the sunk side (bit 0 ->
1 Roman, else 0 enemy), o = the other:
```c
C390[o] = 6; C390[s] = 3;                        /* oars: survivor at rest, wreck frame 3 */
R_s = D919(s); R_o = D97C(o);
dir = (C388[s] > 4 && C388[s] < 12) ? -1 : +1;   /* turn the short way toward heading 4 (east) */
(cx, cy) = pos[s]; if (817C(0, R_o, R_s)) (cx, cy) = ((pos[s] + pos[o]) / 2);   /* idiv, toward 0 */
done = 0;
while (!done) {
    00F8(); 7E74(0, 0, 319, 185);                 /* sea, colour 9; target = work */
    if (817C(0, R_o, R_s)) {                       /* still overlapping: the survivor backs water */
        pos[o] += (9958[h_o], 9968[h_o]);          /* x: 0,-1 x7, 0,+1 x7; y: 1,1,1,1,0,-1 x7,0,1,1,1 */
        if (--C390[o] < 0) C390[o] = 6; if (C390[o] == 5) C390[o] = 3;   /* oar frames 3,2,1,0,6,3,... */
        R_o = D97C(o);
        overlap = 1;
    } else overlap = 0;
    if (C388[s] == 4) { if (!overlap) done = 1; } else C388[s] = (C388[s] + dir) & 15;
    B8E3(o); B8E3(s);                              /* flames continue (CD4B, see 9.1) */
    8E62(smok + 14*rng_roll(3), cx, cy);           /* DICE every frame */
    6887();
    while (01AE() < 14) ;
}
C390[s] = 6;  67B6(); 6837(); 67E4(); 7E74(0, 0, 319, 185); B8E3(o);   /* work buffer: sea + survivor only */
```
The smoke position is computed once, before the loop.

### 7.3 D60C(bits): the sinking frames and the sharks (D; frame counts V from the sheets)

```c
if (!bits || !0F7B(8, &arch)) return;
R_R = D919(1); R_E = D919(0);
if (bits & 1) { B(0x992C) = 9952[C37B];  /* "xsink" -> "ssink"/"msink"/"lsink" */
                SR = 163B(DS:992C); if (!SR) { 59D4("Failed to Load Human Sinking" DS:9978); return; }
                shark0 = (R_R.x0, R_R.y1 + 4 < 185 ? R_R.y1 + 4 : R_R.y0 - 2), frame AAC6 = 0; }
if (bits & 2) { SE = (SR && C37A == C37B) ? SR : 163B(name for C37A) ("Failed to Load Computer Sinking" DS:9995);
                shark1 = (R_E.x0, R_E.y1 + 4 or R_E.y0 - 2), frame AAC7 = 1; }
SH = 163B("shark" DS:99B5); 11A9(&arch);
67E4(); if (SR) 8E62(SR+0, Roman x, y); if (SE) 8E62(SE+0, enemy x, y);
680F(); 6973(0, 0, 319, 199); 67E4(); W(9AFA) = 9;
for (n = 0, doneR = doneE = 0; !(doneR && doneE); n++) {
    00F8();
    if (SR) { if (!8238(shark0, R_E)) D89B(0);
              if (n < 994A[C37B]) { 7E74(R_R); 8E62(SR + 14*n, Roman x, y); } else doneR = 1; } else doneR = 1;
    if (SE) { if (!8238(shark1, R_R)) D89B(1);
              if (n < 994A[C37A]) { 7E74(R_E); 8E62(SE + 14*n, enemy x, y); } else doneE = 1; } else doneE = 1;
    6887();
    while (01AE() < 14) ;
}
free SR (if loaded here), SE (if loaded separately), SH.
```
Sinking frames 994A: SSINK 22, MSINK 27, LSINK 24 (V: the sheets have exactly 22 / 27 / 24 frames), at 5.2 fps
(4.2 / 5.2 / 4.6 s, plus one frame after the last). Error paths return with the archive still open (1990 leak,
cannot happen in the port).

D89B(k), the shark (SHARK sheet: 8 frames, 2x1 .. 8x3 px, lut 28):
```c
if (!SH) return;
7E74(sx[k] - 5, sy[k] - 3, sx[k] + 4, sy[k] + 2);     /* erase 10x6 in colour 9 */
sx[k]++;                                               /* swims right, 1 px per frame (5 px/s) */
8E62(SH + 14*AAC6[k], sx[k], sy[k]);
if (++AAC6[k] == 5) { if (rng_roll(10)) AAC6[k] = 2; }   /* DICE: 9 in 10 keep circling (frames 2,3,4) */
else if (AAC6[k] == 8) AAC6[k] = 0;                      /* 1 in 10: frames 5,6,7 (dives), then surfaces 0,1 */
```
A shark whose position lies inside the OTHER ship's sink rect is neither drawn nor moved for that frame (so it stays
there for good).

## 8. The result message B59F (D; texts V from DS)

`font = A05C; clip 199; 04ED; AC60 = A + " met the " + B + " on the gently\nrolling waves of " + sea + ".\n\n" +
flavour + result; 59D4(AC60)`, A = W(1100 + 2*Roman fleet) ("Fleet I/II/III"), B = W(1100 + 2*enemy)
("Carthaginian Fleet", "Aegyptian Fleet", "Macedonian Fleet", "Marauding Fleet"), sea = W(5222 + 2*zone of the enemy
fleet) ("Mare Nostrum", "Mare Aegyptus", ...). Flavour and result tables: NAVAL_INDEX section 6. Drawn on the screen over
the last battle picture (with the final SHIPS numbers).

## 9. 1990 bugs and quirks seen from the screen side

### 9.1 A burning wreck is "raised": the fleet battle is skipped (D; frequency G)

D3F2 draws the sunk flagship every frame with B8E3(s), and B8E3 calls CD4B while the ship's burn counter C3A0[s] is
not 0. C71B (b479) has just clamped the wreck's strength to 0; CD4B then decrements it to -1 (0xFF) when the counter
reaches a multiple of 16 (or 0):
```
b955: cmp byte ptr [bx - 0x3c60], 0x0     ; B8E3: burning?
b960: call 0xcd4b
cd52: dec byte ptr [si - 0x3c60]          ; C3A0[s]--
cd56: test byte ptr [si - 0x3c60], 0xf
cd5d: dec byte ptr [si - 0x3c72]          ; C38E[s]--   0 -> FF
...
b49e: cmp byte ptr [0xc38f], 0x0 / je 0xb4ac     ; fleet battle only if a flagship is exactly 0
b4a5: cmp byte ptr [0xc38e], 0x0 / jne 0xb4c6
```
A flagship sunk while burning (C3A0 = rng(50) + 10 after any fire-pot hit, falling one per frame) whose counter crosses
a multiple of 16 during D3F2's 1..9+ frames gets strength FF. Then neither test sees 0, the fleet battle is skipped,
both fleets still have ships, and the outcome is 4: "You retreat to safety with your remaining ships." even though
the flagship bits (computed before D3F2) say a flagship sank. For Erik's case (enemy flagship sunk by fire):
"Neptune and Mars smile, and you destroy the enemy flagship, sending your foes to a watery death. / You retreat to
safety with your remaining ships." No victory, and B0E0 then reports the way blocked. (For a burning Roman wreck the
same bug spares the Roman fleet a battle.) The survivor burns on too: a surviving flagship at strength 1..3 can reach
0 during D3F2 and then counts as sunk for the fleet battle (DA50's 99C8 / 99CE tables), although the text and the
ship count treat it as afloat. Fix (default): CD4B must not decrement a strength that is already <= 0
(or B49E / B4A5 test the bits in di); --original keeps it. G: how often, until someone counts D3F2 frames in a run.

### 9.2 Others (D unless marked)

- C18C (palette restore) is dead code: the cycled colours are never put back (4.3). G for any visible effect later.
- The enemy corvus (BA7A(0) from B6A0) never appears: C3AA[0] is never set (5.2).
- "STOPPED" is shown only within 10 frames of a speed change (5.5).
- BEAE's line / fill / "SHIPS" are clipped away on re-engage (4.4), invisible.
- The boarding message box is drawn with clip y1 = 185 (6), invisible for its sizes.
- 0183's 16-bit-wrap saturation (3.1), harmless.
- After a boarding the target is the screen (BB7E ends with 6837), so D3F2's first frame is drawn on the screen and
  at once covered by its own 6887 (which copies the work buffer = the saved screen); from the second frame on the
  target is the work buffer. One sinking frame (192 ms) is lost after a boarding; a port that follows the targets
  literally reproduces it (D/G for the visual).
- **(D, V headless) "No" to re-engage leaves the big font set.** b3db..b3e8 copies A05C to 9B56 for the yes/no box 5C20;
  on No (`b3f3 je b39a`) the loop ends, and b474/b479 `C37C = 0; call C71B` prints "MEN" and both crews with the
  8-px glyphs, on the screen (target from b3c3's 6837): the line reads e.g. "100 MEN 91" from y 194 and is cut off at
  y 199, under the result box until it is clicked. BB7E restores A3B4 after its own box (bd2c), so this is an oversight.
  Fixed in the port by default (naval.c: A3B4 set again after No); --original keeps the big MEN line.
- "Failed to Load ..." boxes would be drawn in the small font (font A3B4 still set) and leave MNAVAL open; unreachable
  in the port.

## 10. Input (D)

During the duel, every pass, C366:
```c
s = 0;
if (B(0xA2)) {                                      /* held direction bits from the INT 9 hook */
    if (B(0xA2) & 8) C395 = +1;                     /* Up / keypad 8:      speed level up   */
    if (B(0xA2) & 4) C395 = -1;                     /* Down / keypad 2:    speed level down */
    if (B(0xA2) & 2) C397 = -1;                     /* Left / keypad 4:    turn port (heading - 1) */
    if (B(0xA2) & 1) C397 = +1;                     /* Right / keypad 6:   turn starboard (heading + 1) */
}                                                   /* Home/PgUp/End/PgDn set two bits; keypad 5 (0x80) does nothing */
switch (03AA()) {                                   /* one key per pass, ASCII, extended keys swallowed */
    case 'r':  s = 4; break;                        /* retreat: lower case only ('R' = 0x52 does nothing) */
    case 0x0D: case '+': C3AB = 0x14; break;        /* Enter / '+': lower the corvus for 20 frames */
    case 0x10: case ' ': C846(1); break;            /* Insert (03AA maps scan 52h to 10h) / Space: fire-pot */
}
return s;
```
The requests C395 / C397 stay set until the speed / turn timer of the Roman ship fires (73..145 ticks), then apply
once and clear; holding an arrow simply keeps requesting. A tap seen by one pass is therefore remembered (one step).
Speed level 0..4 (C38D), C422 refuses to go below 1 until the oars are at frame 6. No Esc, no pause, no mouse in the
duel. Other input points: the flagship menu (54C8: mouse or hotkeys T/Q/G, Esc = no battle), the intro (any ASCII key
or a mouse button held), the message boxes 59D4 (click / Enter / '+'), the yes/no box 5C20.
`04ED` (key flush) runs before the boarding pause and before the result box only.

Port notes: sdl2_platform.c records keysyms < 256 only, so the arrows (held) and Insert never reach the port; the
naval battle needs the arrow / keypad keys as held bits ([A2] form) and Insert as 10h. SDL gives the unshifted keysym:
'+' arrives as '=' (battle_ui.c accepts '=' for it); 'r' must be the lower-case key without Shift / Caps Lock to match
the original (the port sees no modifiers: accept 'r' only).

## 11. Music (D; names V from DS:3EF0, files from MNAVAL)

Table DS:3EF0 (4 bytes per song: name, archive): 0x0D "cn9a", 0x0E "cn9b", 0x0F "cn9c", 0x10 "cn9d", archive 8 =
MNAVAL; list DS:9704 = {0, 0D, 0E, 0F, 10}.

| when | call |
|---|---|
| set-up (BD8A) | 5D20 stop; 5E1A(1) unload the map music |
| BF50 end | 5D73(0D..10) load; 5D29(0D) cn9a during the intro and the start |
| Roman speed change applied (C422, side 1) | gear != 0: 5D29(9704[level]) = cn9a cruising, cn9b battle, cn9c attack, cn9d ramming; gear 0: 5D20 (silence while stopped) |
| boarding (b388) | 5D29(10) cn9d |
| re-engage prompt (b3ba), duel over (b41c) | 5D29(0D) cn9a |
| end (C34F) | 5D20; 5E1A(0D..10) |

So the music follows the oar beat of the Roman galley (G: the four pieces are the drum tempos). The port does the DS
bookkeeping only (map.c music_play_5D29 / nego.c's 5D20 / 5E1A), as elsewhere.

## 12. Sprite sheets (V: decoded from the archives with the LZSS of tools/extract_data.py; uses D)

| sheet | archive | DS handle | frames | size (first) | lut / trans | use |
|---|---|---|---|---|---|---|
| GALLEYS.LZW | NAVL (9) | work buffer | 320x200 raw | - | CMAP | intro picture (water colours 87..8C cycled) |
| OARS1 / OARS2 | NAVL | C3A6 / C3A8 | 56 / 56 | 31x42 | 32 / 13 | oars, frame 7h + phase (h 0..7 / 8..15) |
| SML / MED / LRG | MNAVAL (8) | C3A2 enemy, C3A4 Roman | 16 each | 13x39 / 17x43 / 19x45 | 28 / 0 | hulls by heading |
| BOMBS | MNAVAL | C3AE | 66 | 4x8 .. 17x12 | 0 / 13 | 0..31 fire-pot (2 phases x 16 headings), 32..47 arrow volleys, 48..50 flames, 51..57 splash, 58..65 corvus (8 directions) |
| SMOK | MNAVAL | C3AC | 3 | 54x45, 60x51, 65x54 | 31 / 0 | smoke (boarding, D3F2) |
| FLAG | MNAVAL | C3BC | 4 | 30x12 | 15 / 3 | intro pennant |
| SSINK / MSINK / LSINK | MNAVAL | locals | 22 / 27 / 24 | 43x35, 45x37, 47x34 | 28 / 0 | sinking (name "xsink" at DS:992C, first letter from 9952 "sml") |
| SHARK | MNAVAL | 9956 | 8 | 2x1 .. 8x3 | 28 / 0 | sharks |
| COLLIDE.LZW | MNAVAL | C3B0..C3B8 | 816 bytes | - | - | collision circles (x, y, r: 3 types x 16 headings x 5) + bow points (3 x 16) |

Descriptor (14 bytes): +0 w, +2 xhot, +4 yhot, +6 bytes per row, +7 rows, +8 transparent nibble, +9 lut, +A offset
(counted from 2 bytes before the data; nego.c's sheet_163B reads them right).

## 13. Reference renders (render only, from the decoded rules; superseded by the captures of section 18)

Scratch: `C:\Users\erikk\AppData\Local\Temp\claude\bfb5522be8ee045d\c4bf0f5f-57f9-4ea6-80b3-e1aa05bf0592\scratchpad\fleet\`
`render.py` (intro_ref.png: GALLEYS + pennant frame 1; battle_first_ref.png: BEAE for Quinquereme vs Quinquereme,
example totals 12 / 50), `grid.py` (grid_MED_x3.png: all 16 headings with oars, and the 66 BOMBS frames). Palette =
Centurion-native\cmap.bin, LUTs from the runtime DS.

## 14. Headless deterministic mode (proposal, after battle_ui.c; G)

- Keep the timer in the DS: [42]/[44] = a virtual tick counter; the slot helpers literal (00D7, 00F8, 00FF, 0183,
  011F). Window mode: after each duel pass advance [42] by one and sleep until real time reaches it; headless: just
  advance (3.3). Busy-waits (D3F2 / D60C 14, BB7E 9, 011F 36, the intro's 4 and 5) advance the counter to their
  threshold.
- Run every drawing call (C71B, B96A / B8E3 / CD4B, BA7A, CCDE, DC8E, D3F2, D60C, D89B, BB7E's smoke) also in headless
  mode, into g_wrk / g_scr buffers: they change the DS and roll dice (3, 5.4, 5.5, 7.2, 7.3). Only the presentation
  (platform) is skipped.
- Input per tick: a script needs (tick, ASCII key) events for 03AA ('r', Enter, '+', Space, 10h) and (tick range,
  [A2] bits) for the held arrows; e.g. `fleet <tick> key <k>` and `fleet <tick> hold <up|down|left|right|none>` lines
  next to the existing click/key/shot lines. The flagship menu, the intro, the boxes and the yes/no take the ordinary
  script clicks / keys (the intro: every poll of its loop is one tick, so the pennant animates; a key ends it).
- Shots: a `--naval-shots` mode like battle_ui_shots (frame N of the duel, a boarding frame, a D3F2 / D60C frame, the
  result box) for picture checks.

## 15. Erik's report, from the screen side (D)

- The duel ends at the first flagship sinking, a boarding result, 'r', "No" to re-engage, or the enemy admiral
  running into the edge. Throughout the duel the two fleets fight in the background (DA50 every 2..10 s, b2f4..b321),
  visible only as the SHIPS numbers in the panel. The rest of the fleet battle after the duel (b4ac loop) is computed
  instantly with no picture; the final SHIPS numbers are printed just before the result box.
- The result texts are chosen by B59F as NAVAL_INDEX section 6 says: "witless commander ... his more competent mates
  have swept the seas clean of Roman vessels" is the deliberate "lost although your flagship survived" text, not an
  inverted test (B5F9 `test si, 1` on the flagship bits).
- "In spite of their admiral's bath, your fleet prevails" (won, Roman flagship sunk): "their" can read as the Roman
  crews' admiral ("your fleet ... their admiral"); G: probably not a text bug.
- New, in the drawing: section 9.1 turns some flagship victories into "You retreat to safety" (no win). It does not
  explain losses; whether the fleet battle itself is lopsided is for the fleet-battle notes. Facts from DA50 seen while
  reading (D, for that agent): Roman hit chance per ship 99BC[3*difficulty + type] (5/8/12 % .. 2/3/5 %, `<=`, so +1 %),
  with the Roman flagship sunk 99C8 (lower); enemy per ship of its flagship type only 99C2[type] = 3/5/8 % regardless
  of difficulty (sunk: 99CE 1/2/4 %); flagship bonus rolls Roman 99D4[difficulty] = 8/6/4/2 rolls, enemy 4 rolls, each
  at strength/4 %; the enemy fleets in every snapshot are one type only (Carthage 50 Quinqueremes, Aegyptus 100
  Galleons, Macedonia 30 Quinqueremes), and DA50 / DE51 use only the enemy's flagship-type count (C406 = 110E[6*f +
  C37A]), which is harmless while enemy fleets have a single type (G: a fleet with two types would never lose its
  other ships and the post-duel loop could not end).

## 16. Open questions

- (Answered 10 Nov, section 18: captures exist; the duel frame, panel, intro, boarding, sinking and shark pictures are V.
  Not yet seen on a captured screen: fire-pots in flight, splashes, flames, the corvus, the speed word, the re-engage
  prompt, the BEAE start screen itself.)
- Whether anything after the battle reloads the palette (C18C dead, 4.3).
- The pass rate of the 1990 game on a period machine (how many B6A0 dice per tick) cannot be recovered; the port
  needs a decision (3.3).
- 899C's clipping is taken from the gamectl.c note (glyphs through 8E62), not re-read for this file.
- OVL0 file 0x6725 also references the small font block A3B4: which section and what for.

## 17. Easter eggs and jokes (for Erik)

- The result texts: "the witless commander of the enemy fleet, and his more competent mates", "In spite of their
  admiral's bath", "sent to Neptune's locker without a sword", "together forever in a deadly embrace", "the gently
  rolling waves of <sea>" (in the middle of a sea battle).
- The boarding texts: "Your sailors cheer lustily", "the scurrilous enemy scuttles your flagship", "your axemen chop
  gaping holes ... allowing Neptune to claim the hulk", "bound eternally in Neptune's playground".
- The ship choice offers a "Galleon" (a 16th-century ship) beside the Trireme and the Quinquereme.
- Sharks circle every sinking flagship; one time in ten a shark dives (frames 5..7).
- The galley music speeds up with the rowing: cruising, battle, attack, ramming speed each have their own piece
  (cn9a..cn9d), and the music stops when you stop rowing.
- A second, tiny font exists only for the naval panel.
- The Roman corvus (the boarding bridge Rome really used in the First Punic War) is the Enter key, and only Rome has it.

## 18. Verified against the real game (DOSBox-X captures of 10 Nov; V)

Erik played two naval battles in DOSBox-X on C:\work\cent_play while a recorder stopped the game at breakpoints and
saved the 640K memory (DS = 1686, the work buffer 2687:0000) and the VGA screen (indexed pixels, no palette: compared
as palette indexes). Copies and the exact commands: `tools/dosbox/results/naval/README.md`; tools
`tools/cmp_naval_frame.py`, `tools/cmp_naval_resume.py`; test hooks `centurion --naval-frame` / `--naval-resume`
(naval.c's end). All results with `--original`, and the same without it (no 1990 bug the port fixes was on these paths).

A resumed stage starts from the capture's screen and work buffer, and a stage may leave much of them as they were, so a
0 px result alone does not show that the port drew the picture. The tools therefore run each check twice more with the
input screen and work buffer filled with FD and with FE: a pixel that is the same in both runs was drawn by the port
("drawn" below); the rest is the capture passed through. Every pixel the port drew matches, in every row below.

| picture | how | result | drawn by the port |
|---|---|---|---|
| battle 2, the last duel frame before the boarding (BB7E entry) | `--naval-frame ... duel 4 0`: B12A's frame b324..b370 drawn from the snapshot's state | 0 px | 62023; the other 1977 (rows 186..199) are the panel background, the black line and "SHIPS", which only BEAE draws |
| battle 2, the intro (BEAE entry) | set-up resumed from B0E0's entry with the flagship click and 18 ticks of intro (b2_flagship_trireme_intro.txt): the port's own BD8A, BF50 and C105 loop | 0 px; 9AFA left at 80 as in the game | 64000 |
| (the same picture) | `intro K`: the test hook re-types C105's loop body (the 0x80 erase of (38,2)-(68,15), FLAG frame K at (38,5)) | K = 0: 0 px (1, 2, 3, none: 104, 152, 134, 78) | 64000 |
| battle 2, the last boarding frame + b479's panel (D3F2 entry) | resumed from BB7E's entry: 20 boarding frames, smoke rng(3) at the midpoint, with the snapshot's dice and timer | 0 px | 61711 (the rest: the panel strip, rows 186..199) |
| battle 2, the last D3F2 frame (D60C entry) | resumed from D3F2's entry: the enemy backs water 17 px, the Roman wreck turns from heading 2 to 4, smoke | 0 px | 59520 (the rest: the whole panel strip, rows 186..199) |
| battle 2, the last D60C frame + DC8E (B59F entry) | resumed from D60C's entry: SSINK frames, the shark, DA50, DC8E | 0 px | **2332 only**: D60C restores its rects from the work buffer, which is the capture's here |
| (the same picture) | **chained** from BB7E's entry (BB7E, b419.., D3F2, D60C, DA50, DC8E in one run) | 0 px | 62023 (the rest: rows 186..199, BEAE's panel background); this is the real check of D60C's picture |
| battle 2, after the result box (2C27) | resumed from B59F's entry | 93 px = the EXE's mouse cursor at (148,92)-(158,107); the port shows the system pointer instead | **0**: the box is drawn and taken down again, the screen is the input |
| (the same picture) | **chained** from BB7E's entry to B0E0's return | 93 px, the cursor; DS 0 game-state bytes off | 62023, 93 of them differ (the cursor) |
| battle 1, after the battle (2C27) | `after 1 3 0`: BEAE's panel, the end of D3F2, D60C (MSINK, shark), DC8E, from the final state | 93 px = the mouse cursor at (160,109)-(170,124) | 64000 |

Verified by these (V): the duel frame's order and layering (5, 5.1), the hull and oar frames and hot spots, the arrow
volley sprites (5.3: both volleys were in the air), the strength bars, "MEN" and the men numbers, the ship totals and
their places (5.5..5.7), the small font, the clip at 185, the intro's pennant picture and its place (4.3), the boarding
frames (6), D3F2's back-off steps 9958/9968 and its turn (7.2), D60C's frames, the shark's start, frames and erase box
(7.3), DC8E after the fleet battle (7).
The intro (4.3): the pennant picture is V (the `intro` hook), and C105's loop body as the port runs it is V too (the set-up
resume with 18 ticks of intro ends on frame 0 like the game, and the frames come in the decoded order: holding the intro
0..4 ticks gives no frame, 5..8 frame 1, 9..12 frame 2, 13..16 frame 3, 17..20 frame 0, so ++si comes before the draw
and a frame is drawn every 4 ticks of the port). C105's pacing against the game's real time and the water-colour phase
9812..9817 (excluded from the DS comparison as time) stay D: one stop cannot show how many frames the game drew (its
intro ran about 530 ticks; its tick was 5 past timer slot 0 at BEAE's entry, the port's 2: G, DOSBox frame timing).
The shark dice: battle 1's `after` picture runs D60C on the dice left at the END of the battle (after the real D60C and
all DA50 rounds), and its last shark frame hardly depends on them (other dice give 93 px as well, now and then 100), so
battle 1 checks D60C's sprites and places and its dice only by chance. The dice evidence is battle 2's D60C resume (the
dice end as the game's, NAVAL_LOGIC_NOTES 17) and the chained run above.
Not on any captured screen (still D): the speed word, the corvus, fire-pots, splashes, flames, the re-engage prompt,
BEAE's start screen, B59F's result box (2C27's screen is after the box was taken down).
Geometry (`cmp_naval_frame.py ... check`): at BB7E's entry CF7F's rects equal the snapshot's, C641's bearing 2 / 10 too,
and CE7E finds the ships touching (it was a head-on ram). C382 = 32 is not the distance of the stored positions (25): C4F8
ran at the top of the last pass, before both ships moved one step (both move slots = that pass's tick); undoing the step
from the stored fractions allows the pre-move positions (76..77, 84..85) / (56..57, 107..108), and C558 gives 32 for
(77,84) / (56,108) (its rounding: the floor would be 31), so the value is consistent (V for C558's rounding, G which
pre-move position).
