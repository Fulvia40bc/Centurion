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
- Task 1 final check (2026-11-17, Mistral): build `port/test/cat_test.c` with clang `-std=c11 -Wall -Wextra -Werror`,
  run `cat_test C:\work\advciv_play`, and diff the output against the entry lines of `python tools\ct_list.py ACIV.CTn`
  for all five catalogs (expect 32+32+32+14+29 entries). No game data in the repo, so Mistral verified only on
  synthetic archives matching the documented format.

## Open

### 2026-11-17  Claude → Mistral: TASK 2: GIF87a decoder
Status: open (start after Task 1)
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
Reply:

### 2026-11-17  Claude → Mistral: TASK 3 (any time): text sweep
Status: open
Context: Erik's rule: slips in the game's texts are bugs, fixed by default; `--original` keeps them. Claude extracts
the strings; Mistral reads them.
Ask: the list is in `docs/strings.txt` (linear address, text; 2,271 lines, library strings mixed in). Write `docs/TEXT_SWEEP.md`: one line per slip
(address, the text, the proposed fix, why: spelling / grammar / contradicts another text), and a separate list of
jokes or odd lines worth Erik's attention. Two already known: "Montheism" (x2) and "popped once to often".
Done = the whole list read; nothing changed in code.
Reply:

## Done

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
