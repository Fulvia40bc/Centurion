# Plan: M6, the game loop and its phases (383 functions after main 0x1188C)

Basis: the eight reader reports (docs/INVENTORY_M6.json, 383 records, 382 graded D, 1 G), docs/OVERVIEW.md §2-6,
docs/MAP_PLAN.md, docs/LIBRARY_CALLS.md, port/src/*.h (what is ported), and a call map rebuilt from
disasm/AC_MAIN.asm for this plan.

Grades as elsewhere: **D** read in the code, **G** inference. Plans, proposals and estimates are G by nature; a fact
about the binary carries its grade.

## Summary

- Main reaches the game loop at 0x118C9 (D). Before it, a restored game passes 0x14AA4 (D: main 0x1189C), a new
  PBEM game 0x14A00 (D: main 0x118B1, PBEM only).
- The game loop 0x54824 is a 21-case switch on the phase byte 0xF4102 (D: jump table 0x547D0). Each case runs the
  phase, sets the next phase, calls the empty stub 0x5E7B4, autosaves with 0x5E8EC and checks eliminations with
  0x22B00 (D).
- The 383 functions hold 309,009 bytes (D: the records' sizes; a few sizes include jump-table data).
- The call map confirms the readers' caller lists: every non-library callee of the 383 is either in the 383 or
  already ported (D). Only library code remains outside (fg_, AIL_, C runtime).
- Four qsort comparators (0x13DE8, 0x3530C, 0x35368, 0x65C4C, 388 bytes) are reached only by address (D:
  functions.txt "p"). They are not in the 383 and must be translated with their callers.
- Watcom's qsort 0x8C331 (1,085 bytes, D) must be translated literally: it is not stable, and the AI's choices depend
  on its order for equal keys (D: 0x348B0, 0x36918, 0x4D29C, 0x65010 call it).
- About 45% of the bytes are computer-player code (G: the AI modules plus the functions marked AI in the table
  below, about 138 KB, PBEM left out). The human side of every phase is separable: in an all-human game no AI
  routine runs except the score matrix 0x7C960, which the loop calls every turn (G: per the records, every other
  AI call site tests +0x9A == 2 or sits behind such a test).
- So M6 (human side) and M7 (AI) split cleanly along the modules below. The first end-to-end target is round 1 of
  a new 3-human game (the default setup), which reaches every one of the 21 phase cases.

## How to get a save at the start of any phase

- The original autosaves after every phase into SAVGAMEA.DAT: "AutoSaved after %s - R%d" (D: 0x5E8EC, names
  0xCA9F4). The loop writes the next phase number before the autosave (D), so **the autosave after phase P is a save
  at the start of phase P+1**. Below, "S(P)" means that save.
- Exceptions (D, 0x54824): phases 6 and 15 skip the autosave on some paths; the winning path of phase 20 skips it.
  At the end of a round 0x55258 also writes SAVGAMEB.DAT "AutoSaved at end of Round %d" (D).
- Saves already in hand (OVERVIEW §5, V): S(SHIP MAINTENANCE) R4 (the "R4 autosave"), S(SHIP CONSTRUCTION) R4
  (slot 0, made by clicking FINISHED), and "end of round 3" (= start of TAXATION R4).
- `run_map_states.sh` and `run_steps.sh` keep the saves the original wrote when `KEEP_SAVES_TO=DIR` is set (D:
  the scripts). They keep only the last autosave of the run.
- **Proposal (tool, Claude):** `tools/dosbox/run_phases.sh NAME SLOT "steps"`. It adds a breakpoint on 0x5E8EC
  (run time 0x2608EC). At each stop: copy SAVGAMEA.DAT to NAME_S<phase-1>.dat (the file then holds the previous
  autosave, the start of the phase just finished) and dump the data object to NAME_after<al>.bin (the state the
  autosave is about to write). One run through a round then yields a save at every phase start and a dump at every
  phase end.
  - G: the DOS file is complete on the host once the previous autosave's fclose has run; check once by CRC.
  - The scripts must also back up and restore AC_MAIN's aciv.cfg: 0x7A0A0 rewrites it (D), and its toggles change
    the random stream (risk 2).
- **All-human saves (G, to check first).** Copy a round-4 save and turn every computer nation into a human: byte
  +0x9A = 3, the counts 0xE1620 (humans) / 0xE1621 (computers) adjusted, CRC recomputed (the port's f7BBF0 computes
  it, D: module S1). Restore it once in DOSBox-X to confirm the game treats them as humans. These "H-saves" let the human
  side of every phase be verified before any AI is ported.

## How a phase is compared

- **Original:** restore S(P-1) from the title menu, play the phase's clicks with run_steps.sh (click waits: f78BA4's
  0x78BD3, the movement loop 0x55FFD, and the own loops listed under risk 4), stop at 0x5E8EC.
- **Port (proposed hooks, Claude, with LOOP):**
  - `--restore SLOT --play`: main's restored-game path 0x1189C -> 0x14AA4 -> 0x54824, as the original;
  - `--stop-at-autosave P`: stop when 0x5E8EC is called with al == P, after an optional `--dump-ds`;
  - `--keep-autosave FILE`: let the autosave be written, then copy it;
  - `--aciv-cfg FILE`: pin the toggles (animations, AI pauses, confirmations) to the original's file.
  - Existing: `--click X Y`, `--wait N`, `--settle N`, `--seed`, `--map-state`, `--press`, `--headless-frame`.
- **Compare:**
  1. the two autosave files byte for byte (44,062 bytes with the CRC; D: the S1 check);
  2. the data objects at the 0x5E8EC stop with `tools/verify/dscompare.py` and by_design.txt;
  3. frames at chosen click waits: run_steps.sh's capture/steps/NAME_k.png against the port's `--headless-frame`
     after the same clicks, with `cmpframe.py`, or `rectdiff.py` for boxes over the map;
  4. unit-level: record ranges with `rangecmp.py` (area 0xE31B8 161x69, territory state 0xE2C18 160x3, nations
     0xF14CC 14x199, move lists 0x107694 / 0x1079B8, dialogs `dlg:ADDR`).
- **Proposal (tool, small):** `tools/verify/dlgxy.py DUMP DLG ID` prints a button's centre from a dump's dialog
  record (x, y, w, h at +0..+6, id at +0x0E; D: ui.h). Click scripts then name buttons by id, not by pixel.

## Modules

Bytes are the records' sizes (D). "AI" marks code that runs only for computer nations (D from the records' tests);
it is translated in M7 but listed with its phase so each phase has one home.

| # | Module | Functions | Bytes | Depends on | Mistral |
|---|---|---|---|---|---|
| RULES | Shared rule helpers | 0x78D08, 0x78E74, 0x78F04, 0x78FEC, 0x78D98, 0x789EC, 0x7C760, 0x7C7C4, 0x771F8, 0x76E50, 0x6E460, 0x75CA8, 0x75D84, 0x57CA4, 0x460C0, 0x46158 | 2,516 | MOVE (ported primitives) | all 16 |
| KIT | Message boxes, button-by-id helpers, map-box clears | 0x79270, 0x795A8, 0x7983C, 0x7844C, 0x3B728, 0x3B914, 0x3BA30, 0x3BABC, 0x3BBD8, 0x3BCF4, 0x3BEA0, 0x3C1F4, 0x3C48C, 0x3C530, 0x3C6A0, 0x3C720, 0x3F6E4, 0x3F758, 0x448EC + qsort 0x8C331 | 4,236 + 1,085 | ported UI kit | the by-id helpers |
| ANIM | Calamity animation, Zeus, BRPC picture | 0x7A0A0, 0x7A4A0, 0x7A4C4, 0x79ADC, 0x79C70 | 2,008 | KIT; fg_showflic / fg_flicsize (not in fg.c, D) | none |
| LOOP | Game loop, autosaves, phase music, eliminations | 0x54824, 0x14AA4, 0x5E7B4, 0x5E8EC, 0x5E9B0, 0x55258, 0x55304, 0x553C0, 0x55500, 0x22B00, 0x22DCC, 0x22F3C, 0x7C960 | 7,600 | KIT, RULES | 0x5E9B0 + 0x55258/0x55304 (save writer) |
| END | Final standings, credits | 0x14294, 0x1330C, 0x3CDF0, 0x3CE44, 0x7762C, 0x39F50 | 2,760 | LOOP | 0x7762C |
| TAX | Taxation, tax revolt | 0x66050, 0x66488, 0x667B0 (AI), 0x6685C, 0x66B54 (AI), 0x66C2C (AI), 0x66D8C | 4,952 | KIT, RULES, CAL1's 0x2468C, 0x20C70 | 0x667B0 |
| POP | Population expansion | 0x673A8, 0x67758, 0x680DC (AI) | 5,000 | KIT, CAL1's 0x28CFC; 0x680DC needs AIMOVE, CONFLICT | none |
| CENSUS | Census order | 0x13D44 + comparator 0x13DE8 | 164 + 104 | KIT, qsort | the comparator |
| SHIP | Ship maintenance and construction, human side | 0x60AE0, 0x60BD8, 0x6106C, 0x61940, 0x62488, 0x62658, 0x62774, 0x7A598, 0x7A610 | 5,988 | KIT, RULES, STK; SHIPAI for computers | 0x7A598/0x7A610 |
| MOVE | Movement phase, human loop and AI playback | 0x55574, 0x57160, 0x57254, 0x574B0, 0x577B0, 0x57810, 0x5790C, 0x57940, 0x5799C, 0x57A90, 0x57AFC, 0x57BD8, 0x582CC, 0x58330, 0x5A084, 0x5A2C4, 0x5A4BC, 0x5A750, 0x4D518, 0x4D82C, 0x4DACC | 13,336 | KIT, RULES, STK, ported MOVE/PANEL; AIMOVE for computers | the eight small tests 0x577B0..0x58330 |
| ROUTE | Ship routes over N.RL | 0x5843C, 0x584B4, 0x58568, 0x5861C, 0x58888, 0x58A74, 0x58DCC | 3,304 | load.c's N.RL | all 7 |
| CONFLICT | Conflict phase, plunder, losses box | 0x5AAA4, 0x5B224, 0x5B588, 0x5BA0C, 0x5BF30 (AI), 0x5C110, 0x5C1CC, 0x5CDE8 | 6,904 | KIT, RULES | 0x5C110, 0x5C1CC |
| CITY | City construction, surplus, city support | 0x32214, 0x333E8 (AI), 0x334F4, 0x5C238, 0x5C528, 0x5C62C, 0x5C750, 0x5C880, 0x5DE5C (AI) | 10,760 | KIT, RULES | 0x334F4 |
| CARDS | Acquire goods, discard excess | 0x6BD98, 0x6CB8C, 0x6CDA0, 0x6CF50, 0x6D010, 0x6D0C4, 0x6D178, 0x6D564, 0x6DB00, 0x6DBAC (AI), 0x6DE04 (AI), 0x6E770, 0x6E870, 0x6EC84, 0x6EDA8 (AI), 0x6F078 (AI), 0x77278, 0x77510, 0x6FBDC, 0x6FE68, 0x6FFC4, 0x71D30, 0x71E34 | 14,272 | KIT, RULES, ADV's 0x17EDC (AI) | 0x6DB00, 0x77510 |
| TRADE | Trade sessions, human side | 0x6F270, 0x6F454, 0x6F95C, 0x6FA54, 0x6FB2C, 0x6FDC8, 0x70074, 0x70294, 0x707A0, 0x70984, 0x70A6C, 0x7109C, 0x716BC, 0x71768, 0x71B68, 0x721A0, 0x723DC, 0x72928, 0x72EF4, 0x72FC4, 0x734E4, 0x73D4C, 0x7401C, 0x74D54, 0x75000, 0x75074, 0x75520, 0x75A1C, 0x75BD4, 0x75C08, 0x75DF0, 0x75EB4, 0x75F08, 0x75FB4 | 24,772 | KIT, RULES, CARDS; TRADEAI for computers | 0x75BD4, 0x75C08, 0x75DF0, 0x75EB4, 0x75F08 |
| TRADEAI | Trade, computer side | 0x68730, 0x68788, 0x68A64, 0x68B44, 0x68C90, 0x68FDC, 0x690AC, 0x6937C, 0x698A4, 0x6A5FC, 0x6AD70, 0x6AF34, 0x6B0EC, 0x6B928 | 12,924 | TRADE | 0x690AC, 0x6B928 |
| CAL1 | Calamity phase driver, Zeus, city-loss calamities (Treachery, Superstition, Slave Revolt, Civil Disorder, Iconoclasm, Piracy) | 0x1E0A0, 0x1E340, 0x1E388, 0x1E5B8, 0x1E69C, 0x23230, 0x23574, 0x235E4, 0x237A8, 0x23ACC, 0x213C0 (AI), 0x20564, 0x207B4, 0x20C70 (AI), 0x20FC4, 0x241D8, 0x2468C, 0x24EB8 (AI), 0x252BC (AI), 0x25548 (AI), 0x2E1D0 (AI), 0x28CFC | 18,596 | KIT, RULES, ANIM, CITY's 0x5C880/0x5C750, CAL2's 0x291D0/0x29550/0x27518 | 0x1E340, 0x1E5B8, 0x23574 |
| CAL2 | Unit-loss calamities (Famine, Epidemic, Flood), loss screens, secondary losses | 0x25A38, 0x27850 (AI), 0x27CEC, 0x27FCC, 0x260C4 (AI), 0x27518 (AI), 0x291D0, 0x29550, 0x29C94, 0x2E59C, 0x2EC38, 0x2D4F0, 0x2E45C (AI) | 22,040 | CAL1, CAL3's 0x1EAEC | 0x2EC38 |
| CAL3 | Volcano and Earthquake | 0x1E7A0, 0x1EAEC, 0x1EF28 (AI), 0x1F25C, 0x1F524, 0x1FDDC, 0x20068 (AI), 0x201B8 (AI), 0x203B4 | 7,620 | CAL1, CONFLICT's 0x5CDE8 | 0x20068 |
| CAL4 | Civil War | 0x29E34, 0x2AC94, 0x2AD50, 0x2B6EC, 0x2C190 (AI), 0x2C298 (AI), 0x2C778 (AI), 0x2C894 (AI), 0x2D10C (AI), 0x2D1C0 (AI), 0x2D3E0, 0x2EE6C | 14,320 | CAL1, CAL2, CONFLICT's 0x5CDE8 | 0x2AC94, 0x2C778 |
| CAL5 | Barbarian Hordes | 0x217A0, 0x1C3D0, 0x1C9FC, 0x1D234, 0x1D858, 0x1DEAC, 0x1DFFC, 0x7A4F0, 0x7AB24 | 8,088 | CAL1, CONFLICT's 0x5B224/0x5B588/0x5CDE8, ANIM | 0x1DEAC, 0x1DFFC, 0x7A4F0/0x7AB24 |
| MONO | Monotheism | 0x21914, 0x21A70, 0x21E8C (AI), 0x227B0 | 4,588 | KIT, RULES | none |
| ADV | Acquire civilization advances, human side and wrap-up | 0x162D0, 0x176BC, 0x17EDC, 0x1818C, 0x18AEC, 0x18C68, 0x18DD8, 0x19170, 0x193AC, 0x1A3A0 | 12,004 | KIT, RULES, ported INFO (0x194E4, 0x195D8, 0x71FB0) | 0x176BC, 0x193AC |
| ADVAI | Advances, computer side | 0x177EC, 0x19448, 0x19700, 0x1A888, 0x1B45C, 0x1B4D4, 0x1B538, 0x1B59C, 0x1BE08, 0x1C2DC | 10,212 | ADV | 0x19448, 0x1B4D4, 0x1B538, 0x1C2DC |
| STK | World State and Move List stacks, snapshots | 0x7A688, 0x7A700, 0x7A778, 0x7A7B0, 0x7A7E8, 0x7A85C, 0x7AA1C, 0x7AD24, 0x7AD7C, 0x7AE14, 0x7AF28, 0x7AFB8 | 1,648 | ported stack.c | all 12 |
| AIQ | AI queries, filters, scores | 0x48028, 0x4876C, 0x4892C, 0x48D44, 0x47BA0, 0x495C0, 0x49790, 0x49934, 0x4CC5C, 0x4CEE4, 0x4D110, 0x7BD5C, 0x7C838, 0x7CAB4, 0x7B044, 0x46A4C, 0x46B34, 0x468EC, 0x46998, 0x39C08, 0x160EC, 0x7701C | 15,449 | STK, CONFLICT's 0x5B588 | 0x46A4C, 0x46B34, 0x7CAB4, 0x7701C |
| AICITY | AI city planning and token gathering | 0x337C8, 0x33964, 0x33ECC, 0x3433C, 0x348B0, 0x353C4, 0x3566C, 0x3580C, 0x79128 | 12,768 | AIQ, AISEA, ROUTE | none |
| AISEA | AI ship transport and voyages | 0x36918, 0x374E0, 0x4AC50, 0x4D29C, 0x643C8, 0x648C8, 0x64BB8, 0x65010, 0x65CB0 + comparators 0x3530C, 0x35368, 0x65C4C | 16,908 + 284 | ROUTE, AIQ, STK, qsort | the three comparators |
| AIMOVE | AI movement planner 0x461BC and its attack/spread passes | 0x461BC, 0x37CF8, 0x387C8, 0x38E54, 0x39068, 0x396FC, 0x46BB8, 0x46CBC, 0x471CC, 0x474A0, 0x478D8, 0x47A44, 0x49A1C, 0x49C88, 0x49E04, 0x4A088, 0x4A2E0, 0x4BBDC, 0x4C074, 0x4C6E4, 0x4C9B8 | 22,748 | AIQ, AICITY, AISEA, CONFLICT, CITY's 0x5C238 | none |
| SHIPAI | AI ship maintenance and construction search | 0x61254, 0x617C0, 0x62840, 0x6332C, 0x63950, 0x639E8, 0x63AF0, 0x63B7C, 0x63C6C, 0x63D10 | 8,820 | AIMOVE, SHIP | 0x63950, 0x63AF0, 0x63B7C, 0x63C6C |
| PBEM | Play by e-mail (dropped by Erik's ruling, not ported) | 0x14A00, 0x230B8, 0x5055C, 0x508FC, 0x50D08, 0x518F0, 0x53A00, 0x53C30, 0x53EA8, 0x59298, 0x5A600, 0x5A8FC, 0x6E158, 0x6E53C, 0x7399C, 0x762E0 | 11,704 | - | - |

All 383 functions are assigned once (D: checked by script). Plus the extras qsort 0x8C331 and the four comparators.

Notes on the assignment:
- 0x5A4BC and 0x5A750 sit in MOVE, not PBEM: the AI ship playback 0x4DACC calls them (D: 0x4DB5F, 0x4DC2C,
  0x4DC7F). One reader had them PBEM-only; the inventory is corrected.
- Barbarian routing (CAL5) is not "AI": it runs for every victim, human or computer (D: 0x1C3D0).
- PBEM branches inside the other modules are dropped too (D: each tests byte 0xE1624 bit 0x80).

## Verification per module

Every phase module ends with the same check: restore S(P-1) in both, play the same clicks, compare the autosave of
phase P byte for byte and the data object at the 0x5E8EC stop. The lines below add what is specific.

- **RULES.** No DOSBox-X run needed.
  - Random-operation tests against a Python model written from the disassembly (as MOVE's move_test.c: hand
    vectors, 200,000 random operations, deliberately broken copies).
  - One spot check in DOSBox-X: BP at a 0x78FEC call inside a calamity, rangecmp of the nation and territory
    records before and after.
- **KIT.**
  - 0x79270 in all its modes (wait -560/-550/-500/ms; view -300/-250/-200/territory/-territory): frames at its
    f78BA4 wait (BP 0x78BD3) from the phases that use it. First: CENSUS's 100 ms home view, then a -500 message
    (an elimination or a calamity).
  - 0x795A8 / 0x7983C: box rectangle with rectdiff.py (the RESTART confirmation of the movement phase with
    the Restart Confirmations toggle on; "Lack of nations with sufficient trade goods").
  - qsort 0x8C331: table test of the port against a run of the original's routine. G: dump the array before and
    after one call of 0x13D44 and of 0x4D29C with 20+ routes (BP on the call and its return).
- **ANIM.**
  - First with animations off in aciv.cfg: then 0x7A0A0 is never reached (D: both callers test bit 0x04 of
    0xF097C).
  - Then on: the kaboom3 roll draws game_rand once when the counter in the word 0xF0980 is 0 (D). Same aciv.cfg in
    both runs, seed compare after an Earthquake or Volcano.
  - FLC frames: flcmatch.py against DOSBox-X captures (needs fg_showflic and fg_flicsize in fg.c, not there yet, D).
- **LOOP.**
  - First stop: restore the R4 autosave, BP at 0x54824's entry. Compare the data object. This checks 0x14AA4,
    which copies the restored 0xEED84 over 0xEF3C4 (D). G to settle here: whether the +5 bonus of 0x160EC then
    accumulates across a restore.
  - The autosave writer 0x5E9B0 without any phase: restore a save, let the port write it back, compare with the
    file (all 44,058 bytes plus the CRC). Then 0x55304's "r+b" rewrite: the file grows by 4 bytes and ends in two
    CRCs (D); compare with a file the original wrote at the end of a game.
  - Phase music 0x553C0: the data writes only (0xF61D8, sequence handles); sound by ear (Erik).
  - Eliminations 0x22B00: a save where a nation is down to its last tokens before CONFLICT (G: an H-save edited
    to leave one nation 1 token next to a stronger stack). Message frame, then the data at 0x5E8EC.
- **END.**
  - Final Civilization Standings: frame at its f78BA4 wait, reached by poking a nation to the last AST square
    before SHOW AST (G: in a save at the start of phase 20; f136B0(3) then returns 1).
  - Credits: frames at chosen steps (the roll is clock-paced, so only at a breakpoint: BP on its fg_intkey call
    0x1330C.., k = 0, 10, 52).
  - /C continuous test: ctest.txt lines compared as text (G: the memory statistics differ by design).
- **CENSUS.** S(POPULATION EXPANSION) of any round. No clicks. Compare the order 0xF0698/0xF069A and the autosave.
  Rounds with tied tokens on the board cover the comparator's tie rule.
- **TAX.**
  - Start: S(SHOW AST), since phase 0 follows phase 20 (D, 0x54824). The "end of round 3" save in hand is one.
  - Human rate screen (Coinage): an H-save where a human owns Coinage. Clicks: arrows -11/-12, CONTINUE.
  - Tax revolt: an H-save edited so a human cannot pay (stock < cities x rate), once with Democracy (the message)
    and once without (ChooseMultipleCitiesToRemove 0x66D8C, and the other humans' 0x2468C mode 2 screen).
  - AI rate 0x667B0 and AI annexation: M7 runs.
- **POP.**
  - Automatic expansion: any S(TAXATION) with enough stock. No clicks.
  - ManualPopExpansion 0x67758: an H-save with a human short of stock (edit +0x99). Clicks: ADD ONE (0xDC),
    ADD MAX (0xDD), DISTRIBUTE ALL (0xDB), RESTART (0x41), FINISHED. Frame after each.
  - 0x680DC (computer short of stock): M7.
- **SHIP.**
  - Maintenance (0x60BD8 human): an H-save at S(CENSUS) with ships. Clicks per ship: pay token (-6), pay treasury
    (-5), scrap (-8).
  - Construction (0x61940 human): the R4 autosave (start of SHIP CONSTRUCTION, D). Clicks: arrows, the three
    build buttons 0x36/0x37/0x38, RESTART, FINISHED. Compare with slot 0 (the original's S(SHIP CONSTRUCTION) R4).
  - A human with 4 ships goes through the AI search 0x62840 (D): leave for SHIPAI.
- **MOVE.**
  - Human loop: slot 0 (start of MOVEMENT R4) or an H-save. Clicks as in module MOVE of MAP_PLAN (land move,
    count buttons, ship move, EMBARK/DISEMBARK, UNDO, FIND UNMOVED, RESTART, FINISHED with its confirmation).
    Breakpoint 0x55FFD for each click (D: run_steps.sh already uses it).
  - Then the whole phase: data at the 0x5E8EC stop, plus the snapshots the loop copies after it (0xE5D20,
    0xE2DF8, 0xF1FB0, 0xEDF58/0xEDF5C; D).
  - AI playback (0x4D518, 0x4DACC, 0x5A084, 0x5A2C4): only with AIMOVE. Frames at the 300/500 ms blinks are
    timing-bound; compare the data after each nation (BP at 0x55A5F's return).
- **ROUTE.** No DOSBox-X run needed for most of it.
  - Table test of 0x5843C/0x58888/0x5861C over all (a, b, Astronomy, k 1..5) pairs on the real N.RL, against a
    Python model Claude writes from the disassembly.
  - One DOSBox-X spot check: BP at 0x58888's return during an AI turn, rangecmp 0x108820 (350 x 16) and 0x109E00.
- **CONFLICT.**
  - The phase after MOVEMENT: S(MOVEMENT) of a round with a contested area (R4 has several, G). With animations
    off, no clicks except CONTINUE on the losses box 0x5CDE8 and the human plunder screen (arrows -11/-12,
    CONTINUE).
  - City attack 0x5B224 and plunder 0x5BA0C (one game_rand, D): an H-save where a human has 7+ tokens on another
    nation's city.
  - The silent mode (show = 0) is only reached from the AI (D): M7.
- **CITY.**
  - CITY CONSTRUCTION: S(CONFLICT) with a human on a city site with 6 tokens; with Architecture the treasury arrows
    -11/-12 and BUILD CITY -13.
  - REMOVE SURPLUS and CITY SUPPORT: no clicks for surplus; for support an H-save where a human has too few tokens
    for its cities (0x5C880 REDUCE CITY -14, map click on a city).
- **CARDS.**
  - ACQUIRE GOODS: S(CITY SUPPORT) of a round with cities. Human clicks: ACQUIRE (-0x32), BUY (-0x33), FINISHED.
    With the "buy from any stack" variant (bit 0x20 of 0xE1624): the stack choice -0x34 and CANCEL.
  - REVEAL GOODS (0x6FBDC) appears only with more than one human (D): H-saves show it. Frame of the box.
  - DISCARD EXCESS: an H-save with a human holding 9+ cards. Clicks on cards (-0x35). Then 0x77278 reshuffles with
    game_rand (via f76F70, D): seed compare.
- **TRADE.**
  - DECLARE OFFERS / COUNTER OFFERS / ACCEPT TRADES: S(ACQUIRE GOODS) of a round where two or more humans hold
    the minimum goods (H-save). Each screen frame at its click wait; the offer composer's clicks (hand strip -0x1D,
    chart -0x1C, groups -0x11..-0x15, clear -0x25/-0x7A, DONE).
  - The turn-order shuffle of 0x723DC and the row shuffles of 0x734E4 draw game_rand through f76F70 (D): seed
    compare after each step.
  - Trades executed: rangecmp of the trade log 0x110F74..0x1110F5 and the detail block 0x112570 after ACCEPT.
- **CAL1..CAL5.** Calamities come from the cards, so a save at S(ACCEPT TRADES) whose hands hold the wanted
  calamity is the start. Easiest (G): edit the save to give a nation the calamity card (+0x13 count, +0x14 id,
  +0x1C giver) and remove it from its stack; check once in DOSBox-X that the edited save resolves it.
  - One run per calamity id 0x13..0x1E, human victim and (later) computer victim, primary and secondary.
  - Frames of each chooser at its f78BA4 wait (0x1EAEC, 0x1FDDC, 0x207B4, 0x2468C, 0x27FCC, 0x27CEC, 0x2B6EC,
    0x2D3E0, 0x5C880); the own loop of 0x29550 needs its poll address (risk 4).
  - Zeus (0x1E388, one game_rand per discard, D): a victim holding three calamities.
  - Barbarians (CAL5): the BRPC branch (depth-20 search) only if the normal search fails; G: force it by an edit
    that blocks every route. Data compare after; the spinner frames are timing-bound.
  - Civil War (CAL4): both factions' records 12 and 13 must compare exactly (risk 9).
- **MONO.** S(CALAMITIES) with a human holding Monotheism next to foreign units (H-save). Clicks: arrows,
  CONVERT TERRITORY (-16) or FINISHED, a map click.
- **ADV.**
  - S(CITY SUPPORT, phase 17) of a round where a human can afford an advance. The own click loop of 0x162D0 (risk
    4). Clicks: an advance button, hand cards (-0x1D), goods strip (-0x42), treasury arrows, Mining choice
    arrows, ACQUIRE (-0x3F), RESTART, FINISHED.
  - Tool Acquisition Wrapup 0x1A3A0: full frame at its f78BA4 wait.
- **STK.** No DOSBox-X run needed: unit tests of push/peek/pop/drop round trips over the fixed ranges, and the
  overflow / underflow messages (as stack.c's tests). G: one data compare at the AI's first push in a computer's
  movement (BP 0x7AA1C) for the depth counters [0xCAC54]/[0xCAC58] and high-water marks.
- **AIQ, AICITY, AISEA, AIMOVE.** Translated together, verified together (G: they call each other and only make
  sense as a plan).
  - First: one computer nation's movement. The R4 slot 0 save (start of MOVEMENT); BP at the return of 0x461BC
    (0x55A64, D: call at 0x55A5F) for the first computer. Compare the move lists 0x107694/[0x1079B4] and
    0x1079B8/[0x107D3C] with rangecmp, then the whole data object.
  - Then the AI's other entry points: POPULATION EXPANSION short of stock (0x680DC), SHIP MAINTENANCE housekeeping
    (from turn 7, D), SHIP CONSTRUCTION (via SHIPAI).
  - Then whole rounds of a 1-human + 2-computer new game from a fixed seed, every autosave compared.
- **TRADEAI, ADVAI, the AI parts of CAL1..CAL5, CITY, CONFLICT, MONO, TAX, CARDS.** In the same mixed-game rounds.
  The game_rand order is the main check (risk 2): seed compare at each 0x5E8EC stop.
- **SHIPAI.** Last of the AI: its search prunes by clock() (risk 3). First with a virtual clock that never hits the
  budget, compared with DOSBox-X runs where the budget is never hit either (G: check [0x10F2C8] and the elapsed
  ticks in the dump).
- **M7 end checks (OVERVIEW §5).** The self-running demo /SRD and the continuous test /C: whole AI-only games,
  autosave by autosave against DOSBox-X from the same seed.

## Order

The goal is the earliest end-to-end playable turn, then the rest of M6, then M7.

1. **Helpers:** RULES, KIT (with qsort), STK, ROUTE. Mostly Mistral (see below); Claude does 0x79270, 0x795A8,
   0x7983C and qsort.
2. **LOOP skeleton.** 0x54824 with all 21 cases. Each phase routine stops with `not_ported(address)` until its
   module lands (as dispatch.c did). Plus 0x14AA4, the autosaves, 0x553C0, 0x22B00. Check: the R4 restore to the
   loop entry; the save writer round trip.
3. **T1: round 1 of a new 3-human game, end to end.** In round 1 most phases take their "nothing to do" path
   (G: no cities, no cards, no ships), so T1 needs:
   - TAX's 0x66050 (nobody has cities), POP's 0x673A8 (automatic), CENSUS;
   - SHIP's 0x60AE0/0x60BD8/0x61940/0x62658 (no ships, no sites);
   - MOVE's human loop (the players move their first tokens);
   - CONFLICT (contested areas once tokens meet, G), CITY's 0x32214/0x334F4/0x5C238/0x5C528/0x5C62C/0x5C750;
   - CARDS's 0x6BD98 (no cities), 0x6E770 and 0x77278/0x77510 (reshuffle, game_rand);
   - TRADE's 0x55500 (no session), CAL1's 0x1E0A0/0x1E340, MONO's 0x21914/0x227B0;
   - ADV's 0x162D0 with 0x17EDC (nothing affordable), SHOW AST (f136B0(3), ported) and 0x55258.
   - Check: all 20 autosaves of round 1 and the end-of-round SAVGAMEB.DAT byte-exact, same seed.
   This is the first point where the port plays a turn on its own.
4. **Rounds 2-6 of the same game** bring the rest of the human side, in turn order: POP manual, SHIP, CITY with
   sites, CARDS acquire/buy/discard, TRADE, CAL1..CAL5 (as the cards bring them; edited saves for the rest),
   MONO, ADV, END. Each module lands with its own commit and checks.
5. **M7, the AI:** AIQ + AICITY + AISEA + AIMOVE together (one computer's movement first), then SHIPAI, TRADEAI,
   ADVAI, and the AI parts left in the phase modules. Mixed-game rounds, then /SRD and /C.
6. **ANIM** whenever fg_showflic exists (M8); its game_rand part (0x7A0A0's roll) is needed from step 3 if the
   comparison runs keep animations on. Default: run comparisons with animations off until ANIM lands.

## Mistral candidates (spec-able, no EXE knowledge needed, checkable without DOSBox-X)

Each comes with a written spec from the inventory records and a check Claude prepares (a Python model from the
disassembly, test vectors from DOSBox-X dumps). Claude reviews before anything lands (HANDOFF protocol).

1. **RULES** (16 functions, 2,516 bytes): pure record edits on the nation, area and territory-state records, like
   the eight MOVE primitives that were checked this way. Check: random-operation tests against the model.
2. **STK** (12 functions, 1,648 bytes) and the snapshots 0x7A598/0x7A610, 0x7A4F0/0x7AB24: fixed-range copies,
   depth counters, the overflow and underflow messages. Check: round-trip unit tests.
3. **The save writer** 0x5E9B0 with 0x5E8EC, 0x55258, 0x55304: the field list is in the record (44,058 bytes, the
   order of 0x5F05C, D). Check: read then write every autosave of the run copy, byte-equal, CRC appended; and the
   4-byte growth of 0x55304.
4. **ROUTE** (7 functions): N.RL queries. The format is in OVERVIEW §4 and the records. Check: a table over all
   pairs against Claude's Python model.
5. **Small pure formulas:** 0x667B0 (AI tax rate), 0x690AC (calamity harm), 0x1B4D4, 0x1B538, 0x1C2DC, 0x193AC /
   0x19448, 0x176BC, 0x2AC94, 0x6B928, 0x7CAB4, 0x6DB00, 0x7762C, 0x46A4C, 0x46B34, 0x2EC38, 0x2C778. Check:
   exhaustive or random tables against the model.
6. **Sorts with an exact tie order:** 0x1DEAC, 0x20068, 0x4D82C, and the four qsort comparators. Check: random
   arrays with many ties against the model. (Watcom's qsort itself stays with Claude: it needs the disassembly.)
7. **Small list builders:** 0x1E340, 0x1E5B8, 0x23574, 0x5C110, 0x5C1CC, 0x334F4, 0x63950, 0x63AF0, 0x63B7C,
   0x63C6C, 0x77510, 0x75BD4..0x75F08 (trade bookkeeping). Check: random states against the model.
8. **The text sweep:** add this plan's slips (below) to docs/TEXT_SWEEP.md for Erik's rulings.
9. **tools/verify/dlgxy.py** (button centre from a dump, as specified above).

Not for Mistral (G): the screens (they need the pixel rules and DOSBox-X frames), anything that calls 0x79270, the
AI planner, and everything touching game_rand or the stacks' push/pop order.

## Easter eggs, jokes and oddities (deduplicated, from all eight readers)

Numbers in brackets refer to OVERVIEW §6 when an item is already there.

**Eggs and jokes**
1. **kaboom3.wav (D, 0x7A0A0).** For Earthquake and Volcano only: when a 6-bit counter kept in aciv.cfg (bits
   9..14 of the word 0xF0980) is 0, a 1-in-10 game_rand roll plays kaboom3.wav instead of the usual sound. The
   counter is then set to 51 and counts down once per calamity animation; aciv.cfg is rewritten each time. The game
   busy-waits until kaboom3 ends. [11 has only the file's GoldWave signature.]
2. **The BRPC at work (D, 0x1D858).** During the long barbarian route search ("This could take several minutes.")
   an ASCII spinner | / - \ is appended every 250 search steps, brpc.wav loops and the BRPC picture shows; after at
   least 4 s, "The BRPC is deadlocked." [4]
3. **Skull & Crossbones, Inc.** declines new cities "to their portfolio" when the Pirates hold 9 (D, 0x241D8). [3]
4. **Zeus** has his own animation code: -298 plays zeus.flc with zeus.wav, -299 shows brpc.gif even with
   animations off (D, 0x79270). [5, 13]
5. **The credits roll (D, 0x1330C):** 63 rows; besides "AI Design Nemesis" and "Original Boardgame Design (Thank
   You!)" [9] it credits Ancillary Programming, Animations, Testing Coordination and the Manual. The title row is
   "A D V A N C E D   C I V I L I Z A T I O N ~" with a trailing tilde (glyph in COGNAC19 to check).
6. **The computer never floods its own river (D, 0x2E45C).** Table 0xBEDE0: Egypt the Nile, Assyria and Babylon
   Tigris-Euphrates, Illyria and Thrace the Danube, Iberia the Rhone; Iberia's entry is rewritten to the Po in
   3-player games.
7. **Crete cannot be the victim of the Barbarian Hordes** (D, 0x217A0), and the barbarians are directed by the
   nation with the most units in stock, each city still in stock counting 5 (D).
8. **Hidden nations:** 10 PIRATE, 11 BARBARIAN, 12 FIRST FACTION, 13 SECOND FACTION (D: the name table). Civil War
   works by moving units into records 12 and 13 (D, 0x29E34).
9. **Tax Revolt borrows the Pirates' screen:** ChooseCitiesToLoseToPirates in mode 2 (D, 0x2468C).
10. **A/An by nation:** "A Thracian/Cretan/Babylonian city", "An ..." for the others (D, 0x789EC).
11. **Dead developer switch [0xF40B2] (G):** set, cities cost 1 and 2 tokens instead of 6 and 12 (D, 0x334F4); no
    store in the listing writes it. Like the opening-map override [14] and "show all hands" [19], which the
    trade screens 0x734E4/0x7401C also test.
12. **Developer counters left on screen (D, 0x62840, 0x61254):** "%s considers building ships. (%d,?,?)" and
    "(0,%d,%d)" count down the AI's options; ship maintenance gets a | / - \ spinner; "Maintenance housekeeping
    for %s" and "Construction housekeeping for %s" from turn 7.
13. **The /C continuous test (D, 0x14294)** appends to ctest.txt at every game end: "End of game %d.", memory
    statistics, "Max WorldStack: %d Max MoveStack: %d Max RouteList: %d", "RoutingMax: %d"; then it starts a new
    game by itself. [1, 7]
14. **Developer trace names** passed to no-op loggers (D): "Acquire Civ", "ChooseCitiesToLoseToPirates",
    "NationLosesUnits", "ChooseFirstFaction" (also used for annexing), "ChooseMultipleCitiesToRemove",
    "ManualPopExpansion", "DrawTradingCards", "DrawInitialDrawingScreen", "PBEMViewer", "Animations", "movement",
    "movement - MSStore", "TempTerr", "Tempnation", "DoShipContruction" [10].
15. **Empty "nothing happened" messages (D):** no conflicts (0xB5C52), no surplus (0xB5D54), city support OK
    (0xB5DC7), no maintenance (0xB61A6), no construction (0xB6311), no taxes (0xB892B), no goods (0xB8BF4), no
    discards (0xB8E18), no Monotheism (0xB1E9E), and the census and calamity phases' "". The box shows only a pause
    and the home view (G: texts removed).
16. **Tool Acquisition Wrapup** has eight invisible full-width row buttons (ids 0xB2..0xB9) that do nothing (D,
    0x1A3A0; G: a planned detail view).
17. **Labels that stick (D):** Final Standings relabels the shared CONTINUE "EXIT" and leaves it; REVEAL GOODS leaves
    "NOT REVEAL GDS" on dialog 0xBBBEC.
18. **"Already used to reduce Famine losses."** Grain spent against Famine cannot buy advances (D, 0x1818C).
19. **Volcanoes (D, 0xBDE18):** Mt. Vesuvius (CAMPANIA, NEAPOLIS), Mt. Etna (MILAZZO, SYRACUSE), Mt. Thera (THERA);
    "%s eruption casualties".
20. **Save stamp:** every save holds "Advanced Civilization" and "1.11B1" (D, 0x5E9B0). [2]
21. **The empty hook 0x5E7B4** is called after every phase and every calamity (D; G: a removed trace or autosave
    hook).
22. **Temporary files "junk" and "junk2"** for screen saves under message boxes and animations (D). [7]
23. **Alternate Tool Acquisition Colors** (bit 0 of 0xF0981) makes buyable advances blue (D, 0x17EDC). [21]
24. **The YES/NO box puts NO above YES** (D, 0x795A8). "N O N E" is letter-spaced on the Accept Trades screen (D).
25. **PBEM leftovers (D, not ported):** "%s to xxx file" as a save description; SENDHOST.TXT with "the the", lost
    words and "to to"; "Trade Session %d - %s" whose six session names are empty strings; the nation-count words
    "Zero".."Eight" (9 gives ""); system("del *.pl? >NUL") and system("del *.hst >NUL").
26. **Data quirks (D):** row 0 of the AI preferred-site tables 0xC0618/0xC06B8 holds garbage, and Iberia's rows are
    rewritten at run time by 0x2C298; a sixth era row in 0xC2864 (225/175/125) serves human targets; the
    adjective table 0xC76C4 runs on into the calamity names.
27. **Copy-paste leftover:** the Earthquake chooser 0x1FDDC removes the Volcano's dialog 0xBE21C instead of its own
    0xBE608 (D; harmless only because both start with id 0x39).
28. **Duplicate strings** the compiler did not merge (D): "%s has been eliminated from the game." x2, "Do not send
    anything to the host." x2, "%s is overrun by the Pirates." x3, "Assignments for Famine/Iconoclasm secondary
    losses" x2 each, "GRAIN/USED/LOSSES/REDUCED TO" x2, "Are you sure you want to Restart?" x2, "%s reduces %s" x2,
    "Your offer is incomplete.\n" x2, "Replace your bluffs with..." x2, twenty "Button not found".

**Text slips** (fixed by default, `--original` keeps them; all D, addresses in the records):
- "Montheism" x2 [known]; "Enlightenment nullifys the effect."; "Democrary spares %s from a Tax Revolt.";
- "beneficary" x4 (Civil War; "beneficiary" elsewhere); "%s choses the %s to flood."; "TIGRUS-EUPHRATES";
- "%s reduces a city in %s ." (space before the period); "for the Pirates to takeover" x2;
- "There are no eligible secondary losses" (no period); "%s removes a city in %s" (no period, its twin has one);
  "Assignments for Epidemic secondary losses." vs "... losses" (the two paths differ);
- "%s,you have no stock to pillage with." (no space); "%s, choosing number units to transfer..." ("number of");
- "Crete can not be the Victim of Barbarian Hordes." ("cannot", "victim"); "thru Tax Revolt";
- "NOTE: ... ship maint./const.for the other players" (no space, PBEM);
- "World/Move Stack has popped once to often." [known] and "Move Stack" where the push says "Move List Stack";
  "...blown its max size. > %d" prints 7 for a limit of 6 pushes;
- the computer's annexing messages always say "Second Faction", even for the first;
- "%s has acquired \nA, \nB, \nC and \n." with three or more advances: " and" goes after the last name (the test is
  count-1 where count-2 is meant, 0x19700);
- "Truly stated" vs "Truly Stated"; "The responses to African offer..." (adjective without article);
- "%s is a victim of his own Treachery." (fixed "his"; for Erik);
- abbreviations "Acqrd", "NOT REVEAL GDS" (for Erik: keep, they fit their buttons).
- Not a slip: "SUDETEN" after "PIRACY" in the string pool is territory 0's name.

## Risks

1. **Size and shape.** 309 KB of code, of which the AI is the larger part. Biggest functions: 0x55574 (7,148
   bytes, many gotos), 0x260C4 (5,204), 0x162D0 (5,100), 0x32214 (4,564), 0x3580C (4,364), 0x4AC50 (3,980), 0x29E34
   (3,680). Each gets its own translator and reviewer, with its address map in the file header.
2. **Random-number order.** Direct game_rand calls (D, the call scan): 0x177EC, 0x1B45C, 0x1D234 (x2), 0x1E388,
   0x5BA0C, 0x61254, 0x62840, 0x68C90 (x3), 0x6937C, 0x6A5FC (x2), 0x7A0A0, plus the ported 0x302C4, 0x45F88,
   0x6B9AC, 0x76F70, 0x7BCD4. Indirect through f76F70: 0x723DC, 0x734E4, 0x77278, 0x68C90.
   - Several draws are conditional (only on one branch). One missed branch shifts every later draw.
   - 0x7A0A0 draws only with animations on: the aciv.cfg toggles must be identical in both runs, and aciv.cfg must
     be backed up and restored by the run scripts like the saves.
   - Seed compare at every 0x5E8EC stop catches a shift at its phase.
3. **AI determinism.**
   - Most AI rolls come from the pre-rolled table 0x112A94 (made by 0x7BCD4 each TAXATION, saved; D), not from
     game_rand.
   - [0x108198] (0x474A0's roll counter) is never reset and not saved (D). Keep it a process-lifetime static, and
     compare only runs that both start from a fresh program start.
   - Watcom qsort 0x8C331 is not stable (D): translate its algorithm; libc qsort would change the AI's choices.
   - 0x6332C prunes the ship search by measured clock() time (D). Results then depend on host speed (G). The port
     needs a virtual clock; DOSBox-X runs must be checked to stay under the budget, or the clock values matched.
   - The World State Stack and Move List Stack are pushed, popped, peeked and dropped in branch-specific orders
     (D). Every trial must be undone exactly as in the original.
   - Some AI simulations (0x5AAA4(0), 0x5C238(0)) change globals the stacks do not cover: 0x109E24, 0x109E08..,
     0xF547C/0xF548C, 0xF5934, button 7's redraw (D). Keep them literally.
4. **Click loops that do not use f78BA4** (D): 0x162D0 (Acquire Civ), 0x29550 (secondary-loss assignment, with
   150 ms auto-repeat while held), 0x55574 (movement, known: 0x55FFD), 0x6E158 (PBEM, dropped). The DOSBox-X
   scripts need each loop's poll address (G: the `cmp [0xE1630],0` after fg_mousepos, as pokeclick.sh uses).
5. **Waits.** Busy waits on clock() outside f78A20: 0x1330C, 0x1C3D0 (1.5 s and 4 s), 0x32214, 0x461BC, 0x60BD8,
   0x61254, 0x61940, 0x62840, 0x66050, 0x673A8, 0x680DC (this one not scaled by /C) (D). The port's virtual
   clock must advance in all of them; DOSBox-X frames only at breakpoints.
6. **Loops that can hang (D code, G reachability):** 0x57810 (FIND UNMOVED with only loaded, moveless ships),
   0x260C4 (token passes with no tokens left), 0x2D1C0 (stale faction counts after 0x2C894), 0x6E870 and 0x6EDA8
   (a hand over 8 made of calamities), 0x67758 (DISTRIBUTE ALL), 0x5C750 (support never reached). Rule each:
   literal under `--original`, guarded by default (for Erik).
7. **Reads of stack garbage (D):** 0x14294 (more rows than nations), 0x1E69C / 0x70984 / 0x716BC / 0x75CA8 /
   0x78F04 (card or city not found), 0x21E8C (no unit records), 0x217A0 (no director), 0x2E1D0, 0x6B0EC, 0x71768
   (stores an uninitialised count into [0x1105AC + 4n]). A port cannot reproduce stack garbage. Check in DOSBox-X whether each path is reachable; if it is,
   compare only runs that avoid it, and fix it by default.
8. **Out-of-range reads that are deterministic (D), to keep byte-exact:** 0x2C894's faction rows (nations 12/13
   index 10-nation tables), 0x46998 (t = 157), 0x260C4's flood capacity (two bytes before the count), 0x36918's
   code byte outside 0xC1938, 0x5CDE8's short names for nations 10..13, 0x1A888's three slips, 0x6B928's transposed
   score history (past the table after turn 10).
9. **State outside the save** (D): Iberia's AI rows (0x2C298), 0xBEDE8 (0x2E45C), dialog labels and positions
   rewritten in the data object (EXIT, NOT REVEAL GDS, REPLACE CITY...), the faction records 12/13, the shadow
   tables 0xF09E8/0xEB3F0, the AI time counters 0x10F2C0... A restore does not reset them, so compare runs must
   start from the same program state (fresh start, then restore).
10. **Heap pointers in the data object:** [0xEED74]/[0xEED78]/[0xEED7C] (snapshots of six routines), [0xEED80]
    (stack.c frees it; 0x2C778 reads AI table bytes through [0xEED80 + 160n + t + 1], D: settle the 0xEED80 /
    0xEED84 layout before CAL4), [0xF09B4], the Move List Stack slots 0x112A1C.., route block [0x10740C]. All are
    port handles in by_design.txt.
11. **Possible 1995 bugs that change game state** (D code, G effect), for Erik's rulings. They must stay identical
    in the byte-compare runs (run those with `--original` until ruled):
    - Zeus does not shift the givers' list (0x1E388); 0x24EB8 lets the Pirates take a 10th city;
    - 0x27850 reads Coinage where Grain is meant; 0x2B6EC's ALL path does not reduce the receiver's stock;
    - 0x2C894 removes faction tokens without updating the faction's counts; 0x2D1C0 compares territory+1;
    - 0x2D4F0 tests the primary victim's human flag for each secondary victim; 0x3580C's unscaled record index;
    - 0x4C074 overwrites its own territory list; 0x46BB8 clears 4 bytes of a 640-byte array; 0x49E04 fills only
      the first target; 0x39C08 tests a restored record;
    - 0x213C0's coastal loop reads past its list; 0x1DFFC adds only the first city loss; 0x5C110 / 0x5BF30 /
      0x5DE5C off-by-one nation rows; 0x6AF34 clears the wrong table; 0x680DC cuts 1 token per area and writes
      past its array; 0x61940 reads area record cand for territory cand;
    - 0x6D564: a click between stack columns, a cancelled buy still counted, an empty stack 8 still paid;
    - 0x14AA4 may make the +5 opening-map bonus cumulative across restores (G, check first, LOOP).
12. **Sound and pictures.** AIL calls are port no-ops; their data writes stay (D). fg_showflic and fg_flicsize are
    not in fg.c (D); ANIM waits for them. fg_makegif/fg_showgif "junk" files stay in memory, as in INFO.
13. **Values with no writer found (G):** [0xF5470] (round number?), [0xF40B2], int16 0x112C98. Check in a DOSBox-X
    dump whether a block copy fills them.
14. **PBEM dropped** (Erik's ruling): 16 functions not ported, and PBEM branches inside about 30 others removed.
    Each removal is listed in its file's header, so a later PBEM port knows where they were.
15. **Duplicate helpers.** Many routines here are near-twins of each other or of ported ones (0x193AC/0x19448,
    0x58A74/0x58DCC, 0x5A4BC/0x5A600, 0x70984/0x716BC/0x75CA8, 0x5C1CC/0x7C760). Translate each twin separately
    and grep port/src for an existing f-name before adding (the Fulvia lesson in reverse: a new helper must not
    shadow an old one).
