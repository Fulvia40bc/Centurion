# Gap notes: what of the 1990 game is not ported yet, and the small gaps decoded

**Status 2026-11-17:** gap 1 (4733 after a battle) is ported in battle.c result(); gap 5 (the cursor) in screen.c; gap 11
(the province frame dumps) in province.c / game.c; gap 12's three stops are listed as 1990 bug fixes in enemy.c's header;
the stale comments of section 7 in battle_ui.c / endturn.c / enemy.c are corrected; the window also maps the keypad
(Num Lock digits, + - * /, keypad 0 = Insert). Not done: gaps 2, 3, 4, 6, 7, 8, 9 (not small, or out of scope), 10 (floppy
only, not reachable in the port), 13 (nothing to port).

Claude, 2026-11-17 (completeness audit). Labels as elsewhere: **D** = decoded from the code, **V** = verified by a run against a
DOSBox capture (the check is named), **G** = guess. No C code was changed for this file.

Method. (1) Every "not ported" / "NOT PORTED" / "not yet" / "stub" / `game_not_ported` / `unported(` / "nothing in the port"
mention in port/src. (2) A call graph of the EXE: a fresh llvm-objdump listing of disasm/EXE_RUNTIME.bin (clang .incbin, code16),
every near-call target in the EXE (325 targets below 0xB0E0) plus every EXE call made from the five overlay listings, each checked
for its address in port/src (`0x4733`, `4733`, `_4733`). 235 are cited; the 90 that are not are all C runtime / library / DOS
plumbing (printf family 0xA790..0xAD8D, long arithmetic 0x9D04 / 0x9D36, LZW / file loading 0x13F8 / 0x16FB / 0x17B4 / 0x18C2,
heap 0x198F / 0x7464 / 0xA13A.., video and memory start-up 0x7DAA / 0x80D6 / 0x8268 / 0x843A / 0x9A62, mouse driver glue 0x06B3 /
0x06D4, AdLib probe 0x055C..0x05AF, the file-error box 0x5A9A, exit 0x0EE7 / 0x0322 / 0x03F1), except **0x2983** (keyboard mode,
gap 2). Functions with a `push bp / mov bp, sp` prologue and no direct reference were listed too: all are overlay-only helpers
or behind a jump table (0x762D Save, reached from the Game Control table 0x1ADD). (3) The same for the overlay-internal call
targets: every one of sections 2-5 and the credits is cited (BD48 = free the battle sprite sheets DS:7280..72C6 and C715 = a
position offset helper are inlined in battle.c; naval C4D8 / C4E8 are naval.c's sin_C4D8 / cos_C4E8). (4) The thunks B06E..B0CE
and the section 1 entries checked by hand. Scripts: `<scratch>/finish/audit/cg2.py`, `cit.py`, `ovl.py`.

**Result: every game-logic routine of the EXE and of overlay sections 2-5 is ported, except the one call in gap 1.** What is left is
input / output variety (keyboard-only mode, non-VGA video, sound), start-up, intro and copy protection, the mouse cursor sprite,
floppy-disk plumbing, and three 1990 reads of uninitialised locals that the port stops instead of following.

## 1. The gap table

| # | gap | entry (0823:) | what it does in 1990 | how a player meets it | size | capture to verify against |
|---|---|---|---|---|---|---|
| 1 | **New commander after the general falls** | battle E58E..E5AA -> EXE **4733** | the land battle's result E21A: if the legion's commander ([1A20+6c] status) died in this battle (2) and the legion still exists, 4733(L) rolls a free commander and shows "<name> is assigned\nto command <numeral><legion>" | any land battle in which the Roman general unit is killed and the legion survives (won or lost with men left) | 4 lines in battle.c result(), ported 2026-11-17 (4733: battle.c assign_commander + commander_msg_4733, shared by battle.c, enemy.c and legions.c since the review) | none: no capture has a status-2 commander (all 31 DS images in tools/dosbox/results read 0/1). 4733's roll and message are V through its End Turn caller 50E0 (t9: 0 bytes, rerun today) and TEXTBOX_NOTES (b1.png message box 7285/7285 px). Needs a new DOSBox battle with a fallen general (BP at 0823:E5AA) |
| 2 | **Keyboard-only mode** ([00BA] = 0) | 3603 (82 lines, about 65 of them the keyboard part), 2983 (50), 5901 (93), 56B6 / 57F7 bullet sprite [C1D6] + 5x3 marker, 358A waits in place of 35DB, battle D33A target cursor, [3E40] = 3E16 "Hit any key to continue" | no mouse: a keyboard cursor (arrows / keypad, 8 px, 1 px with Ctrl or Scroll Lock), Enter / '+' = right button, Space / Insert = left button; menus run by arrows with a bullet per item; Build Fleet's +/- by Up / Down | run `CENTURIO k`, or a PC with no mouse driver (start-up: `if ([BA] && mouse_init()) [3E40] = 3E2E; else [BA] = 0`) | about 300-350 asm lines over 8 routines; 3603, 2983, 03FF decoded below, 5901 / bullets in MENU_NOTES 4.1 / 3.3 / 3.5, D33A's keyboard part outlined in BATTLE_UI_NOTES 9 | none (every capture has a mouse, [BA] = FFFF). DOSBox with `CENTURIO k` would give one; ask Erik first |
| 3 | Command-line options | section 1 **B0E0** (73 lines) | `k` = keyboard mode; video `x m e tandy v` -> [9AB2] = 0 2 4 6 8; sound `nosnd pc tn roland mt lapc adlib ad` -> [3EEA] = FF 11 14 13 13 13 12 12 | typed after CENTURIO | small; decoded below (start-up: the start-up agent's lane) | runtime DS: [9AB2] = 8, [3EEA] = 12h, [BA] = FFFF with no option (t10_in.bin) |
| 4 | Non-VGA video (EGA, Tandy, MCGA/CGA G) | branches of 67E4 / 680F / 6887, [9B26] != 2 (province skies, colours [6344..6348] stay 3/7), Tandy keys in 03FF | other graphics adapters | the video options, or a non-VGA PC | medium; not needed (the port is VGA) | none |
| 5 | Mouse cursor sprite | 1BE6, 1CCE, 1D19, 1D55, 1D8A, 86B2, sprite pointer [0684] (1DF5) | the software cursor on every screen | always visible | ported 2026-11-17 (screen.c; CURSOR_NOTES) | the cursor pixels in every capture: map 84 px, naval end screen 93 px, Rome 18 px |
| 6 | Music and sound | INT 8 handler 0000 ([3EE9] on/off, [3EEC]), 072E / 074B driver calls, 5D29 / 5D73 / 5EB5 file part, 055C AdLib probe, Ctrl+S toggle in INT 9 03FF | map / battle / race music, effects | always | ported 2026-11-17 (music.c: the AdLib driver literally, opl2.c the chip, SDL2 audio in a window game; MUSIC_NOTES) | register logs = tools/adlib_regs.py for all 17 songs (no sound capture) |
| 7 | Intro and title | section 1 **B694** (241 lines), B970 (intro credits, the first 11 pairs; in credits.c but not called), texts DS:6452..6610 and 6619 | the legend of Romulus and Remus, title, credits | at every start | ported 2026-11-17 (intro.c; INTRO_NOTES) | none known |
| 8 | Start-up | section 1 **B18A** (220 lines) incl. **B397** (75), 7DAA, 80D6, 1E02 | hardware, sprites, font record, colours, enemy fleet records 3-5 | at every start | out of scope here; the port's start (main.c: state_init + NEWGAME.LZW + map.c map_startup) gives the same state (GAMELOOP check 1/2, 0 bytes outside the description) | V for the state (fresh_275BC.csv) |
| 9 | Copy protection | section 1 **B466** (241 lines) | "What is the <Northern/...> <Town/...> in the province of <name>?", 3 tries, else exit(2) | at every start | skipped by Erik's decision (HANDOFF 2026-11-08) | none |
| 10 | Floppy plumbing | 1E21 (disk check, decoded below), 7B1E (save-disk prompt: ported, silent while [0698] = 0), 0363 / 0373 drive switch of Save / Load (not ported), 1E02 / 037A drive count | prompts "Please Insert Program Disk\nInto Drive A: or B:" (DS:06AC); two-floppy drive swap | only when run from floppies | small; not needed (the overlays are compiled into the port, saves live in the current folder) | none |
| 11 | Province screens, Mode 1 frame | 30E6 frame sprites [C0F6] | the panel frame of the province and Rome screens (and race prepare menus) | every province screen | out of scope here (frame agent) | scr_narbonensis_province / scr_rome_province |
| 12 | Three 1990 reads of uninitialised locals in the enemy armies' turn | 4E2D (no free rebel slot), 4F27 at 4FE5 (an army standing in Rome in mode 0), 4F27 at 4F6F (a mode-0 army with no weaker neighbour) | 1990 goes on with a stale stack value (decoded below); the port stops / skips and prints a stderr line | 4 rebel armies alive and a rebel spawn; an enemy army in Rome (Rome lost); a cornered army | each under 20 lines; partly decoded below | none |
| 13 | Retrace waits | 84BB (and the 84AD / 849D helpers) | waits for the VGA vertical retrace | timing only | nothing to port | n/a |

Not gaps (checked): "Sic Transit" (map.c, 0x0986 test at 0x096E..), About / Difficulty / Fine Tuning / New Game / Save / Load / Quit
(gamectl.c), the battle UI's **Insert key** (sdl2_platform.c maps SDLK_INSERT to key_down[0x10] since commit 4c39bc5, and
battle_ui.c tick_hook tests key(0x10): the comments that say otherwise are stale, section 5), the Asterix village in Move Legion
(legions.c 578 -> province.c hook_click -> map.c zone_at, with the message), the headless-only "not ported" boxes (enemy.c / legions.c / endturn.c `unported`: battle_auto returns 0
only in a scripted or --end-turn run without battle choices; with a screen the battle is always fought).

## 2. Gap 1 decoded: the commander fell (D)

**Where the death is recorded** (battle overlay, ported as battle.c line 262): when a unit of type 3 (the general, one man) loses its
last man (B281..B2C0: `dec W(u); if (W(u) == 0) { B925(4, i); B4E5(side); if (side == 0) B(0x1A20 + 6 * B(0xC28C)) = 2; }`) the
commander of the fighting legion ([C28C] = legion +0x10, copied at the battle's start) gets status 2. Status values of the 24
commander records DS:1A1C + 6c {+0 name pointer, +2 charisma, +3 voice, +4 status}: 0 = on duty, 1 = free, 2 = fallen.
A fallen commander also switches off the general's bonuses in the same battle (B7BE / D8AD test status == 0; port battle.c 114 / 708).

**The call** (end of the result routine E21A(L, Z), after the report and its click wait 35F9 at E58B):
```c
    /* E58E */ if (B(0x1A20 + 6 * B(0xC28C)) == 2 && B(0x1918 + 20 * L + 0x0D) != 0xFF) commander_assign_4733(L);   /* E5AA */
    /* E5AD */ return won;                          /* [bp-0xC] */
```
The legion test: legion_lost 4816 (called earlier in E21A when the legion has no men left) sets +0x0D = FFh, so a destroyed legion
gets no new commander (4816 also leaves a status-2 commander at 2 and sets any other back to 1).

**EXE 0x4733(L), `ret 2`** (48 instructions; ported as battle.c assign_commander + enemy.c / legions.c commander_msg_4733):
```c
void commander_assign_4733(int L) {
    uint16_t r = 20 * L;                            /* [bp-0x1C] */
    if (B(0x1925 + r) == 0xFF) return;              /* 474C: the legion is not raised */
    uint8_t list[24]; int n = 0;                    /* [bp-0x1A].., si */
    for (c = 0; c < 24; c++) if (B(0x1A20 + 6*c) == 1) list[n++] = c;               /* 4756: the free ones */
    if (n == 0)                                                                      /* 4778: none free: */
        for (c = 0; c < 24; c++) if (B(0x1A20 + 6*c) != 0) { B(0x1A20 + 6*c) = 1; list[n++] = c; }  /* the fallen come back */
    c = list[dice_076B(n)];                         /* 47A9: one dice roll */
    B(0x1928 + r) = c;                              /* legion +0x10 */
    B(0x1A20 + 6*c) = 0;                            /* on duty */
    strcpy(AC60, W(0x1A1C + 6*c));                  /* 9E32: the name */
    strcat(AC60, DS:1F89);                          /* " is assigned\nto command " */
    strcat(AC60, roman_6738(B(0x1922 + r)));        /* legion +0x0A, the numeral (buffer C072) */
    strcat(AC60, DS:1918 + r);                      /* the legion's name */
    msgbox_59D4(AC60);                              /* drawn over the battle screen with its report still on it */
}
```
Callers of 4733: 0x44AB (raise a legion, 4392: legions.c, ported), 0x50E0 (the garrison, legion 12, before an enemy army's attack on
Rome when its commander is not on duty: enemy.c, ported, V t9), and the battle E5AA (**not ported**). So the battle path covers every
caller of the battle thunk B06E: the revolt 0E1C, a legion moving into an army 4B85, enemy armies 5056 / 50FC (garrison: L = 12,
+0x0D = DS:1A15 = 2 from 4FFD, numeral 6738(0) = "", so "...to command Rome" as in t9), the rebel branch of 5F82 at 6006 and the
negotiation's war 64BB.

**What the port does instead** (battle.c result(), line 612): prints "the commander fell ... not ported" and sets g_enemy_unported.
Consequences: no dice roll (every later roll is shifted, so an End Turn comparison after such a battle fails), no message, and the
legion keeps the fallen commander: in every later battle of that legion B(CMDR+4) == 2, so the general gives no morale bonus
(battle.c 114) and no voice reach (708): orders reach nobody.

**Port (for whoever fixes it; slim):** in battle.c result(), replace the fprintf / g_enemy_unported lines by
```c
    if (B(CMDR + 4) == 2 && B(leg + 0xD) != 0xFF) {                 /* E58E..E5AA: 4733(L) */
        assign_commander(L);                                        /* the roll and the record writes */
        uint8_t c = B(leg + 0x10);                                  /* 47C6..480B: the message, built in AC60 like the original */
        char num[16]; roman_numeral(B(leg + 0xA), num); strcpy((char *)&g_ds[0xC072], num);
        strcpy((char *)&g_ds[0xAC60], dstr(W(0x1A1C + 6 * c)));
        strcat((char *)&g_ds[0xAC60], dstr(0x1F89)); strcat((char *)&g_ds[0xAC60], num); strcat((char *)&g_ds[0xAC60], dstr(leg));
        message(dstr(0xAC60));                                      /* 59D4: battle_message_hook (on screen) or stderr */
    }
```
`message()` is battle.c's 59D4 hook (battle_ui.c msg_hook in the game). Since the 2026-11-17 review the three copies are one:
battle.c commander_msg_4733(L, show), each caller passing its own way of showing it (message, game_message, legions_message).

**A 1990 quirk to put to Erik (G that it is unintended; not fixed by default, no game text contradicts it):** when no commander is
free, 4733 frees every commander whose status is not 0, the fallen (2) included, so the general who just died can be "assigned" to
his own legion in the same message. Reachable only after about 11 commanders have fallen (24 records, at most 13 on duty).
Put to Erik in HANDOFF.md "Needs Erik" (2026-11-17).

## 3. Gap 2 decoded: the keyboard-only pieces not yet written down (D)

**INT 9 hook 0x03FF** (installed by 03D3, removed by 03F1):
```c
isr09: B(0xA4) = BIOS(0040:0017);                        /* shift flags: 4 = Ctrl, 10h = Scroll Lock */
    sc = inb(0x60);
    if (sc == 0x1F && (B(0xA4) & 4)) { B(0x3EE9) ^= 0xFF; goto chain; }   /* Ctrl+S: music on / off */
    s = sc & 0x7F;
    if (W(0x9AB2) == 6) { if (s == 0x29) s = 0x48; if (s == 0x2B) s = 0x4B; }   /* Tandy: ` = Up, \ = Left */
    else if (s == 0x4E || s == 0x4A) goto chain;         /* keypad + and - are no directions (they are in Tandy mode) */
    if (s >= 0x47 && s <= 0x51) {                        /* keypad / arrows */
        bits = B(0xA6 + s - 0x47); B(0xA3) = bits;       /* DS:00A6 = 0A 08 09 04 02 80 01 01 06 04 05 */
        v = B(0xA2) | bits;
        if (sc & 0x80) { B(0xA5) = 0xFE; v ^= bits; }    /* break: clear the key's bits */
        B(0xA5)++;                                       /* make after a break: 0; typematic repeats: 1, 2, ... */
        B(0xA2) = v;                                     /* Up 8, Down 4, Left 2, Right 1, keypad 5 = 80h */
    }
chain: old_int9(); B(0xA4) = BIOS(0040:0017);
```
(GAMELOOP_NOTES section 2 says "1 px with Shift": it is Ctrl or Scroll Lock, as MENU_NOTES 4.1 has it.)

**0x3603 key / keyboard cursor**, the poll of 359B and of every wait loop:
```c
uint8_t key_3603(void) {
    int dy = 8, dx = 8;                                  /* di, si */
    uint8_t k = key_03AA();                              /* [bp-1] */
    if (W(0xBA) != 0) return k;                          /* 361D -> 36E4: with a mouse 3603 = 03AA (NEGO_LOGIC 10's "36E4 not decoded") */
    switch (k) {
    case '+': case 0x0D: W(0xB2) = 2; k = 0; break;      /* right button */
    case 0x10: case ' ': W(0xB2) = 1; k = 0; break;      /* left button (0x10 = Insert, 03AA) */
    }
    if (B(0xA2) && ticks_01AE() >= 2) {                  /* a direction held, 2 timer ticks since the last step */
        restart_00F8();
        if (B(0xA4) & 0x14) dx = dy = 1;                 /* Ctrl or Scroll Lock: fine steps */
        if (B(0xA2) & 8) dy = -dy;
        if (B(0xA2) & 2) dx = -dx;
        if (B(0xA2) & 0x0C) { y = SW(0xB8) + dy; if (y > SW(0xC2)) y = SW(0xC2); if (y < SW(0xBE)) y = SW(0xBE); W(0xB8) = y; }
        if (B(0xA2) & 0x03) { x = SW(0xB6) + dx; if (x > SW(0xC0)) x = SW(0xC0); if (x < SW(0xBC)) x = SW(0xBC); W(0xB6) = x; }
        cursor_move_1BE6(W(0xB6), W(0xB8));
        if (B(0xA5) == 0) B(0xA2) ^= B(0xA3);            /* a tap moves once; held (typematic) keeps moving */
    }
    return k;
}
```
**0x2983 Build Fleet buy poll, keyboard mode** (2900 calls it when [BA] == 0; items of record DS:11CA: 0 OK, 1 Cancel, 2 +, 3 -):
```c
uint16_t buy_poll_keys_2983(uint16_t rec) {
    uint16_t si = 0x100;                                 /* nothing yet */
    do {
        if (B(0xA2) && ticks_01AE() >= 4) {
            restart_00F8();
            if (B(0xA2) & 8) si = 2;                     /* Up = + */
            else if (B(0xA2) & 4) si = 3;                /* Down = - */
            if (B(0xA5) == 0) B(0xA2) ^= B(0xA3);
        }
        if (si == 0x100) {
            uint8_t k = key_03AA();
            if (k == '+' || k == 0x0D || k == 0x10 || k == ' ') si = 0;          /* OK */
            else if (k != 0) si = hotkey_5593(rec, k);                          /* hotkeys, Esc = FFh, else 100h */
        }
    } while (si == 0x100);
    return si;
}
```
Still to write for the whole mode (D in other notes): 5901 (MENU_NOTES 4.1), the bullet sprite [C1D6] and the 5x3 marker in 56B6 /
57F7 (MENU_NOTES 3.3 / 3.5), D33A's target cursor (BATTLE_UI_NOTES 9), [3E40] "Hit any key to continue" (LEGIONS_NOTES corrections 1), and
every `if ([BA])` branch the ported files skip (grep `0xBA` / "[BA]" in glad.c, race.c, nego.c, battle_ui.c).

## 4. Gap 3 decoded: section 1 B0E0, the command line (D; the start-up agent's lane, written here for gap 2)
```c
void cmdline_B0E0(char **argv, int argc) {               /* ret 4 */
    if (W(0x9AB2) == 6) B(0x3EEA) = 0x14;                /* Tandy: the Tandy sound */
    if (adlib_probe_055C()) B(0x3EEA) = 0x12;            /* port 388h answers: AdLib */
    for (i = 1; i < argc; i++) {                         /* case-sensitive strcmp A186 */
        if (!strcmp(argv[i], "k")) W(0xBA) = 0;          /* DS:6302: keyboard mode */
        for (j = 0; j < 5; j++) if (!strcmp(argv[i], DS:6304[j])) W(0x9AB2) = W(0x630E + 2*j);   /* x m e tandy v -> 0 2 4 6 8 */
        for (j = 0; j < 8; j++) if (!strcmp(argv[i], DS:6318[j])) B(0x3EEA) = B(0x6328 + j);
            /* nosnd FF, pc 11, tn 14, roland 13, mt 13, lapc 13, adlib 12, ad 12 */
    }
}
```
Static DS: [9AB2] = 0, [3EEA] = 11h; runtime (t10_in.bin): [9AB2] = 8 (VGA, set by the video detection), [3EEA] = 12h (AdLib found),
[BA] = FFFF. Which adapter x / m / e stand for is G (e = EGA, m = MCGA).

## 5. Gap 10 decoded: 0x1E21, the disk check before every overlay (D)
```c
void disk_check_1E21(void) {
    int ok = 0;
    do {
        if (find_first_9FA9(W(0x06AA) /* "ovl0.ovl" */, 0, buf) == 0) ok = -1;                 /* found */
        else if (W(0x0698) && W(0x0696)) { set_drive_0363(get_drive_0373() ^ 1);                 /* two floppies: other drive */
                                           if (find_first_9FA9(...) == 0) ok = -1; }
        if (!ok) {
            if (W(0x0698)) msgbox_59D4(DS:06AC "Please Insert Program Disk\nInto Drive A: or B:");   /* running from A: or B: */
            else { msgbox_59D4(DS:06DB "Exiting Program\nCannot Find Overlay File"); exit_0EE7(4); }
        }
    } while (!ok);
}
```
1E02: [C0E4] = current drive (0373); [0698] = FFFF when it is A: or B:; [0696] = FFFF when INT 11h reports 2+ floppy drives (037A).
In the port the overlays are compiled in: nothing to do (as the ported files say).

## 6. Gap 12 decoded: what 1990 does with the uninitialised locals (D, the stack reasoning; G where marked)

All three run inside End Turn's enemy turn 4C24, which calls 4F27(i) for every active army record 0..10 in a loop that calls nothing
else, so every 4F27 frame sits at the same stack address: **4F27's own locals hold what the previous 4F27 call of this End Turn left.**

- **4F27 at 4F6F, no candidate zone** (a mode-0 army whose neighbours are all at least as strong): `list` is empty, n = 0; 076B(0)
  still rolls the dice (the port does too) and returns 0; `si = list[0]` = [bp-0x14] = **the first candidate the previous army of
  this turn wrote** (any previous 4F27 that found n >= 1). The army then marches into that zone (not necessarily a neighbour) and
  fights there as usual. G for the first active army of a turn (the slot holds what earlier End Turn code left at that depth).
- **4F27 at 4FE5, a mode-0 army standing in Rome (zone 0)**: the army is switched off (+8 = 0), then `si = [bp-0xA]` = **the target
  the previous 4F27 call stored at 4FF6** (targets < 27 only; a sea target goes to 51C5 without storing). With that stale zone t:
  t = 1..26 -> 501E: for the first Roman legion in t, the 4BAC "...invades <t> ... meets the enemy" message, then the battle
  thunk, which returns "Rome won" at once because the copied active word [C2A0] is 0 (DF24; battle.c battle(), after counting the
  battle in C0F8); the loop stops there and 4F27 returns (+8 = 0). t = 0 -> 4FFD sets up the garrison record, then the same 501E
  loop for legions standing in Rome; the garrison battle 508D is skipped (+8 = 0). t >= 27 -> 51C5: the switched-off army's zone
  becomes that sea and the naval path runs for the Roman fleets there (531F, B0C2): not followed further (G what B12A does with it).
- **4E2D, no free rebel slot** (records 7..10 all active): `di = [bp-6]`, the same stack word as DX pushed by the dice 076B at
  4C95 (path 4C24 -> 4CAB) or at 4DD6 (path 4C24 -> 4C71 -> 4DB5 -> 4E0D): in both, **DX as the last 4F27 call returned it**
  (04FA and 076B keep DX; that last call is army 10's, since record 10 is active on this path). If that DX < 11 the rebel army is
  spawned into record DX, overwriting whatever army is there (a war army 0-6 included); else nothing happens. DX at 4F27's exits
  depends on the path (51D5 loop imul, 7330, the battle thunks): not decoded further.
The port stops / skips in all three (enemy.c 132 / 173 / 185) and logs it. These are 1990 bugs, so stopping is the port's default; the
enemy.c header should list them as fixes. `--original` cannot follow them exactly without the first-call stack contents; the two
4F27 cases could be modelled by keeping the previous call's target and list[0] (a static in army_move), if Erik wants it.

## 7. Stale comments and notes found (no change made)
- battle_ui.c lines 10 ("The menus (54C8) are still the port's approximation": the menus are 5614 / 54C8 on the DS records 73B6 /
  73C8, line 263), 16-17 and 576-581 ("Insert ... cannot reach the port"): Insert works (sdl2_platform.c 83-84, key(0x10) at 582).
- endturn.c 52: "not ported (the naval battle, enemy.c)": the naval battle is ported (naval.c); game_not_ported is reached only by
  headless runs without battle choices.
- enemy.c 12-14: the same; and "two places" with uninitialised locals are three (132, 173, 185).
- GAMELOOP_NOTES 2: "1 px with Shift" -> Ctrl or Scroll Lock. NEGO_LOGIC_NOTES 252-253 / 637: 3603's mouse-mode part 36E4 is
  `return key` (03AA's AL), nothing more.
- GAMELOOP_NOTES 4 and 7 (NOT PORTED for Legions, 5F82, Build Fleet, naval): historical; section 6 now holds the state.

## 8. Easter eggs, jokes and hidden texts met in this audit (all kept: they are data the port reads)
New (not in any doc before): the random-event texts of End Turn 1E98 (table DS:0F02, texts DS:0F48[i]; amounts checked against the
texts, no slip) are a string of jokes: Nero's fiddle found in a junk shop (DS:07B7, +5 score), a Vesuvius excursion on eruption day
that loses a guest (09B8), getting lost in the catacombs with a senator's daughter (0A46), a legation caught in the backwash of the
parted Red Sea (0AC6), a "Buy a Piece of the Trojan Horse" scheme that fails (0C18), Noah's ark found on Mt. Ararat (0CAD), and a
jeweled bird from Malta sold to an Egyptian trader (0DA1, the Maltese Falcon). Also Spartacus (0B46) and Cleopatra billing Rome for the
burned library of Alexandria (0BAD). Known before and met again: "Albertus Discreditus" and "Marcus the Meek" in the commander roll
of 4733 (BATTLE_NOTES 6), "OOPS" past numeral 999 (6738).
A text slip for Erik (not fixed, not decided): Build Fleet's purchase screen names the second ship type **"Quinquiremes"** (record
DS:11EE + 14 * type, name then cost: Triremes 5, Quinquiremes 10, Galleons 20), every other text spells it "Quinqueremes" /
"Quinquereme" (DS:10A4, DS:1314, DS:96DC). "Quinquireme" is a rare variant spelling, so this may be deliberate.
