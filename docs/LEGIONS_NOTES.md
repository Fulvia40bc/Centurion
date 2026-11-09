# Province command "Legions" (EXE 0x4065) - notes for the port

Sources: EXE code segment 0823 (`disasm/EXE_RUNTIME.bin`, re-disassembled with llvm-objdump from each function start;
`seen.pkl` lacks 4B9F.., 5FE1..), DS strings/tables read from both OVL0.OVL static data (file 0xB400 + X) and the runtime
image `tools/dosbox/results/endturn/t10_in.bin` (identical for every table quoted here).
Tags: **V** = VERIFIED (code + capture pixel check by script), **D** = DECODED (read in code, tables read from DS), **G** = GUESS.
Untagged statements are **D**. `[X]` = DS:X. `rec` = legion record `L1918[L]` (20 bytes), `P` = province record `5270 + 32*z`,
`F` = fleet record `110E + 12*f`. `roll(n)` = EXE 076B (0..n-1, port: `rng_roll`).

## 1. Data touched

### Legion record (12 x 20 bytes at DS:1918; slot 12 = DS:1A08 is Rome's garrison pseudo-legion, never listed here)
| off | addr (L=0) | meaning | written by (this command) |
|---|---|---|---|
| +0x00 | 1918 | name, e.g. " Ital" (leading space), strcpy from `1C2E[province]` | 4392 raise |
| +0x06 w | 191E | infantry | 4392, 453B, 3AE4 |
| +0x08 w | 1920 | cavalry | 4392, 453B, 3AE4 |
| +0x0A | 1922 | numeral (shown via 6738 as Roman numeral) = `P.+0x18` after increment | 4392 |
| +0x0B | 1923 | courage/experience 0..99; name `7486[c/25]` = Panicky/Weak/Good/Fierce (index 4 = garbage bytes; only if c >= 100, G unreachable: End Turn caps +2/turn at 99, but a new legion copies `P.+0x0A`) | 4392, 453B |
| +0x0C | 1924 | zone (0..26 land, 27..36 sea = aboard a fleet); 0xFF when destroyed (4816) | 382A |
| +0x0D | 1925 | type 0 Infantry / 1 Cavalry / 2 Consular Army (`1C64` = "Inf","Cav","Army"); 0xFF = empty slot | 4392, 3F97 |
| +0x0E | 1926 | status 0 Ready / 1 Moved / 2 Forming (`1B54`); End Turn (0x0A26, endturn.c) resets to 0 | 382A (1), 4392 (2), 453B (2) |
| +0x0F | 1927 | 0 at raise (battle: 0xFF after facing elephants, BATTLE_NOTES) | 4392 |
| +0x10 | 1928 | commander index (table DS:1A1C, 6 B, +4 status 0 assigned/1 free/2 dead) | 4733 |
| +0x11 | 1929 | fleet carrying it (0..2) or 0xFF | 4392 (0xFF), 382A |
| +0x12 | 192A | home zone (province where raised); no reader found in this command | 4392 |

### Other DS used
| addr | what |
|---|---|
| C080 b | current province (set by the province screen; 4065 takes no argument) |
| 634A w | treasury (talents). Redrawn by 44E5 |
| 5971 b | rank. `595E[rank]` = max legions {1,2,3,4,6,8,10,12,12} |
| 1EC8[3] b | legion cost {20, 30, 60} (Inf, Cav, Consular). The type menu texts "Infantry Legion: 20" etc. are static strings, not built from this table |
| 1AAC[3] w / 1AB2[3] w | max infantry {4200, 4200, 6000} / max cavalry {0, 300, 600} by type |
| 1C2E[27] w | legion name by province: " Ital"," Car"," Aeg"," Mac"," Gaul"," Par"," Syr"," Thr"," Pont"," Asia"," Cil"," Alp"," Germ"," Dalm"," Dac"," Hisp"," Maur"," Cyrn"," Arab"," Mes"," Arm"," Sic"," Sard"," Brit"," Narb"," Scy"," Sar" |
| A6A2[12] b, A6AE b, A6AF b | filled by 3F44(z): legion indices in zone z; count of ALL active legions (any zone); count in z with status 0 |
| 1B7E | "select legion" menu struct (3 empty items, hotkeys '1','2','3', rect (2,0)-(317,[1B84]) ); 1B84 = its y1, 1B8C = its item count, 1B90 = "legion box is up" flag |
| 1B4C..1B52 w | saved-background rect for 3F23 (x0=2, y0=0, x1=317, y1=[1B52]) |
| 9B5C b / 9B5D b | font height 8 / line gap 3 (runtime values; static image has 0,0) ; 9B69 w text colour ; 9AFA w fill colour ; 6346 w box colour (26 at runtime) |
| 1AB8 + 4z | map anchor of zone z (x word, y word at 1ABA) |
| 50EC + 8z | adjacency, 8 per zone, 0xFF none |
| 8EDE[3] b | ship capacity by ship type {50, 100, 200} |
| C112 + 14i | enemy army record i (0..10): +1 zone, +2 home zone, +8 active (see enemy.c) |
| P.+0x04 w (5274) | recruits ("Manpower" on the province screen; value position V, see 6) ; P.+0x06 w (5276) = cap used when troops are returned |
| P.+0x0A b (527A) | province courage: copied into a new legion's +0x0B and used as the courage of fresh recruits |
| P.+0x15 b (5285) | status: 0 = not Roman (independent), else Roman/allied (1 Occupied, 2 Ally, 3 Colony, 4 Citizens) ; P.+0x17 (5287) != 0 = rebels |
| P.+0x18 b (5288) | per-province legion numeral counter, ++ then wraps 51 -> 1 |

## 2. Main routine 0x4065 (returns 0 = back to the province menu, 0xFFFF = a move was started, leave the province screen)
Callers: province menu item 0 (EXE 6DA4) and Rome's menu (EXE 73CD). On 0xFFFF the province screen ends and the map is redrawn
(6EFF `494A(province)`).
```c
int legions_4065(void) {                       /* z = [C080] throughout */
  int done = 0, item = 0 /*di*/, type_sel = 0 /*bp-8*/;
  cursor_push_1D55(0);
  draw_legions_menu_4710();                     /* menu 1C0A + "Local Legion List" box */
  while (!done) {                               /* 407D */
    item = menu_run_54C8(0x1C0A, item);         /* 0..5 or 0xFF (Esc) */
    switch (item) {
    case 0: /* Raise Legion (40C7) */
      wait_release_35DB();
      if ([A6AE] == 12)                 msgbox_59D4(0x1C9C);   /* "All legions have been raised" */
      else if (595E[rank] <= [A6AE])    msgbox_59D4(0x1CBA);   /* "You cannot command any more\nlegions at this rank" */
      else {
        int save = item, stop = 0;
        menu_draw_5614(0x1C1C, 1);              /* "What type of Legion?" over the same panel */
        while (!stop) {                         /* 4107 */
          type_sel = menu_run_54C8(0x1C1C, type_sel);           /* remembered between raises */
          if (type_sel == 2 && rank < 6)       msgbox_59D4(0x1DD0);   /* Consular Army */
          else if ((type_sel == 1 || type_sel == 2) && rank < 2) msgbox_59D4(0x1D9E);
          else if (type_sel <= 2) { if (raise_4392(z, type_sel)) stop = -1; }
          else stop = -1;                       /* 3 "None", 0xFF Esc */
          menu_mark_585B(0x1C1C, type_sel, 0);  /* G: un-highlight */
        }
        close_box_3F23(); draw_legions_menu_4710(); item = save;
      }
      break;
    case 1: /* Strengthen Legion (4168) */
      menu_off_55FF(0x1C0A);
      L = select_legion_3BAF(z, 0);
      if (L < 12) { strengthen_453B(L, z); close_box_3F23(); }
      legion_box_3CB0(z, 0x1D58 /* "Local Legion List" */);   /* full redraw if closed, else only the title */
      menu_on_55EA(0x1C0A);
      break;
    case 2: /* Move Legion (41A2) */
      menu_off_55FF(0x1C0A);
      L = select_legion_3BAF(z, 0);
      menu_on_55EA(0x1C0A);
      if (L < 12) {                             /* 420D */
        close_box_3F23(); map_redraw_494A(z); move_header_3EA3(L); cursor_show_1CCE();
        move_legion_382A(L);
        cursor_pop_1D8A(); return 0xFFFF;       /* also after Esc / cancel inside 382A */
      }
      legion_box_3CB0(z, 0x1D58);
      break;
    case 3: /* Upgrade Legion (41D0) */
      menu_off_55FF(0x1C0A);
      L = select_legion_3BAF(z, 0xFFFF);        /* may pick a legion that already acted */
      if (L < 12) { upgrade_3F97(L); close_box_3F23(); }
      legion_box_3CB0(z, 0x1D58); menu_on_55EA(0x1C0A);
      break;
    case 4: /* Legion List (41EF) */
      close_box_3F23(); legion_list_4280(); legion_box_3CB0(z, 0x1D58);
      break;
    case 5: case 0xFF: /* Continue / Esc (41F7) */
      close_box_3F23(); done = -1;
      break;
    }
    menu_mark_585B(0x1C0A, item, 0);            /* 41FF, G: un-highlight */
  }
  cursor_pop_1D8A(); return 0;
}
```
Menu 1C0A (D, struct read from DS): rect (6,79)-(156,197), title ptr 5702 "What is your command?", 6 items, item rects x 6..138,
y 23+10i..33+10i, hotkeys R S M U L C: "Raise Legion", "Strengthen Legion", "Move Legion", "Upgrade Legion", "Legion List",
"Continue". Item colour = struct +8 (0 = black); +0x10 = 0xFFFF in the static data (hotkey marks on, 56B6).
Menu 1C1C: same rect, title 1861 "What type of Legion?", items "Infantry Legion: 20" (I), "Cavalry Legion: 30" (C),
"Consular Army: 60" (A), "None" (N), same item rects.
Both are drawn with `5614(menu, 1)` = panel 30E6(x0, y0) + item texts. Item text position = (menu.x + item.x + 0x0B, menu.y + item.y):
**V** for this formula on the province menu DS:5790 (same rect 6,79) against `scr_narbonensis_province.bin`: all 7 items, 0 wrong,
0 extra pixels (script `vmenu.py`, see 7). So Legions items draw at (23, 102 + 10i), colour 0 (**D**, no capture of this menu).

## 3. Sub-routines (all **D**)
| addr | name | ret | args (push order) |
|---|---|---|---|
| 4710 | draw_legions_menu | - | 1D55(0); 35DB; 5614(1C0A,1); 3CB0([C080], 1D58); 1D8A |
| 3F44 | scan_zone(z) | n in z | fills A6A2[], A6AE, A6AF (section 1) |
| 3CB0 | legion_box(z, title) | - | see 6.1 |
| 3DB7 | legion_row(L, y, select) | - | see 6.3 |
| 3F23 | close_box | - | if [1B90]: 6973(1B4C..1B52) restore; [1B90] = 0 |
| 3BAF | select_legion(z, any) | L or 0xFF | below |
| 4392 | raise(z, type) | -1 raised / 0 | below |
| 453B | strengthen(L, z) | - | below |
| 3F97 | upgrade(L) | - | below |
| 4280 | legion_list | - | 6.2 |
| 3EA3 | move_header(L) | - | 6.4 |
| 382A | move_legion(L) | - | below |
| 3AE4 / 3A32 | board(zone, L, f) / fleet_fit(f, L, &cav, &inf) | -1 / 0 | below |
| 4B3F | fight_armies(L, z) | -1 = no defeat, 0 = lost a battle | below |
| 4BAC | army_msg(name, numeral, z, army) | - | below |
| 5F82 | enter_foreign(L, z) | nonzero = province taken / allied | below |
| 4733 | assign_commander(L) | - | already ported (battle.c `assign_commander`) |
| 4816 | legion_lost(L) | - | ported (enemy.c), reached only through battles |
| 44E5 / 4510 | show_treasury(v) / show_recruits(v) | - | **V** positions, 6.5 |
| 494A | map_redraw(z) | - | map + outlines + icons; z != 0xFF: mouse to anchor z (066A) |
| 7330 | score_add(n) | - | ported (endturn.c) |
| 53F8 | province_won(z) | - | enemy fleets 3-5 with home (F+7) == z: F+8 = 0xFF |
| 5C20 | yes_no(text) | 1 Yes / 0 | menu 3E5A ("Yes" Y, "No" N) sized around the text by 8C9F, centred box |
UI helpers: 1D55/1D8A cursor push/pop, 1CCE cursor show, 35DB wait release, 359B wait click or key (key in [C07F]), 35F9 =
35DB,359B,35DB, 54C8 menu run, 5614 draw, 55EA/55FF set menu+0x10 = 0xFFFF/0 and draw/erase hotkey marks (G), 585B mark item,
59D4 message box, 691B/6973 save/restore screen rect (x0,y0,x1,y1), 3134 box (x0,y0,x1,y1,colour), 7E74 fill (x0,y0,x1,y1, colour [9AFA]),
899C text (s,x,y) left, 8C3A (s,xr,y) right-aligned, 8BE4 (s,xc,y) centred multi-line, 8AC8 (long,xr,y) right number, 8E62 (sprite,x,y) blit,
6738 numeral -> C072, 9E32 strcpy(dst,src), 9DF2 strcat, 9FDC itoa, 9D04 long mul, A048 unsigned long div, 04FA add_cap, 5D20 music stop,
1E21 disk swap prompt, B06E land battle thunk = overlay section 2 DF24(army, L, Z) (BATTLE_NOTES 3.1).

### 3F44 scan_zone(z)
```c
[A6AF] = [A6AE] = n = 0;
for (L = 0; L < 12; L++) {                       /* si = 1925 + 20L, bound 1A15 */
  if (rec.type == 0xFF) continue;
  [A6AE]++;
  if (rec.zone != z) continue;
  A6A2[n++] = L;
  if (rec.status == 0) [A6AF]++;
}
return n;
```

### 3BAF select_legion(z, any)        any = 0 (Strengthen, Move), 0xFFFF (Upgrade)
```c
sel = 12; idx = 0;
n = scan_zone_3F44(z); if (n == 0) return 0xFF;
if (n > 3) n = 3;                                /* only the first 3 legions of a zone can ever be chosen (quirk) */
[1B8C] = n; cursor_push(0);
if (n > 1 || L1918[A6A2[0]].status != 0)
  legion_box_3CB0(z, [A6AF] ? 0x1F5E /* "  Select Legion  " */ : 0);   /* box already up: only retitles */
if (n > 1 && ([A6AF] || any)) {
  menu_on_55EA(0x1B7E);
  do {
    idx = menu_run_54C8(0x1B7E, idx);           /* click a row or key 1..3, Esc = 0xFF */
    if (idx == 0xFF) sel = 0xFF;
    else if (L1918[A6A2[idx]].status != 0 && !any) msgbox(0x1D0E);   /* "Legion has already\nacted this turn" */
    else sel = A6A2[idx];
  } while (sel == 12);
  menu_off_55FF(0x1B7E);
} else {
  sel = A6A2[0];
  if (L1918[sel].status != 0 && !any) { msgbox(n > 1 ? 0x1D32 /* "All legions here have\nacted this turn" */ : 0x1D0E); sel = 0xFF; }
}
cursor_pop(); return sel;
```
A single ready legion is picked without a menu.

### 4392 raise(z, t) -> -1 if raised
```c
mp = P.w04;                                      /* signed compare */
if (mp < 100) { msgbox(0x1E98); return 0; }      /* "There are not enough\nrecruits to raise a legion" (no redraws) */
if (cost[t] > treasury) { msgbox(0x1D6A); r = 0; goto out; }   /* "You do not have enough\ntalents to raise that legion" */
inf = maxinf[t]; cav = maxcav[t];
if (inf + cav > mp) { if (cav > mp) { cav = mp; inf = 0; } else inf = mp - cav; }   /* cavalry first */
for (L = 0; L < 12 && rec.type != 0xFF; L++) ;
if (L < 12) {                                    /* (if no slot: still charged, nothing raised; unreachable, caller checks A6AE) */
  rec.type = t; rec.inf = inf; rec.cav = cav; rec.courage = P.b0A; rec.zone = z;
  rec.status = 2 /* Forming */; rec.b0F = 0; rec.home = z; rec.fleet = 0xFF;
  strcpy(rec.name, 1C2E[z]);
  if (++P.b18 > 50) P.b18 = 1;
  rec.numeral = P.b18;
  P.w04 -= inf + cav;
  assign_commander_4733(L);                      /* DICE: roll(#free commanders); message "<name> is assigned\nto command <numeral><legion>" */
}
treasury -= cost[t]; r = -1;
out: show_treasury_44E5(treasury); show_recruits_4510(P.w04); return r;
```

### 453B strengthen(L, z)
```c
t = rec.type; maxsum = maxinf[t] + maxcav[t];
if (rec.inf == maxinf[t] && rec.cav == maxcav[t]) { msgbox(0x1E74); return; }   /* "Legion is already\nat full strength" */
cur = rec.inf + rec.cav; cost = cost_1EC8[t];
afford = (cost > treasury) ? (uint32)treasury * maxsum / cost : P.w04;
avail = min(P.w04, afford);                       /* unsigned */
cav_add = 0;
if (rec.cav < maxcav[t]) cav_add = min(maxcav[t] - rec.cav, min(P.w04 / 4, avail));
avail -= cav_add;
inf_add = (rec.inf < maxinf[t]) ? min(maxinf[t] - rec.inf, avail) : 0;
rec.cav += cav_add; rec.inf += inf_add; added = inf_add + cav_add;
P.w04 -= added;
rec.courage = (uint8)(((uint32)added * P.b0A + (uint32)rec.courage * cur) / (added + cur));   /* /0 if both 0 (G: unreachable) */
treasury -= (uint32)cost * added / maxsum;
if (added + cur) rec.status = 2;                  /* Forming - even when nothing was added (no money / no recruits): quirk */
show_treasury_44E5(treasury); show_recruits_4510(P.w04);
```
No message when nothing can be added; the price is pro rata: a full legion's worth of troops costs the raise price.

### 3F97 upgrade(L)
```c
strcpy(buf, rec.name);                           /* e.g. " Ital" - the numeral is not included */
switch (rec.type) {
case 0: if (rank < 2) msg = 0x1D9E;              /* "You cannot command a\nCavalry Legion at this rank" */
        else if (cost[1] - cost[0] /*10*/ > treasury) msg = 0x1D6A;
        else { treasury -= 10; rec.type = 1; strcat(buf, 0x1E4C /* " has been upgraded\nto a Cavalry Legion" */); msg = buf; } break;
case 1: if (rank < 6) msg = 0x1DD0;              /* "You cannot command a \nConsular Army at this rank" */
        else if (cost[2] - cost[1] /*30*/ > treasury) msg = 0x1D6A;
        else { treasury -= 30; rec.type = 2; strcat(buf, 0x1E26 /* " has been upgraded\nto a Consular Army" */); msg = buf; } break;
case 2: msg = 0x1E02; break;                      /* "Consular Armies\nmay not be upgraded" */
default: msg = 0;                                 /* (code skips the box) */
}
if (msg) msgbox(msg);
show_treasury_44E5(treasury);
```
Troops and status are not changed; only the caps (1AAC/1AB2) rise, so a later Strengthen / End Turn fills them.

### 382A move_legion(L)
```c
dest = 0xFF; z0 = rec.zone; cursor_push(0);
for (k = 0; k < 8; k++) {                        /* highlight */
  ship[k] = 0xFF; n = adj[z0*8 + k];
  if (n < 27) blit_8E62([C10C], anchor[n].x, anchor[n].y);
  if (z0 < 27)                                   /* on land: Roman fleets 0..2 in a neighbouring sea */
    for (f = 0; f < 3; f++)
      if (F[f].zone_6 == n && F[f].b8 != 0xFF && F[f].legion_9 == 0xFF) {
        ship[k] = f; blit_8E62([C10C], anchor[n].x - 3, anchor[n].y - 4); break; }
}
cursor_show_1CCE();
for (done = 0; !done; ) {
  wait_release_35DB();
  if (!wait_click_or_key_359B()) { if ([C07F] == 0x1B) break; continue; }   /* Esc leaves, other keys ignored */
  z = zone_at_6C67([B6], [B8]);                  /* 0xFF = none; the Asterix village shows its message here too (LOGIC_NOTES) */
  if (z == z0) done = -1;                         /* click own zone = cancel */
  for (k = 0; k < 8; k++) {
    if (adj[z0*8 + k] != z) continue;
    if (ship[k] != 0xFF && board_3AE4(z0, L, ship[k])) {         /* embark */
      F[ship[k]].legion_9 = L; rec.fleet = ship[k]; rec.status = 1; rec.zone = z; done = -1; }
    if (z >= 27) continue;                        /* sea without a free fleet: not a target, keep waiting */
    rec.status = 1; music_stop_5D20();
    if (fight_armies_4B3F(L, z)) {                /* won every battle against armies in z (or none there) */
      if (P[z].status == 0) {                     /* not Roman: rebels or negotiation */
        r = enter_foreign_5F82(L, z);
        if (r) score_add_7330(z > 5 ? 5 : z == 2 ? 30 : 15);
      } else r = -1;
      if (r) { rec.zone = z; dest = z;
               if (z0 >= 27) { F[rec.fleet].legion_9 = 0xFF; rec.fleet = 0xFF; } }   /* disembark */
    } else dest = z0;                              /* lost: stays (if it survived) */
    done = -1;
  }
}
map_redraw_494A(dest); cursor_pop();
```
Notes: a legion at sea (zone >= 27) can only land on an adjacent land zone (sea neighbours are never targets); it moves with its
fleet otherwise (fleet code, not here). For a land target the status becomes Moved before any battle, so it stays Moved even when
the move fails (Withdraw, lost battle, failed screen load). A refused boarding (No in 5C20) changes nothing and the routine keeps
waiting for another click. Allied/colony provinces (status != 0) are entered without negotiation. Score: Aegyptus 30, other zones 1..5 15, zones > 5: 5 (Italia cannot be foreign).

### 3AE4 board(z0, L, f) / 3A32 fleet_fit(f, L, &cav, &inf)
```c
cap = sum_t F[f].ships[t] * 8EDE[t] (50/100/200), 32-bit, capped 10000;
cav = rec.cav; inf = rec.inf; left = 0;
if (cav * 4 >= cap) { left = cav - cap/4; cav = cap/4; room = 0; } else room = cap - cav*4;   /* a horse takes 4 places */
if (inf >= room) { left += inf - room; inf = room; }
/* 3AE4: */
if (left) { sprintf(AC60, "%u", left); strcat(AC60, 0x1F28 /* " Legionnaires must be left behind.\nBoard the Legion?" */);
            if (!yes_no_5C20(AC60)) return 0; }
rec.cav = cav; rec.inf = inf;
F[f].b0A = (cav*4 + inf) / (ships0 + ships1 + ships2);   /* 16-bit div */
P[z0].w04 = add_cap(P[z0].w04, left, P[z0].w06);           /* left-behind men return to the recruits pool */
return -1;
```

### 4B3F fight_armies(L, z)
```c
r = -1;
for (i = 0; i < 11; i++) {                       /* army records C112 + 14i */
  if (!army[i].w8 || army[i].zone_1 != z) continue;
  army_msg_4BAC(&rec.name, rec.numeral, z, i);    /* "An army from <home>\ninvades <zone>\n<numeral><name> meets the enemy on\nthe field of battle." */
  disk_swap_1E21();
  if (battle_B06E(i, L, z)) army[i].w8 = 0;       /* Roman victory: army gone */
  else r = 0;                                     /* keeps looping over the other armies even after a defeat */
}
return r;
```
4BAC template pointers DS:3D7A..3D8A: 3CD0 "An army from ", [3D7C] = zone name of army+2 (home), 3CDE "\ninvades ", [3D80] zone z,
3CE8 "\n", C072 numeral, [3D86] legion name, 3CEA " meets the enemy on\nthe ", then 3D03 "field of battle." (z < 27) or
3D14 "rolling waves of destiny.".

### 5F82 enter_foreign(L, z)   (the attack / negotiation)
```c
wait_release();
if (P.b17 /* rebels */) {
  AC60 = numeral(rec.numeral) + rec.name + " arrives in " + zone_name[z] +
         "\nto quell the rebellion, and meets\nthe rebel army on the battlefield.";   /* 4ECC..4ED2: name, 3FD5, zone, 3FE2 */
  msgbox(AC60); disk_swap_1E21();
  r = battle_B06E(0xFF, L, z);                    /* army 0xFF = the province's own troops P.w00/w02/b0C */
  if (r) { P.status = 1; P.b19 = 0; P.b12 = [5FC0] /* tribute policy */; province_won_53F8(z); }
  return r;
}
/* negotiation (D for the flow, G for what the tables mean) */
cursor_push(0);
if (!nego_screen_6570(z, &buf)) { cursor_pop(); return 0; }   /* loads leader<'A'+z>.cbm, text<'A'+z>.lzw, "tab"; Aegyptus also love1/love2 */
result = 0; state = 0; weak = 0;
mood = [634E] / 34; if (P.b0E > 75 && mood) mood--;
A = 455A[z]; B = 42EC[z]; c = 4322[z];
if (rec.inf + rec.cav < P.w00 + P.w02) weak = c;
if (rank >= 4092[z] && [634E] >= 40AE[z]) state = 1;
for (end = 0; !end; ) {
  say(pick_variant_6501(text_4B8(buf, state)), 15);  /* DICE: roll(3) picks one of 3 '~'-separated variants */
  for (k = 0; k < 3; k++) { code[k] = B[state*3 + k]; item[k] = {4028[40E6[code]], hotkey 404A[40E6[code]]}; }
  menu 4080 count = 3 (2 if any code == 0x3D);   /* replies: Withdraw, Go to War, Ultimatum, Offer Alliance, tones, Low/Medium/High tribute */
  do sel = menu_run_54C8(0x4080, 0); while (sel == 0xFF);       /* cannot Esc */
  x = 40E6[code[sel]]; say(4C92[code[sel]], 9);                 /* player's line */
  if (x <= 1) {                                   /* 0 Withdraw / 1 Go to War */
    free files; end = -1;
    if (x == 1) { disk_swap(); result = battle_B06E(0xFF, L, z);
                  if (result) { P.status = 1; P.b19 = 0; P.b12 = [5FC0]; } }
    else delay_143(0x48);
    break;
  }
  delay_143(0x48);
  state = (x >= 14) ? (40CA[z] > x - 14 ? 2 : 3)  /* tribute offer Low/Medium/High accepted or not */
                    : A[(mood*2*c) + 4124[code[sel]] + weak];
  if (state == 2) {                               /* alliance */
    P.b12 = P.b13 = sel + 1; P.status = 2; [C106]++; P.b19 = 0;
    P.w04 = min(P.w04 + P.w02 + P.w00, P.w06);
    say(variant(text(buf, 2)), 15);
    if (z != 2) { end = result = -1; }
    else { /* Aegyptus: Cleopatra (easter egg) - feast text 4DD4, bedchamber text 4E28, state = 15, dialogue continues;
              states 0x11-0x13: guards text 4D0C "...the alliance remains intact." -> end;
              0x15: love animation 4852 + "Click to continue", DICE roll(12) < 2 every >72 ticks for an extra frame flash; -> end;
              0x16: end. All ends set result = -1. */ }
  }
}
if (result) province_won_53F8(z);
cursor_pop(); return result;
```
Tables (D, dumped): replies 4028 = Withdraw, Go to War, Ultimatum, Offer Alliance, Aggressive, Friendly, Diplomatic, Warm, Sarcastic,
Gentle, Pushy, Rude, Approach, Leave, Low, Medium, High; 4092 min rank per zone; 40AE min [634E] per zone; 40CA tribute tolerance;
4322 row width c. The full negotiation (texts, Cleopatra) is its own porting task; it is listed here because the Legions move reaches it.

## 4. All dice (076B) reachable from Legions
| where | call | when |
|---|---|---|
| 4733 (raise) | roll(number of free commanders) | each raised legion; if none is free every non-assigned one (status 1 **or 2 dead**) is made free first (ported) |
| 6501 (negotiation) | roll(3) | every text the province speaks |
| 6443 (Cleopatra animation) | roll(12) | loop while waiting for a click |
| B06E battle | many | BATTLE_NOTES 3 (formation, tactics, casualties) |
No dice in select / raise (besides 4733) / strengthen / upgrade / board / list.

## 5. Messages (DS addresses, verbatim)
1C9C "All legions have been raised" ; 1CBA "You cannot command any more\nlegions at this rank" ; 1D9E "You cannot command a\nCavalry Legion at this rank" ;
1DD0 "You cannot command a \nConsular Army at this rank" ; 1D0E "Legion has already\nacted this turn" ; 1D32 "All legions here have\nacted this turn" ;
1D6A "You do not have enough\ntalents to raise that legion" ; 1E98 "There are not enough\nrecruits to raise a legion" ; 1E74 "Legion is already\nat full strength" ;
1E4C " has been upgraded\nto a Cavalry Legion" ; 1E26 " has been upgraded\nto a Consular Army" ; 1E02 "Consular Armies\nmay not be upgraded" ;
1F28 " Legionnaires must be left behind.\nBoard the Legion?" ; 1F89 " is assigned\nto command " ; 3D7A.. army message ; 4ECC.. rebels ;
box titles 1D58 "Local Legion List", 1F5E "  Select Legion  ", 1F70 "Complete Legion List -- " + [3E40] -> 3E2E "Click to continue" ;
headers 1EFA "Infantry   Cavalry     Courage      Status", 1ECC "Infantry   Cavalry     Location      Status".

## 6. Drawing (coordinates with [9B5C] = 8, [9B5D] = 3, [6346] = 26; **D** unless marked)
### 6.1 3CB0 legion_box(z, title) - the "Local Legion List" / select box at the top of the province screen
```c
cursor_push(0); n = scan_zone_3F44(z); if (!n) goto out;
if (![1B90]) {
  step = 8 + 2 /*10*/; y = title ? 15 : 5; n = min(n, 3); [1B8C] = n;
  y1 = (n + 1) * step + y + 7;  [1B84] = [1B52] = y1;          /* n=1: 42, 2: 52, 3: 62 with a title */
  save_691B(2, 0, 317, y1); menu_draw_5614(0x1B7E, 2);           /* box 3134(2,0,317,y1,26), items are "" */
  [9B69] = 0; text_899C(0x1EFA, 115, y);                         /* header */
  for (i = 0, y += step; i < n; i++, y += step) legion_row_3DB7(A6A2[i], y, 0xFFFF);   /* rows 25, 35, 45 */
}
if (title) { [9AFA] = 26; fill_7E74(7, 5, 312, 15); [9B69] = 15; text_centred_8BE4(title, 160, 5); }
[1B90] = 0xFFFF;
out: cursor_pop();
```
Select-menu hit rects (1B5A items): (8..311, 24..34), (8..311, 34..44), (8..311, 44..54) = rows at 25/35/45 (title present).
### 6.2 4280 legion_list (all legions)
n = [A6AE]; h = (n+3)*8 + 3 + 12 (byte arithmetic); top = (200 - h)/2; bottom = h + top. save 691B(2,top,317,bottom); box 3134(2,top,317,bottom,26);
title "Complete Legion List -- Click to continue" colour 15 centred x 160, y top+5; header 1ECC colour 0 at (115, top+22);
rows every 8 px from y = top+32, one per active legion in record order, `legion_row_3DB7(L, y, 0)`; then 35F9 (release, click/key, release),
restore 6973(2,top,317,bottom).
### 6.3 3DB7 legion_row(L, y, select)   all colour 0
numeral right-aligned at x = 35 (select) / 25 (list); name left at 37 / 27; type `1C64[type]` left at 77; infantry right-aligned at 144;
cavalry right-aligned at 188; column 4 centred at 236: select = courage `7486[courage/25]`, list = fleet name `1100[fleet]` if aboard
else zone name `5222[zone]`; status `1B54[status]` centred at 292.
### 6.4 3EA3 move_header(L)
[1B52] = (8+6)*2 + 3 = 31; save 691B(2,0,317,31); box 3134(2,0,317,31,26); header 1EFA colour 0 at (115,5); row 3DB7(L, 16, 0xFFFF).
[1B90] is not set (the map redraw in 382A removes it).
### 6.5 Numbers on the province screen - **V**
44E5(v): fill (211.., w 55) colour [6346] then v left-aligned at (211,167), colour 0. 4510(v): same at (225,128), width 24.
Checked with the game font (FONT_FORMAT.md) against scr_narbonensis_province.bin ("20", "5279") and scr_rome_province.bin ("20", "5000"):
10-row window x..x+w, 0 mismatching pixels in all four (script `vnum.py`).
### 6.6 382A highlight
Sprite [C10C] (runtime pointer) blitted at each land neighbour's anchor (1AB8/1ABA) and at anchor - (3,4) for a sea neighbour holding a
free Roman fleet. Mouse range after 494A: x 0..311, y 0..185 (06E4/0709).

## 7. Verification scripts (scratch, not in the repo)
C:\Users\erikk\AppData\Local\Temp\claude\bfb5522be8ee045d\c4bf0f5f-57f9-4ea6-80b3-e1aa05bf0592\scratchpad\wf\legions\vnum.py,
...\legions\vmenu.py (font renderer per FONT_FORMAT.md + capture compare), ...\legions\d.sh (llvm-objdump from a start address).

## 8. Quirks and easter eggs for the port
- Only the first 3 legions in a zone are selectable/listed in the select box (3BAF caps n at 3).
- Strengthen with no money or no recruits adds nothing, shows nothing, and still marks the legion Forming.
- 4B3F keeps fighting the remaining armies in the zone after a defeat.
- Moving fails silently after Withdraw; the legion is still marked Moved.
- Easter egg 1: the Asterix village (6C67 code 0xFE) also answers clicks during Move Legion.
- Easter egg 2: Cleopatra (negotiating with Aegyptus, z == 2): alliance leads to the feast/bedchamber scenes (DS:4DD4, 4E28, 4D0C) and
  the love1/love2 animation (4852).
- 4392 charges the talents even if no free slot is found (unreachable).

## 9. Port status (port/src/legions.c / legions.h, Claude, 2026-11-09)
Game logic only, translated from the disassembly (a fresh dedup of `EXE_RUNTIME.clean.asm` per function), each function
commented with its address: 4065 `legions_cmd`, 3F44 `legions_scan_zone`, 3BAF `legions_select`, 4392 `legions_raise`,
453B `legions_strengthen`, 3F97 `legions_upgrade`, 382A `legions_move`, 3A32 `legions_fleet_fit`, 3AE4 `legions_board`,
4B3F `legions_fight_armies` (+ 4BAC army message), 5F82 `legions_enter_foreign` (rebel branch), 53F8, 6738 (into DS:C072),
3CB0/3DB7/3EA3/3F23/4280/4710/44E5/4510 (their DS writes: 1B52, 1B84, 1B8C, 1B90, 9AFA, 9B69, C072, A6A2..A6AF; their drawing as
`legions_ui` events with the layout as data, `legions_list_data` / `legions_row_data`).
- UI: `legions_message` (59D4), `legions_yesno` (5C20), `legions_menu` (54C8 on the DS record), `legions_click` (359B + 6C67 in
  382A: the zone, or Esc / other key), `legions_ui(ev, a, b, data)` for every drawing call (cursor, menus, boxes, map redraw
  494A, highlights 8E62, numbers 44E5/4510, music stop, disk swap). Headless defaults log to stderr and take answers from
  `legions_script()`; when the answers run out, menus return 0xFF, yes/no 0, clicks Esc, so every loop ends as in the game.
- Battles: `battle_auto(A, L, Z, &won)` at 4B85 and 6006 (B06E); without battle choices it counts as not won, reported on stderr
  with `g_enemy_unported = 1`.
- **NOT PORTED**: the negotiation (5F82 from 0x602E: 6570 and the dialogue, incl. Cleopatra). 5F82 reports it at the 6570 call and
  returns as when 6570 fails (cursor pop, 0: the legion does not enter, it stays Moved). Not wired to any screen yet (province menu
  item 0 at EXE 6DA4 / 73CD and the map's right click 08F5 are the callers).
- `legions_selftest(path)` (default `tools/dosbox/results/endturn/t10_in.bin`, run from C:\work\cent_play for BATL.DAT) runs
  nine scripted sub-commands and prints the DS bytes each changed: refused raise at rank 0; Consular Army raised in Italia (rank
  poked to 6: inf 6000 / cav 600, numeral II, treasury 100 -> 40, recruits 6750 -> 150, commander roll); Upgrade of a Consular Army
  + Legion List data (rect 2,72-317,127, rows at y 104/112); Strengthen of I Ital at 3000 infantry in Narbonensis (+1200, courage
  99 -> 85, treasury -5, recruits 6000 -> 4800, Forming); Upgrade to Cavalry (rank 2: -10) then refused; Move to Hispania
  (negotiation reported unported, legion Moved, stays); Move to Alpes with +0x17 poked (rebel battle won: Alpes Occupied, +0x12 =
  [5FC0], +0x19 = 0, score +5, legion moves); boarding fleet 0 (poked: 50 small ships in zone 33): 1700 left behind, No then Yes,
  inf 2500, F+0A 50, then landing in Narbonensis (fleet freed); raise in Italia and attack army 1 in Dalmatia (army message, battle
  won, army +8 = 0, then the negotiation is reached); and (10) the map loop's path 08F5..0919 (3BAF, 3F23, 3EA3, 382A).
- Review fixes (2026-11-09): 3EA3 is exported as `legions_move_header(L)` for the map loop's caller (correction 5);
  5D20 writes [3EEE] = 0xFF before the LEV_MUSIC_STOP event (the 074B driver call); 55EA / 55FF write the menu record's +0x10
  word (DS:1C1A for 1C0A, DS:1B8E for 1B7E: 0xFFFF on, 0 off) before LEV_MENU_ON / LEV_MENU_OFF; 4392 now does 4733's message
  tail itself after `assign_commander()` (47C6..480B: AC60 = commander name + 1F89 + 6738 numeral (C072) + rec name, through
  `legions_message`), see correction 8. battle.c's own stderr line in `assign_commander` still prints (without the numeral):
  its owner should drop it, or move the message into a hook there and then remove `commander_msg_4733` from legions.c.

## Corrections (verifier)
Checked independently on 2026-11-09 with a fresh llvm-objdump of `EXE_RUNTIME.bin` and my own font renderer / capture compare
(scratch: `...\scratchpad\wf\verif\{vnum,vmenu,vtitle,vmarks,menus}.py`). Re-derived and found **correct**: the 4065 control flow and
return values, 3BAF, 4392 (all record writes, cap logic, charge order), 453B (all arithmetic incl. A048/9D04 operand order), 3F97,
382A (highlight, boarding, fight, score 30/15/5, disembark, dest), 3A32/3AE4, 4B3F (11 armies, keeps fighting), 4733 (one roll),
3CB0/3DB7/3EA3/4280 coordinates, the rebel branch of 5F82, the callers 6DA4/73CD, and the 6.5 numbers (my own render: 0 mismatching
pixels for "20" at (211,167) and "5279"/"5000" at (225,128) in a (x..x+w, y+1..y+9) window; shifting by 1 px gives 37/46 mismatches).

1. **[3E40] is not the same in static and runtime data** (section 5 / 6.2, and the "read identically" claim). Static OVL0 has
   [3E40] = 3E16 "Hit any key to continue"; the runtime image has 3E2E "Click to continue". It is set by the resident overlay
   (OVL0 file 0x108, section 1): `if ([00BA] && mouse_init()) [3E40] = 0x3E2E; else [00BA] = 0;` (D). So the Legion List title is
   "Complete Legion List -- Hit any key to continue" without a mouse. The port must pick the string from the mouse flag [00BA].
2. **Menu title and marker drawing (55EA/55FF were G; now D + V).** 5614 never draws the title itself. When menu+0x10 != 0 it calls
   56B6, which draws the item markers and then the title: colour `menu+8 ^ 15` (= 15), `8BE4(title, (x0+x1)/2, y0+6)` (57CC..57EC).
   **V**: "What is your command?" at centre 81, y 85 (left x 27, width 108), colour 15: 0 wrong pixels in both province captures.
   Markers when [00BA] != 0 (mouse) and menu+0x0F != 0 (575F..57C9): with sx = x0+item0.x, ya = y0+item0.y, yb = y0+last.ya-1,
   vertical lines (925C) at x sx+2, sx+4, sx+6 from ya to yb, and horizontal lines sx+1..sx+7 at ya and at yb, colour [9AFA] = 8.
   **V** for menu 5790: x 14/16/18, y 102..171, caps x 13..19 at y 102 and 171, colour 8, 0 wrong pixels in a 11x75 window (both
   captures). Same geometry for 1C0A/1C1C (y 102..161 for 1C0A's 6 items; 102..141 for 1C1C). Without a mouse ([00BA] == 0) 56B6
   instead blits sprite [C1D6] at (sx, ya+1) per item (D, no capture).
   55FF (menu off) sets +0x10 = 0 and calls 57F7: if menu+0x0F != 0, fill (sx, ya, sx+8, yb) with [6346]; then the title again in
   **colour 6** (D). So during Strengthen/Move/Upgrade selection the "What is your command?" title is greyed (6) and the marker
   column erased; 55EA restores both. For the select box 1B7E (title DS:17DC = "") 55EA draws the marker at x 10/12/14, y 24..53
   (caps x 9..15) and 55FF erases (8,24)-(16,53) (D).
3. **Item colour** (5662..5676): an item whose byte +3 is nonzero is drawn in colour 6, else menu+8. All items of 1C0A/1C1C/1B7E have
   +3 = 0, so Legions draws colour 0, but the shared menu engine needs this.
4. **Section 3 "args (push order)" column is inconsistent.** The functions are callee-pops (ret n), first parameter = last pushed.
   Real push orders: 453B push z, L; 4B3F push z, L; 5F82 push z, L; 3AE4 push f, L, z0; 3A32 push f, L, &cav, &inf; 4BAC push
   army, zone, numeral, &name; B06E push z, L, army; 3BAF push z, any; 3CB0 push z, title; 3DB7 push L, y, select. The pseudo-C
   argument meanings are right; only the column label is wrong for the mixed rows. Also 3A32 returns `left` (men left behind,
   0 = all fit), not -1/0.
5. **Other callers (missing).** 3BAF and 382A are also called from the map loop at EXE 08F5..0919 (`L = 3BAF([bp-4] zone, 0);
   3F23(); if (L < 12) { 3EA3(L); 382A(L); }`), i.e. moving a legion straight from the map. There the select box may not be up
   ([1B90] = 0), so 3BAF's 3CB0 call draws it fresh with title "  Select Legion  " or none. Only the province-screen path is described.
6. **6.5 fill rect precisely** (8AF2): fill 7E74(x, y+yoff[' '], x+w, y+yoff[' ']+[9B5C]) with [9AFA] = [6346], yoff[' '] = DS:[[9B58]]
   = 1, so (211,168)-(266,176) and (225,129)-(249,137); then 8A9E(v, x, y) colour [9B69] = 0 (D; the capture cannot show the fill edge
   because the box is the same colour 26).
7. **(port, 2026-11-09) 453B marks the legion Forming only when something was added.** The test at 46EB is `or si, si` where
   si = added (46A0 `mov si, ax` with ax = cx = inf_add + cav_add; 9D04 and A048 preserve si). So the pseudo-C line is
   `if (added) rec.status = 2;`, not `if (added + cur)`, and the quirk in section 8 ("Strengthen with no money or no recruits ...
   still marks the legion Forming") is wrong: with nothing added the legion keeps its status (no message either). Also, when
   added + cur == 0 the courage division (A048) divides by zero (G: unreachable; the port logs it and keeps the courage).
8. **(port) 4733's message includes the numeral**: AC60 = commander name + 1F89 " is assigned\nto command " + 6738(rec numeral) +
   rec name (47D3..480B). battle.c `assign_commander` prints it without the numeral and without writing AC60 / C072.
   Handled in legions.c since 2026-11-09 (`commander_msg_4733` after the call, same DS writes and order); battle.c still has its
   extra stderr line (not owned by the Legions piece).
9. **(port) 4280 rescans**: it calls 3F44([C080]) first, so it rewrites A6A2..A6AF; n = [A6AE] = all active legions.
