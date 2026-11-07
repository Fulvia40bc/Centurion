# Save file format (*.CSV) — VERIFIED against mem_narbonensis_province_640k.bin

- File = 0x38-byte header + a verbatim copy of DS 0x5270 .. 0x5270+ (filesize-0x38).
  VERIFIED: 0.CSV[0x38:] aligns with DS:0x5270 in the snapshot (region matches byte-for-byte
  where the two saves agree); total save size 1780 = 0x38 + 0x6BC, so state area = DS 0x5270-0x58EF.
- Header (0x38 bytes): `"10A\0"` version, then player name NUL-terminated ("Falcon2", "erik"),
  rest zero.
- Province records: 32 bytes each starting at DS 0x5270 (prov index * 32).
  Fields used by the EXE (offsets relative to the 32-byte record), from func_0x6c26 / func_0x6f0c:
  - +0x04 u16  manpower (VERIFIED: Narbonensis 5279 = 0x14A7)
  - +0x0A u8   courage index (table DS:7486, value/0x19)
  - +0x0D u8   mood component A
  - +0x0E u8   ? (0x527e)
  - +0x10 u8   mood component B (0x5280); mood = (A + (B - A) + 0x527d...)/0x19 per func_0x6c26
  - +0x11 u8   tribute (0x5281)
  - +0x15 u8   status index (table DS:57B2: 1 Occupied, 2 Ally, 3 Colony, 4 Citizens)
  - BG/FG strip image indices live somewhere in the record (UNVERIFIED exact offset;
    Narbonensis = NEGBGA (MNEGO index 31) + NEGFGX (index 65))
- Other state (rank, talents, owner, legions, fleets, ...) in the same area — mapping UNVERIFIED.
