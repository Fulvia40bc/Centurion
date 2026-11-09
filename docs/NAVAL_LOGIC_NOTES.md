# Naval battle logic (OVL0 section 5): notes for the port

Source: `disasm/OVL_NAVAL.asm` (addresses are 0823 offsets; index `docs/NAVAL_INDEX.md`), EXE callers in
`disasm/EXE_RUNTIME.clean.asm`. Tables were read from the static DS (OVL0.OVL file 0xB400 + X) and from the runtime image
`tools/dosbox/results/endturn/t10_in.bin`; every table quoted here is identical in both unless noted (V, by comparing).
Marks: **D** decoded from the asm, **V** verified (compared/run), **G** guess. `rng(n)` = EXE 076B, 0..n-1 (high word of
r*n, so `rng(0)` returns 0 but still advances the generator, D). `050F(v, step, lim)` = max(v - step, lim), signed, args pushed
v, step, lim. Ticks: the timer ISR runs at 72.8 Hz; `since(n)` = 0183 ticks since slot n, `start(n)` = 00D7.
`diff` = B(0x0491) = naval difficulty 0 Galley Slave, 1 Legionnaire, 2 Senator, 3 Emperor.

**Side index s:** 0 = the enemy (computer), 1 = the Roman (human). Every per-side array below is `base + s` (bytes),
`base + 2s` (words) or `base + 4s` (x,y pairs). Proof (D): C37B is set from the player's ship menu C274 (BDA0); D60C loads the
"Human Sinking" sheet for bit 0 and draws it at C376/C378 (D756..D761); the Roman fleet's ship slot `[bp-14h] + C37B` is the one
re-added at B450.

---------------------------------------------------------------------------------------------------------------------------

## 0. Erik's report: "I always lost the battle even after sending the enemy's witless commander to Neptune's depths"

**Answer (D): the win/lose decision is not inverted and does not ignore the flagship. The flagship duel decides nothing by
itself; the battle is won or lost only in the fleet battle DA50, a dice race in which every hit sinks one whole ship, and the
enemy fleets are big. Sinking the enemy flagship only lowers the enemy's odds in that race. It is a design, not a bug.**

1. The result is set from the ship counts only, after the fleet battle (B4C6..B558):
   ```
   b4e5: mov bx,[bp-2] / mov ax,[bx] / add [bp-6],ax      ; [bp-6] = sum of the 3 ship words of the ROMAN fleet ([bp+6])
   b4ed: mov bx,[bp-4] / ... / add di,[bx]                ; di     = sum of the ENEMY fleet ([bp+4])
   b506: or dx,dx / je b513 / or di,di / je b513 / mov si,4   ; both left -> 4 = retreat
   b513: or dx,dx / jne b536 ... mov si,1                 ; Roman fleet empty -> 1 = lost (fleet FF, legion 4816)
   b536: or di,di / jne b55b ... or si,2                  ; enemy fleet empty -> |2 = won (3 = both gone)
   ```
   The two sums are taken over the right fleets (B4CD `mul [bp+4]` = enemy, B4D8 `mul [bp+6]` = Roman). V by reading.
2. The fleet battle runs whenever one flagship is gone, until one fleet is empty:
   ```
   b49e: cmp byte ptr [0xc38f],0 / je b4ac               ; Roman flagship sunk
   b4a5: cmp byte ptr [0xc38e],0 / jne b4c6              ; enemy flagship sunk
   b4ac: call 0xda50 (enemy, Roman) / or ax,ax / je b4ac ; until one fleet has 0 ships
   ```
   AND it already runs during the whole duel, every 2..10 s, at the full "both flagships afloat" odds:
   ```
   b2f4: push 8 / call 0x183 / cmp ax,[0xc386] / jbe b324  ; ticks since slot 8 > C386 ?
   b301: push 8 / call 0xd7 / ... call 0xda50              ; a background round of the fleet battle (result ignored)
   b313: push 9 / call 0x76b / inc ax / inc ax / mov cx,0x48 / mul cx / mov [0xc386],ax   ; next one in (2..10)*72 ticks
   ```
   (first one after 0xD8 = 216 ticks = 3 s, BE0F). The panel's "SHIPS" numbers (DC8E) show these losses live. A long
   duel therefore wears the Roman escort down before the flagship is even sunk.
3. What sinking the enemy flagship does (all of it, D):
   - B436 `cmp [0xc38e],0 / jne / or di,2 / add [0xc08a],1`: flag bit 1 (texts) and +1 to the enemy-ships-sunk counter;
   - B45F `test di,2 / jne b474`: the flagship (taken out of its fleet at B174 for the duel) is NOT given back;
   - in every later DA50 round the enemy's per-ship hit chance comes from 99CE instead of 99C2 (DBB6
     `cmp [0xc38e],ah / jne / mov al,[bx+0x99ce]`): trireme 4% -> 2%, quinquereme 6% -> 3%, galleon 9% -> 5%;
   - the 4 "enemy flagship" bonus rolls (DBF1..DC11, hit if `rng(100) <= C38E/4`) drop to 1% each.
   Nothing else: the enemy crews do not lose heart in the dice, the fleet is not routed.
4. The race (DA50, section 9): each round every Roman ship rolls once (hit if rng(100) <= 99BC[3*diff+type], or 99C8 if the
   Roman flagship sank), the legion aboard rolls 6% per 500 men, the Roman flagship rolls 99D4[diff] = 8/6/4/2 times at
   (hull/4+1)%; the enemy rolls the same way with the fixed row 99C2 (= the Senator row) plus 6% per 500 soldiers of its army
   (5000..8000 men = 10..16 rolls) and 4 flagship rolls. Each hit sinks one ship of the other side. The enemy fleets are:
   Carthaginian 50 quinqueremes, Aegyptian 100 galleons, Macedonian 30 quinqueremes (static DS 110E+12f, f = 3..5), rebuilt
   +1 ship per turn back to those caps (EXE 0x4CF2, enemy.c); an army met at sea becomes the "Marauding Fleet" (record 6) of
   men/capacity * 9/8 ships of a random type (EXE 0x531F, section 2).
5. Computed odds (Monte Carlo of the decoded DA50/DD2E/DE51 rules, 400..600 runs each, script
   `scratchpad/fleet/simfleet.py`; G only in that army losses are ignored): Roman quinqueremes with a 4200-man legion and the
   flagship afloat (hull 30) against the Carthaginian fleet after its flagship sank (49 quinqueremes, 5000 soldiers):

   | Roman ships | 10 | 20 | 30 | 40 | 50 | 60 |
   |---|---|---|---|---|---|---|
   | Galley Slave | 0% | 3% | 68% | 100% | 100% | 100% |
   | Legionnaire | 0% | 1% | 37% | 94% | 100% | 100% |
   | Senator | 0% | 0% | 15% | 81% | 100% | 100% |
   | Emperor | 0% | 0% | 1% | 29% | 86% | 100% |

   The same with the enemy flagship still afloat: Legionnaire 40 ships 7%, 50 ships 66%. So sinking the commander turns a
   lost cause into a win only when the Roman fleet is already about 60..80% of the enemy's size. With a smaller fleet (or
   after the background rounds have eaten the escort) the Romans lose every time, and B59F then shows exactly the text Erik
   saw:
   ```
   b5f9: test si,1 / je b604          ; si here = the flagship bits [bp+8]; bit 0 = Roman flagship sank
   b604: mov ax,0x9220                ; "Alas, your ship duel was with the witless commander of the enemy fleet, and his
                                      ;  more competent mates have swept the seas clean of Roman vessels."
   ```
   i.e. outcome 1 (lost) with the Roman flagship afloat. The text was written for this case.
6. "In spite of their admiral's bath, your fleet prevails." (9104) is the WON text for bit 0 = the ROMAN flagship sank
   (B61C). The index wondered whether this is a bit mix-up: it is not. Bit 0 is the Roman (proof above), "their" is the
   fleet's own admiral, i.e. you took the bath. Correct as is (D).

**No fix is proposed for the decision.** Real 1990 bugs found elsewhere in the naval code are in section 13 (the ram
damage wrap-around can sink or strip a flagship on a slow scrape; the boarding bonus looks inverted; hull below zero is not
seen as sunk). If Erik wants the duel to matter more, that would be a design change, not a bug fix (section 13, last item).

---------------------------------------------------------------------------------------------------------------------------

## 1. Entry points and callers (D)

| entry | thunk | caller | call |
|---|---|---|---|
| B0E0(Roman fleet r, zone z), ret 4 | B09E | fleet move 0x2C24 | `for f = 3..5: if B(1114+12f) == z && B(1116+12f) != FF { res = B12A(f, r); if (!(res & 2)) return 0; } return 1;` 2C27: result 1 -> 2C30 the fleet moves into z (1114 = z, and the legion aboard: legion +0x0C = z) |
| B12A(enemy e, Roman r), ret 4 | B0C2 | 0x4B22 (in 4ADA(fleet, zone)) | for each active army i (W(C11A+14i) != 0) whose zone B(C113+14i) == zone: 4BAC message, 1E21, **531F(i)**, `res = B12A(6, fleet)`; `res & 2` -> next army, else 4ADA returns 0 |
| | | 0x5221 (End Turn, army i enters sea zone with Roman fleet k) | 4BAC, 1E21, **531F(i)**, `res = B12A(6, k)`; `res & 2` -> stop; else `res & 4` -> **5244(k)** (Roman fleet retreats); next fleet |

Results: 0 = set-up cancelled (no flagship chosen or graphics failed), 1 = lost, 2 = won, 3 = both fleets gone, 4 = retreat
(both fleets still have ships: re-engage declined, 'r' key, or the duel ended without a sinking).

**Fleet record 6 "Marauding Fleet" (EXE 0x531F(army i), D):** an army at sea is turned into a one-type pseudo fleet:
```
t   = rng(3);                                          /* 532B: the ship type is random */
men = ((B(C11C+14i)*2 + W(C116+14i)) * 4) + W(C118+14i);   /* weighted strength: +0x0A x8, +4 x4, +6 x1 */
n   = men / B(8EDE + t);  n += n >> 3;                 /* ships = men / capacity * 9/8 */
W(1156 + 2j) = (j == t) ? n : 0;  B(115C) = B(C113+14i) (zone); B(115E) = 0; B(115F) = i (army); B(1160) = men / n;
```
(n == 0, an army smaller than one ship's capacity, divides by zero at 53AF: a crash in 1990, enemy.c's business, D.)
Fleet name W(110C) = "Marauding Fleet".

**All enemy fleets have one ship type only (D):** B397 fills 3..5 from static data (one non-zero type each), 0x4CF2 rebuilds
only type W(3D90+2k) = {1, 2, 1}, 0x541F resets to that type, 531F one random type. This matters because DA50/DE51 only
count the enemy flagship's type (section 9).

## 2. Data

### Fleet record F = 110E + 12f (D; GAMELOOP_NOTES, LEGIONS_NOTES)
| off | DS (f=0) | meaning |
|---|---|---|
| +0,+2,+4 w | 110E | ships of type 0 Trireme (sml), 1 Quinquereme (med), 2 Galleon (lrg) |
| +6 | 1114 | zone |
| +7 | 1115 | home province (courage source when no legion/army aboard: B(527A + 32*home) = province +0x0A) |
| +8 | 1116 | status, FF = fleet gone |
| +9 | 1117 | Roman: legion aboard (FF none); enemy 3..5: army record 11..13 (B397); record 6: army i |
| +10 | 1118 | men per ship (weighted, cav x4): written by board 3AE4 / B397 / 4CF2 / 531F |

Legion L (1918+20L): +6 w infantry (191E), +8 w cavalry (1920), +0x0B courage (1923). Enemy army A (C112+14A): +4 w (C116),
+6 w (C118) strengths, +8 w active (C11A, 0 = gone), +0x0A b (C11C, weighted x8), +0x0C b courage (C11E).

### Battle state block (written by BD8A/BE0F/B12A; s = side) (D)
| DS | size | meaning |
|---|---|---|
| C36E[2] | b | courage % (s=0 enemy, s=1 Roman) = army/legion courage + difficulty bonus; +10 on boarding (BB7E) |
| C370[2] | b | bearing from side s to the other ship (0..15); C371 = C641(), C370 = (C371-8)&15 |
| C372[2] | 2w | ship position x,y (s=0 at C372/C374, s=1 at C376/C378) |
| C37A[2] | b | ship type (0..2) of the flagship; C37A enemy = the LAST type with ships (BD8A), C37B = player's choice |
| C37C | b | frames left to show the speed name instead of "MEN" (C422 sets 10) |
| C37E[2] | w | crew (men) = capacity 8EDE[type]; Roman halved when no legion aboard (B20E) |
| C382 | w | distance between the ships = isqrt(dx^2+dy^2) (C4F8/C558) |
| C384 | w | the "rammer" side (whose bow is in the other hull, CFDD); also drawn last; init 1 |
| C386 | w | ticks between background fleet rounds (also the melee round timer in BB7E) |
| C388[2] | b | heading 0..15 (0 = north/up, 4 = east, 8 = south, 12 = west); init enemy 4, Roman 12 |
| C38A[2] | b | speed index = 9818[5*type + C38C] |
| C38C[2] | b | oar level 0..4 (STOPPED, CRUISING, BATTLE, ATTACK, RAMMING SPEED); init 1 |
| C38E[2] | sb | flagship hull = 8EE2[type] (30/45/60); 0 = sunk |
| C390[2] | b | oar animation frame 0..6 (6 = rest) |
| C392[2] | b | fire pot: 0 none, FF flying, 1 splash animation |
| C394[2] | sb | oar level change request (-1/0/+1), consumed by C422 |
| C396[2] | sb | turn request (-1/0/+1), consumed by C3D3 |
| C398[2] | 2w | fixed-point fractions (1/256 px) of x,y |
| C3A0[2] | b | fire on the ship: frames left (CD4B) |
| C3A2[2] | w | ship sheet (sml/med/lrg) per side; C3A6/C3A8 oars1/oars2; C3AC smok; C3AE bombs; C3BC flag |
| C3AA[2] | b | grappling hooks: frames left (only C3AB, the Roman, is ever set) |
| C3B0..C3B8 | w | pointers into collide.lzw: X[240], Y[240], R[240], bowX[48], bowY[48] |
| C3BA[2] | b | arrow volley flying (FF) |
| C3BE[3] | w | remainder accumulators of the Roman fleet losses (DD2E), zeroed per battle in BD8A |
| C3C4[2] | w | arrow volley range left |
| C3C8[2] | w | fire pot range left |
| C3CC[2] | b | fire pot direction; C3CE[2] arrow direction |
| C3D0[2] | 2w | fire pot position; C3D8[2] arrow position; C3E0[2] / C3E8[2] their fractions |
| C3F0[2] | 4w | ship bounding rect x0,y0,x1,y1 (CF7F) |
| C400 / C402 | w | fleet battle: hits scored by the Romans / by the enemy this round |
| C404 / C406 | w | Roman ship total / enemy ships of the flagship type (DA50; DC8E reuses them for the display) |
| C408 / C40A | w | Roman legion men (inf+cav) / enemy army strength (+4 + +6) |
| 98BE[2] | b | fire pot animation frame (0/16 flying, 0x33..0x39 splash) |
| 98E0[2] | b | fire animation frame 0..2 |
| AAC6[2], AAC8[2], AACC[2] | | shark frame / x / y per side (D60C/D89B) |

Totals (long, add/adc): C082 enemy casualties and C086 Roman casualties (shared with the land battle, battle.c 610),
C08A enemy ships sunk, C08E Roman ships lost; C0FC naval battles fought (+1 per set-up), C0FE naval victories (+1 on
outcome 2 only, B618). (G: the totals feed the end-of-game statistics.)

### DS tables (values V: static DS == runtime t10)
| DS | type | values | use |
|---|---|---|---|
| 8EDE[3] | b | 50, 100, 200 | ship capacity = crew per type |
| 8EE2[3] | b | 30, 45, 60 | flagship hull per type |
| 8EE6[4] | sb | -5, -2, 0, +5 | enemy courage bonus by diff |
| 8EEA[4] | sb | +5, +3, 0, -5 | Roman courage bonus by diff |
| 93A6[16] | sb | 0, -1 x7, 0, +1 x7 | AI turn direction by relative bearing |
| 93B6[16] / 93C6[16] / 93D6[16] | sb | 1,5,1,2,0,2,1,5,1,5,1,2,0,2,1,5 / 0,2,1,5,1,5,1,2,0,2,1,5,1,5,1,2 / -1,1,1,1,1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1 | AI tie-break when aligned |
| 93E6[16] | sb | 0,7,..,105 (7h) | oar frame base per heading |
| 93F6[8] | sb | 0,1,1,2,3,4,4,0 | pixels per movement step by speed index |
| 93FE[8] | b | 21,21,17,17,12,12,8,0 | ticks between movement steps by speed index |
| 9818[15] | b | sml 0,2,3,5,6; med 0,2,3,4,5; lrg 0,1,2,3,4 | speed index by type and oar level (9819 = 9818+1 = level 1) |
| 9828[3] / 982C[3] | b | 72, 108, 144 | ticks between turns / oar changes by type |
| 987A[16] / 989A[16] | sw | sin*256: 0,98,181,237,256,... / -cos*256: -256,-237,... | heading vectors (x / y) |
| 98C0[4] / 98C4[4] | b | 8,6,4,2 / 4,6,8,10 | fire pot hull damage to the enemy / to the Roman, by diff |
| 98C8[4] / 98CC[4] | b | 55,50,45,40 / 45,50,55,60 | arrow hit % on the enemy / on the Roman, by diff (10 arrows) |
| 98D0[16] | sb | -4,-8,-16,-32,0,32,16,8,4,8,16,32,0,-32,-16,-8 | fire pot lead divisor by heading difference |
| 98E2[16] / 98F2[3] | b | 5h / 0,80,160 | collide.lzw index: 80*type + 5*heading (+0..4 circles) |
| 98F6/9906/9916[16] | sb/b | arrow volley hit circle dx, dy, r by direction | D11B |
| 95D2[2] | b | 1, 2 | initiative bit: enemy 1, Roman 2 |
| 95D4/95E4[16] | sb | hook frame (0..7) left/right side by heading | BA7A |
| 95F4/9624, 9654/9684[48] | sb | hook offsets x,y by 16*type+heading, left / right side | BA7A |
| 96B4/96BC[8] | sb | 0,10,16,10,0,-10,-16,-10 / -14,-10,0,10,14,10,0,-10 | hook tip offset by hook frame |
| 9704[5] | b | 0, 13, 14, 15, 16 | music by Roman oar level (loaded 1..4 in BF50, freed in C34F) |
| 971A[4] | w | 280, 160, 40, 40 | start: Roman (280,160), enemy (40,40) |
| 9722[3] / 9728[3] | w/b | "Trireme","Quinquereme","Galleon" / 'T','Q','G' | flagship menu items + hotkeys |
| 9750 | menu | (110,70)-(210, 118 or 130), items 972C, title 96F0 "Choose a flag ship:" | C274 |
| 9762[6] / 9778[6x3] | w / sw | colours 140..135 / RGB | water palette cycle A (C1BB, mod 6) |
| 976E[5] / 979C[5x3] | w / sw | colours 132,103,133,191,134 / RGB | water palette cycle B (mod 5) |
| 980C[3] | ptr | "sml","med","lrg" | ship sheets |
| 9870[5] | ptr | "STOPPED","CRUISING SPEED","BATTLE SPEED","ATTACK SPEED","RAMMING SPEED" | panel speed name |
| 9926[2] | b | 8, 4 | D242 result when side s heads off the edge: enemy 8 (flees), Roman 4 (re-engage?) |
| 9928[4] | b | 4, 8, 12, 0 | "outward" heading for the D33D test |
| 9932[3] | 4sw | sml -28,-19,19,17; med -27,-21,21,20; lrg -28,-18,25,18 | sinking rect around the ship |
| 994A[3] | w | 22, 27, 24 | sinking animation frames by type |
| 9950 -> 992C | | "xsink", first char = 9952[type] 's','m','l' | ssink/msink/lsink sheets |
| 9958/9968[16] | sb | survivor drift dx, dy by heading | D3F2 |
| 99BC[12] | b | diff0 5,8,12; diff1 4,6,10; diff2 3,5,8; diff3 2,3,5 | per-ship hit % (-1) in the fleet battle; ram damage factor. Enemy uses row 2 = **99C2** |
| 99C8[12] | b | 2,4,6; 2,3,5; 1,2,4; 1,1,2 | the same when that side's flagship sank. Enemy: **99CE** = 1,2,4 |
| 99D4[4] | b | 8, 6, 4, 2 | Roman flagship bonus rolls by diff (the enemy always 4) |

Strings: 935A "The tides of battle sweep\nyour flag ships apart.\nDo you wish to re-engage?"; 96C4.. boarding texts (section 8);
8EAE.. outcome texts (section 10); 97BA "SHIPS", 98BA "MEN"; 9978 "Failed to Load Human Sinking", 9995 "Failed to Load
Computer Sinking".

## 3. B12A, the battle (D, literal)

```c
int B12A(int e, int r) {                       /* [bp+4] enemy fleet (3..6), [bp+6] Roman fleet (0..2) */
    di = 0; si = 0; flags = 0 /*[bp-0Ah]*/; mode = 0 /*[bp-8]*/;
    1D55(0);                                   /* cursor off */
    if (!BD8A(e, r)) goto out;                 /* set-up; returns 0 -> result 0 */
    res = 0 /*[bp-0Ch]*/;  W(C0FC)++;
    W(110E + 2*(6e + C37A))--;                 /* B174: the enemy flagship leaves its fleet for the duel */
    W(C37E) = B(8EDE + C37A);                  /* enemy crew */
    A = B(1117 + 12e);
    B(C36E) = (A != FF) ? B(C11E + 14A) : B(527A + 32*B(1115 + 12e));
    B(C36E) += B(8EE6 + diff);
    W(110E + 2*(6r + C37B))--;                 /* B1DE: the Roman flagship too */
    W(C380) = B(8EDE + C37B);                  /* Roman crew */
    L = B(1117 + 12r);
    if (L != FF) B(C36F) = B(1923 + 20L);      /* legion courage */
    else { W(C380) >>= 1; B(C36F) = B(527A + 32*B(1115 + 12r)); }   /* B20E: no marines: half crew */
    B(C36F) += B(8EEA + diff);
    BEAE(e, r);                                /* screen + panel */
    clip(185);                                 /* C0E0 = C1EA = 9AC6 = 0xB9 */
    for (;;) {                                 /* b248 */
        if (di) break;
        C382 = C4F8(C372, C374, C376, C378);   /* distance */
        C371 = C641(&C372, &C376); C370 = (C371 - 8) & 15;
        key = C366();                          /* [bp-2], player input */
        B6A0();                                /* enemy AI */
        C3D3(1); C3D3(0); C422(1); C422(0); B7FA(1); B7FA(0);
        if (mode == 0) mode = D242();          /* screen edges: 4 = flagships apart, 8 = enemy flees */
        CF7F(1); CF7F(0); CFDD();
        si = CD96();                           /* ram: initiative bits */
        CB11(1); CB11(0); C92B(1); C92B(0); C9A3(1); C9A3(0);
        if (since(8) > C386) { start(8); DA50(e, r); C386 = (rng(9) + 2) * 72; }   /* background fleet round */
        if (since(0) > 2) {                    /* redraw, every 3 ticks */
            timer0_reset(); fill(9, 0,0,319,185);
            B96A(); si |= BA7A(1); si |= BA7A(0); CCDE(1); CCDE(0); C71B(); DC8E(e, r); flip();
        }
        if (C38F == 0 || C38E == 0) { di = -1; continue; }        /* a flagship sank (exact 0, see 13.3) */
        if (si) { music_play(16); BB7E(si, e, r); di = -1; continue; }   /* boarding */
        if (key) { di = -1; mode = key; continue; }               /* 'r': mode 4 */
        if (mode & 4) {                                           /* b3b4 */
            music_play(13); 67B6(); 6837(); fill(9, 0,0,319,185); textstyle(A05C);
            if (!yes_no(935A)) { di = -1; continue; }             /* -> retreat */
            mode = 0; BE0F(); BEAE(e, r); continue;               /* re-engage: positions reset, hull/crew kept */
        }
        if (mode & 8) { di = -1; continue; }                      /* enemy admiral fled */
    }
    /* b419 */
    di = flags;  music_play(13);
    if (C38F == 0) { di = 1; C08E++; }         /* note: mov, not or */
    if (C38E == 0) { di |= 2; C08A++; }
    if (!(di & 1)) W(110E + 2*(6r + C37B))++;  /* surviving flagships rejoin their fleets */
    if (!(di & 2)) W(110E + 2*(6e + C37A))++;
    C37C = 0; C71B(); 67B6();
    D3F2(di); C312(); D60C(di); 67B6(); 6837();   /* sinking animations, free sheets */
    flags = di;
    if (mode == 8) C38E = 0;                   /* the admiral fled: his fleet fights as if he sank (he was re-added above) */
    if (C38F == 0 || C38E == 0) {
        while (!DA50(e, r)) ;                  /* fleet battle to the end */
        DC8E(e, r);
    }
    rs = sum W(110E+12r+2j); es = sum W(110E+12e+2j);
    si = res;                                  /* 0 */
    if (rs && es) si = 4;
    else {
        if (!rs) { B(1116+12r) = FF; if (B(1117+12r) != FF) legion_destroyed(B(1117+12r)); si = 1; }
        if (!es) { B(1116+12e) = FF; if ((A = B(1117+12e)) != FF) W(C11A + 14A) = 0; si |= 2; }
    }
    W(934A) = W(1100 + 2r); W(934E) = W(1100 + 2e); W(9352) = W(5222 + 2*B(1114 + 12e));
    B59F(si, mode, flags);                     /* outcome message */
    C34F();                                    /* music off/unload */
out: 1D8A(); return si;
}
```

## 4. Set-up (D)

- **BD8A(e, r):** music_stop; music_unload(1); `C37B = C274(r)`; FF -> return 0. For slot 0..2: C3BE[slot] = 0; if the enemy
  fleet has ships of that type, C37A = slot (so the enemy flagship is its biggest type, in practice its only type).
  C38F = 8EE2[C37B], C38E = 8EE2[C37A] (hulls). `if (!BF50()) return 0;` C105(); BE0F(); return -1.
- **C274(r) "Choose a flag ship:"**: one menu item per type with ships (text 9722[t], hotkey 9728[t]); 0 or 1 type -> no
  menu (1 type: that type; 0 types: an uninitialised local `[bp-8]`, only reachable with an empty Roman fleet, see 13.7).
  2 types -> box bottom 118, 3 -> 130. 5614(9750, 2); `i = 54C8(9750, 0)`; FF -> FF (cancel), else the i-th listed type.
- **BF50 graphics:** archive 9 (NAVL): VGA ([9B26] == 2) -> 0F0C "galleys.lzw" (the sea picture); sheets "oars1" (C3A6),
  "oars2" (C3A8). Archive 8 (MNAVAL): not VGA -> "galleys.cbm" picture via 1273/8E62/86A0; "bombs" (C3AE), "smok" (C3AC),
  ship sheet 980C[C37A] (C3A2) and 980C[C37B] (C3A4, or the same sheet); "collide.lzw" as a text member -> 5 tables
  (970A pointers, 9714 sizes 240,240,240,48,48); "flag" (C3BC). Any sheet missing -> returns 0. music_load(13..16),
  music_play(13).
- **C105 intro:** loop until a key (03AA) or a button ([B2]): VGA -> C1BB water palette cycling (it paces itself, every 4
  ticks: colours 9762[(a+i)%6] <- 9778[i], 976E[(b+i)%5] <- 979C[i], a = 0..5, b = 0..4); every 4 ticks fill
  (38,2)-(68,15) with colour 0x80 (VGA) / 0x0B and draw flag frame 0..3 (cycling) at (38,5), flip; at the end restore the
  screen and free the flag sheet. C18C (a static water palette) is never called (D).
  (Argument order here and below is the push order, which is the source order: the compiler uses the Pascal convention,
  callee pops; e.g. B335 `push 0; push 0; push 319; push 185; call 7E74` = fill (0,0)-(319,185).)
- **BE0F (also on re-engage):** headings enemy 4 (east), Roman 12 (west); oar frames 6; Roman at (280,160), enemy at (40,40);
  fractions 0; oar levels 1; speeds 9818[1 + 5*type]; all hooks/fire/pots/arrows/turn/oar requests 0; C384 = 1;
  C386 = 216. Hull and crew are NOT reset.
- **BEAE(e, r):** fill sea (0,0)-(319,185) colour 9; line y=186 colour 0; panel (0,187)-(319,199) colour [6346]; "SHIPS" centred
  at x 159, y 187; C71B; DC8E; draw both ships; save the screen into the work buffer; flip.

## 5. One iteration of the duel (D)

The loop is **not frame-limited**: every function runs once per pass; only the redraw is limited (3 ticks). Most functions
gate themselves by a timer slot (s*8 + n); B6A0 (AI dice), D242 and the collision test run on every pass (see 14).

**C366, the player's controls (side 1):** keypad/arrow bits [A2] (ISR table DS:00A6: Up 8, Down 4, Left 2, Right 1,
Home/PgUp/End/PgDn diagonals): Up -> C395 = +1 (oars faster), Down -> -1 (Down wins over Up), Left -> C397 = -1 (turn
port/anticlockwise), Right -> +1. Keys (03AA): `r` (0x72) -> returns 4 (immediate retreat, no question); Enter (0x0D) or `+`
(0x2B) -> C3AB = 20 (throw grappling hooks); Space (0x20) or Insert (0x10) -> C846(1) (fire pot). No mouse in the duel.

**C3D3(s) turning:** if since(8s+3) > 9828[type] (72/108/144): if C38A[s] != 0 && C396[s] != 0 { C388[s] = (C388+C396)&15;
start(8s+3); } C396[s] = 0. A stopped ship cannot turn.

**C422(s) oars:** if since(8s+4) > 982C[type]: if C394[s]: C38C += C394; if > 4: C38C = 4, C394 = 0; else if <= 0: if
oar frame C390[s] != 6 { C38C++; /* wait for the oars to come to rest, C394 kept */ } else { C38C = 0; C394 = 0; } else
{ if s == 1: C37C = 10; start(8s+4); C394 = 0; }. Then (whenever the interval has passed) C38A[s] = 9818[C38C + 5*type];
Roman: C38A != 0 ? music_play(9704[C38D]) (the rowing drum speeds up: 13..16) : music_stop.

**B7FA(s) movement:** if since(8s+2) > 93FE[C38A]: if C38A: oar frame++ (7 -> 0); step = 93F6[C38A];
`v = sin[h]*step + fx; fx = v & 0xFF; if (v < 0) fx = -fx; x += sign(v) * (|v| >> 8);` same for y with -cos; start(8s+2).
(The negative fraction is wrong, 13.6.)

**D242 screen edges** (returns the mode bits; its pushes keep the ships on screen): `dy1 = D38D(1), dy0 = D38D(0)` with
D38D(s) = y<15 ? 15-y : y>170 ? 170-y : 0. Both non-zero: same sign -> keep only one (`si >= di ? di = 0 : si = 0`);
opposite signs (one ship at the top edge, the other at the bottom) -> `m = D33D(dy1, 1, 1) | D33D(dy0, 1, 0)`. Then
Roman y += dy1, frac y += dy1<<8; enemy y += dy0, frac += dy0<<8; all four projectile y fractions += (dy0+dy1)<<8.
The same for x with D3BF (15 / 304) and D33D(.., 0, s). D33D(v, axis, s): target = v < 0 ? 9928[axis] : 992A[axis]
(y: 8 or 0; x: 4 or 12, i.e. the heading that leaves the screen); `d = |target - C388[s]|; return (4 <= d <= 12) ? 0 :
9926[s]`: the ship heading off its edge gives enemy 8 = "flees", Roman 4 = "swept apart, re-engage?". B12A tests 4 first.

**CF7F(s):** rect C3F0[s] = ship sprite rect (sheet C3A2[s] frame 14*heading: x - hotx, y - hoty, + w, + h).

**CFDD:** decides the rammer C384: if the bow point (bowX/bowY[16*type + heading] + pos) of C384 lies in the other ship
(D070), keep; else if the other's bow lies in C384, C384 = the other.

**CE7E collision:** rects C3F0[0], C3F0[1] overlap (817C) and any of the 5 Roman circles touches any of the 5 enemy circles
(05ED; circle k = (X, Y, R)[80*type + 5*heading + k], X/Y relative to the ship).

**CD96 ram** (only when CE7E): `R = C384, V = R^1; dh = |C388[R] - C388[V]|` (0..15);
`dmg = (uint8)(rng(16) + 99BC[C37A[R] + (R == 1 ? 3*diff : 6)] * C38C[R] - 8)`; if dh <= 2 or dh >= 14 (a scrape)
`dmg >>= 1` (unsigned byte shift); `C38E[V] -= dmg; C37E[V] = 050F(C37E[V], rng(dmg), 0)`; `bits = 95D2[R]`; if
6 <= dh <= 10 (head on): `C38E[R] -= dmg; C37E[R] = 050F(C37E[R], rng(dmg), 0); bits |= 95D2[V]`. Returns bits.
(The byte arithmetic wraps, 13.1.) A collision always ends the duel the same pass: a sinking (hull exactly 0) or boarding.

**C846(s) fire pot launch** (player key, or the AI when pointing at the Roman): if C392[s] == 0 && C3BA[s] == 0 &&
since(8s+5) > 144: start(8s+5); C392 = FF; C3CC = own heading (the pot flies straight ahead); pos = ship pos; frac = 0;
98BE = 0; `k = 98D0[|own heading - other heading|]; lead = k ? C382 / k : 0; if (C38A[other] < 4) lead /= 2;
C3C8[s] = 050F(C382, lead, 90)` (range, at least 90 px).

**CB11(s) fire pot flight** (if C392[s] && since(8s+1) > 6): splash (C392 == 1): 98BE++ until > 0x39, then 0. Flying:
98BE ^= 16; C3C8 -= 15; if > 0 { CC4D(15, s); off screen -> C392 = 0 } else { step 1 px up to C3C8+15 times testing
D070(other, y, x); hit: `dmg = other == 0 ? 98C0[diff] : 98C4[diff]; C38E[other] -= dmg;
C37E[other] = 050F(C37E[other], rng(2*dmg), 0); C3A0[other] = rng(50) + 10; C392 = 0;` miss: C392 = 1, 98BE = 0x33 }.
start(8s+1).

**CD4B(s) fire** (from the ship drawing, while C3A0[s]): C3A0--; every 16th frame (`(C3A0 & 15) == 0`) hull C38E[s]--;
flame frame 98E0 cycles 0..2 (bombs frames 0x30..0x32).

**C92B(s) arrows launch** (automatic, both sides): if C3BA[s] == 0 && C392[s] == 0 && C382 < 90 && crew C37E[s]:
C3BA = FF; C3CE = C370[s] (aimed at the other ship); pos = ship pos; frac 0; C3C4[s] = 050F(C382, 10, 7).

**C9A3(s) arrows flight** (if C3BA[s] && since(8s+1) > 8): range <= 0 -> C3BA = 0; else range -= 6, move 6 px; if now
range <= 0 and D11B(other) (volley circle 98F6/9906/9916[dir] touches a ship circle): `p = other == 0 ? 98C8[diff] :
98CC[diff]; kills = count of 10 x (rng(100) < p); C37E[other] = max(C37E[other] - kills, 0)`. Off screen -> C3BA = 0.
start(8s+1).

**BA7A(s) grappling hooks** (in the redraw; only side 1 ever has hooks): if C3AA[s]: C3AA--; `rel = (C388[s] - C370[s]) &
15`; 1..7 -> hook frame 95D4[h] + 0x3A, offset 95F4/9624[16*type + h]; 9..15 -> 95E4[h] + 0x3A, 9654/9684; 0 or 8 (bow or
stern towards the enemy) -> C3AA = 0, nothing. Draw the hook; tip = hook + 96B4/96BC[frame - 0x3A]; D070(other, tip) hit ->
return 95D2[s] (2 for the Roman) -> boarding. Hooks must be thrown broadside on.

**D070(s, y, x):** point in rect C3F0[s] and in one of ship s's 5 circles (05B0).

## 6. The enemy AI B6A0 (side 0) (D)

```c
weak  = (B(99C2 + C37A) * 4 >= C38F) ? -1 : 0;     /* Roman hull <= 12/20/32 (by enemy type) */
strong = (C36F * C380 > C36E * C37E) ? -1 : 0;      /* the Roman crew is stronger in melee */
h = C388;
if (C396 == 0) {
    rel = (h - C370) & 15; if (rel == 8) rel = rng(16);   /* pointing straight away: pick at random */
    t = 93A6[rel];                                        /* turn towards the Roman */
    if (rel == 0) C846(0);                                /* pointing at him: fire pot */
    if (t == 0) {                                         /* aligned: tie-break by geometry */
        a = 93C6[h] * (C374 - C378); b = 93B6[h] * (C372 - C376);
        if (b) a = |a|; if (a) b = |b|;                   /* sic: each abs depends on the other */
        if (a < b) t = 93D6[h]; else if (a > b) t = -93D6[h];
    }
    if (weak || strong || C382 > 95) C396 = t;            /* close and evenly matched: hold course (ram) */
    if (C393) C396 = rng(3) - 1;                          /* a Roman pot is in the air: dodge at random */
}
target = weak ? 3 : 2;                                    /* ATTACK SPEED when the Roman is weak */
C394 = (target < C38C) ? rng(3) - 1 : rng(2);             /* random walk of the oars */
if (strong && C382 < 85) BA7A(0);                         /* dead: C3AA[0] is never set, result dropped */
```
The enemy never throws hooks; it boards only by collision.

## 7. Panel and drawing (D)

- **C71B:** clip 199; hull <= 0 -> hull 0 and crew 0 (this is where a sunk ship loses its crew; 13.3); enemy hull bar red
  (colour 4) (0,190)-(115*hull/60,196), rest box colour to x 115; Roman bar from the right (319-115*hull/60 .. 319), rest
  from x 204; (119,193)-(199,199) box colour; text colour 0; if C37C == 0: "MEN" centred (159,193), Roman crew at x 174
  (left aligned), enemy crew right-aligned at x 144; else C37C-- and the Roman speed name 9870[C38D] centred. Clip 185.
- **DC8E(e, r):** C404 = Roman ships, C406 = enemy ships (all 3 types); print at (174,187) and right-aligned (144,187),
  width 25, colour [6346]/0.
- **B96A:** draws ship C384^1 then ship C384 (on top): oars (frame C390 + 93E6[h], oars1 for h < 8, oars2 - 0x310 bytes
  for h >= 8), hull frame 14*h of C3A2[s], fire (CD4B). **B8E3(s)**: one ship. **CCDE(s)**: pot (bombs frame 98BE,
  + C3CC while flying) and arrows (bombs frame 0x20 + dir).

## 8. Boarding BB7E(bits, e, r) (D)

```c
n = 20; key_flush(); C37C = 0; frame_wait(36);
if (bits & 2) C36E += 10;            /* value 2 = the ROMAN boarded (95D2[1]) -> +10 to the ENEMY's courage (13.2) */
if (bits & 1) C36F += 10;            /* value 1 = the enemy boarded -> +10 to the Roman's */
C390 = C391 = 6; C386 = 72; mid = midpoint of the ships;
while (!((C37E == 0 || C380 == 0) && n == 0)) {
    wait until since(2) > 8; start(2); if (n) n--;
    if (since(8) > C386) { start(8); C386 = rng(108);
        k_e = BD41(C36E, 0); k_r = BD41(C36F, 1);         /* simultaneous */
        C380 = 050F(C380, k_e, 0); C37E = 050F(C37E, k_r, 0); }
    fill sea; B8E3(1); B8E3(0); smoke frame rng(3) at mid; C71B(); flip();
}
if (C37E == 0) { C38E = 0; tail = 96CA; }                 /* "your axemen chop gaping holes in the enemy vessel..." */
if (C380 == 0) { C38F = 0; tail = C38E == 0 ? 96CC : 96C8; }   /* both sink together / "thrown overboard... scuttles" */
head = (bits & 2) ? 96C4 : 96C6;     /* "Your sailors cheer lustily as they swing across..." / "The enemy swarms across..." */
message_box(head + 96D0 "After fierce combat aboard both ships,\n" + tail); textstyle(A3B4);
```
BD41(courage, s): for each man of crew C37E[s]: `if (rng(100) < courage && rng(100) < 25) kills++` (the second roll only
when the first succeeds). A crew of 0 never ends with 96D2 unset: the loop only exits when one crew is 0.

## 9. The fleet battle DA50(e, r) (D)

```c
C400 = C402 = C404 = C406 = C408 = C40A = 0;
if ((L = B(1117 + 12r)) != FF) {                   /* the legion aboard fights */
    C408 = W(191E + 20L) + W(1920 + 20L);
    for (i = 0; i < C408; i += 500) if (rng(100) <= 5) C400++;
}
for (t = 0; t < 3; t++) {                          /* every Roman ship */
    n = W(110E + 12r + 2t); C404 += n;
    p = (C38F == 0 ? 99C8 : 99BC)[3*diff + t];
    for (i = 0; i < n; i++) if (rng(100) <= p) C400++;
}
if (C404) for (i = 0; i < 99D4[diff]; i++) if (rng(100) <= C38F / 4) C400++;   /* Roman flagship, 8/6/4/2 rolls */
if ((A = B(1117 + 12e)) != FF) {                   /* the soldiers aboard the enemy fleet */
    C40A = W(C116 + 14A) + W(C118 + 14A);
    for (i = 0; i < C40A; i += 500) if (rng(100) <= 5) C402++;
}
C406 = W(110E + 12e + 2*C37A);                     /* ONLY the flagship's type (all enemy fleets are one type) */
p = (C38E == 0 ? 99CE : 99C2)[C37A];               /* fixed row 2, whatever the difficulty */
for (i = 0; i < C406; i++) if (rng(100) <= p) C402++;
if (C406) for (i = 0; i < 4; i++) if (rng(100) <= C38E / 4) C402++;          /* enemy flagship, 4 rolls */
if (C404) DD2E(r);                                 /* Roman losses = C402 ships */
if (C406) DE51(e);                                 /* enemy losses = C400 ships */
return (sum(r) == 0 || sum(e) == 0) ? -1 : 0;
```
**DD2E(r) Roman losses:** for each type t: `x = (uint16)(n_t * C402) + C3BE[t]; q = x / C404; C3BE[t] = x % C404;
q = min(q, n_t); n_t -= q; lost += q` (the hits are spread over the types in proportion, remainders carried between rounds).
If lost: C08E += lost; if C408 (a legion aboard): `t = (uint16)(B(1118+12r) * lost); c = (uint16)(t * cav) / C408 >> 2;
c = min(c, cav); cav -= c; t -= c; t = min(t, inf); inf -= t; C086 += t + c`.
**DE51(e) enemy losses:** `q = min(C400, n); n -= q` on the flagship type only; if q: C08A += q; if C40A: weighted like DD2E
(`t = k*q`; +0x0A byte share `>> 3`, +4 share `>> 2`, rest from +6), C082 += all of it.

Both multiplications keep only the low 16 bits (`mul` then `sub dx,dx`), 13.5. With one-type enemy fleets the "flagship
type only" reading changes nothing; if a mixed enemy fleet ever existed, its other types would never fight nor sink and
DA50 could loop forever (no army aboard) (D, dormant).

## 10. Outcome text B59F(outcome, mode, flags) (D; strings in NAVAL_INDEX section 6)

Frame: `"<Roman fleet> met the <enemy fleet> on the gently\nrolling waves of <sea of the enemy fleet's zone>.\n\n"` + flavour
(flags 0 "", 1 8F52 your flagship sinks, 2 8EEE you destroy the enemy flagship, 3 8F96 both; mode 8 -> 8FF2 the admiral flees)
+ result: 1 lost: flags bit 0 ? 913C : 9220 ("witless commander"), mode 8 -> 90B0; 2 won (C0FE++): bit 0 ? 9104 ("their
admiral's bath") : 9198 ("Neptune's locker without a sword"), mode 8 -> 904C; 3 -> 92B2 ("together forever in a deadly
embrace"); 4 -> 9318 "You retreat to safety with your remaining ships.". Clip 199, text style A05C, message_box(AC60).

## 11. Sinking animations (D)

- **D3F2(flags)**: 0 -> nothing; 3 -> clear the sea; 1/2 -> sinking side k (bit 0 = Roman), survivor k^1: survivor oar frame
  6, sinker 3; turn direction +1, or -1 if the sinker's heading is 5..11; smoke at the midpoint. Every 14 ticks: fill; if the
  ship rects (D97C = hull + oars) still overlap: the survivor drifts by 9958/9968[h], oar frames 6,3,2,1,0,6,...; the
  sinker turns by the direction until heading 4 (east); done when heading 4 and no overlap; draw both, smoke frame rng(3),
  flip. Then the survivor alone, oars at rest.
- **D60C(flags)**: archive 8; flags & 1 -> sheet "<9952[C37B]>sink" (ssink/msink/lsink), failing -> box "Failed to Load
  Human Sinking"; flags & 2 -> the enemy's (the same sheet when the same type), "Failed to Load Computer Sinking"; "shark". Shark k
  starts at the sinking rect's left x, bottom + 4 (top - 2 if that is >= 185). Every 14 ticks for each sinking ship with
  frames left (994A[type] = 22/27/24): if the shark is not inside the OTHER ship's sinking rect, D89B(k) (erase, x++, frame
  AAC6[k]: 0..4, at 5 `rng(10) ? back to 2 : on to 5..7, then 0`); erase rect, draw sink frame i. Until both done.

## 12. Dice, in order (D) (23 call sites)

Per duel pass: **B6A0** [rng(16) if C396 == 0 and rel == 8] [rng(3) if C396 == 0 and C393] then always rng(3) or rng(2);
**CD96** on a collision: rng(16), rng(dmg) [, rng(dmg) head on]; **CB11(1), CB11(0)** on a pot hit: rng(2*dmg), rng(50);
**C9A3(1), C9A3(0)** on an arrow hit: 10 x rng(100); background round: **DA50** then rng(9). **BB7E**: per melee round
rng(108), BD41 enemy (1 or 2 rolls per man), BD41 Roman; per frame rng(3). **DA50**: legion 1/500 men, Roman ships type 0, 1,
2 (one each), Roman flagship 99D4[diff], enemy army 1/500, enemy ships, enemy flagship 4. **D3F2**: rng(3) per frame;
**D89B**: rng(10) when a shark frame reaches 5 (Roman shark first in each frame). 531F (EXE, before B12A for record 6):
rng(3).

## 13. 1990 bugs and quirks (fix by default, keep under --original, per house rule)

1. **Ram damage wraps (D, real bug).** CD96 computes the damage in a byte:
   `cde1: call rng(16) / ... mul byte [bx+C38C] / add cl,al / sub cl,8 / mov [bp-1],cl` then `ce0b: shr byte [bp-1]`.
   When rng(16) + factor*level < 8 the value is negative: a full hit then ADDS 1..8 to the victim's hull
   (`sub [si+C38E],al`) but kills rng(248..255) of its crew (`sub ah,ah` makes it unsigned), and a scrape (dh <= 2 or >= 14)
   turns 0xF8..0xFF into 124..127 hull damage: the flagship (hull 30..60) sinks on the spot. Chance per ram at oar level 1:
   Roman trireme 19% (Galley Slave) .. 37.5% (Emperor), enemy trireme 31%, quinquereme 19%; a stopped rammer (level 0) 50%.
   Fix: `int d = rng(16) + f*level - 8; if (d < 0) d = 0; if (scrape) d /= 2;` and call rng(d) as before (rng(0) still
   consumes a roll, so the dice count is unchanged).
2. **Boarding bonus to the wrong side (G, looks inverted).** `bb99: and al,2 / je / add byte [0xc36e],0xa` gives +10 to the
   ENEMY's courage when the Romans board (value 2 = 95D2[1]; the text for value 2 is "Your sailors cheer lustily as they
   swing across and board the enemy flagship"), and the enemy's boarding (value 1) helps the Romans. In a head-on ram both
   values are set and the bonuses cancel. Proposed fix: value 2 -> C36F, value 1 -> C36E. Ask Erik: it could be meant as a
   defender's advantage.
3. **Hull below zero is not "sunk" (D, rare).** B376 and B422/B436 test `== 0`; only C71B (in the redraw, every 3 ticks)
   clamps negative hulls to 0. A pot hit or ram in a pass without a redraw leaves e.g. -3, the duel goes on for up to 3
   ticks, and if it ends in that window ('r', D242) the flagship counts as afloat: no flag bit, and B45F gives it back to
   its fleet. Fix: test `<= 0` (signed) at B376, B37D, B422, B436.
4. **A sunk flagship still fires (D, minor).** DA50 rolls `rng(100) <= hull/4` with hull 0: 1% per roll (4 enemy rolls,
   99D4[diff] Roman rolls). Leave as is (it reads as "(hull/4+1)%" by design) unless Erik objects.
5. **16-bit casualty products (D).** DD2E `ddf4: mul si / mov si,ax / mul cx / sub dx,dx / div [0xc408]` and DE51 (DEE4,
   DEED, DF24) drop the high word: with a cavalry or consular legion aboard (k = men per ship up to 255) the cavalry/
   infantry split of the losses is garbage once k*lost*cav >= 65536 (the total lost is right). The ship-loss product
   n*C402 (DD71) cannot overflow in practice. Fix: 32-bit products.
6. **Negative fixed-point fractions (D).** B7FA, C9A3, CC4D: `mov ax,di / sub ah,ah / mov [frac],ax / or di,di / jge /
   neg [frac]` stores -(v & 0xFF) instead of -(|v| & 0xFF): moving west/north the ships, pots and arrows advance by other
   amounts than east/south (heading 14 at step 1: 0.5 px/step instead of 0.71). Fix: `frac = v < 0 ? -(-v & 0xFF) : v & 0xFF`.
   Low priority; changes nothing but paths.
7. **Edge cases (D):** an empty Roman fleet reaching B12A (two enemy fleets in one zone and outcome 3 against the first:
   B0E0 continues because bit 1 is set) makes C274 return an uninitialised local; 0x2C30 then writes the zone into the
   destroyed legion's +0x0C. An enemy record with no ships leaves C37A stale. 531F divides by zero for a tiny army.
   Suggest: B0E0 stops when the Roman fleet is gone (1116 == FF); log the others.
8. **Quirks to keep literally (D):** D242 moves a ship off the edge and also adds the push to its fraction (so it is pushed
   twice), and adds both ships' pushes to all projectile fractions; the AI tie-break's conditional abs; the fled admiral's
   ship rejoins its fleet (B45F before B493) and the fleet still fights with the "flagship sunk" odds; the enemy uses the
   Senator row whatever the difficulty; C36F/C36E bonuses never exceed 127.
9. **Not a bug, a design option for Erik:** if sinking the commander should decide more, the faithful knob is the enemy's
   per-ship chance after the sinking (99CE) or making the background rounds stop during the duel. Not proposed.

## 14. For the implementer

- Port B12A as a real-time loop like battle_play, headless via --script (keys: arrows/keypad, Enter or +, Space or Insert,
  r; screenshots). Never open a window.
- **Pacing:** the 1990 loop is unthrottled, so B6A0's rng(3)/rng(2) is consumed a CPU-speed-dependent number of times per
  tick (only the last C394 before C422's interval counts). Dice parity with DOSBox is impossible without modelling that;
  suggest one pass per 72.8 Hz tick (redraw every 3rd), documented as the port's pacing. Every timer-gated function keeps
  its own slot (s*8+1 pots/arrows shared, +2 move, +3 turn, +4 oars, +5 pot reload; slot 8 background rounds/melee,
  slot 2 melee frames, slot 0 redraw).
- The duel draws into the work buffer and flips (6887); the sea is colour 9; clip y 185 during the duel, 199 for the panel
  and texts.
- Sprites: bombs sheet frames 0..31 pots (dir + 16*anim), 0x20..0x2F arrows, 0x30..0x32 fire, 0x33..0x39 splash,
  0x3A..0x41 hooks; ship sheets 16 headings x 14-byte descriptors; oars1 (h 0..7) / oars2 (h 8..15, base - 0x310), frame
  C390 + 7*h; smok 3 frames; flag 4 frames; shark 8 frames; sink sheets 22/27/24 frames.
- collide.lzw: 5 signed-byte tables in one member (sizes 240, 240, 240, 48, 48) = circle X, Y, R (80 per type, 5 per
  heading) and bow X, Y (16 per type).
- Music: 13..16 = the rowing drum by oar level (CN9A..D, G), 16 also for boarding, 13 at the end and the re-engage box.
- Outcome effects outside B12A: B0E0/2C30 (the fleet enters the zone), 4ADA, End Turn 5221 + 5244 retreat (enemy.c).
- Casualty totals C082/C086 are the same longs battle.c adds to.

## 15. Easter eggs and jokes (report to Erik, keep all)

- "Alas, your ship duel was with the witless commander of the enemy fleet, and his more competent mates have swept the seas
  clean of Roman vessels." (lost, your flagship afloat).
- "In spite of their admiral's bath, your fleet prevails." (won after your own flagship sank).
- "...their leader is sent to Neptune's locker without a sword..."; "...the last two vessels sink, together forever in a
  deadly embrace."; "Such commanders lead these fleets, that Neptune calls them both to his underwater realm."; boarding:
  "Your sailors cheer lustily...", "the scurrilous enemy scuttles your flagship", "the two flagships sink together, bound
  eternally in Neptune's playground."
- "RAMMING SPEED" as the top oar level (the Ben-Hur galley line, G), with the rowing drum music speeding up per level.
- A shark circles each sinking flagship (and sometimes dives: rng(10) == 0).
- A Roman "Galleon" (the ship menu: Trireme, Quinquereme, Galleon), an anachronism.
- C18C, a static water palette, is never called.

## 16. Open questions

- The exact sprite-header fields used by CF7F/D919/D97C (+0 width, +2/+4 hot spot, +7 height) are inferred (G); check
  against the sheets when porting.
- Which key code 03AA returns for Insert (0x10 per BATTLE_UI_NOTES) and whether `R` (upper case) is ever seen (only 0x72 is
  tested).
- 13.2 (boarding bonus) needs Erik's ruling.
- Music ids 13..16 -> CN9A..CN9D is not checked against the music table.
