# The mouse cursor (EXE software cursor)

**Status 2026-11-17: ported** (screen.c cursor_move_1BE6 / cursor_show_1CCE / cursor_hide_1D19 / cursor_push_1D55 /
cursor_pop_1D8A with the save-under 86B2 / 8790; called at every site of section 5 in the files that port those routines;
map.c map_startup does B2CF..B2E4; a DS image with the INIT descriptors but [684] = 0, as every .bin the port wrote before
the cursor port, gets [684] = [C0E2] there too (port-only, review 2026-11-17)). Review 2026-11-17: Difficulty / Fine Tuning
(gamectl.c) drew their menus without 5614's push / pop (stale save-under patches: "Legionarne."); fixed, so every 5614 site
is now bracketed (V: h_diff shots = the pre-cursor build + the arrow). V: naval resume BB7E -> 2C27 and B59F -> 2C27 0 px (93 before) and the cursor DS
words equal n010's; `after 1 3 0` 0 px; `--dump-frame province_narbonensis --save snap_back1.bin` 0 px (the arrow clipped
at (310,185) included). Not drawn: the land battle's own screens (battle_ui.c keeps the system pointer there).

Claude, 2026-11-17. Sources: `disasm/EXE_RUNTIME.bin` (segment 0823, fresh llvm-objdump listing with
`--triple=i386-unknown-linux-code16`; the clean listing lacks 0x1C7F..0x1CAE and 0x1DF5), OVL0 sections 1..5 at 0823:B0E0
(OVL0.OVL [0:9A0] [9A0:3EF0] [3EF0:51A0] [51A0:8550] [8550:B3F0]), the static DS (OVL0.OVL file 0xB400 = DS:0000), the INIT
sheet in MINIT.DAT, and every capture in `tools/dosbox/results`. Labels: **D** = decoded from the code, **V** = verified
against a DOSBox capture by a script, **G** = guess. No C code was changed for this note.

Scripts (scratch, `C:\Users\erikk\AppData\Local\Temp\claude\bfb5522be8ee045d\c4bf0f5f-57f9-4ea6-80b3-e1aa05bf0592\scratchpad\finish\cursor\`):
`curs.py` (INIT sheet entry 0 from MINIT.DAT through the OVL0 colour table), `ver1.py` (cursor drawn over a frame, pixel diff),
`find.py` (searches every 64000-byte capture for the sprite), `dsstate.py` (cursor variables of every DS image),
`saveunder.py` (the save-under buffer of the 640K snapshots against their screens), `callers.py` (every call of the routines).

## 1. Summary

- The cursor is a **software sprite drawn by the game into VGA memory** (A000, through the current target descriptor DS:C0D0).
  The mouse driver's own pointer is never shown: the game calls INT 33h only with AX = 0, 3, 4, 5, 6, 7, 8 (no 1/2, no 0Ch
  event handler, no INT 33h in any overlay section) (D).
- **One shape for the whole game**: INIT sheet entry 0 (MINIT.DAT, INITHDR.LZW / INITDAT.LZW), an 11 x 16 arrow, black outline
  (colour 0), white inside (colour 15), hot spot (0, 0) = the tip, transparent nibble 13, colour table 0 (identity). Set once at
  start-up (section 1 B2E4: `1DF5([C0E2])`); 1DF5 is the only writer of [684] in the EXE and in all five sections (D);
  [684] = CE48 (that descriptor) in all 46 run-time DS images of `tools/dosbox/results` (battle, endturn, naval, the three 640K snapshots) (V). No sword, no busy shape.
- **Moved only by the wait loops** (no ISR): 1BE6(x, y) is called from 359B (wait for a click or key), 35DB (wait for release),
  3603 (keyboard mouse), 2900 (buy poll), battle D33A (drag), gladiators BDEA (card grid) (D). There is no idle cursor update.
- **Save-under**: a fixed 24 x 16 byte box (VGA) at the cursor's top-left, in a DOS block allocated at start-up
  (A694:A692 = 3929:0000 in every capture). Saved unclipped, restored clipped (section 4) (D, V).
- **Visibility** [686] (0 / FFFF) plus a stack of 30 bytes at DS:A674 with depth [68A]. All high-level screen drawers bracket
  their work with `1D55(0)` (push, hide) ... `1D8A` (pop, redraw if it was visible) (D). The cursor is **not moved while hidden**:
  the pop redraws it at its old place [680]/[682], not at the mouse ([B6]/[B8]) (V: b2_n010 and b1_n005, see 6).
- Rendering INIT entry 0 at [680]/[682] over the screen without the cursor gives the DOSBox capture **pixel for pixel** in all
  three naval cases that currently differ by 93 px (V, section 6). The sprite has exactly 93 opaque pixels.

## 2. DS variables (DS = 1686)

| addr | size | static OVL0 | meaning |
|---|---|---|---|
| 0680 | word | 00A0 (160) | cursor x where it is drawn (the hot-spot position) |
| 0682 | word | 0064 (100) | cursor y |
| 0684 | word | 0 | near pointer to the shape's 14-byte sprite descriptor (set to [C0E2] = CE48 at start-up) |
| 0686 | word | 0 | visible: 0 = hidden, FFFF = drawn (1D55 stores it as a byte, 1D8A sign-extends it back) |
| 0688 | word | 0 | not referenced anywhere (D) |
| 068A | byte | 0 | depth of the visibility stack (< 30) |
| A674..A691 | 30 bytes | 0 | visibility stack: A674 + depth = the pushed low byte of [686] (00 / FF). Pop does not clear it (n010 has A674 = FF FF with depth 1) |
| A692 | word | 0 | save-under buffer offset (0) |
| A694 | word | 0 | save-under buffer segment (allocated, 3929 in every capture) |
| A696 | word | 0 | save-under width in bytes (VGA 24) |
| A698 | word | 0 | save-under rows (16) |
| C0E2 | word | 0 | INIT sheet entry 0's descriptor (filled by the start-up through the pointer table DS:6350, see map.c map_startup) |
| C0D0..C0E1 | 9 words | | the current drawing target: +0 row table (AAD0, row y = y * 320, V), +2 offset, +4 segment (A000 screen / 2687 work buffer), +6 width 320, +8 height 200, +A..+10 clip x0 y0 x1 y1 |
| 00B2 / 00B6 / 00B8 | words | | mouse buttons / x / y from 0644 (INT 33h AX = 3) |
| 00BA | word | FFFF | a mouse is wanted / present (0 = keyboard mouse in 3603) |
| 00BC / 00C0, 00BE / 00C2 | words | 0 / 13F, 0 / C7 | x min / max, y min / max (06E4 / 0709, also INT 33h AX = 7 / 8) |

The descriptor A692..A699 is a 4-word "picture" record of the same kind 86B2 / 8790 take elsewhere: {offset, segment, width,
rows}.

## 3. Start-up (section 1, D)

```c
/* section 1 B18A (the start-up overlay): ... the INIT sheet loaded from archive 4 ("init", DS:6339) at b276..b2b5, its 14
   descriptors stored through the pointer table DS:6350 (C0E2, C1D6, C108, ...): [C0E2] = entry 0; "init2" -> [C28A] ... */
b2cf: ok &= cursor_alloc_1C7F();
b2dd: mouse_warp_066A(160, 100);
b2e4: cursor_shape_1DF5([C0E2]);           /* the arrow */

int cursor_alloc_1C7F(void) {              /* 0x1C7F (no prologue) */
    W(A698) = 16;                           /* rows */
    if (W(9B26) == 4) {                     /* EGA (planar): off-screen video memory */
        W(A694) = 0xA000; W(A692) = 0x1F40; W(A696) = 3; return -1;
    }
    dx = (W(9B26) == 6) ? 12 : 24;          /* 6 = 4-bit packed modes, else VGA: 24 bytes = 24 pixels */
    W(A696) = dx;
    W(A694) = alloc_863E(dx);               /* INT 21h AH = 48h, dx paragraphs = 16 rows of dx bytes; zero-filled (rep stosw) */
    W(A692) = 0;
    return B(A695) == 0xFF ? 0 : -1;        /* 863E returns AH = FF on failure */
}
void cursor_shape_1DF5(uint16_t desc) { W(684) = desc; }   /* 0x1DF5, ret 2 */
```

The mouse driver is reset in section 1 b1dc (`if ([BA]) { if (!mouse_detect_0632()) [BA] = 0; else [3E40] = 3E2E; }`,
0632 = INT 21h AX = 3533h (vector of INT 33h), 0 if it is null, else INT 33h AX = 0). The cursor is first shown by main at
0x07E9 (`1CCE`) after `map_build(0)`, so it appears at the static (160, 100) and jumps to the mouse at the first 1BE6 of the
map's 359B (D).

## 4. The routines (D, pseudo-C, 0823 addresses)

`hx = W(W(684) + 2)`, `hy = W(W(684) + 4)` = the shape's hot spot (0, 0 for the arrow). `save` = 86B2, `restore` = 8790,
`sprite` = 8E62 (the sprite blitter the port already has: map.c sprite8e62), `vwait` = 84BB.

```c
void cursor_move_1BE6(int x, int y) {              /* args pushed x then y: [bp+6] = x, [bp+4] = y; ret 4 */
    if (W(686) == 0) return;                        /* hidden: the position is NOT updated */
    if (W(680) == x && W(682) == y) return;         /* not moved: nothing (no redraw) */
    int old[2] = { W(680) - hx, W(682) - hy }, nw[2] = { x - hx, y - hy };
    vwait_84BB();                                   /* wait for the vertical retrace (in, then out) */
    if (W(686)) restore_8790(&A692, old, &C0D0);    /* always true here */
    save_86B2(&A692, nw, &C0D0);
    sprite_8E62(W(684), x, y);
    W(686) = 0xFFFF; W(680) = x; W(682) = y;
}
void cursor_show_1CCE(void) {
    if (W(686) != 0) return;
    int p[2] = { W(680) - hx, W(682) - hy };
    save_86B2(&A692, p, &C0D0);
    sprite_8E62(W(684), W(680), W(682));
    W(686) = 0xFFFF;
}
void cursor_hide_1D19(void) {
    if (W(686) == 0) return;
    int p[2] = { W(680) - hx, W(682) - hy };
    restore_8790(&A692, p, &C0D0);
    W(686) = 0;
}
void cursor_push_1D55(int n) {                      /* ret 2 */
    if (B(68A) >= 30) return;                       /* full: nothing at all, not even the hide / show */
    B(A674 + B(68A)) = (uint8_t)W(686);
    B(68A)++;
    if (n == 1) cursor_show_1CCE();
    else if (n == 0) cursor_hide_1D19();            /* any other n (2 at 0x09CB, End Turn): keep as it is */
}
void cursor_pop_1D8A(void) {
    if (B(68A) == 0) return;
    B(68A)--;
    if (W(686)) cursor_hide_1D19();
    W(686) = (int8_t)B(A674 + B(68A));              /* cbw: FF -> FFFF */
    if (W(686)) {                                   /* = 1CCE's body, inline (it does not test [686] first) */
        int p[2] = { W(680) - hx, W(682) - hy };
        save_86B2(&A692, p, &C0D0);
        sprite_8E62(W(684), W(680), W(682));
        W(686) = 0xFFFF;
    }
}
```

### 4.1 save 86B2(buf, pos, scr) and restore 8790(buf, pos, scr), VGA paths

Both take the target descriptor (here always C0D0, the current target) and jump on `[9B26]` through word tables:
save at DS:9B38 + mode (0: 8754 nothing, **2: 8730 VGA**, 4: 86F4 EGA planar, 6: 8761 4-bit), restore at DS:9B40 + mode
(0: 8865 nothing, **2: 8840 VGA**, 4: 87FB, 6: 8871). Every capture has [9B26] = 2.

```c
/* 86B2 + 8730: screen -> buffer, NO clipping at all */
void save_86B2(rec *buf, int pos[2], desc *scr) {
    uint8_t *s = seg(scr->seg) + scr->off + rowtab(scr)[pos[1]] + pos[0];   /* row y from the row table, then linear */
    uint8_t *d = seg(buf->seg) + buf->off;
    for (int r = 0; r < buf->rows; r++) {           /* 16 */
        memcpy(d, s, buf->width); d += buf->width;  /* 24 */
        s += scr->width;                            /* 320: a box past x 319 wraps into the next row's left pixels;
                                                       rows past 199 read A000:FA00.. (off-screen video memory) */
    }
}
/* 8790 + 8840: buffer -> screen, clipped at the right and bottom edges of the target (not at its clip rect) */
void restore_8790(rec *buf, int pos[2], desc *scr) {
    int cut  = buf->width + pos[0] - scr->width; if (cut < 0) cut = 0;   /* bytes lost at the right */
    int rows = scr->height - pos[1]; if (rows > buf->rows) rows = buf->rows;
    if (rows <= 0) return;
    uint8_t *d = seg(scr->seg) + scr->off + rowtab(scr)[pos[1]] + pos[0];
    const uint8_t *s = seg(buf->seg) + buf->off;
    for (int r = 0; r < rows; r++) {
        memcpy(d, s, buf->width - cut);
        s += buf->width;                            /* the "add si, cut" patched into the code + the copied bytes */
        d += scr->width;
    }
}
```

No left / top clipping exists in either routine; the hot spot is (0, 0) and every mouse range starts at 0, so the box never
starts left of or above the screen (D).

The sprite itself (8E62) is clipped to the target's clip rect C0DA..C0E0 (817C intersect) and drawn through the colour table
of its descriptor, skipping the transparent nibble (map.c's decode of 8E62; for the arrow: table 0 = identity, so the nibbles 0
and 15 are the colours 0 and 15, nibble 13 is skipped: V by the captures, section 6). If a caller shows the cursor while the naval code has the clip bottom at 185, rows below 185 would not
be drawn (D; never seen: the naval battle keeps the cursor hidden).

`vwait_84BB`: `in al, 3DAh` until bit 3 set, then until clear (end of a vertical retrace). Only 1BE6 waits (D). Nothing in the
port (as credits.c / naval_gfx.c already do with 84BB).

### 4.2 The arrow (INIT entry 0, V)

Descriptor (snapshot DS:CE48 `0b00 0000 0000 06 10 0d 00 0200 1b37`): width 11, hot spot (0, 0), 6 bytes per row, 16 rows,
transparent nibble 13 (D), colour table 0, pixels at INITDAT offset 2 (the INIT sheet's "offset - 2" rule of map.c).
K = colour 0 (CMAP 00 00 00, black), W = colour 15 (CMAP 3F 3F 3F, white), . = transparent:

```
K..........
KK.........
KWK........
KWWK.......
KWWWK......
KWWWWK.....
KWWWWWK....
KWWWWWWK...
KWWWWWWWK..
KWWWWWWWWK.
KWWWWWWWWWK
KKKKWWKKKKK
...KWWK....
....KWWK...
....KWWK...
....KKKK...
```

93 opaque pixels (= the 93 px of the naval checks). The save-under box is 24 x 16, wider than the arrow (fixed size).

## 5. When it is hidden, shown and moved (D)

Every call in the EXE and the five sections (`callers.py`):

- **push(0) ... pop** (hide while drawing, then back as before): EXE 1A8B (Game Control), 22EA, 2A32, 2B40 (fleet move; shows with
  1CCE at 2B9E), 2C6F, 2DB2, 2F5A, 2FD6, 32E2, 382A (shows at 38E4), 3BAF, 3CB0, 3EA3, 4065 (push(di = 0), show at 421E, two pops),
  4280, 453B, **494A (map build)**, **5614 (menu draw)**, **585B**, **59D4 (message box)**, 5C20, 5EB5 (5F82, the non-Roman province,
  pops at 6043 / 64EC), **691B (save rect screen -> work)**, **6973 (restore rect work -> screen, twice)**, 6CB8, 7330, 7784 (hides
  at 781E too), 78F0, 7CE0 (twice); battle D8AD, DA35, DF24 (the battle itself, pops at DFB1 / E20C); gladiators B0E0 (pops at
  B1FC / B37A); naval B12A (push(si = 0) at B13D, pop at **B592**).
- **push(1)** (show): 2900 (buy poll, with a mouse only) and 541F (menu wait, with a mouse only).
- **push(2)** (keep): End Turn 09BA (0x09CB), popped at 0x0CD9.
- **show 1CCE**: main 07E9 (after the first map), 1A8B (if [BA]), 2B40, 382A, 4065, battle D08E/D198 (D1B3, the orders menu),
  gladiators B38B, B4DA (twice), BDEA (card grid).
- **hide 1D19**: revolt 0D13 (0x0E0C), 7784, battle BD67 (BF0D) and the orders tick (D18F: every tick), gladiators B38B (4x), B4DA
  (2x), race / victory section 4 at DF5E.
- **move 1BE6**: 359B, 35DB, 3603 (keyboard mouse, [BA] = 0: arrows move [B6]/[B8] inside [BC]..[C2]), 2900, battle D33A, gladiators
  BDEA.

The low-level drawers (8E62 sprites, 899C text, 7E74 fills, the 6887 page flip work -> screen) do **not** hide the cursor; their
callers do. In the battle's orders mode, 6887 copies the work buffer over the visible cursor without hiding it, and the tick's
1D19 then "restores" into the work buffer (6887 left the target there) the background it saved from the screen, which is the
same pixels: no visible effect (D).

Mouse ranges (06E4 x, 0709 y, each also INT 33h 7 / 8): the map 494A sets x 0..311, y 0..185 and warps to the zone's anchor
(1AB8 + 4z); the battle DF24 sets 0..319 x 0..199 and warps to (159, 99); the negotiation screen 6570 sets 8..103 x 151..197 and
warps to (88, 171) (NEGO_SCREEN_NOTES). 066A = set [B6]/[B8], INT 33h AX = 4, then 0644 reads it back.

**DOSBox reports even x only** (G: the driver's mode 13h granularity; V: [680] is even in all 46 run-time DS images and in the 5 screen-only captures,
and the warps read back as 125 -> 124 (scr_newgame_map, zone 0's anchor (125, 114) -> cursor (124, 114)) and 159 -> 158
(b10_form, battle warp (159, 99) -> (158, 99))). This is the driver, not the game; a port that wants the captures' positions after a
warp rounds x down to even in its 0644.

## 6. Verification against the captures (V)

All cursor positions below come from the DS of the same stop ([680], [682]); `find.py` found the sprite in every captured screen
whose DS has [686] = FFFF, at exactly ([680], [682]), and in none whose DS has [686] = 0.

| capture (VGA screen) | [686] | cursor in the capture |
|---|---|---|
| naval b1_n002_B09E, b1_n003_2C27 | FFFF | (130,141), 93 px |
| naval b1_n004_B09E | FFFF | (154,170) |
| naval b1_n005_2C27 | FFFF | (160,109) (mouse [B6]/[B8] = (248,116): drawn at the old place) |
| naval b2_n001_B09E, b2_n002_2C27 | FFFF | (194,162) |
| naval b2_n003_B09E | FFFF | (252,154) |
| naval b2_n005 .. b2_n009 | 0 (depth 2, stack FF FF) | none |
| naval b2_n010_2C27 | FFFF (depth 1) | (148,92) (mouse at (134,131): drawn at the old place) |
| scr_battle_formation (DS b10_form) | FFFF | (158,99) |
| scr_game_control_menu (DS mem_map_gamemenu_640k) | FFFF | (296,10) |
| scr_narbonensis_province (DS mem_narbonensis_province_640k) | FFFF | (310,185): 87 px, the right column and the bottom row clipped |
| scr_newgame_map | (no DS) | (124,114) |
| scr_rome_province | (no DS) | (140,125) (GAMELOOP_NOTES' "18 px missing at x 140-143") |
| screen_map | (no DS) | (268,185): 89 px, the bottom row clipped |
| scr_battle_report, scr_chariot_race (mem_race_640k: [686] = 0) | - / 0 | none |
| battle b11_frame_tick0 / tick245 | 0 (target = work buffer 2687) | none (these are the work buffer, not the screen) |

Pixel proofs (`ver1.py`; colours compared as palette indexes):

1. **B59F -> 2C27**: the game's screen at B59F (b2_n009_B59F_vga, no cursor) + the arrow at (148, 92) = b2_n010_2C27_vga:
   **0 differing pixels** (raw difference 93 px at (148,92)-(158,107)).
2. The port's own outputs (centurion-cursor.exe, today's sources, `--original --naval-resume`): B59F -> 2C27 and the chained
   BB7E -> 2C27 both differ from b2_n010_2C27_vga by 93 px at (148,92)-(158,107); with the arrow drawn at (148, 92): **0 px**.
3. **`after 1 3 0`**: the port's `--naval-frame ... after 1 3 0` picture differs from b1_n005_2C27_vga by 93 px at
   (160,109)-(170,124); with the arrow at ([680], [682]) = (160, 109) of b1_n005: **0 px**.
4. Save-under (`saveunder.py`, the 640K snapshots hold the buffer at 3929:0000 = linear 39290):
   - mem_map_gamemenu at (296, 10): the 291 non-cursor bytes of the 24 x 16 box equal scr_game_control_menu, 0 differ.
   - mem_narbonensis_province at (310, 185): 257 bytes equal the screen **including the wrap**: box columns 10..23 of row r are the
     screen's pixels (0..13, 186 + r), i.e. the save reads on linearly past x 319; 38 bytes lie past A000:F9FF (row 14 columns
     10..23 and row 15) and are 0 (G: DOSBox's off-screen video memory is clear). The capture's cursor is clipped to 87 px.
   - mem_race ([686] = 0): the buffer is stale (the box of the last save at (144, 110), 258 bytes differ from today's race screen),
     as expected of a hidden cursor.

## 7. What changes in the existing checks once the cursor is drawn

Checks that compare **game-state DS bytes only** do not change: cmp_endturn.py (save ranges, dice, ...), cmp_battle.py (unit table,
armies, ...), and the DS part of cmp_naval_resume.py (680..68F and A674..A691 are excluded classes; with a ported cursor they could be
compared: at n010 [680] = 148, [682] = 92, [684] = CE48, [686] = FFFF, [68A] = 1, A674 = FF FF). Done 2026-11-17: the tool now
compares 684..68F and A674..A691 (only the position [680] / [682] stays a class); all 11 resumes of naval.sh still 0 bytes. `--zone-check` (anchors) does
not change. cmp_battle_frame.py compares the work buffer with the cursor hidden ([686] = 0 at both orders-open stops): no change.

Screen checks:

| check | today | with the cursor | why |
|---|---|---|---|
| cmp_naval_resume B59F 2C27 (n009 -> n010) | 93 px | **0 px** (proof 1, 2) | B592's pop redraws the arrow at the old (148, 92) |
| cmp_naval_resume chained BB7E 2C27 | 93 px | **0 px** (proof 2) | same pop |
| cmp_naval_frame `after 1 3 0` (b1_n005) | 93 px | **0 px** only if the "after" mode ends as B0E0 does: the pop at B592 from the pre-pop state ([686] = 0, stack top FF) or, equivalently, the arrow at ([680], [682]) because the final DS has [686] = FFFF (proof 3). 1CCE on the final DS draws nothing ([686] is already FFFF). | |
| cmp_naval_resume B09E 2C27 (b2_n001 -> n002, no battle) | 0 px | stays 0 only if the save-under is seeded (below) | B13D push(0) hides (restores the buffer), B592 pops (saves, draws) |
| cmp_naval_resume B09E 2C27 (b1_n002 -> n003, Esc in the flagship menu) | 0 px | as above, and the menu's 1BE6 must not move the arrow: the port's script mouse keeps [B6]/[B8] = (130, 141) of the snapshot (screen.c screen_script) unless the script clicks | |
| cmp_naval_resume B09E BEAE (set-up, b2_n003 -> n005) | 0 px | 0 px | B13D hides, the intro redraws the whole screen, n005 has [686] = 0 |
| other naval resumes (n005..n009 stops), duel / intro / check frames | 0 px | 0 px | the cursor is hidden through the whole battle |
| map / province / menu pictures vs scr_newgame_map, screen_map, scr_rome_province, scr_game_control_menu, scr_narbonensis_province | (their own counts) | minus the cursor's 93 / 89 / 93 / 93 / 87 px, if the port's [680]/[682] equal the capture's (table in 6) | |
| MENU_NOTES scr_battle_formation (2,2)-(106,82) | 0 | 0 | the cursor (158, 99) is outside the compared rect |
| circus scripts (race / gladiators, old vs new exe) | | the shots change wherever the cursor is visible: gladiator card grid (BDEA shows it), training menu (B38B / B4DA); not during the race itself (mem_race: hidden) | |

**Snapshots and the save-under buffer.** The naval snapshots are cut at 0x36270 and the battle / endturn files are 64 KB DS images:
none holds the buffer at 3929:0000 (linear 39290); only the three 640K `mem_*` files do. A port that resumes from a snapshot with
[686] = FFFF has to fill its buffer before the first hide. Filling it with the input screen's 24 x 16 box at ([680] - hx,
[682] - hy) (linear, as 86B2 reads it) puts the captured arrow itself back on hide; that is exact for every resume above (the
arrow is redrawn at the same place, or the screen is redrawn whole), but it would leave an arrow-shaped ghost if a resumed run
moved the cursor before redrawing that area (G; not seen in any existing check).

## 8. Notes for the port

- State: the DS words above plus a 384-byte buffer (24 x 16) for the save-under (the port need not allocate DOS memory; [A694] can
  keep the snapshot's value). The VGA paths only (the port is VGA only).
- Draw into the current target (C0D0's surface: g_scr when it is the screen, g_wrk when it is the work buffer), as the EXE does.
- `sprite_8E62(W(684), x, y)` = map.c's sprite8e62 on descriptor CE48 (map_startup already builds the INIT descriptors there and
  sets [C0E2] = CE48); 1DF5 and the start-up's 066A(160, 100) are trivial.
- With a window, the system pointer has to be hidden for the game's arrow to be the only one (screen.c shows the system pointer
  today) (G: implementation choice).
- Windowed mode: 1BE6 is only called by the wait loops, so the arrow freezes while the game computes (it does in DOSBox too).

## 9. 1990 bugs, quirks, Easter eggs

- No Easter egg or hidden text in this code or in the arrow's data.
- One 1990 bug the player can see (found in review, 2026-11-17; fixed by default in gamectl.c, `--original` keeps it): Fine
  Tuning 19BC draws the level names (1A5E, 899C) after 5614's pop has saved the box under the arrow and drawn it, so the names
  are drawn over the arrow and the next move of the mouse puts the old box back: a hole in the names (V with --original:
  `Galley S` + hole at (215,105) after a click there and a move; the fixed build shows the names whole).
- Quirks, kept literally (none needs a fix):
  - 1D55 at depth 30 skips the push **and** the hide / show, while the matching 1D8A still pops an older entry (unbalanced). The
    deepest stack in any capture is 3 (b10_form); not reachable in practice (D).
  - The save reads 24 bytes on past x 319 into the next row and past A000:F9FF; the restore clips, so nothing visible (D, V).
  - A hidden cursor is not moved, so a pop shows it at its old place until the next wait loop calls 1BE6 (V: n010 (148, 92) with the
    mouse at (134, 131); b1_n005 (160, 109) with the mouse at (248, 116)). The next 359B moves it at once, so a player never
    sees it; it is what the captures show.
