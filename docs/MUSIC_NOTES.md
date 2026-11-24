# Music and sound (Claude, 2026-11-17)

What the game plays, how, and on what; the AdLib driver decoded to the last byte, so it can be ported (sequencer in C,
own OPL2 emulation, SDL2 audio). D = decoded from the code, V = verified by a run or a capture, G = guess.

**Port status (2026-11-17, Claude): ported.** `port/src/opl2.c` (the YM3812, from scratch), `port/src/music.c` (the
driver 0x3E8..0x979 literally on the ADLIB.LZW image, the game's 5D73 / 5D29 / 5D20 / 5E1A / 072E / 074B / 5E86 / ISR /
03FF, `--music-wav`), SDL2 audio in `sdl2_platform.c`, the battle's music calls in `battle.c`, the word `nosnd` and
Ctrl+S. The music plays in a window game only; headless runs open no device and their results are unchanged.
- V: `--original --music-wav N OUT.txt 100` equals `python tools/adlib_regs.py N 100` byte for byte for all 17 songs;
  without `--original` only the bug 1 / bug 2 writes differ (CN1 8 lines, CN4C 160, CN8A 4, CN8B 13, CN9B 128, CN9C 80,
  CN9D 100 in 100 s). Pass lengths as in section 4 (the stderr of `--music-wav` reports every loop and the stop).
- V: after a battle the slot table DS:A6B0..A6BF is the DOSBox capture's (endturn t9_out: slots 1 and 2 = {0, 0, FF}).
- G (not heard on hardware or DOSBox): the OPL2's sound. Rendered WAVs: no clipping (peaks -10..-20 dBFS), loops repeat
  (RMS envelope correlation 0.89..0.996 between passes), pitches right (CN9B note 84 = 2088 Hz, CN8B note 60 = 261 Hz).
  The chip's output level is quiet (RMS -26..-38 dBFS) and has a DC offset from waveforms 1 / 2 (as the chip has).
- Not run: the window (the audio device, the threads, Ctrl+S, the 5.9 s fanfare wait in real time).

Sources: `disasm/ADLIB_DRV.asm` (new: the driver ADLIB.LZW, annotated), `disasm/EXE_RUNTIME.clean.asm` (EXE 0823),
OVL0 section 1 (start-up, own llvm-objdump listing of OVL0.OVL file 0x000-0x9A0 at 0823:B0E0), the section listings
`disasm/OVL_*.asm`, the static DS (OVL0.OVL file 0xB400 = DS:0000) and the DOSBox RAM dumps in `tools/dosbox/results`.
Tool: `tools/adlib_regs.py` (reference model of the driver: register log, check against the RAM dumps).

## 1. Summary

- **Devices (D):** PC speaker, AdLib, Roland MT-32 / LAPC-I (MPU-401 UART at 0x330), Tandy 3-voice (SN76496 at port
  0xC0), or none. One driver per device, each a file in CENT.DAT: `SPEAKER.LZW` (730 bytes unpacked), `ADLIB.LZW`
  (2426), `ROLAND.LZW` (884), `TANDY.LZW` (1711). Default = PC speaker; an AdLib found by the EXE's detection wins; a
  word on the command line wins over both (section 2).
- **Sound effects: none (D).** Neither the EXE nor any OVL0 section touches port 0x61/0x42/0x43 or beeps (no BIOS/DOS
  BEL either); the only PIT access is the timer set-up. All sound is music, played by the driver.
- **Timer (D):** the game reprograms PIT channel 0 to counter 0x3FFF = **72.83 Hz** (1193181.67 / 16383) and calls the
  driver's tick from its INT 08 handler on every interrupt; every 4th interrupt chains to the BIOS handler (int 64h).
- **Songs (D):** 17 songs CN*.LZW (no CN5), one format for all devices: a MIDI-like event stream with 1-byte deltas in
  driver ticks, one header of 0x7F bytes holding a channel map per device. No tempo events: the tempo is the tick rate.
  No pitch bends, no running status, no note-off events (always note-on with velocity 0) are used; the only controller
  is CC7 (volume), which the AdLib driver ignores.
- **AdLib (D, V):** 9 two-operator FM melodic channels, FM connection only, waveforms 0-2, no rhythm mode. 24 built-in
  patches (MIDI program 0..23) inside the driver; 4 fixed drums on channels 5..8. The songs do not carry instruments.
- **Port:** literal C port of the driver's code, working on the ADLIB.LZW image loaded from CENT.DAT like the game does;
  own OPL2 at its native 49716 Hz, one driver tick every 682.625 samples; SDL2 audio. Two 1990 bugs fixed by default
  (section 8). Headless check: `--music-wav SONG OUT.wav SECONDS` (section 10).

## 2. Device choice and driver loading (D)

### 2.1 Command line: section 1 B0E0 (thunk B086, called from main 0x07B4 with argc/argv)
```c
void B0E0(char **argv, int argc) {
    if (W(0x9AB2) == 6) B(0x3EEA) = 0x14;          /* display type 6 (G: Tandy video) -> Tandy sound */
    if (adlib_detect_055C()) B(0x3EEA) = 0x12;      /* AdLib on port 388h -> AdLib */
    for (i = 1; i < argc; i++) {                    /* whole words, no slash; compare A186 (G: strcmp) */
        if (!cmp(argv[i], "k")) W(0xBA) = 0;        /* keyboard only: mouse off */
        for (j = 0; j < 5; j++) if (!cmp(argv[i], DS:6304[j])) W(0x9AB2) = DS:630E[j];   /* x m e tandy v -> 0 2 4 6 8 */
        for (j = 0; j < 8; j++) if (!cmp(argv[i], DS:6318[j])) B(0x3EEA) = DS:6328[j];
    }
}
```
DS:6318 / DS:6328, the sound words: `nosnd` FF, `pc` 11, `tn` 14, `roland` 13, `mt` 13, `lapc` 13, `adlib` 12, `ad` 12.
Static [3EEA] = 0x11 (PC speaker). [3EEA] is an index into the song table (section 4): 0x11 "speaker", 0x12 "adlib",
0x13 "roland", 0x14 "tandy"; FF = no sound. Every DOSBox capture has [3EEA] = 0x12 (V: AdLib found, no word given).

**AdLib detection, EXE 055C (D):** the usual timer test on 388h/389h: reg 4 = 60h, reg 4 = 80h, read status (stored,
never compared), reg 2 = FFh, reg 4 = 21h, `loop` 0xFFFF times, read status: AdLib if `status & E0h == C0h`; reset the
timers again. (The first status read is not checked, unlike Ad Lib's own recipe: harmless.)

### 2.2 Loading: 5E86 (called from section 1 B255, the init B18A)
```c
void sound_init_5E86(void) {
    B(0x3EE9) = 0;                                  /* music enable flag */
    if (B(0x3EEA) == 0xFF) return;                  /* nosnd: [3EEC] stays 0 */
    W(0xC6) = load_5EB5(B(0x3EEA));                 /* far pointer DS:00C4 = segment:0000 */
    if (W(0xC6)) { B(0x3EE9) = 0xFF; W(0x3EEC) = 0xFFFF; } else W(0x3EEC) = 0;
}
```
`load_5EB5(n)`: returns 0 at once when `W(0x3EEC) == 0 && n < 0x11` (songs are not loaded with the sound off; drivers
always are); else name = DS:3EF0[n].name + ".lzw" (DS:3F44), archive DS:3EF0[n].archive is opened (0F7B), the member
is looked up (18C2), must have format bit 0x80 and an unpacked size < 0xFFFF (1172), gets ceil(size/16) paragraphs from
DOS (863E), is unpacked to seg:0000 (01DC), archive closed (11A9); returns the segment or 0. The driver is thus a raw
image at offset 0 of its own segment (no EXE/COM header); the DOSBox runs have it at 3682:0000 (V: [C4] = 3682:0000;
the code bytes 0x3E8.. in RAM equal ADLIB.LZW in all three 640K dumps).

Then B25B loads song 0 (cn1) and the intro plays it (section 4). Exit 0EE7: `[3EE9] = 0; 5D20(); 00AB` (timer back).

### 2.3 Timer and ISR (D)
- **0x0062** (from section 1 B19A): if not done yet, saves INT 08 (DS:86/88) and installs it as INT 64h; INT 08 =
  0823:**0001** (the `clc` at 0000 is skipped); PIT: `out 43h, B6h; out 40h, FFh; out 40h, 3Fh` -> counter 0x3FFF.
- **0x00AB** (exit): INT 08 back, `out 43h, B6h; out 40h, FFh; out 40h, FFh`.
- Quirk (D, harmless, not ported): control word B6h selects PIT channel **2** (the speaker), mode 3; the counter bytes
  go to port 40h = channel 0, which keeps the lo/hi access mode the BIOS gave it, so channel 0 does get 0x3FFF / 0xFFFF.
- **ISR 0823:0001** (D):
```c
W32(0x42)++;                                         /* the game's clock: 72.83 ticks per second */
if (--B(0x8A) == 0) { B(0x8A) = 4; int 64h; }        /* BIOS clock every 4th (it sends its own EOI: double EOI) */
if (B(0x3EEC)) {                                     /* sound on (byte test of the word) */
    if (B(0x3EE9) != B(0x8B)) { B(0x8B) = B(0x3EE9); driver(B(0x3EE9) ? 4 /*enable*/ : 3 /*disable*/); }
    driver(0);                                       /* tick, every interrupt */
}
outb(0x20, 0x20);
```
Static [8A] = 4, [8B] = 0, [3EE9] = 0. So the first interrupt after 5E86 sends AH=4 (V: [8B] = FF in every capture).

### 2.4 Ctrl+S (D)
INT 9 handler 0x03FF: make code 1Fh with Ctrl held (BIOS 40:17 bit 2) -> `[3EE9] ^= 0xFF` (MENU_NOTES 4). The ISR then
sends AH=3 (keys off on all 9 channels; the driver stops keying notes on) or AH=4. The sequencer keeps running while
muted, so the music comes back in time, at the next note-on. Exit sets [3EE9] = 0 (AH=3) before stopping.

## 3. The driver interface (D; all four drivers share it)

The file starts with a jump table: +0 dispatcher, +3 start, +6/+9/+C/+F stop (AdLib: 0x3E8, 0x403, 0x470 x4). The game
only uses +0: `lcall [0xC4]` with AH = function (table at driver 0x3DE; no range check):

| AH | AdLib | caller | does |
|---|---|---|---|
| 0 | 4A9 | ISR, every PIT tick, when [3EEC] != 0 | tick: advance the song (section 5) |
| 1 | 403 | EXE 072E(seg): `if ([3EEC] && seg) { CX = seg; AH = 1 }` | start the song at CX:0000 from its beginning |
| 2 | 470 | EXE 074B: `if ([3EEC]) AH = 2` | stop: playing = 0, chip reset (all TL = 3F, keys off: silent at once) |
| 3 | 483 | ISR when [3EE9] became 0 | disable: [3DD] = 0, B0..B8 = 0 (keys off: notes release with their RR) |
| 4 | 47B | ISR when [3EE9] became nonzero | enable: [3DD] = 1 (static value of [3DD] is 1) |

The driver saves BX CX DX DI SI DS, sets DS = CS (its variables are in its own image) and returns with `retf`. AH=1/2/3
run with interrupts off (`cli ... sti`). The driver does not hook interrupts or touch the PIT.

Game side (EXE, D; the DS parts are already ported in map.c / glad.c / naval.c / nego.c / race.c / legions.c):
- **5D73 load(n):** if no slot of DS:A6B0 (4 x {u16 seg, u8 loaded, u8 id}) has loaded && id == n: the first slot with
  loaded == 0 gets seg = 5EB5(n), and if seg != 0 also id = n, loaded = FF.
- **5D29 play(n):** 5D73(n); if `[3EEE] != n`: for every slot with id == n (loaded not tested): 072E(seg), [3EEE] = n.
  So playing the current song again does nothing, and the game does not know when a one-shot song has ended.
- **5D20 stop:** [3EEE] = FF; 074B.
- **5E1A unload(n):** the first slot with loaded && id == n: DOS free (868C), slot = {0, 0, FF}, [3EEE] = FF. It does
  **not** stop the driver; every caller stops first or the music is already stopped (glad B188: from the Rome screen,
  which stopped it at EXE 0866).
- With [3EEC] = 0 (nosnd, or no driver): no song is ever loaded (seg 0, the slot stays free), 072E/074B do nothing,
  the ISR never calls the driver, and the battle skips its fanfare wait ([3EE9] != FF).

## 4. The songs (D for the places, V for the lengths from the files; DS:3EF0 table {name, archive})

Archive numbers (0F7B, DS:029E): 0 MRACE, 2 BATL, 3 CENT, 5 MMAP, 6 MGLAD, 7 MNEGO, 8 MNAVAL. "Pass" = one run through
the stream at 72.83 ticks/s; loop songs restart at once (FC 80 jumps to the first event in the same tick).

| n | file | archive | end | one pass | where it plays |
|---|---|---|---|---|---|
| 0 | CN1 | CENT | stop | 6800 ticks, 93.4 s | **title**: loaded at init (B25B), played by the intro B694 (B72F stop, B75F play, with the she-wolf picture), stopped when the intro ends or a key ends it (B8AE); unloaded B26B. The intro without a key lasts 6912 ticks (INTRO_NOTES): the theme fits that length (G: written for it) and ends by itself at tick 6800 (on the MT-32 with the bass fading out over ticks 6732-6754), 1.5 s before the intro's own stop. **Victory**: DF58 loads it, the triumph BF6A(1) plays it, the credits B9A1 follow while it runs on (nothing stops it) |
| 1 | CN2 | MMAP | loop | 1351 ticks, 18.6 s | **map** (Italia): map_build 494A (EXE 4A16) on every map draw; stopped by the year button / Game Control (0828), a left click on a Roman province (0866: province and Rome screens are silent), a legion's move into a province (398F, legions.c), the battle (DF31); unloaded by the naval battle (BD8A) and the gladiator show (B188); the next map draw reloads and restarts it |
| 2 | CN3A | MRACE | stop | 760 ticks, 10.4 s | **race parades**: the gallery parade before the race (BE10, BE2D) and the winner's parade BF6A(0) |
| 3 | CN3B | MRACE | loop | 1591 ticks, 21.9 s | **chariot race** (D08A, D0B8); stopped at the end (D162) and by the crash scene (D738) |
| 4 | CN4A | MGLAD | stop | 467 ticks, 6.4 s | **gladiators**: title (B1A6) and the pause before the arena (B291) |
| 5 | CN4B | MGLAD | loop | 2330 ticks, 32.0 s | VERSUS (B208), fight start (B2F0), crowd mood 1 |
| 6 | CN4C | MGLAD | loop | 1486 ticks, 20.4 s | crowd excited (mood 2, BB3B via DS:800A) |
| 7 | CN4D | MGLAD | loop | 1553 ticks, 21.3 s | gladiator selection (B1C5), crowd bored (mood 0) |
| 8 | CN6 | MNEGO | loop | 1740 ticks, 23.9 s | **Cleopatra's love theme**: her feast (EXE 62CF), stopped and unloaded at 6473/6479 |
| 9 | CN7A | BATL | stop | 291 ticks, 4.0 s | **battle fanfare, barbarians** (enemy home zone 11-14, 23-26; BB97 sets C2A6/C2A7) |
| 10 | CN7B | BATL | loop | 1450 ticks, 19.9 s | **battle, barbarians** |
| 11 | CN8A | BATL | stop | 387 ticks, 5.3 s | **battle fanfare** (everyone else) |
| 12 | CN8B | BATL | loop | 1675 ticks, 23.0 s | **battle** (everyone else) |
| 13 | CN9A | MNAVAL | loop | 1161 ticks, 15.9 s | **naval: the rowing drum**, oars level 1 (cruising); also at the start (C0FA) and the re-engage (B3BD/B41F) |
| 14 | CN9B | MNAVAL | loop | 870 ticks, 12.0 s | oars level 2 |
| 15 | CN9C | MNAVAL | loop | 696 ticks, 9.6 s | oars level 3 |
| 16 | CN9D | MNAVAL | loop | 580 ticks, 8.0 s | oars level 4 (ramming speed); also just before boarding (B38B) |
| 17-20 | SPEAKER ADLIB ROLAND TANDY | CENT | - | - | the drivers (5E86) |

Battle (OVL_BATTLE, BATTLE_NOTES 3.1): DF31 stop; BEC2/BECD load C2A6/C2A7; E052 `if ([3EE9] == 0xFF) { timer 2;
play(C2A6); wait until 432 ticks (5.93 s) }` so the fanfare (4.0 / 5.3 s) is heard whole before the battle music
`play(C2A7)` (E077); E1F4 stop + unload both; E236 stop; the report E21A plays the fanfare C2A6 again (E4ED). Naval
(NAVAL_DATA_NOTES 6): DS:9704[1..4] = 13..16; C422/C4C7 plays the song of the oar level, level 0 (stopped) = music
stop (C4CC); C34F stops and unloads 13..16. Gladiators: GLAD_SCREEN_NOTES 7. Race: RACE_SCREEN_NOTES 8.

Not used: **CN5** (no file and no table entry: the numbering skips it), song table entries beyond 16 are the drivers.
**Nothing is unused among CN1..CN9D**: every file is played.

## 5. The song format (D from the driver code; V: all 17 files parse to their last byte, sections 9)

### 5.1 Header (0x7F bytes; every song has the same layout)
| offset | size | used by | meaning |
|---|---|---|---|
| +00 | u16 | all drivers | offset of the first event = loop start (0x007F in every song) |
| +02 | 16 | ROLAND 0x0D0 | per MIDI channel 0..15: play on the MT-32 (1) or not (0) |
| +12 | 16 | ROLAND 0x0E0 | per MIDI channel, the same in every song (`00 4D 5D 7F 5D 6D 00 00 00 7F 00 ..`, G: volumes or pans): read in a loop that does nothing with them (ROLAND.LZW 0x0DE-0x0EA: `mov bl, es:[di+12h]; mov cx, di; inc di` and no call) |
| +22 | 9 | ADLIB 0x417 | per OPL2 channel 0..8: the MIDI channel it plays (9 = drums) |
| +2B | 24 | none | unused by every driver (same bytes in all songs: `01 02 09 02 03 09 04 05 09 06 FF FF FF FF 00 FF FF 00 FF FF 00 FF FF 01`) |
| +43 | 8 | TANDY 0x24C | Tandy voice map |
| +4B | 2 | SPEAKER 0x122 | PC speaker map |
| +4D | 0x32 | none | unused (also identical in all songs) |

Each song carries its arrangements side by side on different MIDI channels: e.g. CN1 plays channels 1-5 and 9 on the
MT-32, the AdLib map `02 02 02 03 03 03 01 04 05`, and channels 10-12 only for Tandy/speaker (Tandy map `0C 0B 0B 09
D0 D0 D0 00`, speaker `0A 00`). On the AdLib, events of MIDI channels that no OPL channel maps are skipped (every song
but CN7A and CN9A has some).

### 5.2 Events
Stream of `event, delta` pairs from +0x7F. Delta (1 byte, 0x58E): 0 = the next event belongs to the same tick; d > 0
= the next event is read d ticks later (the driver stores d-1 in [3CE] and counts it down, one per tick). Events:

| bytes | AdLib driver | used by the songs |
|---|---|---|
| `9c nn vv`, vv > 0 | note on, MIDI channel c, note nn, velocity vv (5E2) | yes (all notes) |
| `9c nn 00` | note off (6AB) | yes (the only note-off form used) |
| `8c nn vv` | note off (6AB) | no |
| `Cc pp` | program change: patch pp (0..23) on every OPL channel mapped to c, ignored for c = 9 (5A6) | yes, all at tick 0; CN1 changes programs mid-song |
| `Ec ll mm` | pitch bend, 14-bit ll + mm<<7: fnum x bend / 0x2000 on every sounding channel of c (7A3; 0x2000 = no change, 0 = frequency 0, 0x3FFF = almost an octave up; linear in frequency) | no |
| `Bc xx yy`, `Ac`, `Dc`, `F0..FB`, `FD..FF` + 2 bytes | skipped (2 data bytes; D0 has only 1 in MIDI: latent misparse) | Bc 07 vv only (CC7, volume): CN1 has 204 (a crescendo on channel 2 at 18-27 s and the bass fading out over the last second, both lost on the AdLib) |
| `FC 80` | end: jump back to +00 and go on in the same tick | the loop songs |
| `FC xx` (xx != 80) | end: stop playing ([3D1] = 0) | `FC 81`: CN1, CN3A, CN4A, CN7A, CN8A |
| data byte first (running status) | repeat the last status (0x4E4) | no |

### 5.3 The tick (0x4A9), state and voice allocation
Per channel v (0..8): busy [365+v], note [36E+v], patch transpose [377+v], carrier 40 byte [39B+v], A0/B0 shadows
[3A4+v]/[3AD+v], age word [3B6+2v]. Each tick (only while playing): every busy channel's age + 1; if the wait [3CE] is
not 0 it is decremented and the tick ends; else events are read until a nonzero delta.

- **Note on, c != 9:** the first channel v with map[v] == c and not busy: busy, note, `voice(v, F(note + tr[v]), vel)`.
  None free: steal the channel of c with the **largest age** (strictly > 0; ties: the lowest v): `voice(v, 0)` with
  keyon 0 (A0 = B0 = 0), age 0, then key it on with the new note. If every channel of c started this very tick (age 0)
  the note is **dropped** (CN1 drops 86 notes a pass this way; no other song).
- **Note on, c == 9 (drums, 708):** the first channel mapped to 9 whose drum note [380+v] equals nn is (re)keyed with
  `F([389+v])` (no transpose), busy or not. Drum table (D): ch5 note 54 (tambourine) index 0x49 patch 10; ch6 note 42
  (closed hi-hat) index 0x17 patch 18; ch7 note 36 (bass drum) index 0x0C patch 16; ch8 note 38 (snare) index 0x26
  patch 23. Other drum notes sound on no channel. In the songs only the snare (38) is ever heard on the AdLib: notes 46
  and 49 have no channel, 54 is used only by CN6 whose map has no drum channel, 36/42 are not in any song.
- **Note off (6AB):** the first channel of c with note == nn: busy 0, note 0, age 0, `voice(v, F(n + tr[v]))` with
  keyon 0, where n = the note, or [389+v] for drums (the drum on-key had no transpose: latent, every drum patch the
  songs use has transpose 0, so nothing to fix).
- **voice(v, word, vel) (73F):** if [3DD] != 1 the keyon bit is forced to 0 (muted). With keyon: `40+car[v] =
  [39B+v] - vel/4` (8-bit subtract, see bug 2). Then `A0+v = word & FF`, `B0+v = word >> 8 | keyon(20h)`.
- **F(i)** = word at `0x14 + ((i << 1) & 0xFF)` (the `shl bl` is 8-bit): the frequency table, 96 words
  `block << 10 | fnum`, fnum C..B = 158 16C 182 199 1B1 1CB 1E6 203 222 242 265 289 for blocks 0..7. i = 48 is
  261.0 Hz (fnum 0x158, block 4). A patch with transpose -12 thus plays MIDI pitch (note 60 = C4); transpose 0 plays
  an octave higher. Out of range i reads the bytes after the table: bug 1.
- **Program change (7FB):** record = DS-relative pointer 0x167[0x14F[pp]] (0x14F is the identity 0..23): writes
  20/40/60/80 of the modulator, 20/40/60/80 of the carrier, C0+v, E0 mod, E0 car; keeps the carrier 40 byte and the
  transpose. Programs > 23 would read past the map (none is used on a mapped channel; CN4B's program 49 is on MIDI 11,
  which the AdLib map does not contain).
- **Start (403):** read pointer = loop start = song +0; map = song +22..+2A; chip reset; all channels free, note 0,
  age 0; channels mapped to 9 get their drum patch [392+v]; wait 0; playing. Shadows, transposes and carrier bytes of
  channels the new song does not touch keep the old song's values (V, section 9); they are only read by a pitch bend.
- **Chip reset (929):** 01 = 20h (waveform select on), 04 = E0h, then 0 into the 101 registers of 0x0D6 (08, BD,
  20-35, 60-75, 80-95, A0-A8, B0-B8, C0-C8, E0-F5), then 3Fh into 40-55.

Dead code in the driver: 0x90B, a random number routine (LFSR on 0x14D, seed 0x563A, taps 0xB400) nobody calls; the
same seed word sits at ROLAND.LZW 0x6C. 195 bytes of patch-like data in 0x21B..0x349 that no pointer reaches (G: the
rest of a bigger instrument bank).

## 6. The AdLib patches (D; driver 0x167, record layout in ADLIB_DRV.asm)

AM/VIB/EG/KSR/MUL = register 20, KSL/TL = 40, AR/DR = 60, SL/RR = 80, WS = E0; FB = C0 bits 1-3; CONN (C0 bit 0) is 0
in every patch (FM). "tr" = transpose in semitones. "used by" = the songs that load it on a mapped channel.

| # | rec | modulator | carrier | FB | tr | used by (AdLib) |
|---|---|---|---|---|---|---|
| 0 | 1F7 | AM1 VIB1 EG1 KSR0 MUL0 KSL1 TL21 AR7 DR7 SL0 RR3 WS2 | AM0 VIB1 EG1 KSR0 MUL1 KSL0 TL34 AR3 DR7 SL0 RR15 WS0 | 5 | -12 | cn1 cn2 |
| 1 | 1D3 | AM0 VIB1 EG1 KSR1 MUL0 KSL0 TL28 AR7 DR1 SL0 RR3 WS2 | AM0 VIB0 EG1 KSR0 MUL0 KSL0 TL29 AR6 DR4 SL3 RR7 WS2 | 7 | -12 | cn6 cn9c cn9d |
| 2 | 1D3 | = patch 1 | | 7 | -12 | cn1 cn3a cn3b cn4a-d cn7b cn8a cn8b |
| 3 | 203 | AM0 VIB0 EG1 KSR1 MUL1 KSL0 TL28 AR5 DR1 SL0 RR3 WS0 | AM0 VIB1 EG1 KSR0 MUL1 KSL2 TL27 AR5 DR4 SL6 RR7 WS0 | 7 | +0 | cn6 |
| 4 | 1DF | AM0 VIB1 EG1 KSR1 MUL0 KSL0 TL28 AR7 DR1 SL0 RR3 WS2 | AM0 VIB0 EG1 KSR0 MUL0 KSL0 TL31 AR6 DR4 SL3 RR7 WS2 | 7 | -12 | cn3b |
| 5 | 1D3 | = patch 1 | | 7 | -12 | cn1 cn2 cn3a cn3b cn4a-d cn6 cn7a cn7b cn9a-d |
| 6 | 1BB | AM0 VIB0 EG0 KSR0 MUL2 KSL0 TL43 AR15 DR5 SL7 RR5 WS0 | AM0 VIB0 EG1 KSR0 MUL1 KSL2 TL28 AR15 DR5 SL5 RR4 WS2 | 0 | -12 | cn1 |
| 7 | 1BB | = patch 6 | | 0 | -12 | cn6 |
| 8 | 1AF | AM0 VIB0 EG1 KSR0 MUL3 KSL0 TL46 AR15 DR2 SL2 RR5 WS0 | AM0 VIB0 EG1 KSR1 MUL2 KSL0 TL40 AR15 DR2 SL15 RR5 WS1 | 0 | -24 | cn1 |
| 9 | 29D | AM1 VIB1 EG1 KSR1 MUL2 KSL0 TL43 AR4 DR1 SL1 RR11 WS0 | AM1 VIB1 EG1 KSR0 MUL0 KSL0 TL29 AR5 DR2 SL1 RR11 WS0 | 6 | +0 | cn6 |
| 10 | 265 | AM1 VIB1 EG1 KSR0 MUL2 KSL0 TL17 AR10 DR14 SL8 RR15 WS0 | AM0 VIB1 EG1 KSR0 MUL1 KSL0 TL40 AR8 DR5 SL2 RR10 WS0 | 7 | +0 | cn3b cn7b cn8b cn9d; drum ch5 |
| 11 | 197 | AM0 VIB0 EG1 KSR1 MUL2 KSL2 TL26 AR5 DR1 SL1 RR11 WS0 | AM0 VIB1 EG1 KSR0 MUL1 KSL2 TL37 AR10 DR2 SL3 RR11 WS0 | 6 | +0 | cn1 cn3b |
| 12 | 1C7 | AM0 VIB0 EG1 KSR1 MUL1 KSL0 TL22 AR6 DR14 SL1 RR3 WS1 | AM1 VIB0 EG1 KSR0 MUL3 KSL0 TL33 AR6 DR11 SL1 RR9 WS2 | 5 | -12 | cn6 |
| 13 | 2FD | AM0 VIB0 EG1 KSR1 MUL1 KSL0 TL28 AR4 DR1 SL0 RR11 WS0 | AM0 VIB1 EG1 KSR0 MUL1 KSL2 TL29 AR9 DR2 SL3 RR11 WS0 | 7 | -12 | cn8b cn9b-d |
| 14 | 20F | AM1 VIB0 EG1 KSR1 MUL1 KSL0 TL28 AR6 DR1 SL1 RR15 WS0 | AM0 VIB1 EG1 KSR0 MUL1 KSL2 TL33 AR9 DR2 SL1 RR11 WS0 | 7 | -12 | cn1 cn9a-d |
| 15 | 1A3 | AM0 VIB0 EG1 KSR0 MUL1 KSL2 TL28 AR7 DR3 SL5 RR10 WS0 | AM0 VIB0 EG1 KSR0 MUL1 KSL2 TL31 AR15 DR10 SL1 RR10 WS0 | 6 | -12 | cn1 cn3a cn4a-d cn7a cn7b cn8a cn8b |
| 16 | 27B | AM0 VIB0 EG1 KSR0 MUL1 KSL0 TL24 AR6 DR3 SL4 RR12 WS0 | AM0 VIB0 EG1 KSR0 MUL1 KSL2 TL29 AR12 DR5 SL0 RR15 WS0 | 6 | -12 | cn1 cn8a cn8b; drum ch7 |
| 17 | 2FD | = patch 13 | | 7 | -12 | cn1 cn3a cn3b cn4a-d cn7b cn8a cn8b |
| 18 | 2BF | AM0 VIB0 EG0 KSR0 MUL7 KSL1 TL19 AR13 DR10 SL2 RR5 WS0 | AM0 VIB0 EG0 KSR0 MUL1 KSL0 TL39 AR15 DR9 SL1 RR5 WS0 | 5 | +0 | cn9b-d; drum ch6 |
| 19 | 2B3 | AM0 VIB0 EG0 KSR0 MUL5 KSL1 TL14 AR13 DR12 SL2 RR5 WS0 | AM0 VIB0 EG0 KSR0 MUL1 KSL0 TL41 AR15 DR10 SL1 RR5 WS0 | 5 | +12 | cn9b |
| 20 | 291 | AM0 VIB1 EG1 KSR0 MUL2 KSL0 TL0 AR15 DR0 SL15 RR5 WS0 | AM0 VIB0 EG0 KSR0 MUL14 KSL0 TL35 AR15 DR6 SL15 RR8 WS0 | 7 | -36 | cn1 cn4a cn4c cn7a cn7b cn8a cn9a-d (the low drum of the rowers) |
| 21 | 22F | AM0 VIB0 EG0 KSR0 MUL7 KSL1 TL15 AR15 DR2 SL6 RR0 WS0 | AM0 VIB0 EG0 KSR1 MUL2 KSL0 TL42 AR15 DR2 SL7 RR2 WS0 | 4 | +0 | cn6 |
| 22 | 245 | AM1 VIB1 EG1 KSR1 MUL2 KSL1 TL11 AR3 DR1 SL0 RR3 WS0 | AM1 VIB1 EG1 KSR1 MUL4 KSL0 TL33 AR5 DR3 SL2 RR12 WS0 | 2 | -24 | - (cn8b's only use is on an unmapped channel) |
| 23 | 1EB | AM0 VIB0 EG0 KSR0 MUL0 KSL0 TL3 AR15 DR0 SL15 RR0 WS0 | AM0 VIB0 EG0 KSR0 MUL0 KSL0 TL31 AR15 DR6 SL15 RR7 WS0 | 7 | +0 | drum ch8 (the snare, in 11 songs) |

## 7. What the OPL2 must do (D: every register value the driver can write, over all songs, from the model)

- Registers written: 01 (= 20h only), 04 (= E0h only), 08 (= 0), BD (= 0), 20-35, 40-55, 60-75, 80-95, A0-A8, B0-B8,
  C0-C8, E0-F5. Never read (no status, no timers needed: the detection is the EXE's, the port skips it).
- **Melodic mode only:** BD = 0 (no rhythm mode; also AM depth 1 dB and vibrato depth 7 cents: the shallow ones),
  08 = 0 (no CSM; NTS = 0, so the key-scale "note" is fnum bit 9).
- 9 channels, 2 operators each, **CONN = 0 (FM) only**; feedback 0, 2(unused patch), 4, 5, 6, 7.
- Waveforms **0, 1, 2** (01 = 20h enables them); 3 never.
- Register 20: AM and VIB both used, EG type both (sustained and percussive), KSR both, MULT 0, 1, 2, 3, 5, 7, 14 (4 in
  the unused patch 22). Register 40: KSL 0-2 and any TL. 60/80: any AR/DR/SL/RR (AR 3..15, DR 0..14, RR 0..15).
- A0/B0: any fnum and block 0..7; B0 bits 6-7 can be set by the garbage of bug 1 (ignored by the chip).
- Write order to honour: a note-on writes 40, A0, then B0 with keyon; a steal writes A0 = 0, B0 = 0, then 40, A0, B0
  (key off then on in the same tick: the envelope restarts its attack from where it is; the phase restarts at key-on).
  A program change rewrites the operators of a sounding channel in place (no key-off).
- All writes of one driver tick happen at the same sample position (the port need not model the I/O delays 0x903 /
  0x921).

## 8. 1990 bugs (fix by default, `--original` keeps them) and quirks

**Bug 1 (D, measured with the model): frequency-table overrun.** F(i) has no range check. `i = note + transpose`
(8-bit) outside 0..95 reads the bytes after the table (0x0D4.., the reset register list) as a frequency word:
- i = 96 -> word 0000: fnum 0, block 0: the note is **silent**. CN9B (MIDI ch2 note 96 with patch 18, ch1 note 84 with
  patch 19 +12: 4 notes a pass), CN9C and CN9D (ch2 note 96: 2 a pass): the top notes of the high part of the rowing
  music are missing.
- i = -5, -3 (251, 253) -> words 9392 / A1A0: CN4C, MIDI ch8 notes 31 and 33 with patch 20 (transpose -36): 4 + 4 a
  pass. 9392 keys **block 4 fnum 0x392 (about 693 Hz x MUL)**: the deep drum of the excited crowd turns into a high
  tick; A1A0 gives block 0 fnum 0x1A0.
- i = -2 (254) -> word A3A2 = block 0 fnum 0x3A2: CN8A (1 a pass) and CN1 (2 a pass), patch 20 again.
Fix: for i < 0 use the word of i + 12 with its fnum halved (block 0); for i > 95 the word of i - 12 with its fnum
doubled (block 7; fits up to i = 102; the songs need only 96). `--original`: read the driver image as the code does.

**Bug 2 (D, measured): velocity underflow.** 73F writes `40+car = [39B] - vel/4` as one byte, KSL bits included. When
the patch's carrier TL is below vel/4 the subtraction borrows into KSL and the note comes out at TL 62-63, i.e.
**silent instead of loudest**. CN8B (the civilized battle loop): MIDI ch1 velocity 127 on patch 2 (1Dh -> FEh, 2 notes
a pass) and ch5 velocity 120 on patch 17 (9Dh -> 7Fh, 1 a pass): the accents vanish. Fix: `(b & 0xC0) | max((b & 0x3F)
- vel/4, 0)`. `--original`: the plain byte subtraction.

Latent, port literally (nothing to fix: no song reaches them): drum note-off adds the patch transpose (the on-key does
not; all drum patches the songs use have 0); `Dc` would skip 2 data bytes; running status; a pitch bend above 1023
would OR into the block bits; programs above 23.

Not bugs (design; keep): CC7 ignored; drum notes 46/49/54 not played; MIDI channels without an OPL channel skipped (the
other devices' parts); the 4th note of a chord on a 3-channel part dropped (CN1); a muted Ctrl+S tick still advances.

EXE quirks, no port effect: PIT control word B6h (section 2.3); double EOI every 4th tick; the AdLib detection ignores
its first status read.

## 9. Verification (V)

`python tools/adlib_regs.py --check-dumps` (from the game folder):
- In `mem_race_640k.bin`, `mem_map_gamemenu_640k.bin` and `mem_narbonensis_province_640k.bin` the driver at [C6]:0
  = 3682:0000 has the code of ADLIB.LZW byte for byte, and the playing (or last) song in RAM equals CN3A / CN2.
- Race dump (CN3A playing): starting the model on CN3A, after **419 ticks** its read pointer (0x530) and wait (0x3F)
  match and then **every driver variable 0x35C..0x3D7** matches the live driver: map, busy flags, notes, transposes,
  carrier bytes, A0/B0 shadows, the 9 age counters, status, channel, flags.
- Map dumps (CN2 stopped by the Game Control menu / the province screen): same at ticks 881 and 697, except 4 bytes:
  the A0/B0 shadows of channels 3 and 7, which CN2 never keys. They hold CN1's values (CN1 has exactly those between
  its ticks 994 and 2027, i.e. a key ended the intro 13.7 to 27.8 s into the title music): start() does not clear them.
  These checks confirm the event grammar, the voice allocation, the age counters and that the dumps' music state is
  the driver's own (D -> V for section 5).
- All 17 songs parse with the grammar of 5.2 exactly to their last byte (no trailing data), every mapped channel gets
  its program at tick 0.
Not verifiable without DOSBox (not run): the 72.83 Hz rate in practice, the sound. A DOSBox-X OPL capture (.dro) of
one song, with Erik's OK, would check the register stream against `adlib_regs.py` tick by tick.

## 10. Port design (for the implementer)

**Files (suggested):** `music.c` (driver port + game glue), `opl2.c` (the chip), hooks in `sdl2_platform.c` and
`main.c`. Keep the existing DS-only stubs' behaviour (map.c `music_load_5D73`, `music_play_5D29`, the static
`music_stop_5D20` / `music_unload_5E1A` copies in glad.c, naval.c, nego.c, race.c, legions.c' `LEV_MUSIC_STOP`) and
add the driver calls 072E / 074B behind them. Five copies of 5D20/5E1A exist today; a shared `music.c` version would
be the moment to merge them.

**Driver = data + literal code.** Load ADLIB.LZW from CENT.DAT (assets.c, as 5EB5(0x12) does) into
`uint8_t g_drv[0x97A]` and translate 0x3E8..0x979 function by function on that array (variables at their offsets,
`w()` helpers as in the DS code), the song pointer = the slot's buffer. No hand-copied tables: the frequency table,
patches, drum tables are the file's. This also makes `--original` exact for bug 1 (it reads `g_drv` past the table).
Fixes as in section 8 under `if (!g_original_bugs)`. List both in the file header.

**Game glue (D):** 5E86's DS part at start-up when music is wanted: `[3EEA] = 0x12`, `[3EE9] = 0xFF`, `[3EEC] =
0xFFFF`, `[C4] = 0`, `[C6]` = any nonzero (3682 as in the captures). 5D73 loads the song into a per-slot buffer
(handle = nonzero, the comparators ignore A6B0..A6BF); 5D29 -> start; 5D20 -> stop; 5E1A frees the slot (the driver
may still point at it: keep the buffer until the slot is reused, or copy the song into the driver's own buffer at
start). With music off (nosnd, no audio device, headless runs): keep today's DS ([3EEC] = 0) so the regression runs do
not change. The battle (battle.c) has no music calls yet: DF31 stop, BEC2/BECD load C2A6/C2A7, E052 fanfare + 432-tick
wait when [3EE9] == FF, E077 play C2A7, E1F4/E236 stop+unload, E4ED fanfare at the report. Accept the original word
`nosnd` ([3EEA] = FF); other device words select nothing new (only the AdLib is emulated). Ctrl+S toggles [3EE9].

**Timing.** OPL2 native rate = 14318180 / 288 = **49715.9 Hz**; PIT tick = 16383 x 12 / 14318180 s = **682.625 OPL
samples** exactly: keep an accumulator, `acc += 8` per sample, `if (acc >= 5461) { acc -= 5461; isr_tick(); }` before
the sample. `isr_tick()` = the ISR body of 2.3 from `if (B(0x3EEC))` on (the AH=3/4 edge, then AH=0). Run it inside the
SDL audio callback; the game thread's AH=1/2 calls take `SDL_LockAudioDevice` (= the driver's `cli`). Open the device
at freq 49716, AUDIO_S16SYS, 1 channel, `allowed_changes = 0` (SDL converts to the hardware rate). The square-wave
`audio_play_square` in sdl2_platform.c has no caller and can go.

**OPL2 from scratch (chip behaviour, public YM3812 facts, not from this game: G until heard).** Only what section 7
lists. Per operator: 20-bit phase, increment `(fnum << block) * M[mult] / 2` with M = {1,2,4,6,8,10,12,14,16,18,20,
20,24,24,30,30}, waveform index = phase >> 10 (1024 per cycle; fnum 0x158 block 4 = 261 Hz); phase reset at key-on.
Output via a 256-entry log-sine quarter table and a 256-entry exp table (attenuation in 1/256-octave steps, total = log
sine + envelope x 8), 13-bit signed results (max 4084, measured); waveform 1 = positive half only, 2 = absolute value.
Envelope 9-bit attenuation (0.1875 dB steps): TL x 4, SL x 16 (15 = 93 dB), KSL by block/fnum (0, 3, 1.5, 6 dB per
octave for KSL 0, 1, 2, 3: units = 4 x {0,32,40,45,48,51,53,55,56,58,59,60,61,62,63,64}[fnum >> 6] - 32 x (8 - block), at
least 0, shifted right by {-,1,2,0}: 42 / 21 / 10.5 dB at the top of block 7. Review 2026-11-17: the first port used the
manual's 3 dB / octave column as the 6 dB one, half the chip's KSL; fixed, the probe now gives -42.4 / -21.1 / -10.5 dB
there and agrees within 0.15 dB with the Nuked OPL3 / DOSBox DBOPL formula at four other notes; the songs come out
1-2 dB quieter, mostly in the high notes), tremolo 0..1 dB triangle at 3.7 Hz when AM; vibrato +-7 cents at 6.1 Hz when VIB. Rates:
effective = 4 x R + (KSR ? block*2 + fnum bit 9 : (block*2 + bit 9) >> 2), R = 0 never moves; attack exponential toward
0 (R = 15 instant), decay/release linear in dB; EG type 1 holds at SL until key-off, type 0 goes on decaying with RR.
Timing reference (YM3812 manual, 0 to 96 dB, R 1..15 at rate offset 0): decay/release 39280.64 ms for R = 1, halved per
step of R (2.40 ms at R = 15); attack 2826.24 ms for R = 1, halved per step (0 at 15). Feedback on the modulator:
phase offset = (out[t-1] + out[t-2]) >> (9 - FB) for FB > 0. Carrier phase input = its phase + the modulator output.
Mix: the 9 carrier outputs added and clipped to 16 bits.

**Headless hook `--music-wav SONG OUT SECONDS`** (no window, no SDL init; run in the game folder):
- SONG = 0..16, a song name (`cn9a`, archive from DS:3EF0 as 5EB5 does) or a path to an unpacked song file.
- Loads ADLIB.LZW from CENT.DAT, AH=1, then renders SECONDS x 49715.9 samples with the tick schedule above (AH=4 is
  not needed: [3DD] is 1 in the image).
- OUT `*.wav`: RIFF PCM, 49716 Hz, mono, 16-bit. OUT `*.txt`: the register log, `tick reg val` in hex per write,
  writes of start() at tick 0, tick t's writes labelled t, for floor(SECONDS x 1193182 / 16383) ticks: it must equal
  `python tools/adlib_regs.py SONG SECONDS` byte for byte (with `--original`; without it, the bug 1/2 lines differ
  exactly where section 8 says).
- stderr: ticks rendered; the tick where the song stopped (FC 81) or each loop.
- Checks to run: all 17 songs `.txt` vs the model (with `--original`); listen to CN9A..CN9D, CN2, CN6; CN4C at 0-20 s
  with and without `--original` (bug 1), CN8B (bug 2).
- Limits (review 2026-11-17): SECONDS must be > 0 and at most 43200 (the RIFF size words); a song file the driver cannot
  get out of (e.g. all zeros: loop start 0, every delta 0) never ends its first tick, here as in 1990: not checked for.

**What the regression tools do not see (review 2026-11-17).** cmp_endturn compares the save blocks, the dice, the war
chance and Rome's garrison only, so its "0 differ" says nothing about the music DS (A6B0..A6BF, [3EEE], [8B]). Checked
by hand: t9's slot table equals DOSBox's (A6B7 / A6BB = FF). In t9 / t10 / t11 [3EEE] is FF in the port where DOSBox has
01: `--end-turn` is headless and skips End Turn's map rebuild 0x0CDF (494A -> 5D29(1)); a harness artefact, not a port
bug. [8A] differs as before (the ISR's BIOS divider is not modelled headless).

**Known limit of a window game loaded from a DOSBox `.bin`** (a dev path; D, not run): it starts silent on the map, as
the snapshot's [3EEE] = 01 names the song DOSBox was playing and map_build's 5D29(1) then does nothing until the song
changes. A `.CSV` load or a new game plays at once ([3EEE] = FF). Left as it is (music.c header).

**Command line and keys (2026-11-17).** B0E0's words: `nosnd` turns the music off; `adlib` / `ad` (and `v`) are what
the port does anyway; `pc tn roland mt lapc`, `k`, `x m e tandy` are noted on stderr and ignored (the last sound word
wins, as in B0E0). Ctrl+S is matched by the physical key (SDL scancode S), as 03FF tests make code 1Fh.

## 11. Hidden texts and other finds

- **ROLAND.LZW 0x6E: an MT-32 display message `*ELECTRONIC ARTS*`** (SysEx data for address 20 00 00, the MT-32's LCD,
  terminated by FF like the driver's other SysEx blocks at 0x2B and 0x44), which **no code sends**: the only SysEx
  sends are `mov si, 0x44` (7F 00 00: all-parameters reset) and `mov si, 0x2B` (10 00 01..: partial reserve and MIDI
  channels). Unused text.
- Credits (DS:675E..): "Music Composition / Jon Newton, Michael Bard", "Music Programming / **Rob Hubbard**" (the
  driver author, famous for the Commodore 64).
- No text inside ADLIB.LZW, SPEAKER.LZW or TANDY.LZW ("SQRWV" and "^_ZY[" in all four drivers are the bytes of
  `push bx/cx/dx/di/si` and `pop` at the dispatcher). No text in any song (the files end with their FC event).
- Unused in the AdLib driver: the random routine 0x90B; 195 bytes of instrument data; patch 22; jump-table entries +3
  to +F. Unused in the songs: header bytes +2B..+42 and +4D..+7E; the Roland-header loop that does nothing (5.1).
- No CN5 song exists.
- Every song is played somewhere; nothing hidden.
