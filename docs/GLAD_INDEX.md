# Gladiator show: OVL0 section 3 index

OVL0.OVL file bytes 0x3EF0-0x51A0 (0x12B0 bytes), loaded at 1331:0000 = 0823:B0E0, code 0xB0E0-0xC384, zero padding
0xC385-0xC38F. Listing: `disasm/OVL_GLAD.asm` (same format as `disasm/OVL_NAVAL.asm`; absolute DS operands printed
unsigned, calls masked to 16 bits, EXE names, DS variable names and DS strings as comments). Marks: D decoded from the
asm/data, V verified by a run or an exact fit, G guess. Nothing here has been run against DOSBox yet: every behaviour claim
is D (read from the listing), not V.

## 1. Section bounds and how the listing was made

**The overlay table (D, V by exact fit).** CENTURIO.EXE is linked with **PLINK86** (Phoenix): the overlay manager sits in
segment 129B (0823:A780-), with the strings "Copyright (C) 1984, 1985, 1986 by Phoenix Software Associates Ltd." (0823:A8FF)
and "PLINK86 Overlay Loader..." (0823:AEDD). Every thunk is `lcall 129B:02EC` (0823:AA6C, saves the registers, reads the
section word after the call, `lcall 129B:0292` = 0823:AA12 the loader) + `dw section` + `jmp 0823:target`. The loader
(AA17) takes `di = (section - 1) * 16` into the table at **1322:0000 = 0823:AFF0** (in `disasm/EXE_RUNTIME.bin`), tests
`es:[di+6] & 0x8000` (loaded), and seeks to the file position in paragraphs `es:[di+0x10]` (A87A: `bx = es:[di+0x10] + dx`,
`ax = es:[di+0x12]`, shifted left 4). So the entry for section n spans 1322:(n-1)*16 .. +0x13 and its file position is
the first dword of the next 16-byte row:

| section | flags +6 | load seg +0xA | end seg +0xC | file pos (+0x10) | size | OVL0 file bytes | contents |
|---|---|---|---|---|---|---|---|
| 1 | 8000 (loaded in the snapshot) | 1331 | 13CB | 0x0000 | 0x09A paras | 0x0000-0x09A0 | copy protection + B18A/B9A1 (thunks B07A, B086, B0CE) |
| 2 | 0 | 1331 | 1686 | 0x009A | 0x355 | 0x09A0-0x3EF0 | land battle (thunk B06E -> DF24) |
| **3** | 0 | 1331 | **145C** | **0x03EF** | **0x12B** | **0x3EF0-0x51A0** | **gladiator show (thunk B0B6 -> B0E0)** |
| 4 | 0 | 1331 | 166C | 0x051A | 0x33B | 0x51A0-0x8550 | chariot race (thunks B0AA -> D08A, B092 -> DF58) |
| 5 | 0 | 1331 | 161B | 0x0855 | 0x2EA | 0x8550-0xB3F0 | naval battle (thunks B09E -> B0E0, B0C2 -> B12A) |

End segment - load segment = size in every row, and the five sections tile OVL0.OVL 0x0000-0xB3F0 exactly; 0xB3F0 is
where nothing more is loaded (OVL0 0xB400 = the DS image). Thunks (D, bytes at 0823:B06E): B06E s2 DF24, B07A s1 B18A,
B086 s1 B0E0, B092 s4 DF58, B09E s5 B0E0, B0AA s4 D08A, B0B6 **s3 B0E0**, B0C2 s5 B12A, B0CE s1 B9A1. The entry addresses
used by the port's stubs are right (D). Two corrections to older notes: section 4 ends at **0x8550**, not 0x8540 (the
last 0x10 bytes are its padding); and OVL0 0x0000-0x09A0 is not a resident part but **section 1** (loaded at B0E0 like the
others), so the "func_0x03BC section loader" of STAGE4 stage 39 is in fact section 1's copy-protection question (PLACES,
STAGE4 line 461) disassembled at the wrong origin; the real loader is PLINK86's (above).

**Listing (V for the checks).**
- `sec3.bin` = OVL0.OVL[0x3EF0:0x51A0]; `.code16` + `.incbin`, clang `--target=i386-unknown-elf -c`, llvm-objdump
  `-d -M intel --triple=i386-unknown-unknown-code16 --adjust-vma=<start>` (capstone has no wheel for this ARM64 Python).
- Made per function: 17 starts = B0E0 + every `push bp; mov bp, sp` (16 incl. B0E0) + every near-call target, found by
  iterating to a fixed point (one more: **B67C**, no prologue). Checks in the generator: the functions tile
  0xB0E0-0xC385 with no gap or overlap; every function ends in `ret`/`ret n`; every jump/call target inside the section
  and every jump-table entry is an instruction start. Unlike sections 2 and 5 there are **no pad bytes** between
  functions (functions start on odd addresses: B38B, B4A3, BB3B, ...), so a linear sweep would not lose a prologue here.
- **One data island:** the jump table B91C-B92F (10 words) right after `jmp word ptr cs:[bx+0xB91C]` at B917 (in B7FE);
  a linear sweep decodes it as `aaa; mov cx, 0xb946; xor [bx+di-0x46ac], bh ...` and resyncs at B930 by luck. It is listed
  as `dw`. No other `cs:` operand, indirect jump/call or `lcall` in the section; no strings in the code (all text is in DS).
- Scripts (scratchpad `glad/`): `seg.py` (segmentation), `gen.py` (listing + checks + index dump), `odis.py`.

## 2. Entry point (D)

| entry | EXE thunk | called from | signature |
|---|---|---|---|
| B0E0 | B0B6 (section 3) | 0x34F5 (Rome Games 0x3457, "Gladiator Show", after the amphitheatre/turn checks and the 1E21 disk check) | `void show(void)`, no args (`ret`), no result |

Archives: `0F7B(6, &arch)` = archive 6 = **MGLAD** (D: DS:029E[6] = DS:63FA, obfuscated "MGLAD" with the same XOR key
"^ENNY" as entry 0's "MRACE"; and the music table DS:3EF0 puts songs 4..7 = cn4a..cn4d in archive 6, which only MGLAD
has). If the open fails: message box "Can't Load Gladiator Graphics" and return. Members used (D):

| member | loaded by | as |
|---|---|---|
| ARENABTM.CBM | B0E0 `13A8(4, &arch)`: picture slot 4 (DS:0250+8*4: name DS:01F0 "arenabtm.cbm", descriptor C0BE) | the lower arena strip, blitted at (0, 126) by 8790 (B0E0, C1A4) |
| COLOSEUM.CBM | B0E0 `1273("coloseum.cbm", &arch, &desc)` | title picture (sic: "coloseum") |
| SHOWHDR/SHOWDAT | `163B("show")` -> [C2BA] | card portraits: frames 0/1 = man/wimp facing right, 2..6 = man/wimp/cat0/cat1/cat2 facing left |
| ARENAHDR/ARENADAT | `163B("arena")` -> [C2B4] | frame 0 = arena background, 1 = card frame piece, 2 = name plate, 3 = thumbs box |
| RMANHDR/DAT or RWIMPHDR/DAT | `163B(DS:7BCC[body])` -> [C2B6] | the player's fighter (body 0 = man, 1 = wimp) |
| LMAN, LWIMP, LCAT0..2 HDR/DAT | `163B(DS:7BD0[body])` -> [C2B8] | the opponent (body 0..4) |
| CN4A..CN4D | `5D73(4..7)` | music: 4 title/arena, 5 VERSUS/fight start, 6/7 crowd moods |

## 3. Functions (17)

Size = bytes to the next function. Frame: `bp frame, locals n` = `push bp; mov bp, sp; sub sp, n`. Args = ret n / 2,
Pascal order (first pushed = first parameter). P = the player's fighter (DS:AA52..), O = the opponent's (DS:AA6A..).

| func | end | size | frame | ret | callers | role |
|---|---|---|---|---|---|---|
| B0E0 | B38B | 0x2ab | bp frame, locals 20 | `ret` | - (thunk B0B6) | ENTRY. The whole show: archive, title, selection B38B, VERSUS cards, fighter sheets, arena, bookkeeping, fight B7FE, turn mark, outcome BDEA, clean-up (D, section 6) |
| B38B | B4A3 | 0x118 | bp frame, locals 14 | `ret` | B0E0 | selection: needs 20 talents; 6 random free gladiators of 0..9 into a 3x2 card grid; player picks his + training, then the opponent + training; returns -1, or 0 = cancelled (D) |
| B4A3 | B4DA | 0x037 | bp frame, locals 0 | `ret 0x2` | B38B | release(lineup): status 3 -> 0 for the 6 lineup records (D) |
| B4DA | B67C | 0x1a2 | bp frame, locals 120 | `ret 0x2` | B38B | pick(lineup): click a card (Esc = cancel); status 1; the card becomes a random beast 10+rng(3); "How Much Training?" menu, cost 10*(level+1); returns record or FF (D) |
| B67C | B6EC | 0x070 | no bp frame | `ret` | B0E0 B38B | draw the card grid: black, 16 hlines (DS:7BDA), 14 frame pieces (DS:7C1A), copy to screen (D) |
| B6EC | B7FE | 0x112 | bp frame, locals 50 | `ret 0x6` | B0E0 B38B | card(pos, record, facing): name + Roman generation, "Bouts Won" + numeral, portrait (D) |
| B7FE | BB3B | 0x33d | bp frame, locals 16 | `ret 0x2` | B0E0 | fight(opponent): health rolls, crowd meter, main loop (4-tick frames, 6-frame animation steps), keys via jump table B91C, AI BC4C, hits BBC8, boredom (D) |
| BB3B | BBC8 | 0x08d | bp frame, locals 0 | `ret 0x2` | B7FE BBC8 BDEA | crowd(delta): meter 0..30, bar C149; 4 exchanges in a row -> mood up, 4 dull spells -> mood down; music DS:800A[mood] (D) |
| BBC8 | BC4C | 0x084 | bp frame, locals 2 | `ret 0x4` | B7FE | exchange(attacker, defender): attack zone vs guard zone within 20 px: hit (health -= attacker.training + 1) or block; crowd +1 either way (D) |
| BC4C | BDEA | 0x19e | bp frame, locals 2 | `ret 0x2` | B7FE | opponent AI by style (DS:7E44): paced by difficulty DS:0490 (D, section 6) |
| BDEA | C0F9 | 0x30f | bp frame, locals 20 | `ret 0x4` | B0E0 | outcome(player, opponent): wins/losses, score, beast kills, the thumbs vote, deaths (D, section 6) |
| C0F9 | C149 | 0x050 | bp frame, locals 2 | `ret 0x2` | BDEA | death(record): wins = losses = 0, generation + 1 (the successor "II", "III"...), humans get a new body rng(2) and style rng(4), status 0 (D) |
| C149 | C1A4 | 0x05b | bp frame, locals 0 | `ret 0x2` | B7FE BB3B BDEA | crowd bar: width 145*v/30 in white at (82,13)-(82+w,17), rest in [6346] (D) |
| C1A4 | C25F | 0x0bb | bp frame, locals 2 | `ret` | B0E0 B7FE BDEA | draw both fighters over arenabtm, the fallen one first (underneath); copy rows 126..185 to the screen (D) |
| C25F | C288 | 0x029 | bp frame, locals 0 | `ret 0x2` | B7FE | health bar of side n (0 = O.health, 1 = P.health) via C302 (D) |
| C288 | C302 | 0x07a | bp frame, locals 0 | `ret 0x4` | B0E0 | set-up(player, opponent): training, move tables, dying states, positions P (192,175), O (127,175) (D) |
| C302 | C385 | 0x083 | bp frame, locals 0 | `ret 0x4` | C25F | health bar(health, side): w = health*128/50; red (colour 4) from the left (side 0) or right edge, rest grey (8; 0x36 when [9B26] = 2, VGA) at y 188..193 (D) |

## 4. Call graph inside the section (D)

| target | sites | called from (call sites) |
|---|---|---|
| B38B | 1 | B1C8 |
| B4A3 | 3 | B424 B470 B494 |
| B4DA | 2 | B416 B481 |
| B67C | 3 | B20B B3AC B430 |
| B6EC | 4 | B22B B23B B3FF B43F |
| B7FE | 1 | B30D |
| BB3B | 4 | BAEB BC3B C0BD C0C9 |
| BBC8 | 2 | BA72 BAA9 |
| BC4C | 1 | B9CC |
| BDEA | 1 | B321 |
| C0F9 | 2 | C0DF C0EE |
| C149 | 3 | B8B3 BB60 BFEE |
| C1A4 | 4 | B2EA BA61 BF8E C0A8 |
| C25F | 4 | B876 B87C BA78 BAB0 |
| C288 | 1 | B2E7 |
| C302 | 1 | C27E |

Tree: B0E0 -> B38B (-> B67C, B6EC, B4DA, B4A3), B67C, B6EC, C288, C1A4, B7FE (-> C25F -> C302, C149, BB3B -> C149,
BBC8 -> BB3B, BC4C, C1A4), BDEA (-> C1A4, C149, BB3B, C0F9). Every function is reached; no dead code (D).

## 5. Calls out to the EXE (D for the call sites; names from the notes, (G) = guessed here)

| EXE | calls | name | called from |
|---|---|---|---|
| 00D7 | 3 | timer_start(n) | B7FE BBC8 |
| 00F8 | 3 | timer0_reset | B0E0 B7FE BDEA |
| 0183 | 1 | ticks_since(n) | B7FE |
| 01AE | 3 | ticks_since(0) | B0E0 B7FE BDEA |
| 03AA | 2 | key_read | B0E0 B7FE |
| 04ED | 3 | key_flush | B7FE BDEA |
| 0644 | 1 | mouse_poll: [B2] buttons, [B6] x, [B8] y | BDEA |
| 076B | 13 | rng_roll(n) | B38B B4DA B7FE BC4C C0F9 |
| 0F7B | 1 | archive open (6 = MGLAD) | B0E0 |
| 11A9 | 2 | archive close | B0E0 |
| 1273 | 1 | load CBM member into a descriptor (coloseum.cbm) | B0E0 |
| 13A8 | 1 | load_picture(n, &archive): picture slot DS:0250+8n (slot 4 = arenabtm.cbm -> C0BE) | B0E0 |
| 163B | 4 | load xxxHDR/xxxDAT sprite sheet | B0E0 |
| 19A0 | 6 | free sprite sheet | B0E0 |
| 1BE6 | 1 | cursor_move(x, y) | BDEA |
| 1CCE | 4 | cursor_show | B38B B4DA BDEA |
| 1D19 | 6 | cursor_hide | B38B B4DA |
| 1D55 | 1 | cursor push/hide | B0E0 |
| 1D8A | 2 | cursor pop | B0E0 |
| 3233 | 1 | box: 7E74 fill + 925C bevel lines (G from the EXE code); here the crowd-meter frame (79,10)-(234,20) in [6346] | B7FE |
| 359B | 1 | wait for click or key (key -> [C07F]) | B4DA |
| 35DB | 1 | wait until the buttons are released | B4DA |
| 54C8 | 1 | menu_run(menu, sel) -> index or 0xFF | B4DA |
| 5614 | 1 | menu_draw(menu, mode) | B4DA |
| 585B | 1 | menu_preselect(menu, sel, 0) | B4DA |
| 59D4 | 9 | message_box(str) | B0E0 B38B B4DA BDEA |
| 5D20 | 3 | music_stop | B0E0 B7FE |
| 5D29 | 6 | music_play(n) | B0E0 BB3B |
| 5D73 | 4 | music_load(n) | B0E0 |
| 5E1A | 9 | music_unload(n) | B0E0 |
| 6738 | 2 | roman_numeral(n) -> DS:C072 | B6EC |
| 67E4 | 4 | draw into the work buffer | B0E0 B67C C1A4 |
| 680F | 6 | target = the screen | B0E0 B67C C1A4 |
| 6973 | 4 | copy rect work buffer -> screen | B0E0 B67C C1A4 |
| 7330 | 6 | score_add(n) | BDEA |
| 7E74 | 8 | fill rect (x0, y0, x1, y1) in [9AFA] | B0E0 B67C C149 C302 |
| 7ECA | 1 | hline | B67C |
| 8238 | 3 | in_rect(rect, y, x) | B4DA BDEA |
| 86A0 | 1 | blit (86A0) | B0E0 |
| 8790 | 2 | blit picture (desc C0BE, point DS:7BC8 = (0, 126), target C0D0) | B0E0 C1A4 |
| 8BE4 | 4 | centred_text(str, cx, y) | B0E0 B6EC |
| 8E62 | 8 | sprite(desc, x, y) | B0E0 B67C B6EC BDEA C1A4 |
| 9DF2 | 2 | strcat | B4DA B6EC |
| 9E32 | 2 | strcpy | B4DA B6EC |

## 6. The show, decoded (D; read from the listing, not yet run)

**B0E0.** Open MGLAD; arenabtm into picture slot 4; if coloseum.cbm loads, show it full screen. Sheets "show", "arena".
Cursor push, text colour 15, unload song 1 (the map music; not reloaded here), load 4..7, play 4, wait until 432 ticks
(about 5.9 s), a key or a button. Play 7; `B38B()`; if 0: unload 4..7, free both sheets, close, cursor pop, music off,
return (nothing paid, no turn mark). Else play 5, card grid + "VERSUS" at (160, 125), player card at grid position 5
(facing right), opponent at 3 (facing left); free "show"; load the two fighter sheets; close the archive; play 4;
**message box with an empty string DS:7BC6** (a pause, G); draw the arena (sheet frame 0 + arenabtm at y 126);
`C288(player, opp)`; `C1A4()`; play 5; **W(C100)++** (games held); **if (SB(0ED4) > 10) B(0ED4) = 10; W(0ED8) = 0**
(the "Rome is bored" crisis ends, its chance drops to 10); `B7FE(opp)`; **W(154C) = W(634C)** (one show per turn);
`BDEA(player, opp)`; music off, unload 4..7; **message box with an empty string DS:7BC7**; status of both records = 0;
free the arena and fighter sheets; cursor pop.

**B38B (selection).** `if (W(634A) < 20) { "You Cannot Afford\nTwo Gladiators."; return 0; }`. Remember the treasury.
For 6 slots: r = rng(10), probe r, r+1, ... (mod 10) for a record with status 0, set it to 3 and draw its card facing
right. "Select a Gladiator\nfor You to Control"; `p = B4DA(lineup)`; FF -> release, return 0. Redraw the 6 cards facing
left (the picked card now shows a beast). If the treasury is now < 10: "You Cannot Afford a Second Gladiator.\nYour
Talents for the First are Refunded.", treasury = remembered value, status[p] = 0, release, return 0. Else "Select a
Gladiator\nTo Fight Against"; `o = B4DA(lineup)`; FF -> status[p] = 0, release, return 0; else release, return -1.

**B4DA (pick a card).** Wait for a click or key (Esc = cancel, any other key uses the mouse position); the card rects are
x = 7 + 106*(i%3), y = 2 + 100*(i/3), 91x91. The clicked record gets status 1 and **its grid slot is replaced by beast
10 + rng(3)**. Menu DS:7CB0 "How Much Training?" (rect (91,79)-(241,197)) with 4 items "<label> (n0 Talents)",
hotkeys 1..4: labels "Trainee", "Slave", "Veteran", "Master" for men, "Fat", "Well-Fed", "Hungry", "Ravenous" for a
beast (style 4); the cost digit is patched into the static string DS:7B68 " (20 Talents)" (byte +2 = '1'..'4').
Choice c: if `10*(c+1) > W(634A)`: "You Cannot Afford Such\na Well-Trained Gladiator" and ask again; else
`training[rec] = c; W(634A) -= 10*(c+1)`. Esc in the menu returns FF.

**Fighters (C288, B7FE).** Per fighter: move table (4-byte states {dx, next state, frame, flags}; flags bit 0 = idle,
accepts a command; bits 1-2 = attack zone 2 low / 4 high; bits 3-4 >> 2 = guard zone), training, dying state (7ED0/
7ED2 by body: 14 man, 12 wimp, 6 cat), health = 10 + sum of `training` rolls of rng(10) (10..37). Crowd meter starts at
P.training + O.training + 2, mood 1. The loop runs in 4-tick frames; each fighter advances one animation state every 6
frames; the player's command is read every (4 - P.training)*4 frames, the AI every (4 - O.training)*2 frames.
Player commands (state idle, health > 0; health <= 0 -> state 9 = fall), from the arrow bits [A2] through the jump table
B91C: Right 1 -> state 2 (step back, if x < 290), Left 2 -> 1 (step in, if x > 30), Down 4 -> 5 (low guard), Down+Left 6
-> 4 (low attack), Up 8 -> 6 or 7 at random (high guard, only from state 0), Up+Left 10 -> 3 (high attack), anything else
-> 0. When a fighter's step ends, BBC8 tests his state against the other's. No exchange for 216 ticks (3 s) -> crowd -1.
The fight ends when a fighter's state reaches FF (the `next` after the last fall frame): at once if the other is
idle (state 0 or FF, baf5..bb1f), otherwise after one more frame (the loop-top test b8bf). No key quits the fight (the
key read at b8b9 is discarded).

**AI (BC4C).** Health <= 0 -> state 4 (beast) or 9. Else `B(800D) += O.training + B(0490) + 5` (DS:0490 = the gladiator
difficulty, 048E[2]); it acts only when the sum reaches 30 (then 0). Range: close = P.x - O.x <= 20, medium <= 36.
Style 0: close -> rng(2) ? high attack 3 : low attack 4, else step in. Style 1: medium -> step in; close -> DS:7FF6[rng(10)]
= {6,6,7,7,5,5,4,3,2,2}; far -> step back. Styles 2 and 3 (same code): medium -> step in; close -> DS:8000[rng(10)] =
{6,7,5,4,3,1,2,5,4,3}; far -> a guard (5, 6 or 7). Beast (4): health > 5: medium -> pounce (1), close -> attack (3), far ->
1; health <= 5: close or cornered (x < 30) -> attack, else back off (2). Men stepping in past x 290 attack low instead,
stepping back below x 30 guard instead; beasts attack in both cases.

**Outcome (BDEA).** `pleased = (crowd > 2*(P.training + O.training + 2))`.
- Player alive (P.health > 0): wins[p]++, score +3; if O.training > P.training: score + (O.training - P.training), +3.
  Dead: losses[p]++.
- Opponent alive: wins[o]++; if P.training > O.training: score + (P.training - O.training), +3; if o is a beast, **your
  gladiator dies** (no vote). Dead: losses[o]++; a dead beast dies (no vote).
- Both dead: score +3 and both die.
- Otherwise (a man lost to a man): the winner walks over, the thumbs box (arena frame 3 at (79,20)) and "Thumbs Up or
  Down?". Click the left half (79,20)-(156,81) or Up = thumbs up, the right half (157,20)-(234,81) or Down = thumbs down.
  Down: the loser is killed (kill animation 8, the loser goes to his dying state), even if the loser is your own man.
  Score and crowd: if the crowd was pleased, Up +5 and Down -5, else Down +5 and Up -5.
- Every death runs C0F9: the record's successor ("Flavius II") with 0 wins, a new body and style (men).

## 7. DS addresses, absolute operands (D; r = read, w = written, rw = read-modify-write)

| DS | r | w | rw | functions | meaning |
|---|---|---|---|---|---|
| 00A2 | 3 | 0 | 0 | B7FE BDEA | arrow bits (INT 9 hook): Up 8, Down 4, Left 2, Right 1 |
| 00B2 | 2 | 0 | 0 | B0E0 BDEA | mouse buttons |
| 00B6 | 4 | 0 | 0 | B4DA BDEA | mouse x |
| 00B8 | 4 | 0 | 0 | B4DA BDEA | mouse y |
| 00BA | 2 | 0 | 0 | BDEA | mouse present |
| 0490 | 1 | 0 | 0 | BC4C | difficulty, gladiators (048E[2], save block) |
| 0ED4 | 1 | 1 | 0 | B0E0 | "Rome is bored" chance (capped at 10) |
| 0ED8 | 0 | 1 | 0 | B0E0 | "Rome is bored" crisis flag (cleared) |
| 154C | 0 | 1 | 0 | B0E0 | last show turn (= W(634C), after the fight) |
| 6346 | 2 | 0 | 0 | B7FE C149 | box colour |
| 634A | 4 | 1 | 1 | B38B B4DA | treasury |
| 634C | 1 | 0 | 0 | B0E0 | turn |
| 7C7E | 2 | 0 | 0 | B4DA | pointer to " (20 Talents)" (DS:7B68) |
| 800D | 1 | 1 | 1 | BC4C | AI pace accumulator (static, never reset) |
| 9AFA | 0 | 10 | 0 | B0E0 B67C C149 C25F C302 | fill colour |
| 9B26 | 1 | 0 | 0 | C302 | video mode (2 = VGA) |
| 9B69 | 0 | 1 | 0 | B0E0 | text colour (15) |
| AA50 | 6 | 1 | 0 | B0E0 B38B | opponent record |
| AA51 | 6 | 1 | 0 | B0E0 B38B | player record |
| AA52 | 7 | 1 | 0 | B7FE BDEA C1A4 C288 | P.movetab (fighter struct 0x18 bytes: +0 movetab, +2 x, +4 y, +6 state, +7 state copy, +8 training, +9 dying state, +A health) |
| AA54 | 10 | 3 | 1 | B7FE BC4C BDEA C1A4 C288 | P.x (192) |
| AA56 | 2 | 1 | 0 | C1A4 C288 | P.y (175) |
| AA58 | 13 | 16 | 0 | B7FE BDEA C1A4 C288 | P.state |
| AA59 | 0 | 1 | 0 | B7FE | P.state copy (written, never read here) |
| AA5A | 7 | 1 | 0 | B7FE BDEA C288 | P.training |
| AA5B | 2 | 1 | 0 | BDEA C288 | P.dying state |
| AA5C | 3 | 1 | 1 | B7FE BDEA C25F | P.health |
| AA5E | 0 | 1 | 1 | B7FE BBC8 | hits landed (never read) |
| AA60 | 1 | 3 | 1 | B7FE BB3B | run of exchanges |
| AA62 | 5 | 3 | 1 | B7FE BB3B BDEA | crowd meter 0..30 |
| AA64 | 0 | 1 | 1 | B7FE BBC8 | blocks (never read) |
| AA66 | 2 | 3 | 2 | B7FE BB3B | crowd mood 0..2 |
| AA68 | 1 | 3 | 1 | B7FE BB3B | run of dull spells |
| AA6A | 7 | 1 | 0 | B7FE BDEA C1A4 C288 | O.movetab (second fighter struct) |
| AA6C | 13 | 2 | 1 | B7FE BC4C BDEA C1A4 C288 | O.x (127) |
| AA6E | 2 | 1 | 0 | C1A4 C288 | O.y (175) |
| AA70 | 16 | 15 | 0 | B7FE BC4C BDEA C1A4 C288 | O.state |
| AA72 | 9 | 1 | 0 | B7FE BC4C BDEA C288 | O.training |
| AA73 | 3 | 1 | 0 | BDEA C1A4 C288 | O.dying state |
| AA74 | 4 | 1 | 1 | B7FE BC4C BDEA C25F | O.health |
| C07F | 1 | 0 | 0 | B4DA | last key (359B) |
| C100 | 0 | 0 | 1 | B0E0 | games held (victory statistics) |
| C2B4 | 5 | 1 | 0 | B0E0 B67C BDEA | sheet "arena" |
| C2B6 | 3 | 1 | 0 | B0E0 C1A4 | sheet, player fighter |
| C2B8 | 3 | 1 | 0 | B0E0 C1A4 | sheet, opponent fighter |
| C2BA | 3 | 1 | 0 | B0E0 B6EC | sheet "show" |

(AA50..AA74 and C2B4..C2BA are zero in the static image: run-time variables.)

## 8. DS tables indexed by a register (D)

| DS base | r | w | rw | index | known |
|---|---|---|---|---|---|
| 7BCC | 1 | 0 | 0 | bx | player sheet names by body: rman, rwimp |
| 7BD0 | 1 | 0 | 0 | bx | opponent sheet names by body: lman, lwimp, lcat0, lcat1, lcat2 |
| 7C6E | 1 | 0 | 0 | bx | training labels (men) |
| 7C76 | 1 | 0 | 0 | bx | training labels (beasts) |
| 7E3E | 1 | 0 | 0 | bx | gladiator record +0: name pointer (13 records of 10 bytes) |
| 7E40 | 2 | 0 | 1 | bx | +2 generation (1 = no numeral; beasts 0) |
| 7E41 | 2 | 1 | 2 | bx | +3 bouts won |
| 7E42 | 0 | 1 | 2 | bx | +4 bouts lost (counted, never shown) |
| 7E43 | 2 | 7 | 0 | bx, di | +5 status: 0 free, 1 picked, 3 in the lineup |
| 7E44 | 7 | 1 | 0 | bx | +6 style 0..3, 4 = beast |
| 7E45 | 6 | 1 | 0 | bx, si | +7 body: 0 man, 1 wimp, 2..4 cats |
| 7E46 | 2 | 1 | 0 | bx, si | +8 training bought (0..3) |
| 7ED0 | 1 | 0 | 0 | bx | player dying state by body {14, 12} |
| 7ED2 | 1 | 0 | 0 | bx | opponent dying state by body {14, 12, 6, 6, 6} |
| 7FE8 | 1 | 0 | 0 | bx | player move tables {7ED8 man, 7F18 wimp} |
| 7FEC | 1 | 0 | 0 | bx | opponent move tables {7F50, 7F90, 7FC8 x3 cats} |
| 7FF6 | 1 | 0 | 0 | bx | AI close-range moves, style 1 |
| 8000 | 1 | 0 | 0 | bx | AI close-range moves, styles 2/3 |
| 800A | 1 | 0 | 0 | bx | music by crowd mood {7 cn4d, 5 cn4b, 6 cn4c} |
| B91C (CS) | 1 | 0 | 0 | bx | the jump table in the code (section 1) |

Record +9 is never used. Records 0..9 (DS:7E3E-7EA1, 0x64 bytes) are the save block at CSV 0x594 (SAVE_FORMAT); the
beasts 10..12 are not saved.

## 9. Static data and every string (D; OVL0.OVL 0xB400 = DS:0000)

The gladiator block is DS:7B06-0x801F.

| DS | used by | content |
|---|---|---|
| 7B06 7B0B 7B11 7B16 7B1C 7B22 7B28 | 7BCC/7BD0 -> 163B | "rman", "rwimp", "lman", "lwimp", "lcat0", "lcat1", "lcat2" |
| 7B2E 7B36 7B3C 7B44 | 7C6E | "Trainee", "Slave", "Veteran", "Master" |
| 7B4B 7B4F 7B58 7B5F | 7C76 | "Fat", "Well-Fed", "Hungry", "Ravenous" |
| 7B68 | 7C7E, B5D8 | " (20 Talents)" (digit patched to 1..4) |
| 7B76 | menu 7CB0 title | "How Much Training?" |
| 7B89 | B108 | "coloseum.cbm" |
| 7B96 | B157 | "show" |
| 7B9B | B168 | "arena" |
| 7BA1 | B37F | "Can't Load Gladiator Graphics" |
| 7BBF | B20E | "VERSUS" |
| 7BC6, 7BC7 | B294, B342 | "" and "" (two empty message boxes) |
| 7BC8 | B2C2, C1B2 | point (0, 126) for 8790 |
| 7BCA | - | 0x007E (rest of the point; 7BCC follows) |
| 7BDA-7C19 | B695 | 16 hlines {colour, y}: colour 8 at y 0/95/100/195, 0 at 1/94/101/194, 7 at 96/99/196/199, 15 at 97/98/197/198 |
| 7C1A-7C6D | B6B8 | 14 sprites {x, y, frame}: frame 1 at x -11/94/201/307, y 2/102; frame 2 at x 16/122/228, y 86/186 |
| 7C6E, 7C76, 7C7E | B4DA | label pointer tables, cost string pointer |
| 7C80-7CAF | menu items | 4 x {text (stack buffer, set at run time), hotkey '1'..'4', rect (6, 23+10i)-(138, 33+10i)} |
| 7CB0 | B601 B617 B652 | menu: rect (91,79)-(241,197), items 7C80, title 7B76, 4 items |
| 7CC2 | B39A | "You Cannot Afford\nTwo Gladiators." |
| 7CE4 | B40B | "Select a Gladiator\nfor You to Control" |
| 7D0A | B452 | "You Cannot Afford a Second Gladiator.\nYour Talents for the First are Refunded." |
| 7D59 | B476 | "Select a Gladiator\nTo Fight Against" |
| 7D7D | B65D | "You Cannot Afford Such\na Well-Trained Gladiator" |
| 7DAD | B778 | "Bouts Won" |
| 7DB8-7E3B | records | "Flavius ", "Marcus ", "Brutus ", "Marius ", "Tillius ", "Cassius ", "Flaminius ", "Spartacus ", "Julius ", "Publius ", "Black Panther ", "Wild Lioness ", "Snow Leopard " (the trailing space is for the numeral) |
| 7E3E-7EBF | records | 13 gladiators (table below) |
| 7EC0, 7EC8 | C02A, C042 | rects thumbs up (79,20)-(156,81), thumbs down (157,20)-(234,81) |
| 7ED0, 7ED2 | C288 | dying states |
| 7ED8-7FE7 | movetabs | move tables (below) |
| 7FE8-7FF5 | C288 | move table pointers |
| 7FF6, 8000, 800A, 800D | BC4C, BB3B | AI tables, mood music, AI pace (0) |
| 800E | BFFB | "Thumbs Up or Down?" |

Gladiators (static image; gen = generation, st = status, sty = style, trn = training):

| # | DS | name | gen | wins | lost | st | sty | body | trn | +9 |
|---|---|---|---|---|---|---|---|---|---|---|
| 0 | 7E3E | "Flavius " | 1 | 0 | 0 | 0 | 1 | 1 wimp | 0 | 0 |
| 1 | 7E48 | "Marcus " | 1 | 0 | 0 | 0 | 0 | 0 man | 0 | 0 |
| 2 | 7E52 | "Brutus " | 1 | 0 | 0 | 0 | 0 | 0 man | 0 | 0 |
| 3 | 7E5C | "Marius " | 1 | 0 | 0 | 0 | 2 | 0 man | 0 | 0 |
| 4 | 7E66 | "Tillius " | 1 | 0 | 0 | 0 | 1 | 1 wimp | 0 | 0 |
| 5 | 7E70 | "Cassius " | 1 | 0 | 0 | 0 | 2 | 0 man | 0 | 0 |
| 6 | 7E7A | "Flaminius " | 1 | 0 | 0 | 0 | 1 | 1 wimp | 0 | 0 |
| 7 | 7E84 | "Spartacus " | 1 | 0 | 0 | 0 | 3 | 1 wimp | 0 | 0 |
| 8 | 7E8E | "Julius " | 1 | 0 | 0 | 0 | 0 | 0 man | 0 | 0 |
| 9 | 7E98 | "Publius " | 1 | 0 | 0 | 0 | 2 | 1 wimp | 0 | 0 |
| 10 | 7EA2 | "Black Panther " | 0 | 0 | 0 | 0 | 4 | 2 cat0 | 0 | 0 |
| 11 | 7EAC | "Wild Lioness " | 0 | 0 | 0 | 0 | 4 | 3 cat1 | 0 | 0 |
| 12 | 7EB6 | "Snow Leopard " | 0 | 0 | 0 | 0 | 4 | 4 cat2 | 0 | 0 |

Move tables, state: (dx, next, frame, flags) (D, dumped):
- P man 7ED8: 0 (0,0,0,01) 1 (0,12,1,00) 2 (15,15,1,00) 3 (0,10,2,00) 4 (0,15,2,02) 5 (0,5,6,09) 6 (0,6,7,11) 7 (0,7,8,11)
  8 (0,8,5,01) 9 (0,13,9,00) 10 (0,11,3,00) 11 (0,15,4,04) 12 (-16,15,0,00) 13 (0,14,10,00) 14 (0,FF,11,00) 15 (0,0,0,00)
- P wimp 7F18: 0 (0,0,0,01) 1 (0,11,1,00) 2 (7,13,1,00) 3 (0,10,2,00) 4 (0,13,2,02) 5 (0,5,5,09) 6 (0,6,6,11) 7 (0,7,7,11)
  8 (0,8,4,01) 9 (0,12,8,00) 10 (0,13,3,04) 11 (-8,13,0,00) 12 (0,FF,9,00) 13 (0,0,0,00)
- O man 7F50: as P man with dx negated, except **10 (0,11,3,04) 11 (0,15,4,00)**: the hit flag is on state 10, not 11
- O wimp 7F90: P wimp with dx negated
- O cats 7FC8: 0 (0,0,0,01) 1 (0,5,1,00) 2 (-20,7,1,00) 3 (0,7,2,02) 4 (0,6,3,00) 5 (20,7,0,00) 6 (0,FF,4,00) 7 (0,0,0,00)

(The man moves 15-16 px a step, the wimp 7-8, a cat 20; cats have no guard. Guards 5/6/7 and the kill 8 loop on
themselves; the fall runs 9 -> 13 -> 14 -> FF for a man.)

## 10. Findings for the port (D unless marked)

1990 bugs (D from the listing; not yet reproduced in DOSBox):
1. **Talents lost on a cancelled opponent.** After the player has paid for his own gladiator's training (B4DA, b649),
   cancelling the opponent (Esc on the grid or in its training menu) goes b48b -> b45f: status reset and return 0, but
   the refund `W(634A) = saved` (b459) is only on the "cannot afford a second" path. The fee 10..40 is gone, no show
   is held, and as 154C is not written the show can be started again the same turn. Fix: refund on that path too.
2. **A picked gladiator leaks.** B4DA sets status 1 on the clicked record before the training menu; if that menu is
   cancelled, B38B only releases status-3 records (B4A3), so the record stays at 1 for good (and is saved: block
   7E3E). Same for the opponent's record when its menu is cancelled. A status-1 record is never offered again. After
   5 leaks, 6 free records out of 10 no longer exist: the probe loop (b3c3..b3e4) falls through without writing the
   lineup slot, so the lineup holds an uninitialised stack byte that B4DA then uses as a record index (writes
   7E43 + 10*garbage). Fix: reset status[rec] when B4DA returns FF (and when B38B backs out).
3. (G, odd) The opponent man's high attack connects one animation step earlier than the player man's (O man state 10
   carries flag 04, P man state 11; both wimps use state 10). Possibly a data slip in the P man table; check the RMAN/LMAN
   frames 3/4 before calling it a bug.
4. (G, odd) Styles 2 and 3 share all AI code (only Spartacus starts with style 3).
5. (G, check in DOSBox) In that one extra frame the fallen fighter is moved and drawn with state FF, i.e. with the 4
   bytes at movetab + 0x3FC, past every table. In the static image those are P man (0,0,0,0) (harmless), P wimp
   (0,0,FF,FF) (sprite frame 255; and flags bit 0 lets b902 set state 9 again), O man (dx +60, frame 0), O wimp (dx +6),
   O cat (dx +28, frame 20 of a 5-frame sheet). BDEA then resets the loser to his dying state, so at most one frame
   would be wrong. Fix if confirmed: treat FF as the dying state when drawing.
6. (G) An upset pays the player either way: a better-trained opponent beaten, or the player's better-trained man beaten,
   both give the difference + 3. Probably meant ("the crowd loves an upset"); keep.

Other notes:
- The once-per-turn word **154C is written by this module** (b313, after the fight; a cancelled selection writes
  nothing), contrary to the comment in port/src/province.c lines 16-17. The amphitheatre purchase (EXE 0x34CB-0x34D6)
  is in the EXE's Rome Games, not here.
- Difficulty DS:0490 only paces the AI (BC4C). The player's reaction interval is (4 - training)*4 frames, the AI's
  (4 - training)*2 frames plus the pace accumulator.
- Song 1 (map music) is unloaded at the start and not reloaded in the section (the naval battle does the same).

Easter eggs and jokes (texts quoted in full in section 9):
- The roster: **Brutus, Cassius and Tillius** (Caesar's assassins, Tillius Cimber) fight beside **Julius**, plus
  **Spartacus** (G for the intent). The player can stage Julius vs Brutus.
- Fighter bodies are literally named **"man" and "wimp"** (RMAN/RWIMP/LMAN/LWIMP sheets); Spartacus, Flavius, Tillius,
  Flaminius and Publius start as wimps.
- The beasts: "Black Panther", "Wild Lioness", "Snow Leopard", trained as "Fat", "Well-Fed", "Hungry" or
  "Ravenous" - the hungrier the beast, the higher the price (10..40 talents). Beasts never enter the lineup by
  themselves: one only appears in the card you just took for yourself, so an animal can only be your opponent.
- "Slave" is the second training grade, above "Trainee" and below "Veteran" and "Master".
- Dead gladiators are succeeded by namesakes with a Roman numeral ("Flavius II"); roman_numeral (EXE 6738) would print
  "OOPS" past 999, unreachable with a byte.
- Thumbs: Up/Down arrows or the two halves of the box; the crowd wants mercy after a good fight and blood after a dull
  one.
- "coloseum.cbm" is spelt with one s (file name only).
