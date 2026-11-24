; OVL0 section 4 (file 0x51A0-0x8550, 0x33B0 bytes) = chariot race + victory sequence, loaded at 1331:0000 = 0823:B0E0.
;   Bounds from the overlay manager's section table at 1322:0000 (EXE image 0xAFF0; manager code 129B:0292/02EC): the
;   entry for section s spans table bytes 16(s-1)+6 .. 16s+5 (flags +6, relocs +8, load seg +A, end seg +C, file-name
;   ptr +E, file offset in paragraphs +10/+12, size in paragraphs +14): section 4 = offset 0x051A, size 0x033B, 0 relocs,
;   load seg 1331, end seg 166C, OVL0.OVL.
; Entries: 0xD08A (EXE thunk 0xB0AA, called from 0x349D: Rome games "Racing Chariots") and 0xDF58 (EXE thunk 0xB092,
;   called from 0x09A4: rank Caesar, the victory sequence). No other thunk or byte-scanned near call targets section 4.
; llvm-objdump code16 listing (clang --target=i386-unknown-elf .incbin + llvm-objdump -M intel --adjust-vma), addresses are
;   0823 offsets. Made by recursive descent from the two entries (every jcc/jmp/call target followed, the two indirect
;   jumps resolved: B598 = CS jump table at B59D, DEB8 = jmp ax through the DS table 8BB0), not by a linear sweep: the
;   sweep loses the instruction stream at the 5 word-alignment pads (BDA3 D089 D391 DE63 DF57, decoded as
;   "add [di-0x75],dl" / "add [bp+si],ch", which swallows the prologues of BDA4, D08A, D392, DE64 and the entry DF58)
;   and runs through the jump table B59D-B5B4. 53 functions tile 0xB0E0-0xE482 with no gap or overlap; every branch
;   target is an instruction start; data inside the code: the jump table B59D-B5B4 only; E482-E48F = section padding.
; Notation: absolute DS operands are printed unsigned ([0xc37b], llvm prints [-0x3c85]); call targets are masked to 16
;   bits (llvm prints 0x1076b for 0x076B). llvm's code16 "cwde" = cbw (98h), "cdq" = cwd (99h). Comments: EXE routine
;   names (from the notes; (G) = guessed here), "tbl DS:xxxx" for reg+disp table bases, and the static DS string
;   (OVL0.OVL 0xB400 = DS:0000) an immediate points at. Index: docs/RACE_INDEX.md.

; ---- func B0E0  size 0x3c  no bp frame, ret  callers: D08A
; init the 10 speckle records DS:AA78..AAB3 (6 bytes: x = 5 + 4*rng(77), y = 45 + rng(109), 16-bit pattern = rng(65535)): track-surface texture (D; role G)
b0e0: push si
b0e1: mov si, 0xaa7a
b0e4: jmp 0xb114
b0e6: mov ax, 0x4d
b0e9: push ax
b0ea: call 0x76b                             ; EXE rng_roll(n)
b0ed: shl ax
b0ef: shl ax
b0f1: add ax, 0x5
b0f4: mov word ptr [si - 0x2], ax
b0f7: mov word ptr [si], 0x2d
b0fb: mov ax, 0x6d
b0fe: push ax
b0ff: call 0x76b                             ; EXE rng_roll(n)
b102: add ax, 0x2d
b105: mov word ptr [si], ax
b107: mov ax, 0xffff
b10a: push ax
b10b: call 0x76b                             ; EXE rng_roll(n)
b10e: mov word ptr [si + 0x2], ax
b111: add si, 0x6
b114: cmp si, 0xaab6
b118: jb 0xb0e6
b11a: pop si
b11b: ret

; ---- func B11C  size 0x74  bp frame, locals 4, ret 0x2  callers: B190 DA2E
; B11C(scroll): scroll the speckles by the player's speed (C2FE accumulated in 83EA, 4.4 fixed) and plot each with DE64; respawn at y 45 past y 170 (D)
b11c: push bp
b11d: mov bp, sp
b11f: sub sp, 0x4
b122: push di
b123: push si
b124: cmp word ptr [bp + 0x4], 0x0
b128: je 0xb146
b12a: mov cl, 0x4
b12c: mov al, byte ptr [0xc2fe]
b12f: sub ah, ah
b131: add word ptr [0x83ea], ax
b135: mov di, word ptr [0x83ea]
b139: shr di, cl
b13b: mov al, byte ptr [0x83ea]
b13e: and ax, 0xf
b141: mov word ptr [0x83ea], ax
b144: jmp 0xb148
b146: sub di, di
b148: mov si, 0xaa7a
b14b: jmp 0xb182
b14d: push word ptr [si - 0x2]
b150: push word ptr [si]
b152: push word ptr [si + 0x2]
b155: call 0xde64
b158: add word ptr [si], di
b15a: cmp word ptr [si], 0xaa
b15e: jle 0xb17f
b160: mov ax, 0x4d
b163: push ax
b164: call 0x76b                             ; EXE rng_roll(n)
b167: shl ax
b169: shl ax
b16b: add ax, 0x5
b16e: mov word ptr [si - 0x2], ax
b171: mov word ptr [si], 0x2d
b175: mov ax, 0xffff
b178: push ax
b179: call 0x76b                             ; EXE rng_roll(n)
b17c: mov word ptr [si + 0x2], ax
b17f: add si, 0x6
b182: cmp si, 0xaab6
b186: jb 0xb14d
b188: pop si
b189: pop di
b18a: mov sp, bp
b18c: pop bp
b18d: ret 0x2

; ---- func B190  size 0x2d0  bp frame, locals 10, ret  callers: D08A
; one race tick: view clear, move the 4 chariots along the 368-point path (laps +6, leader lap C350, 3 laps -> state 7), screen y (BAC5), crash (B6CB), per-chariot step (B460), minimap dots; end of race -> C32C (D/G)
b190: push bp
b191: mov bp, sp
b193: sub sp, 0xa
b196: push di
b197: push si
b198: mov word ptr [0xaab4], 0x0
b19e: inc word ptr [0xc322]
b1a2: mov word ptr [0x9afa], 0x7
b1a8: cmp word ptr [0x9b26], 0x2
b1ad: jne 0xb1b5
b1af: mov word ptr [0x9afa], 0x11
b1b5: cmp word ptr [0x9b26], 0x6
b1ba: je 0xb1bf
b1bc: call 0x67e4                            ; EXE draw into the work buffer
b1bf: sub ax, ax
b1c1: push ax
b1c2: mov ax, 0x3d
b1c5: push ax
b1c6: mov ax, 0x13f
b1c9: push ax
b1ca: mov ax, 0xaa
b1cd: push ax
b1ce: call 0x7e74                            ; EXE fill rect (colour, ...)
b1d1: mov si, 0xc30a
b1d4: jmp 0xb1f9
b1d6: cmp word ptr [0x9b26], 0x6
b1db: jne 0xb1f1
b1dd: mov bx, word ptr [si]
b1df: shl bx
b1e1: shl bx
b1e3: mov di, word ptr [0xc308]
b1e7: push word ptr [bx + di]
b1e9: add di, bx
b1eb: push word ptr [di + 0x2]
b1ee: call 0x82de                            ; EXE putpixel(x, y), colour [9AFA]
b1f1: mov ax, word ptr [si + 0x2]
b1f4: mov word ptr [si], ax
b1f6: add si, 0x4
b1f9: cmp si, 0xc31a
b1fd: jb 0xb1d6
b1ff: mov ax, 0xc0da
b202: mov di, ax
b204: mov si, 0x83d4
b207: push ds
b208: pop es
b209: movsw word ptr es:[di], word ptr [si]
b20a: movsw word ptr es:[di], word ptr [si]
b20b: movsw word ptr es:[di], word ptr [si]
b20c: movsw word ptr es:[di], word ptr [si]
b20d: mov ax, 0xffff
b210: push ax
b211: call 0xb11c
b214: mov word ptr [bp - 0x2], 0xc2be
b219: sub ax, ax
b21b: mov word ptr [bp - 0x4], ax
b21e: mov word ptr [bp - 0x8], ax
b221: mov word ptr [bp - 0xa], 0xc31a
b226: jmp 0xb391
b229: mov bx, word ptr [bp - 0x2]
b22c: cmp word ptr [bx + 0x4], 0x0
b230: jne 0xb235
b232: jmp 0xb382
b235: push word ptr [bp - 0x4]
b238: call 0xd5e1
b23b: cmp byte ptr [0xc34c], 0x0
b240: jne 0xb247
b242: mov bx, word ptr [bp - 0x2]
b245: jmp 0xb25e
b247: mov al, byte ptr [0xc34c]
b24a: and al, 0xf
b24c: mov cl, 0x12
b24e: mul cl
b250: mov bx, ax
b252: mov al, byte ptr [bx + 0xc2c9]         ; tbl DS:C2C9
b256: mov bx, word ptr [bp - 0x2]
b259: cmp al, byte ptr [bx + 0xb]
b25c: jbe 0xb261
b25e: mov al, byte ptr [bx + 0xb]
b261: mov byte ptr [bp - 0x5], al
b264: cmp byte ptr [0xc34e], 0x0
b269: je 0xb285
b26b: mov al, byte ptr [0xc34e]
b26e: and al, 0xf
b270: mov cl, 0x12
b272: mul cl
b274: mov bx, ax
b276: mov al, byte ptr [bx + 0xc2c9]         ; tbl DS:C2C9
b27a: cmp al, byte ptr [bp - 0x5]
b27d: jae 0xb282
b27f: mov al, byte ptr [bp - 0x5]
b282: mov byte ptr [bp - 0x5], al
b285: mov al, byte ptr [bp - 0x5]
b288: sub ah, ah
b28a: mov bx, word ptr [bp - 0x2]
b28d: add word ptr [bx], ax
b28f: mov ax, word ptr [bx]
b291: mov cx, 0x4b
b294: sub dx, dx
b296: div cx
b298: mov bx, word ptr [bp - 0x8]
b29b: sub ax, word ptr [bx + 0xc30c]         ; tbl DS:C30C
b29f: add word ptr [bx + 0xc30c], ax         ; tbl DS:C30C
b2a3: cmp word ptr [bx + 0xc30c], 0x170      ; tbl DS:C30C
b2a9: jb 0xb30c
b2ab: mov bx, word ptr [bp - 0x2]
b2ae: sub word ptr [bx], 0x6bd0
b2b2: mov bx, word ptr [bp - 0x8]
b2b5: sub word ptr [bx + 0xc30c], 0x170      ; tbl DS:C30C
b2bb: mov bx, word ptr [bp - 0x2]
b2be: inc byte ptr [bx + 0x6]
b2c1: mov al, byte ptr [0xc350]
b2c4: cmp byte ptr [bx + 0x6], al
b2c7: jle 0xb30c
b2c9: mov word ptr [0xc332], 0xffff
b2cf: inc byte ptr [0xc350]
b2d3: cmp byte ptr [0xc350], 0x2
b2d8: jne 0xb30c
b2da: mov ax, 0xc0da
b2dd: mov di, ax
b2df: mov si, 0x83dc
b2e2: push ds
b2e3: pop es
b2e4: movsw word ptr es:[di], word ptr [si]
b2e5: movsw word ptr es:[di], word ptr [si]
b2e6: movsw word ptr es:[di], word ptr [si]
b2e7: movsw word ptr es:[di], word ptr [si]
b2e8: mov word ptr [0x9afa], 0xf
b2ee: mov cx, 0x78
b2f1: push cx
b2f2: mov dx, 0x16
b2f5: push dx
b2f6: push cx
b2f7: mov cx, 0x1c
b2fa: push cx
b2fb: call 0x979c                            ; EXE line(x0, y0, x1, y1) via 97B6 with putpixel 82DE
b2fe: mov ax, 0xc0da
b301: mov di, ax
b303: mov si, 0x83d4
b306: push ds
b307: pop es
b308: movsw word ptr es:[di], word ptr [si]
b309: movsw word ptr es:[di], word ptr [si]
b30a: movsw word ptr es:[di], word ptr [si]
b30b: movsw word ptr es:[di], word ptr [si]
b30c: mov bx, word ptr [bp - 0x2]
b30f: cmp byte ptr [bx + 0x6], 0x3
b313: jne 0xb327
b315: cmp byte ptr [bx + 0xd], 0x7
b319: je 0xb327
b31b: mov byte ptr [bx + 0xd], 0x7
b31f: mov ax, word ptr [0xc322]
b322: mov bx, word ptr [bp - 0xa]
b325: mov word ptr [bx], ax
b327: mov bx, word ptr [bp - 0x8]
b32a: mov si, word ptr [bx + 0xc336]         ; tbl DS:C336
b32e: push word ptr [bp - 0x4]
b331: call 0xbac5
b334: mov word ptr [0xaa76], ax
b337: cmp word ptr [bp - 0xa], 0xc320
b33c: je 0xb363
b33e: or word ptr [0xaab4], ax
b342: or ax, ax
b344: jne 0xb363
b346: cmp byte ptr [0xc301], 0x5
b34b: jne 0xb363
b34d: mov al, 0x7
b34f: mov bx, word ptr [bp - 0x2]
b352: mov byte ptr [bx + 0xe], al
b355: mov bx, word ptr [bp - 0x2]
b358: mov byte ptr [bx + 0xd], al
b35b: mov bx, word ptr [bp - 0x2]
b35e: mov word ptr [bx + 0x4], 0x0
b363: mov bx, word ptr [bp - 0x2]
b366: cmp byte ptr [bx + 0xd], 0x5
b36a: jne 0xb372
b36c: push word ptr [bp - 0x4]
b36f: call 0xb6cb
b372: mov bx, word ptr [bp - 0x2]
b375: cmp word ptr [bx + 0x4], 0x0
b379: je 0xb382
b37b: push word ptr [bp - 0x4]
b37e: push si
b37f: call 0xb460
b382: add word ptr [bp - 0x2], 0x12
b386: add word ptr [bp - 0x8], 0x4
b38a: add word ptr [bp - 0xa], 0x2
b38e: inc word ptr [bp - 0x4]
b391: cmp word ptr [bp - 0xa], 0xc322
b396: jae 0xb39b
b398: jmp 0xb229
b39b: mov ax, 0xc0da
b39e: mov di, ax
b3a0: mov si, 0x83dc
b3a3: push ds
b3a4: pop es
b3a5: movsw word ptr es:[di], word ptr [si]
b3a6: movsw word ptr es:[di], word ptr [si]
b3a7: movsw word ptr es:[di], word ptr [si]
b3a8: movsw word ptr es:[di], word ptr [si]
b3a9: cmp word ptr [0x9b26], 0x6
b3ae: je 0xb3d9
b3b0: call 0x680f                            ; EXE target = the screen descriptor
b3b3: push word ptr [0x83d4]
b3b7: push word ptr [0x83d6]
b3bb: push word ptr [0x83d8]
b3bf: push word ptr [0x83da]
b3c3: call 0x6973                            ; EXE restore rect work buffer -> screen
b3c6: mov ax, 0x68
b3c9: push ax
b3ca: mov ax, 0x16
b3cd: push ax
b3ce: mov ax, 0x113
b3d1: push ax
b3d2: mov ax, 0x27
b3d5: push ax
b3d6: call 0x6973                            ; EXE restore rect work buffer -> screen
b3d9: mov word ptr [bp - 0x8], 0x0
b3de: mov di, 0xc30c
b3e1: mov word ptr [bp - 0x6], 0x87de
b3e6: mov si, 0xc2c2
b3e9: jmp 0xb41d
b3eb: cmp word ptr [si], 0x0
b3ee: je 0xb413
b3f0: cmp byte ptr [si + 0x2], 0x3
b3f4: je 0xb413
b3f6: inc word ptr [bp - 0x8]
b3f9: mov bx, word ptr [bp - 0x6]
b3fc: mov ax, word ptr [bx]
b3fe: mov word ptr [0x9afa], ax
b401: mov bx, word ptr [di]
b403: shl bx
b405: shl bx
b407: add bx, word ptr [0xc308]
b40b: push word ptr [bx]
b40d: push word ptr [bx + 0x2]
b410: call 0x82de                            ; EXE putpixel(x, y), colour [9AFA]
b413: add di, 0x4
b416: add word ptr [bp - 0x6], 0x2
b41a: add si, 0x12
b41d: cmp si, 0xc30a
b421: jb 0xb3eb
b423: cmp word ptr [0xc2f8], 0x0
b428: je 0xb454
b42a: cmp byte ptr [0xc301], 0x7
b42f: je 0xb454
b431: cmp word ptr [bp - 0x8], 0x1
b435: jne 0xb45a
b437: cmp byte ptr [0xc301], 0x5
b43c: je 0xb45a
b43e: cmp byte ptr [0xc301], 0xb
b443: je 0xb45a
b445: mov word ptr [0xc32c], 0xffff
b44b: mov ax, word ptr [0xc322]
b44e: inc ax
b44f: mov word ptr [0xc320], ax
b452: jmp 0xb45a
b454: mov word ptr [0xc32c], 0xffff
b45a: pop si
b45b: pop di
b45c: mov sp, bp
b45e: pop bp
b45f: ret

; ---- func B460  size 0x26b  bp frame, locals 6, ret 0x4  callers: B190
; B460(chariot, y): per-chariot step: steering and speed by the state C2CC (switch B598 over 12 states), lane changes, bumps, then draws the chariot (BD0A) (G)
b460: push bp
b461: mov bp, sp
b463: sub sp, 0x6
b466: push di
b467: push si
b468: mov si, word ptr [bp + 0x6]
b46b: mov ax, si
b46d: mov cx, 0x12
b470: imul cx
b472: mov bx, ax
b474: mov word ptr [bp - 0x2], bx
b477: mov al, byte ptr [bx + 0xc2cc]         ; tbl DS:C2CC
b47b: sub ah, ah
b47d: or ax, ax
b47f: je 0xb492
b481: sub ax, 0x8
b484: jne 0xb489
b486: jmp 0xb50d
b489: dec ax
b48a: jl 0xb48f
b48c: dec ax
b48d: jle 0xb492
b48f: jmp 0xb51d
b492: cmp byte ptr [bx + 0xc2cc], 0xa        ; tbl DS:C2CC
b497: jne 0xb4ae
b499: push si
b49a: call 0xb84e
b49d: or ax, ax
b49f: jne 0xb4cc
b4a1: cmp byte ptr [0xc346], 0x0
b4a6: je 0xb4cc
b4a8: push si
b4a9: mov ax, 0x1
b4ac: jmp 0xb4c8
b4ae: cmp byte ptr [bx + 0xc2cc], 0x9        ; tbl DS:C2CC
b4b3: jne 0xb4cc
b4b5: push si
b4b6: call 0xb886
b4b9: or ax, ax
b4bb: jne 0xb4cc
b4bd: cmp byte ptr [0xc348], 0x0
b4c2: je 0xb4cc
b4c4: push si
b4c5: mov ax, 0x2
b4c8: push ax
b4c9: call 0xb977
b4cc: mov bx, si
b4ce: shl bx
b4d0: shl bx
b4d2: add bx, 0xc336
b4d6: mov word ptr [bp - 0x4], bx
b4d9: mov ax, word ptr [bp + 0x4]
b4dc: cmp word ptr [bx], ax
b4de: jle 0xb4f1
b4e0: cmp byte ptr [0xc34e], 0x0
b4e5: je 0xb4f1
b4e7: mov word ptr [bx], ax
b4e9: push si
b4ea: mov ax, 0x5
b4ed: push ax
b4ee: call 0xb977
b4f1: mov bx, word ptr [bp - 0x4]
b4f4: mov ax, word ptr [bp + 0x4]
b4f7: cmp word ptr [bx], ax
b4f9: jge 0xb51d
b4fb: cmp byte ptr [0xc34c], 0x0
b500: je 0xb51d
b502: mov word ptr [bx], ax
b504: mov al, byte ptr [0xc34c]
b507: and ax, 0xf
b50a: push ax
b50b: jmp 0xb516
b50d: push si
b50e: call 0xb810
b511: or ax, ax
b513: jne 0xb51d
b515: push si
b516: mov ax, 0x5
b519: push ax
b51a: call 0xb977
b51d: mov bx, si
b51f: shl bx
b521: mov word ptr [bp - 0x6], bx
b524: mov bx, word ptr [bx + 0x83cc]         ; tbl DS:83CC
b528: mov bx, word ptr [bx]
b52a: cmp word ptr [bx + 0xa], 0x0
b52e: je 0xb53f
b530: mov bx, word ptr [bp - 0x6]
b533: mov bx, word ptr [bx + 0x83cc]         ; tbl DS:83CC
b537: mov bx, word ptr [bx]
b539: cmp byte ptr [bx + 0xd], 0x0
b53d: je 0xb567
b53f: mov bx, word ptr [bp - 0x6]
b542: mov bx, word ptr [bx + 0x83cc]         ; tbl DS:83CC
b546: mov bx, word ptr [bx]
b548: mov bx, word ptr [bx]
b54a: cmp word ptr [bx + 0xa], 0x0
b54e: jne 0xb553
b550: jmp 0xb6ad
b553: mov bx, word ptr [bp - 0x6]
b556: mov bx, word ptr [bx + 0x83cc]         ; tbl DS:83CC
b55a: mov bx, word ptr [bx]
b55c: mov bx, word ptr [bx]
b55e: cmp byte ptr [bx + 0xd], 0x0
b562: je 0xb567
b564: jmp 0xb6ad
b567: mov bx, word ptr [bp - 0x2]
b56a: mov al, byte ptr [bx + 0xc2c8]         ; tbl DS:C2C8
b56e: cmp byte ptr [bx + 0xc2c7], al         ; tbl DS:C2C7
b572: jae 0xb587
b574: mov al, byte ptr [bx + 0xc2c7]         ; tbl DS:C2C7
b578: mov byte ptr [bx + 0xc2c8], al         ; tbl DS:C2C8
b57c: cmp si, 0x3
b57f: jne 0xb587
b581: mov word ptr [0xc32e], 0xffff
b587: mov bx, word ptr [bp - 0x2]
b58a: mov al, byte ptr [bx + 0xc2cc]         ; tbl DS:C2CC
b58e: sub ah, ah
b590: cmp ax, 0xb
b593: ja 0xb60c
b595: shl ax
b597: xchg ax, bx
b598: jmp word ptr cs:[bx + 0xb59d]          ; switch on [bx+C2CC] (0..11): jump table at B59D (12 words, inside the code)
; ---- data island: jump table of B598 (12 near offsets), code resumes at B5B5
b59d: dw 0xb5b5                              ; jump table of B598 (case 0)
b59f: dw 0xb60c                              ; jump table of B598 (case 1)
b5a1: dw 0xb5be                              ; jump table of B598 (case 2)
b5a3: dw 0xb5dd                              ; jump table of B598 (case 3)
b5a5: dw 0xb60c                              ; jump table of B598 (case 4)
b5a7: dw 0xb60c                              ; jump table of B598 (case 5)
b5a9: dw 0xb60c                              ; jump table of B598 (case 6)
b5ab: dw 0xb60c                              ; jump table of B598 (case 7)
b5ad: dw 0xb5b5                              ; jump table of B598 (case 8)
b5af: dw 0xb5b5                              ; jump table of B598 (case 9)
b5b1: dw 0xb5b5                              ; jump table of B598 (case 10)
b5b3: dw 0xb5f1                              ; jump table of B598 (case 11)
b5b5: push si
b5b6: sub ax, ax
b5b8: push ax
b5b9: call 0xba28
b5bc: jmp 0xb60c
b5be: cmp byte ptr [0xc34a], 0x0
b5c3: je 0xb5d3
b5c5: push si
b5c6: mov ax, 0x3
b5c9: push ax
b5ca: call 0xb8bc
b5cd: push si
b5ce: mov ax, 0x2
b5d1: jmp 0xb5b8
b5d3: mov bx, word ptr [bp - 0x2]
b5d6: mov byte ptr [bx + 0xc2cc], 0x1        ; tbl DS:C2CC
b5db: jmp 0xb60c
b5dd: cmp byte ptr [0xc344], 0x0
b5e2: je 0xb5d3
b5e4: push si
b5e5: sub ax, ax
b5e7: push ax
b5e8: call 0xb8bc
b5eb: push si
b5ec: mov ax, 0x3
b5ef: jmp 0xb5b8
b5f1: mov bx, word ptr [bp - 0x6]
b5f4: mov di, word ptr [bx + 0x83cc]         ; tbl DS:83CC
b5f8: mov di, word ptr [di]
b5fa: mov word ptr [di + 0xa], 0x0
b5ff: mov bx, word ptr [bx + 0x83cc]         ; tbl DS:83CC
b603: mov bx, word ptr [bx]
b605: mov bx, word ptr [bx]
b607: mov word ptr [bx + 0xa], 0xffff
b60c: mov bx, word ptr [bp - 0x2]
b60f: cmp byte ptr [bx + 0xc2cc], 0x1        ; tbl DS:C2CC
b614: jne 0xb665
b616: cmp byte ptr [0xc34c], 0x0
b61b: jne 0xb665
b61d: cmp byte ptr [bx + 0xc2ce], 0x0        ; tbl DS:C2CE
b622: je 0xb647
b624: push si
b625: mov ax, 0xa
b628: push ax
b629: call 0x76b                             ; EXE rng_roll(n)
b62c: add ax, 0xa
b62f: push ax
b630: call 0xb7d2
b633: mov bx, word ptr [bp - 0x2]
b636: dec byte ptr [bx + 0xc2ce]             ; tbl DS:C2CE
b63a: cmp si, 0x3
b63d: jne 0xb653
b63f: mov word ptr [0xc330], 0xffff
b645: jmp 0xb653
b647: push si
b648: mov ax, 0xa
b64b: push ax
b64c: call 0x76b                             ; EXE rng_roll(n)
b64f: push ax
b650: call 0xb7d2
b653: push si
b654: mov ax, 0x1
b657: push ax
b658: call 0xba28
b65b: mov ax, word ptr [0xc322]
b65e: mov bx, word ptr [bp - 0x6]
b661: mov word ptr [bx + 0xc324], ax         ; tbl DS:C324
b665: mov bx, word ptr [bp - 0x2]
b668: mov al, byte ptr [bx + 0xc2cc]         ; tbl DS:C2CC
b66c: mov byte ptr [bx + 0xc2cb], al         ; tbl DS:C2CB
b670: mov byte ptr [bx + 0xc2cc], 0x0        ; tbl DS:C2CC
b675: cmp byte ptr [bx + 0xc2cb], 0xb        ; tbl DS:C2CB
b67a: je 0xb6a5
b67c: mov bx, word ptr [bp - 0x2]
b67f: cmp byte ptr [bx + 0xc2cb], 0x5        ; tbl DS:C2CB
b684: jne 0xb6ad
b686: mov bx, word ptr [bp - 0x6]
b689: mov di, word ptr [bx + 0x83cc]         ; tbl DS:83CC
b68d: mov di, word ptr [di]
b68f: mov di, word ptr [di]
b691: mov word ptr [di + 0xa], 0x0
b696: mov bx, word ptr [bx + 0x83cc]         ; tbl DS:83CC
b69a: mov bx, word ptr [bx]
b69c: mov bx, word ptr [bx]
b69e: mov bx, word ptr [bx]
b6a0: mov word ptr [bx + 0xa], 0xffff
b6a5: mov bx, word ptr [bp - 0x2]
b6a8: mov byte ptr [bx + 0xc2cc], 0x5        ; tbl DS:C2CC
b6ad: mov bx, word ptr [bp - 0x6]
b6b0: push word ptr [bx + 0x83cc]            ; tbl DS:83CC
b6b4: shl si
b6b6: shl si
b6b8: push word ptr [si + 0xc334]            ; tbl DS:C334
b6bc: push word ptr [si + 0xc336]            ; tbl DS:C336
b6c0: call 0xbd0a
b6c3: pop si
b6c4: pop di
b6c5: mov sp, bp
b6c7: pop bp
b6c8: ret 0x4

; ---- func B6CB  size 0x107  bp frame, locals 4, ret 0x2  callers: B190
; B6CB(chariot): crash/wreck state 5: counters C2CF to 0x18/0x30, slows down; for the player the crash scene D738 (G)
b6cb: push bp
b6cc: mov bp, sp
b6ce: sub sp, 0x4
b6d1: push si
b6d2: mov si, word ptr [bp + 0x4]
b6d5: mov bx, si
b6d7: mov bx, word ptr [bx + si + 0x83cc]    ; tbl DS:83CC
b6db: mov bx, word ptr [bx]
b6dd: mov bx, word ptr [bx]
b6df: mov word ptr [bx + 0xa], 0x0
b6e4: mov bx, si
b6e6: mov bx, word ptr [bx + si + 0x83cc]    ; tbl DS:83CC
b6ea: mov bx, word ptr [bx]
b6ec: mov bx, word ptr [bx]
b6ee: mov bx, word ptr [bx]
b6f0: mov word ptr [bx + 0xa], 0xffff
b6f5: cmp word ptr [0xaa76], 0x0
b6fa: jne 0xb715
b6fc: mov ax, si
b6fe: mov cx, 0x12
b701: mul cx
b703: mov bx, ax
b705: mov al, 0x7
b707: mov byte ptr [bx + 0xc2cc], al         ; tbl DS:C2CC
b70b: mov byte ptr [bx + 0xc2cb], al         ; tbl DS:C2CB
b70f: mov word ptr [bx + 0xc2c2], 0x0        ; tbl DS:C2C2
b715: mov ax, si
b717: mov cx, 0x12
b71a: mul cx
b71c: mov bx, ax
b71e: mov word ptr [bp - 0x4], bx
b721: inc byte ptr [bx + 0xc2cf]             ; tbl DS:C2CF
b725: cmp byte ptr [bx + 0xc2cf], 0x18       ; tbl DS:C2CF
b72a: jae 0xb75d
b72c: cmp byte ptr [0xc2fe], 0x46
b731: jbe 0xb739
b733: push si
b734: call 0xb810
b737: jmp 0xb748
b739: push si
b73a: mov ax, 0xa
b73d: push ax
b73e: call 0x76b                             ; EXE rng_roll(n)
b741: add ax, 0xa
b744: push ax
b745: call 0xb7d2
b748: mov ax, 0x2
b74b: push ax
b74c: call 0x76b                             ; EXE rng_roll(n)
b74f: or ax, ax
b751: jne 0xb7c3
b753: mov bx, word ptr [bp - 0x4]
b756: mov byte ptr [bx + 0xc2cc], 0x9        ; tbl DS:C2CC
b75b: jmp 0xb7cb
b75d: cmp si, 0x3
b760: jne 0xb7ac
b762: cmp word ptr [0xaab4], 0x0
b767: jne 0xb78f
b769: mov bx, 0xc2c2
b76c: jmp 0xb775
b76e: mov word ptr [bx], 0x0
b772: add bx, 0x12
b775: cmp bx, 0xc30a
b779: jb 0xb76e
b77b: mov bx, word ptr [0x83d2]
b77f: mov bx, word ptr [bx]
b781: mov bx, word ptr [bx]
b783: mov bx, word ptr [bx]
b785: mov word ptr [bx + 0xa], 0x0
b78a: call 0xd738
b78d: jmp 0xb7cb
b78f: mov bx, word ptr [bp - 0x4]
b792: cmp byte ptr [bx + 0xc2cf], 0x30       ; tbl DS:C2CF
b797: jbe 0xb7a3
b799: mov al, byte ptr [bx + 0xc2c8]         ; tbl DS:C2C8
b79d: cmp byte ptr [bx + 0xc2c7], al         ; tbl DS:C2C7
b7a1: jb 0xb7bf
b7a3: cmp byte ptr [bx + 0xc2c8], 0x46       ; tbl DS:C2C8
b7a8: jbe 0xb7c3
b7aa: jmp 0xb7bf
b7ac: mov bx, word ptr [bp - 0x4]
b7af: mov al, byte ptr [bx + 0xc2c7]         ; tbl DS:C2C7
b7b3: mov cl, 0x3
b7b5: sub ah, ah
b7b7: div cl
b7b9: cmp al, byte ptr [bx + 0xc2c8]         ; tbl DS:C2C8
b7bd: jae 0xb7c3
b7bf: push si
b7c0: call 0xb810
b7c3: mov bx, word ptr [bp - 0x4]
b7c6: mov byte ptr [bx + 0xc2cc], 0xa        ; tbl DS:C2CC
b7cb: pop si
b7cc: mov sp, bp
b7ce: pop bp
b7cf: ret 0x2

; ---- func B7D2  size 0x3e  bp frame, locals 0, ret 0x4  callers: B460 B6CB
; B7D2(chariot, n): accelerate: speed C2C8 += n up to the limit (G)
b7d2: push bp
b7d3: mov bp, sp
b7d5: cmp byte ptr [0xc34c], 0x0
b7da: jne 0xb80a
b7dc: mov ax, 0x12
b7df: mul word ptr [bp + 0x6]
b7e2: mov bx, ax
b7e4: mov al, byte ptr [bx + 0xc2c8]         ; tbl DS:C2C8
b7e8: sub ah, ah
b7ea: add ax, word ptr [bp + 0x4]
b7ed: cmp ax, 0x14
b7f0: jae 0xb7f5
b7f2: mov ax, 0x14
b7f5: mov byte ptr [bx + 0xc2c8], al         ; tbl DS:C2C8
b7f9: cmp word ptr [bp + 0x6], 0x3
b7fd: jne 0xb805
b7ff: mov word ptr [0xc32e], 0xffff
b805: mov ax, 0xffff
b808: jmp 0xb80c
b80a: sub ax, ax
b80c: pop bp
b80d: ret 0x4

; ---- func B810  size 0x3e  bp frame, locals 0, ret 0x2  callers: B460 B6CB
; B810(chariot): slow down (G)
b810: push bp
b811: mov bp, sp
b813: cmp byte ptr [0xc34e], 0x0
b818: jne 0xb848
b81a: mov ax, 0x12
b81d: mul word ptr [bp + 0x4]
b820: mov bx, ax
b822: mov al, byte ptr [bx + 0xc2c8]         ; tbl DS:C2C8
b826: sub ah, ah
b828: sub ax, 0x5
b82b: cmp ax, 0x14
b82e: jge 0xb833
b830: mov ax, 0x14
b833: mov byte ptr [bx + 0xc2c8], al         ; tbl DS:C2C8
b837: cmp word ptr [bp + 0x4], 0x3
b83b: jne 0xb843
b83d: mov word ptr [0xc32e], 0xffff
b843: mov ax, 0xffff
b846: jmp 0xb84a
b848: sub ax, ax
b84a: pop bp
b84b: ret 0x2

; ---- func B84E  size 0x38  bp frame, locals 0, ret 0x2  callers: B460
; B84E(chariot): one lane step left (x C334 += 2) unless blocked (C344/C346) (G)
b84e: push bp
b84f: mov bp, sp
b851: cmp byte ptr [0xc344], 0x0
b856: jne 0xb880
b858: cmp byte ptr [0xc346], 0x0
b85d: jne 0xb880
b85f: mov bx, word ptr [bp + 0x4]
b862: shl bx
b864: shl bx
b866: add word ptr [bx + 0xc334], 0x2        ; tbl DS:C334
b86b: mov ax, word ptr [bx + 0xc334]         ; tbl DS:C334
b86f: cmp ax, 0x118
b872: jle 0xb877
b874: mov ax, 0x118
b877: mov word ptr [bx + 0xc334], ax         ; tbl DS:C334
b87b: mov ax, 0xffff
b87e: jmp 0xb882
b880: sub ax, ax
b882: pop bp
b883: ret 0x2

; ---- func B886  size 0x36  bp frame, locals 0, ret 0x2  callers: B460
; B886(chariot): one lane step right (x C334 -= 2) unless blocked (C348/C34A) (G)
b886: push bp
b887: mov bp, sp
b889: cmp byte ptr [0xc34a], 0x0
b88e: jne 0xb8b6
b890: cmp byte ptr [0xc348], 0x0
b895: jne 0xb8b6
b897: mov bx, word ptr [bp + 0x4]
b89a: shl bx
b89c: shl bx
b89e: sub word ptr [bx + 0xc334], 0x2        ; tbl DS:C334
b8a3: mov ax, word ptr [bx + 0xc334]         ; tbl DS:C334
b8a7: or ax, ax
b8a9: jge 0xb8ad
b8ab: sub ax, ax
b8ad: mov word ptr [bx + 0xc334], ax         ; tbl DS:C334
b8b1: mov ax, 0xffff
b8b4: jmp 0xb8b8
b8b6: sub ax, ax
b8b8: pop bp
b8b9: ret 0x2

; ---- func B8BC  size 0xbb  bp frame, locals 4, ret 0x4  callers: B460
; B8BC(chariot, side): side bump: damage (D53E chariot, D590 horses), rng speed change (G)
b8bc: push bp
b8bd: mov bp, sp
b8bf: sub sp, 0x4
b8c2: push di
b8c3: push si
b8c4: mov ax, 0x12
b8c7: mul word ptr [bp + 0x6]
b8ca: mov bx, ax
b8cc: cmp word ptr [bx + 0xc2c2], 0x0        ; tbl DS:C2C2
b8d1: jne 0xb8d6
b8d3: jmp 0xb96f
b8d6: mov bx, word ptr [bp + 0x4]
b8d9: shl bx
b8db: mov al, byte ptr [bx + 0xc344]         ; tbl DS:C344
b8df: mov byte ptr [bp - 0x2], al
b8e2: mov cl, byte ptr [bx + 0xc345]         ; tbl DS:C345
b8e6: mov byte ptr [bp - 0x1], al
b8e9: test cl, 0x10
b8ec: je 0xb8f9
b8ee: test byte ptr [bp - 0x2], 0x20
b8f2: je 0xb8f9
b8f4: mov al, cl
b8f6: mov byte ptr [bp - 0x1], al
b8f9: test byte ptr [bp - 0x1], 0x20
b8fd: je 0xb90e
b8ff: mov al, byte ptr [bp - 0x1]
b902: and ax, 0xf
b905: push ax
b906: mov al, 0x1
b908: push ax
b909: call 0xd53e
b90c: jmp 0xb96f
b90e: test byte ptr [bp - 0x1], 0x10
b912: je 0xb96f
b914: mov al, byte ptr [bp - 0x1]
b917: sub ah, ah
b919: mov cx, 0x12
b91c: mov bx, ax
b91e: imul cx
b920: mov si, ax
b922: mov al, byte ptr [si + 0xc2c8]         ; tbl DS:C2C8
b926: mov byte ptr [bp - 0x2], al
b929: mov ax, 0xa
b92c: push ax
b92d: mov di, bx
b92f: call 0x76b                             ; EXE rng_roll(n)
b932: add byte ptr [si + 0xc2c8], al         ; tbl DS:C2C8
b936: mov al, byte ptr [si + 0xc2c8]         ; tbl DS:C2C8
b93a: cmp al, 0x14
b93c: jae 0xb940
b93e: mov al, 0x14
b940: mov byte ptr [si + 0xc2c8], al         ; tbl DS:C2C8
b944: mov ax, word ptr [0xc322]
b947: shl di
b949: mov word ptr [di + 0xc324], ax         ; tbl DS:C324
b94d: mov al, byte ptr [bp - 0x1]
b950: and ax, 0xf
b953: push ax
b954: mov al, 0x1
b956: push ax
b957: call 0xd590
b95a: mov al, byte ptr [bp - 0x2]
b95d: cmp byte ptr [si + 0xc2c8], al         ; tbl DS:C2C8
b961: je 0xb96f
b963: cmp byte ptr [bp - 0x1], 0x3
b967: jne 0xb96f
b969: mov word ptr [0xc32e], 0xffff
b96f: pop si
b970: pop di
b971: mov sp, bp
b973: pop bp
b974: ret 0x4

; ---- func B977  size 0x52  bp frame, locals 4, ret 0x4  callers: B460
; B977(chariot, side): bump check against the neighbour, then B9C9 (G)
b977: push bp
b978: mov bp, sp
b97a: sub sp, 0x4
b97d: mov ax, 0x12
b980: mul word ptr [bp + 0x6]
b983: mov bx, ax
b985: cmp word ptr [bx + 0xc2c2], 0x0        ; tbl DS:C2C2
b98a: je 0xb9c3
b98c: mov bx, word ptr [bp + 0x4]
b98f: shl bx
b991: mov al, byte ptr [bx + 0xc344]         ; tbl DS:C344
b995: mov byte ptr [bp - 0x2], al
b998: mov cl, byte ptr [bx + 0xc345]         ; tbl DS:C345
b99c: mov byte ptr [bp - 0x1], al
b99f: test cl, 0x20
b9a2: je 0xb9af
b9a4: test byte ptr [bp - 0x2], 0x10
b9a8: je 0xb9af
b9aa: mov al, cl
b9ac: mov byte ptr [bp - 0x1], al
b9af: push word ptr [bp + 0x6]
b9b2: mov al, byte ptr [bp - 0x1]
b9b5: and ax, 0xf
b9b8: push ax
b9b9: mov al, byte ptr [bp - 0x1]
b9bc: and ax, 0x20
b9bf: push ax
b9c0: call 0xb9c9
b9c3: mov sp, bp
b9c5: pop bp
b9c6: ret 0x4

; ---- func B9C9  size 0x5f  bp frame, locals 2, ret 0x6  callers: B977 D17A
; B9C9(a, b, flag): bump damage between two chariots weighted by DS:87EA[type] (20/33/50) (G)
b9c9: push bp
b9ca: mov bp, sp
b9cc: sub sp, 0x2
b9cf: push si
b9d0: mov byte ptr [bp - 0x1], 0x0
b9d4: sub si, si
b9d6: mov ax, 0x64
b9d9: push ax
b9da: call 0x76b                             ; EXE rng_roll(n)
b9dd: mov cx, ax
b9df: mov ax, 0x12
b9e2: mul word ptr [bp + 0x8]
b9e5: mov bx, ax
b9e7: mov bl, byte ptr [bx + 0xc2c6]         ; tbl DS:C2C6
b9eb: sub bh, bh
b9ed: mov al, byte ptr [bx + 0x87ea]         ; tbl DS:87EA
b9f1: sub ah, ah
b9f3: cmp ax, cx
b9f5: jbe 0xb9fa
b9f7: inc byte ptr [bp - 0x1]
b9fa: inc si
b9fb: cmp si, 0x2
b9fe: jl 0xb9d6
ba00: cmp byte ptr [bp - 0x1], 0x0
ba04: je 0xba21
ba06: cmp word ptr [bp + 0x4], 0x0
ba0a: je 0xba18
ba0c: push word ptr [bp + 0x6]
ba0f: mov al, byte ptr [bp - 0x1]
ba12: push ax
ba13: call 0xd53e
ba16: jmp 0xba21
ba18: push word ptr [bp + 0x6]
ba1b: mov al, 0x1
ba1d: push ax
ba1e: call 0xd590
ba21: pop si
ba22: mov sp, bp
ba24: pop bp
ba25: ret 0x6

; ---- func BA28  size 0x9d  bp frame, locals 8, ret 0x4  callers: B460
; BA28(chariot, y): pick the animation chain for the state (83CC) (G)
ba28: push bp
ba29: mov bp, sp
ba2b: sub sp, 0x8
ba2e: push di
ba2f: mov di, word ptr [bp + 0x6]
ba32: mov ax, di
ba34: mov cx, 0x12
ba37: mul cx
ba39: mov bx, ax
ba3b: mov al, byte ptr [bx + 0xc2cb]         ; tbl DS:C2CB
ba3f: mov byte ptr [bp - 0x6], al
ba42: cmp al, byte ptr [bp + 0x4]
ba45: je 0xba4b
ba47: cmp al, 0xb
ba49: jne 0xba4d
ba4b: jmp 0xbabe
ba4d: mov bx, di
ba4f: mov bx, word ptr [bx + di + 0x83cc]    ; tbl DS:83CC
ba53: mov bx, word ptr [bx]
ba55: mov ax, word ptr [bx + 0x2]
ba58: mov word ptr [bp - 0x2], ax
ba5b: mov word ptr [bp - 0x4], ax
ba5e: cmp byte ptr [bp - 0x6], 0x4
ba62: jb 0xba8c
ba64: cmp byte ptr [bp - 0x6], 0x7
ba68: ja 0xba8c
ba6a: mov al, byte ptr [bp - 0x6]
ba6d: cmp al, 0xb
ba6f: jne 0xbabe
ba71: sub dx, dx
ba73: cmp dx, 0x3
ba76: jg 0xbabe
ba78: mov bx, word ptr [bp - 0x2]
ba7b: mov word ptr [bx + 0xa], 0x0
ba80: mov ax, word ptr [bx]
ba82: or ax, ax
ba84: je 0xba89
ba86: mov word ptr [bp - 0x2], ax
ba89: inc dx
ba8a: jmp 0xba73
ba8c: sub dx, dx
ba8e: mov bx, word ptr [bp - 0x2]
ba91: mov word ptr [bx + 0xa], 0x0
ba96: mov ax, word ptr [bx]
ba98: or ax, ax
ba9a: je 0xba9f
ba9c: mov word ptr [bp - 0x2], ax
ba9f: inc dx
baa0: cmp dx, 0x3
baa3: jle 0xba8e
baa5: sub dx, dx
baa7: mov bx, word ptr [bp - 0x4]
baaa: jmp 0xbab4
baac: cmp word ptr [bx], 0x0
baaf: je 0xbab3
bab1: mov bx, word ptr [bx]
bab3: inc dx
bab4: cmp word ptr [bp + 0x4], dx
bab7: jg 0xbaac
bab9: mov word ptr [bx + 0xa], 0xffff
babe: pop di
babf: mov sp, bp
bac1: pop bp
bac2: ret 0x4

; ---- func BAC5  size 0xe8  bp frame, locals 4, ret 0x2  callers: B190
; BAC5(chariot): screen y of a chariot relative to the player (C336, clamped -22..170); returns 0 when off screen (G)
bac5: push bp
bac6: mov bp, sp
bac8: sub sp, 0x4
bacb: push di
bacc: push si
bacd: mov bx, word ptr [bp + 0x4]
bad0: mov ax, word ptr [0xc316]
bad3: mov si, bx
bad5: shl si
bad7: shl si
bad9: mov word ptr [bp - 0x4], si
badc: sub ax, word ptr [si + 0xc30a]         ; tbl DS:C30A
bae0: mov di, ax
bae2: or ax, ax
bae4: jle 0xbaf2
bae6: cmp di, 0xb8
baea: jle 0xbafc
baec: sub di, 0x170
baf0: jmp 0xbafc
baf2: cmp di, 0xff48
baf6: jge 0xbafc
baf8: add di, 0x170
bafc: mov cl, 0x3
bafe: mov ax, di
bb00: shl di, cl
bb02: add di, 0x64
bb05: mov word ptr [bp - 0x2], di
bb08: cdq
bb09: xor ax, dx
bb0b: sub ax, dx
bb0d: cmp ax, 0x18
bb10: jle 0xbb1e
bb12: mov ax, word ptr [bp - 0x2]
bb15: mov si, word ptr [bp - 0x4]
bb18: mov word ptr [si + 0xc336], ax         ; tbl DS:C336
bb1c: jmp 0xbb8a
bb1e: mov si, word ptr [bp - 0x4]
bb21: cmp word ptr [si + 0xc336], 0xaa       ; tbl DS:C336
bb27: jle 0xbb2f
bb29: cmp di, 0xaa
bb2d: jg 0xbb12
bb2f: cmp word ptr [si + 0xc336], -0x16      ; tbl DS:C336
bb34: jge 0xbb3b
bb36: cmp di, -0x16
bb39: jl 0xbb12
bb3b: cmp word ptr [si + 0xc336], 0xaa       ; tbl DS:C336
bb41: jle 0xbb4b
bb43: mov word ptr [si + 0xc336], 0xaa       ; tbl DS:C336
bb49: jmp 0xbb58
bb4b: cmp word ptr [si + 0xc336], -0x16      ; tbl DS:C336
bb50: jge 0xbb58
bb52: mov word ptr [si + 0xc336], 0xffea     ; tbl DS:C336
bb58: mov ax, word ptr [bp - 0x2]
bb5b: mov si, word ptr [bp - 0x4]
bb5e: sub ax, word ptr [si + 0xc336]         ; tbl DS:C336
bb62: cdq
bb63: xor ax, dx
bb65: sub ax, dx
bb67: mov di, ax
bb69: cmp di, 0x4
bb6c: jle 0xbb71
bb6e: mov di, 0x4
bb71: mov ax, word ptr [bp - 0x2]
bb74: cmp word ptr [si + 0xc336], ax         ; tbl DS:C336
bb78: jle 0xbb80
bb7a: sub word ptr [si + 0xc336], di         ; tbl DS:C336
bb7e: jmp 0xbb8a
bb80: cmp word ptr [si + 0xc336], ax         ; tbl DS:C336
bb84: jge 0xbb8a
bb86: add word ptr [si + 0xc336], di         ; tbl DS:C336
bb8a: mov si, word ptr [bp - 0x4]
bb8d: cmp word ptr [si + 0xc336], 0xaa       ; tbl DS:C336
bb93: jg 0xbba3
bb95: mov bx, si
bb97: cmp word ptr [bx + 0xc336], -0x16      ; tbl DS:C336
bb9c: jl 0xbba3
bb9e: mov ax, 0xffff
bba1: jmp 0xbba5
bba3: sub ax, ax
bba5: pop si
bba6: pop di
bba7: mov sp, bp
bba9: pop bp
bbaa: ret 0x2

; ---- func BBAD  size 0x68  bp frame, locals 0, ret 0x4  callers: C2DE
; BBAD(type, chariot): set up a chariot's sprite chain for its type (83C4/83C8/83E4) (G)
bbad: push bp
bbae: mov bp, sp
bbb0: push di
bbb1: push si
bbb2: mov bl, byte ptr [bp + 0x6]
bbb5: sub bh, bh
bbb7: mov al, 0xe
bbb9: mul byte ptr [bx + 0x83c4]             ; tbl DS:83C4
bbbd: add ax, word ptr [0xc2bc]
bbc1: mov si, word ptr [bp + 0x4]
bbc4: shl si
bbc6: mov di, word ptr [si + 0x83cc]         ; tbl DS:83CC
bbca: mov di, word ptr [di]
bbcc: mov word ptr [di + 0x4], ax
bbcf: mov al, 0xe
bbd1: mul byte ptr [bx + 0x83c8]             ; tbl DS:83C8
bbd5: add ax, word ptr [0xc2bc]
bbd9: mov di, word ptr [si + 0x83cc]         ; tbl DS:83CC
bbdd: mov di, word ptr [di]
bbdf: mov di, word ptr [di]
bbe1: mov word ptr [di + 0x4], ax
bbe4: shl bx
bbe6: mov ax, word ptr [bx + 0x83e4]         ; tbl DS:83E4
bbea: mov bx, word ptr [si + 0x83cc]         ; tbl DS:83CC
bbee: mov bx, word ptr [bx]
bbf0: mov bx, word ptr [bx]
bbf2: mov word ptr [bx + 0x6], ax
bbf5: mov bx, word ptr [si + 0x83cc]         ; tbl DS:83CC
bbf9: mov bx, word ptr [bx]
bbfb: mov word ptr [bx + 0xa], 0xffff
bc00: mov bx, word ptr [si + 0x83cc]         ; tbl DS:83CC
bc04: mov bx, word ptr [bx]
bc06: mov bx, word ptr [bx]
bc08: mov word ptr [bx + 0xa], 0x0
bc0d: pop si
bc0e: pop di
bc0f: mov sp, bp
bc11: pop bp
bc12: ret 0x4

; ---- func BC15  size 0x8b  bp frame, locals 12, ret  callers: C2DE
; load the MRACE sheets race, laps, med (C2BC, C306, C352) and number the sprite chains (BCA0); 0 = failure (D)
bc15: push bp
bc16: mov bp, sp
bc18: sub sp, 0xc
bc1b: push si
bc1c: sub ax, ax
bc1e: push ax
bc1f: lea ax, [bp - 0xc]
bc22: push ax
bc23: call 0xf7b                             ; EXE archive open (0F7B(n, &arch))
bc26: mov word ptr [bp - 0x6], ax
bc29: or ax, ax
bc2b: je 0xbc67
bc2d: mov ax, 0x83ec                         ; "race"
bc30: push ax
bc31: lea ax, [bp - 0xc]
bc34: push ax
bc35: sub cx, cx
bc37: push cx
bc38: call 0x163b                            ; EXE sheet_163B: load xxxHDR/xxxDAT sprite sheet
bc3b: mov word ptr [0xc2bc], ax
bc3e: mov ax, 0x83f1                         ; "laps"
bc41: push ax
bc42: lea ax, [bp - 0xc]
bc45: push ax
bc46: sub cx, cx
bc48: push cx
bc49: call 0x163b                            ; EXE sheet_163B: load xxxHDR/xxxDAT sprite sheet
bc4c: mov word ptr [0xc306], ax
bc4f: mov ax, 0x83f6                         ; "med"
bc52: push ax
bc53: lea ax, [bp - 0xc]
bc56: push ax
bc57: sub cx, cx
bc59: push cx
bc5a: call 0x163b                            ; EXE sheet_163B: load xxxHDR/xxxDAT sprite sheet
bc5d: mov word ptr [0xc352], ax
bc60: lea ax, [bp - 0xc]
bc63: push ax
bc64: call 0x11a9                            ; EXE archive close
bc67: cmp word ptr [0xc2bc], 0x0
bc6c: je 0xbc93
bc6e: cmp word ptr [0xc306], 0x0
bc73: je 0xbc93
bc75: sub si, si
bc77: cmp si, 0x8
bc7a: jge 0xbc98
bc7c: mov byte ptr [bp - 0x3], 0x0
bc80: push word ptr [si + 0x83cc]            ; tbl DS:83CC
bc84: push word ptr [si + 0x83bc]            ; tbl DS:83BC
bc88: lea ax, [bp - 0x3]
bc8b: push ax
bc8c: call 0xbca0
bc8f: inc si
bc90: inc si
bc91: jmp 0xbc77
bc93: mov word ptr [bp - 0x6], 0x0
bc98: mov ax, word ptr [bp - 0x6]
bc9b: pop si
bc9c: mov sp, bp
bc9e: pop bp
bc9f: ret

; ---- func BCA0  size 0x54  bp frame, locals 0, ret 0x6  callers: BC15 BCA0
; BCA0(&n, frames, node): recursive: give each node of a sprite-chain tree its frame and offset table (D)
bca0: push bp
bca1: mov bp, sp
bca3: push di
bca4: push si
bca5: mov si, word ptr [bp + 0x8]
bca8: jmp 0xbce8
bcaa: mov bx, word ptr [bp + 0x4]
bcad: mov bl, byte ptr [bx]
bcaf: sub bh, bh
bcb1: mov di, word ptr [bp + 0x6]
bcb4: mov al, 0xe
bcb6: mul byte ptr [bx + di]
bcb8: add ax, word ptr [0xc2bc]
bcbc: mov word ptr [si + 0x4], ax
bcbf: mov bx, word ptr [bp + 0x4]
bcc2: mov bl, byte ptr [bx]
bcc4: sub bh, bh
bcc6: shl bx
bcc8: mov ax, word ptr [bx + 0x83a4]         ; tbl DS:83A4
bccc: mov word ptr [si + 0xa], ax
bccf: mov bx, word ptr [bp + 0x4]
bcd2: inc byte ptr [bx]
bcd4: cmp word ptr [si + 0x2], 0x0
bcd8: je 0xbce6
bcda: push word ptr [si + 0x2]
bcdd: push word ptr [bp + 0x6]
bce0: push word ptr [bp + 0x4]
bce3: call 0xbca0
bce6: mov si, word ptr [si]
bce8: or si, si
bcea: jne 0xbcaa
bcec: pop si
bced: pop di
bcee: mov sp, bp
bcf0: pop bp
bcf1: ret 0x6

; ---- func BCF4  size 0x16  no bp frame, ret  callers: C2DE C70F D08A
; free the three race sheets (D)
bcf4: push word ptr [0xc352]
bcf8: call 0x19a0                            ; EXE sheet_19A0: free sprite sheet
bcfb: push word ptr [0xc306]
bcff: call 0x19a0                            ; EXE sheet_19A0: free sprite sheet
bd02: push word ptr [0xc2bc]
bd06: call 0x19a0                            ; EXE sheet_19A0: free sprite sheet
bd09: ret

; ---- func BD0A  size 0x99  bp frame, locals 6, ret 0x6  callers: B460 BD0A D738
; BD0A(node, x, y): recursive: draw a sprite-chain tree (horses, chariot, driver) and step its animation (D)
bd0a: push bp
bd0b: mov bp, sp
bd0d: sub sp, 0x6
bd10: push di
bd11: push si
bd12: mov si, word ptr [bp + 0x8]
bd15: jmp 0xbd94
bd17: cmp word ptr [si + 0xa], 0x0
bd1b: je 0xbd92
bd1d: mov al, byte ptr [si + 0x8]
bd20: cwde
bd21: mov bl, byte ptr [si + 0xd]
bd24: sub bh, bh
bd26: mov cx, bx
bd28: shl bx
bd2a: add bx, word ptr [si + 0x6]
bd2d: mov dx, ax
bd2f: mov al, byte ptr [bx]
bd31: cwde
bd32: add dx, ax
bd34: add dx, word ptr [bp + 0x6]
bd37: mov di, dx
bd39: mov al, byte ptr [si + 0x9]
bd3c: cwde
bd3d: mov word ptr [bp - 0x4], ax
bd40: mov al, byte ptr [bx + 0x1]
bd43: cwde
bd44: add ax, word ptr [bp - 0x4]
bd47: add ax, word ptr [bp + 0x4]
bd4a: mov word ptr [bp - 0x2], ax
bd4d: mov bx, ax
bd4f: mov ax, cx
bd51: mov cx, 0xe
bd54: mov word ptr [bp - 0x6], dx
bd57: imul cx
bd59: add ax, word ptr [si + 0x4]
bd5c: push ax
bd5d: push word ptr [bp - 0x6]
bd60: push bx
bd61: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
bd64: inc byte ptr [si + 0xe]
bd67: mov al, byte ptr [si + 0xf]
bd6a: cmp byte ptr [si + 0xe], al
bd6d: ja 0xbd82
bd6f: mov byte ptr [si + 0xe], 0x0
bd73: inc byte ptr [si + 0xd]
bd76: mov al, byte ptr [si + 0xd]
bd79: cmp byte ptr [si + 0xc], al
bd7c: ja 0xbd82
bd7e: mov byte ptr [si + 0xd], 0x0
bd82: cmp word ptr [si + 0x2], 0x0
bd86: je 0xbd92
bd88: push word ptr [si + 0x2]
bd8b: push di
bd8c: push word ptr [bp - 0x2]
bd8f: call 0xbd0a
bd92: mov si, word ptr [si]
bd94: or si, si
bd96: je 0xbd9b
bd98: jmp 0xbd17
bd9b: pop si
bd9c: pop di
bd9d: mov sp, bp
bd9f: pop bp
bda0: ret 0x6
bda3: db 0x00                                ; word-alignment pad

; ---- func BDA4  size 0x6c  bp frame, locals 6, ret  callers: C2DE DF58
; open MRACE: picture slot 3 = raceest.cbm (13A8), gallery1.cbm -> C356, sheet race2 -> C354; C364 = loaded (D)
bda4: push bp
bda5: mov bp, sp
bda7: sub sp, 0x6
bdaa: sub ax, ax
bdac: push ax
bdad: lea ax, [bp - 0x6]
bdb0: push ax
bdb1: call 0xf7b                             ; EXE archive open (0F7B(n, &arch))
bdb4: mov word ptr [0xc364], ax
bdb7: or ax, ax
bdb9: je 0xbe0c
bdbb: mov ax, 0x3
bdbe: push ax
bdbf: lea ax, [bp - 0x6]
bdc2: push ax
bdc3: call 0x13a8                            ; EXE load CBM picture from archive (13A8)
bdc6: mov word ptr [0xc364], ax
bdc9: or ax, ax
bdcb: je 0xbe05
bdcd: mov ax, 0x83fa                         ; "gallery1.cbm"
bdd0: push ax
bdd1: lea ax, [bp - 0x6]
bdd4: push ax
bdd5: mov ax, 0xc356
bdd8: push ax
bdd9: call 0x1273                            ; EXE load CBM member into a descriptor
bddc: mov word ptr [0xc364], ax
bddf: or ax, ax
bde1: je 0xbe05
bde3: mov ax, 0x8407                         ; "race2"
bde6: push ax
bde7: lea ax, [bp - 0x6]
bdea: push ax
bdeb: sub ax, ax
bded: push ax
bdee: call 0x163b                            ; EXE sheet_163B: load xxxHDR/xxxDAT sprite sheet
bdf1: mov word ptr [0xc354], ax
bdf4: or ax, ax
bdf6: jne 0xbe05
bdf8: mov ax, 0xc356
bdfb: push ax
bdfc: call 0x86a0                            ; EXE picture_free(desc): DOS free (int 21h/49h) of the block at desc+0C
bdff: mov word ptr [0xc364], 0x0
be05: lea ax, [bp - 0x6]
be08: push ax
be09: call 0x11a9                            ; EXE archive close
be0c: mov sp, bp
be0e: pop bp
be0f: ret

; ---- func BE10  size 0x15a  bp frame, locals 6, ret  callers: C2DE
; pre-race parade in the gallery (song 2 = cn3a): chariots drive past, any key ends it (G)
be10: push bp
be11: mov bp, sp
be13: sub sp, 0x6
be16: push si
be17: sub si, si
be19: mov word ptr [bp - 0x6], si
be1c: mov word ptr [bp - 0x4], 0x14b
be21: cmp word ptr [0xc364], si
be25: jne 0xbe2a
be27: jmp 0xbf4f
be2a: mov al, 0x2
be2c: push ax
be2d: call 0x5d29                            ; EXE music_play(n)
be30: call 0x4ed                             ; EXE key_flush
be33: mov word ptr [0x9afa], si
be37: sub ax, ax
be39: push ax
be3a: push ax
be3b: mov cx, 0x13f
be3e: push cx
be3f: mov dx, 0xc7
be42: push dx
be43: call 0x7e74                            ; EXE fill rect (colour, ...)
be46: mov ax, 0xc356
be49: push ax
be4a: sub ax, ax
be4c: push ax
be4d: push ax
be4e: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
be51: mov ax, 0xc0d0
be54: push ax
be55: mov ax, 0x840e
be58: push ax
be59: mov ax, 0xc0b6
be5c: push ax
be5d: call 0x8790                            ; EXE blit picture (8790(src, dst...))
be60: sub ax, ax
be62: push ax
be63: push ax
be64: mov ax, 0x13f
be67: push ax
be68: mov ax, 0xc7
be6b: push ax
be6c: call 0x691b                            ; EXE save rect screen -> work buffer
be6f: call 0x6887                            ; EXE flip: work buffer -> screen
be72: call 0x35db                            ; EXE wait until mouse buttons released (G)
be75: jmp 0xbecd
be77: call 0xf8                              ; EXE timer0_reset (= 00D7(0))
be7a: call 0x644                             ; EXE mouse_poll: [B2] buttons, [B6] x, [B8] y
be7d: mov ax, 0xc0d0
be80: push ax
be81: mov ax, 0x840e
be84: push ax
be85: mov ax, 0xc0b6
be88: push ax
be89: call 0x8790                            ; EXE blit picture (8790(src, dst...))
be8c: call 0xc299
be8f: sub ax, ax
be91: push ax
be92: sub word ptr [bp - 0x4], 0x6
be96: mov ax, word ptr [bp - 0x4]
be99: push ax
be9a: push si
be9b: call 0xc23d
be9e: cmp word ptr [bp - 0x4], 0x38
bea2: jle 0xbed3
bea4: inc si
bea5: cmp si, 0x4
bea8: jl 0xbeac
beaa: sub si, si
beac: mov word ptr [bp - 0x2], si
beaf: call 0x1ae                             ; EXE ticks_since(0)
beb2: cmp ax, 0x7
beb5: jb 0xbeaf
beb7: call 0x6887                            ; EXE flip: work buffer -> screen
beba: call 0x3aa                             ; EXE key_read: next key or 0
bebd: or al, al
bebf: jne 0xbec8
bec1: cmp word ptr [0xb2], 0x0
bec6: je 0xbecd
bec8: mov word ptr [bp - 0x6], 0xffff
becd: cmp word ptr [bp - 0x6], 0x0
bed1: je 0xbe77
bed3: mov si, 0x3e
bed6: mov word ptr [bp - 0x2], 0x0
bedb: jmp 0xbf46
bedd: call 0xf8                              ; EXE timer0_reset (= 00D7(0))
bee0: call 0x644                             ; EXE mouse_poll: [B2] buttons, [B6] x, [B8] y
bee3: mov ax, 0xc0d0
bee6: push ax
bee7: mov ax, 0x840e
beea: push ax
beeb: mov ax, 0xc0b6
beee: push ax
beef: call 0x8790                            ; EXE blit picture (8790(src, dst...))
bef2: call 0xc299
bef5: sub ax, ax
bef7: push ax
bef8: lea ax, [si - 0x8]
befb: push ax
befc: mov ax, word ptr [bp - 0x2]
beff: add ax, 0x4
bf02: push ax
bf03: call 0xc23d
bf06: mov word ptr [bp - 0x4], si
bf09: call 0x1ae                             ; EXE ticks_since(0)
bf0c: cmp ax, 0x7
bf0f: jb 0xbf09
bf11: call 0x6887                            ; EXE flip: work buffer -> screen
bf14: mov ax, 0x19
bf17: push ax
bf18: call 0x76b                             ; EXE rng_roll(n)
bf1b: or ax, ax
bf1d: jne 0xbf33
bf1f: mov al, byte ptr [bp - 0x2]
bf22: sub al, -0x1
bf24: and ax, 0x1
bf27: mov word ptr [bp - 0x2], ax
bf2a: cmp si, 0x3e
bf2d: jg 0xbf32
bf2f: inc si
bf30: jmp 0xbf33
bf32: dec si
bf33: call 0x3aa                             ; EXE key_read: next key or 0
bf36: or al, al
bf38: jne 0xbf41
bf3a: cmp word ptr [0xb2], 0x0
bf3f: je 0xbf46
bf41: mov word ptr [bp - 0x6], 0xffff
bf46: cmp word ptr [bp - 0x6], 0x0
bf4a: je 0xbedd
bf4c: call 0x6837                            ; EXE draw on the screen (6887 first if needed)
bf4f: call 0x5d20                            ; EXE music_stop
bf52: mov al, 0x2
bf54: push ax
bf55: call 0x5e1a                            ; EXE music_unload(n)
bf58: call 0x4ed                             ; EXE key_flush
bf5b: cmp word ptr [0xb2], 0x0
bf60: je 0xbf65
bf62: call 0x35db                            ; EXE wait until mouse buttons released (G)
bf65: pop si
bf66: mov sp, bp
bf68: pop bp
bf69: ret

; ---- func BF6A  size 0x2d3  bp frame, locals 84, ret 0x2  callers: C9B6 DF58
; BF6A(mode): parade after a won race (mode 0, the 4 racers, song 2) or the triumph (mode 1, 10 random chariots, song 0) (G)
bf6a: push bp
bf6b: mov bp, sp
bf6d: sub sp, 0x54
bf70: push di
bf71: push si
bf72: mov word ptr [bp - 0xa], 0x0
bf77: mov byte ptr [bp - 0x5], 0x3
bf7b: mov word ptr [bp - 0x8], 0x4
bf80: cmp word ptr [0xc364], 0x0
bf85: jne 0xbf8a
bf87: jmp 0xc216
bf8a: mov word ptr [0x9afa], 0x0
bf90: sub ax, ax
bf92: push ax
bf93: push ax
bf94: mov ax, 0x13f
bf97: push ax
bf98: mov ax, 0xc7
bf9b: push ax
bf9c: call 0x7e74                            ; EXE fill rect (colour, ...)
bf9f: cmp byte ptr [bp + 0x4], 0x1
bfa3: jne 0xbfc8
bfa5: mov word ptr [bp - 0x8], 0xa
bfaa: mov byte ptr [bp - 0x5], 0x9
bfae: sub al, al
bfb0: push ax
bfb1: call 0x5d29                            ; EXE music_play(n)
bfb4: sub si, si
bfb6: cmp si, 0xa
bfb9: jge 0xbfe4
bfbb: mov ax, 0x3
bfbe: push ax
bfbf: call 0x76b                             ; EXE rng_roll(n)
bfc2: mov byte ptr [bp + si - 0x14], al
bfc5: inc si
bfc6: jmp 0xbfb6
bfc8: mov al, 0x2
bfca: push ax
bfcb: call 0x5d29                            ; EXE music_play(n)
bfce: sub si, si
bfd0: mov di, 0xc2c6
bfd3: cmp di, 0xc30e
bfd7: jae 0xbfe4
bfd9: mov al, byte ptr [di]
bfdb: add di, 0x12
bfde: mov byte ptr [bp + si - 0x14], al
bfe1: inc si
bfe2: jmp 0xbfd3
bfe4: sub bx, bx
bfe6: mov cx, bx
bfe8: mov dx, 0x14b
bfeb: jmp 0xc002
bfed: mov si, bx
bfef: mov word ptr [bp + si - 0x28], dx
bff2: mov word ptr [bp + si - 0x3c], 0xffff
bff7: mov word ptr [bp + si - 0x50], 0x0
bffc: inc bx
bffd: inc bx
bffe: add dx, 0x5a
c001: inc cx
c002: cmp word ptr [bp - 0x8], cx
c005: jg 0xbfed
c007: mov al, byte ptr [bp - 0x5]
c00a: cwde
c00b: mov si, ax
c00d: mov word ptr [bp - 0x54], si
c010: shl si
c012: mov word ptr [bp - 0x52], si
c015: add word ptr [bp + si - 0x28], 0x2d
c019: mov si, word ptr [bp - 0x54]
c01c: mov byte ptr [bp + si - 0x14], 0x0
c020: call 0x4ed                             ; EXE key_flush
c023: mov ax, 0xc356
c026: push ax
c027: sub ax, ax
c029: push ax
c02a: push ax
c02b: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
c02e: mov ax, 0xc0d0
c031: push ax
c032: mov ax, 0x840e
c035: push ax
c036: mov ax, 0xc0b6
c039: push ax
c03a: call 0x8790                            ; EXE blit picture (8790(src, dst...))
c03d: sub ax, ax
c03f: push ax
c040: push ax
c041: mov ax, 0x13f
c044: push ax
c045: mov ax, 0xc7
c048: push ax
c049: call 0x691b                            ; EXE save rect screen -> work buffer
c04c: call 0x6887                            ; EXE flip: work buffer -> screen
c04f: call 0x35db                            ; EXE wait until mouse buttons released (G)
c052: mov di, word ptr [bp - 0xa]
c055: or di, di
c057: je 0xc05c
c059: jmp 0xc111
c05c: mov word ptr [bp - 0xa], di
c05f: call 0xf8                              ; EXE timer0_reset (= 00D7(0))
c062: call 0x644                             ; EXE mouse_poll: [B2] buttons, [B6] x, [B8] y
c065: mov ax, 0xc0d0
c068: push ax
c069: mov ax, 0x840e
c06c: push ax
c06d: mov ax, 0xc0b6
c070: push ax
c071: call 0x8790                            ; EXE blit picture (8790(src, dst...))
c074: call 0xc299
c077: sub ax, ax
c079: mov word ptr [bp - 0x4], ax
c07c: mov word ptr [bp - 0x2], ax
c07f: mov si, ax
c081: jmp 0xc0de
c083: cmp word ptr [bp + si - 0x3c], 0x0
c087: je 0xc0d9
c089: mov di, ax
c08b: mov al, byte ptr [bp + di - 0x14]
c08e: cwde
c08f: push ax
c090: sub word ptr [bp + si - 0x28], 0x6
c094: push word ptr [bp + si - 0x28]
c097: push word ptr [bp + si - 0x50]
c09a: call 0xc23d
c09d: cmp byte ptr [bp + 0x4], 0x1
c0a1: jne 0xc0c0
c0a3: mov ax, word ptr [bp - 0x4]
c0a6: cmp word ptr [bp - 0x54], ax
c0a9: jne 0xc0c0
c0ab: mov ax, word ptr [0xc354]
c0ae: add ax, 0x118
c0b1: push ax
c0b2: mov ax, word ptr [bp + si - 0x28]
c0b5: add ax, 0x32
c0b8: push ax
c0b9: mov ax, 0x8e
c0bc: push ax
c0bd: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
c0c0: cmp word ptr [bp + si - 0x28], -0x5a
c0c4: jg 0xc0cb
c0c6: mov word ptr [bp + si - 0x3c], 0x0
c0cb: inc word ptr [bp + si - 0x50]
c0ce: cmp word ptr [bp + si - 0x50], 0x4
c0d2: jl 0xc0d9
c0d4: mov word ptr [bp + si - 0x50], 0x0
c0d9: inc si
c0da: inc si
c0db: inc word ptr [bp - 0x4]
c0de: mov ax, word ptr [bp - 0x4]
c0e1: cmp word ptr [bp - 0x8], ax
c0e4: jg 0xc083
c0e6: call 0x1ae                             ; EXE ticks_since(0)
c0e9: cmp ax, 0x7
c0ec: jb 0xc0e6
c0ee: call 0x6887                            ; EXE flip: work buffer -> screen
c0f1: call 0x3aa                             ; EXE key_read: next key or 0
c0f4: or al, al
c0f6: jne 0xc102
c0f8: mov di, word ptr [bp - 0xa]
c0fb: cmp word ptr [0xb2], 0x0
c100: je 0xc105
c102: mov di, 0xffff
c105: mov si, word ptr [bp - 0x52]
c108: cmp word ptr [bp + si - 0x28], 0x38
c10c: jle 0xc111
c10e: jmp 0xc055
c111: sub al, al
c113: mov byte ptr [bp - 0x2], al
c116: mov byte ptr [bp - 0x1], al
c119: mov si, word ptr [bp - 0x52]
c11c: mov word ptr [bp + si - 0x50], 0x0
c121: or di, di
c123: je 0xc128
c125: jmp 0xc216
c128: call 0xf8                              ; EXE timer0_reset (= 00D7(0))
c12b: call 0x644                             ; EXE mouse_poll: [B2] buttons, [B6] x, [B8] y
c12e: mov ax, 0xc0d0
c131: push ax
c132: mov ax, 0x840e
c135: push ax
c136: mov ax, 0xc0b6
c139: push ax
c13a: call 0x8790                            ; EXE blit picture (8790(src, dst...))
c13d: call 0xc299
c140: cmp byte ptr [bp - 0x1], 0x4
c144: jge 0xc172
c146: mov al, byte ptr [bp - 0x1]
c149: cwde
c14a: add ax, 0xb
c14d: push ax
c14e: mov si, word ptr [bp - 0x52]
c151: push word ptr [bp + si - 0x28]
c154: mov ax, word ptr [bp + si - 0x50]
c157: add ax, 0x4
c15a: push ax
c15b: call 0xc23d
c15e: cmp byte ptr [bp + 0x4], 0x1
c162: jne 0xc1a3
c164: mov al, 0xe
c166: imul byte ptr [bp - 0x1]
c169: add ax, word ptr [0xc354]
c16d: add ax, 0x126
c170: jmp 0xc191
c172: sub ax, ax
c174: push ax
c175: mov si, word ptr [bp - 0x52]
c178: push word ptr [bp + si - 0x28]
c17b: mov ax, word ptr [bp + si - 0x50]
c17e: add ax, 0x4
c181: push ax
c182: call 0xc23d
c185: cmp byte ptr [bp + 0x4], 0x1
c189: jne 0xc1a3
c18b: mov ax, word ptr [0xc354]
c18e: add ax, 0x118
c191: push ax
c192: mov si, word ptr [bp - 0x52]
c195: mov ax, word ptr [bp + si - 0x28]
c198: add ax, 0x32
c19b: push ax
c19c: mov ax, 0x8e
c19f: push ax
c1a0: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
c1a3: mov word ptr [bp - 0xa], di
c1a6: call 0x1ae                             ; EXE ticks_since(0)
c1a9: cmp ax, 0x7
c1ac: jb 0xc1a6
c1ae: call 0x6887                            ; EXE flip: work buffer -> screen
c1b1: mov al, byte ptr [bp - 0x1]
c1b4: cwde
c1b5: mov bx, ax
c1b7: inc byte ptr [bp - 0x2]
c1ba: mov al, byte ptr [bp - 0x2]
c1bd: cmp byte ptr [bx + 0x8412], al         ; tbl DS:8412
c1c1: jge 0xc1c6
c1c3: inc byte ptr [bp - 0x1]
c1c6: cmp byte ptr [bp - 0x1], 0x4
c1ca: jg 0xc1f9
c1cc: mov ax, 0xf
c1cf: push ax
c1d0: call 0x76b                             ; EXE rng_roll(n)
c1d3: or ax, ax
c1d5: jne 0xc1f9
c1d7: mov si, word ptr [bp - 0x52]
c1da: mov al, byte ptr [bp + si - 0x50]
c1dd: sub al, -0x1
c1df: and ax, 0x1
c1e2: mov word ptr [bp + si - 0x50], ax
c1e5: cmp word ptr [bp + si - 0x28], 0x38
c1e9: jg 0xc1f3
c1eb: mov si, word ptr [bp - 0x52]
c1ee: inc word ptr [bp + si - 0x28]
c1f1: jmp 0xc1f9
c1f3: mov si, word ptr [bp - 0x52]
c1f6: dec word ptr [bp + si - 0x28]
c1f9: call 0x3aa                             ; EXE key_read: next key or 0
c1fc: or al, al
c1fe: jne 0xc210
c200: cmp word ptr [0xb2], 0x0
c205: jne 0xc210
c207: cmp byte ptr [bp - 0x2], 0x4b
c20b: jg 0xc210
c20d: jmp 0xc121
c210: mov di, 0xffff
c213: jmp 0xc121
c216: cmp byte ptr [bp + 0x4], 0x1
c21a: je 0xc225
c21c: call 0x5d20                            ; EXE music_stop
c21f: mov al, 0x2
c221: push ax
c222: call 0x5e1a                            ; EXE music_unload(n)
c225: call 0x4ed                             ; EXE key_flush
c228: cmp word ptr [0xb2], 0x0
c22d: je 0xc232
c22f: call 0x35db                            ; EXE wait until mouse buttons released (G)
c232: call 0x6837                            ; EXE draw on the screen (6887 first if needed)
c235: pop si
c236: pop di
c237: mov sp, bp
c239: pop bp
c23a: ret 0x2

; ---- func C23D  size 0x5c  bp frame, locals 0, ret 0x6  callers: BE10 BF6A
; C23D(type, x, frame): one parade chariot = 3 race2 sprites (D)
c23d: push bp
c23e: mov bp, sp
c240: mov ax, 0xe
c243: imul word ptr [bp + 0x8]
c246: add ax, word ptr [0xc354]
c24a: add ax, 0x46
c24d: push ax
c24e: mov ax, word ptr [bp + 0x6]
c251: add ax, 0x32
c254: push ax
c255: mov ax, 0x8e
c258: push ax
c259: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
c25c: mov ax, 0xe
c25f: imul word ptr [bp + 0x4]
c262: add ax, word ptr [0xc354]
c266: add ax, 0x70
c269: push ax
c26a: push word ptr [bp + 0x6]
c26d: mov ax, 0x82
c270: push ax
c271: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
c274: mov al, byte ptr [bp + 0x4]
c277: and al, 0x1
c279: mov cl, 0xe
c27b: mul cl
c27d: add ax, word ptr [0xc354]
c281: add ax, 0xc4
c284: push ax
c285: mov ax, word ptr [bp + 0x6]
c288: add ax, 0x43
c28b: push ax
c28c: mov ax, 0x9e
c28f: push ax
c290: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
c293: mov sp, bp
c295: pop bp
c296: ret 0x6

; ---- func C299  size 0x29  no bp frame, ret  callers: BE10 BF6A
; animate the gallery figure (race2 frame W(8418) 0..4) (D)
c299: mov ax, 0xe
c29c: imul word ptr [0x8418]
c2a0: add ax, word ptr [0xc354]
c2a4: push ax
c2a5: mov ax, 0x124
c2a8: push ax
c2a9: mov ax, 0x23
c2ac: push ax
c2ad: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
c2b0: inc word ptr [0x8418]
c2b4: cmp word ptr [0x8418], 0x5
c2b9: jl 0xc2c1
c2bb: mov word ptr [0x8418], 0x0
c2c1: ret

; ---- func C2C2  size 0x1c  no bp frame, ret  callers: C2DE C70F D08A DF58
; free gallery1 (C356) and the race2 sheet if BDA4 loaded them (D)
c2c2: cmp word ptr [0xc364], 0x0
c2c7: je 0xc2dd
c2c9: mov ax, 0xc356
c2cc: push ax
c2cd: call 0x86a0                            ; EXE picture_free(desc): DOS free (int 21h/49h) of the block at desc+0C
c2d0: push word ptr [0xc354]
c2d4: call 0x19a0                            ; EXE sheet_19A0: free sprite sheet
c2d7: mov word ptr [0xc364], 0x0
c2dd: ret

; ---- func C2DE  size 0x431  bp frame, locals 30, ret  callers: D08A
; pre-race screen: loads, field once per turn (DA5A), racer cards, chariot purchase 5/10/20, prepare menu (race, skulduggery, bet, exit), bet paid, parade, race screen and gauges, track (CBE6); C100++, C102++; returns -1 = race, 0 = cancelled (D)
c2de: push bp
c2df: mov bp, sp
c2e1: sub sp, 0x1e
c2e4: push di
c2e5: push si
c2e6: mov word ptr [bp - 0x4], 0x0
c2eb: mov word ptr [0xc322], 0x0
c2f1: call 0xbc15
c2f4: or ax, ax
c2f6: jne 0xc2fd
c2f8: sub ax, ax
c2fa: jmp 0xc709
c2fd: call 0xbda4
c300: sub ax, ax
c302: push ax
c303: lea ax, [bp - 0x10]
c306: push ax
c307: call 0xf7b                             ; EXE archive open (0F7B(n, &arch))
c30a: mov si, ax
c30c: or si, si
c30e: je 0xc337
c310: mov ax, 0x87af                         ; "circus.cbm"
c313: push ax
c314: lea ax, [bp - 0x10]
c317: push ax
c318: lea ax, [bp - 0x1e]
c31b: push ax
c31c: call 0x1273                            ; EXE load CBM member into a descriptor
c31f: mov si, ax
c321: or si, si
c323: je 0xc330
c325: lea ax, [bp - 0x1e]
c328: push ax
c329: sub ax, ax
c32b: push ax
c32c: push ax
c32d: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
c330: lea ax, [bp - 0x10]
c333: push ax
c334: call 0x11a9                            ; EXE archive close
c337: or si, si
c339: jne 0xc343
c33b: call 0xbcf4
c33e: call 0xc2c2
c341: jmp 0xc2f8
c343: mov al, 0x3
c345: push ax
c346: call 0x5d73                            ; EXE music_load(n)
c349: mov al, 0x2
c34b: push ax
c34c: call 0x5d73                            ; EXE music_load(n)
c34f: cmp word ptr [0x9b26], 0x2
c354: jne 0xc35b
c356: mov ax, 0x11
c359: jmp 0xc35e
c35b: mov ax, 0x7
c35e: mov word ptr [bp - 0xa], ax
c361: mov word ptr [0x9afa], ax
c364: sub ax, ax
c366: push ax
c367: mov ax, 0x3d
c36a: push ax
c36b: mov ax, 0x13f
c36e: push ax
c36f: mov ax, 0xc7
c372: push ax
c373: call 0x7e74                            ; EXE fill rect (colour, ...)
c376: sub si, si
c378: mov word ptr [bp - 0x2], 0xc324
c37d: mov di, 0xc2c2
c380: jmp 0xc3a3
c382: mov ax, word ptr [0x892e]
c385: cmp word ptr [0x634c], ax
c389: je 0xc38f
c38b: push si
c38c: call 0xda5a
c38f: mov word ptr [di], 0xffff
c393: add di, 0x12
c396: mov ax, word ptr [0xc322]
c399: mov bx, word ptr [bp - 0x2]
c39c: add word ptr [bp - 0x2], 0x2
c3a0: mov word ptr [bx], ax
c3a2: inc si
c3a3: cmp di, 0xc30a
c3a7: jb 0xc382
c3a9: mov ax, word ptr [0x634c]
c3ac: mov word ptr [0x892e], ax
c3af: sub si, si
c3b1: push si
c3b2: call 0xcdce
c3b5: inc si
c3b6: cmp si, 0x3
c3b9: jb 0xc3b1
c3bb: mov byte ptr [0xc2fb], 0x0
c3c0: mov ax, 0x88e6
c3c3: push ax
c3c4: mov ax, 0x2
c3c7: push ax
c3c8: call 0x5614                            ; EXE menu draw (5614)
c3cb: mov di, word ptr [bp - 0x4]
c3ce: or di, di
c3d0: jne 0xc44d
c3d2: mov ax, 0x88e6
c3d5: push ax
c3d6: push ax
c3d7: sub ax, ax
c3d9: push ax
c3da: call 0x54c8                            ; EXE menu run -> index or 0xFF
c3dd: mov si, ax
c3df: push si
c3e0: sub ax, ax
c3e2: push ax
c3e3: call 0x585b                            ; EXE menu_preselect(menu, sel, -1)
c3e6: cmp si, 0x2
c3e9: ja 0xc438
c3eb: mov al, byte ptr [si + 0x87f2]         ; tbl DS:87F2
c3ef: sub ah, ah
c3f1: cmp ax, word ptr [0x634a]
c3f5: ja 0xc432
c3f7: mov ax, si
c3f9: mov byte ptr [0xc2fc], al
c3fc: mov al, byte ptr [si + 0x87f2]         ; tbl DS:87F2
c400: sub ah, ah
c402: sub word ptr [0x634a], ax
c406: mov di, 0xffff
c409: mov ax, word ptr [bp - 0xa]
c40c: mov word ptr [0x9afa], ax
c40f: mov ax, 0x6
c412: push ax
c413: mov ax, 0x59
c416: push ax
c417: mov ax, 0x9d
c41a: push ax
c41b: mov ax, 0xc7
c41e: push ax
c41f: call 0x7e74                            ; EXE fill rect (colour, ...)
c422: mov ax, 0x3
c425: push ax
c426: call 0xcdce
c429: mov ax, 0x3e02
c42c: push ax
c42d: call 0x59d4                            ; EXE message_box(str)
c430: jmp 0xc3ce
c432: push word ptr [0x880a]
c436: jmp 0xc42d
c438: push word ptr [0x8814]
c43c: call 0x5c20                            ; EXE yes_no box: 1 = Yes, 0 = No
c43f: or ax, ax
c441: je 0xc3ce
c443: lea ax, [bp - 0x1e]
c446: push ax
c447: call 0xc70f
c44a: jmp 0xc2f8
c44d: lea ax, [bp - 0x1e]
c450: push ax
c451: sub ax, ax
c453: push ax
c454: push ax
c455: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
c458: mov ax, word ptr [bp - 0xa]
c45b: mov word ptr [0x9afa], ax
c45e: sub ax, ax
c460: push ax
c461: mov ax, 0x3d
c464: push ax
c465: mov ax, 0x13f
c468: push ax
c469: mov ax, 0xc7
c46c: push ax
c46d: call 0x7e74                            ; EXE fill rect (colour, ...)
c470: mov ax, 0x4
c473: push ax
c474: call 0x76b                             ; EXE rng_roll(n)
c477: mov word ptr [bp - 0x8], ax
c47a: sub di, di
c47c: mov word ptr [bp - 0x4], 0xc334
c481: mov word ptr [bp - 0x6], 0xc31a
c486: mov word ptr [bp - 0x2], 0xc2c6
c48b: mov si, word ptr [bp - 0x2]
c48e: jmp 0xc50e
c490: mov al, byte ptr [si]
c492: push ax
c493: push di
c494: call 0xbbad
c497: mov byte ptr [si + 0x1], 0x64
c49b: cmp si, 0xc2fc
c49f: je 0xc4a5
c4a1: mov byte ptr [si + 0x1], 0x6e
c4a5: mov bl, byte ptr [si]
c4a7: sub bh, bh
c4a9: mov al, byte ptr [bx + 0x87e6]         ; tbl DS:87E6
c4ad: mov byte ptr [si + 0x8], al
c4b0: mov al, byte ptr [si + 0x1]
c4b3: mov cl, 0x3
c4b5: sub ah, ah
c4b7: div cl
c4b9: mov byte ptr [si + 0x2], al
c4bc: mov byte ptr [si + 0x3], al
c4bf: mov byte ptr [si + 0x4], bh
c4c2: mov bl, byte ptr [si]
c4c4: mov al, byte ptr [bx + 0x87fa]         ; tbl DS:87FA
c4c8: mov byte ptr [si + 0x7], al
c4cb: mov al, byte ptr [0xc350]
c4ce: mov byte ptr [si - 0x2], al
c4d1: sub al, al
c4d3: mov byte ptr [si + 0x5], al
c4d6: mov byte ptr [si + 0x6], al
c4d9: mov word ptr [si - 0x8], 0x4074
c4de: mov byte ptr [si + 0x9], al
c4e1: mov bx, word ptr [bp - 0x6]
c4e4: add word ptr [bp - 0x6], 0x2
c4e8: mov word ptr [bx], 0xffff
c4ec: mov al, byte ptr [bp - 0x8]
c4ef: mov cx, di
c4f1: add al, cl
c4f3: and al, 0x3
c4f5: mov cl, 0x46
c4f7: mul cl
c4f9: mov bx, word ptr [bp - 0x4]
c4fc: mov word ptr [bx], ax
c4fe: mov bx, word ptr [bp - 0x4]
c501: mov word ptr [bx + 0x2], 0x64
c506: add word ptr [bp - 0x4], 0x4
c50a: add si, 0x12
c50d: inc di
c50e: cmp si, 0xc30e
c512: jae 0xc517
c514: jmp 0xc490
c517: mov ax, word ptr [0x634a]
c51a: mov word ptr [0xaabc], ax
c51d: mov byte ptr [0xaaba], 0x0
c522: sub si, si
c524: mov word ptr [0xc366], si
c528: mov word ptr [0xaab8], si
c52c: mov word ptr [0xaab6], si
c530: or si, si
c532: je 0xc537
c534: jmp 0xc5ba
c537: mov ax, word ptr [0x634a]
c53a: sub ax, word ptr [0xc366]
c53e: mov word ptr [0xaabc], ax
c541: call 0x3512                            ; EXE panel_frame (title "<rank>" box, 30E6) (G)
c544: sub ax, ax
c546: push ax
c547: call 0x6f0c                            ; EXE province_panel(z) (6F0C)
c54a: push word ptr [0xaabc]
c54e: call 0x44e5                            ; EXE show_treasury(v)
c551: mov ax, 0x88f8
c554: push ax
c555: mov cx, 0x1
c558: push cx
c559: call 0x5614                            ; EXE menu draw (5614)
c55c: mov ax, 0x88f8
c55f: push ax
c560: sub ax, ax
c562: push ax
c563: call 0x54c8                            ; EXE menu run -> index or 0xFF
c566: cmp ax, 0xff
c569: je 0xc5a9
c56b: ja 0xc530
c56d: or al, al
c56f: je 0xc57f
c571: dec al
c573: je 0xc59f
c575: dec al
c577: je 0xc5a4
c579: dec al
c57b: je 0xc5a9
c57d: jmp 0xc530
c57f: cmp word ptr [0xc366], si
c583: jne 0xc59a
c585: cmp word ptr [0xaabc], si
c589: je 0xc59a
c58b: mov ax, 0x87ba                         ; "You Have Not Bet\nRace Anyway?"
c58e: push ax
c58f: call 0x5c20                            ; EXE yes_no box: 1 = Yes, 0 = No
c592: cmp ax, 0x1
c595: cmc
c596: sbb si, si
c598: jmp 0xc530
c59a: mov si, 0xffff
c59d: jmp 0xc530
c59f: call 0xc730
c5a2: jmp 0xc530
c5a4: call 0xc88e
c5a7: jmp 0xc530
c5a9: push word ptr [0x8814]
c5ad: call 0x5c20                            ; EXE yes_no box: 1 = Yes, 0 = No
c5b0: or ax, ax
c5b2: jne 0xc5b7
c5b4: jmp 0xc530
c5b7: jmp 0xc443
c5ba: mov ax, word ptr [0xc366]
c5bd: sub word ptr [0x634a], ax
c5c1: call 0xbe10
c5c4: lea ax, [bp - 0x1e]
c5c7: push ax
c5c8: sub ax, ax
c5ca: push ax
c5cb: push ax
c5cc: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
c5cf: mov ax, word ptr [bp - 0xa]
c5d2: mov word ptr [0x9afa], ax
c5d5: sub ax, ax
c5d7: push ax
c5d8: mov ax, 0x3d
c5db: push ax
c5dc: mov ax, 0x13f
c5df: push ax
c5e0: mov ax, 0xc7
c5e3: push ax
c5e4: call 0x7e74                            ; EXE fill rect (colour, ...)
c5e7: cmp word ptr [0x9b26], 0x2
c5ec: je 0xc604
c5ee: mov ax, 0x1
c5f1: push ax
c5f2: mov ax, 0xab
c5f5: push ax
c5f6: mov ax, 0x13e
c5f9: push ax
c5fa: mov ax, 0xc6
c5fd: push ax
c5fe: push word ptr [0x6346]
c602: jmp 0xc617
c604: mov ax, 0x1
c607: push ax
c608: mov ax, 0xab
c60b: push ax
c60c: mov ax, 0x13e
c60f: push ax
c610: mov ax, 0xc6
c613: push ax
c614: sub ax, ax
c616: push ax
c617: call 0x3134                            ; EXE ui_frame box(x1, y1, x2, y2, fill)
c61a: mov ax, 0xb
c61d: push ax
c61e: mov ax, 0xb4
c621: push ax
c622: mov ax, 0x7f
c625: push ax
c626: mov ax, 0xbd
c629: push ax
c62a: push word ptr [0x6346]
c62e: call 0x3233                            ; EXE box(colour, x1, y1, x2, y2): fill unless colour == -1, then frame lines (925C) (G)
c631: mov ax, 0xc4
c634: push ax
c635: mov cx, 0xb0
c638: push cx
c639: mov cx, 0x138
c63c: push cx
c63d: mov dx, 0xb8
c640: push dx
c641: push word ptr [0x6346]
c645: call 0x3233                            ; EXE box(colour, x1, y1, x2, y2): fill unless colour == -1, then frame lines (925C) (G)
c648: mov ax, 0xc4
c64b: push ax
c64c: mov ax, 0xb9
c64f: push ax
c650: mov ax, 0x138
c653: push ax
c654: mov ax, 0xc1
c657: push ax
c658: push word ptr [0x6346]
c65c: call 0x3233                            ; EXE box(colour, x1, y1, x2, y2): fill unless colour == -1, then frame lines (925C) (G)
c65f: mov ax, 0x9b56
c662: mov di, ax
c664: mov si, 0xa3b4
c667: push ds
c668: pop es
c669: mov cx, 0x5
c66c: rep movsw word ptr es:[di], word ptr [si]
c66e: mov word ptr [0x9b69], 0xf
c674: mov cx, 0x87d8
c677: push cx
c678: mov cx, 0x7
c67b: push cx
c67c: mov cx, 0xb5
c67f: push cx
c680: call 0x899c                            ; EXE text(str, x, y) left-aligned
c683: mov ax, 0x87da
c686: push ax
c687: mov ax, 0xbf
c68a: push ax
c68b: mov cx, 0xb1
c68e: push cx
c68f: call 0x899c                            ; EXE text(str, x, y) left-aligned
c692: mov ax, 0x87dc
c695: push ax
c696: mov ax, 0xbf
c699: push ax
c69a: mov ax, 0xba
c69d: push ax
c69e: call 0x899c                            ; EXE text(str, x, y) left-aligned
c6a1: mov ax, 0x9b56
c6a4: mov di, ax
c6a6: mov si, 0xa05c
c6a9: push ds
c6aa: pop es
c6ab: mov cx, 0x5
c6ae: rep movsw word ptr es:[di], word ptr [si]
c6b0: call 0xd400
c6b3: call 0xd4ec
c6b6: call 0xd49a
c6b9: call 0xd392
c6bc: sub ax, ax
c6be: push ax
c6bf: push ax
c6c0: mov cx, 0x13f
c6c3: push cx
c6c4: mov dx, 0xc7
c6c7: push dx
c6c8: call 0x691b                            ; EXE save rect screen -> work buffer
c6cb: mov ax, 0xa0
c6ce: push ax
c6cf: mov ax, 0x64
c6d2: push ax
c6d3: call 0x66a                             ; EXE mouse_set_pos(x, y) (int 33h fn 4)
c6d6: lea ax, [bp - 0x1e]
c6d9: push ax
c6da: call 0x86a0                            ; EXE picture_free(desc): DOS free (int 21h/49h) of the block at desc+0C
c6dd: call 0xcbe6
c6e0: call 0x67e4                            ; EXE draw into the work buffer
c6e3: mov ax, word ptr [bp - 0xa]
c6e6: mov word ptr [0x9afa], ax
c6e9: sub ax, ax
c6eb: push ax
c6ec: mov ax, 0x3d
c6ef: push ax
c6f0: mov ax, 0x13f
c6f3: push ax
c6f4: mov ax, 0xaa
c6f7: push ax
c6f8: call 0x7e74                            ; EXE fill rect (colour, ...)
c6fb: call 0x680f                            ; EXE target = the screen descriptor
c6fe: inc word ptr [0xc102]
c702: inc word ptr [0xc100]
c706: mov ax, 0xffff
c709: pop si
c70a: pop di
c70b: mov sp, bp
c70d: pop bp
c70e: ret

; ---- func C70F  size 0x21  bp frame, locals 0, ret 0x2  callers: C2DE
; C70F(desc): cancel: free sheets and pictures, unload songs 2 and 3 (D)
c70f: push bp
c710: mov bp, sp
c712: call 0xbcf4
c715: call 0xc2c2
c718: push word ptr [bp + 0x4]
c71b: call 0x86a0                            ; EXE picture_free(desc): DOS free (int 21h/49h) of the block at desc+0C
c71e: mov al, 0x2
c720: push ax
c721: call 0x5e1a                            ; EXE music_unload(n)
c724: mov al, 0x3
c726: push ax
c727: call 0x5e1a                            ; EXE music_unload(n)
c72a: mov sp, bp
c72c: pop bp
c72d: ret 0x2

; ---- func C730  size 0x15e  bp frame, locals 4, ret  callers: C2DE
; "What Skulduggery?" menu: bribe 10 (up to 3), physician 30, gods 50 (each once) (D)
c730: push bp
c731: mov bp, sp
c733: sub sp, 0x4
c736: push di
c737: push si
c738: mov ax, word ptr [0x634a]
c73b: mov word ptr [bp - 0x4], ax
c73e: sub si, si
c740: mov word ptr [bp - 0x2], si
c743: mov ax, word ptr [0xaabc]
c746: mov word ptr [0x634a], ax
c749: mov ax, 0x891c
c74c: push ax
c74d: mov ax, 0x1
c750: push ax
c751: call 0x5614                            ; EXE menu draw (5614)
c754: jmp 0xc870
c757: call 0x3512                            ; EXE panel_frame (title "<rank>" box, 30E6) (G)
c75a: sub ax, ax
c75c: push ax
c75d: call 0x6f0c                            ; EXE province_panel(z) (6F0C)
c760: mov ax, 0x891c
c763: push ax
c764: sub ax, ax
c766: push ax
c767: call 0x54c8                            ; EXE menu run -> index or 0xFF
c76a: mov di, ax
c76c: cmp ax, 0xff
c76f: jne 0xc774
c771: jmp 0xc860
c774: jbe 0xc779
c776: jmp 0xc865
c779: or al, al
c77b: je 0xc792
c77d: dec al
c77f: je 0xc7f8
c781: dec al
c783: jne 0xc788
c785: jmp 0xc82e
c788: dec al
c78a: jne 0xc78f
c78c: jmp 0xc860
c78f: jmp 0xc865
c792: cmp word ptr [0x634a], 0xa
c797: jb 0xc7f2
c799: cmp byte ptr [0xaaba], 0x3
c79e: jb 0xc7aa
c7a0: mov ax, 0x8930                         ; "All Opponents Bribed"
c7a3: push ax
c7a4: call 0x59d4                            ; EXE message_box(str)
c7a7: jmp 0xc865
c7aa: sub word ptr [0x634a], 0xa
c7af: add si, 0xa
c7b2: mov bl, byte ptr [0xaaba]
c7b6: sub bh, bh
c7b8: shl bx
c7ba: push word ptr [bx + 0x8818]            ; tbl DS:8818
c7be: mov ax, 0xac60
c7c1: push ax
c7c2: call 0x9e32                            ; EXE strcpy
c7c5: pop bx
c7c6: pop bx
c7c7: push word ptr [0x881e]
c7cb: mov ax, 0xac60
c7ce: push ax
c7cf: call 0x9df2                            ; EXE strcat
c7d2: pop bx
c7d3: pop bx
c7d4: mov ax, 0xa
c7d7: push ax
c7d8: call 0x76b                             ; EXE rng_roll(n)
c7db: mov cx, ax
c7dd: mov al, 0x12
c7df: mul byte ptr [0xaaba]
c7e3: mov bx, ax
c7e5: sub byte ptr [bx + 0xc2c7], cl         ; tbl DS:C2C7
c7e9: inc byte ptr [0xaaba]
c7ed: mov ax, 0xac60
c7f0: jmp 0xc7a3
c7f2: push word ptr [0x880a]
c7f6: jmp 0xc7a4
c7f8: cmp word ptr [0x634a], 0x1e
c7fd: jb 0xc7f2
c7ff: cmp word ptr [0xaab6], 0x0
c804: je 0xc80c
c806: push word ptr [0x8824]
c80a: jmp 0xc7a4
c80c: mov word ptr [0xaab6], 0xffff
c812: sub word ptr [0x634a], 0x1e
c817: add si, 0x1e
c81a: push word ptr [0x8820]
c81e: call 0x59d4                            ; EXE message_box(str)
c821: mov ax, 0xa
c824: push ax
c825: call 0x76b                             ; EXE rng_roll(n)
c828: add byte ptr [0xc2fd], al
c82c: jmp 0xc865
c82e: cmp word ptr [0x634a], 0x32
c833: jb 0xc7f2
c835: cmp word ptr [0xaab8], 0x0
c83a: jne 0xc806
c83c: mov word ptr [0xaab8], 0xffff
c842: sub word ptr [0x634a], 0x32
c847: add si, 0x32
c84a: push word ptr [0x8822]
c84e: call 0x59d4                            ; EXE message_box(str)
c851: mov ax, 0xa
c854: push ax
c855: call 0x76b                             ; EXE rng_roll(n)
c858: add al, 0xa
c85a: add byte ptr [0xc304], al
c85e: jmp 0xc865
c860: mov word ptr [bp - 0x2], 0xffff
c865: mov ax, 0x891c
c868: push ax
c869: push di
c86a: sub ax, ax
c86c: push ax
c86d: call 0x585b                            ; EXE menu_preselect(menu, sel, -1)
c870: cmp word ptr [bp - 0x2], 0x0
c874: jne 0xc879
c876: jmp 0xc757
c879: mov ax, word ptr [0x634a]
c87c: mov word ptr [0xaabc], ax
c87f: sub si, word ptr [bp - 0x4]
c882: neg si
c884: mov word ptr [0x634a], si
c888: pop si
c889: pop di
c88a: mov sp, bp
c88c: pop bp
c88d: ret

; ---- func C88E  size 0x128  bp frame, locals 82, ret  callers: C2DE
; "What Will You Bet?" menu: +10, -10, limit = min(treasury, 100), odds shown (D)
c88e: push bp
c88f: mov bp, sp
c891: sub sp, 0x52
c894: push di
c895: push si
c896: sub di, di
c898: mov word ptr [bp - 0x2], di
c89b: mov ax, 0x890a
c89e: push ax
c89f: mov ax, 0x1
c8a2: push ax
c8a3: call 0x5614                            ; EXE menu draw (5614)
c8a6: jmp 0xc9a7
c8a9: mov ax, word ptr [0x634a]
c8ac: sub ax, word ptr [0xc366]
c8b0: mov word ptr [0xaabc], ax
c8b3: push ax
c8b4: call 0x44e5                            ; EXE show_treasury(v)
c8b7: mov ax, 0x8945                         ; "Your Bet: "
c8ba: push ax
c8bb: lea ax, [bp - 0x52]
c8be: push ax
c8bf: call 0x9e32                            ; EXE strcpy
c8c2: pop bx
c8c3: pop bx
c8c4: mov ax, 0xa
c8c7: push ax
c8c8: mov cx, 0xac60
c8cb: push cx
c8cc: push word ptr [0xc366]
c8d0: call 0x9fdc                            ; EXE itoa(v, buf, 10)
c8d3: add sp, 0x6
c8d6: push ax
c8d7: lea ax, [bp - 0x52]
c8da: push ax
c8db: call 0x9df2                            ; EXE strcat
c8de: pop bx
c8df: pop bx
c8e0: mov ax, 0x8950                         ; " Talents\nOdds: "
c8e3: push ax
c8e4: lea ax, [bp - 0x52]
c8e7: push ax
c8e8: call 0x9df2                            ; EXE strcat
c8eb: pop bx
c8ec: pop bx
c8ed: mov ax, 0xa
c8f0: push ax
c8f1: mov cx, 0xac60
c8f4: push cx
c8f5: mov bl, byte ptr [0xc2fc]
c8f9: sub bh, bh
c8fb: mov cl, byte ptr [bx + 0x87ee]         ; tbl DS:87EE
c8ff: sub ch, ch
c901: push cx
c902: call 0x9fdc                            ; EXE itoa(v, buf, 10)
c905: add sp, 0x6
c908: push ax
c909: lea ax, [bp - 0x52]
c90c: push ax
c90d: call 0x9df2                            ; EXE strcat
c910: pop bx
c911: pop bx
c912: mov ax, 0x8960                         ; " to 1"
c915: push ax
c916: lea ax, [bp - 0x52]
c919: push ax
c91a: call 0x9df2                            ; EXE strcat
c91d: pop bx
c91e: pop bx
c91f: mov ax, 0xa
c922: push ax
c923: mov ax, 0x96
c926: push ax
c927: mov cx, 0x91
c92a: push cx
c92b: mov cx, 0xb4
c92e: push cx
c92f: call 0x7e74                            ; EXE fill rect (colour, ...)
c932: lea ax, [bp - 0x52]
c935: push ax
c936: mov ax, 0x4e
c939: push ax
c93a: mov ax, 0x96
c93d: push ax
c93e: call 0x8be4                            ; EXE centred_text(str, cx, y)
c941: mov si, word ptr [0x634a]
c945: cmp si, 0x64
c948: jbe 0xc94d
c94a: mov si, 0x64
c94d: mov ax, 0x890a
c950: push ax
c951: push di
c952: call 0x54c8                            ; EXE menu run -> index or 0xFF
c955: mov di, ax
c957: cmp ax, 0xff
c95a: je 0xc997
c95c: ja 0xc99c
c95e: or al, al
c960: je 0xc970
c962: dec al
c964: je 0xc981
c966: dec al
c968: je 0xc991
c96a: dec al
c96c: je 0xc997
c96e: jmp 0xc99c
c970: push word ptr [0xc366]
c974: mov ax, 0xa
c977: push ax
c978: push si
c979: call 0x4fa                             ; EXE add_cap: min(v + step, max), signed (04FA/050F pair)
c97c: mov word ptr [0xc366], ax
c97f: jmp 0xc99c
c981: push word ptr [0xc366]
c985: mov ax, 0xa
c988: push ax
c989: sub ax, ax
c98b: push ax
c98c: call 0x50f                             ; EXE step_toward_min: max(v - step, min), signed (050F family)
c98f: jmp 0xc97c
c991: mov word ptr [0xc366], si
c995: jmp 0xc99c
c997: mov word ptr [bp - 0x2], 0xffff
c99c: mov ax, 0x890a
c99f: push ax
c9a0: push di
c9a1: sub ax, ax
c9a3: push ax
c9a4: call 0x585b                            ; EXE menu_preselect(menu, sel, -1)
c9a7: cmp word ptr [bp - 0x2], 0x0
c9ab: jne 0xc9b0
c9ad: jmp 0xc8a9
c9b0: pop si
c9b1: pop di
c9b2: mov sp, bp
c9b4: pop bp
c9b5: ret

; ---- func C9B6  size 0x230  bp frame, locals 22, ret  callers: D08A
; race result: finishing order, message, score, winnings (odds+1)*bet, odds update, AI skill 8BA6 by place and difficulty, winner's province (D)
c9b6: push bp
c9b7: mov bp, sp
c9b9: sub sp, 0x16
c9bc: push di
c9bd: push si
c9be: sub dx, dx
c9c0: lea bx, [bp - 0x12]
c9c3: jmp 0xc9cb
c9c5: inc bx
c9c6: inc bx
c9c7: mov word ptr [bx - 0x2], dx
c9ca: inc dx
c9cb: lea ax, [bp - 0xa]
c9ce: cmp bx, ax
c9d0: jb 0xc9c5
c9d2: mov dx, 0x1
c9d5: lea bx, [bp - 0x12]
c9d8: jmp 0xca8d
c9db: mov word ptr [bp - 0x6], dx
c9de: mov word ptr [bp - 0x8], bx
c9e1: mov word ptr [bp - 0x4], 0x3
c9e6: lea di, [bp - 0xc]
c9e9: jmp 0xca7a
c9ec: mov word ptr [bp - 0xa], 0x0
c9f1: mov bx, word ptr [bp - 0x8]
c9f4: mov bx, word ptr [bx]
c9f6: mov ax, word ptr [di]
c9f8: mov word ptr [bp - 0x2], ax
c9fb: mov si, ax
c9fd: shl si
c9ff: mov ax, word ptr [si + 0xc31a]         ; tbl DS:C31A
ca03: mov si, bx
ca05: shl si
ca07: cmp word ptr [si + 0xc31a], ax         ; tbl DS:C31A
ca0b: ja 0xca58
ca0d: mov si, bx
ca0f: shl si
ca11: mov ax, word ptr [si + 0xc31a]         ; tbl DS:C31A
ca15: mov si, word ptr [bp - 0x2]
ca18: shl si
ca1a: cmp word ptr [si + 0xc31a], ax         ; tbl DS:C31A
ca1e: jne 0xca5d
ca20: mov ax, bx
ca22: mov cx, 0x12
ca25: mul cx
ca27: mov si, ax
ca29: mov ax, word ptr [si + 0xc2be]         ; tbl DS:C2BE
ca2d: mov word ptr [bp - 0x16], ax
ca30: mov dx, ax
ca32: mov ax, cx
ca34: mul word ptr [bp - 0x2]
ca37: mov si, ax
ca39: mov ax, word ptr [si + 0xc2be]         ; tbl DS:C2BE
ca3d: mov word ptr [bp - 0x14], ax
ca40: cmp ax, word ptr [bp - 0x16]
ca43: ja 0xca58
ca45: mov ax, word ptr [bp - 0x16]
ca48: cmp ax, word ptr [bp - 0x14]
ca4b: jne 0xca5d
ca4d: mov ax, 0x1
ca50: push ax
ca51: call 0x76b                             ; EXE rng_roll(n)
ca54: or ax, ax
ca56: jne 0xca5d
ca58: mov word ptr [bp - 0xa], 0xffff
ca5d: cmp word ptr [bp - 0xa], 0x0
ca61: je 0xca75
ca63: mov ax, word ptr [di]
ca65: mov bx, word ptr [bp - 0x8]
ca68: xor word ptr [bx], ax
ca6a: mov ax, word ptr [bx]
ca6c: xor word ptr [di], ax
ca6e: mov ax, word ptr [di]
ca70: mov bx, word ptr [bp - 0x8]
ca73: xor word ptr [bx], ax
ca75: dec di
ca76: dec di
ca77: dec word ptr [bp - 0x4]
ca7a: mov ax, word ptr [bp - 0x6]
ca7d: cmp word ptr [bp - 0x4], ax
ca80: jb 0xca85
ca82: jmp 0xc9ec
ca85: mov bx, word ptr [bp - 0x8]
ca88: inc bx
ca89: inc bx
ca8a: mov dx, ax
ca8c: inc dx
ca8d: lea ax, [bp - 0xc]
ca90: cmp bx, ax
ca92: jae 0xca97
ca94: jmp 0xc9db
ca97: sub dx, dx
ca99: lea bx, [bp - 0x12]
ca9c: mov di, word ptr [bp - 0x2]
ca9f: jmp 0xcaab
caa1: cmp word ptr [bx], 0x3
caa4: jne 0xcaa8
caa6: mov di, dx
caa8: inc bx
caa9: inc bx
caaa: inc dx
caab: lea ax, [bp - 0xa]
caae: cmp bx, ax
cab0: jb 0xcaa1
cab2: cmp word ptr [0xc320], -0x1
cab7: jne 0xcabc
cab9: jmp 0xcb9c
cabc: mov bx, di
cabe: push word ptr [bx + di + 0x880c]       ; tbl DS:880C
cac2: call 0x59d4                            ; EXE message_box(str)
cac5: mov ax, di
cac7: or ax, ax
cac9: je 0xcad7
cacb: dec ax
cacc: je 0xcb38
cace: dec ax
cacf: je 0xcb41
cad1: dec ax
cad2: je 0xcb48
cad4: jmp 0xcb59
cad7: mov ax, 0xa
cada: push ax
cadb: call 0x7330                            ; EXE score_add(n)
cade: mov al, byte ptr [0x634e]
cae1: sub ah, ah
cae3: push ax
cae4: mov ax, 0x1
cae7: push ax
cae8: mov ax, 0x64
caeb: push ax
caec: call 0x4fa                             ; EXE add_cap: min(v + step, max), signed (04FA/050F pair)
caef: mov byte ptr [0x634e], al
caf2: mov bl, byte ptr [0xc2fc]
caf6: sub bh, bh
caf8: mov al, byte ptr [bx + 0x87ee]         ; tbl DS:87EE
cafc: mov cx, ax
cafe: sub ah, ah
cb00: mov dx, ax
cb02: inc ax
cb03: mov bx, dx
cb05: mul word ptr [0xc366]
cb09: add word ptr [0x634a], ax
cb0d: lea ax, [bx + 0x1]
cb10: imul word ptr [0xc366]
cb14: cdq
cb15: add word ptr [0xc096], ax
cb19: adc word ptr [0xc098], dx
cb1d: cmp cl, 0x1
cb20: jbe 0xcb2c
cb22: mov bl, byte ptr [0xc2fc]
cb26: sub bh, bh
cb28: dec byte ptr [bx + 0x87ee]             ; tbl DS:87EE
cb2c: inc word ptr [0xc104]
cb30: sub al, al
cb32: push ax
cb33: call 0xbf6a
cb36: jmp 0xcb59
cb38: mov ax, 0x5
cb3b: push ax
cb3c: call 0x7330                            ; EXE score_add(n)
cb3f: jmp 0xcb59
cb41: mov ax, 0x1
cb44: push ax
cb45: call 0x7330                            ; EXE score_add(n)
cb48: mov bl, byte ptr [0xc2fc]
cb4c: sub bh, bh
cb4e: cmp byte ptr [bx + 0x87ee], 0x5        ; tbl DS:87EE
cb53: jae 0xcb59
cb55: inc byte ptr [bx + 0x87ee]             ; tbl DS:87EE
cb59: mov al, byte ptr [di + 0x8966]         ; tbl DS:8966
cb5d: add byte ptr [0x8ba6], al
cb61: mov al, byte ptr [0x48f]
cb64: sub ah, ah
cb66: or ax, ax
cb68: je 0xcb72
cb6a: dec ax
cb6b: je 0xcb79
cb6d: dec ax
cb6e: je 0xcb80
cb70: jmp 0xcb85
cb72: sub byte ptr [0x8ba6], 0x3
cb77: jmp 0xcb85
cb79: add byte ptr [0x8ba6], 0x3
cb7e: jmp 0xcb85
cb80: add byte ptr [0x8ba6], 0x6
cb85: mov al, byte ptr [0x8ba6]
cb88: cmp al, 0x5f
cb8a: jle 0xcb8e
cb8c: mov al, 0x5f
cb8e: mov byte ptr [0x8ba6], al
cb91: cmp al, 0x4b
cb93: jge 0xcb97
cb95: mov al, 0x4b
cb97: mov byte ptr [0x8ba6], al
cb9a: jmp 0xcbb4
cb9c: push word ptr [0x8816]
cba0: call 0x59d4                            ; EXE message_box(str)
cba3: mov bl, byte ptr [0xc2fc]
cba7: sub bh, bh
cba9: cmp byte ptr [bx + 0x87ee], 0x5        ; tbl DS:87EE
cbae: jae 0xcbb4
cbb0: inc byte ptr [bx + 0x87ee]             ; tbl DS:87EE
cbb4: cmp word ptr [bp - 0x12], 0x3
cbb8: je 0xcbe0
cbba: mov ax, 0x12
cbbd: mul word ptr [bp - 0x12]
cbc0: mov bx, ax
cbc2: mov bl, byte ptr [bx + 0xc2c5]         ; tbl DS:C2C5
cbc6: sub bh, bh
cbc8: mov cl, 0x5
cbca: shl bx, cl
cbcc: mov al, byte ptr [bx + 0x527e]         ; tbl DS:527E
cbd0: cwde
cbd1: sub ax, 0xa
cbd4: cmp ax, 0x1
cbd7: jge 0xcbdc
cbd9: mov ax, 0x1
cbdc: mov byte ptr [bx + 0x527e], al         ; tbl DS:527E
cbe0: pop si
cbe1: pop di
cbe2: mov sp, bp
cbe4: pop bp
cbe5: ret

; ---- func CBE6  size 0x1e8  bp frame, locals 14, ret  callers: C2DE
; build the race path: calloc(368, 4) -> C308, filled from the polyline DS:896A..8A89 (D)
cbe6: push bp
cbe7: mov bp, sp
cbe9: sub sp, 0xe
cbec: push di
cbed: push si
cbee: mov ax, 0x170
cbf1: push ax
cbf2: mov ax, 0x4
cbf5: push ax
cbf6: call 0x7488                            ; EXE calloc(n, size) -> DS-near pointer (fatal "calloc" on failure)
cbf9: mov word ptr [0xc308], ax
cbfc: mov word ptr [bp - 0x6], 0x0
cc01: mov ax, word ptr [0x896a]
cc04: inc ax
cc05: mov word ptr [bp - 0x8], ax
cc08: mov ax, word ptr [0x896c]
cc0b: mov word ptr [bp - 0xc], ax
cc0e: mov word ptr [bp - 0xa], 0x896a
cc13: mov bx, word ptr [bp - 0xa]
cc16: jmp 0xcd9c
cc19: cmp word ptr [bx], -0x1
cc1c: jne 0xcc69
cc1e: mov si, word ptr [bp - 0x6]
cc21: shl si
cc23: shl si
cc25: mov ax, si
cc27: add si, word ptr [0xc308]
cc2b: mov word ptr [si], 0xffff
cc2f: mov si, word ptr [0xc308]
cc33: add si, ax
cc35: mov word ptr [si + 0x2], 0xffff
cc3a: inc word ptr [bp - 0x6]
cc3d: mov si, word ptr [bp - 0x6]
cc40: shl si
cc42: shl si
cc44: mov ax, si
cc46: add si, word ptr [0xc308]
cc4a: mov word ptr [si], 0xffff
cc4e: mov si, word ptr [0xc308]
cc52: add si, ax
cc54: mov word ptr [si + 0x2], 0xffff
cc59: mov ax, word ptr [bp - 0x6]
cc5c: inc ax
cc5d: mov word ptr [bp - 0x6], ax
cc60: mov dx, word ptr [bx + 0x4]
cc63: mov ax, word ptr [bx + 0x6]
cc66: jmp 0xcd93
cc69: mov ax, word ptr [bp - 0x8]
cc6c: sub ax, word ptr [bx]
cc6e: cdq
cc6f: xor ax, dx
cc71: sub ax, dx
cc73: cmp ax, 0x1
cc76: jle 0xccee
cc78: mov ax, word ptr [bp - 0x8]
cc7b: cmp word ptr [bx], ax
cc7d: jge 0xccbb
cc7f: mov word ptr [bp - 0x2], ax
cc82: mov ax, word ptr [bp - 0x6]
cc85: shl ax
cc87: shl ax
cc89: mov word ptr [bp - 0x4], ax
cc8c: mov ax, word ptr [bx]
cc8e: cmp word ptr [bp - 0x2], ax
cc91: jge 0xcc96
cc93: jmp 0xcd8e
cc96: mov ax, word ptr [bp - 0x2]
cc99: mov si, word ptr [0xc308]
cc9d: add si, word ptr [bp - 0x4]
cca0: mov word ptr [si], ax
cca2: mov ax, word ptr [bx + 0x2]
cca5: mov si, word ptr [0xc308]
cca9: add si, word ptr [bp - 0x4]
ccac: mov word ptr [si + 0x2], ax
ccaf: add word ptr [bp - 0x4], 0x4
ccb3: inc word ptr [bp - 0x6]
ccb6: dec word ptr [bp - 0x2]
ccb9: jmp 0xcc8c
ccbb: mov dx, ax
ccbd: mov ax, word ptr [bp - 0x6]
ccc0: shl ax
ccc2: shl ax
ccc4: mov word ptr [bp - 0x4], ax
ccc7: cmp word ptr [bx], dx
ccc9: jge 0xccce
cccb: jmp 0xcd8e
ccce: mov si, word ptr [0xc308]
ccd2: add si, word ptr [bp - 0x4]
ccd5: mov word ptr [si], dx
ccd7: mov ax, word ptr [bx + 0x2]
ccda: mov si, word ptr [0xc308]
ccde: add si, word ptr [bp - 0x4]
cce1: mov word ptr [si + 0x2], ax
cce4: add word ptr [bp - 0x4], 0x4
cce8: inc word ptr [bp - 0x6]
cceb: inc dx
ccec: jmp 0xccc7
ccee: mov di, word ptr [bp - 0xc]
ccf1: mov ax, di
ccf3: sub ax, word ptr [bx + 0x2]
ccf6: cdq
ccf7: xor ax, dx
ccf9: sub ax, dx
ccfb: cmp ax, 0x1
ccfe: jle 0xcd6e
cd00: cmp word ptr [bx + 0x2], di
cd03: jge 0xcd3e
cd05: mov word ptr [bp - 0x2], di
cd08: mov ax, word ptr [bp - 0x6]
cd0b: shl ax
cd0d: shl ax
cd0f: mov word ptr [bp - 0x4], ax
cd12: mov ax, word ptr [bp - 0x2]
cd15: cmp word ptr [bx + 0x2], ax
cd18: jg 0xcd8e
cd1a: mov ax, word ptr [bx]
cd1c: mov si, word ptr [0xc308]
cd20: add si, word ptr [bp - 0x4]
cd23: mov word ptr [si], ax
cd25: mov ax, word ptr [bp - 0x2]
cd28: mov si, word ptr [0xc308]
cd2c: add si, word ptr [bp - 0x4]
cd2f: mov word ptr [si + 0x2], ax
cd32: add word ptr [bp - 0x4], 0x4
cd36: inc word ptr [bp - 0x6]
cd39: dec word ptr [bp - 0x2]
cd3c: jmp 0xcd12
cd3e: mov dx, di
cd40: mov ax, word ptr [bp - 0x6]
cd43: shl ax
cd45: shl ax
cd47: mov word ptr [bp - 0x4], ax
cd4a: cmp word ptr [bx + 0x2], dx
cd4d: jl 0xcd8e
cd4f: mov ax, word ptr [bx]
cd51: mov si, word ptr [0xc308]
cd55: add si, word ptr [bp - 0x4]
cd58: mov word ptr [si], ax
cd5a: mov si, word ptr [0xc308]
cd5e: add si, word ptr [bp - 0x4]
cd61: mov word ptr [si + 0x2], dx
cd64: add word ptr [bp - 0x4], 0x4
cd68: inc word ptr [bp - 0x6]
cd6b: inc dx
cd6c: jmp 0xcd4a
cd6e: mov ax, word ptr [bx]
cd70: mov si, word ptr [bp - 0x6]
cd73: shl si
cd75: shl si
cd77: mov cx, si
cd79: add si, word ptr [0xc308]
cd7d: mov word ptr [si], ax
cd7f: mov ax, word ptr [bx + 0x2]
cd82: mov si, word ptr [0xc308]
cd86: add si, cx
cd88: mov word ptr [si + 0x2], ax
cd8b: inc word ptr [bp - 0x6]
cd8e: mov dx, word ptr [bx]
cd90: mov ax, word ptr [bx + 0x2]
cd93: mov word ptr [bp - 0xc], ax
cd96: add bx, 0x4
cd99: mov word ptr [bp - 0x8], dx
cd9c: cmp bx, 0x8a8a
cda0: jae 0xcda5
cda2: jmp 0xcc19
cda5: mov cx, 0xc30a
cda8: jmp 0xcdc2
cdaa: sub dx, dx
cdac: mov bx, cx
cdae: mov word ptr [bp - 0x4], cx
cdb1: jmp 0xcdba
cdb3: mov word ptr [bx], 0xdc
cdb7: inc bx
cdb8: inc bx
cdb9: inc dx
cdba: cmp dx, 0x2
cdbd: jl 0xcdb3
cdbf: add cx, 0x4
cdc2: cmp cx, 0xc31a
cdc6: jb 0xcdaa
cdc8: pop si
cdc9: pop di
cdca: mov sp, bp
cdcc: pop bp
cdcd: ret

; ---- func CDCE  size 0xfe  bp frame, locals 6, ret 0x2  callers: C2DE
; CDCE(i): racer card i: box, home province, strategy (opponents), chariot type and sprite (D)
cdce: push bp
cdcf: mov bp, sp
cdd1: sub sp, 0x6
cdd4: push di
cdd5: push si
cdd6: mov si, word ptr [bp + 0x4]
cdd9: mov bx, si
cddb: shl bx
cddd: shl bx
cddf: push word ptr [bx + 0x8a8a]            ; tbl DS:8A8A
cde3: push word ptr [bx + 0x8a8c]            ; tbl DS:8A8C
cde7: mov ax, word ptr [bx + 0x8a8a]         ; tbl DS:8A8A
cdeb: add ax, 0x64
cdee: push ax
cdef: mov ax, word ptr [bx + 0x8a8c]         ; tbl DS:8A8C
cdf3: add ax, 0x50
cdf6: push ax
cdf7: push word ptr [0x6346]
cdfb: mov di, bx
cdfd: call 0x3134                            ; EXE ui_frame box(x1, y1, x2, y2, fill)
ce00: mov ax, di
ce02: mov bx, di
ce04: mov cx, word ptr [bx + 0x8a8a]         ; tbl DS:8A8A
ce08: add cx, 0x32
ce0b: mov di, cx
ce0d: mov dl, byte ptr [0x9b5d]
ce11: sub dh, dh
ce13: mov al, byte ptr [0x9b5c]
ce16: sub ah, ah
ce18: add ax, dx
ce1a: add ax, 0x6
ce1d: mov word ptr [bp - 0x2], ax
ce20: mov ax, bx
ce22: mov bx, si
ce24: mov dx, word ptr [bx + si + 0x87de]    ; tbl DS:87DE
ce28: mov word ptr [0x9b69], dx
ce2c: mov dx, ax
ce2e: mov ax, bx
ce30: mov bx, 0x12
ce33: mov word ptr [bp - 0x6], dx
ce36: mul bx
ce38: mov bx, ax
ce3a: mov word ptr [bp - 0x4], bx
ce3d: mov bl, byte ptr [bx + 0xc2c5]         ; tbl DS:C2C5
ce41: sub bh, bh
ce43: shl bx
ce45: push word ptr [bx + 0x5222]            ; tbl DS:5222
ce49: push cx
ce4a: mov bx, word ptr [bp - 0x6]
ce4d: mov ax, word ptr [bx + 0x8a8c]         ; tbl DS:8A8C
ce51: add ax, 0x6
ce54: push ax
ce55: call 0x8be4                            ; EXE centred_text(str, cx, y)
ce58: cmp si, 0x3
ce5b: je 0xce7b
ce5d: mov bl, byte ptr [si + 0xc368]         ; tbl DS:C368
ce61: sub bh, bh
ce63: shl bx
ce65: push word ptr [bx + 0x8804]            ; tbl DS:8804
ce69: push di
ce6a: mov bx, si
ce6c: shl bx
ce6e: shl bx
ce70: mov ax, word ptr [bx + 0x8a8c]         ; tbl DS:8A8C
ce74: add ax, word ptr [bp - 0x2]
ce77: push ax
ce78: call 0x8be4                            ; EXE centred_text(str, cx, y)
ce7b: mov bx, word ptr [bp - 0x4]
ce7e: mov bl, byte ptr [bx + 0xc2c6]         ; tbl DS:C2C6
ce82: sub bh, bh
ce84: shl bx
ce86: push word ptr [bx + 0x87fe]            ; tbl DS:87FE
ce8a: push di
ce8b: shl si
ce8d: shl si
ce8f: mov ax, word ptr [si + 0x8a8c]         ; tbl DS:8A8C
ce93: sub ax, word ptr [bp - 0x2]
ce96: add ax, 0x50
ce99: push ax
ce9a: call 0x8be4                            ; EXE centred_text(str, cx, y)
ce9d: mov bx, word ptr [bp - 0x4]
cea0: mov bl, byte ptr [bx + 0xc2c6]         ; tbl DS:C2C6
cea4: sub bh, bh
cea6: mov al, 0xe
cea8: mul byte ptr [bx + 0x87f6]             ; tbl DS:87F6
ceac: add ax, word ptr [0xc2bc]
ceb0: push ax
ceb1: mov ax, word ptr [si + 0x8a8a]         ; tbl DS:8A8A
ceb5: add ax, 0x21
ceb8: push ax
ceb9: mov ax, word ptr [si + 0x8a8c]         ; tbl DS:8A8C
cebd: add ax, 0x24
cec0: push ax
cec1: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
cec4: pop si
cec5: pop di
cec6: mov sp, bp
cec8: pop bp
cec9: ret 0x2

; ---- func CECC  size 0x1bd  bp frame, locals 6, ret  callers: D08A
; player input: keys, mouse (re-centred at 160,100) or joystick [A2] -> command C302 (8..11, 1..3) (D/G)
cecc: push bp
cecd: mov bp, sp
cecf: sub sp, 0x6
ced2: push di
ced3: push si
ced4: mov word ptr [bp - 0x6], 0x0
ced9: call 0x3aa                             ; EXE key_read: next key or 0
cedc: mov byte ptr [bp - 0x3], al
cedf: call 0x4ed                             ; EXE key_flush
cee2: cmp word ptr [0xc2f8], 0x0
cee7: je 0xcef7
cee9: cmp byte ptr [0xc301], 0xb
ceee: je 0xcef7
cef0: cmp byte ptr [0xc302], 0xb
cef5: jne 0xcefa
cef7: jmp 0xd083
cefa: cmp word ptr [0xba], 0x0
ceff: je 0xcf76
cf01: mov byte ptr [0x8a9e], 0x0
cf06: call 0x644                             ; EXE mouse_poll: [B2] buttons, [B6] x, [B8] y
cf09: mov di, word ptr [0x8a9a]
cf0d: sub di, word ptr [0xb6]
cf11: mov si, word ptr [0x8a9c]
cf15: sub si, word ptr [0xb8]
cf19: cmp word ptr [0xb6], 0x14
cf1e: jl 0xcf37
cf20: cmp word ptr [0xb6], 0x12c
cf26: jg 0xcf37
cf28: cmp word ptr [0xb8], 0x14
cf2d: jl 0xcf37
cf2f: cmp word ptr [0xb8], 0xb4
cf35: jle 0xcf4e
cf37: mov ax, 0xa0
cf3a: push ax
cf3b: mov ax, 0x64
cf3e: push ax
cf3f: call 0x66a                             ; EXE mouse_set_pos(x, y) (int 33h fn 4)
cf42: mov word ptr [0x8a9a], 0xa0
cf48: mov word ptr [0x8a9c], 0x64
cf4e: mov ax, si
cf50: cdq
cf51: xor ax, dx
cf53: sub ax, dx
cf55: mov word ptr [bp - 0x2], ax
cf58: mov ax, di
cf5a: cdq
cf5b: xor ax, dx
cf5d: sub ax, dx
cf5f: mov cx, ax
cf61: cmp word ptr [bp - 0x2], cx
cf64: jge 0xcf86
cf66: cmp cx, 0x2
cf69: jle 0xcfa0
cf6b: or di, di
cf6d: jge 0xcf7b
cf6f: or byte ptr [0x8a9e], 0x1
cf74: jmp 0xcfa0
cf76: mov si, word ptr [bp - 0x6]
cf79: jmp 0xcfe4
cf7b: or di, di
cf7d: jle 0xcfa0
cf7f: or byte ptr [0x8a9e], 0x2
cf84: jmp 0xcfa0
cf86: cmp word ptr [bp - 0x2], 0x2
cf8a: jle 0xcfa0
cf8c: or si, si
cf8e: jge 0xcf97
cf90: or byte ptr [0x8a9e], 0x8
cf95: jmp 0xcfa0
cf97: or si, si
cf99: jle 0xcfa0
cf9b: or byte ptr [0x8a9e], 0x4
cfa0: cmp word ptr [0xb2], 0x0
cfa5: je 0xcfba
cfa7: mov al, byte ptr [0xb2]
cfaa: and al, 0x2
cfac: je 0xcfbf
cfae: test byte ptr [0xb2], 0x1
cfb3: je 0xcfbf
cfb5: mov si, 0x1
cfb8: jmp 0xcfd8
cfba: mov si, word ptr [bp - 0x6]
cfbd: jmp 0xcfd8
cfbf: mov al, byte ptr [0xb2]
cfc2: and al, 0x2
cfc4: je 0xcfcb
cfc6: mov si, 0x3
cfc9: jmp 0xcfd8
cfcb: mov si, word ptr [bp - 0x6]
cfce: test byte ptr [0xb2], 0x1
cfd3: je 0xcfd8
cfd5: mov si, 0x2
cfd8: mov ax, word ptr [0xb6]
cfdb: mov word ptr [0x8a9a], ax
cfde: mov ax, word ptr [0xb8]
cfe1: mov word ptr [0x8a9c], ax
cfe4: cmp word ptr [0xba], 0x0
cfe9: jne 0xcff6
cfeb: cmp byte ptr [bp - 0x3], 0x0
cfef: jne 0xcff6
cff1: mov byte ptr [0x8a9e], 0x0
cff6: cmp byte ptr [0xa2], 0x0
cffb: je 0xd04f
cffd: test byte ptr [0xa2], 0x8
d002: je 0xd029
d004: or byte ptr [0x8a9e], 0x4
d009: cmp byte ptr [0xc301], 0x0
d00e: jne 0xd04f
d010: test byte ptr [0xa2], 0x1
d015: je 0xd01c
d017: mov si, 0x3
d01a: jmp 0xd04f
d01c: mov al, byte ptr [0xa2]
d01f: and al, 0x2
d021: cmp al, 0x1
d023: sbb si, si
d025: inc si
d026: inc si
d027: jmp 0xd04f
d029: test byte ptr [0xa2], 0x4
d02e: je 0xd035
d030: or byte ptr [0x8a9e], 0x8
d035: test byte ptr [0xa2], 0x2
d03a: je 0xd043
d03c: or byte ptr [0x8a9e], 0x2
d041: jmp 0xd04f
d043: test byte ptr [0xa2], 0x1
d048: je 0xd04f
d04a: or byte ptr [0x8a9e], 0x1
d04f: call 0x4ed                             ; EXE key_flush
d052: test byte ptr [0x8a9e], 0x2
d057: je 0xd060
d059: mov byte ptr [0xc302], 0x9
d05e: jmp 0xd07a
d060: test byte ptr [0x8a9e], 0x1
d065: je 0xd06e
d067: mov byte ptr [0xc302], 0xa
d06c: jmp 0xd07a
d06e: test byte ptr [0x8a9e], 0x8
d073: je 0xd07a
d075: mov byte ptr [0xc302], 0x8
d07a: or si, si
d07c: je 0xd083
d07e: mov ax, si
d080: mov byte ptr [0xc302], al
d083: pop si
d084: pop di
d085: mov sp, bp
d087: pop bp
d088: ret
d089: db 0x00                                ; word-alignment pad

; ---- func D08A  size 0xf0  no bp frame, ret
; ENTRY (EXE thunk B0AA): the chariot race (D)
d08a: push di
d08b: push si
d08c: mov byte ptr [0xc350], -0x1
d091: call 0xb0e0
d094: call 0xc2de
d097: or ax, ax
d099: jne 0xd09e
d09b: jmp 0xd177
d09e: mov ax, word ptr [0x634c]
d0a1: mov word ptr [0x154e], ax
d0a4: sub ax, ax
d0a6: mov word ptr [0xc32e], ax
d0a9: mov word ptr [0xc36c], ax
d0ac: mov word ptr [0xc330], ax
d0af: mov word ptr [0xc332], ax
d0b2: mov word ptr [0xc32c], ax
d0b5: mov al, 0x3
d0b7: push ax
d0b8: call 0x5d29                            ; EXE music_play(n)
d0bb: call 0xf8                              ; EXE timer0_reset (= 00D7(0))
d0be: mov ax, 0x12
d0c1: push ax
d0c2: call 0x11f                             ; EXE frame_wait(n): busy-wait until timer 0 + n ticks
d0c5: jmp 0xd144
d0c7: call 0xf8                              ; EXE timer0_reset (= 00D7(0))
d0ca: call 0xdaba
d0cd: call 0xcecc
d0d0: call 0xd17a
d0d3: call 0xb190
d0d6: mov di, 0xc2ce
d0d9: mov si, 0xc324
d0dc: jmp 0xd118
d0de: mov ax, word ptr [si]
d0e0: add ax, 0x96
d0e3: cmp ax, word ptr [0xc322]
d0e7: ja 0xd113
d0e9: mov al, byte ptr [di]
d0eb: cwde
d0ec: push ax
d0ed: mov ax, 0x1
d0f0: push ax
d0f1: mov bl, byte ptr [di - 0x8]
d0f4: sub bh, bh
d0f6: mov al, byte ptr [bx + 0x87e6]         ; tbl DS:87E6
d0fa: sub ah, ah
d0fc: push ax
d0fd: call 0x4fa                             ; EXE add_cap: min(v + step, max), signed (04FA/050F pair)
d100: mov byte ptr [di], al
d102: mov ax, word ptr [0xc322]
d105: mov word ptr [si], ax
d107: cmp si, 0xc32a
d10b: jne 0xd113
d10d: mov word ptr [0xc330], 0xffff
d113: add di, 0x12
d116: inc si
d117: inc si
d118: cmp si, 0xc32c
d11c: jb 0xd0de
d11e: cmp word ptr [0x9b26], 0x6
d123: jne 0xd12b
d125: sub ax, ax
d127: push ax
d128: call 0xd3c2
d12b: call 0x1ae                             ; EXE ticks_since(0)
d12e: cmp ax, 0x7
d131: jb 0xd12b
d133: cmp word ptr [0x9b26], 0x6
d138: jne 0xd13d
d13a: call 0x6887                            ; EXE flip: work buffer -> screen
d13d: mov ax, 0xffff
d140: push ax
d141: call 0xd3c2
d144: cmp word ptr [0xc32c], 0x0
d149: jne 0xd14e
d14b: jmp 0xd0c7
d14e: call 0xbcf4
d151: push word ptr [0xc308]
d155: call 0x74b7                            ; EXE free(buf)
d158: cmp word ptr [0x9b26], 0x6
d15d: jne 0xd162
d15f: call 0x6837                            ; EXE draw on the screen (6887 first if needed)
d162: call 0x5d20                            ; EXE music_stop
d165: mov al, 0x2
d167: push ax
d168: call 0x5e1a                            ; EXE music_unload(n)
d16b: mov al, 0x3
d16d: push ax
d16e: call 0x5e1a                            ; EXE music_unload(n)
d171: call 0xc9b6
d174: call 0xc2c2
d177: pop si
d178: pop di
d179: ret

; ---- func D17A  size 0x217  bp frame, locals 18, ret  callers: D08A
; opponents vs player and each other: overtaking and bumps (B9C9) (G)
d17a: push bp
d17b: mov bp, sp
d17d: sub sp, 0x12
d180: push di
d181: push si
d182: mov word ptr [bp - 0x6], 0x0
d187: mov word ptr [bp - 0xa], 0xc334
d18c: mov si, 0xc2ca
d18f: mov word ptr [bp - 0xc], 0x83cc
d194: mov di, 0xc30c
d197: jmp 0xd382
d19a: cmp word ptr [si - 0x8], 0x0
d19e: je 0xd1ac
d1a0: cmp byte ptr [si + 0x1], 0xb
d1a4: je 0xd1ac
d1a6: cmp byte ptr [si + 0x2], 0xb
d1aa: jne 0xd1af
d1ac: jmp 0xd371
d1af: sub dx, dx
d1b1: mov cx, dx
d1b3: cmp word ptr [di], 0x152
d1b7: ja 0xd1c5
d1b9: cmp word ptr [di], 0x8e
d1bd: jbe 0xd1c8
d1bf: cmp word ptr [di], 0xac
d1c3: jae 0xd1c8
d1c5: mov cx, 0xffff
d1c8: cmp word ptr [di], dx
d1ca: je 0xd1d1
d1cc: cmp word ptr [di], 0x2a
d1cf: jb 0xd1dd
d1d1: cmp word ptr [di], 0xac
d1d5: jbe 0xd1e0
d1d7: cmp word ptr [di], 0xd5
d1db: jae 0xd1e0
d1dd: mov dx, 0xffff
d1e0: mov word ptr [bp - 0xe], dx
d1e3: or dx, dx
d1e5: jne 0xd20a
d1e7: or cx, cx
d1e9: jne 0xd20a
d1eb: mov al, byte ptr [si]
d1ed: cwde
d1ee: sub ax, 0x3
d1f1: jns 0xd1f5
d1f3: sub ax, ax
d1f5: mov byte ptr [si], al
d1f7: mov bx, word ptr [bp - 0xc]
d1fa: mov bx, word ptr [bx]
d1fc: mov bx, word ptr [bx]
d1fe: mov byte ptr [bx + 0x8], al
d201: mov al, byte ptr [si - 0x2]
d204: mov byte ptr [si - 0x1], al
d207: jmp 0xd371
d20a: mov bx, word ptr [bp - 0xa]
d20d: mov ax, word ptr [bx]
d20f: cdq
d210: xor ax, dx
d212: sub ax, dx
d214: mov cx, 0x4
d217: sar ax, cl
d219: xor ax, dx
d21b: sub ax, dx
d21d: mov byte ptr [bp - 0x4], al
d220: add al, 0xf
d222: shl al
d224: shl al
d226: mov byte ptr [bp - 0x7], al
d229: cmp word ptr [bp - 0xe], 0x0
d22d: je 0xd24d
d22f: mov bl, byte ptr [bp - 0x4]
d232: sub bh, bh
d234: mov al, byte ptr [bx + 0x8aa0]         ; tbl DS:8AA0
d238: mul byte ptr [si - 0x2]
d23b: mov cx, 0xfff1
d23e: sub cx, bx
d240: shl cx
d242: shl cx
d244: cdq
d245: idiv cx
d247: add al, byte ptr [si - 0x2]
d24a: mov byte ptr [si - 0x1], al
d24d: mov al, byte ptr [bp - 0x7]
d250: sub al, byte ptr [si - 0x2]
d253: mov byte ptr [bp - 0x1], al
d256: or al, al
d258: jle 0xd26d
d25a: mov cl, 0x4
d25c: cwde
d25d: idiv cl
d25f: cmp al, 0xf
d261: jle 0xd265
d263: mov al, 0xf
d265: mov cx, ax
d267: mov al, 0xf
d269: sub al, cl
d26b: jmp 0xd26f
d26d: mov al, 0xf
d26f: mov byte ptr [bp - 0x3], al
d272: mov al, byte ptr [si]
d274: cmp byte ptr [bp - 0x3], al
d277: jle 0xd289
d279: cwde
d27a: add ax, 0x3
d27d: mov cx, ax
d27f: mov al, byte ptr [bp - 0x3]
d282: cwde
d283: cmp cx, ax
d285: jle 0xd2a0
d287: jmp 0xd29e
d289: mov al, byte ptr [si]
d28b: cmp byte ptr [bp - 0x3], al
d28e: jge 0xd2a2
d290: cwde
d291: sub ax, 0x3
d294: mov cx, ax
d296: mov al, byte ptr [bp - 0x3]
d299: cwde
d29a: cmp cx, ax
d29c: jge 0xd2a0
d29e: mov cx, ax
d2a0: mov byte ptr [si], cl
d2a2: cmp byte ptr [si], 0x0
d2a5: jge 0xd2aa
d2a7: mov byte ptr [si], 0x0
d2aa: cmp byte ptr [bp - 0x1], 0x0
d2ae: jge 0xd2d1
d2b0: mov al, byte ptr [bp - 0x1]
d2b3: cwde
d2b4: xor al, ah
d2b6: sub al, ah
d2b8: mov byte ptr [bp - 0x3], al
d2bb: cmp word ptr [bp - 0xe], 0x0
d2bf: je 0xd2d1
d2c1: mov ax, 0xf
d2c4: push ax
d2c5: call 0x76b                             ; EXE rng_roll(n)
d2c8: cmp al, byte ptr [bp - 0x3]
d2cb: jge 0xd2d1
d2cd: mov byte ptr [si + 0x2], 0xb
d2d1: cmp word ptr [bp - 0xe], 0x0
d2d5: jne 0xd2da
d2d7: jmp 0xd35f
d2da: cmp byte ptr [bp - 0x3], 0x8
d2de: jle 0xd35f
d2e0: push word ptr [bp - 0x6]
d2e3: call 0xd5e1
d2e6: mov al, byte ptr [0xc346]
d2e9: mov byte ptr [bp - 0x1], al
d2ec: mov al, byte ptr [0xc347]
d2ef: mov byte ptr [bp - 0x2], al
d2f2: cmp byte ptr [si + 0x1], 0x5
d2f6: je 0xd33e
d2f8: test byte ptr [bp - 0x1], 0x20
d2fc: je 0xd317
d2fe: push word ptr [bp - 0x6]
d301: mov al, byte ptr [bp - 0x1]
d304: and ax, 0xf
d307: push ax
d308: mov cx, 0xffff
d30b: push cx
d30c: mov word ptr [bp - 0x10], ax
d30f: call 0xb9c9
d312: push word ptr [bp - 0x10]
d315: jmp 0xd334
d317: test byte ptr [bp - 0x2], 0x20
d31b: je 0xd33e
d31d: push word ptr [bp - 0x6]
d320: mov al, byte ptr [bp - 0x2]
d323: and ax, 0xf
d326: push ax
d327: mov cx, 0xffff
d32a: push cx
d32b: mov word ptr [bp - 0x12], ax
d32e: call 0xb9c9
d331: push word ptr [bp - 0x12]
d334: push word ptr [bp - 0x6]
d337: mov ax, 0xffff
d33a: push ax
d33b: call 0xb9c9
d33e: cmp byte ptr [bp - 0x1], 0x0
d342: jne 0xd35f
d344: cmp byte ptr [bp - 0x2], 0x0
d348: jne 0xd35f
d34a: cmp byte ptr [bp - 0x4], 0xe
d34e: jae 0xd35f
d350: mov al, byte ptr [bp - 0x3]
d353: mov cl, 0x8
d355: cwde
d356: idiv cl
d358: cwde
d359: inc ax
d35a: mov bx, word ptr [bp - 0xa]
d35d: add word ptr [bx], ax
d35f: mov al, byte ptr [si]
d361: cmp al, 0xf
d363: jle 0xd367
d365: mov al, 0xf
d367: mov bx, word ptr [bp - 0xc]
d36a: mov bx, word ptr [bx]
d36c: mov bx, word ptr [bx]
d36e: mov byte ptr [bx + 0x8], al
d371: add word ptr [bp - 0xa], 0x4
d375: add si, 0x12
d378: add word ptr [bp - 0xc], 0x2
d37c: add di, 0x4
d37f: inc word ptr [bp - 0x6]
d382: cmp di, 0xc31c
d386: jae 0xd38b
d388: jmp 0xd19a
d38b: pop si
d38c: pop di
d38d: mov sp, bp
d38f: pop bp
d390: ret
d391: db 0x00                                ; word-alignment pad

; ---- func D392  size 0x30  bp frame, locals 2, ret  callers: C2DE D3C2
; lap counter: laps sheet frame C350 (D)
d392: push bp
d393: mov bp, sp
d395: sub sp, 0x2
d398: mov byte ptr [bp - 0x1], 0x0
d39c: cmp byte ptr [0xc350], 0x0
d3a1: jle 0xd3a9
d3a3: mov al, byte ptr [0xc350]
d3a6: mov byte ptr [bp - 0x1], al
d3a9: mov al, 0xe
d3ab: imul byte ptr [bp - 0x1]
d3ae: add ax, word ptr [0xc306]
d3b2: push ax
d3b3: mov ax, 0x86
d3b6: push ax
d3b7: mov ax, 0xb9
d3ba: push ax
d3bb: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
d3be: mov sp, bp
d3c0: pop bp
d3c1: ret

; ---- func D3C2  size 0x3e  bp frame, locals 0, ret 0x2  callers: D08A
; D3C2(all): redraw the dashboard parts flagged dirty (C330, C36C, C332) (D)
d3c2: push bp
d3c3: mov bp, sp
d3c5: call 0xd400
d3c8: cmp word ptr [0xc330], 0x0
d3cd: je 0xd3d2
d3cf: call 0xd49a
d3d2: cmp word ptr [0xc36c], 0x0
d3d7: je 0xd3dc
d3d9: call 0xd4ec
d3dc: cmp word ptr [0xc332], 0x0
d3e1: je 0xd3e6
d3e3: call 0xd392
d3e6: cmp word ptr [bp + 0x4], 0x0
d3ea: je 0xd3fa
d3ec: sub ax, ax
d3ee: mov word ptr [0xc36c], ax
d3f1: mov word ptr [0xc330], ax
d3f4: mov word ptr [0xc32e], ax
d3f7: mov word ptr [0xc332], ax
d3fa: mov sp, bp
d3fc: pop bp
d3fd: ret 0x2

; ---- func D400  size 0x53  no bp frame, ret  callers: C2DE D3C2
; speed gauge (C2FE) (G)
d400: push si
d401: cmp byte ptr [0xc301], 0x5
d406: je 0xd451
d408: mov word ptr [0x9afa], 0x2
d40e: mov ax, 0xe
d411: push ax
d412: mov cx, 0xb7
d415: push cx
d416: mov al, 0x6e
d418: mul byte ptr [0xc2fe]
d41c: mov bx, 0x82
d41f: cdq
d420: idiv bx
d422: mov si, ax
d424: or si, si
d426: jge 0xd42a
d428: sub si, si
d42a: lea ax, [si + 0xe]
d42d: push ax
d42e: mov ax, 0xba
d431: push ax
d432: call 0x7e74                            ; EXE fill rect (colour, ...)
d435: mov word ptr [0x9afa], 0x7
d43b: lea ax, [si + 0xe]
d43e: push ax
d43f: mov ax, 0xb7
d442: push ax
d443: mov ax, 0x7c
d446: push ax
d447: mov ax, 0xba
d44a: push ax
d44b: call 0x7e74                            ; EXE fill rect (colour, ...)
d44e: call 0xd453
d451: pop si
d452: ret

; ---- func D453  size 0x47  no bp frame, ret  callers: D400
; speed gauge needle (line) (G)
d453: cmp byte ptr [0xc301], 0x5
d458: je 0xd499
d45a: mov word ptr [0x9afa], 0x4
d460: mov ax, word ptr [0xc340]
d463: cdq
d464: xor ax, dx
d466: sub ax, dx
d468: mov cx, 0x4
d46b: sar ax, cl
d46d: xor ax, dx
d46f: sub ax, dx
d471: cmp ax, 0xf
d474: jle 0xd479
d476: mov ax, 0xf
d479: add al, 0xf
d47b: shl al
d47d: shl al
d47f: mov cl, 0x6e
d481: mul cl
d483: mov cx, 0x82
d486: cdq
d487: idiv cx
d489: add ax, 0xd
d48c: push ax
d48d: mov cx, 0xb7
d490: push cx
d491: push ax
d492: mov ax, 0xba
d495: push ax
d496: call 0x979c                            ; EXE line(x0, y0, x1, y1) via 97B6 with putpixel 82DE
d499: ret

; ---- func D49A  size 0x52  no bp frame, ret  callers: C2DE D3C2
; gauge of C304 (G)
d49a: push si
d49b: cmp byte ptr [0xc301], 0x5
d4a0: je 0xd4ea
d4a2: mov word ptr [0x9afa], 0x4
d4a8: mov ax, 0xc7
d4ab: push ax
d4ac: mov cx, 0xb3
d4af: push cx
d4b0: mov al, 0x6e
d4b2: imul byte ptr [0xc304]
d4b6: mov bx, 0x3c
d4b9: cdq
d4ba: idiv bx
d4bc: mov si, ax
d4be: or si, si
d4c0: jge 0xd4c4
d4c2: sub si, si
d4c4: lea ax, [si + 0xc7]
d4c8: push ax
d4c9: mov ax, 0xb5
d4cc: push ax
d4cd: call 0x7e74                            ; EXE fill rect (colour, ...)
d4d0: mov word ptr [0x9afa], 0x7
d4d6: lea ax, [si + 0xc7]
d4da: push ax
d4db: mov ax, 0xb3
d4de: push ax
d4df: mov ax, 0x135
d4e2: push ax
d4e3: mov ax, 0xb5
d4e6: push ax
d4e7: call 0x7e74                            ; EXE fill rect (colour, ...)
d4ea: pop si
d4eb: ret

; ---- func D4EC  size 0x52  no bp frame, ret  callers: C2DE D3C2
; gauge of C303 (G)
d4ec: push si
d4ed: cmp byte ptr [0xc301], 0x5
d4f2: je 0xd53c
d4f4: mov word ptr [0x9afa], 0x1
d4fa: mov ax, 0xc7
d4fd: push ax
d4fe: mov cx, 0xbc
d501: push cx
d502: mov al, 0x6e
d504: imul byte ptr [0xc303]
d508: mov bx, 0x14
d50b: cdq
d50c: idiv bx
d50e: mov si, ax
d510: or si, si
d512: jge 0xd516
d514: sub si, si
d516: lea ax, [si + 0xc7]
d51a: push ax
d51b: mov ax, 0xbe
d51e: push ax
d51f: call 0x7e74                            ; EXE fill rect (colour, ...)
d522: mov word ptr [0x9afa], 0x7
d528: lea ax, [si + 0xc7]
d52c: push ax
d52d: mov ax, 0xbc
d530: push ax
d531: mov ax, 0x135
d534: push ax
d535: mov ax, 0xbe
d538: push ax
d539: call 0x7e74                            ; EXE fill rect (colour, ...)
d53c: pop si
d53d: ret

; ---- func D53E  size 0x52  bp frame, locals 2, ret 0x4  callers: B8BC B9C9
; D53E(chariot, n): chariot damage C2CD -= n; at 0 the chariot is wrecked (state 11) (D)
d53e: push bp
d53f: mov bp, sp
d541: sub sp, 0x2
d544: push si
d545: mov si, word ptr [bp + 0x6]
d548: mov ax, si
d54a: mov cx, 0x12
d54d: mul cx
d54f: mov bx, ax
d551: mov word ptr [bp - 0x2], bx
d554: mov al, byte ptr [bp + 0x4]
d557: cmp byte ptr [bx + 0xc2cd], al         ; tbl DS:C2CD
d55b: jle 0xd563
d55d: sub byte ptr [bx + 0xc2cd], al         ; tbl DS:C2CD
d561: jmp 0xd568
d563: mov byte ptr [bx + 0xc2cd], 0x0        ; tbl DS:C2CD
d568: cmp si, 0x3
d56b: jne 0xd573
d56d: mov word ptr [0xc36c], 0xffff
d573: mov bx, word ptr [bp - 0x2]
d576: cmp byte ptr [bx + 0xc2cd], 0x0        ; tbl DS:C2CD
d57b: jg 0xd589
d57d: cmp byte ptr [bx + 0xc2cb], 0xb        ; tbl DS:C2CB
d582: je 0xd589
d584: mov byte ptr [bx + 0xc2cc], 0xb        ; tbl DS:C2CC
d589: pop si
d58a: mov sp, bp
d58c: pop bp
d58d: ret 0x4

; ---- func D590  size 0x51  bp frame, locals 2, ret 0x4  callers: B8BC B9C9
; D590(chariot, n): horse strength C2CE -= n; speed limited to the maximum (D)
d590: push bp
d591: mov bp, sp
d593: sub sp, 0x2
d596: push si
d597: mov si, word ptr [bp + 0x6]
d59a: mov ax, si
d59c: mov cx, 0x12
d59f: mul cx
d5a1: mov bx, ax
d5a3: mov word ptr [bp - 0x2], bx
d5a6: mov al, byte ptr [bp + 0x4]
d5a9: cmp byte ptr [bx + 0xc2ce], al         ; tbl DS:C2CE
d5ad: jle 0xd5b5
d5af: sub byte ptr [bx + 0xc2ce], al         ; tbl DS:C2CE
d5b3: jmp 0xd5ba
d5b5: mov byte ptr [bx + 0xc2ce], 0x0        ; tbl DS:C2CE
d5ba: cmp si, 0x3
d5bd: jne 0xd5c5
d5bf: mov word ptr [0xc330], 0xffff
d5c5: mov bx, word ptr [bp - 0x2]
d5c8: mov al, byte ptr [bx + 0xc2c7]         ; tbl DS:C2C7
d5cc: cmp al, byte ptr [bx + 0xc2c8]         ; tbl DS:C2C8
d5d0: jbe 0xd5d6
d5d2: mov al, byte ptr [bx + 0xc2c8]         ; tbl DS:C2C8
d5d6: mov byte ptr [bx + 0xc2c8], al         ; tbl DS:C2C8
d5da: pop si
d5db: mov sp, bp
d5dd: pop bp
d5de: ret 0x4

; ---- func D5E1  size 0x157  bp frame, locals 90, ret 0x2  callers: B190 D17A DABA
; D5E1(chariot): neighbours on the left and right (C344..C34B) by rect tests (817C) (G)
d5e1: push bp
d5e2: mov bp, sp
d5e4: sub sp, 0x5a
d5e7: push di
d5e8: push si
d5e9: sub di, di
d5eb: sub bx, bx
d5ed: mov word ptr [bp - 0x4], di
d5f0: jmp 0xd5f8
d5f2: mov byte ptr [bx + di + 0xc344], 0x0   ; tbl DS:C344
d5f7: inc bx
d5f8: cmp bx, 0x2
d5fb: jl 0xd5f2
d5fd: inc di
d5fe: inc di
d5ff: cmp di, 0xc
d602: jl 0xd5eb
d604: sub dx, dx
d606: mov word ptr [bp - 0x2], 0xc334
d60b: jmp 0xd655
d60d: mov si, dx
d60f: push si
d610: lea di, [bp + si - 0x3a]
d613: mov si, 0x8ae4
d616: push ss
d617: pop es
d618: movsw word ptr es:[di], word ptr [si]
d619: movsw word ptr es:[di], word ptr [si]
d61a: movsw word ptr es:[di], word ptr [si]
d61b: movsw word ptr es:[di], word ptr [si]
d61c: pop si
d61d: mov bx, word ptr [bp - 0x2]
d620: mov ax, word ptr [bx]
d622: add word ptr [bp + si - 0x3a], ax
d625: mov bx, word ptr [bp - 0x2]
d628: mov cx, word ptr [bx + 0x2]
d62b: add word ptr [bp + si - 0x38], cx
d62e: add word ptr [bp + si - 0x36], ax
d631: add word ptr [bp + si - 0x34], cx
d634: push cx
d635: push si
d636: lea di, [bp + si - 0x5a]
d639: mov si, 0x8aec
d63c: movsw word ptr es:[di], word ptr [si]
d63d: movsw word ptr es:[di], word ptr [si]
d63e: movsw word ptr es:[di], word ptr [si]
d63f: movsw word ptr es:[di], word ptr [si]
d640: pop si
d641: pop cx
d642: add word ptr [bp + si - 0x5a], ax
d645: add word ptr [bp + si - 0x58], cx
d648: add word ptr [bp + si - 0x56], ax
d64b: add word ptr [bp + si - 0x54], cx
d64e: add dx, 0x8
d651: add word ptr [bp - 0x2], 0x4
d655: cmp word ptr [bp - 0x2], 0xc344
d65a: jb 0xd60d
d65c: mov bx, word ptr [bp + 0x4]
d65f: shl bx
d661: shl bx
d663: mov ax, word ptr [bx + 0xc334]         ; tbl DS:C334
d667: mov word ptr [bp - 0x10], ax
d66a: mov ax, word ptr [bx + 0xc336]         ; tbl DS:C336
d66e: mov word ptr [bp - 0xe], ax
d671: mov word ptr [bp - 0xa], 0x0
d676: mov dx, 0x8ab4
d679: jmp 0xd727
d67c: lea di, [bp - 0x1a]
d67f: mov si, dx
d681: push ss
d682: pop es
d683: movsw word ptr es:[di], word ptr [si]
d684: movsw word ptr es:[di], word ptr [si]
d685: movsw word ptr es:[di], word ptr [si]
d686: movsw word ptr es:[di], word ptr [si]
d687: mov ax, word ptr [bp - 0x10]
d68a: add word ptr [bp - 0x1a], ax
d68d: mov cx, word ptr [bp - 0xe]
d690: add word ptr [bp - 0x18], cx
d693: add word ptr [bp - 0x16], ax
d696: add word ptr [bp - 0x14], cx
d699: sub ax, ax
d69b: mov word ptr [bp - 0x4], ax
d69e: mov word ptr [bp - 0x2], ax
d6a1: mov word ptr [bp - 0x6], ax
d6a4: mov word ptr [bp - 0x8], 0xc2c2
d6a9: mov word ptr [bp - 0xc], dx
d6ac: mov si, ax
d6ae: jmp 0xd716
d6b0: cmp word ptr [bp + 0x4], si
d6b3: je 0xd70d
d6b5: mov bx, word ptr [bp - 0x8]
d6b8: cmp word ptr [bx], 0x0
d6bb: je 0xd70d
d6bd: lea ax, [bp - 0x1a]
d6c0: push ax
d6c1: mov di, word ptr [bp - 0x6]
d6c4: lea ax, [bp + di - 0x3a]
d6c7: push ax
d6c8: sub ax, ax
d6ca: push ax
d6cb: call 0x817c                            ; EXE sprite clip against C0DA..C0E0
d6ce: or ax, ax
d6d0: je 0xd6e5
d6d2: mov bx, word ptr [bp - 0xa]
d6d5: mov di, word ptr [bp - 0x4]
d6d8: mov ax, si
d6da: and al, 0xf
d6dc: or al, 0x10
d6de: mov byte ptr [bx + di + 0xc344], al    ; tbl DS:C344
d6e2: inc word ptr [bp - 0x4]
d6e5: lea ax, [bp - 0x1a]
d6e8: push ax
d6e9: mov di, word ptr [bp - 0x6]
d6ec: lea ax, [bp + di - 0x5a]
d6ef: push ax
d6f0: sub ax, ax
d6f2: push ax
d6f3: call 0x817c                            ; EXE sprite clip against C0DA..C0E0
d6f6: or ax, ax
d6f8: je 0xd70d
d6fa: mov bx, word ptr [bp - 0xa]
d6fd: mov di, word ptr [bp - 0x4]
d700: mov ax, si
d702: and al, 0xf
d704: or al, 0x20
d706: mov byte ptr [bx + di + 0xc344], al    ; tbl DS:C344
d70a: inc word ptr [bp - 0x4]
d70d: add word ptr [bp - 0x6], 0x8
d711: add word ptr [bp - 0x8], 0x12
d715: inc si
d716: cmp word ptr [bp - 0x8], 0xc30a
d71b: jb 0xd6b0
d71d: add word ptr [bp - 0xa], 0x2
d721: mov dx, word ptr [bp - 0xc]
d724: add dx, 0x8
d727: cmp dx, 0x8ae4
d72b: jae 0xd730
d72d: jmp 0xd67c
d730: pop si
d731: pop di
d732: mov sp, bp
d734: pop bp
d735: ret 0x2

; ---- func D738  size 0x2f6  bp frame, locals 32, ret  callers: B6CB
; crash scene for the player (one of two, rng(2)) (G)
d738: push bp
d739: mov bp, sp
d73b: sub sp, 0x20
d73e: push di
d73f: push si
d740: mov word ptr [bp - 0xa], 0x0
d745: call 0x5d20                            ; EXE music_stop
d748: mov ax, 0x2
d74b: push ax
d74c: call 0x76b                             ; EXE rng_roll(n)
d74f: or ax, ax
d751: je 0xd76c
d753: mov word ptr [bp - 0x12], 0x8b3a
d758: mov word ptr [bp - 0xe], 0x8b04
d75d: mov word ptr [bp - 0x10], 0x1c
d762: sub ax, ax
d764: mov word ptr [bp - 0x20], ax
d767: mov word ptr [bp - 0xc], ax
d76a: jmp 0xd785
d76c: mov word ptr [bp - 0x12], 0x8b72
d771: mov word ptr [bp - 0xe], 0x8b20
d776: mov word ptr [bp - 0x10], 0x1a
d77b: mov word ptr [bp - 0x20], 0x19
d780: mov word ptr [bp - 0xc], 0xffff
d785: mov ax, word ptr [0xc340]
d788: mov dx, word ptr [0xc342]
d78c: mov word ptr [bp - 0x1e], ax
d78f: mov word ptr [bp - 0x1c], dx
d792: add ax, 0x9
d795: mov word ptr [bp - 0x1a], ax
d798: mov cx, dx
d79a: add cx, 0x1d
d79d: mov word ptr [bp - 0x18], cx
d7a0: mov dx, 0xc0da
d7a3: push cx
d7a4: mov di, dx
d7a6: mov si, 0x83d4
d7a9: push ds
d7aa: pop es
d7ab: movsw word ptr es:[di], word ptr [si]
d7ac: movsw word ptr es:[di], word ptr [si]
d7ad: movsw word ptr es:[di], word ptr [si]
d7ae: movsw word ptr es:[di], word ptr [si]
d7af: pop cx
d7b0: mov dx, 0xc1e4
d7b3: push cx
d7b4: mov di, dx
d7b6: mov si, 0x83d4
d7b9: movsw word ptr es:[di], word ptr [si]
d7ba: movsw word ptr es:[di], word ptr [si]
d7bb: movsw word ptr es:[di], word ptr [si]
d7bc: movsw word ptr es:[di], word ptr [si]
d7bd: pop cx
d7be: mov dx, 0x9ac0
d7c1: push cx
d7c2: mov di, dx
d7c4: mov si, 0x83d4
d7c7: movsw word ptr es:[di], word ptr [si]
d7c8: movsw word ptr es:[di], word ptr [si]
d7c9: movsw word ptr es:[di], word ptr [si]
d7ca: movsw word ptr es:[di], word ptr [si]
d7cb: pop cx
d7cc: mov bx, 0x4b
d7cf: mov dx, ax
d7d1: mov al, byte ptr [0xc2fe]
d7d4: mov si, cx
d7d6: sub ah, ah
d7d8: mov cl, 0x3
d7da: shl ax, cl
d7dc: mov cx, dx
d7de: cdq
d7df: idiv bx
d7e1: mov word ptr [bp - 0x8], ax
d7e4: mov ax, 0xaa
d7e7: sub ax, si
d7e9: cdq
d7ea: sub ax, dx
d7ec: sar ax
d7ee: add ax, 0xf
d7f1: mov word ptr [bp - 0x6], ax
d7f4: mov dx, ax
d7f6: shl ax
d7f8: sub cx, ax
d7fa: sub cx, 0x13
d7fd: mov word ptr [bp - 0x16], cx
d800: add dx, 0x6
d803: shl dx
d805: add si, dx
d807: mov word ptr [bp - 0x14], si
d80a: call 0x4ed                             ; EXE key_flush
d80d: mov word ptr [bp - 0x4], 0x0
d812: mov si, word ptr [bp - 0x4]
d815: cmp word ptr [bp - 0x6], si
d818: jg 0xd81d
d81a: jmp 0xd8a3
d81d: call 0xf8                              ; EXE timer0_reset (= 00D7(0))
d820: call 0x67e4                            ; EXE draw into the work buffer
d823: call 0xda2e
d826: cmp word ptr [bp - 0x1c], -0x16
d82a: jl 0xd83f
d82c: push word ptr [0x83d2]
d830: push word ptr [bp - 0x1e]
d833: mov ax, word ptr [bp - 0x8]
d836: sub word ptr [bp - 0x1c], ax
d839: push word ptr [bp - 0x1c]
d83c: call 0xbd0a
d83f: mov ax, word ptr [0xc352]
d842: add ax, 0x196
d845: push ax
d846: mov ax, word ptr [bp - 0x1a]
d849: add ax, 0xe
d84c: push ax
d84d: mov ax, word ptr [bp - 0x18]
d850: add ax, 0x6
d853: push ax
d854: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
d857: mov ax, si
d859: and al, 0x1
d85b: mov cl, 0xe
d85d: mul cl
d85f: add ax, word ptr [0xc352]
d863: push ax
d864: push word ptr [bp - 0x16]
d867: push word ptr [bp - 0x14]
d86a: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
d86d: add word ptr [bp - 0x16], 0x2
d871: sub word ptr [bp - 0x14], 0x2
d875: call 0x3aa                             ; EXE key_read: next key or 0
d878: cmp al, 0x1b
d87a: je 0xd89e
d87c: call 0x1ae                             ; EXE ticks_since(0)
d87f: cmp ax, 0x7
d882: jb 0xd87c
d884: call 0x680f                            ; EXE target = the screen descriptor
d887: push word ptr [0x83d4]
d88b: push word ptr [0x83d6]
d88f: push word ptr [0x83d8]
d893: push word ptr [0x83da]
d897: call 0x6973                            ; EXE restore rect work buffer -> screen
d89a: inc si
d89b: jmp 0xd815
d89e: mov word ptr [bp - 0xa], 0xffff
d8a3: cmp word ptr [bp - 0xa], 0x0
d8a7: je 0xd8ac
d8a9: jmp 0xd952
d8ac: sub si, si
d8ae: mov ax, word ptr [bp - 0x12]
d8b1: mov di, ax
d8b3: cmp word ptr [bp - 0x10], si
d8b6: jg 0xd8bb
d8b8: jmp 0xd952
d8bb: call 0xf8                              ; EXE timer0_reset (= 00D7(0))
d8be: call 0x67e4                            ; EXE draw into the work buffer
d8c1: call 0xda2e
d8c4: cmp si, 0x7
d8c7: jg 0xd8e1
d8c9: mov ax, word ptr [0xc352]
d8cc: add ax, 0x196
d8cf: push ax
d8d0: mov ax, word ptr [bp - 0x1a]
d8d3: add ax, 0xe
d8d6: push ax
d8d7: mov ax, word ptr [bp - 0x18]
d8da: add ax, 0x6
d8dd: push ax
d8de: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
d8e1: cmp word ptr [bp - 0xc], 0x0
d8e5: jne 0xd904
d8e7: cmp si, 0x13
d8ea: jle 0xd904
d8ec: mov ax, word ptr [0xc352]
d8ef: add ax, 0x196
d8f2: push ax
d8f3: mov ax, word ptr [bp - 0x1a]
d8f6: sub ax, 0x9
d8f9: push ax
d8fa: mov ax, word ptr [bp - 0x18]
d8fd: add ax, 0x7
d900: push ax
d901: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
d904: mov bx, word ptr [bp - 0xe]
d907: mov al, 0xe
d909: mul byte ptr [bx + si]
d90b: add ax, word ptr [0xc352]
d90f: push ax
d910: mov al, byte ptr [di]
d912: cwde
d913: add ax, word ptr [bp - 0x1a]
d916: push ax
d917: mov al, byte ptr [di + 0x1]
d91a: cwde
d91b: add ax, word ptr [bp - 0x18]
d91e: push ax
d91f: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
d922: call 0x3aa                             ; EXE key_read: next key or 0
d925: cmp al, 0x1b
d927: je 0xd94d
d929: call 0x1ae                             ; EXE ticks_since(0)
d92c: cmp ax, 0x7
d92f: jb 0xd929
d931: call 0x680f                            ; EXE target = the screen descriptor
d934: push word ptr [0x83d4]
d938: push word ptr [0x83d6]
d93c: push word ptr [0x83d8]
d940: push word ptr [0x83da]
d944: call 0x6973                            ; EXE restore rect work buffer -> screen
d947: inc di
d948: inc di
d949: inc si
d94a: jmp 0xd8b3
d94d: mov word ptr [bp - 0xa], 0xffff
d952: add word ptr [bp - 0x16], 0x1b
d956: sub word ptr [bp - 0x14], 0x12
d95a: cmp word ptr [bp - 0xc], 0x0
d95e: je 0xd968
d960: add word ptr [bp - 0x16], 0x9
d964: sub word ptr [bp - 0x14], 0x2
d968: sub si, si
d96a: mov di, word ptr [bp - 0xa]
d96d: jmp 0xd9f2
d970: cmp word ptr [bp - 0x16], 0x13f
d975: jge 0xd97d
d977: cmp word ptr [bp - 0x14], 0x15
d97b: jg 0xd97f
d97d: jmp 0xd9f9
d97f: inc si
d980: call 0xf8                              ; EXE timer0_reset (= 00D7(0))
d983: call 0x67e4                            ; EXE draw into the work buffer
d986: call 0xda2e
d989: cmp word ptr [bp - 0xc], 0x0
d98d: jne 0xd9a7
d98f: mov ax, word ptr [0xc352]
d992: add ax, 0x196
d995: push ax
d996: mov ax, word ptr [bp - 0x1a]
d999: sub ax, 0x9
d99c: push ax
d99d: mov ax, word ptr [bp - 0x18]
d9a0: add ax, 0x7
d9a3: push ax
d9a4: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
d9a7: mov ax, si
d9a9: and ax, 0x1
d9ac: add ax, word ptr [bp - 0x20]
d9af: mov cx, 0xe
d9b2: imul cx
d9b4: add ax, word ptr [0xc352]
d9b8: push ax
d9b9: push word ptr [bp - 0x16]
d9bc: push word ptr [bp - 0x14]
d9bf: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
d9c2: add word ptr [bp - 0x16], 0x2
d9c6: sub word ptr [bp - 0x14], 0x2
d9ca: call 0x3aa                             ; EXE key_read: next key or 0
d9cd: cmp al, 0x1b
d9cf: jne 0xd9d4
d9d1: mov di, 0xffff
d9d4: call 0x1ae                             ; EXE ticks_since(0)
d9d7: cmp ax, 0x7
d9da: jb 0xd9d4
d9dc: call 0x680f                            ; EXE target = the screen descriptor
d9df: push word ptr [0x83d4]
d9e3: push word ptr [0x83d6]
d9e7: push word ptr [0x83d8]
d9eb: push word ptr [0x83da]
d9ef: call 0x6973                            ; EXE restore rect work buffer -> screen
d9f2: or di, di
d9f4: jne 0xd9f9
d9f6: jmp 0xd970
d9f9: call 0x680f                            ; EXE target = the screen descriptor
d9fc: call 0xda2e
d9ff: mov ax, 0xc0da
da02: mov di, ax
da04: mov si, 0x83dc
da07: push ds
da08: pop es
da09: movsw word ptr es:[di], word ptr [si]
da0a: movsw word ptr es:[di], word ptr [si]
da0b: movsw word ptr es:[di], word ptr [si]
da0c: movsw word ptr es:[di], word ptr [si]
da0d: mov ax, 0xc1e4
da10: mov di, ax
da12: mov si, 0x83dc
da15: movsw word ptr es:[di], word ptr [si]
da16: movsw word ptr es:[di], word ptr [si]
da17: movsw word ptr es:[di], word ptr [si]
da18: movsw word ptr es:[di], word ptr [si]
da19: mov ax, 0x9ac0
da1c: mov di, ax
da1e: mov si, 0x83dc
da21: movsw word ptr es:[di], word ptr [si]
da22: movsw word ptr es:[di], word ptr [si]
da23: movsw word ptr es:[di], word ptr [si]
da24: movsw word ptr es:[di], word ptr [si]
da25: call 0x4ed                             ; EXE key_flush
da28: pop si
da29: pop di
da2a: mov sp, bp
da2c: pop bp
da2d: ret

; ---- func DA2E  size 0x2c  no bp frame, ret  callers: D738
; clear the view and draw the speckles (D)
da2e: mov word ptr [0x9afa], 0x7
da34: cmp word ptr [0x9b26], 0x2
da39: jne 0xda41
da3b: mov word ptr [0x9afa], 0x11
da41: sub ax, ax
da43: push ax
da44: mov cx, 0x3d
da47: push cx
da48: mov cx, 0x13f
da4b: push cx
da4c: mov cx, 0xaa
da4f: push cx
da50: call 0x7e74                            ; EXE fill rect (colour, ...)
da53: sub ax, ax
da55: push ax
da56: call 0xb11c
da59: ret

; ---- func DA5A  size 0x60  bp frame, locals 2, ret 0x2  callers: C2DE
; DA5A(i): random racer i: home province rng(27), chariot type rng(3), strategy rng(4) (3 -> 1), skill rng(11) + B(8BA6) - 5 (D)
da5a: push bp
da5b: mov bp, sp
da5d: sub sp, 0x2
da60: push si
da61: mov ax, 0x1b
da64: push ax
da65: call 0x76b                             ; EXE rng_roll(n)
da68: mov cx, ax
da6a: mov ax, 0x12
da6d: mul word ptr [bp + 0x4]
da70: mov bx, ax
da72: mov byte ptr [bx + 0xc2c5], cl         ; tbl DS:C2C5
da76: mov ax, 0x3
da79: push ax
da7a: mov si, bx
da7c: call 0x76b                             ; EXE rng_roll(n)
da7f: mov byte ptr [si + 0xc2c6], al         ; tbl DS:C2C6
da83: mov ax, 0x4
da86: push ax
da87: call 0x76b                             ; EXE rng_roll(n)
da8a: mov byte ptr [bp - 0x1], al
da8d: cmp al, 0x3
da8f: jne 0xda95
da91: mov byte ptr [bp - 0x1], 0x1
da95: mov al, byte ptr [bp - 0x1]
da98: mov bx, word ptr [bp + 0x4]
da9b: mov byte ptr [bx + 0xc368], al         ; tbl DS:C368
da9f: mov ax, 0xb
daa2: push ax
daa3: call 0x76b                             ; EXE rng_roll(n)
daa6: add al, byte ptr [0x8ba6]
daaa: sub al, 0x5
daac: mov bx, word ptr [bp + 0x4]
daaf: mov byte ptr [bx + 0xaac2], al         ; tbl DS:AAC2
dab3: pop si
dab4: mov sp, bp
dab6: pop bp
dab7: ret 0x2

; ---- func DABA  size 0x303  bp frame, locals 24, ret  callers: D08A
; opponents' driving per tick: target speeds AABE by difficulty B(048F), strategy C368 and skill AAC2 (D/G)
daba: push bp
dabb: mov bp, sp
dabd: sub sp, 0x18
dac0: push di
dac1: push si
dac2: mov di, 0xc30a
dac5: mov si, 0xc2c4
dac8: jmp 0xdae3
daca: mov al, byte ptr [si]
dacc: cwde
dacd: inc ax
dace: mov cx, 0x170
dad1: imul cx
dad3: add ax, word ptr [di]
dad5: cmp ax, word ptr [bp - 0x12]
dad8: jbe 0xdadd
dada: mov word ptr [bp - 0x12], ax
dadd: add di, 0x4
dae0: add si, 0x12
dae3: cmp si, 0xc30c
dae7: jb 0xdaca
dae9: sub si, si
daeb: mov di, 0xc2cc
daee: mov word ptr [bp - 0xc], 0xc334
daf3: mov word ptr [bp - 0x8], 0xc30c
daf8: jmp 0xddad
dafb: cmp word ptr [di - 0xa], 0x0
daff: je 0xdb0c
db01: mov al, byte ptr [di - 0x1]
db04: cmp al, 0xb
db06: je 0xdb0c
db08: cmp al, 0x5
db0a: jne 0xdb0f
db0c: jmp 0xdda1
db0f: sub ax, ax
db11: mov word ptr [bp - 0x14], ax
db14: mov word ptr [bp - 0x10], ax
db17: mov word ptr [bp - 0xe], ax
db1a: mov bx, word ptr [bp - 0x8]
db1d: mov ax, word ptr [bx]
db1f: cmp ax, 0x152
db22: ja 0xdb2e
db24: cmp ax, 0x8e
db27: jbe 0xdb33
db29: cmp ax, 0xac
db2c: jae 0xdb33
db2e: mov word ptr [bp - 0x10], 0xffff
db33: or ax, ax
db35: je 0xdb3c
db37: cmp ax, 0x2a
db3a: jb 0xdb46
db3c: cmp ax, 0xac
db3f: jbe 0xdb4b
db41: cmp ax, 0xd5
db44: jae 0xdb4b
db46: mov word ptr [bp - 0xe], 0xffff
db4b: mov al, byte ptr [di - 0x8]
db4e: cwde
db4f: inc ax
db50: mov cx, 0x170
db53: imul cx
db55: add ax, word ptr [bx - 0x2]
db58: cmp ax, word ptr [bp - 0x12]
db5b: jae 0xdb6f
db5d: mov word ptr [bp - 0x14], 0xffff
db62: cmp byte ptr [0xc301], 0x5
db67: jne 0xdb6f
db69: mov byte ptr [di], 0x8
db6c: jmp 0xdda1
db6f: mov bx, word ptr [bp - 0xc]
db72: mov ax, word ptr [bx]
db74: cdq
db75: xor ax, dx
db77: sub ax, dx
db79: mov cx, 0x4
db7c: sar ax, cl
db7e: xor ax, dx
db80: sub ax, dx
db82: cmp ax, 0xf
db85: jle 0xdb8a
db87: mov ax, 0xf
db8a: mov byte ptr [bp - 0x1], al
db8d: add al, 0xe
db8f: shl al
db91: shl al
db93: mov byte ptr [bp - 0x5], al
db96: push si
db97: call 0xd5e1
db9a: cmp word ptr [bp - 0x10], 0x0
db9e: jne 0xdba6
dba0: cmp word ptr [bp - 0xe], 0x0
dba4: je 0xdba9
dba6: jmp 0xdc3b
dba9: cmp byte ptr [si + 0x8bac], 0x0        ; tbl DS:8BAC
dbae: je 0xdbc4
dbb0: dec byte ptr [si + 0x8bac]             ; tbl DS:8BAC
dbb4: jne 0xdbbb
dbb6: mov byte ptr [si + 0x8ba8], 0x0        ; tbl DS:8BA8
dbbb: mov al, byte ptr [si + 0x8ba8]         ; tbl DS:8BA8
dbbf: mov byte ptr [di], al
dbc1: jmp 0xdda1
dbc4: push si
dbc5: call 0xddbd
dbc8: or ax, ax
dbca: jne 0xdbd4
dbcc: push si
dbcd: call 0xde10
dbd0: or ax, ax
dbd2: je 0xdbd7
dbd4: jmp 0xdda1
dbd7: cmp byte ptr [0xc34c], 0x0
dbdc: je 0xdbf8
dbde: cmp byte ptr [bp - 0x1], 0x7
dbe2: jae 0xdbf4
dbe4: mov al, 0xa
dbe6: mov byte ptr [di], al
dbe8: mov byte ptr [si + 0x8ba8], al         ; tbl DS:8BA8
dbec: mov byte ptr [si + 0x8bac], 0x19       ; tbl DS:8BAC
dbf1: jmp 0xdda1
dbf4: mov al, 0x9
dbf6: jmp 0xdbe6
dbf8: cmp word ptr [bp - 0x14], ax
dbfb: jne 0xdc1f
dbfd: mov al, byte ptr [di - 0x5]
dc00: add al, 0xa
dc02: cmp al, byte ptr [di - 0x4]
dc05: ja 0xdc1f
dc07: mov ax, 0xa
dc0a: push ax
dc0b: call 0x76b                             ; EXE rng_roll(n)
dc0e: mov word ptr [bp - 0xa], ax
dc11: cmp ax, 0x6
dc14: ja 0xdbe4
dc16: cmp ax, 0x2
dc19: ja 0xdbf4
dc1b: sub al, al
dc1d: jmp 0xdbe6
dc1f: cmp byte ptr [bp - 0x1], 0xa
dc23: jbe 0xdc30
dc25: mov ax, 0x2
dc28: push ax
dc29: call 0x76b                             ; EXE rng_roll(n)
dc2c: or ax, ax
dc2e: je 0xdbf4
dc30: mov byte ptr [di], 0x1
dc33: mov byte ptr [si + 0x8ba8], 0x0        ; tbl DS:8BA8
dc38: jmp 0xdda1
dc3b: mov ax, 0x64
dc3e: push ax
dc3f: call 0x76b                             ; EXE rng_roll(n)
dc42: mov cl, byte ptr [si + 0xaac2]         ; tbl DS:AAC2
dc46: sub ch, ch
dc48: sub ax, cx
dc4a: mov word ptr [bp - 0xa], ax
dc4d: cdq
dc4e: xor ax, dx
dc50: sub ax, dx
dc52: mov word ptr [bp - 0x2], ax
dc55: mov al, byte ptr [0x48f]
dc58: sub ah, ah
dc5a: or ax, ax
dc5c: je 0xdc73
dc5e: dec ax
dc5f: jne 0xdc64
dc61: jmp 0xdcea
dc64: dec ax
dc65: jne 0xdc6a
dc67: jmp 0xdd64
dc6a: dec ax
dc6b: jne 0xdc70
dc6d: jmp 0xdd70
dc70: jmp 0xdd85
dc73: cmp word ptr [bp - 0xa], 0x0
dc77: jge 0xdce0
dc79: mov al, byte ptr [si + 0xc368]         ; tbl DS:C368
dc7d: sub ah, ah
dc7f: or ax, ax
dc81: je 0xdc8c
dc83: dec ax
dc84: je 0xdcb1
dc86: dec ax
dc87: je 0xdcc8
dc89: jmp 0xdd85
dc8c: push word ptr [bp - 0x2]
dc8f: call 0x76b                             ; EXE rng_roll(n)
dc92: add al, byte ptr [bp - 0x5]
dc95: mov byte ptr [si + 0xaabe], al         ; tbl DS:AABE
dc99: mov ax, word ptr [bp - 0x2]
dc9c: cdq
dc9d: xor ax, dx
dc9f: sub ax, dx
dca1: mov cx, 0x2
dca4: sar ax, cl
dca6: xor ax, dx
dca8: sub ax, dx
dcaa: sub byte ptr [si + 0xaabe], al         ; tbl DS:AABE
dcae: jmp 0xdd85
dcb1: push word ptr [bp - 0x2]
dcb4: call 0x76b                             ; EXE rng_roll(n)
dcb7: add al, byte ptr [bp - 0x5]
dcba: mov byte ptr [si + 0xaabe], al         ; tbl DS:AABE
dcbe: mov ax, word ptr [bp - 0x2]
dcc1: cdq
dcc2: sub ax, dx
dcc4: sar ax
dcc6: jmp 0xdcaa
dcc8: push word ptr [bp - 0x2]
dccb: call 0x76b                             ; EXE rng_roll(n)
dcce: add al, byte ptr [bp - 0x5]
dcd1: mov byte ptr [si + 0xaabe], al         ; tbl DS:AABE
dcd5: mov ax, word ptr [bp - 0x2]
dcd8: mov cx, ax
dcda: shl ax
dcdc: add ax, cx
dcde: jmp 0xdc9c
dce0: mov al, byte ptr [bp - 0x5]
dce3: mov byte ptr [si + 0xaabe], al         ; tbl DS:AABE
dce7: jmp 0xdd85
dcea: cmp word ptr [bp - 0xa], 0x0
dcee: jge 0xdd58
dcf0: mov al, byte ptr [si + 0xc368]         ; tbl DS:C368
dcf4: sub ah, ah
dcf6: or ax, ax
dcf8: je 0xdd02
dcfa: dec ax
dcfb: je 0xdd26
dcfd: dec ax
dcfe: je 0xdd40
dd00: jmp 0xdd5f
dd02: push word ptr [bp - 0x2]
dd05: call 0x76b                             ; EXE rng_roll(n)
dd08: add al, byte ptr [bp - 0x5]
dd0b: mov byte ptr [si + 0xaabe], al         ; tbl DS:AABE
dd0f: mov ax, word ptr [bp - 0x2]
dd12: cdq
dd13: xor ax, dx
dd15: sub ax, dx
dd17: mov cx, 0x3
dd1a: sar ax, cl
dd1c: xor ax, dx
dd1e: sub ax, dx
dd20: sub byte ptr [si + 0xaabe], al         ; tbl DS:AABE
dd24: jmp 0xdd5f
dd26: push word ptr [bp - 0x2]
dd29: call 0x76b                             ; EXE rng_roll(n)
dd2c: add al, byte ptr [bp - 0x5]
dd2f: mov byte ptr [si + 0xaabe], al         ; tbl DS:AABE
dd33: mov ax, word ptr [bp - 0x2]
dd36: cdq
dd37: xor ax, dx
dd39: sub ax, dx
dd3b: mov cx, 0x2
dd3e: jmp 0xdd1a
dd40: push word ptr [bp - 0x2]
dd43: call 0x76b                             ; EXE rng_roll(n)
dd46: add al, byte ptr [bp - 0x5]
dd49: mov byte ptr [si + 0xaabe], al         ; tbl DS:AABE
dd4d: mov ax, word ptr [bp - 0x2]
dd50: mov cx, ax
dd52: shl ax
dd54: add ax, cx
dd56: jmp 0xdd12
dd58: mov al, byte ptr [bp - 0x5]
dd5b: mov byte ptr [si + 0xaabe], al         ; tbl DS:AABE
dd5f: mov ax, 0x3
dd62: jmp 0xdd7a
dd64: mov al, byte ptr [bp - 0x5]
dd67: mov byte ptr [si + 0xaabe], al         ; tbl DS:AABE
dd6b: mov ax, 0x8
dd6e: jmp 0xdd7a
dd70: mov al, byte ptr [bp - 0x5]
dd73: mov byte ptr [si + 0xaabe], al         ; tbl DS:AABE
dd77: mov ax, 0x4
dd7a: push ax
dd7b: call 0x76b                             ; EXE rng_roll(n)
dd7e: or ax, ax
dd80: jne 0xdd85
dd82: mov byte ptr [di], 0x9
dd85: mov al, byte ptr [di - 0x4]
dd88: cmp byte ptr [si + 0xaabe], al         ; tbl DS:AABE
dd8c: jae 0xdd95
dd8e: mov al, byte ptr [si + 0xaabe]         ; tbl DS:AABE
dd92: mov byte ptr [di - 0x4], al
dd95: mov al, byte ptr [di - 0x4]
dd98: cmp al, 0x14
dd9a: jae 0xdd9e
dd9c: mov al, 0x14
dd9e: mov byte ptr [di - 0x4], al
dda1: add di, 0x12
dda4: add word ptr [bp - 0xc], 0x4
dda8: add word ptr [bp - 0x8], 0x4
ddac: inc si
ddad: cmp word ptr [bp - 0x8], 0xc318
ddb2: jae 0xddb7
ddb4: jmp 0xdafb
ddb7: pop si
ddb8: pop di
ddb9: mov sp, bp
ddbb: pop bp
ddbc: ret

; ---- func DDBD  size 0x53  bp frame, locals 0, ret 0x2  callers: DABA
; DDBD(chariot): opponent steers left (G)
ddbd: push bp
ddbe: mov bp, sp
ddc0: mov al, byte ptr [0xc346]
ddc3: and al, 0xf
ddc5: cmp al, 0x3
ddc7: jne 0xdde8
ddc9: mov ax, 0x12
ddcc: mul word ptr [bp + 0x4]
ddcf: mov bx, ax
ddd1: mov al, 0xa
ddd3: mov byte ptr [bx + 0xc2cc], al         ; tbl DS:C2CC
ddd7: mov bx, word ptr [bp + 0x4]
ddda: mov byte ptr [bx + 0x8ba8], al         ; tbl DS:8BA8
ddde: mov byte ptr [bx + 0x8bac], 0x19       ; tbl DS:8BAC
dde3: mov ax, 0xffff
dde6: jmp 0xde0a
dde8: mov al, byte ptr [0xc344]
ddeb: and al, 0xf
dded: cmp al, 0x3
ddef: jne 0xde08
ddf1: mov ax, 0x12
ddf4: mul word ptr [bp + 0x4]
ddf7: mov bx, ax
ddf9: mov byte ptr [bx + 0xc2cc], 0x3        ; tbl DS:C2CC
ddfe: mov bx, word ptr [bp + 0x4]
de01: mov byte ptr [bx + 0x8ba8], 0x0        ; tbl DS:8BA8
de06: jmp 0xddde
de08: sub ax, ax
de0a: mov sp, bp
de0c: pop bp
de0d: ret 0x2

; ---- func DE10  size 0x53  bp frame, locals 0, ret 0x2  callers: DABA
; DE10(chariot): opponent steers right (G)
de10: push bp
de11: mov bp, sp
de13: mov al, byte ptr [0xc348]
de16: and al, 0xf
de18: cmp al, 0x3
de1a: jne 0xde3b
de1c: mov ax, 0x12
de1f: mul word ptr [bp + 0x4]
de22: mov bx, ax
de24: mov al, 0x9
de26: mov byte ptr [bx + 0xc2cc], al         ; tbl DS:C2CC
de2a: mov bx, word ptr [bp + 0x4]
de2d: mov byte ptr [bx + 0x8ba8], al         ; tbl DS:8BA8
de31: mov byte ptr [bx + 0x8bac], 0x19       ; tbl DS:8BAC
de36: mov ax, 0xffff
de39: jmp 0xde5d
de3b: mov al, byte ptr [0xc34a]
de3e: and al, 0xf
de40: cmp al, 0x3
de42: jne 0xde5b
de44: mov ax, 0x12
de47: mul word ptr [bp + 0x4]
de4a: mov bx, ax
de4c: mov byte ptr [bx + 0xc2cc], 0x2        ; tbl DS:C2CC
de51: mov bx, word ptr [bp + 0x4]
de54: mov byte ptr [bx + 0x8ba8], 0x0        ; tbl DS:8BA8
de59: jmp 0xde31
de5b: sub ax, ax
de5d: mov sp, bp
de5f: pop bp
de60: ret 0x2
de63: db 0x00                                ; word-alignment pad

; ---- func DE64  size 0xf3  bp frame, locals 0, ret 0x6  callers: B11C
; DE64(pattern, y, x): hand-written: plot a 16-pixel pattern word on row y, per video mode through DS:8BB0 (D)
de64: push bp
de65: mov bp, sp
de67: push bx
de68: push cx
de69: push dx
de6a: push es
de6b: push si
de6c: push di
de6d: mov ax, word ptr [0xc0d4]
de70: mov es, ax
de72: mov dx, word ptr [bp + 0x6]
de75: mov cx, word ptr [0xc0dc]
de79: sub cx, dx
de7b: js 0xde88
de7d: cmp cx, 0x10
de80: jge 0xdeba
de82: shl word ptr [bp + 0x4], cl
de85: add word ptr [bp + 0x6], cx
de88: mov bx, word ptr [0xc0e0]
de8c: sub bx, dx
de8e: jle 0xdeba
de90: cmp bx, 0x10
de93: jge 0xde9e
de95: shl bx
de97: mov dx, word ptr [bx + 0x8bb8]         ; tbl DS:8BB8
de9b: and word ptr [bp + 0x4], dx
de9e: mov bx, word ptr [0x9b26]
dea2: mov ax, word ptr [bx + 0x8bb0]         ; tbl DS:8BB0
dea6: mov bx, word ptr [bp + 0x6]
dea9: mov di, word ptr [0xc0d0]
dead: shl bx
deaf: mov di, word ptr [bx + di]
deb1: add di, word ptr [0xc0d2]
deb5: mov cx, word ptr [bp + 0x4]
deb8: jmp ax                                 ; jmp ax = W(DS:8BB0 + W(9B26)): per video mode DEBA / DEC4 / DEFE / DEDB
deba: pop di
debb: pop si
debc: pop es
debd: pop dx
debe: pop cx
debf: pop bx
dec0: pop bp
dec1: ret 0x6
dec4: add di, word ptr [bp + 0x8]
dec7: mov ax, 0x5652
deca: shl cx
decc: jae 0xded1
dece: mov word ptr es:[di], ax
ded1: add di, word ptr [0xc0d6]
ded5: or cx, cx
ded7: jne 0xdeca
ded9: je 0xdeba
dedb: mov si, word ptr [0xc0d0]
dedf: mov dx, word ptr [bp + 0x8]
dee2: shr dx
dee4: mov al, -0x8
dee6: shl cx
dee8: jae 0xdef5
deea: mov di, word ptr [bx + si]
deec: add di, word ptr [0xc0d2]
def0: add di, dx
def2: mov byte ptr es:[di], al
def5: add bx, 0x2
def8: or cx, cx
defa: jne 0xdee6
defc: je 0xdeba
defe: mov ax, word ptr [bp + 0x8]
df01: shr ax
df03: shr ax
df05: shr ax
df07: add di, ax
df09: mov dx, 0x3ce
df0c: mov ah, 0x30
df0e: mov al, 0x8
df10: out dx, ax
df11: mov dx, 0x3ce
df14: mov ah, 0x8
df16: mov al, 0x1
df18: out dx, ax
df19: mov dx, 0x3ce
df1c: mov ah, 0x8
df1e: mov al, 0x0
df20: out dx, ax
df21: mov dx, word ptr [0xc0d6]
df25: shr dx
df27: shr dx
df29: shr dx
df2b: mov ah, 0x20
df2d: shl cx
df2f: jae 0xdf36
df31: mov al, ah
df33: xchg byte ptr es:[di], al
df36: add di, dx
df38: or cx, cx
df3a: jne 0xdf2d
df3c: mov dx, 0x3ce
df3f: mov ah, -0x1
df41: mov al, 0x8
df43: out dx, ax
df44: mov dx, 0x3ce
df47: mov ah, 0x0
df49: mov al, 0x1
df4b: out dx, ax
df4c: mov dx, 0x3ce
df4f: mov ah, 0x0
df51: mov al, 0x0
df53: out dx, ax
df54: jmp 0xdeba
df57: db 0x00                                ; word-alignment pad

; ---- func DF58  size 0x1c  no bp frame, ret
; ENTRY (EXE thunk B092): the victory sequence: song 0, DF74 statistics, BDA4, BF6A(1) triumph, C2C2, credits thunk B0CE (D)
df58: sub al, al
df5a: push ax
df5b: call 0x5d73                            ; EXE music_load(n)
df5e: call 0x1d19                            ; EXE mouse cursor hide (G)
df61: call 0xdf74
df64: call 0xbda4
df67: mov al, 0x1
df69: push ax
df6a: call 0xbf6a
df6d: call 0xc2c2
df70: call 0xb0ce                            ; EXE thunk: section 1 B9A1 = credits
df73: ret

; ---- func DF74  size 0x50e  bp frame, locals 62, ret  callers: DF58
; victory statistics: "THE MAKING OF A CONQUEROR" box built in AC60, then the DS:8BD8 epilogue box (D)
df74: push bp
df75: mov bp, sp
df77: sub sp, 0x3e
df7a: push di
df7b: push si
df7c: mov al, byte ptr [0x9b5c]
df7f: sub ah, ah
df81: mov cl, byte ptr [0x9b5d]
df85: sub ch, ch
df87: add ax, cx
df89: mov word ptr [bp - 0x4], ax
df8c: mov ax, 0x8cd0                         ; "THE MAKING OF A CONQUEROR\nIn becoming Caesar, you "
df8f: push ax
df90: mov ax, 0xac60
df93: push ax
df94: call 0x9e32                            ; EXE strcpy
df97: pop bx
df98: pop bx
df99: cmp word ptr [0xc106], 0x0
df9e: je 0xdff9
dfa0: mov ax, 0x8d03                         ; "successfully negotiated "
dfa3: push ax
dfa4: mov ax, 0xac60
dfa7: push ax
dfa8: call 0x9df2                            ; EXE strcat
dfab: pop bx
dfac: pop bx
dfad: mov ax, 0xa
dfb0: push ax
dfb1: lea ax, [bp - 0x3e]
dfb4: push ax
dfb5: mov ax, word ptr [0xc106]
dfb8: cdq
dfb9: push dx
dfba: push ax
dfbb: call 0xa022                            ; EXE ltoa(lo, hi, buf, 10)
dfbe: add sp, 0x8
dfc1: push ax
dfc2: mov ax, 0xac60
dfc5: push ax
dfc6: call 0x9df2                            ; EXE strcat
dfc9: pop bx
dfca: pop bx
dfcb: mov ax, 0x8d1c                         ; " alliance"
dfce: push ax
dfcf: mov ax, 0xac60
dfd2: push ax
dfd3: call 0x9df2                            ; EXE strcat
dfd6: pop bx
dfd7: pop bx
dfd8: cmp word ptr [0xc106], 0x1
dfdd: je 0xdfec
dfdf: mov ax, 0x8d26
dfe2: push ax
dfe3: mov ax, 0xac60
dfe6: push ax
dfe7: call 0x9df2                            ; EXE strcat
dfea: pop bx
dfeb: pop bx
dfec: mov ax, 0x8d28
dfef: push ax
dff0: mov ax, 0xac60
dff3: push ax
dff4: call 0x9df2                            ; EXE strcat
dff7: pop bx
dff8: pop bx
dff9: mov ax, 0x8d2b                         ; "won "
dffc: push ax
dffd: mov ax, 0xac60
e000: push ax
e001: call 0x9df2                            ; EXE strcat
e004: pop bx
e005: pop bx
e006: mov ax, 0xa
e009: push ax
e00a: lea cx, [bp - 0x3e]
e00d: push cx
e00e: mov ax, word ptr [0xc0fa]
e011: cdq
e012: push dx
e013: push ax
e014: call 0xa022                            ; EXE ltoa(lo, hi, buf, 10)
e017: add sp, 0x8
e01a: push ax
e01b: mov ax, 0xac60
e01e: push ax
e01f: call 0x9df2                            ; EXE strcat
e022: pop bx
e023: pop bx
e024: mov ax, 0x8d30                         ; " battles and lost "
e027: push ax
e028: mov ax, 0xac60
e02b: push ax
e02c: call 0x9df2                            ; EXE strcat
e02f: pop bx
e030: pop bx
e031: mov ax, 0xa
e034: push ax
e035: lea cx, [bp - 0x3e]
e038: push cx
e039: mov ax, word ptr [0xc0fa]
e03c: cdq
e03d: mov bx, ax
e03f: mov ax, word ptr [0xc0f8]
e042: mov si, dx
e044: cdq
e045: sub ax, bx
e047: sbb dx, si
e049: push dx
e04a: push ax
e04b: call 0xa022                            ; EXE ltoa(lo, hi, buf, 10)
e04e: add sp, 0x8
e051: push ax
e052: mov ax, 0xac60
e055: push ax
e056: call 0x9df2                            ; EXE strcat
e059: pop bx
e05a: pop bx
e05b: mov ax, 0x8d43                         ; " battles,\nin which "
e05e: push ax
e05f: mov ax, 0xac60
e062: push ax
e063: call 0x9df2                            ; EXE strcat
e066: pop bx
e067: pop bx
e068: mov ax, 0xa
e06b: push ax
e06c: lea cx, [bp - 0x3e]
e06f: push cx
e070: push word ptr [0xc088]
e074: push word ptr [0xc086]
e078: call 0xa022                            ; EXE ltoa(lo, hi, buf, 10)
e07b: add sp, 0x8
e07e: push ax
e07f: mov ax, 0xac60
e082: push ax
e083: call 0x9df2                            ; EXE strcat
e086: pop bx
e087: pop bx
e088: mov ax, 0x8d57                         ; " legionnaires were killed and "
e08b: push ax
e08c: mov ax, 0xac60
e08f: push ax
e090: call 0x9df2                            ; EXE strcat
e093: pop bx
e094: pop bx
e095: mov ax, 0xa
e098: push ax
e099: lea ax, [bp - 0x3e]
e09c: push ax
e09d: push word ptr [0xc084]
e0a1: push word ptr [0xc082]
e0a5: call 0xa022                            ; EXE ltoa(lo, hi, buf, 10)
e0a8: add sp, 0x8
e0ab: push ax
e0ac: mov ax, 0xac60
e0af: push ax
e0b0: call 0x9df2                            ; EXE strcat
e0b3: pop bx
e0b4: pop bx
e0b5: mov ax, 0x8d76                         ; " enemy soldiers\nwere slain.  "
e0b8: push ax
e0b9: mov ax, 0xac60
e0bc: push ax
e0bd: call 0x9df2                            ; EXE strcat
e0c0: pop bx
e0c1: pop bx
e0c2: cmp word ptr [0xc0fc], 0x0
e0c7: jne 0xe0cc
e0c9: jmp 0xe1e0
e0cc: mov ax, 0x8d94                         ; "You "
e0cf: push ax
e0d0: mov ax, 0xac60
e0d3: push ax
e0d4: call 0x9df2                            ; EXE strcat
e0d7: pop bx
e0d8: pop bx
e0d9: cmp word ptr [0xc0fe], 0x0
e0de: je 0xe118
e0e0: mov ax, 0x8d99                         ; "won "
e0e3: push ax
e0e4: mov ax, 0xac60
e0e7: push ax
e0e8: call 0x9df2                            ; EXE strcat
e0eb: pop bx
e0ec: pop bx
e0ed: mov ax, 0xa
e0f0: push ax
e0f1: lea ax, [bp - 0x3e]
e0f4: push ax
e0f5: mov ax, word ptr [0xc0fe]
e0f8: cdq
e0f9: push dx
e0fa: push ax
e0fb: call 0xa022                            ; EXE ltoa(lo, hi, buf, 10)
e0fe: add sp, 0x8
e101: push ax
e102: mov ax, 0xac60
e105: push ax
e106: call 0x9df2                            ; EXE strcat
e109: pop bx
e10a: pop bx
e10b: mov ax, 0x8d9e                         ; " fleet battles\nand "
e10e: push ax
e10f: mov ax, 0xac60
e112: push ax
e113: call 0x9df2                            ; EXE strcat
e116: pop bx
e117: pop bx
e118: mov ax, 0x8db2                         ; "lost "
e11b: push ax
e11c: mov ax, 0xac60
e11f: push ax
e120: call 0x9df2                            ; EXE strcat
e123: pop bx
e124: pop bx
e125: mov ax, word ptr [0xc0fe]
e128: sub ax, word ptr [0xc0fc]
e12c: inc ax
e12d: jne 0xe134
e12f: mov ax, 0x8db8                         ; "one fleet battle"
e132: jmp 0xe16f
e134: mov ax, word ptr [0xc0fe]
e137: cmp word ptr [0xc0fc], ax
e13b: jne 0xe142
e13d: mov ax, 0x8dc9                         ; "no"
e140: jmp 0xe162
e142: mov ax, 0xa
e145: push ax
e146: lea ax, [bp - 0x3e]
e149: push ax
e14a: mov ax, word ptr [0xc0fe]
e14d: cdq
e14e: mov cx, ax
e150: mov ax, word ptr [0xc0fc]
e153: mov bx, dx
e155: cdq
e156: sub ax, cx
e158: sbb dx, bx
e15a: push dx
e15b: push ax
e15c: call 0xa022                            ; EXE ltoa(lo, hi, buf, 10)
e15f: add sp, 0x8
e162: push ax
e163: mov ax, 0xac60
e166: push ax
e167: call 0x9df2                            ; EXE strcat
e16a: pop bx
e16b: pop bx
e16c: mov ax, 0x8dcc                         ; " fleet battles"
e16f: push ax
e170: mov ax, 0xac60
e173: push ax
e174: call 0x9df2                            ; EXE strcat
e177: pop bx
e178: pop bx
e179: mov ax, 0x8ddb                         ; ", in which "
e17c: push ax
e17d: mov ax, 0xac60
e180: push ax
e181: call 0x9df2                            ; EXE strcat
e184: pop bx
e185: pop bx
e186: mov ax, 0xa
e189: push ax
e18a: lea cx, [bp - 0x3e]
e18d: push cx
e18e: push word ptr [0xc090]
e192: push word ptr [0xc08e]
e196: call 0xa022                            ; EXE ltoa(lo, hi, buf, 10)
e199: add sp, 0x8
e19c: push ax
e19d: mov ax, 0xac60
e1a0: push ax
e1a1: call 0x9df2                            ; EXE strcat
e1a4: pop bx
e1a5: pop bx
e1a6: mov ax, 0x8de7                         ; "\nof your ships were sunk and "
e1a9: push ax
e1aa: mov ax, 0xac60
e1ad: push ax
e1ae: call 0x9df2                            ; EXE strcat
e1b1: pop bx
e1b2: pop bx
e1b3: mov ax, 0xa
e1b6: push ax
e1b7: lea ax, [bp - 0x3e]
e1ba: push ax
e1bb: push word ptr [0xc08c]
e1bf: push word ptr [0xc08a]
e1c3: call 0xa022                            ; EXE ltoa(lo, hi, buf, 10)
e1c6: add sp, 0x8
e1c9: push ax
e1ca: mov ax, 0xac60
e1cd: push ax
e1ce: call 0x9df2                            ; EXE strcat
e1d1: pop bx
e1d2: pop bx
e1d3: mov ax, 0x8e05                         ; " enemy ships were sunk.\n\n"
e1d6: push ax
e1d7: mov ax, 0xac60
e1da: push ax
e1db: call 0x9df2                            ; EXE strcat
e1de: pop bx
e1df: pop bx
e1e0: cmp word ptr [0xc100], 0x0
e1e5: jne 0xe1ea
e1e7: jmp 0xe290
e1ea: mov ax, 0x8e1f                         ; "You sponsored "
e1ed: push ax
e1ee: mov ax, 0xac60
e1f1: push ax
e1f2: call 0x9df2                            ; EXE strcat
e1f5: pop bx
e1f6: pop bx
e1f7: mov ax, 0xa
e1fa: push ax
e1fb: lea ax, [bp - 0x3e]
e1fe: push ax
e1ff: mov ax, word ptr [0xc100]
e202: cdq
e203: push dx
e204: push ax
e205: call 0xa022                            ; EXE ltoa(lo, hi, buf, 10)
e208: add sp, 0x8
e20b: push ax
e20c: mov ax, 0xac60
e20f: push ax
e210: call 0x9df2                            ; EXE strcat
e213: pop bx
e214: pop bx
e215: mov ax, 0x8e2e                         ; " games, entered "
e218: push ax
e219: mov ax, 0xac60
e21c: push ax
e21d: call 0x9df2                            ; EXE strcat
e220: pop bx
e221: pop bx
e222: cmp word ptr [0xc102], 0x0
e227: jne 0xe22e
e229: mov ax, 0x8e3f                         ; "no"
e22c: jmp 0xe242
e22e: mov ax, 0xa
e231: push ax
e232: lea ax, [bp - 0x3e]
e235: push ax
e236: mov ax, word ptr [0xc102]
e239: cdq
e23a: push dx
e23b: push ax
e23c: call 0xa022                            ; EXE ltoa(lo, hi, buf, 10)
e23f: add sp, 0x8
e242: push ax
e243: mov ax, 0xac60
e246: push ax
e247: call 0x9df2                            ; EXE strcat
e24a: pop bx
e24b: pop bx
e24c: mov ax, 0x8e42                         ; " races,\nwinning "
e24f: push ax
e250: mov ax, 0xac60
e253: push ax
e254: call 0x9df2                            ; EXE strcat
e257: pop bx
e258: pop bx
e259: cmp word ptr [0xc104], 0x0
e25e: jne 0xe265
e260: mov ax, 0x8e53                         ; "none"
e263: jmp 0xe279
e265: mov ax, 0xa
e268: push ax
e269: lea ax, [bp - 0x3e]
e26c: push ax
e26d: mov ax, word ptr [0xc104]
e270: cdq
e271: push dx
e272: push ax
e273: call 0xa022                            ; EXE ltoa(lo, hi, buf, 10)
e276: add sp, 0x8
e279: push ax
e27a: mov ax, 0xac60
e27d: push ax
e27e: call 0x9df2                            ; EXE strcat
e281: pop bx
e282: pop bx
e283: mov ax, 0x8e58
e286: push ax
e287: mov ax, 0xac60
e28a: push ax
e28b: call 0x9df2                            ; EXE strcat
e28e: pop bx
e28f: pop bx
e290: mov ax, 0x8e5c                         ; "You collected "
e293: push ax
e294: mov ax, 0xac60
e297: push ax
e298: call 0x9df2                            ; EXE strcat
e29b: pop bx
e29c: pop bx
e29d: mov ax, 0xa
e2a0: push ax
e2a1: lea cx, [bp - 0x3e]
e2a4: push cx
e2a5: push word ptr [0xc094]
e2a9: push word ptr [0xc092]
e2ad: call 0xa022                            ; EXE ltoa(lo, hi, buf, 10)
e2b0: add sp, 0x8
e2b3: push ax
e2b4: mov ax, 0xac60
e2b7: push ax
e2b8: call 0x9df2                            ; EXE strcat
e2bb: pop bx
e2bc: pop bx
e2bd: mov ax, 0x8e6b                         ; " talents in tribute, "
e2c0: push ax
e2c1: mov ax, 0xac60
e2c4: push ax
e2c5: call 0x9df2                            ; EXE strcat
e2c8: pop bx
e2c9: pop bx
e2ca: mov ax, 0xa
e2cd: push ax
e2ce: lea cx, [bp - 0x3e]
e2d1: push cx
e2d2: push word ptr [0xc098]
e2d6: push word ptr [0xc096]
e2da: call 0xa022                            ; EXE ltoa(lo, hi, buf, 10)
e2dd: add sp, 0x8
e2e0: push ax
e2e1: mov ax, 0xac60
e2e4: push ax
e2e5: call 0x9df2                            ; EXE strcat
e2e8: pop bx
e2e9: pop bx
e2ea: mov ax, 0x8e81                         ; " talents in bets,\nand "
e2ed: push ax
e2ee: mov ax, 0xac60
e2f1: push ax
e2f2: call 0x9df2                            ; EXE strcat
e2f5: pop bx
e2f6: pop bx
e2f7: mov ax, 0xa
e2fa: push ax
e2fb: lea ax, [bp - 0x3e]
e2fe: push ax
e2ff: push word ptr [0xc09c]
e303: push word ptr [0xc09a]
e307: call 0xa022                            ; EXE ltoa(lo, hi, buf, 10)
e30a: add sp, 0x8
e30d: push ax
e30e: mov ax, 0xac60
e311: push ax
e312: call 0x9df2                            ; EXE strcat
e315: pop bx
e316: pop bx
e317: mov ax, 0x8e98                         ; " talents in plunder."
e31a: push ax
e31b: mov ax, 0xac60
e31e: push ax
e31f: call 0x9df2                            ; EXE strcat
e322: pop bx
e323: pop bx
e324: mov ax, 0xac60
e327: push ax
e328: sub cx, cx
e32a: push cx
e32b: mov dx, 0x1
e32e: push dx
e32f: mov bx, 0x9f
e332: push bx
e333: mov si, 0x63
e336: push si
e337: lea di, [bp - 0x14]
e33a: push di
e33b: call 0x8c9f                            ; EXE text box layout 8C9F(&rect, str, cx, cy)
e33e: push si
e33f: lea di, [bp - 0xc]
e342: mov si, ax
e344: push ss
e345: pop es
e346: movsw word ptr es:[di], word ptr [si]
e347: movsw word ptr es:[di], word ptr [si]
e348: movsw word ptr es:[di], word ptr [si]
e349: movsw word ptr es:[di], word ptr [si]
e34a: pop si
e34b: sub word ptr [bp - 0xc], 0x7
e34f: push word ptr [bp - 0xc]
e352: sub word ptr [bp - 0xa], 0x7
e356: push word ptr [bp - 0xa]
e359: add word ptr [bp - 0x8], 0x7
e35d: push word ptr [bp - 0x8]
e360: add word ptr [bp - 0x6], 0x7
e364: push word ptr [bp - 0x6]
e367: call 0x691b                            ; EXE save rect screen -> work buffer
e36a: push word ptr [bp - 0xc]
e36d: push word ptr [bp - 0xa]
e370: push word ptr [bp - 0x8]
e373: push word ptr [bp - 0x6]
e376: push word ptr [0x6346]
e37a: call 0x3134                            ; EXE ui_frame box(x1, y1, x2, y2, fill)
e37d: mov word ptr [0x9b69], 0x0
e383: mov ax, 0xac60
e386: push ax
e387: mov ax, word ptr [bp - 0x8]
e38a: add ax, word ptr [bp - 0xc]
e38d: cdq
e38e: sub ax, dx
e390: sar ax
e392: mov word ptr [bp - 0x2], ax
e395: push ax
e396: mov ax, word ptr [bp - 0xa]
e399: add ax, 0x7
e39c: push ax
e39d: call 0x8be4                            ; EXE centred_text(str, cx, y)
e3a0: mov word ptr [0x9b69], 0xf
e3a6: push word ptr [0x3e40]
e3aa: push word ptr [bp - 0x2]
e3ad: mov ax, word ptr [bp - 0x6]
e3b0: sub ax, word ptr [bp - 0x4]
e3b3: sub ax, 0x5
e3b6: push ax
e3b7: call 0x8be4                            ; EXE centred_text(str, cx, y)
e3ba: call 0x35f9                            ; EXE wait: release, click or key, release (35DB/359B/35DB)
e3bd: push word ptr [bp - 0xc]
e3c0: push word ptr [bp - 0xa]
e3c3: push word ptr [bp - 0x8]
e3c6: push word ptr [bp - 0x6]
e3c9: call 0x6973                            ; EXE restore rect work buffer -> screen
e3cc: push word ptr [0x8cce]
e3d0: mov ax, 0xac60
e3d3: push ax
e3d4: call 0x9e32                            ; EXE strcpy
e3d7: pop bx
e3d8: pop bx
e3d9: mov ax, 0xac60
e3dc: push ax
e3dd: sub cx, cx
e3df: push cx
e3e0: mov cx, 0x1
e3e3: push cx
e3e4: mov cx, 0x9f
e3e7: push cx
e3e8: push si
e3e9: lea cx, [bp - 0x1c]
e3ec: push cx
e3ed: call 0x8c9f                            ; EXE text box layout 8C9F(&rect, str, cx, cy)
e3f0: lea di, [bp - 0xc]
e3f3: mov si, ax
e3f5: push ss
e3f6: pop es
e3f7: movsw word ptr es:[di], word ptr [si]
e3f8: movsw word ptr es:[di], word ptr [si]
e3f9: movsw word ptr es:[di], word ptr [si]
e3fa: movsw word ptr es:[di], word ptr [si]
e3fb: sub word ptr [bp - 0xc], 0x7
e3ff: push word ptr [bp - 0xc]
e402: sub word ptr [bp - 0xa], 0x7
e406: push word ptr [bp - 0xa]
e409: add word ptr [bp - 0x8], 0x7
e40d: push word ptr [bp - 0x8]
e410: add word ptr [bp - 0x6], 0x7
e414: push word ptr [bp - 0x6]
e417: call 0x691b                            ; EXE save rect screen -> work buffer
e41a: push word ptr [bp - 0xc]
e41d: push word ptr [bp - 0xa]
e420: push word ptr [bp - 0x8]
e423: push word ptr [bp - 0x6]
e426: push word ptr [0x6346]
e42a: call 0x3134                            ; EXE ui_frame box(x1, y1, x2, y2, fill)
e42d: mov word ptr [0x9b69], 0x0
e433: mov ax, 0xac60
e436: push ax
e437: mov ax, word ptr [bp - 0x8]
e43a: add ax, word ptr [bp - 0xc]
e43d: cdq
e43e: sub ax, dx
e440: sar ax
e442: mov word ptr [bp - 0x2], ax
e445: push ax
e446: mov ax, word ptr [bp - 0xa]
e449: add ax, 0x7
e44c: push ax
e44d: call 0x8be4                            ; EXE centred_text(str, cx, y)
e450: mov word ptr [0x9b69], 0xf
e456: push word ptr [0x3e40]
e45a: push word ptr [bp - 0x2]
e45d: mov ax, word ptr [bp - 0x6]
e460: sub ax, word ptr [bp - 0x4]
e463: sub ax, 0x5
e466: push ax
e467: call 0x8be4                            ; EXE centred_text(str, cx, y)
e46a: call 0x35f9                            ; EXE wait: release, click or key, release (35DB/359B/35DB)
e46d: push word ptr [bp - 0xc]
e470: push word ptr [bp - 0xa]
e473: push word ptr [bp - 0x8]
e476: push word ptr [bp - 0x6]
e479: call 0x6973                            ; EXE restore rect work buffer -> screen
e47c: pop si
e47d: pop di
e47e: mov sp, bp
e480: pop bp
e481: ret

; ---- section padding
e482: db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ; section padding (zeros to the end of section 4)
