# Plan: translating the 115 functions under 0x2EFA0 (nation choice and the map screen kit)

Basis: the four reader reports (all 115 graded D), docs/OVERVIEW.md §2-5, ui.h, game.h, the port sources
(none of the 115 is ported yet; the UI kit 0x39F90..0x3CFE4 already in the port is used as is).

## Summary

What the player sees after START GAME is 0x2EFA0, the nation choice on the zoom map. It is reached directly from main
(0x1184F) and returns to 0x11854. That makes 0x11854 the first breakpoint for whole-data-object comparisons.
- With the default setup (3 humans, 0 computers) the choice uses no random numbers and never calls 0x302C4. It can be
  verified first, on its own.
- The rest of the slice hangs off the button dispatcher 0x3FB94: 11 KB, a 261-entry jump table at 0x3F780. Every
  modal loop reaches it through 0x78BA4. It is the only way to the in-game screens.
- The plan translates 0x3FB94 as a skeleton early: the table, the constant-return cases, and the cases whose callees
  exist. Each later module fills in its own cases.
- Unported cases abort with their address, never silently return -100.

Map-screen work can be frame-verified in full only once the next slice is ported. That slice is main's code after
0x11854: 0x6B9AC, then 0x454D0 ("%s place your token in a starting territory", which calls 0x3D434, 0x3D52C,
0x3D9FC, 0x459E4 and 0x783B8).
- Until then, a port test hook restores a save (module S1) and calls f3FB94(id) with scripted clicks.
- Full-screen views (AST, census, Civilization Tools, Goods Held, scoring, Trading Wrapup) are then compared at
  0 px against DOSBox-X after "restore R4, click button X".
- Boxes drawn over the map (options, help, toggles, audio) are compared inside their rectangle only.

## Modules

| # | Module | Functions | Bytes | Depends on | Mistral |
|---|---|---|---|---|---|
| N1 | Nation choice, human players | 0x2EFA0, 0x2F9F8, 0x2FC04, 0x300E0, 0x3AB7C, 0x3A9F4, 0x3AF54, 0x79A9C, 0x78A20, 0x3C940, 0x3C7A0 | 5,832 | ported UI kit, newgame.c | 0x2FC04 (player count -> names/rects table), v-index rule |
| N2 | AST view, button kit, click wait, dispatcher skeleton, score helpers | 0x136B0, 0x78BA4, 0x3FB94 (skeleton + ids 3, 0x2A, constant stubs), 0x3BB48, 0x3BD80, 0x3BE10, 0x3C0D8, 0x3C284, 0x3B7CC, 0x3B870, 0x3B9A4, 0x3C828, 0x3C8C4, 0x3A148, 0x3A3A0, 0x147C4, 0x1486C, 0x17760, 0x6DD98, 0x7BA64 | ~8,200 (of which ~3,500 of 0x3FB94) | N1 | 0x6DD98, 0x17760, 0x1486C, 0x147C4; the id->code table of 0x3FB94's stubs once Claude extracts the pairs |
| N3 | Computer nations | 0x302C4, 0x30174 | 3,044 | N1, N2 | none |
| S1 | Save and restore | 0x5FF28, 0x5F05C, 0x601D8, 0x60694, 0x6016C, 0x7BBF0, 0x7BB84, 0x7BCD4, 0x726DC, 0x5E7D0, 0x3CD10, 0x3CC68, 0x3CCBC, 0x78B0C, 0x78B24 | 8,352 | N2 | 0x7BB84 + 0x7BBF0 (CRC), 0x7BCD4, the 61->69-byte area conversion of 0x5F05C |
| MAP1 | Map window | 0x3D52C, 0x3D18C, 0x3DAD8, 0x3DE40, 0x3E98C, 0x3E9DC, 0x3EA2C, 0x449B8, 0x449E0, 0x3D9FC, 0x3DA60, 0x787F8, 0x7876C, 0x3F624, 0x3F670, 0x3F698, 0x3F70C, 0x3E89C, 0x3E8FC, 0x3D2A4, 0x3CBBC, 0x3F558 | 5,672 | S1 (to load a state) | 0x449B8/0x449E0, 0x3E89C/0x3E8FC, setters |
| START | (next slice, readers first) first map screen | 0x6B9AC, 0x454D0, 0x3D434, 0x45F88, 0x3A924, 0x15FE0 (+0x14C6C with computers) | ~2,350 (+4,980) | MAP1, PANEL's 0x459E4/0x783B8 | none |
| PANEL | Status panel, overview, ship panel | 0x459E4, 0x3E374, 0x5D14C, 0x5DDF4, 0x426D0, 0x572CC, 0x783B8, 0x7853C, 0x78664, 0x3A990, 0x3AFF4 + cases 9..0x10, 0x1B..0x25 | 6,868 + ~1,500 | MAP1 | 0x7853C/0x78664 (record list append/remove) |
| MOVE | Movement primitives | 0x57CE8, 0x57E28, 0x580CC, 0x581C4, 0x576F8, 0x576A8, 0x57A10, 0x57B5C + cases 0x26..0x32, 0xC4..0xC8 | 1,448 + ~3,000 | PANEL | all eight primitives (pure record edits) |
| INFO | Census, Civilization Tools, Goods Held, Trading Wrapup | 0x13E50, 0x19C10, 0x1A124, 0x76A9C, 0x76798, 0x71F2C, 0x71FB0, 0x76F14, 0x194E4, 0x195D8, 0x763FC, 0x764A8 + cases 4, 6, 7, 0x103 | 6,932 | N2, S1 | 0x195D8 + 0x194E4, 0x76F14 |
| MENU1 | Options menu, help, toggles | 0x42754, 0x4290C, 0x42CEC, 0x436FC + cases 5, 0x52, 0x53, 0x57, 0x41, 0x6D, 0x6E, 0xE3..0xE9; 0x545AC (PBEM, not ported) | 5,940 | N2, S1 | 0x42CEC (phase/calamity -> help file + title table) |
| MENU2 | Audio/visuals, scoring, game view | 0x43E88, 0x44D5C, 0x44A34, 0x7B504 + cases 8, 0x56, 0xE2 | 5,784 | MENU1, PANEL | none |
| STACK | World State Stack, in-game new/restore/save, replay | 0x121A0, 0x7A8D0, 0x7A940, 0x7ABD0, 0x7AC60 + cases 0x4F, 0x50, 0x51, 0x101 | 1,172 + ~1,500 | S1, MENU1, MAP1 | 0x7A8D0..0x7AC60 (push/pop of three fixed-size slabs) |

All 115 functions are assigned once: N1 11, N2 20, N3 2, S1 15, MAP1 22, PANEL 11, MOVE 8, INFO 12, MENU1 5, MENU2 4, STACK 5.

## Verification per module

Port side: `advciv.exe GAMEDIR --headless-frame OUT.png --click X Y ...` (exists), `--seed`, `--dump-ds`. New hooks
proposed (Claude, with S1):
- `--restore SLOT` runs the title menu's restore path (0x5FF28 + main 0x115B0..);
- `--press ID` calls f3FB94(ID) on that state;
- `--dump-ds-at ADDR` dumps at a translated function's return.

DOSBox-X side: tools/dosbox/drive.ps1 (clicks land at about X+1, Y+1). Breakpoints use `BP 0180:A+202000`, set
in protected mode after int 21h AH=2Ch. Data dumps go through tools/verify/dscompare.py with by_design.txt.

- **N1.** Setup defaults (3 humans), START A NEW GAME -> START GAME.
  - F1: "Player One choose your nation", candidate 0's ch_ card, pointer parked.
  - F2: after the right arrow (376,389).
  - F3: after the left arrow (251,389).
  - F4: after SELECT (251,409): "Player Two ...", Player One's token drawn.
  - Repeat F1 with 4, 5, 6 and 8 humans (setup '+' 0x60), to cover every branch of 0x2FC04 and the zoom4/5/6/78
    maps.
  - Data: BP 0x11854 after three SELECTs (with arrow moves in between). Compare the whole data object: the
    strcpy'd picture names at 0xB39CC.., the rectangles 0xC14EC..0xC1532, 0xF7B00, 0xF7C78, 0xF7C9C/0xF7CA0,
    0xF40E4+32p, the +0x9A bytes, the message area 0xBB3FA..0xBB400 and 0xEE168. Seed unchanged.
- **N2.**
  - F5: in the nation choice, AST (251,429) -> the AST chart in mode 1 (astblan2.gif, the choosable names only).
  - F6: CONTINUE -> the choose screen again (= F1).
  - F7: AST after one SELECT (one name fewer).
  - Data: BP 0x11854 after a run that visited AST. Shows CONTINUE moved in 0xBB800 by 0x3BB48, and the choice
    list.
  - Score helpers: unit tests on the R4 autosave's nation records. Values are taken from DOSBox-X at the
    returns of 0x147C4 and 0x1486C (eax). Mode 3 of 0x136B0 waits for the AST phase (M6).
- **N3.** Setups 2H+1C and 1H+2C (3 players), 2H+2C, 2H+3C, 2H+4C, 3H+4C, 2H+6C. These hit all five jump tables.
  - Run with the same clock seed: --seed from 0xF422C in the DOSBox-X dump.
  - Data: BP 0x11854. Check the +0x9A bytes (2 = computer), the player records, and that the seed is identical.
  - Frame at the BP: the computer tokens on the zoom map.
  - First, in DOSBox-X, settle the suspected rem++ after Africa (2 humans + 1 computer in a 3-player game:
    one computer nation or two?).
- **S1.**
  - Title -> RESTORE A SAVED GAME: frame of the slot dialog (12 slots, descriptions of the run copy's saves).
  - Click slot 'a': frame of "Restoring" (chariot.gif + description).
  - Data: BP 0x115A0 (return of 0x5FF28 in main) after restoring the R4 autosave. The whole data object. The heap
    lists 0xF0984[1..11] are by design.
  - A phase-14 save covers 0x726DC, which uses game_rand.
  - CRC: f7BBF0 returns 0 for every save in the originals.
  - Save (0x5E7D0) is reached only from the in-game menu. Test it by producing savgame1.dat from a given
    savgamea.dat and description, and comparing byte for byte with a file the original wrote (STACK).
  - F8: the description field after typing.
- **MAP1.**
  - Now: restore R4 in both. The port hook runs f3D9FC(nation) as the game does, with the scroll from the DOSBox-X
    dump. Compare only the map window (8..0x277, 6..0x144) and the mini-map rectangle
    (0x1E0..0x27B, 0x16F..0x1A3) with the DOSBox-X frame after the restore.
  - Detail levels 3/2/1/0 by poking 0xC1A34 (DOSBox-X: button 0x24).
  - Hit test 0x3D2A4: table test over all 640x325 points vs a DOSBox-X run of the routine (BP + register
    read on a few hundred sampled points) or vs the area records.
  - Full: the START screen at 0 px.
- **START** (needs readers first).
  - Frame after the nation choice: "Player One place your token in a starting territory".
  - Data at the next main breakpoint after 0x454D0. 0x6B9AC uses game_rand.
- **PANEL.**
  - Restore R4, then click the player buttons 9..0x10: status panel + map centred. Compare the panel rectangle
    (0x3C..0x9A, 0x162..0x1C5) and the map window.
  - ZOOM (0x25): full-screen overview frame (0x3E374).
  - Ship panel 0x5D14C: restore the autosave before MOVEMENT, click a coastal territory, compare the panel
    (278..357, 370..411).
  - Full: on the START screen.
- **MOVE.**
  - Restore the autosave before MOVEMENT. Script one land move and one ship move (select, count 0xC4.., move
    0x28).
  - Data: BP at the end of case 0x28 in 0x3FB94: the area records 0xE31B8, the nation +0x40..+0x61 bytes.
  - Mistral's versions are checked first against unit vectors Claude takes from those dumps.
  - Full: M6 movement phase, next autosave byte-exact.
- **INFO.** Restore R4 and press, in both:
  - 4: census, full frame.
  - 6: Civilization Tools, full frame; then click an advance for the 0x1A124 pop-up, box rectangle.
  - 7: Goods Held, full frame. With more than one human, the "Select Nation To Show" pop-up comes first, box
    rectangle.
  - 0x103: Trading Wrapup, full frame. Needs a save with trades: the autosave after TRADE.
  - Data: Goods Held calls 0x194E4, which writes 0xF68B0..0xF68C0. Compare at the return of 0x76A9C.
- **MENU1.** Restore R4, press 5: options box. Then:
  - HELP (0x53): help box of the save's phase, rectangle compare. Plus a table test of 0x42CEC over phases
    0..21 and calamities 0..12, against the strings.
  - TOGGLES (0x52): toggles box. Click a row, CONTINUE: aciv.cfg bytes equal.
- **MENU2.**
  - SCORING (0x56): full frame. Data: 0xEE9F0 written by opening it.
  - AUDIO/VISUALS (0xE2): box frame plus aciv.cfg. Sound by ear (Erik).
  - Button 8: frame of the nation's full-map view.
- **STACK.**
  - Replay 0x101 on a save with [0xEDF58] != 0: frame "Map shows Round %d post movement state." Data after
    CONTINUE equals the data before (push/pop round trip).
  - In-game START NEW GAME (0x51): back at the setup screen (frame). The data object at the setup screen equals
    a fresh start, apart from the known leftovers (0x10740C).
  - In-game SAVE: file byte-compare as in S1.

## Order

N1 -> N2 -> N3 -> S1 -> MAP1 -> START (next slice) -> PANEL -> MOVE -> INFO -> MENU1 -> MENU2 -> STACK.
- N1..N3 close the nation choice. 0x11854 matches for every setup, so main can go on to the next slice.
- S1 next: it gives every later module a known state (the R4 autosave), and the title menu's RESTORE button is
  already ported.
- INFO, MENU1 and MENU2 are independent of each other and can run in parallel once S1 and N2 are in.

## Extra roots (reached only indirectly)

- **0x3FB94's jump table at 0x3F780.** In the code object, 261 dwords for ids 3..0x107. Default 0x426C1 returns
  -100; ids above 0x107 or below 3 fall there through a 16-bit unsigned compare. Called from 0x78BA4, 0x60694 and
  0x3D434.
- **Functions reached only through that table:**
  - 0x136B0(2) (id 3), 0x13E50 (4), 0x42754 (5), 0x19C10 (6), 0x76A9C (7), 0x44A34 (8);
  - 0x459E4 + 0x3D9FC / 0x3E374 (9..0x10), 0x3E374 (0x25);
  - 0x763FC (0x103), 0x545AC (0x107), 0x5E7D0 (0x4F), 0x5FF28 (0x50; also direct from main);
  - 0x121A0 + 0x7ABD0 + 0x3F558 (0x51), 0x436FC (0x52), 0x4290C (0x53), 0x44D5C (0x56), 0x43E88 (0xE2);
  - 0x44CAC (0x57), 0x7A940 / 0x7AC60 (0x101);
  - 0x426D0, 0x572CC, 0x576A8, 0x576F8, 0x57CE8, 0x57E28, 0x580CC (0x26..0x32, 0xC4..0xC8).
- **0x302C4's switch tables:** 0x30224, 0x30244, 0x30264, 0x30284 and 0x302A4, 8 entries each. They are internal
  case targets, indexed by a 3-bit mask of which nations of a triple are human.
- **0x42CEC's tables:** 0x42C94 (22 entries, by phase byte 0xF4102) and 0x42C60 (13 entries, by calamity 0xF783C).
- **0x300E0** is a separate small id mapper for the nation-choice dialog 0xC08D0, not the table.
- **Not in the 115, needed to see the map screen:** 0x6B9AC, 0x454D0, 0x3D434, 0x45F88, 0x3A924 and 0x15FE0, plus
  0x14C6C (AI start placement with .bpm) when computers play.

## Risks

1. **0x3FB94 (11 KB, 261 cases, about 50 callees).**
   - Translate it as a switch in its own file, with the address of every case.
   - Extract the id->target->constant table by script from the dd lines and the stubs (`mov eax,imm; jmp epilogue`),
     and check it twice.
   - Unported cases abort with "0x3FB94 case id -> 0xXXXXX not ported".
   - Keep a coverage list in the file header so later modules fill in exactly their cases.
2. **Randomness.**
   - game_rand is used in 0x302C4 (rand*2>>15, rand*3>>15), 0x726DC (restore in phase 14), 0x7BCD4 (old saves)
     and, next slice, 0x6B9AC and 0x14C6C.
   - Every data compare needs the same seed (--seed from the clock bytes 0xF422C) and the same click sequence.
   - Not every rand branch of 0x302C4 was traced: re-read all five tables in N3.
3. **Timing.**
   - 0x78A20 busy-waits on clock() divided by [0xBB3F4], with an unsigned compare: 750 ms in 0x2EFA0 and 0x30174,
     250/2000 ms in 0x136B0, 250/1000 ms in 0x763FC.
   - Headless runs need a virtual clock that advances in the wait.
   - DOSBox-X captures must be taken at a breakpoint or after the wait, never on a timer.
   - 0x7BA64 (/SRD, /C) reads keys and mouse; keep it false in tests.
4. **Input.**
   - 0x2EFA0 hit-tests dialog 0xC08D0 by hand (fg_mousepos, enabled byte, x <= mx < x+w) instead of calling
     0x78BA4.
   - The harness's release/press poll pattern must satisfy both loops.
   - 0x60694 polls fg_intkey and needs scripted keys (a `--keys` hook).
5. **Writes into string literals and tables.**
   - 0x2FC04 strcpy's picture names into the data object's literals (0xB39CC..) and rewrites rectangles. The port
     must do this in g_ds, not in C strings. The names change with the player count across games in one session.
   - Same with dialog working copies: 0x601D8 -> 0x10A174, CONTINUE moved in 0xBB800, the label pointers set by
     0x3C284.
6. **Heap pointers in the data object:**
   - 0xF7C00 / 0xF7C08[9] / 0xF7C40[9] (card GIFs);
   - 0xF0984[1..11] (save lists);
   - 0xF5240 / 0xF5268 / 0xF5290 (state stack);
   - 0xF6260+4s (samples);
   - 0xFFB54.., 0x10740C.
   Add them to by_design.txt as port handles. 0x3F558 leaves 0x10740C dangling (bug).
7. **The 'junk' / 'junk2' GIF files.**
   - 0x19C10, 0x1A124, 0x4290C, 0x44A34, 0x545AC, 0x76798 and cases 0x101/0x103 save the screen with fg_makegif to
     disk and read it back. fg_makegif is not in fg.c.
   - Plan: keep the pixels in memory with the same restore target and position. That is lossless, so it is
     identical on screen.
   - Erik decides whether the port should also leave the files on disk (behaviour) or not (default: not).
8. **Big functions with version branches.**
   - 0x5F05C: 3,788 bytes, about 90 freads, conversions for versions 0x25..0x38. Only versions >= 0x35 are
     accepted outside PBEM, so the older branches are translated but cannot be verified with real files.
   - 0x5FF28 loads before it checks, so a rejected save leaves the state overwritten.
   - 0x5E7D0 runs system("copy savgamea.dat savgameN.dat >NUL"); the port uses a C file copy with the same
     failure message.
9. **Map screen verification depends on the next slice** (START). Until then MAP1 and PANEL are checked by
   rectangles through test hooks. That is weaker than a full frame: the scroll and the state must come from the
   same DOSBox-X moment.
10. **Modes and data not yet reachable.**
    - 0x136B0 mode 3 (the AST phase rule and turn counter) and 0x5D14C outside phase 5 wait for M6.
    - 0x13E50 and 0x19C10 have PBEM lines; PBEM stays out (Erik's ruling). 0x545AC is not ported.
11. **1995 bugs and oddities to rule on** (fixed by default, --original keeps them):
    - rem++ after Africa in 0x302C4 (check first);
    - 0x3F558 frees one pointer and zeroes another;
    - 0x576A8's dead compare;
    - 0x581C4 drops a nation with ships left;
    - opening Scoring writes the score history;
    - 0x545AC does not restore 0xEE168;
    - Esc in the save description still saves (0x601D8 ignores 0x60694's result);
    - stale random table for save versions 0x2D/0x2E;
    - Trading Wrapup cannot scroll (6 trades shown);
    - 0x3E374's own-ships loop stops at nation 8;
    - 0x1486C counts advance 19 twice.
    Behaviour that changes game state (Scoring, the Africa count) must stay identical in the byte-compare runs:
    run those with --original or decide the ruling first.
12. **Duplicate helpers.** Some of these (0x3C940/0x3C828, 0x3A9F4/0x3AB7C, the setters) are near-twins of
    ported ones. Before adding, grep port/src for an existing f-name. Translate each twin separately; do not merge
    them.
