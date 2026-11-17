# Intro notes: start-up, copy-protection question, intro (OVL0 section 1)

**Status 2026-11-17: B694, B970 and B8CF are ported** in port/src/intro.c (with B18A's music_load(0) / music_unload(0)
around it; credits.c exports credit_B9D3 and the poll). main.c play_game runs it after map_startup when there is no
--script and no --load, or always with `--intro` (a --script then drives it: "wait N", "key C", "shot NAME"). Headless
--intro shots: the phase changes fall on ticks 504, 2664, 3024, 6192 and the map at 6912 (V, port only); the DS after
the intro differs from a run without it only in the timer (DS:42..4D) and the cycle words (DS:6AE2..6AE6) (V). Still
no DOSBox capture of the intro. Not ported: B466 (Erik's decision), the EGA path, the sound driver.

Claude, 2026-11-17. Source: **disasm/OVL_SECT1.asm** (new: the whole of section 1, OVL0 file 0x0000-0x09A0, loaded at
0823:B0E0; it merges disasm/OVL_CREDITS.asm, which stays for credits.c's citations), disasm/EXE_RUNTIME.bin (EXE routines,
fresh llvm-objdump), the static DS (OVL0.OVL 0xB400 = DS:0000), the archives MINIT / CENT, the DS image
tools/dosbox/results/endturn/t10_in.bin and the DOSBox screenshot C:\DOSBox-X\hunt\shots\s02.png (the question box).
Labels: **D** = decoded from the code, **V** = checked against a capture or a run (the check is named), **G** = guess.
Scripts and pictures: scratchpad `finish/intro/` (seg.py + gen1.py make the listing, pics.py / comp.py the PNGs,
places.txt the PLACES table). No C code was changed.

## 1. Section 1

| func | thunk / caller | what |
|---|---|---|
| B0E0 | B086, main 07B4 | command line (argc, argv) (D) |
| B18A | B07A, main 07C2 | start-up: ISRs, font, **B466**, palette, MMENU, music, **B694**, INIT/INIT2, cursor, buffers, map picture, B397(3..5); 0 → main exit_game(1) (D) |
| B356 | B18A | work-buffer descriptor DS:C1DA (copy of the screen descriptor DS:9AB6): VGA alloc 64000 bytes, EGA A000:2000 (D) |
| B397 | B18A | enemy fleet i = 3..5 → its marines, army record i+8 (GAMELOOP_NOTES 1, V there) |
| B466 | B18A | copy-protection question (PLACES.LZW); -1 passed, 0 failed (D) |
| B694 | B18A | **the intro** (D) |
| B8B7, B8CF, B8FE, B970, B9A1, B9D3 | | credits and the colour cycle (credits.c; B970 and B8CF are the intro's: intro.c) |

Correction: docs/LOGIC_NOTES.md (line 100) has "B07A -> B18A command line, B086 -> B0E0 init": the other way round
(B0E0 parses argv, B18A is the start-up; GAMELOOP_NOTES section 0 is right) (D). The old listings OVL_RESIDENT.clean.asm
and the start of OVL_MAP.clean.asm are section 1 at origin 0 (their 0x0146 = B226 here) with wrong call targets.

## 2. The start-up chain (D)

```c
main 0x079D:  dice_seed 075A; video_init 80D6 (-> [9AB2] adapter, 8 in the DOSBox DS); B0E0(argc, argv);
              B(9AF8) = FF; mem_gfx_init 7DAA ([9B26] = 2 VGA); detect_drives 1E02; if (!B18A()) exit_game(1);
              [9AFA] = 0; fill(0,0,319,199); map_build(0); cursor_show; map loop.

void B0E0(int argc, char **argv) {            /* Pascal: argc pushed first. Switches compared case-sensitively (A186 strcmp) */
    if (W(9AB2) == 6) B(3EEA) = 0x14;          /* Tandy video -> Tandy sound */
    if (adlib_detect_055C()) B(3EEA) = 0x12;   /* static default 0x11 = PC speaker */
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "k")) W(BA) = 0;                                   /* DS:6302: no mouse */
        for (j = 0; j < 5; j++) if (!strcmp(argv[i], vid[j])) W(9AB2) = val[j]; /* x m e tandy v -> 0 2 4 6 8 (DS:6304/630E) */
        for (j = 0; j < 8; j++) if (!strcmp(argv[i], snd[j])) B(3EEA) = drv[j];/* nosnd pc tn roland mt lapc adlib ad ->
                                                                                   FF 11 14 13 13 13 12 12 (DS:6318/6328) */
    }
}

int B18A(void) {
    ok = -1;
    isr_0302 (int 24h); timer_0062 (int 08, PIT counter 0 = 3FFFh: DS:42/44 tick at 72.8 Hz; the old int 08 every 4th);
    kbd_03D3 (int 09); memcpy(DS:9B56, DS:A05C, 10) /* font record */; target_screen_680F();
    if (!B466()) exit_game(2);                                       /* the question: before the palette (grey box) */
    W(9AFA) = 0; fill(0, 0, 319, 199);
    if (W(BA)) { if (!mouse_reset_0632()) W(BA) = 0; else W(3E40) = 0x3E2E; }   /* "Click to continue" (else "Hit any key ...") */
    B356();
    if ((ok = archive_open(3 /* cent */))) {
        if (VGA) { member_load("cmap.bin"); set_dac_block_9772(buf) /* 256 entries from a 750-byte member */; free;
                   W(6344) = 0x74; W(6346) = 0x1A; W(6348) = 0x10C; B(636C) = 'm'; }   /* "xmenu" -> "mmenu" */
        else B(636C) = 'e';
        W(C0F6) = sheet_load("mmenu");  sound_driver_5E86();  music_load(0) /* cn1 */;  archive_close;
    }
    B694();                                                          /* the intro (also when cent failed) */
    music_unload(0);
    if ((ok &= archive_open(4 /* init */))) {
        d = sheet_load("init"); for (k = 0; k < 14; k++) W(W(6350 + 2k)) = d + 14k;   /* the map icons, map.c map_startup */
        W(C28A) = sheet_load("init2"); archive_close; }
    ok &= cursor_buffer_1C7F();  mouse_warp_066A(160, 100);  cursor_sprite_1DF5(W(C0E2)); /* INIT sprite 0 = the mouse cursor */
    VGA: W(C0C6) = alloc(64000 B); EGA: B(0252 + 8n) = n, n = 0..4; Tandy: W(C0C6) = alloc(32000 B);
    if (archive_open(5 /* map */)) { load_picture_13A8(0); archive_close; }   /* failure not counted */
    for (i = 3; i < 6; i++) B397(i);
    return ok;
}
```

## 3. B466, the copy-protection question (D; stays skipped, Erik's decision)

- Archive 3 (cent), member PLACES.LZW: 66 records of 20 bytes {u8 province, u8 (direction << 4 | type), char answer[18]}.
  `r = rng_roll(66)`: **one roll of the game's dice** (DS:C8/CA, the same as every other roll).
- Text: `"What is the " + dir + type + " in the\nprovince of " + name + "?"` (DS:63CA, DS:63D7, DS:63EC); dir DS:642E =
  "Northern ", "Southern ", "Eastern ", "Western ", "Land ", "Naval ", ""; type DS:643C = "Capital", "City", "Town",
  "Battle"; name DS:5222[province]. Box 3134(70, 70, 240, 130, [6346]) (7 = grey: the VGA colours are not set yet),
  the question 8BE4 centred at (155, 80) in white, the bar (95, 110)-(215, 122) in colour 8, edit_line 36ED(buf,
  (97, 112), 20, 120). **V** (geometry): s02.png ("What is the City in the province of Asia?" = record 50) has the
  grey box at x 70..240, y 70..130 and the white cursor at y 112..121, the 1990 default palette (grey 170 = index 7).
- Three tries. Esc (36ED returns 0) uses a try; Enter (-1) puts both strings through strupr and strcmp(answer, input),
  and in the 1990 code a wrong answer would use a try too. **In this copy the result is ignored**: `b5f4 or ax, ax / b5f6 eb 06 jmp b5fe` sets "passed"
  whatever was typed. MSC never emits a flag test followed by an unconditional jump; the 1990 code must have been
  `74 06 je b5fe` (wrong answer → the next try). **G: a one-byte crack (74 → EB) in Erik's copy.** So in this copy any
  Enter passes, three Escs fail (exit_game(2)).
- Passed: the archive names are decrypted. DS:029E (archive table) points at seven scrambled names DS:63EE.. (lengths
  DS:644A 5 5 5 4 6 5 4); key = DS:6444 `,"))>` XOR 47h = **"kenny"**: name[k] ^= key[k] → "xnego", "xrace", "xglad",
  "xmap", "xnaval", "xinit", "batl"; then the key is overwritten with five EAh bytes and the first letter of the six
  x-names becomes 'm' (VGA) or 'e' (other modes). Plain, unprotected: "cent" (it holds PLACES itself), "batlvga", "navl".
  Without the right answer the game cannot open MRACE, BATL, MINIT, MMAP, MGLAD, MNEGO, MNAVAL. **V**: t10_in.bin has
  `mnego mrace mglad mmap mnaval minit batl`, `EA EA EA EA EA` at DS:6444 and "mmenu" at DS:636C.
- The 66 questions (province: dir type → answer), from places.txt: Italia: Capital Rome, W City Pisae, E City Brundisium,
  W Battle L. Trasimeno, E Battle Cannae; Britannia: N Town Eburacum, S Town Londinium; Germania: N Town Colonia Agrippina,
  S Town Durocortorum, E Town Veldidena; Sarmatia: Town Aquincum; Gaul: N Town Coriallum, S Town Burdigala, N Battle
  Camaradum, S Battle Alesia; Alpes: Town Axima; Dalmatia: Town Senia, City Salonae, Battle Noreia; Dacia: Capital
  Sarmizegetusa, Town Apulum; Thracia: W City Serdica, E City Byzantium; Pontus: City Amastris, Town Amisus; Armenia:
  Capital Ancyra, W Town Tandeia, E Town Artaxata; Parthia: Town Rhagae; Mesopotamia: Capital Nisibis, Battle Carrhae;
  Hispania: Capital Carthago Nova, City Saguntum, N Town Numantia, S Town Gades, W Battle Llipa, E Battle Baecula;
  Narbonensis: Capital Narbo, City Massilia, Battle Arausio; Sardinia: N Town Sassari, S Town Cagliari; Sicilia: W City
  Lilybaeum, E City Syracuse; Macedonia: Capital Pella, W City Sparta, E City Athenae, Land Battle Pharsalus, Naval Battle
  Actium; Asia: Capital Pergamum, City Ephesus; Cilicia: Capital Tarsus, Town Selinus; Syria: Capital Antiochia, City Tyre;
  Arabia: City Bostra; Mauretania: W Town Portus Magnus, E Town Icosium; Carthago: Capital Carthago, Town Oea, W Battle Zama,
  E Battle Thapsus; Cyrenaica: Capital Cyrene; Aegyptus: Capital Alexandria, N City Memphis, S City Thebae.
  (Mixed Latin, Italian and English spellings, as the manual's map presumably has them; not checked against the manual.)

## 4. B694, the intro (D)

```c
void B694(void) {
    int skip = 0;                                         /* si; [bp-2] keeps it across the legend lines */
    ok = archive_open(4);                                 /* "minit" (decrypted by B466) */
    if (ok) { ok &= cbm_load("romulus.cbm", &rom); ok &= cbm_load("title.cbm", &tit);
              target_work_67E4();
              if (VGA) load_far_0F0C("world.lzw", W(C1DE), W(C1DC));   /* 64000 raw bytes straight into the work buffer */
              else { ok &= cbm_load("world.cbm", &w); sprite(&w, 0, 0); free_86A0(&w); }   /* EGA: absent from MINIT */
              archive_close; target_screen_680F(); }
    if (!ok) { message_box("Could Not Open Data File."); exit_game(1); }          /* DS:6A41 */
    music_stop(); W(9AFA) = 0; fill(0, 0, 319, 199);
    sprite(&rom, 0, 0); free_86A0(&rom);                  /* the she-wolf, on the screen */
    timer0_reset(); music_play(0);                        /* cn1 (CENT), loaded by B18A */
    while (!skip && ticks0() < 504) if (key_read()) skip = -1;                    /* 03AA only: no colour cycle */
    if (!skip) {
        W(9B69) = 6; y = 5;                               /* brown, palette 6 = (2A,15,00) */
        for (k = 0; k < 8; k++) {                         /* DS:6A5C: the legend, 8 strings */
            timer_start(1); text_899C(line[k], 5, y); y += B(9B5C) + B(9B5D);       /* 8 + 3 */
            while (!skip && ticks(1) < 144) if (key_read()) skip = -1;            /* after a key: the rest at once */
        }
        while (!skip && ticks0() < 2664) if (key_read()) skip = -1;
        if (!skip) {
            restore_6973(0, 0, 319, 199);                 /* the world from the work buffer */
            sprite(&tit, 0, 64);                          /* hot (-4, -10): at (4, 74), transparent nibble 0 */
            while (!skip && ticks0() < 3024) if (B8B7()) skip = -1;               /* the sea cycles (VGA) */
        }
    }
    free_86A0(&tit);
    if (!skip) { restore_6973(0, 0, 319, 199); B970(); }  /* the title erased; 11 credits; B970's -1 ignored */
    frame30e6(84, 40);                                    /* always, also over the she-wolf after an early key */
    r = text_rect_8C9F(DS:6619, 0, 0, 159, 99); W(9B69) = 15;
    centred_text_8BE4(DS:6619, 159, r.y0);                /* "It is 400 years after the\nFounding, ..." */
    timer0_reset();
    while (ticks0() < 720) if (B8B7()) break;
    B8CF();  music_stop();                                /* DAC A2..A6 back; B18A then unloads cn1 */
}
void B970(void) { frame30e6(84, 40); for (p = 6A6E; p < 6A9A; p += 4) if (B9D3(p, 288) == -1) return -1; return 0; }
void B8CF(void) { if (VGA) { wait_vretrace(); for (k = 0; k < 5; k++) set_dac(W(6AD8 + 2k), rgb DS:6ABA + 6k); } }
```

Timeline with no key (timer 0 from the music start, 72.8 Hz):

| tick | s | screen |
|---|---|---|
| 0 | 0 | black, then ROMULUS.CBM (320x200, LUT 6) on the screen; music cn1 starts |
| 504 + 144k, k = 0..7 | 6.9 .. 20.8 | legend line k at (5, 5 + 11k) in colour 6; widest line ends at x 310 (D, font widths DS:9C9C) |
| 2664 | 36.6 | WORLD.LZW (320x200 raw indices, 30 colours) + TITLE.CBM (313x51, LUT 18) at (4, 74); the sea (DAC A2..A6, 20847 px) cycles every 5 ticks (b907..b919: when last + 4 < now; V: the DAC writes fall at 2664, 2669, 2674 ... 6909, then B8CF at 6912) |
| 3024 | 41.5 | world again (title erased), frame 30E6 (84, 40); credits 1..11, 288 ticks (4.0 s) each, the sea still cycling |
| 6192 | 85.0 | frame redrawn, the closing text centred on (159, 99) in white |
| +720 | 94.9 | DAC A2..A6 restored, music stopped; B18A goes on; main fills black and draws the map |

The 11 intro credits are DS:6A6E..6A96: Game Design, Programming, Graphic Artists, Music Composition, Music Programming,
Producers, Associate Producer, Assistant Producer, Tools, Tools, Project Director. The other 7 pairs and "Beware the ides of
March" appear only in the end credits (B9A1).

**Skipping**: keys only (03AA; a mouse click does nothing). The first key in any phase jumps to the closing box (the
remaining legend lines are printed at once on the way; after a key in the she-wolf or legend phase the world, the title and
the credits are never shown and the box lands on the she-wolf picture); a second key ends the closing box. Window closed in
the port = a key (as credits.c).

**1990 bugs in the intro**: only B8FE's mod 4 (credits.c fix already in place: with it the sea cycles all five colours;
--original keeps mod 4, where DAC A6 never moves). B8CF writes the cmap values back (DS:6ABA = CMAP A2..A6, checked). Kept
as 1990 (not bugs, G): the early-key path above; the text "400 years after the Founding" while the game starts in 275 BC
(478 years after 753 BC; no text of the game gives a founding year, so no contradiction with the game itself). 9772 sets 256
DAC entries from the 750-byte CMAP.BIN buffer: entries 250..255 get whatever follows it in memory (D; unused, G).

## 5. The archives

- **MINIT** (archive 4): INIT2DAT/INIT2HDR (sheet "init2" → [C28A]: the four crowd portraits, HANDOFF 2026-11-07),
  INITDAT/INITHDR (sheet "init": 14 sprites, sprite 0 = the mouse cursor, the others the map icons), ROMULUS.CBM, TITLE.CBM,
  WORLD.LZW. All seven used; WORLD.CBM (EGA) is referenced but not in the archive (this release ships only the VGA "m"
  archives; the EGA ones would be "einit", "emap", ...). Every INIT pointer DS:6350[k] has a reader in the code (byte search).
- **CENT** (archive 3): CMAP.BIN (palette), CN1.LZW (music 0, the intro theme: DS:3EF0[0] = "cn1", archive DS:3EF2[0] = 3),
  MMENU (VGA frame sheet) / EMENU (EGA), NEWGAME.LZW (Game Control New Game), PLACES.LZW (the question), ADLIB / ROLAND /
  SPEAKER / TANDY (sound drivers, music numbers 18 / 19 / 17 / 20).

## 6. Port start vs 1990 (D unless marked)

Today a double-click (or `--play`) goes straight to the map: state from NEWGAME.LZW, map_startup, map_loop (V: a
headless `--play --seed 1 --script` shot at the first input is the 275 BC map, as s03.png minus the cursor). 1990 shows
the question (black screen, grey box), then ~95 s of intro, then the map from the static DS + B397.

| 1990 (B0E0 / B18A) | port |
|---|---|
| switches k, x/m/e/tandy/v, nosnd/pc/tn/roland/mt/lapc/adlib/ad | main.c: `nosnd` turns the music off; `adlib` / `ad` / `v` are what the port does anyway; the others are noted on stderr and ignored (2026-11-17) |
| ISRs, 72.8 Hz timer | nv_tick paces 72.83 Hz in a window |
| font record, VGA colours 6344/6346/6348, mouse prompt [3E40] | map_startup (same values) |
| B466: question, **one dice roll**, archive names decrypted, key wiped | skipped: the port's dice at the map are one roll behind a 1990 run with the same BIOS seed; DS:63EE..6418 / 6444 stay scrambled (the port opens archives by literal names, so nothing reads them) |
| cmap.bin → DAC (256 entries) | screen_open loads CMAP.BIN (250) |
| MMENU sheet [C0F6], INIT2 sheet [C28A] | province.c loads the sheets itself; W(C0F6) / W(C28A) stay 0 |
| music_load(0), sound driver | music.c (the AdLib driver and an OPL2, in a window game); intro_run loads / unloads CN1 |
| **B694 intro** | **intro.c** (2026-11-17) |
| INIT sheet → DS:6350 pointers | map_startup (descriptors at DS:CE48 + 14k as in the snapshots) |
| cursor buffer, warp (160, 100), cursor = INIT sprite 0 ([684] = [C0E2] = CE48 in the snapshots) | map.c map_startup (B2CF..B2E4), the cursor in screen.c (docs/CURSOR_NOTES.md, ported 2026-11-17) |
| C0C6 second buffer, load_picture(0) MAP | map.c map_image_load |
| static DS + B397(3..5) | new_game() = NEWGAME.LZW: same bytes except the description (GAMELOOP_NOTES 1 and its verifier corrections, V there) |

## 7. Where the intro goes (design for the implementer)

- **Code**: credits.c (it is section 1, and B694 needs its statics poll_B8B7 / cycle_B8FE / credit_B9D3): add B694,
  B970 and B8CF there, literal, addresses in the comments. Pictures: the pic_t loader of race_gfx.c (1273 + 8E62 + 86A0
  of a CBM member; it is static there: share it or repeat the three small functions) on MINIT (assets_load("MINIT.DIR",
  "MINIT.DAT")); WORLD.LZW: assets_decode → 64000 bytes into g_wrk (0F0C into the work buffer), shown by nv_restore_6973.
  Text: the 899C / 8BE4 / 8C9F equivalents credits.c and race_gfx.c already use. Music: the DS parts (music_load_5D73(0)
  before, music_play_5D29(0), the stops, music_unload(0) after), as the other screens do.
- **Call**: main.c play_game, after map_startup() (which supplies what B18A had set before B694: the font record, [6346]
  = 1Ah for the frame, [9B26] = 2 for the cycle) and before map_loop. The intro writes no game state (its DS writes are
  9AFA, 9B69, 6AE2..6AE6, the timer slots, the music slots), so running it after new_game() instead of before B397 changes
  nothing (D).
- **When**: only for an interactive start: no `--script` (and, G / Erik's call, no `--load`: 1990 has no load before the
  intro, a load happens later from Game Control, and --load is a port-only test start). Justification: 1990 shows it at
  every start and has no switch for it (B0E0 has none); only a key cuts it short. Scripted runs skip it, so every existing
  script and check is unchanged: they all start at the map's first input, and the battle / naval / End Turn / zone hooks
  never reach play_game. Optional, for a headless check of the intro: an opt-in `--intro` that runs it in a scripted run
  too ("wait N", "key", "shot" lines drive and capture it). The B466 dice roll is not to be added (it would shift every
  scripted --seed run).
- **Bugs list for its header**: fix 1 = B8FE mod 4 (shared with the credits, already there). Nothing else found.
- Decisions for Erik (not bugs): a mouse click to skip (1990: keys only); intro with --load.
- As built (2026-11-17, review): the intro's DS writes also include the drawing target DS:C0D0..C0E1 (67E4 / 680F) and,
  in a window, the mouse words DS:B2 / B6 / B8 (credits_poll reads the mouse; 1990's B694 reads keys only); both are
  rewritten before they are read. With `--intro --load` a DS whose arrow is visible ([686] = FFFF, every DOSBox image)
  has it hidden around the intro (intro_run, port-only): 1990 sets the cursor up only after B694 (V: the title shot of
  `--load t10_in.bin --intro` was 93 px off the independent reference at (250,10), now 0 px).
- 0183 (the timer slots' elapsed time) returns FFFF when the high words of DS:42/44 differ, so a 16-bit wrap of DS:42
  during the intro would end every running wait at once (D). Not reachable at a normal start (DS:42 starts at 0, the
  intro ends by tick 6912); only `--load .bin --intro` could. No action.

## 8. Easter eggs, hidden texts (all kept)

- The name key **"kenny"** (DS:6444, stored XOR 47h as `,"))>`), probably Ken Zarifes of the credits' "Programming" (G),
  wiped with EAh bytes once used.
- The copy-protection bypass byte EB at B5F6 (G: a crack in this copy, not 1990's).
- Undocumented-looking start switches (B0E0): `k` (no mouse), `x m e tandy v` (video), `nosnd pc tn roland mt lapc adlib ad`
  (sound). Whether the manual lists them: not checked.
- The intro shows the tools credits ("Soft-ICE & MagicCV by Nu-Mega", "Plink-86plus by Polytron", "MSC v5.1, v6.0 & MASM
  v5.1 by Microsoft", "Deluxe Paint II ... by Dan Silva & Steve Shaw"); "Beware the ides of March" is end-credits only.
- No unused text: every string in section 1's data (DS:6280..6AEF) has a reference; no unused picture in MINIT.

## 9. Not verified

No capture of the intro exists (the hunt shots start at the question and the map); the timings, positions and colours
above are D. To verify: a DOSBox-X capture of the intro screens (needs Erik: DOSBox must not be started without him).
