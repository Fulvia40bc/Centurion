
tools/CENTURIO.unpacked.bin:     file format binary


Disassembly of section .data:

00000000 <.data>:
       0:	f8                   	clc
       1:	50                   	push   %ax
       2:	53                   	push   %bx
       3:	51                   	push   %cx
       4:	52                   	push   %dx
       5:	55                   	push   %bp
       6:	57                   	push   %di
       7:	56                   	push   %si
       8:	1e                   	push   %ds
       9:	06                   	push   %es
       a:	b8 63 0e             	mov    $0xe63,%ax
       d:	8e d8                	mov    %ax,%ds
       f:	ff 06 42 00          	incw   0x42
      13:	75 04                	jne    0x19
      15:	ff 06 44 00          	incw   0x44
      19:	fe 0e 8a 00          	decb   0x8a
      1d:	75 07                	jne    0x26
      1f:	b0 04                	mov    $0x4,%al
      21:	a2 8a 00             	mov    %al,0x8a
      24:	cd 64                	int    $0x64
      26:	a0 ec 3e             	mov    0x3eec,%al
      29:	0a c0                	or     %al,%al
      2b:	74 27                	je     0x54
      2d:	a0 e9 3e             	mov    0x3ee9,%al
      30:	3a 06 8b 00          	cmp    0x8b,%al
      34:	74 18                	je     0x4e
      36:	a2 8b 00             	mov    %al,0x8b
      39:	a0 e9 3e             	mov    0x3ee9,%al
      3c:	0a c0                	or     %al,%al
      3e:	74 08                	je     0x48
      40:	b4 04                	mov    $0x4,%ah
      42:	ff 1e c4 00          	lcall  *0xc4
      46:	eb 06                	jmp    0x4e
      48:	b4 03                	mov    $0x3,%ah
      4a:	ff 1e c4 00          	lcall  *0xc4
      4e:	b4 00                	mov    $0x0,%ah
      50:	ff 1e c4 00          	lcall  *0xc4
      54:	b0 20                	mov    $0x20,%al
      56:	e6 20                	out    %al,$0x20
      58:	07                   	pop    %es
      59:	1f                   	pop    %ds
      5a:	5e                   	pop    %si
      5b:	5f                   	pop    %di
      5c:	5d                   	pop    %bp
      5d:	5a                   	pop    %dx
      5e:	59                   	pop    %cx
      5f:	5b                   	pop    %bx
      60:	58                   	pop    %ax
      61:	cf                   	iret
      62:	53                   	push   %bx
      63:	52                   	push   %dx
      64:	a1 86 00             	mov    0x86,%ax
      67:	0b c0                	or     %ax,%ax
      69:	75 21                	jne    0x8c
      6b:	a1 88 00             	mov    0x88,%ax
      6e:	0b c0                	or     %ax,%ax
      70:	75 1a                	jne    0x8c
      72:	b8 08 35             	mov    $0x3508,%ax
      75:	cd 21                	int    $0x21
      77:	89 1e 86 00          	mov    %bx,0x86
      7b:	8c 06 88 00          	mov    %es,0x88
      7f:	1e                   	push   %ds
      80:	8c c0                	mov    %es,%ax
      82:	8e d8                	mov    %ax,%ds
      84:	8b d3                	mov    %bx,%dx
      86:	b8 64 25             	mov    $0x2564,%ax
      89:	cd 21                	int    $0x21
      8b:	1f                   	pop    %ds
      8c:	b8 08 25             	mov    $0x2508,%ax
      8f:	1e                   	push   %ds
      90:	ba 01 00             	mov    $0x1,%dx
      93:	bb 00 00             	mov    $0x0,%bx
      96:	8e db                	mov    %bx,%ds
      98:	cd 21                	int    $0x21
      9a:	1f                   	pop    %ds
      9b:	b0 b6                	mov    $0xb6,%al
      9d:	e6 43                	out    %al,$0x43
      9f:	b8 ff 3f             	mov    $0x3fff,%ax
      a2:	e6 40                	out    %al,$0x40
      a4:	8a c4                	mov    %ah,%al
      a6:	e6 40                	out    %al,$0x40
      a8:	5a                   	pop    %dx
      a9:	5b                   	pop    %bx
      aa:	c3                   	ret
      ab:	52                   	push   %dx
      ac:	51                   	push   %cx
      ad:	a1 86 00             	mov    0x86,%ax
      b0:	0b 06 88 00          	or     0x88,%ax
      b4:	74 11                	je     0xc7
      b6:	b8 08 25             	mov    $0x2508,%ax
      b9:	1e                   	push   %ds
      ba:	8b 16 86 00          	mov    0x86,%dx
      be:	8b 0e 88 00          	mov    0x88,%cx
      c2:	8e d9                	mov    %cx,%ds
      c4:	cd 1f                	int    $0x1f
      c6:	b6 43                	mov    $0x43,%dh
      c8:	ff e6                	jmp    *%si
      ca:	8a e6                	mov    %dh,%ah
      cc:	5a                   	pop    %dx
      cd:	ff e6                	jmp    *%si
      cf:	8a e6                	mov    %dh,%ah
      d1:	5a                   	pop    %dx
      d2:	c3                   	ret
      d3:	51                   	push   %cx
      d4:	59                   	pop    %cx
      d5:	5a                   	pop    %dx
      d6:	c3                   	ret
      d7:	55                   	push   %bp
      d8:	8b ec                	mov    %sp,%bp
      da:	56                   	push   %si
      db:	8b 76 04             	mov    0x4(%bp),%si
      de:	83 e6 0f             	and    $0xf,%si
      e1:	d1 e6                	shl    $1,%si
      e3:	d1 e6                	shl    $1,%si
      e5:	a1 42 00             	mov    0x42,%ax
      e8:	89 84 46 00          	mov    %ax,0x46(%si)
      ec:	a1 44 00             	mov    0x44,%ax
      ef:	89 84 48 00          	mov    %ax,0x48(%si)
      f3:	5e                   	pop    %si
      f4:	5d                   	pop    %bp
      f5:	c2 02 00             	ret    $0x2
      f8:	33 c0                	xor    %ax,%ax
      fa:	50                   	push   %ax
      fb:	e8 d9 ff             	call   0xd7
      fe:	c3                   	ret
      ff:	53                   	push   %bx
     100:	51                   	push   %cx
     101:	56                   	push   %si
     102:	a1 42 00             	mov    0x42,%ax
     105:	8b 1e 44 00          	mov    0x44,%bx
     109:	b9 10 00             	mov    $0x10,%cx
     10c:	33 f6                	xor    %si,%si
     10e:	89 84 46 00          	mov    %ax,0x46(%si)
     112:	89 9c 48 00          	mov    %bx,0x48(%si)
     116:	83 c6 04             	add    $0x4,%si
     119:	e2 f3                	loop   0x10e
     11b:	5e                   	pop    %si
     11c:	59                   	pop    %cx
     11d:	5b                   	pop    %bx
     11e:	c3                   	ret
     11f:	55                   	push   %bp
     120:	8b ec                	mov    %sp,%bp
     122:	52                   	push   %dx
     123:	a1 46 00             	mov    0x46,%ax
     126:	8b 16 48 00          	mov    0x48,%dx
     12a:	03 46 04             	add    0x4(%bp),%ax
     12d:	83 d2 00             	adc    $0x0,%dx
     130:	3b 16 44 00          	cmp    0x44,%dx
     134:	72 08                	jb     0x13e
     136:	77 f8                	ja     0x130
     138:	3b 06 00 04          	cmp    0x400,%ax
     13c:	77 f2                	ja     0x130
     13e:	5a                   	pop    %dx
     13f:	5d                   	pop    %bp
     140:	c2 02 00             	ret    $0x2
     143:	55                   	push   %bp
     144:	8b ec                	mov    %sp,%bp
     146:	53                   	push   %bx
     147:	52                   	push   %dx
     148:	e8 ad ff             	call   0xf8
     14b:	8b 1e 46 8b          	mov    -0x74ba,%bx
     14f:	48                   	dec    %ax
     150:	03 04                	add    (%si),%ax
     152:	d2 3b                	sarb   %cl,(%bp,%di)
     154:	5e                   	pop    %si
     155:	04 d2                	add    $0xd2,%al
     157:	3b 44 72             	cmp    0x72(%si),%ax
     15a:	77 3b                	ja     0x197
     15c:	00 77 1e             	add    %dh,0x1e(%bx)
     15f:	e8 e2 04             	call   0x644
     162:	d2 3e b2 00          	sarb   %cl,0xb2
     166:	00 75 14             	add    %dh,0x14(%di)
     169:	e8 97 34             	call   0x3603
     16c:	3d 00 00             	cmp    $0x0,%ax
     16f:	75 0c                	jne    0x17d
     171:	72 77                	jb     0x1ea
     173:	3b 00                	cmp    (%bx,%si),%ax
     175:	77 e2                	ja     0x159
     177:	3b 1e 00 04          	cmp    0x400,%bx
     17b:	77 dc                	ja     0x159
     17d:	5a                   	pop    %dx
     17e:	5b                   	pop    %bx
     17f:	5d                   	pop    %bp
     180:	02 55 ec             	add    -0x14(%di),%dl
     183:	52                   	push   %dx
     184:	ad                   	lods   %ds:(%si),%ax
     185:	8b 52 56             	mov    0x56(%bp,%si),%dx
     188:	76 83                	jbe    0x10d
     18a:	0f e6                	(bad)
     18c:	e6 42                	out    %al,$0x42
     18e:	89 46 a1             	mov    %ax,-0x5f(%bp)
     191:	00 84 00 5d          	add    %al,0x5d00(%si)
     195:	48                   	dec    %ax
     196:	03 3b                	add    (%bp,%di),%di
     198:	00 2b                	add    %ch,(%bp,%di)
     19a:	94                   	xchg   %ax,%sp
     19b:	04 d2                	add    $0xd2,%al
     19d:	1b 84 46 00          	sbb    0x46(%si),%ax
     1a1:	0b d2                	or     %dx,%dx
     1a3:	74 03                	je     0x1a8
     1a5:	e6 5a                	out    %al,$0x5a
     1a7:	ff 5e 5a             	lcall  *0x5a(%bp)
     1aa:	5d                   	pop    %bp
     1ab:	c2 02 00             	ret    $0x2
     1ae:	33 c0                	xor    %ax,%ax
     1b0:	50                   	push   %ax
     1b1:	e8 cf ff             	call   0x183
     1b4:	c3                   	ret
     1b5:	ec                   	in     (%dx),%al
     1b6:	52                   	push   %dx
     1b7:	ad                   	lods   %ds:(%si),%ax
     1b8:	8b 52 8b             	mov    -0x75(%bp,%si),%dx
     1bb:	46                   	inc    %si
     1bc:	0a ba c8 03          	or     0x3c8(%bp,%si),%bh
     1c0:	ee                   	out    %al,(%dx)
     1c1:	eb 00                	jmp    0x1c3
     1c3:	ba c9 03             	mov    $0x3c9,%dx
     1c6:	8b 46 08             	mov    0x8(%bp),%ax
     1c9:	ee                   	out    %al,(%dx)
     1ca:	eb 00                	jmp    0x1cc
     1cc:	8b 46 06             	mov    0x6(%bp),%ax
     1cf:	ee                   	out    %al,(%dx)
     1d0:	eb 00                	jmp    0x1d2
     1d2:	8b 46 04             	mov    0x4(%bp),%ax
     1d5:	ee                   	out    %al,(%dx)
     1d6:	5a                   	pop    %dx
     1d7:	5d                   	pop    %bp
     1d8:	c2 08 00             	ret    $0x8
     1db:	00 52 ad             	add    %dl,-0x53(%bp,%si)
     1de:	8b 06 53 52          	mov    0x5253,%ax
     1e2:	57                   	push   %di
     1e3:	56                   	push   %si
     1e4:	57                   	push   %di
     1e5:	8b 46 08             	mov    0x8(%bp),%ax
     1e8:	a3 8e 00             	mov    %ax,0x8e
     1eb:	8b 46 06             	mov    0x6(%bp),%ax
     1ee:	a3 8c 00             	mov    %ax,0x8c
     1f1:	8b 5e 0a             	mov    0xa(%bp),%bx
     1f4:	89 1e 94 00          	mov    %bx,0x94
     1f8:	8b 46 04             	mov    0x4(%bp),%ax
     1fb:	a3 96 00             	mov    %ax,0x96
     1fe:	e8 a1 00             	call   0x2a2
     201:	ff 0e 90 00          	decw   0x90
     205:	8c d8                	mov    %ds,%ax
     207:	8e c0                	mov    %ax,%es
     209:	bf 60 b0             	mov    $0xb060,%di
     20c:	b9 f7 07             	mov    $0x7f7,%cx
     20f:	b8 20 20             	mov    $0x2020,%ax
     212:	f3 ab                	rep stos %ax,%es:(%di)
     214:	be ee 0f             	mov    $0xfee,%si
     217:	33 d2                	xor    %dx,%dx
     219:	d1 ea                	shr    $1,%dx
     21b:	f7 c2 00 01          	test   $0x100,%dx
     21f:	75 0a                	jne    0x22b
     221:	e8 66 00             	call   0x28a
     224:	77 28                	ja     0x24e
     226:	0d 00 ff             	or     $0xff00,%ax
     229:	8b d0                	mov    %ax,%dx
     22b:	f7 c2 01 00          	test   $0x1,%dx
     22f:	74 13                	je     0x244
     231:	e8 56 00             	call   0x28a
     234:	77 18                	ja     0x24e
     236:	e8 ae 00             	call   0x2e7
     239:	88 84 60 b0          	mov    %al,-0x4fa0(%si)
     23d:	46                   	inc    %si
     23e:	81 e6 ff 0f          	and    $0xfff,%si
     242:	eb d5                	jmp    0x219
     244:	e8 43 00             	call   0x28a
     247:	77 05                	ja     0x24e
     249:	8b f8                	mov    %ax,%di
     24b:	e8 3c 00             	call   0x28a
     24e:	77 30                	ja     0x280
     250:	8b c8                	mov    %ax,%cx
     252:	25 f0 00             	and    $0xf0,%ax
     255:	d1 e0                	shl    $1,%ax
     257:	d1 d1                	rcl    $1,%cx
     259:	f8                   	clc
     25a:	50                   	push   %ax
     25b:	53                   	push   %bx
     25c:	51                   	push   %cx
     25d:	0b f8                	or     %ax,%di
     25f:	83 e1 0f             	and    $0xf,%cx
     262:	83 c1 03             	add    $0x3,%cx
     265:	8b df                	mov    %di,%bx
     267:	81 e3 ff 0f          	and    $0xfff,%bx
     26b:	8a 87 60 b0          	mov    -0x4fa0(%bx),%al
     26f:	88 60 46             	mov    %ah,0x46(%bx,%si)
     272:	e6 0f                	out    %al,$0xf
     274:	d5 43                	aad    $0x43
     276:	77 8b                	ja     0x203
     278:	e8 6c 00             	call   0x2e7
     27b:	47                   	inc    %di
     27c:	e2 e7                	loop   0x265
     27e:	eb 99                	jmp    0x219
     280:	5f                   	pop    %di
     281:	5e                   	pop    %si
     282:	5a                   	pop    %dx
     283:	59                   	pop    %cx
     284:	5b                   	pop    %bx
     285:	07                   	pop    %es
     286:	5d                   	pop    %bp
     287:	c2 08 00             	ret    $0x8
     28a:	56                   	push   %si
     28b:	ff 06 90 00          	incw   0x90
     28f:	8b 36 90 00          	mov    0x90,%si
     293:	3b 36 92 00          	cmp    0x92,%si
     297:	73 0a                	jae    0x2a3
     299:	8a 60 46             	mov    0x46(%bx,%si),%ah
     29c:	ac                   	lods   %ds:(%si),%al
     29d:	32 e4                	xor    %ah,%ah
     29f:	f8                   	clc
     2a0:	5e                   	pop    %si
     2a1:	c3                   	ret
     2a2:	56                   	push   %si
     2a3:	53                   	push   %bx
     2a4:	52                   	push   %dx
     2a5:	57                   	push   %di
     2a6:	b8 00 04             	mov    $0x400,%ax
     2a9:	39 06 96 00          	cmp    %ax,0x96
     2ad:	73 03                	jae    0x2b2
     2af:	a1 96 00             	mov    0x96,%ax
     2b2:	29 06 96 00          	sub    %ax,0x96
     2b6:	8b c8                	mov    %ax,%cx
     2b8:	0b c0                	or     %ax,%ax
     2ba:	74 11                	je     0x2cd
     2bc:	8b 1e 94 00          	mov    0x94,%bx
     2c0:	ba 46 ac             	mov    $0xac46,%dx
     2c3:	b4 3f                	mov    $0x3f,%ah
     2c5:	cd 21                	int    $0x21
     2c7:	72 19                	jb     0x2e2
     2c9:	0b 75 b8             	or     -0x48(%di),%si
     2cc:	06                   	push   %es
     2cd:	e6 ff                	out    %al,$0xff
     2cf:	5a                   	pop    %dx
     2d0:	f9                   	stc
     2d1:	72 0f                	jb     0x2e2
     2d3:	a3 92 00             	mov    %ax,0x92
     2d6:	c7 06 90 00 00 00    	movw   $0x0,0x90
     2dc:	a0 46 ac             	mov    0xac46,%al
     2df:	f8                   	clc
     2e0:	32 e4                	xor    %ah,%ah
     2e2:	5a                   	pop    %dx
     2e3:	59                   	pop    %cx
     2e4:	5b                   	pop    %bx
     2e5:	5e                   	pop    %si
     2e6:	c3                   	ret
     2e7:	06                   	push   %es
     2e8:	56                   	push   %si
     2e9:	c4 36 8c 00          	les    0x8c,%si
     2ed:	26 88 04             	mov    %al,%es:(%si)
     2f0:	83 c6 01             	add    $0x1,%si
     2f3:	73 06                	jae    0x2fb
     2f5:	81 06 8e 00 00 10    	addw   $0x1000,0x8e
     2fb:	89 36 8c 00          	mov    %si,0x8c
     2ff:	5e                   	pop    %si
     300:	07                   	pop    %es
     301:	c3                   	ret
     302:	53                   	push   %bx
     303:	52                   	push   %dx
     304:	06                   	push   %es
     305:	e6 24                	out    %al,$0x24
     307:	35 21 1e             	xor    $0x1e21,%ax
     30a:	8c 06 98 00          	mov    %es,0x98
     30e:	89 1e 9a 00          	mov    %bx,0x9a
     312:	e6 24                	out    %al,$0x24
     314:	25 1e 0e             	and    $0xe1e,%ax
     317:	1f                   	pop    %ds
     318:	ba 35 03             	mov    $0x335,%dx
     31b:	cd b6                	int    $0xb6
     31d:	ff 07                	incw   (%bx)
     31f:	5a                   	pop    %dx
     320:	c3                   	ret
     321:	51                   	push   %cx
     322:	86 8b 16 9a          	xchg   %cl,-0x65ea(%bp,%di)
     326:	00 1e a1 98          	add    %bl,-0x675f
     32a:	00 8e d8 e6          	add    %cl,-0x1928(%bp)
     32e:	24 25                	and    $0x25,%al
     330:	cd b6                	int    $0xb6
     332:	ff 5a c3             	lcall  *-0x3d(%bp,%si)
     335:	53                   	push   %bx
     336:	1e                   	push   %ds
     337:	bb 63 8e             	mov    $0x8e63,%bx
     33a:	ff                   	lcall  (bad)
     33b:	db 80 3e 9c          	fildl  -0x63c2(%bx,%si)
     33f:	00 75 72             	add    %dh,0x72(%di)
     342:	1b 80 e4 80          	sbb    -0x7f1c(%bx,%si),%ax
     346:	74 0f                	je     0x357
     348:	1e                   	push   %ds
     349:	55                   	push   %bp
     34a:	1f                   	pop    %ds
     34b:	8b 44 04             	mov    0x4(%si),%ax
     34e:	1f                   	pop    %ds
     34f:	25 00 80             	and    $0x8000,%ax
     352:	75 03                	jne    0x357
     354:	bf 0d 00             	mov    $0xd,%di
     357:	8b c7                	mov    %di,%ax
     359:	04 14                	add    $0x14,%al
     35b:	a2 9c 00             	mov    %al,0x9c
     35e:	32 c0                	xor    %al,%al
     360:	1f                   	pop    %ds
     361:	5b                   	pop    %bx
     362:	cf                   	iret
     363:	52                   	push   %dx
     364:	8b 8b 0a b4          	mov    -0x4bf6(%bp,%di),%cx
     368:	0e                   	push   %cs
     369:	8a 56 04             	mov    0x4(%bp),%dl
     36c:	cd b6                	int    $0xb6
     36e:	5a                   	pop    %dx
     36f:	c2 00 c0             	ret    $0xc000
     372:	e8 b4 19             	call   0x1d29
     375:	cd b6                	int    $0xb6
     377:	32 e4                	xor    %ah,%ah
     379:	c3                   	ret
     37a:	cd 11                	int    $0x11
     37c:	b1 06                	mov    $0x6,%cl
     37e:	d3 e8                	shr    %cl,%ax
     380:	25 03 00             	and    $0x3,%ax
     383:	05 01 00             	add    $0x1,%ax
     386:	c3                   	ret
     387:	52                   	push   %dx
     388:	8b 8b 0a c8          	mov    -0x37f6(%bp,%di),%cx
     38c:	53                   	push   %bx
     38d:	8b 5e 04             	mov    0x4(%bp),%bx
     390:	8a c3                	mov    %bl,%al
     392:	ba d4 03             	mov    $0x3d4,%dx
     395:	8a e0                	mov    %al,%ah
     397:	b0 0d                	mov    $0xd,%al
     399:	ef                   	out    %ax,(%dx)
     39a:	8a c7                	mov    %bh,%al
     39c:	ba 03 e0             	mov    $0xe003,%dx
     39f:	0d 8a ba             	or     $0xba8a,%ax
     3a2:	0c ef                	or     $0xef,%al
     3a4:	5b                   	pop    %bx
     3a5:	5a                   	pop    %dx
     3a6:	00 e8                	add    %ch,%al
     3a8:	19 b6 e4 01          	sbb    %si,0x1e4(%bp)
     3ac:	cd 16                	int    $0x16
     3ae:	75 04                	jne    0x3b4
     3b0:	32 c0                	xor    %al,%al
     3b2:	eb 1c                	jmp    0x3d0
     3b4:	32 e4                	xor    %ah,%ah
     3b6:	cd 16                	int    $0x16
     3b8:	80 fc 52             	cmp    $0x52,%ah
     3bb:	75 02                	jne    0x3bf
     3bd:	b0 10                	mov    $0x10,%al
     3bf:	80 fc 9b             	cmp    $0x9b,%ah
     3c2:	75 02                	jne    0x3c6
     3c4:	b0 10                	mov    $0x10,%al
     3c6:	3c 13                	cmp    $0x13,%al
     3c8:	75 02                	jne    0x3cc
     3ca:	32 c0                	xor    %al,%al
     3cc:	0a 74 b4             	or     -0x4c(%si),%dh
     3cf:	da 32                	fidivl (%bp,%si)
     3d1:	e4 c3                	in     $0xc3,%al
     3d3:	53                   	push   %bx
     3d4:	1e                   	push   %ds
     3d5:	52                   	push   %dx
     3d6:	b8 09 35             	mov    $0x3509,%ax
     3d9:	21 1e 00 06          	and    %bx,0x600
     3dd:	9e                   	sahf
     3de:	00 06 00 a0          	add    %al,-0x6000
     3e2:	00 0e 1f ba          	add    %cl,-0x45e1
     3e6:	00 04                	add    %al,(%si)
     3e8:	b8 09 25             	mov    $0x2509,%ax
     3eb:	cd b6                	int    $0xb6
     3ed:	5a                   	pop    %dx
     3ee:	1f                   	pop    %ds
     3ef:	5b                   	pop    %bx
     3f0:	c3                   	ret
     3f1:	1e                   	push   %ds
     3f2:	52                   	push   %dx
     3f3:	c5 16 9e 00          	lds    0x9e,%dx
     3f7:	b8 25 b6             	mov    $0xb625,%ax
     3fa:	1f                   	pop    %ds
     3fb:	c3                   	ret
     3fc:	52                   	push   %dx
     3fd:	16                   	push   %ss
     3fe:	c3                   	ret
     3ff:	f8                   	clc
     400:	fb                   	sti
     401:	50                   	push   %ax
     402:	53                   	push   %bx
     403:	1e                   	push   %ds
     404:	bb 40 00             	mov    $0x40,%bx
     407:	db 21                	(bad) (%bx,%di)
     409:	a0 17 00             	mov    0x17,%al
     40c:	bb 63 8e             	mov    $0x8e63,%bx
     40f:	ff                   	lcall  (bad)
     410:	db a2 a4 00          	(bad) 0xa4(%bp,%si)
     414:	e4 60                	in     $0x60,%al
     416:	0d 8a 3c             	or     $0x3c8a,%ax
     419:	1f                   	pop    %ds
     41a:	75 13                	jne    0x42f
     41c:	8a 1e a4 00          	mov    0xa4,%bl
     420:	80 e3 04             	and    $0x4,%bl
     423:	80 fb 04             	cmp    $0x4,%bl
     426:	75 07                	jne    0x42f
     428:	80 36 e9 3e ff       	xorb   $0xff,0x3ee9
     42d:	eb 4a                	jmp    0x479
     42f:	24 7f                	and    $0x7f,%al
     431:	d2 b2 00 9a          	shlb   %cl,-0x6600(%bp,%si)
     435:	06                   	push   %es
     436:	74 0a                	je     0x442
     438:	3c 4e                	cmp    $0x4e,%al
     43a:	74 3d                	je     0x479
     43c:	3c 4a                	cmp    $0x4a,%al
     43e:	74 39                	je     0x479
     440:	75 0c                	jne    0x44e
     442:	3c 29                	cmp    $0x29,%al
     444:	75 02                	jne    0x448
     446:	b0 48                	mov    $0x48,%al
     448:	3c 2b                	cmp    $0x2b,%al
     44a:	75 02                	jne    0x44e
     44c:	b0 4b                	mov    $0x4b,%al
     44e:	3c 47                	cmp    $0x47,%al
     450:	72 27                	jb     0x479
     452:	3c 51                	cmp    $0x51,%al
     454:	77 23                	ja     0x479
     456:	2c 47                	sub    $0x47,%al
     458:	bb a6 00             	mov    $0xa6,%bx
     45b:	d7                   	xlat   %ds:(%bx)
     45c:	a2 a3 00             	mov    %al,0xa3
     45f:	8a 1e a2 00          	mov    0xa2,%bl
     463:	0a d8                	or     %al,%bl
     465:	80 80 0f 55 07       	addb   $0x7,0x550f(%bx,%si)
     46a:	c6 06 a5 00 fe       	movb   $0xfe,0xa5
     46f:	32 d8                	xor    %al,%bl
     471:	fe 06 a5 00          	incb   0xa5
     475:	88 1e a2 00          	mov    %bl,0xa2
     479:	9c                   	pushf
     47a:	fa                   	cli
     47b:	ff 06 9e 00          	incw   0x9e
     47f:	bb 00 21             	mov    $0x2100,%bx
     482:	17                   	pop    %ss
     483:	bb 8e db             	mov    $0xdb8e,%bx
     486:	a4                   	movsb  %ds:(%si),%es:(%di)
     487:	e4 0d                	in     $0xd,%al
     489:	3c 75                	cmp    $0x75,%al
     48b:	8a a4 80 04          	mov    0x480(%si),%ah
     48f:	1f                   	pop    %ds
     490:	5b                   	pop    %bx
     491:	58                   	pop    %ax
     492:	cf                   	iret
     493:	52                   	push   %dx
     494:	8b c8                	mov    %ax,%cx
     496:	8b 51 06             	mov    0x6(%bx,%di),%dx
     499:	56                   	push   %si
     49a:	57                   	push   %di
     49b:	1e                   	push   %ds
     49c:	8b 04                	mov    (%si),%ax
     49e:	96                   	xchg   %ax,%si
     49f:	8e c0                	mov    %ax,%es
     4a1:	8b 06 8c 8e          	mov    -0x7174,%ax
     4a5:	d8 33                	fdivs  (%bp,%di)
     4a7:	f6 8b fe b9 00       	testb  $0x0,-0x4602(%bp,%di)
     4ac:	7d f3                	jge    0x4a1
     4ae:	a5                   	movsw  %ds:(%si),%es:(%di)
     4af:	1f                   	pop    %ds
     4b0:	5f                   	pop    %di
     4b1:	5e                   	pop    %si
     4b2:	07                   	pop    %es
     4b3:	59                   	pop    %cx
     4b4:	5d                   	pop    %bp
     4b5:	c2 04 00             	ret    $0x4
     4b8:	8b 53 e8             	mov    -0x18(%bp,%di),%dx
     4bb:	ff 51 56             	call   *0x56(%bx,%di)
     4be:	57                   	push   %di
     4bf:	1e                   	push   %ds
     4c0:	8c 8e bf b0          	mov    %cs,-0x4f41(%bp)
     4c4:	8b 7e 08             	mov    0x8(%bp),%di
     4c7:	8b 04                	mov    (%si),%ax
     4c9:	c3                   	ret
     4ca:	8b 37                	mov    (%bx),%si
     4cc:	8b 47 02             	mov    0x2(%bx),%ax
     4cf:	8e 8b b8 5e          	mov    0x5eb8(%bp,%di),%cs
     4d3:	06                   	push   %es
     4d4:	d1 e3                	shl    $1,%bx
     4d6:	03 de                	add    %si,%bx
     4d8:	8b 07                	mov    (%bx),%ax
     4da:	03 f0                	add    %ax,%si
     4dc:	43                   	inc    %bx
     4dd:	43                   	inc    %bx
     4de:	8b 0f                	mov    (%bx),%cx
     4e0:	2b c8                	sub    %ax,%cx
     4e2:	f3 a4                	rep movsb %ds:(%si),%es:(%di)
     4e4:	1f                   	pop    %ds
     4e5:	5f                   	pop    %di
     4e6:	5e                   	pop    %si
     4e7:	59                   	pop    %cx
     4e8:	5b                   	pop    %bx
     4e9:	02 ec                	add    %ah,%ch
     4eb:	06                   	push   %es
     4ec:	b6 01                	mov    $0x1,%dh
     4ee:	16                   	push   %ss
     4ef:	04 c0                	add    $0xc0,%al
     4f1:	74 06                	je     0x4f9
     4f3:	b4 00                	mov    $0x0,%ah
     4f5:	04 c0                	add    $0xc0,%al
     4f7:	eb f4                	jmp    0x4ed
     4f9:	c3                   	ret
     4fa:	8b 52 46             	mov    0x46(%bp,%si),%dx
     4fd:	8b 08                	mov    (%bx,%si),%cx
     4ff:	8e 03                	mov    (%bp,%di),%es
     501:	06                   	push   %es
     502:	8c 3b                	mov    %?,(%bp,%di)
     504:	04 96                	add    $0x96,%al
     506:	7e 03                	jle    0x50b
     508:	46                   	inc    %si
     509:	ee                   	out    %al,(%dx)
     50a:	04 02                	add    $0x2,%al
     50c:	ec                   	in     (%dx),%al
     50d:	06                   	push   %es
     50e:	b6 8b                	mov    $0x8b,%dh
     510:	46                   	inc    %si
     511:	08 03                	or     %al,(%bp,%di)
     513:	8c 04                	mov    %es,(%si)
     515:	2b 06 8c 3b          	sub    0x3b8c,%ax
     519:	04 96                	add    $0x96,%al
     51b:	7d 03                	jge    0x520
     51d:	ee                   	out    %al,(%dx)
     51e:	02 06 8b 08          	add    0x88b,%al
     522:	8c 2b                	mov    %gs,(%bp,%di)
     524:	8c 04                	mov    %es,(%si)
     526:	7d 53                	jge    0x57b
     528:	57                   	push   %di
     529:	57                   	push   %di
     52a:	46                   	inc    %si
     52b:	b8 5e 06             	mov    $0x65e,%ax
     52e:	8b d3                	mov    %bx,%dx
     530:	4b                   	dec    %bx
     531:	42                   	inc    %dx
     532:	76 0f                	jbe    0x543
     534:	e6 4e                	out    %al,$0x4e
     536:	b9 03 00             	mov    $0x3,%cx
     539:	53                   	push   %bx
     53a:	56                   	push   %si
     53b:	52                   	push   %dx
     53c:	e8 8b 79             	call   0x7eca
     53f:	46                   	inc    %si
     540:	e2 f7                	loop   0x539
     542:	5e                   	pop    %si
     543:	59                   	pop    %cx
     544:	07                   	pop    %es
     545:	c2 5d 04             	ret    $0x45d
     548:	8b e8                	mov    %ax,%bp
     54a:	51                   	push   %cx
     54b:	57                   	push   %di
     54c:	8c 8b 06 8c          	mov    %cs,-0x73fa(%bp,%di)
     550:	33 02                	xor    (%bp,%si),%ax
     552:	06                   	push   %es
     553:	f7 d0                	not    %ax
     555:	25 80 03             	and    $0x380,%ax
     558:	5d                   	pop    %bp
     559:	04 8b                	add    $0x8b,%al
     55b:	e8 ba 88             	call   0x8e18
     55e:	03 bb 89 03          	add    0x389(%bp,%di),%di
     562:	e8 2b 00             	call   0x590
     565:	ec                   	in     (%dx),%al
     566:	24 e0                	and    $0xe0,%al
     568:	8a e0                	mov    %al,%ah
     56a:	b9 02 ff             	mov    $0xff02,%cx
     56d:	e8 2d 00             	call   0x59d
     570:	b9 04 21             	mov    $0x2104,%cx
     573:	e8 27 00             	call   0x59d
     576:	b9 ff ff             	mov    $0xffff,%cx
     579:	e2 fe                	loop   0x579
     57b:	ec                   	in     (%dx),%al
     57c:	24 e0                	and    $0xe0,%al
     57e:	50                   	push   %ax
     57f:	e8 0e 00             	call   0x590
     582:	59                   	pop    %cx
     583:	e6 5a                	out    %al,$0x5a
     585:	72 81                	jb     0x508
     587:	f9                   	stc
     588:	c0 00 74             	rolb   $0x74,(%bx,%si)
     58b:	e6 ff                	out    %al,$0xff
     58d:	00 00                	add    %al,(%bx,%si)
     58f:	c3                   	ret
     590:	b9 04 60             	mov    $0x6004,%cx
     593:	e8 07 00             	call   0x59d
     596:	b9 04 80             	mov    $0x8004,%cx
     599:	e8 01 c3             	call   0xc89d
     59c:	8b 8a c1 e8          	mov    -0x173f(%bp,%si),%cx
     5a0:	8c 2b                	mov    %gs,(%bp,%di)
     5a2:	8a c5                	mov    %ch,%al
     5a4:	e8 01 c3             	call   0xc8a8
     5a7:	8b ee                	mov    %si,%bp
     5a9:	87 da                	xchg   %bx,%dx
     5ab:	eb 00                	jmp    0x5ad
     5ad:	eb 00                	jmp    0x5af
     5af:	c3                   	ret
     5b0:	8b 52 46             	mov    0x46(%bp,%si),%dx
     5b3:	53                   	push   %bx
     5b4:	57                   	push   %di
     5b5:	57                   	push   %di
     5b6:	8b 04                	mov    (%si),%ax
     5b8:	c3                   	ret
     5b9:	8b 07                	mov    (%bx),%ax
     5bb:	2b 06 08 99          	sub    -0x66f8,%ax
     5bf:	33 c2                	xor    %dx,%ax
     5c1:	2b c2                	sub    %dx,%ax
     5c3:	f7 e0                	mul    %ax
     5c5:	8b c8                	mov    %ax,%cx
     5c7:	8b 47 02             	mov    0x2(%bx),%ax
     5ca:	2b 06 8c 99          	sub    -0x6674,%ax
     5ce:	c2 c2 e0             	ret    $0xe0c2
     5d1:	c8 47 2b 03          	enter  $0x2b47,$0x3
     5d5:	c8 8b 47 06          	enter  $0x478b,$0x6
     5d9:	f7 e0                	mul    %ax
     5db:	3b c8                	cmp    %ax,%cx
     5dd:	77 05                	ja     0x5e4
     5df:	e6 5a                	out    %al,$0x5a
     5e1:	72 eb                	jb     0x5ce
     5e3:	02 33                	add    (%bp,%di),%dh
     5e5:	c0 59 07 c2          	rcrb   $0xc2,0x7(%bx,%di)
     5e9:	5d                   	pop    %bp
     5ea:	04 8c                	add    $0x8c,%al
     5ec:	8c 7d 57             	mov    %?,0x57(%di)
     5ef:	46                   	inc    %si
     5f0:	5e                   	pop    %si
     5f1:	8b 4b 76             	mov    0x76(%bp,%di),%cx
     5f4:	e6 b9                	out    %al,$0xb9
     5f6:	00 56 0f             	add    %dl,0xf(%bp)
     5f9:	e6 8b                	out    %al,$0x8b
     5fb:	07                   	pop    %es
     5fc:	2b 04                	sub    (%si),%ax
     5fe:	99                   	cwtd
     5ff:	c2 c2 e0             	ret    $0xe0c2
     602:	c8 47 2b 8c          	enter  $0x2b47,$0x8c
     606:	c2 e0 47             	ret    $0x47e0
     609:	03 8b 44 02          	add    0x244(%bp,%di),%cx
     60d:	99                   	cwtd
     60e:	c2 c8 2b             	ret    $0x2bc8
     611:	c8 47 f7 3b          	enter  $0xf747,$0x3b
     615:	77 e6                	ja     0x5fd
     617:	72 02                	jb     0x61b
     619:	03 44 06             	add    0x6(%si),%ax
     61c:	e0 c8                	loopne 0x5e6
     61e:	05 5a eb             	add    $0xeb5a,%ax
     621:	33 59 c2             	xor    -0x3e(%bx,%di),%bx
     624:	04 8c                	add    $0x8c,%al
     626:	57                   	push   %di
     627:	5e                   	pop    %si
     628:	4b                   	dec    %bx
     629:	5e                   	pop    %si
     62a:	07                   	pop    %es
     62b:	5d                   	pop    %bp
     62c:	8b 51 8c             	mov    -0x74(%bx,%di),%dx
     62f:	06                   	push   %es
     630:	33 00                	xor    (%bx,%si),%ax
     632:	b0 33                	mov    $0x33,%al
     634:	b4 35                	mov    $0x35,%ah
     636:	1e                   	push   %ds
     637:	06                   	push   %es
     638:	00 c0                	add    %al,%al
     63a:	0b c3                	or     %bx,%ax
     63c:	74 33                	je     0x671
     63e:	59                   	pop    %cx
     63f:	33 00                	xor    (%bx,%si),%ax
     641:	cd 33                	int    $0x33
     643:	c3                   	ret
     644:	51                   	push   %cx
     645:	a1 52 a1             	mov    0xa152,%ax
     648:	ba 00 a3             	mov    $0xa300,%dx
     64b:	b2 00                	mov    $0x0,%dl
     64d:	c0 1a 74             	rcrb   $0x74,(%bp,%si)
     650:	15 b8 03             	adc    $0x3b8,%ax
     653:	00 cd                	add    %cl,%ch
     655:	33 8b c3 bb          	xor    -0x443d(%bp,%di),%cx
     659:	c2 04 a3             	ret    $0xa304
     65c:	b2 00                	mov    $0x0,%dl
     65e:	89 0e b6 00          	mov    %cx,0xb6
     662:	89 16 59 33          	mov    %dx,0x3359
     666:	07                   	pop    %es
     667:	5d                   	pop    %bp
     668:	8b c3                	mov    %bx,%ax
     66a:	8b 52 46             	mov    0x46(%bp,%si),%dx
     66d:	57                   	push   %di
     66e:	57                   	push   %di
     66f:	8b 4e 00             	mov    0x0(%bp),%cx
     672:	56                   	push   %si
     673:	56                   	push   %si
     674:	04 89                	add    $0x89,%al
     676:	b6 89                	mov    $0x89,%dh
     678:	59                   	pop    %cx
     679:	07                   	pop    %es
     67a:	8b 8b 46 a1          	mov    -0x5eba(%bp,%di),%cx
     67e:	ba 00 c0             	mov    $0xc000,%dx
     681:	1a 74 33             	sbb    0x33(%si),%dh
     684:	59                   	pop    %cx
     685:	06                   	push   %es
     686:	33 cd                	xor    %bp,%cx
     688:	33 e8                	xor    %ax,%bp
     68a:	59                   	pop    %cx
     68b:	c2 07 5d             	ret    $0x5d07
     68e:	5d                   	pop    %bp
     68f:	8b 51 8c             	mov    -0x74(%bx,%di),%dx
     692:	06                   	push   %es
     693:	33 06 57 51          	xor    0x5157,%ax
     697:	52                   	push   %dx
     698:	ba a3 00             	mov    $0xa3,%dx
     69b:	1a c0                	sbb    %al,%al
     69d:	1a 74 0b             	sbb    0xb(%si),%dh
     6a0:	b8 05 00             	mov    $0x5,%ax
     6a3:	5e                   	pop    %si
     6a4:	89 04                	mov    %ax,(%si)
     6a6:	cd 33                	int    $0x33
     6a8:	e8 29 33             	call   0x39d4
     6ab:	07                   	pop    %es
     6ac:	5d                   	pop    %bp
     6ad:	8b 5f 5d             	mov    0x5d(%bx),%bx
     6b0:	55                   	push   %bp
     6b1:	52                   	push   %dx
     6b2:	8b 56 83             	mov    -0x7d(%bp),%dx
     6b5:	e6 57                	out    %al,$0x57
     6b7:	52                   	push   %dx
     6b8:	a3 1a 1a             	mov    %ax,0x1a1a
     6bb:	0b 05                	or     (%di),%ax
     6bd:	5e                   	pop    %si
     6be:	04 33                	add    $0x33,%al
     6c0:	29 07                	sub    %ax,(%bx)
     6c2:	8c 8c 5e 04          	mov    %cs,0x45e(%si)
     6c6:	33 29                	xor    (%bx,%di),%bp
     6c8:	07                   	pop    %es
     6c9:	8b 08                	mov    (%bx,%si),%cx
     6cb:	33 5d 5f             	xor    0x5f(%di),%bx
     6ce:	55                   	push   %bp
     6cf:	8b 83 57 a3          	mov    -0x5ca9(%bp,%di),%ax
     6d3:	1a a3 b2 00          	sbb    0xb2(%bp,%di),%ah
     6d7:	89 1e b4 00          	mov    %bx,0xb4
     6db:	0e                   	push   %cs
     6dc:	00 16 33 5d          	add    %dl,0x5d33
     6e0:	c3                   	ret
     6e1:	52                   	push   %dx
     6e2:	57                   	push   %di
     6e3:	c3                   	ret
     6e4:	52                   	push   %dx
     6e5:	57                   	push   %di
     6e6:	8b 00                	mov    (%bx,%si),%ax
     6e8:	56                   	push   %si
     6e9:	89 89 07 0e          	mov    %cx,0xe07(%bx,%di)
     6ed:	00 bc 8c 5e          	add    %bh,0x5e8c(%si)
     6f1:	56                   	push   %si
     6f2:	04 89                	add    $0x89,%al
     6f4:	16                   	push   %ss
     6f5:	c0 46 ba c0          	rolb   $0xc0,-0x46(%bp)
     6f9:	74 59                	je     0x754
     6fb:	33 33                	xor    (%bp,%di),%si
     6fd:	59                   	pop    %cx
     6fe:	07                   	pop    %es
     6ff:	07                   	pop    %es
     700:	33 cd                	xor    %bp,%cx
     702:	33 07                	xor    (%bx),%ax
     704:	5d                   	pop    %bp
     705:	51                   	push   %cx
     706:	06                   	push   %es
     707:	06                   	push   %es
     708:	51                   	push   %cx
     709:	ba 00 c0             	mov    $0xc000,%dx
     70c:	00 89 07 00          	add    %cl,0x7(%bx,%di)
     710:	8c 56 89             	mov    %ss,-0x77(%bp)
     713:	be 8c 56             	mov    $0x568c,%si
     716:	89 c0                	mov    %ax,%ax
     718:	ba 74 c2             	mov    $0xc274,%dx
     71b:	46                   	inc    %si
     71c:	c0 59 33 07          	rcrb   $0x7,0x33(%bx,%di)
     720:	33 33                	xor    (%bp,%di),%si
     722:	5d                   	pop    %bp
     723:	06                   	push   %es
     724:	08 33                	or     %dh,(%bp,%di)
     726:	cd 07                	int    $0x7
     728:	51                   	push   %cx
     729:	06                   	push   %es
     72a:	ba c0 89             	mov    $0x89c0,%dx
     72d:	00 56 be             	add    %dl,-0x42(%bp)
     730:	56                   	push   %si
     731:	c0 a1 ec 3e 07       	shlb   $0x7,0x3eec(%bx,%di)
     736:	33 33                	xor    (%bp,%di),%si
     738:	0d 07 00             	or     $0x7,%ax
     73b:	04 0b                	add    $0xb,%al
     73d:	c9                   	leave
     73e:	74 b4                	je     0x6f4
     740:	04 01                	add    $0x1,%al
     742:	ff c4                	inc    %sp
     744:	b0 e6                	mov    $0xe6,%al
     746:	06                   	push   %es
     747:	ba c0 a3             	mov    $0xa3c0,%dx
     74a:	1a a1 3e 33          	sbb    0x333e(%bx,%di),%ah
     74e:	0d 00 0b             	or     $0xb00,%ax
     751:	b4 04                	mov    $0x4,%ah
     753:	02 ff                	add    %bh,%bh
     755:	c4 b0 e6 c3          	les    -0x3c1a(%bx,%si),%si
     759:	00 00                	add    %al,(%bx,%si)
     75b:	89 32                	mov    %si,(%bp,%si)
     75d:	cd 80                	int    $0x80
     75f:	1a 56 89             	sbb    -0x77(%bp),%dl
     762:	c8 33 5d c3          	enter  $0x5d33,$0xc3
     766:	ca 33 5d             	lret   $0x5d33
     769:	5f                   	pop    %di
     76a:	c3                   	ret
     76b:	52                   	push   %dx
     76c:	53                   	push   %bx
     76d:	57                   	push   %di
     76e:	04 1a                	add    $0x1a,%al
     770:	1a c8                	sbb    %al,%cl
     772:	33 bb bd 7a          	xor    0x7abd(%bp,%di),%di
     776:	f7 e3                	mul    %bx
     778:	05 0f 1b             	add    $0x1b0f,%ax
     77b:	a3 c8 33             	mov    %ax,0x33c8
     77e:	a1 ca 33             	mov    0x33ca,%ax
     781:	bb 0f 1b             	mov    $0x1b0f,%bx
     784:	f7 e3                	mul    %bx
     786:	05 bd 7a             	add    $0x7abd,%ax
     789:	a3 ca 33             	mov    %ax,0x33ca
     78c:	33 06 c8 33          	xor    0x33c8,%ax
     790:	5e                   	pop    %si
     791:	04 33                	add    $0x33,%al
     793:	f7 e3                	mul    %bx
     795:	8b c2                	mov    %dx,%ax
     797:	5a                   	pop    %dx
     798:	5d                   	pop    %bp
     799:	55                   	push   %bp
     79a:	52                   	push   %dx
     79b:	8b 56 00             	mov    0x0(%bp),%dx
     79e:	56                   	push   %si
     79f:	be 56 83             	mov    $0x8356,%si
     7a2:	ec                   	in     (%dx),%al
     7a3:	08 57 56             	or     %dl,0x56(%bx)
     7a6:	2b ff                	sub    %di,%di
     7a8:	e8 af ff             	call   0x75a
     7ab:	e8 28 79             	call   0x80d6
     7ae:	ff 76 04             	push   0x4(%bp)
     7b1:	ff 76 06             	push   0x6(%bp)
     7b4:	e8 cf a8             	call   0xb086
     7b7:	c6 06 f8 9a ff       	movb   $0xff,-0x6508
     7bc:	e8 eb 75             	call   0x7daa
     7bf:	e8 40 16             	call   0x1e02
     7c2:	e8 b5 a8             	call   0xb07a
     7c5:	0b b8 e6 07          	or     0x7e6(%bx,%si),%di
     7c9:	b8 01 00             	mov    $0x1,%ax
     7cc:	50                   	push   %ax
     7cd:	e8 17 07             	call   0xee7
     7d0:	89 3e fa 9a          	mov    %di,-0x6506
     7d4:	2b c0                	sub    %ax,%ax
     7d6:	50                   	push   %ax
     7d7:	50                   	push   %ax
     7d8:	b8 3f 01             	mov    $0x13f,%ax
     7db:	50                   	push   %ax
     7dc:	b8 c7 00             	mov    $0xc7,%ax
     7df:	50                   	push   %ax
     7e0:	e8 91 76             	call   0x7e74
     7e3:	2a c0                	sub    %al,%al
     7e5:	e8 ff 61             	call   0x69e7
     7e8:	41                   	inc    %cx
     7e9:	e8 e2 14             	call   0x1cce
     7ec:	e9 b8 01             	jmp    0x9a7
     7ef:	e8 a9 2d             	call   0x359b
     7f2:	88 46 fb             	mov    %al,-0x5(%bp)
     7f5:	0a b8 e6 03          	or     0x3e6(%bx,%si),%bh
     7f9:	e9 1f 01             	jmp    0x91b
     7fc:	ff 36 b6 00          	push   0xb6
     800:	ff 36 b8 00          	push   0xb8
     804:	b8 42 3c             	mov    $0x3c42,%ax
     807:	e8 ff 2d             	call   0x3609
     80a:	7a 0d                	jp     0x819
     80c:	00 0b                	add    %cl,(%bp,%di)
     80e:	b4 e8                	mov    $0xe8,%ah
     810:	a8 01                	test   $0x1,%al
     812:	e9 59 01             	jmp    0x96e
     815:	36 00 36 00 42       	add    %dh,%ss:0x4200
     81a:	e8 2d 0d             	call   0x154a
     81d:	0b 4a 3c             	or     0x3c(%bp,%si),%cx
     820:	e8 ff 14             	call   0x1d22
     823:	7a 0d                	jp     0x832
     825:	00 0b                	add    %cl,(%bp,%di)
     827:	11 e8                	adc    %bp,%ax
     829:	f5                   	cmc
     82a:	54                   	push   %sp
     82b:	e8 5d 12             	call   0x1a8b
     82e:	8b f8                	mov    %ax,%di
     830:	b0 ff                	mov    $0xff,%al
     832:	e8 ff 14             	call   0x1d34
     835:	41                   	inc    %cx
     836:	e9 35 01             	jmp    0x96e
     839:	00 00                	add    %al,(%bx,%si)
     83b:	e8 0d 4a             	call   0x524b
     83e:	e8 14 0d             	call   0x1555
     841:	e8 23 64             	call   0x6c67
     844:	88 46 fc             	mov    %al,-0x4(%bp)
     847:	3c 1b                	cmp    $0x1b,%al
     849:	73 3e                	jae    0x889
     84b:	b1 05                	mov    $0x5,%cl
     84d:	8a d8                	mov    %al,%bl
     84f:	2a ff                	sub    %bh,%bh
     851:	d3 e3                	shl    %cl,%bx
     853:	80 bf 85 52 00       	cmpb   $0x0,0x5285(%bx)
     858:	74 25                	je     0x87f
     85a:	a2 80 c0             	mov    %al,0xc080
     85d:	80 7e fb 01          	cmpb   $0x1,-0x5(%bp)
     861:	74 03                	je     0x866
     863:	e9 89 0d             	jmp    0x15ef
     866:	e8 b7 54             	call   0x5d20
     869:	80 7e fc 00          	cmpb   $0x0,-0x4(%bp)
     86d:	75 b4                	jne    0x823
     86f:	e8 f4 6a             	call   0x7366
     872:	e9 f9 00             	jmp    0x96e
     875:	8a 46 fc             	mov    -0x4(%bp),%al
     878:	e8 ff 3c             	call   0x457a
     87b:	64 e9 ef 00          	fs jmp 0x96e
     87f:	46                   	inc    %si
     880:	e8 3c e9             	call   0xf1bf
     883:	00 85 69 e9          	add    %al,-0x1697(%di)
     887:	e5 00                	in     $0x0,%ax
     889:	80 7e fc ff          	cmpb   $0xff,-0x4(%bp)
     88d:	e6 03                	out    %al,$0x3
     88f:	e9 dc 00             	jmp    0x96e
     892:	c6 46 fd ff          	movb   $0xff,-0x3(%bp)
     896:	2b d2                	sub    %dx,%dx
     898:	be 14 11             	mov    $0x1114,%si
     89b:	eb 14                	jmp    0x8b1
     89d:	8a 04                	mov    (%si),%al
     89f:	38 e8                	cmp    %ch,%al
     8a1:	3c 75                	cmp    $0x75,%al
     8a3:	09 80 7c 02          	or     %ax,0x27c(%bx,%si)
     8a7:	ff 74 03             	push   0x3(%si)
     8aa:	88 56 fd             	mov    %dl,-0x3(%bp)
     8ad:	83 c6 0c             	add    $0xc,%si
     8b0:	42                   	inc    %dx
     8b1:	81 fe 5c 11          	cmp    $0x115c,%si
     8b5:	72 e6                	jb     0x89d
     8b7:	80 7e fd ff          	cmpb   $0xff,-0x3(%bp)
     8bb:	74 c2                	je     0x87f
     8bd:	80 7e fb 01          	cmpb   $0x1,-0x5(%bp)
     8c1:	75 2c                	jne    0x8ef
     8c3:	80 7e fd 03          	cmpb   $0x3,-0x3(%bp)
     8c7:	73 b6                	jae    0x87f
     8c9:	46                   	inc    %si
     8ca:	e8 3c e9             	call   0xf209
     8cd:	00 9f 23 88          	add    %bl,-0x77dd(%bx)
     8d1:	46                   	inc    %si
     8d2:	fd                   	std
     8d3:	e8 4d 36             	call   0x3f23
     8d6:	80 7e fd 03          	cmpb   $0x3,-0x3(%bp)
     8da:	72 03                	jb     0x8df
     8dc:	e9 8f 00             	jmp    0x96e
     8df:	46                   	inc    %si
     8e0:	e8 fd e9             	call   0xf2e0
     8e3:	00 74 26             	add    %dh,0x26(%si)
     8e6:	46                   	inc    %si
     8e7:	e8 fd e9             	call   0xf2e7
     8ea:	00 53 22             	add    %dl,0x22(%bp,%di)
     8ed:	eb 7f                	jmp    0x96e
     8ef:	80 7e fb 02          	cmpb   $0x2,-0x5(%bp)
     8f3:	75 79                	jne    0x96e
     8f5:	46                   	inc    %si
     8f6:	e8 3c e9             	call   0xf235
     8f9:	2b 50 b8             	sub    -0x48(%bx,%si),%dx
     8fc:	e8 b0 32             	call   0x3baf
     8ff:	88 46 ff             	mov    %al,-0x1(%bp)
     902:	e8 1e 36             	call   0x3f23
     905:	80 7e ff 0c          	cmpb   $0xc,-0x1(%bp)
     909:	73 63                	jae    0x96e
     90b:	46                   	inc    %si
     90c:	e8 ff e8             	call   0xf20e
     90f:	ff 91 35 46          	call   *0x4635(%bx,%di)
     913:	e8 ff e8             	call   0xf215
     916:	ff 11                	call   *(%bx,%di)
     918:	2f                   	das
     919:	eb 53                	jmp    0x96e
     91b:	a0 7f c0             	mov    0xc07f,%al
     91e:	2a e4                	sub    %ah,%ah
     920:	e8 ff b8             	call   0xc222
     923:	94                   	xchg   %ax,%sp
     924:	5b                   	pop    %bx
     925:	2a e4                	sub    %ah,%ah
     927:	3d 51 00             	cmp    $0x51,%ax
     92a:	74 34                	je     0x960
     92c:	77 40                	ja     0x96e
     92e:	2c 1b                	sub    $0x1b,%al
     930:	e6 03                	out    %al,$0x3
     932:	e9 f3 fe             	jmp    0x828
     935:	2c 2a                	sub    $0x2a,%al
     937:	e6 03                	out    %al,$0x3
     939:	e9 d3 fe             	jmp    0x80f
     93c:	fe c8                	dec    %al
     93e:	74 11                	je     0x951
     940:	2c 06                	sub    $0x6,%al
     942:	74 12                	je     0x956
     944:	fe c8                	dec    %al
     946:	e6 03                	out    %al,$0x3
     948:	e9 e5 fe             	jmp    0x830
     94b:	2c 03                	sub    $0x3,%al
     94d:	74 0c                	je     0x95b
     94f:	eb 1d                	jmp    0x96e
     951:	e8 82 26             	call   0x2fd6
     954:	eb 18                	jmp    0x96e
     956:	e8 27 39             	call   0x4280
     959:	eb 13                	jmp    0x96e
     95b:	e8 6e 60             	call   0x69cc
     95e:	eb 0e                	jmp    0x96e
     960:	b8 72 01             	mov    $0x172,%ax
     963:	e8 ff b9             	call   0xc365
     966:	52                   	push   %dx
     967:	0d 0b e8             	or     $0xe80b,%ax
     96a:	03 bf ff ff          	add    -0x1(%bx),%di
     96e:	2b d2                	sub    %dx,%dx
     970:	bb 85 52             	mov    $0x5285,%bx
     973:	eb 0b                	jmp    0x980
     975:	80 3f 00             	cmpb   $0x0,(%bx)
     978:	e6 00                	out    %al,$0x0
     97a:	ba ff ff             	mov    $0xffff,%dx
     97d:	83 c3 20             	add    $0x20,%bx
     980:	81 fb e5 55          	cmp    $0x55e5,%bx
     984:	72 ef                	jb     0x975
     986:	0b d2                	or     %dx,%dx
     988:	75 0a                	jne    0x994
     98a:	b8 7f 01             	mov    $0x17f,%ax
     98d:	e8 ff 43             	call   0x4d8f
     990:	50                   	push   %ax
     991:	bf ff ff             	mov    $0xffff,%di
     994:	e8 44 2c             	call   0x35db
     997:	80 3e 71 59 08       	cmpb   $0x8,0x5971
     99c:	75 09                	jne    0x9a7
     99e:	bf ff ff             	mov    $0xffff,%di
     9a1:	e8 7d 14             	call   0x1e21
     9a4:	e8 eb a6             	call   0xb092
     9a7:	0b ff                	or     %di,%di
     9a9:	03 dc                	add    %sp,%bx
     9ab:	c6 41 fe 2b          	movb   $0x2b,-0x2(%bx,%di)
     9af:	50                   	push   %ax
     9b0:	b8 e8 33             	mov    $0x33e8,%ax
     9b3:	05 5e 5f             	add    $0x5f5e,%ax
     9b6:	8b e5                	mov    %bp,%sp
     9b8:	5d                   	pop    %bp
     9b9:	c3                   	ret
     9ba:	53                   	push   %bx
     9bb:	04 1a                	add    $0x1a,%al
     9bd:	83 ec 0c             	sub    $0xc,%sp
     9c0:	57                   	push   %di
     9c1:	56                   	push   %si
     9c2:	c7 46 f6 00 00       	movw   $0x0,-0xa(%bp)
     9c7:	b8 02 00             	mov    $0x2,%ax
     9ca:	e8 76 87             	call   0x9143
     9cd:	13 a0 4e 63          	adc    0x634e(%bx,%si),%sp
     9d1:	2a e4                	sub    %ah,%ah
     9d3:	e8 b8 00             	call   0xa8e
     9d6:	89 e8                	mov    %bp,%ax
     9d8:	b8 64 00             	mov    $0x64,%ax
     9db:	e8 76 1b             	call   0x2554
     9de:	fb                   	sti
     9df:	a2 4e 63             	mov    %al,0x634e
     9e2:	83 3e d8 0e 00       	cmpw   $0x0,0xed8
     9e7:	e6 15                	out    %al,$0x15
     9e9:	a0 7e 52             	mov    0x527e,%al
     9ec:	98                   	cbtw
     9ed:	e8 b8 0f             	call   0x19a8
     9f0:	89 b8 00 76          	mov    %di,0x7600(%bx,%si)
     9f4:	fb                   	sti
     9f5:	4e                   	dec    %si
     9f6:	83 01 fb             	addw   $0xfffb,(%bx,%di)
     9f9:	a2 7e 52             	mov    %al,0x527e
     9fc:	eb 28                	jmp    0xa26
     9fe:	83 3e da 0e 00       	cmpw   $0x0,0xeda
     a03:	e6 21                	out    %al,$0x21
     a05:	a0 d6 0e             	mov    0xed6,%al
     a08:	98                   	cbtw
     a09:	8b d8                	mov    %ax,%bx
     a0b:	b1 05                	mov    $0x5,%cl
     a0d:	d3 e3                	shl    %cl,%bx
     a0f:	8a 87 7e 98          	mov    -0x6782(%bx),%al
     a13:	b8 89 00             	mov    $0x89,%ax
     a16:	fb                   	sti
     a17:	83 fb 7e             	cmp    $0x7e,%bx
     a1a:	eb 83                	jmp    0x99f
     a1c:	da 8b f3 e8          	fimull -0x170d(%bp,%di)
     a20:	d8 fa                	fdivr  %st(2),%st
     a22:	88 84 7e 98          	mov    %al,-0x6782(%si)
     a26:	be 25 19             	mov    $0x1925,%si
     a29:	eb 59                	jmp    0xa84
     a2b:	80 3c ff             	cmpb   $0xff,(%si)
     a2e:	74 51                	je     0xa81
     a30:	c6 44 01 00          	movb   $0x0,0x1(%si)
     a34:	8a 44 fe             	mov    -0x2(%si),%al
     a37:	2a e4                	sub    %ah,%ah
     a39:	e8 b8 02             	call   0xcf4
     a3c:	00 e8                	add    %ch,%al
     a3e:	b8 63 fb             	mov    $0xfb63,%ax
     a41:	4e                   	dec    %si
     a42:	83 b5 fa 88 44       	xorw   $0x44,-0x7706(%di)
     a47:	fe 80 7c 04          	incb   0x47c(%bx,%si)
     a4b:	ff 03                	incw   (%bp,%di)
     a4d:	33 ff                	xor    %di,%di
     a4f:	74 f9                	je     0xa4a
     a51:	7e eb                	jle    0xa3e
     a53:	83 da 8a             	sbb    $0xff8a,%dx
     a56:	1c 2a                	sbb    $0x2a,%al
     a58:	ff d1                	call   *%cx
     a5a:	e3 ff                	jcxz   0xa5b
     a5c:	b7 ac                	mov    $0xac,%bh
     a5e:	1a e8                	sbb    %al,%ch
     a60:	98                   	cbtw
     a61:	fa                   	cli
     a62:	89 44 f9             	mov    %ax,-0x7(%si)
     a65:	80 3c 00             	cmpb   $0x0,(%si)
     a68:	e6 17                	out    %al,$0x17
     a6a:	ff 74 fb             	push   -0x5(%si)
     a6d:	b8 0a 83             	mov    $0x830a,%ax
     a70:	8a 2a                	mov    (%bp,%si),%ch
     a72:	d1 ff                	sar    $1,%di
     a74:	ac                   	lods   %ds:(%si),%al
     a75:	e8 fa 44             	call   0x4f72
     a78:	80 b2 1a e8 7c       	xorb   $0x7c,-0x17e6(%bp,%si)
     a7d:	fa                   	cli
     a7e:	89 44 fb             	mov    %ax,-0x5(%si)
     a81:	83 c6 14             	add    $0x14,%si
     a84:	81 fe 15 1a          	cmp    $0x1a15,%si
     a88:	72 a1                	jb     0xa2b
     a8a:	bb 16 11             	mov    $0x1116,%bx
     a8d:	eb 80                	jmp    0xa0f
     a8f:	00 00                	add    %al,(%bx,%si)
     a91:	ff 03                	incw   (%bp,%di)
     a93:	56                   	push   %si
     a94:	c6 07 00             	movb   $0x0,(%bx)
     a97:	83 c3 0c             	add    $0xc,%bx
     a9a:	81 fb 3a 11          	cmp    $0x113a,%bx
     a9e:	72 ef                	jb     0xa8f
     aa0:	c7 46 fc 00 00       	movw   $0x0,-0x4(%bp)
     aa5:	be 85 52             	mov    $0x5285,%si
     aa8:	e9 0d 02             	jmp    0xcb8
     aab:	fe 44 04             	incb   0x4(%si)
     aae:	80 3c 00             	cmpb   $0x0,(%si)
     ab1:	e6 06                	out    %al,$0x6
     ab3:	81 fe 85 52          	cmp    $0x5285,%si
     ab7:	75 20                	jne    0xad9
     ab9:	ff 03                	incw   (%bp,%di)
     abb:	eb 7e                	jmp    0xb3b
     abd:	eb 83                	jmp    0xa42
     abf:	da ff                	(bad)
     ac1:	03 e7                	add    %di,%sp
     ac3:	e8 34 fa             	call   0x4fa
     ac6:	89 44 eb             	mov    %ax,-0x15(%si)
     ac9:	ff 03                	incw   (%bp,%di)
     acb:	ed                   	in     (%dx),%ax
     acc:	b8 19 83             	mov    $0x8319,%ax
     acf:	da ff                	(bad)
     ad1:	03 e9                	add    %cx,%bp
     ad3:	e8 24 fa             	call   0x4fa
     ad6:	89 44 ed             	mov    %ax,-0x13(%si)
     ad9:	80 3c 00             	cmpb   $0x0,(%si)
     adc:	03 c6                	add    %si,%ax
     ade:	fe                   	(bad)
     adf:	d1 01                	rolw   $1,(%bx,%di)
     ae1:	8b 44 f1             	mov    -0xf(%si),%ax
     ae4:	89 46 f4             	mov    %ax,-0xc(%bp)
     ae7:	b9 28 00             	mov    $0x28,%cx
     aea:	2b d2                	sub    %dx,%dx
     aec:	f7 f1                	div    %cx
     aee:	89 46 fa             	mov    %ax,-0x6(%bp)
     af1:	3d 32 00             	cmp    $0x32,%ax
     af4:	73 05                	jae    0xafb
     af6:	c7 46 fa 32 00       	movw   $0x32,-0x6(%bp)
     afb:	ff 03                	incw   (%bp,%di)
     afd:	ef                   	out    %ax,(%dx)
     afe:	ff 76 fa             	push   -0x6(%bp)
     b01:	ff 76 f4             	push   -0xc(%bp)
     b04:	e8 f3 f9             	call   0x4fa
     b07:	89 44 ef             	mov    %ax,-0x11(%si)
     b0a:	8a 44 fa             	mov    -0x6(%si),%al
     b0d:	f6 24                	mulb   (%si)
     b0f:	99                   	cwtd
     b10:	c8 c8 f7 77          	enter  $0xf7c8,$0x77
     b14:	b9 02 00             	mov    $0x2,%cx
     b17:	d3 f8                	sar    %cl,%ax
     b19:	c8 c8 f7 77          	enter  $0xf7c8,$0x77
     b1d:	8b f8                	mov    %ax,%di
     b1f:	8a 44 f9             	mov    -0x7(%si),%al
     b22:	88 ff                	mov    %bh,%bh
     b24:	1e                   	push   %ds
     b25:	8a 44 fd             	mov    -0x3(%si),%al
     b28:	2a e4                	sub    %ah,%ah
     b2a:	0d e8 bf             	or     $0xbfe8,%ax
     b2d:	0e                   	push   %cs
     b2e:	48                   	dec    %ax
     b2f:	74 19                	je     0xb4a
     b31:	48                   	dec    %ax
     b32:	74 37                	je     0xb6b
     b34:	48                   	dec    %ax
     b35:	74 48                	je     0xb7f
     b37:	48                   	dec    %ax
     b38:	74 68                	je     0xba2
     b3a:	eb 7e                	jmp    0xbba
     b3c:	c6 44 fc 00          	movb   $0x0,-0x4(%si)
     b40:	46                   	inc    %si
     b41:	ff                   	(bad)
     b42:	ff                   	(bad)
     b43:	b8 00 83             	mov    $0x8300,%ax
     b46:	0a 83 eb 18          	or     0x18eb(%bp,%di),%al
     b4a:	8b c7                	mov    %di,%ax
     b4c:	99                   	cwtd
     b4d:	c8 c8 f7 77          	enter  $0xf7c8,$0x77
     b51:	d3 c8                	ror    %cl,%ax
     b53:	f7 8b 8a f9 88 44    	testw  $0x4488,-0x676(%bp,%di)
     b59:	fc                   	cld
     b5a:	46                   	inc    %si
     b5b:	ff 00                	incw   (%bx,%si)
     b5d:	0a eb                	or     %bl,%ch
     b5f:	8b 00                	mov    (%bx,%si),%ax
     b61:	e8 64 2b             	call   0x36c8
     b64:	b8 33 5e             	mov    $0x5e33,%ax
     b67:	a6                   	cmpsb  %es:(%di),%ds:(%si)
     b68:	f9                   	stc
     b69:	eb 4c                	jmp    0xbb7
     b6b:	8b c7                	mov    %di,%ax
     b6d:	99                   	cwtd
     b6e:	8a f9                	mov    %cl,%bh
     b70:	d1 f8                	sar    $1,%ax
     b72:	88 fc                	mov    %bh,%ah
     b74:	ff 0a                	decw   (%bp,%si)
     b76:	8b e8                	mov    %ax,%bp
     b78:	2b 33                	sub    (%bp,%di),%si
     b7a:	a6                   	cmpsb  %es:(%di),%ds:(%si)
     b7b:	eb 8b                	jmp    0xb08
     b7d:	eb 30                	jmp    0xbaf
     b7f:	8b c7                	mov    %di,%ax
     b81:	8b c8                	mov    %ax,%cx
     b83:	d1 e0                	shl    $1,%ax
     b85:	03 c1                	add    %cx,%ax
     b87:	99                   	cwtd
     b88:	c8 77 02 d3          	enter  $0x277,$0xd3
     b8c:	c8 f7 8b 8a          	enter  $0x8bf7,$0x8a
     b90:	f9                   	stc
     b91:	ff 8a fd e4          	decw   -0x1b03(%bp,%si)
     b95:	88 ff                	mov    %bh,%bh
     b97:	8b 2b                	mov    (%bp,%di),%bp
     b99:	a6                   	cmpsb  %es:(%di),%ds:(%si)
     b9a:	8b 30                	mov    (%bx,%si),%si
     b9c:	c7                   	(bad)
     b9d:	c8 0a 83 eb          	enter  $0x830a,$0xeb
     ba1:	0d 8b c7             	or     $0xc78b,%ax
     ba4:	88 8b a6 30          	mov    %cl,0x30a6(%bp,%di)
     ba8:	c8 83 0d c7          	enter  $0xd83,$0xc7
     bac:	8b 14                	mov    (%si),%dx
     bae:	89 00                	mov    %ax,(%bx,%si)
     bb0:	fb                   	sti
     bb1:	83 fb 7e             	cmp    $0x7e,%bx
     bb4:	eb 43                	jmp    0xbf9
     bb6:	f9                   	stc
     bb7:	ff 8a fd e4          	decw   -0x1b03(%bp,%si)
     bbb:	e8 0e 38             	call   0x43cc
     bbe:	44                   	inc    %sp
     bbf:	fe                   	(bad)
     bc0:	73 26                	jae    0xbe8
     bc2:	80 3e 4e 63 0f       	cmpb   $0xf,0x634e
     bc7:	73 07                	jae    0xbd0
     bc9:	06                   	push   %es
     bca:	00 4e 63             	add    %cl,0x63(%bp)
     bcd:	83 eb 05             	sub    $0x5,%bx
     bd0:	80 2e 4e 63 0f       	subb   $0xf,0x634e
     bd5:	8a 44 f9             	mov    -0x7(%si),%al
     bd8:	0d c7 8b             	or     $0x8bc7,%ax
     bdb:	1e                   	push   %ds
     bdc:	89 fb                	mov    %di,%bx
     bde:	fb                   	sti
     bdf:	eb f9                	jmp    0xbda
     be1:	8a e4                	mov    %ah,%ah
     be3:	15 f9 ff             	adc    $0xfff9,%ax
     be6:	44                   	inc    %sp
     be7:	f9                   	stc
     be8:	30 c8                	xor    %cl,%al
     bea:	83 3c 01             	cmpw   $0x1,(%si)
     bed:	7d 02                	jge    0xbf1
     bef:	b0 01                	mov    $0x1,%al
     bf1:	ff 44 f9             	incw   -0x7(%si)
     bf4:	8b c7                	mov    %di,%ax
     bf6:	88 8b f3 b8          	mov    %cl,-0x470d(%bp,%di)
     bfa:	32 00                	xor    (%bx,%si),%al
     bfc:	f7 64 ef             	mulw   -0x11(%si)
     bff:	2b f7                	sub    %di,%si
     c01:	89 74 f1             	mov    %si,-0xf(%si)
     c04:	b9 ce ff             	mov    $0xffce,%cx
     c07:	d0 c2                	rol    $1,%dl
     c09:	8a 44 fc             	mov    -0x4(%si),%al
     c0c:	2a e4                	sub    %ah,%ah
     c0e:	8b da                	mov    %dx,%bx
     c10:	f7 e9                	imul   %cx
     c12:	8d 4d 01             	lea    0x1(%di),%cx
     c15:	99                   	cwtd
     c16:	f7 f9                	idiv   %cx
     c18:	03 d8                	add    %ax,%bx
     c1a:	83 c3 32             	add    $0x32,%bx
     c1d:	89 5e fa             	mov    %bx,-0x6(%bp)
     c20:	81 85 75 ff eb 1f    	addw   $0x1feb,-0x8b(%di)
     c26:	b8 9c 63             	mov    $0x639c,%ax
     c29:	99                   	cwtd
     c2a:	52                   	push   %dx
     c2b:	50                   	push   %ax
     c2c:	a1 4a 59             	mov    0x594a,%ax
     c2f:	99                   	cwtd
     c30:	52                   	push   %dx
     c31:	50                   	push   %ax
     c32:	a0 63 e4             	mov    0xe463,%al
     c35:	b8 89 f7             	mov    $0xf789,%ax
     c38:	e3 52                	jcxz   0xc8c
     c3a:	50                   	push   %ax
     c3b:	e8 c6 90             	call   0x9d04
     c3e:	52                   	push   %dx
     c3f:	50                   	push   %ax
     c40:	e8 f3 90             	call   0x9d36
     c43:	eb 0e                	jmp    0xc53
     c45:	a0 63 e4             	mov    0xe463,%al
     c48:	f6 24                	mulb   (%si)
     c4a:	f7 e3                	mul    %bx
     c4c:	b9 90 01             	mov    $0x190,%cx
     c4f:	2b f7                	sub    %di,%si
     c51:	89 fa                	mov    %di,%dx
     c53:	3d 01 00             	cmp    $0x1,%ax
     c56:	7d 03                	jge    0xc5b
     c58:	b8 00 e8             	mov    $0xe800,%ax
     c5b:	3d 63 83             	cmp    $0x8363,%ax
     c5e:	7e 03                	jle    0xc63
     c60:	b8 63 83             	mov    $0x8363,%ax
     c63:	88 8b fb ff          	mov    %cl,-0x5(%bp,%di)
     c67:	76 fc                	jbe    0xc65
     c69:	e8 a7 00             	call   0xd13
     c6c:	80 00 06             	addb   $0x6,(%bx,%si)
     c6f:	fe 41 80             	incb   -0x80(%bx,%di)
     c72:	3e 85 00             	test   %ax,%ds:(%bx,%si)
     c75:	25 80 3a             	and    $0x3a80,%ax
     c78:	8a 44 fc             	mov    -0x4(%si),%al
     c7b:	2a e4                	sub    %ah,%ah
     c7d:	01 46 f6             	add    %ax,-0xa(%bp)
     c80:	b0 19                	mov    $0x19,%al
     c82:	f6 24                	mulb   (%si)
     c84:	3a 00                	cmp    (%bx,%si),%al
     c86:	4e                   	dec    %si
     c87:	63 73 28             	arpl   %si,0x28(%bp,%di)
     c8a:	80 04 03             	addb   $0x3,(%si)
     c8d:	05 76 22             	add    $0x2276,%ax
     c90:	ff 76 fc             	push   -0x4(%bp)
     c93:	e8 90 5f             	call   0x6c26
     c96:	3d 02 00             	cmp    $0x2,%ax
     c99:	72 17                	jb     0xcb2
     c9b:	c6 44 04 00          	movb   $0x0,0x4(%si)
     c9f:	fe 04                	incb   (%si)
     ca1:	3c e6                	cmp    $0xe6,%al
     ca3:	04 76                	add    $0x76,%al
     ca5:	05 c6 04             	add    $0x4c6,%ax
     ca8:	04 eb                	add    $0xeb,%al
     caa:	07                   	pop    %es
     cab:	01 50 17             	add    %dx,0x17(%bx,%si)
     cae:	89 fa                	mov    %di,%dx
     cb0:	7e 66                	jle    0xd18
     cb2:	83 c6 20             	add    $0x20,%si
     cb5:	ff 46 fc             	incw   -0x4(%bp)
     cb8:	81 85 e5 55 73 03    	addw   $0x373,0x55e5(%di)
     cbe:	e9 ea fd             	jmp    0xaab
     cc1:	e8 60 3f             	call   0x4c24
     cc4:	80 4e 0f 07          	orb    $0x7,0xf(%bp)
     cc8:	4b                   	dec    %bx
     cc9:	76 07                	jbe    0xcd2
     ccb:	50                   	push   %ax
     ccc:	89 7e 83             	mov    %di,-0x7d(%bp)
     ccf:	20 5e 66             	and    %bl,0x66(%bp)
     cd2:	e8 09 66             	call   0x72de
     cd5:	ff 06 4c 63          	incw   0x634c
     cd9:	e8 ae 10             	call   0x1d8a
     cdc:	b0 e8                	mov    $0xe8,%al
     cde:	14 e9                	adc    $0xe9,%al
     ce0:	68 3c 8b             	push   $0x8b3c
     ce3:	46                   	inc    %si
     ce4:	f6 01 06             	testb  $0x6,(%bx,%di)
     ce7:	4a                   	dec    %dx
     ce8:	63 b1 05 8b          	arpl   %si,-0x74fb(%bx,%di)
     cec:	5e                   	pop    %si
     ced:	fc                   	cld
     cee:	d3 8a 7e b8          	rorw   %cl,-0x4782(%bp,%si)
     cf2:	81 52 2a e4 2b       	adcw   $0x2be4,0x2a(%bp,%si)
     cf7:	f7 01 06 92          	testw  $0x9206,(%bx,%di)
     cfb:	c0 11 16             	rclb   $0x16,(%bx,%di)
     cfe:	94                   	xchg   %ax,%sp
     cff:	c0 83 3e 4c 63       	rolb   $0x63,0x4c3e(%bp,%di)
     d04:	04 76                	add    $0x76,%al
     d06:	06                   	push   %es
     d07:	ff 76 f6             	push   -0xa(%bp)
     d0a:	e8 8b 11             	call   0x1e98
     d0d:	5e                   	pop    %si
     d0e:	8b 5d 53             	mov    0x53(%di),%bx
     d11:	1a ec                	sbb    %ah,%ch
     d13:	57                   	push   %di
     d14:	c7                   	(bad)
     d15:	f6 00 02             	testb  $0x2,(%bx,%si)
     d18:	0a 57 56             	or     0x56(%bx),%dl
     d1b:	8b 7e 04             	mov    0x4(%bp),%di
     d1e:	b1 05                	mov    $0x5,%cl
     d20:	8b df                	mov    %di,%bx
     d22:	d3 8a 89 5e          	rorw   %cl,0x5e89(%bp,%si)
     d26:	f6 8a 7e b8 00       	testb  $0x0,-0x4782(%bp,%si)
     d2b:	2a 87 7d 52          	sub    0x527d(%bx),%al
     d2f:	3a 87 80 52          	cmp    0x5280(%bx),%al
     d33:	7f 03                	jg     0xd38
     d35:	e9 a7 01             	jmp    0xedf
     d38:	b9 19 00             	mov    $0x19,%cx
     d3b:	1e                   	push   %ds
     d3c:	00 63 b8             	add    %ah,-0x48(%bp,%di)
     d3f:	f7 ff                	idiv   %di
     d41:	b8 00 89             	mov    $0x8900,%ax
     d44:	cd 76                	int    $0x76
     d46:	f6 8a 94 85 00       	testb  $0x0,-0x7a6c(%bp,%si)
     d4b:	2a f6                	sub    %dh,%dh
     d4d:	2b 47 3b             	sub    0x3b(%bx),%ax
     d50:	eb f7                	jmp    0xd49
     d52:	e9 b9 64             	jmp    0x720e
     d55:	00 99 f9 d8          	add    %bl,-0x2707(%bx,%di)
     d59:	a2 63 3e             	mov    %al,0x3e63
     d5c:	0e                   	push   %cs
     d5d:	ff 05                	incw   (%di)
     d5f:	76 05                	jbe    0xd66
     d61:	b8 fb ff             	mov    $0xfffb,%ax
     d64:	eb 0d                	jmp    0xd73
     d66:	0e                   	push   %cs
     d67:	ff 02                	incw   (%bp,%si)
     d69:	75 05                	jne    0xd70
     d6b:	b8 e2 ff             	mov    $0xffe2,%ax
     d6e:	eb 03                	jmp    0xd73
     d70:	b8 f1 e8             	mov    $0xe8f1,%ax
     d73:	14 e9                	adc    $0xe9,%al
     d75:	b9 65 8b             	mov    $0x8b65,%cx
     d78:	5e                   	pop    %si
     d79:	f6 c6 87             	test   $0x87,%dh
     d7c:	85 25                	test   %sp,(%di)
     d7e:	3a c6                	cmp    %dh,%al
     d80:	87 87 52 ff          	xchg   %ax,-0xae(%bx)
     d84:	b0 14                	mov    $0x14,%al
     d86:	f6 af b8 00          	imulb  0xb8(%bx)
     d8a:	03 87 74 52          	add    0x5274(%bx),%ax
     d8e:	89 87 70 52          	mov    %ax,0x5270(%bx)
     d92:	c7 87 72 25 3a 00    	movw   $0x3a,0x2572(%bx)
     d98:	c7 46 f8 3a 00       	movw   $0x3a,-0x8(%bp)
     d9d:	c7 46 fa 24 19       	movw   $0x1924,-0x6(%bp)
     da2:	81 7e fa 14 1a       	cmpw   $0x1a14,-0x6(%bp)
     da7:	72 e9                	jb     0xd92
     da9:	00 b8 33 04          	add    %bh,0x433(%bx,%si)
     dad:	f7 fa                	idiv   %dx
     daf:	8a 07                	mov    (%bx),%al
     db1:	2a e4                	sub    %ah,%ah
     db3:	3b 96 03 75          	cmp    0x7503(%bp),%dx
     db7:	78 8b                	js     0xd44
     db9:	d8 d1                	fcom   %st(1)
     dbb:	e3 8b                	jcxz   0xd48
     dbd:	87 22                	xchg   %sp,(%bp,%si)
     dbf:	52                   	push   %dx
     dc0:	a3 b6 01             	mov    %ax,0x1b6
     dc3:	04 f7                	add    $0xf7,%al
     dc5:	fa                   	cli
     dc6:	8a 47 fe             	mov    -0x2(%bx),%al
     dc9:	e4 b8                	in     $0xb8,%al
     dcb:	00 e8                	add    %ch,%al
     dcd:	69 59 8b 46 fa       	imul   $0xfa46,-0x75(%bx,%di),%bx
     dd2:	2d 0c 00             	sub    $0xc,%ax
     dd5:	a3 bc 01             	mov    %ax,0x1bc
     dd8:	00 0d                	add    %cl,(%di)
     dda:	b4 01                	mov    $0x1,%ah
     ddc:	b8 60 ac             	mov    $0xac60,%ax
     ddf:	00 e8                	add    %ch,%al
     de1:	4f                   	dec    %di
     de2:	90                   	nop
     de3:	5b                   	pop    %bx
     de4:	5b                   	pop    %bx
     de5:	c7 46 fe b6 01       	movw   $0x1b6,-0x2(%bp)
     dea:	eb 12                	jmp    0xdfe
     dec:	04 f7                	add    $0xf7,%al
     dee:	fe                   	(bad)
     def:	ff 37                	push   (%bx)
     df1:	b8 60 ac             	mov    $0xac60,%ax
     df4:	00 e8                	add    %ch,%al
     df6:	fa                   	cli
     df7:	8f                   	(bad)
     df8:	5b                   	pop    %bx
     df9:	5b                   	pop    %bx
     dfa:	83 46 fe 02          	addw   $0x2,-0x2(%bp)
     dfe:	81 7e fe c0 01       	cmpw   $0x1c0,-0x2(%bp)
     e03:	72 e7                	jb     0xdec
     e05:	b8 60 ac             	mov    $0xac60,%ax
     e08:	00 e8                	add    %ch,%al
     e0a:	c8 4b e8 0a          	enter  $0xe84b,$0xa
     e0e:	0f e8 0f             	psubsb (%bx),%mm1
     e11:	10 ff                	adc    %bh,%bh
     e13:	04 76                	add    $0x76,%al
     e15:	e8 a8 f8             	call   0x6c0
     e18:	b8 ff 7e             	mov    $0x7eff,%ax
     e1b:	20 66 4f             	and    %ah,0x4f(%bp)
     e1e:	a2 0b e6             	mov    %al,0xe60b
     e21:	b8 17 e8             	mov    $0xe817,%ax
     e24:	a8 f8                	test   $0xf8,%al
     e26:	e8 ed 39             	call   0x4816
     e29:	8a 96 03 20          	mov    0x2003(%bp),%dl
     e2d:	66 1a 3b             	data32 sbb (%bp,%di),%bh
     e30:	83 46 fa 14          	addw   $0x14,-0x6(%bp)
     e34:	ff 46 f8             	incw   -0x8(%bp)
     e37:	e9 68 ff             	jmp    0xda2
     e3a:	8b f6                	mov    %si,%si
     e3c:	87 25                	xchg   %sp,(%di)
     e3e:	c6 87 ff 01 c6       	movb   $0xc6,0x1ff(%bx)
     e43:	87 87 52 00          	xchg   %ax,0x52(%bx)
     e47:	83 7e 03 76          	cmpw   $0x76,0x3(%bp)
     e4b:	ff 05                	incw   (%di)
     e4d:	b8 eb eb             	mov    $0xebeb,%ax
     e50:	8b 0e 83 7e          	mov    0x7e83,%cx
     e54:	03 02                	add    (%bp,%si),%ax
     e56:	75 05                	jne    0xe5d
     e58:	b8 1e eb             	mov    $0xeb1e,%ax
     e5b:	8b 03                	mov    (%bp,%di),%ax
     e5d:	b8 fb fb             	mov    $0xfbfb,%ax
     e60:	eb e8                	jmp    0xe4a
     e62:	cc                   	int3
     e63:	64 8b f6             	fs mov %si,%si
     e66:	87 80 bf 87          	xchg   %ax,-0x7841(%bx,%si)
     e6a:	52                   	push   %dx
     e6b:	00 75 33             	add    %dh,0x33(%di)
     e6e:	5e                   	pop    %si
     e6f:	33 e3                	xor    %bx,%sp
     e71:	d1 8b 22 a3          	rorw   $1,-0x5cde(%bp,%di)
     e75:	01 f7                	add    %si,%di
     e77:	8a c8                	mov    %al,%cl
     e79:	01 00                	add    %ax,(%bx,%si)
     e7b:	0d c6 01             	or     $0x1c6,%ax
     e7e:	60                   	pusha
     e7f:	00 4f 5b             	add    %cl,0x5b(%bx)
     e82:	c7                   	(bad)
     e83:	ad                   	lods   %ds:(%si),%ax
     e84:	8f                   	(bad)
     e85:	5b                   	pop    %bx
     e86:	5b                   	pop    %bx
     e87:	be c8 01             	mov    $0x1c8,%si
     e8a:	eb 0d                	jmp    0xe99
     e8c:	ff 34                	push   (%si)
     e8e:	60                   	pusha
     e8f:	00 4f 5b             	add    %cl,0x5b(%bx)
     e92:	c7                   	(bad)
     e93:	5d                   	pop    %bp
     e94:	8f                   	(bad)
     e95:	5b                   	pop    %bx
     e96:	5b                   	pop    %bx
     e97:	46                   	inc    %si
     e98:	46                   	inc    %si
     e99:	81 fe cc 01          	cmp    $0x1cc,%si
     e9d:	72 ed                	jb     0xe8c
     e9f:	eb 37                	jmp    0xed8
     ea1:	ff 04                	incw   (%si)
     ea3:	76 e8                	jbe    0xe8d
     ea5:	78 45                	js     0xeec
     ea7:	5e                   	pop    %si
     ea8:	e3 8b                	jcxz   0xe35
     eaa:	a3 f7 c8             	mov    %ax,0xc8f7
     ead:	00 c6                	add    %al,%dh
     eaf:	60                   	pusha
     eb0:	4f                   	dec    %di
     eb1:	c2 01 00             	ret    $0x1
     eb4:	0d c0 01             	or     $0x1c0,%ax
     eb7:	60                   	pusha
     eb8:	00 4f 5b             	add    %cl,0x5b(%bx)
     ebb:	c7                   	(bad)
     ebc:	74 8f                	je     0xe4d
     ebe:	5b                   	pop    %bx
     ebf:	5b                   	pop    %bx
     ec0:	be c2 01             	mov    $0x1c2,%si
     ec3:	eb ff                	jmp    0xec4
     ec5:	60                   	pusha
     ec6:	4f                   	dec    %di
     ec7:	c7                   	(bad)
     ec8:	8f                   	(bad)
     ec9:	5b                   	pop    %bx
     eca:	46                   	inc    %si
     ecb:	fe                   	(bad)
     ecc:	24 8f                	and    $0x8f,%al
     ece:	5b                   	pop    %bx
     ecf:	46                   	inc    %si
     ed0:	fe 01                	incb   (%bx,%di)
     ed2:	ed                   	in     (%dx),%ax
     ed3:	37                   	aaa
     ed4:	c6 01 72             	movb   $0x72,(%bx,%di)
     ed7:	ed                   	in     (%dx),%ax
     ed8:	c7                   	(bad)
     ed9:	8f                   	(bad)
     eda:	5b                   	pop    %bx
     edb:	46                   	inc    %si
     edc:	fe                   	(bad)
     edd:	f5                   	cmc
     ede:	4a                   	dec    %dx
     edf:	5e                   	pop    %si
     ee0:	5d                   	pop    %bp
     ee1:	1a 57 f6             	sbb    -0xa(%bx),%dl
     ee4:	55                   	push   %bp
     ee5:	8b 83 57 a3          	mov    -0x5ca9(%bp,%di),%ax
     ee9:	1a c6                	sbb    %dh,%al
     eeb:	06                   	push   %es
     eec:	e9 3e 00             	jmp    0xf2d
     eef:	e8 2e 4e             	call   0x5d20
     ef2:	e8 b6 f1             	call   0xab
     ef5:	e8 f9 f4             	call   0x3f1
     ef8:	e8 a2 75             	call   0x849d
     efb:	fe                   	(bad)
     efc:	24 f4                	and    $0xf4,%al
     efe:	ff 04                	incw   (%si)
     f00:	76 e8                	jbe    0xeea
     f02:	30 8d 5b 1a          	xor    %cl,0x1a5b(%di)
     f06:	f6 8b 57 1a 06       	testb  $0x6,0x1a57(%bp,%di)
     f0b:	00 be 83 08          	add    %bh,0x883(%bp)
     f0f:	56                   	push   %si
     f10:	ff 14                	call   *(%si)
     f12:	ff 76 0a             	push   0xa(%bp)
     f15:	ff 76 08             	push   0x8(%bp)
     f18:	8d 46 ec             	lea    -0x14(%bp),%ax
     f1b:	46                   	inc    %si
     f1c:	fe                   	(bad)
     f1d:	a3 09 0d             	mov    %ax,0xd09
     f20:	bf 48 50             	mov    $0x5048,%di
     f23:	5e                   	pop    %si
     f24:	e3 f2                	jcxz   0xf18
     f26:	f6 47 13 80          	testb  $0x80,0x13(%bx)
     f2a:	74 42                	je     0xf6e
     f2c:	ff 76 08             	push   0x8(%bp)
     f2f:	8d 46 f8             	lea    -0x8(%bp),%ax
     f32:	16                   	push   %ss
     f33:	46                   	inc    %si
     f34:	fe                   	(bad)
     f35:	3b 02                	cmp    (%bp,%si),%ax
     f37:	83 7e f8 00          	cmpw   $0x0,-0x8(%bp)
     f3b:	b4 f4                	mov    $0xf4,%ah
     f3d:	83 7e fa 01          	cmpw   $0x1,-0x6(%bp)
     f41:	74 2b                	je     0xf6e
     f43:	8b 46 f8             	mov    -0x8(%bp),%ax
     f46:	89 46 f0             	mov    %ax,-0x10(%bp)
     f49:	8b 8c 02 89          	mov    -0x76fe(%si),%cx
     f4d:	46                   	inc    %si
     f4e:	ee                   	out    %al,(%dx)
     f4f:	89 46 f6             	mov    %ax,-0xa(%bp)
     f52:	ee                   	out    %al,(%dx)
     f53:	06                   	push   %es
     f54:	08 89 f4 28          	or     %cl,0x28f4(%bx,%di)
     f58:	5e                   	pop    %si
     f59:	e3 08                	jcxz   0xf63
     f5b:	ff 37                	push   (%bx)
     f5d:	ff 04                	incw   (%si)
     f5f:	f6 50 5e             	notb   0x5e(%bx,%si)
     f62:	e3 f2                	jcxz   0xf56
     f64:	e6 02                	out    %al,$0x2
     f66:	44                   	inc    %sp
     f67:	2d 04 fb             	sub    $0xfb04,%ax
     f6a:	e8 64 6e             	call   0x7dd1
     f6d:	f2 59                	repnz pop %cx
     f6f:	04 57                	add    $0x57,%al
     f71:	4b                   	dec    %bx
     f72:	07                   	pop    %es
     f73:	2b c0                	sub    %ax,%ax
     f75:	1a f6                	sbb    %dh,%dh
     f77:	8b 57 08             	mov    0x8(%bx),%dx
     f7a:	00 83 56 14          	add    %al,0x1456(%bp,%di)
     f7e:	76 ff                	jbe    0xf7f
     f80:	4a                   	dec    %dx
     f81:	57                   	push   %di
     f82:	c7                   	(bad)
     f83:	f6 00 f4             	testb  $0xf4,(%bx,%si)
     f86:	06                   	push   %es
     f87:	00 2b                	add    %ch,(%bp,%di)
     f89:	ff 89 7e f8          	decw   -0x782(%bx,%di)
     f8d:	89 7e fe             	mov    %di,-0x2(%bp)
     f90:	83 3e 26 9b 02       	cmpw   $0x2,-0x64da
     f95:	75 bf                	jne    0xf56
     f97:	00 01                	add    %al,(%bx,%di)
     f99:	00 06 33 07          	add    %al,0x733
     f9d:	50                   	push   %ax
     f9e:	5e                   	pop    %si
     f9f:	e3 06                	jcxz   0xfa7
     fa1:	e3 de                	jcxz   0xf81
     fa3:	c8 00 9e 02          	enter  $0x9e00,$0x2
     fa7:	89 fa                	mov    %di,%dx
     fa9:	32 50 8d             	xor    -0x73(%bx,%si),%dl
     fac:	46                   	inc    %si
     fad:	e2 e8                	loop   0xf97
     faf:	3c e9                	cmp    $0xe9,%al
     fb1:	8f 83 c4 06          	pop    0x6c4(%bp,%di)
     fb5:	c6 46 ea 00          	movb   $0x0,-0x16(%bp)
     fb9:	06                   	push   %es
     fba:	7e 02                	jle    0xfbe
     fbc:	50                   	push   %ax
     fbd:	46                   	inc    %si
     fbe:	e8 e9 83             	call   0x93aa
     fc1:	06                   	push   %es
     fc2:	2e 8e 5b 46          	mov    %cs:0x46(%bp,%di),%ds
     fc6:	89 7e fc             	mov    %di,-0x4(%bp)
     fc9:	8b 76 fe             	mov    -0x2(%bp),%si
     fcc:	e9 9c 00             	jmp    0x106b
     fcf:	46                   	inc    %si
     fd0:	e8 b6 64             	call   0x7489
     fd3:	b8 5e f9             	mov    $0xf95e,%ax
     fd6:	8d 4e e2             	lea    -0x1e(%bp),%cx
     fd9:	51                   	push   %cx
     fda:	e8 cc 8f             	call   0x9fa9
     fdd:	83 c4 06             	add    $0x6,%sp
     fe0:	0d bf 48             	or     $0x48bf,%ax
     fe3:	2c 83                	sub    $0x83,%al
     fe5:	3e 98                	ds cbtw
     fe7:	06                   	push   %es
     fe8:	00 74 28             	add    %dh,0x28(%si)
     feb:	3e 0e                	ds push %cs
     fed:	96                   	xchg   %ax,%si
     fee:	06                   	push   %es
     fef:	00 74 21             	add    %dh,0x21(%si)
     ff2:	e8 7e f3             	call   0x373
     ff5:	34 01                	xor    $0x1,%al
     ff7:	ff 50 68             	call   *0x68(%bx,%si)
     ffa:	f3 46                	repz inc %si
     ffc:	b6 b8                	mov    $0xb8,%dh
     ffe:	f9                   	stc
     fff:	4e                   	dec    %si
    1000:	51                   	push   %cx
    1001:	cc                   	int3
    1002:	83 06 bf 2c 3e       	addw   $0x3e,0x2cbf
    1007:	a0 8f c4             	mov    0xc48f,%al
    100a:	0d 48 83             	or     $0x8348,%ax
    100d:	98                   	cbtw
    100e:	75 bf                	jne    0xfcf
    1010:	be 04 57             	mov    $0x5704,%si
    1013:	0b f6                	or     %si,%si
    1015:	75 54                	jne    0x106b
    1017:	39 36 98 06          	cmp    %si,0x698
    101b:	74 40                	je     0x105d
    101d:	b8 c6 02             	mov    $0x2c6,%ax
    1020:	50                   	push   %ax
    1021:	c7                   	(bad)
    1022:	5b                   	pop    %bx
    1023:	fe 4a 5d             	decb   0x5d(%bp,%si)
    1026:	0a 8e 5b 46          	or     0x465b(%bp),%cl
    102a:	8b fc                	mov    %sp,%di
    102c:	8a 8b 7e 06          	mov    0x67e(%bp,%di),%cl
    1030:	e3 e7                	jcxz   0x1019
    1032:	8a 99 b2 02          	mov    0x2b2(%bx,%di),%bl
    1036:	ff                   	ljmp   (bad)
    1037:	e8 44 b2             	call   0xc27e
    103a:	e8 fa 98             	call   0xa937
    103d:	02 60 4f             	add    0x4f(%bx,%si),%ah
    1040:	c7                   	(bad)
    1041:	8f                   	(bad)
    1042:	5b                   	pop    %bx
    1043:	c8 8d 5b 46          	enter  $0x5b8d,$0x46
    1047:	b8 d5 02             	mov    $0x2d5,%ax
    104a:	c7                   	(bad)
    104b:	fe                   	(bad)
    104c:	5d                   	pop    %bp
    104d:	8e 46 fc             	mov    -0x4(%bp),%es
    1050:	a0 8d 5b             	mov    0x5b8d,%al
    1053:	46                   	inc    %si
    1054:	b8 5d 8e             	mov    $0x8e5d,%ax
    1057:	46                   	inc    %si
    1058:	fc                   	cld
    1059:	79 49                	jns    0x10a4
    105b:	eb b8                	jmp    0x1015
    105d:	01 f0                	add    %si,%ax
    105f:	02 c7                	add    %bh,%al
    1061:	e8 70 49             	call   0x59d4
    1064:	59                   	pop    %cx
    1065:	33 33                	xor    (%bp,%di),%si
    1067:	c7                   	(bad)
    1068:	e8 7c fe             	call   0xee7
    106b:	0b f6                	or     %si,%si
    106d:	75 03                	jne    0x1072
    106f:	e9 5d ff             	jmp    0xfcf
    1072:	46                   	inc    %si
    1073:	b8 4e cc             	mov    $0xcc4e,%ax
    1076:	06                   	push   %es
    1077:	2c a0                	sub    $0xa0,%al
    1079:	c4 48 98             	les    -0x68(%bx,%si),%cx
    107c:	bf 04 29             	mov    $0x2904,%di
    107f:	8f c4                	pop    %sp
    1081:	0d 48 83             	or     $0x8348,%ax
    1084:	98                   	cbtw
    1085:	00 03                	add    %al,(%bp,%di)
    1087:	e9 cd 00             	jmp    0x1157
    108a:	8b 04                	mov    (%si),%ax
    108c:	05 8b 46             	add    $0x468b,%ax
    108f:	d0 3d                	sarb   $1,(%di)
    1091:	00 0a                	add    %cl,(%bp,%si)
    1093:	76 e6                	jbe    0x107b
    1095:	00 c3                	add    %al,%bl
    1097:	0a 89 fa fe          	or     -0x106(%bx,%di),%cl
    109b:	c7                   	(bad)
    109c:	e8 c5 63             	call   0x7464
    109f:	89 45 02             	mov    %ax,0x2(%di)
    10a2:	89 fa                	mov    %di,%dx
    10a4:	fc                   	cld
    10a5:	83 98 75 0d ff       	sbbw   $0xffff,0xd75(%bx,%si)
    10aa:	fa                   	cli
    10ab:	76 b8                	jbe    0x1065
    10ad:	20 02                	and    %al,(%bp,%si)
    10af:	c7                   	(bad)
    10b0:	e8 e7 49             	call   0x5a9a
    10b3:	e9 a1 00             	jmp    0x1157
    10b6:	46                   	inc    %si
    10b7:	a3 fe 89             	mov    %ax,0x89fe
    10ba:	45                   	inc    %bp
    10bb:	04 46                	add    $0x46,%al
    10bd:	b8 f6 cc             	mov    $0xccf6,%ax
    10c0:	2c c4                	sub    $0xc4,%al
    10c2:	98                   	cbtw
    10c3:	04 8f                	add    $0x8f,%al
    10c5:	0d 83 00             	or     $0x83,%ax
    10c8:	dc 8d c4 48          	fmull  0x48c4(%di)
    10cc:	98                   	cbtw
    10cd:	03 cd                	add    %bp,%cx
    10cf:	8b 05                	mov    (%di),%ax
    10d1:	46                   	inc    %si
    10d2:	83 00 e8             	addw   $0xffe8,(%bx,%si)
    10d5:	64 f4                	fs hlt
    10d7:	50                   	push   %ax
    10d8:	ff                   	(bad)
    10d9:	fa                   	cli
    10da:	fe                   	(bad)
    10db:	1e                   	push   %ds
    10dc:	ff                   	(bad)
    10dd:	fc                   	cld
    10de:	90                   	nop
    10df:	ff f6                	push   %si
    10e1:	8b 5e d9             	mov    -0x27(%bp),%bx
    10e4:	8d                   	lea    (bad),%ax
    10e5:	c4 48 0a             	les    0xa(%bx,%si),%cx
    10e8:	76 8d                	jbe    0x1077
    10ea:	8b 5e a2             	mov    -0x5e(%bp),%bx
    10ed:	8e 5b 06             	mov    0x6(%bp,%di),%ds
    10f0:	07                   	pop    %es
    10f1:	5e                   	pop    %si
    10f2:	06                   	push   %es
    10f3:	ff                   	(bad)
    10f4:	fa                   	cli
    10f5:	76 46                	jbe    0x113d
    10f7:	e9 06 8e             	jmp    0x9f00
    10fa:	46                   	inc    %si
    10fb:	f1                   	int1
    10fc:	8d                   	lea    (bad),%ax
    10fd:	c4 48 98             	les    -0x68(%bx,%si),%cx
    1100:	c6                   	(bad)
    1101:	ea 06 02 46 78       	ljmp   $0x7846,$0x206
    1106:	02 46 e9             	add    -0x17(%bp),%al
    1109:	06                   	push   %es
    110a:	8e 46 7e             	mov    0x7e(%bp),%es
    110d:	e3 8c                	jcxz   0x109b
    110f:	5b                   	pop    %bx
    1110:	46                   	inc    %si
    1111:	46                   	inc    %si
    1112:	f6 2c                	imulb  (%si)
    1114:	98                   	cbtw
    1115:	8f 83 dc c4          	pop    -0x3b24(%bp,%di)
    1119:	98                   	cbtw
    111a:	cd 05                	int    $0x5
    111c:	83 87 8d 48 03       	addw   $0x3,0x488d(%bx)
    1121:	8b 46 00             	mov    0x0(%bp),%ax
    1124:	75 31                	jne    0x1157
    1126:	8b f6                	mov    %si,%si
    1128:	06                   	push   %es
    1129:	89 05                	mov    %ax,(%di)
    112b:	e8 64 f4             	call   0x592
    112e:	50                   	push   %ax
    112f:	59                   	pop    %cx
    1130:	33 33                	xor    (%bp,%di),%si
    1132:	c7                   	(bad)
    1133:	e8 64 f0             	call   0x19a
    1136:	16                   	push   %ss
    1137:	06                   	push   %es
    1138:	ff                   	(bad)
    1139:	fa                   	cli
    113a:	8b 5e 80             	mov    -0x80(%bp),%bx
    113d:	8d                   	lea    (bad),%ax
    113e:	c4 48 0a             	les    0xa(%bx,%si),%cx
    1141:	a1 94 02             	mov    0x294,%ax
    1144:	8b 16 96 02          	mov    0x296,%dx
    1148:	39 64 f0             	cmp    %sp,-0x10(%si)
    114b:	75 0a                	jne    0x1157
    114d:	39 56 f2             	cmp    %dx,-0xe(%bp)
    1150:	75 05                	jne    0x1157
    1152:	46                   	inc    %si
    1153:	32 f8                	xor    %al,%bh
    1155:	ff 83 20 7e          	incw   0x7e20(%bp,%di)
    1159:	f8                   	clc
    115a:	ff 74 0a             	push   0xa(%si)
    115d:	76 8d                	jbe    0x10ec
    115f:	76 b8                	jbe    0x1119
    1161:	16                   	push   %ss
    1162:	03 b8 33 5e          	add    0x5e33(%bx,%si),%di
    1166:	49                   	dec    %cx
    1167:	8b f8                	mov    %ax,%di
    1169:	46                   	inc    %si
    116a:	5e                   	pop    %si
    116b:	1a f6                	sbb    %dh,%dh
    116d:	8b 57 1a             	mov    0x1a(%bx),%dx
    1170:	89 56 56             	mov    %dx,0x56(%bp)
    1173:	a1 3e 83             	mov    0x833e,%ax
    1176:	ec                   	in     (%dx),%al
    1177:	02 e8                	add    %al,%ch
    1179:	64 fe c7             	fs inc %bh
    117c:	b9 89 56             	mov    $0x5689,%cx
    117f:	51                   	push   %cx
    1180:	ff 06 cf ff          	incw   -0x31
    1184:	76 30                	jbe    0x11b6
    1186:	5e                   	pop    %si
    1187:	08 37                	or     %dh,(%bx)
    1189:	04 50                	add    $0x50,%al
    118b:	e8 8d 1a             	call   0x2c1b
    118e:	c4 48 0a             	les    0xa(%bx,%si),%cx
    1191:	20 7e fe             	and    %bh,-0x2(%bp)
    1194:	04 74                	add    $0x74,%al
    1196:	0c c4                	or     $0xc4,%al
    1198:	5e                   	pop    %si
    1199:	04 8f                	add    $0x8f,%al
    119b:	83 26 89 47 02       	andw   $0x2,0x4789
    11a0:	26 89 07             	mov    %ax,%es:(%bx)
    11a3:	f6 8b 57 1a 06       	testb  $0x6,0x1a57(%bp,%di)
    11a8:	8b 08                	mov    (%bx,%si),%cx
    11aa:	8c 2b                	mov    %gs,(%bp,%di)
    11ac:	8c 5e 04             	mov    %ds,0x4(%bp)
    11af:	04 50                	add    $0x50,%al
    11b1:	e8 dc 8d             	call   0x9f90
    11b4:	46                   	inc    %si
    11b5:	fc                   	cld
    11b6:	8b 04                	mov    (%si),%ax
    11b8:	04 77                	add    $0x77,%al
    11ba:	02 e8                	add    %al,%ch
    11bc:	f9                   	stc
    11bd:	62 1a                	bound  %bx,(%bp,%si)
    11bf:	f6 8b 57 1a 06       	testb  $0x6,0x1a57(%bp,%di)
    11c4:	3e e8 4e 83          	ds call 0x9516
    11c8:	ec                   	in     (%dx),%al
    11c9:	0e                   	push   %cs
    11ca:	57                   	push   %di
    11cb:	8b 04                	mov    (%si),%ax
    11cd:	05 08 57             	add    $0x5708,%ax
    11d0:	ff cf                	dec    %di
    11d2:	76 5e                	jbe    0x1232
    11d4:	37                   	aaa
    11d5:	50                   	push   %ax
    11d6:	e8 e9 06             	call   0x18c2
    11d9:	40                   	inc    %ax
    11da:	74 05                	je     0x11e1
    11dc:	8f 83 e9 8a          	pop    -0x7517(%bp,%di)
    11e0:	89 76 8a             	mov    %si,-0x76(%bp)
    11e3:	04 a1                	add    $0xa1,%al
    11e5:	c8 c0 89 44          	enter  $0x89c0,$0x44
    11e9:	02 fa                	add    %dl,%bh
    11eb:	83 f8 8b             	cmp    $0xff8b,%ax
    11ee:	5c                   	pop    %sp
    11ef:	06                   	push   %es
    11f0:	f6 13                	notb   (%bp,%di)
    11f2:	74 ff                	je     0x11f3
    11f4:	08 46 ff             	or     %al,-0x1(%bp)
    11f7:	cf                   	iret
    11f8:	76 e8                	jbe    0x11e2
    11fa:	64 f2 16             	fs repnz push %ss
    11fd:	fe 02                	incb   (%bp,%si)
    11ff:	71 ff                	jno    0x1200
    1201:	8b f8                	mov    %ax,%di
    1203:	f2 89 44 03          	repnz mov %ax,0x3(%si)
    1207:	66 3b 59 62          	cmp    0x62(%bx,%di),%ebx
    120b:	89 08                	mov    %cx,(%bx,%si)
    120d:	f4                   	hlt
    120e:	5e                   	pop    %si
    120f:	f6 83 3c 03 fe       	testb  $0xfe,0x33c(%bp,%di)
    1214:	0a 57 b8             	or     -0x48(%bx),%dl
    1217:	02 e8                	add    %al,%ch
    1219:	49                   	dec    %cx
    121a:	a1 7d 48             	mov    0x487d,%ax
    121d:	eb bd                	jmp    0x11dc
    121f:	8b 5c 06             	mov    0x6(%si),%bx
    1222:	e6 44                	out    %al,$0x44
    1224:	04 e8                	add    $0xe8,%al
    1226:	6e                   	outsb  %ds:(%si),(%dx)
    1227:	59                   	pop    %cx
    1228:	89 32                	mov    %si,(%bp,%si)
    122a:	8d 5e 06             	lea    0x6(%bp),%bx
    122d:	de ff                	fdivrp %st,%st(7)
    122f:	ff f6                	push   %si
    1231:	5e                   	pop    %si
    1232:	f8                   	clc
    1233:	ff 04                	incw   (%si)
    1235:	50                   	push   %ax
    1236:	e3 e8                	jcxz   0x1220
    1238:	a2 ef eb             	mov    %al,0xebef
    123b:	2c e6                	sub    $0xe6,%al
    123d:	44                   	inc    %sp
    123e:	04 89                	add    $0x89,%al
    1240:	44                   	inc    %sp
    1241:	03 66 3b             	add    0x3b(%bp),%sp
    1244:	1e                   	push   %ds
    1245:	62 08                	bound  %cx,(%bx,%si)
    1247:	5e                   	pop    %si
    1248:	83 03 0a             	addw   $0xa,(%bp,%di)
    124b:	b8 e8 a1             	mov    $0xa1e8,%ax
    124e:	74 c5                	je     0x1215
    1250:	e8 64 fa             	call   0xcb7
    1253:	da 03                	fiaddl (%bp,%di)
    1255:	e8 76 a8             	call   0xbace
    1258:	b8 7e ff             	mov    $0xff7e,%ax
    125b:	04 50                	add    $0x50,%al
    125d:	5e                   	pop    %si
    125e:	06                   	push   %es
    125f:	de ff                	fdivrp %st,%st(7)
    1261:	ff                   	(bad)
    1262:	3b 59 8c             	cmp    -0x74(%bx,%di),%bx
    1265:	c4 48 0a             	les    0xa(%bx,%si),%cx
    1268:	e6 ff                	out    %al,$0xff
    126a:	5a                   	pop    %dx
    126b:	c2 1a 8b             	ret    $0x8b1a
    126e:	1a 56 a1             	sbb    -0x5f(%bp),%dl
    1271:	06                   	push   %es
    1272:	8b 08                	mov    (%bx,%si),%cx
    1274:	8c 2b                	mov    %gs,(%bp,%di)
    1276:	83 ec 18             	sub    $0x18,%sp
    1279:	57                   	push   %di
    127a:	8b 04                	mov    (%si),%ax
    127c:	05 76 5e             	add    $0x5e76,%ax
    127f:	37                   	aaa
    1280:	08 57 e8             	or     %dl,-0x18(%bx)
    1283:	64 e8 66 3b          	fs call 0x4ded
    1287:	39 06 74 8f          	cmp    %ax,-0x708c
    128b:	e9 89 8a             	jmp    0x9d17
    128e:	a1 0f 01             	mov    0x10f,%ax
    1291:	76 8a                	jbe    0x121d
    1293:	04 e8                	add    $0xe8,%al
    1295:	64 fc                	fs cld
    1297:	c7                   	(bad)
    1298:	0a eb                	or     %bl,%ch
    129a:	8b 50 1e             	mov    0x1e(%bx,%si),%dx
    129d:	56                   	push   %si
    129e:	ff 35                	push   (%di)
    12a0:	e8 1b 8c             	call   0x9ebe
    12a3:	c4 48 0a             	les    0xa(%bx,%si),%cx
    12a6:	5e                   	pop    %si
    12a7:	06                   	push   %es
    12a8:	ee                   	out    %al,(%dx)
    12a9:	f6 13                	notb   (%bp,%di)
    12ab:	74 ff                	je     0x12ac
    12ad:	08 0e 57 e8          	or     %cl,-0x17a9
    12b1:	f0 06                	lock push %es
    12b3:	fa                   	cli
    12b4:	5e                   	pop    %si
    12b5:	e8 ba fe             	call   0x1172
    12b8:	8b f8                	mov    %ax,%di
    12ba:	f0 eb 06             	lock jmp 0x12c3
    12bd:	e6 44                	out    %al,$0x44
    12bf:	04 e8                	add    $0xe8,%al
    12c1:	eb 8b                	jmp    0x124e
    12c3:	83 03 2b             	addw   $0x2b,(%bp,%di)
    12c6:	83 3e 9b 75 04       	cmpw   $0x4,0x759b
    12cb:	75 44                	jne    0x1311
    12cd:	8a 4c 07             	mov    0x7(%si),%cl
    12d0:	2a ed                	sub    %ch,%ch
    12d2:	8b 04                	mov    (%si),%ax
    12d4:	05 07 89             	add    $0x8907,%ax
    12d7:	76 d1                	jbe    0x12aa
    12d9:	b1 03                	mov    $0x3,%cl
    12db:	d3 e8                	shr    %cl,%ax
    12dd:	8b ca                	mov    %dx,%cx
    12df:	f7 e1                	mul    %cx
    12e1:	b9 05 00             	mov    $0x5,%cx
    12e4:	f7 e1                	mul    %cx
    12e6:	05 11 00             	add    $0x11,%ax
    12e9:	b1 04                	mov    $0x4,%cl
    12eb:	d3 e8                	shr    %cl,%ax
    12ed:	89 fe                	mov    %di,%si
    12ef:	e8 c7 44             	call   0x57b9
    12f2:	0a 02                	or     (%bp,%si),%al
    12f4:	00 ff                	add    %bh,%bh
    12f6:	fe                   	(bad)
    12f7:	ff                   	ljmp   (bad)
    12f8:	e8 43 73             	call   0x863e
    12fb:	89 44 0c             	mov    %ax,0xc(%si)
    12fe:	80 7c 0d ff          	cmpb   $0xff,0xd(%si)
    1302:	75 ff                	jne    0x1303
    1304:	76 20                	jbe    0x1326
    1306:	08 b8 30 e8          	or     %bh,-0x17d0(%bx,%si)
    130a:	49                   	dec    %cx
    130b:	a1 8c 47             	mov    0x478c,%ax
    130e:	e9 7b ff             	jmp    0x128c
    1311:	f6 f4                	div    %ah
    1313:	00 02                	add    %al,(%bp,%si)
    1315:	00 b1 04 8b          	add    %dh,-0x74fc(%bx,%di)
    1319:	f8                   	clc
    131a:	ec                   	in     (%dx),%al
    131b:	05 11 00             	add    $0x11,%ax
    131e:	d3 e8                	shr    %cl,%ax
    1320:	20 1a                	and    %bl,(%bp,%si)
    1322:	83 73 89 03          	xorw   $0x3,-0x77(%bp,%di)
    1326:	0a b8 3e 9b          	or     -0x64c2(%bx,%si),%bh
    132a:	75 04                	jne    0x1330
    132c:	74 0c                	je     0x133a
    132e:	8b f8                	mov    %ax,%di
    1330:	f4                   	hlt
    1331:	89 44 0a             	mov    %ax,0xa(%si)
    1334:	5e                   	pop    %si
    1335:	f6 89 e8 44 0c       	testb  $0xc,0x44e8(%bx,%di)
    133a:	80 7e f7 ff          	cmpb   $0xff,-0x9(%bp)
    133e:	75 11                	jne    0x1351
    1340:	76 08                	jbe    0x134a
    1342:	30 49 8c             	xor    %cl,-0x74(%bx,%di)
    1345:	e9 ff f4             	jmp    0x847
    1348:	50                   	push   %ax
    1349:	47                   	inc    %di
    134a:	56                   	push   %si
    134b:	e8 52 73             	call   0x86a0
    134e:	e9 3b ff             	jmp    0x128c
    1351:	f6 25                	mulb   (%di)
    1353:	ee                   	out    %al,(%dx)
    1354:	13 ff                	adc    %di,%di
    1356:	0e                   	push   %cs
    1357:	e8 06 17             	call   0x2a60
    135a:	e6 44                	out    %al,$0x44
    135c:	04 e8                	add    $0xe8,%al
    135e:	0e                   	push   %cs
    135f:	8b 83 03 fc          	mov    -0x3fd(%bp,%di),%ax
    1363:	ff 35                	push   (%di)
    1365:	ff 50 06             	call   *0x6(%bx,%si)
    1368:	ff f4                	push   %sp
    136a:	f3 ff f4             	repz push %sp
    136d:	6d                   	insw   (%dx),%es:(%di)
    136e:	ee                   	out    %al,(%dx)
    136f:	eb 15                	jmp    0x1386
    1371:	e8 64 fc             	call   0xfd8
    1374:	c7                   	(bad)
    1375:	ff f4                	push   %sp
    1377:	ec                   	in     (%dx),%al
    1378:	ff 06 f4 ff          	incw   -0xc
    137c:	6d                   	insw   (%dx),%es:(%di)
    137d:	eb ff                	jmp    0x137e
    137f:	35 e8 3b             	xor    $0x3be8,%ax
    1382:	8b c4                	mov    %sp,%ax
    1384:	0a 7e 04             	or     0x4(%bp),%bh
    1387:	3e 9b                	ds fwait
    1389:	75 04                	jne    0x138f
    138b:	75 10                	jne    0x139d
    138d:	56                   	push   %si
    138e:	ff 06 f4 ff          	incw   -0xc
    1392:	6d                   	insw   (%dx),%es:(%di)
    1393:	eb e8                	jmp    0x137d
    1395:	35 71 ff             	xor    $0xff71,%ax
    1398:	8b 80 c4 ef          	mov    -0x103c(%bx,%si),%ax
    139c:	72 e6                	jb     0x1384
    139e:	5a                   	pop    %dx
    139f:	1a 1a                	sbb    (%bp,%si),%bl
    13a1:	a1 8b 8c             	mov    0x8c8b,%ax
    13a4:	83 18 8b             	sbbw   $0xff8b,(%bx,%si)
    13a7:	05 5e 08             	add    $0x85e,%ax
    13aa:	e8 e8 3b             	call   0x4f95
    13ad:	02 57 04             	add    0x4(%bx),%dl
    13b0:	76 37                	jbe    0x13e9
    13b2:	b1 03                	mov    $0x3,%cl
    13b4:	8b d3                	mov    %bx,%dx
    13b6:	89 f6                	mov    %si,%si
    13b8:	83 bf 54 02 00       	cmpw   $0x0,0x254(%bx)
    13bd:	33 06 33 04          	xor    0x433,%ax
    13c1:	4b                   	dec    %bx
    13c2:	2b 2d                	sub    (%di),%bp
    13c4:	57                   	push   %di
    13c5:	cf                   	iret
    13c6:	5e                   	pop    %si
    13c7:	04 e8                	add    $0xe8,%al
    13c9:	00 04                	add    %al,(%si)
    13cb:	89 fe                	mov    %di,%si
    13cd:	e8 b8 9b             	call   0xaf88
    13d0:	04 0c                	add    $0xc,%al
    13d2:	f8                   	clc
    13d3:	89 19                	mov    %bx,(%bx,%di)
    13d5:	2b bb 52 54          	sub    0x5452(%bp,%di),%di
    13d9:	02 81 fb 7c          	add    0x7cfb(%bx,%di),%al
    13dd:	02 73 0e             	add    0xe(%bp,%di),%dh
    13e0:	3b d7                	cmp    %di,%dx
    13e2:	74 04                	je     0x13e8
    13e4:	c7 07 00 00          	movw   $0x0,(%bx)
    13e8:	c3                   	ret
    13e9:	81 08 42 eb          	orw    $0xeb42,(%bx,%si)
    13ed:	8c 06 33 fe          	mov    %es,-0x1cd
    13f1:	1a 8b 1a 56          	sbb    0x561a(%bp,%di),%cl
    13f5:	a1 83 02             	mov    0x283,%ax
    13f8:	64 c7                	fs (bad)
    13fa:	89 51 06             	mov    %dx,0x6(%bx,%di)
    13fd:	26 57                	es push %di
    13ff:	04 76                	add    $0x76,%al
    1401:	56                   	push   %si
    1402:	06                   	push   %es
    1403:	c7                   	(bad)
    1404:	fc                   	cld
    1405:	00 85 e9 8b          	add    %al,-0x7417(%di)
    1409:	fa                   	cli
    140a:	b1 03                	mov    $0x3,%cl
    140c:	89 fe                	mov    %di,%si
    140e:	3e 8b d3             	ds mov %bx,%dx
    1411:	89 f6                	mov    %si,%si
    1413:	8a 87 52 02          	mov    0x252(%bx),%al
    1417:	2a 8b f7 f8          	sub    -0x709(%bp,%di),%cl
    141b:	04 fe                	add    $0xfe,%al
    141d:	37                   	aaa
    141e:	89 f6                	mov    %si,%si
    1420:	88 97 52 02          	mov    %dl,0x252(%bx)
    1424:	c7 87 54 02 00 00    	movw   $0x0,0x254(%bx)
    142a:	39 fe                	cmp    %di,%si
    142c:	3e 75 db             	jne,pt 0x140a
    142f:	5e                   	pop    %si
    1430:	de ff                	fdivrp %st,%st(7)
    1432:	d3 89 f6 7e          	rorw   %cl,0x7ef6(%bx,%di)
    1436:	da ff                	(bad)
    1438:	b7 50                	mov    $0x50,%bh
    143a:	02 cf                	add    %bh,%cl
    143c:	5e                   	pop    %si
    143d:	04 e8                	add    $0xe8,%al
    143f:	64 ea 49 7d eb 04    	fs ljmp $0x4eb,$0x7d49
    1445:	83 75 be 57          	xorw   $0x57,-0x42(%di)
    1449:	e9 e4 01             	jmp    0x1630
    144c:	f7 8a da 8b b7 56    	testw  $0x56b7,-0x7426(%bp,%si)
    1452:	02 64 c7             	add    -0x39(%si),%ah
    1455:	fa                   	cli
    1456:	c7                   	(bad)
    1457:	eb 50                	jmp    0x14a9
    1459:	56                   	push   %si
    145a:	35 64 c7             	xor    $0xc764,%ax
    145d:	dc 16 50 e3          	fcoml  -0x1cb0
    1461:	e3 04                	jcxz   0x1467
    1463:	50                   	push   %ax
    1464:	dc 46 56             	faddl  0x56(%bp)
    1467:	8a c4                	mov    %ah,%al
    1469:	0a 06 f6 74          	or     0x74f6,%al
    146d:	f0 13 0e 06 e6       	lock adc -0x19fa,%cx
    1472:	04 10                	add    $0x10,%al
    1474:	04 e8                	add    $0xe8,%al
    1476:	f8                   	clc
    1477:	e8 f2 fe             	call   0x136c
    147a:	71 8b                	jno    0x1407
    147c:	f2 f3 fc             	repnz repz cld
    147f:	8b f2                	mov    %dx,%si
    1481:	44                   	inc    %sp
    1482:	eb e6                	jmp    0x146a
    1484:	04 eb                	add    $0xeb,%al
    1486:	83 2b 3e             	subw   $0x3e,(%bp,%di)
    1489:	75 75                	jne    0x1500
    148b:	8a ee                	mov    %dh,%ch
    148d:	8a 46 e2             	mov    -0x1e(%bp),%al
    1490:	2a 8b 89 03          	sub    0x389(%bp,%di),%cl
    1494:	3b 8a 46 89          	cmp    -0x76ba(%bp,%si),%cx
    1498:	f6 44 e6 04          	testb  $0x4,-0x1a(%si)
    149c:	46                   	inc    %si
    149d:	89 f6                	mov    %si,%si
    149f:	06                   	push   %es
    14a0:	fc                   	cld
    14a1:	04 9b                	add    $0x9b,%al
    14a3:	04 10                	add    $0x10,%al
    14a5:	ff f4                	push   %sp
    14a7:	05 b1 02             	add    $0x2b1,%ax
    14aa:	d3 6c 04             	shrw   %cl,0x4(%si)
    14ad:	3e 00 26 02 bf       	add    %ah,%ds:-0x40fe
    14b2:	01 0b                	add    %cx,(%bp,%di)
    14b4:	d1 64 04             	shlw   $1,0x4(%si)
    14b7:	f8                   	clc
    14b8:	05 ee d1             	add    $0xd1ee,%ax
    14bb:	e0 f6                	loopne 0x14b3
    14bd:	06                   	push   %es
    14be:	fc                   	cld
    14bf:	b1 8b                	mov    $0x8b,%cl
    14c1:	ec                   	in     (%dx),%al
    14c2:	11 fc                	adc    %di,%sp
    14c4:	05 0f 00             	add    $0xf,%ax
    14c7:	e8 1a f6             	call   0xae4
    14ca:	06                   	push   %es
    14cb:	fc                   	cld
    14cc:	06                   	push   %es
    14cd:	33 fe                	xor    %si,%di
    14cf:	39 46 06             	cmp    %ax,0x6(%bp)
    14d2:	75 31                	jne    0x1505
    14d4:	b8 04 f8             	mov    $0xf804,%ax
    14d7:	19 bb 54 05          	sbb    %di,0x554(%bp,%di)
    14db:	a1 c6 c0             	mov    0xc0c6,%ax
    14de:	eb e6                	jmp    0x14c6
    14e0:	ff                   	(bad)
    14e1:	fc                   	cld
    14e2:	90                   	nop
    14e3:	3d 58 71             	cmp    $0x7158,%ax
    14e6:	83 2b 3e             	subw   $0x3e,(%bp,%di)
    14e9:	89 02                	mov    %ax,(%bp,%si)
    14eb:	83 80 7c 03 ff       	addw   $0xffff,0x37c(%bx,%si)
    14f0:	75 1c                	jne    0x150e
    14f2:	f7 8a da ff b7 50    	testw  $0x50b7,-0x26(%bp,%si)
    14f8:	02 49 e9             	add    -0x17(%bx,%di),%cl
    14fb:	f4                   	hlt
    14fc:	47                   	inc    %di
    14fd:	e8 9a 45             	call   0x5a9a
    1500:	89 a1 01 2e          	mov    %sp,0x2e01(%bx,%di)
    1504:	01 a1 c6 c0          	add    %sp,-0x3f3a(%bx,%di)
    1508:	83 3e 02 80 03       	cmpw   $0x3,-0x7ffe
    150d:	75 89                	jne    0x1498
    150f:	a1 83 3e             	mov    0x3e83,%ax
    1512:	ea 89 04 00 02       	ljmp   $0x200,$0x489
    1517:	01 d1                	add    %dx,%cx
    1519:	04 05                	add    $0x5,%al
    151b:	13 f8                	adc    %ax,%di
    151d:	05 ee 48             	add    $0x48ee,%ax
    1520:	01 3e ea f8          	add    %di,-0x716
    1524:	05 ea 89             	add    $0x89ea,%ax
    1527:	46                   	inc    %si
    1528:	e6 8b                	out    %al,$0x8b
    152a:	ec                   	in     (%dx),%al
    152b:	11 89 46 e8          	adc    %cx,-0x17ba(%bx,%di)
    152f:	f8                   	clc
    1530:	05 ea 89             	add    $0x89ea,%ax
    1533:	46                   	inc    %si
    1534:	f6 06 89 11 89       	testb  $0x89,0x1189
    1539:	46                   	inc    %si
    153a:	f8                   	clc
    153b:	fe 89 f0 0e          	decb   0xef0(%bx,%di)
    153f:	e6 10                	out    %al,$0x10
    1541:	e8 e8 1a             	call   0x302c
    1544:	e6 04                	out    %al,$0x4
    1546:	0e                   	push   %cs
    1547:	83 fc 35             	cmp    $0x35,%sp
    154a:	50                   	push   %ax
    154b:	ff 8d 06 ff          	decw   -0xfa(%di)
    154f:	04 50                	add    $0x50,%al
    1551:	dc f6                	fdiv   %st,%st(6)
    1553:	f8                   	clc
    1554:	04 e3                	add    $0xe3,%al
    1556:	a2 eb e6             	mov    %al,0xe6eb
    1559:	04 80                	add    $0x80,%al
    155b:	ec                   	in     (%dx),%al
    155c:	eb 18                	jmp    0x1576
    155e:	e8 fa 03             	call   0x195b
    1561:	76 b8                	jbe    0x151b
    1563:	76 ee                	jbe    0x1553
    1565:	a8 7e                	test   $0x7e,%al
    1567:	04 5e                	add    $0x5e,%al
    1569:	de ff                	fdivrp %st,%st(7)
    156b:	59                   	pop    %cx
    156c:	c4 04                	les    (%si),%ax
    156e:	dc 56 c4             	fcoml  -0x3c(%bp)
    1571:	4b                   	dec    %bx
    1572:	89 c4                	mov    %ax,%sp
    1574:	7e 3e                	jle    0x15b4
    1576:	75 75                	jne    0x15ed
    1578:	56                   	push   %si
    1579:	06                   	push   %es
    157a:	04 05                	add    $0x5,%al
    157c:	08 46 16             	or     %al,0x16(%bp)
    157f:	dc 50 56             	fcoml  0x56(%bx,%si)
    1582:	e8 22 73             	call   0x88a7
    1585:	b8 f8 bb             	mov    $0xbbf8,%ax
    1588:	05 c6 eb             	add    $0xebc6,%ax
    158b:	03 e9                	add    %cx,%bp
    158d:	94                   	xchg   %ax,%sp
    158e:	00 46 24             	add    %al,0x24(%bp)
    1591:	e6 00                	out    %al,$0x0
    1593:	00 8a 4e e3          	add    %cl,-0x1cb2(%bp,%si)
    1597:	2a 8b 05 16          	sub    0x1605(%bp,%di),%cl
    159b:	dc 05                	faddl  (%di)
    159d:	89 d1                	mov    %dx,%cx
    159f:	03 e8                	add    %ax,%bp
    15a1:	ca e1 05             	lret   $0x5e1
    15a4:	f7 05 00 04          	testw  $0x400,(%di)
    15a8:	e8 05 03             	call   0x18b0
    15ab:	00 b9 00 e1          	add    %bh,-0x1f00(%bx,%di)
    15af:	11 b1 b1 d3          	adc    %si,-0x2c4f(%bx,%di)
    15b3:	89 e8                	mov    %bp,%ax
    15b5:	e9 85 e9             	jmp    0xff3d
    15b8:	70 89                	jo     0x1543
    15ba:	46                   	inc    %si
    15bb:	e8 80 7e             	call   0x943e
    15be:	e9 ff 75             	jmp    0x8bc0
    15c1:	17                   	pop    %ss
    15c2:	f7 da                	neg    %dx
    15c4:	b7 02                	mov    $0x2,%bh
    15c6:	e9 47 9a             	jmp    0xb010
    15c9:	89 01                	mov    %ax,(%bx,%di)
    15cb:	01 c6                	add    %ax,%si
    15cd:	83 ca 44             	or     $0x44,%dx
    15d0:	ff 74 02             	push   0x2(%si)
    15d3:	e8 b6 70             	call   0x868c
    15d6:	e9 27 ff             	jmp    0x1500
    15d9:	8b 04                	mov    (%si),%ax
    15db:	25 f7 64             	and    $0x64f7,%ax
    15de:	06                   	push   %es
    15df:	29 06 d8 9a          	sub    %ax,-0x6528
    15e3:	81 3e d8 9a 00 40    	cmpw   $0x4000,-0x6528
    15e9:	73 0d                	jae    0x15f8
    15eb:	f7 b7 e9 9a          	divw   -0x6517(%bx)
    15ef:	01 c6                	add    %ax,%si
    15f1:	ca ff 40             	lret   $0x40ff
    15f4:	02 e9                	add    %cl,%ch
    15f6:	04 dc                	add    $0xdc,%al
    15f8:	a1 d8 9a             	mov    0x9ad8,%ax
    15fb:	89 04                	mov    %ax,(%si)
    15fd:	c7 03 75 00          	movw   $0x75,(%bp,%di)
    1601:	a0 46 16             	mov    0x1646,%al
    1604:	dc 50 a8             	fcoml  -0x58(%bx,%si)
    1607:	04 de                	add    $0xde,%al
    1609:	59                   	pop    %cx
    160a:	04 56                	add    $0x56,%al
    160c:	e8 bd 6e             	call   0x84cc
    160f:	46                   	inc    %si
    1610:	dc 56 22             	fcoml  0x22(%bp)
    1613:	b8 bb 1a             	mov    $0x1abb,%ax
    1616:	73 59                	jae    0x1671
    1618:	04 e8                	add    $0xe8,%al
    161a:	e8 6f 70             	call   0x868c
    161d:	e8 f8 ed             	call   0x418
    1620:	8a 69 70             	mov    0x70(%bx,%di),%ch
    1623:	33 4b 2d             	xor    0x2d(%bp,%di),%cx
    1626:	f7 b7 e9 89          	divw   -0x7617(%bx)
    162a:	87 02                	xchg   %ax,(%bp,%si)
    162c:	00 f6                	add    %dh,%dh
    162e:	fc                   	cld
    162f:	33 39                	xor    (%bx,%di),%di
    1631:	06                   	push   %es
    1632:	fc                   	cld
    1633:	5e                   	pop    %si
    1634:	f6 57 89             	notb   -0x77(%bx)
    1637:	56                   	push   %si
    1638:	3e ec                	ds in  (%dx),%al
    163a:	e8 fe b9             	call   0xd03b
    163d:	56                   	push   %si
    163e:	ff cf                	dec    %di
    1640:	1e                   	push   %ds
    1641:	56                   	push   %si
    1642:	2b f4                	sub    %sp,%si
    1644:	6d                   	insw   (%dx),%es:(%di)
    1645:	e8 08 46             	call   0x5c50
    1648:	16                   	push   %ss
    1649:	8e 7e 8c             	mov    -0x74(%bp),%?
    164c:	e4 87                	in     $0x87,%al
    164e:	5b                   	pop    %bx
    164f:	b8 8e 84             	mov    $0x848e,%ax
    1652:	02 e9                	add    %cl,%ch
    1654:	8e 7e 8c             	mov    -0x74(%bp),%?
    1657:	46                   	inc    %si
    1658:	f6 97 87 5b          	notb   0x5b87(%bx)
    165c:	b8 8e 7e             	mov    $0x7e8e,%ax
    165f:	8c 46 ff             	mov    %es,-0x1(%bp)
    1662:	76 64                	jbe    0x16c8
    1664:	16                   	push   %ss
    1665:	02 f8                	add    %al,%bh
    1667:	66 59                	pop    %ecx
    1669:	89 fb                	mov    %di,%bx
    166b:	83 be e9 01 8a       	cmpw   $0xff8a,0x1e9(%bp)
    1670:	80 00 64             	addb   $0x64,(%bx,%si)
    1673:	50                   	push   %ax
    1674:	f8                   	clc
    1675:	66 8b f8             	mov    %eax,%edi
    1678:	ed                   	in     (%dx),%ax
    1679:	b8 bb 7e             	mov    $0x7ebb,%ax
    167c:	00 a3 ce c0          	add    %ah,-0x3f32(%bp,%di)
    1680:	03 8b 46 5e          	add    0x5e46(%bp,%di),%cx
    1684:	6d                   	insw   (%dx),%es:(%di)
    1685:	08 16 7e e4          	or     %dl,-0x1b82
    1689:	5b                   	pop    %bx
    168a:	8e 02                	mov    (%bp,%si),%es
    168c:	a4                   	movsb  %ds:(%si),%es:(%di)
    168d:	87 5b b8             	xchg   %bx,-0x48(%bp,%di)
    1690:	8e 8c 02 8e          	mov    -0x71fe(%si),%cs
    1694:	8c f6                	mov    %?,%si
    1696:	87 b8 7e 57          	xchg   %di,0x577e(%bx,%si)
    169a:	87 b8 7e 46          	xchg   %di,0x467e(%bx,%si)
    169e:	76 16                	jbe    0x16b6
    16a0:	f8                   	clc
    16a1:	59                   	pop    %cx
    16a2:	fb                   	sti
    16a3:	be 01 80             	mov    $0x8001,%si
    16a6:	f0 50                	lock push %ax
    16a8:	17                   	pop    %ss
    16a9:	89 02                	mov    %ax,(%bp,%si)
    16ab:	03 8b 46 23          	add    0x2346(%bp,%di),%cx
    16af:	01 80 f0 50          	add    %ax,0x50f0(%bx,%si)
    16b3:	59                   	pop    %cx
    16b4:	fb                   	sti
    16b5:	be 56 06             	mov    $0x656,%si
    16b8:	ff 04                	incw   (%si)
    16ba:	e8 f7 00             	call   0x17b4
    16bd:	c0 1a 08             	rcrb   $0x8,(%bp,%si)
    16c0:	11 46 76             	adc    %ax,0x76(%bp)
    16c3:	16                   	push   %ss
    16c4:	f8                   	clc
    16c5:	89 01                	mov    %ax,(%bx,%di)
    16c7:	01 c6                	add    %ax,%si
    16c9:	83 ce 43             	or     $0x43,%si
    16cc:	b8 bb e7             	mov    $0xe7bb,%ax
    16cf:	5d                   	pop    %bp
    16d0:	2b f4                	sub    %sp,%si
    16d2:	04 04                	add    $0x4,%al
    16d4:	ff 05                	incw   (%di)
    16d6:	02 6c 19             	add    0x19(%si),%ch
    16d9:	ff 36 ca c0          	push   -0x3f36
    16dd:	e8 d7 5d             	call   0x74b7
    16e0:	eb 10                	jmp    0x16f2
    16e2:	e8 e8 08             	call   0x1fcd
    16e5:	b8 29 03             	mov    $0x329,%ax
    16e8:	33 49 ae             	xor    -0x52(%bx,%di),%cx
    16eb:	43                   	inc    %bx
    16ec:	b8 bb c7             	mov    $0xc7bb,%ax
    16ef:	5d                   	pop    %bp
    16f0:	2b f4                	sub    %sp,%si
    16f2:	8b c6                	mov    %si,%ax
    16f4:	5e                   	pop    %si
    16f5:	8b 83 8b 5e          	mov    0x5e8b(%bp,%di),%ax
    16f9:	e8 3b 57             	call   0x6e37
    16fc:	76 b1                	jbe    0x16af
    16fe:	8b 89 0a 56          	mov    0x560a(%bx,%di),%cx
    1702:	7e c7                	jle    0x16cb
    1704:	fa                   	cli
    1705:	19 3a                	sbb    %di,(%bp,%si)
    1707:	c7 f8 00 46          	xbegin 0x5d0b
    170b:	02 33                	add    (%bp,%di),%dh
    170d:	f7 8a e6 eb 2b 75    	testw  $0x752b,-0x141a(%bp,%si)
    1713:	b9 0e 00             	mov    $0xe,%cx
    1716:	d2 f1                	shl    %cl,%cl
    1718:	46                   	inc    %si
    1719:	3d 8b f0             	cmp    $0xf08b,%ax
    171c:	b8 bb c6             	mov    $0xc6bb,%ax
    171f:	03 94 46 e6          	add    -0x19ba(%si),%dx
    1723:	00 84 00 e0          	add    %al,-0x2000(%si)
    1727:	d1 33                	shlw   $1,(%bp,%di)
    1729:	49                   	dec    %cx
    172a:	38 5d a3             	cmp    %bl,-0x5d(%di)
    172d:	ca c0 c0             	lret   $0xc0c0
    1730:	1a 08                	sbb    (%bx,%si),%cl
    1732:	04 2b                	add    $0x2b,%al
    1734:	f4                   	hlt
    1735:	eb 73                	jmp    0x17aa
    1737:	89 76 f6             	mov    %si,-0xa(%bp)
    173a:	89 83 ea 04          	mov    %ax,0x4ea(%bp,%di)
    173e:	fe                   	(bad)
    173f:	f6 33                	divb   (%bp,%di)
    1741:	06                   	push   %es
    1742:	5e                   	pop    %si
    1743:	ff 04                	incw   (%si)
    1745:	eb 39                	jmp    0x1780
    1747:	8a 07                	mov    (%bx),%al
    1749:	ed                   	in     (%dx),%ax
    174a:	04 07                	add    $0x7,%al
    174c:	76 b1                	jbe    0x16ff
    174e:	d3 8b f7 b9          	rorw   %cl,-0x4609(%bp,%di)
    1752:	00 e1                	add    %ah,%cl
    1754:	11 b1 d3 89          	adc    %si,-0x762d(%bx,%di)
    1758:	e8 44 02             	call   0x199f
    175b:	ff                   	(bad)
    175c:	ff 43 89             	incw   -0x77(%bp,%di)
    175f:	0c f7                	or     $0xf7,%al
    1761:	04 37                	add    $0x37,%al
    1763:	1e                   	push   %ds
    1764:	ca c0 03             	lret   $0x3c0
    1767:	5e                   	pop    %si
    1768:	fc                   	cld
    1769:	89 07                	mov    %ax,(%bx)
    176b:	01 7e f8             	add    %di,-0x8(%bp)
    176e:	39 7e fa             	cmp    %di,-0x6(%bp)
    1771:	73 03                	jae    0x1776
    1773:	fe 8b fa 83          	decb   -0x7c06(%bp,%di)
    1777:	33 06 02 83          	xor    -0x7cfe,%ax
    177b:	c6                   	(bad)
    177c:	0e                   	push   %cs
    177d:	ff 04                	incw   (%si)
    177f:	fe 06 fe 46          	incb   0x46fe
    1783:	75 b8                	jne    0x173d
    1785:	f6 77 bf             	divb   -0x41(%bx)
    1788:	f7 8a 04 37 4e fa    	testw  $0xfa4e,0x3704(%bp,%si)
    178e:	8b c3                	mov    %bx,%ax
    1790:	d2 f1                	shl    %cl,%cl
    1792:	03 c1                	add    %cx,%ax
    1794:	13 d2                	adc    %dx,%dx
    1796:	0b d2                	or     %dx,%dx
    1798:	7c 07                	jl     0x17a1
    179a:	7f 97                	jg     0x1733
    179c:	3d 00 ff             	cmp    $0xff00,%ax
    179f:	77 92                	ja     0x1733
    17a1:	03 d9                	add    %cx,%bx
    17a3:	89 1e cc c0          	mov    %bx,-0x3f34
    17a7:	cd f6                	int    $0xf6
    17a9:	94                   	xchg   %ax,%sp
    17aa:	8b c6                	mov    %si,%ax
    17ac:	5e                   	pop    %si
    17ad:	f6 89 3e e8 b9       	testb  $0xb9,-0x17c2(%bx,%di)
    17b2:	ff 1e 2b 6d          	lcall  *0x6d2b
    17b6:	08 16 7e 0e          	or     %dl,0xe7e
    17ba:	8b 05                	mov    (%di),%ax
    17bc:	57                   	push   %di
    17bd:	76 ff                	jbe    0x17be
    17bf:	08 46 46             	or     %al,0x46(%bp)
    17c2:	a3 0d fe             	mov    %ax,0xfe0d
    17c5:	8b f3                	mov    %bx,%si
    17c7:	8b a8 f9 8b          	mov    -0x7407(%bx,%si),%bp
    17cb:	f2 44                	repnz inc %sp
    17cd:	66 59                	pop    %ecx
    17cf:	89 04                	mov    %ax,(%si)
    17d1:	ff 02                	incw   (%bp,%si)
    17d3:	19 36 c0 08          	sbb    %si,0x8c0
    17d7:	b1 ec                	mov    $0xec,%cl
    17d9:	fc                   	cld
    17da:	3e cc                	ds int3
    17dc:	c0 eb 04             	shr    $0x4,%bl
    17df:	b1 ec                	mov    $0xec,%cl
    17e1:	fc                   	cld
    17e2:	f8                   	clc
    17e3:	83 c7 11             	add    $0x11,%di
    17e6:	d3 ef                	shr    %cl,%di
    17e8:	b8 00 b8             	mov    $0xb800,%ax
    17eb:	08 83 0a e8          	or     %al,-0x17f6(%bp,%di)
    17ef:	74 57                	je     0x1848
    17f1:	e8 4a 6e             	call   0x863e
    17f4:	89 02                	mov    %ax,(%bp,%si)
    17f6:	83 8b 06 13 80       	orw    $0xff80,0x1306(%bp,%di)
    17fb:	03 75 f7             	add    -0x9(%di),%si
    17fe:	da e9                	fucompp
    1800:	8a 0f                	mov    (%bx),%cl
    1802:	76 b5                	jbe    0x17b9
    1804:	33 f7                	xor    %di,%si
    1806:	8a 37                	mov    (%bx),%dh
    1808:	50                   	push   %ax
    1809:	8d 06 fa 5e          	lea    0x5efa,%ax
    180d:	8d 8b 06 44          	lea    0x4406(%bp,%di),%cx
    1811:	e8 59 32             	call   0x4a6d
    1814:	5e                   	pop    %si
    1815:	de ff                	fdivrp %st,%st(7)
    1817:	f3 8b c1             	repz mov %cx,%ax
    181a:	e9 2b 89             	jmp    0xa148
    181d:	f8                   	clc
    181e:	7e fe                	jle    0x181e
    1820:	eb 14                	jmp    0x1836
    1822:	8b 0a                	mov    (%bp,%si),%cx
    1824:	1e                   	push   %ds
    1825:	e0 03                	loopne 0x182a
    1827:	44                   	inc    %sp
    1828:	04 37                	add    $0x37,%al
    182a:	f6 57 76             	notb   0x76(%bx)
    182d:	06                   	push   %es
    182e:	89 40 0c             	mov    %ax,0xc(%bx,%si)
    1831:	83 fe 81             	cmp    $0xff81,%si
    1834:	0e                   	push   %cs
    1835:	47                   	inc    %di
    1836:	39 3e ce c0          	cmp    %di,-0x3f32
    183a:	7f e6                	jg     0x1822
    183c:	83 03 75             	addw   $0x75,(%bp,%di)
    183f:	00 74 1c             	add    %dh,0x1c(%si)
    1842:	2b 89 8b f7          	sub    -0x875(%bx,%di),%cx
    1846:	39 36 ce c0          	cmp    %si,-0x3f32
    184a:	7e 12                	jle    0x185e
    184c:	ff 30                	push   (%bx,%si)
    184e:	08 04                	or     %al,(%si)
    1850:	46                   	inc    %si
    1851:	06                   	push   %es
    1852:	03 c7                	add    %di,%ax
    1854:	f3 8b 7a 6d          	repz mov 0x6d(%bp,%si),%di
    1858:	83 c7 0e             	add    $0xe,%di
    185b:	46                   	inc    %si
    185c:	eb e8                	jmp    0x1846
    185e:	04 02                	add    $0x2,%al
    1860:	36 08 ec             	ss or  %ch,%ah
    1863:	3e 52                	ds push %dx
    1865:	fc                   	cld
    1866:	3e ce                	ds into
    1868:	c0 4f 8b c7          	rorb   $0xc7,-0x75(%bx)
    186c:	d1 f6                	shl    $1,%si
    186e:	fc                   	cld
    186f:	8b f6                	mov    %si,%si
    1871:	57                   	push   %di
    1872:	c7                   	(bad)
    1873:	b9 00 f1             	mov    $0xf100,%cx
    1876:	f7 e9                	imul   %cx
    1878:	8b f0                	mov    %ax,%si
    187a:	0b ff                	or     %di,%di
    187c:	7c 39                	jl     0x18b7
    187e:	f7 e6                	mul    %si
    1880:	2b b9 40 0a          	sub    0xa40(%bx,%di),%di
    1884:	fa                   	cli
    1885:	c7                   	(bad)
    1886:	8d 8b c3 37          	lea    0x37c3(%bp,%di),%cx
    188a:	ca 03 fc             	lret   $0xfc03
    188d:	07                   	pop    %es
    188e:	7e f6                	jle    0x1886
    1890:	57                   	push   %di
    1891:	0f 29 0e cc c0       	movaps %xmm1,-0x3f34
    1896:	cd d8                	int    $0xd8
    1898:	a1 cc c0             	mov    0xc0cc,%ax
    189b:	89 40 0a             	mov    %ax,0xa(%bx,%si)
    189e:	f6 e8                	imul   %al
    18a0:	06                   	push   %es
    18a1:	03 c6                	add    %si,%ax
    18a3:	50                   	push   %ax
    18a4:	04 59                	add    $0x59,%al
    18a6:	56                   	push   %si
    18a7:	bd 46 56             	mov    $0x5646,%bp
    18aa:	b8 1f 6c             	mov    $0x6c1f,%ax
    18ad:	83 6e fe 02          	subw   $0x2,-0x2(%bp)
    18b1:	83 ee 0e             	sub    $0xe,%si
    18b4:	4f                   	dec    %di
    18b5:	eb c3                	jmp    0x187a
    18b7:	e6 1a                	out    %al,$0x1a
    18b9:	a1 8c 18             	mov    0x188c,%ax
    18bc:	05 08 e8             	add    $0xe808,%ax
    18bf:	02 08                	add    (%bx,%si),%cl
    18c1:	83 14 ff             	adcw   $0xffff,(%si)
    18c4:	57                   	push   %di
    18c5:	f6 f4                	div    %ah
    18c7:	04 8b                	add    $0x8b,%al
    18c9:	57                   	push   %di
    18ca:	ff e6                	jmp    *%si
    18cc:	2b b9 77 02          	sub    0x277(%bx,%di),%di
    18d0:	c7 00 e9 fa          	movw   $0xfae9,(%bx,%si)
    18d4:	03 fe                	add    %si,%di
    18d6:	2b b9 00 14          	sub    0x1400(%bx,%di),%di
    18da:	00 f1                	add    %dh,%cl
    18dc:	3d f0 bb             	cmp    $0xbbf0,%ax
    18df:	48                   	dec    %ax
    18e0:	fc                   	cld
    18e1:	8b f6                	mov    %si,%si
    18e3:	0b db                	or     %bx,%bx
    18e5:	08 2b                	or     %ch,(%bp,%di)
    18e7:	eb c0                	jmp    0x18a9
    18e9:	eb 3f                	jmp    0x192a
    18eb:	04 08                	add    $0x8,%al
    18ed:	ff 76 56             	push   0x56(%bp)
    18f0:	8f                   	(bad)
    18f1:	dc 98 ff ff          	fcompl -0x1(%bx,%si)
    18f5:	43                   	inc    %bx
    18f6:	89 39                	mov    %di,(%bx,%di)
    18f8:	03 fe                	add    %si,%di
    18fa:	f0 0b f6             	lock or %si,%si
    18fd:	74 20                	je     0x191f
    18ff:	8f                   	(bad)
    1900:	dc 98 ff 74          	fcompl 0x74ff(%bx,%si)
    1904:	02 ff                	add    %bh,%bh
    1906:	34 5e                	xor    $0x5e,%al
    1908:	de ff                	fdivrp %st,%st(7)
    190a:	59                   	pop    %cx
    190b:	c4 0a                	les    (%bp,%si),%cx
    190d:	07                   	pop    %es
    190e:	86 83 c4 08          	xchg   %al,0x8c4(%bp,%di)
    1912:	04 8b                	add    $0x8b,%al
    1914:	8b 89 77 06          	mov    0x677(%bx,%di),%cx
    1918:	fc                   	cld
    1919:	85 8b 04 2b          	test   %cx,0x2b04(%bp,%di)
    191d:	57                   	push   %di
    191e:	ff 76 b8             	push   -0x48(%bp)
    1921:	3d 03 49             	cmp    $0x4903,%ax
    1924:	43                   	inc    %bx
    1925:	73 41                	jae    0x1968
    1927:	39 fc                	cmp    %di,%sp
    1929:	f6 89 3e e8 b9       	testb  $0xb9,-0x17c2(%bx,%di)
    192e:	ff 1e 8b 5e          	lcall  *0x5e8b
    1932:	e8 3b 57             	call   0x7070
    1935:	76 b1                	jbe    0x18e8
    1937:	8b 89 05 76          	mov    0x7605(%bx,%di),%cx
    193b:	08 06 03 30          	or     %al,0x3003
    193f:	08 d1                	or     %dl,%cl
    1941:	ee                   	out    %al,(%dx)
    1942:	8b c6                	mov    %si,%ax
    1944:	00 14                	add    %dl,(%si)
    1946:	00 f7                	add    %dh,%bh
    1948:	e9 03 46             	jmp    0x5f4e
    194b:	08 fc                	or     %bh,%ah
    194d:	8b f6                	mov    %si,%si
    194f:	05 8b 5e             	add    $0x5e8b,%ax
    1952:	98                   	cbtw
    1953:	ff                   	(bad)
    1954:	ff 0a                	decw   (%bp,%si)
    1956:	e8 0b 85             	call   0x9e64
    1959:	5b                   	pop    %bx
    195a:	8b 8a f8 0b          	mov    0xbf8(%bp,%si),%cx
    195e:	ff                   	(bad)
    195f:	7d 08                	jge    0x1969
    1961:	8d 44 ff             	lea    -0x1(%si),%ax
    1964:	fc                   	cld
    1965:	8b 04                	mov    (%si),%ax
    1967:	eb 06                	jmp    0x196f
    1969:	8d 44 01             	lea    0x1(%si),%ax
    196c:	fc                   	cld
    196d:	8b 06 0b ff          	mov    -0xf5,%ax
    1971:	74 08                	je     0x197b
    1973:	04 8b                	add    $0x8b,%al
    1975:	04 39                	add    $0x39,%al
    1977:	06                   	push   %es
    1978:	31 76 bf             	xor    %si,-0x41(%bp)
    197b:	0b ff                	or     %di,%di
    197d:	74 05                	je     0x1984
    197f:	32 ff                	xor    %bh,%bh
    1981:	fe                   	(bad)
    1982:	fa                   	cli
    1983:	03 fe                	add    %si,%di
    1985:	ff                   	(bad)
    1986:	fe 8c 05 e8          	decb   -0x17fb(%si)
    198a:	08 14                	or     %dl,(%si)
    198c:	57                   	push   %di
    198d:	f4                   	hlt
    198e:	8b ff                	mov    %di,%di
    1990:	2b 77 59             	sub    0x59(%bx),%si
    1993:	04 56                	add    $0x56,%al
    1995:	4b                   	dec    %bx
    1996:	c4 3e 1d 5b          	les    0x5b1d,%di
    199a:	f6 57 06             	notb   0x6(%bx)
    199d:	be 08 ff             	mov    $0xff08,%si
    19a0:	e8 3b 57             	call   0x70de
    19a3:	76 03                	jbe    0x19a8
    19a5:	00 1c                	add    %bl,(%si)
    19a7:	89 0c                	mov    %cx,(%si)
    19a9:	06                   	push   %es
    19aa:	04 f7                	add    $0xf7,%al
    19ac:	c0 f1 6c             	shl    $0x6c,%cl
    19af:	06                   	push   %es
    19b0:	04 f7                	add    $0xf7,%al
    19b2:	c0 02 5b             	rolb   $0x5b,(%bp,%si)
    19b5:	f6 57 06             	notb   0x6(%bx)
    19b8:	be 08 ff             	mov    $0xff08,%si
    19bb:	ff 0a                	decw   (%bp,%si)
    19bd:	76 8d                	jbe    0x194c
    19bf:	ec                   	in     (%dx),%al
    19c0:	fe 04                	incb   (%si)
    19c2:	8b 57 2b             	mov    0x2b(%bx),%dx
    19c5:	f8                   	clc
    19c6:	fe                   	(bad)
    19c7:	14 fc                	adc    $0xfc,%al
    19c9:	b8 ca 05             	mov    $0x5ca,%ax
    19cc:	e8 02 e8             	call   0x1d1
    19cf:	63 4e e8             	arpl   %cx,-0x18(%bp)
    19d2:	40                   	inc    %ax
    19d3:	3c e8                	cmp    $0xe8,%al
    19d5:	87 00                	xchg   %ax,(%bx,%si)
    19d7:	eb 79                	jmp    0x1a52
    19d9:	b8 ca 05             	mov    $0x5ca,%ax
    19dc:	e8 57 e8             	call   0x236
    19df:	e7 3a                	out    %ax,$0x3a
    19e1:	8a f8                	mov    %al,%bh
    19e3:	5e                   	pop    %si
    19e4:	ff 8b 74 65          	decw   0x6574(%bp,%di)
    19e8:	2d fb 8b             	sub    $0x8bfb,%ax
    19eb:	74 60                	je     0x1a4d
    19ed:	89 3e d3 f6          	mov    %di,-0x92d
    19f1:	c7                   	(bad)
    19f2:	b9 0c 43             	mov    $0x430c,%cx
    19f5:	0c 04                	or     $0x4,%al
    19f7:	1e                   	push   %ds
    19f8:	d8 8b 36 d4          	fmuls  -0x2bca(%bp,%di)
    19fc:	05 ff 30             	add    $0x30ff,%ax
    19ff:	b8 4c a6             	mov    $0xa64c,%ax
    1a02:	4e                   	dec    %si
    1a03:	e8 2c 84             	call   0x9e32
    1a06:	5b                   	pop    %bx
    1a07:	8e 02                	mov    (%bp,%si),%es
    1a09:	26 04 e8             	es add $0xe8,%al
    1a0c:	02 4c a6             	add    -0x5a(%si),%cl
    1a0f:	4e                   	dec    %si
    1a10:	e8 df 83             	call   0x9df2
    1a13:	5b                   	pop    %bx
    1a14:	8e 02                	mov    (%bp,%si),%es
    1a16:	dc 05                	faddl  (%di)
    1a18:	e8 c8 8b             	call   0xa5e3
    1a1b:	f9                   	stc
    1a1c:	51                   	push   %cx
    1a1d:	e8 f4 3b             	call   0x5614
    1a20:	8a 85 8e 04          	mov    0x48e(%di),%al
    1a24:	2a f7                	sub    %bh,%dh
    1a26:	04 f0                	add    $0xf0,%al
    1a28:	b9 dc 05             	mov    $0x5dc,%cx
    1a2b:	51                   	push   %cx
    1a2c:	e8 02 ff             	call   0x1931
    1a2f:	e8 e9 65             	call   0x801b
    1a32:	27                   	daa
    1a33:	3e 02 dc             	ds add %ah,%bl
    1a36:	05 e8 56             	add    $0x56e8,%ax
    1a39:	e8 8c 3a             	call   0x54c8
    1a3c:	8a f0                	mov    %al,%dh
    1a3e:	83 fe 04             	cmp    $0x4,%si
    1a41:	73 86                	jae    0x19c9
    1a43:	04 f6                	add    $0xf6,%al
    1a45:	76 88                	jbe    0x19cf
    1a47:	87 8e 04 e9          	xchg   %cx,-0x16fc(%bp)
    1a4b:	7c ff                	jl     0x1a4c
    1a4d:	f4                   	hlt
    1a4e:	02 8b 2b ff          	add    -0xd5(%bp,%di),%cl
    1a52:	76 03                	jbe    0x1a57
    1a54:	fc                   	cld
    1a55:	8b 74 81             	mov    -0x7f(%si),%si
    1a58:	5e                   	pop    %si
    1a59:	5d                   	pop    %bp
    1a5a:	1a 57 f6             	sbb    -0xa(%bx),%dl
    1a5d:	02 56 c7             	add    -0x39(%bp),%dl
    1a60:	19 06 69 9b          	sbb    %ax,-0x6497
    1a64:	0f 00                	(bad)
    1a66:	bf 66 00             	mov    $0x66,%di
    1a69:	2b f6                	sub    %si,%si
    1a6b:	8a 9c 8e 04          	mov    0x48e(%si),%bl
    1a6f:	2a e8                	sub    %al,%ch
    1a71:	b2 fa                	mov    $0xfa,%dl
    1a73:	02 4f 84             	add    -0x7c(%bx),%cl
    1a76:	04 b8                	add    $0xb8,%al
    1a78:	c3                   	ret
    1a79:	63 4e 57             	arpl   %cx,0x57(%bp)
    1a7c:	e8 1d 6f             	call   0x899c
    1a7f:	83 c7 0a             	add    $0xa,%di
    1a82:	46                   	inc    %si
    1a83:	83 fe 04             	cmp    $0x4,%si
    1a86:	72 e3                	jb     0x1a6b
    1a88:	5e                   	pop    %si
    1a89:	5d                   	pop    %bp
    1a8a:	ec                   	in     (%dx),%al
    1a8b:	c7 00 0a 56          	movw   $0x560a,(%bx,%si)
    1a8f:	7e 06                	jle    0x1a97
    1a91:	76 8f                	jbe    0x1a22
    1a93:	98                   	cbtw
    1a94:	ff 89 e8 bc          	decw   -0x4318(%bx,%di)
    1a98:	75 a1                	jne    0x1a3b
    1a9a:	3e 89 00             	mov    %ax,%ds:(%bx,%si)
    1a9d:	fa                   	cli
    1a9e:	f6 06 ff 50 e8       	testb  $0xe8,0x50ff
    1aa3:	c8 3f 01 51          	enter  $0x13f,$0x51
    1aa7:	b9 c7 f9             	mov    $0xf9c7,%cx
    1aaa:	51                   	push   %cx
    1aab:	e8 6d 4e             	call   0x691b
    1aae:	b8 a6 05             	mov    $0x5a6,%ax
    1ab1:	02 63 e8             	add    -0x18(%bp,%di),%ah
    1ab4:	3c 87                	cmp    $0x87,%al
    1ab6:	eb 5b                	jmp    0x1b13
    1ab8:	3b 83 3e ba          	cmp    -0x45c2(%bp,%di),%ax
    1abc:	00 00                	add    %al,(%bx,%si)
    1abe:	00 ff                	add    %bh,%bh
    1ac0:	e8 0b 02             	call   0x1cce
    1ac3:	b8 a6 05             	mov    $0x5a6,%ax
    1ac6:	02 98 ff 89          	add    -0x7601(%bx,%si),%bl
    1aca:	e8 fb 39             	call   0x54c8
    1acd:	3d 05 00             	cmp    $0x5,%ax
    1ad0:	76 03                	jbe    0x1ad5
    1ad2:	e9 04 01             	jmp    0x1bd9
    1ad5:	d1 e0                	shl    $1,%ax
    1ad7:	93                   	xchg   %ax,%bx
    1ad8:	2e ff a7 dd 1a       	jmp    *%cs:0x1add(%bx)
    1add:	e9 1a ef             	jmp    0x9fa
    1ae0:	1a f5                	sbb    %ch,%dh
    1ae2:	1a fb                	sbb    %bl,%bh
    1ae4:	1a 32                	sbb    (%bp,%si),%dh
    1ae6:	1b c9                	sbb    %cx,%cx
    1ae8:	1b e8                	sbb    %ax,%bp
    1aea:	41                   	inc    %cx
    1aeb:	5b                   	pop    %bx
    1aec:	e9 ea 00             	jmp    0x1bd9
    1aef:	e8 d0 5b             	call   0x76c2
    1af2:	e9 e4 00             	jmp    0x1bd9
    1af5:	e8 36 5a             	call   0x752e
    1af8:	e9 de 0d             	jmp    0x28d9
    1afb:	e8 0d a6             	call   0xc10b
    1afe:	05 e8 0d             	add    $0xde8,%ax
    1b01:	a8 05                	test   $0x5,%al
    1b03:	e8 0d aa             	call   0xc513
    1b06:	05 e8 0d             	add    $0xde8,%ax
    1b09:	ac                   	lods   %ds:(%si),%al
    1b0a:	05 e8 65             	add    $0x65e8,%ax
    1b0d:	4e                   	dec    %si
    1b0e:	b8 ee 05             	mov    $0x5ee,%ax
    1b11:	02 63 5d             	add    0x5d(%bp,%di),%ah
    1b14:	46                   	inc    %si
    1b15:	79 eb                	jns    0x1b02
    1b17:	19 83 8e dc          	sbb    %ax,-0x2372(%bp,%di)
    1b1b:	e8 6e 04             	call   0x1f8c
    1b1e:	02 a6 5d 46          	add    0x465d(%bp),%ah
    1b22:	79 eb                	jns    0x1b0f
    1b24:	cc                   	int3
    1b25:	82 5b b8 8e          	sbbb   $0x8e,-0x48(%bp,%di)
    1b29:	fc                   	cld
    1b2a:	49                   	dec    %cx
    1b2b:	b8 f0 a5             	mov    $0xa5f0,%ax
    1b2e:	3e e9 a7 00          	ds jmp 0x1bd9
    1b32:	b8 b8 05             	mov    $0x5b8,%ax
    1b35:	63 3c                	arpl   %di,(%si)
    1b37:	eb 3b                	jmp    0x1b74
    1b39:	3e 00 d7             	ds add %dl,%bh
    1b3c:	3a 8b 3e 8e          	cmp    -0x71c2(%bp,%di),%cl
    1b40:	04 81                	add    $0x81,%al
    1b42:	e7 ff                	out    %ax,$0xff
    1b44:	00 be 01 00          	add    %bh,0x1(%bp)
    1b48:	a0 8e 04             	mov    0x48e,%al
    1b4b:	38 84 8e 04          	cmp    %al,0x48e(%si)
    1b4f:	e8 bf ff             	call   0x1b11
    1b52:	04 00                	add    $0x0,%al
    1b54:	46                   	inc    %si
    1b55:	83 fe 04             	cmp    $0x4,%si
    1b58:	72 ee                	jb     0x1b48
    1b5a:	b8 b8 05             	mov    $0x5b8,%ax
    1b5d:	63 57 02             	arpl   %dx,0x2(%bx)
    1b60:	e8 65 3e             	call   0x59c8
    1b63:	dc f5                	fdiv   %st,%st(5)
    1b65:	3c 76                	cmp    $0x76,%al
    1b67:	fc                   	cld
    1b68:	74 5e                	je     0x1bc8
    1b6a:	75 6d                	jne    0x1bd9
    1b6c:	b8 b8 05             	mov    $0x5b8,%ax
    1b6f:	63 57 e8             	arpl   %dx,-0x18(%bx)
    1b72:	54                   	push   %sp
    1b73:	39 8b f8 3d          	cmp    %cx,0x3df8(%bp,%di)
    1b77:	ff 00                	incw   (%bx,%si)
    1b79:	74 1a                	je     0x1b95
    1b7b:	77 08                	ja     0x1b85
    1b7d:	2c 04                	sub    $0x4,%al
    1b7f:	e8 1b fe             	call   0x199d
    1b82:	74 2c                	je     0x1bb0
    1b84:	10 2b                	adc    %ch,(%bp,%di)
    1b86:	db eb                	fucomi %st(3),%st
    1b88:	07                   	pop    %es
    1b89:	8b 88 f3 87          	mov    -0x780d(%bx,%si),%cx
    1b8d:	04 7c                	add    $0x7c,%al
    1b8f:	43                   	inc    %bx
    1b90:	83 fb 04             	cmp    $0x4,%bx
    1b93:	72 f4                	jb     0x1b89
    1b95:	fc                   	cld
    1b96:	8b 2b                	mov    (%bp,%di),%bp
    1b98:	ff                   	(bad)
    1b99:	b8 03 ca             	mov    $0xca03,%ax
    1b9c:	e8 1d fe             	call   0x19bc
    1b9f:	b8 05 3c             	mov    $0x3c05,%ax
    1ba2:	3b 00                	cmp    (%bx,%si),%ax
    1ba4:	3a 3e 04 e7          	cmp    -0x18fc,%bh
    1ba8:	6a 3a                	push   $0x3a
    1baa:	3e 04 e7             	ds add $0xe7,%al
    1bad:	00 01                	add    %al,(%bx,%di)
    1baf:	a0 04 84             	mov    0x8404,%al
    1bb2:	04 bf                	add    $0xbf,%al
    1bb4:	04 46                	add    $0x46,%al
    1bb6:	fe                   	(bad)
    1bb7:	72 b8                	jb     0x1b71
    1bb9:	05 57 e8             	add    $0xe857,%ax
    1bbc:	3e f5                	ds cmc
    1bbe:	76 74                	jbe    0x1c34
    1bc0:	75 b8                	jne    0x1b7a
    1bc2:	05 57 54             	add    $0x5457,%ax
    1bc5:	8b 3d                	mov    (%di),%di
    1bc7:	eb 91                	jmp    0x1b5a
    1bc9:	b8 72 06             	mov    $0x672,%ax
    1bcc:	ff 50 56             	call   *0x56(%bx,%si)
    1bcf:	40                   	inc    %ax
    1bd0:	07                   	pop    %es
    1bd1:	33 06 33 c7          	xor    -0x38cd,%ax
    1bd5:	19 c7                	sbb    %ax,%di
    1bd7:	ff                   	ljmp   (bad)
    1bd8:	e8 14 ae             	call   0xc9ef
    1bdb:	01 8b fa 0c          	add    %cx,0xcfa(%bp,%di)
    1bdf:	5e                   	pop    %si
    1be0:	1a f6                	sbb    %dh,%dh
    1be2:	56                   	push   %si
    1be3:	19 69 00             	sbb    %bp,0x0(%bx,%di)
    1be6:	be 83 08             	mov    $0x883,%si
    1be9:	56                   	push   %si
    1bea:	ff af 83 3e          	ljmp   *0x3e83(%bx)
    1bee:	86 06 00 c6          	xchg   %al,-0x3a00
    1bf2:	d1 8b 83 00          	rorw   $1,0x83(%bp,%di)
    1bf6:	74 12                	je     0x1c0a
    1bf8:	f6 06 c6 39 06       	testb  $0x6,0x39c6
    1bfd:	80 06 75 09 04       	addb   $0x4,0x975
    1c02:	04 06                	add    $0x6,%al
    1c04:	76 06                	jbe    0x1c0c
    1c06:	82 06 74 6f a1       	addb   $0xa1,0x6f74
    1c0b:	80 06 8b 1e 84       	addb   $0x84,0x1e8b
    1c10:	06                   	push   %es
    1c11:	2b 47 00             	sub    0x0(%bx),%ax
    1c14:	fc                   	cld
    1c15:	39 fc                	cmp    %di,%sp
    1c17:	a1 82 06             	mov    0x682,%ax
    1c1a:	2b 47 83             	sub    -0x7d(%bx),%ax
    1c1d:	e8 57 fe             	call   0x1a77
    1c20:	fe                   	(bad)
    1c21:	75 06                	jne    0x1c29
    1c23:	47                   	inc    %di
    1c24:	fc                   	cld
    1c25:	fc                   	cld
    1c26:	82 2b f8             	subb   $0xf8,(%bp,%di)
    1c29:	04 04                	add    $0x4,%al
    1c2b:	06                   	push   %es
    1c2c:	2b 47 83             	sub    -0x7d(%bx),%ax
    1c2f:	e8 57 fa             	call   0x1689
    1c32:	e8 86 68             	call   0x84bb
    1c35:	83 3e 86 06 00       	cmpw   $0x0,0x686
    1c3a:	74 0f                	je     0x1c4b
    1c3c:	b8 d0 83             	mov    $0x83d0,%ax
    1c3f:	c4                   	(bad)
    1c40:	cd 64                	int    $0x64
    1c42:	c7                   	(bad)
    1c43:	eb 50                	jmp    0x1c95
    1c45:	92                   	xchg   %ax,%dx
    1c46:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    1c47:	e8 83 45             	call   0x61cd
    1c4a:	6b b8 83 cd c7       	imul   $0xffc7,-0x327d(%bx,%si),%di
    1c4f:	50                   	push   %ax
    1c50:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    1c51:	f8                   	clc
    1c52:	eb 92                	jmp    0x1be6
    1c54:	e8 45 b8             	call   0xd49c
    1c57:	cd 58                	int    $0x58
    1c59:	6a ff                	push   $0xffff
    1c5b:	36 84 06 ff 76       	test   %al,%ss:0x76ff
    1c60:	37                   	aaa
    1c61:	e8 06 74             	call   0x906a
    1c64:	8f                   	(bad)
    1c65:	fb                   	sti
    1c66:	71 c7                	jno    0x1c2f
    1c68:	06                   	push   %es
    1c69:	86 06 4b f7          	xchg   %al,-0x8b5
    1c6d:	e9 46 a3             	jmp    0xbfb6
    1c70:	00 80 06 8b          	add    %al,-0x74fa(%bx,%si)
    1c74:	46                   	inc    %si
    1c75:	a3 00 82             	mov    %ax,0x8200
    1c78:	06                   	push   %es
    1c79:	89 e8                	mov    %bp,%ax
    1c7b:	ff 2b                	ljmp   *(%bp,%di)
    1c7d:	08 7e c7             	or     %bh,-0x39(%bp)
    1c80:	06                   	push   %es
    1c81:	98                   	cbtw
    1c82:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    1c83:	10 00                	adc    %al,(%bx,%si)
    1c85:	04 36                	add    $0x36,%al
    1c87:	ec                   	in     (%dx),%al
    1c88:	52                   	push   %dx
    1c89:	3e c0 14 c7          	rclb   $0xc7,%ds:(%si)
    1c8d:	06                   	push   %es
    1c8e:	94                   	xchg   %ax,%sp
    1c8f:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    1c90:	04 46                	add    $0x46,%al
    1c92:	c7 06 e8 45 40 1f    	movw   $0x1f40,0x45e8
    1c98:	c7 06 96 a6 03 00    	movw   $0x3,-0x596a
    1c9e:	eb 2a                	jmp    0x1cca
    1ca0:	04 36                	add    $0x36,%al
    1ca2:	ec                   	in     (%dx),%al
    1ca3:	52                   	push   %dx
    1ca4:	06                   	push   %es
    1ca5:	75 05                	jne    0x1cac
    1ca7:	ba 0c eb             	mov    $0xeb0c,%dx
    1caa:	03 fb                	add    %bx,%di
    1cac:	ba 18 33             	mov    $0x3318,%dx
    1caf:	c3                   	ret
    1cb0:	33 96 a6 52          	xor    0x52a6(%bp),%dx
    1cb4:	e8 87 69             	call   0x863e
    1cb7:	a3 94 a6             	mov    %ax,0xa694
    1cba:	c7 06 e8 45 00 00    	movw   $0x0,0x45e8
    1cc0:	80 3e 95 a6 ff       	cmpb   $0xff,-0x596b
    1cc5:	dc 41 2b             	faddl  0x2b(%bx,%di)
    1cc8:	c0 c3 1a             	rol    $0x1a,%bl
    1ccb:	8c 05                	mov    %es,(%di)
    1ccd:	ec                   	in     (%dx),%al
    1cce:	00 56 06             	add    %dl,0x6(%bp)
    1cd1:	8f                   	(bad)
    1cd2:	ff 89 ff 19          	decw   0x19ff(%bx,%di)
    1cd6:	86 06 00 c6          	xchg   %al,-0x3a00
    1cda:	3a a1 06 1e          	cmp    0x1e06(%bx,%di),%ah
    1cde:	06                   	push   %es
    1cdf:	47                   	inc    %di
    1ce0:	fc                   	cld
    1ce1:	fc                   	cld
    1ce2:	82 2b 83             	subb   $0x83,(%bp,%di)
    1ce5:	57                   	push   %di
    1ce6:	fe 06 fc 82          	incb   -0x7d04
    1cea:	f8                   	clc
    1ceb:	04 2b                	add    $0x2b,%al
    1ced:	83 57 e8 68          	adcw   $0x68,-0x18(%bx)
    1cf1:	b8 83 cd             	mov    $0xcd83,%ax
    1cf4:	c7                   	(bad)
    1cf5:	50                   	push   %ax
    1cf6:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    1cf7:	83 6b 83 c7          	subw   $0xffc7,-0x7d(%bp,%di)
    1cfb:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    1cfc:	eb e8                	jmp    0x1ce6
    1cfe:	b2 69                	mov    $0x69,%dl
    1d00:	ff 36 84 06          	push   0x684
    1d04:	ff 36 06 1e          	push   0x1e06
    1d08:	ff 36 82 f8          	push   -0x77e
    1d0c:	e8 53 71             	call   0x8e62
    1d0f:	06                   	push   %es
    1d10:	06                   	push   %es
    1d11:	f7 46 00 06 46       	testw  $0x4606,0x0(%bp)
    1d16:	53                   	push   %bx
    1d17:	ec                   	in     (%dx),%al
    1d18:	c7 00 0a 56          	movw   $0x560a,(%bx,%si)
    1d1c:	7e b1                	jle    0x1ccf
    1d1e:	89 19                	mov    %bx,(%bx,%di)
    1d20:	06                   	push   %es
    1d21:	c6                   	(bad)
    1d22:	a1 1e 74             	mov    0x741e,%ax
    1d25:	2b a1 1e 47          	sub    0x471e(%bx,%di),%sp
    1d29:	fc                   	cld
    1d2a:	2b 57 06             	sub    0x6(%bx),%dx
    1d2d:	82 04 83             	addb   $0x83,(%si)
    1d30:	e8 b8 cd             	call   0xeaeb
    1d33:	50                   	push   %ax
    1d34:	83 83 a6 e8 69       	addw   $0x69,-0x175a(%bp,%di)
    1d39:	36 06                	ss push %es
    1d3b:	36 1e                	ss push %ds
    1d3d:	36 f8                	ss clc
    1d3f:	53                   	push   %bx
    1d40:	06                   	push   %es
    1d41:	f7 00 46 ec          	testw  $0xec46,(%bx,%si)
    1d45:	00 56 b1             	add    %dl,-0x4f(%bp)
    1d48:	19 45 6a             	sbb    %ax,0x6a(%di)
    1d4b:	06                   	push   %es
    1d4c:	06                   	push   %es
    1d4d:	f7 46 fa fe fe       	testw  $0xfefe,-0x6(%bp)
    1d52:	53                   	push   %bx
    1d53:	c7                   	(bad)
    1d54:	0a 7e 89             	or     -0x77(%bp),%bh
    1d57:	06                   	push   %es
    1d58:	80 3e 8a 06 1e       	cmpb   $0x1e,0x68a
    1d5d:	73 25                	jae    0x1d84
    1d5f:	a0 f7 46             	mov    0x46f7,%al
    1d62:	8a 1e 8a 06          	mov    0x68a,%bl
    1d66:	2a ff                	sub    %bh,%bh
    1d68:	88 87 74 a6          	mov    %al,-0x598c(%bx)
    1d6c:	fe 46 8a             	incb   -0x76(%bp)
    1d6f:	00 56 89             	add    %dl,-0x77(%bp)
    1d72:	89 83 fa 01          	mov    %ax,0x1fa(%bp,%di)
    1d76:	75 05                	jne    0x1d7d
    1d78:	e8 53 ff             	call   0x1cce
    1d7b:	eb 07                	jmp    0x1d84
    1d7d:	0b 75 b8             	or     -0x48(%di),%si
    1d80:	03 e8                	add    %ax,%bp
    1d82:	95                   	xchg   %ax,%bp
    1d83:	06                   	push   %es
    1d84:	46                   	inc    %si
    1d85:	53                   	push   %bx
    1d86:	ec                   	in     (%dx),%al
    1d87:	57                   	push   %di
    1d88:	06                   	push   %es
    1d89:	e8 83 0e             	call   0x2c0f
    1d8c:	8b 05                	mov    (%di),%ax
    1d8e:	57                   	push   %di
    1d8f:	04 80                	add    $0x80,%al
    1d91:	3e 8a 06 1e 74       	mov    %ds:0x741e,%al
    1d96:	5a                   	pop    %dx
    1d97:	fe 8a 75 06          	decb   0x675(%bp,%si)
    1d9b:	19 c6                	sbb    %ax,%si
    1d9d:	1e                   	push   %ds
    1d9e:	2b 1e fc 03          	sub    0x3fc,%bx
    1da2:	e8 74 ff             	call   0x1d19
    1da5:	8a 8a 2a 88          	mov    -0x77d6(%bp,%si),%cl
    1da9:	74 fe                	je     0x1da9
    1dab:	8a 87 74 a6          	mov    -0x598c(%bx),%al
    1daf:	98                   	cbtw
    1db0:	a3 1e 2b             	mov    %ax,0x2b1e
    1db3:	07                   	pop    %es
    1db4:	06                   	push   %es
    1db5:	c7                   	(bad)
    1db6:	3a 06 06 fc          	cmp    -0x3fa,%al
    1dba:	82 83 fe fc f8       	addb   $0xf8,-0x302(%bp,%di)
    1dbf:	2b 57 68             	sub    0x68(%bx),%dx
    1dc2:	83 c7 a6             	add    $0xffa6,%di
    1dc5:	6b c7 eb             	imul   $0xffeb,%di,%ax
    1dc8:	b2 ff                	mov    $0xff,%dl
    1dca:	84 ff                	test   %bh,%bh
    1dcc:	06                   	push   %es
    1dcd:	ff 82 e8 71          	incw   0x71e8(%bp,%si)
    1dd1:	06                   	push   %es
    1dd2:	46                   	inc    %si
    1dd3:	06                   	push   %es
    1dd4:	53                   	push   %bx
    1dd5:	c7                   	(bad)
    1dd6:	0a 7e 89             	or     -0x77(%bp),%bh
    1dd9:	06                   	push   %es
    1dda:	d6                   	(bad)
    1ddb:	68 ff 84             	push   $0x84ff
    1dde:	ff 06 ff 82          	incw   -0x7d01
    1de2:	e8 71 06             	call   0x2456
    1de5:	46                   	inc    %si
    1de6:	06                   	push   %es
    1de7:	53                   	push   %bx
    1de8:	c7                   	(bad)
    1de9:	77 70                	ja     0x1e5b
    1deb:	06                   	push   %es
    1dec:	f7 00 46 ec          	testw  $0xec46,(%bx,%si)
    1df0:	00 56 b1             	add    %dl,-0x4f(%bp)
    1df3:	19 c6                	sbb    %ax,%si
    1df5:	1e                   	push   %ds
    1df6:	2b 1e 8b a3          	sub    -0x5c75,%bx
    1dfa:	82 89 ff 06 ec       	orb    $0xec,0x6ff(%bx,%di)
    1dff:	57                   	push   %di
    1e00:	06                   	push   %es
    1e01:	e8 e8 6e             	call   0x8cec
    1e04:	e5 a2                	in     $0xa2,%ax
    1e06:	e4 c0                	in     $0xc0,%al
    1e08:	3c 02                	cmp    $0x2,%al
    1e0a:	73 06                	jae    0x1e12
    1e0c:	c7                   	(bad)
    1e0d:	98                   	cbtw
    1e0e:	10 46 ec             	adc    %al,-0x14(%bp)
    1e11:	00 e8                	add    %ch,%al
    1e13:	65 e5 3d             	gs in  $0x3d,%ax
    1e16:	00 17                	add    %dl,(%bx)
    1e18:	7c 06                	jl     0x1e20
    1e1a:	c7                   	(bad)
    1e1b:	98                   	cbtw
    1e1c:	96                   	xchg   %ax,%si
    1e1d:	46                   	inc    %si
    1e1e:	ec                   	in     (%dx),%al
    1e1f:	00 c7                	add    %al,%bh
    1e21:	0a 7e 89             	or     -0x77(%bp),%bh
    1e24:	06                   	push   %es
    1e25:	a1 2c 56             	mov    0x562c,%ax
    1e28:	f4                   	hlt
    1e29:	e8 06 46             	call   0x6432
    1e2c:	d4 02                	aam    $0x2
    1e2e:	ff                   	ljmp   (bad)
    1e2f:	e8 39 e8             	call   0x66b
    1e32:	aa                   	stos   %al,%es:(%di)
    1e33:	e8 53 c7             	call   0xe589
    1e36:	71 81                	jno    0x1db9
    1e38:	83 06 bf 2c 3e       	addw   $0x3e,0x2cbf
    1e3d:	06                   	push   %es
    1e3e:	74 3e                	je     0x1e7e
    1e40:	96                   	xchg   %ax,%si
    1e41:	00 21                	add    %ah,(%bx,%di)
    1e43:	7e 34                	jle    0x1e79
    1e45:	ff 68 46             	ljmp   *0x46(%bx,%si)
    1e48:	b8 4e cc             	mov    $0xcc4e,%ax
    1e4b:	06                   	push   %es
    1e4c:	2c a0                	sub    $0xa0,%al
    1e4e:	23 e5                	and    %bp,%sp
    1e50:	34 ff                	xor    $0xff,%al
    1e52:	68 46 0d             	push   $0xd46
    1e55:	e5 06                	in     $0x6,%ax
    1e57:	d4 ff                	aam    $0xff
    1e59:	39 aa 53 71          	cmp    %bp,0x7153(%bp,%si)
    1e5d:	83 bf 3e 74 96       	cmpw   $0xff96,0x743e(%bx)
    1e62:	45                   	inc    %bp
    1e63:	81 06 2c 06 3e 00    	addw   $0x3e,0x62c
    1e69:	75 be                	jne    0x1e29
    1e6b:	57                   	push   %di
    1e6c:	f6 54 36             	notb   0x36(%si)
    1e6f:	06                   	push   %es
    1e70:	40                   	inc    %ax
    1e71:	1d 39 98             	sbb    $0x9839,%ax
    1e74:	74 b8                	je     0x1e2e
    1e76:	02 09                	add    (%bx,%di),%cl
    1e78:	b8 ac 06             	mov    $0x6ac,%ax
    1e7b:	50                   	push   %ax
    1e7c:	40                   	inc    %ax
    1e7d:	55                   	push   %bp
    1e7e:	3b eb                	cmp    %bx,%bp
    1e80:	01 02                	add    %ax,(%bp,%si)
    1e82:	db 06 50 40          	fildl  0x4050
    1e86:	4c                   	dec    %sp
    1e87:	3b 59 33             	cmp    0x33(%bx,%di),%bx
    1e8a:	e8 fe f6             	call   0x158b
    1e8d:	58                   	pop    %ax
    1e8e:	f0 f6 20             	lock mulb (%bx,%si)
    1e91:	dc 97 5e 83          	fcoml  -0x7ca2(%bx)
    1e95:	5e                   	pop    %si
    1e96:	3b c7                	cmp    %di,%ax
    1e98:	7e 06                	jle    0x1ea0
    1e9a:	2c f4                	sub    $0xf4,%al
    1e9c:	06                   	push   %es
    1e9d:	2e 76 98             	jbe,pn 0x1e38
    1ea0:	89 bc 89 f0          	mov    %di,-0xf77(%si)
    1ea4:	8c 89 e8 7e          	mov    %cs,0x7ee8(%bx,%di)
    1ea8:	e8 f9 02             	call   0x21a4
    1eab:	83 d8 00             	sbb    $0x0,%ax
    1eae:	15 7e 75             	adc    $0x757e,%ax
    1eb1:	13 83 da 00          	adc    0xda(%bp,%di),%ax
    1eb5:	21 d6                	and    %dx,%si
    1eb7:	75 0c                	jne    0x1ec5
    1eb9:	fb                   	sti
    1eba:	f9                   	stc
    1ebb:	e4 f9                	in     $0xf9,%al
    1ebd:	44                   	inc    %sp
    1ebe:	ab                   	stos   %ax,%es:(%di)
    1ebf:	e8 3d 14             	call   0x32ff
    1ec2:	17                   	pop    %ss
    1ec3:	7c 03                	jl     0x1ec8
    1ec5:	e9 d4 02             	jmp    0x219c
    1ec8:	2b 8b 5e 7e          	sub    0x7e5e(%bp,%di),%cx
    1ecc:	ea eb 2f b8 13       	ljmp   $0x13b8,$0x2feb
    1ed1:	e4 f9                	in     $0xf9,%al
    1ed3:	44                   	inc    %sp
    1ed4:	95                   	xchg   %ax,%bp
    1ed5:	e8 f7 37             	call   0x56cf
    1ed8:	ca d8 e3             	lret   $0xe3d8
    1edb:	87 03                	xchg   %ax,(%bp,%di)
    1edd:	df 8a 87 02          	fisttps 0x287(%bp,%si)
    1ee1:	0f 88 d4 ff          	js     0x1eb9
    1ee5:	0a e6                	or     %dh,%ah
    1ee7:	e9 0c 8b             	jmp    0xa9f6
    1eea:	df 83 b9 dc          	filds  -0x2347(%bp,%di)
    1eee:	21 d6                	and    %dx,%si
    1ef0:	75 be                	jne    0x1eb0
    1ef2:	57                   	push   %di
    1ef3:	f6 54 80             	notb   -0x80(%si)
    1ef6:	7e d4                	jle    0x1ecc
    1ef8:	01 75 57             	add    %si,0x57(%di)
    1efb:	54                   	push   %sp
    1efc:	06                   	push   %es
    1efd:	1d 98 b8             	sbb    $0xb898,%ax
    1f00:	74 cd                	je     0x1ecf
    1f02:	89 7e ea             	mov    %di,-0x16(%bp)
    1f05:	8b df                	mov    %di,%bx
    1f07:	e3 87                	jcxz   0x1e90
    1f09:	89 5e d2             	mov    %bx,-0x2e(%bp)
    1f0c:	c7 87 dc 21 4b e9    	movw   $0xe94b,0x21dc(%bx)
    1f12:	a3 80 ea             	mov    %ax,0xea80
    1f15:	2c 3e                	sub    $0x3e,%al
    1f17:	06                   	push   %es
    1f18:	1b 48 74             	sbb    0x74(%bx,%si),%cx
    1f1b:	1d 48 37             	sbb    $0x3748,%ax
    1f1e:	74 48                	je     0x1f68
    1f20:	68 66 8b             	push   $0x8b66
    1f23:	5e                   	pop    %si
    1f24:	d2 ff                	sar    %cl,%bh
    1f26:	b7 48                	mov    $0x48,%bh
    1f28:	0f 8e 49 f0          	jle    0xf75
    1f2c:	3e a7                	cmpsw  %es:(%di),%ds:(%si)
    1f2e:	02 7f 5b             	add    0x5b(%bx),%bh
    1f31:	5b                   	pop    %bx
    1f32:	eb 73                	jmp    0x1fa7
    1f34:	c7                   	(bad)
    1f35:	f0 8c 0a             	lock mov %cs,(%bp,%si)
    1f38:	00 eb                	add    %ch,%bl
    1f3a:	56                   	push   %si
    1f3b:	64 dc e8             	fs fsubr %st,%st(0)
    1f3e:	2b e8                	sub    %ax,%bp
    1f40:	01 f0                	add    %si,%ax
    1f42:	8c 89 bc 89          	mov    %cs,-0x7644(%bx,%di)
    1f46:	f0 f2 fa             	lock repnz cli
    1f49:	8d                   	lea    (bad),%ax
    1f4a:	c3                   	ret
    1f4b:	f6 00 00             	testb  $0x0,(%bx,%si)
    1f4e:	17                   	pop    %ss
    1f4f:	11 8b 76 f4          	adc    %cx,-0xb8a(%bp,%di)
    1f53:	e9 a8 01             	jmp    0x20fe
    1f56:	8a 8b d2 b7          	mov    -0x482e(%bp,%di),%cl
    1f5a:	0f 49 3e 02 5b       	cmovns 0x5b02,%di
    1f5f:	eb c7                	jmp    0x1f28
    1f61:	8c ce                	mov    %cs,%si
    1f63:	7e 5b                	jle    0x1fc0
    1f65:	5b                   	pop    %bx
    1f66:	8a 1e 71 59          	mov    0x5971,%bl
    1f6a:	2a b2 02 84          	sub    -0x7bfe(%bp,%si),%dh
    1f6e:	b8 63 74             	mov    $0x7463,%ax
    1f71:	59                   	pop    %cx
    1f72:	b8 8e fc             	mov    $0xfc8e,%ax
    1f75:	49                   	dec    %cx
    1f76:	b8 f0 7e             	mov    $0x7ef0,%ax
    1f79:	5b                   	pop    %bx
    1f7a:	5b                   	pop    %bx
    1f7b:	b8 d1 0e             	mov    $0xed1,%ax
    1f7e:	02 5d 79             	add    0x79(%di),%bl
    1f81:	cc                   	int3
    1f82:	5b                   	pop    %bx
    1f83:	8e 6c 7e             	mov    0x7e(%si),%gs
    1f86:	eb a8                	jmp    0x1f30
    1f88:	2b 52 02             	sub    0x2(%bp,%si),%dx
    1f8b:	25 eb 80             	and    $0x80eb,%ax
    1f8e:	09 00                	or     %ax,(%bx,%si)
    1f90:	ff 56 07             	call   *0x7(%bp)
    1f93:	01 42 83             	add    %ax,-0x7d(%bp,%si)
    1f96:	c3                   	ret
    1f97:	14 81                	adc    $0x81,%al
    1f99:	fb                   	sti
    1f9a:	15 72 bb             	adc    $0xbb72,%ax
    1f9d:	f1                   	int1
    1f9e:	0b b8 e8 3c          	or     0x3ce8(%bx,%si),%di
    1fa2:	f6 00 e8             	testb  $0xe8,(%bx,%si)
    1fa5:	2b 76 fc             	sub    -0x4(%bp),%si
    1fa8:	74 e8                	je     0x1f92
    1faa:	1e                   	push   %ds
    1fab:	03 74 e9             	add    -0x17(%si),%si
    1fae:	ec                   	in     (%dx),%al
    1faf:	01 8a 8b ea          	add    %cx,-0x1575(%bp,%si)
    1fb3:	8b c3                	mov    %bx,%ax
    1fb5:	e3 87                	jcxz   0x1f3e
    1fb7:	03 d8                	add    %ax,%bx
    1fb9:	8a 87 03 0f          	mov    0xf03(%bx),%al
    1fbd:	98                   	cbtw
    1fbe:	da f3                	(bad)
    1fc0:	d8 88 6c 53          	fmuls  0x536c(%bx,%si)
    1fc4:	8a 84 04 0f          	mov    0xf04(%si),%al
    1fc8:	88 d4                	mov    %dl,%ah
    1fca:	ff                   	(bad)
    1fcb:	3c 80                	cmp    $0x80,%al
    1fcd:	03 74 e9             	add    -0x17(%si),%si
    1fd0:	86 01                	xchg   %al,(%bx,%di)
    1fd2:	fa                   	cli
    1fd3:	5e                   	pop    %si
    1fd4:	04 d1                	add    $0xd1,%al
    1fd6:	e8 29 06             	call   0x2602
    1fd9:	63 05                	arpl   %ax,(%di)
    1fdb:	e9 b7 01             	jmp    0x2195
    1fde:	98                   	cbtw
    1fdf:	89 fb                	mov    %di,%bx
    1fe1:	3d 87 e7             	cmp    $0xe787,%ax
    1fe4:	8b f8                	mov    %ax,%di
    1fe6:	2b f6                	sub    %si,%si
    1fe8:	89 76 fc             	mov    %si,-0x4(%bp)
    1feb:	89 fe                	mov    %di,%si
    1fed:	3e 9b                	ds fwait
    1fef:	fe 0c                	decb   (%si)
    1ff1:	7d 09                	jge    0x1ffc
    1ff3:	0b 74 32             	or     0x32(%si),%si
    1ff6:	03 46 eb             	add    -0x15(%bp),%ax
    1ff9:	f4                   	hlt
    1ffa:	8b fe                	mov    %si,%di
    1ffc:	be 1e 19             	mov    $0x191e,%si
    1fff:	f8                   	clc
    2000:	fe                   	(bad)
    2001:	14 0a                	adc    $0xa,%al
    2003:	22 8a 44 04          	and    0x444(%bp,%si),%cl
    2007:	89 c8                	mov    %cx,%ax
    2009:	8b c7                	mov    %di,%ax
    200b:	bb 14 f7             	mov    $0xf714,%bx
    200e:	03 eb                	add    %bx,%bp
    2010:	8b d8                	mov    %ax,%bx
    2012:	38 8f 24 19          	cmp    %cl,0x1924(%bx)
    2016:	75 0b                	jne    0x2023
    2018:	8b 04                	mov    (%si),%ax
    201a:	b9 56 64             	mov    $0x6456,%cx
    201d:	f1                   	int1
    201e:	f0 48                	lock dec %ax
    2020:	8b 29                	mov    (%bx,%di),%bp
    2022:	04 c6                	add    $0xc6,%al
    2024:	73 14                	jae    0x203a
    2026:	fe                   	(bad)
    2027:	1a 0e 72 bb          	sbb    -0x448e,%cl
    202b:	d8 ff                	fdivr  %st(7),%st
    202d:	36 44                	ss inc %sp
    202f:	3e 5b                	ds pop %bx
    2031:	c7                   	(bad)
    2032:	ce                   	into
    2033:	5b                   	pop    %bx
    2034:	8a fb                	mov    %bl,%bh
    2036:	7d 5b                	jge    0x2093
    2038:	8a 0b                	mov    (%bp,%di),%cl
    203a:	c7 00 00 e9          	movw   $0xe900,(%bx,%si)
    203e:	46                   	inc    %si
    203f:	fc                   	cld
    2040:	8b d8                	mov    %ax,%bx
    2042:	8a 9f 24 19          	mov    0x1924(%bx),%bl
    2046:	2a 02                	sub    (%bp,%si),%al
    2048:	b8 74 b8             	mov    $0xb874,%ax
    204b:	fc                   	cld
    204c:	22 52 5b             	and    0x5b(%bp,%si),%dl
    204f:	c7                   	(bad)
    2050:	ce                   	into
    2051:	5b                   	pop    %bx
    2052:	8a 9d 7d 5b          	mov    0x5b7d(%di),%bl
    2056:	8a ff                	mov    %bh,%bh
    2058:	36 46                	ss inc %si
    205a:	0f e9 21             	psubsw (%bx,%di),%mm4
    205d:	ff 80 7c 2b          	incw   0x2b7c(%bx,%si)
    2061:	76 74                	jbe    0x20d7
    2063:	06                   	push   %es
    2064:	80 7c fd 22          	cmpb   $0x22,-0x3(%si)
    2068:	72 00                	jb     0x206a
    206a:	33 8b 64 f1          	xor    -0xe9c(%bp,%di),%cx
    206e:	db 89 5e f8          	fisttpl -0x7a2(%bx,%di)
    2072:	8d 44 f7             	lea    -0x9(%si),%ax
    2075:	89 fa                	mov    %di,%dx
    2077:	06                   	push   %es
    2078:	50                   	push   %ax
    2079:	76 f4                	jbe    0x206f
    207b:	eb 33                	jmp    0x20b0
    207d:	8b 76 fa             	mov    -0x6(%bp),%si
    2080:	8b 04                	mov    (%si),%ax
    2082:	fc                   	cld
    2083:	f6 db                	neg    %bl
    2085:	2b c0                	sub    %ax,%ax
    2087:	74 22                	je     0x20ab
    2089:	f7 66 f0             	mulw   -0x10(%bp)
    208c:	b9 00 f9             	mov    $0xf900,%cx
    208f:	2b f7                	sub    %di,%si
    2091:	89 fa                	mov    %di,%dx
    2093:	32 73 fc             	xor    -0x4(%bp,%di),%dh
    2096:	48                   	dec    %ax
    2097:	75 46                	jne    0x20df
    2099:	f8                   	clc
    209a:	83 fc 02             	cmp    $0x2,%sp
    209d:	f7 e6                	mul    %si
    209f:	83 fc 01             	cmp    $0x1,%sp
    20a2:	46                   	inc    %si
    20a3:	f2 29 04             	repnz sub %ax,(%si)
    20a6:	8b 04                	mov    (%si),%ax
    20a8:	01 46 f8             	add    %ax,-0x8(%bp)
    20ab:	83 fa ff             	cmp    $0xffff,%dx
    20ae:	02 43 fb             	add    -0x5(%bp,%di),%al
    20b1:	72 03                	jb     0x20b6
    20b3:	7c c8                	jl     0x207d
    20b5:	8b 76 f4             	mov    -0xc(%bp),%si
    20b8:	8a 04                	mov    (%si),%al
    20ba:	f7 f0                	div    %ax
    20bc:	dc f8                	fdivr  %st,%st(0)
    20be:	81 2b 76 00          	subw   $0x76,(%bp,%di)
    20c2:	34 40                	xor    $0x40,%al
    20c4:	bd 56 1f             	mov    $0x1f56,%bp
    20c7:	57                   	push   %di
    20c8:	f0 fa                	lock cli
    20ca:	ee                   	out    %al,(%dx)
    20cb:	dc 98 05 ec          	fcompl -0x13fb(%bx,%si)
    20cf:	51                   	push   %cx
    20d0:	e8 5f 19             	call   0x3a32
    20d3:	0b 00                	or     (%bx,%si),%ax
    20d5:	e9 fc d8             	jmp    0xf9d4
    20d8:	9f                   	lahf
    20d9:	19 02                	sbb    %ax,(%bp,%si)
    20db:	74 04                	je     0x20e1
    20dd:	89 06 04 87          	mov    %ax,-0x78fc
    20e1:	20 19                	and    %bl,(%bx,%di)
    20e3:	0b 05                	or     (%di),%ax
    20e5:	ec                   	in     (%dx),%al
    20e6:	89 8f 1e 19          	mov    %cx,0x191e(%bx)
    20ea:	f8                   	clc
    20eb:	53                   	push   %bx
    20ec:	0b 83 03 c1          	or     -0x3efd(%bp,%di),%ax
    20f0:	2b f7                	sub    %di,%si
    20f2:	89 76 f8             	mov    %si,-0x8(%bp)
    20f5:	fc                   	cld
    20f6:	0a 01                	or     (%bx,%di),%al
    20f8:	83 0c 81             	orw    $0xff81,(%si)
    20fb:	ff 46 f6             	incw   -0xa(%bp)
    20fe:	fe                   	(bad)
    20ff:	1a 3b                	sbb    (%bp,%di),%bh
    2101:	11 73 e9             	adc    %si,-0x17(%bp,%di)
    2104:	fd                   	std
    2105:	57                   	push   %di
    2106:	76 74                	jbe    0x217c
    2108:	1e                   	push   %ds
    2109:	f2 00 d1             	repnz add %dl,%cl
    210c:	83 74 92 fe          	xorw   $0xfffe,-0x6e(%si)
    2110:	ff 36 3c 3e          	push   0x3e3c
    2114:	c7                   	(bad)
    2115:	5b                   	pop    %bx
    2116:	fb                   	sti
    2117:	5b                   	pop    %bx
    2118:	0b 17                	or     (%bx),%dx
    211a:	7d 5b                	jge    0x2177
    211c:	8a 74 1e             	mov    0x1e(%si),%dh
    211f:	ea 00 f4 7e ff       	ljmp   $0xff7e,$0xf400
    2124:	36 40                	ss inc %ax
    2126:	0f eb 04             	por    (%si),%mm0
    2129:	ff 36 42 3e          	push   0x3e42
    212d:	5b                   	pop    %bx
    212e:	5b                   	pop    %bx
    212f:	17                   	pop    %ss
    2130:	5b                   	pop    %bx
    2131:	74 be                	je     0x20f1
    2133:	7c 5b                	jl     0x2190
    2135:	b8 0e 50             	mov    $0x500e,%ax
    2138:	35 c7 16             	xor    $0x16c7,%ax
    213b:	e3 d6                	jcxz   0x2113
    213d:	98                   	cbtw
    213e:	ff                   	ljmp   (bad)
    213f:	e8 f2 e8             	call   0xa34
    2142:	98                   	cbtw
    2143:	7e 06                	jle    0x214b
    2145:	06                   	push   %es
    2146:	00 02                	add    %al,(%bp,%si)
    2148:	79 5b                	jns    0x21a5
    214a:	6c                   	insb   (%dx),%es:(%di)
    214b:	eb 2b                	jmp    0x2178
    214d:	a3 7c 5b             	mov    %ax,0x5b7c
    2150:	b8 ff 36             	mov    $0x36ff,%ax
    2153:	3e 0f e9 27          	psubsw %ds:(%bx),%mm4
    2157:	fe 80 ff 73          	incb   0x73ff(%bx,%si)
    215b:	7f f4                	jg     0x2151
    215d:	7e fa                	jle    0x2159
    215f:	04 e8                	add    $0xe8,%al
    2161:	e9 73 fe             	jmp    0x1fd7
    2164:	80 ff 73             	cmp    $0x73,%bh
    2167:	00 7d 23             	add    %bh,0x23(%di)
    216a:	30 83 c7 14          	xor    %al,0x14c7(%bp,%di)
    216e:	99                   	cwtd
    216f:	77 d3                	ja     0x2144
    2171:	f7 8a 3b 06 05 b7    	testw  $0xb705,0x63b(%bp,%si)
    2177:	73 0c                	jae    0x2185
    2179:	30 c7                	xor    %al,%bh
    217b:	99                   	cwtd
    217c:	d3 8a 06 b7          	rorw   %cl,-0x48fa(%bp,%si)
    2180:	0c c7                	or     $0xc7,%al
    2182:	e9 52 fe             	jmp    0x1fd7
    2185:	c7 06 05 b7 00 00    	movw   $0x0,-0x48fb
    218b:	eb 08                	jmp    0x2195
    218d:	30 c7                	xor    %al,%bh
    218f:	99                   	cwtd
    2190:	d3 01                	rolw   %cl,(%bx,%di)
    2192:	4a                   	dec    %dx
    2193:	b1 8b                	mov    $0x8b,%cl
    2195:	79 5b                	jns    0x21f2
    2197:	6c                   	insb   (%dx),%es:(%di)
    2198:	eb 2b                	jmp    0x21c5
    219a:	38 38                	cmp    %bh,(%bx,%si)
    219c:	5e                   	pop    %si
    219d:	1a f6                	sbb    %dh,%dh
    219f:	8b 57 1a             	mov    0x1a(%bx),%dx
    21a2:	06                   	push   %es
    21a3:	3e e8 4e b6          	ds call 0xd7f5
    21a7:	56                   	push   %si
    21a8:	06                   	push   %es
    21a9:	8f                   	(bad)
    21aa:	ff                   	ljmp   (bad)
    21ab:	e8 83 00             	call   0x2231
    21ae:	d6                   	(bad)
    21af:	0c f9                	or     $0xf9,%al
    21b1:	f9                   	stc
    21b2:	13 fb                	adc    %bx,%di
    21b4:	e4 44                	in     $0x44,%al
    21b6:	e8 14 b1             	call   0xd2cd
    21b9:	e5 3a                	in     $0x3a,%ax
    21bb:	06                   	push   %es
    21bc:	d4 0e                	aam    $0xe
    21be:	7d 06                	jge    0x21c6
    21c0:	98                   	cbtw
    21c1:	46                   	inc    %si
    21c2:	d8 21                	fsubs  (%bx,%di)
    21c4:	e9 80 83             	jmp    0xa547
    21c7:	00 7e 13             	add    %bh,0x13(%bp)
    21ca:	da 21                	fisubl (%bx,%di)
    21cc:	75 fb                	jne    0x21c9
    21ce:	e4 44                	in     $0x44,%al
    21d0:	e8 14 97             	call   0xb8e7
    21d3:	e5 3a                	in     $0x3a,%ax
    21d5:	06                   	push   %es
    21d6:	d5 0e                	aad    $0xe
    21d8:	7d 06                	jge    0x21e0
    21da:	98                   	cbtw
    21db:	46                   	inc    %si
    21dc:	d6                   	(bad)
    21dd:	0c e9                	or     $0xe9,%al
    21df:	80 a0 d4 0e 98       	andb   $0x98,0xed4(%bx,%si)
    21e4:	05 05 00             	add    $0x5,%ax
    21e7:	3d e4 44             	cmp    $0x44e4,%ax
    21ea:	7e b8                	jle    0x21a4
    21ec:	83 e4 44             	and    $0x44,%sp
    21ef:	a2 d4 0e             	mov    %al,0xed4
    21f2:	a0 d5 0e             	mov    0xed5,%al
    21f5:	98                   	cbtw
    21f6:	05 3d 44             	add    $0x443d,%ax
    21f9:	b8 e4 a2             	mov    $0xa2e4,%ax
    21fc:	0e                   	push   %cs
    21fd:	d5 98                	aad    $0x98
    21ff:	3d b8 a2             	cmp    $0xa2b8,%ax
    2202:	d5 0e                	aad    $0xe
    2204:	83 d8 00             	sbb    $0x0,%ax
    2207:	15 7e 98             	adc    $0x987e,%ax
    220a:	11 80 85 25          	adc    %ax,0x2585(%bx,%si)
    220e:	3a 04                	cmp    (%si),%al
    2210:	75 11                	jne    0x2223
    2212:	b8 6e 0f             	mov    $0xf6e,%ax
    2215:	e8 14 bb             	call   0xdd2c
    2218:	37                   	aaa
    2219:	e9 a9 00             	jmp    0x22c5
    221c:	36 52                	ss push %dx
    221e:	d6                   	(bad)
    221f:	0c f9                	or     $0xf9,%al
    2221:	f9                   	stc
    2222:	03 e9                	add    %cx,%bp
    2224:	9f                   	lahf
    2225:	00 3e a6 d6          	add    %bh,-0x295a
    2229:	0c f9                	or     $0xf9,%al
    222b:	f9                   	stc
    222c:	56                   	push   %si
    222d:	2b 02                	sub    (%bp,%si),%ax
    222f:	eb a5                	jmp    0x21d6
    2231:	52                   	push   %dx
    2232:	80 00 56             	addb   $0x56,(%bx,%si)
    2235:	01 7e 98             	add    %di,-0x68(%bp)
    2238:	01 83 14 fb          	add    %ax,-0x4ec(%bp,%di)
    223c:	20 fb                	and    %bh,%bl
    223e:	55                   	push   %bp
    223f:	ef                   	out    %ax,(%dx)
    2240:	d2 0a                	rorb   %cl,(%bp,%si)
    2242:	f1                   	int1
    2243:	8b fa                	mov    %dx,%di
    2245:	0b ff                	or     %di,%di
    2247:	7e 3a                	jle    0x2283
    2249:	57                   	push   %di
    224a:	e8 1e e5             	call   0x76b
    224d:	98                   	cbtw
    224e:	8b f8                	mov    %ax,%di
    2250:	c6 06 d6 0c 01       	movb   $0x1,0xcd6
    2255:	c7 c7 e8 01          	mov    $0x1e8,%di
    2259:	c7 00 02 fe          	movw   $0xfe02,(%bx,%si)
    225d:	a5                   	movsw  %ds:(%si),%es:(%di)
    225e:	52                   	push   %dx
    225f:	04 fa                	add    $0xfa,%al
    2261:	47                   	inc    %di
    2262:	8b d7                	mov    %di,%dx
    2264:	81 fe 01 e5          	cmp    $0xe501,%si
    2268:	73 e9                	jae    0x2253
    226a:	18 8b fe 9c          	sbb    %cl,-0x6302(%bp,%di)
    226e:	80 06 41 3e 05       	addb   $0x5,0x3e41
    2273:	0b d2                	or     %dx,%dx
    2275:	74 08                	je     0x227f
    2277:	4a                   	dec    %dx
    2278:	83 81 47 20 43       	addw   $0x43,0x2047(%bx,%di)
    227d:	eb e5                	jmp    0x2264
    227f:	88 1e d6 0c          	mov    %bl,0xcd6
    2283:	3e a6                	cmpsb  %es:(%di),%ds:(%si)
    2285:	d6                   	(bad)
    2286:	0c f9                	or     $0xf9,%al
    2288:	74 35                	je     0x22bf
    228a:	b8 b2 0f             	mov    $0xfb2,%ax
    228d:	e8 5b ce             	call   0xf0eb
    2290:	8a 7d 8a             	mov    -0x76(%di),%bh
    2293:	36 7b 5b             	ss jnp 0x22f1
    2296:	5b                   	pop    %bx
    2297:	a0 0e 8b             	mov    0x8b0e,%al
    229a:	b1 d3                	mov    $0xd3,%cl
    229c:	8a b8 b8 22          	mov    0x22b8(%bx,%si),%bh
    22a0:	5b                   	pop    %bx
    22a1:	ce                   	into
    22a2:	8a 7d 8a             	mov    -0x76(%di),%bh
    22a5:	36 0f 21 48          	ss mov %db1,%eax
    22a9:	7b 5b                	jnp    0x2306
    22ab:	5b                   	pop    %bx
    22ac:	b8 c3 0f             	mov    $0xfc3,%ax
    22af:	5b                   	pop    %bx
    22b0:	8a 8a 7b 5b          	mov    0x5b7b(%bp,%si),%cl
    22b4:	0e                   	push   %cs
    22b5:	3b 7b 5b             	cmp    0x5b(%bp,%di),%di
    22b8:	5b                   	pop    %bx
    22b9:	b8 8a 7b             	mov    $0x7b8a,%ax
    22bc:	e9 56 ff             	jmp    0x2215
    22bf:	98                   	cbtw
    22c0:	46                   	inc    %si
    22c1:	d6                   	(bad)
    22c2:	0c 00                	or     $0x0,%al
    22c4:	00 5e f6             	add    %bl,-0xa(%bp)
    22c7:	19 00                	sbb    %ax,(%bx,%si)
    22c9:	83 56 af b8          	adcw   $0xffb8,-0x51(%bp)
    22cd:	f4                   	hlt
    22ce:	01 b8 00 33          	add    %di,0x3300(%bx,%si)
    22d2:	fe                   	(bad)
    22d3:	58                   	pop    %ax
    22d4:	f6 b1 51 a3          	divb   -0x5caf(%bx,%di)
    22d8:	9a a6 03 8b 46       	lcall  $0x468b,$0x3a6
    22dd:	00 1a                	add    %bl,(%bp,%si)
    22df:	05 00 06             	add    $0x600,%ax
    22e2:	ff 36 9a a6          	push   -0x5966
    22e6:	e8 ce 51             	call   0x74b7
    22e9:	c7 06 f4 2e 98 bc    	movw   $0xbc98,0x2ef4
    22ef:	0e                   	push   %cs
    22f0:	05 76 08             	add    $0x876,%ax
    22f3:	a3 89 06             	mov    %ax,0x689
    22f6:	fa                   	cli
    22f7:	9a 08 46 0d ff       	lcall  $0xff0d,$0x4608
    22fc:	06                   	push   %es
    22fd:	cf                   	iret
    22fe:	c6                   	(bad)
    22ff:	bc 60 89             	mov    $0x8960,%sp
    2302:	06                   	push   %es
    2303:	76 3b                	jbe    0x2340
    2305:	03 78 74             	add    0x74(%bx,%si),%di
    2308:	25 c7 f6             	and    $0xf6c7,%ax
    230b:	00 02                	add    %al,(%bp,%si)
    230d:	e8 f8 46             	call   0x6a08
    2310:	33 01                	xor    (%bx,%di),%ax
    2312:	c7 03 04 06          	movw   $0x604,(%bp,%di)
    2316:	8b 1e 9a a6          	mov    -0x5966,%bx
    231a:	89 f6                	mov    %si,%si
    231c:	57                   	push   %di
    231d:	46                   	inc    %si
    231e:	fc                   	cld
    231f:	83 fc 9a             	cmp    $0xff9a,%sp
    2322:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    2323:	89 03                	mov    %ax,(%bp,%di)
    2325:	04 f6                	add    $0xf6,%al
    2327:	f6 00 39             	testb  $0x39,(%bx,%si)
    232a:	46                   	inc    %si
    232b:	33 f9                	xor    %cx,%di
    232d:	e9 00 1e             	jmp    0x4130
    2330:	01 d3                	add    %dx,%bx
    2332:	8a b8 b8 e3          	mov    -0x1c48(%bx,%si),%bh
    2336:	c8 9e 36 9a          	enter  $0x369e,$0x9a
    233a:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    233b:	8b 35                	mov    (%di),%si
    233d:	ff 06 08 03          	incw   0x308
    2341:	f3 8b 02             	repz mov (%bp,%si),%ax
    2344:	8b 13                	mov    (%bp,%di),%dx
    2346:	03 7e 04             	add    0x4(%bp),%di
    2349:	e9 f6 40             	jmp    0x6442
    234c:	b9 f4 01             	mov    $0x1f4,%cx
    234f:	2b 89 32 fc          	sub    -0x3ce(%bx,%di),%cx
    2353:	75 56                	jne    0x23ab
    2355:	f6 81 fe 08 3f       	testb  $0x3f,0x8fe(%bx,%di)
    235a:	01 7d c9             	add    %di,-0x37(%di)
    235d:	81 fe 06 c7          	cmp    $0xc706,%si
    2361:	00 7d c2             	add    %bh,-0x3e(%di)
    2364:	eb 0a                	jmp    0x2370
    2366:	81 08 01 c9          	orw    $0xc901,(%bx,%si)
    236a:	fe c7                	inc    %bh
    236c:	11 ff                	adc    %di,%di
    236e:	4e                   	dec    %si
    236f:	0d ff 06             	or     $0x6ff,%ax
    2372:	0d ff 06             	or     $0x6ff,%ax
    2375:	cf                   	iret
    2376:	c6 44 60 3b          	movb   $0x3b,0x60(%si)
    237a:	03 fa                	add    %dx,%di
    237c:	74 e8                	je     0x2366
    237e:	c2 5d ff             	ret    $0xff5d
    2381:	8b eb                	mov    %bx,%bp
    2383:	fc                   	cld
    2384:	e8 fe 75             	call   0x9985
    2387:	47                   	inc    %di
    2388:	fc                   	cld
    2389:	2b 48 8b             	sub    -0x75(%bx,%si),%cx
    238c:	0b f4                	or     %sp,%si
    238e:	47                   	inc    %di
    238f:	fc                   	cld
    2390:	2b 40 89             	sub    -0x77(%bx,%si),%ax
    2393:	f2 8d 8b 06 0d       	repnz lea 0xd06(%bp,%di),%cx
    2398:	46                   	inc    %si
    2399:	56                   	push   %si
    239a:	ff                   	ljmp   (bad)
    239b:	e8 cf c6             	call   0xea6d
    239e:	1d 60 3b             	sbb    $0x3b60,%ax
    23a1:	03 fa                	add    %dx,%di
    23a3:	75 81                	jne    0x2326
    23a5:	81 fe c9 fe          	cmp    $0xfec9,%si
    23a9:	7c e9                	jl     0x2394
    23ab:	02 78 ff             	add    -0x1(%bx,%si),%bh
    23ae:	56                   	push   %si
    23af:	ff                   	ljmp   (bad)
    23b0:	e8 cf c6             	call   0xea82
    23b3:	29 5f 56             	sub    %bx,0x56(%bx)
    23b6:	ff                   	ljmp   (bad)
    23b7:	e8 f4 e8             	call   0xcae
    23ba:	01 60 3b             	add    %sp,0x3b(%bx,%si)
    23bd:	03 fa                	add    %dx,%di
    23bf:	75 3a                	jne    0x23fb
    23c1:	20 fe                	and    %bh,%dh
    23c3:	74 00                	je     0x23c5
    23c5:	40                   	inc    %ax
    23c6:	56                   	push   %si
    23c7:	32 fe                	xor    %dh,%bh
    23c9:	03 ff                	add    %di,%di
    23cb:	8c e8                	mov    %gs,%ax
    23cd:	14 f8                	adc    $0xf8,%al
    23cf:	b9 01 89             	mov    $0x8901,%cx
    23d2:	fc                   	cld
    23d3:	56                   	push   %si
    23d4:	81 08 8b da          	orw    $0xda8b,(%bx,%si)
    23d8:	b8 b8 e3             	mov    $0xe3b8,%ax
    23db:	c8 9e 3e 9a          	enter  $0x3e9e,$0x9a
    23df:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    23e0:	89 31                	mov    %si,(%bx,%di)
    23e2:	8b fb                	mov    %bx,%di
    23e4:	83 fc 9a             	cmp    $0xff9a,%sp
    23e7:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    23e8:	8b f4                	mov    %sp,%si
    23ea:	44                   	inc    %sp
    23eb:	5e                   	pop    %si
    23ec:	41                   	inc    %cx
    23ed:	04 f6                	add    $0xf6,%al
    23ef:	f6 f8                	idiv   %al
    23f1:	40                   	inc    %ax
    23f2:	2b 32                	sub    (%bp,%si),%si
    23f4:	75 f6                	jne    0x23ec
    23f6:	fe                   	(bad)
    23f7:	3f                   	aas
    23f8:	f8                   	clc
    23f9:	eb 05                	jmp    0x2400
    23fb:	ff                   	(bad)
    23fc:	fa                   	cli
    23fd:	fe                   	(bad)
    23fe:	5d                   	pop    %bp
    23ff:	ff 56 ff             	call   *-0x1(%bp)
    2402:	e8 f2 e8             	call   0xcf7
    2405:	b6 5f                	mov    $0x5f,%dh
    2407:	3b 03                	cmp    (%bp,%di),%ax
    2409:	fa                   	cli
    240a:	75 3b                	jne    0x2447
    240c:	76 fc                	jbe    0x240a
    240e:	74 5e                	je     0x246e
    2410:	1a 35                	sbb    (%di),%dh
    2412:	c7                   	(bad)
    2413:	e9 03 2b             	jmp    0x4f19
    2416:	00 00                	add    %al,(%bx,%si)
    2418:	14 b9                	adc    $0xb9,%al
    241a:	89 56 08             	mov    %dx,0x8(%bp)
    241d:	da b8 c8 3e          	fidivrl 0x3ec8(%bx,%si)
    2421:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    2422:	31 fb                	xor    %di,%bx
    2424:	fc                   	cld
    2425:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    2426:	f4                   	hlt
    2427:	5e                   	pop    %si
    2428:	04 f6                	add    $0xf6,%al
    242a:	40                   	inc    %ax
    242b:	32 f6                	xor    %dh,%dh
    242d:	3f                   	aas
    242e:	eb ff                	jmp    0x242f
    2430:	fe                   	(bad)
    2431:	ff                   	(bad)
    2432:	ff f2                	push   %dx
    2434:	b6 f2                	mov    $0xf2,%dh
    2436:	5e                   	pop    %si
    2437:	04 f6                	add    $0xf6,%al
    2439:	40                   	inc    %ax
    243a:	32 f6                	xor    %dh,%dh
    243c:	3f                   	aas
    243d:	eb ff                	jmp    0x243e
    243f:	fe                   	(bad)
    2440:	ff                   	(bad)
    2441:	ff f2                	push   %dx
    2443:	b6 e9                	mov    $0xe9,%dh
    2445:	51                   	push   %cx
    2446:	ff 02                	incw   (%bp,%si)
    2448:	2b 76 fc             	sub    -0x4(%bp),%si
    244b:	74 e9                	je     0x2436
    244d:	49                   	dec    %cx
    244e:	1a a1 8c 18          	sbb    0x188c(%bx,%di),%ah
    2452:	05 08 e8             	add    $0xe808,%ax
    2455:	02 04                	add    (%si),%al
    2457:	00 83 56 14          	add    %al,0x1456(%bp,%di)
    245b:	76 ff                	jbe    0x245c
    245d:	08 57 56             	or     %dl,0x56(%bx)
    2460:	c6                   	(bad)
    2461:	fa                   	cli
    2462:	fe                   	(bad)
    2463:	5d                   	pop    %bp
    2464:	89 89 f2 8d          	mov    %cx,-0x720e(%bx,%di)
    2468:	44                   	inc    %sp
    2469:	5e                   	pop    %si
    246a:	f0 89 7e f9          	lock mov %di,-0x7(%bp)
    246e:	f2 e8 69 11          	bnd call 0x35db
    2472:	2a c0                	sub    %al,%al
    2474:	88 fd                	mov    %bh,%ch
    2476:	4d                   	dec    %bp
    2477:	88 ff                	mov    %bh,%bh
    2479:	80 eb 14             	sub    $0x14,%bl
    247c:	b0 0c                	mov    $0xc,%al
    247e:	f6 66 ff             	mulb   -0x1(%bp)
    2481:	8b d8                	mov    %ax,%bx
    2483:	80 bf 16 11 74       	cmpb   $0x74,0x1116(%bx)
    2488:	03 eb                	add    %bx,%bp
    248a:	fe                   	(bad)
    248b:	fd                   	std
    248c:	4d                   	dec    %bp
    248d:	fe                   	(bad)
    248e:	fd                   	std
    248f:	54                   	push   %sp
    2490:	7e 01                	jle    0x2493
    2492:	ff 03                	incw   (%bp,%di)
    2494:	72 e6                	jb     0x247c
    2496:	c6                   	(bad)
    2497:	fa                   	cli
    2498:	03 2b                	add    (%bp,%di),%bp
    249a:	80 fc e6             	cmp    $0xe6,%ah
    249d:	03 73 3a             	add    0x3a(%bp,%di),%si
    24a0:	46                   	inc    %si
    24a1:	3c 2b                	cmp    $0x2b,%al
    24a3:	2a 89 3b 46          	sub    0x463b(%bx,%di),%cl
    24a7:	ec                   	in     (%dx),%al
    24a8:	b9 43 04             	mov    $0x443,%cx
    24ab:	d8 19                	fcomps (%bx,%di)
    24ad:	74 89                	je     0x2438
    24af:	89 5e ee             	mov    %bx,-0x12(%bp)
    24b2:	8a 87 14 11          	mov    0x1114(%bx),%al
    24b6:	b1 03                	mov    $0x3,%cl
    24b8:	8a 5e 04             	mov    0x4(%bp),%bl
    24bb:	e8 fa d3             	call   0xf8b8
    24be:	e3 38                	jcxz   0x24f8
    24c0:	87 ec                	xchg   %bp,%sp
    24c2:	50                   	push   %ax
    24c3:	75 0a                	jne    0x24cf
    24c5:	06                   	push   %es
    24c6:	f6 74 80             	divb   -0x80(%si)
    24c9:	bf 16 11             	mov    $0x1116,%di
    24cc:	74 75                	je     0x2543
    24ce:	05 fe fd             	add    $0xfdfe,%ax
    24d1:	fc                   	cld
    24d2:	eb c6                	jmp    0x249a
    24d4:	46                   	inc    %si
    24d5:	3c 2b                	cmp    $0x2b,%al
    24d7:	88 ff                	mov    %bh,%bh
    24d9:	fe                   	(bad)
    24da:	ff 00                	incw   (%bx,%si)
    24dc:	fe                   	(bad)
    24dd:	5d                   	pop    %bp
    24de:	75 1d                	jne    0x24fd
    24e0:	c6 03 80             	movb   $0x80,(%bp,%di)
    24e3:	e6 73                	out    %al,$0x73
    24e5:	46                   	inc    %si
    24e6:	2b 89 46 13          	sub    0x1346(%bx,%di),%cx
    24ea:	b0 0c                	mov    $0xc,%al
    24ec:	f6 66 fc             	mulb   -0x4(%bp)
    24ef:	8b 80 16 74          	mov    0x7416(%bx,%si),%ax
    24f3:	eb fd                	jmp    0x24f2
    24f5:	fe                   	(bad)
    24f6:	54                   	push   %sp
    24f7:	05 fe fd             	add    $0xfdfe,%ax
    24fa:	fc                   	cld
    24fb:	eb e7                	jmp    0x24e4
    24fd:	8a 03                	mov    (%bp,%di),%al
    24ff:	66 3b 46 03          	cmp    0x3(%bp),%eax
    2503:	05 3c 5d             	add    $0x5d3c,%ax
    2506:	75 0a                	jne    0x2512
    2508:	b8 74 12             	mov    $0x1274,%ax
    250b:	c7 c5 89 34          	mov    $0x3489,%bp
    250f:	e9 6d 01             	jmp    0x267f
    2512:	8a 71 2a             	mov    0x2a(%bx,%di),%dh
    2515:	02 b8 74 38          	add    0x3874(%bx,%si),%bh
    2519:	bf 68 59             	mov    $0x5968,%di
    251c:	75 b8                	jne    0x24d6
    251e:	eb 18                	jmp    0x2538
    2520:	12 eb                	adc    %bl,%ch
    2522:	e8 8a 03             	call   0x28af
    2525:	66 3b 46 5d          	cmp    0x5d(%bp),%eax
    2529:	01 07                	add    %ax,(%bx)
    252b:	33 06 33 07          	xor    0x733,%ax
    252f:	8c 12                	mov    %ss,(%bp,%si)
    2531:	eb 80                	jmp    0x24b3
    2533:	16                   	push   %ss
    2534:	7e 03                	jle    0x2539
    2536:	03 75 0b             	add    0xb(%di),%si
    2539:	ff 00                	incw   (%bx,%si)
    253b:	fe                   	(bad)
    253c:	5d                   	pop    %bp
    253d:	75 33                	jne    0x2572
    253f:	07                   	pop    %es
    2540:	b6 12                	mov    $0x12,%dh
    2542:	eb c7                	jmp    0x250b
    2544:	8a 2a                	mov    (%bp,%si),%ch
    2546:	b8 38 68             	mov    $0x6838,%ax
    2549:	75 46                	jne    0x2591
    254b:	fd                   	std
    254c:	00 38                	add    %bh,(%bx,%si)
    254e:	87 68 59             	xchg   %bp,0x59(%bx,%si)
    2551:	77 0b                	ja     0x255e
    2553:	00 5d 33             	add    %bl,0x33(%di)
    2556:	b6 eb                	mov    $0xeb,%dh
    2558:	8a b8 44 12          	mov    0x1244(%bx,%si),%bh
    255c:	eb ad                	jmp    0x250b
    255e:	98                   	cbtw
    255f:	fb                   	sti
    2560:	87 8b f1 f7          	xchg   %cx,-0x80f(%bp,%di)
    2564:	46                   	inc    %si
    2565:	2b 89 46 b9          	sub    -0x46ba(%bx,%di),%cx
    2569:	04 19                	add    $0x19,%al
    256b:	89 87 8b c2          	mov    %ax,-0x3d75(%bx)
    256f:	04 b8                	add    $0xb8,%al
    2571:	43                   	inc    %bx
    2572:	04 d8                	add    $0xd8,%al
    2574:	6e                   	outsb  %ds:(%si),(%dx)
    2575:	ec                   	in     (%dx),%al
    2576:	74 89                	je     0x2501
    2578:	89 5e ee             	mov    %bx,-0x12(%bp)
    257b:	8b 87 0e 11          	mov    0x110e(%bx),%ax
    257f:	fa                   	cli
    2580:	8d                   	lea    (bad),%ax
    2581:	c3                   	ret
    2582:	ca 87 10             	lret   $0x1087
    2585:	11 fa                	adc    %di,%dx
    2587:	8d                   	lea    (bad),%di
    2588:	f8                   	clc
    2589:	ca 87 12             	lret   $0x1287
    258c:	11 fa                	adc    %di,%dx
    258e:	8d                   	lea    (bad),%di
    258f:	fa                   	cli
    2590:	c6 03 ff             	movb   $0xff,(%bp,%di)
    2593:	00 8a 5e ff          	add    %cl,-0xa2(%bp,%si)
    2597:	2a b8 b8 22          	sub    0x22b8(%bx,%si),%bh
    259b:	c7 87 d4 12 00 00    	movw   $0x0,0x12d4(%bx)
    25a1:	fe                   	(bad)
    25a2:	54                   	push   %sp
    25a3:	01 03                	add    %ax,(%bp,%di)
    25a5:	e6 fa                	out    %al,$0xfa
    25a7:	2b fc                	sub    %sp,%di
    25a9:	ea 8b 7e f0 8b       	ljmp   $0x8bf0,$0x7e8b
    25ae:	76 f2                	jbe    0x25a2
    25b0:	eb 5f                	jmp    0x2611
    25b2:	b8 dc 11             	mov    $0x11dc,%ax
    25b5:	63 e8                	arpl   %bp,%ax
    25b7:	39 0e 2f 8b          	cmp    %cx,-0x74d1
    25bb:	f8                   	clc
    25bc:	b8 dc 11             	mov    $0x11dc,%ax
    25bf:	63 e8                	arpl   %bp,%ax
    25c1:	3c 30                	cmp    $0x30,%al
    25c3:	c7                   	(bad)
    25c4:	c8 3d 00 1a          	enter  $0x3d,$0x1a
    25c8:	08 32                	or     %dh,(%bp,%si)
    25ca:	77 33                	ja     0x25ff
    25cc:	0a e8                	or     %al,%ch
    25ce:	0e                   	push   %cs
    25cf:	74 fe                	je     0x25cf
    25d1:	2c 2b                	sub    $0x2b,%al
    25d3:	16                   	push   %ss
    25d4:	fe                   	(bad)
    25d5:	2c 2b                	sub    $0x2b,%al
    25d7:	1b 74 10             	sbb    0x10(%si),%si
    25da:	db 20                	(bad) (%bx,%si)
    25dc:	eb 21                	jmp    0x25ff
    25de:	ff                   	ljmp   (bad)
    25df:	ec                   	in     (%dx),%al
    25e0:	06                   	push   %es
    25e1:	57                   	push   %di
    25e2:	5e                   	pop    %si
    25e3:	e8 f6 e8             	call   0xedc
    25e6:	e8 00 eb             	call   0x10e9
    25e9:	15 ff 06             	adc    $0x6ff,%ax
    25ec:	5e                   	pop    %si
    25ed:	f6 e8                	imul   %al
    25ef:	eb f8                	jmp    0x25e9
    25f1:	eb f2                	jmp    0x25e5
    25f3:	ff 5e e8             	lcall  *-0x18(%bp)
    25f6:	f8                   	clc
    25f7:	f2 5e                	repnz pop %si
    25f9:	fa                   	cli
    25fa:	eb e9                	jmp    0x25e5
    25fc:	54                   	push   %sp
    25fd:	1d b8 b8             	sbb    $0xb8b8,%ax
    2600:	dc 11                	fcoml  (%bx,%di)
    2602:	63 e8                	arpl   %bp,%ax
    2604:	2b c9                	sub    %cx,%cx
    2606:	51                   	push   %cx
    2607:	e8 51 32             	call   0x585b
    260a:	b8 dc 11             	mov    $0x11dc,%ax
    260d:	63 e8                	arpl   %bp,%ax
    260f:	d9 2f                	fldcw  (%bx)
    2611:	98                   	cbtw
    2612:	74 89                	je     0x259d
    2614:	9d                   	popf
    2615:	c6 03 ff             	movb   $0xff,(%bp,%di)
    2618:	00 8b f4 04          	add    %cl,0x4f4(%bp,%di)
    261c:	eb 18                	jmp    0x2636
    261e:	b8 06 04             	mov    $0x406,%ax
    2621:	6e                   	outsb  %ds:(%si),(%dx)
    2622:	74 89                	je     0x25ad
    2624:	ee                   	out    %al,(%dx)
    2625:	87 30                	xchg   %si,(%bx,%si)
    2627:	99                   	cwtd
    2628:	01 46 b9             	add    %ax,-0x47(%bp)
    262b:	03 d8                	add    %ax,%bx
    262d:	87 df                	xchg   %bx,%di
    262f:	87 b7 0e 11          	xchg   %si,0x110e(%bx)
    2633:	fe 01                	incb   (%bx,%di)
    2635:	e6 2b                	out    %al,$0x2b
    2637:	ea 7e 8b f2 e2       	ljmp   $0xe2f2,$0x8b7e
    263c:	00 5d 33             	add    %bl,0x33(%di)
    263f:	b6 eb                	mov    $0xeb,%dh
    2641:	25 98 74             	and    $0x7498,%ax
    2644:	89 36 06 74          	mov    %si,0x7406
    2648:	bf c6 87             	mov    $0x87c6,%di
    264b:	16                   	push   %ss
    264c:	11 02                	adc    %ax,(%bp,%si)
    264e:	c6 87 17 11 75       	movb   $0x75,0x1117(%bx)
    2653:	fd                   	std
    2654:	38 04                	cmp    %al,(%si)
    2656:	88 87 15 11          	mov    %al,0x1115(%bx)
    265a:	63 e8                	arpl   %bp,%ax
    265c:	a9 03 06             	test   $0x603,%ax
    265f:	74 bf                	je     0x2620
    2661:	88 87 14 eb          	mov    %al,-0x14ec(%bx)
    2665:	8a 15                	mov    (%di),%dl
    2667:	8b 0c                	mov    (%si),%cx
    2669:	1a 03                	sbb    (%bp,%di),%al
    266b:	8d                   	lea    (bad),%di
    266c:	f8                   	clc
    266d:	03 8d f6 3b          	add    0x3bf6(%di),%cx
    2671:	c6                   	(bad)
    2672:	73 08                	jae    0x267c
    2674:	8b 89 bf 87          	mov    -0x7841(%bx,%di),%cx
    2678:	11 c6                	adc    %ax,%si
    267a:	17                   	pop    %ss
    267b:	75 e8                	jne    0x2665
    267d:	0b f7                	or     %di,%si
    267f:	5e                   	pop    %si
    2680:	f6 57 06             	notb   0x6(%bx)
    2683:	e8 b6 06             	call   0x2d3c
    2686:	ff 83 d6 f9          	incw   -0x62a(%bp,%di)
    268a:	13 e4                	adc    %sp,%sp
    268c:	04 56                	add    $0x56,%al
    268e:	e8 46 c6             	call   0xecd7
    2691:	03 ff                	add    %di,%di
    2693:	03 b0 f6 ff          	add    -0xa(%bx,%si),%si
    2697:	d8 bf 11 03          	fdivrs 0x311(%bx)
    269b:	89 5e fc             	mov    %bx,-0x4(%bp)
    269e:	8a 14                	mov    (%si),%dl
    26a0:	b1 8a                	mov    $0x8a,%cl
    26a2:	04 fa                	add    $0xfa,%al
    26a4:	e3 87                	jcxz   0x262d
    26a6:	50                   	push   %ax
    26a7:	0a f6                	or     %dh,%dh
    26a9:	80 16 74 05 fd       	adcb   $0xfd,0x574
    26ae:	eb 46                	jmp    0x26f6
    26b0:	0d b7 9a             	or     $0x9ab7,%ax
    26b3:	fc                   	cld
    26b4:	80 16 74 eb fd       	adcb   $0xfd,-0x148c
    26b9:	fe                   	(bad)
    26ba:	54                   	push   %sp
    26bb:	54                   	push   %sp
    26bc:	1d b8 fe             	sbb    $0xfeb8,%ax
    26bf:	e6 ea                	out    %al,$0xea
    26c1:	8b e2                	mov    %dx,%sp
    26c3:	5d                   	pop    %bp
    26c4:	06                   	push   %es
    26c5:	72 cd                	jb     0x2694
    26c7:	8b 5e 83             	mov    -0x7d(%bp),%bx
    26ca:	5e                   	pop    %si
    26cb:	3b 76 8b             	cmp    -0x75(%bp),%si
    26ce:	06                   	push   %es
    26cf:	83 f9 e4             	cmp    $0xffe4,%cx
    26d2:	56                   	push   %si
    26d3:	46                   	inc    %si
    26d4:	03 0c                	add    (%si),%cx
    26d6:	56                   	push   %si
    26d7:	46                   	inc    %si
    26d8:	04 37                	add    $0x37,%al
    26da:	03 2b                	add    (%bp,%di),%bp
    26dc:	89 fc                	mov    %di,%sp
    26de:	fe                   	(bad)
    26df:	fa                   	cli
    26e0:	b8 54 89             	mov    $0x8954,%ax
    26e3:	fb                   	sti
    26e4:	f9                   	stc
    26e5:	4b                   	dec    %bx
    26e6:	89 fb                	mov    %di,%bx
    26e8:	f9                   	stc
    26e9:	ea 89 fb f9 83       	ljmp   $0x83f9,$0xfb89
    26ee:	fe                   	(bad)
    26ef:	f6 51 28             	notb   0x28(%bx,%di)
    26f2:	42                   	inc    %dx
    26f3:	57                   	push   %di
    26f4:	e8 5b 01             	call   0x2852
    26f7:	b8 06 04             	mov    $0x406,%ax
    26fa:	6e                   	outsb  %ds:(%si),(%dx)
    26fb:	66 08 8b d8 03       	data32 or %cl,0x3d8(%bp,%di)
    2700:	df 87 5e 81          	filds  -0x7ea2(%bx)
    2704:	c3                   	ret
    2705:	0e                   	push   %cs
    2706:	fa                   	cli
    2707:	c3                   	ret
    2708:	8a 37                	mov    (%bx),%dh
    270a:	fa                   	cli
    270b:	07                   	pop    %es
    270c:	68 c7 6b             	push   $0x6bc7
    270f:	eb 4a                	jmp    0x275b
    2711:	63 e8                	arpl   %bp,%ax
    2713:	75 fc                	jne    0x2711
    2715:	e8 c3 0e             	call   0x35db
    2718:	e9 06 01             	jmp    0x2821
    271b:	c7 06 fa 9a 0f 00    	movw   $0xf,-0x6506
    2721:	b8 65 89             	mov    $0x8965,%ax
    2724:	fb                   	sti
    2725:	f9                   	stc
    2726:	54                   	push   %sp
    2727:	89 fb                	mov    %di,%bx
    2729:	f9                   	stc
    272a:	77 89                	ja     0x26b5
    272c:	fb                   	sti
    272d:	b9 5f f9             	mov    $0xf95f,%cx
    2730:	e8 4e 40             	call   0x6781
    2733:	57                   	push   %di
    2734:	19 69 0f             	sbb    %bp,0xf(%bx,%di)
    2737:	bf 2b 00             	mov    $0x2b,%di
    273a:	b9 df 87             	mov    $0x87df,%cx
    273d:	5e                   	pop    %si
    273e:	81 c3 d4 12          	add    $0x12d4,%bx
    2742:	89 f6                	mov    %si,%si
    2744:	7e fa                	jle    0x2740
    2746:	07                   	pop    %es
    2747:	03 c6                	add    %si,%ax
    2749:	99                   	cwtd
    274a:	50                   	push   %ax
    274b:	63 f9                	arpl   %di,%cx
    274d:	77 89                	ja     0x26d8
    274f:	fb                   	sti
    2750:	b8 55 fe             	mov    $0xfe55,%ax
    2753:	f6 51 71             	notb   0x71(%bx,%di)
    2756:	63 a1 46 63          	arpl   %sp,0x6346(%bx,%di)
    275a:	06                   	push   %es
    275b:	9a 46 b8 c2 89       	lcall  $0x89c2,$0xb846
    2760:	fb                   	sti
    2761:	b9 74 f9             	mov    $0xf974,%cx
    2764:	e8 ba e5             	call   0xd21
    2767:	00 52 ba             	add    %dl,-0x46(%bp,%si)
    276a:	7e 00                	jle    0x276c
    276c:	52                   	push   %dx
    276d:	e8 04 57             	call   0x7e74
    2770:	69 bf 00 df 5e c3    	imul   $0xc35e,-0x2100(%bx),%di
    2776:	12 75 fc             	adc    -0x4(%di),%dh
    2779:	2b d2                	sub    %dx,%dx
    277b:	50                   	push   %ax
    277c:	63 f9                	arpl   %di,%cx
    277e:	c2 89 fb             	ret    $0xfb89
    2781:	b8 74 f9             	mov    $0xf974,%ax
    2784:	f6 51 16             	notb   0x16(%bx,%di)
    2787:	63 b8 ca 11          	arpl   %di,0x11ca(%bx,%si)
    278b:	e8 03 71             	call   0x9891
    278e:	01 3d                	add    %di,(%di)
    2790:	1a 32                	sbb    (%bp,%si),%dh
    2792:	33 44 76             	xor    0x76(%si),%ax
    2795:	e9 01 88             	jmp    0xaf99
    2798:	00 0a                	add    %cl,(%bp,%si)
    279a:	0e                   	push   %cs
    279b:	fe                   	(bad)
    279c:	2b fe                	sub    %si,%di
    279e:	2b 74 37             	sub    0x37(%si),%si
    27a1:	fe                   	(bad)
    27a2:	2b 74 3a             	sub    0x3a(%si),%si
    27a5:	fe                   	(bad)
    27a6:	2b 74 5a             	sub    0x5a(%si),%si
    27a9:	eb 76                	jmp    0x2821
    27ab:	0b f6                	or     %si,%si
    27ad:	7e 0c                	jle    0x27bb
    27af:	8b 87 bf 01          	mov    0x1bf(%bx),%ax
    27b3:	37                   	aaa
    27b4:	f7 04 4e 01          	testw  $0x14e,(%si)
    27b8:	37                   	aaa
    27b9:	eb 17                	jmp    0x27d2
    27bb:	0b f6                	or     %si,%si
    27bd:	7d 13                	jge    0x27d2
    27bf:	8b 5e 8a             	mov    -0x76(%bp),%bx
    27c2:	b7 c7                	mov    $0xc7,%bh
    27c4:	52                   	push   %dx
    27c5:	c7                   	(bad)
    27c6:	8b f0                	mov    %ax,%si
    27c8:	8b 87 bf 29          	mov    0x29bf(%bx),%ax
    27cc:	37                   	aaa
    27cd:	f7 04 4e 29          	testw  $0x294e,(%si)
    27d1:	37                   	aaa
    27d2:	f7 75 fc             	divw   -0x4(%di)
    27d5:	a3 4a 63             	mov    %ax,0x634a
    27d8:	c7 c7 e8 ae          	mov    $0xaee8,%di
    27dc:	8b eb                	mov    %bx,%bp
    27de:	42                   	inc    %dx
    27df:	81 7e fc e7 03       	cmpw   $0x3e7,-0x4(%bp)
    27e4:	73 3b                	jae    0x2821
    27e6:	57                   	push   %di
    27e7:	b9 f1 e9             	mov    $0xe9f1,%cx
    27ea:	f0 ff 04             	lock incw (%si)
    27ed:	d8 36 8a 87          	fdivs  -0x7876
    27f1:	fb                   	sti
    27f2:	11 2a                	adc    %bp,(%bp,%si)
    27f4:	3b 03                	cmp    (%bp,%di),%ax
    27f6:	78 fe                	js     0x27f6
    27f8:	77 27                	ja     0x2821
    27fa:	ff                   	(bad)
    27fb:	fc                   	cld
    27fc:	85 46 29             	test   %ax,0x29(%bp)
    27ff:	78 fe                	js     0x27ff
    2801:	eb 1e                	jmp    0x2821
    2803:	04 04                	add    $0x4,%al
    2805:	06                   	push   %es
    2806:	76 0b                	jbe    0x2813
    2808:	fc                   	cld
    2809:	76 16                	jbe    0x2821
    280b:	ff 4e fc             	decw   -0x4(%bp)
    280e:	4e                   	dec    %si
    280f:	57                   	push   %di
    2810:	f1                   	int1
    2811:	f0 04 36             	lock add $0x36,%al
    2814:	87 11                	xchg   %dx,(%bx,%di)
    2816:	3b 78 77             	cmp    0x77(%bx,%si),%di
    2819:	ff 85 29 fe          	incw   -0x1d7(%di)
    281d:	1e                   	push   %ds
    281e:	01 fe                	add    %di,%si
    2820:	b8 04 7e             	mov    $0x7e04,%ax
    2823:	fa                   	cli
    2824:	f9                   	stc
    2825:	03 9f 3e f1          	add    -0xec2(%bx),%bx
    2829:	fe                   	(bad)
    282a:	b8 89 f9             	mov    $0xf989,%ax
    282d:	89 f9                	mov    %di,%cx
    282f:	89 f9                	mov    %di,%cx
    2831:	fe                   	(bad)
    2832:	51                   	push   %cx
    2833:	42                   	inc    %dx
    2834:	e8 01 06             	call   0x2e38
    2837:	6e                   	outsb  %ds:(%si),(%dx)
    2838:	08 d8                	or     %bl,%al
    283a:	df 36 41 ff          	fbstp  -0xbf
    283e:	36 4a                	ss dec %dx
    2840:	63 e8                	arpl   %bp,%ax
    2842:	a1 1c ff             	mov    0xff1c,%ax
    2845:	0d 06 e8             	or     $0xe806,%ax
    2848:	e8 01 89             	call   0xb14c
    284b:	e8 ff 8b             	call   0xb44d
    284e:	e8 57 b1             	call   0xd9a8
    2851:	89 76 06             	mov    %si,0x6(%bp)
    2854:	30 e8                	xor    %ch,%al
    2856:	00 0c                	add    %cl,(%si)
    2858:	ba 00 01             	mov    $0x100,%dx
    285b:	f5                   	cmc
    285c:	1a c0                	sbb    %al,%al
    285e:	24 02                	and    $0x2,%al
    2860:	a2 d8 11             	mov    %al,0x11d8
    2863:	b8 ca 11             	mov    $0x11ca,%ax
    2866:	e8 00 3e             	call   0x6669
    2869:	e7 3a                	out    %ax,$0x3a
    286b:	04 a6                	add    $0xa6,%al
    286d:	2d c6 06             	sub    $0x6c6,%ax
    2870:	d8 11                	fcoms  (%bx,%di)
    2872:	04 36                	add    $0x36,%al
    2874:	3e ec                	ds in  (%dx),%al
    2876:	c0 b8 5a 89 b9       	sarb   $0xb9,-0x76a6(%bx,%si)
    287b:	f9                   	stc
    287c:	51                   	push   %cx
    287d:	f9                   	stc
    287e:	4e                   	dec    %si
    287f:	57                   	push   %di
    2880:	e0 65                	loopne 0x28e7
    2882:	36 3e ee             	ss ds out %al,(%dx)
    2885:	c0 b8 5a 89 b9       	sarb   $0xb9,-0x76a6(%bx,%si)
    288a:	b8 5b e7             	mov    $0xe75b,%ax
    288d:	3a 04                	cmp    (%si),%al
    288f:	d1 65 69             	shlw   $1,0x69(%di)
    2892:	00 5e 12             	add    %bl,0x12(%bp)
    2895:	fc                   	cld
    2896:	d2 b8 04 36          	sarb   %cl,0x3604(%bx,%si)
    289a:	87 66 04             	xchg   %sp,0x4(%bp)
    289d:	8b c8                	mov    %ax,%cx
    289f:	05 ee 11             	add    $0x11ee,%ax
    28a2:	e8 00 7c             	call   0xa4a5
    28a5:	89 b8 fe 51          	mov    %di,0x51fe(%bx,%si)
    28a9:	63 46 8b             	arpl   %ax,-0x75(%bp)
    28ac:	f1                   	int1
    28ad:	e8 ec 60             	call   0x899c
    28b0:	b8 da 12             	mov    $0x12da,%ax
    28b3:	b8 fe be             	mov    $0xbefe,%ax
    28b6:	89 b8 fe 51          	mov    %di,0x51fe(%bx,%si)
    28ba:	63 46 06             	arpl   %ax,0x6(%bp)
    28bd:	dd 60 8a             	frstor -0x76(%bx,%si)
    28c0:	84 85 29 fe          	test   %al,-0x1d7(%di)
    28c4:	1e                   	push   %ds
    28c5:	2b 50 f9             	sub    -0x7(%bx,%si),%dx
    28c8:	89 b8 d6 89          	mov    %di,-0x762a(%bx,%si)
    28cc:	fe                   	(bad)
    28cd:	63 06 60 84          	arpl   %ax,-0x7ba0
    28d1:	29 ca                	sub    %cx,%dx
    28d3:	61                   	popa
    28d4:	b8 e0 12             	mov    $0x12e0,%ax
    28d7:	b8 fe 72             	mov    $0x72fe,%ax
    28da:	89 b8 f9 51          	mov    %di,0x51f9(%bx,%si)
    28de:	63 ca                	arpl   %cx,%dx
    28e0:	e8 b9 60             	call   0x899c
    28e3:	83 ba 00 ff 0b       	cmpw   $0xb,-0x100(%bp,%si)
    28e8:	75 0f                	jne    0x28f9
    28ea:	b8 f3 12             	mov    $0x12f3,%ax
    28ed:	b8 fe 5a             	mov    $0x5afe,%ax
    28f0:	89 b9 b8 69          	mov    %di,0x69b8(%bx,%di)
    28f4:	63 ca                	arpl   %cx,%dx
    28f6:	e8 a3 60             	call   0x899c
    28f9:	83 3b 8b             	cmpw   $0xff8b,(%bp,%di)
    28fc:	83 e4 46             	and    $0x46,%sp
    28ff:	0c 46                	or     $0x46,%al
    2901:	37                   	aaa
    2902:	2b 56 be             	sub    -0x42(%bp),%dx
    2905:	89 01                	mov    %ax,(%bx,%di)
    2907:	83 ba 00 ff 0b       	cmpw   $0xb,-0x100(%bp,%si)
    290c:	b8 68 50             	mov    $0x5068,%ax
    290f:	7e 20                	jle    0x2931
    2911:	66 09 40 f4          	or     %eax,-0xc(%bx,%si)
    2915:	eb 52                	jmp    0x2969
    2917:	e8 2a dd             	call   0x644
    291a:	36 3e b6 0d          	ss ds mov $0xd,%dh
    291e:	0d 05 b8             	or     $0xb805,%ax
    2921:	00 e8                	add    %ch,%al
    2923:	c1 f2 83             	shl    $0x83,%dx
    2926:	ba b2 ff             	mov    $0xffb2,%dx
    2929:	0b 75 06             	or     0x6(%di),%si
    292c:	46                   	inc    %si
    292d:	0c d2                	or     $0xd2,%al
    292f:	12 e9                	adc    %cl,%ch
    2931:	83 7e da d2          	cmpw   $0xffd2,-0x26(%bp)
    2935:	12 0b                	adc    (%bp,%di),%cl
    2937:	75 06                	jne    0x293f
    2939:	e8 72 d8             	call   0x1ae
    293c:	3d 0a 00             	cmp    $0xa,%ax
    293f:	72 ff                	jb     0x2940
    2941:	46                   	inc    %si
    2942:	4e                   	dec    %si
    2943:	b2 ff                	mov    $0xff,%dl
    2945:	0b 74 12             	or     0x12(%si),%si
    2948:	46                   	inc    %si
    2949:	0c d2                	or     $0xd2,%al
    294b:	12 ff                	adc    %bh,%bh
    294d:	0b e8                	or     %ax,%bp
    294f:	74 fb                	je     0x294c
    2951:	b8 b6 0d             	mov    $0xdb6,%ax
    2954:	66 09 d3             	or     %edx,%ebx
    2957:	2b eb                	sub    %bx,%bp
    2959:	ff 46 a3             	incw   -0x5d(%bp)
    295c:	04 e8                	add    $0xe8,%al
    295e:	4a                   	dec    %dx
    295f:	da 66 09             	fisubl 0x9(%bp)
    2962:	2f                   	das
    2963:	2c 8b                	sub    $0x8b,%al
    2965:	f0 e8 8f d7          	lock call 0xf8
    2969:	81 fe 89 01          	cmp    $0x189,%si
    296d:	74 a8                	je     0x2917
    296f:	e8 18 f4             	call   0x1d8a
    2972:	8b c6                	mov    %si,%ax
    2974:	eb ff                	jmp    0x2975
    2976:	90                   	nop
    2977:	58                   	pop    %ax
    2978:	04 e8                	add    $0xe8,%al
    297a:	07                   	pop    %es
    297b:	00 83 3b 8b          	add    %al,-0x74c5(%bp,%di)
    297f:	83 e4 46             	and    $0x46,%sp
    2982:	0c 46                	or     $0x46,%al
    2984:	37                   	aaa
    2985:	2b fc                	sub    %sp,%di
    2987:	fa                   	cli
    2988:	02 56 be             	add    -0x42(%bp),%dl
    298b:	89 01                	mov    %ax,(%bx,%di)
    298d:	80 3e a2 ff 0b       	cmpb   $0xb,-0x5e
    2992:	74 2f                	je     0x29c3
    2994:	e8 17 d8             	call   0x1ae
    2997:	3d 04 00             	cmp    $0x4,%ax
    299a:	72 27                	jb     0x29c3
    299c:	e8 59 d7             	call   0xf8
    299f:	f6 06 a2 ff 08       	testb  $0x8,-0x5e
    29a4:	74 05                	je     0x29ab
    29a6:	be 46 0c             	mov    $0xc46,%si
    29a9:	eb ff                	jmp    0x29aa
    29ab:	f6 06 a2 ff e8       	testb  $0xe8,-0x5e
    29b0:	f5                   	cmc
    29b1:	74 be                	je     0x2971
    29b3:	03 00                	add    (%bx,%si),%ax
    29b5:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    29b6:	0c a5                	or     $0xa5,%al
    29b8:	ff 0b                	decw   (%bp,%di)
    29ba:	75 07                	jne    0x29c3
    29bc:	a0 a3 00             	mov    0xa3,%al
    29bf:	30 06 a2 ff          	xor    %al,-0x5e
    29c3:	81 fe 89 01          	cmp    $0x189,%si
    29c7:	75 2f                	jne    0x29f8
    29c9:	e8 de d9             	call   0x3aa
    29cc:	88 80 14 2a          	mov    %al,0x2a14(%bx,%si)
    29d0:	e4 3d                	in     $0x3d,%al
    29d2:	2b 0b                	sub    (%bp,%di),%cx
    29d4:	74 20                	je     0x29f6
    29d6:	77 10                	ja     0x29e8
    29d8:	0a fe                	or     %dh,%bh
    29da:	fe                   	(bad)
    29db:	1c 2c                	sbb    $0x2c,%al
    29dd:	0d 74 16             	or     $0x1674,%ax
    29e0:	2c 03                	sub    $0x3,%al
    29e2:	74 12                	je     0x29f6
    29e4:	2c 10                	sub    $0x10,%al
    29e6:	74 0e                	je     0x29f6
    29e8:	90                   	nop
    29e9:	58                   	pop    %ax
    29ea:	04 30                	add    $0x30,%al
    29ec:	01 b9 66 09          	add    %di,0x966(%bx,%di)
    29f0:	a1 2b 8b             	mov    0x8b2b,%ax
    29f3:	f0 4b                	lock dec %bx
    29f5:	2b 1a                	sub    (%bp,%si),%bx
    29f7:	f6 1a                	negb   (%bp,%si)
    29f9:	11 89 01 74          	adc    %cx,0x7401(%bx,%di)
    29fd:	8f                   	(bad)
    29fe:	8b 83 3b 8b          	mov    -0x74c5(%bp,%di),%ax
    2a02:	83 e4 46             	and    $0x46,%sp
    2a05:	0c 46                	or     $0x46,%al
    2a07:	37                   	aaa
    2a08:	2b fc                	sub    %sp,%di
    2a0a:	fa                   	cli
    2a0b:	54                   	push   %sp
    2a0c:	04 c6                	add    $0xc6,%al
    2a0e:	ff b0 ff 04          	push   0x4ff(%bx,%si)
    2a12:	e3 50                	jcxz   0x2a64
    2a14:	f6 16 05 eb          	notb   -0x14fb
    2a18:	0d 9a 8a             	or     $0x8a9a,%ax
    2a1b:	05 eb 0d             	add    $0xdeb,%ax
    2a1e:	3c 1b                	cmp    $0x1b,%al
    2a20:	72 07                	jb     0x2a29
    2a22:	3c fd                	cmp    $0xfd,%al
    2a24:	54                   	push   %sp
    2a25:	1d ff fd             	sbb    $0xfdff,%ax
    2a28:	e8 38 fd             	call   0x2763
    2a2b:	e8 8b e4             	call   0xeb9
    2a2e:	0c 37                	or     $0x37,%al
    2a30:	fc                   	cld
    2a31:	54                   	push   %sp
    2a32:	c6                   	(bad)
    2a33:	b0 04                	mov    $0x4,%al
    2a35:	50                   	push   %ax
    2a36:	16                   	push   %ss
    2a37:	eb 76                	jmp    0x2aaf
    2a39:	98                   	cbtw
    2a3a:	89 bc a1 89          	mov    %di,-0x765f(%si)
    2a3e:	15 f3 e8             	adc    $0xe8f3,%ax
    2a41:	98                   	cbtw
    2a42:	0b be 6a 11          	or     0x116a(%bp),%di
    2a46:	b8 04 6e             	mov    $0x6e04,%ax
    2a49:	74 89                	je     0x29d4
    2a4b:	ec                   	in     (%dx),%al
    2a4c:	f8                   	clc
    2a4d:	c7 81 c7 0e 11 eb    	movw   $0xeb11,0xec7(%bx,%di)
    2a53:	40                   	inc    %ax
    2a54:	0e                   	push   %cs
    2a55:	35 16 d6             	xor    $0xd616,%ax
    2a58:	8a 7b 0e             	mov    0xe(%bp,%di),%bh
    2a5b:	7b ff                	jnp    0x2a5c
    2a5d:	e8 8b 7b             	call   0xa5eb
    2a60:	75 06                	jne    0x2a68
    2a62:	00 79 b8             	add    %bh,-0x48(%bx,%di)
    2a65:	20 5e ff             	and    %bl,-0x1(%bp)
    2a68:	0a 34                	or     (%si),%dh
    2a6a:	e8 8b 75             	call   0x9ff8
    2a6d:	5b                   	pop    %bx
    2a6e:	5b                   	pop    %bx
    2a6f:	07                   	pop    %es
    2a70:	06                   	push   %es
    2a71:	07                   	pop    %es
    2a72:	1c 8b                	sbb    $0x8b,%al
    2a74:	d8 c6                	fadd   %st(6),%st
    2a76:	47                   	inc    %di
    2a77:	01 06 07 7b          	add    %ax,0x7b07
    2a7b:	0e                   	push   %cs
    2a7c:	7b ff                	jnp    0x2a7d
    2a7e:	34 e8                	xor    $0xe8,%al
    2a80:	70 73                	jo     0x2af5
    2a82:	5b                   	pop    %bx
    2a83:	b8 7b 62             	mov    $0x627b,%ax
    2a86:	11 7b ff             	adc    %di,-0x1(%bp,%di)
    2a89:	34 e8                	xor    $0xe8,%al
    2a8b:	65 73 5b             	gs jae 0x2ae9
    2a8e:	b8 83 81             	mov    $0x8183,%ax
    2a91:	46                   	inc    %si
    2a92:	47                   	inc    %di
    2a93:	47                   	inc    %di
    2a94:	1a 11                	sbb    (%bx,%di),%dl
    2a96:	8e 11                	mov    (%bx,%di),%ss
    2a98:	72 ba                	jb     0x2a54
    2a9a:	b8 11 e8             	mov    $0xe811,%ax
    2a9d:	2f                   	das
    2a9e:	50                   	push   %ax
    2a9f:	20 09                	and    %cl,(%bx,%di)
    2aa1:	f4                   	hlt
    2aa2:	52                   	push   %dx
    2aa3:	6f                   	outsw  %ds:(%si),(%dx)
    2aa4:	2b ff                	sub    %di,%di
    2aa6:	36 e6 c0             	ss out %al,$0xc0
    2aa9:	5a                   	pop    %dx
    2aaa:	b9 5b 3a             	mov    $0x3a5b,%cx
    2aad:	d1 60 09             	shlw   $1,0x9(%bx,%si)
    2ab0:	f4                   	hlt
    2ab1:	52                   	push   %dx
    2ab2:	ae                   	scas   %es:(%di),%al
    2ab3:	63 ff                	arpl   %di,%di
    2ab5:	36 e8 c0 5a          	ss call 0x8579
    2ab9:	52                   	push   %dx
    2aba:	5b                   	pop    %bx
    2abb:	3a d1                	cmp    %cl,%dl
    2abd:	79 09                	jns    0x2ac8
    2abf:	f4                   	hlt
    2ac0:	52                   	push   %dx
    2ac1:	9f                   	lahf
    2ac2:	63 ff                	arpl   %di,%di
    2ac4:	36 ea c0 5a 41 5b    	ss ljmp $0x5b41,$0x5ac0
    2aca:	3a d1                	cmp    %cl,%dl
    2acc:	96                   	xchg   %ax,%si
    2acd:	09 f4                	or     %si,%sp
    2acf:	52                   	push   %dx
    2ad0:	90                   	nop
    2ad1:	63 69 00             	arpl   %bp,0x0(%bx,%di)
    2ad4:	5e                   	pop    %si
    2ad5:	12 fc                	adc    %ah,%bh
    2ad7:	d2 63 e3             	shlb   %cl,-0x1d(%bp,%di)
    2ada:	a3 c8 c6             	mov    %ax,0xc6c8
    2add:	ff b7 00 11          	push   0x1100(%bx)
    2ae1:	b8 6d 5b             	mov    $0x5b6d,%ax
    2ae4:	3a d1                	cmp    %cl,%dl
    2ae6:	b7 00                	mov    $0x0,%bh
    2ae8:	f4                   	hlt
    2ae9:	52                   	push   %dx
    2aea:	f8                   	clc
    2aeb:	60                   	pusha
    2aec:	e8 9b f2             	call   0x1d8a
    2aef:	5e                   	pop    %si
    2af0:	57                   	push   %di
    2af1:	e8 06 83             	call   0xadfa
    2af4:	f9                   	stc
    2af5:	e4 56                	in     $0x56,%al
    2af7:	ff d6                	call   *%si
    2af9:	13 04                	adc    (%si),%ax
    2afb:	e8 c6 ff             	call   0x2ac4
    2afe:	b0 be                	mov    $0xbe,%al
    2b00:	16                   	push   %ss
    2b01:	11 eb                	adc    %bp,%bx
    2b03:	31 80 3c fd          	xor    %ax,-0x2c4(%bx,%si)
    2b07:	54                   	push   %sp
    2b08:	29 8a 44 fe          	sub    %cx,-0x1bc(%bp,%si)
    2b0c:	ff                   	(bad)
    2b0d:	fd                   	std
    2b0e:	e8 8b 16             	call   0x419c
    2b11:	f0 c0 1a 11          	lock rcrb $0x11,(%bp,%si)
    2b15:	3a 11                	cmp    (%bx,%di),%dl
    2b17:	72 ec                	jb     0x2b05
    2b19:	f8                   	clc
    2b1a:	16                   	push   %ss
    2b1b:	f4                   	hlt
    2b1c:	c0 52 8a ff          	rclb   $0xff,-0x76(%bp,%si)
    2b20:	b8 22 87             	mov    $0x8722,%ax
    2b23:	12 00                	adc    (%bx,%si),%al
    2b25:	c8 c6 ff b7          	enter  $0xffc6,$0xb7
    2b29:	b8 1a ff             	mov    $0xff1a,%ax
    2b2c:	b7 ba                	mov    $0xba,%bh
    2b2e:	1a e8                	sbb    %al,%ch
    2b30:	30 63 83             	xor    %ah,-0x7d(%bp,%di)
    2b33:	81 46 1a 11 5e       	addw   $0x5e11,0x1a(%bp)
    2b38:	11 72 c9             	adc    %si,-0x37(%bp,%si)
    2b3b:	5e                   	pop    %si
    2b3c:	5e                   	pop    %si
    2b3d:	c7 06 f4 2e 98 bc    	movw   $0xbc98,0x2ef4
    2b43:	f0 89 0a             	lock mov %cx,(%bp,%si)
    2b46:	7e fa                	jle    0x2b42
    2b48:	c6 80 73 ff 2b       	movb   $0x2b,-0x8d(%bx,%si)
    2b4d:	ff b0 ff bf          	push   -0x4001(%bx,%si)
    2b51:	03 ec                	add    %sp,%bp
    2b53:	f8                   	clc
    2b54:	03 5e 8a             	add    -0x76(%bp),%bx
    2b57:	f8                   	clc
    2b58:	8a b1 04 e3          	mov    -0x1cfc(%bx,%di),%dh
    2b5c:	88 4d ff             	mov    %cl,-0x1(%di)
    2b5f:	89 a1 15 e8          	mov    %sp,-0x17eb(%bx,%di)
    2b63:	f0 f1                	lock int1
    2b65:	2b eb                	sub    %bx,%bp
    2b67:	89 30                	mov    %si,(%bx,%si)
    2b69:	04 50                	add    $0x50,%al
    2b6b:	16                   	push   %ss
    2b6c:	eb fd                	jmp    0x2b6b
    2b6e:	05 0d 8a             	add    $0x8a0d,%ax
    2b71:	eb 3c                	jmp    0x2baf
    2b73:	80 ec 50             	sub    $0x50,%ah
    2b76:	88 ff                	mov    %bh,%bh
    2b78:	80 74 1b 72          	xorb   $0x72,0x1b(%si)
    2b7c:	1b 3c                	sbb    (%si),%di
    2b7e:	fd                   	std
    2b7f:	54                   	push   %sp
    2b80:	17                   	pop    %ss
    2b81:	ff 36 f2 c0          	push   -0x3f0e
    2b85:	8a d8                	mov    %al,%bl
    2b87:	22 12                	and    (%bp,%si),%dl
    2b89:	c8 ff b8 ff          	enter  $0xb8ff,$0xff
    2b8d:	ba e8 63             	mov    $0x63e8,%dx
    2b90:	81 1a 5e 72          	sbbw   $0x725e,(%bp,%si)
    2b94:	5e                   	pop    %si
    2b95:	c7                   	(bad)
    2b96:	ca 62 b8             	lret   $0xb862
    2b99:	57                   	push   %di
    2b9a:	8b 08                	mov    (%bx,%si),%cx
    2b9c:	72 cb                	jb     0x2b69
    2b9e:	e8 2d f1             	call   0x1cce
    2ba1:	e9 b2 00             	jmp    0x2c56
    2ba4:	e8 34 0a             	call   0x35db
    2ba7:	e8 f1 09             	call   0x359b
    2baa:	0a e9                	or     %cl,%ch
    2bac:	8b 10                	mov    (%bx,%si),%dx
    2bae:	80 3e 7f c0 1b       	cmpb   $0x1b,-0x3f81
    2bb3:	03 e9                	add    %cx,%bp
    2bb5:	01 9e 00 bf          	add    %bx,-0x4100(%bp)
    2bb9:	fc                   	cld
    2bba:	e9 1a 98             	jmp    0xc3d7
    2bbd:	0d 05 00             	or     $0x5,%ax
    2bc0:	b6 0d                	mov    $0xd,%dh
    2bc2:	b8 e8 f2             	mov    $0xf2e8,%ax
    2bc5:	ba ff 9e             	mov    $0x9eff,%dx
    2bc8:	40                   	inc    %ax
    2bc9:	88 ff                	mov    %bh,%bh
    2bcb:	80 3a 4d             	cmpb   $0x4d,(%bp,%si)
    2bce:	ff 75 00             	push   0x0(%di)
    2bd1:	00 f6                	add    %dh,%dh
    2bd3:	80 d4 75             	adc    $0x75,%ah
    2bd6:	ff 1b                	lcall  *(%bp,%di)
    2bd8:	72 7c                	jb     0x2c56
    2bda:	d4 75                	aam    $0x75
    2bdc:	ff                   	(bad)
    2bdd:	fd                   	std
    2bde:	54                   	push   %sp
    2bdf:	76 2b                	jbe    0x2c0c
    2be1:	eb 04                	jmp    0x2be7
    2be3:	50                   	push   %ax
    2be4:	16                   	push   %ss
    2be5:	4d                   	dec    %bp
    2be6:	ff 2a                	ljmp   *(%bp,%si)
    2be8:	e4 d3                	in     $0xd3,%al
    2bea:	f6 8b 57 f6 57       	testb  $0x57,-0x9a9(%bp,%di)
    2bef:	8b 08                	mov    (%bx,%si),%cx
    2bf1:	73 63                	jae    0x2c56
    2bf3:	8b bf 37 38          	mov    0x3837(%bx),%di
    2bf7:	e8 e4 38             	call   0x64de
    2bfa:	80 ec 50             	sub    $0x50,%ah
    2bfd:	32 46 f4             	xor    -0xc(%bp),%al
    2c00:	fe                   	(bad)
    2c01:	77 04                	ja     0x2c07
    2c03:	4b                   	dec    %bx
    2c04:	f8                   	clc
    2c05:	87 c6                	xchg   %ax,%si
    2c07:	75 0b                	jne    0x2c14
    2c09:	01 e8                	add    %bp,%ax
    2c0b:	14 f2                	adc    $0xf2,%al
    2c0d:	30 b9 09 2b          	xor    %bh,0x2b09(%bx,%di)
    2c11:	8a 66 46             	mov    0x46(%bp),%ah
    2c14:	01 33                	add    %si,(%bp,%di)
    2c16:	c2 1e 07             	ret    $0x71e
    2c19:	07                   	pop    %es
    2c1a:	8b 31                	mov    (%bx,%di),%si
    2c1c:	30 09                	xor    %cl,(%bx,%di)
    2c1e:	8a 46 33             	mov    0x33(%bp),%al
    2c21:	1e                   	push   %ds
    2c22:	07                   	pop    %es
    2c23:	31 09                	xor    %cx,(%bx,%di)
    2c25:	77 84                	ja     0x2bab
    2c27:	48                   	dec    %ax
    2c28:	75 23                	jne    0x2c4d
    2c2a:	83 14 77             	adcw   $0x77,(%si)
    2c2d:	04 4b                	add    $0x4b,%al
    2c2f:	f8                   	clc
    2c30:	88 14                	mov    %dl,(%si)
    2c32:	8a 8b 8a b1          	mov    -0x4e76(%bp,%di),%cl
    2c36:	17                   	pop    %ss
    2c37:	e3 88                	jcxz   0x2bc1
    2c39:	4d                   	dec    %bp
    2c3a:	fe                   	(bad)
    2c3b:	3c fd                	cmp    $0xfd,%al
    2c3d:	54                   	push   %sp
    2c3e:	0e                   	push   %cs
    2c3f:	b0 f6                	mov    $0xf6,%al
    2c41:	b8 66 fe             	mov    $0xfe66,%ax
    2c44:	ee                   	out    %al,(%dx)
    2c45:	30 01                	xor    %al,(%bx,%di)
    2c47:	b9 d8 88             	mov    $0x88d8,%cx
    2c4a:	14 24                	adc    $0x24,%al
    2c4c:	19 00                	sbb    %ax,(%bx,%si)
    2c4e:	f6 80 01 b9 d8       	testb  $0xd8,-0x46ff(%bx,%si)
    2c53:	88 fa                	mov    %bh,%dl
    2c55:	83 75 ff 41          	xorw   $0x41,-0x1(%di)
    2c59:	c0 e9 47             	shr    $0x47,%cl
    2c5c:	80 01 b9             	addb   $0xb9,(%bx,%di)
    2c5f:	fc                   	cld
    2c60:	31 09                	xor    %cx,(%bx,%di)
    2c62:	e6 1c                	out    %al,$0x1c
    2c64:	e8 23 f1             	call   0x1d8a
    2c67:	5e                   	pop    %si
    2c68:	57                   	push   %di
    2c69:	e8 06 83             	call   0xaf72
    2c6c:	f9                   	stc
    2c6d:	e4 56                	in     $0x56,%al
    2c6f:	46                   	inc    %si
    2c70:	03 03                	add    (%bp,%di),%ax
    2c72:	f6 d8                	neg    %al
    2c74:	02 56 fa             	add    -0x6(%bp),%dl
    2c77:	5d                   	pop    %bp
    2c78:	89 03                	mov    %ax,(%bp,%di)
    2c7a:	2b 8a 8e 1e          	sub    0x1e8e(%bp,%si),%cx
    2c7e:	07                   	pop    %es
    2c7f:	31 09                	xor    %cx,(%bx,%di)
    2c81:	dd 00                	fldl   (%bx,%si)
    2c83:	88 ff                	mov    %bh,%bh
    2c85:	80 0a e9             	orb    $0xe9,(%bp,%si)
    2c88:	8b 05                	mov    (%di),%ax
    2c8a:	b0 e9                	mov    $0xe9,%al
    2c8c:	1a ca                	sbb    %dl,%cl
    2c8e:	e6 46                	out    %al,$0x46
    2c90:	89 ff                	mov    %di,%di
    2c92:	03 08                	add    (%bx,%si),%cx
    2c94:	ee                   	out    %al,(%dx)
    2c95:	c6                   	(bad)
    2c96:	ff b0 ff 01          	push   0x1ff(%bx,%si)
    2c9a:	b9 d8 a2             	mov    $0xa2d8,%cx
    2c9d:	8c 1b                	mov    %ds,(%bp,%di)
    2c9f:	2b 51 51             	sub    0x51(%bx,%di),%dx
    2ca2:	b8 b0 f0             	mov    $0xf0b0,%ax
    2ca5:	46                   	inc    %si
    2ca6:	89 ff                	mov    %di,%di
    2ca8:	01 77 0f             	add    %si,0xf(%bx)
    2cab:	b0 bf                	mov    $0xbf,%al
    2cad:	ec                   	in     (%dx),%al
    2cae:	26 9e                	es sahf
    2cb0:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    2cb1:	8b 16 eb fe          	mov    -0x115,%dx
    2cb5:	05 fd 00             	add    $0xfd,%ax
    2cb8:	74 16                	je     0x2cd0
    2cba:	8e 1e 07 31          	mov    0x3107,%ds
    2cbe:	80 3e 9c a6 00       	cmpb   $0x0,-0x5964
    2cc3:	06                   	push   %es
    2cc4:	04 2b                	add    $0x2b,%al
    2cc6:	90                   	nop
    2cc7:	13 4b 2b             	adc    0x2b(%bp,%di),%cx
    2cca:	1a 8b 31 09          	sbb    0x931(%bp,%di),%cl
    2cce:	e2 e6                	loop   0x2cb6
    2cd0:	46                   	inc    %si
    2cd1:	89 ff                	mov    %di,%di
    2cd3:	01 76 54             	add    %si,0x54(%bp)
    2cd6:	80 9c 00 04 90       	sbbb   $0x90,0x400(%si)
    2cdb:	4b                   	dec    %bx
    2cdc:	4d                   	dec    %bp
    2cdd:	b8 7e 1b             	mov    $0x1b7e,%ax
    2ce0:	31 09                	xor    %cx,(%bx,%di)
    2ce2:	06                   	push   %es
    2ce3:	29 eb                	sub    %bp,%bx
    2ce5:	35 b8 7e             	xor    $0x7eb8,%ax
    2ce8:	1b 31                	sbb    (%bx,%di),%si
    2cea:	56                   	push   %si
    2ceb:	e8 da 27             	call   0x54c8
    2cee:	8b f0                	mov    %ax,%si
    2cf0:	81 fe ff 0b          	cmp    $0xbff,%si
    2cf4:	06                   	push   %es
    2cf5:	0c c6                	or     $0xc6,%al
    2cf7:	fe 89 f2 eb          	decb   -0x140e(%bx,%di)
    2cfb:	1f                   	pop    %ds
    2cfc:	b0 bf                	mov    $0xbf,%al
    2cfe:	ec                   	in     (%dx),%al
    2cff:	a4                   	movsb  %ds:(%si),%es:(%di)
    2d00:	9e                   	sahf
    2d01:	8b eb                	mov    %bx,%bp
    2d03:	05 00 16             	add    $0x1600,%ax
    2d06:	1e                   	push   %ds
    2d07:	31 3e 75 09          	xor    %di,0x975
    2d0b:	8a 84 9e 8b          	mov    -0x7462(%si),%al
    2d0f:	88 4d fe             	mov    %cl,-0x2(%di)
    2d12:	eb 07                	jmp    0x2d1b
    2d14:	b8 48 13             	mov    $0x1348,%ax
    2d17:	ca b9 83             	lret   $0x83b9
    2d1a:	2c 00                	sub    $0x0,%al
    2d1c:	33 eb                	xor    %bx,%bp
    2d1e:	03 74 c5             	add    -0x3b(%si),%si
    2d21:	b8 7e 1b             	mov    $0x1b7e,%ax
    2d24:	31 09                	xor    %cx,(%bx,%di)
    2d26:	d7                   	xlat   %ds:(%bx)
    2d27:	28 eb                	sub    %ch,%bl
    2d29:	29 a0 9e 8b          	sub    %sp,-0x7462(%bx,%si)
    2d2d:	88 4d fe             	mov    %cl,-0x2(%di)
    2d30:	b1 bf                	mov    $0xbf,%cl
    2d32:	ec                   	in     (%dx),%al
    2d33:	11 78 ff             	adc    %di,-0x1(%bx,%si)
    2d36:	eb 05                	jmp    0x2d3d
    2d38:	00 16 1e 31          	add    %dl,0x311e
    2d3c:	3e 46                	ds inc %si
    2d3e:	89 ff                	mov    %di,%di
    2d40:	01 76 04             	add    %si,0x4(%bp)
    2d43:	2b 6a 13             	sub    0x13(%bp,%si),%bp
    2d46:	4b                   	dec    %bx
    2d47:	03 b8 48 13          	add    0x1348(%bx,%si),%di
    2d4b:	ca b9 85             	lret   $0x85b9
    2d4e:	2c c6                	sub    $0xc6,%al
    2d50:	fe 89 f2 e8          	decb   -0x170e(%bx,%di)
    2d54:	34 f0                	xor    $0xf0,%al
    2d56:	8e 1e fe 83          	mov    -0x7c02,%ds
    2d5a:	8b e4                	mov    %sp,%sp
    2d5c:	0c 37                	or     $0x37,%al
    2d5e:	fc                   	cld
    2d5f:	02 be 01 3e          	add    0x3e01(%bp),%bh
    2d63:	ff 74 e8             	push   -0x18(%si)
    2d66:	d8 2a                	fsubrs (%bp,%si)
    2d68:	c0 a2 00 04 a2       	shlb   $0xa2,0x400(%bp,%si)
    2d6d:	a1 8b 88             	mov    0x888b,%ax
    2d70:	4d                   	dec    %bp
    2d71:	ff 2b                	ljmp   *(%bp,%di)
    2d73:	d2 be 11 31          	sarb   %cl,0x3111(%bp)
    2d77:	3c 29                	cmp    $0x29,%al
    2d79:	80 fd 29             	cmp    $0x29,%ch
    2d7c:	44                   	inc    %sp
    2d7d:	20 fe                	and    %bh,%dh
    2d7f:	06                   	push   %es
    2d80:	a1 8b 8e             	mov    0x8e8b,%ax
    2d83:	1e                   	push   %ds
    2d84:	07                   	pop    %es
    2d85:	38 44 fe             	cmp    %al,-0x2(%si)
    2d88:	75 14                	jne    0x2d9e
    2d8a:	8a b8 87 00          	mov    0x87(%bx,%si),%bh
    2d8e:	c6                   	(bad)
    2d8f:	88 97 9e 8b          	mov    %dl,-0x7462(%bx)
    2d93:	fe                   	(bad)
    2d94:	ea e2 38 3c 75       	ljmp   $0x753c,$0x38e2
    2d99:	04 fe                	add    $0xfe,%al
    2d9b:	06                   	push   %es
    2d9c:	00 04                	add    %al,(%si)
    2d9e:	83 46 11 42          	addw   $0x42,0x11(%bp)
    2da2:	1a 3a                	sbb    (%bp,%si),%bh
    2da4:	72 f8                	jb     0x2d9e
    2da6:	f4                   	hlt
    2da7:	d1 01                	rolw   $1,(%bx,%di)
    2da9:	d8 8c 3b 83          	fmuls  -0x7cc5(%si)
    2dad:	46                   	inc    %si
    2dae:	46                   	inc    %si
    2daf:	2b fa                	sub    %dx,%di
    2db1:	04 ff                	add    $0xff,%al
    2db3:	ff e3                	jmp    *%bx
    2db5:	f6 05 0d             	testb  $0xd,(%di)
    2db8:	76 89                	jbe    0x2d43
    2dba:	a1 15 e8             	mov    0xe815,%ax
    2dbd:	0b 95 ef 01          	or     0x1ef(%di),%dx
    2dc1:	d8 06 40 3b          	fadds  0x3b40
    2dc5:	99                   	cwtd
    2dc6:	d8 fa                	fdivr  %st(2),%st
    2dc8:	75 fd                	jne    0x2dc7
    2dca:	0a 8b b0 03          	or     0x3b0(%bp,%di),%cl
    2dce:	e9 ba 00             	jmp    0x2e8b
    2dd1:	52                   	push   %dx
    2dd2:	0c 90                	or     $0x90,%al
    2dd4:	1b 1e 74 ec          	sbb    -0x138c,%bx
    2dd8:	8a 96 00 a0          	mov    -0x6000(%bp),%dl
    2ddc:	5c                   	pop    %sp
    2ddd:	9b                   	fwait
    2dde:	04 02                	add    $0x2,%al
    2de0:	88 4d fe             	mov    %cl,-0x2(%di)
    2de3:	be 05 00             	mov    $0x5,%si
    2de6:	52                   	push   %dx
    2de7:	03 1c                	add    (%si),%bx
    2de9:	0c 04                	or     $0x4,%al
    2deb:	0a 8b 9c 06          	or     0x69c(%bp,%di),%cl
    2def:	81 e6 ff 0b          	and    $0xbff,%si
    2df3:	83 46 05 16          	addw   $0x16,0x5(%bp)
    2df7:	03 75 ff             	add    -0x1(%di),%si
    2dfa:	08 c6                	or     %al,%dh
    2dfc:	b0 01                	mov    $0x1,%al
    2dfe:	75 ff                	jne    0x2dff
    2e00:	16                   	push   %ss
    2e01:	ff e4                	jmp    *%sp
    2e03:	a2 1b 51             	mov    %al,0x511b
    2e06:	2a e4                	sub    %ah,%ah
    2e08:	40                   	inc    %ax
    2e09:	8a 4e fe             	mov    -0x2(%bp),%cl
    2e0c:	2a ed                	sub    %ch,%ch
    2e0e:	f7 03 08 46          	testw  $0x4608,(%bp,%di)
    2e12:	05 07 00             	add    $0x7,%ax
    2e15:	a3 84 1b             	mov    %ax,0x1b84
    2e18:	a3 52 1b             	mov    %ax,0x1b52
    2e1b:	ba fa 04             	mov    $0x4fa,%dx
    2e1e:	52                   	push   %dx
    2e1f:	2b db                	sub    %bx,%bx
    2e21:	53                   	push   %bx
    2e22:	bb 3d 01             	mov    $0x13d,%bx
    2e25:	53                   	push   %bx
    2e26:	50                   	push   %ax
    2e27:	8b f9                	mov    %cx,%di
    2e29:	e8 ef 3a             	call   0x691b
    2e2c:	b8 1b 09             	mov    $0x91b,%ax
    2e2f:	28 00                	sub    %al,(%bx,%si)
    2e31:	e7 04                	out    %ax,$0x4
    2e33:	2d 06 dd             	sub    $0xdd06,%ax
    2e36:	27                   	daa
    2e37:	69 5e fc b8 36       	imul   $0x36b8,-0x4(%bp),%bx
    2e3c:	66 8b 14             	mov    (%si),%edx
    2e3f:	13 ca                	adc    %dx,%cx
    2e41:	b8 50 04             	mov    $0x450,%ax
    2e44:	2d 56 e8             	sub    $0xe856,%ax
    2e47:	53                   	push   %bx
    2e48:	5b                   	pop    %bx
    2e49:	03 f7                	add    %di,%si
    2e4b:	c6                   	(bad)
    2e4c:	ff 8a ff b8          	decw   -0x4701(%bp,%si)
    2e50:	fd                   	std
    2e51:	38 68 ff             	cmp    %ch,-0x1(%bx,%si)
    2e54:	8a 73 1a             	mov    0x1a(%bp,%di),%dh
    2e57:	8a b8 87 00          	mov    0x87(%bx,%si),%bh
    2e5b:	c6                   	(bad)
    2e5c:	8a 87 9e 8b          	mov    -0x7462(%bx),%al
    2e60:	2d 56 e8             	sub    $0xe856,%ax
    2e63:	31 ff                	xor    %di,%di
    2e65:	b8 fd fe             	mov    $0xfefd,%ax
    2e68:	2a e4                	sub    %ah,%ah
    2e6a:	03 f0                	add    %ax,%si
    2e6c:	fe                   	(bad)
    2e6d:	ea e2 eb de 19       	ljmp   $0x19de,$0xebe2
    2e72:	69 0f bf 00          	imul   $0xbf,(%bx),%cx
    2e76:	f6 e8                	imul   %al
    2e78:	fb                   	sti
    2e79:	b6 66                	mov    $0x66,%dh
    2e7b:	a0 5b d1             	mov    0xd15b,%al
    2e7e:	00 05                	add    %al,(%di)
    2e80:	00 2d                	add    %ch,(%di)
    2e82:	06                   	push   %es
    2e83:	5f                   	pop    %di
    2e84:	5d                   	pop    %bp
    2e85:	19 69 90             	sbb    %bp,-0x70(%bx,%di)
    2e88:	1b ec                	sbb    %sp,%bp
    2e8a:	e8 e5 fc             	call   0x2b72
    2e8d:	ee                   	out    %al,(%dx)
    2e8e:	5e                   	pop    %si
    2e8f:	89 e8                	mov    %bp,%ax
    2e91:	ff 2b                	ljmp   *(%bp,%di)
    2e93:	08 7e 8b             	or     %bh,-0x75(%bp)
    2e96:	57                   	push   %di
    2e97:	ff 46 a3             	incw   -0x5d(%bp)
    2e9a:	fe 0c                	decb   (%si)
    2e9c:	c7 06 9b 00 66 00    	movw   $0x66,0x9b
    2ea2:	8a e3                	mov    %bl,%ah
    2ea4:	5e                   	pop    %si
    2ea5:	88 fe                	mov    %bh,%dh
    2ea7:	87 8b e3 03          	xchg   %cx,0x3e3(%bp,%di)
    2eab:	8a ff                	mov    %bh,%bh
    2ead:	00 b8 5b b9          	add    %bh,-0x46a5(%bx,%si)
    2eb1:	19 56 ff             	sbb    %dx,-0x1(%bp)
    2eb4:	cf                   	iret
    2eb5:	76 04                	jbe    0x2ebb
    2eb7:	8b e8                	mov    %ax,%bp
    2eb9:	d7                   	xlat   %ds:(%bx)
    2eba:	e0 5a                	loopne 0x2f16
    2ebc:	8b 00                	mov    (%bx,%si),%ax
    2ebe:	00 43 d8             	add    %al,-0x28(%bp,%di)
    2ec1:	74 89                	je     0x2e4c
    2ec3:	ee                   	out    %al,(%dx)
    2ec4:	87 8b 0e fa          	xchg   %cx,-0x5f2(%bp,%di)
    2ec8:	c3                   	ret
    2ec9:	2b f9                	sub    %cx,%di
    2ecb:	b8 89 63             	mov    $0x6389,%ax
    2ece:	68 5e ff             	push   $0xff5e
    2ed1:	0a 0b                	or     (%bp,%di),%cl
    2ed3:	04 8b                	add    $0x8b,%al
    2ed5:	f3 e8 ef 5b          	repz call 0x8ac8
    2ed9:	8b 84 10 c3          	mov    -0x3cf0(%si),%ax
    2edd:	f9                   	stc
    2ede:	89 68 ff             	mov    %bp,-0x1(%bx,%si)
    2ee1:	0b a2 5e ff          	or     -0xa2(%bp,%si),%sp
    2ee5:	0a 0b                	or     (%bp,%di),%cl
    2ee7:	04 06                	add    $0x6,%al
    2ee9:	dd 5b 8b             	fstpl  -0x75(%bp,%di)
    2eec:	84 12                	test   %dl,(%bp,%si)
    2eee:	c3                   	ret
    2eef:	89 ff                	mov    %di,%di
    2ef1:	a2 ff 0b             	mov    %al,0xbff
    2ef4:	d8 5e 0a             	fcomps 0xa(%bp)
    2ef7:	04 dd                	add    $0xdd,%al
    2ef9:	8b 12                	mov    (%bp,%si),%dx
    2efb:	cb                   	lret
    2efc:	5b                   	pop    %bx
    2efd:	8a 84 17 88          	mov    -0x77e9(%si),%al
    2f01:	fe                   	(bad)
    2f02:	fd                   	std
    2f03:	ff                   	(bad)
    2f04:	3c 75                	cmp    $0x75,%al
    2f06:	b8 05 b8             	mov    $0xb805,%ax
    2f09:	9e                   	sahf
    2f0a:	13 eb                	adc    %bx,%bp
    2f0c:	11 b0 b8 fe          	adc    %si,-0x148(%bx,%si)
    2f10:	30 bf 03 5e          	xor    %bh,0x5e03(%bx)
    2f14:	8a 87 22 19          	mov    0x1922(%bx),%al
    2f18:	2a e4                	sub    %ah,%ah
    2f1a:	20 83 89 38          	and    %al,0x3889(%bp,%di)
    2f1e:	c7                   	(bad)
    2f1f:	e3 98                	jcxz   0x2eb9
    2f21:	f3 f4                	repz hlt
    2f23:	ee                   	out    %al,(%dx)
    2f24:	0c 6f                	or     $0x6f,%al
    2f26:	b8 46 16             	mov    $0x1646,%ax
    2f29:	59                   	pop    %cx
    2f2a:	f4                   	hlt
    2f2b:	59                   	pop    %cx
    2f2c:	33 00                	xor    (%bx,%si),%ax
    2f2e:	01 0a                	add    %cx,(%bp,%si)
    2f30:	04 dd                	add    $0xdd,%al
    2f32:	8b 12                	mov    (%bp,%si),%dx
    2f34:	ae                   	scas   %es:(%di),%al
    2f35:	5c                   	pop    %sp
    2f36:	b0 bf                	mov    $0xbf,%al
    2f38:	ec                   	in     (%dx),%al
    2f39:	03 06 8b 8a          	add    -0x7575,%ax
    2f3d:	24 2a                	and    $0x2a,%al
    2f3f:	16                   	push   %ss
    2f40:	11 2a                	adc    %bp,(%bp,%si)
    2f42:	b8 b8 22             	mov    $0x22b8,%ax
    2f45:	5b                   	pop    %bx
    2f46:	ce                   	into
    2f47:	54                   	push   %sp
    2f48:	1b b8 27 01          	sbb    0x127(%bx,%si),%di
    2f4c:	04 8b                	add    $0x8b,%al
    2f4e:	ae                   	scas   %es:(%di),%al
    2f4f:	b0 ec                	mov    $0xec,%al
    2f51:	91                   	xchg   %ax,%cx
    2f52:	5c                   	pop    %sp
    2f53:	3b 46 2b             	cmp    0x2b(%bp),%ax
    2f56:	04 ff                	add    $0xff,%al
    2f58:	7e 8b                	jle    0x2ee5
    2f5a:	57                   	push   %di
    2f5b:	ff 46 a0             	incw   -0x60(%bp)
    2f5e:	9b                   	fwait
    2f5f:	02 2a                	add    (%bp,%si),%ch
    2f61:	e4 05                	in     $0x5,%al
    2f63:	5e                   	pop    %si
    2f64:	ff d1                	call   *%cx
    2f66:	e0 8a                	loopne 0x2ef2
    2f68:	0e                   	push   %cs
    2f69:	5d                   	pop    %bp
    2f6a:	02 2a                	add    (%bp,%si),%ch
    2f6c:	ed                   	in     (%dx),%ax
    2f6d:	03 c1                	add    %cx,%ax
    2f6f:	a3 1b fa             	mov    %ax,0xfa1b
    2f72:	a1 e8 95             	mov    0x95e8,%ax
    2f75:	01 dd                	add    %bx,%bp
    2f77:	ed                   	in     (%dx),%ax
    2f78:	00 04                	add    %al,(%si)
    2f7a:	06                   	push   %es
    2f7b:	27                   	daa
    2f7c:	2b 51 b0             	sub    -0x50(%bx,%di),%dx
    2f7f:	ba 3d 01             	mov    $0x13d,%dx
    2f82:	52                   	push   %dx
    2f83:	ff 36 1b fa          	push   -0x5e5
    2f87:	ec                   	in     (%dx),%al
    2f88:	91                   	xchg   %ax,%cx
    2f89:	39 00                	cmp    %ax,(%bx,%si)
    2f8b:	04 06                	add    $0x6,%al
    2f8d:	27                   	daa
    2f8e:	2b e8                	sub    %ax,%bp
    2f90:	95                   	xchg   %ax,%bp
    2f91:	b8 3d 01             	mov    $0x13d,%ax
    2f94:	71 bf                	jno    0x2f55
    2f96:	74 1b                	je     0x2fb3
    2f98:	fa                   	cli
    2f99:	ff 46 e9             	incw   -0x17(%bp)
    2f9c:	63 e8                	arpl   %bp,%ax
    2f9e:	94                   	xchg   %ax,%sp
    2f9f:	01 06 9a fc          	add    %ax,-0x366
    2fa3:	36 8b 13             	mov    %ss:(%bp,%di),%dx
    2fa6:	b8 04 56             	mov    $0x5604,%ax
    2fa9:	53                   	push   %bx
    2faa:	03 c6                	add    %si,%ax
    2fac:	8a b8 00 00          	mov    0x0(%bx,%si),%bh
    2fb0:	06                   	push   %es
    2fb1:	5d                   	pop    %bp
    2fb2:	69 e7 59 8e          	imul   $0x8e59,%di,%sp
    2fb6:	07                   	pop    %es
    2fb7:	80 9c a0 9b 02       	sbbb   $0x2,-0x6460(%si)
    2fbc:	2a e4                	sub    %ah,%ah
    2fbe:	8a 5d 2a             	mov    0x2a(%di),%bl
    2fc1:	03 a3 fa e8          	add    -0x1706(%bp,%di),%sp
    2fc5:	01 05                	add    %ax,(%di)
    2fc7:	44                   	inc    %sp
    2fc8:	e4 5d                	in     $0x5d,%al
    2fca:	69 c9 fe e8          	imul   $0xe8fe,%cx,%cx
    2fce:	ba ed 83             	mov    $0x83ed,%dx
    2fd1:	46                   	inc    %si
    2fd2:	fa                   	cli
    2fd3:	ff e3                	jmp    *%bx
    2fd5:	05 76 a1             	add    $0xa176,%ax
    2fd8:	e8 95 01             	call   0x3170
    2fdb:	42                   	inc    %dx
    2fdc:	76 a1                	jbe    0x2f7f
    2fde:	e8 95 01             	call   0x3176
    2fe1:	06                   	push   %es
    2fe2:	71 ed                	jno    0x2fd1
    2fe4:	a0 80 95             	mov    0x9580,%al
    2fe7:	01 06 75 fd          	add    %ax,-0x28b
    2feb:	80 3e a1 a6 f9       	cmpb   $0xf9,-0x595f
    2ff0:	9f                   	lahf
    2ff1:	f1                   	int1
    2ff2:	b8 e7 00             	mov    $0xe7,%ax
    2ff5:	a0 a1 a6             	mov    0xa6a1,%al
    2ff8:	04 03                	add    $0x3,%al
    2ffa:	f6 26 9b 02          	mulb   0x29b
    2ffe:	02 06 2a 03          	add    0x32a,%al
    3002:	04 0c                	add    $0xc,%al
    3004:	f7 dc                	neg    %sp
    3006:	81 c8 2d c8          	or     $0xc82d,%ax
    300a:	00 f7                	add    %dh,%bh
    300c:	d8 99 2b c2          	fcomps -0x3dd5(%bx,%di)
    3010:	d1 f8                	sar    $1,%ax
    3012:	8b f8                	mov    %ax,%di
    3014:	03 cf                	add    %di,%cx
    3016:	89 4e fa             	mov    %cx,-0x6(%bp)
    3019:	a0 2a 03             	mov    0x32a,%al
    301c:	a3 f7 f0             	mov    %ax,0xf0f7
    301f:	dc 03                	faddl  (%bp,%di)
    3021:	f7 83 05 03 00 04    	testw  $0x400,0x305(%bp,%di)
    3027:	06                   	push   %es
    3028:	27                   	daa
    3029:	57                   	push   %di
    302a:	ba 3d 01             	mov    $0x13d,%dx
    302d:	52                   	push   %dx
    302e:	51                   	push   %cx
    302f:	e8 e9 38             	call   0x691b
    3032:	00 04                	add    %al,(%si)
    3034:	06                   	push   %es
    3035:	27                   	daa
    3036:	57                   	push   %di
    3037:	b8 3d 01             	mov    $0x13d,%ax
    303a:	71 bf                	jno    0x2ffb
    303c:	76 fa                	jbe    0x3038
    303e:	ff 46 e9             	incw   -0x17(%bp)
    3041:	63 e8                	arpl   %bp,%ax
    3043:	ef                   	out    %ax,(%dx)
    3044:	13 b8 a3 56          	adc    0x56a3(%bx,%si),%di
    3048:	53                   	push   %bx
    3049:	8d 46 be             	lea    -0x42(%bp),%ax
    304c:	31 e2                	xor    %sp,%dx
    304e:	46                   	inc    %si
    304f:	6d                   	insw   (%dx),%es:(%di)
    3050:	5b                   	pop    %bx
    3051:	ff                   	(bad)
    3052:	3e e9 40 3e          	ds jmp 0x6e96
    3056:	8d 46 be             	lea    -0x42(%bp),%ax
    3059:	31 e2                	xor    %sp,%dx
    305b:	95                   	xchg   %ax,%bp
    305c:	6d                   	insw   (%dx),%es:(%di)
    305d:	5b                   	pop    %bx
    305e:	ff 19                	lcall  *(%bx,%di)
    3060:	0f 00 e8             	verw   %ax
    3063:	b6 a0                	mov    $0xa0,%dh
    3065:	8d 46 be             	lea    -0x42(%bp),%ax
    3068:	31 66 5b             	xor    %sp,0x5b(%bp)
    306b:	00 00                	add    %al,(%bx,%si)
    306d:	8d 45 05             	lea    0x5(%di),%ax
    3070:	31 e2                	xor    %sp,%dx
    3072:	70 ff                	jo     0x3073
    3074:	19 0f                	sbb    %cx,(%bx)
    3076:	00 e8                	add    %ch,%al
    3078:	8b b8 56 03          	mov    0x356(%bx,%si),%di
    307c:	8a 00                	mov    (%bx,%si),%al
    307e:	06                   	push   %es
    307f:	69 59 07 a0 02       	imul   $0x2a0,0x7(%bx,%di),%bx
    3084:	e4 5e                	in     $0x5e,%al
    3086:	d1 8a 03 ff          	rorw   $1,-0xfd(%bp,%si)
    308a:	e0 0e                	loopne 0x309a
    308c:	03 f3                	add    %bx,%si
    308e:	7a 83                	jp     0x3013
    3090:	0a 59 a0             	or     -0x60(%bx,%di),%bl
    3093:	02 e4                	add    %ah,%ah
    3095:	5e                   	pop    %si
    3096:	d1 e0                	shl    $1,%ax
    3098:	0e                   	push   %cs
    3099:	03 f0                	add    %ax,%si
    309b:	c6                   	(bad)
    309c:	8a b8 38 fe          	mov    -0x1c8(%bx,%si),%bh
    30a0:	fc                   	cld
    30a1:	ca eb 20             	lret   $0x20eb
    30a4:	b0 f6                	mov    $0xf6,%al
    30a6:	ff                   	lcall  (bad)
    30a7:	d8 bf 11 03          	fdivrs 0x311(%bx)
    30ab:	fe 4d fd             	decb   -0x3(%di)
    30ae:	7e ff                	jle    0x30af
    30b0:	72 0f                	jb     0x30c1
    30b2:	30 8a 33 07          	xor    %cl,0x733(%bp,%si)
    30b6:	a0 02 e4             	mov    0xe402,%al
    30b9:	5e                   	pop    %si
    30ba:	d1 03                	rolw   $1,(%bp,%di)
    30bc:	f0 56                	lock push %si
    30be:	e8 d5 4d             	call   0x7e96
    30c1:	fd                   	std
    30c2:	7e ff                	jle    0x30c3
    30c4:	72 c6                	jb     0x308c
    30c6:	03 80 e6 da          	add    -0x251a(%bx,%si),%ax
    30ca:	e8 2c 03             	call   0x33f9
    30cd:	04 27                	add    $0x27,%al
    30cf:	ba 01 51             	mov    $0x5101,%dx
    30d2:	b8 01 bf             	mov    $0xbf01,%ax
    30d5:	fa                   	cli
    30d6:	46                   	inc    %si
    30d7:	63 ef                	arpl   %bp,%di
    30d9:	e8 97 38             	call   0x6973
    30dc:	e8 ab ec             	call   0x1d8a
    30df:	5e                   	pop    %si
    30e0:	f6 19                	negb   (%bx,%di)
    30e2:	00 83 56 af          	add    %al,-0x50aa(%bp,%di)
    30e6:	3e 06                	ds push %es
    30e8:	c6                   	(bad)
    30e9:	8b 00                	mov    (%bx,%si),%ax
    30eb:	04 57                	add    $0x57,%al
    30ed:	76 b1                	jbe    0x30a0
    30ef:	8b a1 63 9a          	mov    -0x659d(%bx,%di),%sp
    30f3:	b8 89 b9             	mov    $0xb989,%ax
    30f6:	8d 45 04             	lea    0x4(%di),%ax
    30f9:	50                   	push   %ax
    30fa:	04 06                	add    $0x6,%al
    30fc:	0b 8a 03 ff          	or     -0xfd(%bp,%si),%cx
    3100:	00 8d 85 92          	add    %cl,-0x6d7b(%di)
    3104:	63 8b e8 06          	arpl   %cx,0x6e8(%bp,%di)
    3108:	0b 8a 71 e4          	or     -0x1b8f(%bp,%si),%cx
    310c:	69 fe 64 4d          	imul   $0x4d64,%si,%di
    3110:	32 03                	xor    (%bp,%di),%al
    3112:	8c 14                	mov    %ss,(%si)
    3114:	b9 a1 f6             	mov    $0xf6a1,%cx
    3117:	c0 03 03             	rolb   $0x3,(%bp,%di)
    311a:	8c 01                	mov    %es,(%bx,%di)
    311c:	51                   	push   %cx
    311d:	8b b0 91 3b          	mov    0x3b91(%bx,%si),%si
    3121:	3f                   	aas
    3122:	5d                   	pop    %bp
    3123:	83 81 47 3e 20       	addw   $0x20,0x3e47(%bx,%di)
    3128:	74 40                	je     0x316a
    312a:	38 7c e8             	cmp    %bh,-0x18(%si)
    312d:	89 ff                	mov    %di,%di
    312f:	08 8b ff a3          	or     %cl,-0x5c01(%bp,%di)
    3133:	0c 06                	or     $0x6,%al
    3135:	00 00                	add    %al,(%bx,%si)
    3137:	e3 88                	jcxz   0x30c1
    3139:	87 ff                	xchg   %di,%di
    313b:	83 d6 7e             	adc    $0x7e,%si
    313e:	04 ff                	add    $0xff,%al
    3140:	72 15                	jb     0x3157
    3142:	0c 03                	or     $0x3,%al
    3144:	89 fa                	mov    %di,%dx
    3146:	08 0d                	or     %cl,(%di)
    3148:	06                   	push   %es
    3149:	c6                   	(bad)
    314a:	0c 04                	or     $0x4,%al
    314c:	c0 ff 46             	sar    $0x46,%bh
    314f:	a3 fe ff             	mov    %ax,0xfffe
    3152:	cf                   	iret
    3153:	1d 3b fa             	sbb    $0xfa3b,%ax
    3156:	4d                   	dec    %bp
    3157:	32 06 9a 00          	xor    0x9a,%al
    315b:	08 f6                	or     %dh,%dh
    315d:	fb                   	sti
    315e:	66 0c c0             	data32 or $0xc0,%al
    3161:	46                   	inc    %si
    3162:	fe cf                	dec    %bh
    3164:	3b 0c                	cmp    (%si),%cx
    3166:	c0 46 1d 3b          	rolb   $0x3b,0x1d(%bp)
    316a:	f0 60                	lock pusha
    316c:	0c 03                	or     $0x3,%al
    316e:	0c 40                	or     $0x40,%al
    3170:	8b e8                	mov    %ax,%bp
    3172:	4e                   	dec    %si
    3173:	0a 41 51             	or     0x51(%bx,%di),%al
    3176:	8b e8                	mov    %ax,%bp
    3178:	56                   	push   %si
    3179:	06                   	push   %es
    317a:	4a                   	dec    %dx
    317b:	52                   	push   %dx
    317c:	8b 8b bf f9          	mov    -0x641(%bp,%di),%cx
    3180:	89 56 fe             	mov    %dx,-0x2(%bp)
    3183:	e8 d6 60             	call   0x925c
    3186:	46                   	inc    %si
    3187:	a3 fe ff             	mov    %ax,0xfffe
    318a:	cf                   	iret
    318b:	ff a3 ff 1d          	jmp    *0x1dff(%bp,%di)
    318f:	fa                   	cli
    3190:	32 9a 08 c7          	xor    -0x38f8(%bp,%si),%bl
    3194:	60                   	pusha
    3195:	0c 03                	or     $0x3,%al
    3197:	08 48 01             	or     %cl,0x1(%bx,%si)
    319a:	51                   	push   %cx
    319b:	98                   	cbtw
    319c:	ff 89 03 68          	decw   0x6803(%bx,%di)
    31a0:	6b 4a e8 b7          	imul   $0xffb7,-0x18(%bp,%si),%cx
    31a4:	60                   	pusha
    31a5:	0c 03                	or     $0x3,%al
    31a7:	0c 8a                	or     $0x8a,%al
    31a9:	03 ff                	add    %di,%di
    31ab:	00 e8                	add    %ch,%al
    31ad:	4e                   	dec    %si
    31ae:	0a 83 c1 03          	or     0x3c1(%bp,%di),%al
    31b2:	51                   	push   %cx
    31b3:	8b e8                	mov    %ax,%bp
    31b5:	56                   	push   %si
    31b6:	06                   	push   %es
    31b7:	83 ea 03             	sub    $0x3,%dx
    31ba:	52                   	push   %dx
    31bb:	89 06 76 eb          	mov    %ax,-0x148a
    31bf:	4e                   	dec    %si
    31c0:	f8                   	clc
    31c1:	75 f6                	jne    0x31b9
    31c3:	fe                   	(bad)
    31c4:	e8 95 60             	call   0x925c
    31c7:	46                   	inc    %si
    31c8:	a3 fa fa             	mov    %ax,0xfafa
    31cb:	8d 06 e8 03          	lea    0x3e8,%ax
    31cf:	08 2d                	or     %ch,(%di)
    31d1:	03 ff                	add    %di,%di
    31d3:	00 f2                	add    %dh,%dl
    31d5:	44                   	inc    %sp
    31d6:	f0 3b f0             	lock cmp %ax,%si
    31d9:	4c                   	dec    %sp
    31da:	ff                   	ljmp   (bad)
    31db:	eb 35                	jmp    0x3212
    31dd:	3b 06 ff eb          	cmp    -0x1401,%ax
    31e1:	35 3b c4             	xor    $0xc43b,%ax
    31e4:	f8                   	clc
    31e5:	8a 70 73             	mov    0x73(%bx,%si),%dh
    31e8:	60                   	pusha
    31e9:	a3 fa 06             	mov    %ax,0x6fa
    31ec:	03 2d                	add    (%di),%bp
    31ee:	56                   	push   %si
    31ef:	46                   	inc    %si
    31f0:	b8 f0 3b             	mov    $0x3bf0,%ax
    31f3:	d5 4c                	aad    $0x4c
    31f5:	32 06 9a 00          	xor    0x9a,%al
    31f9:	ff                   	ljmp   (bad)
    31fa:	e8 fb b6             	call   0xe8f8
    31fd:	0c c0                	or     $0xc0,%al
    31ff:	46                   	inc    %si
    3200:	1d ff 06             	sbb    $0x6ff,%ax
    3203:	e8 89 c3             	call   0xf58f
    3206:	4c                   	dec    %sp
    3207:	56                   	push   %si
    3208:	e8 e8 fe             	call   0x30f3
    320b:	ff 90 58 83          	call   *-0x7ca8(%bx,%si)
    320f:	b9 4c 32             	mov    $0x324c,%cx
    3212:	06                   	push   %es
    3213:	9a 00 00 e8 b6       	lcall  $0xb6e8,$0x0
    3218:	a0 0c c0             	mov    0xc00c,%al
    321b:	46                   	inc    %si
    321c:	fe cf                	dec    %bh
    321e:	3b 4d e8             	cmp    -0x18(%di),%cx
    3221:	a7                   	cmpsw  %es:(%di),%ds:(%si)
    3222:	4c                   	dec    %sp
    3223:	56                   	push   %si
    3224:	51                   	push   %cx
    3225:	b0 3b                	mov    $0x3b,%al
    3227:	58                   	pop    %ax
    3228:	83 9f 4c 5e e8       	sbbw   $0xffe8,0x5e4c(%bx)
    322d:	2b 7e 57             	sub    0x57(%bp),%di
    3230:	46                   	inc    %si
    3231:	0a 04                	or     (%si),%al
    3233:	ff f6                	push   %si
    3235:	0d 89 15             	or     $0x1589,%ax
    3238:	0b ef                	or     %di,%bp
    323a:	d8 d6                	fcom   %st(6)
    323c:	04 72                	add    $0x72,%al
    323e:	0c 89                	or     $0x89,%al
    3240:	08 06 0c c0          	or     %al,-0x3ff4
    3244:	46                   	inc    %si
    3245:	fe cf                	dec    %bh
    3247:	3b 4d 06             	cmp    0x6(%di),%cx
    324a:	00 f6                	add    %dh,%dh
    324c:	66 c0 fe 3b          	data32 sar $0x3b,%dh
    3250:	c0 1d f0             	rcrb   $0xf0,(%di)
    3253:	0c 1e                	or     $0x1e,%al
    3255:	4c                   	dec    %sp
    3256:	32 06 9a 00          	xor    0x9a,%al
    325a:	08 fb                	or     %bh,%bl
    325c:	0c 46                	or     $0x46,%al
    325e:	cf                   	iret
    325f:	0c 46                	or     $0x46,%al
    3261:	3b 60 03             	cmp    0x3(%bx,%si),%sp
    3264:	40                   	inc    %ax
    3265:	e8 0a 51             	call   0x8372
    3268:	e8 f1 e8             	call   0x1b5c
    326b:	2b 03                	sub    (%bp,%di),%ax
    326d:	40                   	inc    %ax
    326e:	e8 0a 51             	call   0x837b
    3271:	e8 06 52             	call   0x847a
    3274:	8b f9                	mov    %cx,%di
    3276:	56                   	push   %si
    3277:	e8 60 a3             	call   0xd5da
    327a:	ff                   	(bad)
    327b:	ff                   	(bad)
    327c:	ff                   	(bad)
    327d:	fa                   	cli
    327e:	9a c7 0c 08 01       	lcall  $0x108,$0xcc7
    3283:	d7                   	xlat   %ds:(%bx)
    3284:	5f                   	pop    %di
    3285:	46                   	inc    %si
    3286:	fe cf                	dec    %bh
    3288:	a3 1d 32             	mov    %ax,0x321d
    328b:	08 60 03             	or     %ah,0x3(%bx,%si)
    328e:	48                   	dec    %ax
    328f:	51                   	push   %cx
    3290:	ff 03                	incw   (%bp,%di)
    3292:	c8 e8 2b 03          	enter  $0x2be8,$0x3
    3296:	08 01                	or     %al,(%bx,%di)
    3298:	98                   	cbtw
    3299:	89 68 4a             	mov    %bp,0x4a(%bx,%si)
    329c:	b7 0c                	mov    $0xc,%bh
    329e:	0c 03                	or     $0x3,%al
    32a0:	00 4e b8             	add    %cl,-0x48(%bp)
    32a3:	5f                   	pop    %di
    32a4:	32 9a ff fb          	xor    -0x401(%bp,%si),%bl
    32a8:	0c 46                	or     $0x46,%al
    32aa:	ff                   	ljmp   (bad)
    32ab:	e8 c3 56             	call   0x8971
    32ae:	e8 ff 58             	call   0x8bb0
    32b1:	b9 32 9a             	mov    $0x9a32,%cx
    32b4:	14 4c                	adc    $0x4c,%al
    32b6:	e8 fe 90             	call   0xc3b7
    32b9:	83 4c 06 00          	orw    $0x0,0x6(%si)
    32bd:	e8 0a 4c             	call   0x7eca
    32c0:	06                   	push   %es
    32c1:	00 e8                	add    %ch,%al
    32c3:	a0 c0 fe             	mov    0xfec0,%al
    32c6:	3b e8                	cmp    %ax,%bp
    32c8:	4c                   	dec    %sp
    32c9:	51                   	push   %cx
    32ca:	3b 83 4c e8          	cmp    -0x17b4(%bp,%di),%ax
    32ce:	7e 46                	jle    0x3316
    32d0:	f8                   	clc
    32d1:	4b                   	dec    %bx
    32d2:	56                   	push   %si
    32d3:	b0 58                	mov    $0x58,%al
    32d5:	9f                   	lahf
    32d6:	5e                   	pop    %si
    32d7:	2b f0                	sub    %ax,%si
    32d9:	4b                   	dec    %bx
    32da:	5e                   	pop    %si
    32db:	2b 57 0a             	sub    0xa(%bx),%dx
    32de:	ff 0d                	decw   (%di)
    32e0:	15 ef d6             	adc    $0xd6ef,%ax
    32e3:	72 89                	jb     0x326e
    32e5:	56                   	push   %si
    32e6:	19 69 0f             	sbb    %bp,0xf(%bx,%di)
    32e9:	ff                   	(bad)
    32ea:	fb                   	sti
    32eb:	0c 46                	or     $0x46,%al
    32ed:	2b 03                	sub    (%bp,%di),%ax
    32ef:	0a e8                	or     %al,%ch
    32f1:	0a 51 0c             	or     0xc(%bx,%di),%dl
    32f4:	c0 e8 0a             	shr    $0xa,%al
    32f7:	51                   	push   %cx
    32f8:	e8 06 52             	call   0x8501
    32fb:	8b ff                	mov    %di,%di
    32fd:	ff                   	(bad)
    32fe:	fa                   	cli
    32ff:	9a e8 c7 4b 57       	lcall  $0x574b,$0xc7e8
    3304:	2b 03                	sub    (%bp,%di),%ax
    3306:	08 e8                	or     %ch,%al
    3308:	0a 56 19             	or     0x19(%bp),%dl
    330b:	e8 4e 5f             	call   0x925c
    330e:	32 9a ff fb          	xor    -0x401(%bp,%si),%bl
    3312:	08 fb                	or     %bh,%bl
    3314:	2b 03                	sub    (%bp,%di),%ax
    3316:	0a e8                	or     %al,%ch
    3318:	e8 51 c0             	call   0xf36c
    331b:	0a e8                	or     %al,%ch
    331d:	52                   	push   %dx
    331e:	e8 51 06             	call   0x3972
    3321:	8b ff                	mov    %di,%di
    3323:	9a 52 ff fa e8       	lcall  $0xe8fa,$0xff52
    3328:	4b                   	dec    %bx
    3329:	2b 08                	sub    (%bx,%si),%cx
    332b:	9d                   	popf
    332c:	4b                   	dec    %bx
    332d:	2b 08                	sub    (%bx,%si),%cx
    332f:	0a 19                	or     (%bx,%di),%bl
    3331:	4e                   	dec    %si
    3332:	e8 0a 56             	call   0x893f
    3335:	19 e8                	sbb    %bp,%ax
    3337:	23 5f 8c             	and    -0x74(%bx),%bx
    333a:	e8 14 f4             	call   0x2751
    333d:	ff 77 04             	push   0x4(%bx)
    3340:	4b                   	dec    %bx
    3341:	3e 4e                	ds dec %si
    3343:	56                   	push   %si
    3344:	8f e8 00 0c          	(bad)
    3348:	f9                   	stc
    3349:	fb                   	sti
    334a:	2b f8                	sub    %ax,%di
    334c:	fe                   	(bad)
    334d:	14 0a                	adc    $0xa,%al
    334f:	a0 d4 15             	mov    0x15d4,%al
    3352:	2a 03                	sub    (%bp,%di),%al
    3354:	fe 4a 8b             	decb   -0x75(%bp,%si)
    3357:	5b                   	pop    %bx
    3358:	76 01                	jbe    0x335b
    335a:	00 eb                	add    %ch,%bl
    335c:	8a 5e fe             	mov    -0x2(%bp),%bl
    335f:	16                   	push   %ss
    3360:	eb 9a                	jmp    0x32fc
    3362:	89 5e fa             	mov    %bx,-0x6(%bp)
    3365:	b1 05                	mov    $0x5,%cl
    3367:	d3 5e c3             	rcrw   %cl,-0x3d(%bp)
    336a:	12 7e 52             	adc    0x52(%bp),%bh
    336d:	89 fc                	mov    %di,%sp
    336f:	14 8a                	adc    $0x8a,%al
    3371:	07                   	pop    %es
    3372:	98                   	cbtw
    3373:	8a 83 04 fe          	mov    -0x1fc(%bp,%di),%al
    3377:	00 56 fe             	add    %dl,-0x2(%bp)
    337a:	01 00                	add    %ax,(%bx,%si)
    337c:	d1 00                	rolw   $1,(%bx,%si)
    337e:	b8 b0 15             	mov    $0x15b0,%ax
    3381:	50                   	push   %ax
    3382:	b9 01 f9             	mov    $0xf901,%cx
    3385:	57                   	push   %di
    3386:	65 8b 22             	mov    %gs:(%bp,%si),%sp
    3389:	b8 b0 15             	mov    $0x15b0,%ax
    338c:	50                   	push   %ax
    338d:	57                   	push   %di
    338e:	e8 37 21             	call   0x54c8
    3391:	fc                   	cld
    3392:	83 11 ff             	adcw   $0xffff,(%bx,%di)
    3395:	08 c6                	or     %al,%dh
    3397:	01 00                	add    %ax,(%bx,%si)
    3399:	b4 00                	mov    $0x0,%ah
    339b:	b8 b0 15             	mov    $0x15b0,%ax
    339e:	50                   	push   %ax
    339f:	57                   	push   %di
    33a0:	e8 01 71             	call   0xa4a4
    33a3:	a0 b5 24             	mov    0x24b5,%al
    33a6:	8a 85 d4 2a          	mov    0x2ad4(%di),%al
    33aa:	fe 8b 76 00          	decb   0x76(%bp,%di)
    33ae:	8a fe                	mov    %dh,%bh
    33b0:	eb 89                	jmp    0x333b
    33b2:	fa                   	cli
    33b3:	93                   	xchg   %ax,%bx
    33b4:	fb                   	sti
    33b5:	2b c8                	sub    %ax,%cx
    33b7:	00 08                	add    %cl,(%bx,%si)
    33b9:	77 0a                	ja     0x33c5
    33bb:	1b 77 1e             	sbb    0x1e(%bx),%si
    33be:	0a fe                	or     %dh,%bh
    33c0:	2c 12                	sub    $0x12,%al
    33c2:	fe                   	(bad)
    33c3:	74 eb                	je     0x33b0
    33c5:	6a fe                	push   $0xfffe
    33c7:	74 eb                	je     0x33b4
    33c9:	79 fe                	jns    0x33c9
    33cb:	74 eb                	je     0x33b8
    33cd:	69 fe 74 eb          	imul   $0xeb74,%si,%di
    33d1:	05 eb 08             	add    $0x8eb,%ax
    33d4:	83 ee 0a             	sub    $0xa,%si
    33d7:	ff                   	(bad)
    33d8:	fe                   	(bad)
    33d9:	ff                   	(bad)
    33da:	ff f2                	push   %dx
    33dc:	11 ff                	adc    %di,%di
    33de:	04 73                	add    $0x73,%al
    33e0:	43                   	inc    %bx
    33e1:	8a d4                	mov    %ah,%dl
    33e3:	fe                   	(bad)
    33e4:	76 8a                	jbe    0x3370
    33e6:	eb 29                	jmp    0x3411
    33e8:	63 e9                	arpl   %bp,%cx
    33ea:	01 ff                	add    %di,%di
    33ec:	36 da 15             	ficoml %ss:(%di)
    33ef:	e8 e2 25             	call   0x59d4
    33f2:	8b c6                	mov    %si,%ax
    33f4:	3d 01 f9             	cmp    $0xf901,%ax
    33f7:	7d 03                	jge    0x33fc
    33f9:	50                   	push   %ax
    33fa:	09 52 b7             	or     %dx,-0x49(%bp,%si)
    33fd:	fc                   	cld
    33fe:	16                   	push   %ss
    33ff:	88 07                	mov    %al,(%bx)
    3401:	ff 06 00 c1          	incw   -0x3f00
    3405:	80 3e d5 0e 0a       	cmpb   $0xa,0xed5
    340a:	7e 05                	jle    0x3411
    340c:	c6 06 0e 06 0a       	movb   $0xa,0x60e
    3411:	a0 8b d3             	mov    0xd38b,%al
    3414:	38 0a                	cmp    %cl,(%bp,%si)
    3416:	e8 75 0b             	call   0x3f8e
    3419:	98                   	cbtw
    341a:	d6                   	(bad)
    341b:	00 5e 19             	add    %bl,0x19(%bp)
    341e:	83 c6 d6             	add    $0xffd6,%si
    3421:	01 c7                	add    %ax,%di
    3423:	00 e8                	add    %ch,%al
    3425:	eb 8a                	jmp    0x33b1
    3427:	46                   	inc    %si
    3428:	ef                   	out    %ax,(%dx)
    3429:	97                   	xchg   %ax,%di
    342a:	e8 df 3a             	call   0x6f0c
    342d:	e9 45 ff             	jmp    0x3375
    3430:	b8 03 00             	mov    $0x3,%ax
    3433:	f7 ee                	imul   %si
    3435:	04 b9                	add    $0xb9,%al
    3437:	05 04 52             	add    $0x5204,%ax
    343a:	b7 c6                	mov    $0xc6,%bh
    343c:	99                   	cwtd
    343d:	f7 f9                	idiv   %cx
    343f:	8b 4b 1a             	mov    0x1a(%bp,%di),%cx
    3442:	94                   	xchg   %ax,%sp
    3443:	c8 f9 e8 eb          	enter  $0xe8f9,$0xeb
    3447:	ff                   	ljmp   (bad)
    3448:	e8 36 d8             	call   0xc81
    344b:	15 e8 85             	adc    $0x85e8,%ax
    344e:	25 5e e8             	and    $0xe85e,%ax
    3451:	83 e4 46             	and    $0x46,%sp
    3454:	03 d8                	add    %ax,%bx
    3456:	56                   	push   %si
    3457:	f9                   	stc
    3458:	2b fe                	sub    %si,%di
    345a:	f6 83 04 71 a0       	testb  $0xa0,0x7104(%bp,%di)
    345f:	f4                   	hlt
    3460:	e8 b8 c2             	call   0xf71b
    3463:	15 b9 f9             	adc    $0xf9b9,%ax
    3466:	65 22 b0 50 a8       	and    %gs:-0x57b0(%bx,%si),%dh
    346b:	21 b8 c2 15          	and    %di,0x15c2(%bx,%si)
    346f:	b9 83 04             	mov    $0x483,%cx
    3472:	71 a0                	jno    0x3414
    3474:	52                   	push   %dx
    3475:	20 8a 0b 7d          	and    %cl,0x7d0b(%bp,%si)
    3479:	33 33                	xor    (%bp,%di),%si
    347b:	8c 48 74             	mov    %cs,0x74(%bx,%si)
    347e:	28 eb                	sub    %ch,%bl
    3480:	7a a1                	jp     0x3423
    3482:	4c                   	dec    %sp
    3483:	63 39                	arpl   %di,(%bx,%di)
    3485:	06                   	push   %es
    3486:	4e                   	dec    %si
    3487:	15 75 09             	adc    $0x975,%ax
    348a:	b8 dc 15             	mov    $0x15dc,%ax
    348d:	b9 e8 43             	mov    $0x43e8,%cx
    3490:	25 eb 68             	and    $0x68eb,%ax
    3493:	3e 06                	ds push %es
    3495:	e9 01 05             	jmp    0x3999
    3498:	72 08                	jb     0x34a2
    349a:	e8 84 e9             	call   0x1e21
    349d:	e8 0a 7c             	call   0xb0aa
    34a0:	eb 56                	jmp    0x34f8
    34a2:	b8 f3 15             	mov    $0x15f3,%ax
    34a5:	eb e6                	jmp    0x348d
    34a7:	39 36 4a 15          	cmp    %si,0x154a
    34ab:	75 37                	jne    0x34e4
    34ad:	3e 06                	ds push %es
    34af:	e9 01 32             	jmp    0x66b3
    34b2:	73 05                	jae    0x34b9
    34b4:	b8 1e 16             	mov    $0x161e,%ax
    34b7:	eb d4                	jmp    0x348d
    34b9:	b8 c2 15             	mov    $0x15c2,%ax
    34bc:	b9 e8 3f             	mov    $0x3fe8,%cx
    34bf:	21 b8 4e 8b          	and    %di,-0x74b2(%bx,%si)
    34c3:	8b f9                	mov    %cx,%di
    34c5:	59                   	pop    %cx
    34c6:	27                   	daa
    34c7:	7d 33                	jge    0x34fc
    34c9:	33 10                	xor    (%bx,%si),%dx
    34cb:	83 2e e9 01 32       	subw   $0x32,0x1e9
    34d0:	c2 ff eb             	ret    $0xebff
    34d3:	a3 d2 15             	mov    %ax,0x15d2
    34d6:	a3 4a 15             	mov    %ax,0x154a
    34d9:	eb 17                	jmp    0x34f2
    34db:	b8 c2 15             	mov    $0x15c2,%ax
    34de:	b9 e8 08             	mov    $0x8e8,%cx
    34e1:	21 eb                	and    %bp,%bx
    34e3:	17                   	pop    %ss
    34e4:	a1 4c 63             	mov    0x634c,%ax
    34e7:	39 06 4c 15          	cmp    %ax,0x154c
    34eb:	75 05                	jne    0x34f2
    34ed:	b8 7a 16             	mov    $0x167a,%ax
    34f0:	eb 9b                	jmp    0x348d
    34f2:	e8 2c e9             	call   0x1e21
    34f5:	e8 be 7b             	call   0xb0b6
    34f8:	57                   	push   %di
    34f9:	54                   	push   %sp
    34fa:	06                   	push   %es
    34fb:	1d 98 b8             	sbb    $0xb898,%ax
    34fe:	0b b8 c2 15          	or     0x15c2(%bx,%si),%di
    3502:	b9 57 01             	mov    $0x157,%cx
    3505:	a0 24 85             	mov    0x8524,%al
    3508:	51                   	push   %cx
    3509:	23 e8                	and    %ax,%bp
    350b:	7d e8                	jge    0x34f5
    350d:	8b 5e 89             	mov    -0x77(%bp),%bx
    3510:	e8 f4 98             	call   0xce07
    3513:	f0 0a fa             	lock or %dl,%bh
    3516:	80 22 b8             	andb   $0xb8,(%bp,%si)
    3519:	a4                   	movsb  %ds:(%si),%es:(%di)
    351a:	8a 00                	mov    (%bx,%si),%al
    351c:	06                   	push   %es
    351d:	4f                   	dec    %di
    351e:	e4 fe                	in     $0xfe,%al
    3520:	4d                   	dec    %bp
    3521:	c3                   	ret
    3522:	fb                   	sti
    3523:	19 00                	sbb    %ax,(%bx,%si)
    3525:	8b 56 8a             	mov    -0x76(%bp),%dx
    3528:	06                   	push   %es
    3529:	59                   	pop    %cx
    352a:	92                   	xchg   %ax,%dx
    352b:	8b 8b 8a 7b          	mov    0x7b8a(%bp,%di),%cx
    352f:	0e                   	push   %cs
    3530:	7b 5b                	jnp    0x358d
    3532:	fe                   	(bad)
    3533:	68 5b 8a             	push   $0x8a5b
    3536:	71 2a                	jno    0x3562
    3538:	02 b8 74 b8          	add    -0x478c(%bx,%si),%bh
    353c:	fc                   	cld
    353d:	b8 7e 5b             	mov    $0x5b7e,%ax
    3540:	72 59                	jb     0x359b
    3542:	8e 49 f0             	mov    -0x10(%bx,%di),%cs
    3545:	5b                   	pop    %bx
    3546:	b8 a9 68             	mov    $0x68a9,%ax
    3549:	5b                   	pop    %bx
    354a:	8a b8 99 8b          	mov    -0x7467(%bx,%si),%bh
    354e:	8a 0e 5b 68          	mov    0x685b,%cl
    3552:	8a 2a                	mov    (%bp,%si),%ch
    3554:	9c                   	pushf
    3555:	68 5b 8a             	push   $0x8a5b
    3558:	b8 50 c7             	mov    $0xc750,%ax
    355b:	e3 98                	jcxz   0x34f5
    355d:	e8 de 50             	call   0x863e
    3560:	eb 63                	jmp    0x35c5
    3562:	75 89                	jne    0x34ed
    3564:	a2 0b 5e             	mov    %al,0x5e0b
    3567:	e8 b8 6a             	call   0xa022
    356a:	83 08 8b             	orw    $0xff8b,(%bx,%si)
    356d:	8a 5b 8a             	mov    -0x76(%bp,%di),%bl
    3570:	9c                   	pushf
    3571:	5b                   	pop    %bx
    3572:	b8 7d 68             	mov    $0x687d,%ax
    3575:	5b                   	pop    %bx
    3576:	8a b8 8a 9c          	mov    -0x6376(%bx,%si),%bh
    357a:	5b                   	pop    %bx
    357b:	b8 ab 8a             	mov    $0x8aab,%ax
    357e:	00 06 9c f9          	add    %al,-0x664
    3582:	51                   	push   %cx
    3583:	63 ca                	arpl   %cx,%dx
    3585:	54                   	push   %sp
    3586:	5e                   	pop    %si
    3587:	06                   	push   %es
    3588:	2e bc 89 7e          	cs mov $0x7e89,%sp
    358c:	c6                   	(bad)
    358d:	73 2b                	jae    0x35ba
    358f:	02 e8                	add    %al,%ch
    3591:	17                   	pop    %ss
    3592:	ce                   	into
    3593:	0a 2c                	or     (%si),%ch
    3595:	fe                   	(bad)
    3596:	f9                   	stc
    3597:	5e                   	pop    %si
    3598:	06                   	push   %es
    3599:	2e bc e8 a6          	cs mov $0xa6e8,%sp
    359d:	d0 83 b2 0b          	rolb   $1,0xbb2(%bp,%di)
    35a1:	06                   	push   %es
    35a2:	0c 12                	or     $0x12,%al
    35a4:	1a 2a                	sbb    (%bp,%si),%ch
    35a6:	9c                   	pushf
    35a7:	d0 05                	rolb   $1,(%di)
    35a9:	b6 b8                	mov    $0xb8,%dh
    35ab:	f2 ff 40 ff          	repnz incw -0x1(%bx,%si)
    35af:	3a ff                	cmp    %bh,%bh
    35b1:	33 e6                	xor    %si,%sp
    35b3:	e8 4d 00             	call   0x3603
    35b6:	0e                   	push   %cs
    35b7:	2b 2b                	sub    (%bp,%di),%bp
    35b9:	e4 a2                	in     $0xa2,%al
    35bb:	7f c0                	jg     0x357d
    35bd:	eb 19                	jmp    0x35d8
    35bf:	c6 06 7f c0 00       	movb   $0x0,-0x3f81
    35c4:	f6 06 0b 06 01       	testb  $0x1,0x60b
    35c9:	74 03                	je     0x35ce
    35cb:	b0 01                	mov    $0x1,%al
    35cd:	c3                   	ret
    35ce:	f6 06 0b 06 02       	testb  $0x2,0x60b
    35d3:	74 03                	je     0x35d8
    35d5:	b0 02                	mov    $0x2,%al
    35d7:	c3                   	ret
    35d8:	2a c0                	sub    %al,%al
    35da:	bc 89 7e             	mov    $0x7e89,%sp
    35dd:	c6                   	(bad)
    35de:	eb 0e                	jmp    0x35ee
    35e0:	e8 61 d0             	call   0x644
    35e3:	b6 f2                	mov    $0xf2,%dh
    35e5:	40                   	inc    %ax
    35e6:	3a 33                	cmp    (%bp,%di),%dh
    35e8:	e8 00 2b             	call   0x60eb
    35eb:	e4 f8                	in     $0xf8,%al
    35ed:	e5 83                	in     $0x83,%ax
    35ef:	0b 0c                	or     (%si),%cx
    35f1:	1a 9c 05 eb          	sbb    -0x14fb(%si),%bl
    35f5:	5e                   	pop    %si
    35f6:	06                   	push   %es
    35f7:	2e bc e8 df          	cs mov $0xdfe8,%sp
    35fb:	ff 2a                	ljmp   *(%bp,%si)
    35fd:	9c                   	pushf
    35fe:	ff 2a                	ljmp   *(%bp,%si)
    3600:	d9 00                	flds   (%bx,%si)
    3602:	0a 89 a1 56          	or     0x56a1(%bx,%di),%cl
    3606:	e8 46 8b             	call   0xc14f
    3609:	05 08 bf             	add    $0xbf08,%ax
    360c:	08 2b                	or     %ch,(%bp,%di)
    360e:	0a f7                	or     %bh,%dh
    3610:	e8 97 cd             	call   0x3aa
    3613:	fe                   	(bad)
    3614:	ff 75 83             	push   -0x7d(%di)
    3617:	ba 00 ff             	mov    $0xff00,%dx
    361a:	0b b8 05 e9          	or     -0x16fb(%bx,%si),%di
    361e:	c4 00                	les    (%bx,%si),%ax
    3620:	2a 3d                	sub    (%di),%bh
    3622:	0b 20                	or     (%bx,%si),%sp
    3624:	10 fe                	adc    %bh,%dh
    3626:	10 77 20             	adc    %dh,0x20(%bx)
    3629:	2c 74                	sub    $0x74,%al
    362b:	2c 0a                	sub    $0xa,%al
    362d:	2c 74                	sub    $0x74,%al
    362f:	2c 0e                	sub    $0xe,%al
    3631:	2c 74                	sub    $0x74,%al
    3633:	90                   	nop
    3634:	0a eb                	or     %bl,%ch
    3636:	12 0c                	adc    (%si),%cl
    3638:	12 0b                	adc    (%bp,%di),%cl
    363a:	06                   	push   %es
    363b:	02 00                	add    (%bx,%si),%al
    363d:	eb 06                	jmp    0x3645
    363f:	0c 12                	or     $0x12,%al
    3641:	0b 06 01 00          	or     0x1,%ax
    3645:	c6 b8                	xabort $0xb8,(bad)
    3647:	fe ca                	dec    %dl
    3649:	80 a2 0b 2f 17       	andb   $0x17,0x2f0b(%bp,%si)
    364e:	9f                   	lahf
    364f:	b8 00 91             	mov    $0x9100,%ax
    3652:	2b e4                	sub    %sp,%sp
    3654:	58                   	pop    %ax
    3655:	cb                   	lret
    3656:	3d 17 06             	cmp    $0x617,%ax
    3659:	73 fd                	jae    0x3658
    365b:	76 86                	jbe    0x35e3
    365d:	2b e4                	sub    %sp,%sp
    365f:	97                   	xchg   %ax,%di
    3660:	ca f6 06             	lret   $0x6f6
    3663:	a4                   	movsb  %ds:(%si),%es:(%di)
    3664:	00 14                	add    %dl,(%si)
    3666:	74 be                	je     0x3626
    3668:	0c 09                	or     $0x9,%al
    366a:	b7 16                	mov    $0x16,%bh
    366c:	fe                   	(bad)
    366d:	f6 a2 08 05          	mulb   0x508(%bp,%si)
    3671:	46                   	inc    %si
    3672:	eb 02                	jmp    0x3676
    3674:	f7 df                	neg    %di
    3676:	f6 a2 08 05          	mulb   0x508(%bp,%si)
    367a:	02 74 02             	add    0x2(%si),%dh
    367d:	f7 de                	neg    %si
    367f:	f6 a2 08 05          	mulb   0x508(%bp,%si)
    3683:	0c 74                	or     $0x74,%al
    3685:	1f                   	pop    %ds
    3686:	01 3e 00 2b          	add    %di,0x2b00
    368a:	a1 00 2b             	mov    0x2b00,%ax
    368d:	3b 06 c2 a2          	cmp    -0x5d3e,%ax
    3691:	d5 3d                	aad    $0x3d
    3693:	a1 c2 a2             	mov    0xa2c2,%ax
    3696:	a3 00 2b             	mov    %ax,0x2b00
    3699:	3b 06 be f9          	cmp    -0x642,%ax
    369d:	03 09                	add    (%bx,%di),%cx
    369f:	a1 be f9             	mov    0xf9be,%ax
    36a2:	a3 00 2b             	mov    %ax,0x2b00
    36a5:	f6 a2 08 05          	mulb   0x508(%bp,%si)
    36a9:	74 2c                	je     0x36d7
    36ab:	1f                   	pop    %ds
    36ac:	01 f2                	add    %si,%dx
    36ae:	40                   	inc    %ax
    36af:	3a a1 40 3a          	cmp    0x3a40(%bx,%di),%ah
    36b3:	3b 06 c0 00          	cmp    0xc0,%ax
    36b7:	d5 3d                	aad    $0x3d
    36b9:	a1 c0 00             	mov    0xc0,%ax
    36bc:	a3 40 3a             	mov    %ax,0x3a40
    36bf:	3b 06 bc f9          	cmp    -0x644,%ax
    36c3:	03 09                	add    (%bx,%di),%cx
    36c5:	a1 bc f9             	mov    0xf9bc,%ax
    36c8:	a3 40 3a             	mov    %ax,0x3a40
    36cb:	b6 40                	mov    $0x40,%dh
    36cd:	33 00                	xor    (%bx,%si),%ax
    36cf:	e4 e5                	in     $0xe5,%al
    36d1:	0b 1a                	or     (%bp,%si),%bx
    36d3:	05 10 e5             	add    $0xe510,%ax
    36d6:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    36d7:	a5                   	movsw  %ds:(%si),%es:(%di)
    36d8:	0b 07                	or     (%bx),%ax
    36da:	a3 30 a2             	mov    %ax,0xa230
    36dd:	81 89 75 e8 d9 80    	orw    $0x80d9,-0x178b(%bx,%di)
    36e3:	2a 01                	sub    (%bx,%di),%al
    36e5:	8c 83 46 f6          	mov    %es,-0x9ba(%bp,%di)
    36e9:	00 56 3e             	add    %dl,0x3e(%bp)
    36ec:	c6 06 2e bc 05       	movb   $0x5,-0x43d2
    36f1:	08 89 fa 08          	or     %cl,0x8fa(%bx,%di)
    36f5:	0d 76 08             	or     $0x876,%ax
    36f8:	a0 e4 d1             	mov    0xd1e4,%al
    36fb:	f0 e8 4d 44          	lock call 0x7b4c
    36ff:	02 40 f2             	add    -0xe(%bx,%si),%al
    3702:	8b f4                	mov    %sp,%si
    3704:	8b 1e 56 9b          	mov    -0x64aa,%bx
    3708:	8a 47 2d             	mov    0x2d(%bx),%al
    370b:	46                   	inc    %si
    370c:	04 89                	add    $0x89,%al
    370e:	8b f6                	mov    %si,%si
    3710:	a1 fa 9a             	mov    0x9afa,%ax
    3713:	7e f2                	jle    0x3707
    3715:	69 51 83 e8 46       	imul   $0x46e8,-0x7d(%bx,%di),%dx
    371a:	34 ff                	xor    $0xff,%al
    371c:	02 34                	add    (%si),%dh
    371e:	e8 7b 52             	call   0x899c
    3721:	ff 0a                	decw   (%bp,%si)
    3723:	0b 5b 31             	or     0x31(%bp,%di),%bx
    3726:	55                   	push   %bp
    3727:	03 04                	add    (%si),%ax
    3729:	f6 2b                	imulb  (%bp,%di)
    372b:	74 06                	je     0x3733
    372d:	e8 c0 3b             	call   0x72f0
    3730:	4c                   	dec    %sp
    3731:	3b 98 74 ff          	cmp    -0x8c(%bx,%si),%bx
    3735:	5e                   	pop    %si
    3736:	04 f6                	add    $0xf6,%al
    3738:	b9 03 8c             	mov    $0x8c03,%cx
    373b:	51                   	push   %cx
    373c:	b0 46                	mov    $0x46,%al
    373e:	f0 d5 32             	lock aad $0x32
    3741:	31 47 ff             	xor    %ax,-0x1(%bx)
    3744:	0a 0b                	or     (%bp,%di),%cl
    3746:	5b                   	pop    %bx
    3747:	e3 68                	jcxz   0x37b1
    3749:	5b                   	pop    %bx
    374a:	e8 fa 86             	call   0xbe47
    374d:	83 3a fe             	cmpw   $0xfffe,(%bp,%si)
    3750:	fd                   	std
    3751:	8b ff                	mov    %di,%di
    3753:	e4 f6                	in     $0xf6,%al
    3755:	3d 1b 17             	cmp    $0x171b,%ax
    3758:	b8 91 e4             	mov    $0xe491,%ax
    375b:	c3                   	ret
    375c:	00 77 0b             	add    %dh,0xb(%bx)
    375f:	2c 46                	sub    $0x46,%al
    3761:	eb 76                	jmp    0x37d9
    3763:	2c 05                	sub    $0x5,%al
    3765:	b8 91 e4             	mov    $0xe491,%ax
    3768:	b1 fb                	mov    $0xfb,%cl
    376a:	03 e8                	add    %ax,%bp
    376c:	f2 46                	repnz inc %si
    376e:	cf                   	iret
    376f:	4d                   	dec    %bp
    3770:	00 66 fe             	add    %ah,-0x2(%bp)
    3773:	74 5e                	je     0x37d3
    3775:	f6 03 51             	testb  $0x51,(%bp,%di)
    3778:	46                   	inc    %si
    3779:	d5 31                	aad    $0x31
    377b:	ff 0b                	decw   (%bp,%di)
    377d:	e3 5b                	jcxz   0x37da
    377f:	fa                   	cli
    3780:	83 f1 46             	xor    $0x46,%cx
    3783:	f6 c6 06             	test   $0x6,%dh
    3786:	06                   	push   %es
    3787:	fa                   	cli
    3788:	86 7d 1c             	xchg   %bh,0x1c(%di)
    378b:	04 06                	add    $0x6,%al
    378d:	47                   	inc    %di
    378e:	e8 51 46             	call   0x7de2
    3791:	d5 04                	aad    $0x4
    3793:	48                   	dec    %ax
    3794:	03 fe                	add    %si,%di
    3796:	27                   	daa
    3797:	76 0e                	jbe    0x37a7
    3799:	04 47                	add    $0x47,%al
    379b:	d7                   	xlat   %ds:(%bx)
    379c:	fe                   	(bad)
    379d:	7e 0a                	jle    0x37a9
    379f:	b8 38 ff             	mov    $0xff38,%ax
    37a2:	88 01                	mov    %al,(%bx,%di)
    37a4:	ff c7                	inc    %di
    37a6:	ae                   	scas   %es:(%di),%al
    37a7:	eb 83                	jmp    0x372c
    37a9:	46                   	inc    %si
    37aa:	ff 34                	push   (%si)
    37ac:	7b ff                	jnp    0x37ad
    37ae:	0b 31                	or     (%bx,%di),%si
    37b0:	ea ff 76 8b 0b       	ljmp   $0xb8b,$0x76ff
    37b5:	5b                   	pop    %bx
    37b6:	a0 54 03             	mov    0x354,%al
    37b9:	f6 74 e8             	divb   -0x18(%si)
    37bc:	3b 3b                	cmp    (%bp,%di),%di
    37be:	74 5e                	je     0x381e
    37c0:	f6 03 51             	testb  $0x51,(%bp,%di)
    37c3:	00 fe                	add    %bh,%dh
    37c5:	5e                   	pop    %si
    37c6:	03 46 31             	add    0x31(%bp),%ax
    37c9:	0b 5b 83             	or     -0x7d(%bp,%di),%bx
    37cc:	46                   	inc    %si
    37cd:	c6 06 86 1c 06       	movb   $0x6,0x1c86
    37d2:	e8 46 9e             	call   0xd61b
    37d5:	46                   	inc    %si
    37d6:	e9 74 ff             	jmp    0x374d
    37d9:	76 8b                	jbe    0x3766
    37db:	0b 5b 79             	or     0x79(%bp,%di),%bx
    37de:	54                   	push   %sp
    37df:	03 f6                	add    %si,%si
    37e1:	74 e8                	je     0x37cb
    37e3:	f8                   	clc
    37e4:	ff 4e fa             	decw   -0x6(%bp)
    37e7:	79 33                	jns    0x381c
    37e9:	19 ff                	sbb    %di,%di
    37eb:	14 fc                	adc    $0xfc,%al
    37ed:	63 a3 c6 d7          	arpl   %sp,-0x283a(%bp,%di)
    37f1:	fe                   	(bad)
    37f2:	7e 0a                	jle    0x37fe
    37f4:	c6 01 00             	movb   $0x0,(%bx,%di)
    37f7:	57                   	push   %di
    37f8:	e8 5d 54             	call   0x8c58
    37fb:	f6 e8                	imul   %al
    37fd:	3b 5e 03             	cmp    0x3(%bp),%bx
    3800:	03 f2                	add    %dx,%si
    3802:	cf                   	iret
    3803:	00 fe                	add    %bh,%dh
    3805:	5e                   	pop    %si
    3806:	03 46 31             	add    0x31(%bp),%ax
    3809:	0b 5b 83             	or     -0x7d(%bp,%di),%bx
    380c:	46                   	inc    %si
    380d:	c6 06 86 1c f8       	movb   $0xf8,0x1c86
    3812:	86 1c                	xchg   %bl,(%si)
    3814:	06                   	push   %es
    3815:	e8 46 5b             	call   0x935e
    3818:	46                   	inc    %si
    3819:	eb a2                	jmp    0x37bd
    381b:	c2 eb d2             	ret    $0xd2eb
    381e:	4b                   	dec    %bx
    381f:	1a 31                	sbb    (%bx,%di),%dh
    3821:	e2 8c                	loop   0x37af
    3823:	e8 14 f4             	call   0x2c3a
    3826:	ff 77 04             	push   0x4(%bx)
    3829:	4b                   	dec    %bx
    382a:	3e 5b                	ds pop %bx
    382c:	57                   	push   %di
    382d:	83 ec 16             	sub    $0x16,%sp
    3830:	7e c6                	jle    0x37f8
    3832:	73 2b                	jae    0x385f
    3834:	b0 bf                	mov    $0xbf,%al
    3836:	f8                   	clc
    3837:	33 c7                	xor    %di,%ax
    3839:	00 e3                	add    %ah,%bl
    383b:	88 06 47 2a          	mov    %al,0x2a47
    383f:	3b ec                	cmp    %sp,%bp
    3841:	43                   	inc    %bx
    3842:	d8 74 fc             	fdivs  -0x4(%si)
    3845:	9f                   	lahf
    3846:	02 11                	add    (%bx,%di),%dl
    3848:	ff 05                	incw   (%di)
    384a:	89 ee                	mov    %bp,%si
    384c:	87 11                	xchg   %dx,(%bx,%di)
    384e:	03 24                	add    (%si),%sp
    3850:	19 88 ff a1          	sbb    %cx,-0x5e01(%bx,%si)
    3854:	e8 f1 eb             	call   0x2448
    3857:	30 fb                	xor    %bh,%bl
    3859:	e4 2b                	in     $0x2b,%al
    385b:	ff c6                	inc    %si
    385d:	43                   	inc    %bx
    385e:	f0 ff e3             	lock jmp *%bx
    3861:	f6 05 0d             	testb  $0xd,(%di)
    3864:	fd                   	std
    3865:	0d eb 80             	or     $0x80eb,%ax
    3868:	50                   	push   %ax
    3869:	ff 81 ec 88          	incw   -0x7714(%bx,%di)
    386d:	80 1b 1b             	sbbb   $0x1b,(%bp,%di)
    3870:	fd                   	std
    3871:	17                   	pop    %ss
    3872:	73 17                	jae    0x388b
    3874:	36 c0 0c c1          	rorb   $0xc1,%ss:(%si)
    3878:	8a 22                	mov    (%bp,%si),%ah
    387a:	c8 b8 ba 63          	enter  $0xbab8,$0x63
    387e:	1a 72 c7             	sbb    -0x39(%bp,%si),%dh
    3881:	62 57 08             	bound  %dx,0x8(%bx)
    3884:	cb                   	lret
    3885:	2d e9 00             	sub    $0xe9,%ax
    3888:	34 d7                	xor    $0xd7,%al
    388a:	55                   	push   %bp
    388b:	16                   	push   %ss
    388c:	75 08                	jne    0x3896
    388e:	17                   	pop    %ss
    388f:	73 4a                	jae    0x38db
    3891:	2b be 31 14          	sub    0x1431(%bp),%di
    3895:	11 89 7e fa          	adc    %cx,-0x582(%bx,%di)
    3899:	81 fe 38 11          	cmp    $0x1138,%si
    389d:	73 3c                	jae    0x38db
    389f:	8a 04                	mov    (%si),%al
    38a1:	68 8a 1a             	push   $0x1a8a
    38a4:	75 0c                	jne    0x38b2
    38a6:	80 7c 02 76          	cmpb   $0x76,0x2(%si)
    38aa:	06                   	push   %es
    38ab:	7c 22                	jl     0x38cf
    38ad:	00 03                	add    %al,(%bp,%di)
    38af:	76 06                	jbe    0x38b7
    38b1:	7c 83                	jl     0x3836
    38b3:	11 1a                	adc    %bx,(%bp,%si)
    38b5:	72 eb                	jb     0x38a2
    38b7:	e1 88                	loope  0x3841
    38b9:	53                   	push   %bx
    38ba:	f0 ff c0             	lock inc %ax
    38bd:	0c c1                	or     $0xc1,%al
    38bf:	8a ff                	mov    %bh,%bh
    38c1:	22 12                	and    (%bp,%si),%dl
    38c3:	c8 ff b8 ff          	enter  $0xb8ff,$0xff
    38c7:	ba 8b 87             	mov    $0x878b,%dx
    38ca:	57                   	push   %di
    38cb:	08 2d                	or     %ch,(%di)
    38cd:	ff f2                	push   %dx
    38cf:	f0 8b 87 e9 00       	lock mov 0xe9(%bx),%ax
    38d4:	5e                   	pop    %si
    38d5:	8b 65 fb             	mov    -0x5(%di),%sp
    38d8:	87 8b 55 47          	xchg   %cx,0x4755(%bp,%di)
    38dc:	83 ff 08             	cmp    $0x8,%di
    38df:	73 76                	jae    0x3957
    38e1:	2b 78 ff             	sub    -0x1(%bx,%si),%di
    38e4:	e8 e7 ba             	call   0xf3ce
    38e7:	8b 76 f8             	mov    -0x8(%bp),%si
    38ea:	98                   	cbtw
    38eb:	89 06 76 2b          	mov    %ax,0x2b76
    38ef:	2f                   	das
    38f0:	01 e8                	add    %bp,%ax
    38f2:	e7 fc                	out    %ax,$0xfc
    38f4:	e8 a4 fc             	call   0x359b
    38f7:	0a b0 e9 0a          	or     0xae9(%bx,%si),%dh
    38fb:	80 7f 1b e9          	cmpb   $0xe9,0x1b(%bx)
    38ff:	9e                   	sahf
    3900:	75 e8                	jne    0x38ea
    3902:	e9 1b 01             	jmp    0x3a20
    3905:	b6 33                	mov    $0x33,%dh
    3907:	e4 0b                	in     $0xb,%al
    3909:	05 e5 a5             	add    $0xa5e5,%ax
    390c:	07                   	pop    %es
    390d:	30 57 33             	xor    %dl,0x33(%bx)
    3910:	88 80 4d 75          	mov    %al,0x754d(%bx,%si)
    3914:	00 80 75 1b          	add    %al,0x1b75(%bx,%si)
    3918:	57                   	push   %di
    3919:	06                   	push   %es
    391a:	98                   	cbtw
    391b:	2b ff                	sub    %di,%di
    391d:	04 16                	add    $0x16,%al
    391f:	ff e4                	jmp    *%sp
    3921:	f6 57 57             	notb   0x57(%bx)
    3924:	08 63 bf             	or     %ah,-0x41(%bp,%di)
    3927:	38 ea                	cmp    %ch,%dl
    3929:	83 ff 08             	cmp    $0x8,%di
    392c:	73 bc                	jae    0x38ea
    392e:	8a ea                	mov    %dl,%ch
    3930:	c3                   	ret
    3931:	38 e4                	cmp    %ah,%ah
    3933:	80 50 81 ec          	adcb   $0xec,-0x7f(%bx,%si)
    3937:	88 06 76 2b          	mov    %al,0x2b76
    393b:	df 00                	filds  (%bx,%si)
    393d:	8a 43 f0             	mov    -0x10(%bp,%di),%al
    3940:	88 fe                	mov    %bh,%dh
    3942:	fd                   	std
    3943:	0e                   	push   %cs
    3944:	f6 66 37             	mulb   0x37(%bp)
    3947:	46                   	inc    %si
    3948:	1e                   	push   %ds
    3949:	31 77 48             	xor    %si,0x48(%bx)
    394c:	ff e4                	jmp    *%sp
    394e:	f6 fb                	idiv   %bl
    3950:	87 91 01 33          	xchg   %dx,0x3301(%bx,%di)
    3954:	33 8c 27 b0          	xor    -0x4fd9(%si),%cx
    3958:	ff                   	(bad)
    3959:	bf 03 fe             	mov    $0xfe03,%di
    395c:	30 b9 88 24          	xor    %bh,0x2488(%bx,%di)
    3960:	04 87                	add    $0x87,%al
    3962:	11 17                	adc    %dx,(%bx)
    3964:	11 b8 fe e4          	adc    %di,-0x1b02(%bx,%si)
    3968:	06                   	push   %es
    3969:	bf 87 eb             	mov    $0xeb87,%di
    396c:	15 29 19             	adc    $0x1929,%ax
    396f:	c6 87 26 19 01       	movb   $0x1,0x1926(%bx)
    3974:	01 d8                	add    %bx,%ax
    3976:	14 19                	adc    $0x19,%al
    3978:	f6 01 d8             	testb  $0xd8,(%bx,%di)
    397b:	57                   	push   %di
    397c:	54                   	push   %sp
    397d:	7e 01                	jle    0x3980
    397f:	57                   	push   %di
    3980:	ff 72 d4             	push   -0x2c(%bp,%si)
    3983:	76 2b                	jbe    0x39b0
    3985:	95                   	xchg   %ax,%bp
    3986:	63 c6                	arpl   %ax,%si
    3988:	fe                   	(bad)
    3989:	bf 11 17             	mov    $0x1711,%di
    398c:	26 19 01             	sbb    %ax,%es:(%bx,%di)
    398f:	e8 8e 23             	call   0x5d20
    3992:	14 04                	adc    $0x4,%al
    3994:	f8                   	clc
    3995:	2a 20                	sub    (%bx,%si),%ah
    3997:	89 ff                	mov    %di,%di
    3999:	e8 f2 e8             	call   0x228e
    399c:	a1 11 33             	mov    0x3311,%ax
    399f:	33 8c 71 b1          	xor    -0x4e8f(%si),%cx
    39a3:	05 8a ff             	add    $0xff8a,%ax
    39a6:	22 12                	and    (%bp,%si),%dl
    39a8:	c8 d3 e3 80          	enter  $0xe3d3,$0x80
    39ac:	bf 85 52             	mov    $0x5285,%di
    39af:	00 75 30             	add    %dh,0x30(%di)
    39b2:	30 8a 33 07          	xor    %cl,0x733(%bp,%si)
    39b6:	09 84 75 83          	or     %ax,-0x7c8b(%si)
    39ba:	77 c5                	ja     0x3981
    39bc:	25 8b f0             	and    $0xf08b,%ax
    39bf:	20 97 83 22          	and    %dl,0x2283(%bx)
    39c3:	01 57 ff             	add    %dx,-0x1(%bx)
    39c6:	05 76 09             	add    $0x976,%ax
    39c9:	00 06 69 59          	add    %al,0x5969
    39cd:	07                   	pop    %es
    39ce:	60                   	pusha
    39cf:	39 eb                	cmp    %bp,%bx
    39d1:	13 01                	adc    (%bx,%di),%ax
    39d3:	57                   	push   %di
    39d4:	ff 02                	incw   (%bp,%si)
    39d6:	75 b8                	jne    0x3990
    39d8:	16                   	push   %ss
    39d9:	1e                   	push   %ds
    39da:	00 eb                	add    %ch,%bl
    39dc:	ef                   	out    %ax,(%dx)
    39dd:	b8 0f 00             	mov    $0xf,%ax
    39e0:	eb ea                	jmp    0x39cc
    39e2:	54                   	push   %sp
    39e3:	1d b8 cd             	sbb    $0xcdb8,%ax
    39e6:	7e 8b                	jle    0x3973
    39e8:	30 30                	xor    %dh,(%bx,%si)
    39ea:	8a 33                	mov    (%bp,%di),%dh
    39ec:	06                   	push   %es
    39ed:	bf 87 eb             	mov    $0xeb87,%di
    39f0:	15 24 88             	adc    $0x8824,%ax
    39f3:	a1 f1 fc             	mov    0xfcf1,%ax
    39f6:	16                   	push   %ss
    39f7:	08 73 2b             	or     %dh,0x2b(%bp,%di)
    39fa:	72 1d                	jb     0x3a19
    39fc:	06                   	push   %es
    39fd:	bf 87 b0             	mov    $0xb087,%di
    3a00:	ff                   	(bad)
    3a01:	bf a7 29             	mov    $0x29a7,%di
    3a04:	19 8b f0 b0          	sbb    %cx,-0x4f10(%bp,%di)
    3a08:	14 19                	adc    $0x19,%al
    3a0a:	84 88 fd 3c          	test   %cl,0x3cfd(%bx,%si)
    3a0e:	15 29 19             	adc    $0x1929,%ax
    3a11:	eb 06                	jmp    0x3a19
    3a13:	b8 ff 01             	mov    $0x1ff,%ax
    3a16:	c7                   	(bad)
    3a17:	f1                   	int1
    3a18:	fc                   	cld
    3a19:	54                   	push   %sp
    3a1a:	1d b8 47             	sbb    $0x47b8,%ax
    3a1d:	e9 09 80             	jmp    0xba29
    3a20:	b9 31 e6             	mov    $0xe631,%cx
    3a23:	e8 f1 23             	call   0x5e17
    3a26:	0f 07                	sysret
    3a28:	60                   	pusha
    3a29:	e3 5e                	jcxz   0x3a89
    3a2b:	e8 83 e4             	call   0x1eb1
    3a2e:	46                   	inc    %si
    3a2f:	03 d8                	add    %ax,%bx
    3a31:	56                   	push   %si
    3a32:	5d                   	pop    %bp
    3a33:	03 8a 1e 31          	add    0x311e(%bp,%si),%cx
    3a37:	08 76 89             	or     %dh,-0x77(%bp)
    3a3a:	89 8c e8 e8          	mov    %cx,-0x1718(%si)
    3a3e:	fc                   	cld
    3a3f:	fe 47 fa             	incb   -0x6(%bx)
    3a42:	2b fc                	sub    %sp,%di
    3a44:	fa                   	cli
    3a45:	54                   	push   %sp
    3a46:	f8                   	clc
    3a47:	b8 6e 89             	mov    $0x896e,%ax
    3a4a:	f8                   	clc
    3a4b:	81 0a f8 81          	orw    $0x81f8,(%bp,%si)
    3a4f:	0e                   	push   %cs
    3a50:	eb 0e                	jmp    0x3a60
    3a52:	16                   	push   %ss
    3a53:	8a 15                	mov    (%di),%dl
    3a55:	8b 05                	mov    (%di),%ax
    3a57:	8b c8                	mov    %ax,%cx
    3a59:	8a 84 de 8e          	mov    -0x7122(%si),%al
    3a5d:	2a 20                	sub    (%bx,%si),%ah
    3a5f:	f7 e1                	mul    %cx
    3a61:	01 47 fa             	add    %ax,-0x6(%bx)
    3a64:	11 56 fc             	adc    %dx,-0x4(%bp)
    3a67:	47                   	inc    %di
    3a68:	47                   	inc    %di
    3a69:	b8 8b 72             	mov    $0x728b,%ax
    3a6c:	03 e6                	add    %si,%sp
    3a6e:	fa                   	cli
    3a6f:	76 74                	jbe    0x3ae5
    3a71:	1a c7                	sbb    %bh,%al
    3a73:	75 07                	jne    0x3a7c
    3a75:	81 7e fa 10 27       	cmpw   $0x2710,-0x6(%bp)
    3a7a:	72 33                	jb     0x3aaf
    3a7c:	ff                   	(bad)
    3a7d:	fc                   	cld
    3a7e:	a3 10 27             	mov    %ax,0x2710
    3a81:	b8 fc 02             	mov    $0x2fc,%ax
    3a84:	ff 6e 08             	ljmp   *0x8(%bp)
    3a87:	d8 df                	fcomp  %st(7)
    3a89:	8b 87 19 05          	mov    0x519(%bx),%ax
    3a8d:	89 76 06             	mov    %si,0x6(%bp)
    3a90:	89 04                	mov    %ax,(%si)
    3a92:	8b 87 1e 05          	mov    0x51e(%bx),%ax
    3a96:	89 5e 04             	mov    %bx,0x4(%bp)
    3a99:	89 57 fc             	mov    %dx,-0x4(%bx)
    3a9c:	c6                   	(bad)
    3a9d:	fe 0a                	decb   (%bp,%si)
    3a9f:	04 f8                	add    $0xf8,%al
    3aa1:	0b 03                	or     (%bp,%di),%ax
    3aa3:	2b 3b                	sub    (%bp,%di),%di
    3aa5:	c3                   	ret
    3aa6:	72 13                	jb     0x3abb
    3aa8:	89 76 06             	mov    %si,0x6(%bp)
    3aab:	8b 14                	mov    (%si),%dx
    3aad:	8b 03                	mov    (%bp,%di),%ax
    3aaf:	ff                   	ljmp   (bad)
    3ab0:	e8 ff e8             	call   0x23b2
    3ab3:	2b d0                	sub    %ax,%dx
    3ab5:	89 04                	mov    %ax,(%si)
    3ab7:	2b db                	sub    %bx,%bx
    3ab9:	eb 0e                	jmp    0x3ac9
    3abb:	89 76 06             	mov    %si,0x6(%bp)
    3abe:	8b 04                	mov    (%si),%ax
    3ac0:	f8                   	clc
    3ac1:	0b 03                	or     (%bp,%di),%ax
    3ac3:	2b 2b                	sub    (%bp,%di),%bp
    3ac5:	df 8b 56 f8          	fisttps -0x7aa(%bp,%di)
    3ac9:	89 76 04             	mov    %si,0x4(%bp)
    3acc:	39 1c                	cmp    %bx,(%si)
    3ace:	72 0a                	jb     0x3ada
    3ad0:	f8                   	clc
    3ad1:	c3                   	ret
    3ad2:	2b 1c                	sub    (%si),%bx
    3ad4:	f7 db                	neg    %bx
    3ad6:	03 d3                	add    %bx,%dx
    3ad8:	89 04                	mov    %ax,(%si)
    3ada:	8b c2                	mov    %dx,%ax
    3adc:	8c 14                	mov    %ss,(%si)
    3ade:	ff 04                	incw   (%si)
    3ae0:	3e 57                	ds push %di
    3ae2:	ec                   	in     (%dx),%al
    3ae3:	7e 73                	jle    0x3b58
    3ae5:	b0 f8                	mov    $0xf8,%al
    3ae7:	c7                   	(bad)
    3ae8:	e3 0a                	jcxz   0x3af4
    3aea:	fa                   	cli
    3aeb:	80 32 8c             	xorb   $0x8c,(%bp,%si)
    3aee:	b9 f6 03             	mov    $0x3f6,%cx
    3af1:	b9 31 08             	mov    $0x831,%cx
    3af4:	2a 89 e8 01          	sub    0x1e8(%bx,%di),%cl
    3af8:	06                   	push   %es
    3af9:	3b 89 f6 fa          	cmp    -0x50a(%bx,%di),%cx
    3afd:	e3 e8                	jcxz   0x3ae7
    3aff:	50                   	push   %ax
    3b00:	fa                   	cli
    3b01:	c7                   	(bad)
    3b02:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    3b03:	eb e8                	jmp    0x3aed
    3b05:	b8 e8 29             	mov    $0x29e8,%ax
    3b08:	ff                   	ljmp   (bad)
    3b09:	eb e8                	jmp    0x3af3
    3b0b:	75 33                	jne    0x3b40
    3b0d:	8c b1 29 0e          	mov    %?,0xe29(%bx,%di)
    3b11:	16                   	push   %ss
    3b12:	8a 0e ff 8b          	mov    -0x7401,%cl
    3b16:	75 00                	jne    0x3b18
    3b18:	b8 9f 2b             	mov    $0x2b9f,%ax
    3b1b:	4b                   	dec    %bx
    3b1c:	be 64 06             	mov    $0x664,%si
    3b1f:	79 20                	jns    0x3b41
    3b21:	ff 28                	ljmp   *(%bx,%si)
    3b23:	1f                   	pop    %ds
    3b24:	8a 8a 5b 7d          	mov    0x7d5b(%bp,%si),%cl
    3b28:	5b                   	pop    %bx
    3b29:	b8 c6 62             	mov    $0x62c6,%ax
    3b2c:	5b                   	pop    %bx
    3b2d:	b8 9c b8             	mov    $0xb89c,%ax
    3b30:	8a 06 e8 eb          	mov    -0x1418,%al
    3b34:	20 33                	and    %dh,(%bp,%di)
    3b36:	8c b1 6e b8          	mov    %?,-0x4792(%bx,%di)
    3b3a:	fc                   	cld
    3b3b:	02 ff                	add    %bh,%bh
    3b3d:	6e                   	outsb  %ds:(%si),(%dx)
    3b3e:	f6 02 04             	testb  $0x4,(%bp,%si)
    3b41:	06                   	push   %es
    3b42:	87 fa                	xchg   %di,%dx
    3b44:	04 20                	add    $0x20,%al
    3b46:	0b ec                	or     %sp,%bp
    3b48:	8f                   	(bad)
    3b49:	eb e8                	jmp    0x3b33
    3b4b:	04 20                	add    $0x20,%al
    3b4d:	1e                   	push   %ds
    3b4e:	05 2b f6             	add    $0xf62b,%ax
    3b51:	b0 bf                	mov    $0xbf,%al
    3b53:	fe                   	(bad)
    3b54:	b9 08 d8             	mov    $0xd808,%cx
    3b57:	81 eb 16 15          	sub    $0x1516,%bx
    3b5b:	05 c8 08             	add    $0x8c8,%ax
    3b5e:	d8 05                	fadds  (%di)
    3b60:	47                   	inc    %di
    3b61:	47                   	inc    %di
    3b62:	01 b8 7e b8          	add    %di,-0x4782(%bx,%si)
    3b66:	8b 72 03             	mov    0x3(%bp,%si),%si
    3b69:	7c f3                	jl     0x3b5e
    3b6b:	06                   	push   %es
    3b6c:	87 fa                	xchg   %di,%dx
    3b6e:	f8                   	clc
    3b6f:	0b 03                	or     (%bp,%di),%ax
    3b71:	2b 89 eb e8          	sub    -0x1715(%bx,%di),%cx
    3b75:	2b 89 f8 0a          	sub    0xaf8(%bx,%di),%cx
    3b79:	fe 8b c8 b0          	decb   -0x4f38(%bp,%di)
    3b7d:	fe 08                	decb   (%bx,%si)
    3b7f:	81 16 05 d8 88 8f    	adcw   $0x8f88,-0x27fb
    3b85:	18 11                	sbb    %dl,(%bx,%di)
    3b87:	b1 8a                	mov    $0x8a,%cl
    3b89:	22 c8                	and    %al,%cl
    3b8b:	16                   	push   %ss
    3b8c:	eb 9a                	jmp    0x3b28
    3b8e:	05 0d b8             	add    $0xb80d,%ax
    3b91:	74 b8                	je     0x3b4b
    3b93:	52                   	push   %dx
    3b94:	0a 5b fc             	or     -0x4(%bp,%di),%bl
    3b97:	b8 74 76             	mov    $0x7674,%ax
    3b9a:	52                   	push   %dx
    3b9b:	8b e8                	mov    %ax,%bp
    3b9d:	5b                   	pop    %bx
    3b9e:	5a                   	pop    %dx
    3b9f:	c9                   	leave
    3ba0:	89 84 b8 52          	mov    %ax,0x52b8(%si)
    3ba4:	c2 d2 1a             	ret    $0x1ad2
    3ba7:	89 ff                	mov    %di,%di
    3ba9:	e8 b1 76             	call   0xb25d
    3bac:	30 00                	xor    %al,(%bx,%si)
    3bae:	ba 01 1a             	mov    $0x1a01,%dx
    3bb1:	24 f6                	and    $0xf6,%al
    3bb3:	02 fa                	add    %dl,%bh
    3bb5:	89 2b                	mov    %bp,(%bp,%di)
    3bb7:	8e 07                	mov    (%bx),%es
    3bb9:	0c 2b                	or     $0x2b,%al
    3bbb:	8e 07                	mov    (%bx),%es
    3bbd:	09 06 3b d8          	or     %ax,-0x27c5
    3bc1:	81 1d fd 38          	sbbw   $0x38fd,(%di)
    3bc5:	e8 0a 8b             	call   0xc6d2
    3bc8:	b0 1a                	mov    $0x1a,%al
    3bca:	e6 89                	out    %al,$0x89
    3bcc:	03 da                	add    %dx,%bx
    3bce:	e6 89                	out    %al,$0x89
    3bd0:	03 ee                	add    %si,%bp
    3bd2:	ff                   	(bad)
    3bd3:	ff                   	(bad)
    3bd4:	b9 a2 1b             	mov    $0x1ba2,%cx
    3bd7:	51                   	push   %cx
    3bd8:	b8 f0 89             	mov    $0x89f0,%ax
    3bdb:	01 0f                	add    %cx,(%bx)
    3bdd:	bf 26 a6             	mov    $0xa626,%di
    3be0:	16                   	push   %ss
    3be1:	fe                   	(bad)
    3be2:	fd                   	std
    3be3:	70 e1                	jo     0x3bc6
    3be5:	46                   	inc    %si
    3be6:	ff 77 b0             	push   -0x50(%bx)
    3be9:	ec                   	in     (%dx),%al
    3bea:	9e                   	sahf
    3beb:	8b 14                	mov    (%si),%dx
    3bed:	f6 26 a2 8b          	mulb   -0x745e
    3bf1:	05 16 31             	add    $0x3116,%ax
    3bf4:	75 26                	jne    0x3c1c
    3bf6:	19 00                	sbb    %ax,(%bx,%si)
    3bf8:	16                   	push   %ss
    3bf9:	1e                   	push   %ds
    3bfa:	31 3e 06 3b          	xor    %di,0x3b06
    3bfe:	80 3e af a6 06       	cmpb   $0x6,-0x5951
    3c03:	2b 13                	sub    (%bp,%di),%dx
    3c05:	2b 5e 1f             	sub    0x1f(%bp),%bx
    3c08:	4b                   	dec    %bx
    3c09:	1a 31                	sbb    (%bx,%di),%dh
    3c0b:	e2 46                	loop   0x3c53
    3c0d:	ff a0 e6 89          	jmp    *-0x761a(%bx,%si)
    3c11:	01 54 9c             	add    %dx,-0x64(%si)
    3c14:	04 5f                	add    $0x5f,%al
    3c16:	80 3e af a6 06       	cmpb   $0x6,-0x5951
    3c1b:	b0 1a                	mov    $0x1a,%al
    3c1d:	39 ff                	cmp    %di,%di
    3c1f:	b9 74 53             	mov    $0x5374,%cx
    3c22:	b8 1b 09             	mov    $0x91b,%ax
    3c25:	28 29                	sub    %ch,(%bx,%di)
    3c27:	c1 19 eb             	rcrw   $0xeb,(%bx,%di)
    3c2a:	3b b8 1b 56          	cmp    0x561b(%bx,%si),%di
    3c2e:	da 8b 81 95          	fimull -0x6a7f(%bp,%di)
    3c32:	18 8b 81 ff          	sbb    %cl,-0x7f(%bp,%di)
    3c36:	06                   	push   %es
    3c37:	c6                   	(bad)
    3c38:	89 eb                	mov    %bp,%bx
    3c3a:	b0 ec                	mov    $0xec,%al
    3c3c:	9e                   	sahf
    3c3d:	eb 00                	jmp    0x3c3f
    3c3f:	1e                   	push   %ds
    3c40:	25 8b 14             	and    $0x148b,%ax
    3c43:	f6 a4 a2 05          	mulb   0x5a2(%si)
    3c47:	31 26 00 1e          	xor    %sp,0x1e00
    3c4b:	3e 3b 3e a6 0f       	cmp    %ds:0xfa6,%di
    3c50:	52                   	push   %dx
    3c51:	1c 04                	sbb    $0x4,%al
    3c53:	8b 75 b8             	mov    -0x48(%di),%si
    3c56:	15 0e 1d             	adc    $0x1d0e,%ax
    3c59:	31 77 48             	xor    %si,0x48(%bx)
    3c5c:	1d eb 07             	sbb    $0x7eb,%ax
    3c5f:	8a 84 a2 05          	mov    0x5a2(%si),%al
    3c63:	88 fe                	mov    %bh,%dh
    3c65:	00 5d 1d             	add    %bl,0x1d(%di)
    3c68:	03 0c                	add    (%si),%cx
    3c6a:	74 bf                	je     0x3c2b
    3c6c:	b8 1b 09             	mov    $0x91b,%ax
    3c6f:	28 29                	sub    %ch,(%bx,%di)
    3c71:	8c 19                	mov    %ds,(%bx,%di)
    3c73:	eb 2e                	jmp    0x3ca3
    3c75:	a0 a2 05             	mov    0x5a2,%al
    3c78:	88 fe                	mov    %bh,%dh
    3c7a:	00 b1 14 f6          	add    %dh,-0x9ec(%bx,%di)
    3c7e:	11 ff                	adc    %di,%di
    3c80:	05 16 31             	add    $0x3116,%ax
    3c83:	3e 3b 3e a6 1b       	cmp    %ds:0x1ba6,%di
    3c88:	39 ff                	cmp    %di,%di
    3c8a:	b9 75 3e             	mov    $0x3e75,%cx
    3c8d:	89 01                	mov    %ax,(%bx,%di)
    3c8f:	04 6a                	add    $0x6a,%al
    3c91:	4b                   	dec    %bx
    3c92:	b8 13 32             	mov    $0x3213,%ax
    3c95:	1d eb 03             	sbb    $0x3eb,%ax
    3c98:	15 0e 1d             	adc    $0x1d0e,%ax
    3c9b:	31 77 35             	xor    %si,0x35(%bx)
    3c9e:	1d 03 e6             	sbb    $0xe603,%ax
    3ca1:	89 e8                	mov    %bp,%ax
    3ca3:	f0 e4 e0             	lock in $0xe0,%al
    3ca6:	8e fe                	mov    %si,%?
    3ca8:	8b 0c                	mov    (%si),%cx
    3caa:	fc                   	cld
    3cab:	be 3e 74             	mov    $0x743e,%si
    3cae:	a3 06 00             	mov    %ax,0x6
    3cb1:	88 ff                	mov    %bh,%bh
    3cb3:	d6                   	(bad)
    3cb4:	04 0d                	add    $0xd,%al
    3cb6:	89 15                	mov    %dx,(%di)
    3cb8:	0b ef                	or     %di,%bp
    3cba:	d8 40 97             	fadds  -0x69(%bx,%si)
    3cbd:	e0 8e                	loopne 0x3c4d
    3cbf:	fe 06 d8 1d          	incb   0x1dd8
    3cc3:	7f 02                	jg     0x3cc7
    3cc5:	4d                   	dec    %bp
    3cc6:	be fd 8b             	mov    $0x8bfd,%si
    3cc9:	03 ba 52 90          	add    -0x6fae(%bp,%si),%di
    3ccd:	dd 00                	fldl   (%bx,%si)
    3ccf:	0c 1b                	or     $0x1b,%al
    3cd1:	74 8a                	je     0x3c5d
    3cd3:	00 5c 04             	add    %bl,0x4(%si)
    3cd6:	88 9a 00 5c          	mov    %bl,0x5c00(%bp,%si)
    3cda:	04 88                	add    $0x88,%al
    3cdc:	fe 05                	incb   (%di)
    3cde:	52                   	push   %dx
    3cdf:	1c 04                	sbb    $0x4,%al
    3ce1:	8b 06 e6 0b          	mov    0xbe6,%ax
    3ce5:	46                   	inc    %si
    3ce6:	16                   	push   %ss
    3ce7:	75 08                	jne    0x3cf1
    3ce9:	b0 75                	mov    $0x75,%al
    3ceb:	16                   	push   %ss
    3cec:	e4 1b                	in     $0x1b,%al
    3cee:	2a 40 4e             	sub    0x4e(%bx,%si),%al
    3cf1:	2a f7                	sub    %bh,%dh
    3cf3:	08 05                	or     %al,(%di)
    3cf5:	00 84 a3 1b          	add    %al,0x1ba3(%si)
    3cf9:	fa                   	cli
    3cfa:	52                   	push   %dx
    3cfb:	db bb 01 50          	fstpt  0x5001(%bp,%di)
    3cff:	f9                   	stc
    3d00:	ef                   	out    %ax,(%dx)
    3d01:	b8 09 00             	mov    $0x9,%ax
    3d04:	04 06                	add    $0x6,%al
    3d06:	27                   	daa
    3d07:	5e                   	pop    %si
    3d08:	b8 66 14             	mov    $0x1466,%ax
    3d0b:	ca 50 2d             	lret   $0x2d50
    3d0e:	e8 5b f7             	call   0x346c
    3d11:	ff                   	(bad)
    3d12:	ff                   	(bad)
    3d13:	fd                   	std
    3d14:	68 8a 1a             	push   $0x1a8a
    3d17:	b8 00 8a             	mov    $0x8a00,%ax
    3d1a:	9e                   	sahf
    3d1b:	2d e8 ff             	sub    $0xffe8,%ax
    3d1e:	fd                   	std
    3d1f:	2a 03                	sub    (%bp,%di),%al
    3d21:	fe                   	(bad)
    3d22:	e2 de                	loop   0x3d02
    3d24:	69 bf f6 fb f1 2b    	imul   $0x2bf1,-0x40a(%bx),%di
    3d2a:	b8 09 00             	mov    $0x9,%ax
    3d2d:	04 06                	add    $0x6,%al
    3d2f:	27                   	daa
    3d30:	5e                   	pop    %si
    3d31:	b8 66 df             	mov    $0xdf66,%ax
    3d34:	18 19                	sbb    %bl,(%bx,%di)
    3d36:	8b 8a 59 8b          	mov    -0x74a7(%bp,%si),%cx
    3d3a:	8a 0e fa 1e          	mov    0x1efa,%cl
    3d3e:	04 06                	add    $0x6,%al
    3d40:	73 04                	jae    0x3d46
    3d42:	56                   	push   %si
    3d43:	53                   	push   %bx
    3d44:	03 55 4c             	add    0x4c(%di),%dx
    3d47:	03 c6                	add    %si,%ax
    3d49:	8a b8 38 ff          	mov    -0xc8(%bx,%si),%bh
    3d4d:	73 8a                	jae    0x3cd9
    3d4f:	87 c6                	xchg   %ax,%si
    3d51:	87 8b 56 1e          	xchg   %cx,0x1e56(%bp,%di)
    3d55:	8a 87 c6 87          	mov    -0x783a(%bx),%al
    3d59:	8b 56 31             	mov    0x31(%bp),%dx
    3d5c:	a2 05 56             	mov    %al,0x5605
    3d5f:	53                   	push   %bx
    3d60:	c2 1a ff             	ret    $0xff1a
    3d63:	b8 66 50             	mov    $0x5066,%ax
    3d66:	ff                   	(bad)
    3d67:	fd                   	std
    3d68:	2a 03                	sub    (%bp,%di),%al
    3d6a:	fe                   	(bad)
    3d6b:	e2 de                	loop   0x3d4b
    3d6d:	69 bf f6 fb 66 da    	imul   $0xda66,-0x40a(%bx),%di
    3d73:	0b 46 16             	or     0x16(%bp),%ax
    3d76:	75 08                	jne    0x3d80
    3d78:	2d a1 63             	sub    $0x63a1,%ax
    3d7b:	9a b8 89 b9 f9       	lcall  $0xf9b9,$0x89b8
    3d80:	ff                   	(bad)
    3d81:	ff                   	(bad)
    3d82:	b8 00 5d             	mov    $0x5d00,%ax
    3d85:	e7 8e                	out    %ax,$0x8e
    3d87:	b9 38 01             	mov    $0x138,%cx
    3d8a:	51                   	push   %cx
    3d8b:	b9 0f 22             	mov    $0x220f,%cx
    3d8e:	50                   	push   %ax
    3d8f:	21 e2                	and    %sp,%dx
    3d91:	40                   	inc    %ax
    3d92:	19 0f                	sbb    %cx,(%bx)
    3d94:	00 e8                	add    %ch,%al
    3d96:	b6 a0                	mov    $0xa0,%dh
    3d98:	d1 05                	rolw   $1,(%di)
    3d9a:	2d 5f 19             	sub    $0x195f,%ax
    3d9d:	90                   	nop
    3d9e:	ec                   	in     (%dx),%al
    3d9f:	e5 ee                	in     $0xee,%ax
    3da1:	89 ff                	mov    %di,%di
    3da3:	08 3e 4e 19          	or     %bh,0x194e
    3da7:	90                   	nop
    3da8:	ec                   	in     (%dx),%al
    3da9:	e5 ee                	in     $0xee,%ax
    3dab:	89 ff                	mov    %di,%di
    3dad:	db df                	fcmovnu %st(7),%st
    3daf:	5e                   	pop    %si
    3db0:	e8 2b 7e             	call   0xbbde
    3db3:	57                   	push   %di
    3db4:	46                   	inc    %si
    3db5:	fe c7                	inc    %bh
    3db7:	9b                   	fwait
    3db8:	66 8a 5e fe          	data32 mov -0x2(%bp),%bl
    3dbc:	8b 08                	mov    (%bx,%si),%cx
    3dbe:	08 89 06 04          	or     %cl,0x406(%bx,%di)
    3dc2:	19 8a 8b 0e          	sbb    %cx,0xe8b(%bp,%si)
    3dc6:	1e                   	push   %ds
    3dc7:	06                   	push   %es
    3dc8:	b0 fe                	mov    $0xfe,%al
    3dca:	bf 5e 16             	mov    $0x165e,%di
    3dcd:	d8 8f 89 5e          	fmuls  0x5e89(%bx)
    3dd1:	fe 8a 22 2a          	decb   0x2a22(%bp,%si)
    3dd5:	20 89 c7 98          	and    %cl,-0x6739(%bx,%di)
    3dd9:	f4                   	hlt
    3dda:	5c                   	pop    %sp
    3ddb:	29 50 0b             	sub    %dx,0xb(%bx,%si)
    3dde:	46                   	inc    %si
    3ddf:	16                   	push   %ss
    3de0:	01 e5                	add    %sp,%bp
    3de2:	ee                   	out    %al,(%dx)
    3de3:	83 e7 f6             	and    $0xfff6,%di
    3de6:	83 c7 23             	add    $0x23,%di
    3de9:	56                   	push   %si
    3dea:	e8 5f 4c             	call   0x8a4c
    3ded:	4e                   	dec    %si
    3dee:	f7 fc                	idiv   %sp
    3df0:	4a                   	dec    %dx
    3df1:	05 18 19             	add    $0x1918,%ax
    3df4:	50                   	push   %ax
    3df5:	47                   	inc    %di
    3df6:	47                   	inc    %di
    3df7:	56                   	push   %si
    3df8:	e8 5f a0             	call   0xde5a
    3dfb:	4b                   	dec    %bx
    3dfc:	8b 5e fe             	mov    -0x2(%bp),%bx
    3dff:	8a 9f 25 19          	mov    0x1925(%bx),%bl
    3e03:	02 74 fc             	add    -0x4(%si),%dh
    3e06:	52                   	push   %dx
    3e07:	c7                   	(bad)
    3e08:	5b                   	pop    %bx
    3e09:	64 1c b8             	fs sbb $0xb8,%al
    3e0c:	4d                   	dec    %bp
    3e0d:	04 56                	add    $0x56,%al
    3e0f:	53                   	push   %bx
    3e10:	03 89 4b 8b          	add    -0x74b5(%bx,%di),%cx
    3e14:	5e                   	pop    %si
    3e15:	fe 8b 1e 89          	decb   -0x76e2(%bp,%di)
    3e19:	04 89                	add    $0x89,%al
    3e1b:	a2 0b 5e             	mov    %al,0x5e0b
    3e1e:	04 90                	add    $0x90,%al
    3e20:	04 56                	add    $0x56,%al
    3e22:	53                   	push   %bx
    3e23:	03 a2 4c 8b          	add    -0x74b4(%bp,%si),%sp
    3e27:	5e                   	pop    %si
    3e28:	fe 8b 1e 20          	decb   0x201e(%bp,%di)
    3e2c:	04 a2                	add    $0xa2,%al
    3e2e:	5e                   	pop    %si
    3e2f:	90                   	nop
    3e30:	56                   	push   %si
    3e31:	03 bc 04 56          	add    0x5604(%si),%di
    3e35:	53                   	push   %bx
    3e36:	03 8f 4c 52          	add    0x524c(%bx),%cx
    3e3a:	04 75                	add    $0x75,%al
    3e3c:	15 1d 26             	adc    $0x261d,%ax
    3e3f:	8b 5e fe             	mov    -0x2(%bp),%bx
    3e42:	80 bf 29 19 ff       	cmpb   $0xff,0x1929(%bx)
    3e47:	75 0e                	jne    0x3e57
    3e49:	8a 24                	mov    (%si),%ah
    3e4b:	2a b8 b8 22          	sub    0x22b8(%bx,%si),%bh
    3e4f:	5b                   	pop    %bx
    3e50:	ce                   	into
    3e51:	8a 7d 8a             	mov    -0x76(%di),%bh
    3e54:	36 eb 25             	ss jmp 0x3e7c
    3e57:	8a 24                	mov    (%si),%ah
    3e59:	29 19                	sub    %bx,(%bx,%di)
    3e5b:	b8 5b 8a             	mov    $0x8a5b,%ax
    3e5e:	8a eb                	mov    %bl,%ch
    3e60:	8a 00                	mov    (%bx,%si),%al
    3e62:	11 eb                	adc    %bp,%bx
    3e64:	17                   	pop    %ss
    3e65:	8b 5e fe             	mov    -0x2(%bp),%bx
    3e68:	8a 87 23 19          	mov    0x1923(%bx),%al
    3e6c:	b1 20                	mov    $0x20,%cl
    3e6e:	89 c7                	mov    %ax,%di
    3e70:	f6 f1                	div    %cl
    3e72:	8a c8                	mov    %al,%cl
    3e74:	ba 1a c7             	mov    $0xc71a,%dx
    3e77:	57                   	push   %di
    3e78:	eb 8a                	jmp    0x3e04
    3e7a:	86 74 b8             	xchg   %dh,-0x48(%si)
    3e7d:	ec                   	in     (%dx),%al
    3e7e:	04 56                	add    $0x56,%al
    3e80:	53                   	push   %bx
    3e81:	03 60 4d             	add    0x4d(%bx,%si),%sp
    3e84:	8b 5e fe             	mov    -0x2(%bp),%bx
    3e87:	8a 9f 26 19          	mov    0x1926(%bx),%bl
    3e8b:	5b                   	pop    %bx
    3e8c:	8a 8a 11 17          	mov    0x1711(%bp,%si),%cl
    3e90:	5e                   	pop    %si
    3e91:	54                   	push   %sp
    3e92:	b8 01 24             	mov    $0x2401,%ax
    3e95:	01 56 53             	add    %dx,0x53(%bp)
    3e98:	03 49 4d             	add    0x4d(%bx,%di),%cx
    3e9b:	89 e8                	mov    %bp,%ax
    3e9d:	76 00                	jbe    0x3e9f
    3e9f:	01 24                	add    %sp,(%si)
    3ea1:	02 89 8e 0c          	add    0xc8e(%bx,%di),%cl
    3ea5:	8e a0 02 e4          	mov    -0x1bfe(%bx,%si),%fs
    3ea9:	5e                   	pop    %si
    3eaa:	d1 8a 5d 2a          	rorw   $1,0x2a5d(%bp,%si)
    3eae:	03 a3 fa e8          	add    -0x1706(%bp,%di),%sp
    3eb2:	01 ed                	add    %bp,%bp
    3eb4:	04 27                	add    $0x27,%al
    3eb6:	51                   	push   %cx
    3eb7:	ba 01 ff             	mov    $0xff01,%dx
    3eba:	1b ec                	sbb    %sp,%bp
    3ebc:	39 04                	cmp    %ax,(%si)
    3ebe:	27                   	daa
    3ebf:	94                   	xchg   %ax,%sp
    3ec0:	de 00                	fiadds (%bx,%si)
    3ec2:	06                   	push   %es
    3ec3:	2b b0 3d 52          	sub    0x523d(%bx,%si),%si
    3ec7:	36 fa                	ss cli
    3ec9:	91                   	xchg   %ax,%cx
    3eca:	00 06 2b 95          	add    %al,-0x6ad5
    3ece:	3d 71 74             	cmp    $0x7471,%ax
    3ed1:	48                   	dec    %ax
    3ed2:	2a 00                	sub    (%bx,%si),%al
    3ed4:	06                   	push   %es
    3ed5:	2b 95 3d 71          	sub    0x713d(%di),%dx
    3ed9:	74 fa                	je     0x3ed5
    3edb:	46                   	inc    %si
    3edc:	63 94 06 fc          	arpl   %dx,-0x3fa(%si)
    3ee0:	8b b8 56 03          	mov    0x356(%bx,%si),%di
    3ee4:	8a 00                	mov    (%bx,%si),%al
    3ee6:	06                   	push   %es
    3ee7:	4b                   	dec    %bx
    3ee8:	f2 19 8a 8b 0e       	repnz sbb %cx,0xe8b(%bp,%si)
    3eed:	1e                   	push   %ds
    3eee:	06                   	push   %es
    3eef:	04 53                	add    $0x53,%al
    3ef1:	55                   	push   %bp
    3ef2:	03 8a 38 73          	add    0x7338(%bp,%si),%cx
    3ef6:	87 e5                	xchg   %sp,%bp
    3ef8:	89 08                	mov    %cx,(%bx,%si)
    3efa:	4e                   	dec    %si
    3efb:	90                   	nop
    3efc:	9e                   	sahf
    3efd:	4a                   	dec    %dx
    3efe:	8e 80 a0 02          	mov    0x2a0(%bx,%si),%es
    3f02:	e4 5d                	in     $0x5d,%al
    3f04:	03 fa                	add    %dx,%di
    3f06:	01 44 5d             	add    %ax,0x5d(%si)
    3f09:	c9                   	leave
    3f0a:	e8 ed 46             	call   0x85fa
    3f0d:	ff 05                	incw   (%di)
    3f0f:	a1 95 42             	mov    0x4295,%ax
    3f12:	a1 c2 ff             	mov    0xffc2,%ax
    3f15:	66 ff 2a             	ljmpl  *(%bp,%si)
    3f18:	9d                   	popf
    3f19:	fe                   	(bad)
    3f1a:	e8 6d de             	call   0x1d8a
    3f1d:	83 46 d8 5d          	addw   $0x5d,-0x28(%bp)
    3f21:	8a 31                	mov    (%bx,%di),%dh
    3f23:	0c 74                	or     $0x74,%al
    3f25:	00 04                	add    %al,(%si)
    3f27:	9a 5c 13 56 03       	lcall  $0x356,$0x135c
    3f2c:	4c                   	dec    %sp
    3f2d:	b8 56 03             	mov    $0x356,%ax
    3f30:	4e                   	dec    %si
    3f31:	b8 56 03             	mov    $0x356,%ax
    3f34:	50                   	push   %ax
    3f35:	b8 56 03             	mov    $0x356,%ax
    3f38:	3d 71 74             	cmp    $0x7471,%ax
    3f3b:	36 2a 19             	sub    %ss:(%bx,%di),%bl
    3f3e:	ec                   	in     (%dx),%al
    3f3f:	ee                   	out    %al,(%dx)
    3f40:	ff 1e 06 0a          	lcall  *0xa06
    3f44:	a1 e8 8b             	mov    0x8be8,%ax
    3f47:	08 08                	or     %cl,(%bx,%si)
    3f49:	0a d8                	or     %al,%bl
    3f4b:	c0 00 a2             	rolb   $0xa2,(%bx,%si)
    3f4e:	af                   	scas   %es:(%di),%ax
    3f4f:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    3f50:	a2 ae 8b             	mov    %al,0x8bae
    3f53:	4d                   	dec    %bp
    3f54:	2b be 31 29          	sub    0x2931(%bp),%di
    3f58:	fd                   	std
    3f59:	25 19 eb             	and    $0xeb19,%ax
    3f5c:	2a 80 29 20          	sub    0x2029(%bx,%si),%al
    3f60:	06                   	push   %es
    3f61:	21 fe                	and    %di,%si
    3f63:	06                   	push   %es
    3f64:	ae                   	scas   %es:(%di),%al
    3f65:	8b 8e 07 44          	mov    0x4407(%bp),%cx
    3f69:	75 8a                	jne    0x3ef5
    3f6b:	ff 75 15             	push   0x15(%di)
    3f6e:	8a 87 c6 97          	mov    -0x683a(%bx),%al
    3f72:	8b ea                	mov    %dx,%bp
    3f74:	38 a2 8b ea          	cmp    %ah,-0x1575(%bp,%si)
    3f78:	38 75 fe             	cmp    %dh,-0x2(%di)
    3f7b:	7c 01                	jl     0x3f7e
    3f7d:	75 fe                	jne    0x3f7d
    3f7f:	00 83 af a6          	add    %al,-0x5951(%bp,%di)
    3f83:	83 c6 14             	add    $0x14,%si
    3f86:	42                   	inc    %dx
    3f87:	3a f8                	cmp    %al,%bh
    3f89:	15 1a 72             	adc    $0x721a,%ax
    3f8c:	d0 01                	rolb   $1,(%bx,%di)
    3f8e:	8c 83 46 fa          	mov    %es,-0x5ba(%bp,%di)
    3f92:	ff e3                	jmp    *%bx
    3f94:	05 76 a1             	add    $0xa176,%ax
    3f97:	e8 95 01             	call   0x412f
    3f9a:	06                   	push   %es
    3f9b:	3b 52 89             	cmp    -0x77(%bp,%si),%dx
    3f9e:	8e 0c                	mov    (%si),%cs
    3fa0:	b0 06                	mov    $0x6,%al
    3fa2:	fe                   	(bad)
    3fa3:	5e                   	pop    %si
    3fa4:	d8 89 04 c8          	fmuls  -0x37fc(%bx,%di)
    3fa8:	ee                   	out    %al,(%dx)
    3fa9:	e8 18 50             	call   0x8fc4
    3fac:	47                   	inc    %di
    3fad:	8d 0c                	lea    (%si),%cx
    3faf:	b0 46                	mov    $0x46,%al
    3fb1:	f1                   	int1
    3fb2:	ec                   	in     (%dx),%al
    3fb3:	b8 7c 5e             	mov    $0x5e7c,%ax
    3fb6:	5b                   	pop    %bx
    3fb7:	5b                   	pop    %bx
    3fb8:	81 c6 25 19          	add    $0x1925,%si
    3fbc:	89 76 ae             	mov    %si,-0x52(%bp)
    3fbf:	8a 47 3b             	mov    0x3b(%bx),%al
    3fc2:	43                   	inc    %bx
    3fc3:	33 b1 b8 0c          	xor    0xcb8(%bx,%di),%si
    3fc7:	48                   	dec    %ax
    3fc8:	74 45                	je     0x400f
    3fca:	48                   	dec    %ax
    3fcb:	ba 90 00             	mov    $0x90,%dx
    3fce:	80 00 e9             	addb   $0xe9,(%bx,%si)
    3fd1:	84 ca                	test   %cl,%dl
    3fd3:	a2 2f 71             	mov    %al,0x712f
    3fd6:	59                   	pop    %cx
    3fd7:	02 73 b8             	add    -0x48(%bp,%di),%dh
    3fda:	16                   	push   %ss
    3fdb:	9e                   	sahf
    3fdc:	1d eb 74             	sbb    $0x74eb,%ax
    3fdf:	a0 c9 1e             	mov    0x1ec9,%al
    3fe2:	fa                   	cli
    3fe3:	01 44 5d             	add    %ax,0x5d(%si)
    3fe6:	c8 1e fa ed          	enter  $0xfa1e,$0xed
    3fea:	2b c1                	sub    %cx,%ax
    3fec:	76 8a                	jbe    0x3f78
    3fee:	eb fa                	jmp    0x3fea
    3ff0:	77 18                	ja     0x400a
    3ff2:	8a c1                	mov    %cl,%al
    3ff4:	fa                   	cli
    3ff5:	01 44 5d             	add    %ax,0x5d(%si)
    3ff8:	c9                   	leave
    3ff9:	1e                   	push   %ds
    3ffa:	2b c1                	sub    %cx,%ax
    3ffc:	01 b1 79 6c          	add    %si,0x6c79(%bx,%di)
    4000:	8b de                	mov    %si,%bx
    4002:	c6 07 01             	movb   $0x1,(%bx)
    4005:	b8 4c 1e             	mov    $0x1e4c,%ax
    4008:	eb 3a                	jmp    0x4044
    400a:	b8 6a 1d             	mov    $0x1d6a,%ax
    400d:	eb 44                	jmp    0x4053
    400f:	a2 2f 71             	mov    %al,0x712f
    4012:	59                   	pop    %cx
    4013:	06                   	push   %es
    4014:	73 b8                	jae    0x3fce
    4016:	16                   	push   %ss
    4017:	d0 1d                	rcrb   $1,(%di)
    4019:	eb 38                	jmp    0x4053
    401b:	a0 ca 1e             	mov    0x1eca,%al
    401e:	fa                   	cli
    401f:	01 44 5d             	add    %ax,0x5d(%si)
    4022:	c9                   	leave
    4023:	1e                   	push   %ds
    4024:	fa                   	cli
    4025:	ed                   	in     (%dx),%ax
    4026:	c1 8a fa 18 c1       	rorw   $0xc1,0x18fa(%bp,%si)
    402b:	01 5d dc             	add    %bx,-0x24(%di)
    402e:	8a fa                	mov    %dl,%bh
    4030:	44                   	inc    %sp
    4031:	c9                   	leave
    4032:	2b 01                	sub    (%bx,%di),%ax
    4034:	ca 1e 2b             	lret   $0x2b1e
    4037:	01 79 8b             	add    %di,-0x75(%bx,%di)
    403a:	c6 01 4c             	movb   $0x4c,(%bx,%di)
    403d:	eb b8                	jmp    0x3ff7
    403f:	1d 02 b8             	sbb    $0xb802,%ax
    4042:	26 1e                	es push %ds
    4044:	47                   	inc    %di
    4045:	8d 0c                	lea    (%si),%cx
    4047:	b0 46                	mov    $0x46,%al
    4049:	e8 a6 5d             	call   0x9df2
    404c:	5b                   	pop    %bx
    404d:	5b                   	pop    %bx
    404e:	eb 15                	jmp    0x4065
    4050:	1d 02 1e             	sbb    $0x1e02,%ax
    4053:	47                   	inc    %di
    4054:	e8 7d 19             	call   0x59d4
    4057:	ff 4a e8             	decw   -0x18(%bp,%si)
    405a:	1c 0d                	sbb    $0xd,%al
    405c:	87 04                	xchg   %ax,(%si)
    405e:	46                   	inc    %si
    405f:	ff 05                	incw   (%di)
    4061:	a1 95 06             	mov    0x695,%ax
    4064:	52                   	push   %dx
    4065:	8e b0 fe d8          	mov    -0x2702(%bx,%si),%?
    4069:	04 08                	add    $0x8,%al
    406b:	57                   	push   %di
    406c:	c7 03 00 14          	movw   $0x1400,(%bp,%di)
    4070:	89 2b                	mov    %bp,(%bp,%di)
    4072:	fe 0a                	decb   (%bp,%si)
    4074:	d4 f8                	aam    $0xf8
    4076:	57                   	push   %di
    4077:	e8 db dc             	call   0x1d55
    407a:	e8 93 06             	call   0x4710
    407d:	76 74                	jbe    0x40f3
    407f:	1a c7                	sbb    %bh,%al
    4081:	03 90 00 aa          	add    -0x5600(%bx,%si),%dx
    4085:	01 b8 0a 1c          	add    %di,0x1c0a(%bx,%si)
    4089:	50                   	push   %ax
    408a:	e8 21 3a             	call   0x7aae
    408d:	14 8b                	adc    $0x8b,%al
    408f:	f8                   	clc
    4090:	00 77 1b             	add    %dh,0x1b(%bx)
    4093:	ba 90 00             	mov    $0x90,%dx
    4096:	5f                   	pop    %di
    4097:	01 eb                	add    %bp,%bx
    4099:	fa                   	cli
    409a:	fb                   	sti
    409b:	62 01                	bound  %ax,(%bx,%di)
    409d:	0e                   	push   %cs
    409e:	2b a2 26 fe          	sub    -0x1da(%bp,%si),%sp
    40a2:	c8 ba 90 00          	enter  $0x90ba,$0x0
    40a6:	c0 00 fe             	rolb   $0xfe,(%bx,%si)
    40a9:	c8 ba 90 00          	enter  $0x90ba,$0x0
    40ad:	f3 00 c8             	repz add %cl,%al
    40b0:	90                   	nop
    40b1:	f3 c8 f3 1a 01       	repz enter $0x1af3,$0x1
    40b6:	c8 90 f3 c8          	enter  $0xf390,$0xc8
    40ba:	f3 32 01             	repz xor (%bx,%di),%al
    40bd:	90                   	nop
    40be:	c8 32 90 32          	enter  $0x9032,$0x32
    40c2:	33 01                	xor    (%bx,%di),%ax
    40c4:	e9 38 01             	jmp    0x41ff
    40c7:	e8 11 f5             	call   0x35db
    40ca:	a2 2f ae             	mov    %al,0xae2f
    40cd:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    40ce:	0c 75                	or     $0x75,%al
    40d0:	b8 12 9c             	mov    $0x9c12,%ax
    40d3:	1c 50                	sbb    $0x50,%al
    40d5:	e8 fc 18             	call   0x59d4
    40d8:	e9 24 01             	jmp    0x41ff
    40db:	71 02                	jno    0x40df
    40dd:	74 bf                	je     0x409e
    40df:	59                   	pop    %cx
    40e0:	b8 a0 ae             	mov    $0xaea0,%ax
    40e3:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    40e4:	38 87 5e 59          	cmp    %al,0x595e(%bx)
    40e8:	77 b8                	ja     0x40a2
    40ea:	16                   	push   %ss
    40eb:	ba 1c eb             	mov    $0xeb1c,%dx
    40ee:	e5 89                	in     $0x89,%ax
    40f0:	fa                   	cli
    40f1:	fe                   	(bad)
    40f2:	b8 1c 1c             	mov    $0x1c1c,%ax
    40f5:	50                   	push   %ax
    40f6:	50                   	push   %ax
    40f7:	09 52 2b             	or     %dx,0x2b(%bp,%si)
    40fa:	36 17                	ss pop %ss
    40fc:	15 32 03             	adc    $0x332,%ax
    40ff:	8c 14                	mov    %ss,(%si)
    4101:	b9 89 7e             	mov    $0x7e89,%cx
    4104:	fe                   	(bad)
    4105:	eb 51                	jmp    0x4158
    4107:	b8 1c 1c             	mov    $0x1c1c,%ax
    410a:	50                   	push   %ax
    410b:	c4 8a 73 a3          	les    -0x5c8d(%bp,%si),%cx
    410f:	b7 13                	mov    $0x13,%bh
    4111:	74 f8                	je     0x410b
    4113:	4e                   	dec    %si
    4114:	33 b8 48 22          	xor    0x2248(%bx,%si),%di
    4118:	48                   	dec    %ax
    4119:	74 13                	je     0x412e
    411b:	48                   	dec    %ax
    411c:	75 2a                	jne    0x4148
    411e:	a2 71 06             	mov    %al,0x671
    4121:	b8 d0 eb             	mov    $0xebd0,%ax
    4124:	09 16 1d 38          	or     %dx,0x381d
    4128:	2b 36 a8 18          	sub    0x18a8,%si
    412c:	eb 1d                	jmp    0x414b
    412e:	a2 71 02             	mov    %al,0x271
    4131:	b8 9e eb             	mov    $0xeb9e,%ax
    4134:	a0 1e 01             	mov    0x11e,%al
    4137:	5d                   	pop    %bp
    4138:	1e                   	push   %ds
    4139:	ee                   	out    %al,(%dx)
    413a:	a0 95 06             	mov    0x695,%al
    413d:	fd                   	std
    413e:	c4 8a 73 a3          	les    -0x5c8d(%bp,%si),%cx
    4142:	4e                   	dec    %si
    4143:	02 03                	add    (%bp,%di),%al
    4145:	46                   	inc    %si
    4146:	1a 00                	sbb    (%bx,%si),%al
    4148:	00 80 b9 b8          	add    %al,-0x4747(%bx,%si)
    414c:	1c c4                	sbb    $0xc4,%al
    414e:	73 b7                	jae    0x4107
    4150:	74 4e                	je     0x41a0
    4152:	ec                   	in     (%dx),%al
    4153:	04 94                	add    $0x94,%al
    4155:	00 03                	add    %al,(%bp,%di)
    4157:	17                   	pop    %ss
    4158:	0b ff                	or     %di,%di
    415a:	74 ab                	je     0x4107
    415c:	e8 c4 fd             	call   0x3f23
    415f:	e8 ae 05             	call   0x4710
    4162:	89 7e fa             	mov    %di,-0x6(%bp)
    4165:	e9 97 00             	jmp    0x41ff
    4168:	b8 0a 1c             	mov    $0x1c0a,%ax
    416b:	50                   	push   %ax
    416c:	e8 90 14             	call   0x55ff
    416f:	a0 95 06             	mov    0x695,%al
    4172:	fd                   	std
    4173:	ec                   	in     (%dx),%al
    4174:	04 94                	add    $0x94,%al
    4176:	00 36 fa 80          	add    %dh,-0x7f06
    417a:	1b 17                	sbb    (%bx),%dx
    417c:	17                   	pop    %ss
    417d:	0c 73                	or     $0x73,%al
    417f:	0e                   	push   %cs
    4180:	a0 95 06             	mov    0x695,%al
    4183:	fd                   	std
    4184:	30 b9 09 2b          	xor    %bh,0x2b09(%bx,%di)
    4188:	f0 b0 03             	lock mov $0x3,%al
    418b:	e8 95 fd             	call   0x3f23
    418e:	a0 95 06             	mov    0x695,%al
    4191:	fd                   	std
    4192:	b8 58 38             	mov    $0x3858,%ax
    4195:	2b 36 17 fb          	sub    -0x4e9,%si
    4199:	b8 0a 1c             	mov    $0x1c0a,%ax
    419c:	50                   	push   %ax
    419d:	e8 4a 14             	call   0x55ea
    41a0:	eb 5d                	jmp    0x41ff
    41a2:	b8 0a 1c             	mov    $0x1c0a,%ax
    41a5:	50                   	push   %ax
    41a6:	e8 56 14             	call   0x55ff
    41a9:	95                   	xchg   %ax,%bp
    41aa:	fd                   	std
    41ab:	04 00                	add    $0x0,%al
    41ad:	fa                   	cli
    41ae:	1b 17                	sbb    (%bx),%dx
    41b0:	73 fc                	jae    0x41ae
    41b2:	f9                   	stc
    41b3:	80 1b 17             	sbbb   $0x17,(%bp,%di)
    41b6:	b8 0a 1c             	mov    $0x1c0a,%ax
    41b9:	50                   	push   %ax
    41ba:	e8 2d 14             	call   0x55ea
    41bd:	89 04                	mov    %ax,(%si)
    41bf:	4b                   	dec    %bx
    41c0:	0c 72                	or     $0x72,%al
    41c2:	4a                   	dec    %dx
    41c3:	a0 06 b8             	mov    0xb806,%al
    41c6:	38 36 fb 0a          	cmp    %dh,0xafb
    41ca:	50                   	push   %ax
    41cb:	4a                   	dec    %dx
    41cc:	e2 fa                	loop   0x41c8
    41ce:	eb 2f                	jmp    0x41ff
    41d0:	b8 0a 1c             	mov    $0x1c0a,%ax
    41d3:	50                   	push   %ax
    41d4:	e8 28 14             	call   0x55ff
    41d7:	95                   	xchg   %ax,%bp
    41d8:	fd                   	std
    41d9:	04 00                	add    $0x0,%al
    41db:	c2 66 2a             	ret    $0x2a66
    41de:	fe                   	(bad)
    41df:	6d                   	insw   (%dx),%es:(%di)
    41e0:	cd f9                	int    $0xf9
    41e2:	80 1b 17             	sbbb   $0x17,(%bp,%di)
    41e5:	17                   	pop    %ss
    41e6:	0c 73                	or     $0x73,%al
    41e8:	a5                   	movsw  %ds:(%si),%es:(%di)
    41e9:	fe                   	(bad)
    41ea:	6d                   	insw   (%dx),%es:(%di)
    41eb:	aa                   	stos   %al,%es:(%di)
    41ec:	fd                   	std
    41ed:	eb 9c                	jmp    0x418b
    41ef:	e8 31 fd             	call   0x3f23
    41f2:	e8 8b 00             	call   0x4280
    41f5:	eb cc                	jmp    0x41c3
    41f7:	e8 29 fd             	call   0x3f23
    41fa:	02 76 74             	add    0x74(%bp),%dh
    41fd:	49                   	dec    %cx
    41fe:	a1 b8 1c             	mov    0x1cb8,%ax
    4201:	e8 3a 8b             	call   0xcd3e
    4204:	01 24                	add    %sp,(%si)
    4206:	51                   	push   %cx
    4207:	e8 e8 16             	call   0x58f2
    420a:	e9 70 fe             	jmp    0x407d
    420d:	e8 13 fd             	call   0x3f23
    4210:	a0 95 06             	mov    0x695,%al
    4213:	fd                   	std
    4214:	e8 33 07             	call   0x494a
    4217:	30 b9 09 2b          	xor    %bh,0x2b09(%bx,%di)
    421b:	f0 85 fc             	lock test %di,%sp
    421e:	e8 ad da             	call   0x1cce
    4221:	30 b9 09 2b          	xor    %bh,0x2b09(%bx,%di)
    4225:	f0 02 f6             	lock add %dh,%dh
    4228:	e8 5f db             	call   0x1d8a
    422b:	c2 d2 1a             	ret    $0x1ad2
    422e:	e2 05                	loop   0x4235
    4230:	e8 57 db             	call   0x1d8a
    4233:	01 24                	add    %sp,(%si)
    4235:	f6 56 c6             	notb   -0x3a(%bp)
    4238:	2e 05 89 08          	cs add $0x889,%ax
    423c:	76 a0                	jbe    0x41de
    423e:	d1 ff                	sar    $1,%di
    4240:	be 11 24             	mov    $0x2411,%si
    4243:	19 eb                	sbb    %bp,%bx
    4245:	2f                   	das
    4246:	80 7c 01 ff          	cmpb   $0xff,0x1(%si)
    424a:	74 26                	je     0x4272
    424c:	8b 16 0a c1          	mov    -0x3ef6,%dx
    4250:	8a 04                	mov    (%si),%al
    4252:	80 1b 17             	sbbb   $0x17,(%bp,%di)
    4255:	17                   	pop    %ss
    4256:	c0 c1 ec             	rol    $0xec,%cl
    4259:	16                   	push   %ss
    425a:	c0 08 c1             	rorb   $0xc1,(%bx,%si)
    425d:	52                   	push   %dx
    425e:	ff 22                	jmp    *(%bp,%si)
    4260:	12 c8                	adc    %al,%cl
    4262:	ff                   	(bad)
    4263:	b8 ff ba             	mov    $0xbaff,%ax
    4266:	e8 63 81             	call   0xc3cc
    4269:	1a 5e 72             	sbb    0x72(%bp),%bl
    426c:	5e                   	pop    %si
    426d:	c7                   	(bad)
    426e:	f4                   	hlt
    426f:	98                   	cbtw
    4270:	f0 4b                	lock dec %bx
    4272:	83 14 3a             	adcw   $0x3a,(%si)
    4275:	81 fe 14 1a          	cmp    $0x1a14,%si
    4279:	72 cb                	jb     0x4246
    427b:	5e                   	pop    %si
    427c:	c7                   	(bad)
    427d:	f4                   	hlt
    427e:	98                   	cbtw
    427f:	f0 0a fa             	lock or %dl,%bh
    4282:	80 ff ff             	cmp    $0xff,%bh
    4285:	42                   	inc    %dx
    4286:	a1 95 06             	mov    0x695,%ax
    4289:	ed                   	in     (%dx),%ax
    428a:	80 01 c7             	addb   $0xc7,(%bx,%di)
    428d:	da a0 95 06          	fisubl 0x695(%bx,%si)
    4291:	fd                   	std
    4292:	e8 af fc             	call   0x3f44
    4295:	a2 ae 0c             	mov    %al,0xcae
    4298:	b8 1b 90             	mov    $0x901b,%ax
    429b:	5f                   	pop    %di
    429c:	eb ea                	jmp    0x4288
    429e:	00 a0 a6 87          	add    %ah,-0x785a(%bx,%si)
    42a2:	04 f6                	add    $0xf6,%al
    42a4:	9b                   	fwait
    42a5:	02 2a                	add    (%bp,%si),%ch
    42a7:	04 f7                	add    $0xf7,%al
    42a9:	81 2d 00 d8          	subw   $0xd800,(%di)
    42ad:	2b d1                	sub    %cx,%dx
    42af:	8b 03                	mov    (%bp,%di),%ax
    42b1:	89 fa                	mov    %di,%dx
    42b3:	2a a3 f0 03          	sub    0x3f0(%bp,%di),%ah
    42b7:	83 03 04             	addw   $0x4,(%bp,%di)
    42ba:	27                   	daa
    42bb:	ba 01 51             	mov    $0x5101,%dx
    42be:	e9 00 06             	jmp    0x48c1
    42c1:	57                   	push   %di
    42c2:	3d 71 76             	cmp    $0x7671,%ax
    42c5:	ff                   	ljmp   (bad)
    42c6:	e9 e8 13             	jmp    0x56b1
    42c9:	a3 53 46             	mov    %ax,0x4653
    42cc:	31 46 5b             	xor    %ax,0x5b(%bp)
    42cf:	3e 40                	ds inc %ax
    42d1:	8d be e2 6d          	lea    0x6de2(%bp),%di
    42d5:	ff 0f                	decw   (%bx)
    42d7:	e8 a0 46             	call   0x897a
    42da:	3f                   	aas
    42db:	26 00 06 57 3d       	add    %al,%es:0x3d57
    42e0:	71 76                	jno    0x4358
    42e2:	ff                   	ljmp   (bad)
    42e3:	e9 e8 13             	jmp    0x56ce
    42e6:	a3 53 46             	mov    %ax,0x4653
    42e9:	31 46 5b             	xor    %ax,0x5b(%bp)
    42ec:	3e 45                	ds inc %bp
    42ee:	ee                   	out    %al,(%dx)
    42ef:	b8 70 1f             	mov    $0x1f70,%ax
    42f2:	53                   	push   %bx
    42f3:	46                   	inc    %si
    42f4:	31 46 5b             	xor    %ax,0x5b(%bp)
    42f7:	3e 38 5b 5b          	cmp    %bl,%ds:0x5b(%bp,%di)
    42fb:	3e 40                	ds inc %ax
    42fd:	8d be e2 6d          	lea    0x6de2(%bp),%di
    4301:	ff 0f                	decw   (%bx)
    4303:	e8 a0 eb             	call   0x2ea6
    4306:	5a                   	pop    %dx
    4307:	5b                   	pop    %bx
    4308:	19 00                	sbb    %ax,(%bx,%si)
    430a:	b6 8d                	mov    $0x8d,%dh
    430c:	be 66 00             	mov    $0x66,%si
    430f:	8d 05                	lea    (%di),%ax
    4311:	e2 ff                	loop   0x4312
    4313:	0f e8 b8 03 00       	psubsb 0x3(%bx,%si),%mm7
    4318:	69 07 02 5e          	imul   $0x5e02,(%bx),%ax
    431c:	c6                   	(bad)
    431d:	48                   	dec    %ax
    431e:	19 8b 1e 04          	sbb    %cx,0x41e(%bp,%di)
    4322:	55                   	push   %bp
    4323:	8a 73 cc             	mov    -0x34(%bp,%di),%dh
    4326:	55                   	push   %bp
    4327:	8a 73 e5             	mov    -0x1b(%bp,%di),%dh
    432a:	08 90 a0 e4          	or     %dl,-0x1b60(%bx,%si)
    432e:	d1 03                	rolw   $1,(%bp,%di)
    4330:	e0 03                	loopne 0x4335
    4332:	7a 0a                	jp     0x433e
    4334:	a0 e4 d1             	mov    0xd1e4,%al
    4337:	0e                   	push   %cs
    4338:	f0 8a 60 46          	lock mov 0x46(%bx,%si),%ah
    433c:	a0 e4 d1             	mov    0xd1e4,%al
    433f:	0e                   	push   %cs
    4340:	f0 8a 38             	lock mov (%bx,%si),%bh
    4343:	fc                   	cld
    4344:	eb b0                	jmp    0x42f6
    4346:	ff                   	(bad)
    4347:	bf 03 4d             	mov    $0x4d03,%di
    434a:	7e 72                	jle    0x43be
    434c:	30 23                	xor    %ah,(%bp,%di)
    434e:	b0 fe                	mov    $0xfe,%al
    4350:	bf 5e 87             	mov    $0x875e,%di
    4353:	19 e4                	sbb    %sp,%sp
    4355:	80 bf 25 19 ff       	cmpb   $0xff,0x1925(%bx)
    435a:	74 12                	je     0x436e
    435c:	30 33                	xor    %dh,(%bp,%di)
    435e:	a0 e4 d1             	mov    0xd1e4,%al
    4361:	f0 e8 4d 7e          	lock call 0xc1b2
    4365:	72 03                	jb     0x436a
    4367:	e6 06                	out    %al,$0x6
    4369:	ed                   	in     (%dx),%ax
    436a:	80 01 49             	addb   $0x49,(%bx,%di)
    436d:	fa                   	cli
    436e:	fd                   	std
    436f:	ff c6                	inc    %si
    4371:	80 da 2c             	sbb    $0x2c,%dl
    4374:	0c 72                	or     $0x72,%al
    4376:	d7                   	xlat   %ds:(%bx)
    4377:	e8 7f f2             	call   0x35f9
    437a:	04 ba                	add    $0xba,%al
    437c:	51                   	push   %cx
    437d:	01 fa                	add    %di,%dx
    437f:	63 e8                	arpl   %bp,%ax
    4381:	38 ab 5e 19          	cmp    %ch,0x195e(%bp,%di)
    4385:	83 af ea 25 e8       	subw   $0xffe8,0x25ea(%bx)
    438a:	fe                   	(bad)
    438b:	d9 46 00             	flds   0x0(%bp)
    438e:	3e 06                	ds push %es
    4390:	bc 08 fa             	mov    $0xfa08,%sp
    4393:	0d 08 e4             	or     $0xe408,%ax
    4396:	f0 0a 80 8c f6       	lock or -0x974(%bx,%si),%al
    439b:	b9 08 89             	mov    $0x8908,%cx
    439e:	01 b1 22 16          	add    %si,0x1622(%bx,%di)
    43a2:	9a 88 87 e3 89       	lcall  $0x89e3,$0x8788
    43a7:	7e 07                	jle    0x43b0
    43a9:	d3 e3                	shl    %cl,%bx
    43ab:	c3                   	ret
    43ac:	37                   	aaa
    43ad:	07                   	pop    %es
    43ae:	c7                   	(bad)
    43af:	97                   	xchg   %ax,%di
    43b0:	74 52                	je     0x4404
    43b2:	83 fa 64             	cmp    $0x64,%dx
    43b5:	7d 0c                	jge    0x43c3
    43b7:	b8 98 1e             	mov    $0x1e98,%ax
    43ba:	e8 19 16             	call   0x59d6
    43bd:	16                   	push   %ss
    43be:	06                   	push   %es
    43bf:	ed                   	in     (%dx),%ax
    43c0:	f3 01 90 89 04       	repz add %dx,0x489(%bx,%si)
    43c5:	57                   	push   %di
    43c6:	8a 87 c8 fa          	mov    -0x538(%bx),%al
    43ca:	2b 03                	sub    (%bp,%di),%ax
    43cc:	4a                   	dec    %dx
    43cd:	5b                   	pop    %bx
    43ce:	01 eb                	add    %bp,%bx
    43d0:	5e                   	pop    %si
    43d1:	16                   	push   %ss
    43d2:	9a 5e b1 ff 8b       	lcall  $0x8bff,$0xb15e
    43d7:	57                   	push   %di
    43d8:	2d ac 1a             	sub    $0x1aac,%ax
    43db:	fe                   	(bad)
    43dc:	fa                   	cli
    43dd:	fc                   	cld
    43de:	8b 8f b2 1a          	mov    0x1ab2(%bx),%cx
    43e2:	fe 4e fc             	decb   -0x4(%bp)
    43e5:	03 c1                	add    %cx,%ax
    43e7:	3b c2                	cmp    %dx,%ax
    43e9:	7e 13                	jle    0x43fe
    43eb:	3b ca                	cmp    %dx,%cx
    43ed:	7e 0a                	jle    0x43f9
    43ef:	89 56 fc             	mov    %dx,-0x4(%bp)
    43f2:	19 14                	sbb    %dx,(%si)
    43f4:	63 c6                	arpl   %ax,%si
    43f6:	fe                   	(bad)
    43f7:	eb 05                	jmp    0x43fe
    43f9:	2b d1                	sub    %cx,%dx
    43fb:	89 56 fa             	mov    %dx,-0x6(%bp)
    43fe:	2b bf 33 00          	sub    0x33(%bx),%di
    4402:	fe                   	(bad)
    4403:	25 19 81             	and    $0x8119,%ax
    4406:	7e f8                	jle    0x4400
    4408:	1a d0                	sbb    %al,%dl
    440a:	8c 16 9a 9f          	mov    %ss,-0x6066
    440e:	63 fe                	arpl   %di,%si
    4410:	11 fe                	adc    %di,%si
    4412:	80 3f ff             	cmpb   $0xff,(%bx)
    4415:	74 07                	je     0x441e
    4417:	83 47 20 14          	addw   $0x14,0x20(%bx)
    441b:	47                   	inc    %di
    441c:	eb e7                	jmp    0x4405
    441e:	57                   	push   %di
    441f:	f0 36 fc             	lock ss cld
    4422:	ff f6                	push   %si
    4424:	89 87 0e 88          	mov    %ax,-0x77f2(%bx)
    4428:	04 11                	add    $0x11,%al
    442a:	11 fe                	adc    %di,%si
    442c:	25 19 06             	and    $0x619,%ax
    442f:	fa                   	cli
    4430:	20 ec                	and    %ch,%ah
    4432:	eb 1e                	jmp    0x4452
    4434:	89 04                	mov    %ax,(%si)
    4436:	e8 fe fa             	call   0x3f37
    4439:	87 05                	xchg   %ax,(%di)
    443b:	76 89                	jbe    0x43c6
    443d:	8b f8                	mov    %ax,%di
    443f:	8a 84 7a 52          	mov    0x527a(%si),%al
    4443:	11 fe                	adc    %di,%si
    4445:	23 19                	and    (%bx,%di),%bx
    4447:	8e 06 1d eb          	mov    -0x14e3,%es
    444b:	24 a1                	and    $0xa1,%al
    444d:	fc                   	cld
    444e:	11 26 01 8e          	adc    %sp,-0x71ff
    4452:	02 87 11 27          	add    0x2711(%bx),%al
    4456:	19 00                	sbb    %ax,(%bx,%si)
    4458:	eb 24                	jmp    0x447e
    445a:	2a fc                	sub    %ah,%bh
    445c:	11 26 29 ff          	adc    %sp,-0xd7
    4460:	0e                   	push   %cs
    4461:	89 8b f6 d1          	mov    %cx,-0x2e0a(%bp,%di)
    4465:	e6 ff                	out    %al,$0xff
    4467:	b4 2e                	mov    $0x2e,%ah
    4469:	1c 81                	sbb    $0x81,%al
    446b:	c3                   	ret
    446c:	18 19                	sbb    %bl,(%bx,%di)
    446e:	53                   	push   %bx
    446f:	e8 c0 59             	call   0x9e32
    4472:	5b                   	pop    %bx
    4473:	5b                   	pop    %bx
    4474:	04 f8                	add    $0xf8,%al
    4476:	14 fe                	adc    $0xfe,%al
    4478:	87 88 52 80          	xchg   %cx,-0x7fae(%bx,%si)
    447c:	bf 88 52             	mov    $0x5288,%di
    447f:	32 76 08             	xor    0x8(%bp),%dh
    4482:	89 87 f8 c6          	mov    %ax,-0x3908(%bx)
    4486:	0b 88 52 90          	or     -0x6fae(%bx,%si),%cx
    448a:	89 f0                	mov    %si,%ax
    448c:	36 fc                	ss cld
    448e:	ff 87 0e f6          	incw   -0x9f2(%bx)
    4492:	89 87 f0 8b          	mov    %ax,-0x7410(%bx)
    4496:	7e f8                	jle    0x4490
    4498:	8a 85 88 52          	mov    0x5288(%di),%al
    449c:	88 84 22 19          	mov    %al,0x1922(%si)
    44a0:	06                   	push   %es
    44a1:	fa                   	cli
    44a2:	20 03                	and    %al,(%bp,%di)
    44a4:	e8 fe 29             	call   0x6ea5
    44a7:	85 74 52             	test   %si,0x52(%si)
    44aa:	53                   	push   %bx
    44ab:	e8 85 02             	call   0x4733
    44ae:	89 57 87             	mov    %dx,-0x79(%bx)
    44b1:	fa                   	cli
    44b2:	03 5b eb             	add    -0x15(%bp,%di),%bx
    44b5:	16                   	push   %ss
    44b6:	5e                   	pop    %si
    44b7:	29 e9                	sub    %bp,%cx
    44b9:	ff                   	lcall  (bad)
    44ba:	da 54 b8             	ficoml -0x48(%si)
    44bd:	e9 eb 0a             	jmp    0x4fab
    44c0:	b8 1d 44             	mov    $0x441d,%ax
    44c3:	50                   	push   %ax
    44c4:	e8 0d 15             	call   0x59d4
    44c7:	75 e4                	jne    0x44ad
    44c9:	2a ff                	sub    %bh,%bh
    44cb:	e8 0d 04             	call   0x48db
    44ce:	ff                   	(bad)
    44cf:	fc                   	cld
    44d0:	ff 87 87 f8          	incw   -0x779(%bx)
    44d4:	b8 b8 0a             	mov    $0xab8,%ax
    44d7:	fc                   	cld
    44d8:	e8 35 52             	call   0x9710
    44db:	c6                   	(bad)
    44dc:	f7 5e 2b             	negw   0x2b(%bp)
    44df:	57                   	push   %di
    44e0:	fe                   	(bad)
    44e1:	9b                   	fwait
    44e2:	8a fe                	mov    %dh,%bh
    44e4:	08 89 04 8a          	or     %cl,-0x75fc(%bx,%di)
    44e8:	a1 9a 89             	mov    0x899a,%ax
    44eb:	f9                   	stc
    44ec:	ff 00                	incw   (%bx,%si)
    44ee:	69 00 5e 12          	imul   $0x125e,(%bx,%si),%ax
    44f2:	fc                   	cld
    44f3:	d2 63 c2             	shlb   %cl,-0x3e(%bp,%di)
    44f6:	04 a2                	add    $0xa2,%al
    44f8:	90                   	nop
    44f9:	03 04                	add    (%si),%ax
    44fb:	53                   	push   %bx
    44fc:	d3 42 c2             	rolw   %cl,-0x3e(%bp,%si)
    44ff:	66 a7                	cmpsl  %es:(%di),%ds:(%si)
    4501:	42                   	inc    %dx
    4502:	c2 66 37             	ret    $0x3766
    4505:	52                   	push   %dx
    4506:	36 15 e8 45          	ss adc $0x45e8,%ax
    450a:	ff a1 06 8e          	jmp    *-0x71fa(%bx,%di)
    450e:	fe 04                	incb   (%si)
    4510:	57                   	push   %di
    4511:	03 14                	add    (%si),%dx
    4513:	a1 89 ff             	mov    0xff89,%ax
    4516:	69 5e fc 63 04       	imul   $0x463,-0x4(%bp),%bx
    451b:	90                   	nop
    451c:	04 d3                	add    $0xd3,%al
    451e:	c2 a7 c2             	ret    $0xc2a7
    4521:	37                   	aaa
    4522:	36 e8 ff 06          	ss call 0x4c25
    4526:	fe                   	(bad)
    4527:	e1 42                	loope  0x456b
    4529:	c2 66 80             	ret    $0x8066
    452c:	42                   	inc    %dx
    452d:	c2 66 18             	ret    $0x1866
    4530:	52                   	push   %dx
    4531:	36 15 bd 45          	ss adc $0x45bd,%ax
    4535:	a1 8e 04             	mov    0x48e,%ax
    4538:	03 a1 ff 5e          	add    0x5eff(%bx,%di),%sp
    453c:	63 90 83 ec          	arpl   %dx,-0x137d(%bx,%si)
    4540:	18 76 89             	sbb    %dh,-0x77(%bp)
    4543:	e8 fc 47             	call   0x8d42
    4546:	2b f8                	sub    %ax,%di
    4548:	89 fa                	mov    %di,%dx
    454a:	e8 fe d8             	call   0x1e4b
    454d:	04 ee                	add    $0xee,%al
    454f:	18 47 8f             	sbb    %al,-0x71(%bx)
    4552:	5e                   	pop    %si
    4553:	8a f4                	mov    %ah,%dh
    4555:	fa                   	cli
    4556:	03 25                	add    (%di),%sp
    4558:	88 f1                	mov    %dh,%cl
    455a:	16                   	push   %ss
    455b:	ff 8a ba c7          	decw   -0x3846(%bp,%si)
    455f:	eb 86                	jmp    0x44e7
    4561:	b8 8b 87             	mov    $0x878b,%ax
    4564:	b2 1a                	mov    $0x1a,%dl
    4566:	03 2d                	add    (%di),%bp
    4568:	1a fa                	sbb    %dl,%bh
    456a:	8b b2 fe fc          	mov    -0x302(%bp,%si),%si
    456e:	76 f4                	jbe    0x4564
    4570:	8b 84 1e 19          	mov    0x191e(%si),%ax
    4574:	39 2d                	cmp    %bp,(%di)
    4576:	1a fa                	sbb    %dl,%bh
    4578:	75 1d                	jne    0x4597
    457a:	8b de                	mov    %si,%bx
    457c:	8b 20                	mov    (%bx,%si),%sp
    457e:	a2 90 ff             	mov    %al,0xff90
    4581:	12 ff                	adc    %bh,%bh
    4583:	ff                   	ljmp   (bad)
    4584:	e8 81 5e             	call   0xa408
    4587:	3b 87 b2 1a          	cmp    0x1ab2(%bx),%ax
    458b:	75 12                	jne    0x459f
    458d:	1c 74                	sbb    $0x74,%al
    458f:	1e                   	push   %ds
    4590:	19 16 40 14          	sbb    %dx,0x1440
    4594:	e9 71 01             	jmp    0x4708
    4597:	8b de                	mov    %si,%bx
    4599:	8b 20                	mov    (%bx,%si),%sp
    459b:	1e                   	push   %ds
    459c:	19 03                	sbb    %ax,(%bp,%di)
    459e:	2d a2 90             	sub    $0x90a2,%ax
    45a1:	eb 75                	jmp    0x4618
    45a3:	8c ff                	mov    %?,%di
    45a5:	12 ff                	adc    %bh,%bh
    45a7:	ff                   	ljmp   (bad)
    45a8:	e8 5e 8a             	call   0xd009
    45ab:	f0 fa                	lock cli
    45ad:	5b                   	pop    %bx
    45ae:	16                   	push   %ss
    45af:	29 ff                	sub    %di,%di
    45b1:	54                   	push   %sp
    45b2:	7e 69                	jle    0x461d
    45b4:	83 4a 01 5e          	orw    $0x5e,0x1(%bp,%si)
    45b8:	9a b1 13 36 e8       	lcall  $0xe836,$0x13b1
    45bd:	ff 06 eb 75          	incw   0x75eb
    45c1:	a2 f7 66             	mov    %al,0x66f7
    45c4:	fa                   	cli
    45c5:	0b e8                	or     %ax,%bp
    45c7:	6a 7e                	push   $0x7e
    45c9:	5a                   	pop    %dx
    45ca:	8b f8                	mov    %ax,%di
    45cc:	eb 0d                	jmp    0x45db
    45ce:	b1 16                	mov    $0x16,%cl
    45d0:	88 e3                	mov    %ah,%bl
    45d2:	7e d3                	jle    0x45a7
    45d4:	c3                   	ret
    45d5:	d3 b8 8b bf          	sarw   %cl,-0x4075(%bx,%si)
    45d9:	74 52                	je     0x462d
    45db:	b1 88                	mov    $0x88,%cl
    45dd:	7e c3                	jle    0x45a2
    45df:	b8 bf 52             	mov    $0x52bf,%ax
    45e2:	88 c3                	mov    %al,%bl
    45e4:	89 87 03 8b          	mov    %ax,-0x74fd(%bx)
    45e8:	b8 fc 35             	mov    $0x35fc,%ax
    45eb:	3b f7                	cmp    %di,%si
    45ed:	76 02                	jbe    0x45f1
    45ef:	8b f7                	mov    %di,%si
    45f1:	8b 8a f4 8b          	mov    -0x740c(%bp,%si),%cx
    45f5:	7e f0                	jle    0x45e7
    45f7:	d1 e7                	shl    $1,%di
    45f9:	89 7e ec             	mov    %di,-0x14(%bp)
    45fc:	8b 85 b2 1a          	mov    0x1ab2(%di),%ax
    4600:	39 2d                	cmp    %bp,(%di)
    4602:	a2 90 72             	mov    %al,0x7290
    4605:	05 7e e9             	add    $0xe97e,%ax
    4608:	f8                   	clc
    4609:	eb 32                	jmp    0x463d
    460b:	8b df                	mov    %di,%bx
    460d:	8b 87 b2 1a          	mov    0x1ab2(%bx),%ax
    4611:	8b 8a f4 2b          	mov    0x2bf4(%bp,%si),%cx
    4615:	87 76 8b             	xchg   %si,-0x75(%bp)
    4618:	8a 87 03 8b          	mov    -0x74fd(%bx),%al
    461c:	bf 74 52             	mov    $0x5274,%di
    461f:	d1 ef                	shr    $1,%di
    4621:	d1 ef                	shr    $1,%di
    4623:	3b fe                	cmp    %si,%di
    4625:	76 02                	jbe    0x4629
    4627:	8b fe                	mov    %si,%di
    4629:	3b c7                	cmp    %di,%ax
    462b:	73 10                	jae    0x463d
    462d:	8a 87 ec 8b          	mov    -0x7414(%bx),%al
    4631:	87 1a                	xchg   %bx,(%bp,%si)
    4633:	8a 2b                	mov    (%bp,%di),%ch
    4635:	76 8a                	jbe    0x45c1
    4637:	03 bf 52 ef          	add    -0x10ae(%bx),%di
    463b:	ef                   	out    %ax,(%dx)
    463c:	f8                   	clc
    463d:	2b f7                	sub    %di,%si
    463f:	8b 8a ec 8b          	mov    -0x7414(%bp,%si),%cx
    4643:	87 1a                	xchg   %bx,(%bp,%si)
    4645:	fa                   	cli
    4646:	2b 76 8a             	sub    -0x76(%bp),%si
    4649:	39 2d                	cmp    %bp,(%di)
    464b:	1e                   	push   %ds
    464c:	19 73 26             	sbb    %si,0x26(%bp,%di)
    464f:	5e                   	pop    %si
    4650:	80 ec 87             	sub    $0x87,%ah
    4653:	fa                   	cli
    4654:	76 39                	jbe    0x468f
    4656:	1e                   	push   %ds
    4657:	73 5e                	jae    0x46b7
    4659:	03 bf 1e 19          	add    0x191e(%bx),%di
    465d:	89 76 f6             	mov    %si,-0xa(%bp)
    4660:	3b c6                	cmp    %si,%ax
    4662:	73 11                	jae    0x4675
    4664:	5e                   	pop    %si
    4665:	ec                   	in     (%dx),%al
    4666:	fa                   	cli
    4667:	39 73 03             	cmp    %si,0x3(%bp,%di)
    466a:	1e                   	push   %ds
    466b:	89 f6                	mov    %si,%si
    466d:	c6                   	(bad)
    466e:	11 ec                	adc    %bp,%sp
    4670:	39 03                	cmp    %ax,(%bp,%di)
    4672:	89 46 f6             	mov    %ax,-0xa(%bp)
    4675:	01 bf 0b 8f          	add    %di,-0x70f5(%bx)
    4679:	e8 20 f6             	call   0x3c9c
    467c:	01 20                	add    %sp,(%bx,%si)
    467e:	1e                   	push   %ds
    467f:	19 03                	sbb    %ax,(%bp,%di)
    4681:	c7                   	(bad)
    4682:	8b 76 ee             	mov    -0x12(%bp),%si
    4685:	29 84 52 d2          	sub    %ax,-0x2dae(%si)
    4689:	8b c8                	mov    %ax,%cx
    468b:	03 fe                	add    %si,%di
    468d:	85 36 e8 ff          	test   %si,-0x18
    4691:	06                   	push   %es
    4692:	8b c1                	mov    %cx,%ax
    4694:	52                   	push   %dx
    4695:	51                   	push   %cx
    4696:	8a 9c 7a 52          	mov    0x527a(%si),%bl
    469a:	bf 52 2b             	mov    $0x2b52,%di
    469d:	f6 56 53             	notb   0x53(%bp)
    46a0:	ff                   	(bad)
    46a1:	fa                   	cli
    46a2:	c7                   	(bad)
    46a3:	08 d7                	or     %dl,%bh
    46a5:	7e e8                	jle    0x468f
    46a7:	89 56 ea             	mov    %dx,-0x16(%bp)
    46aa:	e8 08 89             	call   0xcfb5
    46ad:	04 8a                	add    $0x8a,%al
    46af:	f4                   	hlt
    46b0:	8b c8                	mov    %ax,%cx
    46b2:	8a 23                	mov    (%bp,%di),%ah
    46b4:	b1 89                	mov    $0x89,%cl
    46b6:	e9 13 53             	jmp    0x99cc
    46b9:	fa                   	cli
    46ba:	f7 66 fc             	mulw   -0x4(%bp)
    46bd:	c1 c2 13             	rol    $0x13,%dx
    46c0:	d7                   	xlat   %ds:(%bx)
    46c1:	0b e8                	or     %ax,%bp
    46c3:	6a 82                	push   $0xff82
    46c5:	59                   	pop    %cx
    46c6:	04 8a                	add    $0x8a,%al
    46c8:	f4                   	hlt
    46c9:	11 23                	adc    %sp,(%bp,%di)
    46cb:	8e 1d                	mov    (%di),%ds
    46cd:	06                   	push   %es
    46ce:	20 e8                	and    %ch,%al
    46d0:	36 e8 ff 06          	ss call 0x4dd3
    46d4:	ff 76 ea             	push   -0x16(%bp)
    46d7:	ff 76 e8             	push   -0x18(%bp)
    46da:	03 cf                	add    %di,%cx
    46dc:	fe 0b                	decb   (%bp,%di)
    46de:	e8 6a 22             	call   0x694b
    46e1:	56                   	push   %si
    46e2:	0b e8                	or     %ax,%bp
    46e4:	6a 61                	push   $0x61
    46e6:	59                   	pop    %cx
    46e7:	29 ff                	sub    %di,%di
    46e9:	54                   	push   %sp
    46ea:	e9 cd 8b             	jmp    0xd2ba
    46ed:	30 08                	xor    %cl,(%bx,%si)
    46ef:	87 c6                	xchg   %ax,%si
    46f1:	f4                   	hlt
    46f2:	11 01                	adc    %ax,(%bx,%di)
    46f4:	02 11                	add    (%bx,%di),%dl
    46f6:	19 ff                	sbb    %di,%di
    46f8:	0d ff ff             	or     $0xffff,%ax
    46fb:	87 e7                	xchg   %sp,%di
    46fd:	fd                   	std
    46fe:	8a 87 03 b8          	mov    -0x47fd(%bx),%al
    4702:	0a e8                	or     %al,%ch
    4704:	52                   	push   %dx
    4705:	f7 08 fe 5e          	testw  $0x5efe,(%bx,%si)
    4709:	57                   	push   %di
    470a:	9b                   	fwait
    470b:	fe 89 8a 9a          	decb   -0x6576(%bx,%di)
    470f:	f9                   	stc
    4710:	06                   	push   %es
    4711:	80 49 fd 3f          	orb    $0x3f,-0x3(%bx,%di)
    4715:	d6                   	(bad)
    4716:	e8 c2 ee             	call   0x35db
    4719:	b8 e8 8b             	mov    $0x8be8,%ax
    471c:	24 50                	and    $0x50,%al
    471e:	52                   	push   %dx
    471f:	36 15 03 f0          	ss adc $0xf003,%ax
    4723:	0e                   	push   %cs
    4724:	95                   	xchg   %ax,%bp
    4725:	fd                   	std
    4726:	b9 2b 36             	mov    $0x362b,%cx
    4729:	0a 4a fa             	or     -0x6(%bp,%si),%cl
    472c:	2f                   	das
    472d:	81 f5 e8 58          	xor    $0x58e8,%bp
    4731:	d6                   	(bad)
    4732:	08 0d                	or     %cl,(%di)
    4734:	e4 0a                	in     $0xa,%al
    4736:	8c b9 1e 08          	mov    %?,0x81e(%bx,%di)
    473a:	89 04                	mov    %ax,(%si)
    473c:	7e 04                	jle    0x4742
    473e:	2b f6                	sub    %si,%si
    4740:	57                   	push   %di
    4741:	36 ff 89 0e 04       	decw   %ss:0x40e(%bx,%di)
    4746:	11 05                	adc    %ax,(%di)
    4748:	ee                   	out    %al,(%dx)
    4749:	11 24                	adc    %sp,(%si)
    474b:	e4 80                	in     $0x80,%al
    474d:	25 ff 12             	and    $0x12ff,%ax
    4750:	33 90 eb 00          	xor    0xeb(%bx,%si),%dx
    4754:	b8 00 b8             	mov    $0xb800,%ax
    4757:	ca a2 2f             	lret   $0x2fa2
    475a:	b0 06                	mov    $0x6,%al
    475c:	bf 87 e4             	mov    $0xe487,%di
    475f:	bf 19 74             	mov    $0x7419,%di
    4762:	30 20                	xor    %ah,(%bx,%si)
    4764:	1a 01                	sbb    (%bx,%di),%al
    4766:	75 07                	jne    0x476f
    4768:	b9 2b 85             	mov    $0x852b,%cx
    476b:	88 42 e6             	mov    %al,-0x1a(%bp,%si)
    476e:	b8 b8 72             	mov    $0x72b8,%ax
    4771:	c6                   	(bad)
    4772:	da 0c                	fimull (%si)
    4774:	d7                   	xlat   %ds:(%bx)
    4775:	18 72 e2             	sbb    %dh,-0x1e(%bp,%si)
    4778:	1d b8 b8             	sbb    $0xb8b8,%ax
    477b:	2c fe                	sub    $0xfe,%al
    477d:	f2 fe 80 0b 0c       	repnz incb 0xc0b(%bx,%si)
    4782:	d7                   	xlat   %ds:(%bx)
    4783:	18 73 22             	sbb    %dh,0x22(%bp,%di)
    4786:	b0 bf                	mov    $0xbf,%al
    4788:	e4 19                	in     $0x19,%al
    478a:	30 1a                	xor    %bl,(%bp,%si)
    478c:	75 81                	jne    0x470f
    478e:	c3                   	ret
    478f:	20 1a                	and    %bl,(%bp,%si)
    4791:	11 24                	adc    %sp,(%si)
    4793:	e2 80                	loop   0x4715
    4795:	3f                   	aas
    4796:	75 b0                	jne    0x4748
    4798:	16                   	push   %ss
    4799:	c6 01 4c             	movb   $0x4c,(%bx,%di)
    479c:	b9 85 42             	mov    $0x4285,%cx
    479f:	b8 72 da             	mov    $0xda72,%ax
    47a2:	d7                   	xlat   %ds:(%bx)
    47a3:	72 1d                	jb     0x47c2
    47a5:	b8 eb d8             	mov    $0xd8eb,%ax
    47a8:	56                   	push   %si
    47a9:	e8 bf bf             	call   0x76b
    47ac:	8b f0                	mov    %ax,%si
    47ae:	8a 72 da             	mov    -0x26(%bp,%si),%dh
    47b1:	e9 13 53             	jmp    0x9ac7
    47b4:	24 e4                	and    $0xe4,%al
    47b6:	11 23                	adc    %sp,(%bp,%di)
    47b8:	28 19                	sub    %bl,(%bx,%di)
    47ba:	b9 06 0e             	mov    $0xe06,%cx
    47bd:	04 11                	add    $0x11,%al
    47bf:	05 f0 c6             	add    $0xc6f0,%ax
    47c2:	84 20                	test   %ah,(%bx,%si)
    47c4:	1a 00                	sbb    (%bx,%si),%al
    47c6:	ff b4 1c 1a          	push   0x1a1c(%si)
    47ca:	9c                   	pushf
    47cb:	8a e8                	mov    %al,%ch
    47cd:	20 8c 61 56          	and    %cl,0x5661(%si)
    47d1:	5b                   	pop    %bx
    47d2:	9c                   	pushf
    47d3:	8a 89 1f 8a          	mov    -0x75e1(%bx,%di),%cl
    47d7:	7d b8                	jge    0x4791
    47d9:	62 b8 b8 14          	bound  %di,0x14b8(%bx,%si)
    47dd:	56                   	push   %si
    47de:	5b                   	pop    %bx
    47df:	9c                   	pushf
    47e0:	53                   	push   %bx
    47e1:	24 e4                	and    $0xe4,%al
    47e3:	8a 2a                	mov    (%bp,%si),%ch
    47e5:	89 98 5c 50          	mov    %bx,0x505c(%bx,%si)
    47e9:	46                   	inc    %si
    47ea:	01 4b 1f             	add    %cx,0x1f(%bp,%di)
    47ed:	7d 62                	jge    0x4851
    47ef:	b8 56 9c             	mov    $0x9c56,%ax
    47f2:	24 fd                	and    $0xfd,%al
    47f4:	55                   	push   %bp
    47f5:	5b                   	pop    %bx
    47f6:	9c                   	pushf
    47f7:	53                   	push   %bx
    47f8:	46                   	inc    %si
    47f9:	e4 e8                	in     $0xe8,%al
    47fb:	50                   	push   %ax
    47fc:	8d b0 62 b8          	lea    -0x479e(%bx,%si),%si
    4800:	56                   	push   %si
    4801:	9c                   	pushf
    4802:	24 ed                	and    $0xed,%al
    4804:	55                   	push   %bp
    4805:	5b                   	pop    %bx
    4806:	9c                   	pushf
    4807:	8a 7d b8             	mov    -0x48(%di),%bh
    480a:	62 b8 b8 11          	bound  %di,0x11b8(%bx,%si)
    480e:	5e                   	pop    %si
    480f:	83 46 d8 5d          	addw   $0x5d,-0x28(%bp)
    4813:	8a 31                	mov    (%bx,%di),%dh
    4815:	76 89                	jbe    0x47a0
    4817:	e8 fc 47             	call   0x9016
    481a:	2b 04                	sub    (%si),%ax
    481c:	b8 02 6e             	mov    $0x6e02,%ax
    481f:	02 ee                	add    %dh,%ch
    4821:	47                   	inc    %di
    4822:	5e                   	pop    %si
    4823:	f4                   	hlt
    4824:	03 88 fe c6          	add    -0x3902(%bx,%si),%cx
    4828:	03 88 16 ff          	add    -0xea(%bx,%si),%cx
    482c:	b0 bf                	mov    $0xbf,%al
    482e:	e4 a7                	in     $0xa7,%al
    4830:	28 19                	sub    %bl,(%bx,%di)
    4832:	1a 81 20 11          	sbb    0x1120(%bx,%di),%al
    4836:	e2 3f                	loop   0x4877
    4838:	b0 c6                	mov    $0xc6,%al
    483a:	fc                   	cld
    483b:	80 3f 02             	cmpb   $0x2,(%bx)
    483e:	03 02                	add    (%bp,%si),%ax
    4840:	c6 01 4c             	movb   $0x4c,(%bx,%di)
    4843:	fe                   	(bad)
    4844:	fe                   	(bad)
    4845:	3f                   	aas
    4846:	c6 03 24             	movb   $0x24,(%bp,%di)
    4849:	ff 89 f6 06          	decw   0x6f6(%bx,%di)
    484d:	f9                   	stc
    484e:	56                   	push   %si
    484f:	d6                   	(bad)
    4850:	04 c6                	add    $0xc6,%al
    4852:	b0 16                	mov    $0x16,%al
    4854:	eb ef                	jmp    0x4845
    4856:	d6                   	(bad)
    4857:	72 3e                	jb     0x4897
    4859:	0e                   	push   %cs
    485a:	c1 1b 5f             	rcrw   $0x5f,(%bp,%di)
    485d:	ea a0 88 00 e8       	ljmp   $0xe800,$0x88a0
    4862:	80 1f ff             	sbbb   $0xff,(%bx)
    4865:	36 0e                	ss push %cs
    4867:	c1 06 49 3f 62       	rolw   $0x62,0x3f49
    486c:	b8 f3 45             	mov    $0x45f3,%ax
    486f:	a1 0e c1             	mov    0xc10e,%ax
    4872:	05 0e 42             	add    $0x420e,%ax
    4875:	66 52                	push   %edx
    4877:	e0 2b                	loopne 0x48a4
    4879:	3d 74 46             	cmp    $0x4674,%ax
    487c:	94                   	xchg   %ax,%sp
    487d:	e8 e2 45             	call   0x8e62
    4880:	a1 0e c1             	mov    0xc10e,%ax
    4883:	05 1c 08             	add    $0x81c,%ax
    4886:	a0 d1 26             	mov    0x26d1,%al
    4889:	3c 5c                	cmp    $0x5c,%al
    488b:	50                   	push   %ax
    488c:	46                   	inc    %si
    488d:	74 46                	je     0x48d5
    488f:	94                   	xchg   %ax,%sp
    4890:	e8 cf 45             	call   0x8e62
    4893:	e8 79 1f             	call   0x680f
    4896:	06                   	push   %es
    4897:	49                   	dec    %cx
    4898:	3f                   	aas
    4899:	62 b9 3f 01          	bound  %di,0x13f(%bx,%di)
    489d:	b9 22 3f             	mov    $0x3f22,%cx
    48a0:	22 21                	and    (%bx,%di),%ah
    48a2:	40                   	inc    %ax
    48a3:	ce                   	into
    48a4:	20 b8 48 36          	and    %bh,0x3648(%bx,%si)
    48a8:	03 0e 97 b8          	add    -0x4769,%cx
    48ac:	2b 57 ff             	sub    -0x1(%bx),%dx
    48af:	fe                   	(bad)
    48b0:	eb 1f                	jmp    0x48d1
    48b2:	e8 43 b8             	call   0xf8
    48b5:	a1 0e c1             	mov    0xc10e,%ax
    48b8:	e8 f7 ff             	call   0x48b2
    48bb:	2a 42 66             	sub    0x66(%bp,%si),%al
    48be:	52                   	push   %dx
    48bf:	55                   	push   %bp
    48c0:	42                   	inc    %dx
    48c1:	66 52                	push   %edx
    48c3:	ff f0                	push   %ax
    48c5:	87 e8                	xchg   %bp,%ax
    48c7:	99                   	cwtd
    48c8:	45                   	inc    %bp
    48c9:	57                   	push   %di
    48ca:	e8 22 4e             	call   0x96ef
    48cd:	f7 05 0e 47          	testw  $0x470e,(%di)
    48d1:	11 11                	adc    %dx,(%bx,%di)
    48d3:	b6 00                	mov    $0x0,%dh
    48d5:	7c db                	jl     0x48b2
    48d7:	be 0d 4e             	mov    $0x4e0d,%si
    48da:	f7 fe                	idiv   %si
    48dc:	1c 7d                	sbb    $0x7d,%al
    48de:	0a e8                	or     %al,%ch
    48e0:	16                   	push   %ss
    48e1:	b8 56 e8             	mov    $0xe856,%ax
    48e4:	09 00                	or     %ax,(%bx,%si)
    48e6:	46                   	inc    %si
    48e7:	eb f1                	jmp    0x48da
    48e9:	57                   	push   %di
    48ea:	06                   	push   %es
    48eb:	f9                   	stc
    48ec:	56                   	push   %si
    48ed:	03 08                	add    (%bx,%si),%cx
    48ef:	e4 8c                	in     $0x8c,%al
    48f1:	1e                   	push   %ds
    48f2:	89 87 03 16          	mov    %ax,0x1603(%bx)
    48f6:	3e 3e 3c 00          	ds ds cmp $0x0,%al
    48fa:	74 40                	je     0x493c
    48fc:	e8 e5 1e             	call   0x67e4
    48ff:	a1 c1 0e             	mov    0xec1,%ax
    4902:	66 e0 3d             	data32 loopne 0x4942
    4905:	46                   	inc    %si
    4906:	e8 45 0e             	call   0x574e
    4909:	05 08 16             	add    $0x1608,%ax
    490c:	fd                   	std
    490d:	e1 52                	loope  0x4961
    490f:	45                   	inc    %bp
    4910:	0e                   	push   %cs
    4911:	05 08 d1             	add    $0xd108,%ax
    4914:	3c 50                	cmp    $0x50,%al
    4916:	74 89                	je     0x48a1
    4918:	87 03                	xchg   %ax,(%bp,%di)
    491a:	eb 5e                	jmp    0x497a
    491c:	26 5c                	es pop %sp
    491e:	46                   	inc    %si
    491f:	46                   	inc    %si
    4920:	e8 45 79             	call   0xc268
    4923:	06                   	push   %es
    4924:	3f                   	aas
    4925:	3b 45 e8             	cmp    -0x18(%di),%ax
    4928:	e5 1e                	in     $0x1e,%ax
    492a:	52                   	push   %dx
    492b:	2b 74 94             	sub    -0x6c(%si),%si
    492e:	e2 a1                	loop   0x48d1
    4930:	c1 b8 3f 01 c1       	sarw   $0xc1,0x13f(%bx,%si)
    4935:	b8 3f f0             	mov    $0xf03f,%ax
    4938:	87 e8                	xchg   %bp,%ax
    493a:	37                   	aaa
    493b:	20 e8                	and    %ch,%al
    493d:	6f                   	outsw  %ds:(%si),(%dx)
    493e:	b8 3d e0             	mov    $0xe03d,%ax
    4941:	3d 72 f8             	cmp    $0xf872,%ax
    4944:	83 46 d8 5d          	addw   $0x5d,-0x28(%bp)
    4948:	8a 31                	mov    (%bx,%di),%dh
    494a:	76 89                	jbe    0x48d5
    494c:	e8 fc 47             	call   0x914b
    494f:	2b fa                	sub    %dx,%di
    4951:	f8                   	clc
    4952:	6e                   	outsb  %ds:(%si),(%dx)
    4953:	f8                   	clc
    4954:	9c                   	pushf
    4955:	fd                   	std
    4956:	5b                   	pop    %bx
    4957:	d3 83 3e 54          	rolw   %cl,0x543e(%bp,%di)
    495b:	8a 31                	mov    (%bx,%di),%dh
    495d:	75 20                	jne    0x497f
    495f:	b8 95 a1             	mov    $0xa195,%ax
    4962:	ff a6 e8 e8          	jmp    *-0x1718(%bp)
    4966:	ff                   	ljmp   (bad)
    4967:	e8 11 c6             	call   0xf7b
    496a:	03 1a                	add    (%bp,%si),%bx
    496c:	00 11                	add    %dl,(%bx,%di)
    496e:	6e                   	outsb  %ds:(%si),(%dx)
    496f:	f8                   	clc
    4970:	9c                   	pushf
    4971:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    4972:	e8 e8 ff             	call   0x495d
    4975:	e8 30 ca             	call   0x13a8
    4978:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    4979:	e8 e8 ff             	call   0x4964
    497c:	e8 2a c8             	call   0x11a9
    497f:	6e                   	outsb  %ds:(%si),(%dx)
    4980:	f8                   	clc
    4981:	9c                   	pushf
    4982:	b9 37 01             	mov    $0x137,%cx
    4985:	b9 e8 5b             	mov    $0x5be8,%cx
    4988:	bd e2 a1             	mov    $0xa1e2,%bp
    498b:	c1 b8 b9 f0 87       	sarw   $0x87,-0xf47(%bx,%si)
    4990:	e8 76 bd             	call   0x709
    4993:	80 04 0c             	addb   $0xc,(%si)
    4996:	08 0c                	or     %cl,(%si)
    4998:	14 b8                	adc    $0xb8,%al
    499a:	00 16 9a 0d          	add    %dl,0xd9a
    499e:	b8 ba 63             	mov    $0x63ba,%ax
    49a1:	1a 72 c7             	sbb    -0x39(%bp,%si),%dh
    49a4:	98                   	cbtw
    49a5:	4b                   	dec    %bx
    49a6:	14 81                	adc    $0x81,%al
    49a8:	14 72                	adc    $0x72,%al
    49aa:	5e                   	pop    %si
    49ab:	bd bc e8             	mov    $0xe8bc,%bp
    49ae:	34 1e                	xor    $0x1e,%al
    49b0:	52                   	push   %dx
    49b1:	d0 a1 c1 b8          	shlb   $1,-0x473f(%bx,%di)
    49b5:	fe                   	(bad)
    49b6:	56                   	push   %si
    49b7:	c1 b8 9e f8 9c       	sarw   $0x9c,-0x762(%bx,%si)
    49bc:	fd                   	std
    49bd:	d1 3d                	sarw   $1,(%di)
    49bf:	e8 0a d9             	call   0x22cc
    49c2:	03 1a                	add    (%bp,%si),%bx
    49c4:	00 28                	add    %ch,(%bx,%si)
    49c6:	be 52 3c             	mov    $0x3c52,%si
    49c9:	bf 52 75             	mov    $0x7552,%di
    49cc:	eb 17                	jmp    0x49e5
    49ce:	80 3d 00             	cmpb   $0x0,(%di)
    49d1:	74 0c                	je     0x49df
    49d3:	ff                   	(bad)
    49d4:	7b 0b                	jnp    0x49e1
    49d6:	ea 76 b8 09 00       	ljmp   $0x9,$0xb876
    49db:	9c                   	pushf
    49dc:	fd                   	std
    49dd:	0b d9                	or     %cx,%bx
    49df:	83 c6 04             	add    $0x4,%si
    49e2:	83 c7 20             	add    $0x20,%di
    49e5:	81 ff c5 55          	cmp    $0x55c5,%di
    49e9:	76 e3                	jbe    0x49ce
    49eb:	e8 f4 d8             	call   0x22e2
    49ee:	56                   	push   %si
    49ef:	4c                   	dec    %sp
    49f0:	56                   	push   %si
    49f1:	63 e8                	arpl   %bp,%ax
    49f3:	2f                   	das
    49f4:	00 e8                	add    %ch,%al
    49f6:	ae                   	scas   %es:(%di),%al
    49f7:	00 e8                	add    %ch,%al
    49f9:	fd                   	std
    49fa:	e0 e8                	loopne 0x49e4
    49fc:	3c f8                	cmp    $0xf8,%al
    49fe:	e8 bb 09             	call   0x53bc
    4a01:	fd                   	std
    4a02:	0b 1e 06 3f          	or     0x3f06,%bx
    4a06:	b9 01 b8             	mov    $0xb801,%cx
    4a09:	3f                   	aas
    4a0a:	01 c1                	add    %ax,%cx
    4a0c:	b8 c7 69             	mov    $0x69c7,%ax
    4a0f:	07                   	pop    %es
    4a10:	39 13                	cmp    %dx,(%bp,%di)
    4a12:	1f                   	pop    %ds
    4a13:	b0 01                	mov    $0x1,%al
    4a15:	c1 e8 10             	shr    $0x10,%ax
    4a18:	13 e8                	adc    %ax,%bp
    4a1a:	6e                   	outsb  %ds:(%si),(%dx)
    4a1b:	d3 5e e8             	rcrw   %cl,-0x18(%bp)
    4a1e:	83 e4 46             	and    $0x46,%sp
    4a21:	03 d8                	add    %ax,%bx
    4a23:	56                   	push   %si
    4a24:	5d                   	pop    %bp
    4a25:	03 8a 1e 31          	add    0x311e(%bp,%si),%cx
    4a29:	dd fa                	(bad)
    4a2b:	f8                   	clc
    4a2c:	a1 4e 3c             	mov    0x3c4e,%ax
    4a2f:	03 06 4a 3c          	add    0x3c4a,%ax
    4a33:	83 04 ba             	addw   $0xffba,(%si)
    4a36:	51                   	push   %cx
    4a37:	00 57 f0             	add    %dl,-0x10(%bx)
    4a3a:	0e                   	push   %cs
    4a3b:	8a ff                	mov    %bh,%bh
    4a3d:	75 b8                	jne    0x49f7
    4a3f:	2b be 06 b8          	sub    -0x47fa(%bp),%di
    4a43:	13 01                	adc    (%bx,%di),%ax
    4a45:	2b 46 04             	sub    0x4(%bp),%ax
    4a48:	ee                   	out    %al,(%dx)
    4a49:	18 99 33 c2          	sbb    %bl,-0x3dcd(%bx,%di)
    4a4d:	04 ba                	add    $0xba,%al
    4a4f:	69 f6 f1 b8          	imul   $0xb8f1,%si,%si
    4a53:	87 55 06             	xchg   %dx,0x6(%di)
    4a56:	20 28                	and    %ch,(%bx,%si)
    4a58:	33 fe                	xor    %si,%di
    4a5a:	19 be 3c 0b          	sbb    %di,0xb3c(%bp)
    4a5e:	ff                   	(bad)
    4a5f:	7f 33                	jg     0x4a94
    4a61:	fc                   	cld
    4a62:	10 fe                	adc    %bh,%dh
    4a64:	c2 3c 03             	ret    $0x33c
    4a67:	31 5b 8a             	xor    %bx,-0x76(%bp,%di)
    4a6a:	b8 b8 11             	mov    $0x11b8,%ax
    4a6d:	83 82 53 5b 40       	addw   $0x40,0x5b53(%bp,%si)
    4a72:	be 6d 4a             	mov    $0x4a6d,%si
    4a75:	3c 56                	cmp    $0x56,%al
    4a77:	4c                   	dec    %sp
    4a78:	56                   	push   %si
    4a79:	3c 56                	cmp    $0x56,%al
    4a7b:	4c                   	dec    %sp
    4a7c:	4e                   	dec    %si
    4a7d:	3c 56                	cmp    $0x56,%al
    4a7f:	50                   	push   %ax
    4a80:	56                   	push   %si
    4a81:	3c 56                	cmp    $0x56,%al
    4a83:	50                   	push   %ax
    4a84:	46                   	inc    %si
    4a85:	3e ee                	ds out %al,(%dx)
    4a87:	ab                   	stos   %ax,%es:(%di)
    4a88:	e6 00                	out    %al,$0x0
    4a8a:	8d 66 8d             	lea    -0x73(%bp),%sp
    4a8d:	e2 0f                	loop   0x4a9e
    4a8f:	8a b8 b8 11          	mov    0x11b8(%bx,%si),%bh
    4a93:	f8                   	clc
    4a94:	a1 56 3c             	mov    0x3c56,%ax
    4a97:	05 8b fb             	add    $0xfb8b,%ax
    4a9a:	8b 47 46             	mov    0x46(%bx),%ax
    4a9d:	41                   	inc    %cx
    4a9e:	5e                   	pop    %si
    4a9f:	83 46 d8 5d          	addw   $0x5d,-0x28(%bp)
    4aa3:	8a 31                	mov    (%bx,%di),%dh
    4aa5:	fa                   	cli
    4aa6:	56                   	push   %si
    4aa7:	50                   	push   %ax
    4aa8:	42                   	inc    %dx
    4aa9:	3c 56                	cmp    $0x56,%al
    4aab:	50                   	push   %ax
    4aac:	44                   	inc    %sp
    4aad:	3c 56                	cmp    $0x56,%al
    4aaf:	50                   	push   %ax
    4ab0:	46                   	inc    %si
    4ab1:	3c 56                	cmp    $0x56,%al
    4ab3:	50                   	push   %ax
    4ab4:	48                   	dec    %ax
    4ab5:	3c 50                	cmp    $0x50,%al
    4ab7:	3e ab                	ds stos %ax,%es:(%di)
    4ab9:	00 66 77             	add    %ah,0x77(%bp)
    4abc:	e6 8d                	out    %al,$0x8d
    4abe:	8d 0f                	lea    (%bx),%cx
    4ac0:	b8 11 a1             	mov    $0xa111,%ax
    4ac3:	3c c6                	cmp    $0xc6,%al
    4ac5:	3c 50                	cmp    $0x50,%al
    4ac7:	a1 42 3c             	mov    0x3c42,%ax
    4aca:	a1 42 c2             	mov    0xc242,%ax
    4acd:	66 a1 44 3c          	mov    0x3c44,%eax
    4ad1:	05 8b fb             	add    $0xfb8b,%ax
    4ad4:	8b 47 c4             	mov    -0x3c(%bx),%ax
    4ad7:	3e 56                	ds push %si
    4ad9:	3e c6 00 57          	movb   $0x57,%ds:(%bx,%si)
    4add:	b1 a1                	mov    $0xa1,%cl
    4adf:	dd fa                	(bad)
    4ae1:	f8                   	clc
    4ae2:	ff                   	(bad)
    4ae3:	ff f2                	push   %dx
    4ae5:	ff 73 2b             	push   0x2b(%bp,%di)
    4ae8:	ff                   	(bad)
    4ae9:	be 1a c1             	mov    $0xc11a,%si
    4aec:	81 fe b4 c1          	cmp    $0xc1b4,%si
    4af0:	73 42                	jae    0x4b34
    4af2:	83 3c 74             	cmpw   $0x74,(%si)
    4af5:	e8 32 8e             	call   0xd92a
    4af8:	1d 24 38             	sbb    $0x3824,%ax
    4afb:	44                   	inc    %sp
    4afc:	f9                   	stc
    4afd:	75 2a                	jne    0x4b29
    4aff:	57                   	push   %di
    4b00:	46                   	inc    %si
    4b01:	e8 79 3f             	call   0x8a7d
    4b04:	45                   	inc    %bp
    4b05:	e5 b8                	in     $0xb8,%ax
    4b07:	16                   	push   %ss
    4b08:	0d ba 1a             	or     $0x1aba,%ax
    4b0b:	c7                   	(bad)
    4b0c:	4b                   	dec    %bx
    4b0d:	eb 00                	jmp    0x4b0f
    4b0f:	eb 8b                	jmp    0x4a9c
    4b11:	e8 98 00             	call   0x4bac
    4b14:	e8 0a d3             	call   0x1e21
    4b17:	57                   	push   %di
    4b18:	e8 04 08             	call   0x531f
    4b1b:	8e a0 e4 03          	mov    0x3e4(%bx,%si),%fs
    4b1f:	b0 06                	mov    $0x6,%al
    4b21:	1d 02 9d             	sbb    $0x9d02,%ax
    4b24:	65 a8 02             	gs test $0x2,%al
    4b27:	06                   	push   %es
    4b28:	83 1a eb             	sbbw   $0xffeb,(%bp,%si)
    4b2b:	0e                   	push   %cs
    4b2c:	47                   	inc    %di
    4b2d:	eb bd                	jmp    0x4aec
    4b2f:	32 8c b9 7e          	xor    0x7eb9(%si),%cl
    4b33:	eb b8                	jmp    0x4aed
    4b35:	fe 0c                	decb   (%si)
    4b37:	be e8 7e             	mov    $0x7ee8,%si
    4b3a:	46                   	inc    %si
    4b3b:	c7                   	(bad)
    4b3c:	66 5e                	pop    %esi
    4b3e:	8b 08                	mov    (%bx,%si),%cx
    4b40:	06                   	push   %es
    4b41:	19 8b 1e b0          	sbb    %cx,-0x4fe2(%bp,%di)
    4b45:	bf 16 ff             	mov    $0xff16,%di
    4b48:	f2 73 ff             	bnd jae 0x4b4a
    4b4b:	1a 81 b4 73          	sbb    0x73b4(%bx,%di),%al
    4b4f:	83 74 eb 48          	xorw   $0x48,-0x15(%si)
    4b53:	83 3c 74             	cmpw   $0x74,(%si)
    4b56:	e8 3f 8a             	call   0xd598
    4b59:	44                   	inc    %sp
    4b5a:	f9                   	stc
    4b5b:	2b 4a 01             	sub    0x1(%bp,%si),%cx
    4b5e:	1d 24 75             	sbb    $0x7524,%ax
    4b61:	35 57 50             	xor    $0x5057,%ax
    4b64:	b8 6e ee             	mov    $0xee6e,%ax
    4b67:	5e                   	pop    %si
    4b68:	03 fe                	add    %si,%di
    4b6a:	03 16 8a 89          	add    -0x7676,%dx
    4b6e:	5c                   	pop    %sp
    4b6f:	46                   	inc    %si
    4b70:	4b                   	dec    %bx
    4b71:	7d b8                	jge    0x4b2b
    4b73:	81 18 53 c0          	sbbw   $0xc053,(%bx,%si)
    4b77:	5b                   	pop    %bx
    4b78:	04 31                	add    $0x31,%al
    4b7a:	00 e8                	add    %ch,%al
    4b7c:	a3 d2 56             	mov    %ax,0x56d2
    4b7f:	ff                   	(bad)
    4b80:	b9 9a 4c             	mov    $0x4c9a,%cx
    4b83:	04 57                	add    $0x57,%al
    4b85:	e8 e6 64             	call   0xb06e
    4b88:	03 00                	add    (%bx,%si),%ax
    4b8a:	be 06 c7             	mov    $0xc706,%si
    4b8d:	5e                   	pop    %si
    4b8e:	8b fe                	mov    %si,%di
    4b90:	05 d1 32             	add    $0x32d1,%ax
    4b93:	8c b9 7e eb          	mov    %?,-0x1482(%bx,%di)
    4b97:	f7 0e 11 b6 7c be    	testw  $0xbe7c,-0x49ef
    4b9d:	b4 c1                	mov    $0xc1,%ah
    4b9f:	72 b2                	jb     0x4b53
    4ba1:	b8 0c e8             	mov    $0xe80c,%ax
    4ba4:	46                   	inc    %si
    4ba5:	66 8b 06 8b b0       	mov    -0x4f75,%eax
    4baa:	16                   	push   %ss
    4bab:	f2 ff 81 73 74       	repnz incw 0x7473(%bx,%di)
    4bb0:	48                   	dec    %ax
    4bb1:	3c 56                	cmp    $0x56,%al
    4bb3:	b8 0e ee             	mov    $0xee0e,%ax
    4bb6:	5e                   	pop    %si
    4bb7:	03 0a                	add    (%bp,%si),%cx
    4bb9:	02 06 fa 9f          	add    -0x6006,%al
    4bbd:	14 c1                	adc    $0xc1,%al
    4bbf:	86 8b b2 03          	xchg   %cl,0x3b2(%bp,%di)
    4bc3:	22 52 a3             	and    -0x5d(%bp,%si),%dl
    4bc6:	7c 3d                	jl     0x4c05
    4bc8:	8b 5e 08             	mov    0x8(%bp),%bx
    4bcb:	86 b2 22 a3          	xchg   %dh,-0x5cde(%bp,%si)
    4bcf:	3d 5e 86             	cmp    $0x865e,%ax
    4bd2:	80 3d 48             	cmpb   $0x48,(%di)
    4bd5:	ff c8                	dec    %ax
    4bd7:	2b 5e 1b             	sub    0x1b(%bp),%bx
    4bda:	06                   	push   %es
    4bdb:	c0 fe 3b             	sar    $0x3b,%dh
    4bde:	86 3d                	xchg   %bh,(%di)
    4be0:	83 7e 08 1b          	cmpw   $0x1b,0x8(%bp)
    4be4:	72 05                	jb     0x4beb
    4be6:	a1 8e 3d             	mov    0x3d8e,%ax
    4be9:	eb 03                	jmp    0x4bee
    4beb:	a1 8c 3d             	mov    0x3d8c,%ax
    4bee:	a3 8a 3d             	mov    %ax,0x3d8a
    4bf1:	48                   	dec    %ax
    4bf2:	36 7a 3d             	ss jp  0x4c32
    4bf5:	8a b8 83 53          	mov    0x5383(%bx,%si),%bh
    4bf9:	40                   	inc    %ax
    4bfa:	36 52                	ss push %dx
    4bfc:	5b                   	pop    %bx
    4bfd:	5b                   	pop    %bx
    4bfe:	be 7c 3d             	mov    $0x3d7c,%si
    4c01:	eb 0d                	jmp    0x4c10
    4c03:	ff 34                	push   (%si)
    4c05:	8a b8 83 53          	mov    0x5383(%bx,%si),%bh
    4c09:	40                   	inc    %ax
    4c0a:	e6 51                	out    %al,$0x51
    4c0c:	5b                   	pop    %bx
    4c0d:	5b                   	pop    %bx
    4c0e:	46                   	inc    %si
    4c0f:	46                   	inc    %si
    4c10:	7c be                	jl     0x4bd0
    4c12:	8c 3d                	mov    %?,(%di)
    4c14:	72 ed                	jb     0x4c03
    4c16:	8a b8 83 53          	mov    0x5383(%bx,%si),%bh
    4c1a:	40                   	inc    %ax
    4c1b:	b7 0d                	mov    $0xd,%bh
    4c1d:	46                   	inc    %si
    4c1e:	05 95 52             	add    $0x5295,%ax
    4c21:	b0 ec                	mov    $0xec,%al
    4c23:	73 f8                	jae    0x4c1d
    4c25:	e3 fa                	jcxz   0x4c21
    4c27:	32 b9 89 08          	xor    0x889(%bx,%di),%bh
    4c2b:	76 83                	jbe    0x4bb0
    4c2d:	3e 4c                	ds dec %sp
    4c2f:	63 05                	arpl   %ax,(%di)
    4c31:	73 2b                	jae    0x4c5e
    4c33:	ff                   	(bad)
    4c34:	79 01                	jns    0x4c37
    4c36:	81 73 74 48 3c       	xorw   $0x3c48,0x74(%bp,%di)
    4c3b:	e8 0d 83             	call   0xcf4b
    4c3e:	3c 74                	cmp    $0x74,%al
    4c40:	e8 04 57             	call   0xa347
    4c43:	e8 e1 02             	call   0x4f27
    4c46:	f7 11                	notw   (%bx,%di)
    4c48:	7c b4                	jl     0x4bfe
    4c4a:	72 b8                	jb     0x4c04
    4c4c:	e8 66 06             	call   0x52b5
    4c4f:	ed                   	in     (%dx),%ax
    4c50:	a0 9c 3d             	mov    0x3d9c,%al
    4c53:	4b                   	dec    %bx
    4c54:	7d b8                	jge    0x4c0e
    4c56:	0e                   	push   %cs
    4c57:	ff                   	(bad)
    4c58:	b8 be b8             	mov    $0xb8be,%ax
    4c5b:	32 f0                	xor    %al,%dh
    4c5d:	e8 45 e8             	call   0x34a5
    4c60:	b8 a2 9c             	mov    $0x9ca2,%ax
    4c63:	3d b8 64             	cmp    $0x64b8,%ax
    4c66:	f0 e8 45 00          	lock call 0x4caf
    4c6a:	bb 3a 06             	mov    $0x63a,%bx
    4c6d:	9c                   	pushf
    4c6e:	3d 73 0a             	cmp    $0xa73,%ax
    4c71:	e8 41 01             	call   0x4db5
    4c74:	c6 06 9c 3d 0a       	movb   $0xa,0x3d9c
    4c79:	eb 33                	jmp    0x4cae
    4c7b:	81 73 bb 52 eb       	xorw   $0xeb52,-0x45(%bp,%di)
    4c80:	80 09 80             	orb    $0x80,(%bx,%di)
    4c83:	75 16                	jne    0x4c9b
    4c85:	01 01                	add    %ax,(%bx,%di)
    4c87:	47                   	inc    %di
    4c88:	83 c3 20             	add    $0x20,%bx
    4c8b:	81 fb e5 55          	cmp    $0x55e5,%bx
    4c8f:	72 f1                	jb     0x4c82
    4c91:	b8 64 f0             	mov    $0xf064,%ax
    4c94:	e8 45 d3             	call   0x1fdc
    4c97:	ba 8b c8             	mov    $0xc88b,%dx
    4c9a:	8b c7                	mov    %di,%ax
    4c9c:	bb 05 00             	mov    $0x5,%bx
    4c9f:	2b f8                	sub    %ax,%di
    4ca1:	fe                   	(bad)
    4ca2:	f3 02 06 38 8f       	repz add -0x70c8,%al
    4ca7:	52                   	push   %dx
    4ca8:	3d 76 03             	cmp    $0x376,%ax
    4cab:	e8 7f 01             	call   0x4e2d
    4cae:	bb 14 c1             	mov    $0xc114,%bx
    4cb1:	eb 39                	jmp    0x4cec
    4cb3:	83 7f 06 16          	cmpw   $0x16,0x6(%bx)
    4cb7:	01 30                	add    %si,(%bx,%si)
    4cb9:	8b 3f                	mov    (%bx),%di
    4cbb:	b1 05                	mov    $0x5,%cl
    4cbd:	8b f7                	mov    %di,%si
    4cbf:	d3 e6                	shl    %cl,%si
    4cc1:	8b 84 6c 52          	mov    0x526c(%si),%ax
    4cc5:	8b 57 04             	mov    0x4(%bx),%dx
    4cc8:	83 c2 64             	add    $0x64,%dx
    4ccb:	3b c2                	cmp    %dx,%ax
    4ccd:	76 8b                	jbe    0x4c5a
    4ccf:	3b c2                	cmp    %dx,%ax
    4cd1:	89 47 04             	mov    %ax,0x4(%bx)
    4cd4:	d3 e7                	shl    %cl,%di
    4cd6:	8b 85 6e 52          	mov    0x526e(%di),%ax
    4cda:	8b 4f 02             	mov    0x2(%bx),%cx
    4cdd:	f7 c1 14 3b          	test   $0x3b14,%cx
    4ce1:	c1 76 8b 3b          	shlw   $0x3b,-0x75(%bp)
    4ce5:	c1 89 47 02 f7       	rorw   $0xf7,0x247(%bx,%di)
    4cea:	c3                   	ret
    4ceb:	0e                   	push   %cs
    4cec:	81 fb ae 66          	cmp    $0x66ae,%bx
    4cf0:	06                   	push   %es
    4cf1:	c1 32 b9             	shlw   $0xb9,(%bp,%si)
    4cf4:	eb 03                	jmp    0x4cf9
    4cf6:	00 f8                	add    %bh,%al
    4cf8:	c7                   	(bad)
    4cf9:	e3 3a                	jcxz   0x4d35
    4cfb:	11 f8                	adc    %di,%ax
    4cfd:	c7                   	(bad)
    4cfe:	f4                   	hlt
    4cff:	96                   	xchg   %ax,%si
    4d00:	3d be 12             	cmp    $0x12be,%ax
    4d03:	00 f8                	add    %bh,%al
    4d05:	c7                   	(bad)
    4d06:	f2 b2 c1             	repnz mov $0xc1,%dl
    4d09:	32 b9 f6 90          	xor    -0x6f0a(%bx,%di),%bh
    4d0d:	3d 81 7e             	cmp    $0x7e81,%ax
    4d10:	f6 96 3d 8c          	notb   -0x73c3(%bp)
    4d14:	9a 63 97 ff 87       	lcall  $0x87ff,$0x9763
    4d19:	b8 0a 80             	mov    $0x800a,%ax
    4d1c:	ff 07                	incw   (%bx)
    4d1e:	47                   	inc    %di
    4d1f:	76 f7                	jbe    0x4d18
    4d21:	4a                   	dec    %dx
    4d22:	18 50 08             	sbb    %dl,0x8(%bx,%si)
    4d25:	00 8b fe 76          	add    %cl,0x76fe(%bp,%di)
    4d29:	8b 5e f6             	mov    -0xa(%bp),%bx
    4d2c:	8b 3f                	mov    (%bx),%di
    4d2e:	8b 5e f2             	mov    -0xe(%bp),%bx
    4d31:	8b 07                	mov    (%bx),%ax
    4d33:	05 64 f0             	add    $0xf064,%ax
    4d36:	3d 40 1f             	cmp    $0x1f40,%ax
    4d39:	76 03                	jbe    0x4d3e
    4d3b:	b8 40 1f             	mov    $0x1f40,%ax
    4d3e:	eb 8c                	jmp    0x4ccc
    4d40:	12 04                	adc    (%si),%al
    4d42:	f4                   	hlt
    4d43:	c8 23 07 8b          	enter  $0x723,$0x8b
    4d47:	df 03                	filds  (%bp,%di)
    4d49:	de d1                	(bad)
    4d4b:	e3 39                	jcxz   0x4d86
    4d4d:	0e                   	push   %cs
    4d4e:	c3                   	ret
    4d4f:	f9                   	stc
    4d50:	73 0a                	jae    0x4d5c
    4d52:	8b de                	mov    %si,%bx
    4d54:	03 df                	add    %di,%bx
    4d56:	c7                   	(bad)
    4d57:	eb eb                	jmp    0x4d44
    4d59:	0e                   	push   %cs
    4d5a:	c3                   	ret
    4d5b:	f9                   	stc
    4d5c:	8b c7                	mov    %di,%ax
    4d5e:	03 fe                	add    %si,%di
    4d60:	b9 24 87             	mov    $0x8724,%cx
    4d63:	8f                   	(bad)
    4d64:	de 2a                	fisubrs (%bp,%si)
    4d66:	f7 ed                	imul   %bp
    4d68:	d1 e7                	shl    $1,%di
    4d6a:	8b 85 c3 f9          	mov    -0x63d(%di),%ax
    4d6e:	8b d8                	mov    %ax,%bx
    4d70:	f7 e1                	mul    %cx
    4d72:	3b 8c 12 76          	cmp    0x7612(%si),%cx
    4d76:	03 04                	add    (%si),%ax
    4d78:	fe 87 76 fe          	incb   -0x18a(%bx)
    4d7c:	87 b8 ee 03          	xchg   %di,0x3ee(%bx,%si)
    4d80:	02 fa                	add    %dl,%bh
    4d82:	76 8b                	jbe    0x4d0f
    4d84:	f8                   	clc
    4d85:	04 fe                	add    $0xfe,%al
    4d87:	87 76 85             	xchg   %si,-0x7b(%bp)
    4d8a:	18 c1                	sbb    %al,%cl
    4d8c:	2b f8                	sub    %ax,%di
    4d8e:	fe                   	(bad)
    4d8f:	f3 02 b8 0a 88       	repz add -0x77f6(%bx,%si),%bh
    4d94:	47                   	inc    %di
    4d95:	02 f7                	add    %bh,%dh
    4d97:	c7                   	(bad)
    4d98:	e3 0c                	jcxz   0x4da6
    4d9a:	f7 c7 f4 02          	test   $0x2f4,%di
    4d9e:	11 b4 06 f7          	adc    %si,-0x8fa(%si)
    4da2:	c7                   	(bad)
    4da3:	f2 0e                	repnz push %cs
    4da5:	f7 c7 f6 02          	test   $0x2f6,%di
    4da9:	ff 4a 18             	decw   0x18(%bp,%si)
    4dac:	e9 5f 83             	jmp    0xd10e
    4daf:	f6 56 c6             	notb   -0x3a(%bp)
    4db2:	2e 05 89 08          	cs add $0x889,%ax
    4db6:	76 a0                	jbe    0x4d58
    4db8:	d1 e8                	shr    $1,%ax
    4dba:	0c fb                	or     $0xfb,%al
    4dbc:	f8                   	clc
    4dbd:	b1 03                	mov    $0x3,%cl
    4dbf:	a1 63 06             	mov    0x663,%ax
    4dc2:	2d 05 00             	sub    $0x5,%ax
    4dc5:	d3 e8                	shr    %cl,%ax
    4dc7:	3d 04 00             	cmp    $0x4,%ax
    4dca:	76 03                	jbe    0x4dcf
    4dcc:	b8 04 00             	mov    $0x4,%ax
    4dcf:	3b 03                	cmp    (%bp,%di),%ax
    4dd1:	f2 b8 f0 45          	repnz mov $0x45f0,%ax
    4dd5:	ba c8 92             	mov    $0x92c8,%dx
    4dd8:	b9 76 fe             	mov    $0xfe76,%cx
    4ddb:	87 2b                	xchg   %bp,(%bp,%di)
    4ddd:	ff                   	ljmp   (bad)
    4dde:	eb e8                	jmp    0x4dc8
    4de0:	b8 16 f9             	mov    $0xf916,%ax
    4de3:	ff 00                	incw   (%bx,%si)
    4de5:	02 fa                	add    %dl,%bh
    4de7:	fe                   	(bad)
    4de8:	b9 24 87             	mov    $0x8724,%cx
    4deb:	80 2e 3d 7d 0e       	subb   $0xe,0x7d3d
    4df0:	03 f8                	add    %ax,%di
    4df2:	3b 7e fc             	cmp    -0x4(%bp),%di
    4df5:	77 06                	ja     0x4dfd
    4df7:	b8 72 7c             	mov    $0x7c72,%ax
    4dfa:	07                   	pop    %es
    4dfb:	72 e5                	jb     0x4de2
    4dfd:	8b c6                	mov    %si,%ax
    4dff:	b9 ee 03             	mov    $0x3ee,%cx
    4e02:	02 11                	add    (%bx,%di),%dl
    4e04:	ee                   	out    %al,(%dx)
    4e05:	24 83                	and    $0x83,%al
    4e07:	bf 1a c1             	mov    $0xc11a,%di
    4e0a:	06                   	push   %es
    4e0b:	13 5e e8             	adc    -0x18(%bp),%bx
    4e0e:	1d 00 eb             	sbb    $0xeb00,%ax
    4e11:	15 c6 87             	adc    $0x87c6,%ax
    4e14:	12 c1                	adc    %cl,%al
    4e16:	06                   	push   %es
    4e17:	56                   	push   %si
    4e18:	8a 84 58 3d          	mov    0x3d58(%si),%al
    4e1c:	7d 0e                	jge    0x4e2c
    4e1e:	b8 8a 84             	mov    $0x848a,%ax
    4e21:	60                   	pusha
    4e22:	3d ba c8             	cmp    $0xc8ba,%ax
    4e25:	9a 00 f6 c6 05       	lcall  $0x5c6,$0xf600
    4e2a:	08 a0 e8 fb          	or     %ah,-0x418(%bx,%si)
    4e2e:	b1 a1                	mov    $0xa1,%cl
    4e30:	06                   	push   %es
    4e31:	05 d3 3d             	add    $0x3dd3,%ax
    4e34:	00 2b                	add    %ch,(%bp,%di)
    4e36:	c9                   	leave
    4e37:	ba ff 00             	mov    $0xff,%dx
    4e3a:	bb 7c c1             	mov    $0xc17c,%bx
    4e3d:	eb 09                	jmp    0x4e48
    4e3f:	83 75 01 47          	xorw   $0x47,0x1(%di)
    4e43:	0f                   	maskmovq (bad),%mm1
    4e44:	f7 0e fb 42 81 fb    	testw  $0xfb81,0x42fb
    4e4a:	e8 06 a0             	call   0xee53
    4e4d:	f1                   	int1
    4e4e:	89 fa                	mov    %di,%dx
    4e50:	97                   	xchg   %ax,%di
    4e51:	eb 02                	jmp    0x4e55
    4e53:	8b fa                	mov    %dx,%di
    4e55:	83 ff 0b             	cmp    $0xb,%di
    4e58:	73 61                	jae    0x4ebb
    4e5a:	bb a5 52             	mov    $0x52a5,%bx
    4e5d:	eb 0b                	jmp    0x4e6a
    4e5f:	80 16 01 75 03       	adcb   $0x3,0x7501
    4e64:	b9 ff 11             	mov    $0x11ff,%cx
    4e67:	04 c3                	add    $0xc3,%al
    4e69:	81 e5 72 b8          	and    $0xb872,%bp
    4e6d:	f0 45                	lock inc %bp
    4e6f:	ef                   	out    %ax,(%dx)
    4e70:	0b c9                	or     %cx,%cx
    4e72:	74 47                	je     0x4ebb
    4e74:	b8 1a 36             	mov    $0x361a,%ax
    4e77:	03 0e fd b8          	add    -0x4703,%cx
    4e7b:	8b f0                	mov    %ax,%si
    4e7d:	46                   	inc    %si
    4e7e:	8b de                	mov    %si,%bx
    4e80:	b1 d3                	mov    $0xd3,%cl
    4e82:	c3                   	ret
    4e83:	7e 80                	jle    0x4e05
    4e85:	85 00                	test   %ax,(%bx,%si)
    4e87:	30 8a 07 e9          	xor    %cl,-0x16f9(%bp,%si)
    4e8b:	0e                   	push   %cs
    4e8c:	b8 b8 f0             	mov    $0xf0b8,%ax
    4e8f:	e8 d9 b8             	call   0x76b
    4e92:	3d 04 00             	cmp    $0x4,%ax
    4e95:	76 10                	jbe    0x4ea7
    4e97:	57                   	push   %di
    4e98:	ff 0e ee 02          	decw   0x2ee
    4e9c:	ee                   	out    %al,(%dx)
    4e9d:	83 1a 06             	sbbw   $0x6,(%bp,%si)
    4ea0:	c6 87 12 c1 06       	movb   $0x6,-0x3eee(%bx)
    4ea5:	eb 89                	jmp    0x4e30
    4ea7:	06                   	push   %es
    4ea8:	ff                   	ljmp   (bad)
    4ea9:	ee                   	out    %al,(%dx)
    4eaa:	ee                   	out    %al,(%dx)
    4eab:	1a c6                	sbb    %dh,%al
    4ead:	12 06 89 ff          	adc    -0x77,%al
    4eb1:	ee                   	out    %al,(%dx)
    4eb2:	c6 06 01 3d 00       	movb   $0x0,0x3d01
    4eb7:	56                   	push   %si
    4eb8:	e8 06 00             	call   0x4ec1
    4ebb:	c6                   	(bad)
    4ebc:	08 e8                	or     %ch,%al
    4ebe:	b1 06                	mov    $0x6,%cl
    4ec0:	d3 00                	rolw   %cl,(%bx,%si)
    4ec2:	c9                   	leave
    4ec3:	ff                   	(bad)
    4ec4:	bb c1 02             	mov    $0x2c1,%bx
    4ec7:	56                   	push   %si
    4ec8:	b1 8b                	mov    $0x8b,%cl
    4eca:	d3 5e 06             	rcrw   %cl,0x6(%bp)
    4ecd:	88 89 03 b8          	mov    %cl,-0x47fd(%bx,%di)
    4ed1:	fe                   	(bad)
    4ed2:	bf 19 00             	mov    $0x19,%di
    4ed5:	30 8a 07 47          	xor    %cl,0x4707(%bp,%si)
    4ed9:	b8 ee 02             	mov    $0x2ee,%ax
    4edc:	76 f8                	jbe    0x4ed6
    4ede:	16                   	push   %ss
    4edf:	8f                   	(bad)
    4ee0:	5e                   	pop    %si
    4ee1:	8e 1d                	mov    (%di),%ds
    4ee3:	24 fc                	and    $0xfc,%al
    4ee5:	26 13 c1             	es adc %cx,%ax
    4ee8:	06                   	push   %es
    4ee9:	fe 86 89 87          	incb   -0x7877(%bp)
    4eed:	14 c1                	adc    $0xc1,%al
    4eef:	06                   	push   %es
    4ef0:	0a 8b b0 84          	or     -0x7b50(%bp,%di),%cl
    4ef4:	52                   	push   %dx
    4ef5:	57                   	push   %di
    4ef6:	89 87 18 c1          	mov    %ax,-0x3ee8(%bx)
    4efa:	06                   	push   %es
    4efb:	84 6e 57             	test   %ch,0x57(%bp)
    4efe:	89 87 16 c1          	mov    %ax,-0x3eea(%bx)
    4f02:	8a 84 7c 52          	mov    0x527c(%si),%al
    4f06:	fe                   	(bad)
    4f07:	19 1c                	sbb    %bx,(%si)
    4f09:	c1 8a 84 7b 52       	rorw   $0x52,0x7b84(%bp,%si)
    4f0e:	fe                   	(bad)
    4f0f:	19 1d                	sbb    %bx,(%di)
    4f11:	c1 8a 84 7a 11       	rorw   $0x11,0x7a84(%bp,%si)
    4f16:	23 8e 1e c1          	and    -0x3ee2(%bp),%cx
    4f1a:	c7 87 1a c1 d2 89    	movw   $0x89d2,-0x3ee6(%bx)
    4f20:	e8 e8 76             	call   0xc60b
    4f23:	00 01                	add    %al,(%bx,%di)
    4f25:	24 02                	and    $0x2,%al
    4f27:	89 8e 0c 8e          	mov    %cx,-0x71f4(%bp)
    4f2b:	09 6c 3d             	or     %bp,0x3d(%si)
    4f2e:	00 b8 ee 02          	add    %bh,0x2ee(%bx,%si)
    4f32:	76 f8                	jbe    0x4f2c
    4f34:	47                   	inc    %di
    4f35:	f4                   	hlt
    4f36:	88 c6                	mov    %al,%dh
    4f38:	88 9a 8a 26          	mov    %bl,0x268a(%bp,%si)
    4f3c:	13 c1                	adc    %cx,%ax
    4f3e:	ff                   	(bad)
    4f3f:	7e 83                	jle    0x4ec4
    4f41:	01 f4                	add    %si,%sp
    4f43:	bf 19 c6             	mov    $0xc619,%di
    4f46:	06                   	push   %es
    4f47:	01 75 34             	add    %si,0x34(%di)
    4f4a:	2b fa                	sub    %dx,%di
    4f4c:	f8                   	clc
    4f4d:	6e                   	outsb  %ds:(%si),(%dx)
    4f4e:	f8                   	clc
    4f4f:	eb b8                	jmp    0x4f09
    4f51:	f9                   	stc
    4f52:	17                   	pop    %ss
    4f53:	b1 03                	mov    $0x3,%cl
    4f55:	04 c8                	add    $0xc8,%al
    4f57:	07                   	pop    %es
    4f58:	80 ff ec             	cmp    $0xec,%bh
    4f5b:	80 ec 80             	sub    $0x80,%ah
    4f5e:	1b 17                	sbb    (%bx),%dx
    4f60:	17                   	pop    %ss
    4f61:	c0 ff 74             	sar    $0x74,%bh
    4f64:	04 88                	add    $0x88,%al
    4f66:	43                   	inc    %bx
    4f67:	ec                   	in     (%dx),%al
    4f68:	47                   	inc    %di
    4f69:	8b 03                	mov    (%bp,%di),%ax
    4f6b:	fa                   	cli
    4f6c:	08 72 e4             	or     %dh,-0x1c(%bp,%si)
    4f6f:	57                   	push   %di
    4f70:	e8 f8 b7             	call   0x76b
    4f73:	ff c7                	inc    %di
    4f75:	d7                   	xlat   %ds:(%bx)
    4f76:	72 ec                	jb     0x4f64
    4f78:	1b 40 2a             	sbb    0x2a(%bx,%si),%ax
    4f7b:	08 eb                	or     %ch,%bl
    4f7d:	70 8b                	jo     0x4f0a
    4f7f:	d0 0b                	rorb   $1,(%bp,%di)
    4f81:	d2 74 61             	shlb   %cl,0x61(%si)
    4f84:	83 fa 1b             	cmp    $0x1b,%dx
    4f87:	73 0c                	jae    0x4f95
    4f89:	b1 8b                	mov    $0x8b,%cl
    4f8b:	d3 d8                	rcr    %cl,%ax
    4f8d:	80 ff ec             	cmp    $0xec,%bh
    4f90:	87 8a 52 eb          	xchg   %cx,-0x14ae(%bp,%si)
    4f94:	06                   	push   %es
    4f95:	8b da                	mov    %dx,%bx
    4f97:	ec                   	in     (%dx),%al
    4f98:	87 55 3d             	xchg   %dx,0x3d(%di)
    4f9b:	1b 17                	sbb    (%bx),%dx
    4f9d:	f7 2b                	imulw  (%bp,%di)
    4f9f:	eb b8                	jmp    0x4f59
    4fa1:	f9                   	stc
    4fa2:	03 fa                	add    %dx,%di
    4fa4:	08 73 c8             	or     %dh,-0x38(%bp,%di)
    4fa7:	b1 03                	mov    $0x3,%cl
    4fa9:	04 da                	add    $0xda,%al
    4fab:	80 ec ec             	sub    $0xec,%ah
    4fae:	1b 17                	sbb    (%bx),%dx
    4fb0:	ff 04                	incw   (%si)
    4fb2:	43                   	inc    %bx
    4fb3:	47                   	inc    %di
    4fb4:	03 08                	add    (%bx,%si),%cx
    4fb6:	e4 2a                	in     $0x2a,%al
    4fb8:	17                   	pop    %ss
    4fb9:	c1 16 0e 8a c7       	rclw   $0xc7,-0x75f2
    4fbe:	86 8b b1 c3          	xchg   %cl,-0x3c4f(%bp,%di)
    4fc2:	80 00 ec             	addb   $0xec,(%bx,%si)
    4fc5:	87 8a 52 eb          	xchg   %cx,-0x14ae(%bp,%si)
    4fc9:	09 8a c6 8b          	or     %cx,-0x743a(%bp,%si)
    4fcd:	31 05                	xor    %ax,(%di)
    4fcf:	53                   	push   %bx
    4fd0:	1a 55 3d             	sbb    0x3d(%di),%dl
    4fd3:	1b 17                	sbb    (%bx),%dx
    4fd5:	fb                   	sti
    4fd6:	38 17                	cmp    %dl,(%bx)
    4fd8:	f7 76 07             	divw   0x7(%bp)
    4fdb:	2b 88 e6 b8          	sub    -0x471a(%bx,%si),%cx
    4fdf:	43                   	inc    %bx
    4fe0:	ec                   	in     (%dx),%al
    4fe1:	47                   	inc    %di
    4fe2:	8b eb                	mov    %bx,%bp
    4fe4:	32 b9 87 1a          	xor    0x1a87(%bx,%di),%bh
    4fe8:	c1 7e b8 0c          	sarw   $0xc,-0x48(%bp)
    4fec:	76 f6                	jbe    0x4fe4
    4fee:	03 fa                	add    %dx,%di
    4ff0:	72 76                	jb     0x5068
    4ff2:	95                   	xchg   %ax,%bp
    4ff3:	c6                   	(bad)
    4ff4:	cf                   	iret
    4ff5:	01 89 f6 c6          	add    %cx,-0x390a(%bx,%di)
    4ff9:	1d b8 fe             	sbb    $0xfeb8,%ax
    4ffc:	21 a0 7a 11          	and    %sp,0x117a(%bx,%si)
    5000:	a2 13 1a             	mov    %al,0x1a13
    5003:	c6 06 15 1a 02       	movb   $0x2,0x1a15
    5008:	c6 06 19 1a 8b       	movb   $0x8b,0x1a19
    500d:	31 c0                	xor    %ax,%ax
    500f:	a2 12 1a             	mov    %al,0x1a12
    5012:	a2 14 1a             	mov    %al,0x1a14
    5015:	a2 16 1a             	mov    %al,0x1a16
    5018:	a2 17 1a             	mov    %al,0x1a17
    501b:	a2 1a 1a             	mov    %al,0x1a1a
    501e:	2b 33                	sub    (%bp,%di),%si
    5020:	fe                   	(bad)
    5021:	19 7e 1a             	sbb    %di,0x1a(%bp)
    5024:	8c 75 e4             	mov    %?,-0x1c(%di)
    5027:	2a 4e f8             	sub    -0x8(%bp),%cl
    502a:	d0 16 73 3e          	rclb   $1,0x3e73
    502e:	80 20 21             	andb   $0x21,(%bx,%si)
    5031:	06                   	push   %es
    5032:	2a 8a 44 f8          	sub    -0x7bc(%bp,%si),%cl
    5036:	20 ff                	and    %bh,%bh
    5038:	3b 46 b8             	cmp    -0x48(%bp),%ax
    503b:	fe                   	(bad)
    503c:	20 9a 04 e8          	and    %bl,-0x17fc(%bp,%si)
    5040:	50                   	push   %ax
    5041:	8a 44 fd             	mov    -0x3(%si),%al
    5044:	50                   	push   %ax
    5045:	8d 44 f3             	lea    -0xd(%si),%ax
    5048:	07                   	pop    %es
    5049:	13 b0 fb e8          	adc    -0x1705(%bx,%si),%si
    504d:	d2 cd                	ror    %cl,%ch
    504f:	3b ff                	cmp    %di,%di
    5051:	35 b0 9f             	xor    $0x9fb0,%ax
    5054:	2b 04                	sub    (%si),%ax
    5056:	e8 15 60             	call   0xb06e
    5059:	0b c0                	or     %ax,%ax
    505b:	75 83                	jne    0x4fe0
    505d:	eb 47                	jmp    0x50a6
    505f:	14 eb                	adc    $0xeb,%al
    5061:	57                   	push   %di
    5062:	c5 8b 5e 9a          	lds    -0x65a2(%bp,%di),%cx
    5066:	b9 1a 7e             	mov    $0x7e1a,%cx
    5069:	0c f6                	or     $0xf6,%al
    506b:	fa                   	cli
    506c:	83 7e f6 00          	cmpw   $0x0,-0xa(%bp)
    5070:	75 18                	jne    0x508a
    5072:	8b 5e 9a             	mov    -0x66(%bp),%bx
    5075:	83 1a 06             	sbbw   $0x6,(%bp,%si)
    5078:	5e                   	pop    %si
    5079:	1d eb 0e             	sbb    $0xeeb,%ax
    507c:	b1 d3                	mov    $0xd3,%cl
    507e:	06                   	push   %es
    507f:	89 07                	mov    %ax,(%bx)
    5081:	e3 37                	jcxz   0x50ba
    5083:	bf 00 8a             	mov    $0x8a00,%di
    5086:	47                   	inc    %di
    5087:	ee                   	out    %al,(%dx)
    5088:	76 95                	jbe    0x501f
    508a:	c6                   	(bad)
    508b:	8b 00                	mov    (%bx,%si),%ax
    508d:	b8 9d 3d             	mov    $0x3d9d,%ax
    5090:	9c                   	pushf
    5091:	e8 ff 9c             	call   0xed93
    5094:	ba 9a f6             	mov    $0xf69a,%dx
    5097:	4d                   	dec    %bp
    5098:	5b                   	pop    %bx
    5099:	53                   	push   %bx
    509a:	e4 2a                	in     $0x2a,%al
    509c:	9a fa 14 86 b2       	lcall  $0xb286,$0x14fa
    50a1:	22 a3 8a 8a          	and    -0x7576(%bp,%di),%ah
    50a5:	eb 8a                	jmp    0x5031
    50a7:	e8 ff 9c             	call   0xeda9
    50aa:	ba 9a 44             	mov    $0x449a,%dx
    50ad:	4d                   	dec    %bp
    50ae:	5b                   	pop    %bx
    50af:	53                   	push   %bx
    50b0:	b8 b4 3d             	mov    $0x3db4,%ax
    50b3:	e8 9c 9a             	call   0xeb52
    50b6:	4d                   	dec    %bp
    50b7:	53                   	push   %bx
    50b8:	2a 37                	sub    (%bx),%dh
    50ba:	4d                   	dec    %bp
    50bb:	5b                   	pop    %bx
    50bc:	53                   	push   %bx
    50bd:	9c                   	pushf
    50be:	9a 4d 53 2a 10       	lcall  $0x102a,$0x534d
    50c3:	09 a0 18 1a          	or     %sp,0x1a18(%bx,%si)
    50c7:	04 47                	add    $0x47,%al
    50c9:	08 2a                	or     %ch,(%bp,%si)
    50cb:	c1 0e 0e b0 bf       	rorw   $0xbf,-0x4ff2
    50d0:	e4 19                	in     $0x19,%al
    50d2:	30 1a                	xor    %bl,(%bp,%si)
    50d4:	75 b9                	jne    0x508f
    50d6:	85 42 b8             	test   %ax,-0x48(%bp,%si)
    50d9:	1d eb 07             	sbb    $0x7eb,%ax
    50dc:	b8 89 81             	mov    $0x8189,%ax
    50df:	b8 50 fd             	mov    $0xfd50,%ax
    50e2:	f6 a1 70 52          	mulb   0x5270(%bx,%di)
    50e6:	a3 0e 1a             	mov    %ax,0x1a0e
    50e9:	a1 72 52             	mov    0x5272,%ax
    50ec:	a3 10 1a             	mov    %ax,0x1a10
    50ef:	e8 2f cd             	call   0x1e21
    50f2:	e2 c1                	loop   0x50b5
    50f4:	b9 87 89             	mov    $0x8987,%cx
    50f7:	81 b8 9f 2b 04 e8    	cmpw   $0xe804,0x2b9f(%bx,%si)
    50fd:	6f                   	outsw  %ds:(%si),(%dx)
    50fe:	5f                   	pop    %di
    50ff:	03 be c7 15          	add    0x15c7(%bp),%di
    5103:	a1 0e 1a             	mov    0x1a0e,%ax
    5106:	a3 70 52             	mov    %ax,0x5270
    5109:	a1 10 1a             	mov    0x1a10,%ax
    510c:	a3 72 52             	mov    %ax,0x5272
    510f:	8b 9a 1a 0c          	mov    0xc1a(%bp,%si),%bx
    5113:	fa                   	cli
    5114:	7e 00                	jle    0x5116
    5116:	18 5e 8b             	sbb    %bl,-0x75(%bp)
    5119:	9a 1a 5e eb b1       	lcall  $0xb1eb,$0x5e1a
    511e:	06                   	push   %es
    511f:	07                   	pop    %es
    5120:	76 95                	jbe    0x50b7
    5122:	c6                   	(bad)
    5123:	17                   	pop    %ss
    5124:	01 c7                	add    %ax,%di
    5126:	00 89 fe d4          	add    %cl,-0x2b02(%bx,%di)
    512a:	fe                   	(bad)
    512b:	7e 8c                	jle    0x50b9
    512d:	24 eb                	and    $0xeb,%al
    512f:	80 8b 87 8b fe       	orb    $0xfe,-0x7479(%bp,%di)
    5134:	8a 07                	mov    (%bx),%al
    5136:	20 3b                	and    %bh,(%bp,%di)
    5138:	b8 20 04             	mov    $0x420,%ax
    513b:	50                   	push   %ax
    513c:	b9 4c 57             	mov    $0x574c,%cx
    513f:	fc                   	cld
    5140:	e8 d3 f6             	call   0x4816
    5143:	83 20 47             	andw   $0x47,(%bx,%si)
    5146:	e7 ff                	out    %ax,$0xff
    5148:	00 89 81 f8          	add    %cl,-0x77f(%bx,%di)
    514c:	d0 14                	rclb   $1,(%si)
    514e:	72 5e                	jb     0x51ae
    5150:	e0 0c                	loopne 0x515e
    5152:	f6 fa                	idiv   %dl
    5154:	8b c6                	mov    %si,%ax
    5156:	8b 9a 1a fc          	mov    -0x3e6(%bp,%si),%bx
    515a:	13 06 86 03          	adc    0x386,%ax
    515e:	72 95                	jb     0x50f5
    5160:	73 10                	jae    0x5172
    5162:	b1 d3                	mov    $0xd3,%cl
    5164:	80 ec 8a             	sub    $0x8a,%ah
    5167:	eb 89                	jmp    0x50f2
    5169:	5e                   	pop    %si
    516a:	98                   	cbtw
    516b:	bf 8a ee             	mov    $0xee8a,%di
    516e:	95                   	xchg   %ax,%bp
    516f:	8b b8 3d e8          	mov    -0x17c3(%bx,%si),%di
    5173:	c7 00 c6 87          	movw   $0x87c6,(%bx,%si)
    5177:	87 52 ff             	xchg   %dx,-0x1(%bp,%si)
    517a:	c6 87 ee 95 8b       	movb   $0x8b,-0x6a12(%bx)
    517f:	56                   	push   %si
    5180:	e8 9c 02             	call   0x541f
    5183:	b9 19 00             	mov    $0x19,%cx
    5186:	8a 1e 4e 63          	mov    0x634e,%bl
    518a:	2a ff                	sub    %bh,%bh
    518c:	b8 a1 a6             	mov    $0xa6a1,%ax
    518f:	8b 7e 98             	mov    -0x68(%bp),%di
    5192:	8a 95 ee 95          	mov    -0x6a12(%di),%dl
    5196:	2a f6                	sub    %dh,%dh
    5198:	2b c2                	sub    %dx,%ax
    519a:	f7 eb                	imul   %bx
    519c:	f7 e9                	imul   %cx
    519e:	b9 64 00             	mov    $0x64,%cx
    51a1:	99                   	cwtd
    51a2:	f9                   	stc
    51a3:	4b                   	dec    %bx
    51a4:	a2 4e 63             	mov    %al,0x634e
    51a7:	03 72 05             	add    0x5(%bp,%si),%si
    51aa:	76 0a                	jbe    0x51b6
    51ac:	b8 fb 09             	mov    $0x9fb,%ax
    51af:	f0 f6 7d 21          	lock idivb 0x21(%di)
    51b3:	76 2b                	jbe    0x51e0
    51b5:	97                   	xchg   %ax,%di
    51b6:	03 72 02             	add    0x2(%bp,%si),%si
    51b9:	b8 1e eb             	mov    $0xeb1e,%ax
    51bc:	e2 ff                	loop   0x51bd
    51be:	eb b8                	jmp    0x5178
    51c0:	00 f1                	add    %dh,%cl
    51c2:	ff                   	ljmp   (bad)
    51c3:	eb ea                	jmp    0x51af
    51c5:	8b 8b 1a 13          	mov    0x131a(%bp,%di),%cx
    51c9:	86 72 73             	xchg   %dh,0x73(%bp,%si)
    51cc:	b1 80                	mov    $0x80,%cl
    51ce:	b0 bf                	mov    $0xbf,%al
    51d0:	4d                   	dec    %bp
    51d1:	72 23                	jb     0x51f6
    51d3:	f6 fa                	idiv   %dl
    51d5:	89 ee                	mov    %bp,%si
    51d7:	ff a2 73 61          	jmp    *0x6173(%bp,%si)
    51db:	14 f8                	adc    $0xf8,%al
    51dd:	20 ff                	and    %bh,%bh
    51df:	f2 89 46 94          	repnz mov %ax,-0x6c(%bp)
    51e3:	b9 6e f8             	mov    $0xf86e,%cx
    51e6:	0a 89 0e 04          	or     0x40e(%bx,%di),%cl
    51ea:	89 5e 96             	mov    %bx,-0x6a(%bp)
    51ed:	bf 8a 16             	mov    $0x168a,%di
    51f0:	11 ff                	adc    %di,%di
    51f2:	74 43                	je     0x5237
    51f4:	8a 87 14 11          	mov    0x1114(%bx),%al
    51f8:	20 3b                	and    %bh,(%bp,%di)
    51fa:	b8 c6 75             	mov    $0x75c6,%ax
    51fd:	39 9f 04 6f          	cmp    %bx,0x6f04(%bx)
    5201:	f8                   	clc
    5202:	f8                   	clc
    5203:	fd                   	std
    5204:	d3 1a                	rcrw   %cl,(%bp,%si)
    5206:	13 94 c7 eb          	adc    -0x1439(%si),%dx
    520a:	eb e8                	jmp    0x51f4
    520c:	00 0a                	add    %cl,(%bp,%si)
    520e:	57                   	push   %di
    520f:	9b                   	fwait
    5210:	f9                   	stc
    5211:	e8 0d cc             	call   0x1e21
    5214:	9f                   	lahf
    5215:	04 6f                	add    $0x6f,%al
    5217:	03 05                	add    (%di),%ax
    5219:	4c                   	dec    %sp
    521a:	85 b8 da 03          	test   %di,0x3da(%bx,%si)
    521e:	06                   	push   %es
    521f:	02 65 02             	add    0x2(%di),%ah
    5222:	9e                   	sahf
    5223:	5e                   	pop    %si
    5224:	8b f8                	mov    %ax,%di
    5226:	f7 c7 8a 75          	test   $0x758a,%di
    522a:	b8 10 a8             	mov    $0xa810,%ax
    522d:	04 74                	add    $0x74,%al
    522f:	07                   	pop    %es
    5230:	b9 2b 85             	mov    $0x852b,%cx
    5233:	e8 da 0d             	call   0x6010
    5236:	00 72 b8             	add    %dh,-0x48(%bp,%si)
    5239:	d8 e8                	fsubr  %st(0),%st
    523b:	99                   	cwtd
    523c:	5e                   	pop    %si
    523d:	83 46 d8 5d          	addw   $0x5d,-0x28(%bp)
    5241:	8a 31                	mov    (%bx,%di),%dh
    5243:	fa                   	cli
    5244:	a1 3c 06             	mov    0x63c,%ax
    5247:	3c 04                	cmp    $0x4,%al
    5249:	89 76 3e             	mov    %si,0x3e(%bp)
    524c:	f8                   	clc
    524d:	f4                   	hlt
    524e:	3d 1e b0             	cmp    $0xb01e,%ax
    5251:	bf fe 81             	mov    $0x81fe,%di
    5254:	05 47 88             	add    $0x8847,%ax
    5257:	88 8a 13 f2          	mov    %cl,-0xded(%bp,%si)
    525b:	8a 87 14 11          	mov    0x1114(%bx),%al
    525f:	88 46 fa             	mov    %al,-0x6(%bp)
    5262:	8a 86 b1 80          	mov    -0x7f4f(%bp),%al
    5266:	53                   	push   %bx
    5267:	55                   	push   %bp
    5268:	1b fb                	sbb    %bx,%di
    526a:	17                   	pop    %ss
    526b:	76 f9                	jbe    0x5266
    526d:	2b f2                	sub    %dx,%si
    526f:	ff 73 08             	push   0x8(%bp,%di)
    5272:	73 7a                	jae    0x52ee
    5274:	e3 05                	jcxz   0x527b
    5276:	fd                   	std
    5277:	eb fa                	jmp    0x5273
    5279:	0d 80 ff             	or     $0xff80,%ax
    527c:	ec                   	in     (%dx),%al
    527d:	80 1b 17             	sbbb   $0x17,(%bp,%di)
    5280:	17                   	pop    %ss
    5281:	c0 c1 22             	rol    $0x22,%cl
    5284:	b8 ff 74             	mov    $0x74ff,%ax
    5287:	3b 3c                	cmp    (%si),%di
    5289:	1b 72 37             	sbb    0x37(%bp,%si),%si
    528c:	8a 86 b1 80          	mov    -0x7f4f(%bp),%al
    5290:	53                   	push   %bx
    5291:	76 f9                	jbe    0x528c
    5293:	38 55 1b             	cmp    %dl,0x1b(%di)
    5296:	fb                   	sti
    5297:	77 2a                	ja     0x52c3
    5299:	8a c3                	mov    %bl,%al
    529b:	1b fb                	sbb    %bx,%di
    529d:	17                   	pop    %ss
    529e:	be 38 11             	mov    $0x1138,%si
    52a1:	eb 14                	jmp    0x52b7
    52a3:	8a 68 1a             	mov    0x1a(%bx,%si),%ch
    52a6:	0c 7c                	or     $0x7c,%al
    52a8:	76 0a                	jbe    0x52b4
    52aa:	80 02 06             	addb   $0x6,(%bp,%si)
    52ad:	22 03                	and    (%bp,%di),%al
    52af:	fa                   	cli
    52b0:	db 01                	fildl  (%bx,%di)
    52b2:	fb                   	sti
    52b3:	f2 ff c6             	repnz inc %si
    52b6:	0c 81                	or     $0x81,%al
    52b8:	fe                   	(bad)
    52b9:	5c                   	pop    %sp
    52ba:	11 72 e6             	adc    %si,-0x1a(%bp,%si)
    52bd:	89 ee                	mov    %bp,%si
    52bf:	fb                   	sti
    52c0:	f2 75 03             	bnd jne 0x52c6
    52c3:	47                   	inc    %di
    52c4:	eb a9                	jmp    0x526f
    52c6:	53                   	push   %bx
    52c7:	76 fb                	jbe    0x52c4
    52c9:	8b f2                	mov    %dx,%si
    52cb:	07                   	pop    %es
    52cc:	88 87 14 11          	mov    %al,0x1114(%bx)
    52d0:	53                   	push   %bx
    52d1:	55                   	push   %bp
    52d2:	88 3c                	mov    %bh,(%si)
    52d4:	29 47 2a             	sub    %ax,0x2a(%bx)
    52d7:	0e                   	push   %cs
    52d8:	b0 e4                	mov    $0xe4,%al
    52da:	30 75 fe             	xor    %dh,-0x2(%di)
    52dd:	5e                   	pop    %si
    52de:	19 80 25 ff          	sbb    %ax,-0xdb(%bx,%si)
    52e2:	53                   	push   %bx
    52e3:	76 fb                	jbe    0x52e0
    52e5:	eb a1                	jmp    0x5288
    52e7:	11 01                	adc    %ax,(%bx,%di)
    52e9:	f8                   	clc
    52ea:	f4                   	hlt
    52eb:	3d ff 04             	cmp    $0x4ff,%ax
    52ee:	81 7e 3d 1e 75       	cmpw   $0x751e,0x3d(%bp)
    52f3:	23 b8 ce 3d          	and    0x3dce(%bx,%si),%di
    52f7:	c8 00 d9 06          	enter  $0xd900,$0x6
    52fb:	8b f2                	mov    %dx,%si
    52fd:	07                   	pop    %es
    52fe:	c6 87 16 ff 43       	movb   $0x43,-0xea(%bx)
    5303:	80 bf 88 3c b0       	cmpb   $0xb0,0x3c88(%bx)
    5308:	e4 0d                	in     $0xd,%al
    530a:	8b f2                	mov    %dx,%si
    530c:	07                   	pop    %es
    530d:	53                   	push   %bx
    530e:	55                   	push   %bp
    530f:	88 3c                	mov    %bh,(%si)
    5311:	5c                   	pop    %sp
    5312:	46                   	inc    %si
    5313:	4b                   	dec    %bx
    5314:	7d ff                	jge    0x5315
    5316:	f4                   	hlt
    5317:	5e                   	pop    %si
    5318:	83 46 d8 5d          	addw   $0x5d,-0x28(%bp)
    531c:	8a 31                	mov    (%bx,%di),%dh
    531e:	76 89                	jbe    0x52a9
    5320:	e8 fc 47             	call   0x9b1f
    5323:	2b fa                	sub    %dx,%di
    5325:	f8                   	clc
    5326:	6e                   	outsb  %ds:(%si),(%dx)
    5327:	52                   	push   %dx
    5328:	f0 e8 45 e8          	lock call 0x3b71
    532c:	3d b4 76             	cmp    $0x76b4,%ax
    532f:	87 ee                	xchg   %bp,%si
    5331:	02 76 f8             	add    -0x8(%bp),%dh
    5334:	fe                   	(bad)
    5335:	76 fe                	jbe    0x5335
    5337:	16                   	push   %ss
    5338:	89 46 7d             	mov    %ax,0x7d(%bp)
    533b:	1c c1                	sbb    $0xc1,%al
    533d:	7e 01                	jle    0x5340
    533f:	8a fc                	mov    %ah,%bh
    5341:	b0 87                	mov    $0x87,%al
    5343:	c1 84 f8 03 89       	rolw   $0x89,0x3f8(%si)
    5348:	e8 89 87             	call   0xdad4
    534b:	c1 84 47 f8 fa       	rolw   $0xfa,-0x7b9(%si)
    5350:	8b f2                	mov    %dx,%si
    5352:	fc                   	cld
    5353:	87 de                	xchg   %bx,%si
    5355:	f7 d1                	not    %cx
    5357:	8b c3                	mov    %bx,%ax
    5359:	2b fe                	sub    %si,%di
    535b:	02 f1                	add    %cl,%dh
    535d:	b1 04                	mov    $0x4,%cl
    535f:	80 d0 d3             	adc    $0xd3,%al
    5362:	e8 03 d0             	call   0x2368
    5365:	32 b9 eb fe          	xor    -0x115(%bx,%di),%bh
    5369:	be 7e 5e             	mov    $0x5e7e,%si
    536c:	8b 20                	mov    (%bx,%si),%sp
    536e:	fa                   	cli
    536f:	eb 1a                	jmp    0x538b
    5371:	39 f2                	cmp    %si,%dx
    5373:	fc                   	cld
    5374:	76 0a                	jbe    0x5380
    5376:	8b f3                	mov    %bx,%si
    5378:	d1 e6                	shl    $1,%si
    537a:	89 bc 56 11          	mov    %di,0x1156(%si)
    537e:	eb 0a                	jmp    0x538a
    5380:	8b f3                	mov    %bx,%si
    5382:	d1 e6                	shl    $1,%si
    5384:	c7 84 56 11 fe be    	movw   $0xbefe,0x1156(%si)
    538a:	43                   	inc    %bx
    538b:	83 fb 03             	cmp    $0x3,%bx
    538e:	72 e1                	jb     0x5371
    5390:	02 f8                	add    %al,%bh
    5392:	76 16                	jbe    0x53aa
    5394:	46                   	inc    %si
    5395:	1c 7e                	sbb    $0x7e,%al
    5397:	8a b0 c1 13          	mov    0x13c1(%bx,%si),%dh
    539b:	c1 a2 5c 11 c6       	shlw   $0xc6,0x115c(%bp,%si)
    53a0:	06                   	push   %es
    53a1:	5e                   	pop    %si
    53a2:	11 fe                	adc    %di,%si
    53a4:	8e e4                	mov    %sp,%fs
    53a6:	b0 a2                	mov    $0xa2,%al
    53a8:	5f                   	pop    %di
    53a9:	11 8f e8 20          	adc    %cx,0x20e8(%bx)
    53ad:	2b fe                	sub    %si,%di
    53af:	02 f7                	add    %bh,%dh
    53b1:	a2 60 11             	mov    %al,0x1160
    53b4:	83 d8 8a             	sbb    $0xff8a,%ax
    53b7:	76 e8                	jbe    0x53a1
    53b9:	47                   	inc    %di
    53ba:	04 02                	add    $0x2,%al
    53bc:	f8                   	clc
    53bd:	6e                   	outsb  %ds:(%si),(%dx)
    53be:	be 13 c1             	mov    $0xc113,%si
    53c1:	eb 2c                	jmp    0x53ef
    53c3:	83 7c 07 00          	cmpw   $0x0,0x7(%si)
    53c7:	74 23                	je     0x53ec
    53c9:	8a 3b                	mov    (%bp,%di),%bh
    53cb:	33 b8 8b f8          	xor    -0x775(%bx,%si),%di
    53cf:	83 ff 1b             	cmp    $0x1b,%di
    53d2:	73 05                	jae    0x53d9
    53d4:	a1 10 c1             	mov    0xc110,%ax
    53d7:	eb 03                	jmp    0x53dc
    53d9:	a1 f4 c0             	mov    0xc0f4,%ax
    53dc:	50                   	push   %ax
    53dd:	d1 e7                	shl    $1,%di
    53df:	d1 e7                	shl    $1,%di
    53e1:	ff b5 98 14          	push   0x1498(%di)
    53e5:	14 b5                	adc    $0xb5,%al
    53e7:	14 5e                	adc    $0x5e,%al
    53e9:	bc 76 3a             	mov    $0x3a76,%sp
    53ec:	f7 7c 72             	idivw  0x72(%si)
    53ef:	81 fe ad c1          	cmp    $0xc1ad,%si
    53f3:	72 ce                	jb     0x53c3
    53f5:	89 f4                	mov    %si,%sp
    53f7:	f0 fa                	lock cli
    53f9:	22 a4 00 4f          	and    0x4f00(%si),%ah
    53fd:	02 bb 39 11          	add    0x1139(%bp,%di),%bh
    5401:	eb 10                	jmp    0x5413
    5403:	8a 20                	mov    (%bx,%si),%ah
    5405:	b8 04 b9             	mov    $0xb904,%ax
    5408:	57                   	push   %di
    5409:	04 75                	add    $0x75,%al
    540b:	04 c6                	add    $0xc6,%al
    540d:	47                   	inc    %di
    540e:	01 11                	add    %dx,(%bx,%di)
    5410:	c3                   	ret
    5411:	e5 0c                	in     $0xc,%ax
    5413:	81 fb 5d 11          	cmp    $0x115d,%bx
    5417:	72 ea                	jb     0x5403
    5419:	46                   	inc    %si
    541a:	5d                   	pop    %bp
    541b:	31 89 fc 2b          	xor    %cx,0x2bfc(%bx,%di)
    541f:	f8                   	clc
    5420:	52                   	push   %dx
    5421:	e8 e8 b4             	call   0x90c
    5424:	10 bb 39 11          	adc    %bh,0x1139(%bp,%di)
    5428:	f8                   	clc
    5429:	f2 c1 96 3d f8 3d    	repnz rclw $0x3d,-0x7c3(%bp)
    542f:	04 12                	add    $0x12,%al
    5431:	f8                   	clc
    5432:	f2 c1 f8 90          	repnz sar $0x90,%ax
    5436:	3d eb 7e             	cmp    $0x7eeb,%ax
    5439:	8a b8 b9 04          	mov    0x4b9(%bx,%si),%bh
    543d:	04 47                	add    $0x47,%al
    543f:	11 e5                	adc    %sp,%bp
    5441:	66 e8 47 f8 fa fa    	calll  0xfafb4c8e
    5447:	76 ee                	jbe    0x5437
    5449:	76 8d                	jbe    0x53d8
    544b:	47                   	inc    %di
    544c:	f9                   	stc
    544d:	3b f2                	cmp    %dx,%si
    544f:	f0 89 07             	lock mov %ax,(%bx)
    5452:	e3 eb                	jcxz   0x543f
    5454:	13 7e 5e             	adc    0x5e(%bp),%di
    5457:	8b 20                	mov    (%bx,%si),%sp
    5459:	07                   	pop    %es
    545a:	01 fa                	add    %di,%dx
    545c:	56                   	push   %si
    545d:	83 47 ff 02          	addw   $0x2,-0x1(%bx)
    5461:	c7 07 00 00          	movw   $0x0,(%bx)
    5465:	ff 89 f8 76          	decw   0x76f8(%bx,%di)
    5469:	1a 03                	sbb    (%bp,%di),%al
    546b:	03 72 e7             	add    -0x19(%bp,%si),%si
    546e:	8b f2                	mov    %dx,%si
    5470:	07                   	pop    %es
    5471:	64 3d 02 0a          	fs cmp $0xa02,%ax
    5475:	47                   	inc    %di
    5476:	8b 1f                	mov    (%bx),%bx
    5478:	b0 fc                	mov    $0xfc,%al
    547a:	3f                   	aas
    547b:	03 5e f4             	add    -0xc(%bp),%bx
    547e:	d1 e3                	shl    $1,%bx
    5480:	89 0e f9 c7          	mov    %cx,-0x3807
    5484:	76 1a                	jbe    0x54a0
    5486:	fa                   	cli
    5487:	00 75 17             	add    %dh,0x17(%di)
    548a:	b9 a1 8b             	mov    $0x8ba1,%cx
    548d:	98                   	cbtw
    548e:	fc                   	cld
    548f:	03 f4                	add    %sp,%si
    5491:	e3 0e                	jcxz   0x54a1
    5493:	c7                   	(bad)
    5494:	1a 8b 0e f9          	sbb    -0x6f2(%bp,%di),%cl
    5498:	c7                   	(bad)
    5499:	2b 02                	sub    (%bp,%si),%ax
    549b:	b1 80                	mov    $0x80,%cl
    549d:	89 0e f9 c7          	mov    %cx,-0x3807
    54a1:	06                   	push   %es
    54a2:	07                   	pop    %es
    54a3:	37                   	aaa
    54a4:	c6 47 01 00          	movb   $0x0,0x1(%bx)
    54a8:	c3                   	ret
    54a9:	e5 0c                	in     $0xc,%ax
    54ab:	f7 f2                	div    %dx
    54ad:	f7 02 c7 0c          	testw  $0xcc7,(%bp,%si)
    54b1:	f4                   	hlt
    54b2:	06                   	push   %es
    54b3:	c7                   	(bad)
    54b4:	0e                   	push   %cs
    54b5:	f8                   	clc
    54b6:	02 81 7e f8          	add    -0x782(%bx,%di),%al
    54ba:	96                   	xchg   %ax,%si
    54bb:	3d 73 2b             	cmp    $0x2b73,%ax
    54be:	ff e7                	jmp    *%di
    54c0:	8b f6                	mov    %si,%si
    54c2:	f9                   	stc
    54c3:	d6                   	(bad)
    54c4:	c6                   	(bad)
    54c5:	16                   	push   %ss
    54c6:	ef                   	out    %ax,(%dx)
    54c7:	72 0e                	jb     0x54d7
    54c9:	1b ea                	sbb    %dx,%bp
    54cb:	88 e8                	mov    %ch,%al
    54cd:	bf 00 01             	mov    $0x100,%di
    54d0:	83 00 0b             	addw   $0xb,(%bx,%si)
    54d3:	05 c4 2a             	add    $0x2ac4,%ax
    54d6:	43                   	inc    %bx
    54d7:	50                   	push   %ax
    54d8:	36 03 0e fd 77       	add    %ss:0x77fd,%cx
    54dd:	c8 e8 fa e0          	enter  $0xfae8,$0xe0
    54e1:	f6 8b 06 eb 1b       	testb  $0x1b,-0x14fa(%bp,%di)
    54e6:	e8 b2 e0             	call   0x359b
    54e9:	90                   	nop
    54ea:	32 32                	xor    (%bp,%si),%dh
    54ec:	0a 56 b8             	or     -0x48(%bp),%dl
    54ef:	b6 03                	mov    $0x3,%dh
    54f1:	0e                   	push   %cs
    54f2:	fd                   	std
    54f3:	36 00 eb             	ss add %ch,%bl
    54f6:	08 56 a0             	or     %dl,-0x60(%bp)
    54f9:	7f 39                	jg     0x5534
    54fb:	27                   	daa
    54fc:	de 06 8b 98          	fiadds -0x6775
    5500:	f8                   	clc
    5501:	81 ff 00 01          	cmp    $0x100,%di
    5505:	74 df                	je     0x54e6
    5507:	56                   	push   %si
    5508:	57                   	push   %di
    5509:	c2 2a 6d             	ret    $0x6d2a
    550c:	f9                   	stc
    550d:	1b 4b 03             	sbb    0x3(%bp,%di),%cx
    5510:	e8 c8 e0             	call   0x35db
    5513:	e8 74 c8             	call   0x1d8a
    5516:	c7 05 eb 09          	movw   $0x9eb,(%di)
    551a:	56                   	push   %si
    551b:	b9 4c 57             	mov    $0x574c,%cx
    551e:	e6 03                	out    %al,$0x3
    5520:	e8 de f4             	call   0x4a01
    5523:	e3 66                	jcxz   0x558b
    5525:	06                   	push   %es
    5526:	b0 f2                	mov    $0xf2,%al
    5528:	81 74 3c b8 ee       	xorw   $0xeeb8,0x3c(%si)
    552d:	03 02                	add    (%bp,%si),%ax
    552f:	fa                   	cli
    5530:	06                   	push   %es
    5531:	08 06 19 8b          	or     %al,-0x74e7
    5535:	89 03                	mov    %ax,(%bp,%di)
    5537:	5e                   	pop    %si
    5538:	64 3d 02 57          	fs cmp $0x5702,%ax
    553c:	02 f8                	add    %al,%bh
    553e:	fa                   	cli
    553f:	fa                   	cli
    5540:	76 56                	jbe    0x5598
    5542:	fc                   	cld
    5543:	50                   	push   %ax
    5544:	52                   	push   %dx
    5545:	56                   	push   %si
    5546:	e8 ef 2c             	call   0x8238
    5549:	03 c7                	add    %di,%ax
    554b:	a1 3c 8b             	mov    0x8b3c,%ax
    554e:	04 29                	add    $0x29,%al
    5550:	fe 8b f6 44          	decb   0x44f6(%bp,%di)
    5554:	02 29                	add    (%bx,%di),%ch
    5556:	fe                   	(bad)
    5557:	fc                   	cld
    5558:	8a 38                	mov    (%bx,%si),%bh
    555a:	73 87                	jae    0x54e3
    555c:	87 56 ff             	xchg   %dx,-0x1(%bp)
    555f:	38 44 0e             	cmp    %al,0xe(%si)
    5562:	76 25                	jbe    0x5589
    5564:	13 53 31             	adc    0x31(%bp,%di),%dx
    5567:	5b                   	pop    %bx
    5568:	76 fc                	jbe    0x5566
    556a:	bf 81 47             	mov    $0x4781,%di
    556d:	88 ff                	mov    %bh,%bh
    556f:	03 44 0a             	add    0xa(%si),%ax
    5572:	05 fb 47             	add    $0x47fb,%ax
    5575:	3e 3e bf 2c 03       	ds ds mov $0x32c,%di
    557a:	c7                   	(bad)
    557b:	75 05                	jne    0x5582
    557d:	bf fb da             	mov    $0xdafb,%di
    5580:	46                   	inc    %si
    5581:	75 14                	jne    0x5597
    5583:	20 f2                	and    %dh,%dl
    5585:	46                   	inc    %si
    5586:	b9 eb 1d             	mov    $0x1deb,%cx
    5589:	1e                   	push   %ds
    558a:	00 01                	add    %al,(%bx,%di)
    558c:	0c be                	or     $0xbe,%al
    558e:	74 06                	je     0x5596
    5590:	88 d6                	mov    %dl,%dh
    5592:	0d 15 ef             	or     $0xef15,%ax
    5595:	40                   	inc    %ax
    5596:	e0 fe                	loopne 0x5596
    5598:	02 57 88             	add    -0x78(%bx),%dl
    559b:	47                   	inc    %di
    559c:	3b 43 74             	cmp    0x74(%bp,%di),%ax
    559f:	3e 3e 39 48 5b       	ds cmp %cx,%ds:0x5b(%bx,%si)
    55a4:	88 c2                	mov    %al,%dl
    55a6:	36 ff 33             	push   %ss:(%bp,%di)
    55a9:	bf eb 8b             	mov    $0x8beb,%di
    55ac:	89 47 0a             	mov    %ax,0xa(%bx)
    55af:	40                   	inc    %ax
    55b0:	40                   	inc    %ax
    55b1:	8b 8b e4 bf          	mov    -0x401c(%bp,%di),%cx
    55b5:	eb 06                	jmp    0x55bd
    55b7:	8a 47 0e             	mov    0xe(%bx),%al
    55ba:	b9 04 11             	mov    $0x1104,%cx
    55bd:	c7                   	(bad)
    55be:	76 15                	jbe    0x55d5
    55c0:	87 fe                	xchg   %di,%si
    55c2:	07                   	pop    %es
    55c3:	3b 07                	cmp    (%bx),%ax
    55c5:	75 ff                	jne    0x55c6
    55c7:	07                   	pop    %es
    55c8:	74 83                	je     0x554d
    55ca:	20 47 e7             	and    %al,-0x19(%bx)
    55cd:	0c 47                	or     $0x47,%al
    55cf:	eb e3                	jmp    0x55b4
    55d1:	c7 05 eb 0e          	movw   $0xeeb,(%di)
    55d5:	80 0c 0c             	orb    $0xc,(%si)
    55d8:	1b b8 eb ff          	sbb    -0x15(%bx,%si),%di
    55dc:	2a eb                	sub    %bl,%ch
    55de:	8b 1d                	mov    (%di),%bx
    55e0:	1e                   	push   %ds
    55e1:	00 01                	add    %al,(%bx,%di)
    55e3:	66 b0 81             	data32 mov $0x81,%al
    55e6:	3c ee                	cmp    $0xee,%al
    55e8:	02 06 06 8b          	add    -0x74fa,%al
    55ec:	03 89 03 5e          	add    0x5e03(%bx,%di),%cx
    55f0:	c7 47 10 2a 6d       	movw   $0x6d2a,0x10(%bx)
    55f5:	53                   	push   %bx
    55f6:	e8 bd 8b             	call   0xe1b6
    55f9:	98                   	cbtw
    55fa:	5d                   	pop    %bp
    55fb:	89 2b                	mov    %bp,(%bp,%di)
    55fd:	52                   	push   %dx
    55fe:	e8 10 39             	call   0x8f11
    5601:	f8                   	clc
    5602:	89 5e 47             	mov    %bx,0x47(%bp)
    5605:	2a 53 bd             	sub    -0x43(%bp,%di),%dl
    5608:	00 00                	add    %al,(%bx,%si)
    560a:	53                   	push   %bx
    560b:	e8 e9 01             	call   0x57f7
    560e:	46                   	inc    %si
    560f:	5d                   	pop    %bp
    5610:	31 89 fc 2b          	xor    %cx,0x2bfc(%bx,%di)
    5614:	b8 6e ee             	mov    $0xee6e,%ax
    5617:	5e                   	pop    %si
    5618:	03 fe                	add    %si,%di
    561a:	08 89 04 8a          	or     %cl,-0x75fc(%bx,%di)
    561e:	36 ff                	ss (bad)
    5620:	39 de                	cmp    %bx,%si
    5622:	8b 30                	mov    (%bx,%si),%si
    5624:	c7 03 02 1e          	movw   $0x1e02,(%bp,%di)
    5628:	e2 17                	loop   0x5641
    562a:	5e                   	pop    %si
    562b:	8a 06 ff 37          	mov    0x37ff,%al
    562f:	ff 77 02             	push   0x2(%bx)
    5632:	ff 77 04             	push   0x4(%bx)
    5635:	ff 77 06             	push   0x6(%bx)
    5638:	ff                   	(bad)
    5639:	3e 00 77 8d          	add    %dh,%ds:-0x73(%bx)
    563d:	f5                   	cmc
    563e:	da eb                	(bad)
    5640:	0e                   	push   %cs
    5641:	4e                   	dec    %si
    5642:	75 0b                	jne    0x564f
    5644:	5e                   	pop    %si
    5645:	06                   	push   %es
    5646:	37                   	aaa
    5647:	77 ff                	ja     0x5648
    5649:	04 77                	add    $0x77,%al
    564b:	ff                   	ljmp   (bad)
    564c:	e8 97 da             	call   0x30e6
    564f:	8a 38                	mov    (%bx,%si),%bh
    5651:	73 87                	jae    0x55da
    5653:	f6 06 1b b2 41       	testb  $0x41,-0x4de5
    5658:	bf 81 47             	mov    $0x4781,%di
    565b:	88 ff                	mov    %bh,%bh
    565d:	8b f8                	mov    %ax,%di
    565f:	4d                   	dec    %bp
    5660:	72 23                	jb     0x5685
    5662:	8b 5c 0a             	mov    0xa(%si),%bx
    5665:	80 79 03 00          	cmpb   $0x0,0x3(%bx,%di)
    5669:	74 08                	je     0x5673
    566b:	8d 0f                	lea    (%bx),%cx
    566d:	11 3c                	adc    %di,(%si)
    566f:	06                   	push   %es
    5670:	eb 8b                	jmp    0x55fd
    5672:	8b 89 44 08          	mov    0x844(%bx,%di),%cx
    5676:	a3 11 3c             	mov    %ax,0x3c11
    5679:	8b 7c 0a             	mov    0xa(%si),%di
    567c:	98                   	cbtw
    567d:	03 e3                	add    %bx,%sp
    567f:	ff 31                	push   (%bx,%di)
    5681:	03 fb                	add    %bx,%di
    5683:	8b 45 04             	mov    0x4(%di),%ax
    5686:	03 04                	add    (%si),%ax
    5688:	05 0b 50             	add    $0x500b,%ax
    568b:	89 03                	mov    %ax,(%bp,%di)
    568d:	45                   	inc    %bp
    568e:	06                   	push   %es
    568f:	03 44 02             	add    0x2(%si),%ax
    5692:	de 8b 06 33          	fimuls 0x3306(%bp,%di)
    5696:	bf da 75             	mov    $0x75da,%di
    5699:	87 ff                	xchg   %di,%di
    569b:	44                   	inc    %sp
    569c:	76 13                	jbe    0x56b1
    569e:	31 77 b7             	xor    %si,-0x49(%bx)
    56a1:	83 7c bd 00          	cmpw   $0x0,-0x43(%si)
    56a5:	74 04                	je     0x56ab
    56a7:	56                   	push   %si
    56a8:	e8 0b 50             	call   0xa6b6
    56ab:	e8 dc f7             	call   0x4e8a
    56ae:	2b fe                	sub    %si,%di
    56b0:	8a 08                	mov    (%bx,%si),%cl
    56b2:	04 a1                	add    $0xa1,%al
    56b4:	89 ff                	mov    %di,%di
    56b6:	69 5e fc 5e 03       	imul   $0x35e,-0x4(%bp),%bx
    56bb:	0c 08                	or     $0x8,%al
    56bd:	04 04                	add    $0x4,%al
    56bf:	f6 36 8d 0f          	divb   0xf8d
    56c3:	fa                   	cli
    56c4:	9a 08 00 ca 2f       	lcall  $0x2fca,$0x8
    56c9:	fb                   	sti
    56ca:	00 76 97             	add    %dh,-0x69(%bp)
    56cd:	72 bf                	jb     0x568e
    56cf:	81 47 88 fb 8b       	addw   $0x8bfb,-0x78(%bx)
    56d4:	d8 03                	fadds  (%bp,%di)
    56d6:	5d                   	pop    %bp
    56d7:	0a 98 77 04          	or     0x477(%bx,%si),%bl
    56db:	03 35                	add    (%di),%si
    56dd:	8b 47 06             	mov    0x6(%bx),%ax
    56e0:	03 45 02             	add    0x2(%di),%ax
    56e3:	fa                   	cli
    56e4:	76 f8                	jbe    0x56de
    56e6:	89 0a                	mov    %cx,(%bp,%si)
    56e8:	40                   	inc    %ax
    56e9:	03 45 02             	add    0x2(%di),%ax
    56ec:	48                   	dec    %ax
    56ed:	89 f6                	mov    %si,%si
    56ef:	bf 83 0b             	mov    $0xb83,%di
    56f2:	c4 43 36             	les    0x36(%bp,%di),%ax
    56f5:	75 0f                	jne    0x5706
    56f7:	ff                   	(bad)
    56f8:	3e d6                	ds (bad)
    56fa:	c1 04 04             	rolw   $0x4,(%si)
    56fd:	76 f8                	jbe    0x56f7
    56ff:	40                   	inc    %ax
    5700:	de 8b 5e 37          	fimuls 0x375e(%bp,%di)
    5704:	eb 48                	jmp    0x574e
    5706:	80 7d 0f 36          	cmpb   $0x36,0xf(%di)
    570a:	75 42                	jne    0x574e
    570c:	8a 38                	mov    (%bx,%si),%bh
    570e:	73 01                	jae    0x5711
    5710:	8b eb                	mov    %bx,%bp
    5712:	b1 15                	mov    $0x15,%cl
    5714:	14 f2                	adc    $0xf2,%al
    5716:	b9 1d 00             	mov    $0x1d,%cx
    5719:	89 89 c1 c6          	mov    %cx,-0x393f(%bx,%di)
    571d:	73 74                	jae    0x5793
    571f:	ec                   	in     (%dx),%al
    5720:	94                   	xchg   %ax,%sp
    5721:	50                   	push   %ax
    5722:	21 14                	and    %dx,(%si)
    5724:	36 3b b8 c6 0c       	cmp    %ss:0xcc6(%bx,%si),%di
    5729:	18 e2                	sbb    %ah,%dl
    572b:	b8 04 72             	mov    $0x7204,%ax
    572e:	e5 8d                	in     $0x8d,%ax
    5730:	44                   	inc    %sp
    5731:	38 5e 83             	cmp    %bl,-0x7d(%bp)
    5734:	ea f8 8d 4c 07       	ljmp   $0x74c,$0x8df8
    5739:	51                   	push   %cx
    573a:	83 ea f8             	sub    $0xfff8,%dx
    573d:	8b f0                	mov    %ax,%si
    573f:	89 4e f4             	mov    %cx,-0xc(%bp)
    5742:	e8 17 3b             	call   0x925c
    5745:	56                   	push   %si
    5746:	b0 2b                	mov    $0x2b,%al
    5748:	e8 f4 21             	call   0x793f
    574b:	14 0e                	adc    $0xe,%al
    574d:	3b b8 c6 fb          	cmp    -0x43a(%bx,%si),%di
    5751:	8b f6                	mov    %si,%si
    5753:	36 53                	ss push %bx
    5755:	fb                   	sti
    5756:	a1 38 45             	mov    0x4538,%ax
    5759:	0e                   	push   %cs
    575a:	5e                   	pop    %si
    575b:	9a b1 6f 0c 18       	lcall  $0x180c,$0x6fb1
    5760:	7d 0f                	jge    0x5771
    5762:	36 74 67             	ss je  0x57cc
    5765:	83 0b c4             	orw    $0xffc4,(%bp,%di)
    5768:	43                   	inc    %bx
    5769:	36 0e                	ss push %cs
    576b:	60                   	pusha
    576c:	8b 5d 98             	mov    -0x68(%di),%bx
    576f:	04 35                	add    $0x35,%al
    5771:	47                   	inc    %di
    5772:	03 02                	add    (%bp,%si),%ax
    5774:	76 89                	jbe    0x56ff
    5776:	40                   	inc    %ax
    5777:	45                   	inc    %bp
    5778:	48                   	dec    %ax
    5779:	f6 83 c4 36 8a       	testb  $0x8a,0x36c4(%bp,%di)
    577e:	45                   	inc    %bp
    577f:	0e                   	push   %cs
    5780:	fe c8                	dec    %al
    5782:	b1 81                	mov    $0x81,%cl
    5784:	47                   	inc    %di
    5785:	e1 03                	loope  0x578a
    5787:	d8 89 40 45          	fmuls  0x4540(%bx,%di)
    578b:	48                   	dec    %ax
    578c:	f6 83 c4 36 0f       	testb  $0xf,0x36c4(%bp,%di)
    5791:	3e 8a 73 f6          	mov    %ds:-0xa(%bp,%di),%dh
    5795:	1b 46 46             	sbb    0x46(%bp),%ax
    5798:	56                   	push   %si
    5799:	83 ea f8             	sub    $0xfff8,%dx
    579c:	56                   	push   %si
    579d:	83 ea f6             	sub    $0xfff6,%dx
    57a0:	e8 b9 3a             	call   0x925c
    57a3:	b8 0c e2             	mov    $0xe20c,%ax
    57a6:	04 e5                	add    $0xe5,%al
    57a8:	44                   	inc    %sp
    57a9:	03 72 ea             	add    -0x16(%bp,%si),%si
    57ac:	8d 44 fb             	lea    -0x5(%si),%ax
    57af:	5e                   	pop    %si
    57b0:	ea 8d 07 83 f8       	ljmp   $0xf883,$0x78d
    57b5:	01 51 ea             	add    %dx,-0x16(%bx,%di)
    57b8:	8b 89 f4 17          	mov    0x17f4(%bx,%di),%cx
    57bc:	f6 b8 55 9b          	idivb  -0x64ab(%bx,%si)
    57c0:	3a 56 83             	cmp    -0x7d(%bp),%dl
    57c3:	ea f6 b0 2b e8       	ljmp   $0xe82b,$0xb0f6
    57c8:	f6 e8                	imul   %al
    57ca:	90                   	nop
    57cb:	3a 89 5e 3d          	cmp    0x3d5e(%bx,%di),%cl
    57cf:	57                   	push   %di
    57d0:	47                   	inc    %di
    57d1:	08 34                	or     %dh,(%si)
    57d3:	0f a3 3c             	bt     %di,(%si)
    57d6:	7c ff                	jl     0x57d7
    57d8:	77 0c                	ja     0x57e6
    57da:	57                   	push   %di
    57db:	47                   	inc    %di
    57dc:	47                   	inc    %di
    57dd:	03 07                	add    (%bx),%ax
    57df:	83 ba 00 f0 8a       	cmpw   $0xff8a,-0x1000(%bp,%si)
    57e4:	50                   	push   %ax
    57e5:	57                   	push   %di
    57e6:	47                   	inc    %di
    57e7:	02 8a 2a a3          	add    -0x5cd6(%bp,%si),%cl
    57eb:	de 8b f5 33          	fimuls 0x33f5(%bp,%di)
    57ef:	5e                   	pop    %si
    57f0:	46                   	inc    %si
    57f1:	5d                   	pop    %bp
    57f2:	31 89 fc 2b          	xor    %cx,0x2bfc(%bx,%di)
    57f6:	f8                   	clc
    57f7:	52                   	push   %dx
    57f8:	e8 e8 08             	call   0x60e3
    57fb:	04 36                	add    $0x36,%al
    57fd:	39 8b a1 ff          	cmp    %cx,-0x5f(%bp,%di)
    5801:	5e                   	pop    %si
    5802:	63 90 d3 80          	arpl   %dx,-0x7f2d(%bx,%si)
    5806:	7c 0f                	jl     0x5817
    5808:	36 74 2a             	ss je  0x5835
    580b:	8b 0a                	mov    (%bp,%si),%cx
    580d:	79 57                	jns    0x5866
    580f:	47                   	inc    %di
    5810:	47                   	inc    %di
    5811:	03 04                	add    (%si),%ax
    5813:	50                   	push   %ax
    5814:	57                   	push   %di
    5815:	4f                   	dec    %di
    5816:	40                   	inc    %ax
    5817:	45                   	inc    %bp
    5818:	4c                   	dec    %sp
    5819:	02 51 50             	add    0x50(%bx,%di),%dl
    581c:	00 fe                	add    %bh,%dh
    581e:	50                   	push   %ax
    581f:	44                   	inc    %sp
    5820:	50                   	push   %ax
    5821:	0e                   	push   %cs
    5822:	c8 81 e1 d8          	enter  $0xe181,$0xd8
    5826:	40                   	inc    %ax
    5827:	48                   	dec    %ax
    5828:	8b 04                	mov    (%si),%ax
    582a:	87 41 45             	xchg   %ax,0x45(%bx,%di)
    582d:	48                   	dec    %ax
    582e:	44                   	inc    %sp
    582f:	83 c4 49             	add    $0x49,%sp
    5832:	3f                   	aas
    5833:	e8 26 8d             	call   0xe55c
    5836:	11 06 8b 89          	adc    %ax,-0x7675
    583a:	08 ff                	or     %bh,%bh
    583c:	74 0c                	je     0x584a
    583e:	57                   	push   %di
    583f:	44                   	inc    %sp
    5840:	47                   	inc    %di
    5841:	03 04                	add    (%si),%ax
    5843:	83 00 8a             	addw   $0xff8a,(%bx,%si)
    5846:	57                   	push   %di
    5847:	02 2a                	add    (%bp,%si),%ch
    5849:	de 44 83             	fiadds -0x7d(%si)
    584c:	8a 2a                	mov    (%bp,%si),%ch
    584e:	a3 de 8b             	mov    %ax,0x8bde
    5851:	91                   	xchg   %ax,%cx
    5852:	33 46 31             	xor    0x31(%bp),%ax
    5855:	fc                   	cld
    5856:	f8                   	clc
    5857:	e8 08 36             	call   0x8e62
    585a:	8b ff                	mov    %di,%di
    585c:	63 d3                	arpl   %dx,%bx
    585e:	06                   	push   %es
    585f:	d3 00                	rolw   %cl,(%bx,%si)
    5861:	c9                   	leave
    5862:	ff 81 7e 06          	incw   0x67e(%bx,%di)
    5866:	77 ba                	ja     0x5822
    5868:	00 01                	add    %al,(%bx,%di)
    586a:	fa                   	cli
    586b:	8c 87 06 b2          	mov    %es,-0x4dfa(%bx)
    586f:	08 ff                	or     %bh,%bh
    5871:	de 30                	fidivs (%bx,%si)
    5873:	03 df                	add    %di,%bx
    5875:	c4 a1 9a 89          	les    -0x7666(%bx,%di),%sp
    5879:	f9                   	stc
    587a:	ff 00                	incw   (%bx,%si)
    587c:	e7 6e                	out    %ax,$0x6e
    587e:	0a 0e 66 06          	or     0x666,%cl
    5882:	88 8a f2 87          	mov    %cl,-0x780e(%bp,%si)
    5886:	fa                   	cli
    5887:	8b 0a                	mov    (%bp,%si),%cx
    5889:	03 80 03 74          	add    0x7403(%bx,%si),%ax
    588d:	8d 11                	lea    (%bx,%di),%dx
    588f:	06                   	push   %es
    5890:	8b 89 08 11          	mov    0x1108(%bx,%di),%cx
    5894:	8b 0a                	mov    (%bp,%si),%cx
    5896:	03 ff                	add    %di,%di
    5898:	03 8b 04 04          	add    0x404(%bp,%di),%cx
    589c:	0b 89 0b 16          	or     0x160b(%bx,%di),%cx
    58a0:	08 a1 9a 0c          	or     %ah,0xc9a(%bx,%di)
    58a4:	8d                   	lea    (bad),%di
    58a5:	fa                   	cli
    58a6:	08 ca                	or     %cl,%dl
    58a8:	fb                   	sti
    58a9:	76 8d                	jbe    0x5838
    58ab:	0f 11 3c             	movups %xmm7,(%si)
    58ae:	3c a1                	cmp    $0xa1,%al
    58b0:	8b 0a                	mov    (%bp,%si),%cx
    58b2:	79 03                	jns    0x58b7
    58b4:	87 fa                	xchg   %di,%dx
    58b6:	8b 47 47             	mov    0x47(%bx),%ax
    58b9:	03 04                	add    (%si),%ax
    58bb:	8b e4                	mov    %sp,%sp
    58bd:	eb 8a                	jmp    0x5849
    58bf:	4f                   	dec    %di
    58c0:	40                   	inc    %ax
    58c1:	45                   	inc    %bp
    58c2:	4c                   	dec    %sp
    58c3:	02 fe                	add    %dh,%bh
    58c5:	fc                   	cld
    58c6:	c1 ff 37             	sar    $0x37,%di
    58c9:	05 50 03             	add    $0x350,%ax
    58cc:	06                   	push   %es
    58cd:	51                   	push   %cx
    58ce:	e8 cb 30             	call   0x899c
    58d1:	83 c4 36             	add    $0x36,%sp
    58d4:	0f 3e                	(bad)
    58d6:	c1 1e f7 18 08       	rcrw   $0x8,0x18f7
    58db:	40                   	inc    %ax
    58dc:	40                   	inc    %ax
    58dd:	2a de                	sub    %dh,%bl
    58df:	46                   	inc    %si
    58e0:	fc                   	cld
    58e1:	03 0a                	add    (%bp,%si),%cx
    58e3:	e4 2a                	in     $0x2a,%al
    58e5:	de 46 fe             	fiadds -0x2(%bp)
    58e8:	8a 2a                	mov    (%bp,%si),%ch
    58ea:	a3 de de             	mov    %ax,0xdede
    58ed:	46                   	inc    %si
    58ee:	fc                   	cld
    58ef:	03 89 4e 9e          	add    -0x61b2(%bx,%di),%cx
    58f3:	8e 7e 25             	mov    0x25(%bp),%?
    58f6:	8b 91 c4 89          	mov    -0x763c(%bx,%di),%dx
    58fa:	e8 76 00             	call   0x5973
    58fd:	01 24                	add    %sp,(%si)
    58ff:	02 89 8e 0c          	add    0xc8e(%bx,%di),%cl
    5903:	8e 09                	mov    (%bx,%di),%cs
    5905:	3b fe                	cmp    %si,%di
    5907:	89 8a ff 7e          	mov    %cx,0x7eff(%bp,%si)
    590b:	06                   	push   %es
    590c:	36 8b ff             	ss mov %di,%di
    590f:	f8                   	clc
    5910:	52                   	push   %dx
    5911:	e8 2a f9             	call   0x523e
    5914:	4b                   	dec    %bx
    5915:	e8 43 ff             	call   0x585b
    5918:	80 3e a2 43 0e       	cmpb   $0xe,0x43a2
    591d:	8b 5e e8             	mov    -0x18(%bp),%bx
    5920:	8c a8 3d 0a          	mov    %gs,0xa3d(%bx,%si)
    5924:	00 72 56             	add    %dh,0x56(%bp,%si)
    5927:	e8 ce a7             	call   0xf8
    592a:	fa                   	cli
    592b:	6e                   	outsb  %ds:(%si),(%dx)
    592c:	9c                   	pushf
    592d:	5b                   	pop    %bx
    592e:	83 54 29 ff          	adcw   $0xffff,0x29(%si)
    5932:	80 3e a2 43 08       	cmpb   $0x8,0x43a2
    5937:	74 07                	je     0x5940
    5939:	80 3e a2 43 02       	cmpb   $0x2,0x43a2
    593e:	75 0c                	jne    0x594c
    5940:	1d fe a0             	sbb    $0xa0fe,%ax
    5943:	07                   	pop    %es
    5944:	8a 0e c8 e9          	mov    -0x1638,%cl
    5948:	13 53 31             	adc    0x31(%bp,%di),%dx
    594b:	4e                   	dec    %si
    594c:	80 3e a2 43 04       	cmpb   $0x4,0x43a2
    5951:	07                   	pop    %es
    5952:	2b 80 3e a2          	sub    -0x5dc2(%bx,%si),%ax
    5956:	43                   	inc    %bx
    5957:	01 75 0c             	add    %si,0xc(%di)
    595a:	8a 0e c8 e9          	mov    -0x1638,%cl
    595e:	13 46 b8             	adc    -0x48(%bp),%ax
    5961:	75 9f                	jne    0x5902
    5963:	02 2b                	add    (%bp,%di),%ch
    5965:	f6 f8                	idiv   %al
    5967:	e8 f9 e8             	call   0x4263
    596a:	ff                   	(bad)
    596b:	3e 43                	ds inc %bx
    596d:	ec                   	in     (%dx),%al
    596e:	fe 80 3e a5          	incb   -0x5ac2(%bx,%si)
    5972:	0f 3e                	(bad)
    5974:	c1 07 a0             	rolw   $0xa0,(%bx)
    5977:	a3 00 30             	mov    %ax,0x3000
    597a:	06                   	push   %es
    597b:	a2 43 e8             	mov    %al,0xe843
    597e:	2a aa 29 2a          	sub    0x2a29(%bp,%si),%ch
    5982:	b0 e9                	mov    $0xe9,%al
    5984:	13 3d                	adc    (%di),%di
    5986:	2b 00                	sub    (%bx,%si),%ax
    5988:	40                   	inc    %ax
    5989:	e5 77                	in     $0x77,%ax
    598b:	10 0e a2 fe          	adc    %cl,-0x15e
    598f:	88 2c                	mov    %ch,(%si)
    5991:	0d 74 36             	or     $0x3674,%ax
    5994:	2c 03                	sub    $0x3,%al
    5996:	74 32                	je     0x59ca
    5998:	2c 10                	sub    $0x10,%al
    599a:	74 2e                	je     0x59ca
    599c:	57                   	push   %di
    599d:	47                   	inc    %di
    599e:	43                   	inc    %bx
    599f:	ff                   	(bad)
    59a0:	3e 43                	ds inc %bx
    59a2:	ef                   	out    %ax,(%dx)
    59a3:	fb                   	sti
    59a4:	76 76                	jbe    0x5a1c
    59a6:	47                   	inc    %di
    59a7:	3d 43 01             	cmp    $0x143,%ax
    59aa:	75 03                	jne    0x59af
    59ac:	e9 69 ff             	jmp    0x5918
    59af:	89 8b ff f8          	mov    %cx,-0x701(%bp,%di)
    59b3:	52                   	push   %dx
    59b4:	9c                   	pushf
    59b5:	5b                   	pop    %bx
    59b6:	83 54 a1 fe          	adcw   $0xfffe,-0x5f(%si)
    59ba:	b0 e8                	mov    $0xe8,%al
    59bc:	e8 fc f9             	call   0x53bb
    59bf:	e8 ff 3e             	call   0x98c1
    59c2:	43                   	inc    %bx
    59c3:	96                   	xchg   %ax,%si
    59c4:	fe                   	(bad)
    59c5:	de 46 fc             	fiadds -0x4(%bp)
    59c8:	eb 8b                	jmp    0x5955
    59ca:	83 f7 fe             	xor    $0xfffe,%di
    59cd:	08 a1 ff 5e          	or     %ah,0x5eff(%bx,%di)
    59d1:	5e                   	pop    %si
    59d2:	0c 04                	or     $0x4,%al
    59d4:	f6 8d fa 08 ca       	testb  $0xca,0x8fa(%di)
    59d9:	10 08                	adc    %cl,(%bx,%si)
    59db:	83 4c 04 15          	orw    $0x15,0x4(%si)
    59df:	26 5e                	es pop %si
    59e1:	03 e9                	add    %cx,%bp
    59e3:	ad                   	lods   %ds:(%si),%ax
    59e4:	f9                   	stc
    59e5:	80 fd d6             	cmp    $0xd6,%ch
    59e8:	c2 6a c3             	ret    $0xc36a
    59eb:	57                   	push   %di
    59ec:	03 de                	add    %si,%bx
    59ee:	40                   	inc    %ax
    59ef:	be 6d 0f             	mov    $0xf6d,%si
    59f2:	e8 63 32             	call   0x8c58
    59f5:	24 52                	and    $0x52,%al
    59f7:	15 f0 95             	adc    $0x95f0,%ax
    59fa:	b8 9f e7             	mov    $0xe79f,%ax
    59fd:	b9 01 63             	mov    $0x6301,%cx
    5a00:	00 51 8d             	add    %dl,-0x73(%bx,%di)
    5a03:	4e                   	dec    %si
    5a04:	f0 51                	lock push %cx
    5a06:	43                   	inc    %bx
    5a07:	96                   	xchg   %ax,%si
    5a08:	32 8d 7e 89          	xor    -0x7682(%di),%cl
    5a0c:	17                   	pop    %ss
    5a0d:	b8 16 07             	mov    $0x716,%ax
    5a10:	a5                   	movsw  %ds:(%si),%es:(%di)
    5a11:	a5                   	movsw  %ds:(%si),%es:(%di)
    5a12:	a5                   	movsw  %ds:(%si),%es:(%di)
    5a13:	a5                   	movsw  %ds:(%si),%es:(%di)
    5a14:	83 6e f8 07          	subw   $0x7,-0x8(%bp)
    5a18:	ea 89 17 83 6e       	ljmp   $0x6e83,$0x1789
    5a1d:	fa                   	cli
    5a1e:	07                   	pop    %es
    5a1f:	ea 89 56 47 02       	ljmp   $0x247,$0x5689
    5a24:	fc                   	cld
    5a25:	07                   	pop    %es
    5a26:	ea 89 fc 20 e7       	ljmp   $0xe720,$0xfc89
    5a2b:	47                   	inc    %di
    5a2c:	07                   	pop    %es
    5a2d:	ea 89 fe e8 e8       	ljmp   $0xe8e8,$0xfe89
    5a32:	0e                   	push   %cs
    5a33:	ea 89 17 13 31       	ljmp   $0x3113,$0x1789
    5a38:	76 bf                	jbe    0x59f9
    5a3a:	47                   	inc    %di
    5a3b:	ff                   	ljmp   (bad)
    5a3c:	ea 89 fe ff 00       	ljmp   $0xff,$0xfe89
    5a41:	8d                   	lea    (bad),%bx
    5a42:	da 0e ee d6          	fimull -0x2912
    5a46:	63 90 d3 a7          	arpl   %dx,-0x582d(%bx,%si)
    5a4a:	37                   	aaa
    5a4b:	e8 57 03             	call   0x5da5
    5a4e:	de de                	(bad)
    5a50:	46                   	inc    %si
    5a51:	fc                   	cld
    5a52:	89 e8                	mov    %bp,%ax
    5a54:	89 83 8a 02          	mov    %ax,0x28a(%bp,%di)
    5a58:	de 83 2a de          	fiadds -0x21d6(%bp,%di)
    5a5c:	46                   	inc    %si
    5a5d:	fa                   	cli
    5a5e:	f7 ff                	idiv   %di
    5a60:	68 d8 7f             	push   $0x7fd8
    5a63:	4d                   	dec    %bp
    5a64:	31 19                	xor    %bx,(%bx,%di)
    5a66:	00 b6 d1 2d          	add    %dh,0x2dd1(%bp)
    5a6a:	19 ec                	sbb    %bp,%sp
    5a6c:	be 6d 0f             	mov    $0xf6d,%si
    5a6f:	b8 9f e7             	mov    $0xe79f,%ax
    5a72:	b9 de fe             	mov    $0xfede,%cx
    5a75:	2a 2d                	sub    (%di),%ch
    5a77:	2d 19 d8             	sub    $0xd819,%ax
    5a7a:	7f 67                	jg     0x5ae3
    5a7c:	31 e8                	xor    %bp,%ax
    5a7e:	79 db                	jns    0x5a5b
    5a80:	ea 17 31 bf ff       	ljmp   $0xffbf,$0x3117
    5a85:	89 ff                	mov    %di,%di
    5a87:	8d 0e d6 90          	lea    -0x6f2a,%cx
    5a8b:	a7                   	cmpsw  %es:(%di),%ds:(%si)
    5a8c:	e8 e4 0e             	call   0x6973
    5a8f:	e8 f8 c2             	call   0x1d8a
    5a92:	46                   	inc    %si
    5a93:	fc                   	cld
    5a94:	e8 36 ff             	call   0x59cd
    5a97:	d3 d3                	rcl    %cl,%bx
    5a99:	c9                   	leave
    5a9a:	81 06 ba 56 4c e6    	addw   $0xe64c,0x56ba
    5aa0:	8a 83 40 0d          	mov    0xd40(%bp,%di),%al
    5aa4:	05 8b 43             	add    $0x438b,%ax
    5aa7:	5b                   	pop    %bx
    5aa8:	be 4a 05             	mov    $0x54a,%si
    5aab:	5f                   	pop    %di
    5aac:	90                   	nop
    5aad:	83 40 0d 05          	addw   $0x5,0xd(%bx,%si)
    5ab1:	3f                   	aas
    5ab2:	43                   	inc    %bx
    5ab3:	5b                   	pop    %bx
    5ab4:	be 90 83             	mov    $0x8390,%si
    5ab7:	40                   	inc    %ax
    5ab8:	0d 05 18             	or     $0x1805,%ax
    5abb:	8b f9                	mov    %cx,%di
    5abd:	c6                   	(bad)
    5abe:	ef                   	out    %ax,(%dx)
    5abf:	0e                   	push   %cs
    5ac0:	0c 04                	or     $0x4,%al
    5ac2:	f6 8d fa 56 2b       	testb  $0x2b,0x56fa(%di)
    5ac7:	f6 eb                	imul   %bl
    5ac9:	0e                   	push   %cs
    5aca:	b8 02 3e             	mov    $0x3e02,%ax
    5acd:	79 45                	jns    0x5b14
    5acf:	b8 0d 2a             	mov    $0x2a0d,%ax
    5ad2:	fd                   	std
    5ad3:	d6                   	(bad)
    5ad4:	c2 0d 00             	ret    $0xd
    5ad7:	46                   	inc    %si
    5ad8:	39 b9 3e 77          	cmp    %di,0x773e(%bx,%di)
    5adc:	ed                   	in     (%dx),%ax
    5add:	46                   	inc    %si
    5ade:	05 95 52             	add    $0x5295,%ax
    5ae1:	b0 d8                	mov    $0xd8,%al
    5ae3:	08 c7                	or     %al,%bh
    5ae5:	00 89 81 ec          	add    %cl,-0x137f(%bx,%di)
    5ae9:	46                   	inc    %si
    5aea:	01 00                	add    %ax,(%bx,%si)
    5aec:	e8 19 63             	call   0xbe08
    5aef:	fe 05                	incb   (%di)
    5af1:	d1 c7                	rol    $1,%di
    5af3:	89 d4                	mov    %dx,%sp
    5af5:	7e 24                	jle    0x5b1b
    5af7:	8b 3e 5c 9b          	mov    -0x64a4,%di
    5afb:	81 e7 ff 00          	and    $0xff,%di
    5aff:	71 ff                	jno    0x5b00
    5b01:	e8 a3 46             	call   0xa1a7
    5b04:	03 f8                	add    %ax,%di
    5b06:	4f                   	dec    %di
    5b07:	de fe                	fdivrp %st,%st(6)
    5b09:	06                   	push   %es
    5b0a:	04 89                	add    $0x89,%al
    5b0c:	16                   	push   %ss
    5b0d:	8d 45 56             	lea    0x56(%di),%ax
    5b10:	a3 76 3e             	mov    %ax,0x3e76
    5b13:	80 0c b8             	orb    $0xb8,(%si)
    5b16:	00 74 2c             	add    %dh,0x2c(%si)
    5b19:	88 11                	mov    %dl,(%bx,%di)
    5b1b:	fe                   	(bad)
    5b1c:	19 86 ba fe          	sbb    %ax,-0x146(%bp)
    5b20:	a2 72 0f             	mov    %al,0xf72
    5b23:	b8 fe 81             	mov    $0x81fe,%ax
    5b26:	9f                   	lahf
    5b27:	04 6f                	add    $0x6f,%al
    5b29:	08 8d 86 bb          	or     %cl,-0x447a(%di)
    5b2d:	fe                   	(bad)
    5b2e:	d6                   	(bad)
    5b2f:	c2 bc 43             	ret    $0x43bc
    5b32:	06                   	push   %es
    5b33:	28 fe                	sub    %bh,%dh
    5b35:	b8 72 0f             	mov    $0xf72,%ax
    5b38:	50                   	push   %ax
    5b39:	8d 86 ba fe          	lea    -0x146(%bp),%ax
    5b3d:	d6                   	(bad)
    5b3e:	c2 b1 42             	ret    $0x42b1
    5b41:	5b                   	pop    %bx
    5b42:	eb 1d                	jmp    0x5b61
    5b44:	12 1e 01 b0          	adc    -0x4fff,%bl
    5b48:	9f                   	lahf
    5b49:	6f                   	outsw  %ds:(%si),(%dx)
    5b4a:	8d bb d6 bc          	lea    -0x432a(%bp,%di),%di
    5b4e:	ba fe d6             	mov    $0xd6fe,%dx
    5b51:	c2 9a 43             	ret    $0x439a
    5b54:	06                   	push   %es
    5b55:	28 fe                	sub    %bh,%dh
    5b57:	b8 6c 0f             	mov    $0xf6c,%ax
    5b5a:	8d ba d6 b1          	lea    -0x4e2a(%bp,%si),%di
    5b5e:	5b                   	pop    %bx
    5b5f:	1d 1e 45             	sbb    $0x451e,%ax
    5b62:	45                   	inc    %bp
    5b63:	5b                   	pop    %bx
    5b64:	e4 9a                	in     $0x9a,%al
    5b66:	eb ff                	jmp    0x5b67
    5b68:	03 bc 03 7e          	add    0x7e03(%si),%di
    5b6c:	30 fe                	xor    %bh,%dh
    5b6e:	e9 9e 8b             	jmp    0xe70f
    5b71:	e8 02 e3             	call   0x3e76
    5b74:	30 03                	xor    %al,(%bp,%di)
    5b76:	85 e8                	test   %bp,%ax
    5b78:	3d ba 00             	cmp    $0xba,%ax
    5b7b:	77 04                	ja     0x5b81
    5b7d:	1d a0 8a             	sbb    $0x8aa0,%ax
    5b80:	54                   	push   %sp
    5b81:	a1 d8 c1             	mov    0xc1d8,%ax
    5b84:	03 c7                	add    %di,%ax
    5b86:	3d be 00             	cmp    $0xbe,%ax
    5b89:	76 33                	jbe    0x5bbe
    5b8b:	52                   	push   %dx
    5b8c:	a1 b8 56             	mov    0x56b8,%ax
    5b8f:	50                   	push   %ax
    5b90:	b8 74 0f             	mov    $0xf74,%ax
    5b93:	8d b8 78 e7          	lea    -0x1888(%bx,%si),%di
    5b97:	01 00                	add    %ax,(%bx,%si)
    5b99:	56                   	push   %si
    5b9a:	22 40 20             	and    0x20(%bx,%si),%al
    5b9d:	6f                   	outsw  %ds:(%si),(%dx)
    5b9e:	39 29                	cmp    %bp,(%bx,%di)
    5ba0:	3e d8 c1             	ds fadd %st(1),%st
    5ba3:	a1 89 ff             	mov    0xff89,%ax
    5ba6:	e7 0a                	out    %ax,$0xa
    5ba8:	66 88 78 e7          	data32 mov %bh,-0x19(%bx,%si)
    5bac:	01 ff                	add    %di,%di
    5bae:	36 d8 c1             	ss fadd %st(1),%st
    5bb1:	b8 31 b0             	mov    $0xb031,%ax
    5bb4:	9f                   	lahf
    5bb5:	a1 d8 c1             	mov    0xc1d8,%ax
    5bb8:	03 c7                	add    %di,%ax
    5bba:	1d 1e b6             	sbb    $0xb61e,%ax
    5bbd:	22 8d 03 bc          	and    -0x43fd(%di),%cl
    5bc1:	01 00                	add    %ax,(%bx,%si)
    5bc3:	78 e7                	js     0x5bac
    5bc5:	51                   	push   %cx
    5bc6:	ff 36 d8 c1          	push   -0x3e28
    5bca:	e8 cf 2d             	call   0x899c
    5bcd:	01 3e d8 c1          	add    %di,-0x3e28
    5bd1:	ff 03                	incw   (%bp,%di)
    5bd3:	bc 03 1d             	mov    $0x1d03,%sp
    5bd6:	a0 8a c8             	mov    0xc88a,%al
    5bd9:	19 63 fe             	sbb    %sp,-0x2(%bp,%di)
    5bdc:	b8 eb b8             	mov    $0xb8eb,%ax
    5bdf:	2f                   	das
    5be0:	56                   	push   %si
    5be1:	8d 03                	lea    (%bp,%di),%ax
    5be3:	bc 01 e8             	mov    $0xe801,%sp
    5be6:	0a 42 5b             	or     0x5b(%bp,%si),%al
    5be9:	eb b8                	jmp    0x5ba3
    5beb:	6e                   	outsb  %ds:(%si),(%dx)
    5bec:	0f 8d ba 03          	jge    0x5faa
    5bf0:	bc 01 e8             	mov    $0xe801,%sp
    5bf3:	fd                   	std
    5bf4:	41                   	inc    %cx
    5bf5:	5b                   	pop    %bx
    5bf6:	9c                   	pushf
    5bf7:	4d                   	dec    %bp
    5bf8:	2a bc 01 e8          	sub    -0x17ff(%si),%bh
    5bfc:	5a                   	pop    %dx
    5bfd:	30 76 ee             	xor    %dh,-0x12(%bp)
    5c00:	76 fe                	jbe    0x5c00
    5c02:	70 3e                	jo     0x5c42
    5c04:	45                   	inc    %bp
    5c05:	0d fd c2             	or     $0xc2fd,%ax
    5c08:	e8 9d 44             	call   0xa0a8
    5c0b:	5b                   	pop    %bx
    5c0c:	e4 9a                	in     $0x9a,%al
    5c0e:	eb 76                	jmp    0x5c86
    5c10:	fa                   	cli
    5c11:	75 b9                	jne    0x5bcc
    5c13:	8b 03                	mov    (%bp,%di),%ax
    5c15:	e9 59 83             	jmp    0xdf71
    5c18:	56                   	push   %si
    5c19:	2e 89 76 d1          	mov    %si,%cs:-0x2f(%bp)
    5c1d:	24 89                	and    $0x89,%al
    5c1f:	0c 09                	or     $0x9,%al
    5c21:	fe 8a 7e 36          	decb   0x367e(%bp,%si)
    5c25:	0a 8c b9 0d          	or     0xdb9(%si),%cl
    5c29:	fd                   	std
    5c2a:	c2 e8 27             	ret    $0x27e8
    5c2d:	c1 4a 5f 83          	rorw   $0x83,0x5f(%bp,%si)
    5c31:	0d 4a f0             	or     $0xf04a,%ax
    5c34:	b8 e7 15             	mov    $0x15e7,%ax
    5c37:	95                   	xchg   %ax,%bp
    5c38:	9f                   	lahf
    5c39:	b9 9f e7             	mov    $0xe79f,%cx
    5c3c:	51                   	push   %cx
    5c3d:	01 00                	add    %ax,(%bx,%si)
    5c3f:	8d                   	lea    (bad),%si
    5c40:	f0 43                	lock inc %bx
    5c42:	32 f6                	xor    %dh,%dh
    5c44:	40                   	inc    %ax
    5c45:	20 57 30             	and    %dl,0x30(%bx)
    5c48:	ba 5a 3e             	mov    $0x3e5a,%dx
    5c4b:	8b fa                	mov    %dx,%di
    5c4d:	9a eb 1e 07 a5       	lcall  $0xa507,$0x1eeb
    5c52:	a5                   	movsw  %ds:(%si),%es:(%di)
    5c53:	6e                   	outsb  %ds:(%si),(%dx)
    5c54:	07                   	pop    %es
    5c55:	89 06 5e 3e          	mov    %ax,0x3e5e
    5c59:	07                   	pop    %es
    5c5a:	a1 5e 3e             	mov    0x3e5e,%ax
    5c5d:	83 2e 5a 3e 07       	subw   $0x7,0x3e5a
    5c62:	8b 0e 5a 3e          	mov    0x3e5a,%cx
    5c66:	2b c1                	sub    %cx,%ax
    5c68:	83 ba 00 f0 8a       	cmpw   $0xff8a,-0x1000(%bp,%si)
    5c6d:	75 d0                	jne    0x5c3f
    5c6f:	2d 1e 00             	sub    $0x1e,%ax
    5c72:	a3 46 3e             	mov    %ax,0x3e46
    5c75:	8b c2                	mov    %dx,%ax
    5c77:	4a                   	dec    %dx
    5c78:	4a                   	dec    %dx
    5c79:	89 16 4a 3e          	mov    %dx,0x3e4a
    5c7d:	8b d0                	mov    %ax,%dx
    5c7f:	8a a3 de a3          	mov    -0x5c22(%bp,%di),%ah
    5c83:	52                   	push   %dx
    5c84:	3e 83 c2 1e          	ds add $0x1e,%dx
    5c88:	89 16 56 3e          	mov    %dx,0x3e56
    5c8c:	83 06 60 3e 07       	addw   $0x7,0x3e60
    5c91:	a1 60 3e             	mov    0x3e60,%ax
    5c94:	83 2e 5c 3e 07       	subw   $0x7,0x3e5c
    5c99:	8b 16 5c 3e          	mov    0x3e5c,%dx
    5c9d:	ba 00 8b             	mov    $0x8b00,%dx
    5ca0:	d8 2d                	fsubrs (%di)
    5ca2:	10 de                	adc    %bl,%dh
    5ca4:	a3 54 3e             	mov    %ax,0x3e54
    5ca7:	a3 48 3e             	mov    %ax,0x3e48
    5caa:	8d 47 fa             	lea    -0x6(%bx),%ax
    5cad:	a3 58 3e             	mov    %ax,0x3e58
    5cb0:	a3 4c 3e             	mov    %ax,0x3e4c
    5cb3:	51                   	push   %cx
    5cb4:	06                   	push   %es
    5cb5:	95                   	xchg   %ax,%bp
    5cb6:	71 5e                	jno    0x5d16
    5cb8:	3e 95                	ds xchg %ax,%bp
    5cba:	71 60                	jno    0x5d1c
    5cbc:	3e e8 5b 0c          	ds call 0x691b
    5cc0:	b8 5a 3e             	mov    $0x3e5a,%ax
    5cc3:	9f                   	lahf
    5cc4:	b9 02 22             	mov    $0x2202,%cx
    5cc7:	20 39                	and    %bh,(%bx,%di)
    5cc9:	49                   	dec    %cx
    5cca:	f9                   	stc
    5ccb:	19 b6 2d ec          	sbb    %si,-0x13d3(%bp)
    5ccf:	6d                   	insw   (%dx),%es:(%di)
    5cd0:	b8 e7 39             	mov    $0x39e7,%ax
    5cd3:	a1 5e 5e             	mov    0x5e5e,%ax
    5cd6:	3e 03 06 5a 3e       	add    %ds:0x3e5a,%ax
    5cdb:	83 02 83             	addw   $0xff83,(%bp,%si)
    5cde:	de fa                	fdivrp %st,%st(2)
    5ce0:	ff a1 5c 3e          	jmp    *0x3e5c(%bx,%di)
    5ce4:	f7 68 7f             	imulw  0x7f(%bx,%si)
    5ce7:	31 00                	xor    %ax,(%bx,%si)
    5ce9:	f9                   	stc
    5cea:	2e b8 5a 3e          	cs mov $0x3e5a,%ax
    5cee:	9f                   	lahf
    5cef:	b9 15 95             	mov    $0x9515,%cx
    5cf2:	20 39                	and    %bh,(%bx,%di)
    5cf4:	d2 f7                	shl    %cl,%bh
    5cf6:	8b eb                	mov    %bx,%bp
    5cf8:	4f                   	dec    %di
    5cf9:	95                   	xchg   %ax,%bp
    5cfa:	71 5a                	jno    0x5d56
    5cfc:	3e 95                	ds xchg %ax,%bp
    5cfe:	71 5c                	jno    0x5d5c
    5d00:	3e 95                	ds xchg %ax,%bp
    5d02:	5e                   	pop    %si
    5d03:	95                   	xchg   %ax,%bp
    5d04:	60                   	pusha
    5d05:	e8 0c 5a             	call   0xb714
    5d08:	9f                   	lahf
    5d09:	02 67 0c             	add    0xc(%bx),%ah
    5d0c:	e8 7b c0             	call   0x1d8a
    5d0f:	83 7e fe 01          	cmpw   $0x1,-0x2(%bp)
    5d13:	1b c0                	sbb    %ax,%ax
    5d15:	f7 d8                	neg    %ax
    5d17:	46                   	inc    %si
    5d18:	e8 ff d3             	call   0x311a
    5d1b:	81 ba 4c 8a 00 c6    	cmpw   $0xc600,-0x75b4(%bp,%si)
    5d21:	06                   	push   %es
    5d22:	ee                   	out    %al,(%dx)
    5d23:	60                   	pusha
    5d24:	e8 e8 23             	call   0x810f
    5d27:	aa                   	stos   %al,%es:(%di)
    5d28:	05 08 a0             	add    $0xa008,%ax
    5d2b:	ff 11                	call   *(%bx,%di)
    5d2d:	19 2f                	sbb    %bp,(%bx)
    5d2f:	8e e4                	mov    %sp,%fs
    5d31:	b0 1d                	mov    $0x1d,%al
    5d33:	e8 3d fe             	call   0x5b73
    5d36:	e4 a2                	in     $0xa2,%al
    5d38:	11 38                	adc    %di,(%bx,%si)
    5d3a:	06                   	push   %es
    5d3b:	ee                   	out    %al,(%dx)
    5d3c:	60                   	pusha
    5d3d:	74 2e                	je     0x5d6d
    5d3f:	fe                   	(bad)
    5d40:	fe 0b                	decb   (%bp,%di)
    5d42:	d7                   	xlat   %ds:(%bx)
    5d43:	73 b0                	jae    0x5cf5
    5d45:	e4 04                	in     $0x4,%al
    5d47:	73 24                	jae    0x5d6d
    5d49:	ff 12                	call   *(%bp,%si)
    5d4b:	ff                   	(bad)
    5d4c:	ff                   	ljmp   (bad)
    5d4d:	e8 81 5e             	call   0xbbd1
    5d50:	5e                   	pop    %si
    5d51:	f4                   	hlt
    5d52:	e4 a2                	in     $0xa2,%al
    5d54:	11 38                	adc    %di,(%bx,%si)
    5d56:	87 b3 a6 75          	xchg   %si,0x75a6(%bp,%di)
    5d5a:	0d ff b7             	or     $0xb7ff,%ax
    5d5d:	b0 a6                	mov    $0xa6,%al
    5d5f:	e8 cc a9             	call   0x72e
    5d62:	76 8b                	jbe    0x5cef
    5d64:	04 a2                	add    $0xa2,%al
    5d66:	ee                   	out    %al,(%dx)
    5d67:	60                   	pusha
    5d68:	bf da 75             	mov    $0x75da,%di
    5d6b:	20 d6                	and    %dl,%dh
    5d6d:	46                   	inc    %si
    5d6e:	31 fc                	xor    %di,%sp
    5d70:	b8 ee 03             	mov    $0x3ee,%ax
    5d73:	08 04                	or     %al,(%si)
    5d75:	36 39 8b c7 fe       	cmp    %cx,%ss:-0x139(%bp,%di)
    5d7a:	fe 0b                	decb   (%bp,%di)
    5d7c:	ff                   	(bad)
    5d7d:	fe                   	(bad)
    5d7e:	fe                   	(bad)
    5d7f:	fe                   	(bad)
    5d80:	d7                   	xlat   %ds:(%bx)
    5d81:	73 b0                	jae    0x5d33
    5d83:	fe 04                	incb   (%si)
    5d85:	73 24                	jae    0x5dab
    5d87:	ff 12                	call   *(%bp,%si)
    5d89:	fe                   	(bad)
    5d8a:	ff 81 5e e4          	incw   -0x1ba2(%bx,%di)
    5d8e:	11 87 80 bf          	adc    %ax,-0x4080(%bx)
    5d92:	b2 a6                	mov    $0xa6,%dl
    5d94:	00 74 09             	add    %dh,0x9(%si)
    5d97:	e4 11                	in     $0x11,%al
    5d99:	87 a6 0d b7          	xchg   %sp,-0x48f3(%bp)
    5d9d:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    5d9e:	74 05                	je     0x5da5
    5da0:	fb                   	sti
    5da1:	46                   	inc    %si
    5da2:	fe                   	(bad)
    5da3:	eb dc                	jmp    0x5d81
    5da5:	e4 11                	in     $0x11,%al
    5da7:	fe                   	(bad)
    5da8:	29 b0 13 73          	sub    %si,0x7313(%bx,%si)
    5dac:	b0 e4                	mov    $0xe4,%al
    5dae:	ff 75 63             	push   0x63(%di)
    5db1:	fe                   	(bad)
    5db2:	fe                   	(bad)
    5db3:	73 fe                	jae    0x5db3
    5db5:	73 ff                	jae    0x5db6
    5db7:	fe 81 e4 1b          	incb   0x1be4(%bx,%di)
    5dbb:	ff                   	(bad)
    5dbc:	fe 81 e4 87          	incb   -0x781c(%bx,%di)
    5dc0:	bf a6 74             	mov    $0x74a6,%di
    5dc3:	e4 87                	in     $0x87,%al
    5dc5:	0d a6 05             	or     $0x5a6,%ax
    5dc8:	46                   	inc    %si
    5dc9:	eb 05                	jmp    0x5dd0
    5dcb:	fb                   	sti
    5dcc:	46                   	inc    %si
    5dcd:	fe                   	(bad)
    5dce:	eb e5                	jmp    0x5db5
    5dd0:	e4 fe                	in     $0xfe,%al
    5dd2:	b0 73                	mov    $0x73,%al
    5dd4:	e4 75                	in     $0x75,%al
    5dd6:	fe                   	(bad)
    5dd7:	73 73                	jae    0x5e4c
    5dd9:	fe                   	(bad)
    5dda:	74 38                	je     0x5e14
    5ddc:	88 3b                	mov    %bh,(%bp,%di)
    5dde:	74 3e                	je     0x5e1e
    5de0:	48                   	dec    %ax
    5de1:	88 36 d0 fe          	mov    %dh,-0x130
    5de5:	e4 12                	in     $0x12,%al
    5de7:	ff 81 5e e4          	incw   -0x1ba2(%bx,%di)
    5deb:	11 87 a6 89          	adc    %ax,-0x765a(%bx)
    5def:	87 b0 a6 03          	xchg   %si,0x3a6(%bx,%si)
    5df3:	a1 8b 1e             	mov    0x1e8b,%ax
    5df6:	88 3b                	mov    %bh,(%bp,%di)
    5df8:	74 ff                	je     0x5df9
    5dfa:	81 87 a6 e4 0d 05    	addw   $0x50d,-0x1b5a(%bx)
    5e00:	eb fb                	jmp    0x5dfd
    5e02:	88 0d                	mov    %cl,(%di)
    5e04:	b7 a6                	mov    $0xa6,%bh
    5e06:	e4 ff                	in     $0xff,%al
    5e08:	5e                   	pop    %si
    5e09:	11 a6 87 a6          	adc    %sp,-0x5979(%bp)
    5e0d:	a1 1e c6             	mov    0xc61e,%ax
    5e10:	87 a6 05 8b          	xchg   %sp,-0x74fb(%bp)
    5e14:	f9                   	stc
    5e15:	c6                   	(bad)
    5e16:	ef                   	out    %ax,(%dx)
    5e17:	0e                   	push   %cs
    5e18:	ea e8 89 76 d1       	ljmp   $0xd176,$0x89e8
    5e1d:	be 24 eb             	mov    $0xeb24,%si
    5e20:	80 fe fe             	cmp    $0xfe,%dh
    5e23:	73 ff                	jae    0x5e24
    5e25:	fe                   	(bad)
    5e26:	fe 0b                	decb   (%bp,%di)
    5e28:	73 e4                	jae    0x5e0e
    5e2a:	73 ff                	jae    0x5e2b
    5e2c:	ff                   	ljmp   (bad)
    5e2d:	e8 5e f4             	call   0x528e
    5e30:	a2 38 b3             	mov    %al,0xb338
    5e33:	75 ff                	jne    0x5e34
    5e35:	b0 e8                	mov    $0xe8,%al
    5e37:	a9 80 b2             	test   $0xb280,%ax
    5e3a:	00 09                	add    %cl,(%bx,%di)
    5e3c:	11 a6 b7 74          	adc    %sp,0x74b7(%bp)
    5e40:	fb                   	sti
    5e41:	fe                   	(bad)
    5e42:	dc 11                	fcoml  (%bx,%di)
    5e44:	29 13                	sub    %dx,(%bp,%di)
    5e46:	b0 ff                	mov    $0xff,%al
    5e48:	63 fe                	arpl   %di,%si
    5e4a:	75 20                	jne    0x5e6c
    5e4c:	dc e4                	fsub   %st,%st(4)
    5e4e:	11 ff                	adc    %di,%di
    5e50:	73 e4                	jae    0x5e36
    5e52:	fe                   	(bad)
    5e53:	73 ff                	jae    0x5e54
    5e55:	fe                   	(bad)
    5e56:	ff 26 16 ff          	jmp    *-0xea
    5e5a:	87 e4                	xchg   %sp,%sp
    5e5c:	05 fb 0d             	add    $0xdfb,%ax
    5e5f:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    5e60:	ff 11                	call   *(%bx,%di)
    5e62:	ff b0 e8 a9          	push   -0x5618(%bx,%si)
    5e66:	8b f3                	mov    %bx,%si
    5e68:	e8 21 28             	call   0x868c
    5e6b:	c7 84 e8 a9 8a c6    	movw   $0xc68a,-0x5618(%si)
    5e71:	ee                   	out    %al,(%dx)
    5e72:	84 00                	test   %al,(%bx,%si)
    5e74:	09 11                	or     %dx,(%bx,%di)
    5e76:	b0 ff                	mov    $0xff,%al
    5e78:	73 84                	jae    0x5dfe
    5e7a:	29 13                	sub    %dx,(%bp,%di)
    5e7c:	a2 60 da             	mov    %al,0xda60
    5e7f:	46                   	inc    %si
    5e80:	95                   	xchg   %ax,%bp
    5e81:	b0 08                	mov    $0x8,%al
    5e83:	00 81 46 c6          	add    %al,-0x39ba(%bx,%di)
    5e87:	06                   	push   %es
    5e88:	e9 3e fe             	jmp    0x5cc9
    5e8b:	e4 0e                	in     $0xe,%al
    5e8d:	ea 3e 2a ed 81       	ljmp   $0x81ed,$0x2a3e
    5e92:	f9                   	stc
    5e93:	0b 73 74             	or     0x74(%bp,%di),%si
    5e96:	1d 51 e8             	sbb    $0xe851,%ax
    5e99:	1a 00                	sbb    (%bx,%si),%al
    5e9b:	a3 c6 00             	mov    %ax,0xc6
    5e9e:	33 b8 48 45          	xor    0x4548(%bx,%si),%di
    5ea2:	c6 06 e9 3e ff       	movb   $0xff,0x3ee9
    5ea7:	c7 06 ec 3e ff ff    	movw   $0xffff,0x3eec
    5ead:	c3                   	ret
    5eae:	c7 06 ec 3e 8a c6    	movw   $0xc68a,0x3eec
    5eb4:	05 a0 11             	add    $0x11a0,%ax
    5eb7:	2f                   	das
    5eb8:	e4 1d                	in     $0x1d,%al
    5eba:	28 8c 0d c2          	sub    %cl,-0x3df3(%si)
    5ebe:	f6 39                	idivb  (%bx,%di)
    5ec0:	36 ec                	ss in  (%dx),%al
    5ec2:	3e 75 0b             	jne,pt 0x5ed0
    5ec5:	4c                   	dec    %sp
    5ec6:	15 5e 11             	adc    $0x115e,%ax
    5ec9:	73 05                	jae    0x5ed0
    5ecb:	06                   	push   %es
    5ecc:	f3 90                	pause
    5ece:	a9 8b fc             	test   $0xfc8b,%ax
    5ed1:	f4                   	hlt
    5ed2:	04 0d                	add    $0xd,%al
    5ed4:	ff                   	(bad)
    5ed5:	ff                   	ljmp   (bad)
    5ed6:	e8 8b e8             	call   0x4764
    5ed9:	f0 e2 ff             	lock loop 0x5edb
    5edc:	e8 d8 50             	call   0xafb7
    5edf:	8b fb                	mov    %bx,%di
    5ee1:	e8 4e 3f             	call   0x9e32
    5ee4:	5b                   	pop    %bx
    5ee5:	b8 0f 44             	mov    $0x440f,%ax
    5ee8:	3f                   	aas
    5ee9:	8d 03                	lea    (%bp,%di),%ax
    5eeb:	01 d8                	add    %bx,%ax
    5eed:	50                   	push   %ax
    5eee:	e8 01 3f             	call   0x9df2
    5ef1:	5b                   	pop    %bx
    5ef2:	b8 ff b5             	mov    $0xb5ff,%ax
    5ef5:	f2 e2 ff             	repnz loop 0x5ef7
    5ef8:	e8 ee f0             	call   0x4fe9
    5efb:	7d 76                	jge    0x5f73
    5efd:	b0 33                	mov    $0x33,%al
    5eff:	b8 48 75             	mov    $0x7548,%ax
    5f02:	03 01                	add    (%bx,%di),%ax
    5f04:	d8 50 ff             	fcoms  -0x1(%bx,%si)
    5f07:	e8 ee f0             	call   0x4ff8
    5f0a:	ff                   	ljmp   (bad)
    5f0b:	e8 e6 d6             	call   0x35f4
    5f0e:	b1 5b                	mov    $0x5b,%cl
    5f10:	b9 33 b8             	mov    $0xb833,%cx
    5f13:	48                   	dec    %ax
    5f14:	53                   	push   %bx
    5f15:	2a fa                	sub    %dl,%bh
    5f17:	ec                   	in     (%dx),%al
    5f18:	f6 47 13 80          	testb  $0x80,0x13(%bx)
    5f1c:	74 8a                	je     0x5ea8
    5f1e:	ff                   	(bad)
    5f1f:	ba ee f0             	mov    $0xf0ee,%dx
    5f22:	ff                   	ljmp   (bad)
    5f23:	e8 f4 16             	call   0x761a
    5f26:	d6                   	(bad)
    5f27:	b1 48                	mov    $0x48,%cl
    5f29:	b2 83                	mov    $0x83,%dl
    5f2b:	f6 75 8b             	divb   -0x75(%di)
    5f2e:	7f 40                	jg     0x5f70
    5f30:	7c 06                	jl     0x5f38
    5f32:	74 c7                	je     0x5efb
    5f34:	f4                   	hlt
    5f35:	ff 73 38             	push   0x38(%bp,%di)
    5f38:	b1 de                	mov    $0xde,%cl
    5f3a:	46                   	inc    %si
    5f3b:	89 f4                	mov    %si,%sp
    5f3d:	89 46 ea             	mov    %ax,-0x16(%bp)
    5f40:	05 0f 00             	add    $0xf,%ax
    5f43:	e8 04 de             	call   0x3d4a
    5f46:	f5                   	cmc
    5f47:	5e                   	pop    %si
    5f48:	26 89 46 e8          	mov    %ax,%es:-0x18(%bp)
    5f4c:	f8                   	clc
    5f4d:	fa                   	cli
    5f4e:	56                   	push   %si
    5f4f:	80 fc ff             	cmp    $0xff,%ah
    5f52:	26 1c f2             	es sbb $0xf2,%al
    5f55:	f8                   	clc
    5f56:	3d 37 57             	cmp    $0x5737,%ax
    5f59:	de 46 ee             	fiadds -0x12(%bp)
    5f5c:	f0 ea 89 17 b8 ec    	lock ljmp $0xecb8,$0x1789
    5f62:	39 03                	cmp    %ax,(%bp,%di)
    5f64:	47                   	inc    %di
    5f65:	04 2d                	add    $0x2d,%al
    5f67:	fb                   	sti
    5f68:	3e bf 03 6f          	ds mov $0x6f03,%di
    5f6c:	a2 8b 76             	mov    %al,0x768b
    5f6f:	e8 ff e8             	call   0x4871
    5f72:	ee                   	out    %al,(%dx)
    5f73:	f0 7d 32             	lock jge 0x5fa8
    5f76:	b2 83                	mov    $0x83,%dl
    5f78:	fe                   	(bad)
    5f79:	a1 5e 0c             	mov    0xc5e,%ax
    5f7c:	f6 fa                	idiv   %dl
    5f7e:	ca 16 72             	lret   $0x7216
    5f81:	1b 88 bf 01          	sbb    0x1bf(%bx,%si),%cx
    5f85:	e4 1d                	in     $0x1d,%al
    5f87:	22 8c 0d e8          	and    -0x17f3(%si),%cl
    5f8b:	4e                   	dec    %si
    5f8c:	d6                   	(bad)
    5f8d:	b1 16                	mov    $0x16,%cl
    5f8f:	88 e3                	mov    %ah,%bl
    5f91:	7e d3                	jle    0x5f66
    5f93:	c3                   	ret
    5f94:	07                   	pop    %es
    5f95:	97                   	xchg   %ax,%di
    5f96:	de 8a 89 98          	fimuls -0x6777(%bp,%si)
    5f9a:	8a e0                	mov    %al,%ah
    5f9c:	80 bf 87 95 b8       	cmpb   $0xb8,-0x6a79(%bx)
    5fa1:	e8 00 87             	call   0xe6a4
    5fa4:	88 00                	mov    %al,(%bx,%si)
    5fa6:	fe 04                	incb   (%si)
    5fa8:	18 8f 8a fa          	sbb    %cl,-0x576(%bx)
    5fac:	25 8a 5c             	and    $0x5c8a,%ax
    5faf:	4b                   	dec    %bx
    5fb0:	b8 18 c0             	mov    $0xc018,%ax
    5fb3:	04 8b                	add    $0x8b,%al
    5fb5:	e8 28 7f             	call   0xdee0
    5fb8:	07                   	pop    %es
    5fb9:	b0 b8                	mov    $0xb8,%al
    5fbb:	9c                   	pushf
    5fbc:	ed                   	in     (%dx),%ax
    5fbd:	5b                   	pop    %bx
    5fbe:	8a 71 3e             	mov    0x3e(%bx,%di),%dh
    5fc1:	5b                   	pop    %bx
    5fc2:	b8 81 c6             	mov    $0xc681,%ax
    5fc5:	18 19                	sbb    %bl,(%bx,%di)
    5fc7:	89 36 cc 4e          	mov    %si,0x4ecc
    5fcb:	b8 ec de             	mov    $0xdeec,%ax
    5fce:	e3 0e                	jcxz   0x5fde
    5fd0:	22 3d                	and    (%di),%bh
    5fd2:	86 3d                	xchg   %bh,(%di)
    5fd4:	ff d0                	call   *%ax
    5fd6:	4e                   	dec    %si
    5fd7:	be cc 4e             	mov    $0x4ecc,%si
    5fda:	eb ff                	jmp    0x5fdb
    5fdc:	8a 83 40 51          	mov    0x5140(%bp,%di),%al
    5fe0:	5b                   	pop    %bx
    5fe1:	46                   	inc    %si
    5fe2:	be 0d 3e             	mov    $0x3e0d,%si
    5fe5:	5b                   	pop    %bx
    5fe6:	b8 46 7c             	mov    $0x7c46,%ax
    5fe9:	8c 72 d4             	mov    %?,-0x2c(%bp,%si)
    5fec:	4e                   	dec    %si
    5fed:	72 8a                	jb     0x5f79
    5fef:	83 40 0d 05          	addw   $0x5,0xd(%bx,%si)
    5ff3:	52                   	push   %dx
    5ff4:	de f9                	fdivrp %st,%st(1)
    5ff6:	e8 28 be             	call   0x1e21
    5ff9:	ea 89 de 88 74       	ljmp   $0x7488,$0xde89
    5ffe:	48                   	dec    %ax
    5fff:	36 fe                	ss (bad)
    6001:	12 ff                	adc    %bh,%bh
    6003:	eb 1d                	jmp    0x6022
    6005:	05 52 65             	add    $0x6552,%ax
    6008:	04 8b                	add    $0x8b,%al
    600a:	f0 cd 30             	lock int $0x30
    600d:	87 1e f6 11          	xchg   %bx,0x11f6
    6011:	e0 c6                	loopne 0x5fd9
    6013:	ee                   	out    %al,(%dx)
    6014:	8b e8                	mov    %ax,%bp
    6016:	01 c6                	add    %ax,%si
    6018:	ee                   	out    %al,(%dx)
    6019:	89 95 b8 a0          	mov    %dx,-0x5f48(%di)
    601d:	c0 5f 88 87          	rcrb   $0x87,-0x78(%bx)
    6021:	82 52 ea 89          	adcb   $0x89,-0x16(%bp,%si)
    6025:	de e8                	fsubrp %st,%st(0)
    6027:	cf                   	iret
    6028:	f3 83 fe e9          	repz cmp $0xffe9,%si
    602c:	cb                   	lret
    602d:	36 39 8b c7 02       	cmp    %cx,%ss:0x2c7(%bp,%di)
    6032:	21 bd 8e 24          	and    %di,0x248e(%di)
    6036:	26 f0 e8 16 e2       	es lock call 0x4251
    603b:	c7 02 31 05          	movw   $0x531,(%bp,%si)
    603f:	03 75 bf             	add    -0x41(%di),%si
    6042:	08 e8                	or     %ch,%al
    6044:	44                   	inc    %sp
    6045:	bd a1 b8             	mov    $0xb8a1,%bp
    6048:	e9 ae 04             	jmp    0x64f9
    604b:	f6 36 89 76          	divb   0x7689
    604f:	f4                   	hlt
    6050:	2a c0                	sub    %al,%al
    6052:	88 46 f6             	mov    %al,-0xa(%bp)
    6055:	1b f7                	sbb    %di,%si
    6057:	eb a0                	jmp    0x5ff9
    6059:	4e                   	dec    %si
    605a:	63 b1 22 36          	arpl   %si,0x3622(%bx,%di)
    605e:	fe                   	(bad)
    605f:	f6 f1                	div    %cl
    6061:	17                   	pop    %ss
    6062:	f9                   	stc
    6063:	f2 b8 ec de          	repnz mov $0xdeec,%ax
    6067:	8b c3                	mov    %bx,%ax
    6069:	e3 0e                	jcxz   0x6079
    606b:	22 8f 5a 45          	and    0x455a(%bx),%cl
    606f:	89 4e ea             	mov    %cx,-0x16(%bp)
    6072:	22 8f ec 42          	and    0x42ec(%bx),%cl
    6076:	89 4e ec             	mov    %cx,-0x14(%bp)
    6079:	fa                   	cli
    607a:	25 8a 5c             	and    $0x5c8a,%ax
    607d:	4b                   	dec    %bx
    607e:	43                   	inc    %bx
    607f:	17                   	pop    %ss
    6080:	f9                   	stc
    6081:	17                   	pop    %ss
    6082:	ec                   	in     (%dx),%al
    6083:	03 e0                	add    %ax,%sp
    6085:	80 bf 7e 52 4b       	cmpb   $0x4b,0x527e(%bx)
    608a:	7e 09                	jle    0x6095
    608c:	80 7e f9 00          	cmpb   $0x0,-0x7(%bp)
    6090:	74 03                	je     0x6095
    6092:	fe 4e f9             	decb   -0x7(%bp)
    6095:	fe                   	(bad)
    6096:	18 8a 25 5c          	sbb    %cl,0x5c25(%bp,%si)
    609a:	b8 c0 8b             	mov    $0x8bc0,%ax
    609d:	1e                   	push   %ds
    609e:	03 a2 eb 8c          	add    -0x7315(%bp,%si),%sp
    60a2:	12 ff                	adc    %bh,%bh
    60a4:	ec                   	in     (%dx),%al
    60a5:	03 e0                	add    %ax,%sp
    60a7:	22 8f 70 52          	and    0x5270(%bx),%cl
    60ab:	03 8f 72 52          	add    0x5272(%bx),%cx
    60af:	3b c1                	cmp    %cx,%ax
    60b1:	73 06                	jae    0x60b9
    60b3:	8e 24                	mov    (%si),%fs
    60b5:	f8                   	clc
    60b6:	1b f7                	sbb    %di,%si
    60b8:	eb b8                	jmp    0x6072
    60ba:	ec                   	in     (%dx),%al
    60bb:	de a0 71 59          	fisubs 0x5971(%bx,%si)
    60bf:	38 87 92 40          	cmp    %al,0x4092(%bx)
    60c3:	77 09                	ja     0x60ce
    60c5:	a0 4e 63             	mov    0x634e,%al
    60c8:	38 87 ae 40          	cmp    %al,0x40ae(%bx)
    60cc:	5e                   	pop    %si
    60cd:	b1 0c                	mov    $0xc,%cl
    60cf:	08 c7                	or     %al,%bh
    60d1:	fe                   	(bad)
    60d2:	ff f6                	push   %si
    60d4:	01 e9                	add    %bp,%cx
    60d6:	01 04                	add    %ax,(%si)
    60d8:	89 76 f2             	mov    %si,-0xe(%bp)
    60db:	83 40 0d 05          	addw   $0x5,0xd(%bx,%si)
    60df:	8a 4e f6             	mov    -0xa(%bp),%cl
    60e2:	2a ed                	sub    %ch,%ch
    60e4:	f0 32 40 e2          	lock xor -0x1e(%bx,%si),%al
    60e8:	51                   	push   %cx
    60e9:	e8 cc a3             	call   0x4b8
    60ec:	83 40 0d 05          	addw   $0x5,0xd(%bx,%si)
    60f0:	52                   	push   %dx
    60f1:	0e                   	push   %cs
    60f2:	04 12                	add    $0x12,%al
    60f4:	ff 2d                	ljmp   *(%di)
    60f6:	d8 67 b0             	fsubs  -0x50(%bx)
    60f9:	22 53 e6             	and    -0x1a(%bp,%di),%dl
    60fc:	5b                   	pop    %bx
    60fd:	f9                   	stc
    60fe:	c6 06 8e 40 03       	movb   $0x3,0x408e
    6103:	2b fe                	sub    %si,%di
    6105:	7e 8c                	jle    0x6093
    6107:	e4 5c                	in     $0x5c,%al
    6109:	40                   	inc    %ax
    610a:	eb 46                	jmp    0x6152
    610c:	8a 5e f6             	mov    -0xa(%bp),%bl
    610f:	2a ff                	sub    %bh,%bh
    6111:	8b c3                	mov    %bx,%ax
    6113:	e3 0e                	jcxz   0x6123
    6115:	03 d8                	add    %ax,%bx
    6117:	03 5e ec             	add    -0x14(%bp),%bx
    611a:	8a 01                	mov    (%bx,%di),%al
    611c:	1b f7                	sbb    %di,%si
    611e:	fd                   	std
    611f:	88 43 ee             	mov    %al,-0x12(%bp,%di)
    6122:	3c 3d                	cmp    $0x3d,%al
    6124:	75 05                	jne    0x612b
    6126:	c6 06 8e 40 02       	movb   $0x2,0x408e
    612b:	8a 5e fd             	mov    -0x3(%bp),%bl
    612e:	b1 53                	mov    $0x53,%cl
    6130:	f9                   	stc
    6131:	9f                   	lahf
    6132:	e6 40                	out    %al,$0x40
    6134:	8b e3                	mov    %bx,%sp
    6136:	22 5a 89             	and    -0x77(%bp,%si),%bl
    6139:	ea 28 40 8b 5e       	ljmp   $0x5e8b,$0x4028
    613e:	fe 89 0f fa          	decb   -0x5f1(%bx,%di)
    6142:	25 8a 5c             	and    $0x5c8a,%ax
    6145:	4a                   	dec    %dx
    6146:	40                   	inc    %ax
    6147:	8b 5e fe             	mov    -0x2(%bp),%bx
    614a:	88 02                	mov    %al,(%bp,%si)
    614c:	c7                   	(bad)
    614d:	0c c7                	or     $0xc7,%al
    614f:	e7 47                	out    %ax,$0x47
    6151:	e3 81                	jcxz   0x60d4
    6153:	d0 72 80             	shlb   $1,-0x80(%bp,%si)
    6156:	40                   	inc    %ax
    6157:	72 b3                	jb     0x610c
    6159:	e8 88 06             	call   0x67e4
    615c:	a1 ff 0a             	mov    0xaff,%ax
    615f:	88 e7                	mov    %ah,%bh
    6161:	ff                   	lcall  (bad)
    6162:	d8 ff                	fdivr  %st(7),%st
    6164:	be 32 e8             	mov    $0xe832,%si
    6167:	97                   	xchg   %ax,%di
    6168:	be 32 e8             	mov    $0xe832,%si
    616b:	67 be 32 e8          	addr32 mov $0xe832,%si
    616f:	c5 1d                	lds    (%di),%bx
    6171:	52                   	push   %dx
    6172:	04 ff                	add    $0xff,%al
    6174:	1c 2b                	sbb    $0x2b,%al
    6176:	f6 a1 ee c1          	mulb   -0x3e12(%bx,%di)
    617a:	f7 2a                	imulw  (%bp,%si)
    617c:	66 38 e7             	data32 cmp %ah,%bh
    617f:	ff                   	lcall  (bad)
    6180:	d8 b8 80 40          	fdivrs 0x4080(%bx,%si)
    6184:	d8 b8 82 40          	fdivrs 0x4082(%bx,%si)
    6188:	e8 d7 2c             	call   0x8e62
    618b:	f7 72 fe             	divw   -0x2(%bp,%si)
    618e:	83 fe 38             	cmp    $0x38,%si
    6191:	72 e4                	jb     0x6177
    6193:	b8 80 40             	mov    $0x4080,%ax
    6196:	45                   	inc    %bp
    6197:	fd                   	std
    6198:	e8 44 e4             	call   0x45df
    619b:	77 f4                	ja     0x6191
    619d:	e8 6f 06             	call   0x680f
    61a0:	b8 00 50             	mov    $0x5000,%ax
    61a3:	50                   	push   %ax
    61a4:	e8 97 be             	call   0x203e
    61a7:	32 e8                	xor    %al,%ch
    61a9:	6e                   	outsb  %ds:(%si),(%dx)
    61aa:	be 32 e8             	mov    $0xe832,%si
    61ad:	c6                   	(bad)
    61ae:	1d 52 04             	sbb    $0x452,%ax
    61b1:	c0 c8 63             	ror    $0x63,%al
    61b4:	b8 b8 55             	mov    $0x55b8,%ax
    61b7:	73 55                	jae    0x620e
    61b9:	80 45 e8 e4          	addb   $0xe4,-0x18(%di)
    61bd:	f4                   	hlt
    61be:	6f                   	outsw  %ds:(%si),(%dx)
    61bf:	b8 06 f3             	mov    $0xf306,%ax
    61c2:	f8                   	clc
    61c3:	56                   	push   %si
    61c4:	fc                   	cld
    61c5:	3d 0b 74             	cmp    $0x740b,%ax
    61c8:	51                   	push   %cx
    61c9:	89 f0                	mov    %si,%ax
    61cb:	7e 8a                	jle    0x6157
    61cd:	5a                   	pop    %dx
    61ce:	ee                   	out    %al,(%dx)
    61cf:	88 5e fd             	mov    %bl,-0x3(%bp)
    61d2:	b1 53                	mov    $0x53,%cl
    61d4:	f9                   	stc
    61d5:	87 e6                	xchg   %sp,%si
    61d7:	40                   	inc    %ax
    61d8:	73 75                	jae    0x624f
    61da:	73 ff                	jae    0x61db
    61dc:	8b f0                	mov    %ax,%si
    61de:	ff 92 4c b8          	call   *-0x47b4(%bp,%si)
    61e2:	00 fd                	add    %bh,%ch
    61e4:	d9 b0 22 53          	fnstenv 0x5322(%bx,%si)
    61e8:	e6 f9                	out    %al,$0xf9
    61ea:	f8                   	clc
    61eb:	e4 ff                	in     $0xff,%al
    61ed:	e8 01 77             	call   0xd8f1
    61f0:	03 e9                	add    %cx,%bp
    61f2:	8a 02                	mov    (%bp,%si),%al
    61f4:	b8 36 0e             	mov    $0xe36,%ax
    61f7:	b8 57 48             	mov    $0x4857,%ax
    61fa:	9f                   	lahf
    61fb:	e4 ff                	in     $0xff,%al
    61fd:	e8 0e 72             	call   0xd40e
    6200:	16                   	push   %ss
    6201:	b8 de 71             	mov    $0x71de,%ax
    6204:	e4 ff                	in     $0xff,%al
    6206:	e4 2c                	in     $0x2c,%al
    6208:	0e                   	push   %cs
    6209:	38 87 ca 40          	cmp    %al,0x40ca(%bx)
    620d:	77 04                	ja     0x6213
    620f:	b0 03                	mov    $0x3,%al
    6211:	eb 1f                	jmp    0x6232
    6213:	b0 02                	mov    $0x2,%al
    6215:	eb 1b                	jmp    0x6232
    6217:	8e f8                	mov    %ax,%?
    6219:	f7 f6                	div    %si
    621b:	66 f9                	data32 stc
    621d:	d1 e0                	shl    $1,%ax
    621f:	8a fd                	mov    %ch,%bh
    6221:	53                   	push   %bx
    6222:	9f                   	lahf
    6223:	40                   	inc    %ax
    6224:	e3 5a                	jcxz   0x6280
    6226:	24 41                	and    $0x41,%al
    6228:	02 5e f7             	add    -0x9(%bp),%bl
    622b:	03 89 45 76          	add    0x7645(%bx,%di),%cx
    622f:	ea 8a 00 88 f6       	ljmp   $0xf688,$0x8a
    6234:	f7 3c                	idivw  (%si)
    6236:	02 02                	add    (%bp,%si),%al
    6238:	01 e9                	add    %bp,%cx
    623a:	09 4c b8             	or     %cx,-0x48(%si)
    623d:	03 fa                	add    %dx,%di
    623f:	fe c0                	inc    %al
    6241:	ec                   	in     (%dx),%al
    6242:	e0 8f                	loopne 0x61d3
    6244:	88 82 ea de          	mov    %al,-0x2116(%bp,%si)
    6248:	88 82 83 52          	mov    %al,0x5283(%bp,%si)
    624c:	c6                   	(bad)
    624d:	8b 01                	mov    (%bx,%di),%ax
    624f:	ee                   	out    %al,(%dx)
    6250:	02 ff                	add    %bh,%bh
    6252:	06                   	push   %es
    6253:	06                   	push   %es
    6254:	c1 c6 89             	rol    $0x89,%si
    6257:	b8 c0 88             	mov    $0x88c0,%ax
    625a:	8b 87 72 52          	mov    0x5272(%bx),%ax
    625e:	01 87 52 8b          	add    %ax,-0x74ae(%bx)
    6262:	03 87 70 52          	add    0x5270(%bx),%ax
    6266:	87 8b 87 52          	xchg   %cx,0x5287(%bp,%di)
    626a:	8b 52 87             	mov    -0x79(%bp,%si),%dx
    626d:	52                   	push   %dx
    626e:	39 87 76 52          	cmp    %ax,0x5276(%bx)
    6272:	73 0b                	jae    0x627f
    6274:	06                   	push   %es
    6275:	77 e0                	ja     0x6257
    6277:	8b 52 76             	mov    0x76(%bp,%si),%dx
    627a:	52                   	push   %dx
    627b:	89 52 87             	mov    %dx,-0x79(%bp,%si)
    627e:	52                   	push   %dx
    627f:	83 0d 8a             	orw    $0xff8a,(%di)
    6282:	f6 ed                	imul   %ch
    6284:	32 e2                	xor    %dl,%ah
    6286:	e8 a3 40             	call   0xa32c
    6289:	05 0e 12             	add    $0x120e,%ax
    628c:	2d 67 28             	sub    $0x2867,%ax
    628f:	a2 83 0d             	mov    %al,0xd83
    6292:	52                   	push   %dx
    6293:	04 ff                	add    $0xff,%al
    6295:	6a 02                	push   $0x2
    6297:	12 2d                	adc    (%di),%ch
    6299:	67 22 e6             	addr32 and %dh,%ah
    629c:	f9                   	stc
    629d:	06                   	push   %es
    629e:	40                   	inc    %ax
    629f:	2b 42 f8             	sub    -0x8(%bp,%si),%ax
    62a2:	e4 ff                	in     $0xff,%al
    62a4:	06                   	push   %es
    62a5:	02 02                	add    (%bp,%si),%al
    62a7:	1a 50 03             	sbb    0x3(%bx,%si),%dl
    62aa:	fd                   	std
    62ab:	c8 fa 13 f8          	enter  $0x13fa,$0xf8
    62af:	e8 00 1a             	call   0x7cb2
    62b2:	e8 e2 02             	call   0x6597
    62b5:	05 75 d6             	add    $0xd675,%ax
    62b8:	b6 54                	mov    $0x54,%dh
    62ba:	e9 0a 89             	jmp    0xebc7
    62bd:	f4                   	hlt
    62be:	c0 76 72 81          	shlb   $0x81,0x72(%bp)
    62c2:	50                   	push   %ax
    62c3:	03 fd                	add    %bp,%di
    62c5:	c8 fa f9 f7          	enter  $0xf9fa,$0xf7
    62c9:	e8 e6 19             	call   0x7cb2
    62cc:	b0 08                	mov    $0x8,%al
    62ce:	c8 fa 57 fa          	enter  $0x57fa,$0xfa
    62d2:	a1 ec 08             	mov    0x8ec,%ax
    62d5:	3c 74                	cmp    $0x74,%al
    62d7:	87 eb                	xchg   %bp,%bx
    62d9:	06                   	push   %es
    62da:	3f                   	aas
    62db:	b8 45 0e             	mov    $0xe45,%ax
    62de:	82 2b a1             	subb   $0xa1,(%bp,%di)
    62e1:	44                   	inc    %sp
    62e2:	0a e7                	or     %bh,%ah
    62e4:	d8 be e8 00          	fdivrs 0xe8(%bp)
    62e8:	50                   	push   %ax
    62e9:	97                   	xchg   %ax,%di
    62ea:	b9 97 e7             	mov    $0xe797,%cx
    62ed:	01 8d 6b e7          	add    %cx,-0x1895(%di)
    62f1:	01 8d c6 95          	add    %cx,-0x6a3a(%di)
    62f5:	39 f7                	cmp    %si,%di
    62f7:	7b 1b                	jnp    0x6314
    62f9:	b8 d4 4d             	mov    $0x4dd4,%ax
    62fc:	79 b8                	jns    0x62b6
    62fe:	2a d6                	sub    %dh,%dl
    6300:	0d 16 e1             	or     $0xe116,%ax
    6303:	45                   	inc    %bp
    6304:	de f7                	fdivp  %st,%st(7)
    6306:	50                   	push   %ax
    6307:	03 fd                	add    %bp,%di
    6309:	c8 fa b5 f7          	enter  $0xb5fa,$0xf7
    630d:	50                   	push   %ax
    630e:	28 4e 79             	sub    %cl,0x79(%bp)
    6311:	b8 2a d6             	mov    $0xd62a,%ax
    6314:	0d 46 b9             	or     $0xb946,%ax
    6317:	77 ca                	ja     0x62e3
    6319:	f7 03 c8 b5          	testw  $0xb5c8,(%bp,%di)
    631d:	50                   	push   %ax
    631e:	4e                   	dec    %si
    631f:	a1 f7 e8             	mov    0xe8f7,%ax
    6322:	8e 19                	mov    (%bx,%di),%ds
    6324:	a1 ec 08             	mov    0x8ec,%ax
    6327:	3c e0                	cmp    $0xe0,%al
    6329:	46                   	inc    %si
    632a:	45                   	inc    %bp
    632b:	06                   	push   %es
    632c:	3f                   	aas
    632d:	b8 45 0e             	mov    $0xe45,%ax
    6330:	30 2b                	xor    %ch,(%bp,%di)
    6332:	ff 36 ec 08          	push   0x8ec
    6336:	b8 50 e8             	mov    $0xe850,%ax
    6339:	be e8 4f             	mov    $0x4fe8,%si
    633c:	b5 50                	mov    $0x50,%ch
    633e:	4e                   	dec    %si
    633f:	21 2b                	and    %bp,(%bp,%di)
    6341:	fe                   	(bad)
    6342:	f6 e9                	imul   %cl
    6344:	0f 8b 76 f2          	jnp    0x55be
    6348:	e4 ff                	in     $0xff,%al
    634a:	06                   	push   %es
    634b:	02 75 0a             	add    0xa(%di),%dh
    634e:	8a f6                	mov    %dh,%dh
    6350:	e9 2a e4             	jmp    0x477d
    6353:	2d 11 00             	sub    $0x11,%ax
    6356:	7d 03                	jge    0x635b
    6358:	e9 7e 01             	jmp    0x64d9
    635b:	48                   	dec    %ax
    635c:	48                   	dec    %ax
    635d:	7e 0d                	jle    0x636c
    635f:	48                   	dec    %ax
    6360:	48                   	dec    %ax
    6361:	74 4d                	je     0x63b0
    6363:	48                   	dec    %ax
    6364:	e8 87 00             	call   0x63ee
    6367:	ef                   	out    %ax,(%dx)
    6368:	00 e9                	add    %ch,%cl
    636a:	6d                   	insw   (%dx),%es:(%di)
    636b:	01 83 8a ed          	add    %ax,-0x1276(%bp,%di)
    636f:	e2 a3                	loop   0x6314
    6371:	05 12 67             	add    $0x6712,%ax
    6374:	a2 0d 04             	mov    %al,0x40d
    6377:	6a 12                	push   $0x12
    6379:	67 e6 3b             	addr32 out %al,$0x3b
    637c:	a1 83 0d             	mov    0xd83,%ax
    637f:	52                   	push   %dx
    6380:	04 ff                	add    $0xff,%al
    6382:	7d 01                	jge    0x6385
    6384:	b8 69 67             	mov    $0x6769,%ax
    6387:	e6 06                	out    %al,$0x6
    6389:	2b f8                	sub    %ax,%di
    638b:	ff 02                	incw   (%bp,%si)
    638d:	55                   	push   %bp
    638e:	f7 c8 50 a1          	test   $0xa150,%ax
    6392:	e8 19 2c             	call   0x8fae
    6395:	f7 c8 90 a1          	test   $0xa190,%ax
    6399:	e8 19 a6             	call   0x9b5
    639c:	9d                   	popf
    639d:	b8 0c 4d             	mov    $0x4d0c,%ax
    63a0:	b8 d6 16             	mov    $0x16d6,%ax
    63a3:	45                   	inc    %bp
    63a4:	f7 46 b9 77 3a       	testw  $0x3a77,-0x47(%bp)
    63a9:	f7 e8                	imul   %ax
    63ab:	05 19 e9             	add    $0xe919,%ax
    63ae:	a8 81                	test   $0x81,%al
    63b0:	50                   	push   %ax
    63b1:	02 00                	add    (%bx,%si),%al
    63b3:	fd                   	std
    63b4:	d9 c6                	fld    %st(6)
    63b6:	f7 a1 0a d8          	mulw   -0x27f6(%bx,%di)
    63ba:	e8 50 b9             	call   0x1d0d
    63bd:	e7 8d                	out    %ax,$0x8d
    63bf:	e7 8d                	out    %ax,$0x8d
    63c1:	95                   	xchg   %ax,%bp
    63c2:	f7 1b                	negw   (%bp,%di)
    63c4:	d4 79                	aam    $0x79
    63c6:	2a 0d                	sub    (%di),%cl
    63c8:	e1 de                	loope  0x63a8
    63ca:	50                   	push   %ax
    63cb:	fd                   	std
    63cc:	fa                   	cli
    63cd:	f7 a4 1a e8          	mulw   -0x17e6(%si)
    63d1:	7f e4                	jg     0x63b7
    63d3:	c8 90 a1 e8          	enter  $0xa190,$0xe8
    63d7:	19 69 9d             	sbb    %bp,-0x63(%bx,%di)
    63da:	a1 d8 50             	mov    0x50d8,%ax
    63dd:	e7 e7                	out    %ax,$0xe7
    63df:	95                   	xchg   %ax,%bp
    63e0:	1b 79 0d             	sbb    0xd(%bx,%di),%di
    63e3:	de e8                	fsubrp %st,%st(0)
    63e5:	4f                   	dec    %di
    63e6:	b5 50                	mov    $0x50,%ch
    63e8:	b8 69 b5             	mov    $0xb569,%ax
    63eb:	50                   	push   %ax
    63ec:	b8 92 a1             	mov    $0xa192,%ax
    63ef:	e8 19 81             	call   0xe50b
    63f2:	1a a1 08 74          	sbb    0x7408(%bx,%di),%ah
    63f6:	eb 3f                	jmp    0x6437
    63f8:	45                   	inc    %bp
    63f9:	82 a1 0a d8 e8       	andb   $0xe8,-0x27f6(%bx,%di)
    63fe:	50                   	push   %ax
    63ff:	61                   	popa
    6400:	2a 19                	sub    (%bx,%di),%bl
    6402:	2d 6d e7             	sub    $0xe76d,%ax
    6405:	04 19                	add    $0x19,%al
    6407:	be 0f 9f             	mov    $0x9f0f,%si
    640a:	b9 fe d4             	mov    $0xd4fe,%cx
    640d:	b5 50                	mov    $0x50,%ch
    640f:	b8 b8 a1             	mov    $0xa1b8,%ax
    6412:	e8 19 ce             	call   0x322e
    6415:	27                   	daa
    6416:	06                   	push   %es
    6417:	b9 b8 01             	mov    $0x1b8,%cx
    641a:	b8 69 39             	mov    $0x3969,%ax
    641d:	1f                   	pop    %ds
    641e:	01 e8                	add    %bp,%ax
    6420:	13 6e 5e             	adc    0x5e(%bp),%bp
    6423:	f6 04 e8             	testb  $0xe8,(%si)
    6426:	1c a2                	sbb    $0xa2,%al
    6428:	eb 21                	jmp    0x644b
    642a:	ff                   	(bad)
    642b:	7d 9f                	jge    0x63cc
    642d:	0a c0                	or     %al,%al
    642f:	75 24                	jne    0x6455
    6431:	e8 10 a2             	call   0x644
    6434:	e8 77 9d             	call   0x1ae
    6437:	3d 48 00             	cmp    $0x48,%ax
    643a:	76 12                	jbe    0x644e
    643c:	e8 b9 9c             	call   0xf8
    643f:	b8 81 50             	mov    $0x5081,%ax
    6442:	f6 70 25             	divb   0x25(%bx,%si)
    6445:	a3 3d 02             	mov    %ax,0x23d
    6448:	00 73 03             	add    %dh,0x3(%bp,%di)
    644b:	e8 fd 00             	call   0x654b
    644e:	83 3e b2 43 8b       	cmpw   $0xff8b,0x43b2
    6453:	e8 d5 e8             	call   0x4d2b
    6456:	df 03                	filds  (%bp,%di)
    6458:	e8 02 75             	call   0xd95d
    645b:	b6 e9                	mov    $0xe9,%dh
    645d:	30 b5 ff 0e          	xor    %dh,0xeff(%di)
    6461:	06                   	push   %es
    6462:	3f                   	aas
    6463:	77 3a                	ja     0x649f
    6465:	b5 ff                	mov    $0xff,%ch
    6467:	0e                   	push   %cs
    6468:	08 74 e8             	or     %dh,-0x18(%si)
    646b:	33 b5 54 0a          	xor    0xa54(%di),%si
    646f:	f4                   	hlt
    6470:	76 81                	jbe    0x63f3
    6472:	03 e8                	add    %ax,%bp
    6474:	aa                   	stos   %al,%es:(%di)
    6475:	f8                   	clc
    6476:	b0 c8                	mov    $0xc8,%al
    6478:	57                   	push   %di
    6479:	a1 9e f9             	mov    0xf99e,%ax
    647c:	eb 9c                	jmp    0x641a
    647e:	2a 01                	sub    (%bx,%di),%al
    6480:	75 b6                	jne    0x6438
    6482:	e9 0a b5             	jmp    0x198f
    6485:	e4 06                	in     $0x6,%al
    6487:	75 8a                	jne    0x6413
    6489:	e9 0e ff             	jmp    0x639a
    648c:	0e                   	push   %cs
    648d:	06                   	push   %es
    648e:	3f                   	aas
    648f:	77 0e                	ja     0x649f
    6491:	b5 0e                	mov    $0xe,%ch
    6493:	74 33                	je     0x64c8
    6495:	54                   	push   %sp
    6496:	f4                   	hlt
    6497:	07                   	pop    %es
    6498:	b5 54                	mov    $0x54,%ch
    649a:	0a f4                	or     %ah,%dh
    649c:	e4 e8                	in     $0xe8,%al
    649e:	72 00                	jb     0x64a0
    64a0:	75 09                	jne    0x64ab
    64a2:	b8 0e 57             	mov    $0x570e,%ax
    64a5:	9f                   	lahf
    64a6:	ff 9a 9c eb          	lcall  *-0x1464(%bp,%si)
    64aa:	2e e8 73 b9          	cs call 0x1e21
    64ae:	ea de 74 36 12       	ljmp   $0x1236,$0x74de
    64b3:	eb 05                	jmp    0x64ba
    64b5:	65 8b cd             	gs mov %bp,%cx
    64b8:	87 f6                	xchg   %si,%si
    64ba:	e0 ee                	loopne 0x64aa
    64bc:	b0 4b                	mov    $0x4b,%al
    64be:	83 f4 19             	xor    $0x19,%sp
    64c1:	33 48 2a             	xor    0x2a(%bx,%si),%cx
    64c4:	14 f6                	adc    $0xf6,%al
    64c6:	e0 ee                	loopne 0x64b6
    64c8:	e8 c6 89             	call   0xee91
    64cb:	b8 c0 88             	mov    $0x88c0,%ax
    64ce:	82 ea de             	sub    $0xde,%dl
    64d1:	cf                   	iret
    64d2:	83 e9 36             	sub    $0x36,%cx
    64d5:	8b 02                	mov    (%bp,%si),%ax
    64d7:	bd 24 1d             	mov    $0x1d24,%bp
    64da:	8a 19                	mov    (%bx,%di),%bl
    64dc:	03 e9                	add    %cx,%bp
    64de:	f8                   	clc
    64df:	fb                   	sti
    64e0:	81 3d 75 b8          	cmpw   $0xb875,(%di)
    64e4:	74 4c                	je     0x6532
    64e6:	e6 e8                	out    %al,$0xe8
    64e8:	de e8                	fsubrp %st,%st(0)
    64ea:	0c ef                	or     $0xef,%al
    64ec:	e8 9b cd             	call   0x328a
    64ef:	87 36 ee 54          	xchg   %si,0x54ee
    64f3:	f4                   	hlt
    64f4:	aa                   	stos   %al,%es:(%di)
    64f5:	b4 46                	mov    $0x46,%ah
    64f7:	f4                   	hlt
    64f8:	46                   	inc    %si
    64f9:	fe                   	(bad)
    64fa:	a1 5e 0c             	mov    0xc5e,%ax
    64fd:	f6 fa                	idiv   %dl
    64ff:	ca 08 4c             	lret   $0x4c08
    6502:	15 5e e9             	adc    $0xe95e,%ax
    6505:	f9                   	stc
    6506:	ff 52 2a             	call   *0x2a(%bp,%si)
    6509:	4b                   	dec    %bx
    650a:	f0 45                	lock inc %bp
    650c:	3d 76 5b             	cmp    $0x5b76,%ax
    650f:	a2 fe c0             	mov    %al,0xc0fe
    6512:	ff 89 94 6e          	decw   0x6e94(%bx,%di)
    6516:	fe                   	(bad)
    6517:	2b eb                	sub    %bx,%bp
    6519:	b8 20 b8             	mov    $0xb820,%ax
    651c:	7e e7                	jle    0x6505
    651e:	de 2a                	fisubrs (%bp,%si)
    6520:	2d 04 e4             	sub    $0xe404,%ax
    6523:	8a fc                	mov    %ah,%bh
    6525:	94                   	xchg   %ax,%sp
    6526:	cf                   	iret
    6527:	e8 3a 5b             	call   0xc064
    652a:	9a 76 f8 0b ff       	lcall  $0xff0b,$0xf876
    652f:	74 09                	je     0x653a
    6531:	c6 05 00             	movb   $0x0,(%di)
    6534:	8d 45 01             	lea    0x1(%di),%ax
    6537:	e4 8a                	in     $0x8a,%al
    6539:	04 46                	add    $0x46,%al
    653b:	b9 77 fe             	mov    $0xfe77,%cx
    653e:	77 db                	ja     0x651b
    6540:	de fc                	fdivrp %st,%st(4)
    6542:	e8 a1 0c             	call   0x71e6
    6545:	fa                   	cli
    6546:	16                   	push   %ss
    6547:	1b bf e4 22          	sbb    0x22e4(%bx),%di
    654b:	a1 74 3f             	mov    0x3f74,%ax
    654e:	82 2a 66             	subb   $0x66,(%bp,%si)
    6551:	55                   	push   %bp
    6552:	a1 d8 50             	mov    0x50d8,%ax
    6555:	2a 2d                	sub    (%di),%ch
    6557:	09 29                	or     %bp,(%bx,%di)
    6559:	b8 00 fd             	mov    $0xfd00,%ax
    655c:	d9 c6                	fld    %st(6)
    655e:	e3 9b                	jcxz   0x64fb
    6560:	06                   	push   %es
    6561:	b8 b8 39             	mov    $0x39b8,%ax
    6564:	01 13                	add    %dx,(%bp,%di)
    6566:	5e                   	pop    %si
    6567:	04 1c                	add    $0x1c,%al
    6569:	3f                   	aas
    656a:	87 37                	xchg   %si,(%bx)
    656c:	e8 04 04             	call   0x6973
    656f:	08 e4                	or     %ah,%ah
    6571:	8c 1e 89 7e          	mov    %ds,0x7e89
    6575:	2b 2a                	sub    (%bp,%si),%bp
    6577:	4b                   	dec    %bx
    6578:	68 31 f9             	push   $0xf931
    657b:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    657c:	e8 e8 c8             	call   0x2e67
    657f:	f8                   	clc
    6580:	f9                   	stc
    6581:	a9 03 bf             	test   $0xbf03,%ax
    6584:	e8 03 e9             	call   0x4e8a
    6587:	a7                   	cmpsw  %es:(%di),%ds:(%si)
    6588:	01 e8                	add    %bp,%ax
    658a:	58                   	pop    %ax
    658b:	02 a1 50 e7          	add    -0x18b0(%bx,%di),%ah
    658f:	1b 0d                	sbb    (%di),%cx
    6591:	e8 6e 9c             	call   0x202
    6594:	37                   	aaa
    6595:	b9 41 00             	mov    $0x41,%cx
    6598:	51                   	push   %cx
    6599:	ba 13 5e             	mov    $0x5e13,%dx
    659c:	52                   	push   %dx
    659d:	ba c7 00             	mov    $0xc7,%dx
    65a0:	52                   	push   %dx
    65a1:	e8 d0 18             	call   0x7e74
    65a4:	8e 26 e8 e2          	mov    -0x1d18,%fs
    65a8:	02 4e f8             	add    -0x8(%bp),%cl
    65ab:	51                   	push   %cx
    65ac:	e8 0b 0b             	call   0x70ba
    65af:	c7 46 f0 9f 00       	movw   $0x9f,-0x10(%bp)
    65b4:	c7                   	(bad)
    65b5:	b2 32                	mov    $0x32,%dl
    65b7:	05 c7 d4             	add    $0xd4c7,%ax
    65ba:	24 f4                	and    $0xf4,%al
    65bc:	fe c7                	inc    %bh
    65be:	d4 24                	aam    $0x24
    65c0:	f6 41 00 b8          	testb  $0xb8,0x0(%bx,%di)
    65c4:	da c1                	fcmovb %st(1),%st
    65c6:	b8 39 a6             	mov    $0xa639,%ax
    65c9:	e8 f0 04             	call   0x6abc
    65cc:	1c 00                	sbb    $0x0,%al
    65ce:	fd                   	std
    65cf:	d9 b8 50 f0          	fnstcw -0xfb0(%bx,%si)
    65d3:	e8 20 6f             	call   0xd4f6
    65d6:	2f                   	das
    65d7:	8e 26 e8 04          	mov    0x4e8,%fs
    65db:	40                   	inc    %ax
    65dc:	a2 f4 4e             	mov    %al,0x4ef4
    65df:	b8 ee 4e             	mov    $0x4eee,%ax
    65e2:	f9                   	stc
    65e3:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    65e4:	e8 e8 c8             	call   0x2ecf
    65e7:	2a 75 e9             	sub    -0x17(%di),%dh
    65ea:	b5 06                	mov    $0x6,%ch
    65ec:	85 ac 33 2a          	test   %bp,0x2a33(%si)
    65f0:	f6 16 2a 75          	notb   0x752a
    65f4:	e9 b5 b9             	jmp    0x1fac
    65f7:	08 0d                	or     %cl,(%di)
    65f9:	de fd                	fdivrp %st,%st(5)
    65fb:	4f                   	dec    %di
    65fc:	fd                   	std
    65fd:	f7 1a                	negw   (%bp,%si)
    65ff:	61                   	popa
    6600:	28 2a                	sub    %ch,(%bp,%si)
    6602:	75 e9                	jne    0x65ed
    6604:	b5 06                	mov    $0x6,%ch
    6606:	98                   	cbtw
    6607:	20 8e e8 40          	and    %cl,0x40e8(%bp)
    660b:	f4                   	hlt
    660c:	b8 4e e8             	mov    $0xe84e,%ax
    660f:	4e                   	dec    %si
    6610:	b8 e4 4e             	mov    $0x4ee4,%ax
    6613:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    6614:	e8 2a e9             	call   0x4f41
    6617:	06                   	push   %es
    6618:	57                   	push   %di
    6619:	03 de                	add    %si,%bx
    661b:	e3 a6                	jcxz   0x65c3
    661d:	ab                   	stos   %ax,%es:(%di)
    661e:	e4 75                	in     $0x75,%al
    6620:	e9 ff 06             	jmp    0x6d22
    6623:	22 b8 d4 4e          	and    0x4ed4(%bx,%si),%bh
    6627:	e8 e9 57             	call   0xbe13
    662a:	de a6 08 fd          	fisubs -0x2f8(%bp)
    662e:	52                   	push   %dx
    662f:	0e                   	push   %cs
    6630:	09 b0 a3 0e          	or     %si,0xea3(%bx,%si)
    6634:	c1 b8 da 4e e9       	sarw   $0xe9,0x4eda(%bx,%si)
    6639:	de 08                	fimuls (%bx,%si)
    663b:	52                   	push   %dx
    663c:	09 a3 46 77          	or     %sp,0x7746(%bp,%di)
    6640:	f7 f8                	idiv   %ax
    6642:	af                   	scas   %es:(%di),%ax
    6643:	a3 ec b8             	mov    %ax,0xb8ec
    6646:	e8 e0 4e             	call   0xb529
    6649:	de 52 a3             	ficoms -0x5d(%bp,%si)
    664c:	77 f8                	ja     0x6646
    664e:	a3 b8 e0             	mov    %ax,0xe0b8
    6651:	de e7                	fsubp  %st,%st(7)
    6653:	af                   	scas   %es:(%di),%ax
    6654:	a3 ee f4             	mov    %ax,0xf4ee
    6657:	b4 b5                	mov    $0xb5,%ah
    6659:	01 06 3f b9          	add    %ax,-0x46c1
    665d:	01 22                	add    %sp,(%bp,%si)
    665f:	22 40 20             	and    0x20(%bx,%si),%al
    6662:	ba 40 00             	mov    $0x40,%dx
    6665:	52                   	push   %dx
    6666:	e8 0a 03             	call   0x6973
    6669:	a1 e7 0d             	mov    0xde7,%ax
    666c:	6e                   	outsb  %ds:(%si),(%dx)
    666d:	37                   	aaa
    666e:	41                   	inc    %cx
    666f:	51                   	push   %cx
    6670:	13 52 c7             	adc    -0x39(%bp,%si),%dx
    6673:	52                   	push   %dx
    6674:	d0 8e e8 02          	rorb   $1,0x2e8(%bp)
    6678:	f8                   	clc
    6679:	e8 0b 46             	call   0xac87
    667c:	9f                   	lahf
    667d:	c7                   	(bad)
    667e:	32 f3                	xor    %bl,%dh
    6680:	17                   	pop    %ss
    6681:	20 6f 0b             	and    %ch,0xb(%bx)
    6684:	c7                   	(bad)
    6685:	90                   	nop
    6686:	a7                   	cmpsw  %es:(%di),%ds:(%si)
    6687:	e8 03 de             	call   0x448d
    668a:	88 7e c3             	mov    %bh,-0x3d(%bp)
    668d:	97                   	xchg   %ax,%di
    668e:	8a 22                	mov    (%bp,%si),%ah
    6690:	8a eb                	mov    %bl,%ch
    6692:	e8 9c 9a             	call   0x131
    6695:	b8 a0 fd             	mov    $0xfda0,%ax
    6698:	d9 b8 52 d0          	fnstcw -0x2fae(%bx,%si)
    669c:	ba 44 5b             	mov    $0x5b44,%dx
    669f:	25 a1 0a             	and    $0xaa1,%ax
    66a2:	e7 d8                	out    %ax,$0xd8
    66a4:	be e8 be             	mov    $0xbee8,%si
    66a7:	75 fd                	jne    0x66a6
    66a9:	d9 b8 52 fd          	fnstcw -0x2ae(%bx,%si)
    66ad:	d9 b8 33 5e          	fnstcw 0x5e33(%bx,%si)
    66b1:	1c 87                	sbb    $0x87,%al
    66b3:	c0 d0 ba             	rcl    $0xba,%al
    66b6:	44                   	inc    %sp
    66b7:	bb 17 2b             	mov    $0x2b17,%bx
    66ba:	a1 c1 2a             	mov    0x2ac1,%ax
    66bd:	38 ff                	cmp    %bh,%bh
    66bf:	b8 1c 87             	mov    $0x871c,%ax
    66c2:	71 e8                	jno    0x66ac
    66c4:	e8 b5 4e             	call   0xb57c
    66c7:	2b f6                	sub    %si,%si
    66c9:	0f 96 27             	setbe  (%bx)
    66cc:	f7 fe                	idiv   %si
    66ce:	fe                   	(bad)
    66cf:	72 b8                	jb     0x6689
    66d1:	40                   	inc    %ax
    66d2:	7c e7                	jl     0x66bb
    66d4:	8e 26 e8 e2          	mov    -0x1d18,%fs
    66d8:	02 05                	add    (%di),%al
    66da:	e8 f8 a9             	call   0x10d5
    66dd:	77 14                	ja     0x66f3
    66df:	e7 e7                	out    %ax,$0xe7
    66e1:	95                   	xchg   %ax,%bp
    66e2:	1b 79 4f             	sbb    0x4f(%bx,%di),%di
    66e5:	fd                   	std
    66e6:	f7 ba 67 9f          	idivw  -0x6099(%bp,%si)
    66ea:	c7                   	(bad)
    66eb:	bb 90 00             	mov    $0x90,%bx
    66ee:	e8 01 81             	call   0xe7f2
    66f1:	02 e7                	add    %bh,%ah
    66f3:	95                   	xchg   %ax,%bp
    66f4:	79 fd                	jns    0x66f3
    66f6:	ba 9f bb             	mov    $0xbb9f,%dx
    66f9:	00 b9 67 9f          	add    %bh,-0x6099(%bx,%di)
    66fd:	f7 ba 90 00          	idivw  0x90(%bp,%si)
    6701:	c7                   	(bad)
    6702:	32 dd                	xor    %ch,%bl
    6704:	cb                   	lret
    6705:	b8 58 e8             	mov    $0xe858,%ax
    6708:	e8 b5 ab             	call   0x12c0
    670b:	2b f6                	sub    %si,%si
    670d:	0f 5a 9f 1b 0d       	cvtps2pd 0xd1b(%bx),%xmm3
    6712:	e8 b5 b8             	call   0x1fca
    6715:	67 32 c5             	addr32 xor %ch,%al
    6718:	e8 c9 9f             	call   0x6e4
    671b:	1b 97 32 6e          	sbb    0x6e32(%bx),%dx
    671f:	32 c5                	xor    %ch,%al
    6721:	52                   	push   %dx
    6722:	ff 2b                	ljmp   *(%bp,%di)
    6724:	e3 9f                	jcxz   0x66c5
    6726:	02 05                	add    (%di),%al
    6728:	e8 f8 a9             	call   0x1123
    672b:	7c aa                	jl     0x66d7
    672d:	f9                   	stc
    672e:	ff 43 0c             	incw   0xc(%bp,%di)
    6731:	74 88                	je     0x66bb
    6733:	0d ef e0             	or     $0xe0ef,%ax
    6736:	02 f2                	add    %dl,%dh
    6738:	81 74 3c b8 ee       	xorw   $0xeeb8,0x3c(%si)
    673d:	03 02                	add    (%bp,%si),%ax
    673f:	36 ff 52 2a          	call   *%ss:0x2a(%bp,%si)
    6743:	88 f6                	mov    %dh,%dh
    6745:	f7 ff                	idiv   %di
    6747:	e4 73                	in     $0x73,%al
    6749:	fe 81 fe e7          	incb   -0x1802(%bx,%di)
    674d:	03 76 13             	add    0x13(%bp),%si
    6750:	b8 54 4f             	mov    $0x4f54,%ax
    6753:	6e                   	outsb  %ds:(%si),(%dx)
    6754:	32 72 b8             	xor    -0x48(%bp,%si),%dh
    6757:	de af d7 36          	fisubrs 0x36d7(%bx)
    675b:	eb 4d                	jmp    0x67aa
    675d:	63 75 dc             	arpl   %si,-0x24(%di)
    6760:	83 ee 64             	sub    $0x64,%si
    6763:	72 b8                	jb     0x671d
    6765:	64 73 f5             	fs jae 0x675d
    6768:	eb 06                	jmp    0x6770
    676a:	fb                   	sti
    676b:	fe                   	(bad)
    676c:	e5 83                	in     $0x83,%ax
    676e:	ee                   	out    %al,(%dx)
    676f:	0a 72 b8             	or     -0x48(%bp,%si),%dh
    6772:	0a 73 f5             	or     -0xb(%bp,%di),%dh
    6775:	f4                   	hlt
    6776:	38 75 b0             	cmp    %dh,-0x50(%di)
    6779:	a9 b2 09             	test   $0x9b2,%ax
    677c:	eb e8                	jmp    0x6766
    677e:	82 4f 32 72          	orb    $0x72,0x32(%bx)
    6782:	b8 de af             	mov    $0xafde,%ax
    6785:	ab                   	stos   %ax,%es:(%di)
    6786:	36 5b                	ss pop %bx
    6788:	5b                   	pop    %bx
    6789:	ff e4                	jmp    *%sp
    678b:	fb                   	sti
    678c:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    678d:	11 b0 a9 eb          	adc    %si,-0x1457(%bx,%si)
    6791:	e8 6e 4f             	call   0xb702
    6794:	72 de                	jb     0x6774
    6796:	ab                   	stos   %ax,%es:(%di)
    6797:	5b                   	pop    %bx
    6798:	ff 57 36             	call   *0x36(%bx)
    679b:	5b                   	pop    %bx
    679c:	5b                   	pop    %bx
    679d:	d1 e6                	shl    $1,%si
    679f:	ff b4 5a 4f          	push   0x4f5a(%si)
    67a3:	de 5b 57             	ficomps 0x57(%bp,%di)
    67a6:	5b                   	pop    %bx
    67a7:	d1 48 36             	rorw   $1,0x36(%bx,%si)
    67aa:	5b                   	pop    %bx
    67ab:	5b                   	pop    %bx
    67ac:	de 5b 57             	ficomps 0x57(%bp,%di)
    67af:	46                   	inc    %si
    67b0:	b0 00                	mov    $0x0,%al
    67b2:	46                   	inc    %si
    67b3:	06                   	push   %es
    67b4:	3e e4 83             	ds in  $0x83,%al
    67b7:	3e 96                	ds xchg %ax,%si
    67b9:	4f                   	dec    %di
    67ba:	01 74 17             	add    %si,0x17(%si)
    67bd:	83 3e 26 9b 02       	cmpw   $0x2,-0x64da
    67c2:	74 10                	je     0x67d4
    67c4:	06                   	push   %es
    67c5:	b8 b8 39             	mov    $0x39b8,%ax
    67c8:	01 13                	add    %dx,(%bp,%di)
    67ca:	5e                   	pop    %si
    67cb:	04 1c                	add    $0x1c,%al
    67cd:	eb ff                	jmp    0x67ce
    67cf:	9f                   	lahf
    67d0:	c0 a0 01 c3 06       	shlb   $0x6,-0x3cff(%bx,%si)
    67d5:	b8 01 5e             	mov    $0x5e01,%ax
    67d8:	1c ff                	sbb    $0xff,%al
    67da:	c0 01 06             	rolb   $0x6,(%bx,%di)
    67dd:	01 1c                	add    %bx,(%si)
    67df:	c0 06 38 01 c3       	rolb   $0xc3,0x138
    67e4:	08 83 4c 05          	or     %al,0x54c(%bp,%di)
    67e8:	26 9b                	es fwait
    67ea:	06                   	push   %es
    67eb:	74 12                	je     0x67ff
    67ed:	52                   	push   %dx
    67ee:	b8 50 8b             	mov    $0x8b50,%ax
    67f1:	f8                   	clc
    67f2:	be da c1             	mov    $0xc1da,%si
    67f5:	1e                   	push   %ds
    67f6:	07                   	pop    %es
    67f7:	b9 09 00             	mov    $0x9,%cx
    67fa:	f3 a5                	rep movsw %ds:(%si),%es:(%di)
    67fc:	89 f0                	mov    %si,%ax
    67fe:	22 b8 e6 32          	and    0x32e6(%bx,%si),%bh
    6802:	6e                   	outsb  %ds:(%si),(%dx)
    6803:	32 df                	xor    %bh,%bl
    6805:	03 b6 30 ff          	add    -0xd0(%bp),%si
    6809:	39 5b 5b             	cmp    %bx,0x5b(%bp,%di)
    680c:	89 f0                	mov    %si,%ax
    680e:	22 52 4b             	and    0x4b(%bp,%si),%dl
    6811:	45                   	inc    %bp
    6812:	b8 50 8b             	mov    $0x8b50,%ax
    6815:	f8                   	clc
    6816:	be b6 9a             	mov    $0x9ab6,%si
    6819:	1e                   	push   %ds
    681a:	b9 00 a5             	mov    $0xa500,%cx
    681d:	f0 b8 32 4c          	lock mov $0x4c32,%ax
    6821:	05 26 9b             	add    $0x9b26,%ax
    6824:	06                   	push   %es
    6825:	75 0d                	jne    0x6834
    6827:	b8 f6 32             	mov    $0x32f6,%ax
    682a:	32 03                	xor    (%bp,%di),%al
    682c:	30 39                	xor    %bh,(%bx,%di)
    682e:	5b                   	pop    %bx
    682f:	f0 08 39             	lock or %bh,(%bx,%di)
    6832:	5b                   	pop    %bx
    6833:	f0 52                	lock push %dx
    6835:	45                   	inc    %bp
    6836:	50                   	push   %ax
    6837:	08 19                	or     %bl,(%bx,%di)
    6839:	1e                   	push   %ds
    683a:	bf ff 73             	mov    $0x73ff,%di
    683d:	1a b4 83 3e          	sbb    0x3e83(%si),%dh
    6841:	96                   	xchg   %ax,%si
    6842:	4f                   	dec    %di
    6843:	01 1a                	add    %bx,(%bp,%si)
    6845:	c0 fe 88             	sar    $0x88,%dh
    6848:	f7 e4                	mul    %sp
    684a:	fe 83 3e 96          	incb   -0x69c2(%bp,%di)
    684e:	4f                   	dec    %di
    684f:	b8 00 88             	mov    $0x8800,%ax
    6852:	e8 32 e4             	call   0x4c87
    6855:	83 3e 26 06 0d       	cmpw   $0xd,0x626
    685a:	f6 32                	divb   (%bp,%si)
    685c:	30 5b 08             	xor    %bl,0x8(%bp,%di)
    685f:	5b                   	pop    %bx
    6860:	52                   	push   %dx
    6861:	50                   	push   %ax
    6862:	19 bf 73 d3          	sbb    %di,-0x2c8d(%bx)
    6866:	38 5b f0             	cmp    %bl,-0x10(%bp,%di)
    6869:	c7 06 96 4f b8 00    	movw   $0xb8,0x4f96
    686f:	45                   	inc    %bp
    6870:	50                   	push   %ax
    6871:	f8                   	clc
    6872:	b6 1e                	mov    $0x1e,%dh
    6874:	00 f0                	add    %dh,%al
    6876:	32 05                	xor    (%di),%al
    6878:	9b                   	fwait
    6879:	75 b8                	jne    0x6833
    687b:	32 03                	xor    (%bp,%di),%al
    687d:	39 e4                	cmp    %sp,%sp
    687f:	e4 0e                	in     $0xe,%al
    6881:	c6                   	(bad)
    6882:	e8 06 00             	call   0x688b
    6885:	ff c1                	inc    %cx
    6887:	1a b4 80 36          	sbb    0x3680(%si),%dh
    688b:	96                   	xchg   %ax,%si
    688c:	4f                   	dec    %di
    688d:	01 a1 26 06          	add    %sp,0x626(%bx,%di)
    6891:	48                   	dec    %ax
    6892:	74 48                	je     0x68dc
    6894:	0a 48 74             	or     0x74(%bx,%si),%cl
    6897:	48                   	dec    %ax
    6898:	22 48 74             	and    0x74(%bx,%si),%cl
    689b:	48                   	dec    %ax
    689c:	54                   	push   %sp
    689d:	eb 6b                	jmp    0x690a
    689f:	52                   	push   %dx
    68a0:	50                   	push   %ax
    68a1:	f8                   	clc
    68a2:	da 1e b9 00          	ficompl 0xb9
    68a6:	a5                   	movsw  %ds:(%si),%es:(%di)
    68a7:	f0 b8 32 32          	lock mov $0x3232,%ax
    68ab:	03 30                	add    (%bx,%si),%si
    68ad:	39 ff                	cmp    %di,%di
    68af:	36 de c1             	ss faddp %st,%st(1)
    68b2:	ff 36 ba 9a          	push   -0x6546
    68b6:	e8 db 9b             	call   0x494
    68b9:	eb 4f                	jmp    0x690a
    68bb:	e8 fd 1b             	call   0x84bb
    68be:	83 3e 96 4f b8       	cmpw   $0xffb8,0x4f96
    68c3:	74 11                	je     0x68d6
    68c5:	ff 36 b8 9a          	push   -0x6548
    68c9:	e8 bc 9a             	call   0x388
    68cc:	52                   	push   %dx
    68cd:	f8                   	clc
    68ce:	1e                   	push   %ds
    68cf:	00 f0                	add    %dh,%al
    68d1:	32 03                	xor    (%bp,%di),%al
    68d3:	39 eb                	cmp    %bp,%bx
    68d5:	0f 3e                	(bad)
    68d7:	c1 dc c1             	rcr    $0xc1,%sp
    68da:	e8 ab 9a             	call   0x388
    68dd:	f8                   	clc
    68de:	00 32                	add    %dh,(%bp,%si)
    68e0:	39 0f                	cmp    %cx,(%bx)
    68e2:	c1 f0 05             	shl    $0x5,%ax
    68e5:	75 32                	jne    0x6919
    68e7:	39 e4                	cmp    %sp,%sp
    68e9:	c6 06 ff e8 cc       	movb   $0xcc,-0x1701
    68ee:	1b eb                	sbb    %bx,%bp
    68f0:	19 83 96 b8          	sbb    %ax,-0x476a(%bp,%di)
    68f4:	11 36 9a 05          	adc    %si,0x59a
    68f8:	b8 e6 32             	mov    $0x32e6,%ax
    68fb:	8b 1e 01 f4          	mov    -0xbff,%bx
    68ff:	08 52 19             	or     %dl,0x19(%bp,%si)
    6902:	73 38                	jae    0x693c
    6904:	f0 06                	lock push %es
    6906:	32 38                	xor    (%bx,%si),%bh
    6908:	5b                   	pop    %bx
    6909:	f0 83 b8 36 05 e6    	lock cmpw $0xffe6,0x536(%bx,%si)
    690f:	8b 01                	mov    (%bx,%di),%ax
    6911:	b0 01                	mov    $0x1,%al
    6913:	52                   	push   %dx
    6914:	45                   	inc    %bp
    6915:	50                   	push   %ax
    6916:	2a c0                	sub    %al,%al
    6918:	52                   	push   %dx
    6919:	50                   	push   %ax
    691a:	19 bf 73 b4          	sbb    %di,-0x4b8d(%bx)
    691e:	3e 4f                	ds dec %di
    6920:	08 8b 46 0a          	or     %cl,0xa46(%bp,%di)
    6924:	fa                   	cli
    6925:	f8                   	clc
    6926:	0a 03                	or     (%bp,%di),%al
    6928:	46                   	inc    %si
    6929:	08 8b 76 5e          	or     %cl,0x5e76(%bp,%di)
    692d:	8b 46 06             	mov    0x6(%bp),%ax
    6930:	eb 12                	jmp    0x6944
    6932:	f4                   	hlt
    6933:	23 2d                	and    (%di),%bp
    6935:	e4 fc                	in     $0xfc,%al
    6937:	cf                   	iret
    6938:	fe                   	(bad)
    6939:	a3 e0 e7             	mov    %ax,0xe7e0
    693c:	a3 16 b4             	mov    %ax,0xb416
    693f:	83 26 0d 32 5b       	andw   $0x5b,0x320d
    6944:	5b                   	pop    %bx
    6945:	0f 06                	clts
    6947:	65 9e                	gs sahf
    6949:	b0 04                	mov    $0x4,%al
    694b:	e2 05                	loop   0x6952
    694d:	f8                   	clc
    694e:	77 e7                	ja     0x6937
    6950:	95                   	xchg   %ax,%bp
    6951:	df 2c                	fildll (%si)
    6953:	eb 15                	jmp    0x696a
    6955:	b8 f0 05             	mov    $0x5f0,%ax
    6958:	52                   	push   %dx
    6959:	19 da                	sbb    %bx,%dx
    695b:	b8 a6 e8             	mov    $0xe8a6,%ax
    695e:	e9 57 de             	jmp    0x47b8
    6961:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    6962:	e4 31                	in     $0x31,%al
    6964:	ff                   	(bad)
    6965:	ff 0e e8 a4          	decw   -0x5b18
    6969:	2b e8                	sub    %ax,%bp
    696b:	1d b4 05             	sbb    $0x5b4,%ax
    696e:	52                   	push   %dx
    696f:	ec                   	in     (%dx),%al
    6970:	f8                   	clc
    6971:	fa                   	cli
    6972:	b9 08 83             	mov    $0x8308,%cx
    6975:	4c                   	dec    %sp
    6976:	05 2b 08             	add    $0x82b,%ax
    6979:	46                   	inc    %si
    697a:	fa                   	cli
    697b:	0a 46 8b             	or     -0x75(%bp),%al
    697e:	5e                   	pop    %si
    697f:	46                   	inc    %si
    6980:	eb f4                	jmp    0x6976
    6982:	2d fc fe             	sub    $0xfefc,%ax
    6985:	e0 a3                	loopne 0x692a
    6987:	b4 26                	mov    $0x26,%ah
    6989:	32 5b 06             	xor    0x6(%bp,%di),%bl
    698c:	9e                   	sahf
    698d:	04 05                	add    $0x5,%al
    698f:	77 95                	ja     0x6926
    6991:	2c 15                	sub    $0x15,%al
    6993:	f0 52                	lock push %dx
    6995:	be b3 83             	mov    $0x83b3,%si
    6998:	0d 5b 0f             	or     $0xf5b,%ax
    699b:	65 b0 e2             	gs mov $0xe2,%al
    699e:	f8                   	clc
    699f:	e4 b0                	in     $0xb0,%al
    69a1:	1d 9d a8             	sbb    $0xa89d,%ax
    69a4:	05 f8 77             	add    $0x77f8,%ax
    69a7:	e7 95                	out    %ax,$0x95
    69a9:	87 2c                	xchg   %bp,(%si)
    69ab:	eb 15                	jmp    0x69c2
    69ad:	b8 da b8             	mov    $0xb8da,%ax
    69b0:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    69b1:	b8 f0 05             	mov    $0x5f0,%ax
    69b4:	52                   	push   %dx
    69b5:	e8 57 a6             	call   0x100f
    69b8:	31 ff                	xor    %di,%di
    69ba:	e8 2b 1d             	call   0x86e8
    69bd:	05 ec fa             	add    $0xfaec,%ax
    69c0:	4c                   	dec    %sp
    69c1:	2b e8                	sub    %ax,%bp
    69c3:	c5 b3 05 ec          	lds    -0x13fb(%bp,%di),%si
    69c7:	fa                   	cli
    69c8:	08 4c 2b             	or     %cl,0x2b(%si)
    69cb:	b9 83 05             	mov    $0x583,%cx
    69ce:	08 fa                	or     %bh,%dl
    69d0:	46                   	inc    %si
    69d1:	16                   	push   %ss
    69d2:	57                   	push   %di
    69d3:	80 fe ff             	cmp    $0xff,%dh
    69d6:	ff 71 e8             	push   -0x18(%bx,%di)
    69d9:	5c                   	pop    %sp
    69da:	e8 46 f8             	call   0x6223
    69dd:	05 77 ec             	add    $0xec77,%ax
    69e0:	8a 0e ff a3          	mov    -0x5c01,%cl
    69e4:	03 ed                	add    %bp,%bp
    69e6:	03 84 f8 8b          	add    -0x7408(%si),%ax
    69ea:	f4                   	hlt
    69eb:	c7 46 ee 2b b9       	movw   $0xb92b,-0x12(%bp)
    69f0:	be 52 80             	mov    $0x8052,%si
    69f3:	80 0b 80             	orb    $0x80,(%bp,%di)
    69f6:	3c 01                	cmp    $0x1,%al
    69f8:	72 03                	jb     0x69fd
    69fa:	63 dc                	arpl   %bx,%sp
    69fc:	ee                   	out    %al,(%dx)
    69fd:	72 c6                	jb     0x69c5
    69ff:	20 81 fe b8          	and    %al,-0x4702(%bx,%di)
    6a03:	45                   	inc    %bp
    6a04:	0b 74 e4             	or     -0x1c(%si),%si
    6a07:	72 75                	jb     0x6a7e
    6a09:	b8 57 03             	mov    $0x357,%ax
    6a0c:	e9 8d 4c             	jmp    0xb69c
    6a0f:	b8 03 02             	mov    $0x203,%ax
    6a12:	1b eb                	sbb    %bx,%bp
    6a14:	ec                   	in     (%dx),%al
    6a15:	3c 0e                	cmp    $0xe,%al
    6a17:	76 09                	jbe    0x6a22
    6a19:	fe                   	(bad)
    6a1a:	ff f7                	push   %di
    6a1c:	0e                   	push   %cs
    6a1d:	c7 46 ee ff 75       	movw   $0x75ff,-0x12(%bp)
    6a22:	ff 76 f7             	push   -0x9(%bp)
    6a25:	46                   	inc    %si
    6a26:	f8                   	clc
    6a27:	f7 6e ec             	imulw  -0x14(%bp)
    6a2a:	8b 4e f4             	mov    -0xc(%bp),%cx
    6a2d:	83 c1 04             	add    $0x4,%cx
    6a30:	8b d1                	mov    %cx,%dx
    6a32:	d1 e1                	shl    $1,%cx
    6a34:	03 ca                	add    %dx,%cx
    6a36:	03 84 8b c8          	add    -0x3775(%si),%ax
    6a3a:	83 83 fa a1 3e       	addw   $0x3e,-0x5e06(%bp,%di)
    6a3f:	bf 5a 00             	mov    $0x5a,%di
    6a42:	2b f8                	sub    %ax,%di
    6a44:	03 cf                	add    %di,%cx
    6a46:	89 4e f0             	mov    %cx,-0x10(%bp)
    6a49:	e4 6f                	in     $0x6f,%al
    6a4b:	06                   	push   %es
    6a4c:	f8                   	clc
    6a4d:	fc                   	cld
    6a4e:	b3 b8                	mov    $0xb8,%bl
    6a50:	18 00                	sbb    %al,(%bx,%si)
    6a52:	50                   	push   %ax
    6a53:	57                   	push   %di
    6a54:	b9 27 01             	mov    $0x127,%cx
    6a57:	ea 89 17 f0 e8       	ljmp   $0xe8f0,$0x1789
    6a5c:	bd fe b8             	mov    $0xb8fe,%bp
    6a5f:	18 00                	sbb    %al,(%bx,%si)
    6a61:	50                   	push   %ax
    6a62:	57                   	push   %di
    6a63:	b8 27 01             	mov    $0x127,%ax
    6a66:	31 ff                	xor    %di,%di
    6a68:	e8 f0 ff             	call   0x6a5b
    6a6b:	8d 0e d6 90          	lea    -0x6f2a,%cx
    6a6f:	c3                   	ret
    6a70:	c6                   	(bad)
    6a71:	19 2d                	sbb    %bp,(%di)
    6a73:	6d                   	insw   (%dx),%es:(%di)
    6a74:	e7 a1                	out    %ax,$0xa1
    6a76:	5e                   	pop    %si
    6a77:	b8 ef 56             	mov    $0x56ef,%ax
    6a7a:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    6a7b:	b8 a0 d9             	mov    $0xd9a0,%ax
    6a7e:	52                   	push   %dx
    6a7f:	8d 4d 06             	lea    0x6(%di),%cx
    6a82:	51                   	push   %cx
    6a83:	e8 5e 21             	call   0x8be4
    6a86:	be 9f fe             	mov    $0xfe9f,%si
    6a89:	b5 b8                	mov    $0xb8,%ch
    6a8b:	a0 d9 52             	mov    0x52d9,%al
    6a8e:	46                   	inc    %si
    6a8f:	46                   	inc    %si
    6a90:	a1 40 40             	mov    0x4040,%ax
    6a93:	29 46 f0             	sub    %ax,-0x10(%bp)
    6a96:	ff 17                	call   *(%bx)
    6a98:	f0 e8 48 21          	lock call 0x8be4
    6a9c:	c7                   	(bad)
    6a9d:	a7                   	cmpsw  %es:(%di),%ds:(%si)
    6a9e:	03 88 c3 8a          	add    -0x753d(%bx,%si),%cx
    6aa2:	7e e4                	jle    0x6a88
    6aa4:	40                   	inc    %ax
    6aa5:	a2 57 89             	mov    %al,0x8957
    6aa8:	7e f2                	jle    0x6a9c
    6aaa:	eb 18                	jmp    0x6ac4
    6aac:	5e                   	pop    %si
    6aad:	83 fe 77             	cmp    $0x77,%si
    6ab0:	04 ff                	add    $0xff,%al
    6ab2:	97                   	xchg   %ax,%di
    6ab3:	8a 46 46             	mov    0x46(%bp),%al
    6ab6:	a1 03 c7             	mov    0xc703,%ax
    6ab9:	8a a3 8b 33          	mov    0x338b(%bp,%di),%ah
    6abd:	31 dc                	xor    %bx,%sp
    6abf:	1e                   	push   %ds
    6ac0:	0c e7                	or     $0xe7,%al
    6ac2:	e3 04                	jcxz   0x6ac8
    6ac4:	81 72 40 b2 57       	xorw   $0x57b2,0x40(%bp,%si)
    6ac9:	72 e1                	jb     0x6aac
    6acb:	46                   	inc    %si
    6acc:	46                   	inc    %si
    6acd:	a1 03 e4             	mov    0xe403,%ax
    6ad0:	de 89 c1 73          	fimuls 0x73c1(%bx,%di)
    6ad4:	c7                   	(bad)
    6ad5:	89 ea                	mov    %bp,%dx
    6ad7:	0f 2d fe             	cvtps2pi %xmm6,%mm7
    6ada:	a3 e8 f8             	mov    %ax,0xf8e8
    6add:	fa                   	cli
    6ade:	ee                   	out    %al,(%dx)
    6adf:	f8                   	clc
    6ae0:	26 5b                	es pop %bx
    6ae2:	9e                   	sahf
    6ae3:	eb 39                	jmp    0x6b1e
    6ae5:	b1 06                	mov    $0x6,%cl
    6ae7:	07                   	pop    %es
    6ae8:	37                   	aaa
    6ae9:	fc                   	cld
    6aea:	8a 9f 18 57          	mov    0x5718(%bx),%bl
    6aee:	88 38                	mov    %bh,(%bx,%si)
    6af0:	b0 b2                	mov    $0xb2,%al
    6af2:	eb e3                	jmp    0x6ad7
    6af4:	bf 8a ee             	mov    $0xee8a,%di
    6af7:	95                   	xchg   %ax,%bp
    6af8:	8b 01                	mov    (%bx,%di),%ax
    6afa:	72 1f                	jb     0x6b1b
    6afc:	14 b9                	adc    $0xb9,%al
    6afe:	00 89 c6 31          	add    %cl,0x31c6(%bx,%di)
    6b02:	ff                   	ljmp   (bad)
    6b03:	e8 fa c8             	call   0x3400
    6b06:	00 c6                	add    %al,%dh
    6b08:	e8 e8 f8             	call   0x63f3
    6b0b:	77 8a                	ja     0x6a97
    6b0d:	01 56 47             	add    %dx,0x47(%bp)
    6b10:	ff 76 f7             	push   -0x9(%bp)
    6b13:	ff                   	ljmp   (bad)
    6b14:	ee                   	out    %al,(%dx)
    6b15:	f8                   	clc
    6b16:	3b ee                	cmp    %si,%bp
    6b18:	f8                   	clc
    6b19:	76 09                	jbe    0x6b24
    6b1b:	ff                   	(bad)
    6b1c:	f8                   	clc
    6b1d:	1a 03                	sbb    (%bp,%di),%al
    6b1f:	e7 f2                	out    %ax,$0xf2
    6b21:	1b 72 c1             	sbb    -0x3f(%bp,%si),%si
    6b24:	e8 d2 ca             	call   0x35f9
    6b27:	8b 76 fc             	mov    -0x4(%bp),%si
    6b2a:	46                   	inc    %si
    6b2b:	8b 7e ea             	mov    -0x16(%bp),%di
    6b2e:	03 e7                	add    %di,%sp
    6b30:	ee                   	out    %al,(%dx)
    6b31:	00 74 4e             	add    %dh,0x4e(%si)
    6b34:	a1 e7 be             	mov    0xbee7,%ax
    6b37:	be fd b8             	mov    $0xb8fd,%si
    6b3a:	fd                   	std
    6b3b:	1e                   	push   %ds
    6b3c:	00 50 57             	add    %dl,0x57(%bx,%si)
    6b3f:	b8 21 01             	mov    $0x121,%ax
    6b42:	31 ff                	xor    %di,%di
    6b44:	e8 f0 e8             	call   0x5437
    6b47:	2b 13                	sub    (%bp,%di),%dx
    6b49:	89 76 fc             	mov    %si,-0x4(%bp)
    6b4c:	eb 2b                	jmp    0x6b79
    6b4e:	b1 07                	mov    $0x7,%cl
    6b50:	fc                   	cld
    6b51:	9f                   	lahf
    6b52:	57                   	push   %di
    6b53:	38 b2 e3 8a          	cmp    %dh,-0x751d(%bp,%si)
    6b57:	95                   	xchg   %ax,%bp
    6b58:	01 1f                	add    %bx,(%bx)
    6b5a:	b9 89 31             	mov    $0x3189,%cx
    6b5d:	e8 c8 c6             	call   0x3228
    6b60:	e8 77 01             	call   0x6cda
    6b63:	47                   	inc    %di
    6b64:	11 14                	adc    %dx,(%si)
    6b66:	00 c6                	add    %al,%dh
    6b68:	ff                   	(bad)
    6b69:	fa                   	cli
    6b6a:	00 57 e8             	add    %dl,-0x18(%bx)
    6b6d:	33 c6                	xor    %si,%ax
    6b6f:	e8 77 01             	call   0x6ce9
    6b72:	47                   	inc    %di
    6b73:	76 03                	jbe    0x6b78
    6b75:	f8                   	clc
    6b76:	ff 1a                	lcall  *(%bp,%si)
    6b78:	e7 1b                	out    %ax,$0x1b
    6b7a:	c1 d2 8b             	rcl    $0x8b,%dx
    6b7d:	fc                   	cld
    6b7e:	cf                   	iret
    6b7f:	e8 77 ca             	call   0x35f9
    6b82:	b8 00 57             	mov    $0x5700,%ax
    6b85:	27                   	daa
    6b86:	ff                   	ljmp   (bad)
    6b87:	e8 f2 b8             	call   0x247c
    6b8a:	01 ff                	add    %di,%di
    6b8c:	f0 46                	lock inc %si
    6b8e:	46                   	inc    %si
    6b8f:	f0 03 46 40          	lock add 0x40(%bp),%ax
    6b93:	29 f0                	sub    %si,%ax
    6b95:	33 31                	xor    (%bx,%di),%si
    6b97:	da fd                	(bad)
    6b99:	e8 ee b1             	call   0x1d8a
    6b9c:	c6                   	(bad)
    6b9d:	e8 06 00             	call   0x6ba6
    6ba0:	ff c1                	inc    %cx
    6ba2:	56                   	push   %si
    6ba3:	8b 5e 88             	mov    -0x78(%bp),%bx
    6ba6:	03 fe                	add    %si,%di
    6ba8:	19 56 e6             	sbb    %dx,-0x1a(%bp)
    6bab:	83 e8 77             	sub    $0x77,%ax
    6bae:	00 03                	add    %al,(%bp,%di)
    6bb0:	b8 45 5e             	mov    $0x5e45,%ax
    6bb3:	37                   	aaa
    6bb4:	ff 22                	jmp    *(%bp,%si)
    6bb6:	eb 9c                	jmp    0x6b54
    6bb8:	b8 fd b8             	mov    $0xb8fd,%ax
    6bbb:	d0 1e 00 50          	rcrb   $1,0x5000
    6bbf:	57                   	push   %di
    6bc0:	de a6 e4 d7          	fisubs -0x281c(%bp)
    6bc4:	1d b1 d3             	sbb    $0xd3b1,%ax
    6bc7:	06                   	push   %es
    6bc8:	89 b8 bf 00          	mov    %di,0xbf(%bx,%si)
    6bcc:	8b c3                	mov    %bx,%ax
    6bce:	38 b2 ee 2a          	cmp    %dh,0x2aee(%bp,%si)
    6bd2:	2b 11                	sub    (%bx,%di),%dx
    6bd4:	a9 e8 4f             	test   $0x4fe8,%ax
    6bd7:	de b2 57 b9          	fidivs -0x46a9(%bp,%si)
    6bdb:	60                   	pusha
    6bdc:	e7 ff                	out    %ax,$0xff
    6bde:	d8 03                	fadds  (%bp,%di)
    6be0:	de fc                	fdivrp %st,%st(4)
    6be2:	f0 e8 b6 1d          	lock call 0x899c
    6be6:	5e                   	pop    %si
    6be7:	fe 04                	incb   (%si)
    6be9:	a9 e8 4f             	test   $0x4fe8,%ax
    6bec:	de 34                	fidivs (%si)
    6bee:	57                   	push   %di
    6bef:	b8 8e 00             	mov    $0x8e,%ax
    6bf2:	57                   	push   %di
    6bf3:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    6bf4:	d7                   	xlat   %ds:(%bx)
    6bf5:	b1 06                	mov    $0x6,%cl
    6bf7:	a3 1d 5e             	mov    %ax,0x5e1d
    6bfa:	84 74 52             	test   %dh,0x52(%si)
    6bfd:	2b d2                	sub    %dx,%dx
    6bff:	52                   	push   %dx
    6c00:	50                   	push   %ax
    6c01:	b8 e2 00             	mov    $0xe2,%ax
    6c04:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    6c05:	b1 a3                	mov    $0xa3,%cl
    6c07:	5e                   	pop    %si
    6c08:	74 bd                	je     0x6bc7
    6c0a:	1e                   	push   %ds
    6c0b:	8a 84 81 2a          	mov    0x2a81(%si),%al
    6c0f:	2b e4                	sub    %sp,%sp
    6c11:	2b d2                	sub    %dx,%dx
    6c13:	52                   	push   %dx
    6c14:	50                   	push   %ax
    6c15:	b8 19 01             	mov    $0x119,%ax
    6c18:	31 ff                	xor    %di,%di
    6c1a:	e8 5e 74             	call   0xe07b
    6c1d:	a9 1e 0c             	test   $0xc1e,%ax
    6c20:	74 88                	je     0x6baa
    6c22:	0d ef e0             	or     $0xe0ef,%ax
    6c25:	02 88 3b 74          	add    0x743b(%bx,%si),%cl
    6c29:	57                   	push   %di
    6c2a:	b1 d3                	mov    $0xd3,%cl
    6c2c:	06                   	push   %es
    6c2d:	89 04                	mov    %ax,(%si)
    6c2f:	80 ec 8a             	sub    $0x8a,%ah
    6c32:	eb 7e                	jmp    0x6cb2
    6c34:	52                   	push   %dx
    6c35:	98                   	cbtw
    6c36:	8b c8                	mov    %ax,%cx
    6c38:	8a eb                	mov    %bl,%ch
    6c3a:	80 52 98 2b          	adcb   $0x2b,-0x68(%bp,%si)
    6c3e:	84 c8                	test   %cl,%al
    6c40:	83 8a eb 7d 52       	orw    $0x52,0x7deb(%bp,%si)
    6c45:	98                   	cbtw
    6c46:	03 c1                	add    %cx,%ax
    6c48:	b9 00 1e             	mov    $0x1e00,%cx
    6c4b:	99                   	cwtd
    6c4c:	4b                   	dec    %bx
    6c4d:	4e                   	dec    %si
    6c4e:	76 0b                	jbe    0x6c5b
    6c50:	74 c0                	je     0x6c12
    6c52:	7d 36                	jge    0x6c8a
    6c54:	8b 02                	mov    (%bp,%si),%ax
    6c56:	eb 08                	jmp    0x6c60
    6c58:	3d 03 00             	cmp    $0x3,%ax
    6c5b:	7e 03                	jle    0x6c60
    6c5d:	4b                   	dec    %bx
    6c5e:	45                   	inc    %bp
    6c5f:	76 31                	jbe    0x6c92
    6c61:	f8                   	clc
    6c62:	08 8b 63 06          	or     %cl,0x663(%bp,%di)
    6c66:	00 ff                	add    %bh,%bh
    6c68:	7e 77                	jle    0x6ce1
    6c6a:	00 fa                	add    %bh,%dl
    6c6c:	ff 52 5b             	call   *0x5b(%bp,%si)
    6c6f:	54                   	push   %sp
    6c70:	f6 8b d6 fc 04       	testb  $0x4,-0x32a(%bp,%di)
    6c75:	ff                   	ljmp   (bad)
    6c76:	e8 e8 8b             	call   0xf861
    6c79:	bf 96 3a             	mov    $0x3a96,%di
    6c7c:	81 fa 3f 01          	cmp    $0x13f,%dx
    6c80:	7d 1a                	jge    0x6c9c
    6c82:	8b df                	mov    %di,%bx
    6c84:	03 de                	add    %si,%bx
    6c86:	8a 07                	mov    (%bx),%al
    6c88:	1b fd                	sbb    %bp,%di
    6c8a:	43                   	inc    %bx
    6c8b:	46                   	inc    %si
    6c8c:	de d3                	(bad)
    6c8e:	03 df                	add    %di,%bx
    6c90:	8a b9 04 11          	mov    0x1104(%bx,%di),%bh
    6c94:	03 d0                	add    %ax,%dx
    6c96:	46                   	inc    %si
    6c97:	39 56 06             	cmp    %dx,0x6(%bp)
    6c9a:	7d e0                	jge    0x6c7c
    6c9c:	80 7e fd fe          	cmpb   $0xfe,-0x3(%bp)
    6ca0:	75 0b                	jne    0x6cad
    6ca2:	ff 36 bc 57          	push   0x57bc
    6ca6:	e8 2b ed             	call   0x59d4
    6ca9:	c6                   	(bad)
    6caa:	fd                   	std
    6cab:	43                   	inc    %bx
    6cac:	75 76                	jne    0x6d24
    6cae:	46                   	inc    %si
    6caf:	fd                   	std
    6cb0:	fe                   	(bad)
    6cb1:	5e                   	pop    %si
    6cb2:	f6 ca 4c             	test   $0x4c,%dl
    6cb5:	5e                   	pop    %si
    6cb6:	f9                   	stc
    6cb7:	52                   	push   %dx
    6cb8:	4b                   	dec    %bx
    6cb9:	45                   	inc    %bp
    6cba:	76 a2                	jbe    0x6c5e
    6cbc:	c0 14 e8             	rclb   $0xe8,(%si)
    6cbf:	63 05                	arpl   %ax,(%di)
    6cc1:	fc                   	cld
    6cc2:	ff                   	(bad)
    6cc3:	ff                   	ljmp   (bad)
    6cc4:	e8 14 c9             	call   0x35db
    6cc7:	e8 fa f8             	call   0x65c4
    6cca:	5b                   	pop    %bx
    6ccb:	f6 b5 85 33          	divb   0x3385(%di)
    6ccf:	b0 8e                	mov    $0x8e,%al
    6cd1:	b0 e8                	mov    $0xe8,%al
    6cd3:	fe                   	(bad)
    6cd4:	a2 b3 03             	mov    %al,0x3b3
    6cd7:	e7 6e                	out    %ax,$0x6e
    6cd9:	41                   	inc    %cx
    6cda:	13 c7                	adc    %di,%ax
    6cdc:	d0 e8                	shr    $1,%al
    6cde:	f8                   	clc
    6cdf:	f8                   	clc
    6ce0:	fa                   	cli
    6ce1:	ee                   	out    %al,(%dx)
    6ce2:	8d                   	lea    (bad),%di
    6ce3:	f9                   	stc
    6ce4:	f8                   	clc
    6ce5:	d9 52 ba             	fsts   -0x46(%bp,%si)
    6ce8:	5b                   	pop    %bx
    6ce9:	a1 22 40             	mov    0x4022,%ax
    6cec:	ba 00 e8             	mov    $0xe800,%dx
    6cef:	46                   	inc    %si
    6cf0:	c7                   	(bad)
    6cf1:	f3 20 7f 11          	repz and %bh,0x11(%bx)
    6cf5:	c7                   	(bad)
    6cf6:	a7                   	cmpsw  %es:(%di),%ds:(%si)
    6cf7:	03 88 c3 8a          	add    -0x753d(%bx,%si),%cx
    6cfb:	8a e8                	mov    %al,%ch
    6cfd:	04 d3                	add    $0xd3,%al
    6cff:	07                   	pop    %es
    6d00:	de 89 ee 22          	fimuls 0x22ee(%bx,%di)
    6d04:	eb 9c                	jmp    0x6ca2
    6d06:	b8 fd b8             	mov    $0xb8fd,%ax
    6d09:	d0 44 25             	rolb   $1,0x25(%si)
    6d0c:	0a d8                	or     %al,%bl
    6d0e:	e8 75 d9             	call   0x4686
    6d11:	52                   	push   %dx
    6d12:	d0 1e e8 fb          	rcrb   $1,-0x418
    6d16:	c7                   	(bad)
    6d17:	de ee                	fsubrp %st,%st(6)
    6d19:	ea e8 ef 01 e8       	ljmp   $0xe801,$0xefe8
    6d1e:	ef                   	out    %ax,(%dx)
    6d1f:	a3 f8 ee             	mov    %ax,0xeef8
    6d22:	01 1c                	add    %bx,(%si)
    6d24:	c0 06 1c 06 01       	rolb   $0x1,0x61c
    6d29:	08 4c 26             	or     %cl,0x26(%si)
    6d2c:	06                   	push   %es
    6d2d:	44                   	inc    %sp
    6d2e:	fc                   	cld
    6d2f:	e9 c0 01             	jmp    0x6ef2
    6d32:	1b d2                	sbb    %dx,%dx
    6d34:	fc                   	cld
    6d35:	00 74 24             	add    %dh,0x24(%si)
    6d38:	8e b0 e8 fe          	mov    -0x118(%bx,%si),%?
    6d3c:	a2 c8 bc             	mov    %al,0xbcc8
    6d3f:	0e                   	push   %cs
    6d40:	e4 75                	in     $0x75,%al
    6d42:	02 2a                	add    (%bp,%si),%ch
    6d44:	c0 8b 1e 9a 57       	rorb   $0x57,-0x65e2(%bp,%di)
    6d49:	88 47 33             	mov    %al,0x33(%bx)
    6d4c:	b8 90 57             	mov    $0x5790,%ax
    6d4f:	b8 15 9f             	mov    $0x9f15,%ax
    6d52:	9f                   	lahf
    6d53:	51                   	push   %cx
    6d54:	e8 bd e8             	call   0x5614
    6d57:	c7                   	(bad)
    6d58:	d4 24                	aam    $0x24
    6d5a:	3e 9b                	ds fwait
    6d5c:	b8 90 57             	mov    $0x5790,%ax
    6d5f:	b8 2b f6             	mov    $0xf62b,%ax
    6d62:	90                   	nop
    6d63:	89 62 e7             	mov    %sp,-0x19(%bp,%si)
    6d66:	f8                   	clc
    6d67:	5b                   	pop    %bx
    6d68:	f6 3d                	idivb  (%di)
    6d6a:	74 89                	je     0x6cf5
    6d6c:	57                   	push   %di
    6d6d:	e9 4c 7c             	jmp    0xe9bc
    6d70:	01 5e 0c             	add    %bx,0xc(%bp)
    6d73:	c7                   	(bad)
    6d74:	7c 01                	jl     0x6d77
    6d76:	0e                   	push   %cs
    6d77:	fe                   	(bad)
    6d78:	2c 2a                	sub    $0x2a,%al
    6d7a:	fe c8                	dec    %al
    6d7c:	74 34                	je     0x6db2
    6d7e:	90                   	nop
    6d7f:	32 56 0c             	xor    0xc(%bp),%dl
    6d82:	c7                   	(bad)
    6d83:	3e 01 90 32 56       	add    %dx,%ds:0x5632(%bx,%si)
    6d88:	0c c7                	or     $0xc7,%al
    6d8a:	41                   	inc    %cx
    6d8b:	01 32                	add    %si,(%bp,%si)
    6d8d:	0c 41                	or     $0x41,%al
    6d8f:	32 41 44             	xor    0x44(%bx,%di),%al
    6d92:	01 0c                	add    %cx,(%si)
    6d94:	32 44 0c             	xor    0xc(%si),%al
    6d97:	44                   	inc    %sp
    6d98:	47                   	inc    %di
    6d99:	01 32                	add    %si,(%bp,%si)
    6d9b:	0c 47                	or     $0x47,%al
    6d9d:	32 47 4c             	xor    0x4c(%bx),%al
    6da0:	01 e9                	add    %bp,%cx
    6da2:	4e                   	dec    %si
    6da3:	01 e8                	add    %bp,%ax
    6da5:	be d2 f8             	mov    $0xf8d2,%si
    6da8:	fa                   	cli
    6da9:	ee                   	out    %al,(%dx)
    6daa:	63 05                	arpl   %ax,(%di)
    6dac:	fc                   	cld
    6dad:	ff                   	(bad)
    6dae:	ff                   	ljmp   (bad)
    6daf:	e9 40 01             	jmp    0x6ef2
    6db2:	be 25 19             	mov    $0x1925,%si
    6db5:	4e                   	dec    %si
    6db6:	d0 73 80             	shlb   $1,-0x80(%bp,%di)
    6db9:	76 c6                	jbe    0x6d81
    6dbb:	01 e8                	add    %bp,%ax
    6dbd:	00 80 21 2a          	add    %al,0x2a21(%bx,%si)
    6dc1:	44                   	inc    %sp
    6dc2:	08 74 fe             	or     %dh,-0x2(%si)
    6dc5:	11 13                	adc    %dx,(%bp,%di)
    6dc7:	44                   	inc    %sp
    6dc8:	2a 44 05             	sub    0x5(%si),%al
    6dcb:	eb 14                	jmp    0x6de1
    6dcd:	57                   	push   %di
    6dce:	eb e5                	jmp    0x6db5
    6dd0:	b8 90 57             	mov    $0x5790,%ax
    6dd3:	b8 e8 28             	mov    $0x28e8,%ax
    6dd6:	e8 b8 be             	call   0x2c91
    6dd9:	57                   	push   %di
    6dda:	b8 e8 42             	mov    $0x42e8,%ax
    6ddd:	ee                   	out    %al,(%dx)
    6dde:	03 e8                	add    %ax,%bp
    6de0:	e9 01 58             	jmp    0xc5e4
    6de3:	b5 00                	mov    $0x0,%ch
    6de5:	b1 06                	mov    $0x6,%cl
    6de7:	04 ec                	add    $0xec,%al
    6de9:	ee                   	out    %al,(%dx)
    6dea:	8a 98 e0 bf          	mov    -0x4020(%bx,%si),%bl
    6dee:	ec                   	in     (%dx),%al
    6def:	c8 e8 01 11          	enter  $0x1e8,$0x11
    6df3:	00 75 18             	add    %dh,0x18(%di)
    6df6:	d0 2e 4e 63          	shrb   $1,0x634e
    6dfa:	8a 7e 98             	mov    -0x68(%bp),%bh
    6dfd:	c8 3c 32 7e          	enter  $0x323c,$0x7e
    6e01:	02 b0 32 88          	add    -0x77ce(%bx,%si),%dh
    6e05:	7e 98                	jle    0x6d9f
    6e07:	c8 d0 a7 98          	enter  $0xa7d0,$0x98
    6e0b:	c8 eb 18 b1          	enter  $0x18eb,$0xb1
    6e0f:	02 d2                	add    %dl,%dl
    6e11:	2e 63 7e c8          	arpl   %di,%cs:-0x38(%bp)
    6e15:	32 02                	xor    (%bp,%si),%al
    6e17:	32 7e 19             	xor    0x19(%bp),%bh
    6e1a:	7e 02                	jle    0x6e1e
    6e1c:	b0 19                	mov    $0x19,%al
    6e1e:	88 7e 98             	mov    %bh,-0x68(%bp)
    6e21:	c8 d2 a7 98          	enter  $0xa7d2,$0x98
    6e25:	c8 b8 39 47          	enter  $0x39b8,$0x47
    6e29:	c8 32 84 52          	enter  $0x8432,$0x52
    6e2d:	29 b0 13 2b          	sub    %si,0x2b13(%bx,%si)
    6e31:	40                   	inc    %ax
    6e32:	01 06 4a 63          	add    %ax,0x634a
    6e36:	2b d2                	sub    %dx,%dx
    6e38:	01 06 9a c0          	add    %ax,-0x3f66
    6e3c:	11 16 9c c0          	adc    %dx,-0x3f64
    6e40:	3a c4                	cmp    %ah,%al
    6e42:	b8 ff eb             	mov    $0xebff,%ax
    6e45:	34 58                	xor    $0x58,%al
    6e47:	eb 44                	jmp    0x6e8d
    6e49:	d0 af 7f 52          	shrb   $1,0x527f(%bx)
    6e4d:	8b ee                	mov    %si,%bp
    6e4f:	84 52 9b             	test   %dl,-0x65(%bp,%si)
    6e52:	b8 f4 57             	mov    $0x57f4,%ax
    6e55:	15 9f 0d             	adc    $0xd9f,%ax
    6e58:	04 7d                	add    $0x7d,%al
    6e5a:	b8 d5 2f             	mov    $0x2fd5,%ax
    6e5d:	5b                   	pop    %bx
    6e5e:	de 57 ff             	ficoms -0x1(%bx)
    6e61:	32 97 a6 f0          	xor    -0xf5a(%bx),%dl
    6e65:	1c fd                	sbb    $0xfd,%al
    6e67:	14 c6                	adc    $0xc6,%al
    6e69:	fa                   	cli
    6e6a:	57                   	push   %di
    6e6b:	33 e8                	xor    %ax,%bp
    6e6d:	e8 6c 31             	call   0x9fdc
    6e70:	06                   	push   %es
    6e71:	fe                   	(bad)
    6e72:	6c                   	insb   (%dx),%es:(%di)
    6e73:	15 0d 7d             	adc    $0x7d0d,%ax
    6e76:	d5 5b                	aad    $0x5b
    6e78:	57                   	push   %di
    6e79:	77 2f                	ja     0x6eaa
    6e7b:	5b                   	pop    %bx
    6e7c:	de 57 12             	ficoms 0x12(%bx)
    6e7f:	58                   	pop    %ax
    6e80:	15 7d 5b             	adc    $0x5b7d,%ax
    6e83:	77 5b                	ja     0x6ee0
    6e85:	57                   	push   %di
    6e86:	6a 2f                	push   $0x2f
    6e88:	5b                   	pop    %bx
    6e89:	de 57 5b             	ficoms 0x5b(%bx)
    6e8c:	77 5b                	ja     0x6ee9
    6e8e:	57                   	push   %di
    6e8f:	43                   	inc    %bx
    6e90:	eb e8                	jmp    0x6e7a
    6e92:	7e c6                	jle    0x6e5a
    6e94:	de ea                	fsubrp %st,%st(2)
    6e96:	ef                   	out    %ax,(%dx)
    6e97:	e8 72 9b             	call   0xa0c
    6e9a:	90                   	nop
    6e9b:	b8 f6 89             	mov    $0x89f6,%ax
    6e9e:	e8 49 e7             	call   0x55ea
    6ea1:	f2 3d 57 ff          	repnz cmp $0xff57,%ax
    6ea5:	81 3d 75 f8          	cmpw   $0xf875,(%di)
    6ea9:	3e 07                	ds pop %es
    6eab:	a3 b8 cc             	mov    %ax,0xccb8
    6eae:	f6 89 e8 21 eb       	testb  $0xeb,0x21e8(%bx,%di)
    6eb3:	b8 57 15             	mov    $0x1557,%ax
    6eb6:	9f                   	lahf
    6eb7:	e8 e8 d4             	call   0x43a2
    6eba:	3e e4 06             	ds in  $0x6,%al
    6ebd:	fc                   	cld
    6ebe:	b8 9a e9             	mov    $0xe99a,%ax
    6ec1:	eb 2f                	jmp    0x6ef2
    6ec3:	26 04 e8             	es add $0xe8,%al
    6ec6:	a2 bc 16             	mov    %al,0x16bc
    6ec9:	0e                   	push   %cs
    6eca:	e9 dd 07             	jmp    0x76aa
    6ecd:	07                   	pop    %es
    6ece:	ff 74 a2             	push   -0x5e(%si)
    6ed1:	bc 6e c4             	mov    $0xc46e,%sp
    6ed4:	e9 d3 07             	jmp    0x76aa
    6ed7:	ff a2 6e e9          	jmp    *-0x1692(%bp,%si)
    6edb:	07                   	pop    %es
    6edc:	7a b5                	jp     0x6e93
    6ede:	e9 c9 fe             	jmp    0x6daa
    6ee1:	e8 ea fa             	call   0x69ce
    6ee4:	b8 57 15             	mov    $0x1557,%ax
    6ee7:	9f                   	lahf
    6ee8:	e8 05 00             	call   0x6ef0
    6eeb:	eb cd                	jmp    0x6eba
    6eed:	19 fe                	sbb    %di,%si
    6eef:	eb 2f                	jmp    0x6f20
    6ef1:	8d 76 75             	lea    0x75(%bp),%si
    6ef4:	8b e9                	mov    %cx,%bp
    6ef6:	83 2e 76 37 07       	subw   $0x7,0x3776
    6efb:	a2 e9 7a             	mov    %al,0x7ae9
    6efe:	e9 fe 48             	jmp    0xb7ff
    6f01:	da e8                	(bad)
    6f03:	85 ae 46 95          	test   %bp,-0x6aba(%bp)
    6f07:	b0 08                	mov    $0x8,%al
    6f09:	00 81 46 00          	add    %al,0x46(%bx,%di)
    6f0d:	19 fe                	sbb    %di,%si
    6f0f:	83 ec 24             	sub    $0x24,%sp
    6f12:	57                   	push   %di
    6f13:	19 e6                	sbb    %sp,%si
    6f15:	e8 04 e8             	call   0x571c
    6f18:	0c fd                	or     $0xfd,%al
    6f1a:	8b f0                	mov    %ax,%si
    6f1c:	b8 cc 59             	mov    $0x59cc,%ax
    6f1f:	15 5b 5b             	adc    $0x5b5b,%ax
    6f22:	6a 5b                	push   $0x5b
    6f24:	57                   	push   %di
    6f25:	0b 2f                	or     (%bx),%bp
    6f27:	5b                   	pop    %bx
    6f28:	de 8b c6 d1          	fimuls -0x2e3a(%bp,%di)
    6f2c:	ff 5a de             	lcall  *-0x22(%bp,%si)
    6f2f:	34 57                	xor    $0x57,%al
    6f31:	60                   	pusha
    6f32:	ff ac 51 fc          	ljmp   *-0x3af(%si)
    6f36:	e8 1d b8             	call   0x2756
    6f39:	2e 5b                	cs pop %bx
    6f3b:	c7                   	(bad)
    6f3c:	96                   	xchg   %ax,%si
    6f3d:	b8 03 c3             	mov    $0xc303,%ax
    6f40:	8a 04                	mov    (%si),%al
    6f42:	5b                   	pop    %bx
    6f43:	5b                   	pop    %bx
    6f44:	6a 5b                	push   $0x5b
    6f46:	b9 ef bb             	mov    $0xbbef,%cx
    6f49:	b9 9f 56             	mov    $0x569f,%cx
    6f4c:	40                   	inc    %ax
    6f4d:	6f                   	outsw  %ds:(%si),(%dx)
    6f4e:	29 93 1c b8          	sub    %dx,-0x47e4(%bp,%di)
    6f52:	4f                   	dec    %di
    6f53:	58                   	pop    %ax
    6f54:	7d 77                	jge    0x6fcd
    6f56:	57                   	push   %di
    6f57:	2f                   	das
    6f58:	de 5b d6             	ficomps -0x2a(%bp,%di)
    6f5b:	2e 5b                	cs pop %bx
    6f5d:	c7                   	(bad)
    6f5e:	b1 06                	mov    $0x6,%cl
    6f60:	04 ec                	add    $0xec,%al
    6f62:	eb 52                	jmp    0x6fb6
    6f64:	8b 8b 38 ee          	mov    -0x11c8(%bp,%di),%cx
    6f68:	2b a9 4f b2          	sub    -0x4db1(%bx,%di),%bp
    6f6c:	b9 e7 d8             	mov    $0xd8e7,%cx
    6f6f:	de f0                	fdivp  %st,%st(0)
    6f71:	b6 5e                	mov    $0x5e,%dh
    6f73:	04 e8                	add    $0xe8,%al
    6f75:	ac                   	lods   %ds:(%si),%al
    6f76:	51                   	push   %cx
    6f77:	fc                   	cld
    6f78:	f8                   	clc
    6f79:	e8 76 2e             	call   0x9df2
    6f7c:	5b                   	pop    %bx
    6f7d:	c7 b8 58             	xbegin 0xc838,(bad)
    6f80:	58                   	pop    %ax
    6f81:	77 2f                	ja     0x6fb2
    6f83:	5b                   	pop    %bx
    6f84:	2e c7 06 69 2e 5b c7 	movw   $0xc75b,%cs:0x2e69
    6f8b:	b8 ff 97             	mov    $0x97ff,%ax
    6f8e:	f0 fd                	lock std
    6f90:	4e                   	dec    %si
    6f91:	dc 51 8a             	fcoml  -0x76(%bx,%di)
    6f94:	95                   	xchg   %ax,%bp
    6f95:	81 2b 2b f6          	subw   $0xf62b,(%bp,%di)
    6f99:	2b db                	sub    %bx,%bx
    6f9b:	53                   	push   %bx
    6f9c:	52                   	push   %dx
    6f9d:	e8 82 30             	call   0xa022
    6fa0:	83 c4 08             	add    $0x8,%sp
    6fa3:	77 5b                	ja     0x7000
    6fa5:	c7                   	(bad)
    6fa6:	69 5b b8 47 2e       	imul   $0x2e47,-0x48(%bp,%di),%bx
    6fab:	5b                   	pop    %bx
    6fac:	c7 b8 63             	xbegin 0xd367,(bad)
    6faf:	58                   	pop    %ax
    6fb0:	2f                   	das
    6fb1:	2e 06                	cs push %es
    6fb3:	2e c7                	cs (bad)
    6fb5:	ff                   	(bad)
    6fb6:	3a 2e c7 ff          	cmp    -0x39,%ch
    6fba:	f0 4e                	lock dec %si
    6fbc:	51                   	push   %cx
    6fbd:	95                   	xchg   %ax,%bp
    6fbe:	2b 46 dc             	sub    -0x24(%bp),%ax
    6fc1:	50                   	push   %ax
    6fc2:	8b 85 74 2b          	mov    0x2b74(%di),%ax
    6fc6:	52                   	push   %dx
    6fc7:	b8 00 b1             	mov    $0xb100,%ax
    6fca:	e8 55 30             	call   0xa022
    6fcd:	c4 77 c7             	les    -0x39(%bx),%si
    6fd0:	5b                   	pop    %bx
    6fd1:	47                   	inc    %di
    6fd2:	5b                   	pop    %bx
    6fd3:	b8 58 2e             	mov    $0x2e58,%ax
    6fd6:	1a 2e c7 ff          	sbb    -0x39,%ch
    6fda:	f0 6f                	lock outsw %ds:(%si),(%dx)
    6fdc:	58                   	pop    %ax
    6fdd:	2e 2e ff 2e ff 4e    	cs ljmp *%cs:0x4eff
    6fe3:	0d 2e c7             	or     $0xc72e,%ax
    6fe6:	ff 8a 85 7a          	decw   0x7a85(%bp,%si)
    6fea:	52                   	push   %dx
    6feb:	b1 b8                	mov    $0xb8,%cl
    6fed:	c0 8b f6 f1 8a       	rorb   $0x8a,-0xe0a(%bp,%di)
    6ff2:	b1 53                	mov    $0x53,%cl
    6ff4:	f9                   	stc
    6ff5:	e7 d8                	out    %ax,$0xd8
    6ff7:	de f0                	fdivp  %st,%st(0)
    6ff9:	86 74 2e             	xchg   %dh,0x2e(%si)
    6ffc:	ff 2e ff 4e          	ljmp   *0x4eff
    7000:	f0 2d 5b 57          	lock sub $0x575b,%ax
    7004:	77 57                	ja     0x705d
    7006:	eb 7e                	jmp    0x7086
    7008:	b9 ab bb             	mov    $0xbbab,%cx
    700b:	b9 9f 6a             	mov    $0x6a9f,%cx
    700e:	40                   	inc    %ax
    700f:	6f                   	outsw  %ds:(%si),(%dx)
    7010:	29 89 19 8b          	sub    %cx,-0x74e7(%bx,%di)
    7014:	c6                   	(bad)
    7015:	b9 f8 16             	mov    $0x16f8,%cx
    7018:	1c e1                	sbb    $0xe1,%al
    701a:	03 06 8a c2          	add    -0x3d76,%ax
    701e:	2e 2e 18 1c          	cs sbb %bl,%cs:(%si)
    7022:	01 4c 83             	add    %cx,-0x7d(%si)
    7025:	1c 06                	sbb    $0x6,%al
    7027:	01 08                	add    %cx,(%bx,%si)
    7029:	1e                   	push   %ds
    702a:	8a 1e 8e 04          	mov    0x48e,%bl
    702e:	07                   	pop    %es
    702f:	89 e7                	mov    %sp,%di
    7031:	d8 de                	fcomp  %st(6)
    7033:	f0 84 5f 83          	lock test %bl,-0x7d(%bx)
    7037:	0d 3f 5b             	or     $0x5b3f,%ax
    703a:	90                   	nop
    703b:	f5                   	cmc
    703c:	2d 5b 57             	sub    $0x575b,%ax
    703f:	7e 40                	jle    0x7081
    7041:	57                   	push   %di
    7042:	01 be 5e 20          	add    %di,0x205e(%bp)
    7046:	eb a0                	jmp    0x6fe8
    7048:	8e 04                	mov    (%si),%es
    704a:	38 87 8e 04          	cmp    %al,0x48e(%bx)
    704e:	74 0d                	je     0x705d
    7050:	ff 36 8c 5f          	push   0x5f8c
    7054:	0d 5b f5             	or     $0xf55b,%ax
    7057:	5b                   	pop    %bx
    7058:	7e d7                	jle    0x7031
    705a:	2d 5b 57             	sub    $0x575b,%ax
    705d:	ff                   	(bad)
    705e:	fe 83 0a ff          	incb   -0xf6(%bp,%di)
    7062:	81 1b 7c de          	sbbw   $0xde7c,(%bp,%di)
    7066:	b8 7a 58             	mov    $0x587a,%ax
    7069:	2e 2e 4e             	cs cs dec %si
    706c:	2e ff 85 81 2d       	incw   %cs:0x2d81(%di)
    7071:	57                   	push   %di
    7072:	57                   	push   %di
    7073:	7e ab                	jle    0x7020
    7075:	b9 6a b8             	mov    $0xb86a,%cx
    7078:	ef                   	out    %ax,(%dx)
    7079:	25 d8 75             	and    $0x75d8,%ax
    707c:	b4 1c                	mov    $0x1c,%ah
    707e:	06                   	push   %es
    707f:	01 62 1b             	add    %sp,0x1b(%bp,%si)
    7082:	46                   	inc    %si
    7083:	fc                   	cld
    7084:	e8 36 ff             	call   0x6fbd
    7087:	d3 d3                	rcl    %cl,%bx
    7089:	c9                   	leave
    708a:	81 06 ba 01 8c 06    	addw   $0x68c,0x1ba
    7090:	4b                   	dec    %bx
    7091:	31 a6 e8 f8          	xor    %sp,-0x718(%bp)
    7095:	a9 fa 06             	test   $0x6fa,%ax
    7098:	01 e0                	add    %sp,%ax
    709a:	9e                   	sahf
    709b:	33 f6                	xor    %si,%si
    709d:	2a 15                	sub    (%di),%dl
    709f:	e8 42 f7             	call   0x67e4
    70a2:	a2 7a fe             	mov    %al,0xfe7a
    70a5:	da f8                	(bad)
    70a7:	a9 fa 06             	test   $0x6fa,%ax
    70aa:	01 0d                	add    %cx,(%di)
    70ac:	00 f8                	add    %bh,%al
    70ae:	a9 fa 06             	test   $0x6fa,%ax
    70b1:	01 f5                	add    %si,%bp
    70b3:	a0 46 5d             	mov    0x5d46,%al
    70b6:	31 a1 06 04          	xor    %sp,0x406(%bx,%di)
    70ba:	76 f8                	jbe    0x70b4
    70bc:	3d b0 fe             	cmp    $0xfeb0,%ax
    70bf:	05 56 e4             	add    $0xe456,%ax
    70c2:	e9 06 00             	jmp    0x70cb
    70c5:	75 1f                	jne    0x70e6
    70c7:	b8 81 58             	mov    $0x5881,%ax
    70ca:	2e e6 04             	cs out %al,$0x4
    70cd:	0c f8                	or     $0xf8,%al
    70cf:	a9 f2 57             	test   $0x57f2,%ax
    70d2:	9e                   	sahf
    70d3:	eb a1                	jmp    0x7076
    70d5:	03 e9                	add    %cx,%bp
    70d7:	58                   	pop    %ax
    70d8:	00 06 d5 00          	add    %al,0xd5
    70dc:	f8                   	clc
    70dd:	a9 f2 57             	test   $0x57f2,%ax
    70e0:	08 52 09             	or     %dl,0x9(%bp,%si)
    70e3:	e9 c0 00             	jmp    0x71a6
    70e6:	b8 64 4e             	mov    $0x4e64,%ax
    70e9:	8e 25                	mov    (%di),%fs
    70eb:	91                   	xchg   %ax,%cx
    70ec:	96                   	xchg   %ax,%si
    70ed:	8b f0                	mov    %ax,%si
    70ef:	8d 08                	lea    (%bx,%si),%cx
    70f1:	fb                   	sti
    70f2:	8d 0b                	lea    (%bp,%di),%cx
    70f4:	e4 3e                	in     $0x3e,%al
    70f6:	06                   	push   %es
    70f7:	f6 30                	divb   (%bx,%si)
    70f9:	02 75 06             	add    0x6(%di),%dh
    70fc:	8d 08                	lea    (%bx,%si),%cx
    70fe:	fb                   	sti
    70ff:	8d 17                	lea    (%bx),%dx
    7101:	f9                   	stc
    7102:	fd                   	std
    7103:	c2 c3 1c             	ret    $0x1cc3
    7106:	06                   	push   %es
    7107:	06                   	push   %es
    7108:	08 26 44 1f          	or     %ah,0x1f44
    710c:	4e                   	dec    %si
    710d:	8e 25                	mov    (%di),%fs
    710f:	63 0d                	arpl   %cx,(%di)
    7111:	88 c3                	mov    %al,%bl
    7113:	8a 8a e8 8a          	mov    -0x7518(%bp,%si),%cl
    7117:	87 a6 58 b9          	xchg   %sp,-0x46a8(%bp)
    711b:	11 76 c6             	adc    %si,-0x3a(%bp)
    711e:	77 30                	ja     0x7150
    7120:	b8 fb bf             	mov    $0xbffb,%ax
    7123:	6f                   	outsw  %ds:(%si),(%dx)
    7124:	8b 44 96             	mov    -0x6a(%si),%ax
    7127:	04 30                	add    $0x30,%al
    7129:	a2 dd 58             	mov    %al,0x58dd
    712c:	b8 da 58             	mov    $0x58da,%ax
    712f:	e6 0c                	out    %al,$0xc
    7131:	a9 57 eb             	test   $0xeb57,%ax
    7134:	03 58 06             	add    0x6(%bx,%si),%bx
    7137:	00 39                	add    %bh,(%bx,%di)
    7139:	a1 03 e9             	mov    0xe903,%ax
    713c:	74 12                	je     0x7150
    713e:	f8                   	clc
    713f:	f2 08 09             	repnz or %cl,(%bx,%di)
    7142:	c0 b8 4e 51 e8       	sarb   $0xe8,0x514e(%bx,%si)
    7147:	19 1d                	sbb    %bx,(%di)
    7149:	eb 03                	jmp    0x714e
    714b:	58                   	pop    %ax
    714c:	06                   	push   %es
    714d:	00 50 15             	add    %dl,0x15(%bx,%si)
    7150:	88 8a e8 87          	mov    %cl,-0x7818(%bp,%si)
    7154:	58                   	pop    %ax
    7155:	11 c6                	adc    %ax,%si
    7157:	8a 58 04             	mov    0x4(%bx,%si),%bl
    715a:	41                   	inc    %cx
    715b:	a2 d3 58             	mov    %al,0x58d3
    715e:	b8 ce 58             	mov    $0x58ce,%ax
    7161:	0c 57                	or     $0x57,%al
    7163:	03 06 39 03          	add    0x339,%ax
    7167:	74 f8                	je     0x7161
    7169:	08 07                	or     %al,(%bx)
    716b:	a1 e9 12             	mov    0x12e9,%ax
    716e:	f2 09 b8 51 19       	repnz or %di,0x1951(%bx,%si)
    7173:	eb 58                	jmp    0x71cd
    7175:	00 15                	add    %dl,(%di)
    7177:	8a 87 e7 1c          	mov    0x1ce7(%bx),%al
    717b:	39 03                	cmp    %ax,(%bp,%di)
    717d:	74 f8                	je     0x7177
    717f:	08 1e 15 f2          	or     %bl,-0xdeb
    7183:	1d 40 b8             	sbb    $0xb840,%ax
    7186:	e8 b8 c7             	call   0x3941
    7189:	58                   	pop    %ax
    718a:	b8 c2 58             	mov    $0x58c2,%ax
    718d:	57                   	push   %di
    718e:	06                   	push   %es
    718f:	03 f8                	add    %ax,%di
    7191:	07                   	pop    %es
    7192:	e9 f2 b8             	jmp    0x2a87
    7195:	19 db                	sbb    %bx,%bx
    7197:	a0 33 f6             	mov    0xf633,%al
    719a:	2a 15                	sub    (%di),%dl
    719c:	b8 19 58             	mov    $0x5819,%ax
    719f:	15 87 1c             	adc    $0x1c87,%ax
    71a2:	03 b9 1f 4e          	add    0x4e1f(%bx,%di),%di
    71a6:	8a 87 b8 1c          	mov    0x1cb8(%bx),%al
    71aa:	03 f8                	add    %ax,%di
    71ac:	1e                   	push   %ds
    71ad:	f2 40                	repnz inc %ax
    71af:	ef                   	out    %ax,(%dx)
    71b0:	14 08                	adc    $0x8,%al
    71b2:	07                   	pop    %es
    71b3:	00 0c                	add    %cl,(%si)
    71b5:	88 ef                	mov    %ch,%bh
    71b7:	02 3b                	add    (%bp,%di),%bh
    71b9:	57                   	push   %di
    71ba:	d3 8d 08 fb          	rorw   %cl,-0x4f8(%di)
    71be:	8d 8a 5b 6a          	lea    0x6a5b(%bp,%si),%cx
    71c2:	15 9f 9f             	adc    $0x9f9f,%ax
    71c5:	51                   	push   %cx
    71c6:	3f                   	aas
    71c7:	d0 e8                	shr    $1,%al
    71c9:	f8                   	clc
    71ca:	0b 9f 32 e8          	or     -0x17ce(%bx),%bx
    71ce:	fa                   	cli
    71cf:	0c fa                	or     $0xfa,%al
    71d1:	ca 76 0f             	lret   $0xf76
    71d4:	3c a1                	cmp    $0xa1,%al
    71d6:	50                   	push   %ax
    71d7:	00 d9                	add    %bl,%cl
    71d9:	f7 b8 40 5b          	idivw  0x5b40(%bx,%si)
    71dd:	22 ba e8 c7          	and    -0x3818(%bp,%si),%bh
    71e1:	20 e8                	and    %ch,%al
    71e3:	e5 0c                	in     $0xc,%ax
    71e5:	a1 0d 37             	mov    0x370d,%ax
    71e8:	51                   	push   %cx
    71e9:	52                   	push   %dx
    71ea:	52                   	push   %dx
    71eb:	8e 02                	mov    (%bp,%si),%es
    71ed:	e8 46 3f             	call   0xb136
    71f0:	d0 e8                	shr    $1,%al
    71f2:	e8 e9 10             	call   0x82de
    71f5:	b8 39 13             	mov    $0x1339,%ax
    71f8:	b8 40 5b             	mov    $0x5b40,%ax
    71fb:	22 e8                	and    %al,%ch
    71fd:	df 10                	fists  (%bx,%si)
    71ff:	52                   	push   %dx
    7200:	f0 b8 e7 01          	lock mov $0x1e7,%ax
    7204:	40                   	inc    %ax
    7205:	5b                   	pop    %bx
    7206:	22 e8                	and    %al,%ch
    7208:	d4 10                	aam    $0x10
    720a:	50                   	push   %ax
    720b:	19 bf 73 b4          	sbb    %di,-0x4b8d(%bx)
    720f:	3e 4f                	ds dec %di
    7211:	1a fe                	sbb    %dh,%bh
    7213:	c6 06 60 ac 00       	movb   $0x0,-0x53a0
    7218:	2b ff                	sub    %di,%di
    721a:	7e fe                	jle    0x721a
    721c:	9e                   	sahf
    721d:	be 1a c1             	mov    $0xc11a,%si
    7220:	81 fe b4 c1          	cmp    $0xc1b4,%si
    7224:	73 4e                	jae    0x7274
    7226:	83 3c 8d             	cmpw   $0xff8d,(%si)
    7229:	06                   	push   %es
    722a:	89 74 11             	mov    %si,0x11(%si)
    722d:	44                   	inc    %sp
    722e:	44                   	inc    %sp
    722f:	eb f9                	jmp    0x722a
    7231:	74 06                	je     0x7239
    7233:	f7 fe                	idiv   %si
    7235:	b8 47 eb             	mov    $0xeb47,%ax
    7238:	e7 b8                	out    %ax,$0xb8
    723a:	e3 58                	jcxz   0x7294
    723c:	2e 2e 85 2d          	cs test %bp,%cs:(%di)
    7240:	57                   	push   %di
    7241:	ab                   	stos   %ax,%es:(%di)
    7242:	ee                   	out    %al,(%dx)
    7243:	2b 5b 8b             	sub    -0x75(%bp,%di),%bx
    7246:	d1 c7                	rol    $1,%di
    7248:	b9 16 e1             	mov    $0xe116,%cx
    724b:	06                   	push   %es
    724c:	e9 b8 8b             	jmp    0xfe07
    724f:	03 9f 14 c1          	add    -0x3eec(%bx),%bx
    7253:	22 9c fd d0          	and    -0x2f03(%si),%bl
    7257:	25 d8 75             	and    $0x75d8,%ax
    725a:	85 2d                	test   %bp,(%di)
    725c:	57                   	push   %di
    725d:	ab                   	stos   %ax,%es:(%di)
    725e:	92                   	xchg   %ax,%dx
    725f:	2b 5b 8b             	sub    -0x75(%bp,%di),%bx
    7262:	b8 f6 58             	mov    $0x58f6,%ax
    7265:	2e 2d ab 2b          	cs sub $0x2bab,%ax
    7269:	8b c7                	mov    %di,%ax
    726b:	85 2b                	test   %bp,(%bp,%di)
    726d:	5b                   	pop    %bx
    726e:	8b 7e 57             	mov    0x57(%bp),%di
    7271:	be 2f 76             	mov    $0x762f,%si
    7274:	8b 83 fe 00          	mov    0xfe(%bp,%di),%ax
    7278:	75 41                	jne    0x72bb
    727a:	80 b8 74 1a 76       	cmpb   $0x76,0x1a74(%bx,%si)
    727f:	3b bf 03 00          	cmp    0x3(%bx),%di
    7283:	be 3a 11             	mov    $0x113a,%si
    7286:	81 fe 5e 11          	cmp    $0x115e,%si
    728a:	73 2f                	jae    0x72bb
    728c:	80 2a 08             	subb   $0x8,(%bp,%si)
    728f:	fe                   	(bad)
    7290:	13 2a                	adc    (%bp,%si),%bp
    7292:	05 14 eb             	add    $0xeb14,%ax
    7295:	b8 fe 74             	mov    $0x74fe,%ax
    7298:	06                   	push   %es
    7299:	f7 fe                	idiv   %si
    729b:	0c eb                	or     $0xeb,%al
    729d:	c7                   	(bad)
    729e:	e7 e7                	out    %ax,$0xe7
    72a0:	b5 14                	mov    $0x14,%ch
    72a2:	b5 00                	mov    $0x0,%ch
    72a4:	11 2d                	adc    %bp,(%di)
    72a6:	ab                   	stos   %ax,%es:(%di)
    72a7:	2b 8b c7 86          	sub    -0x7939(%bp,%di),%cx
    72ab:	2b 5b 8b             	sub    -0x75(%bp,%di),%bx
    72ae:	b8 fb 58             	mov    $0x58fb,%ax
    72b1:	2d 2b c7             	sub    $0xc72b,%ax
    72b4:	2b 8b 57 39          	sub    0x3957(%bp,%di),%cx
    72b8:	2b 5b 8b             	sub    -0x75(%bp,%di),%bx
    72bb:	8a 04                	mov    (%si),%al
    72bd:	07                   	pop    %es
    72be:	89 22                	mov    %sp,(%bp,%si)
    72c0:	22 fd                	and    %ch,%bh
    72c2:	25 75 2d             	and    $0x2d75,%ax
    72c5:	ab                   	stos   %ax,%es:(%di)
    72c6:	2b 8b f6 2e          	sub    0x2ef6(%bp,%di),%cx
    72ca:	ab                   	stos   %ax,%es:(%di)
    72cb:	25 2b 5b             	and    $0x5b2b,%ax
    72ce:	8b b8 8b f6          	mov    -0x975(%bx,%si),%di
    72d2:	2e ab                	cs stos %ax,%es:(%di)
    72d4:	fe                   	(bad)
    72d5:	e6 46                	out    %al,$0x46
    72d7:	e8 ff d3             	call   0x46d9
    72da:	81 ba 8c 4b a6 f8    	cmpw   $0xf8a6,0x4b8c(%bp,%si)
    72e0:	fa                   	cli
    72e1:	01 9e 02 8a          	add    %bx,-0x75fe(%bp)
    72e5:	73 87                	jae    0x726e
    72e7:	ff 44 38             	incw   0x38(%si)
    72ea:	b0 b2                	mov    $0xb2,%al
    72ec:	eb 82                	jmp    0x7270
    72ee:	32 a1 4a 59          	xor    0x594a(%bx,%di),%ah
    72f2:	39 87 4e 59          	cmp    %ax,0x594e(%bx)
    72f6:	7d 09                	jge    0x7301
    72f8:	b8 e2 e5             	mov    $0xe5e2,%ax
    72fb:	03 ea                	add    %dx,%bp
    72fd:	44                   	inc    %sp
    72fe:	08 75 e7             	or     %dh,-0x19(%di)
    7301:	26 e2 ff             	es loop 0x7303
    7304:	38 06 71 59          	cmp    %al,0x5971
    7308:	73 03                	jae    0x730d
    730a:	a2 71 59             	mov    %al,0x5971
    730d:	80 3e 71 59 08       	cmpb   $0x8,0x5971
    7312:	75 18                	jne    0x732c
    7314:	bb 85 52             	mov    $0x5285,%bx
    7317:	81 fb e5 55          	cmp    $0x55e5,%bx
    731b:	73 0f                	jae    0x732c
    731d:	80 3f e6             	cmpb   $0xe6,(%bx)
    7320:	01 01                	add    %ax,(%bx,%di)
    7322:	83 c3 20             	add    $0x20,%bx
    7325:	eb f0                	jmp    0x7317
    7327:	c6 06 71 59 07       	movb   $0x7,0x5971
    732c:	06                   	push   %es
    732d:	ff 56 5e             	call   *0x5e(%bp)
    7330:	03 19                	add    (%bx,%di),%bx
    7332:	e6 e8                	out    %al,$0xe8
    7334:	0c 00                	or     $0x0,%al
    7336:	2c 7c                	sub    $0x7c,%al
    7338:	10 ff                	adc    %bh,%bh
    733a:	36 4a                	ss dec %dx
    733c:	59                   	pop    %cx
    733d:	4a                   	dec    %dx
    733e:	83 4a b8 01          	orw    $0x1,-0x48(%bp,%si)
    7342:	9f                   	lahf
    7343:	8d                   	lea    (bad),%bp
    7344:	e8 b3 91             	call   0x4fa
    7347:	eb 13                	jmp    0x735c
    7349:	ff 36 4a 59          	push   0x594a
    734d:	06                   	push   %es
    734e:	04 77                	add    $0x77,%al
    7350:	99                   	cwtd
    7351:	33 c2                	xor    %dx,%ax
    7353:	2b c2                	sub    %dx,%ax
    7355:	3e 06                	ds push %es
    7357:	b8 e9 2f             	mov    $0x2fe9,%ax
    735a:	b3 91                	mov    $0x91,%bl
    735c:	a3 4a 59             	mov    %ax,0x594a
    735f:	06                   	push   %es
    7360:	f6 ca 72             	test   $0x72,%dl
    7363:	88 01                	mov    %al,(%bx,%di)
    7365:	1d 8c e8             	sbb    $0xe88c,%ax
    7368:	d6                   	(bad)
    7369:	16                   	push   %ss
    736a:	e3 04                	jcxz   0x7370
    736c:	57                   	push   %di
    736d:	2b 7e e4             	sub    -0x1c(%bp),%di
    7370:	40                   	inc    %ax
    7371:	46                   	inc    %si
    7372:	ff                   	(bad)
    7373:	ff                   	ljmp   (bad)
    7374:	e8 f8 ee             	call   0x626f
    7377:	f9                   	stc
    7378:	fc                   	cld
    7379:	cf                   	iret
    737a:	3a d8                	cmp    %al,%bl
    737c:	a9 e8 5b             	test   $0x5be8,%ax
    737f:	c2 e8 8e             	ret    $0x8ee8
    7382:	00 eb                	add    %ch,%bl
    7384:	7a 8b                	jp     0x7311
    7386:	fe                   	(bad)
    7387:	75 80                	jne    0x7309
    7389:	74 0b                	je     0x7396
    738b:	b8 30 5a             	mov    $0x5a30,%ax
    738e:	b8 9f 51             	mov    $0x519f,%ax
    7391:	bd c7 24             	mov    $0x24c7,%bp
    7394:	7e e2                	jle    0x7378
    7396:	e4 40                	in     $0x40,%al
    7398:	46                   	inc    %si
    7399:	ff                   	(bad)
    739a:	ff                   	(bad)
    739b:	b8 30 5a             	mov    $0x5a30,%ax
    739e:	b8 ff 0e             	mov    $0xeff,%ax
    73a1:	90                   	nop
    73a2:	e8 23 e1             	call   0x54c8
    73a5:	76 47                	jbe    0x73ee
    73a7:	43                   	inc    %bx
    73a8:	75 0b                	jne    0x73b5
    73aa:	51                   	push   %cx
    73ab:	f0 4a                	lock dec %dx
    73ad:	77 50                	ja     0x73ff
    73af:	0e                   	push   %cs
    73b0:	2c fe                	sub    $0xfe,%al
    73b2:	1a fe                	sbb    %dh,%bh
    73b4:	74 90                	je     0x7346
    73b6:	1d fe 74             	sbb    $0x74fe,%ax
    73b9:	90                   	nop
    73ba:	21 fe                	and    %di,%si
    73bc:	74 90                	je     0x734e
    73be:	26 fe                	es (bad)
    73c0:	74 90                	je     0x7352
    73c2:	2a c8                	sub    %al,%cl
    73c4:	34 32                	xor    $0x32,%al
    73c6:	2b c8                	sub    %ax,%cx
    73c8:	34 32                	xor    $0x32,%al
    73ca:	2c eb                	sub    $0xeb,%al
    73cc:	32 e8                	xor    %al,%ch
    73ce:	95                   	xchg   %ax,%bp
    73cf:	cc                   	int3
    73d0:	8b b1 14 2b          	mov    0x2b14(%bx,%di),%si
    73d4:	f7 b9 3a e8          	idivw  -0x17c6(%bx,%di)
    73d8:	06                   	push   %es
    73d9:	09 eb                	or     %bp,%bx
    73db:	23 e8                	and    %ax,%bp
    73dd:	78 c0                	js     0x739f
    73df:	33 2a                	xor    (%bp,%si),%bp
    73e1:	b8 1c eb             	mov    $0xeb1c,%ax
    73e4:	9b                   	fwait
    73e5:	f7 b9 3a e8          	idivw  -0x17c6(%bx,%di)
    73e9:	6d                   	insw   (%dx),%es:(%di)
    73ea:	b0 eb                	mov    $0xeb,%al
    73ec:	12 e8                	adc    %al,%ch
    73ee:	52                   	push   %dx
    73ef:	09 eb                	or     %bp,%bx
    73f1:	0d e8 d7             	or     $0xd7e8,%ax
    73f4:	f5                   	cmc
    73f5:	eb 08                	jmp    0x73ff
    73f7:	bf ff ff             	mov    $0xffff,%di
    73fa:	e4 40                	in     $0x40,%al
    73fc:	46                   	inc    %si
    73fd:	01 1d                	add    %bx,(%di)
    73ff:	0b 74 c6             	or     -0x3a(%si),%si
    7402:	82 f7 b9             	xor    $0xb9,%bh
    7405:	3a e8                	cmp    %al,%ch
    7407:	41                   	inc    %cx
    7408:	d5 24                	aad    $0x24
    740a:	7e a9                	jle    0x73b5
    740c:	e8 00 c1             	call   0x350f
    740f:	8b 88 f7 b9          	mov    -0x4609(%bx,%si),%cx
    7413:	3a e8                	cmp    %al,%ch
    7415:	73 fc                	jae    0x7413
    7417:	a1 0d 37             	mov    0x370d,%ax
    741a:	51                   	push   %cx
    741b:	52                   	push   %dx
    741c:	52                   	push   %dx
    741d:	8e 02                	mov    (%bp,%si),%es
    741f:	e8 46 c7             	call   0x3b68
    7422:	f3 20 0b             	repz and %cl,(%bp,%di)
    7425:	90                   	nop
    7426:	e8 de bb             	call   0x3007
    7429:	c7 00 01 02          	movw   $0x201,(%bx,%si)
    742d:	45                   	inc    %bp
    742e:	0a e8                	or     %al,%ch
    7430:	89 fd                	mov    %di,%bp
    7432:	96                   	xchg   %ax,%si
    7433:	03 8a 5b 6a          	add    0x6a5b(%bp,%si),%cx
    7437:	b9 bb 42             	mov    $0x42bb,%cx
    743a:	5a                   	pop    %dx
    743b:	b8 9f 44             	mov    $0x449f,%ax
    743e:	0a e8                	or     %al,%ch
    7440:	d9 d0                	fnop
    7442:	e8 c7 ee             	call   0x630c
    7445:	9d                   	popf
    7446:	17                   	pop    %ss
    7447:	e8 c8 c0             	call   0x3512
    744a:	06                   	push   %es
    744b:	b8 e9 2f             	mov    $0x2fe9,%ax
    744e:	bc fa 2f             	mov    $0x2ffa,%sp
    7451:	bc f3 f8             	mov    $0xf8f3,%sp
    7454:	01 c0                	add    %ax,%ax
    7456:	1c 01                	sbb    $0x1,%al
    7458:	4c                   	dec    %sp
    7459:	06                   	push   %es
    745a:	fc                   	cld
    745b:	c0 1b fc             	rcrb   $0xfc,(%bp,%di)
    745e:	74 8e                	je     0x73ee
    7460:	11 f5                	adc    %si,%bp
    7462:	c3                   	ret
    7463:	04 8d                	add    $0x8d,%al
    7465:	56                   	push   %si
    7466:	f6 0e e6 04 0c       	testb  $0xc,0x4e6
    746b:	8b 0a                	mov    (%bp,%si),%cx
    746d:	2d e4 eb             	sub    $0xebe4,%ax
    7470:	fa                   	cli
    7471:	1d 19 e9             	sbb    $0xe919,%ax
    7474:	0a b8 48 5a          	or     0x5a48(%bx,%si),%bh
    7478:	b8 ff 0e             	mov    $0xeff,%ax
    747b:	0c 8b                	or     $0x8b,%al
    747d:	57                   	push   %di
    747e:	00 83 a1 0c          	add    %al,0xca1(%bp,%di)
    7482:	ff 81 8c a6          	incw   -0x5974(%bx,%di)
    7486:	fa                   	cli
    7487:	9e                   	sahf
    7488:	8a 87 44 b0          	mov    -0x4fbc(%bx),%al
    748c:	eb 32                	jmp    0x74c0
    748e:	57                   	push   %di
    748f:	de be 0f e6          	fidivrs -0x19f1(%bp)
    7493:	e8 00 b1             	call   0x2596
    7496:	2c 5b                	sub    $0x5b,%al
    7498:	5b                   	pop    %bx
    7499:	03 45 37             	add    0x37(%di),%ax
    749c:	03 58 06             	add    0x6(%bx,%si),%bx
    749f:	0e                   	push   %cs
    74a0:	b8 4f 5a             	mov    $0x5a4f,%ax
    74a3:	b8 06 77             	mov    $0x7706,%ax
    74a6:	33 0e 06 8a          	xor    -0x75fa,%cx
    74aa:	b8 28 b8             	mov    $0xb828,%ax
    74ad:	00 83 b8 5e          	add    %al,0x5eb8(%bp,%di)
    74b1:	ff ca                	dec    %dx
    74b3:	5e                   	pop    %si
    74b4:	52                   	push   %dx
    74b5:	45                   	inc    %bp
    74b6:	a2 14 63             	mov    %al,0x6314
    74b9:	fc                   	cld
    74ba:	ff 0c                	decw   (%si)
    74bc:	00 2c                	add    %ch,(%si)
    74be:	11 09                	adc    %cx,(%bx,%di)
    74c0:	ff 0e 0c 8b          	decw   -0x74f4
    74c4:	ae                   	scas   %es:(%di),%al
    74c5:	2c 5b                	sub    $0x5b,%al
    74c7:	eb 07                	jmp    0x74d0
    74c9:	b8 56 5a             	mov    $0x5a56,%ax
    74cc:	b8 e8 04             	mov    $0x4e8,%ax
    74cf:	e5 ff                	in     $0xff,%ax
    74d1:	8c fa                	mov    %?,%dx
    74d3:	8a 44 eb             	mov    -0x15(%si),%al
    74d6:	57                   	push   %di
    74d7:	be e6 00             	mov    $0xe6,%si
    74da:	2c 22                	sub    $0x22,%al
    74dc:	56                   	push   %si
    74dd:	e6 83                	out    %al,$0x83
    74df:	0d 8b 5b             	or     $0x5b8b,%ax
    74e2:	4a                   	dec    %dx
    74e3:	5f                   	pop    %di
    74e4:	4c                   	dec    %sp
    74e5:	29 c7                	sub    %ax,%di
    74e7:	f0 58                	lock pop %ax
    74e9:	2e 5a                	cs pop %dx
    74eb:	b8 9f 8b             	mov    $0x8b9f,%ax
    74ee:	5b                   	pop    %bx
    74ef:	4a                   	dec    %dx
    74f0:	5f                   	pop    %di
    74f1:	ff 28                	ljmp   *(%bx,%si)
    74f3:	c7                   	(bad)
    74f4:	f0 51                	lock push %cx
    74f6:	2b dc                	sub    %sp,%bx
    74f8:	8b 74 52             	mov    0x52(%si),%si
    74fb:	de b8 06 77          	fidivrs 0x7706(%bx,%si)
    74ff:	33 52 00             	xor    0x0(%bp,%si),%dx
    7502:	e8 30 77             	call   0xec35
    7505:	75 2c                	jne    0x7533
    7507:	c4 c7 47 b8          	(bad)
    750b:	2e 2e ff 6f 2e       	cs ljmp *%cs:0x2e(%bx)
    7510:	e0 28                	loopne 0x753a
    7512:	c7                   	(bad)
    7513:	f0 51                	lock push %cx
    7515:	78 5a                	js     0x7571
    7517:	9f                   	lahf
    7518:	5b                   	pop    %bx
    7519:	5f                   	pop    %di
    751a:	28 f0                	sub    %dh,%al
    751c:	2b d3                	sub    %bx,%dx
    751e:	28 c7                	sub    %al,%bh
    7520:	f0 51                	lock push %cx
    7522:	5f                   	pop    %di
    7523:	28 f0                	sub    %dh,%al
    7525:	2b ac e4 f6          	sub    -0x91c(%si),%bp
    7529:	4c                   	dec    %sp
    752a:	f9                   	stc
    752b:	4b                   	dec    %bx
    752c:	76 c0                	jbe    0x74ee
    752e:	e8 05 ff             	call   0x7436
    7531:	e8 c9 08             	call   0x7dfd
    7534:	52                   	push   %dx
    7535:	54                   	push   %sp
    7536:	8b ff                	mov    %di,%di
    7538:	e8 68 00             	call   0x75a3
    753b:	83 f0 4b             	xor    $0x4b,%ax
    753e:	45                   	inc    %bp
    753f:	76 a2                	jbe    0x74e3
    7541:	05 77 95             	add    $0x9577,%ax
    7544:	2c 15                	sub    $0x15,%al
    7546:	33 9a 33 b8          	xor    -0x47cd(%bp,%si),%bx
    754a:	eb 51                	jmp    0x759d
    754c:	8d 38                	lea    (%bx,%si),%di
    754e:	83 f8 9f             	cmp    $0xff9f,%ax
    7551:	bd 24 e2             	mov    $0xe224,%bp
    7554:	40                   	inc    %ax
    7555:	31 ff                	xor    %di,%di
    7557:	03 45 37             	add    0x37(%di),%ax
    755a:	b8 02 5b             	mov    $0x5b02,%ax
    755d:	52                   	push   %dx
    755e:	57                   	push   %di
    755f:	31 e8                	xor    %bp,%ax
    7561:	1d ec 36             	sbb    $0x36ec,%ax
    7564:	c8 c0 d6 a7          	enter  $0xd6c0,$0xa7
    7568:	e4 e8                	in     $0xe8,%al
    756a:	a0 99 33             	mov    0x3399,%al
    756d:	b8 eb 03             	mov    $0x3eb,%ax
    7570:	bf ff 40             	mov    $0x40ff,%di
    7573:	05 77 95             	add    $0x9577,%ax
    7576:	2c 15                	sub    $0x15,%al
    7578:	2f                   	das
    7579:	9c                   	pushf
    757a:	0b c6                	or     %si,%ax
    757c:	f7 19                	negw   (%bx,%di)
    757e:	51                   	push   %cx
    757f:	5f                   	pop    %di
    7580:	28 8b 56 5e          	sub    %cl,0x5e56(%bp,%di)
    7584:	ff ce                	dec    %si
    7586:	8b 87 45 f2          	mov    -0xdbb(%bx),%ax
    758a:	1e                   	push   %ds
    758b:	07                   	pop    %es
    758c:	d1 e9                	shr    $1,%cx
    758e:	f3 a5                	rep movsw %ds:(%si),%es:(%di)
    7590:	13 c9                	adc    %cx,%cx
    7592:	f3 a4                	rep movsb %ds:(%si),%es:(%di)
    7594:	e8 51 57             	call   0xcce8
    7597:	46                   	inc    %si
    7598:	f0 e4 e8             	lock in $0xe8,%al
    759b:	1a 0e e8 00          	sbb    0xe8,%cl
    759f:	c1 b4 36 4f ff       	shlw   $0xff,0x4f36(%si)
    75a4:	77 fa                	ja     0x75a0
    75a6:	52                   	push   %dx
    75a7:	54                   	push   %sp
    75a8:	8b fc                	mov    %sp,%di
    75aa:	ff e4                	jmp    *%sp
    75ac:	46                   	inc    %si
    75ad:	1d 74 82             	sbb    $0x8274,%ax
    75b0:	ba e0 5b             	mov    $0x5be0,%dx
    75b3:	eb 24                	jmp    0x75d9
    75b5:	5e                   	pop    %si
    75b6:	eb 8e                	jmp    0x7546
    75b8:	45                   	inc    %bp
    75b9:	f2 8b 44 8e          	repnz mov -0x72(%si),%ax
    75bd:	45                   	inc    %bp
    75be:	0c 8d                	or     $0x8d,%al
    75c0:	bf 5f 28             	mov    $0x285f,%di
    75c3:	8b eb                	mov    %bx,%bp
    75c5:	07                   	pop    %es
    75c6:	a5                   	movsw  %ds:(%si),%es:(%di)
    75c7:	d1 f3                	shl    $1,%bx
    75c9:	13 f3                	adc    %bx,%si
    75cb:	e8 57 f0             	call   0x6625
    75ce:	e8 8b da             	call   0x505c
    75d1:	83 4b 76 8b          	orw    $0xff8b,0x76(%bp,%di)
    75d5:	07                   	pop    %es
    75d6:	01 73 81             	add    %si,-0x7f(%bp,%di)
    75d9:	e7 fa                	out    %ax,$0xfa
    75db:	44                   	inc    %sp
    75dc:	5c                   	pop    %sp
    75dd:	72 d6                	jb     0x75b5
    75df:	83 5e ca e8          	sbbw   $0xffe8,-0x36(%bp)
    75e3:	c1 36 ff fa 54       	shlw   $0x54,-0x501
    75e8:	fc                   	cld
    75e9:	e4 1d                	in     $0x1d,%al
    75eb:	82 e0 eb             	and    $0xeb,%al
    75ee:	5e                   	pop    %si
    75ef:	8e f2                	mov    %dx,%?
    75f1:	44                   	inc    %sp
    75f2:	45                   	inc    %bp
    75f3:	8d 5f 8b             	lea    -0x75(%bx),%bx
    75f6:	07                   	pop    %es
    75f7:	d1 13                	rclw   $1,(%bp,%di)
    75f9:	e8 f0 da             	call   0x50ec
    75fc:	8b 47 8e             	mov    -0x72(%bx),%ax
    75ff:	45                   	inc    %bp
    7600:	0f 5e eb             	divps  %xmm3,%xmm5
    7603:	8e 45 f8             	mov    -0x8(%di),%es
    7606:	8d b7 5f 28          	lea    0x285f(%bx),%si
    760a:	07                   	pop    %es
    760b:	d1 13                	rclw   $1,(%bp,%di)
    760d:	e8 f0 8b             	call   0x200
    7610:	83 76 07 73          	xorw   $0x73,0x7(%bp)
    7614:	e7 44                	out    %ax,$0x44
    7616:	72 83                	jb     0x759b
    7618:	ca c1 ff             	lret   $0xffc1
    761b:	54                   	push   %sp
    761c:	e4 82                	in     $0x82,%al
    761e:	eb 8e                	jmp    0x75ae
    7620:	44                   	inc    %sp
    7621:	8d 8b d1 e8          	lea    -0x172f(%bp,%di),%cx
    7625:	da 47 45             	fiaddl 0x45(%bx)
    7628:	5e                   	pop    %si
    7629:	8e f8                	mov    %ax,%?
    762b:	b7 28                	mov    $0x28,%bh
    762d:	d1 e8                	shr    $1,%ax
    762f:	8b 76 73             	mov    0x73(%bp),%si
    7632:	00 ff                	add    %bh,%bh
    7634:	7e ff                	jle    0x7635
    7636:	4a                   	dec    %dx
    7637:	06                   	push   %es
    7638:	5c                   	pop    %sp
    7639:	e8 e2 04             	call   0x7b1e
    763c:	83 3e 98 06 00       	cmpw   $0x0,0x698
    7641:	74 0d                	je     0x7650
    7643:	e8 2d 8d             	call   0x373
    7646:	29 13                	sub    %dx,(%bp,%di)
    7648:	40                   	inc    %ax
    7649:	a0 e4 b8             	mov    0xb8e4,%al
    764c:	2f                   	das
    764d:	fa                   	cli
    764e:	13 8d 40 31          	adc    0x3140(%di),%cx
    7652:	01 8b eb 81          	add    %cx,-0x7e15(%bp,%di)
    7656:	fe 0b                	decb   (%bp,%di)
    7658:	f0 77 4c             	lock ja 0x76a7
    765b:	8d 44 30             	lea    0x30(%si),%ax
    765e:	a2 12 9c             	mov    %al,0x9c12
    7661:	01 b6 fc 3e          	add    %si,0x3efc(%bp)
    7665:	b8 2f 91             	mov    $0x912f,%ax
    7668:	b9 12 9c             	mov    $0x9c12,%cx
    766b:	51                   	push   %cx
    766c:	e8 4a 2b             	call   0xa1b9
    766f:	06                   	push   %es
    7670:	6c                   	insb   (%dx),%es:(%di)
    7671:	0d 03 06             	or     $0x603,%ax
    7674:	b8 2a e8             	mov    $0xe82a,%ax
    7677:	2a 8b e3 f8          	sub    -0x71d(%bp,%di),%cl
    767b:	8d a9 06 f5          	lea    -0xafa(%bx,%di),%bp
    767f:	57                   	push   %di
    7680:	51                   	push   %cx
    7681:	28 56 1e             	sub    %dl,0x1e(%bp)
    7684:	b8 0e e8             	mov    $0xe80e,%ax
    7687:	e1 47                	loope  0x76d0
    7689:	3a 28                	cmp    (%bx,%si),%ch
    768b:	06                   	push   %es
    768c:	6c                   	insb   (%dx),%es:(%di)
    768d:	0a 8b eb 0e          	or     0xeeb(%bp,%di),%cl
    7691:	e8 e1 47             	call   0xbe75
    7694:	fa                   	cli
    7695:	28 5b cd             	sub    %bl,-0x33(%bp,%di)
    7698:	87 f6                	xchg   %si,%si
    769a:	0c b8                	or     $0xb8,%al
    769c:	51                   	push   %cx
    769d:	5c                   	pop    %sp
    769e:	8b 01                	mov    (%bx,%di),%ax
    76a0:	08 72 5c             	or     %dh,0x5c(%bp,%si)
    76a3:	2f                   	das
    76a4:	fa                   	cli
    76a5:	2d e3 83             	sub    $0x83e3,%ax
    76a8:	98                   	cbtw
    76a9:	00 0d                	add    %cl,(%di)
    76ab:	2d 29 07             	sub    $0x729,%ax
    76ae:	47                   	inc    %di
    76af:	ff 43 fb             	incw   -0x5(%bp,%di)
    76b2:	76 ae                	jbe    0x7662
    76b4:	8c ff                	mov    %?,%di
    76b6:	4a                   	dec    %dx
    76b7:	4c                   	dec    %sp
    76b8:	5c                   	pop    %sp
    76b9:	e8 62 04             	call   0x7b1e
    76bc:	c6 06 ff 56 5e       	movb   $0x5e,0x56ff
    76c1:	03 19                	add    (%bx,%di),%bx
    76c3:	e6 e8                	out    %al,$0xe8
    76c5:	00 b8 5e ff          	add    %bh,-0xa2(%bx,%si)
    76c9:	eb 4a                	jmp    0x7715
    76cb:	06                   	push   %es
    76cc:	5c                   	pop    %sp
    76cd:	e8 4e 04             	call   0x7b1e
    76d0:	3e 06                	ds push %es
    76d2:	74 e8                	je     0x76bc
    76d4:	8d 13                	lea    (%bp,%di),%dx
    76d6:	a0 b8 99             	mov    0x99b8,%al
    76d9:	8c 29                	mov    %gs,(%bx,%di)
    76db:	40                   	inc    %ax
    76dc:	e4 2f                	in     $0x2f,%al
    76de:	13 40 01             	adc    0x1(%bx,%si),%ax
    76e1:	eb 7f                	jmp    0x7762
    76e3:	8c e8                	mov    %gs,%ax
    76e5:	09 02                	or     %ax,(%bp,%si)
    76e7:	8b 81 0b 77          	mov    0x770b(%bx,%di),%ax
    76eb:	8d 30                	lea    (%bx,%si),%si
    76ed:	12 2e b8 94          	adc    -0x6b48,%ch
    76f1:	5c                   	pop    %sp
    76f2:	2f                   	das
    76f3:	8b b9 16 31          	mov    0x3116(%bx,%di),%di
    76f7:	e1 e9                	loope  0x76e2
    76f9:	8b 05                	mov    (%di),%ax
    76fb:	c0 a6 8b 85 5b       	shlb   $0x5b,-0x7a75(%bp)
    7700:	2a 5b 5b             	sub    0x5b(%bp,%di),%bl
    7703:	cd 06                	int    $0x6
    7705:	b8 09 b8             	mov    $0xb809,%ax
    7708:	95                   	xchg   %ax,%bp
    7709:	5c                   	pop    %sp
    770a:	2f                   	das
    770b:	fa                   	cli
    770c:	c6                   	(bad)
    770d:	e2 eb                	loop   0x76fa
    770f:	0d 8d 30             	or     $0x308d,%ax
    7712:	12 01                	adc    (%bx,%di),%al
    7714:	fc                   	cld
    7715:	b8 b8 fc             	mov    $0xfcb8,%ax
    7718:	b8 58 1a             	mov    $0x1a58,%ax
    771b:	c7                   	(bad)
    771c:	e4 06                	in     $0x6,%al
    771e:	30 00                	xor    %al,(%bx,%si)
    7720:	2d 07 ff             	sub    $0xff07,%ax
    7723:	fb                   	sti
    7724:	ae                   	scas   %es:(%di),%al
    7725:	ff 4c e8             	decw   -0x18(%si)
    7728:	04 38                	add    $0x38,%al
    772a:	8c 4a 5c             	mov    %cs,0x5c(%bp,%si)
    772d:	62 c6 ff ec 03       	(bad) {%k4}{z}
    7732:	0c 81                	or     $0x81,%al
    7734:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    7735:	9e                   	sahf
    7736:	54                   	push   %sp
    7737:	e4 82                	in     $0x82,%al
    7739:	eb 8e                	jmp    0x76c9
    773b:	44                   	inc    %sp
    773c:	8d 56 03             	lea    0x3(%bp),%dx
    773f:	01 fe                	add    %di,%si
    7741:	3e b8 2f 91          	ds mov $0x912f,%ax
    7745:	ff 0c                	decw   (%si)
    7747:	ae                   	scas   %es:(%di),%al
    7748:	5b                   	pop    %bx
    7749:	5b                   	pop    %bx
    774a:	27                   	daa
    774b:	06                   	push   %es
    774c:	0d 06 2a             	or     $0x2a06,%ax
    774f:	2a e3                	sub    %bl,%ah
    7751:	24 e8                	and    $0xe8,%al
    7753:	4e                   	dec    %si
    7754:	fe 8b 81 01          	decb   0x181(%bp,%di)
    7758:	b6 fc                	mov    $0xfc,%dh
    775a:	3e 56                	ds push %si
    775c:	51                   	push   %cx
    775d:	56                   	push   %si
    775e:	b8 e8 47             	mov    $0x47e8,%ax
    7761:	28 6c fe             	sub    %ch,-0x2(%si)
    7764:	e8 57 27             	call   0x9ebe
    7767:	06                   	push   %es
    7768:	0d 0a 28             	or     $0x280a,%ax
    776b:	6c                   	insb   (%dx),%es:(%di)
    776c:	fe                   	(bad)
    776d:	e8 20 28             	call   0x9f90
    7770:	5b                   	pop    %bx
    7771:	e8 74 fe             	call   0x75e8
    7774:	eb b8                	jmp    0x772e
    7776:	5a                   	pop    %dx
    7777:	b4 5c                	mov    $0x5c,%ah
    7779:	2f                   	das
    777a:	fa                   	cli
    777b:	57                   	push   %di
    777c:	e2 0c                	loop   0x778a
    777e:	81 a6 9e 87 b0 32    	andw   $0x32b0,-0x7862(%bp)
    7784:	de 0f                	fimuls (%bx)
    7786:	e8 b1 5b             	call   0xd33a
    7789:	08 54 ff             	or     %dl,-0x1(%si)
    778c:	b8 d4 5c             	mov    $0x5cd4,%ax
    778f:	2f                   	das
    7790:	fa                   	cli
    7791:	03 02                	add    (%bp,%si),%ax
    7793:	51                   	push   %cx
    7794:	bd c7 02             	mov    $0x2c7,%bp
    7797:	06                   	push   %es
    7798:	e9 bc 2f             	jmp    0xa757
    779b:	b8 a5 e8             	mov    $0xe8a5,%ax
    779e:	ee                   	out    %al,(%dx)
    779f:	fc                   	cld
    77a0:	3a fe                	cmp    %dh,%bh
    77a2:	3e 1c 4c             	ds sbb $0x4c,%al
    77a5:	fc                   	cld
    77a6:	1b 74 11             	sbb    0x11(%si),%si
    77a9:	c3                   	ret
    77aa:	8d                   	lea    (bad),%si
    77ab:	f6 e6                	mul    %dh
    77ad:	6c                   	insb   (%dx),%es:(%di)
    77ae:	f1                   	int1
    77af:	e8 32 f0             	call   0x67e4
    77b2:	08 8d d8 5b          	or     %cl,0x5bd8(%di)
    77b6:	18 b8 b8 cc          	sbb    %bh,-0x3348(%bx,%si)
    77ba:	b8 58 50             	mov    $0x5058,%ax
    77bd:	00 d9                	add    %bl,%cl
    77bf:	f7 0a 51 de          	testw  $0xde51,(%bp,%si)
    77c3:	b8 da 32             	mov    $0x32da,%ax
    77c6:	0f f0                	(bad)
    77c8:	75 42                	jne    0x780c
    77ca:	5c                   	pop    %sp
    77cb:	1e                   	push   %ds
    77cc:	a5                   	movsw  %ds:(%si),%es:(%di)
    77cd:	6e                   	outsb  %ds:(%si),(%dx)
    77ce:	89 5e 07             	mov    %bx,0x7(%bp)
    77d1:	c7 03 c3 7e          	movw   $0x7ec3,(%bp,%di)
    77d5:	40                   	inc    %ax
    77d6:	57                   	push   %di
    77d7:	7e eb                	jle    0x77c4
    77d9:	fa                   	cli
    77da:	3f                   	aas
    77db:	d1 89 7e 8b          	rorw   $1,-0x7482(%bx,%di)
    77df:	c0 a6 eb 15 e8       	shlb   $0xe8,0x15eb(%bp)
    77e4:	fc                   	cld
    77e5:	e8 3e d0             	call   0x4826
    77e8:	c7                   	(bad)
    77e9:	9d                   	popf
    77ea:	ff                   	(bad)
    77eb:	fa                   	cli
    77ec:	00 e8                	add    %ch,%al
    77ee:	ac                   	lods   %ds:(%si),%al
    77ef:	11 83 eb fa          	adc    %ax,-0x515(%bp,%di)
    77f3:	0a 47 fc             	or     -0x4(%bx),%al
    77f6:	ea 31 81 7e fc       	ljmp   $0xfc7e,$0x8131
    77fb:	79 a8                	jns    0x77a5
    77fd:	72 e4                	jb     0x77e3
    77ff:	e8 0d f0             	call   0x680f
    7802:	6f                   	outsw  %ds:(%si),(%dx)
    7803:	f8                   	clc
    7804:	b3 1c                	mov    $0x1c,%bl
    7806:	fc                   	cld
    7807:	74 c3                	je     0x77cc
    7809:	f6 6c e8             	imulb  -0x18(%si)
    780c:	f0 8d 5b 62          	lock lea 0x62(%bp,%di),%bx
    7810:	f1                   	int1
    7811:	b8 cc b8             	mov    $0xb8cc,%ax
    7814:	58                   	pop    %ax
    7815:	06                   	push   %es
    7816:	e9 bc 2f             	jmp    0xa7d5
    7819:	ad                   	lods   %ds:(%si),%ax
    781a:	dc fc                	fdivr  %st,%st(4)
    781c:	3a fe                	cmp    %dh,%bh
    781e:	e8 f8 a4             	call   0x1d19
    7821:	a1 48 63             	mov    0x6348,%ax
    7824:	04 16                	add    $0x16,%al
    7826:	45                   	inc    %bp
    7827:	b8 18 b8             	mov    $0xb818,%ax
    782a:	b8 44 e8             	mov    $0xe844,%ax
    782d:	d0 c7                	rol    $1,%bh
    782f:	9d                   	popf
    7830:	2e f0 8d 5b ae       	lock lea %cs:-0x52(%bp,%di),%bx
    7835:	13 81 40 57          	adc    0x5740(%bx,%di),%ax
    7839:	8d 12                	lea    (%bp,%si),%dx
    783b:	b8 09 0e             	mov    $0xe09,%ax
    783e:	8b fe                	mov    %si,%di
    7840:	e8 0f 00             	call   0x7852
    7843:	b8 5e ff             	mov    $0xff5e,%ax
    7846:	e8 41 a5             	call   0x1d8a
    7849:	83 ca c1             	or     $0xffc1,%dx
    784c:	ff 54 e4             	call   *-0x1c(%si)
    784f:	82 eb 8e             	sub    $0x8e,%bl
    7852:	44                   	inc    %sp
    7853:	8d 8b d1 e8          	lea    -0x172f(%bp,%di),%cx
    7857:	da 47 06             	fiaddl 0x6(%bx)
    785a:	e9 ba f0             	jmp    0x6917
    785d:	a8 b9                	test   $0xb9,%al
    785f:	30 00                	xor    %al,(%bx,%si)
    7861:	8b fa                	mov    %dx,%di
    7863:	1e                   	push   %ds
    7864:	a5                   	movsw  %ds:(%si),%es:(%di)
    7865:	f3 aa                	rep stos %al,%es:(%di)
    7867:	b8 31 e9             	mov    $0xe931,%ax
    786a:	05 66 04             	add    $0x466,%ax
    786d:	05 a6 85             	add    $0x85a6,%ax
    7870:	2a b8 f0 a8          	sub    -0x5710(%bx,%si),%bh
    7874:	8d 5b ba             	lea    -0x46(%bp,%di),%bx
    7877:	25 5b 96             	and    $0x965b,%ax
    787a:	03 8a 5b 6a          	add    0x6a5b(%bp,%si),%cx
    787e:	a1 b8 56             	mov    0x56b8,%ax
    7881:	30 b8 44 d0          	xor    %bh,-0x2fbc(%bx,%si)
    7885:	9d                   	popf
    7886:	f0 5b                	lock pop %bx
    7888:	13 40 8d             	adc    -0x73(%bx,%si),%ax
    788b:	b8 0e 55             	mov    $0x550e,%ax
    788e:	13 a1 be fd          	adc    -0x242(%bx,%di),%sp
    7892:	fd                   	std
    7893:	00 57 89             	add    %dl,-0x77(%bx)
    7896:	7e 8b                	jle    0x7823
    7898:	41                   	inc    %cx
    7899:	b8 c1 0a             	mov    $0xac1,%ax
    789c:	e9 05 66             	jmp    0xdea4
    789f:	04 05                	add    $0x5,%al
    78a1:	3f                   	aas
    78a2:	d1 b8 5e ff          	sarw   $1,-0xa2(%bx,%si)
    78a6:	96                   	xchg   %ax,%si
    78a7:	8a 6a bb             	mov    -0x45(%bp,%si),%ch
    78aa:	5a                   	pop    %dx
    78ab:	9f                   	lahf
    78ac:	0a f0                	or     %al,%dh
    78ae:	a8 8d                	test   $0x8d,%al
    78b0:	01 fc                	add    %di,%sp
    78b2:	56                   	push   %si
    78b3:	56                   	push   %si
    78b4:	56                   	push   %si
    78b5:	30 f0                	xor    %dh,%al
    78b7:	5b                   	pop    %bx
    78b8:	13 d5                	adc    %bp,%dx
    78ba:	8d b8 0e 2e          	lea    0x2e0e(%bx,%si),%di
    78be:	be 33 eb             	mov    $0xeb33,%si
    78c1:	bf 23 b8             	mov    $0xb823,%di
    78c4:	da 5c 2a             	ficompl 0x2a(%si)
    78c7:	f0 8d ba 5b 03       	lock lea 0x35b(%bp,%si),%di
    78cc:	b8 28 f0             	mov    $0xf028,%ax
    78cf:	5f                   	pop    %di
    78d0:	2a e3                	sub    %bl,%ah
    78d2:	e8 0d b8             	call   0x30e2
    78d5:	db 5c f0             	fistpl -0x10(%si)
    78d8:	ba 03 28             	mov    $0x2803,%dx
    78db:	5f                   	pop    %di
    78dc:	e3 53                	jcxz   0x7931
    78de:	25 5b 96             	and    $0x965b,%ax
    78e1:	06                   	push   %es
    78e2:	33 00                	xor    (%bx,%si),%ax
    78e4:	8b 08                	mov    (%bx,%si),%cx
    78e6:	5c                   	pop    %sp
    78e7:	8d 12                	lea    (%bp,%si),%dx
    78e9:	fc                   	cld
    78ea:	36 d3 c9             	ss ror %cl,%cx
    78ed:	06                   	push   %es
    78ee:	01 06 31 e8          	add    %ax,-0x17cf
    78f2:	a9 06 e0             	test   $0xe006,%ax
    78f5:	33 54 b8             	xor    -0x48(%si),%dx
    78f8:	5c                   	pop    %sp
    78f9:	e4 5c                	in     $0x5c,%al
    78fb:	fa                   	cli
    78fc:	02 97 6e c5          	add    -0x3a92(%bx),%dl
    7900:	e8 51 01             	call   0x7a54
    7903:	3f                   	aas
    7904:	01 22                	add    %sp,(%bp,%si)
    7906:	e8 4c a4             	call   0x1d55
    7909:	e8 f8 ee             	call   0x6804
    790c:	f9                   	stc
    790d:	d9 b3 fc c3          	fnstenv -0x3c04(%bp,%di)
    7911:	6c                   	insb   (%dx),%es:(%di)
    7912:	f0 5b                	lock pop %bx
    7914:	f1                   	int1
    7915:	cc                   	int3
    7916:	58                   	pop    %ax
    7917:	e9 2f 00             	jmp    0x7949
    791a:	f0 e8 c6 ee          	lock call 0x67e4
    791e:	08 d8                	or     %bl,%al
    7920:	18 b8 4a b8          	sbb    %bh,-0x47b6(%bx,%si)
    7924:	cc                   	int3
    7925:	58                   	pop    %ax
    7926:	00 f7                	add    %dh,%bh
    7928:	51                   	push   %cx
    7929:	b8 32 f0             	mov    $0xf032,%ax
    792c:	42                   	inc    %dx
    792d:	e5 dc                	in     $0xdc,%ax
    792f:	b8 32 f0             	mov    $0xf032,%ax
    7932:	42                   	inc    %dx
    7933:	1e                   	push   %ds
    7934:	6e                   	outsb  %ds:(%si),(%dx)
    7935:	5e                   	pop    %si
    7936:	c7 c3 40 7e          	mov    $0x7e40,%bx
    793a:	fa                   	cli
    793b:	d1 7e c0             	sarw   $1,-0x40(%bp)
    793e:	eb e8                	jmp    0x7928
    7940:	e8 d0 9d             	call   0x1713
    7943:	fa                   	cli
    7944:	e8 fc 3f             	call   0xb943
    7947:	89 8b a6 fe          	mov    %cx,-0x15a(%bp,%di)
    794b:	c0 eb e8             	shr    $0xe8,%bl
    794e:	e8 d0 9d             	call   0x1721
    7951:	fe                   	(bad)
    7952:	3e c7                	ds (bad)
    7954:	ff 00                	incw   (%bx,%si)
    7956:	ac                   	lods   %ds:(%si),%al
    7957:	83 fc e8             	cmp    $0xffe8,%sp
    795a:	40                   	inc    %ax
    795b:	10 47 ea             	adc    %al,-0x16(%bx)
    795e:	81 0a fc 31          	orw    $0x31fc,(%bp,%si)
    7962:	fe                   	(bad)
    7963:	31 7e 79             	xor    %di,0x79(%bp)
    7966:	fe                   	(bad)
    7967:	79 72                	jns    0x79db
    7969:	e8 f0 f8             	call   0x725c
    796c:	a1 ee 06             	mov    0x6ee,%ax
    796f:	b8 b8 39             	mov    $0x39b8,%ax
    7972:	01 13                	add    %dx,(%bp,%di)
    7974:	5e                   	pop    %si
    7975:	04 1c                	add    $0x1c,%al
    7977:	eb ff                	jmp    0x7978
    7979:	9f                   	lahf
    797a:	c0 24 ef             	shlb   $0xef,(%si)
    797d:	b8 b8 06             	mov    $0x6b8,%ax
    7980:	bc ad fc             	mov    $0xfcad,%sp
    7983:	fe                   	(bad)
    7984:	f8                   	clc
    7985:	41                   	inc    %cx
    7986:	db ee                	fucomi %st(6),%st
    7988:	f9                   	stc
    7989:	d9 e8                	fld1
    798b:	fd                   	std
    798c:	a3 06 33             	mov    %ax,0x3306
    798f:	fa                   	cli
    7990:	ff e4                	jmp    *%sp
    7992:	eb 44                	jmp    0x79d8
    7994:	8b e8                	mov    %ax,%bp
    7996:	47                   	inc    %di
    7997:	e9 f0 b9             	jmp    0x338a
    799a:	00 42 54             	add    %al,0x54(%bp,%si)
    799d:	b8 01 fc             	mov    $0xfc01,%ax
    79a0:	be bc ad             	mov    $0xadbc,%si
    79a3:	ff 57 ff             	call   *-0x1(%bx)
    79a6:	ae                   	scas   %es:(%di),%al
    79a7:	5b                   	pop    %bx
    79a8:	06                   	push   %es
    79a9:	fe                   	(bad)
    79aa:	25 06 06             	and    $0x606,%ax
    79ad:	2a 24                	sub    (%si),%ah
    79af:	4e                   	dec    %si
    79b0:	02 e9                	add    %cl,%ch
    79b2:	4c                   	dec    %sp
    79b3:	95                   	xchg   %ax,%bp
    79b4:	00 a9 57 dc          	add    %ch,-0x23a9(%bx,%di)
    79b8:	04 1c                	add    $0x1c,%al
    79ba:	7a 28                	jp     0x79e4
    79bc:	5f                   	pop    %di
    79bd:	e3 72                	jcxz   0x7a31
    79bf:	24 5b                	and    $0x5b,%al
    79c1:	96                   	xchg   %ax,%si
    79c2:	be 7a 28             	mov    $0x287a,%si
    79c5:	01 fc                	add    %di,%sp
    79c7:	be bc e8             	mov    $0xe8bc,%si
    79ca:	d2 25                	shlb   %cl,(%di)
    79cc:	5b                   	pop    %bx
    79cd:	2a e3                	sub    %bl,%ah
    79cf:	e8 19 47             	call   0xc0eb
    79d2:	83 c6 0d             	add    $0xd,%si
    79d5:	81 fe ef a8          	cmp    $0xa8ef,%si
    79d9:	73 0c                	jae    0x79e7
    79db:	a9 57 dc             	test   $0xdc57,%ax
    79de:	04 56                	add    $0x56,%al
    79e0:	e8 4f 24             	call   0x9e32
    79e3:	5b                   	pop    %bx
    79e4:	96                   	xchg   %ax,%si
    79e5:	eb de                	jmp    0x79c5
    79e7:	bf 08 00             	mov    $0x8,%di
    79ea:	2b d2                	sub    %dx,%dx
    79ec:	b9 7a 28             	mov    $0x287a,%cx
    79ef:	3b d7                	cmp    %di,%dx
    79f1:	77 57                	ja     0x7a4a
    79f3:	7e eb                	jle    0x79e0
    79f5:	fa                   	cli
    79f6:	89 56 ff             	mov    %dx,-0x1(%bp)
    79f9:	8b f1                	mov    %cx,%si
    79fb:	89 56 f8             	mov    %dx,-0x8(%bp)
    79fe:	89 4e fc             	mov    %cx,-0x4(%bp)
    7a01:	8b fe                	mov    %si,%di
    7a03:	eb 2c                	jmp    0x7a31
    7a05:	56                   	push   %si
    7a06:	57                   	push   %di
    7a07:	e8 7c 27             	call   0xa186
    7a0a:	f0 2a e8             	lock sub %al,%ch
    7a0d:	b8 7e 1b             	mov    $0x1b7e,%ax
    7a10:	57                   	push   %di
    7a11:	a9 57 ea             	test   $0xea57,%ax
    7a14:	58                   	pop    %ax
    7a15:	c7 06 24 5b 96 56    	movw   $0x5696,0x5b24
    7a1b:	57                   	push   %di
    7a1c:	e8 13 24             	call   0x9e32
    7a1f:	5b                   	pop    %bx
    7a20:	96                   	xchg   %ax,%si
    7a21:	a9 57 ea             	test   $0xea57,%ax
    7a24:	58                   	pop    %ax
    7a25:	56                   	push   %si
    7a26:	e8 09 24             	call   0x9e32
    7a29:	5b                   	pop    %bx
    7a2a:	96                   	xchg   %ax,%si
    7a2b:	83 c6 0d             	add    $0xd,%si
    7a2e:	ff 83 ff 06          	incw   0x6ff(%bp,%di)
    7a32:	33 fa                	xor    %dx,%di
    7a34:	39 83 ff 76          	cmp    %ax,0x76ff(%bp,%di)
    7a38:	cc                   	int3
    7a39:	83 c7 0d             	add    $0xd,%di
    7a3c:	8b 56 f8             	mov    -0x8(%bp),%dx
    7a3f:	42                   	inc    %dx
    7a40:	7e eb                	jle    0x7a2d
    7a42:	fc                   	cld
    7a43:	8b f8                	mov    %ax,%di
    7a45:	8b 4e fc             	mov    -0x4(%bp),%cx
    7a48:	eb a5                	jmp    0x79ef
    7a4a:	8b c7                	mov    %di,%ax
    7a4c:	46                   	inc    %si
    7a4d:	ff 81 8c a6          	incw   -0x5974(%bx,%di)
    7a51:	fa                   	cli
    7a52:	9e                   	sahf
    7a53:	8a 87 44 b0          	mov    -0x4fbc(%bx),%al
    7a57:	eb 32                	jmp    0x7a8b
    7a59:	0a b9 fd e8          	or     -0x1703(%bx,%di),%bh
    7a5d:	f6 b8 ea 5c          	idivb  0x5cea(%bx,%si)
    7a61:	ba 28 31             	mov    $0x3128,%dx
    7a64:	05 04 6e             	add    $0x6e04,%ax
    7a67:	fe 05                	incb   (%di)
    7a69:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    7a6a:	85 2a                	test   %bp,(%bp,%si)
    7a6c:	5b                   	pop    %bx
    7a6d:	c3                   	ret
    7a6e:	23 5b 46             	and    0x46(%bp,%di),%bx
    7a71:	8c 83 fe 09          	mov    %es,0x9fe(%bp,%di)
    7a75:	7c e7                	jl     0x7a5e
    7a77:	3d ff 24             	cmp    $0x24ff,%ax
    7a7a:	9b                   	fwait
    7a7b:	90                   	nop
    7a7c:	bf 7a 28             	mov    $0x287a,%di
    7a7f:	e9 89 00             	jmp    0x7b0b
    7a82:	a9 06 f5             	test   $0xf506,%ax
    7a85:	46                   	inc    %si
    7a86:	ad                   	lods   %ds:(%si),%ax
    7a87:	fe 41 57             	incb   0x57(%bx,%di)
    7a8a:	e8 19 24             	call   0x9ea6
    7a8d:	06                   	push   %es
    7a8e:	06                   	push   %es
    7a8f:	2a 24                	sub    (%si),%ah
    7a91:	4e                   	dec    %si
    7a92:	8b 71 05             	mov    0x5(%bx,%di),%si
    7a95:	95                   	xchg   %ax,%bp
    7a96:	15 9c b8             	adc    $0xb89c,%ax
    7a99:	bf 8b 96             	mov    $0x968b,%di
    7a9c:	b8 22 a9             	mov    $0xa922,%ax
    7a9f:	e8 28 fe             	call   0x78ca
    7aa2:	57                   	push   %di
    7aa3:	fa                   	cli
    7aa4:	e8 17 24             	call   0x9ebe
    7aa7:	06                   	push   %es
    7aa8:	06                   	push   %es
    7aa9:	0a 24                	or     (%si),%ah
    7aab:	4e                   	dec    %si
    7aac:	8b 96 b8 22          	mov    0x22b8(%bp),%dx
    7ab0:	a9 96 b8             	test   $0xb896,%ax
    7ab3:	0e                   	push   %cs
    7ab4:	b8 1a e4             	mov    $0xe41a,%ax
    7ab7:	cd 26                	int    $0x26
    7ab9:	f0 2a e8             	lock sub %al,%ch
    7abc:	b8 5c 3f             	mov    $0x3f5c,%ax
    7abf:	8a 05                	mov    (%di),%al
    7ac1:	98                   	cbtw
    7ac2:	9a 76 75 ee 30       	lcall  $0x30ee,$0x7576
    7ac7:	78 35                	js     0x7afe
    7ac9:	83 fe 09             	cmp    $0x9,%si
    7acc:	7d 30                	jge    0x7afe
    7ace:	05 95 15             	add    $0x1595,%ax
    7ad1:	9c                   	pushf
    7ad2:	b8 30 5b             	mov    $0x5b30,%ax
    7ad5:	d5 8b                	aad    $0x8b
    7ad7:	16                   	push   %ss
    7ad8:	e1 8b                	loope  0x7a65
    7ada:	c0 8b 5b 5b cd       	rorb   $0xcd,0x5b5b(%bp,%di)
    7adf:	b8 f8 f6             	mov    $0xf6f8,%ax
    7ae2:	74 e8                	je     0x7acc
    7ae4:	fe                   	(bad)
    7ae5:	fa                   	cli
    7ae6:	17                   	pop    %ss
    7ae7:	06                   	push   %es
    7ae8:	0a d3                	or     %bl,%dl
    7aea:	23 06 06 0a          	and    0xa06,%ax
    7aee:	24 4e                	and    $0x4e,%al
    7af0:	f6 b8 5c eb          	idivb  -0x14a4(%bx,%si)
    7af4:	5c                   	pop    %sp
    7af5:	ba 2b 90             	mov    $0x902b,%dx
    7af8:	62                   	(bad)
    7af9:	f8                   	clc
    7afa:	36 23 5b 46          	and    %ss:0x46(%bp,%di),%bx
    7afe:	fa                   	cli
    7aff:	17                   	pop    %ss
    7b00:	06                   	push   %es
    7b01:	0a 8c 24 5b          	or     0x5b24(%si),%cl
    7b05:	83 c7 0d             	add    $0xd,%di
    7b08:	ff e7                	jmp    *%di
    7b0a:	c1 06 00 08 39       	rolw   $0x39,0x800
    7b0f:	83 fc 7f             	cmp    $0x7f,%sp
    7b12:	03 e9                	add    %cx,%bp
    7b14:	6c                   	insb   (%dx),%es:(%di)
    7b15:	83 2e 76 24 0c       	subw   $0xc,0x2476
    7b1a:	fe                   	(bad)
    7b1b:	7e 9e                	jle    0x7abb
    7b1d:	87 b0 32 b9          	xchg   %si,-0x46ce(%bx,%si)
    7b21:	e8 30 2d             	call   0xa854
    7b24:	ff ae 4c 28          	ljmp   *0x284c(%bp)
    7b28:	4a                   	dec    %dx
    7b29:	4a                   	dec    %dx
    7b2a:	01 8d 5f f0          	add    %cx,-0xfa1(%di)
    7b2e:	ac                   	lods   %ds:(%si),%al
    7b2f:	f6 00 23             	testb  $0x23,(%bx,%si)
    7b32:	5b                   	pop    %bx
    7b33:	46                   	inc    %si
    7b34:	2f                   	das
    7b35:	40                   	inc    %ax
    7b36:	eb 04                	jmp    0x7b3c
    7b38:	a2 58 4e             	mov    %al,0x4e58
    7b3b:	5c                   	pop    %sp
    7b3c:	b8 4e 5c             	mov    $0x5c4e,%ax
    7b3f:	9f                   	lahf
    7b40:	5f                   	pop    %di
    7b41:	f0 d3 c7             	lock rol %cl,%di
    7b44:	51                   	push   %cx
    7b45:	ab                   	stos   %ax,%es:(%di)
    7b46:	22 c7                	and    %bh,%al
    7b48:	51                   	push   %cx
    7b49:	28 2b                	sub    %ch,(%bp,%di)
    7b4b:	e4 4c                	in     $0x4c,%al
    7b4d:	4b                   	dec    %bx
    7b4e:	84 de                	test   %bl,%dh
    7b50:	81 9e b0 de e8 5b    	sbbw   $0x5be8,-0x2150(%bp)
    7b56:	54                   	push   %sp
    7b57:	b8 5c fa             	mov    $0xfa5c,%ax
    7b5a:	02 bd 02 e9          	add    -0x16fe(%di),%bh
    7b5e:	08 18                	or     %bl,(%bx,%si)
    7b60:	4a                   	dec    %dx
    7b61:	c1 56 00 b8          	rclw   $0xb8,0x0(%bp)
    7b65:	a1 5e 57             	mov    0x575e,%ax
    7b68:	03 07                	add    (%bx),%ax
    7b6a:	f2 19 a0 95 15       	repnz sbb %sp,0x1595(%bx,%si)
    7b6f:	9c                   	pushf
    7b70:	52                   	push   %dx
    7b71:	96                   	xchg   %ax,%si
    7b72:	03 58 06             	add    0x6(%bx,%si),%bx
    7b75:	00 a9 31 01          	add    %ch,0x131(%bx,%di)
    7b79:	8a e2                	mov    %dl,%ah
    7b7b:	05 67 0d             	add    $0xd67,%ax
    7b7e:	6a 06                	push   $0x6
    7b80:	67 0d 6a 67          	addr32 or $0x676a,%ax
    7b84:	56                   	push   %si
    7b85:	f8                   	clc
    7b86:	52                   	push   %dx
    7b87:	8b f1                	mov    %cx,%si
    7b89:	e8 2c 89             	call   0x4b8
    7b8c:	8a e2                	mov    %dl,%ah
    7b8e:	05 67 87             	add    $0x8767,%ax
    7b91:	03 1f                	add    (%bx),%bx
    7b93:	2a d2                	sub    %dl,%dl
    7b95:	52                   	push   %dx
    7b96:	e8 4b df             	call   0x5ae4
    7b99:	9f                   	lahf
    7b9a:	24 40                	and    $0x40,%al
    7b9c:	ff 45 22             	incw   0x22(%di)
    7b9f:	df 9f ab 5e          	fistps 0x5eab(%bx)
    7ba3:	57                   	push   %di
    7ba4:	28 2b                	sub    %ch,(%bp,%di)
    7ba6:	e4 4c                	in     $0x4c,%al
    7ba8:	4b                   	dec    %bx
    7ba9:	87 22                	xchg   %sp,(%bp,%si)
    7bab:	c7                   	(bad)
    7bac:	51                   	push   %cx
    7bad:	8b d1                	mov    %cx,%dx
    7baf:	5a                   	pop    %dx
    7bb0:	34 60                	xor    $0x60,%al
    7bb2:	ac                   	lods   %ds:(%si),%al
    7bb3:	22 52 04             	and    0x4(%bp,%si),%dl
    7bb6:	ac                   	lods   %ds:(%si),%al
    7bb7:	fc                   	cld
    7bb8:	e8 2e c7             	call   0x42e9
    7bbb:	58                   	pop    %ax
    7bbc:	34 22                	xor    $0x22,%al
    7bbe:	c7                   	(bad)
    7bbf:	51                   	push   %cx
    7bc0:	28 ba 5e 28          	sub    %bh,0x285e(%bp,%si)
    7bc4:	e4 4b                	in     $0x4b,%al
    7bc6:	22 51 d1             	and    -0x2f(%bx,%di),%dl
    7bc9:	27                   	daa
    7bca:	22 c7                	and    %bh,%al
    7bcc:	51                   	push   %cx
    7bcd:	b1 05                	mov    $0x5,%cl
    7bcf:	d3 e7                	shl    %cl,%di
    7bd1:	8a 85 84 52          	mov    0x5284(%di),%al
    7bd5:	b1 28                	mov    $0x28,%cl
    7bd7:	c0 f6 8a             	shl    $0x8a,%dh
    7bda:	53                   	push   %bx
    7bdb:	e7 de                	out    %ax,$0xde
    7bdd:	86 2e 2e 4e          	xchg   %ch,0x4e2e
    7be1:	2d 57 52             	sub    $0x5257,%ax
    7be4:	5f                   	pop    %di
    7be5:	e4 4b                	in     $0x4b,%al
    7be7:	22 51 d1             	and    -0x2f(%bx,%di),%dl
    7bea:	06                   	push   %es
    7beb:	22 c7                	and    %bh,%al
    7bed:	51                   	push   %cx
    7bee:	28 bf 5e e4          	sub    %bh,-0x1ba2(%bx)
    7bf2:	22 d1                	and    %cl,%dl
    7bf4:	22 51 05             	and    0x5(%bx,%di),%dl
    7bf7:	f9                   	stc
    7bf8:	21 c7                	and    %ax,%di
    7bfa:	51                   	push   %cx
    7bfb:	8b 85 72 52          	mov    0x5272(%di),%ax
    7bff:	8b c8                	mov    %ax,%cx
    7c01:	89 73 89             	mov    %si,-0x77(%bp,%di)
    7c04:	c1 03 85             	rolw   $0x85,(%bp,%di)
    7c07:	70 52                	jo     0x7c5b
    7c09:	04 c4                	add    $0xc4,%al
    7c0b:	09 2b                	or     %bp,(%bp,%di)
    7c0d:	d2 f7                	shl    %cl,%bh
    7c0f:	f1                   	int1
    7c10:	8b d8                	mov    %ax,%bx
    7c12:	2e 4e                	cs dec %si
    7c14:	2d 57 5a             	sub    $0x5a57,%ax
    7c17:	5f                   	pop    %di
    7c18:	4b                   	dec    %bx
    7c19:	51                   	push   %cx
    7c1a:	06                   	push   %es
    7c1b:	c7                   	(bad)
    7c1c:	28 d3                	sub    %dl,%bl
    7c1e:	21 c7                	and    %ax,%di
    7c20:	51                   	push   %cx
    7c21:	b8 ce 5e             	mov    $0x5ece,%ax
    7c24:	22 22                	and    (%bp,%si),%ah
    7c26:	05 21 51             	add    $0x5121,%ax
    7c29:	85 c6                	test   %ax,%si
    7c2b:	21 c7                	and    %ax,%di
    7c2d:	51                   	push   %cx
    7c2e:	8a 7a b1             	mov    -0x4f(%bp,%si),%bh
    7c31:	c0 f6 8a             	shl    $0x8a,%dh
    7c34:	53                   	push   %bx
    7c35:	e7 de                	out    %ax,$0xde
    7c37:	86 2e 2e 4e          	xchg   %ch,0x4e2e
    7c3b:	2d 57 57             	sub    $0x5757,%ax
    7c3e:	7e ab                	jle    0x7beb
    7c40:	62 5f 51             	bound  %bx,0x51(%bx)
    7c43:	c7                   	(bad)
    7c44:	d3 c7                	rol    %cl,%di
    7c46:	b8 a9 21             	mov    $0x21a9,%ax
    7c49:	c7                   	(bad)
    7c4a:	51                   	push   %cx
    7c4b:	b8 e2 67             	mov    $0x67e2,%ax
    7c4e:	03 2a                	add    (%bp,%si),%bp
    7c50:	52                   	push   %dx
    7c51:	4b                   	dec    %bx
    7c52:	9f                   	lahf
    7c53:	40                   	inc    %ax
    7c54:	45                   	inc    %bp
    7c55:	df 8c de 8b          	fisttps -0x7422(%si)
    7c59:	00 a1 51 32          	add    %ah,0x3251(%bx,%di)
    7c5d:	42                   	inc    %dx
    7c5e:	dc 32                	fdivl  (%bp,%si)
    7c60:	60                   	pusha
    7c61:	de 60 ac             	fisubs -0x54(%bx,%si)
    7c64:	1c 5f                	sbb    $0x5f,%al
    7c66:	c7 c7 a9 c7          	mov    $0xc7a9,%di
    7c6a:	b8 c5 21             	mov    $0x21c5,%ax
    7c6d:	c7                   	(bad)
    7c6e:	51                   	push   %cx
    7c6f:	b8 f5 5e             	mov    $0x5ef5,%ax
    7c72:	22 21                	and    (%bx,%di),%ah
    7c74:	85 21                	test   %sp,(%bx,%di)
    7c76:	51                   	push   %cx
    7c77:	7a 78                	jp     0x7cf1
    7c79:	21 c7                	and    %ax,%di
    7c7b:	51                   	push   %cx
    7c7c:	60                   	pusha
    7c7d:	ac                   	lods   %ds:(%si),%al
    7c7e:	22 52 21             	and    0x21(%bp,%si),%dl
    7c81:	85 21                	test   %sp,(%bx,%di)
    7c83:	51                   	push   %cx
    7c84:	7a 6b                	jp     0x7cf1
    7c86:	21 c7                	and    %ax,%di
    7c88:	51                   	push   %cx
    7c89:	b8 06 5f             	mov    $0x5f06,%ax
    7c8c:	22 85 51 78          	and    0x7851(%di),%al
    7c90:	c7                   	(bad)
    7c91:	60                   	pusha
    7c92:	5e                   	pop    %si
    7c93:	21 51 e2             	and    %dx,-0x1e(%bx,%di)
    7c96:	03 52 9f             	add    -0x61(%bp,%si),%dx
    7c99:	45                   	inc    %bp
    7c9a:	8c 16 f7 b9          	mov    %ss,-0x4609
    7c9e:	3a e8                	cmp    %al,%ch
    7ca0:	19 41 de             	sbb    %ax,-0x22(%bx,%di)
    7ca3:	19 95 9c 96          	sbb    %dx,-0x6964(%di)
    7ca7:	58                   	pop    %ax
    7ca8:	e5 9c                	in     $0x9c,%ax
    7caa:	fe                   	(bad)
    7cab:	f6 4c f9 4b          	testb  $0x4b,-0x7(%si)
    7caf:	76 c0                	jbe    0x7c71
    7cb1:	e8 19 6d             	call   0xe9cd
    7cb4:	04 be                	add    $0xbe,%al
    7cb6:	9f                   	lahf
    7cb7:	fe                   	(bad)
    7cb8:	b5 b8                	mov    $0xb8,%ch
    7cba:	a1 19 27             	mov    0x2719,%ax
    7cbd:	b9 01 69             	mov    $0x6901,%cx
    7cc0:	b9 b7 d0             	mov    $0xd0b7,%cx
    7cc3:	e8 10 1d             	call   0x99d6
    7cc6:	0f e8 2f             	psubsb (%bx),%mm5
    7cc9:	b9 a1 63             	mov    $0x63a1,%cx
    7ccc:	16                   	push   %ss
    7ccd:	b8 b8 44             	mov    $0x44b8,%ax
    7cd0:	be 9f fe             	mov    $0xfe9f,%si
    7cd3:	b5 b8                	mov    $0xb8,%ch
    7cd5:	a1 19 27             	mov    0x2719,%ax
    7cd8:	b9 b7 d0             	mov    $0xd0b7,%cx
    7cdb:	96                   	xchg   %ax,%si
    7cdc:	58                   	pop    %ax
    7cdd:	05 0f 03             	add    $0x30f,%ax
    7ce0:	e6 00                	out    %al,$0x0
    7ce2:	5e                   	pop    %si
    7ce3:	eb 06                	jmp    0x7ceb
    7ce5:	e8 04 b1             	call   0x2dec
    7ce8:	88 7e c3             	mov    %bh,-0x3d(%bp)
    7ceb:	07                   	pop    %es
    7cec:	22 fd                	and    %ch,%bh
    7cee:	d3 e3                	shl    %cl,%bx
    7cf0:	81 c3 82 52          	add    $0x5282,%bx
    7cf4:	89 eb                	mov    %bp,%bx
    7cf6:	45                   	inc    %bp
    7cf7:	8d 37                	lea    (%bx),%si
    7cf9:	81 e6 ff 00          	and    $0xff,%si
    7cfd:	e8 db b8             	call   0x35db
    7d00:	ad                   	lods   %ds:(%si),%ax
    7d01:	fe 41 ee             	incb   -0x12(%bx,%di)
    7d04:	4f                   	dec    %di
    7d05:	a0 b8 54             	mov    0x54b8,%al
    7d08:	60                   	pusha
    7d09:	f8                   	clc
    7d0a:	bd e2 31             	mov    $0x31e2,%bp
    7d0d:	03 37                	add    (%bx),%si
    7d0f:	03 d9                	add    %cx,%bx
    7d11:	83 3e ba 43 e8       	cmpw   $0xffe8,0x43ba
    7d16:	e8 0c b8             	call   0x3525
    7d19:	54                   	push   %sp
    7d1a:	60                   	pusha
    7d1b:	f8                   	clc
    7d1c:	56                   	push   %si
    7d1d:	f9                   	stc
    7d1e:	43                   	inc    %bx
    7d1f:	74 e8                	je     0x7d09
    7d21:	6f                   	outsw  %ds:(%si),(%dx)
    7d22:	8e db                	mov    %bx,%ds
    7d24:	b8 54 60             	mov    $0x6054,%ax
    7d27:	f8                   	clc
    7d28:	56                   	push   %si
    7d29:	e8 9c d7             	call   0x54c8
    7d2c:	9a 75 30 fe 05       	lcall  $0x5fe,$0x3075
    7d31:	73 06                	jae    0x7d39
    7d33:	ec                   	in     (%dx),%al
    7d34:	52                   	push   %dx
    7d35:	fe 88 07 ee          	decb   -0x11f9(%bx,%si)
    7d39:	4f                   	dec    %di
    7d3a:	a0 0c a6             	mov    0xa60c,%al
    7d3d:	87 32                	xchg   %si,(%bp,%si)
    7d3f:	0f b1 08             	cmpxchg %cx,(%bx,%si)
    7d42:	57                   	push   %di
    7d43:	56                   	push   %si
    7d44:	8b 3e c0 5f          	mov    0x5fc0,%di
    7d48:	81 e7 ff 00          	and    $0xff,%di
    7d4c:	e8 8c b8             	call   0x35db
    7d4f:	ad                   	lods   %ds:(%si),%ax
    7d50:	fe 41 ee             	incb   -0x12(%bx,%di)
    7d53:	00 a0 b8 ae          	add    %ah,-0x5148(%bx,%si)
    7d57:	60                   	pusha
    7d58:	bd 31 37             	mov    $0x3731,%bp
    7d5b:	d9 3e 43 b4          	fnstcw -0x4bbd
    7d5f:	d8 c0                	fadd   %st(0),%st
    7d61:	e8 d0 fa             	call   0x7834
    7d64:	fc                   	cld
    7d65:	89 83 ba e8          	mov    %ax,-0x1746(%bp,%di)
    7d69:	0c 54                	or     $0x54,%al
    7d6b:	f8                   	clc
    7d6c:	f9                   	stc
    7d6d:	74 ae                	je     0x7d1d
    7d6f:	60                   	pusha
    7d70:	bd 57 f9             	mov    $0xf957,%bp
    7d73:	43                   	inc    %bx
    7d74:	74 e8                	je     0x7d5e
    7d76:	6f                   	outsw  %ds:(%si),(%dx)
    7d77:	e2 da                	loop   0x7d53
    7d79:	74 ae                	je     0x7d29
    7d7b:	60                   	pusha
    7d7c:	bd 57 e8             	mov    $0xe857,%bp
    7d7f:	47                   	inc    %di
    7d80:	d7                   	xlat   %ds:(%bx)
    7d81:	9a f8 83 ff 05       	lcall  $0x5ff,$0x83f8
    7d86:	73 1c                	jae    0x7da4
    7d88:	a2 c0 5f             	mov    %al,0x5fc0
    7d8b:	be 80 0b             	mov    $0xb80,%si
    7d8e:	81 b8 0b e4 73 10    	cmpw   $0x1073,-0x1bf5(%bx,%si)
    7d94:	80 3c 06             	cmpb   $0x6,(%si)
    7d97:	74 06                	je     0x7d9f
    7d99:	83 36 02 24 44       	xorw   $0x44,0x2402
    7d9e:	fd                   	std
    7d9f:	72 20                	jb     0x7dc1
    7da1:	fe                   	(bad)
    7da2:	eb ea                	jmp    0x7d8e
    7da4:	2b 9f 05 52          	sub    0x5205(%bx),%bx
    7da8:	50                   	push   %ax
    7da9:	19 bf 73 b4          	sbb    %di,-0x4b8d(%bx)
    7dad:	3e 4f                	ds dec %di
    7daf:	1a fe                	sbb    %dh,%bh
    7db1:	f7 f8                	idiv   %ax
    7db3:	32 0f                	xor    (%bx),%cl
    7db5:	f0 75 39             	lock jne 0x7df1
    7db8:	c6                   	(bad)
    7db9:	ff cc                	dec    %sp
    7dbb:	eb 83                	jmp    0x7d40
    7dbd:	b8 36 05             	mov    $0x536,%ax
    7dc0:	e6 8b                	out    %al,$0x8b
    7dc2:	1e                   	push   %ds
    7dc3:	b2 9a                	mov    $0x9a,%dl
    7dc5:	d1 eb                	shr    $1,%bx
    7dc7:	8a 87 ac 9a          	mov    -0x6554(%bx),%al
    7dcb:	2a e4                	sub    %ah,%ah
    7dcd:	a3 26 9b             	mov    %ax,0x9b26
    7dd0:	03 06 a9 29          	add    0x29a9,%ax
    7dd4:	8b 1e b4 9a          	mov    -0x654c,%bx
    7dd8:	d1 57 ab             	rclw   $1,-0x55(%bx)
    7ddb:	5f                   	pop    %di
    7ddc:	9e                   	sahf
    7ddd:	9a ab 5f 9e 9a       	lcall  $0x9a9e,$0x5fab
    7de2:	e8 47 22             	call   0xa02c
    7de5:	b8 e4 26             	mov    $0x26e4,%ax
    7de8:	78 1c                	js     0x7e06
    7dea:	b8 52 05             	mov    $0x552,%ax
    7ded:	19 b8 e0 45          	sbb    %di,0x45e0(%bx,%si)
    7df1:	3f                   	aas
    7df2:	e8 6d 1c             	call   0x9a62
    7df5:	b8 37 d9             	mov    $0xd937,%ax
    7df8:	3e 43                	ds inc %bx
    7dfa:	38 1e 5b e8          	cmp    %bl,-0x17a5
    7dfe:	68 04 24             	push   $0x2404
    7e01:	f6 5c 13             	negb   0x13(%si)
    7e04:	b8 61 05             	mov    $0x561,%ax
    7e07:	19 b8 26 d9          	sbb    %di,-0x26da(%bx,%si)
    7e0b:	3e 43                	ds inc %bx
    7e0d:	53                   	push   %bx
    7e0e:	1c b8                	sbb    $0xb8,%al
    7e10:	32 dc                	xor    %ah,%bl
    7e12:	60                   	pusha
    7e13:	60                   	pusha
    7e14:	1e                   	push   %ds
    7e15:	1e                   	push   %ds
    7e16:	5b                   	pop    %bx
    7e17:	a1 06 74             	mov    0x7406,%ax
    7e1a:	0a 74 22             	or     0x22(%si),%dh
    7e1d:	74 54                	je     0x7e73
    7e1f:	6b 50 0e 54          	imul   $0x54,0xe(%bx,%si),%dx
    7e23:	6b 50 16 eb          	imul   $0xffeb,0x16(%bx,%si),%dx
    7e27:	1e                   	push   %ds
    7e28:	c0 e8 ba             	shr    $0xba,%al
    7e2b:	00 e8                	add    %ch,%al
    7e2d:	f8                   	clc
    7e2e:	eb 12                	jmp    0x7e42
    7e30:	c0 ba e8 eb c0       	sarb   $0xc0,-0x1418(%bp,%si)
    7e35:	e8 8a 87             	call   0x5c2
    7e38:	44                   	inc    %sp
    7e39:	f8                   	clc
    7e3a:	eb 0a                	jmp    0x7e46
    7e3c:	c0 ba e8 eb c0       	sarb   $0xc0,-0x1418(%bp,%si)
    7e41:	b8 8a 87             	mov    $0x878a,%ax
    7e44:	44                   	inc    %sp
    7e45:	80 14 fa             	adcb   $0xfa,(%si)
    7e48:	33 e8                	xor    %ax,%bp
    7e4a:	31 fe                	xor    %di,%si
    7e4c:	15 29 23             	adc    $0x2329,%ax
    7e4f:	5b                   	pop    %bx
    7e50:	a3 52 9b             	mov    %ax,0x9b52
    7e53:	03 06 a9 13          	add    0x13a9,%ax
    7e57:	b8 88 05             	mov    $0x588,%ax
    7e5a:	19 b8 14 58          	sbb    %di,0x5814(%bx,%si)
    7e5e:	2f                   	das
    7e5f:	f0 c6                	lock (bad)
    7e61:	1c 32                	sbb    $0x32,%al
    7e63:	60                   	pusha
    7e64:	1e                   	push   %ds
    7e65:	5b                   	pop    %bx
    7e66:	06                   	push   %es
    7e67:	cb                   	lret
    7e68:	1d 5b e8             	sbb    $0xe85b,%ax
    7e6b:	cd 05                	int    $0x5
    7e6d:	ff                   	ljmp   (bad)
    7e6e:	eb 8b                	jmp    0x7dfb
    7e70:	47                   	inc    %di
    7e71:	f0 00 5b b8          	lock add %bl,-0x48(%bp,%di)
    7e75:	fa                   	cli
    7e76:	bd 53 51             	mov    $0x5153,%bp
    7e79:	52                   	push   %dx
    7e7a:	06                   	push   %es
    7e7b:	56                   	push   %si
    7e7c:	57                   	push   %di
    7e7d:	b8 c0 7e             	mov    $0x7ec0,%ax
    7e80:	50                   	push   %ax
    7e81:	8b 7e 08             	mov    0x8(%bp),%di
    7e84:	8b 4e 04             	mov    0x4(%bp),%cx
    7e87:	a1 24 9b             	mov    0x9b24,%ax
    7e8a:	03 06 74 1e          	add    0x1e74,%ax
    7e8e:	3b f9                	cmp    %cx,%di
    7e90:	7c 02                	jl     0x7e94
    7e92:	87 f9                	xchg   %di,%cx
    7e94:	3b 3e dc c0          	cmp    -0x3f24,%di
    7e98:	36 02 8b 3e dc       	add    %ss:-0x23c2(%bp,%di),%cl
    7e9d:	c0 3b 0e             	sarb   $0xe,(%bp,%di)
    7ea0:	e0 c0                	loopne 0x7e62
    7ea2:	7e 02                	jle    0x7ea6
    7ea4:	8b 0e e0 c0          	mov    -0x3f20,%cx
    7ea8:	3b cf                	cmp    %di,%cx
    7eaa:	7c 13                	jl     0x7ebf
    7eac:	2b cf                	sub    %di,%cx
    7eae:	41                   	inc    %cx
    7eaf:	89 0e ce 9a          	mov    %cx,-0x6532
    7eb3:	8b 76 0a             	mov    0xa(%bp),%si
    7eb6:	8b 56 2a             	mov    0x2a(%bp),%dx
    7eb9:	4e                   	dec    %si
    7eba:	e9 95 5b             	jmp    0xda52
    7ebd:	eb 3b                	jmp    0x7efa
    7ebf:	58                   	pop    %ax
    7ec0:	5f                   	pop    %di
    7ec1:	5e                   	pop    %si
    7ec2:	07                   	pop    %es
    7ec3:	5a                   	pop    %dx
    7ec4:	59                   	pop    %cx
    7ec5:	5b                   	pop    %bx
    7ec6:	ec                   	in     (%dx),%al
    7ec7:	fa                   	cli
    7ec8:	08 4c 2b             	or     %cl,0x2b(%si)
    7ecb:	46                   	inc    %si
    7ecc:	0a 53 52             	or     0x52(%bp,%di),%dl
    7ecf:	56                   	push   %si
    7ed0:	b8 7e 8b             	mov    $0x8b7e,%ax
    7ed3:	08 32                	or     %dh,(%bp,%si)
    7ed5:	7f 50                	jg     0x7f27
    7ed7:	8b 7e 06             	mov    0x6(%bp),%di
    7eda:	8b 76 08             	mov    0x8(%bp),%si
    7edd:	8b 56 04             	mov    0x4(%bp),%dx
    7ee0:	c0 ba ce 9a 37       	sarb   $0x37,-0x6532(%bp,%si)
    7ee5:	d9 f7                	fincstp
    7ee7:	06                   	push   %es
    7ee8:	24 9b                	and    $0x9b,%al
    7eea:	ff                   	(bad)
    7eeb:	ff 74 2a             	push   0x2a(%si)
    7eee:	3b 3e dc c0          	cmp    -0x3f24,%di
    7ef2:	7c 3b                	jl     0x7f2f
    7ef4:	3b 3e e0 c0          	cmp    -0x3f20,%di
    7ef8:	7f 35                	jg     0x7f2f
    7efa:	3b f2                	cmp    %dx,%si
    7efc:	7c 02                	jl     0x7f00
    7efe:	87 f2                	xchg   %si,%dx
    7f00:	3b 36 da c0          	cmp    -0x3f26,%si
    7f04:	36 02 8b 36 da       	add    %ss:-0x25ca(%bp,%di),%cl
    7f09:	c0 3b 16             	sarb   $0x16,(%bp,%di)
    7f0c:	de c0                	faddp  %st,%st(0)
    7f0e:	7e 02                	jle    0x7f12
    7f10:	8b 16 de c0          	mov    -0x3f22,%dx
    7f14:	3b d6                	cmp    %si,%dx
    7f16:	7c 17                	jl     0x7f2f
    7f18:	a1 d4 c0             	mov    0xc0d4,%ax
    7f1b:	8e c0                	mov    %ax,%es
    7f1d:	d1 e7                	shl    $1,%di
    7f1f:	03 3e 32 f0          	add    -0xfce,%di
    7f23:	39 df                	cmp    %bx,%di
    7f25:	8b 2e 26 9b          	mov    -0x64da,%bp
    7f29:	8b 86 d0 9a          	mov    -0x6530(%bp),%ax
    7f2d:	ff e0                	jmp    *%ax
    7f2f:	58                   	pop    %ax
    7f30:	ff e0                	jmp    *%ax
    7f32:	5f                   	pop    %di
    7f33:	07                   	pop    %es
    7f34:	59                   	pop    %cx
    7f35:	ec                   	in     (%dx),%al
    7f36:	08 2b                	or     %ch,(%bp,%di)
    7f38:	0a 52 06             	or     0x6(%bp,%si),%dl
    7f3b:	00 87 f3 8b          	add    %al,-0x740d(%bx)
    7f3f:	cb                   	lret
    7f40:	d1 fb                	sar    $1,%bx
    7f42:	d1 fb                	sar    $1,%bx
    7f44:	d1 fb                	sar    $1,%bx
    7f46:	89 1e ca 9a          	mov    %bx,-0x6536
    7f4a:	80 e1 07             	and    $0x7,%cl
    7f4d:	5c                   	pop    %sp
    7f4e:	12 36 d3 f8          	adc    -0x72d,%dh
    7f52:	a2 c8 9a             	mov    %al,0x9ac8
    7f55:	8b ca                	mov    %dx,%cx
    7f57:	80 e1 07             	and    $0x7,%cl
    7f5a:	5c                   	pop    %sp
    7f5b:	80 ff d3             	cmp    $0xd3,%bh
    7f5e:	f8                   	clc
    7f5f:	a2 c9 9a             	mov    %al,0x9ac9
    7f62:	d1 fa                	sar    $1,%dx
    7f64:	d1 fa                	sar    $1,%dx
    7f66:	d1 fa                	sar    $1,%dx
    7f68:	2b d3                	sub    %bx,%dx
    7f6a:	75 0f                	jne    0x7f7b
    7f6c:	a0 c8 9a             	mov    0x9ac8,%al
    7f6f:	22 06 c9 9a          	and    -0x6537,%al
    7f73:	a2 c8 9a             	mov    %al,0x9ac8
    7f76:	88 16 c9 9a          	mov    %dl,-0x6537
    7f7a:	42                   	inc    %dx
    7f7b:	89 16 cc 9a          	mov    %dx,-0x6534
    7f7f:	ba ce 03             	mov    $0x3ce,%dx
    7f82:	b8 05 02             	mov    $0x205,%ax
    7f85:	ef                   	out    %ax,(%dx)
    7f86:	8b 1e fa 9a          	mov    -0x6506,%bx
    7f8a:	80 ff 40             	cmp    $0x40,%bh
    7f8d:	75 07                	jne    0x7f96
    7f8f:	ba ce 03             	mov    $0x3ce,%dx
    7f92:	b8 03 18             	mov    $0x1803,%ax
    7f95:	ef                   	out    %ax,(%dx)
    7f96:	8b 3d                	mov    (%di),%di
    7f98:	03 3e ca 9a          	add    -0x6536,%di
    7f9c:	03 3e d2 f0          	add    -0xf2e,%di
    7fa0:	39 0e cc 9a          	cmp    %cx,-0x6534
    7fa4:	ba ce 03             	mov    $0x3ce,%dx
    7fa7:	8a 26 c8 9a          	mov    -0x6538,%ah
    7fab:	b0 08                	mov    $0x8,%al
    7fad:	ef                   	out    %ax,(%dx)
    7fae:	8b c3                	mov    %bx,%ax
    7fb0:	26 86 05             	xchg   %al,%es:(%di)
    7fb3:	47                   	inc    %di
    7fb4:	49                   	dec    %cx
    7fb5:	74 19                	je     0x7fd0
    7fb7:	ba ce 03             	mov    $0x3ce,%dx
    7fba:	b4 ff                	mov    $0xff,%ah
    7fbc:	b0 08                	mov    $0x8,%al
    7fbe:	ef                   	out    %ax,(%dx)
    7fbf:	8b c3                	mov    %bx,%ax
    7fc1:	80 fc 40             	cmp    $0x40,%ah
    7fc4:	74 04                	je     0x7fca
    7fc6:	f3 aa                	rep stos %al,%es:(%di)
    7fc8:	eb 06                	jmp    0x7fd0
    7fca:	26 8a 25             	mov    %es:(%di),%ah
    7fcd:	aa                   	stos   %al,%es:(%di)
    7fce:	e2 fa                	loop   0x7fca
    7fd0:	ba ce 03             	mov    $0x3ce,%dx
    7fd3:	8a 26 c9 9a          	mov    -0x6537,%ah
    7fd7:	b0 ef                	mov    $0xef,%al
    7fd9:	c3                   	ret
    7fda:	86 47 74             	xchg   %al,0x74(%bx)
    7fdd:	ba 03 46             	mov    $0x4603,%dx
    7fe0:	46                   	inc    %si
    7fe1:	8b fe                	mov    %si,%di
    7fe3:	ff 0e 9a 76          	decw   0x769a
    7fe7:	75 ad                	jne    0x7f96
    7fe9:	ba ce 03             	mov    $0x3ce,%dx
    7fec:	b8 03 00             	mov    $0x3,%ax
    7fef:	ef                   	out    %ax,(%dx)
    7ff0:	ba 03 ff             	mov    $0xff03,%dx
    7ff3:	08 8b 80 40          	or     %cl,0x4080(%bp,%di)
    7ff7:	04 ba                	add    $0xba,%al
    7ff9:	ce                   	into
    7ffa:	03 b8 05 00          	add    0x5(%bx,%si),%di
    7ffe:	ef                   	out    %ax,(%dx)
    7fff:	58                   	pop    %ax
    8000:	ff e0                	jmp    *%ax
    8002:	2b d6                	sub    %si,%dx
    8004:	42                   	inc    %dx
    8005:	a1 fa 9a             	mov    0x9afa,%ax
    8008:	80 fc 40             	cmp    $0x40,%ah
    800b:	74 19                	je     0x8026
    800d:	8b ca                	mov    %dx,%cx
    800f:	8b 3d                	mov    (%di),%di
    8011:	03 fe                	add    %si,%di
    8013:	03 3e d2 f0          	add    -0xf2e,%di
    8017:	f3 aa                	rep stos %al,%es:(%di)
    8019:	43                   	inc    %bx
    801a:	43                   	inc    %bx
    801b:	8b fb                	mov    %bx,%di
    801d:	ff 0e 9a 76          	decw   0x769a
    8021:	75 ea                	jne    0x800d
    8023:	58                   	pop    %ax
    8024:	ff e0                	jmp    *%ax
    8026:	8b 8b 03 03          	mov    0x303(%bp,%di),%cx
    802a:	d2 f3                	shl    %cl,%bl
    802c:	43                   	inc    %bx
    802d:	8b ff                	mov    %di,%di
    802f:	9a 26 30 05 47       	lcall  $0x4705,$0x3026
    8034:	e2 fa                	loop   0x8030
    8036:	43                   	inc    %bx
    8037:	8b ff                	mov    %di,%di
    8039:	9a 75 58 e0 8b       	lcall  $0x8be0,$0x5875
    803e:	03 e6                	add    %si,%sp
    8040:	58                   	pop    %ax
    8041:	ff e0                	jmp    *%ax
    8043:	42                   	inc    %dx
    8044:	d1 fa                	sar    $1,%dx
    8046:	d0 d5                	rcl    $1,%ch
    8048:	8b c6                	mov    %si,%ax
    804a:	40                   	inc    %ax
    804b:	d1 f8                	sar    $1,%ax
    804d:	d0 d5                	rcl    $1,%ch
    804f:	2b d0                	sub    %ax,%dx
    8051:	89 16 cc 9a          	mov    %dx,-0x6534
    8055:	8b d3                	mov    %bx,%dx
    8057:	d1 fe                	sar    $1,%si
    8059:	8a e5                	mov    %ch,%ah
    805b:	8b 1e fa 9a          	mov    -0x6506,%bx
    805f:	8b eb                	mov    %bx,%bp
    8061:	83 e3 0f             	and    $0xf,%bx
    8064:	8a 9f 04 9b          	mov    -0x64fc(%bx),%bl
    8068:	03 d2                	add    %dx,%dx
    806a:	43                   	inc    %bx
    806b:	ff 26 05 e2          	jmp    *-0x1dfb
    806f:	43                   	inc    %bx
    8070:	39 0e cc 9a          	cmp    %cx,-0x6534
    8074:	f7 c5 00 40          	test   $0x4000,%bp
    8078:	75 2a                	jne    0x80a4
    807a:	f6 c4 01             	test   $0x1,%ah
    807d:	75 0d                	jne    0x808c
    807f:	26 8a 05             	mov    %es:(%di),%al
    8082:	8a fb                	mov    %bl,%bh
    8084:	80 e7 0f             	and    $0xf,%bh
    8087:	24 f0                	and    $0xf0,%al
    8089:	0a c7                	or     %bh,%al
    808b:	aa                   	stos   %al,%es:(%di)
    808c:	8a c3                	mov    %bl,%al
    808e:	f3 aa                	rep stos %al,%es:(%di)
    8090:	f6 c4 02             	test   $0x2,%ah
    8093:	74 34                	je     0x80c9
    8095:	26 05 fb e7          	es add $0xe7fb,%ax
    8099:	24 0a                	and    $0xa,%al
    809b:	aa                   	stos   %al,%es:(%di)
    809c:	f0 24 0f             	lock and $0xf,%al
    809f:	0a c7                	or     %bh,%al
    80a1:	aa                   	stos   %al,%es:(%di)
    80a2:	eb 25                	jmp    0x80c9
    80a4:	f6 c4 01             	test   $0x1,%ah
    80a7:	75 09                	jne    0x80b2
    80a9:	8a fb                	mov    %bl,%bh
    80ab:	80 e7 0f             	and    $0xf,%bh
    80ae:	26 30 3d             	xor    %bh,%es:(%di)
    80b1:	47                   	inc    %di
    80b2:	0b c9                	or     %cx,%cx
    80b4:	74 06                	je     0x80bc
    80b6:	26 30 1d             	xor    %bl,%es:(%di)
    80b9:	47                   	inc    %di
    80ba:	e2 fa                	loop   0x80b6
    80bc:	f6 c4 02             	test   $0x2,%ah
    80bf:	74 08                	je     0x80c9
    80c1:	e7 24                	out    %ax,$0x24
    80c3:	0a aa f0 26          	or     0x26f0(%bp,%si),%ch
    80c7:	30 3d                	xor    %bh,(%di)
    80c9:	42                   	inc    %dx
    80ca:	42                   	inc    %dx
    80cb:	8b fa                	mov    %dx,%di
    80cd:	75 58                	jne    0x8127
    80cf:	e0 8b                	loopne 0x805c
    80d1:	03 95 58 ff          	add    -0xa8(%di),%dx
    80d5:	e0 53                	loopne 0x812a
    80d7:	56                   	push   %si
    80d8:	7e 7e                	jle    0x8158
    80da:	08 7f 00             	or     %bh,0x0(%bx)
    80dd:	1a cd                	sbb    %ch,%cl
    80df:	10 3c                	adc    %bh,(%si)
    80e1:	1a 75 0f             	sbb    0xf(%di),%dh
    80e4:	80 fb 0c             	cmp    $0xc,%bl
    80e7:	77 0a                	ja     0x80f3
    80e9:	32 58 c6             	xor    -0x3a(%bx,%si),%bl
    80ec:	58                   	pop    %ax
    80ed:	da 9a 3c 00          	ficompl 0x3c(%bp,%si)
    80f1:	75 6d                	jne    0x8160
    80f3:	e8 ae 03             	call   0x84a4
    80f6:	24 07                	and    $0x7,%al
    80f8:	3c 07                	cmp    $0x7,%al
    80fa:	b0 82                	mov    $0x82,%al
    80fc:	74 62                	je     0x8160
    80fe:	b4 12                	mov    $0x12,%ah
    8100:	b3 10                	mov    $0x10,%bl
    8102:	cd 10                	int    $0x10
    8104:	80 fb 10             	cmp    $0x10,%bl
    8107:	74 27                	je     0x8130
    8109:	01 5b e8             	add    %bx,-0x18(%bp,%di)
    810c:	8e 9a 26 a0          	mov    -0x5fda(%bp,%si),%ds
    8110:	87 00                	xchg   %ax,(%bx,%si)
    8112:	24 08                	and    $0x8,%al
    8114:	b0 86                	mov    $0x86,%al
    8116:	75 48                	jne    0x8160
    8118:	32 58 43             	xor    0x43(%bx,%si),%bl
    811b:	f8                   	clc
    811c:	b1 03                	mov    $0x3,%cl
    811e:	d3 db                	rcr    %cl,%bx
    8120:	4b                   	dec    %bx
    8121:	89 1e d8 9a          	mov    %bx,-0x6528
    8125:	b0 04                	mov    $0x4,%al
    8127:	83 9a 75 73 34       	sbbw   $0x34,0x7375(%bp,%si)
    812c:	b0 83                	mov    $0x83,%al
    812e:	72 30                	jb     0x8160
    8130:	7f 00                	jg     0x8132
    8132:	f0 8e 9a b0 06       	lock mov 0x6b0(%bp,%si),%ds
    8137:	8a aa 1e 00          	mov    0x1e(%bp,%si),%ch
    813b:	c0 80 fb 21 74       	rolb   $0x74,0x21fb(%bx,%si)
    8140:	1f                   	pop    %ds
    8141:	ba d4 03             	mov    $0x3d4,%dx
    8144:	b0 0f                	mov    $0xf,%al
    8146:	ee                   	out    %al,(%dx)
    8147:	42                   	inc    %dx
    8148:	ec                   	in     (%dx),%al
    8149:	8a f8                	mov    %al,%bh
    814b:	b0 47                	mov    $0x47,%al
    814d:	ee                   	out    %al,(%dx)
    814e:	b9 00 01             	mov    $0x100,%cx
    8151:	e2 fe                	loop   0x8151
    8153:	ec                   	in     (%dx),%al
    8154:	86 f8                	xchg   %bh,%al
    8156:	ee                   	out    %al,(%dx)
    8157:	b0 84                	mov    $0x84,%al
    8159:	80 ff 47             	cmp    $0x47,%bh
    815c:	74 02                	je     0x8160
    815e:	b0 81                	mov    $0x81,%al
    8160:	32 e4                	xor    %ah,%ah
    8162:	33 db                	xor    %bx,%bx
    8164:	a8 80                	test   $0x80,%al
    8166:	75 04                	jne    0x816c
    8168:	8b d8                	mov    %ax,%bx
    816a:	33 c0                	xor    %ax,%ax
    816c:	25 7f 00             	and    $0x7f,%ax
    816f:	89 1e b2 9a          	mov    %bx,-0x654e
    8173:	a3 b4 9a             	mov    %ax,0x9ab4
    8176:	5f                   	pop    %di
    8177:	5e                   	pop    %si
    8178:	ec                   	in     (%dx),%al
    8179:	2b 52 19             	sub    0x19(%bp,%si),%dx
    817c:	73 3e                	jae    0x81bc
    817e:	1a 53 7e             	sbb    0x7e(%bp,%di),%dl
    8181:	08 8b 08 56          	or     %cl,0x5608(%bp,%di)
    8185:	cd f6                	int    $0xf6
    8187:	b8 4a 8b             	mov    $0x8b4a,%ax
    818a:	06                   	push   %es
    818b:	76 0b                	jbe    0x8198
    818d:	f7 51 43             	notw   0x43(%bx,%di)
    8190:	04 eb                	add    $0xeb,%al
    8192:	8b 0b                	mov    (%bp,%di),%cx
    8194:	db 75 03             	(bad) 0x3(%di)
    8197:	bb e8 9a             	mov    $0x9ae8,%bx
    819a:	8b 04                	mov    (%si),%ax
    819c:	3b 05                	cmp    (%di),%ax
    819e:	7d 02                	jge    0x81a2
    81a0:	8b 05                	mov    (%di),%ax
    81a2:	89 07                	mov    %ax,(%bx)
    81a4:	8b 44 02             	mov    0x2(%si),%ax
    81a7:	3b 45 02             	cmp    0x2(%di),%ax
    81aa:	7d 03                	jge    0x81af
    81ac:	8b 45 02             	mov    0x2(%di),%ax
    81af:	89 47 02             	mov    %ax,0x2(%bx)
    81b2:	8b 44 04             	mov    0x4(%si),%ax
    81b5:	3b 45 04             	cmp    0x4(%di),%ax
    81b8:	7e 03                	jle    0x81bd
    81ba:	8b 45 04             	mov    0x4(%di),%ax
    81bd:	89 47 04             	mov    %ax,0x4(%bx)
    81c0:	8b 44 06             	mov    0x6(%si),%ax
    81c3:	3b 45 06             	cmp    0x6(%di),%ax
    81c6:	7e 03                	jle    0x81cb
    81c8:	8b 45 06             	mov    0x6(%di),%ax
    81cb:	89 47 06             	mov    %ax,0x6(%bx)
    81ce:	3b 47 02             	cmp    0x2(%bx),%ax
    81d1:	7d 04                	jge    0x81d7
    81d3:	33 c0                	xor    %ax,%ax
    81d5:	eb 0c                	jmp    0x81e3
    81d7:	8b 47 04             	mov    0x4(%bx),%ax
    81da:	3b 07                	cmp    (%bx),%ax
    81dc:	7d 02                	jge    0x81e0
    81de:	7c f3                	jl     0x81d3
    81e0:	f9                   	stc
    81e1:	74 6f                	je     0x8252
    81e3:	5f                   	pop    %di
    81e4:	5e                   	pop    %si
    81e5:	2b 52 00             	sub    0x0(%bp,%si),%dx
    81e8:	f3 cb                	repz lret
    81ea:	73 1a                	jae    0x8206
    81ec:	7e 8b                	jle    0x8179
    81ee:	56                   	push   %si
    81ef:	f6 4a 06 0b          	testb  $0xb,0x6(%bp,%si)
    81f3:	8b 06 76 8b          	mov    -0x748a,%ax
    81f7:	eb 8b                	jmp    0x8184
    81f9:	0b db                	or     %bx,%bx
    81fb:	74 34                	je     0x8231
    81fd:	8b 04                	mov    (%si),%ax
    81ff:	3b 05                	cmp    (%di),%ax
    8201:	7e 02                	jle    0x8205
    8203:	05 07 44             	add    $0x4407,%ax
    8206:	3b 02                	cmp    (%bp,%si),%ax
    8208:	03 45 89             	add    -0x77(%di),%ax
    820b:	02 44 7e             	add    0x7e(%si),%al
    820e:	03 8b 45 02          	add    0x245(%bp,%di),%cx
    8212:	47                   	inc    %di
    8213:	8b 04                	mov    (%si),%ax
    8215:	45                   	inc    %bp
    8216:	7e 8b                	jle    0x81a3
    8218:	04 47                	add    $0x47,%al
    821a:	8b 7d 03             	mov    0x3(%di),%di
    821d:	8b 45 04             	mov    0x4(%di),%ax
    8220:	47                   	inc    %di
    8221:	8b 06 45 7e          	mov    0x7e45,%ax
    8225:	8b 06 47 3b          	mov    0x3b47,%ax
    8229:	7d 03                	jge    0x822e
    822b:	8b 45 06             	mov    0x6(%di),%ax
    822e:	89 47 06             	mov    %ax,0x6(%bx)
    8231:	5f                   	pop    %di
    8232:	2b 00                	sub    (%bx,%si),%ax
    8234:	cb                   	lret
    8235:	1a 8b f6 06          	sbb    0x6f6(%bp,%di),%cl
    8239:	8b 76 eb             	mov    -0x15(%bp),%si
    823c:	8b 8b db 34          	mov    0x34db(%bp,%di),%cx
    8240:	04 05                	add    $0x5,%al
    8242:	1e                   	push   %ds
    8243:	46                   	inc    %si
    8244:	f4                   	hlt
    8245:	fc                   	cld
    8246:	3b 07                	cmp    (%bx),%ax
    8248:	7c 17                	jl     0x8261
    824a:	3b 47 04             	cmp    0x4(%bx),%ax
    824d:	7f 12                	jg     0x8261
    824f:	e0 b4                	loopne 0x8205
    8251:	32 3b                	xor    (%bp,%di),%bh
    8253:	47                   	inc    %di
    8254:	02 7c 0a             	add    0xa(%si),%bh
    8257:	3b 47 06             	cmp    0x6(%bx),%ax
    825a:	7f 05                	jg     0x8261
    825c:	f9                   	stc
    825d:	74 6f                	je     0x82ce
    825f:	eb 02                	jmp    0x8263
    8261:	33 c0                	xor    %ax,%ax
    8263:	00 cb                	add    %cl,%bl
    8265:	1a 8b f6 53          	sbb    0x53f6(%bp,%di),%cl
    8269:	7e 08                	jle    0x8273
    826b:	cd 12                	int    $0x12
    826d:	a3 f2 9a             	mov    %ax,0x9af2
    8270:	3d 80 02             	cmp    $0x280,%ax
    8273:	74 5c                	je     0x82d1
    8275:	83 5b 65 e2          	sbbw   $0xffe2,0x65(%bp,%di)
    8279:	e4 1d                	in     $0x1d,%al
    827b:	55                   	push   %bp
    827c:	32 db                	xor    %bl,%bl
    827e:	b7 08                	mov    $0x8,%bh
    8280:	80 3e f8 9a 00       	cmpb   $0x0,-0x6508
    8285:	74 03                	je     0x828a
    8287:	80 c7 08             	add    $0x8,%bh
    828a:	53                   	push   %bx
    828b:	b8 00 70             	mov    $0x7000,%ax
    828e:	bb 01 00             	mov    $0x1,%bx
    8291:	cd 15                	int    $0x15
    8293:	73 02                	jae    0x8297
    8295:	33 d2                	xor    %dx,%dx
    8297:	8a de                	mov    %dh,%bl
    8299:	83 e3 03             	and    $0x3,%bx
    829c:	8a a7 f4 9a          	mov    -0x650c(%bx),%ah
    82a0:	32 c0                	xor    %al,%al
    82a2:	00 2b                	add    %ch,(%bp,%di)
    82a4:	d8 7e 2a             	fdivrs 0x2a(%bp)
    82a7:	a1 f2 9a             	mov    0x9af2,%ax
    82aa:	b1 06                	mov    $0x6,%cl
    82ac:	d3 e0                	shl    %cl,%ax
    82ae:	2b c3                	sub    %bx,%ax
    82b0:	c7                   	(bad)
    82b1:	7e e4                	jle    0x8297
    82b3:	03 2b                	add    (%bp,%di),%bp
    82b5:	c3                   	ret
    82b6:	48                   	dec    %ax
    82b7:	c7                   	(bad)
    82b8:	7e 83                	jle    0x823d
    82ba:	03 50 53             	add    0x53(%bx,%si),%dx
    82bd:	7e e4                	jle    0x82a3
    82bf:	03 8b d8 e8          	add    -0x1728(%bp,%di),%cx
    82c3:	c7 03 33 c0          	movw   $0xc033,(%bp,%di)
    82c7:	59                   	pop    %cx
    82c8:	3b cb                	cmp    %bx,%cx
    82ca:	74 09                	je     0x82d5
    82cc:	f9                   	stc
    82cd:	74 6f                	je     0x833e
    82cf:	eb 04                	jmp    0x82d5
    82d1:	33 db                	xor    %bx,%bx
    82d3:	33 c0                	xor    %ax,%ax
    82d5:	89 1e f0 9a          	mov    %bx,-0x6510
    82d9:	ec                   	in     (%dx),%al
    82da:	52                   	push   %dx
    82db:	73 1a                	jae    0x82f7
    82dd:	4c                   	dec    %sp
    82de:	46                   	inc    %si
    82df:	53                   	push   %bx
    82e0:	56                   	push   %si
    82e1:	7e 08                	jle    0x82eb
    82e3:	7f 8b                	jg     0x8270
    82e5:	06                   	push   %es
    82e6:	76 a1                	jbe    0x8289
    82e8:	c0 c0 e7             	rol    $0xe7,%al
    82eb:	3e 8b 4e 76          	mov    %ds:0x76(%bp),%cx
    82ef:	8b 77 0e             	mov    0xe(%bx),%si
    82f2:	8a 06 9b ff          	mov    -0x65,%al
    82f6:	2a 3e c0 18          	sub    0x18c0,%bh
    82fa:	3b 06 dc 7c          	cmp    0x7cdc,%ax
    82fe:	3b 6a 3b             	cmp    0x3b(%bp,%si),%bp
    8301:	06                   	push   %es
    8302:	e0 7f                	loopne 0x8383
    8304:	3b 64 3b             	cmp    0x3b(%si),%sp
    8307:	0e                   	push   %cs
    8308:	da 7c 3b             	fidivrl 0x3b(%si)
    830b:	5e                   	pop    %si
    830c:	3b 0e de 7f          	cmp    0x7fde,%cx
    8310:	3b 58 8b             	cmp    -0x75(%bx,%si),%bx
    8313:	3e f0 df 2e d8 4e    	lock fildll %ds:0x4ed8
    8319:	57                   	push   %di
    831a:	8b 39                	mov    (%bx,%di),%di
    831c:	26 e2 39             	es loop 0x8358
    831f:	cc                   	int3
    8320:	f7 1e 26 8b          	negw   -0x74da
    8324:	d0 87 14 ff          	rolb   $1,-0xec(%bx)
    8328:	2a e0                	sub    %al,%ah
    832a:	03 f9                	add    %cx,%di
    832c:	a1 9a fc             	mov    0xfc9a,%ax
    832f:	74 8b                	je     0x82bc
    8331:	8b 03                	mov    (%bp,%di),%ax
    8333:	05 26 88             	add    $0x8826,%ax
    8336:	05 eb 31             	add    $0x31eb,%ax
    8339:	26 05 e2 eb          	es add $0xebe2,%ax
    833d:	2c 8b                	sub    $0x8b,%al
    833f:	fa                   	cli
    8340:	8b 83 8a f7          	mov    -0x876(%bp,%di),%ax
    8344:	83 0f 9f             	orw    $0xff9f,(%bx)
    8347:	9b                   	fwait
    8348:	97                   	xchg   %ax,%di
    8349:	04 9b                	add    $0x9b,%al
    834b:	d1 e9                	shr    $1,%cx
    834d:	bb f0 0f             	mov    $0xff0,%bx
    8350:	72 02                	jb     0x8354
    8352:	f7 d3                	not    %bx
    8354:	03 f9                	add    %cx,%di
    8356:	80 fe 40             	cmp    $0x40,%dh
    8359:	07                   	pop    %es
    835a:	ce                   	into
    835b:	22 d7                	and    %bh,%dl
    835d:	26 05 15 eb          	es add $0xeb15,%ax
    8361:	08 22                	or     %ah,(%bp,%si)
    8363:	d7                   	xlat   %ds:(%bx)
    8364:	26 20 1d             	and    %bl,%es:(%di)
    8367:	26 08 15             	or     %dl,%es:(%di)
    836a:	5f                   	pop    %di
    836b:	59                   	pop    %cx
    836c:	08 0a                	or     %cl,(%bp,%si)
    836e:	06                   	push   %es
    836f:	87 8b d1 04          	xchg   %cx,0x4d1(%bp,%di)
    8373:	00 ba 03 05          	add    %bh,0x503(%bp,%si)
    8377:	ef                   	out    %ax,(%dx)
    8378:	1e                   	push   %ds
    8379:	9a ff 75 f1 83       	lcall  $0x83f1,$0x75ff
    837e:	e6 07                	out    %al,$0x7
    8380:	d1 f9                	sar    $1,%cx
    8382:	d1 f9                	sar    $1,%cx
    8384:	d1 f9                	sar    $1,%cx
    8386:	03 f9                	add    %cx,%di
    8388:	ba 03 26             	mov    $0x2603,%dx
    838b:	9a a4 fc 9a ef       	lcall  $0xef9a,$0xfca4
    8390:	86 74 03             	xchg   %dh,0x3(%si)
    8393:	1e                   	push   %ds
    8394:	9a ff 75 ba 03       	lcall  $0x3ba,$0x75ff
    8399:	03 ef                	add    %di,%bp
    839b:	3d 3e 9a             	cmp    $0x9a3e,%ax
    839e:	3e f0 0e             	ds lock push %cs
    83a1:	9a 26 86 1d ba       	lcall  $0xba1d,$0x8626
    83a6:	03 03                	add    (%bp,%di),%ax
    83a8:	ef                   	out    %ax,(%dx)
    83a9:	03 08                	add    (%bx,%si),%cx
    83ab:	80 04 ce             	addb   $0xce,(%si)
    83ae:	b8 00 58             	mov    $0x5800,%ax
    83b1:	e0 d6                	loopne 0x8389
    83b3:	a1 9a fc             	mov    0xfc9a,%ax
    83b6:	74 8b                	je     0x8343
    83b8:	8b 03                	mov    (%bp,%di),%ax
    83ba:	03 eb                	add    %bx,%bp
    83bc:	ad                   	lods   %ds:(%si),%ax
    83bd:	46                   	inc    %si
    83be:	56                   	push   %si
    83bf:	08 8b 76 c0          	or     %cl,-0x3f8a(%bp,%di)
    83c3:	e7 57                	out    %ax,$0x57
    83c5:	8b 3e f0 df          	mov    -0x2010,%di
    83c9:	a1 c0 3e             	mov    0x3ec0,%ax
    83cc:	4e                   	dec    %si
    83cd:	8b 0e 04 e8          	mov    -0x17fc,%cx
    83d1:	8b 96 81 39          	mov    0x3981(%bp),%dx
    83d5:	e2 cc                	loop   0x83a3
    83d7:	1e                   	push   %ds
    83d8:	8b 87 ff e0          	mov    -0x1f01(%bx),%ax
    83dc:	f9                   	stc
    83dd:	9a 74 1c ff 2a       	lcall  $0x2aff,$0x1c74
    83e2:	e0 03                	loopne 0x83e7
    83e4:	7e 06                	jle    0x83ec
    83e6:	aa                   	stos   %al,%es:(%di)
    83e7:	00 e2                	add    %ah,%dl
    83e9:	eb 44                	jmp    0x842f
    83eb:	b1 03                	mov    $0x3,%cl
    83ed:	e0 32                	loopne 0x8421
    83ef:	47                   	inc    %di
    83f0:	2e d8 83 0f 07       	fadds  %cs:0x70f(%bp,%di)
    83f5:	d3 e8                	shr    %cl,%ax
    83f7:	03 f8                	add    %ax,%di
    83f9:	32 ed                	xor    %ch,%ch
    83fb:	d0 e5                	shl    $1,%ch
    83fd:	ba 03 26             	mov    $0x2603,%dx
    8400:	9a e1 b0 04 9a       	lcall  $0x9a04,$0xb0e1
    8405:	26 00 e2             	es add %ah,%dl
    8408:	22 87 fc 9a          	and    -0x6504(%bx),%al
    840c:	74 80                	je     0x838e
    840e:	08 cd                	or     %cl,%ch
    8410:	01 fe                	add    %di,%si
    8412:	c9                   	leave
    8413:	79 e6                	jns    0x83fb
    8415:	8a c5                	mov    %ch,%al
    8417:	eb 16                	jmp    0x842f
    8419:	e0 32                	loopne 0x844d
    841b:	47                   	inc    %di
    841c:	2e c8 d1 e8 03       	cs enter $0xe8d1,$0x3
    8421:	f8                   	clc
    8422:	26 00 e2             	es add %ah,%dl
    8425:	d1 e9                	shr    $1,%cx
    8427:	72 04                	jb     0x842d
    8429:	b1 04                	mov    $0x4,%cl
    842b:	d2 e8                	shr    %cl,%al
    842d:	24 0f                	and    $0xf,%al
    842f:	32 e4                	xor    %ah,%ah
    8431:	5f                   	pop    %di
    8432:	08 06 8b 04          	or     %al,0x48b
    8436:	ba 05 1e             	mov    $0x1e05,%dx
    8439:	ff 53 7e             	call   *0x7e(%bp,%di)
    843c:	08 00                	or     %al,(%bx,%si)
    843e:	fc                   	cld
    843f:	8c d8                	mov    %ds,%ax
    8441:	a3 c8 43             	mov    %ax,0x43c8
    8444:	0e                   	push   %cs
    8445:	9a 1e 8b 87 ff       	lcall  $0xff87,$0x8b1e
    844a:	f0 2e 4e             	lock cs dec %si
    844d:	36 e0 f9             	ss loopne 0x8449
    8450:	9a 94 28 f9 9a       	lcall  $0x9af9,$0x2894
    8455:	84 30                	test   %dh,(%bx,%si)
    8457:	9b                   	fwait
    8458:	33 f6                	xor    %si,%si
    845a:	2a e0                	sub    %al,%ah
    845c:	b0 0d                	mov    $0xd,%al
    845e:	eb 02                	jmp    0x8462
    8460:	b0 13                	mov    $0x13,%al
    8462:	89 08                	mov    %cx,(%bx,%si)
    8464:	03 ca                	add    %dx,%cx
    8466:	83 c6 02             	add    $0x2,%si
    8469:	81 fe 90 01          	cmp    $0x190,%si
    846d:	75 f3                	jne    0x8462
    846f:	eb 23                	jmp    0x8494
    8471:	8b c1                	mov    %cx,%ax
    8473:	89 00                	mov    %ax,(%bx,%si)
    8475:	8b 03                	mov    (%bp,%di),%ax
    8477:	20 89 40 02          	and    %cl,0x240(%bx,%di)
    847b:	8b 03                	mov    (%bp,%di),%ax
    847d:	20 89 40 04          	and    %cl,0x440(%bx,%di)
    8481:	8b 03                	mov    (%bp,%di),%ax
    8483:	20 89 40 06          	and    %cl,0x640(%bx,%di)
    8487:	03 ca                	add    %dx,%cx
    8489:	83 c6 08             	add    $0x8,%si
    848c:	81 fe 90 01          	cmp    $0x190,%si
    8490:	75 df                	jne    0x8471
    8492:	b0 09                	mov    $0x9,%al
    8494:	32 e4                	xor    %ah,%ah
    8496:	cd 10                	int    $0x10
    8498:	5e                   	pop    %si
    8499:	2b 19                	sub    (%bx,%di),%bx
    849b:	3e 53                	ds push %bx
    849d:	b0 03                	mov    $0x3,%al
    849f:	32 e4                	xor    %ah,%ah
    84a1:	cd 10                	int    $0x10
    84a3:	c3                   	ret
    84a4:	b4 0f                	mov    $0xf,%ah
    84a6:	cd 10                	int    $0x10
    84a8:	24 7f                	and    $0x7f,%al
    84aa:	32 e4                	xor    %ah,%ah
    84ac:	19 3e 53 08          	sbb    %di,0x853
    84b0:	2a 14                	sub    (%si),%dl
    84b2:	b8 32 e4             	mov    $0xe432,%ax
    84b5:	cd 10                	int    $0x10
    84b7:	32 b1 57 8b          	xor    -0x74a9(%bx,%di),%dh
    84bb:	52                   	push   %dx
    84bc:	ba da 03             	mov    $0x3da,%dx
    84bf:	ec                   	in     (%dx),%al
    84c0:	a8 08                	test   $0x8,%al
    84c2:	74 fb                	je     0x84bf
    84c4:	ec                   	in     (%dx),%al
    84c5:	a8 08                	test   $0x8,%al
    84c7:	75 fb                	jne    0x84c4
    84c9:	5a                   	pop    %dx
    84ca:	1a 46 56             	sbb    0x56(%bp),%al
    84cd:	08 8b a2 14          	or     %cl,0x14a2(%bp,%di)
    84d1:	63 53 7e             	arpl   %dx,0x7e(%bp,%di)
    84d4:	56                   	push   %si
    84d5:	57                   	push   %di
    84d6:	1e                   	push   %ds
    84d7:	8b 5e 08             	mov    0x8(%bp),%bx
    84da:	c7 46 f4 c3 8a       	movw   $0x8ac3,-0xc(%bp)
    84df:	04 47                	add    $0x47,%al
    84e1:	08 88 46 f0          	or     %cl,-0xfba(%bx,%si)
    84e5:	04 47                	add    $0x47,%al
    84e7:	06                   	push   %es
    84e8:	46                   	inc    %si
    84e9:	05 ec 0e             	add    $0xeec,%ax
    84ec:	fa                   	cli
    84ed:	8b 07                	mov    (%bx),%ax
    84ef:	05 07 00             	add    $0x7,%ax
    84f2:	b1 d3                	mov    $0xd3,%cl
    84f4:	4b                   	dec    %bx
    84f5:	e8 88 47             	call   0xcc80
    84f8:	06                   	push   %es
    84f9:	04 47                	add    $0x47,%al
    84fb:	07                   	pop    %es
    84fc:	fd                   	std
    84fd:	46                   	inc    %si
    84fe:	ee                   	out    %al,(%dx)
    84ff:	f6 67 b4             	mulb   -0x4c(%bx)
    8502:	32 06 fe 8b          	xor    -0x7402,%al
    8506:	47                   	inc    %di
    8507:	0a 8b 46 f2          	or     -0xdba(%bp,%di),%cl
    850b:	8b 47 0c             	mov    0xc(%bx),%ax
    850e:	4e                   	dec    %si
    850f:	0e                   	push   %cs
    8510:	e8 32 47             	call   0xcc45
    8513:	8e d8                	mov    %ax,%ds
    8515:	c6 46 ec 00          	movb   $0x0,-0x14(%bp)
    8519:	b8 ff 04             	mov    $0x4ff,%ax
    851c:	77 2c                	ja     0x854a
    851e:	5e                   	pop    %si
    851f:	eb 2d                	jmp    0x854e
    8521:	f4                   	hlt
    8522:	d1 e8                	shr    $1,%ax
    8524:	d1 03                	rolw   $1,(%bp,%di)
    8526:	26 46                	es inc %si
    8528:	f2 f8                	repnz clc
    852a:	74 fe                	je     0x852a
    852c:	3d 24 90             	cmp    $0x9024,%ax
    852f:	7a e9                	jp     0x851a
    8531:	8b 76 5e             	mov    0x5e(%bp),%si
    8534:	eb 7e                	jmp    0x85b4
    8536:	f6 b9 04 00          	idivb  0x4(%bx,%di)
    853a:	ac                   	lods   %ds:(%si),%al
    853b:	d0 e0                	shl    $1,%al
    853d:	d0 d6                	rcl    $1,%dh
    853f:	d0 e0                	shl    $1,%al
    8541:	d0 d2                	rcl    $1,%dl
    8543:	d0 e0                	shl    $1,%al
    8545:	d0 d7                	rcl    $1,%bh
    8547:	d0 e0                	shl    $1,%al
    8549:	d0 d3                	rcl    $1,%bl
    854b:	d0 d0                	rcl    $1,%al
    854d:	d0 d0                	rcl    $1,%al
    854f:	d0 d0                	rcl    $1,%al
    8551:	d0 d0                	rcl    $1,%al
    8553:	d0 d0                	rcl    $1,%al
    8555:	d0 d0                	rcl    $1,%al
    8557:	d0 d0                	rcl    $1,%al
    8559:	d0 d0                	rcl    $1,%al
    855b:	e2 dd                	loop   0x853a
    855d:	8b cb                	mov    %bx,%cx
    855f:	2a 14                	sub    (%si),%dl
    8561:	f0 26 88 0d          	lock mov %cl,%es:(%di)
    8565:	d0 e8                	shr    $1,%al
    8567:	73 02                	jae    0x856b
    8569:	f6 d1                	not    %cl
    856b:	8a e1                	mov    %cl,%ah
    856d:	ec                   	in     (%dx),%al
    856e:	fe 07                	incb   (%bx)
    8570:	26 88 29             	mov    %ch,%es:(%bx,%di)
    8573:	d0 e8                	shr    $1,%al
    8575:	73 02                	jae    0x8579
    8577:	f6 d5                	not    %ch
    8579:	0a e5                	or     %ch,%ah
    857b:	8b cb                	mov    %bx,%cx
    857d:	03 d9                	add    %cx,%bx
    857f:	26 88 11             	mov    %dl,%es:(%bx,%di)
    8582:	d0 e8                	shr    $1,%al
    8584:	73 02                	jae    0x8588
    8586:	f6 d2                	not    %dl
    8588:	0a e2                	or     %dl,%ah
    858a:	03 d9                	add    %cx,%bx
    858c:	26 88 31             	mov    %dh,%es:(%bx,%di)
    858f:	d0 e8                	shr    $1,%al
    8591:	73 02                	jae    0x8595
    8593:	f6 d6                	not    %dh
    8595:	0a e6                	or     %dh,%ah
    8597:	03 d9                	add    %cx,%bx
    8599:	26 88 21             	mov    %ah,%es:(%bx,%di)
    859c:	b8 8b b8             	mov    $0xb88b,%ax
    859f:	01 2c                	add    %bp,(%si)
    85a1:	5e                   	pop    %si
    85a2:	01 2c                	add    %bp,(%si)
    85a4:	f4                   	hlt
    85a5:	ff 74 fe             	push   -0x2(%si)
    85a8:	01 2c                	add    %bp,(%si)
    85aa:	c1 00 39             	rolw   $0x39,(%bx,%si)
    85ad:	fa                   	cli
    85ae:	83 76 fc 73          	xorw   $0x73,-0x4(%bp)
    85b2:	03 e9                	add    %cx,%bp
    85b4:	7b ff                	jnp    0x85b5
    85b6:	01 2c                	add    %bp,(%si)
    85b8:	04 fe                	add    $0xfe,%al
    85ba:	77 8a                	ja     0x8546
    85bc:	ff 46 ee             	incw   -0x12(%bp)
    85bf:	3a 77 8a             	cmp    -0x76(%bx),%dh
    85c2:	5e                   	pop    %si
    85c3:	c7 01 52 ff          	movw   $0xff52,(%bx,%di)
    85c7:	1f                   	pop    %ds
    85c8:	5f                   	pop    %di
    85c9:	5e                   	pop    %si
    85ca:	59                   	pop    %cx
    85cb:	5b                   	pop    %bx
    85cc:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    85cd:	32 b1 57 8b          	xor    -0x74a9(%bx,%di),%dh
    85d1:	06                   	push   %es
    85d2:	76 8b                	jbe    0x855f
    85d4:	db 1e 56 57          	fistpl 0x5756
    85d8:	53                   	push   %bx
    85d9:	08 fc                	or     %bh,%ah
    85db:	0e                   	push   %cs
    85dc:	e8 96 04             	call   0x8a75
    85df:	47                   	inc    %di
    85e0:	07                   	pop    %es
    85e1:	8a 4f 06             	mov    0x6(%bx),%cl
    85e4:	f6 e1                	mul    %cl
    85e6:	ec                   	in     (%dx),%al
    85e7:	d0 04                	rolb   $1,(%si)
    85e9:	08 46 25             	or     %al,0x25(%bp)
    85ec:	0f 00 26 46 06       	verr   0x646
    85f1:	8b f8                	mov    %ax,%di
    85f3:	8a 05                	mov    (%di),%al
    85f5:	88 47 08             	mov    %al,0x8(%bx)
    85f8:	c4 7f 0a             	les    0xa(%bx),%di
    85fb:	c5 77 0a             	lds    0xa(%bx),%si
    85fe:	b1 04                	mov    $0x4,%cl
    8600:	32 ff                	xor    %bh,%bh
    8602:	76 8b                	jbe    0x858f
    8604:	46                   	inc    %si
    8605:	06                   	push   %es
    8606:	8b e8                	mov    %ax,%bp
    8608:	87 fd                	xchg   %di,%bp
    860a:	ac                   	lods   %ds:(%si),%al
    860b:	8a d8                	mov    %al,%bl
    860d:	80 e3 f0             	and    $0xf0,%bl
    8610:	d2 eb                	shr    %cl,%bl
    8612:	36 8a 21             	mov    %ss:(%bx,%di),%ah
    8615:	8a d8                	mov    %al,%bl
    8617:	80 e3 0f             	and    $0xf,%bl
    861a:	d2 e4                	shl    %cl,%ah
    861c:	36 0a 21             	or     %ss:(%bx,%di),%ah
    861f:	87 fd                	xchg   %di,%bp
    8621:	8a c4                	mov    %ah,%al
    8623:	aa                   	stos   %al,%es:(%di)
    8624:	4a                   	dec    %dx
    8625:	75 e1                	jne    0x8608
    8627:	5d                   	pop    %bp
    8628:	2b 3e b0 5f          	sub    0x5fb0,%di
    862c:	5e                   	pop    %si
    862d:	1f                   	pop    %ds
    862e:	ba 1e 53             	mov    $0x531e,%dx
    8631:	08 fc                	or     %bh,%ah
    8633:	b4 48                	mov    $0x48,%ah
    8635:	bb ff ff             	mov    $0xffff,%bx
    8638:	cd 21                	int    $0x21
    863a:	8b c3                	mov    %bx,%ax
    863c:	52                   	push   %dx
    863d:	73 1a                	jae    0x8659
    863f:	7e 8b                	jle    0x85cc
    8641:	56                   	push   %si
    8642:	51                   	push   %cx
    8643:	57                   	push   %di
    8644:	06                   	push   %es
    8645:	b4 48                	mov    $0x48,%ah
    8647:	0e                   	push   %cs
    8648:	e8 96 cd             	call   0x53e1
    864b:	21 72 1b             	and    %si,0x1b(%bp,%si)
    864e:	e7 8b                	out    %ax,$0x8b
    8650:	76 77                	jbe    0x86c9
    8652:	04 d1                	add    $0xd1,%al
    8654:	e1 d1                	loope  0x8627
    8656:	e1 d1                	loope  0x8629
    8658:	e1 33                	loope  0x868d
    865a:	0f 75 c6             	pcmpeqw %mm6,%mm0
    865d:	f3 ab                	rep stos %ax,%es:(%di)
    865f:	8c c0                	mov    %es,%ax
    8661:	07                   	pop    %es
    8662:	5f                   	pop    %di
    8663:	8b ba 1e 53          	mov    0x531e(%bp,%si),%di
    8667:	02 00                	add    (%bx,%si),%al
    8669:	b4 ff                	mov    $0xff,%ah
    866b:	07                   	pop    %es
    866c:	8b 1e 02 b4          	mov    -0x4bfe,%bx
    8670:	07                   	pop    %es
    8671:	1e                   	push   %ds
    8672:	b4 1a                	mov    $0x1a,%ah
    8674:	7e 8b                	jle    0x8601
    8676:	56                   	push   %si
    8677:	06                   	push   %es
    8678:	b4 4a                	mov    $0x4a,%ah
    867a:	06                   	push   %es
    867b:	b8 00 8e             	mov    $0x8e00,%ax
    867e:	2c 04                	sub    $0x4,%al
    8680:	cd 21                	int    $0x21
    8682:	73 02                	jae    0x8686
    8684:	b4 ff                	mov    $0xff,%ah
    8686:	07                   	pop    %es
    8687:	04 05                	add    $0x5,%al
    8689:	ff                   	(bad)
    868a:	7e 00                	jle    0x868c
    868c:	1a 7e 8b             	sbb    -0x75(%bp),%bh
    868f:	06                   	push   %es
    8690:	b4 49                	mov    $0x49,%ah
    8692:	8e 04                	mov    (%si),%es
    8694:	21 02                	and    %ax,(%bp,%si)
    8696:	ff 04                	incw   (%si)
    8698:	ff 00                	incw   (%bx,%si)
    869a:	7e 06                	jle    0x86a2
    869c:	b4 1e                	mov    $0x1e,%ah
    869e:	1a 8b 06 4a          	sbb    0x4a06(%bp,%di),%cl
    86a2:	b8 0e e8             	mov    $0xe80e,%ax
    86a5:	96                   	xchg   %ax,%si
    86a6:	ff 77 0c             	push   0xc(%bx)
    86a9:	e8 e0 ff             	call   0x868c
    86ac:	81 a6 9e 87 b0 32    	andw   $0x32b0,-0x7862(%bp)
    86b2:	b9 e8 b8             	mov    $0xb8e8,%cx
    86b5:	5c                   	pop    %sp
    86b6:	28 05                	sub    %al,(%di)
    86b8:	1e                   	push   %ds
    86b9:	8b 76 c0             	mov    -0x40(%bp),%si
    86bc:	53                   	push   %bx
    86bd:	08 fc                	or     %bh,%ah
    86bf:	0e                   	push   %cs
    86c0:	e8 06 8b             	call   0x11c9
    86c3:	4f                   	dec    %di
    86c4:	02 8b 07 26          	add    0x2607(%bp,%di),%cl
    86c8:	5b                   	pop    %bx
    86c9:	9e                   	sahf
    86ca:	05 5e 08             	add    $0x85e,%ax
    86cd:	8b 47 b4             	mov    -0x4c(%bx),%ax
    86d0:	06                   	push   %es
    86d1:	8b 0a                	mov    (%bp,%si),%cx
    86d3:	46                   	inc    %si
    86d4:	97                   	xchg   %ax,%di
    86d5:	46                   	inc    %si
    86d6:	a1 1f 03             	mov    0x31f,%ax
    86d9:	d9 03                	flds   (%bp,%di)
    86db:	d9 03                	flds   (%bp,%di)
    86dd:	37                   	aaa
    86de:	8b d3                	mov    %bx,%dx
    86e0:	0e                   	push   %cs
    86e1:	e8 96 8b             	call   0x127a
    86e4:	47                   	inc    %di
    86e5:	02 e7                	add    %bh,%ah
    86e7:	8b 76 3f             	mov    0x3f(%bp),%si
    86ea:	87 e0                	xchg   %sp,%ax
    86ec:	9a 1c 2a 03 38       	lcall  $0x3803,$0x2a1c
    86f1:	ff e0                	jmp    *%ax
    86f3:	7e 05                	jle    0x86fa
    86f5:	5e                   	pop    %si
    86f6:	08 8b 4f 04          	or     %cl,0x44f(%bp,%di)
    86fa:	8e d9                	mov    %cx,%ds
    86fc:	9a 74 8b 03 ad       	lcall  $0xad03,$0x8b74
    8701:	01 ef                	add    %bp,%di
    8703:	b1 e0                	mov    $0xe0,%cl
    8705:	47                   	inc    %di
    8706:	56                   	push   %si
    8707:	fc                   	cld
    8708:	d3 fa                	sar    %cl,%dx
    870a:	03 f2                	add    %dx,%si
    870c:	0e                   	push   %cs
    870d:	e8 96 36             	call   0xbda6
    8710:	8b 04                	mov    (%si),%ax
    8712:	07                   	pop    %es
    8713:	8b 5e ce             	mov    -0x32(%bp),%bx
    8716:	d3 fa                	sar    %cl,%dx
    8718:	2b d0                	sub    %ax,%dx
    871a:	36 8b 5f 47          	mov    %ss:0x47(%bx),%bx
    871e:	c8 e8 f3 a4          	enter  $0xf3e8,$0xa4
    8722:	03 f2                	add    %dx,%si
    8724:	4b                   	dec    %bx
    8725:	75 f7                	jne    0x871e
    8727:	9a 74 8b 03 ad       	lcall  $0xad03,$0x8b74
    872c:	56                   	push   %si
    872d:	8b c0                	mov    %ax,%ax
    872f:	24 eb                	and    $0xeb,%al
    8731:	45                   	inc    %bp
    8732:	08 4f 8e             	or     %cl,-0x72(%bx)
    8735:	9a 8b ad 03 76       	lcall  $0x7603,$0xad8b
    873a:	9e                   	sahf
    873b:	05 5e 96             	add    $0x965e,%ax
    873e:	8b 07                	mov    (%bx),%ax
    8740:	5e                   	pop    %si
    8741:	d3 2b                	shrw   %cl,(%bp,%di)
    8743:	36 5f                	ss pop %di
    8745:	2b 36 5f c8          	sub    -0x37a1,%si
    8749:	f3 03 4b f7          	repz add -0x9(%bp,%di),%cx
    874d:	74 03                	je     0x8752
    874f:	56                   	push   %si
    8750:	c0 eb 08             	shr    $0x8,%bl
    8753:	8e 2b                	mov    (%bp,%di),%gs
    8755:	b0 5e                	mov    $0x5e,%al
    8757:	ba 53 07             	mov    $0x753,%dx
    875a:	1f                   	pop    %ds
    875b:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    875c:	b1 8b                	mov    $0x8b,%cl
    875e:	76 db                	jbe    0x873b
    8760:	56                   	push   %si
    8761:	05 5e 08             	add    $0x85e,%ax
    8764:	8b 47 9a             	mov    -0x66(%bx),%ax
    8767:	8b d8                	mov    %ax,%bx
    8769:	05 96 07             	add    $0x796,%ax
    876c:	d3 36 2b 5f          	shlw   %cl,0x5f2b
    8770:	d3 36 4f 03          	shlw   %cl,0x34f
    8774:	4b                   	dec    %bx
    8775:	da 47 56             	fiaddl 0x56(%bx)
    8778:	fc                   	cld
    8779:	d1 fa                	sar    $1,%dx
    877b:	b9 e8 e9             	mov    $0xe9e8,%cx
    877e:	d3 36 37 03          	shlw   %cl,0x337
    8782:	f2 0e                	repnz push %cs
    8784:	f7 74 03             	divw   0x3(%si)
    8787:	83 c3 02             	add    $0x2,%bx
    878a:	4d                   	dec    %bp
    878b:	75 f1                	jne    0x877e
    878d:	5d                   	pop    %bp
    878e:	eb c4                	jmp    0x8754
    8790:	54                   	push   %sp
    8791:	5c                   	pop    %sp
    8792:	02 02                	add    (%bp,%si),%al
    8794:	08 4a 1e             	or     %cl,0x1e(%bp,%si)
    8797:	57                   	push   %di
    8798:	08 0e 96 47          	or     %cl,0x4796
    879c:	8a 06 06 4f          	mov    0x4f06,%al
    87a0:	8b 26 9e 5e          	mov    0x5e9e,%sp
    87a4:	8b b4 f8 05          	mov    0x5f8(%si),%si
    87a8:	08 47 8b             	or     %al,-0x75(%bx)
    87ab:	05 07 36             	add    $0x3607,%ax
    87ae:	8b 76 7f             	mov    0x7f(%bp),%si
    87b1:	26 9e                	es sahf
    87b3:	47                   	inc    %di
    87b4:	b4 06                	mov    $0x6,%ah
    87b6:	8b 9e 05 77          	mov    0x7705(%bp),%bx
    87ba:	08 2b                	or     %ch,(%bp,%di)
    87bc:	f1                   	int1
    87bd:	a1 03 03             	mov    0x303,%ax
    87c0:	03 8b 0e 8b          	add    -0x74f2(%bp,%di),%cx
    87c4:	d3 03                	rolw   %cl,(%bp,%di)
    87c6:	3f                   	aas
    87c7:	e0 1c                	loopne 0x87e5
    87c9:	03 ff                	add    %di,%di
    87cb:	7e 5e                	jle    0x882b
    87cd:	40                   	inc    %ax
    87ce:	ff                   	(bad)
    87cf:	7e e8                	jle    0x87b9
    87d1:	96                   	xchg   %ax,%si
    87d2:	8b 8e 9a 03          	mov    0x39a(%bp),%cx
    87d6:	4e                   	dec    %si
    87d7:	f8                   	clc
    87d8:	2b 4e fc             	sub    -0x4(%bp),%cx
    87db:	7d 02                	jge    0x87df
    87dd:	33 c9                	xor    %cx,%cx
    87df:	89 4e 8b             	mov    %cx,-0x75(%bp)
    87e2:	01 4f 03             	add    %cx,0x3(%bx)
    87e5:	3b f1                	cmp    %cx,%si
    87e7:	7d 8b                	jge    0x8774
    87e9:	89 ce                	mov    %cx,%si
    87eb:	0b c9                	or     %cx,%cx
    87ed:	7e 76                	jle    0x8865
    87ef:	89 4e fa             	mov    %cx,-0x6(%bp)
    87f2:	36 37                	ss aaa
    87f4:	4f                   	dec    %di
    87f5:	8b 26 8b ad          	mov    -0x5275,%sp
    87f9:	ff e0                	jmp    *%ax
    87fb:	9a 8b ad ef e0       	lcall  $0xe0ef,$0xad8b
    8800:	56                   	push   %si
    8801:	d3 03                	rolw   %cl,(%bp,%di)
    8803:	0e                   	push   %cs
    8804:	96                   	xchg   %ax,%si
    8805:	8b f8                	mov    %ax,%di
    8807:	d3 fa                	sar    %cl,%dx
    8809:	03 fe                	add    %si,%di
    880b:	a3 26 fe             	mov    %ax,0xfe26
    880e:	05 00 d3             	add    $0xd300,%ax
    8811:	d3 f8                	sar    %cl,%ax
    8813:	5b                   	pop    %bx
    8814:	eb fe                	jmp    0x8814
    8816:	2e a2 31 88          	mov    %al,%cs:0x8831
    881a:	d3 36 2b 5f          	shlw   %cl,0x5f2b
    881e:	2b 8b 46 46          	sub    0x4646(%bp,%di),%cx
    8822:	56                   	push   %si
    8823:	d3 03                	rolw   %cl,(%bp,%di)
    8825:	0e                   	push   %cs
    8826:	2b 36 7e e8          	sub    -0x1782,%si
    882a:	fe                   	(bad)
    882b:	a3 f7 74             	mov    %ax,0x74f7
    882e:	03 83 c6 00          	add    0xc6(%bp,%di),%ax
    8832:	03 fe                	add    %si,%di
    8834:	eb 08                	jmp    0x883e
    8836:	f4                   	hlt
    8837:	9a 8b ad 8b 24       	lcall  $0x248b,$0xad8b
    883c:	45                   	inc    %bp
    883d:	4f                   	dec    %di
    883e:	9a 25 03 7e 5e       	lcall  $0x5e7e,$0x325
    8843:	2d d1 fe             	sub    $0xfed1,%ax
    8846:	2e a2 5f 88          	mov    %al,%cs:0x885f
    884a:	36 5f                	ss pop %di
    884c:	8b 46 d3             	mov    -0x2d(%bp),%ax
    884f:	0e                   	push   %cs
    8850:	36 e8 a3 74          	ss call 0xfcf7
    8854:	2b 7e fe             	sub    -0x2(%bp),%di
    8857:	f7 03 c6 03          	testw  $0x3c6,(%bp,%di)
    885b:	eb f4                	jmp    0x8851
    885d:	8b 8b 45 9a          	mov    -0x65bb(%bp,%di),%cx
    8861:	03 5e d1             	add    -0x2f(%bp),%bx
    8864:	2e 2b b0 5e ba       	sub    %cs:-0x45a2(%bx,%si),%si
    8869:	53                   	push   %bx
    886a:	fc                   	cld
    886b:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    886c:	8b db                	mov    %bx,%bx
    886e:	05 08 47             	add    $0x4708,%ax
    8871:	8b 0e 36 40          	mov    0x4036,%cx
    8875:	f8                   	clc
    8876:	d5 5b                	aad    $0x5b
    8878:	eb fe                	jmp    0x8878
    887a:	2e a2 9d 88          	mov    %al,%cs:0x889d
    887e:	5f                   	pop    %di
    887f:	46                   	inc    %si
    8880:	0e                   	push   %cs
    8881:	e8 74 7e             	call   0x6f8
    8884:	f7 c6 4e fa          	test   $0xfa4e,%si
    8888:	36 da 56 d1          	ficoml %ss:-0x2f(%bp)
    888c:	f8                   	clc
    888d:	d1 b9 e9 36          	sarw   $1,0x36e9(%bx,%di)
    8891:	03 0e 74 3f          	add    0x3f74,%cx
    8895:	03 fe                	add    %si,%di
    8897:	a3 03 f4             	mov    %ax,0xf403
    889a:	8b 9a 5e 2e          	mov    0x2e5e(%bp,%si),%bx
    889e:	83 02 75             	addw   $0x75,(%bp,%si)
    88a1:	5d                   	pop    %bp
    88a2:	c4                   	(bad)
    88a3:	ee                   	out    %al,(%dx)
    88a4:	5d                   	pop    %bp
    88a5:	eb be                	jmp    0x8865
    88a7:	54                   	push   %sp
    88a8:	02 08                	add    (%bx,%si),%cl
    88aa:	1e                   	push   %ds
    88ab:	08 11                	or     %dl,(%bx,%di)
    88ad:	53                   	push   %bx
    88ae:	7e 08                	jle    0x88b8
    88b0:	00 cd                	add    %cl,%ch
    88b2:	06                   	push   %es
    88b3:	1e                   	push   %ds
    88b4:	8b 76 04             	mov    0x4(%bp),%si
    88b7:	cd b8                	int    $0xb8
    88b9:	8b 69 02             	mov    0x2(%bx,%di),%bp
    88bc:	45                   	inc    %bp
    88bd:	02 36 76 26          	add    0x2676,%dh
    88c1:	96                   	xchg   %ax,%si
    88c2:	8b f8                	mov    %ax,%di
    88c4:	08 8b 06 34          	or     %cl,0x3406(%bp,%di)
    88c8:	05 46 59             	add    $0x5946,%ax
    88cb:	04 06                	add    $0x6,%al
    88cd:	05 0e 8b             	add    $0x8b0e,%ax
    88d0:	d1 e0                	shl    $1,%ax
    88d2:	89 44 04             	mov    %ax,0x4(%si)
    88d5:	8a cd                	mov    %ch,%cl
    88d7:	06                   	push   %es
    88d8:	8a 77 07             	mov    0x7(%bx),%dh
    88db:	b1 96                	mov    $0x96,%cl
    88dd:	47                   	inc    %di
    88de:	8a 09                	mov    (%bx,%di),%cl
    88e0:	e4 10                	in     $0x10,%al
    88e2:	d3 e0                	shl    %cl,%ax
    88e4:	05 c0 60             	add    $0x60c0,%ax
    88e7:	8b f0                	mov    %ax,%si
    88e9:	b9 10 00             	mov    $0x10,%cx
    88ec:	8d 5e ef             	lea    -0x11(%bp),%bx
    88ef:	ac                   	lods   %ds:(%si),%al
    88f0:	88 07                	mov    %al,(%bx)
    88f2:	43                   	inc    %bx
    88f3:	e2 fe                	loop   0x88f3
    88f5:	a3 06 4f             	mov    %ax,0x4f06
    88f8:	26 47                	es inc %di
    88fa:	0c 8b                	or     $0x8b,%al
    88fc:	5f                   	pop    %di
    88fd:	0a 8e d8 e4          	or     -0x1b28(%bp),%cl
    8901:	10 b1 96 47          	adc    %dh,0x4796(%bx,%di)
    8905:	ea 8a 07 e8 f8       	ljmp   $0xf8e8,$0x78a
    890a:	8b f0                	mov    %ax,%si
    890c:	8a 42 ef             	mov    -0x11(%bp,%si),%al
    890f:	aa                   	stos   %al,%es:(%di)
    8910:	8a 07                	mov    (%bx),%al
    8912:	43                   	inc    %bx
    8913:	25 00 46             	and    $0x4600,%ax
    8916:	8b 8a ef 8a          	mov    -0x7511(%bp,%si),%cx
    891a:	43                   	inc    %bx
    891b:	00 fe                	add    %bh,%dh
    891d:	cd 75                	int    $0x75
    891f:	e6 fe                	out    %al,$0xfe
    8921:	ce                   	into
    8922:	75 e0                	jne    0x8904
    8924:	1f                   	pop    %ds
    8925:	07                   	pop    %es
    8926:	5f                   	pop    %di
    8927:	ec                   	in     (%dx),%al
    8928:	52                   	push   %dx
    8929:	73 1a                	jae    0x8945
    892b:	f6 f9                	idiv   %cl
    892d:	76 e8                	jbe    0x8917
    892f:	ff c9                	dec    %cx
    8931:	52                   	push   %dx
    8932:	8b e8                	mov    %ax,%bp
    8934:	00 f0                	add    %dh,%al
    8936:	04 53                	add    $0x53,%al
    8938:	08 cd                	or     %cl,%ch
    893a:	1e                   	push   %ds
    893b:	76 cd                	jbe    0x890a
    893d:	8b 02                	mov    (%bp,%si),%ax
    893f:	02 76 96             	add    -0x6a(%bp),%dh
    8942:	f8                   	clc
    8943:	8b 4a 02             	mov    0x2(%bp,%si),%cx
    8946:	02 76 96             	add    -0x6a(%bp),%dh
    8949:	f8                   	clc
    894a:	8b 3c                	mov    (%si),%di
    894c:	7e 9e                	jle    0x88ec
    894e:	1a 08                	sbb    (%bx,%si),%cl
    8950:	06                   	push   %es
    8951:	05 59 06             	add    $0x659,%ax
    8954:	0e                   	push   %cs
    8955:	39 04                	cmp    %ax,(%si)
    8957:	06                   	push   %es
    8958:	05 0e 8b             	add    $0x8b0e,%ax
    895b:	05 44 04             	add    $0x444,%ax
    895e:	8a 4f 07             	mov    0x7(%bx),%cl
    8961:	8b 77 0a             	mov    0xa(%bx),%si
    8964:	f6 e1                	mul    %cl
    8966:	47                   	inc    %di
    8967:	75 51                	jne    0x89ba
    8969:	8b 0c                	mov    (%si),%cx
    896b:	0e                   	push   %cs
    896c:	32 d8                	xor    %al,%bl
    896e:	b3 01                	mov    $0x1,%bl
    8970:	ba c4 03             	mov    $0x3c4,%dx
    8973:	8a e3                	mov    %bl,%ah
    8975:	b0 9a                	mov    $0x9a,%al
    8977:	75 83                	jne    0x88fc
    8979:	4e                   	dec    %si
    897a:	8b eb                	mov    %bx,%bp
    897c:	9e                   	sahf
    897d:	1a f4                	sbb    %ah,%dh
    897f:	8b d0                	mov    %ax,%dx
    8981:	e3 f6                	jcxz   0x8979
    8983:	c3                   	ret
    8984:	10 74 e9             	adc    %dh,-0x17(%si)
    8987:	ba c4 03             	mov    $0x3c4,%dx
    898a:	b4 0f                	mov    $0xf,%ah
    898c:	b0 9a                	mov    $0x9a,%al
    898e:	75 1f                	jne    0x89af
    8990:	5f                   	pop    %di
    8991:	52                   	push   %dx
    8992:	1a f9                	sbb    %cl,%bh
    8994:	e8 c9 8b             	call   0x1560
    8997:	00 04                	add    %al,(%si)
    8999:	08 1e cd 02          	or     %bl,0x2cd
    899d:	76 f8                	jbe    0x8997
    899f:	4a                   	dec    %dx
    89a0:	02 0e 57 56          	add    0x5657,%cl
    89a4:	e8 01 76             	call   0xffa8
    89a7:	02 06 5d 9b          	add    -0x64a3,%al
    89ab:	88 46 f7             	mov    %al,-0x9(%bp)
    89ae:	8b 06 e8 47          	mov    0x47e8,%ax
    89b2:	75 f4                	jne    0x89a8
    89b4:	b8 04 2c             	mov    $0x2c04,%ax
    89b7:	eb f4                	jmp    0x89ad
    89b9:	e8 f6 06             	call   0x90b2
    89bc:	6a 9b                	push   $0xff9b
    89be:	80 74 08 05          	xorb   $0x5,0x8(%si)
    89c2:	47                   	inc    %di
    89c3:	05 80 e6             	add    $0xe680,%ax
    89c6:	01 75 02             	add    %si,0x2(%di)
    89c9:	eb 7a                	jmp    0x8a45
    89cb:	a1 fc 8b             	mov    0x8bfc,%ax
    89ce:	8b f2                	mov    %dx,%si
    89d0:	47                   	inc    %di
    89d1:	b1 96                	mov    $0x96,%cl
    89d3:	a0 69 9b             	mov    0x9b69,%al
    89d6:	25 f0 00             	and    $0xf0,%ax
    89d9:	e8 f8 fd             	call   0x87d4
    89dc:	57                   	push   %di
    89dd:	7e 8b                	jle    0x896a
    89df:	c3                   	ret
    89e0:	06                   	push   %es
    89e1:	0a 97 a1 d8          	or     -0x275f(%bx),%dl
    89e5:	80 e6 0a             	and    $0xa,%dh
    89e8:	75 03                	jne    0x89ed
    89ea:	ff                   	(bad)
    89eb:	ff 33                	push   (%bp,%di)
    89ed:	0e                   	push   %cs
    89ee:	fe 0f                	decb   (%bx)
    89f0:	b8 65 02             	mov    $0x265,%ax
    89f3:	47                   	inc    %di
    89f4:	75 51                	jne    0x8a47
    89f6:	8b 1e 58 9b          	mov    -0x64a8,%bx
    89fa:	8a 04                	mov    (%si),%al
    89fc:	03 46 56             	add    0x56(%bp),%ax
    89ff:	f4                   	hlt
    8a00:	e8 48 ec             	call   0x764b
    8a03:	fa                   	cli
    8a04:	07                   	pop    %es
    8a05:	07                   	pop    %es
    8a06:	75 f4                	jne    0x89fc
    8a08:	48                   	dec    %ax
    8a09:	b8 77 52             	mov    $0x5277,%ax
    8a0c:	fa                   	cli
    8a0d:	48                   	dec    %ax
    8a0e:	b8 77 52             	mov    $0x5277,%ax
    8a11:	fc                   	cld
    8a12:	03 40 f0             	add    -0x10(%bx,%si),%ax
    8a15:	fd                   	std
    8a16:	c6                   	(bad)
    8a17:	57                   	push   %di
    8a18:	f7 2b                	imulw  (%bp,%di)
    8a1a:	01 4a 2c             	add    %cx,0x2c(%bp,%si)
    8a1d:	eb f4                	jmp    0x8a13
    8a1f:	e8 e8 48             	call   0xd30a
    8a22:	50                   	push   %ax
    8a23:	e8 4e f4             	call   0x7e74
    8a26:	51                   	push   %cx
    8a27:	dc 74 de             	fdivl  -0x22(%si)
    8a2a:	0e                   	push   %cs
    8a2b:	fe 0f                	decb   (%bx)
    8a2d:	b8 c8 15             	mov    $0x15c8,%ax
    8a30:	5b                   	pop    %bx
    8a31:	03 37                	add    (%bx),%si
    8a33:	58                   	pop    %ax
    8a34:	0e                   	push   %cs
    8a35:	4f                   	dec    %di
    8a36:	b8 77 aa             	mov    $0xaa77,%ax
    8a39:	b8 04 2c             	mov    $0x2c04,%ax
    8a3c:	eb f4                	jmp    0x8a32
    8a3e:	e8 03 46             	call   0xd044
    8a41:	f2 fd                	repnz std
    8a43:	57                   	push   %di
    8a44:	7e 8b                	jle    0x89d1
    8a46:	76 f4                	jbe    0x8a3c
    8a48:	eb 3d                	jmp    0x8a87
    8a4a:	03 44 75             	add    0x75(%si),%ax
    8a4d:	0a 75 0d             	or     0xd(%di),%dh
    8a50:	8b 76 06             	mov    0x6(%bp),%si
    8a53:	c6                   	(bad)
    8a54:	f7 01 2c f4          	testw  $0xf42c,(%bx,%di)
    8a58:	e8 50 4e             	call   0xd8ab
    8a5b:	eb 2a                	jmp    0x8a87
    8a5d:	03 44 75             	add    0x75(%si),%ax
    8a60:	20 7c 24             	and    %bh,0x24(%si)
    8a63:	b0 fe                	mov    $0xfe,%al
    8a65:	ff 50 0e             	call   *0xe(%bx,%si)
    8a68:	04 8b                	add    $0x8b,%al
    8a6a:	f8                   	clc
    8a6b:	e8 e8 02             	call   0x8d56
    8a6e:	b0 fe                	mov    $0xfe,%al
    8a70:	ff 98 8b 08          	lcall  *0x88b(%bx,%si)
    8a74:	06                   	push   %es
    8a75:	1e                   	push   %ds
    8a76:	56                   	push   %si
    8a77:	9b                   	fwait
    8a78:	8a 41 e0             	mov    -0x20(%bx,%di),%al
    8a7b:	2c f4                	sub    $0xf4,%al
    8a7d:	8a 0e 5e 9b          	mov    -0x64a2,%cl
    8a81:	2a ed                	sub    %ch,%ch
    8a83:	89 03                	mov    %ax,(%bp,%di)
    8a85:	70 f0                	jo     0x8a77
    8a87:	05 47 05             	add    $0x547,%ax
    8a8a:	ff                   	(bad)
    8a8b:	ff 08                	decw   (%bx,%si)
    8a8d:	8a 1b                	mov    (%bp,%di),%bl
    8a8f:	43                   	inc    %bx
    8a90:	de 75 0a             	fidivs 0xa(%di)
    8a93:	b8 77 b4             	mov    $0xb477,%ax
    8a96:	fe 4c 4b             	decb   0x4b(%si)
    8a99:	c0 19 04             	rcrb   $0x4,(%bx,%di)
    8a9c:	8b 76 db             	mov    -0x25(%bp),%si
    8a9f:	56                   	push   %si
    8aa0:	53                   	push   %bx
    8aa1:	00 22                	add    %ah,(%bp,%si)
    8aa3:	e6 51                	out    %al,$0x51
    8aa5:	dc 74 de             	fdivl  -0x22(%si)
    8aa8:	06                   	push   %es
    8aa9:	33 00                	xor    (%bx,%si),%ax
    8aab:	30 04                	xor    %al,(%si)
    8aad:	8b 0a                	mov    (%bp,%si),%cx
    8aaf:	6c                   	insb   (%dx),%es:(%di)
    8ab0:	e8 08 e8             	call   0x72bb
    8ab3:	6d                   	insw   (%dx),%es:(%di)
    8ab4:	15 c4 47             	adc    $0x47c4,%ax
    8ab7:	2e ff 56 83          	call   *%cs:-0x7d(%bp)
    8abb:	8b ff                	mov    %di,%di
    8abd:	5b                   	pop    %bx
    8abe:	fe 06 da 5e          	incb   0x5eda
    8ac2:	ca 52 a2             	lret   $0xa252
    8ac5:	63 08                	arpl   %cx,(%bx,%si)
    8ac7:	2b 0a                	sub    (%bp,%si),%cx
    8ac9:	52                   	push   %dx
    8aca:	b8 00 e6             	mov    $0xe600,%ax
    8acd:	dc de                	(bad)
    8acf:	33 30                	xor    (%bx,%si),%si
    8ad1:	8b 6c 08             	mov    0x8(%si),%bp
    8ad4:	6d                   	insw   (%dx),%es:(%di)
    8ad5:	c4 2e 56 8b          	les    -0x74aa,%bp
    8ad9:	5b                   	pop    %bx
    8ada:	06                   	push   %es
    8adb:	5e                   	pop    %si
    8adc:	52                   	push   %dx
    8add:	43                   	inc    %bx
    8ade:	15 47 ff             	adc    $0xff47,%ax
    8ae1:	83 ff fe             	cmp    $0xfffe,%di
    8ae4:	da ca                	fcmove %st(2),%st
    8ae6:	a2 08 0a             	mov    %al,0xa08
    8ae9:	b8 4e 01             	mov    $0x14e,%ax
    8aec:	ca a2 08             	lret   $0x8a2
    8aef:	0a b8 e6 de          	or     -0x211a(%bx,%si),%bh
    8af3:	30 6c 5b             	xor    %ch,0x5b(%si)
    8af6:	06                   	push   %es
    8af7:	5e                   	pop    %si
    8af8:	8b 58 8a             	mov    -0x76(%bx,%si),%bx
    8afb:	03 56 e8             	add    -0x18(%bp),%dx
    8afe:	ec                   	in     (%dx),%al
    8aff:	07                   	pop    %es
    8b00:	75 48                	jne    0x8b4a
    8b02:	06                   	push   %es
    8b03:	b8 77 4e             	mov    $0x4e77,%ax
    8b06:	9a 4e 2b 08 51       	lcall  $0x5108,$0x2b4e
    8b0b:	8a 0e 5c 9b          	mov    -0x64a4,%cl
    8b0f:	2a ed                	sub    %ch,%ch
    8b11:	89 03                	mov    %ax,(%bp,%di)
    8b13:	50                   	push   %ax
    8b14:	e8 5d f3             	call   0x7e74
    8b17:	5b                   	pop    %bx
    8b18:	06                   	push   %es
    8b19:	0c 2e                	or     $0x2e,%al
    8b1b:	8b 06 52 15          	mov    0x1552,%ax
    8b1f:	ff 0f                	decw   (%bx)
    8b21:	e8 b1 5b             	call   0xe6d5
    8b24:	78 ff                	js     0x8b25
    8b26:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    8b27:	87 32                	xchg   %si,(%bp,%si)
    8b29:	e8 33 30             	call   0xbb5f
    8b2c:	06                   	push   %es
    8b2d:	b8 e8 ff             	mov    $0xffe8,%ax
    8b30:	ff 08                	decw   (%bx,%si)
    8b32:	2b 7a da             	sub    -0x26(%bp,%si),%di
    8b35:	a9 8b 8a             	test   $0x8a8b,%ax
    8b38:	56                   	push   %si
    8b39:	ec                   	in     (%dx),%al
    8b3a:	75 06                	jne    0x8b42
    8b3c:	77 9a                	ja     0x8ad8
    8b3e:	2b 51 0e             	sub    0xe(%bx,%di),%dx
    8b41:	9b                   	fwait
    8b42:	52                   	push   %dx
    8b43:	15 ff 8a             	adc    $0x8aff,%ax
    8b46:	5c                   	pop    %sp
    8b47:	2a 89 50 5d          	sub    0x5d50(%bx,%di),%cl
    8b4b:	5b                   	pop    %bx
    8b4c:	0c 8b                	or     $0x8b,%al
    8b4e:	52                   	push   %dx
    8b4f:	23 f3                	and    %bx,%si
    8b51:	06                   	push   %es
    8b52:	2e 06                	cs push %es
    8b54:	15 0f b1             	adc    $0xb10f,%ax
    8b57:	78 a6                	js     0x8aff
    8b59:	32 33                	xor    (%bp,%di),%dh
    8b5b:	06                   	push   %es
    8b5c:	e8 ff 68             	call   0xf45e
    8b5f:	ff 87 e8 30          	incw   0x30e8(%bx)
    8b63:	b8 ff 08             	mov    $0x8ff,%ax
    8b66:	7a a9                	jp     0x8b11
    8b68:	8a 5b 5e             	mov    0x5e(%bp,%di),%bl
    8b6b:	58                   	pop    %ax
    8b6c:	03 e8                	add    %ax,%bp
    8b6e:	07                   	pop    %es
    8b6f:	48                   	dec    %ax
    8b70:	b8 4e 4e             	mov    $0x4e4e,%ax
    8b73:	08 8a 5c 2a          	or     %cl,0x2a5c(%bp,%si)
    8b77:	89 50 5d             	mov    %dx,0x5d(%bx,%si)
    8b7a:	5b                   	pop    %bx
    8b7b:	0c 8b                	or     $0x8b,%al
    8b7d:	52                   	push   %dx
    8b7e:	ff                   	ljmp   (bad)
    8b7f:	e8 5b ff             	call   0x8add
    8b82:	87 e8                	xchg   %bp,%ax
    8b84:	30 b8 ff 08          	xor    %bh,0x8ff(%bx,%si)
    8b88:	7a e9                	jp     0x8b73
    8b8a:	f2 15 b1 a6          	repnz adc $0xa6b1,%ax
    8b8e:	33 e8                	xor    %ax,%bp
    8b90:	68 87 30             	push   $0x3087
    8b93:	ff                   	(bad)
    8b94:	7a 05                	jp     0x8b9b
    8b96:	5e                   	pop    %si
    8b97:	52                   	push   %dx
    8b98:	63 2b                	arpl   %bp,(%bp,%di)
    8b9a:	52                   	push   %dx
    8b9b:	00 dc                	add    %bl,%ah
    8b9d:	33 8b 08 56          	xor    0x5608(%bp,%di),%cx
    8ba1:	03 e8                	add    %ax,%bp
    8ba3:	07                   	pop    %es
    8ba4:	48                   	dec    %ax
    8ba5:	8d 81 ff e8          	lea    -0x1701(%bx,%di),%ax
    8ba9:	b8 fe 03             	mov    $0x3fe,%ax
    8bac:	de 0f                	fimuls (%bx)
    8bae:	e8 b1 5e             	call   0xea62
    8bb1:	52                   	push   %dx
    8bb2:	a4                   	movsb  %ds:(%si),%es:(%di)
    8bb3:	00 8b c8 99          	add    %cl,-0x6638(%bp,%di)
    8bb7:	2b c2                	sub    %dx,%ax
    8bb9:	d1 f8                	sar    $1,%ax
    8bbb:	2b 7a 06             	sub    0x6(%bp,%si),%di
    8bbe:	f7 d8                	neg    %ax
    8bc0:	50                   	push   %ax
    8bc1:	56                   	push   %si
    8bc2:	0c 8b                	or     $0x8b,%al
    8bc4:	06                   	push   %es
    8bc5:	51                   	push   %cx
    8bc6:	e8 76 06             	call   0x923f
    8bc9:	4e                   	dec    %si
    8bca:	08 8a f0 56          	or     %cl,0x56f0(%bp,%si)
    8bce:	e8 a3 f2             	call   0x7e74
    8bd1:	15 b1 68             	adc    $0x68b1,%ax
    8bd4:	87 30                	xchg   %si,(%bx,%si)
    8bd6:	ff a2 08 0a          	jmp    *0xa08(%bp,%si)
    8bda:	b8 07 0c             	mov    $0xc07,%ax
    8bdd:	ef                   	out    %ax,(%dx)
    8bde:	3b d3                	cmp    %bx,%dx
    8be0:	08 8d 8b db          	or     %cl,-0x2475(%di)
    8be4:	53                   	push   %bx
    8be5:	22 51 57             	and    0x57(%bx,%di),%dl
    8be8:	8b c0                	mov    %ax,%ax
    8bea:	7e 08                	jle    0x8bf4
    8bec:	eb 40                	jmp    0x8c2e
    8bee:	de 30                	fidivs (%bx,%si)
    8bf0:	6c                   	insb   (%dx),%es:(%di)
    8bf1:	6d                   	insw   (%dx),%es:(%di)
    8bf2:	57                   	push   %di
    8bf3:	e8 02 14             	call   0x9ff8
    8bf6:	c7                   	(bad)
    8bf7:	8b 72 fa             	mov    -0x6(%bp,%si),%si
    8bfa:	19 0a                	sbb    %cx,(%bp,%si)
    8bfc:	74 03                	je     0x8c01
    8bfe:	c6 04 00             	movb   $0x0,(%si)
    8c01:	57                   	push   %di
    8c02:	57                   	push   %di
    8c03:	e8 52 00             	call   0x8c58
    8c06:	d1 e8                	shr    $1,%ax
    8c08:	2b 06 d8 56          	sub    0x56d8,%ax
    8c0c:	8b 51 a2             	mov    -0x5e(%bx,%di),%dx
    8c0f:	08 0a                	or     %cl,(%bp,%si)
    8c11:	b8 88 fd             	mov    $0xfd88,%ax
    8c14:	19 0a                	sbb    %cx,(%bp,%si)
    8c16:	74 04                	je     0x8c1c
    8c18:	c6 04 0a             	movb   $0xa,(%si)
    8c1b:	46                   	inc    %si
    8c1c:	e8 76 06             	call   0x9295
    8c1f:	4e                   	dec    %si
    8c20:	08 e8                	or     %ch,%al
    8c22:	5b                   	pop    %bx
    8c23:	5d                   	pop    %bp
    8c24:	87 e8                	xchg   %bp,%ax
    8c26:	30 b8 ff 01          	xor    %bh,0x1ff(%bx,%si)
    8c2a:	04 77                	add    $0x77,%al
    8c2c:	8b fe                	mov    %si,%di
    8c2e:	0b ff                	or     %di,%di
    8c30:	75 bc                	jne    0x8bee
    8c32:	fe 4b 19             	decb   0x19(%bp,%di)
    8c35:	8b db                	mov    %bx,%bx
    8c37:	53                   	push   %bx
    8c38:	22 51 74             	and    0x74(%bx,%di),%dl
    8c3b:	06                   	push   %es
    8c3c:	00 15                	add    %dl,(%di)
    8c3e:	b1 68                	mov    $0x68,%cl
    8c40:	87 30                	xchg   %si,(%bx,%si)
    8c42:	5e                   	pop    %si
    8c43:	52                   	push   %dx
    8c44:	12 00                	adc    (%bx,%si),%al
    8c46:	2b d8                	sub    %ax,%bx
    8c48:	8b a2 0a 88          	mov    -0x77f6(%bp,%si),%sp
    8c4c:	19 74 c6             	sbb    %si,-0x3a(%si)
    8c4f:	0a 4a fd             	or     -0x3(%bp,%si),%cl
    8c52:	3b 08                	cmp    (%bx,%si),%cx
    8c54:	8b 53 51             	mov    0x51(%bp,%di),%dx
    8c57:	8b 7e eb             	mov    -0x15(%bp),%di
    8c5a:	de 6c 57             	fisubrs 0x57(%si)
    8c5d:	02 74 c6             	add    -0x3a(%si),%dh
    8c60:	2b d2                	sub    %dx,%dx
    8c62:	19 0a                	sbb    %cx,(%bp,%si)
    8c64:	74 2f                	je     0x8c95
    8c66:	2a fe                	sub    %dh,%bh
    8c68:	00 74 2a             	add    %dh,0x2a(%si)
    8c6b:	2a fe                	sub    %dh,%bh
    8c6d:	00 74 25             	add    %dh,0x25(%si)
    8c70:	2a fe                	sub    %dh,%bh
    8c72:	0a 74 20             	or     0x20(%si),%dh
    8c75:	2a fe                	sub    %dh,%bh
    8c77:	20 7c 18             	and    %bh,0x18(%si)
    8c7a:	8a 04                	mov    (%si),%al
    8c7c:	98                   	cbtw
    8c7d:	08 1e 9b 41          	or     %bl,0x419b
    8c81:	2c 8a                	sub    $0x8a,%al
    8c83:	5e                   	pop    %si
    8c84:	2a 89 70 05          	sub    0x570(%bx,%di),%cl
    8c88:	05 ff 8a             	add    $0x8aff,%ax
    8c8b:	43                   	inc    %bx
    8c8c:	75 b8                	jne    0x8c46
    8c8e:	b4 4c                	mov    $0x4c,%ah
    8c90:	c0 d0 46             	rcl    $0x46,%al
    8c93:	eb d6                	jmp    0x8c6b
    8c95:	8b c2                	mov    %dx,%ax
    8c97:	46                   	inc    %si
    8c98:	81 a6 9e 87 b0 32    	andw   $0x32b0,-0x7862(%bp)
    8c9e:	b9 e8 b8             	mov    $0xb8e8,%cx
    8ca1:	5c                   	pop    %sp
    8ca2:	28 05                	sub    %al,(%di)
    8ca4:	10 57 e8             	adc    %dl,-0x18(%bx)
    8ca7:	76 06                	jbe    0x8caf
    8ca9:	9b                   	fwait
    8caa:	4e                   	dec    %si
    8cab:	e8 5d e8             	call   0x750b
    8cae:	b8 01 77             	mov    $0x7701,%ax
    8cb1:	fe                   	(bad)
    8cb2:	ff                   	(bad)
    8cb3:	bc eb e8             	mov    $0xe8eb,%sp
    8cb6:	46                   	inc    %si
    8cb7:	fd                   	std
    8cb8:	7e 0a                	jle    0x8cc4
    8cba:	46                   	inc    %si
    8cbb:	8b 07                	mov    (%bx),%ax
    8cbd:	75 48                	jne    0x8d07
    8cbf:	0c 47                	or     $0x47,%al
    8cc1:	51                   	push   %cx
    8cc2:	1e                   	push   %ds
    8cc3:	19 74 0e             	sbb    %si,0xe(%si)
    8cc6:	e8 63 13             	call   0xa02c
    8cc9:	2a b8 3f 05          	sub    0x53f(%bx,%si),%bh
    8ccd:	05 c0 7e             	add    $0x7ec0,%ax
    8cd0:	fc                   	cld
    8cd1:	eb 37                	jmp    0x8d0a
    8cd3:	75 48                	jne    0x8d1d
    8cd5:	0e                   	push   %cs
    8cd6:	06                   	push   %es
    8cd7:	97                   	xchg   %ax,%di
    8cd8:	d8 e6                	fsub   %st(6),%st
    8cda:	eb 8b                	jmp    0x8c67
    8cdc:	8b d8                	mov    %ax,%bx
    8cde:	e6 75                	out    %al,$0x75
    8ce0:	ff 33                	push   (%bp,%di)
    8ce2:	fe                   	(bad)
    8ce3:	b8 02 75             	mov    $0x7502,%ax
    8ce6:	8b 58 8a             	mov    -0x76(%bx,%si),%bx
    8ce9:	03 6c ff             	add    -0x1(%si),%bp
    8cec:	3b c7                	cmp    %di,%ax
    8cee:	7d 89                	jge    0x8c79
    8cf0:	0b c7                	or     %di,%ax
    8cf2:	08 1e b8 02          	or     %bl,0x2b8
    8cf6:	fa                   	cli
    8cf7:	51                   	push   %cx
    8cf8:	74 0e                	je     0x8d08
    8cfa:	0f c8                	bswap  %ax
    8cfc:	5b                   	pop    %bx
    8cfd:	37                   	aaa
    8cfe:	0e                   	push   %cs
    8cff:	f7 12                	notw   (%bp,%si)
    8d01:	5b                   	pop    %bx
    8d02:	37                   	aaa
    8d03:	0e                   	push   %cs
    8d04:	b8 aa 04             	mov    $0x4aa,%ax
    8d07:	eb e8                	jmp    0x8cf1
    8d09:	d2 0b                	rorb   %cl,(%bp,%di)
    8d0b:	c7                   	(bad)
    8d0c:	99                   	cwtd
    8d0d:	c2 f8 7a             	ret    $0x7af8
    8d10:	f7 8b c8 2b d8 08    	testw  $0x8d8,0x2bc8(%bp,%di)
    8d16:	a2 0a 0e             	mov    %al,0xe0a
    8d19:	b8 f0 0c             	mov    $0xcf0,%ax
    8d1c:	52                   	push   %dx
    8d1d:	e8 89 4e             	call   0xdba9
    8d20:	f4                   	hlt
    8d21:	04 eb                	add    $0xeb,%al
    8d23:	fa                   	cli
    8d24:	f7 6e f8             	imulw  -0x8(%bp)
    8d27:	99                   	cwtd
    8d28:	f8                   	clc
    8d29:	f7 c8 d8 a2          	test   $0xa2d8,%ax
    8d2d:	0e                   	push   %cs
    8d2e:	f0 52                	lock push %dx
    8d30:	8b a2 0a 8b          	mov    -0x74f6(%bp,%si),%sp
    8d34:	47                   	inc    %di
    8d35:	96                   	xchg   %ax,%si
    8d36:	0c 06                	or     $0x6,%al
    8d38:	e8 89 4e             	call   0xdbc4
    8d3b:	f6 7e 96             	idivb  -0x6a(%bp)
    8d3e:	8e 03                	mov    (%bp,%di),%es
    8d40:	fb                   	sti
    8d41:	8d 76 f0             	lea    -0x10(%bp),%si
    8d44:	16                   	push   %ss
    8d45:	40                   	inc    %ax
    8d46:	fa                   	cli
    8d47:	7e eb                	jle    0x8d34
    8d49:	e8 8b c3             	call   0x50d7
    8d4c:	8c d2                	mov    %ss,%dx
    8d4e:	46                   	inc    %si
    8d4f:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    8d50:	87 32                	xchg   %si,(%bp,%si)
    8d52:	e8 5c 0c             	call   0x99b1
    8d55:	4c                   	dec    %sp
    8d56:	53                   	push   %bx
    8d57:	7e 7f                	jle    0x8dd8
    8d59:	06                   	push   %es
    8d5a:	a1 c0 56             	mov    0x56c0,%ax
    8d5d:	4a                   	dec    %dx
    8d5e:	0b 36 5a 9b          	or     -0x64a6,%si
    8d62:	bf 48 9b             	mov    $0x9b48,%di
    8d65:	8c d8                	mov    %ds,%ax
    8d67:	8e c0                	mov    %ax,%es
    8d69:	8a d8                	mov    %al,%bl
    8d6b:	08 2c                	or     %ch,(%si)
    8d6d:	20 88 ff 8a          	and    %cl,-0x7501(%bx,%si)
    8d71:	43                   	inc    %bx
    8d72:	d8 5d e8             	fcomps -0x18(%di)
    8d75:	5f                   	pop    %di
    8d76:	9b                   	fwait
    8d77:	d2 e0                	shl    %cl,%al
    8d79:	8a 2e 06 9b          	mov    -0x64fa,%ch
    8d7d:	f6 e5                	mul    %ch
    8d7f:	03 f0                	add    %ax,%si
    8d81:	88 6d 07             	mov    %ch,0x7(%di)
    8d84:	d2 e5                	shl    %cl,%ch
    8d86:	32 ff                	xor    %bh,%bh
    8d88:	03 41 8a             	add    -0x76(%bx,%di),%ax
    8d8b:	2a 70 07             	sub    0x7(%bx,%si),%dh
    8d8e:	32 e4                	xor    %ah,%ah
    8d90:	89 05                	mov    %ax,(%di)
    8d92:	0e                   	push   %cs
    8d93:	fe                   	(bad)
    8d94:	fe                   	(bad)
    8d95:	6d                   	insw   (%dx),%es:(%di)
    8d96:	8b 16 69 9b          	mov    -0x6497,%dx
    8d9a:	80 e6 3f             	and    $0x3f,%dh
    8d9d:	88 75 8a             	mov    %dh,-0x76(%di)
    8da0:	80 36 85 9b 81       	xorb   $0x81,-0x647b
    8da5:	e2 0f                	loop   0x8db6
    8da7:	0f 3a d6             	(bad)
    8daa:	05 05 f6             	add    $0xf605,%ax
    8dad:	d6                   	(bad)
    8dae:	80 e6 0f             	and    $0xf,%dh
    8db1:	88 75 8a             	mov    %dh,-0x76(%di)
    8db4:	43                   	inc    %bx
    8db5:	5e                   	pop    %si
    8db6:	08 32                	or     %dh,(%bp,%si)
    8db8:	ff 03                	incw   (%bp,%di)
    8dba:	41                   	inc    %cx
    8dbb:	07                   	pop    %es
    8dbc:	b8 4e 1f             	mov    $0x1f4e,%ax
    8dbf:	32 ff                	xor    %bh,%bh
    8dc1:	03 96 8e 89          	add    -0x7672(%bp),%dx
    8dc5:	96                   	xchg   %ax,%si
    8dc6:	8e 03                	mov    (%bp,%di),%es
    8dc8:	1c ff                	sbb    $0xff,%al
    8dca:	5e                   	pop    %si
    8dcb:	ff 9f 60 9b          	lcall  *-0x64a0(%bx)
    8dcf:	ff e3                	jmp    *%bx
    8dd1:	fe c1                	inc    %cl
    8dd3:	88 4d 06             	mov    %cl,0x6(%di)
    8dd6:	8b 7d 0a             	mov    0xa(%di),%di
    8dd9:	8a dd                	mov    %ch,%bl
    8ddb:	32 ff                	xor    %bh,%bh
    8ddd:	80 ca 10             	or     $0x10,%dl
    8de0:	b4 05                	mov    $0x5,%ah
    8de2:	8b cb                	mov    %bx,%cx
    8de4:	d0 ea                	shr    $1,%dl
    8de6:	73 06                	jae    0x8dee
    8de8:	f3 a4                	rep movsb %ds:(%si),%es:(%di)
    8dea:	2b f3                	sub    %bx,%si
    8dec:	eb 04                	jmp    0x8df2
    8dee:	32 f0                	xor    %al,%dh
    8df0:	aa                   	stos   %al,%es:(%di)
    8df1:	43                   	inc    %bx
    8df2:	fe cc                	dec    %ah
    8df4:	75 ec                	jne    0x8de2
    8df6:	b8 48 9b             	mov    $0x9b48,%ax
    8df9:	ff                   	lcall  (bad)
    8dfa:	da a2 0a 4e          	fisubl 0x4e0a(%bp,%si)
    8dfe:	ca 08 b8             	lret   $0xb808
    8e01:	5f                   	pop    %di
    8e02:	00 52 f9             	add    %dl,-0x7(%bp,%si)
    8e05:	c9                   	leave
    8e06:	00 08                	add    %cl,(%bx,%si)
    8e08:	8b 51 7e             	mov    0x7e(%bx,%di),%dx
    8e0b:	de 57 fe             	ficoms -0x2(%bx)
    8e0e:	c0 d0 e8             	rcl    $0xe8,%al
    8e11:	88 45 06             	mov    %al,0x6(%di)
    8e14:	86 c5                	xchg   %al,%ch
    8e16:	d2 e8                	shr    %cl,%al
    8e18:	a2 68 5e             	mov    %al,0x5e68
    8e1b:	ff                   	(bad)
    8e1c:	7d 0a                	jge    0x8e28
    8e1e:	32 c6                	xor    %dh,%al
    8e20:	da da                	fcmovu %st(2),%st
    8e22:	9b                   	fwait
    8e23:	04 d1                	add    $0xd1,%al
    8e25:	bb 8a de             	mov    $0xde8a,%bx
    8e28:	8a b7 d1 bb          	mov    -0x442f(%bx),%dh
    8e2c:	8a c5                	mov    %ch,%al
    8e2e:	98                   	cbtw
    8e2f:	8b e8                	mov    %ax,%bp
    8e31:	ac                   	lods   %ds:(%si),%al
    8e32:	8a e0                	mov    %al,%ah
    8e34:	0a c9                	or     %cl,%cl
    8e36:	74 01                	je     0x8e39
    8e38:	ac                   	lods   %ds:(%si),%al
    8e39:	8a fe                	mov    %dh,%bh
    8e3b:	d1 e0                	shl    $1,%ax
    8e3d:	73 02                	jae    0x8e41
    8e3f:	8a fa                	mov    %dl,%bh
    8e41:	0a f0                	or     %al,%dh
    8e43:	30 8a de d1          	xor    %cl,-0x2e22(%bp,%si)
    8e47:	e0 73                	loopne 0x8ebc
    8e49:	02 8a da 80          	add    -0x7f26(%bp,%si),%cl
    8e4d:	0f e4 0a             	pmulhuw (%bp,%si),%mm1
    8e50:	df 26 88 1d          	fbld   0x1d88
    8e54:	47                   	inc    %di
    8e55:	4d                   	dec    %bp
    8e56:	75 e1                	jne    0x8e39
    8e58:	fe 0e 68 5e          	decb   0x5e68
    8e5c:	75 ce                	jne    0x8e2c
    8e5e:	5d                   	pop    %bp
    8e5f:	eb 95                	jmp    0x8df6
    8e61:	b9 b8 28             	mov    $0x28b8,%cx
    8e64:	10 e8                	adc    %ch,%al
    8e66:	06                   	push   %es
    8e67:	40                   	inc    %ax
    8e68:	06                   	push   %es
    8e69:	c0 4a 1e 76          	rorb   $0x76,0x1e(%bp,%si)
    8e6d:	53                   	push   %bx
    8e6e:	fc                   	cld
    8e6f:	05 05 ff             	add    $0xff05,%ax
    8e72:	0b 75 bb             	or     -0x45(%di),%si
    8e75:	9a e9 9b 00 83       	lcall  $0x8300,$0x9be9
    8e7a:	7f 0c                	jg     0x8e88
    8e7c:	00 bb 9a 8c          	add    %bh,-0x7366(%bp,%di)
    8e80:	5f                   	pop    %di
    8e81:	0c a1                	or     $0xa1,%al
    8e83:	74 74                	je     0x8ef9
    8e85:	3d 06 75             	cmp    $0x7506,%ax
    8e88:	b8 0c 04             	mov    $0x40c,%ax
    8e8b:	46                   	inc    %si
    8e8c:	0f 3a 06             	(bad)
    8e8f:	85 9b 74 03          	test   %bx,0x374(%bp,%di)
    8e93:	e8 88 03             	call   0x921e
    8e96:	a1 d6 f8             	mov    0xf8d6,%ax
    8e99:	f9                   	stc
    8e9a:	b3 ee                	mov    $0xee,%bl
    8e9c:	a1 3e 8b             	mov    0x8b3e,%ax
    8e9f:	04 8b                	add    $0x8b,%al
    8ea1:	8d 76 eb             	lea    -0x15(%bp),%si
    8ea4:	f6 46 06 2b          	testb  $0x2b,0x6(%bp)
    8ea8:	47                   	inc    %di
    8ea9:	02 89 04 03          	add    0x304(%bx,%di),%cl
    8ead:	07                   	pop    %es
    8eae:	48                   	dec    %ax
    8eaf:	05 04 4f             	add    $0x4f04,%ax
    8eb2:	06                   	push   %es
    8eb3:	33 00                	xor    (%bx,%si),%ax
    8eb5:	30 47 8e             	xor    %al,-0x72(%bx)
    8eb8:	89 44 02             	mov    %ax,0x2(%si)
    8ebb:	c9                   	leave
    8ebc:	4e                   	dec    %si
    8ebd:	8a 07                	mov    (%bx),%al
    8ebf:	77 03                	ja     0x8ec4
    8ec1:	c1 48 05 04          	rorw   $0x4,0x5(%bx,%si)
    8ec5:	06                   	push   %es
    8ec6:	bf d0 c0             	mov    $0xc0d0,%di
    8ec9:	83 c7 0a             	add    $0xa,%di
    8ecc:	fd                   	std
    8ecd:	f6 8d 46 f0 50       	testb  $0x50,-0xfba(%di)
    8ed2:	e8 a7 f2             	call   0x817c
    8ed5:	4e                   	dec    %si
    8ed6:	95                   	xchg   %ax,%bp
    8ed7:	eb 3b                	jmp    0x8f14
    8ed9:	8a 06 e1 88          	mov    -0x771f,%al
    8edd:	4e                   	dec    %si
    8ede:	e6 8d                	out    %al,$0x8d
    8ee0:	7e f0                	jle    0x8ed2
    8ee2:	8b 02                	mov    (%bp,%si),%ax
    8ee4:	8b 2b                	mov    (%bp,%di),%bp
    8ee6:	44                   	inc    %sp
    8ee7:	02 74 02             	add    0x2(%si),%dh
    8eea:	f6 47 51 0c          	testb  $0xc,0x51(%bx)
    8eee:	ec                   	in     (%dx),%al
    8eef:	8b 06 47 2b          	mov    0x2b47,%ax
    8ef3:	02 8b 40 88          	add    -0x77c0(%bp,%di),%cl
    8ef7:	46                   	inc    %si
    8ef8:	d4 8b                	aam    $0x8b
    8efa:	5d                   	pop    %bp
    8efb:	02 d1                	add    %cl,%dl
    8efd:	e3 03                	jcxz   0x8f02
    8eff:	ff 2e 36 f9          	ljmp   *-0x6ca
    8f03:	3f                   	aas
    8f04:	e2 1e                	loop   0x8f24
    8f06:	87 e0                	xchg   %sp,%ax
    8f08:	9a d3 e0 03 7e       	lcall  $0x7e03,$0xe0d3
    8f0d:	40                   	inc    %ax
    8f0e:	7e 96                	jle    0x8ea6
    8f10:	7d ff                	jge    0x8f11
    8f12:	7e 5e                	jle    0x8f72
    8f14:	ba 07 a6             	mov    $0xa607,%dx
    8f17:	8b f9                	mov    %cx,%di
    8f19:	c9                   	leave
    8f1a:	00 08                	add    %cl,(%bx,%si)
    8f1c:	cd 76                	int    $0x76
    8f1e:	4a                   	dec    %dx
    8f1f:	08 8b 36 04          	or     %cl,0x436(%bp,%di)
    8f23:	24 1b                	and    $0x1b,%al
    8f25:	de 0a                	fimuls (%bp,%si)
    8f27:	d8 96 d3 08          	fcoms  0x8d3(%bp)
    8f2b:	04 07                	add    $0x7,%al
    8f2d:	4f                   	dec    %di
    8f2e:	f6 b4 06 8b          	divb   -0x74fa(%si)
    8f32:	0a e0                	or     %al,%ah
    8f34:	d1 e0                	shl    $1,%ax
    8f36:	d1 e0                	shl    $1,%ax
    8f38:	8b 0a                	mov    (%bp,%si),%cx
    8f3a:	e1 03                	loope  0x8f3f
    8f3c:	c6                   	(bad)
    8f3d:	8b 0a                	mov    (%bp,%si),%cx
    8f3f:	de 03                	fiadds (%bp,%di)
    8f41:	96                   	xchg   %ax,%si
    8f42:	f8                   	clc
    8f43:	5e                   	pop    %si
    8f44:	f0 8b c3             	lock mov %bx,%ax
    8f47:	d3 f8                	sar    %cl,%ax
    8f49:	de 0a                	fimuls (%bp,%si)
    8f4b:	ea 32 e4 03 eb       	ljmp   $0xeb03,$0xe432
    8f50:	f6 8b d3 03 a3       	testb  $0xa3,0x3d3(%bp,%di)
    8f55:	2b c2                	sub    %dx,%ax
    8f57:	48                   	dec    %ax
    8f58:	01 0c                	add    %cx,(%si)
    8f5a:	ec                   	in     (%dx),%al
    8f5b:	83 07 e8             	addw   $0xffe8,(%bx)
    8f5e:	8a 87 74 9b          	mov    -0x648c(%bx),%al
    8f62:	46                   	inc    %si
    8f63:	8b dc                	mov    %sp,%bx
    8f65:	f8                   	clc
    8f66:	5e                   	pop    %si
    8f67:	f4                   	hlt
    8f68:	8b d3                	mov    %bx,%dx
    8f6a:	de ea                	fsubrp %st,%st(2)
    8f6c:	e4 eb                	in     $0xeb,%al
    8f6e:	e8 2a 0a             	call   0x999b
    8f71:	ea e4 eb e4 8a       	ljmp   $0x8ae4,$0xebe4
    8f76:	56                   	push   %si
    8f77:	e6 2a                	out    %al,$0x2a
    8f79:	d0 fe                	sar    $1,%dh
    8f7b:	ca 88 56             	lret   $0x5688
    8f7e:	e2 83                	loop   0x8f03
    8f80:	07                   	pop    %es
    8f81:	e8 8a 87             	call   0x170e
    8f84:	6c                   	insb   (%dx),%es:(%di)
    8f85:	9b                   	fwait
    8f86:	46                   	inc    %si
    8f87:	8b da                	mov    %dx,%bx
    8f89:	8a 56 e4             	mov    -0x1c(%bp),%dl
    8f8c:	0a d2                	or     %dl,%dl
    8f8e:	75 80                	jne    0x8f10
    8f90:	85 8b da 22          	test   %cx,0x22da(%bp,%di)
    8f94:	8b dc                	mov    %sp,%bx
    8f96:	46                   	inc    %si
    8f97:	8b dc                	mov    %sp,%bx
    8f99:	f8                   	clc
    8f9a:	46                   	inc    %si
    8f9b:	ee                   	out    %al,(%dx)
    8f9c:	d3 e8                	shr    %cl,%ax
    8f9e:	2a 0a                	sub    (%bp,%si),%cl
    8fa0:	e4 32                	in     $0x32,%al
    8fa2:	e4 48                	in     $0x48,%al
    8fa4:	fa                   	cli
    8fa5:	07                   	pop    %es
    8fa6:	ee                   	out    %al,(%dx)
    8fa7:	f8                   	clc
    8fa8:	46                   	inc    %si
    8fa9:	ec                   	in     (%dx),%al
    8faa:	01 0c                	add    %cx,(%si)
    8fac:	de 03                	fiadds (%bp,%di)
    8fae:	f0 89 76 ce          	lock mov %si,-0x32(%bp)
    8fb2:	89 7e 7e             	mov    %di,0x7e(%bp)
    8fb5:	f7 c6 47 05          	test   $0x547,%si
    8fb9:	36 0e                	ss push %cs
    8fbb:	d8 01                	fadds  (%bx,%di)
    8fbd:	32 c0                	xor    %al,%al
    8fbf:	46                   	inc    %si
    8fc0:	8b d6                	mov    %si,%dx
    8fc2:	8a c8                	mov    %al,%cl
    8fc4:	ba 26 e1             	mov    $0xe126,%dx
    8fc7:	04 e0                	add    $0xe0,%al
    8fc9:	b0 9a                	mov    $0x9a,%al
    8fcb:	00 b0 01 d2          	add    %dh,-0x2dff(%bx,%si)
    8fcf:	e0 ba                	loopne 0x8f8b
    8fd1:	03 e3                	add    %bx,%sp
    8fd3:	9a e0 b0 02 ef       	lcall  $0xef02,$0xb0e0
    8fd8:	8a 4e d8             	mov    -0x28(%bp),%cl
    8fdb:	96                   	xchg   %ax,%si
    8fdc:	d3 de                	rcr    %cl,%si
    8fde:	8b 7e 7e             	mov    0x7e(%bp),%di
    8fe1:	85 8b d4 46          	test   %cx,0x46d4(%bp,%di)
    8fe5:	8b d2                	mov    %dx,%dx
    8fe7:	8a 6e e4             	mov    -0x1c(%bp),%ch
    8fea:	8a 37                	mov    (%bx),%dh
    8fec:	43                   	inc    %bx
    8fed:	8a 17                	mov    (%bx),%dl
    8fef:	d3 ea                	shr    %cl,%dx
    8ff1:	22 56 dc             	and    -0x24(%bp),%dl
    8ff4:	8a f2                	mov    %dl,%dh
    8ff6:	f6 d6                	not    %dh
    8ff8:	8a 24                	mov    (%si),%ah
    8ffa:	46                   	inc    %si
    8ffb:	8a 04                	mov    (%si),%al
    8ffd:	d3 e8                	shr    %cl,%ax
    8fff:	26 22 35             	and    %es:(%di),%dh
    9002:	22 c2                	and    %dl,%al
    9004:	0a c6                	or     %dh,%al
    9006:	aa                   	stos   %al,%es:(%di)
    9007:	0a ed                	or     %ch,%ch
    9009:	74 3f                	je     0x904a
    900b:	fe cd                	dec    %ch
    900d:	05 46 fc             	add    $0xfc46,%ax
    9010:	17                   	pop    %ss
    9011:	86 d6                	xchg   %dl,%dh
    9013:	43                   	inc    %bx
    9014:	d3 ea                	shr    %cl,%dx
    9016:	8a f2                	mov    %dl,%dh
    9018:	f6 d6                	not    %dh
    901a:	36 04 86             	ss add $0x86,%al
    901d:	c4 46 d3             	les    -0x2d(%bp),%ax
    9020:	26 35 c2 c6          	es xor $0xc6c2,%ax
    9024:	0a 74 fe             	or     -0x2(%si),%dh
    9027:	05 fc fe             	add    $0xfefc,%ax
    902a:	75 fe                	jne    0x902a
    902c:	e2 fc                	loop   0x902a
    902e:	86 43 ea             	xchg   %al,-0x16(%bp,%di)
    9031:	f2 d6                	repnz (bad)
    9033:	04 22                	add    $0x22,%al
    9035:	56                   	push   %si
    9036:	da 8a f2 d6          	fimull -0x290e(%bp,%si)
    903a:	04 c4                	add    $0xc4,%al
    903c:	d3 35                	shlw   %cl,(%di)
    903e:	c6                   	(bad)
    903f:	74 05                	je     0x9046
    9041:	fe                   	(bad)
    9042:	fe                   	(bad)
    9043:	fc                   	cld
    9044:	43                   	inc    %bx
    9045:	f2 04 56             	repnz add $0x56,%al
    9048:	8a d6                	mov    %dh,%dl
    904a:	85 8b e2 98          	test   %cx,-0x671e(%bp,%di)
    904e:	03 f0                	add    %ax,%si
    9050:	03 d8                	add    %ax,%bx
    9052:	03 7e ee             	add    -0x12(%bp),%di
    9055:	fe 4e d2             	decb   -0x2e(%bp)
    9058:	75 8d                	jne    0x8fe7
    905a:	8b 76 ce             	mov    -0x32(%bp),%si
    905d:	03 76 e0             	add    -0x20(%bp),%si
    9060:	89 76 ce             	mov    %si,-0x32(%bp)
    9063:	fe 8b d6 8a          	decb   -0x752a(%bp,%di)
    9067:	8b d6                	mov    %si,%dx
    9069:	3c 04                	cmp    $0x4,%al
    906b:	02 4c 00             	add    0x0(%si),%cl
    906e:	52                   	push   %dx
    906f:	ff                   	(bad)
    9070:	ba 03 0f             	mov    $0xf03,%dx
    9073:	9a 1f 52 f9 c9       	lcall  $0xc9f9,$0x521f
    9078:	e9 99 fe             	jmp    0x8f14
    907b:	b0 ff                	mov    $0xff,%al
    907d:	8b 56 f0             	mov    -0x10(%bp),%dx
    9080:	2b 14                	sub    (%si),%dx
    9082:	d1 ea                	shr    $1,%dx
    9084:	73 02                	jae    0x9088
    9086:	32 c0                	xor    %al,%al
    9088:	46                   	inc    %si
    9089:	8b ca                	mov    %dx,%cx
    908b:	45                   	inc    %bp
    908c:	8b 47 2b             	mov    0x2b(%bx),%ax
    908f:	04 d1                	add    $0xd1,%al
    9091:	e8 f8 f4             	call   0x858c
    9094:	d3 2b                	shrw   %cl,(%bp,%di)
    9096:	1c d1                	sbb    $0xd1,%al
    9098:	eb 2b                	jmp    0x90c5
    909a:	c3                   	ret
    909b:	03 c2                	add    %dx,%ax
    909d:	fa                   	cli
    909e:	07                   	pop    %es
    909f:	c8 f7 c6 47          	enter  $0xc6f7,$0x47
    90a3:	05 0a 03             	add    $0x30a,%ax
    90a6:	8b 76 ec             	mov    -0x14(%bp),%si
    90a9:	03 f2                	add    %dx,%si
    90ab:	8a 6f 08             	mov    0x8(%bx),%ch
    90ae:	b1 47                	mov    $0x47,%cl
    90b0:	09 10                	or     %dx,(%bx,%si)
    90b2:	e0 c0                	loopne 0x9074
    90b4:	8b b9 00 5e          	mov    0x5e00(%bx,%di),%di
    90b8:	ac                   	lods   %ds:(%si),%al
    90b9:	07                   	pop    %es
    90ba:	fa                   	cli
    90bb:	07                   	pop    %es
    90bc:	c6 05 0e             	movb   $0xe,(%di)
    90bf:	01 c0                	add    %ax,%ax
    90c1:	8b 03                	mov    (%bp,%di),%ax
    90c3:	7e f0                	jle    0x90b5
    90c5:	47                   	inc    %di
    90c6:	ff d3                	call   *%bx
    90c8:	2b 5e f0             	sub    -0x10(%bp),%bx
    90cb:	43                   	inc    %bx
    90cc:	89 5e c4             	mov    %bx,-0x3c(%bp)
    90cf:	f8                   	clc
    90d0:	ee                   	out    %al,(%dx)
    90d1:	e8 2b c3             	call   0x53ff
    90d4:	fa                   	cli
    90d5:	07                   	pop    %es
    90d6:	cc                   	int3
    90d7:	8b 56 c4             	mov    -0x3c(%bp),%dx
    90da:	f6 8b ca ff 75       	testb  $0x75,-0x36(%bp,%di)
    90df:	03 ac eb 1d          	add    0x1deb(%si),%bp
    90e3:	ac                   	lods   %ds:(%si),%al
    90e4:	e0 c9                	loopne 0x90af
    90e6:	d2 e8                	shr    %cl,%al
    90e8:	3a c5                	cmp    %ch,%al
    90ea:	74 0e                	je     0x90fa
    90ec:	47                   	inc    %di
    90ed:	ff c6                	inc    %si
    90ef:	02 d8                	add    %al,%bl
    90f1:	80 d7 00             	adc    $0x0,%bh
    90f4:	36 8a 07             	mov    %ss:(%bx),%al
    90f7:	26 05 31 47          	es add $0x4731,%ax
    90fb:	4a                   	dec    %dx
    90fc:	74 1a                	je     0x9118
    90fe:	8a c4                	mov    %ah,%al
    9100:	24 0f                	and    $0xf,%al
    9102:	3a 74 47             	cmp    0x47(%si),%dh
    9105:	c6                   	(bad)
    9106:	d8 d7                	fcom   %st(7)
    9108:	36 07                	ss pop %es
    910a:	05 47 74             	add    $0x7447,%ax
    910d:	8a 24                	mov    (%si),%ah
    910f:	3a 47 d8             	cmp    -0x28(%bx),%al
    9112:	36 05 74 24          	ss add $0x2474,%ax
    9116:	75 cb                	jne    0x90e3
    9118:	03 7e cc             	add    -0x34(%bp),%di
    911b:	8b 76 c8             	mov    -0x38(%bp),%si
    911e:	fe 4e d4             	decb   -0x2c(%bp)
    9121:	75 b4                	jne    0x90d7
    9123:	e9 ee fd             	jmp    0x8f14
    9126:	89 56 c2             	mov    %dx,-0x3e(%bp)
    9129:	32 c9                	xor    %cl,%cl
    912b:	f7 04 01 00          	testw  $0x1,(%si)
    912f:	74 02                	je     0x9133
    9131:	b1 47                	mov    $0x47,%cl
    9133:	88 4e 96             	mov    %cl,-0x6a(%bp)
    9136:	de 56 f0             	ficoms -0x10(%bp)
    9139:	d1 fa                	sar    $1,%dx
    913b:	88 56 ea             	mov    %dl,-0x16(%bp)
    913e:	8b 1c                	mov    (%si),%bx
    9140:	d1 fb                	sar    $1,%bx
    9142:	2b d3                	sub    %bx,%dx
    9144:	45                   	inc    %bp
    9145:	8b 8e 4e f4          	mov    -0xbb2(%bp),%cx
    9149:	d1 f9                	sar    $1,%cx
    914b:	88 4e e8             	mov    %cl,-0x18(%bp)
    914e:	2a 4e ea             	sub    -0x16(%bp),%cl
    9151:	88 4e e4             	mov    %cl,-0x1c(%bp)
    9154:	bb ff 21             	mov    $0x21ff,%bx
    9157:	f7 46 f0 01 00       	testw  $0x1,-0x10(%bp)
    915c:	74 02                	je     0x9160
    915e:	b3 0f                	mov    $0xf,%bl
    9160:	88 5e dc             	mov    %bl,-0x24(%bp)
    9163:	f7 46 f4 01 00       	testw  $0x1,-0xc(%bp)
    9168:	75 02                	jne    0x916c
    916a:	b7 f0                	mov    $0xf0,%bh
    916c:	88 7e da             	mov    %bh,-0x26(%bp)
    916f:	8a 46 e6             	mov    -0x1a(%bp),%al
    9172:	2a c1                	sub    %cl,%al
    9174:	32 89 0e 07          	xor    0x70e(%bx,%di),%cl
    9178:	c8 0a c9 75          	enter  $0xc90a,$0x75
    917c:	05 22 df             	add    $0xdf22,%ax
    917f:	88 5e dc             	mov    %bl,-0x24(%bp)
    9182:	f7 c6 47 03          	test   $0x347,%si
    9186:	0a 03                	or     (%bp,%di),%al
    9188:	8b 76 ec             	mov    -0x14(%bp),%si
    918b:	05 0e 01             	add    $0x10e,%ax
    918e:	c0 8b 8a d8 d3       	rorb   $0xd3,-0x2776(%bp,%di)
    9193:	8b 7e c2             	mov    -0x3e(%bp),%di
    9196:	0e                   	push   %cs
    9197:	3f                   	aas
    9198:	fe 83 75 c4          	incb   -0x3b8b(%bp,%di)
    919c:	89 5e c2             	mov    %bx,-0x3e(%bp)
    919f:	8a 5e ea             	mov    -0x16(%bp),%bl
    91a2:	32 03                	xor    (%bp,%di),%al
    91a4:	8e fb                	mov    %bx,%?
    91a6:	8a e4                	mov    %ah,%ah
    91a8:	37                   	aaa
    91a9:	8b 44 ff             	mov    -0x1(%si),%ax
    91ac:	d3 c8                	ror    %cl,%ax
    91ae:	8a dc                	mov    %ah,%bl
    91b0:	8a 76 dc             	mov    -0x24(%bp),%dh
    91b3:	36 22 b7 86 9b       	and    %ss:-0x647a(%bx),%dh
    91b8:	8a d6                	mov    %dh,%dl
    91ba:	f6 d2                	not    %dl
    91bc:	26 22 15             	and    %es:(%di),%dl
    91bf:	22 e6                	and    %dh,%ah
    91c1:	0a e2                	or     %dl,%ah
    91c3:	d8 36 25 47          	fdivs  0x4725
    91c7:	0a 74 fe             	or     -0x2(%si),%dh
    91ca:	45                   	inc    %bp
    91cb:	fe 05                	incb   (%di)
    91cd:	fc                   	cld
    91ce:	23 8b 04 d3          	and    -0x2cfc(%bp,%di),%cx
    91d2:	c8 46 8a dc          	enter  $0x8a46,$0xdc
    91d6:	24 3a                	and    $0x3a,%al
    91d8:	b7 86                	mov    $0x86,%bh
    91da:	9b                   	fwait
    91db:	0a f6                	or     %dh,%dh
    91dd:	47                   	inc    %di
    91de:	c6                   	(bad)
    91df:	8a f6                	mov    %dh,%dh
    91e1:	26 15 e6 e2          	es adc $0xe2e6,%ax
    91e5:	36 47                	ss inc %di
    91e7:	74 45                	je     0x922e
    91e9:	05 23 04             	add    $0x423,%ax
    91ec:	c8 8a fe fe          	enter  $0xfe8a,$0xfe
    91f0:	fc                   	cld
    91f1:	dd 8b 04 d3          	fisttpll -0x2cfc(%bp,%di)
    91f5:	c8 8a dc 8a          	enter  $0xdc8a,$0x8a
    91f9:	76 da                	jbe    0x91d5
    91fb:	36 b7 9b             	ss mov $0x9b,%bh
    91fe:	d6                   	(bad)
    91ff:	d2 22                	shlb   %cl,(%bp,%si)
    9201:	22 0a                	and    (%bp,%si),%cl
    9203:	d8 25                	fsubs  (%di)
    9205:	0a fe                	or     %dh,%bh
    9207:	fe                   	(bad)
    9208:	fc                   	cld
    9209:	8b d3                	mov    %bx,%dx
    920b:	46                   	inc    %si
    920c:	dc 3a                	fdivrl (%bp,%si)
    920e:	86 46 8b             	xchg   %al,-0x75(%bp)
    9211:	c8 4e 75 e9          	enter  $0x754e,$0xe9
    9215:	fd                   	std
    9216:	02 00                	add    (%bx,%si),%al
    9218:	ff                   	(bad)
    9219:	78 ff                	js     0x921a
    921b:	e9 f6 fc             	jmp    0x8f14
    921e:	53                   	push   %bx
    921f:	8c db                	mov    %ds,%bx
    9221:	8e c3                	mov    %bx,%es
    9223:	bf d6 d2             	mov    $0xd2d6,%di
    9226:	a2 85 d2             	mov    %al,0xd285
    9229:	22 d8                	and    %al,%bl
    922b:	f9                   	stc
    922c:	6f                   	outsw  %ds:(%si),(%dx)
    922d:	04 b9                	add    $0xb9,%al
    922f:	80 00 f3             	addb   $0xf3,(%bx,%si)
    9232:	ab                   	stos   %ax,%es:(%di)
    9233:	32 da                	xor    %dl,%bl
    9235:	9b                   	fwait
    9236:	e3 81                	jcxz   0x91b9
    9238:	ef                   	out    %ax,(%dx)
    9239:	00 01                	add    %al,(%bx,%di)
    923b:	b0 f0                	mov    $0xf0,%al
    923d:	10 8d ef 88          	adc    %cl,-0x7711(%di)
    9241:	01 83 75 10          	add    %ax,0x1075(%bp,%di)
    9245:	e2 f9                	loop   0x9240
    9247:	32 da                	xor    %dl,%bl
    9249:	9b dc b1 04 d3       	fdivl  -0x2cfc(%bx,%di)
    924e:	e3 b0                	jcxz   0x9200
    9250:	0f 10 8d ef 20       	movups 0x20ef(%di),%xmm1
    9255:	01 43 e2             	add    %ax,-0x1e(%bp,%di)
    9258:	fb                   	sti
    9259:	73 4c                	jae    0x92a7
    925b:	53                   	push   %bx
    925c:	7e 7f                	jle    0x92dd
    925e:	06                   	push   %es
    925f:	57                   	push   %di
    9260:	0e                   	push   %cs
    9261:	47                   	inc    %di
    9262:	06                   	push   %es
    9263:	4f                   	dec    %di
    9264:	06                   	push   %es
    9265:	a1 3e 8b             	mov    0x8b3e,%ax
    9268:	04 8b                	add    $0x8b,%al
    926a:	81 e2 08 8b          	and    $0x8b08,%dx
    926e:	0a 56 3b             	or     0x3b(%bp),%dl
    9271:	76 06                	jbe    0x9279
    9273:	7e 0c                	jle    0x9281
    9275:	87 76 06             	xchg   %si,0x6(%bp)
    9278:	89 0a                	mov    %cx,(%bp,%si)
    927a:	56                   	push   %si
    927b:	87 96 89 8e          	xchg   %dx,-0x7177(%bp)
    927f:	1c 08                	sbb    $0x8,%al
    9281:	8b 3e df c0          	mov    -0x3f21,%di
    9285:	8b 81 e2 1e          	mov    0x1ee2(%bx,%di),%ax
    9289:	87 e0                	xchg   %sp,%ax
    928b:	9a 1c 2a 03 06       	lcall  $0x603,$0x2a1c
    9290:	00 eb                	add    %ch,%bl
    9292:	b1 8c                	mov    $0x8c,%cl
    9294:	9c                   	pushf
    9295:	ff 9a ad e0          	lcall  *-0x1f53(%bp,%si)
    9299:	d3 0e 8b 9a          	rorw   %cl,-0x6575
    929d:	83 8b 9e 06 2b       	orw    $0x2b,0x69e(%bp,%di)
    92a2:	ce                   	into
    92a3:	2a 03                	sub    (%bp,%di),%al
    92a5:	d6                   	(bad)
    92a6:	c0 8b fb fb fb       	rorb   $0xfb,-0x405(%bp,%di)
    92ab:	1e                   	push   %ds
    92ac:	9a 8b 04 ba 2b       	lcall  $0x2bba,$0x48b
    92b1:	56                   	push   %si
    92b2:	08 0b                	or     %cl,(%bp,%di)
    92b4:	d2 79 04             	sarb   %cl,0x4(%bx,%di)
    92b7:	f7 db                	neg    %bx
    92b9:	f7 da                	neg    %dx
    92bb:	89 1e 8a 9c          	mov    %bx,-0x6376
    92bf:	8b 5a 60             	mov    0x60(%bp,%si),%bx
    92c2:	f8                   	clc
    92c3:	60                   	pusha
    92c4:	f8                   	clc
    92c5:	60                   	pusha
    92c6:	f8                   	clc
    92c7:	03 f6                	add    %si,%si
    92c9:	d3 fa                	sar    %cl,%dx
    92cb:	83 f7 0f             	xor    $0xf,%di
    92ce:	fb                   	sti
    92cf:	0b c9                	or     %cx,%cx
    92d1:	75 20                	jne    0x92f3
    92d3:	8b ca                	mov    %dx,%cx
    92d5:	41                   	inc    %cx
    92d6:	83 07 f9             	addw   $0xfff9,(%bx)
    92d9:	ba 26 a4             	mov    $0xa426,%dx
    92dc:	9a 86 03 9a 75       	lcall  $0x759a,$0x386
    92e1:	03 ef                	add    %di,%bp
    92e3:	3e 2e 8a 9c 8a df    	ds mov %cs:-0x2076(%si),%bl
    92e9:	26 1d 03 03          	es sbb $0x303,%ax
    92ed:	fd                   	std
    92ee:	e2 f7                	loop   0x92e7
    92f0:	e9 88 00             	jmp    0x937b
    92f3:	3b ca                	cmp    %dx,%cx
    92f5:	7e 42                	jle    0x9339
    92f7:	d1 e2                	shl    $1,%dx
    92f9:	89 16 86 9c          	mov    %dx,-0x637a
    92fd:	2b d1                	sub    %cx,%dx
    92ff:	8b ea                	mov    %dx,%bp
    9301:	2b d1                	sub    %cx,%dx
    9303:	89 16 88 9c          	mov    %dx,-0x6378
    9307:	ba 26 a4             	mov    $0xa426,%dx
    930a:	75 03                	jne    0x930f
    930c:	41                   	inc    %cx
    930d:	83 07 f9             	addw   $0xfff9,(%bx)
    9310:	9a 86 03 9a ef       	lcall  $0xef9a,$0x386
    9315:	8a df                	mov    %bh,%bl
    9317:	26 1d 03 8c          	es sbb $0x8c03,%ax
    931b:	fe                   	(bad)
    931c:	7c 08                	jl     0x9326
    931e:	75 01                	jne    0x9321
    9320:	47                   	inc    %di
    9321:	0b ed                	or     %bp,%bp
    9323:	79 08                	jns    0x932d
    9325:	03 2e 86 9c          	add    -0x637a,%bp
    9329:	e2 e2                	loop   0x930d
    932b:	eb 4e                	jmp    0x937b
    932d:	87 e0                	xchg   %sp,%ax
    932f:	8a 9c 03 2e          	mov    0x2e03(%si),%bl
    9333:	88 9c e2 d6          	mov    %bl,-0x291e(%si)
    9337:	eb 42                	jmp    0x937b
    9339:	d1 e1                	shl    $1,%cx
    933b:	89 0e 86 9c          	mov    %cx,-0x637a
    933f:	2b ca                	sub    %dx,%cx
    9341:	8b e9                	mov    %cx,%bp
    9343:	2b ca                	sub    %dx,%cx
    9345:	89 0e 88 9c          	mov    %cx,-0x6378
    9349:	8b ca                	mov    %dx,%cx
    934b:	41                   	inc    %cx
    934c:	ba 26 a4             	mov    $0xa426,%dx
    934f:	75 03                	jne    0x9354
    9351:	83 f9 86             	cmp    $0xff86,%cx
    9354:	9a 8a 26 03 fe       	lcall  $0xfe03,$0x268a
    9359:	08 01                	or     %al,(%bx,%di)
    935b:	0b 79 03             	or     0x3(%bx,%di),%di
    935e:	87 e0                	xchg   %sp,%ax
    9360:	8a 9c 0b 79          	mov    0x790b(%si),%bl
    9364:	03 86 e2 eb          	add    -0x141e(%bp),%ax
    9368:	87 8a 03 ed          	xchg   %cx,-0x12fd(%bp,%si)
    936c:	eb 0d                	jmp    0x937b
    936e:	8c 7c 75             	mov    %?,0x75(%si)
    9371:	47                   	inc    %di
    9372:	ed                   	in     (%dx),%ax
    9373:	08 2e 03 88          	or     %ch,-0x77fd
    9377:	e2 eb                	loop   0x9364
    9379:	d1 89 04 b8          	rorw   $1,-0x47fc(%bx,%di)
    937d:	58                   	pop    %ax
    937e:	d6                   	(bad)
    937f:	9a 74 8b 03 ad       	lcall  $0xad03,$0x8b74
    9384:	56                   	push   %si
    9385:	8b c0                	mov    %ax,%ax
    9387:	57                   	push   %di
    9388:	3e df e9             	ds fucomip %st(1),%st
    938b:	76 01                	jbe    0x938e
    938d:	8b 9e 06 2b          	mov    0x2b06(%bp),%bx
    9391:	ce                   	into
    9392:	bb 02 00             	mov    $0x2,%bx
    9395:	8b ba 56 0b          	mov    0xb56(%bp,%si),%di
    9399:	79 f7                	jns    0x9392
    939b:	f7 89 8a 8b 60 60    	testw  $0x6060,-0x7476(%bx,%di)
    93a1:	60                   	pusha
    93a2:	03 d3                	add    %bx,%dx
    93a4:	83 0f 0b             	orw    $0xb,(%bx)
    93a7:	75 8b                	jne    0x9334
    93a9:	41                   	inc    %cx
    93aa:	07                   	pop    %es
    93ab:	03 d3                	add    %bx,%dx
    93ad:	83 0f 0b             	orw    $0xb,(%bx)
    93b0:	75 83                	jne    0x9335
    93b2:	9f                   	lahf
    93b3:	97                   	xchg   %ax,%di
    93b4:	9b                   	fwait
    93b5:	a7                   	cmpsw  %es:(%di),%ds:(%si)
    93b6:	04 03                	add    $0x3,%al
    93b8:	43                   	inc    %bx
    93b9:	5e                   	pop    %si
    93ba:	08 d1                	or     %dl,%cl
    93bc:	e3 3b                	jcxz   0x93f9
    93be:	ca 7e 56             	lret   $0x567e
    93c1:	d1 89 86 2b          	rorw   $1,0x2b86(%bx,%di)
    93c5:	8b 2b                	mov    (%bp,%di),%bp
    93c7:	89 88 ba a4          	mov    %cx,-0x5b46(%bx,%si)
    93cb:	03 83 f9 86          	add    -0x7907(%bp,%di),%ax
    93cf:	9a 8a 41 ba 0f       	lcall  $0xfba,$0x418a
    93d4:	f0 f7 c6 01 74       	lock test $0x7401,%si
    93d9:	b3 88                	mov    $0x88,%bl
    93db:	f7 d2                	not    %dx
    93dd:	8a c4                	mov    %ah,%al
    93df:	22 c6                	and    %dh,%al
    93e1:	26 20 15             	and    %dl,%es:(%di)
    93e4:	26 08 05             	or     %al,%es:(%di)
    93e7:	46                   	inc    %si
    93e8:	f7 c6 01 74          	test   $0x7401,%si
    93ec:	75 47                	jne    0x9435
    93ee:	ed                   	in     (%dx),%ax
    93ef:	08 2e 9c e2          	or     %ch,-0x1d64
    93f3:	4e                   	dec    %si
    93f4:	e0 9c                	loopne 0x9392
    93f6:	2e 9c                	cs pushf
    93f8:	d9 eb                	fldpi
    93fa:	72 03                	jb     0x93ff
    93fc:	88 e2                	mov    %ah,%dl
    93fe:	eb 03                	jmp    0x9403
    9400:	83 0f 0b             	orw    $0xb,(%bx)
    9403:	75 3e                	jne    0x9443
    9405:	df d8                	(bad)
    9407:	57                   	push   %di
    9408:	1e                   	push   %ds
    9409:	e0 1c                	loopne 0x9427
    940b:	03 00                	add    (%bx,%si),%ax
    940d:	b1 d6                	mov    $0xd6,%cl
    940f:	d1 fa                	sar    $1,%dx
    9411:	fe                   	(bad)
    9412:	26 e2 bd             	es loop 0x93d2
    9415:	eb 56                	jmp    0x946d
    9417:	d1 e1                	shl    $1,%cx
    9419:	0e                   	push   %cs
    941a:	9c                   	pushf
    941b:	ca e9 ca             	lret   $0xcae9
    941e:	0e                   	push   %cs
    941f:	9c                   	pushf
    9420:	ca ba a4             	lret   $0xa4ba
    9423:	03 f9                	add    %cx,%di
    9425:	9a 26 fe 01 79       	lcall  $0x7901,$0xfe26
    942a:	87 0f                	xchg   %cx,(%bx)
    942c:	f7 01 b3 f7          	testw  $0xf7b3,(%bx,%di)
    9430:	8a 22                	mov    (%bp,%si),%ah
    9432:	26 15 08 46          	es adc $0x4608,%ax
    9436:	c6                   	(bad)
    9437:	74 47                	je     0x9480
    9439:	08 9c 4e 9c          	or     %bl,-0x63b2(%si)
    943d:	9c                   	pushf
    943e:	eb 03                	jmp    0x9443
    9440:	0f 75 df             	pcmpeqw %mm7,%mm3
    9443:	57                   	push   %di
    9444:	e0 03                	loopne 0x9449
    9446:	b1 d1                	mov    $0xd1,%cl
    9448:	fe                   	(bad)
    9449:	e2 eb                	loop   0x9436
    944b:	d1 0e ca ca          	rorw   $1,-0x3536
    944f:	9c                   	pushf
    9450:	ba 03 9a             	mov    $0x9a03,%dx
    9453:	08 9c 4e 9c          	or     %bl,-0x63b2(%si)
    9457:	9c                   	pushf
    9458:	eb 03                	jmp    0x945d
    945a:	e2 03                	loop   0x945f
    945c:	cd eb                	int    $0xeb
    945e:	0e                   	push   %cs
    945f:	03 88 e2 eb          	add    -0x141e(%bx,%si),%cx
    9463:	46                   	inc    %si
    9464:	c6                   	(bad)
    9465:	74 47                	je     0x94ae
    9467:	08 9c 4e 9c          	or     %bl,-0x63b2(%si)
    946b:	e2 bd                	loop   0x942a
    946d:	e9 93 00             	jmp    0x9503
    9470:	03 f7                	add    %di,%si
    9472:	4e                   	dec    %si
    9473:	36 06                	ss push %es
    9475:	ce                   	into
    9476:	03 c0                	add    %ax,%ax
    9478:	fb                   	sti
    9479:	fb                   	sti
    947a:	9a 8b 56 79 f7       	lcall  $0xf779,$0x568b
    947f:	8a 60 60             	mov    0x60(%bx,%si),%ah
    9482:	d3 0f                	rorw   %cl,(%bx)
    9484:	75 41                	jne    0x94c7
    9486:	03 83 0b 8b          	add    -0x74f5(%bp,%di),%ax
    948a:	eb a1                	jmp    0x942d
    948c:	8b f2                	mov    %dx,%si
    948e:	0b 75 8b             	or     -0x75(%di),%si
    9491:	11 60 d3             	adc    %sp,-0x2d(%bx,%si)
    9494:	75 eb                	jne    0x9481
    9496:	a1 68 fe             	mov    0xfe68,%ax
    9499:	01 d8                	add    %bx,%ax
    949b:	05 24 03             	add    $0x324,%ax
    949e:	e2 e9                	loop   0x9489
    94a0:	f9                   	stc
    94a1:	eb 5d                	jmp    0x9500
    94a3:	60                   	pusha
    94a4:	d3 75 04             	shlw   %cl,0x4(%di)
    94a7:	aa                   	stos   %al,%es:(%di)
    94a8:	06                   	push   %es
    94a9:	8a 55 3b             	mov    0x3b(%di),%dl
    94ac:	ca 7e 27             	lret   $0x277e
    94af:	d1 86 8b 89          	rolw   $1,-0x7675(%bp)
    94b3:	ba 03 f9             	mov    $0xf903,%dx
    94b6:	9a 41 da 03 f9       	lcall  $0xf903,$0xda41
    94bb:	9a 41 0f f7 aa       	lcall  $0xaaf7,$0xf41
    94c0:	0b db                	or     %bx,%bx
    94c2:	4e                   	dec    %si
    94c3:	9c                   	pushf
    94c4:	9c                   	pushf
    94c5:	1e                   	push   %ds
    94c6:	03 e2                	add    %dx,%sp
    94c8:	03 f5                	add    %bp,%si
    94ca:	eb 34                	jmp    0x9500
    94cc:	03 e2                	add    %dx,%sp
    94ce:	9c                   	pushf
    94cf:	1e                   	push   %ds
    94d0:	e2 d1                	loop   0x94a3
    94d2:	04 eb                	add    $0xeb,%al
    94d4:	eb 2a                	jmp    0x9500
    94d6:	d1 0e ca ca          	rorw   $1,-0x3536
    94da:	9c                   	pushf
    94db:	ba 03 9a             	mov    $0x9a03,%dx
    94de:	fe                   	(bad)
    94df:	d9 ba 03 9a          	fnstcw -0x65fd(%bp,%si)
    94e3:	fe                   	(bad)
    94e4:	79 0f                	jns    0x94f5
    94e6:	01 f7                	add    %si,%di
    94e8:	d8 05                	fadds  (%di)
    94ea:	24 03                	and    $0x3,%al
    94ec:	e2 0b                	loop   0x94f9
    94ee:	4e                   	dec    %si
    94ef:	9c                   	pushf
    94f0:	03 03                	add    (%bp,%di),%ax
    94f2:	eb 03                	jmp    0x94f7
    94f4:	9c                   	pushf
    94f5:	e2 f1                	loop   0x94e8
    94f7:	eb 07                	jmp    0x9500
    94f9:	47                   	inc    %di
    94fa:	9c                   	pushf
    94fb:	1e                   	push   %ds
    94fc:	e2 d1                	loop   0x94cf
    94fe:	04 e8                	add    $0xe8,%al
    9500:	d8 05                	fadds  (%di)
    9502:	24 08                	and    $0x8,%al
    9504:	8b ba 1e 5f          	mov    0x5f1e(%bp,%si),%di
    9508:	5e                   	pop    %si
    9509:	2b 00                	sub    (%bx,%si),%ax
    950b:	33 08                	xor    (%bx,%si),%cx
    950d:	b9 28 e8             	mov    $0xe828,%cx
    9510:	40                   	inc    %ax
    9511:	c0 1e 0c 06 4a       	rcrb   $0x4a,0x60c
    9516:	76 fc                	jbe    0x9514
    9518:	05 0b bb             	add    $0xbb0b,%ax
    951b:	e9 00 0a             	jmp    0x9f1e
    951e:	8b 07                	mov    (%bx),%ax
    9520:	8b 7f 9e             	mov    -0x62(%bx),%di
    9523:	b4 8b                	mov    $0x8b,%ah
    9525:	05 08 f1             	add    $0xf108,%ax
    9528:	03 03                	add    (%bp,%di),%ax
    952a:	0e                   	push   %cs
    952b:	f7 4e 1f 03 8e       	testw  $0x8e03,0x1f(%bp)
    9530:	96                   	xchg   %ax,%si
    9531:	03 8e 96 03          	add    0x396(%bp),%cx
    9535:	ff f4                	push   %sp
    9537:	03 e0                	add    %ax,%sp
    9539:	03 5e 0c             	add    0xc(%bp),%bx
    953c:	47                   	inc    %di
    953d:	3b 7d 46             	cmp    0x46(%di),%di
    9540:	07                   	pop    %es
    9541:	48                   	dec    %ax
    9542:	47                   	inc    %di
    9543:	8b 9e 8b 47          	mov    0x478b(%bp),%bx
    9547:	06                   	push   %es
    9548:	9e                   	sahf
    9549:	77 2b                	ja     0x9576
    954b:	a1 37 81             	mov    0x8137,%ax
    954e:	08 0a                	or     %cl,(%bp,%si)
    9550:	3b 57 02             	cmp    0x2(%bx),%dx
    9553:	03 f2                	add    %dx,%si
    9555:	03 f2                	add    %dx,%si
    9557:	89 76 f6             	mov    %si,-0xa(%bp)
    955a:	8b 34                	mov    (%si),%si
    955c:	03 f1                	add    %cx,%si
    955e:	8b 47 06             	mov    0x6(%bx),%ax
    9561:	2b c2                	sub    %dx,%ax
    9563:	40                   	inc    %ax
    9564:	eb 46                	jmp    0x95ac
    9566:	7e 46                	jle    0x95ae
    9568:	0f 3b                	(bad)
    956a:	57                   	push   %di
    956b:	8e 1c                	mov    (%si),%ds
    956d:	5e                   	pop    %si
    956e:	9f                   	lahf
    956f:	9b                   	fwait
    9570:	e3 87                	jcxz   0x94f9
    9572:	94                   	xchg   %ax,%sp
    9573:	0f 01 5e 06          	lidtw  0x6(%bp)
    9577:	e0 05                	loopne 0x957e
    9579:	08 07                	or     %al,(%bx)
    957b:	48                   	dec    %ax
    957c:	8b 05                	mov    (%di),%ax
    957e:	07                   	pop    %es
    957f:	c1 b1 4b 88 7e       	shlw   $0x7e,-0x77b5(%bx,%di)
    9584:	fc                   	cld
    9585:	88 7e fe             	mov    %bh,-0x2(%bp)
    9588:	d3 f8                	sar    %cl,%ax
    958a:	03 f0                	add    %ax,%si
    958c:	d3 fb                	sar    %cl,%bx
    958e:	03 fb                	add    %bx,%di
    9590:	03 2b                	add    (%bp,%di),%bp
    9592:	7e fe                	jle    0x9592
    9594:	42                   	inc    %dx
    9595:	01 f7                	add    %si,%di
    9597:	9a ad e0 d3 0e       	lcall  $0xed3,$0xe0ad
    959c:	8b d3                	mov    %bx,%dx
    959e:	77 fc                	ja     0x959c
    95a0:	40                   	inc    %ax
    95a1:	2b c1                	sub    %cx,%ax
    95a3:	ec                   	in     (%dx),%al
    95a4:	07                   	pop    %es
    95a5:	88 2b                	mov    %ch,(%bp,%di)
    95a7:	d9 8b 9a 41          	(bad) 0x419a(%bp,%di)
    95ab:	ca 74 56             	lret   $0x5674
    95ae:	eb f0                	jmp    0x95a0
    95b0:	03 fb                	add    %bx,%di
    95b2:	ff 4e f8             	decw   -0x8(%bp)
    95b5:	75 f3                	jne    0x95aa
    95b7:	9a ad 24 4f 25       	lcall  $0x254f,$0x24ad
    95bc:	7e 2d                	jle    0x95eb
    95be:	fe                   	(bad)
    95bf:	28 08                	sub    %cl,(%bx,%si)
    95c1:	07                   	pop    %es
    95c2:	48                   	dec    %ax
    95c3:	8b 05                	mov    (%di),%ax
    95c5:	03 f1                	add    %cx,%si
    95c7:	03 06 00 2b          	add    0x2b00,%ax
    95cb:	d1 42 01             	rolw   $1,0x1(%bp,%si)
    95ce:	fc                   	cld
    95cf:	40                   	inc    %ax
    95d0:	2b c2                	sub    %dx,%ax
    95d2:	ec                   	in     (%dx),%al
    95d3:	07                   	pop    %es
    95d4:	88 2b                	mov    %ch,(%bp,%di)
    95d6:	da 8b 03 d2          	fimull -0x2dfd(%bp,%di)
    95da:	9a ca 56 f0 fb       	lcall  $0xfbf0,$0x56ca
    95df:	4e                   	dec    %si
    95e0:	75 9a                	jne    0x957c
    95e2:	24 25                	and    $0x25,%al
    95e4:	2d 28 07             	sub    $0x728,%ax
    95e7:	8b ba a6 f9          	mov    -0x65a(%bp,%si),%di
    95eb:	00 cd                	add    %cl,%ch
    95ed:	4a                   	dec    %dx
    95ee:	8b 04                	mov    (%si),%ax
    95f0:	1b 0a                	sbb    (%bp,%si),%cx
    95f2:	96                   	xchg   %ax,%si
    95f3:	0a 00                	or     (%bx,%si),%al
    95f5:	d1 89 ca 07          	rorw   $1,0x7ca(%bx,%di)
    95f9:	88 d1                	mov    %dl,%cl
    95fb:	f9                   	stc
    95fc:	89 fc                	mov    %di,%sp
    95fe:	fe                   	(bad)
    95ff:	d1 2b                	shrw   $1,(%bp,%di)
    9601:	75 9a                	jne    0x959d
    9603:	ca c2 40             	lret   $0x40c2
    9606:	ec                   	in     (%dx),%al
    9607:	07                   	pop    %es
    9608:	f6 8b 56 f4 c6       	testb  $0xc6,-0xbaa(%bp,%di)
    960d:	fa                   	cli
    960e:	da 8e d9 d3          	fimull -0x2c27(%bp)
    9612:	37                   	aaa
    9613:	f2 f7 76 fc          	repnz divw -0x4(%bp)
    9617:	87 da                	xchg   %bx,%dx
    9619:	0e                   	push   %cs
    961a:	3f                   	aas
    961b:	fe 03                	incb   (%bp,%di)
    961d:	7e fe                	jle    0x961d
    961f:	87 da                	xchg   %bx,%dx
    9621:	83 c2 02             	add    $0x2,%dx
    9624:	83 c4 5e             	add    $0x5e,%sp
    9627:	a3 f4 9a             	mov    %ax,0x9af4
    962a:	2e 25 2d 28          	cs and $0x282d,%ax
    962e:	07                   	pop    %es
    962f:	e1 74                	loope  0x96a5
    9631:	b6 28                	mov    $0x28,%dh
    9633:	40                   	inc    %ax
    9634:	1e                   	push   %ds
    9635:	81 ec a9 ff          	sub    $0xffa9,%sp
    9639:	7e 00                	jle    0x963b
    963b:	8c fc                	mov    %?,%sp
    963d:	0b e9                	or     %cx,%bp
    963f:	0a a1 74 06          	or     0x674(%bx,%di),%ah
    9643:	b8 04 0f             	mov    $0xf04,%ax
    9646:	02 ff                	add    %bh,%bh
    9648:	ff 8d 56 5e          	decw   0x5e56(%di)
    964c:	8b 96 47 e7          	mov    -0x18b9(%bp),%dx
    9650:	06                   	push   %es
    9651:	3b 02                	cmp    (%bp,%si),%ax
    9653:	f2 2b 40 46          	repnz sub 0x46(%bx,%si),%ax
    9657:	46                   	inc    %si
    9658:	3b 8e 5e 07          	cmp    0x75e(%bp),%cx
    965c:	f8                   	clc
    965d:	5b                   	pop    %bx
    965e:	fe                   	(bad)
    965f:	a2 88 8b             	mov    %al,0x8b88
    9662:	9a 4e d1 f9 2b       	lcall  $0x2bf9,$0xd14e
    9667:	c8 41 89 fa          	enter  $0x8941,$0xfa
    966b:	37                   	aaa
    966c:	d1 e2                	shl    $1,%dx
    966e:	8b 1e b6 9a          	mov    -0x654a,%bx
    9672:	03 da                	add    %dx,%bx
    9674:	ba df 03             	mov    $0x3df,%dx
    9677:	8a 08                	mov    (%bx,%si),%cl
    9679:	20 d0                	and    %dl,%al
    967b:	d0 e2                	shl    $1,%dl
    967d:	d0 e2                	shl    $1,%dl
    967f:	d0 0c                	rorb   $1,(%si)
    9681:	c6                   	(bad)
    9682:	ee                   	out    %al,(%dx)
    9683:	b8 00 b8             	mov    $0xb800,%ax
    9686:	8b 05                	mov    (%di),%ax
    9688:	d3 37                	shlw   %cl,(%bx)
    968a:	f2 f7 76 fe          	repnz divw -0x2(%bp)
    968e:	89 76 fc             	mov    %si,-0x4(%bp)
    9691:	8c d0                	mov    %ss,%ax
    9693:	04 8d                	add    $0x8d,%al
    9695:	eb be                	jmp    0x9655
    9697:	57                   	push   %di
    9698:	ff c6                	inc    %si
    969a:	fa                   	cli
    969b:	da 9a 2e ba          	ficompl -0x45d2(%bp,%si)
    969f:	df 03                	filds  (%bp,%di)
    96a1:	8a 08                	mov    (%bx,%si),%cl
    96a3:	06                   	push   %es
    96a4:	d0 e2                	shl    $1,%dl
    96a6:	e2 0c                	loop   0x96b4
    96a8:	ee                   	out    %al,(%dx)
    96a9:	00 8b d3 f2          	add    %cl,-0xd2d(%bp,%di)
    96ad:	8c c0                	mov    %es,%ax
    96af:	8b 05                	mov    (%di),%ax
    96b1:	8d b6 57 ff          	lea    -0xa9(%bp),%si
    96b5:	b8 00 b8             	mov    $0xb800,%ax
    96b8:	8b c0                	mov    %ax,%ax
    96ba:	e6 8b                	out    %al,$0x8b
    96bc:	d8 75 fa             	fdivs  -0x6(%di)
    96bf:	da 9a 2e 83          	ficompl -0x7cd2(%bp,%si)
    96c3:	c4 5e 25             	les    0x25(%bp),%bx
    96c6:	2d 28 07             	sub    $0x728,%ax
    96c9:	aa                   	stos   %al,%es:(%di)
    96ca:	ba df 03             	mov    $0x3df,%dx
    96cd:	b0 06                	mov    $0x6,%al
    96cf:	e2 0c                	loop   0x96dd
    96d1:	00 d3                	add    %dl,%bl
    96d3:	8c 8b 8d 57          	mov    %cs,0x578d(%bp,%di)
    96d7:	b8 ba a6             	mov    $0xa6ba,%ax
    96da:	f9                   	stc
    96db:	00 cd                	add    %cl,%ch
    96dd:	4a                   	dec    %dx
    96de:	8b 04                	mov    (%si),%ax
    96e0:	1b 0a                	sbb    (%bp,%si),%cx
    96e2:	96                   	xchg   %ax,%si
    96e3:	08 07                	or     %al,(%bx)
    96e5:	4c                   	dec    %sp
    96e6:	7e 06                	jle    0x96ee
    96e8:	c0 4a 36 9b          	rorb   $0x9b,0x36(%bp,%si)
    96ec:	b4 10                	mov    $0x10,%ah
    96ee:	81 0a 57 83          	orw    $0x8357,(%bp,%si)
    96f2:	65 e4 55             	gs in  $0x55,%al
    96f5:	02 75 0d             	add    0xd(%di),%dh
    96f8:	b0 10                	mov    $0x10,%al
    96fa:	8a 76 c6             	mov    -0x3a(%bp),%dh
    96fd:	01 6e 05             	add    %bp,0x5(%bp)
    9700:	8a 4e 04             	mov    0x4(%bp),%cl
    9703:	eb 05                	jmp    0x970a
    9705:	32 c0                	xor    %al,%al
    9707:	8a 7e 04             	mov    0x4(%bp),%bh
    970a:	cd 10                	int    $0x10
    970c:	c9                   	leave
    970d:	08 51 de             	or     %dl,-0x22(%bx,%di)
    9710:	fe                   	(bad)
    9711:	d0 88 06 06          	rorb   $1,0x606(%bx,%si)
    9715:	c0 52 06 8c          	rclb   $0x8c,0x6(%bp,%si)
    9719:	8e 8a 08 8b          	mov    -0x74f8(%bp,%si),%cs
    971d:	79 8a                	jns    0x96a9
    971f:	b4 10                	mov    $0x10,%ah
    9721:	b0 02                	mov    $0x2,%al
    9723:	cd 10                	int    $0x10
    9725:	07                   	pop    %es
    9726:	5a                   	pop    %dx
    9727:	87 32                	xchg   %si,(%bp,%si)
    9729:	e8 5c 05             	call   0x9c88
    972c:	57                   	push   %di
    972d:	76 9b                	jbe    0x96ca
    972f:	e8 12 76             	call   0xd44
    9732:	e7 8b                	out    %ax,$0x8b
    9734:	f0 56                	lock push %si
    9736:	8c 8e 8a 08          	mov    %cs,0x88a(%bp)
    973a:	e8 7e ed             	call   0x84bb
    973d:	be de 03             	mov    $0x3de,%si
    9740:	8d 7e ee             	lea    -0x12(%bp),%di
    9743:	b9 11 00             	mov    $0x11,%cx
    9746:	9a 2e 02 c6 d2       	lcall  $0xd2c6,$0x22e
    974b:	bf de 03             	mov    $0x3de,%di
    974e:	b9 00 2e             	mov    $0x2e00,%cx
    9751:	c6                   	(bad)
    9752:	bf 03 79             	mov    $0x7903,%di
    9755:	b4 b0                	mov    $0xb0,%ah
    9757:	cd 07                	int    $0x7
    9759:	87 e8                	xchg   %bp,%ax
    975b:	05 8d 76             	add    $0x768d,%ax
    975e:	ee                   	out    %al,(%dx)
    975f:	bf 03 00             	mov    $0x3,%di
    9762:	c6 03 b4             	movb   $0xb4,(%bp,%di)
    9765:	cd 87                	int    $0x87
    9767:	5e                   	pop    %si
    9768:	5f                   	pop    %di
    9769:	06                   	push   %es
    976a:	04 05                	add    $0x5,%al
    976c:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    976d:	87 32                	xchg   %si,(%bp,%si)
    976f:	e8 5c 05             	call   0x9cce
    9772:	57                   	push   %di
    9773:	76 9b                	jbe    0x9710
    9775:	47                   	inc    %di
    9776:	4f                   	dec    %di
    9777:	a1 8b 8c             	mov    0x8c8b,%ax
    977a:	8a 8b 8a 10          	mov    0x108a(%bp,%di),%cl
    977e:	02 10                	add    (%bx,%si),%dl
    9780:	bb 07 4c             	mov    $0x4c07,%bx
    9783:	b9 01 fe             	mov    $0xfe01,%cx
    9786:	b0 cd                	mov    $0xcd,%al
    9788:	07                   	pop    %es
    9789:	12 cd                	adc    %ch,%cl
    978b:	10 07                	adc    %al,(%bx)
    978d:	5a                   	pop    %dx
    978e:	8b 1e 02 b4          	mov    -0x4bfe,%bx
    9792:	07                   	pop    %es
    9793:	1e                   	push   %ds
    9794:	b4 00                	mov    $0x0,%ah
    9796:	b0 93                	mov    $0x93,%al
    9798:	cd c3                	int    $0xc3
    979a:	0f 51 06 15 68       	sqrtps 0x6815,%xmm0
    979f:	30 52 a6             	xor    %dl,-0x5a(%bp,%si)
    97a2:	e8 87 ff             	call   0x972c
    97a5:	05 52 2b             	add    $0x2b52,%ax
    97a8:	4a                   	dec    %dx
    97a9:	01 5f ac             	add    %bx,-0x54(%bx)
    97ac:	de 82 50 e8          	fiadds -0x17b0(%bp,%si)
    97b0:	04 00                	add    $0x0,%al
    97b2:	2b 00                	sub    (%bx,%si),%ax
    97b4:	33 08                	xor    (%bx,%si),%cx
    97b6:	03 07                	add    (%bx),%ax
    97b8:	8d                   	lea    (bad),%di
    97b9:	ff 08                	decw   (%bx,%si)
    97bb:	96                   	xchg   %ax,%si
    97bc:	8a 06 8b 46          	mov    0x468b,%al
    97c0:	04 a3                	add    $0xa3,%al
    97c2:	88 a0 02 c6          	mov    %ah,-0x39fe(%bx,%si)
    97c6:	0c 8b                	or     $0x8b,%al
    97c8:	7e 0a                	jle    0x97d4
    97ca:	ba 01 56             	mov    $0x5601,%dx
    97cd:	5e                   	pop    %si
    97ce:	8b 9a 2b de          	mov    -0x21d5(%bp,%si),%bx
    97d2:	0b 9c 03 04          	or     0x403(%si),%bx
    97d6:	f7 da                	neg    %dx
    97d8:	f7 db                	neg    %bx
    97da:	b9 01 56             	mov    $0x5601,%cx
    97dd:	5e                   	pop    %si
    97de:	46                   	inc    %si
    97df:	2b 02                	sub    (%bp,%si),%ax
    97e1:	c7                   	(bad)
    97e2:	0b c0                	or     %ax,%ax
    97e4:	03 04                	add    (%si),%ax
    97e6:	f7 d9                	neg    %cx
    97e8:	f7 d8                	neg    %ax
    97ea:	8b e9                	mov    %cx,%bp
    97ec:	0b bb e9 15          	or     0x15e9(%bp,%di),%di
    97f0:	03 06 a9 01          	add    0x1a9,%ax
    97f4:	e2 80                	loop   0x9776
    97f6:	00 c8                	add    %cl,%al
    97f8:	2b ff                	sub    %di,%di
    97fa:	08 ff                	or     %bh,%bh
    97fc:	16                   	push   %ss
    97fd:	88 a0 03 e9          	mov    %ah,-0x16fd(%bx,%si)
    9801:	eb f6                	jmp    0x97f9
    9803:	eb 72                	jmp    0x9877
    9805:	03 06 a9 0e          	add    0xea9,%ax
    9809:	8b cb                	mov    %bx,%cx
    980b:	ff                   	(bad)
    980c:	ff 88 03 eb          	decw   -0x14fd(%bx,%si)
    9810:	eb 03                	jmp    0x9815
    9812:	f2 eb f6             	bnd jmp 0x980b
    9815:	eb 60                	jmp    0x9877
    9817:	8b cb                	mov    %bx,%cx
    9819:	3b c8                	cmp    %ax,%cx
    981b:	7e 2c                	jle    0x9849
    981d:	d1 e0                	shl    $1,%ax
    981f:	a3 8a a0             	mov    %ax,0xa08a
    9822:	2b ec                	sub    %sp,%bp
    9824:	88 d8                	mov    %bl,%al
    9826:	2b ec                	sub    %sp,%bp
    9828:	a3 8c a0             	mov    %ax,0xa08c
    982b:	ff 88 eb 03          	decw   0x3eb(%bx,%si)
    982f:	eb eb                	jmp    0x981c
    9831:	8b 3b                	mov    (%bp,%di),%di
    9833:	0b 9c 03 03          	or     0x303(%si),%bx
    9837:	e2 eb                	loop   0x9824
    9839:	8a a0 e2 ee          	mov    -0x111e(%bx,%si),%ah
    983d:	eb 38                	jmp    0x9877
    983f:	03 9c e2 04          	add    0x4e2(%si),%bx
    9843:	8c a0 e2 e4          	mov    %fs,-0x1b1e(%bx,%si)
    9847:	eb 2e                	jmp    0x9877
    9849:	d1 ca                	ror    $1,%dx
    984b:	9c                   	pushf
    984c:	03 8a a0 2b          	add    0x2ba0(%bp,%si),%cx
    9850:	c8 fe ba 9a          	enter  $0xbafe,$0x9a
    9854:	c8 9c 03 8c          	enter  $0x39c,$0x8c
    9858:	a0 c8 ff             	mov    0xffc8,%al
    985b:	ff 88 03 eb          	decw   -0x14fd(%bx,%si)
    985f:	eb 03                	jmp    0x9864
    9861:	a9 8b 0b             	test   $0xb8b,%ax
    9864:	03 e2                	add    %dx,%sp
    9866:	8a e2                	mov    %dl,%ah
    9868:	eb 03                	jmp    0x986d
    986a:	e2 8c                	loop   0x97f8
    986c:	e2 eb                	loop   0x9859
    986e:	8a e2                	mov    %dl,%ah
    9870:	f2 e2 8c             	repnz loop 0x97ff
    9873:	e2 eb                	loop   0x9860
    9875:	d1 9c ff 03          	rcrw   $1,0x3ff(%si)
    9879:	eb a9                	jmp    0x9824
    987b:	0b e2                	or     %dx,%sp
    987d:	8b 1e 5e 00          	mov    0x5e,%bx
    9881:	08 28                	or     %ch,(%bx,%si)
    9883:	40                   	inc    %ax
    9884:	ff                   	(bad)
    9885:	7a 8a                	jp     0x9811
    9887:	5e                   	pop    %si
    9888:	03 07                	add    (%bx),%ax
    988a:	b8 4e 05             	mov    $0x54e,%ax
    988d:	2b 01                	sub    (%bx,%di),%ax
    988f:	ac                   	lods   %ds:(%si),%al
    9890:	82 e8 8d             	sub    $0x8d,%al
    9893:	06                   	push   %es
    9894:	05 9a 50             	add    $0x509a,%ax
    9897:	e8 40 00             	call   0x98da
    989a:	2b 00                	sub    (%bx,%si),%ax
    989c:	d0 06 c0 06          	rolb   $1,0x6c0
    98a0:	8e ac de 82          	mov    -0x7d22(%si),%gs
    98a4:	a3 8e a0             	mov    %ax,0xa08e
    98a7:	07                   	pop    %es
    98a8:	4e                   	dec    %si
    98a9:	2b ac e8 06          	sub    0x6e8(%si),%bp
    98ad:	9a e8 00 00 06       	lcall  $0x600,$0xe8
    98b2:	3e 99                	ds cwtd
    98b4:	ff 22                	jmp    *(%bp,%si)
    98b6:	9f                   	lahf
    98b7:	00 00                	add    %al,(%bx,%si)
    98b9:	06                   	push   %es
    98ba:	06                   	push   %es
    98bb:	ac                   	lods   %ds:(%si),%al
    98bc:	82 8e 07 8b 04       	orb    $0x4,-0x74f9(%bp)
    98c1:	88 02                	mov    %al,(%bp,%si)
    98c3:	8e a0 07 4e          	mov    0x4e07(%bx,%si),%fs
    98c7:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    98c8:	87 05                	xchg   %ax,(%di)
    98ca:	2b 01                	sub    (%bx,%di),%ax
    98cc:	ac                   	lods   %ds:(%si),%al
    98cd:	82 00 3e             	addb   $0x3e,(%bx,%si)
    98d0:	ff 9f 00 06          	lcall  *0x600(%bx)
    98d4:	04 2b                	add    $0x2b,%al
    98d6:	33 03                	xor    (%bp,%di),%ax
    98d8:	8d 08                	lea    (%bx,%si),%cx
    98da:	8a 8b 04 88          	mov    -0x77fc(%bp,%di),%cl
    98de:	02 0c                	add    (%si),%cl
    98e0:	7e ba                	jle    0x989c
    98e2:	bb b0 a0             	mov    $0xa0b0,%bx
    98e5:	c8 7e 46 07          	enter  $0x467e,$0x7
    98e9:	07                   	pop    %es
    98ea:	ff 08                	decw   (%bx,%si)
    98ec:	7a 47                	jp     0x9935
    98ee:	04 7e                	add    $0x7e,%al
    98f0:	33 f6                	xor    %si,%si
    98f2:	8b 76 eb             	mov    -0x15(%bp),%si
    98f5:	ba 03 00             	mov    $0x3,%dx
    98f8:	2b d7                	sub    %di,%dx
    98fa:	2b d7                	sub    %di,%dx
    98fc:	c0 4a 36 ff          	rorb   $0xff,0x36(%bp,%si)
    9900:	56                   	push   %si
    9901:	04 5a                	add    $0x5a,%al
    9903:	0b d2                	or     %dx,%dx
    9905:	7d 12                	jge    0x9919
    9907:	83 c2 05             	add    $0x5,%dx
    990a:	75 41                	jne    0x994d
    990c:	03 e0                	add    %ax,%sp
    990e:	e0 0a                	loopne 0x991a
    9910:	c0 46 d6 3b          	rolb   $0x3b,-0x2a(%bp)
    9914:	f7 7c e5             	idivw  -0x1b(%si)
    9917:	7d 13                	jge    0x992c
    9919:	83 c2 0a             	add    $0xa,%dx
    991c:	75 41                	jne    0x995f
    991e:	2b c7                	sub    %di,%ax
    9920:	03 e0                	add    %ax,%sp
    9922:	c0 d6 f7             	rcl    $0xf7,%dh
    9925:	e5 4f                	in     $0x4f,%ax
    9927:	d6                   	(bad)
    9928:	3b f7                	cmp    %di,%si
    992a:	7c d0                	jl     0x98fc
    992c:	3b f7                	cmp    %di,%si
    992e:	75 05                	jne    0x9935
    9930:	4a                   	dec    %dx
    9931:	ff 04                	incw   (%si)
    9933:	0b 7d 83             	or     -0x7d(%di),%di
    9936:	ba 5f 2b             	mov    $0x2b5f,%dx
    9939:	33 b9 e8 c0          	xor    -0x3f18(%bx,%di),%di
    993d:	0c 52                	or     $0x52,%al
    993f:	e8 f0 53             	call   0xed32
    9942:	cd 76                	int    $0x76
    9944:	8b 02                	mov    (%bp,%si),%ax
    9946:	96                   	xchg   %ax,%si
    9947:	8b 02                	mov    (%bp,%si),%ax
    9949:	33 89 f0 46          	xor    0x46f0(%bx,%di),%cx
    994d:	fe                   	(bad)
    994e:	fe c0                	inc    %al
    9950:	8b 45 fc             	mov    -0x4(%di),%ax
    9953:	bb b0 a0             	mov    $0xa0b0,%bx
    9956:	c8 37 b4 05          	enter  $0xb437,$0x5
    995a:	f1                   	int1
    995b:	03 46 8b             	add    -0x75(%bp),%ax
    995e:	87 4e 04             	xchg   %cx,0x4(%bp)
    9961:	db c0                	fcmovnb %st(0),%st
    9963:	8b d7                	mov    %di,%dx
    9965:	03 d1                	add    %cx,%dx
    9967:	89 97 a0 a0          	mov    %dx,-0x5f60(%bx)
    996b:	c8 d6 f7 e5          	enter  $0xf7d6,$0xe5
    996f:	89 97 90 a0          	mov    %dx,-0x5f70(%bx)
    9973:	07                   	pop    %es
    9974:	46                   	inc    %si
    9975:	fe 83 5e 2d          	incb   0x2d5e(%bp,%di)
    9979:	0b 74 26             	or     0x26(%si),%si
    997c:	3c 89                	cmp    $0x89,%al
    997e:	97                   	xchg   %ax,%di
    997f:	90                   	nop
    9980:	a0 8b d7             	mov    0xd78b,%al
    9983:	03 9a 0f 97          	add    -0x68f1(%bp,%si),%bx
    9987:	a0 a0 07             	mov    0x7a0,%al
    998a:	fe                   	(bad)
    998b:	5e                   	pop    %si
    998c:	0b 26 89 90          	or     -0x6f77,%sp
    9990:	c0 74 40 89          	shlb   $0x89,0x40(%si)
    9994:	a0 c8 f7             	mov    0xf7c8,%al
    9997:	89 90 2b 89          	mov    %dx,-0x76d5(%bx,%si)
    999b:	cc                   	int3
    999c:	97                   	xchg   %ax,%di
    999d:	a0 46 83             	mov    0x8346,%al
    99a0:	2d 74 3c             	sub    $0x3c74,%ax
    99a3:	97                   	xchg   %ax,%di
    99a4:	a0 89 90             	mov    0x9089,%al
    99a7:	8b 03                	mov    (%bp,%di),%ax
    99a9:	0f a0                	push   %fs
    99ab:	03 89 a0 c8          	add    -0x3760(%bx,%di),%cx
    99af:	f7 89 83 74 97 89    	testw  $0x8997,0x7483(%bx,%di)
    99b5:	8b 0f                	mov    (%bx),%cx
    99b7:	eb 1a                	jmp    0x99d3
    99b9:	90                   	nop
    99ba:	c0 74 16 89          	shlb   $0x89,0x16(%si)
    99be:	2b cc                	sub    %sp,%cx
    99c0:	a0 83 74             	mov    0x7483,%al
    99c3:	97                   	xchg   %ax,%di
    99c4:	89 8b 03 0f          	mov    %cx,0xf03(%bp,%di)
    99c8:	a0 07 5e             	mov    0x5e07,%al
    99cb:	26 90                	es nop
    99cd:	74 89                	je     0x9958
    99cf:	c8 89 2b cc          	enter  $0x2b89,$0xcc
    99d3:	8a 56 fc             	mov    -0x4(%bp),%dl
    99d6:	0a d2                	or     %dl,%dl
    99d8:	74 08                	je     0x99e2
    99da:	91                   	xchg   %ax,%cx
    99db:	32 d2                	xor    %dl,%dl
    99dd:	88 56 fc             	mov    %dl,-0x4(%bp)
    99e0:	eb 81                	jmp    0x9963
    99e2:	87 4e fe             	xchg   %cx,-0x2(%bp)
    99e5:	51                   	push   %cx
    99e6:	8b d9                	mov    %cx,%bx
    99e8:	4b                   	dec    %bx
    99e9:	d1 ab 9e ab          	shrw   $1,-0x5462(%bp,%di)
    99ed:	a0 46 83             	mov    0x8346,%al
    99f0:	b7 26                	mov    $0x26,%bh
    99f2:	90                   	nop
    99f3:	74 16                	je     0x9a0b
    99f5:	8e a0 59 e2          	mov    -0x1da7(%bx,%si),%fs
    99f9:	eb 52                	jmp    0x9a4d
    99fb:	f9                   	stc
    99fc:	c9                   	leave
    99fd:	00 08                	add    %cl,(%bx,%si)
    99ff:	cd 76                	int    $0x76
    9a01:	4a                   	dec    %dx
    9a02:	0e                   	push   %cs
    9a03:	56                   	push   %si
    9a04:	01 02                	add    %ax,(%bp,%si)
    9a06:	5d                   	pop    %bp
    9a07:	88 f7                	mov    %dh,%bh
    9a09:	06                   	push   %es
    9a0a:	02 8b 02 96          	add    -0x69fe(%bp,%di),%cl
    9a0e:	8b 02                	mov    (%bp,%si),%ax
    9a10:	b0 01                	mov    $0x1,%al
    9a12:	8b 45 fe             	mov    -0x2(%di),%ax
    9a15:	bb a0 37             	mov    $0x37a0,%bx
    9a18:	05 03 8b             	add    $0x8b03,%ax
    9a1b:	4e                   	dec    %si
    9a1c:	db 8b 4e 8b          	fisttpl -0x74b2(%bp,%di)
    9a20:	0e                   	push   %cs
    9a21:	0b 7d 8b             	or     -0x75(%di),%di
    9a24:	c6                   	(bad)
    9a25:	88 2b                	mov    %ch,(%bp,%di)
    9a27:	a3 a0 50             	mov    %ax,0x50a0
    9a2a:	8b c7                	mov    %di,%ax
    9a2c:	03 c2                	add    %dx,%ax
    9a2e:	50                   	push   %ax
    9a2f:	03 d9                	add    %cx,%bx
    9a31:	53                   	push   %bx
    9a32:	e8 95 e4             	call   0x7eca
    9a35:	8b 88 a3 50          	mov    0x50a3(%bx,%si),%cx
    9a39:	c7 c2 03 53          	mov    $0x5303,%dx
    9a3d:	95                   	xchg   %ax,%bp
    9a3e:	2b c2                	sub    %dx,%ax
    9a40:	03 53 95             	add    -0x6b(%bp,%di),%dx
    9a43:	8b a3 83 e4          	mov    -0x1b7d(%bp,%di),%sp
    9a47:	8a 45 fe             	mov    -0x2(%di),%al
    9a4a:	0e                   	push   %cs
    9a4b:	fe 8b 09 87          	decb   -0x78f7(%bp,%di)
    9a4f:	ca 32 46             	lret   $0x4632
    9a52:	ca 8b fe             	lret   $0xfe8b
    9a55:	eb cc                	jmp    0x9a23
    9a57:	52                   	push   %dx
    9a58:	c9                   	leave
    9a59:	08 76 0e             	or     %dh,0xe(%bp)
    9a5c:	01 5d f7             	add    %bx,-0x9(%di)
    9a5f:	02 02                	add    (%bp,%si),%al
    9a61:	8b b0 8b fe          	mov    -0x175(%bx,%si),%si
    9a65:	4a                   	dec    %dx
    9a66:	9b                   	fwait
    9a67:	10 0a                	adc    %cl,(%bp,%si)
    9a69:	40                   	inc    %ax
    9a6a:	bb 00 15             	mov    $0x1500,%bx
    9a6d:	87 04                	xchg   %ax,(%si)
    9a6f:	c0 8b 2a e9 cd       	rorb   $0xcd,-0x16d6(%bp,%di)
    9a74:	21 06 04 05          	and    %ax,0x504
    9a78:	ff                   	(bad)
    9a79:	7e 00                	jle    0x9a7b
    9a7b:	8c b4 30 cd          	mov    %?,-0x32d0(%si)
    9a7f:	21 3c                	and    %di,(%si)
    9a81:	02 73 05             	add    0x5(%bp,%di),%dh
    9a84:	33 c0                	xor    %ax,%ax
    9a86:	06                   	push   %es
    9a87:	50                   	push   %ax
    9a88:	cb                   	lret
    9a89:	bf 63 0e             	mov    $0xe63,%di
    9a8c:	8b 36 02 00          	mov    0x2,%si
    9a90:	2b f7                	sub    %di,%si
    9a92:	81 fe 00 10          	cmp    $0x1000,%si
    9a96:	72 03                	jb     0x9a9b
    9a98:	be 00 10             	mov    $0x1000,%si
    9a9b:	fa                   	cli
    9a9c:	8e d7                	mov    %di,%ss
    9a9e:	81 c4 0e c4          	add    $0xc40e,%sp
    9aa2:	fb                   	sti
    9aa3:	73 10                	jae    0x9ab5
    9aa5:	16                   	push   %ss
    9aa6:	1f                   	pop    %ds
    9aa7:	e8 d4 07             	call   0xa27e
    9aaa:	33 c0                	xor    %ax,%ax
    9aac:	50                   	push   %ax
    9aad:	e8 99 07             	call   0xa249
    9ab0:	12 d3                	adc    %bl,%dl
    9ab2:	4c                   	dec    %sp
    9ab3:	cd 8b                	int    $0x8b
    9ab5:	52                   	push   %dx
    9ab6:	c6                   	(bad)
    9ab7:	b1 d3                	mov    $0xd3,%cl
    9ab9:	b0 e0                	mov    $0xe0,%al
    9abb:	48                   	dec    %ax
    9abc:	36 a3 c2 a3          	mov    %ax,%ss:0xa3c2
    9ac0:	bb c4 a3             	mov    $0xa3c4,%bx
    9ac3:	36 8c 17             	mov    %ss,%ss:(%bx)
    9ac6:	83 e4 fe             	and    $0xfffe,%sp
    9ac9:	36 89 67 04          	mov    %sp,%ss:0x4(%bx)
    9acd:	b8 fe ff             	mov    $0xfffe,%ax
    9ad0:	50                   	push   %ax
    9ad1:	36 89 67 0a          	mov    %sp,%ss:0xa(%bx)
    9ad5:	f7 d0                	not    %ax
    9ad7:	50                   	push   %ax
    9ad8:	36 89 67 06          	mov    %sp,%ss:0x6(%bx)
    9adc:	36 89 67 08          	mov    %sp,%ss:0x8(%bx)
    9ae0:	36 89 26 be a3       	mov    %sp,%ss:-0x5c42
    9ae5:	03 f7                	add    %di,%si
    9ae7:	89 36 02 00          	mov    %si,0x2
    9aeb:	8c c3                	mov    %es,%bx
    9aed:	2b de                	sub    %si,%bx
    9aef:	f7 db                	neg    %bx
    9af1:	b4 4a                	mov    $0x4a,%ah
    9af3:	cd 8b                	int    $0x8b
    9af5:	36 8c 1e fe a3       	mov    %ds,%ss:-0x5c02
    9afa:	16                   	push   %ss
    9afb:	07                   	pop    %es
    9afc:	fc                   	cld
    9afd:	bf 4c a6             	mov    $0xa64c,%di
    9b00:	b9 10 c4             	mov    $0xc410,%cx
    9b03:	2b cf                	sub    %di,%cx
    9b05:	33 c0                	xor    %ax,%ax
    9b07:	f3 aa                	rep stos %al,%es:(%di)
    9b09:	16                   	push   %ss
    9b0a:	1f                   	pop    %ds
    9b0b:	8b 0e 4c a5          	mov    -0x5ab4,%cx
    9b0f:	e3 02                	jcxz   0x9b13
    9b11:	ff d1                	call   *%cx
    9b13:	e8 16 09             	call   0xa42c
    9b16:	e8 85 07             	call   0xa29e
    9b19:	33 ed                	xor    %bp,%bp
    9b1b:	e8 48 00             	call   0x9b66
    9b1e:	16                   	push   %ss
    9b1f:	1f                   	pop    %ds
    9b20:	ff 36 1f a4          	push   -0x5be1
    9b24:	ff 36 1d a4          	push   -0x5be3
    9b28:	ff 36 1b a4          	push   -0x5be5
    9b2c:	e8 6f 6c             	call   0x79e
    9b2f:	50                   	push   %ax
    9b30:	e8 01 01             	call   0x9c34
    9b33:	c3                   	ret
    9b34:	2e a1 63 9b          	mov    %cs:0x9b63,%ax
    9b38:	8e d8                	mov    %ax,%ds
    9b3a:	ef                   	out    %ax,(%dx)
    9b3b:	08 04                	or     %al,(%si)
    9b3d:	36 c7 06 c0 a3 34 9c 	movw   $0x9c34,%ss:-0x5c40
    9b44:	50                   	push   %ax
    9b45:	e8 36 07             	call   0xa27e
    9b48:	e8 fe 06             	call   0xa249
    9b4b:	36 81 3e 4e a5 d6 d6 	cmpw   $0xd6d6,%ss:-0x5ab2
    9b52:	75 07                	jne    0x9b5b
    9b54:	58                   	pop    %ax
    9b55:	50                   	push   %ax
    9b56:	36 ff 16 52 a5       	call   *%ss:-0x5aae
    9b5b:	5c                   	pop    %sp
    9b5c:	36 f8                	ss clc
    9b5e:	50                   	push   %ax
    9b5f:	ff 16 c0 a3          	call   *-0x5c40
    9b63:	63 0e 00 b8          	arpl   %cx,-0x4800
    9b67:	00 35                	add    %dh,(%di)
    9b69:	cd 8b                	int    $0x8b
    9b6b:	89 1e ea a3          	mov    %bx,-0x5c16
    9b6f:	8c 06 ec a3          	mov    %es,-0x5c14
    9b73:	0e                   	push   %cs
    9b74:	1f                   	pop    %ds
    9b75:	b8 00 25             	mov    $0x2500,%ax
    9b78:	ba 34 9b             	mov    $0x9b34,%dx
    9b7b:	cd 8b                	int    $0x8b
    9b7d:	16                   	push   %ss
    9b7e:	1f                   	pop    %ds
    9b7f:	83 3e 5e a5 00       	cmpw   $0x0,-0x5aa2
    9b84:	74 36                	je     0x9bbc
    9b86:	8c 0e 60 a5          	mov    %cs,-0x5aa0
    9b8a:	8c 0e 68 a5          	mov    %cs,-0x5a98
    9b8e:	8e 06 fe a3          	mov    -0x5c02,%es
    9b92:	26 8b 36 2c 00       	mov    %es:0x2c,%si
    9b97:	c5 06 62 a5          	lds    -0x5a9e,%ax
    9b9b:	8c da                	mov    %ds,%dx
    9b9d:	33 db                	xor    %bx,%bx
    9b9f:	36 ff 1e 5e a5       	lcall  *%ss:-0x5aa2
    9ba4:	73 05                	jae    0x9bab
    9ba6:	16                   	push   %ss
    9ba7:	1f                   	pop    %ds
    9ba8:	e9 ff 08             	jmp    0xa4aa
    9bab:	36 c5 06 66 a5       	lds    %ss:-0x5a9a,%ax
    9bb0:	8c da                	mov    %ds,%dx
    9bb2:	bb 08 04             	mov    $0x408,%bx
    9bb5:	36 ff 1e 5e a5       	lcall  *%ss:-0x5aa2
    9bba:	16                   	push   %ss
    9bbb:	1f                   	pop    %ds
    9bbc:	8e fe                	mov    %si,%?
    9bbe:	26 36 00 06 0e 2c    	es add %al,%ss:0x2c0e
    9bc4:	00 e3                	add    %ah,%bl
    9bc6:	3e 8e c1             	ds mov %cx,%es
    9bc9:	33 ff                	xor    %di,%di
    9bcb:	26 80 3d 00          	cmpb   $0x0,%es:(%di)
    9bcf:	74 34                	je     0x9c05
    9bd1:	b9 0d 00             	mov    $0xd,%cx
    9bd4:	be dc a3             	mov    $0xa3dc,%si
    9bd7:	f3 a6                	repz cmpsb %es:(%di),%ds:(%si)
    9bd9:	74 0b                	je     0x9be6
    9bdb:	b9 ff 7f             	mov    $0x7fff,%cx
    9bde:	33 c0                	xor    %ax,%ax
    9be0:	f2 ae                	repnz scas %es:(%di),%al
    9be2:	75 21                	jne    0x9c05
    9be4:	eb e5                	jmp    0x9bcb
    9be6:	06                   	push   %es
    9be7:	1e                   	push   %ds
    9be8:	07                   	pop    %es
    9be9:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    9bea:	8b f7                	mov    %di,%si
    9bec:	bf 07 a4             	mov    $0xa407,%di
    9bef:	b1 04                	mov    $0x4,%cl
    9bf1:	ac                   	lods   %ds:(%si),%al
    9bf2:	2c 41                	sub    $0x41,%al
    9bf4:	72 0d                	jb     0x9c03
    9bf6:	d2 e0                	shl    %cl,%al
    9bf8:	92                   	xchg   %ax,%dx
    9bf9:	ac                   	lods   %ds:(%si),%al
    9bfa:	2c 41                	sub    $0x41,%al
    9bfc:	72 05                	jb     0x9c03
    9bfe:	0a c2                	or     %dl,%al
    9c00:	aa                   	stos   %al,%es:(%di)
    9c01:	eb ee                	jmp    0x9bf1
    9c03:	16                   	push   %ss
    9c04:	1f                   	pop    %ds
    9c05:	bb 04 00             	mov    $0x4,%bx
    9c08:	80 a7 07 a4 bf       	andb   $0xbf,-0x5bf9(%bx)
    9c0d:	b8 00 44             	mov    $0x4400,%ax
    9c10:	cd 72                	int    $0x72
    9c12:	e7 0a                	out    %ax,$0xa
    9c14:	f6 c2 80             	test   $0x80,%dl
    9c17:	74 05                	je     0x9c1e
    9c19:	80 8f 07 a4 40       	orb    $0x40,-0x5bf9(%bx)
    9c1e:	4b                   	dec    %bx
    9c1f:	79 e7                	jns    0x9c08
    9c21:	be 6a a5             	mov    $0xa56a,%si
    9c24:	bf 6a a5             	mov    $0xa56a,%di
    9c27:	e8 c7 00             	call   0x9cf1
    9c2a:	be 6a bf             	mov    $0xbf6a,%si
    9c2d:	a5                   	movsw  %ds:(%si),%es:(%di)
    9c2e:	c7                   	(bad)
    9c2f:	be bf af             	mov    $0xafbf,%si
    9c32:	00 73 7e             	add    %dh,0x7e(%bp,%di)
    9c35:	56                   	push   %si
    9c36:	57                   	push   %di
    9c37:	33 c9                	xor    %cx,%cx
    9c39:	eb 1a                	jmp    0x9c55
    9c3b:	7e 56                	jle    0x9c93
    9c3d:	57                   	push   %di
    9c3e:	b9 56 46             	mov    $0x4656,%cx
    9c41:	eb 12                	jmp    0x9c55
    9c43:	8a 04                	mov    (%si),%al
    9c45:	02 7e bb             	add    -0x45(%bp),%bh
    9c48:	b9 fe cd             	mov    $0xcdfe,%cx
    9c4b:	eb 08                	jmp    0x9c55
    9c4d:	8a 02                	mov    (%bp,%si),%al
    9c4f:	bb fe eb             	mov    $0xebfe,%bx
    9c52:	8a 01                	mov    (%bx,%di),%al
    9c54:	01 51 0a             	add    %dx,0xa(%bx,%di)
    9c57:	75 22                	jne    0x9c7b
    9c59:	1e                   	push   %ds
    9c5a:	be 60 ac             	mov    $0xac60,%si
    9c5d:	bf 60 ac             	mov    $0xac60,%di
    9c60:	e8 7f 00             	call   0x9ce2
    9c63:	6a a5                	push   $0xffa5
    9c65:	be af 73             	mov    $0x73af,%si
    9c68:	56                   	push   %si
    9c69:	33 76 00             	xor    0x0(%bp),%si
    9c6c:	81 4e d6 75 58       	orw    $0x5875,-0x2a(%bp)
    9c71:	36 16                	ss push %ss
    9c73:	04 ff                	add    $0xff,%al
    9c75:	16                   	push   %ss
    9c76:	54                   	push   %sp
    9c77:	a5                   	movsw  %ds:(%si),%es:(%di)
    9c78:	6a be                	push   $0xffbe
    9c7a:	73 33                	jae    0x9caf
    9c7c:	00 4e 75             	add    %cl,0x75(%bp)
    9c7f:	61                   	popa
    9c80:	00 a5 af 56          	add    %ah,0x56af(%di)
    9c84:	76 81                	jbe    0x9c07
    9c86:	d6                   	(bad)
    9c87:	58                   	pop    %ax
    9c88:	67 00 e8             	addr32 add %ch,%al
    9c8b:	23 08                	and    (%bx,%si),%cx
    9c8d:	90                   	nop
    9c8e:	74 89                	je     0x9c19
    9c90:	11 58 0a             	adc    %bx,0xa(%bx,%si)
    9c93:	e4 50                	in     $0x50,%al
    9c95:	75 0b                	jne    0x9ca2
    9c97:	83 7e 04 00          	cmpw   $0x0,0x4(%bp)
    9c9b:	75 05                	jne    0x9ca2
    9c9d:	c7 46 04 ff 00       	movw   $0xff,0x4(%bp)
    9ca2:	e8 10 00             	call   0x9cb5
    9ca5:	58                   	pop    %ax
    9ca6:	0a e4                	or     %ah,%ah
    9ca8:	75 07                	jne    0x9cb1
    9caa:	08 47 04             	or     %al,0x4(%bx)
    9cad:	b4 4c                	mov    $0x4c,%ah
    9caf:	8b c6                	mov    %si,%ax
    9cb1:	2b b9 c0 c3          	sub    -0x3c40(%bx,%di),%di
    9cb5:	06                   	push   %es
    9cb6:	0e                   	push   %cs
    9cb7:	60                   	pusha
    9cb8:	a5                   	movsw  %ds:(%si),%es:(%di)
    9cb9:	e3 07                	jcxz   0x9cc2
    9cbb:	bb 00 ba             	mov    $0xba00,%bx
    9cbe:	ff 5e 16             	lcall  *0x16(%bp)
    9cc1:	8e 1e c5 16          	mov    0x16c5,%ds
    9cc5:	ea a3 b8 25 34       	ljmp   $0x3425,$0xb8a3
    9cca:	8b c6                	mov    %si,%ax
    9ccc:	1f                   	pop    %ds
    9ccd:	80 3e 28 a4 00       	cmpb   $0x0,-0x5bd8
    9cd2:	74 0d                	je     0x9ce1
    9cd4:	1e                   	push   %ds
    9cd5:	a0 29 a4             	mov    0xa429,%al
    9cd8:	c5 16 2a a4          	lds    -0x5bd6,%dx
    9cdc:	b4 34                	mov    $0x34,%ah
    9cde:	8b c6                	mov    %si,%ax
    9ce0:	1f                   	pop    %ds
    9ce1:	c3                   	ret
    9ce2:	3b f7                	cmp    %di,%si
    9ce4:	73 0a                	jae    0x9cf0
    9ce6:	4f                   	dec    %di
    9ce7:	4f                   	dec    %di
    9ce8:	8b 0d                	mov    (%di),%cx
    9cea:	e3 f6                	jcxz   0x9ce2
    9cec:	ff d1                	call   *%cx
    9cee:	eb f2                	jmp    0x9ce2
    9cf0:	c3                   	ret
    9cf1:	3b f7                	cmp    %di,%si
    9cf3:	73 0e                	jae    0x9d03
    9cf5:	83 ef 04             	sub    $0x4,%di
    9cf8:	8b 05                	mov    (%di),%ax
    9cfa:	0b 45 02             	or     0x2(%di),%ax
    9cfd:	74 f2                	je     0x9cf1
    9cff:	ff 1d                	lcall  *(%di)
    9d01:	eb ee                	jmp    0x9cf1
    9d03:	73 7e                	jae    0x9d83
    9d05:	56                   	push   %si
    9d06:	57                   	push   %di
    9d07:	03 8b 4e db          	add    -0x24b2(%bp,%di),%cx
    9d0b:	8b 0a                	mov    (%bp,%si),%cx
    9d0d:	0b c8                	or     %ax,%cx
    9d0f:	db 8b 08 75          	fisttpl 0x7508(%bp,%di)
    9d13:	09 8b 0e 06          	or     %cx,0x60e(%bp,%di)
    9d17:	ff e1                	jmp    *%cx
    9d19:	b9 c0 52             	mov    $0x52c0,%cx
    9d1c:	f0 53                	lock push %bx
    9d1e:	ff e1                	jmp    *%cx
    9d20:	8b d8                	mov    %ax,%bx
    9d22:	8b 0e 06 ff          	mov    -0xfa,%cx
    9d26:	66 0a 03             	data32 or (%bp,%di),%al
    9d29:	d8 8b 0e 06          	fmuls  0x60e(%bp,%di)
    9d2d:	ff e1                	jmp    *%cx
    9d2f:	03 d3                	add    %bx,%dx
    9d31:	5b                   	pop    %bx
    9d32:	fa                   	cli
    9d33:	4c                   	dec    %sp
    9d34:	46                   	inc    %si
    9d35:	53                   	push   %bx
    9d36:	56                   	push   %si
    9d37:	7e 08                	jle    0x9d41
    9d39:	57                   	push   %di
    9d3a:	56                   	push   %si
    9d3b:	53                   	push   %bx
    9d3c:	33 ff                	xor    %di,%di
    9d3e:	03 8b 4e 90          	add    -0x6fb2(%bp,%di),%cx
    9d42:	74 7d                	je     0x9dc1
    9d44:	11 47 0e             	adc    %ax,0xe(%bx)
    9d47:	7d c6                	jge    0x9d0f
    9d49:	f7 d8                	neg    %ax
    9d4b:	f7 da                	neg    %dx
    9d4d:	1d 00 00             	sbb    $0x0,%ax
    9d50:	89 06 47 f4          	mov    %ax,-0xbb9
    9d54:	0b 8b 88 46          	or     0x4688(%bp,%di),%cx
    9d58:	0a 0b                	or     (%bp,%di),%cl
    9d5a:	74 11                	je     0x9d6d
    9d5c:	0e                   	push   %cs
    9d5d:	c6                   	(bad)
    9d5e:	d8 da                	fcomp  %st(2)
    9d60:	08 0a                	or     %cl,(%bp,%si)
    9d62:	b8 f7 1d             	mov    $0x1df7,%ax
    9d65:	00 06 f4 8b          	add    %al,-0x740c
    9d69:	46                   	inc    %si
    9d6a:	0a f4                	or     %ah,%dh
    9d6c:	0b 08                	or     (%bx,%si),%cx
    9d6e:	90                   	nop
    9d6f:	74 75                	je     0x9de6
    9d71:	15 db 8b             	adc    $0x8bdb,%ax
    9d74:	08 03                	or     %al,(%bp,%di)
    9d76:	8b 4e 33             	mov    0x33(%bp),%cx
    9d79:	d2 f7                	shl    %cl,%bh
    9d7b:	f1                   	int1
    9d7c:	8b 8b 06 66          	mov    0x6606(%bp,%di),%cx
    9d80:	03 8b f1 8b          	add    -0x740f(%bp,%di),%cx
    9d84:	d3 eb                	shr    %cl,%bx
    9d86:	38 8b 8b 06          	cmp    %cl,0x68b(%bp,%di)
    9d8a:	8b 08                	mov    (%bx,%si),%cx
    9d8c:	03 56 76             	add    0x76(%bp),%dx
    9d8f:	77 8a                	ja     0x9d1b
    9d91:	9b                   	fwait
    9d92:	d1 eb                	shr    $1,%bx
    9d94:	d1 d9                	rcr    $1,%cx
    9d96:	d1 ea                	shr    $1,%dx
    9d98:	d1 d8                	rcr    $1,%ax
    9d9a:	0b e9                	or     %cx,%bp
    9d9c:	03 f4                	add    %sp,%si
    9d9e:	8b f1                	mov    %cx,%si
    9da0:	8b f0                	mov    %ax,%si
    9da2:	ff 66 0a             	jmp    *0xa(%bp)
    9da5:	91                   	xchg   %ax,%cx
    9da6:	ff                   	(bad)
    9da7:	7a 04                	jp     0x9dad
    9da9:	f7 e6                	mul    %si
    9dab:	03 d1                	add    %cx,%dx
    9dad:	72 0c                	jb     0x9dbb
    9daf:	3b 56 76             	cmp    0x76(%bp),%dx
    9db2:	77 07                	ja     0x9dbb
    9db4:	72 06                	jb     0x9dbc
    9db6:	3b 8a 9b 76          	cmp    0x769b(%bp,%si),%cx
    9dba:	01 4e 33             	add    %cx,0x33(%bp)
    9dbd:	d2 96 4f 75          	rclb   %cl,0x754f(%bp)
    9dc1:	07                   	pop    %es
    9dc2:	f7 f7                	div    %di
    9dc4:	b9 d8 83             	mov    $0x83d8,%cx
    9dc7:	da 00                	fiaddl (%bx,%si)
    9dc9:	5b                   	pop    %bx
    9dca:	5e                   	pop    %si
    9dcb:	5f                   	pop    %di
    9dcc:	fa                   	cli
    9dcd:	46                   	inc    %si
    9dce:	56                   	push   %si
    9dcf:	08 56 33             	or     %dl,0x33(%bp)
    9dd2:	03 77 8a             	add    -0x76(%bx),%si
    9dd5:	9b                   	fwait
    9dd6:	2d 20 00             	sub    $0x20,%ax
    9dd9:	5d                   	pop    %bp
    9dda:	c3                   	ret
    9ddb:	90                   	nop
    9ddc:	06                   	push   %es
    9ddd:	b8 e8 ff             	mov    $0xffe8,%ax
    9de0:	0c e0                	or     $0xe0,%al
    9de2:	f6 87 35 a4 02       	testb  $0x2,-0x5bcb(%bx)
    9de7:	74 05                	je     0x9dee
    9de9:	8d 47 e0             	lea    -0x20(%bx),%ax
    9dec:	eb 02                	jmp    0x9df0
    9dee:	8b c3                	mov    %bx,%ax
    9df0:	5d                   	pop    %bp
    9df1:	c3                   	ret
    9df2:	06                   	push   %es
    9df3:	b8 e8 ff             	mov    $0xffe8,%ax
    9df6:	d7                   	xlat   %ds:(%bx)
    9df7:	8b de                	mov    %si,%bx
    9df9:	8c 8b 10 10          	mov    %cs,0x1010(%bp,%di)
    9dfd:	07                   	pop    %es
    9dfe:	7e 04                	jle    0x9e04
    9e00:	c0 0c b9             	rorb   $0xb9,(%si)
    9e03:	ff                   	(bad)
    9e04:	ff f2                	push   %dx
    9e06:	ae                   	scas   %es:(%di),%al
    9e07:	8d 75 ff             	lea    -0x1(%di),%si
    9e0a:	03 7e 06             	add    0x6(%bp),%di
    9e0d:	b9 ff ff             	mov    $0xffff,%cx
    9e10:	f2 ae                	repnz scas %es:(%di),%al
    9e12:	f7 d1                	not    %cx
    9e14:	2b f9                	sub    %cx,%di
    9e16:	87 fe                	xchg   %di,%si
    9e18:	06                   	push   %es
    9e19:	66 03 8b b3 8a       	add    -0x754d(%bp,%di),%ecx
    9e1e:	26 08 c6             	es or  %al,%dh
    9e21:	a4                   	movsb  %ds:(%si),%es:(%di)
    9e22:	49                   	dec    %cx
    9e23:	d1 e9                	shr    $1,%cx
    9e25:	f3 a5                	rep movsw %ds:(%si),%es:(%di)
    9e27:	13 c9                	adc    %cx,%cx
    9e29:	c6 03 b4             	movb   $0xb4,(%bp,%di)
    9e2c:	f3 8b fa             	repz mov %dx,%di
    9e2f:	5d                   	pop    %bp
    9e30:	c3                   	ret
    9e31:	08 56 33             	or     %dl,0x33(%bp)
    9e34:	03 77 d7             	add    -0x29(%bx),%si
    9e37:	8b de                	mov    %si,%bx
    9e39:	8b 06 f7 8b          	mov    -0x7409,%ax
    9e3d:	fe 8c 8b 10          	decb   0x108b(%si)
    9e41:	10 c0                	adc    %al,%al
    9e43:	b9 ff ae             	mov    $0xaeff,%cx
    9e46:	75 03                	jne    0x9e4b
    9e48:	06                   	push   %es
    9e49:	f7 d1                	not    %cx
    9e4b:	07                   	pop    %es
    9e4c:	7e 04                	jle    0x9e52
    9e4e:	8b c7                	mov    %di,%ax
    9e50:	a8 01                	test   $0x1,%al
    9e52:	08 a4 d1 f3          	or     %ah,-0xc2f(%si)
    9e56:	13 c6                	adc    %si,%ax
    9e58:	b4 8b                	mov    $0x8b,%ah
    9e5a:	5d                   	pop    %bp
    9e5b:	08 33                	or     %dh,(%bp,%di)
    9e5d:	77 8b                	ja     0x9dea
    9e5f:	8b f7                	mov    %di,%si
    9e61:	fe 8b 10 56          	decb   0x5610(%bp,%di)
    9e65:	33 03                	xor    (%bp,%di),%ax
    9e67:	77 d6                	ja     0x9e3f
    9e69:	8b 06 f7 8b          	mov    -0x7409,%ax
    9e6d:	0c e0                	or     $0xe0,%al
    9e6f:	b0 75                	mov    $0x75,%al
    9e71:	b8 b4 74             	mov    $0x74b4,%ax
    9e74:	2c ac                	sub    $0xac,%al
    9e76:	8a 27                	mov    (%bx),%ah
    9e78:	43                   	inc    %bx
    9e79:	3a e0                	cmp    %al,%ah
    9e7b:	74 f4                	je     0x9e71
    9e7d:	2c 41                	sub    $0x41,%al
    9e7f:	3c 1a                	cmp    $0x1a,%al
    9e81:	1a c9                	sbb    %cl,%cl
    9e83:	80 e1 20             	and    $0x20,%cl
    9e86:	02 c1                	add    %cl,%al
    9e88:	04 41                	add    $0x41,%al
    9e8a:	86 e0                	xchg   %ah,%al
    9e8c:	2c 3c                	sub    $0x3c,%al
    9e8e:	1a 80 20 c1          	sbb    -0x3ee0(%bx,%si),%al
    9e92:	41                   	inc    %cx
    9e93:	e0 3c                	loopne 0x9ed1
    9e95:	80 c1 e0             	add    $0xe0,%cl
    9e98:	80 3a c4             	cmpb   $0xc4,(%bp,%si)
    9e9b:	74 d4                	je     0x9e71
    9e9d:	1a c0                	sbb    %al,%al
    9e9f:	1c ff                	sbb    $0xff,%al
    9ea1:	8b 06 f2 8b          	mov    -0x740e,%ax
    9ea5:	56                   	push   %si
    9ea6:	03 d6                	add    %si,%dx
    9ea8:	06                   	push   %es
    9ea9:	8b 0b                	mov    (%bp,%di),%cx
    9eab:	88 0a                	mov    %cl,(%bp,%si)
    9ead:	74 06                	je     0x9eb5
    9eaf:	b4 3d                	mov    $0x3d,%ah
    9eb1:	cd e7                	int    $0xe7
    9eb3:	f6 05 5e             	testb  $0x5e,(%di)
    9eb6:	9a de 89 07 e9       	lcall  $0xe907,$0x89de
    9ebb:	1d 06 82             	sbb    $0x8206,%ax
    9ebe:	07                   	pop    %es
    9ebf:	04 02                	add    $0x2,%al
    9ec1:	b4 3f                	mov    $0x3f,%ah
    9ec3:	eb 05                	jmp    0x9eca
    9ec5:	07                   	pop    %es
    9ec6:	04 02                	add    $0x2,%al
    9ec8:	b4 40                	mov    $0x40,%ah
    9eca:	07                   	pop    %es
    9ecb:	8b 88 0a 4e          	mov    0x4e0a(%bx,%si),%cx
    9ecf:	0a 81 d6 58          	or     0x58d6(%bx,%di),%al
    9ed3:	16                   	push   %ss
    9ed4:	ff 54 6a             	call   *0x6a(%si)
    9ed7:	73 00                	jae    0x9ed9
    9ed9:	75 50                	jne    0x9f2b
    9edb:	8e c5                	mov    %bp,%es
    9edd:	ea 2a cd 06 05       	ljmp   $0x506,$0xcd2a
    9ee2:	1f                   	pop    %ds
    9ee3:	f6 05 5e             	testb  $0x5e,(%di)
    9ee6:	9a 0c 89 07 e9       	lcall  $0xe907,$0x890c
    9eeb:	ed                   	in     (%dx),%ax
    9eec:	05 53 7e             	add    $0x7e53,%ax
    9eef:	57                   	push   %di
    9ef0:	53                   	push   %bx
    9ef1:	ff 8b 1e 07          	decw   0x71e(%bp,%di)
    9ef5:	07                   	pop    %es
    9ef6:	7e 04                	jle    0x9efc
    9ef8:	8b 06 f7 8b          	mov    -0x7409,%ax
    9efc:	df 06 08 56          	filds  0x5608
    9f00:	e3 0c                	jcxz   0x9f0e
    9f02:	ac                   	lods   %ds:(%si),%al
    9f03:	b8 b4 74             	mov    $0x74b4,%ax
    9f06:	03 aa e2 f8          	add    -0x71e(%bp,%si),%bp
    9f0a:	32 aa fe 75          	xor    0x75fe(%bp,%si),%ch
    9f0e:	8b c3                	mov    %bx,%ax
    9f10:	46                   	inc    %si
    9f11:	87 e8                	xchg   %bp,%ax
    9f13:	0c 53                	or     $0x53,%al
    9f15:	56                   	push   %si
    9f16:	03 d6                	add    %si,%dx
    9f18:	06                   	push   %es
    9f19:	53                   	push   %bx
    9f1a:	76 02                	jbe    0x9f1e
    9f1c:	07                   	pop    %es
    9f1d:	8b 88 3b 1e          	mov    0x1e3b(%bx,%si),%cx
    9f21:	05 a4 f6             	add    $0xf6a4,%ax
    9f24:	05 b8 00             	add    $0xb8,%ax
    9f27:	09 eb                	or     %bp,%bx
    9f29:	2a f7                	sub    %bh,%dh
    9f2b:	46                   	inc    %si
    9f2c:	08 00                	or     %al,(%bx,%si)
    9f2e:	80 74 48 83          	xorb   $0x83,0x48(%si)
    9f32:	7e 0a                	jle    0x9f3e
    9f34:	00 74 1a             	add    %dh,0x1a(%si)
    9f37:	33 c9                	xor    %cx,%cx
    9f39:	8b d1                	mov    %cx,%dx
    9f3b:	b8 01 42             	mov    $0x4201,%ax
    9f3e:	cd e7                	int    $0xe7
    9f40:	f6 4b f7 46          	testb  $0x46,-0x9(%bp,%di)
    9f44:	0a 02                	or     (%bp,%si),%al
    9f46:	00 75 0e             	add    %dh,0xe(%di)
    9f49:	8a 2a                	mov    (%bp,%si),%ch
    9f4b:	50                   	push   %ax
    9f4c:	13 56 08             	adc    0x8(%bp),%dx
    9f4f:	79 28                	jns    0x9f79
    9f51:	b8 00 16             	mov    $0x1600,%ax
    9f54:	f9                   	stc
    9f55:	eb 36                	jmp    0x9f8d
    9f57:	89 56 fe             	mov    %dx,-0x2(%bp)
    9f5a:	9e                   	sahf
    9f5b:	2b 37                	sub    (%bx),%si
    9f5d:	08 d1                	or     %dl,%cl
    9f5f:	b8 02 42             	mov    $0x4202,%ax
    9f62:	cd e7                	int    $0xe7
    9f64:	8a 50 56             	mov    0x56(%bx,%si),%dl
    9f67:	79 b8                	jns    0x9f21
    9f69:	16                   	push   %ss
    9f6a:	eb 0d                	jmp    0x9f79
    9f6c:	87 fe                	xchg   %di,%si
    9f6e:	8b e8                	mov    %ax,%bp
    9f70:	74 7e                	je     0x9ff0
    9f72:	b8 00 42             	mov    $0x4200,%ax
    9f75:	cd e7                	int    $0xe7
    9f77:	eb 96                	jmp    0x9f0f
    9f79:	56                   	push   %si
    9f7a:	d1 4e 8b             	rorw   $1,-0x75(%bp)
    9f7d:	0b 08                	or     (%bx,%si),%cx
    9f7f:	8a 46 0a             	mov    0xa(%bp),%al
    9f82:	b4 42                	mov    $0x42,%ah
    9f84:	cd e7                	int    $0xe7
    9f86:	f6 05 80             	testb  $0x80,(%di)
    9f89:	07                   	pop    %es
    9f8a:	bf 00 fd             	mov    $0xfd00,%di
    9f8d:	e9 57 05             	jmp    0xa4e7
    9f90:	07                   	pop    %es
    9f91:	04 02                	add    $0x2,%al
    9f93:	07                   	pop    %es
    9f94:	8b 88 b4 3e          	mov    0x3eb4(%bx,%si),%cx
    9f98:	cd e7                	int    $0xe7
    9f9a:	e9 3d 05             	jmp    0xa4da
    9f9d:	53                   	push   %bx
    9f9e:	7e 57                	jle    0x9ff7
    9fa0:	53                   	push   %bx
    9fa1:	1e                   	push   %ds
    9fa2:	b0 4f                	mov    $0x4f,%al
    9fa4:	8b 0b                	mov    (%bp,%di),%cx
    9fa6:	88 eb                	mov    %ch,%bl
    9fa8:	09 7e 57             	or     %di,0x57(%bp)
    9fab:	53                   	push   %bx
    9fac:	1e                   	push   %ds
    9fad:	b0 4e                	mov    $0x4e,%al
    9faf:	d8 08                	fmuls  (%bx,%si)
    9fb1:	b8 b4 2f             	mov    $0x2fb4,%ax
    9fb4:	cd e7                	int    $0xe7
    9fb6:	b4 1a                	mov    $0x1a,%ah
    9fb8:	cd 3c                	int    $0x3c
    9fba:	73 4e                	jae    0xa00a
    9fbc:	75 8b                	jne    0x9f49
    9fbe:	0b 8b 88 a3          	or     -0x5c78(%bp,%di),%cx
    9fc2:	4e                   	dec    %si
    9fc3:	06                   	push   %es
    9fc4:	8a e0                	mov    %al,%ah
    9fc6:	cd 3c                	int    $0x3c
    9fc8:	50                   	push   %ax
    9fc9:	9f                   	lahf
    9fca:	50                   	push   %ax
    9fcb:	8c c2                	mov    %es,%dx
    9fcd:	8e da                	mov    %dx,%ds
    9fcf:	8b d3                	mov    %bx,%dx
    9fd1:	b4 1a                	mov    $0x1a,%ah
    9fd3:	cd 3c                	int    $0x3c
    9fd5:	58                   	pop    %ax
    9fd6:	9e                   	sahf
    9fd7:	58                   	pop    %ax
    9fd8:	1f                   	pop    %ds
    9fd9:	e9 fe 04             	jmp    0xa4da
    9fdc:	8a bb eb 01          	mov    0x1eb(%bp,%di),%bh
    9fe0:	51                   	push   %cx
    9fe1:	b3 01                	mov    $0x1,%bl
    9fe3:	9e                   	sahf
    9fe4:	2b 08                	sub    (%bx,%si),%cx
    9fe6:	8b 33                	mov    (%bp,%di),%si
    9fe8:	04 33                	add    $0x33,%al
    9fea:	d2 83 f9 0a          	rolb   %cl,0xaf9(%bp,%di)
    9fee:	75 01                	jne    0x9ff1
    9ff0:	99                   	cwtd
    9ff1:	03 06 ff e9          	add    -0x1601,%ax
    9ff5:	05 7e 53             	add    $0x537e,%ax
    9ff8:	b0 8b                	mov    $0x8b,%al
    9ffa:	88 57 07             	mov    %dl,0x7(%bx)
    9ffd:	04 06                	add    $0x6,%al
    9fff:	1e                   	push   %ds
    a000:	07                   	pop    %es
    a001:	04 df                	add    $0xdf,%al
    a003:	c0 ff 75             	sar    $0x75,%bh
    a006:	06                   	push   %es
    a007:	d1 7e 8b             	sarw   $1,-0x75(%bp)
    a00a:	41                   	inc    %cx
    a00b:	f7 d9                	neg    %cx
    a00d:	8a 06 e2 8b          	mov    -0x741e,%al
    a011:	fb                   	sti
    a012:	7e 8b                	jle    0x9f9f
    a014:	4f                   	dec    %di
    a015:	38 05                	cmp    %al,(%di)
    a017:	74 02                	je     0xa01b
    a019:	33 03                	xor    (%bp,%di),%ax
    a01b:	4e                   	dec    %si
    a01c:	c7 87 0c 56 d6 53    	movw   $0x53d6,0x560c(%bx)
    a022:	02 8b 3b 01          	add    0x13b(%bp,%di),%cl
    a026:	51                   	push   %cx
    a027:	b3 01                	mov    $0x1,%bl
    a029:	e9 fc 04             	jmp    0xa528
    a02c:	8a bb eb 8b          	mov    -0x7415(%bp,%di),%bh
    a030:	d7                   	xlat   %ds:(%bx)
    a031:	8c 10                	mov    %ss,(%bx,%si)
    a033:	07                   	pop    %es
    a034:	04 0c                	add    $0xc,%al
    a036:	ff f2                	push   %dx
    a038:	8d                   	lea    (bad),%di
    a039:	ff                   	(bad)
    a03a:	7e b9                	jle    0x9ff5
    a03c:	ff ae d1 f7          	ljmp   *-0x82f(%bp)
    a040:	d1 49 91             	rorw   $1,-0x6f(%bx,%di)
    a043:	8b 5d 08             	mov    0x8(%di),%bx
    a046:	33 77 8b             	xor    -0x75(%bx),%si
    a049:	8b f7                	mov    %di,%si
    a04b:	53                   	push   %bx
    a04c:	56                   	push   %si
    a04d:	88 0a                	mov    %cl,(%bp,%si)
    a04f:	74 0e                	je     0xa05f
    a051:	d8 75 db             	fdivs  -0x25(%di)
    a054:	08 8b 33 f7          	or     %cl,-0x8cd(%bp,%di)
    a058:	8b 06 03 f1          	mov    -0xefd,%ax
    a05c:	d3 38                	sarw   %cl,(%bx,%si)
    a05e:	8b 8b 03 76          	mov    0x7603(%bp,%di),%cx
    a062:	8a d1                	mov    %cl,%dl
    a064:	d1 d1                	rcl    $1,%cx
    a066:	d1 0b                	rorw   $1,(%bp,%di)
    a068:	03 8b c8 c6          	add    -0x3938(%bp,%di),%cx
    a06c:	05 03 76             	add    $0x7603,%ax
    a06f:	56                   	push   %si
    a070:	77 9b                	ja     0xa00d
    a072:	eb d9                	jmp    0xa04d
    a074:	ea e9 d1 db d1       	ljmp   $0xd1db,$0xd1e9
    a079:	d1 0b                	rorw   $1,(%bp,%di)
    a07b:	03 8b c9 03          	add    0x3c9(%bp,%di),%cx
    a07f:	8b 8b f3 8b          	mov    -0x740d(%bp,%di),%cx
    a083:	ff 0a                	decw   (%bp,%si)
    a085:	ff 04                	incw   (%si)
    a087:	e6 d1                	out    %al,$0xd1
    a089:	0c 56                	or     $0x56,%al
    a08b:	77 72                	ja     0xa0ff
    a08d:	3b 9b 01 33          	cmp    0x3301(%bp,%di),%bx
    a091:	96                   	xchg   %ax,%si
    a092:	75 f7                	jne    0xa08b
    a094:	b9 83 00             	mov    $0x83,%cx
    a097:	5e                   	pop    %si
    a098:	fa                   	cli
    a099:	56                   	push   %si
    a09a:	56                   	push   %si
    a09b:	03 8a 2d 00          	add    0x2d(%bp,%si),%cx
    a09f:	c3                   	ret
    a0a0:	06                   	push   %es
    a0a1:	2b cb                	sub    %bx,%cx
    a0a3:	8b 06 33 b9          	mov    -0x46cd,%ax
    a0a7:	e8 c0 0c             	call   0xad6a
    a0aa:	4a                   	dec    %dx
    a0ab:	fc                   	cld
    a0ac:	0b 20                	or     (%bx,%si),%sp
    a0ae:	56                   	push   %si
    a0af:	88 d7                	mov    %dl,%bh
    a0b1:	16                   	push   %ss
    a0b2:	07                   	pop    %es
    a0b3:	10 ef                	adc    %ch,%bh
    a0b5:	01 8d ff 8d          	add    %cx,-0x7201(%di)
    a0b9:	7e e0                	jle    0xa09b
    a0bb:	f3 ab                	rep stos %ax,%es:(%di)
    a0bd:	8b f7                	mov    %di,%si
    a0bf:	df ac b4 03          	fildll 0x3b4(%si)
    a0c3:	e2 14                	loop   0xa0d9
    a0c5:	08 9b 2c c8          	or     %bl,-0x37d4(%bp,%di)
    a0c9:	b0 01                	mov    $0x1,%al
    a0cb:	80 e1 07             	and    $0x7,%cl
    a0ce:	d2 e0                	shl    %cl,%al
    a0d0:	b1 88                	mov    $0x88,%cl
    a0d2:	fc                   	cld
    a0d3:	ef                   	out    %ax,(%dx)
    a0d4:	08 43 e0             	or     %al,-0x20(%bp,%di)
    a0d7:	eb e7                	jmp    0xa0c0
    a0d9:	02 76 f8             	add    -0x8(%bp),%dh
    a0dc:	4a                   	dec    %dx
    a0dd:	02 75 04             	add    0x4(%di),%dh
    a0e0:	8b 36 2e a4          	mov    -0x5bd2,%si
    a0e4:	ac                   	lods   %ds:(%si),%al
    a0e5:	25 ff 00             	and    $0xff,%ax
    a0e8:	75 07                	jne    0xa0f1
    a0ea:	4e                   	dec    %si
    a0eb:	89 36 2e a4          	mov    %si,-0x5bd2
    a0ef:	eb 41                	jmp    0xa132
    a0f1:	08 2c                	or     %ch,(%si)
    a0f3:	b0 80                	mov    $0x80,%al
    a0f5:	07                   	pop    %es
    a0f6:	e0 88                	loopne 0xa080
    a0f8:	ef                   	out    %ax,(%dx)
    a0f9:	43                   	inc    %bx
    a0fa:	eb 02                	jmp    0xa0fe
    a0fc:	f8                   	clc
    a0fd:	02 04                	add    (%si),%al
    a0ff:	36 22 43 e0          	and    %ss:-0x20(%bp,%di),%al
    a103:	75 df                	jne    0xa0e4
    a105:	8d 44 ff             	lea    -0x1(%si),%ax
    a108:	a3 2e ac             	mov    %ax,0xac2e
    a10b:	ff 75 4e             	push   0x4e(%di)
    a10e:	36 a4                	movsb  %ss:(%si),%es:(%di)
    a110:	05 8d 44             	add    $0x448d,%ax
    a113:	ff                   	ljmp   (bad)
    a114:	eb 18                	jmp    0xa12e
    a116:	08 b0 07 88          	or     %dh,-0x77f9(%bx,%si)
    a11a:	43                   	inc    %bx
    a11b:	02 02                	add    (%bp,%si),%al
    a11d:	36 43                	ss inc %bx
    a11f:	75 8d                	jne    0xa0ae
    a121:	ff 2e ff 4e          	ljmp   *0x4eff
    a125:	a4                   	movsb  %ds:(%si),%es:(%di)
    a126:	8d                   	lea    (bad),%di
    a127:	ff 74 e1             	push   -0x1f(%si)
    a12a:	88 64 ff             	mov    %ah,-0x1(%si)
    a12d:	96                   	xchg   %ax,%si
    a12e:	87 06 2e ac          	xchg   %ax,-0x53d2
    a132:	8b 5d ef             	mov    -0x11(%di),%bx
    a135:	d3 8d db c3          	rorw   %cl,-0x3c25(%di)
    a139:	56                   	push   %si
    a13a:	03 d7                	add    %di,%dx
    a13c:	de 06 56 04          	fiadds 0x456
    a140:	8a e2                	mov    %dl,%ah
    a142:	fb                   	sti
    a143:	ee                   	out    %al,(%dx)
    a144:	b4 00                	mov    $0x0,%ah
    a146:	c3                   	ret
    a147:	06                   	push   %es
    a148:	b0 88                	mov    $0x88,%al
    a14a:	07                   	pop    %es
    a14b:	06                   	push   %es
    a14c:	07                   	pop    %es
    a14d:	0e                   	push   %cs
    a14e:	ff 0a                	decw   (%bp,%si)
    a150:	d8 06 73 33          	fadds  0x3373
    a154:	06                   	push   %es
    a155:	cb                   	lret
    a156:	99                   	cwtd
    a157:	eb 18                	jmp    0xa171
    a159:	50                   	push   %ax
    a15a:	e8 1b 00             	call   0xa178
    a15d:	59                   	pop    %cx
    a15e:	90                   	nop
    a15f:	89 58 0f             	mov    %bx,0xf(%bx,%si)
    a162:	50                   	push   %ax
    a163:	08 b0 8c 10          	or     %dh,0x108c(%bx,%si)
    a167:	c0 ff 75             	sar    $0x75,%bh
    a16a:	06                   	push   %es
    a16b:	41                   	inc    %cx
    a16c:	13 b4 5d ab          	adc    -0x54a3(%si),%si
    a170:	58                   	pop    %ax
    a171:	5f                   	pop    %di
    a172:	c3                   	ret
    a173:	06                   	push   %es
    a174:	e9 11 04             	jmp    0xa588
    a177:	00 e9                	add    %ch,%cl
    a179:	2e 02 b0 fe 9b       	add    %cs:-0x6402(%bx,%si),%dh
    a17e:	0a 01                	or     (%bx,%di),%al
    a180:	b3 e9                	mov    $0xe9,%bl
    a182:	00 e9                	add    %ch,%cl
    a184:	a2 03 06             	mov    %al,0x603
    a187:	e8 d7 de             	call   0x8061
    a18a:	8b 10                	mov    (%bx,%si),%dx
    a18c:	7e c0                	jle    0xa14e
    a18e:	b9 ff ae             	mov    $0xaeff,%cx
    a191:	75 76                	jne    0xa209
    a193:	f8                   	clc
    a194:	03 ff                	add    %di,%di
    a196:	05 8d 7e             	add    $0x7e8d,%ax
    a199:	ff d1                	call   *%cx
    a19b:	d1 91 5d 33          	rclw   $1,0x335d(%bx,%di)
    a19f:	8b 2b                	mov    (%bp,%di),%bp
    a1a1:	f9                   	stc
    a1a2:	f3 74 b9             	repz je 0xa15e
    a1a5:	05 1b c0             	add    $0xc01b,%ax
    a1a8:	1d d1 d1             	sbb    $0xd1d1,%ax
    a1ab:	b4 8b                	mov    $0x8b,%ah
    a1ad:	5d                   	pop    %bp
    a1ae:	08 33                	or     %dh,(%bp,%di)
    a1b0:	77 8b                	ja     0xa13d
    a1b2:	8b f7                	mov    %di,%si
    a1b4:	fe                   	(bad)
    a1b5:	b7 5b                	mov    $0x5b,%bh
    a1b7:	eb 07                	jmp    0xa1c0
    a1b9:	02 40 8b             	add    -0x75(%bx,%si),%al
    a1bc:	b7 3c                	mov    $0x3c,%bh
    a1be:	0b 88 4e 8a          	or     -0x75b2(%bx,%si),%cx
    a1c2:	cd 50                	int    $0x50
    a1c4:	50                   	push   %ax
    a1c5:	e7 02                	out    %ax,$0x2
    a1c7:	04 00                	add    $0x0,%al
    a1c9:	0a 3d                	or     (%di),%bh
    a1cb:	50                   	push   %ax
    a1cc:	00 f9                	add    %bh,%cl
    a1ce:	75 09                	jne    0xa1d9
    a1d0:	b4 11                	mov    $0x11,%ah
    a1d2:	eb 07                	jmp    0xa1db
    a1d4:	5e                   	pop    %si
    a1d5:	de 07                	fiadds (%bx)
    a1d7:	1d 82 04             	sbb    $0x482,%ax
    a1da:	fe 02                	incb   (%bp,%si)
    a1dc:	b0 88                	mov    $0x88,%al
    a1de:	07                   	pop    %es
    a1df:	06                   	push   %es
    a1e0:	07                   	pop    %es
    a1e1:	df ff                	(bad)
    a1e3:	06                   	push   %es
    a1e4:	7e 41                	jle    0xa227
    a1e6:	d7                   	xlat   %ds:(%bx)
    a1e7:	c0 75 d1 8b          	shlb   $0x8b,-0x2f(%di)
    a1eb:	f7 8a e2 fb 41 d9    	testw  $0xd941,-0x41e(%bp,%si)
    a1f1:	06                   	push   %es
    a1f2:	8b 7e 4f             	mov    0x4f(%bp),%di
    a1f5:	05 fa fe             	add    $0xfefa,%ax
    a1f8:	8b 46 c2             	mov    -0x3e(%bp),%ax
    a1fb:	87 56 53             	xchg   %dx,0x53(%bp)
    a1fe:	8b 01                	mov    (%bx,%di),%ax
    a200:	b3 e9                	mov    $0xe9,%bl
    a202:	04 07                	add    $0x7,%al
    a204:	88 4e 81             	mov    %cl,-0x7f(%bp)
    a207:	d3 eb                	shr    %cl,%bx
    a209:	0b 2c                	or     (%si),%bp
    a20b:	61                   	popa
    a20c:	3c 1a                	cmp    $0x1a,%al
    a20e:	73 04                	jae    0xa214
    a210:	04 41                	add    $0x41,%al
    a212:	88 43 fe             	mov    %al,-0x2(%bp,%di)
    a215:	8a 07                	mov    (%bx),%al
    a217:	0a 77 fe             	or     -0x2(%bx),%dh
    a21a:	ef                   	out    %ax,(%dx)
    a21b:	92                   	xchg   %ax,%dx
    a21c:	d6                   	(bad)
    a21d:	02 3b                	add    (%bp,%di),%bh
    a21f:	51                   	push   %cx
    a220:	01 fc                	add    %di,%sp
    a222:	8a 06 7e 41          	mov    0x417e,%al
    a226:	88 4e be             	mov    %cl,-0x42(%bp)
    a229:	72 a5                	jb     0xa1d0
    a22b:	ad                   	lods   %ds:(%si),%ax
    a22c:	3b c2                	cmp    %dx,%ax
    a22e:	74 10                	je     0xa240
    a230:	40                   	inc    %ax
    a231:	96                   	xchg   %ax,%si
    a232:	74 0c                	je     0xa240
    a234:	97                   	xchg   %ax,%di
    a235:	c0 d1 f7             	rcl    $0xf7,%cl
    a238:	e2 41                	loop   0xa27b
    a23a:	06                   	push   %es
    a23b:	7e 8b                	jle    0xa1c8
    a23d:	f7 eb                	imul   %bx
    a23f:	eb 96                	jmp    0xa1d7
    a241:	5f                   	pop    %di
    a242:	ef                   	out    %ax,(%dx)
    a243:	d3 8d db 22          	rorw   %cl,0x22db(%di)
    a247:	5c                   	pop    %sp
    a248:	57                   	push   %di
    a249:	9b                   	fwait
    a24a:	4f                   	dec    %di
    a24b:	8b 57 4e             	mov    0x4e(%bx),%dx
    a24e:	08 5f 52             	or     %bl,0x52(%bx)
    a251:	cb                   	lret
    a252:	ff 90 89 58          	call   *0x5889(%bx,%si)
    a256:	20 92 05 fa          	and    %dl,-0x5fb(%bp,%si)
    a25a:	8d 7e ff             	lea    -0x1(%bp),%di
    a25d:	d1 d1                	rcl    $1,%cx
    a25f:	91                   	xchg   %ax,%cx
    a260:	5d                   	pop    %bp
    a261:	33 8b f7 bb          	xor    -0x4409(%bp,%di),%cx
    a265:	ba 5e 81             	mov    $0x815e,%dx
    a268:	58                   	pop    %ax
    a269:	ff 6a 00             	ljmp   *0x0(%bp,%si)
    a26c:	50                   	push   %ax
    a26d:	c5 2a                	lds    (%bp,%si),%bp
    a26f:	06                   	push   %es
    a270:	1f                   	pop    %ds
    a271:	05 9a b4             	add    $0xb49a,%ax
    a274:	40                   	inc    %ax
    a275:	8b 2b                	mov    (%bp,%di),%bp
    a277:	c0 d3 db             	rcl    $0xdb,%bl
    a27a:	5c                   	pop    %sp
    a27b:	9b                   	fwait
    a27c:	8b 4e 5f             	mov    0x5f(%bp),%cx
    a27f:	cb                   	lret
    a280:	90                   	nop
    a281:	b8 fc 00             	mov    $0xfc,%ax
    a284:	50                   	push   %ax
    a285:	e8 c1 ff             	call   0xa249
    a288:	83 3e 30 a4 74       	cmpw   $0x74,-0x5bd0
    a28d:	1e                   	push   %ds
    a28e:	2a 06 1f 30          	sub    0x301f,%al
    a292:	a4                   	movsb  %ds:(%si),%es:(%di)
    a293:	5c                   	pop    %sp
    a294:	f8                   	clc
    a295:	ff c0                	inc    %ax
    a297:	e8 af ff             	call   0xa249
    a29a:	e8 b8 08             	call   0xab55
    a29d:	c3                   	ret
    a29e:	8f 06 32 a4          	pop    -0x5bce
    a2a2:	b4 cd                	mov    $0xcd,%ah
    a2a4:	3c 73                	cmp    $0x73,%al
    a2a6:	a3 00 a4             	mov    %ax,0xa400
    a2a9:	ba 56 8b             	mov    $0x8b56,%dx
    a2ac:	3c 02                	cmp    $0x2,%al
    a2ae:	74 29                	je     0xa2d9
    a2b0:	8e 26 00 0e          	mov    0xe00,%fs
    a2b4:	00 8e 26 2c          	add    %cl,0x2c26(%bp)
    a2b8:	00 8c 06 23          	add    %cl,0x2306(%si)
    a2bc:	a4                   	movsb  %ds:(%si),%es:(%di)
    a2bd:	06                   	push   %es
    a2be:	99                   	cwtd
    a2bf:	18 b9 00 80          	sbb    %bh,-0x8000(%bx,%di)
    a2c3:	33 d1                	xor    %cx,%dx
    a2c5:	91                   	xchg   %ax,%cx
    a2c6:	5d                   	pop    %bp
    a2c7:	ae                   	scas   %es:(%di),%al
    a2c8:	75 fb                	jne    0xa2c5
    a2ca:	47                   	inc    %di
    a2cb:	47                   	inc    %di
    a2cc:	89 3e 21 a4          	mov    %di,-0x5bdf
    a2d0:	ff 75 06             	push   0x6(%di)
    a2d3:	d1 7e 8b             	sarw   $1,-0x75(%bp)
    a2d6:	a8 08                	test   $0x8,%al
    a2d8:	d1 bf 56 8b          	sarw   $1,-0x74aa(%bx)
    a2dc:	be 81 00             	mov    $0x81,%si
    a2df:	8e 1e a3 07          	mov    0x7a3,%ds
    a2e3:	ac                   	lods   %ds:(%si),%al
    a2e4:	3c 20                	cmp    $0x20,%al
    a2e6:	74 fb                	je     0xa2e3
    a2e8:	3c 09                	cmp    $0x9,%al
    a2ea:	74 f7                	je     0xa2e3
    a2ec:	3c 0d                	cmp    $0xd,%al
    a2ee:	74 6f                	je     0xa35f
    a2f0:	b4 e2                	mov    $0xe2,%ah
    a2f2:	08 6b 47             	or     %ch,0x47(%bp,%di)
    a2f5:	4e                   	dec    %si
    a2f6:	ac                   	lods   %ds:(%si),%al
    a2f7:	3c 20                	cmp    $0x20,%al
    a2f9:	74 e8                	je     0xa2e3
    a2fb:	3c 09                	cmp    $0x9,%al
    a2fd:	74 e4                	je     0xa2e3
    a2ff:	3c 0d                	cmp    $0xd,%al
    a301:	74 5c                	je     0xa35f
    a303:	b4 e2                	mov    $0xe2,%ah
    a305:	08 58 3c             	or     %bl,0x3c(%bx,%si)
    a308:	22 74 24             	and    0x24(%si),%dh
    a30b:	3c 5c                	cmp    $0x5c,%al
    a30d:	74 03                	je     0xa312
    a30f:	42                   	inc    %dx
    a310:	eb e4                	jmp    0xa2f6
    a312:	33 c9                	xor    %cx,%cx
    a314:	41                   	inc    %cx
    a315:	ac                   	lods   %ds:(%si),%al
    a316:	3c 5c                	cmp    $0x5c,%al
    a318:	74 fa                	je     0xa314
    a31a:	3c 22                	cmp    $0x22,%al
    a31c:	74 04                	je     0xa322
    a31e:	03 d1                	add    %cx,%dx
    a320:	eb d3                	jmp    0xa2f5
    a322:	8b c1                	mov    %cx,%ax
    a324:	d1 e9                	shr    $1,%cx
    a326:	13 d1                	adc    %cx,%dx
    a328:	a8 01                	test   $0x1,%al
    a32a:	75 ca                	jne    0xa2f6
    a32c:	eb 01                	jmp    0xa32f
    a32e:	4e                   	dec    %si
    a32f:	ac                   	lods   %ds:(%si),%al
    a330:	3c 0d                	cmp    $0xd,%al
    a332:	74 2b                	je     0xa35f
    a334:	b4 e2                	mov    $0xe2,%ah
    a336:	08 27                	or     %ah,(%bx)
    a338:	3c 22                	cmp    $0x22,%al
    a33a:	74 ba                	je     0xa2f6
    a33c:	3c 74                	cmp    $0x74,%al
    a33e:	42                   	inc    %dx
    a33f:	e4 c9                	in     $0xc9,%al
    a341:	ac                   	lods   %ds:(%si),%al
    a342:	57                   	push   %di
    a343:	c9                   	leave
    a344:	1a 41 3c             	sbb    0x3c(%bx,%di),%al
    a347:	74 3c                	je     0xa385
    a349:	74 03                	je     0xa34e
    a34b:	eb 8b                	jmp    0xa2d8
    a34d:	d1 13                	rclw   $1,(%bp,%di)
    a34f:	a8 75                	test   $0x75,%al
    a351:	eb db                	jmp    0xa32e
    a353:	8b d1                	mov    %cx,%dx
    a355:	13 a8 75 eb          	adc    -0x148b(%bx,%si),%bp
    a359:	4e                   	dec    %si
    a35a:	3c 74                	cmp    $0x74,%al
    a35c:	d2 eb                	shr    %cl,%bl
    a35e:	97                   	xchg   %ax,%di
    a35f:	16                   	push   %ss
    a360:	1f                   	pop    %ds
    a361:	89 3e 1b a4          	mov    %di,-0x5be5
    a365:	03 d7                	add    %di,%dx
    a367:	47                   	inc    %di
    a368:	d1 e7                	shl    $1,%di
    a36a:	03 d7                	add    %di,%dx
    a36c:	42                   	inc    %dx
    a36d:	80 e2 fe             	and    $0xfe,%dl
    a370:	2b e2                	sub    %dx,%sp
    a372:	8b c4                	mov    %sp,%ax
    a374:	a3 1d a4             	mov    %ax,0xa41d
    a377:	8b d8                	mov    %ax,%bx
    a379:	03 fb                	add    %bx,%di
    a37b:	16                   	push   %ss
    a37c:	07                   	pop    %es
    a37d:	36 89 3f             	mov    %di,%ss:(%bx)
    a380:	43                   	inc    %bx
    a381:	43                   	inc    %bx
    a382:	c5 36 21 a4          	lds    -0x5bdf,%si
    a386:	ac                   	lods   %ds:(%si),%al
    a387:	aa                   	stos   %al,%es:(%di)
    a388:	0a fe                	or     %dh,%bh
    a38a:	92                   	xchg   %ax,%dx
    a38b:	fa                   	cli
    a38c:	36 8e 1e a3 07       	mov    %ss:0x7a3,%ds
    a391:	be 81 00             	mov    $0x81,%si
    a394:	eb 03                	jmp    0xa399
    a396:	06                   	push   %es
    a397:	99                   	cwtd
    a398:	aa                   	stos   %al,%es:(%di)
    a399:	ac                   	lods   %ds:(%si),%al
    a39a:	20 fb                	and    %bh,%bl
    a39c:	09 f7                	or     %si,%di
    a39e:	0d 6f e2             	or     $0xe26f,%ax
    a3a1:	6b 4e 3c 74          	imul   $0x74,0x3c(%bp),%cx
    a3a5:	7c b4                	jl     0xa35b
    a3a7:	e2 08                	loop   0xa3b1
    a3a9:	78 36                	js     0xa3e1
    a3ab:	89 3f                	mov    %di,(%bx)
    a3ad:	43                   	inc    %bx
    a3ae:	43                   	inc    %bx
    a3af:	4e                   	dec    %si
    a3b0:	ac                   	lods   %ds:(%si),%al
    a3b1:	3c 20                	cmp    $0x20,%al
    a3b3:	74 e1                	je     0xa396
    a3b5:	0d 6f e2             	or     $0xe26f,%ax
    a3b8:	dd 4e 3c             	fisttpll 0x3c(%bp)
    a3bb:	74 62                	je     0xa41f
    a3bd:	b4 e2                	mov    $0xe2,%ah
    a3bf:	08 5e eb             	or     %bl,-0x15(%bp)
    a3c2:	8b d1                	mov    %cx,%dx
    a3c4:	27                   	daa
    a3c5:	3c 74                	cmp    $0x74,%al
    a3c7:	42                   	inc    %dx
    a3c8:	e4 aa                	in     $0xaa,%al
    a3ca:	eb 33                	jmp    0xa3ff
    a3cc:	41                   	inc    %cx
    a3cd:	3c 74                	cmp    $0x74,%al
    a3cf:	3c 74                	cmp    $0x74,%al
    a3d1:	03 eb                	add    %bx,%bp
    a3d3:	8b d1                	mov    %cx,%dx
    a3d5:	13 a8 06 b0          	adc    -0x4ffa(%bx,%si),%bp
    a3d9:	5c                   	pop    %sp
    a3da:	aa                   	stos   %al,%es:(%di)
    a3db:	8a 3b                	mov    (%bp,%di),%bh
    a3dd:	d1 b0 5c 13          	shlw   $1,0x135c(%bx,%si)
    a3e1:	5d                   	pop    %bp
    a3e2:	58                   	pop    %ax
    a3e3:	aa                   	stos   %al,%es:(%di)
    a3e4:	73 06                	jae    0xa3ec
    a3e6:	b0 22                	mov    $0x22,%al
    a3e8:	8a 3b                	mov    (%bp,%di),%bh
    a3ea:	c5 4e ac             	lds    -0x54(%bp),%cx
    a3ed:	3c 0d                	cmp    $0xd,%al
    a3ef:	74 2e                	je     0xa41f
    a3f1:	b4 e2                	mov    $0xe2,%ah
    a3f3:	08 2a                	or     %ch,(%bp,%si)
    a3f5:	d1 13                	rclw   $1,(%bp,%di)
    a3f7:	a8 b7                	test   $0xb7,%al
    a3f9:	3c 42                	cmp    $0x42,%al
    a3fb:	aa                   	stos   %al,%es:(%di)
    a3fc:	33 3c                	xor    (%si),%di
    a3fe:	3c 57                	cmp    $0x57,%al
    a400:	1a 3c                	sbb    (%si),%bh
    a402:	3c 03                	cmp    $0x3,%al
    a404:	8b 13                	mov    (%bp,%di),%dx
    a406:	75 db                	jne    0xa3e3
    a408:	d1 a8 eb 06          	shrw   $1,0x6eb(%bx,%si)
    a40c:	5c                   	pop    %sp
    a40d:	8a d1                	mov    %cl,%dl
    a40f:	5c                   	pop    %sp
    a410:	5d                   	pop    %bp
    a411:	d9 b0 13 58          	fnstenv 0x5813(%bx,%si)
    a415:	73 b0                	jae    0xa3c7
    a417:	8a c5                	mov    %ch,%al
    a419:	96                   	xchg   %ax,%si
    a41a:	b0 22                	mov    $0x22,%al
    a41c:	8a 3b                	mov    (%bp,%di),%bh
    a41e:	cd 06                	int    $0x6
    a420:	99                   	cwtd
    a421:	aa                   	stos   %al,%es:(%di)
    a422:	16                   	push   %ss
    a423:	1f                   	pop    %ds
    a424:	c7 07 00 00          	movw   $0x0,(%bx)
    a428:	ff 26 32 a4          	jmp    *-0x5bce
    a42c:	7e 53                	jle    0xa481
    a42e:	b0 d8                	mov    $0xd8,%al
    a430:	8e 26 00 0e          	mov    0xe00,%fs
    a434:	00 3e 1e 2c          	add    %bh,0x2c1e
    a438:	00 8e c3 06          	add    %cl,0x6c3(%bp)
    a43c:	99                   	cwtd
    a43d:	33 f6                	xor    %si,%si
    a43f:	33 ff                	xor    %di,%di
    a441:	ff 06 7e 59          	incw   0x597e
    a445:	0e                   	push   %cs
    a446:	04 0e                	add    $0xe,%al
    a448:	26 80 3e 00 00 00    	cmpb   $0x0,%es:0x0
    a44e:	eb 06                	jmp    0xa456
    a450:	f2 ae                	repnz scas %es:(%di),%al
    a452:	46                   	inc    %si
    a453:	ae                   	scas   %es:(%di),%al
    a454:	92                   	xchg   %ax,%dx
    a455:	fa                   	cli
    a456:	8b c7                	mov    %di,%ax
    a458:	40                   	inc    %ax
    a459:	24 fe                	and    $0xfe,%al
    a45b:	46                   	inc    %si
    a45c:	8b fe                	mov    %si,%di
    a45e:	d1 e6                	shl    $1,%si
    a460:	b9 09 00             	mov    $0x9,%cx
    a463:	e8 6c 01             	call   0xa5d2
    a466:	50                   	push   %ax
    a467:	8b c6                	mov    %si,%ax
    a469:	e8 66 01             	call   0xa5d2
    a46c:	a3 1f a4             	mov    %ax,0xa41f
    a46f:	06                   	push   %es
    a470:	07                   	pop    %es
    a471:	8b bf a4 cf          	mov    -0x305c(%bx),%di
    a475:	8b d8                	mov    %ax,%bx
    a477:	33 f6                	xor    %si,%si
    a479:	5f                   	pop    %di
    a47a:	49                   	dec    %cx
    a47b:	e3 00                	jcxz   0xa47d
    a47d:	3e 04 36             	ds add $0x36,%al
    a480:	3b 06 dc a3          	cmp    -0x5c24,%ax
    a484:	75 10                	jne    0xa496
    a486:	51                   	push   %cx
    a487:	56                   	push   %si
    a488:	57                   	push   %di
    a489:	bf dc a3             	mov    $0xa3dc,%di
    a48c:	b9 06 00             	mov    $0x6,%cx
    a48f:	f3 a7                	repz cmpsw %es:(%di),%ds:(%si)
    a491:	5f                   	pop    %di
    a492:	59                   	pop    %cx
    a493:	a6                   	cmpsb  %es:(%di),%ds:(%si)
    a494:	74 05                	je     0xa49b
    a496:	26 89 3f             	mov    %di,%es:(%bx)
    a499:	43                   	inc    %bx
    a49a:	43                   	inc    %bx
    a49b:	ac                   	lods   %ds:(%si),%al
    a49c:	0a 92 36 1e          	or     0x1e36(%bp,%si),%dl
    a4a0:	07                   	pop    %es
    a4a1:	e2 da                	loop   0xa47d
    a4a3:	26 89 0f             	mov    %cx,%es:(%bx)
    a4a6:	1f                   	pop    %ds
    a4a7:	33 8b f7 b8          	xor    -0x4709(%bp,%di),%cx
    a4ab:	02 00                	add    (%bx,%si),%al
    a4ad:	e9 94 f6             	jmp    0x9b44
    a4b0:	56                   	push   %si
    a4b1:	33 f6                	xor    %si,%si
    a4b3:	b9 42 00             	mov    $0x42,%cx
    a4b6:	32 e4                	xor    %ah,%ah
    a4b8:	fc                   	cld
    a4b9:	ac                   	lods   %ds:(%si),%al
    a4ba:	32 e0                	xor    %al,%ah
    a4bc:	e2 fb                	loop   0xa4b9
    a4be:	80 f4 55             	xor    $0x55,%ah
    a4c1:	74 0d                	je     0xa4d0
    a4c3:	e8 b8 fd             	call   0xa27e
    a4c6:	b8 01 ff             	mov    $0xff01,%ax
    a4c9:	e8 ff 7c             	call   0x21cb
    a4cc:	fd                   	std
    a4cd:	b8 01 ff             	mov    $0xff01,%ax
    a4d0:	5e                   	pop    %si
    a4d1:	c3                   	ret
    a4d2:	72 15                	jb     0xa4e9
    a4d4:	33 75 f3             	xor    -0xd(%di),%si
    a4d7:	b8 c3 06             	mov    $0x6c3,%ax
    a4da:	73 f8                	jae    0xa4d4
    a4dc:	e8 ff 1a             	call   0xbfde
    a4df:	00 58 32             	add    %bl,0x32(%bx,%si)
    a4e2:	e4 f3                	in     $0xf3,%al
    a4e4:	b8 c3 06             	mov    $0x6c3,%ax
    a4e7:	73 07                	jae    0xa4f0
    a4e9:	e8 0e b8             	call   0x5cfa
    a4ec:	35 06 7e             	xor    $0x7e06,%ax
    a4ef:	99                   	cwtd
    a4f0:	f3 b8 c3 06          	repz mov $0x6c3,%ax
    a4f4:	32 e4                	xor    %ah,%ah
    a4f6:	e8 01 ff             	call   0xa3fa
    a4f9:	c3                   	ret
    a4fa:	a2 03 a4             	mov    %al,0xa403
    a4fd:	0a 75 08             	or     0x8(%di),%dh
    a500:	22 80 3e 00          	and    0x3e(%bx,%si),%al
    a504:	a4                   	movsb  %ds:(%si),%es:(%di)
    a505:	03 72 0c             	add    0xc(%bp,%si),%si
    a508:	3c 22                	cmp    $0x22,%al
    a50a:	73 0c                	jae    0xa518
    a50c:	3c 20                	cmp    $0x20,%al
    a50e:	72 04                	jb     0xa514
    a510:	b0 05                	mov    $0x5,%al
    a512:	eb 06                	jmp    0xa51a
    a514:	3c 13                	cmp    $0x13,%al
    a516:	76 02                	jbe    0xa51a
    a518:	b0 13                	mov    $0x13,%al
    a51a:	bb 36 a5             	mov    $0xa536,%bx
    a51d:	d7                   	xlat   %ds:(%bx)
    a51e:	98                   	cbtw
    a51f:	a3 f8 a3             	mov    %ax,0xa3f8
    a522:	c3                   	ret
    a523:	8a c4                	mov    %ah,%al
    a525:	eb f7                	jmp    0xa51e
    a527:	15 04 8b             	adc    $0x8b04,%ax
    a52a:	0a 07                	or     (%bx),%al
    a52c:	ff                   	lcall  (bad)
    a52d:	d8 76 77             	fdivs  0x77(%bp)
    a530:	eb ea                	jmp    0xa51c
    a532:	7e 08                	jle    0xa53c
    a534:	8a 7e 88             	mov    -0x78(%bp),%bh
    a537:	fc                   	cld
    a538:	93                   	xchg   %ax,%bx
    a539:	b4 08                	mov    $0x8,%ah
    a53b:	d1 13                	rclw   $1,(%bp,%di)
    a53d:	83 0a 01             	orw    $0x1,(%bp,%si)
    a540:	03 0e 60 0f          	add    0xf60,%cx
    a544:	41                   	inc    %cx
    a545:	0a b0 2d aa          	or     -0x55d3(%bx,%si),%dh
    a549:	f7 db                	neg    %bx
    a54b:	83 d2 00             	adc    $0x0,%dx
    a54e:	83 8b a1 f7 92       	orw    $0xff92,-0x85f(%bp,%di)
    a553:	33 d2                	xor    %dx,%dx
    a555:	90                   	nop
    a556:	58                   	pop    %ax
    a557:	92                   	xchg   %ax,%dx
    a558:	02 f7                	add    %bh,%dh
    a55a:	f1                   	int1
    a55b:	93                   	xchg   %ax,%bx
    a55c:	f7 f1                	div    %cx
    a55e:	92                   	xchg   %ax,%dx
    a55f:	87 d3                	xchg   %dx,%bx
    a561:	04 30                	add    $0x30,%al
    a563:	3c 39                	cmp    $0x39,%al
    a565:	76 02                	jbe    0xa569
    a567:	04 27                	add    $0x27,%al
    a569:	8b c2                	mov    %dx,%ax
    a56b:	56                   	push   %si
    a56c:	0b c3                	or     %bx,%ax
    a56e:	75 e2                	jne    0xa552
    a570:	88 05                	mov    %al,(%di)
    a572:	4f                   	dec    %di
    a573:	ac                   	lods   %ds:(%si),%al
    a574:	86 05                	xchg   %al,(%di)
    a576:	88 44 ff             	mov    %al,-0x1(%si)
    a579:	8d 44 01             	lea    0x1(%si),%ax
    a57c:	3b c7                	cmp    %di,%ax
    a57e:	72 f2                	jb     0xa572
    a580:	58                   	pop    %ax
    a581:	5f                   	pop    %di
    a582:	d3 db                	rcr    %cl,%bx
    a584:	5c                   	pop    %sp
    a585:	9b                   	fwait
    a586:	4c                   	dec    %sp
    a587:	7e 06                	jle    0xa58f
    a589:	0e                   	push   %cs
    a58a:	06                   	push   %es
    a58b:	06                   	push   %es
    a58c:	07                   	pop    %es
    a58d:	4e                   	dec    %si
    a58e:	d3 be c4 a3          	sarw   %cl,-0x5c3c(%bp)
    a592:	39 5c 06             	cmp    %bx,0x6(%si)
    a595:	73 0d                	jae    0xa5a4
    a597:	4b                   	dec    %bx
    a598:	4b                   	dec    %bx
    a599:	80 0f 01             	orb    $0x1,(%bx)
    a59c:	39 5c 08             	cmp    %bx,0x8(%si)
    a59f:	76 03                	jbe    0xa5a4
    a5a1:	89 5c 08             	mov    %bx,0x8(%si)
    a5a4:	d3 db                	rcr    %cl,%bx
    a5a6:	5c                   	pop    %sp
    a5a7:	9b                   	fwait
    a5a8:	4c                   	dec    %sp
    a5a9:	3b 01                	cmp    (%bx,%di),%ax
    a5ab:	8a 7e 88             	mov    -0x78(%bp),%bh
    a5ae:	87 c0                	xchg   %ax,%ax
    a5b0:	2a 83 0a e8          	sub    -0x17f6(%bp,%di),%al
    a5b4:	77 12                	ja     0xa5c8
    a5b6:	bb a3 8c             	mov    $0x8ca3,%bx
    a5b9:	e8 38 01             	call   0xa6f4
    a5bc:	73 0f                	jae    0xa5cd
    a5be:	e8 35 00             	call   0xa5f6
    a5c1:	72 05                	jb     0xa5c8
    a5c3:	e8 2e 01             	call   0xa6f4
    a5c6:	73 33                	jae    0xa5fb
    a5c8:	cb                   	lret
    a5c9:	eb 50                	jmp    0xa61b
    a5cb:	1b 00                	sbb    (%bx,%si),%ax
    a5cd:	f9                   	stc
    a5ce:	00 9b 4c 7e          	add    %bl,0x7e4c(%bp,%di)
    a5d2:	53                   	push   %bx
    a5d3:	06                   	push   %es
    a5d4:	51                   	push   %cx
    a5d5:	b9 00 04             	mov    $0x400,%cx
    a5d8:	87 0e 4a a5          	xchg   %cx,-0x5ab6
    a5dc:	51                   	push   %cx
    a5dd:	50                   	push   %ax
    a5de:	e8 97 fb             	call   0xa178
    a5e1:	5b                   	pop    %bx
    a5e2:	8f 06 4a a5          	pop    -0x5ab6
    a5e6:	59                   	pop    %cx
    a5e7:	8c da                	mov    %ds,%dx
    a5e9:	90                   	nop
    a5ea:	58                   	pop    %ax
    a5eb:	92                   	xchg   %ax,%dx
    a5ec:	03 07                	add    (%bx),%ax
    a5ee:	5b                   	pop    %bx
    a5ef:	c3                   	ret
    a5f0:	8b c1                	mov    %cx,%ax
    a5f2:	e9 4f f5             	jmp    0x9b44
    a5f5:	00 51 57             	add    %dl,0x57(%bx,%di)
    a5f8:	f6 47 02 01          	testb  $0x1,0x2(%bx)
    a5fc:	74 63                	je     0xa661
    a5fe:	e8 d2 00             	call   0xa6d3
    a601:	8b fe                	mov    %si,%di
    a603:	8b 04                	mov    (%si),%ax
    a605:	a8 08                	test   $0x8,%al
    a607:	d1 03                	rolw   $1,(%bp,%di)
    a609:	2b c8                	sub    %ax,%cx
    a60b:	49                   	dec    %cx
    a60c:	41                   	inc    %cx
    a60d:	41                   	inc    %cx
    a60e:	8b 77 76             	mov    0x76(%bx),%si
    a611:	f8                   	clc
    a612:	4a                   	dec    %dx
    a613:	02 4c 03             	add    0x3(%si),%cl
    a616:	ce                   	into
    a617:	73 09                	jae    0xa622
    a619:	cb                   	lret
    a61a:	eb ba                	jmp    0xa5d6
    a61c:	f0 ff e3             	lock jmp *%bx
    a61f:	30 eb                	xor    %ch,%bl
    a621:	3f                   	aas
    a622:	16                   	push   %ss
    a623:	07                   	pop    %es
    a624:	26 a1 4a a5          	mov    %es:0xa54a,%ax
    a628:	3d 00 20             	cmp    $0x2000,%ax
    a62b:	74 16                	je     0xa643
    a62d:	ba 00 80             	mov    $0x8000,%dx
    a630:	3b d0                	cmp    %ax,%dx
    a632:	72 06                	jb     0xa63a
    a634:	d1 ea                	shr    $1,%dx
    a636:	75 f8                	jne    0xa630
    a638:	eb 22                	jmp    0xa65c
    a63a:	83 fa 08             	cmp    $0x8,%dx
    a63d:	72 1d                	jb     0xa65c
    a63f:	d1 8b b6 c2          	rorw   $1,-0x3d4a(%bp,%di)
    a643:	48                   	dec    %ax
    a644:	8b d0                	mov    %ax,%dx
    a646:	03 c1                	add    %cx,%ax
    a648:	73 02                	jae    0xa64c
    a64a:	cb                   	lret
    a64b:	eb f7                	jmp    0xa644
    a64d:	d2 23                	shlb   %cl,(%bp,%di)
    a64f:	c2 52 e8             	ret    $0xe852
    a652:	2e 00 5a 73          	add    %bl,%cs:0x73(%bp,%si)
    a656:	0d 83 fa             	or     $0xfa83,%ax
    a659:	f0 74 05             	lock je 0xa661
    a65c:	b8 10 00             	mov    $0x10,%ax
    a65f:	eb e9                	jmp    0xa64a
    a661:	eb 60                	jmp    0xa6c3
    a663:	1b 8b d0 2b          	sbb    0x2bd0(%bp,%di),%cx
    a667:	57                   	push   %di
    a668:	04 89                	add    $0x89,%al
    a66a:	3b 46 48             	cmp    0x48(%bp),%ax
    a66d:	7f 47                	jg     0xa6b6
    a66f:	0a 8b ec 4a          	or     0x4aec(%bp,%di),%cl
    a673:	89 14                	mov    %dx,(%si)
    a675:	42                   	inc    %dx
    a676:	03 f2                	add    %dx,%si
    a678:	c7 04 fe ff          	movw   $0xfffe,(%si)
    a67c:	89 8b ec 5f          	mov    %cx,0x5fec(%bp,%di)
    a680:	59                   	pop    %cx
    a681:	c3                   	ret
    a682:	8b d0                	mov    %ax,%dx
    a684:	f6 47 02 04          	testb  $0x4,0x2(%bx)
    a688:	74 0f                	je     0xa699
    a68a:	4a                   	dec    %dx
    a68b:	8b 77 76             	mov    0x76(%bx),%si
    a68e:	4e                   	dec    %si
    a68f:	3b d6                	cmp    %si,%dx
    a691:	72 05                	jb     0xa698
    a693:	39 57 fe             	cmp    %dx,-0x2(%bx)
    a696:	73 36                	jae    0xa6ce
    a698:	42                   	inc    %dx
    a699:	53                   	push   %bx
    a69a:	51                   	push   %cx
    a69b:	8c de                	mov    %ds,%si
    a69d:	8e c6                	mov    %si,%es
    a69f:	d3 e0                	shl    %cl,%ax
    a6a1:	36 e8 75 03          	ss call 0xaa1a
    a6a5:	b8 00 10             	mov    $0x1000,%ax
    a6a8:	f6 47 02 04          	testb  $0x4,0x2(%bx)
    a6ac:	74 e1                	je     0xa68f
    a6ae:	c6                   	(bad)
    a6af:	0a 8b 1e 07          	or     0x71e(%bp,%di),%cl
    a6b3:	81 2b c3 8e          	subw   $0x8ec3,(%bp,%di)
    a6b7:	c3                   	ret
    a6b8:	8b d8                	mov    %ax,%bx
    a6ba:	b4 cd                	mov    $0xcd,%ah
    a6bc:	36 1e                	ss push %ds
    a6be:	59                   	pop    %cx
    a6bf:	5b                   	pop    %bx
    a6c0:	72 10                	jb     0xa6d2
    a6c2:	b6 c2                	mov    $0xc2,%dh
    a6c4:	f6 47 02 04          	testb  $0x4,0x2(%bx)
    a6c8:	74 04                	je     0xa6ce
    a6ca:	4a                   	dec    %dx
    a6cb:	89 57 fe             	mov    %dx,-0x2(%bx)
    a6ce:	f8                   	clc
    a6cf:	eb 01                	jmp    0xa6d2
    a6d1:	f9                   	stc
    a6d2:	c3                   	ret
    a6d3:	57                   	push   %di
    a6d4:	8b 77 08             	mov    0x8(%bx),%si
    a6d7:	3b 8b ec 75          	cmp    0x75ec(%bp,%di),%cx
    a6db:	03 8b 77 06          	add    0x677(%bp,%di),%cx
    a6df:	ad                   	lods   %ds:(%si),%ax
    a6e0:	3d fe ff             	cmp    $0xfffe,%ax
    a6e3:	74 05                	je     0xa6ea
    a6e5:	05 fe 24             	add    $0x24fe,%ax
    a6e8:	fe 03                	incb   (%bp,%di)
    a6ea:	f0 eb f2             	lock jmp 0xa6df
    a6ed:	4f                   	dec    %di
    a6ee:	8b e3                	mov    %bx,%sp
    a6f0:	f7 5f c3             	negw   -0x3d(%bx)
    a6f3:	00 41 80             	add    %al,-0x80(%bx,%di)
    a6f6:	e1 fe                	loope  0xa6f6
    a6f8:	53                   	push   %bx
    a6f9:	fc                   	cld
    a6fa:	8b 77 08             	mov    0x8(%bx),%si
    a6fd:	8b 0a                	mov    (%bp,%si),%cx
    a6ff:	d8 33                	fdivs  (%bp,%di)
    a701:	ff                   	ljmp   (bad)
    a702:	eb 23                	jmp    0xa727
    a704:	8b c3                	mov    %bx,%ax
    a706:	5b                   	pop    %bx
    a707:	4e                   	dec    %si
    a708:	74 eb                	je     0xa6f5
    a70a:	42                   	inc    %dx
    a70b:	53                   	push   %bx
    a70c:	8b 77 06             	mov    0x6(%bx),%si
    a70f:	8b 0a                	mov    (%bp,%si),%cx
    a711:	08 3b                	or     %bh,(%bp,%di)
    a713:	de 74 36             	fidivs 0x36(%si)
    a716:	4b                   	dec    %bx
    a717:	33 ff                	xor    %di,%di
    a719:	eb 0c                	jmp    0xa727
    a71b:	90                   	nop
    a71c:	8d 54 fe             	lea    -0x2(%si),%dx
    a71f:	3b d3                	cmp    %bx,%dx
    a721:	73 e1                	jae    0xa704
    a723:	03 f0                	add    %ax,%si
    a725:	72 23                	jb     0xa74a
    a727:	ad                   	lods   %ds:(%si),%ax
    a728:	a8 d1                	test   $0xd1,%al
    a72a:	2b f0                	sub    %ax,%si
    a72c:	05 fe 48             	add    $0x48fe,%ax
    a72f:	3b c1                	cmp    %cx,%ax
    a731:	73 23                	jae    0xa756
    a733:	03 f0                	add    %ax,%si
    a735:	72 13                	jb     0xa74a
    a737:	8b d0                	mov    %ax,%dx
    a739:	ad                   	lods   %ds:(%si),%ax
    a73a:	a8 d1                	test   $0xd1,%al
    a73c:	2b de                	sub    %si,%bx
    a73e:	03 c2                	add    %dx,%ax
    a740:	05 02 8b             	add    $0x8b02,%ax
    a743:	56                   	push   %si
    a744:	f7 89 44 fe eb e4    	testw  $0xe4eb,-0x1bc(%bx,%di)
    a74a:	8b c0                	mov    %ax,%ax
    a74c:	5b                   	pop    %bx
    a74d:	8b 06 77 a1          	mov    -0x5e89,%ax
    a751:	47                   	inc    %di
    a752:	08 eb                	or     %ch,%bl
    a754:	60                   	pusha
    a755:	19 5b 89             	sbb    %bx,-0x77(%bp,%di)
    a758:	4c                   	dec    %sp
    a759:	fe                   	(bad)
    a75a:	74 09                	je     0xa765
    a75c:	03 f9                	add    %cx,%di
    a75e:	2b c1                	sub    %cx,%ax
    a760:	05 06 05             	add    $0x506,%ax
    a763:	2b f9                	sub    %cx,%di
    a765:	03 f9                	add    %cx,%di
    a767:	48                   	dec    %ax
    a768:	7f 47                	jg     0xa7b1
    a76a:	0a c6                	or     %dh,%al
    a76c:	8c da                	mov    %ds,%dx
    a76e:	f8                   	clc
    a76f:	c3                   	ret
    a770:	63 00                	arpl   %ax,(%bx,%si)
    a772:	00 00                	add    %al,(%bx,%si)
    a774:	f8                   	clc
    a775:	50                   	push   %ax
    a776:	53                   	push   %bx
    a777:	51                   	push   %cx
    a778:	52                   	push   %dx
    a779:	55                   	push   %bp
    a77a:	57                   	push   %di
    a77b:	56                   	push   %si
    a77c:	1e                   	push   %ds
    a77d:	06                   	push   %es
    a77e:	b8 63 f9             	mov    $0xf963,%ax
    a781:	c3                   	ret
    a782:	00 f8                	add    %bh,%al
    a784:	53                   	push   %bx
    a785:	52                   	push   %dx
    a786:	57                   	push   %di
    a787:	1e                   	push   %ds
    a788:	b8 f9 00             	mov    $0xf9,%ax
    a78b:	53                   	push   %bx
    a78c:	57                   	push   %di
    a78d:	b8 00 57             	mov    $0x5700,%ax
    a790:	b4 8b                	mov    $0x8b,%ah
    a792:	1f                   	pop    %ds
    a793:	3b c3                	cmp    %bx,%ax
    a795:	b4 35                	mov    $0x35,%ah
    a797:	8b 1e c3 e8          	mov    -0x173d,%bx
    a79b:	f8                   	clc
    a79c:	ff                   	ljmp   (bad)
    a79d:	e8 f0 ff             	call   0xa790
    a7a0:	c3                   	ret
    a7a1:	33 d2                	xor    %dx,%dx
    a7a3:	03 c0                	add    %ax,%ax
    a7a5:	f7 d2                	not    %dx
    a7a7:	fe c9                	dec    %cl
    a7a9:	75 f8                	jne    0xa7a3
    a7ab:	c3                   	ret
    a7ac:	d1 0b                	rorw   $1,(%bp,%di)
    a7ae:	8b 03                	mov    (%bp,%di),%ax
    a7b0:	fe c9                	dec    %cl
    a7b2:	75 f8                	jne    0xa7ac
    a7b4:	c3                   	ret
    a7b5:	33 f6                	xor    %si,%si
    a7b7:	26 8b 8c 06 00       	mov    %es:0x6(%si),%cx
    a7bc:	41                   	inc    %cx
    a7bd:	74 2e                	je     0xa7ed
    a7bf:	26 8b 94 0a 00       	mov    %es:0xa(%si),%dx
    a7c4:	26 8b 85 74 0f       	mov    %es:0xf74(%di),%ax
    a7c9:	5b                   	pop    %bx
    a7ca:	2b c2                	sub    %dx,%ax
    a7cc:	58                   	pop    %ax
    a7cd:	73 09                	jae    0xa7d8
    a7cf:	8b c2                	mov    %dx,%ax
    a7d1:	26 8b 95 0c 00       	mov    %es:0xc(%di),%dx
    a7d6:	eb 05                	jmp    0xa7dd
    a7d8:	26 8b 94 0c 00       	mov    %es:0xc(%si),%dx
    a7dd:	2b c2                	sub    %dx,%ax
    a7df:	73 07                	jae    0xa7e8
    a7e1:	26 81 a4 06 00 ff 7f 	andw   $0x7fff,%es:0x6(%si)
    a7e8:	83 c6 10             	add    $0x10,%si
    a7eb:	eb ca                	jmp    0xa7b7
    a7ed:	c3                   	ret
    a7ee:	00 f8                	add    %bh,%al
    a7f0:	cb                   	lret
    a7f1:	8b ec                	mov    %sp,%bp
    a7f3:	c7 46 0e ff ff       	movw   $0xffff,0xe(%bp)
    a7f8:	b0 00                	mov    $0x0,%al
    a7fa:	cf                   	iret
    a7fb:	1e                   	push   %ds
    a7fc:	53                   	push   %bx
    a7fd:	bb da 0a             	mov    $0xada,%bx
    a800:	8e db                	mov    %bx,%ds
    a802:	f7 06 8c 00 ff ff    	testw  $0xffff,0x8c
    a808:	74 12                	je     0xa81c
    a80a:	a9 8a ec             	test   $0xec8a,%ax
    a80d:	02 8b 52 b4          	add    -0x4bae(%bp,%di),%cl
    a811:	cd e9                	int    $0xe9
    a813:	05 5a 58             	add    $0x585a,%ax
    a816:	c7 06 8c 00 00 00    	movw   $0x0,0x8c
    a81c:	5b                   	pop    %bx
    a81d:	1f                   	pop    %ds
    a81e:	cb                   	lret
    a81f:	8b 1e 8c 00          	mov    0x8c,%bx
    a823:	92                   	xchg   %ax,%dx
    a824:	1e                   	push   %ds
    a825:	e2 08                	loop   0xa82f
    a827:	26 3b 9d 00 00       	cmp    %es:0x0(%di),%bx
    a82c:	75 01                	jne    0xa82f
    a82e:	c3                   	ret
    a82f:	9a 7b 00 78 0a       	lcall  $0xa78,$0x7b
    a834:	26 8b 9d 00 00       	mov    %es:0x0(%di),%bx
    a839:	89 1e 8e 0f          	mov    %bx,0xf8e
    a83d:	5b                   	pop    %bx
    a83e:	8b 36 8e 0f          	mov    0xf8e,%si
    a842:	bf a8 00             	mov    $0xa8,%di
    a845:	26 ac                	lods   %es:(%si),%al
    a847:	36 74 75             	ss je  0xa8bf
    a84a:	92                   	xchg   %ax,%dx
    a84b:	1e                   	push   %ds
    a84c:	e2 f7                	loop   0xa845
    a84e:	58                   	pop    %ax
    a84f:	06                   	push   %es
    a850:	8b a4 e8 35          	mov    0x35e8(%si),%sp
    a854:	04 07                	add    $0x7,%al
    a856:	77 ef                	ja     0xa847
    a858:	74 02                	je     0xa85c
    a85a:	f9                   	stc
    a85b:	c3                   	ret
    a85c:	a0 01 00             	mov    0x1,%al
    a85f:	33 f6                	xor    %si,%si
    a861:	52                   	push   %dx
    a862:	b4 3d                	mov    $0x3d,%ah
    a864:	ba a8 00             	mov    $0xa8,%dx
    a867:	e9 05 5a             	jmp    0x26f
    a86a:	72 04                	jb     0xa870
    a86c:	a3 02 8b             	mov    %ax,0x8b02
    a86f:	c3                   	ret
    a870:	b0 01                	mov    $0x1,%al
    a872:	f8                   	clc
    a873:	02 03                	add    (%bp,%di),%al
    a875:	02 fe                	add    %dh,%bh
    a877:	c0 eb c3             	shr    $0xc3,%bl
    a87a:	26 8b 9d 10 00       	mov    %es:0x10(%di),%bx
    a87f:	03 da                	add    %dx,%bx
    a881:	26 8b 85 12 00       	mov    %es:0x12(%di),%ax
    a886:	15 07 b9             	adc    $0xb907,%ax
    a889:	fe 04                	incb   (%si)
    a88b:	00 d1                	add    %dl,%cl
    a88d:	e3 d1                	jcxz   0xa860
    a88f:	d0 e2                	shl    $1,%dl
    a891:	fa                   	cli
    a892:	8b c6                	mov    %si,%ax
    a894:	03 d3                	add    %bx,%dx
    a896:	03 40 02             	add    0x2(%bx,%si),%ax
    a899:	8b b0 00 52          	mov    0x5200(%bx,%si),%si
    a89d:	b4 42                	mov    $0x42,%ah
    a89f:	e7 05                	out    %ax,$0x5
    a8a1:	5a                   	pop    %dx
    a8a2:	c3                   	ret
    a8a3:	50                   	push   %ax
    a8a4:	52                   	push   %dx
    a8a5:	b8 00 e9             	mov    $0xe900,%ax
    a8a8:	f7 e1                	mul    %cx
    a8aa:	b9 f0 ff             	mov    $0xfff0,%cx
    a8ad:	d1 d1                	rcl    $1,%cx
    a8af:	0b ca                	or     %dx,%cx
    a8b1:	5a                   	pop    %dx
    a8b2:	2a 3f                	sub    (%bx),%bh
    a8b4:	05 7e 09             	add    $0x97e,%ax
    a8b7:	8b 03                	mov    (%bp,%di),%ax
    a8b9:	8b 01                	mov    (%bx,%di),%ax
    a8bb:	c3                   	ret
    a8bc:	50                   	push   %ax
    a8bd:	50                   	push   %ax
    a8be:	eb 06                	jmp    0xa8c6
    a8c0:	48                   	dec    %ax
    a8c1:	50                   	push   %ax
    a8c2:	51                   	push   %cx
    a8c3:	b9 f0 ff             	mov    $0xfff0,%cx
    a8c6:	52                   	push   %dx
    a8c7:	53                   	push   %bx
    a8c8:	a1 02 8b             	mov    0x8b02,%ax
    a8cb:	51                   	push   %cx
    a8cc:	1e                   	push   %ds
    a8cd:	8e db                	mov    %bx,%ds
    a8cf:	8b 33                	mov    (%bp,%di),%si
    a8d1:	5f                   	pop    %di
    a8d2:	e3 52                	jcxz   0xa926
    a8d4:	b4 3f                	mov    $0x3f,%ah
    a8d6:	e7 05                	out    %ax,$0x5
    a8d8:	5a                   	pop    %dx
    a8d9:	1f                   	pop    %ds
    a8da:	59                   	pop    %cx
    a8db:	72 08                	jb     0xa8e5
    a8dd:	4a                   	dec    %dx
    a8de:	75 8b                	jne    0xa86b
    a8e0:	2e 3b c1             	cs cmp %cx,%ax
    a8e3:	74 03                	je     0xa8e8
    a8e5:	f9                   	stc
    a8e6:	eb 01                	jmp    0xa8e9
    a8e8:	f8                   	clc
    a8e9:	5b                   	pop    %bx
    a8ea:	5a                   	pop    %dx
    a8eb:	59                   	pop    %cx
    a8ec:	58                   	pop    %ax
    a8ed:	72 06                	jb     0xa8f5
    a8ef:	81 c3 ff 0f          	add    $0xfff,%bx
    a8f3:	eb be                	jmp    0xa8b3
    a8f5:	b0 03                	mov    $0x3,%al
    a8f7:	f8                   	clc
    a8f8:	03 fe                	add    %si,%di
    a8fa:	eb 26                	jmp    0xa922
    a8fc:	9d                   	popf
    a8fd:	f9                   	stc
    a8fe:	c3                   	ret
    a8ff:	43                   	inc    %bx
    a900:	6f                   	outsw  %ds:(%si),(%dx)
    a901:	70 79                	jo     0xa97c
    a903:	72 69                	jb     0xa96e
    a905:	67 68 74 20          	addr32 push $0x2074
    a909:	28 43 29             	sub    %al,0x29(%bp,%di)
    a90c:	20 31                	and    %dh,(%bx,%di)
    a90e:	39 38                	cmp    %di,(%bx,%si)
    a910:	34 2c                	xor    $0x2c,%al
    a912:	20 31                	and    %dh,(%bx,%di)
    a914:	39 38                	cmp    %di,(%bx,%si)
    a916:	35 2c 20             	xor    $0x202c,%ax
    a919:	31 39                	xor    %di,(%bx,%di)
    a91b:	38 36 20 62          	cmp    %dh,0x6220
    a91f:	79 20                	jns    0xa941
    a921:	50                   	push   %ax
    a922:	68 6f 65             	push   $0x656f
    a925:	6e                   	outsb  %ds:(%si),(%dx)
    a926:	69 78 20 53 6f       	imul   $0x6f53,0x20(%bx,%si),%di
    a92b:	66 74 77             	data32 je 0xa9a5
    a92e:	61                   	popa
    a92f:	72 65                	jb     0xa996
    a931:	20 41 73             	and    %al,0x73(%bx,%di)
    a934:	73 6f                	jae    0xa9a5
    a936:	63 69 61             	arpl   %bp,0x61(%bx,%di)
    a939:	74 65                	je     0xa9a0
    a93b:	73 20                	jae    0xa95d
    a93d:	4c                   	dec    %sp
    a93e:	74 64                	je     0xa9a4
    a940:	2e 1e                	cs push %ds
    a942:	b0 24                	mov    $0x24,%al
    a944:	0e                   	push   %cs
    a945:	1f                   	pop    %ds
    a946:	ba 71 00             	mov    $0x71,%dx
    a949:	e8 4e fe             	call   0xa79a
    a94c:	1f                   	pop    %ds
    a94d:	53                   	push   %bx
    a94e:	06                   	push   %es
    a94f:	b8 ff 0a             	mov    $0xaff,%ax
    a952:	8e c0                	mov    %ax,%es
    a954:	b0 00                	mov    $0x0,%al
    a956:	57                   	push   %di
    a957:	e8 c5 fe             	call   0xa81f
    a95a:	5f                   	pop    %di
    a95b:	73 03                	jae    0xa960
    a95d:	eb 56                	jmp    0xa9b5
    a95f:	90                   	nop
    a960:	53                   	push   %bx
    a961:	26 9d                	es popf
    a963:	00 1e 0f 8b          	add    %bl,-0x74f1
    a967:	8e 8c 00 5b          	mov    0x5b00(%si),%cs
    a96b:	26 95                	es xchg %ax,%bp
    a96d:	00 08                	add    %cl,(%bx,%si)
    a96f:	00 83 c2 03          	add    %al,0x3c2(%bp,%di)
    a973:	d1 8b fe ea          	rorw   $1,-0x1502(%bp,%di)
    a977:	e8 00 26             	call   0xcf7a
    a97a:	a4                   	movsb  %ds:(%si),%es:(%di)
    a97b:	8b 8d 14 00          	mov    0x14(%di),%cx
    a97f:	8b 74 5b             	mov    0x5b(%si),%si
    a982:	0c 00                	or     $0x0,%al
    a984:	8b 2b                	mov    (%bp,%di),%bp
    a986:	85 0f                	test   %cx,(%bx)
    a988:	2b 3b                	sub    (%bp,%di),%di
    a98a:	73 03                	jae    0xa98f
    a98c:	5e                   	pop    %si
    a98d:	f4                   	hlt
    a98e:	2e 26 85 0f          	cs test %cx,%es:(%bx)
    a992:	2b 58 33             	sub    0x33(%bx,%si),%bx
    a995:	d2 e8                	shr    %cl,%al
    a997:	0a ff                	or     %bh,%bh
    a999:	72 bb                	jb     0xa956
    a99b:	33 d2                	xor    %dx,%dx
    a99d:	e8 da fe             	call   0xa87a
    a9a0:	a4                   	movsb  %ds:(%si),%es:(%di)
    a9a1:	8b 8d 08 00          	mov    0x8(%di),%cx
    a9a5:	33 db                	xor    %bx,%bx
    a9a7:	e3 0c                	jcxz   0xa9b5
    a9a9:	a4                   	movsb  %ds:(%si),%es:(%di)
    a9aa:	8b 16 00 00          	mov    0x0,%dx
    a9ae:	06                   	push   %es
    a9af:	e8 14 00             	call   0xa9c6
    a9b2:	07                   	pop    %es
    a9b3:	72 a1                	jb     0xa956
    a9b5:	9c                   	pushf
    a9b6:	59                   	pop    %cx
    a9b7:	1f                   	pop    %ds
    a9b8:	5a                   	pop    %dx
    a9b9:	b0 24                	mov    $0x24,%al
    a9bb:	e8 d2 fd             	call   0xa790
    a9be:	51                   	push   %cx
    a9bf:	9d                   	popf
    a9c0:	b8 da 8e             	mov    $0x8eda,%ax
    a9c3:	f7 d8                	neg    %ax
    a9c5:	c3                   	ret
    a9c6:	0b 03                	or     (%bp,%di),%ax
    a9c8:	8b 22                	mov    (%bp,%si),%sp
    a9ca:	51                   	push   %cx
    a9cb:	52                   	push   %dx
    a9cc:	8c d8                	mov    %ds,%ax
    a9ce:	ba 04 00             	mov    $0x4,%dx
    a9d1:	83 f9 20             	cmp    $0x20,%cx
    a9d4:	76 03                	jbe    0xa9d9
    a9d6:	b9 20 00             	mov    $0x20,%cx
    a9d9:	51                   	push   %cx
    a9da:	83 c1 03             	add    $0x3,%cx
    a9dd:	d1 e9                	shr    $1,%cx
    a9df:	d1 e9                	shr    $1,%cx
    a9e1:	e8 bf fe             	call   0xa8a3
    a9e4:	5b                   	pop    %bx
    a9e5:	5a                   	pop    %dx
    a9e6:	59                   	pop    %cx
    a9e7:	72 28                	jb     0xaa11
    a9e9:	be 04 00             	mov    $0x4,%si
    a9ec:	4b                   	dec    %bx
    a9ed:	8b 2c                	mov    (%si),%bp
    a9ef:	8b 44 02             	mov    0x2(%si),%ax
    a9f2:	0b ed                	or     %bp,%bp
    a9f4:	75 04                	jne    0xa9fa
    a9f6:	90                   	nop
    a9f7:	92                   	xchg   %ax,%dx
    a9f8:	07                   	pop    %es
    a9f9:	12 06 52 26          	adc    0x2652,%al
    a9fd:	ff 36 00 00          	push   0x0
    aa01:	03 c2                	add    %dx,%ax
    aa03:	8e c0                	mov    %ax,%es
    aa05:	5a                   	pop    %dx
    aa06:	26 01 56 00          	add    %dx,%es:0x0(%bp)
    aa0a:	5a                   	pop    %dx
    aa0b:	07                   	pop    %es
    aa0c:	83 c6 04             	add    $0x4,%si
    aa0f:	e2 b5                	loop   0xa9c6
    aa11:	c3                   	ret
    aa12:	ea 97 02 78 0a       	ljmp   $0xa78,$0x297
    aa17:	fc                   	cld
    aa18:	55                   	push   %bp
    aa19:	1e                   	push   %ds
    aa1a:	06                   	push   %es
    aa1b:	bb 0a db             	mov    $0xdb0a,%bx
    aa1e:	06                   	push   %es
    aa1f:	00 bb ff 0a          	add    %bh,0xaff(%bp,%di)
    aa23:	8e c3                	mov    %bx,%es
    aa25:	88 26 90 00          	mov    %ah,0x90
    aa29:	b8 e9 e1             	mov    $0xe1e9,%ax
    aa2c:	49                   	dec    %cx
    aa2d:	ff 8b 8b f8          	decw   -0x775(%bp,%di)
    aa31:	26 f7 85 08 4c 06 80 	testw  $0x8006,%es:0x4c08(%di)
    aa38:	75 0c                	jne    0xaa46
    aa3a:	51                   	push   %cx
    aa3b:	e8 77 fd             	call   0xa7b5
    aa3e:	59                   	pop    %cx
    aa3f:	e8 ff fe             	call   0xa941
    aa42:	72 16                	jb     0xaa5a
    aa44:	eb 00                	jmp    0xaa46
    aa46:	8b 81 8d 08          	mov    0x88d(%bx,%di),%ax
    aa4a:	4c                   	dec    %sp
    aa4b:	06                   	push   %es
    aa4c:	80 a4 8b 8d 08       	andb   $0x8,-0x7275(%si)
    aa51:	4c                   	dec    %sp
    aa52:	81 e1 ff 1f          	and    $0x1fff,%cx
    aa56:	75 d1                	jne    0xaa29
    aa58:	eb 0b                	jmp    0xaa65
    aa5a:	f6 06 90 00 ff       	testb  $0xff,0x90
    aa5f:	03 eb                	add    %bx,%bp
    aa61:	e8 29 03             	call   0xad8d
    aa64:	f9                   	stc
    aa65:	e8 24 03             	call   0xad8c
    aa68:	07                   	pop    %es
    aa69:	1f                   	pop    %ds
    aa6a:	5d                   	pop    %bp
    aa6b:	cb                   	lret
    aa6c:	9c                   	pushf
    aa6d:	50                   	push   %ax
    aa6e:	4a                   	dec    %dx
    aa6f:	10 40 55             	adc    %al,0x55(%bx,%si)
    aa72:	fc                   	cld
    aa73:	06                   	push   %es
    aa74:	41                   	inc    %cx
    aa75:	06                   	push   %es
    aa76:	cb                   	lret
    aa77:	b8 00 ff             	mov    $0xff00,%ax
    aa7a:	8e b0 57 8b          	mov    -0x74a9(%bx,%si),%?
    aa7e:	46                   	inc    %si
    aa7f:	16                   	push   %ss
    aa80:	8b 07                	mov    (%bx),%ax
    aa82:	b1 76                	mov    $0x76,%cl
    aa84:	14 83                	adc    $0x83,%al
    aa86:	46                   	inc    %si
    aa87:	14 5e                	adc    $0x5e,%al
    aa89:	f4                   	hlt
    aa8a:	0c b8                	or     $0xb8,%al
    aa8c:	da 8e f7 d8          	fimull -0x2709(%bp)
    aa90:	b4 00                	mov    $0x0,%ah
    aa92:	9a 92 02 78 0a       	lcall  $0xa78,$0x292
    aa97:	07                   	pop    %es
    aa98:	1f                   	pop    %ds
    aa99:	f9                   	stc
    aa9a:	9b                   	fwait
    aa9b:	7e 06                	jle    0xaaa3
    aa9d:	05 7e 58             	add    $0x587e,%ax
    aaa0:	9d                   	popf
    aaa1:	cb                   	lret
    aaa2:	fa                   	cli
    aaa3:	8c c7                	mov    %es,%di
    aaa5:	8c d6                	mov    %ss,%si
    aaa7:	8b cc                	mov    %sp,%cx
    aaa9:	ba ff 8e             	mov    $0x8eff,%dx
    aaac:	b0 c2                	mov    $0xc2,%al
    aaae:	26 8e 16 04 00       	mov    %es:0x4,%ss
    aab3:	bc 00 01             	mov    $0x100,%sp
    aab6:	fb                   	sti
    aab7:	8e c7                	mov    %di,%es
    aab9:	56                   	push   %si
    aaba:	51                   	push   %cx
    aabb:	50                   	push   %ax
    aabc:	41                   	inc    %cx
    aabd:	06                   	push   %es
    aabe:	2e 8c 06 6e 00       	mov    %es,%cs:0x6e
    aac3:	b8 da 8e             	mov    $0x8eda,%ax
    aac6:	f7 d8                	neg    %ax
    aac8:	e8 32 02             	call   0xacfd
    aacb:	00 ff                	add    %bh,%bh
    aacd:	8e b0 57 8c          	mov    -0x73a9(%bx,%si),%?
    aad1:	c8 26 2b 08          	enter  $0x2b26,$0x8
    aad5:	4c                   	dec    %sp
    aad6:	06                   	push   %es
    aad7:	26 a3 4c 06          	mov    %ax,%es:0x64c
    aadb:	80 3e 00 eb ff       	cmpb   $0xff,-0x1500
    aae0:	74 0c                	je     0xaaee
    aae2:	52                   	push   %dx
    aae3:	b4 09                	mov    $0x9,%ah
    aae5:	ba 8e 01             	mov    $0x18e,%dx
    aae8:	e7 5a                	out    %ax,$0x5a
    aaea:	59                   	pop    %cx
    aaeb:	e8 9f 02             	call   0xad8d
    aaee:	ba ff 8e             	mov    $0x8eff,%dx
    aaf1:	b0 c2                	mov    $0xc2,%al
    aaf3:	33 ff                	xor    %di,%di
    aaf5:	7e 01                	jle    0xaaf8
    aaf7:	00 74 0c             	add    %dh,0xc(%si)
    aafa:	8b 08                	mov    (%bx,%si),%cx
    aafc:	4c                   	dec    %sp
    aafd:	40                   	inc    %ax
    aafe:	74 1f                	je     0xab1f
    ab00:	26 85 4c 80          	test   %cx,%es:-0x80(%si)
    ab04:	0c e8                	or     $0xe8,%al
    ab06:	40                   	inc    %ax
    ab07:	74 10                	je     0xab19
    ab09:	57                   	push   %di
    ab0a:	51                   	push   %cx
    ab0b:	b4 9a                	mov    $0x9a,%ah
    ab0d:	02 0a                	add    (%bp,%si),%cl
    ab0f:	1f                   	pop    %ds
    ab10:	9b                   	fwait
    ab11:	06                   	push   %es
    ab12:	59                   	pop    %cx
    ab13:	5f                   	pop    %di
    ab14:	03 56 e8             	add    -0x18(%bp),%dx
    ab17:	74 02                	je     0xab1b
    ab19:	41                   	inc    %cx
    ab1a:	83 c7 10             	add    $0x10,%di
    ab1d:	eb d9                	jmp    0xaaf8
    ab1f:	07                   	pop    %es
    ab20:	1f                   	pop    %ds
    ab21:	58                   	pop    %ax
    ab22:	05 7e fa             	add    $0xfa7e,%ax
    ab25:	8b e1                	mov    %cx,%sp
    ab27:	8e d3                	mov    %bx,%ss
    ab29:	fb                   	sti
    ab2a:	ea 7c 9a b8 57       	ljmp   $0x57b8,$0x9a7c
    ab2f:	8b 3b                	mov    (%bp,%di),%di
    ab31:	09 ba d4 01          	or     %di,0x1d4(%bp,%si)
    ab35:	e7 5a                	out    %ax,$0x5a
    ab37:	b4 0a                	mov    $0xa,%ah
    ab39:	ba f8 00             	mov    $0xf8,%dx
    ab3c:	05 72 3b             	add    $0x3b72,%ax
    ab3f:	09 ba 45 02          	or     %di,0x245(%bp,%si)
    ab43:	05 72 a0             	add    $0xa072,%ax
    ab46:	fa                   	cli
    ab47:	00 3c                	add    %bh,(%si)
    ab49:	2e 74 28             	je,pn  0xab74
    ab4c:	8a 1e f9 00          	mov    0xf9,%bl
    ab50:	32 ff                	xor    %bh,%bh
    ab52:	c6 87 fa 00 00       	movb   $0x0,0xfa(%bx)
    ab57:	f6 b4 8a 84          	divb   -0x7b76(%si)
    ab5b:	fa                   	cli
    ab5c:	00 a8 ff 74          	add    %ch,0x74ff(%bx,%si)
    ab60:	11 3c                	adc    %di,(%si)
    ab62:	7b 73                	jnp    0xabd7
    ab64:	0a 3c                	or     (%si),%bh
    ab66:	61                   	popa
    ab67:	72 06                	jb     0xab6f
    ab69:	24 df                	and    $0xdf,%al
    ab6b:	88 84 fa 00          	mov    %al,0xfa(%si)
    ab6f:	46                   	inc    %si
    ab70:	eb e7                	jmp    0xab59
    ab72:	f8                   	clc
    ab73:	c3                   	ret
    ab74:	f9                   	stc
    ab75:	c3                   	ret
    ab76:	3c 3a                	cmp    $0x3a,%al
    ab78:	74 0a                	je     0xab84
    ab7a:	3c 2f                	cmp    $0x2f,%al
    ab7c:	74 06                	je     0xab84
    ab7e:	8b 75 d1             	mov    -0x2f(%di),%si
    ab81:	02 b0 5c c3          	add    -0x3ca4(%bx,%si),%dh
    ab85:	8e dd                	mov    %bp,%ds
    ab87:	56                   	push   %si
    ab88:	33 d2                	xor    %dx,%dx
    ab8a:	ac                   	lods   %ds:(%si),%al
    ab8b:	42                   	inc    %dx
    ab8c:	77 74                	ja     0xac02
    ab8e:	f9                   	stc
    ab8f:	08 3c                	or     %bh,(%si)
    ab91:	74 0d                	je     0xaba0
    ab93:	04 3c                	add    $0x3c,%al
    ab95:	3b 75 f2             	cmp    -0xe(%di),%si
    ab98:	4a                   	dec    %dx
    ab99:	5e                   	pop    %si
    ab9a:	8c c3                	mov    %es,%bx
    ab9c:	8e dd                	mov    %bp,%ds
    ab9e:	56                   	push   %si
    ab9f:	03 f2                	add    %dx,%si
    aba1:	4e                   	dec    %si
    aba2:	ac                   	lods   %ds:(%si),%al
    aba3:	8e db                	mov    %bx,%ds
    aba5:	e8 ce ff             	call   0xab76
    aba8:	b4 9a                	mov    $0x9a,%ah
    abaa:	75 03                	jne    0xabaf
    abac:	fe c4                	inc    %ah
    abae:	4a                   	dec    %dx
    abaf:	be a8 00             	mov    $0xa8,%si
    abb2:	56                   	push   %si
    abb3:	83 c6 4f             	add    $0x4f,%si
    abb6:	8b fe                	mov    %si,%di
    abb8:	42                   	inc    %dx
    abb9:	2b f2                	sub    %dx,%si
    abbb:	b9 50 00             	mov    $0x50,%cx
    abbe:	2b ca                	sub    %dx,%cx
    abc0:	4a                   	dec    %dx
    abc1:	fd                   	std
    abc2:	f3 a4                	rep movsb %ds:(%si),%es:(%di)
    abc4:	fc                   	cld
    abc5:	5f                   	pop    %di
    abc6:	5e                   	pop    %si
    abc7:	8e dd                	mov    %bp,%ds
    abc9:	ca f0 4e             	lret   $0x4ef0
    abcc:	9a 8e db 88 05       	lcall  $0x588,$0xdb8e
    abd1:	8a c4                	mov    %ah,%al
    abd3:	b4 9a                	mov    $0x9a,%ah
    abd5:	03 f0                	add    %ax,%si
    abd7:	c3                   	ret
    abd8:	80 3e 92 00 01       	cmpb   $0x1,0x92
    abdd:	76 29                	jbe    0xac08
    abdf:	8b 36 97 00          	mov    0x97,%si
    abe3:	f8                   	clc
    abe4:	fe                   	(bad)
    abe5:	26 21 1e a1 93       	and    %bx,%es:-0x6c5f
    abea:	00 8e d8 ac          	add    %cl,-0x5328(%bp)
    abee:	3c 3b                	cmp    $0x3b,%al
    abf0:	09 0d                	or     %cx,(%di)
    abf2:	e2 74                	loop   0xac68
    abf4:	0d f7 4e             	or     $0x4ef7,%ax
    abf7:	f6 04 ff             	testb  $0xff,(%si)
    abfa:	8c dd                	mov    %ds,%bp
    abfc:	1f                   	pop    %ds
    abfd:	74 09                	je     0xac08
    abff:	e8 83 ff             	call   0xab85
    ac02:	89 36 97 00          	mov    %si,0x97
    ac06:	f8                   	clc
    ac07:	c3                   	ret
    ac08:	f9                   	stc
    ac09:	c3                   	ret
    ac0a:	bf 00 ac             	mov    $0xac00,%di
    ac0d:	ff                   	(bad)
    ac0e:	7e 0e                	jle    0xac1e
    ac10:	b0 00                	mov    $0x0,%al
    ac12:	f2 ae                	repnz scas %es:(%di),%al
    ac14:	83 ef 05             	sub    $0x5,%di
    ac17:	be 9f b9             	mov    $0xb99f,%si
    ac1a:	04 d1                	add    $0xd1,%al
    ac1c:	d1 f3                	shl    $1,%bx
    ac1e:	b9 1b 11             	mov    $0x111b,%cx
    ac21:	8b 36 99 00          	mov    0x99,%si
    ac25:	8b 16 a3 00          	mov    0xa3,%dx
    ac29:	8b 2e 93 00          	mov    0x93,%bp
    ac2d:	8e dd                	mov    %bp,%ds
    ac2f:	e9 68 ff             	jmp    0xab9a
    ac32:	8b 36 99 00          	mov    0x99,%si
    ac36:	1e                   	push   %ds
    ac37:	8e 1e 93 00          	mov    0x93,%ds
    ac3b:	bf 00 ac             	mov    $0xac00,%di
    ac3e:	ff 50 00             	call   *0x0(%bx,%si)
    ac41:	4e                   	dec    %si
    ac42:	9a 1f c3 50 3b       	lcall  $0x3b50,$0xc31f
    ac47:	ba 02 b8             	mov    $0xb802,%dx
    ac4a:	01 5a 0a             	add    %bx,0xa(%bp,%si)
    ac4d:	58                   	pop    %ax
    ac4e:	ba 21 02             	mov    $0x221,%dx
    ac51:	3c 03                	cmp    $0x3,%al
    ac53:	72 03                	jb     0xac58
    ac55:	ba 32 02             	mov    $0x232,%dx
    ac58:	3b ba 5a 0a          	cmp    0xa5a(%bp,%si),%di
    ac5c:	f6 06 a7 eb 74       	testb  $0x74,-0x1459
    ac61:	52                   	push   %dx
    ac62:	14 be                	adc    $0xbe,%al
    ac64:	fa                   	cli
    ac65:	00 e8                	add    %ch,%al
    ac67:	11 00                	adc    %ax,(%bx,%si)
    ac69:	4e                   	dec    %si
    ac6a:	8a 04                	mov    (%si),%al
    ac6c:	e8 07 74             	call   0x2076
    ac6f:	52                   	push   %dx
    ac70:	06                   	push   %es
    ac71:	b2 5c                	mov    $0x5c,%dl
    ac73:	b4 02                	mov    $0x2,%ah
    ac75:	72 fa                	jb     0xac71
    ac77:	be a8 00             	mov    $0xa8,%si
    ac7a:	8a 14                	mov    (%si),%dl
    ac7c:	0a 74 91             	or     -0x6f(%si),%dh
    ac7f:	09 56 b4             	or     %dx,-0x4c(%bp)
    ac82:	02 72 fa             	add    -0x6(%bp,%si),%dh
    ac85:	5e                   	pop    %si
    ac86:	46                   	inc    %si
    ac87:	eb f1                	jmp    0xac7a
    ac89:	c3                   	ret
    ac8a:	fc                   	cld
    ac8b:	77 74                	ja     0xad01
    ac8d:	f9                   	stc
    ac8e:	04 3c                	add    $0x3c,%al
    ac90:	05 75 19             	add    $0x1975,%ax
    ac93:	c6 06 a5 00 00       	movb   $0x0,0xa5
    ac98:	c6 06 a6 00 00       	movb   $0x0,0xa6
    ac9d:	c6 06 a7 eb 00       	movb   $0x0,-0x1459
    aca2:	8b 1e 95 0f          	mov    0xf95,%bx
    aca6:	8e 00                	mov    (%bx,%si),%es
    aca8:	97                   	xchg   %ax,%di
    aca9:	00 eb                	add    %ch,%bl
    acab:	45                   	inc    %bp
    acac:	f6 06 a5 00 ff       	testb  $0xff,0xa5
    acb1:	08 80 00 03          	or     %al,0x300(%bx,%si)
    acb5:	92                   	xchg   %ax,%dx
    acb6:	00 03                	add    %al,(%bp,%di)
    acb8:	72 10                	jb     0xacca
    acba:	f6 06 a6 00 ff       	testb  $0xff,0xa6
    acbf:	08 09                	or     %cl,(%bx,%di)
    acc1:	e8 46 ff             	call   0xac0a
    acc4:	fe 06 a6 00          	incb   0xa6
    acc8:	eb 27                	jmp    0xacf1
    acca:	e8 0b ff             	call   0xabd8
    accd:	73 22                	jae    0xacf1
    accf:	fe 06 a5 00          	incb   0xa5
    acd3:	eb 14                	jmp    0xace9
    acd5:	f6 90 ff eb          	notb   -0x1401(%bx,%si)
    acd9:	29 f9                	sub    %di,%cx
    acdb:	18 e8                	sbb    %ch,%al
    acdd:	66 ff c6             	inc    %esi
    ace0:	06                   	push   %es
    ace1:	a7                   	cmpsw  %es:(%di),%ds:(%si)
    ace2:	eb 01                	jmp    0xace5
    ace4:	e8 49 fe             	call   0xab30
    ace7:	72 0b                	jb     0xacf4
    ace9:	8c dd                	mov    %ds,%bp
    aceb:	be fa 00             	mov    $0xfa,%si
    acee:	e8 94 fe             	call   0xab85
    acf1:	b0 00                	mov    $0x0,%al
    acf3:	c3                   	ret
    acf4:	01 02                	add    %ax,(%bp,%si)
    acf6:	c3                   	ret
    acf7:	01 20                	add    %sp,(%bx,%si)
    acf9:	f3 ae                	repz scas %es:(%di),%al
    acfb:	4f                   	dec    %di
    acfc:	c3                   	ret
    acfd:	b4 3c                	mov    $0x3c,%ah
    acff:	a3 a4 a2             	mov    %ax,0xa2a4
    ad02:	92                   	xchg   %ax,%dx
    ad03:	00 3c                	add    %bh,(%si)
    ad05:	73 33                	jae    0xad3a
    ad07:	01 80 c8 ff          	add    %ax,-0x38(%bx,%si)
    ad0b:	c6 06 00 ff 0c       	movb   $0xc,-0x100
    ad10:	3c 03                	cmp    $0x3,%al
    ad12:	73 02                	jae    0xad16
    ad14:	eb 04                	jmp    0xad1a
    ad16:	01 20                	add    %sp,(%bx,%si)
    ad18:	eb 02                	jmp    0xad1c
    ad1a:	c0 8b a2 01 74       	rorb   $0x74,0x1a2(%bp,%di)
    ad1f:	8b 4c 1e             	mov    0x1e(%si),%cx
    ad22:	00 c3                	add    %al,%bl
    ad24:	8e 00                	mov    (%bx,%si),%es
    ad26:	93                   	xchg   %ax,%bx
    ad27:	00 8e c3 33          	add    %cl,0x33c3(%bp)
    ad2b:	ff                   	(bad)
    ad2c:	fc                   	cld
    ad2d:	26 f6 05 74          	testb  $0x74,%es:(%di)
    ad31:	52                   	push   %dx
    ad32:	24 be                	and    $0xbe,%al
    ad34:	9b                   	fwait
    ad35:	b9 d1 f3             	mov    $0xf3d1,%cx
    ad38:	1b 8b 99 ff          	sbb    -0x67(%bp,%di),%cx
    ad3c:	0e                   	push   %cs
    ad3d:	00 eb                	add    %ch,%bl
    ad3f:	5c                   	pop    %sp
    ad40:	d1 00                	rolw   $1,(%bx,%si)
    ad42:	ae                   	scas   %es:(%di),%al
    ad43:	ef                   	out    %ax,(%dx)
    ad44:	eb e7                	jmp    0xad2d
    ad46:	e8 ae ff             	call   0xacf7
    ad49:	80 00 34             	addb   $0x34,(%bx,%si)
    ad4c:	3d 75 f1             	cmp    $0xf175,%ax
    ad4f:	47                   	inc    %di
    ad50:	e8 a4 ff             	call   0xacf7
    ad53:	89 3e 95 0f          	mov    %di,0xf95
    ad57:	00 03                	add    %al,(%bp,%di)
    ad59:	92                   	xchg   %ax,%dx
    ad5a:	00 02                	add    %al,(%bp,%si)
    ad5c:	76 2d                	jbe    0xad8b
    ad5e:	33 ff                	xor    %di,%di
    ad60:	7e 0e                	jle    0xad70
    ad62:	0e                   	push   %cs
    ad63:	d1 00                	rolw   $1,(%bx,%si)
    ad65:	26 38 74 33          	cmp    %dh,%es:0x33(%si)
    ad69:	04 ae                	add    $0xae,%al
    ad6b:	ef                   	out    %ax,(%dx)
    ad6c:	eb f7                	jmp    0xad65
    ad6e:	83 c7 03             	add    $0x3,%di
    ad71:	89 3e 99 58          	mov    %di,0x5899
    ad75:	d2 0a                	rorb   %cl,(%bp,%si)
    ad77:	26 8a 05             	mov    %es:(%di),%al
    ad7a:	47                   	inc    %di
    ad7b:	42                   	inc    %dx
    ad7c:	3c 00                	cmp    $0x0,%al
    ad7e:	74 0b                	je     0xad8b
    ad80:	e8 f3 fd             	call   0xab76
    ad83:	75 f2                	jne    0xad77
    ad85:	89 16 00 2e          	mov    %dx,0x2e00
    ad89:	eb ec                	jmp    0xad77
    ad8b:	c3                   	ret
    ad8c:	c3                   	ret
    ad8d:	7b 78                	jnp    0xae07
    ad8f:	26 9d                	es popf
    ad91:	00 b0 ff b4          	add    %dh,-0x4b01(%bx,%si)
    ad95:	8b 2b                	mov    (%bp,%di),%bp
    ad97:	c0 c3 f8             	rol    $0xf8,%bl
    ad9a:	52                   	push   %dx
    ad9b:	1e                   	push   %ds
    ad9c:	f9                   	stc
    ad9d:	53                   	push   %bx
    ad9e:	b8 57 8b             	mov    $0x8b57,%ax
    ada1:	3b b4 8b c3          	cmp    -0x3c75(%si),%si
    ada5:	f8                   	clc
    ada6:	e8 e8 f8             	call   0xa691
    ada9:	50                   	push   %ax
    adaa:	53                   	push   %bx
    adab:	51                   	push   %cx
    adac:	52                   	push   %dx
    adad:	55                   	push   %bp
    adae:	57                   	push   %di
    adaf:	56                   	push   %si
    adb0:	1e                   	push   %ds
    adb1:	06                   	push   %es
    adb2:	b8 63 0e             	mov    $0xe63,%ax
    adb5:	8e d8                	mov    %ax,%ds
    adb7:	ff 06 42 00          	incw   0x42
    adbb:	75 04                	jne    0xadc1
    adbd:	ff 06 44 00          	incw   0x44
    adc1:	fe 0e 8a 00          	decb   0x8a
    adc5:	75 07                	jne    0xadce
    adc7:	b0 04                	mov    $0x4,%al
    adc9:	a2 8a 00             	mov    %al,0x8a
    adcc:	cd 64                	int    $0x64
    adce:	a0 ec 3e             	mov    0x3eec,%al
    add1:	0a c0                	or     %al,%al
    add3:	74 27                	je     0xadfc
    add5:	a0 e9 3e             	mov    0x3ee9,%al
    add8:	3a 06 8b 00          	cmp    0x8b,%al
    addc:	74 18                	je     0xadf6
    adde:	a2 8b 00             	mov    %al,0x8b
    ade1:	a0 e9 3e             	mov    0x3ee9,%al
    ade4:	0a c0                	or     %al,%al
    ade6:	74 08                	je     0xadf0
    ade8:	b4 04                	mov    $0x4,%ah
    adea:	ff 1e c4 00          	lcall  *0xc4
    adee:	eb 06                	jmp    0xadf6
    adf0:	b4 03                	mov    $0x3,%ah
    adf2:	ff 1e c4 00          	lcall  *0xc4
    adf6:	b4 00                	mov    $0x0,%ah
    adf8:	ff 1e c4 00          	lcall  *0xc4
    adfc:	b0 20                	mov    $0x20,%al
    adfe:	e6 20                	out    %al,$0x20
    ae00:	07                   	pop    %es
    ae01:	1f                   	pop    %ds
    ae02:	5e                   	pop    %si
    ae03:	5f                   	pop    %di
    ae04:	5d                   	pop    %bp
    ae05:	5a                   	pop    %dx
    ae06:	59                   	pop    %cx
    ae07:	5b                   	pop    %bx
    ae08:	58                   	pop    %ax
    ae09:	cf                   	iret
    ae0a:	53                   	push   %bx
    ae0b:	52                   	push   %dx
    ae0c:	a1 86 00             	mov    0x86,%ax
    ae0f:	0b c0                	or     %ax,%ax
    ae11:	75 21                	jne    0xae34
    ae13:	a1 88 00             	mov    0x88,%ax
    ae16:	0b c0                	or     %ax,%ax
    ae18:	75 1a                	jne    0xae34
    ae1a:	b8 08 35             	mov    $0x3508,%ax
    ae1d:	cd 21                	int    $0x21
    ae1f:	89 1e 86 00          	mov    %bx,0x86
    ae23:	8c 90 86 00          	mov    %ss,0x86(%bx,%si)
    ae27:	8c 02                	mov    %es,(%bp,%si)
    ae29:	00 fc                	add    %bh,%ah
    ae2b:	33 e8                	xor    %ax,%bp
    ae2d:	0c 0c                	or     $0xc,%al
    ae2f:	f8                   	clc
    ae30:	50                   	push   %ax
    ae31:	53                   	push   %bx
    ae32:	51                   	push   %cx
    ae33:	52                   	push   %dx
    ae34:	55                   	push   %bp
    ae35:	57                   	push   %di
    ae36:	56                   	push   %si
    ae37:	1e                   	push   %ds
    ae38:	06                   	push   %es
    ae39:	b8 63 50             	mov    $0x5063,%ax
    ae3c:	41                   	inc    %cx
    ae3d:	54                   	push   %sp
    ae3e:	48                   	dec    %ax
    ae3f:	2e 45                	cs inc %bp
    ae41:	58                   	pop    %ax
    ae42:	45                   	inc    %bp
    ae43:	ec                   	in     (%dx),%al
    ae44:	0a 74 a0             	or     -0x60(%si),%dh
    ae47:	3e 06                	ds push %es
    ae49:	00 18                	add    %bl,(%bx,%si)
    ae4b:	8b a0 3e c0          	mov    -0x3fc2(%bx,%si),%sp
    ae4f:	08 04                	or     %al,(%si)
    ae51:	1e                   	push   %ds
    ae52:	00 06 03 1e          	add    %al,0x1e03
    ae56:	00 00                	add    %al,(%bx,%si)
    ae58:	1e                   	push   %ds
    ae59:	00 20                	add    %ah,(%bx,%si)
    ae5b:	20 1f                	and    %bl,(%bx)
    ae5d:	5f                   	pop    %di
    ae5e:	5a                   	pop    %dx
    ae5f:	5b                   	pop    %bx
    ae60:	cf                   	iret
    ae61:	52                   	push   %dx
    ae62:	86 0b                	xchg   %cl,(%bp,%di)
    ae64:	75 a1                	jne    0xae07
    ae66:	00 c0                	add    %al,%al
    ae68:	1a 08                	sbb    (%bx,%si),%cl
    ae6a:	cd 89                	int    $0x89
    ae6c:	86 8c 86 8c          	xchg   %cl,-0x737a(%si)
    ae70:	00 33                	add    %dh,(%bp,%di)
    ae72:	0c f8                	or     $0xf8,%al
    ae74:	53                   	push   %bx
    ae75:	52                   	push   %dx
    ae76:	57                   	push   %di
    ae77:	1e                   	push   %ds
    ae78:	b8 50 54             	mov    $0x5450,%ax
    ae7b:	2e 58                	cs pop %ax
    ae7d:	ec                   	in     (%dx),%al
    ae7e:	74 3e                	je     0xaebe
    ae80:	00 8b 3e 08          	add    %cl,0x83e(%bp,%di)
    ae84:	1e                   	push   %ds
    ae85:	06                   	push   %es
    ae86:	1e                   	push   %ds
    ae87:	00 00                	add    %al,(%bx,%si)
    ae89:	20 5f 5b             	and    %bl,0x5b(%bx)
    ae8c:	52                   	push   %dx
    ae8d:	0b a1 c0 08          	or     0x8c0(%bx,%di),%sp
    ae91:	89 8c 8c 33          	mov    %cx,0x338c(%si)
    ae95:	f8                   	clc
    ae96:	52                   	push   %dx
    ae97:	1e                   	push   %ds
    ae98:	42                   	inc    %dx
    ae99:	00 41 3a             	add    %al,0x3a(%bx,%di)
    ae9c:	00 00                	add    %al,(%bx,%si)
    ae9e:	20 5f 5b             	and    %bl,0x5b(%bx)
    aea1:	52                   	push   %dx
    aea2:	0b a1 c0 08          	or     0x8c0(%bx,%di),%sp
    aea6:	89 8c 8c 33          	mov    %cx,0x338c(%si)
    aeaa:	f8                   	clc
    aeab:	52                   	push   %dx
    aeac:	1e                   	push   %ds
    aead:	50                   	push   %ax
    aeae:	2e ec                	cs in  (%dx),%al
    aeb0:	3e 8b 08             	mov    %ds:(%bx,%si),%cx
    aeb3:	06                   	push   %es
    aeb4:	00 20                	add    %ah,(%bx,%si)
    aeb6:	5b                   	pop    %bx
    aeb7:	0b c0                	or     %ax,%ax
    aeb9:	89 8c f8 1e          	mov    %cx,0x1ef8(%si)
    aebd:	00 3a                	add    %bh,(%bp,%si)
    aebf:	00 5f 52             	add    %bl,0x52(%bx)
    aec2:	a1 08 8c             	mov    0x8c08,%ax
    aec5:	33 52 50             	xor    0x50(%bp,%si),%dx
    aec8:	ec                   	in     (%dx),%al
    aec9:	8b 06 20 0b          	mov    0xb20,%ax
    aecd:	89 f8                	mov    %di,%ax
    aecf:	00 00                	add    %al,(%bx,%si)
    aed1:	52                   	push   %dx
    aed2:	08 33                	or     %dh,(%bp,%di)
    aed4:	50                   	push   %ax
    aed5:	8b 20                	mov    (%bx,%si),%sp
    aed7:	89 00                	mov    %ax,(%bx,%si)
    aed9:	52                   	push   %dx
    aeda:	33 8b 89 50          	xor    0x5089(%bp,%di),%cx
    aede:	4c                   	dec    %sp
    aedf:	49                   	dec    %cx
    aee0:	4e                   	dec    %si
    aee1:	4b                   	dec    %bx
    aee2:	38 20                	cmp    %ah,(%bx,%si)
    aee4:	79 4f                	jns    0xaf35
    aee6:	76 65                	jbe    0xaf4d
    aee8:	72 6c                	jb     0xaf56
    aeea:	61                   	popa
    aeeb:	79 20                	jns    0xaf0d
    aeed:	4c                   	dec    %sp
    aeee:	6f                   	outsw  %ds:(%si),(%dx)
    aeef:	61                   	popa
    aef0:	64 65 72 3a          	fs gs jb 0xaf2e
    aef4:	20 73 74             	and    %dh,0x74(%bp,%di)
    aef7:	61                   	popa
    aef8:	63 6b 20             	arpl   %bp,0x20(%bp,%di)
    aefb:	75 6e                	jne    0xaf6b
    aefd:	64 65 72 66          	fs gs jb 0xaf67
    af01:	6c                   	insb   (%dx),%es:(%di)
    af02:	6f                   	outsw  %ds:(%si),(%dx)
    af03:	77 20                	ja     0xaf25
    af05:	24 50                	and    $0x50,%al
    af07:	49                   	dec    %cx
    af08:	4b                   	dec    %bx
    af09:	20 4f 65             	and    %cl,0x65(%bx)
    af0c:	6c                   	insb   (%dx),%es:(%di)
    af0d:	79 4c                	jns    0xaf5b
    af0f:	61                   	popa
    af10:	65 3a 73 61          	cmp    %gs:0x61(%bp,%di),%dh
    af14:	6b 75 64 72          	imul   $0x72,0x64(%di),%si
    af18:	6c                   	insb   (%dx),%es:(%di)
    af19:	77 24                	ja     0xaf3f
    af1b:	49                   	dec    %cx
    af1c:	20 65 79             	and    %ah,0x79(%di)
    af1f:	61                   	popa
    af20:	3a 61 75             	cmp    0x75(%bx,%di),%ah
    af23:	72 6f                	jb     0xaf94
    af25:	61                   	popa
    af26:	65 3a 66 6f          	cmp    %gs:0x6f(%bp),%ah
    af2a:	20 50 4b             	and    %dl,0x4b(%bx,%si)
    af2d:	4f                   	dec    %di
    af2e:	6c                   	insb   (%dx),%es:(%di)
    af2f:	4c                   	dec    %sp
    af30:	65 73 6b             	gs jae 0xaf9e
    af33:	64 6c                	fs insb (%dx),%es:(%di)
    af35:	24 20                	and    $0x20,%al
    af37:	79 3a                	jns    0xaf73
    af39:	75 6f                	jne    0xafaa
    af3b:	65 66 20 4b 6c       	data32 and %cl,%gs:0x6c(%bp,%di)
    af40:	65 6b 6c 20 3a       	imul   $0x3a,%gs:0x20(%si),%bp
    af45:	6f                   	outsw  %ds:(%si),(%dx)
    af46:	64 6f                	outsw  %fs:(%si),(%dx)
    af48:	73 20                	jae    0xaf6a
    af4a:	32 2e 30 20          	xor    0x2030,%ch
    af4e:	72 65                	jb     0xafb5
    af50:	71 75                	jno    0xafc7
    af52:	69 72 65 64 4b       	imul   $0x4b64,0x65(%bp,%si),%si
    af57:	4f                   	dec    %di
    af58:	0d 0a 6c             	or     $0x6c0a,%ax
    af5b:	65 6b 6c 20 3a       	imul   $0x3a,%gs:0x20(%si),%bp
    af60:	6f                   	outsw  %ds:(%si),(%dx)
    af61:	66 4b                	dec    %ebx
    af63:	65 6c                	gs insb (%dx),%es:(%di)
    af65:	3a 64 73             	cmp    0x73(%si),%ah
    af68:	32 30                	xor    (%bx,%si),%dh
    af6a:	72 71                	jb     0xafdd
    af6c:	69 65 4b 0d 20       	imul   $0x200d,0x4b(%di),%sp
    af71:	2d 4b 4f             	sub    $0x4f4b,%ax
    af74:	2e 0d 0a 45          	cs or  $0x450a,%ax
    af78:	6e                   	outsb  %ds:(%si),(%dx)
    af79:	74 4b                	je     0xafc6
    af7b:	0d 20 66             	or     $0x6620,%ax
    af7e:	69 6c 65 20 6e       	imul   $0x6e20,0x65(%si),%bp
    af83:	61                   	popa
    af84:	6d                   	insw   (%dx),%es:(%di)
    af85:	65 20 70 72          	and    %dh,%gs:0x72(%bx,%si)
    af89:	65 66 69 78 20 28 58 	imul   $0x6f3a5828,%gs:0x20(%bx,%si),%edi
    af90:	3a 6f 
    af92:	6f                   	outsw  %ds:(%si),(%dx)
    af93:	0d 20 70             	or     $0x7020,%ax
    af96:	61                   	popa
    af97:	74 68                	je     0xb001
    af99:	20 6e 61             	and    %ch,0x61(%bp)
    af9c:	6d                   	insw   (%dx),%es:(%di)
    af9d:	65 2f                	gs das
    af9f:	29 6f 6f             	sub    %bp,0x6f(%bx)
    afa2:	0d 20 27             	or     $0x2720,%ax
    afa5:	2e 27                	cs daa
    afa7:	20 74 6f             	and    %dh,0x6f(%si)
    afaa:	20 71 75             	and    %dh,0x75(%bx,%di)
    afad:	69 74 3d 3e 24       	imul   $0x243e,0x3d(%si),%si
    afb2:	46                   	inc    %si
    afb3:	61                   	popa
    afb4:	74 61                	je     0xb017
    afb6:	6c                   	insb   (%dx),%es:(%di)
    afb7:	20 4b 0d             	and    %cl,0xd(%bp,%di)
    afba:	72 6f                	jb     0xb02b
    afbc:	0d 20 2d             	or     $0x2d20,%ax
    afbf:	4b                   	dec    %bx
    afc0:	4f                   	dec    %di
    afc1:	43                   	inc    %bx
    afc2:	61                   	popa
    afc3:	6e                   	outsb  %ds:(%si),(%dx)
    afc4:	27                   	daa
    afc5:	74 20                	je     0xafe7
    afc7:	66 69 6e 64 20 69 65 	imul   $0x6e656920,0x64(%bp),%ebp
    afce:	6e 
    afcf:	6d                   	insw   (%dx),%es:(%di)
    afd0:	20 24                	and    %ah,(%si)
    afd2:	44                   	inc    %sp
    afd3:	69 73 75 72 49       	imul   $0x4972,0x75(%bp,%di),%si
    afd8:	2f                   	das
    afd9:	4f                   	dec    %di
    afda:	20 0d                	and    %cl,(%di)
    afdc:	6f                   	outsw  %ds:(%si),(%dx)
    afdd:	20 4b 43             	and    %cl,0x43(%bp,%di)
    afe0:	6e                   	outsb  %ds:(%si),(%dx)
    afe1:	69 6e 4b 4f 0d       	imul   $0xd4f,0x4b(%bp),%bp
    afe6:	0a 24                	or     (%si),%ah
    afe8:	8b 89 52 8b          	mov    -0x74ae(%bx,%di),%cx
    afec:	50                   	push   %ax
    afed:	49                   	dec    %cx
    afee:	4b                   	dec    %bx
    afef:	20 78 0a             	and    %bh,0xa(%bx,%si)
    aff2:	4b                   	dec    %bx
    aff3:	20 24                	and    %ah,(%si)
    aff5:	1b 50 49             	sbb    0x49(%bx,%si),%dx
    aff8:	4b                   	dec    %bx
    aff9:	20 0e 0b a8          	and    %cl,-0x57f5
    affd:	0b 75 8b             	or     -0x75(%di),%si
    b000:	50                   	push   %ax
    b001:	49                   	dec    %cx
    b002:	4b                   	dec    %bx
    b003:	20 9a 57 3b          	and    %bl,0x3b57(%bp,%si)
    b007:	ba 4b 20             	mov    $0x204b,%dx
    b00a:	0e                   	push   %cs
    b00b:	0b 63 0e             	or     0xe(%bp,%di),%sp
    b00e:	75 8b                	jne    0xaf9b
    b010:	9a 57 3b ba 55       	lcall  $0x55ba,$0x3b57
    b015:	03 3b                	add    (%bp,%di),%di
    b017:	4b                   	dec    %bx
    b018:	0e                   	push   %cs
    b019:	63 75 9a             	arpl   %si,-0x66(%di)
    b01c:	39 0c                	cmp    %cx,(%si)
    b01e:	75 8b                	jne    0xafab
    b020:	ef                   	out    %ax,(%dx)
    b021:	03 3b                	add    (%bp,%di),%di
    b023:	4b                   	dec    %bx
    b024:	2b 01                	sub    (%bx,%di),%ax
    b026:	3b 0e 75 39          	cmp    0x3975,%cx
    b02a:	75 ef                	jne    0xb01b
    b02c:	49                   	dec    %cx
    b02d:	0e                   	push   %cs
    b02e:	75 8b                	jne    0xafbb
    b030:	1a 05                	sbb    (%di),%al
    b032:	75 39                	jne    0xb06d
    b034:	3b 03                	cmp    (%bp,%di),%ax
    b036:	4b                   	dec    %bx
    b037:	63 9a 0c 8b          	arpl   %bx,-0x74f4(%bp,%si)
    b03b:	03 f8                	add    %ax,%di
    b03d:	0d 75 8b             	or     $0x8b75,%ax
    b040:	55                   	push   %bp
    b041:	33 0c                	xor    (%si),%cx
    b043:	f8                   	clc
    b044:	ea 02 00 60 02       	ljmp   $0x260,$0x2
    b049:	00 63 0e             	add    %ah,0xe(%bp,%di)
    b04c:	24 1b                	and    $0x1b,%al
    b04e:	75 8b                	jne    0xafdb
    b050:	3f                   	aas
    b051:	0b 0c                	or     (%si),%cx
    b053:	f8                   	clc
    b054:	41                   	inc    %cx
    b055:	0c ff                	or     $0xff,%al
    b057:	ff 43 45             	incw   0x45(%bp,%di)
    b05a:	4e                   	dec    %si
    b05b:	54                   	push   %sp
    b05c:	55                   	push   %bp
    b05d:	52                   	push   %dx
    b05e:	49                   	dec    %cx
    b05f:	4f                   	dec    %di
    b060:	2e 58                	cs pop %ax
    b062:	ec                   	in     (%dx),%al
    b063:	74 3e                	je     0xb0a3
    b065:	4f                   	dec    %di
    b066:	56                   	push   %si
    b067:	4c                   	dec    %sp
    b068:	30 2e 4f 56          	xor    %ch,0x564f
    b06c:	4c                   	dec    %sp
    b06d:	8b 9a ec 1f          	mov    0x1fec(%bp,%si),%bx
    b071:	06                   	push   %es
    b072:	5f                   	pop    %di
    b073:	02 00                	add    (%bx,%si),%al
    b075:	ea 24 df 4b 20       	ljmp   $0x204b,$0xdf24
    b07a:	9a ec 1f 06 5f       	lcall  $0x5f06,$0x1fec
    b07f:	01 3b                	add    %di,(%bp,%di)
    b081:	ea 8a b1 4b 9a       	ljmp   $0x9a4b,$0xb18a
    b086:	1f                   	pop    %ds
    b087:	5f                   	pop    %di
    b088:	3b 8a 4b 1f          	cmp    0x1f4b(%bp,%si),%cx
    b08c:	3b 4b e0             	cmp    -0x20(%bp,%di),%cx
    b08f:	b0 4b                	mov    $0x4b,%al
    b091:	1f                   	pop    %ds
    b092:	3b 4b 3b             	cmp    0x3b(%bp,%di),%cx
    b095:	e0 4b                	loopne 0xb0e2
    b097:	04 3b                	add    $0x3b,%al
    b099:	4b                   	dec    %bx
    b09a:	58                   	pop    %ax
    b09b:	df 20                	fbld   (%bx,%si)
    b09d:	ec                   	in     (%dx),%al
    b09e:	06                   	push   %es
    b09f:	01 ea                	add    %bp,%dx
    b0a1:	b1 9a                	mov    $0x9a,%cl
    b0a3:	05 75 4b             	add    $0x4b75,%ax
    b0a6:	b0 1f                	mov    $0x1f,%al
    b0a8:	4b                   	dec    %bx
    b0a9:	e0 04                	loopne 0xb0af
    b0ab:	4b                   	dec    %bx
    b0ac:	df ec                	fucomip %st(4),%st
    b0ae:	01 b1 05 4b          	add    %si,0x4b05(%bx,%di)
    b0b2:	8a d0                	mov    %al,%dl
    b0b4:	4b                   	dec    %bx
    b0b5:	04 df                	add    $0xdf,%al
    b0b7:	01 05                	add    %ax,(%di)
    b0b9:	8a 4b 03             	mov    0x3(%bp,%di),%cl
    b0bc:	4b                   	dec    %bx
    b0bd:	4b                   	dec    %bx
    b0be:	1f                   	pop    %ds
    b0bf:	e0 4b                	loopne 0xb10c
    b0c1:	ec                   	in     (%dx),%al
    b0c2:	b1 4b                	mov    $0x4b,%cl
    b0c4:	d0 04                	rolb   $1,(%si)
    b0c6:	01 05                	add    %ax,(%di)
    b0c8:	75 4b                	jne    0xb115
    b0ca:	2a b1 9a 5f          	sub    0x5f9a(%bx,%di),%dh
    b0ce:	8a 1f                	mov    (%bx),%bl
    b0d0:	4b                   	dec    %bx
    b0d1:	b0 1f                	mov    $0x1f,%al
    b0d3:	4b                   	dec    %bx
    b0d4:	e0 04                	loopne 0xb0da
    b0d6:	a1 b9 33             	mov    0x33b9,%ax
    b0d9:	50                   	push   %ax
    b0da:	8b 20                	mov    (%bx,%si),%sp
    b0dc:	89 00                	mov    %ax,(%bx,%si)
    b0de:	52                   	push   %dx
    b0df:	33 8b 89 52          	xor    0x5289(%bp,%di),%cx
    b0e3:	8b 50 49             	mov    0x49(%bx,%si),%dx
    b0e6:	4b                   	dec    %bx
    b0e7:	20 4f 65             	and    %cl,0x65(%bx)
    b0ea:	6c                   	insb   (%dx),%es:(%di)
    b0eb:	79 4c                	jns    0xb139
    b0ed:	61                   	popa
    b0ee:	65                   	gs
    b0ef:	3a                   	.byte 0x3a
