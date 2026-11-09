# Negotiation data (MNEGO archive 7)

Claude, 2026-11-09. Data side of the negotiation screen (0x5F82 from 0x602E, 0x6570, 0x6501, 0x04B8, 0x7B56, 0x4852).
Marks as in the other notes: **D** decoded from the disassembly / dumped from the data, **V** verified by running or by an
exact fit, **G** guess. Nothing here was run in the game. The readable dump of every province text is in
[nego_texts.md](nego_texts.md) (generated, with the reply menus and the state transitions).

## 1. The archive (D, dumped)
`C:\work\Centurion-native\MNEGO.DIR/.DAT` (archive 7, opened by `0F7B(7, &arch)` at 6577), 102 members. DIR / LZSS as in
`tools/extract_data.py` (fmt 0x80 = u32 size + LZSS; 0xC0 = CBM). Members used by the negotiation:

| member | fmt | packed -> unpacked | what |
|---|---|---|---|
| TEXTA..TEXTZ.LZW | 0x80 | 386..1086 -> 502..1527 | the province's lines (section 2) |
| SCOUT.LZW | 0x80 | 2441 -> 4247 | scout text per zone (section 3) |
| TABHDR / TABDAT.LZW | 0x80 | 103 -> 112 / 479 -> 2374 | 8 frame sprites ("tab" = tablet frame) (section 4) |
| LOVE1HDR / LOVE1DAT.LZW | 0x80 | 205 -> 224 / 9519 -> 20931 | 16 sprites, the bedchamber animation (Aegyptus only) |
| LOVE2HDR / LOVE2DAT.LZW | 0x80 | 56 -> 56 / 12894 -> 26065 | 4 sprites: 2nd Cleopatra portrait, bedchamber, barge, flash |
| LEADERA..Z.CBM | 0xC0 | 1265..1967 | portraits 96x66, LUT 1, transparent nibble 8 (section 5) |
| CN6.LZW | 0x80 | 1911 -> 2544 | music: song 8 (section 6) |

The other members (NEGBG*, NEGFG*, ROME, SKY*) are the province strips (CBM_FORMAT.md).

## 2. Who loads what: 0x6570 nego_screen(z, &text) (D)
- **Letter = '@' + z** (not 'A' + z): 65D7 `mov al,[bp+6]; add al,0x40; mov [0x4EF4],al` patches "leaderA.cbm" (DS:4EEE, letter
  at 4EF4) and 6608 patches "textA.lzw" (DS:4EE4, letter at 4EE8). So zone 1 Carthago = A ... zone 26 Sarmatia = Z; zone 0 (Italia) and
  the seas (27+) have no files. Zone names DS:5222[z]; full table in nego_texts.md.
- LEADER<x>.CBM via 0x1273 into a local descriptor, drawn 0x8E62 at x 8, y 79 (pushes `desc, 8, 0x4F`), freed (86A0). If the file is
  missing it is silently skipped (65EE).
- TEXT<x>.LZW via 0x11C4 into the caller's far buffer (5F82's [bp-1Eh], a {offset, segment} pair).
- z == 2 only: 0x163B("love1") -> [C10E], 0x163B("love2") -> [C1EC] (DS:4ED4, 4EDA).
- Always: 0x163B("tab") (DS:4EE0) -> [C1EE].
- 0x7B56(z, &arch) (66DC): SCOUT.LZW and the scout report (section 3).
- 5F82 frees: text 198F, love1/love2 19A0 (z == 2), tab 19A0 at the end (64EF).

## 3. Text files

### 3.1 TEXTx.LZW and SCOUT.LZW format (D, all 27 files checked by script)
```
u16 off[n]       off[0] = 2n (the size of this table), off[n-1] = file size
char str[n-1][]  string i = bytes off[i] .. off[i+1]-1, each ending in its own NUL
```
No other control bytes (no '\n'; the text box wraps, 5AE4). Counts: TEXTB 21 strings, TEXTD/K/L 15, TEXTF/H/I/V 14,
TEXTO/P/R/U 12, all others 13; SCOUT 27 (index z, string 0 = "" for Italia). Longest string 254 bytes with the NUL (TEXTP 2).

**0x04B8 text_get(&far, i, dest)** (pushes `dest, i, &far`; ret 6): `si = far.off; ds = far.seg; p = si + off[i];
len = off[i+1] - off[i]; rep movsb -> DS:dest`. The caller always passes dest = DS:AC60. No bounds check: i must be < n-1
(the dialogue never asks for more, see 3.3). The NUL is copied with the string.

**0x6501 pick_variant(s)** (ret 2): `n = roll(3) + 1; for (k = 0; k < n; k++) { last = s; p = strchr(s, '~');
if (p) { *p = 0; s = p + 1; } } return last;` so variant n (1-based) is returned, or the last one if the string has fewer.
roll(3) is drawn for EVERY string said, even one without '~' (dice order!). 3 variants: 1/3 each; 2 variants (TEXTD 3/10,
TEXTL 3/10): the 2nd is shown 2 times in 3; 1 variant: always. It writes NULs into the AC60 copy only.

### 3.2 State = string index (D)
The province says `pick_variant(text_get(buf, state))` in colour 15 (5AE4(..., 15, 0x22)); the player's line `4C92[code]` in
colour 9. Fixed meanings: **0** = opening when rank [5971] < 4092[z] or [634E] < 40AE[z] (hostile), **1** = opening otherwise,
**2** = alliance accepted (ends the talk; Aegyptus continues), **3** = what a refused tribute offer leads to (also reachable from
the A table). All other indices are free per province and only mean something through the tables:
- B = 42EC[z]: 3 reply codes per state (`B[3*state + k]`); code 0x3D in the 3rd slot makes it a 2-item menu ([408E] = 2).
- reply code -> reply 40E6[code] (label 4028[], hotkey 404A[]), column 4124[code], player line 4C92[code] (table in 3.4).
- next state: reply 0 Withdraw / 1 Go to War end; replies 14..16 (Low/Medium/High) give 2 if 40CA[z] > reply-14 else 3;
  all others `A[mood*2*c + 4124[code] + weak]`, A = 455A[z], c = 4322[z], weak = c when legion inf+cav < province w00+w02,
  mood = [634E] / 34 (byte div), minus 1 if province +0x0E > 75 and mood > 0. The A tables are exactly 6 rows of c bytes
  (3 moods x normal/weak), so mood 3 ([634E] >= 102) would read the next table (G: [634E] is probably capped below 102).
- Alliance writes `P+0x12 = P+0x13 = sel + 1` (menu position). Every path to state 2 goes through the Low/Medium/High menu with
  Low/Medium/High at positions 0/1/2 (checked for all 26 zones), so it is the tribute level 1..3 in practice (D).

Table groups (zones sharing B and A; the texts of a group are written to the same state plan):

| B / A (c) | zones | note |
|---|---|---|
| 4226 / 442E (12) | A Carthago, E Parthia, N Dacia, X Narbonensis, Z Sarmatia | **state 2 is never reached: these never ally** (their string 2 is empty, TEXTA/E/N/X; TEXTZ 2 empty too) |
| 4162 / 433E (19) | B Aegyptus | Cleopatra, states 0..22 (3.3) |
| 42A0 / 44E2 (11) | C Macedonia, G Thracia, S Mesopotamia, T Armenia | |
| 41A2 / 43B0 (10) | D Gaul, K Alpes | |
| 41D0 / 43B0 (10) | L Germania | own B, Gaul's A |
| 41FE / 43EC (11) | M Dalmatia, W Britannia, Y Scythia | Scythia has tribute tolerance 0: never allies |
| 424E / 4476 (10) | F Syria, H Pontus, I Asia, V Sardinia | Asia never reaches 3 (tolerance 3 accepts all) |
| 4278 / 44B2 (8) | J Cilicia, Q Cyrenaica | the "pirate" texts |
| 42C8 / 4524 (9) | O Hispania, P Mauretania, R Arabia, U Sicilia | Hispania never reaches 3 (tolerance 3) |

Reachability (script, every mood/weak combination from states 0 and 1): every reachable state is < the file's string count, so
04B8 never reads past a table (D). Unreached strings: 2 in the never-ally group and Scythia, 3 in Asia and Hispania.

### 3.3 Aegyptus (z 2, TEXTB, LEADERB, Cleopatra) (D)
States 0..14 are a normal talk (only the tribute menu at 13 leads to 2). After string 2 ("Dine with us tonight..."): music 8
(CN6), LOVE2 frame 2 (barge) at (0,0), feast text DS:4DD4, bedchamber text DS:4E28 (both colour 0), LOVE2 frame 1 (bedchamber)
at (0,0) and frame 0 (second portrait) at (8,79), state = 15. Then the dialogue continues in TEXTB 15..20 with the replies
Warm / Sarcastic / Aggressive (15), Gentle / Pushy / Rude (16), Approach / Leave (20):
- 17, 18, 19: the string is said, delay 0x90, then DS:4D0C (Cleopatra calls the guards ... "the alliance remains intact"); end.
- 21 (Approach, "Then I offer you this gift..."): no string; the love animation (section 4.3), then the barge with the flashes; end.
- 22 (Leave, "My queen, I must go."): no string; end.
All ends return -1 (the alliance stands; 53F8 runs). TEXTB has 21 strings, so 21/22 must never be passed to 04B8 (they are not).
DS strings: 4DD4 "Cleopatra entertains the Romans in her palace, and the celebration lasts five days."; 4E28 "Infatuated with
the Aegyptian queen, you tarry in Alexandria until finally you gain entrance to her bedchamber on her barge. Cleopatra is as
sharp-tongued as ever."

### 3.4 Reply codes (D, tables 40E6 / 4124 / 4C92; code 0 and 0x3D are not replies)
Replies 4028 (hotkey 404A): 0 Withdraw W, 1 Go to War G, 2 Ultimatum U, 3 Offer Alliance O, 4 Aggressive A, 5 Friendly F,
6 Diplomatic D, 7 Warm W, 8 Sarcastic S, 9 Gentle G, 10 Pushy P, 11 Rude R, 12 Approach A, 13 Leave L, 14 Low L, 15 Medium M, 16 High H.

| code | reply | col | player line |
|---|---|---|---|
| 1 | Ultimatum | 0 | Surrender or die! |
| 2 | Friendly | 1 | We come in friendship. |
| 3 | Diplomatic | 2 | Your courage and wisdom are legendary. |
| 4 | Go to War | 0 | Then we are at war. |
| 5 | Withdraw | 0 | We leave you in peace. |
| 6 | Offer Alliance | 3 | We offer an alliance. |
| 7 | Aggressive | 4 | Submit to the power of Rome! |
| 8 | Friendly | 5 | We should trust each other, my friend. |
| 9 | Diplomatic | 6 | Our terms are extremely generous. |
| 10 | Offer Alliance | 7 | We want you as allies. |
| 11 | Aggressive | 8 | Lay down your weapons. |
| 12 | Go to War | 0 | Die, barbarian pig! |
| 13 | Diplomatic | 9 | We seek to negotiate with you. |
| 14 | Low | 0 | The lowest in the Republic. |
| 15 | Medium | 0 | Our demands will not be unreasonable. |
| 16 | High | 0 | We require tribute befitting the grandeur of Rome. |
| 17 | Offer Alliance | 0 | We come in peace and seek an ally. |
| 18 | Diplomatic | 1 | Hear me, wise and honorable chieftain. |
| 19 | Aggressive | 2 | Your choice is simple. Surrender with honor or die in disgrace. |
| 20 | Friendly | 3 | I am here to make peace. |
| 21 | Diplomatic | 5 | Forgive our arrogance, honorable chieftain. |
| 22 | Offer Alliance | 6 | Join us and the armies of Rome will protect you. |
| 23 | Diplomatic | 7 | We will try to earn your trust. |
| 24 | Aggressive | 8 | That is the choice we offer, and we will not make it again. |
| 25 | Offer Alliance | 9 | Then we offer an alliance, and the promise of protection. |
| 26 | Ultimatum | 10 | Submit now, or be destroyed. |
| 27 | Friendly | 1 | Rome can be a powerful friend. |
| 28 | Diplomatic | 2 | Great queen, we merely wish to negotiate. |
| 29 | Aggressive | 3 | Capitulate now, and there will be no bloodshed. |
| 30 | Friendly | 4 | Protection, great queen.  Aegyptus has weakened, and Rome is great. |
| 31 | Diplomatic | 5 | Rome desires only peace between our great nations. |
| 32 | Aggressive | 6 | Take care! Arrogant queens are easily crushed! |
| 33 | Diplomatic | 7 | Indeed, we are humbled by your presence. |
| 34 | Offer Alliance | 8 | I can offer you an alliance with Rome. |
| 35 | Offer Alliance | 9 | We offer an alliance, and promise protection in exchange for tribute. |
| 36 | Friendly | 10 | We should trust each other, great queen. |
| 37 | Friendly | 2 | I am here to make peace. |
| 38 | Diplomatic | 4 | We will try to earn your trust. |
| 39 | Aggressive | 6 | Capitulate now, and there will be no bloodshed. |
| 40 | Diplomatic | 11 | Rome desires only peace between our great nations. |
| 41 | Aggressive | 3 | Your choice is simple. Surrender with honor or die in disgrace. |
| 42 | Friendly | 8 | Rome can be a powerful friend. |
| 43 | Diplomatic | 0 | Rome desires only peace between our great nations. |
| 44 | Aggressive | 2 | Lay down your weapons. |
| 45 | Diplomatic | 3 | We seek to negotiate with you. |
| 46 | Diplomatic | 4 | Hear me, wise and honorable chieftain. |
| 47 | Friendly | 7 | Rome can be a powerful friend. |
| 48 | Diplomatic | 1 | Your courage and wisdom are legendary. |
| 49 | Offer Alliance | 4 | Then we offer an alliance, and the promise of protection. |
| 50 | Offer Alliance | 0 | We offer an alliance. |
| 51 | Aggressive | 5 | Submit to the power of Rome! |
| 52 | Offer Alliance | 7 | We offer an alliance, and promise protection in exchange for tribute. |
| 53 | Warm | 11 | My feelings for you are genuine.  Your beauty, Cleopatra, is unsurpassed throughout the world. |
| 54 | Sarcastic | 12 | How you flatter yourself, great Queen. |
| 55 | Aggressive | 13 | You are the only conquest I seek, and I never lose a battle. |
| 56 | Gentle | 14 | My queen, hunger is a simple need, while love is a great gift. |
| 57 | Pushy | 15 | Romans take what is theirs.  It is that simple. |
| 58 | Rude | 16 | I grow impatient with you, Cleopatra. |
| 59 | Approach | 17 | Then I offer you this gift... |
| 60 | Leave | 18 | My queen, I must go. |

Menu record 4080 (D, raw): x0 8, y0 0x97, x1 0x67, y1 0xC5, +8 colour 0, +0A items 405C, +0C title 3E02 (""), +0E count
(= [408E], 3 or 2), +0F 0xFF, +10 0xFFFF. Items 405C/4068/4074 (12 bytes): +0 text (written per state), +2 hotkey (written),
+3 0, then x 7, y 4 / 0x11 / 0x1E, x1 0x5D, y1 0x0E / 0x1B / 0x28. Static DS = runtime snapshot for 4028..4D90 (byte compare).

### 3.5 Scout report 0x7B56(&arch, z) (D)
Sets [C1D8] = 0x56; loads SCOUT.LZW (DS:5EA1 "scout.lzw") with 11C4 (returns at once if missing); says string z in colour 0
(5AE4(AC60, 0, 0)) and waits (5AC2(1)); then builds in AC60 and says (colour 0):
`"The wealth of " + zone + " is " + 5F52[P+0x14 / 40] + ", the army is " + 5F5A[(3*P.w02 + P.w00) / 2500] +
" and the soldiers are known for their " + 5F62[P+0x0A / 25]`, then 7CB2 ("Click to continue" blink at (0xD4,0xB7)),
5AC2(2), and `5F1C[z] + ", the leader of " + zone + ", offers a greeting:"` (colour 0); frees the file.
5F52 small / unremarkable / immense / legendary; 5F5A insignificant / ordinary / strong / huge; 5F62 "tendency to panic." /
"lack of courage." / "tenacity." / "fierce courage.". Leader names 5F1C[z] (z 0 "Cinna", unused): see nego_texts.md.
**1990 bugs (D, keep them under --original):** the three tables have 4 entries and no clamp, and they are adjacent:
- wealth 160..199 -> index 4 = 5F5A[0] "insignificant", 200..239 -> "ordinary". **Aegyptus starts with wealth 200** (static DS
  P+0x14 = 200), so the scout says "The wealth of Aegyptus is ordinary" at the start of the game (D, not seen in the game).
- army (3*cav + inf) >= 10000 -> index 4 = 5F62[0]: "the army is tendency to panic." (Germania starts at 8000.)
- courage 100 -> index 4 = the word at 5F6A, which points at bytes `1E 1E 14 14 ...` (garbage text). Starting values are <= 90.

## 4. HDR/DAT sprite sheets: TAB, LOVE1, LOVE2

### 4.1 Format (D for the reading, V by exact fit)
`xxxHDR.LZW` = n x 14-byte descriptors, the same layout the DS sprite descriptors have (0x8E62):
```
+0 u16 width px   +2 s16 xhot   +4 s16 yhot   +6 u8 bytes per row   +7 u8 rows
+8 u8 transparent nibble   +9 u8 colour table (DS:60C0 + 16*t)   +0A u32 offset of the pixels in xxxDAT
```
`xxxDAT.LZW` = 4-bit pixels, high nibble first, rows of `bytes per row`. The offset counts from 2 bytes before the first DAT
byte (0x16FB starts its running size at 2): entry 0 has offset 2 and each sprite ends where the next starts; the last ends at
DAT size + 2 for all three sheets (V, exact fit). Drawn by 0x8E62(y, x, desc) at (x - xhot, y - yhot); the transparent
nibble is skipped. Loader 0x163B(name, &arch, 0): name + "HDR.LZW" (DS:0284) by 11C4, n = size / 14 (16FB; in EGA mode
[9B26] == 4 it also sizes planar copies, (w+7)/8*rows*5), then name + "DAT.LZW" (DS:028C); errors ":No DOS Memory" (0230),
":Header Scan failed" (0329). Returns the descriptor array; sprite k = array + 14*k. Same format as the port's
province.c `sheet_load` (MMENU / INIT / INIT2), which can be reused.

### 4.2 TAB (8 sprites, LUT 26, transparent 9): the stone tablet frames (D for positions)
| k | size | hot | drawn at | is |
|---|---|---|---|---|
| 0 | 200x6 | (0,-114) | (113,79) -> top-left (113,193) | bottom bar of the speech box |
| 1 | 5x110 | (0,-4) | (113,83) | left post |
| 2 | 6x110 | (-194,-4) | (307,83) | right post |
| 3 | 200x4 | (0,0) | (113,79) | top bar |
| 4 | 98x5 | (0,-43) | (8,194) | bottom bar of the reply box |
| 5 | 5x39 | (0,-4) | (8,155) | left post |
| 6 | 8x39 | (-90,-4) | (98,155) | right post |
| 7 | 98x4 | (0,0) | (8,151) | top bar |
0..3 at 0x6570 (`8E62([C1EE]+14k, y 0x4F, x 0x71)`), speech area fill (117,82)-(307,192) colour [6346]; 4..7 before every reply
menu at (y [4082] = 151, x [4080] = 8), after a fill (10,151)-(103,197) colour [6346] (6159: pushes 0x0A, 0x97, 0x67, 0xC5,
read in the same order as the speech fill 0x75, 0x52, 0x133, 0xC0 = (117,82)-(307,192)). Layout: portrait 96x66 at (8,79), speech box x 113..312 y 79..198, replies x 8..105 y 151..198, zone name centred
x 160 y 65 (8BE4, colour 0), strip rows 0..63.

### 4.3 LOVE1 (16 sprites, LUT 7, transparent 0) and the love animation 0x4852 (D)
- 0: 224x66, the bedchamber: hieroglyph cartouche and pillars on the left, Cleopatra reclining on a draped couch.
- 1: 96x66, a bare-backed guard with a spear, seen from behind (right foreground).
- 2: 44x63, the guard's head and back again (no spear), the moving part of him.
- 3..15: 6x25 .. 80x48 delta frames (they accumulate; the screen is not restored between them): a man in a white loincloth (the
  player's general, G) walks in from the left to the couch and Cleopatra turns towards him.
4852: if [C10E] != 0: on the work buffer 0 at (0,0), 1 at (224,0), 2 at ([3C26], 0); copy rows 0..63 to the screen; delay 0x48;
then for k = 3..15 (di = 0..12): 0x00F8 (timer reset, G), frame k at (x 85, y 3) straight on the screen, then 48EF(di); then
48EF(13..27). 48EF(i): if 3C26[i] != 0, redraw 1 at x 224 and 2 at x 3C26[i] on the work buffer and copy (224,0)-(319,63);
then wait until the timer 0x01AE reaches 14 ticks. 3C26 (28 bytes used) = 229,0,0,0,0,0,230,231,233,0,0,0,0,234,235,236,0,0,0,0,0,0,
238,240,242,243,0,244 (+ 227 at index 28, unused): the guard edges right, 229 -> 244 (G: turning away / sneaking off).

### 4.4 LOVE2 (4 sprites)
- 0: 96x66, LUT 9, transparent 15: a second Cleopatra portrait (dark hair, jewelled headband, bare shoulders, hieroglyph wall).
  Replaces the LEADERB portrait at (8,79) after the bedchamber text.
- 1: 319x63, LUT 9, transparent 15: the bedchamber strip, Cleopatra reclining half-nude, chin on her hand.
- 2: 320x63, LUT 8, transparent 8: her barge on the Nile, red sails, pavilion, pyramids on the far bank, cloudy sky. Drawn at
  the feast and again after the love animation.
- 3: 238x23, hot (-51,-17), LUT 8, transparent 15: a band of the barge picture with white forked lightning (or fireworks) bolts.
  0x654B: drawn at (51,17), delay 9, then rows 0..63 restored from the saved screen (6973). In state 21's wait loop (642A..6453):
  after more than 0x48 ticks, `roll(12) < 2` (DICE) triggers a flash; the loop ends on a click/key ([B2] / 0x03AA).
  "Click to continue" ([3E40]) colour 4 at (0xD4, 0xB8) during that loop.

## 5. LEADERx.CBM (D: header; descriptions G from the rendered PNGs)
All 26: 96x66, hot (0,0), 48 bytes/row, LUT 1, transparent nibble 8 (the light blue background of the PNGs is opaque in a plain
render), unpacked 3168. Drawn at (8,79).
A Carthago (Hannibal): dark curly hair and beard. B Aegyptus (Cleopatra): pharaonic striped headdress with uraeus, green eye
paint. C Macedonia (Philip): Corinthian helmet. D Gaul (Vercingetorix): conical helmet with cheek guards. E Parthia (Darius):
black beard, tall cylindrical cap with a jewel. F Syria (Antiochus): crested helmet. G Thracia (Thorax): helmet with a green
plume. H Pontus (Mithridates): young man with a green headband and green cloak. I Asia (Attalus): curly hair and beard.
J Cilicia (Absolna the Rotten): mail coif. K Alpes (Vindelic): bare-chested, wild curly hair, moustache, torc. L Germania
(Arminius): bare-chested, shaggy red-blond hair and beard. M Dalmatia (Bato): plumed helmet, green collar. N Dacia (Decebalus):
hooded cloak, ginger beard. O Hispania (Hasdrubal): dark hair and beard, white tunic with red crosses. P Mauretania (King Juba),
Q Cyrenaica (Bocchus), R Arabia (Mamud the Terrible): head cloths. S Mesopotamia (Horaxus): pointed helmet with a mail neck
guard and a shield. T Armenia (Bertonus): hood. U Sicilia (Hamilcar): round helmet, moustache. V Sardinia (Darminus): mail coif,
beard. W Britannia (Caswallon): horned helmet and a spear. **X Narbonensis (Galba): three bald, bearded cavemen side by side.**
Y Scythia (Chrysogones): fur-trimmed hood. Z Sarmatia (Malodor the Impaler): long hair and beard, bare-chested.
PNGs (scratch): `...\scratchpad\nego\png\leader?.cbm.png`, sheets `leaders_AI.png`, `leaders_JR.png`, `leaders_SZ.png`.

## 6. CN6.LZW: music (D for the use, format not decoded)
DS:3EF8 is the song table, 4 bytes per song {name pointer, archive}: 0 cn1 (3), 1 cn2 (5), 2 cn3a (0), 3 cn3b (0), 4..7 cn4a..cn4d
(6), **8 cn6 (7 = MNEGO)**, 9 cn7a (2), ... cn9d, then "speaker", "adl" (driver names). 0x5D29(n) plays song n (loads it via 5D73,
then the overlay driver 072E); 0x5E1A(n) unloads. In the EXE, song 8 is played only at 62CC (Cleopatra's feast) and unloaded at
6479 (end of her branch). CN6 is 2544 bytes, begins `7F 00 00 00 01 01 01 01 ...`; the music format belongs to the sound overlay
(not decoded here).

## 7. Easter eggs and jokes (all kept; quotes exact)
Texts (TEXTx unless said):
- **Scythia (Y) talks like Yoda**: "An ally? Hear ye I will, trust ye I will not." (4), "Powerful are the Scythians, but honorable
  they are not." (8), "Fight ye, the Scythians will." (9).
- **Narbonensis (X), a Roman-coast province, are cavemen** in pidgin capitals with a three-caveman portrait: "UH-LIE-ANCE.  THIS WORD
  I NOT LIKE." (8), "YOU TRIBE WEAR CLOTHES.  NOT RIGHT!" (1, variant 1). Leader "Galba".
- **Sicilia (U) is the Mafia**: "You will sleep with the fishes!" (0), "your offer is one we cannot refuse" (2), "We are willing to
  pay for your protection." (2).
- Germania (L) 3: "ROMAN SMELL LIKE ELEPHANT." / "ROMAN LOOK LIKE WARTHOG."
- Gaul (D) 0: "I SPIT ON STINKING ROMAN DOGS.  PTOOEE!!" (Gaul, Alpes, Germania, Narbonensis all shout in capitals).
- Mesopotamia (S) 2: "So let it be written." (The Ten Commandments); "heirs of Hammurabi" (12).
- Sarmatia (Z) 12: "Sarmatians make war, not peace." (inverted "make love, not war").
- Armenia (T): "Might Romans be part Armenian?" (2) and a run of neighbour jabs: "You extort like a Cilician" (3), "graceless as a
  Syrian's" (6), "You are worse than a Greek!" (7), "You lie better than even a Parthian." (11).
- Cilicia (J) pirates: "Pirates and Romans belong together!" (2), "Who is the true pirate here?" (3). **Cyrenaica (Q)**, a desert
  province, uses the same pirate plan and calls itself pirates: "Who wouldn't want rapacious pirates as allies?" (11), "Roman shark" (2).
- Macedonia (C) 6: "Your compliment was not THAT good, Roman."; 12 quotes Euripides.
- Parthia (E) 7: "Trust Rome? Are you sober?"
- Dacia (N) 0: "We won't deal with women.  Go bid your menfolk farewell." / "They send a eunuch to parlay with warriors."
- Dalmatia (M) 2: "Perhaps it will even last beyond sundown."
- Britannia (W) 2: "But Roman rule will be hard to enforce on our island."
- Cleopatra (B): "Tell the Roman to take his tin soldiers elsewhere." (0), "Flattery is beneath me, Roman, and SO ARE YOU." (17),
  "Perhaps you are about to have a new experience, Roman.  GUARDS!!!" (18); the player's "You are the only conquest I seek, and I
  never lose a battle." (code 55).
- SCOUT: Cilicia "The scouts' advice is simple: SQUASH THEM!" and "salacious brigands"; Alpes "the dreaded word: BARBARIANS!";
  Britannia "unpredictable weather".
- Leader names (DS:5F1C): "Thorax of Thrace", "Absolna the Rotten" (Cilicia), "Malodor the Impaler" (Sarmatia), "Mamud the Terrible"
  (Arabia); "Horaxus", "Bertonus", "Darminus" look like made-up or in-joke names (G: perhaps team members' names latinised).
Pictures: the whole Cleopatra sequence (feast, bedchamber, the walk-in animation with the guard edging away, then the cut to the
barge with lightning flashes: the classic "fireworks" cutaway); the second Cleopatra portrait; LEADERX's three cavemen; LEADERW's
horned helmet (anachronistic Viking cliche, G). Mild nudity in LOVE1 0 and LOVE2 1.

## 8. Typos and 1990 bugs (keep under --original)
- Typos: TEXTE 11 "Begin the the attack!"; TEXTH 2 "suprises"; TEXTU 2 "The alliance is made" (no full stop); SCOUT 4 "battle-
  hardened" (space after the hyphen); TEXTN 0 "parlay".
- Scout table overflows (3.5): Aegyptus reported "ordinary" (wealth 200), big armies "tendency to panic.", courage 100 garbage.
- Carthago, Parthia, Dacia, Narbonensis, Sarmatia and Scythia can never become allies (by data, D); intended, probably.

## 9. For the implementer
- File names: `'@' + z`. Load with assets.c (`assets_load("MNEGO.DIR","MNEGO.DAT")`, `assets_find`, `assets_decode`); sprites like
  province.c `sheet_load`, portraits with `cbm_draw`.
- text_get must copy exactly `off[i]..off[i+1]` (with the NUL) into DS:AC60, and pick_variant must call `rng_roll(3)` once per
  string said (also for one-variant strings, also for Cleopatra's 15..20 and the alliance string), before anything else draws.
- The love flash calls `rng_roll(12)` only after 0x48 ticks of waiting, once per loop pass: headless it depends on timing
  (G: the port should make the tick count deterministic in --script mode).
- 5AE4 is called with (text, colour, 0x22) for the dialogue and (text, 0, 0) for the scout/feast/guards texts (push order).

## 10. Open questions
- 5AE4's box and wrapping for the speech area (TEXTBOX_NOTES) were not re-checked here.
- 0x00F8 and 0x01AE (timer reset / ticks) in 4852 and 6443: G from use.
- The exact meaning of `P+0x0E` in the mood rule (GAMELOOP_NOTES calls it unrest, G).
- CN6 / song format.
- Whether [634E] can reach 102 (mood 3 would overrun the A table).

## 11. Scripts (scratch, not in the repo)
`C:\Users\erikk\AppData\Local\Temp\claude\bfb5522be8ee045d\c4bf0f5f-57f9-4ea6-80b3-e1aa05bf0592\scratchpad\nego\scripts\`:
ext.py (extract MNEGO to `..\out`), ds.py / zones.py / tabs.py (DS dumps), graph.py (tables, state graph, reachability),
gen_md.py (writes docs/nego_texts.md), render.py / sheet.py / lovesheet.py / guard.py / walk.py / crop.py (PNGs in `..\png`).
Run with `python -I`.
