# Naval battle: OVL0 section 5 index

OVL0.OVL file bytes 0x8550-0xB3F0 (0x2EA0 bytes), loaded at 1331:0000 = 0823:B0E0, code 0xB0E0-0xDF79, zero padding
0xDF7A-0xDF7F. Listing: `disasm/OVL_NAVAL.asm` (same format as `disasm/OVL_BATTLE.asm`; absolute DS operands printed
unsigned, calls masked to 16 bits, EXE names and DS strings as comments). Marks: D decoded from the asm, V verified by
running/comparing, G guess.

The port (port/src/naval.c, naval_gfx.c) has been compared with two battles captured in DOSBox-X on 10 Nov
(`tools/dosbox/results/naval/README.md`): the set-up (BD8A, b152..b231), BB7E, b419..b4c3, D3F2, D60C, DA50 (one round),
the write-back, B59F and B0E0 give the game's DS game state byte for byte (the clock aside: NAVAL_LOGIC_NOTES 17) and
every pixel the port draws itself as the game's (the cursor aside; B59F's box was never on a captured screen); the duel
passes could not be replayed (NAVAL_LOGIC_NOTES 17, NAVAL_SCREEN_NOTES 18).

## 1. How the listing was made (V)

- `sec5.bin` = OVL0.OVL[0x8550:0xB3F0]; `.code16` + `.incbin`, clang `--target=i386-unknown-elf -c`, llvm-objdump
  `-d -M intel --triple=i386-unknown-unknown-code16 --adjust-vma=<start>`.
- The linear sweep is right everywhere except at 7 word-alignment pad bytes (00) after a `ret`: B7F9, BD89, C4D7, C845,
  D241, D3F1, DA4F, where `00 55 8B EC` decodes as `add [di-0x75],dl; in al,dx` and the next prologue is lost (the D0F1/
  D0F2 effect of BATTLE_UI_NOTES). The listing is therefore made per function: 54 starts = every `push bp; mov bp, sp`
  (46) + every near-call target without a prologue (BE0F C105 C1BB C312 C34F C366 C71B CFDD) + C18C (after a `ret`,
  no caller). Checks: the segments tile 0xB0E0-0xDF7A with no gap or overlap; every jump/call target inside the section
  is an instruction start; every function ends in `ret`/`ret n` (then at most one 00 pad); no indirect jumps or calls,
  no `cs:` jump tables, no `lcall`: there is NO data inside the code apart from the pad bytes (V).
- Out-of-section calls are near calls into EXE segment 0823 (the overlay runs in the same CS).

## 2. Entry points (D)

| entry | EXE thunk | called from | signature |
|---|---|---|---|
| B0E0 | B09E | 0x2C24 (fleet move 2B40) | B0E0(Roman fleet [bp+4], sea zone [bp+6]), ret 4; loops enemy fleets 3..5 in the zone, calls B12A(f, Roman fleet); returns si = 1 unless a battle result had bit 1 clear |
| B12A | B0C2 | 0x4B22 (4ADA/4B3F), 0x5221 (End Turn, enemy.c) | B12A(enemy fleet [bp+4], Roman fleet [bp+6]), ret 4; returns si = outcome code (section 6) |

B0E0 in full (D): `for (f = 3; f < 6; f++) if (B(0x1114+12f) == zone && B(0x1116+12f) != 0xFF) { r = B12A(f, roman);
if (!(r & 2)) { si = 0; break; } }` (the asm: b0fe cmp [bx+0x1114], b104 cmp [bx+0x1116], -1; b113 call b12a; b116
`test al, 2; je b11f`; b11f `sub si, si`). So the way is clear only if every enemy fleet there was destroyed (bit 1).
A result of 4 (retreat) or 1 (lost) also stops the loop with 0; B12A returns 0 when the set-up BD8A is cancelled
(b14b `or ax, ax; jne b152; jmp b592`).

## 3. Functions (54)

Size = bytes up to the pad. Frame: `bp frame, locals n` = `push bp; mov bp, sp; sub sp, n`. Args = ret n / 2.

| func | end | size | frame | ret | callers | role |
|---|---|---|---|---|---|---|
| B0E0 | B12A | 0x04a | bp frame, locals 6 | `ret 0x4` | - | ENTRY (thunk B09E). Fleet enters sea zone [bp+6]: for enemy fleets f = 3..5 with 1114+12f == zone and 1116+12f != FF, B12A(f, Roman fleet [bp+4]); stops when the result has bit 1 clear; returns 1 (si) = way clear (D) |
| B12A | B59F | 0x475 | bp frame, locals 24 | `ret 0x4` | B0E0 | ENTRY (thunk B0C2). The naval battle: set-up, duel loop, fleet battle, outcome (D for the outcome tail, see section 6) |
| B59F | B6A0 | 0x101 | bp frame, locals 4 | `ret 0x6` | B12A | outcome text: builds "<A> met the <B> on the gently rolling waves of <sea>." + flavour + result into AC60, message box (D) |
| B6A0 | B7F9 | 0x159 | bp frame, locals 8 | `ret` | B12A | enemy ship AI step (rng, C846) (G) |
| B7FA | B8E3 | 0x0e9 | bp frame, locals 6 | `ret 0x2` | B12A | timed movement/heading update (C4D8/C4E8 lookups) (G) |
| B8E3 | B96A | 0x087 | bp frame, locals 8 | `ret 0x2` | BB7E BEAE D3F2 | draw a ship sprite (G) |
| B96A | BA7A | 0x110 | bp frame, locals 14 | `ret` | B12A | draw both ships / oars (G) |
| BA7A | BB7E | 0x104 | bp frame, locals 12 | `ret 0x2` | B12A B6A0 | draw projectile / effect (D070 hit test) (G) |
| BB7E | BD41 | 0x1c3 | bp frame, locals 6 | `ret 0x6` | B12A | event with message box and dice (ram/board?) (G) |
| BD41 | BD89 | 0x048 | bp frame, locals 2 | `ret 0x4` | BB7E | dice helper for BB7E (2x rng_roll) (G) |
| BD8A | BE0F | 0x085 | bp frame, locals 2 | `ret 0x4` | B12A | battle set-up: music off, C274 pick the ship, flagship slots C37A/C37B, BF50 load, C105 intro, BE0F init (D/G) |
| BE0F | BEAE | 0x09f | no bp frame | `ret` | B12A BD8A | init duel state (positions from 971A/971E, 56FE, ship strength by 9819 table) (D) |
| BEAE | BF50 | 0x0a2 | bp frame, locals 0 | `ret 0x4` | B12A | panel / text box (691B, 8BE4) (G) |
| BF50 | C105 | 0x1b5 | bp frame, locals 16 | `ret` | BD8A | load graphics + music: galleys.lzw, oars1, oars2, galleys.cbm, bombs, smok, collide.lzw, flag (D) |
| C105 | C18C | 0x087 | no bp frame | `ret` | BD8A | intro screen loop until key/click, palette cycling via C1BB (D) |
| C18C | C1BB | 0x02f | no bp frame | `ret` | - | set palette from 9762/9778 tables; NO CALLER in the section (dead code?) |
| C1BB | C274 | 0x0b9 | no bp frame | `ret` | C105 | palette cycling of the water colours (01B6 + 84BB), timer-paced (D) |
| C274 | C312 | 0x09e | bp frame, locals 10 | `ret 0x2` | BD8A | choose the Roman ship (54C8, 5614); returns slot or FF = cancel (G) |
| C312 | C34F | 0x03d | no bp frame | `ret` | B12A | free the sheets (19A0) + 74B7 (D) |
| C34F | C366 | 0x017 | no bp frame | `ret` | B12A | music stop + unload songs listed at 9704+1..4 (D) |
| C366 | C3D3 | 0x06d | no bp frame | `ret` | B12A | player input: joystick/keys [A2] bits, keys 0x72, 0x0D, 0x10, 0x20, 0x2B (G) |
| C3D3 | C422 | 0x04f | bp frame, locals 0 | `ret 0x2` | B12A | timer-paced step (G) |
| C422 | C4D7 | 0x0b5 | bp frame, locals 0 | `ret 0x2` | B12A | timer-paced step with music (G) |
| C4D8 | C4E8 | 0x010 | bp frame, locals 0 | `ret 0x2` | B7FA C641 C9A3 CC4D | table lookup W(9878+2*n) (sin?) (G) |
| C4E8 | C4F8 | 0x010 | bp frame, locals 0 | `ret 0x2` | B7FA C641 C9A3 CC4D | table lookup W(989A+2*n) (cos?) (G) |
| C4F8 | C558 | 0x060 | bp frame, locals 6 | `ret 0x8` | B12A | long arithmetic (9D04) -> C558 (G) |
| C558 | C641 | 0x0e9 | bp frame, locals 16 | `ret 0x6` | C4F8 | long division helper (A048) (G) |
| C641 | C71B | 0x0da | bp frame, locals 14 | `ret 0x4` | B12A | heading / vector (9D04, C4D8/C4E8) (G) |
| C71B | C845 | 0x12a | no bp frame | `ret` | B12A BB7E BEAE | status panel: "MEN", numbers (8A9E/8AC8), centred text (D) |
| C846 | C92B | 0x0e5 | bp frame, locals 6 | `ret 0x2` | B6A0 C366 | speed/turn step (050F) (G) |
| C92B | C9A3 | 0x078 | bp frame, locals 0 | `ret 0x2` | B12A | speed step (050F) (G) |
| C9A3 | CB11 | 0x16e | bp frame, locals 16 | `ret 0x2` | B12A | enemy movement step (rng, D11B collision) (G) |
| CB11 | CC4D | 0x13c | bp frame, locals 12 | `ret 0x2` | B12A | shot / projectile step (rng, CC4D, D070) (G) |
| CC4D | CCDE | 0x091 | bp frame, locals 2 | `ret 0x4` | CB11 | projectile vector (C4D8/C4E8) (G) |
| CCDE | CD4B | 0x06d | bp frame, locals 2 | `ret 0x2` | B12A | draw (8E62) (G) |
| CD4B | CD96 | 0x04b | bp frame, locals 0 | `ret 0x2` | B8E3 B96A | draw (8E62) (G) |
| CD96 | CE7E | 0x0e8 | bp frame, locals 6 | `ret` | B12A | move step with rng and CE7E collision (G) |
| CE7E | CF7F | 0x101 | bp frame, locals 38 | `ret` | CD96 | ship-ship collision test (05ED circles_touch, 817C) (G) |
| CF7F | CFDD | 0x05e | bp frame, locals 2 | `ret 0x2` | B12A | per-tick state (G) |
| CFDD | D070 | 0x093 | no bp frame | `ret` | B12A | hit test via D070 (G) |
| D070 | D11B | 0x0ab | bp frame, locals 12 | `ret 0x6` | BA7A CB11 CFDD | point in circle hit test (05B0, 8238) (G) |
| D11B | D241 | 0x126 | bp frame, locals 38 | `ret 0x2` | C9A3 | collision test (05ED, 817C) (G) |
| D242 | D33D | 0x0fb | bp frame, locals 2 | `ret` | B12A | damage / ram resolution (054A same_sign, D33D/D38D/D3BF) (G) |
| D33D | D38D | 0x050 | bp frame, locals 2 | `ret 0x6` | D242 | helper of D242 (G) |
| D38D | D3BF | 0x032 | bp frame, locals 2 | `ret 0x2` | D242 | helper of D242 (G) |
| D3BF | D3F1 | 0x032 | bp frame, locals 2 | `ret 0x2` | D242 | helper of D242 (G) |
| D3F2 | D60C | 0x21a | bp frame, locals 62 | `ret 0x2` | B12A | sinking animation, part 1 (arg = sunk-flags) (G) |
| D60C | D89B | 0x28f | bp frame, locals 52 | `ret 0x2` | B12A | sinking animation, part 2: LSINK/MSINK/SSINK, shark; "Failed to Load Human/Computer Sinking" (D) |
| D89B | D919 | 0x07e | bp frame, locals 0 | `ret 0x2` | D60C | shark / splash draw (rng) (G) |
| D919 | D97C | 0x063 | bp frame, locals 8 | `ret 0x4` | D3F2 D60C | helper of D3F2/D60C (G) |
| D97C | DA4F | 0x0d3 | bp frame, locals 26 | `ret 0x4` | D3F2 | rect combine (81EA) (G) |
| DA50 | DC8E | 0x23e | bp frame, locals 8 | `ret 0x4` | B12A | fleet battle round (6 rng_roll), loops until it returns non-zero (D for the loop, G for content) |
| DC8E | DD2E | 0x0a0 | bp frame, locals 4 | `ret 0x4` | B12A BEAE | print both fleets' ship numbers (8AF2/8B2C) (G) |
| DD2E | DE51 | 0x123 | bp frame, locals 14 | `ret 0x2` | DA50 | fleet battle: losses of one side (C086..C090 casualty counters) (G) |
| DE51 | DF7A | 0x129 | bp frame, locals 18 | `ret 0x2` | DA50 | fleet battle: losses of the other side (C082..C08C) (G) |

Totals: 54 functions, 0x2E9A bytes of code. Biggest: B12A (0x475), D60C (0x28F), DA50 (0x23E), D3F2 (0x21A).

## 4. Call graph inside the section (D)

| target | sites | called from (call sites) |
|---|---|---|
| B12A | 1 | B113 |
| B59F | 1 | B58C |
| B6A0 | 1 | B283 |
| B7FA | 2 | B2A4 B2AA |
| B8E3 | 7 | BC7B BC81 BF2D BF33 D5A9 D5AF D601 |
| B96A | 1 | B344 |
| BA7A | 3 | B34B B353 B7F0 |
| BB7E | 1 | B397 |
| BD41 | 2 | BC34 BC41 |
| BD8A | 1 | B148 |
| BE0F | 2 | B3FA BE03 |
| BEAE | 2 | B239 B405 |
| BF50 | 1 | BDF9 |
| C105 | 1 | BE00 |
| C1BB | 1 | C12B |
| C274 | 1 | BD9D |
| C312 | 1 | B483 |
| C34F | 1 | B58F |
| C366 | 1 | B27D |
| C3D3 | 2 | B28A B290 |
| C422 | 2 | B297 B29D |
| C4D8 | 4 | B858 C679 C9EC CC5E |
| C4E8 | 4 | B89C C692 CA30 CCA0 |
| C4F8 | 1 | B25F |
| C558 | 1 | C549 |
| C641 | 1 | B26D |
| C71B | 4 | B365 B479 BCA0 BF1B |
| C846 | 2 | B721 C3C7 |
| C92B | 2 | B2DE B2E4 |
| C9A3 | 2 | B2EB B2F1 |
| CB11 | 2 | B2D1 B2D7 |
| CC4D | 2 | CB74 CBB9 |
| CCDE | 2 | B35C B362 |
| CD4B | 3 | B960 BA48 BA71 |
| CD96 | 1 | B2C8 |
| CE7E | 1 | CDA0 |
| CF7F | 2 | B2BC B2C2 |
| CFDD | 1 | B2C5 |
| D070 | 4 | BB5A CBCD D024 D062 |
| D11B | 1 | CA78 |
| D242 | 1 | B2B2 |
| D33D | 4 | D281 D28F D2F5 D300 |
| D38D | 2 | D24E D259 |
| D3BF | 2 | D2C3 D2CB |
| D3F2 | 1 | B480 |
| D60C | 1 | B487 |
| D89B | 2 | D7C2 D81C |
| D919 | 4 | D43B D45C D64A D65F |
| D97C | 2 | D484 D575 |
| DA50 | 2 | B310 B4B4 |
| DC8E | 3 | B370 B4C3 BF26 |
| DD2E | 1 | DC1E |
| DE51 | 1 | DC2C |

Callees per function:

- B0E0 -> B12A
- B12A -> B59F B6A0 B7FA B96A BA7A BB7E BD8A BE0F BEAE C312 C34F C366 C3D3 C422 C4F8 C641 C71B C92B C9A3 CB11 CCDE CD96 CF7F CFDD D242 D3F2 D60C DA50 DC8E
- B6A0 -> BA7A C846
- B7FA -> C4D8 C4E8
- B8E3 -> CD4B
- B96A -> CD4B
- BA7A -> D070
- BB7E -> B8E3 BD41 C71B
- BD8A -> BE0F BF50 C105 C274
- BEAE -> B8E3 C71B DC8E
- C105 -> C1BB
- C366 -> C846
- C4F8 -> C558
- C641 -> C4D8 C4E8
- C9A3 -> C4D8 C4E8 D11B
- CB11 -> CC4D D070
- CC4D -> C4D8 C4E8
- CD96 -> CE7E
- CFDD -> D070
- D242 -> D33D D38D D3BF
- D3F2 -> B8E3 D919 D97C
- D60C -> D89B D919
- DA50 -> DD2E DE51

## 5. Calls out to the EXE (D for the call sites; names from the notes, (G) = guessed here from the EXE code)

| EXE | calls | name | called from |
|---|---|---|---|
| 00D7 | 9 | timer_start(n) (copy tick DS:42/44 to slot 46+4n) | B12A B7FA BB7E C3D3 C422 C846 C9A3 CB11 |
| 00F8 | 5 | timer0_reset (= 00D7(0)) | B12A C105 D3F2 D60C |
| 00FF | 1 | timer start helper (00D7/00FF pair) | BEAE |
| 011F | 1 | frame_wait(n): busy-wait until timer 0 + n ticks | BB7E |
| 0183 | 9 | ticks_since(n) | B12A B7FA BB7E C3D3 C422 C846 C9A3 CB11 |
| 01AE | 4 | ticks_since(0) | B12A C105 D3F2 D60C |
| 01B6 | 3 | vga_set_color(idx, r, g, b) (out 3C8/3C9) (G) | C18C C1BB |
| 03AA | 2 | key_read: next key or 0 | C105 C366 |
| 04ED | 2 | key_flush | B59F BB7E |
| 050F | 7 | step_toward_min: max(v - step, min), signed (050F family) | BB7E C846 C92B CB11 CD96 |
| 054A | 2 | same_sign(a, b): ~(a ^ b) & 0x8000 (G) | D242 |
| 05B0 | 1 | point_in_circle(circ{x,y,r}*, y, x): dx^2 + dy^2 <= r^2 -> -1/0 (G) | D070 |
| 05ED | 2 | circles_touch(c1*, c2*): dist^2 <= (r1 + r2)^2 -> -1/0, collision (G) | CE7E D11B |
| 0644 | 1 | mouse_poll: [B2] buttons, [B6] x, [B8] y | C105 |
| 076B | 23 | rng_roll(n) | B12A B6A0 BB7E BD41 C9A3 CB11 CD96 D3F2 D89B DA50 |
| 0F0C | 1 | file helper (stat/open; 18C2, 1172) (G) | BF50 |
| 0F7B | 3 | archive open (0F7B(n, &arch)) | BF50 D60C |
| 11A9 | 3 | archive close | BF50 D60C |
| 11C4 | 1 | archive member (text) load | BF50 |
| 1273 | 1 | load CBM member into a descriptor | BF50 |
| 163B | 10 | sheet_163B: load xxxHDR/xxxDAT sprite sheet | BF50 D60C |
| 19A0 | 10 | sheet_19A0: free sprite sheet | C105 C312 D60C |
| 1D55 | 1 | mouse cursor push/hide | B12A |
| 1D8A | 1 | mouse cursor pop | B12A |
| 4816 | 1 | legion_destroyed (L): +0x0D=0xFF, +0x0C=0xFF, commander status | B12A |
| 54C8 | 1 | wait for buttons/click (54C8 polling) | C274 |
| 5614 | 1 | 5614(src, mode) screen/palette copy into the work buffer | C274 |
| 59D4 | 3 | message_box(str) | B59F BB7E D60C |
| 5C20 | 1 | yes_no box: 1 = Yes, 0 = No | B12A |
| 5D20 | 3 | music_stop | BD8A C34F C422 |
| 5D29 | 5 | music_play(n) | B12A BF50 C422 |
| 5D73 | 1 | music_load(n) | BF50 |
| 5E1A | 2 | music_unload(n) | BD8A C34F |
| 67B6 | 6 | screen restore 6973(0,0,319,199) unless [9B26]==2 path (G) | B12A BB7E C105 D3F2 |
| 67E4 | 5 | draw into the work buffer | BF50 D3F2 D60C |
| 680F | 2 | target = the screen descriptor | BF50 D60C |
| 6837 | 5 | draw on the screen (6887 first if needed) | B12A BB7E C105 D3F2 |
| 6887 | 6 | flip: work buffer -> screen | B12A BB7E BEAE C105 D3F2 D60C |
| 691B | 1 | save rect screen -> work buffer | BEAE |
| 6973 | 2 | restore rect work buffer -> screen | BF50 D60C |
| 74B7 | 1 | free(buf) | C312 |
| 7E74 | 17 | fill rect (colour, ...) | B12A BB7E BEAE C105 C71B D3F2 D60C D89B |
| 7ECA | 1 | hline(x1, y, x0) clipped | BEAE |
| 817C | 4 | sprite clip against C0DA..C0E0 | CE7E D11B D3F2 |
| 81EA | 1 | rect min/max combine (rect union/intersect) (G) | D97C |
| 8238 | 3 | in_rect(rect, y, x): inclusive, signed | D070 D60C |
| 84BB | 2 | wait_vretrace (port 3DA bit 3) | C18C C1BB |
| 86A0 | 1 | blit (86A0) | BF50 |
| 8A9E | 1 | print number (left) | C71B |
| 8AC8 | 1 | print number (right-aligned) | C71B |
| 8AF2 | 1 | erase rect then print number | DC8E |
| 8B2C | 1 | erase rect then print (8B2C) | DC8E |
| 8BE4 | 3 | centred_text(str, cx, y) | BEAE C71B |
| 8E62 | 19 | sprite8e62(desc k of sheet, x, y) | B8E3 B96A BA7A BB7E BF50 C105 CCDE CD4B D3F2 D60C D89B |
| 9D04 | 4 | long mul | C4F8 C641 |
| 9DF2 | 3 | strcat | B59F BB7E |
| 9E32 | 2 | strcpy | B59F BB7E |
| A048 | 1 | unsigned long div | C558 |

rng_roll (076B) is called 23 times: B12A 1, B6A0 4, BB7E 2, BD41 2, C9A3 1, CB11 2, CD96 3, D3F2 1, D89B 1, DA50 6.
The port must keep this order (house rule).

## 6. Outcome of the battle (D, quoted from the listing; for Erik's report)

End of B12A after the duel (b419..b59c):
```
b422: cmp byte ptr [0xc38f], 0x0      ; Roman flagship gone?
b427: jne 0xb436
b429: mov di, 0x1                     ; di bit 0 = Roman flagship sunk
b42c: add word ptr [0xc08e], 0x1 ...
b436: cmp byte ptr [0xc38e], 0x0      ; enemy flagship gone?
b43b: jne 0xb44a
b43d: or di, 0x2                      ; di bit 1 = enemy flagship sunk
b44a: test di, 0x1 / jne b45f        ; Roman flagship survived: inc W(110E + 2*(C37B + [bp-14h]))
b45f: test di, 0x2 / jne b474        ; enemy flagship survived: inc W(110E + 2*(C37A + [bp-0Eh]))
...
b49e: cmp byte ptr [0xc38f], 0x0 / je b4ac
b4a5: cmp byte ptr [0xc38e], 0x0 / jne b4c6   ; both flagships afloat: no fleet battle
b4ac: call 0xda50 (enemy, roman) / or ax, ax / je b4ac   ; fleet battle rounds until DA50 != 0
b4c3: call 0xdc8e
b4c6..b4fe: di = sum of 3 ship words of fleet [bp+4] (enemy), dx = sum of fleet [bp+6] (Roman)
b503: mov si, word ptr [bp - 0xc]      ; default outcome
b506: dx != 0 && di != 0  -> si = 4 (retreat, "You retreat to safety ...")
b513: dx == 0 -> Roman fleet: 1116 = FF, legion aboard (1117) -> 4816 legion_destroyed; si = 1 (LOST)
b536: di == 0 -> enemy fleet: 1116 = FF, W(C11A + 14*army) = 0; si |= 2 (WON; 3 = both gone)
b585: push [bp-0xa] (flagship bits di) ; push [bp-8] ; push si ; call 0xb59f
```
B59F(outcome si [bp+4], mode [bp+6], flagship bits [bp+8]) picks the texts (D):

| flagship bits | flavour line (9356) |
|---|---|
| 0 | DS:3E02 = "" (empty) |
| 1 (Roman sunk) | 8F52 "The Sea and War Gods frown, and your flagship sinks beneath you." |
| 2 (enemy sunk) | 8EEE "Neptune and Mars smile, and you destroy the enemy flagship, sending your foes to a watery death." |
| 3 (both) | 8F96 "Such commanders lead these fleets, that Neptune calls them both to his underwater realm." |
| mode 8 (any bits) | 8FF2 "The enemy admiral, recognizing your superiority, flees for the safety of his home port." |

| outcome | bit 0 set (Roman flagship sunk) | bit 0 clear | mode 8 (admiral fled) |
|---|---|---|---|
| 1 lost | 913C "With the sinking of the flagship, your sailors lose heart and the Roman fleet is defeated." | 9220 "Alas, your ship duel was with the witless commander of the enemy fleet, and his more competent mates have swept the seas clean of Roman vessels." | 90B0 "In spite of their admiral's cowardly flight, the enemy fleet overwhelms your ships." |
| 2 won (also `inc [C0FE]`) | 9104 "In spite of their admiral's bath, your fleet prevails." | 9198 "The enemy force is disheartened when their leader is sent to Neptune's locker without a sword, and your ships make quick work of them." | 904C "Their leader's flight from battle disheartens the enemy, and your brave crews destroy them easily." |
| 3 both gone | 92B2 "The fleets fight until sunset, when the last two vessels sink, together forever in a deadly embrace." | same | same |
| 4 retreat | 9318 "You retreat to safety with your remaining ships." | same | same |

So Erik's message (9220, "witless commander") is the LOST text for the case where the Roman flagship survived the duel
(bit 0 clear), i.e. the enemy flagship was sunk (or neither, which cannot reach the fleet battle without mode 8) and the
fleet battle DA50 still wiped out the Roman fleet. The text is written for that case on purpose (D), so the inversion is
not in B59F. Note (G, to check in the fleet battle): when the enemy flagship sinks, its ship is not added back to the
enemy count (b45f) while the Roman one is (b44a), so sinking the commander should help; whether DA50/DD2E/DE51 (the
fleet battle) favour the enemy regardless is the open question. Also note: 9104 ("their admiral's bath") is the WON
text when the ROMAN flagship sank (bit 0): "their" reads as the enemy admiral, which looks like the 1990 wording expects
bit 0 = enemy flagship (G: a possible text/bit mix-up, check C38E/C38F against the duel code before calling it a bug).

The "<A> met the <B> on the gently rolling waves of <sea>." frame: B59F strcpy(AC60, [934A]) then strcat 934C..9358
(934C -> 8EAE " met the ", 9350 -> 8EB8 " on the gently\nrolling waves of ", 9354 -> 8ED9 ".\n\n"); 934A/934E =
W(1100 + 2*n) names, 9352 = W(5222 + 2*zone of the enemy fleet). Then 59D4 message box (D).

Re-engage: b3e8.. `push 0x935a` "The tides of battle sweep your flag ships apart. Do you wish to re-engage?" -> 5C20 yes/no (D).

## 7. DS addresses, absolute operands (D; r = read, w = written, rw = read-modify-write)

| DS | r | w | rw | functions | meaning |
|---|---|---|---|---|---|
| 0042 | 2 | 0 | 0 | C1BB | tick counter low (timer ISR) |
| 0044 | 2 | 0 | 0 | C1BB | tick counter high |
| 00A2 | 5 | 0 | 0 | C366 | joystick/arrow bits |
| 00B2 | 1 | 0 | 0 | C105 | mouse buttons |
| 0491 | 9 | 0 | 0 | B12A C9A3 CB11 CD96 DA50 | difficulty per module (save block) |
| 56FE | 3 | 0 | 0 | BE0F C846 C92B | long pair (5700) copied to C398/C39C |
| 5700 | 3 | 0 | 0 | BE0F C846 C92B |  |
| 6346 | 4 | 0 | 0 | BEAE C71B DC8E | box fill colour |
| 934A | 1 | 1 | 0 | B12A B59F | outcome text: name A (1100[...]) |
| 934E | 0 | 1 | 0 | B12A | outcome text: name B |
| 9352 | 0 | 1 | 0 | B12A | outcome text: sea name (5222[zone]) |
| 9356 | 0 | 5 | 0 | B59F | outcome text: flavour line (flagship sinkings) |
| 9358 | 0 | 6 | 0 | B59F | outcome text: result line |
| 96C4 | 1 | 0 | 0 | BB7E |  |
| 96C6 | 1 | 0 | 0 | BB7E |  |
| 96C8 | 1 | 0 | 0 | BB7E |  |
| 96CA | 1 | 0 | 0 | BB7E |  |
| 96CC | 1 | 0 | 0 | BB7E |  |
| 96CE | 0 | 1 | 0 | BB7E |  |
| 96D0 | 1 | 0 | 0 | BB7E |  |
| 96D2 | 1 | 2 | 0 | BB7E |  |
| 970A | 1 | 0 | 0 | C312 |  |
| 971A | 1 | 0 | 0 | BE0F |  |
| 971C | 1 | 0 | 0 | BE0F |  |
| 971E | 1 | 0 | 0 | BE0F |  |
| 9720 | 1 | 0 | 0 | BE0F |  |
| 9756 | 0 | 1 | 0 | C274 |  |
| 975E | 0 | 1 | 0 | C274 |  |
| 9812 | 1 | 1 | 0 | C1BB |  |
| 9814 | 1 | 1 | 0 | C1BB |  |
| 9816 | 2 | 1 | 1 | C1BB |  |
| 9817 | 2 | 1 | 1 | C1BB |  |
| 9950 | 4 | 0 | 0 | D60C |  |
| 9956 | 4 | 1 | 0 | D60C D89B |  |
| 9AC6 | 0 | 6 | 0 | B12A B59F C71B DC8E | set to C7 with C0E0/C1EA (clip bottom?) |
| 9AFA | 0 | 14 | 0 | B12A BB7E BEAE C105 C71B D3F2 D60C DC8E | draw colour |
| 9B26 | 5 | 0 | 0 | BF50 C105 C18C | video mode (2 = VGA) |
| 9B69 | 0 | 3 | 0 | BEAE C71B DC8E | text colour |
| AAC6 | 0 | 1 | 0 | D60C |  |
| AAC7 | 0 | 1 | 0 | D60C |  |
| AAC8 | 1 | 1 | 0 | D60C |  |
| AACA | 1 | 1 | 0 | D60C |  |
| AACC | 1 | 2 | 0 | D60C |  |
| AACE | 1 | 2 | 0 | D60C |  |
| C082 | 0 | 0 | 1 | DE51 | long casualty counter (DE51) |
| C084 | 0 | 0 | 1 | DE51 |  |
| C086 | 0 | 0 | 1 | DD2E | long casualty counter (DD2E) |
| C088 | 0 | 0 | 1 | DD2E |  |
| C08A | 0 | 0 | 2 | B12A DE51 | long counter: enemy flagships sunk? (+1 when C38E == 0) (G) |
| C08C | 0 | 0 | 2 | B12A DE51 |  |
| C08E | 0 | 0 | 2 | B12A DD2E | long counter: Roman flagships sunk? (+1 when C38F == 0) (G) |
| C090 | 0 | 0 | 2 | B12A DD2E |  |
| C0E0 | 0 | 6 | 0 | B12A B59F C71B DC8E | clip rect bottom (with C1EA, 9AC6) |
| C0FC | 0 | 0 | 1 | B12A |  |
| C0FE | 0 | 0 | 1 | B59F |  |
| C1DC | 1 | 0 | 0 | BF50 |  |
| C1DE | 1 | 0 | 0 | BF50 |  |
| C1EA | 0 | 6 | 0 | B12A B59F C71B DC8E |  |
| C36E | 2 | 1 | 2 | B12A B6A0 BB7E |  |
| C36F | 2 | 1 | 2 | B12A B6A0 BB7E |  |
| C370 | 1 | 2 | 0 | B12A B6A0 |  |
| C371 | 0 | 1 | 0 | B12A |  |
| C372 | 6 | 1 | 1 | B12A B6A0 BB7E BE0F CE7E D242 D60C |  |
| C374 | 6 | 1 | 1 | B12A B6A0 BB7E BE0F CE7E D242 D60C |  |
| C376 | 6 | 1 | 1 | B12A B6A0 BB7E BE0F CE7E D242 D60C |  |
| C378 | 6 | 1 | 1 | B12A B6A0 BB7E BE0F CE7E D242 D60C |  |
| C37A | 14 | 1 | 0 | B12A B6A0 BD8A BE0F BF50 CE7E D60C DA50 DE51 | enemy flagship slot (0..2) |
| C37B | 10 | 1 | 0 | B12A BD8A BE0F BF50 CE7E D60C | Roman flagship slot (C274), FF = cancelled |
| C37C | 1 | 4 | 1 | B12A BB7E BE0F C422 C71B |  |
| C37E | 5 | 3 | 0 | B12A B6A0 BB7E C71B |  |
| C380 | 5 | 3 | 1 | B12A B6A0 BB7E C71B |  |
| C382 | 10 | 1 | 0 | B12A B6A0 C641 C846 C92B |  |
| C384 | 12 | 2 | 0 | B96A BE0F CD96 CFDD |  |
| C386 | 2 | 4 | 0 | B12A BB7E BE0F |  |
| C388 | 2 | 1 | 0 | B6A0 BE0F CE7E |  |
| C389 | 1 | 1 | 0 | BE0F CE7E |  |
| C38A | 0 | 1 | 0 | BE0F |  |
| C38B | 0 | 1 | 0 | BE0F |  |
| C38C | 1 | 1 | 0 | B6A0 BE0F |  |
| C38D | 2 | 1 | 0 | BE0F C422 C71B |  |
| C38E | 8 | 4 | 0 | B12A BB7E BD8A C71B DA50 | enemy flagship strength (0 = sunk) (G from B436) |
| C38F | 8 | 3 | 0 | B12A B6A0 BB7E BD8A C71B DA50 | Roman flagship strength (0 = sunk) (G from B422) |
| C390 | 0 | 2 | 0 | BB7E BE0F |  |
| C391 | 0 | 2 | 0 | BB7E BE0F |  |
| C392 | 0 | 1 | 0 | BE0F |  |
| C393 | 1 | 1 | 0 | B6A0 BE0F |  |
| C394 | 0 | 2 | 0 | B6A0 BE0F |  |
| C395 | 0 | 3 | 0 | BE0F C366 |  |
| C396 | 1 | 3 | 0 | B6A0 BE0F |  |
| C397 | 0 | 3 | 0 | BE0F C366 |  |
| C398 | 0 | 1 | 1 | BE0F D242 |  |
| C39A | 0 | 1 | 1 | BE0F D242 |  |
| C39C | 0 | 1 | 1 | BE0F D242 |  |
| C39E | 0 | 1 | 1 | BE0F D242 |  |
| C3A0 | 0 | 1 | 0 | BE0F |  |
| C3A1 | 0 | 1 | 0 | BE0F |  |
| C3A2 | 4 | 1 | 0 | BF50 C312 | sheet/buffer handle (freed in C312) |
| C3A4 | 2 | 1 | 0 | BF50 C312 |  |
| C3A6 | 6 | 1 | 0 | B8E3 B96A BF50 C312 D97C |  |
| C3A8 | 6 | 1 | 0 | B8E3 B96A BF50 C312 D97C |  |
| C3AA | 0 | 1 | 0 | BE0F |  |
| C3AB | 0 | 2 | 0 | BE0F C366 |  |
| C3AC | 4 | 1 | 0 | BB7E BF50 C312 D3F2 |  |
| C3AE | 7 | 1 | 0 | BA7A BF50 C312 CCDE CD4B D11B |  |
| C3B0 | 4 | 0 | 0 | CE7E D070 D11B |  |
| C3B2 | 4 | 0 | 0 | CE7E D070 D11B |  |
| C3B4 | 4 | 0 | 0 | CE7E D070 D11B |  |
| C3B6 | 2 | 0 | 0 | CFDD |  |
| C3B8 | 2 | 0 | 0 | CFDD |  |
| C3BA | 0 | 1 | 0 | BE0F |  |
| C3BB | 0 | 1 | 0 | BE0F |  |
| C3BC | 2 | 1 | 0 | BF50 C105 |  |
| C3E0 | 0 | 0 | 1 | D242 |  |
| C3E2 | 0 | 0 | 1 | D242 |  |
| C3E4 | 0 | 0 | 1 | D242 |  |
| C3E6 | 0 | 0 | 1 | D242 |  |
| C3E8 | 0 | 0 | 1 | D242 |  |
| C3EA | 0 | 0 | 1 | D242 |  |
| C3EC | 0 | 0 | 1 | D242 |  |
| C3EE | 0 | 0 | 1 | D242 |  |
| C400 | 1 | 1 | 3 | DA50 DE51 |  |
| C402 | 1 | 1 | 3 | DA50 DD2E |  |
| C404 | 7 | 3 | 3 | DA50 DC8E DD2E |  |
| C406 | 5 | 4 | 2 | DA50 DC8E |  |
| C408 | 3 | 2 | 0 | DA50 DD2E |  |
| C40A | 4 | 2 | 0 | DA50 DE51 |  |

Most of the battle state is the block C36E..C40A (written by BE0F at the start of each duel), mirrored per side as
pairs (C38E/C38F, C390/C391, ... and the table bases C372/C374, C388.. below). C082..C090 are long counters
(add 1 / adc 0).

## 8. DS tables indexed by a register (reg + displacement) (D)

| DS base | r | w | rw | index regs | known |
|---|---|---|---|---|---|
| 1100 | 2 | 0 | 0 | bx | name pointers (fleet/owner names) |
| 110E | 3 | 0 | 4 | bx | fleet ship counts, 3 words per fleet at 110E+12f |
| 1114 | 2 | 0 | 0 | bx | fleet zone |
| 1115 | 2 | 0 | 0 | bx | fleet record +7 |
| 1116 | 1 | 2 | 0 | bx | fleet record +8 (FF = fleet gone) |
| 1117 | 9 | 0 | 0 | bx, si | legion aboard (FF = none) |
| 1118 | 2 | 0 | 0 | bx | fleet record +10 |
| 191E | 1 | 0 | 0 | bx | legion record (20 bytes) +0 |
| 1920 | 2 | 1 | 0 | bx | legion record +2 |
| 1923 | 1 | 0 | 0 | bx | legion record +5 |
| 5222 | 1 | 0 | 0 | bx | zone/sea name pointers |
| 527A | 2 | 0 | 0 | bx | zone table |
| 8EDE | 2 | 0 | 0 | bx | per-legion/fleet strength table (8EDE[3]) |
| 8EE2 | 2 | 0 | 0 | bx |  |
| 8EE6 | 1 | 0 | 0 | bx |  |
| 8EEA | 1 | 0 | 0 | bx |  |
| 93A6 | 1 | 0 | 0 | bx |  |
| 93B6 | 1 | 0 | 0 | bx |  |
| 93C6 | 1 | 0 | 0 | bx |  |
| 93D6 | 2 | 0 | 0 | bx |  |
| 93E6 | 5 | 0 | 0 | bx |  |
| 93F6 | 1 | 0 | 0 | bx |  |
| 93FE | 1 | 0 | 0 | bx |  |
| 95D2 | 3 | 0 | 0 | di, bx, si |  |
| 95D4 | 1 | 0 | 0 | bx |  |
| 95E4 | 1 | 0 | 0 | bx |  |
| 95F4 | 1 | 0 | 0 | bx |  |
| 9624 | 1 | 0 | 0 | bx |  |
| 9654 | 1 | 0 | 0 | bx |  |
| 9684 | 1 | 0 | 0 | bx |  |
| 96B4 | 1 | 0 | 0 | bx |  |
| 96BC | 1 | 0 | 0 | bx |  |
| 9704 | 3 | 0 | 0 | si, bx |  |
| 9714 | 1 | 0 | 0 | di |  |
| 9722 | 1 | 0 | 0 | bx |  |
| 9728 | 1 | 0 | 0 | bx |  |
| 9762 | 1 | 0 | 0 | bx |  |
| 976E | 1 | 0 | 0 | bx |  |
| 980C | 2 | 0 | 0 | bx |  |
| 9818 | 1 | 0 | 0 | bx, +, di |  |
| 9819 | 2 | 0 | 0 | bx |  |
| 9828 | 1 | 0 | 0 | bx |  |
| 982C | 1 | 0 | 0 | bx |  |
| 9870 | 1 | 0 | 0 | bx |  |
| 987A | 1 | 0 | 0 | bx |  |
| 989A | 1 | 0 | 0 | bx |  |
| 98BE | 2 | 2 | 2 | si, di |  |
| 98C0 | 1 | 0 | 0 | bx |  |
| 98C4 | 1 | 0 | 0 | bx |  |
| 98C8 | 1 | 0 | 0 | bx |  |
| 98CC | 1 | 0 | 0 | bx |  |
| 98D0 | 1 | 0 | 0 | bx |  |
| 98E0 | 2 | 1 | 1 | si |  |
| 98E2 | 4 | 0 | 0 | bx, si |  |
| 98F2 | 4 | 0 | 0 | bx |  |
| 98F6 | 1 | 0 | 0 | bx |  |
| 9906 | 1 | 0 | 0 | bx |  |
| 9916 | 1 | 0 | 0 | bx |  |
| 9926 | 1 | 0 | 0 | si |  |
| 9928 | 1 | 0 | 0 | bx |  |
| 992A | 1 | 0 | 0 | bx |  |
| 9932 | 0 | 0 | 0 | bx |  |
| 994A | 2 | 0 | 0 | bx |  |
| 9952 | 2 | 0 | 0 | bx |  |
| 9958 | 1 | 0 | 0 | si |  |
| 9968 | 1 | 0 | 0 | si |  |
| 99BC | 2 | 0 | 0 | bx, si |  |
| 99C2 | 2 | 0 | 0 | bx |  |
| 99C8 | 1 | 0 | 0 | si |  |
| 99CE | 1 | 0 | 0 | bx |  |
| 99D4 | 1 | 0 | 0 | bx |  |
| AAC6 | 3 | 2 | 1 | si |  |
| AAC8 | 2 | 0 | 1 | bx, di |  |
| AACC | 2 | 0 | 0 | bx, di |  |
| C116 | 1 | 0 | 0 | bx | army records C112 + 14*i (+4..+0xC) |
| C118 | 1 | 0 | 0 | bx | army record +6 |
| C11A | 0 | 1 | 0 | bx | army record +8 (zeroed at b554) |
| C11C | 1 | 1 | 0 | bx | army record +0xA |
| C11E | 1 | 0 | 0 | bx | army record +0xC |
| C370 | 2 | 0 | 0 | di, si |  |
| C372 | 18 | 0 | 2 | bx, si, di |  |
| C374 | 18 | 0 | 2 | bx, si, di |  |
| C37A | 10 | 0 | 0 | di, si, bx |  |
| C37E | 4 | 3 | 0 | bx, si, bx, +, si |  |
| C388 | 24 | 0 | 4 | si, bx, di |  |
| C38A | 6 | 1 | 0 | si, bx |  |
| C38C | 4 | 2 | 2 | si, bx |  |
| C38E | 0 | 0 | 4 | bx, si |  |
| C390 | 6 | 6 | 2 | si, bx |  |
| C392 | 6 | 3 | 0 | si, di |  |
| C394 | 2 | 2 | 0 | si |  |
| C396 | 2 | 1 | 0 | si |  |
| C398 | 1 | 1 | 1 | bx |  |
| C39A | 1 | 1 | 1 | bx |  |
| C3A0 | 4 | 1 | 1 | bx, si |  |
| C3A2 | 5 | 0 | 0 | bx, bx, +, si, si |  |
| C3AA | 1 | 1 | 1 | di |  |
| C3BA | 4 | 3 | 0 | si, di, bx |  |
| C3BE | 0 | 1 | 0 | bx |  |
| C3C4 | 0 | 1 | 0 | si |  |
| C3C8 | 1 | 0 | 1 | bx, +, di |  |
| C3CC | 3 | 1 | 0 | si |  |
| C3CE | 4 | 1 | 0 | si, di |  |
| C3D0 | 4 | 1 | 1 | bx |  |
| C3D2 | 4 | 1 | 1 | bx |  |
| C3D8 | 5 | 1 | 1 | si, bx, di |  |
| C3DA | 5 | 1 | 1 | si, bx, di |  |
| C3E0 | 1 | 2 | 1 | bx |  |
| C3E2 | 1 | 2 | 1 | bx |  |
| C3E8 | 1 | 2 | 1 | si, bx |  |
| C3EA | 1 | 2 | 1 | si, bx |  |
| C3F0 | 1 | 1 | 0 | bx |  |
| C3F2 | 1 | 1 | 0 | bx |  |
| C3F4 | 0 | 1 | 0 | bx |  |
| C3F6 | 0 | 1 | 0 | bx |  |

## 9. Embedded tables and immediates

No tables or strings are embedded in the section itself (V: section 1). Immediates that point into DS (D; strings
read from the static DS, OVL0.OVL 0xB400 = DS:0000):

| DS | used at | content |
|---|---|---|
| 3E02 | B5BE | "" (empty string, flavour line when no flagship sank) |
| 49F0 | C649 | constant/limit (C641) (G) |
| 8EEE | B5CE | "Neptune and Mars smile, and you destroy the enemy\nflagship, sending your foes to a watery death.\n\n" |
| 8F52 | B5C6 | "The Sea and War Gods frown, and your\nflagship sinks beneath you.\n\n" |
| 8F96 | B5D6 | "Such commanders lead these fleets, that Neptune calls\nthem both to his underwater realm.\n\n" |
| 8FF2 | B5E2 | "The enemy admiral, recognizing your superiority,\nflees for the safety of his home port.\n\n" |
| 904C | B633 | "Their leader's flight from battle disheartens\nthe enemy, and your brave crews destroy them easily." |
| 90B0 | B610 | "In spite of their admiral's cowardly flight,\nthe enemy fleet overwhelms your ships." |
| 9104 | B622 | "In spite of their admiral's bath,\nyour fleet prevails." |
| 913C | B5FF | "With the sinking of the flagship, your sailors lose\nheart and the Roman fleet is defeated." |
| 9198 | B627 | "The enemy force is disheartened when their leader\nis sent to Neptune's locker without a sword, and\nyour ships make quick work of them." |
| 9220 | B604 | "Alas, your ship duel was with the witless commander\nof the enemy fleet, and his more competent mates\nhave swept the seas clean of Roman vessels." |
| 92B2 | B63B | "The fleets fight until sunset, when the last two\nvessels sink, together forever in a deadly embrace." |
| 9318 | B643 | "You retreat to safety with your remaining ships." |
| 934C | B671 | pointer list 934C..9358 of the outcome text |
| 935A | B3EA B68A | "The tides of battle sweep\nyour flag ships apart.\nDo you wish to re-engage?" |
| 970A | C0A3 | buffer pointer freed in C312 |
| 9714 | C0BC | table (BF50) |
| 972C | C285 | table (C274) |
| 9750 | C2E6 C2F1 | table (C274) |
| 9762 | C19B | palette table (C18C) |
| 9778 | C198 C19E C203 | palette RGB triples (C18C, C1BB) |
| 979C | C22C C234 | palette RGB triples (C1BB) |
| 97BA | BF0C C25D | "SHIPS" |
| 97CC | BF70 | "galleys.lzw" |
| 97D8 | BF83 | "oars1" |
| 97DE | BF94 | "oars2" |
| 97E4 | BFC8 | "galleys.cbm" |
| 97F0 | BFFE | "bombs" |
| 97F6 | C00F | "smok" |
| 97FB | C08C | "collide.lzw" |
| 9807 | C0C9 | "flag" |
| 98BA | C7E7 | "MEN" |
| 9978 | D698 | "Failed to Load Human Sinking" |
| 9995 | D70A | "Failed to Load Computer Sinking" |
| 99B5 | D735 | "shark" |
| 9B56 | B3DB B655 BCDF BD2C BEFD | 10-byte block, filled from A05C (rep movsw, 5 words) (G: text box rect/style) |
| A05C | B3E0 B65A BCE4 | 10-byte default text box style (G) |
| A3B4 | BD2F BF02 | text box parameter (G) |
| AC60 | B668 B67D B691 BD02 BD0F BD1C BD25 | message text buffer (strcpy/strcat) |
| C118 | DEC5 | army record field (DE51) |
| C356 | BFD0 BFDA BFE8 | sheet descriptor (BF50) |
| C372 | B269 | state block address (B12A) |
| C376 | B265 | state block address (B12A) |
| C3BE | DD5D | state (DD2E) |
| C3C4 | DDB2 | state (DD2E) |
| C3F0 | CE89 | state (CE7E) |
| C3F8 | CE85 | state (CE7E) |

Easter eggs / jokes in the texts (report to Erik): "witless commander ... his more competent mates"; "In spite of
their admiral's bath"; "sent to Neptune's locker without a sword"; "together forever in a deadly embrace"; the shark
sprite (MNAVAL SHARK) used by D60C/D89B after a sinking. C18C (palette set from 9762/9778) is never called (D).

Archive names used (BF50, D60C): galleys.lzw, oars1, oars2, galleys.cbm, bombs, smok, collide.lzw, flag, shark (and
the sinking sheets loaded in D60C: "Failed to Load Human Sinking" / "Failed to Load Computer Sinking").
