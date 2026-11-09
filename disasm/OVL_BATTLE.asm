; OVL0 section 2 (file 0x09A0-0x3EF0) = land battle, loaded at 1331:0000 = 0823:B0E0. Entry 0xDF24 (EXE thunk 0xB06E).
; Linear llvm-objdump listing (code16), addresses are 0823 offsets; data inside code may decode as junk.
b0e0: push bp
b0e1: mov bp, sp
b0e3: sub sp, 0xe
b0e6: push di
b0e7: push si
b0e8: mov di, word ptr [bp + 0x4]
b0eb: mov ax, di
b0ed: mov cx, 0x12
b0f0: mul cx
b0f2: mov bx, ax
b0f4: mov word ptr [bp - 0xc], bx
b0f7: mov si, word ptr [-0x3d6e]
b0fb: cmp word ptr [bx + si], 0x0
b0fe: jne 0xb103
b100: jmp 0xb311
b103: add bx, si
b105: cmp byte ptr [bx + 0xa], 0x1
b109: je 0xb116
b10b: mov bx, word ptr [bp - 0xc]
b10e: add bx, si
b110: cmp byte ptr [bx + 0xd], 0x3
b114: je 0xb14e
b116: mov bx, word ptr [bp - 0xc]
b119: add bx, si
b11b: cmp byte ptr [bx + 0xd], 0x4
b11f: je 0xb14e
b121: mov bx, word ptr [bp - 0xc]
b124: add bx, si
b126: mov ax, bx
b128: mov bl, byte ptr [bx + 0xa]
b12b: sub bh, bh
b12d: mov cl, byte ptr [bx + 0x6ae8]
b131: mov byte ptr [bp - 0x7], cl
b134: push di
b135: mov bx, ax
b137: mov al, byte ptr [bx + 0xe]
b13a: push ax
b13b: lea ax, [bp - 0xa]
b13e: push ax
b13f: call 0xc948
b142: mov byte ptr [bp - 0x3], 0x0
b146: mov al, byte ptr [bp - 0x7]
b149: cmp byte ptr [bp - 0x3], al
b14c: jb 0xb151
b14e: jmp 0xb311
b151: mov bl, byte ptr [bp - 0x3]
b154: sub bh, bh
b156: lea ax, [bp - 0xa]
b159: add bx, ax
b15b: mov al, byte ptr [bx]
b15d: sub ah, ah
b15f: mov si, ax
b161: or al, al
b163: jne 0xb168
b165: jmp 0xb30b
b168: cmp al, -0x1
b16a: je 0xb170
b16c: cmp al, -0x2
b16e: jne 0xb173
b170: jmp 0xb30b
b173: mov word ptr [bp - 0x6], 0x0
b178: mov cx, 0x12
b17b: mul cx
b17d: mov bx, ax
b17f: mov word ptr [bp - 0xe], bx
b182: add bx, word ptr [-0x3d6e]
b186: mov al, byte ptr [bx + 0xb]
b189: mov bx, word ptr [bp - 0xc]
b18c: add bx, word ptr [-0x3d6e]
b190: cmp byte ptr [bx + 0xb], al
b193: je 0xb19a
b195: mov word ptr [bp - 0x6], 0xffff
b19a: mov bx, word ptr [bp - 0xc]
b19d: add bx, word ptr [-0x3d6e]
b1a1: cmp byte ptr [bx + 0xa], 0x1
b1a5: jne 0xb1ac
b1a7: mov word ptr [bp - 0x6], 0xffff
b1ac: cmp word ptr [bp - 0x6], 0x0
b1b0: jne 0xb1b5
b1b2: jmp 0xb30b
b1b5: push di
b1b6: mov al, 0x3
b1b8: push ax
b1b9: call 0xb925
b1bc: mov bx, word ptr [bp - 0xc]
b1bf: add bx, word ptr [-0x3d6e]
b1c3: cmp byte ptr [bx + 0xa], 0x1
b1c7: jne 0xb211
b1c9: mov bx, word ptr [bp - 0xc]
b1cc: add bx, word ptr [-0x3d6e]
b1d0: mov bl, byte ptr [bx + 0xb]
b1d3: sub bh, bh
b1d5: shl bx
b1d7: cmp word ptr [bx - 0x3d72], 0x0
b1dc: je 0xb211
b1de: push si
b1df: call 0xb7be
b1e2: sub ah, ah
b1e4: push ax
b1e5: mov ax, 0x19
b1e8: push ax
b1e9: sub ax, ax
b1eb: push ax
b1ec: call 0x50f
b1ef: mov byte ptr [bp - 0x1], al
b1f2: mov ax, 0x64
b1f5: push ax
b1f6: call 0x76b
b1f9: mov cl, byte ptr [bp - 0x1]
b1fc: sub ch, ch
b1fe: cmp ax, cx
b200: jbe 0xb211
b202: mov bx, word ptr [bp - 0xe]
b205: add bx, word ptr [-0x3d6e]
b209: mov byte ptr [bx + 0xd], 0x3
b20d: push si
b20e: call 0xd08e
b211: mov bx, word ptr [bp - 0xe]
b214: add bx, word ptr [-0x3d6e]
b218: cmp byte ptr [bx + 0x10], 0x0
b21c: jne 0xb267
b21e: mov bx, word ptr [bp - 0xc]
b221: add bx, word ptr [-0x3d6e]
b225: cmp byte ptr [bx + 0xd], 0x3
b229: jae 0xb267
b22b: mov bx, word ptr [bp - 0xc]
b22e: add bx, word ptr [-0x3d6e]
b232: mov al, byte ptr [bx + 0xe]
b235: sub al, -0x2
b237: and al, 0x3
b239: mov byte ptr [bp - 0x1], al
b23c: mov bx, word ptr [bp - 0xe]
b23f: add bx, word ptr [-0x3d6e]
b243: cmp al, byte ptr [bx + 0xe]
b246: je 0xb267
b248: mov bx, word ptr [bp - 0xe]
b24b: add bx, word ptr [-0x3d6e]
b24f: mov word ptr [bx + 0x4], 0xffff
b254: mov cl, 0x6
b256: mov al, byte ptr [bp - 0x1]
b259: shl al, cl
b25b: or al, 0x1
b25d: mov bx, word ptr [bp - 0xe]
b260: add bx, word ptr [-0x3d6e]
b264: mov byte ptr [bx + 0x6], al
b267: mov bx, si
b269: shl bx
b26b: add bx, word ptr [-0x3d6a]
b26f: mov ax, word ptr [bx]
b271: mov word ptr [bp - 0x2], ax
b274: mov bx, word ptr [bp - 0xe]
b277: add bx, word ptr [-0x3d6e]
b27b: cmp byte ptr [bx + 0xa], 0x3
b27f: jne 0xb2c7
b281: mov bx, word ptr [bp - 0xe]
b284: add bx, word ptr [-0x3d6e]
b288: dec word ptr [bx]
b28a: mov bx, word ptr [bp - 0xe]
b28d: add bx, word ptr [-0x3d6e]
b291: cmp word ptr [bx], 0x0
b294: jne 0xb2fe
b296: push si
b297: mov al, 0x4
b299: push ax
b29a: call 0xb925
b29d: mov bx, word ptr [bp - 0xe]
b2a0: add bx, word ptr [-0x3d6e]
b2a4: mov al, byte ptr [bx + 0xb]
b2a7: push ax
b2a8: call 0xb4e5
b2ab: mov bx, word ptr [bp - 0xe]
b2ae: add bx, word ptr [-0x3d6e]
b2b2: cmp byte ptr [bx + 0xb], 0x0
b2b6: jne 0xb2fe
b2b8: mov al, 0x6
b2ba: mul byte ptr [-0x3d74]
b2be: mov bx, ax
b2c0: mov byte ptr [bx + 0x1a20], 0x2
b2c5: jmp 0xb2fe
b2c7: mov bx, word ptr [bp - 0xc]
b2ca: add bx, word ptr [-0x3d6e]
b2ce: cmp byte ptr [bx + 0xa], 0x3
b2d2: jne 0xb2db
b2d4: mov word ptr [bp - 0x6], 0x0
b2d9: jmp 0xb2f1
b2db: mov ax, word ptr [bp - 0xc]
b2de: add ax, word ptr [-0x3d6e]
b2e2: push ax
b2e3: mov ax, word ptr [bp - 0xe]
b2e6: add ax, word ptr [-0x3d6e]
b2ea: push ax
b2eb: call 0xb319
b2ee: mov word ptr [bp - 0x6], ax
b2f1: mov ax, word ptr [bp - 0x6]
b2f4: sub word ptr [bp - 0x2], ax
b2f7: jns 0xb2fe
b2f9: mov word ptr [bp - 0x2], 0x0
b2fe: mov bx, si
b300: shl bx
b302: mov si, word ptr [-0x3d6a]
b306: mov ax, word ptr [bp - 0x2]
b309: mov word ptr [bx + si], ax
b30b: inc byte ptr [bp - 0x3]
b30e: jmp 0xb146
b311: pop si
b312: pop di
b313: mov sp, bp
b315: pop bp
b316: ret 0x2
b319: push bp
b31a: mov bp, sp
b31c: sub sp, 0xa
b31f: push di
b320: push si
b321: mov word ptr [bp - 0x4], 0x0
b326: mov bx, word ptr [bp + 0x6]
b329: mov si, word ptr [bx + 0xa]
b32c: and si, 0xff
b330: mov ax, si
b332: shl si
b334: add si, ax
b336: mov di, word ptr [bp + 0x4]
b339: mov bl, byte ptr [bx + 0xe]
b33c: sub bl, byte ptr [di + 0xe]
b33f: mov al, byte ptr [di + 0xa]
b342: sub ah, ah
b344: add si, ax
b346: shl si
b348: shl si
b34a: and bx, 0x3
b34d: mov al, byte ptr [bx + si + 0x6af0]
b351: mov byte ptr [bp - 0x1], al
b354: mov al, byte ptr [0x48e]
b357: or ax, ax
b359: je 0xb364
b35b: dec ax
b35c: dec ax
b35d: je 0xb36c
b35f: dec ax
b360: je 0xb389
b362: jmp 0xb3a4
b364: mov bx, di
b366: cmp byte ptr [bx + 0xb], 0x0
b36a: jmp 0xb38f
b36c: mov bx, di
b36e: cmp byte ptr [bx + 0xb], 0x1
b372: jne 0xb3a4
b374: mov al, 0x5
b376: mul byte ptr [bp - 0x1]
b379: cdq
b37a: xor ax, dx
b37c: sub ax, dx
b37e: mov cx, 0x2
b381: sar ax, cl
b383: xor ax, dx
b385: sub ax, dx
b387: jmp 0xb3a1
b389: mov bx, di
b38b: cmp byte ptr [bx + 0xb], 0x1
b38f: jne 0xb3a4
b391: mov al, byte ptr [bp - 0x1]
b394: sub ah, ah
b396: mov cx, ax
b398: shl ax
b39a: add ax, cx
b39c: cdq
b39d: sub ax, dx
b39f: sar ax
b3a1: mov byte ptr [bp - 0x1], al
b3a4: mov ax, 0x40
b3a7: mov cl, byte ptr [bp - 0x1]
b3aa: sub ch, ch
b3ac: cdq
b3ad: idiv cx
b3af: mov word ptr [bp - 0x8], ax
b3b2: mov bx, word ptr [bp + 0x6]
b3b5: mov ax, word ptr [bx]
b3b7: mov bl, byte ptr [bx + 0xa]
b3ba: sub bh, bh
b3bc: mov cl, byte ptr [bx + 0x6b14]
b3c0: cdq
b3c1: idiv cx
b3c3: inc ax
b3c4: mov word ptr [bp - 0xa], ax
b3c7: mov word ptr [bp - 0x6], 0x0
b3cc: mov di, word ptr [bp - 0x4]
b3cf: jmp 0xb3de
b3d1: mov ax, word ptr [bp - 0x8]
b3d4: inc ax
b3d5: push ax
b3d6: call 0x76b
b3d9: add di, ax
b3db: inc word ptr [bp - 0x6]
b3de: mov ax, word ptr [bp - 0xa]
b3e1: cmp word ptr [bp - 0x6], ax
b3e4: jb 0xb3d1
b3e6: mov bx, word ptr [bp + 0x4]
b3e9: cmp byte ptr [bx + 0xa], 0x1
b3ed: je 0xb3fb
b3ef: mov ax, di
b3f1: cmp ax, 0x1
b3f4: jae 0xb3f9
b3f6: mov ax, 0x1
b3f9: mov di, ax
b3fb: mov ax, di
b3fd: pop si
b3fe: pop di
b3ff: mov sp, bp
b401: pop bp
b402: ret 0x4
b405: push bp
b406: mov bp, sp
b408: sub sp, 0x6
b40b: push di
b40c: push si
b40d: mov di, 0x1
b410: mov word ptr [bp - 0x4], 0x2
b415: mov si, 0x12
b418: jmp 0xb4d6
b41b: mov bx, word ptr [-0x3d6e]
b41f: add bx, si
b421: mov word ptr [bp - 0x6], bx
b424: cmp word ptr [bx], 0x0
b427: je 0xb42f
b429: cmp byte ptr [bx + 0xa], 0x3
b42d: jne 0xb432
b42f: jmp 0xb4ce
b432: mov ax, word ptr [bx]
b434: mov bx, word ptr [-0x3d6a]
b438: add bx, word ptr [bp - 0x4]
b43b: mov cx, ax
b43d: sub ax, word ptr [bx]
b43f: mov word ptr [bp - 0x2], ax
b442: mov dx, cx
b444: mov cl, 0x4
b446: shl ax, cl
b448: cmp ax, dx
b44a: jle 0xb488
b44c: mov ax, word ptr [bp - 0x2]
b44f: cdq
b450: sub ax, dx
b452: sar ax
b454: mov bx, word ptr [bp - 0x6]
b457: sub al, byte ptr [bx + 0xc]
b45a: neg al
b45c: or al, al
b45e: jge 0xb462
b460: sub al, al
b462: mov byte ptr [bx + 0xc], al
b465: mov bx, word ptr [-0x3d6e]
b469: cmp byte ptr [bx + si + 0xc], 0x32
b46d: jge 0xb4af
b46f: push di
b470: call 0xb7be
b473: sub ah, ah
b475: mov word ptr [bp - 0x2], ax
b478: mov ax, 0x64
b47b: push ax
b47c: call 0x76b
b47f: mov cx, word ptr [bp - 0x2]
b482: shl cx
b484: cmp ax, cx
b486: jmp 0xb4a1
b488: mov bx, word ptr [bp - 0x6]
b48b: cmp byte ptr [bx + 0xa], 0x1
b48f: jne 0xb4af
b491: cmp word ptr [bp - 0x2], 0x0
b495: je 0xb4af
b497: mov ax, 0x64
b49a: push ax
b49b: call 0x76b
b49e: cmp ax, 0x4b
b4a1: jbe 0xb4af
b4a3: mov bx, word ptr [-0x3d6e]
b4a7: mov byte ptr [bx + si + 0xd], 0x3
b4ab: push di
b4ac: call 0xd08e
b4af: mov bx, word ptr [-0x3d6a]
b4b3: add bx, word ptr [bp - 0x4]
b4b6: mov ax, word ptr [bx]
b4b8: mov bx, word ptr [-0x3d6e]
b4bc: mov word ptr [bx + si], ax
b4be: mov bx, word ptr [-0x3d6e]
b4c2: cmp word ptr [bx + si], 0x0
b4c5: jne 0xb4ce
b4c7: push di
b4c8: mov al, 0x4
b4ca: push ax
b4cb: call 0xb925
b4ce: add word ptr [bp - 0x4], 0x2
b4d2: add si, 0x12
b4d5: inc di
b4d6: cmp word ptr [-0x3d6c], di
b4da: jbe 0xb4df
b4dc: jmp 0xb41b
b4df: pop si
b4e0: pop di
b4e1: mov sp, bp
b4e3: pop bp
b4e4: ret
b4e5: push bp
b4e6: mov bp, sp
b4e8: sub sp, 0x2
b4eb: push di
b4ec: push si
b4ed: mov di, 0x1
b4f0: mov si, 0x12
b4f3: jmp 0xb53d
b4f5: mov bx, word ptr [-0x3d6e]
b4f9: add bx, si
b4fb: mov word ptr [bp - 0x2], bx
b4fe: mov al, byte ptr [bp + 0x4]
b501: cmp byte ptr [bx + 0xb], al
b504: jne 0xb539
b506: cmp byte ptr [bx + 0xd], 0x4
b50a: je 0xb539
b50c: mov al, byte ptr [bx + 0xc]
b50f: cwde
b510: push ax
b511: mov ax, 0xf
b514: push ax
b515: sub ax, ax
b517: push ax
b518: call 0x50f
b51b: mov bx, word ptr [bp - 0x2]
b51e: mov byte ptr [bx + 0xc], al
b521: mov ax, 0x64
b524: push ax
b525: call 0x76b
b528: mov bx, word ptr [-0x3d6e]
b52c: cmp byte ptr [bx + si + 0xc], al
b52f: jge 0xb539
b531: mov byte ptr [bx + si + 0xd], 0x3
b535: push di
b536: call 0xd08e
b539: add si, 0x12
b53c: inc di
b53d: cmp word ptr [-0x3d6c], di
b541: ja 0xb4f5
b543: pop si
b544: pop di
b545: mov sp, bp
b547: pop bp
b548: ret 0x2
b54b: add byte ptr [di - 0x75], dl
b54e: in al, dx
b54f: sub sp, 0x4
b552: push di
b553: push si
b554: sub di, di
b556: cmp word ptr [bp + 0x4], 0xf
b55a: jbe 0xb5db
b55c: mov dx, 0x1
b55f: mov bx, word ptr [-0x3d6e]
b563: add bx, 0x1f
b566: jmp 0xb572
b568: cmp byte ptr [bx], 0x4
b56b: je 0xb56e
b56d: inc di
b56e: add bx, 0x12
b571: inc dx
b572: cmp word ptr [-0x3d6c], dx
b576: ja 0xb568
b578: mov ax, di
b57a: cmp ax, 0xd
b57d: jbe 0xb582
b57f: mov ax, 0xd
b582: mov di, ax
b584: dec ax
b585: and al, -0x4
b587: add ax, 0x6b18
b58a: mov word ptr [bp - 0x2], ax
b58d: mov ax, 0x12
b590: mul word ptr [bp + 0x6]
b593: mov bx, ax
b595: mov word ptr [bp - 0x4], bx
b598: add bx, word ptr [-0x3d6e]
b59c: cmp byte ptr [bx + 0xd], 0x1
b5a0: jne 0xb5db
b5a2: mov ax, 0x64
b5a5: push ax
b5a6: call 0x76b
b5a9: mov bx, word ptr [bp - 0x4]
b5ac: add bx, word ptr [-0x3d6e]
b5b0: mov cx, ax
b5b2: mov al, byte ptr [bx + 0xc]
b5b5: mov bl, 0x19
b5b7: cwde
b5b8: idiv bl
b5ba: cwde
b5bb: mov bx, ax
b5bd: mov si, word ptr [bp - 0x2]
b5c0: mov al, byte ptr [bx + si]
b5c2: sub ah, ah
b5c4: mov dl, byte ptr [-0x3d5d]
b5c8: sub dh, dh
b5ca: sub dx, ax
b5cc: cmp dx, cx
b5ce: jae 0xb5db
b5d0: mov si, word ptr [bp - 0x4]
b5d3: mov bx, word ptr [-0x3d6e]
b5d7: mov byte ptr [bx + si + 0xd], 0x2
b5db: pop si
b5dc: pop di
b5dd: mov sp, bp
b5df: pop bp
b5e0: ret 0x4
b5e3: push bp
b5e4: mov bp, sp
b5e6: sub sp, 0x10
b5e9: push di
b5ea: push si
b5eb: mov word ptr [bp - 0xc], 0x0
b5f0: mov word ptr [bp - 0x4], 0xffff
b5f5: mov di, 0x1
b5f8: mov si, word ptr [-0x3d6e]
b5fc: add si, 0x1f
b5ff: jmp 0xb676
b601: mov al, byte ptr [si - 0x2]
b604: mov cx, ax
b606: mov ax, 0x12
b609: mul word ptr [bp + 0x4]
b60c: mov bx, ax
b60e: mov word ptr [bp - 0x10], bx
b611: add bx, word ptr [-0x3d6e]
b615: cmp byte ptr [bx + 0xb], cl
b618: je 0xb672
b61a: cmp byte ptr [si], 0x4
b61d: je 0xb672
b61f: cmp byte ptr [si], 0x3
b622: je 0xb672
b624: mov al, byte ptr [si - 0xa]
b627: cwde
b628: mov bx, word ptr [bp - 0x10]
b62b: add bx, word ptr [-0x3d6e]
b62f: mov cx, ax
b631: mov al, byte ptr [bx + 0x3]
b634: cwde
b635: mov dx, cx
b637: mov cx, ax
b639: sub cx, dx
b63b: mov ax, cx
b63d: mul cx
b63f: mov cx, ax
b641: mov al, byte ptr [si - 0xb]
b644: cwde
b645: mov dx, ax
b647: mov al, byte ptr [bx + 0x2]
b64a: cwde
b64b: mov bx, ax
b64d: sub bx, dx
b64f: mov ax, bx
b651: mul bx
b653: add cx, ax
b655: mov word ptr [bp - 0x2], cx
b658: cmp cx, word ptr [bp - 0x4]
b65b: jb 0xb66a
b65d: mov ax, cx
b65f: cmp word ptr [bp - 0x4], ax
b662: jne 0xb672
b664: cmp byte ptr [si - 0x3], 0x3
b668: jne 0xb672
b66a: mov word ptr [bp - 0xc], di
b66d: mov ax, cx
b66f: mov word ptr [bp - 0x4], ax
b672: add si, 0x12
b675: inc di
b676: cmp word ptr [-0x3d6c], di
b67a: ja 0xb601
b67c: mov di, word ptr [bp + 0x4]
b67f: cmp word ptr [bp - 0xc], 0x0
b683: jne 0xb688
b685: jmp 0xb7a5
b688: mov ax, 0x12
b68b: mul word ptr [bp - 0xc]
b68e: mov bx, word ptr [-0x3d6e]
b692: add bx, ax
b694: mov al, byte ptr [bx + 0x2]
b697: cwde
b698: mov cx, ax
b69a: mov ax, di
b69c: mov si, 0x12
b69f: mul si
b6a1: mov si, ax
b6a3: mov word ptr [bp - 0x10], si
b6a6: add si, word ptr [-0x3d6e]
b6aa: mov al, byte ptr [si + 0x2]
b6ad: cwde
b6ae: sub ax, cx
b6b0: mov word ptr [bp - 0xa], ax
b6b3: mov cx, ax
b6b5: mov al, byte ptr [bx + 0x3]
b6b8: cwde
b6b9: mov dx, ax
b6bb: mov al, byte ptr [si + 0x3]
b6be: cwde
b6bf: sub ax, dx
b6c1: mov word ptr [bp - 0x4], ax
b6c4: mov dx, ax
b6c6: mov ax, cx
b6c8: mov bx, dx
b6ca: cdq
b6cb: xor ax, dx
b6cd: sub ax, dx
b6cf: inc ax
b6d0: cdq
b6d1: sub ax, dx
b6d3: sar ax
b6d5: mov byte ptr [bp - 0x5], al
b6d8: mov ax, bx
b6da: cdq
b6db: xor ax, dx
b6dd: sub ax, dx
b6df: inc ax
b6e0: cdq
b6e1: sub ax, dx
b6e3: sar ax
b6e5: mov byte ptr [bp - 0x6], al
b6e8: mov byte ptr [bp - 0x7], 0x0
b6ec: or cx, cx
b6ee: jle 0xb6f4
b6f0: mov byte ptr [bp - 0x7], 0x2
b6f4: mov byte ptr [bp - 0x8], 0x1
b6f8: or bx, bx
b6fa: jle 0xb700
b6fc: mov byte ptr [bp - 0x8], 0x3
b700: mov cl, 0x6
b702: mov al, byte ptr [bp - 0x7]
b705: shl al, cl
b707: or al, byte ptr [bp - 0x5]
b70a: mov byte ptr [bp - 0x1], al
b70d: mov al, byte ptr [bp - 0x8]
b710: shl al, cl
b712: or al, byte ptr [bp - 0x6]
b715: mov byte ptr [bp - 0x2], al
b718: mov ax, word ptr [bp - 0xa]
b71b: cmp bx, ax
b71d: jle 0xb731
b71f: mov al, byte ptr [bp - 0x2]
b722: xor byte ptr [bp - 0x1], al
b725: mov al, byte ptr [bp - 0x1]
b728: xor byte ptr [bp - 0x2], al
b72b: xor al, byte ptr [bp - 0x2]
b72e: mov byte ptr [bp - 0x1], al
b731: test byte ptr [bp - 0x1], 0x3f
b735: je 0xb78a
b737: mov al, byte ptr [bp - 0x1]
b73a: mov bx, word ptr [bp - 0x10]
b73d: add bx, word ptr [-0x3d6e]
b741: mov byte ptr [bx + 0x6], al
b744: push di
b745: mov cl, 0x6
b747: shr al, cl
b749: push ax
b74a: lea ax, [bp - 0xe]
b74d: push ax
b74e: call 0xc948
b751: sub si, si
b753: cmp si, 0x2
b756: jae 0xb797
b758: cmp byte ptr [bp + si - 0xe], 0x0
b75c: je 0xb787
b75e: mov bx, word ptr [bp - 0x10]
b761: add bx, word ptr [-0x3d6e]
b765: mov al, byte ptr [bx + 0xb]
b768: mov cx, ax
b76a: mov al, 0x12
b76c: mul byte ptr [bp + si - 0xe]
b76f: mov bx, ax
b771: add bx, word ptr [-0x3d6e]
b775: cmp byte ptr [bx + 0xb], cl
b778: jne 0xb787
b77a: mov al, byte ptr [bp - 0x2]
b77d: mov bx, word ptr [bp - 0x10]
b780: add bx, word ptr [-0x3d6e]
b784: mov byte ptr [bx + 0x6], al
b787: inc si
b788: jmp 0xb753
b78a: mov al, byte ptr [bp - 0x2]
b78d: mov bx, word ptr [bp - 0x10]
b790: add bx, word ptr [-0x3d6e]
b794: mov byte ptr [bx + 0x6], al
b797: mov si, word ptr [bp - 0x10]
b79a: mov bx, word ptr [-0x3d6e]
b79e: mov word ptr [bx + si + 0x4], 0xffff
b7a3: jmp 0xb7b6
b7a5: mov ax, di
b7a7: mov cx, 0x12
b7aa: mul cx
b7ac: mov si, ax
b7ae: mov bx, word ptr [-0x3d6e]
b7b2: mov byte ptr [bx + si + 0x6], -0x80
b7b6: pop si
b7b7: pop di
b7b8: mov sp, bp
b7ba: pop bp
b7bb: ret 0x2
b7be: push bp
b7bf: mov bp, sp
b7c1: sub sp, 0x6
b7c4: push si
b7c5: mov ax, 0x12
b7c8: mul word ptr [bp + 0x4]
b7cb: mov bx, ax
b7cd: add bx, word ptr [-0x3d6e]
b7d1: mov word ptr [bp - 0x4], bx
b7d4: mov al, byte ptr [bx + 0xc]
b7d7: mov byte ptr [bp - 0x1], al
b7da: mov al, 0x6
b7dc: mul byte ptr [-0x3d74]
b7e0: mov bx, ax
b7e2: mov word ptr [bp - 0x6], bx
b7e5: cmp byte ptr [bx + 0x1a20], 0x0
b7ea: jne 0xb83d
b7ec: mov bx, word ptr [bp - 0x4]
b7ef: cmp byte ptr [bx + 0xb], 0x0
b7f3: jne 0xb83d
b7f5: mov al, byte ptr [bx + 0x2]
b7f8: cwde
b7f9: mov cx, ax
b7fb: mov al, 0x12
b7fd: mul byte ptr [-0x3d68]
b801: mov si, ax
b803: add si, word ptr [-0x3d6e]
b807: mov al, byte ptr [si + 0x2]
b80a: cwde
b80b: sub ax, cx
b80d: cdq
b80e: xor ax, dx
b810: sub ax, dx
b812: mov cx, ax
b814: mov al, byte ptr [bx + 0x3]
b817: cwde
b818: mov dx, ax
b81a: mov al, byte ptr [si + 0x3]
b81d: cwde
b81e: sub ax, dx
b820: cdq
b821: xor ax, dx
b823: sub ax, dx
b825: mov bx, word ptr [bp - 0x6]
b828: mov dx, ax
b82a: mov al, byte ptr [bx + 0x1a1e]
b82e: sub ah, ah
b830: sub ax, dx
b832: sub ax, cx
b834: jns 0xb838
b836: sub ax, ax
b838: add al, byte ptr [bp - 0x1]
b83b: jmp 0xb840
b83d: mov al, byte ptr [bp - 0x1]
b840: pop si
b841: mov sp, bp
b843: pop bp
b844: ret 0x2
b847: add byte ptr [di - 0x75], dl
b84a: in al, dx
b84b: sub sp, 0x8
b84e: push di
b84f: push si
b850: mov si, word ptr [bp + 0x4]
b853: mov ax, si
b855: mov cx, 0x12
b858: mul cx
b85a: mov bx, ax
b85c: mov word ptr [bp - 0x6], bx
b85f: add bx, word ptr [-0x3d6e]
b863: mov word ptr [bp - 0x4], bx
b866: cmp byte ptr [bx + 0x10], 0x4
b86a: jne 0xb877
b86c: mov byte ptr [bx + 0xd], 0x4
b870: push si
b871: call 0xcb3c
b874: jmp 0xb91d
b877: mov al, byte ptr [bx + 0x11]
b87a: and al, 0xf
b87c: mov byte ptr [bp - 0x8], al
b87f: cmp al, 0x2
b881: jne 0xb8c6
b883: mov cl, 0x4
b885: mov bl, byte ptr [bx + 0x11]
b888: and bl, 0x30
b88b: shr bl, cl
b88d: sub bh, bh
b88f: mov di, word ptr [bp - 0x8]
b892: and di, 0xff
b896: shl di
b898: shl di
b89a: mov ax, bx
b89c: mov bx, word ptr [bp - 0x4]
b89f: mov cl, byte ptr [bx + 0xa]
b8a2: sub ch, ch
b8a4: add di, cx
b8a6: shl di
b8a8: shl di
b8aa: mov bx, ax
b8ac: mov al, byte ptr [bx + di + 0x6c24]
b8b0: mov byte ptr [bp - 0x1], al
b8b3: mov bx, word ptr [bp - 0x4]
b8b6: mov al, byte ptr [bx + 0xe]
b8b9: cmp al, 0x2
b8bb: je 0xb8c1
b8bd: cmp al, 0x1
b8bf: jne 0xb8ef
b8c1: inc byte ptr [bp - 0x1]
b8c4: jmp 0xb8ef
b8c6: mov bl, byte ptr [bx + 0xe]
b8c9: sub bh, bh
b8cb: mov di, word ptr [bp - 0x8]
b8ce: and di, 0xff
b8d2: shl di
b8d4: shl di
b8d6: mov ax, bx
b8d8: mov bx, word ptr [bp - 0x4]
b8db: mov cl, byte ptr [bx + 0xa]
b8de: sub ch, ch
b8e0: add di, cx
b8e2: shl di
b8e4: shl di
b8e6: mov bx, ax
b8e8: mov al, byte ptr [bx + di + 0x6c24]
b8ec: mov byte ptr [bp - 0x1], al
b8ef: mov al, byte ptr [bp - 0x1]
b8f2: mov bx, word ptr [bp - 0x4]
b8f5: mov byte ptr [bx + 0x9], al
b8f8: mov bx, word ptr [bp - 0x6]
b8fb: add bx, word ptr [-0x3d6e]
b8ff: mov al, byte ptr [bx + 0x11]
b902: mov byte ptr [bx + 0x10], al
b905: mov si, word ptr [bp - 0x6]
b908: mov bx, word ptr [-0x3d6e]
b90c: lea ax, [bx + si + 0x11]
b90f: mov bl, byte ptr [bx + si + 0x11]
b912: and bx, 0xf
b915: mov cl, byte ptr [bx + 0x724c]
b919: mov bx, ax
b91b: mov byte ptr [bx], cl
b91d: pop si
b91e: pop di
b91f: mov sp, bp
b921: pop bp
b922: ret 0x2
b925: push bp
b926: mov bp, sp
b928: sub sp, 0x2
b92b: mov ax, 0x12
b92e: mul word ptr [bp + 0x6]
b931: mov bx, ax
b933: add bx, word ptr [-0x3d6e]
b937: add bx, 0x11
b93a: mov word ptr [bp - 0x2], bx
b93d: cmp byte ptr [bx], 0x4
b940: je 0xb947
b942: mov al, byte ptr [bp + 0x4]
b945: mov byte ptr [bx], al
b947: mov sp, bp
b949: pop bp
b94a: ret 0x4
b94d: push bp
b94e: mov bp, sp
b950: sub sp, 0xa
b953: push di
b954: push si
b955: mov si, word ptr [bp + 0x6]
b958: mov ax, si
b95a: mov cx, 0x12
b95d: mul cx
b95f: mov bx, ax
b961: mov word ptr [bp - 0x6], bx
b964: add bx, word ptr [-0x3d6e]
b968: mov word ptr [bp - 0x4], bx
b96b: cmp byte ptr [bx + 0xd], 0x4
b96f: jne 0xb974
b971: jmp 0xba24
b974: mov bl, byte ptr [bx + 0x9]
b977: sub bh, bh
b979: shl bx
b97b: mov word ptr [bp - 0x8], bx
b97e: mov bx, word ptr [bx + 0x6da6]
b982: cmp byte ptr [bx], -0x1
b985: jne 0xb98f
b987: mov bx, word ptr [bp - 0x4]
b98a: mov al, byte ptr [bx + 0xf]
b98d: jmp 0xb9a2
b98f: mov bx, word ptr [bp - 0x4]
b992: mov di, word ptr [bx + 0xf]
b995: and di, 0xff
b999: mov bx, word ptr [bp - 0x8]
b99c: mov bx, word ptr [bx + 0x6da6]
b9a0: mov al, byte ptr [bx + di]
b9a2: mov bx, word ptr [bp - 0x8]
b9a5: sub ah, ah
b9a7: mov di, ax
b9a9: add di, word ptr [bx + 0x6b28]
b9ad: mov bx, word ptr [bp - 0x4]
b9b0: mov al, byte ptr [bx + 0x2]
b9b3: mov ch, byte ptr [bx + 0x3]
b9b6: sub cl, cl
b9b8: cwde
b9b9: or cx, ax
b9bb: mov ax, 0x6
b9be: mul word ptr [-0x56d8]
b9c2: mov bx, ax
b9c4: mov word ptr [bp - 0xa], bx
b9c7: mov word ptr [bx - 0x56d6], cx
b9cb: mov word ptr [bx - 0x56d4], di
b9cf: mov bx, word ptr [bp - 0x4]
b9d2: cmp byte ptr [bx + 0xb], 0x1
b9d6: jne 0xb9e1
b9d8: mov bx, word ptr [bp - 0xa]
b9db: add word ptr [bx - 0x56d4], 0xbf
b9e1: mov bx, word ptr [bp - 0x4]
b9e4: mov al, byte ptr [bx + 0xf]
b9e7: mov di, word ptr [bp - 0xa]
b9ea: mov byte ptr [di - 0x56d2], al
b9ee: mov al, byte ptr [bx + 0x9]
b9f1: mov byte ptr [di - 0x56d1], al
b9f5: inc word ptr [-0x56d8]
b9f9: cmp word ptr [bp + 0x4], 0x0
b9fd: je 0xba24
b9ff: mov bx, word ptr [bp - 0x4]
ba02: inc byte ptr [bx + 0xf]
ba05: mov bx, word ptr [bp - 0x6]
ba08: add bx, word ptr [-0x3d6e]
ba0c: mov word ptr [bp - 0x4], bx
ba0f: mov al, byte ptr [bx + 0xf]
ba12: mov bl, byte ptr [bx + 0x9]
ba15: sub bh, bh
ba17: cmp byte ptr [bx + 0x6bd0], al
ba1b: ja 0xba24
ba1d: mov bx, word ptr [bp - 0x4]
ba20: mov byte ptr [bx + 0xf], 0x0
ba24: pop si
ba25: pop di
ba26: mov sp, bp
ba28: pop bp
ba29: ret 0x4
ba2c: push si
ba2d: mov word ptr [-0x56d8], 0x0
ba33: mov si, 0xa92a
ba36: jmp 0xba47
ba38: mov word ptr [si], 0x0
ba3c: sub al, al
ba3e: mov byte ptr [si + 0x4], al
ba41: mov byte ptr [si + 0x5], al
ba44: add si, 0x6
ba47: cmp si, 0xaa02
ba4b: jb 0xba38
ba4d: pop si
ba4e: ret
ba4f: sub dx, dx
ba51: mov bx, 0xaa02
ba54: jmp 0xba5c
ba56: inc bx
ba57: inc bx
ba58: mov word ptr [bx - 0x2], dx
ba5b: inc dx
ba5c: cmp word ptr [-0x56d8], dx
ba60: ja 0xba56
ba62: mov ax, 0xaa02
ba65: push ax
ba66: sub ax, ax
ba68: push ax
ba69: mov ax, word ptr [0xa928]
ba6c: dec ax
ba6d: push ax
ba6e: call 0xbd67
ba71: ret
ba72: push bp
ba73: mov bp, sp
ba75: sub sp, 0x16
ba78: push si
ba79: mov word ptr [bp - 0xc], 0x0
ba7e: mov word ptr [bp - 0xa], 0xaa02
ba83: jmp 0xbb3d
ba86: mov ax, 0x6
ba89: mov bx, word ptr [bp - 0xa]
ba8c: mul word ptr [bx]
ba8e: mov bx, ax
ba90: mov al, byte ptr [bx - 0x56d6]
ba94: mov byte ptr [bp - 0x10], al
ba97: mov al, byte ptr [bx - 0x56d5]
ba9b: mov byte ptr [bp - 0xf], al
ba9e: lea ax, [bp - 0x10]
baa1: push ax
baa2: lea ax, [bp - 0x16]
baa5: push ax
baa6: call 0xc7b0
baa9: mov ax, 0x6
baac: mov bx, word ptr [bp - 0xa]
baaf: mul word ptr [bx]
bab1: mov bx, ax
bab3: mov al, byte ptr [bx - 0x56d2]
bab7: mov byte ptr [bp - 0x7], al
baba: mov ax, bx
babc: mov bl, byte ptr [bx - 0x56d1]
bac0: sub bh, bh
bac2: shl bx
bac4: mov cx, word ptr [bx + 0x719c]
bac8: mov word ptr [bp - 0xe], cx
bacb: mov bx, ax
bacd: mov ax, word ptr [bx - 0x56d4]
bad1: mov word ptr [bp - 0x6], ax
bad4: mov word ptr [bp - 0x4], 0x0
bad9: mov word ptr [bp - 0x2], 0x727e
bade: cmp word ptr [bp - 0x2], 0x72c4
bae3: jae 0xbb0b
bae5: mov bx, word ptr [bp - 0x2]
bae8: cmp word ptr [bx], ax
baea: jbe 0xbaf5
baec: add word ptr [bp - 0x2], 0xa
baf0: inc word ptr [bp - 0x4]
baf3: jmp 0xbade
baf5: mov ax, 0xa
baf8: mul word ptr [bp - 0x4]
bafb: mov bx, ax
bafd: mov ax, word ptr [bx + 0x7280]
bb01: mov word ptr [bp - 0x12], ax
bb04: mov ax, word ptr [bx + 0x727e]
bb08: sub word ptr [bp - 0x6], ax
bb0b: mov ax, 0xe
bb0e: mul word ptr [bp - 0x6]
bb11: add ax, word ptr [bp - 0x12]
bb14: push ax
bb15: mov bl, byte ptr [bp - 0x7]
bb18: sub bh, bh
bb1a: shl bx
bb1c: mov si, word ptr [bp - 0xe]
bb1f: mov al, byte ptr [bx + si]
bb21: cwde
bb22: add ax, word ptr [bp - 0x16]
bb25: push ax
bb26: add si, bx
bb28: mov al, byte ptr [si + 0x1]
bb2b: cwde
bb2c: add ax, word ptr [bp - 0x14]
bb2f: sub ax, 0x4
bb32: push ax
bb33: call 0x8e62
bb36: add word ptr [bp - 0xa], 0x2
bb3a: inc word ptr [bp - 0xc]
bb3d: mov ax, word ptr [0xa928]
bb40: cmp word ptr [bp - 0xc], ax
bb43: jae 0xbb48
bb45: jmp 0xba86
bb48: pop si
bb49: mov sp, bp
bb4b: pop bp
bb4c: ret
bb4d: push di
bb4e: push si
bb4f: call 0xba2c
bb52: mov si, 0x1
bb55: mov di, 0x12
bb58: jmp 0xbb6f
bb5a: mov bx, word ptr [-0x3d6e]
bb5e: cmp byte ptr [bx + di + 0xd], 0x4
bb62: je 0xbb6b
bb64: push si
bb65: sub ax, ax
bb67: push ax
bb68: call 0xb94d
bb6b: add di, 0x12
bb6e: inc si
bb6f: cmp word ptr [-0x3d6c], si
bb73: ja 0xbb5a
bb75: call 0xba4f
bb78: call 0xba72
bb7b: pop si
bb7c: pop di
bb7d: ret
bb7e: mov ax, 0xc0d0
bb81: push ax
bb82: mov ax, 0x56fe
bb85: push ax
bb86: mov cl, 0x3
bb88: mov al, byte ptr [0xa926]
bb8b: sub ah, ah
bb8d: shl ax, cl
bb8f: add ax, 0xc0a6
bb92: push ax
bb93: call 0x8790
bb96: ret
bb97: push bp
bb98: mov bp, sp
bb9a: sub sp, 0x1a
bb9d: push di
bb9e: push si
bb9f: mov cl, 0x5
bba1: mov bx, word ptr [bp + 0x4]
bba4: shl bx, cl
bba6: mov al, byte ptr [bx + 0x5279]
bbaa: mov byte ptr [0xa926], al
bbad: cmp word ptr [-0x64da], 0x2
bbb2: je 0xbbb7
bbb4: jmp 0xbc84
bbb7: sub si, si
bbb9: mov ax, 0x2
bbbc: push ax
bbbd: lea ax, [bp - 0xc]
bbc0: push ax
bbc1: call 0xf7b
bbc4: or ax, ax
bbc6: jne 0xbbcb
bbc8: jmp 0xbc7f
bbcb: call 0x67e4
bbce: cmp byte ptr [-0x56da], 0x1
bbd3: jne 0xbbfc
bbd5: mov ax, 0x72c4
bbd8: push ax
bbd9: lea ax, [bp - 0xc]
bbdc: push ax
bbdd: lea ax, [bp - 0x1a]
bbe0: push ax
bbe1: call 0x1273
bbe4: or ax, ax
bbe6: je 0xbc34
bbe8: lea ax, [bp - 0x1a]
bbeb: push ax
bbec: sub cx, cx
bbee: push cx
bbef: push cx
bbf0: call 0x8e62
bbf3: lea ax, [bp - 0x1a]
bbf6: push ax
bbf7: call 0x86a0
bbfa: jmp 0xbc37
bbfc: mov ax, 0x72cf
bbff: push ax
bc00: lea ax, [bp - 0xc]
bc03: push ax
bc04: lea ax, [bp - 0x1a]
bc07: push ax
bc08: call 0x1273
bc0b: or ax, ax
bc0d: je 0xbc34
bc0f: lea ax, [bp - 0x1a]
bc12: push ax
bc13: sub cx, cx
bc15: push cx
bc16: push cx
bc17: call 0x8e62
bc1a: lea ax, [bp - 0x1a]
bc1d: push ax
bc1e: call 0x86a0
bc21: mov ax, 0x72dc
bc24: push ax
bc25: lea ax, [bp - 0xc]
bc28: push ax
bc29: lea ax, [bp - 0x1a]
bc2c: push ax
bc2d: call 0x1273
bc30: or ax, ax
bc32: jne 0xbbe8
bc34: mov si, 0xffff
bc37: lea ax, [bp - 0xc]
bc3a: push ax
bc3b: call 0x11a9
bc3e: call 0x680f
bc41: mov ax, word ptr [0xc0c6]
bc44: mov cl, 0x3
bc46: mov bl, byte ptr [-0x56da]
bc4a: sub bh, bh
bc4c: shl bx, cl
bc4e: mov word ptr [bx - 0x3f58], ax
bc52: mov word ptr [bx - 0x3f5a], 0x0
bc58: mov word ptr [bx - 0x3f56], 0x140
bc5e: mov word ptr [bx - 0x3f54], 0xc8
bc64: mov ax, 0xc1da
bc67: push ax
bc68: mov ax, 0x56fe
bc6b: push ax
bc6c: add bx, 0xc0a6
bc70: push bx
bc71: call 0x86b2
bc74: mov word ptr [0x254], 0x0
bc7a: mov word ptr [bp - 0x4], si
bc7d: jmp 0xbc84
bc7f: mov word ptr [bp - 0x4], 0xffff
bc84: mov ax, 0x2
bc87: push ax
bc88: lea ax, [bp - 0xc]
bc8b: push ax
bc8c: call 0xf7b
bc8f: mov di, ax
bc91: or di, di
bc93: jne 0xbc98
bc95: jmp 0xbd3e
bc98: mov word ptr [bp - 0x6], di
bc9b: cmp word ptr [-0x3d66], 0xb
bca0: jb 0xbca9
bca2: cmp word ptr [-0x3d66], 0xe
bca7: jbe 0xbcb7
bca9: cmp word ptr [-0x3d66], 0x17
bcae: jb 0xbccb
bcb0: cmp word ptr [-0x3d66], 0x1a
bcb5: ja 0xbccb
bcb7: mov al, 0x62
bcb9: mov byte ptr [0x728c], al
bcbc: mov byte ptr [0x72a0], al
bcbf: mov byte ptr [-0x3d59], 0xa
bcc4: mov byte ptr [-0x3d5a], 0x9
bcc9: jmp 0xbcd5
bccb: mov byte ptr [-0x3d59], 0xc
bcd0: mov byte ptr [-0x3d5a], 0xb
bcd5: mov si, 0x7282
bcd8: jmp 0xbcfd
bcda: mov bx, word ptr [-0x3d66]
bcde: mov al, byte ptr [bx + 0x7262]
bce2: mov byte ptr [0x7256], al
bce5: push si
bce6: lea ax, [bp - 0xc]
bce9: push ax
bcea: push word ptr [si + 0x4]
bced: call 0x163b
bcf0: mov word ptr [si - 0x2], ax
bcf3: or ax, ax
bcf5: jne 0xbcfa
bcf7: mov word ptr [bp - 0x6], ax
bcfa: add si, 0xa
bcfd: cmp si, 0x72c8
bd01: jb 0xbcda
bd03: cmp word ptr [bp - 0x4], 0x0
bd07: jne 0xbd15
bd09: cmp word ptr [-0x64da], 0x2
bd0e: jne 0xbd15
bd10: mov di, word ptr [bp - 0x6]
bd13: jmp 0xbd2f
bd15: mov dx, 0x1
bd18: cmp byte ptr [-0x56da], 0x0
bd1d: je 0xbd22
bd1f: mov dx, 0x2
bd22: mov di, word ptr [bp - 0x6]
bd25: push dx
bd26: lea ax, [bp - 0xc]
bd29: push ax
bd2a: call 0x13a8
bd2d: and di, ax
bd2f: lea ax, [bp - 0xc]
bd32: push ax
bd33: call 0x11a9
bd36: mov al, 0x72
bd38: mov byte ptr [0x728c], al
bd3b: mov byte ptr [0x72a0], al
bd3e: mov ax, di
bd40: pop si
bd41: pop di
bd42: mov sp, bp
bd44: pop bp
bd45: ret 0x2
bd48: push si
bd49: mov si, 0x7280
bd4c: jmp 0xbd5f
bd4e: mov dx, word ptr [si]
bd50: or dx, dx
bd52: je 0xbd5c
bd54: push dx
bd55: call 0x19a0
bd58: mov word ptr [si], 0x0
bd5c: add si, 0xa
bd5f: cmp si, 0x72c6
bd63: jb 0xbd4e
bd65: pop si
bd66: ret
bd67: push bp
bd68: mov bp, sp
bd6a: sub sp, 0x8
bd6d: push di
bd6e: push si
bd6f: mov ax, word ptr [bp + 0x4]
bd72: mov word ptr [bp - 0x4], ax
bd75: mov di, word ptr [bp + 0x6]
bd78: mov bx, di
bd7a: add bx, ax
bd7c: and bl, -0x2
bd7f: mov si, word ptr [bp + 0x8]
bd82: mov ax, 0x6
bd85: mul word ptr [bx + si]
bd87: mov bx, ax
bd89: mov ax, word ptr [bx - 0x56d6]
bd8d: mov word ptr [bp - 0x8], ax
bd90: mov bx, di
bd92: shl bx
bd94: add bx, word ptr [bp + 0x8]
bd97: mov word ptr [bp - 0x6], di
bd9a: jmp 0xbda9
bd9c: mov ax, word ptr [bp + 0x4]
bd9f: cmp word ptr [bp - 0x6], ax
bda2: jge 0xbdb9
bda4: inc bx
bda5: inc bx
bda6: inc word ptr [bp - 0x6]
bda9: mov ax, 0x6
bdac: mul word ptr [bx]
bdae: mov si, ax
bdb0: mov ax, word ptr [bp - 0x8]
bdb3: cmp word ptr [si - 0x56d6], ax
bdb7: jl 0xbd9c
bdb9: mov bx, word ptr [bp - 0x4]
bdbc: shl bx
bdbe: add bx, word ptr [bp + 0x8]
bdc1: mov si, word ptr [bp - 0x4]
bdc4: jmp 0xbdce
bdc6: cmp word ptr [bp + 0x6], si
bdc9: jge 0xbdde
bdcb: dec bx
bdcc: dec bx
bdcd: dec si
bdce: mov ax, 0x6
bdd1: mul word ptr [bx]
bdd3: mov di, ax
bdd5: mov ax, word ptr [bp - 0x8]
bdd8: cmp word ptr [di - 0x56d6], ax
bddc: jg 0xbdc6
bdde: mov di, word ptr [bp - 0x6]
bde1: cmp si, di
bde3: jl 0xbe0b
bde5: mov bx, di
bde7: shl bx
bde9: add bx, word ptr [bp + 0x8]
bdec: mov ax, word ptr [bx]
bdee: mov word ptr [bp - 0x2], ax
bdf1: mov ax, bx
bdf3: mov bx, si
bdf5: shl bx
bdf7: add bx, word ptr [bp + 0x8]
bdfa: mov cx, word ptr [bx]
bdfc: mov dx, bx
bdfe: mov bx, ax
be00: mov word ptr [bx], cx
be02: mov ax, word ptr [bp - 0x2]
be05: mov bx, dx
be07: mov word ptr [bx], ax
be09: inc di
be0a: dec si
be0b: mov word ptr [bp - 0x4], si
be0e: cmp si, di
be10: jl 0xbe15
be12: jmp 0xbd90
be15: cmp word ptr [bp + 0x6], si
be18: jge 0xbe24
be1a: push word ptr [bp + 0x8]
be1d: push word ptr [bp + 0x6]
be20: push si
be21: call 0xbd67
be24: cmp word ptr [bp + 0x4], di
be27: jle 0xbe33
be29: push word ptr [bp + 0x8]
be2c: push di
be2d: push word ptr [bp + 0x4]
be30: call 0xbd67
be33: pop si
be34: pop di
be35: mov sp, bp
be37: pop bp
be38: ret 0x6
be3b: add byte ptr [di - 0x75], dl
be3e: in al, dx
be3f: sub sp, 0x1a
be42: push di
be43: push si
be44: mov byte ptr [-0x3d57], -0x1
be49: sub si, si
be4b: mov di, si
be4d: mov word ptr [bp - 0x8], di
be50: jmp 0xbe5d
be52: mov ax, 0x12
be55: sub ax, si
be57: push ax
be58: push si
be59: call 0xc11e
be5c: inc si
be5d: cmp si, 0x13
be60: jb 0xbe52
be62: sub si, si
be64: push si
be65: lea ax, [si + 0x17]
be68: push ax
be69: call 0xc11e
be6c: mov ax, 0x2b
be6f: sub ax, si
be71: push ax
be72: mov ax, 0x17
be75: sub ax, si
be77: push ax
be78: call 0xc11e
be7b: inc si
be7c: cmp si, 0x18
be7f: jb 0xbe64
be81: sub si, si
be83: lea ax, [si + 0x18]
be86: push ax
be87: mov ax, 0x2f
be8a: sub ax, si
be8c: push ax
be8d: call 0xc11e
be90: inc si
be91: cmp si, 0x14
be94: jb 0xbe83
be96: mov si, 0x13
be99: sub ax, ax
be9b: push ax
be9c: push si
be9d: call 0xc11e
bea0: inc si
bea1: cmp si, 0x15
bea4: jbe 0xbe99
bea6: mov ax, 0x13
bea9: push ax
beaa: sub ax, ax
beac: push ax
bead: call 0xc11e
beb0: push word ptr [bp + 0x6]
beb3: call 0xbb97
beb6: or ax, ax
beb8: jne 0xbec2
beba: call 0xbd48
bebd: sub ax, ax
bebf: jmp 0xc116
bec2: mov al, byte ptr [0xc2a6]
bec5: push ax
bec6: call 0x5d73
bec9: mov al, byte ptr [0xc2a7]
becc: push ax
becd: call 0x5d73
bed0: mov ax, 0x2
bed3: push ax
bed4: lea ax, [bp - 0xe]
bed7: push ax
bed8: call 0xf7b
bedb: or ax, ax
bedd: je 0xbefb
bedf: mov ax, 0x733d
bee2: push ax
bee3: lea ax, [bp - 0xe]
bee6: push ax
bee7: lea cx, [bp - 0x16]
beea: push cx
beeb: call 0x11c4
beee: mov ax, word ptr [bp - 0x16]
bef1: mov word ptr [0x76ea], ax
bef4: lea ax, [bp - 0xe]
bef7: push ax
bef8: call 0x11a9
befb: call 0xbb7e
befe: sub ax, ax
bf00: push ax
bf01: push ax
bf02: mov cx, 0x13f
bf05: push cx
bf06: mov cx, 0xc7
bf09: push cx
bf0a: call 0x691b
bf0d: call 0x1d19
bf10: mov ax, 0x73b6
bf13: push ax
bf14: mov ax, 0x2
bf17: push ax
bf18: call 0x5614
bf1b: push word ptr [bp + 0x4]
bf1e: call 0xc153
bf21: or ax, ax
bf23: jne 0xbf40
bf25: call 0xbd48
bf28: mov al, byte ptr [0xc2a6]
bf2b: push ax
bf2c: call 0x5e1a
bf2f: mov al, byte ptr [0xc2a7]
bf32: push ax
bf33: call 0x5e1a
bf36: lea ax, [bp - 0x16]
bf39: push ax
bf3a: call 0x198f
bf3d: jmp 0xbebd
bf40: mov cl, 0x5
bf42: mov bx, word ptr [-0x3d66]
bf46: shl bx, cl
bf48: mov al, byte ptr [bx + 0x5286]
bf4c: mov byte ptr [bp - 0x5], al
bf4f: mov ax, word ptr [0x73fe]
bf52: dec ax
bf53: dec ax
bf54: shr ax
bf56: shr ax
bf58: mov byte ptr [bp - 0x1], al
bf5b: mov ax, 0x64
bf5e: push ax
bf5f: call 0x76b
bf62: mov si, ax
bf64: mov al, byte ptr [bp - 0x5]
bf67: sub ah, ah
bf69: shl ax
bf6b: shl ax
bf6d: mov cl, byte ptr [bp - 0x1]
bf70: sub ch, ch
bf72: add ax, cx
bf74: mov cx, 0x5
bf77: imul cx
bf79: mov word ptr [bp - 0x18], ax
bf7c: mov word ptr [bp - 0x4], si
bf7f: inc byte ptr [-0x3d57]
bf83: mov bl, byte ptr [-0x3d57]
bf87: sub bh, bh
bf89: add bx, word ptr [bp - 0x18]
bf8c: mov al, byte ptr [bx + 0x77f0]
bf90: sub ah, ah
bf92: add di, ax
bf94: cmp di, si
bf96: jbe 0xbf7f
bf98: mov ax, 0x73b6
bf9b: push ax
bf9c: sub ax, ax
bf9e: push ax
bf9f: call 0x54c8
bfa2: mov byte ptr [0xc2aa], al
bfa5: cmp al, -0x1
bfa7: jne 0xbfcb
bfa9: mov al, byte ptr [0xc2a6]
bfac: push ax
bfad: call 0x5e1a
bfb0: mov al, byte ptr [0xc2a7]
bfb3: push ax
bfb4: call 0x5e1a
bfb7: call 0xbd48
bfba: push word ptr [-0x3d6e]
bfbe: call 0x74b7
bfc1: push word ptr [-0x3d6a]
bfc5: call 0x74b7
bfc8: jmp 0xbf36
bfcb: mov al, 0x2
bfcd: push ax
bfce: sub al, al
bfd0: push ax
bfd1: mov cl, byte ptr [-0x3d56]
bfd5: push cx
bfd6: call 0xc2f2
bfd9: sub al, al
bfdb: push ax
bfdc: mov al, 0x1
bfde: push ax
bfdf: mov al, byte ptr [0xc2a9]
bfe2: push ax
bfe3: call 0xc2f2
bfe6: sub ax, ax
bfe8: push ax
bfe9: push ax
bfea: mov cx, 0x13f
bfed: push cx
bfee: mov dx, 0xc7
bff1: push dx
bff2: call 0x6973
bff5: call 0xbb4d
bff8: sub ax, ax
bffa: push ax
bffb: push ax
bffc: mov ax, 0x13f
bfff: push ax
c000: mov ax, 0xc7
c003: push ax
c004: call 0x691b
c007: sub si, si
c009: mov di, si
c00b: jmp 0xc02a
c00d: mov al, 0x5
c00f: mul byte ptr [-0x3d56]
c013: add ax, di
c015: mov cx, 0xa
c018: mul cx
c01a: mov bx, ax
c01c: mov ax, word ptr [bx + 0x76f6]
c020: mov bx, word ptr [0x73d2]
c024: mov word ptr [bx + si], ax
c026: add si, 0xc
c029: inc di
c02a: cmp si, 0x30
c02d: jb 0xc00d
c02f: mov al, 0x5
c031: mul byte ptr [bp - 0x5]
c034: mov bx, ax
c036: mov al, byte ptr [0xc2a9]
c039: sub ah, ah
c03b: add bx, ax
c03d: shl bx
c03f: mov di, word ptr [bx + 0x7986]
c043: or di, di
c045: je 0xc07e
c047: mov ax, 0x64
c04a: push ax
c04b: call 0x76b
c04e: mov word ptr [bp - 0x4], ax
c051: sub si, si
c053: mov byte ptr [-0x3d55], -0x1
c058: mov al, 0x5
c05a: mul byte ptr [-0x3d56]
c05e: mov word ptr [bp - 0x1a], ax
c061: mov word ptr [bp - 0x2], di
c064: inc byte ptr [-0x3d55]
c068: mov bl, byte ptr [-0x3d55]
c06c: sub bh, bh
c06e: add bx, word ptr [bp - 0x1a]
c071: mov al, byte ptr [bx + di]
c073: sub ah, ah
c075: add si, ax
c077: cmp si, word ptr [bp - 0x4]
c07a: jbe 0xc064
c07c: jmp 0xc088
c07e: mov ax, 0x4
c081: push ax
c082: call 0x76b
c085: mov byte ptr [0xc2ab], al
c088: mov al, 0x5
c08a: mul byte ptr [-0x3d57]
c08e: mov cl, byte ptr [-0x3d55]
c092: sub ch, ch
c094: add ax, cx
c096: mov cx, 0xa
c099: imul cx
c09b: mov bx, ax
c09d: mov al, byte ptr [bx + 0x76fe]
c0a1: mov byte ptr [0xc2ad], al
c0a4: mov ax, 0x73c8
c0a7: push ax
c0a8: mov cx, 0x2
c0ab: push cx
c0ac: call 0x5614
c0af: mov ax, 0x73c8
c0b2: push ax
c0b3: sub ax, ax
c0b5: push ax
c0b6: call 0x54c8
c0b9: mov byte ptr [0xc2ac], al
c0bc: cmp al, -0x1
c0be: jne 0xc0c3
c0c0: jmp 0xbfa9
c0c3: sub ax, ax
c0c5: push ax
c0c6: push ax
c0c7: mov ax, 0x13f
c0ca: push ax
c0cb: mov ax, 0xc7
c0ce: push ax
c0cf: call 0x6973
c0d2: sub al, al
c0d4: push ax
c0d5: mov al, 0x5
c0d7: mul byte ptr [-0x3d56]
c0db: mov cl, byte ptr [-0x3d54]
c0df: sub ch, ch
c0e1: add ax, cx
c0e3: mov cx, 0xa
c0e6: imul cx
c0e8: add ax, 0x76f6
c0eb: push ax
c0ec: call 0xc525
c0ef: mov al, 0x1
c0f1: push ax
c0f2: mov al, 0x5
c0f4: mul byte ptr [-0x3d57]
c0f8: mov cl, byte ptr [-0x3d55]
c0fc: sub ch, ch
c0fe: add ax, cx
c100: mov cx, 0xa
c103: imul cx
c105: add ax, 0x76f6
c108: push ax
c109: call 0xc525
c10c: lea ax, [bp - 0x16]
c10f: push ax
c110: call 0x198f
c113: mov ax, 0xffff
c116: pop si
c117: pop di
c118: mov sp, bp
c11a: pop bp
c11b: ret 0x4
c11e: push bp
c11f: mov bp, sp
c121: push si
c122: cmp word ptr [bp + 0x6], 0x0
c126: jl 0xc14c
c128: cmp word ptr [bp + 0x6], 0x30
c12c: jge 0xc14c
c12e: cmp word ptr [bp + 0x4], 0x0
c132: jl 0xc14c
c134: cmp word ptr [bp + 0x4], 0x30
c138: jge 0xc14c
c13a: mov ax, 0x30
c13d: imul word ptr [bp + 0x6]
c140: mov bx, ax
c142: add bx, word ptr [-0x3d52]
c146: mov si, word ptr [bp + 0x4]
c149: mov byte ptr [bx + si], -0x2
c14c: pop si
c14d: mov sp, bp
c14f: pop bp
c150: ret 0x4
c153: push bp
c154: mov bp, sp
c156: sub sp, 0x6
c159: push di
c15a: push si
c15b: mov di, word ptr [bp + 0x4]
c15e: mov ax, di
c160: mov cx, 0x14
c163: mul cx
c165: mov bx, ax
c167: mov word ptr [bp - 0x6], bx
c16a: mov ax, word ptr [bx + 0x191e]
c16e: add ax, 0xb
c171: mov cx, 0xc
c174: sub dx, dx
c176: div cx
c178: mov bl, byte ptr [0x48e]
c17c: sub bh, bh
c17e: shl bx
c180: mov cx, word ptr [bx + 0x7402]
c184: cmp ax, cx
c186: jb 0xc18b
c188: jmp 0xc215
c18b: mov si, cx
c18d: cmp si, 0x1f4
c191: jbe 0xc196
c193: mov si, 0x1f4
c196: mov al, byte ptr [0xc2a2]
c199: sub ah, ah
c19b: add ax, 0x13
c19e: mov cx, 0x14
c1a1: cdq
c1a2: idiv cx
c1a4: inc ax
c1a5: mov word ptr [0x73fe], ax
c1a8: mov cx, ax
c1aa: mov ax, word ptr [0xc29c]
c1ad: add ax, 0x95
c1b0: mov bx, 0x96
c1b3: sub dx, dx
c1b5: div bx
c1b7: add cx, ax
c1b9: mov word ptr [0x73fe], cx
c1bd: mov ax, word ptr [0xc29e]
c1c0: add ax, 0x1f3
c1c3: mov bx, 0x1f4
c1c6: sub dx, dx
c1c8: div bx
c1ca: add cx, ax
c1cc: mov word ptr [0x73fe], cx
c1d0: mov bx, word ptr [bp - 0x6]
c1d3: mov ax, word ptr [bx + 0x1920]
c1d7: add ax, 0x95
c1da: mov bx, 0x96
c1dd: sub dx, dx
c1df: div bx
c1e1: inc ax
c1e2: mov word ptr [0x7400], ax
c1e5: mov bx, word ptr [bp - 0x6]
c1e8: mov dx, ax
c1ea: mov ax, word ptr [bx + 0x191e]
c1ee: add ax, si
c1f0: dec ax
c1f1: mov bx, dx
c1f3: sub dx, dx
c1f5: div si
c1f7: add bx, ax
c1f9: mov word ptr [0x7400], bx
c1fd: add bx, cx
c1ff: inc bx
c200: mov word ptr [-0x3d6c], bx
c204: push bx
c205: mov ax, 0x12
c208: push ax
c209: call 0x7488
c20c: mov word ptr [0xc292], ax
c20f: or ax, ax
c211: jne 0xc21a
c213: jmp 0xc233
c215: mov si, ax
c217: jmp 0xc18d
c21a: push word ptr [-0x3d6c]
c21e: mov ax, 0x2
c221: push ax
c222: call 0x7488
c225: mov word ptr [0xc296], ax
c228: or ax, ax
c22a: jne 0xc238
c22c: push word ptr [-0x3d6e]
c230: call 0x74b7
c233: sub ax, ax
c235: jmp 0xc2ea
c238: mov word ptr [-0x55b6], 0x1
c23e: mov bx, word ptr [bp - 0x6]
c241: mov al, byte ptr [bx + 0x1923]
c245: mov byte ptr [bp - 0x1], al
c248: push word ptr [bx + 0x1920]
c24c: mov cx, 0x96
c24f: push cx
c250: sub dl, dl
c252: push dx
c253: mov cl, 0x3
c255: push cx
c256: push dx
c257: push ax
c258: call 0xc5ec
c25b: mov bx, word ptr [bp - 0x6]
c25e: push word ptr [bx + 0x191e]
c262: push si
c263: mov al, 0x2
c265: push ax
c266: mov cl, 0x3
c268: push cx
c269: sub dl, dl
c26b: push dx
c26c: mov bl, byte ptr [bp - 0x1]
c26f: push bx
c270: call 0xc5ec
c273: mov al, byte ptr [0xaa4a]
c276: mov byte ptr [0xc298], al
c279: mov ax, 0x5
c27c: push ax
c27d: mov cx, ax
c27f: push cx
c280: mov dl, 0x3
c282: push dx
c283: push dx
c284: sub bl, bl
c286: push bx
c287: mov al, byte ptr [bp - 0x1]
c28a: push ax
c28b: call 0xc5ec
c28e: mov al, byte ptr [0xc2a2]
c291: sub ah, ah
c293: push ax
c294: mov ax, 0x14
c297: push ax
c298: mov al, 0x1
c29a: push ax
c29b: push ax
c29c: push ax
c29d: mov cl, byte ptr [-0x3d5c]
c2a1: mov byte ptr [bp - 0x1], cl
c2a4: push cx
c2a5: call 0xc5ec
c2a8: push word ptr [-0x3d64]
c2ac: mov ax, 0x96
c2af: push ax
c2b0: sub al, al
c2b2: push ax
c2b3: mov al, 0x1
c2b5: push ax
c2b6: push ax
c2b7: mov cl, byte ptr [bp - 0x1]
c2ba: push cx
c2bb: call 0xc5ec
c2be: push word ptr [-0x3d62]
c2c2: mov ax, 0x1f4
c2c5: push ax
c2c6: mov al, 0x2
c2c8: push ax
c2c9: mov al, 0x1
c2cb: push ax
c2cc: push ax
c2cd: mov cl, byte ptr [bp - 0x1]
c2d0: push cx
c2d1: call 0xc5ec
c2d4: mov ax, 0x5
c2d7: push ax
c2d8: push ax
c2d9: mov al, 0x3
c2db: push ax
c2dc: mov al, 0x1
c2de: push ax
c2df: push ax
c2e0: mov al, byte ptr [bp - 0x1]
c2e3: push ax
c2e4: call 0xc5ec
c2e7: mov ax, 0xffff
c2ea: pop si
c2eb: pop di
c2ec: mov sp, bp
c2ee: pop bp
c2ef: ret 0x2
c2f2: push bp
c2f3: mov bp, sp
c2f5: sub sp, 0x16
c2f8: push di
c2f9: push si
c2fa: mov word ptr [bp - 0x10], 0x0
c2ff: mov bl, byte ptr [bp + 0x4]
c302: sub bh, bh
c304: mov word ptr [bp - 0x14], bx
c307: shl bx
c309: add bx, 0x76ec
c30d: mov word ptr [bp - 0x12], bx
c310: mov ax, word ptr [bx]
c312: mov cx, ax
c314: inc ax
c315: inc ax
c316: mov word ptr [bp - 0xc], ax
c319: mov si, cx
c31b: mov bl, byte ptr [bp + 0x6]
c31e: sub bh, bh
c320: shl bx
c322: mov ax, word ptr [bx + si + 0x24]
c325: mov word ptr [bp - 0xe], ax
c328: cmp byte ptr [bp + 0x6], 0x1
c32c: jne 0xc386
c32e: mov word ptr [bp - 0x4], 0x0
c333: mov di, 0x1
c336: mov si, 0x12
c339: cmp word ptr [-0x3d6c], di
c33d: jbe 0xc386
c33f: mov bx, word ptr [-0x3d6e]
c343: add bx, si
c345: cmp byte ptr [bx + 0xa], 0x1
c349: jne 0xc380
c34b: mov ax, 0x6
c34e: mul word ptr [bp - 0x14]
c351: mov bx, ax
c353: add bx, word ptr [bp - 0x4]
c356: mov al, byte ptr [bx + 0x73da]
c35a: mov bx, word ptr [-0x3d6e]
c35e: add bx, si
c360: mov byte ptr [bx + 0x2], al
c363: mov bx, word ptr [bp - 0x14]
c366: mov al, byte ptr [bx + 0x73f8]
c36a: mov bx, word ptr [-0x3d6e]
c36e: mov byte ptr [bx + si + 0x3], al
c371: push di
c372: call 0xca55
c375: mov bx, word ptr [-0x3d6e]
c379: mov byte ptr [bx + si + 0x6], -0x2
c37d: inc word ptr [bp - 0x4]
c380: add si, 0x12
c383: inc di
c384: jmp 0xc339
c386: mov di, 0x1
c389: mov word ptr [bp - 0x8], 0x0
c38e: cmp word ptr [-0x3d6c], di
c392: jae 0xc397
c394: jmp 0xc43b
c397: mov ax, di
c399: mov cx, 0x12
c39c: mul cx
c39e: add ax, word ptr [-0x3d6e]
c3a2: add ax, 0xb
c3a5: mov word ptr [bp - 0x2], ax
c3a8: mov si, ax
c3aa: cmp word ptr [-0x3d6c], di
c3ae: jbe 0xc42f
c3b0: mov al, byte ptr [bp + 0x6]
c3b3: cmp byte ptr [si], al
c3b5: jne 0xc3c9
c3b7: cmp byte ptr [si - 0x1], 0x0
c3bb: jne 0xc3c9
c3bd: cmp byte ptr [si - 0x9], 0x0
c3c1: jne 0xc3c9
c3c3: cmp byte ptr [si - 0x8], 0x0
c3c7: je 0xc3cf
c3c9: add si, 0x12
c3cc: inc di
c3cd: jmp 0xc3aa
c3cf: mov ax, di
c3d1: mov cx, 0x12
c3d4: mul cx
c3d6: mov word ptr [bp - 0x16], ax
c3d9: add ax, word ptr [-0x3d6e]
c3dd: inc ax
c3de: inc ax
c3df: push ax
c3e0: mov al, byte ptr [bp + 0x8]
c3e3: push ax
c3e4: mov bx, word ptr [bp - 0x8]
c3e7: mov si, word ptr [bp - 0x12]
c3ea: mov si, word ptr [si]
c3ec: mov al, byte ptr [bx + si + 0x28]
c3ef: sub ah, ah
c3f1: shl ax
c3f3: add ax, word ptr [bp - 0xc]
c3f6: push ax
c3f7: lea ax, [bp - 0xe]
c3fa: push ax
c3fb: call 0xc715
c3fe: cmp byte ptr [bp + 0x6], 0x1
c402: jne 0xc415
c404: mov si, word ptr [bp - 0x16]
c407: mov bx, word ptr [-0x3d6e]
c40b: dec byte ptr [bx + si + 0x2]
c40e: mov bx, word ptr [-0x3d6e]
c412: dec byte ptr [bx + si + 0x3]
c415: mov bx, word ptr [bp - 0x8]
c418: mov si, word ptr [bp - 0x12]
c41b: mov si, word ptr [si]
c41d: mov al, byte ptr [bx + si + 0x28]
c420: mov si, word ptr [bp - 0x16]
c423: mov bx, word ptr [-0x3d6e]
c427: mov byte ptr [bx + si + 0x6], al
c42a: push di
c42b: call 0xca55
c42e: inc di
c42f: inc word ptr [bp - 0x8]
c432: cmp word ptr [bp - 0x8], 0x4
c436: jae 0xc43b
c438: jmp 0xc38e
c43b: mov di, 0x1
c43e: mov si, 0x12
c441: jmp 0xc514
c444: mov bx, word ptr [-0x3d6e]
c448: add bx, si
c44a: mov al, byte ptr [bp + 0x6]
c44d: cmp byte ptr [bx + 0xb], al
c450: jne 0xc45e
c452: mov bx, word ptr [-0x3d6e]
c456: add bx, si
c458: cmp byte ptr [bx + 0x6], -0x80
c45c: je 0xc461
c45e: jmp 0xc510
c461: mov word ptr [bp - 0x8], di
c464: sub dx, dx
c466: mov word ptr [bp - 0x4], si
c469: jmp 0xc4f4
c46c: mov bx, word ptr [-0x3d6e]
c470: add bx, si
c472: mov al, byte ptr [bx + 0xa]
c475: mov byte ptr [bp - 0x1], al
c478: cmp word ptr [bp - 0x10], 0x11
c47c: ja 0xc4fb
c47e: mov word ptr [bp - 0xa], dx
c481: cmp al, 0x3
c483: jne 0xc490
c485: mov byte ptr [bp - 0x5], 0x10
c489: mov word ptr [bp - 0xa], 0xffff
c48e: jmp 0xc499
c490: mov al, byte ptr [bp - 0x10]
c493: mov byte ptr [bp - 0x5], al
c496: inc word ptr [bp - 0x10]
c499: mov ax, word ptr [0xc292]
c49c: add ax, si
c49e: inc ax
c49f: inc ax
c4a0: push ax
c4a1: mov al, byte ptr [bp + 0x8]
c4a4: push ax
c4a5: mov al, byte ptr [bp - 0x5]
c4a8: sub ah, ah
c4aa: shl ax
c4ac: add ax, word ptr [bp - 0xc]
c4af: push ax
c4b0: lea ax, [bp - 0xe]
c4b3: push ax
c4b4: call 0xc715
c4b7: cmp byte ptr [bp + 0x6], 0x1
c4bb: jne 0xc4dc
c4bd: mov al, 0x1
c4bf: mov bl, byte ptr [bp - 0x1]
c4c2: sub bh, bh
c4c4: sub al, byte ptr [bx + 0x6ae8]
c4c8: mov bx, word ptr [-0x3d6e]
c4cc: add byte ptr [bx + si + 0x2], al
c4cf: cmp byte ptr [bp - 0x1], 0x2
c4d3: je 0xc4dc
c4d5: mov bx, word ptr [-0x3d6e]
c4d9: dec byte ptr [bx + si + 0x3]
c4dc: mov ax, word ptr [0xc292]
c4df: add ax, si
c4e1: push ax
c4e2: push word ptr [bp - 0x8]
c4e5: call 0xc7e7
c4e8: or al, al
c4ea: jne 0xc4f1
c4ec: mov dx, 0xffff
c4ef: jmp 0xc4f4
c4f1: mov dx, word ptr [bp - 0xa]
c4f4: or dx, dx
c4f6: jne 0xc4fb
c4f8: jmp 0xc46c
c4fb: mov di, word ptr [bp - 0x8]
c4fe: or dx, dx
c500: je 0xc510
c502: mov bx, word ptr [-0x3d6e]
c506: mov al, byte ptr [bp - 0x5]
c509: mov byte ptr [bx + si + 0x6], al
c50c: push di
c50d: call 0xca55
c510: add si, 0x12
c513: inc di
c514: cmp word ptr [-0x3d6c], di
c518: jbe 0xc51d
c51a: jmp 0xc444
c51d: pop si
c51e: pop di
c51f: mov sp, bp
c521: pop bp
c522: ret 0x6
c525: push bp
c526: mov bp, sp
c528: sub sp, 0x10
c52b: push di
c52c: push si
c52d: mov cx, 0x1
c530: mov dx, 0x12
c533: cmp word ptr [-0x3d6c], cx
c537: ja 0xc53c
c539: jmp 0xc5e4
c53c: mov bx, word ptr [-0x3d6e]
c540: add bx, dx
c542: mov al, byte ptr [bp + 0x6]
c545: cmp byte ptr [bx + 0xb], al
c548: jne 0xc56e
c54a: mov al, byte ptr [bx + 0x6]
c54d: mov byte ptr [bp - 0x9], al
c550: mov byte ptr [bx + 0x6], -0x80
c554: cmp byte ptr [bp - 0x9], -0x2
c558: jne 0xc574
c55a: mov si, word ptr [-0x3d6e]
c55e: mov bx, dx
c560: mov byte ptr [bx + si + 0x6], 0x4b
c564: mov si, word ptr [-0x3d6e]
c568: mov bx, dx
c56a: mov byte ptr [bx + si + 0xd], 0x1
c56e: add dx, 0x12
c571: inc cx
c572: jmp 0xc533
c574: mov word ptr [bp - 0xc], cx
c577: sub si, si
c579: mov ax, word ptr [bp + 0x4]
c57c: inc ax
c57d: inc ax
c57e: mov word ptr [bp - 0x4], ax
c581: cmp si, 0x3
c584: jae 0xc5ce
c586: mov bx, word ptr [bp - 0x4]
c589: mov ax, word ptr [bx]
c58b: cmp ax, 0xffff
c58e: je 0xc5d3
c590: mov bl, byte ptr [bp - 0x9]
c593: sub bh, bh
c595: add bx, ax
c597: mov di, word ptr [0x76ea]
c59b: mov al, byte ptr [bx + di]
c59d: mov cl, 0x6
c59f: mov bx, ax
c5a1: shr al, cl
c5a3: mov byte ptr [bp - 0x1], al
c5a6: and bl, 0x3f
c5a9: mov byte ptr [bp - 0x5], bl
c5ac: mov bx, word ptr [-0x3d6e]
c5b0: add bx, dx
c5b2: cmp byte ptr [bx + 0xb], 0x1
c5b6: jne 0xc5bf
c5b8: sub al, -0x2
c5ba: and al, 0x3
c5bc: mov byte ptr [bp - 0x1], al
c5bf: mov cl, 0x6
c5c1: mov al, byte ptr [bp - 0x1]
c5c4: shl al, cl
c5c6: or al, byte ptr [bp - 0x5]
c5c9: mov byte ptr [bx + si + 0x6], al
c5cc: jmp 0xc5dd
c5ce: mov cx, word ptr [bp - 0xc]
c5d1: jmp 0xc564
c5d3: mov bx, word ptr [-0x3d6e]
c5d7: add bx, dx
c5d9: mov byte ptr [bx + si + 0x6], -0x80
c5dd: add word ptr [bp - 0x4], 0x2
c5e1: inc si
c5e2: jmp 0xc581
c5e4: pop si
c5e5: pop di
c5e6: mov sp, bp
c5e8: pop bp
c5e9: ret 0x4
c5ec: push bp
c5ed: mov bp, sp
c5ef: sub sp, 0x2
c5f2: push di
c5f3: push si
c5f4: mov di, word ptr [bp + 0xe]
c5f7: jmp 0xc706
c5fa: mov ax, 0x12
c5fd: mul word ptr [-0x55b6]
c601: mov si, ax
c603: mov bx, word ptr [-0x3d6e]
c607: mov al, byte ptr [bp + 0x6]
c60a: mov byte ptr [bx + si + 0xb], al
c60d: mov ax, 0x12
c610: mul word ptr [-0x55b6]
c614: mov si, ax
c616: mov bx, word ptr [-0x3d6e]
c61a: mov al, byte ptr [bp + 0xa]
c61d: mov byte ptr [bx + si + 0xa], al
c620: mov ax, 0x12
c623: mul word ptr [-0x55b6]
c627: mov si, ax
c629: mov bx, word ptr [-0x3d6e]
c62d: mov al, byte ptr [bp + 0x8]
c630: mov byte ptr [bx + si + 0xe], al
c633: mov ax, 0x12
c636: mul word ptr [-0x55b6]
c63a: mov si, ax
c63c: mov bx, word ptr [-0x3d6e]
c640: mov word ptr [bx + si + 0x4], 0xffff
c645: mov ax, 0x12
c648: mul word ptr [-0x55b6]
c64c: mov bx, ax
c64e: mov si, word ptr [-0x3d6e]
c652: mov ax, word ptr [bp + 0xc]
c655: cmp ax, di
c657: jle 0xc65b
c659: mov ax, di
c65b: mov word ptr [bx + si], ax
c65d: sub di, word ptr [bp + 0xc]
c660: mov ax, 0x12
c663: mul word ptr [-0x55b6]
c667: mov si, ax
c669: mov bx, word ptr [-0x3d6e]
c66d: mov al, byte ptr [bp + 0x4]
c670: mov byte ptr [bx + si + 0xc], al
c673: mov ax, 0x12
c676: mul word ptr [-0x55b6]
c67a: mov si, ax
c67c: mov bx, word ptr [-0x3d6e]
c680: mov byte ptr [bx + si + 0xd], 0x0
c684: mov ax, 0x12
c687: mul word ptr [-0x55b6]
c68b: mov si, ax
c68d: mov bx, word ptr [-0x3d6e]
c691: mov byte ptr [bx + si + 0xf], 0x0
c695: mov bl, byte ptr [bp + 0x6]
c698: sub bh, bh
c69a: mov si, word ptr [bp + 0xa]
c69d: and si, 0xff
c6a1: shl si
c6a3: mov al, byte ptr [bx + si + 0x7244]
c6a7: mov cx, ax
c6a9: mov ax, 0x12
c6ac: mul word ptr [-0x55b6]
c6b0: mov si, ax
c6b2: mov bx, word ptr [-0x3d6e]
c6b6: mov byte ptr [bx + si + 0x9], cl
c6b9: mov ax, 0x12
c6bc: mul word ptr [-0x55b6]
c6c0: mov si, ax
c6c2: mov bx, word ptr [-0x3d6e]
c6c6: mov byte ptr [bx + si + 0x10], 0x0
c6ca: mov ax, 0x12
c6cd: mul word ptr [-0x55b6]
c6d1: mov si, ax
c6d3: mov bx, word ptr [-0x3d6e]
c6d7: mov byte ptr [bx + si + 0x11], 0x0
c6db: mov word ptr [bp - 0x2], 0x0
c6e0: mov word ptr [bp + 0xe], di
c6e3: jmp 0xc6fc
c6e5: mov bx, word ptr [bp - 0x2]
c6e8: mov ax, 0x12
c6eb: mul word ptr [-0x55b6]
c6ef: mov si, ax
c6f1: add si, word ptr [-0x3d6e]
c6f5: mov byte ptr [bx + si + 0x6], -0x80
c6f9: inc word ptr [bp - 0x2]
c6fc: cmp word ptr [bp - 0x2], 0x3
c700: jb 0xc6e5
c702: inc word ptr [-0x55b6]
c706: or di, di
c708: jle 0xc70d
c70a: jmp 0xc5fa
c70d: pop si
c70e: pop di
c70f: mov sp, bp
c711: pop bp
c712: ret 0xc
c715: push bp
c716: mov bp, sp
c718: push si
c719: mov bx, word ptr [bp + 0x4]
c71c: mov ax, word ptr [bx]
c71e: mov bx, word ptr [bp + 0xa]
c721: mov word ptr [bx], ax
c723: cmp byte ptr [bp + 0x8], 0x2
c727: jne 0xc73b
c729: mov bx, word ptr [bp + 0x6]
c72c: mov al, byte ptr [bx]
c72e: mov si, word ptr [bp + 0xa]
c731: add byte ptr [si], al
c733: mov al, byte ptr [bx + 0x1]
c736: add byte ptr [si + 0x1], al
c739: jmp 0xc74b
c73b: mov bx, word ptr [bp + 0x6]
c73e: mov al, byte ptr [bx]
c740: mov si, word ptr [bp + 0xa]
c743: sub byte ptr [si], al
c745: mov al, byte ptr [bx + 0x1]
c748: sub byte ptr [si + 0x1], al
c74b: pop si
c74c: mov sp, bp
c74e: pop bp
c74f: ret 0x8
c752: push bp
c753: mov bp, sp
c755: sub sp, 0x2
c758: push di
c759: push si
c75a: mov bx, word ptr [bp + 0x6]
c75d: mov ax, word ptr [bx + 0x2]
c760: dec ax
c761: dec ax
c762: shl ax
c764: mov cx, word ptr [bx]
c766: sub cx, 0x4
c769: mov dx, ax
c76b: add ax, cx
c76d: mov bx, dx
c76f: mov si, cx
c771: cdq
c772: xor ax, dx
c774: sub ax, dx
c776: mov cx, 0x4
c779: sar ax, cl
c77b: xor ax, dx
c77d: sub ax, dx
c77f: mov word ptr [bp - 0x2], ax
c782: sub bx, si
c784: mov di, bx
c786: or bx, bx
c788: jge 0xc78d
c78a: sub di, 0xf
c78d: mov al, byte ptr [bp - 0x2]
c790: mov bx, word ptr [bp + 0x4]
c793: mov byte ptr [bx], al
c795: mov ax, di
c797: cdq
c798: xor ax, dx
c79a: sub ax, dx
c79c: sar ax, cl
c79e: xor ax, dx
c7a0: sub ax, dx
c7a2: add ax, 0x14
c7a5: mov byte ptr [bx + 0x1], al
c7a8: pop si
c7a9: pop di
c7aa: mov sp, bp
c7ac: pop bp
c7ad: ret 0x4
c7b0: push bp
c7b1: mov bp, sp
c7b3: push di
c7b4: mov bx, word ptr [bp + 0x6]
c7b7: mov al, byte ptr [bx]
c7b9: cwde
c7ba: mov dx, ax
c7bc: mov cx, dx
c7be: mov al, byte ptr [bx + 0x1]
c7c1: cwde
c7c2: mov di, ax
c7c4: sub cx, ax
c7c6: mov ax, cx
c7c8: mov cl, 0x3
c7ca: shl ax, cl
c7cc: add ax, 0xa4
c7cf: mov bx, word ptr [bp + 0x4]
c7d2: mov word ptr [bx], ax
c7d4: add dx, di
c7d6: shl dx
c7d8: shl dx
c7da: sub dx, 0x4a
c7dd: mov word ptr [bx + 0x2], dx
c7e0: pop di
c7e1: mov sp, bp
c7e3: pop bp
c7e4: ret 0x4
c7e7: push bp
c7e8: mov bp, sp
c7ea: sub sp, 0x6
c7ed: push di
c7ee: push si
c7ef: mov si, word ptr [bp + 0x6]
c7f2: mov byte ptr [bp - 0x6], 0x0
c7f6: mov bl, byte ptr [si + 0xa]
c7f9: sub bh, bh
c7fb: mov al, byte ptr [bx + 0x6ae8]
c7ff: mov byte ptr [bp - 0x3], al
c802: mov al, byte ptr [bx + 0x6aec]
c806: mov byte ptr [bp - 0x4], al
c809: cmp byte ptr [si + 0xe], 0x3
c80d: je 0xc815
c80f: cmp byte ptr [si + 0xe], 0x1
c813: jne 0xc827
c815: mov al, byte ptr [bp - 0x4]
c818: xor byte ptr [bp - 0x3], al
c81b: mov al, byte ptr [bp - 0x3]
c81e: xor byte ptr [bp - 0x4], al
c821: xor al, byte ptr [bp - 0x4]
c824: mov byte ptr [bp - 0x3], al
c827: mov byte ptr [bp - 0x5], 0x0
c82b: jmp 0xc87d
c82d: mov byte ptr [bp - 0x1], 0x0
c831: jmp 0xc872
c833: mov al, byte ptr [si + 0x2]
c836: add al, byte ptr [bp - 0x5]
c839: mov cl, 0x30
c83b: mul cl
c83d: mov bx, ax
c83f: mov al, byte ptr [si + 0x3]
c842: add al, byte ptr [bp - 0x1]
c845: sub ah, ah
c847: add bx, ax
c849: mov di, word ptr [-0x3d52]
c84d: mov al, byte ptr [bx + di]
c84f: mov byte ptr [bp - 0x2], al
c852: cmp al, -0x2
c854: jne 0xc85b
c856: mov byte ptr [bp - 0x6], al
c859: jmp 0xc86f
c85b: cmp byte ptr [bp - 0x2], 0x0
c85f: je 0xc86f
c861: mov al, byte ptr [bp - 0x2]
c864: sub ah, ah
c866: cmp ax, word ptr [bp + 0x4]
c869: je 0xc86f
c86b: mov byte ptr [bp - 0x6], -0x1
c86f: inc byte ptr [bp - 0x1]
c872: mov al, byte ptr [bp - 0x1]
c875: cmp byte ptr [bp - 0x3], al
c878: ja 0xc833
c87a: inc byte ptr [bp - 0x5]
c87d: mov al, byte ptr [bp - 0x4]
c880: cmp byte ptr [bp - 0x5], al
c883: jb 0xc82d
c885: mov al, byte ptr [bp - 0x6]
c888: pop si
c889: pop di
c88a: mov sp, bp
c88c: pop bp
c88d: ret 0x4
c890: push bp
c891: mov bp, sp
c893: sub sp, 0x8
c896: push di
c897: push si
c898: mov si, word ptr [bp + 0x4]
c89b: mov byte ptr [bp - 0x7], 0x0
c89f: mov bl, byte ptr [si + 0xa]
c8a2: sub bh, bh
c8a4: mov al, byte ptr [bx + 0x6ae8]
c8a8: mov byte ptr [bp - 0x3], al
c8ab: mov al, byte ptr [bx + 0x6aec]
c8af: mov byte ptr [bp - 0x4], al
c8b2: cmp byte ptr [si + 0xe], 0x3
c8b6: je 0xc8be
c8b8: cmp byte ptr [si + 0xe], 0x1
c8bc: jne 0xc8d0
c8be: mov al, byte ptr [bp - 0x4]
c8c1: xor byte ptr [bp - 0x3], al
c8c4: mov al, byte ptr [bp - 0x3]
c8c7: xor byte ptr [bp - 0x4], al
c8ca: xor al, byte ptr [bp - 0x4]
c8cd: mov byte ptr [bp - 0x3], al
c8d0: mov al, byte ptr [si + 0xb]
c8d3: mov byte ptr [bp - 0x6], al
c8d6: mov byte ptr [bp - 0x5], 0x0
c8da: jmp 0xc935
c8dc: mov byte ptr [bp - 0x1], 0x0
c8e0: jmp 0xc92a
c8e2: mov al, byte ptr [si + 0x2]
c8e5: add al, byte ptr [bp - 0x5]
c8e8: mov cl, 0x30
c8ea: mul cl
c8ec: mov bx, ax
c8ee: mov al, byte ptr [si + 0x3]
c8f1: add al, byte ptr [bp - 0x1]
c8f4: sub ah, ah
c8f6: add bx, ax
c8f8: mov di, word ptr [-0x3d52]
c8fc: mov al, byte ptr [bx + di]
c8fe: mov byte ptr [bp - 0x2], al
c901: cmp al, -0x2
c903: jne 0xc90a
c905: mov byte ptr [bp - 0x7], al
c908: jmp 0xc927
c90a: cmp byte ptr [bp - 0x2], 0x0
c90e: je 0xc927
c910: mov al, 0x12
c912: mul byte ptr [bp - 0x2]
c915: mov di, ax
c917: mov bx, word ptr [-0x3d6e]
c91b: mov al, byte ptr [bp - 0x6]
c91e: cmp byte ptr [bx + di + 0xb], al
c921: je 0xc927
c923: mov byte ptr [bp - 0x7], -0x1
c927: inc byte ptr [bp - 0x1]
c92a: mov al, byte ptr [bp - 0x1]
c92d: cmp byte ptr [bp - 0x3], al
c930: ja 0xc8e2
c932: inc byte ptr [bp - 0x5]
c935: mov al, byte ptr [bp - 0x5]
c938: cmp byte ptr [bp - 0x4], al
c93b: ja 0xc8dc
c93d: mov al, byte ptr [bp - 0x7]
c940: pop si
c941: pop di
c942: mov sp, bp
c944: pop bp
c945: ret 0x2
c948: push bp
c949: mov bp, sp
c94b: sub sp, 0x4
c94e: push di
c94f: push si
c950: mov ax, 0x12
c953: mul word ptr [bp + 0x8]
c956: mov bx, ax
c958: add bx, word ptr [-0x3d6e]
c95c: mov ax, bx
c95e: mov bl, byte ptr [bx + 0xa]
c961: sub bh, bh
c963: mov cl, byte ptr [bx + 0x6ae8]
c967: mov byte ptr [bp - 0x1], cl
c96a: mov cl, byte ptr [bx + 0x6aec]
c96e: mov byte ptr [bp - 0x4], cl
c971: mov bx, ax
c973: mov cl, byte ptr [bx + 0x2]
c976: mov byte ptr [bp - 0x2], cl
c979: mov cl, byte ptr [bx + 0x3]
c97c: mov byte ptr [bp - 0x3], cl
c97f: mov al, byte ptr [bp + 0x6]
c982: sub ah, ah
c984: or ax, ax
c986: je 0xc997
c988: dec ax
c989: je 0xc9cb
c98b: dec ax
c98c: je 0xc9fa
c98e: dec ax
c98f: jne 0xc994
c991: jmp 0xca22
c994: jmp 0xca4d
c997: sub si, si
c999: mov al, byte ptr [bp - 0x1]
c99c: sub ah, ah
c99e: cmp ax, si
c9a0: ja 0xc9a5
c9a2: jmp 0xca4d
c9a5: mov al, byte ptr [bp - 0x2]
c9a8: mov cl, byte ptr [bp - 0x4]
c9ab: sub ch, ch
c9ad: add ax, cx
c9af: mov cx, 0x30
c9b2: mul cx
c9b4: mov bx, ax
c9b6: mov al, byte ptr [bp - 0x3]
c9b9: sub ah, ah
c9bb: add bx, ax
c9bd: add bx, word ptr [-0x3d52]
c9c1: mov al, byte ptr [bx + si]
c9c3: mov bx, word ptr [bp + 0x4]
c9c6: mov byte ptr [bx + si], al
c9c8: inc si
c9c9: jmp 0xc999
c9cb: sub si, si
c9cd: mov al, 0x30
c9cf: mul byte ptr [bp - 0x2]
c9d2: mov di, ax
c9d4: mov al, byte ptr [bp - 0x3]
c9d7: sub ah, ah
c9d9: add di, ax
c9db: mov al, byte ptr [bp - 0x4]
c9de: add di, ax
c9e0: mov al, byte ptr [bp - 0x1]
c9e3: sub ah, ah
c9e5: cmp ax, si
c9e7: jbe 0xca4d
c9e9: mov bx, word ptr [-0x3d52]
c9ed: mov al, byte ptr [bx + di]
c9ef: mov bx, word ptr [bp + 0x4]
c9f2: mov byte ptr [bx + si], al
c9f4: add di, 0x30
c9f7: inc si
c9f8: jmp 0xc9e0
c9fa: sub si, si
c9fc: mov al, byte ptr [bp - 0x1]
c9ff: sub ah, ah
ca01: cmp ax, si
ca03: jbe 0xca4d
ca05: mov al, 0x30
ca07: mul byte ptr [bp - 0x2]
ca0a: mov bx, ax
ca0c: mov al, byte ptr [bp - 0x3]
ca0f: sub ah, ah
ca11: add bx, ax
ca13: add bx, word ptr [-0x3d52]
ca17: mov al, byte ptr [bx + si - 0x30]
ca1a: mov bx, word ptr [bp + 0x4]
ca1d: mov byte ptr [bx + si], al
ca1f: inc si
ca20: jmp 0xc9fc
ca22: sub si, si
ca24: mov al, 0x30
ca26: mul byte ptr [bp - 0x2]
ca29: mov di, ax
ca2b: mov al, byte ptr [bp - 0x3]
ca2e: sub ah, ah
ca30: add di, ax
ca32: mov al, byte ptr [bp - 0x1]
ca35: sub ah, ah
ca37: cmp ax, si
ca39: jbe 0xca4d
ca3b: mov bx, word ptr [-0x3d52]
ca3f: mov al, byte ptr [bx + di - 0x1]
ca42: mov bx, word ptr [bp + 0x4]
ca45: mov byte ptr [bx + si], al
ca47: add di, 0x30
ca4a: inc si
ca4b: jmp 0xca32
ca4d: pop si
ca4e: pop di
ca4f: mov sp, bp
ca51: pop bp
ca52: ret 0x6
ca55: push bp
ca56: mov bp, sp
ca58: sub sp, 0xa
ca5b: push di
ca5c: push si
ca5d: mov si, word ptr [bp + 0x4]
ca60: mov ax, si
ca62: mov cx, 0x12
ca65: mul cx
ca67: mov bx, ax
ca69: mov word ptr [bp - 0xa], bx
ca6c: add bx, word ptr [-0x3d6e]
ca70: mov word ptr [bp - 0x8], bx
ca73: cmp byte ptr [bx + 0xd], 0x4
ca77: jne 0xca7c
ca79: jmp 0xcb34
ca7c: mov bl, byte ptr [bx + 0xa]
ca7f: sub bh, bh
ca81: mov al, byte ptr [bx + 0x6ae8]
ca85: mov byte ptr [bp - 0x4], al
ca88: mov al, byte ptr [bx + 0x6aec]
ca8c: mov byte ptr [bp - 0x5], al
ca8f: mov bx, word ptr [bp - 0x8]
ca92: cmp byte ptr [bx + 0xe], 0x2
ca96: je 0xca9e
ca98: cmp byte ptr [bx + 0xe], 0x0
ca9c: jne 0xcab0
ca9e: mov al, byte ptr [bp - 0x5]
caa1: xor byte ptr [bp - 0x4], al
caa4: mov al, byte ptr [bp - 0x4]
caa7: xor byte ptr [bp - 0x5], al
caaa: xor al, byte ptr [bp - 0x5]
caad: mov byte ptr [bp - 0x4], al
cab0: mov byte ptr [bp - 0x6], 0x0
cab4: mov al, byte ptr [bp - 0x6]
cab7: cmp byte ptr [bp - 0x4], al
caba: jbe 0xcb34
cabc: mov byte ptr [bp - 0x1], 0x0
cac0: jmp 0xcb18
cac2: mov bx, word ptr [bp - 0xa]
cac5: add bx, word ptr [-0x3d6e]
cac9: mov al, byte ptr [bx + 0x2]
cacc: add al, byte ptr [bp - 0x6]
cacf: mov byte ptr [bp - 0x2], al
cad2: mov cl, byte ptr [bx + 0x3]
cad5: add cl, byte ptr [bp - 0x1]
cad8: mov byte ptr [bp - 0x3], cl
cadb: mov dl, 0x30
cadd: mul dl
cadf: mov bx, ax
cae1: sub ch, ch
cae3: add bx, cx
cae5: mov di, word ptr [-0x3d52]
cae9: cmp byte ptr [bx + di], ch
caeb: jne 0xcafe
caed: mov al, dl
caef: mul byte ptr [bp - 0x2]
caf2: mov bx, ax
caf4: mov al, cl
caf6: sub ah, ah
caf8: add bx, ax
cafa: mov ax, si
cafc: mov byte ptr [bx + di], al
cafe: mov al, 0x30
cb00: mul byte ptr [bp - 0x2]
cb03: mov bx, ax
cb05: mov al, byte ptr [bp - 0x3]
cb08: sub ah, ah
cb0a: add bx, ax
cb0c: mov di, word ptr [-0x3d52]
cb10: cmp byte ptr [bx + di], -0x2
cb13: je 0xcb25
cb15: inc byte ptr [bp - 0x1]
cb18: mov al, byte ptr [bp - 0x1]
cb1b: cmp byte ptr [bp - 0x5], al
cb1e: ja 0xcac2
cb20: inc byte ptr [bp - 0x6]
cb23: jmp 0xcab4
cb25: mov di, word ptr [bp - 0xa]
cb28: mov bx, word ptr [-0x3d6e]
cb2c: mov byte ptr [bx + di + 0xd], 0x4
cb30: push si
cb31: call 0xcb3c
cb34: pop si
cb35: pop di
cb36: mov sp, bp
cb38: pop bp
cb39: ret 0x2
cb3c: push bp
cb3d: mov bp, sp
cb3f: sub sp, 0xa
cb42: push di
cb43: push si
cb44: mov si, word ptr [bp + 0x4]
cb47: mov ax, si
cb49: mov cx, 0x12
cb4c: mul cx
cb4e: mov bx, ax
cb50: mov word ptr [bp - 0x8], bx
cb53: add bx, word ptr [-0x3d6e]
cb57: mov ax, bx
cb59: mov bl, byte ptr [bx + 0xa]
cb5c: sub bh, bh
cb5e: mov cl, byte ptr [bx + 0x6ae8]
cb62: mov byte ptr [bp - 0x4], cl
cb65: mov cl, byte ptr [bx + 0x6aec]
cb69: mov byte ptr [bp - 0x5], cl
cb6c: mov bx, ax
cb6e: mov al, byte ptr [bx + 0xe]
cb71: cmp al, 0x2
cb73: je 0xcb79
cb75: or al, al
cb77: jne 0xcb8b
cb79: mov al, byte ptr [bp - 0x5]
cb7c: xor byte ptr [bp - 0x4], al
cb7f: mov al, byte ptr [bp - 0x4]
cb82: xor byte ptr [bp - 0x5], al
cb85: xor al, byte ptr [bp - 0x5]
cb88: mov byte ptr [bp - 0x4], al
cb8b: mov byte ptr [bp - 0x6], 0x0
cb8f: jmp 0xcbde
cb91: mov byte ptr [bp - 0x1], 0x0
cb95: jmp 0xcbd3
cb97: mov bx, word ptr [bp - 0x8]
cb9a: add bx, word ptr [-0x3d6e]
cb9e: mov al, byte ptr [bx + 0x2]
cba1: add al, byte ptr [bp - 0x6]
cba4: mov byte ptr [bp - 0x2], al
cba7: mov cl, byte ptr [bx + 0x3]
cbaa: add cl, byte ptr [bp - 0x1]
cbad: mov dl, 0x30
cbaf: mul dl
cbb1: mov bx, ax
cbb3: sub ch, ch
cbb5: add bx, cx
cbb7: mov di, word ptr [-0x3d52]
cbbb: mov ax, si
cbbd: cmp byte ptr [bx + di], al
cbbf: jne 0xcbd0
cbc1: mov al, dl
cbc3: mul byte ptr [bp - 0x2]
cbc6: mov bx, ax
cbc8: mov al, cl
cbca: sub ah, ah
cbcc: add bx, ax
cbce: mov byte ptr [bx + di], ah
cbd0: inc byte ptr [bp - 0x1]
cbd3: mov al, byte ptr [bp - 0x1]
cbd6: cmp byte ptr [bp - 0x5], al
cbd9: ja 0xcb97
cbdb: inc byte ptr [bp - 0x6]
cbde: mov al, byte ptr [bp - 0x6]
cbe1: cmp byte ptr [bp - 0x4], al
cbe4: ja 0xcb91
cbe6: pop si
cbe7: pop di
cbe8: mov sp, bp
cbea: pop bp
cbeb: ret 0x2
cbee: push bp
cbef: mov bp, sp
cbf1: sub sp, 0xa
cbf4: push di
cbf5: mov byte ptr [bp - 0x1], 0x0
cbf9: mov ax, 0x12
cbfc: mul word ptr [bp + 0x8]
cbff: mov bx, ax
cc01: add bx, word ptr [-0x3d6e]
cc05: mov al, byte ptr [bx + 0x2]
cc08: cwde
cc09: mov cl, byte ptr [bp + 0x6]
cc0c: sub ch, ch
cc0e: sub ax, cx
cc10: mov word ptr [bp - 0x4], ax
cc13: mov cl, byte ptr [bp + 0x4]
cc16: mov dx, ax
cc18: mov al, byte ptr [bx + 0x3]
cc1b: cwde
cc1c: sub ax, cx
cc1e: mov word ptr [bp - 0x8], ax
cc21: mov cx, ax
cc23: mov ax, dx
cc25: cdq
cc26: xor ax, dx
cc28: sub ax, dx
cc2a: mov di, ax
cc2c: mov dx, ax
cc2e: mov ax, cx
cc30: mov cx, di
cc32: cdq
cc33: xor ax, dx
cc35: sub ax, dx
cc37: mov word ptr [bp - 0x6], ax
cc3a: cmp ax, cx
cc3c: jge 0xcc40
cc3e: mov ax, cx
cc40: mov byte ptr [bp - 0x9], al
cc43: or al, al
cc45: je 0xcc6b
cc47: cmp word ptr [bp - 0x6], di
cc4a: jg 0xcc5b
cc4c: mov byte ptr [bp - 0x1], 0x2
cc50: cmp di, word ptr [bp - 0x4]
cc53: je 0xcc6b
cc55: mov byte ptr [bp - 0x1], 0x0
cc59: jmp 0xcc6b
cc5b: mov byte ptr [bp - 0x1], 0x3
cc5f: mov ax, word ptr [bp - 0x8]
cc62: cmp word ptr [bp - 0x6], ax
cc65: je 0xcc6b
cc67: mov byte ptr [bp - 0x1], 0x1
cc6b: mov al, byte ptr [bp - 0x1]
cc6e: mov cl, 0x6
cc70: shl al, cl
cc72: or al, byte ptr [bp - 0x9]
cc75: pop di
cc76: mov sp, bp
cc78: pop bp
cc79: ret 0x6
cc7c: push bp
cc7d: mov bp, sp
cc7f: sub sp, 0xc
cc82: push di
cc83: push si
cc84: mov si, word ptr [bp + 0x4]
cc87: mov ax, si
cc89: mov cx, 0x12
cc8c: mul cx
cc8e: mov bx, ax
cc90: mov word ptr [bp - 0x8], bx
cc93: add bx, word ptr [-0x3d6e]
cc97: mov word ptr [bp - 0x6], bx
cc9a: test byte ptr [bx + 0x6], 0x3f
cc9e: jne 0xcca8
cca0: mov word ptr [bx + 0x4], 0x0
cca5: jmp 0xcdbf
cca8: push si
cca9: call 0xcb3c
ccac: mov bx, word ptr [bp - 0x8]
ccaf: add bx, word ptr [-0x3d6e]
ccb3: mov word ptr [bp - 0x6], bx
ccb6: mov al, byte ptr [bx + 0xe]
ccb9: mov byte ptr [bp - 0x1], al
ccbc: mov cl, 0x6
ccbe: mov dl, byte ptr [bx + 0x6]
ccc1: sar dl, cl
ccc3: and dl, 0x3
ccc6: mov byte ptr [bp - 0x2], dl
ccc9: cmp dl, al
cccb: jne 0xccd0
cccd: jmp 0xcdb6
ccd0: cmp byte ptr [bx + 0xa], 0x2
ccd4: jne 0xccd9
ccd6: jmp 0xcda5
ccd9: mov al, dl
ccdb: sub ah, ah
ccdd: mov cl, byte ptr [bp - 0x1]
cce0: sub ch, ch
cce2: mov word ptr [bp - 0xc], cx
cce5: sub ax, cx
cce7: cdq
cce8: xor ax, dx
ccea: sub ax, dx
ccec: cmp ax, 0x2
ccef: jne 0xccff
ccf1: mov al, byte ptr [bp - 0x2]
ccf4: sub ah, ah
ccf6: add ax, cx
ccf8: cdq
ccf9: sub ax, dx
ccfb: sar ax
ccfd: jmp 0xcd02
ccff: mov al, byte ptr [bp - 0x2]
cd02: mov byte ptr [bx + 0xe], al
cd05: mov bx, word ptr [bp - 0x8]
cd08: add bx, word ptr [-0x3d6e]
cd0c: mov word ptr [bp - 0x6], bx
cd0f: mov ax, word ptr [bx + 0x2]
cd12: mov word ptr [bp - 0x4], ax
cd15: cmp byte ptr [bx + 0xa], 0x1
cd19: je 0xcd21
cd1b: cmp byte ptr [bx + 0xa], 0x3
cd1f: jne 0xcd55
cd21: mov bx, word ptr [bp - 0xc]
cd24: shl bx
cd26: shl bx
cd28: mov di, word ptr [bp - 0x6]
cd2b: mov di, word ptr [di + 0xe]
cd2e: and di, 0xff
cd32: mov al, byte ptr [bx + di + 0x740a]
cd36: mov di, word ptr [bp - 0x6]
cd39: add byte ptr [di + 0x2], al
cd3c: mov di, word ptr [bp - 0x8]
cd3f: add di, word ptr [-0x3d6e]
cd43: mov ax, di
cd45: mov di, word ptr [di + 0xe]
cd48: and di, 0xff
cd4c: mov cl, byte ptr [bx + di + 0x741a]
cd50: mov bx, ax
cd52: add byte ptr [bx + 0x3], cl
cd55: mov ax, word ptr [bp - 0x8]
cd58: add ax, word ptr [-0x3d6e]
cd5c: push ax
cd5d: push si
cd5e: call 0xc7e7
cd61: cmp al, -0x1
cd63: jne 0xcd85
cd65: mov di, word ptr [bp - 0x8]
cd68: mov bx, word ptr [-0x3d6e]
cd6c: mov ax, word ptr [bp - 0x4]
cd6f: mov word ptr [bx + di + 0x2], ax
cd72: mov bx, word ptr [-0x3d6e]
cd76: mov al, byte ptr [bp - 0x1]
cd79: mov byte ptr [bx + di + 0xe], al
cd7c: push si
cd7d: sub al, al
cd7f: push ax
cd80: call 0xb925
cd83: jmp 0xcdbb
cd85: mov bx, word ptr [bp - 0x8]
cd88: add bx, word ptr [-0x3d6e]
cd8c: mov al, byte ptr [bp - 0x2]
cd8f: cmp byte ptr [bx + 0xe], al
cd92: jne 0xcd99
cd94: mov word ptr [bx + 0x4], 0x0
cd99: push si
cd9a: mov cl, 0x4
cd9c: mov al, byte ptr [bp - 0x1]
cd9f: shl al, cl
cda1: or al, 0x2
cda3: jmp 0xcd7f
cda5: mov word ptr [bx + 0x4], 0x0
cdaa: mov di, word ptr [bp - 0x8]
cdad: mov bx, word ptr [-0x3d6e]
cdb1: mov al, byte ptr [bp - 0x2]
cdb4: jmp 0xcd79
cdb6: mov word ptr [bx + 0x4], 0x0
cdbb: push si
cdbc: call 0xca55
cdbf: pop si
cdc0: pop di
cdc1: mov sp, bp
cdc3: pop bp
cdc4: ret 0x2
cdc7: push bp
cdc8: mov bp, sp
cdca: sub sp, 0x10
cdcd: push di
cdce: push si
cdcf: mov di, word ptr [bp + 0x4]
cdd2: mov word ptr [bp - 0x8], 0x0
cdd7: mov ax, di
cdd9: mov cx, 0x12
cddc: mul cx
cdde: mov bx, ax
cde0: mov word ptr [bp - 0x10], bx
cde3: add bx, word ptr [-0x3d6e]
cde7: mov word ptr [bp - 0xe], bx
cdea: cmp word ptr [bx], 0x0
cded: je 0xce04
cdef: cmp byte ptr [bx + 0xd], 0x4
cdf3: je 0xce04
cdf5: cmp byte ptr [bx + 0x10], 0x4
cdf9: je 0xce04
cdfb: mov al, byte ptr [bx + 0x10]
cdfe: and al, 0xf
ce00: cmp al, 0x2
ce02: jne 0xce07
ce04: jmp 0xd008
ce07: mov al, byte ptr [bx + 0x6]
ce0a: mov cx, ax
ce0c: and al, 0x3f
ce0e: mov byte ptr [bp - 0x1], al
ce11: cmp cl, -0x80
ce14: je 0xce1a
ce16: or al, al
ce18: jne 0xce1d
ce1a: jmp 0xcff9
ce1d: mov ax, word ptr [bx + 0x2]
ce20: mov word ptr [bp - 0xa], ax
ce23: push di
ce24: call 0xcb3c
ce27: mov cl, 0x6
ce29: mov bx, word ptr [bp - 0x10]
ce2c: add bx, word ptr [-0x3d6e]
ce30: mov word ptr [bp - 0xe], bx
ce33: mov al, byte ptr [bx + 0x6]
ce36: sar al, cl
ce38: and al, 0x3
ce3a: mov byte ptr [bp - 0x5], al
ce3d: sub ah, ah
ce3f: or ax, ax
ce41: je 0xce4e
ce43: dec ax
ce44: je 0xce53
ce46: dec ax
ce47: je 0xce58
ce49: dec ax
ce4a: je 0xce5d
ce4c: jmp 0xce60
ce4e: inc byte ptr [bx + 0x2]
ce51: jmp 0xce60
ce53: inc byte ptr [bx + 0x3]
ce56: jmp 0xce60
ce58: dec byte ptr [bx + 0x2]
ce5b: jmp 0xce60
ce5d: dec byte ptr [bx + 0x3]
ce60: dec byte ptr [bp - 0x1]
ce63: push di
ce64: mov al, 0x1
ce66: push ax
ce67: call 0xb925
ce6a: mov bx, word ptr [bp - 0x10]
ce6d: add bx, word ptr [-0x3d6e]
ce71: mov word ptr [bp - 0xe], bx
ce74: cmp byte ptr [bx + 0xd], 0x3
ce78: jne 0xced9
ce7a: cmp byte ptr [bx + 0xa], 0x1
ce7e: je 0xced9
ce80: push bx
ce81: call 0xc890
ce84: cmp al, -0x1
ce86: jne 0xcea4
ce88: inc byte ptr [bp - 0x1]
ce8b: mov si, word ptr [bp - 0x10]
ce8e: mov bx, word ptr [-0x3d6e]
ce92: mov ax, word ptr [bp - 0xa]
ce95: mov word ptr [bx + si + 0x2], ax
ce98: push di
ce99: sub al, al
ce9b: push ax
ce9c: call 0xb925
ce9f: mov si, word ptr [bp - 0x8]
cea2: jmp 0xcea7
cea4: mov si, 0xffff
cea7: cmp byte ptr [bp - 0x1], 0x0
ceab: je 0xceb0
cead: jmp 0xcfad
ceb0: mov bx, word ptr [bp - 0x10]
ceb3: add bx, word ptr [-0x3d6e]
ceb7: mov word ptr [bx + 0x4], 0xffff
cebc: mov bx, word ptr [bp - 0x10]
cebf: add bx, word ptr [-0x3d6e]
cec3: mov word ptr [bp - 0xe], bx
cec6: cmp byte ptr [bx + 0xd], 0x2
ceca: je 0xcecf
cecc: jmp 0xcf8d
cecf: push di
ced0: call 0xb5e3
ced3: mov si, 0xffff
ced6: jmp 0xcfc1
ced9: push bx
ceda: push di
cedb: call 0xc7e7
cede: cmp al, -0x1
cee0: jne 0xcea4
cee2: inc byte ptr [bp - 0x1]
cee5: mov si, word ptr [bp - 0x10]
cee8: mov bx, word ptr [-0x3d6e]
ceec: mov ax, word ptr [bp - 0xa]
ceef: mov word ptr [bx + si + 0x2], ax
cef2: push di
cef3: mov bx, word ptr [-0x3d6e]
cef7: mov al, byte ptr [bx + si + 0xe]
cefa: push ax
cefb: lea ax, [bp - 0xc]
cefe: push ax
ceff: call 0xc948
cf02: mov word ptr [bp - 0x4], 0xffff
cf07: sub si, si
cf09: cmp byte ptr [bp + si - 0xc], 0x0
cf0d: je 0xcf30
cf0f: mov bx, word ptr [bp - 0x10]
cf12: add bx, word ptr [-0x3d6e]
cf16: mov al, byte ptr [bx + 0xb]
cf19: mov cx, ax
cf1b: mov al, 0x12
cf1d: mul byte ptr [bp + si - 0xc]
cf20: mov bx, ax
cf22: add bx, word ptr [-0x3d6e]
cf26: cmp byte ptr [bx + 0xb], cl
cf29: je 0xcf30
cf2b: mov word ptr [bp - 0x4], 0x0
cf30: inc si
cf31: cmp si, 0x2
cf34: jb 0xcf09
cf36: cmp word ptr [bp - 0x4], 0x0
cf3a: je 0xcf80
cf3c: mov si, word ptr [bp - 0x10]
cf3f: mov bx, word ptr [-0x3d6e]
cf43: cmp byte ptr [bx + si + 0xd], 0x2
cf47: jne 0xcf80
cf49: mov si, word ptr [bp - 0x8]
cf4c: push di
cf4d: sub al, al
cf4f: push ax
cf50: call 0xb925
cf53: mov bx, word ptr [bp - 0x10]
cf56: add bx, word ptr [-0x3d6e]
cf5a: mov byte ptr [bx + 0x6], -0x80
cf5e: push di
cf5f: call 0xb5e3
cf62: mov cl, 0x6
cf64: mov bx, word ptr [bp - 0x10]
cf67: add bx, word ptr [-0x3d6e]
cf6b: mov al, byte ptr [bx + 0x6]
cf6e: mov dx, ax
cf70: sar al, cl
cf72: and al, 0x3
cf74: mov byte ptr [bp - 0x5], al
cf77: and dl, 0x3f
cf7a: mov byte ptr [bp - 0x1], dl
cf7d: jmp 0xcea7
cf80: mov si, word ptr [bp - 0x8]
cf83: push di
cf84: sub al, al
cf86: push ax
cf87: call 0xb925
cf8a: jmp 0xcea7
cf8d: mov al, byte ptr [bx + 0x7]
cf90: mov byte ptr [bx + 0x6], al
cf93: mov bx, word ptr [bp - 0x10]
cf96: add bx, word ptr [-0x3d6e]
cf9a: mov al, byte ptr [bx + 0x8]
cf9d: mov byte ptr [bx + 0x7], al
cfa0: mov bx, word ptr [bp - 0x10]
cfa3: add bx, word ptr [-0x3d6e]
cfa7: mov byte ptr [bx + 0x8], -0x80
cfab: jmp 0xcfc1
cfad: mov cl, 0x6
cfaf: mov al, byte ptr [bp - 0x5]
cfb2: shl al, cl
cfb4: or al, byte ptr [bp - 0x1]
cfb7: mov bx, word ptr [bp - 0x10]
cfba: add bx, word ptr [-0x3d6e]
cfbe: mov byte ptr [bx + 0x6], al
cfc1: push di
cfc2: call 0xca55
cfc5: mov bx, word ptr [bp - 0x10]
cfc8: add bx, word ptr [-0x3d6e]
cfcc: mov word ptr [bp - 0xe], bx
cfcf: cmp byte ptr [bx + 0x3], 0x1
cfd3: je 0xcfdb
cfd5: cmp byte ptr [bx + 0x3], 0x2c
cfd9: jne 0xcfe3
cfdb: mov byte ptr [bx + 0xd], 0x4
cfdf: push di
cfe0: call 0xcb3c
cfe3: mov di, word ptr [bp - 0x10]
cfe6: mov bx, word ptr [-0x3d6e]
cfea: mov bl, byte ptr [bx + di + 0xb]
cfed: sub bh, bh
cfef: shl bx
cff1: mov word ptr [bx - 0x3d50], si
cff5: mov ax, si
cff7: jmp 0xd00a
cff9: cmp byte ptr [bx + 0xd], 0x2
cffd: jne 0xd008
cfff: push di
d000: call 0xb5e3
d003: mov ax, 0xffff
d006: jmp 0xd00a
d008: sub ax, ax
d00a: pop si
d00b: pop di
d00c: mov sp, bp
d00e: pop bp
d00f: ret 0x2
d012: push bp
d013: mov bp, sp
d015: sub sp, 0x4
d018: push di
d019: push si
d01a: mov ax, 0x12
d01d: mul word ptr [bp + 0x4]
d020: mov bx, ax
d022: mov word ptr [bp - 0x4], bx
d025: mov si, word ptr [-0x3d6e]
d029: cmp word ptr [bx + si], 0x0
d02c: je 0xd086
d02e: add bx, si
d030: mov al, byte ptr [bx + 0xe]
d033: mov byte ptr [bp - 0x1], al
d036: push word ptr [bp + 0x4]
d039: call 0xcb3c
d03c: mov ax, 0x3
d03f: push ax
d040: call 0x76b
d043: or ax, ax
d045: jne 0xd052
d047: mov ax, 0x4
d04a: push ax
d04b: call 0x76b
d04e: mov di, ax
d050: jmp 0xd059
d052: mov di, word ptr [bp - 0x1]
d055: and di, 0xff
d059: mov al, byte ptr [bp - 0x1]
d05c: sub ah, ah
d05e: cmp ax, di
d060: je 0xd06e
d062: mov si, word ptr [bp - 0x4]
d065: mov bx, word ptr [-0x3d6e]
d069: mov word ptr [bx + si + 0x4], 0xffff
d06e: mov cl, 0x6
d070: mov ax, di
d072: shl al, cl
d074: or al, 0x2
d076: mov si, word ptr [bp - 0x4]
d079: mov bx, word ptr [-0x3d6e]
d07d: mov byte ptr [bx + si + 0x7], al
d080: push word ptr [bp + 0x4]
d083: call 0xca55
d086: pop si
d087: pop di
d088: mov sp, bp
d08a: pop bp
d08b: ret 0x2
d08e: push bp
d08f: mov bp, sp
d091: sub sp, 0x6
d094: push si
d095: mov si, word ptr [bp + 0x4]
d098: mov ax, si
d09a: mov cx, 0x12
d09d: mul cx
d09f: mov bx, ax
d0a1: mov word ptr [bp - 0x6], bx
d0a4: add bx, word ptr [-0x3d6e]
d0a8: cmp byte ptr [bx + 0xd], 0x4
d0ac: je 0xd0ea
d0ae: cmp byte ptr [bx + 0xb], 0x1
d0b2: jne 0xd0bd
d0b4: mov byte ptr [bp - 0x1], 0x3
d0b8: mov al, byte ptr [bx + 0x3]
d0bb: jmp 0xd0c6
d0bd: mov byte ptr [bp - 0x1], 0x1
d0c1: mov al, 0x2c
d0c3: sub al, byte ptr [bx + 0x3]
d0c6: mov byte ptr [bp - 0x2], al
d0c9: mov cl, 0x6
d0cb: mov al, byte ptr [bp - 0x1]
d0ce: shl al, cl
d0d0: or al, byte ptr [bp - 0x2]
d0d3: mov byte ptr [bx + 0x6], al
d0d6: mov bx, word ptr [bp - 0x6]
d0d9: add bx, word ptr [-0x3d6e]
d0dd: mov al, byte ptr [bp - 0x1]
d0e0: cmp byte ptr [bx + 0xe], al
d0e3: je 0xd0ea
d0e5: mov word ptr [bx + 0x4], 0xffff
d0ea: pop si
d0eb: mov sp, bp
d0ed: pop bp
d0ee: ret 0x2
d0f1: add byte ptr [di - 0x75], dl
d0f4: in al, dx
d0f5: sub sp, 0x2
d0f8: push di
d0f9: push si
d0fa: sub si, si
d0fc: mov di, si
d0fe: cmp word ptr [0xba], si
d102: je 0xd123
d104: sub ax, ax
d106: push ax
d107: call 0x692
d10a: cmp word ptr [0xb4], si
d10e: je 0xd113
d110: mov di, 0xffff
d113: mov ax, 0x1
d116: push ax
d117: call 0x692
d11a: cmp word ptr [0xb4], si
d11e: je 0xd123
d120: mov di, 0xffff
d123: call 0x3aa
d126: mov byte ptr [bp - 0x1], al
d129: or al, al
d12b: je 0xd146
d12d: cmp al, 0x20
d12f: je 0xd13d
d131: cmp al, 0x10
d133: je 0xd13d
d135: cmp al, 0x2b
d137: je 0xd13d
d139: cmp al, 0xd
d13b: jne 0xd140
d13d: mov di, 0xffff
d140: mov al, byte ptr [bp - 0x1]
d143: mov byte ptr [0xc07f], al
d146: or di, di
d148: je 0xd18f
d14a: call 0x4ed
d14d: call 0x6837
d150: call 0xbb7e
d153: call 0xba72
d156: sub ax, ax
d158: push ax
d159: push ax
d15a: mov ax, 0x13f
d15d: push ax
d15e: mov ax, 0xc7
d161: push ax
d162: call 0x691b
d165: jmp 0xd16c
d167: call 0xd198
d16a: mov si, ax
d16c: or si, si
d16e: je 0xd167
d170: sub ax, ax
d172: push ax
d173: push ax
d174: mov ax, 0x13f
d177: push ax
d178: mov ax, 0xc7
d17b: push ax
d17c: call 0x6973
d17f: call 0x6887
d182: mov ax, 0x1
d185: push ax
d186: call 0x692
d189: sub ax, ax
d18b: push ax
d18c: call 0x692
d18f: call 0x1d19
d192: pop si
d193: pop di
d194: mov sp, bp
d196: pop bp
d197: ret
d198: push di
d199: push si
d19a: sub si, si
d19c: mov di, si
d19e: call 0x644
d1a1: sub ax, ax
d1a3: push ax
d1a4: push ax
d1a5: mov ax, 0x13f
d1a8: push ax
d1a9: mov ax, 0xc7
d1ac: push ax
d1ad: call 0x6973
d1b0: call 0xd92d
d1b3: call 0x1cce
d1b6: or si, si
d1b8: jne 0xd1fc
d1ba: call 0x359b
d1bd: or al, al
d1bf: jne 0xd1ea
d1c1: cmp byte ptr [-0x3f81], 0x1b
d1c6: je 0xd1f1
d1c8: mov al, byte ptr [0xc07f]
d1cb: sub ah, ah
d1cd: push ax
d1ce: call 0x9ddc
d1d1: pop bx
d1d2: mov byte ptr [0xc07f], al
d1d5: cmp al, 0x4d
d1d7: je 0xd1e5
d1d9: cmp al, 0x52
d1db: je 0xd1e5
d1dd: cmp al, 0x44
d1df: je 0xd1e5
d1e1: cmp al, 0x53
d1e3: jne 0xd1b6
d1e5: mov si, 0xffff
d1e8: jmp 0xd1b6
d1ea: mov byte ptr [-0x3f81], 0x0
d1ef: jmp 0xd1e5
d1f1: mov byte ptr [-0x3f81], 0x0
d1f6: mov ax, 0xffff
d1f9: pop si
d1fa: pop di
d1fb: ret
d1fc: call 0xd978
d1ff: or ax, ax
d201: je 0xd218
d203: dec ax
d204: jne 0xd209
d206: jmp 0xd2d2
d209: dec ax
d20a: je 0xd21e
d20c: dec ax
d20d: jne 0xd212
d20f: jmp 0xd299
d212: call 0xd33a
d215: jmp 0xd2d5
d218: call 0xd9ef
d21b: jmp 0xd2d5
d21e: push word ptr [0x74b6]
d222: mov ax, 0xac60
d225: push ax
d226: call 0x9e32
d229: pop bx
d22a: pop bx
d22b: push word ptr [0x74b8]
d22f: mov ax, 0xac60
d232: push ax
d233: call 0x9df2
d236: pop bx
d237: pop bx
d238: mov ax, 0xac60
d23b: push ax
d23c: call 0x59d4
d23f: mov di, 0x1
d242: mov si, 0x12
d245: jmp 0xd291
d247: mov bx, word ptr [-0x3d6e]
d24b: add bx, si
d24d: cmp byte ptr [bx + 0xb], 0x0
d251: jne 0xd28d
d253: cmp byte ptr [bx + 0xd], 0x2
d257: jne 0xd269
d259: mov al, byte ptr [bx + 0x6]
d25c: and al, -0x40
d25e: mov byte ptr [bx + 0x6], al
d261: mov bx, word ptr [-0x3d6e]
d265: mov byte ptr [bx + si + 0xd], 0x1
d269: mov bx, word ptr [-0x3d6e]
d26d: add bx, si
d26f: cmp byte ptr [bx + 0xd], 0x1
d273: jne 0xd28d
d275: cmp byte ptr [bx + 0xa], 0x3
d279: je 0xd285
d27b: mov byte ptr [bx + 0xd], 0x2
d27f: push di
d280: call 0xb5e3
d283: jmp 0xd28d
d285: mov al, byte ptr [bx + 0x6]
d288: and al, -0x40
d28a: mov byte ptr [bx + 0x6], al
d28d: add si, 0x12
d290: inc di
d291: cmp word ptr [-0x3d6c], di
d295: ja 0xd247
d297: jmp 0xd2d2
d299: mov ax, 0x74d4
d29c: push ax
d29d: call 0x59d4
d2a0: mov di, 0x1
d2a3: mov si, 0x12
d2a6: jmp 0xd2cc
d2a8: mov bx, word ptr [-0x3d6e]
d2ac: add bx, si
d2ae: cmp byte ptr [bx + 0xb], 0x0
d2b2: jne 0xd2c8
d2b4: cmp byte ptr [bx + 0xd], 0x4
d2b8: je 0xd2c8
d2ba: cmp byte ptr [bx + 0xd], 0x2
d2be: jne 0xd2c4
d2c0: mov byte ptr [bx + 0xd], 0x1
d2c4: push di
d2c5: call 0xd08e
d2c8: add si, 0x12
d2cb: inc di
d2cc: cmp word ptr [-0x3d6c], di
d2d0: ja 0xd2a8
d2d2: mov di, 0xffff
d2d5: call 0x35db
d2d8: mov ax, di
d2da: pop si
d2db: pop di
d2dc: ret
d2dd: push bp
d2de: mov bp, sp
d2e0: sub sp, 0x4
d2e3: push di
d2e4: push si
d2e5: mov si, 0x74bc
d2e8: jmp 0xd329
d2ea: mov al, byte ptr [si]
d2ec: cwde
d2ed: mov bx, word ptr [bp + 0x4]
d2f0: mov cx, ax
d2f2: mov al, byte ptr [bx + 0x1]
d2f5: mov dx, ax
d2f7: mov al, byte ptr [bx]
d2f9: cwde
d2fa: add ax, cx
d2fc: mov cx, 0x30
d2ff: mov bx, dx
d301: imul cx
d303: mov cx, bx
d305: mov bx, ax
d307: mov al, cl
d309: cwde
d30a: add bx, ax
d30c: mov al, byte ptr [si + 0x1]
d30f: cwde
d310: add bx, ax
d312: mov di, word ptr [-0x3d52]
d316: mov al, byte ptr [bx + di]
d318: mov byte ptr [bp - 0x3], al
d31b: or al, al
d31d: je 0xd327
d31f: cmp al, -0x1
d321: je 0xd327
d323: cmp al, -0x2
d325: jne 0xd32f
d327: inc si
d328: inc si
d329: cmp si, 0x74d4
d32d: jb 0xd2ea
d32f: mov al, byte ptr [bp - 0x3]
d332: pop si
d333: pop di
d334: mov sp, bp
d336: pop bp
d337: ret 0x2
d33a: push bp
d33b: mov bp, sp
d33d: sub sp, 0x10
d340: push di
d341: push si
d342: sub si, si
d344: mov ax, 0xb6
d347: push ax
d348: lea ax, [bp - 0x8]
d34b: push ax
d34c: call 0xc752
d34f: mov al, 0x30
d351: imul byte ptr [bp - 0x8]
d354: mov bx, ax
d356: mov al, byte ptr [bp - 0x7]
d359: cwde
d35a: add bx, ax
d35c: mov di, word ptr [-0x3d52]
d360: mov al, byte ptr [bx + di]
d362: mov byte ptr [bp - 0x2], al
d365: or al, al
d367: jne 0xd373
d369: lea ax, [bp - 0x8]
d36c: push ax
d36d: call 0xd2dd
d370: mov byte ptr [bp - 0x2], al
d373: cmp byte ptr [bp - 0x2], 0x0
d377: jne 0xd37c
d379: jmp 0xd773
d37c: cmp byte ptr [bp - 0x2], -0x1
d380: je 0xd388
d382: cmp byte ptr [bp - 0x2], -0x2
d386: jne 0xd38b
d388: jmp 0xd773
d38b: mov al, byte ptr [bp - 0x2]
d38e: sub ah, ah
d390: mov word ptr [bp - 0x10], ax
d393: mov cx, 0x12
d396: mul cx
d398: mov di, ax
d39a: mov word ptr [bp - 0xe], di
d39d: mov bx, word ptr [-0x3d6e]
d3a1: cmp byte ptr [bx + di + 0xb], 0x0
d3a5: je 0xd3c3
d3a7: push word ptr [bp - 0x10]
d3aa: call 0xdc72
d3ad: cmp word ptr [0xba], 0x0
d3b2: je 0xd3ba
d3b4: call 0x35db
d3b7: jmp 0xd70d
d3ba: call 0x358a
d3bd: mov byte ptr [0xc07f], al
d3c0: jmp 0xd70d
d3c3: sub ax, ax
d3c5: push ax
d3c6: push ax
d3c7: mov ax, 0x13f
d3ca: push ax
d3cb: mov ax, 0xc7
d3ce: push ax
d3cf: call 0x691b
d3d2: push word ptr [bp - 0x10]
d3d5: call 0xdc72
d3d8: push word ptr [bp - 0x10]
d3db: call 0xda35
d3de: push word ptr [bp - 0x10]
d3e1: call 0xd8ad
d3e4: mov di, ax
d3e6: call 0x35db
d3e9: mov bx, word ptr [bp - 0xe]
d3ec: add bx, word ptr [-0x3d6e]
d3f0: cmp byte ptr [bx + 0xd], 0x3
d3f4: je 0xd3fa
d3f6: or di, di
d3f8: jne 0xd3fd
d3fa: jmp 0xd74f
d3fd: mov byte ptr [-0x3f81], 0x0
d402: cmp word ptr [0xba], 0x0
d407: jne 0xd41b
d409: mov bx, word ptr [bp - 0xe]
d40c: add bx, word ptr [-0x3d6e]
d410: mov ax, word ptr [bx + 0x2]
d413: mov word ptr [bp - 0x8], ax
d416: mov al, byte ptr [bp - 0x2]
d419: jmp 0xd48e
d41b: or si, si
d41d: jne 0xd461
d41f: call 0x359b
d422: or al, al
d424: jne 0xd44f
d426: cmp byte ptr [-0x3f81], 0x1b
d42b: je 0xd456
d42d: mov al, byte ptr [0xc07f]
d430: sub ah, ah
d432: push ax
d433: call 0x9ddc
d436: pop bx
d437: mov byte ptr [0xc07f], al
d43a: cmp al, 0x4d
d43c: je 0xd44a
d43e: cmp al, 0x52
d440: je 0xd44a
d442: cmp al, 0x44
d444: je 0xd44a
d446: cmp al, 0x53
d448: jne 0xd41b
d44a: mov si, 0xffff
d44d: jmp 0xd41b
d44f: mov byte ptr [-0x3f81], 0x0
d454: jmp 0xd44a
d456: mov byte ptr [-0x3f81], 0x0
d45b: mov ax, 0xffff
d45e: jmp 0xd776
d461: mov ax, 0xb6
d464: push ax
d465: lea ax, [bp - 0x8]
d468: push ax
d469: call 0xc752
d46c: mov al, 0x30
d46e: imul byte ptr [bp - 0x8]
d471: mov bx, ax
d473: mov al, byte ptr [bp - 0x7]
d476: cwde
d477: add bx, ax
d479: mov si, word ptr [-0x3d52]
d47d: mov al, byte ptr [bx + si]
d47f: mov byte ptr [bp - 0x4], al
d482: cmp al, byte ptr [bp - 0x2]
d485: je 0xd491
d487: lea ax, [bp - 0x8]
d48a: push ax
d48b: call 0xd2dd
d48e: mov byte ptr [bp - 0x4], al
d491: mov cl, 0x6
d493: mov si, word ptr [bp - 0xe]
d496: mov bx, word ptr [-0x3d6e]
d49a: mov al, byte ptr [bx + si + 0x6]
d49d: mov dx, ax
d49f: sar al, cl
d4a1: and al, 0x3
d4a3: mov byte ptr [bp - 0x5], al
d4a6: and dl, 0x3f
d4a9: mov byte ptr [bp - 0x6], dl
d4ac: or dl, dl
d4ae: je 0xd4b4
d4b0: mov byte ptr [bp - 0x6], 0x1
d4b4: mov cl, 0x6
d4b6: mov al, byte ptr [bp - 0x5]
d4b9: shl al, cl
d4bb: or al, byte ptr [bp - 0x6]
d4be: mov byte ptr [bp - 0x3], al
d4c1: mov al, byte ptr [bp - 0x2]
d4c4: cmp byte ptr [bp - 0x4], al
d4c7: jne 0xd4d0
d4c9: cmp byte ptr [-0x3f81], 0x0
d4ce: je 0xd4d3
d4d0: jmp 0xd714
d4d3: mov word ptr [bp - 0xc], 0x0
d4d8: cmp word ptr [0xba], 0x0
d4dd: je 0xd4e2
d4df: jmp 0xd66a
d4e2: mov word ptr [bp - 0x6], 0x0
d4e7: cmp word ptr [bp - 0x6], 0x0
d4eb: je 0xd4f0
d4ed: jmp 0xd6e4
d4f0: mov ax, word ptr [bp - 0x8]
d4f3: mov word ptr [bp - 0xa], ax
d4f6: mov byte ptr [bp - 0x1], 0x0
d4fa: call 0xf8
d4fd: jmp 0xd513
d4ff: call 0x3aa
d502: mov byte ptr [bp - 0x4], al
d505: or al, al
d507: je 0xd513
d509: sub ah, ah
d50b: push ax
d50c: call 0x9ddc
d50f: pop bx
d510: mov byte ptr [bp - 0x1], al
d513: call 0x1ae
d516: cmp ax, 0x9
d519: jb 0xd4ff
d51b: mov al, byte ptr [bp - 0x1]
d51e: sub ah, ah
d520: cmp ax, 0x52
d523: jne 0xd528
d525: jmp 0xd64f
d528: ja 0xd551
d52a: sub al, 0xd
d52c: jne 0xd531
d52e: jmp 0xd647
d531: sub al, 0x3
d533: je 0xd539
d535: sub al, 0xb
d537: jne 0xd53c
d539: jmp 0xd647
d53c: sub al, 0x5
d53e: jne 0xd543
d540: jmp 0xd5ee
d543: sub al, 0xb
d545: jne 0xd54a
d547: jmp 0xd647
d54a: sub al, 0x22
d54c: jne 0xd551
d54e: jmp 0xd62e
d551: cmp byte ptr [bp - 0x4], 0x0
d555: jne 0xd4e7
d557: mov al, byte ptr [0xa2]
d55a: sub ah, ah
d55c: dec ax
d55d: cmp ax, 0x9
d560: ja 0xd590
d562: shl ax
d564: xchg ax, bx
d565: jmp word ptr cs:[bx - 0x2a96]
d56a: jle 0xd541
d56c: adc bp, -0x70
d56f: aad -0x78
d571: aad 0x7e
d573: aad -0x78
d575: aad -0x70
d577: aad -0x73
d579: aad -0x73
d57b: aad -0x7d
d57d: aad -0x2
d57f: inc si
d580: imul bl
d582: or ax, 0x4efe
d585: imul bl
d587: or dh, bh
d589: inc si
d58a: imul bx
d58c: add di, si
d58e: dec si
d58f: <unknown>
d593: cmp byte ptr [bp - 0xa], al
d596: jne 0xd5a3
d598: mov al, byte ptr [bp - 0x7]
d59b: cmp byte ptr [bp - 0x9], al
d59e: jne 0xd5a3
d5a0: jmp 0xd4e7
d5a3: mov word ptr [bp - 0xc], 0xffff
d5a8: sub ax, ax
d5aa: push ax
d5ab: push ax
d5ac: mov ax, 0x13f
d5af: push ax
d5b0: mov ax, 0xc7
d5b3: push ax
d5b4: call 0x6973
d5b7: push word ptr [bp - 0x10]
d5ba: mov al, byte ptr [bp - 0xa]
d5bd: push ax
d5be: mov al, byte ptr [bp - 0x9]
d5c1: push ax
d5c2: call 0xcbee
d5c5: mov si, word ptr [bp - 0xe]
d5c8: mov bx, word ptr [-0x3d6e]
d5cc: mov byte ptr [bx + si + 0x6], al
d5cf: mov bx, word ptr [-0x3d6e]
d5d3: mov byte ptr [bx + si + 0x7], -0x80
d5d7: mov bx, word ptr [-0x3d6e]
d5db: mov byte ptr [bx + si + 0x8], -0x80
d5df: push word ptr [bp - 0x10]
d5e2: call 0xda35
d5e5: mov ax, word ptr [bp - 0xa]
d5e8: mov word ptr [bp - 0x8], ax
d5eb: jmp 0xd4e7
d5ee: mov si, word ptr [bp - 0xe]
d5f1: mov bx, word ptr [-0x3d6e]
d5f5: mov ax, word ptr [bx + si + 0x2]
d5f8: mov word ptr [bp - 0xa], ax
d5fb: mov word ptr [bp - 0x8], ax
d5fe: sub ax, ax
d600: push ax
d601: push ax
d602: mov ax, 0x13f
d605: push ax
d606: mov ax, 0xc7
d609: push ax
d60a: call 0x6973
d60d: sub si, si
d60f: mov ax, 0x12
d612: mul word ptr [bp - 0x10]
d615: mov bx, ax
d617: add bx, word ptr [-0x3d6e]
d61b: mov byte ptr [bx + si + 0x6], -0x80
d61f: inc si
d620: cmp si, 0x3
d623: jb 0xd60f
d625: push word ptr [bp - 0x10]
d628: call 0xda35
d62b: jmp 0xd4e7
d62e: sub ax, ax
d630: push ax
d631: push ax
d632: mov ax, 0x13f
d635: push ax
d636: mov ax, 0xc7
d639: push ax
d63a: call 0x6973
d63d: push word ptr [bp - 0x10]
d640: mov al, byte ptr [bp - 0x3]
d643: push ax
d644: call 0xd7e8
d647: mov word ptr [bp - 0x6], 0xffff
d64c: jmp 0xd4e7
d64f: sub ax, ax
d651: push ax
d652: push ax
d653: mov ax, 0x13f
d656: push ax
d657: mov ax, 0xc7
d65a: push ax
d65b: call 0x6973
d65e: push word ptr [bp - 0x10]
d661: mov al, byte ptr [bp - 0x3]
d664: push ax
d665: call 0xd77c
d668: jmp 0xd647
d66a: cmp word ptr [0xb2], 0x0
d66f: je 0xd6e4
d671: call 0x644
d674: push word ptr [0xb6]
d678: push word ptr [0xb8]
d67c: call 0x1be6
d67f: mov ax, 0xb6
d682: push ax
d683: lea ax, [bp - 0xa]
d686: push ax
d687: call 0xc752
d68a: mov al, byte ptr [bp - 0x8]
d68d: cmp byte ptr [bp - 0xa], al
d690: jne 0xd69a
d692: mov al, byte ptr [bp - 0x7]
d695: cmp byte ptr [bp - 0x9], al
d698: je 0xd66a
d69a: mov word ptr [bp - 0xc], 0xffff
d69f: sub ax, ax
d6a1: push ax
d6a2: push ax
d6a3: mov ax, 0x13f
d6a6: push ax
d6a7: mov ax, 0xc7
d6aa: push ax
d6ab: call 0x6973
d6ae: push word ptr [bp - 0x10]
d6b1: mov al, byte ptr [bp - 0xa]
d6b4: push ax
d6b5: mov al, byte ptr [bp - 0x9]
d6b8: push ax
d6b9: call 0xcbee
d6bc: mov si, word ptr [bp - 0xe]
d6bf: mov bx, word ptr [-0x3d6e]
d6c3: mov byte ptr [bx + si + 0x6], al
d6c6: mov bx, word ptr [-0x3d6e]
d6ca: mov byte ptr [bx + si + 0x7], -0x80
d6ce: mov bx, word ptr [-0x3d6e]
d6d2: mov byte ptr [bx + si + 0x8], -0x80
d6d6: push word ptr [bp - 0x10]
d6d9: call 0xda35
d6dc: mov ax, word ptr [bp - 0xa]
d6df: mov word ptr [bp - 0x8], ax
d6e2: jmp 0xd66a
d6e4: cmp word ptr [bp - 0xc], 0x0
d6e8: je 0xd6fe
d6ea: mov si, word ptr [bp - 0xe]
d6ed: mov bx, word ptr [-0x3d6e]
d6f1: mov word ptr [bx + si + 0x4], 0xffff
d6f6: mov bx, word ptr [-0x3d6e]
d6fa: mov byte ptr [bx + si + 0xd], 0x1
d6fe: sub ax, ax
d700: push ax
d701: push ax
d702: mov ax, 0x13f
d705: push ax
d706: mov ax, 0xc7
d709: push ax
d70a: call 0x6973
d70d: mov word ptr [bp - 0x6], 0xffff
d712: jmp 0xd773
d714: sub ax, ax
d716: push ax
d717: push ax
d718: mov ax, 0x13f
d71b: push ax
d71c: mov ax, 0xc7
d71f: push ax
d720: call 0x6973
d723: mov word ptr [bp - 0x6], 0xffff
d728: call 0xd978
d72b: dec ax
d72c: dec ax
d72d: je 0xd735
d72f: dec ax
d730: je 0xd742
d732: jmp 0xd45b
d735: push word ptr [bp - 0x10]
d738: mov al, byte ptr [bp - 0x3]
d73b: push ax
d73c: call 0xd7e8
d73f: jmp 0xd45b
d742: push word ptr [bp - 0x10]
d745: mov al, byte ptr [bp - 0x3]
d748: push ax
d749: call 0xd77c
d74c: jmp 0xd45b
d74f: cmp word ptr [0xba], 0x0
d754: je 0xd75b
d756: call 0x35db
d759: jmp 0xd761
d75b: call 0x358a
d75e: mov byte ptr [0xc07f], al
d761: sub ax, ax
d763: push ax
d764: push ax
d765: mov ax, 0x13f
d768: push ax
d769: mov ax, 0xc7
d76c: push ax
d76d: call 0x6973
d770: jmp 0xd45b
d773: mov ax, word ptr [bp - 0x6]
d776: pop si
d777: pop di
d778: mov sp, bp
d77a: pop bp
d77b: ret
d77c: push bp
d77d: mov bp, sp
d77f: sub sp, 0x6
d782: push di
d783: push si
d784: mov si, word ptr [bp + 0x6]
d787: mov ax, si
d789: mov cx, 0x12
d78c: mul cx
d78e: mov di, ax
d790: mov word ptr [bp - 0x4], di
d793: mov bx, word ptr [-0x3d6e]
d797: mov al, byte ptr [bx + di + 0x6]
d79a: and al, 0x3f
d79c: mov byte ptr [bp - 0x1], al
d79f: mov ax, 0x74e8
d7a2: push ax
d7a3: call 0x59d4
d7a6: add di, word ptr [-0x3d6e]
d7aa: add di, 0xd
d7ad: cmp byte ptr [di], 0x4
d7b0: je 0xd7e0
d7b2: mov bx, di
d7b4: cmp byte ptr [bx], 0x2
d7b7: jne 0xd7bc
d7b9: mov byte ptr [bx], 0x1
d7bc: push si
d7bd: call 0xd08e
d7c0: cmp byte ptr [bp - 0x1], 0x0
d7c4: je 0xd7e0
d7c6: mov bx, word ptr [bp - 0x4]
d7c9: add bx, word ptr [-0x3d6e]
d7cd: mov al, byte ptr [bx + 0x6]
d7d0: mov byte ptr [bx + 0x7], al
d7d3: mov al, byte ptr [bp + 0x4]
d7d6: mov si, word ptr [bp - 0x4]
d7d9: mov bx, word ptr [-0x3d6e]
d7dd: mov byte ptr [bx + si + 0x6], al
d7e0: pop si
d7e1: pop di
d7e2: mov sp, bp
d7e4: pop bp
d7e5: ret 0x4
d7e8: push bp
d7e9: mov bp, sp
d7eb: sub sp, 0x6
d7ee: push di
d7ef: push si
d7f0: mov si, word ptr [bp + 0x6]
d7f3: mov ax, si
d7f5: mov cx, 0x12
d7f8: mul cx
d7fa: mov bx, ax
d7fc: mov word ptr [bp - 0x6], bx
d7ff: add bx, word ptr [-0x3d6e]
d803: cmp byte ptr [bx + 0xa], 0x3
d807: jne 0xd80c
d809: jmp 0xd8a5
d80c: mov al, byte ptr [bx + 0x6]
d80f: and al, 0x3f
d811: mov byte ptr [bp - 0x1], al
d814: push word ptr [0x74b6]
d818: mov ax, 0xac60
d81b: push ax
d81c: call 0x9e32
d81f: pop bx
d820: pop bx
d821: push word ptr [0x74ba]
d825: mov ax, 0xac60
d828: push ax
d829: call 0x9df2
d82c: pop bx
d82d: pop bx
d82e: mov ax, 0x74f2
d831: push ax
d832: mov ax, 0xac60
d835: push ax
d836: call 0x9df2
d839: pop bx
d83a: pop bx
d83b: push si
d83c: call 0x6738
d83f: push ax
d840: mov ax, 0xac60
d843: push ax
d844: call 0x9df2
d847: pop bx
d848: pop bx
d849: mov ax, 0xac60
d84c: push ax
d84d: call 0x59d4
d850: mov bx, word ptr [bp - 0x6]
d853: add bx, word ptr [-0x3d6e]
d857: cmp byte ptr [bx + 0xd], 0x2
d85b: jne 0xd870
d85d: mov al, byte ptr [bx + 0x6]
d860: and al, -0x40
d862: mov byte ptr [bx + 0x6], al
d865: mov di, word ptr [bp - 0x6]
d868: mov bx, word ptr [-0x3d6e]
d86c: mov byte ptr [bx + di + 0xd], 0x1
d870: mov bx, word ptr [bp - 0x6]
d873: add bx, word ptr [-0x3d6e]
d877: cmp byte ptr [bx + 0xd], 0x1
d87b: jne 0xd8a5
d87d: mov byte ptr [bx + 0xd], 0x2
d881: push si
d882: call 0xb5e3
d885: cmp byte ptr [bp - 0x1], 0x0
d889: je 0xd8a5
d88b: mov bx, word ptr [bp - 0x6]
d88e: add bx, word ptr [-0x3d6e]
d892: mov al, byte ptr [bx + 0x6]
d895: mov byte ptr [bx + 0x7], al
d898: mov al, byte ptr [bp + 0x4]
d89b: mov si, word ptr [bp - 0x6]
d89e: mov bx, word ptr [-0x3d6e]
d8a2: mov byte ptr [bx + si + 0x6], al
d8a5: pop si
d8a6: pop di
d8a7: mov sp, bp
d8a9: pop bp
d8aa: ret 0x4
d8ad: push bp
d8ae: mov bp, sp
d8b0: sub sp, 0x2
d8b3: push di
d8b4: push si
d8b5: mov al, 0x6
d8b7: mul byte ptr [-0x3d74]
d8bb: mov bx, ax
d8bd: mov word ptr [bp - 0x2], bx
d8c0: cmp byte ptr [bx + 0x1a20], 0x0
d8c5: jne 0xd923
d8c7: mov ax, 0x12
d8ca: mul word ptr [bp + 0x4]
d8cd: mov bx, ax
d8cf: add bx, word ptr [-0x3d6e]
d8d3: mov al, byte ptr [bx + 0x2]
d8d6: cwde
d8d7: mov cx, ax
d8d9: mov al, 0x12
d8db: mul byte ptr [-0x3d68]
d8df: mov si, ax
d8e1: add si, word ptr [-0x3d6e]
d8e5: mov al, byte ptr [bx + 0x3]
d8e8: cwde
d8e9: mov dx, ax
d8eb: mov al, byte ptr [si + 0x3]
d8ee: mov bx, ax
d8f0: mov al, byte ptr [si + 0x2]
d8f3: cwde
d8f4: mov si, ax
d8f6: sub si, cx
d8f8: mov ax, si
d8fa: mov cx, dx
d8fc: mul si
d8fe: mov di, ax
d900: mov al, bl
d902: cwde
d903: mov bx, ax
d905: sub bx, cx
d907: mov ax, bx
d909: mul bx
d90b: add di, ax
d90d: mov bx, word ptr [bp - 0x2]
d910: mov al, byte ptr [bx + 0x1a1f]
d914: sub ah, ah
d916: mov cx, ax
d918: mul cx
d91a: cmp ax, di
d91c: jb 0xd923
d91e: mov ax, 0xffff
d921: jmp 0xd925
d923: sub ax, ax
d925: pop si
d926: pop di
d927: mov sp, bp
d929: pop bp
d92a: ret 0x2
d92d: push di
d92e: push si
d92f: sub ax, ax
d931: push ax
d932: call 0x1d55
d935: mov word ptr [-0x6497], 0xf
d93b: mov di, 0x74ae
d93e: mov si, 0x748e
d941: jmp 0xd96c
d943: push word ptr [si]
d945: push word ptr [si + 0x2]
d948: push word ptr [si + 0x4]
d94b: push word ptr [si + 0x6]
d94e: push word ptr [0x6346]
d952: call 0x3134
d955: push word ptr [di]
d957: mov ax, word ptr [si]
d959: add ax, 0x5
d95c: push ax
d95d: mov ax, word ptr [si + 0x2]
d960: add ax, 0x5
d963: push ax
d964: call 0x899c
d967: inc di
d968: inc di
d969: add si, 0x8
d96c: cmp si, 0x74ae
d970: jb 0xd943
d972: call 0x1d8a
d975: pop si
d976: pop di
d977: ret
d978: push di
d979: push si
d97a: mov al, byte ptr [0xc07f]
d97d: sub ah, ah
d97f: push ax
d980: call 0x9ddc
d983: pop bx
d984: cmp ax, 0x53
d987: je 0xd9ea
d989: ja 0xd997
d98b: sub al, 0x44
d98d: je 0xd9d8
d98f: sub al, 0x9
d991: je 0xd9de
d993: sub al, 0x5
d995: je 0xd9e4
d997: cmp word ptr [0xba], 0x0
d99c: je 0xd9a5
d99e: cmp word ptr [0xb2], 0x0
d9a3: jne 0xd9ac
d9a5: cmp word ptr [0xba], 0x0
d9aa: jne 0xd9d2
d9ac: sub di, di
d9ae: mov si, 0x748e
d9b1: cmp si, 0x74ae
d9b5: jae 0xd9d2
d9b7: push word ptr [0xb6]
d9bb: push word ptr [0xb8]
d9bf: push si
d9c0: call 0x8238
d9c3: or ax, ax
d9c5: jne 0xd9cd
d9c7: add si, 0x8
d9ca: inc di
d9cb: jmp 0xd9b1
d9cd: mov ax, di
d9cf: pop si
d9d0: pop di
d9d1: ret
d9d2: mov ax, 0x4
d9d5: pop si
d9d6: pop di
d9d7: ret
d9d8: mov ax, 0x1
d9db: pop si
d9dc: pop di
d9dd: ret
d9de: mov ax, 0x2
d9e1: pop si
d9e2: pop di
d9e3: ret
d9e4: mov ax, 0x3
d9e7: pop si
d9e8: pop di
d9e9: ret
d9ea: sub ax, ax
d9ec: pop si
d9ed: pop di
d9ee: ret
d9ef: push di
d9f0: push si
d9f1: sub ax, ax
d9f3: push ax
d9f4: push ax
d9f5: mov ax, 0x13f
d9f8: push ax
d9f9: mov ax, 0xc7
d9fc: push ax
d9fd: call 0x691b
da00: mov si, 0x1
da03: mov di, 0x12
da06: jmp 0xda1a
da08: mov bx, word ptr [-0x3d6e]
da0c: cmp byte ptr [bx + di + 0xb], 0x0
da10: jne 0xda16
da12: push si
da13: call 0xda35
da16: add di, 0x12
da19: inc si
da1a: cmp word ptr [-0x3d6c], si
da1e: ja 0xda08
da20: call 0x35f9
da23: sub ax, ax
da25: push ax
da26: push ax
da27: mov ax, 0x13f
da2a: push ax
da2b: mov ax, 0xc7
da2e: push ax
da2f: call 0x6973
da32: pop si
da33: pop di
da34: ret
da35: push bp
da36: mov bp, sp
da38: sub sp, 0x22
da3b: push di
da3c: push si
da3d: mov di, word ptr [bp + 0x4]
da40: mov ax, di
da42: mov cx, 0x12
da45: mul cx
da47: mov si, ax
da49: mov word ptr [bp - 0x1c], si
da4c: mov bx, word ptr [-0x3d6e]
da50: cmp byte ptr [bx + si + 0xd], 0x4
da54: jne 0xda59
da56: jmp 0xdb82
da59: mov word ptr [-0x6506], 0x1
da5f: sub ax, ax
da61: push ax
da62: call 0x1d55
da65: mov bx, word ptr [bp - 0x1c]
da68: add bx, word ptr [-0x3d6e]
da6c: mov word ptr [bp - 0x1e], bx
da6f: mov al, byte ptr [bx + 0xa]
da72: mov byte ptr [bp - 0x8], al
da75: mov al, byte ptr [bx + 0xe]
da78: mov byte ptr [bp - 0x7], al
da7b: lea si, [bp - 0x18]
da7e: jmp 0xda8b
da80: mov ax, 0xffff
da83: mov word ptr [si], ax
da85: mov word ptr [si - 0x2], ax
da88: add si, 0x4
da8b: lea ax, [bp - 0x8]
da8e: cmp si, ax
da90: jb 0xda80
da92: mov bx, word ptr [bp - 0x1e]
da95: mov ax, word ptr [bx + 0x2]
da98: mov word ptr [bp - 0xa], ax
da9b: mov al, byte ptr [bp - 0x8]
da9e: push ax
da9f: mov al, byte ptr [bp - 0x7]
daa2: push ax
daa3: lea ax, [bp - 0xa]
daa6: push ax
daa7: lea ax, [bp - 0x1a]
daaa: push ax
daab: call 0xdbeb
daae: sub si, si
dab0: lea ax, [bp - 0x16]
dab3: mov word ptr [bp - 0x6], ax
dab6: jmp 0xdb2d
dab8: mov bx, word ptr [bp - 0x1c]
dabb: add bx, word ptr [-0x3d6e]
dabf: mov word ptr [bp - 0x1e], bx
dac2: cmp byte ptr [bx + si + 0x6], -0x80
dac6: je 0xdb1f
dac8: mov cl, 0x6
daca: mov al, byte ptr [bx + si + 0x6]
dacd: mov dx, ax
dacf: sar al, cl
dad1: and al, 0x3
dad3: mov byte ptr [bp - 0x7], al
dad6: and dl, 0x3f
dad9: mov byte ptr [bp - 0x1], dl
dadc: sub ah, ah
dade: or ax, ax
dae0: je 0xdaed
dae2: dec ax
dae3: je 0xdaf5
dae5: dec ax
dae6: je 0xdafd
dae8: dec ax
dae9: je 0xdb05
daeb: jmp 0xdb0b
daed: mov al, byte ptr [bp - 0x1]
daf0: add byte ptr [bp - 0xa], al
daf3: jmp 0xdb0b
daf5: mov al, byte ptr [bp - 0x1]
daf8: add byte ptr [bp - 0x9], al
dafb: jmp 0xdb0b
dafd: mov al, byte ptr [bp - 0x1]
db00: sub byte ptr [bp - 0xa], al
db03: jmp 0xdb0b
db05: mov al, byte ptr [bp - 0x1]
db08: sub byte ptr [bp - 0x9], al
db0b: mov al, byte ptr [bp - 0x8]
db0e: push ax
db0f: mov al, byte ptr [bp - 0x7]
db12: push ax
db13: lea ax, [bp - 0xa]
db16: push ax
db17: push word ptr [bp - 0x6]
db1a: call 0xdbeb
db1d: jmp 0xdb28
db1f: push word ptr [bp - 0x1a]
db22: push word ptr [bp - 0x18]
db25: call 0x524
db28: add word ptr [bp - 0x6], 0x4
db2c: inc si
db2d: lea ax, [bp - 0xa]
db30: cmp word ptr [bp - 0x6], ax
db33: jb 0xdab8
db35: lea si, [bp - 0x16]
db38: jmp 0xdb6c
db3a: cmp word ptr [si], -0x1
db3d: je 0xdb69
db3f: cmp word ptr [si + 0x2], -0x1
db43: je 0xdb69
db45: mov ax, word ptr [si - 0x4]
db48: mov word ptr [bp - 0x20], ax
db4b: inc ax
db4c: je 0xdb69
db4e: mov ax, word ptr [si - 0x2]
db51: mov word ptr [bp - 0x22], ax
db54: inc ax
db55: je 0xdb69
db57: push word ptr [bp - 0x20]
db5a: push word ptr [bp - 0x22]
db5d: push word ptr [si]
db5f: push word ptr [si + 0x2]
db62: mov ax, 0x524
db65: push ax
db66: call 0x97b6
db69: add si, 0x4
db6c: lea ax, [bp - 0xa]
db6f: cmp si, ax
db71: jb 0xdb3a
db73: cmp byte ptr [bp - 0x8], 0x3
db77: jne 0xdb7f
db79: push word ptr [bp + 0x4]
db7c: call 0xdb8a
db7f: call 0x1d8a
db82: pop si
db83: pop di
db84: mov sp, bp
db86: pop bp
db87: ret 0x2
db8a: push bp
db8b: mov bp, sp
db8d: mov ax, 0x12
db90: mul word ptr [bp + 0x4]
db93: mov bx, ax
db95: add bx, word ptr [-0x3d6e]
db99: mov al, byte ptr [bx + 0x2]
db9c: cwde
db9d: push ax
db9e: mov al, byte ptr [bx + 0x3]
dba1: cwde
dba2: push ax
dba3: mov al, 0x6
dba5: mul byte ptr [-0x3d74]
dba9: mov bx, ax
dbab: mov al, byte ptr [bx + 0x1a1f]
dbaf: sub ah, ah
dbb1: push ax
dbb2: mov ax, 0xdbbf
dbb5: push ax
dbb6: call 0x98bc
dbb9: mov sp, bp
dbbb: pop bp
dbbc: ret 0x2
dbbf: push bp
dbc0: mov bp, sp
dbc2: sub sp, 0x6
dbc5: mov al, byte ptr [bp + 0x6]
dbc8: mov byte ptr [bp - 0x2], al
dbcb: mov al, byte ptr [bp + 0x4]
dbce: mov byte ptr [bp - 0x1], al
dbd1: lea ax, [bp - 0x2]
dbd4: push ax
dbd5: lea ax, [bp - 0x6]
dbd8: push ax
dbd9: call 0xc7b0
dbdc: push word ptr [bp - 0x6]
dbdf: push word ptr [bp - 0x4]
dbe2: call 0x524
dbe5: mov sp, bp
dbe7: pop bp
dbe8: ret 0x4
dbeb: push bp
dbec: mov bp, sp
dbee: sub sp, 0xc
dbf1: mov bl, byte ptr [bp + 0xa]
dbf4: sub bh, bh
dbf6: mov al, byte ptr [bx + 0x6ae8]
dbfa: mov byte ptr [bp - 0x1], al
dbfd: mov al, byte ptr [bx + 0x6aec]
dc01: mov byte ptr [bp - 0x2], al
dc04: cmp byte ptr [bp + 0x8], 0x2
dc08: je 0xdc0f
dc0a: cmp byte ptr [bp + 0x8], bh
dc0d: jne 0xdc21
dc0f: mov al, byte ptr [bp - 0x2]
dc12: xor byte ptr [bp - 0x1], al
dc15: mov al, byte ptr [bp - 0x1]
dc18: xor byte ptr [bp - 0x2], al
dc1b: xor al, byte ptr [bp - 0x2]
dc1e: mov byte ptr [bp - 0x1], al
dc21: mov bx, word ptr [bp + 0x6]
dc24: mov ax, word ptr [bx]
dc26: mov word ptr [bp - 0x4], ax
dc29: lea ax, [bp - 0x4]
dc2c: push ax
dc2d: lea cx, [bp - 0x8]
dc30: push cx
dc31: call 0xc7b0
dc34: mov al, byte ptr [bp - 0x1]
dc37: add byte ptr [bp - 0x4], al
dc3a: mov al, byte ptr [bp - 0x2]
dc3d: add byte ptr [bp - 0x3], al
dc40: lea ax, [bp - 0x4]
dc43: push ax
dc44: lea ax, [bp - 0xc]
dc47: push ax
dc48: call 0xc7b0
dc4b: mov ax, word ptr [bp - 0xc]
dc4e: add ax, word ptr [bp - 0x8]
dc51: cdq
dc52: sub ax, dx
dc54: sar ax
dc56: mov bx, word ptr [bp + 0x4]
dc59: mov word ptr [bx], ax
dc5b: mov ax, word ptr [bp - 0xa]
dc5e: add ax, word ptr [bp - 0x6]
dc61: sub ax, 0x8
dc64: cdq
dc65: sub ax, dx
dc67: sar ax
dc69: mov word ptr [bx + 0x2], ax
dc6c: mov sp, bp
dc6e: pop bp
dc6f: ret 0x8
dc72: push bp
dc73: mov bp, sp
dc75: sub sp, 0x48
dc78: push di
dc79: push si
dc7a: mov si, word ptr [bp + 0x4]
dc7d: mov al, byte ptr [0x9b5c]
dc80: sub ah, ah
dc82: mov di, ax
dc84: mov al, byte ptr [0x9b5d]
dc87: add di, ax
dc89: mov ax, si
dc8b: mov cx, 0x12
dc8e: mul cx
dc90: mov bx, ax
dc92: mov word ptr [bp - 0x46], bx
dc95: add bx, word ptr [-0x3d6e]
dc99: mov word ptr [bp - 0x44], bx
dc9c: cmp word ptr [bx], 0x0
dc9f: jne 0xdca4
dca1: jmp 0xdf1c
dca4: mov ax, bx
dca6: inc ax
dca7: inc ax
dca8: push ax
dca9: lea ax, [bp - 0x10]
dcac: push ax
dcad: call 0xc7b0
dcb0: mov word ptr [bp - 0x8], 0x2
dcb5: mov word ptr [bp - 0x6], 0x18
dcba: cmp word ptr [bp - 0x10], 0x9f
dcbf: jge 0xdccb
dcc1: mov word ptr [bp - 0x8], 0xf7
dcc6: mov word ptr [bp - 0x6], 0x18
dccb: mov ax, word ptr [bp - 0x8]
dcce: add ax, 0x46
dcd1: mov word ptr [bp - 0xc], ax
dcd4: mov word ptr [bp - 0xa], 0x40
dcd9: mov bx, word ptr [bp - 0x46]
dcdc: add bx, word ptr [-0x3d6e]
dce0: mov word ptr [bp - 0x44], bx
dce3: cmp byte ptr [bx + 0xa], 0x3
dce7: jne 0xdcec
dce9: jmp 0xddea
dcec: push word ptr [bp - 0x8]
dcef: mov ax, 0x18
dcf2: push ax
dcf3: push word ptr [bp - 0xc]
dcf6: push word ptr [bp - 0xa]
dcf9: push word ptr [0x6346]
dcfd: call 0x3134
dd00: push si
dd01: call 0x6738
dd04: push ax
dd05: mov ax, 0xac60
dd08: push ax
dd09: call 0x9e32
dd0c: pop bx
dd0d: pop bx
dd0e: mov bx, word ptr [bp - 0x46]
dd11: add bx, word ptr [-0x3d6e]
dd15: cmp byte ptr [bx + 0xb], 0x1
dd19: je 0xdd21
dd1b: push word ptr [0x74ba]
dd1f: jmp 0xdd25
dd21: mov ax, 0x751a
dd24: push ax
dd25: mov ax, 0xac60
dd28: push ax
dd29: call 0x9df2
dd2c: pop bx
dd2d: pop bx
dd2e: mov word ptr [-0x6497], 0xf
dd34: mov ax, 0xac60
dd37: push ax
dd38: mov cx, word ptr [bp - 0x8]
dd3b: add cx, 0x5
dd3e: push cx
dd3f: mov dx, 0x1d
dd42: push dx
dd43: mov word ptr [bp - 0x48], cx
dd46: call 0x899c
dd49: mov ax, 0xa
dd4c: push ax
dd4d: mov cx, 0xac60
dd50: push cx
dd51: mov bx, word ptr [bp - 0x46]
dd54: add bx, word ptr [-0x3d6e]
dd58: push word ptr [bx]
dd5a: call 0x9fdc
dd5d: add sp, 0x6
dd60: mov bx, word ptr [bp - 0x46]
dd63: add bx, word ptr [-0x3d6e]
dd67: mov bl, byte ptr [bx + 0xa]
dd6a: sub bh, bh
dd6c: shl bx
dd6e: push word ptr [bx + 0x753c]
dd72: mov ax, 0xac60
dd75: push ax
dd76: call 0x9df2
dd79: pop bx
dd7a: pop bx
dd7b: mov ax, 0xac60
dd7e: push ax
dd7f: push word ptr [bp - 0x48]
dd82: lea ax, [di + 0x1d]
dd85: push ax
dd86: call 0x899c
dd89: mov bx, word ptr [bp - 0x46]
dd8c: add bx, word ptr [-0x3d6e]
dd90: mov word ptr [bp - 0x44], bx
dd93: cmp byte ptr [bx + 0xd], 0x3
dd97: jne 0xddb8
dd99: mov word ptr [-0x6497], 0x4
dd9f: mov ax, 0x7520
dda2: push ax
dda3: mov ax, word ptr [bp - 0x8]
dda6: add ax, 0x5
dda9: push ax
ddaa: mov ax, di
ddac: shl ax
ddae: add ax, 0x1d
ddb1: push ax
ddb2: call 0x899c
ddb5: jmp 0xdf1c
ddb8: mov al, byte ptr [bx + 0xc]
ddbb: mov cl, 0x19
ddbd: cwde
ddbe: idiv cl
ddc0: cwde
ddc1: mov si, ax
ddc3: shl si
ddc5: mov ax, word ptr [si + 0x7544]
ddc9: mov word ptr [0x9b69], ax
ddcc: mov al, byte ptr [bx + 0xc]
ddcf: cwde
ddd0: idiv cl
ddd2: cwde
ddd3: mov bx, ax
ddd5: shl bx
ddd7: push word ptr [bx + 0x7486]
dddb: mov ax, word ptr [bp - 0x8]
ddde: add ax, 0x5
dde1: push ax
dde2: shl di
dde4: add di, 0x1d
dde7: push di
dde8: jmp 0xddb2
ddea: cmp byte ptr [bx + 0xb], 0x1
ddee: jne 0xddfd
ddf0: push word ptr [0x7542]
ddf4: mov ax, 0xac60
ddf7: push ax
ddf8: call 0x9e32
ddfb: jmp 0xde78
ddfd: mov al, 0x6
ddff: mul byte ptr [-0x3d74]
de03: mov bx, ax
de05: push word ptr [bx + 0x1a1c]
de09: mov ax, 0xac60
de0c: push ax
de0d: call 0x9e32
de10: pop bx
de11: pop bx
de12: mov ax, 0x7529
de15: push ax
de16: mov ax, 0xac60
de19: push ax
de1a: call 0x9df2
de1d: pop bx
de1e: pop bx
de1f: mov ax, 0xa
de22: push ax
de23: lea cx, [bp - 0x42]
de26: push cx
de27: mov al, 0x6
de29: mul byte ptr [-0x3d74]
de2d: mov bx, ax
de2f: mov al, byte ptr [bx + 0x1a1e]
de33: sub ah, ah
de35: push ax
de36: call 0x9fdc
de39: add sp, 0x6
de3c: push ax
de3d: mov ax, 0xac60
de40: push ax
de41: call 0x9df2
de44: pop bx
de45: pop bx
de46: mov ax, 0x7534
de49: push ax
de4a: mov ax, 0xac60
de4d: push ax
de4e: call 0x9df2
de51: pop bx
de52: pop bx
de53: mov ax, 0xa
de56: push ax
de57: lea ax, [bp - 0x42]
de5a: push ax
de5b: mov al, 0x6
de5d: mul byte ptr [-0x3d74]
de61: mov bx, ax
de63: mov al, byte ptr [bx + 0x1a1f]
de67: sub ah, ah
de69: push ax
de6a: call 0x9fdc
de6d: add sp, 0x6
de70: push ax
de71: mov ax, 0xac60
de74: push ax
de75: call 0x9df2
de78: pop bx
de79: pop bx
de7a: mov ax, 0xac60
de7d: push ax
de7e: sub ax, ax
de80: push ax
de81: sub cx, cx
de83: push cx
de84: push ax
de85: push ax
de86: lea ax, [bp - 0x20]
de89: push ax
de8a: call 0x8c9f
de8d: lea di, [bp - 0x18]
de90: mov si, ax
de92: push ss
de93: pop es
de94: movsw word ptr es:[di], word ptr [si]
de95: movsw word ptr es:[di], word ptr [si]
de96: movsw word ptr es:[di], word ptr [si]
de97: movsw word ptr es:[di], word ptr [si]
de98: add word ptr [bp - 0x14], 0x7
de9c: mov ax, word ptr [bp - 0x14]
de9f: sub word ptr [bp - 0x18], 0x7
dea3: sub ax, word ptr [bp - 0x18]
dea6: mov word ptr [bp - 0x2], ax
dea9: add word ptr [bp - 0x12], 0x7
dead: mov ax, word ptr [bp - 0x12]
deb0: sub word ptr [bp - 0x16], 0x7
deb4: sub ax, word ptr [bp - 0x16]
deb7: mov word ptr [bp - 0x4], ax
deba: cmp word ptr [bp - 0x10], 0x9f
debf: jge 0xded1
dec1: mov word ptr [bp - 0x14], 0x13d
dec6: mov ax, 0x13d
dec9: sub ax, word ptr [bp - 0x2]
decc: mov word ptr [bp - 0x18], ax
decf: jmp 0xdede
ded1: mov word ptr [bp - 0x18], 0x2
ded6: mov ax, word ptr [bp - 0x2]
ded9: inc ax
deda: inc ax
dedb: mov word ptr [bp - 0x14], ax
dede: mov ax, word ptr [bp - 0x4]
dee1: add ax, 0x18
dee4: mov word ptr [bp - 0x12], ax
dee7: push word ptr [bp - 0x18]
deea: mov cx, 0x18
deed: mov word ptr [bp - 0x16], cx
def0: push cx
def1: push word ptr [bp - 0x14]
def4: push ax
def5: push word ptr [0x6346]
def9: call 0x3134
defc: mov word ptr [-0x6497], 0xf
df02: mov ax, 0xac60
df05: push ax
df06: mov ax, word ptr [bp - 0x14]
df09: add ax, word ptr [bp - 0x18]
df0c: cdq
df0d: sub ax, dx
df0f: sar ax
df11: push ax
df12: mov ax, word ptr [bp - 0x16]
df15: add ax, 0x7
df18: push ax
df19: call 0x8be4
df1c: pop si
df1d: pop di
df1e: mov sp, bp
df20: pop bp
df21: ret 0x2
df24: push bp
df25: mov bp, sp
df27: sub sp, 0xa
df2a: push di
df2b: push si
df2c: mov word ptr [bp - 0x2], 0x0
df31: call 0x5d20
df34: mov ax, 0x14
df37: mul word ptr [bp + 0x6]
df3a: mov bx, ax
df3c: mov word ptr [bp - 0x8], bx
df3f: mov al, byte ptr [bx + 0x1928]
df43: mov byte ptr [0xc28c], al
df46: inc word ptr [-0x3f08]
df4a: sub ax, ax
df4c: mov word ptr [bp - 0x6], ax
df4f: push ax
df50: call 0x1d55
df53: cmp word ptr [bp + 0x4], 0xff
df58: jne 0xdf92
df5a: mov cl, 0x5
df5c: mov bx, word ptr [bp + 0x8]
df5f: shl bx, cl
df61: mov ax, word ptr [bx + 0x5270]
df65: mov word ptr [0xc29e], ax
df68: mov ax, word ptr [bx + 0x5272]
df6c: mov word ptr [0xc29c], ax
df6f: mov al, byte ptr [bx + 0x527c]
df73: mov byte ptr [0xc2a2], al
df76: mov al, byte ptr [bx + 0x527b]
df7a: mov byte ptr [0xc2a3], al
df7d: mov al, byte ptr [bx + 0x527a]
df81: mov byte ptr [0xc2a4], al
df84: mov ax, word ptr [bp + 0x8]
df87: mov word ptr [0xc29a], ax
df8a: mov word ptr [-0x3d60], 0xffff
df90: jmp 0xdfaa
df92: mov ax, 0xe
df95: mul word ptr [bp + 0x4]
df98: mov bx, ax
df9a: mov ax, 0xc29a
df9d: mov di, ax
df9f: lea si, [bx - 0x3eec]
dfa3: push ds
dfa4: pop es
dfa5: mov cx, 0x6
dfa8: rep movsw word ptr es:[di], word ptr [si]
dfaa: cmp word ptr [-0x3d60], 0x0
dfaf: jne 0xdfba
dfb1: call 0x1d8a
dfb4: mov ax, 0xffff
dfb7: jmp 0xe212
dfba: sub ax, ax
dfbc: push ax
dfbd: mov cx, 0x13f
dfc0: push cx
dfc1: call 0x6e4
dfc4: sub ax, ax
dfc6: push ax
dfc7: mov ax, 0xc7
dfca: push ax
dfcb: call 0x709
dfce: mov ax, 0x9f
dfd1: push ax
dfd2: mov ax, 0x63
dfd5: push ax
dfd6: call 0x66a
dfd9: mov ax, 0x1
dfdc: push ax
dfdd: mov ax, 0x900
dfe0: push ax
dfe1: call 0x7488
dfe4: mov word ptr [0xc2ae], ax
dfe7: mov byte ptr [-0x55b4], 0x0
dfec: or ax, ax
dfee: je 0xe000
dff0: push word ptr [bp + 0x8]
dff3: push word ptr [bp + 0x6]
dff6: call 0xbe3c
dff9: or ax, ax
dffb: jne 0xe000
dffd: jmp 0xe205
e000: call 0x67e4
e003: mov bx, word ptr [bp - 0x8]
e006: cmp byte ptr [bx + 0x1927], 0x1
e00b: sbb ax, ax
e00d: neg ax
e00f: mov word ptr [0xc28e], ax
e012: cmp byte ptr [-0x3d5e], 0x0
e017: je 0xe024
e019: mov byte ptr [bx + 0x1927], -0x1
e01e: mov word ptr [-0x3d70], 0x0
e024: sub bx, bx
e026: mov word ptr [bp - 0x2], 0xc2b0
e02b: jmp 0xe03e
e02d: mov byte ptr [bx - 0x55b2], 0x0
e032: mov si, word ptr [bp - 0x2]
e035: add word ptr [bp - 0x2], 0x2
e039: mov word ptr [si], 0x0
e03d: inc bx
e03e: cmp word ptr [bp - 0x2], 0xc2b4
e043: jb 0xe02d
e045: sub ax, ax
e047: push ax
e048: call 0x692
e04b: mov ax, 0x1
e04e: push ax
e04f: call 0x692
e052: cmp byte ptr [0x3ee9], -0x1
e057: jne 0xe073
e059: mov ax, 0x2
e05c: push ax
e05d: call 0xd7
e060: mov al, byte ptr [0xc2a6]
e063: push ax
e064: call 0x5d29
e067: mov ax, 0x2
e06a: push ax
e06b: call 0x183
e06e: cmp ax, 0x1b0
e071: jb 0xe067
e073: mov al, byte ptr [0xc2a7]
e076: push ax
e077: call 0x5d29
e07a: jmp 0xe19c
e07d: call 0xba2c
e080: call 0xf8
e083: call 0x644
e086: mov word ptr [bp - 0x2], 0x1
e08b: mov di, 0x12
e08e: mov word ptr [bp - 0x4], 0x2
e093: jmp 0xe0ae
e095: mov bx, word ptr [-0x3d6e]
e099: mov ax, word ptr [bx + di]
e09b: mov bx, word ptr [-0x3d6a]
e09f: mov si, word ptr [bp - 0x4]
e0a2: mov word ptr [bx + si], ax
e0a4: add di, 0x12
e0a7: add word ptr [bp - 0x4], 0x2
e0ab: inc word ptr [bp - 0x2]
e0ae: mov ax, word ptr [bp - 0x2]
e0b1: cmp word ptr [-0x3d6c], ax
e0b5: ja 0xe095
e0b7: mov si, 0x1
e0ba: mov di, 0x12
e0bd: jmp 0xe120
e0bf: mov bx, word ptr [-0x3d6e]
e0c3: add bx, di
e0c5: cmp byte ptr [bx + 0xd], 0x4
e0c9: je 0xe11c
e0cb: cmp byte ptr [bx + 0xf], 0x0
e0cf: jne 0xe114
e0d1: cmp word ptr [bx + 0x4], 0x0
e0d5: jne 0xe0fb
e0d7: push si
e0d8: call 0xcdc7
e0db: or ax, ax
e0dd: jne 0xe0e3
e0df: push si
e0e0: call 0xb0e0 <.text>
e0e3: mov bx, word ptr [-0x3d6e]
e0e7: add bx, di
e0e9: cmp byte ptr [bx + 0xd], 0x3
e0ed: jne 0xe0ff
e0ef: cmp byte ptr [bx + 0xa], 0x1
e0f3: jne 0xe0ff
e0f5: push si
e0f6: call 0xd012
e0f9: jmp 0xe0ff
e0fb: push si
e0fc: call 0xcc7c
e0ff: mov bx, word ptr [-0x3d6e]
e103: cmp byte ptr [bx + di + 0xb], 0x1
e107: jne 0xe110
e109: push si
e10a: push word ptr [bp - 0x6]
e10d: call 0xb54c
e110: push si
e111: call 0xb848
e114: push si
e115: mov ax, 0xffff
e118: push ax
e119: call 0xb94d
e11c: add di, 0x12
e11f: inc si
e120: cmp word ptr [-0x3d6c], si
e124: ja 0xe0bf
e126: call 0xb405
e129: call 0xba4f
e12c: call 0xbb7e
e12f: call 0xba72
e132: call 0x6887
e135: mov ax, 0xc
e138: push ax
e139: call 0x11f
e13c: call 0xd0f2
e13f: sub si, si
e141: mov di, 0xc2b0
e144: jmp 0xe15d
e146: cmp word ptr [di], 0x0
e149: jne 0xe151
e14b: inc byte ptr [si - 0x55b2]
e14f: jmp 0xe156
e151: mov byte ptr [si - 0x55b2], 0x0
e156: mov word ptr [di], 0x0
e15a: inc di
e15b: inc di
e15c: inc si
e15d: cmp di, 0xc2b4
e161: jb 0xe146
e163: cmp byte ptr [-0x3d53], 0x1
e168: jne 0xe199
e16a: cmp byte ptr [-0x55b1], 0xa
e16f: jb 0xe199
e171: mov dx, 0x1
e174: mov bx, 0x12
e177: cmp word ptr [-0x3d6c], dx
e17b: jbe 0xe199
e17d: mov si, word ptr [-0x3d6e]
e181: add si, bx
e183: cmp byte ptr [si + 0xb], 0x1
e187: jne 0xe193
e189: cmp byte ptr [si + 0xd], 0x1
e18d: jne 0xe193
e18f: mov byte ptr [si + 0xd], 0x2
e193: add bx, 0x12
e196: inc dx
e197: jmp 0xe177
e199: inc word ptr [bp - 0x6]
e19c: call 0xe5b8
e19f: or ax, ax
e1a1: jne 0xe1a6
e1a3: jmp 0xe07d
e1a6: mov di, word ptr [bp + 0x8]
e1a9: call 0xbd48
e1ac: call 0x6837
e1af: push di
e1b0: push word ptr [bp + 0x6]
e1b3: call 0xe21a
e1b6: mov word ptr [bp - 0x2], ax
e1b9: cmp word ptr [-0x3d66], di
e1bd: je 0xe1d7
e1bf: mov ax, 0xe
e1c2: mul word ptr [bp + 0x4]
e1c5: mov bx, ax
e1c7: lea di, [bx - 0x3eec]
e1cb: mov si, 0xc29a
e1ce: push ds
e1cf: pop es
e1d0: mov cx, 0x6
e1d3: rep movsw word ptr es:[di], word ptr [si]
e1d5: jmp 0xe1f4
e1d7: or ax, ax
e1d9: jne 0xe1f4
e1db: mov ax, word ptr [0xc29c]
e1de: mov cl, 0x5
e1e0: shl di, cl
e1e2: mov word ptr [di + 0x5272], ax
e1e6: mov al, byte ptr [0xc2a2]
e1e9: mov byte ptr [di + 0x527c], al
e1ed: mov ax, word ptr [0xc29e]
e1f0: mov word ptr [di + 0x5270], ax
e1f4: call 0x5d20
e1f7: mov al, byte ptr [0xc2a6]
e1fa: push ax
e1fb: call 0x5e1a
e1fe: mov al, byte ptr [0xc2a7]
e201: push ax
e202: call 0x5e1a
e205: push word ptr [-0x3d52]
e209: call 0x74b7
e20c: call 0x1d8a
e20f: mov ax, word ptr [bp - 0x2]
e212: pop si
e213: pop di
e214: mov sp, bp
e216: pop bp
e217: ret 0x6
e21a: push bp
e21b: mov bp, sp
e21d: sub sp, 0x2c
e220: push di
e221: push si
e222: sub ax, ax
e224: mov word ptr [bp - 0x6], ax
e227: mov word ptr [bp - 0x8], ax
e22a: mov word ptr [bp - 0x4], ax
e22d: mov word ptr [bp - 0xa], ax
e230: mov word ptr [bp - 0x2], ax
e233: mov word ptr [bp - 0xc], ax
e236: call 0x5d20
e239: mov di, 0x1
e23c: mov si, word ptr [-0x3d6e]
e240: add si, 0x12
e243: jmp 0xe2b0
e245: cmp word ptr [si], 0x0
e248: je 0xe2ac
e24a: cmp byte ptr [si + 0xb], 0x0
e24e: jne 0xe282
e250: cmp byte ptr [si + 0xd], 0x3
e254: je 0xe267
e256: cmp byte ptr [si + 0xd], 0x4
e25a: je 0xe267
e25c: cmp byte ptr [si + 0xa], 0x3
e260: je 0xe267
e262: mov word ptr [bp - 0xc], 0xffff
e267: mov al, byte ptr [si + 0xa]
e26a: sub ah, ah
e26c: or ax, ax
e26e: je 0xe276
e270: dec ax
e271: dec ax
e272: je 0xe27d
e274: jmp 0xe282
e276: mov ax, word ptr [si]
e278: add word ptr [bp - 0x6], ax
e27b: jmp 0xe282
e27d: mov ax, word ptr [si]
e27f: add word ptr [bp - 0x8], ax
e282: cmp byte ptr [si + 0xb], 0x1
e286: jne 0xe2ac
e288: mov al, byte ptr [si + 0xa]
e28b: sub ah, ah
e28d: or ax, ax
e28f: je 0xe299
e291: dec ax
e292: je 0xe2a0
e294: dec ax
e295: je 0xe2a7
e297: jmp 0xe2ac
e299: mov ax, word ptr [si]
e29b: add word ptr [bp - 0x4], ax
e29e: jmp 0xe2ac
e2a0: mov ax, word ptr [si]
e2a2: add word ptr [bp - 0xa], ax
e2a5: jmp 0xe2ac
e2a7: mov ax, word ptr [si]
e2a9: add word ptr [bp - 0x2], ax
e2ac: add si, 0x12
e2af: inc di
e2b0: cmp word ptr [-0x3d6c], di
e2b4: ja 0xe245
e2b6: mov di, word ptr [bp + 0x4]
e2b9: mov ax, di
e2bb: mov cx, 0x14
e2be: mul cx
e2c0: mov bx, ax
e2c2: mov word ptr [bp - 0x26], bx
e2c5: mov ax, word ptr [bx + 0x191e]
e2c9: add ax, word ptr [bx + 0x1920]
e2cd: sub ax, word ptr [bp - 0x8]
e2d0: sub ax, word ptr [bp - 0x6]
e2d3: mov word ptr [bp - 0xe], ax
e2d6: mov ax, word ptr [bp - 0x8]
e2d9: add ax, word ptr [bp - 0x6]
e2dc: mov word ptr [bp - 0x12], ax
e2df: mov ax, word ptr [bp - 0x6]
e2e2: mov word ptr [bx + 0x1920], ax
e2e6: mov ax, word ptr [bp - 0x8]
e2e9: mov word ptr [bx + 0x191e], ax
e2ed: push word ptr [-0x3d6a]
e2f1: call 0x74b7
e2f4: push word ptr [-0x3d6e]
e2f8: call 0x74b7
e2fb: mov al, byte ptr [0xc2a2]
e2fe: sub ah, ah
e300: sub ax, word ptr [bp - 0x2]
e303: sub ax, word ptr [bp - 0xa]
e306: sub ax, word ptr [bp - 0x4]
e309: add ax, word ptr [-0x3d62]
e30d: add ax, word ptr [-0x3d64]
e311: mov word ptr [bp - 0x10], ax
e314: mov ax, word ptr [bp - 0x2]
e317: add ax, word ptr [bp - 0x4]
e31a: mov word ptr [bp - 0x28], ax
e31d: mov word ptr [bp - 0x14], ax
e320: mov ax, word ptr [bp - 0x4]
e323: mov word ptr [0xc29c], ax
e326: mov al, byte ptr [bp - 0xa]
e329: mov byte ptr [0xc2a2], al
e32c: mov ax, word ptr [bp - 0x2]
e32f: mov word ptr [0xc29e], ax
e332: cmp word ptr [bp - 0xc], 0x0
e336: jne 0xe33b
e338: jmp 0xe3be
e33b: mov word ptr [-0x3d60], 0x0
e341: mov bx, word ptr [bp - 0x26]
e344: cmp byte ptr [bx + 0x1923], 0x5e
e349: jbe 0xe352
e34b: mov byte ptr [bx + 0x1923], 0x63
e350: jmp 0xe357
e352: add byte ptr [bx + 0x1923], 0x5
e357: mov ax, word ptr [0xc29a]
e35a: cmp word ptr [bp + 0x6], ax
e35d: jne 0xe380
e35f: mov ax, word ptr [bp - 0x28]
e362: mov cl, 0x5
e364: mov bx, word ptr [bp + 0x6]
e367: shl bx, cl
e369: mov word ptr [bx + 0x5274], ax
e36d: cmp word ptr [bx + 0x5276], ax
e371: jae 0xe380
e373: mov bx, word ptr [bp + 0x6]
e376: shl bx, cl
e378: mov ax, word ptr [bx + 0x5276]
e37c: mov word ptr [bx + 0x5274], ax
e380: sub si, si
e382: mov cl, 0x3
e384: mov bx, word ptr [bp + 0x6]
e387: shl bx, cl
e389: mov al, byte ptr [bx + si + 0x50ec]
e38d: mov byte ptr [bp - 0x1], al
e390: cmp al, 0x1b
e392: jae 0xe3b3
e394: mov cl, 0x5
e396: mov bl, al
e398: sub bh, bh
e39a: shl bx, cl
e39c: mov al, byte ptr [bx + 0x527d]
e3a0: cwde
e3a1: push ax
e3a2: mov ax, 0x19
e3a5: push ax
e3a6: mov ax, 0x64
e3a9: push ax
e3aa: mov di, bx
e3ac: call 0x4fa
e3af: mov byte ptr [di + 0x527d], al
e3b3: inc si
e3b4: cmp si, 0x8
e3b7: jb 0xe382
e3b9: mov di, word ptr [bp + 0x4]
e3bc: jmp 0xe3ed
e3be: mov bx, word ptr [bp - 0x26]
e3c1: cmp word ptr [bx + 0x1920], 0x0
e3c6: jne 0xe3d5
e3c8: cmp word ptr [bx + 0x191e], 0x0
e3cd: jne 0xe3d5
e3cf: push di
e3d0: call 0x4816
e3d3: jmp 0xe3ed
e3d5: mov al, byte ptr [bx + 0x1923]
e3d9: sub ah, ah
e3db: push ax
e3dc: mov ax, 0x5
e3df: push ax
e3e0: sub ax, ax
e3e2: push ax
e3e3: call 0x50f
e3e6: mov bx, word ptr [bp - 0x26]
e3e9: mov byte ptr [bx + 0x1923], al
e3ed: mov bx, word ptr [bp - 0x26]
e3f0: mov al, byte ptr [bx + 0x1922]
e3f4: sub ah, ah
e3f6: push ax
e3f7: call 0x6738
e3fa: mov ax, word ptr [bp - 0x26]
e3fd: add ax, 0x1918
e400: mov word ptr [0x7ae2], ax
e403: mov bx, word ptr [-0x3d66]
e407: shl bx
e409: mov ax, word ptr [bx + 0x5222]
e40d: mov word ptr [0x7ae6], ax
e410: mov ax, word ptr [bx + 0x5f1c]
e414: mov word ptr [0x7aea], ax
e417: mov al, 0x5
e419: mul byte ptr [-0x3d57]
e41d: mov cl, byte ptr [-0x3d55]
e421: sub ch, ch
e423: add ax, cx
e425: mov cx, 0xa
e428: imul cx
e42a: mov bx, ax
e42c: mov ax, word ptr [bx + 0x76f6]
e430: mov word ptr [0x7aee], ax
e433: mov al, 0x5
e435: mul byte ptr [-0x3d56]
e439: mov dl, byte ptr [-0x3d54]
e43d: sub dh, dh
e43f: add ax, dx
e441: imul cx
e443: mov bx, ax
e445: mov ax, word ptr [bx + 0x76f6]
e449: mov word ptr [0x7af2], ax
e44c: mov al, 0x6
e44e: mul byte ptr [-0x3d74]
e452: mov bx, ax
e454: mov ax, word ptr [bx + 0x1a1c]
e458: mov word ptr [0x7af6], ax
e45b: cmp word ptr [bp - 0xc], 0x0
e45f: je 0xe473
e461: mov ax, word ptr [0x7ad8]
e464: mov word ptr [0x7adc], ax
e467: mov ax, word ptr [0x7ad6]
e46a: mov word ptr [0x7afa], ax
e46d: inc word ptr [-0x3f06]
e471: jmp 0xe47f
e473: mov ax, word ptr [0x7ada]
e476: mov word ptr [0x7adc], ax
e479: mov ax, word ptr [0x7ad4]
e47c: mov word ptr [0x7afa], ax
e47f: mov ax, word ptr [0x7ae2]
e482: mov word ptr [0x7afe], ax
e485: mov ax, word ptr [0x7ae6]
e488: mov word ptr [0x7b02], ax
e48b: push word ptr [0x7adc]
e48f: mov ax, 0xac60
e492: push ax
e493: call 0x9e32
e496: pop bx
e497: pop bx
e498: mov si, 0x7ade
e49b: jmp 0xe4aa
e49d: push word ptr [si]
e49f: mov ax, 0xac60
e4a2: push ax
e4a3: call 0x9df2
e4a6: pop bx
e4a7: pop bx
e4a8: inc si
e4a9: inc si
e4aa: cmp si, 0x7b06
e4ae: jb 0xe49d
e4b0: mov ax, 0xac60
e4b3: push ax
e4b4: push word ptr [0x7ad4]
e4b8: call 0x8c58
e4bb: push ax
e4bc: sub ax, ax
e4be: push ax
e4bf: mov cx, 0x9f
e4c2: push cx
e4c3: mov cx, 0x63
e4c6: push cx
e4c7: lea cx, [bp - 0x24]
e4ca: push cx
e4cb: call 0x8c9f
e4ce: lea di, [bp - 0x1c]
e4d1: mov si, ax
e4d3: push ss
e4d4: pop es
e4d5: movsw word ptr es:[di], word ptr [si]
e4d6: movsw word ptr es:[di], word ptr [si]
e4d7: movsw word ptr es:[di], word ptr [si]
e4d8: movsw word ptr es:[di], word ptr [si]
e4d9: sub word ptr [bp - 0x1c], 0x7
e4dd: sub word ptr [bp - 0x1a], 0x7
e4e1: add word ptr [bp - 0x18], 0x7
e4e5: add word ptr [bp - 0x16], 0x7
e4e9: mov al, byte ptr [0xc2a6]
e4ec: push ax
e4ed: call 0x5d29
e4f0: mov word ptr [-0x6497], 0x0
e4f6: push word ptr [bp - 0x1c]
e4f9: push word ptr [bp - 0x1a]
e4fc: push word ptr [bp - 0x18]
e4ff: push word ptr [bp - 0x16]
e502: push word ptr [0x6346]
e506: call 0x3134
e509: mov ax, 0xac60
e50c: push ax
e50d: mov ax, word ptr [bp - 0x18]
e510: add ax, word ptr [bp - 0x1c]
e513: cdq
e514: sub ax, dx
e516: sar ax
e518: push ax
e519: mov ax, word ptr [bp - 0x1a]
e51c: add ax, 0x7
e51f: push ax
e520: call 0x8be4
e523: mov ax, word ptr [bp - 0xe]
e526: sub dx, dx
e528: push dx
e529: push ax
e52a: mov cx, 0x78
e52d: push cx
e52e: mov bx, 0x8a
e531: push bx
e532: mov si, ax
e534: mov di, dx
e536: call 0x8ac8
e539: mov ax, word ptr [bp - 0x10]
e53c: sub dx, dx
e53e: push dx
e53f: push ax
e540: mov cx, 0xc4
e543: push cx
e544: mov bx, 0x8a
e547: push bx
e548: mov word ptr [bp - 0x2c], ax
e54b: mov word ptr [bp - 0x2a], dx
e54e: call 0x8a9e
e551: mov ax, word ptr [bp - 0x12]
e554: sub dx, dx
e556: push dx
e557: push ax
e558: mov ax, 0x78
e55b: push ax
e55c: mov ax, 0x95
e55f: push ax
e560: call 0x8ac8
e563: mov ax, word ptr [bp - 0x14]
e566: sub dx, dx
e568: push dx
e569: push ax
e56a: mov ax, 0xc4
e56d: push ax
e56e: mov ax, 0x95
e571: push ax
e572: call 0x8a9e
e575: add word ptr [-0x3f7a], si
e579: adc word ptr [-0x3f78], di
e57d: mov ax, word ptr [bp - 0x2c]
e580: mov dx, word ptr [bp - 0x2a]
e583: add word ptr [-0x3f7e], ax
e587: adc word ptr [-0x3f7c], dx
e58b: call 0x35f9
e58e: mov al, 0x6
e590: mul byte ptr [-0x3d74]
e594: mov bx, ax
e596: cmp byte ptr [bx + 0x1a20], 0x2
e59b: jne 0xe5ad
e59d: mov bx, word ptr [bp - 0x26]
e5a0: cmp byte ptr [bx + 0x1925], -0x1
e5a5: je 0xe5ad
e5a7: push word ptr [bp + 0x4]
e5aa: call 0x4733
e5ad: mov ax, word ptr [bp - 0xc]
e5b0: pop si
e5b1: pop di
e5b2: mov sp, bp
e5b4: pop bp
e5b5: ret 0x4
e5b8: push bp
e5b9: mov bp, sp
e5bb: sub sp, 0x2
e5be: push di
e5bf: sub di, di
e5c1: mov cx, di
e5c3: mov dx, 0x1
e5c6: mov bx, word ptr [-0x3d6e]
e5ca: add bx, 0x1f
e5cd: jmp 0xe5ff
e5cf: cmp word ptr [bx - 0xd], 0x0
e5d3: je 0xe5fb
e5d5: cmp byte ptr [bx - 0x3], 0x3
e5d9: je 0xe5fb
e5db: cmp byte ptr [bx], 0x4
e5de: je 0xe5fb
e5e0: cmp byte ptr [bx], 0x3
e5e3: je 0xe5fb
e5e5: mov al, byte ptr [bx - 0x2]
e5e8: mov byte ptr [bp - 0x2], al
e5eb: or al, al
e5ed: jne 0xe5f2
e5ef: mov di, 0xffff
e5f2: cmp byte ptr [bp - 0x2], 0x1
e5f6: jne 0xe5fb
e5f8: mov cx, 0xffff
e5fb: add bx, 0x12
e5fe: inc dx
e5ff: cmp word ptr [-0x3d6c], dx
e603: ja 0xe5cf
e605: or cx, cx
e607: je 0xe60d
e609: or di, di
e60b: jne 0xe61d
e60d: inc byte ptr [-0x55b4]
e611: cmp byte ptr [-0x55b4], 0x1e
e616: jbe 0xe61d
e618: mov ax, 0xffff
e61b: jmp 0xe61f
e61d: sub ax, ax
e61f: pop di
e620: mov sp, bp
e622: pop bp
e623: ret
