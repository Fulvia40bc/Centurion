# Gladiator show logic (OVL0 section 3): notes for the port

Source: `disasm/OVL_GLAD.asm` (addresses are 0823 offsets; index `docs/GLAD_INDEX.md`), the caller in
`disasm/EXE_RUNTIME.clean.asm` (Rome Games 0x3457). Tables were read from the static DS (OVL0.OVL file 0xB400 + X).
Marks: **D** decoded from the asm/data, **V** verified (a run, a render, an exact fit or a byte comparison), **G** guess.
Nothing in this module has been run against DOSBox yet, so every behaviour claim is D unless marked otherwise.
`rng(n)` = EXE 076B, 0..n-1 (NAVAL_LOGIC_NOTES). Ticks: 72.8 Hz; `since(n)` = 0183 ticks since timer slot n, `start(n)`
= 00D7, slot 0 also via 00F8 (reset) / 01AE (since). `score_add(n)` = EXE 7330: n >= 0: `score = min(score + n, 256)`
(04FA), n < 0: `score = max(score - |n|, 0)` (050F); score = W(594A), the number that drives rank (D, read at 0x7330).
`diff` = B(0x0490) = DS:048E[2] = the "Gladiatorial Combat" level of Fine Tuning (menu DS:05CA order Land Battle, Racing
Chariots, Gladiatorial Combat, Naval Battle; D from gamectl.c 0x19BC).

Fighter blocks (0x18 bytes each, D): **P** = the player's fighter at DS:AA52, **O** = the opponent at DS:AA6A.

| off | P | O | meaning |
|---|---|---|---|
| +0 w | AA52 | AA6A | move table pointer (4-byte states {dx (signed), next, frame, flags}) |
| +2 w | AA54 | AA6C | x (P starts at 192, O at 127; P stands on the right, faces left) |
| +4 w | AA56 | AA6E | y (175) |
| +6 b | AA58 | AA70 | state (index into the move table; FF = finished falling) |
| +7 b | AA59 | AA71 | state copy (P only, written at b98f, never read) |
| +8 b | AA5A | AA72 | training 0..3 (= the record's +8, bought this show) |
| +9 b | AA5B | AA73 | dying state (P {14 man, 12 wimp}, O {14, 12, 6, 6, 6} by body) |
| +A sb | AA5C | AA74 | health (signed byte) |

Between them (D): AA5E hits landed, AA64 blocks (both counted, never read), AA60 run of exchanges, AA68 run of dull spells,
AA62 crowd meter 0..30, AA66 crowd mood 0..2. AA50 = opponent record, AA51 = player record. All of AA50..AA75 are zero in
the static image and in the runtime snapshots t1/t10 (V): run-time only, not saved.

Move-table flags (D): bit 0 = idle (the fighter accepts a new command at a sample), bits 1-2 = attack zone (2 low, 4 high),
bits 3-4 >> 2 = guard zone (2 low, 4 high). The tables are in GLAD_INDEX section 9 (V, dumped again here). States by
meaning, man (wimp in brackets where different): 0 stand; 1 step in (-> 12 [11], dx -16 [-8] for P / +16 [+8] for O);
2 step back (dx +15 [+7] for P / -15 [-7] for O); 3 -> 10 (-> 11) high attack; 4 low attack (flag 02); 5 low guard
(flags 09, loops); 6 / 7 high guard (flags 11, loop; two pictures); 8 the kill (flags 01, loops; only used by the vote);
9 -> 13 -> 14 -> FF fall [9 -> 12 -> FF]; 15 [13] recover -> 0. Cats: 0 stand; 1 crouch -> 5 pounce (dx +20) -> 7 -> 0;
2 back off (dx -20) -> 7; 3 attack (flag 02, low) -> 7; 4 -> 6 -> FF fall; no guard states.

---------------------------------------------------------------------------------------------------------------------------

## 1. Entry and exit contract (D)

| entry | thunk | caller | contract |
|---|---|---|---|
| B0E0, `void(void)`, plain `ret` | B0B6 = `lcall 129B:02EC / dw 3 / jmp 0823:B0E0` (V by the index's table fit) | EXE 0x34F5 in Rome Games 0x3457 | no args, no result; the caller sets `si = -1` (34F8) whatever happened, so the Rome screen is always redrawn |

The caller (EXE 0x3457, D, quoted because the port's stub skips part of it):
```
34a7: cmp [0x154a],si(0) / jne 34e4            ; amphitheatre built?
34ad: cmp word [0x634a],0x32 / jae 34b9         ; no: 50 talents?  else box 161E "You need an amphitheatre\nand cannot afford one."
34b9: 55FF(15C2); 5C20(164E) "You need an amphitheatre.\nSpend 50 Talents?"  ; no -> 55EA(15C2), back to the menu
34cb: sub word [0x634a],0x32 / mov ax,-1 / mov [0x15d2],ax / mov [0x154a],ax / jmp 34f2   ; pay, build, straight to the show
34e4: mov ax,[0x634c] / cmp [0x154c],ax / jne 34f2   ; built: one show per turn, else box 167A "Only One Show Per Turn"
34f2: call 1e21 (disk check) / call b0b6 (the show) / mov si,-1
```
So buying the amphitheatre leads straight into a show in the same turn without the per-turn test (it cannot have been
used yet). **Port: when the show is ported, restore 0x34CB-0x34D6 (treasury -= 50, W(154A) = FFFF)**; W(154C) is written
by the module itself (section 2, b313), contrary to the comment at province.c lines 16-17.

Exit: the module returns to Rome Games with the archive closed, its sheets freed, songs 4..7 unloaded, song 1 (the map
music) unloaded too (b185) and not reloaded here, the music stopped, the cursor popped (1D8A), text colour W(9B69) = 15.

**Every DS write that a save or End Turn reads (D):**

| DS | written by | when | how |
|---|---|---|---|
| 634A treasury (save CSV 5FA) | B4DA b649 | each training purchase | `-= 10*(level+1)` (10..40), twice per show |
| 634A | B38B b45c | "cannot afford a second" | restored to the value at selection start (refund) |
| 594A score (CSV 5FC) | BDEA via 7330 | outcome | +3 win, +diff+3 upset, +3 both dead, +-5 thumbs (section 7) |
| 154C last show turn (CSV 601) | B0E0 b316 | after the fight, before the outcome | `= W(634C)`; a cancelled selection writes nothing |
| C100 games sponsored (CSV 6B6 block) | B0E0 b2f3 | before the fight | `++`; shown in the victory statistics "You sponsored N games" (section 4 DF74, RACE_INDEX) |
| 0ED4 "Rome is bored" chance (CSV 6C6) | B0E0 b2f7..b2fe | before the fight | `if (SB(0ED4) > 10) B(0ED4) = 10` (signed) |
| 0ED8 "Rome is bored" crisis (CSV 6C9) | B0E0 b303 | before the fight | `= 0`: ends the crisis (End Turn adds +15 unrest to Rome, province +0x0E, per turn while it is set: endturn.c 0x09CE) |
| 7E3E.. gladiator records 0..9 (CSV 594, 0x64 bytes) | B38B, B4DA, B4A3, BDEA, C0F9, B0E0 | | +2 generation, +3 wins, +4 losses, +5 status, +6 style, +7 body, +8 training (section 3) |

Run-time only (not saved, D): records 10..12 (the beasts, DS:7EA2-7EBF, outside the save block: their wins and generation
survive a game load and are lost on restart), AA50..AA75, B(800D) the AI pace accumulator (static, never reset), the
patched digit of DS:7B68 " (20 Talents)", the item text pointers at DS:7C80 + 12i (set to stack buffers by B4DA),
9AFA / 9B69 colours. **No other game state is touched**: no unrest or loyalty change for Rome (the show only ends the
bored crisis), no legion, province, fleet or rank write. Rank follows the score elsewhere (EXE).

## 2. B0E0, the show (D, literal)

```c
void B0E0(void) {
    si = 0F7B(6, &arch);                         /* archive 6 = MGLAD */
    if (!si) { 59D4(7BA1); return; }             /* "Can't Load Gladiator Graphics" */
    si &= 13A8(4, &arch);                        /* picture slot 4 = arenabtm.cbm */
    if (si & 1273(7B89 "coloseum.cbm", &arch, &desc)) { title picture full screen (fill 0, sprite, 86A0, copy) }
    C2BA = 163B("show"); C2B4 = 163B("arena");
    1D55(0); W(9B69) = 15;
    music_unload(1); music_load(4..7); music_play(4);
    timer0_reset();
    while (since(0) < 0x1B0 && !key_read() && W(B2) == 0) ;   /* 432 ticks = 5.9 s; B2 is NOT polled here (12.7) */
    music_play(7);
    if (B38B() == 0) {                           /* selection cancelled or unaffordable */
        music_unload(4..7); free C2BA, C2B4; 11A9(&arch); 1D8A(); music_stop(); return;
    }
    music_play(5);
    B67C();  8BE4(7BBF "VERSUS", 160, 125);
    B6EC(5, B(AA51), -1);  B6EC(3, B(AA50), 0);  /* player card bottom right, opponent bottom left */
    free C2BA;
    C2B6 = 163B(7BCC[body(AA51)]);  C2B8 = 163B(7BD0[body(AA50)]);   /* rman/rwimp; lman/lwimp/lcat0..2 */
    11A9(&arch);  music_play(4);
    59D4(7BC6 "");                               /* an empty box: only "Click to continue" - the pause on the VERSUS screen */
    arena: fill 0, arena frame 0 at (0,0), arenabtm at (0,126), copy;
    C288(B(AA51), B(AA50));  C1A4();  music_play(5);
    W(C100)++;
    if (SB(0ED4) > 10) B(0ED4) = 10;
    W(0ED8) = 0;
    B7FE(B(AA50));                               /* the fight */
    W(154C) = W(634C);                           /* b313: one show per turn */
    BDEA(B(AA51), B(AA50));                      /* outcome, vote, deaths */
    music_stop(); music_unload(4..7);
    59D4(7BC7 "");                               /* second empty box: the pause over the last scene */
    B(7E43 + 10*B(AA51)) = 0;  B(7E43 + 10*B(AA50)) = 0;   /* status free */
    free C2B4, C2B6, C2B8;  1D8A();
}
```
The two empty message boxes are real boxes holding only "Click to continue" (TEXTBOX_NOTES 59D4: `""` still shows a box),
i.e. "press to go on" pauses (D; intent G).

## 3. Selection

### Gladiator record R = 7E3E + 10r (D; r 0..9 men, 10..12 beasts)
| off | meaning | written by |
|---|---|---|
| +0 w | name pointer ("Flavius " ... "Snow Leopard ", trailing space for the numeral) | - |
| +2 b | generation (1 = no numeral; beasts 0, never shown) | C0F9 `++` |
| +3 b | bouts won (shown as a Roman numeral on the card when != 0) | BDEA `++`, C0F9 `= 0` |
| +4 b | bouts lost (never shown) | BDEA `++`, C0F9 `= 0` |
| +5 b | status: 0 free, 1 picked, 3 in the line-up | B38B, B4A3, B4DA, B0E0, C0F9 |
| +6 b | style 0..3 (AI behaviour), 4 = beast | C0F9 `= rng(4)` (men) |
| +7 b | body 0 man, 1 wimp, 2..4 cat sheets | C0F9 `= rng(2)` (men) |
| +8 b | training bought 0..3 (kept after the show) | B4DA |
| +9 b | unused | - |

The saves 0/1/2.CSV of a played game (C:\work\cent_play) hold histories consistent with this: e.g. 0.CSV Flaminius gen 2
with body 0 / style 0 (originally wimp, style 1: re-rolled on death), Spartacus II with 0 wins 1 loss (reset on death,
then one lost bout survived), Publius II with style 3; every status byte is 0 (no leak, 12.2) (V for consistency).

### B38B selection, returns -1 = go, 0 = cancelled (D)
```c
int B38B(void) {
    if (W(634A) < 20) { 59D4(7CC2 "You Cannot Afford\nTwo Gladiators."); return 0; }
    saved = W(634A);                                         /* [bp-8] */
    B67C();
    for (slot = 0; slot < 6; slot++) {                       /* [bp-6] */
        r = rng(10); si = r;                                 /* DICE: 6 x rng(10), always */
        do { if (B(7E43 + 10*si) == 0) { lineup[slot] = si; B(7E43 + 10*si) = 3; break; }
             si = (si + 1) % 10; } while (si != r);          /* none free: lineup[slot] NOT written (12.2) */
        B6EC(slot, si, -1);                                  /* card facing right */
    }
    59D4(7CE4 "Select a Gladiator\nfor You to Control");
    B(AA51) = B4DA(lineup);
    if (B(AA51) == FF) { B4A3(lineup); 1D19(); return 0; }   /* the picked record keeps status 1 (12.2) */
    1D19(); B67C(); for (i = 0; i < 6; i++) B6EC(i, lineup[i], 0);   /* redraw facing left: your card is now a beast */
    1CCE();
    if (W(634A) < 10) {
        59D4(7D0A "You Cannot Afford a Second Gladiator.\nYour Talents for the First are Refunded.");
        W(634A) = saved;                                     /* b459: the only refund */
    back_out:                                                /* b45f */
        B(7E43 + 10*B(AA51)) = 0; B4A3(lineup); return 0;
    }
    59D4(7D59 "Select a Gladiator\nTo Fight Against");
    B(AA50) = B4DA(lineup);
    if (B(AA50) == FF) { 1D19(); goto back_out; }            /* b48b: NO refund (12.1) */
    B4A3(lineup); 1D19(); return -1;
}
void B4A3(lineup) { for (i = 0; i < 6; i++) if (B(7E43 + 10*lineup[i]) == 3) B(7E43 + 10*lineup[i]) = 0; }
```

### B4DA pick(lineup) -> record or FF (D)
```c
uint8_t B4DA(uint8_t *lineup) {
    done = 0 /*[bp-2]*/; armed = 0 /*[bp-8]*/; sel = 0 /*[bp-0C]*/; paid = 0 /*[bp-0A]*/;
    1CCE();
    while (!done) {                                          /* b4f3 */
        while (!armed) {                                     /* b514 */
            if (359B()) { if (B(C07F) == 0x1B) { 1D19(); return FF; }  armed = -1; }   /* click or key */
        }
        for (i = 0; i < 6; i++)                              /* card i: x 7+106(i%3) .. +91, y 2+100(i/3) .. +91 */
            if (8238(rect_i, W(B8), W(B6))) {
                rec = lineup[i] /*[bp-5]*/;  B(7E43 + 10*rec) = 1;
                lineup[i] = 10 + rng(3);                     /* DICE: rng(3); a big cat takes the card's place */
                done = -1; break;
            }
        35DB();                                              /* wait for release: polls the mouse ONLY while a button is down */
    }                                                        /* armed is never cleared: a miss spins here (12.3) */
    1D19();
    for (k = 0; k < 4; k++) {                                /* 4 item texts in a stack buffer, 25 bytes each */
        strcpy(buf_k, style(rec) == 4 ? 7C76[k] : 7C6E[k]);  /* Fat/Well-Fed/Hungry/Ravenous; Trainee/Slave/Veteran/Master */
        B(W(7C7E) + 2) = '1' + k;  strcat(buf_k, " (k0 Talents)");   /* DS:7B68 patched */
        W(7C80 + 12k) = buf_k;
    }
    5614(7CB0, 1); 1CCE();                                   /* menu "How Much Training?" */
    for (;;) {
        sel = 54C8(7CB0, sel);
        if (sel == FF) return FF;                            /* rec keeps status 1 (12.2) */
        if (10*(sel+1) > W(634A)) { 585B(7CB0, sel, 0); 59D4(7D7D "You Cannot Afford Such\na Well-Trained Gladiator"); continue; }
        B(7E46 + 10*rec) = sel;  W(634A) -= 10*(sel+1);      /* b63d/b649 */
        return rec;
    }
}
```
Input (D): mouse click on a card, or with no mouse the EXE's keyboard cursor (3603: arrows move [B6]/[B8], Enter/+ click);
any key other than Esc acts as a click at the cursor; Esc cancels only before the first click. The training menu takes
hotkeys '1'..'4' (DS:7C80) and Esc. Costs 10/20/30/40 for both your man and the opponent; the player pays for both.
Beasts appear only after your own pick (in your card's slot), so a beast can only be the opponent (D).

B67C / B6EC only draw (card grid, name + generation numeral, "Bouts Won" + numeral, portrait; no DS state written, D).

## 4. Set-up C288(player, opp) (D)
```c
P.trn = B(7E46 + 10p);  O.trn = B(7E46 + 10o);  P.state = O.state = 0;
P.movetab = 7FE8[body(p)] {7ED8 man, 7F18 wimp};  O.movetab = 7FEC[body(o)] {7F50, 7F90, 7FC8 x3};
P.dying = 7ED0[body(p)] {14, 12};  O.dying = 7ED2[body(o)] {14, 12, 6, 6, 6};
P.x = 192; O.x = 127; P.y = O.y = 175;
```

## 5. The fight B7FE(opp) (D, literal)

```c
void B7FE(uint8_t opp) {
    Ptick = Otick = 6;  end = 0;  Pmoved = Omoved = 0;  Pcmd = Ocmd = 0;      /* [bp-0A] [bp-0C] [bp-4] [bp-10] [bp-0E] [bp-8] [bp-6] */
    W(AA68) = W(AA60) = W(AA5E) = W(AA64) = 0;  W(AA66) = 1;                /* mood 1 */
    P.health = O.health = 10;
    for (i = 0; i < P.trn; i++) P.health += rng(10);                         /* DICE */
    for (i = 0; i < O.trn; i++) O.health += rng(10);                         /* DICE */
    C25F(1); C25F(0); start(1);
    W(AA62) = O.trn + P.trn + 2;                                             /* crowd meter */
    3233(79, 10, 234, 20, W(6346)); C149(W(AA62));
    while (!end) {                                                           /* bb24 */
        key_read(); key_flush();                                             /* b8b9: typed keys are discarded */
        if (P.state == FF || O.state == FF) end = -1;                        /* b8bf */
        /* player: sampled every (4 - P.trn)*4 frames */
        if (Pcmd <= 0) {
            Pcmd = (4 - P.trn) * 4;
            if (P.movetab[P.state].flags & 1) {                              /* idle */
                if (P.health <= 0) P.state = 9;                              /* fall */
                else switch (B(A2)) {                                        /* arrows: Up 8, Down 4, Left 2, Right 1 */
                case 1:  if (P.x < 290) P.state = 2; break;                  /* Right: step back */
                case 2:  if (P.x > 30)  P.state = 1; break;                  /* Left: step in */
                case 4:  P.state = 5; break;                                 /* Down: low guard */
                case 6:  P.state = 4; break;                                 /* Down+Left: low attack */
                case 8:  if (P.state == 0) P.state = rng(2) ? 6 : 7; break;  /* Up: high guard; DICE rng(2) */
                case 10: P.state = 3; break;                                 /* Up+Left: high attack */
                default: P.state = 0;                                        /* none or another combination */
                }
                P.statecopy = P.state;  Pmoved = -1;
            }
        }
        Pcmd--; Ptick--;
        /* opponent: sampled every (4 - O.trn)*2 frames */
        if (Ocmd <= 0) {
            Ocmd = (4 - O.trn) * 2;
            if (O.movetab[O.state].flags & 1) { BC4C(opp); Omoved = -1; }
        }
        Ocmd--; Otick--;
        if (end) break;                                                      /* b9da: no frame on the last pass */
        timer0_reset();
        si = P.x; if (Pmoved) { si += P.movetab[P.state].dx; Pmoved = 0; }   /* a state's dx applies once, on entry */
        di = O.x; if (Omoved) { di += O.movetab[O.state].dx; Omoved = 0; }
        if (di < si) { P.x = si; O.x = di; }                                 /* no contact: both move */
        else if (rng(2) == 0) { if (O.x < si) P.x = si; }                    /* DICE: contact, a coin decides who may move */
        else                  { if (P.x > di) O.x = di; }
        C1A4();                                                              /* draw both */
        if (Ptick == 0) { BBC8(&P, &O); C25F(0); P.state = P.movetab[P.state].next; Pmoved = -1; Ptick = 6; }
        if (Otick == 0) { BBC8(&O, &P); C25F(1); O.state = O.movetab[O.state].next; Omoved = -1; Otick = 6; }
        while (since(0) < 4) ;                                               /* 4-tick frame = 18.2 frames/s */
        if (since(1) > 216) { BB3B(-1); start(1); }                          /* 3 s without an exchange: crowd -1 */
        if ((P.state == FF || O.state == FF) &&
            (P.state == 0 || P.state == FF) && (O.state == 0 || O.state == FF)) end = -1;
    }
    key_flush(); music_stop();
}
```
Timing facts (D):
- One animation step = 6 frames = 24 ticks = 0.33 s for both fighters, on a fixed clock per fighter (Ptick/Otick run from
  the fight's start and are not reset by a command). A command state therefore lasts only the rest of the current step; the
  attack test (BBC8) happens at the step's end with whatever state is current then.
- The player's input is sampled every 16 / 12 / 8 / 4 frames for training 0..3; if the fighter is not idle at the sample,
  the input is lost until the next sample. The key must be held at the sample ([A2] is the live arrow state).
- No key ends or pauses the fight; Esc does nothing.
- On the pass after a fighter reached FF (when the other was not idle), the player sample and the AI sample still run
  (and may roll dice) but no frame is drawn or moved. **Correction to GLAD_INDEX 10.5**: no frame is ever drawn or moved
  with state FF; what does read past the tables is that last pass's idle test `movetab[FF].flags` (movetab + 0x3FC),
  see 12.5.

### BBC8 exchange(attacker, defender) (D)
```c
z = A->movetab[A->state].flags & 6;                       /* attack zone; 0 = not attacking: return, nothing counted */
if (z && abs(D->x - A->x) < 20) {
    g = (D->movetab[D->state].flags & 0x18) >> 2;          /* guard zone */
    if (g != z) { W(AA5E)++; D->health -= A->trn + 1; }    /* hit: 1..4 */
    else W(AA64)++;                                        /* block */
    BB3B(+1); start(1);                                    /* an exchange either way: crowd +1, boredom timer reset */
}
```
Out of reach (>= 20 px) an attack is a plain miss with no crowd effect. A fighter with health <= 0 keeps fighting until his
next idle sample (then falls), so both can die; an attack on a falling or fallen man within reach still counts (D).

### BB3B crowd(delta) and C149 (D)
```c
W(AA62) = clamp(W(AA62) + delta, 0, 30); C149(W(AA62));          /* bar width 145*v/30 at (82,13) */
if (delta > 0) { W(AA68) = 0; if (++W(AA60) > 3) { W(AA60) = 0; W(AA66) = min(W(AA66)+1, 2); music_play(800A[W(AA66)]); } }
else           { W(AA60) = 0; if (++W(AA68) > 3) { W(AA68) = 0; W(AA66) = max(W(AA66)-1, 0); music_play(800A[W(AA66)]); } }
```
800A = {7, 5, 6}: mood 0 dull = song 7 (CN4D), 1 = song 5 (CN4B), 2 excited = song 6 (CN4C) (D). The song is re-played after
every 4th exchange even when the mood was already 2 (G whether 5D29 restarts a playing song).

### BC4C the opponent's AI (D)
```c
if (O.health <= 0) { O.state = style(opp) == 4 ? 4 : 9; return; }       /* fall; no dice, no pace */
B(800D) += O.trn + diff + 5;  if (B(800D) < 30) return;  B(800D) = 0;   /* PACE: the only use of difficulty */
close  = P.x - O.x <= 20;  medium = !close && P.x - O.x <= 36;
switch (style(opp)) {
case 0:  O.state = close ? (rng(2) ? 3 : 4) : 1; break;                 /* brawler: walks in, attacks high/low */
case 1:  O.state = medium ? 1 : close ? 7FF6[rng(10)] : 2; break;       /* counter-puncher: backs off when far */
case 2: case 3:                                                         /* (3 = Spartacus' original style, same code) */
         if (close) O.state = 8000[rng(10)];
         else if (medium) O.state = 1;
         else { r = rng(4); O.state = r < 2 ? 5 : r == 2 ? 6 : 7; }     /* waits in guard */
         break;
case 4:  if (O.health > 5) O.state = medium ? 1 : close ? 3 : 1;         /* beast: pounce, bite */
         else O.state = (close || O.x < 30) ? 3 : 2;                     /* wounded: bites when cornered, else backs off */
         break;
}
if (style(opp) != 4) {
    if (O.state == 1 && O.x > 290) O.state = 4;
    if (O.state == 2 && O.x < 30)  O.state = rng(2) ? 6 : 5;            /* DICE */
} else if ((O.state == 1 && O.x > 290) || (O.state == 2 && O.x < 30)) O.state = 3;
```
7FF6 = {6,6,7,7,5,5,4,3,2,2} (high guard 40%, low guard 20%, low attack 10%, high attack 10%, back 20%);
8000 = {6,7,5,4,3,1,2,5,4,3} (high guard 20%, low guard 20%, low attack 20%, high attack 20%, in 10%, back 10%) (V, dumped).
When the pace sum is short the AI leaves its state alone, so an AI guard (5/6/7, looping) is held until its next decision.

**Difficulty (D):** only the pace. Frames between AI decisions while the opponent stays idle, `ceil(30/(trn+diff+5)) *
(4-trn)*2` (x 4 ticks), and the player's sample period for comparison:

| O.trn | Galley Slave | Legionnaire | Senator | Emperor | player sample (same training) |
|---|---|---|---|---|---|
| 0 | 48 | 40 | 40 | 32 | 16 |
| 1 | 30 | 30 | 24 | 24 | 12 |
| 2 | 20 | 16 | 16 | 12 | 8 |
| 3 | 8 | 8 | 6 | 6 | 4 |

B(800D) is static and never reset (only at program start), so the first decision of a fight depends on the previous show's
remainder (D). Difficulty does not touch health, damage, prices, the crowd or the score.

**Strength (D):** health 10 + sum of `trn` x rng(10): trn 0 = 10, 1 = 10..19 (mean 14.5), 2 = 10..28 (19), 3 = 10..37
(23.5); damage per hit = attacker's trn + 1. An untrained man needs 10 hits to kill an untrained man; a Master (4 per hit)
kills him in 3.

## 6. Timer slots (D)
- slot 0: title wait (432 ticks), fight frames (4 ticks), the winner's walk in BDEA (4 ticks).
- slot 1: boredom (216 ticks without an exchange -> crowd -1), restarted by each exchange and each boredom step.

## 7. Outcome BDEA(player p, opponent o) (D, literal)

```c
pleased = W(AA62) > 2 * (O.trn + P.trn + 2);              /* [bp-10]: the crowd rose by more than its start value */
if (P.health > 0) { B(7E41+10p)++; Pwon = -1;
                    if (O.trn > P.trn) { score_add(O.trn - P.trn); upset = -1; }
                    score_add(3); }
else B(7E42+10p)++;
if (O.health > 0) { B(7E41+10o)++; Owon = -1;
                    if (P.trn > O.trn) { score_add(P.trn - O.trn); upset = -1; }
                    if (style(o) == 4) killP = -1; }       /* the beast won: your man dies, no vote */
else { B(7E42+10o)++; if (style(o) == 4) killO = -1; }    /* the beast lost: it dies, no vote */
if (upset) score_add(3);
if (!Pwon && !Owon) { score_add(3); killO = killP = -1; goto deaths; }   /* both dead */
if (killO || killP) goto deaths;
/* man against man, one fallen: the winner walks over to the loser */
do {
    timer0_reset();
    if (Pwon) { O.state = O.dying; if (idle(P)) P.state = 1; P.x += dx(P); }
    else      { P.state = P.dying; if (idle(O)) O.state = 1; O.x += dx(O); }
    C1A4(); while (since(0) < 4) ;
    done = O.x > P.x;                                      /* the winner has passed the body */
    P.state = next(P); O.state = next(O);
} while (!done);
sprite(arena frame 3, 79, 20); C149(W(AA62)); if (W(BA)) 1CCE();
59D4(800E "Thumbs Up or Down?");
up = down = 0;
while (!up && !down) {                                     /* mouse polled (0644) and keyboard [A2] */
    if (W(BA)) { 0644(); 1BE6(x, y);
                 if (W(B2)) { if (in(7EC0 (79,20)-(156,81))) up = -1; if (in(7EC8 (157,20)-(234,81))) down = -1; } }
    if (B(A2) == 8) up = -1; else if (B(A2) == 4) down = -1;    /* Up / Down arrow exactly */
}
key_flush();
if (down) {
    if (Pwon) { killO = -1; P.state = 8; O.state = O.dying; }   /* your man kills the loser */
    else if (Owon) { killP = -1; O.state = 8; P.state = P.dying; }   /* your own man is killed */
    C1A4();                                                 /* one frame, then B0E0's empty box holds it */
}
v = (down ? !pleased : pleased) ? +5 : -5;                 /* the crowd wants mercy after a good bout, blood after a dull one */
BB3B(v); score_add(v);
deaths:
if (killO) C0F9(o);
if (killP) C0F9(p);
```
If Up and Down are both seen in one poll, Down wins (tested first at c076) (D). Both alive cannot happen (the fight ends
only on a fall).

### C0F9 death(r): the successor (D)
```c
B(7E42+10r) = B(7E41+10r) = 0;  B(7E40+10r)++;                     /* "Flavius" -> "Flavius II" */
if (style(r) != 4) { B(7E45+10r) = rng(2); B(7E44+10r) = rng(4); }   /* DICE: new body (man/wimp), new style 0..3 */
B(7E43+10r) = 0;
```

### Score and texts per outcome (D)

| outcome | score | records | text |
|---|---|---|---|
| your man wins, opponent trained the same or less | +3 | wins[p]++, losses[o]++ | vote |
| your man beats a better-trained man | +3 + (O.trn - P.trn) + 3 | " | vote |
| your man loses to a less-trained man | (P.trn - O.trn) + 3 | losses[p]++, wins[o]++ | vote |
| your man loses to an equal or better one | 0 | " | vote |
| both fall | +3 | losses both, both die | none |
| beast beaten | +3 (+ upset bonus if it had more training) | beast dies (generation, wins reset) | none |
| beast wins | upset bonus if your man had more training | your man dies | none |
| vote | +5 if the crowd got what it wanted (Up when pleased, Down when not), else -5 | Down: the loser dies (C0F9) | "Thumbs Up or Down?" |

Score caps 0..256. Cheapest show: 20 talents (two Trainees), worth +3 + 5 with a won bout and the right vote. The only
texts of the module are the boxes listed in GLAD_INDEX section 9; there is no outcome text, the vote is the outcome
screen. **Section 3 has no victory sequence:** the rank-Caesar sequence is section 4 DF58 (thunk B092: song 0, DF74 "THE
MAKING OF A CONQUEROR" statistics incl. "You sponsored <C100> games", the triumph parade, credits; RACE_INDEX). This
module's only link to it is W(C100) (D).

## 8. Dice, in order (D) (rng call sites: B38B 1, B4DA 1, B7FE 4, BC4C 5, C0F9 2)

1. B38B: 6 x rng(10) (one per card slot, also when no free record is found).
2. B4DA (player's pick): rng(3) at the click (the beast for the card), before the training menu, also if then cancelled.
3. B4DA (opponent's pick): rng(3).
4. B7FE start: P.trn x rng(10), then O.trn x rng(10).
5. Per pass, in this order: player sample [rng(2) if Up from state 0]; AI sample [BC4C: style 0 close rng(2); style 1
   close rng(10); styles 2/3 close rng(10) or far rng(4); men stepping back below x 30: rng(2); beasts none; none when the
   pace sum < 30 or health <= 0]; frame body [rng(2) when the moves would touch or cross]. The last pass (end set at the
   loop top) still runs the two samples but not the frame body.
6. BDEA: C0F9(opponent) then C0F9(player) for each death of a man: rng(2), rng(4); beasts none.

Unlike the naval duel the loop is paced by the timer (one pass = one 4-tick frame, the AI acts only at frame samples), so
the dice are reproducible from the recorded [A2] states at the player's sample frames and a recorded click/key timeline
(G for DOSBox parity; the samples and frames are deterministic in the code).

## 9. Player input and its effect (D)
- Title: any key or the 432-tick timeout (a click works only if [B2] happens to be set: not polled, 12.7).
- Card grid: click (or key at the cursor; Esc = cancel before the first click). Training menu: 1..4, mouse, Esc.
- Fight: arrows only (live [A2] bits; keypad diagonals 7 = Up+Left 10, 1 = Down+Left 6 if the INT 9 hook maps them so: G),
  sampled per section 5. Left = step in, Right = step back, Down = low guard, Up = high guard, Down+Left = low attack,
  Up+Left = high attack; nothing = stand (a guard must be held). Up does not change a guard already held (only from 0);
  Right at the wall (x >= 290) and Left at x <= 30 keep the current state.
- Vote: Up/Down arrow or a click on the left/right half of the thumbs box.

## 10. Opponents
- The opponent is a gladiator the player picks from the same line-up (or the beast that took his own card), with
  training the player buys. Styles (D): 0 brawler (Marcus, Brutus, Julius), 1 counter-puncher (Flavius, Tillius,
  Flaminius), 2 guarded (Marius, Cassius, Publius), 3 Spartacus (= style 2 code), 4 beasts. Deaths re-roll style 0..3.
- A beast's only attack (state 3) has the low zone (flags 02) and cats have no guard: **the low guard (Down) blocks every
  beast attack** (D), and each block still counts as an exchange for the crowd.

## 11. Crowd model (D)
Start = P.trn + O.trn + 2; +1 per exchange (hit or block within 20 px); -1 per 3 s without one; clamp 0..30.
"Pleased" at the end = meter > twice its start. Mood 0..2 (start 1) only changes the music (4 exchanges up, 4 dull spells
down). The vote's +-5 also moves the meter and mood (cosmetic, the music is stopped right after).

---------------------------------------------------------------------------------------------------------------------------

## 12. 1990 bugs and quirks (fix by default, keep under --original, per house rule)

1. **No refund when the opponent is cancelled (D, real bug).** After the player paid his own man's training (b649),
   Esc on the grid or in the opponent's training menu goes `b48b: call 1d19 / jmp b45f` - status reset, B4A3, return 0 -
   past the refund at `b459: mov ax,[bp-8] / mov [0x634a],ax`. 10..40 talents are lost and, as 154C is not written, the
   show may be started again at once (paying again). Fix: `jmp b459` semantics (restore `saved`) on that path; no dice
   change.
2. **A picked gladiator leaks status 1 (D, real bug).** `b579: mov byte [di+0x7e43],1` is set at the click, before the
   training menu; the card's slot is overwritten by the beast (`b58a`), so B4A3 (which frees only status-3 records still in
   the line-up) never sees the record. Esc in the player's training menu (`b666`) leaves the player's record at 1; Esc in
   the opponent's menu leaves the opponent's record at 1 (b45f frees only the player's). Status 1 is saved (block
   7E3E) and the record is never offered again. After 5 leaks only 5 of 10 are free: the probe loop `b3c3..b3e4` falls
   through without `b3eb: mov [bp+di-0xe],al`, so the slot holds an uninitialised stack byte while the card shows a
   duplicate (record si = the start record); picking it writes `7E43 + 10*garbage` (up to DS:8839) and C288 reads a body
   and move table from out of the tables: DS corruption or a crash (D for the path; G for the crash). The played saves
   0/1/2.CSV have no leaked record (V). Fix: in B4DA, on FF from the menu, `B(7E43 + 10*rec) = 0` (the slot keeps its
   beast; harmless). Dice unchanged. Under --original keep the leak but fill a fall-through slot with si (the card shown)
   instead of stack garbage (documented divergence: the garbage cannot be reproduced).
   Port addition (review 2026-11-12, default only): B38B first sets every record 0..12 at status 1 or 3 back to 0, so a
   save made by 1990 or under --original with leaked records is repaired. Without it (V, headless run on a save with
   records 0..4 at status 1) the fall-through slot showed Cassius twice and the same record could be picked as player
   and opponent: he fought himself and a thumbs down on "the loser" killed the winner. No dice.
3. **A missed click on the card grid freezes the game (D, real bug, not reproduced).** `armed` ([bp-8]) is set by the first
   click or key (`b50f`) and never cleared. If the click (or a key, which uses the cursor position) is not on a card
   (gaps x 0-6/99-112/205-218/311-319, y 94-101/194-199, the bottom strip), `b591: call 35db` waits for the release
   (polling while the button is down), and the loop returns to `b4f3 -> b514 -> b51a` without waiting: the rect test runs
   on the last polled position, and 35DB (`35ee: cmp word [0xb2],0 / jne 35e0` - it polls only while a button is down)
   returns at once. Nothing polls the mouse or the keyboard any more: an endless loop (the timer ISR and music go on).
   Only a release position that happens to be on a card escapes. Fix (both modes; a hang has nothing to keep): on a miss
   clear `armed` so B4DA waits for the next click/key (Esc then works again). No dice involved.
   Worth one DOSBox check (click between two cards at "Select a Gladiator").
4. **The player man's high attack connects one step late (D asymmetry, G which table is wrong).** Move tables:
   P man `10 (0,11,3,00) 11 (0,15,4,04)`, O man `10 (0,11,3,04) 11 (0,15,4,00)`; both wimp tables have the hit on state
   10 (frame 3). Everything else in the P and O man tables is an exact mirror (dx negated), and RMAN/LMAN are mirrored
   sheets of identical frame sizes (V, rendered: `scratchpad/gladlogic/men.png`). Frame 3 is the full-extension thrust,
   frame 4 the sword raised after it (V by the render; reading of the pose G). So the player's man thrusts, holds the
   thrust for a whole step, and is tested only on the recovery frame: two full steps (12 frames) after the step in which
   the command was taken, against one step (6 frames) for every other fighter's high attack. Proposed default fix: P man state 10 flags 04, state 11 flags 00 (the O man / wimp pattern),
   --original keeps 1990. Dice count per frame unchanged, but fights then run differently. Erik's ruling welcome (G).
   **Port (2026-11-12): glad.c fix 7 took the OTHER table**, after GLAD_DATA_NOTES 2.4: O man state 10 flags 00, state 11
   flags 04 (the AI's high attack 6 frames slower; the hit on the raised-sword pose, as the wimps' state 10 = their
   raised-sword frame 3). Pose reading (V by render, `men.png` / `sheet_rman.png`): man frame 3 = arm level, sword
   horizontal; man frame 4 = sword angled up; wimp frame 3 = sword angled up (the wimp has no level pose). Either
   change makes the two men equal (1990: the AI's man 6 frames faster); which is right stays Erik's call (open, listed
   in glad.c's header).
5. **The last pass reads past the move tables (D, no visible effect).** After a fall ends at state FF, the last pass's
   idle tests read `movetab + 4*FF + 3`: P man 0, P wimp FF (-> `b987` sets P.state 9 again), O man 0, O wimp 3B (-> BC4C,
   health <= 0 -> O.state 9), cat 0; and in the pass where P reaches FF, an O attack reads P's guard from `movetab + 0x3FF`
   (P wimp: guard 6, never equal to 2/4 -> a hit). The bytes are the same in the static DS and the runtime snapshots
   t1_in / t10_in (V). Effects: none on the outcome (BDEA resets the loser's state; the both-dead case draws nothing more);
   no dice. Port: do not index past the tables; return the 1990 bytes for FF (P wimp flags FF, O wimp 3B) or flags 0 -
   identical results either way (D). (Supersedes GLAD_INDEX 10.5.)
6. **An upset pays both ways (G: intended).** `be8f..bea6` gives `P.trn - O.trn` (+3) when the player's BETTER-trained man
   LOSES - more than an even win. Reads as "the crowd loves an upset", symmetric by construction. Keep.
7. **Title skip by mouse uses a stale [B2] (D, quirk).** `b1bb: cmp word [0xb2],0` without 0644 in the loop: a click does
   not skip the 5.9 s title (a key does); a button state left set by the previous screen skips it at once. Keep; the port
   may poll (cosmetic).
8. **Beasts cannot be blocked high and are fully blocked low (D, design).** See 10. Keep.
9. **AI pace carries over (D, quirk).** B(800D) is never reset; keep it in the DS (not saved, zero at program start).
10. **Beast records are not saved (D, quirk).** 7EA2-7EBF lie outside the save block (7E3E + 0x64): a beast's wins and
    generation carry across a game load within a session and reset on restart. Keep literal.
11. **Unreachable overflows (D):** wins/losses/generation are bytes (256 bouts or deaths of one name wrap; a wrapped
    generation 0 would print roman_numeral(0)); treasury compares are unsigned (`jae`/`ja`). Ignore.
12. Quirks to keep literally (D): the Up key changes only from stand (state 0); a held guard is dropped by any sample with
    no key; the contact coin (rng(2)) every frame two moves would touch; attacks on a falling man still count; Spartacus'
    style 3 runs the style 2 code; the music is re-played at every 4th exchange at full mood.

## 13. For the implementer
- Port as a headless-scriptable loop like battle_play/naval: one pass per 4-tick frame, timer slots 0 and 1 kept in the DS
  clock; the title (432 ticks), the two empty "Click to continue" boxes, the vote box and the card/menu waits take script
  input. Script keys: arrows set [A2] (Up 8, Down 4, Left 2, Right 1, combinations ORed).
- Keep B(800D) and the beast records in g_ds; write W(154C) at b313 order (before BDEA); restore the amphitheatre writes
  in province.c rome_games3457 (0x34CB..0x34D6) and drop the "neither module writes 154C" comment for the show.
- Sprites: "show" (cards), "arena" (frame 0 background, 1 card frame piece, 2 name plate, 3 thumbs box), fighter sheets
  rman/rwimp (P), lman/lwimp/lcat0..2 (O): same HDR/DAT format as battle_gfx load_sheet (offset - 2) (V, rendered).
  arenabtm.cbm at (0,126); C1A4 draws the fallen one first and copies rows 126..185.
- Health bars C302: width = health*128/50, red from the left for O (side 0) and from the right for P.

## 14. Easter eggs, jokes, hidden text (report to Erik, keep all)
- The roster puts **Julius** beside **Brutus, Cassius and Tillius** (Caesar's assassins, Tillius Cimber gave the signal):
  you can stage Julius against Brutus. Plus **Spartacus**, the only gladiator with his own style number (3) - which runs
  exactly the code of style 2 (an unfinished idea or a joke: G).
- Fighter bodies are literally named **"man" and "wimp"** (RMAN/RWIMP/LMAN/LWIMP); Spartacus starts as a wimp, as do
  Flavius, Tillius, Flaminius and Publius.
- Training grades **"Trainee", "Slave", "Veteran", "Master"** (Slave ranks above Trainee); for beasts **"Fat",
  "Well-Fed", "Hungry", "Ravenous"** - the hungrier the cat, the dearer (10..40 talents).
- **"Black Panther", "Wild Lioness", "Snow Leopard"**: a big cat literally takes the place of the card you pick, so the
  beasts can only ever be your opponent; and Down (low guard) makes your man immune to them.
- Dead gladiators return as namesakes: **"Flavius II", "Brutus III"** (wins and losses reset, new body and style);
  roman_numeral prints "OOPS" past 999 (EXE, unreachable).
- The thumbs vote: the crowd wants **mercy after a good bout and blood after a dull one**; you can also turn thumbs down
  on your own beaten man, and then he is killed.
- Hidden statistics: bouts lost (+4) are counted for every gladiator but never shown; hits landed and blocks
  (AA5E/AA64) are counted in every fight and never read (debug leftovers, G).
- Two **empty message boxes** (DS:7BC6/7BC7, "" -> only "Click to continue") serve as pauses.
- "coloseum.cbm" (one s), file name only; record byte +9 unused.
- Unlike the provincial games (0x3401), the show does not lower Rome's unrest: it only ends "The people of Rome are
  bored.\nThe citizens demand a gladiator show." and caps that crisis' chance at 10%.

## 15. Open questions
- 12.3 (the freeze) and 12.4 (which man table is right) want one DOSBox look each.
- Whether 5D29 restarts a song that is already playing (11, music only).
- Whether the INT 9 hook maps the keypad diagonals onto [A2] 6/10 (keyboard notes).
