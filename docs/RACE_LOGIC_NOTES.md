# Chariot race and victory sequence logic (OVL0 section 4): notes for the port

Source: `disasm/OVL_RACE.asm` (addresses are 0823 offsets; index `docs/RACE_INDEX.md`), EXE callers in
`disasm/EXE_RUNTIME.clean.asm`. Tables were read from the static DS (OVL0.OVL file 0xB400 + X). Nothing here has been run in
the original yet. Marks: **D** decoded from the asm or data, **V** verified (run, render, exact fit or a script that replays
the decoded rule), **G** guess. `rng(n)` = EXE 076B, 0..n-1. `add_cap(v, s, m)` = EXE 04FA = min(v + s, m), signed (D, read at
04FA: `add ax,[bp+6]; cmp ax,[bp+4]; jle`). Race tick = one pass of the D08A loop = 7 ticks of the 72.8 Hz timer (timer slot 0),
about 10.4 race ticks per second (D). `rec[i]` = the 18-byte racer record DS:C2BE + 18i; i = 0 Red, 1 Green, 2 Blue (opponents),
3 the player (D, colours DS:8818 and 87DE). `diff` = B(0x048F) = chariot race difficulty (0 Galley Slave .. 3 Emperor; it can be
changed in mid game from Game Control, GAMELOOP_NOTES).

Scratch scripts (data dumps, path replay, AI simulation):
`C:\Users\erikk\AppData\Local\Temp\claude\bfb5522be8ee045d\c4bf0f5f-57f9-4ea6-80b3-e1aa05bf0592\scratchpad\race\` (dump.py,
path.py, nodes.py, strs.py, scan.py, aisim.py).

Corrections to RACE_INDEX.md found while decoding (D): B84E steers **right** (x + 2, outward), B886 steers **left** (x - 2, inward,
x = 0 is the inside rail next to the spina); C34C/C34E are not "bumped-by" values but the front and rear probe results of D5E1
(racer index | 0x10 horses part / 0x20 chariot part); rec +0x0B is the per-tick **step** along the path (not "top/3"), +0x0C the
**skid**, +0x0D the action taken at the last animation boundary, +0x0E the pending command; C340/C342 are the player's lane x and
screen y (C334/C336 + 4*3); music_play (5D29) loads the song itself if needed (5D33 `call 5D73`), so playing song 2 after
D08A unloaded it is fine.

---------------------------------------------------------------------------------------------------------------------------

## 1. Entry and exit contracts (D)

### 1.1 The race: D08A (EXE thunk B0AA, from EXE 0x349D, port province.c rome_games3457)

In: the EXE has done the 1E21 disk check, `W(634A) >= 5` (Light chariot is always affordable) and `W(154E) != W(634C)`
(one race per turn). No arguments, no result (ax not set). Out: the screen is left for the EXE to redraw.

```c
void D08A(void) {                                   /* d08a */
    B(0xC350) = 0xFF;                               /* leader lap = -1 */
    B0E0();                                         /* 10 speckles: 30 rng calls, even if the player then cancels */
    if (!C2DE()) return;                            /* cancelled: chariot price and skulduggery stay paid */
    W(0x154E) = W(0x634C);                          /* d09e: this turn's race is used */
    W(C32E) = W(C36C) = W(C330) = W(C332) = W(C32C) = 0;
    music_play(3); timer0_reset(); frame_wait(18);
    while (!W(0xC32C)) {                            /* d0c7 */
        timer0_reset();
        DABA();  CECC();  D17A();  B190();          /* AI, input, curves, movement + draw */
        for (i = 0; i < 4; i++)                     /* d0de: horse recovery */
            if (W(0xC324 + 2*i) + 150 <= W(0xC322)) {
                B(0xC2CE + 18*i) = add_cap((int8)B(0xC2CE + 18*i), 1, B(0x87E6 + B(0xC2C6 + 18*i)));
                W(0xC324 + 2*i) = W(0xC322);
                if (i == 3) W(0xC330) = -1;
            }
        if (W(0x9B26) == 6) D3C2(0);
        while (ticks_since(0) < 7) ;                /* d12b */
        if (W(0x9B26) == 6) flip();
        D3C2(-1);                                   /* gauges, clear dirty flags */
    }
    BCF4(); free(W(0xC308)); if (W(0x9B26) == 6) 6837();
    music_stop(); music_unload(2); music_unload(3);
    C9B6();                                         /* result, money, score, odds, AI skill, province */
    C2C2();
}
```

### 1.2 The victory: DF58 (EXE thunk B092, from EXE 0x09A4)

In: main map loop, `B(5971) == 8` (rank Caesar: score 256 and every province Roman, GAMELOOP_NOTES), disk check 1E21 done,
`di = -1` already set by the caller (0x099E). Out: **the game ends**: DF58 returns to 0x09A7, `di != 0` -> 0x09AE `exit_game(0)`
(0x0EE7: music off, ISRs back, video mode back, DOS exit). No save is written, no game state is changed (D).
```c
void DF58(void) { music_load(0); cursor_hide_1D19(); DF74(); BDA4(); BF6A(1); C2C2(); credits_B0CE(); }
```
Section 1 (credits B9A1) overwrites B0E0..BA7F; DF58's own `ret` at DF73 lies outside that range and survives (index 11.7, D).

## 2. Every write to persistent game state (D)

Everything else the module writes (racer records C2BE.., C300.., AAB4..AAC5, C322..C36C, path C308, gauges, clip rects) is race
scratch, rebuilt by C2DE for each race. Saved = in the DS:5BDE save table (SAVE_FORMAT.md).

| DS | when | change | saved | read by |
|---|---|---|---|---|
| W(154E) | race started (C2DE returned -1) | = W(634C) | yes (154C..154F) | EXE 349D one race per turn |
| W(634A) treasury | C2DE chariot bought | -= 5 / 10 / 20 (87F2[type]) | yes | |
| | C730 skulduggery | -= 10 per bribe, 30 physician, 50 gods | | |
| | C2DE Race Now (c5bd) | -= bet (0..min(treasury,100), steps of 10 or the limit) | | |
| | C9B6 1st place | += (odds+1) * bet (old odds) | | |
| W(594A) score | C9B6 | score_add(10 / 5 / 1) for 1st / 2nd / 3rd; nothing for 4th or not finished | yes | rank 72DE at End Turn |
| B(634E) | C9B6 1st | add_cap(B(634E), 1, 100) | yes | negotiations, End Turn |
| B(87EE + type) odds | C9B6 | 1st: -1 if > 1; 3rd, 4th, not finished: +1 if < 5; 2nd: none | yes | bet menu, payout |
| W(C096):W(C098) | C9B6 1st | += (odds+1) * bet (gross, stake included) | yes | victory text "talents in bets" |
| W(C100), W(C102) | C2DE Race Now | ++ (games sponsored, races entered) | yes | victory text |
| W(C104) | C9B6 1st | ++ (races won) | yes | victory text |
| B(8BA6) AI skill base | C9B6, when the player finished | += 8 / 4 / 0 / -2 by place, then diff 0: -3, 1: +3, 2: +6, 3: 0; clamp 75..95 | yes | DA5A |
| B(527E + 32p) province unrest | C9B6, when an opponent ranks first | -= 10, min 1 (p = that opponent's home province) | yes (province records) | End Turn, negotiations |
| W(892E) | C2DE | = W(634C) (the field of this turn) | no | C2DE |
| B(8BA8..8BAB), B(8BAC..) AI steering command/timer | DABA | not reset per race: a timer left running carries into the next race (D, harmless) | no | DABA |
| animation nodes DS:808C..837F (+0x0D frame, +0x0E sub-tick) | BD0A | not reset per race: the animation phase (and so the command timing, 3.4) carries over | no | B460 |

Nothing writes legions, fleets, armies, province owners or the turn counter. The victory sequence writes none of the above.

## 3. Data and mechanics

### 3.1 Racer record rec[i] = DS:C2BE + 18i (D)

| off | i=0 | type | meaning |
|---|---|---|---|
| +0 | C2BE | w | position along the lap, 75 units per path point; -27600 (= 368*75) at each lap; start 0x4074 = 16500 (point 220) |
| +4 | C2C2 | w | running (-1); 0 = out of the race (never set back) |
| +6 | C2C4 | sb | laps: start = C350 = -1; +1 at each pass of point 0; 3 = finished |
| +7 | C2C5 | b | home province 0..26 (opponents rng(27); the player 0 = Italia) |
| +8 | C2C6 | b | chariot type 0 Light, 1 Medium, 2 Heavy |
| +9 | C2C7 | b | top speed: player 100, opponents 110 (C2DE c497); bribe -rng(10), physician +rng(10) |
| +A | C2C8 | b | speed (start top/3); always >= 20 after any change; capped at top at animation boundaries and by D590 |
| +B | C2C9 | b | step: path units moved per tick (D17A: = speed on straights, reduced in the curve, frozen in the curve approach) |
| +C | C2CA | sb | skid 0..15 (D17A); shown as the horses' x offset (node +8) |
| +D | C2CB | b | action taken at the last animation boundary; 7 written by B190 when finished |
| +E | C2CC | b | pending command (see 3.4) |
| +F | C2CD | b | chariot strength 10/15/20 (87FA[type]); 0 -> command 11 (wreck) |
| +10 | C2CE | b | horse strength 40/30/20 (87E6[type]); whip -1, horse bump -1; +1 per 150 ticks without whip/bump, up to 87E6[type] |
| +11 | C2CF | b | crash counter (B6CB) |

Per racer elsewhere: C30A/C30C + 4i path point previous/current; C31A + 2i finish tick (-1 none; C320 = the player); C324 + 2i
last whip/bump tick (horse recovery clock); C334/C336 + 4i lane x (0 inside .. 280 outside) / screen y (player always 100);
C368 + i strategy; AABE + i AI speed cap; AAC2 + i AI skill; 8BA8/8BAC + i AI steering command/timer.

Per type (DS:87E6.., 4 bytes per table, D): horses 40/30/20; bump weight 20/33/50 (% per die); odds 1/2/3 (saved, 1..5);
price 5/10/20; card sprite 28/24/20; chariot strength 10/15/20. The Light chariot has the strongest team and the weakest
body, the Heavy the reverse.

### 3.2 The track (D; V for the point count)

CBE6 rasterises the polyline DS:896A..8A89 into calloc(368, 4) points at C308. Replayed by `path.py`: exactly **368 points**
(no overflow, V). It is the mini-map outline: point 0 = (120,25) top left (finish line), 1..41 the left turn, 42..~203 the bottom
straight (y 39) left to right, the right turn, then the top straight right to left (y 25) with 16 pen-up pairs (-1,-1) at 214/215,
221/222, ... 366/367 that hide the mini-map dots (behind the spina monuments, G). The race runs counter-clockwise, as in a Roman
circus (D from the order of the points). Start: every racer at point 220 (260,25), on the top straight after the right turn;
the race = 148 points to the line, then 3 full laps = 1252 points (D).

Curve zones (D, D17A d1b3 and DABA db1a, same tests):
```
zone A (approach): 142 < p < 172  or  p > 338          ; skid builds, step frozen, AI caps its speed
zone B (curve):    0 < p < 42     or  172 < p < 213    ; step reduced, crash dice, outward drift, AI caps its speed
```
Points 0, 142 and 172 belong to neither zone (off-by-one gaps, 11.12).

Curve speed limit by lane (D17A d21d): `lane = |x| >> 4` (0..17), `limit = (lane + 15) * 4` = 60..128. In zone B
`step = speed - 8AA0[lane] * speed / limit` with 8AA0 = 10,15,20,25,30,35,40,45,50,55,55,60,65,70,75,75,75,75 (D): at the limit
the inside lane 0 advances 50 units per tick, lane 8 42, lane 15 45, so the inside is the fastest line (D, arithmetic).
The red needle on the speed gauge (D453) marks the limit of the player's lane (lane capped at 15): `x = (lane+15)*4*110/130 + 13` (D).

### 3.3 Bodies and probes (D5E1, D; the 817C test is an inclusive rectangle intersection, read at EXE 817C)

Each racer j has two rectangles relative to (x, y): A = horses (0,0)-(41,31) flag 0x10 (DS:8AE4), B = chariot (0,31)-(41,46)
flag 0x20 (DS:8AEC). Six probes relative to racer i (DS:8AB4), result bytes `j | flag`, 2 per probe:

| probe | rect | result | meaning |
|---|---|---|---|
| 0 | (42,0)-(45,31) | C344, C345 | right, beside the horses |
| 1 | (42,31)-(45,46) | C346, C347 | right, beside the chariot |
| 2 | (-4,31)-(-1,46) | C348, C349 | left, beside the chariot |
| 3 | (-4,0)-(-1,31) | C34A, C34B | left, beside the horses |
| 4 | (0,-2)-(41,-1) | C34C, C34D | in front |
| 5 | (0,47)-(41,48) | C34E, C34F | behind |

```c
void D5E1(int i) {                                  /* d5e1 */
    memset(&B(0xC344), 0, 12);
    for (k = 0; k < 6; k++) {
        n = 0;
        for (j = 0; j < 4; j++) if (j != i && W(rec[j]+4)) {
            if (hit(probe[k] + pos[i], A + pos[j])) B(0xC344 + 2*k + n++) = j | 0x10;
            if (hit(probe[k] + pos[i], B + pos[j])) B(0xC344 + 2*k + n++) = j | 0x20;   /* n is never limited: 11.4 */
        }
    }
}
```

### 3.4 Commands and the animation boundary (B460, D)

Commands in +0x0E: 0 none, 1 whip, 2 bump left (probe 3), 3 bump right (probe 0), 5 crashing, 7 out, 8 rein in, 9 steer left
(x - 2, min 0), 10 steer right (x + 2, max 280), 11 wreck. Steering (9/10) and reining (8, speed -5, min 20) act on **every**
tick while the command is pending; everything else acts only at an **animation boundary**: the racer's gallop animation
(node at *83CC[i]: 4 frames, 2 ticks each, nodes.py) is at frame 0, i.e. on 2 consecutive ticks of every 8 (D, the node table and
BD0A's counter logic; G that nothing else changes the cycle in normal running). At a boundary the speed is capped at top, the
command is performed, `+0x0D = +0x0E; +0x0E = 0`, and +0x0D 11 -> +0x0E 5, +0x0D 5 -> +0x0E 5 (a crash never ends).

```c
void B460(int i, int y_old) {                       /* b460 */
    c = B(rec[i]+0xE);
    if (c == 0 || c == 9 || c == 10) {
        if (c == 10 && !B84E(i) && B(0xC346)) B977(i, 1);     /* blocked on the right: bump the chariot there */
        if (c == 9  && !B886(i) && B(0xC348)) B977(i, 2);     /* blocked on the left */
        if (W(C336+4i) > y_old && B(0xC34E)) { W(C336+4i) = y_old; B977(i, 5); }               /* pushed back */
        if (W(C336+4i) < y_old && B(0xC34C)) { W(C336+4i) = y_old; B977(B(0xC34C) & 15, 5); } /* 11.5 */
    } else if (c == 8) { if (!B810(i)) B977(i, 5); }
    if (boundary(i)) {                              /* b51d */
        if (speed > top) speed = top;
        switch (c) {                                /* jump table B59D */
        case 0: case 8: case 9: case 10: BA28(i, 0); break;
        case 2: if (B(0xC34A)) { B8BC(i, 3); BA28(i, 2); } else c = 1; break;   /* nobody there: it becomes a whip */
        case 3: if (B(0xC344)) { B8BC(i, 0); BA28(i, 3); } else c = 1; break;
        case 11: switch to the crash animation; break;
        }
        if (c == 1 && !B(0xC34C)) {                 /* the whip */
            if (horses) { B7D2(i, 10 + rng(10)); horses--; if (i == 3) W(C330) = -1; }
            else          B7D2(i, rng(10));
            BA28(i, 1);  W(C324+2i) = W(C322);      /* the recovery clock restarts */
        }
        B(rec[i]+0xD) = c;  B(rec[i]+0xE) = 0;
        if (c == 11) B(rec[i]+0xE) = 5;  else if (c == 5) { runaway animation; B(rec[i]+0xE) = 5; }
    }
    BD0A(83CC[i], W(C334+4i), W(C336+4i));          /* draw, advance the animation */
}
B7D2(i, n): if (B(C34C)) return 0; speed = max(speed + n, 20); return -1;    /* no top cap here */
B810(i):    if (B(C34E)) return 0; speed = max(speed - 5, 20);  return -1;
B84E(i):    if (B(C344) || B(C346)) return 0; x = min(x + 2, 280); return -1;
B886(i):    if (B(C34A) || B(C348)) return 0; x = max(x - 2, 0);   return -1;
```

### 3.5 Bumps and damage (D)

```c
void B8BC(int i, int side) {                        /* the player's/AI's deliberate side bump; no dice weight */
    if (!running(i)) return;
    e = B(C344+2*side); e2 = B(C345+2*side); if ((e2 & 0x10) && (e & 0x20)) e = e2;   /* prefers the horses */
    j = e & 15;
    if (e & 0x20) D53E(j, 1);                                    /* chariot strength -1 */
    else if (e & 0x10) { speed[j] = max(speed[j] + rng(10), 20); W(C324+2j) = W(C322); D590(j, 1); }
}                                                                /* horses spooked: -1 strength, speed capped at top again */
void B977(int c, int side) {                        /* collision */
    if (!running(c)) return;
    e = B(C344+2*side); e2 = B(C345+2*side); if ((e2 & 0x20) && (e & 0x10)) e = e2;   /* prefers the chariot */
    B9C9(c, e & 15, e & 0x20);
}
void B9C9(int a, int b, int flag) {                 /* b9c9: two dice against the attacker's weight */
    n = (rng(100) < 87EA[type(a)]) + (rng(100) < 87EA[type(a)]);   /* 20/33/50 % each */
    if (n) { if (flag) D53E(b, n); else D590(b, 1); }
}
void D53E(int j, int n) { strength[j] = strength[j] > n ? strength[j] - n : 0; if (j == 3) W(C36C) = -1;
                          if (strength[j] <= 0 && B(rec[j]+0xD) != 11) B(rec[j]+0xE) = 11; }
void D590(int j, int n) { horses[j] = horses[j] > n ? horses[j] - n : 0; if (j == 3) W(C330) = -1;
                          speed[j] = min(speed[j], top[j]); }
```

### 3.6 Curves, skid, crash dice (D17A, all four racers, D)

```c
void D17A(void) {                                   /* d17a */
    for (i = 0; i < 4; i++) {
        if (!running(i) || B(rec[i]+0xD) == 11 || B(rec[i]+0xE) == 11) continue;
        p = W(C30C+4i);  zA = ...; zB = ...;        /* 3.2 */
        if (!zA && !zB) { skid = max(skid - 3, 0); node.dx = skid; step = speed; continue; }
        lane = |x| >> 4;  limit = (lane + 15) * 4;  /* byte; lane 17 gives 0x80, still right as unsigned */
        if (zB) step = speed - 8AA0[lane] * speed / limit;        /* zone A keeps the old step */
        d = (int8)(limit - speed);
        t = d > 0 ? 15 - min(d / 4, 15) : 15;
        skid moves toward t by at most 3, min 0;
        if (d < 0) { t = -d;  if (zB && rng(15) < t) B(rec[i]+0xE) = 11; }      /* overspeed in the curve: wreck */
        if (zB && t > 8) {
            D5E1(i);  e1 = B(C346); e2 = B(C347);                 /* right, beside the chariot */
            if (B(rec[i]+0xD) != 5) {
                if (e1 & 0x20) { B9C9(i, e1&15, -1); B9C9(e1&15, i, -1); }
                else if (e2 & 0x20) { B9C9(i, e2&15, -1); B9C9(e2&15, i, -1); }
            }
            if (!e1 && !e2 && lane < 14) x += t / 8 + 1;          /* drift outward */
        }
        node.dx = min(skid, 15);
    }
}
```
So: over the lane's limit by d in the curve proper, each tick wrecks with chance d/15 (certain at 15 over). Within 28 under the
limit, or over it by more than 8, the chariot drifts outward 2+ px per tick and grinds the chariot on its right (mutual chariot
damage). In the approach (zone A) nothing happens except the skid picture, and the step stays at the speed the racer had when it
left the straight (D; 11.13).

### 3.7 One race tick: movement, finish, crash removal, race end (B190, D)

```c
void B190(void) {                                   /* b190 */
    W(AAB4) = 0;  W(C322)++;
    view clear, old mini-map dots erased (EGA), C30A[i] = C30C[i];  B11C(-1);   /* speckles scroll (rng on respawn) */
    for (i = 0; i < 4; i++) {
        if (!running(i)) continue;
        D5E1(i);
        step = B(rec[i]+0xB);
        if (B(C34C)) step = min(step, B(rec[C34C&15]+0xB));       /* blocked in front */
        if (B(C34E)) step = max(step, B(rec[C34E&15]+0xB));       /* pushed from behind */
        W(rec[i]+0) += step;  W(C30C+4i) = W(rec[i]+0) / 75;
        if (W(C30C+4i) >= 368) {
            W(rec[i]+0) -= 27600; W(C30C+4i) -= 368;
            if ((int8)++B(rec[i]+6) > (int8)B(C350)) { W(C332) = -1;
                if (++B(C350) == 2) white line (120,22)-(120,28) on the mini-map; }   /* leader on the last lap */
        }
        if (B(rec[i]+6) == 3 && B(rec[i]+0xD) != 7) { B(rec[i]+0xD) = 7; W(C31A+2i) = W(C322); }   /* 11.1 */
        y_old = W(C336+4i);  W(AA76) = BAC5(i);                 /* screen y, visible */
        if (i != 3) { W(AAB4) |= W(AA76);
            if (!W(AA76) && B(C301) == 5) { B(rec[i]+0xE) = B(rec[i]+0xD) = 7; W(rec[i]+4) = 0; } }  /* player crashing: off-screen opponents leave */
        if (B(rec[i]+0xD) == 5) B6CB(i);
        if (running(i)) B460(i, y_old);
    }
    restore the view and the mini-map (104,22)-(275,39); n = racers running with laps != 3, their dots in 87DE colours;
    if (W(C2F8) && B(C301) != 7) {
        if (n == 1 && B(C301) != 5 && B(C301) != 11) { W(C32C) = -1; W(C320) = W(C322) + 1; }   /* only the player left */
    } else W(C32C) = -1;                            /* the player finished (7) or is out (crash scene done) */
}
```
The race ends as soon as **the player** finishes: opponents still on the track are not waited for (their finish ticks stay -1).
It also ends when everybody else has finished or left and the player is still racing (the player then gets tick+1, i.e. behind
all finishers but ahead of those who left).

BAC5(i) (screen y, D): `d = C316 - C30A[i]` wrapped to -184..184; target `y = 8d + 100`; if |d| > 24 or both old and new y are
beyond the same edge (170 / -22) jump, else clamp to -22..170 and move at most 4 px toward the target; visible = -22 <= y <= 170.

### 3.8 Crashes (B6CB per tick in state 5; D738 the player's crash scene, D)

```c
void B6CB(int i) {                                  /* b6cb */
    switch the animation to the runaway team;
    if (!W(AA76)) { B(rec[i]+0xE) = B(rec[i]+0xD) = 7; W(rec[i]+4) = 0; }   /* off screen: gone (not the player) */
    if (++B(rec[i]+0x11) < 24) {                    /* ~2.3 s of veering */
        if (B(0xC2FE) > 70) B810(i); else B7D2(i, 10 + rng(10));         /* C2FE = the PLAYER's speed: 11.6 */
        B(rec[i]+0xE) = rng(2) ? 10 : 9;  return;
    }
    if (i == 3) {
        if (!W(AAB4)) { all four running = 0; D738(); return; }          /* crash scene, then the race is over */
        if ((B(rec[i]+0x11) > 48 && top < speed) || speed > 70) B810(i);
    } else if (top / 3 < speed) B810(i);
    B(rec[i]+0xE) = 10;                             /* drift to the outside */
}
```
The player's crash: state 5 lasts until no opponent is on screen (B190 removes off-screen opponents meanwhile; DABA makes the
ones behind the leader rein in), then D738: music stops, one of two scenes by `rng(2)` (sequences DS:8B04/8B3A 28 frames or
8B20/8B72 26 frames, sheet "med"), 7 timer ticks per frame, Esc skips; then the race ends with C320 = -1: "You Failed To Finish".
A chariot in state 5 still moves: a player who crashes just before the line can cross it and finish normally (11.14).
Crash counter is a byte: after 256 ticks of waiting it wraps and the veering phase (with its dice) restarts (D).

### 3.9 Player input (CECC, every tick; D)

```c
void CECC(void) {                                   /* cecc */
    k = key_read(); key_flush();  si = 0;
    if (!W(C2F8) || B(C301) == 11 || B(C302) == 11) return;
    if (W(0xBA)) {                                  /* mouse */
        B(8A9E) = 0; mouse_poll(); dx = W(8A9A) - mx; dy = W(8A9C) - my;
        if (mx < 20 || mx > 300 || my < 20 || my > 180) { mouse_set_pos(160,100); W(8A9A) = 160; W(8A9C) = 100; }
        if (|dy| >= |dx|) { if (|dy| > 2) B(8A9E) |= dy < 0 ? 8 : 4; }          /* down 8 = rein; up 4 = nothing */
        else if (|dx| > 2) B(8A9E) |= dx < 0 ? 1 : 2;                           /* right 1, left 2 */
        si = both buttons ? 1 : right ? 3 : left ? 2 : 0;                       /* whip / bump right / bump left */
        W(8A9A) = mx; W(8A9C) = my;
    }
    if (!W(0xBA) && !k) B(8A9E) = 0;                /* without a mouse the bits stay while key events arrive */
    if (B(0xA2)) {                                  /* INT 9 arrow bits: 8 up, 4 down, 2 left, 1 right */
        if (B(0xA2) & 8) { B(8A9E) |= 4; if (B(C301) == 0) si = (A2 & 1) ? 3 : (A2 & 2) ? 2 : 1; }
        else { if (A2 & 4) B(8A9E) |= 8; if (A2 & 2) B(8A9E) |= 2; else if (A2 & 1) B(8A9E) |= 1; }
    }
    key_flush();
    if (B(8A9E) & 2) B(C302) = 9; else if (& 1) B(C302) = 10; else if (& 8) B(C302) = 8;
    if (si) B(C302) = si;
}
```
Keyboard: Up = whip (only when the last action was "none", so holding Up whips every other boundary), Up+Left / Up+Right
(keypad 7 / 9) = bump, Down = rein in, Left / Right = steer. Mouse: move left/right = steer, move down = rein in, both buttons =
whip, left button = bump left, right button = bump right; moving up does nothing (bit 4 unmapped, G whether intended).
A bump with nobody beside you becomes a whip (B460 case 2/3).

### 3.10 The opponents (DABA every tick for i = 0..2, D; DDBD/DE10 D)

```c
void DABA(void) {                                   /* daba */
    uint16_t lead;                                  /* [bp-12h] NOT initialised: 11.9 */
    for (j = 0; j < 4; j++) lead = max(lead, ((int8)laps[j] + 1) * 368 + W(C30A+4j));     /* unsigned */
    for (i = 0; i < 3; i++) {
        if (!running(i) || B(rec[i]+0xD) == 11 || B(rec[i]+0xD) == 5) continue;
        behind = progress(i) < lead;
        if (behind && B(C301) == 5) { B(rec[i]+0xE) = 8; continue; }    /* the player is crashing: rein in */
        lane = min(|x| >> 4, 15);  base = (lane + 14) * 4;               /* = the lane's limit - 4 */
        D5E1(i);
        if (!zA && !zB) {                           /* straights */
            if (B(8BAC+i)) { if (--B(8BAC+i) == 0) B(8BA8+i) = 0; B(rec[i]+0xE) = B(8BA8+i); continue; }
            if (DDBD(i) || DE10(i)) continue;       /* attack the player */
            if (B(C34C)) { steer(i, lane < 7 ? 10 : 9, 25); continue; }  /* blocked: pull out (inner lanes) or in */
            if (!behind && (uint8)(top + 10) <= speed) { r = rng(10); steer(i, r > 6 ? 10 : r > 2 ? 9 : 0, 25); continue; }  /* dead: 11.10 */
            if (lane > 10 && rng(2) == 0) { steer(i, 9, 25); continue; }
            B(rec[i]+0xE) = 1; B(8BA8+i) = 0; continue;                  /* whip, all the time */
        }
        r = rng(100) - B(AAC2+i);  a = |r|;         /* curves: a speed cap */
        switch (diff) {
        case 0: t = r >= 0 ? base : base + rng(a) - (strat 0: a/4, 1: a/2, 2: 3a/4); break;          /* 11.7, 11.8 */
        case 1: t = r >= 0 ? base : base + rng(a) - (strat 0: a/8, 1: a/4, 2: 3a/8); if (rng(3) == 0) B(rec[i]+0xE) = 9; break;
        case 2: t = base; if (rng(8) == 0) B(rec[i]+0xE) = 9; break;
        case 3: t = base; if (rng(4) == 0) B(rec[i]+0xE) = 9; break;
        }
        B(AABE+i) = (uint8)t;  if (B(AABE+i) < speed) speed = B(AABE+i);  speed = max(speed, 20);
    }
}
steer(i, c, n): B(rec[i]+0xE) = B(8BA8+i) = c; B(8BAC+i) = n;
DDBD(i): if ((C346 & 15) == 3) steer(i, 10, 25)            /* player beside our chariot on the right: close in */
         else if ((C344 & 15) == 3) { B(rec[i]+0xE) = 3; B(8BA8+i) = 0; B(8BAC+i) = 25; }   /* beside our horses: bump */
         else return 0;  return -1;
DE10(i): the same on the left: C348 -> steer 9, C34A -> command 2.
```
(`(e & 15) == 3` only matches the player: an empty slot gives 0.)

What the opponents do (D unless marked): on straights they whip on every boundary (top 110 against the player's 100), dodge a
chariot in front, drift in from the outer lanes, and attack only the player (never each other) by closing in and side-bumping.
In curves they never accelerate; each tick re-rolls a cap and keeps the lowest, so their cornering speed is the minimum of the
caps. Senator and Emperor drive exactly 4 below their lane's limit and cut inside (1 tick in 8 / 4); Galley Slave and Legionnaire
add random errors, scaled by "skill", always downward in effect. Strategy (Aggressive / Steady / Cautious, Steady twice as likely)
and skill matter only at Galley Slave and Legionnaire (D). By the replay of these rules (`aisim.py`, one opponent, lane 3, one
curve, 3000 runs, G for the simplifications): ticks through the curve with skill 75/95: Galley Slave Aggressive 103/110, Steady
138/177, Cautious 217/235; Legionnaire 93/95, 103/110, 117/132; Senator/Emperor 88. No opponent ever crashed from overspeed in a
curve (0 of 3000 in every case): their crashes come from bumps only.

### 3.11 Pre-race: field, purchase, skulduggery, bet (C2DE, C730, C88E, DA5A; D)

```c
int C2DE(void) {                                    /* c2de */
    W(C322) = 0;
    if (!BC15()) return 0;                          /* race/laps/med sheets */
    BDA4();                                         /* gallery graphics (failure tolerated: no parades) */
    if (!load circus.cbm) { BCF4(); C2C2(); return 0; }
    music_load(3); music_load(2);  draw circus;
    for (i = 0; i < 4; i++) { if (W(892E) != W(634C)) DA5A(i);  W(rec[i]+4) = -1;  W(C324+2i) = W(C322); }  /* DA5A also for the player */
    W(892E) = W(634C);
    cards CDCE(0..2);  B(C2FB) = 0;                 /* the player's home = Italia */
    for (bought = 0; !bought; ) {                   /* menu 88E6 "Race what type of Chariot?" */
        t = menu();
        if (t <= 2) { if (87F2[t] > W(634A)) msg(880A "You don't have enough Talents");
                      else { B(C2FC) = t; W(634A) -= 87F2[t]; bought = 1; card CDCE(3); message_box(DS:3E02 ""); } }
        else if (yes_no(8814 "Are You Sure\nYou Don't Want To Race?")) { C70F(); return 0; }
    }
    r = rng(4);
    for (i = 0; i < 4; i++) {                       /* c490 */
        BBAD(type, i);  top = (i == 3) ? 100 : 110;  horses = 87E6[type];  speed = step = top / 3;  skid = 0;
        strength = 87FA[type];  laps = B(C350) (= -1);  +0xD = +0xE = 0;  pos = 16500;  crash = 0;
        W(C31A+2i) = -1;  W(C334+4i) = ((r + i) & 3) * 70;  W(C336+4i) = 100;   /* lanes 0/70/140/210, random rotation */
    }
    W(AABC) = W(634A); B(AABA) = 0; W(C366) = W(AAB8) = W(AAB6) = 0;
    for (go = 0; !go; ) {                           /* menu 88F8 "How Shall We Prepare?" */
        W(AABC) = W(634A) - W(C366);  panel, treasury shown = AABC;
        switch (menu()) {
        case 0: go = (W(C366) || !W(AABC)) ? -1 : yes_no("You Have Not Bet\nRace Anyway?") ? -1 : 0; break;
        case 1: C730(); break;   case 2: C88E(); break;
        case 3: case 0xFF: if (yes_no(8814)) { C70F(); return 0; } break;
        }
    }
    W(634A) -= W(C366);  BE10();  race screen, gauges S/H/C, laps, mouse (160,100), CBE6();
    W(C102)++; W(C100)++;  return -1;
}
void DA5A(int i) {                                  /* da5a */
    B(rec[i]+7) = rng(27);  B(rec[i]+8) = rng(3);  s = rng(4); B(C368+i) = s == 3 ? 1 : s;
    B(AAC2+i) = rng(11) + B(8BA6) - 5;              /* 70..105 with the base clamped 75..95 */
}
```
C730 "What Skulduggery?" (costs are checked against treasury minus the current bet; c738 swaps W(634A) with AABC and back):
Bribe Opponent 10 (needs >= 10; after 3: "All Opponents Bribed"): the next driver in the order Red, Green, Blue (not chosen),
message "<Colour> driver has been paid\n10 talents to let you win.", **then** `top[AABA] -= rng(10)`. Hire Physician 30 (once):
message, then `top[3] += rng(10)`. Invoke Gods 50 (once): message, then `horses[3] += 10 + rng(10)`. Not enough money ->
"You don't have enough Talents" (checked before "You can only do that once"). Done/Esc leaves.
C88E "What Will You Bet?": shows "Your Bet: <n> Talents\nOdds: <odds[type]> to 1"; More = min(bet + 10, lim), Less =
max(bet - 10, 0), The Limit = lim, lim = min(treasury, 100) (treasury after skulduggery; the bet is paid only at Race Now).
Exit/cancel (C70F): sheets, pictures and songs freed; the chariot price and skulduggery stay paid, the turn's race is not used,
the same field comes back until the turn changes (892E), bribes/potions are lost (C2DE rebuilds the records).

BE10 (pre-race parade, D): song 2; one chariot drives in from x = 331 by 6 px per frame (7 timer ticks), then idles at x = 54..
with a wobble `rng(25) == 0` per frame **until a key or a click** (no time-out); a key during the drive-in skips the rest.

### 3.12 Result (C9B6, D)

```c
void C9B6(void) {                                   /* c9b6 */
    o[] = {0,1,2,3};
    for (a = 0; a < 3; a++) for (b = 3; b > a; b--) {          /* exchange sort, best first */
        A = o[a], Bb = o[b];
        if (W(C31A+2A) > W(C31A+2Bb)                              /* unsigned: -1 (not finished) is last */
            || (W(C31A+2A) == W(C31A+2Bb) && (W(rec[Bb]) > W(rec[A])        /* position within the lap only: 11.2 */
                || (W(rec[Bb]) == W(rec[A]) && rng(1) == 0))))              /* always swaps: 11.3 */
            swap(o[a], o[b]);
    }
    p = place of 3 in o;  t = B(C2FC);
    if (W(C320) == 0xFFFF) { message_box("You Failed To Finish"); if (odds[t] < 5) odds[t]++; }
    else {
        message_box(880C[p]);                       /* Win / Creditable Second / Poor Showing / Utter, Ignominious Failure */
        switch (p) {
        case 0: score_add(10); B(634E) = add_cap(B(634E), 1, 100);
                W(634A) += (odds[t] + 1) * W(C366);  C096:C098 += (odds[t] + 1) * W(C366);
                if (odds[t] > 1) odds[t]--;  W(C104)++;  BF6A(0);  break;
        case 1: score_add(5); break;
        case 2: score_add(1);                       /* falls into case 3 (no jmp at cb45) */
        case 3: if (odds[t] < 5) odds[t]++; break;
        }
        B(8BA6) += 8966[p] (+8, +4, 0, -2);  B(8BA6) += diff 0: -3, 1: +3, 2: +6, 3: 0;  clamp 75..95 (signed);
    }
    if (o[0] != 3) { pr = B(rec[o[0]]+7); B(527E + 32*pr) = max((int8)B(527E + 32*pr) - 10, 1); }
}
```
Texts (D): "You Win The Race!", "A Creditable Second", "A Poor Showing", "Utter, Ignominious Failure", "You Failed To Finish".
BF6A(0) after a win: song 2 (reloaded by music_play), the 4 chariots (Red, Green, Blue, then the player's, drawn as type 0 with
the driver's frames: G) drive past at 6 px per frame and 90 px apart, the player's stops at x 56, then 76 frames of the driver
animation (race2 frames 16..19 per the thresholds DS:8412 = 0,1,32,33,34) with a `rng(15) == 0` wobble on frames 1..34; a key
or click ends it at any point (D; sprite meanings G).

## 4. Dice (rng 076B) call sites in order (D)

Race entry, always: **B0E0** 10 speckles x (rng(77) x, rng(109) y, rng(65535) pattern) = 30 calls.
**C2DE**: if W(892E) != W(634C): DA5A for i = 0..3, each rng(27), rng(3), rng(4), rng(11) = 16 calls (the player's 4 are
thrown away); after the purchase **rng(4)** (lane rotation). C730, in menu order: each bribe rng(10) (after its text is built),
physician rng(10) (after its message box), gods rng(10) (after its message box). **BE10**: rng(25) per idle frame until a key.

Per race tick, in this order:
1. **DABA**, opponents 0, 1, 2 (skipped if out, wrecking or crashing, or reining in because the player crashes):
   straights: `rng(2)` only when lane > 10 and no timer/attack/blocked case applies (the rng(10) leader case is dead);
   curves: `rng(100)`, then `rng(a)` if diff <= 1 and r < 0, then `rng(3)` (diff 1), `rng(8)` (diff 2), `rng(4)` (diff 3).
2. **CECC**: none.
3. **D17A**, racers 0..3: `rng(15)` when over the limit in zone B; then, if a skid collision applies, B9C9 twice = 4 x rng(100).
4. **B190**: B11C: for each speckle that passes y 170, rng(77), rng(65535). Then racers 0..3: B6CB (state 5): veering phase
   `rng(10)` (only if the player's speed <= 70) then `rng(2)`; D738 `rng(2)`. B460: B977 -> B9C9 2 x rng(100) for each blocked
   steer / blocked y move / blocked rein (in that order); at a boundary B8BC `rng(10)` (horse bump), whip `rng(10)`.

After the race: **C9B6** one rng(1) per full tie met by the sort; **BF6A(0)** rng(15) per frame on frames 1..34 (fewer if a key
ends it). Victory: **BF6A(1)** 10 x rng(3) (parade chariot types), then rng(15) per frame on frames 1..34.

## 5. Difficulty (B(048F), D)

Used in two places only: DABA (opponents' cornering, 3.10) and C9B6 (drift of the skill base 8BA6). Prices, odds, payouts,
horses, top speeds, the player's crash rules and the bribes do not depend on it.

## 6. Timing (D)

Timer slot 0 only. Race tick: `timer0_reset; work; wait until ticks_since(0) >= 7` (D08A d12b) = 96 ms; start pause
frame_wait(18). Parades and crash scene: 7 ticks per frame. Horse recovery: 150 race ticks (14.4 s) since the racer's last whip
or horse bump. AI steering hold: 25 race ticks. Crash veering: 24 race ticks. Animation boundary: every 8 race ticks (2 ticks
long). Race length about 1252 points * 75 / step: at a step of 90, about 1040 ticks = 100 s (G, arithmetic).

## 7. The victory sequence (DF58 / DF74, D)

What it shows, in order: song 0 (cn1) loaded and the mouse cursor hidden; DF74 box 1 "THE MAKING OF A CONQUEROR" (the
statistics sentence, RACE_INDEX section 10, numbers from C082..C09C and C0F8..C106, all read-only), centred, "Hit any key to
continue" in white, wait; box 2 the epilogue DS:8BD8 ("The known world lies at your feet. ... and nothing is forever."), wait;
BDA4 loads the gallery graphics; BF6A(1) the triumph: song 0, ten chariots of random types (rng(3) each) drive past the gallery,
the tenth (drawn as type 0 with an extra figure, race2 frame 20, then frames 21..24: G the triumphator) stops and animates for
up to 76 frames; any key ends it; music is not stopped (mode 1 skips music_stop); C2C2 frees the graphics; the credits (section 1
B9A1); **then the program exits to DOS** (1.2). Statistic sentence conditions (D): alliances only if C106 != 0 (plural if != 1);
fleet part only if C0FC != 0, "won <n> fleet battles\nand " only if C0FE != 0, losses "one fleet battle" (no " fleet battles"
after it) if C0FC - C0FE == 1, "no" if 0; games part only if C100 != 0, "no" races if C102 == 0, "none" won if C104 == 0.

### 7.1 The credits (OVL0 section 1 B9A1, thunk B0CE; listing disasm/OVL_CREDITS.asm, port port/src/credits.c; D)
Section 1 = OVL0 file 0x0000-0x09A0 (PLINK86 table: load 1331, end 13CB, file position 0); the thunk bytes at EXE B0CE are
`9a ec 02 9b 12 01 00 ea a1 b9 23 08` (V, read from disasm/EXE_RUNTIME.bin). B9A1: the menu frame 30E6 at (84, 40) on the
screen (over the last triumph frame), then B9D3 for the 18 pairs DS:6A6E..6AB2 (title, names; 216 ticks = 3.0 s each), a key
ending the list; then always B9D3(6AB6, 360): the pair ("" DS:69FB, "\"Beware the ides of March\"" DS:69FC), 4.9 s or a key.
B9D3(pair, n): rect = 8C9F(names, minw 8C58(title), 0 lines, 159, 99) (8C58 pops the title, the names stay pushed as 8C9F's
first argument); the title by 8BE4 at (159, 47) in colour [9B69] = 1, the names at (159, y0) in colour 0; [9AFA] = [6346];
timer slot 1; until 0183(1) >= n, B8B7 (in VGA B8FE, then 03AA: -1 on a key; no mouse); then 7E74(x0, 47, x1, y1) in [6346]
erases the text; returns -1 on a key. B970 (the intro's credits) shows only the first 11 pairs, 288 ticks each.
The pairs (D, DS image): Game Design / Kellyn Beck, Evan Robinson, Nicky Robinson; Programming / Evan Robinson, Nicky Robinson,
Ken Zarifes; Graphic Artists / Jim Sachs, Scott Wallin, Dennis Clevenger, Michael Kosaka; Music Composition / Jon Newton,
Michael Bard; Music Programming / Rob Hubbard; Producers / Chris Wilson, Dave Albert; Associate Producer / Roland Kippenhan;
Assistant Producer / James R. Bailey; Tools / Soft-ICE & MagicCV by Nu-Mega, Plink-86plus by Polytron, MSC v5.1, v6.0 & MASM
v5.1 by Microsoft; Tools / Deluxe Paint II for IBM and Deluxe Paint II & III for Amiga by Dan Silva & Steve Shaw; Project
Director / Kellyn Beck; Technical Director / Jesse Taylor; Product Manager / Chris Thompson; Package Design & Illustration /
Lance Anderson, Robert Hunt; Documentation / David Luoto; Map Design / Kid Blast Graphics, Jennie Maruyama; Testers / Karyn
Taylor, Mike Meischeid, Jack Falk; Artist Photo / Bill Fiesterman; then the untitled quote.
**1990 bug (D effect, G intent):** B8FE cycles the five colours DS:6ABA into the DAC entries DS:6AD8 (A2..A6) at place
(counter + k) mod 4 (b93c `mov cx, 4; idiv`) while the counter DS:6AE6 runs 0..4 and B8CF sets all five entries: A6 never
cycles, colour 0 is overwritten by colour 4, and step 4 repeats step 0. credits.c fixes it (mod 5; --original keeps mod 4).
No visible effect on the end credits: no pixel of that screen uses A2..A6 (V, the port's frame), and CMAP A2..A6 are the
same five colours; it matters for the intro (B694, not ported), whose B8CF/B970 use the same cycle.
V (headless, caesar0.bin seed 7): the frame with "Game Design", "Programming", "Graphic Artists" (title in colour 1 = blue,
names black), a key skipping to the quote, the quote alone in the frame, the frame erased at the end.

## 8. Routine summaries not shown above (D)

- B0E0 / B11C / DA2E / DE64: track speckles (10 records DS:AA78, x 5..309, y 45..170, 16-bit pattern); B11C(s) scrolls by the
  player's speed (`acc += C2FE; dy = acc >> 4; acc &= 15`), respawns at y 45 with new x and pattern; DE64 plots per video mode.
- BA28 / BBAD / BC15 / BCA0 / BD0A: sprite-chain animation (variant 0 run, 1 whip, 2/3 bump left/right; crash and runaway trees).
- D392 lap counter: laps sheet frame max(C350, 0) at (134,185); D400 speed bar 110*speed/130, D453 the limit needle (3.2),
  D49A horses bar 110*horses/60, D4EC chariot bar 110*strength/20 (none drawn while the player crashes); D3C2 redraws the
  flagged ones. C32E is written 6 times and never read.
- CDCE racer card: box at DS:8A8A, home province name, strategy (opponents), type name, card sprite.
- C23D / C299 parade sprites; BDA4 / C2C2 / BCF4 / C70F loading and freeing.

## 9. Port notes

- Keep every value in g_ds (B()/W()/setW()) including the animation nodes 808C..837F and 8BA8/8BAC: they survive between races
  and decide command timing (2.); the result then matches DOSBox snapshots byte for byte.
- The race is deterministic given the rng stream and the per-tick input; for headless tests, script input per tick
  (CECC reads A2 bits, mouse buttons and motion).
- D738, BE10 and BF6A wait on keys/clicks: in --script mode feed a key or they idle (BE10 idles forever).
- The probe overflow (11.4) must never write past C351 in the port, even under --original (C352 is a sheet handle).

## 10. Jokes, easter eggs, hidden text (D for the texts; the readings are G)

1. The physician: "exotic herbs 'guaranteed' to make your horses win. You doubt that this will work but pay the 30 talents
   he asks." The code agrees with the doubt: top speed + rng(10), so 1 time in 10 it does nothing at all.
2. The Vestal Virgins: "a squinty-eyed attendant promises the gods' favor in return for a small donation of 50 talents."
   The dearest trick, and the only sure one (+10..19 horse strength), until the horses rest (11.7).
3. The bribe: "<Colour> driver has been paid\n10 talents to let you win." The bribed driver loses 0..9 top speed, so he is still
   at 101..110 against your 100: he takes the money and races anyway (11.15). You cannot even choose whom to bribe: Red,
   then Green, then Blue.
4. "Utter, Ignominious Failure" for fourth place; "A Poor Showing" for third (which also lengthens your odds like a loss).
5. Victory: "like those of Alexander ... Remember, though, that empires rise and fall like tides, and nothing is forever."
   followed at once by the exit to DOS.
6. The mini-map's dashed top straight (16 gaps) hides the racers' dots, probably behind the spina monuments (G).
7. MRACE member GALLERY2.CBM (same pixels as RACEEST) is never named by any code: unused (RACE_INDEX 9, D by byte search).
8. Outside this section, found on the way (RACE_INDEX 11.12): the archive names DS:63EE.. are stored XOR-encoded with "~enny"
   ("kenn" once): a hidden name, maybe Kenny (G).
9. The end credits (7.1, section 1) close with an untitled card: "Beware the ides of March" (DS:69FC, after an empty title
   DS:69FB), shown for 4.9 s even when a key skipped the rest. The credits also name the 1990 tools (Soft-ICE & MagicCV,
   Plink-86plus, MSC / MASM, Deluxe Paint) and an "Artist Photo" credit. Kept (credits.c).
No unreferenced strings exist in the race's DS string area 83EC..8EAD (scan.py, D): only the listed texts.

## 11. 1990 bugs and odd things

Proposed default fixes keep the dice count unless said otherwise; --original keeps the 1990 code.

1. **Finish ticks are re-stamped (D).** b315 tests `cmp byte ptr [bx+0xd],7` but B460 overwrites +0x0D at every animation
   boundary (b66c `mov [bx+0xc2cb],al`), so a finished racer's C31A is set again within 8 ticks, over and over until the race
   ends. The player's place is still right (the race ends the tick the player finishes), but the order of opponents who finished
   before the player is by animation phase, not by who finished first, so the wrong province can get the -10 unrest when an
   opponent wins. Fix: stamp only when `W(C31A+2i) == 0xFFFF`.
2. **Unfinished racers are ranked by position within the lap, laps ignored (D).** ca29..ca43 compare W(rec+0) only. It matters
   when nobody finished (the player crashed): a racer a lap behind can be "first" and its province gets the unrest relief.
   Fix: compare (laps, position).
3. **The tie-break never flips (D).** ca4d `push 1; call 0x76b` = rng(1) = 0: a full tie always swaps. Fix: rng(2) (same count).
4. **Probe results overflow (D code; how often G).** D5E1 d6e2/d70a `inc word ptr [bp-0x4]` has no limit, and three racers can
   give a probe up to 6 hits; the rect geometry allows 3 hits in one probe (e.g. two chariots behind, one at dy 16..17 hitting
   both its parts). Probe 0..4 hits spill into the next probe's slots (stale "blocked" flags); probe 5 hits spill into **C350**
   (leader lap: the lap counter and the last-lap mark freeze) and, at 4 hits, into C352 (the "med" sheet handle: crash scene and
   the BCF4 free at the race end use a corrupted pointer). Fix: keep at most 2 hits per probe.
5. **Ramming the chariot in front hits the wrong one (D code, intent G).** b504 `mov al,[0xc34c]; and ax,0xf; push ax` ... `push 5;
   call 0xb977` = B977(front, 5), which reads probe 5 of the moving racer (its rear), not the front chariot's: with nobody behind
   it becomes B9C9(front, 0, 0), i.e. **Red's horses** lose strength whoever rammed whom. The same author's B977(i, 5) at b4ee
   (pushed back: the racer behind is hit) suggests the intent "the rear chariot is hit by the front one". Fix: B9C9(front, i, 0)
   (2 rng(100) as before: the front racer is running, so B977 always rolled).
6. **Crashing opponents steer by the player's speed (D).** B6CB b72c `cmp byte ptr [0xc2fe],0x46` (C2FE = rec[3] speed) for any
   i. Fix: B(rec[i]+0x0A). The rng(10) of the speed-up branch then follows the opponent's own speed (the count cannot be kept).
7. **The gods' favour is taken back by the first rest (D code; intent supported by the gauge).** D08A d0fd
   `add_cap(horses, 1, 87E6[type])` = min(h + 1, max): 59 becomes 40 at the first recovery, 150 ticks after the last whip. The
   horses gauge D49A divides by 60 (`mov bx,0x3c; idiv`), i.e. it was built for 40 + 19. Fix: recover only when below the max.
8. **The opponents' "skill" works backwards (D code; intent G but well supported).** DABA dc3b..dc77 `r = rng(100) - skill;
   if (r >= 0) cap = base` : a skill-95 driver takes the clean line only 5 % of the time, and the random cap (size up to 95, always
   lower in effect) makes high skill **slower** (3.10 table). Every adjustment in C9B6 reads as "skill = % chance of a clean line":
   the base rises +8 when you win (rubber band), falls at Galley Slave (-3/race), rises at Legionnaire/Senator (+3/+6); the range
   75..95 is a percentage range. With the code as is they all work in reverse at Galley Slave and Legionnaire (Senator/Emperor
   ignore skill). Fix: clean line when `rng(100) < skill`, else the error with a = rng(100) - skill (1..25). Dice: rng(a) then
   comes on the misses instead of the hits (the count per tick changes with the branch; it cannot be kept).
   **Erik's call (open, 2026-11-12 review):** the port has the fix on by default (race.c bug 8, --original keeps 1990), but
   it changes the difficulty a lot (12.5: the two easy fields corner almost as well as Senator's), so it is listed with the
   BD0A counter under "Erik's call" in race.c's header and in HANDOFF "Needs Erik".
9. **Galley Slave Cautious cap wraps (D).** dcc8..dcaa: `base + rng(a) - 3a/4` stored as a byte goes below 0 for a > 70 and a low
   roll (lane 0: base 56, a 95, roll < 15), giving 238..255 = no cap that tick. Small effect (the next tick caps again).
   Fix: clamp at 20 (no dice change). Moot after fix 8 (a <= 25).
10. **Dead leader code (D).** dbfd `mov al,[di-5]; add al,0xa; cmp al,[di-4]; ja dc1f`: top + 10 <= speed never holds (speed <=
    top), so the leading opponent's random weaving (rng(10): 10/9/0) never happens. No fix proposed (would add dice).
11. **Uninitialised maximum in DABA (D).** `[bp-0x12]` is read before any write. By the stack it holds a large value left by the
    gauge drawing (0xD499 or a stack address) in normal ticks, so no opponent counts as leader, and the last mini-map dot's y
    (22..39) while the player crashes, so the true leader is found exactly when it matters (D by tracing the stack frames of
    D3C2/D400/D453/B190; G for the internals of 7E74/82DE). Fix: initialise to 0 (identical except on a lap-change tick).
12. **Curve zones have one-point gaps (D).** Points 142, 172 and 0 count as straight: for one tick mid-curve the step is the full
    speed and the AI whips; the next tick's cap undoes the whip before any crash roll. Negligible; no fix proposed.
13. **The approach freezes the step (D, odd, G intent).** In zone A D17A sets neither branch's step, so braking there changes the
    crash risk in zone B but not the progress until zone B. No fix proposed.
14. **A crashing chariot can still finish (D, G whether intended).** State 5 keeps moving; crossing the line sets +0x0D = 7 and
    ends the race with a normal place. No fix proposed.
15. **The bribe text promises more than the code gives (G, Erik's call).** "paid 10 talents to let you win" vs top 101..110 >
    the player's 100. A fix would need a design (e.g. the bribed driver reins in on the last lap); none proposed. Listed
    under "Erik's call" in race.c's header and in HANDOFF "Needs Erik" (2026-11-12): joke or slip, the code unchanged.
16. **Emperor has no skill drift (D code, G bug).** cb61 switch has no case 3. Skill is unused at Senator/Emperor, so it only
    matters after switching difficulty in mid game. If fix 8 is taken, Emperor +9 would continue the series (G).
17. **Third place lengthens the odds like a loss (D, G intent).** cb41 falls through into cb48. No fix proposed.
18. **Victory text singulars (D, text slip).** Only "alliance(s)" and "one fleet battle" have singular forms: "won 1 battles",
    "lost 1 battles", "won 1 fleet battles", "sponsored 1 games", "entered 1 races", "winning 1." occur. The game's own singulars
    show the intent. Fix: singular forms for 1. The same slip with the verb (2026-11-12 review, D: e088 / e0b5 / e1a6 / e1d3 /
    e2bd / e2ea / e317 strcat the plural strings unconditionally): "1 legionnaires were killed", "1 enemy soldiers were
    slain", "1 of your ships were sunk", "1 enemy ships were sunk", "1 talents in tribute / bets / plunder". race.c bug 12
    now covers them too (singular and "was" for 1; V by render of a save with every count at 1, --original keeps the
    plurals).
19. **"talents in bets" is gross (D, G slip).** C096:C098 gets (odds+1)*bet with the stake; losses are not subtracted. No fix.
20. **Keyboard direction bits stick (D, minor).** Without a mouse, 8A9E is cleared only on a tick with no key event, so with
    typematic repeat a released direction can linger while another key repeats (Left wins over Right). Port: keep.
21. **Empty message box after buying (D).** message_box(DS:3E02 = "") is a pause showing the cards; keep.

## 12. Found while porting (port/src/race.c, 2026-11-12)

Corrections to the sections above (D, read again at the bytes; V where a headless run showed it):

1. **BD0A's animation counter is inverted (D, bytes 38 44 0E 77 13 at bd6a).** `inc [si+0Eh]; cmp [si+0Eh], al(per);
   ja bd82` skips the reset when tick > per, so with per = 1 or 2 the tick goes 0 -> 1 -> reset and **every active node
   advances one frame per race tick**; the per byte does nothing (per 0 would mean one frame per 256 ticks). So the
   chariot node's 4-frame cycle is at frame 0 one tick in every 4, not "2 ticks of every 8" (3.4, SCREEN_NOTES 5.7):
   whips, lashes and the cap at top come every 4 race ticks. Kept in both modes (the game was balanced with it; Erik's
   call whether it is a slip).
2. **B8BC's horse lash indexes with the unmasked probe byte (D, b914..b949).** ax = e (j | 10h) goes into si = 18 * ax and
   di = ax, so the surge rng(10) is added to DS byte C3E8 + 18j and the rest clock C322 is written to C344 + 2j (probe
   slots, rebuilt before use); the victim never surges and its rest clock never restarts; only D590(e & 15, 1) works. 3.5's
   pseudo-code shows the intent. race.c bug 11 (fixed; --original writes those bytes).
3. **Finished racers lap on (D; V in --original runs).** +6 keeps counting past 3, and B190 counts racers still racing and
   draws their dots by laps != 3: a finished opponent at laps 4 counts as racing again, so the race can run until the
   player finishes (5806 race ticks at Galley Slave with the backwards skill), and C350 passes 3: D392 then draws laps
   frames 4, 5, ... past the 4-frame sheet. race.c bug 13 (fixed: laps < 3; frame at most 3).
4. The chariot node, not the horses, takes the skid as its x offset (d1f7 / d367: W(W(83CC[i])) + 8): the chariot and its
   driver slide sideways under the team (3.1 "the horses' x offset" corrected).
5. With bug 8 fixed (skill = % chance of a clean line) the Galley Slave / Legionnaire field corners almost as well as
   Senator's; with the 1990 code it corners far slower. In headless runs (race.c header) a player who stays in the
   outer lane (x 280, safe up to speed 128) and whips won at Galley Slave in 5 of 14 seeds (fixed; one 2nd) and 4 of 8
   (--original); in the others the opponents rammed the player's chariot to pieces in the first 400 ticks (an opponent
   beside the player closes in for 25 ticks, and each blocked step is two dice of its weight against the player's
   chariot). In the curves the opponents cap their speed and climb back slowly with tired horses.
