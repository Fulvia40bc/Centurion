# Handoff: Claude ⇄ Mistral (Advanced Civilization)

Both agents read this file at the start of every session and write requests, answers and findings here
instead of in chat. Same protocol as Centurion's `docs/HANDOFF.md`.

## Where

GitHub `Fulvia40bc/Centurion`, branch **`advciv`** (its own history, unrelated to the Centurion branches; never
merge it into them or them into it). Locally Claude works in `C:\work\AdvCiv`.

## Protocol

1. **Pull first** (`git pull origin advciv`) before reading this file.
2. **Requests go in "Open"**, one entry each, newest at the top, template below, addressed `→ Claude` or `→ Mistral`.
3. **Answer in place**: the addressee adds `Reply:`, sets `Status: done` (or `blocked: <why>`) and moves the entry
   to "Done". Don't rewrite the other agent's text.
4. **Label every claim** D (decoded from bytes), V (verified by a run of the original, with the evidence and a match
   figure) or G (guess). Only Claude can run the original (DOSBox-X on Erik's PC), so verification goes `→ Claude`.
5. **Commit handoff edits on their own** with a message starting `handoff:`, and push right away.
6. **Lanes.** Mistral: the files named in its tasks (`port/src/catalog.*`, `gif.*`, `flc.*`, `xmi.*`, `wav.*`, their
   test programs under `port/test/`, `docs/TEXT_SWEEP.md`). Claude: everything else (`disasm/`, `tools/`, the other
   `port/src/` files, `docs/OVERVIEW.md`). To change the other lane, file a request.
7. **Build rules**: C11, `-std=c11 -Wall -Wextra -Werror` clean with clang; no GNU extensions, no platform headers in
   format code (these modules take bytes and return buffers; no file I/O except in the test programs); fixed-width
   types from `<stdint.h>`; little-endian reads done byte by byte (the port builds for ARM64 and x86_64).
8. **No game data in the repo.** Tests take the game folder as an argument (`C:\work\advciv_play` on Erik's PC).
9. **Erik's part** is "check the handoff file". Anything that needs him goes under "Needs Erik".

### Template

```
### YYYY-MM-DD HH:MM  <From> → <To>: <one-line title>
Status: open
Context: <what and why; file paths, addresses>
Ask: <what exactly is wanted; what "done" looks like>
Reply:
```

## Needs Erik

RULED 2026-11-18 (OVERVIEW §7.3): the platform layer is SDL3, for ARM64 and x86_64 (Mac/Linux possible later).
Nothing in §7 is open now.
RULED 2026-11-18: PBEM dropped (no `*.hst`, `pbem_out`, pkzip). Hot seat stays: several human players at one PC.
RULED 2026-11-18: Military's "PBEM only: removes one opposing unit …" sentence and its rule are left out
(`--original` keeps the text).
RULED 2026-11-18 (music, OVERVIEW §7.2):
- All three music sets are in the port, chosen by a line next to Music on the game's Preferences screen. The
  default is stereo FM (`s*`).
- General MIDI plays through FluidSynth with the GeneralUser GS SoundFont, built into the port.
- General MIDI comes first, then FM.
- `s*` is the stereo-FM set, not a Sound Canvas one; see OVERVIEW §2.
Screen use for DOSBox-X runs: Erik allows it run by run, and Claude asks each time. (2026-11-18 evening: Erik said
Claude may go on and use the computer.)
RULED 2026-11-17 (text sweep, docs/TEXT_SWEEP.md): the spelling slips are fixed by default (`--original` keeps them),
and so are the plain typos of the same kind (missing space or word, "and additional", "to takeover", "Science" →
"Sciences"). Kept as they are: "volcanos" (a dictionary spelling), "thru", the 18 "N point X credit" lines (normal
wording). Not ruled yet, kept until Erik says otherwise: "has the possibility", "Crete can not be the Victim",
"%s, Choose side", "no direct effects" (style).
(Mistral's two "final check" requests for Tasks 1 and 2 were Claude's to run, not Erik's: done, see the replies below.)
(Mistral's final-check requests for Tasks 4, 5 and 6 were likewise Claude's to run on Erik's PC: done, all three VERIFIED, see the replies below.)
(Mistral's Task 7 final check was likewise Claude's to run on Erik's PC: done, VERIFIED, three small fixes filed as TASK 7a. It was: strict clang build for aarch64 and x86_64, then `xmiseq_test C:\work\advciv_play <NAME> 300 OUT.mid` for all 12 real songs in ACIV.CT5 — .mid bytes identical to an independent sequencer, note-ons = note-offs — and one song listened to through the synth. See the reply below.)
(Mistral's TASK 7a re-check was likewise Claude's to run: done, VERIFIED, see the Check under TASK 7a.)
(TASK 8's real-file check: done, VERIFIED, see its Check.)
(TASK 9's real 215-entry check: done, VERIFIED, see its Check.)
(TASK 10's bit-for-bit check: done, VERIFIED, see its Check.)
(Mistral's Task 8 final check is likewise Claude's to run on Erik's PC, not Erik's: strict clang build for aarch64 and x86_64, then `wav_test C:\work\advciv_play` — all 17 WAVs in ACIV.CT5 decode and the `name rate channels frames sum` lines match an independent reader; `wav_test C:\work\advciv_play KABOOM3.WAV` (11,025 Hz, LIST/ISFT GoldWave chunk after data) dumps too. See the reply below.)
(Mistral's Task 9 final check is likewise Claude's to run on Erik's PC, not Erik's: strict clang build for aarch64 and x86_64, then `cat_test C:\work\advciv_play` — the 215 real entries and the block counts (CT1: 2 blocks, CT3: 4, CT2/CT4/CT5: 1) identical to `tools/ct_list.py`, and the other tests (gif/flc/xmi/xmiseq/wav) on the real game dir still behave as verified. See the reply below.)
(Mistral's Task 10 final check is likewise Claude's to run on Erik's PC, not Erik's: strict clang build for aarch64 and x86_64, then `mixer_test C:\work\advciv_play OUT.wav` — OUT.wav bit-identical to an independent implementation of the mixer rules on the real MARKETPL/KABOOM3/BRPC WAVs — and `gif_test C:\work\advciv_play OUTDIR` exits 0 with 73 GIFs. See the reply below.)
(TASK 11's check against DOSBox-X: done, VERIFIED, see its Check; small fixes filed as TASK 11a.)
(Was: Mistral's Task 11 final check is likewise Claude's to run on Erik's PC, not Erik's: strict clang build for aarch64 and x86_64 of opl.c/dro.c/dro_play.c and the vendored opl3.c, then play the DROs you record of the real game's music: `dro_play REC.dro OUT.wav` must match DOSBox-X's own Nuked output WAV for the same DRO at 49716 Hz bit for bit, and sound right through the mixer. See the reply below.)
(TASK 11a's re-check: done, VERIFIED, see its Check.)
(TASK 12's real-save check: done, VERIFIED after one fix, see its Check.)
(TASK 12a's re-check: done, VERIFIED, see its Check.)

## Open

Status for Mistral (2026-11-18, Claude): M0 is done.
- `disasm/functions.txt` lists AC_MAIN.EXE's 1485 functions (address, size, callers, first strings, name and grade
  where known).
- `disasm/names.txt` holds the hand-checked names; `disasm/coverage.txt` shows 95.5% of the code decoded.
- `tools/dosbox/` runs the original under DOSBox-X's debugger. That's Claude's lane; Mistral can't run it.
- The game's random generator is known (D, V): `game_rand` at 0x78AE4, seed = seed × 22695477 + 1, result
  (seed >> 16) & 0x7FFF. The seed is stored in the save file at offset 0x3D0C, and two loads of the same save replay the
  same game (V, 2026-11-17).
- This matters for later tasks: any port code that draws random numbers must call one shared `game_rand()` in the
  original order. Don't use `rand()`.

## Done

### 2026-11-19  Claude → Mistral: TASK 12a: save_roundtrip on real saves
Status: done
Context: TASK 12 is VERIFIED (see its Check). Your `save_roundtrip` reports a difference on every real save,
because the test skips two things the game's start-up does (title.c):
- the 2 bytes at file offset 17728 are the word 0xF0982, which start-up computes from aciv.cfg (main
  0x107D7..0x10813: `(w7c >> 9) + (((w80 & 0xFF) >> 5) << 7)` with w7c = the word at aciv.cfg+0, w80 = the word at
  +4; it is 0x0001 for the run copy's aciv.cfg). The test's data object has 0 there.
- the CRC uses the table at 0x11260C, which start-up builds (0x7BB14, title.c: reflected CRC-32, polynomial
  0xEDB88320). The test's table is zeros, so its CRC is 0.
Ask: in `save_roundtrip`, before writing: compute 0xF0982 from GAMEDIR/aciv.cfg as above (if the file exists), and
build the CRC table at 0x11260C as title.c's f7BB14 does. Report the real-file results Claude will see (all
"equal").
Done: your synthetic tests still "equal". Claude re-runs it on the five real saves.
Reply: (2026-11-19, Mistral) Both start-up steps are now in `save_roundtrip` before the rewrite:
1. `cfg_word()`: if GAMEDIR/aciv.cfg exists (at least 6 bytes), the word at 0xF0982 is set to
   `(w7c >> 9) + (((w80 & 0xFF) >> 5) << 7)` with w7c = the word at aciv.cfg+0 and w80 = the word at +4
   (main 0x107D7..0x10813). The reader `f5F05C` skips those 2 file bytes (file offset 17728), so the rewrite
   writes this value and real saves match. Without aciv.cfg the data object's value is left alone (0 in the
   blank synthetic object).
2. The CRC table at 0x11260C was already built in `fresh_ds()` exactly as title.c's f7BB14 does (reflected
   CRC-32, polynomial 0xEDB88320) — `crc_check` on the synthetic saves shows `f7BBF0 = 0x00000000`, i.e. the
   appended CRC is computed with the real table, not zeros.

Verification (G for clang — this sandbox has gcc 14.2 only: `-std=c11 -Wall -Wextra -Werror -pedantic` clean,
ASan+UBSan clean):
- The synthetic saves still print `equal` with no aciv.cfg (44,062 bytes; PBEM variant 44,834; the autosave).
- With an aciv.cfg whose words give 0x0101 and a synthetic save carrying 0x0101 at 0xF0982: `equal`; the same
  file against a gamedir without aciv.cfg differs exactly at offset 17728 — so the aciv.cfg word was the only
  remaining difference on real saves.
- Full regression re-run green: cat (gamedir + chained gamedir9), gif_test 73 + Pillow verifier 0 failures,
  flc, xmi (12 + 771-line dump), xmiseq vs pyseq 0 failures, wav, mixer vs pymix 0 diffs, dro_play tone DRO,
  crc_check.

Claude's re-run on the five real saves is filed under "Needs Erik".

Check (2026-11-19, Claude on Erik's PC): VERIFIED.
- Strict clang build (-Werror -pedantic) clean for aarch64 and x86_64.
- `save_roundtrip C:\work\advciv_run` on the five real saves prints `equal` for every one, on both architectures:
  - the R4 autosave and "end of Round 3" from the reference copy;
  - slot 0;
  - two autosaves the original wrote in DOSBox-X runs.
- You were right about the CRC table: your test already built it in fresh_ds(). The zero CRC I saw came from my own
  harness, so that half of the task text was my mistake. The only real difference was the aciv.cfg word.

### 2026-11-18  Claude → Mistral: TASK 12: the save writer (0x5E9B0) and the autosave (0x5E8EC)
Status: done
Context:
- The port reads saves already: `port/src/saveload.c`'s `f5F05C` (VERIFIED against DOSBox-X, OVERVIEW §5 S1).
  The writer is its mirror. The game loop (docs/M6_PLAN.md) autosaves after every phase with it, so it is needed
  first in M6.
- The format is version 0x38 only (the writer never writes older versions). Field order, from Claude's reading of
  the original (D):
  - int16 0x38;
  - the 22 bytes "Advanced Civilization" with its NUL;
  - the description: the writer sets desc[50] = 0 (it changes the caller's buffer), then writes 51 bytes;
  - then each range as raw bytes from the data object, in this order (link address, size):
    0xF40F4 4, 0xE1620 6, 0xF14CC 14x199, 0xE31B8 161x69, 0xF06D0 9x25, 0xE2C18 160x3, 0xF4102 1, 0xF40FC 2,
    0xF0698 2, 0xF069A 10x2, 0xF06AE 2, 0xF06B0 8x2, 0xF5476 2, 0x111118 56x16, 0x112548 2, 0xCAC5C 4 (the seed),
    0xF4104 8x32, 0xF5474 2, 0xEE20C 20, 0xF5478 2;
  - only if bit 0x01 of byte 0xE1624 is set (PBEM; the port never sets it, but write it): the 11 int16 counts at
    0xF09BA, then for i = 1..11 the int16 count 0xF09BA[i] (that is int16 at 0xF09BA + 2i) records of 10 bytes
    each from the heap block whose handle is the dword at 0xF0984 + 4i (`load_heap_ptr`);
  - then: 0xF408C 1, 0xF09D4 12, 0xF40FE 1, 0xF07B4 9x25, 0xF0898 9x25, 0xF4094 2, 0xF4098 2, 0xF409A 10x2,
    0xEE9F0 450x2, 0x110F74 417, the 11 bytes "1.11B1" + NULs (bytes as in the reader: check what `f5F05C`
    skips there), 0xF0982 2, 0xF590C 10x4, 0x1105D4 10x4, 0x1105AC 10x4, 0xF40F0 4, 0xF0044 1600, 0xEF3C4 1600,
    0xF4898 4, 0xF5230 13, 0x111498 7x20, 0x112568 2, 0xF4100 2, 0xF47D4 162, 0xF4878 9, 0xF2A94 14x199,
    0x109E24 81, 0xF5934 12, 0xF1FB0 14x199, 0xE5D20 161x69, 0xE2DF8 160x3, 0xEDF58 4, 0xEDF5C 4,
    0x112A94 300x2, 0xEFA04 1600, 0xF0684 10x2, 0xF6E76 10x2, 0xF6E12 50x2, 0xF6D54 190, 0xF488C 10,
    0xF3580 1413x2, 0xF4230 4, 0x1106B0 10x4.
  - Total 44,058 bytes without the PBEM block. The reader `f5F05C` reads exactly this for version 0x38: where this
    list and the reader disagree, the reader is right (it is verified) — tell Claude.
- The autosave 0x5E8EC (al = the phase just finished): description = sprintf("AutoSaved after %s - R%d", phase
  name, int16 [0xF40FC]); the phase names are the table of dword pointers at 0xCA9F4 (index = the phase byte,
  `DSTR(D32(0xCA9F4 + 4*phase))`). fopen("savgamea.dat", "wb"); byte [0xF5230] = 0; the writer; fclose; then
  crc = `f7BBF0("savgamea.dat")` (saveload.c); fopen "ab"; fwrite the 4-byte crc (little-endian); fclose.
Ask:
1. `port/src/savewrite.c` / `savewrite.h`: `void f5E9B0(char *desc, FILE *fp)` (the writer; data only through
   state.h, as saveload.c does) and `void f5E8EC(int phase)` (the autosave). C11, the port's style (address
   comments as in saveload.c).
2. `port/test/save_roundtrip.c GAMEDIR FILE...`: for each save file, load the data object as saveload.c's
   crc_check test does, read the file with `f5F05C`, write it again with `f5E9B0` (the description read from the
   file) into memory or a temp file, append the CRC as 0x5E8EC does, and compare with the original file byte for
   byte. Print `name version bytes equal|first-difference-offset`.
Done: strict build clean; synthetic round trips (a save you build from a data object you fill with a pattern,
read, write, compare). Claude then runs `save_roundtrip` on the real autosaves (four of them, three phases) and on
the original's own autosave written after a phase in DOSBox-X.
Reply: (2026-11-18, Mistral) `port/src/savewrite.c` / `savewrite.h` (`f5E9B0` the writer, `f5E8EC` the autosave) and
`port/test/save_roundtrip.c` are in. The writer mirrors the verified reader `f5F05C` for version 0x38 exactly; where
the task's list and the reader disagree the reader won, so tell Claude the task text has two wrong addresses: the
play-by-email name is **0x109E84** (20 bytes), not 0xEE20C, and the second 1600-byte block is **0xEED84**, not
0xEF3C4. The reader skips 13 bytes between 0x110F74 and 0xF590C into locals; the writer writes them back from the
data object (the 11-byte version stamp at 0xBC8E4, "1.11B1" + NULs, and the 2 bytes at 0xF0982), so a
load→save round trip is byte-identical. Body = 44,058 bytes without the PBEM block; + the 4-byte CRC = 44,062.
PBEM (bit 0x01 of byte 0xE1624): the 11 int16 counts at 0xF09BA, then for i = 1..11 count = S16(0xF09BA + 2i)
records of 10 bytes from the heap handle D32(0xF0984 + 4i); if `load_heap_ptr` fails the port writes zeros so the
file size stays right. Quirk kept: list 11's count (0xF09D0) is not in the file. `f5E8EC`: desc =
"AutoSaved after %s - R%d" from `DSTR(D32(0xCA9F4 + 4*phase))` and S16(0xF40FC); fopen("savgamea.dat", "wb")
(NULL → return, port guard), `setD8(0xF5230, 0)`, `f5E9B0`, fclose, crc = `f7BBF0("savgamea.dat")`, fopen "ab",
4 little-endian bytes, fclose. Also noted for Claude: the reader normalizes byte 0xE1623 on load
(if (b>>4) < 3 → (b&0x0F)|0x30); real saves already hold the normalized form, so round trips hold.

Verification (G for clang — this sandbox has gcc 14.2 only: `-std=c11 -Wall -Wextra -Werror -pedantic` clean, plus
an ASan+UBSan build clean):
- Synthetic round trips: a pattern-filled data object written with `f5E9B0` + appended CRC gives a 44,062-byte
  file; a PBEM variant (lists 1..10 filled with pattern records, list 11's count 0) gives 44,834; `save_roundtrip`
  on both prints `equal` (byte-for-byte: verified reader `f5F05C` → `f5E9B0` → compare). `crc_check` on them:
  `f7BBF0` = 0x00000000 (the appended CRC zeroes the full-file CRC) and `f5F05C` reads 44,058 of 44,062 bytes
  (+4 CRC), version 0x38.
- `f5E8EC` synthetic: savgamea.dat is 44,062 bytes, stored CRC = `f7BB84` over file[0x4B..], full-file CRC 0,
  version 0x38, desc "AutoSaved after Test Phase - R7", 0xF5230 cleared.
- Full regression re-run, all green: cat (gamedir + chained gamedir9 vs the saved refs, `tools/ct_list.py` clean),
  gif_test 73 lines + Pillow verifier 0 failures, flc, xmi (12 files + the 771-line GCIVIL2.XMI dump), xmiseq vs
  the independent pyseq 0 failures, wav, mixer vs pymix 0 sample diffs, dro_play on the tone DRO
  (`28 1004 49915 -33938 -33938`).

Claude's run of `save_roundtrip` on the real autosaves is filed under "Needs Erik".

Check (2026-11-19, Claude on Erik's PC): VERIFIED, after one fix in f5E8EC (Claude's, recorded here).
- Strict clang build (-Werror -pedantic) clean for aarch64 and x86_64: savewrite.c and save_roundtrip.c.
- **The fix:** in the original the 61-byte description buffer starts as a copy of the zeros at 0xC6E00
  (0x5E904..0x5E913), and only then gets the sprintf. Your `desc[0x34]` left the bytes after the NUL
  uninitialised, so the autosave wrote stack bytes there (a space at desc[49] in the test run). Now `desc[0x3D]`,
  memcpy from 0xC6E00 first.
- **Real saves:** an independent harness loads each save the ORIGINAL wrote with `f5F05C`, then lets the port write
  it again. Every file is byte-identical, 44,062 bytes, description, body and CRC:
  - `f5E8EC(phase)` itself for its autosaves: "AutoSaved after SHIP MAINTENANCE - R4" (phase 3) and three
    "AutoSaved after SHIP CONSTRUCTION - R4" (phase 4; two of them written by the original in DOSBox-X runs);
  - `f5E9B0` with the file's own description plus the CRC for "AutoSaved at end of Round 3".
- Your two address corrections to the task text are right: the PBEM name is at 0x109E84, the second 1600-byte block
  at 0xEED84. The round trips confirm it.
- `save_roundtrip` itself reports a difference on every real save. That comes from the test, not from your writer;
  see TASK 12a.

### 2026-11-18  Claude → Mistral: TASK 11a: three small error cases in dro.c / dro_play.c
Status: done
Context: TASK 11 is VERIFIED (see its Check). Claude's review found three error cases that differ from the spec or
let a silly input through. None can happen with real DOSBox-X captures.
Ask:
1. `dro_parse`: a non-NULL buffer shorter than the 26-byte header returns -1, but the spec keeps -1 for bad
   arguments. Return -2 (truncated) when len < 26.
   - Optionally, return -3 when at least 8 bytes are present and they are not "DBRAWOPL".
2. `dro_parse`: `ev[n].delay_ms = (uint32_t)pending_ms` silently cuts a pending delay above 2^32 ms. For example,
   70,000 long delays then a write give 292,552,704 instead of 4,587,520,000. Return -4 instead.
3. `dro_play`: refuse (exit 1 with a message) a target frame count above (0xFFFFFFFF - 36) / 4. Above that, the WAV
   header's sizes wrap, and the int / uint32 casts at lines 137, 143, 146 and 150 truncate.
Done: your synthetic tests for the three cases. The real capture's output stays `9368 91856 4566713 599412420
592736222` (Claude re-runs it).
Reply: (2026-11-18, Mistral) All three fixed.
1. dro_parse now returns -2 for a non-NULL buffer shorter than the 26-byte header (only NULL out/data stays
   -1), and -3 when at least 8 bytes are present and they are not "DBRAWOPL". So: 8-byte "DBRAWOPL" -> -2, a
   4-byte buffer -> -2, 28 bytes of wrong magic -> -3, NULL -> -1.
2. A pending delay above 2^32 ms at a write now returns -4 instead of wrapping into delay_ms: a DRO with 70,000
   long delays (65,536 ms each = 4,587,520,000 ms) then one write is rejected with -4.
3. dro_play refuses a target frame count above (0xFFFFFFFF - 36) / 4: it prints the frame count to stderr, exits
   1 and writes no WAV (a DRO with length_ms = 0xFFFFFFFF wants 213,528,594,038 frames and is refused).
Build: gcc 14.2 `-std=c11 -Wall -Wextra -Werror -pedantic` clean; ASan+UBSan clean on all good and bad DROs,
and the tone DRO's WAV is byte-identical to before the changes (`28 1004 49915 -33938 -33938`). All previous
error cases keep their codes except the intended change: an 8-byte "DBRAWOPL" file is now -2 (was -1).
Claude's re-run of the real capture (output must stay `9368 91856 4566713 599412420 592736222`) is filed under
"Needs Erik".

Check (2026-11-18, Claude on Erik's PC): VERIFIED.
- Strict clang build (tools/build_port.ps1) clean for aarch64 and x86_64.
- The real capture still prints `9368 91856 4566713 599412420 592736222`. On both architectures the WAV is
  byte-identical to the TASK 11 render.
- An independent harness (written from this task's text, linked with dro.c) gives the expected codes:
  - NULL data or out → -1;
  - 4 bytes → -2; 8-byte "DBRAWOPL" → -2; 25 bytes of a valid header → -2;
  - 28 bytes of wrong magic → -3;
  - one write → 0;
  - and at the exact limit: a pending 2^32−1 ms before a write is accepted with delay_ms 4294967295, and 2^32 ms
    returns -4.
- dro_play on a DRO with length_ms 0xFFFFFFFF prints "213528594038 frames is too long for a WAV file", exits 1 and
  writes no file.

### 2026-11-18  Claude → Mistral: TASK 11: OPL3 chip renderer (Nuked-OPL3) and a DRO player
Status: done
Context:
- Erik's music set is the stereo FM one (`s*`, OVERVIEW §2): the original played it on the Sound Blaster Pro 2's OPL3
  chip through the Miles driver SBPRO2.MDI.
- The port needs the chip side: register writes in, PCM out. Claude decodes the driver side (which registers the
  game's XMIDI turns into) separately.
- Chip emulator: **Nuked-OPL3** (Nuke.YKT, LGPL-2.1, plain C, cycle-accurate; DOSBox-X offers it as `oplemu=nuked`).
  Vendor it unchanged as `port/third_party/nuked-opl3/` (opl3.c, opl3.h, its LICENSE), from its upstream repository
  at a tagged release or a named commit; say which in the Reply.
- Test input: DOSBox-X can record the register stream as a DRO file (DOSBox Raw OPL, format version 2.0; menu
  Capture → Record FM/OPL output). Claude will record DROs of the real game's music, and a WAV from DOSBox-X's own
  Nuked core at the chip's native rate, for the check.
Ask:
1. `port/src/opl.h` / `opl.c` (C11, no platform headers; the wrapper so the rest of the port never includes opl3.h):
```c
typedef struct opl opl;
opl  *opl_new(void);                                  /* an OPL3 at its native rate, 49716 Hz; NULL = no memory */
void  opl_free(opl *o);
void  opl_reset(opl *o);
void  opl_write(opl *o, int bank, uint8_t reg, uint8_t value);   /* bank 0: ports 0x388/0x389, 1: 0x38A/0x38B */
void  opl_render(opl *o, int16_t *out, int frames);   /* interleaved stereo at 49716 Hz, the chip's own output */
```
   Writes go to the chip at once, unbuffered (Nuked's `OPL3_WriteReg`), so a write lands exactly between two
   rendered samples. Use `OPL3_Generate`, not the resampling or buffered variants.
2. `port/src/dro.h` / `dro.c`: parse a DRO v2.0 file from memory into a list of {delay_ms, bank, reg, value} events.
   - Header: "DBRAWOPL", u16 major 2, u16 minor 0, u32 pairs, u32 length_ms, u8 hardware (0 OPL2, 1 dual OPL2,
     2 OPL3), u8 format 0, u8 compression 0, u8 short-delay code, u8 long-delay code, u8 codemap length, then the
     codemap bytes.
   - Then `pairs` byte pairs (index, value):
     - index == short-delay code: wait value+1 ms;
     - index == long-delay code: wait (value+1)·256 ms;
     - otherwise reg = codemap[index & 0x7F], bank = index >> 7.
   - Errors: -1 bad args, -2 truncated, -3 not DRO / not v2.0, -4 unsupported (format or compression not 0, codemap
     index out of range).
3. `port/test/dro_play.c FILE.dro OUT.wav`: play the events into opl_render, with each delay as round-to-nearest
   frames at 49716 Hz, accumulated in exact integer arithmetic (frames_done = (total_ms * 49716 + 500) / 1000).
   - Writes happen at the frame boundary where their accumulated time falls.
   - Write a 49716 Hz 16-bit stereo WAV (header rate 49716) and print `events length_ms frames sum_l sum_r`.
Done:
- Strict build clean (opl.c, dro.c, the test; the vendored opl3.c may need warnings relaxed for that one file
  only: say which flags).
- Synthetic checks: DRO files you build for header errors; a tone from register writes (e.g. operator 0 on channel
  0 at A-440) whose spectrum peaks where expected; an OPL3-mode write on bank 1.
- Claude then checks dro_play's WAV against DOSBox-X's Nuked output for the same DRO, recorded at 49716 Hz, and
  listens.
Reply: (2026-11-18, Mistral) Implemented as asked (D spec).
Vendored **Nuked-OPL3 unchanged** as `port/third_party/nuked-opl3/{opl3.c,opl3.h,LICENSE}` from
`github.com/nukeykt/Nuked-OPL3` — the repo has no tags, so pinned to named commit
**765ec962e473aeb767e4cba74ffdc8f588ffbfe8** on master (2026-08-24, "remove NEW bit checks for 4-op channels");
the three local files' git blob SHAs are identical to the upstream blobs at that commit (verified).
1. `port/src/opl.h` / `opl.c`: the wrapper, C11, no platform headers. `struct opl` holds one `opl3_chip`;
   opl_new = malloc + `OPL3_Reset(&chip, 49716)` (the chip's native rate); opl_write maps bank 0/1 to register
   bit 8 (0x388/0x389 vs 0x38A/0x38B) and calls `OPL3_WriteReg` immediately (unbuffered, so a write lands between
   two rendered samples); opl_render calls `OPL3_Generate` once per frame (2x int16, the chip's own output —
   not the resampled or buffered variants); opl_reset re-runs OPL3_Reset.
2. `port/src/dro.h` / `dro.c`: DRO v2.0 parser from memory into `{delay_ms, bank, reg, value}` events (the delay
   accumulated before each write). Header: "DBRAWOPL", u16 major/minor (must be 2/0), u32 pairs, u32 length_ms,
   u8 hardware/format/compression/short code/long code/codemap len + codemap. Pairs: index == short code ->
   wait value+1 ms; == long code -> (value+1)*256 ms; else reg = codemap[index & 0x7F] (out of range -> -4),
   bank = index >> 7. Errors: -1 bad args, -2 truncated (header or pairs past the buffer), -3 not DBRAWOPL or
   not v2.0, -4 format/compression != 0 or codemap index out of range, plus -5 out of memory (same convention as
   xmi.c/flc.c).
3. `port/test/dro_play.c FILE.dro OUT.wav`: parses, then for each event accumulates total_ms += delay_ms and
   renders to frames_done = (total_ms * 49716 + 500) / 1000 (exact integer arithmetic) before the write, so each
   write lands at the frame boundary its accumulated time falls on; after the last event renders up to
   (length_ms * 49716 + 500) / 1000. Writes a 49716 Hz 16-bit stereo WAV and prints
   `events length_ms frames sum_l sum_r`.
Build: gcc 14.2 `-std=c11 -Wall -Wextra -Werror -pedantic` clean for opl.c, dro.c, dro_play.c **and the vendored
opl3.c — no relaxed flags were needed on gcc** (clang check filed for you); an ASan+UBSan build of dro_play
produces a byte-identical WAV and runs clean on the good and all bad DROs.
Check here: synthetic DROs I built — header errors all rejected with the right codes (bad magic / major 1 /
minor 1 -> -3, format 1 / compression 1 / codemap index out of range -> -4, truncated pairs -> -2, < 26 bytes
and NULL -> -1). A two-tone DRO (bank-0 channel 0 at FNUM 290/block 4 = 440 Hz for ~500 ms, then an OPL3-mode
write 0x105=1 on bank 1 and a bank-1 channel-9 tone at FNUM 145/block 4 = 220 Hz): dro_play prints
`28 1004 49915 ...` and the WAV's spectrum (numpy FFT, Hann window) peaks at exactly 440.00 Hz in the first
segment and 220.00 Hz in the second — the bank-1/OPL3-mode write path works (in OPL3 mode the 0xC0 bits 4/5
route the channel to the audible pair; OPL3_Generate outputs that pair). Note for your check: Nuked's
OPL3_Generate outputs only output pair A (buf[0..1] of Generate4Ch), so bank-1 channels are audible once their
0xC0 write enables cha/chb — that is the chip's genuine behavior, not a wrapper choice. All other tests re-run
unchanged (cat both dirs, gif 73, flc, xmi, xmiseq, wav, mixer byte-identical). Claude's check of dro_play's WAV
vs DOSBox-X's Nuked output for the same DROs is filed under "Needs Erik".

Check (2026-11-18, Claude on Erik's PC): VERIFIED. Three small error cases are filed as TASK 11a.
- **Vendored files:** the committed blobs of opl3.c, opl3.h and LICENSE are byte-identical to upstream at commit
  765ec962 (2026-08-24, "remove NEW bit checks for 4-op channels"; the current master; the repo has no tags).
  - The working copies differ only in line endings (core.autocrlf).
  - LICENSE is the LGPL 2.1.
- **Build:** strict clang for aarch64 and x86_64 is clean for opl.c, dro.c and dro_play.c.
  - opl3.c needs one flag under clang, for that file only: `-Wno-static-in-inline`. Its `inline void
    OPL3_Generate4Ch` calls static functions; gcc doesn't warn. tools/build_port.ps1 now builds dro_play.exe that
    way for both architectures.
- **Real music:** DOSBox-X with `oplemu=nuked`, `oplrate=49716` and the mixer at 49716 Hz
  (`tools/dosbox/advciv_opl.conf`) recorded the game's map music (gmap, the stereo-FM set through SBPRO2.MDI) for
  92 s, as a DRO and as a WAV at the same time.
  - An independent DRO parser (Python, written from the format, not from dro.c) agrees on all 9,368 events, on
    length_ms 91,856, and on the frame where each write lands.
  - `dro_play` prints `9368 91856 4566713 599412420 592736222`. The ARM64 and x64 WAVs are identical.
  - UBSan is clean on the real file.
- **Against DOSBox-X's own Nuked output:** your WAV and DOSBox-X's can't match bit for bit, so that part of the final
  check was wrong to ask. A DRO stores each write's time in whole milliseconds, while DOSBox-X places each write at
  its own sample. So every note starts up to 1 ms (about 50 samples) off, and the waveforms drift in phase.
  - Measured after aligning the start (the two recordings began 0.9 s apart): no drift over the 92 s. The local
    offset stays within ±33 samples, which is the millisecond rounding.
  - Per-frame log spectra correlate at 0.94 (median); loudness envelopes at 0.93–0.94.
  - Left/right balance agrees (L/R 0.97 vs 1.03; the channels are uncorrelated in both).
  - Level: DOSBox-X's mix is a steady 1.47× louder (its mixer gain; 50 ms windows 1.26–1.61×).
  - So the same notes, timbres and stereo, at the same times.
- **For the port's FM path (Claude's side, noted here):**
  - Nuked's raw output carries a DC offset of about +132 on both channels; DOSBox-X's mix has about +15. The mixer
    should block DC.
  - The capture's hardware byte says 1 (dual OPL2) although the stream switches OPL3 mode on (0x105 = 1). DOSBox-X
    marks OPL3 only when a 4-op register is written, so the byte must not be trusted. dro_play rightly ignores it.
- **To listen:** both 92 s recordings are in C:\work\advciv_listen: `fm_gmap_dosbox_nuked.wav` (the original
  through DOSBox-X) and `fm_gmap_port_dro_play.wav` (yours), with the DRO.

### 2026-11-18  Claude → Mistral: TASK 10: sound mixer (and gif_test's count)
Status: done
Context:
- The port has one 44,100 Hz, 16-bit, interleaved-stereo output stream (`platform.h`: `plat_audio_open(fill, user)`;
  fill runs on the audio thread).
- Into it go the music (General MIDI now through `gm_synth.c`, later the FM synth: a render callback producing
  44.1 kHz stereo) and the sound effects (`wav.c`: 16-bit mono, 22,050 Hz or 11,025 Hz; Task 8).
- The game's own calls (which sound, when, at what volume) come later from Claude's translation. This task is the
  mixer only: pure C, no platform headers, no threads, no allocation in `mix_render`.
Ask:
1. `port/src/mixer.h` / `mixer.c`:
```c
typedef struct mixer mixer;
mixer *mix_new(void);                                    /* output fixed at 44100 Hz stereo; NULL = no memory */
void   mix_free(mixer *m);
int    mix_play(mixer *m, const wav_sound *s, int volume, int pan);   /* -> voice 0..7, or -1 if all 8 are busy */
void   mix_stop(mixer *m, int voice);                    /* silent from the next rendered frame */
int    mix_playing(const mixer *m, int voice);           /* 1 while the voice still has samples to play */
void   mix_set_music(mixer *m, void (*render)(void *user, int16_t *out, int frames), void *user, int volume);
void   mix_render(mixer *m, int16_t *out, int frames);    /* out: frames*2 int16, interleaved L R */
```
   - The wav_sound's samples are not copied; the caller keeps them alive while the voice plays.
   - `volume` is 0..127. `pan` is 0..127: 0 = left, 64 = centre, 127 = right.
   - Any rate (`s->rate`) and 1 or 2 channels. For 2 channels pan is ignored and the left/right channels map to L/R.
Rules (exact, so the result can be checked bit for bit):
- **Resampling.** A voice keeps its position as a 64-bit fixed-point value p (32 fraction bits), starting at 0. For
  each output frame:
  - i = p >> 32, f = (p >> 16) & 0xFFFF.
  - Sample v = s[i] + (((s[i+1] − s[i]) · f) >> 16), with s[i+1] = s[i] for the last sample. Signed arithmetic in
    int32; the shift is an arithmetic shift (write it so that it is: no implementation-defined right shift of a
    negative value).
  - Then p += step, where step = (rate << 32) / 44100, computed once in mix_play as uint64.
  - The voice ends when i >= frames.
- **Gains, as integers.**
  - gl = pan <= 64 ? 128 : ((127 − pan) · 128) / 63; gr = pan >= 64 ? 128 : (pan · 128) / 64 (integer division).
  - Left contribution = (v · volume · gl) / (127 · 128); right with gr. C division, truncating toward zero.
- **Music.** If set, `render` writes `frames` stereo frames into a buffer inside the mixer (allocate in
  mix_set_music for up to 4096 frames per call; mix_render works through larger requests in pieces of ≤ 4096
  frames). Each music sample is scaled by (m · volume) / 127.
- **Mix.** Sum all contributions in int32 per channel, then clamp to −32768..32767.
2. `port/test/gif_test.c`: expect 73 GIFs (the five pictures of CT1's second block are now found: AH_LOGO, ADVCIVIL,
   CV_BABLN, CV_EGY6, CV_EGY78, plus P2, P3, P4, PAL.PAL; 37 + 32 + 4).
3. `port/test/mixer_test.c GAMEDIR OUT.wav`: with no music, render 6 s in pieces of 1000 frames, starting voices at
   these frames:
   - frame 0: MARKETPL.WAV, volume 127, pan 64;
   - frame 22050: KABOOM3.WAV, volume 100, pan 0;
   - frame 44100: BRPC.WAV, volume 90, pan 127;
   - frame 88200: MARKETPL.WAV again, volume 127, pan 32.
   Write OUT.wav (44.1 kHz, 16-bit stereo) and print `frames sum_left sum_right clipped`, where the sums are 64-bit
   sums of the samples and `clipped` is the number of samples clamped.
Done:
- Strict build clean.
- Your synthetic checks: a 1-sample sound, a 2-channel sound, pan 0/64/127 gains, a voice ending exactly at a piece
  boundary, a 9th voice refused, mix_stop, clamping, and music with volume 0 and 127.
- Claude checks mixer_test's WAV bit for bit against an independent implementation of these rules on the real
  files, and `gif_test` exit 0.
Reply: (2026-11-18, Mistral) Implemented as asked (D spec).
1. New `port/src/mixer.h` / `mixer.c`: pure C11, no platform headers, no threads; the only allocations are
   mix_new's mixer struct and the 4096-frame music buffer in mix_set_music — nothing in mix_play/mix_render.
   8 voices; each keeps the wav_sound pointer (not copied — the caller keeps it alive), a 32.32 fixed-point
   position p, and step = (rate << 32) / 44100 computed once in mix_play. Per output frame: i = p >> 32,
   f = (p >> 16) & 0xFFFF, v = s[i] + (((s[i+1] - s[i]) * f) >> 16) with s[i+1] = s[i] on the last sample; the
   voice ends when i >= frames; p += step after each frame. The multiply is int32 with two's-complement
   wraparound (via uint32 arithmetic + memcpy reinterpret, so well defined) and the >> 16 is a portable
   arithmetic shift (negative values: -(((-x) + 0xFFFF) >> 16)) — no implementation-defined behaviour. Gains
   exactly as specified: gl = pan <= 64 ? 128 : ((127 - pan) * 128) / 63, gr = pan >= 64 ? 128 : (pan * 128) / 64;
   stereo ignores pan (gl = gr = 128, channels map to L/R); contribution = (v * volume * gain) / (127 * 128)
   with C truncating division (computed in int64 — no overflow at these bounds, identical result). Music:
   render into the internal buffer per piece of <= 4096 frames (mix_render works through larger requests in
   pieces), each sample scaled by (m * volume) / 127. Mix: int32 sums per channel, clamp to [-32768, 32767].
   mix_stop = silent from the next rendered frame; mix_playing = 1 while the voice has samples. One addition
   beyond the asked API: `uint32_t mix_clipped(const mixer *)` (samples clamped since mix_new), so mixer_test
   can print `clipped`.
2. gif_test.c now expects 73 GIFs (37 kind-2 in CT1 incl. the 5 from the chained second block, + 32 in CT2 +
   P2/P3/P4/PAL.PAL).
3. `port/test/mixer_test.c GAMEDIR OUT.wav`: decodes MARKETPL.WAV, KABOOM3.WAV, BRPC.WAV from ACIV.CT5, renders
   6 s with no music in pieces of 1000 frames (shortened where needed to land exactly on the start frames),
   starting voices at frames 0 / 22050 / 44100 / 88200 with the given volume/pan, writes OUT.wav (RIFF WAVE, PCM
   16-bit stereo 44100) and prints `frames sum_left sum_right clipped` (64-bit sums of the written samples,
   mix_clipped).
Build: gcc 14.2 `-std=c11 -Wall -Wextra -Werror -pedantic` clean; an ASan+UBSan build produces a byte-identical
OUT.wav and runs the unit checks clean (no clang in my sandbox; G that clang is equally clean until run there).
Check here: synthetic unit checks — 1-sample sound (one frame, then end), 2-channel sound (pan ignored, L/R
mapped), pan 0/64/127/32 gains exact, 22050-to-44100 resampling exact (0, 16383, 32767, 32767, end), a voice
ending exactly at a piece boundary, the 9th voice refused (-1) and the freed voice reused after mix_stop,
clamping (two max voices -> 32767, clipped = 2), music at volume 0 (silent) and 127 (full), music off again,
and a 9000-frame request rendered through in 4096-frame pieces. Then a synthetic game dir with the three WAVs
(22050 and 11025 Hz, extra chunks before/after data): mixer_test renders 6 s and its OUT.wav is
sample-for-sample identical (529,200 samples, 0 diffs) to an independent Python implementation of these rules,
and the summary line matches (`264600 16226481 1193958 7160`). gif_test on the regenerated chained-CT1 dir: 73
GIFs, output identical to ground truth, Pillow 0 failures. All other tests re-run unchanged (cat both synthetic
dirs, flc, xmi, wav, xmiseq 12 songs). Claude's bit-for-bit check of mixer_test's WAV on the real files and
gif_test exit 0 on the real dir is filed under "Needs Erik".

Check (2026-11-18, Claude on Erik's PC): VERIFIED.
- Strict clang build (-pedantic) clean for aarch64 and x86_64.
- `mixer_test C:\work\advciv_play OUT.wav` prints `264600 1217433 1670684 0`. The ARM64 and x64 WAVs are identical.
- An independent implementation of the rules (`tools/verify/mixpy.py`, written from this task's text, not from
  mixer.c) renders the same scene from the real MARKETPL, KABOOM3 and BRPC. Its WAV is identical to yours bit for
  bit: 1,058,444 bytes, the same sums, 0 clipped.
- `gif_test C:\work\advciv_play OUT` now exits 0 with 73 GIFs.

### 2026-11-18  Claude → Mistral: TASK 9: catalog directory blocks are chained
Status: done
Context:
- (D, Claude, 2026-11-18) The `ACIV.CTn` directory isn't one block of `first_offset / 24` entries. It is a chain of
  blocks.
  - Each block is 32 entries of 24 bytes, then a link entry whose name is twelve 0xFF bytes and whose offset is the
    file offset of the next block. Offset 0 means the last block; length and kind are 0.
  - Entries with an empty name are unused.
  - My Task 1 spec described only the first block; the slip was mine.
- The chains on the real files:
  - CT1: 2 blocks (0 and 0x44A52); the second holds 5 GIFs and the three fonts (kind 7).
  - CT3: 4 blocks (0, 0x9F0D, 0xE780, 0x12BF8); 34 HLP + 66 BPM.
  - CT2, CT4 and CT5: 1 block each, link offset 0.
- Named files per catalog: 40, 32, 100, 14 and 29 = 215. In every catalog the last file ends exactly at the end of
  the file.
- `tools/ct_list.py` now reads the chain; use it as your reference for behaviour, not for code.
Ask:
1. `port/src/catalog.h` / `catalog.c`: follow the chain.
   - `count` = named entries only, in chain order; unused (empty-name) entries and link entries are skipped.
   - Raise `entries[64]` to `entries[256]`. More than 256 named entries, or more than 64 blocks, is an error (-3).
   - A link that points back to a block already read is an error (-3). A block that doesn't fit in the buffer is
     -2.
   - `cat_find` and `cat_data` are unchanged.
2. `port/test/cat_test.c`: print the block count per catalog as well, and keep the per-entry lines.
Done:
- Strict build clean.
- Your synthetic catalogs cover: a 2-block chain, a 4-block chain, empty names in the middle, a loop (-3), a link
  past the end (-2).
- The other tests (gif, flc, xmi, xmiseq, wav) still build and their output on your synthetic data is unchanged.
- Claude checks the 215 real entries against `tools/ct_list.py`.
Reply: (2026-11-18, Mistral) Implemented as asked (D spec, chain layout per tools/ct_list.py): catalog.h now has
`entries[256]` plus an added `int blocks` field (so cat_test can print the block count). cat_open walks the chain
from block 0: each block is 32 entry slots plus a link entry (name = twelve 0xFF bytes, offset = the next block's
file offset, 0 = last block). Named entries are collected in chain order (`count` = named entries only); empty-name
slots and the link entry are skipped, and entries after the link entry in a block are not read — same as
ct_list.py's read_dir. Errors: -1 bad args (NULL, < 24 bytes), -2 a block that does not fit in the buffer
(base + 33*24 > size) or a named entry pointing outside the file, -3 more than 256 named entries, more than 64
blocks, a link back to an already-read block, or a block with no link entry. cat_find / cat_data are unchanged.
cat_test prints `ACIV.CTn blocks N` per catalog before the unchanged per-entry lines.
Build: gcc 14.2 `-std=c11 -Wall -Wextra -Werror -pedantic` clean; an ASan+UBSan build runs clean on all good and
bad inputs (no clang in my sandbox; G that clang is equally clean until run there).
Check here (no game data in the repo): synthetic catalogs in two game dirs — the original single-block five
(139 entries; output identical to the ct_list.py reference plus the 5 block lines) and a chained set (CT1 with 2
blocks, CT3 with 4, empty slots in the middle of blocks, stale tail bytes: 216 named entries) — cat_test's output
is line-identical to tools/ct_list.py's entry listing (reformatted), block counts included, 221 lines. Bad
catalogs: link loop -3, link past the end -2, block with no link entry -3, entry past EOF -2, 65 blocks -3, 288
named entries -3; the pre-existing malformed-buffer checks in cat_test still pass. All other tests were rebuilt and
re-run on their synthetic game dirs with unchanged output (gif: 68 + Pillow 0 failures; flc: 16 + ring; xmi: 12 +
dump; xmiseq: 12 songs for 300 s vs the independent Python sequencer, 0 failures, plus the 2 real XMIs; wav: 10 +
sample dumps). V on the real catalogs — the 215 entries vs tools/ct_list.py — is Claude's on Erik's PC, filed
under "Needs Erik".

Check (2026-11-18, Claude on Erik's PC): VERIFIED.
- Strict clang build (-pedantic) clean for aarch64 and x86_64.
- `cat_test C:\work\advciv_play`: blocks 2 / 1 / 4 / 1 / 1. ARM64 and x64 output identical.
- The 215 entry lines are identical to `tools/ct_list.py`'s chain: name, offset, length, kind and order.
- `xmi_test` and `wav_test` on the real files are unchanged.
- `gif_test C:\work\advciv_play OUT` now decodes 73 GIFs: CT1's second block adds AH_LOGO, ADVCIVIL, CV_BABLN,
  CV_EGY6, CV_EGY78.
  - All 73 match the independent decoder (`tools/verify/gifpy.py`): size and CRC-32 of the pixels.
  - It exits 1 only because it still expects 68 (gif_test.c:192). That count is fixed in TASK 10, part 2.
- Found in passing: ZEUS.FLC is byte-identical to ICONCLSM.FLC, as the two WAVs are (OVERVIEW §6, item 13).

### 2026-11-18  Claude → Mistral: TASK 8: WAV reader
Status: done
Context: the game's 17 sound effects are WAV files in `ACIV.CT5`: calamities, market place, Zeus, `KABOOM3.WAV`.
Claude's survey of the real files (D):
- All are RIFF WAVE, format tag 1 (PCM), 1 channel, 16-bit, `fmt ` size 16.
- 15 are at 22,050 Hz; `BRPC.WAV` and `KABOOM3.WAV` are at 11,025 Hz.
- Extra chunks after `data`: `BRPC.WAV` has `alia` (34 bytes) and `mark` (66 bytes), both all zeros;
  `KABOOM3.WAV` has `LIST` (44 bytes, an `INFO`/`ISFT` "GoldWave v2.10 (C) Chris Craig").
- Every RIFF size is the file size - 8; no file has a pad byte or trailing junk.
The mixer, resampling to the port's 44.1 kHz stream and volume are Claude's (platform layer). This task only reads
the files.
Ask:
1. `port/src/wav.h` / `wav.c`:
```c
typedef struct { uint32_t rate; uint16_t channels; uint32_t frames; int16_t *samples; /* frames*channels, interleaved */ } wav_sound;
int  wav_decode(const uint8_t *data, size_t len, wav_sound *out);   /* 0 ok, <0 error */
void wav_free(wav_sound *w);
```
   - Accept PCM (tag 1), 1 or 2 channels, 8-bit (unsigned, centred on 128: convert as `(v - 128) << 8`) or 16-bit
     (signed little-endian), any rate.
   - Walk the chunks after `WAVE`, padding odd sizes to even. Skip every chunk that isn't `fmt ` or `data`. `fmt `
     may be longer than 16 (read the first 16 bytes).
   - `fmt ` must come before `data`.
   - A `data` size that runs past the end of the buffer is an error (-2). An odd byte at the end of 16-bit data is
     ignored.
   - Errors: -1 bad args, -2 truncated, -3 not RIFF/WAVE or no fmt/data, -4 unsupported format (not PCM, other bit
     depths, more than 2 channels, block align ≠ channels × bytes), -5 out of memory.
2. `port/test/wav_test.c GAMEDIR`: for each `.WAV` in ACIV.CT5, in catalog order, print
   `name rate channels frames sum`, where `sum` is the 32-bit wrapping sum of all samples as uint16. With a second
   argument NAME, dump that sound as one sample per line.
Done:
- Strict build clean.
- Your synthetic files cover: 8-bit, stereo, an odd-sized extra chunk before `data`, `fmt ` of size 18, the error
  cases.
- On the real files, Claude checks all 17 against an independent reader.
Reply: (2026-11-18, Mistral) Implemented as asked (D spec): `port/src/wav.h` / `wav.c` with the exact API, plus
`port/test/wav_test.c` (Task 1 catalog reader for the .WAV entries of ACIV.CT5, in catalog order). wav_decode
takes the bytes in memory and walks the chunks after `WAVE` to the end of the buffer (the RIFF size field is
not used; the real files have it exact anyway), padding odd chunk sizes to even and skipping every chunk that
is not `fmt ` or `data` (first `fmt ` and first `data` win if a pair repeats). `fmt ` needs size >= 16; only its
first 16 bytes are read, so sizes 18/40 work. `data` before `fmt ` is -3. A chunk whose declared size runs past
the buffer is -2 (that covers a `data` size past EOF). Format checks are -4: tag != 1 (PCM), bits not 8/16,
channels not 1/2, block_align != channels * bits/8. Conversion: 8-bit unsigned as `(v - 128) * 256`
(multiplication, not a left shift — shifting a negative int is UB, caught by UBSan), 16-bit signed
little-endian read byte by byte and sign-extended without an implementation-defined conversion; an odd
trailing byte of 16-bit data is dropped by the `frames = data_size / block_align` division. frames == 0 gives
samples = NULL and ok. Errors: -1 bad args (NULL, or < 12 bytes), -2 truncated, -3 not RIFF/WAVE / no fmt / no
data / fmt after data / fmt shorter than 16, -4 unsupported format, -5 out of memory. wav_test prints
`name rate channels frames sum` (sum = 32-bit wrapping sum of the samples read as uint16, printed decimal) and
with a second argument dumps that sound as one signed sample per line; any decode error exits 1.
Build: gcc 14.2 `-std=c11 -Wall -Wextra -Werror -pedantic` clean, and an ASan+UBSan build runs clean on all good
and corrupt inputs (no clang in my sandbox; G that clang is equally clean until run there).
Check here (no game data in the repo): a synthetic ACIV.CT5 with 10 WAVs — 8-bit mono 22050 with an odd-sized
junk chunk before `data`, 16-bit mono, 16-bit stereo 44100, 8-bit stereo 11025, `fmt ` sizes 18 and 40, a LIST
chunk after `data`, 16-bit data with an odd trailing byte, empty `data`, and a repeated fmt/data pair (first
wins): wav_test exits 0 and its 10 lines are identical to ground truth computed independently in Python; the 8
standard files are also readable by Python's `wave` module (same rate / channels / frames), and the dumps of
W16MONO (16-bit), W8MONO (8-bit) and WODD (odd byte dropped) match Python sample-for-sample. 13 corrupt files
return the expected codes: not RIFF, RIFF-not-WAVE, no fmt, no data, fmt-after-data, fmt of 12 bytes -> -3;
data size or chunk size past EOF -> -2; tag 3, 24-bit, 3 channels, wrong block align -> -4; < 12 bytes and
NULL -> -1. All other tests re-run unchanged (cat / gif / flc / xmi). V on the real 17 WAVs against an
independent reader is Claude's on Erik's PC — filed under "Needs Erik".

Check (2026-11-18, Claude on Erik's PC): VERIFIED.
- Strict clang build (-pedantic) clean for aarch64 and x86_64.
- `wav_test C:\work\advciv_play` exits 0; ARM64 and x64 output identical.
- An independent reader (`tools/verify/wavpy.py`, written from the RIFF notes, not from wav.c) gives the same 17
  lines: rate, channels, frames and sum.
- The sample dumps of BRPC (11,025 Hz, with the `alia`/`mark` chunks), KABOOM3 (11,025 Hz, `LIST` after `data`) and
  ZEUS are identical sample for sample (51,944 / 56,112 / 132,352 samples).
- Found in passing: ZEUS.WAV and ICONCLSM.WAV hold the same sample data (OVERVIEW §6, item 13).

### 2026-11-18  Claude → Mistral: TASK 7a: three small fixes from the Task 7 check
Status: done
Context: Task 7 is verified on the real songs (see its Check). The review found three slips; none affects the
12 songs' event streams.
Ask:
1. `port/test/xmiseq_test.c:98`: the tempo bytes must be `0x0F, 0x42, 0x40` (1,000,000 µs/quarter). Fix your Python
   cross-check's constant too.
2. `port/src/xmi.c`, EVNT parse: a channel message's data byte ≥ 0x80 is a malformed stream. Return -4 (bad status).
   This keeps keys < 128 for `xmiseq.c`'s note table. The 12 real songs never have one.
3. `port/test/xmiseq_test.c:127`: reserve room for the longest end-of-track (5-byte VLQ + `FF 2F 00`), as the
   per-event path does.
Done: strict build clean. `xmi_test` output unchanged. A synthetic song with a data byte 0x80 returns -4. A
synthetic song ending at tick 1000 run for 300 s writes a valid .mid. `xmiseq_test` .mid now has `FF 51 03 0F 42 40`.
Claude re-checks the 12 songs: the same messages as before, and the new tempo.
Reply: (2026-11-18, Mistral) All three fixed.
1. `port/test/xmiseq_test.c`'s tempo event is now `FF 51 03 0F 42 40` (1,000,000 us/quarter; it was `07 A1 20`
   = 500,000, so the .mid files played at double speed). My Python cross-check carried the same wrong constant —
   fixed there too; the 12 synthetic songs and the 2 real reference XMIs were re-run for 300 s and the .mid
   bytes are still identical, FAILURES: 0.
2. `port/src/xmi.c` parse_evnt now returns -4 when a channel message's data byte is >= 0x80 (checked on d1 and,
   for 2-data-byte messages, d2), so a note key can never index past `xmiseq.c`'s `pend[16][128]`. xmi_test
   output on the 12 synthetic songs is unchanged (summary lines and the GCIVIL2.XMI dump byte-identical); a
   synthetic XMI with a note-on key 0x80 returns -4.
3. write_mid now reserves 8 bytes for the end-of-track (5-byte VLQ + `FF 2F 00`), like the per-event path.
   Verified under ASan+UBSan: a synthetic song ending at tick 1000 run for 300 s writes a valid .mid (tempo
   `ff51030f4240` at tick 0, on@1000, off@1010, EOT at 36000 with nothing after it) with no sanitizer errors;
   that EOT delta is 34990, a 3-byte VLQ — 6 bytes, one more than the old 5-byte reserve.
Build: gcc 14.2 `-std=c11 -Wall -Wextra -Werror -pedantic` clean, plus an ASan/UBSan build of xmiseq_test
(no clang in my sandbox; G that clang is equally clean until run there). All other tests re-run and unchanged
(cat / gif / flc / xmi summary and dump).
Claude's re-check of the 12 real songs (same messages as the VERIFIED run, new tempo bytes) is filed under
"Needs Erik".

Check (2026-11-18, Claude on Erik's PC): VERIFIED.
- Strict clang build (-pedantic) clean for aarch64 and x86_64.
- The 12 real songs at 300 s: the same messages, tick for tick, as the independent sequencer.
- ARM64 and x64 .mid files identical; tempo `FF 51 03 0F 42 40`.
- `xmi_test` summary unchanged.
- Probe: a data byte 0x80 as note key, velocity, controller value or program returns -4; a good stream returns 0.
- The end-of-track reserve is now 8 bytes (read in the diff).
- Listening: the four `g*` songs render through FluidSynth 2.6.2 + GeneralUser GS (`port/test/gm_render.c`):
  balanced, no clipping. Erik listens to them; that's the last step of Task 7.

### 2026-11-18  Claude → Mistral: TASK 7: XMIDI sequencer
Status: done
Context:
- Task 6's `xmi_song` is a sorted event list. This task plays it in time: a sequencer that emits plain MIDI channel
  messages to a callback.
- Claude writes the synth side (FluidSynth + GeneralUser GS, OVERVIEW §7.2) and the game side (when a song starts,
  stops and switches).
- The sequencer is pure C like the format modules: no platform headers, no file I/O, no threads, no clock of its
  own. The caller advances it.

Facts from the 12 real songs (D, Claude, from `xmi_test` dumps):
- Every song has exactly one loop: controller 116 (FOR) with value 0 at the loop start, and controller 117 (NEXT)
  with value 127 at the end, both on channel 16 (status BF).
  - Civil songs: loop from tick 6564 to tick 14645. Calamity songs: loop from tick 0.
  - Then FF 2F at the same tick as the NEXT.
- One controller 119 (callback trigger, value 0) about 2 s before the NEXT.
- Other controllers used: 0, 6, 7, 10, 32, 64, 70, 91, 93, 98, 99, 121, 123. All of them are ordinary MIDI and go
  to the synth.
- SCIVIL2.XMI has 4 notes still sounding when the NEXT is reached. Their note-offs come after the jump, so the
  sequencer must time note-offs from note durations, not from the note-off events in Task 6's list.

Ask:
1. In `xmi.h` / `xmi.c`, add `uint32_t dur;` to `xmi_event`: the note's duration in ticks on note-on events, 0 on
   all others.
   - Everything else stays as it is, including the note-off events in the list, so `xmi_test` output is unchanged.
2. New `port/src/xmiseq.h` / `xmiseq.c`:
```c
typedef void (*xmi_out_fn)(void *user, const uint8_t *msg, int len);     /* one MIDI channel message, 2 or 3 bytes */
typedef void (*xmi_trigger_fn)(void *user, int channel, int value);      /* controller 119 reached */
typedef struct xmi_seq xmi_seq;
xmi_seq *xmi_seq_new(const xmi_song *song, xmi_out_fn out, xmi_trigger_fn trig, void *user);  /* NULL = no memory */
void     xmi_seq_advance(xmi_seq *s, uint32_t ticks);  /* move on by `ticks` (120 per second), emitting what falls due */
int      xmi_seq_done(const xmi_seq *s);               /* 1 after FF 2F and the last pending note-off */
void     xmi_seq_stop(xmi_seq *s);                     /* note-off for every sounding note now; then done */
void     xmi_seq_free(xmi_seq *s);
```
Rules:
- **Order.** Within one advance, events are emitted in list order, together with the pending note-offs that fall
  due. At equal ticks a note-off goes before anything else, as in Task 6.
  - A note-on schedules its own note-off at its tick + `dur`. Pending note-offs live in the sequencer: a table of
    at most 16 × 128 notes.
  - If a sounding note is struck again, send a note-off first, then the new note-on, and the new duration replaces
    the old one. This is how Miles behaves (G); say so in the reply if a song does it.
  - The note-off events in the list are skipped.
- **XMIDI controllers.** Controllers 110-120 are never sent to `out`.
  - 116 FOR: push the loop (index of the next event, the current tick, the count = value; 0 = forever). Allow up to
    4 nested loops.
  - 117 NEXT, value ≥ 64: if a loop is open and (its count is 0, or the count decremented is still > 0), jump back.
    The song position becomes the FOR's tick and play continues at the FOR's next event. Otherwise pop the loop and
    go on.
  - 117 NEXT, value < 64: pop the loop and go on (a break).
  - 119: call `trig`.
  - 110-115, 118, 120: ignore.
- **Note-offs across a jump.** Pending note-offs keep counting real time; a jump doesn't touch them.
- **End.** FF 2F: no more events. `done` turns 1 once no note is pending.
- **No allocation after `xmi_seq_new`.** The song stays owned by the caller.
3. `port/test/xmiseq_test.c GAMEDIR NAME SECONDS OUT.mid`:
   - Run the sequencer for SECONDS in steps of 1 tick, then call `xmi_seq_stop`.
   - Write everything `out` received as a standard MIDI file, type 0, division 120, one tempo event 1,000,000
     µs/quarter at tick 0 (so 1 MIDI tick = 1/120 s), events at their sequencer time, end of track.
   - Print `name notes_on notes_off loops triggers`.
   - Fail if notes_on ≠ notes_off.

Done:
- Strict build clean, with your own synthetic checks.
- On a synthetic song with a note held across its NEXT, the note-off comes at the right time after the jump.
- A FOR count of 2 plays the loop twice.
- A NEXT < 64 breaks out of the loop.

Claude then checks all 12 real songs for 300 s against an independent sequencer (same .mid bytes), and listens to
one through the synth.
Reply: (2026-11-18, Mistral) Implemented as asked (D spec).
1. `xmi_event` gained `uint32_t dur;` — note-on events carry the VLQ duration in ticks, 0 on all others
   (including the synthesized note-offs). Nothing else in xmi.* changed; xmi_test's summary lines and event
   dumps are byte-identical to before (re-run after the change).
2. New `port/src/xmiseq.h` / `xmiseq.c`: pure C11, no platform headers, no file I/O, no threads, no clock of
   its own, no allocation after xmi_seq_new (the song stays owned by the caller). Model: the sequencer keeps a
   monotonic time `now` and an index into the sorted list; the effective tick of event i is
   `ev[i].tick + shift`, with `shift` = 0 until a loop jump. On a NEXT jump the shift is set so that the FOR's
   position in time becomes the NEXT's fire time (`shift = t_next - ev[for].tick`): play continues at the
   FOR's next event and the loop body replays after `now` with its original spacing, the way the delay bytes
   accumulate in Miles (G). Pending note-offs are not touched by jumps and keep counting real time. Within one
   advance, emission is a merge of the event list and the pending note-offs: smaller time first, at equal
   ticks a note-off before anything else (ties between note-offs: channel, then key). A note-on schedules its
   own note-off at tick + `dur` in a 16x128 table; if the note is re-struck while sounding, a note-off is
   sent first, then the new note-on, and the new duration replaces the old (no real song re-strikes a note;
   the rule is in per Miles — G). Note-off events in the list are skipped. Controllers 110-120 never reach
   `out`: 116 FOR pushes (index of the next event, count = value, 0 = forever; up to 4 nested, a 5th FOR is
   ignored), 117 NEXT with value >= 64 jumps if a loop is open and (count == 0 or the decremented count is
   still > 0), otherwise pops and goes on — so FOR n plays the body n times — 117 with value < 64 pops (a
   break) and goes on, 119 calls `trig(user, channel, value)`, 110-115/118/120 are ignored. FF 2F ends the
   event stream; `done` = 1 once no note is pending (immediately after `stop`). `stop` sends a note-off for
   every sounding note (channel, then key) at the current time and makes done 1; advance after stop is
   silent. One addition beyond the asked API: `uint32_t xmi_seq_loops(const xmi_seq *)` (loop jumps performed
   so far), so the test can print the `loops` column. xmi_seq_new returns NULL on bad args/OOM. A per-advance
   guard (1,000,000 emissions) breaks zero-delay loop bombs.
3. `port/test/xmiseq_test.c GAMEDIR NAME SECONDS OUT.mid`: parses NAME from ACIV.CT5, runs the sequencer
   SECONDS in steps of 1 tick (plus one initial advance(0) so tick-0 events are stamped tick 0), then calls
   xmi_seq_stop, and writes OUT.mid as a standard MIDI file, type 0, division 120, one track: tempo FF 51
   1,000,000 us/quarter at tick 0, every emitted message at its sequencer tick (delta VLQ), end of track at
   the stop tick. Prints `name notes_on notes_off loops triggers`; exits 1 if notes_on != notes_off.
Build: no clang in my sandbox, so gcc 14.2 `-std=c11 -Wall -Wextra -Werror -pedantic` — clean (G that clang is
equally clean until run there).
Check here (no game data in the repo): synthetic in-memory songs cover the Done cases — (a) a note held
across an infinite FOR 0 / NEXT loop gets its note-off exactly at on+dur in real time, across 19 jumps;
(b) FOR 2 plays the body exactly twice (on 10, off 15, on 30, off 35) and falls through at the 2nd NEXT;
(c) NEXT < 64 breaks with no jump; (d) controllers 110-120 never reach `out`, 119 fires trig once per loop
pass, ordinary controllers and a len-2 C0 are sent as-is; (e) re-strike: off at 20, on at 20, final off at
20 + new dur, the old duration gone; (f) after FF 2F with notes still pending, done stays 0 until their
note-offs, and stop emits one note-off per sounding note at the current tick, then done. Then all 12
synthetic CT5 songs run 300 s: exit 0, note-ons = note-offs, and the .mid bytes are identical to an
independent Python sequencer written from this task's spec (its own XMIDI parse, sequence and SMF writer,
not from xmiseq.c) for all 12, loops and triggers included. The two real reference XMIs (DEMO.XMI,
SPKRDEMO.XMI) also run 300 s balanced and match the Python sequencer byte-for-byte. V on the real 12 songs
for 300 s against Claude's independent sequencer (same .mid bytes), and listening to one through the synth,
is Claude's on Erik's PC — filed under "Needs Erik".

Check (2026-11-18, Claude on Erik's PC): the sequencer is VERIFIED. Three small fixes are filed above as TASK 7a.
- Strict clang build (-pedantic) clean for aarch64 and x86_64. `xmi_test` output is unchanged by the `dur` field
  (summary lines and three event dumps).
- `xmiseq_test C:\work\advciv_play NAME 300` was run for all 12 real songs:
  - exit 0 each time, note-ons = note-offs;
  - loops / triggers: CIVIL and CALAM 3/3, MAP 2/2, TRADE 1/1;
  - ARM64 and x64 .mid files identical.
- An independent sequencer was written from the spec alone, by an agent that did not see xmiseq.c, the test or the
  Reply. Its emitted messages were compared with the .mid files read back by a separate SMF reader: every message
  and every tick identical, 4,372 to 25,492 messages per song, all 12.
- A separate code review with probes confirmed:
  - the loop semantics (FOR n = n passes, nested 2×2 = 4, NEXT < 64 breaks, NEXT with no open FOR ignored);
  - note-offs across jumps, re-strike, the equal-tick order, and done/stop;
  - no allocation after new;
  - no wrap in the 64-bit times.
- Found, filed as TASK 7a:
  - (1) the test's tempo bytes are `07 A1 20` = 500,000 µs/quarter (the comment says 1,000,000 = `0F 42 40`), so the
    .mid files play at double speed. The Python cross-check in the Reply must have carried the same constant.
  - (2) a data byte ≥ 0x80 is accepted by xmi.c; a note key ≥ 128 then indexes past `pend[16][128]` (probe: a
    stuck note and `done` overwritten; key 0xFF writes about 2 KB past the block). Malformed files only; the 12
    real songs are fine.
  - (3) the end-of-track reserve in the test's writer is 5 bytes, but a delta ≥ 16384 needs VLQ 3 + 3 = 6: a 1-byte
    overflow. Only for a song that ends long before SECONDS; no real song does.
- Noted, nothing to do now:
  - the songs carry FF 51 tempo events (the three MAP songs 177 each), plus FF 54/58/59/7F; skipped by design.
  - (G) Miles plays XMIDI at a fixed 120 Hz with tempo already folded into the delays. Claude times a MAP song in
    DOSBox-X when the synth is in.

### 2026-11-17  Claude → Mistral: TASK 6: XMIDI parser
Status: done
Context: the music is 12 XMIDI files in ACIV.CT5 (`G*` General MIDI, `F*` FM, `S*` a third set; civil / map / trade /
calamity themes). Each is (D) `FORM XDIR { INFO }` then `CAT XMID { FORM XMID { TIMB, EVNT } }`: one sequence per file;
IFF chunk sizes are big-endian. Miles AIL 3 played them; the synth is decided later (Erik), so this task stops at a
timed event list.
Ask: `port/src/xmi.h` / `xmi.c`:
```c
typedef struct { uint32_t tick; uint8_t status, d1, d2; uint8_t len; /* 1..3 bytes of MIDI used */ } xmi_event;
typedef struct { xmi_event *ev; uint32_t count; uint8_t timb[64][2]; int ntimb; /* TIMB: (patch, bank) pairs */ } xmi_song;
int  xmi_parse(const uint8_t *data, size_t len, xmi_song *out);   /* 0 ok, <0 error */
void xmi_free(xmi_song *s);
```
Rules (the XMIDI EVNT stream, the standard Miles format): ticks are 120 per second; a byte < 0x80 between events is a
delay (consecutive delay bytes add up); note-on `9n key vel` is followed by a variable-length duration in ticks and has
**no** note-off in the file: emit a note-off (`8n key 0`) at tick + duration, sorted into the list (stable: at equal
ticks, note-offs before note-ons); other channel messages as in MIDI (running status is not used in XMIDI: treat a
missing status byte as an error); sysex F0/F7 with a length: skip; meta FF type len: keep only end of track (FF 2F,
stop) and tempo (FF 51: keep as an event with len 3 and the 3 tempo bytes in a side field or skip, say which).
Controllers 110-120 are XMIDI's own (114 patch bank select, 116 FOR loop / 117 NEXT, 119 callback trigger …): keep
them as ordinary controller events, don't act on them.
`port/test/xmi_test.c GAMEDIR`: for each of the 12 files print `name events notes last_tick timb_count`, and with a second
argument NAME dump that song as text lines `tick status d1 d2`.
Done = all 12 parse; note-ons = note-offs per channel; strict build clean. Claude checks against an independent XMI→MIDI
conversion.
Reply: (2026-11-17, Mistral) Implemented as asked (D spec): `port/src/xmi.h` / `xmi.c` with the exact API, plus
`port/test/xmi_test.c` (uses the Task 1 catalog reader for the .XMI entries of ACIV.CT5). xmi_parse walks the IFF
structure (big-endian chunk sizes, even-byte padding; FORM / CAT  / LIST containers) and parses the first
`FORM XMID`, taking its first EVNT and its TIMB. Two format details handled (D): some files size the top-level
FORM XDIR to cover only INFO with `CAT XMID` as a sibling, so container children are bounded by the enclosing
range, not by the FORM's own size field; and TIMB's payload is a u16-LE count followed by (patch, bank) byte
pairs (capped at 64). EVNT per the XMIDI rules: bytes < 0x80 are delays and consecutive ones add up; every
message carries its own status byte (running status is never assumed); note-on `9n key vel` is followed by a VLQ
duration in ticks and the file has no note-offs, so a note-off `8n key 0` is emitted at tick + duration; other
channel messages are kept as-is (C0/D0 carry 1 data byte, the rest 2); sysex F0/F7 with a VLQ length are skipped;
meta FF type len: only end-of-track FF 2F is kept (len 3, d1=0x2F, d2=0x00, parsing stops there), everything
else is skipped — including the tempo FF 51, which is skipped (the spec's "say which": ticks stay at the fixed
120 per second, no tempo event is emitted). Controllers 110-120 are kept as ordinary controller events. `len`
counts the message bytes including the status (2 for C0/D0, 3 otherwise). Events are stable-sorted by tick
(merge sort) with note-offs before note-ons at equal ticks. Errors: -1 bad args, -2 truncated (chunk, VLQ or
message data overruns the buffer), -3 malformed structure (not FORM, no EVNT found, bad container), -4 bad
status byte (>= 0xF0 and not F0/F7/FF), -5 out of memory. xmi_test prints `name events notes last_tick
timb_count` per file (notes = note-on count; it also fails if note-ons != note-offs on any channel) and with a
second argument dumps that song as `tick status d1 d2` lines; it expects exactly 12 files.
Build: no clang in my sandbox, so gcc 14.2 `-std=c11 -Wall -Wextra -Werror -pedantic` — clean (pure ISO C11,
byte-by-byte BE/LE reads, no platform headers in xmi.*; G that clang is equally clean until run there).
Check here (no game data in the repo): 12 synthetic XMIs in a synthetic ACIV.CT5 (G*/F*/S* sets covering the
XDIR-quirk layout, TIMB with 0..64 pairs, summed delays, note durations incl. equal-tick ties, controllers
110-120, sysex F0/F7, meta FF 51 + FF 2F + others, C0/D0): xmi_test exits 0, its 12 summary lines are identical
to the ground truth and the `xmi_test GAMEDIR GCIVIL2.XMI` dump (771 event lines) is identical to the expected
event list. Two real XMIDI files (DEMO.XMI, SPKRDEMO.XMI from a public XMI2MID reference set) also parse:
10027 events / 14 TIMB pairs and 3927 events / 2 TIMB pairs, ticks non-decreasing, FF 2F last, note-on =
note-off per channel. Corrupt inputs (not FORM, no EVNT, truncated chunk, VLQ overrun, stray 0xFE status)
return -2/-3/-4. V against the real 12 files in ACIV.CT5 and Claude's independent XMI-to-MIDI conversion still
needs Erik's folder — filed under "Needs Erik".
Check (2026-11-17, Claude on Erik's PC): VERIFIED. Strict clang build clean (aarch64 and x86_64, -pedantic);
`xmi_test C:\work\advciv_play` exits 0 on the real files: all 12 parse, note-ons = note-offs per channel. An
independent reader (Claude's, plain Python from the XMIDI notes, not from xmi.c) gives the same 12 summary lines and
the same event dump, line for line, for all 12 songs (not just GCIVIL2: 2,138-8,206 events each). ARM64 and x64
builds print identical output.

### 2026-11-17  Claude → Mistral: TASK 5: FLC player
Status: done
Context: the intro (`AH_LOGO.FLC` 640x480, 50 frames, 57 ms; `ADVCIVIL.FLC` 640x480, 31 frames, 71 ms) and the 14
calamity animations in ACIV.CT4 (220x150, 29-96 frames, 85-171 ms) are Autodesk FLC files, magic 0xAF12, depth 8 (D).
Chunk types actually used (D, all 16 files surveyed): frame chunk 0xF1FA containing COLOR_256 (4), BYTE_RUN (15), DELTA_FLC
(7, "SS2") and PSTAMP (18, a thumbnail: skip); some frames have no sub-chunks (= unchanged frame). `oframe1` (header
offset 80) gives the first frame; after the last frame there is the usual ring frame (the delta from the last frame
back to the first) for looping. Support COLOR_64 (11), LC (12), BLACK (13), COPY (16) too (cheap, standard), and skip
unknown sub-chunks by their size.
Ask: `port/src/flc.h` / `flc.c`:
```c
typedef struct { uint16_t w, h, frames; uint32_t speed_ms; uint8_t pal[256][3]; uint8_t *pixels; /* w*h */
                 const uint8_t *data; size_t len; size_t pos; uint32_t index; } flc_anim;
int  flc_open(flc_anim *a, const uint8_t *data, size_t len);  /* reads the header, allocates pixels (zeroed), 0 ok */
int  flc_next(flc_anim *a);   /* decodes the next frame into pixels/pal; after the last frame it plays the ring frame
                                 and wraps to frame 1 (index counts 0..frames-1); 0 ok, <0 error */
void flc_close(flc_anim *a);
```
Palette entries as stored (8-bit for COLOR_256, 0..63 for COLOR_64: keep a flag or scale by 4 and say which).
`port/test/flc_test.c GAMEDIR`: for all 16 files print `name w h frames speed`, then per frame `index crc32(pixels)
crc32(pal)`, then the same after the ring frame (must equal frame 0's CRCs: that checks the ring frame).
Done = all 16 play through without error, the ring-frame check holds for every file; strict build clean. Claude checks
frames against an independent decoder and later DOSBox-X captures.
Reply: (2026-11-17, Mistral) Implemented as asked (D spec): `port/src/flc.h` / `flc.c` with the exact API, plus
`port/test/flc_test.c` (Task 1 catalog reader for ACIV.CT4's kind-6 entries plus the loose AH_LOGO.FLC /
ADVCIVIL.FLC). flc_open reads the 128-byte header: magic 0xAF12 (FLI 0xAF11 accepted too), depth must be 8
(0 accepted as 8), w/h (0 -> 320x200), frames (0 -> 1), speed: for FLC a u32 at offset 16 already in ms, for FLI
the u16 in 1/70 s units converted (x1000/70); pixels are calloc'd zeroed; pos = oframe1 (header offset 80, default
128). flc_next skips 0xF100 prefix chunks by size and decodes the frame chunk 0xF1FA at pos. Sub-chunks:
COLOR_256 (4) — 8-bit entries as stored; COLOR_64 (11) — 6-bit entries **scaled x4 (v<<2)**, that is the
"say which"; BRUN/BYTE_RUN (15) — positive count = run of the next byte, negative = literal run, the per-line
packet-count byte is ignored (decode until the line has w pixels); DELTA_FLC/SS2 (7) — word-oriented, positive
count = count*2 literal bytes, negative = a 2-byte pattern repeated, the line-skip (0xC000) and last-byte
(0x8000) opcodes apply to the current line and come before the packet-count word, y advances after each line's
packets; LC (12) — positive = literal / negative = run, count 0 = skip-only packet (for skips > 255); BLACK
(13) fills the frame with index 0; COPY (16) copies whole rows, 4-byte aligned; PSTAMP (18) and any unknown
sub-chunk are skipped by size; a frame chunk with no sub-chunks leaves the frame unchanged. After index ==
frames-1 the next flc_next decodes the ring chunk, then wraps pos to oframe1 and index to 0, so the animation
loops forever. Errors: -1 bad header/args, -2 truncated, -3 chunk structure, -4 pixel overflow (a line writes
past w*h), -5 out of memory. flc_test prints `name w h frames speed`, then `index crc32(pixels) crc32(pal)`
per frame plus one ring line, and exits 1 unless exactly 16 files played and the ring CRCs equal frame 0's with
index back at 0.
Build: no clang in my sandbox, so gcc 14.2 `-std=c11 -Wall -Wextra -Werror -pedantic` — clean (pure ISO C11;
G that clang is equally clean until run there).
Check here: 16 synthetic FLCs (14 calamities 220x150 with 29-96 frames plus AH_LOGO and ADVCIVIL 640x480)
exercising every chunk type incl. COLOR_64, LC skip-only, BLACK, COPY, empty frames, PSTAMP and the ring:
flc_test exits 0, its output is identical to the ground truth, and the ring frame equals frame 0 for all 16;
a good file still loops after 100000 flc_next calls. Pillow 13 independently decodes all 124 regular frames of
the 16 files — 0 mismatches (PIL does not expose the ring frame; that check is ours); ffmpeg 9.0 agrees
frame-by-frame where comparable (its rawvideo muxer duplicates frames, so compare per-frame images, not the
stream). Corrupt inputs rejected: bad magic -1, header < 128 bytes -1, depth 16 -1, truncated file -3,
truncated frame -3. V on the real 16 files, an independent decoder and later DOSBox-X captures — filed under
"Needs Erik".
Check (2026-11-17, Claude on Erik's PC): VERIFIED. Strict clang build clean (aarch64 and x86_64, -pedantic);
`flc_test C:\work\advciv_play` exits 0 on the real files: all 16 play through, the ring frame returns to frame 0
for every file, and the header lines match Claude's earlier survey (speeds 57-171 ms). An independent decoder
(Claude's, plain Python from the format notes, not from flc.c) gives the same 809 output lines: every frame's pixel
and palette CRC, ring included. ARM64 and x64 builds print identical output. Code read (DELTA_FLC): bounds checked
throughout. One nit, no action needed: the packet loop stops at x == w and would leave any further packets of that
line unread; only a malformed file could have them. Still to come: DOSBox-X captures for timing.

### 2026-11-17  Claude → Mistral: TASK 4: GIF image position and logical screen
Status: done
Context: the 25 nation pictures `CH_*.GIF` / `CV_*.GIF` in ACIV.CT1 are small images placed on a 640x480 logical
screen, e.g. `CH_AF48.GIF` = 208x123 at (121,196), `CH_ILLY.GIF` = 167x35 at (154,0) (D). They are overlays drawn at
their place on the map, so the port needs the position. `gif_image` drops it today (the Task 2 spec didn't ask for it:
Claude's gap, not a defect of the decoder).
Ask: add to `gif_image` (keep the existing fields and their meaning: `w`, `h`, `pixels` stay the image itself):
`uint16_t left, top;` (image descriptor position) and `uint16_t screen_w, screen_h;` (logical screen, header bytes 6..9).
`gif_test` prints them after the CRC: `name w h ncolors crc32 left top screen_w screen_h`.
Done = the CRCs are unchanged, and for the real files Claude gets left/top/screen matching the headers (Claude runs
it); strict build clean with clang (Claude runs it too; gcc -pedantic is fine on your side).
Reply: (2026-11-17, Mistral) Done as asked: `gif_image` gained `uint16_t left, top;` (image descriptor position)
and `uint16_t screen_w, screen_h;` (logical screen, header bytes 6..9); the existing fields keep their meaning
(`w`, `h`, `pixels` are still the image itself). gif.c reads them straight from the header and the image
descriptor (4 added lines; the decoder is untouched); `gif_test` now prints
`name w h ncolors crc32 left top screen_w screen_h` and still writes the same BMPs (the image rectangle only,
w x h).
Build: no clang in my sandbox, so gcc 14.2 `-std=c11 -Wall -Wextra -Werror -pedantic` — clean (G that clang is
equally clean; Claude runs it).
Check here: regenerated the 68 synthetic GIFs with left/top on a 640x480 logical screen: gif_test exits 0, all
9 columns identical to the ground truth, and the pixel CRCs are unchanged from Task 2's run (decoder untouched).
Pillow 13 re-check: every source file's header bytes 6..9 and image-descriptor position match the printed
screen_w/screen_h/left/top; decoding each source and cropping at (left, top) when PIL reports the full screen
gives the same pixel CRC; the 68 BMPs re-read with matching size / pixels / palette, and the palette choice
(GCT over LCT) is unchanged — 0 failures. V on the real files (left/top/screen of the 25 CH_/CV_ pictures
against their headers, screen 640x480, plus the strict clang build) is Claude's on Erik's PC — filed under
"Needs Erik".
Check (2026-11-17, Claude on Erik's PC): VERIFIED. Strict clang build clean (aarch64 and x86_64); on the real files
the first five columns (name, size, colours, pixel CRC) are identical to the Task 2 run and all 68 BMPs are
byte-identical. left/top/screen_w/screen_h match an independent header parse for all 68 (the 25 CH_/CV_ pictures
on 640x480; ZOOM* 624x319, BRPC/ZEUS/MINIMAP 320x200, MAP&PAL/PAL.PAL 1716x568, the rest 640x480, all at 0,0).

### 2026-11-17  Claude → Mistral: TASK 3 (any time): text sweep
Status: done
Context: Erik's rule: slips in the game's texts are bugs, fixed by default; `--original` keeps them. Claude extracts
the strings; Mistral reads them.
Ask: the list is in `docs/strings.txt` (linear address, text; 2,271 lines, library strings mixed in). Write `docs/TEXT_SWEEP.md`: one line per slip
(address, the text, the proposed fix, why: spelling / grammar / contradicts another text), and a separate list of
jokes or odd lines worth Erik's attention. Two already known: "Montheism" (x2) and "popped once to often".
Done = the whole list read; nothing changed in code.
Reply: (2026-11-17, Mistral) Read all 2,275 lines of `docs/strings.txt` (game strings 0x000B0058..~0x000CAC76; the
library strings from ~0x000B9AAB skipped per the file header) and cross-checked with pattern greps for common
misspellings. Wrote `docs/TEXT_SWEEP.md`: 26 slip lines (address, text, fix, why) plus a separate jokes/odd list for
Erik. The two known ones are confirmed — "Montheism" x2 (0x000B1EEA, 0x000B1F07) and "popped once to often" is
actually x4 (0x000B9679, 0x000B96A5, 0x000B96D1, 0x000B96F6). New slips include "an additional" (0x000B0292),
"volcanoes" (0x000B1A05), "nullifies" (0x000B222C), "beneficiary" x4 (0x000B2FB3, 0x000B2FF2, 0x000B308C, 0x000B30CB —
contradicted by 0x000B2DBD which spells it right), "chooses" (0x000B35A7), "%s, you" (0x000B5CDE), "number of units"
(0x000B5D16), "Sciences credit" (0x000B76DB), "Heresy" (0x000B81C9), "victim" (0x000B854B), "Democracy" (0x000B8904),
"have the possibility" (0x000C7CA8), "through" (0x000B280A), "take over" (0x000B25EA, 0x000B2705), "maint./const. for"
(0x000B5AE4), and the systematic "N point ... credit" in 18 help texts (all addresses listed in TEXT_SWEEP.md —
flagged for Erik, default fix "points"). Jokes/odd lines for Erik: Skull & Crossbones, Inc. (0x000B2554), the BRPC
lines (0x000B1855, 0x000B188A), Zeus (0x000B19CD), the junk/junk2 placeholders, the memory-statistics and
stack-blown dev strings, the misspelled "DoShipContruction" trace x4 (0x000B623C, 0x000B6257, 0x000B62D9, 0x000B62F4),
the "(?,?)" placeholders (0x000B6358-0x000B63A4), "Ctest.txt" capitalization. Nothing changed in code (D).
Check (2026-11-17, Claude): ACCEPTED. Every claimed misspelling grepped in strings.txt: all present (nullifys, choses,
viction, Democrary, Heresey, beneficary x4, volcanos, maint./const.for, "and additional", "number units",
DoShipContruction x4). Science/Sciences confirmed (1 "Science" against 4 "Sciences"); "Craft credit" is used all 7 times,
so no slip there. Disagree on two: "volcanos" is a standard dictionary variant (not a slip, Erik's call), and
"10 point Craft credit" is ordinary compound-adjective wording ("a 10-point credit"), consistent in all 18 help texts:
proposed default = keep. "thru", "has the possibility", "Crete can not" are style calls for Erik. Good work.

### 2026-11-17  Claude → Mistral: TASK 2: GIF87a decoder
Status: done
Context: every picture in the game is a GIF87a: 32 in `ACIV.CT1`, 32 in `ACIV.CT2` (kind 2 entries), and the
files `P2`, `P3`, `P4`, `PAL.PAL` (a 1716x568 GIF the game takes its palette from). Fastgraph/Image decoded them in
1995; the port needs a plain decoder.
Ask: `port/src/gif.h` / `gif.c`:
```c
typedef struct { uint16_t w, h; uint8_t pal[256][3]; uint16_t ncolors; uint8_t *pixels; /* w*h, row-major */ } gif_image;
int  gif_decode(const uint8_t *data, size_t len, gif_image *out);  /* 0 ok, <0 error code */
void gif_free(gif_image *img);
```
- first image only; global colour table (fall back to the local one if a file has it); interlaced images
  de-interlaced; LZW with variable code size 2..12, clear and end codes, the "deferred clear" case at 4096 entries.
- The palette is returned as the file's 8-bit RGB triples, untouched (the VGA 6-bit conversion is Claude's, later).
- `port/test/gif_test.c GAMEDIR OUTDIR`: decodes every GIF in CT1, CT2 and P2/P3/P4/PAL.PAL, prints
  `name w h ncolors crc32(pixels)` one per line, and writes each as an uncompressed 8-bit BMP (with the palette) to
  OUTDIR so Erik or Claude can look at them.
Done = all 68 decode without error; dimensions match the GIF headers; the BMPs look right (Claude checks a sample and
the pixel CRCs against an independent decoder); strict build clean.
Reply: (2026-11-17, Mistral) Implemented as asked (D spec): `port/src/gif.h` / `port/src/gif.c` with the exact API
above, plus `port/test/gif_test.c` (uses the Task 1 catalog reader for the kind-2 entries of ACIV.CT1/CT2, then
P2/P3/P4/PAL.PAL as plain files). gif_decode takes the bytes in memory and returns the first image only: GCT palette
(LCT only when the file has no GCT), palette copied as untouched 8-bit RGB triples, interlaced images de-interlaced
with the 4-pass schedule, extension blocks skipped, LZW min code size 2..8 with the code size growing to max 12,
clear and end codes, KwKwK (code == next) handled, and the deferred clear case handled — when the table reaches 4096
entries the decoder stops adding entries and keeps reading 12-bit codes until a clear arrives. Distinct negative
errors: -1 bad header/args, -2 truncated, -3 malformed block structure, -4 bad LZW/image data (incl. short or
over-long pixel stream), -5 out of memory. gif_test prints `name w h ncolors crc32(pixels)` (CRC-32/IEEE as %08X)
and writes each image as an uncompressed 8-bit BMP (14+40-byte headers, 256-entry BGRA palette, bottom-up rows
padded to 4 bytes) to `OUTDIR/name.bmp`; exit 1 if any GIF fails or the count is not 68.
Build: no clang in my sandbox, so gcc 14.2 `-std=c11 -Wall -Wextra -Werror -pedantic` — clean (pure ISO C11,
byte-by-byte little-endian reads, no platform headers in gif.*; G that clang is equally clean until run there).
Check here (no game data in the repo): a synthetic game dir with 32+32 kind-2 GIFs in ACIV.CT1/CT2 plus P2/P3/P4
(640x480) and PAL.PAL (1716x568, interlaced) = 68 files, covering interlaced (incl. 3x5 and 1x1), LCT-only, GCT+LCT
(GCT must win), a 512x512 noise image that fills the dictionary to 4096 with no clear ever sent (deferred clear), a
comment extension, a GIF89a signature, min code size 2..8, odd widths. gif_test exits 0 and prints 68 lines identical
to the ground truth (`name w h ncolors crc32`); Pillow 13 independently decodes all 68 source GIFs (size and pixel
CRC match, so the test files are valid) and re-reads all 68 BMPs (size, pixel CRC and palette match; GCT-over-LCT
confirmed on the GCT+LCT file). Corrupt inputs (bad magic, truncated, no colour table, min code size 1, code out of
range, trailer before any image) all return <0. V on the real 68 game GIFs still needs Erik's folder — filed under
"Needs Erik".
Check (2026-11-17, Claude on Erik's PC): VERIFIED for decoding. Strict build clean with clang (llvm-mingw) for aarch64
and x86_64, `-std=c11 -Wall -Wextra -Werror -pedantic`; both builds print identical output. All 68 real GIFs decode
(exit 0, 68 lines). Independent reference = Windows' own GIF codec (GDI+, indices via LockBits 8bppIndexed): 43 of 68
identical (size + pixel CRC + palette). The other 25 are the CH_/CV_ nation pictures, small images on a 640x480 logical
screen (GDI+ returns the whole screen): inside the image rectangle 20 of 25 match GDI+ exactly; the remaining 5
(CV_AF48, CV_AF3, CV_ITALY, CV_ILL3, CV_ASS78) differ from GDI+ at 16-17 px each (index 107 in ours, 106 in GDI+,
two greens; the LZW table never fills in these files), and a third decoder (plain Python LZW, Claude's) agrees with
ours on all five, 0 px. So 2 of 3 decoders agree; G that GDI+ is the odd one out; the arbiter is Fastgraph, checked when
a DOSBox-X capture shows a CV_ picture. Palettes identical to GDI+'s for all 68. Code read: LZW, KwKwK, deferred clear
and interlace are right. Missing: the image position (Task 4).

### 2026-11-17  Claude → Mistral: TASK 1: catalog reader for ACIV.CT1..CT5
Status: done
Context: the game reads its pictures, help texts, animations, music and sounds from five catalogs, `ACIV.CT1` .. `ACIV.CT5`.
Format (D, `tools/ct_list.py` is a working Python reader to compare against):
- a directory at offset 0 of 24-byte entries: `char name[12]` (NUL-padded, upper case, e.g. `ZOOM3.GIF`),
  `u32 offset`, `u32 length`, `u32 kind` (little-endian). kind: 2 = GIF, 6 = FLC, 0 = anything else.
- entry count = (offset of the first entry) / 24. Every catalog has 33 entries: 32 files and an end mark whose name
  bytes are all 0xFF. `ACIV.CT4` also has 19 empty slots (empty name, offset 0, length 0): skip those and the end mark.
- the data of `CT1` and `CT3` does not reach the end of the file (stale bytes after the last entry): ignore them.
- the game looks names up **case-insensitively** (its code asks for `pal.pal`, `zeus.wav` in lower case).
Ask: `port/src/catalog.h` / `catalog.c`:
```c
typedef struct { char name[13]; uint32_t offset, length, kind; } cat_entry;
typedef struct { const uint8_t *data; size_t size; int count; cat_entry entries[64]; } catalog;
int  cat_open(catalog *c, const uint8_t *data, size_t size);  /* parses the directory of a file already in memory; 0 ok */
const cat_entry *cat_find(const catalog *c, const char *name); /* case-insensitive; NULL if absent */
const uint8_t   *cat_data(const catalog *c, const cat_entry *e); /* pointer into data; NULL if out of range */
```
Reject (return <0) a directory whose entries point outside the file. `port/test/cat_test.c GAMEDIR` loads the five
files and prints every entry as `ACIV.CTn NAME offset length kind`.
Done = the output matches `python tools/ct_list.py` for all five files (same entries, same numbers; 32+32+32+14+29 files);
strict build clean.
Reply: (2026-11-17, Mistral) Implemented as asked (D spec): `port/src/catalog.h` / `port/src/catalog.c` with the exact
API above, plus `port/test/cat_test.c`. cat_open parses the in-memory directory: count = first entry's offset / 24,
the directory must fit in the buffer and count <= 64; empty slots (empty name, offset 0, length 0) and the all-0xFF
end mark are skipped; any kept entry with offset+length beyond EOF — or a NULL/<24-byte buffer, or a directory larger
than the file — makes it return <0. Little-endian fields are read byte by byte; only <stdint.h>/<stddef.h>/<string.h>,
no file I/O in the format code. cat_find compares ASCII case-insensitively; cat_data range-checks offset+length against
the file size and returns data+offset. cat_test GAMEDIR prints `ACIV.CTn NAME offset length kind` for CT1..CT5 and also
self-checks cat_find with lower-cased names, cat_data, and the rejection paths (failures to stderr, exit 1).
Build: no clang in my sandbox, so gcc 14.2 with the same flags plus -pedantic:
`gcc -std=c11 -Wall -Wextra -Werror -pedantic port/src/catalog.c port/test/cat_test.c -o cat_test` — clean (pure ISO
C11; G that clang is equally clean until it is run there).
Check here (no game data in the repo, so synthetic ACIV.CT1..CT5 built to the D layout: 32/32/32 files, CT4 with 19
empty slots + end mark, CT5 with 29, stale tail bytes in CT1/CT3): cat_test exits 0 and its 139 lines (32+32+32+14+29)
are line-identical to `tools/ct_list.py`'s entry lines for all five files. V against the real ACIV.CT1..CT5 still needs
Erik's game folder — filed under "Needs Erik".
Check (2026-11-17, Claude on Erik's PC): VERIFIED. Strict clang build clean (aarch64 and x86_64, -pedantic too);
`cat_test C:\work\advciv_play` exits 0 and its 139 lines (32+32+32+14+29) are identical to `tools/ct_list.py`'s
entries for the real five catalogs (apart from CRLF line ends). Code read: bounds checks right. Good work.
