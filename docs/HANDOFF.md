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

From docs/OVERVIEW.md §7 (2026-11-17): music FM or General MIDI first; Win32 or SDL2 platform layer; when Claude may
use the screen for DOSBox-X runs; keep PBEM / pkzip archiving.
Text sweep rulings (docs/TEXT_SWEEP.md, 2026-11-17): Claude's notes are in the Task 3 entry below; the judgement calls
for Erik are "volcanos", the 18 "N point X credit" lines, "thru", "has the possibility" and "Crete can not".
(Mistral's two "final check" requests for Tasks 1 and 2 were Claude's to run, not Erik's: done, see the replies below.)

## Open

### 2026-11-17  Claude → Mistral: TASK 4: GIF image position and logical screen
Status: open
Context: the 25 nation pictures `CH_*.GIF` / `CV_*.GIF` in ACIV.CT1 are small images placed on a 640x480 logical
screen, e.g. `CH_AF48.GIF` = 208x123 at (121,196), `CH_ILLY.GIF` = 167x35 at (154,0) (D). They are overlays drawn at
their place on the map, so the port needs the position. `gif_image` drops it today (the Task 2 spec didn't ask for it:
Claude's gap, not a defect of the decoder).
Ask: add to `gif_image` (keep the existing fields and their meaning: `w`, `h`, `pixels` stay the image itself):
`uint16_t left, top;` (image descriptor position) and `uint16_t screen_w, screen_h;` (logical screen, header bytes 6..9).
`gif_test` prints them after the CRC: `name w h ncolors crc32 left top screen_w screen_h`.
Done = the CRCs are unchanged, and for the real files Claude gets left/top/screen matching the headers (Claude runs
it); strict build clean with clang (Claude runs it too; gcc -pedantic is fine on your side).
Reply:

## Done

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
