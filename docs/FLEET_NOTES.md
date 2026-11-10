# Fleet orders (EXE) and how the naval battle's results are used

Scope: the map's fleet click and key F, 0x2C6F select_fleet, 0x2F5A fleet_info, 0x2B40 fleet_move, 0x4ADA (armies at
sea), 0x531F (an army made into fleet record 6), 0x5244 (fleet retreat), End Turn's naval path 0x51C5..0x523C (enemy.c),
0x2FD6 fleet list, 0x2AF8 fleet icons, boarding/disembarking (legion move 0x382A), and the fleet-battle part of section 5
(DA50/DD2E/DE51) for Erik's report. Marks: **D** decoded from the asm, **V** verified by running or comparing, **G** guess.
Listings: EXE from `disasm/EXE_RUNTIME.bin` with llvm-objdump (same method as NAVAL_INDEX section 1); section 5 from
`disasm/OVL_NAVAL.asm`. "push order" below is the asm's; the callee sees the LAST pushed word at [bp+4].

## 0. Status in the port

| piece | EXE | port today |
|---|---|---|
| map: sea-zone click, "last fleet record wins", enemy → zone_info, right button → legions | 0x0889..0x08ED | **done** (map.c; 0x08C9 calls fleet.c: 2C6F, 3F23, 2F5A, 2B40) |
| key F = fleet list | 0x2FD6 | **done and matches** (province.c fleet_list, D: compared line by line) |
| fleets_in 0x2D60, fleet_row 0x2E96 | | **done** (province.c, `static`: make them visible to the new code) |
| fleet icons | 0x2AF8 | **done and matches** (map.c fleet_icons, D) |
| select_fleet + table 0x2DB2 | 0x2C6F | **ported** (fleet.c, 2026-11-09; headless script runs, V for the DS results) |
| fleet_info | 0x2F5A | **ported** (fleet.c) |
| fleet_move | 0x2B40 | **ported** (fleet.c; map.c's stub replaced) |
| armies at sea on a fleet move | 0x4ADA | **ported** (fleet.c, bug 9.3 fixed by default) |
| army → fleet record 6 | 0x531F | **ported** (fleet.c, bug 9.2: 1 ship) |
| fleet retreat | 0x5244 | **ported** (fleet.c, bug 9.1 fixed by default) |
| End Turn naval path | 0x51C5..0x523C | **ported** (enemy.c calls fleet.c and naval.c) |
| boarding 0x3AE4 / disembarking (legion move 0x382A) | | **done and matches** (legions.c, D: compared) |
| naval battle B0E0 / B12A (section 5) | | **ported** (naval.c + naval_gfx.c, the whole section; headless via --script with "hold KEY N" / "wait N" lines; without a screen, --end-turn, B12A still returns 0) |

## 1. Records (D)

Fleet record k at F = DS:110E + 12k (k 0..2 Roman, 3..5 enemy, **6 = "Marauding Fleet", the army-at-sea record built by
0x531F**; the save block covers 110E + 72 = records 0..5, record 6 is scratch):

| offset | DS (k=0) | meaning |
|---|---|---|
| +0,+2,+4 | 110E,1110,1112 | ship counts, words, by type 0 trireme / 1 quinquereme / 2 galleon |
| +6 | 1114 | zone (27..36 = seas) |
| +7 | 1115 | home province (Build Fleet writes the province z; B12A reads province +0x0A of it as the crew's skill when no legion is aboard) |
| +8 | 1116 | status: 0 Ready, 1 Moved, 2 Forming (Build Fleet), 0xFF no fleet; texts W(1B54 + 2*status) = "Ready" DS:17C5, "Moved" DS:17CB, "Forming" DS:17D1. End Turn 0x0A8A sets every Roman one != FF to 0 |
| +9 | 1117 | legion aboard (0xFF none). Enemy fleets 3..5: army record i+8 (11..13) = their marines (B397); record 6: the army index |
| +0x0A | 1118 | **men per ship** (D: B397 f[4] = strength / ships; 3AE4 board = (cav*4 + inf) / ships; 531F = total / ships; DD2E/DE51 multiply ships lost by it). enemy.c's header calls it "ships shown": wrong |
| +0x0B | 1119 | unused here |

Names W(1100 + 2k): "Fleet I", "Fleet II", "Fleet III", "Carthaginian Fleet", "Aegyptian Fleet", "Macedonian Fleet",
"Marauding Fleet" (k = 6, W(110C)). Static DS: fleets 3/4/5 = 50 quinqueremes in zone 30 / 100 galleons in 27 / 30
quinqueremes in 28; each enemy fleet has ONE ship type (DS:3D90 = {1,2,1}, max DS:3D96 = {50,100,30}; 0x4CF2 and 0x541F
only ever write that type) (D). Per-type tables: DS:8EDE capacity {50,100,200} men per ship, DS:8EE2 flagship hull
{30,45,60}, DS:9722 names "Trireme"/"Quinquereme"/"Galleon" (flagship menu DS:9750). Army record A = DS:C112 + 14i as in
enemy.c (A+1 zone, A+4/A+6 strengths, A+8 active, A+0x0A a byte strength).

## 2. Map loop, sea zone (0x0889..0x08ED) (D; map.c already has it)

```c
f = 0xFF; for (k = 0, si = 0x1114; si < 0x115C; si += 12, k++)                   /* records 0..5 only */
    if (B(si) == z && B(si+2) != 0xFF) f = k;                                     /* the LAST one wins */
if (f == 0xFF) { zone_info(z); goto after; }                                      /* 0x087F */
if (btn != 1) goto right;                                                         /* 0x08EF: right = Select Legion 3BAF(z, 0) */
if (f >= 3) { zone_info(z); goto after; }                                         /* an enemy fleet listed last */
f = select_fleet_2C6F(z);  close_box_3F23();                                      /* 0x08C9, 0x08D3 */
if (f < 3) { fleet_info_2F5A(f); fleet_move_2B40(f); }                            /* 0x08E3, 0x08EA */
```
Key 'F' (any case, via 9DDC) = fleet_list 0x2FD6. The map stub at map.c:374 is the only missing call.

## 3. Selecting a fleet (D)

```c
uint8_t fleets_in_2D60(uint8_t z) {                    /* ported as province.c fleets_in2d60 (D, matches) */
    B(0xA69C) = 0; B(0xA6A1) = 0; n = 0;
    for (k = 0..2) { if (B(0x1116+12k) == 0xFF) continue;
        B(0xA6A1)++;                                   /* fleets in service, all zones */
        if (B(0x1114+12k) != z) continue;
        B(0xA69E + n++) = k;                           /* list of the fleets in z */
        if (B(0x1116+12k) == 0) B(0xA69C)++; }         /* how many of them are Ready */
    return n; }

void fleet_table_2DB2(uint16_t title /*[bp+4]*/, uint8_t z /*[bp+6]*/) {   /* ret 4 */
    cursor_push(0);                                    /* 1D55 */
    n = fleets_in_2D60(z); if (n == 0) goto out;
    if (W(0x1B90) == 0) {                              /* box not drawn yet (3F23 clears the flag) */
        lh = B(0x9B5C) + 2;                            /* [bp-2] */
        y = 5; if (title) y = lh + 5;                  /* si */
        if (n > 3) n = 3; B(0x1B8C) = n;               /* menu record DS:1B7E item count (+0x0E) */
        h = (n + 1) * lh + y + 7; W(0x1B84) = h; W(0x1B52) = h;   /* record's Y1; the restore rect of 3F23 */
        save_691B(2, 0, 0x13D, h);                     /* push 2, 0, 0x13D, h */
        menu_draw_5614(rec 0x1B7E, mode 2);            /* = box 3134(2, 0, 317, h, [6346]) */
        W(0x9B69) = 0;
        text_899C(x 0x50, y, DS:1314 "Triremes  Quinqueremes  Galleons   Legion   Status");
        y += lh;
        for (i = 0; i < n; i++) { fleet_row_2E96(y, B(0xA69E + i)); y += lh; }
    }
    W(0x9B69) = 15; centred_8BE4(cx 0xA0, y 5, title);  /* title 0 = DS:0000 = "" */
    W(0x1B90) = 0xFFFF;
out: cursor_pop(); }                                   /* 1D8A */

void fleet_row_2E96(int y, uint8_t f) {                /* ported as province.c fleet_row2e96 (D, matches) */
    W(0x9B69) = 0; text_899C(25, y, W(0x1100 + 2f));
    num_8AC8(104, y, W(0x110E+12f)); num_8AC8(162, y, W(0x1110+12f)); num_8AC8(216, y, W(0x1112+12f));
    s = B(0x1117+12f) == 0xFF ? DS:139E "None" : numeral_6738(B(0x1922 + 20*B(0x1117+12f)));  /* legion's number */
    strcpy(local, s); centred_8BE4(256, y, local);
    centred_8BE4(295, y, W(0x1B54 + 2*B(0x1116+12f))); }   /* Ready / Moved / Forming */

uint8_t select_fleet_2C6F(uint8_t z) {                  /* ret 2 */
    pick = 3;                                          /* [bp-2] "nothing yet" */
    si = 0;
    n = fleets_in_2D60(z); if (n == 0) return 0xFF;
    if (n > 3) n = 3; B(0x1B8C) = n;
    cursor_push(0);
    if (n > 1 || B(0x1116 + 12*B(0xA69E)) != 0)        /* several, or the only one has acted: show the table */
        fleet_table_2DB2(B(0xA69C) ? 0x1390 /* "Select Fleet" */ : 0, z);
    if (n > 1 && B(0xA69C) != 0) {                     /* a choice to make */
        menu_on_55EA(0x1B7E);
        do {
            si = menu_run_54C8(0x1B7E, si);            /* items DS:1B5A: texts "", hotkeys '1' '2' '3', rows y 24/34/44 */
            if (si == 0xFF) pick = 0xFF;               /* Esc */
            else if (B(0x1116 + 12*B(0xA69E + si)) == 0) pick = B(0xA69E + si);
            else msgbox_59D4(DS:1348 "Fleet has already\nacted this turn");
        } while (pick == 3);
        menu_off_55FF(0x1B7E);
    } else {
        pick = B(0xA69E);
        if (B(0x1116 + 12*pick) != 0) {
            msgbox_59D4(n > 1 ? DS:136A "All fleets here have\nacted this turn" : DS:1348 "Fleet has already\nacted this turn");
            pick = 0xFF;
        }
    }
    cursor_pop();
    return pick; }
```
Menu record DS:1B7E (D): (2,0)-(317, Y1 patched), colour 0, items DS:1B5A, title DS:17DC "", count 3 (patched), single
0xFF, enabled 0; the word after it, **DS:1B90, is the "a box is up" flag** shared with the legion boxes (legions.c sets
it, 3F23 restores rect W(1B4C..1B52) when it is set and clears it). A single Ready fleet is taken without any box.

```c
void fleet_info_2F5A(uint8_t f) {                      /* ret 2; same frame as legions_move_header 3EA3 but W(1B90) is NOT set */
    W(0x1B52) = (B(0x9B5C) + 6) * 2 + B(0x9B5D);
    cursor_push(0);
    save_691B(2, 0, 0x13D, W(0x1B52));
    box_3134(2, 0, 0x13D, W(0x1B52), W(0x6346));
    W(0x9B69) = 0; text_899C(0x50, 5, DS:1314);
    fleet_row_2E96(B(0x9B5C) + B(0x9B5D) + 5, f);
    cursor_pop(); }
```
The info box stays on screen during the move and disappears with fleet_move's closing map_build (D).

## 4. fleet_move 0x2B40(f) (D)

```c
void fleet_move_2B40(uint8_t f) {                      /* ret 2 */
    dest = 0xFF;                                       /* [bp-4]: the map_build argument (pointer warp) */
    done = 0;                                          /* di */
    z0 = B(0x1114 + 12f);                              /* [bp-3] */
    cursor_push(0);                                    /* 1D55(0) */
    for (k = 0; k < 8; k++) {                          /* markers: adjacent SEAS only, on the screen */
        a = B(0x50EC + 8*z0 + k);
        if (a >= 27 && a != 0xFF) sprite_8E62(W(0xC0F2), anchor_x[a] (W 1AB8+4a), anchor_y[a] (W 1ABA+4a));
    }
    cursor_show();                                     /* 1CCE */
    while (!done) {
        wait_release();                                /* 35DB */
        if (wait_input() == 0) {                       /* 359B: a key */
            if (B(0xC07F) == 0x1B) done = -1;          /* Esc; every other key is ignored */
            continue;
        }
        z = zone_at(W(0xB6), W(0xB8));                 /* 6C67, either button */
        if (z == z0) done = -1;                        /* click on its own sea = cancel */
        if (z < 27 || z == 0xFF) continue;             /* land or nothing: ignored */
        for (k = 0; k < 8 && B(0x50EC + 8*z0 + k) != z; k++) ;
        if (k == 8) continue;                          /* not adjacent: ignored */
        B(0x1116 + 12f) = 1;                           /* "Moved", set BEFORE any fight */
        disk_check();                                  /* 1E21 */
        if (fight_armies_at_sea_4ADA(f, z) != 0        /* push z, f */
            && naval_B09E(f, z) == 1) {                /* thunk -> section 5 B0E0; `dec ax; jne` = only exactly 1 */
            B(0x1114 + 12f) = z;                       /* the fleet moves */
            L = B(0x1117 + 12f);
            if (L != 0xFF) B(0x1924 + 20L) = z;        /* and its legion with it */
        }
        done = -1;
        dest = z;                                      /* warp to the target even when the move failed */
    }
    map_build(dest);                                   /* 494A */
    cursor_pop();                                      /* 1D8A */
}
```
Notes (D): an enemy fleet's sea and a sea with an army get markers like any other sea; a Roman fleet may move onto another
Roman fleet. There is no "move" for a fleet that loses or retreats: it stays in z0 with status Moved. 0x5244 is never
called from here (a retreat on your own move = staying put).

## 5. 0x4ADA(f, z): every army at sea in z fights the fleet (D)

```c
uint16_t fight_armies_at_sea_4ADA(uint8_t f /*[bp+4]*/, uint8_t z /*[bp+6]*/) {   /* ret 4 */
    r = 0xFFFF;
    for (i = 0, si = 0xC11A; si < 0xC1B4; si += 14, i++) {      /* army records 0..10, si = A+8 */
        if (W(si) == 0 || B(si - 7) != z) continue;             /* active, A+1 == z */
        army_meets_4BAC(W(0x1100 + 2f), 0, z, i);               /* push i, z, 0, name */
        disk_check();                                           /* 1E21 */
        army_to_fleet6_531F(i);
        if (!(naval_B0C2(6, f) & 2)) { r = 0; break; }          /* push f, 6: B12A(enemy = record 6, Roman = f) */
    }
    return r; }
```
4BAC with numeral 0 at sea: "An army from <home>\ninvades <sea>\n" + 6738(0) + "Fleet I" + " meets the enemy on\nthe " +
W(3D8E) = DS:3D14 "rolling waves of destiny." (word list DS:3D7A: 3CD0 "An army from ", 3CDE "\ninvades ", 3CE8 "\n",
C072, 3CEA " meets the enemy on\nthe "; land: W(3D8C) = DS:3D03 "field of battle."). Already ported twice (enemy.c
army_meets_4bac, legions.c army_msg).

### 0x531F(i): army i becomes fleet record 6 (D)

```c
void army_to_fleet6_531F(uint16_t i) {                  /* ret 2 */
    t = rng_roll(3);                                   /* [bp-4]: ONE die, the ship type */
    A = 0xC112 + 14i;
    total = (uint16_t)(((B(A+0x0A) * 2 + W(A+4)) * 4) + W(A+6));   /* 16-bit; cavalry-like A+4 weighs 4 */
    q = total / B(0x8EDE + t);                         /* capacity 50 / 100 / 200 */
    n = q + (q >> 3);                                  /* ships = 9/8 of what carries the army */
    for (j = 0; j < 3; j++) W(0x1156 + 2j) = (j == t) ? n : 0;
    B(0x115C) = B(A+1);                                /* zone */
    B(0x115E) = 0;                                     /* active */
    B(0x115F) = i;                                     /* marines = army i */
    B(0x1160) = total / n;                             /* men per ship (byte); DIVIDES BY ZERO when n == 0 */
}
```
Record 6's +7 (DS:115D, home) is not written (it stays 0xFF from the static DS; B12A only reads it when +9 is 0xFF,
which never happens for record 6) (D).

## 6. End Turn: an army moves out to sea, 0x51C5..0x523C (D; replaces enemy.c lines 186..197)

```c
/* in army_move 0x4F27(i), t >= 27 */
B(A+1) = t;                                            /* 0x51C5: the army is at sea whatever follows */
for (k = 0; k < 3; k++) {                              /* [bp-1] */
    if (B(0x1116+12k) == 0xFF || B(0x1114+12k) != t) continue;
    army_meets_4BAC(W(0x1100+2k), 0, t, i);            /* 520E */
    disk_check();                                      /* 5211 */
    army_to_fleet6_531F(i);                            /* 5217: a NEW die per Roman fleet */
    r = naval_B0C2(6, k);                              /* 5221 */
    if (r & 2) return;                                 /* 522A: army beaten (2) or both gone (3) */
    if (r & 4) fleet_retreat_5244(k);                  /* 522C: `test al, 4` on the same result */
}                                                      /* 1 (lost) or 0 (cancelled): the next Roman fleet in t fights */
return;
```

### 0x5244(k): fleet retreat (D)

```c
void fleet_retreat_5244(uint8_t k) {                   /* ret 2 */
    ok = 0;                                            /* [bp-0xC] */
    z = B(0x1114 + 12k); d = B(0x3D55 + z);            /* sea "strength" (same table as enemy.c zone_strength) */
    for (n = 0; n < 8; n++) {
        a = B(0x50EC + 8z + n);
        if (a == 0xFF || a < 27) continue;             /* seas only */
        if (B(0x3D55 + a) > d) continue;               /* unsigned: only to an equal or weaker sea */
        for (si = 0x1138; si < 0x115C; si += 12)       /* enemy fleets 3..5 (+6 zone, +8 status) */
            if (B(si) == a && B(si+2) != 0xFF) goto next;
        B(0x1114 + 12k) = a;                           /* first acceptable neighbour, in table order */
        if (B(0x1117 + 12k) != 0xFF) B(0x1924 + 20*B(0x1117 + 12k)) = a;
        ok = -1; break;
    next: ;
    }
    if (!ok) {
        msgbox_59D4(DS:3DCE "Your fleet has\nnowhere to retreat\nand is destroyed!");
        B(0x1116 + 12k) = 0xFF;                        /* ship counts and +9 are NOT cleared (bug 1, section 9) */
        if (B(0x1117 + 12k) != 0xFF) legion_lost_4816(B(0x1117 + 12k));
    }
}
```
DS:3D55 for the seas 27..36 = {3,2,2,2,1,1,2,3,3,3}. Armies at sea in the target do not block a retreat (only fleets
3..5) (D).

## 7. What B0E0 and B12A return and how each caller uses it (D)

B12A(enemy record [bp+4], Roman fleet [bp+6]) (section 5, NAVAL_INDEX section 6), result si:

| r | meaning | DS effects inside B12A |
|---|---|---|
| 0 | set-up cancelled: Esc in the flagship menu C274 (only shown when the Roman fleet has 2+ ship types) or the graphics failed to load (BF50) | none (C0FC not counted) |
| 1 | lost: Roman ship sum 0 | Roman +8 = 0xFF, its legion → 4816 (destroyed) |
| 2 | won: enemy ship sum 0 | enemy +8 = 0xFF, W(C11A + 14*enemy+9) = 0 (the army / marine record inactive); C0FE++ (fleet battles won) |
| 3 | both sums 0 | both of the above, C0FE not counted |
| 4 | both fleets still have ships (no fleet battle: player pressed 'r', declined to re-engage) | none |

V (DOSBox captures of 10 Nov, tools/dosbox/results/naval): result 0 by Esc at the menu -> B0E0 0 (BD8A had already
stopped the music and freed its slot 0, so "none" above is not quite true: 3EEE = FF, A6B0..A6B3 = 00 00 00 FF);
result 1 -> B0E0 0; result 2 -> B0E0 1 with C0FE + 1; no enemy fleet in the zone -> B0E0 1 with no DS change. The port
gives the same DS and return values (NAVAL_LOGIC_NOTES 17).

| caller | use |
|---|---|
| B0E0(Roman f [bp+4], zone [bp+6]) | for enemy fleets 3..5 in the zone: `if (!(B12A(e, f) & 2)) return 0;` else go on; returns 1 = way clear |
| 2B40 fleet_move | 4ADA != 0 and B0E0 == 1 → the fleet (and legion) move to z; anything else: it stays (status Moved) |
| 4ADA | stops and returns 0 at the first result without bit 1 (0, 1, 4); 2 and 3 go on to the next army |
| End Turn 0x51C5 | bit 1 (2, 3): the army is beaten, stop; bit 2 (4): 0x5244 retreat; 0 or 1: the next Roman fleet in t |

## 8. Disembarking and boarding (legion move 0x382A, legions.c) (D: the asm 0x3929..0x3A1C compared with legions_move)

- On land, every adjacent sea holding a Roman fleet with +8 != 0xFF and no legion (+9 == 0xFF) gets the ship marker
  (C10C at anchor - (3,4)); clicking it boards via 3AE4 (capacity, "N Legionnaires must be left behind.\nBoard the
  Legion?" DS:1F28): fleet +9 = L, legion +0x11 = f, +0x0E = 1, zone = the sea. The fleet's own status is not looked at.
- At sea (z0 >= 27) only adjacent provinces get markers; a click on an adjacent sea does nothing. Clicking a province:
  +0x0E = 1, 4B3F land fight, then 5F82 for a foreign province; on success zone = z and, because z0 >= 27, fleet
  +9 = 0xFF and legion +0x11 = 0xFF (the legion leaves the fleet); a lost fight leaves it aboard (dest = z0).
- legions.c does all of this already. A boarded legion moves with fleet_move (section 4) and 5244.

## 9. 1990 bugs and quirks found here

1. **(D) 5244 "destroyed" keeps the ships.** It sets +8 = 0xFF but leaves the three counts (and +9). Build Fleet 0x2458
   takes the first record with +8 == 0xFF and starts from its counts (province.c build_fleet2458: `a/b/c = W(...)`, and
   with n != 0 it re-activates the record as Forming even if you buy nothing). So a fleet "destroyed" by a failed retreat
   comes back with all its ships at the next Build Fleet. (B12A's own "lost" path is safe: the sum is 0 there.)
   Fix (default): clear the 3 counts and +9 in 5244's destroy branch; --original keeps them.
2. **(D code, G how often) 531F divides by zero** when total < capacity of the rolled type (n = 0): `div di` at 0x53AF,
   a CPU divide error in DOS. Needs an army at sea under 50/100/200 men. Fix (default): n = 1 when 0, with a stderr note
   as elsewhere in enemy.c; --original can only report it (a crash cannot be reproduced usefully).
3. **(D code, G reachability) a dead Roman fleet fights on.** Result 3 (both fleets gone, "together forever in a deadly
   embrace") has bit 1 set, so neither 4ADA (`0x4b25 test al,2`) nor B0E0 (`b116 test al,2`) stops: 4ADA goes on to the
   next army in z, and fleet_move then calls B0E0, which fights any enemy fleet in z, each with a Roman fleet of 0 ships.
   B12A's C274 with no ship type returns an uninitialised local ([bp-8]) as the flagship type, and the set-up decrements a
   zero count to 0xFFFF. Needs a mutual wipe-out and a second army at sea (or an enemy fleet) in the same sea.
   Fix (default, 2026-11-09 after review): 4ADA and B0E0 test B(0x1116 + 12f) == 0xFF **before** each further battle and
   start none; the loops' return values stay the original's (4ADA FFFF, B0E0 1). So the common case, a result 3 against a
   single army or a single enemy fleet, writes exactly what the original writes. --original fights on.
   **Quirk kept in both modes (D):** after such a result 3, 2B40 still "moves" the dead fleet (`0x2c30` +6 = z) and writes z
   into the zone byte (+0x0C, DS:1924 + 20L) of the legion that 4816 has just destroyed (`0x4846` FFh, overwritten by
   `0x2c49`; B12A's lost branch b513..b530 leaves +9 = L). Invisible: every reader tests the fleet's +8 (the map's sea
   click in map.c, province.c, legions.c, B0E0) or the legion's +0x0D (1925, FFh) first. (A first version of the fix returned 0 after any
   result 3, which skipped these writes; that changed the DS in the reachable single-enemy case and was dropped.)
4. **(D) Esc at the flagship choice during End Turn skips the attack.** B12A returns 0, which is neither "beaten" nor
   "retreat": the fleet stays in the sea with the army, nothing is lost, and the next Roman fleet there is attacked. On
   your own move Esc just cancels the move. Not fixed unless Erik wants it (a fix would treat 0 as 4 in End Turn).
5. **(D code, G reachability) an unselectable fleet.** The map loop takes the LAST fleet record in the zone; if an enemy
   fleet shares a sea with a Roman fleet (0x541F restores a destroyed enemy fleet in its home sea when its province is
   lost, and a Roman fleet may be parked there), a left click always shows zone_info and the Roman fleet can never be
   selected again. Report only (no fix by default).
6. **(D) 531F's total is 16-bit** ((A+0x0A*2 + A+4)*4 + A+6); it would wrap above 65535 (A+4 > ~14000). Not seen in the
   captures (army 0: 7040). Keep as is.
7. Minor (section 5, D): the duel's retreat key is lowercase 'r' only (0xC366 compares 0x72); with Caps Lock 'R' does
   nothing.

No new Easter eggs in the fleet-order code itself. Worth keeping: "Galleons" (DS:1314, DS:96E8) as a Roman ship type is a
1990 anachronism; the naval texts (witless commander, the admiral's bath, Neptune's locker without a sword, the deadly
embrace, the shark) are listed in NAVAL_INDEX section 6.

## 10. Erik's report: "I always lost the battle even after sending the enemy's witless commander to Neptune's depths"

**Answer: the outcome logic is not inverted and does not ignore the flagship (D).** The message Erik saw, DS:9220
"Alas, your ship duel was with the witless commander of the enemy fleet, and his more competent mates have swept the seas
clean of Roman vessels.", is exactly the text for "your flagship survived, the enemy's sank, and your fleet was wiped
out" (result 1 with flag bit 0 clear, mode != 8). The flags are right: BD8A sets C38F = hull of the ROMAN flagship type
C37B and C38E = hull of the enemy's C37A (`bde1..bdf6`), and B12A bit 0 = C38F == 0, bit 1 = C38E == 0. Sinking the
enemy flagship only changes the fleet battle's odds a little; the fleet battle is decided by numbers, and the numbers are
usually far on the enemy's side. Three things do it:

**(a) The fleets fight in the background during the whole duel (D).** In the duel loop (b2f4..b321):
```
b2f4: push 8 / call 0x183            ; ticks_since(timer 8)
b2fb: cmp ax, [0xc386] / jbe b324
b305: call 0xd7 (timer 8 restart)
b310: call 0xda50 (enemy, roman)     ; a full fleet-battle round; its return value is ignored
b317: rng_roll(9); C386 = (r + 2) * 0x48
```
The timer runs at 72.8 Hz (BATTLE_NOTES), so a round every 2..10 s (the first after C386 = 0xD8 = 3 s, BE0F). A one-
minute duel is about 10 rounds, with BOTH flagships afloat, i.e. the enemy at its full rates. The ship numbers at the
bottom of the duel screen (DC8E, (0x19,0xBB) Roman / 0x90 enemy) show it. When the duel ends, the surviving Roman
flagship is put back (b450 `inc W(110E + 2*(C37B + 6r))`), and if the escort is already gone the post-duel fleet battle
is one ship against the rest: Erik's message.

**(b) After a sunk flagship, DA50 rounds run until one side has no ships (D, the whole round):**
```c
int fleet_round_DA50(uint8_t e /*[bp+4]*/, uint8_t r /*[bp+6]*/) {      /* ret 4 */
    C400 = C402 = C404 = C406 = C408 = C40A = 0;     /* C400 hits on the enemy, C402 hits on Rome */
    L = B(0x1117 + 12r);
    if (L != 0xFF) { C408 = W(0x191E + 20L) + W(0x1920 + 20L);              /* legion aboard = marines */
        for (s = 0; s < C408; s += 500) if (rng_roll(100) <= 5) C400++; }    /* 6% per 500 men */
    for (j = 0; j < 3; j++) { n = W(0x110E + 12r + 2j); C404 += n;
        p = C38F ? B(0x99BC + 3*B(0x491) + j) : B(0x99C8 + 3*B(0x491) + j);  /* by naval difficulty DS:0491 */
        for (s = 0; s < n; s++) if (rng_roll(100) <= p) C400++; }
    if (C404) for (s = 0; s < B(0x99D4 + B(0x491)); s++)                   /* {8,6,4,2} flagship rolls */
        if ((int)rng_roll(100) <= (int8_t)C38F / 4) C400++;
    a = B(0x1117 + 12e);
    if (a != 0xFF) { C40A = W(0xC116 + 14a) + W(0xC118 + 14a);             /* enemy marines A+4 + A+6 */
        for (s = 0; s < C40A; s += 500) if (rng_roll(100) <= 5) C402++; }
    C406 = W(0x110E + 12e + 2*C37A);                                        /* only the flagship's type */
    p = C38E ? B(0x99C2 + C37A) : B(0x99CE + C37A);                          /* fixed row, by TYPE */
    for (s = 0; s < C406; s++) if (rng_roll(100) <= p) C402++;
    if (C406) for (s = 0; s < 4; s++) if ((int)rng_roll(100) <= (int8_t)C38E / 4) C402++;
    if (C404) losses_roman_DD2E(r);                  /* C402 hits spread over the Roman types in proportion, remainders
                                                        kept in C3BE..C3C3; legion men lost = hits * fleet +0x0A, cavalry
                                                        share / 4; stats C08E (ships), C086 (men) */
    if (C406) losses_enemy_DE51(e);                  /* C400 hits off the flagship type's count; marines lost = hits *
                                                        enemy +0x0A over A+0x0A (/8), A+4 (/4), A+6; stats C08A, C082 */
    C404 = sum of the Roman counts; C406 = sum of the enemy counts;
    return (C404 == 0 || C406 == 0) ? 0xFFFF : 0; }
```
Tables (static DS): Roman per-ship p, rows by difficulty 0..3 (Galley Slave .. Emperor), columns trireme / quinquereme /
galleon: 99BC = {5,8,12},{4,6,10},{3,5,8},{2,3,5}; flagship sunk 99C8 = {2,4,6},{2,3,5},{1,2,4},{1,1,2}. The enemy
always uses the Senator row: 99C2 = {3,5,8}, flagship sunk 99CE = {1,2,4}. A hit is `roll <= p`, so p+1 percent. So the
enemy flagship's loss lowers the enemy's per-ship chance from 4/6/9% to 2/3/5% and its 4 bonus rolls to 1% each; that is
all it does. Rome keeps its flagship bonus (8/6/4/2 rolls at hull/4 percent) only while its flagship floats.

**(c) The enemy is usually much bigger (D for the arithmetic, from the static DS and the captures).** The standing
fleets have 50 quinqueremes (Carthage), 100 galleons (Egypt), 30 quinqueremes (Macedonia), each with marines up to 8000
(army records 11..13, 6% per 500 men per round). A marauding army at sea becomes record 6 with ships = 9/8 x men /
capacity of a RANDOM type: army 0 in `endturn/t8_in.bin` (total 7040) is 157 triremes, 78 quinqueremes or 39 galleons
depending on one die (rng_roll(3)), plus 4600 marines.

Monte Carlo of the post-duel rounds alone, with the decoded rules (G: a model without the background rounds, 1000-2000
runs per cell; script `C:\Users\erikk\AppData\Local\Temp\claude\bfb5522be8ee045d\c4bf0f5f-57f9-4ea6-80b3-e1aa05bf0592\scratchpad\fleet\fo\sim.py`), share of Roman wins after the ENEMY flagship sank and the
Roman one (hull 45) survived, difficulty 0 / 1 / 2 / 3:

| Rome | enemy | win share |
|---|---|---|
| 20 quinqueremes | Carthage 49 + 5000 marines | 0.08 / 0.00 / 0.00 / 0.00 |
| 30 quinqueremes | Carthage 49 | 0.83 / 0.47 / 0.12 / 0.00 |
| 30 quinqueremes + a 5000-man legion | Carthage 49 | 0.93 / 0.71 / 0.38 / 0.04 |
| 30 quinqueremes | marauders as 156 triremes + 4600 | 0.00 at every level |
| 30 galleons (hull 60) | marauders as 38 galleons + 4600 | 0.97 / 0.85 / 0.47 / 0.04 |

So "always lost" is what the 1990 rules give for a Roman fleet of a few dozen ships, especially at Senator/Emperor, and
the background rounds during a long duel make it worse. **No bug to fix in the outcome (D).** Two design points Erik may
want to know (not bugs, G as to intent): the enemy's skill row ignores the difficulty (always Senator), and the marauder
fleet's size depends on a single die (triremes = 4x as many ships as galleons). B59F's "In spite of their admiral's
bath, your fleet prevails." (DS:9104) is shown when YOUR flagship sank and you still won; "their" reads as the fleet's
(your sailors') admiral, so this is a joke at the player's expense rather than a bit mix-up (G).

## 11. For the implementer

- Map: replace the stub at map.c:374 with `f = select_fleet_2C6F(z); legions_close_box(); if (f < 3) { fleet_info_2F5A(f);
  fleet_move_2B40(f); }`. Reuse province.c's fleets_in2d60 / fleet_row2e96 (drop `static`), its menu helpers
  (menu_run54c8, menu_on55ea, menu_off55ff, msgbox59d4) and legions.c's close box (3F23) and LEV_HIGHLIGHT-style marker
  drawing (marker sprite here is W(0xC0F2), MINIT descriptor 7, drawn at the anchor itself, no (-3,-4) offset).
- Fleet moves work without the naval battle whenever 4ADA finds no army at sea in z and B0E0 finds no enemy fleet in z:
  port 2B40/4ADA/531F/5244 now and keep one stub `naval_battle_B12A(e, f)` that reports "The naval battle\nis not ported
  yet." and returns 0 (= cancelled: nothing moves, the fleet is Moved) until section 5 is ported. In End Turn the stub's 0
  means "next fleet", the same as the original's Esc.
- Dice order: on a fleet move 4ADA → per army: 531F's rng_roll(3) then B12A's dice; End Turn: 531F's die is rolled again
  for every Roman fleet attacked. B0E0 rolls nothing itself.
- 1E21 (disk check) and the cursor calls do nothing in the port (house style).
- Headless test: no capture has a Roman fleet (all `endturn/*.bin` have records 0..2 at 0xFF; armies 0/1 at sea in
  t7_out..t9_in). Set one up as legions.c's test 8 does (`setW(FLEET(0), 50); B(FLEET(0)+6) = 33; B(FLEET(0)+8) = 0;
  B(FLEET(0)+9) = 0xFF`) and script: click the sea, (menu click if 2+ fleets), click a neighbour. Expected: status Moved,
  zone = target, icon moved, the legion aboard (if any) with it; Esc / own sea = cancel with status unchanged.

## 12. Open questions

- What DS:3D55 really measures for the seas (the "strength" enemy.c uses for armies; here retreats only go to an equal or
  lower value) (G).
- Whether the background DA50 rounds during the duel were meant to stop when a side reaches 0 (the return value is
  dropped at b310) (G). As decoded, they never stop the duel.
- Where the victory text DF74 reads "You won N fleet battles": C0FE (incremented only for result 2, not 3) is the likely
  counter (G).
