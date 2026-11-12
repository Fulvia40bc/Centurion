; OVL0 section 3 (file 0x3EF0-0x51A0, 0x12B0 bytes) = the gladiator show, loaded at 1331:0000 = 0823:B0E0-0xC390.
;   Bounds from the PLINK86 overlay table at 1322:0000 (= 0823:AFF0 in disasm/EXE_RUNTIME.bin): 16-byte entries, section n
;   at 1322:(n-1)*16 with flags +6, load segment +0xA (1331), end segment +0xC (145C = 1331 + 0x12B paragraphs), file
;   name +0xE (1322:0075 "OVL0.OVL") and the file position in paragraphs at +0x10 (= the next entry's +0: 0x03EF).
;   One entry: 0xB0E0 (EXE thunk 0xB0B6 = lcall 129B:02EC / dw 3 / jmp 0823:B0E0, called once, from Rome Games 0x34F5).
; llvm-objdump code16 listing (clang --target=i386-unknown-elf .incbin + llvm-objdump -M intel --adjust-vma), addresses are
;   0823 offsets. Disassembled one function at a time from B0E0, every push bp/mov bp,sp prologue and every near-call
;   target (B67C has no prologue), so nothing depends on a linear sweep. Checks (gen.py): the 17 functions tile
;   0xB0E0-0xC385 with no gap or overlap and no pad bytes (this section is not word-aligned); every function ends in
;   ret/ret n; every jump/call target and every jump-table entry inside the section is an instruction start.
; ONE data island: the jump table at 0xB91C-0xB92F (10 words) after `jmp cs:[bx+0xB91C]` at 0xB917 in B7FE (the
;   player's direction code [A2] = 1..10 -> new state), listed as dw. No other indirect jump or call, no lcall, no
;   other cs: operand. Bytes 0xC385-0xC38F are the section's zero padding (db).
; Notation: absolute DS operands are printed unsigned ([0xaa58], llvm prints [-0x55a8]); call targets are masked to
;   16 bits (llvm prints 0x1076b for 0x076B). llvm's code16 "cwde" = cbw (98h), "cdq" = cwd (99h).
;   Comments: EXE routine names (from the notes; (G) = guessed here), DS variable names (this section's, from
;   docs/GLAD_INDEX.md), "tbl DS:xxxx" for reg+disp table bases, and the static DS string (OVL0.OVL 0xB400 = DS:0000)
;   an immediate points at. Arguments are Pascal (callee pops, ret n): the FIRST pushed is the FIRST parameter
;   ([bp+4+2*(n-1)]). Index: docs/GLAD_INDEX.md.

; ---- func B0E0  size 0x2ab  bp frame, locals 20, ret
; ENTRY (EXE thunk B0B6 = lcall 129B:02EC, dw 3, jmp 0823:B0E0; called from Rome Games 0x34F5): the gladiator show, no args, no result
b0e0: push bp
b0e1: mov bp, sp
b0e3: sub sp, 0x14
b0e6: push si
b0e7: mov ax, 0x6
b0ea: push ax
b0eb: lea ax, [bp - 0x6]
b0ee: push ax
b0ef: call 0xf7b                             ; EXE archive open (0F7B(n, &arch))
b0f2: mov si, ax
b0f4: or si, si
b0f6: jne 0xb0fb
b0f8: jmp 0xb37f
b0fb: mov ax, 0x4
b0fe: push ax
b0ff: lea ax, [bp - 0x6]
b102: push ax
b103: call 0x13a8                            ; EXE load_picture(n, &archive): picture slot DS:0250+8n (name, flag, descriptor); slot 4 = arenabtm.cbm -> desc C0BE
b106: and si, ax
b108: mov ax, 0x7b89                         ; "coloseum.cbm"
b10b: push ax
b10c: lea ax, [bp - 0x6]
b10f: push ax
b110: lea ax, [bp - 0x14]
b113: push ax
b114: call 0x1273                            ; EXE load CBM member into a descriptor
b117: test si, ax
b119: je 0xb157
b11b: call 0x67e4                            ; EXE draw into the work buffer
b11e: mov word ptr [0x9afa], 0x0             ; fill colour
b124: sub ax, ax
b126: push ax
b127: push ax
b128: mov cx, 0x13f
b12b: push cx
b12c: mov dx, 0xc7
b12f: push dx
b130: call 0x7e74                            ; EXE fill rect (colour, ...)
b133: lea ax, [bp - 0x14]
b136: push ax
b137: sub cx, cx
b139: push cx
b13a: push cx
b13b: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
b13e: call 0x680f                            ; EXE target = the screen descriptor
b141: lea ax, [bp - 0x14]
b144: push ax
b145: call 0x86a0                            ; EXE blit (86A0)
b148: sub ax, ax
b14a: push ax
b14b: push ax
b14c: mov ax, 0x13f
b14f: push ax
b150: mov ax, 0xc7
b153: push ax
b154: call 0x6973                            ; EXE restore rect work buffer -> screen
b157: mov ax, 0x7b96                         ; "show"
b15a: push ax
b15b: lea ax, [bp - 0x6]
b15e: push ax
b15f: sub cx, cx
b161: push cx
b162: call 0x163b                            ; EXE sheet_163B: load xxxHDR/xxxDAT sprite sheet
b165: mov word ptr [0xc2ba], ax              ; sheet "show"
b168: mov ax, 0x7b9b                         ; "arena"
b16b: push ax
b16c: lea ax, [bp - 0x6]
b16f: push ax
b170: sub ax, ax
b172: push ax
b173: call 0x163b                            ; EXE sheet_163B: load xxxHDR/xxxDAT sprite sheet
b176: mov word ptr [0xc2b4], ax              ; sheet "arena"
b179: sub ax, ax
b17b: push ax
b17c: call 0x1d55                            ; EXE mouse cursor push/hide
b17f: mov word ptr [0x9b69], 0xf             ; text colour
b185: mov al, 0x1
b187: push ax
b188: call 0x5e1a                            ; EXE music_unload(n)
b18b: mov al, 0x4
b18d: push ax
b18e: call 0x5d73                            ; EXE music_load(n)
b191: mov al, 0x5
b193: push ax
b194: call 0x5d73                            ; EXE music_load(n)
b197: mov al, 0x6
b199: push ax
b19a: call 0x5d73                            ; EXE music_load(n)
b19d: mov al, 0x7
b19f: push ax
b1a0: call 0x5d73                            ; EXE music_load(n)
b1a3: mov al, 0x4
b1a5: push ax
b1a6: call 0x5d29                            ; EXE music_play(n)
b1a9: call 0xf8                              ; EXE timer0_reset (= 00D7(0))
b1ac: call 0x1ae                             ; EXE ticks_since(0)
b1af: cmp ax, 0x1b0
b1b2: jae 0xb1c2
b1b4: call 0x3aa                             ; EXE key_read: next key or 0
b1b7: or al, al
b1b9: jne 0xb1c2
b1bb: cmp word ptr [0xb2], 0x0               ; mouse buttons
b1c0: je 0xb1ac
b1c2: mov al, 0x7
b1c4: push ax
b1c5: call 0x5d29                            ; EXE music_play(n)
b1c8: call 0xb38b
b1cb: or ax, ax
b1cd: jne 0xb205
b1cf: mov al, 0x4
b1d1: push ax
b1d2: call 0x5e1a                            ; EXE music_unload(n)
b1d5: mov al, 0x5
b1d7: push ax
b1d8: call 0x5e1a                            ; EXE music_unload(n)
b1db: mov al, 0x6
b1dd: push ax
b1de: call 0x5e1a                            ; EXE music_unload(n)
b1e1: mov al, 0x7
b1e3: push ax
b1e4: call 0x5e1a                            ; EXE music_unload(n)
b1e7: push word ptr [0xc2ba]                 ; sheet "show"
b1eb: call 0x19a0                            ; EXE sheet_19A0: free sprite sheet
b1ee: push word ptr [0xc2b4]                 ; sheet "arena"
b1f2: call 0x19a0                            ; EXE sheet_19A0: free sprite sheet
b1f5: lea ax, [bp - 0x6]
b1f8: push ax
b1f9: call 0x11a9                            ; EXE archive close
b1fc: call 0x1d8a                            ; EXE mouse cursor pop
b1ff: call 0x5d20                            ; EXE music_stop
b202: jmp 0xb386
b205: mov al, 0x5
b207: push ax
b208: call 0x5d29                            ; EXE music_play(n)
b20b: call 0xb67c
b20e: mov ax, 0x7bbf                         ; "VERSUS"
b211: push ax
b212: mov ax, 0xa0
b215: push ax
b216: mov ax, 0x7d
b219: push ax
b21a: call 0x8be4                            ; EXE centred_text(str, cx, y)
b21d: mov ax, 0x5
b220: push ax
b221: mov al, byte ptr [0xaa51]              ; player record
b224: sub ah, ah
b226: push ax
b227: mov ax, 0xffff
b22a: push ax
b22b: call 0xb6ec
b22e: mov ax, 0x3
b231: push ax
b232: mov al, byte ptr [0xaa50]              ; opponent record
b235: sub ah, ah
b237: push ax
b238: sub ax, ax
b23a: push ax
b23b: call 0xb6ec
b23e: push word ptr [0xc2ba]                 ; sheet "show"
b242: call 0x19a0                            ; EXE sheet_19A0: free sprite sheet
b245: mov al, 0xa
b247: mul byte ptr [0xaa51]                  ; player record
b24b: mov bx, ax
b24d: mov bl, byte ptr [bx + 0x7e45]         ; tbl DS:7E45 glad.body
b251: sub bh, bh
b253: shl bx
b255: push word ptr [bx + 0x7bcc]            ; tbl DS:7BCC player sheet names
b259: lea ax, [bp - 0x6]
b25c: push ax
b25d: sub cx, cx
b25f: push cx
b260: call 0x163b                            ; EXE sheet_163B: load xxxHDR/xxxDAT sprite sheet
b263: mov word ptr [0xc2b6], ax              ; sheet player fighter
b266: mov al, 0xa
b268: mul byte ptr [0xaa50]                  ; opponent record
b26c: mov bx, ax
b26e: mov bl, byte ptr [bx + 0x7e45]         ; tbl DS:7E45 glad.body
b272: sub bh, bh
b274: shl bx
b276: push word ptr [bx + 0x7bd0]            ; tbl DS:7BD0 opponent sheet names
b27a: lea ax, [bp - 0x6]
b27d: push ax
b27e: sub cx, cx
b280: push cx
b281: call 0x163b                            ; EXE sheet_163B: load xxxHDR/xxxDAT sprite sheet
b284: mov word ptr [0xc2b8], ax              ; sheet opponent fighter
b287: lea ax, [bp - 0x6]
b28a: push ax
b28b: call 0x11a9                            ; EXE archive close
b28e: mov al, 0x4
b290: push ax
b291: call 0x5d29                            ; EXE music_play(n)
b294: mov ax, 0x7bc6                         ; "" (empty message box)
b297: push ax
b298: call 0x59d4                            ; EXE message_box(str)
b29b: call 0x67e4                            ; EXE draw into the work buffer
b29e: mov word ptr [0x9afa], 0x0             ; fill colour
b2a4: sub ax, ax
b2a6: push ax
b2a7: push ax
b2a8: mov cx, 0x13f
b2ab: push cx
b2ac: mov dx, 0xc7
b2af: push dx
b2b0: call 0x7e74                            ; EXE fill rect (colour, ...)
b2b3: push word ptr [0xc2b4]                 ; sheet "arena"
b2b7: sub ax, ax
b2b9: push ax
b2ba: push ax
b2bb: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
b2be: mov ax, 0xc0d0                         ; current target descriptor
b2c1: push ax
b2c2: mov ax, 0x7bc8                         ; point {0, 126} (arenabtm position)
b2c5: push ax
b2c6: mov ax, 0xc0be                         ; picture slot 4 descriptor (arenabtm.cbm)
b2c9: push ax
b2ca: call 0x8790                            ; EXE blit picture (8790(desc, dst, ...))
b2cd: call 0x680f                            ; EXE target = the screen descriptor
b2d0: sub ax, ax
b2d2: push ax
b2d3: push ax
b2d4: mov ax, 0x13f
b2d7: push ax
b2d8: mov ax, 0xc7
b2db: push ax
b2dc: call 0x6973                            ; EXE restore rect work buffer -> screen
b2df: mov al, byte ptr [0xaa51]              ; player record
b2e2: push ax
b2e3: mov al, byte ptr [0xaa50]              ; opponent record
b2e6: push ax
b2e7: call 0xc288
b2ea: call 0xc1a4
b2ed: mov al, 0x5
b2ef: push ax
b2f0: call 0x5d29                            ; EXE music_play(n)
b2f3: inc word ptr [0xc100]                  ; games held
b2f7: cmp byte ptr [0xed4], 0xa              ; Rome-bored chance
b2fc: jle 0xb303
b2fe: mov byte ptr [0xed4], 0xa              ; Rome-bored chance
b303: mov word ptr [0xed8], 0x0              ; Rome-bored crisis flag
b309: mov al, byte ptr [0xaa50]              ; opponent record
b30c: push ax
b30d: call 0xb7fe
b310: call 0x680f                            ; EXE target = the screen descriptor
b313: mov ax, word ptr [0x634c]              ; turn
b316: mov word ptr [0x154c], ax              ; last show turn
b319: mov al, byte ptr [0xaa51]              ; player record
b31c: push ax
b31d: mov al, byte ptr [0xaa50]              ; opponent record
b320: push ax
b321: call 0xbdea
b324: call 0x680f                            ; EXE target = the screen descriptor
b327: call 0x5d20                            ; EXE music_stop
b32a: mov al, 0x4
b32c: push ax
b32d: call 0x5e1a                            ; EXE music_unload(n)
b330: mov al, 0x5
b332: push ax
b333: call 0x5e1a                            ; EXE music_unload(n)
b336: mov al, 0x6
b338: push ax
b339: call 0x5e1a                            ; EXE music_unload(n)
b33c: mov al, 0x7
b33e: push ax
b33f: call 0x5e1a                            ; EXE music_unload(n)
b342: mov ax, 0x7bc7                         ; "" (empty message box)
b345: push ax
b346: call 0x59d4                            ; EXE message_box(str)
b349: mov al, 0xa
b34b: mul byte ptr [0xaa51]                  ; player record
b34f: mov bx, ax
b351: sub al, al
b353: mov byte ptr [bx + 0x7e43], al         ; tbl DS:7E43 glad.status
b357: mov cx, ax
b359: mov al, 0xa
b35b: mul byte ptr [0xaa50]                  ; opponent record
b35f: mov bx, ax
b361: mov byte ptr [bx + 0x7e43], cl         ; tbl DS:7E43 glad.status
b365: push word ptr [0xc2b4]                 ; sheet "arena"
b369: call 0x19a0                            ; EXE sheet_19A0: free sprite sheet
b36c: push word ptr [0xc2b6]                 ; sheet player fighter
b370: call 0x19a0                            ; EXE sheet_19A0: free sprite sheet
b373: push word ptr [0xc2b8]                 ; sheet opponent fighter
b377: call 0x19a0                            ; EXE sheet_19A0: free sprite sheet
b37a: call 0x1d8a                            ; EXE mouse cursor pop
b37d: jmp 0xb386
b37f: mov ax, 0x7ba1                         ; "Can't Load Gladiator Graphics"
b382: push ax
b383: call 0x59d4                            ; EXE message_box(str)
b386: pop si
b387: mov sp, bp
b389: pop bp
b38a: ret

; ---- func B38B  size 0x118  bp frame, locals 14, ret  callers: B0E0
b38b: push bp
b38c: mov bp, sp
b38e: sub sp, 0xe
b391: push di
b392: push si
b393: cmp word ptr [0x634a], 0x14            ; treasury
b398: jae 0xb3a6
b39a: mov ax, 0x7cc2                         ; "You Cannot Afford\nTwo Gladiators."
b39d: push ax
b39e: call 0x59d4                            ; EXE message_box(str)
b3a1: sub ax, ax
b3a3: jmp 0xb49d
b3a6: mov ax, word ptr [0x634a]              ; treasury
b3a9: mov word ptr [bp - 0x8], ax
b3ac: call 0xb67c
b3af: mov word ptr [bp - 0x6], 0x0
b3b4: mov ax, 0xa
b3b7: push ax
b3b8: call 0x76b                             ; EXE rng_roll(n)
b3bb: mov word ptr [bp - 0x2], ax
b3be: mov word ptr [bp - 0x4], ax
b3c1: mov si, ax
b3c3: mov ax, si
b3c5: mov cx, 0xa
b3c8: mul cx
b3ca: mov bx, ax
b3cc: cmp byte ptr [bx + 0x7e43], 0x0        ; tbl DS:7E43 glad.status
b3d1: je 0xb3e6
b3d3: lea ax, [si + 0x1]
b3d6: mov cx, 0xa
b3d9: sub dx, dx
b3db: div cx
b3dd: mov si, dx
b3df: cmp si, word ptr [bp - 0x2]
b3e2: jne 0xb3c3
b3e4: jmp 0xb3f7
b3e6: mov ax, si
b3e8: mov di, word ptr [bp - 0x6]
b3eb: mov byte ptr [bp + di - 0xe], al
b3ee: mul cx
b3f0: mov bx, ax
b3f2: mov byte ptr [bx + 0x7e43], 0x3        ; tbl DS:7E43 glad.status
b3f7: push word ptr [bp - 0x6]
b3fa: push si
b3fb: mov ax, 0xffff
b3fe: push ax
b3ff: call 0xb6ec
b402: inc word ptr [bp - 0x6]
b405: cmp word ptr [bp - 0x6], 0x6
b409: jb 0xb3b4
b40b: mov ax, 0x7ce4                         ; "Select a Gladiator\nfor You to Control"
b40e: push ax
b40f: call 0x59d4                            ; EXE message_box(str)
b412: lea ax, [bp - 0xe]
b415: push ax
b416: call 0xb4da
b419: mov byte ptr [0xaa51], al              ; player record
b41c: cmp al, -0x1
b41e: jne 0xb42d
b420: lea ax, [bp - 0xe]
b423: push ax
b424: call 0xb4a3
b427: call 0x1d19                            ; EXE cursor_hide
b42a: jmp 0xb3a1
b42d: call 0x1d19                            ; EXE cursor_hide
b430: call 0xb67c
b433: sub si, si
b435: push si
b436: mov al, byte ptr [bp + si - 0xe]
b439: sub ah, ah
b43b: push ax
b43c: sub ax, ax
b43e: push ax
b43f: call 0xb6ec
b442: inc si
b443: cmp si, 0x6
b446: jb 0xb435
b448: call 0x1cce                            ; EXE cursor_show
b44b: cmp word ptr [0x634a], 0xa             ; treasury
b450: jae 0xb476
b452: mov ax, 0x7d0a                         ; "You Cannot Afford a Second Gladiator.\nYour Talents for the First are Refunded."
b455: push ax
b456: call 0x59d4                            ; EXE message_box(str)
b459: mov ax, word ptr [bp - 0x8]
b45c: mov word ptr [0x634a], ax              ; treasury
b45f: mov al, 0xa
b461: mul byte ptr [0xaa51]                  ; player record
b465: mov bx, ax
b467: mov byte ptr [bx + 0x7e43], 0x0        ; tbl DS:7E43 glad.status
b46c: lea ax, [bp - 0xe]
b46f: push ax
b470: call 0xb4a3
b473: jmp 0xb3a1
b476: mov ax, 0x7d59                         ; "Select a Gladiator\nTo Fight Against"
b479: push ax
b47a: call 0x59d4                            ; EXE message_box(str)
b47d: lea ax, [bp - 0xe]
b480: push ax
b481: call 0xb4da
b484: mov byte ptr [0xaa50], al              ; opponent record
b487: cmp al, -0x1
b489: jne 0xb490
b48b: call 0x1d19                            ; EXE cursor_hide
b48e: jmp 0xb45f
b490: lea ax, [bp - 0xe]
b493: push ax
b494: call 0xb4a3
b497: call 0x1d19                            ; EXE cursor_hide
b49a: mov ax, 0xffff
b49d: pop si
b49e: pop di
b49f: mov sp, bp
b4a1: pop bp
b4a2: ret

; ---- func B4A3  size 0x37  bp frame, locals 0, ret 0x2  callers: B38B
b4a3: push bp
b4a4: mov bp, sp
b4a6: push di
b4a7: push si
b4a8: sub si, si
b4aa: mov bx, word ptr [bp + 0x4]
b4ad: mov al, byte ptr [bx + si]
b4af: sub ah, ah
b4b1: mov di, ax
b4b3: mov cx, 0xa
b4b6: imul cx
b4b8: mov bx, ax
b4ba: cmp byte ptr [bx + 0x7e43], 0x3        ; tbl DS:7E43 glad.status
b4bf: jne 0xb4cc
b4c1: mov ax, di
b4c3: imul cx
b4c5: mov bx, ax
b4c7: mov byte ptr [bx + 0x7e43], 0x0        ; tbl DS:7E43 glad.status
b4cc: inc si
b4cd: cmp si, 0x6
b4d0: jl 0xb4aa
b4d2: pop si
b4d3: pop di
b4d4: mov sp, bp
b4d6: pop bp
b4d7: ret 0x2

; ---- func B4DA  size 0x1a2  bp frame, locals 120, ret 0x2  callers: B38B
b4da: push bp
b4db: mov bp, sp
b4dd: sub sp, 0x78
b4e0: push di
b4e1: push si
b4e2: sub ax, ax
b4e4: mov word ptr [bp - 0x2], ax
b4e7: mov word ptr [bp - 0xa], ax
b4ea: mov word ptr [bp - 0x8], ax
b4ed: mov word ptr [bp - 0xc], ax
b4f0: call 0x1cce                            ; EXE cursor_show
b4f3: cmp word ptr [bp - 0x2], 0x0
b4f7: je 0xb4fc
b4f9: jmp 0xb59f
b4fc: jmp 0xb514
b4fe: call 0x359b                            ; EXE wait for click or key (key -> [C07F])
b501: or al, al
b503: je 0xb514
b505: cmp byte ptr [0xc07f], 0x1b            ; last key (359B)
b50a: jne 0xb50f
b50c: jmp 0xb597
b50f: mov word ptr [bp - 0x8], 0xffff
b514: cmp word ptr [bp - 0x8], 0x0
b518: je 0xb4fe
b51a: sub si, si
b51c: cmp si, 0x6
b51f: jge 0xb591
b521: mov ax, si
b523: mov cx, 0x3
b526: cdq
b527: idiv cx
b529: mov bx, 0x64
b52c: imul bx
b52e: inc ax
b52f: inc ax
b530: mov word ptr [bp - 0x12], ax
b533: mov bx, 0x6a
b536: mov dx, ax
b538: mov ax, si
b53a: mov di, dx
b53c: cdq
b53d: idiv cx
b53f: mov ax, dx
b541: imul bx
b543: add ax, 0x7
b546: mov word ptr [bp - 0x14], ax
b549: add ax, 0x5b
b54c: mov word ptr [bp - 0x10], ax
b54f: lea ax, [di + 0x5b]
b552: mov word ptr [bp - 0xe], ax
b555: push word ptr [0xb6]                   ; mouse x
b559: push word ptr [0xb8]                   ; mouse y
b55d: lea ax, [bp - 0x14]
b560: push ax
b561: call 0x8238                            ; EXE in_rect(rect, y, x): inclusive, signed
b564: or ax, ax
b566: jne 0xb56b
b568: inc si
b569: jmp 0xb51c
b56b: mov bx, word ptr [bp + 0x4]
b56e: mov al, byte ptr [bx + si]
b570: mov byte ptr [bp - 0x5], al
b573: mov cl, 0xa
b575: mul cl
b577: mov di, ax
b579: mov byte ptr [di + 0x7e43], 0x1        ; tbl DS:7E43 glad.status
b57e: mov ax, 0x3
b581: push ax
b582: call 0x76b                             ; EXE rng_roll(n)
b585: add al, 0xa
b587: mov bx, word ptr [bp + 0x4]
b58a: mov byte ptr [bx + si], al
b58c: mov word ptr [bp - 0x2], 0xffff
b591: call 0x35db                            ; EXE wait until the mouse buttons are released
b594: jmp 0xb4f3
b597: call 0x1d19                            ; EXE cursor_hide
b59a: mov al, -0x1
b59c: jmp 0xb674
b59f: call 0x1d19                            ; EXE cursor_hide
b5a2: mov word ptr [bp - 0x4], 0x7c80
b5a7: sub di, di
b5a9: mov word ptr [bp - 0x2], di
b5ac: lea si, [bp - 0x78]
b5af: jmp 0xb5fa
b5b1: mov al, 0xa
b5b3: mul byte ptr [bp - 0x5]
b5b6: mov bx, ax
b5b8: cmp byte ptr [bx + 0x7e44], 0x4        ; tbl DS:7E44 glad.style (4 = beast)
b5bd: jne 0xb5c8
b5bf: mov bx, word ptr [bp - 0x2]
b5c2: push word ptr [bx + 0x7c76]            ; tbl DS:7C76 beast labels
b5c6: jmp 0xb5cf
b5c8: mov bx, word ptr [bp - 0x2]
b5cb: push word ptr [bx + 0x7c6e]            ; tbl DS:7C6E training labels
b5cf: push si
b5d0: call 0x9e32                            ; EXE strcpy
b5d3: pop bx
b5d4: pop bx
b5d5: lea ax, [di + 0x31]
b5d8: mov bx, word ptr [0x7c7e]              ; ptr to " (20 Talents)" (DS:7B68)
b5dc: mov byte ptr [bx + 0x2], al
b5df: push word ptr [0x7c7e]                 ; ptr to " (20 Talents)" (DS:7B68)
b5e3: push si
b5e4: call 0x9df2                            ; EXE strcat
b5e7: pop bx
b5e8: pop bx
b5e9: mov bx, word ptr [bp - 0x4]
b5ec: add word ptr [bp - 0x4], 0xc
b5f0: mov word ptr [bx], si
b5f2: add si, 0x19
b5f5: add word ptr [bp - 0x2], 0x2
b5f9: inc di
b5fa: lea ax, [bp - 0x14]
b5fd: cmp si, ax
b5ff: jb 0xb5b1
b601: mov ax, 0x7cb0                         ; menu "How Much Training?"
b604: push ax
b605: mov ax, 0x1
b608: push ax
b609: call 0x5614                            ; EXE 5614(src, mode) screen/palette copy into the work buffer
b60c: call 0x1cce                            ; EXE cursor_show
b60f: mov di, word ptr [bp - 0xc]
b612: mov si, word ptr [bp - 0xa]
b615: jmp 0xb66d
b617: mov ax, 0x7cb0                         ; menu "How Much Training?"
b61a: push ax
b61b: push di
b61c: call 0x54c8                            ; EXE wait for buttons/click (54C8 polling)
b61f: mov di, ax
b621: sub ax, 0xff
b624: je 0xb666
b626: lea ax, [di + 0x1]
b629: mov cx, 0xa
b62c: mul cx
b62e: cmp ax, word ptr [0x634a]              ; treasury
b632: ja 0xb652
b634: mov al, 0xa
b636: mul byte ptr [bp - 0x5]
b639: mov bx, ax
b63b: mov ax, di
b63d: mov byte ptr [bx + 0x7e46], al         ; tbl DS:7E46 glad.training
b641: lea ax, [di + 0x1]
b644: mov cx, 0xa
b647: mul cx
b649: sub word ptr [0x634a], ax              ; treasury
b64d: mov si, 0xffff
b650: jmp 0xb66d
b652: mov ax, 0x7cb0                         ; menu "How Much Training?"
b655: push ax
b656: push di
b657: sub ax, ax
b659: push ax
b65a: call 0x585b                            ; EXE menu_preselect(menu, sel, -1)
b65d: mov ax, 0x7d7d                         ; "You Cannot Afford Such\na Well-Trained Gladiator"
b660: push ax
b661: call 0x59d4                            ; EXE message_box(str)
b664: jmp 0xb66d
b666: mov si, 0xffff
b669: mov byte ptr [bp - 0x5], -0x1
b66d: or si, si
b66f: je 0xb617
b671: mov al, byte ptr [bp - 0x5]
b674: pop si
b675: pop di
b676: mov sp, bp
b678: pop bp
b679: ret 0x2

; ---- func B67C  size 0x70  no bp frame, ret  callers: B0E0 B38B
b67c: push si
b67d: call 0x67e4                            ; EXE draw into the work buffer
b680: mov word ptr [0x9afa], 0x1             ; fill colour
b686: sub ax, ax
b688: push ax
b689: push ax
b68a: mov ax, 0x13f
b68d: push ax
b68e: mov ax, 0xc7
b691: push ax
b692: call 0x7e74                            ; EXE fill rect (colour, ...)
b695: mov si, 0x7bda                         ; hline table {colour, y}
b698: jmp 0xb6b2
b69a: mov ax, word ptr [si]
b69c: mov word ptr [0x9afa], ax              ; fill colour
b69f: sub ax, ax
b6a1: push ax
b6a2: mov al, byte ptr [si + 0x2]
b6a5: sub ah, ah
b6a7: push ax
b6a8: mov ax, 0x13f
b6ab: push ax
b6ac: call 0x7eca                            ; EXE hline(x1, y, x0) clipped
b6af: add si, 0x4
b6b2: cmp si, 0x7c1a
b6b6: jb 0xb69a
b6b8: mov si, 0x7c1e                         ; card-piece table {x, y, frame} + 4
b6bb: jmp 0xb6d2
b6bd: mov al, 0xe
b6bf: mul byte ptr [si]
b6c1: add ax, word ptr [0xc2b4]              ; sheet "arena"
b6c5: push ax
b6c6: push word ptr [si - 0x4]
b6c9: push word ptr [si - 0x2]
b6cc: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
b6cf: add si, 0x6
b6d2: cmp si, 0x7c72
b6d6: jb 0xb6bd
b6d8: call 0x680f                            ; EXE target = the screen descriptor
b6db: sub ax, ax
b6dd: push ax
b6de: push ax
b6df: mov ax, 0x13f
b6e2: push ax
b6e3: mov ax, 0xc7
b6e6: push ax
b6e7: call 0x6973                            ; EXE restore rect work buffer -> screen
b6ea: pop si
b6eb: ret

; ---- func B6EC  size 0x112  bp frame, locals 50, ret 0x6  callers: B0E0 B38B
b6ec: push bp
b6ed: mov bp, sp
b6ef: sub sp, 0x32
b6f2: push si
b6f3: mov si, word ptr [bp + 0x6]
b6f6: mov ax, word ptr [bp + 0x8]
b6f9: mov cx, 0x3
b6fc: sub dx, dx
b6fe: div cx
b700: mov bx, 0x64
b703: mul bx
b705: add ax, 0x57
b708: mov word ptr [bp - 0x4], ax
b70b: mov ax, word ptr [bp + 0x8]
b70e: sub dx, dx
b710: div cx
b712: mov ax, dx
b714: mov cx, 0x6a
b717: mul cx
b719: add ax, 0x38
b71c: mov word ptr [bp - 0x2], ax
b71f: mov ax, si
b721: mov cx, 0xa
b724: mul cx
b726: mov bx, ax
b728: mov word ptr [bp - 0x32], bx
b72b: push word ptr [bx + 0x7e3e]            ; tbl DS:7E3E glad.name
b72f: lea ax, [bp - 0x30]
b732: push ax
b733: call 0x9e32                            ; EXE strcpy
b736: pop bx
b737: pop bx
b738: mov bx, word ptr [bp - 0x32]
b73b: cmp byte ptr [bx + 0x7e44], 0x4        ; tbl DS:7E44 glad.style (4 = beast)
b740: je 0xb75d
b742: cmp byte ptr [bx + 0x7e40], 0x1        ; tbl DS:7E40 glad.generation
b747: je 0xb75d
b749: mov al, byte ptr [bx + 0x7e40]         ; tbl DS:7E40 glad.generation
b74d: sub ah, ah
b74f: push ax
b750: call 0x6738                            ; EXE roman_numeral(n) -> DS:C072
b753: push ax
b754: lea ax, [bp - 0x30]
b757: push ax
b758: call 0x9df2                            ; EXE strcat
b75b: pop bx
b75c: pop bx
b75d: lea ax, [bp - 0x30]
b760: push ax
b761: push word ptr [bp - 0x2]
b764: mov ax, word ptr [bp - 0x4]
b767: sub ax, 0x50
b76a: push ax
b76b: call 0x8be4                            ; EXE centred_text(str, cx, y)
b76e: mov bx, word ptr [bp - 0x32]
b771: cmp byte ptr [bx + 0x7e41], 0x0        ; tbl DS:7E41 glad.wins
b776: je 0xb7a4
b778: mov ax, 0x7dad                         ; "Bouts Won"
b77b: push ax
b77c: push word ptr [bp - 0x2]
b77f: mov ax, word ptr [bp - 0x4]
b782: sub ax, 0x46
b785: push ax
b786: call 0x8be4                            ; EXE centred_text(str, cx, y)
b789: mov bx, word ptr [bp - 0x32]
b78c: mov al, byte ptr [bx + 0x7e41]         ; tbl DS:7E41 glad.wins
b790: sub ah, ah
b792: push ax
b793: call 0x6738                            ; EXE roman_numeral(n) -> DS:C072
b796: push ax
b797: push word ptr [bp - 0x2]
b79a: mov ax, word ptr [bp - 0x4]
b79d: sub ax, 0x3c
b7a0: push ax
b7a1: call 0x8be4                            ; EXE centred_text(str, cx, y)
b7a4: cmp word ptr [bp + 0x4], 0x0
b7a8: je 0xb7c1
b7aa: mov bx, word ptr [bp - 0x32]
b7ad: mov al, byte ptr [bx + 0x7e45]         ; tbl DS:7E45 glad.body
b7b1: sub ah, ah
b7b3: mov word ptr [bp - 0x8], ax
b7b6: mov ax, word ptr [bp - 0x2]
b7b9: sub ax, 0xa
b7bc: mov word ptr [bp - 0x6], ax
b7bf: jmp 0xb7e3
b7c1: mov bx, word ptr [bp - 0x32]
b7c4: mov al, byte ptr [bx + 0x7e45]         ; tbl DS:7E45 glad.body
b7c8: sub ah, ah
b7ca: inc ax
b7cb: inc ax
b7cc: mov word ptr [bp - 0x8], ax
b7cf: mov ax, word ptr [bp - 0x2]
b7d2: add ax, 0xa
b7d5: mov word ptr [bp - 0x6], ax
b7d8: cmp byte ptr [bx + 0x7e44], 0x4        ; tbl DS:7E44 glad.style (4 = beast)
b7dd: jne 0xb7e3
b7df: add word ptr [bp - 0x6], 0xf
b7e3: mov ax, 0xe
b7e6: mul word ptr [bp - 0x8]
b7e9: add ax, word ptr [0xc2ba]              ; sheet "show"
b7ed: push ax
b7ee: push word ptr [bp - 0x6]
b7f1: push word ptr [bp - 0x4]
b7f4: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
b7f7: pop si
b7f8: mov sp, bp
b7fa: pop bp
b7fb: ret 0x6

; ---- func B7FE  size 0x33d  bp frame, locals 16, ret 0x2  callers: B0E0
b7fe: push bp
b7ff: mov bp, sp
b801: sub sp, 0x10
b804: push di
b805: push si
b806: mov ax, 0x6
b809: mov word ptr [bp - 0xa], ax
b80c: mov word ptr [bp - 0xc], ax
b80f: sub ax, ax
b811: mov word ptr [bp - 0x4], ax
b814: mov word ptr [bp - 0x10], ax
b817: mov word ptr [bp - 0xe], ax
b81a: mov word ptr [bp - 0x8], ax
b81d: mov word ptr [bp - 0x6], ax
b820: mov word ptr [0xaa68], ax              ; boredom run (-)
b823: mov word ptr [0xaa60], ax              ; exchange run (+)
b826: mov word ptr [0xaa5e], ax              ; hits landed
b829: mov word ptr [0xaa64], ax              ; blocks
b82c: mov word ptr [0xaa66], 0x1             ; crowd mood 0..2
b832: mov al, 0xa
b834: mov byte ptr [0xaa74], al              ; O.health
b837: mov byte ptr [0xaa5c], al              ; P.health
b83a: mov word ptr [bp - 0x2], 0x0
b83f: jmp 0xb84f
b841: mov ax, 0xa
b844: push ax
b845: call 0x76b                             ; EXE rng_roll(n)
b848: add byte ptr [0xaa5c], al              ; P.health
b84c: inc word ptr [bp - 0x2]
b84f: mov al, byte ptr [0xaa5a]              ; P.training
b852: sub ah, ah
b854: cmp ax, word ptr [bp - 0x2]
b857: ja 0xb841
b859: sub si, si
b85b: jmp 0xb869
b85d: mov ax, 0xa
b860: push ax
b861: call 0x76b                             ; EXE rng_roll(n)
b864: add byte ptr [0xaa74], al              ; O.health
b868: inc si
b869: mov al, byte ptr [0xaa72]              ; O.training
b86c: sub ah, ah
b86e: cmp ax, si
b870: ja 0xb85d
b872: mov ax, 0x1
b875: push ax
b876: call 0xc25f
b879: sub ax, ax
b87b: push ax
b87c: call 0xc25f
b87f: mov ax, 0x1
b882: push ax
b883: call 0xd7                              ; EXE timer_start(n) (copy tick DS:42/44 to slot 46+4n)
b886: mov al, byte ptr [0xaa72]              ; O.training
b889: sub ah, ah
b88b: mov cl, byte ptr [0xaa5a]              ; P.training
b88f: sub ch, ch
b891: add ax, cx
b893: inc ax
b894: inc ax
b895: mov word ptr [0xaa62], ax              ; crowd meter 0..30
b898: mov ax, 0x4f
b89b: push ax
b89c: mov ax, 0xa
b89f: push ax
b8a0: mov ax, 0xea
b8a3: push ax
b8a4: mov ax, 0x14
b8a7: push ax
b8a8: push word ptr [0x6346]                 ; box colour
b8ac: call 0x3233                            ; EXE box(colour, x0, y0, x1, y1): 7E74 fill + 925C bevel lines (G)
b8af: push word ptr [0xaa62]                 ; crowd meter 0..30
b8b3: call 0xc149
b8b6: jmp 0xbb24
b8b9: call 0x3aa                             ; EXE key_read: next key or 0
b8bc: call 0x4ed                             ; EXE key_flush
b8bf: cmp byte ptr [0xaa58], -0x1            ; P.state
b8c4: je 0xb8cd
b8c6: cmp byte ptr [0xaa70], -0x1            ; O.state
b8cb: jne 0xb8d2
b8cd: mov word ptr [bp - 0x4], 0xffff
b8d2: cmp word ptr [bp - 0x8], 0x0
b8d6: jg 0xb8ff
b8d8: mov al, byte ptr [0xaa5a]              ; P.training
b8db: sub ah, ah
b8dd: sub ax, 0x4
b8e0: neg ax
b8e2: shl ax
b8e4: shl ax
b8e6: mov word ptr [bp - 0x8], ax
b8e9: mov si, word ptr [0xaa58]              ; P.state
b8ed: and si, 0xff
b8f1: shl si
b8f3: shl si
b8f5: mov bx, word ptr [0xaa52]              ; P.movetab
b8f9: test byte ptr [bx + si + 0x3], 0x1
b8fd: jne 0xb902
b8ff: jmp 0xb997
b902: cmp byte ptr [0xaa5c], 0x0             ; P.health
b907: jle 0xb987
b909: mov al, byte ptr [0xa2]                ; key/direction code (EXE input)
b90c: sub ah, ah
b90e: dec ax
b90f: cmp ax, 0x9
b912: ja 0xb930
b914: shl ax
b916: xchg ax, bx
b917: jmp word ptr cs:[bx - 0x46e4]          ; CS jump table at 0xB91C (data island, 10 words)
b91c: dw 0xb937                              ; jump table B91C[0] (key [A2] = 1) -> B937
b91e: dw 0xb946                              ; jump table B91C[1] (key [A2] = 2) -> B946
b920: dw 0xb930                              ; jump table B91C[2] (key [A2] = 3) -> B930
b922: dw 0xb954                              ; jump table B91C[3] (key [A2] = 4) -> B954
b924: dw 0xb930                              ; jump table B91C[4] (key [A2] = 5) -> B930
b926: dw 0xb95b                              ; jump table B91C[5] (key [A2] = 6) -> B95B
b928: dw 0xb930                              ; jump table B91C[6] (key [A2] = 7) -> B930
b92a: dw 0xb962                              ; jump table B91C[7] (key [A2] = 8) -> B962
b92c: dw 0xb930                              ; jump table B91C[8] (key [A2] = 9) -> B930
b92e: dw 0xb980                              ; jump table B91C[9] (key [A2] = 10) -> B980
b930: mov byte ptr [0xaa58], 0x0             ; P.state
b935: jmp 0xb98c
b937: cmp word ptr [0xaa54], 0x122           ; P.x
b93d: jge 0xb98c
b93f: mov byte ptr [0xaa58], 0x2             ; P.state
b944: jmp 0xb98c
b946: cmp word ptr [0xaa54], 0x1e            ; P.x
b94b: jle 0xb98c
b94d: mov byte ptr [0xaa58], 0x1             ; P.state
b952: jmp 0xb98c
b954: mov byte ptr [0xaa58], 0x5             ; P.state
b959: jmp 0xb98c
b95b: mov byte ptr [0xaa58], 0x4             ; P.state
b960: jmp 0xb98c
b962: cmp byte ptr [0xaa58], 0x0             ; P.state
b967: jne 0xb98c
b969: mov byte ptr [0xaa58], 0x7             ; P.state
b96e: mov ax, 0x2
b971: push ax
b972: call 0x76b                             ; EXE rng_roll(n)
b975: or ax, ax
b977: je 0xb98c
b979: mov byte ptr [0xaa58], 0x6             ; P.state
b97e: jmp 0xb98c
b980: mov byte ptr [0xaa58], 0x3             ; P.state
b985: jmp 0xb98c
b987: mov byte ptr [0xaa58], 0x9             ; P.state
b98c: mov al, byte ptr [0xaa58]              ; P.state
b98f: mov byte ptr [0xaa59], al              ; P.state copy
b992: mov word ptr [bp - 0x10], 0xffff
b997: dec word ptr [bp - 0x8]
b99a: dec word ptr [bp - 0xa]
b99d: cmp word ptr [bp - 0x6], 0x0
b9a1: jg 0xb9d4
b9a3: mov al, byte ptr [0xaa72]              ; O.training
b9a6: sub ah, ah
b9a8: sub ax, 0x4
b9ab: neg ax
b9ad: shl ax
b9af: mov word ptr [bp - 0x6], ax
b9b2: mov si, word ptr [0xaa70]              ; O.state
b9b6: and si, 0xff
b9ba: shl si
b9bc: shl si
b9be: mov bx, word ptr [0xaa6a]              ; O.movetab
b9c2: test byte ptr [bx + si + 0x3], 0x1
b9c6: je 0xb9d4
b9c8: mov al, byte ptr [bp + 0x4]
b9cb: push ax
b9cc: call 0xbc4c
b9cf: mov word ptr [bp - 0xe], 0xffff
b9d4: dec word ptr [bp - 0x6]
b9d7: dec word ptr [bp - 0xc]
b9da: cmp word ptr [bp - 0x4], 0x0
b9de: je 0xb9e3
b9e0: jmp 0xbb24
b9e3: call 0xf8                              ; EXE timer0_reset (= 00D7(0))
b9e6: cmp word ptr [bp - 0x10], 0x0
b9ea: je 0xba0a
b9ec: mov bl, byte ptr [0xaa58]              ; P.state
b9f0: sub bh, bh
b9f2: shl bx
b9f4: shl bx
b9f6: mov si, word ptr [0xaa52]              ; P.movetab
b9fa: mov al, byte ptr [bx + si]
b9fc: cwde
b9fd: mov si, ax
b9ff: add si, word ptr [0xaa54]              ; P.x
ba03: mov word ptr [bp - 0x10], 0x0
ba08: jmp 0xba0e
ba0a: mov si, word ptr [0xaa54]              ; P.x
ba0e: cmp word ptr [bp - 0xe], 0x0
ba12: je 0xba32
ba14: mov bl, byte ptr [0xaa70]              ; O.state
ba18: sub bh, bh
ba1a: shl bx
ba1c: shl bx
ba1e: add bx, word ptr [0xaa6a]              ; O.movetab
ba22: mov al, byte ptr [bx]
ba24: cwde
ba25: mov di, ax
ba27: add di, word ptr [0xaa6c]              ; O.x
ba2b: mov word ptr [bp - 0xe], 0x0
ba30: jmp 0xba36
ba32: mov di, word ptr [0xaa6c]              ; O.x
ba36: cmp di, si
ba38: jge 0xba40
ba3a: mov word ptr [0xaa54], si              ; P.x
ba3e: jmp 0xba5d
ba40: mov ax, 0x2
ba43: push ax
ba44: call 0x76b                             ; EXE rng_roll(n)
ba47: or ax, ax
ba49: jne 0xba57
ba4b: cmp word ptr [0xaa6c], si              ; O.x
ba4f: jge 0xba61
ba51: mov word ptr [0xaa54], si              ; P.x
ba55: jmp 0xba61
ba57: cmp word ptr [0xaa54], di              ; P.x
ba5b: jle 0xba61
ba5d: mov word ptr [0xaa6c], di              ; O.x
ba61: call 0xc1a4
ba64: cmp word ptr [bp - 0xa], 0x0
ba68: jne 0xba9b
ba6a: mov ax, 0xaa52                         ; &player fighter
ba6d: push ax
ba6e: mov cx, 0xaa6a                         ; &opponent fighter
ba71: push cx
ba72: call 0xbbc8
ba75: sub ax, ax
ba77: push ax
ba78: call 0xc25f
ba7b: mov si, word ptr [0xaa58]              ; P.state
ba7f: and si, 0xff
ba83: shl si
ba85: shl si
ba87: mov bx, word ptr [0xaa52]              ; P.movetab
ba8b: mov al, byte ptr [bx + si + 0x1]
ba8e: mov byte ptr [0xaa58], al              ; P.state
ba91: mov word ptr [bp - 0x10], 0xffff
ba96: mov word ptr [bp - 0xa], 0x6
ba9b: cmp word ptr [bp - 0xc], 0x0
ba9f: jne 0xbad3
baa1: mov ax, 0xaa6a                         ; &opponent fighter
baa4: push ax
baa5: mov cx, 0xaa52                         ; &player fighter
baa8: push cx
baa9: call 0xbbc8
baac: mov ax, 0x1
baaf: push ax
bab0: call 0xc25f
bab3: mov si, word ptr [0xaa70]              ; O.state
bab7: and si, 0xff
babb: shl si
babd: shl si
babf: mov bx, word ptr [0xaa6a]              ; O.movetab
bac3: mov al, byte ptr [bx + si + 0x1]
bac6: mov byte ptr [0xaa70], al              ; O.state
bac9: mov word ptr [bp - 0xe], 0xffff
bace: mov word ptr [bp - 0xc], 0x6
bad3: call 0x1ae                             ; EXE ticks_since(0)
bad6: cmp ax, 0x4
bad9: jb 0xbad3
badb: mov ax, 0x1
bade: push ax
badf: call 0x183                             ; EXE ticks_since(n)
bae2: cmp ax, 0xd8
bae5: jbe 0xbaf5
bae7: mov ax, 0xffff
baea: push ax
baeb: call 0xbb3b
baee: mov ax, 0x1
baf1: push ax
baf2: call 0xd7                              ; EXE timer_start(n) (copy tick DS:42/44 to slot 46+4n)
baf5: cmp byte ptr [0xaa58], -0x1            ; P.state
bafa: je 0xbb03
bafc: cmp byte ptr [0xaa70], -0x1            ; O.state
bb01: jne 0xbb24
bb03: cmp byte ptr [0xaa58], 0x0             ; P.state
bb08: je 0xbb11
bb0a: cmp byte ptr [0xaa58], -0x1            ; P.state
bb0f: jne 0xbb24
bb11: cmp byte ptr [0xaa70], 0x0             ; O.state
bb16: je 0xbb1f
bb18: cmp byte ptr [0xaa70], -0x1            ; O.state
bb1d: jne 0xbb24
bb1f: mov word ptr [bp - 0x4], 0xffff
bb24: cmp word ptr [bp - 0x4], 0x0
bb28: jne 0xbb2d
bb2a: jmp 0xb8b9
bb2d: call 0x4ed                             ; EXE key_flush
bb30: call 0x5d20                            ; EXE music_stop
bb33: pop si
bb34: pop di
bb35: mov sp, bp
bb37: pop bp
bb38: ret 0x2

; ---- func BB3B  size 0x8d  bp frame, locals 0, ret 0x2  callers: B7FE BBC8 BDEA
bb3b: push bp
bb3c: mov bp, sp
bb3e: mov ax, word ptr [bp + 0x4]
bb41: add word ptr [0xaa62], ax              ; crowd meter 0..30
bb45: jns 0xbb4f
bb47: mov word ptr [0xaa62], 0x0             ; crowd meter 0..30
bb4d: jmp 0xbb5c
bb4f: cmp word ptr [0xaa62], 0x1e            ; crowd meter 0..30
bb54: jle 0xbb5c
bb56: mov word ptr [0xaa62], 0x1e            ; crowd meter 0..30
bb5c: push word ptr [0xaa62]                 ; crowd meter 0..30
bb60: call 0xc149
bb63: cmp word ptr [bp + 0x4], 0x0
bb67: jle 0xbb93
bb69: mov word ptr [0xaa68], 0x0             ; boredom run (-)
bb6f: inc word ptr [0xaa60]                  ; exchange run (+)
bb73: cmp word ptr [0xaa60], 0x3             ; exchange run (+)
bb78: jle 0xbbc2
bb7a: mov word ptr [0xaa60], 0x0             ; exchange run (+)
bb80: inc word ptr [0xaa66]                  ; crowd mood 0..2
bb84: cmp word ptr [0xaa66], 0x2             ; crowd mood 0..2
bb89: jle 0xbbb6
bb8b: mov word ptr [0xaa66], 0x2             ; crowd mood 0..2
bb91: jmp 0xbbb6
bb93: mov word ptr [0xaa60], 0x0             ; exchange run (+)
bb99: inc word ptr [0xaa68]                  ; boredom run (-)
bb9d: cmp word ptr [0xaa68], 0x3             ; boredom run (-)
bba2: jle 0xbbc2
bba4: mov word ptr [0xaa68], 0x0             ; boredom run (-)
bbaa: dec word ptr [0xaa66]                  ; crowd mood 0..2
bbae: jns 0xbbb6
bbb0: mov word ptr [0xaa66], 0x0             ; crowd mood 0..2
bbb6: mov bx, word ptr [0xaa66]              ; crowd mood 0..2
bbba: mov al, byte ptr [bx - 0x7ff6]         ; tbl DS:800A crowd music by mood
bbbe: push ax
bbbf: call 0x5d29                            ; EXE music_play(n)
bbc2: mov sp, bp
bbc4: pop bp
bbc5: ret 0x2

; ---- func BBC8  size 0x84  bp frame, locals 2, ret 0x4  callers: B7FE
bbc8: push bp
bbc9: mov bp, sp
bbcb: sub sp, 0x2
bbce: push si
bbcf: mov bx, word ptr [bp + 0x6]
bbd2: mov si, word ptr [bx + 0x6]
bbd5: and si, 0xff
bbd9: shl si
bbdb: shl si
bbdd: mov bx, word ptr [bx]
bbdf: mov al, byte ptr [bx + si + 0x3]
bbe2: and al, 0x6
bbe4: mov byte ptr [bp - 0x1], al
bbe7: or al, al
bbe9: je 0xbc45
bbeb: mov bx, word ptr [bp + 0x4]
bbee: mov ax, word ptr [bx + 0x2]
bbf1: mov bx, word ptr [bp + 0x6]
bbf4: sub ax, word ptr [bx + 0x2]
bbf7: cdq
bbf8: xor ax, dx
bbfa: sub ax, dx
bbfc: cmp ax, 0x14
bbff: jge 0xbc45
bc01: mov bx, word ptr [bp + 0x4]
bc04: mov si, word ptr [bx + 0x6]
bc07: and si, 0xff
bc0b: shl si
bc0d: shl si
bc0f: mov bx, word ptr [bx]
bc11: mov al, byte ptr [bx + si + 0x3]
bc14: and al, 0x18
bc16: sar al
bc18: sar al
bc1a: cmp al, byte ptr [bp - 0x1]
bc1d: je 0xbc33
bc1f: inc word ptr [0xaa5e]                  ; hits landed
bc23: mov bx, word ptr [bp + 0x6]
bc26: mov al, byte ptr [bx + 0x8]
bc29: inc al
bc2b: mov bx, word ptr [bp + 0x4]
bc2e: sub byte ptr [bx + 0xa], al
bc31: jmp 0xbc37
bc33: inc word ptr [0xaa64]                  ; blocks
bc37: mov ax, 0x1
bc3a: push ax
bc3b: call 0xbb3b
bc3e: mov ax, 0x1
bc41: push ax
bc42: call 0xd7                              ; EXE timer_start(n) (copy tick DS:42/44 to slot 46+4n)
bc45: pop si
bc46: mov sp, bp
bc48: pop bp
bc49: ret 0x4

; ---- func BC4C  size 0x19e  bp frame, locals 2, ret 0x2  callers: B7FE
bc4c: push bp
bc4d: mov bp, sp
bc4f: sub sp, 0x2
bc52: push di
bc53: push si
bc54: sub di, di
bc56: mov si, di
bc58: cmp byte ptr [0xaa74], 0x0             ; O.health
bc5d: jg 0xbc79
bc5f: mov al, 0xa
bc61: mul byte ptr [bp + 0x4]
bc64: mov bx, ax
bc66: cmp byte ptr [bx + 0x7e44], 0x4        ; tbl DS:7E44 glad.style (4 = beast)
bc6b: jne 0xbc71
bc6d: mov al, 0x4
bc6f: jmp 0xbc73
bc71: mov al, 0x9
bc73: mov byte ptr [0xaa70], al              ; O.state
bc76: jmp 0xbde2
bc79: mov al, byte ptr [0xaa72]              ; O.training
bc7c: add al, byte ptr [0x490]               ; difficulty: gladiators (048E[2])
bc80: add al, 0x5
bc82: add byte ptr [0x800d], al              ; enemy AI pace accumulator
bc86: cmp byte ptr [0x800d], 0x1e            ; enemy AI pace accumulator
bc8b: jae 0xbc90
bc8d: jmp 0xbde2
bc90: mov byte ptr [0x800d], 0x0             ; enemy AI pace accumulator
bc95: mov ax, word ptr [0xaa6c]              ; O.x
bc98: add ax, 0x14
bc9b: cmp ax, word ptr [0xaa54]              ; P.x
bc9f: jl 0xbca4
bca1: mov si, 0xffff
bca4: mov ax, word ptr [0xaa6c]              ; O.x
bca7: add ax, 0x24
bcaa: cmp ax, word ptr [0xaa54]              ; P.x
bcae: jl 0xbcb7
bcb0: or si, si
bcb2: jne 0xbcb7
bcb4: mov di, 0xffff
bcb7: mov al, 0xa
bcb9: mul byte ptr [bp + 0x4]
bcbc: mov bx, ax
bcbe: add bx, 0x7e44
bcc2: mov word ptr [bp - 0x2], bx
bcc5: mov al, byte ptr [bx]
bcc7: sub ah, ah
bcc9: or ax, ax
bccb: je 0xbcdf
bccd: dec ax
bcce: je 0xbcf4
bcd0: dec ax
bcd1: jge 0xbcd6
bcd3: jmp 0xbd83
bcd6: dec ax
bcd7: jle 0xbd14
bcd9: dec ax
bcda: je 0xbd58
bcdc: jmp 0xbd83
bcdf: or si, si
bce1: je 0xbcf8
bce3: mov ax, 0x2
bce6: push ax
bce7: call 0x76b                             ; EXE rng_roll(n)
bcea: cmp ax, 0x1
bced: sbb al, al
bcef: and al, 0x1
bcf1: jmp 0xbd7e
bcf4: or di, di
bcf6: je 0xbcfd
bcf8: mov al, 0x1
bcfa: jmp 0xbd80
bcfd: or si, si
bcff: je 0xbd10
bd01: mov ax, 0xa
bd04: push ax
bd05: call 0x76b                             ; EXE rng_roll(n)
bd08: mov bx, ax
bd0a: mov al, byte ptr [bx + 0x7ff6]         ; tbl DS:7FF6 AI moves (style 1)
bd0e: jmp 0xbd80
bd10: mov al, 0x2
bd12: jmp 0xbd80
bd14: or si, si
bd16: je 0xbd27
bd18: mov ax, 0xa
bd1b: push ax
bd1c: call 0x76b                             ; EXE rng_roll(n)
bd1f: mov bx, ax
bd21: mov al, byte ptr [bx - 0x8000]         ; tbl DS:8000 AI moves (style 2)
bd25: jmp 0xbd80
bd27: or di, di
bd29: je 0xbd32
bd2b: mov byte ptr [0xaa70], 0x1             ; O.state
bd30: jmp 0xbd83
bd32: mov ax, 0x4
bd35: push ax
bd36: call 0x76b                             ; EXE rng_roll(n)
bd39: mov si, ax
bd3b: cmp si, 0x2
bd3e: jb 0xbd51
bd40: mov byte ptr [0xaa70], 0x6             ; O.state
bd45: cmp si, 0x3
bd48: jne 0xbd83
bd4a: mov byte ptr [0xaa70], 0x7             ; O.state
bd4f: jmp 0xbd83
bd51: mov byte ptr [0xaa70], 0x5             ; O.state
bd56: jmp 0xbd83
bd58: cmp byte ptr [0xaa74], 0x5             ; O.health
bd5d: jle 0xbd6c
bd5f: or di, di
bd61: jne 0xbcf8
bd63: cmp si, 0x1
bd66: sbb al, al
bd68: and al, -0x2
bd6a: jmp 0xbd7e
bd6c: cmp word ptr [0xaa6c], 0x1e            ; O.x
bd71: jge 0xbd77
bd73: mov al, 0x3
bd75: jmp 0xbd80
bd77: cmp si, 0x1
bd7a: sbb al, al
bd7c: and al, -0x1
bd7e: add al, 0x3
bd80: mov byte ptr [0xaa70], al              ; O.state
bd83: mov bx, word ptr [bp - 0x2]
bd86: cmp byte ptr [bx], 0x4
bd89: je 0xbdc0
bd8b: cmp byte ptr [0xaa70], 0x1             ; O.state
bd90: jne 0xbd9f
bd92: cmp word ptr [0xaa6c], 0x122           ; O.x
bd98: jle 0xbd9f
bd9a: mov byte ptr [0xaa70], 0x4             ; O.state
bd9f: cmp byte ptr [0xaa70], 0x2             ; O.state
bda4: jne 0xbde2
bda6: cmp word ptr [0xaa6c], 0x1e            ; O.x
bdab: jge 0xbde2
bdad: mov ax, 0x2
bdb0: push ax
bdb1: call 0x76b                             ; EXE rng_roll(n)
bdb4: cmp ax, 0x1
bdb7: sbb al, al
bdb9: and al, -0x1
bdbb: add al, 0x6
bdbd: jmp 0xbc73
bdc0: cmp byte ptr [0xaa70], 0x1             ; O.state
bdc5: jne 0xbdcf
bdc7: cmp word ptr [0xaa6c], 0x122           ; O.x
bdcd: jg 0xbddd
bdcf: cmp byte ptr [0xaa70], 0x2             ; O.state
bdd4: jne 0xbde2
bdd6: cmp word ptr [0xaa6c], 0x1e            ; O.x
bddb: jge 0xbde2
bddd: mov byte ptr [0xaa70], 0x3             ; O.state
bde2: pop si
bde3: pop di
bde4: mov sp, bp
bde6: pop bp
bde7: ret 0x2

; ---- func BDEA  size 0x30f  bp frame, locals 20, ret 0x4  callers: B0E0
bdea: push bp
bdeb: mov bp, sp
bded: sub sp, 0x14
bdf0: push di
bdf1: push si
bdf2: sub ax, ax
bdf4: mov word ptr [bp - 0x8], ax
bdf7: mov word ptr [bp - 0x12], ax
bdfa: mov word ptr [bp - 0xa], ax
bdfd: mov word ptr [bp - 0xc], ax
be00: mov word ptr [bp - 0xe], ax
be03: mov word ptr [bp - 0x6], ax
be06: mov word ptr [bp - 0x2], ax
be09: mov word ptr [bp - 0x4], ax
be0c: mov word ptr [bp - 0x10], ax
be0f: mov al, byte ptr [0xaa72]              ; O.training
be12: sub ah, ah
be14: mov cl, byte ptr [0xaa5a]              ; P.training
be18: sub ch, ch
be1a: add ax, cx
be1c: inc ax
be1d: inc ax
be1e: shl ax
be20: cmp ax, word ptr [0xaa62]              ; crowd meter 0..30
be24: jge 0xbe2b
be26: mov word ptr [bp - 0x10], 0xffff
be2b: cmp byte ptr [0xaa5c], 0x0             ; P.health
be30: jle 0xbe6a
be32: mov al, 0xa
be34: mul byte ptr [bp + 0x6]
be37: mov bx, ax
be39: inc byte ptr [bx + 0x7e41]             ; tbl DS:7E41 glad.wins
be3d: mov word ptr [bp - 0x8], 0xffff
be42: mov al, byte ptr [0xaa5a]              ; P.training
be45: cmp byte ptr [0xaa72], al              ; O.training
be49: jbe 0xbe61
be4b: mov al, byte ptr [0xaa72]              ; O.training
be4e: sub ah, ah
be50: mov cl, byte ptr [0xaa5a]              ; P.training
be54: sub ch, ch
be56: sub ax, cx
be58: push ax
be59: call 0x7330                            ; EXE score_add(n)
be5c: mov word ptr [bp - 0x2], 0xffff
be61: mov ax, 0x3
be64: push ax
be65: call 0x7330                            ; EXE score_add(n)
be68: jmp 0xbe75
be6a: mov al, 0xa
be6c: mul byte ptr [bp + 0x6]
be6f: mov bx, ax
be71: inc byte ptr [bx + 0x7e42]             ; tbl DS:7E42 glad.losses
be75: cmp byte ptr [0xaa74], 0x0             ; O.health
be7a: jle 0xbebc
be7c: mov al, 0xa
be7e: mul byte ptr [bp + 0x4]
be81: mov bx, ax
be83: mov word ptr [bp - 0x14], bx
be86: inc byte ptr [bx + 0x7e41]             ; tbl DS:7E41 glad.wins
be8a: mov word ptr [bp - 0x12], 0xffff
be8f: mov al, byte ptr [0xaa5a]              ; P.training
be92: cmp byte ptr [0xaa72], al              ; O.training
be96: jae 0xbeab
be98: sub ah, ah
be9a: mov cl, byte ptr [0xaa72]              ; O.training
be9e: sub ch, ch
bea0: sub ax, cx
bea2: push ax
bea3: call 0x7330                            ; EXE score_add(n)
bea6: mov word ptr [bp - 0x2], 0xffff
beab: mov bx, word ptr [bp - 0x14]
beae: cmp byte ptr [bx + 0x7e44], 0x4        ; tbl DS:7E44 glad.style (4 = beast)
beb3: jne 0xbed3
beb5: mov word ptr [bp - 0xa], 0xffff
beba: jmp 0xbed3
bebc: mov al, 0xa
bebe: mul byte ptr [bp + 0x4]
bec1: mov bx, ax
bec3: inc byte ptr [bx + 0x7e42]             ; tbl DS:7E42 glad.losses
bec7: cmp byte ptr [bx + 0x7e44], 0x4        ; tbl DS:7E44 glad.style (4 = beast)
becc: jne 0xbed3
bece: mov word ptr [bp - 0xc], 0xffff
bed3: cmp word ptr [bp - 0x2], 0x0
bed7: je 0xbee0
bed9: mov ax, 0x3
bedc: push ax
bedd: call 0x7330                            ; EXE score_add(n)
bee0: cmp word ptr [bp - 0x8], 0x0
bee4: jne 0xbeff
bee6: cmp word ptr [bp - 0x12], 0x0
beea: jne 0xbeff
beec: mov ax, 0x3
beef: push ax
bef0: call 0x7330                            ; EXE score_add(n)
bef3: mov ax, 0xffff
bef6: mov word ptr [bp - 0xc], ax
bef9: mov word ptr [bp - 0xa], ax
befc: jmp 0xc0d3
beff: cmp word ptr [bp - 0xc], 0x0
bf03: je 0xbf08
bf05: jmp 0xc0d3
bf08: cmp word ptr [bp - 0xa], 0x0
bf0c: je 0xbf11
bf0e: jmp 0xc0d3
bf11: mov di, word ptr [bp - 0x4]
bf14: jmp 0xbfd1
bf17: call 0xf8                              ; EXE timer0_reset (= 00D7(0))
bf1a: cmp word ptr [bp - 0x8], 0x0
bf1e: je 0xbf58
bf20: mov al, byte ptr [0xaa73]              ; O.dying state
bf23: mov byte ptr [0xaa70], al              ; O.state
bf26: mov si, word ptr [0xaa58]              ; P.state
bf2a: and si, 0xff
bf2e: shl si
bf30: shl si
bf32: mov bx, word ptr [0xaa52]              ; P.movetab
bf36: test byte ptr [bx + si + 0x3], 0x1
bf3a: je 0xbf41
bf3c: mov byte ptr [0xaa58], 0x1             ; P.state
bf41: mov bl, byte ptr [0xaa58]              ; P.state
bf45: sub bh, bh
bf47: shl bx
bf49: shl bx
bf4b: mov si, word ptr [0xaa52]              ; P.movetab
bf4f: mov al, byte ptr [bx + si]
bf51: cwde
bf52: add word ptr [0xaa54], ax              ; P.x
bf56: jmp 0xbf8e
bf58: mov al, byte ptr [0xaa5b]              ; P.dying state
bf5b: mov byte ptr [0xaa58], al              ; P.state
bf5e: mov si, word ptr [0xaa70]              ; O.state
bf62: and si, 0xff
bf66: shl si
bf68: shl si
bf6a: mov bx, word ptr [0xaa6a]              ; O.movetab
bf6e: test byte ptr [bx + si + 0x3], 0x1
bf72: je 0xbf79
bf74: mov byte ptr [0xaa70], 0x1             ; O.state
bf79: mov bl, byte ptr [0xaa70]              ; O.state
bf7d: sub bh, bh
bf7f: shl bx
bf81: shl bx
bf83: mov si, word ptr [0xaa6a]              ; O.movetab
bf87: mov al, byte ptr [bx + si]
bf89: cwde
bf8a: add word ptr [0xaa6c], ax              ; O.x
bf8e: call 0xc1a4
bf91: call 0x1ae                             ; EXE ticks_since(0)
bf94: cmp ax, 0x4
bf97: jb 0xbf91
bf99: mov ax, word ptr [0xaa54]              ; P.x
bf9c: cmp word ptr [0xaa6c], ax              ; O.x
bfa0: jle 0xbfa5
bfa2: mov di, 0xffff
bfa5: mov si, word ptr [0xaa58]              ; P.state
bfa9: and si, 0xff
bfad: shl si
bfaf: shl si
bfb1: mov bx, word ptr [0xaa52]              ; P.movetab
bfb5: mov al, byte ptr [bx + si + 0x1]
bfb8: mov byte ptr [0xaa58], al              ; P.state
bfbb: mov si, word ptr [0xaa70]              ; O.state
bfbf: and si, 0xff
bfc3: shl si
bfc5: shl si
bfc7: mov bx, word ptr [0xaa6a]              ; O.movetab
bfcb: mov al, byte ptr [bx + si + 0x1]
bfce: mov byte ptr [0xaa70], al              ; O.state
bfd1: or di, di
bfd3: jne 0xbfd8
bfd5: jmp 0xbf17
bfd8: mov ax, word ptr [0xc2b4]              ; sheet "arena"
bfdb: add ax, 0x2a
bfde: push ax
bfdf: mov ax, 0x4f
bfe2: push ax
bfe3: mov ax, 0x14
bfe6: push ax
bfe7: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
bfea: push word ptr [0xaa62]                 ; crowd meter 0..30
bfee: call 0xc149
bff1: cmp word ptr [0xba], 0x0               ; mouse present
bff6: je 0xbffb
bff8: call 0x1cce                            ; EXE cursor_show
bffb: mov ax, 0x800e                         ; "Thumbs Up or Down?"
bffe: push ax
bfff: call 0x59d4                            ; EXE message_box(str)
c002: jmp 0xc06a
c004: or di, di
c006: jne 0xc073
c008: cmp word ptr [0xba], di                ; mouse present
c00c: je 0xc050
c00e: call 0x644                             ; EXE mouse_poll: [B2] buttons, [B6] x, [B8] y
c011: push word ptr [0xb6]                   ; mouse x
c015: push word ptr [0xb8]                   ; mouse y
c019: call 0x1be6                            ; EXE cursor_move(x, y)
c01c: cmp word ptr [0xb2], di                ; mouse buttons
c020: je 0xc050
c022: push word ptr [0xb6]                   ; mouse x
c026: push word ptr [0xb8]                   ; mouse y
c02a: mov ax, 0x7ec0                         ; rect thumbs up
c02d: push ax
c02e: call 0x8238                            ; EXE in_rect(rect, y, x): inclusive, signed
c031: or ax, ax
c033: je 0xc03a
c035: mov word ptr [bp - 0xe], 0xffff
c03a: push word ptr [0xb6]                   ; mouse x
c03e: push word ptr [0xb8]                   ; mouse y
c042: mov ax, 0x7ec8                         ; rect thumbs down
c045: push ax
c046: call 0x8238                            ; EXE in_rect(rect, y, x): inclusive, signed
c049: or ax, ax
c04b: je 0xc050
c04d: mov di, 0xffff
c050: cmp byte ptr [0xa2], 0x8               ; key/direction code (EXE input)
c055: jne 0xc05e
c057: mov word ptr [bp - 0xe], 0xffff
c05c: jmp 0xc06d
c05e: cmp byte ptr [0xa2], 0x4               ; key/direction code (EXE input)
c063: jne 0xc06d
c065: mov word ptr [bp - 0x6], 0xffff
c06a: mov di, word ptr [bp - 0x6]
c06d: cmp word ptr [bp - 0xe], 0x0
c071: je 0xc004
c073: call 0x4ed                             ; EXE key_flush
c076: or di, di
c078: je 0xc0b3
c07a: cmp word ptr [bp - 0x8], 0x0
c07e: je 0xc092
c080: mov word ptr [bp - 0xc], 0xffff
c085: mov byte ptr [0xaa58], 0x8             ; P.state
c08a: mov al, byte ptr [0xaa73]              ; O.dying state
c08d: mov byte ptr [0xaa70], al              ; O.state
c090: jmp 0xc0a8
c092: cmp word ptr [bp - 0x12], 0x0
c096: je 0xc0a8
c098: mov word ptr [bp - 0xa], 0xffff
c09d: mov byte ptr [0xaa70], 0x8             ; O.state
c0a2: mov al, byte ptr [0xaa5b]              ; P.dying state
c0a5: mov byte ptr [0xaa58], al              ; P.state
c0a8: call 0xc1a4
c0ab: cmp word ptr [bp - 0x10], 0x0
c0af: je 0xc0b9
c0b1: jmp 0xc0c5
c0b3: cmp word ptr [bp - 0x10], 0x0
c0b7: je 0xc0c5
c0b9: mov ax, 0x5
c0bc: push ax
c0bd: call 0xbb3b
c0c0: mov ax, 0x5
c0c3: jmp 0xc0cf
c0c5: mov ax, 0xfffb
c0c8: push ax
c0c9: call 0xbb3b
c0cc: mov ax, 0xfffb
c0cf: push ax
c0d0: call 0x7330                            ; EXE score_add(n)
c0d3: cmp word ptr [bp - 0xc], 0x0
c0d7: je 0xc0e2
c0d9: mov al, byte ptr [bp + 0x4]
c0dc: sub ah, ah
c0de: push ax
c0df: call 0xc0f9
c0e2: cmp word ptr [bp - 0xa], 0x0
c0e6: je 0xc0f1
c0e8: mov al, byte ptr [bp + 0x6]
c0eb: sub ah, ah
c0ed: push ax
c0ee: call 0xc0f9
c0f1: pop si
c0f2: pop di
c0f3: mov sp, bp
c0f5: pop bp
c0f6: ret 0x4

; ---- func C0F9  size 0x50  bp frame, locals 2, ret 0x2  callers: BDEA
c0f9: push bp
c0fa: mov bp, sp
c0fc: sub sp, 0x2
c0ff: mov ax, 0xa
c102: mul word ptr [bp + 0x4]
c105: mov bx, ax
c107: mov word ptr [bp - 0x2], bx
c10a: sub al, al
c10c: mov byte ptr [bx + 0x7e42], al         ; tbl DS:7E42 glad.losses
c110: mov byte ptr [bx + 0x7e41], al         ; tbl DS:7E41 glad.wins
c114: inc byte ptr [bx + 0x7e40]             ; tbl DS:7E40 glad.generation
c118: cmp byte ptr [bx + 0x7e44], 0x4        ; tbl DS:7E44 glad.style (4 = beast)
c11d: je 0xc13b
c11f: mov ax, 0x2
c122: push ax
c123: call 0x76b                             ; EXE rng_roll(n)
c126: mov bx, word ptr [bp - 0x2]
c129: mov byte ptr [bx + 0x7e45], al         ; tbl DS:7E45 glad.body
c12d: mov ax, 0x4
c130: push ax
c131: call 0x76b                             ; EXE rng_roll(n)
c134: mov bx, word ptr [bp - 0x2]
c137: mov byte ptr [bx + 0x7e44], al         ; tbl DS:7E44 glad.style (4 = beast)
c13b: mov bx, word ptr [bp - 0x2]
c13e: mov byte ptr [bx + 0x7e43], 0x0        ; tbl DS:7E43 glad.status
c143: mov sp, bp
c145: pop bp
c146: ret 0x2

; ---- func C149  size 0x5b  bp frame, locals 0, ret 0x2  callers: B7FE BB3B BDEA
c149: push bp
c14a: mov bp, sp
c14c: push si
c14d: mov ax, 0x91
c150: imul word ptr [bp + 0x4]
c153: mov cx, 0x1e
c156: cdq
c157: idiv cx
c159: mov si, ax
c15b: or si, si
c15d: jge 0xc163
c15f: sub si, si
c161: jmp 0xc16c
c163: cmp si, 0x91
c167: jle 0xc16c
c169: mov si, 0x91
c16c: mov word ptr [0x9afa], 0xf             ; fill colour
c172: mov ax, 0x52
c175: push ax
c176: mov cx, 0xd
c179: push cx
c17a: lea dx, [si + 0x52]
c17d: push dx
c17e: mov bx, 0x11
c181: push bx
c182: mov si, dx
c184: call 0x7e74                            ; EXE fill rect (colour, ...)
c187: mov ax, word ptr [0x6346]              ; box colour
c18a: mov word ptr [0x9afa], ax              ; fill colour
c18d: push si
c18e: mov ax, 0xd
c191: push ax
c192: mov ax, 0xdd
c195: push ax
c196: mov ax, 0x11
c199: push ax
c19a: call 0x7e74                            ; EXE fill rect (colour, ...)
c19d: pop si
c19e: mov sp, bp
c1a0: pop bp
c1a1: ret 0x2

; ---- func C1A4  size 0xbb  bp frame, locals 2, ret  callers: B0E0 B7FE BDEA
c1a4: push bp
c1a5: mov bp, sp
c1a7: sub sp, 0x2
c1aa: push si
c1ab: call 0x67e4                            ; EXE draw into the work buffer
c1ae: mov ax, 0xc0d0                         ; current target descriptor
c1b1: push ax
c1b2: mov ax, 0x7bc8                         ; point {0, 126} (arenabtm position)
c1b5: push ax
c1b6: mov ax, 0xc0be                         ; picture slot 4 descriptor (arenabtm.cbm)
c1b9: push ax
c1ba: call 0x8790                            ; EXE blit picture (8790(desc, dst, ...))
c1bd: mov si, word ptr [0xaa58]              ; P.state
c1c1: and si, 0xff
c1c5: shl si
c1c7: shl si
c1c9: mov bx, word ptr [0xaa52]              ; P.movetab
c1cd: mov al, byte ptr [bx + si + 0x2]
c1d0: mov byte ptr [bp - 0x1], al
c1d3: mov si, word ptr [0xaa70]              ; O.state
c1d7: and si, 0xff
c1db: shl si
c1dd: shl si
c1df: mov bx, word ptr [0xaa6a]              ; O.movetab
c1e3: mov al, byte ptr [bx + si + 0x2]
c1e6: mov byte ptr [bp - 0x2], al
c1e9: mov al, byte ptr [0xaa73]              ; O.dying state
c1ec: cmp byte ptr [0xaa70], al              ; O.state
c1f0: je 0xc21b
c1f2: mov al, 0xe
c1f4: mul byte ptr [bp - 0x2]
c1f7: add ax, word ptr [0xc2b8]              ; sheet opponent fighter
c1fb: push ax
c1fc: push word ptr [0xaa6c]                 ; O.x
c200: push word ptr [0xaa6e]                 ; O.y
c204: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
c207: mov al, 0xe
c209: mul byte ptr [bp - 0x1]
c20c: add ax, word ptr [0xc2b6]              ; sheet player fighter
c210: push ax
c211: push word ptr [0xaa54]                 ; P.x
c215: push word ptr [0xaa56]                 ; P.y
c219: jmp 0xc242
c21b: mov al, 0xe
c21d: mul byte ptr [bp - 0x1]
c220: add ax, word ptr [0xc2b6]              ; sheet player fighter
c224: push ax
c225: push word ptr [0xaa54]                 ; P.x
c229: push word ptr [0xaa56]                 ; P.y
c22d: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
c230: mov al, 0xe
c232: mul byte ptr [bp - 0x2]
c235: add ax, word ptr [0xc2b8]              ; sheet opponent fighter
c239: push ax
c23a: push word ptr [0xaa6c]                 ; O.x
c23e: push word ptr [0xaa6e]                 ; O.y
c242: call 0x8e62                            ; EXE sprite8e62(desc k of sheet, x, y)
c245: call 0x680f                            ; EXE target = the screen descriptor
c248: sub ax, ax
c24a: push ax
c24b: mov ax, 0x7e
c24e: push ax
c24f: mov ax, 0x13f
c252: push ax
c253: mov ax, 0xb9
c256: push ax
c257: call 0x6973                            ; EXE restore rect work buffer -> screen
c25a: pop si
c25b: mov sp, bp
c25d: pop bp
c25e: ret

; ---- func C25F  size 0x29  bp frame, locals 0, ret 0x2  callers: B7FE
c25f: push bp
c260: mov bp, sp
c262: push si
c263: mov word ptr [0x9afa], 0x8             ; fill colour
c269: cmp word ptr [bp + 0x4], 0x0
c26d: jne 0xc274
c26f: mov al, byte ptr [0xaa74]              ; O.health
c272: jmp 0xc277
c274: mov al, byte ptr [0xaa5c]              ; P.health
c277: cwde
c278: mov si, ax
c27a: push si
c27b: push word ptr [bp + 0x4]
c27e: call 0xc302
c281: pop si
c282: mov sp, bp
c284: pop bp
c285: ret 0x2

; ---- func C288  size 0x7a  bp frame, locals 0, ret 0x4  callers: B0E0
c288: push bp
c289: mov bp, sp
c28b: push si
c28c: mov al, 0xa
c28e: mul byte ptr [bp + 0x6]
c291: mov bx, ax
c293: mov al, byte ptr [bx + 0x7e46]         ; tbl DS:7E46 glad.training
c297: mov byte ptr [0xaa5a], al              ; P.training
c29a: mov al, 0xa
c29c: mul byte ptr [bp + 0x4]
c29f: mov si, ax
c2a1: mov al, byte ptr [si + 0x7e46]         ; tbl DS:7E46 glad.training
c2a5: mov byte ptr [0xaa72], al              ; O.training
c2a8: sub al, al
c2aa: mov byte ptr [0xaa70], al              ; O.state
c2ad: mov byte ptr [0xaa58], al              ; P.state
c2b0: mov bl, byte ptr [bx + 0x7e45]         ; tbl DS:7E45 glad.body
c2b4: sub bh, bh
c2b6: mov ax, bx
c2b8: shl bx
c2ba: mov cx, word ptr [bx + 0x7fe8]         ; tbl DS:7FE8 player move tables
c2be: mov word ptr [0xaa52], cx              ; P.movetab
c2c2: mov bl, byte ptr [si + 0x7e45]         ; tbl DS:7E45 glad.body
c2c6: sub bh, bh
c2c8: mov cx, bx
c2ca: shl bx
c2cc: mov dx, word ptr [bx + 0x7fec]         ; tbl DS:7FEC opponent move tables
c2d0: mov word ptr [0xaa6a], dx              ; O.movetab
c2d4: mov bx, ax
c2d6: mov al, byte ptr [bx + 0x7ed0]         ; tbl DS:7ED0 player dying state by body
c2da: mov byte ptr [0xaa5b], al              ; P.dying state
c2dd: mov bx, cx
c2df: mov al, byte ptr [bx + 0x7ed2]         ; tbl DS:7ED2 opponent dying state by body
c2e3: mov byte ptr [0xaa73], al              ; O.dying state
c2e6: mov word ptr [0xaa54], 0xc0            ; P.x
c2ec: mov word ptr [0xaa6c], 0x7f            ; O.x
c2f2: mov ax, 0xaf
c2f5: mov word ptr [0xaa6e], ax              ; O.y
c2f8: mov word ptr [0xaa56], ax              ; P.y
c2fb: pop si
c2fc: mov sp, bp
c2fe: pop bp
c2ff: ret 0x4

; ---- func C302  size 0x83  bp frame, locals 0, ret 0x4  callers: C25F
c302: push bp
c303: mov bp, sp
c305: push di
c306: push si
c307: mov si, 0x8
c30a: cmp word ptr [0x9b26], 0x2             ; video mode
c30f: jne 0xc314
c311: mov si, 0x36
c314: mov ax, word ptr [bp + 0x6]
c317: or ax, ax
c319: jge 0xc31d
c31b: sub ax, ax
c31d: mov word ptr [bp + 0x6], ax
c320: mov cl, 0x7
c322: shl ax, cl
c324: mov cx, 0x32
c327: cdq
c328: idiv cx
c32a: mov di, ax
c32c: mov word ptr [0x9afa], 0x4             ; fill colour
c332: cmp word ptr [bp + 0x4], 0x0
c336: jne 0xc355
c338: sub ax, ax
c33a: push ax
c33b: mov ax, 0xbc
c33e: push ax
c33f: push di
c340: mov cx, 0xc1
c343: push cx
c344: call 0x7e74                            ; EXE fill rect (colour, ...)
c347: mov word ptr [0x9afa], si              ; fill colour
c34b: push di
c34c: mov ax, 0xbc
c34f: push ax
c350: mov ax, 0x80
c353: jmp 0xc375
c355: mov ax, 0x13f
c358: push ax
c359: mov cx, 0xbc
c35c: push cx
c35d: sub ax, di
c35f: push ax
c360: mov dx, 0xc1
c363: push dx
c364: mov di, ax
c366: call 0x7e74                            ; EXE fill rect (colour, ...)
c369: mov word ptr [0x9afa], si              ; fill colour
c36d: push di
c36e: mov ax, 0xbc
c371: push ax
c372: mov ax, 0xbf
c375: push ax
c376: mov ax, 0xc1
c379: push ax
c37a: call 0x7e74                            ; EXE fill rect (colour, ...)
c37d: pop si
c37e: pop di
c37f: mov sp, bp
c381: pop bp
c382: ret 0x4

; ---- section padding
c385: db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
