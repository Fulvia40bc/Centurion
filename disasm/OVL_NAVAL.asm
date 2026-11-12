; OVL0 section 5 (file 0x8550-0xB3F0) = naval battle, loaded at 1331:0000 = 0823:B0E0. Entries 0xB0E0 (EXE thunk 0xB09E,
;   called from fleet move 0x2C24) and 0xB12A (EXE thunk 0xB0C2, called from 0x4B22 and End Turn 0x5221).
; llvm-objdump code16 listing (clang --target=i386-unknown-elf .incbin + llvm-objdump -M intel --adjust-vma), addresses are
;   0823 offsets. Resynced per function: the section is disassembled one function at a time from every
;   push bp/mov bp,sp prologue and every near-call target, so the word-alignment pad bytes (00) between functions
;   are listed as db and no longer swallow the next prologue (the linear sweep decoded B7F9/BD89/C4D7/C845/D241/
;   D3F1/DA4F as "add [di-0x75],dl"). Every branch target lands on an instruction start; no jump tables, no
;   data inside code. Bytes 0xDF7A-0xDF7F are the section's zero padding (db).
; Notation: absolute DS operands are printed unsigned ([0xc37b], llvm prints [-0x3c85]); call targets are
;   masked to 16 bits (llvm prints 0x1076b for 0x076B). llvm's code16 "cwde" = cbw (98h), "cdq" = cwd (99h).
;   Comments: EXE routine names (from the notes; (G) = guessed here), "tbl DS:xxxx" for reg+disp table bases,
;   and the static DS string (OVL0.OVL 0xB400 = DS:0000) an immediate points at. Index: docs/NAVAL_INDEX.md.

; ---- func B0E0  size 0x4a  bp frame, locals 6, ret 0x4
; ENTRY (EXE thunk B09E): fleet enters a sea zone, fights enemy fleets 3..5 there
b0e0: push bp
b0e1: mov bp, sp
b0e3: sub sp, 0x6
b0e6: push si
b0e7: mov si, 0x1
b0ea: mov byte ptr [bp - 0x1], 0x3
b0ee: cmp byte ptr [bp - 0x1], 0x6
b0f2: jae 0xb121
b0f4: mov al, 0xc
b0f6: mul byte ptr [bp - 0x1]
b0f9: mov bx, ax
b0fb: mov al, byte ptr [bp + 0x6]
b0fe: cmp byte ptr [bx + 0x1114], al         ; tbl DS:1114
b102: jne 0xb11a
b104: cmp byte ptr [bx + 0x1116], -0x1       ; tbl DS:1116
b109: je 0xb11a
b10b: mov al, byte ptr [bp + 0x4]
b10e: push ax
b10f: mov al, byte ptr [bp - 0x1]
b112: push ax
b113: call 0xb12a
b116: test al, 0x2
b118: je 0xb11f
b11a: inc byte ptr [bp - 0x1]
b11d: jmp 0xb0ee
b11f: sub si, si
b121: mov ax, si
b123: pop si
b124: mov sp, bp
b126: pop bp
b127: ret 0x4

; ---- func B12A  size 0x475  bp frame, locals 24, ret 0x4  callers: B0E0
; ENTRY (EXE thunk B0C2): naval battle B12A(enemy fleet record, Roman fleet)
b12a: push bp
b12b: mov bp, sp
b12d: sub sp, 0x18
b130: push di
b131: push si
b132: sub di, di
b134: sub si, si
b136: mov word ptr [bp - 0xa], si
b139: mov word ptr [bp - 0x8], si
b13c: push si
b13d: call 0x1d55                            ; EXE mouse cursor push/hide
b140: mov al, byte ptr [bp + 0x6]
b143: push ax
b144: mov al, byte ptr [bp + 0x4]
b147: push ax
b148: call 0xbd8a
b14b: or ax, ax
b14d: jne 0xb152
b14f: jmp 0xb592
b152: mov word ptr [bp - 0xc], si
b155: inc word ptr [0xc0fc]
b159: mov al, byte ptr [bp + 0x4]
b15c: sub ah, ah
b15e: mov word ptr [bp - 0x10], ax
b161: mov cx, 0x6
b164: imul cx
b166: mov bx, ax
b168: mov word ptr [bp - 0xe], bx
b16b: mov al, byte ptr [0xc37a]
b16e: sub ah, ah
b170: add bx, ax
b172: shl bx
b174: dec word ptr [bx + 0x110e]             ; tbl DS:110E
b178: mov bx, ax
b17a: mov al, byte ptr [bx - 0x7122]         ; tbl DS:8EDE
b17e: mov word ptr [0xc37e], ax
b181: mov ax, 0xc
b184: imul word ptr [bp - 0x10]
b187: mov bx, ax
b189: mov word ptr [bp - 0x12], bx
b18c: mov al, byte ptr [bx + 0x1117]         ; tbl DS:1117
b190: mov byte ptr [bp - 0x1], al
b193: cmp al, -0x1
b195: je 0xb1a4
b197: mov al, 0xe
b199: mul byte ptr [bp - 0x1]
b19c: mov bx, ax
b19e: mov al, byte ptr [bx - 0x3ee2]         ; tbl DS:C11E
b1a2: jmp 0xb1b2
b1a4: mov cl, 0x5
b1a6: mov bl, byte ptr [bx + 0x1115]         ; tbl DS:1115
b1aa: sub bh, bh
b1ac: shl bx, cl
b1ae: mov al, byte ptr [bx + 0x527a]         ; tbl DS:527A
b1b2: mov byte ptr [0xc36e], al
b1b5: mov bl, byte ptr [0x491]
b1b9: sub bh, bh
b1bb: mov al, byte ptr [bx - 0x711a]         ; tbl DS:8EE6
b1bf: add byte ptr [0xc36e], al
b1c3: mov al, byte ptr [bp + 0x6]
b1c6: sub ah, ah
b1c8: mov word ptr [bp - 0x16], ax
b1cb: mov cx, 0x6
b1ce: imul cx
b1d0: mov bx, ax
b1d2: mov word ptr [bp - 0x14], bx
b1d5: mov al, byte ptr [0xc37b]
b1d8: sub ah, ah
b1da: add bx, ax
b1dc: shl bx
b1de: dec word ptr [bx + 0x110e]             ; tbl DS:110E
b1e2: mov bx, ax
b1e4: mov al, byte ptr [bx - 0x7122]         ; tbl DS:8EDE
b1e8: mov word ptr [0xc380], ax
b1eb: mov ax, 0xc
b1ee: imul word ptr [bp - 0x16]
b1f1: mov bx, ax
b1f3: mov word ptr [bp - 0x18], bx
b1f6: mov al, byte ptr [bx + 0x1117]         ; tbl DS:1117
b1fa: mov byte ptr [bp - 0x1], al
b1fd: cmp al, -0x1
b1ff: je 0xb20e
b201: mov al, 0x14
b203: mul byte ptr [bp - 0x1]
b206: mov bx, ax
b208: mov al, byte ptr [bx + 0x1923]         ; tbl DS:1923
b20c: jmp 0xb220
b20e: shr word ptr [0xc380]
b212: mov cl, 0x5
b214: mov bl, byte ptr [bx + 0x1115]         ; tbl DS:1115
b218: sub bh, bh
b21a: shl bx, cl
b21c: mov al, byte ptr [bx + 0x527a]         ; tbl DS:527A
b220: mov byte ptr [0xc36f], al
b223: mov bl, byte ptr [0x491]
b227: sub bh, bh
b229: mov al, byte ptr [bx - 0x7116]         ; tbl DS:8EEA
b22d: add byte ptr [0xc36f], al
b231: mov al, byte ptr [bp + 0x6]
b234: push ax
b235: mov al, byte ptr [bp + 0x4]
b238: push ax
b239: call 0xbeae
b23c: mov ax, 0xb9
b23f: mov word ptr [0xc0e0], ax
b242: mov word ptr [0xc1ea], ax
b245: mov word ptr [0x9ac6], ax
b248: or di, di
b24a: je 0xb24f
b24c: jmp 0xb419
b24f: push word ptr [0xc378]
b253: push word ptr [0xc376]
b257: push word ptr [0xc374]
b25b: push word ptr [0xc372]
b25f: call 0xc4f8
b262: mov word ptr [0xc382], ax
b265: mov ax, 0xc376
b268: push ax
b269: mov ax, 0xc372
b26c: push ax
b26d: call 0xc641
b270: mov byte ptr [0xc371], al
b273: sub al, 0x8
b275: mov byte ptr [0xc370], al
b278: and al, 0xf
b27a: mov byte ptr [0xc370], al
b27d: call 0xc366
b280: mov word ptr [bp - 0x2], ax
b283: call 0xb6a0
b286: mov ax, 0x1
b289: push ax
b28a: call 0xc3d3
b28d: sub ax, ax
b28f: push ax
b290: call 0xc3d3
b293: mov ax, 0x1
b296: push ax
b297: call 0xc422
b29a: sub ax, ax
b29c: push ax
b29d: call 0xc422
b2a0: mov ax, 0x1
b2a3: push ax
b2a4: call 0xb7fa
b2a7: sub ax, ax
b2a9: push ax
b2aa: call 0xb7fa
b2ad: cmp word ptr [bp - 0x8], di
b2b0: jne 0xb2b8
b2b2: call 0xd242
b2b5: mov word ptr [bp - 0x8], ax
b2b8: mov ax, 0x1
b2bb: push ax
b2bc: call 0xcf7f
b2bf: sub ax, ax
b2c1: push ax
b2c2: call 0xcf7f
b2c5: call 0xcfdd
b2c8: call 0xcd96
b2cb: mov si, ax
b2cd: mov ax, 0x1
b2d0: push ax
b2d1: call 0xcb11
b2d4: sub ax, ax
b2d6: push ax
b2d7: call 0xcb11
b2da: mov ax, 0x1
b2dd: push ax
b2de: call 0xc92b
b2e1: sub ax, ax
b2e3: push ax
b2e4: call 0xc92b
b2e7: mov ax, 0x1
b2ea: push ax
b2eb: call 0xc9a3
b2ee: sub ax, ax
b2f0: push ax
b2f1: call 0xc9a3
b2f4: mov ax, 0x8
b2f7: push ax
b2f8: call 0x183                             ; EXE ticks_since(n)
b2fb: cmp ax, word ptr [0xc386]
b2ff: jbe 0xb324
b301: mov ax, 0x8
b304: push ax
b305: call 0xd7                              ; EXE timer_start(n) (copy tick DS:42/44 to slot 46+4n)
b308: mov al, byte ptr [bp + 0x6]
b30b: push ax
b30c: mov al, byte ptr [bp + 0x4]
b30f: push ax
b310: call 0xda50
b313: mov ax, 0x9
b316: push ax
b317: call 0x76b                             ; EXE rng_roll(n)
b31a: inc ax
b31b: inc ax
b31c: mov cx, 0x48
b31f: mul cx
b321: mov word ptr [0xc386], ax
b324: call 0x1ae                             ; EXE ticks_since(0)
b327: cmp ax, 0x2
b32a: jbe 0xb376
b32c: call 0xf8                              ; EXE timer0_reset (= 00D7(0))
b32f: mov word ptr [0x9afa], 0x9
b335: sub ax, ax
b337: push ax
b338: push ax
b339: mov ax, 0x13f
b33c: push ax
b33d: mov ax, 0xb9
b340: push ax
b341: call 0x7e74                            ; EXE fill rect (colour, ...)
b344: call 0xb96a
b347: mov ax, 0x1
b34a: push ax
b34b: call 0xba7a
b34e: or si, ax
b350: sub ax, ax
b352: push ax
b353: call 0xba7a
b356: or si, ax
b358: mov ax, 0x1
b35b: push ax
b35c: call 0xccde
b35f: sub ax, ax
b361: push ax
b362: call 0xccde
b365: call 0xc71b
b368: mov al, byte ptr [bp + 0x6]
b36b: push ax
b36c: mov al, byte ptr [bp + 0x4]
b36f: push ax
b370: call 0xdc8e
b373: call 0x6887                            ; EXE flip: work buffer -> screen
b376: cmp byte ptr [0xc38f], 0x0
b37b: je 0xb39a
b37d: cmp byte ptr [0xc38e], 0x0
b382: je 0xb39a
b384: or si, si
b386: je 0xb3a0
b388: mov al, 0x10
b38a: push ax
b38b: call 0x5d29                            ; EXE music_play(n)
b38e: mov al, byte ptr [bp + 0x6]
b391: push ax
b392: mov al, byte ptr [bp + 0x4]
b395: push ax
b396: push si
b397: call 0xbb7e
b39a: mov di, 0xffff
b39d: jmp 0xb248
b3a0: mov word ptr [bp - 0x4], di
b3a3: cmp word ptr [bp - 0x2], di
b3a6: je 0xb3b4
b3a8: mov di, 0xffff
b3ab: mov ax, word ptr [bp - 0x2]
b3ae: mov word ptr [bp - 0x8], ax
b3b1: jmp 0xb248
b3b4: test byte ptr [bp - 0x8], 0x4
b3b8: je 0xb40e
b3ba: mov al, 0xd
b3bc: push ax
b3bd: call 0x5d29                            ; EXE music_play(n)
b3c0: call 0x67b6                            ; EXE screen restore 6973(0,0,319,199) unless [9B26]==2 path (G)
b3c3: call 0x6837                            ; EXE draw on the screen (6887 first if needed)
b3c6: mov word ptr [0x9afa], 0x9
b3cc: sub ax, ax
b3ce: push ax
b3cf: push ax
b3d0: mov cx, 0x13f
b3d3: push cx
b3d4: mov cx, 0xb9
b3d7: push cx
b3d8: call 0x7e74                            ; EXE fill rect (colour, ...)
b3db: mov ax, 0x9b56
b3de: mov di, ax
b3e0: mov si, 0xa05c
b3e3: push ds
b3e4: pop es
b3e5: mov cx, 0x5
b3e8: rep movsw word ptr es:[di], word ptr [si]
b3ea: mov ax, 0x935a                         ; "The tides of battle sweep\nyour flag ships apart.\nDo you wish to re-engage?"
b3ed: push ax
b3ee: call 0x5c20                            ; EXE yes_no box: 1 = Yes, 0 = No
b3f1: or ax, ax
b3f3: je 0xb39a
b3f5: mov word ptr [bp - 0x8], 0x0
b3fa: call 0xbe0f
b3fd: mov al, byte ptr [bp + 0x6]
b400: push ax
b401: mov al, byte ptr [bp + 0x4]
b404: push ax
b405: call 0xbeae
b408: mov di, word ptr [bp - 0x4]
b40b: jmp 0xb248
b40e: test byte ptr [bp - 0x8], 0x8
b412: jne 0xb417
b414: jmp 0xb248
b417: jmp 0xb39a
b419: mov di, word ptr [bp - 0xa]
b41c: mov al, 0xd
b41e: push ax
b41f: call 0x5d29                            ; EXE music_play(n)
b422: cmp byte ptr [0xc38f], 0x0
b427: jne 0xb436
b429: mov di, 0x1
b42c: add word ptr [0xc08e], 0x1
b431: adc word ptr [0xc090], 0x0
b436: cmp byte ptr [0xc38e], 0x0
b43b: jne 0xb44a
b43d: or di, 0x2
b440: add word ptr [0xc08a], 0x1
b445: adc word ptr [0xc08c], 0x0
b44a: test di, 0x1
b44e: jne 0xb45f
b450: mov bl, byte ptr [0xc37b]
b454: sub bh, bh
b456: add bx, word ptr [bp - 0x14]
b459: shl bx
b45b: inc word ptr [bx + 0x110e]             ; tbl DS:110E
b45f: test di, 0x2
b463: jne 0xb474
b465: mov bl, byte ptr [0xc37a]
b469: sub bh, bh
b46b: add bx, word ptr [bp - 0xe]
b46e: shl bx
b470: inc word ptr [bx + 0x110e]             ; tbl DS:110E
b474: mov byte ptr [0xc37c], 0x0
b479: call 0xc71b
b47c: call 0x67b6                            ; EXE screen restore 6973(0,0,319,199) unless [9B26]==2 path (G)
b47f: push di
b480: call 0xd3f2
b483: call 0xc312
b486: push di
b487: call 0xd60c
b48a: call 0x67b6                            ; EXE screen restore 6973(0,0,319,199) unless [9B26]==2 path (G)
b48d: call 0x6837                            ; EXE draw on the screen (6887 first if needed)
b490: mov word ptr [bp - 0xa], di
b493: cmp word ptr [bp - 0x8], 0x8
b497: jne 0xb49e
b499: mov byte ptr [0xc38e], 0x0
b49e: cmp byte ptr [0xc38f], 0x0
b4a3: je 0xb4ac
b4a5: cmp byte ptr [0xc38e], 0x0
b4aa: jne 0xb4c6
b4ac: mov al, byte ptr [bp + 0x6]
b4af: push ax
b4b0: mov al, byte ptr [bp + 0x4]
b4b3: push ax
b4b4: call 0xda50
b4b7: or ax, ax
b4b9: je 0xb4ac
b4bb: mov al, byte ptr [bp + 0x6]
b4be: push ax
b4bf: mov al, byte ptr [bp + 0x4]
b4c2: push ax
b4c3: call 0xdc8e
b4c6: sub si, si
b4c8: mov di, si
b4ca: mov word ptr [bp - 0x6], di
b4cd: mov al, 0xc
b4cf: mul byte ptr [bp + 0x4]
b4d2: add ax, 0x110e
b4d5: mov word ptr [bp - 0x4], ax
b4d8: mov al, 0xc
b4da: mul byte ptr [bp + 0x6]
b4dd: add ax, 0x110e
b4e0: mov word ptr [bp - 0x2], ax
b4e3: jmp 0xb4fb
b4e5: mov bx, word ptr [bp - 0x2]
b4e8: mov ax, word ptr [bx]
b4ea: add word ptr [bp - 0x6], ax
b4ed: mov bx, word ptr [bp - 0x4]
b4f0: add word ptr [bp - 0x4], 0x2
b4f4: add di, word ptr [bx]
b4f6: add word ptr [bp - 0x2], 0x2
b4fa: inc si
b4fb: cmp si, 0x3
b4fe: jb 0xb4e5
b500: mov dx, word ptr [bp - 0x6]
b503: mov si, word ptr [bp - 0xc]
b506: or dx, dx
b508: je 0xb513
b50a: or di, di
b50c: je 0xb513
b50e: mov si, 0x4
b511: jmp 0xb55b
b513: or dx, dx
b515: jne 0xb536
b517: mov bx, word ptr [bp - 0x18]
b51a: mov byte ptr [bx + 0x1116], -0x1       ; tbl DS:1116
b51f: cmp byte ptr [bx + 0x1117], -0x1       ; tbl DS:1117
b524: je 0xb533
b526: mov bx, word ptr [bp - 0x18]
b529: mov al, byte ptr [bx + 0x1117]         ; tbl DS:1117
b52d: sub ah, ah
b52f: push ax
b530: call 0x4816                            ; EXE legion_destroyed (L): +0x0D=0xFF, +0x0C=0xFF, commander status
b533: mov si, 0x1
b536: or di, di
b538: jne 0xb55b
b53a: mov bx, word ptr [bp - 0x12]
b53d: mov byte ptr [bx + 0x1116], -0x1       ; tbl DS:1116
b542: mov al, byte ptr [bx + 0x1117]         ; tbl DS:1117
b546: mov byte ptr [bp - 0x1], al
b549: cmp al, -0x1
b54b: je 0xb558
b54d: mov al, 0xe
b54f: mul byte ptr [bp - 0x1]
b552: mov bx, ax
b554: mov word ptr [bx - 0x3ee6], di         ; tbl DS:C11A
b558: or si, 0x2
b55b: mov bx, word ptr [bp - 0x16]
b55e: shl bx
b560: mov ax, word ptr [bx + 0x1100]         ; tbl DS:1100
b564: mov word ptr [0x934a], ax
b567: mov bx, word ptr [bp - 0x10]
b56a: shl bx
b56c: mov ax, word ptr [bx + 0x1100]         ; tbl DS:1100
b570: mov word ptr [0x934e], ax
b573: mov bx, word ptr [bp - 0x12]
b576: mov bl, byte ptr [bx + 0x1114]         ; tbl DS:1114
b57a: sub bh, bh
b57c: shl bx
b57e: mov ax, word ptr [bx + 0x5222]         ; tbl DS:5222
b582: mov word ptr [0x9352], ax
b585: push word ptr [bp - 0xa]
b588: push word ptr [bp - 0x8]
b58b: push si
b58c: call 0xb59f
b58f: call 0xc34f
b592: call 0x1d8a                            ; EXE mouse cursor pop
b595: mov ax, si
b597: pop si
b598: pop di
b599: mov sp, bp
b59b: pop bp
b59c: ret 0x4

; ---- func B59F  size 0x101  bp frame, locals 4, ret 0x6  callers: B12A
b59f: push bp
b5a0: mov bp, sp
b5a2: sub sp, 0x4
b5a5: push di
b5a6: push si
b5a7: mov si, word ptr [bp + 0x8]
b5aa: call 0x4ed                             ; EXE key_flush
b5ad: mov ax, si
b5af: or ax, ax
b5b1: je 0xb5be
b5b3: dec ax
b5b4: je 0xb5c6
b5b6: dec ax
b5b7: je 0xb5ce
b5b9: dec ax
b5ba: je 0xb5d6
b5bc: jmp 0xb5dc
b5be: mov word ptr [0x9356], 0x3e02
b5c4: jmp 0xb5dc
b5c6: mov word ptr [0x9356], 0x8f52          ; "The Sea and War Gods frown, and your\nflagship sinks beneath you.\n\n"
b5cc: jmp 0xb5dc
b5ce: mov word ptr [0x9356], 0x8eee          ; "Neptune and Mars smile, and you destroy the enemy\nflagship, sending your foes to a watery death.\n\n"
b5d4: jmp 0xb5dc
b5d6: mov word ptr [0x9356], 0x8f96          ; "Such commanders lead these fleets, that Neptune calls\nthem both to his underwater realm.\n\n"
b5dc: cmp word ptr [bp + 0x6], 0x8
b5e0: jne 0xb5e8
b5e2: mov word ptr [0x9356], 0x8ff2          ; "The enemy admiral, recognizing your superiority,\nflees for the safety of his home port.\n\n"
b5e8: mov ax, word ptr [bp + 0x4]
b5eb: dec ax
b5ec: je 0xb5f9
b5ee: dec ax
b5ef: je 0xb618
b5f1: dec ax
b5f2: je 0xb63b
b5f4: dec ax
b5f5: je 0xb643
b5f7: jmp 0xb649
b5f9: test si, 0x1
b5fd: je 0xb604
b5ff: mov ax, 0x913c                         ; "With the sinking of the flagship, your sailors lose\nheart and the Roman fleet is defeated."
b602: jmp 0xb607
b604: mov ax, 0x9220                         ; "Alas, your ship duel was with the witless commander\nof the enemy fleet, and his more competent mates\nhave swept the seas clean of Roman vessels."
b607: mov word ptr [0x9358], ax
b60a: cmp word ptr [bp + 0x6], 0x8
b60e: jne 0xb649
b610: mov word ptr [0x9358], 0x90b0          ; "In spite of their admiral's cowardly flight,\nthe enemy fleet overwhelms your ships."
b616: jmp 0xb649
b618: inc word ptr [0xc0fe]
b61c: test si, 0x1
b620: je 0xb627
b622: mov ax, 0x9104                         ; "In spite of their admiral's bath,\nyour fleet prevails."
b625: jmp 0xb62a
b627: mov ax, 0x9198                         ; "The enemy force is disheartened when their leader\nis sent to Neptune's locker without a sword, and\nyour ships make quick work of them."
b62a: mov word ptr [0x9358], ax
b62d: cmp word ptr [bp + 0x6], 0x8
b631: jne 0xb649
b633: mov word ptr [0x9358], 0x904c          ; "Their leader's flight from battle disheartens\nthe enemy, and your brave crews destroy them easily."
b639: jmp 0xb649
b63b: mov word ptr [0x9358], 0x92b2          ; "The fleets fight until sunset, when the last two\nvessels sink, together forever in a deadly embrace."
b641: jmp 0xb649
b643: mov word ptr [0x9358], 0x9318          ; "You retreat to safety with your remaining ships."
b649: mov ax, 0xc7
b64c: mov word ptr [0xc0e0], ax
b64f: mov word ptr [0xc1ea], ax
b652: mov word ptr [0x9ac6], ax
b655: mov ax, 0x9b56
b658: mov di, ax
b65a: mov si, 0xa05c
b65d: push ds
b65e: pop es
b65f: mov cx, 0x5
b662: rep movsw word ptr es:[di], word ptr [si]
b664: push word ptr [0x934a]
b668: mov ax, 0xac60
b66b: push ax
b66c: call 0x9e32                            ; EXE strcpy
b66f: pop bx
b670: pop bx
b671: mov word ptr [bp - 0x2], 0x934c
b676: jmp 0xb68a
b678: mov bx, word ptr [bp - 0x2]
b67b: push word ptr [bx]
b67d: mov ax, 0xac60
b680: push ax
b681: call 0x9df2                            ; EXE strcat
b684: pop bx
b685: pop bx
b686: add word ptr [bp - 0x2], 0x2
b68a: cmp word ptr [bp - 0x2], 0x935a
b68f: jb 0xb678
b691: mov ax, 0xac60
b694: push ax
b695: call 0x59d4                            ; EXE message_box(str)
b698: pop si
b699: pop di
b69a: mov sp, bp
b69c: pop bp
b69d: ret 0x6

; ---- func B6A0  size 0x159  bp frame, locals 8, ret  callers: B12A
b6a0: push bp
b6a1: mov bp, sp
b6a3: sub sp, 0x8
b6a6: push di
b6a7: push si
b6a8: sub ax, ax
b6aa: mov word ptr [bp - 0x4], ax
b6ad: mov word ptr [bp - 0x6], ax
b6b0: mov bl, byte ptr [0xc37a]
b6b4: sub bh, bh
b6b6: mov al, byte ptr [bx - 0x663e]         ; tbl DS:99C2
b6ba: shl al
b6bc: shl al
b6be: cmp al, byte ptr [0xc38f]
b6c2: jl 0xb6c9
b6c4: mov word ptr [bp - 0x4], 0xffff
b6c9: mov al, byte ptr [0xc36e]
b6cc: cwde
b6cd: imul word ptr [0xc37e]
b6d1: mov cx, ax
b6d3: mov al, byte ptr [0xc36f]
b6d6: cwde
b6d7: imul word ptr [0xc380]
b6db: cmp ax, cx
b6dd: jle 0xb6e4
b6df: mov word ptr [bp - 0x6], 0xffff
b6e4: mov al, byte ptr [0xc388]
b6e7: mov byte ptr [bp - 0x1], al
b6ea: cmp byte ptr [0xc396], 0x0
b6ef: je 0xb6f4
b6f1: jmp 0xb7bb
b6f4: sub al, byte ptr [0xc370]
b6f8: and al, 0xf
b6fa: mov byte ptr [bp - 0x2], al
b6fd: cmp al, 0x8
b6ff: jne 0xb70b
b701: mov ax, 0x10
b704: push ax
b705: call 0x76b                             ; EXE rng_roll(n)
b708: mov byte ptr [bp - 0x2], al
b70b: mov al, byte ptr [bp - 0x2]
b70e: cwde
b70f: mov bx, ax
b711: mov al, byte ptr [bx - 0x6c5a]         ; tbl DS:93A6
b715: mov byte ptr [bp - 0x7], al
b718: cmp byte ptr [bp - 0x2], 0x0
b71c: jne 0xb724
b71e: sub ax, ax
b720: push ax
b721: call 0xc846
b724: cmp byte ptr [bp - 0x7], 0x0
b728: jne 0xb78f
b72a: mov al, byte ptr [bp - 0x1]
b72d: cwde
b72e: mov bx, ax
b730: mov al, byte ptr [bx - 0x6c3a]         ; tbl DS:93C6
b734: cwde
b735: mov cx, word ptr [0xc374]
b739: sub cx, word ptr [0xc378]
b73d: imul cx
b73f: mov si, ax
b741: mov al, byte ptr [bx - 0x6c4a]         ; tbl DS:93B6
b745: cwde
b746: mov cx, word ptr [0xc372]
b74a: sub cx, word ptr [0xc376]
b74e: imul cx
b750: mov di, ax
b752: or di, di
b754: je 0xb75f
b756: mov ax, si
b758: cdq
b759: xor ax, dx
b75b: sub ax, dx
b75d: mov si, ax
b75f: or si, si
b761: je 0xb76c
b763: mov ax, di
b765: cdq
b766: xor ax, dx
b768: sub ax, dx
b76a: mov di, ax
b76c: cmp si, di
b76e: jge 0xb77c
b770: mov al, byte ptr [bp - 0x1]
b773: cwde
b774: mov bx, ax
b776: mov al, byte ptr [bx - 0x6c2a]         ; tbl DS:93D6
b77a: jmp 0xb78c
b77c: cmp si, di
b77e: jle 0xb78f
b780: mov al, byte ptr [bp - 0x1]
b783: cwde
b784: mov bx, ax
b786: mov al, byte ptr [bx - 0x6c2a]         ; tbl DS:93D6
b78a: neg al
b78c: mov byte ptr [bp - 0x7], al
b78f: cmp word ptr [bp - 0x4], 0x0
b793: jne 0xb7a2
b795: cmp word ptr [bp - 0x6], 0x0
b799: jne 0xb7a2
b79b: cmp word ptr [0xc382], 0x5f
b7a0: jbe 0xb7a8
b7a2: mov al, byte ptr [bp - 0x7]
b7a5: mov byte ptr [0xc396], al
b7a8: cmp byte ptr [0xc393], 0x0
b7ad: je 0xb7bb
b7af: mov ax, 0x3
b7b2: push ax
b7b3: call 0x76b                             ; EXE rng_roll(n)
b7b6: dec al
b7b8: mov byte ptr [0xc396], al
b7bb: cmp word ptr [bp - 0x4], 0x1
b7bf: sbb al, al
b7c1: and al, -0x1
b7c3: add al, 0x3
b7c5: cmp al, byte ptr [0xc38c]
b7c9: jge 0xb7d6
b7cb: mov ax, 0x3
b7ce: push ax
b7cf: call 0x76b                             ; EXE rng_roll(n)
b7d2: dec al
b7d4: jmp 0xb7dd
b7d6: mov ax, 0x2
b7d9: push ax
b7da: call 0x76b                             ; EXE rng_roll(n)
b7dd: mov byte ptr [0xc394], al
b7e0: cmp word ptr [bp - 0x6], 0x0
b7e4: je 0xb7f3
b7e6: cmp word ptr [0xc382], 0x55
b7eb: jae 0xb7f3
b7ed: sub ax, ax
b7ef: push ax
b7f0: call 0xba7a
b7f3: pop si
b7f4: pop di
b7f5: mov sp, bp
b7f7: pop bp
b7f8: ret
b7f9: db 0x00                                ; word-alignment pad

; ---- func B7FA  size 0xe9  bp frame, locals 6, ret 0x2  callers: B12A
b7fa: push bp
b7fb: mov bp, sp
b7fd: sub sp, 0x6
b800: push di
b801: push si
b802: mov si, word ptr [bp + 0x4]
b805: mov cl, 0x3
b807: mov ax, si
b809: shl ax, cl
b80b: inc ax
b80c: inc ax
b80d: mov word ptr [bp - 0x2], ax
b810: push ax
b811: call 0x183                             ; EXE ticks_since(n)
b814: mov cx, ax
b816: mov al, byte ptr [si - 0x3c76]         ; tbl DS:C38A
b81a: cwde
b81b: mov bx, ax
b81d: mov al, byte ptr [bx - 0x6c02]         ; tbl DS:93FE
b821: sub ah, ah
b823: cmp ax, cx
b825: jb 0xb82a
b827: jmp 0xb8db
b82a: cmp byte ptr [si - 0x3c76], ah         ; tbl DS:C38A
b82e: je 0xb83f
b830: inc byte ptr [si - 0x3c70]             ; tbl DS:C390
b834: cmp byte ptr [si - 0x3c70], 0x6        ; tbl DS:C390
b839: jle 0xb83f
b83b: mov byte ptr [si - 0x3c70], ah         ; tbl DS:C390
b83f: mov al, byte ptr [si - 0x3c76]         ; tbl DS:C38A
b843: cwde
b844: mov bx, ax
b846: mov al, byte ptr [bx - 0x6c0a]         ; tbl DS:93F6
b84a: cwde
b84b: mov word ptr [bp - 0x4], ax
b84e: mov cx, ax
b850: mov al, byte ptr [si - 0x3c78]         ; tbl DS:C388
b854: cwde
b855: push ax
b856: mov di, cx
b858: call 0xc4d8
b85b: imul di
b85d: mov di, ax
b85f: mov bx, si
b861: shl bx
b863: shl bx
b865: mov word ptr [bp - 0x6], bx
b868: add di, word ptr [bx - 0x3c68]         ; tbl DS:C398
b86c: mov ax, di
b86e: sub ah, ah
b870: mov word ptr [bx - 0x3c68], ax         ; tbl DS:C398
b874: or di, di
b876: jge 0xb87f
b878: mov bx, word ptr [bp - 0x6]
b87b: neg word ptr [bx - 0x3c68]             ; tbl DS:C398
b87f: mov ax, di
b881: cdq
b882: xor ax, dx
b884: sub ax, dx
b886: mov cx, 0x8
b889: sar ax, cl
b88b: xor ax, dx
b88d: sub ax, dx
b88f: mov bx, word ptr [bp - 0x6]
b892: add word ptr [bx - 0x3c8e], ax         ; tbl DS:C372
b896: mov al, byte ptr [si - 0x3c78]         ; tbl DS:C388
b89a: cwde
b89b: push ax
b89c: call 0xc4e8
b89f: imul word ptr [bp - 0x4]
b8a2: mov di, ax
b8a4: mov bx, word ptr [bp - 0x6]
b8a7: add di, word ptr [bx - 0x3c66]         ; tbl DS:C39A
b8ab: mov ax, di
b8ad: sub ah, ah
b8af: mov word ptr [bx - 0x3c66], ax         ; tbl DS:C39A
b8b3: or di, di
b8b5: jge 0xb8be
b8b7: mov bx, word ptr [bp - 0x6]
b8ba: neg word ptr [bx - 0x3c66]             ; tbl DS:C39A
b8be: mov ax, di
b8c0: cdq
b8c1: xor ax, dx
b8c3: sub ax, dx
b8c5: mov cx, 0x8
b8c8: sar ax, cl
b8ca: xor ax, dx
b8cc: sub ax, dx
b8ce: mov bx, word ptr [bp - 0x6]
b8d1: add word ptr [bx - 0x3c8c], ax         ; tbl DS:C374
b8d5: push word ptr [bp - 0x2]
b8d8: call 0xd7                              ; EXE timer_start(n) (copy tick DS:42/44 to slot 46+4n)
b8db: pop si
b8dc: pop di
b8dd: mov sp, bp
b8df: pop bp
b8e0: ret 0x2

; ---- func B8E3  size 0x87  bp frame, locals 8, ret 0x2  callers: BB7E BEAE D3F2
b8e3: push bp
b8e4: mov bp, sp
b8e6: sub sp, 0x8
b8e9: push di
b8ea: mov bx, word ptr [bp + 0x4]
b8ed: shl bx
b8ef: shl bx
b8f1: mov di, word ptr [bx - 0x3c8e]         ; tbl DS:C372
b8f5: mov ax, word ptr [bx - 0x3c8c]         ; tbl DS:C374
b8f9: mov word ptr [bp - 0x2], ax
b8fc: mov bx, word ptr [bp + 0x4]
b8ff: mov al, byte ptr [bx - 0x3c78]         ; tbl DS:C388
b903: cwde
b904: mov word ptr [bp - 0x4], ax
b907: mov al, byte ptr [bx - 0x3c70]         ; tbl DS:C390
b90b: cwde
b90c: mov bx, word ptr [bp - 0x4]
b90f: mov cx, ax
b911: mov al, byte ptr [bx - 0x6c1a]         ; tbl DS:93E6
b915: cwde
b916: add cx, ax
b918: cmp bx, 0x8
b91b: jge 0xb928
b91d: mov ax, 0xe
b920: imul cx
b922: add ax, word ptr [0xc3a6]
b926: jmp 0xb934
b928: mov ax, 0xe
b92b: imul cx
b92d: add ax, word ptr [0xc3a8]
b931: sub ax, 0x310
b934: push ax
b935: push di
b936: push word ptr [bp - 0x2]
b939: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
b93c: mov ax, 0xe
b93f: imul word ptr [bp - 0x4]
b942: mov bx, word ptr [bp + 0x4]
b945: shl bx
b947: mov cx, word ptr [bx - 0x3c5e]         ; tbl DS:C3A2
b94b: add cx, ax
b94d: push cx
b94e: push di
b94f: push word ptr [bp - 0x2]
b952: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
b955: mov bx, word ptr [bp + 0x4]
b958: cmp byte ptr [bx - 0x3c60], 0x0        ; tbl DS:C3A0
b95d: je 0xb963
b95f: push bx
b960: call 0xcd4b
b963: pop di
b964: mov sp, bp
b966: pop bp
b967: ret 0x2

; ---- func B96A  size 0x110  bp frame, locals 14, ret  callers: B12A
b96a: push bp
b96b: mov bp, sp
b96d: sub sp, 0xe
b970: push di
b971: push si
b972: mov si, word ptr [0xc384]
b976: xor si, 0x1
b979: mov bx, si
b97b: shl bx
b97d: shl bx
b97f: mov ax, word ptr [bx - 0x3c8e]         ; tbl DS:C372
b983: mov word ptr [bp - 0x4], ax
b986: mov ax, word ptr [bx - 0x3c8c]         ; tbl DS:C374
b98a: mov word ptr [bp - 0x6], ax
b98d: mov al, byte ptr [si - 0x3c78]         ; tbl DS:C388
b991: cwde
b992: mov word ptr [bp - 0xa], ax
b995: mov al, byte ptr [si - 0x3c70]         ; tbl DS:C390
b999: mov bx, word ptr [bp - 0xa]
b99c: mov cx, ax
b99e: mov al, byte ptr [bx - 0x6c1a]         ; tbl DS:93E6
b9a2: cwde
b9a3: mov di, ax
b9a5: mov al, cl
b9a7: cwde
b9a8: add di, ax
b9aa: cmp bx, 0x8
b9ad: jge 0xb9bc
b9af: mov ax, di
b9b1: mov cx, 0xe
b9b4: imul cx
b9b6: add ax, word ptr [0xc3a6]
b9ba: jmp 0xb9ca
b9bc: mov ax, di
b9be: mov cx, 0xe
b9c1: imul cx
b9c3: add ax, word ptr [0xc3a8]
b9c7: sub ax, 0x310
b9ca: push ax
b9cb: push word ptr [bp - 0x4]
b9ce: push word ptr [bp - 0x6]
b9d1: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
b9d4: mov bx, word ptr [0xc384]
b9d8: shl bx
b9da: shl bx
b9dc: mov di, word ptr [bx - 0x3c8e]         ; tbl DS:C372
b9e0: mov ax, word ptr [bx - 0x3c8c]         ; tbl DS:C374
b9e4: mov word ptr [bp - 0x2], ax
b9e7: mov bx, word ptr [0xc384]
b9eb: mov al, byte ptr [bx - 0x3c78]         ; tbl DS:C388
b9ef: cwde
b9f0: mov word ptr [bp - 0x8], ax
b9f3: mov al, byte ptr [bx - 0x3c70]         ; tbl DS:C390
b9f7: cwde
b9f8: mov bx, word ptr [bp - 0x8]
b9fb: mov cx, ax
b9fd: mov al, byte ptr [bx - 0x6c1a]         ; tbl DS:93E6
ba01: cwde
ba02: add cx, ax
ba04: cmp bx, 0x8
ba07: jge 0xba14
ba09: mov ax, 0xe
ba0c: imul cx
ba0e: add ax, word ptr [0xc3a6]
ba12: jmp 0xba20
ba14: mov ax, 0xe
ba17: imul cx
ba19: add ax, word ptr [0xc3a8]
ba1d: sub ax, 0x310
ba20: push ax
ba21: push di
ba22: push word ptr [bp - 0x2]
ba25: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
ba28: mov ax, 0xe
ba2b: imul word ptr [bp - 0xa]
ba2e: mov bx, si
ba30: mov cx, word ptr [bx + si - 0x3c5e]    ; tbl DS:C3A2
ba34: add cx, ax
ba36: push cx
ba37: push word ptr [bp - 0x4]
ba3a: push word ptr [bp - 0x6]
ba3d: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
ba40: cmp byte ptr [si - 0x3c60], 0x0        ; tbl DS:C3A0
ba45: je 0xba4b
ba47: push si
ba48: call 0xcd4b
ba4b: mov ax, 0xe
ba4e: imul word ptr [bp - 0x8]
ba51: mov bx, word ptr [0xc384]
ba55: shl bx
ba57: mov cx, word ptr [bx - 0x3c5e]         ; tbl DS:C3A2
ba5b: add cx, ax
ba5d: push cx
ba5e: push di
ba5f: push word ptr [bp - 0x2]
ba62: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
ba65: mov bx, word ptr [0xc384]
ba69: cmp byte ptr [bx - 0x3c60], 0x0        ; tbl DS:C3A0
ba6e: je 0xba74
ba70: push bx
ba71: call 0xcd4b
ba74: pop si
ba75: pop di
ba76: mov sp, bp
ba78: pop bp
ba79: ret

; ---- func BA7A  size 0x104  bp frame, locals 12, ret 0x2  callers: B12A B6A0
ba7a: push bp
ba7b: mov bp, sp
ba7d: sub sp, 0xc
ba80: push di
ba81: mov di, word ptr [bp + 0x4]
ba84: mov byte ptr [bp - 0xb], -0x1
ba88: mov word ptr [bp - 0x2], 0x0
ba8d: cmp byte ptr [di - 0x3c56], 0x0        ; tbl DS:C3AA
ba92: jne 0xba97
ba94: jmp 0xbb67
ba97: dec byte ptr [di - 0x3c56]             ; tbl DS:C3AA
ba9b: mov al, byte ptr [di - 0x3c78]         ; tbl DS:C388
ba9f: mov byte ptr [bp - 0x5], al
baa2: cwde
baa3: mov cl, 0x4
baa5: mov dl, byte ptr [di - 0x3c86]         ; tbl DS:C37A
baa9: sub dh, dh
baab: shl dx, cl
baad: mov cx, ax
baaf: add ax, dx
bab1: mov word ptr [bp - 0xa], ax
bab4: mov dx, cx
bab6: sub dl, byte ptr [di - 0x3c90]         ; tbl DS:C370
baba: and dx, 0xf
babd: jle 0xbae6
babf: cmp dx, 0x8
bac2: jge 0xbae6
bac4: mov al, byte ptr [bp - 0x5]
bac7: cwde
bac8: mov bx, ax
baca: mov al, byte ptr [bx - 0x6a2c]         ; tbl DS:95D4
bace: add al, 0x3a
bad0: mov byte ptr [bp - 0xb], al
bad3: mov bx, word ptr [bp - 0xa]
bad6: mov al, byte ptr [bx - 0x6a0c]         ; tbl DS:95F4
bada: cwde
badb: mov word ptr [bp - 0x8], ax
bade: mov al, byte ptr [bx - 0x69dc]         ; tbl DS:9624
bae2: cwde
bae3: mov word ptr [bp - 0x4], ax
bae6: cmp dx, 0x8
bae9: jle 0xbb0d
baeb: mov al, byte ptr [bp - 0x5]
baee: cwde
baef: mov bx, ax
baf1: mov al, byte ptr [bx - 0x6a1c]         ; tbl DS:95E4
baf5: add al, 0x3a
baf7: mov byte ptr [bp - 0xb], al
bafa: mov bx, word ptr [bp - 0xa]
bafd: mov al, byte ptr [bx - 0x69ac]         ; tbl DS:9654
bb01: cwde
bb02: mov word ptr [bp - 0x8], ax
bb05: mov al, byte ptr [bx - 0x697c]         ; tbl DS:9684
bb09: cwde
bb0a: mov word ptr [bp - 0x4], ax
bb0d: cmp byte ptr [bp - 0xb], -0x1
bb11: je 0xbb62
bb13: mov bx, di
bb15: shl bx
bb17: shl bx
bb19: mov ax, word ptr [bx - 0x3c8e]         ; tbl DS:C372
bb1d: add word ptr [bp - 0x8], ax
bb20: mov al, 0xe
bb22: mul byte ptr [bp - 0xb]
bb25: add ax, word ptr [0xc3ae]
bb29: push ax
bb2a: push word ptr [bp - 0x8]
bb2d: mov ax, word ptr [bx - 0x3c8c]         ; tbl DS:C374
bb31: add word ptr [bp - 0x4], ax
bb34: mov ax, word ptr [bp - 0x4]
bb37: push ax
bb38: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
bb3b: mov bl, byte ptr [bp - 0xb]
bb3e: sub bl, 0x3a
bb41: sub bh, bh
bb43: mov al, byte ptr [bx - 0x694c]         ; tbl DS:96B4
bb47: cwde
bb48: add ax, word ptr [bp - 0x8]
bb4b: push ax
bb4c: mov al, byte ptr [bx - 0x6944]         ; tbl DS:96BC
bb50: cwde
bb51: add ax, word ptr [bp - 0x4]
bb54: push ax
bb55: mov ax, di
bb57: xor al, 0x1
bb59: push ax
bb5a: call 0xd070
bb5d: mov word ptr [bp - 0x2], ax
bb60: jmp 0xbb67
bb62: mov byte ptr [di - 0x3c56], 0x0        ; tbl DS:C3AA
bb67: cmp word ptr [bp - 0x2], 0x0
bb6b: je 0xbb75
bb6d: mov al, byte ptr [di - 0x6a2e]         ; tbl DS:95D2
bb71: sub ah, ah
bb73: jmp 0xbb77
bb75: sub ax, ax
bb77: pop di
bb78: mov sp, bp
bb7a: pop bp
bb7b: ret 0x2

; ---- func BB7E  size 0x1c3  bp frame, locals 6, ret 0x6  callers: B12A
bb7e: push bp
bb7f: mov bp, sp
bb81: sub sp, 0x6
bb84: push di
bb85: push si
bb86: mov byte ptr [bp - 0x1], 0x14
bb8a: call 0x4ed                             ; EXE key_flush
bb8d: mov byte ptr [0xc37c], 0x0
bb92: mov ax, 0x24
bb95: push ax
bb96: call 0x11f                             ; EXE frame_wait(n): busy-wait until timer 0 + n ticks
bb99: mov al, byte ptr [bp + 0x4]
bb9c: and al, 0x2
bb9e: je 0xbba5
bba0: add byte ptr [0xc36e], 0xa
bba5: test byte ptr [bp + 0x4], 0x1
bba9: je 0xbbb0
bbab: add byte ptr [0xc36f], 0xa
bbb0: mov al, 0x6
bbb2: mov byte ptr [0xc391], al
bbb5: mov byte ptr [0xc390], al
bbb8: mov word ptr [0xc386], 0x48
bbbe: mov ax, word ptr [0xc376]
bbc1: add ax, word ptr [0xc372]
bbc5: cdq
bbc6: sub ax, dx
bbc8: sar ax
bbca: mov word ptr [bp - 0x4], ax
bbcd: mov ax, word ptr [0xc378]
bbd0: add ax, word ptr [0xc374]
bbd4: cdq
bbd5: sub ax, dx
bbd7: sar ax
bbd9: mov word ptr [bp - 0x6], ax
bbdc: cmp word ptr [0xc37e], 0x0
bbe1: je 0xbbea
bbe3: cmp word ptr [0xc380], 0x0
bbe8: jne 0xbbf3
bbea: cmp byte ptr [bp - 0x1], 0x0
bbee: jne 0xbbf3
bbf0: jmp 0xbca9
bbf3: mov ax, 0x2
bbf6: push ax
bbf7: call 0x183                             ; EXE ticks_since(n)
bbfa: cmp ax, 0x8
bbfd: jbe 0xbbdc
bbff: mov ax, 0x2
bc02: push ax
bc03: call 0xd7                              ; EXE timer_start(n) (copy tick DS:42/44 to slot 46+4n)
bc06: cmp byte ptr [bp - 0x1], 0x0
bc0a: je 0xbc0f
bc0c: dec byte ptr [bp - 0x1]
bc0f: mov ax, 0x8
bc12: push ax
bc13: call 0x183                             ; EXE ticks_since(n)
bc16: cmp ax, word ptr [0xc386]
bc1a: jbe 0xbc62
bc1c: mov ax, 0x8
bc1f: push ax
bc20: call 0xd7                              ; EXE timer_start(n) (copy tick DS:42/44 to slot 46+4n)
bc23: mov ax, 0x6c
bc26: push ax
bc27: call 0x76b                             ; EXE rng_roll(n)
bc2a: mov word ptr [0xc386], ax
bc2d: sub ax, ax
bc2f: push ax
bc30: mov al, byte ptr [0xc36e]
bc33: push ax
bc34: call 0xbd41
bc37: mov si, ax
bc39: mov ax, 0x1
bc3c: push ax
bc3d: mov al, byte ptr [0xc36f]
bc40: push ax
bc41: call 0xbd41
bc44: mov di, ax
bc46: push word ptr [0xc380]
bc4a: push si
bc4b: sub ax, ax
bc4d: push ax
bc4e: call 0x50f                             ; EXE step_toward_min: max(v - step, min), signed (050F family)
bc51: mov word ptr [0xc380], ax
bc54: push word ptr [0xc37e]
bc58: push di
bc59: sub ax, ax
bc5b: push ax
bc5c: call 0x50f                             ; EXE step_toward_min: max(v - step, min), signed (050F family)
bc5f: mov word ptr [0xc37e], ax
bc62: mov word ptr [0x9afa], 0x9
bc68: sub ax, ax
bc6a: push ax
bc6b: push ax
bc6c: mov ax, 0x13f
bc6f: push ax
bc70: mov ax, 0xb9
bc73: push ax
bc74: call 0x7e74                            ; EXE fill rect (colour, ...)
bc77: mov ax, 0x1
bc7a: push ax
bc7b: call 0xb8e3
bc7e: sub ax, ax
bc80: push ax
bc81: call 0xb8e3
bc84: mov ax, 0x3
bc87: push ax
bc88: call 0x76b                             ; EXE rng_roll(n)
bc8b: mov si, ax
bc8d: mov cx, 0xe
bc90: mul cx
bc92: add ax, word ptr [0xc3ac]
bc96: push ax
bc97: push word ptr [bp - 0x4]
bc9a: push word ptr [bp - 0x6]
bc9d: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
bca0: call 0xc71b
bca3: call 0x6887                            ; EXE flip: work buffer -> screen
bca6: jmp 0xbbdc
bca9: cmp word ptr [0xc37e], 0x0
bcae: jne 0xbcbb
bcb0: mov byte ptr [0xc38e], 0x0
bcb5: mov ax, word ptr [0x96ca]
bcb8: mov word ptr [0x96d2], ax
bcbb: cmp word ptr [0xc380], 0x0
bcc0: jne 0xbcd9
bcc2: mov byte ptr [0xc38f], 0x0
bcc7: cmp byte ptr [0xc38e], 0x0
bccc: jne 0xbcd3
bcce: mov ax, word ptr [0x96cc]
bcd1: jmp 0xbcd6
bcd3: mov ax, word ptr [0x96c8]
bcd6: mov word ptr [0x96d2], ax
bcd9: call 0x67b6                            ; EXE screen restore 6973(0,0,319,199) unless [9B26]==2 path (G)
bcdc: call 0x6837                            ; EXE draw on the screen (6887 first if needed)
bcdf: mov ax, 0x9b56
bce2: mov di, ax
bce4: mov si, 0xa05c
bce7: push ds
bce8: pop es
bce9: mov cx, 0x5
bcec: rep movsw word ptr es:[di], word ptr [si]
bcee: mov cl, byte ptr [bp + 0x4]
bcf1: and cl, 0x2
bcf4: je 0xbcfb
bcf6: mov ax, word ptr [0x96c4]
bcf9: jmp 0xbcfe
bcfb: mov ax, word ptr [0x96c6]
bcfe: mov word ptr [0x96ce], ax
bd01: push ax
bd02: mov ax, 0xac60
bd05: push ax
bd06: call 0x9e32                            ; EXE strcpy
bd09: pop bx
bd0a: pop bx
bd0b: push word ptr [0x96d0]
bd0f: mov ax, 0xac60
bd12: push ax
bd13: call 0x9df2                            ; EXE strcat
bd16: pop bx
bd17: pop bx
bd18: push word ptr [0x96d2]
bd1c: mov ax, 0xac60
bd1f: push ax
bd20: call 0x9df2                            ; EXE strcat
bd23: pop bx
bd24: pop bx
bd25: mov ax, 0xac60
bd28: push ax
bd29: call 0x59d4                            ; EXE message_box(str)
bd2c: mov di, 0x9b56
bd2f: mov si, 0xa3b4
bd32: push ds
bd33: pop es
bd34: mov cx, 0x5
bd37: rep movsw word ptr es:[di], word ptr [si]
bd39: pop si
bd3a: pop di
bd3b: mov sp, bp
bd3d: pop bp
bd3e: ret 0x6

; ---- func BD41  size 0x48  bp frame, locals 2, ret 0x4  callers: BB7E
bd41: push bp
bd42: mov bp, sp
bd44: sub sp, 0x2
bd47: push di
bd48: push si
bd49: sub si, si
bd4b: mov di, si
bd4d: mov ax, word ptr [bp + 0x6]
bd50: shl ax
bd52: add ax, 0xc37e
bd55: mov word ptr [bp - 0x2], ax
bd58: jmp 0xbd78
bd5a: mov ax, 0x64
bd5d: push ax
bd5e: call 0x76b                             ; EXE rng_roll(n)
bd61: mov cl, byte ptr [bp + 0x4]
bd64: sub ch, ch
bd66: cmp ax, cx
bd68: jae 0xbd77
bd6a: mov ax, 0x64
bd6d: push ax
bd6e: call 0x76b                             ; EXE rng_roll(n)
bd71: cmp ax, 0x19
bd74: jae 0xbd77
bd76: inc di
bd77: inc si
bd78: mov bx, word ptr [bp - 0x2]
bd7b: cmp word ptr [bx], si
bd7d: ja 0xbd5a
bd7f: mov ax, di
bd81: pop si
bd82: pop di
bd83: mov sp, bp
bd85: pop bp
bd86: ret 0x4
bd89: db 0x00                                ; word-alignment pad

; ---- func BD8A  size 0x85  bp frame, locals 2, ret 0x4  callers: B12A
bd8a: push bp
bd8b: mov bp, sp
bd8d: sub sp, 0x2
bd90: call 0x5d20                            ; EXE music_stop
bd93: mov al, 0x1
bd95: push ax
bd96: call 0x5e1a                            ; EXE music_unload(n)
bd99: mov al, byte ptr [bp + 0x6]
bd9c: push ax
bd9d: call 0xc274
bda0: mov byte ptr [0xc37b], al
bda3: cmp al, -0x1
bda5: jne 0xbdab
bda7: sub ax, ax
bda9: jmp 0xbe09
bdab: mov byte ptr [bp - 0x1], 0x0
bdaf: mov bl, byte ptr [bp - 0x1]
bdb2: sub bh, bh
bdb4: mov ax, bx
bdb6: shl bx
bdb8: mov word ptr [bx - 0x3c42], 0x0        ; tbl DS:C3BE
bdbe: mov cx, ax
bdc0: mov al, 0x6
bdc2: mul byte ptr [bp + 0x4]
bdc5: mov bx, ax
bdc7: add bx, cx
bdc9: shl bx
bdcb: cmp word ptr [bx + 0x110e], 0x0        ; tbl DS:110E
bdd0: je 0xbdd8
bdd2: mov al, byte ptr [bp - 0x1]
bdd5: mov byte ptr [0xc37a], al
bdd8: inc byte ptr [bp - 0x1]
bddb: cmp byte ptr [bp - 0x1], 0x3
bddf: jb 0xbdaf
bde1: mov bl, byte ptr [0xc37b]
bde5: sub bh, bh
bde7: mov al, byte ptr [bx - 0x711e]         ; tbl DS:8EE2
bdeb: mov byte ptr [0xc38f], al
bdee: mov bl, byte ptr [0xc37a]
bdf2: mov al, byte ptr [bx - 0x711e]         ; tbl DS:8EE2
bdf6: mov byte ptr [0xc38e], al
bdf9: call 0xbf50
bdfc: or ax, ax
bdfe: je 0xbda7
be00: call 0xc105
be03: call 0xbe0f
be06: mov ax, 0xffff
be09: mov sp, bp
be0b: pop bp
be0c: ret 0x4

; ---- func BE0F  size 0x9f  no bp frame, ret  callers: B12A BD8A
be0f: mov byte ptr [0xc389], 0xc
be14: mov byte ptr [0xc388], 0x4
be19: mov al, 0x6
be1b: mov byte ptr [0xc390], al
be1e: mov byte ptr [0xc391], al
be21: mov ax, word ptr [0x971a]
be24: mov dx, word ptr [0x971c]
be28: mov word ptr [0xc376], ax
be2b: mov word ptr [0xc378], dx
be2f: mov ax, word ptr [0x971e]
be32: mov dx, word ptr [0x9720]
be36: mov word ptr [0xc372], ax
be39: mov word ptr [0xc374], dx
be3d: mov ax, word ptr [0x56fe]
be40: mov dx, word ptr [0x5700]
be44: mov word ptr [0xc398], ax
be47: mov word ptr [0xc39a], dx
be4b: mov word ptr [0xc39c], ax
be4e: mov word ptr [0xc39e], dx
be52: mov al, 0x1
be54: mov byte ptr [0xc38c], al
be57: mov byte ptr [0xc38d], al
be5a: mov al, 0x5
be5c: mul byte ptr [0xc37b]
be60: mov bx, ax
be62: mov al, byte ptr [bx - 0x67e7]         ; tbl DS:9819
be66: mov byte ptr [0xc38b], al
be69: mov al, 0x5
be6b: mul byte ptr [0xc37a]
be6f: mov bx, ax
be71: mov al, byte ptr [bx - 0x67e7]         ; tbl DS:9819
be75: mov byte ptr [0xc38a], al
be78: sub al, al
be7a: mov byte ptr [0xc3a0], al
be7d: mov byte ptr [0xc3a1], al
be80: mov byte ptr [0xc3aa], al
be83: mov byte ptr [0xc3ab], al
be86: mov byte ptr [0xc37c], al
be89: mov byte ptr [0xc394], al
be8c: mov byte ptr [0xc395], al
be8f: mov byte ptr [0xc396], al
be92: mov byte ptr [0xc397], al
be95: mov byte ptr [0xc392], al
be98: mov byte ptr [0xc393], al
be9b: mov byte ptr [0xc3ba], al
be9e: mov byte ptr [0xc3bb], al
bea1: mov word ptr [0xc384], 0x1
bea7: mov word ptr [0xc386], 0xd8
bead: ret

; ---- func BEAE  size 0xa2  bp frame, locals 0, ret 0x4  callers: B12A
beae: push bp
beaf: mov bp, sp
beb1: push di
beb2: push si
beb3: call 0xff                              ; EXE timer start helper (00D7/00FF pair)
beb6: mov word ptr [0x9afa], 0x9
bebc: sub ax, ax
bebe: push ax
bebf: push ax
bec0: mov cx, 0x13f
bec3: push cx
bec4: mov dx, 0xb9
bec7: push dx
bec8: call 0x7e74                            ; EXE fill rect (colour, ...)
becb: mov word ptr [0x9afa], 0x0
bed1: sub ax, ax
bed3: push ax
bed4: mov cx, 0xba
bed7: push cx
bed8: mov cx, 0x13f
bedb: push cx
bedc: call 0x7eca                            ; EXE hline(x1, y, x0) clipped
bedf: mov ax, word ptr [0x6346]
bee2: mov word ptr [0x9afa], ax
bee5: sub ax, ax
bee7: push ax
bee8: mov cx, 0xbb
beeb: push cx
beec: mov dx, 0x13f
beef: push dx
bef0: mov bx, 0xc7
bef3: push bx
bef4: call 0x7e74                            ; EXE fill rect (colour, ...)
bef7: mov word ptr [0x9b69], 0x0
befd: mov ax, 0x9b56
bf00: mov di, ax
bf02: mov si, 0xa3b4
bf05: push ds
bf06: pop es
bf07: mov cx, 0x5
bf0a: rep movsw word ptr es:[di], word ptr [si]
bf0c: mov ax, 0x97ba                         ; "SHIPS"
bf0f: push ax
bf10: mov ax, 0x9f
bf13: push ax
bf14: mov ax, 0xbb
bf17: push ax
bf18: call 0x8be4                            ; EXE centred_text(str, cx, y)
bf1b: call 0xc71b
bf1e: mov al, byte ptr [bp + 0x6]
bf21: push ax
bf22: mov al, byte ptr [bp + 0x4]
bf25: push ax
bf26: call 0xdc8e
bf29: mov ax, 0x1
bf2c: push ax
bf2d: call 0xb8e3
bf30: sub ax, ax
bf32: push ax
bf33: call 0xb8e3
bf36: sub ax, ax
bf38: push ax
bf39: push ax
bf3a: mov ax, 0x13f
bf3d: push ax
bf3e: mov ax, 0xc7
bf41: push ax
bf42: call 0x691b                            ; EXE save rect screen -> work buffer
bf45: call 0x6887                            ; EXE flip: work buffer -> screen
bf48: pop si
bf49: pop di
bf4a: mov sp, bp
bf4c: pop bp
bf4d: ret 0x4

; ---- func BF50  size 0x1b5  bp frame, locals 16, ret  callers: BD8A
bf50: push bp
bf51: mov bp, sp
bf53: sub sp, 0x10
bf56: push di
bf57: push si
bf58: mov ax, 0x9
bf5b: push ax
bf5c: lea ax, [bp - 0x8]
bf5f: push ax
bf60: call 0xf7b                             ; EXE archive open (0F7B(n, &arch))
bf63: mov di, ax
bf65: or di, di
bf67: je 0xbfac
bf69: cmp word ptr [0x9b26], 0x2
bf6e: jne 0xbf83
bf70: mov ax, 0x97cc                         ; "galleys.lzw"
bf73: push ax
bf74: lea ax, [bp - 0x8]
bf77: push ax
bf78: push word ptr [0xc1de]
bf7c: push word ptr [0xc1dc]
bf80: call 0xf0c                             ; EXE file helper (stat/open; 18C2, 1172) (G)
bf83: mov ax, 0x97d8                         ; "oars1"
bf86: push ax
bf87: lea ax, [bp - 0x8]
bf8a: push ax
bf8b: sub cx, cx
bf8d: push cx
bf8e: call 0x163b                            ; EXE sheet_163B: load xxxHDR/xxxDAT sprite sheet
bf91: mov word ptr [0xc3a6], ax
bf94: mov ax, 0x97de                         ; "oars2"
bf97: push ax
bf98: lea ax, [bp - 0x8]
bf9b: push ax
bf9c: sub cx, cx
bf9e: push cx
bf9f: call 0x163b                            ; EXE sheet_163B: load xxxHDR/xxxDAT sprite sheet
bfa2: mov word ptr [0xc3a8], ax
bfa5: lea ax, [bp - 0x8]
bfa8: push ax
bfa9: call 0x11a9                            ; EXE archive close
bfac: mov ax, 0x8
bfaf: push ax
bfb0: lea ax, [bp - 0x8]
bfb3: push ax
bfb4: call 0xf7b                             ; EXE archive open (0F7B(n, &arch))
bfb7: and di, ax
bfb9: jne 0xbfbe
bfbb: jmp 0xc0e1
bfbe: mov word ptr [bp - 0x2], di
bfc1: cmp word ptr [0x9b26], 0x2
bfc6: je 0xbfef
bfc8: mov ax, 0x97e4                         ; "galleys.cbm"
bfcb: push ax
bfcc: lea ax, [bp - 0x8]
bfcf: push ax
bfd0: mov ax, 0xc356
bfd3: push ax
bfd4: call 0x1273                            ; EXE load CBM member into a descriptor
bfd7: call 0x67e4                            ; EXE draw into the work buffer
bfda: mov ax, 0xc356
bfdd: push ax
bfde: sub cx, cx
bfe0: push cx
bfe1: push cx
bfe2: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
bfe5: call 0x680f                            ; EXE target = the screen descriptor
bfe8: mov ax, 0xc356
bfeb: push ax
bfec: call 0x86a0                            ; EXE blit (86A0)
bfef: sub ax, ax
bff1: push ax
bff2: push ax
bff3: mov ax, 0x13f
bff6: push ax
bff7: mov ax, 0xc7
bffa: push ax
bffb: call 0x6973                            ; EXE restore rect work buffer -> screen
bffe: mov ax, 0x97f0                         ; "bombs"
c001: push ax
c002: lea ax, [bp - 0x8]
c005: push ax
c006: sub cx, cx
c008: push cx
c009: call 0x163b                            ; EXE sheet_163B: load xxxHDR/xxxDAT sprite sheet
c00c: mov word ptr [0xc3ae], ax
c00f: mov ax, 0x97f6                         ; "smok"
c012: push ax
c013: lea ax, [bp - 0x8]
c016: push ax
c017: sub cx, cx
c019: push cx
c01a: call 0x163b                            ; EXE sheet_163B: load xxxHDR/xxxDAT sprite sheet
c01d: mov word ptr [0xc3ac], ax
c020: mov bl, byte ptr [0xc37a]
c024: sub bh, bh
c026: shl bx
c028: push word ptr [bx - 0x67f4]            ; tbl DS:980C
c02c: lea ax, [bp - 0x8]
c02f: push ax
c030: sub ax, ax
c032: push ax
c033: call 0x163b                            ; EXE sheet_163B: load xxxHDR/xxxDAT sprite sheet
c036: mov word ptr [0xc3a2], ax
c039: mov al, byte ptr [0xc37a]
c03c: cmp byte ptr [0xc37b], al
c040: je 0xc05a
c042: mov bl, byte ptr [0xc37b]
c046: sub bh, bh
c048: shl bx
c04a: push word ptr [bx - 0x67f4]            ; tbl DS:980C
c04e: lea ax, [bp - 0x8]
c051: push ax
c052: sub ax, ax
c054: push ax
c055: call 0x163b                            ; EXE sheet_163B: load xxxHDR/xxxDAT sprite sheet
c058: jmp 0xc05d
c05a: mov ax, word ptr [0xc3a2]
c05d: mov word ptr [0xc3a4], ax
c060: cmp word ptr [0xc3a2], 0x0
c065: je 0xc087
c067: or ax, ax
c069: je 0xc087
c06b: cmp word ptr [0xc3ae], 0x0
c070: je 0xc087
c072: cmp word ptr [0xc3a6], 0x0
c077: je 0xc087
c079: cmp word ptr [0xc3a8], 0x0
c07e: je 0xc087
c080: cmp word ptr [0xc3ac], 0x0
c085: jne 0xc08c
c087: mov word ptr [bp - 0x2], 0x0
c08c: mov ax, 0x97fb                         ; "collide.lzw"
c08f: push ax
c090: lea ax, [bp - 0x8]
c093: push ax
c094: lea ax, [bp - 0x10]
c097: push ax
c098: call 0x11c4                            ; EXE archive member (text) load
c09b: or ax, ax
c09d: je 0xc0c7
c09f: sub di, di
c0a1: mov dx, di
c0a3: mov bx, 0x970a
c0a6: jmp 0xc0bc
c0a8: mov ax, word ptr [bp - 0x10]
c0ab: add ax, dx
c0ad: mov si, word ptr [bx]
c0af: mov word ptr [si], ax
c0b1: mov al, byte ptr [di - 0x68ec]         ; tbl DS:9714
c0b5: sub ah, ah
c0b7: add dx, ax
c0b9: inc bx
c0ba: inc bx
c0bb: inc di
c0bc: cmp bx, 0x9714
c0c0: jb 0xc0a8
c0c2: mov di, word ptr [bp - 0x2]
c0c5: jmp 0xc0c9
c0c7: sub di, di
c0c9: mov ax, 0x9807                         ; "flag"
c0cc: push ax
c0cd: lea ax, [bp - 0x8]
c0d0: push ax
c0d1: sub cx, cx
c0d3: push cx
c0d4: call 0x163b                            ; EXE sheet_163B: load xxxHDR/xxxDAT sprite sheet
c0d7: mov word ptr [0xc3bc], ax
c0da: lea ax, [bp - 0x8]
c0dd: push ax
c0de: call 0x11a9                            ; EXE archive close
c0e1: mov si, 0x1
c0e4: mov word ptr [bp - 0x2], di
c0e7: jmp 0xc0f2
c0e9: mov al, byte ptr [si - 0x68fc]         ; tbl DS:9704
c0ed: push ax
c0ee: call 0x5d73                            ; EXE music_load(n)
c0f1: inc si
c0f2: cmp si, 0x5
c0f5: jb 0xc0e9
c0f7: mov al, 0xd
c0f9: push ax
c0fa: call 0x5d29                            ; EXE music_play(n)
c0fd: mov ax, di
c0ff: pop si
c100: pop di
c101: mov sp, bp
c103: pop bp
c104: ret

; ---- func C105  size 0x87  no bp frame, ret  callers: BD8A
c105: push di
c106: push si
c107: sub si, si
c109: cmp word ptr [0x9b26], 0x2
c10e: jne 0xc115
c110: mov di, 0x80
c113: jmp 0xc118
c115: mov di, 0xb
c118: call 0xf8                              ; EXE timer0_reset (= 00D7(0))
c11b: jmp 0xc16f
c11d: cmp word ptr [0xb2], 0x0
c122: jne 0xc17c
c124: cmp word ptr [0x9b26], 0x2
c129: jne 0xc12e
c12b: call 0xc1bb
c12e: call 0x1ae                             ; EXE ticks_since(0)
c131: cmp ax, 0x3
c134: jbe 0xc172
c136: call 0xf8                              ; EXE timer0_reset (= 00D7(0))
c139: mov word ptr [0x9afa], di
c13d: mov ax, 0x26
c140: push ax
c141: mov ax, 0x2
c144: push ax
c145: mov ax, 0x44
c148: push ax
c149: mov ax, 0xf
c14c: push ax
c14d: call 0x7e74                            ; EXE fill rect (colour, ...)
c150: inc si
c151: cmp si, 0x3
c154: jbe 0xc158
c156: sub si, si
c158: mov ax, si
c15a: mov cx, 0xe
c15d: mul cx
c15f: add ax, word ptr [0xc3bc]
c163: push ax
c164: mov ax, 0x26
c167: push ax
c168: mov ax, 0x5
c16b: push ax
c16c: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
c16f: call 0x6887                            ; EXE flip: work buffer -> screen
c172: call 0x644                             ; EXE mouse_poll: [B2] buttons, [B6] x, [B8] y
c175: call 0x3aa                             ; EXE key_read: next key or 0
c178: or al, al
c17a: je 0xc11d
c17c: call 0x67b6                            ; EXE screen restore 6973(0,0,319,199) unless [9B26]==2 path (G)
c17f: call 0x6837                            ; EXE draw on the screen (6887 first if needed)
c182: push word ptr [0xc3bc]
c186: call 0x19a0                            ; EXE sheet_19A0: free sprite sheet
c189: pop si
c18a: pop di
c18b: ret

; ---- func C18C  size 0x2f  no bp frame, ret
; no caller found (no near call, no pushed offset): dead code in 1990?
c18c: push di
c18d: push si
c18e: cmp word ptr [0x9b26], 0x2
c193: jne 0xc1b8
c195: call 0x84bb                            ; EXE wait_vretrace (port 3DA bit 3)
c198: mov si, 0x9778
c19b: mov di, 0x9762
c19e: cmp di, 0x9778
c1a2: jae 0xc1b8
c1a4: push word ptr [di]
c1a6: push word ptr [si]
c1a8: push word ptr [si + 0x2]
c1ab: push word ptr [si + 0x4]
c1ae: call 0x1b6                             ; EXE vga_set_color(idx, r, g, b) (out 3C8/3C9) (G)
c1b1: add si, 0x6
c1b4: inc di
c1b5: inc di
c1b6: jmp 0xc19e
c1b8: pop si
c1b9: pop di
c1ba: ret

; ---- func C1BB  size 0xb9  no bp frame, ret  callers: C105
c1bb: push di
c1bc: push si
c1bd: mov ax, word ptr [0x9812]
c1c0: mov dx, word ptr [0x9814]
c1c4: add ax, 0x4
c1c7: adc dx, 0x0
c1ca: cmp dx, word ptr [0x44]
c1ce: jbe 0xc1d3
c1d0: jmp 0xc271
c1d3: jb 0xc1de
c1d5: cmp ax, word ptr [0x42]
c1d9: jb 0xc1de
c1db: jmp 0xc271
c1de: inc byte ptr [0x9816]
c1e2: cmp byte ptr [0x9816], 0x5
c1e7: jbe 0xc1ee
c1e9: mov byte ptr [0x9816], 0x0
c1ee: inc byte ptr [0x9817]
c1f2: cmp byte ptr [0x9817], 0x4
c1f7: jbe 0xc1fe
c1f9: mov byte ptr [0x9817], 0x0
c1fe: call 0x84bb                            ; EXE wait_vretrace (port 3DA bit 3)
c201: sub di, di
c203: mov si, 0x9778
c206: jmp 0xc22c
c208: mov al, byte ptr [0x9816]
c20b: sub ah, ah
c20d: add ax, di
c20f: mov cx, 0x6
c212: cdq
c213: idiv cx
c215: mov bx, dx
c217: shl bx
c219: push word ptr [bx - 0x689e]            ; tbl DS:9762
c21d: push word ptr [si]
c21f: push word ptr [si + 0x2]
c222: push word ptr [si + 0x4]
c225: call 0x1b6                             ; EXE vga_set_color(idx, r, g, b) (out 3C8/3C9) (G)
c228: add si, 0x6
c22b: inc di
c22c: cmp si, 0x979c
c230: jb 0xc208
c232: sub di, di
c234: mov si, 0x979c
c237: jmp 0xc25d
c239: mov al, byte ptr [0x9817]
c23c: sub ah, ah
c23e: add ax, di
c240: mov cx, 0x5
c243: cdq
c244: idiv cx
c246: mov bx, dx
c248: shl bx
c24a: push word ptr [bx - 0x6892]            ; tbl DS:976E
c24e: push word ptr [si]
c250: push word ptr [si + 0x2]
c253: push word ptr [si + 0x4]
c256: call 0x1b6                             ; EXE vga_set_color(idx, r, g, b) (out 3C8/3C9) (G)
c259: add si, 0x6
c25c: inc di
c25d: cmp si, 0x97ba
c261: jb 0xc239
c263: mov ax, word ptr [0x42]
c266: mov dx, word ptr [0x44]
c26a: mov word ptr [0x9812], ax
c26d: mov word ptr [0x9814], dx
c271: pop si
c272: pop di
c273: ret

; ---- func C274  size 0x9e  bp frame, locals 10, ret 0x2  callers: BD8A
c274: push bp
c275: mov bp, sp
c277: sub sp, 0xa
c27a: push di
c27b: push si
c27c: sub di, di
c27e: mov word ptr [bp - 0x4], di
c281: mov byte ptr [bp - 0x1], 0x0
c285: mov si, 0x972c
c288: jmp 0xc2c3
c28a: mov al, 0x6
c28c: mul byte ptr [bp + 0x4]
c28f: mov bx, ax
c291: mov al, byte ptr [bp - 0x1]
c294: sub ah, ah
c296: mov word ptr [bp - 0xa], ax
c299: add bx, ax
c29b: shl bx
c29d: cmp word ptr [bx + 0x110e], 0x0        ; tbl DS:110E
c2a2: je 0xc2c0
c2a4: mov byte ptr [bp + di - 0x8], al
c2a7: mov bx, word ptr [bp - 0xa]
c2aa: shl bx
c2ac: mov ax, word ptr [bx - 0x68de]         ; tbl DS:9722
c2b0: mov word ptr [si], ax
c2b2: mov bx, word ptr [bp - 0xa]
c2b5: mov al, byte ptr [bx - 0x68d8]         ; tbl DS:9728
c2b9: mov byte ptr [si + 0x2], al
c2bc: add si, 0xc
c2bf: inc di
c2c0: inc byte ptr [bp - 0x1]
c2c3: cmp byte ptr [bp - 0x1], 0x3
c2c7: jb 0xc28a
c2c9: mov ax, di
c2cb: mov byte ptr [0x975e], al
c2ce: mov si, word ptr [bp - 0x4]
c2d1: cmp di, 0x1
c2d4: jbe 0xc307
c2d6: cmp di, 0x2
c2d9: jne 0xc2e0
c2db: mov ax, 0x76
c2de: jmp 0xc2e3
c2e0: mov ax, 0x82
c2e3: mov word ptr [0x9756], ax
c2e6: mov ax, 0x9750
c2e9: push ax
c2ea: mov cx, 0x2
c2ed: push cx
c2ee: call 0x5614                            ; EXE 5614(src, mode) screen/palette copy into the work buffer
c2f1: mov ax, 0x9750
c2f4: push ax
c2f5: sub ax, ax
c2f7: push ax
c2f8: call 0x54c8                            ; EXE wait for buttons/click (54C8 polling)
c2fb: mov si, ax
c2fd: cmp si, 0xff
c301: jne 0xc307
c303: mov al, -0x1
c305: jmp 0xc30a
c307: mov al, byte ptr [bp + si - 0x8]
c30a: pop si
c30b: pop di
c30c: mov sp, bp
c30e: pop bp
c30f: ret 0x2

; ---- func C312  size 0x3d  no bp frame, ret  callers: B12A
c312: mov ax, word ptr [0xc3a2]
c315: cmp word ptr [0xc3a4], ax
c319: je 0xc322
c31b: push word ptr [0xc3a4]
c31f: call 0x19a0                            ; EXE sheet_19A0: free sprite sheet
c322: push word ptr [0xc3a2]
c326: call 0x19a0                            ; EXE sheet_19A0: free sprite sheet
c329: push word ptr [0xc3a6]
c32d: call 0x19a0                            ; EXE sheet_19A0: free sprite sheet
c330: push word ptr [0xc3a8]
c334: call 0x19a0                            ; EXE sheet_19A0: free sprite sheet
c337: push word ptr [0xc3ae]
c33b: call 0x19a0                            ; EXE sheet_19A0: free sprite sheet
c33e: push word ptr [0xc3ac]
c342: call 0x19a0                            ; EXE sheet_19A0: free sprite sheet
c345: mov bx, word ptr [0x970a]
c349: push word ptr [bx]
c34b: call 0x74b7                            ; EXE free(buf)
c34e: ret

; ---- func C34F  size 0x17  no bp frame, ret  callers: B12A
c34f: push si
c350: call 0x5d20                            ; EXE music_stop
c353: mov si, 0x1
c356: mov al, byte ptr [si - 0x68fc]         ; tbl DS:9704
c35a: push ax
c35b: call 0x5e1a                            ; EXE music_unload(n)
c35e: inc si
c35f: cmp si, 0x5
c362: jb 0xc356
c364: pop si
c365: ret

; ---- func C366  size 0x6d  no bp frame, ret  callers: B12A
c366: push si
c367: sub si, si
c369: cmp byte ptr [0xa2], 0x0
c36e: je 0xc3a0
c370: test byte ptr [0xa2], 0x8
c375: je 0xc37c
c377: mov byte ptr [0xc395], 0x1
c37c: test byte ptr [0xa2], 0x4
c381: je 0xc388
c383: mov byte ptr [0xc395], -0x1
c388: test byte ptr [0xa2], 0x2
c38d: je 0xc394
c38f: mov byte ptr [0xc397], -0x1
c394: test byte ptr [0xa2], 0x1
c399: je 0xc3a0
c39b: mov byte ptr [0xc397], 0x1
c3a0: call 0x3aa                             ; EXE key_read: next key or 0
c3a3: sub ah, ah
c3a5: cmp ax, 0x72
c3a8: je 0xc3cc
c3aa: ja 0xc3cf
c3ac: sub al, 0xd
c3ae: je 0xc3bc
c3b0: sub al, 0x3
c3b2: je 0xc3c3
c3b4: sub al, 0x10
c3b6: je 0xc3c3
c3b8: sub al, 0xb
c3ba: jne 0xc3cf
c3bc: mov byte ptr [0xc3ab], 0x14
c3c1: jmp 0xc3cf
c3c3: mov ax, 0x1
c3c6: push ax
c3c7: call 0xc846
c3ca: jmp 0xc3cf
c3cc: mov si, 0x4
c3cf: mov ax, si
c3d1: pop si
c3d2: ret

; ---- func C3D3  size 0x4f  bp frame, locals 0, ret 0x2  callers: B12A
c3d3: push bp
c3d4: mov bp, sp
c3d6: push di
c3d7: push si
c3d8: mov si, word ptr [bp + 0x4]
c3db: mov cl, 0x3
c3dd: mov di, si
c3df: shl di, cl
c3e1: lea di, [di + 0x3]
c3e4: push di
c3e5: call 0x183                             ; EXE ticks_since(n)
c3e8: mov bl, byte ptr [si - 0x3c86]         ; tbl DS:C37A
c3ec: sub bh, bh
c3ee: mov cl, byte ptr [bx - 0x67d8]         ; tbl DS:9828
c3f2: sub ch, ch
c3f4: cmp cx, ax
c3f6: jae 0xc41a
c3f8: cmp byte ptr [si - 0x3c76], ch         ; tbl DS:C38A
c3fc: je 0xc415
c3fe: cmp byte ptr [si - 0x3c6a], ch         ; tbl DS:C396
c402: je 0xc415
c404: mov al, byte ptr [si - 0x3c6a]         ; tbl DS:C396
c408: add byte ptr [si - 0x3c78], al         ; tbl DS:C388
c40c: and byte ptr [si - 0x3c78], 0xf        ; tbl DS:C388
c411: push di
c412: call 0xd7                              ; EXE timer_start(n) (copy tick DS:42/44 to slot 46+4n)
c415: mov byte ptr [si - 0x3c6a], 0x0        ; tbl DS:C396
c41a: pop si
c41b: pop di
c41c: mov sp, bp
c41e: pop bp
c41f: ret 0x2

; ---- func C422  size 0xb5  bp frame, locals 0, ret 0x2  callers: B12A
c422: push bp
c423: mov bp, sp
c425: push di
c426: push si
c427: mov si, word ptr [bp + 0x4]
c42a: mov cl, 0x3
c42c: mov di, si
c42e: shl di, cl
c430: lea di, [di + 0x4]
c433: push di
c434: call 0x183                             ; EXE ticks_since(n)
c437: mov bl, byte ptr [si - 0x3c86]         ; tbl DS:C37A
c43b: sub bh, bh
c43d: mov cl, byte ptr [bx - 0x67d4]         ; tbl DS:982C
c441: sub ch, ch
c443: cmp cx, ax
c445: jb 0xc44a
c447: jmp 0xc4cf
c44a: cmp byte ptr [si - 0x3c6c], ch         ; tbl DS:C394
c44e: je 0xc499
c450: mov al, byte ptr [si - 0x3c6c]         ; tbl DS:C394
c454: add byte ptr [si - 0x3c74], al         ; tbl DS:C38C
c458: cmp byte ptr [si - 0x3c74], 0x4        ; tbl DS:C38C
c45d: jle 0xc466
c45f: mov byte ptr [si - 0x3c74], 0x4        ; tbl DS:C38C
c464: jmp 0xc494
c466: cmp byte ptr [si - 0x3c74], 0x0        ; tbl DS:C38C
c46b: jg 0xc486
c46d: cmp byte ptr [si - 0x3c70], 0x6        ; tbl DS:C390
c472: je 0xc47a
c474: inc byte ptr [si - 0x3c74]             ; tbl DS:C38C
c478: jmp 0xc499
c47a: sub al, al
c47c: mov byte ptr [si - 0x3c74], al         ; tbl DS:C38C
c480: mov byte ptr [si - 0x3c6c], al         ; tbl DS:C394
c484: jmp 0xc499
c486: cmp si, 0x1
c489: jne 0xc490
c48b: mov byte ptr [0xc37c], 0xa
c490: push di
c491: call 0xd7                              ; EXE timer_start(n) (copy tick DS:42/44 to slot 46+4n)
c494: mov byte ptr [si - 0x3c6c], 0x0        ; tbl DS:C394
c499: mov al, byte ptr [si - 0x3c74]         ; tbl DS:C38C
c49d: cwde
c49e: mov bx, ax
c4a0: mov al, 0x5
c4a2: mul byte ptr [si - 0x3c86]             ; tbl DS:C37A
c4a6: mov di, ax
c4a8: mov al, byte ptr [bx + di - 0x67e8]    ; tbl DS:9818
c4ac: mov byte ptr [si - 0x3c76], al         ; tbl DS:C38A
c4b0: mov ax, si
c4b2: dec ax
c4b3: jne 0xc4cf
c4b5: cmp byte ptr [si - 0x3c76], 0x0        ; tbl DS:C38A
c4ba: je 0xc4cc
c4bc: mov al, byte ptr [0xc38d]
c4bf: cwde
c4c0: mov bx, ax
c4c2: mov al, byte ptr [bx - 0x68fc]         ; tbl DS:9704
c4c6: push ax
c4c7: call 0x5d29                            ; EXE music_play(n)
c4ca: jmp 0xc4cf
c4cc: call 0x5d20                            ; EXE music_stop
c4cf: pop si
c4d0: pop di
c4d1: mov sp, bp
c4d3: pop bp
c4d4: ret 0x2
c4d7: db 0x00                                ; word-alignment pad

; ---- func C4D8  size 0x10  bp frame, locals 0, ret 0x2  callers: B7FA C641 C9A3 CC4D
c4d8: push bp
c4d9: mov bp, sp
c4db: mov bx, word ptr [bp + 0x4]
c4de: shl bx
c4e0: mov ax, word ptr [bx - 0x6786]         ; tbl DS:987A
c4e4: pop bp
c4e5: ret 0x2

; ---- func C4E8  size 0x10  bp frame, locals 0, ret 0x2  callers: B7FA C641 C9A3 CC4D
c4e8: push bp
c4e9: mov bp, sp
c4eb: mov bx, word ptr [bp + 0x4]
c4ee: shl bx
c4f0: mov ax, word ptr [bx - 0x6766]         ; tbl DS:989A
c4f4: pop bp
c4f5: ret 0x2

; ---- func C4F8  size 0x60  bp frame, locals 6, ret 0x8  callers: B12A
c4f8: push bp
c4f9: mov bp, sp
c4fb: sub sp, 0x6
c4fe: push di
c4ff: push si
c500: mov word ptr [bp - 0x6], 0x0
c505: mov ax, word ptr [bp + 0x8]
c508: sub ax, word ptr [bp + 0x4]
c50b: cdq
c50c: xor ax, dx
c50e: sub ax, dx
c510: cdq
c511: push dx
c512: push ax
c513: push dx
c514: push ax
c515: call 0x9d04                            ; EXE long mul
c518: mov cx, ax
c51a: mov ax, word ptr [bp + 0xa]
c51d: sub ax, word ptr [bp + 0x6]
c520: mov bx, dx
c522: cdq
c523: xor ax, dx
c525: sub ax, dx
c527: cdq
c528: push dx
c529: push ax
c52a: push dx
c52b: push ax
c52c: mov si, cx
c52e: mov di, bx
c530: call 0x9d04                            ; EXE long mul
c533: add si, ax
c535: adc di, dx
c537: mov word ptr [bp - 0x4], si
c53a: mov word ptr [bp - 0x2], di
c53d: or di, si
c53f: je 0xc54e
c541: push word ptr [bp - 0x2]
c544: push si
c545: lea ax, [bp - 0x6]
c548: push ax
c549: call 0xc558
c54c: jmp 0xc550
c54e: sub ax, ax
c550: pop si
c551: pop di
c552: mov sp, bp
c554: pop bp
c555: ret 0x8

; ---- func C558  size 0xe9  bp frame, locals 16, ret 0x6  callers: C4F8
c558: push bp
c559: mov bp, sp
c55b: sub sp, 0x10
c55e: mov word ptr [bp - 0xc], 0x8c
c563: mov word ptr [bp - 0xa], 0x0
c568: mov ax, word ptr [bp + 0x6]
c56b: sub ax, 0x4c90
c56e: cdq
c56f: xor ax, dx
c571: sub ax, dx
c573: cdq
c574: mov word ptr [bp - 0x10], ax
c577: mov word ptr [bp - 0xe], dx
c57a: push word ptr [bp - 0xa]
c57d: push word ptr [bp - 0xc]
c580: push word ptr [bp + 0x8]
c583: push word ptr [bp + 0x6]
c586: call 0xa048                            ; EXE unsigned long div
c589: mov cx, ax
c58b: mov bx, dx
c58d: add cx, word ptr [bp - 0xc]
c590: adc bx, word ptr [bp - 0xa]
c593: shr bx
c595: rcr cx
c597: mov word ptr [bp - 0x6], bx
c59a: mov ax, cx
c59c: mul cx
c59e: sub ax, word ptr [bp + 0x6]
c5a1: neg ax
c5a3: cdq
c5a4: xor ax, dx
c5a6: sub ax, dx
c5a8: cdq
c5a9: mov word ptr [bp - 0x4], ax
c5ac: mov word ptr [bp - 0x2], dx
c5af: cmp ax, word ptr [bp - 0x10]
c5b2: jne 0xc5b9
c5b4: cmp dx, word ptr [bp - 0xe]
c5b7: je 0xc5d9
c5b9: mov ax, cx
c5bb: mov dx, bx
c5bd: mov word ptr [bp - 0xc], ax
c5c0: mov word ptr [bp - 0xa], dx
c5c3: mov ax, word ptr [bp - 0x4]
c5c6: mov dx, word ptr [bp - 0x2]
c5c9: mov word ptr [bp - 0x10], ax
c5cc: mov word ptr [bp - 0xe], dx
c5cf: mov bx, word ptr [bp + 0x4]
c5d2: inc word ptr [bx]
c5d4: cmp word ptr [bx], 0x14
c5d7: jne 0xc57a
c5d9: mov ax, word ptr [bp - 0xa]
c5dc: mov cx, word ptr [bp - 0xc]
c5df: add cx, 0x1
c5e2: adc ax, 0x0
c5e5: mov word ptr [bp - 0x6], ax
c5e8: mov ax, cx
c5ea: mul cx
c5ec: sub ax, word ptr [bp + 0x6]
c5ef: neg ax
c5f1: cdq
c5f2: xor ax, dx
c5f4: sub ax, dx
c5f6: cdq
c5f7: cmp dx, word ptr [bp - 0xe]
c5fa: ja 0xc603
c5fc: jb 0xc62d
c5fe: cmp ax, word ptr [bp - 0x10]
c601: jb 0xc62d
c603: mov ax, word ptr [bp - 0xa]
c606: mov cx, word ptr [bp - 0xc]
c609: sub cx, 0x1
c60c: sbb ax, 0x0
c60f: mov word ptr [bp - 0x6], ax
c612: mov ax, cx
c614: mul cx
c616: sub ax, word ptr [bp + 0x6]
c619: neg ax
c61b: cdq
c61c: xor ax, dx
c61e: sub ax, dx
c620: cdq
c621: cmp dx, word ptr [bp - 0xe]
c624: ja 0xc638
c626: jb 0xc62d
c628: cmp ax, word ptr [bp - 0x10]
c62b: jae 0xc638
c62d: mov ax, cx
c62f: mov dx, word ptr [bp - 0x6]
c632: mov word ptr [bp - 0xc], ax
c635: mov word ptr [bp - 0xa], dx
c638: mov ax, word ptr [bp - 0xc]
c63b: mov sp, bp
c63d: pop bp
c63e: ret 0x6

; ---- func C641  size 0xda  bp frame, locals 14, ret 0x4  callers: B12A
c641: push bp
c642: mov bp, sp
c644: sub sp, 0xe
c647: push di
c648: push si
c649: mov word ptr [bp - 0xa], 0x49f0
c64e: mov word ptr [bp - 0x8], 0x2
c653: cmp word ptr [0xc382], 0x64
c658: jbe 0xc664
c65a: mov dx, word ptr [0xc382]
c65e: shr dx
c660: shr dx
c662: jmp 0xc668
c664: mov dx, word ptr [0xc382]
c668: mov byte ptr [bp - 0x1], 0x0
c66c: mov word ptr [bp - 0xc], dx
c66f: jmp 0xc707
c672: mov al, byte ptr [bp - 0x1]
c675: cwde
c676: push ax
c677: mov si, ax
c679: call 0xc4d8
c67c: imul word ptr [bp - 0xc]
c67f: cdq
c680: xor ax, dx
c682: sub ax, dx
c684: mov cx, 0x8
c687: sar ax, cl
c689: xor ax, dx
c68b: sub ax, dx
c68d: mov cx, si
c68f: mov si, ax
c691: push cx
c692: call 0xc4e8
c695: imul word ptr [bp - 0xc]
c698: cdq
c699: xor ax, dx
c69b: sub ax, dx
c69d: mov cx, 0x8
c6a0: sar ax, cl
c6a2: xor ax, dx
c6a4: sub ax, dx
c6a6: mov di, ax
c6a8: mov bx, word ptr [bp + 0x4]
c6ab: mov ax, word ptr [bx]
c6ad: mov bx, word ptr [bp + 0x6]
c6b0: sub ax, word ptr [bx]
c6b2: sub ax, si
c6b4: cdq
c6b5: xor ax, dx
c6b7: sub ax, dx
c6b9: cdq
c6ba: push dx
c6bb: push ax
c6bc: push dx
c6bd: push ax
c6be: call 0x9d04                            ; EXE long mul
c6c1: mov bx, word ptr [bp + 0x4]
c6c4: mov cx, ax
c6c6: mov ax, word ptr [bx + 0x2]
c6c9: mov bx, word ptr [bp + 0x6]
c6cc: sub ax, word ptr [bx + 0x2]
c6cf: sub ax, di
c6d1: mov bx, dx
c6d3: cdq
c6d4: xor ax, dx
c6d6: sub ax, dx
c6d8: cdq
c6d9: push dx
c6da: push ax
c6db: push dx
c6dc: push ax
c6dd: mov si, cx
c6df: mov di, bx
c6e1: call 0x9d04                            ; EXE long mul
c6e4: add si, ax
c6e6: adc di, dx
c6e8: cmp di, word ptr [bp - 0x8]
c6eb: ja 0xc704
c6ed: jb 0xc6f4
c6ef: cmp si, word ptr [bp - 0xa]
c6f2: jae 0xc704
c6f4: mov al, byte ptr [bp - 0x1]
c6f7: mov byte ptr [bp - 0xd], al
c6fa: mov ax, si
c6fc: mov dx, di
c6fe: mov word ptr [bp - 0xa], ax
c701: mov word ptr [bp - 0x8], dx
c704: inc byte ptr [bp - 0x1]
c707: cmp byte ptr [bp - 0x1], 0x10
c70b: jge 0xc710
c70d: jmp 0xc672
c710: mov al, byte ptr [bp - 0xd]
c713: pop si
c714: pop di
c715: mov sp, bp
c717: pop bp
c718: ret 0x4

; ---- func C71B  size 0x12a  no bp frame, ret  callers: B12A BB7E BEAE
c71b: push si
c71c: mov ax, 0xc7
c71f: mov word ptr [0xc0e0], ax
c722: mov word ptr [0xc1ea], ax
c725: mov word ptr [0x9ac6], ax
c728: cmp byte ptr [0xc38e], 0x0
c72d: jg 0xc73a
c72f: mov byte ptr [0xc38e], 0x0
c734: mov word ptr [0xc37e], 0x0
c73a: mov word ptr [0x9afa], 0x4
c740: sub ax, ax
c742: push ax
c743: mov ax, 0xbe
c746: push ax
c747: mov al, 0x73
c749: imul byte ptr [0xc38e]
c74d: mov cx, 0x3c
c750: cdq
c751: idiv cx
c753: mov si, ax
c755: push si
c756: mov ax, 0xc4
c759: push ax
c75a: call 0x7e74                            ; EXE fill rect (colour, ...)
c75d: mov ax, word ptr [0x6346]
c760: mov word ptr [0x9afa], ax
c763: push si
c764: mov ax, 0xbe
c767: push ax
c768: mov ax, 0x73
c76b: push ax
c76c: mov ax, 0xc4
c76f: push ax
c770: call 0x7e74                            ; EXE fill rect (colour, ...)
c773: cmp byte ptr [0xc38f], 0x0
c778: jg 0xc785
c77a: mov byte ptr [0xc38f], 0x0
c77f: mov word ptr [0xc380], 0x0
c785: mov word ptr [0x9afa], 0x4
c78b: mov ax, 0x13f
c78e: push ax
c78f: mov cx, 0xbe
c792: push cx
c793: mov al, 0x73
c795: imul byte ptr [0xc38f]
c799: mov bx, 0x3c
c79c: cdq
c79d: idiv bx
c79f: mov si, ax
c7a1: mov ax, 0x13f
c7a4: sub ax, si
c7a6: push ax
c7a7: mov ax, 0xc4
c7aa: push ax
c7ab: call 0x7e74                            ; EXE fill rect (colour, ...)
c7ae: mov ax, word ptr [0x6346]
c7b1: mov word ptr [0x9afa], ax
c7b4: sub si, 0x13f
c7b8: neg si
c7ba: push si
c7bb: mov ax, 0xbe
c7be: push ax
c7bf: mov ax, 0xcc
c7c2: push ax
c7c3: mov ax, 0xc4
c7c6: push ax
c7c7: call 0x7e74                            ; EXE fill rect (colour, ...)
c7ca: mov ax, 0x77
c7cd: push ax
c7ce: mov ax, 0xc1
c7d1: push ax
c7d2: mov ax, 0xc7
c7d5: push ax
c7d6: push ax
c7d7: call 0x7e74                            ; EXE fill rect (colour, ...)
c7da: mov word ptr [0x9b69], 0x0
c7e0: cmp byte ptr [0xc37c], 0x0
c7e5: jne 0xc81c
c7e7: mov ax, 0x98ba                         ; "MEN"
c7ea: push ax
c7eb: mov ax, 0x9f
c7ee: push ax
c7ef: mov ax, 0xc1
c7f2: push ax
c7f3: call 0x8be4                            ; EXE centred_text(str, cx, y)
c7f6: mov ax, word ptr [0xc380]
c7f9: sub dx, dx
c7fb: push dx
c7fc: push ax
c7fd: mov ax, 0xae
c800: push ax
c801: mov ax, 0xc1
c804: push ax
c805: call 0x8a9e                            ; EXE print number (left)
c808: mov ax, word ptr [0xc37e]
c80b: sub dx, dx
c80d: push dx
c80e: push ax
c80f: mov ax, 0x90
c812: push ax
c813: mov ax, 0xc1
c816: push ax
c817: call 0x8ac8                            ; EXE print number (right-aligned)
c81a: jmp 0xc837
c81c: dec byte ptr [0xc37c]
c820: mov al, byte ptr [0xc38d]
c823: cwde
c824: mov bx, ax
c826: shl bx
c828: push word ptr [bx - 0x6790]            ; tbl DS:9870
c82c: mov ax, 0x9f
c82f: push ax
c830: mov ax, 0xc1
c833: push ax
c834: call 0x8be4                            ; EXE centred_text(str, cx, y)
c837: mov ax, 0xb9
c83a: mov word ptr [0xc0e0], ax
c83d: mov word ptr [0xc1ea], ax
c840: mov word ptr [0x9ac6], ax
c843: pop si
c844: ret
c845: db 0x00                                ; word-alignment pad

; ---- func C846  size 0xe5  bp frame, locals 6, ret 0x2  callers: B6A0 C366
c846: push bp
c847: mov bp, sp
c849: sub sp, 0x6
c84c: push di
c84d: push si
c84e: mov si, word ptr [bp + 0x4]
c851: mov cl, 0x3
c853: mov di, si
c855: shl di, cl
c857: add di, 0x5
c85a: cmp byte ptr [si - 0x3c6e], 0x0        ; tbl DS:C392
c85f: jne 0xc871
c861: cmp byte ptr [si - 0x3c46], 0x0        ; tbl DS:C3BA
c866: jne 0xc871
c868: push di
c869: call 0x183                             ; EXE ticks_since(n)
c86c: cmp ax, 0x90
c86f: ja 0xc874
c871: jmp 0xc923
c874: push di
c875: call 0xd7                              ; EXE timer_start(n) (copy tick DS:42/44 to slot 46+4n)
c878: mov byte ptr [si - 0x3c6e], -0x1       ; tbl DS:C392
c87d: mov al, byte ptr [si - 0x3c78]         ; tbl DS:C388
c881: mov byte ptr [si - 0x3c34], al         ; tbl DS:C3CC
c885: mov bx, si
c887: shl bx
c889: shl bx
c88b: mov cx, word ptr [bx - 0x3c8e]         ; tbl DS:C372
c88f: mov dx, word ptr [bx - 0x3c8c]         ; tbl DS:C374
c893: mov word ptr [bx - 0x3c30], cx         ; tbl DS:C3D0
c897: mov word ptr [bx - 0x3c2e], dx         ; tbl DS:C3D2
c89b: mov cx, word ptr [0x56fe]
c89f: mov dx, word ptr [0x5700]
c8a3: mov word ptr [bx - 0x3c20], cx         ; tbl DS:C3E0
c8a7: mov word ptr [bx - 0x3c1e], dx         ; tbl DS:C3E2
c8ab: mov byte ptr [si - 0x6742], 0x0        ; tbl DS:98BE
c8b0: mov cx, word ptr [0xc382]
c8b4: mov bx, si
c8b6: shl bx
c8b8: add bx, 0xc3c8
c8bc: mov word ptr [bp - 0x6], bx
c8bf: mov word ptr [bx], cx
c8c1: mov bx, si
c8c3: xor bl, 0x1
c8c6: mov cx, ax
c8c8: mov al, byte ptr [bx - 0x3c78]         ; tbl DS:C388
c8cc: cwde
c8cd: mov dx, ax
c8cf: mov al, cl
c8d1: cwde
c8d2: sub ax, dx
c8d4: cdq
c8d5: xor ax, dx
c8d7: sub ax, dx
c8d9: mov bx, ax
c8db: mov al, byte ptr [bx - 0x6730]         ; tbl DS:98D0
c8df: cwde
c8e0: mov word ptr [bp - 0x4], ax
c8e3: or ax, ax
c8e5: je 0xc8f3
c8e7: mov ax, word ptr [0xc382]
c8ea: cdq
c8eb: idiv word ptr [bp - 0x4]
c8ee: mov word ptr [bp - 0x2], ax
c8f1: jmp 0xc8f8
c8f3: mov word ptr [bp - 0x2], 0x0
c8f8: mov bx, si
c8fa: xor bl, 0x1
c8fd: cmp byte ptr [bx - 0x3c76], 0x4        ; tbl DS:C38A
c902: jge 0xc910
c904: mov cx, 0x2
c907: mov ax, word ptr [bp - 0x2]
c90a: cdq
c90b: idiv cx
c90d: mov word ptr [bp - 0x2], ax
c910: push word ptr [0xc382]
c914: push word ptr [bp - 0x2]
c917: mov ax, 0x5a
c91a: push ax
c91b: call 0x50f                             ; EXE step_toward_min: max(v - step, min), signed (050F family)
c91e: mov bx, word ptr [bp - 0x6]
c921: mov word ptr [bx], ax
c923: pop si
c924: pop di
c925: mov sp, bp
c927: pop bp
c928: ret 0x2

; ---- func C92B  size 0x78  bp frame, locals 0, ret 0x2  callers: B12A
c92b: push bp
c92c: mov bp, sp
c92e: push si
c92f: mov si, word ptr [bp + 0x4]
c932: cmp byte ptr [si - 0x3c46], 0x0        ; tbl DS:C3BA
c937: jne 0xc99c
c939: cmp byte ptr [si - 0x3c6e], 0x0        ; tbl DS:C392
c93e: jne 0xc99c
c940: cmp word ptr [0xc382], 0x5a
c945: jae 0xc99c
c947: mov bx, si
c949: shl bx
c94b: cmp word ptr [bx - 0x3c82], 0x0        ; tbl DS:C37E
c950: je 0xc99c
c952: mov byte ptr [si - 0x3c46], -0x1       ; tbl DS:C3BA
c957: mov al, byte ptr [si - 0x3c90]         ; tbl DS:C370
c95b: mov byte ptr [si - 0x3c32], al         ; tbl DS:C3CE
c95f: mov ax, si
c961: shl si
c963: shl si
c965: mov cx, word ptr [si - 0x3c8e]         ; tbl DS:C372
c969: mov dx, word ptr [si - 0x3c8c]         ; tbl DS:C374
c96d: mov word ptr [si - 0x3c28], cx         ; tbl DS:C3D8
c971: mov word ptr [si - 0x3c26], dx         ; tbl DS:C3DA
c975: mov cx, word ptr [0x56fe]
c979: mov dx, word ptr [0x5700]
c97d: mov word ptr [si - 0x3c18], cx         ; tbl DS:C3E8
c981: mov word ptr [si - 0x3c16], dx         ; tbl DS:C3EA
c985: push word ptr [0xc382]
c989: mov cx, 0xa
c98c: push cx
c98d: mov cx, 0x7
c990: push cx
c991: mov si, ax
c993: call 0x50f                             ; EXE step_toward_min: max(v - step, min), signed (050F family)
c996: shl si
c998: mov word ptr [si - 0x3c3c], ax         ; tbl DS:C3C4
c99c: pop si
c99d: mov sp, bp
c99f: pop bp
c9a0: ret 0x2

; ---- func C9A3  size 0x16e  bp frame, locals 16, ret 0x2  callers: B12A
c9a3: push bp
c9a4: mov bp, sp
c9a6: sub sp, 0x10
c9a9: push di
c9aa: push si
c9ab: mov di, word ptr [bp + 0x4]
c9ae: mov cl, 0x3
c9b0: mov ax, di
c9b2: shl ax, cl
c9b4: inc ax
c9b5: mov word ptr [bp - 0xa], ax
c9b8: cmp byte ptr [di - 0x3c46], 0x0        ; tbl DS:C3BA
c9bd: je 0xc9c8
c9bf: push ax
c9c0: call 0x183                             ; EXE ticks_since(n)
c9c3: cmp ax, 0x8
c9c6: ja 0xc9cb
c9c8: jmp 0xcb09
c9cb: mov bx, di
c9cd: shl bx
c9cf: add bx, 0xc3c4
c9d3: mov word ptr [bp - 0xc], bx
c9d6: cmp word ptr [bx], 0x0
c9d9: jg 0xc9e3
c9db: mov byte ptr [di - 0x3c46], 0x0        ; tbl DS:C3BA
c9e0: jmp 0xcb03
c9e3: sub word ptr [bx], 0x6
c9e6: mov al, byte ptr [di - 0x3c32]         ; tbl DS:C3CE
c9ea: cwde
c9eb: push ax
c9ec: call 0xc4d8
c9ef: mov cx, 0x6
c9f2: imul cx
c9f4: mov si, ax
c9f6: mov bx, di
c9f8: shl bx
c9fa: shl bx
c9fc: mov word ptr [bp - 0xe], bx
c9ff: add si, word ptr [bx - 0x3c18]         ; tbl DS:C3E8
ca03: mov ax, si
ca05: sub ah, ah
ca07: mov word ptr [bx - 0x3c18], ax         ; tbl DS:C3E8
ca0b: mov ax, si
ca0d: cdq
ca0e: xor ax, dx
ca10: sub ax, dx
ca12: mov cx, 0x8
ca15: sar ax, cl
ca17: xor ax, dx
ca19: sub ax, dx
ca1b: add word ptr [bx - 0x3c28], ax         ; tbl DS:C3D8
ca1f: or si, si
ca21: jge 0xca2a
ca23: mov bx, word ptr [bp - 0xe]
ca26: neg word ptr [bx - 0x3c18]             ; tbl DS:C3E8
ca2a: mov al, byte ptr [di - 0x3c32]         ; tbl DS:C3CE
ca2e: cwde
ca2f: push ax
ca30: call 0xc4e8
ca33: mov cx, 0x6
ca36: imul cx
ca38: mov si, ax
ca3a: mov bx, word ptr [bp - 0xe]
ca3d: add si, word ptr [bx - 0x3c16]         ; tbl DS:C3EA
ca41: mov ax, si
ca43: sub ah, ah
ca45: mov word ptr [bx - 0x3c16], ax         ; tbl DS:C3EA
ca49: mov ax, si
ca4b: cdq
ca4c: xor ax, dx
ca4e: sub ax, dx
ca50: mov cx, 0x8
ca53: sar ax, cl
ca55: xor ax, dx
ca57: sub ax, dx
ca59: add word ptr [bx - 0x3c26], ax         ; tbl DS:C3DA
ca5d: or si, si
ca5f: jge 0xca68
ca61: mov bx, word ptr [bp - 0xe]
ca64: neg word ptr [bx - 0x3c16]             ; tbl DS:C3EA
ca68: mov bx, word ptr [bp - 0xc]
ca6b: cmp word ptr [bx], 0x0
ca6e: jg 0xcada
ca70: mov ax, di
ca72: xor al, 0x1
ca74: mov word ptr [bp - 0x8], ax
ca77: push ax
ca78: call 0xd11b
ca7b: or ax, ax
ca7d: je 0xcada
ca7f: cmp word ptr [bp - 0x8], 0x0
ca83: jne 0xca91
ca85: mov bl, byte ptr [0x491]
ca89: sub bh, bh
ca8b: mov al, byte ptr [bx - 0x6738]         ; tbl DS:98C8
ca8f: jmp 0xca9b
ca91: mov bl, byte ptr [0x491]
ca95: sub bh, bh
ca97: mov al, byte ptr [bx - 0x6734]         ; tbl DS:98CC
ca9b: sub ah, ah
ca9d: mov word ptr [bp - 0x6], ax
caa0: sub ax, ax
caa2: mov word ptr [bp - 0x4], ax
caa5: mov word ptr [bp - 0x2], ax
caa8: jmp 0xcabc
caaa: mov ax, 0x64
caad: push ax
caae: call 0x76b                             ; EXE rng_roll(n)
cab1: cmp ax, word ptr [bp - 0x6]
cab4: jae 0xcab9
cab6: inc word ptr [bp - 0x4]
cab9: inc word ptr [bp - 0x2]
cabc: cmp word ptr [bp - 0x2], 0xa
cac0: jb 0xcaaa
cac2: mov bx, word ptr [bp - 0x8]
cac5: shl bx
cac7: add bx, 0xc37e
cacb: mov ax, word ptr [bp - 0x4]
cace: cmp word ptr [bx], ax
cad0: jbe 0xcad6
cad2: sub word ptr [bx], ax
cad4: jmp 0xcada
cad6: mov word ptr [bx], 0x0
cada: mov bx, word ptr [bp - 0xe]
cadd: cmp word ptr [bx - 0x3c28], 0x0        ; tbl DS:C3D8
cae2: jle 0xcafb
cae4: cmp word ptr [bx - 0x3c28], 0x13f      ; tbl DS:C3D8
caea: jge 0xcafb
caec: cmp word ptr [bx - 0x3c26], 0x0        ; tbl DS:C3DA
caf1: jle 0xcafb
caf3: cmp word ptr [bx - 0x3c26], 0xb9       ; tbl DS:C3DA
caf9: jl 0xcb03
cafb: mov bx, word ptr [bp + 0x4]
cafe: mov byte ptr [bx - 0x3c46], 0x0        ; tbl DS:C3BA
cb03: push word ptr [bp - 0xa]
cb06: call 0xd7                              ; EXE timer_start(n) (copy tick DS:42/44 to slot 46+4n)
cb09: pop si
cb0a: pop di
cb0b: mov sp, bp
cb0d: pop bp
cb0e: ret 0x2

; ---- func CB11  size 0x13c  bp frame, locals 12, ret 0x2  callers: B12A
cb11: push bp
cb12: mov bp, sp
cb14: sub sp, 0xc
cb17: push di
cb18: push si
cb19: mov di, word ptr [bp + 0x4]
cb1c: mov cl, 0x3
cb1e: mov ax, di
cb20: shl ax, cl
cb22: inc ax
cb23: mov word ptr [bp - 0x6], ax
cb26: mov word ptr [bp - 0x8], 0x0
cb2b: cmp byte ptr [di - 0x3c6e], 0x0        ; tbl DS:C392
cb30: je 0xcb3b
cb32: push ax
cb33: call 0x183                             ; EXE ticks_since(n)
cb36: cmp ax, 0x6
cb39: ja 0xcb3e
cb3b: jmp 0xcc45
cb3e: cmp byte ptr [di - 0x3c6e], 0x1        ; tbl DS:C392
cb43: jne 0xcb5b
cb45: inc byte ptr [di - 0x6742]             ; tbl DS:98BE
cb49: cmp byte ptr [di - 0x6742], 0x39       ; tbl DS:98BE
cb4e: jg 0xcb53
cb50: jmp 0xcc3f
cb53: mov byte ptr [di - 0x3c6e], 0x0        ; tbl DS:C392
cb58: jmp 0xcc3f
cb5b: xor byte ptr [di - 0x6742], 0x10       ; tbl DS:98BE
cb60: mov bx, di
cb62: sub word ptr [bx + di - 0x3c38], 0xf   ; tbl DS:C3C8
cb67: mov ax, word ptr [bx + di - 0x3c38]    ; tbl DS:C3C8
cb6b: or ax, ax
cb6d: jle 0xcba0
cb6f: push di
cb70: mov ax, 0xf
cb73: push ax
cb74: call 0xcc4d
cb77: mov bx, di
cb79: shl bx
cb7b: shl bx
cb7d: cmp word ptr [bx - 0x3c30], 0x0        ; tbl DS:C3D0
cb82: jle 0xcb53
cb84: cmp word ptr [bx - 0x3c30], 0x13f      ; tbl DS:C3D0
cb8a: jge 0xcb53
cb8c: cmp word ptr [bx - 0x3c2e], 0x0        ; tbl DS:C3D2
cb91: jle 0xcb53
cb93: cmp word ptr [bx - 0x3c2e], 0xb9       ; tbl DS:C3D2
cb99: jge 0xcb9e
cb9b: jmp 0xcc3f
cb9e: jmp 0xcb53
cba0: add ax, 0xf
cba3: mov word ptr [bp - 0x2], ax
cba6: mov ax, di
cba8: xor al, 0x1
cbaa: mov word ptr [bp - 0x4], ax
cbad: sub si, si
cbaf: cmp word ptr [bp - 0x2], si
cbb2: jbe 0xcbdc
cbb4: push di
cbb5: mov ax, 0x1
cbb8: push ax
cbb9: call 0xcc4d
cbbc: mov bx, di
cbbe: shl bx
cbc0: shl bx
cbc2: push word ptr [bx - 0x3c30]            ; tbl DS:C3D0
cbc6: push word ptr [bx - 0x3c2e]            ; tbl DS:C3D2
cbca: push word ptr [bp - 0x4]
cbcd: call 0xd070
cbd0: or ax, ax
cbd2: jne 0xcbd7
cbd4: inc si
cbd5: jmp 0xcbaf
cbd7: mov word ptr [bp - 0x8], 0xffff
cbdc: cmp word ptr [bp - 0x8], 0x0
cbe0: je 0xcc35
cbe2: cmp word ptr [bp - 0x4], 0x0
cbe6: jne 0xcbf4
cbe8: mov bl, byte ptr [0x491]
cbec: sub bh, bh
cbee: mov al, byte ptr [bx - 0x6740]         ; tbl DS:98C0
cbf2: jmp 0xcbfe
cbf4: mov bl, byte ptr [0x491]
cbf8: sub bh, bh
cbfa: mov al, byte ptr [bx - 0x673c]         ; tbl DS:98C4
cbfe: mov byte ptr [bp - 0x1], al
cc01: mov bx, word ptr [bp - 0x4]
cc04: sub byte ptr [bx - 0x3c72], al         ; tbl DS:C38E
cc08: shl bx
cc0a: push word ptr [bx - 0x3c82]            ; tbl DS:C37E
cc0e: cwde
cc0f: shl ax
cc11: push ax
cc12: mov si, bx
cc14: call 0x76b                             ; EXE rng_roll(n)
cc17: push ax
cc18: sub ax, ax
cc1a: push ax
cc1b: call 0x50f                             ; EXE step_toward_min: max(v - step, min), signed (050F family)
cc1e: mov word ptr [si - 0x3c82], ax         ; tbl DS:C37E
cc22: mov ax, 0x32
cc25: push ax
cc26: call 0x76b                             ; EXE rng_roll(n)
cc29: add al, 0xa
cc2b: mov bx, word ptr [bp - 0x4]
cc2e: mov byte ptr [bx - 0x3c60], al         ; tbl DS:C3A0
cc32: jmp 0xcb53
cc35: mov byte ptr [di - 0x3c6e], 0x1        ; tbl DS:C392
cc3a: mov byte ptr [di - 0x6742], 0x33       ; tbl DS:98BE
cc3f: push word ptr [bp - 0x6]
cc42: call 0xd7                              ; EXE timer_start(n) (copy tick DS:42/44 to slot 46+4n)
cc45: pop si
cc46: pop di
cc47: mov sp, bp
cc49: pop bp
cc4a: ret 0x2

; ---- func CC4D  size 0x91  bp frame, locals 2, ret 0x4  callers: CB11
cc4d: push bp
cc4e: mov bp, sp
cc50: sub sp, 0x2
cc53: push di
cc54: push si
cc55: mov si, word ptr [bp + 0x6]
cc58: mov al, byte ptr [si - 0x3c34]         ; tbl DS:C3CC
cc5c: cwde
cc5d: push ax
cc5e: call 0xc4d8
cc61: imul word ptr [bp + 0x4]
cc64: mov di, ax
cc66: mov bx, si
cc68: shl bx
cc6a: shl bx
cc6c: mov word ptr [bp - 0x2], bx
cc6f: add di, word ptr [bx - 0x3c20]         ; tbl DS:C3E0
cc73: mov ax, di
cc75: sub ah, ah
cc77: mov word ptr [bx - 0x3c20], ax         ; tbl DS:C3E0
cc7b: mov ax, di
cc7d: cdq
cc7e: xor ax, dx
cc80: sub ax, dx
cc82: mov cx, 0x8
cc85: sar ax, cl
cc87: xor ax, dx
cc89: sub ax, dx
cc8b: add word ptr [bx - 0x3c30], ax         ; tbl DS:C3D0
cc8f: or di, di
cc91: jge 0xcc9a
cc93: mov bx, word ptr [bp - 0x2]
cc96: neg word ptr [bx - 0x3c20]             ; tbl DS:C3E0
cc9a: mov al, byte ptr [si - 0x3c34]         ; tbl DS:C3CC
cc9e: cwde
cc9f: push ax
cca0: call 0xc4e8
cca3: imul word ptr [bp + 0x4]
cca6: mov di, ax
cca8: mov bx, word ptr [bp - 0x2]
ccab: add di, word ptr [bx - 0x3c1e]         ; tbl DS:C3E2
ccaf: mov ax, di
ccb1: sub ah, ah
ccb3: mov word ptr [bx - 0x3c1e], ax         ; tbl DS:C3E2
ccb7: mov ax, di
ccb9: cdq
ccba: xor ax, dx
ccbc: sub ax, dx
ccbe: mov cx, 0x8
ccc1: sar ax, cl
ccc3: xor ax, dx
ccc5: sub ax, dx
ccc7: add word ptr [bx - 0x3c2e], ax         ; tbl DS:C3D2
cccb: or di, di
cccd: jge 0xccd6
cccf: mov bx, word ptr [bp - 0x2]
ccd2: neg word ptr [bx - 0x3c1e]             ; tbl DS:C3E2
ccd6: pop si
ccd7: pop di
ccd8: mov sp, bp
ccda: pop bp
ccdb: ret 0x4

; ---- func CCDE  size 0x6d  bp frame, locals 2, ret 0x2  callers: B12A
ccde: push bp
ccdf: mov bp, sp
cce1: sub sp, 0x2
cce4: push si
cce5: mov si, word ptr [bp + 0x4]
cce8: cmp byte ptr [si - 0x3c6e], 0x0        ; tbl DS:C392
cced: je 0xcd1f
ccef: mov al, byte ptr [si - 0x6742]         ; tbl DS:98BE
ccf3: mov byte ptr [bp - 0x1], al
ccf6: cmp byte ptr [si - 0x3c6e], 0x1        ; tbl DS:C392
ccfb: je 0xcd04
ccfd: mov al, byte ptr [si - 0x3c34]         ; tbl DS:C3CC
cd01: add byte ptr [bp - 0x1], al
cd04: mov al, 0xe
cd06: imul byte ptr [bp - 0x1]
cd09: add ax, word ptr [0xc3ae]
cd0d: push ax
cd0e: mov bx, si
cd10: shl bx
cd12: shl bx
cd14: push word ptr [bx - 0x3c30]            ; tbl DS:C3D0
cd18: push word ptr [bx - 0x3c2e]            ; tbl DS:C3D2
cd1c: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
cd1f: cmp byte ptr [si - 0x3c46], 0x0        ; tbl DS:C3BA
cd24: je 0xcd44
cd26: mov al, byte ptr [si - 0x3c32]         ; tbl DS:C3CE
cd2a: add al, 0x20
cd2c: mov cl, 0xe
cd2e: imul cl
cd30: add ax, word ptr [0xc3ae]
cd34: push ax
cd35: shl si
cd37: shl si
cd39: push word ptr [si - 0x3c28]            ; tbl DS:C3D8
cd3d: push word ptr [si - 0x3c26]            ; tbl DS:C3DA
cd41: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
cd44: pop si
cd45: mov sp, bp
cd47: pop bp
cd48: ret 0x2

; ---- func CD4B  size 0x4b  bp frame, locals 0, ret 0x2  callers: B8E3 B96A
cd4b: push bp
cd4c: mov bp, sp
cd4e: push si
cd4f: mov si, word ptr [bp + 0x4]
cd52: dec byte ptr [si - 0x3c60]             ; tbl DS:C3A0
cd56: test byte ptr [si - 0x3c60], 0xf       ; tbl DS:C3A0
cd5b: jne 0xcd61
cd5d: dec byte ptr [si - 0x3c72]             ; tbl DS:C38E
cd61: inc byte ptr [si - 0x6720]             ; tbl DS:98E0
cd65: cmp byte ptr [si - 0x6720], 0x3        ; tbl DS:98E0
cd6a: jl 0xcd71
cd6c: mov byte ptr [si - 0x6720], 0x0        ; tbl DS:98E0
cd71: mov al, byte ptr [si - 0x6720]         ; tbl DS:98E0
cd75: add al, 0x30
cd77: mov cl, 0xe
cd79: imul cl
cd7b: add ax, word ptr [0xc3ae]
cd7f: push ax
cd80: shl si
cd82: shl si
cd84: push word ptr [si - 0x3c8e]            ; tbl DS:C372
cd88: push word ptr [si - 0x3c8c]            ; tbl DS:C374
cd8c: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
cd8f: pop si
cd90: mov sp, bp
cd92: pop bp
cd93: ret 0x2

; ---- func CD96  size 0xe8  bp frame, locals 6, ret  callers: B12A
cd96: push bp
cd97: mov bp, sp
cd99: sub sp, 0x6
cd9c: push di
cd9d: push si
cd9e: sub di, di
cda0: call 0xce7e
cda3: or ax, ax
cda5: jne 0xcdaa
cda7: jmp 0xce76
cdaa: mov bx, word ptr [0xc384]
cdae: mov al, byte ptr [bx - 0x3c78]         ; tbl DS:C388
cdb2: mov si, bx
cdb4: xor si, 0x1
cdb7: sub al, byte ptr [si - 0x3c78]         ; tbl DS:C388
cdbb: cwde
cdbc: xor al, ah
cdbe: sub al, ah
cdc0: mov byte ptr [bp - 0x2], al
cdc3: mov al, byte ptr [bx - 0x3c86]         ; tbl DS:C37A
cdc7: mov byte ptr [bp - 0x3], al
cdca: dec bx
cdcb: jne 0xcdd8
cdcd: mov al, byte ptr [0x491]
cdd0: mov cx, ax
cdd2: shl al
cdd4: add al, cl
cdd6: jmp 0xcdda
cdd8: mov al, 0x6
cdda: add byte ptr [bp - 0x3], al
cddd: mov ax, 0x10
cde0: push ax
cde1: call 0x76b                             ; EXE rng_roll(n)
cde4: mov bl, byte ptr [bp - 0x3]
cde7: sub bh, bh
cde9: mov cx, ax
cdeb: mov al, byte ptr [bx - 0x6644]         ; tbl DS:99BC
cdef: mov bx, word ptr [0xc384]
cdf3: mul byte ptr [bx - 0x3c74]             ; tbl DS:C38C
cdf7: add cl, al
cdf9: sub cl, 0x8
cdfc: mov byte ptr [bp - 0x1], cl
cdff: cmp byte ptr [bp - 0x2], 0x2
ce03: jle 0xce0b
ce05: cmp byte ptr [bp - 0x2], 0xe
ce09: jl 0xce0e
ce0b: shr byte ptr [bp - 0x1]
ce0e: mov al, byte ptr [bp - 0x1]
ce11: sub byte ptr [si - 0x3c72], al         ; tbl DS:C38E
ce15: mov bx, si
ce17: push word ptr [bx + si - 0x3c82]       ; tbl DS:C37E
ce1b: mov al, byte ptr [bp - 0x1]
ce1e: sub ah, ah
ce20: push ax
ce21: call 0x76b                             ; EXE rng_roll(n)
ce24: push ax
ce25: sub ax, ax
ce27: push ax
ce28: call 0x50f                             ; EXE step_toward_min: max(v - step, min), signed (050F family)
ce2b: mov bx, si
ce2d: mov word ptr [bx + si - 0x3c82], ax    ; tbl DS:C37E
ce31: mov bx, word ptr [0xc384]
ce35: mov al, byte ptr [bx - 0x6a2e]         ; tbl DS:95D2
ce39: sub ah, ah
ce3b: mov di, ax
ce3d: cmp byte ptr [bp - 0x2], 0x6
ce41: jl 0xce76
ce43: cmp byte ptr [bp - 0x2], 0xa
ce47: jg 0xce76
ce49: mov al, byte ptr [bp - 0x1]
ce4c: sub byte ptr [bx - 0x3c72], al         ; tbl DS:C38E
ce50: shl bx
ce52: push word ptr [bx - 0x3c82]            ; tbl DS:C37E
ce56: mov al, byte ptr [bp - 0x1]
ce59: push ax
ce5a: mov word ptr [bp - 0x6], bx
ce5d: call 0x76b                             ; EXE rng_roll(n)
ce60: push ax
ce61: sub ax, ax
ce63: push ax
ce64: call 0x50f                             ; EXE step_toward_min: max(v - step, min), signed (050F family)
ce67: mov bx, word ptr [bp - 0x6]
ce6a: mov word ptr [bx - 0x3c82], ax         ; tbl DS:C37E
ce6e: mov al, byte ptr [si - 0x6a2e]         ; tbl DS:95D2
ce72: sub ah, ah
ce74: or di, ax
ce76: mov ax, di
ce78: pop si
ce79: pop di
ce7a: mov sp, bp
ce7c: pop bp
ce7d: ret

; ---- func CE7E  size 0x101  bp frame, locals 38, ret  callers: CD96
ce7e: push bp
ce7f: mov bp, sp
ce81: sub sp, 0x26
ce84: push si
ce85: mov ax, 0xc3f8
ce88: push ax
ce89: mov ax, 0xc3f0
ce8c: push ax
ce8d: lea ax, [bp - 0x26]
ce90: push ax
ce91: call 0x817c                            ; EXE sprite clip against C0DA..C0E0
ce94: or ax, ax
ce96: je 0xcef3
ce98: mov ax, word ptr [0xc376]
ce9b: mov word ptr [bp - 0x12], ax
ce9e: mov ax, word ptr [0xc378]
cea1: mov word ptr [bp - 0x10], ax
cea4: mov ax, word ptr [0xc372]
cea7: mov word ptr [bp - 0xa], ax
ceaa: mov ax, word ptr [0xc374]
cead: mov word ptr [bp - 0x8], ax
ceb0: mov al, byte ptr [0xc389]
ceb3: cwde
ceb4: mov bx, ax
ceb6: mov al, byte ptr [bx - 0x671e]         ; tbl DS:98E2
ceba: sub ah, ah
cebc: mov bl, byte ptr [0xc37b]
cec0: sub bh, bh
cec2: mov cl, byte ptr [bx - 0x670e]         ; tbl DS:98F2
cec6: sub ch, ch
cec8: add ax, cx
ceca: mov word ptr [bp - 0xc], ax
cecd: mov al, byte ptr [0xc388]
ced0: cwde
ced1: mov bx, ax
ced3: mov al, byte ptr [bx - 0x671e]         ; tbl DS:98E2
ced7: sub ah, ah
ced9: mov bl, byte ptr [0xc37a]
cedd: sub bh, bh
cedf: mov cl, byte ptr [bx - 0x670e]         ; tbl DS:98F2
cee3: add ax, cx
cee5: mov word ptr [bp - 0xe], ax
cee8: mov word ptr [bp - 0x6], 0x0
ceed: cmp word ptr [bp - 0x6], 0x5
cef1: jb 0xcef6
cef3: jmp 0xcf78
cef6: mov bx, word ptr [bp - 0xc]
cef9: mov si, word ptr [0xc3b0]
cefd: mov al, byte ptr [bx + si]
ceff: cwde
cf00: add ax, word ptr [bp - 0x12]
cf03: mov word ptr [bp - 0x18], ax
cf06: mov si, word ptr [0xc3b2]
cf0a: mov al, byte ptr [bx + si]
cf0c: cwde
cf0d: add ax, word ptr [bp - 0x10]
cf10: mov word ptr [bp - 0x16], ax
cf13: mov si, word ptr [0xc3b4]
cf17: mov al, byte ptr [bx + si]
cf19: cwde
cf1a: mov word ptr [bp - 0x14], ax
cf1d: inc word ptr [bp - 0xc]
cf20: mov ax, word ptr [bp - 0xe]
cf23: mov word ptr [bp - 0x2], ax
cf26: mov word ptr [bp - 0x4], 0x0
cf2b: mov bx, word ptr [bp - 0x2]
cf2e: mov si, word ptr [0xc3b0]
cf32: mov al, byte ptr [bx + si]
cf34: cwde
cf35: add ax, word ptr [bp - 0xa]
cf38: mov word ptr [bp - 0x1e], ax
cf3b: mov si, word ptr [0xc3b2]
cf3f: mov al, byte ptr [bx + si]
cf41: cwde
cf42: add ax, word ptr [bp - 0x8]
cf45: mov word ptr [bp - 0x1c], ax
cf48: mov si, word ptr [0xc3b4]
cf4c: mov al, byte ptr [bx + si]
cf4e: cwde
cf4f: mov word ptr [bp - 0x1a], ax
cf52: inc word ptr [bp - 0x2]
cf55: lea ax, [bp - 0x18]
cf58: push ax
cf59: lea ax, [bp - 0x1e]
cf5c: push ax
cf5d: call 0x5ed                             ; EXE circles_touch(c1*, c2*): dist^2 <= (r1 + r2)^2 -> -1/0, collision (G)
cf60: or ax, ax
cf62: jne 0xcf73
cf64: inc word ptr [bp - 0x4]
cf67: cmp word ptr [bp - 0x4], 0x5
cf6b: jb 0xcf2b
cf6d: inc word ptr [bp - 0x6]
cf70: jmp 0xceed
cf73: mov ax, 0xffff
cf76: jmp 0xcf7a
cf78: sub ax, ax
cf7a: pop si
cf7b: mov sp, bp
cf7d: pop bp
cf7e: ret

; ---- func CF7F  size 0x5e  bp frame, locals 2, ret 0x2  callers: B12A
cf7f: push bp
cf80: mov bp, sp
cf82: sub sp, 0x2
cf85: push di
cf86: push si
cf87: mov si, word ptr [bp + 0x4]
cf8a: mov al, 0xe
cf8c: imul byte ptr [si - 0x3c78]            ; tbl DS:C388
cf90: mov cx, si
cf92: shl si
cf94: mov bx, word ptr [si - 0x3c5e]         ; tbl DS:C3A2
cf98: add bx, ax
cf9a: mov si, cx
cf9c: shl si
cf9e: shl si
cfa0: mov ax, word ptr [si - 0x3c8e]         ; tbl DS:C372
cfa4: sub ax, word ptr [bx + 0x2]
cfa7: mov di, bx
cfa9: mov bl, 0x3
cfab: xchg cx, bx
cfad: shl bx, cl
cfaf: mov word ptr [bx - 0x3c10], ax         ; tbl DS:C3F0
cfb3: mov ax, word ptr [di]
cfb5: add ax, word ptr [bx - 0x3c10]         ; tbl DS:C3F0
cfb9: mov word ptr [bx - 0x3c0c], ax         ; tbl DS:C3F4
cfbd: mov ax, word ptr [si - 0x3c8c]         ; tbl DS:C374
cfc1: sub ax, word ptr [di + 0x4]
cfc4: mov word ptr [bx - 0x3c0e], ax         ; tbl DS:C3F2
cfc8: mov al, byte ptr [di + 0x7]
cfcb: sub ah, ah
cfcd: add ax, word ptr [bx - 0x3c0e]         ; tbl DS:C3F2
cfd1: mov word ptr [bx - 0x3c0a], ax         ; tbl DS:C3F6
cfd5: pop si
cfd6: pop di
cfd7: mov sp, bp
cfd9: pop bp
cfda: ret 0x2

; ---- func CFDD  size 0x93  no bp frame, ret  callers: B12A
cfdd: push di
cfde: push si
cfdf: mov ax, word ptr [0xc384]
cfe2: xor al, 0x1
cfe4: mov si, ax
cfe6: mov bx, word ptr [0xc384]
cfea: mov cx, si
cfec: mov al, byte ptr [bx - 0x3c78]         ; tbl DS:C388
cff0: cwde
cff1: mov dx, cx
cff3: mov cl, 0x4
cff5: mov bl, byte ptr [bx - 0x3c86]         ; tbl DS:C37A
cff9: sub bh, bh
cffb: shl bx, cl
cffd: add bx, ax
cfff: mov di, word ptr [0xc3b6]
d003: mov al, byte ptr [bx + di]
d005: cwde
d006: mov di, word ptr [0xc384]
d00a: shl di
d00c: shl di
d00e: add ax, word ptr [di - 0x3c8e]         ; tbl DS:C372
d012: push ax
d013: mov ax, word ptr [di - 0x3c8c]         ; tbl DS:C374
d017: mov di, word ptr [0xc3b8]
d01b: mov cx, ax
d01d: mov al, byte ptr [bx + di]
d01f: cwde
d020: add cx, ax
d022: push cx
d023: push dx
d024: call 0xd070
d027: or ax, ax
d029: jne 0xd06d
d02b: mov al, byte ptr [si - 0x3c78]         ; tbl DS:C388
d02f: cwde
d030: mov cl, 0x4
d032: mov bl, byte ptr [si - 0x3c86]         ; tbl DS:C37A
d036: sub bh, bh
d038: shl bx, cl
d03a: add bx, ax
d03c: mov di, word ptr [0xc3b6]
d040: mov al, byte ptr [bx + di]
d042: cwde
d043: mov di, si
d045: shl di
d047: shl di
d049: add ax, word ptr [di - 0x3c8e]         ; tbl DS:C372
d04d: push ax
d04e: mov ax, word ptr [di - 0x3c8c]         ; tbl DS:C374
d052: mov di, word ptr [0xc3b8]
d056: mov cx, ax
d058: mov al, byte ptr [bx + di]
d05a: cwde
d05b: add cx, ax
d05d: push cx
d05e: push word ptr [0xc384]
d062: call 0xd070
d065: or ax, ax
d067: je 0xd06d
d069: mov word ptr [0xc384], si
d06d: pop si
d06e: pop di
d06f: ret

; ---- func D070  size 0xab  bp frame, locals 12, ret 0x6  callers: BA7A CB11 CFDD
d070: push bp
d071: mov bp, sp
d073: sub sp, 0xc
d076: push si
d077: push word ptr [bp + 0x8]
d07a: push word ptr [bp + 0x6]
d07d: mov cl, 0x3
d07f: mov ax, word ptr [bp + 0x4]
d082: shl ax, cl
d084: add ax, 0xc3f0
d087: push ax
d088: call 0x8238                            ; EXE in_rect(rect, y, x): inclusive, signed
d08b: or ax, ax
d08d: jne 0xd092
d08f: jmp 0xd112
d092: mov bx, word ptr [bp + 0x4]
d095: shl bx
d097: shl bx
d099: mov ax, word ptr [bx - 0x3c8e]         ; tbl DS:C372
d09d: mov word ptr [bp - 0x4], ax
d0a0: mov ax, word ptr [bx - 0x3c8c]         ; tbl DS:C374
d0a4: mov word ptr [bp - 0x6], ax
d0a7: mov bx, word ptr [bp + 0x4]
d0aa: mov al, byte ptr [bx - 0x3c78]         ; tbl DS:C388
d0ae: cwde
d0af: mov si, ax
d0b1: mov si, word ptr [si - 0x671e]         ; tbl DS:98E2
d0b5: and si, 0xff
d0b9: mov bl, byte ptr [bx - 0x3c86]         ; tbl DS:C37A
d0bd: sub bh, bh
d0bf: mov al, byte ptr [bx - 0x670e]         ; tbl DS:98F2
d0c3: sub ah, ah
d0c5: add si, ax
d0c7: mov word ptr [bp - 0x2], 0x0
d0cc: cmp word ptr [bp - 0x2], 0x5
d0d0: jge 0xd112
d0d2: mov bx, word ptr [0xc3b0]
d0d6: mov al, byte ptr [bx + si]
d0d8: cwde
d0d9: add ax, word ptr [bp - 0x4]
d0dc: mov word ptr [bp - 0xc], ax
d0df: mov bx, word ptr [0xc3b2]
d0e3: mov al, byte ptr [bx + si]
d0e5: cwde
d0e6: add ax, word ptr [bp - 0x6]
d0e9: mov word ptr [bp - 0xa], ax
d0ec: mov bx, word ptr [0xc3b4]
d0f0: mov al, byte ptr [bx + si]
d0f2: cwde
d0f3: mov word ptr [bp - 0x8], ax
d0f6: inc si
d0f7: push word ptr [bp + 0x8]
d0fa: push word ptr [bp + 0x6]
d0fd: lea ax, [bp - 0xc]
d100: push ax
d101: call 0x5b0                             ; EXE point_in_circle(circ{x,y,r}*, y, x): dx^2 + dy^2 <= r^2 -> -1/0 (G)
d104: or ax, ax
d106: jne 0xd10d
d108: inc word ptr [bp - 0x2]
d10b: jmp 0xd0cc
d10d: mov ax, 0xffff
d110: jmp 0xd114
d112: sub ax, ax
d114: pop si
d115: mov sp, bp
d117: pop bp
d118: ret 0x6

; ---- func D11B  size 0x126  bp frame, locals 38, ret 0x2  callers: C9A3
d11b: push bp
d11c: mov bp, sp
d11e: sub sp, 0x26
d121: push di
d122: push si
d123: mov di, word ptr [bp + 0x4]
d126: xor di, 0x1
d129: mov al, byte ptr [di - 0x3c32]         ; tbl DS:C3CE
d12d: mov byte ptr [bp - 0x5], al
d130: mov cl, 0xe
d132: imul cl
d134: mov bx, ax
d136: add bx, word ptr [0xc3ae]
d13a: add bx, 0x1c0
d13e: mov si, di
d140: shl si
d142: shl si
d144: mov ax, word ptr [si - 0x3c28]         ; tbl DS:C3D8
d148: sub ax, word ptr [bx + 0x2]
d14b: mov word ptr [bp - 0x1e], ax
d14e: mov ax, word ptr [bx]
d150: add ax, word ptr [bp - 0x1e]
d153: mov word ptr [bp - 0x1a], ax
d156: mov ax, word ptr [si - 0x3c26]         ; tbl DS:C3DA
d15a: sub ax, word ptr [bx + 0x4]
d15d: mov word ptr [bp - 0x1c], ax
d160: mov al, byte ptr [bx + 0x7]
d163: sub ah, ah
d165: add ax, word ptr [bp - 0x1c]
d168: mov word ptr [bp - 0x18], ax
d16b: mov cl, 0x3
d16d: mov ax, word ptr [bp + 0x4]
d170: shl ax, cl
d172: add ax, 0xc3f0
d175: push ax
d176: lea ax, [bp - 0x1e]
d179: push ax
d17a: lea ax, [bp - 0x26]
d17d: push ax
d17e: call 0x817c                            ; EXE sprite clip against C0DA..C0E0
d181: or ax, ax
d183: jne 0xd188
d185: jmp 0xd237
d188: mov al, byte ptr [bp - 0x5]
d18b: cwde
d18c: mov bx, ax
d18e: mov al, byte ptr [bx - 0x670a]         ; tbl DS:98F6
d192: cwde
d193: shl di
d195: shl di
d197: add ax, word ptr [di - 0x3c28]         ; tbl DS:C3D8
d19b: mov word ptr [bp - 0x10], ax
d19e: mov al, byte ptr [bx - 0x66fa]         ; tbl DS:9906
d1a2: cwde
d1a3: add ax, word ptr [di - 0x3c26]         ; tbl DS:C3DA
d1a7: mov word ptr [bp - 0xe], ax
d1aa: mov al, byte ptr [bx - 0x66ea]         ; tbl DS:9916
d1ae: sub ah, ah
d1b0: mov word ptr [bp - 0xc], ax
d1b3: mov bx, word ptr [bp + 0x4]
d1b6: shl bx
d1b8: shl bx
d1ba: mov ax, word ptr [bx - 0x3c8e]         ; tbl DS:C372
d1be: mov word ptr [bp - 0x8], ax
d1c1: mov ax, word ptr [bx - 0x3c8c]         ; tbl DS:C374
d1c5: mov word ptr [bp - 0xa], ax
d1c8: mov bx, word ptr [bp + 0x4]
d1cb: mov al, byte ptr [bx - 0x3c78]         ; tbl DS:C388
d1cf: cwde
d1d0: mov si, ax
d1d2: mov al, byte ptr [si - 0x671e]         ; tbl DS:98E2
d1d6: sub ah, ah
d1d8: mov bl, byte ptr [bx - 0x3c86]         ; tbl DS:C37A
d1dc: sub bh, bh
d1de: mov cl, byte ptr [bx - 0x670e]         ; tbl DS:98F2
d1e2: sub ch, ch
d1e4: add ax, cx
d1e6: mov word ptr [bp - 0x2], ax
d1e9: mov word ptr [bp - 0x4], 0x0
d1ee: cmp word ptr [bp - 0x4], 0x5
d1f2: jae 0xd237
d1f4: mov bx, word ptr [bp - 0x2]
d1f7: mov si, word ptr [0xc3b0]
d1fb: mov al, byte ptr [bx + si]
d1fd: cwde
d1fe: add ax, word ptr [bp - 0x8]
d201: mov word ptr [bp - 0x16], ax
d204: mov si, word ptr [0xc3b2]
d208: mov al, byte ptr [bx + si]
d20a: cwde
d20b: add ax, word ptr [bp - 0xa]
d20e: mov word ptr [bp - 0x14], ax
d211: mov si, word ptr [0xc3b4]
d215: mov al, byte ptr [bx + si]
d217: cwde
d218: mov word ptr [bp - 0x12], ax
d21b: inc word ptr [bp - 0x2]
d21e: lea ax, [bp - 0x10]
d221: push ax
d222: lea ax, [bp - 0x16]
d225: push ax
d226: call 0x5ed                             ; EXE circles_touch(c1*, c2*): dist^2 <= (r1 + r2)^2 -> -1/0, collision (G)
d229: or ax, ax
d22b: jne 0xd232
d22d: inc word ptr [bp - 0x4]
d230: jmp 0xd1ee
d232: mov ax, 0xffff
d235: jmp 0xd239
d237: sub ax, ax
d239: pop si
d23a: pop di
d23b: mov sp, bp
d23d: pop bp
d23e: ret 0x2
d241: db 0x00                                ; word-alignment pad

; ---- func D242  size 0xfb  bp frame, locals 2, ret  callers: B12A
d242: push bp
d243: mov bp, sp
d245: sub sp, 0x2
d248: push di
d249: push si
d24a: mov ax, 0x1
d24d: push ax
d24e: call 0xd38d
d251: mov di, ax
d253: sub ax, ax
d255: mov word ptr [bp - 0x2], ax
d258: push ax
d259: call 0xd38d
d25c: mov si, ax
d25e: or di, di
d260: je 0xd295
d262: or si, si
d264: je 0xd295
d266: push di
d267: push si
d268: call 0x54a                             ; EXE same_sign(a, b): ~(a ^ b) & 0x8000 (G)
d26b: or ax, ax
d26d: je 0xd27b
d26f: cmp si, di
d271: jge 0xd277
d273: sub si, si
d275: jmp 0xd295
d277: sub di, di
d279: jmp 0xd295
d27b: mov ax, 0x1
d27e: push ax
d27f: push ax
d280: push di
d281: call 0xd33d
d284: mov word ptr [bp - 0x2], ax
d287: sub ax, ax
d289: push ax
d28a: mov ax, 0x1
d28d: push ax
d28e: push si
d28f: call 0xd33d
d292: or word ptr [bp - 0x2], ax
d295: add word ptr [0xc378], di
d299: mov cl, 0x8
d29b: shl di, cl
d29d: add word ptr [0xc39a], di
d2a1: add word ptr [0xc374], si
d2a5: shl si, cl
d2a7: add word ptr [0xc39e], si
d2ab: mov ax, si
d2ad: add ax, di
d2af: add word ptr [0xc3e6], ax
d2b3: add word ptr [0xc3e2], ax
d2b7: add word ptr [0xc3ee], ax
d2bb: add word ptr [0xc3ea], ax
d2bf: mov ax, 0x1
d2c2: push ax
d2c3: call 0xd3bf
d2c6: mov di, ax
d2c8: sub ax, ax
d2ca: push ax
d2cb: call 0xd3bf
d2ce: mov si, ax
d2d0: or di, di
d2d2: je 0xd306
d2d4: or si, si
d2d6: je 0xd306
d2d8: push di
d2d9: push si
d2da: call 0x54a                             ; EXE same_sign(a, b): ~(a ^ b) & 0x8000 (G)
d2dd: or ax, ax
d2df: je 0xd2ed
d2e1: cmp si, di
d2e3: jge 0xd2e9
d2e5: sub si, si
d2e7: jmp 0xd306
d2e9: sub di, di
d2eb: jmp 0xd306
d2ed: mov ax, 0x1
d2f0: push ax
d2f1: sub ax, ax
d2f3: push ax
d2f4: push di
d2f5: call 0xd33d
d2f8: or word ptr [bp - 0x2], ax
d2fb: sub ax, ax
d2fd: push ax
d2fe: push ax
d2ff: push si
d300: call 0xd33d
d303: or word ptr [bp - 0x2], ax
d306: add word ptr [0xc376], di
d30a: mov ax, di
d30c: mov ah, al
d30e: sub al, al
d310: add word ptr [0xc398], ax
d314: add word ptr [0xc372], si
d318: mov cx, si
d31a: mov ch, cl
d31c: sub cl, cl
d31e: add word ptr [0xc39c], cx
d322: add ax, cx
d324: add word ptr [0xc3e4], ax
d328: add word ptr [0xc3e0], ax
d32c: add word ptr [0xc3ec], ax
d330: add word ptr [0xc3e8], ax
d334: mov ax, word ptr [bp - 0x2]
d337: pop si
d338: pop di
d339: mov sp, bp
d33b: pop bp
d33c: ret

; ---- func D33D  size 0x50  bp frame, locals 2, ret 0x6  callers: D242
d33d: push bp
d33e: mov bp, sp
d340: sub sp, 0x2
d343: push si
d344: mov si, word ptr [bp + 0x8]
d347: cmp word ptr [bp + 0x4], 0x0
d34b: jge 0xd356
d34d: mov bx, word ptr [bp + 0x6]
d350: mov al, byte ptr [bx - 0x66d8]         ; tbl DS:9928
d354: jmp 0xd35d
d356: mov bx, word ptr [bp + 0x6]
d359: mov al, byte ptr [bx - 0x66d6]         ; tbl DS:992A
d35d: cwde
d35e: mov cx, ax
d360: mov al, byte ptr [si - 0x3c78]         ; tbl DS:C388
d364: cwde
d365: mov dx, ax
d367: mov ax, cx
d369: sub ax, dx
d36b: cdq
d36c: xor ax, dx
d36e: sub ax, dx
d370: mov cx, ax
d372: cmp cx, 0x4
d375: jl 0xd380
d377: cmp cx, 0xc
d37a: jg 0xd380
d37c: sub ax, ax
d37e: jmp 0xd386
d380: mov al, byte ptr [si - 0x66da]         ; tbl DS:9926
d384: sub ah, ah
d386: pop si
d387: mov sp, bp
d389: pop bp
d38a: ret 0x6

; ---- func D38D  size 0x32  bp frame, locals 2, ret 0x2  callers: D242
d38d: push bp
d38e: mov bp, sp
d390: sub sp, 0x2
d393: sub dx, dx
d395: mov bx, word ptr [bp + 0x4]
d398: shl bx
d39a: shl bx
d39c: mov ax, word ptr [bx - 0x3c8c]         ; tbl DS:C374
d3a0: mov word ptr [bp - 0x2], ax
d3a3: cmp ax, 0xf
d3a6: jge 0xd3ad
d3a8: mov dx, 0xf
d3ab: jmp 0xd3b5
d3ad: cmp ax, 0xaa
d3b0: jle 0xd3b7
d3b2: mov dx, 0xaa
d3b5: sub dx, ax
d3b7: mov ax, dx
d3b9: mov sp, bp
d3bb: pop bp
d3bc: ret 0x2

; ---- func D3BF  size 0x32  bp frame, locals 2, ret 0x2  callers: D242
d3bf: push bp
d3c0: mov bp, sp
d3c2: sub sp, 0x2
d3c5: sub dx, dx
d3c7: mov bx, word ptr [bp + 0x4]
d3ca: shl bx
d3cc: shl bx
d3ce: mov ax, word ptr [bx - 0x3c8e]         ; tbl DS:C372
d3d2: mov word ptr [bp - 0x2], ax
d3d5: cmp ax, 0xf
d3d8: jge 0xd3df
d3da: mov dx, 0xf
d3dd: jmp 0xd3e7
d3df: cmp ax, 0x130
d3e2: jle 0xd3e9
d3e4: mov dx, 0x130
d3e7: sub dx, ax
d3e9: mov ax, dx
d3eb: mov sp, bp
d3ed: pop bp
d3ee: ret 0x2
d3f1: db 0x00                                ; word-alignment pad

; ---- func D3F2  size 0x21a  bp frame, locals 62, ret 0x2  callers: B12A
d3f2: push bp
d3f3: mov bp, sp
d3f5: sub sp, 0x3e
d3f8: push di
d3f9: push si
d3fa: mov dx, word ptr [bp + 0x4]
d3fd: mov word ptr [bp - 0xe], 0x0
d402: or dx, dx
d404: jne 0xd409
d406: jmp 0xd604
d409: mov word ptr [0x9afa], 0x9
d40f: cmp dx, 0x3
d412: jne 0xd429
d414: call 0x67e4                            ; EXE draw into the work buffer
d417: sub ax, ax
d419: push ax
d41a: push ax
d41b: mov ax, 0x13f
d41e: push ax
d41f: mov ax, 0xb9
d422: push ax
d423: call 0x7e74                            ; EXE fill rect (colour, ...)
d426: jmp 0xd604
d429: test dl, 0x1
d42c: je 0xd450
d42e: mov word ptr [bp - 0x8], 0x1
d433: mov ax, 0x1
d436: push ax
d437: lea ax, [bp - 0x1e]
d43a: push ax
d43b: call 0xd919
d43e: lea di, [bp - 0x16]
d441: mov si, ax
d443: push ss
d444: pop es
d445: movsw word ptr es:[di], word ptr [si]
d446: movsw word ptr es:[di], word ptr [si]
d447: movsw word ptr es:[di], word ptr [si]
d448: movsw word ptr es:[di], word ptr [si]
d449: mov word ptr [bp - 0x6], 0x0
d44e: jmp 0xd46f
d450: mov word ptr [bp - 0x8], 0x0
d455: sub ax, ax
d457: push ax
d458: lea ax, [bp - 0x26]
d45b: push ax
d45c: call 0xd919
d45f: lea di, [bp - 0x16]
d462: mov si, ax
d464: push ss
d465: pop es
d466: movsw word ptr es:[di], word ptr [si]
d467: movsw word ptr es:[di], word ptr [si]
d468: movsw word ptr es:[di], word ptr [si]
d469: movsw word ptr es:[di], word ptr [si]
d46a: mov word ptr [bp - 0x6], 0x1
d46f: mov bx, word ptr [bp - 0x6]
d472: mov byte ptr [bx - 0x3c70], 0x6        ; tbl DS:C390
d477: mov si, word ptr [bp - 0x8]
d47a: mov byte ptr [si - 0x3c70], 0x3        ; tbl DS:C390
d47f: push bx
d480: lea ax, [bp - 0x36]
d483: push ax
d484: call 0xd97c
d487: push si
d488: lea di, [bp - 0x2e]
d48b: mov si, ax
d48d: push ss
d48e: pop es
d48f: movsw word ptr es:[di], word ptr [si]
d490: movsw word ptr es:[di], word ptr [si]
d491: movsw word ptr es:[di], word ptr [si]
d492: movsw word ptr es:[di], word ptr [si]
d493: pop si
d494: mov byte ptr [bp - 0x1], 0x1
d498: cmp byte ptr [si - 0x3c78], 0x4        ; tbl DS:C388
d49d: jle 0xd4ad
d49f: mov bx, word ptr [bp - 0x8]
d4a2: cmp byte ptr [bx - 0x3c78], 0xc        ; tbl DS:C388
d4a7: jge 0xd4ad
d4a9: mov byte ptr [bp - 0x1], -0x1
d4ad: mov bx, word ptr [bp - 0x8]
d4b0: shl bx
d4b2: shl bx
d4b4: mov ax, word ptr [bx - 0x3c8e]         ; tbl DS:C372
d4b8: mov word ptr [bp - 0xc], ax
d4bb: mov ax, word ptr [bx - 0x3c8c]         ; tbl DS:C374
d4bf: mov word ptr [bp - 0xa], ax
d4c2: lea ax, [bp - 0x2e]
d4c5: push ax
d4c6: lea ax, [bp - 0x16]
d4c9: push ax
d4ca: sub ax, ax
d4cc: push ax
d4cd: call 0x817c                            ; EXE sprite clip against C0DA..C0E0
d4d0: or ax, ax
d4d2: je 0xd4fe
d4d4: mov bx, word ptr [bp - 0x6]
d4d7: shl bx
d4d9: shl bx
d4db: mov ax, word ptr [bx - 0x3c8e]         ; tbl DS:C372
d4df: add word ptr [bp - 0xc], ax
d4e2: mov cx, 0x2
d4e5: mov ax, word ptr [bp - 0xc]
d4e8: cdq
d4e9: idiv cx
d4eb: mov word ptr [bp - 0xc], ax
d4ee: mov ax, word ptr [bx - 0x3c8c]         ; tbl DS:C374
d4f2: add word ptr [bp - 0xa], ax
d4f5: mov ax, word ptr [bp - 0xa]
d4f8: cdq
d4f9: idiv cx
d4fb: mov word ptr [bp - 0xa], ax
d4fe: cmp word ptr [bp - 0xe], 0x0
d502: je 0xd507
d504: jmp 0xd5de
d507: call 0xf8                              ; EXE timer0_reset (= 00D7(0))
d50a: sub ax, ax
d50c: push ax
d50d: push ax
d50e: mov cx, 0x13f
d511: push cx
d512: mov cx, 0xb9
d515: push cx
d516: call 0x7e74                            ; EXE fill rect (colour, ...)
d519: lea ax, [bp - 0x2e]
d51c: push ax
d51d: lea ax, [bp - 0x16]
d520: push ax
d521: sub ax, ax
d523: push ax
d524: call 0x817c                            ; EXE sprite clip against C0DA..C0E0
d527: mov word ptr [bp - 0x4], ax
d52a: or ax, ax
d52c: je 0xd583
d52e: mov bx, word ptr [bp - 0x6]
d531: mov al, byte ptr [bx - 0x3c78]         ; tbl DS:C388
d535: cwde
d536: mov si, ax
d538: mov al, byte ptr [si - 0x66a8]         ; tbl DS:9958
d53c: cwde
d53d: shl bx
d53f: shl bx
d541: add word ptr [bx - 0x3c8e], ax         ; tbl DS:C372
d545: mov al, byte ptr [si - 0x6698]         ; tbl DS:9968
d549: cwde
d54a: add word ptr [bx - 0x3c8c], ax         ; tbl DS:C374
d54e: mov bx, word ptr [bp - 0x6]
d551: dec byte ptr [bx - 0x3c70]             ; tbl DS:C390
d555: jns 0xd55f
d557: mov bx, word ptr [bp - 0x6]
d55a: mov byte ptr [bx - 0x3c70], 0x6        ; tbl DS:C390
d55f: mov bx, word ptr [bp - 0x6]
d562: cmp byte ptr [bx - 0x3c70], 0x5        ; tbl DS:C390
d567: jne 0xd56e
d569: mov byte ptr [bx - 0x3c70], 0x3        ; tbl DS:C390
d56e: push word ptr [bp - 0x6]
d571: lea ax, [bp - 0x3e]
d574: push ax
d575: call 0xd97c
d578: lea di, [bp - 0x2e]
d57b: mov si, ax
d57d: push ss
d57e: pop es
d57f: movsw word ptr es:[di], word ptr [si]
d580: movsw word ptr es:[di], word ptr [si]
d581: movsw word ptr es:[di], word ptr [si]
d582: movsw word ptr es:[di], word ptr [si]
d583: mov bx, word ptr [bp - 0x8]
d586: cmp byte ptr [bx - 0x3c78], 0x4        ; tbl DS:C388
d58b: jne 0xd59a
d58d: cmp word ptr [bp - 0x4], 0x0
d591: jne 0xd5a6
d593: mov word ptr [bp - 0xe], 0xffff
d598: jmp 0xd5a6
d59a: mov al, byte ptr [bp - 0x1]
d59d: add byte ptr [bx - 0x3c78], al         ; tbl DS:C388
d5a1: and byte ptr [bx - 0x3c78], 0xf        ; tbl DS:C388
d5a6: push word ptr [bp - 0x6]
d5a9: call 0xb8e3
d5ac: push word ptr [bp - 0x8]
d5af: call 0xb8e3
d5b2: mov ax, 0x3
d5b5: push ax
d5b6: call 0x76b                             ; EXE rng_roll(n)
d5b9: mov si, ax
d5bb: mov cx, 0xe
d5be: mul cx
d5c0: add ax, word ptr [0xc3ac]
d5c4: push ax
d5c5: push word ptr [bp - 0xc]
d5c8: push word ptr [bp - 0xa]
d5cb: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
d5ce: call 0x6887                            ; EXE flip: work buffer -> screen
d5d1: call 0x1ae                             ; EXE ticks_since(0)
d5d4: cmp ax, 0xe
d5d7: jb 0xd5dc
d5d9: jmp 0xd4fe
d5dc: jmp 0xd5d1
d5de: mov bx, word ptr [bp - 0x8]
d5e1: mov byte ptr [bx - 0x3c70], 0x6        ; tbl DS:C390
d5e6: call 0x67b6                            ; EXE screen restore 6973(0,0,319,199) unless [9B26]==2 path (G)
d5e9: call 0x6837                            ; EXE draw on the screen (6887 first if needed)
d5ec: call 0x67e4                            ; EXE draw into the work buffer
d5ef: sub ax, ax
d5f1: push ax
d5f2: push ax
d5f3: mov ax, 0x13f
d5f6: push ax
d5f7: mov ax, 0xb9
d5fa: push ax
d5fb: call 0x7e74                            ; EXE fill rect (colour, ...)
d5fe: push word ptr [bp - 0x6]
d601: call 0xb8e3
d604: pop si
d605: pop di
d606: mov sp, bp
d608: pop bp
d609: ret 0x2

; ---- func D60C  size 0x28f  bp frame, locals 52, ret 0x2  callers: B12A
d60c: push bp
d60d: mov bp, sp
d60f: sub sp, 0x34
d612: push di
d613: push si
d614: sub ax, ax
d616: mov word ptr [bp - 0x8], ax
d619: mov word ptr [bp - 0x6], ax
d61c: mov word ptr [bp - 0x2], ax
d61f: mov word ptr [bp - 0x4], ax
d622: mov word ptr [bp - 0xc], ax
d625: mov word ptr [bp - 0xa], ax
d628: mov word ptr [bp - 0xe], ax
d62b: cmp word ptr [bp + 0x4], ax
d62e: je 0xd63f
d630: mov ax, 0x8
d633: push ax
d634: lea ax, [bp - 0x14]
d637: push ax
d638: call 0xf7b                             ; EXE archive open (0F7B(n, &arch))
d63b: or ax, ax
d63d: jne 0xd642
d63f: jmp 0xd893
d642: mov ax, 0x1
d645: push ax
d646: lea ax, [bp - 0x2c]
d649: push ax
d64a: call 0xd919
d64d: lea di, [bp - 0x1c]
d650: mov si, ax
d652: push ss
d653: pop es
d654: movsw word ptr es:[di], word ptr [si]
d655: movsw word ptr es:[di], word ptr [si]
d656: movsw word ptr es:[di], word ptr [si]
d657: movsw word ptr es:[di], word ptr [si]
d658: sub ax, ax
d65a: push ax
d65b: lea ax, [bp - 0x34]
d65e: push ax
d65f: call 0xd919
d662: lea di, [bp - 0x24]
d665: mov si, ax
d667: push ss
d668: pop es
d669: movsw word ptr es:[di], word ptr [si]
d66a: movsw word ptr es:[di], word ptr [si]
d66b: movsw word ptr es:[di], word ptr [si]
d66c: movsw word ptr es:[di], word ptr [si]
d66d: test byte ptr [bp + 0x4], 0x1
d671: je 0xd6c8
d673: mov bl, byte ptr [0xc37b]
d677: sub bh, bh
d679: mov al, byte ptr [bx - 0x66ae]         ; tbl DS:9952
d67d: mov bx, word ptr [0x9950]
d681: mov byte ptr [bx], al
d683: push word ptr [0x9950]
d687: lea ax, [bp - 0x14]
d68a: push ax
d68b: sub ax, ax
d68d: push ax
d68e: call 0x163b                            ; EXE sheet_163B: load xxxHDR/xxxDAT sprite sheet
d691: mov word ptr [bp - 0x8], ax
d694: or ax, ax
d696: jne 0xd6a2
d698: mov ax, 0x9978                         ; "Failed to Load Human Sinking"
d69b: push ax
d69c: call 0x59d4                            ; EXE message_box(str)
d69f: jmp 0xd893
d6a2: mov word ptr [bp - 0xc], 0xffff
d6a7: mov ax, word ptr [bp - 0x1c]
d6aa: mov word ptr [0xaac8], ax
d6ad: mov ax, word ptr [bp - 0x16]
d6b0: add ax, 0x4
d6b3: mov word ptr [0xaacc], ax
d6b6: cmp ax, 0xb9
d6b9: jl 0xd6c3
d6bb: mov ax, word ptr [bp - 0x1a]
d6be: dec ax
d6bf: dec ax
d6c0: mov word ptr [0xaacc], ax
d6c3: mov byte ptr [0xaac6], 0x0
d6c8: test byte ptr [bp + 0x4], 0x2
d6cc: je 0xd735
d6ce: cmp word ptr [bp - 0x8], 0x0
d6d2: je 0xd6e5
d6d4: mov al, byte ptr [0xc37a]
d6d7: cmp byte ptr [0xc37b], al
d6db: jne 0xd6e5
d6dd: mov ax, word ptr [bp - 0x8]
d6e0: mov word ptr [bp - 0x6], ax
d6e3: jmp 0xd735
d6e5: mov bl, byte ptr [0xc37a]
d6e9: sub bh, bh
d6eb: mov al, byte ptr [bx - 0x66ae]         ; tbl DS:9952
d6ef: mov bx, word ptr [0x9950]
d6f3: mov byte ptr [bx], al
d6f5: push word ptr [0x9950]
d6f9: lea ax, [bp - 0x14]
d6fc: push ax
d6fd: sub ax, ax
d6ff: push ax
d700: call 0x163b                            ; EXE sheet_163B: load xxxHDR/xxxDAT sprite sheet
d703: mov word ptr [bp - 0x6], ax
d706: or ax, ax
d708: jne 0xd70f
d70a: mov ax, 0x9995                         ; "Failed to Load Computer Sinking"
d70d: jmp 0xd69b
d70f: mov word ptr [bp - 0xa], 0xffff
d714: mov ax, word ptr [bp - 0x24]
d717: mov word ptr [0xaaca], ax
d71a: mov ax, word ptr [bp - 0x1e]
d71d: add ax, 0x4
d720: mov word ptr [0xaace], ax
d723: cmp ax, 0xb9
d726: jl 0xd730
d728: mov ax, word ptr [bp - 0x22]
d72b: dec ax
d72c: dec ax
d72d: mov word ptr [0xaace], ax
d730: mov byte ptr [0xaac7], 0x1
d735: mov ax, 0x99b5                         ; "shark"
d738: push ax
d739: lea ax, [bp - 0x14]
d73c: push ax
d73d: sub cx, cx
d73f: push cx
d740: call 0x163b                            ; EXE sheet_163B: load xxxHDR/xxxDAT sprite sheet
d743: mov word ptr [0x9956], ax
d746: lea ax, [bp - 0x14]
d749: push ax
d74a: call 0x11a9                            ; EXE archive close
d74d: call 0x67e4                            ; EXE draw into the work buffer
d750: cmp word ptr [bp - 0x8], 0x0
d754: je 0xd764
d756: push word ptr [bp - 0x8]
d759: push word ptr [0xc376]
d75d: push word ptr [0xc378]
d761: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
d764: cmp word ptr [bp - 0x6], 0x0
d768: je 0xd778
d76a: push word ptr [bp - 0x6]
d76d: push word ptr [0xc372]
d771: push word ptr [0xc374]
d775: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
d778: call 0x680f                            ; EXE target = the screen descriptor
d77b: sub ax, ax
d77d: push ax
d77e: push ax
d77f: mov ax, 0x13f
d782: push ax
d783: mov ax, 0xc7
d786: push ax
d787: call 0x6973                            ; EXE restore rect work buffer -> screen
d78a: call 0x67e4                            ; EXE draw into the work buffer
d78d: mov word ptr [0x9afa], 0x9
d793: mov si, word ptr [bp - 0xe]
d796: cmp word ptr [bp - 0x2], 0x0
d79a: je 0xd7a5
d79c: cmp word ptr [bp - 0x4], 0x0
d7a0: je 0xd7a5
d7a2: jmp 0xd86d
d7a5: call 0xf8                              ; EXE timer0_reset (= 00D7(0))
d7a8: cmp word ptr [bp - 0x8], 0x0
d7ac: je 0xd7fa
d7ae: push word ptr [0xaac8]
d7b2: push word ptr [0xaacc]
d7b6: lea ax, [bp - 0x24]
d7b9: push ax
d7ba: call 0x8238                            ; EXE in_rect(rect, y, x): inclusive, signed
d7bd: or ax, ax
d7bf: jne 0xd7c5
d7c1: push ax
d7c2: call 0xd89b
d7c5: mov bl, byte ptr [0xc37b]
d7c9: sub bh, bh
d7cb: shl bx
d7cd: cmp word ptr [bx - 0x66b6], si         ; tbl DS:994A
d7d1: jle 0xd7fa
d7d3: push word ptr [bp - 0x1c]
d7d6: push word ptr [bp - 0x1a]
d7d9: push word ptr [bp - 0x18]
d7dc: push word ptr [bp - 0x16]
d7df: call 0x7e74                            ; EXE fill rect (colour, ...)
d7e2: mov ax, si
d7e4: mov cx, 0xe
d7e7: imul cx
d7e9: add ax, word ptr [bp - 0x8]
d7ec: push ax
d7ed: push word ptr [0xc376]
d7f1: push word ptr [0xc378]
d7f5: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
d7f8: jmp 0xd7ff
d7fa: mov word ptr [bp - 0x2], 0xffff
d7ff: cmp word ptr [bp - 0x6], 0x0
d803: je 0xd854
d805: push word ptr [0xaaca]
d809: push word ptr [0xaace]
d80d: lea ax, [bp - 0x1c]
d810: push ax
d811: call 0x8238                            ; EXE in_rect(rect, y, x): inclusive, signed
d814: or ax, ax
d816: jne 0xd81f
d818: mov ax, 0x1
d81b: push ax
d81c: call 0xd89b
d81f: mov bl, byte ptr [0xc37a]
d823: sub bh, bh
d825: shl bx
d827: cmp word ptr [bx - 0x66b6], si         ; tbl DS:994A
d82b: jle 0xd854
d82d: push word ptr [bp - 0x24]
d830: push word ptr [bp - 0x22]
d833: push word ptr [bp - 0x20]
d836: push word ptr [bp - 0x1e]
d839: call 0x7e74                            ; EXE fill rect (colour, ...)
d83c: mov ax, si
d83e: mov cx, 0xe
d841: imul cx
d843: add ax, word ptr [bp - 0x6]
d846: push ax
d847: push word ptr [0xc372]
d84b: push word ptr [0xc374]
d84f: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
d852: jmp 0xd859
d854: mov word ptr [bp - 0x4], 0xffff
d859: call 0x6887                            ; EXE flip: work buffer -> screen
d85c: inc si
d85d: mov word ptr [bp - 0xe], si
d860: call 0x1ae                             ; EXE ticks_since(0)
d863: cmp ax, 0xe
d866: jb 0xd86b
d868: jmp 0xd796
d86b: jmp 0xd860
d86d: cmp word ptr [bp - 0xc], 0x0
d871: je 0xd879
d873: push word ptr [bp - 0x8]
d876: call 0x19a0                            ; EXE sheet_19A0: free sprite sheet
d879: cmp word ptr [bp - 0xa], 0x0
d87d: je 0xd885
d87f: push word ptr [bp - 0x6]
d882: call 0x19a0                            ; EXE sheet_19A0: free sprite sheet
d885: cmp word ptr [0x9956], 0x0
d88a: je 0xd893
d88c: push word ptr [0x9956]
d890: call 0x19a0                            ; EXE sheet_19A0: free sprite sheet
d893: pop si
d894: pop di
d895: mov sp, bp
d897: pop bp
d898: ret 0x2

; ---- func D89B  size 0x7e  bp frame, locals 0, ret 0x2  callers: D60C
d89b: push bp
d89c: mov bp, sp
d89e: push di
d89f: push si
d8a0: cmp word ptr [0x9956], 0x0
d8a5: je 0xd911
d8a7: mov si, word ptr [bp + 0x4]
d8aa: mov bx, si
d8ac: shl bx
d8ae: mov ax, word ptr [bx - 0x5538]         ; tbl DS:AAC8
d8b2: mov cx, ax
d8b4: sub ax, 0x5
d8b7: push ax
d8b8: mov ax, word ptr [bx - 0x5534]         ; tbl DS:AACC
d8bc: mov dx, ax
d8be: sub ax, 0x3
d8c1: push ax
d8c2: add cx, 0x4
d8c5: push cx
d8c6: inc dx
d8c7: inc dx
d8c8: push dx
d8c9: mov di, bx
d8cb: call 0x7e74                            ; EXE fill rect (colour, ...)
d8ce: mov al, 0xe
d8d0: mul byte ptr [si - 0x553a]             ; tbl DS:AAC6
d8d4: add ax, word ptr [0x9956]
d8d8: push ax
d8d9: inc word ptr [di - 0x5538]             ; tbl DS:AAC8
d8dd: push word ptr [di - 0x5538]            ; tbl DS:AAC8
d8e1: push word ptr [di - 0x5534]            ; tbl DS:AACC
d8e5: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
d8e8: inc byte ptr [si - 0x553a]             ; tbl DS:AAC6
d8ec: cmp byte ptr [si - 0x553a], 0x5        ; tbl DS:AAC6
d8f1: jne 0xd905
d8f3: mov ax, 0xa
d8f6: push ax
d8f7: call 0x76b                             ; EXE rng_roll(n)
d8fa: or ax, ax
d8fc: je 0xd911
d8fe: mov byte ptr [si - 0x553a], 0x2        ; tbl DS:AAC6
d903: jmp 0xd911
d905: cmp byte ptr [si - 0x553a], 0x8        ; tbl DS:AAC6
d90a: jne 0xd911
d90c: mov byte ptr [si - 0x553a], 0x0        ; tbl DS:AAC6
d911: pop si
d912: pop di
d913: mov sp, bp
d915: pop bp
d916: ret 0x2

; ---- func D919  size 0x63  bp frame, locals 8, ret 0x4  callers: D3F2 D60C
d919: push bp
d91a: mov bp, sp
d91c: sub sp, 0x8
d91f: push di
d920: push si
d921: mov cl, 0x3
d923: mov bx, word ptr [bp + 0x6]
d926: mov bl, byte ptr [bx - 0x3c86]         ; tbl DS:C37A
d92a: sub bh, bh
d92c: shl bx, cl
d92e: lea di, [bp - 0x8]
d931: lea si, [bx - 0x66ce]                  ; tbl DS:9932
d935: push ss
d936: pop es
d937: movsw word ptr es:[di], word ptr [si]
d938: movsw word ptr es:[di], word ptr [si]
d939: movsw word ptr es:[di], word ptr [si]
d93a: movsw word ptr es:[di], word ptr [si]
d93b: mov bx, word ptr [bp + 0x6]
d93e: shl bx
d940: shl bx
d942: mov ax, word ptr [bx - 0x3c8e]         ; tbl DS:C372
d946: add word ptr [bp - 0x8], ax
d949: mov cx, word ptr [bx - 0x3c8c]         ; tbl DS:C374
d94d: add word ptr [bp - 0x6], cx
d950: add word ptr [bp - 0x4], ax
d953: add word ptr [bp - 0x2], cx
d956: mov ax, word ptr [bp - 0x2]
d959: cmp ax, 0xb9
d95c: jle 0xd961
d95e: mov ax, 0xb9
d961: mov word ptr [bp - 0x2], ax
d964: mov bx, word ptr [bp + 0x4]
d967: mov di, bx
d969: lea si, [bp - 0x8]
d96c: movsw word ptr es:[di], word ptr [si]
d96d: movsw word ptr es:[di], word ptr [si]
d96e: movsw word ptr es:[di], word ptr [si]
d96f: movsw word ptr es:[di], word ptr [si]
d970: mov ax, bx
d972: mov dx, ss
d974: pop si
d975: pop di
d976: mov sp, bp
d978: pop bp
d979: ret 0x4

; ---- func D97C  size 0xd3  bp frame, locals 26, ret 0x4  callers: D3F2
d97c: push bp
d97d: mov bp, sp
d97f: sub sp, 0x1a
d982: push di
d983: push si
d984: mov si, word ptr [bp + 0x6]
d987: mov al, 0xe
d989: imul byte ptr [si - 0x3c78]            ; tbl DS:C388
d98d: mov bx, si
d98f: mov bx, word ptr [bx + si - 0x3c5e]    ; tbl DS:C3A2
d993: mov word ptr [bp - 0x2], bx
d996: add bx, ax
d998: mov di, si
d99a: shl di
d99c: shl di
d99e: mov ax, word ptr [di - 0x3c8e]         ; tbl DS:C372
d9a2: sub ax, word ptr [bx + 0x2]
d9a5: mov word ptr [bp - 0xa], ax
d9a8: mov ax, word ptr [bx]
d9aa: add ax, word ptr [bp - 0xa]
d9ad: mov word ptr [bp - 0x6], ax
d9b0: mov ax, word ptr [di - 0x3c8c]         ; tbl DS:C374
d9b4: sub ax, word ptr [bx + 0x4]
d9b7: mov word ptr [bp - 0x8], ax
d9ba: mov al, byte ptr [bx + 0x7]
d9bd: sub ah, ah
d9bf: add ax, word ptr [bp - 0x8]
d9c2: mov word ptr [bp - 0x4], ax
d9c5: cmp byte ptr [si - 0x3c78], 0x8        ; tbl DS:C388
d9ca: jge 0xd9e4
d9cc: mov al, byte ptr [si - 0x3c78]         ; tbl DS:C388
d9d0: cwde
d9d1: mov bx, ax
d9d3: mov al, 0xe
d9d5: imul byte ptr [bx - 0x6c1a]            ; tbl DS:93E6
d9d9: mov di, ax
d9db: add di, word ptr [0xc3a6]
d9df: add di, 0x54
d9e2: jmp 0xd9fb
d9e4: mov al, byte ptr [si - 0x3c78]         ; tbl DS:C388
d9e8: cwde
d9e9: mov bx, ax
d9eb: mov al, 0xe
d9ed: imul byte ptr [bx - 0x6c1a]            ; tbl DS:93E6
d9f1: mov di, ax
d9f3: add di, word ptr [0xc3a8]
d9f7: sub di, 0x2bc
d9fb: shl si
d9fd: shl si
d9ff: mov ax, word ptr [si - 0x3c8e]         ; tbl DS:C372
da03: sub ax, word ptr [di + 0x2]
da06: mov word ptr [bp - 0x12], ax
da09: mov ax, word ptr [di]
da0b: add ax, word ptr [bp - 0x12]
da0e: mov word ptr [bp - 0xe], ax
da11: mov ax, word ptr [si - 0x3c8c]         ; tbl DS:C374
da15: sub ax, word ptr [di + 0x4]
da18: mov word ptr [bp - 0x10], ax
da1b: mov al, byte ptr [di + 0x7]
da1e: sub ah, ah
da20: add ax, word ptr [bp - 0x10]
da23: mov word ptr [bp - 0xc], ax
da26: lea ax, [bp - 0xa]
da29: push ax
da2a: lea ax, [bp - 0x12]
da2d: push ax
da2e: lea ax, [bp - 0x1a]
da31: push ax
da32: call 0x81ea                            ; EXE rect min/max combine (rect union/intersect) (G)
da35: mov bx, word ptr [bp + 0x4]
da38: mov di, bx
da3a: lea si, [bp - 0x1a]
da3d: push ss
da3e: pop es
da3f: movsw word ptr es:[di], word ptr [si]
da40: movsw word ptr es:[di], word ptr [si]
da41: movsw word ptr es:[di], word ptr [si]
da42: movsw word ptr es:[di], word ptr [si]
da43: mov ax, bx
da45: mov dx, ss
da47: pop si
da48: pop di
da49: mov sp, bp
da4b: pop bp
da4c: ret 0x4
da4f: db 0x00                                ; word-alignment pad

; ---- func DA50  size 0x23e  bp frame, locals 8, ret 0x4  callers: B12A
da50: push bp
da51: mov bp, sp
da53: sub sp, 0x8
da56: push di
da57: push si
da58: sub ax, ax
da5a: mov word ptr [0xc402], ax
da5d: mov word ptr [0xc400], ax
da60: mov word ptr [0xc406], ax
da63: mov word ptr [0xc404], ax
da66: mov word ptr [0xc40a], ax
da69: mov word ptr [0xc408], ax
da6c: mov al, 0xc
da6e: mul byte ptr [bp + 0x6]
da71: mov bx, ax
da73: mov al, byte ptr [bx + 0x1117]         ; tbl DS:1117
da77: mov byte ptr [bp - 0x1], al
da7a: cmp al, -0x1
da7c: je 0xdaae
da7e: mov al, 0x14
da80: mul byte ptr [bp - 0x1]
da83: mov bx, ax
da85: mov ax, word ptr [bx + 0x191e]         ; tbl DS:191E
da89: add ax, word ptr [bx + 0x1920]         ; tbl DS:1920
da8d: mov word ptr [0xc408], ax
da90: sub si, si
da92: cmp word ptr [0xc408], si
da96: jbe 0xdaae
da98: mov ax, 0x64
da9b: push ax
da9c: call 0x76b                             ; EXE rng_roll(n)
da9f: cmp ax, 0x5
daa2: ja 0xdaa8
daa4: inc word ptr [0xc400]
daa8: add si, 0x1f4
daac: jmp 0xda92
daae: sub di, di
dab0: mov al, 0xc
dab2: mul byte ptr [bp + 0x6]
dab5: add ax, 0x110e
dab8: mov word ptr [bp - 0x6], ax
dabb: jmp 0xdb13
dabd: mov bx, word ptr [bp - 0x6]
dac0: mov ax, word ptr [bx]
dac2: mov word ptr [bp - 0x4], ax
dac5: add word ptr [0xc404], ax
dac9: mov al, byte ptr [0x491]
dacc: sub ah, ah
dace: mov si, ax
dad0: shl si
dad2: add si, ax
dad4: add si, di
dad6: mov al, byte ptr [si - 0x6644]         ; tbl DS:99BC
dada: mov byte ptr [bp - 0x1], al
dadd: mov word ptr [bp - 0x8], di
dae0: cmp byte ptr [0xc38f], ah
dae4: jne 0xdaed
dae6: mov al, byte ptr [si - 0x6638]         ; tbl DS:99C8
daea: mov byte ptr [bp - 0x1], al
daed: sub si, si
daef: jmp 0xdb06
daf1: mov ax, 0x64
daf4: push ax
daf5: call 0x76b                             ; EXE rng_roll(n)
daf8: mov cl, byte ptr [bp - 0x1]
dafb: sub ch, ch
dafd: cmp ax, cx
daff: ja 0xdb05
db01: inc word ptr [0xc400]
db05: inc si
db06: cmp word ptr [bp - 0x4], si
db09: ja 0xdaf1
db0b: add word ptr [bp - 0x6], 0x2
db0f: mov di, word ptr [bp - 0x8]
db12: inc di
db13: cmp di, 0x3
db16: jb 0xdabd
db18: cmp word ptr [0xc404], 0x0
db1d: je 0xdb53
db1f: mov bl, byte ptr [0x491]
db23: sub bh, bh
db25: mov di, word ptr [bx - 0x662c]         ; tbl DS:99D4
db29: and di, 0xff
db2d: sub si, si
db2f: mov word ptr [bp - 0x2], di
db32: cmp si, di
db34: jae 0xdb53
db36: mov ax, 0x64
db39: push ax
db3a: call 0x76b                             ; EXE rng_roll(n)
db3d: mov cx, ax
db3f: mov al, byte ptr [0xc38f]
db42: mov bl, 0x4
db44: cwde
db45: idiv bl
db47: cwde
db48: cmp cx, ax
db4a: jg 0xdb50
db4c: inc word ptr [0xc400]
db50: inc si
db51: jmp 0xdb32
db53: mov al, 0xc
db55: mul byte ptr [bp + 0x4]
db58: mov bx, ax
db5a: mov al, byte ptr [bx + 0x1117]         ; tbl DS:1117
db5e: mov byte ptr [bp - 0x1], al
db61: cmp al, -0x1
db63: je 0xdb95
db65: mov al, 0xe
db67: mul byte ptr [bp - 0x1]
db6a: mov bx, ax
db6c: mov ax, word ptr [bx - 0x3eea]         ; tbl DS:C116
db70: add ax, word ptr [bx - 0x3ee8]         ; tbl DS:C118
db74: mov word ptr [0xc40a], ax
db77: sub si, si
db79: cmp word ptr [0xc40a], si
db7d: jbe 0xdb95
db7f: mov ax, 0x64
db82: push ax
db83: call 0x76b                             ; EXE rng_roll(n)
db86: cmp ax, 0x5
db89: ja 0xdb8f
db8b: inc word ptr [0xc402]
db8f: add si, 0x1f4
db93: jmp 0xdb79
db95: mov al, 0x6
db97: mul byte ptr [bp + 0x4]
db9a: mov bx, ax
db9c: mov al, byte ptr [0xc37a]
db9f: sub ah, ah
dba1: add bx, ax
dba3: shl bx
dba5: mov cx, word ptr [bx + 0x110e]         ; tbl DS:110E
dba9: mov word ptr [0xc406], cx
dbad: mov bx, ax
dbaf: mov al, byte ptr [bx - 0x663e]         ; tbl DS:99C2
dbb3: mov byte ptr [bp - 0x1], al
dbb6: cmp byte ptr [0xc38e], ah
dbba: jne 0xdbc9
dbbc: mov bl, byte ptr [0xc37a]
dbc0: sub bh, bh
dbc2: mov al, byte ptr [bx - 0x6632]         ; tbl DS:99CE
dbc6: mov byte ptr [bp - 0x1], al
dbc9: sub si, si
dbcb: jmp 0xdbe2
dbcd: mov ax, 0x64
dbd0: push ax
dbd1: call 0x76b                             ; EXE rng_roll(n)
dbd4: mov cl, byte ptr [bp - 0x1]
dbd7: sub ch, ch
dbd9: cmp ax, cx
dbdb: ja 0xdbe1
dbdd: inc word ptr [0xc402]
dbe1: inc si
dbe2: cmp word ptr [0xc406], si
dbe6: ja 0xdbcd
dbe8: cmp word ptr [0xc406], 0x0
dbed: je 0xdc13
dbef: sub si, si
dbf1: cmp si, 0x4
dbf4: jae 0xdc13
dbf6: mov ax, 0x64
dbf9: push ax
dbfa: call 0x76b                             ; EXE rng_roll(n)
dbfd: mov cx, ax
dbff: mov al, byte ptr [0xc38e]
dc02: mov bl, 0x4
dc04: cwde
dc05: idiv bl
dc07: cwde
dc08: cmp cx, ax
dc0a: jg 0xdc10
dc0c: inc word ptr [0xc402]
dc10: inc si
dc11: jmp 0xdbf1
dc13: cmp word ptr [0xc404], 0x0
dc18: je 0xdc21
dc1a: mov al, byte ptr [bp + 0x6]
dc1d: push ax
dc1e: call 0xdd2e
dc21: cmp word ptr [0xc406], 0x0
dc26: je 0xdc2f
dc28: mov al, byte ptr [bp + 0x4]
dc2b: push ax
dc2c: call 0xde51
dc2f: sub di, di
dc31: mov word ptr [0xc406], di
dc35: mov word ptr [0xc404], di
dc39: mov al, 0xc
dc3b: mul byte ptr [bp + 0x4]
dc3e: add ax, 0x110e
dc41: mov word ptr [bp - 0x4], ax
dc44: mov al, 0xc
dc46: mul byte ptr [bp + 0x6]
dc49: add ax, 0x110e
dc4c: mov word ptr [bp - 0x2], ax
dc4f: jmp 0xdc6c
dc51: mov bx, word ptr [bp - 0x2]
dc54: mov ax, word ptr [bx]
dc56: add word ptr [0xc404], ax
dc5a: mov bx, word ptr [bp - 0x4]
dc5d: add word ptr [bp - 0x4], 0x2
dc61: mov ax, word ptr [bx]
dc63: add word ptr [0xc406], ax
dc67: add word ptr [bp - 0x2], 0x2
dc6b: inc di
dc6c: cmp di, 0x3
dc6f: jb 0xdc51
dc71: cmp word ptr [0xc404], 0x0
dc76: je 0xdc83
dc78: cmp word ptr [0xc406], 0x0
dc7d: je 0xdc83
dc7f: sub ax, ax
dc81: jmp 0xdc86
dc83: mov ax, 0xffff
dc86: pop si
dc87: pop di
dc88: mov sp, bp
dc8a: pop bp
dc8b: ret 0x4

; ---- func DC8E  size 0xa0  bp frame, locals 4, ret 0x4  callers: B12A BEAE
dc8e: push bp
dc8f: mov bp, sp
dc91: sub sp, 0x4
dc94: push di
dc95: sub di, di
dc97: mov word ptr [0xc406], di
dc9b: mov word ptr [0xc404], di
dc9f: mov al, 0xc
dca1: mul byte ptr [bp + 0x4]
dca4: add ax, 0x110e
dca7: mov word ptr [bp - 0x4], ax
dcaa: mov al, 0xc
dcac: mul byte ptr [bp + 0x6]
dcaf: add ax, 0x110e
dcb2: mov word ptr [bp - 0x2], ax
dcb5: jmp 0xdcd2
dcb7: mov bx, word ptr [bp - 0x2]
dcba: mov ax, word ptr [bx]
dcbc: add word ptr [0xc404], ax
dcc0: mov bx, word ptr [bp - 0x4]
dcc3: add word ptr [bp - 0x4], 0x2
dcc7: mov ax, word ptr [bx]
dcc9: add word ptr [0xc406], ax
dccd: add word ptr [bp - 0x2], 0x2
dcd1: inc di
dcd2: cmp di, 0x3
dcd5: jb 0xdcb7
dcd7: mov ax, 0xc7
dcda: mov word ptr [0xc0e0], ax
dcdd: mov word ptr [0xc1ea], ax
dce0: mov word ptr [0x9ac6], ax
dce3: mov ax, word ptr [0x6346]
dce6: mov word ptr [0x9afa], ax
dce9: mov word ptr [0x9b69], 0x0
dcef: mov ax, word ptr [0xc404]
dcf2: sub dx, dx
dcf4: push dx
dcf5: push ax
dcf6: mov ax, 0xae
dcf9: push ax
dcfa: mov ax, 0xbb
dcfd: push ax
dcfe: mov cx, 0x19
dd01: push cx
dd02: call 0x8af2                            ; EXE erase rect then print number
dd05: mov ax, word ptr [0xc406]
dd08: sub dx, dx
dd0a: push dx
dd0b: push ax
dd0c: mov ax, 0x90
dd0f: push ax
dd10: mov ax, 0xbb
dd13: push ax
dd14: mov ax, 0x19
dd17: push ax
dd18: call 0x8b2c                            ; EXE erase rect then print (8B2C)
dd1b: mov ax, 0xb9
dd1e: mov word ptr [0xc0e0], ax
dd21: mov word ptr [0xc1ea], ax
dd24: mov word ptr [0x9ac6], ax
dd27: pop di
dd28: mov sp, bp
dd2a: pop bp
dd2b: ret 0x4

; ---- func DD2E  size 0x123  bp frame, locals 14, ret 0x2  callers: DA50
dd2e: push bp
dd2f: mov bp, sp
dd31: sub sp, 0xe
dd34: push di
dd35: push si
dd36: cmp word ptr [0xc404], 0x0
dd3b: jne 0xdd40
dd3d: jmp 0xde49
dd40: mov al, byte ptr [bp + 0x4]
dd43: sub ah, ah
dd45: mov cx, 0xc
dd48: mov bx, ax
dd4a: mul cx
dd4c: mov si, ax
dd4e: mov word ptr [bp - 0xc], si
dd51: mov al, byte ptr [si + 0x1117]         ; tbl DS:1117
dd55: mov byte ptr [bp - 0x9], al
dd58: mov word ptr [bp - 0x6], 0x0
dd5d: mov si, 0xc3be
dd60: mov ax, bx
dd62: mul cx
dd64: mov di, ax
dd66: add di, 0x110e
dd6a: jmp 0xddb2
dd6c: mov ax, word ptr [di]
dd6e: mov word ptr [bp - 0x2], ax
dd71: mul word ptr [0xc402]
dd75: add ax, word ptr [si]
dd77: mov cx, ax
dd79: sub dx, dx
dd7b: div word ptr [0xc404]
dd7f: mov word ptr [bp - 0x4], ax
dd82: mov ax, cx
dd84: sub dx, dx
dd86: div word ptr [0xc404]
dd8a: mov word ptr [si], dx
dd8c: mov ax, word ptr [bp - 0x4]
dd8f: cmp word ptr [bp - 0x2], ax
dd92: jb 0xdd99
dd94: sub word ptr [bp - 0x2], ax
dd97: jmp 0xdda3
dd99: mov ax, word ptr [di]
dd9b: mov word ptr [bp - 0x4], ax
dd9e: mov word ptr [bp - 0x2], 0x0
dda3: mov ax, word ptr [bp - 0x2]
dda6: mov word ptr [di], ax
dda8: inc di
dda9: inc di
ddaa: mov ax, word ptr [bp - 0x4]
ddad: add word ptr [bp - 0x6], ax
ddb0: inc si
ddb1: inc si
ddb2: cmp si, 0xc3c4
ddb6: jb 0xdd6c
ddb8: mov si, word ptr [bp - 0x6]
ddbb: or si, si
ddbd: jne 0xddc2
ddbf: jmp 0xde49
ddc2: sub ax, ax
ddc4: add word ptr [0xc08e], si
ddc8: adc word ptr [0xc090], ax
ddcc: cmp word ptr [0xc408], ax
ddd0: je 0xde49
ddd2: mov al, 0x14
ddd4: mul byte ptr [bp - 0x9]
ddd7: mov bx, ax
ddd9: mov ax, word ptr [bx + 0x1920]         ; tbl DS:1920
dddd: mov word ptr [bp - 0x2], ax
dde0: add bx, 0x191e
dde4: mov word ptr [bp - 0xe], bx
dde7: mov di, word ptr [bx]
dde9: mov bx, word ptr [bp - 0xc]
ddec: mov cx, ax
ddee: mov al, byte ptr [bx + 0x1118]         ; tbl DS:1118
ddf2: sub ah, ah
ddf4: mul si
ddf6: mov si, ax
ddf8: mul cx
ddfa: sub dx, dx
ddfc: div word ptr [0xc408]
de00: shr ax
de02: shr ax
de04: mov word ptr [bp - 0x8], ax
de07: cmp ax, cx
de09: ja 0xde10
de0b: sub word ptr [bp - 0x2], ax
de0e: jmp 0xde1a
de10: mov ax, cx
de12: mov word ptr [bp - 0x8], ax
de15: mov word ptr [bp - 0x2], 0x0
de1a: mov al, 0x14
de1c: mul byte ptr [bp - 0x9]
de1f: mov bx, ax
de21: mov ax, word ptr [bp - 0x2]
de24: mov word ptr [bx + 0x1920], ax         ; tbl DS:1920
de28: sub si, word ptr [bp - 0x8]
de2b: cmp si, di
de2d: ja 0xde33
de2f: sub di, si
de31: jmp 0xde37
de33: mov si, di
de35: sub di, di
de37: mov bx, word ptr [bp - 0xe]
de3a: mov word ptr [bx], di
de3c: add si, word ptr [bp - 0x8]
de3f: sub cx, cx
de41: add word ptr [0xc086], si
de45: adc word ptr [0xc088], cx
de49: pop si
de4a: pop di
de4b: mov sp, bp
de4d: pop bp
de4e: ret 0x2

; ---- func DE51  size 0x129  bp frame, locals 18, ret 0x2  callers: DA50
de51: push bp
de52: mov bp, sp
de54: sub sp, 0x12
de57: push di
de58: push si
de59: mov al, byte ptr [bp + 0x4]
de5c: sub ah, ah
de5e: mov cx, 0xc
de61: mov bx, ax
de63: imul cx
de65: mov si, ax
de67: mov word ptr [bp - 0xc], si
de6a: mov al, byte ptr [si + 0x1117]         ; tbl DS:1117
de6e: mov byte ptr [bp - 0x5], al
de71: mov ax, bx
de73: mov cx, 0x6
de76: imul cx
de78: mov bx, ax
de7a: mov al, byte ptr [0xc37a]
de7d: sub ah, ah
de7f: add bx, ax
de81: shl bx
de83: add bx, 0x110e
de87: mov si, word ptr [bx]
de89: mov di, word ptr [0xc400]
de8d: cmp si, di
de8f: jb 0xde95
de91: sub si, di
de93: jmp 0xde99
de95: mov di, si
de97: sub si, si
de99: mov word ptr [bx], si
de9b: or di, di
de9d: jne 0xdea2
de9f: jmp 0xdf72
dea2: sub ax, ax
dea4: add word ptr [0xc08a], di
dea8: adc word ptr [0xc08c], ax
deac: cmp word ptr [0xc40a], ax
deb0: jne 0xdeb5
deb2: jmp 0xdf72
deb5: mov al, 0xe
deb7: mul byte ptr [bp - 0x5]
deba: mov bx, ax
debc: add bx, 0xc116
dec0: mov word ptr [bp - 0x10], bx
dec3: mov si, word ptr [bx]
dec5: mov bx, 0xc118
dec8: add bx, ax
deca: mov word ptr [bp - 0x12], bx
decd: mov cx, word ptr [bx]
decf: mov word ptr [bp - 0x4], cx
ded2: mov bx, ax
ded4: mov al, byte ptr [bx - 0x3ee4]         ; tbl DS:C11C
ded8: sub ah, ah
deda: mov word ptr [bp - 0x2], ax
dedd: mov bx, word ptr [bp - 0xc]
dee0: mov al, byte ptr [bx + 0x1118]         ; tbl DS:1118
dee4: mul di
dee6: mov di, ax
dee8: mov cx, di
deea: mov ax, word ptr [bp - 0x2]
deed: mul cx
deef: sub dx, dx
def1: div word ptr [0xc40a]
def5: mov cl, 0x3
def7: shr ax, cl
def9: mov word ptr [bp - 0xa], ax
defc: cmp ax, word ptr [bp - 0x2]
deff: ja 0xdf06
df01: sub word ptr [bp - 0x2], ax
df04: jmp 0xdf11
df06: mov ax, word ptr [bp - 0x2]
df09: mov word ptr [bp - 0xa], ax
df0c: mov word ptr [bp - 0x2], 0x0
df11: mov al, 0xe
df13: mul byte ptr [bp - 0x5]
df16: mov bx, ax
df18: mov al, byte ptr [bp - 0x2]
df1b: mov byte ptr [bx - 0x3ee4], al         ; tbl DS:C11C
df1f: sub di, word ptr [bp - 0xa]
df22: mov ax, di
df24: mul si
df26: sub dx, dx
df28: div word ptr [0xc40a]
df2c: shr ax
df2e: shr ax
df30: mov word ptr [bp - 0x8], ax
df33: cmp ax, si
df35: ja 0xdf3b
df37: sub si, ax
df39: jmp 0xdf40
df3b: mov word ptr [bp - 0x8], si
df3e: sub si, si
df40: mov bx, word ptr [bp - 0x10]
df43: mov word ptr [bx], si
df45: sub di, word ptr [bp - 0x8]
df48: cmp di, word ptr [bp - 0x4]
df4b: ja 0xdf52
df4d: sub word ptr [bp - 0x4], di
df50: jmp 0xdf5a
df52: mov di, word ptr [bp - 0x4]
df55: mov word ptr [bp - 0x4], 0x0
df5a: mov ax, word ptr [bp - 0x4]
df5d: mov bx, word ptr [bp - 0x12]
df60: mov word ptr [bx], ax
df62: add di, word ptr [bp - 0xa]
df65: add di, word ptr [bp - 0x8]
df68: sub cx, cx
df6a: add word ptr [0xc082], di
df6e: adc word ptr [0xc084], cx
df72: pop si
df73: pop di
df74: mov sp, bp
df76: pop bp
df77: ret 0x2

; ---- section padding
df7a: db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
