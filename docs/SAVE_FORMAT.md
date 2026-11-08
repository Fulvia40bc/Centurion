# Save file format (*.CSV) — VERIFIED against mem_narbonensis_province_640k.bin

- File = 0x38-byte header + a verbatim copy of DS 0x526C .. (filesize-0x38).
  VERIFIED (Claude vs 3.CSV + snapshot): CSV[0x38:] matches DS:0x526C 100% for the first 0x300+
  bytes; save size 1780 = 0x38 + 0x6BC, so state area = DS 0x526C-0x5927. The tail past CSV 0x380
  is a different block (UNVERIFIED).
- Header (0x38 bytes): `"10A\0"` version, then player name NUL-terminated ("Falcon2", "erik"),
  rest zero.
- Province records: 32 bytes each starting at CSV 0x3C = DS 0x5270 (i.e. CSV offset
  `0x3C + idx*32 + field`). VERIFIED: Narbonensis = record 24 (manpower 5279 = 0x149F at
  CSV 0x340 = DS:5574); Rome = record 0 in 0.CSV (manpower 4914, tribute 8, status 4).
  Fields used by the EXE (offsets relative to the 32-byte record), from func_0x6c26 / func_0x6f0c:
  - +0x04 u16  manpower (VERIFIED: Narbonensis 5279 = 0x149F)
  - +0x0A u8   courage index (table DS:7486, value/0x19)
  - +0x0D u8   mood component A
  - +0x0E u8   ? (0x527e)
  - +0x10 u8   mood component B (0x5280); mood = (A + (B - A) + 0x527d...)/0x19 per func_0x6c26
  - +0x11 u8   tribute (0x5281)
  - +0x15 u8   status index (table DS:57B2: 1 Occupied, 2 Ally, 3 Colony, 4 Citizens)
  - BG/FG strip image indices live somewhere in the record (UNVERIFIED exact offset;
    Narbonensis = NEGBGA (MNEGO index 31) + NEGFGX (index 65))
- Other state (rank, talents, owner, legions, fleets, ...) in the same area — mapping UNVERIFIED.

## Units (map icons)
- The icon-draw code (EXE 0x2b04 / 0x382a) iterates a unit array at DS 0x1114..0x115E, 12-byte records
  (0xFF-padded = empty). **The fleet array IS in the save: CSV 0x48E-0x4D5 = DS 0x1114-0x115B byte for
  byte** (VERIFIED vs 3.CSV): 6 records x 12 bytes, byte 0 = zone (fleets: 30, 27, 28), bytes 1/3/4 =
  id/strength (UNVERIFIED beyond byte 0). Legion records are in the save tail (CSV 0x380-0x6F3,
  UNVERIFIED which block).
- Year: runtime global DS:0x717C (i16, -275 = 275 BC). In-save storage UNVERIFIED (turn-1 saves only;
  probably a turn counter).
