# Chariot race and victory sequence: OVL0 section 4 index

OVL0.OVL file bytes 0x51A0-0x8550 (0x33B0 bytes), loaded at 1331:0000 = 0823:B0E0; code 0xB0E0-0xE481, zero padding
0xE482-0xE48F. Listing: `disasm/OVL_RACE.asm` (format of `disasm/OVL_NAVAL.asm`: absolute DS operands printed unsigned,
calls masked to 16 bits, EXE names and DS strings as comments, a `; ---- func` line with size/frame/ret/callers and a one-line
role before each function). It replaces the old linear-sweep `disasm/OVL_RACE.clean.asm` (0-based addresses, 621 "functions").
Marks: D decoded from the asm/data, V verified (by a run, a render or an exact fit), G guess. Nothing in this section has been
run or compared with DOSBox yet: every behaviour claim below is D or G.

## 1. Section bounds and how the listing was made

**Section table (D).** The EXE overlay manager (segment 129B, EXE image 0xA780; thunk target 129B:02EC, loader 129B:0297) keeps
its section table at 1322:0000 (EXE image 0xAFF0). The thunk `lcall 129B:02EC; dw s; jmp 0823:entry` passes s in cx;
the loader computes `di = 16*(s-1)` (02A9 `mov ax,0x10; dec cx; mul cx`) and reads es:[di+6] flags (0x8000 = loaded),
[di+8] relocation count, [di+0A] load segment, [di+0C] end segment, [di+0E] file-name pointer (1322:0075 = "OVL0.OVL"),
[di+10]/[di+12] file offset in paragraphs (0x0FA: `shl 4` x4 + `int 21h/42h`), [di+14] size in paragraphs (0x1FA). So the
fields of section s straddle two 16-byte rows (the first row doubles as the header: 1322:0000 = root segment, 1322:0004 = the
manager's stack segment). Decoded from the EXE image (bytes as in the runtime image `disasm/EXE_RUNTIME.bin`):

| section | file offset | size | file range | relocs | load seg - end seg | module |
|---|---|---|---|---|---|---|
| 1 | 0x0000 par | 0x009A par | 0x00000-0x009A0 | 0 | 1331-13CB | init, command line, copy-protection quiz, intro, credits |
| 2 | 0x009A par | 0x0355 par | 0x009A0-0x03EF0 | 0 | 1331-1686 | land battle |
| 3 | 0x03EF par | 0x012B par | 0x03EF0-0x051A0 | 0 | 1331-145C | gladiator show |
| 4 | 0x051A par | 0x033B par | 0x051A0-0x08550 | 0 | 1331-166C | **chariot race + victory sequence** |
| 5 | 0x0855 par | 0x02EA par | 0x08550-0x0B3F0 | 0 | 1331-161B | naval battle |
| 6 | 0x0B3F par | 0x0C41 par | 0x0B3F0-0x17800 | 2 | 1686-2347 | DS data image (2 relocations), at 1686 = DS |

Section 4 = OVL0.OVL 0x051A0-0x08550, 0 relocations, loaded at 1331:0000 (D). Section 6 starts with a 16-byte relocation
block at 0xB3F0, hence "DS:0000 = OVL0 0xB400" (D). Sections 1-5 all load at 1331 (= 0823:B0E0); only one is resident at a time.

**Listing method (V: exact fit, checked by script).** `sec4.bin` = OVL0.OVL[0x51A0:0x8550]; `.code16` + `.incbin`, clang
`--target=i386-unknown-elf -c`, llvm-objdump `-d -M intel --triple=i386-unknown-unknown-code16 --adjust-vma=0xB0E0` with
`--start-address` per block. Recursive descent from the two entries D08A and DF58: every jcc/jmp/loop target and every
near-call target inside the section followed; two indirect jumps resolved by hand: **B598** `jmp cs:[bx+0xB59D]` (switch on
the racer state, `cmp ax,0x0B; ja B60C` before it: 12 words at **B59D-B5B4**, the only data inside the code) and **DEB8**
`jmp ax` with ax = W(DS:8BB0 + W(9B26)) (video-mode dispatch, 4 words DS:8BB0 = DEBA, DEC4, DEFE, DEDB). Result:

- 53 functions (2 entries + 51 near-call targets); with the 5 pad bytes (00 at BDA3, D089, D391, DE63, DF57), the jump table
  and the tail padding E482-E48F they tile 0xB0E0-0xE490 exactly: no gap, no overlap, no instruction shared by two functions,
  no tail jump into another function. No byte of the section is unreached.
- Every branch/call target inside the section is an instruction start; every function ends in `ret`/`ret n` (DE64 ends in a
  `jmp DEBA` back to its own `ret 6`). No far calls, no `int`, no other indirect jump or call; `out` only in DE64 (EGA
  graphics controller 3CE, the DEFE path).
- The linear sweep (the old listing) loses sync at the 5 pads, where `00 55 8B EC` / `00 2A C0` decode as `add [di-0x75],dl` /
  `add [bp+si],ch`: it swallowed the prologues of BDA4, D392, DE64 and **both entries D08A and DF58**, and it decoded the jump
  table as code. Instruction starts the descent has and the sweep lacks: B5B5 BDA4 BDA5 D08A D08B D392 D393 DE64 DE65 DF58
  DF5A DF5B DF5E DF61.
- Out-of-section calls are near calls into EXE segment 0823 (the overlay runs with CS = 0823), plus one call of the thunk B0CE.

## 2. Entry points (D)

Thunks (EXE 0xB06E.., 12 bytes): only two of the nine target section 4, and a byte scan of the EXE (0-0xB06E) and of
sections 1-5 for `E8 rel16` to B092/B0AA finds exactly the two known callers.

| entry | EXE thunk | called from | signature |
|---|---|---|---|
| D08A | B0AA (`dw 4`, `jmp 0823:D08A`) | EXE 0x349D (Rome, Hold Games, "Racing Chariots"; port/src/province.c rome_games3457) | D08A(void), no bp frame, plain `ret`; no result (ax not set on the race path) |
| DF58 | B092 (`dw 4`, `jmp 0823:DF58`) | EXE 0x09A4 (map loop, rank Caesar; port/src/map.c ~400) | DF58(void), plain `ret` |

D08A in full (D):
```c
B(0xC350) = 0xFF;                 /* leader lap = -1 */
B0E0();                            /* speckles */
if (C2DE()) {                      /* pre-race screen; 0 = cancelled (chariot fee already paid, no race used) */
    W(0x154E) = W(0x634C);         /* one race per turn: this turn is used */
    W(C32E) = W(C36C) = W(C330) = W(C332) = W(C32C) = 0;
    music_play(3); timer0_reset(); frame_wait(0x12);
    while (!W(0xC32C)) {           /* race loop */
        timer0_reset(); DABA(); CECC(); D17A(); B190();
        for (i = 0; i < 4; i++)    /* horses recover 1 point every 150 ticks, up to DS:87E6[type] */
            if (W(C324+2i) + 150 <= W(C322)) { B(C2CE+18i) = add_cap(B(C2CE+18i), 1, B(87E6 + B(C2C6+18i)));
                                               W(C324+2i) = W(C322); if (i == 3) W(C330) = -1; }
        if (W(9B26) == 6) D3C2(0);
        while (ticks_since(0) < 7) ;
        if (W(9B26) == 6) flip();  D3C2(-1);
    }
    BCF4(); free(W(0xC308)); if (W(9B26) == 6) 6837(); music_stop(); music_unload(2); music_unload(3);
    C9B6();                        /* result, payout */
    C2C2();                        /* (on the cancel path C2DE has already freed everything) */
}
```
The EXE side checks (1E21 disk, treasury >= 5, W(154E) != W(634C)) are before the thunk; the chariot fee, the bet and the
skulduggery are paid inside section 4 (C2DE, C730).

DF58 in full (D): `music_load(0); 1D19(); DF74(); BDA4(); BF6A(1); C2C2(); B0CE();` - song 0 (cn1, archive 3 per the
3EF0 song table in GAMELOOP_NOTES), the statistics boxes, the gallery and race2 graphics, the triumph parade (10 chariots), then
the credits in section 1 (thunk B0CE = section 1 B9A1). See section 9 for what happens to the return address.

## 3. Functions (53)

Size = bytes up to the pad. Frame: `bp frame, locals n` = `push bp; mov bp, sp; sub sp, n`. Args = ret n / 2.
Roles: D where the code was read for this index, G where only the shape was looked at.

| func | end | size | frame | ret | callers | role |
|---|---|---|---|---|---|---|
| B0E0 | B11C | 0x03c | no bp frame | `ret` | D08A | init the 10 speckle records DS:AA78..AAB3 (6 bytes: x = 5 + 4*rng(77), y = 45 + rng(109), 16-bit pattern = rng(65535)): track-surface texture (D; role G) |
| B11C | B190 | 0x074 | bp frame, locals 4 | `ret 0x2` | B190 DA2E | B11C(scroll): scroll the speckles by the player's speed (C2FE accumulated in 83EA, 4.4 fixed) and plot each with DE64; respawn at y 45 past y 170 (D) |
| B190 | B460 | 0x2d0 | bp frame, locals 10 | `ret` | D08A | one race tick: view clear, move the 4 chariots along the 368-point path (laps +6, leader lap C350, 3 laps -> state 7), screen y (BAC5), crash (B6CB), per-chariot step (B460), minimap dots; end of race -> C32C (D/G) |
| B460 | B6CB | 0x26b | bp frame, locals 6 | `ret 0x4` | B190 | B460(chariot, y): per-chariot step: steering and speed by the state C2CC (switch B598 over 12 states), lane changes, bumps, then draws the chariot (BD0A) (G) |
| B6CB | B7D2 | 0x107 | bp frame, locals 4 | `ret 0x2` | B190 | B6CB(chariot): crash/wreck state 5: counters C2CF to 0x18/0x30, slows down; for the player the crash scene D738 (G) |
| B7D2 | B810 | 0x03e | bp frame, locals 0 | `ret 0x4` | B460 B6CB | B7D2(chariot, n): accelerate: speed C2C8 += n up to the limit (G) |
| B810 | B84E | 0x03e | bp frame, locals 0 | `ret 0x2` | B460 B6CB | B810(chariot): slow down (G) |
| B84E | B886 | 0x038 | bp frame, locals 0 | `ret 0x2` | B460 | B84E(chariot): one lane step left (x C334 += 2) unless blocked (C344/C346) (G) |
| B886 | B8BC | 0x036 | bp frame, locals 0 | `ret 0x2` | B460 | B886(chariot): one lane step right (x C334 -= 2) unless blocked (C348/C34A) (G) |
| B8BC | B977 | 0x0bb | bp frame, locals 4 | `ret 0x4` | B460 | B8BC(chariot, side): side bump: damage (D53E chariot, D590 horses), rng speed change (G) |
| B977 | B9C9 | 0x052 | bp frame, locals 4 | `ret 0x4` | B460 | B977(chariot, side): bump check against the neighbour, then B9C9 (G) |
| B9C9 | BA28 | 0x05f | bp frame, locals 2 | `ret 0x6` | B977 D17A | B9C9(a, b, flag): bump damage between two chariots weighted by DS:87EA[type] (20/33/50) (G) |
| BA28 | BAC5 | 0x09d | bp frame, locals 8 | `ret 0x4` | B460 | BA28(chariot, y): pick the animation chain for the state (83CC) (G) |
| BAC5 | BBAD | 0x0e8 | bp frame, locals 4 | `ret 0x2` | B190 | BAC5(chariot): screen y of a chariot relative to the player (C336, clamped -22..170); returns 0 when off screen (G) |
| BBAD | BC15 | 0x068 | bp frame, locals 0 | `ret 0x4` | C2DE | BBAD(type, chariot): set up a chariot's sprite chain for its type (83C4/83C8/83E4) (G) |
| BC15 | BCA0 | 0x08b | bp frame, locals 12 | `ret` | C2DE | load the MRACE sheets race, laps, med (C2BC, C306, C352) and number the sprite chains (BCA0); 0 = failure (D) |
| BCA0 | BCF4 | 0x054 | bp frame, locals 0 | `ret 0x6` | BC15 BCA0 | BCA0(&n, frames, node): recursive: give each node of a sprite-chain tree its frame and offset table (D) |
| BCF4 | BD0A | 0x016 | no bp frame | `ret` | C2DE C70F D08A | free the three race sheets (D) |
| BD0A | BDA3 | 0x099 | bp frame, locals 6 | `ret 0x6` | B460 BD0A D738 | BD0A(node, x, y): recursive: draw a sprite-chain tree (horses, chariot, driver) and step its animation (D) |
| BDA4 | BE10 | 0x06c | bp frame, locals 6 | `ret` | C2DE DF58 | open MRACE: picture slot 3 = raceest.cbm (13A8), gallery1.cbm -> C356, sheet race2 -> C354; C364 = loaded (D) |
| BE10 | BF6A | 0x15a | bp frame, locals 6 | `ret` | C2DE | pre-race parade in the gallery (song 2 = cn3a): chariots drive past, any key ends it (G) |
| BF6A | C23D | 0x2d3 | bp frame, locals 84 | `ret 0x2` | C9B6 DF58 | BF6A(mode): parade after a won race (mode 0, the 4 racers, song 2) or the triumph (mode 1, 10 random chariots, song 0) (G) |
| C23D | C299 | 0x05c | bp frame, locals 0 | `ret 0x6` | BE10 BF6A | C23D(type, x, frame): one parade chariot = 3 race2 sprites (D) |
| C299 | C2C2 | 0x029 | no bp frame | `ret` | BE10 BF6A | animate the gallery figure (race2 frame W(8418) 0..4) (D) |
| C2C2 | C2DE | 0x01c | no bp frame | `ret` | C2DE C70F D08A DF58 | free gallery1 (C356) and the race2 sheet if BDA4 loaded them (D) |
| C2DE | C70F | 0x431 | bp frame, locals 30 | `ret` | D08A | pre-race screen: loads, field once per turn (DA5A), racer cards, chariot purchase 5/10/20, prepare menu (race, skulduggery, bet, exit), bet paid, parade, race screen and gauges, track (CBE6); C100++, C102++; returns -1 = race, 0 = cancelled (D) |
| C70F | C730 | 0x021 | bp frame, locals 0 | `ret 0x2` | C2DE | C70F(desc): cancel: free sheets and pictures, unload songs 2 and 3 (D) |
| C730 | C88E | 0x15e | bp frame, locals 4 | `ret` | C2DE | "What Skulduggery?" menu: bribe 10 (up to 3), physician 30, gods 50 (each once) (D) |
| C88E | C9B6 | 0x128 | bp frame, locals 82 | `ret` | C2DE | "What Will You Bet?" menu: +10, -10, limit = min(treasury, 100), odds shown (D) |
| C9B6 | CBE6 | 0x230 | bp frame, locals 22 | `ret` | D08A | race result: finishing order, message, score, winnings (odds+1)*bet, odds update, AI skill 8BA6 by place and difficulty, winner's province (D) |
| CBE6 | CDCE | 0x1e8 | bp frame, locals 14 | `ret` | C2DE | build the race path: calloc(368, 4) -> C308, filled from the polyline DS:896A..8A89 (D) |
| CDCE | CECC | 0x0fe | bp frame, locals 6 | `ret 0x2` | C2DE | CDCE(i): racer card i: box, home province, strategy (opponents), chariot type and sprite (D) |
| CECC | D089 | 0x1bd | bp frame, locals 6 | `ret` | D08A | player input: keys, mouse (re-centred at 160,100) or joystick [A2] -> command C302 (8..11, 1..3) (D/G) |
| D08A | D17A | 0x0f0 | no bp frame | `ret` | - | ENTRY (EXE thunk B0AA): the chariot race (D) |
| D17A | D391 | 0x217 | bp frame, locals 18 | `ret` | D08A | opponents vs player and each other: overtaking and bumps (B9C9) (G) |
| D392 | D3C2 | 0x030 | bp frame, locals 2 | `ret` | C2DE D3C2 | lap counter: laps sheet frame C350 (D) |
| D3C2 | D400 | 0x03e | bp frame, locals 0 | `ret 0x2` | D08A | D3C2(all): redraw the dashboard parts flagged dirty (C330, C36C, C332) (D) |
| D400 | D453 | 0x053 | no bp frame | `ret` | C2DE D3C2 | speed gauge (C2FE) (G) |
| D453 | D49A | 0x047 | no bp frame | `ret` | D400 | speed gauge needle (line) (G) |
| D49A | D4EC | 0x052 | no bp frame | `ret` | C2DE D3C2 | gauge of C304 (G) |
| D4EC | D53E | 0x052 | no bp frame | `ret` | C2DE D3C2 | gauge of C303 (G) |
| D53E | D590 | 0x052 | bp frame, locals 2 | `ret 0x4` | B8BC B9C9 | D53E(chariot, n): chariot damage C2CD -= n; at 0 the chariot is wrecked (state 11) (D) |
| D590 | D5E1 | 0x051 | bp frame, locals 2 | `ret 0x4` | B8BC B9C9 | D590(chariot, n): horse strength C2CE -= n; speed limited to the maximum (D) |
| D5E1 | D738 | 0x157 | bp frame, locals 90 | `ret 0x2` | B190 D17A DABA | D5E1(chariot): neighbours on the left and right (C344..C34B) by rect tests (817C) (G) |
| D738 | DA2E | 0x2f6 | bp frame, locals 32 | `ret` | B6CB | crash scene for the player (one of two, rng(2)) (G) |
| DA2E | DA5A | 0x02c | no bp frame | `ret` | D738 | clear the view and draw the speckles (D) |
| DA5A | DABA | 0x060 | bp frame, locals 2 | `ret 0x2` | C2DE | DA5A(i): random racer i: home province rng(27), chariot type rng(3), strategy rng(4) (3 -> 1), skill rng(11) + B(8BA6) - 5 (D) |
| DABA | DDBD | 0x303 | bp frame, locals 24 | `ret` | D08A | opponents' driving per tick: target speeds AABE by difficulty B(048F), strategy C368 and skill AAC2 (D/G) |
| DDBD | DE10 | 0x053 | bp frame, locals 0 | `ret 0x2` | DABA | DDBD(chariot): opponent steers left (G) |
| DE10 | DE63 | 0x053 | bp frame, locals 0 | `ret 0x2` | DABA | DE10(chariot): opponent steers right (G) |
| DE64 | DF57 | 0x0f3 | bp frame, locals 0 | `ret 0x6` | B11C | DE64(pattern, y, x): hand-written: plot a 16-pixel pattern word on row y, per video mode through DS:8BB0 (D) |
| DF58 | DF74 | 0x01c | no bp frame | `ret` | - | ENTRY (EXE thunk B092): the victory sequence: song 0, DF74 statistics, BDA4, BF6A(1) triumph, C2C2, credits thunk B0CE (D) |
| DF74 | E482 | 0x50e | bp frame, locals 62 | `ret` | DF58 | victory statistics: "THE MAKING OF A CONQUEROR" box built in AC60, then the DS:8BD8 epilogue box (D) |

B460 includes the jump table B59D-B5B4 (24 bytes). DE64 is hand-written assembly (saves bx/cx/dx/es/si/di, no C frame shape).

## 4. Call graph inside the section (D)

```
B11C -> DE64
B190 -> B11C B460 B6CB BAC5 D5E1
B460 -> B7D2 B810 B84E B886 B8BC B977 BA28 BD0A
B6CB -> B7D2 B810 D738
B8BC -> D53E D590
B977 -> B9C9
B9C9 -> D53E D590
BC15 -> BCA0
BCA0 -> BCA0
BD0A -> BD0A
BE10 -> C23D C299
BF6A -> C23D C299
C2DE -> BBAD BC15 BCF4 BDA4 BE10 C2C2 C70F C730 C88E CBE6 CDCE D392 D400 D49A D4EC DA5A
C70F -> BCF4 C2C2
C9B6 -> BF6A
D08A -> B0E0 B190 BCF4 C2C2 C2DE C9B6 CECC D17A D3C2 DABA
D17A -> B9C9 D5E1
D3C2 -> D392 D400 D49A D4EC
D400 -> D453
D738 -> BD0A DA2E
DA2E -> B11C
DABA -> D5E1 DDBD DE10
DF58 -> BDA4 BF6A C2C2 DF74
```
Recursive: BCA0 and BD0A (sprite-chain trees). Uncalled from inside: D08A, DF58 (entries). D08A -> B0E0 is the speckle init
(section 4 has its own B0E0; it is not the naval B0E0).

## 5. Calls out to the EXE (D for the call sites; names from the notes, (G) = guessed here)

| EXE | calls | name | called from |
|---|---|---|---|
| 00F8 | 9 | timer0_reset (= 00D7(0)) | BE10 BF6A D08A D738 |
| 011F | 1 | frame_wait(n): busy-wait until timer 0 + n ticks | D08A |
| 01AE | 8 | ticks_since(0) | BE10 BF6A D08A D738 |
| 03AA | 8 | key_read: next key or 0 | BE10 BF6A CECC D738 |
| 04ED | 8 | key_flush | BE10 BF6A CECC D738 |
| 04FA | 3 | add_cap: min(v + step, max), signed (04FA/050F pair) | C88E C9B6 D08A |
| 050F | 1 | step_toward_min: max(v - step, min), signed (050F family) | C88E |
| 0644 | 5 | mouse_poll: [B2] buttons, [B6] x, [B8] y | BE10 BF6A CECC |
| 066A | 2 | mouse_set_pos(x, y) (int 33h fn 4) | C2DE CECC |
| 076B | 35 | rng_roll(n) | B0E0 B11C B460 B6CB B8BC B9C9 BE10 BF6A C2DE C730 C9B6 D17A D738 DA5A DABA |
| 0F7B | 3 | archive open (0F7B(n, &arch)) | BC15 BDA4 C2DE |
| 11A9 | 3 | archive close | BC15 BDA4 C2DE |
| 1273 | 2 | load CBM member into a descriptor | BDA4 C2DE |
| 13A8 | 1 | load CBM picture from archive (13A8) | BDA4 |
| 163B | 4 | sheet_163B: load xxxHDR/xxxDAT sprite sheet | BC15 BDA4 |
| 19A0 | 4 | sheet_19A0: free sprite sheet | BCF4 C2C2 |
| 1D19 | 1 | mouse cursor hide (G) | DF58 |
| 3134 | 4 | ui_frame box(x1, y1, x2, y2, fill) | C2DE CDCE DF74 |
| 3233 | 3 | box(colour, x1, y1, x2, y2): fill unless colour == -1, then frame lines (925C) (G) | C2DE |
| 3512 | 2 | panel_frame (title "<rank>" box, 30E6) (G) | C2DE C730 |
| 35DB | 4 | wait until mouse buttons released (G) | BE10 BF6A |
| 35F9 | 2 | wait: release, click or key, release (35DB/359B/35DB) | DF74 |
| 44E5 | 2 | show_treasury(v) | C2DE C88E |
| 54C8 | 4 | menu run -> index or 0xFF | C2DE C730 C88E |
| 5614 | 4 | menu draw (5614) | C2DE C730 C88E |
| 585B | 3 | menu_preselect(menu, sel, -1) | C2DE C730 C88E |
| 59D4 | 6 | message_box(str) | C2DE C730 C9B6 |
| 5C20 | 3 | yes_no box: 1 = Yes, 0 = No | C2DE |
| 5D20 | 4 | music_stop | BE10 BF6A D08A D738 |
| 5D29 | 4 | music_play(n) | BE10 BF6A D08A |
| 5D73 | 3 | music_load(n) | C2DE DF58 |
| 5E1A | 6 | music_unload(n) | BE10 BF6A C70F D08A |
| 67E4 | 5 | draw into the work buffer | B190 C2DE D738 |
| 680F | 6 | target = the screen descriptor | B190 C2DE D738 |
| 6837 | 3 | draw on the screen (6887 first if needed) | BE10 BF6A D08A |
| 6887 | 7 | flip: work buffer -> screen | BE10 BF6A D08A |
| 691B | 5 | save rect screen -> work buffer | BE10 BF6A C2DE DF74 |
| 6973 | 7 | restore rect work buffer -> screen | B190 D738 DF74 |
| 6F0C | 2 | province_panel(z) (6F0C) | C2DE C730 |
| 7330 | 3 | score_add(n) | C9B6 |
| 7488 | 1 | calloc(n, size) -> DS-near pointer (fatal "calloc" on failure) | CBE6 |
| 74B7 | 1 | free(buf) | D08A |
| 7E74 | 16 | fill rect (colour, ...) | B190 BE10 BF6A C2DE C88E D400 D49A D4EC DA2E |
| 817C | 2 | sprite clip against C0DA..C0E0 | D5E1 |
| 82DE | 2 | putpixel(x, y), colour [9AFA] | B190 |
| 86A0 | 4 | picture_free(desc): DOS free (int 21h/49h) of the block at desc+0C | BDA4 C2C2 C2DE C70F |
| 8790 | 6 | blit picture (8790(src, dst...)) | BE10 BF6A |
| 899C | 3 | text(str, x, y) left-aligned | C2DE |
| 8BE4 | 8 | centred_text(str, cx, y) | C88E CDCE DF74 |
| 8C9F | 2 | text box layout 8C9F(&rect, str, cx, cy) | DF74 |
| 8E62 | 21 | sprite8e62(desc k of sheet, x, y) | BD0A BE10 BF6A C23D C299 C2DE CDCE D392 D738 |
| 979C | 2 | line(x0, y0, x1, y1) via 97B6 with putpixel 82DE | B190 D453 |
| 9DF2 | 45 | strcat | C730 C88E DF74 |
| 9E32 | 4 | strcpy | C730 C88E DF74 |
| 9FDC | 2 | itoa(v, buf, 10) | C88E |
| A022 | 15 | ltoa(lo, hi, buf, 10) | DF74 |
| B0CE | 1 | thunk: section 1 B9A1 = credits | DF58 |

rng_roll (076B, 0..n-1) is called 35 times: B0E0 3, B11C 2, B460 2, B6CB 2, B8BC 1, B9C9 1, BE10 1, BF6A 2, C2DE 1, C730 3, C9B6 1, D17A 1, D738 1, DA5A 4, DABA 10. The port must keep this order (house rule).

Name notes (D): **86A0 is not a blit**: `86A0(desc)` = `868C(desc->+0C)` = `int 21h/49h` (free DOS memory block); C2C2, C70F and
C2DE use it to free a CBM picture after drawing it (NAVAL_INDEX calls it "blit (86A0)"). **3233** (`push bp` at EXE 0x3233,
`ret 0x0A`): `box(colour, x1, y1, x2, y2)`: if colour != -1, [9AFA] = colour and 7E74 fill, then four 925C lines with
[9AFA] = 8 / 0 (bevel); EXE_RUNTIME.clean.asm has no instruction start at 0x3233 (it sits after `ret` at 0x3230). **13A8(&arch,
3)**: picture slot 3 of the table DS:0250 = "raceest.cbm" (cache flags DS:0254+8n). The song numbers: 0 = cn1, 2 = cn3a,
3 = cn3b (DS:3EF0 table, GAMELOOP_NOTES); archive 0 of 0F7B = "mrace" (DS:029E[0] -> DS:63F4).

## 6. The race data (D unless marked)

**Racer records**: 4 x 18 bytes at DS:C2BE + 18i, i = 0, 1, 2 opponents (colours "Red", "Green", "Blue" = DS:8818[i]),
i = 3 the player (C2F0..C301). Fields (from the C2DE initialisation C490-C50D, DA5A and the users):

| off | DS (i=0) | meaning |
|---|---|---|
| +0 | C2BE w | position on the path, fixed point (/75 = path index; -0x6BD0 per lap); start 0x4074 |
| +4 | C2C2 w | running flag (-1 at the start, 0 = out of the race) |
| +6 | C2C4 b | laps done (start = leader lap -1); 3 laps -> state 7 (B30F) |
| +7 | C2C5 b | home province (DA5A: rng(27); the player: 0 = Italia, C3BB); shown on the card via DS:5222 |
| +8 | C2C6 b | chariot type 0 light, 1 medium, 2 heavy (opponents rng(3); the player: the purchase) |
| +9 | C2C7 b | top speed: 100 for the player, 110 for the opponents (C497-C4A1); bribe -rng(10), physician +rng(10) |
| +A | C2C8 b | speed (start = top/3); gauge "S" for the player (D400, C2FE) |
| +B | C2C9 b | top/3 at the start (G: acceleration or cruise) |
| +D, +E | C2CB, C2CC b | previous and current state (0..11, switch B598; 5 crash, 7 finished, 11 wrecked) |
| +F | C2CD b | chariot strength = DS:87FA[type] = 10/15/20; damage D53E; 0 -> wrecked; gauge "C" (D4EC, C303) |
| +10 | C2CE b | horses' strength = DS:87E6[type] = 40/30/20; fatigue D590, +1 per 150 ticks up to the same max; gauge "H" (D49A, C304); gods +10+rng(10) |
| +11 | C2CF b | crash counter (B6CB) |

Per-type tables (3 bytes each, type 0/1/2): 87E6 horses max 40/30/20; 87EA bump weight 20/33/50; 87EE **odds 1/2/3 to 1**
(changed by results, saved); 87F2 **price 5/10/20**; 87F6 card sprite 28/24/20; 87FA chariot strength 10/15/20. Per racer:
C30A+4i path index pair (previous, current), C31A+2i finish tick (-1 = not finished; C320 = the player), C324+2i last
horse-recovery tick, C334+4i lane x / screen y (C336), C344..C34B neighbour flags, C368+i strategy (0 Aggressive, 1 Steady,
2 Cautious: rng(4) with 3 -> 1, so Steady is twice as likely), AAC2+i skill = rng(11) + B(8BA6) - 5, AABE+i target speed (DABA).
The path: C308 = calloc(368, 4) of (x, y) points rasterised by CBE6 from the polyline DS:896A..8A89 (-1 pairs = pen up).
Racer cards (CDCE) at DS:8A8A: (6,6) (213,6) (213,117) (6,117), 100 x 80, name colour DS:87DE[i].

**Pre-race (C2DE).** Opponents are re-rolled (DA5A for all four records) only when W(892E) != W(634C) (once per turn; 892E is not
saved). Menu DS:88E6 "Race what type of Chariot?": price DS:87F2[t] > treasury -> "You don't have enough Talents"; else
treasury -= price, the player's card, message_box(DS:3E02) (an empty string). Exit -> "Are You Sure\nYou Don't Want To Race?"
-> C70F, return 0 (the fee is kept, the race of the turn is not used). Then the menu DS:88F8 "How Shall We Prepare?" loops:
Race Now (no bet while AABC != 0 -> "You Have Not Bet\nRace Anyway?"), Skulduggery (C730), Bet (C88E), Exit (as above).
Race: treasury -= bet (C5BD), BE10 parade, race screen, C100++ (games sponsored) and C102++ (races entered), return -1.

**Skulduggery (C730).** Bribe Opponent 10 (treasury >= 10; at most 3, AABA): strcpy(AC60, "Red"/"Green"/"Blue") + " driver has
been paid\n10 talents to let you win."; that opponent's top speed -= rng(10) (0..9: the bribe can do nothing). Hire Physician
30 (once, AAB6): the herbs text; the player's top speed += rng(10). Invoke Gods 50 (once, AAB8): the Vestal text; the player's
horses += 10 + rng(10). Not enough money -> "You don't have enough Talents"; again -> "You can only do that once"; fourth bribe
-> "All Opponents Bribed". Cost is taken from the treasury at once (W(634A) is juggled with AABC = treasury - bet).

**Bet (C88E).** "Your Bet: <n> Talents\nOdds: <odds> to 1" centred at (78,150); Bet 10 More = add_cap(bet, 10, lim), Bet 10 Less =
max(bet - 10, 0), Bet The Limit = lim, lim = min(treasury, 100).

**Result (C9B6).** Bubble sort of the 4 racers by finish tick C31A (earlier first), then by path position C2BE (further first),
then `rng_roll(1) == 0` -> swap. Player's place p (0..3). If C320 == -1: "You Failed To Finish", odds[type]++ (max 5). Else
message DS:880C[p] and:

| place | text | effect |
|---|---|---|
| 1st | "You Win The Race!" | score_add(10); B(634E) = add_cap(B(634E), 1, 100); treasury += (odds+1)*bet; C096:C098 (bets total) += (odds+1)*bet; odds-- if > 1; C104++ (races won); BF6A(0) parade |
| 2nd | "A Creditable Second" | score_add(5) |
| 3rd | "A Poor Showing" | score_add(1), then falls through into the 4th-place case: odds++ (max 5) |
| 4th | "Utter, Ignominious Failure" | odds++ (max 5) |

Then (not after "Failed To Finish") the opponents' skill base B(8BA6) += DS:8966[p] = +8/+4/0/-2, then by the race difficulty
B(048F): Galley Slave -3, Legionnaire +3, Senator +6, Emperor (3) nothing; clamped to 75..95 (starts at 95, saved). Last: if the
winner is an opponent, its home province's byte +0x0E (unrest, lowered by the provincial games too) -= 10, min 1.

## 7. DS addresses, absolute operands (D; r = read, w = written, rw = read-modify-write)

| DS | r | w | rw | meaning | functions |
|---|---|---|---|---|---|
| 00A2 | 7 | 0 | 0 | joystick/keypad direction and button bits (CECC) (G) | CECC |
| 00B2 | 11 | 0 | 0 | mouse buttons | BE10 BF6A CECC |
| 00B6 | 4 | 0 | 0 | mouse x | CECC |
| 00B8 | 4 | 0 | 0 | mouse y | CECC |
| 00BA | 2 | 0 | 0 | mouse present | CECC |
| 048F | 2 | 0 | 0 | difficulty, chariot race (DS:048E[1]: 0 Galley Slave .. 3 Emperor) | C9B6 DABA |
| 154E | 0 | 1 | 0 | turn of the last race (one race per turn; = W(634C)) | D08A |
| 3E40 | 2 | 0 | 0 | pointer to DS:3E16 "Hit any key to continue" | DF74 |
| 6346 | 7 | 0 | 0 | box fill colour | C2DE CDCE DF74 |
| 634A | 10 | 2 | 6 | treasury (talents) | C2DE C730 C88E C9B6 |
| 634C | 3 | 0 | 0 | turn counter | C2DE D08A |
| 634E | 1 | 1 | 0 | B: Rome's favour 0..100 (plunder halves it) (G name) | C9B6 |
| 83D2 | 2 | 0 | 0 | crash-scene parameter (G) | B6CB D738 |
| 83D4 | 4 | 0 | 0 | race-view clip rect (0,61,319,170), x1 | B190 D738 |
| 83D6 | 4 | 0 | 0 | clip rect y1 | B190 D738 |
| 83D8 | 4 | 0 | 0 | clip rect x2 | B190 D738 |
| 83DA | 4 | 0 | 0 | clip rect y2 | B190 D738 |
| 83EA | 2 | 1 | 1 | speckle scroll accumulator (4.4) | B11C |
| 8418 | 2 | 1 | 1 | gallery figure frame 0..4 | C299 |
| 880A | 2 | 0 | 0 | ptr "You don't have enough Talents" | C2DE C730 |
| 8814 | 2 | 0 | 0 | ptr "Are You Sure\nYou Don't Want To Race?" | C2DE |
| 8816 | 1 | 0 | 0 | ptr "You Failed To Finish" | C9B6 |
| 881E | 1 | 0 | 0 | ptr " driver has been paid..." | C730 |
| 8820 | 1 | 0 | 0 | ptr physician text | C730 |
| 8822 | 1 | 0 | 0 | ptr Vestal text | C730 |
| 8824 | 1 | 0 | 0 | ptr "You can only do that once" | C730 |
| 892E | 1 | 1 | 0 | turn of the current field (DA5A re-roll) | C2DE |
| 896A | 1 | 0 | 0 | path polyline, first x | CBE6 |
| 896C | 1 | 0 | 0 | path polyline, first y | CBE6 |
| 8A9A | 1 | 2 | 0 | last mouse x | CECC |
| 8A9C | 1 | 2 | 0 | last mouse y | CECC |
| 8A9E | 3 | 2 | 8 | direction bits 1/2/4/8 | CECC |
| 8BA6 | 2 | 2 | 4 | opponents' skill base 75..95 (saved) | C9B6 DA5A |
| 8CCE | 1 | 0 | 0 | pointer to the epilogue DS:8BD8 | DF74 |
| 9AFA | 0 | 20 | 0 | draw colour | B190 BE10 BF6A C2DE D400 D453 D49A D4EC DA2E |
| 9B26 | 11 | 0 | 0 | video mode (2 VGA, 6 EGA path) | B190 C2DE D08A DA2E DE64 |
| 9B5C | 2 | 0 | 0 | font metric (line height part) (G) | CDCE DF74 |
| 9B5D | 2 | 0 | 0 | font metric (G) | CDCE DF74 |
| 9B69 | 0 | 6 | 0 | text colour | C2DE CDCE DF74 |
| AA76 | 1 | 1 | 0 | chariot visible (BAC5 result) | B190 B6CB |
| AAB4 | 1 | 1 | 1 | any opponent visible | B190 B6CB |
| AAB6 | 1 | 2 | 0 | physician hired | C2DE C730 |
| AAB8 | 1 | 2 | 0 | gods invoked | C2DE C730 |
| AABA | 3 | 1 | 1 | bribes paid 0..3 | C2DE C730 |
| AABC | 3 | 4 | 0 | treasury minus bet (shown) | C2DE C730 C88E |
| C082 | 1 | 0 | 0 | stats u32: enemy soldiers slain (lo) | DF74 |
| C084 | 1 | 0 | 0 | (hi) | DF74 |
| C086 | 1 | 0 | 0 | stats u32: legionnaires killed (lo) | DF74 |
| C088 | 1 | 0 | 0 | (hi) | DF74 |
| C08A | 1 | 0 | 0 | stats u32: enemy ships sunk (lo) | DF74 |
| C08C | 1 | 0 | 0 | (hi) | DF74 |
| C08E | 1 | 0 | 0 | stats u32: own ships sunk (lo) | DF74 |
| C090 | 1 | 0 | 0 | (hi) | DF74 |
| C092 | 1 | 0 | 0 | stats u32: tribute collected (lo) | DF74 |
| C094 | 1 | 0 | 0 | (hi) | DF74 |
| C096 | 1 | 0 | 1 | stats u32: talents in bets (lo) | C9B6 DF74 |
| C098 | 1 | 0 | 1 | (hi) | C9B6 DF74 |
| C09A | 1 | 0 | 0 | stats u32: plunder (lo) | DF74 |
| C09C | 1 | 0 | 0 | (hi) | DF74 |
| C0D0 | 2 | 0 | 0 | screen descriptor: row-offset table | DE64 |
| C0D2 | 2 | 0 | 0 | descriptor: base offset | DE64 |
| C0D4 | 1 | 0 | 0 | descriptor: segment | DE64 |
| C0D6 | 2 | 0 | 0 | descriptor: row stride | DE64 |
| C0DC | 1 | 0 | 0 | clip y1 | DE64 |
| C0E0 | 1 | 0 | 0 | clip y2 | DE64 |
| C0F8 | 1 | 0 | 0 | stats: battles fought | DF74 |
| C0FA | 2 | 0 | 0 | stats: battles won | DF74 |
| C0FC | 4 | 0 | 0 | stats: fleet battles fought | DF74 |
| C0FE | 5 | 0 | 0 | stats: fleet battles won | DF74 |
| C100 | 2 | 0 | 1 | stats: games sponsored (each race counts) | C2DE DF74 |
| C102 | 2 | 0 | 1 | stats: races entered | C2DE DF74 |
| C104 | 2 | 0 | 1 | stats: races won | C9B6 DF74 |
| C106 | 3 | 0 | 0 | stats: alliances negotiated | DF74 |
| C2BC | 6 | 1 | 0 | sheet "race" | BBAD BC15 BCA0 BCF4 CDCE |
| C2F8 | 2 | 0 | 0 | player running flag (record 3 +4) | B190 CECC |
| C2FB | 0 | 1 | 0 | player home province (0) | C2DE |
| C2FC | 5 | 1 | 0 | player chariot type | C2DE C88E C9B6 |
| C2FD | 0 | 0 | 1 | player top speed | C730 |
| C2FE | 4 | 0 | 0 | player speed | B11C B6CB D400 D738 |
| C301 | 11 | 0 | 0 | player state | B190 CECC D400 D453 D49A D4EC DABA |
| C302 | 1 | 4 | 0 | player command from CECC | CECC |
| C303 | 1 | 0 | 0 | player chariot strength | D4EC |
| C304 | 1 | 0 | 1 | player horses | C730 D49A |
| C306 | 3 | 1 | 0 | sheet "laps" | BC15 BCF4 D392 |
| C308 | 17 | 1 | 0 | path buffer (calloc) | B190 CBE6 D08A |
| C316 | 1 | 0 | 0 | player path index (previous) | BAC5 |
| C320 | 1 | 1 | 0 | player finish tick (-1 = not finished) | B190 C9B6 |
| C322 | 7 | 1 | 1 | race tick counter | B190 B460 B8BC C2DE D08A |
| C32C | 1 | 3 | 0 | race over | B190 D08A |
| C32E | 0 | 6 | 0 | set to -1 when a speed changes (6 writes), never read (D) | B460 B7D2 B810 B8BC D08A D3C2 |
| C330 | 1 | 5 | 0 | dirty: horses gauge | B460 D08A D3C2 D590 |
| C332 | 1 | 3 | 0 | dirty: lap counter | B190 D08A D3C2 |
| C340 | 2 | 0 | 0 | gauge needle / crash scene coordinate (G) | D453 D738 |
| C342 | 1 | 0 | 0 | (G) | D738 |
| C344 | 3 | 0 | 0 | neighbour flags (D5E1) | B460 B84E DDBD |
| C346 | 4 | 0 | 0 | neighbour flags | B460 B84E D17A DDBD |
| C347 | 1 | 0 | 0 | neighbour flags | D17A |
| C348 | 3 | 0 | 0 | neighbour flags | B460 B886 DE10 |
| C34A | 3 | 0 | 0 | neighbour flags | B460 B886 DE10 |
| C34C | 7 | 0 | 0 | bumped-by index + 1 (low nibble) (G) | B190 B460 B7D2 DABA |
| C34E | 4 | 0 | 0 | bumped-by index + 1 (G) | B190 B460 B810 |
| C350 | 5 | 1 | 1 | leader lap (-1 start) | B190 C2DE D08A D392 |
| C352 | 8 | 1 | 0 | sheet "med" | BC15 BCF4 D738 |
| C354 | 8 | 1 | 0 | sheet "race2" | BDA4 BF6A C23D C299 C2C2 |
| C364 | 3 | 5 | 0 | gallery graphics loaded | BDA4 BE10 BF6A C2C2 |
| C366 | 9 | 3 | 0 | bet | C2DE C88E C9B6 |
| C36C | 1 | 3 | 0 | dirty: chariot gauge | D08A D3C2 D53E |

Static DS tables and pointers read by the code without an absolute operand: DS:8BB0 (code pointers DEBA/DEC4/DEFE/DEDB),
DS:5222 (province names), DS:527E (province records +0x0E), DS:87FE (string pointers, section 10).

## 8. DS tables indexed by a register (reg + displacement) (D)

| base | r | w | rw | index | meaning | functions |
|---|---|---|---|---|---|---|
| 5222 | 1 | 0 | 0 | bx | province name pointers (CDCE: home province) | CDCE |
| 527E | 1 | 1 | 0 | bx | province record +0x0E (32-byte records from 5270): winner's province unrest -10 | C9B6 |
| 83A4 | 1 | 0 | 0 | bx | sprite-chain offset tables | BCA0 |
| 83BC | 1 | 0 | 0 | si | sprite-chain roots (8) | BC15 |
| 83C4 | 1 | 0 | 0 | bx | frames per type | BBAD |
| 83C8 | 1 | 0 | 0 | bx | frames per type | BBAD |
| 83CC | 18 | 0 | 0 | bx bx + si bx + di si | sprite-chain node pointers | B460 B6CB BA28 BBAD BC15 |
| 83E4 | 1 | 0 | 0 | bx | chain per type | BBAD |
| 8412 | 1 | 0 | 0 | bx | parade sprite table | BF6A |
| 87DE | 1 | 0 | 0 | bx + si | racer name colours (4 words) | CDCE |
| 87E6 | 2 | 0 | 0 | bx | horses max 40/30/20 | C2DE D08A |
| 87EA | 1 | 0 | 0 | bx | bump weight 20/33/50 | B9C9 |
| 87EE | 4 | 0 | 3 | bx | odds 1/2/3 (rw: results) | C88E C9B6 |
| 87F2 | 2 | 0 | 0 | si | chariot price 5/10/20 | C2DE |
| 87F6 | 1 | 0 | 0 | bx | card sprite 28/24/20 | CDCE |
| 87FA | 1 | 0 | 0 | bx | chariot strength 10/15/20 | C2DE |
| 87FE | 1 | 0 | 0 | bx | string pointers [type] | CDCE |
| 8804 | 1 | 0 | 0 | bx | string pointers [3 + strategy] | CDCE |
| 880C | 1 | 0 | 0 | bx + di | string pointers [7 + place] | C9B6 |
| 8818 | 1 | 0 | 0 | bx | string pointers [13 + k] Red/Green/Blue | C730 |
| 8966 | 1 | 0 | 0 | di | skill change by place +8/+4/0/-2 | C9B6 |
| 8A8A | 4 | 0 | 0 | bx si | racer card x | CDCE |
| 8A8C | 6 | 0 | 0 | bx si | racer card y | CDCE |
| 8AA0 | 1 | 0 | 0 | bx | speed table (G) | D17A |
| 8BA8 | 1 | 7 | 0 | si bx | opponent steering state (G) | DABA DDBD DE10 |
| 8BAC | 1 | 3 | 1 | si bx | opponent steering timer (25) | DABA DDBD DE10 |
| 8BB0 | 1 | 0 | 0 | bx | video-mode code pointers | DE64 |
| 8BB8 | 1 | 0 | 0 | bx | left-mask words | DE64 |
| AABE | 2 | 10 | 2 | si | opponent target speed | DABA |
| AAC2 | 1 | 1 | 0 | bx si | opponent skill | DA5A DABA |
| C2BE | 2 | 0 | 0 | si | racer +0 position | C9B6 |
| C2C2 | 2 | 1 | 0 | bx | racer +4 running | B6CB B8BC B977 |
| C2C5 | 2 | 1 | 0 | bx | racer +7 province | C9B6 CDCE DA5A |
| C2C6 | 3 | 1 | 0 | bx si | racer +8 type | B9C9 CDCE DA5A |
| C2C7 | 5 | 0 | 1 | bx | racer +9 top speed | B460 B6CB C730 D590 |
| C2C8 | 11 | 5 | 1 | bx si | racer +A speed | B460 B6CB B7D2 B810 B8BC D590 |
| C2C9 | 2 | 0 | 0 | bx | racer +B | B190 |
| C2CB | 4 | 2 | 0 | bx | racer +D previous state | B460 B6CB BA28 D53E |
| C2CC | 6 | 11 | 0 | bx | racer +E state | B460 B6CB D53E DDBD DE10 |
| C2CD | 2 | 1 | 1 | bx | racer +F chariot strength | D53E |
| C2CE | 2 | 1 | 2 | bx | racer +10 horses | B460 D590 |
| C2CF | 2 | 0 | 1 | bx | racer +11 crash counter | B6CB |
| C30A | 1 | 0 | 0 | si | path index pairs | BAC5 |
| C30C | 2 | 0 | 2 | bx | path index (current) | B190 |
| C31A | 4 | 0 | 0 | si | finish ticks | C9B6 |
| C324 | 0 | 2 | 0 | bx di | last recovery tick | B460 B8BC |
| C334 | 4 | 2 | 2 | si bx | lane x | B460 B84E B886 D5E1 |
| C336 | 12 | 3 | 2 | bx si | screen y | B190 B460 BAC5 D5E1 |
| C344 | 2 | 3 | 0 | bx bx + di | neighbour flags | B8BC B977 D5E1 |
| C345 | 2 | 0 | 0 | bx | neighbour flags | B8BC B977 |
| C368 | 3 | 1 | 0 | si bx | strategy | CDCE DA5A DABA |

Racer fields appear as table bases because the code indexes them with bx = 18*i (C2C2..C2CF = record 0 + 4..+0x11).

## 9. Archives, files, music (D)

- 0F7B(0, &arch) = archive 0 = MRACE.DIR/.DAT (runtime name "mrace"), opened three times: BC15 (sheets `race`, `laps`, `med`
  -> RACEHDR/RACEDAT, LAPSHDR/LAPSDAT, MEDHDR/MEDDAT .LZW), BDA4 (picture slot 3 `raceest.cbm` via 13A8, `gallery1.cbm` via 1273
  into DS:C356, sheet `race2` -> RACE2HDR/RACE2DAT), C2DE (`circus.cbm` via 1273 into a local descriptor, drawn at (0,0), freed).
- MRACE members not named by section 4: CN3A.LZW / CN3B.LZW are songs 2 and 3 (loaded by 5D73 from archive 0), and
  GALLERY2.CBM: no "gallery2" string exists in OVL0.OVL or CENTURIO.EXE (byte search; only gallery1, raceest, circus). It has
  exactly RACEEST's pixels (CBM_FORMAT.md), so the 2026-11-06 race match was RACEEST from picture slot 3 (G): GALLERY2 looks
  like an unused member.
- Songs: race = 3 (cn3b), parades = 2 (cn3a), triumph = 0 (cn1, archive 3 CENT).
- No file is written. calloc(368, 4) for the path (CBE6), freed by D08A.

## 10. DS strings it uses, in full (D: OVL0.OVL 0xB400 + address; identical in the runtime snapshots)

File names: "race", "laps", "med", "gallery1.cbm", "race2", "circus.cbm" (immediates), "raceest.cbm" (DS:0250 slot 3).

String pointer table DS:87FE-8825 (20 words, every entry used; the menu items follow at 8826):

| index | DS | text | used by |
|---|---|---|---|
| 0 | 841A | "Light Chariot" | CDCE type |
| 1 | 8428 | "Medium Chariot" | CDCE type |
| 2 | 8437 | "Heavy Chariot" | CDCE type |
| 3 | 8445 | "Aggressive" | CDCE strategy |
| 4 | 8450 | "Steady" | CDCE strategy |
| 5 | 8457 | "Cautious" | CDCE strategy |
| 6 | 8460 | "You don't have enough Talents" | C2DE, C730 |
| 7 | 847E | "You Win The Race!" | C9B6 place 1 |
| 8 | 8490 | "A Creditable Second" | C9B6 place 2 |
| 9 | 84A4 | "A Poor Showing" | C9B6 place 3 |
| 10 | 84B3 | "Utter, Ignominious Failure" | C9B6 place 4 |
| 11 | 84CE | "Are You Sure\nYou Don't Want To Race?" | C2DE exit |
| 12 | 84F3 | "You Failed To Finish" | C9B6 |
| 13 | 8508 | "Red" | C730 bribe |
| 14 | 850C | "Green" | C730 bribe |
| 15 | 8512 | "Blue" | C730 bribe |
| 16 | 8517 | " driver has been paid\n10 talents to let you win." | C730 bribe |
| 17 | 8548 | "A physician from the far East supplies\nexotic herbs 'guaranteed' to make your\nhorses win.  You doubt that this will\nwork but pay the 30 talents he asks." | C730 physician |
| 18 | 85E1 | "You send an emissary to the Temple of\nthe Vestal Virgins where a squinty-eyed\nattendant promises the gods' favor in\nreturn for a small donation of 50 talents." | C730 gods |
| 19 | 8680 | "You can only do that once" | C730 |

Menus (header 18 bytes: x1, y1, x2, y2, sel, items, title, count; items 12 bytes: text, hotkey, x1, y1, x2, y2):

| menu | title | items (hotkey) |
|---|---|---|
| 88E6 (6,117)-(156,197) | "Race what type of Chariot?" | "Light Chariot: 5" (L), "Medium Chariot: 10" (M), "Heavy Chariot: 20" (H), "Exit" (E) |
| 88F8 (6,79)-(156,197) | "How Shall We Prepare?" | "Race Now" (R), "Skulduggery" (S), "Bet" (B), "Exit" (E) |
| 890A (6,79)-(156,197) | "What Will You Bet?" | "Bet 10 More" (+), "Bet 10 Less" (-), "Bet The Limit" (L), "Done" (D) |
| 891C (6,79)-(156,197) | "What Skulduggery?" | "Bribe Opponent: 10" (B), "Hire Physician: 30" (H), "Invoke Gods: 50" (I), "Done" (D) |

Immediates: "You Have Not Bet\nRace Anyway?", "All Opponents Bribed", "Your Bet: ", " Talents\nOdds: ", " to 1"; dashboard gauge labels "S", "H", "C" (Speed, Horses, Chariot: G) at (7,181), (191,177), (191,186).
message_box(DS:3E02) after the chariot purchase: DS:3E02 is an empty string (the byte before "Continue").

Victory statistics (DF74), concatenated into AC60 in this order (bracketed parts conditional):

```
"THE MAKING OF A CONQUEROR\nIn becoming Caesar, you " [ "successfully negotiated " <C106> " alliance" [ "s" ] ",\n" ] "won " <C0FA> " battles and lost " <C0F8-C0FA> " battles,\nin which " <C086:C088> " legionnaires were killed and " <C082:C084> " enemy soldiers\nwere slain.  " [ "You " [ "won " <C0FE> " fleet battles\nand " ] "lost " ( "one fleet battle" | "no" |<C0FC-C0FE> ) " fleet battles" ", in which " <C08E:C090> "\nof your ships were sunk and " <C08A:C08C> " enemy ships were sunk.\n\n" ] [ "You sponsored " <C100> " games, entered " ( "no" |<C102>) " races,\nwinning " ( "none" |<C104>) ".\n\n" ] "You collected " <C092:C094> " talents in tribute, " <C096:C098> " talents in bets,\nand " <C09A:C09C> " talents in plunder."
```
Conditions (D): alliances part only if C106 != 0 ("s" if != 1); fleet part only if C0FC != 0, "won <n> fleet battles\nand " only
if C0FE != 0, then "lost " + ("one fleet battle" if lost == 1 [no " fleet battles" after it], "no" if lost == 0, else the
number); games part only if C100 != 0, races "no" if 0, wins "none" if 0. The box: 8C9F(&rect, 0x63, 0x9F, 1, 0, AC60) layout (argument meaning G), save rect, ui_frame, text
colour 0, then "Hit any key to continue" (W(3E40)) in colour 15, wait 35F9. Second box, same layout, DS:8CCE -> DS:8BD8:

> The known world lies at your feet.  Your deeds,
> like those of Alexander, will be remembered for
> generations and recited by children whose parents
> are not yet born.  Remember, though, that empires
> rise and fall like tides, and nothing is forever.

## 11. Odd things, candidate 1990 bugs, jokes

1. **Tie-break that never flips (D).** C9B6 ca4d: `rng_roll(1)` is always 0, so a full tie (same finish tick, same path
   position) always swaps; a coin flip would be rng_roll(2). Fix candidate (keeps the rng call count); --original keeps 1.
2. **Emperor adapts less than Senator (D code, bug G).** C9B6 cb61: the skill base 8BA6 moves -3/+3/+6 for Galley Slave/
   Legionnaire/Senator and not at all for Emperor (no case 3), so after a loss the Emperor field gets weaker (-2) where the Senator
   field gets stronger (+4). DABA handles all four levels; only this switch misses one. Intended value unknown (G: +9).
3. **3rd place falls into the 4th-place case (D).** cb41 has no jump: "A Poor Showing" scores 1 and also lengthens the odds,
   like last place; 2nd place leaves them. Plausibly intended (G); not a fix candidate without evidence.
4. **Grammar in the victory text (D).** Only "alliance(s)" and "one fleet battle" have singular forms: "won 1 battles",
   "lost 1 battles", "won 1 fleet battles", "sponsored 1 games", "entered 1 races", "winning 1." all occur. A text slip
   candidate under Erik's rule (the game's own singulars show the intent). Also "1 legionnaires were killed", "1 enemy
   soldiers were slain", "1 of your ships were sunk", "1 enemy ships were sunk", "1 talents in ..." (e088..e317). All fixed by
   race.c bug 12 (RACE_LOGIC_NOTES 11.18).
5. **"talents in bets" is gross (D).** C9B6 adds (odds+1)*bet (stake included) to C096:C098; the stake was paid before the
   race and is not subtracted on a loss. "collected" can mean gross, so G whether this is a slip.
6. **Empty message box (D).** After buying a chariot, message_box(DS:3E02) shows a box whose text is the empty string
   (probably just a pause with "Continue" while the player's card is shown: G).
7. **Credits from inside the overlay (D, not run).** DF70 calls thunk B0CE, which loads section 1 (0x9A0 bytes) over
   B0E0-BA7F; section 4's own `ret` at DF73 lies outside that range, so it survives and returns to EXE 0x09A7. It works
   because section 1 is short.
8. **The bribe can do nothing (D).** "...paid 10 talents to let you win" lowers that driver's top speed by rng(10) = 0..9.
   The physician's text says "You doubt that this will work" and it adds 0..9 to your top speed (the joke holds: it may do
   nothing); the Vestal Virgins' "small donation" is the only sure effect (+10..19 horse strength, above the 40/30/20 cap).
9. **Opponents are faster (D).** Top speed 110 for every opponent, 100 for the player (C497-C4A1), before bribes and potions.
10. **Exit after buying (D).** Leaving at "Are You Sure..." after the purchase keeps the price but does not use the race of
    the turn (W(154E) is set only when C2DE returns -1), and the same field is shown again (892E) until the turn changes.
11. **Odds walk (D).** Win: odds-1 (not below 1); 3rd, 4th or not finishing: odds+1 (not above 5); saved (DS:87EE in the save
    table 5BDE), so a Light chariot pays 1 to 1 at best and the odds drift with the player's record.
12. Outside section 4 but seen on the way (D): the archive-name strings DS:63EE-641F are stored XOR-encoded in OVL0.OVL;
    XOR of the file bytes with the runtime snapshot gives the key "~enny" for each name ("kenn" for one): something at start-up
    decodes them. A hidden name, maybe ("Kenny"?) (G).

