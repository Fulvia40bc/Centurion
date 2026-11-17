# Land battle (OVL0 section 2): structural map for the port

Source: `disasm/OVL_BATTLE.asm` (linear sweep, 0823:B0E0-E62F, code ends at E623, E624-E62F = zero padding), DS tables read
from `tools/dosbox/results/endturn/t10_in.bin`, EXE helpers from `disasm/EXE_RUNTIME.clean.asm` / `EXE_RUNTIME.bin`.
Notation: `[X]` = DS:X; negative displacements already converted (e.g. `[-0x3d6e]` = `[C292]`). U = unit record (0x12 bytes).
**V** = verified in the code; **G** = label guess / interpretation. Everything without a tag is V.

Linear-sweep junk: the lines `add byte ptr [di - 0x75], dl / in al, dx` at B54B, B847, BE3B, D0F1 are a `00` pad byte plus the
`55 8B EC` prologue: the real functions start at **B54C, B848, BE3C, D0F2**. D565 `jmp word ptr cs:[bx-0x2a96]` is a jump table at
**CS:D56A** (10 words: D57E D583 D590 D588 D57E D588 D590 D58D D58D D583); bytes D56A-D592 decode as junk. Real code there:
`D57E inc [bp-0xa]; jmp D590 | D583 dec [bp-0xa]; jmp D590 | D588 inc [bp-9]; jmp D590 | D58D dec [bp-9] | D590 mov al,[bp-8] ...`.

## 0. EXE helpers called (V unless noted)
| addr | what |
|---|---|
| 076B | roll(n) = 0..n-1 (dice) |
| 04FA / 050F | min(v+step,max) / max(v-step,min), signed; args pushed v, step, limit |
| 00D7(n) | start timer n (copy tick counter DS:42/44 to slot DS:46+4n) |
| 00F8 | = 00D7(0) |
| 0183(n) | ticks elapsed since timer n (0xFFFF if >65535); **01AE** = 0183(0) |
| 011F(n) | busy-wait until timer 0 + n ticks (frame limiter). Timer ISR (EXE 0000) runs at 4x18.2 = **72.8 Hz** (chains int 8 every 4th tick) |
| 03AA | non-blocking key read: ASCII or 0; Insert (scan 52h/9Bh) -> 0x10; extended keys ignored; Ctrl-S dropped |
| 04ED | flush keyboard buffer |
| [00A2] | numpad/arrow bitmask kept by the int 9 hook (EXE 042F): 1 right, 2 left, 4 down, 8 up (xlat table DS:00A6) |
| 0644 | poll mouse (int33 fn3) -> [B2] buttons, [B6] x, [B8] y; [BA] = mouse present |
| 0692(b) | int33 fn5 (button-press count, result [B4]) ; 066A/06E4/0709 set mouse pos / x range / y range |
| 358A | wait for a key; 359B wait for click or key (key -> [C07F]); 35DB wait until buttons released (G); 35F9 = 35DB,359B,35DB |
| 9DDC | toupper ; 9E32 strcpy ; 9DF2 strcat ; 9FDC itoa(v, buf, 10) |
| 6738(n) | number -> Roman-numeral string (buffer DS:C072, "OOPS" if > 999) (G: numeral; buffer V) |
| 7488(n,sz) | calloc -> **DS-near** pointer; on failure fatal error "calloc" (so the battle never sees 0) ; 74B7 = free |
| 4816(L) | legion destroyed: +0x0D=0xFF, +0x0C=0xFF, commander status -> 1 unless 2 |
| 4733(L) | assign a commander to legion L (garrison message "... is assigned to") |
| 54C8/5614 | menu run / draw (index or 0xFF on cancel); 59D4 message box; 8238 point-in-rect |
| 5D20, 5D29, 5D73, 5E1A | music stop / play / load / unload (G) ; 0F7B,11A9,11C4,1273,13A8,163B,198F,19A0 file+graphics loading (G) |
| 1D55/1D8A/1CCE/1D19 | mouse-cursor push/pop/show/hide ; 3134 box, 899C/8BE4/8C58/8C9F text, 8A9E/8AC8 numbers, 8E62/8790/86A0/86B2 blits, 97B6 line, 98BC circle, 0524 3x3 dot, 6837/6887/67E4/680F/691B/6973 page/screen buffers (all UI) |

## 1. Functions (start, length in bytes, callers). [S] = simulation logic, [UI] = graphics/UI/input
### Entry and setup
- **DF24** (758) entry, `ret 6` (army, legion, zone). [S] glue, see 3.1.
- **BE3C** (738, DF24) setup: border, graphics, enemy formation roll, formation menu, unit table, placement, enemy tactic roll, tactic menu, initial orders. Returns 0 = cancelled. [S+UI]
- **C11E** (53, BE3C) grid[x][y] = 0xFE if 0<=x,y<48 (border cell). [S]
- **BB97** (433, BE3C) load backgrounds (desert.cbm or battlenw.cbm+water.cbm by [A926]=prov[Z]+9) and sprite files; picks music/sprite set by enemy home zone. [UI]
- **C153** (415, BE3C) build the unit table (sizes, counts, allocations). [S]
- **C5EC** (297, C153) create units of one kind: (morale, side, facing, type, size, total). [S]
- **C2F2** (563, BE3C) place one side's units by formation layout. [S]
- **C715** (61, C2F2) pos = base +/- offset (mode 2 = add). [S]
- **C525** (199, BE3C) load 3-leg initial orders from the tactic, set state 1. [S]
- **BD48** (31) free sprite files ; **BB4D** (49) draw all units once (setup) [UI]
### Per tick
- **CDC7** (587, DF24) execute one step of the current order (move/retarget/flee/leave field); returns nonzero if it moved. [S]
- **CC7C** (331, DF24) turn toward the order direction (cav/ele/gen in 90-degree steps, infantry instantly). [S]
- **B0E0** (569, DF24) melee: attack the units in the front cells. [S]
- **B319** (236, B0E0) casualties of one hit (dice + difficulty). [S]
- **B405** (224, DF24) end of tick: commit casualties, morale loss, rout checks. [S]
- **B4E5** (103, B0E0) general killed: morale shock + rout checks for his side. [S]
- **B54C** (151, DF24) enemy AI: a holding enemy unit may switch to attack. [S]
- **B5E3** (475, CDC7/D198/D7E8) pick nearest enemy, set a half-way order toward it. [S]
- **B7BE** (138, B0E0/B405) effective morale (+ general's charisma bonus by distance). [S]
- **D012** (124, DF24) routed elephant runs amok (random new direction). [S]
- **D08E** (100, B0E0/B405/B4E5/D198/D77C) order "flee to own edge". [S]
- **B848** (221, DF24) action/animation state machine (die -> removed). [S] (timing matters)
- **B925** (40) set pending action unless dying. [S]
- **B94D** (223, DF24/BB4D) append sprite + advance animation frame. [S for the frame counter, UI otherwise]
- **E5B8** (108, DF24) end test. [S]
### Grid helpers (48x48 bytes at [C2AE], index x*48+y; 0 empty, 0xFE border, else unit index)
- **CA55** (231) stamp footprint (only into empty cells); any border cell -> state 4 + erase. [S]
- **CB3C** (178) erase footprint (cells holding this unit -> 0). [S]
- **C7E7** (169) footprint test: 0xFE border / 0xFF other unit / 0 free. [S]
- **C890** (184) footprint test vs enemy-side units only (routed movement). [S]
- **C948** (269) read the w cells along one edge (dir 0..3) into a buffer. [S]
- **CBEE** (142) order byte from unit to cell (dominant axis, Chebyshev distance). [S, used by UI]
- **C7B0** (55) grid -> screen: sx = (x-y)*8+0xA4, sy = (x+y)*4-0x4A (isometric). **C752** (94) screen -> grid. [UI]
### Result
- **E21A** (926, DF24) casualties, write-back, report screen. [S+UI]
### Player orders (all [UI] except the order effects)
- **D0F2** (166) per-tick input check, enters the modal orders mode. **D198** (325) orders menu loop. **D978** (119) map key/click to button.
- **D92D** (75) draw the 4 buttons. **D9EF** (70) show all Roman orders. **D33A** (1090) select a unit on the field and edit its orders.
- **D77C** (108) unit Retreat ("Run Away!"). **D7E8** (197) unit Melee ("Release Cohort N"). **D8AD** (128) unit within the general's voice?
- **D2DD** (93) unit near a cell (12 neighbour offsets DS:74BC). **DA35** (341) draw a unit's order path; **DB8A**/**DBBF** voice circle; **DBEB** (135) footprint centre on screen; **DC72** (690) unit info panel.
### Rendering
- **BA2C** clear sprite list; **BA4F** index array + sort; **BD67** quicksort by screen pos; **BA72** draw sprites; **BB7E** blit background. [UI]

## 2. Data
### 2.1 Unit record U (0x12 bytes, table at [C292], count [C294] incl. unused unit 0; loops run 1..count-1)
| off | type | meaning | written | read |
|---|---|---|---|---|
| +0 | u16 | strength (men; general = 5 "hit points") | C5EC, B405 (from C296), B288 (general -1) | everywhere |
| +2,+3 | s8,s8 | x, y on the grid (footprint origin) | C2F2, CDC7, CC7C | all |
| +4 | u16 | 0xFFFF = facing must be checked/turned (CC7C runs instead of CDC7); 0 = ok | C5EC, B24F, B79E, CC7C, CDC7, D012, D08E, D6F1 | DF24 E0D1 |
| +6,+7,+8 | u8 x3 | order queue, each `dir<<6 | steps` (dir 0 = x+1, 1 = y+1, 2 = x-1, 3 = y-1), 0x80 = none. During setup +6 temporarily holds the formation slot (0xFE = elephant) | C5EC, C2F2, C525, B5E3, CDC7 (shift +7->+6->...), D012 (+7), D08E, UI | CDC7, CC7C |
| +9 | u8 | animation id (tables DS:6DA6/6B28/6BD0) | C5EC (7244[type*2+side]), B848 | B94D |
| +0x0A | u8 | type: 0 cavalry, 1 elephants, 2 infantry, 3 general | C5EC | all |
| +0x0B | u8 | side: 0 Roman, 1 enemy | C5EC | all |
| +0x0C | s8 | morale (Roman = legion +0x0B, enemy = [C2A4]) | C5EC, B405, B4E5 | B7BE, B54C, DC72 |
| +0x0D | u8 | state: 0 created, 1 holding/following orders, 2 attacking (auto-target), 3 routed (never rallies, V: no write 3->other), 4 gone (dead or left the field) | see 2.4 | all |
| +0x0E | u8 | facing 0..3 (same coding as dir); Romans start 3, enemy 1 | C5EC, CC7C | all |
| +0x0F | u8 | animation frame; unit logic runs only when it is 0 | C5EC, B94D | DF24 E0CB |
| +0x10 | u8 | current action (0 idle, 1 move, 2 turn (bits 4-5 = old facing), 3 attack, 4 die) | B848 | B848, B210, CDC7 |
| +0x11 | u8 | pending action (B925); after use replaced by 724C[action] (= 0, or 4 for die) | B925, B848 | B848 |
Footprint: w = 6AE8[type] = {2,2,2,1}, d = 6AEC[type] = {2,3,2,2}; w along x and d along y when facing 1/3, swapped when facing 0/2
(CA55/CB3C/C7E7/C890). C948 uses w as the edge length and d as the front offset **without** the swap.
Grid stores unit indices as bytes: unit indices must stay < 0xFE.

### 2.2 Battle globals
| addr | meaning | written | read |
|---|---|---|---|
| C28C b | commander index (legion +0x10) -> commander table DS:1A1C (6 B: +0 name ptr, +2 charisma, +3 voice, +4 status 0 assigned / 1 free / 2 dead) | DF43 | B2BA B7DC D8B7 DBA5 DDFF E44E E590 |
| C28E w | (legion +0x0F == 0); indexed by **attacker** side in B1D7 | E00F | B1D7 |
| C290 w | same for side 1; set 0 when the enemy has elephants | E01E | B1D7 |
| C292 w | unit table ptr ; C294 w unit count ; C296 w ptr to u16[count] strength-after-this-tick | C20C/C200/C225 | all |
| C298 b | index of the Roman general unit | C276 | B7FD D8DB |
| C29A w | enemy home zone (army +2) or the zone itself (province fight) | DF87/DFA8 | report, E1B9, E357, BB97 |
| C29C w / C29E w | enemy cavalry / infantry | DF6C DF65 / copy, E323 E32F | C153 E309-E311 |
| C2A0 w | enemy active (army +8); province fight = 0xFFFF; 0 after a Roman win | DF8A, E33B | DFAA |
| C2A2 b | enemy elephants (count of animals) | DF73, E329 | C196 C28E E012 E2FB |
| C2A3 b | enemy "discipline" for the AI charge test (prov/army +0x0B) (G name) | DF7A | B5C4 |
| C2A4 b | enemy morale (prov/army +0x0A; names DS:7486 Panicky/Weak/Good/Fierce = value/25) | DF81 | C29D |
| C2A5, C2A8 | copied (army +0x0D) / unused | | |
| C2A6/C2A7 b | music ids: 9/0x0A if enemy home zone in 11-14 or 23-26 ("barbarian" sprites 'b'), else 0x0B/0x0C (G) | BCBF-BCD0 | |
| C2A9 b | enemy formation 0..4 ; C2AA player formation 0..3 ; C2AB enemy tactic 0..4 ; C2AC player tactic 0..4 | BE44/BF7F ; BFA2 ; C053/C064/C085 ; C0B9 | C2F2/C525 calls, report E417-E445 |
| C2AD b | enemy tactic flag (tactic record +8): 1 = charge after 10 idle ticks | C0A1 | E163 |
| C2AE w | grid ptr (calloc 0x900) | DFE4 | grid helpers |
| C2B0 w[2] | per side: result of the last CDC7 of that side this tick (-1 moved, 0 blocked) | CFF1 | E146 |
| AA4A w | unit creation counter (starts 1) | C238 C702 | C5EC |
| AA4C b | end-test counter (never reset after DFE7) | DFE7 E60D | E611 |
| AA4E b[2] | per side: consecutive ticks with C2B0[side]==0 | E02D E14B E151 | E16A (side 1 only) |
| A926 b | prov[Z]+9 (1 = desert background) | BBAA | BB88 |
| A928 w / A92A..AA01 / AA02.. | sprite count / 36 sprite records of 6 B (xy, sprite id (+0xBF enemy), frame, anim) / u16 sort index | BA2C B94D BA4F | BA72 |
| 73FE w / 7400 w | enemy / Roman unit counts | C1A5-C1CC / C1E2 C1F9 | BF4F |
| 76EA w | tactics.lzw buffer | BEF1 | C597 |
| C07F b | last key | D143 D1D2 D3BD | D1C8 D978 |
| C0F8 w ++ | battles fought | DF46 | |
| C0FA w ++ | battles won | E46D | |
| C086:C088 / C082:C084 | 32-bit totals: Roman casualties / enemy casualties | E575 / E583 | |

### 2.3 Static tables (DS, verified values from t10_in.bin)
- 6AF0 defence factor f[atype*3+ttype][rel facing], rel = (attacker.facing - target.facing)&3 (0 = from behind, 2 = head on):
  cav->cav 4 8 16 8 | cav->ele 32 32 64 32 | cav->inf 4 8 16 8 | ele->cav 8 16 64 16 | ele->ele 16 16 16 16 | ele->inf 2 8 16 8 |
  inf->cav 8 16 64 16 | inf->ele 8 32 64 32 | inf->inf 4 8 16 8.  6B14 men per attack die by attacker type: 15, 2, 50, (0).
- 6B18 AI thresholds by alive-unit count n (both sides): n 1-4: 25 20 15 10 | 5-8: 20 15 10 5 | 9-12: 15 10 5 0 | 13+: 10 5 0 0 (by morale/25).
- 7402 Roman infantry unit size floor by difficulty: 500, 400, 300, 200.
- 76EC formation layouts (5 x 0x2C at 7580): +0 name ptr ("Standard","Wedge","Strong Right","Strong Left","Massed Army"),
  +2 17 slot offsets (s8 dx,dy), +0x24 base side 0, +0x26 base side 1, +0x28 4 cavalry slot numbers. Slot 17 reads +0x24 (overlap).
  E.g. Standard: base0 (9,28), base1 (34,16), cav slots 14 15 8 9, slot 16 (general) = (12,7).
- 73DA elephant x [formation*6+k], 73F8 elephant y [formation] = 18 19 21 21 18.
- 76F6 tactic records [formation 0..4][tactic 0..4] x 10 B: +0 name ptr, +2/+4/+6 offsets of 17-byte order lists in tactics.lzw
  (0xFFFF = none), +8 charge flag. Names: f0 Frontal Assault / Scipio's Defense / Drive a Wedge / Outflank / Stand Fast;
  f1 Frontal Assault / Cannae Tactic / Drive a Wedge / Form a Line / Stand Fast; f2,f3 Frontal Assault / Sweep Right / Sweep Left /
  Mass Troops / Stand Fast; f4 (enemy only) Frontal Assault / Sweep Right / Sweep Left / Encircle / Stand Fast. Flag 0 for
  Scipio's Defense, Form a Line, every Stand Fast; 1 otherwise. Offsets run 0x00..0x264 step 0x11.
  tactics.lzw is member TACTICS.LZW of BATL.DAT; a decoded copy appears to survive in t10_in.bin at DS:F356 (first list = 15 x 0xCE,
  0xCD, 0x80 = "all forward 14") - G, compare after decoding the file.
- 77F0 enemy formation weights, row (prov[C29A]+0x16 style 0..5)*4 + size, size = (73FE-2)>>2, 5 weights (formations 0..4).
- 7986 u16[style*5 + enemyFormation] -> 26-byte block, row [playerFormation] = 5 tactic weights; 0 ptr -> uniform roll(4).
- 6C24 anim id [action][type][facing] (16 B/action); 6BD0 anim length; 6B28 sprite base; 6DA6 frame lists; 724C next action.
  Cycle lengths = ticks between decisions: idle cav/inf/gen 1, ele 3; move cav 5, ele 8, inf 12, gen 5; turn cav 6, ele 7, gen 5
  (inf turns instantly, CC7C); attack cav 4, ele 3, inf 4, gen 3; die cav 10, ele 10, inf 5, gen 26.
- 740A/741A position shift when ele/general turn [oldfacing*4+newfacing]; 7244 initial anim [type*2+side].
- Strings: see 6.

### 2.4 Unit state transitions (all writes of +0x0D)
0 (C680 create) -> 1 (C56A, every unit of a side after C525) -> 2 (B5D7 AI, E18F mass charge, D27B/D87D player melee) ->
1 (D265/D2C0/D6FA/D86C player) ; any -> 3 (B209 dead code, B4A7, B531) ; any -> 4 (B86C die anim done, CB2C stamped on border,
CFDB reached y==1 or y==0x2C).

## 3. Control flow (pseudo-C)
### 3.1 Entry DF24(A = army or 0xFF, L = legion 0..12, Z = zone)
```c
music_stop_5D20(); C28C = leg[L].f10; C0F8++; cursor_push(0);
if (A == 0xFF) { P = prov[Z]; C29E = P.w0; C29C = P.w2; C2A2 = P.b0C; C2A3 = P.b0B; C2A4 = P.b0A; C29A = Z; C2A0 = 0xFFFF; }
else memcpy(&C29A, &army[A].w2, 12);   /* C29A=+2 home, C29C=+4, C29E=+6, C2A0=+8, C2A2=+0A, C2A3=+0B, C2A4=+0C, C2A5=+0D */
if (C2A0 == 0) { cursor_pop(); return -1; }            /* inactive army: "destroyed" */
mouse ranges 0..319 / 0..199, pos (159,99);
C2AE = calloc(1, 0x900); AA4C = 0;
if (!BE3C(L, Z)) { ret = 0; goto cleanup; }           /* Esc in a menu: no casualties, nothing written back */
C28E = (leg[L].f0F == 0); if (C2A2) { leg[L].f0F = 0xFF; C290 = 0; }
AA4E[0..1] = 0; C2B0[0..1] = 0; clear mouse presses;
if ([3EE9] == 0xFF) { timer_start(2); music(C2A6); while (elapsed(2) < 432) ; }   /* ~5.9 s fanfare (G) */
music(C2A7);
for (tick = 0; !E5B8(); tick++) {                       /* main loop E07D-E19C */
  sprites_clear(); timer_start(0); mouse_poll();
  for (i = 1; i < n; i++) C296[i] = U[i].str;
  for (i = 1; i < n; i++) {
    if (U[i].state == 4) continue;
    if (U[i].frame == 0) {
      if (U[i].turn == 0) {
        if (!CDC7(i)) B0E0(i);
        if (U[i].state == 3 && U[i].type == 1) D012(i);
      } else CC7C(i);
      if (U[i].side == 1) B54C(tick, i);
      B848(i);
    }
    B94D(1, i);                                          /* sprite + frame++ (wraps at 6BD0[anim]) */
  }
  B405(); BA4F(); BB7E(); BA72(); page_flip();
  wait_ticks(12);                                        /* 12/72.8 s = 0.165 s per tick */
  D0F2();                                                /* player may open the (modal) orders mode */
  for (s = 0; s < 2; s++) { AA4E[s] = C2B0[s] ? 0 : AA4E[s] + 1; C2B0[s] = 0; }
  if (C2AD == 1 && AA4E[1] >= 10) for each enemy unit with state 1: state = 2;
}
free_sprites(); restore_screen(); ret = E21A(L, Z);
if (C29A != Z) memcpy(&army[A].w2, &C29A, 12);          /* army fight: strengths, elephants, active flag back */
else if (!ret) { prov[Z].w2 = C29C; prov[Z].b0C = C2A2; prov[Z].w0 = C29E; }   /* province (or army with home == Z!) */
cleanup: music_stop(); unload(C2A6); unload(C2A7); free(C2AE); cursor_pop(); return ret;   /* -1 = Roman victory */
```
Note (V): an army whose home zone (+2) equals the battle zone takes the province branch: its record is not written back and on a
Roman loss its survivors overwrite prov[Z] +0/+2/+0x0C. The callers (EXE 4B85, 5056) clear the army's +8 themselves on a win.
### 3.2 Setup BE3C(L, Z)
```c
C2A9 = 0xFF; mark border: for s<19 C11E(18-s, s); for s<24 { C11E(s, s+23); C11E(43-s, 23-s); } for s<20 C11E(24+s, 47-s);
  C11E(0,19..21); C11E(19,0);        /* diamond field: x+y>=18, x-y<=20, y-x<=23, x+y<=71 (grid x major) */
if (!BB97(Z)) { free_sprites(); return 0; }
load music C2A6, C2A7; [76EA] = load("tactics.lzw"); draw field; draw menu 73B6 "Choose Formation";
if (!C153(L)) { cleanup; return 0; }
style = prov[C29A].b16; size = (enemyUnits_73FE - 2) >> 2;
r = roll(100); sum = 0; do { C2A9++; sum += W77F0[(style*4+size)*5 + C2A9]; } while (sum <= r);   /* dice #7 */
C2AA = menu(73B6);  if (C2AA == 0xFF) { free everything; return 0; }   /* Balanced Army/Wedge/Strong Right/Strong Left (B,W,R,L) */
C2F2(C2AA, 0, 2); C2F2(C2A9, 1, 0); draw;
for (i = 0; i < 4; i++) tacticMenu.item[i].text = T76F6[C2AA][i].name;   /* 5th item fixed "Stand Fast"; hotkeys 1-5 */
p = W7986[style*5 + C2A9];
if (p) { r = roll(100); sum = 0; C2AB = 0xFF; do { C2AB++; sum += p[C2AA*5 + C2AB]; } while (sum <= r); }  /* dice #8 */
else C2AB = roll(4);                                                                                /* dice #9 */
C2AD = T76F6[C2A9][C2AB].flag;
C2AC = menu(73C8 "Choose Tactic"); if (C2AC == 0xFF) goto cancel;
C525(0, &T76F6[C2AA][C2AC]); C525(1, &T76F6[C2A9][C2AB]); free tactics buffer; return -1;
```
RNG order: enemy formation is rolled before the player picks a formation; the enemy tactic after the formation pick (it depends on
it), before the tactic menu. Style = prov[enemy home]+0x16 (values 0..5 in the snapshot). size >= 4 (18+ enemy units) reads the
next style's row (no clamp).
### 3.3 Unit table C153(L) and C5EC
```c
leg = &L1918[L];
si = max((leg.inf_w6 + 11) / 12, W7402[diff]);  if (si > 500) si = 500;      /* Roman infantry unit size */
73FE = (C2A2 + 19)/20 + 1 + (C29C + 149)/150 + (C29E + 499)/500;            /* enemy units incl. general */
7400 = (leg.cav_w8 + 149)/150 + 1 + (leg.inf_w6 + si - 1)/si;               /* Roman units incl. general */
C294 = 7400 + 73FE + 1; C292 = calloc(C294, 0x12); C296 = calloc(C294, 2); AA4A = 1;
C5EC(m=leg.b0B, side 0, facing 3, type 0, 150, leg.cav);    /* Roman cavalry          */
C5EC(m,         0,      3,        2,      si,  leg.inf);    /* Roman infantry         */
C298 = AA4A; C5EC(m, 0, 3, 3, 5, 5);                        /* Roman general, str 5   */
C5EC(C2A4, 1, 1, 1, 20,  C2A2);                             /* elephants, 20 a unit   */
C5EC(C2A4, 1, 1, 0, 150, C29C);  C5EC(C2A4, 1, 1, 2, 500, C29E);  C5EC(C2A4, 1, 1, 3, 5, 5);
/* C5EC: while (total > 0) { u = U[AA4A++]; u.side, type, facing; u.turn = 0xFFFF; u.str = min(size, total); total -= size;
   u.morale = m; state = 0; frame = 0; anim = 7244[type*2+side]; act = pend = 0; orders +6..+8 = 0x80; } (x,y stay 0) */
```
Garrison (L = 12, DS:1A08): the EXE caller 50E3 copies prov[0]+0 -> 1A0E (inf) and +2 -> 1A10 (cav) before, and back after a win.
### 3.4 Placement C2F2(formation f, side, mode) (mode 2 = Roman: pos = base0 + off; 0 = enemy: pos = base1 - off)
```c
lay = W76EC[f]; base = lay.base[side];
if (side == 1) { k = 0; for each unit of type 1 (side 1): x = B73DA[f*6+k], y = B73F8[f]; CA55; slot(+6) = 0xFE; k++; }  /* k>=6 overflows */
for (c = 0; c < 4; c++) { next unplaced (x==y==0) cavalry unit of side: pos = base +/- lay.off[lay.cav[c]];
                          if (side) { x--; y--; } slot = lay.cav[c]; CA55; }
k = 0; for each unit of side with +6 == 0x80, in index order:
  for (;;) { if (k > 17) break /* unit stays at (0,0), unplaced - checked before the type, so also for the general */;
             s = (type == 3) ? 16 : k++; pos = base +/- lay.off[s];
             if (side) { x += 1 - w[type]; if (type != 2) y--; }
             if (C7E7(i) == 0 || type == 3) { slot = s; CA55(i); break; } }   /* occupied slot -> next slot */
```
### 3.5 Initial orders C525(side, tac)
For each unit of side: slot = +6; +6 = 0x80; if slot == 0xFE (elephant) +6 = 0x4B (dir 1, 11 steps); else for q in 0..2:
`b = tac.list[q] == 0xFFFF ? 0x80 : tactics[tac.list[q] + slot]; if (side) dir = (dir+2)&3; +6+q = b;` then state = 1.
### 3.6 Movement CDC7(i) (returns nonzero = moved, so no attack this tick)
```c
if (!str || state == 4 || act == 4 || (act & 15) == 2) return 0;
o = +6; if (o == 0x80 || (o & 63) == 0) { if (state == 2) { B5E3(i); return -1; } return 0; }
save pos; erase; step 1 cell in dir; steps--; B925(i, 1);
if (state == 3 && type != 1) { if (C890(u) == 0xFF) { steps++; restore; B925(i,0); moved = 0; } else moved = -1; }  /* routers pass friends */
else if (C7E7(i,u) == 0xFF) { steps++; restore;
     enemyAhead = any of 2 cells of C948(facing) holds an enemy;
     if (!enemyAhead && state == 2) { B925(i,0); +6 = 0x80; B5E3(i); reload o; }     /* blocked by friend: retarget */
     else B925(i,0);  moved = 0; }
else moved = -1;                                           /* also when the new footprint touches the border (CA55 kills it) */
if (steps == 0) { turn = 0xFFFF; if (state == 2) { B5E3(i); moved = -1; } else { +6 = +7; +7 = +8; +8 = 0x80; } }
else +6 = dir<<6 | steps;
CA55(i); if (y == 1 || y == 44) { state = 4; erase; }     /* left the field: keeps its strength (= survivor) */
C2B0[side] = moved; return moved;
```
CC7C(i) turning: if no steps: turn = 0. Else erase; dir = +6>>6; if dir == facing turn = 0; infantry: facing = dir, turn = 0,
B925(i,0). Others: new = (|dir-facing| == 2) ? (dir+facing)/2 : dir; ele/general shift pos by 740A/741A[old*4+new];
if C7E7 == 0xFF -> revert, B925(i,0); else { if new == dir turn = 0; B925(i, old<<4 | 2); } ; CA55.
### 3.7 Melee B0E0(i) and B319
```c
u = U[i]; if (!u.str) return; if (u.type != 1 && u.state == 3) return; if (u.state == 4) return;   /* routed elephants still fight */
C948(buf, u.facing, i);
for (k = 0; k < w[u.type]; k++) { t = buf[k]; if (t == 0 || t >= 0xFE) continue;
  if (U[t].side == u.side && u.type != 1) continue;       /* elephants also trample their own side */
  B925(i, 3);
  if (u.type == 1 && C28E[u.side] && roll(100) > max(B7BE(t) - 25, 0)) { U[t].state = 3; D08E(t); }   /* dice #1, DEAD CODE: */
      /* indexed by the elephant's own side = C290, which E01E zeroes whenever elephants exist */
  if (U[t].act == 0 && u.state < 3 && (nf = (u.facing+2)&3) != U[t].facing) { U[t].turn = 0xFFFF; U[t].o6 = nf<<6 | 1; }
  v = C296[t];
  if (U[t].type == 3) { if (--U[t].str == 0) { B925(t,4); B4E5(U[t].side); if (U[t].side == 0) cmdr[C28C].status = 2; } }
  else { v -= (u.type == 3) ? 0 : B319(t, i); if (v < 0) v = 0; }
  C296[t] = v; }
```
A target covering 2 front cells is hit twice per attack (no de-duplication). The general deals no damage; hitting a general removes
exactly 1 of his 5 points.
```c
B319(t, a): f = 6AF0[(a.type*3 + t.type)*4 + ((a.facing - t.facing)&3)];
  switch (diff [048E]) { case 0: if (t.side == 0) f = f*3/2; break;   /* Galley Slave: Romans harder to hurt */
                         case 2: if (t.side == 1) f = f*5/4; break;   /* Senator: enemy tougher */
                         case 3: if (t.side == 1) f = f*3/2; break; } /* Emperor; Legionnaire = no change. f kept as a byte */
  maxd = 64 / f; n = a.str / 6B14[a.type] + 1; loss = 0;
  while (n--) loss += roll(maxd + 1);                                 /* dice #2 */
  if (t.type != 1 && loss < 1) loss = 1; return loss;                 /* elephants can take 0 */
```
### 3.8 End of tick B405, B4E5, B54C, B5E3, B7BE, D012, D08E
```c
B405: for i: if (!str || type == 3) continue; loss = str - C296[i];
  if (loss*16 > str) { morale = (s8)(morale - (u8)(loss/2)); if (morale < 0) morale = 0;     /* u8 truncation of loss/2 */
                       if (morale < 50 && roll(100) > 2*B7BE(i)) { state = 3; D08E(i); } }    /* dice #3 */
  else if (type == 1 && loss && roll(100) > 75) { state = 3; D08E(i); }                       /* dice #4: elephants 24% */
  str = C296[i]; if (!str) B925(i, 4);
B4E5(side): every unit of side with state != 4: morale = max(morale-15, 0); if (morale < roll(100)) { state = 3; D08E(i); }  /* dice #5 */
B54C(tick, i): if (tick <= 15) return; n = min(#units with state != 4 (both sides), 13);
  if (state == 1) { r = roll(100); thr = 6B18[((n-1)&~3) + morale/25];                          /* dice #6 */
                    if ((u16)(C2A3 - thr) < r) state = 2; }   /* C2A3 < thr wraps -> never charges */
B5E3(i): nearest (dx^2+dy^2) unit of the other side with state not 3/4 (tie -> prefer a general);
  none -> +6 = 0x80; else dx = u.x - t.x, dy = u.y - t.y; legs a = (dx>0?2:0)<<6 | (|dx|+1)/2, b = (dy>0?3:1)<<6 | (|dy|+1)/2;
  if (dy > dx /* signed, not abs */) swap(a, b);
  if (a & 63) { +6 = a; if either of 2 cells of C948(a>>6) holds a friend: +6 = b; } else +6 = b;  turn = 0xFFFF;
B7BE(i): m = morale; if (cmdr[C28C].status == 0 && side == 0) m += max(0, charisma - |dx| - |dy|) (distance to unit C298); return (u8)m;
D012(i) routed elephant: erase; d = roll(3) ? facing : roll(4);  if (d != facing) turn = 0xFFFF; +7 = d<<6 | 2; CA55.  /* dice #10,#11 */
D08E(i) flee: if state != 4: dir = side ? 3 : 1; steps = side ? y : 44 - y; +6 = dir<<6 | steps; if dir != facing turn = 0xFFFF.
B848(i): if (act == 4) { state = 4; erase; return; } a = pend & 15; anim = 6C24[a][type][a==2 ? (pend>>4)&3 : facing]
  (+1 if a == 2 and facing is 1 or 2); act = pend; pend = 724C[pend & 15].
```
### 3.9 End test E5B8 (also evaluated once before the first tick)
```c
rom = ene = 0; for i: if (!str || type == 3 || state == 4 || state == 3) continue; if (side == 0) rom = 1; if (side == 1) ene = 1;
if (!rom || !ene) if (++AA4C > 30) return 1;   /* AA4C is cumulative, never reset */
return 0;
```
### 3.10 Result E21A(L, Z) - returns won (-1/0)
```c
won = 0; rc = ri = ec = ee = ei = 0;
for i: if (!str) continue;
  if (side == 0) { if (state != 3 && state != 4 && type != 3) won = -1; if (type == 0) rc += str; if (type == 2) ri += str; }
  if (side == 1) { type 0: ec += str; type 1: ee += str; type 2: ei += str; }   /* routed / fled units count as survivors */
romanCas = leg.inf + leg.cav - ri - rc;  romanSurv = ri + rc;
leg.cav(+8) = rc; leg.inf(+6) = ri; free(C296); free(C292);
enemyCas = C2A2 - ei - ee - ec + C29E + C29C;     /* old elephants counted in casualties ... */
enemySurv = ei + ec;                              /* ... but not in survivors */
C29C = ec; C2A2 = (u8)ee; C29E = ei;
if (won) { C2A0 = 0; leg.b0B = (leg.b0B > 94) ? 99 : leg.b0B + 5;
           if (Z == C29A) { prov[Z].w4 = enemySurv; if (prov[Z].w6 < prov[Z].w4) prov[Z].w4 = prov[Z].w6; }
           for (k = 0; k < 8; k++) { n = adj[Z*8+k] /* DS:50EC */; if (n < 27) prov[n].b0D = min(prov[n].b0D + 25, 100); } }
else if (!leg.cav && !leg.inf) legion_destroyed_4816(L);
else leg.b0B = max(leg.b0B - 5, 0);
Roman_numeral_6738(leg.b0A);                      /* -> DS:C072, used in the report text */
report: title = won ? "A Glorious Victory!" : "An Ignominious Defeat"; text = 7ADE..7B04 list (see 6);
  numbers romanCas, enemyCas, romanSurv, enemySurv; if (won) C0FA++;
C086:C088 += romanCas; C082:C084 += enemyCas; wait key/click;
if (cmdr[C28C].status == 2 && leg.b0D != 0xFF) assign_commander_4733(L);
return won;
```
"won" only needs one Roman non-general unit not routed and not gone at the end; the battle only ends when one side has none, so
won == enemy has no active units. Enemy survivors may remain (written back) but the army is deactivated (C2A0 = 0).

## 4. Dice (0x076B) sites and difficulty
| site | n | decides |
|---|---|---|
| B1F6 | 100 | elephant terror: target routs if roll > effMorale-25. Unreachable (flag indexed by the elephant's side, always 0) |
| B3D6 | 64/f + 1 | casualties per die; dice count = attacker.str / {15,2,50}[type] + 1 |
| B47C | 100 | unit lost > 1/16 this tick and morale < 50: routs if roll > 2*effMorale |
| B49B | 100 | elephant unit hit lightly (loss*16 <= str, loss > 0): routs if roll > 75 |
| B525 | 100 | general killed: each unit of his side (morale-15) routs if morale < roll |
| B5A6 | 100 | enemy AI, holding unit after tick 15: charges if (C2A3 - thr) < roll (unsigned) |
| BF5F | 100 | enemy formation (weights 77F0) |
| C04B | 100 | enemy tactic (weights via 7986, row = player formation) |
| C082 | 4 | enemy tactic when the weight pointer is 0 (style 1 with formation 0/1, style 2 with 2/3: those weights are 0, so only reachable when the size class overflows into the next style's row, i.e. 18+ enemy units) |
| D040 | 3 | routed elephant: 0 -> pick a new direction |
| D04B | 4 | that direction |
Difficulty [048E]: **B354** defence factor (Galley Slave Romans x1.5, Senator enemy x1.25, Emperor enemy x1.5, Legionnaire none);
**C178** Roman infantry unit size floor 500/400/300/200 (more, smaller Roman infantry units on harder levels; a smaller unit
passes the "lost > 1/16" morale test more easily and rolls fewer dice per unit). No other reads (V: only these two references).

## 5. Player input during the battle
No timed input is needed: the battle runs on its own (one tick per 12 timer ticks = 0.165 s) and only pauses when the player opens
the orders mode. D0F2 runs once per tick: triggers = mouse button press (0692 for buttons 0 and 1), or key (03AA) Space, Enter,
'+' or Insert (0x10). Any key is stored in C07F. Orders mode is modal (simulation frozen) until "Done"/Esc or a global order:
- Menu (D198/D978): buttons at DS:748E with labels "Select Roman Unit" (S), "Done" (D), "Melee" (M), "Retreat" (R); Esc leaves.
  S: draw every Roman unit's order path (D9EF), stay in the menu. D: leave. M: "Release The Legion!": every Roman unit in state 2
  is reset (+6 &= 0xC0, state 1), then every state-1 non-general -> state 2 + B5E3; the general halts. Leaves.
  R: "Run Away!\nRun Away!": every Roman unit not gone: state 2 -> 1, D08E (orderly retreat to y = 44, state stays 1, units
  that walk off keep their men). Leaves.
- A click on the field (D33A): unit under the cursor (or a neighbour, D2DD). Enemy: info panel only. Roman: info panel + order path;
  orders only if not routed **and within the general's voice** (D8AD: dx^2+dy^2 <= voice^2, general alive). Then:
  drag with the button held: order = CBEE(unit, cell) = one straight leg along the dominant axis with Chebyshev length, +7/+8
  cleared; on release state = 1, turn = 0xFFFF. Keyboard (no mouse): numpad/arrows move a target cursor (via [00A2] and the
  D56A table: right/PgDn x+1, left/Home x-1, down/End y+1, up/PgUp y-1), polled every 9 timer ticks; 'M' unit Melee (D7E8:
  "Release Cohort <n>", state -> 2 + B5E3, the current leg kept as +6 with 1 step); 'R' unit Retreat (D77C: "Run Away!",
  state 2->1, D08E, current leg kept for 1 step); Space = hold (+6..+8 = 0x80); Enter/Esc/'+'/Insert = done.
  Clicking a button while a unit is selected dispatches Melee/Retreat for that unit (D714).
For deterministic replays record (tick number, order) pairs; with no orders the battle depends only on the RNG state, the two menu
choices and the DS inputs.

## 6. Strings and Easter eggs
Files: DS:72C4 "desert.cbm", 72CF "battlenw.cbm", 72DC "water.cbm", 733D "tactics.lzw" (sprites EE/RC/RG/RI/BG/BI in BATL.DAT).
Menus: 731E "Choose Formation" (72E6 "Balanced Army", 72F4 "Wedge", 72FA "Strong Right", 7307 "Strong Left"), 732F "Choose Tactic"
(+ 7313 "Stand Fast"), tactic names 765C..76E0 (2.3), enemy formation names 754C "Standard" .. 7574 "Massed Army".
Orders: 7443 "Select Roman Unit", 7455 "Done", 745A "Melee", 7460 "Retreat", 7468 "Release" + 7470 " The Legion!" / 747D " Cohort"
+ 74F2 " " + numeral, 74D4 "Run Away!\nRun Away!", 74E8 "Run Away!".
Unit panel: 74F4 " Cavalry", 74FD " Elephants", 7508 " Infantry", 7512 "General", 751A " Unit" (enemy), " Cohort" (Roman),
7520 "Panicked", 7529 "\nCharisma:", 7534 "\nVoice:", morale 742A "Panicky" 7432 "Weak" 7437 "Good" 743C "Fierce" (colours 7544:
4,12,15,9; index morale/25, no clamp - morale >= 100 would read 7486[4] = "Select Roman Unit").
Report (7ADC + list 7ADE..7B04): title 7A02 "A Glorious Victory!" / 7A16 "An Ignominious Defeat"; text = "\n\nThe legionnaires of "
+ numeral(leg+0x0A) + legion name (leg+0, e.g. " Ital", garrison "Rome") + " have met the army of " + zone name (5222[C29A]) +
"\non the field of battle.  " + leader 5F1C[C29A] + "'s army used the\n" + enemy tactic + " tactic against the " + Roman tactic +
" of\n" + commander name + " and the outcome was " + "a\nglorious victory for Rome!\n\n" / "an\nignominious defeat for Rome\n\n" +
numeral + legion name + "  vs  " + zone name + "\nCasualties\nSurvivors".
Easter eggs / jokes: "Run Away! Run Away!" (Monty Python and the Holy Grail) for the general retreat; garrison commander
"Albertus Discreditus" (cmdr 22) and "Marcus the Meek"; enemy leaders in the report (5F1C) "Absolna the Rotten" (Cilicia),
"Malodor the Impaler" (Sarmatia), "Thorax of Thrace", "Mamud the Terrible", "Horaxus", "Bertonus", "Darminus" next to historical
names. No special-cased names or magic values in the battle code itself (V).

## 7. Quirks worth keeping (all V)
- Elephant-terror roll unreachable (B1D7); legion +0x0F (set 0xFF after facing elephants) therefore has no in-battle effect.
- Elephants (any state, even routed) attack whatever stands in front, own side included; routed elephants pick random directions.
- Damage is applied simultaneously (C296 snapshot); only general hits change +0 immediately.
- Morale loss = (u8)(loss/2), so a loss of 600 removes only 44 points.
- Units do not de-duplicate targets: a 2-wide target is hit twice by one 2-wide attacker.
- Routed units never rally; units that leave the field (y 1/44 or the border) keep their strength and count as survivors.
- AA4C (31 ticks with one side inactive) is cumulative across the whole battle.
- C2B0[side] takes the last unit's CDC7 result, not an OR.
- B5E3 swaps legs on signed dy > dx; moves half the distance per leg.
- Placement: more than 6 elephant units read past their formation row; units beyond 18 regular slots stay at (0,0) outside the
  diamond with orders read at tactics[list + 0x80]; enemy size class > 3 reads the next style row.
- Sprite list holds 36 records (A92A-AA01) with no bound check; > 36 living units overwrite the sort array AA02 and, through it,
  AA4A/AA4C/AA4E (end and idle counters). Possible with big armies (e.g. 16 cavalry + 16 infantry units); replicate or avoid.
- Esc in either setup menu ends the battle with no losses and no write-back (return 0 = "not won").
- If the legion has no commander (+0x10 = 0xFF) the commander reads go past DS:1A1C (callers assign one first, G).

## 8. Porting order and deterministic tests
1. Tables: dump DS 6AE8-6B27, 6B28/6BD0/6C24/6DA6/724C/7244, 73DA-73FC, 7402, 740A-7429, 74BC, 7580-765B, 76F6-77EF, 77F0-79B1
   into C arrays; decode TACTICS.LZW from BATL.DAT (check against DS:F356 in t10_in.bin).
2. C153/C5EC unit table (no RNG): test counts/sizes against 73FE/7400 and a mid-setup DS capture (the unit table, C296 and the
   grid are calloc'ed in DS, so any DOSBox DS dump during the battle contains them at [C292]/[C296]/[C2AE]).
3. Grid helpers + C11E border + C2F2 placement + C525 orders (no RNG): compare unit x/y/+6..+8/state after setup.
4. BE3C dice: enemy formation + tactic rolls; compare C2A9/C2AB and the RNG words DS:00C8/00CA.
5. Simulation: B848/B94D frame counter (needed for timing), CDC7, CC7C, B0E0+B319, B405, B4E5, B54C, B5E3, D012, D08E, E5B8.
   Test tick by tick: run the original with no orders, dump DS every N ticks (breakpoint at E07D), diff the unit table, C296,
   AA4C/AA4E, C2B0 and the RNG state.
6. E21A + DF24 write-back: given a DS snapshot just before the overlay call (legion, army/province, commander table, difficulty,
   RNG state) and the two menu choices with no in-battle orders, the after-battle DS must match exactly: legion +6/+8/+0x0B
   (+0x0D/+0x0C if destroyed), army record +2..+0x0D or prov[Z] +0/+2/+4/+0x0C, neighbours' +0x0D, commander status (1A20),
   C0F8/C0FA, C082-C088, DS:00C8/00CA (the number of rolls is a strong check), return value.
7. UI last: orders mode (D0F2..DC72), report screen, sprites. For orders, replay recorded (tick, action) pairs.
Open points (G): exact music/sprite roles of C2A6/C2A7/[3EE9]/7256; whether EXE helpers called during the battle (music, menus)
ever consume roll(); 0x1A1C status values 0/1 named from 4816/4733 only.
