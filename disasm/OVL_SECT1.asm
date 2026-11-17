; OVL0 section 1 (file 0x0000-0x09A0, 0x9A0 bytes) = the start-up, the intro and the credits, loaded at 1331:0000 =
;   0823:B0E0-0xBA80. Bounds from the PLINK86 overlay table at 1322:0000 (= 0823:AFF0 in disasm/EXE_RUNTIME.bin):
;   section 1 load segment 1331, end segment 13CB, file position 0. Claude, 2026-11-17. Notes: docs/INTRO_NOTES.md.
;   Three entries (EXE thunks: lcall 129B:02EC / dw 1 / jmp 0823:entry):
;     B086 -> B0E0  command line (called from main 0x079D at 07B4)
;     B07A -> B18A  start-up + copy-protection question + intro (called from main at 07C2; 0 -> exit_game(1))
;     B0CE -> B9A1  end credits (called from section 4 DF70, the victory)
;   docs/LOGIC_NOTES.md has B07A/B086 the wrong way round; GAMELOOP_NOTES section 0 is right (D, from the code below).
; This listing replaces two older ones of the same bytes: disasm/OVL_RESIDENT.clean.asm and the first part of
;   disasm/OVL_MAP.clean.asm are section 1 at origin 0 (their 0x0000 = B0E0 here; "resident overlay 0x0146" in
;   map.c = B226), with wrong call targets (UNVERIFIED speculative decodes); disasm/OVL_CREDITS.asm is the credits
;   part B8B7..BA74 only (credits.c cites it); its comments are merged below, the instructions are the same.
; llvm-objdump code16 listing (clang --target=i386-unknown-elf .incbin + llvm-objdump -M intel --adjust-vma), addresses are
;   0823 offsets. Disassembled one function at a time from every entry, every push bp/mov bp,sp prologue and every
;   near-call target (B356, B8B7, B8CF, B8FE, B970 have no prologue; B9A1 is an entry without one). Checks (gen1.py):
;   the 12 functions tile 0xB0E0-0xBA75 with one zero byte at 0xB465 (listed as db); every function ends in ret/ret n;
;   every jump/call target inside the section is an instruction start. No jump table, no data island, no lcall.
;   Bytes 0xBA75-0xBA7F are the section's zero padding.
; Notation: absolute DS operands are printed unsigned ([0x9b26], llvm prints [-0x64da]); call targets are masked to
;   16 bits (llvm prints 0x10ee7 for EXE 0EE7). llvm's code16 "cwde" = cbw (98h), "cdq" = cwd (99h).
;   Comments: EXE routine names (from the other notes; (G) = guessed), DS variable names, "tbl DS:xxxx" for reg+disp
;   table bases, and the static DS string (OVL0.OVL 0xB400 = DS:0000) an immediate points at. Arguments: the callee
;   pops (ret n) and the FIRST pushed is the FIRST parameter; the C runtime (9DF2 .. A200) is cdecl (pop bx after).
; Timer: EXE 0062 sets PIT counter 0 to 3FFFh, so DS:42/44 count at 72.8 Hz (the BIOS clock is chained every 4th).

; ---- func B0E0  size 0xaa  bp frame, locals 6, ret 0x4
; B0E0(argc, argv) (Pascal: main pushes argc first; [bp+6] argc, [bp+4] argv): ENTRY (thunk B086, called from main 07B4): the command line. Sound default by hardware, then switches (case-sensitive): "k" no mouse; video x m e tandy v -> [9AB2] 0 2 4 6 8; sound nosnd pc tn roland mt lapc adlib ad -> [3EEA]
b0e0: push bp
b0e1: mov bp, sp
b0e3: sub sp, 0x6
b0e6: push di
b0e7: push si
b0e8: cmp word ptr [0x9ab2], 0x6               ; DS:9AB2 video adapter code; adapter 6 (Tandy) -> Tandy sound 14h
b0ed: jne 0xb0f4
b0ef: mov byte ptr [0x3eea], 0x14              ; DS:3EEA sound driver (11 speaker, 12 AdLib, 13 Roland, 14 Tandy, FF none)
b0f4: call 0x055c                              ; EXE 055C adlib_detect (ports 388/389): -1 = AdLib; AdLib found -> 12h
b0f7: or ax, ax
b0f9: je 0xb100
b0fb: mov byte ptr [0x3eea], 0x12              ; DS:3EEA sound driver (11 speaker, 12 AdLib, 13 Roland, 14 Tandy, FF none)
b100: cmp word ptr [bp + 0x6], 0x1             ; argc > 1
b104: jle 0xb182
b106: mov word ptr [bp - 0x6], 0x1
b10b: mov ax, word ptr [bp + 0x4]              ; argv + 1
b10e: inc ax
b10f: inc ax
b110: mov word ptr [bp - 0x4], ax
b113: mov ax, word ptr [bp + 0x6]
b116: cmp word ptr [bp - 0x6], ax
b119: jge 0xb182
b11b: mov ax, 0x6302                           ; DS:6302 "k" (no mouse)
b11e: push ax
b11f: mov bx, word ptr [bp - 0x4]
b122: push word ptr [bx]
b124: call 0xa186                              ; EXE A186 strcmp; strcmp(argv[i], "k") == 0 -> no mouse
b127: pop bx
b128: pop bx
b129: or ax, ax
b12b: jne 0xb130
b12d: mov word ptr [0xba], ax                  ; DS:00BA mouse present
b130: sub si, si
b132: push word ptr [si + 0x6304]              ; tbl DS:6304 video switch names; five video switches x m e tandy v
b136: mov bx, word ptr [bp - 0x4]
b139: push word ptr [bx]
b13b: call 0xa186                              ; EXE A186 strcmp
b13e: pop bx
b13f: pop bx
b140: or ax, ax
b142: jne 0xb14b
b144: mov ax, word ptr [si + 0x630e]           ; tbl DS:630E video switch values; [9AB2] = 0, 2, 4, 6, 8
b148: mov word ptr [0x9ab2], ax                ; DS:9AB2 video adapter code
b14b: inc si
b14c: inc si
b14d: cmp si, 0xa
b150: jl 0xb132
b152: sub si, si
b154: mov di, 0x6318                           ; DS:6318 sound switch names
b157: jmp 0xb173
b159: push word ptr [di]                       ; eight sound switches
b15b: mov bx, word ptr [bp - 0x4]
b15e: push word ptr [bx]
b160: call 0xa186                              ; EXE A186 strcmp
b163: pop bx
b164: pop bx
b165: or ax, ax
b167: jne 0xb170
b169: mov al, byte ptr [si + 0x6328]           ; tbl DS:6328 sound switch driver bytes; [3EEA] = FF, 11, 14, 13, 13, 13, 12, 12
b16d: mov byte ptr [0x3eea], al                ; DS:3EEA sound driver (11 speaker, 12 AdLib, 13 Roland, 14 Tandy, FF none)
b170: inc di
b171: inc di
b172: inc si
b173: cmp di, 0x6328                           ; DS:6328 end of sound switch names
b177: jb 0xb159
b179: add word ptr [bp - 0x4], 0x2
b17d: inc word ptr [bp - 0x6]
b180: jmp 0xb113
b182: pop si
b183: pop di
b184: mov sp, bp
b186: pop bp
b187: ret 0x4

; ---- func B18A  size 0x1cc  bp frame, locals 24, ret
; B18A(): ENTRY (thunk B07A, called from main 07C2): the start-up. ISRs, font, B466 question, palette, mmenu, music cn1, B694 intro, INIT/INIT2 sheets, cursor, buffers, map picture, B397(3..5). 0 -> exit_game(1)
b18a: push bp
b18b: mov bp, sp
b18d: sub sp, 0x18
b190: push di
b191: push si
b192: mov word ptr [bp - 0x8], 0xffff          ; [bp-8] ok = -1
b197: call 0x0302                              ; EXE 0302 int 24h (critical error) handler install
b19a: call 0x0062                              ; EXE 0062 timer_install: int 08 -> 0823:0001, old one -> int 64h, PIT counter 0 = 3FFFh (72.8 Hz)
b19d: call 0x03d3                              ; EXE 03D3 int 09 keyboard hook install
b1a0: mov ax, 0x9b56                           ; DS:9B56 font record
b1a3: mov di, ax
b1a5: mov si, 0xa05c                           ; DS:A05C font record source
b1a8: push ds
b1a9: pop es
b1aa: mov cx, 0x5
b1ad: rep movsw word ptr es:[di], word ptr [si] ; font record DS:9B56 <- DS:A05C (5 words)
b1af: call 0x680f                              ; EXE 680F target = the screen
b1b2: call 0xb466                              ; B466 copy-protection question
b1b5: or ax, ax
b1b7: jne 0xb1c0
b1b9: mov ax, 0x2
b1bc: push ax
b1bd: call 0x0ee7                              ; EXE 0EE7 exit_game(code); wrong: exit_game(2)
b1c0: mov word ptr [0x9afa], 0x0               ; DS:9AFA fill colour; [9AFA] = 0
b1c6: sub ax, ax
b1c8: push ax
b1c9: push ax
b1ca: mov cx, 0x13f
b1cd: push cx
b1ce: mov cx, 0xc7
b1d1: push cx
b1d2: call 0x7e74                              ; EXE 7E74 fill rect (x0, y0, x1, y1) in [9AFA]; fill(0, 0, 319, 199) black
b1d5: cmp word ptr [0xba], 0x0                 ; DS:00BA mouse present
b1da: je 0xb1ee
b1dc: call 0x0632                              ; EXE 0632 mouse_reset (int 33h ax 0 if a handler is installed): -1 = mouse
b1df: or ax, ax
b1e1: jne 0xb1e8
b1e3: mov word ptr [0xba], ax                  ; DS:00BA mouse present
b1e6: jmp 0xb1ee
b1e8: mov word ptr [0x3e40], 0x3e2e            ; DS:3E40 continue prompt ptr; DS:3E2E "Click to continue"; [3E40] = "Click to continue" (else "Hit any key to continue")
b1ee: call 0xb356                              ; B356 work buffer
b1f1: mov ax, 0x3
b1f4: push ax
b1f5: lea ax, [bp - 0x10]
b1f8: push ax
b1f9: call 0x0f7b                              ; EXE 0F7B archive_open(n, &arch): n -> name DS:029E[n] + ".DIR"; archive 3 = cent
b1fc: mov word ptr [bp - 0x8], ax
b1ff: or ax, ax
b201: je 0xb265
b203: cmp word ptr [0x9b26], 0x2               ; DS:9B26 graphics mode (2 = VGA)
b208: jne 0xb23f
b20a: mov ax, 0x6330                           ; DS:6330 "cmap.bin"
b20d: push ax
b20e: lea ax, [bp - 0x10]
b211: push ax
b212: lea ax, [bp - 0x18]
b215: push ax
b216: call 0x11c4                              ; EXE 11C4 member_load(name, &arch, &ptr); "cmap.bin"
b219: push word ptr [bp - 0x18]
b21c: call 0x9772                              ; EXE 9772 set_dac_block(buf): 256 entries (int 10h ax 1012h); the VGA palette
b21f: lea ax, [bp - 0x18]
b222: push ax
b223: call 0x198f                              ; EXE 198F free the member buffer (G)
b226: mov word ptr [0x6344], 0x74              ; DS:6344 panel colour; VGA colours [6344] 74h [6346] 1Ah [6348] 10Ch
b22c: mov word ptr [0x6346], 0x1a              ; DS:6346 box fill colour
b232: mov word ptr [0x6348], 0x10c             ; DS:6348 title colour
b238: mov byte ptr [0x636c], 0x6d              ; DS:636C "xmenu" first letter; "mmenu"
b23d: jmp 0xb244
b23f: mov byte ptr [0x636c], 0x65              ; DS:636C "xmenu" first letter; "emenu"
b244: mov ax, 0x636c                           ; DS:636C "xmenu" (end of the table)
b247: push ax
b248: lea ax, [bp - 0x10]
b24b: push ax
b24c: sub cx, cx
b24e: push cx
b24f: call 0x163b                              ; EXE 163B sheet_load(name, &arch, 0): xxxHDR/xxxDAT, returns the first 14-byte descriptor; sheet "mmenu" / "emenu" -> [C0F6]
b252: mov word ptr [0xc0f6], ax                ; DS:C0F6 sheet "mmenu"
b255: call 0x5e86                              ; EXE 5E86 sound driver load ([3EEA] -> 5EB5 -> [00C6])
b258: sub al, al
b25a: push ax
b25b: call 0x5d73                              ; EXE 5D73 music_load(n): n -> DS:3EF0[n] name + ".lzw" in archive DS:3EF2[n]; music_load(0) = cn1 (CENT)
b25e: lea ax, [bp - 0x10]
b261: push ax
b262: call 0x11a9                              ; EXE 11A9 archive_close(&arch)
b265: call 0xb694                              ; B694 the intro (also when cent failed)
b268: sub al, al
b26a: push ax
b26b: call 0x5e1a                              ; EXE 5E1A music_unload(n); music_unload(0)
b26e: mov ax, 0x4
b271: push ax
b272: lea ax, [bp - 0x10]
b275: push ax
b276: call 0x0f7b                              ; EXE 0F7B archive_open(n, &arch): n -> name DS:029E[n] + ".DIR"; archive 4 = init
b279: and word ptr [bp - 0x8], ax
b27c: je 0xb2cf
b27e: mov ax, 0x6339                           ; DS:6339 "init"
b281: push ax
b282: lea ax, [bp - 0x10]
b285: push ax
b286: sub ax, ax
b288: push ax
b289: call 0x163b                              ; EXE 163B sheet_load(name, &arch, 0): xxxHDR/xxxDAT, returns the first 14-byte descriptor; sheet "init"
b28c: mov word ptr [bp - 0xa], ax
b28f: mov word ptr [bp - 0x6], 0x0
b294: mov word ptr [bp - 0x4], 0x6350          ; DS:6350 INIT descriptor pointer table (14)
b299: jmp 0xb2b0
b29b: mov ax, word ptr [bp - 0xa]
b29e: add ax, word ptr [bp - 0x6]
b2a1: mov bx, word ptr [bp - 0x4]
b2a4: mov bx, word ptr [bx]
b2a6: mov word ptr [bx], ax                    ; W(W(6350+2k)) = first + 14k: the 14 INIT sprites
b2a8: add word ptr [bp - 0x6], 0xe
b2ac: add word ptr [bp - 0x4], 0x2
b2b0: cmp word ptr [bp - 0x4], 0x636c          ; DS:636C "xmenu" (end of the table)
b2b5: jb 0xb29b
b2b7: mov ax, 0x633e                           ; DS:633E "init2"
b2ba: push ax
b2bb: lea ax, [bp - 0x10]
b2be: push ax
b2bf: sub cx, cx
b2c1: push cx
b2c2: call 0x163b                              ; EXE 163B sheet_load(name, &arch, 0): xxxHDR/xxxDAT, returns the first 14-byte descriptor; sheet "init2" -> [C28A]
b2c5: mov word ptr [0xc28a], ax                ; DS:C28A sheet "init2"
b2c8: lea ax, [bp - 0x10]
b2cb: push ax
b2cc: call 0x11a9                              ; EXE 11A9 archive_close(&arch)
b2cf: call 0x1c7f                              ; EXE 1C7F cursor save-buffer setup: 0 = failed (G)
b2d2: and word ptr [bp - 0x8], ax
b2d5: mov ax, 0xa0
b2d8: push ax
b2d9: mov ax, 0x64
b2dc: push ax
b2dd: call 0x066a                              ; EXE 066A mouse_warp(x, y); mouse_warp(160, 100)
b2e0: push word ptr [0xc0e2]                   ; DS:C0E2 INIT sprite 0 (mouse cursor)
b2e4: call 0x1df5                              ; EXE 1DF5 cursor_sprite(desc) -> [0684]; cursor = INIT sprite 0
b2e7: mov ax, word ptr [0x9b26]                ; DS:9B26 graphics mode (2 = VGA)
b2ea: dec ax
b2eb: dec ax
b2ec: je 0xb2f8
b2ee: dec ax
b2ef: dec ax
b2f0: je 0xb2fd
b2f2: dec ax
b2f3: dec ax
b2f4: je 0xb316
b2f6: jmp 0xb320
b2f8: mov ax, 0xfa0                            ; mode 2 (VGA): 64000-byte buffer
b2fb: jmp 0xb319
b2fd: mov word ptr [0xc0c6], 0x0               ; DS:C0C6 second picture buffer segment; mode 4 (EGA): picture slots 0..4 get their numbers
b303: sub dx, dx
b305: mov bx, 0x252                            ; DS:0252 picture slot 0 +2
b308: cmp bx, 0x27a                            ; DS:027A picture slot 5 +2
b30c: jae 0xb320
b30e: mov byte ptr [bx], dl
b310: add bx, 0x8
b313: inc dx
b314: jmp 0xb308
b316: mov ax, 0x7d0                            ; mode 6 (Tandy): 32000-byte buffer
b319: push ax
b31a: call 0x863e                              ; EXE 863E alloc(paragraphs) -> segment (G)
b31d: mov word ptr [0xc0c6], ax                ; DS:C0C6 second picture buffer segment
b320: mov ax, 0x5
b323: push ax
b324: lea ax, [bp - 0x10]
b327: push ax
b328: call 0x0f7b                              ; EXE 0F7B archive_open(n, &arch): n -> name DS:029E[n] + ".DIR"; archive 5 = map
b32b: or ax, ax
b32d: je 0xb340
b32f: sub ax, ax
b331: push ax
b332: lea ax, [bp - 0x10]
b335: push ax
b336: call 0x13a8                              ; EXE 13A8 load_picture(slot, &arch): picture slot DS:0250+8n; load_picture(0): the map picture
b339: lea ax, [bp - 0x10]
b33c: push ax
b33d: call 0x11a9                              ; EXE 11A9 archive_close(&arch)
b340: mov si, 0x3
b343: push si
b344: call 0xb397                              ; B397(3), B397(4), B397(5)
b347: inc si
b348: cmp si, 0x6
b34b: jl 0xb343
b34d: mov ax, word ptr [bp - 0x8]              ; result: 0 -> main exit_game(1)
b350: pop si
b351: pop di
b352: mov sp, bp
b354: pop bp
b355: ret

; ---- func B356  size 0x41  no bp frame, ret  callers: B18A
; B356(): the work buffer descriptor (C1DA..): VGA alloc 64000 bytes, EGA A000:2000
b356: push di
b357: push si
b358: mov ax, 0xc1da                           ; DS:C1DA work buffer descriptor
b35b: mov di, ax
b35d: mov si, 0x9ab6                           ; DS:9AB6 screen descriptor
b360: push ds
b361: pop es
b362: mov cx, 0x9
b365: rep movsw word ptr es:[di], word ptr [si] ; work buffer descriptor C1DA <- screen descriptor 9AB6 (9 words)
b367: mov ax, word ptr [0x9b26]                ; DS:9B26 graphics mode (2 = VGA)
b36a: dec ax
b36b: dec ax
b36c: je 0xb375
b36e: dec ax
b36f: dec ax
b370: je 0xb388
b372: pop si
b373: pop di
b374: ret
b375: mov ax, 0xfa0
b378: push ax
b379: call 0x863e                              ; EXE 863E alloc(paragraphs) -> segment (G); VGA: 64000-byte work buffer
b37c: mov word ptr [0xc1de], ax                ; DS:C1DE work buffer segment
b37f: mov word ptr [0xc1dc], 0x0               ; DS:C1DC work buffer offset
b385: pop si
b386: pop di
b387: ret
b388: mov word ptr [0xc1de], 0xa000            ; DS:C1DE work buffer segment; EGA: the work buffer is video memory A000:2000
b38e: mov word ptr [0xc1dc], 0x2000            ; DS:C1DC work buffer offset
b394: pop si
b395: pop di
b396: ret

; ---- func B397  size 0xcf  bp frame, locals 6, ret 0x2  callers: B18A
; B397(i): enemy fleet i (3..5) -> its marines, army record i + 8 (GAMELOOP_NOTES 1)
b397: push bp
b398: mov bp, sp
b39a: sub sp, 0x6
b39d: push di
b39e: push si
b39f: mov ax, 0xc
b3a2: mul word ptr [bp + 0x4]
b3a5: mov bx, ax
b3a7: mov di, word ptr [bp + 0x4]
b3aa: lea di, [di + 0x8]
b3ad: mov ax, di
b3af: mov byte ptr [bx + 0x1117], al           ; tbl DS:1117 fleet.army; f[3] = i + 8
b3b3: mov al, byte ptr [bx + 0x1115]           ; tbl DS:1115 fleet.home
b3b7: sub ah, ah
b3b9: mov cx, ax
b3bb: mov ax, di
b3bd: mov si, 0xe
b3c0: mul si
b3c2: mov si, ax
b3c4: mov word ptr [si - 0x3eec], cx           ; tbl DS:C114 army.+2 home; army.+2 = home
b3c8: mov al, byte ptr [bx + 0x1114]           ; tbl DS:1114 fleet.zone
b3cc: mov byte ptr [si - 0x3eed], al           ; tbl DS:C113 army.+1; army.+1 = f[0]
b3d0: mov byte ptr [si - 0x3ee4], 0x0          ; tbl DS:C11C army.+0x0A; army.+0x0A = 0
b3d5: mov ax, 0xe
b3d8: mul word ptr [bp + 0x4]
b3db: mov dx, bx
b3dd: mov bx, ax
b3df: mov word ptr [bx - 0x3e7a], 0x0          ; tbl DS:C186 army(i+8).+4; W(C186 + 14i) = army(i+8).+4 = 0
b3e5: mov bl, 0x5
b3e7: xchg cx, bx
b3e9: shl bx, cl
b3eb: mov al, byte ptr [bx + 0x527b]           ; tbl DS:527B province.+0x0B
b3ef: mov byte ptr [si - 0x3ee3], al           ; tbl DS:C11D army.+0x0B; army.+0x0B = province(home).+0x0B
b3f3: mov al, byte ptr [bx + 0x527a]           ; tbl DS:527A province.+0x0A
b3f7: mov byte ptr [si - 0x3ee2], al           ; tbl DS:C11E army.+0x0C; army.+0x0C = province(home).+0x0A
b3fb: mov word ptr [si - 0x3ee6], 0xffff       ; tbl DS:C11A army.+8; army.+8 = FFFF
b401: mov word ptr [bp - 0x2], 0x0
b406: add dx, 0x110e
b40a: mov word ptr [bp - 0x4], dx
b40d: mov word ptr [bp - 0x6], di
b410: jmp 0xb457
b412: mov bx, word ptr [bp - 0x4]
b415: mov di, word ptr [bx]
b417: or di, di
b419: je 0xb450
b41b: mov bx, word ptr [bp - 0x2]
b41e: mov al, byte ptr [bx - 0x7122]           ; tbl DS:8EDE men per ship type; men per ship of type j
b422: sub ah, ah
b424: mul di
b426: cmp ax, 0x1f40
b429: jbe 0xb42e
b42b: mov ax, 0x1f40                           ; at most 8000
b42e: mov cx, ax
b430: mov ax, 0xe
b433: mul word ptr [bp - 0x6]
b436: mov bx, ax
b438: mov word ptr [bx - 0x3ee8], cx           ; tbl DS:C118 army.+6 strength; army.+6 = strength
b43c: mov ax, cx
b43e: sub dx, dx
b440: div di
b442: mov cx, ax
b444: mov ax, 0xc
b447: mul word ptr [bp + 0x4]
b44a: mov bx, ax
b44c: mov byte ptr [bx + 0x1118], cl           ; tbl DS:1118 fleet.men per ship; f[4] = strength / ships
b450: add word ptr [bp - 0x4], 0x2
b454: inc word ptr [bp - 0x2]
b457: cmp word ptr [bp - 0x2], 0x3
b45b: jb 0xb412
b45d: pop si
b45e: pop di
b45f: mov sp, bp
b461: pop bp
b462: ret 0x2
b465: db 0x00                                  ; zero byte between functions (B466 starts on an even address)

; ---- func B466  size 0x22e  bp frame, locals 142, ret  callers: B18A
; B466(): the copy-protection question (PLACES.LZW, 3 tries); -1 = passed (then the archive names are decrypted), 0 = failed
b466: push bp
b467: mov bp, sp
b469: sub sp, 0x8e
b46d: push di
b46e: push si
b46f: mov word ptr [bp - 0xc], 0x0
b474: mov ax, 0x3
b477: push ax
b478: lea ax, [bp - 0x16]
b47b: push ax
b47c: call 0x0f7b                              ; EXE 0F7B archive_open(n, &arch): n -> name DS:029E[n] + ".DIR"; archive 3 = cent
b47f: or ax, ax
b481: jne 0xb486
b483: jmp 0xb68b
b486: mov ax, 0x63bf                           ; DS:63BF "places.lzw"
b489: push ax
b48a: lea ax, [bp - 0x16]
b48d: push ax
b48e: lea ax, [bp - 0x1e]
b491: push ax
b492: call 0x11c4                              ; EXE 11C4 member_load(name, &arch, &ptr); "places.lzw"
b495: or ax, ax
b497: jne 0xb49c
b499: jmp 0xb684
b49c: mov ax, word ptr [bp - 0x1e]
b49f: mov word ptr [bp - 0xa], ax
b4a2: sub ax, ax
b4a4: mov cx, 0x14
b4a7: lea di, [bp - 0x34]
b4aa: push ss
b4ab: pop es
b4ac: rep stosb byte ptr es:[di], al           ; answer buffer [bp-34h] cleared (20 bytes)
b4ae: mov ax, 0x42
b4b1: push ax
b4b2: call 0x076b                              ; EXE 076B rng_roll(n); rng_roll(66): one of the 66 PLACES records
b4b5: mov word ptr [bp - 0x4], ax
b4b8: mov cx, 0x14
b4bb: mul cx
b4bd: mov bx, ax
b4bf: add bx, word ptr [bp - 0xa]
b4c2: mov word ptr [bp - 0x8e], bx
b4c6: mov al, byte ptr [bx + 0x1]
b4c9: mov cx, ax
b4cb: and ax, 0xf
b4ce: mov word ptr [bp - 0x6], ax              ; type = rec[1] & 0Fh
b4d1: mov al, 0x4
b4d3: xchg cl, al
b4d5: shr al, cl
b4d7: sub ah, ah
b4d9: mov word ptr [bp - 0x8], ax              ; direction = rec[1] >> 4
b4dc: mov ax, 0x46
b4df: push ax
b4e0: push ax
b4e1: mov ax, 0xf0
b4e4: push ax
b4e5: mov ax, 0x82
b4e8: push ax
b4e9: push word ptr [0x6346]                   ; DS:6346 box fill colour
b4ed: call 0x3134                              ; EXE 3134 box(x0, y0, x1, y1, fill); box(70, 70, 240, 130, [6346]) (7 = grey here)
b4f0: mov ax, 0x63ca                           ; DS:63CA "What is the "
b4f3: push ax
b4f4: lea ax, [bp - 0x8c]
b4f8: push ax
b4f9: call 0x9e32                              ; EXE 9E32 strcpy
b4fc: pop bx
b4fd: pop bx
b4fe: mov bx, word ptr [bp - 0x8]
b501: shl bx
b503: push word ptr [bx + 0x642e]              ; tbl DS:642E direction words; direction word
b507: lea ax, [bp - 0x8c]
b50b: push ax
b50c: call 0x9df2                              ; EXE 9DF2 strcat
b50f: pop bx
b510: pop bx
b511: mov bx, word ptr [bp - 0x6]
b514: shl bx
b516: push word ptr [bx + 0x643c]              ; tbl DS:643C place-type words; type word
b51a: lea ax, [bp - 0x8c]
b51e: push ax
b51f: call 0x9df2                              ; EXE 9DF2 strcat
b522: pop bx
b523: pop bx
b524: mov ax, 0x63d7                           ; DS:63D7 " in the\nprovince of "
b527: push ax
b528: lea ax, [bp - 0x8c]
b52c: push ax
b52d: call 0x9df2                              ; EXE 9DF2 strcat
b530: pop bx
b531: pop bx
b532: mov bx, word ptr [bp - 0x8e]
b536: mov bl, byte ptr [bx]
b538: sub bh, bh
b53a: shl bx
b53c: push word ptr [bx + 0x5222]              ; tbl DS:5222 province names; province name of rec[0]
b540: lea ax, [bp - 0x8c]
b544: push ax
b545: call 0x9df2                              ; EXE 9DF2 strcat
b548: pop bx
b549: pop bx
b54a: mov ax, 0x63ec
b54d: push ax
b54e: lea ax, [bp - 0x8c]
b552: push ax
b553: call 0x9df2                              ; EXE 9DF2 strcat
b556: pop bx
b557: pop bx
b558: lea ax, [bp - 0x8c]
b55c: push ax
b55d: mov ax, 0x9b
b560: push ax
b561: mov ax, 0x50
b564: push ax
b565: call 0x8be4                              ; EXE 8BE4 centred_text(str, cx, y) in [9B69]; centred_text(question, 155, 80)
b568: mov word ptr [0x9b69], 0xf               ; DS:9B69 text colour; white
b56e: mov word ptr [bp - 0x10], 0x61           ; input at (97, 112)
b573: mov word ptr [bp - 0xe], 0x70
b578: mov word ptr [bp - 0x2], 0x0
b57d: cmp word ptr [bp - 0x2], 0x3             ; three tries
b581: jb 0xb586
b583: jmp 0xb67d
b586: mov word ptr [0x9afa], 0x8               ; DS:9AFA fill colour; [9AFA] = 8
b58c: mov ax, 0x5f
b58f: push ax
b590: mov ax, 0x6e
b593: push ax
b594: mov ax, 0xd7
b597: push ax
b598: mov al, byte ptr [0x9b5c]                ; DS:9B5C font line height
b59b: sub ah, ah
b59d: add ax, 0x72
b5a0: push ax
b5a1: call 0x7e74                              ; EXE 7E74 fill rect (x0, y0, x1, y1) in [9AFA]; the bar (95, 110)-(215, 122)
b5a4: sub ax, ax
b5a6: push ax
b5a7: lea cx, [bp - 0x34]
b5aa: push cx
b5ab: call 0xa1dc                              ; EXE A1DC strset(s, c); strset(answer, 0)
b5ae: pop bx
b5af: pop bx
b5b0: lea ax, [bp - 0x34]
b5b3: push ax
b5b4: lea ax, [bp - 0x10]
b5b7: push ax
b5b8: mov ax, 0x14
b5bb: push ax
b5bc: mov ax, 0x78
b5bf: push ax
b5c0: call 0x36ed                              ; EXE 36ED edit_line(buf, &xy, max, xmax): Enter -1, Esc 0; edit_line(answer, &(97, 112), 20, 120)
b5c3: or ax, ax
b5c5: je 0xb5f8                                ; Esc: a try used
b5c7: lea ax, [bp - 0x34]
b5ca: push ax
b5cb: call 0xa200                              ; EXE A200 strupr; strupr(answer)
b5ce: pop bx
b5cf: mov ax, word ptr [bp - 0x8e]
b5d3: inc ax
b5d4: inc ax
b5d5: push ax
b5d6: lea ax, [bp - 0x4a]
b5d9: push ax
b5da: call 0x9e32                              ; EXE 9E32 strcpy; strcpy(local, rec + 2)
b5dd: pop bx
b5de: pop bx
b5df: lea ax, [bp - 0x4a]
b5e2: push ax
b5e3: call 0xa200                              ; EXE A200 strupr; strupr(local)
b5e6: pop bx
b5e7: lea ax, [bp - 0x4a]
b5ea: push ax
b5eb: lea ax, [bp - 0x34]
b5ee: push ax
b5ef: call 0xa186                              ; EXE A186 strcmp; strcmp(local, answer)
b5f2: pop bx
b5f3: pop bx
b5f4: or ax, ax
b5f6: jmp 0xb5fe                               ; eb 06: ALWAYS right (the 1990 code must have had je = 74 06; a one-byte patch, G)
b5f8: inc word ptr [bp - 0x2]
b5fb: jmp 0xb57d
b5fe: mov word ptr [bp - 0xc], 0xffff          ; ok = -1
b603: sub si, si
b605: jmp 0xb60d
b607: xor byte ptr [si + 0x6444], 0x47         ; tbl DS:6444 name key; the key ",\"))>" ^ 47h = "kenny"
b60c: inc si
b60d: mov ax, 0x6444                           ; DS:6444 name key ",\"))>" (XOR 47h = "kenny")
b610: push ax
b611: call 0xa02c                              ; EXE A02C strlen
b614: pop bx
b615: cmp ax, si
b617: ja 0xb607
b619: sub di, di
b61b: sub si, si
b61d: mov word ptr [bp - 0x2], di
b620: jmp 0xb62f
b622: mov al, byte ptr [si + 0x6444]           ; tbl DS:6444 name key
b626: mov bx, di
b628: mov bx, word ptr [bx + di + 0x6420]      ; tbl DS:6420 protected archive names
b62c: xor byte ptr [bx + si], al               ; name[k] ^= key[k] for the 7 names
b62e: inc si
b62f: mov al, byte ptr [di + 0x644a]           ; tbl DS:644A name lengths
b633: sub ah, ah
b635: cmp ax, si
b637: ja 0xb622
b639: inc di
b63a: cmp di, 0x7
b63d: jb 0xb61b
b63f: mov ax, 0x6444                           ; DS:6444 name key ",\"))>" (XOR 47h = "kenny")
b642: push ax
b643: call 0xa02c                              ; EXE A02C strlen
b646: pop bx
b647: mov si, ax
b649: mov ax, 0xea
b64c: mov dx, 0x6444                           ; DS:6444 name key ",\"))>" (XOR 47h = "kenny")
b64f: mov cx, si
b651: mov di, dx
b653: push ds
b654: pop es
b655: rep stosb byte ptr es:[di], al           ; the key overwritten with EAh
b657: mov word ptr [bp - 0x4], 0x6420          ; DS:6420 protected archive names (7 ptrs)
b65c: cmp word ptr [bp - 0x4], 0x642c          ; DS:642C end of the 6 mode-lettered names
b661: jae 0xb67d
b663: cmp word ptr [0x9b26], 0x2               ; DS:9B26 graphics mode (2 = VGA)
b668: jne 0xb66e
b66a: mov al, 0x6d                             ; 'm' (VGA)
b66c: jmp 0xb670
b66e: mov al, 0x65                             ; 'e'
b670: mov bx, word ptr [bp - 0x4]
b673: mov bx, word ptr [bx]
b675: mov byte ptr [bx], al                    ; first letter of the 6 x-names
b677: add word ptr [bp - 0x4], 0x2
b67b: jmp 0xb65c
b67d: push word ptr [bp - 0xa]
b680: call 0xa174                              ; EXE A174 free; free(places)
b683: pop bx
b684: lea ax, [bp - 0x16]
b687: push ax
b688: call 0x11a9                              ; EXE 11A9 archive_close(&arch); archive_close
b68b: mov ax, word ptr [bp - 0xc]
b68e: pop si
b68f: pop di
b690: mov sp, bp
b692: pop bp
b693: ret

; ---- func B694  size 0x223  bp frame, locals 72, ret  callers: B18A
; B694(): the intro: romulus + music cn1, the legend, world + title, 11 credits, "It is 400 years after..."
b694: push bp
b695: mov bp, sp
b697: sub sp, 0x48
b69a: push di
b69b: push si
b69c: sub si, si
b69e: mov ax, 0x4
b6a1: push ax
b6a2: lea ax, [bp - 0xe]
b6a5: push ax
b6a6: call 0x0f7b                              ; EXE 0F7B archive_open(n, &arch): n -> name DS:029E[n] + ".DIR"; archive 4 = init
b6a9: mov di, ax
b6ab: or di, di
b6ad: je 0xb71d
b6af: mov ax, 0x6a17                           ; DS:6A17 "romulus.cbm"
b6b2: push ax
b6b3: lea ax, [bp - 0xe]
b6b6: push ax
b6b7: lea cx, [bp - 0x2c]
b6ba: push cx
b6bb: call 0x1273                              ; EXE 1273 cbm_load(name, &arch, &desc); romulus.cbm -> [bp-2Ch]
b6be: and di, ax
b6c0: mov ax, 0x6a23                           ; DS:6A23 "title.cbm"
b6c3: push ax
b6c4: lea ax, [bp - 0xe]
b6c7: push ax
b6c8: lea ax, [bp - 0x3a]
b6cb: push ax
b6cc: call 0x1273                              ; EXE 1273 cbm_load(name, &arch, &desc); title.cbm -> [bp-3Ah]
b6cf: and di, ax
b6d1: call 0x67e4                              ; EXE 67E4 target = the work buffer; draw into the work buffer
b6d4: cmp word ptr [0x9b26], 0x2               ; DS:9B26 graphics mode (2 = VGA)
b6d9: jne 0xb6f0
b6db: mov ax, 0x6a2d                           ; DS:6A2D "world.lzw"
b6de: push ax
b6df: lea ax, [bp - 0xe]
b6e2: push ax
b6e3: push word ptr [0xc1de]                   ; DS:C1DE work buffer segment
b6e7: push word ptr [0xc1dc]                   ; DS:C1DC work buffer offset
b6eb: call 0x0f0c                              ; EXE 0F0C load member into a far buffer (name, &arch, seg, off); VGA: world.lzw unpacked straight into the work buffer
b6ee: jmp 0xb713
b6f0: mov ax, 0x6a37                           ; DS:6A37 "world.cbm"
b6f3: push ax
b6f4: lea ax, [bp - 0xe]
b6f7: push ax
b6f8: lea ax, [bp - 0x48]
b6fb: push ax
b6fc: call 0x1273                              ; EXE 1273 cbm_load(name, &arch, &desc); EGA: world.cbm
b6ff: and di, ax
b701: lea ax, [bp - 0x48]
b704: push ax
b705: sub cx, cx
b707: push cx
b708: push cx
b709: call 0x8e62                              ; EXE 8E62 sprite(desc, x, y): drawn at (x - xhot, y - yhot); world drawn into the work buffer
b70c: lea ax, [bp - 0x48]
b70f: push ax
b710: call 0x86a0                              ; EXE 86A0 free CBM descriptor; free world
b713: lea ax, [bp - 0xe]
b716: push ax
b717: call 0x11a9                              ; EXE 11A9 archive_close(&arch)
b71a: call 0x680f                              ; EXE 680F target = the screen; target = the screen
b71d: or di, di
b71f: jne 0xb72f
b721: mov ax, 0x6a41                           ; DS:6A41 "Could Not Open Data File."
b724: push ax
b725: call 0x59d4                              ; EXE 59D4 message_box(str); "Could Not Open Data File."
b728: mov ax, 0x1
b72b: push ax
b72c: call 0x0ee7                              ; EXE 0EE7 exit_game(code); exit_game(1)
b72f: call 0x5d20                              ; EXE 5D20 music_stop; music_stop
b732: mov word ptr [0x9afa], 0x0               ; DS:9AFA fill colour
b738: sub ax, ax
b73a: push ax
b73b: push ax
b73c: mov cx, 0x13f
b73f: push cx
b740: mov cx, 0xc7
b743: push cx
b744: call 0x7e74                              ; EXE 7E74 fill rect (x0, y0, x1, y1) in [9AFA]; black screen
b747: lea ax, [bp - 0x2c]
b74a: push ax
b74b: sub cx, cx
b74d: push cx
b74e: push cx
b74f: call 0x8e62                              ; EXE 8E62 sprite(desc, x, y): drawn at (x - xhot, y - yhot); romulus at (0, 0) on the screen
b752: lea ax, [bp - 0x2c]
b755: push ax
b756: call 0x86a0                              ; EXE 86A0 free CBM descriptor; free romulus
b759: call 0x00f8                              ; EXE 00F8 timer0_reset (= 00D7(0)); timer 0 = 0
b75c: sub al, al
b75e: push ax
b75f: call 0x5d29                              ; EXE 5D29 music_play(n); music_play(0): cn1
b762: jmp 0xb776
b764: call 0x01ae                              ; EXE 01AE ticks_since(0)
b767: cmp ax, 0x1f8                            ; romulus until timer 0 = 504 (6.9 s)
b76a: jae 0xb77a
b76c: call 0x03aa                              ; EXE 03AA key_read: next key or 0; only the keyboard skips
b76f: or al, al
b771: je 0xb776
b773: mov si, 0xffff
b776: or si, si
b778: je 0xb764
b77a: mov word ptr [bp - 0x2], si
b77d: or si, si                                ; no key: the story
b77f: je 0xb7c0
b781: or si, si
b783: je 0xb788
b785: jmp 0xb83b
b788: sub ax, ax
b78a: push ax
b78b: push ax
b78c: mov cx, 0x13f
b78f: push cx
b790: mov cx, 0xc7
b793: push cx
b794: call 0x6973                              ; EXE 6973 restore rect work buffer -> screen (x0, y0, x1, y1); the world picture on the screen
b797: lea ax, [bp - 0x3a]
b79a: push ax
b79b: sub ax, ax
b79d: push ax
b79e: mov ax, 0x40
b7a1: push ax
b7a2: call 0x8e62                              ; EXE 8E62 sprite(desc, x, y): drawn at (x - xhot, y - yhot); title at (0, 64): hot (-4, -10) -> (4, 74)
b7a5: or si, si
b7a7: jne 0xb7b1
b7a9: call 0x01ae                              ; EXE 01AE ticks_since(0)
b7ac: cmp ax, 0xbd0                            ; until timer 0 = 3024 (41.5 s)
b7af: jb 0xb7b4
b7b1: jmp 0xb83b
b7b4: call 0xb8b7                              ; B8B7: sea colour cycle + key
b7b7: or ax, ax
b7b9: je 0xb7a5
b7bb: mov si, 0xffff
b7be: jmp 0xb7a5
b7c0: mov word ptr [0x9b69], 0x6               ; DS:9B69 text colour; story in colour 6
b7c6: mov word ptr [bp - 0x6], 0x5             ; y = 5
b7cb: mov di, 0x6a5c                           ; DS:6A5C story lines (8 ptrs)
b7ce: jmp 0xb81a
b7d0: mov ax, 0x1
b7d3: push ax
b7d4: call 0x00d7                              ; EXE 00D7 timer_start(n); timer 1 = 0
b7d7: push word ptr [di]
b7d9: mov ax, 0x5
b7dc: push ax
b7dd: push word ptr [bp - 0x6]
b7e0: call 0x899c                              ; EXE 899C text(str, x, y) in [9B69]; text(line, 5, y)
b7e3: mov al, byte ptr [0x9b5c]                ; DS:9B5C font line height
b7e6: sub ah, ah
b7e8: mov cl, byte ptr [0x9b5d]                ; DS:9B5D font line gap
b7ec: sub ch, ch
b7ee: add ax, cx
b7f0: add word ptr [bp - 0x6], ax              ; y += 8 + 3
b7f3: mov si, word ptr [bp - 0x2]
b7f6: mov word ptr [bp - 0x4], di
b7f9: jmp 0xb811
b7fb: mov ax, 0x1
b7fe: push ax
b7ff: call 0x0183                              ; EXE 0183 ticks_since(n)
b802: cmp ax, 0x90                             ; 144 ticks (2.0 s) a line
b805: jae 0xb815
b807: call 0x03aa                              ; EXE 03AA key_read: next key or 0
b80a: or al, al
b80c: je 0xb811
b80e: mov si, 0xffff                           ; a key: the rest printed at once
b811: or si, si
b813: je 0xb7fb
b815: inc di
b816: inc di
b817: mov word ptr [bp - 0x2], si
b81a: cmp di, 0x6a6c                           ; DS:6A6C end of the story lines
b81e: jb 0xb7d0
b820: or si, si
b822: jne 0xb82c
b824: call 0x01ae                              ; EXE 01AE ticks_since(0)
b827: cmp ax, 0xa68                            ; the whole text until timer 0 = 2664 (36.6 s)
b82a: jb 0xb82f
b82c: jmp 0xb781
b82f: call 0x03aa                              ; EXE 03AA key_read: next key or 0
b832: or al, al
b834: je 0xb820
b836: mov si, 0xffff
b839: jmp 0xb820
b83b: lea ax, [bp - 0x3a]
b83e: push ax
b83f: call 0x86a0                              ; EXE 86A0 free CBM descriptor; free title
b842: or si, si                                ; skipped: no world, no credits
b844: jne 0xb858
b846: sub ax, ax
b848: push ax
b849: push ax
b84a: mov ax, 0x13f
b84d: push ax
b84e: mov ax, 0xc7
b851: push ax
b852: call 0x6973                              ; EXE 6973 restore rect work buffer -> screen (x0, y0, x1, y1); the world again (title erased)
b855: call 0xb970                              ; B970 the first 11 credits (result ignored)
b858: mov ax, 0x54
b85b: push ax
b85c: mov ax, 0x28
b85f: push ax
b860: call 0x30e6                              ; EXE 30E6 frame30e6(x, y): the MMENU parchment frame; frame30e6(84, 40), also after a key
b863: push word ptr [0x6a6c]                   ; DS:6A6C ptr to the closing text DS:6619
b867: sub ax, ax
b869: push ax
b86a: push ax
b86b: mov ax, 0x9f
b86e: push ax
b86f: mov cx, 0x63
b872: push cx
b873: lea cx, [bp - 0x1e]
b876: push cx
b877: call 0x8c9f                              ; EXE 8C9F text_rect(str, minw, 0, cx, cy, &buf); text_rect(DS:6619, 0, 0, 159, 99)
b87a: lea di, [bp - 0x16]
b87d: mov si, ax
b87f: push ss
b880: pop es
b881: movsw word ptr es:[di], word ptr [si]
b882: movsw word ptr es:[di], word ptr [si]
b883: movsw word ptr es:[di], word ptr [si]
b884: movsw word ptr es:[di], word ptr [si]
b885: mov word ptr [0x9b69], 0xf               ; DS:9B69 text colour; white
b88b: push word ptr [0x6a6c]                   ; DS:6A6C ptr to the closing text DS:6619
b88f: mov ax, 0x9f
b892: push ax
b893: push word ptr [bp - 0x14]
b896: call 0x8be4                              ; EXE 8BE4 centred_text(str, cx, y) in [9B69]; centred_text("It is 400 years after the...", 159, y0)
b899: call 0x00f8                              ; EXE 00F8 timer0_reset (= 00D7(0)); timer 0 = 0
b89c: call 0x01ae                              ; EXE 01AE ticks_since(0)
b89f: cmp ax, 0x2d0                            ; 720 ticks (9.9 s) or a key
b8a2: jae 0xb8ab
b8a4: call 0xb8b7
b8a7: or ax, ax
b8a9: je 0xb89c
b8ab: call 0xb8cf                              ; B8CF: DAC A2..A6 back
b8ae: call 0x5d20                              ; EXE 5D20 music_stop; music_stop
b8b1: pop si
b8b2: pop di
b8b3: mov sp, bp
b8b5: pop bp
b8b6: ret

; ---- func B8B7  size 0x18  no bp frame, ret  callers: B694 B9D3
; B8B7: the wait poll: B8FE in VGA, then -1 on a key
b8b7: cmp word ptr [0x9b26], 0x2               ; DS:9B26 graphics mode (2 = VGA); [9B26] graphics mode: 2 = VGA
b8bc: jne 0xb8c1
b8be: call 0xb8fe                              ; B8FE colour cycle
b8c1: call 0x03aa                              ; EXE 03AA key_read: next key or 0; EXE 03AA key_read
b8c4: or al, al
b8c6: je 0xb8cc
b8c8: mov ax, 0xffff
b8cb: ret
b8cc: sub ax, ax
b8ce: ret

; ---- func B8CF  size 0x2f  no bp frame, ret  callers: B694
; B8CF: the five cycle colours DS:6ABA into DAC A2..A6 (VGA only; intro)
b8cf: push di
b8d0: push si
b8d1: cmp word ptr [0x9b26], 0x2               ; DS:9B26 graphics mode (2 = VGA)
b8d6: jne 0xb8fb
b8d8: call 0x84bb                              ; EXE 84BB wait_vretrace; EXE 84BB wait_vretrace
b8db: mov si, 0x6aba                           ; DS:6ABA cycle colours (r, g, b)
b8de: mov di, 0x6ad8                           ; DS:6AD8 DAC entries A2..A6
b8e1: cmp di, 0x6ae2                           ; DS:6AE2 end of the DAC entries
b8e5: jae 0xb8fb
b8e7: push word ptr [di]
b8e9: push word ptr [si]
b8eb: push word ptr [si + 0x2]
b8ee: push word ptr [si + 0x4]
b8f1: call 0x01b6                              ; EXE 01B6 set_dac(i, r, g, b); EXE 01B6 set_dac(i, r, g, b)
b8f4: add si, 0x6
b8f7: inc di
b8f8: inc di
b8f9: jmp 0xb8e1
b8fb: pop si
b8fc: pop di
b8fd: ret

; ---- func B8FE  size 0x72  no bp frame, ret  callers: B8B7
; B8FE: every 4 ticks the colours one place on (counter DS:6AE6)
b8fe: push di
b8ff: push si
b900: mov ax, word ptr [0x6ae2]                ; DS:6AE2 last cycle time low; DS:6AE2/6AE4 last cycle time
b903: mov dx, word ptr [0x6ae4]                ; DS:6AE4 last cycle time high
b907: add ax, 0x4
b90a: adc dx, 0x0
b90d: cmp dx, word ptr [0x44]                  ; DS:0044 timer high; DS:44 timer high
b911: ja 0xb96d
b913: jb 0xb91b
b915: cmp ax, word ptr [0x42]                  ; DS:0042 timer low; DS:42 timer low
b919: jae 0xb96d
b91b: inc byte ptr [0x6ae6]                    ; DS:6AE6 cycle counter 0..4; DS:6AE6 cycle counter 0..4
b91f: cmp byte ptr [0x6ae6], 0x4               ; DS:6AE6 cycle counter 0..4
b924: jbe 0xb92b
b926: mov byte ptr [0x6ae6], 0x0               ; DS:6AE6 cycle counter 0..4
b92b: call 0x84bb                              ; EXE 84BB wait_vretrace; EXE 84BB wait_vretrace
b92e: sub di, di
b930: mov si, 0x6aba                           ; DS:6ABA cycle colours (r, g, b)
b933: jmp 0xb959
b935: mov al, byte ptr [0x6ae6]                ; DS:6AE6 cycle counter 0..4
b938: sub ah, ah
b93a: add ax, di
b93c: mov cx, 0x4                              ; mod 4 (bug: 5 colours, counter 0..4)
b93f: cdq
b940: idiv cx
b942: mov bx, dx
b944: shl bx
b946: push word ptr [bx + 0x6ad8]              ; tbl DS:6AD8 DAC entries A2..A6; DS:6AD8 DAC entries A2..A6
b94a: push word ptr [si]                       ; DS:6ABA five r, g, b triples
b94c: push word ptr [si + 0x2]
b94f: push word ptr [si + 0x4]
b952: call 0x01b6                              ; EXE 01B6 set_dac(i, r, g, b); EXE 01B6 set_dac(i, r, g, b)
b955: add si, 0x6
b958: inc di
b959: cmp si, 0x6ad8                           ; DS:6AD8 DAC entries A2..A6
b95d: jb 0xb935
b95f: mov ax, word ptr [0x42]                  ; DS:0042 timer low
b962: mov dx, word ptr [0x44]                  ; DS:0044 timer high
b966: mov word ptr [0x6ae2], ax                ; DS:6AE2 last cycle time low
b969: mov word ptr [0x6ae4], dx                ; DS:6AE4 last cycle time high
b96d: pop si
b96e: pop di
b96f: ret

; ---- func B970  size 0x31  no bp frame, ret  callers: B694
; B970: the intro credits (11 pairs, 288 ticks each); -1 on a key
b970: push di
b971: push si
b972: sub di, di
b974: mov ax, 0x54
b977: push ax
b978: mov ax, 0x28
b97b: push ax
b97c: call 0x30e6                              ; EXE 30E6 frame30e6(x, y): the MMENU parchment frame; EXE 30E6 frame(x 84, y 40)
b97f: mov si, 0x6a6e                           ; DS:6A6E credit pairs (title, names); DS:6A6E credits pairs (title, names)
b982: jmp 0xb996
b984: push si
b985: mov ax, 0x120
b988: push ax
b989: call 0xb9d3                              ; B9D3(pair, 288)
b98c: mov di, ax
b98e: cmp di, -0x1
b991: je 0xb99c
b993: add si, 0x4
b996: cmp si, 0x6a9a                           ; DS:6A9A the first 11 pairs only; the first 11 pairs only
b99a: jb 0xb984
b99c: mov ax, di
b99e: pop si
b99f: pop di
b9a0: ret

; ---- func B9A1  size 0x32  no bp frame, ret
; B9A1: ENTRY (thunk B0CE, called from section 4 DF70): the end credits, 18 pairs 216 ticks each, then the quote 360 ticks
b9a1: push si
b9a2: mov ax, 0x54
b9a5: push ax
b9a6: mov ax, 0x28
b9a9: push ax
b9aa: call 0x30e6                              ; EXE 30E6 frame30e6(x, y): the MMENU parchment frame; EXE 30E6 frame(x 84, y 40)
b9ad: mov si, 0x6a6e                           ; DS:6A6E credit pairs (title, names); DS:6A6E credits pairs (title, names)
b9b0: jmp 0xb9c0
b9b2: push si
b9b3: mov ax, 0xd8
b9b6: push ax
b9b7: call 0xb9d3                              ; B9D3(pair, 216)
b9ba: inc ax
b9bb: je 0xb9c6
b9bd: add si, 0x4
b9c0: cmp si, 0x6ab6                           ; DS:6AB6 ("", "\"Beware the ides of March\""); 18 pairs
b9c4: jb 0xb9b2
b9c6: mov ax, 0x6ab6                           ; DS:6AB6 ("", "\"Beware the ides of March\""); DS:6AB6 ("", "\"Beware the ides of March\"")
b9c9: push ax
b9ca: mov ax, 0x168
b9cd: push ax
b9ce: call 0xb9d3                              ; B9D3(6AB6, 360)
b9d1: pop si
b9d2: ret

; ---- func B9D3  size 0xa2  bp frame, locals 18, ret 0x4  callers: B970 B9A1
; B9D3(pair [bp+6], ticks [bp+4]): one credit; -1 when a key ended the wait
b9d3: push bp
b9d4: mov bp, sp
b9d6: sub sp, 0x12
b9d9: push di
b9da: push si
b9db: mov word ptr [bp - 0x2], 0x0
b9e0: mov bx, word ptr [bp + 0x6]
b9e3: push word ptr [bx + 0x2]
b9e6: push word ptr [bx]
b9e8: call 0x8c58                              ; EXE 8C58 text_width(str); EXE 8C58 text_width(title); the names stay pushed
b9eb: push ax
b9ec: sub ax, ax
b9ee: push ax
b9ef: mov cx, 0x9f
b9f2: push cx
b9f3: mov dx, 0x63
b9f6: push dx
b9f7: lea dx, [bp - 0x12]
b9fa: push dx
b9fb: call 0x8c9f                              ; EXE 8C9F text_rect(str, minw, 0, cx, cy, &buf); EXE 8C9F text_rect(names, minw, 0, 159, 99, &buf)
b9fe: lea di, [bp - 0xa]
ba01: mov si, ax
ba03: push ss
ba04: pop es
ba05: movsw word ptr es:[di], word ptr [si]
ba06: movsw word ptr es:[di], word ptr [si]
ba07: movsw word ptr es:[di], word ptr [si]
ba08: movsw word ptr es:[di], word ptr [si]
ba09: mov word ptr [0x9b69], 0x1               ; DS:9B69 text colour; DS:9B69 text colour = 1
ba0f: mov bx, word ptr [bp + 0x6]
ba12: push word ptr [bx]
ba14: mov ax, 0x9f
ba17: push ax
ba18: mov cx, 0x2f
ba1b: push cx
ba1c: call 0x8be4                              ; EXE 8BE4 centred_text(str, cx, y) in [9B69]; EXE 8BE4 centred_text(title, 159, 47)
ba1f: mov word ptr [0x9b69], 0x0               ; DS:9B69 text colour; DS:9B69 text colour = 0
ba25: mov bx, word ptr [bp + 0x6]
ba28: push word ptr [bx + 0x2]
ba2b: mov ax, 0x9f
ba2e: push ax
ba2f: push word ptr [bp - 0x8]
ba32: call 0x8be4                              ; EXE 8BE4 centred_text(str, cx, y) in [9B69]; EXE 8BE4 centred_text(names, 159, y0)
ba35: mov ax, word ptr [0x6346]                ; DS:6346 box fill colour; DS:6346 frame fill
ba38: mov word ptr [0x9afa], ax                ; DS:9AFA fill colour; DS:9AFA fill colour
ba3b: mov ax, 0x1
ba3e: push ax
ba3f: call 0x00d7                              ; EXE 00D7 timer_start(n); EXE 00D7 timer_start(1)
ba42: mov ax, 0x1
ba45: push ax
ba46: call 0x0183                              ; EXE 0183 ticks_since(n); EXE 0183 ticks_since(1)
ba49: cmp ax, word ptr [bp + 0x4]
ba4c: jae 0xba5a
ba4e: call 0xb8b7                              ; B8B7: -1 on a key
ba51: or ax, ax
ba53: je 0xba42
ba55: mov word ptr [bp - 0x2], 0xffff
ba5a: push word ptr [bp - 0xa]
ba5d: mov ax, 0x2f
ba60: push ax
ba61: push word ptr [bp - 0x6]
ba64: push word ptr [bp - 0x4]
ba67: call 0x7e74                              ; EXE 7E74 fill rect (x0, y0, x1, y1) in [9AFA]; EXE 7E74 fill(x0, 47, x1, y1)
ba6a: mov ax, word ptr [bp - 0x2]
ba6d: pop si
ba6e: pop di
ba6f: mov sp, bp
ba71: pop bp
ba72: ret 0x4

; ---- section padding
ba75: db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
