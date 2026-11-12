; OVL0 section 1 (file 0x0000-0x09A0, 0x9A0 bytes, loaded at 1331:0000 = 0823:B0E0-0xBA80): the credits part only,
;   B8B7..BA74 (file 0x07D7-0x0995; the bytes before B8B7 end in pop bp / ret, the bytes after BA74 are the section's
;   zero padding). Bounds from the PLINK86 overlay table at 1322:0000 (= 0823:AFF0 in disasm/EXE_RUNTIME.bin): section 1
;   load segment 1331, end segment 13CB, file position 0 (section 2's +0). Section 1 also holds the start-up (B0E0 command
;   line, B18A init, B466 copy-protection question, B694 intro); not listed here.
; Entry: B9A1 (EXE thunk B0CE = 9a ec 02 9b 12 / dw 1 / jmp 0823:B9A1, called from section 4 DF70, the victory).
;   B970 (the intro's credits, the first 11 pairs) has no thunk: called inside section 1.
; llvm-objdump code16 listing (as OVL_GLAD.asm), addresses are 0823 offsets; call targets >= 0x10000 are EXE routines
;   (masked: 0x100d7 = EXE 00D7). [-0x64da] = DS:9B26, [-0x6497] = DS:9B69. Pascal arguments: the first pushed is the
;   first parameter. Port: port/src/credits.c.


; ---- func B8B7  B8B7: the wait poll: B8FE in VGA, then -1 on a key
b8b7: cmp word ptr [-0x64da], 0x2              ; [9B26] graphics mode: 2 = VGA
b8bc: jne 0xb8c1
b8be: call 0xb8fe                              ; B8FE colour cycle
b8c1: call 0x03aa                              ; EXE 03AA key_read
b8c4: or al, al
b8c6: je 0xb8cc
b8c8: mov ax, 0xffff
b8cb: ret
b8cc: sub ax, ax
b8ce: ret

; ---- func B8CF  B8CF: the five cycle colours DS:6ABA into DAC A2..A6 (VGA only; intro)
b8cf: push di
b8d0: push si
b8d1: cmp word ptr [-0x64da], 0x2
b8d6: jne 0xb8fb
b8d8: call 0x84bb                              ; EXE 84BB wait_vretrace
b8db: mov si, 0x6aba
b8de: mov di, 0x6ad8
b8e1: cmp di, 0x6ae2
b8e5: jae 0xb8fb
b8e7: push word ptr [di]
b8e9: push word ptr [si]
b8eb: push word ptr [si + 0x2]
b8ee: push word ptr [si + 0x4]
b8f1: call 0x01b6                              ; EXE 01B6 set_dac(i, r, g, b)
b8f4: add si, 0x6
b8f7: inc di
b8f8: inc di
b8f9: jmp 0xb8e1
b8fb: pop si
b8fc: pop di
b8fd: ret

; ---- func B8FE  B8FE: every 4 ticks the colours one place on (counter DS:6AE6)
b8fe: push di
b8ff: push si
b900: mov ax, word ptr [0x6ae2]                ; DS:6AE2/6AE4 last cycle time
b903: mov dx, word ptr [0x6ae4]
b907: add ax, 0x4
b90a: adc dx, 0x0
b90d: cmp dx, word ptr [0x44]                  ; DS:44 timer high
b911: ja 0xb96d
b913: jb 0xb91b
b915: cmp ax, word ptr [0x42]                  ; DS:42 timer low
b919: jae 0xb96d
b91b: inc byte ptr [0x6ae6]                    ; DS:6AE6 cycle counter 0..4
b91f: cmp byte ptr [0x6ae6], 0x4
b924: jbe 0xb92b
b926: mov byte ptr [0x6ae6], 0x0
b92b: call 0x84bb                              ; EXE 84BB wait_vretrace
b92e: sub di, di
b930: mov si, 0x6aba
b933: jmp 0xb959
b935: mov al, byte ptr [0x6ae6]
b938: sub ah, ah
b93a: add ax, di
b93c: mov cx, 0x4                              ; mod 4 (bug: 5 colours, counter 0..4)
b93f: cdq
b940: idiv cx
b942: mov bx, dx
b944: shl bx
b946: push word ptr [bx + 0x6ad8]              ; DS:6AD8 DAC entries A2..A6
b94a: push word ptr [si]                       ; DS:6ABA five r, g, b triples
b94c: push word ptr [si + 0x2]
b94f: push word ptr [si + 0x4]
b952: call 0x01b6                              ; EXE 01B6 set_dac(i, r, g, b)
b955: add si, 0x6
b958: inc di
b959: cmp si, 0x6ad8
b95d: jb 0xb935
b95f: mov ax, word ptr [0x42]
b962: mov dx, word ptr [0x44]
b966: mov word ptr [0x6ae2], ax
b969: mov word ptr [0x6ae4], dx
b96d: pop si
b96e: pop di
b96f: ret

; ---- func B970  B970: the intro credits (11 pairs, 288 ticks each); -1 on a key
b970: push di
b971: push si
b972: sub di, di
b974: mov ax, 0x54
b977: push ax
b978: mov ax, 0x28
b97b: push ax
b97c: call 0x30e6                              ; EXE 30E6 frame(x 84, y 40)
b97f: mov si, 0x6a6e                           ; DS:6A6E credits pairs (title, names)
b982: jmp 0xb996
b984: push si
b985: mov ax, 0x120
b988: push ax
b989: call 0xb9d3                              ; B9D3(pair, 288)
b98c: mov di, ax
b98e: cmp di, -0x1
b991: je 0xb99c
b993: add si, 0x4
b996: cmp si, 0x6a9a                           ; the first 11 pairs only
b99a: jb 0xb984
b99c: mov ax, di
b99e: pop si
b99f: pop di
b9a0: ret

; ---- func B9A1  B9A1: ENTRY (thunk B0CE): the end credits, 18 pairs 216 ticks each, then the quote 360 ticks
b9a1: push si
b9a2: mov ax, 0x54
b9a5: push ax
b9a6: mov ax, 0x28
b9a9: push ax
b9aa: call 0x30e6                              ; EXE 30E6 frame(x 84, y 40)
b9ad: mov si, 0x6a6e                           ; DS:6A6E credits pairs (title, names)
b9b0: jmp 0xb9c0
b9b2: push si
b9b3: mov ax, 0xd8
b9b6: push ax
b9b7: call 0xb9d3                              ; B9D3(pair, 216)
b9ba: inc ax
b9bb: je 0xb9c6
b9bd: add si, 0x4
b9c0: cmp si, 0x6ab6                           ; 18 pairs
b9c4: jb 0xb9b2
b9c6: mov ax, 0x6ab6                           ; DS:6AB6 ("", "\"Beware the ides of March\"")
b9c9: push ax
b9ca: mov ax, 0x168
b9cd: push ax
b9ce: call 0xb9d3                              ; B9D3(6AB6, 360)
b9d1: pop si
b9d2: ret

; ---- func B9D3  B9D3(pair [bp+6], ticks [bp+4]): one credit; -1 when a key ended the wait
b9d3: push bp
b9d4: mov bp, sp
b9d6: sub sp, 0x12
b9d9: push di
b9da: push si
b9db: mov word ptr [bp - 0x2], 0x0
b9e0: mov bx, word ptr [bp + 0x6]
b9e3: push word ptr [bx + 0x2]
b9e6: push word ptr [bx]
b9e8: call 0x8c58                              ; EXE 8C58 text_width(title); the names stay pushed
b9eb: push ax
b9ec: sub ax, ax
b9ee: push ax
b9ef: mov cx, 0x9f
b9f2: push cx
b9f3: mov dx, 0x63
b9f6: push dx
b9f7: lea dx, [bp - 0x12]
b9fa: push dx
b9fb: call 0x8c9f                              ; EXE 8C9F text_rect(names, minw, 0, 159, 99, &buf)
b9fe: lea di, [bp - 0xa]
ba01: mov si, ax
ba03: push ss
ba04: pop es
ba05: movsw word ptr es:[di], word ptr [si]
ba06: movsw word ptr es:[di], word ptr [si]
ba07: movsw word ptr es:[di], word ptr [si]
ba08: movsw word ptr es:[di], word ptr [si]
ba09: mov word ptr [-0x6497], 0x1              ; DS:9B69 text colour = 1
ba0f: mov bx, word ptr [bp + 0x6]
ba12: push word ptr [bx]
ba14: mov ax, 0x9f
ba17: push ax
ba18: mov cx, 0x2f
ba1b: push cx
ba1c: call 0x8be4                              ; EXE 8BE4 centred_text(title, 159, 47)
ba1f: mov word ptr [-0x6497], 0x0              ; DS:9B69 text colour = 0
ba25: mov bx, word ptr [bp + 0x6]
ba28: push word ptr [bx + 0x2]
ba2b: mov ax, 0x9f
ba2e: push ax
ba2f: push word ptr [bp - 0x8]
ba32: call 0x8be4                              ; EXE 8BE4 centred_text(names, 159, y0)
ba35: mov ax, word ptr [0x6346]                ; DS:6346 frame fill
ba38: mov word ptr [0x9afa], ax                ; DS:9AFA fill colour
ba3b: mov ax, 0x1
ba3e: push ax
ba3f: call 0x00d7                              ; EXE 00D7 timer_start(1)
ba42: mov ax, 0x1
ba45: push ax
ba46: call 0x0183                              ; EXE 0183 ticks_since(1)
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
ba67: call 0x7e74                              ; EXE 7E74 fill(x0, 47, x1, y1)
ba6a: mov ax, word ptr [bp - 0x2]
ba6d: pop si
ba6e: pop di
ba6f: mov sp, bp
ba71: pop bp
ba72: ret 0x4
