; ADLIB.LZW (CENT.DAT member, format 0x80 = u32 size + LZSS; 2426 bytes unpacked): the AdLib music driver of
;   Centurion: Defender of Rome (DOS, EA 1990). Credits (DS:6788): "Music Programming / Rob Hubbard".
; Claude, 2026-11-17. Decoded from the bytes (D); the sequencer model py-checked against the DOSBox RAM dumps (V, see
;   docs/MUSIC_NOTES.md 9). Addresses = offsets in the driver's own segment (loaded at seg:0000 by EXE 5EB5; in the
;   DOSBox runs seg = 3682, far pointer DS:00C4 = 3682:0000). All variables live in that segment (0x3E8: push cs / pop ds).
; Listing: llvm-objdump code16 (clang --target=i386-unknown-elf .incbin + llvm-objdump -M intel --adjust-vma=0x3E8) of
;   0x3E8..0x979 only; 0x000..0x3E7 is data and is listed as db/dw. Every jump/call target in 0x3E8..0x979 is an
;   instruction start (checked). OPL2 ports: 0x388 address, 0x389 data; delays 0x921 (12 loops) after an address write,
;   0x903 (32 loops) after a data write.
;
; Interface: the game calls `lcall [0xC4]` (= seg:0000) with AH = function, CX = argument:
;   AH=0 tick (0x4A9)          every PIT interrupt (EXE ISR 0823:0001, PIT counter 0x3FFF = 72.83 Hz), when [3EEC] != 0
;   AH=1 start song (0x403)    CX = segment of the unpacked CN*.LZW (EXE 072E, from 5D29)
;   AH=2 stop (0x470)          (EXE 074B, from 5D20) playing = 0, chip reset (everything silent at once)
;   AH=3 disable (0x483)       (ISR, when [3EE9] went to 0: Ctrl+S or exit) [3DD] = 0, key-off on all 9 channels
;   AH=4 enable (0x47B)        (ISR, when [3EE9] went to nonzero) [3DD] = 1
; Entries +3 (0x403) and +6/+9/+C/+F (0x470) of the jump table are never called by the game (it only uses +0).
; No range check on AH (0x3F6). The driver hooks no interrupt and never touches the PIT.

; ---- 0x000 jump table (near jmps): +0 -> 0x3E8 dispatcher, +3 -> 0x403 start, +6 +9 +C +F -> 0x470 stop
0000: jmp 0x3e8
0003: jmp 0x403
0006: jmp 0x470
0009: jmp 0x470
000c: jmp 0x470
000f: jmp 0x470
; ---- 0x012 dw song segment (ES of the tick; set by start)
0012: dw 0x0000
; ---- 0x014 frequency table, 96 words: index i = note + patch transpose -> (block << 10) | fnum, i.e. the low byte
;      goes to A0+ch and the high byte | keyon(0x20) to B0+ch. i = 12*block + semitone, fnum C..B =
;      158 16C 182 199 1B1 1CB 1E6 203 222 242 265 289 (i 48 = 261.0 Hz = C4 at 49716 Hz). Indexed as
;      [((i << 1) & 0xFF) + 0x14] (8-bit shl bl), so i >= 96 or i < 0 read the bytes after it (0x0D4..0x113).
0014: dw 0x0158, 0x016c, 0x0182, 0x0199, 0x01b1, 0x01cb, 0x01e6, 0x0203, 0x0222, 0x0242, 0x0265, 0x0289    ; block 0
002c: dw 0x0558, 0x056c, 0x0582, 0x0599, 0x05b1, 0x05cb, 0x05e6, 0x0603, 0x0622, 0x0642, 0x0665, 0x0689    ; block 1
0044: dw 0x0958, 0x096c, 0x0982, 0x0999, 0x09b1, 0x09cb, 0x09e6, 0x0a03, 0x0a22, 0x0a42, 0x0a65, 0x0a89    ; block 2
005c: dw 0x0d58, 0x0d6c, 0x0d82, 0x0d99, 0x0db1, 0x0dcb, 0x0de6, 0x0e03, 0x0e22, 0x0e42, 0x0e65, 0x0e89    ; block 3
0074: dw 0x1158, 0x116c, 0x1182, 0x1199, 0x11b1, 0x11cb, 0x11e6, 0x1203, 0x1222, 0x1242, 0x1265, 0x1289    ; block 4
008c: dw 0x1558, 0x156c, 0x1582, 0x1599, 0x15b1, 0x15cb, 0x15e6, 0x1603, 0x1622, 0x1642, 0x1665, 0x1689    ; block 5
00a4: dw 0x1958, 0x196c, 0x1982, 0x1999, 0x19b1, 0x19cb, 0x19e6, 0x1a03, 0x1a22, 0x1a42, 0x1a65, 0x1a89    ; block 6
00bc: dw 0x1d58, 0x1d6c, 0x1d82, 0x1d99, 0x1db1, 0x1dcb, 0x1de6, 0x1e03, 0x1e22, 0x1e42, 0x1e65, 0x1e89    ; block 7
00d4: dw 0x0000    ; i = 96 reads this: fnum 0, block 0 (silent)
; ---- 0x0D6 the 101 registers the reset 0x929 writes 0 (from 0x13A down to 0x0D6): 08, BD, 20-35, 60-75, 80-95, A0-A8,
;      B0-B8, C0-C8, E0-F5. BD = 0: no rhythm mode, AM depth 1 dB, vibrato depth 7 cents; 08 = 0: no CSM, NTS 0.
00d6: db 0x08, 0xbd, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x30, 0x31
00e6: db 0x32, 0x33, 0x34, 0x35, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d
00f6: db 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x88, 0x89, 0x8a, 0x8b
0106: db 0x8c, 0x8d, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7
0116: db 0xa8, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5
0126: db 0xc6, 0xc7, 0xc8, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xf0
0136: db 0xf1, 0xf2, 0xf3, 0xf4, 0xf5
; ---- 0x13B the 18 TL registers (40-55) the reset writes 0x3F (TL max = quietest)
013b: db 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x50, 0x51, 0x52, 0x53
014b: db 0x54, 0x55
; ---- 0x14D dw LFSR seed 0x563A of the random routine 0x90B (never called: dead code; the same word sits at
;      ROLAND.LZW 0x6C)
014d: dw 0x563a
; ---- 0x14F program -> patch index, 24 bytes (identity 0..23); MIDI programs > 23 read past it (no song does that)
014f: db 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17
; ---- 0x167 24 patch pointers. Patch record (12 bytes, 0x7FB): +0 20 mod, +1 40 mod, +2 60 mod, +3 80 mod, +4 20 car,
;      +5 40 car (also kept in [39B+ch]: the base of the velocity volume), +6 60 car, +7 80 car, +8 C0 (FB/CONN),
;      +9 transpose (signed, kept in [377+ch]), +10 E0 mod, +11 E0 car. Patches 1/2/5, 6/7 and 13/17 share records.
0167: dw 0x1f7    ; patch  0: E0 55 77 03 61 22 37 0F 0A F4 02 00  transpose -12
0169: dw 0x1d3    ; patch  1: 70 1C 71 03 20 1D 64 37 0E F4 02 02  transpose -12
016b: dw 0x1d3    ; patch  2: 70 1C 71 03 20 1D 64 37 0E F4 02 02  transpose -12
016d: dw 0x203    ; patch  3: 31 1C 51 03 61 9B 54 67 0E 00 00 00  transpose +0
016f: dw 0x1df    ; patch  4: 70 1C 71 03 20 1F 64 37 0E F4 02 02  transpose -12
0171: dw 0x1d3    ; patch  5: 70 1C 71 03 20 1D 64 37 0E F4 02 02  transpose -12
0173: dw 0x1bb    ; patch  6: 02 2B F5 75 21 9C F5 54 00 F4 00 02  transpose -12
0175: dw 0x1bb    ; patch  7: 02 2B F5 75 21 9C F5 54 00 F4 00 02  transpose -12
0177: dw 0x1af    ; patch  8: 23 2E F2 25 32 28 F2 F5 00 E8 00 01  transpose -24
0179: dw 0x29d    ; patch  9: F2 2B 41 1B E0 1D 52 1B 0C 00 00 00  transpose +0
017b: dw 0x265    ; patch 10: E2 11 AE 8F 61 28 85 2A 0E 00 00 00  transpose +0
017d: dw 0x197    ; patch 11: 32 9A 51 1B 61 A5 A2 3B 0C 00 00 00  transpose +0
017f: dw 0x1c7    ; patch 12: 31 16 6E 13 A3 21 6B 19 0A F4 01 02  transpose -12
0181: dw 0x2fd    ; patch 13: 31 1C 41 0B 61 9D 92 3B 0E F4 00 00  transpose -12
0183: dw 0x20f    ; patch 14: B1 1C 61 1F 61 A1 92 1B 0E F4 00 00  transpose -12
0185: dw 0x1a3    ; patch 15: 21 9C 73 5A 21 9F FA 1A 0C F4 00 00  transpose -12
0187: dw 0x27b    ; patch 16: 21 18 63 4C 21 9D C5 0F 0C F4 00 00  transpose -12
0189: dw 0x2fd    ; patch 17: 31 1C 41 0B 61 9D 92 3B 0E F4 00 00  transpose -12
018b: dw 0x2bf    ; patch 18: 07 53 DA 25 01 27 F9 15 0A 00 00 00  transpose +0
018d: dw 0x2b3    ; patch 19: 05 4E DC 25 01 29 FA 15 0A 0C 00 00  transpose +12
018f: dw 0x291    ; patch 20: 62 00 F0 F5 0E 23 F6 F8 0E DC 00 00  transpose -36
0191: dw 0x22f    ; patch 21: 07 4F F2 60 12 2A F2 72 08 00 00 00  transpose +0
0193: dw 0x245    ; patch 22: F2 4B 31 03 F4 21 53 2C 04 E8 00 00  transpose -24
0195: dw 0x1eb    ; patch 23: 00 03 F0 F0 00 1F F6 F7 0E 00 00 00  transpose +0
; ---- 0x197 patch data (0x197..0x308 reached by the pointers; the 195 bytes no pointer reaches, in 0x21B..0x349, look
;      like more patches of a bigger bank: unused)
0197: db 0x32, 0x9a, 0x51, 0x1b, 0x61, 0xa5, 0xa2, 0x3b, 0x0c, 0x00, 0x00, 0x00, 0x21, 0x9c, 0x73, 0x5a
01a7: db 0x21, 0x9f, 0xfa, 0x1a, 0x0c, 0xf4, 0x00, 0x00, 0x23, 0x2e, 0xf2, 0x25, 0x32, 0x28, 0xf2, 0xf5
01b7: db 0x00, 0xe8, 0x00, 0x01, 0x02, 0x2b, 0xf5, 0x75, 0x21, 0x9c, 0xf5, 0x54, 0x00, 0xf4, 0x00, 0x02
01c7: db 0x31, 0x16, 0x6e, 0x13, 0xa3, 0x21, 0x6b, 0x19, 0x0a, 0xf4, 0x01, 0x02, 0x70, 0x1c, 0x71, 0x03
01d7: db 0x20, 0x1d, 0x64, 0x37, 0x0e, 0xf4, 0x02, 0x02, 0x70, 0x1c, 0x71, 0x03, 0x20, 0x1f, 0x64, 0x37
01e7: db 0x0e, 0xf4, 0x02, 0x02, 0x00, 0x03, 0xf0, 0xf0, 0x00, 0x1f, 0xf6, 0xf7, 0x0e, 0x00, 0x00, 0x00
01f7: db 0xe0, 0x55, 0x77, 0x03, 0x61, 0x22, 0x37, 0x0f, 0x0a, 0xf4, 0x02, 0x00, 0x31, 0x1c, 0x51, 0x03
0207: db 0x61, 0x9b, 0x54, 0x67, 0x0e, 0x00, 0x00, 0x00, 0xb1, 0x1c, 0x61, 0x1f, 0x61, 0xa1, 0x92, 0x1b
0217: db 0x0e, 0xf4, 0x00, 0x00, 0xf0, 0x86, 0x61, 0x03, 0x71, 0x23, 0xe3, 0x2c, 0x0a, 0x00, 0x30, 0x15
0227: db 0xd5, 0x19, 0x71, 0x9b, 0x61, 0x1b, 0x0c, 0x00, 0x07, 0x4f, 0xf2, 0x60, 0x12, 0x2a, 0xf2, 0x72
0237: db 0x08, 0x00, 0x00, 0x00, 0x01, 0x40, 0xf4, 0x53, 0x08, 0x64, 0xf3, 0x53, 0x00, 0xee, 0xf2, 0x4b
0247: db 0x31, 0x03, 0xf4, 0x21, 0x53, 0x2c, 0x04, 0xe8, 0x00, 0x00, 0xe4, 0x8a, 0xff, 0x0f, 0xe1, 0xa9
0257: db 0xff, 0x0f, 0x01, 0xe8, 0x01, 0x4f, 0xf1, 0x50, 0x01, 0x22, 0xe4, 0x7c, 0x06, 0x00, 0xe2, 0x11
0267: db 0xae, 0x8f, 0x61, 0x28, 0x85, 0x2a, 0x0e, 0x00, 0x00, 0x00, 0xb0, 0x86, 0x61, 0x03, 0xf3, 0x1f
0277: db 0xe3, 0x2c, 0x08, 0x00, 0x21, 0x18, 0x63, 0x4c, 0x21, 0x9d, 0xc5, 0x0f, 0x0c, 0xf4, 0x00, 0x00
0287: db 0x10, 0x07, 0xff, 0x31, 0x01, 0x25, 0xff, 0x04, 0x0a, 0x00, 0x62, 0x00, 0xf0, 0xf5, 0x0e, 0x23
0297: db 0xf6, 0xf8, 0x0e, 0xdc, 0x00, 0x00, 0xf2, 0x2b, 0x41, 0x1b, 0xe0, 0x1d, 0x52, 0x1b, 0x0c, 0x00
02a7: db 0x00, 0x00, 0x32, 0x44, 0xf8, 0xff, 0x11, 0x1f, 0xf5, 0x7f, 0x0e, 0xe2, 0x05, 0x4e, 0xdc, 0x25
02b7: db 0x01, 0x29, 0xfa, 0x15, 0x0a, 0x0c, 0x00, 0x00, 0x07, 0x53, 0xda, 0x25, 0x01, 0x27, 0xf9, 0x15
02c7: db 0x0a, 0x00, 0x00, 0x00, 0x4e, 0x00, 0xfc, 0x05, 0x0c, 0x24, 0xf9, 0xf8, 0x00, 0x00, 0x6e, 0x00
02d7: db 0x75, 0x05, 0x01, 0x22, 0xf6, 0xf8, 0x0e, 0x00, 0x31, 0x90, 0xf1, 0x11, 0x18, 0xa5, 0xd4, 0x43
02e7: db 0x08, 0xe8, 0xe0, 0x8d, 0xf4, 0x1b, 0xe0, 0x20, 0xf3, 0x18, 0x0a, 0x00, 0xe0, 0xe8, 0xae, 0x8f
02f7: db 0x61, 0x21, 0x85, 0x0a, 0x0e, 0x00, 0x31, 0x1c, 0x41, 0x0b, 0x61, 0x9d, 0x92, 0x3b, 0x0e, 0xf4
0307: db 0x00, 0x00, 0xf2, 0x84, 0x62, 0x29, 0x32, 0xa2, 0x53, 0x2c, 0x0a, 0xe8, 0xf2, 0x16, 0x90, 0x29
0317: db 0xf2, 0x22, 0x61, 0x2c, 0x0a, 0xe8, 0xe0, 0x8d, 0xf4, 0x2b, 0xe0, 0x1f, 0xf3, 0x48, 0x08, 0x00
0327: db 0x31, 0x45, 0xf3, 0x23, 0x33, 0x25, 0x43, 0x9c, 0x06, 0xe8, 0x00, 0x00, 0xa7, 0x4c, 0x00, 0x25
0337: db 0xc6, 0x4f, 0x00, 0x00, 0x00, 0x00, 0xa7, 0x4c, 0x00, 0x2b, 0xc6, 0x4f, 0x00, 0xe8, 0x04, 0x00
0347: db 0xf7, 0xb5, 0x0c
; ---- 0x34A modulator operator offsets of channels 0..8 (20/40/60/80/E0 + off)
034a: db 0x00, 0x01, 0x02, 0x08, 0x09, 0x0a, 0x10, 0x11, 0x12
; ---- 0x353 carrier operator offsets of channels 0..8
0353: db 0x03, 0x04, 0x05, 0x0b, 0x0c, 0x0d, 0x13, 0x14, 0x15
; ---- 0x35C channel -> MIDI channel map (9 bytes, copied from song header +0x22 at start; this is the default)
035c: db 0x02, 0x02, 0x05, 0x01, 0x03, 0x02, 0x09, 0x09, 0x09
; ---- 0x365 per channel: 1 = note sounding (voice busy)
0365: db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
; ---- 0x36E per channel: MIDI note now on it (0 after its note-off)
036e: db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
; ---- 0x377 per channel: transpose of its patch (patch +9)
0377: db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
; ---- 0x380 per channel: the MIDI drum note a channel mapped to MIDI channel 9 answers to: ch5 0x36 (54), ch6 0x2A (42),
;      ch7 0x24 (36), ch8 0x26 (38). Other drum notes are not played.
0380: db 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x2a, 0x24, 0x26
; ---- 0x389 per channel: frequency-table index of that drum: ch5 0x49, ch6 0x17, ch7 0x0C, ch8 0x26
0389: db 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x17, 0x0c, 0x26
; ---- 0x392 per channel: drum patch loaded at start for channels mapped to MIDI 9: ch5 10, ch6 18, ch7 16, ch8 23
0392: db 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x12, 0x10, 0x17
; ---- 0x39B per channel: carrier 40 byte of the patch (KSL | TL); 0x3A4 shadow of A0; 0x3AD shadow of B0 (with keyon)
039b: db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
03a4: db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
03ad: db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
; ---- 0x3B6 9 words: age of each sounding channel in ticks (voice stealing takes the oldest)
03b6: db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
03c6: db 0x00, 0x00
; ---- 0x3C8 dw read pointer; 0x3CA dw loop start (= song +0); 0x3CC last status (high nibble); 0x3CD its MIDI channel;
;      0x3CE ticks still to wait; 0x3CF 1 = last status was a skipped one; 0x3D0 1 = it has 2 data bytes;
;      0x3D1 1 = playing; 0x3D2 keyon bit (0x20 or 0) for 0x73F; 0x3D3 dw channel chosen to steal (FFFF none);
;      0x3D5 dw pitch bend (14 bit); 0x3D7 B0 keyon|block bits during a bend; 0x3D8 zeroed by start, never read;
;      0x3DD 1 = enabled (static 1; AH=3 clears it, AH=4 sets it)
03c8: db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
03d8: db 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
; ---- 0x3DE AH table: 0 tick 0x4A9, 1 start 0x403, 2 stop 0x470, 3 disable 0x483, 4 enable 0x47B
03de: dw 0x4a9, 0x403, 0x470, 0x483, 0x47b

; ================================ code ================================

; ---- 0x3E8 dispatcher (jump table +0): call [AH*2 + 0x3DE] with DS = CS, SI = caller BX
03e8: push bx
03e9: push cx
03ea: push dx
03eb: push di
03ec: push si
03ed: push ds
03ee: push cs
03ef: pop ds
03f0: mov si, bx
03f2: xor bh, bh
03f4: mov bl, ah
03f6: shl bl
03f8: call word ptr [bx + 0x3de]
03fc: pop ds
03fd: pop si
03fe: pop di
03ff: pop dx
0400: pop cx
0401: pop bx
0402: retf

; ---- AH=1 0x403 start(CX = song segment)
0403: cli
0404: mov word ptr [0x12], cx
0408: push es
0409: mov es, cx
040b: mov ax, word ptr es:[0x0]           ; song +0: offset of the first event (0x7F in every song) = read pointer and loop start
040f: mov word ptr [0x3c8], ax
0412: mov word ptr [0x3ca], ax
0415: xor di, di
0417: mov al, byte ptr es:[di + 0x22]     ; song +0x22..+0x2A -> channel map 0x35C (9 channels)
041b: mov byte ptr [di + 0x35c], al
041f: inc di
0420: cmp di, 0x9
0423: jne 0x417
0425: pop es
0426: call 0x929                          ; chip reset
0429: xor di, di
042b: mov bx, di                          ; every channel: free, no note, age 0
042d: shl bx
042f: mov byte ptr [di + 0x365], 0x0
0434: mov byte ptr [di + 0x36e], 0x0
0439: mov word ptr [bx + 0x3b6], 0x0
043f: inc di
0440: cmp di, 0x9
0443: jne 0x42b
0445: xor di, di
0447: cmp byte ptr [di + 0x35c], 0x9      ; channels mapped to MIDI 9 get their fixed drum patch now
044c: jne 0x459
044e: mov bl, byte ptr [di + 0x392]
0452: mov cx, di
0454: push di
0455: call 0x7fb
0458: pop di
0459: inc di
045a: cmp di, 0x9
045d: jne 0x447
045f: mov byte ptr [0x3ce], 0x0           ; no wait, (0x3D8 = 0), playing
0464: mov byte ptr [0x3d8], 0x0
0469: mov byte ptr [0x3d1], 0x1
046e: sti
046f: ret

; ---- AH=2 0x470 stop: playing = 0 and chip reset (all TL 3F, keys off: silence at once)
0470: cli
0471: mov byte ptr [0x3d1], 0x0
0476: call 0x929
0479: sti
047a: ret

; ---- AH=4 0x47B enable
047b: cli
047c: mov byte ptr [0x3dd], 0x1
0481: sti
0482: ret

; ---- AH=3 0x483 disable: [3DD] = 0 and B0..B8 = 0 (keys off: the notes release with their RR)
0483: cli
0484: mov byte ptr [0x3dd], 0x0
0489: xor di, di
048b: xor cl, cl
048d: mov dx, 0x388
0490: mov al, -0x50
0492: add al, cl
0494: out dx, al
0495: call 0x921
0498: inc dx
0499: xor al, al
049b: out dx, al
049c: call 0x921
049f: inc cl
04a1: inc di
04a2: cmp di, 0x9
04a5: jne 0x48d
04a7: sti
04a8: ret

; ---- AH=0 0x4A9 tick
04a9: cmp byte ptr [0x3d1], 0x1
04ae: je 0x4b3
04b0: jmp 0x5a5
04b3: mov es, word ptr [0x12]
04b7: mov si, word ptr [0x3c8]
04bb: xor di, di
04bd: mov bx, di                          ; age + 1 for every sounding channel
04bf: shl bx
04c1: cmp byte ptr [di + 0x365], 0x1
04c6: jne 0x4cc
04c8: inc word ptr [bx + 0x3b6]
04cc: inc di
04cd: cmp di, 0x9
04d0: jne 0x4bd
04d2: cmp byte ptr [0x3ce], 0x0           ; still waiting: one tick less
04d7: je 0x4e0
04d9: dec byte ptr [0x3ce]
04dd: jmp 0x5a1
04e0: mov al, byte ptr es:[si]            ; next event
04e3: inc si
04e4: test al, al                         ; data byte: running status (no song uses it)
04e6: js 0x500
04e8: mov ah, byte ptr [0x3cc]
04ec: dec si
04ed: cmp byte ptr [0x3cf], 0x0
04f2: je 0x519
04f4: inc si
04f5: cmp byte ptr [0x3d0], 0x0
04fa: je 0x4fd
04fc: inc si
04fd: jmp 0x58e
0500: cmp al, -0x4                        ; FC = end of song
0502: jne 0x507
0504: jmp 0x5cc
0507: mov ah, al                          ; status: channel 0x3CD, type 0x3CC
0509: and ah, 0xf
050c: mov byte ptr [0x3cd], ah
0510: mov ah, al
0512: and ah, -0x10
0515: mov byte ptr [0x3cc], ah
0519: cmp ah, -0x20                       ; E0 pitch bend (no song uses it): 14-bit value lsb | msb << 7 -> 0x3D5, bend every sounding channel of the MIDI channel
051c: jne 0x558
051e: mov byte ptr [0x3d0], 0x1
0523: mov dl, byte ptr es:[si]
0526: inc si
0527: mov dh, byte ptr es:[si]
052a: inc si
052b: shr dh
052d: jae 0x532
052f: or dl, -0x80
0532: mov word ptr [0x3d5], dx
0536: mov al, byte ptr [0x3cd]
0539: xor di, di
053b: cmp al, byte ptr [di + 0x35c]
053f: jne 0x54f
0541: cmp byte ptr [di + 0x365], 0x1
0546: jne 0x54f
0548: mov cx, di
054a: push di
054b: call 0x7a3
054e: pop di
054f: inc di
0550: cmp di, 0x9
0553: jne 0x53b
0555: jmp 0x58e
0557: nop
0558: mov byte ptr [0x3cf], 0x0           ; C0 program change
055d: mov byte ptr [0x3d0], 0x1
0562: cmp ah, -0x40
0565: jne 0x56f
0567: mov byte ptr [0x3d0], 0x0
056c: jmp 0x5a6
056e: nop
056f: cmp ah, -0x70                       ; 90 note on
0572: jne 0x577
0574: jmp 0x5e2
0576: nop
0577: cmp ah, -0x80                       ; 80 note off
057a: jne 0x57f
057c: jmp 0x6ab
057f: inc si                              ; anything else (B0 controllers in the songs, A0, D0, F0..FB, FD..FF): 2 data bytes skipped
0580: inc si
0581: mov byte ptr [0x3d0], 0x1
0586: mov byte ptr [0x3cf], 0x1
058b: jmp 0x58e
058d: nop
058e: mov al, byte ptr es:[si]            ; delta byte: 0 = next event in the same tick, else wait delta - 1 more ticks (the next event comes delta ticks later)
0591: inc si
0592: cmp al, 0x0
0594: jne 0x599
0596: jmp 0x4e0
0599: dec al
059b: mov byte ptr [0x3ce], al
059e: jmp 0x5a1
05a0: nop
05a1: mov word ptr [0x3c8], si
05a5: ret

; ---- C0: load patch [byte] into every channel mapped to this MIDI channel (not for MIDI channel 9)
05a6: xor di, di
05a8: xor cx, cx
05aa: mov bl, byte ptr es:[si]
05ad: inc si
05ae: mov al, byte ptr [0x3cd]
05b1: cmp al, 0x9
05b3: je 0x5c4
05b5: cmp al, byte ptr [di + 0x35c]
05b9: jne 0x5c4
05bb: mov cx, di
05bd: push di
05be: push bx
05bf: call 0x7fb
05c2: pop bx
05c3: pop di
05c4: inc di
05c5: cmp di, 0x9
05c8: jne 0x5ae
05ca: jmp 0x58e

; ---- FC xx: xx = 0x80 loop to the start, anything else (the songs use 0x81) = stop playing
05cc: mov al, byte ptr es:[si]
05cf: inc si
05d0: cmp al, -0x80
05d2: je 0x5db
05d4: mov byte ptr [0x3d1], 0x0
05d9: jmp 0x5a5
05db: mov si, word ptr [0x3ca]
05df: jmp 0x4e0

; ---- 90 note velocity: velocity 0 = note off
05e2: mov bl, byte ptr es:[si + 0x1]
05e6: cmp bl, 0x0
05e9: jne 0x5ee
05eb: jmp 0x6ab
05ee: xor di, di
05f0: xor cx, cx
05f2: mov bl, byte ptr es:[si]
05f5: inc si
05f6: mov ah, byte ptr es:[si]
05f9: inc si
05fa: mov al, byte ptr [0x3cd]            ; MIDI channel 9 = drums (0x708)
05fd: cmp al, 0x9
05ff: jne 0x604
0601: jmp 0x708
0604: cmp al, byte ptr [di + 0x35c]       ; the first free channel mapped to this MIDI channel gets the note: keyon, freq = table[note + transpose]
0608: jne 0x635
060a: cmp byte ptr [di + 0x365], 0x1
060f: je 0x635
0611: mov cx, di
0613: push di
0614: mov byte ptr [0x3d2], 0x20
0619: mov byte ptr [di + 0x365], 0x1
061e: mov byte ptr [di + 0x36e], bl
0622: add bl, byte ptr [di + 0x377]
0626: shl bl
0628: xor bh, bh
062a: mov bx, word ptr [bx + 0x14]
062e: call 0x73f
0631: pop di
0632: jmp 0x58e
0635: inc di
0636: cmp di, 0x9
0639: jne 0x604
063b: push bx                             ; no free channel: steal the one with the biggest age (strictly > 0; all 0 = note dropped)
063c: xor di, di
063e: xor dx, dx
0640: mov word ptr [0x3d3], 0xffff
0646: mov bx, di
0648: shl bx
064a: cmp al, byte ptr [di + 0x35c]
064e: jne 0x65e
0650: mov cx, word ptr [bx + 0x3b6]
0654: cmp dx, cx
0656: jae 0x65e
0658: mov dx, cx
065a: mov word ptr [0x3d3], di
065e: inc di
065f: cmp di, 0x9
0662: jne 0x646
0664: cmp word ptr [0x3d3], -0x1
0669: jne 0x66f
066b: pop bx
066c: jmp 0x58e
066f: mov cx, word ptr [0x3d3]            ; key it off (A0 = B0 = 0) and on again with the new note
0673: mov di, cx
0675: mov bx, 0x0
0678: mov byte ptr [0x3d2], 0x0
067d: call 0x73f
0680: mov bx, di
0682: shl bx
0684: mov word ptr [bx + 0x3b6], 0x0
068a: pop bx
068b: mov byte ptr [0x3d2], 0x20
0690: mov byte ptr [di + 0x365], 0x1
0695: mov byte ptr [di + 0x36e], bl
0699: add bl, byte ptr [di + 0x377]
069d: shl bl
069f: xor bh, bh
06a1: mov bx, word ptr [bx + 0x14]
06a5: call 0x73f
06a8: jmp 0x58e

; ---- 80 / 90 v0 note off: the first channel of this MIDI channel holding this note: keyoff with the same freq
06ab: xor di, di
06ad: xor cx, cx
06af: mov bl, byte ptr es:[si]
06b2: inc si
06b3: inc si
06b4: mov al, byte ptr [0x3cd]
06b7: cmp al, byte ptr [di + 0x35c]
06bb: jne 0x6ff
06bd: cmp byte ptr [di + 0x36e], bl
06c1: jne 0x6ff
06c3: mov cx, di
06c5: push di
06c6: mov byte ptr [0x3d2], 0x0
06cb: mov byte ptr [di + 0x365], 0x0
06d0: cmp al, 0x9                         ; drums: their table index, normal notes: the note; then + transpose (also for the drums, unlike 0x708: latent, see notes)
06d2: jne 0x6db
06d4: mov bl, byte ptr [di + 0x389]
06d8: jmp 0x6df
06da: nop
06db: mov bl, byte ptr [di + 0x36e]
06df: mov byte ptr [di + 0x36e], 0x0
06e4: add bl, byte ptr [di + 0x377]
06e8: shl bl
06ea: xor bh, bh
06ec: mov bx, word ptr [bx + 0x14]
06f0: shl di
06f2: mov word ptr [di + 0x3b6], 0x0
06f8: call 0x73f
06fb: pop di
06fc: jmp 0x58e
06ff: inc di
0700: cmp di, 0x9
0703: jne 0x6b7
0705: jmp 0x58e

; ---- drum note on: the channel mapped to MIDI 9 whose drum note [380] is this note (retriggered even if busy)
0708: cmp al, byte ptr [di + 0x35c]
070c: jne 0x736
070e: cmp bl, byte ptr [di + 0x380]
0712: jne 0x736
0714: mov cx, di
0716: mov byte ptr [0x3d2], 0x20
071b: mov byte ptr [di + 0x365], 0x1
0720: mov byte ptr [di + 0x36e], bl
0724: mov bl, byte ptr [di + 0x389]       ; freq = table[[389]] (no transpose)
0728: shl bl
072a: xor bh, bh
072c: mov bx, word ptr [bx + 0x14]
0730: call 0x73f
0733: jmp 0x58e
0736: inc di
0737: cmp di, 0x9
073a: jne 0x708
073c: jmp 0x58e

; ---- 0x73F voice(CX = ch, BX = freq word, AH = velocity, [3D2] = keyon bit)
073f: cmp byte ptr [0x3dd], 0x1
0744: je 0x74b
0746: mov byte ptr [0x3d2], 0x0           ; disabled: never key on
074b: push si
074c: mov si, cx
074e: cmp byte ptr [0x3d2], 0x0
0753: je 0x771
0755: mov dx, 0x388                       ; key on: carrier 40 = [39B] - velocity / 4 (8-bit subtract: borrows into KSL when TL < vel/4: 1990 bug)
0758: mov al, byte ptr [si + 0x353]
075c: add al, 0x40
075e: out dx, al
075f: call 0x921
0762: inc dx
0763: mov al, byte ptr [si + 0x39b]
0767: shr ah
0769: shr ah
076b: sub al, ah
076d: out dx, al
076e: call 0x903
0771: mov dx, 0x388                       ; A0 = low byte, shadow 0x3A4
0774: mov al, -0x60
0776: add al, cl
0778: out dx, al
0779: call 0x921
077c: inc dx
077d: mov al, bl
077f: out dx, al
0780: mov byte ptr [si + 0x3a4], al
0784: call 0x903
0787: mov dx, 0x388                       ; B0 = high byte | keyon, shadow 0x3AD
078a: mov al, -0x50
078c: add al, cl
078e: out dx, al
078f: call 0x921
0792: inc dx
0793: mov al, bh
0795: or al, byte ptr [0x3d2]
0799: out dx, al
079a: mov byte ptr [si + 0x3ad], al
079e: call 0x921
07a1: pop si
07a2: ret

; ---- 0x7A3 bend(DI = ch): fnum = fnum * bend >> 13 (0x2000 = no change, range 0 .. 2x), block and keyon kept
07a3: push ax
07a4: mov al, byte ptr [di + 0x3a4]
07a8: mov ah, byte ptr [di + 0x3ad]
07ac: mov bl, ah
07ae: and bl, -0x4
07b1: mov byte ptr [0x3d7], bl
07b5: and ah, 0x3
07b8: mov dx, word ptr [0x3d5]
07bc: mul dx
07be: mov al, ah
07c0: mov ah, dl
07c2: mov dl, dh
07c4: push cx
07c5: mov cx, 0x5
07c8: shr dl
07ca: rcr ax
07cc: loop 0x7c8
07ce: pop cx
07cf: mov bx, ax
07d1: mov dx, 0x388
07d4: mov al, -0x60
07d6: add al, cl
07d8: out dx, al
07d9: call 0x921
07dc: inc dx
07dd: mov al, bl
07df: out dx, al
07e0: call 0x903
07e3: mov dx, 0x388
07e6: mov al, -0x50
07e8: add al, cl
07ea: out dx, al
07eb: call 0x921
07ee: inc dx
07ef: mov al, bh
07f1: or al, byte ptr [0x3d7]
07f5: out dx, al
07f6: call 0x921
07f9: pop ax
07fa: ret

; ---- 0x7FB patch(BL = program, CX = ch): 0x14F maps the program to a record, which is written to the operators
07fb: xor bh, bh
07fd: mov di, bx
07ff: mov bl, byte ptr [di + 0x14f]
0803: shl bl
0805: mov di, bx
0807: mov di, word ptr [di + 0x167]
080b: push si
080c: mov si, cx
080e: mov dx, 0x388
0811: mov al, byte ptr [si + 0x34a]
0815: add al, 0x20
0817: out dx, al
0818: call 0x921
081b: inc dx
081c: mov al, byte ptr [di]
081e: out dx, al
081f: call 0x903
0822: mov dx, 0x388
0825: mov al, byte ptr [si + 0x34a]
0829: add al, 0x40
082b: out dx, al
082c: call 0x921
082f: inc dx
0830: inc di
0831: mov al, byte ptr [di]
0833: out dx, al
0834: call 0x903
0837: mov dx, 0x388
083a: mov al, byte ptr [si + 0x34a]
083e: add al, 0x60
0840: out dx, al
0841: call 0x921
0844: inc dx
0845: inc di
0846: mov al, byte ptr [di]
0848: out dx, al
0849: call 0x903
084c: mov dx, 0x388
084f: mov al, byte ptr [si + 0x34a]
0853: add al, -0x80
0855: out dx, al
0856: call 0x921
0859: inc dx
085a: inc di
085b: mov al, byte ptr [di]
085d: out dx, al
085e: call 0x903
0861: mov dx, 0x388
0864: mov al, byte ptr [si + 0x353]
0868: add al, 0x20
086a: out dx, al
086b: call 0x921
086e: inc dx
086f: inc di
0870: mov al, byte ptr [di]
0872: out dx, al
0873: call 0x903
0876: mov dx, 0x388
0879: mov al, byte ptr [si + 0x353]
087d: add al, 0x40
087f: out dx, al
0880: call 0x921
0883: inc dx
0884: inc di
0885: mov al, byte ptr [di]
0887: mov byte ptr [si + 0x39b], al
088b: out dx, al
088c: call 0x903
088f: mov dx, 0x388
0892: mov al, byte ptr [si + 0x353]
0896: add al, 0x60
0898: out dx, al
0899: call 0x921
089c: inc dx
089d: inc di
089e: mov al, byte ptr [di]
08a0: out dx, al
08a1: call 0x903
08a4: mov dx, 0x388
08a7: mov al, byte ptr [si + 0x353]
08ab: add al, -0x80
08ad: out dx, al
08ae: call 0x921
08b1: inc dx
08b2: inc di
08b3: mov al, byte ptr [di]
08b5: out dx, al
08b6: call 0x903
08b9: mov dx, 0x388
08bc: mov al, -0x40
08be: add al, cl
08c0: out dx, al
08c1: call 0x921
08c4: inc dx
08c5: inc di
08c6: mov al, byte ptr [di]
08c8: out dx, al
08c9: call 0x903
08cc: inc di                              ; record +9: transpose -> 0x377
08cd: mov al, byte ptr [di]
08cf: push di
08d0: mov di, cx
08d2: mov byte ptr [di + 0x377], al
08d6: pop di
08d7: inc di
08d8: mov dx, 0x388
08db: mov al, byte ptr [si + 0x34a]
08df: add al, -0x20
08e1: out dx, al
08e2: call 0x921
08e5: inc dx
08e6: mov al, byte ptr [di]
08e8: out dx, al
08e9: call 0x903
08ec: inc di
08ed: mov dx, 0x388
08f0: mov al, byte ptr [si + 0x353]
08f4: add al, -0x20
08f6: out dx, al
08f7: call 0x921
08fa: inc dx
08fb: mov al, byte ptr [di]
08fd: out dx, al
08fe: call 0x903
0901: pop si
0902: ret
0903: mov al, 0x20                        ; delay after a data write
0905: dec al
0907: nop
0908: jne 0x905
090a: ret

; ---- 0x90B random(CX): LFSR on 0x14D (taps 0xB400), returns CX = value mod CX. Never called (dead code)
090b: push dx
090c: mov ax, word ptr [0x14d]
090f: shr ax
0911: jae 0x916
0913: xor ah, -0x4c
0916: mov word ptr [0x14d], ax
0919: xor dx, dx
091b: div cx
091d: mov cx, dx
091f: pop dx
0920: ret
0921: mov al, 0xc                         ; delay after an address write
0923: dec al
0925: nop
0926: jne 0x923
0928: ret

; ---- 0x929 chip reset: 01 = 0x20 (waveform select on), 04 = 0xE0 (timers off), the 101 registers of 0x0D6 = 0, TL 40-55 = 0x3F
0929: mov dx, 0x388
092c: mov al, 0x1
092e: out dx, al
092f: call 0x903
0932: inc dx
0933: mov al, 0x20
0935: out dx, al
0936: call 0x903
0939: mov dx, 0x388
093c: mov al, 0x4
093e: out dx, al
093f: call 0x903
0942: inc dx
0943: mov al, -0x20
0945: out dx, al
0946: call 0x903
0949: mov si, 0x64
094c: mov al, byte ptr [si + 0xd6]
0950: mov dx, 0x388
0953: out dx, al
0954: call 0x903
0957: mov al, 0x0
0959: inc dx
095a: out dx, al
095b: call 0x903
095e: dec si
095f: jns 0x94c
0961: mov si, 0x11
0964: mov al, byte ptr [si + 0x13b]
0968: mov dx, 0x388
096b: out dx, al
096c: call 0x903
096f: mov al, 0x3f
0971: inc dx
0972: out dx, al
0973: call 0x903
0976: dec si
0977: jns 0x964
0979: ret
