# Library routines called by AC_MAIN.EXE

Each routine was identified by one agent from the disassembly, then checked by a second agent that tried to
refute it (2026-11-18, workflow `advciv-identify-library-calls`). "Check" is the second agent's verdict;
where it corrected the first, the correction is the authority. D = read in the code, G = inference.

## 0x877AD stricmp (Watcom C runtime) — D, check: confirmed (D)

- **Signature:** `int stricmp(const char *s1 /*eax*/, const char *s2 /*edx*/); Watcom register convention`
- **Semantics:** Loop: a=*s1, b=*s2 (bytes); each folded to lower case only if in 'A'..'Z' (+0x20, ASCII only, no locale). If a==b and b!=0, advance both and continue; otherwise return (int)(unsigned char)a - (int)(unsigned char)b (both lower-cased). Returns 0 only when both strings end together. Preserves ebx; no globals.
- **Evidence:** 0x877AE-0x877E5: mov bl,[eax]/mov bh,[edx], cmp 0x41/0x5A, add 0x20, cmp bl,bh / test bh,bh, then movzx-style subtract dl-al.
- **Use in the game:** 0x10C94: eax=lea [ebp-0x134] (sound driver name), edx="adlib.mdi"; 0x10CA8 edx="sblaster.mdi"; 0x10D32 "sbpro1.mdi"; result tested with test eax,eax / je (equal).

## 0x877E6 free (_nfree) (Watcom C runtime) — D, check: confirmed (D)

- **Signature:** `void free(void *p /*eax*/); Watcom register convention (edx/ebx at call sites are ignored: edx is clobbered on entry)`
- **Semantics:** Walks the near-heap segment list starting at [0xCB6AC] (__nheapbeg), following link at +8, to find the heap block h with h <= p < h->next (or the last one). Calls 0x958A4(p, ds, h) = __MemFree, which returns at once for p==NULL, checks the used bit (bit0 of the size word at p-4), clears it and coalesces with following free block(s), maintaining the free list and counts. Afterwards, if h != [0xCB6B0] (__MiniHeapRover) and h->largest_free ([h+0x14]) > [0xCB6B4] (__LargestSizeB4MiniHeapRover), updates [0xCB6B4]. Clears byte [0x11B0ED] (heap-clean flag). No return value. For the port: plain free().
- **Evidence:** 0x877EC mov ecx,[0xcb6ac]; loop at 0x877F4-0x87804; call 0x958a4 (0x958AA: or eax,eax / je; sub esi,4; test al,1; coalesce); 0x8781C-0x87839. The matching malloc at 0x822D5.. uses the same globals 0xCB6AC/0xCB6B0/0xCB6B4/0x11B0ED.
- **Use in the game:** 0x10F98: eax=[0xF522C] (edx="trans_buffer" set up but unused); 0x17683: eax=[0xEED78]; 0x1DE42/0x1DE4A/0x1DE52: frees three local buffers [ebp-0x24],[ebp-0x20],[ebp-0x1C] in a row.

## 0x87844 setjmp (longjmp at 0x87879) (Watcom C runtime) — D, check: confirmed (D)

- **Signature:** `int setjmp(jmp_buf env /*eax*/); Watcom register convention. longjmp(env /*eax*/, int val /*edx*/) entry is 0x87879 (reached by jmp from game code)`
- **Semantics:** Stores into env: +0 ebx, +4 ecx, +8 edx, +0xC esi, +0x10 edi, +0x14 ebp, +0x18 return EIP, +0x1C esp (after popping return addr), +0x20 es, +0x22 ds, +0x24 cs, +0x26 fs, +0x28 gs, +0x2A ss (words); returns 0. longjmp (0x87879): calls hook [0xDB44C](saved esp, saved ss), restores ss:esp, pushes saved EIP, returns val (val 0 -> 1), restores ebx/ecx/esi/edi/ebp/es/fs/gs (segment regs verified with verr, else 0), edx and ds. For the port: C setjmp/longjmp.
- **Evidence:** 0x87844-0x87878 register stores and sub eax,eax; 0x87893 or edx,edx / jne / inc edx.
- **Use in the game:** 0x11036: setjmp(0xF58D4) during start-up; 0x115CE: setjmp(0xF58A0) after "Just loaded sprites"; longjmp via jmp 0x87879 at 0x22F36, 0x230B2, 0x3FE9B, 0x3FF82.

## 0x878D6 fprintf (Watcom C runtime) — D, check: confirmed (D)

- **Signature:** `int fprintf(FILE *fp, const char *fmt, ...); cdecl stack (variadic)`
- **Semantics:** Wrapper: eax=fp, edx=fmt, ebx=&va_list (pointer to the 3rd stack arg), calls 0x9480D = vfprintf body: clears fp->_flag bits 0x30 for the duration, allocates the buffer if fp->_base ([fp+8]) is NULL (0x95602), if the stream is unbuffered (_flag 0x400) temporarily switches it to full buffering, formats via __prtf (0x94C08) with output routine 0x947FC, flushes (0x94AC2) and restores _IONBF; returns the number of characters written, or -1 if the error flag 0x20 got set; ORs the saved 0x30 bits back into _flag.
- **Evidence:** 0x878DB lea ebx,[esp+0x18]; 0x878EC call 0x9480d; 0x9480D-0x94891 uses FILE fields +8 and +0xC/+0xD, calls 0x94c08 with ecx=0x947fc.
- **Use in the game:** Game log file FILE* in [0xF52D8]: 0x1106E fprintf(fp, "\nStart of game %d. ", [0xBC8DC]); 0x110D7 fprintf(fp, "%s ", name); 0x1467D "End of game %d.\n"; 0x146A5 "Memory available after game is loaded: %s\n".

## 0x878F7 fg_gifpal (Fastgraph) — D, check: corrected (D)

- **Signature:** `int fg_gifpal(char *filename, char *palette); cdecl stack`
- **Semantics:** DOS open (int21 3Dh, read-only); on failure returns -1. Reads 13 header bytes to scratch 0xD7694; requires first 3 bytes case-insensitively 'gif' (version not checked) else close and return -2. If global colour table flag (byte 10 bit 7): n = 2^((b&7)+1), reads 3n bytes over the scratch start (0xD7694), esi += 3n. Then reads 10 bytes (assumes an image descriptor follows directly; extension blocks are NOT skipped) at esi; if that block's byte 9 bit 7 (local table) is set, n = 2^((b&7)+1) and reads 3n bytes at esi (any read error -> -2). Closes the file, then copies 3n bytes (n of the LAST table read) from 0xD7694 to palette, each byte >> 2 (8-bit to 6-bit DAC). Note: if both global and local tables exist, the copy still comes from 0xD7694 (the global table) with the local table's length; if neither exists, count/return are uninitialised. Returns n (number of colours).
- **Evidence:** 0x87902 int21 3Dh; 0x8792A cmp ax,0xd; 0x87930 or ax,0x2020 / cmp 0x6967, cmp 0x66; 0x87946-0x8796E global table; 0x87970-0x879A3 descriptor/local table; 0x879C5 lodsw/shr ax,2/and al,0x3F/stosw loop; 0x879D1 return [ebp-4]; -1 at 0x8790D, -2 at 0x879A9.
- **Use in the game:** 0x1117C fg_gifpal("pal.pal", 0xF5D58) then fg_setdacs(0,256,0xF5D58) at 0x11190; 0x3CE76 and 0x3D018 fg_gifpal(name, 0xF8B40).
- **Check's correction:** Name, argument order, return values (-1 open fail at 0x8790D, -2 at 0x879A9, n = [ebp-4] at 0x879D1) and the global/local-table copy quirk are all right. One claim is wrong: 'any read error -> -2'. Only the 13-byte header read (0x8792A cmp ax,0xd) and the local-table read (0x879A3 jae, carry flag) catch DOS errors. The global-table read (0x8796A) and the 10-byte descriptor read (0x8797B) test only 'or eax,eax / je', which catches a 0-byte read (EOF). A DOS error sets CF and returns a non-zero error code in ax, so it gets past those two checks, and short reads are never caught. The copy at 0x879B4-0x879CF moves edi/2 words, so it copies exactly 3n bytes, which is always even because n >= 2. Each byte ends up >> 2, as stated.

## 0x879DC fg_setdacs (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_setdacs(int start, int count, char *values); cdecl stack`
- **Semantics:** Does nothing if the current video mode byte [0xD5F98] < 13 or count == 0. Otherwise, if [0xD5F9A] (vertical-retrace wait flag, set to 1 by fg_setmode) != 0, busy-waits until port 0x3DA bit 3 is set (in vertical retrace; it does not wait for the start of a new retrace). Then cli, out 0x3C8 <- low byte of start, outsb 3*count bytes from values to 0x3C9 (R,G,B 6-bit each, DAC auto-increments; start+count beyond 256 wraps in hardware), sti. Returns 0. For the port: palette[(start+i)&255] = values[3i..3i+2] (6-bit, scale to 8-bit as needed).
- **Evidence:** 0x879E1 cmp byte [0xd5f98],0xd / jl; 0x879ED jecxz; 0x879EF-0x87A05 retrace wait on 0x3DA bit 8; 0x87A08 mov dl,0xC8 / out; 0x87A0E outsb x3 / loop; 0x87A14 xor eax,eax. [0xD5F9A]=1 written by fg_setmode at 0x80D40.
- **Use in the game:** 0x11190, 0x13006, 0x130B3: fg_setdacs(0, 256, 0xF5D58) (whole 256-colour palette buffer at 0xF5D58).

## 0x87A1A fg_vbcopy (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_vbcopy(int minx, int maxx, int miny, int maxy, int newx, int newy, int source, int dest); cdecl stack`
- **Semantics:** Copies the inclusive rectangle x=minx..maxx, y=miny..maxy of virtual buffer `source` to virtual buffer `dest` with the rectangle's LOWER-LEFT corner at (newx, newy): dest top row = newy - (maxy - miny), dest left = newx. Handles are masked & 0x1F; buffer address from table 0xCB196[h], row width from 0xCB296[h]; if either address is 0 it returns 0 without copying. Byte offsets: src = addr_s + miny*width_s + minx; dst = addr_d + (newy + miny - maxy)*width_d + newx. Copies height = maxy-miny+1 rows top to bottom, each row of w = maxx-minx+1 bytes left to right (dwords, then word, then byte). NO clipping against either buffer's size or clip region, no transparency, no check on w/h (h <= 0 would loop ~2^32 times). Forward copy, so overlapping same-buffer copies moving down/right can smear. Ignores the active vb and graphics cursor. Returns 0.
- **Evidence:** 0x87A25-0x87A5D handle & 0x1F, tables 0xCB296/0xCB196 and null checks; 0x87A61-0x87A7B offset math (eax=[ebp+0x1C]+[ebp+0x10]-[ebp+0x14]); 0x87A7D-0x87A92 h/w and strides; 0x87A94-0x87AB0 row loop; 0x87AB8 xor eax,eax. Table meaning confirmed from fg_vbdefine at 0x82398 (0xCB196=addr, 0xCB296=width, 0xCB216=height).
- **Use in the game:** 0x11493/0x1150A/0x1158E: fg_vbcopy(0,639,0,479,0,479,[0xF52CC],[0xF52C8]) full-screen copy (newy=479 = bottom row); 0x18970: fg_vbcopy(460,547,30,69,460,69,[0xF52CC],[0xF52C8]) copies a region to the same place; 0x18CB3: fg_vbcopy(7,576,406,451,7,451,...).

## 0x87AC2 fg_setcolor (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_setcolor(int color); cdecl stack`
- **Semantics:** Dispatches on [0xD5F88] = mode*4 through the jump table at 0x87BB9. The game uses mode 25 (640x480x256: fg_testmode(25,1) at 0x109BC, [0xF40D0]=0x19, fg_setmode([0xF40D0]) at 0x109F3) -> table entry 25 = 0x87B99; with a virtual buffer open [0xD5F88]=0x20 (index 8) -> also 0x87B99. That path: c = color & 0xFF; [0xD5F66] = c (virtual colour), [0xD5F61] = c (current colour), dword [0xD5F62] = c replicated in all 4 bytes (c*0x01010101). Returns 0. Graphics cursor and clip untouched. (Other modes map through the 0xD615A virtual-colour table, or program the EGA set/reset registers 3CEh.)
- **Evidence:** 0x87AC9 mov ebx,[0xd5f88] / jmp cs:[ebx+0x87bb9]; table entries at 0x87BD9 (index 8) and 0x87C1D (index 25) = 0x87B99; 0x87B99-0x87BB4 stores; fg_setmode 0x80A33 writes [0xD5F98]=mode and [0xD5F88]=mode<<2 (0x80A6B/0x80A74); fg_vbopen sets [0xD5F88]=0x20.
- **Use in the game:** 0x114A2 fg_setcolor(5) after a full-screen fg_vbcopy; 0x11523 fg_setcolor(5) after fg_vbopen([0xF52C8]); 0x133E2 fg_setcolor(13). 337 call sites.

## 0x87C31 fg_vbopen (Fastgraph) — D, check: corrected (D)

- **Signature:** `int fg_vbopen(int handle); cdecl stack`
- **Semantics:** Makes virtual buffer `handle` the active drawing surface. handle > 31 (unsigned, so negative too) -> return -1; address 0xCB196[h] == 0 (not defined) -> return -2. If no vb is active ([0xD65F8] < 0), first saves the physical-screen state: clip maxx [0xD5FB0]->0xD65E0, maxy [0xD5FB8]->0xD65E4, minx [0xD5FAC]->0xD65E8, miny [0xD5FB4]->0xD65EC, screen width [0xDA2F0]->0xD65F0, height [0xDA36C]->0xD65F4, mode index [0xD5F88]->0xD65FA (switching directly from one vb to another does not re-save). Then: [0xD65F8]=h; width=0xCB296[h] -> [0xD65FE] and [0xDA2F0], width/8 -> [0xDA17C]; height=0xCB216[h] -> [0xDA36C]; clip loaded from the vb's own saved clip: minx=0xCB096[h], maxx=0xCAF96[h], miny=0xCB116[h], maxy=0xCB016[h] (fg_vbdefine initialises them to 0,width-1,0,height-1; fg_setclip 0x91260 also writes them for the active vb); [0xDA274]=height / BIOS byte 0040:0085 (char height); [0xD6606]=address, [0xD660A]=ds, [0xD6602]=width*height; [0xD5F88]=0x20 (mode index 8 = virtual-buffer drawing routines). Graphics cursor, colour and DAC unchanged. Returns 0. fg_vbclose is 0x87D73 (restores the saved state, [0xD5F88]=[0xD5F98]*4, [0xD65F8]=-1).
- **Evidence:** 0x87C3A cmp ebx,0x1f / ja -> -1 (0x87CA2); 0x87C4D null -> -2 (0x87CAC); 0x87C51-0x87C9B save; 0x87CB6-0x87D65 load; clip global meaning from fg_setmode 0x80E9A-0x80ECB (0xD5FAC=0, 0xD5FB0=width-1, 0xD5FB4=0, 0xD5FB8=height-1) and fg_setclip 0x91260 (minx,maxx,miny,maxy order).
- **Use in the game:** 0x11519, 0x115DA: fg_vbopen([0xF52C8]) (one of the two full-screen 640x480 buffers); 0x116B2: fg_vbopen([0xF52C4]). Usually paired with fg_vbclose 0x87D73.
- **Check's correction:** Everything about fg_vbopen itself checks out: the bounds (0x87C3A unsigned ja -> -1, 0x87C4D -> -2), the save and load mapping, [0xDA274] = height / es:[0x485], and [0xD5F88] = 0x20. The correction is to the side reference to fg_setclip. Its entry is 0x9125D (push ebp / mov ebp,esp); 0x91260 is the third instruction. It also does not write the vb's saved clip only when a vb is active. At 0x91284-0x9128E it does xor ebx,ebx / mov bl,[0xD65F8] / or ebx,ebx / jl. Because bl is zero-extended, the jl is never taken. With no vb active ([0xD65F8] = 0xFF), fg_setclip still writes the four clip values at index 255, at 0xCB096+0x3FC, 0xCAF96+0x3FC, 0xCB116+0x3FC and 0xCB016+0x3FC, past the 32-entry tables. That corrupts whatever follows them. Matching fg_setclip(minx,maxx,miny,maxy) -> 0xD5FAC/0xD5FB0/0xD5FB4/0xD5FB8 at 0x9126D-0x9127E confirms the clip-global meanings used here.

## 0x87D73 fg_vbclose (Fastgraph) — D, check: confirmed (D)

- **Signature:** `void fg_vbclose(void);  /* no args, cdecl; returns eax=0 */`
- **Semantics:** Undoes fg_vbopen (0x87C31). If byte [0xD65F8] (active virtual-buffer handle, signed; -1 = none) is negative, does nothing. Otherwise it restores the physical-page state saved by fg_vbopen: width [0xDA2F0] = [0xD65F0], [0xDA17C] = width>>3, height [0xDA36C] = [0xD65F4], clip region minx [0xD5FAC] = [0xD65E8], maxx [0xD5FB0] = [0xD65E0], miny [0xD5FB4] = [0xD65EC], maxy [0xD5FB8] = [0xD65E4] (clip order confirmed from fg_setclip at 0x9125D: d5fac=minx, d5fb0=maxx, d5fb4=miny, d5fb8=maxy). [0xDA274] = height / BIOS character height (byte at 0040:0085, read through selector [0xD666C]) = text rows. Recomputes the mode dispatch index [0xD5F88] = mode byte [0xD5F98]*4 (it does not use the copy fg_vbopen saved at [0xD65FA]). Sets [0xD65F8] = -1. It does not touch the graphics cursor (d5f9c/d5fa0), the colour, the bytes-per-line value [0xDA100] or the vb table. Fastgraph's vb path (index 0x20) is linear memory at [0xD6606] with width [0xD65FE]; after close, drawing goes to the 640x480 SVGA page again (index 25*4 = 0x64).
- **Evidence:** Body 0x87D73-0x87DF6. It mirrors fg_vbopen 0x87C31 (saves d5fb0->d65e0, d5fb8->d65e4, d5fac->d65e8, d5fb4->d65ec, da2f0->d65f0, da36c->d65f4 when d65f8<0, then sets d65f8=handle and d5f88=0x20). fg_setclip 0x9125D shows the clip-global order. 223 call sites, e.g. 0x1154B after drawing 'Loading game data and maps', 0x1162D after fg_vbopen([0xF52C8]) 0x115DA + fg_move(0,0) + catalog GIF 'minimap.gif'.
- **Use in the game:** Always called with no arguments after a block that began with fg_vbopen(handle) (0x87C31) and drew into the virtual buffer: 0x115DA vbopen([f52c8]) ... 0x1162D vbclose(); 0x1190F vbopen ... 'maped.gif'; 0x1154B after text drawing.

## 0x87DF7 fg_move (Fastgraph) — D, check: confirmed (D)

- **Signature:** `void fg_move(int x, int y);  /* cdecl stack; returns eax=0 */`
- **Semantics:** Sets the graphics cursor: [0xD5F9C] = x, [0xD5FA0] = y (32-bit, stored as given). No clipping, no range check, no other globals. Screen coordinates, y grows downward. fg_flicplay with flag bit 1 reads these two globals as the top-left corner, which confirms d5f9c = x and d5fa0 = y.
- **Evidence:** 0x87DFA mov eax,[ebp+8]; mov [0xd5f9c],eax; mov eax,[ebp+0xc]; mov [0xd5fa0],eax; xor eax,eax; ret. fg_flicplay 0x88033/0x8803B uses d5f9c as x0 and d5fa0 as y0.
- **Use in the game:** 0x115E6 fg_move(0,0) before drawing 'minimap.gif' from catalog 3 (0xA22FF); 0x1191B fg_move(0,0) before 'maped.gif'; 0x11B27 fg_move(0,0) before 'tokens2.gif'. Also inside fgf_print.

## 0x87E29 _dos_gettime (Watcom C runtime) — D, check: corrected (D)

- **Signature:** `unsigned _dos_gettime(struct dostime_t *t);  /* Watcom register: eax = t; returns eax = 0 */`
- **Semantics:** Calls INT 21h AH=2Ch and stores t->hour = CH (byte +0), t->minute = CL (+1), t->second = DH (+2), t->hsecond = DL (+3). Returns 0. Saves edx, ecx and ebx. (0x87E0E just before it is _dos_getdate: +0 day = DL, +1 month = DH, +2 year (word) = CX, +4 dayofweek = AL.)
- **Evidence:** 0x87E2C mov ebx,eax; mov ah,0x2c; int 0x21; mov [ebx],ch; [ebx+1],cl; [ebx+2],dh; [ebx+3],dl; sub eax,eax.
- **Use in the game:** Only one call, at 0x1178E with eax=0xF422C. The caller then builds (sec<<24)|(hour<<16)|(sec_or_min<<8)... from bytes f422c/f422e/f422f, probably as a random seed.
- **Check's correction:** The routine is correct as described (0x87E2C-0x87E3D: INT 21h AH=2Ch, [ebx]=CH hour, +1=CL minute, +2=DH second, +3=DL hundredths, returns 0). The callers_note gets the seed wrong. At 0x11793-0x117DA the caller builds [0xCAC5C] = (byte f422f = hundredths)<<24 | (byte f422c = hour)<<16 | (byte f422e = second)<<8 | (byte f422d = minute). It is not '(sec<<24)|(hour<<16)|(sec_or_min<<8)'. This runs only when [0xE1624]&4 (0x1177C). That it is a random seed is still G.

## 0x87E43 fg_mousefin (Fastgraph) — D, check: confirmed (D)

- **Signature:** `void fg_mousefin(void);  /* cdecl, no args; returns eax=0 */`
- **Semantics:** Unhooks Fastgraph's mouse event handler. It acts only if byte [0xD5FD6] (mouse initialised; set to 1 by fg_mouseini 0x814BD, 0 on failure) is non-zero AND mode byte [0xD5F98] >= 20 (the Mode X and SVGA modes, where Fastgraph installs its own handler to draw the cursor; the game's mode 25 qualifies). Then: INT 33h AX=0Ch CX=0 (event mask 0, which removes the handler); CLI; the same call is made in real mode through [0xD6682] = 0x7DEA2 (DPMI 0300h simulate real-mode INT 33h with AX=0Ch CX=0, ES = [0xD6640]); then [0xD665E] = 0x7E02D (DPMI 0304h frees the real-mode callback whose address is at 0xCAC97); STI. It does not clear [0xD5FD6] and does not hide the cursor. For the port: stop any mouse callback, nothing visible happens.
- **Evidence:** Body 0x87E43-0x87E7B. The function pointers are set at 0x7DD60 ([d6682]=0x7DEA2: int 31h AX=0300h BX=33h) and 0x7DDE4 ([d665e]=0x7E02D: int 31h AX=0304h). d5fd6 is written by 0x814BD (fg_mouseini).
- **Use in the game:** 0x118D8 at program exit (after 0x39F50(-1), before 0x80A33, probably fg_setmode/restore); 0x44D09 in the fatal-exit path, only if [0xF625A] (game's mouse-present flag), followed by 0x9174F(0).

## 0x87E7C fg_intkey (Fastgraph) — D, check: confirmed (D)

- **Signature:** `void fg_intkey(unsigned char *key, unsigned char *aux);  /* cdecl stack; returns eax=0 */`
- **Semantics:** Non-blocking keyboard read through the BIOS. INT 16h AH=01h: if no key is waiting (ZF=1), *key = 0 and *aux = 0. Otherwise INT 16h AH=00h removes the keystroke: if AL != 0 (ASCII key), *key = AL and *aux = 0; if AL == 0 (extended key), *key = 0 and *aux = AH (scan code). Writes one byte to each pointer. Uses the standard (not enhanced 10h/11h) BIOS functions, so F11/F12 are not seen.
- **Evidence:** 0x87E82 mov ah,1; int 16h; jne 0x87e8c; xor eax,eax ... 0x87E8C xor ah,ah; int 16h; or al,al; je; xor ah,ah; then mov [arg0],al; mov [arg1],ah.
- **Use in the game:** 0x12E16 / 0x12F46 / 0x1324C fg_intkey(&[0xE1634], &[0xE1635]); the caller compares byte e1634 with 0x1B (Esc) to skip the intro FLICs (sets [0xF52DC]=1). 8 call sites in total.

## 0x87EA7 fg_flichead (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_flichead(char *filename, unsigned char *header);  /* cdecl stack */`
- **Semantics:** Opens filename read-only (INT 21h 3D00h). If the open fails it returns -1. Otherwise it reads 128 bytes into header (INT 21h 3Fh, CX=0x80) and closes the file (3Eh). If the read failed (CF) it returns -2. Otherwise it checks the word at header+4: 0xAF11 (FLI) or 0xAF12 (FLC) returns 0, anything else returns -2. A short read without an error still checks whatever is in the buffer. The header is the raw FLIC header: +0 size dword, +4 magic, +6 frames word, +8 width, +10 height, +12 depth, +14 flags, +16 speed (FLI word in 1/70 s; FLC dword in ms).
- **Evidence:** Body 0x87EA7-0x87EF8: cmp ax,0xaf11 / 0xaf12 range check at 0x87ED7; returns 0 / 0xFFFFFFFF / 0xFFFFFFFE.
- **Use in the game:** 0x12E68 fg_flichead('<dir>ah_logo.flc' via sprintf '%sah_logo.flc', ebp-0x114); 0x131DB with '%sadvcivil.flc'. The game plays the intro only if it returns 0, and loops header.frames (word at ebp-0x10E = header+6) times.

## 0x87EF9 fg_flicopen (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_flicopen(char *filename, unsigned char *context /* 16 bytes */);  /* cdecl stack */`
- **Semantics:** Opens filename read-only (3D00h). If that fails it returns -1. It reads 128 header bytes into the internal buffer at 0xD7694 (the same buffer fg_flicplay later uses as its line buffer). If the read fails (CF) or the magic at +4 is neither 0xAF11 nor 0xAF12, it returns -2 and LEAVES THE FILE OPEN (handle leak). On success it fills the context with words: +0 DOS handle (file stays open, positioned at 128); +2 dword = 0x80 (data position: word 0x80, word 0); +6 frames; +8 width; +10 height (copied from header +6/+8/+10); +12 frame delay in BIOS ticks: FLI: (speed_word * 1429) / 5500, 16-bit mul/div, truncated (1/70 s units converted to 18.2 Hz ticks); FLC: (speed_dword_ms) / 55, 32/16 div, truncated (a value of 55*65536 ms or more would fault); +14 word = 0. Returns 0.
- **Evidence:** Body 0x87EF9-0x87F93: lea edx,[0xd7694]; cmp 0xaf11 -> mov dx,0x595; mul dx; mov cx,0x157c; div cx; cmp 0xaf12 -> dx:ax=[+0x10..0x12]; mov cx,0x37; div cx; stores bx, 0x80, 0, movsw x3 from header+6, stosw ax, stosw 0.
- **Use in the game:** 0x12E89 fg_flicopen(path 'ah_logo.flc', ebp-0x2C); 0x131FC with 'advcivil.flc'. Each is followed by 0x7D334(path, [0xF522C]), a game routine that loads the whole file into [0xF522C], which the game set as fg_imagebuf(f522c, 0x19F0A0) at 0x10AF3 (0x8240F = fg_imagebuf: d668a = ptr, d6690 = size; size 0 selects an internal 4096-byte buffer).

## 0x87F94 fg_flicplay (Fastgraph) — D, check: corrected (D)

- **Signature:** `int fg_flicplay(unsigned char *context, int frames, int flags);  /* cdecl stack; returns the number of frames played */`
- **Semantics:** Returns 0 at once if [0xD9F8C] (colour count of the current mode) != 256 or frames == 0. Sets context+14 = 0. Reads from context: handle (+0), position (+2 dword), width W (+8), height H (+10), delay ticks (+12). Top-left corner (x0,y0) = (0,0), or the graphics cursor ([0xD5F9C],[0xD5FA0]) if flags&2. Data source: if flags&4, memory: ptr = fg_imagebuf [0xD668A] + pos, available = [0xD6690] - pos (return 0 if that is 0), handle treated as 0 so there is no refill. Otherwise the DOS file: refill reads [0xD6690] bytes into the imagebuf whenever the buffer runs empty (0x88F79); the file is read from its current position (the context position is not used). Reading past the end yields byte 0xFF.
Frame loop (frames times): read the frame header size dword and type word. A size of 0xFFFF0xFFFF (EOF) or a type other than 0xF1FA/0xF100 ends the call. 0xF100 (prefix): skip size-6 bytes, does not count as a frame (frames++ and played--, then played++). 0xF1FA: size<16 ends the call; read the chunk count word and skip 8 reserved bytes; for each chunk read size dword and type word and dispatch types 0..16 through the table at 0x88BC0. Unknown types (and >16) are skipped. After each chunk, any unread bytes of the chunk are skipped; if the handler overran the chunk (remaining<0), the call ends and returns played-1, clamped to >= 0. After the chunks, the frame's slack (size-16-sum(chunk sizes)) is skipped.
Chunks: type 4 COLOR_256 and type 11 COLOR_64 (both skipped if flags&0x10): word packet count; if byte [0xD5F9A] (Fastgraph retrace-wait flag, default 1) != 0, waits until port 3DAh bit 3 is set (start of vertical retrace; it does not first wait for it to clear); index starts at 0; per packet: index += skip byte; count byte (0 means 256); OUT 3C8h,index; count RGB triples to 3C9h, each byte >>2 for type 4 and unchanged for type 11; index += count (8-bit wrap). Writes the DAC directly; no palette shadow is updated. Type 13 BLACK: H rows of W zero bytes at (x0, y0..). Type 15 BRUN: y=y0; per row the packet-count byte is read and ignored; then until W bytes are done: signed byte n; n>0 (or 0) replicates the next byte n times, n<0 copies -n literal bytes; the row is then written as W bytes at x0. Type 16 COPY: H rows of W literal bytes. Type 12 LC (DELTA_FLI): y = y0 + word skip; word line count; per line: unsigned packet-count byte; x = x0; per packet: x += unsigned skip byte; signed n: n>=0 copies n literal bytes, n<0 replicates the next byte -n times; each packet is written at its x (count bytes) and x advances by count; then y++. Type 7 SS2 (DELTA_FLC): y = y0; word line count; per line read words until a packet count: if bit15 and bit14 set, y += -word (lines skipped, row recomputed); if bit15 set and bit14 clear, the low byte is written at (x0+W-1, y) and reading continues; otherwise it is the packet count (0 goes to the next line); per packet: x += unsigned skip byte; signed n: n>=0 copies n literal 16-bit pairs (2n bytes), n<0 replicates the next 2-byte pair -n times; written at x, x += 2*|n|; then y++.
Pixel output (row writer chosen by mode index [0xD5F88] through 0x88E71; the game uses mode 25 = 640x480x256 SVGA, index 0x64 -> 0x88E1F, or the vb path index 0x20 -> 0x88E3B if a virtual buffer is open): row offset = y*[0xDA100] (bytes per line), with the SVGA bank = high word + [0xD65DA] switched through [0xD65C6], and bank crossings inside a row handled. Clipping: the ONLY test is that a row is skipped when its y >= [0xDA36C] (screen or vb height, signed compare). There is no x clipping (runs past W or the screen edge wrap into the next scanline), no test for negative y, and the fg_setclip region is ignored. Quirk: BLACK does not set the y it tests (ebp-0x72), so its row clip uses a stale y. The graphics cursor and the colour are not changed.
Timing: after each frame (including a prefix frame or an empty frame), unless flags&1 or delay==0, it busy-waits on the BIOS tick count (INT 1Ah AH=0, low word, wrap handled) until delay ticks have passed since the END of that frame's decode. End: in memory mode context+2 = imagebufsize - bytes left (new position) and context+14 = 1; in file mode it LSEEKs the handle back (INT 21h 4201h) by the bytes left in the buffer. It resets the Mode X map mask only in Mode X (0x88EE9). Returns the number of 0xF1FA frames played.
- **Evidence:** Entry 0x87F94/0x87F9D (cmp [0xd9f8c],0x100; frames==0 -> 0); flag tests: 0x8802B (&2 -> d5f9c/d5fa0), 0x88062 and 0x88B7A (&4), 0x883C6 and 0x886CB (&0x10), 0x88B2E (&1); frame magic 0xF100/0xF1FA at 0x88141; chunk jump table 0x88BC0 (4->0x883C6, 7->0x884DD, 11->0x886CB, 12->0x887D5, 13->0x88964, 15->0x8899D, 16->0x88A85, others->0x88349 skip); mode setup 0x88DDA with table 0x88E71 (0x20->0x88E3B vb, 0x4C->0x88DE7 mode 19, 0x50-0x5C->0x88E03 Mode X, 0x60-0x6C->0x88E1F SVGA); SVGA row start 0x88C18, row write 0x88CF1 (cmp [ebp-0x72],[0xda36c]; jge skip), next row 0x88C53; delay loop 0x88B3C-0x88B68 (int 1Ah); retrace wait 0x88413 (in 3DAh test 8); refill 0x88F79. The mode is chosen at 0x109BC: 0x8063C(25,1) then [0xF40D0]=25, with the 640x480 requirement message.
- **Use in the game:** 0x12F34 and 0x1323A: fg_flicplay(&ctx /*ebp-0x2C*/, 1, 4), one frame at a time from the imagebuf (whole file loaded at [0xF522C] = fg_imagebuf), at (0,0), with the header delay; after each frame fg_intkey and Esc abort; the loop runs header.frames times, so the FLC ring frame is never played. Files 'ah_logo.flc' and 'advcivil.flc'.
- **Check's correction:** Most of the description is right: the chunk table at 0x88BC0, the BRUN/COPY/BLACK/LC/SS2/COLOR_256/COLOR_64 decoding, the row writer chosen through 0x88E71 (index 0x64 gives 0x88E1F), the only clip being the signed test y >= [0xDA36C] (0x88CFA, 0x88DB4), no x clipping, the 16-bit-mul bank row setup (0x88C18) and the timing loop (0x88B3C). These things need correcting:
(1) The end-of-call update runs ONLY when every requested frame was played (0x88B6D dec frames, je 0x88B77). That update is context+2 = imagebufsize - bytes left and context+14 = 1 in memory mode (0x88B9B-0x88BA6), or the LSEEK back in file mode (0x88B86). Every early exit jumps straight to 0x88BAC and skips it: no available data in memory mode (0x8807F), the -1 word (0x880C3), an unknown frame type (0x8814D), frame size < 16 (0x881D0), a chunk overrun (0x88B13-0x88B21). In those cases context+2 is NOT advanced, context+14 stays 0 (cleared at 0x87FE5), and in file mode nothing is seeked.
(2) The early return at entry (0x87FB2/0x87FB8 to 0x87F96) happens before context+14 is cleared, so context+14 is left unchanged in that case.
(3) In memory mode 'available' is [0xD6690] - pos, the imagebuf SIZE (0x19F0A0, set at 0x10AE7), not the file size. Reading past the end of the FLIC therefore returns stale imagebuf bytes. 0xFF only comes back after the whole imagebuf is used up (the refill 0x88F79 returns 0 when handle==0).
(4) The end-of-data test is eax == -1 after the low size WORD is assembled (0x880C0). It holds only when the second byte read hit buffer exhaustion and the first byte was 0xFF. It is not a test of the whole dword against 0xFFFFFFFF; otherwise the bad-type check at 0x88141 catches end of data.
(5) The palette packet count is not checked for 0 (0x884CE / 0x887C6 dec then je). A count of 0 loops about 2^32 packets. In the same way BRUN, COPY and BLACK with H=0 loop about 2^32 rows (dec edx; jne).
(6) The frame slack skip at 0x88B26 does add esi / sub [ebp-0x1a] with no refill check. In file mode, slack larger than the buffered bytes corrupts the read state.
(7) BLACK also never increments ebp-0x72, so all H rows are tested against the same stale y: either every row is drawn or none is.
Grade note: the global names d9f8c = colour count, d5f9a = retrace flag and da100 = bytes per line rest on how they are used (G). The behaviour itself is D.

## 0x88F98 fg_showgif (Fastgraph/Image) — D, check: confirmed (D)

- **Signature:** `int fg_showgif(char *filename, int flags); cdecl stack`
- **Semantics:** 0x88F98 is just `jmp 0x88FB2`. The bytes after it, mis-labelled fn_00088F9A/FA3/FA8, are data: a bit-mask table and the interlace step table. Returns 0 straight away, drawing nothing, if [0xDA100]==0 (video bytes per line) or the mode byte [0xD5F98]==12.

Flags (all D):
- bit 2 (4): the GIF comes from the fg_imagebuf buffer (far ptr [0xD668A], size [0xD6690]) instead of a file.
- bit 1 (2): the image's top-left corner goes at the graphics cursor ([0xD5F9C],[0xD5FA0]); otherwise at the left/top in the image descriptor.
- bit 0 (1): keep the current palette. When it is 0, the colour table (local if present, else global, each byte >>2) is loaded into DAC entries 0..n-1. For mode 25 the DAC is written even while a virtual buffer is open.

Decoding: checks that the file starts with 'GIF' (case-insensitive) and skips 7 bytes. Skips '!' extension blocks; ';' or EOF ends; any other byte returns 2. Only the first ',' image is decoded (LZW, max 12 bits). Interlace bit 0x40 is honoured (row step table).

Clipping: only against the right and bottom of the current surface. Drawn width = min(x+w-1, [0xDA2F0]-1) - x + 1. At most min(y+h-1, [0xDA36C]-1) - y + 1 rows are drawn, top row first, in decode order. No clip-region test and no check for negative x/y. Rows are copied opaque (colour 0 is written too).

Target: dispatches on [0xD5F88] (= mode*4, or 0x20 while fg_vbopen is active). Mode 25 is a banked SVGA copy with stride [0xDA100]. In a virtual buffer the row goes to [0xD6606] + row*[0xD65FE] + x.

Does not move the graphics cursor. Returns 0 OK, 1 if open (int 21h/3Dh) fails, 2 if not a GIF.
- **Evidence:** 0x88FC2-0x88FD1 early exits. 0x88FE8 test flags&4 skips the open (int 21h AH=3Dh, else return 1 at 0x88FFD). 0x890BA-0x890FA 'g','i','f' compare, return 2 at 0x891AA. 0x8927D test flags&2 picks [0xD5F9C]/[0xD5FA0] (=[ebp-0xA2]/[ebp-0xA6]). 0x892DB-0x892EE and 0x8932A-0x8933F clip to [0xDA2F0]-1 and [0xDA36C]-1. 0x89BB2 `test flags,1; ret` skips the palette; mode-25 entry 0x89C4C writes DAC 0x3C8/0x3C9 from index 0. 0x89A63 does >>2. 0x89FDD tests interlace 0x40. Row writers: 0x89593 (SVGA, via [0xD65C6] bank switch) and 0x8962D (VB). Mode table 0x89717: [0x64] gives 0x896E1, [0x20] gives 0x8970E.
- **Use in the game:** Title slides: fg_move(0,0) inside a virtual buffer, then fg_showgif("<dir>P2",2) at 0x12FDC (also P3 0x13089, P4 0x13136), then fg_vbclose.

Screen save/restore: fg_move(left,top), then fg_showgif("junk",3) at 0x19BF6, 0x1A105, 0x3FCB1, 0x406BA ("junk2" at 0x1A386). These pair with fg_makegif(...,"junk").

Catalog images: fg_showgif(name,7) (from fg_imagebuf, keep palette, at cursor) at 0x2F23E, 0x2F334, 0x2F662.
- **Check's correction:** No errors found. Rechecked in the code: 0x88F98 jmp 0x88FB2. Early exits at 0x88FC2/0x88FCA return eax=0 through 0x88FAD->0x893E7. Flag 4 skips int 21h/3Dh (0x88FE8). Open failure returns 1 at 0x88FFD. 'g','i','f' are compared with or al,0x20 (0x890BA-0x890FA), then 7 bytes are skipped (0x89101: '87a' plus the screen width and height). Any byte other than ',', '!' or ';' closes the file and returns 2 (0x8919E-0x891AA). EOF or ';' goes to 0x893D4 and returns 0. Flag 2 uses [ebp-0xA2]/[ebp-0xA6] (0x8927D). Width clip at 0x892DB-0x892EE and row clip at 0x8932A-0x8933F; the min uses an unsigned borrow (sbb). Rows are drawn while [ebp-0x44]!=0 in decode order (0x89FBA-0x89FD1). The interlace step table is 0x88FA3 {8,8,4,2}; start rows {4,2,1} are at 0x88FA9. The palette gate 0x89BB2 skips when bit 0 is set. Palette read and write tables (0x89ABA, 0x89C6C) send index 0x20 through the saved physical mode [ebp-0x96]=[0xD65FA], so the DAC is written in a VB too. Mode 25: 0x89C4C writes from index 0, count [ebp-0x64]. Row writers: [0x64]->0x896E1->0x89593 and [0x20]->0x8970E->0x8962D, both opaque rep movs. Two minor additions. (a) In file mode the fg_imagebuf buffer ([0xD668A], size [0xD6690]) is also the file read buffer (0x89CE4 reads into [ebp-6]). (b) If y >= surface height, the row count goes non-positive but only ==0 is tested, so rows would be written off-surface. The caller at 0x19BF6 is fg_move(x,y) then fg_showgif("junk",3), as stated.

## 0x8A080 clock (Watcom C runtime) — D, check: confirmed (D)

- **Signature:** `clock_t clock(void); no arguments, result in eax`
- **Semantics:** `jmp 0x9CF6A`. That routine calls 0x9CF04, which reads the DOS time (int 21h AH=2Ch, through 0xA1EBC with a REGS block) and returns hundredths of a second since midnight: ((h*60+m)*60+s)*100+cs. It subtracts the start-up base [0x11B384], stored by 0x9CF5F. 0x9CF5F is an init routine referenced only from the data object (fixup at 0xE15F6, so an XI/initialiser table). If the current time is below the base, 8,640,000 (0x83D600, one day) is added first. Result: hundredths of a second since program start (CLOCKS_PER_SEC = 100), wrapping once at midnight. The resolution is the DOS clock: about 55 ms steps, reported in 1/100 s. Saves ebx and edx.
- **Evidence:** 0x8A080 jmp 0x9CF6A. 0x9CF6C call 0x9CF04, cmp with [0x11B384], add 0x83D600, sub base. 0x9CF04 builds the value from [esp+9]=CH (hours), [esp+8]=CL (minutes), [esp+0xD]=DH (seconds), [esp+0xC]=DL (hundredths). 0x9CF5F stores the base. fixups.txt: 000E15F6 off32 0009CF5F.
- **Use in the game:** Delay loops in the intro (0x132A2, 0x132CC): t0=clock(); loop until clock() >= t0 + (n/[0xBB3F4])/10, polling fg_kbtest-like 0x87E7C. [0xBB3F4] is set to 10 at 0x10494. Also 0x133EA, 0x135D1 (39 call sites in total).
- **Check's correction:** Behaviour confirmed: jmp 0x9CF6A. It calls 0x9CF04, which reads the time with AH=2Ch through 0xA1EBC using REGS bytes CH=[+9], CL=[+8], DH=[+0xD], DL=[+0xC] and computes ((h*60+m)*60+s)*100+cs. The base [0x11B384] is stored by 0x9CF5F. The data at 0xE15F4 is an XI-style entry '00 20 5F CF 09 00' (fixup 0xE15F6). If below the base (unsigned jae), 0x83D600 is added. Saves ebx and edx. There are 39 call sites. Caller 0x132A2/0x132CC: loops while t0 + (n/[0xBB3F4])/10 > clock(), polling 0x87E7C, and Esc aborts. The name 'clock' is not printed anywhere, but the body is the Watcom clock() implementation exactly.

## 0x8A085 fg_vbpaste (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_vbpaste(int minx, int maxx, int miny, int maxy, int x, int y); cdecl stack`
- **Semantics:** Copies the rectangle [minx..maxx] x [miny..maxy] (inclusive) of the active virtual buffer to video memory. Source: base [0xD6606], width [0xD65FE], selector [0xD660A]. The destination's lower-left corner is (x, y): destination top row = y - (maxy - miny), left column = x.

Copies width = maxx-minx+1 bytes per row and height = maxy-miny+1 rows, top to bottom, opaque (no transparent colour).

No clipping at all: neither the clip region nor the screen edges. The caller must pass ordered, in-range values.

Dispatches on [0xD65FA], the physical mode saved by fg_vbopen, not on [0xD5F88]. So it always writes the screen (selector [0xD6014], banked, stride [0xDA100], page bank [0xD65DA]), even while a virtual buffer is open. It works after fg_vbclose too, because the vb descriptor globals stay set.

Mode 25 path: 0x8A5C0. Does not touch the graphics cursor. Returns 0.
- **Evidence:** 0x8A090-0x8A0A0: esi = miny*[0xD65FE] + minx + [0xD6606]. 0x8A0A6 width = maxx+1-minx; source skip = [0xD65FE]-width. 0x8A0B5 rows = maxy+1-miny. ebx = x. eax = y + miny - maxy. 0x8A0D5 `jmp [edi+0x8A765]` with edi = [0xD65FA]; table[0x64] = 0x8A5C0 (rep movsd/movsw/movsb per row, bank switch via [0xD65C6] at a 64 KB crossing). 0x8A747 returns eax=0.
- **Use in the game:** Full-screen paste: fg_vbopen(h); fg_vbpaste(0,639,0,479,0,479); fg_vbclose at 0x133B4.

Partial restores after fg_vbclose:
- fg_vbpaste(585,634,415,444,585,444) at 0x18DA0
- fg_vbpaste(7,576,406,451,7,451) at 0x18DC0

These are same-place copies: y = maxy, x = minx.
- **Check's correction:** No errors found. Rechecked: esi = miny*[0xD65FE] + minx + [0xD6606]. width = maxx+1-minx; rows = maxy+1-miny. Destination top = y+miny-maxy; destination left = x. The dispatch is on [0xD65FA], which fg_vbopen saves from [0xD5F88] only when no VB is open (0x87C51-0x87C9B). Table 0x8A765: [0x64]=0x8A5C0, [0x20]=0x8A747 (no-op, returns 0). 0x8A5C0 writes es=[0xD6014] with stride [0xDA100], bank = high word + [0xD65DA] via [0xD65C6], and bank++ on a 16-bit carry. There is no clipping anywhere. fg_vbclose (0x87D73) does not clear [0xD6606]/[0xD65FE]/[0xD660A]. Call site 0x18DA0 pushes (585,634,415,444,585,444) after fg_vbclose at 0x18D78, as stated.

## 0x8A7DD fgf_width (string width in the current font) (Fastgraph/Fonts) — D, check: corrected (G)

- **Signature:** `int fgf_width(char *string, int n); cdecl stack`
- **Semantics:** Returns 0 if n <= 0 or no font is selected ([0xE0A74]==0). Otherwise walks n bytes.

Per byte:
- Bytes with bit 7 set, and bytes < 0x20, add nothing.
- Byte 0x7F is an escape: it and the following byte are skipped. Both count toward n, and the loop stops if n runs out on the 0x7F.
- Any other byte c adds width[c-0x20] + [0xE0A70]. width[] is the selected font's u8 width table at far ptr [0xE0A66], set by fgf_select. [0xE0A70] is the per-character spacing.

So the result counts the spacing after every counted glyph, the last one included. Same loop shape as the string-height routine 0x8E1EB, which takes the maximum of rows[] at +0x60 instead of a sum. Pure function: draws nothing, moves no cursor.
- **Evidence:** 0x8A7E7-0x8A7F4 guards. 0x8A7F6 edx=[0xE0A70]. 0x8A7FC les ebx,[0xE0A66]. 0x8A809-0x8A81F: lodsb; js skip; cmp 0x7F; sub 0x20; js skip; add width[c]; add spacing. 0x8A825 escape handling. fgf_select (0x8E188) sets [0xE0A74], [0xE0A66] and [0xE0A6A].
- **Use in the game:** Centring text: w = fgf_width(s, strlen(s)) at 0x134B6; fg_move((480-w)/2+78, row*30+111); then fg_draw(that x + w + 2, same y) at 0x13519 (an underline). Also at 0x144C5, 0x1637A, 0x18575.
- **Check's correction:** The behaviour is correct as described: guards at 0x8A7E7/0x8A7EE; per byte, js skips bit 7 and bytes <0x20; 0x7F is handled at 0x8A825 (dec ecx; je end; inc esi; loop); width byte at es:[[0xE0A66] + c-0x20] plus [0xE0A70] per glyph. [0xE0A66]/[0xE0A6A] are set by fgf_select at 0x8E1D7/0x8E1E0, [0xE0A74] at 0x8E1B6, and the spacing [0xE0A70] at 0x8E24A. Call site 0x134B6 is fgf_width(s, strlen(s)) via repne scasb. The correction is to the grade: the name 'fgf_width' is an inference (the author says so in open question 3), so the entry should be graded G for the name and D for the behaviour, not plain D.

## 0x8A833 fg_draw (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_draw(int x, int y); cdecl stack`
- **Semantics:** Draws a solid line in colour byte [0xD5F62] (fg_setcolor stores the colour replicated in 4 bytes there; low byte used) from the graphics cursor P = ([0xD5F9C],[0xD5FA0]) to Q = (x, y). It is clipped to the clip region minx=[0xD5FAC], maxx=[0xD5FB0], miny=[0xD5FB4], maxy=[0xD5FB8].

At the end the cursor is always set to the original (x, y), even when the line is rejected. Returns 0.

Clipping (Cohen-Sutherland). Outcode bits are taken from the sign of the subtraction:
- 1: x < minx
- 2: y < miny
- 4: x > maxx
- 8: y > maxy

Loop:
1. If both codes are 0, draw.
2. If the two codes share a bit, reject.
3. If code(P) == 0, swap P and Q.
4. Fix P on the first set bit, in the order 1, 2, 4, 8, then repeat from 1:
   - bit 1: P.y += (Q.y-P.y)*(minx-P.x)/(Q.x-P.x); P.x = minx
   - bit 2: P.x += (Q.x-P.x)*(miny-P.y)/(Q.y-P.y); P.y = miny
   - bit 4: as bit 1 with maxx
   - bit 8: as bit 2 with maxy

The arithmetic is 16-bit signed (imul dx then idiv cx), so the quotient truncates toward 0.

Rasterising: if Q.y < P.y, swap so the start S is the endpoint with the smaller y (P when the ys are equal). Let dx=|Q.x-S.x|, dy=|Q.y-S.y| and sx=sign(Q.x-S.x).
- X-major (dx >= dy): d = 2dy-dx. Repeat dx times: plot(cur); if d >= 0 { y++; d += 2dy-2dx } else d += 2dy; x += sx. Then plot(cur). That is dx+1 pixels; dx==0 gives one pixel.
- Y-major (dy > dx): plot(S); d = 2dx-dy. Repeat dy times: if d >= 0 { x += sx; d += 2dx-2dy } else d += 2dx; y++; plot(cur). That is dy+1 pixels.

Pixels are stored opaque (no XOR or other raster op on these paths). Mode 25 writes banked video memory (stride [0xDA100]). The virtual-buffer path (0x8BDD9) uses the same algorithm into [0xD6606] with stride [0xD65FE].
- **Evidence:** 0x8A83A pushes the original x, y. 0x8A848-0x8A8AA build the outcodes in bl/bh with cmp/lahf/shl/rcl. 0x8A8AC-0x8A9CB is the clip loop. 0x8A9D0-0x8A9F5 orders by y. 0x8A9FD dispatches through the 0x8BEDB table: [0x64] = 0x8B92A (SVGA), [0x20] = 0x8BDD9 (VB). Bresenham at 0x8B96A-0x8B9CF and 0x8B9D5-0x8BA44 (x decreasing: 0x8BA49-0x8BB2D). Epilogue 0x8BEC7 pops [0xD5FA0] and [0xD5F9C], eax=0. fg_setcolor 0x87B99/0x87BA3 sets [0xD5F61]=[0xD5F66]=c and [0xD5F62]=c*0x01010101.
- **Use in the game:** Underlines after fgf_width + fg_move (0x13519, 0x14532). Also 0x18583, 0x1A285 (22 call sites).
- **Check's correction:** No errors found. Outcodes: cmp then lahf; shl ah moves SF into CF, and rcl builds bit3=y>maxy, bit2=x>maxx, bit1=y<miny, bit0=x<minx. bl is the code of Q (the arguments) and bh is the code of P (the cursor). Trivial accept at 0x8A8AC; reject with test bl,bh at 0x8A8B0. Swap if bh==0 (0x8A8C0). Fixes are tried in the order shr bh bit0..bit3, using 16-bit imul dx / idiv cx / cwde. Y ordering at 0x8A9D0 (swap only when Q.y<P.y). Table 0x8BEDB: [0x64]=0x8B92A, [0x20]=0x8BDD9. The VB Bresenham at 0x8BE0C-0x8BEC5 matches the described d updates, plotting before each step and once at the end, so dx+1 or dy+1 pixels. Epilogue 0x8BEC7 pops y into [0xD5FA0] and x into [0xD5F9C] and returns eax=0. Colour is [0xD5F62] low byte, stored opaque with stosb. Caller 0x13519 is fg_draw(x+w+2, y). There are 22 call sites.

## 0x8C22C fg_mousepos (Fastgraph) — D, check: confirmed (D)

- **Signature:** `void/int fg_mousepos(int *x, int *y, int *buttons); cdecl stack`
- **Semantics:** If there is no mouse ([0xD5FD6]==0), stores x=0, y=0, buttons=0.

Otherwise calls int 33h AX=3:
- x = CX >> [0xD5FE8]
- y = DX >> [0xD5FE9]
- buttons = BX (bit 0 left, bit 1 right, bit 2 middle)

In mode 12 ([0xD5F98]==12) it rescales x=(x-40)/2 and y=(2*(y-24)+1)/3; this branch is not used here.

Then, if [0xDA100] != 0 and x >= [0xDA2F0] (surface width), x is decremented once.

Writes *x, *y and *buttons as 32-bit ints. Returns 0. Does not touch the graphics cursor.
- **Evidence:** 0x8C232 tests [0xD5FD6]. 0x8C23B mov ax,3; int 33h. 0x8C24A-0x8C25A shifts by [0xD5FE8] and [0xD5FE9]. 0x8C283-0x8C294 edge adjustment. 0x8C295-0x8C2A2 stores to [ebp+8], [ebp+0xC], [ebp+0x10]. 0x8C2AB no-mouse path.
- **Use in the game:** fg_mousepos(&mx 0xE1628, &my 0xE162C, &btn 0xE1630) at 0x13CC6. At 0x1660F the call is in a loop until btn != 0. Also 0x29AAD, 0x2F507.
- **Check's correction:** No errors found. [0xD5FD6]==0 leads to 0x8C2AB, which stores zeros. Otherwise int 33h AX=3, then shr by [0xD5FE8] and [0xD5FE9]. The mode-12 rescale is at 0x8C25C. The x edge adjustment is signed jl against [0xDA2F0], applied only when [0xDA100]!=0. Stores are 32-bit to [ebp+8], [+0xC] and [+0x10]; eax=0. Caller 0x13CC6 pushes 0xE1628 last (the x pointer), as stated. The shift values in mode 25 remain unchecked (open question 5).

## 0x8C331 qsort (Watcom C runtime) — D, check: confirmed (D)

- **Signature:** `void qsort(void *base, size_t num, size_t width, int (*compar)(const void*, const void*)); Watcom registers: eax=base, edx=num, ebx=width, ecx=compar. compar is itself called with eax, edx.`
- **Semantics:** Not stable. The order of elements that compare equal follows this exact algorithm, a Bentley-McIlroy style quicksort with an explicit stack.

Swap mode:
- width == 4 with base aligned to 4: dword swaps, and the pivot is held in a temporary.
- otherwise: byte/dword swap helper 0x8C2B3, and the pivot is first swapped into a[0].

Loop on (lo, n):
- n <= 1: pop.
- n < 16: gapped insertion sort, first with gap 3 then gap 1. For i = lo+gap, lo+2*gap, ... < lo+n: j = i; while j > lo and cmp(a[j-gap], a[j]) > 0, swap them and j -= gap. Note that the gap-3 pass only visits indices that are multiples of 3.
- n >= 16, pivot choice: middle element a[n/2]. If n > 29, median-of-3 (0x8C2D9) of first, middle, last. If n > 42, ninther with d = n/8: med3(lo, lo+d, lo+2d), med3(mid-d, mid, mid+d), med3(last-2d, last-d, last), then med3 of those three.
- Partition with pa=pb=lo, pc=pd=last:
  - while pb <= pc and (r = cmp(pb, piv)) <= 0: if r == 0 { swap(pa, pb); pa++ } pb++
  - while pb <= pc and (r = cmp(pc, piv)) >= 0: if r == 0 { swap(pc, pd); pd-- } pc--
  - if pb > pc, stop; else swap(pb, pc); pb++; pc--; repeat.
- Move the equal runs to the middle: r = min(pa-lo, pb-pa); vecswap(lo, pb-r, r). r = min(pd-pc, end-pd-1); vecswap(pb, end-r, r).
- Recurse: left part = (pb-pa) elements at lo, right part = (pd-pc) elements at end-(pd-pc). If right >= left: push right, continue with left. Else if left <= 1: pop. Else push left, continue with right.
- **Evidence:** 0x8C33C-0x8C358 swap-mode selection ((base|width)&3, width>4). 0x8C35B gap 3*w and step 2*w. 0x8C377 n<=1. 0x8C384 n<16 runs the insertion code 0x8C391-0x8C413. 0x8C444 mid. 0x8C459 n>0x1D. 0x8C485 n>0x2A. Median-of-3 calls at 0x8C4C3, 0x8C4DA, 0x8C4F0, 0x8C501. Partition 0x8C563-0x8C63C. vecswaps 0x8C641-0x8C6E7. Stack push/pop 0x8C6E8-0x8C769 and 0x8C415-0x8C43F.
- **Use in the game:** - 0x13DDA: qsort(0xF069A, count, 2, cmp 0x13DE8): 16-bit entries.
- 0x34CE6: qsort(0x108820, n, 16, cmp 0x3530C).
- Also 0x35044, 0x36BC9.
- **Check's correction:** No errors found. Locals: base=[ebp-0x148], n=[ebp-0x14C], w=[ebp-0x150], cmp=[ebp-0x14]. Swap mode: (base|w)&3 gives 2, else w>4 gives 1, else 0 (dword path, pivot copied to [ebp-0x44]). Modes 1 and 2 both use helper 0x8C2B3 and swap the pivot into a[lo] (0x8C519); 0x8C528 is dead code. Insertion sort: gap 3w then w (gap -= 2w), i stepping by the gap, so the gap-3 pass touches only indices that are multiples of 3; inner loop while j>lo and cmp(a[j-gap],a[j])>0. n<=1 and n>=16 use unsigned tests. Mid = lo+(n>>1)*w. n>0x1D gives med3(lo,mid,last); n>0x2A gives the ninther with d=(n>>3)*w. The med3 at 0x8C2D9 is a correct median. The partition and the two vecswaps (signed min, then unsigned min) match. The push and recurse logic at 0x8C6E8-0x8C769 matches, including popping when right<left and left<=1. Caller 0x13DDA: eax=0xF069A, edx=[0xF0696]>>16, ebx=2, ecx=0x13DE8.

## 0x8C76E fg_box (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_box(int minx, int maxx, int miny, int maxy); cdecl stack`
- **Semantics:** Draws an unfilled rectangle as four filled bands, using the box depths set by fg_boxdepth (0x912BE): xdepth=[0xD5FA4], ydepth=[0xD5FA8], both 1 after mode set (0x80D31). With xd=[0xD5FA4] and yd=[0xD5FA8], it calls fg_clprect (0x9D21A) four times in this order:
1. (minx, maxx, miny, min(miny+yd-1, maxy))
2. (minx, maxx, max(maxy-yd+1, miny), maxy)
3. (minx, min(minx+xd-1, maxx), miny, maxy)
4. (max(maxx-xd+1, minx), maxx, miny, maxy)

The bands overlap at the corners; with opaque fills this does not matter.

fg_clprect (0x9D21A, D) returns without drawing if minx > clip maxx [0xD5FB0], maxx < clip minx [0xD5FAC], miny > clip maxy [0xD5FB8] or maxy < clip miny [0xD5FB4]. Otherwise it clamps each edge to the clip region and calls fg_rect (0x8C8B2).

Net effect: the box is clipped to the clip region, in the current colour. The graphics cursor is unchanged. Returns 0.
- **Evidence:** 0x8C772 esi=[0xD5FA8]. 0x8C778-0x8C7B0 top and bottom bands. 0x8C7B8 esi=[0xD5FA4]. 0x8C7BE-0x8C7F6 left and right bands. All four go through call 0x9D21A. 0x9D21A-0x9D285 clip-and-clamp, then call 0x8C8B2. fg_boxdepth at 0x912BE stores ([ebp+8], [ebp+0xC]) into [0xD5FA4], [0xD5FA8].
- **Use in the game:** 0x140AD: fg_box(cell rectangle), then fg_setcolor(15), then four fg_dashrel calls (a dotted inner frame). 0x3A88D, 0x3A8B7, 0x3A8E7: UI button frames.
- **Check's correction:** No errors found in the four calls: (minx,maxx,miny,min(miny+yd-1,maxy)), (minx,maxx,max(maxy-yd+1,miny),maxy), (minx,min(minx+xd-1,maxx),miny,maxy), (max(maxx-xd+1,minx),maxx,miny,maxy), all via 0x9D21A. fg_clprect 0x9D21A rejects on minx>clip maxx, maxx<clip minx, miny>clip maxy or maxy<clip miny, clamps each edge, then calls 0x8C8B2 and returns 0. fg_boxdepth 0x912BE stores x into [0xD5FA4] and y into [0xD5FA8]; both are set to 1 at 0x80D31. One caveat for the port: the game calls fg_boxdepth(2,2) at 0x3DCF6 and 0x3E063 (and (1,1) at 0x3D96B, 0x3DD63, 0x3E0D0), so the depth is not always 1 at fg_box time.

## 0x8C803 fg_dashrel (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_dashrel(int ix, int iy, int pattern); cdecl stack`
- **Semantics:** Calls fg_dash (0x91AB3) with ([0xD5F9C]+ix, [0xD5FA0]+iy, pattern), i.e. a dashed line relative to the graphics cursor. Returns 0.

fg_dash(x, y, pattern), D:
- The start is always the cursor C (no endpoint swap). Let dx=|x-Cx|, dy=|y-Cy|, sx=sign(x-Cx), sy=sign(y-Cy); a zero difference counts as +1.
- Major axis is x if dx >= dy, else y. A straight step moves along the major axis only; a diagonal step moves (sx, sy).
- d = 2*minor - major. Repeat major times: rotate the 16-bit pattern left by 1; if its bit 0 (the old bit 15) is set, plot the current point; then if d < 0 { straight step; d += 2*minor } else { diagonal step; d += 2*minor - 2*major }.
- Set the cursor to the final point, which is (x, y). Rotate once more and plot the endpoint if that bit is set.
- In total major+1 points; point i uses pattern bit 15-(i mod 16), MSB first.

Each point goes through fg_point (0x90FC7, D). It skips points outside the clip region [0xD5FAC..0xD5FB0] x [0xD5FB4..0xD5FB8] and stores colour byte [0xD5F61] (mode 25: banked store; VB: [0xD6606] + y*[0xD65FE] + x). So the dashes are clipped per pixel.
- **Evidence:** 0x8C806-0x8C81C computes cursor+offset and calls 0x91AB3. 0x91ABB-0x91B14 sets up the steps. Loop 0x91B15-0x91B44: rol ax, test 1, call 0x90FC7. 0x91B46-0x91B5D stores the cursor and plots the endpoint. 0x90FC7-0x90FF2 clip test. 0x9119C / 0x911C0 store [0xD5F61].
- **Use in the game:** Only at 0x140C8, 0x140D9, 0x140EA, 0x140FB, after fg_setcolor(15):
- fg_dashrel(23, 0, 0xDDDD)
- (0, 22, 0xDDDD)
- (-23, 0, 0xDDDD)
- (0, -22, 0xDDDD)

This draws a dotted square around a cell.
- **Check's correction:** No errors found. 0x8C806-0x8C81C calls fg_dash(Cx+ix, Cy+iy, pattern) and returns 0. fg_dash 0x91AB3: sx and sy are +1 when the difference is >=0. Major axis is x when dx>=dy (signed jge). Straight step is (sx,0) or (0,sy). d=2minor-major. The loop runs major times: rol ax, plot if bit0 (0x90FC7(x,y)), then the step. At the end the cursor is set to the computed point, then a final rol and plot. fg_point 0x90FC7 clips to [0xD5FAC..0xD5FB0]x[0xD5FB4..0xD5FB8]; table 0x911E5 has [0x64]=0x9119C (banked) and [0x20]=0x911C0 (VB); both store [0xD5F61]. Callers 0x140C8-0x140FB pass (23,0), (0,22), (-23,0), (-22? no: 0,-22), each with pattern 0xDDDD, after fg_setcolor(15) at 0x140B7.

## 0x8C897 itoa (Watcom C runtime) — D, check: confirmed (D)

- **Signature:** `char *itoa(int value, char *buffer, int radix); Watcom registers eax=value, edx=buffer, ebx=radix; returns buffer in eax (ecx preserved)`
- **Semantics:** If radix == 10 and value < 0: writes '-' and converts -value. Then utoa (0x8C84D) converts the unsigned value: repeated unsigned division by the radix, digits from "0123456789abcdefghijklmnopqrstuvwxyz" (table at 0x8C828), written reversed, NUL-terminated. Returns the original buffer pointer. For any other radix, negative values are converted as unsigned 32-bit.
- **Evidence:** 0x8C89A cmp ebx,10. 0x8C89F test eax. 0x8C8A3 neg; store '-'. call 0x8C84D. 0x8C84D-0x8C896: div loop into a stack buffer, then reverse copy. Digit string at 0x8C828.
- **Use in the game:** Always radix 10 in the game (mov ebx,0xA before every call), into a stack buffer, e.g. itoa(n, buf=[ebp-0x84], 10) at 0x165C2 and 0x17463, followed by text output (89 call sites).
- **Check's correction:** No errors found. 0x8C89A checks radix==10 and value<0, then neg and stores '-'. utoa 0x8C84D does an unsigned div loop into a stack buffer with a leading NUL, digits from table 0x8C828, and a reverse copy including the terminator. itoa returns the original buffer saved in ecx (0x8C8AE); ecx is preserved and edx is clobbered. Caller 0x18D54: ebx=10, edx=ecx=[ebp-0x4c]. There are 89 call sites.

## 0x8C8B2 fg_rect (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_rect(int minx, int maxx, int miny, int maxy); cdecl stack`
- **Semantics:** Fills the inclusive rectangle [minx..maxx] x [miny..maxy] with the current colour byte (low byte of [0xD5F62], as set by fg_setcolor). Row by row from miny, each row width = maxx+1-minx, using rep stosd/stosw/stosb.

No clipping: neither the clip region nor the screen. The clipped version is fg_clprect 0x9D21A.

No check for empty or inverted rectangles. maxx < minx or maxy < miny leads to a huge or garbage fill in the original, so callers must order the arguments.

Targets:
- mode 25: banked video memory, stride [0xDA100], page bank [0xD65DA]
- open virtual buffer: [0xD6606] + y*[0xD65FE] + x

The graphics cursor is unchanged. Returns 0.
- **Evidence:** 0x8C8BD edi=minx, esi=miny. 0x8C8D0 jumps through the 0x8CDD8 table: [0x64] = 0x8CB84, [0x20] = 0x8CD82. 0x8CBA3-0x8CBB7 / 0x8CD9B-0x8CDAF: rows = maxy+1-miny, width = maxx+1-minx, eax=[0xD5F62]. Fill loops 0x8CBBC (with 64 KB bank crossing) and 0x8CDB4. 0x8CDCE returns eax=0. Also reached from fg_clprect 0x9D285 and from fgf_print.
- **Use in the game:** Game: 0x1838D fg_rect(x-2, x+w/2.., ...) as a bar/panel fill. Also 0x183E1, 0x3A85F, 0x3AAB2. Used by fg_box through fg_clprect.
- **Check's correction:** No errors found. edi=minx, esi=miny. Table 0x8CDD8: [0x64]=0x8CB84, [0x20]=0x8CD82. Both compute rows = maxy+1-miny and width = maxx+1-minx, then fill with eax=[0xD5F62] using rep stosd/stosw/stosb. 0x8CB84 uses stride [0xDA100], bank [0xD65DA] via [0xD65C6], and handles a mid-row bank crossing at 0x8CBFD. 0x8CD82 uses [0xD6606]+y*[0xD65FE]. There is no clipping and no ordering check; rows<=0 underflows the dec/jne loop. Returns 0 at 0x8CDCE. The call from fg_clprect at 0x9D285 is confirmed.

## 0x8CE50 fg_makegif (Fastgraph/Image) — D, check: corrected (D)

- **Signature:** `int fg_makegif(int minx, int maxx, int miny, int maxy, char *filename); cdecl stack`
- **Semantics:** Writes the region [minx..maxx] x [miny..maxy] of the current surface (active page, or the open virtual buffer through the [0xD5F88] dispatch) to a new GIF87a file.

Header: logical screen = ([0xDA2F0], [0xDA36C]) (640x480 here). Global colour table with packed byte 0x80|(bits-1): 256 entries for [0xD9F8C]=256. The colour table comes from the current palette (not traced for mode 25).

Image descriptor: left=minx, top=miny, w=maxx-minx+1, h=maxy-miny+1, not interlaced. LZW with min code size 8, clear=256, EOI=257, 12-bit maximum with a clear when full. Ends with the 2-byte trailer at 0xD5CED. The fg_imagebuf buffer ([0xD668A], size [0xD6690]) is used as the staging buffer for the writes.

Only works if mode [0xD5F98] >= 13 or == 9; otherwise returns 0 and writes nothing.

Returns 1 if create (int 21h AH=3Ch, attributes 0) fails, else 0.

The game uses it only as a scratch screen save, file "junk"/"junk2", reloaded later with fg_showgif("junk",3) at the same top-left with the palette kept. A port can treat the pair as save/restore of the pixel rectangle.
- **Evidence:** 0x8CE5B-0x8CE6B mode test. 0x8CE6D int 21h AH=3Ch, return 1 at 0x8CE78. 0x8CEC8 'GIF87a' at 0xD5CE0, with words [0xDA2F0], [0xDA36C] and the packed byte from [0xD9F8C]. 0x8CF04-0x8CF15 width/height +1. 0x8CF1D-0x8CF3C ',' descriptor with minx, miny, w, h, flags 0 and min code size. Clear/EOI from [0xD9F8C] (0x8CF4D-0x8CF5A). LZW loop 0x8CF85-0x8CFD4 (code size up to 12 at 0x8CFB6). Trailer and close at 0x8D012-0x8D026.
- **Use in the game:** - fg_makegif(0, 639, 0, 479, "junk") at 0x19C48 (full screen)
- fg_makegif(0, 639, 0, 478, "junk") at 0x3FC75
- fg_makegif(x, x+w-1, y, y+h-1, "junk") at 0x199AE

Each call is bracketed by 0x78884 / 0x788AC, probably mouse hide/show (G).
- **Check's correction:** One byte is wrong: the image-descriptor packed byte is not 0. At 0x8CF34-0x8CF3C the code writes al=[ebp-0x1c]-1 (=7 for 256 colours) as the descriptor's packed byte, then [ebp-0x1c] (=8) as the LZW minimum code size. So the packed byte is 0x07: no local table and not interlaced, with the unused local-table-size bits = 7. Everything else checks out. Mode gate: [0xD5F98]>=13 (signed) or ==9, else return 0 (0x8CE82). Create int 21h/3Ch with cx=0, failure returns 1. Header: the 13-byte template at 0xD5CE0 is 'GIF87a' followed by [0xDA2F0] and [0xDA36C] as words, packed byte ((([0xD9F8C]<<2)>>8)|3)+1 = bits, stored as 0x80|(bits-1) = 0x87 (colour-resolution field 0), and background and aspect 0. Descriptor fields are minx, miny, maxx-minx+1, maxy-miny+1. clear=[0xD9F8C] and EOI=+1. Code size grows up to 12, then a clear is emitted (0x8CFB6-0x8CFC9). The trailer at 0xD5CED is the bytes 00 3B. Staging buffer: [0xD668A], with [0xD6690]-0x100 as the flush threshold. The pixel source is chosen by the [0xD5F88] dispatch at 0x8D39D (table 0x8D5C5); its body and the colour-table writer were not traced. Caller 0x19C48: fg_makegif(0,639,0,479,"junk"), between 0x78884 and 0x788AC.

## 0x8DEE1 fg_settrans (Fastgraph) — D, check: corrected (G)

- **Signature:** `int fg_settrans(int number, int flag);  /* cdecl stack */`
- **Semantics:** Marks colour `number` as transparent (flag != 0) or opaque (flag == 0) for fg_vbtccopy (0x8DF22). Byte table 0xD625A[number & 0xFF] = (flag & 0xFF) | ((flag >> 8) & 0xFF), so only the low 16 bits of flag count and any nonzero value means transparent. It also sets bit `number` of the 16-bit mask at 0xD5F92 to (that byte != 0), which is the 16-colour-mode copy and does nothing useful for numbers >= 16. Returns 0. fg_setmode (0x80E5D) zeroes the whole 256-byte table, so no colour is transparent until this is called. Does not affect fg_clpimage/fg_flpimage, which always treat colour 0 as transparent.
- **Evidence:** 0x8DEF3 mov [ebx+0xD625A],al after `or al,ah`; 0x8DEF9-0x8DF16 builds the bit and and/or's it into word [0xD5F92]; the table is read only by xlatb in 0x8DF22 (0x8DF30, 0x8DFC9); 0x80E5D-0x80E6A rep stosd 0x40 dwords of 0 into 0xD625A.
- **Use in the game:** Only caller is 0x2F1F5, fg_settrans(0, 1): colour 0 is transparent before the fg_vbtccopy at 0x2F42C.
- **Check's correction:** The behaviour is read correctly from the code: table[0xD625A + (number&0xFF)] = (flag | flag>>8) & 0xFF at 0x8DEF1-0x8DEF3, the bit built by sub/rcl at 0x8DEF9-0x8DF16, returns 0, and setmode clears the table at 0x80E5D-0x80E6A. The name is wrong. Fastgraph has no fg_settrans. The routine that sets one colour's transparency for fg_vbtccopy/fg_tcxfer is fg_tcdefine(int number, int attr). The 16-bit word at 0xD5F92 is fg_tcmask's mask for colours 0-15; its only writers are 0x80CF8 in setmode and this routine. Small fix: the shift count is cl = number&0xFF, which x86 masks to 5 bits. Numbers 16-31 leave the word unchanged, but 32-47 change bit (number-32), so 'does nothing for >= 16' is not quite true. It does not matter in 256-colour mode. Caller 0x2F1EC-0x2F1F5 pushes (0, 1), confirmed. Name grade G (API knowledge); behaviour D.

## 0x8DF22 fg_vbtccopy (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_vbtccopy(int minx, int maxx, int miny, int maxy, int newx, int newy, int source, int dest);  /* cdecl stack */`
- **Semantics:** Copies the inclusive rectangle [minx..maxx] x [miny..maxy] from virtual buffer `source` to virtual buffer `dest`, skipping pixels whose colour is marked transparent (table 0xD625A[pix] != 0, set by fg_settrans). (newx, newy) is the LOWER-left corner in dest: dest row for source row miny is newy - (maxy - miny), and rows go downward from there. Pixel-exact C: for (r = 0; r <= maxy-miny; r++) for (c = 0; c <= maxx-minx; c++) { p = S[(miny+r)*Sw + minx + c]; if (!trans[p]) D[(newy-(maxy-miny)+r)*Dw + newx + c] = p; }. Here S/D = vb base table 0xCB196[h & 0x1F] and Sw/Dw = vb width table 0xCB296[h & 0x1F]. Handles are masked with & 0x1F, not range-checked. If either base is 0 (undefined vb), it does nothing. There is NO clipping, it does not use the clip region or the active vb, and the graphics cursor is unchanged. It copies 0xD9F8C bytes of the transparency table (the colour count, 256 here) to the stack first. Returns 0.
- **Evidence:** 0x8DF49-0x8DF7C index 0xCB296/0xCB196 by args 7/8; 0x8DF85-0x8DF9F src = miny*Sw+minx, dst = (newy+miny-maxy)*Dw+newx; 0x8DFA1-0x8DFBB rows = maxy+1-miny, cols = maxx+1-minx; loop 0x8DFC6: lodsb/xlatb/`jne skip`/store ah.
- **Use in the game:** 0x2F42C (game 0x2F2xx): fg_vbtccopy(x+8, x+8+w-1, y+6, y+6+h-1, x+8, y+6+h-1, [0xF52CC], [0xF52C8]), the same rectangle copied from the vb in 0xF52CC to the vb in 0xF52C8 with colour 0 transparent (fg_settrans(0,1) at 0x2F1F5).
- **Check's correction:** Re-read 0x8DF22-0x8DFED. The arguments are +8 minx, +C maxx, +10 miny, +14 maxy, +18 newx, +1C newy, +20 source, +24 dest. Width and base come from 0xCB296 and 0xCB196, indexed by (h&0x1F)*4. If either base is 0 it jumps to 0x8DFE4 and returns 0. src = miny*Sw+minx. dst = (newy+miny-maxy)*Dw+newx. rows = maxy+1-miny, cols = maxx+1-minx. The xlatb uses the stack copy of the table, and a pixel is skipped when its entry is nonzero. No clipping, returns 0. Small detail: the copy is [0xD9F8C]>>2 dwords into a 0x110-byte stack buffer, so it rounds down to a multiple of 4. That 0xD9F8C holds the colour count of 256 is G. The caller at 0x2F341-0x2F42C pushes dest = [0xF52C8] (ecx) and source = [0xF52CC] (esi), as stated.

## 0x8E444 fg_getimage (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_getimage(unsigned char *buffer, int width, int height);  /* cdecl stack */`
- **Semantics:** Reads a width x height block of pixels (8-bit, one byte per pixel in 256-colour modes and virtual buffers) whose LOWER-left corner is the graphics cursor (x = [0xD5F9C], y = [0xD5FA0]). The buffer is stored BOTTOM ROW FIRST: buffer[r*width + c] = pixel(x + c, y - r) for r = 0..height-1, c = 0..width-1. There is no clipping (not even against the screen) and the cursor is unchanged. Returns 0. Dispatches on mode index [0xD5F88]/4 through the table at 0x8EA96. Index 8 = a virtual buffer is active (fg_vbopen 0x87C31 sets 0xD5F88 = 0x20) -> 0x8EA44: src = [0xD6606] + y*[0xD65FE] + x, with the vb width at 0xD65FE. Index 25 = mode 25, 640x480x256 (fg_setmode(0x19) at 0x109F3) -> 0x8E7EF: the same through 64 KB banks (offset y*[0xDA100] + x, bank [0xD65DA] + carry, bank switch [0xD65BE], rows step up by pitch with bank borrow).
- **Evidence:** 0x8E44F-0x8E462 ebx = cursor x, eax = cursor y, jmp [0xD5F88 + 0x8EA96]; vb path 0x8EA44-0x8EA87: rep movs width bytes, `sub esi, width+vbwidth` (moves up one row), dec height; SVGA path 0x8E7EF-0x8E849 `sub si, ax` with ax = width+pitch; returns via 0x8EA8C xor eax,eax.
- **Use in the game:** Only caller is 0x3AF1D in the sprite-grab routine 0x3AE90(spr, x, y, w, h): fg_move(x, y+h-1); fg_getimage(tmp, w, h); fg_unpack(tmp, spr->data, w*h). So a sprite's top-left is (x, y) and its data is stored bottom row first.
- **Check's correction:** Re-read 0x8E444-0x8E462, the vb path 0x8EA44-0x8EA87 and the mode 24-27 path 0x8E7EF-0x8E849. The table entry for index 25 (0x8EAFA) is 0x8E7EF. The vb path reads through selector [0xD660A], adds [0xD6606] + y*[0xD65FE] + x, and after each row of width bytes does `sub esi, width+vbw`, so it goes upward and stores the bottom row first. No clipping, cursor unchanged, returns 0 via 0x8EA8C. Caller 0x3AE90 confirmed: spr eax, x edx, y ebx, w ecx, h on the stack, then fg_move(x, y+h-1) at 0x3AF09 and fg_getimage(tmp, w, h) at 0x3AF1D. Sprite layout: data at +0, w (word) at +4, h (word) at +6.

## 0x8EB0E fg_unpack (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_unpack(unsigned char *source, unsigned char *dest, unsigned int size);  /* cdecl stack */`
- **Semantics:** Converts a mode-specific packed bitmap into one pixel per byte. `size` is the number of SOURCE bytes. CGA 4-colour (index 4, 5, 12): 4 bytes out per byte. 2-colour (index 6, 11): 8 out. 16-colour (index 9, 13-18, 28, 29): high nibble first, then the low one. Index 8 (virtual buffer) and 19-27, which includes mode 25 = 640x480x256: a plain memcpy(dest, source, size) (rep movsd/movsw/movsb). Text modes and index 10 do nothing. Returns 0, globals untouched.
- **Evidence:** 0x8EB16-0x8EB27 jmp [0xD5F88 + 0x8EBD0]; table entries 8 and 19..27 = 0x8EBB5 (shr ecx/rep movsd/adc/rep movsw/adc/rep movsb); 0x8EBA4 nibble split; 0x8EB2E/0x8EB77 bit splits.
- **Use in the game:** Only caller is 0x3AF37: fg_unpack(tmp, spr->data, w*h) right after fg_getimage, which in mode 25 or a vb is just a copy.
- **Check's correction:** Table 0x8EBD0 checked. Indices 0-3, 7 and 10 do nothing. 4, 5 and 12 go to 0x8EB77 (2 bits per pixel, 4 bytes out per byte in). 6 and 11 go to 0x8EB2E (1 bit, 8 out). 9, 13-18, 28 and 29 go to 0x8EBA4 (high nibble first). 8 and 19-27 go to 0x8EBB5 (memcpy of size bytes). ecx = size = source bytes. Returns 0. Caller 0x3AF37: fg_unpack(tmp, spr->data, w*h).

## 0x8EC48 fg_clpimage (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_clpimage(const unsigned char *buffer, int width, int height);  /* cdecl stack */`
- **Semantics:** Draws a width x height 8-bit image (bottom row first, the fg_getimage layout) with its LOWER-left corner at the graphics cursor (x = [0xD5F9C], y = [0xD5FA0]), clipped to the clip region (inclusive): minx [0xD5FAC], maxx [0xD5FB0], miny [0xD5FB4], maxy [0xD5FB8]. Colour 0 is always transparent: zero bytes are skipped, and the fg_settrans table is not used. Reimplementation: lclip = max(0, minx - x); x0 = max(x, minx); top = y + 1 - height; if (top < miny) height -= miny - top; bclip = 0; y0 = y; if (y > maxy) { bclip = y - maxy; height -= bclip; y0 = maxy; } if (height <= 0) return -1; rclip = max(0, x + width - 1 - maxx); w = width - rclip - lclip; if (w <= 0) return -1; then for (r = 0; r < height; r++) for (c = 0; c < w; c++) { p = buffer[(bclip + r)*width + lclip + c]; if (p) PIX(x0 + c, y0 - r) = p; }. Returns 0 when drawn, -1 when fully clipped. The cursor is not moved. Index 8 (active vb) -> 0x8F944: dest = [0xD6606] + y0*[0xD65FE] + x0, row step -(w + vbwidth). Index 25 (640x480x256) -> 0x8F65D: the same with 64 KB bank switching ([0xD65C6] = write bank, base bank [0xD65DA]).
- **Evidence:** 0x8EC56-0x8ECD8 left/top/bottom clipping, skip = bclip*width (0x8ECAD mul [ebp+0xC]), `jle 0x8ECDF` -> eax = -1; jmp [0xD5F88 + 0x8F9B8]; vb path 0x8F944-0x8F9AA: right clip, `sub [ebp+0xC], lclip; jg`, loop lodsb / `or al,al; je` skip / store, `add esi, rclip`, `sub edi, w+vbwidth`; 0x8F9AE xor eax,eax.
- **Use in the game:** 0x3AFE3 in the sprite-draw routine 0x3AF8C(spr, x, y): fg_move(x, y + spr->h - 1); fg_clpimage(spr->data, spr->w, spr->h), so the top-left is at (x, y). Its only direct caller, but 0x3AF8C itself is called widely (for example 0x3D958 draws sprite 0xF520C at (0x1E1, 0x170)).
- **Check's correction:** Re-read the clipping head 0x8EC56-0x8ECE4 and the vb path 0x8F944-0x8F9AE. The right clip uses the original cursor x [0xD5F9C] (x+width-1 > maxx). The width in [ebp+0xC] has rclip and then lclip subtracted, and `jg` else -1. The source starts at buf + bclip*width. Each row does esi += lclip, copies w bytes skipping 0, esi += rclip, edi -= w+vbw. Returns 0 at 0x8F9AE, or -1 at 0x8ECDF. Index 25 (table 0x8FA1C) is 0x8F65D, which I checked: same clipping, banked write via [0xD65C6], same `or al,al; je` transparency. The reimplementation is exact. Caller 0x3AF8C confirmed: fg_move(x, y+h-1), then clpimage(spr->data, w, h).

## 0x8FA30 fg_flpimage (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_flpimage(const unsigned char *buffer, int width, int height);  /* cdecl stack */`
- **Semantics:** Same as fg_clpimage (0x8EC48), including lower-left corner at the graphics cursor, inclusive clipping to [0xD5FAC..0xD5FB0] x [0xD5FB4..0xD5FB8], colour 0 transparent, return -1 when fully clipped and 0 otherwise, cursor unchanged, but the image is MIRRORED left-right. Screen column x + j shows source column width-1-j of the same row. Reimplementation with lclip, rclip, bclip, x0, y0 and the clipped height as in fg_clpimage: w = width - lclip - rclip; for (r...) for (c = 0; c < w; c++) { p = buffer[(bclip + r)*width + (width - 1 - lclip - c)]; if (p) PIX(x0 + c, y0 - r) = p; }. Clipping is applied in screen space. Index 8 (vb) -> 0x9084A, index 25 (640x480x256, banked) -> 0x90552. The direction flag is set (std) for the source reads and cleared on exit.
- **Evidence:** 0x8FA3D std; identical clipping head 0x8FA3E-0x8FAC0 (table 0x908C8); vb path 0x9084A-0x908B8: esi = buf + bclip*width - 1, per row `add esi, width; sub esi, lclip`, backward lodsb, forward `mov es:[edi],al; inc edi`, then `sub esi, rclip; add esi, width`; 0x908BC xor eax,eax; 0x908BE cld.
- **Use in the game:** Only caller is 0x3B04B in 0x3AFF4(spr, x, y), the mirror twin of 0x3AF8C: fg_move(x, y+h-1); fg_flpimage(spr->data, w, h).
- **Check's correction:** Re-read 0x8FA30-0x8FACC and the vb path 0x9084A-0x908C7. The clipping head is the same as clpimage, and std is at 0x8FA3D. Traced the source pointer: start = buf+bclip*width-1, each row += width-lclip, then read w bytes backward down to column rclip, then -rclip +width = next row start-1. So screen column x0+c gets source column width-1-lclip-c, as stated. Clipping is in screen space. Unlike clpimage, the vb path does not modify [ebp+0xC]. cld at 0x908BE and 0x908C6. Index 25 is 0x90552 (table 0x9092C); I did not trace that body. Caller 0x3AFF4/0x3B04B confirmed.

## 0x90940 fg_vbcut (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_vbcut(int minx, int maxx, int miny, int maxy, int newx, int newy);  /* cdecl stack */`
- **Semantics:** Copies the inclusive rectangle [minx..maxx] x [miny..maxy] from the VIDEO page (the real screen in the mode saved at fg_vbopen, [0xD65FA]) to the ACTIVE virtual buffer. (newx, newy) is the destination's LOWER-left corner: screen row miny goes to vb row newy - (maxy - miny), and rows go downward. Opaque copy with no transparency and no clipping. In C: for (r = 0; r <= maxy-miny; r++) memcpy(VB + (newy-(maxy-miny)+r)*vbw + newx, SCREEN + (miny+r)*pitch + minx, maxx-minx+1). VB = [0xD6606], vbw = [0xD65FE], pitch = [0xDA100]. Mode 25 path 0x90D7D reads with 64 KB bank switching (read bank via [0xD65BE], base bank [0xD65DA]). It needs a vb open (it writes through 0xD6606 unconditionally). Returns 0, cursor unchanged.
- **Evidence:** 0x9094B-0x90961 edi = (newy+miny-maxy)*[0xD65FE] + newx + [0xD6606]; 0x9096E width = maxx+1-minx, row skip = vbw - width; 0x90983 rows = maxy+1-miny; jmp [[0xD65FA] + 0x90F0C] (the saved screen mode, not 0xD5F88 which is 0x20 while a vb is open); 0x90DAF-0x90DD5 rep movs, `add si, pitch-width`, `add edi, vbw-width`; 0x90F02 xor eax,eax.
- **Use in the game:** 0x3CCF5: fg_vbopen([0xF52C8]); fg_vbcut(0, 639, 0, 479, 0, 479); fg_vbclose(), which grabs the whole screen. 0x3CD49 does the same into [0xF52CC]. 0x54066 and 0x6E1E8: fg_vbcut(x, x+0xDB, y, y+h-1, x, y+h-1), a panel copied to the same position.
- **Check's correction:** Re-read 0x90940-0x9098F and the mode 24-27 path 0x90D7D-0x90E1C. edi = (newy+miny-maxy)*[0xD65FE] + newx + [0xD6606], es = [0xD660A]. width = maxx+1-minx, vb skip = vbw-width, rows = maxy+1-miny. The dispatch is on [0xD65FA], which fg_vbopen sets from [0xD5F88] at 0x87C96-0x87C9B. The banked source is miny*[0xDA100]+minx plus base bank [0xD65DA], read through [0xD65BE], and the row step is pitch-width. Index 8 in the 0x90F0C table is a no-op. If no vb was ever opened, [0xD65FA] is stale or 0. Callers 0x3CCF5 (0,639,0,479,0,479 between vbopen([0xF52C8]) and vbclose) and 0x54066 (x, x+0xDB, y, y+h-1, x, y+h-1) confirmed.

## 0x90F84 fg_getdacs (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_getdacs(int start, int count, unsigned char *values);  /* cdecl stack */`
- **Semantics:** Reads `count` DAC registers starting at `start` into values[0..3*count-1] as R,G,B triplets of 6-bit values (0..63). If the mode number [0xD5F98] < 13 or count == 0, it does nothing. If [0xD5F9A] (the vertical-retrace flag, set to 1 at 0x80D40 during setmode) is nonzero, it first busy-waits until port 0x3DA bit 3 (vertical retrace) is set. Then: cli; out 0x3C7, start; 3*count x in 0x3C9; sti. Returns 0. Port: return the current 6-bit palette.
- **Evidence:** 0x90F8A cmp byte [0xD5F98],0xD / jl; 0x90F96 jecxz; 0x90FA4-0x90FB0 retrace wait on 0x3DA; 0x90FB3 dl=0xC7 out; 0x90FBA 3x insb from 0x3C9 per count.
- **Use in the game:** 0x3CE0C and 0x3CEDB: fg_getdacs(0, 256, 0xF8B40), saving the full palette to 0xF8B40 (the buffer the fade-in at 0x3CFE4 writes its final target from).
- **Check's correction:** Re-read 0x90F84-0x90FC6. Signed byte compare of [0xD5F98] with 13 (jl skips). jecxz on count. If [0xD5F9A] != 0 it waits on 0x3DA until bit 3 is set. Then cli, out 0x3C7 with start, insb x3 per count from 0x3C9, sti, return 0. The arguments are (start, count, values). 0x80D40 does set [0xD5F9A] = 1 during setmode. Caller 0x3CE00-0x3CE0C pushes (0, 0x100, 0xF8B40), confirmed.

## 0x90FC7 fg_point (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_point(int x, int y);  /* cdecl stack */`
- **Semantics:** Sets pixel (x, y) to the current colour (byte [0xD5F61], written by fg_setcolor 0x87AC2) if minx[0xD5FAC] <= x <= maxx[0xD5FB0] and miny[0xD5FB4] <= y <= maxy[0xD5FB8], all inclusive. Otherwise it does nothing. Opaque: colour 0 is drawn too. The graphics cursor is not moved. Always returns 0. Index 8 (active vb) -> 0x911C0: VB[[0xD6606] + y*[0xD65FE] + x] = colour. Index 25 (640x480x256) -> 0x9119C: banked write at y*[0xDA100] + x.
- **Evidence:** 0x90FCC-0x90FF2 four clip compares (jl/jg -> 0x911DF); jmp [0xD5F88 + 0x911E5]; 0x911C0-0x911DC vb store of [0xD5F61]; 0x9119C-0x911BB SVGA store; 0x911DF xor eax,eax.
- **Use in the game:** 0x448D8 / 0x44966: c = fg_getpixel(x, y); fg_setcolor(table[c]) (tables 0xC2620 / 0xC2720); fg_point(x, y), recolouring a rectangle pixel by pixel. 0x3D265: fg_point(px - 0x28?, py - 0x24?) on a plotted map point (offsets from locals). Also used inside the library at 0x91B24/0x91B5D.
- **Check's correction:** Re-read 0x90FC7-0x91008 and the paths. Inclusive clip (jl/jg on minx, maxx, miny, maxy). The 0x911E5 table maps index 8 to 0x911C0, which writes [0xD5F61] to [0xD660A]:[0xD6606] + y*[0xD65FE] + x, and indices 24-27 to 0x9119C (banked through [0xD65C6]). Returns 0, cursor untouched. Caller 0x448D8 confirmed: getpixel at 0x448B4, setcolor(table 0xC2620[c]), point. At 0x3D265 the offsets are locals: fg_point(word[ebp-0x14]-[ebp-0x24... ]), i.e. x = word[ebp-0x14] - dword[ebp-0x28] and y = word[ebp-0xC] - dword[ebp-0x24]. They are not constants. The caller pre-checks 6 <= y <= 0x144.

## 0x9125D fg_setclip (Fastgraph) — D, check: corrected (D)

- **Signature:** `int fg_setclip(int minx, int maxx, int miny, int maxy);  /* cdecl stack */`
- **Semantics:** Sets the clip region (inclusive bounds) in the globals minx [0xD5FAC], maxx [0xD5FB0], miny [0xD5FB4], maxy [0xD5FB8]. If a virtual buffer is active (handle byte [0xD65F8] >= 0) it also stores them in that vb's clip tables 0xCB096 (minx), 0xCAF96 (maxx), 0xCB116 (miny) and 0xCB016 (maxy), indexed by handle*4. The values are not validated. Returns 0. Consequence: a clip set while a vb is open belongs to that vb. fg_vbclose (0x87D73) restores the screen's clip saved by fg_vbopen (0xD65E0..0xD65EC), and fg_vbopen (0x87C31) loads the vb's clip from those tables.
- **Evidence:** 0x91261-0x9127E stores to 0xD5FAC/B0/B4/B8; 0x91286-0x912B5 handle check and stores to the four tables; matching loads in fg_vbopen 0x87CE6-0x87D19 and restores in fg_vbclose 0x87D9B-0x87DBE.
- **Use in the game:** 0x3D61E (after fg_vbopen([0xF52C8])): fg_setclip(8, 0x277, 6, 0x144), the map window 8..631 x 6..324 inside the vb. 0x3D906: fg_setclip(0, 0x27F, 0, 0x1DF) restores the full 640x480.
- **Check's correction:** Name, argument order (minx, maxx, miny, maxy), the global stores and the return of 0 are right. The condition is wrong. At 0x91284-0x9128E it does `xor ebx,ebx; mov bl,[0xD65F8]; or ebx,ebx; jl`. ebx is zero-extended, so the jl can never be taken and the per-vb table stores ALWAYS run. With no vb active the handle byte is 0xFF (0x81309, 0x87DEC), so it writes to entry 255: maxx to 0xCB392, maxy to 0xCB412, minx to 0xCB492 and miny to 0xCB512. Those addresses are inside other Fastgraph data, the 0xCB356/0xCB3AA/0xCB4AA tables used by 0x816xx-0x81Dxx. In this game it is harmless: both callers run while vb [0xF52C8] is open. 0x3D61E comes right after vbopen at 0x3D608. 0x3D906 comes before vbclose at 0x3D935, so that 0..639 x 0..479 clip goes into the vb's own clip tables, not the screen's. The port should not reproduce the scribble.

## 0x912BE fg_boxdepth (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_boxdepth(int xdepth, int ydepth);  /* cdecl stack */`
- **Semantics:** Stores the border thickness used by fg_box: xdepth -> [0xD5FA4] (width of the left/right sides in pixels), ydepth -> [0xD5FA8] (height of the top/bottom sides). Returns 0. fg_setmode resets both to 1 (0x80D31/0x80D36). fg_box at 0x8C76E(minx, maxx, miny, maxy) draws four filled rects via 0x9D21A: top/bottom bands of ydepth rows (each clamped to the box) and left/right bands of xdepth columns.
- **Evidence:** 0x912C1-0x912CC two stores; the only readers of 0xD5FA4/0xD5FA8 are in 0x8C76E (0x8C772, 0x8C7B8), which uses them as band thickness for four 0x9D21A calls; 0x80D30-0x80D36 inc eax / store 1.
- **Use in the game:** 0x3D96B: fg_setcolor(15); fg_boxdepth(1, 1) before drawing frames; also 0x3DCF6, 0x3DD63, 0x3E063, 0x3E0D0.
- **Check's correction:** Re-read 0x912BE-0x912D4: xdepth goes to [0xD5FA4], ydepth to [0xD5FA8], returns 0. 0x80D30-0x80D36 resets both to 1. The only readers are in 0x8C76E. Top band: miny..min(miny+yd-1, maxy). Bottom band: max(maxy-yd+1, miny)..maxy. Left and right bands span miny..maxy with xdepth columns, clamped the same way. All four are calls to 0x9D21A(minx, maxx, miny, maxy). Caller 0x3D967-0x3D96B pushes (1, 1) after fg_setcolor(15). Confirmed.

## 0x912D5 fwrite (Watcom C runtime) — D, check: confirmed (D)

- **Signature:** `size_t fwrite(const void *buf /*eax*/, size_t size /*edx*/, size_t count /*ebx*/, FILE *fp /*ecx*/);  /* Watcom register convention */`
- **Semantics:** Standard fwrite. If the stream lacks _WRITE (flag 0x2 at fp+0xC), it sets errno via 0x94956(4) and the error flag 0x20 and returns 0. If size*count == 0 it returns 0. It allocates the buffer if fp+8 is 0 (0x95602). Binary streams (flag 0x40): when the buffer is empty and the remainder >= bufsize, it writes directly via 0x94222 (handle fp+0x10) the remainder rounded down to a multiple of 512, or all of it if that is 0. Otherwise it memcpys into the buffer, marks it dirty (0x1000) and flushes (0x94AC2) when full or unbuffered. Text streams go byte by byte through fputc 0x9E01C, which does the LF -> CRLF, temporarily switching an unbuffered stream to full buffering and flushing at the end. Returns bytes_written / size, or 0 if the error flag was set. The previous EOF/error bits are OR-ed back into the flags. FILE layout: +0 _ptr, +4 _cnt, +8 _base, +0xC _flag, +0x10 _handle, +0x14 _bufsize.
- **Evidence:** 0x912DF test [ecx+0xC],2; 0x912FA imul ebx,edx; 0x9132E test dl,0x40 (binary); 0x9134A `and ah,0xFE; xor al,al` (512 rounding) then call 0x94222; 0x913BF repne movsd memcpy into the buffer; 0x9145E call 0x9E01C per byte; 0x914B3 div edi (size).
- **Use in the game:** 81 call sites. 0x43E6F / 0x44CF3: fp = fopen("aciv.cfg", "wb") (0x7D8C8, mode string 0xB453B), then fwrite(0xF097C, 6, 1, fp) and fclose (0x7D95E): the 6-byte preferences. 0x4FF99: fwrite(&local, 4, 1, fp) while writing a file.
- **Check's correction:** Re-read 0x912D5-0x914BB. On _WRITE (0x2) failure: errno(4), flag 0x20, return 0. imul size*count, and 0 returns 0. Allocates the buffer if [fp+8] is 0. Saves flags&0x30 and clears them, then ORs them back at 0x914AA. On the binary (0x40) path, if the buffer is empty and the remainder >= bufsize, it writes remainder & ~0x1FF directly (or the whole remainder if that is 0). Otherwise it does the memcpy, sets 0x1000, and flushes when the buffer is full or _IONBF (0x400). The text path calls fputc 0x9E01C per byte, temporarily switching _IONBF to 0x100 and flushing at the end. Returns count/size, or 0 on error. The call count of 81 is verified. Call site 0x43E6F: fopen("aciv.cfg", 0xB453B="wb") and fwrite(0xF097C, 6, 1, fp). The twin site 0x44CF3 uses the mode string at 0xB458A (also "wb"), not 0xB453B. Helper names (fopen, fclose, flush and so on) remain G.

## 0x914BC fg_getpixel (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_getpixel(int x, int y);  /* cdecl stack */`
- **Semantics:** Returns the colour index (0..255, zero-extended to int) of pixel (x, y). If (x, y) lies outside the CLIP region (minx[0xD5FAC] <= x <= maxx[0xD5FB0], miny[0xD5FB4] <= y <= maxy[0xD5FB8], inclusive), it returns -1 (ax = 0xFFFF, then cwde), even if the pixel is on screen. Index 8 (active vb) -> 0x916B6: VB[[0xD6606] + y*[0xD65FE] + x]. Index 25 (640x480x256) -> 0x91695: banked read at y*[0xDA100] + x (read bank [0xD65BE]). Text modes return 0. The cursor is unchanged.
- **Evidence:** 0x914C1-0x914E7 clip compares -> 0x914FD `mov ax,0xFFFF`; jmp [0xD5F88 + 0x916D7]; 0x916B6-0x916D0 vb read, `xor ah,ah`; 0x916D2 cwde.
- **Use in the game:** 0x448B4 and 0x44942: c = fg_getpixel(x, y); fg_setcolor(byte table 0xC2620[c] or 0xC2720[c]); fg_point(x, y), a colour remap of a rectangle. Note that if the point is clipped, c = -1 indexes table[-1] (0xC261F or 0xC271F), but the fg_point that follows is clipped too, so nothing changes on screen.
- **Check's correction:** Re-read 0x914BC-0x91506 and 0x91695-0x916D6. Inclusive clip, and when clipped: ax=0xFFFF, cwde gives -1. Index 8 goes to 0x916B6 (vb read, xor ah,ah). Indices 24-27 go to 0x91695 (banked read via [0xD65BE]). Indices 0-3 and 7 go to 0x91506 and return 0. Small fix: index 10 jumps straight to 0x916D2 and returns cwde of y, i.e. garbage, not 0. Irrelevant for mode 25. The note about the caller at 0x448B4/0x448BC reading table[-1] when the point is clipped is also confirmed.

## 0x9174F fg_setvpage (Fastgraph) — D, check: corrected (D)

- **Signature:** `int fg_setvpage(int page);  /* cdecl stack */`
- **Semantics:** Makes video page (page & 0x3F) the VISIBLE page. If it equals the current visual page [0xD5F99], it does nothing. Otherwise it dispatches on [0xD5F88]; index 8 (vb active) re-dispatches on the saved screen mode [0xD65FA]. For mode 25 (0x91954) it stores page in BIOS 0x462 and [0xD5F99], sets [0xD65DB] from the page table 0xD6058[page*2], and (if [0xD5F9A] != 0) waits for display-enable to end on 0x3DA. Then it calls the chipset display-start routine [0xD65BA] with ecx = [0xD5F8C] and waits for vertical retrace. If a mouse is active ([0xD5FD6]) and mode < 20 it calls the mouse driver via [0xD6682]. Returns 0. The EGA/VGA path (0x91865) writes CRTC 0x0C/0x0D (start address), which shows this is the visual page, not the active (drawing) page. Port: no-op (single page).
- **Evidence:** 0x9175B and 0x3F / cmp [0xD5F99] / je return; table 0x919DE (entry 8 -> 0x91776 uses [0xD65FA]; entries 24-29 -> 0x91954); 0x918B8-0x918E3 CRTC 0x0D/0x0C writes in the planar path; 0x91999 call [0xD65BA]. The chipset routine behind 0xD65BA was not traced.
- **Use in the game:** Only caller is 0x44D10 in the exit path: fg_setvpage(0), then fg_setmode([0xF6058]) (the original mode) and printf("%s\n", msg). It makes sure page 0 is visible before the mode is restored.
- **Check's correction:** Name and purpose are right: page&0x3F, early-out if it equals [0xD5F99], index 8 re-dispatches on [0xD65FA], indices 24-29 go to 0x91954, the planar path writes CRTC 0x0D/0x0C at 0x918B8-0x918E3, the mouse call through [0xD6682] uses ax=0x1D when [0xD5FD6] is set and mode < 20, and it returns 0. Three corrections. (1) The pre-wait at 0x9198C loops `in al,dx; test al,1; jne`, i.e. while bit 0 is set. It waits until the display is ACTIVE (out of blanking), not 'for display-enable to end'. (2) The chipset routine [0xD65BA] is called under cli with eax = [0xD5F8C], ecx = 0 (0x91995-0x91997), ebx = the word from 0xD6058[page*2] and dx = the CRTC port ([0x463]). It is not called with ecx = [0xD5F8C]. (3) The [0xD65DB] store is only the low byte of that word. The 'waits for vertical retrace after' part (bit 3 at 0x919B4) is correct, and it only happens when [0xD5F9A] != 0. Caller 0x44D0E-0x44D10 pushes 0 and is followed by fg_setmode([0xF6058]), confirmed.

## 0x91AB3 fg_dash (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_dash(int ix, int iy, int pattern); cdecl stack, caller pops (add esp,0xC)`
- **Semantics:** Draws a dashed line from the graphics cursor (cx=[0xD5F9C], cy=[0xD5FA0]) to (ix,iy) using integer Bresenham. With dx=|ix-cx|, dy=|iy-cy|, sx/sy=sign (+1 if difference >= 0, else -1), major=max(dx,dy), minor=min(dx,dy); if dx<dy the major axis is y (a tie means x is major). err=2*minor-major; straight step (err<0): advance along the major axis only, err+=2*minor; otherwise diagonal step (both sx and sy), err+=2*minor-2*major. Plots major+1 pixels, both endpoints included. Pattern: the low 16 bits are rotated left by 1 before each pixel (rol ax), and the pixel is drawn when the bit rotated into bit 0 is set, so pixel i (i=0 at the cursor) uses bit 15-(i mod 16) (MSB first, repeats every 16 pixels, restarts on every call). Each pixel goes through fg_point (0x90FC7), so it is clipped per pixel to the clip box and drawn in the current colour. When the loop ends, the cursor is set to (ix,iy) (writes 0xD5F9C/0xD5FA0), then the endpoint is drawn with the next pattern bit. If (ix,iy)==cursor, one pixel is drawn using bit 15. Returns 0. fg_point 0x90FC7(x,y) (D): returns if x<[0xD5FAC] or x>[0xD5FB0] or y<[0xD5FB4] or y>[0xD5FB8] (inclusive clip box set by fg_setclip 0x9125D(minx,maxx,miny,maxy)). Otherwise it dispatches on [0xD5F88] through the table at 0x911E5. Mode 25 (index 0x64) goes to 0x9119C: byte at y*[0xDA100]+x, with the bank set through [0xD65C6], = colour byte [0xD5F61]. With a virtual buffer active (index 0x20) it goes to 0x911C0: byte [0xD6606] + y*[0xD65FE] + x = [0xD5F61].
- **Evidence:** 0x91AC2-0x91B14 sets up |dy|/|dx|, the signs, the swap and the 2*minor / 2*minor-2*major increments; the loop at 0x91B15 does rol ax / test ax,1 / call 0x90FC7; 0x91B46 stores the end point to 0xD5F9C/0xD5FA0; 0x91B52 plots the end point; xor eax,eax. The mode index comes from fg_setmode 0x80A33 ([0xD5F88]=mode*4), and the start-up code calls it with mode 0x19 (0x109C9/0x109F3; the fallback message says 640x480 256 colour). fg_vbopen 0x87C31 sets [0xD5F88]=0x20.
- **Use in the game:** 0x45032: after fg_setcolor(7) and fg_move(0x2D, 0x190-k*n) it calls fg_dash(0x23F, same y, 0x0101): a horizontal dotted line, with dots at pixels i where i mod 16 is 7 or 15 (every 8th pixel starting at x=0x2D+7). It is also called at 0x45160/0x452CA/0x453FD the same way. Its other caller is 0x8C803 = fg_dashrel(dx,dy,pattern), which calls fg_dash(cx+dx, cy+dy, pattern).

## 0x91B6E _dos_findfirst (Watcom C runtime) — D, check: confirmed (D)

- **Signature:** `unsigned _dos_findfirst(const char *path /*eax*/, unsigned attr /*edx*/, struct find_t *buf /*ebx*/); Watcom register convention`
- **Semantics:** INT 21h AH=1Ah sets the DTA to buf, then AH=4Eh does find-first with path in edx and attributes in cx. 0x9E0C0: if CF is set it sets errno through 0x9E0DF and returns the DOS error code (16-bit), otherwise it returns 0. 0x91BAA: if [0xD5D2A]==9 (a DOS-extender type flag) it copies 0x2B bytes from the real DTA (AH=2Fh) into buf.
- **Evidence:** 0x91B73 mov ah,1A / int 21; 0x91B79 mov ah,4E / int 21; call 0x9E0C0 (jae -> 0, else error code); 0x91BAA rep movsb with ecx=0x2B
- **Use in the game:** 0x4F24C: eax="*.1st", edx=0 (normal files), ebx=&local find_t [ebp-0x78]; the result is tested with test eax,eax. The other callers (0x4F3A3, 0x4F504, 0x4FB10, ...) are similar (save-game / PBEM file scans).

## 0x91B8B _dos_findnext (Watcom C runtime) — D, check: confirmed (D)

- **Signature:** `unsigned _dos_findnext(struct find_t *buf /*eax*/); Watcom register convention`
- **Semantics:** Sets the DTA to buf (AH=1Ah). If [0xD5D2A]==9, copies 0x2B bytes from buf into the real DTA first (0x91BCD). Then INT 21h AH=4Fh, then the result is copied back (0x91BAA). Returns 0, or the DOS error code with errno set.
- **Evidence:** 0x91B8E mov ah,1A; call 0x91BCD; mov ah,4F; int 21; call 0x9E0C0; call 0x91BAA
- **Use in the game:** 0x4FC83: eax=&find_t [ebp-0x78] in the loop after _dos_findfirst; test eax,eax ends the loop. 0x504A7 and 0x51041 are the same.

## 0x91BF0 strnicmp (Watcom C runtime) — D, check: confirmed (D)

- **Signature:** `int strnicmp(const char *s1 /*eax*/, const char *s2 /*edx*/, size_t n /*ebx*/); Watcom register convention`
- **Semantics:** For up to n characters: maps 'A'..'Z' to lower case in both strings, and if they differ returns (int)(unsigned char)c1 - (int)(unsigned char)c2. Returns 0 if they are equal and the character is NUL, or when n runs out (n==0 returns 0 straight away).
- **Evidence:** 0x91BF4 test ebx,ebx -> 0; cmp cl,41h/5Ah add 20h; sub edx,eax returns the difference; 0x91C3E test ch,ch -> 0; dec ebx loop
- **Use in the game:** 0x4F338: strnicmp((char*)0xEE248, table[i] (dword [4*i+0xC7664]), 4) for i<=9. 0x4FB2A and 0x4FB44 are similar compares of file names / extensions.

## 0x91C47 rewind (Watcom C runtime) — D, check: confirmed (D)

- **Signature:** `void rewind(FILE *fp /*eax*/); Watcom register convention`
- **Semantics:** Clears the EOF (0x10) and error (0x20) bits of fp->_flag (byte [fp+0xC] &= 0xCF), then calls fseek(fp, 0, SEEK_SET) (0x91D94 with edx=0, ebx=0).
- **Evidence:** 0x91C49 mov dl,[eax+0xC]; and dl,0xCF; xor ebx,ebx; xor edx,edx; call 0x91D94
- **Use in the game:** 0x4FBD0: rewind(fp) after a header check against "Advanced Civilization" (0x7D610). Also called at 0x4FD3A and 0x5535F.

## 0x91C5E rename (Watcom C runtime) — D, check: confirmed (D)

- **Signature:** `int rename(const char *old /*eax*/, const char *new /*edx*/); Watcom register convention`
- **Semantics:** INT 21h AH=56h with ds:edx=old and es:edi=new (es=ds). Returns 0 on success. On failure it sets errno from the DOS code (0x9E0DF) and returns -1.
- **Evidence:** 0x91C60 mov edi,edx; mov edx,eax; es=ds; mov ah,56; int 21; rcl/ror carry into the sign bit; 0x9E0DF returns -1
- **Use in the game:** 0x4FC3E: rename(local name [ebp-0x34], (char*)0xF5230) after stricmp(0xF5230, local) != 0. Also at 0x4FDB4.

## 0x91C88 mkdir (Watcom C runtime) — D, check: confirmed (D)

- **Signature:** `int mkdir(const char *path /*eax*/); Watcom register convention`
- **Semantics:** INT 21h AH=39h. Returns 0, or -1 with errno set.
- **Evidence:** 0x91C8B mov ah,39; int 21; the error path calls 0x9E0DF
- **Use in the game:** 0x4FE48: mkdir("PBEM_OUT"), result ignored. Also called at 0x50085, 0x50580 and 0x5385D.

## 0x91CA7 remove (unlink) (Watcom C runtime) — D, check: confirmed (D)

- **Signature:** `int remove(const char *path /*eax*/); Watcom register convention`
- **Semantics:** Jumps to 0x9E145: INT 21h AH=41h (delete file) with edx=path. Returns 0, or -1 with errno set.
- **Evidence:** 0x91CA7 jmp 0x9E145; 0x9E148 mov ah,41; int 21; 0x9E159 call 0x9E0DF
- **Use in the game:** 0x508B3: remove((char*)0xEE220) after closing a file (0x7D95E). Also at 0x508CB/0x508D5, 0x50EC7 and 0x53415 (deleting temporary or PBEM files).

## 0x91CAC realloc (Watcom C runtime) — D, check: confirmed (D)

- **Signature:** `void *realloc(void *ptr /*eax*/, size_t size /*edx*/); Watcom register convention`
- **Semantics:** ptr==NULL: returns malloc(size) (0x822B5). size==0: free(ptr) (0x877E6), returns NULL. Otherwise: old=_msize(ptr) (0x9E164: header dword at ptr-4 with bit 0 cleared, minus 4), then tries _expand(ptr,size) in place (0x9E16D); if that succeeds it returns ptr. If it fails and [0xD5D2A]==1, it tries 0x95A4C(ptr-4, size+4) (heap-segment grow) and returns the result+4 if nonzero. Otherwise: p=malloc(size); if p, copies old bytes (rep movsd + movsb), frees ptr and returns p; if malloc fails, calls _expand(ptr, old) to restore the block and returns NULL.
- **Evidence:** 0x91CBA..0x91D49; calls 0x822B5, 0x877E6, 0x9E164, 0x9E16D, 0x95A4C
- **Use in the game:** 0x56090: [0xF09B4] = realloc([0xF09B4], ([0xF09B6]>>16)*10). Also at 0x57045.

## 0x91D94 fseek (Watcom C runtime) — D, check: corrected (D)

- **Signature:** `int fseek(FILE *fp /*eax*/, long offset /*edx*/, int whence /*ebx*/); Watcom register convention`
- **Semantics:** Watcom FILE layout: _ptr +0, _cnt +4, _base +8, _flag +0xC, _handle +0x10. If _flag & (_WRITE|_UNGET 0x6) is set: when _DIRTY (0x1000) is set it flushes (0x94AC2), and a flush failure returns -1 (errno 9 if whence==0 and offset<0); otherwise for SEEK_CUR it subtracts _cnt from offset and empties the buffer; then it clears flag bits 0x14 and calls lseek (0x93E23, INT 21h AH=42h). Read-only stream: SEEK_SET first tries to move inside the buffer (0x91D52: offset - (current position = tell - _cnt)); SEEK_CUR uses 0x91D52 with offset and falls back to lseek(handle, offset-_cnt, SEEK_CUR); SEEK_END empties the buffer, clears EOF and calls lseek. Any other whence sets errno 9 and returns -1. Returns 0 on success, -1 on error.
- **Evidence:** 0x91D9B test [eax+0xC],6; 0x91DA1 test [ecx+0xD],0x10; 0x91DFC whence switch; 0x93E23 = lseek (mov ah,42; int 21); 0x9E397 = tell (lseek(h,0,SEEK_CUR))
- **Use in the game:** 0x5E898: fseek(fp [ebp-0x14], 0, SEEK_CUR) between a read and a write (the usual stdio switch). 0x7D89C: another caller. 0x91C56: called by rewind.
- **Check's correction:** The name and signature are right. Three details of the semantics need fixing. (1) Write/unget stream, not dirty (0x91DCD-0x91DDF): SEEK_CUR subtracts _cnt from the offset, but the buffer is emptied (_ptr=_base, _cnt=0) for every whence, not only SEEK_CUR. (2) In the write/unget path whence is never checked. A bad whence goes straight to lseek 0x93E23, so errno comes from the DOS error through 0x9E0DF, not errno 9. errno 9 at 0x91DBA is set only in the read-only path (0x91DFC-0x91E0E), or when a flush fails with whence==0 and offset<0. (3) Read-only SEEK_SET (0x91E35): position = tell(h) (0x9E397) - _cnt. 0x91D52(offset-position) moves inside the buffer. If that fails, it calls lseek(handle, offset, SEEK_SET) (0x91E53, ebx still 0). 0x91D52 always clears _EOF (and [edx+0xC],0xEF). Call sites: 0x5E898 is fseek(fp,0,SEEK_CUR) after fread 0x7DB6D. 0x7D89C is fseek(fp,0,SEEK_END) in fopen for mode 'a' (cmp dl,'a' at 0x7D88E).

## 0x91E9E fg_mousevis (Fastgraph) — D, check: confirmed (D)

- **Signature:** `int fg_mousevis(int state); cdecl stack, caller pops`
- **Semantics:** Software mouse cursor show (state&1 = 1) or hide (0). Does nothing (returns 0) if the mouse is not initialised ([0xD5FD6]==0) or if state&1 already equals the current visibility byte [0xD5FD7]. Otherwise it dispatches on [0xD5F88] through the table at 0x9238E. With a virtual buffer open (index 0x20) it re-dispatches on the saved screen mode index [0xD65FA], so it always acts on the screen, never on the virtual buffer. Mode 25 (index 0x64) goes to 0x91FDF, with interrupts disabled (cli) and [0xD5FD7]=0 while it works. HIDE: copies the saved 16x16 background at 0xCB3AA back to screen offset [0xD5FCC] (bank [0xD65D9]), 16 bytes per row, pitch [0xDA100], with bank switching. SHOW: INT 33h AX=3 gives cx,dx, stored to [0xD5FD8]/[0xD5FDC]; x=(cx-[0xD5FE0])>>[0xD5FE8] and y=dx-[0xD5FE4] (each clamped to >=0); offset=y*pitch+x with its bank, stored to [0xD5FCC]/[0xD65D9]; saves the 16x16 block under the cursor into 0xCB3AA (no clipping at the right or bottom edge: rows always copy 16 bytes); then sets [0xD5FD8]=-1 and calls the cursor-draw routine [0xD5FD0] with ecx=mouse x, edx=mouse y. At the end it sets [0xD5FD7]=state, then sti. Returns 0.
- **Evidence:** 0x91EA6 cmp [0xD5FD6],0; 0x91EB5 cmp al,[0xD5FD7]; jmp [ebx+0x9238E]; table entry 0x923AE (index 0x20) -> 0x91ECA (uses [0xD65FA]); 0x923F2 (index 0x64) -> 0x91FDF; 0x92011.. restore loop; 0x9209A int 33h fn 3; 0x920F7 store [0xD5FCC]; 0x92360 store state; 0x9237F call [0xD5FD0]
- **Use in the game:** 0x7889B: fg_mousevis(0) (hide) inside a game wrapper (0x78884) used before screen drawing, e.g. at 0x79BAE before an animation. 0x788C2/0x788EE/0x7891A are the same wrapper family (show/hide).

## 0x92406 int386x (Watcom C runtime) — D, check: corrected (D)

- **Signature:** `int int386x(int intno /*eax*/, union REGS *in /*edx*/, union REGS *out /*ebx*/, struct SREGS *sr /*ecx*/); Watcom register convention`
- **Semantics:** Loads the registers from in and sr and runs software interrupt intno (0x9E3E0, not traced). Then stores eax, ebx, ecx, edx, esi, edi and cflag (sbb: -1 if CF else 0) to out at +0,+4,+8,+0xC,+0x10,+0x14,+0x18, and es/ds to sr at +0/+6. Returns out->x.eax.
- **Evidence:** 0x92409..0x92413 moves the args (esi=intno, edi=in, edx=out, ebx=sregs) and calls 0x9E3A8; 0x9E3BC..0x9E3CF store the REGS fields; 0x9E3D6/0x9E3DA store ds/es; 0x92418 mov eax,[out]
- **Use in the game:** 0x78B8D: int386x(0x31, &r, &r, &sr), a DPMI call with sregs.ds=ds. 0x93F88: an internal runtime use.
- **Check's correction:** The name int386x(intno eax, in edx, out ebx, sregs ecx) and the return value out->x.eax are right (0x92418). 0x9E3E0 can be traced: it pushes 0x9E486+3*intno as the return address, loads es=sregs[+0], ds=sregs[+6] and the registers from in, then 'returns' into a stub table of 'int n; ret' entries (obj1 bytes at 0x9E486+3*0x21 = CD 21 C3, +3*0x31 = CD 31 C3). The callers_note is wrong. At 0x78B44 the SREGS at [ebp-0x18] is zeroed by 0x7D5D0 (memset, 12 bytes), then only sregs.es (offset +0) is set to ds (0x78B74); sregs.ds stays 0. REGS.eax=0x500 (0x78B57) and REGS.edi=0x1125A8 ([ebp-0x20] = [ebp-0x34]+0x14). This is DPMI 0500h 'get free memory information' into es:edi=0x1125A8. The function then returns [0x1125BC] (info +0x14) << 12, i.e. pages to bytes. The out ds/es written back come from the post-interrupt ds/es (0x9E3D6/0x9E3DA).

## 0x92420 _dos_read (Watcom C runtime) — D, check: confirmed (D)

- **Signature:** `unsigned _dos_read(int handle /*eax*/, void __far *buf /*offset ebx, segment ecx*/, unsigned count /*edx*/, unsigned *bytes /*stack; callee pops (ret 4)*/)`
- **Semantics:** INT 21h AH=3Fh: bx=handle, ds:edx=buf, ecx=count. On success *bytes = bytes read and it returns 0. On CF it returns the DOS error code with errno set (via 0x9E0C0) and *bytes is not written. 0x9244A is the twin _dos_write (AH=40h).
- **Evidence:** 0x92420 push 0x3F00; mov ds,ecx; ecx=edx; edx=ebx; ebx=eax; int 21; jb skip; mov ebx,[esp+0xC]; mov [ebx],eax; call 0x9E0C0; ret 4
- **Use in the game:** 0x79B24: _dos_read([0xF40E0] (the ACIV.CT4 handle), ds:&local[ebp-0xB4], 0x82, &n). This reads the FLC header from the catalog entry the preceding 0xA24A7 call positioned the file at. 0x79CB9/0x79CE7/0x79D04/0x7A114 are similar.

## 0x92451 fg_flicsize (Fastgraph/Image) — D, check: confirmed (D)

- **Signature:** `int fg_flicsize(void *header, int *width, int *height); cdecl stack`
- **Semantics:** If the word at header+4 is 0xAF11 (FLI) or 0xAF12 (FLC): *width = word at header+8, *height = word at header+0xA. Otherwise *width = *height = -1. Always returns 0.
- **Evidence:** 0x92458 cmp ax,0AF11h/0AF12h; movzx [esi+8], [esi+0xA]; 0x9247F the -1 path; 0x9247A xor eax,eax
- **Use in the game:** 0x79B64: fg_flicsize(&hdr[ebp-0xB4], &w, &h). Then the game centres the animation: x=(0x270-w)/2+8, y=(0x13F-h)/2+6, fg_move(x,y). Also at 0x7A306.

## 0x92486 fg_showflic (Fastgraph/Image) — D, check: corrected (D)

- **Signature:** `int fg_showflic(char *filename, int count, int flags); cdecl stack`
- **Semantics:** Requires a 256-colour mode ([0xD9F8C]==0x100), otherwise returns 0 doing nothing. FLAGS: bit 0 = no frame delay; bit 1 = top-left at the graphics cursor (else (0,0)); bit 2 = the file is already in the fg_imagebuf buffer (far ptr [0xD668A], size [0xD6690]; the header is at its start, data from +0x80; filename ignored), else open filename read-only, read the 128-byte header into the image buffer and stream the rest through it in [0xD6690]-byte reads; bit 4 = ignore palette chunks. HEADER: magic word +4 must be 0xAF11 or 0xAF12, else returns 2 (1 if the file cannot be opened). frames=word +6, w=word +8, h=word +0xA. Delay in BIOS ticks: AF11 speed(word +0x10)*1429/5500 (16-bit mul/div); AF12 speed(dword +0x10 ms)/55. The +0x50 frame-1 offset is ignored: frames are read one after another from +0x80. FRAME LOOP: reads the frame size dword and type word. 0xF100 (prefix) is skipped but still counts as a frame. 0xF1FA reads the chunk count, skips 8 bytes, and handles each chunk (size dword, type word): 4 COLOR_256 (packets: skip byte added to the index, count byte with 0=256, RGB bytes >>2 to DAC 3C8h/3C9h, waits for vertical retrace start before each packet if [0xD5F9A]!=0); 11 COLOR_64 (same but values not shifted); 7 SS2/DELTA_FLC; 12 LC/DELTA_FLI; 13 BLACK (w x h of colour 0); 15 BYTE_RUN; 16 COPY (w x h raw bytes). Types 0-0x10 not listed are skipped. Any other frame type ends the playback (returns 0, file not closed). Once all chunks are done it skips the leftover frame bytes. KEYS: after each frame, INT 16h: if a key is waiting it is read; Esc stops playback (closes the file, returns 0); other keys are discarded. DELAY: unless bit 0, busy-waits until the 16-bit BIOS tick counter (INT 1Ah) has advanced by >= delay since the frame finished. After `frames` frames (the ring frame is never shown): if count!=0 it decrements count and stops at 0; otherwise (count 0 = endless until Esc) it restarts at offset 0x80. End of data (a 0xFFFF size word) also ends the loop. OUTPUT (mode 25, through table 0x93375 index 0x64 -> 0x93359): each decoded run is written to screen row y0+row at x0+x, with bank switching. The only clipping: a row is skipped if y >= [0xDA36C] (screen height). There is no left, right or top clipping and fg_setclip is ignored. The virtual-buffer entry (index 0x20) of that table is 0, so it is not usable with a virtual buffer open. Return 0 = played.
- **Evidence:** 0x9249A cmp [0xD9F8C],100h; 0x924D1 flag 4; 0x9251E magic test; 0x9252C/0x92544 delay conversion; 0x92598 flag 2 -> cursor; 0x926A8 0xF100/0xF1FA; 0x92805 chunk table 0x9313A (4->0x9291A, 7->0x92A31, 11->0x92C1F, 12->0x92D29, 13->0x92EB8, 15->0x92EF1, 16->0x92FD9); 0x9291A/0x92C1F test flags,0x10; 0x9306B int 16h Esc; 0x93086 flag 1; 0x93094 int 1Ah loop; 0x930CC count; 0x930E8 rewind to 0x80; 0x9325A row clip cmp [0xDA36C]
- **Use in the game:** 0x79C13: fg_showflic("" (0xB95A7), 1, 6) after fg_imagebuf(buf,size) (0x8240F), with the whole FLC loaded from ACIV.CT4 by 0xA25B3 and fg_move to the centred position: play once, at the cursor, from memory, with timing. 0x7A3E2: the same, fg_showflic(0xB9613, 1, 6).
- **Check's correction:** The name, flags, delay, chunk table, output path and bottom-only clip are confirmed (D). Fix these points. (1) Palette retrace wait: in COLOR_256 (0x9291A) the 3DAh bit-3 wait (0x92972) runs once per chunk, before the first packet. The packet loop jumps back to 0x92979, after the wait (0x92A27). It also waits only until bit 3 is set (in retrace); it does not wait for the start of a new retrace. COLOR_64 (0x92C68/0x92C73) has the same structure. (2) 'End of data (a 0xFFFF size word)' is wrong. Each byte read zero-extends, so 'cmp eax,-1' at 0x92627 matches only when the refill 0x9347D returns 0 and the following dec makes eax=-1. A literal 0xFFFF word gives 0x0000FFFF and is treated as a size. The refill returns 0 at end of file, and always in memory mode (flag 4), because the handle [ebp-0x3a] is 0 there, so running past the fg_imagebuf size counts as end of data. The jump goes to 0x925E6 -> 0x930CC, the end-of-pass logic, not out of the player. With count!=0 it decrements count and stops at 0; with count==0 it seeks back to 0x80 (or +0x80 in memory) and plays again. (3) Unknown chunk types above 0x10 are skipped too (0x92809 ja -> 0x92817 -> 0x9289D), not only the unlisted ones in 0-0x10. (4) A failed header read (CF at 0x924FF) also returns 2. A short header read is not detected. (5) Quirk: in memory mode a bad magic jumps to 0x92501, which closes handle ebx if it is nonzero. On that path ebx is the caller's leftover register, never set by fg_showflic; at 0x79C13 it holds [0xF40E0] (the CT4 handle) or [0xF6278], so a corrupt in-memory FLC could close an unrelated DOS handle. (6) A 0xF100 prefix frame also takes the key check and the delay, because it ends at 0x9306B.

## 0x9670A AIL_background (other (Miles AIL 3.03)) — D, check: corrected (D)

- **Signature:** `int AIL_background(void); no args`
- **Semantics:** Returns the dword [0xDA90A], the nesting count of the AIL timer interrupt handler: it is incremented on entry to the ISR (0x95F96), decremented on exit (0x96049/0x9607D) and zeroed at init (0x96178). Nonzero means it was called from inside the timer callback.
- **Evidence:** 0x9670D mov eax,[0xDA90A]; the ISR fn_00095F6E tests cs:[0xDA90A] and does inc/dec around the callback
- **Use in the game:** 0x827A9/0x827FC (AIL_shutdown wrapper) and other AIL API wrappers: if debug tracing is on and !AIL_background(), print "AIL_shutdown()\n" to the log. Not called by game logic.
- **Check's correction:** The name AIL_background and the return of dword [0xDA90A] are right. It is a 0/1 'inside timer ISR' flag, not a nesting count. The ISR 0x95F6E tests cs:[0xDA90A] first and, if it is nonzero, branches to 0x9608D without incrementing (ja at 0x95F76), so it never goes above 1. inc at 0x95F96, dec at 0x96049/0x9607D, zeroed at 0x96178. The caller 0x827A9 inside AIL_shutdown 0x82784 is confirmed.

## 0xA22C3 cat_open (catalog open) (catalog) — D, check: confirmed (D)

- **Signature:** `int cat_open(const char *path); cdecl stack`
- **Semantics:** Opens path read-only (INT 21h 3D00h) and reads the first 0x318-byte directory block (33 entries x 24 bytes) into 0xDA446. Sets the directory cache: [0xDA770]=handle, [0xDA774]=1. Returns the DOS handle. Returns 0 if the open fails, or if the read fails (the file is then closed). The file position is left at 0x318.
- **Evidence:** 0xA22C7 mov ah,3D / al=0; 0xA22D5 ecx=0x318, edx=0xDA446, ah=3F; 0xA22E8 stores [0xDA770]; 0xA22ED [0xDA774]=1; 0xA22FB xor eax
- **Use in the game:** Start-up 0x1086F/0x1089C/0x108C9: cat_open("aciv.ct1"/"aciv.ct2"/"aciv.ct3") -> [0xF40D4]/[0xF40D8]/[0xF40DC]; 0x10911: "%saciv.ct4" with a path prefix -> [0xF40E0]; 0 means fatal "Can't open catalog file".

## 0xA22FF cat_show (display a catalog image or animation) (catalog) — D, check: confirmed (D)

- **Signature:** `int cat_show(const char *name, int flags, int handle); cdecl stack`
- **Semantics:** DIRECTORY LOOKUP (shared with 0xA24A7/0xA25B3): the name is copied into a zeroed 12-byte key at 0xDA75E with 'a'-'z' upper-cased, NUL included (no length check: names of 12+ characters overflow into 0xDA76A). If the cached block is not this handle's first block ([0xDA770]!=handle or [0xDA774]!=1), it seeks to 0 and rereads 0x318 bytes (cache=handle, valid=1). Entries are 24 bytes: name[12] +0, file offset dword +0xC, size dword +0x10, type byte +0x14, word +0x16. Scan: an entry whose first byte is 0xFF is a link: its +0xC dword is the offset of the next directory block (0 means not found, return -2); otherwise it seeks there, reads 0x318 bytes and sets valid=0. Match = all 12 bytes equal. DOS errors return -1. FOUND: seeks to the entry offset, [0xDA76C]=size, clears flags bit 2 (data always read from the handle), then dispatches on type. 1 (PCX): no virtual buffer -> 0xA4DC8(handle,flags), else 0xA26CD(handle,flags). 2 (GIF) -> 0xA3D14(handle,flags). 3/4 -> 0xA5936(handle, word+0x16, type-3), returns 0. 5/6 (FLI/FLC) -> 0xA2D29(handle, 1, flags). For types 1, 2, 5, 6 a nonzero result from the decoder becomes -3; any other type returns -3. Returns 0 on success. GIF decoder 0xA3D14 (partly traced): flags bit 1 = place at the graphics cursor, else at the image descriptor's left/top; output is clipped only on the right/bottom, to [0xDA2F0]-1 / [0xDA36C]-1 (width/height of the active screen or virtual buffer), not to fg_setclip; flags bit 0 set = keep the palette (0xA48FA returns at once), clear = load the DAC from the local or global colour table (>>2); the colour table is stored >>2 (0xA47AB for index 0x64).
- **Evidence:** 0xA2309..0xA2331 key build; 0xA233F cache test; 0xA2385 scan with rep cmpsw x6; 0xA239E link 0xFF; 0xA23EB seek; 0xA2400 and [ebp+0xC],0xFFFB; 0xA2410 [0xDA76C]=size; 0xA2412 type dispatch. Parsing the directories of C:\work\advciv_play\ACIV.CT1/CT2 shows every *.GIF is type 2 (data starts 'GIF8'), *.FLC in CT4 is type 6, *.FGF is type 7, HLP/XMI are type 0. 0xA26CD checks byte 0x0A and reads the PCX header, so type 1 = PCX. 0xA3E18.. checks 'g','i','f' (or'd 0x20); 0xA3FD0 flag 2 -> cursor; 0xA4031/0xA4083 right/bottom clip; 0xA48FA test flags,1
- **Use in the game:** 0x115FB: after fg_vbopen([0xF52C8]) and fg_move(0,0): cat_show("minimap.gif", 3, [0xF40D8]=CT2), drawn into the virtual buffer at the cursor with the palette kept. 0x116D4 "map&pal.gif", 0x11931 "maped.gif", 0x11ACA "c_invld.gif", 0x11CAE "tokens.gif" are the same, all flags 3, CT2.

## 0xA24A7 cat_size (look up an entry, return its size and seek to it) (catalog) — D, check: confirmed (D)

- **Signature:** `long cat_size(const char *name, int handle); cdecl stack`
- **Semantics:** Same directory lookup and cache as cat_show (0xA22FF). On a match it seeks the DOS file pointer to the entry's data offset and returns the size (dword +0x10). It does not store [0xDA76C]. Returns -1 on a DOS seek/read error and -2 if not found.
- **Evidence:** 0xA24B1..0xA2585 lookup identical to 0xA22FF with handle at [ebp+0xC]; 0xA2587 seek with cx:dx = +0xE:+0xC; 0xA259C mov eax,[esi+0x10]; 0xA25A5 -1, 0xA25AC -2
- **Use in the game:** 0x79B03: size = cat_size(animname, [0xF40E0]=CT4). The game then reads the header straight from the handle with _dos_read (0x92420), allocates size bytes (0x822B5) and calls cat_load. Also at 0x16005 and 0x2FA21/0x2FB02/0x2FBA3, each followed by 0xA25B3.

## 0xA25B3 cat_load (read a catalog entry into memory) (catalog) — D, check: confirmed (D)

- **Signature:** `int cat_load(const char *name, void *buf, int handle); cdecl stack`
- **Semantics:** Same directory lookup and cache. On a match it seeks to the entry offset and reads size bytes (dword +0x10) into buf with one INT 21h AH=3Fh call. Returns 0 if all size bytes were read, -3 on a read error or short read, -1 on a DOS seek/dir-read error, -2 if not found.
- **Evidence:** 0xA25BD..0xA2689 lookup; 0xA26A0 seek; 0xA26B5 ecx=[esi+0x10], edx=[ebp+0xC], ah=3F; 0xA26C1 cmp eax,ecx; 0xA2699 -3
- **Use in the game:** 0x79B4D: cat_load(animname, malloc'd buf, CT4), then fg_imagebuf(buf,size) and fg_showflic. 0x16024 and 0x2FA55/0x2FB4A/0x2FBEB load data/help entries the same way into a buffer sized by cat_size.

## Open questions (identify agents)

1) fg_gifpal: if a GIF has both a global and a local colour table, the copy still comes from the global table at 0xD7694 with the local table's length. Extension blocks (GIF89a) before the image descriptor are not skipped, so byte 9 of an extension would be read as the local-table flag. This only matters if the game's GIF/pal files have such blocks; check pal.pal and the files at 0x3CE76/0x3D018. 2) fg_vbcopy does no clipping and no size checks; game call sites appear to stay in range (0..639, 0..479), but not all 110 were checked. 3) The hook at [0xDB44C] that longjmp calls was not traced (probably a stack-unwind/stack-check reset). 4) 0xD5F66 vs 0xD5F61: in 256-colour modes both hold the same value. Their separate meaning (virtual colour vs hardware colour) only differs in 16-colour/EGA modes; I named them by the general Fastgraph meaning and did not verify which one each drawing routine reads. 5) 0x878D6's FILE* target [0xF52D8] is the game log; the fopen that sets it was not traced. Side finds: fg_setmode = 0x80A33, fg_testmode = 0x8063C, fg_vbdefine(addr,width,height) = 0x82398 (returns the first free handle, or -1), fg_setclip(minx,maxx,miny,maxy) = 0x91260, fg_vbclose = 0x87D73, malloc = the routine containing 0x822D5 (graded G, entry not confirmed).

## Open questions (identify agents)

1) The Fastgraph names (fg_vbclose, fg_mousefin, fg_intkey, fg_flichead, fg_flicopen, fg_flicplay) rest on body behaviour, arity and pairing (vbopen 0x87C31, imagebuf 0x8240F, mouseini 0x814BD). The behaviour was all read in the code; I did not check the names against a symbol source. The meaning of fg_flicplay flag bit 4 (0x10 = skip palette chunks) is as observed, not checked against the Fastgraph manual.
2) 0x8063C(25,1) at start-up was not traced. It is taken to be fg_testmode/fg_bestmode-style; the evidence that mode 25 = 640x480x256 is the jump-table index and the error string. I also did not trace where fg_setmode(25) itself is called.
3) [0xD5F9A] is treated as Fastgraph's wait-for-retrace flag (set to 1 at mode init, 0x80D40; it gates the 3DAh wait in the palette chunks and in fg_setdacs 0x879EF). Its setter API was not identified.
4) [0xDA100] is taken to be bytes per scanline (640 in mode 25) and [0xD65DA] the bank base of the active page; neither was traced to where it is set.
5) In memory mode the game also calls fg_flicopen, which leaves a DOS handle open; I did not check whether the game closes it later (fg_flicdone is not in this list).
6) The BLACK chunk's row clip uses a stale y (ebp-0x72). It is irrelevant for full-screen 640x480 intros but is not pixel-defined in general.
7) The size of the line buffer at 0xD7694 (shared with the 128-byte header read) was not checked; it must be at least the FLIC width (640).

## Open questions (identify agents)

Mode index: [0xD5F88] = mode*4 (fg_setmode 0x80A33: `shl eax,2`). The game calls fg_setmode(0x19 = 25, 640x480x256) at 0x109F3 after fg_testmode(25,1) at 0x109BC (0x8063C, G), so index 25 is the screen path. fg_vbopen (0x87C31, D) sets [0xD5F88] = 0x20 (index 8) while a virtual buffer is active and saves the screen mode in [0xD65FA]; fg_vbclose (0x87D73, D) restores it. The game draws mostly into vbs (handles in 0xF52C8 / 0xF52CC; a 640x480 buffer allocated at 0x10A3D), so for every dispatched routine I gave both the index-8 (vb) path and the index-25 (banked SVGA) path; in this game they behave the same.

Globals confirmed: graphics cursor x/y 0xD5F9C/0xD5FA0 (fg_move 0x87DF7, D); clip minx/maxx/miny/maxy 0xD5FAC/0xD5FB0/0xD5FB4/0xD5FB8; colour byte 0xD5F61 (fg_setcolor 0x87AC2, so that one is now D for mode 25); vb tables per handle (0..31): base 0xCB196, width 0xCB296, height 0xCB216, clip 0xCB096/0xCAF96/0xCB116/0xCB016; active vb base 0xD6606, width 0xD65FE, handle 0xD65F8 (-1 = none).

Image format: fg_getimage / fg_clpimage / fg_flpimage all store rows bottom first, anchored at the lower-left corner (the cursor). The game's sprite helpers 0x3AE90 / 0x3AF8C / 0x3AFF4 call fg_move(x, y+h-1), so their (x, y) is the top-left. Colour 0 is always transparent in clpimage/flpimage, so a pixel of colour 0 captured by getimage is not redrawn.

Not decoded, seen next to these calls (worth a later pass): 0x8A085 (6 args, used after vb work, for example (0, 639, 0, 0x144, 0, 0x144) at 0x3D928; probably fg_vbpaste, G); 0x87A1A (8 args at 0x3D5F9; perhaps fg_vbcopy, G); 0x8C76E = fg_box(minx, maxx, miny, maxy) using fg_boxdepth (body read only for its use of 0xD5FA4/A8; G otherwise); 0x9D21A = the rect fill fg_box uses (G); 0x7D8C8 = fopen, 0x7D95E = fclose, 0x9E01C = fputc, 0x94956 = errno setter, 0x94222 = low-level write, 0x94AC2 = flush (all G, from fwrite's use); 0x788AC/0x78884 bracket the vb grabs (probably mouse hide/show, G).

0x9174F's SVGA display-start routine (pointer at 0xD65BA, set by chipset init) was not traced. The game calls fg_setvpage only once, with 0 at exit, so it does not matter for the port.

## Open questions (identify agents)

1) Mode paths. Start-up calls fg_setmode(0x19) = mode 25, 640x480x256 SVGA. fg_setmode is 0x80A33 and sets [0xD5F88]=mode*4, so the mode index is 0x64. fg_vbopen (0x87C31) sets [0xD5F88]=0x20 and saves the screen index in [0xD65FA]; fg_vbclose (0x87D73) restores it. So the game's drawing uses two Fastgraph paths: index 0x64 (the banked screen) and index 0x20 (the virtual buffer, linear at [0xD6606], width [0xD65FE]). I followed both for fg_point. fg_mousevis always works on the screen. fg_showflic has no virtual-buffer entry (its table slot is 0), so the game must close the virtual buffer before playing an animation. I did not check that.
2) fg_showflic and the GIF decoder. In fg_showflic I traced the overall flow, the chunk dispatch, the palette chunks, BLACK, BRUN, COPY and LC, but not every byte of the SS2 packet loop (0x92AEF-0x92C07). It looks like standard FLC SS2 (opcodes 11 = skip lines, 10 = last byte, else packet count). Inside the GIF decoder (0xA3D14) I traced positioning, clipping and the palette flag, but not the LZW/interlace code. These parts can be implemented as standard FLC and GIF.
3) BLACK chunk quirk. The BLACK handler (0x92EB8) neither sets nor increments the row y used by the bottom-clip check ([ebp-0x6e]). Its clipping therefore uses a stale y. This is harmless at 640x480.
4) Prefix chunk quirk. A 0xF100 prefix chunk counts as a frame, which is a quirk of this fg_showflic.
5) Names related to these routines. I identified these from their bodies but they are outside the list:
- 0x8240F = fg_imagebuf(buf,size): size 0 selects the internal 4096-byte buffer at 0xD6694.
- 0x9125D = fg_setclip(minx,maxx,miny,maxy): also saves the clip box per virtual buffer.
- 0x8C803 = fg_dashrel.
- 0x90FC7 = fg_point.
- 0x80A33 = fg_setmode.
- 0x87C31 = fg_vbopen (G as a name; its body is clear).
- 0x822B5, listed in names.txt as "tagged_alloc? (G)", is really Watcom malloc. Realloc calls it with only eax=size, and it rounds the size up to 4 with a minimum of 12, returns NULL for 0 or more than -0x2C, and walks the heap list at 0xCB6AC. The extra edx/ebx values its game callers pass are leftover registers.
- 0x877E6 = free.
6) Unresolved.
- I could not confirm the role of [0xD5D2A] (values 1 and 9 select DOS-extender-specific paths in _dos_find* and realloc).
- I did not identify the type 3/4 handler (0xA5936) or the FLC-from-handle handler (0xA2D29) behind cat_show; no catalog entries of type 3/4 were seen.
- In fg_mousevis, the names of [0xD5FE0], [0xD5FE4] and [0xD5FE8] (hotspot or offset, and the x scale shift) are inferred.

## Open questions (identify agents)

How the start-up mode was established: start-up calls 0x8063C(25, 1) (G: fg_testmode) and then 0x80A33(25) (G: fg_setmode), at 0x109B8-0x109F3. Every dispatch therefore indexes [0xD5F88] = 25*4 = 0x64, or 0x20 while fg_vbopen (0x87C31, D) has a virtual buffer open; fg_vbclose (0x87D73, D) restores mode*4. All of the above follows those two paths.

Confirmed in passing:
- fg_setcolor 0x87AC2 is now D: in 256-colour modes and VBs, [0xD5F61]=[0xD5F66]=colour byte and [0xD5F62]=colour*0x01010101.
- fg_move 0x87DF7 is D: it stores [0xD5F9C] and [0xD5FA0].
- Clip globals are D: minx [0xD5FAC], maxx [0xD5FB0], miny [0xD5FB4], maxy [0xD5FB8].
- Also identified: fg_clprect 0x9D21A (D), fg_boxdepth 0x912BE (D), fg_dash 0x91AB3 (D), fg_point 0x90FC7 (D, clipped).

Still open:
1. fg_showgif with interlaced GIFs: rows are counted in decode order against the bottom clip, so a clipped interlaced image would lose the wrong rows. Whether any shipped GIF is interlaced is unchecked.
2. fg_makegif's palette source and pixel reader for mode 25 and VBs (tables 0x8D5C5 etc.) were not traced. This is irrelevant if the port treats junk/junk2 as an in-memory save/restore.
3. The name fgf_width for 0x8A7DD is inferred (G); its behaviour is D. It sits outside the Fonts block 0x8DFEE-0x8E444 but uses the same globals.
4. qsort is unstable. Port this exact algorithm wherever a comparator can return 0 and the order matters (callers 0x13DDA, 0x34CE6, 0x35044, 0x36BC9).
5. In fg_mousepos, the shifts [0xD5FE8]/[0xD5FE9] are assumed to be 0 in mode 25 (not checked in the fg_setmode tables).
6. 0x78884 / 0x788AC around the fg_makegif calls are probably mouse hide/show (G).
