/* The button dispatcher of AC_MAIN.EXE, 0x3FB94..0x426CD (module N2 of docs/MAP_PLAN.md): a SKELETON. Every modal loop
   reaches it through the generic click wait 0x78BA4 (also the map click 0x3D434, the text input 0x60694, 0x166C1,
   0x29B5F and 0x14294): it turns a button id into a code for the caller's loop, or runs the screen behind the button.
   Literal translation: one switch case per id with the address its jump-table entry points to, the instruction
   addresses in the comments. D = read in the code, G = inference. Prototype and the layout notes: game.h (section
   "AST chart, button kit, click wait, dispatcher, score helpers").

   The original (D):
     0x3FBA9  id in eax, kept at [ebp-0x1C]
     0x3FBAC  code = -100 (int32 [ebp-0x3C]); every case sets it (or leaves it) and ends in the epilogue at 0x426C1,
              which returns it (most cases carry a copy of that epilogue)
     0x3FBB3  index = id - 3, compared as a 16-bit unsigned value: cmp ax, 0x104; ja 0x426C1 (0x3FBB9)
     0x3FBC3  and eax, 0xFFFF; jmp cs:[4*eax + 0x3F780] (fixup at 0x3FBCC -> 0x3F780)
   So only the low 16 bits of id - 3 select the entry; an id with higher bits set would still hit an entry (callers
   pass the record's int16 +0x0E sign-extended, so this never happens, G). The switch below keeps that rule.

   The jump table 0x3F780..0x3FB93: 261 dwords for ids 3..0x107 in the code object (obj1.bin offset 0x2F780), right
   before the function. Each of the 261 entries has an off32 fixup in disasm/le/fixups.txt, and each fixup's target
   equals the dword stored in obj1.bin (checked by script: 261 of 261). 231 distinct targets:
     - 0x426C1, the default (-100): ids 0x11, 0x1A, 0x54, 0x55, 0x59..0x68, 0xAC..0xAF, 0xEB, 0x102, 0x104..0x106
     - 154 targets (156 ids) that only store a constant in code: 153 by mov [ebp-0x3C], imm (0x3FD35 for 0x2A among
       them; 0x426BA, id 0x100, falls into 0x426C1) and 0x4202C, id 0x42, which stores 0 by xor ecx, ecx / mov
       [ebp-0x3C], ecx. Two targets serve two ids each: 0x418BB (0x34, 0x36: -5) and 0x418CF (0x35, 0x37: -6)
     - 24 targets for 0x8E..0xA5 that store id - 0x8E (0..23) in int32 [0xF6758] and return -0x3E
     - 0x3FD91 (0x57): QUIT, through the ported f44CAC
     - 51 targets with real work (other modules).
   Note for game.h's list of constant entries: it leaves out 0x12..0x19, which return -0x36..-0x3D (0x40207..0x40293,
   D), and 0x42 is the xor form above (same value, 0).

   Port (MAP_PLAN N2): the default, the constant entries, 0x8E..0xA5 and QUIT are translated. Every case whose body
   calls code that is not ported yet calls not_ported(<the case's target address>) at its start, so a click on it
   ends the run with that address instead of returning -100. The comment in each such case says what the body does
   and which module of MAP_PLAN.md will bring it:
     N2, needs S1's 0x3CCBC : 3 (f3CCBC, f136B0(2), f3CC14; everything else of it is ported or in N2)
     INFO  : 4, 6, 7, 0x103
     MENU1 : 5, 0x52, 0x53
     MENU2 : 8, 0x56, 0xE2
     PANEL : 9..0x10, 0x25
     MAP1  : 0x1B..0x24
     MOVE  : 0x26, 0x27, 0x28, 0x2B..0x32, 0xC4..0xC8
     STACK : 0x4F, 0x51, 0x101
     S1    : 0x50
     PBEM  : 0x107 (SHOW PBEM, 0x545AC; PBEM is dropped by Erik's ruling, so it stays not_ported)
   Callees these cases need that are not ported: 0x3CCBC, 0x3CC68, 0x3CBBC, 0x3CD10 (screen copies), 0x13E50, 0x42754,
   0x19C10, 0x76A9C, 0x44A34, 0x763FC, 0x545AC, 0x44D5C, 0x436FC, 0x43E88, 0x4290C, 0x5E7D0, 0x5FF28, 0x121A0,
   0x7A8D0, 0x7A940, 0x7AC60, 0x7ABD0, 0x3F558, 0x459E4, 0x3D9FC, 0x3E374, 0x3D52C, 0x3A990, 0x572CC, 0x57CE8,
   0x57E28, 0x580CC, 0x576F8, 0x576A8, 0x3E89C, 0x3E8FC, 0x426D0, 0x3F698, 0x3F70C, 0x3F624, 0x3F670, 0x3DA60, and the
   library's fg_makegif (0x8CE50, not in fg.c), longjmp (0x87879).

   Heap pointers in data fields: the translated cases touch none. Cases 0x50 and 0x51 (not ported) free the fields
   0xF0984[1..11], [0xEED74], [0xEED78], [0xEED7C] and [0xEED80] (load_heap_free when ported, game.h).

   Runtime: __CHK dropped. 1995 bugs: none in the translated cases (no g_original branches). */

#include <stdint.h>
#include "game.h"

/* 0x3FB94: the button dispatcher. id = the button's int16 +0x0E, sign-extended by the callers. Returns the code for
   the caller's loop, -100 = nothing to do. */
int f3FB94(int id)
{
    int32_t code;
    uint16_t index;

    code = -100;                                /* 0x3FBAC mov [ebp-0x3C], 0xFFFFFF9C */
    index = (uint16_t)((uint32_t)id - 3u);      /* 0x3FBB3 mov eax, [ebp-0x1C]; sub eax, 3 */
    if (index > 0x104)                          /* 0x3FBB9 cmp ax, 0x104; ja 0x426C1 (unsigned) */
        return code;

    switch (index + 3) {                        /* 0x3FBC3 and eax, 0xFFFF; jmp cs:[4*eax + 0x3F780]: entry = id - 3 */
    case 0x03:  /* -> 0x3FBD0 */
        f3CCBC();                               /* 0x3FBD0 screen -> back buffer [0xF52C8] */
        f136B0(2);                              /* 0x3FBDA the AST chart from the panel (steps shown, not applied) */
        f3CC14();                               /* 0x3FBDF */
        return code;                            /* 0x3FBE4 mov eax, [ebp-0x3C] (still -100) */
    case 0x04:  /* -> 0x3FBF1 */
        /* f3CCBC, 0x13E50 (census screen), f3CC14; -100 (INFO) */
        not_ported(0x3FBF1u);
    case 0x05:  /* -> 0x3FC0D */
        /* 0x42754 (options menu; ebx = -100 kept across the call, 0x3FC0D/0x3FC17); -100 (MENU1) */
        not_ported(0x3FC0Du);
    case 0x06:  /* -> 0x3FC27 */
        /* 0x19C10 (Civilization Tools); -100 (INFO) */
        not_ported(0x3FC27u);
    case 0x07:  /* -> 0x3FC39 */
        /* 0x76A9C (Goods Held); -100 (INFO) */
        not_ported(0x3FC39u);
    case 0x08:  /* -> 0x3FC4B */
        /* 0x44A34 (G: the nation's full-map view); -100 (MENU2) */
        not_ported(0x3FC4Bu);
    case 0x103: /* -> 0x3FC5D */
        /* Trading Wrapup: f78884, fg_makegif(0, 0x27F, 0, 0x1DE, "junk"), f788AC; save byte [0xEE168]; 0x763FC;
           fg_vbopen([0xF52CC]), fg_move(0, 0), fg_showgif("junk", 3) (vbclose not called here); f3CC68, f3CCBC;
           [0xF52D0] = 0; restore [0xEE168]; -100 (INFO) */
        not_ported(0x3FC5Du);
    case 0x107: /* -> 0x3FCE0 */
        /* SHOW PBEM: f3CC68, 0x545AC, byte [0xF52D0] = 0; -1 (PBEM is dropped by Erik's ruling: stays not_ported)
           (PBEM) */
        not_ported(0x3FCE0u);
    case 0x56:  /* -> 0x3FD07 */
        /* SCORING: f3CC68, 0x44D5C (scoring history, writes 0xEE9F0); -1 (MENU2) */
        not_ported(0x3FD07u);
    case 0x52:  /* -> 0x3FD26 */
        /* TOGGLES: f3CC68, f3CCBC, 0x436FC, then falls into 0x3FD35: -1 (MENU1) */
        not_ported(0x3FD26u);
    case 0x2A:  /* -> 0x3FD35 */
        code = -1;                      /* 0x3FD35 mov [ebp-0x3C], 0xFFFFFFFF */
        break;
    case 0xE2:  /* -> 0x3FD49 */
        /* AUDIO/VISUALS: f3CC68, f3CCBC, 0x43E88; -1 (MENU2) */
        not_ported(0x3FD49u);
    case 0x53:  /* -> 0x3FD6D */
        /* HELP: f3CC68, f3CCBC, 0x4290C; -1 (MENU1) */
        not_ported(0x3FD6Du);
    case 0x57:  /* -> 0x3FD91 */
        setD8(0xF5500u, 0);                 /* 0x3FD91 xor dh, dh; 0x3FD93 mov [0xF5500], dh: no quit message */
        f44CAC();                           /* 0x3FD99 quit to DOS; does not return */
    case 0x4F:  /* -> 0x3FDAB */
        /* SAVE GAME: f3CC68, f3CCBC, 0x5E7D0; -1 (STACK) */
        not_ported(0x3FDABu);
    case 0x51:  /* -> 0x3FDCF */
        /* START NEW GAME: f3CC68, 0x121A0 (free sprites), 0x7ABD0 (free the state stack), 0x3F558 (free map
           blocks), free + zero 0xF0984[1..11] and [0xEED74], [0xEED7C], [0xEED78], [0xEED80] (each if non-0),
           f3C7A0([0xF8E78]), longjmp(0xF58D4) (0x87879) (STACK) */
        not_ported(0x3FDCFu);
    case 0x50:  /* -> 0x3FEA0 */
        /* RESTORE GAME: f3CC68, f3CCBC, 0x5FF28; -15 (cancel) -> -1 via 0x3FD35; else byte [0xE1624] &= ~4,
           0x7ABD0, the same frees as 0x51, f3C7A0([0xF8E78]), longjmp(0xF58A0) (S1) */
        not_ported(0x3FEA0u);
    case 0x09:  /* -> 0x3FF87 */
        /* player button 0: n = int16 [0xF06B0]; int16 [0x107690] == 0: 0x459E4(n), 0x3D9FC(n), f3CBBC; else
           0x3E374(n); -100 (PANEL) */
        not_ported(0x3FF87u);
    case 0x0A:  /* -> 0x3FFD7 */
        /* player button 1: n = int16 [0xF06B2]; int16 [0x107690] == 0: 0x459E4(n), 0x3D9FC(n), f3CBBC; else
           0x3E374(n); -100 (PANEL) */
        not_ported(0x3FFD7u);
    case 0x0B:  /* -> 0x40027 */
        /* player button 2: n = int16 [0xF06B4]; int16 [0x107690] == 0: 0x459E4(n), 0x3D9FC(n), f3CBBC; else
           0x3E374(n); -100 (PANEL) */
        not_ported(0x40027u);
    case 0x0C:  /* -> 0x40077 */
        /* player button 3: n = int16 [0xF06B6]; int16 [0x107690] == 0: 0x459E4(n), 0x3D9FC(n), f3CBBC; else
           0x3E374(n); -100 (PANEL) */
        not_ported(0x40077u);
    case 0x0D:  /* -> 0x400C7 */
        /* player button 4: n = int16 [0xF06B8]; int16 [0x107690] == 0: 0x459E4(n), 0x3D9FC(n), f3CBBC; else
           0x3E374(n); -100 (PANEL) */
        not_ported(0x400C7u);
    case 0x0E:  /* -> 0x40117 */
        /* player button 5: n = int16 [0xF06BA]; int16 [0x107690] == 0: 0x459E4(n), 0x3D9FC(n), f3CBBC; else
           0x3E374(n); -100 (PANEL) */
        not_ported(0x40117u);
    case 0x0F:  /* -> 0x40167 */
        /* player button 6: n = int16 [0xF06BC]; int16 [0x107690] == 0: 0x459E4(n), 0x3D9FC(n), f3CBBC; else
           0x3E374(n); -100 (PANEL) */
        not_ported(0x40167u);
    case 0x10:  /* -> 0x401B7 */
        /* player button 7: n = int16 [0xF06BE]; int16 [0x107690] == 0: 0x459E4(n), 0x3D9FC(n), f3CBBC; else
           0x3E374(n); -100 (PANEL) */
        not_ported(0x401B7u);
    case 0x12:  /* -> 0x40207 */
        code = -0x36;                   /* 0x40207 mov [ebp-0x3C], 0xFFFFFFCA */
        break;
    case 0x13:  /* -> 0x4021B */
        code = -0x37;                   /* 0x4021B mov [ebp-0x3C], 0xFFFFFFC9 */
        break;
    case 0x14:  /* -> 0x4022F */
        code = -0x38;                   /* 0x4022F mov [ebp-0x3C], 0xFFFFFFC8 */
        break;
    case 0x15:  /* -> 0x40243 */
        code = -0x39;                   /* 0x40243 mov [ebp-0x3C], 0xFFFFFFC7 */
        break;
    case 0x16:  /* -> 0x40257 */
        code = -0x3A;                   /* 0x40257 mov [ebp-0x3C], 0xFFFFFFC6 */
        break;
    case 0x17:  /* -> 0x4026B */
        code = -0x3B;                   /* 0x4026B mov [ebp-0x3C], 0xFFFFFFC5 */
        break;
    case 0x18:  /* -> 0x4027F */
        code = -0x3C;                   /* 0x4027F mov [ebp-0x3C], 0xFFFFFFC4 */
        break;
    case 0x19:  /* -> 0x40293 */
        code = -0x3D;                   /* 0x40293 mov [ebp-0x3C], 0xFFFFFFC3 */
        break;
    case 0x1B:  /* -> 0x402A7 */
        /* scroll: f3D52C(-1, -1); -100 (MAP1) */
        not_ported(0x402A7u);
    case 0x1C:  /* -> 0x402C0 */
        /* scroll: f3D52C(0, -1); -100 (MAP1) */
        not_ported(0x402C0u);
    case 0x1D:  /* -> 0x402D9 */
        /* scroll: f3D52C(1, -1); -100 (MAP1) */
        not_ported(0x402D9u);
    case 0x1E:  /* -> 0x402F5 */
        /* scroll: f3D52C(-1, 0); -100 (MAP1) */
        not_ported(0x402F5u);
    case 0x1F:  /* -> 0x4030E */
        /* scroll: f3D52C(1, 0); -100 (MAP1) */
        not_ported(0x4030Eu);
    case 0x20:  /* -> 0x40327 */
        /* scroll: f3D52C(-1, 1); -100 (MAP1) */
        not_ported(0x40327u);
    case 0x21:  /* -> 0x40343 */
        /* scroll: f3D52C(0, 1); -100 (MAP1) */
        not_ported(0x40343u);
    case 0x22:  /* -> 0x4035C */
        /* scroll: f3D52C(1, 1); -100 (MAP1) */
        not_ported(0x4035Cu);
    case 0x23:  /* -> 0x40375 */
        /* mini-map click: int32 [0xF4884] = ([0xE1628] - 0x1E1) / 7 - 3, [0xF4888] = 2 * ([0xE162C] - 0x170) / 7 -
           4 (signed idiv), f3D52C(0, 0); -100 (MAP1) */
        not_ported(0x40375u);
    case 0x24:  /* -> 0x403C8 */
        /* DETAIL: byte [0xC1A34] = (int8 [0xC1A34] == 3) ? 0 : 3; int16 [0x107690] == 0: f3D52C(0, 0), else
           0x3E374([0xF40F8]); -100 (MAP1) */
        not_ported(0x403C8u);
    case 0x101: /* -> 0x4041D */
        /* REPLAY: [0xEDF58] == 0 -> -100; int16 [0x107692] != 0 -> -1 via 0x3FD35; else "Map shows Round %d post
           movement state.": message area resized, "junk" GIF of the strip, f3C828(0xF596C), labels via sprintf
           into 0x109E7C + f3C284, enables, World State Stack (0x7A8D0, 0x7A940 push, copies 0xE5D20 -> 0xE31B8,
           0xE2DF8 -> 0xE2C18, 0xF1FB0 -> 0xF14CC), f3D52C, f3CBBC, f78BA4(0xF596C, 0) until -1, 0x7AC60 pop
           (0x7ABD0 if it allocated), restore; -100 (STACK) */
        not_ported(0x4041Du);
    case 0x25:  /* -> 0x406E0 */
        /* ZOOM: int16 [0x107690] != 0 -> -1 via 0x3FD35; else f3CD10, [0xF52D0] = 0, f3C828(0xF596C), label "UN-
           ZOOM", enable 0x25, 0x24 and 9..9+[0xF40EC]-1, [0x107690] = 1, 0x3E374([0xF40F8]), f78BA4(0xF596C, 0)
           until -1, [0x107690] = 0, label "ZOOM", f3C7A0, f3CC68, f3D52C(0, 0); -100 (PANEL) */
        not_ported(0x406E0u);
    case 0x26:  /* -> 0x407DC */
        /* LAND: f3B9A4(0xF596C, 0x26) != 0 -> -100; else 0x26 pressed (f3B7CC), byte [0x108804] = 1, [0xF06CC] =
           0, 0x572CC([0x109E10]) (count buttons), 0x27 up (f3B870), f3CBBC; -3 (MOVE) */
        not_ported(0x407DCu);
    case 0xC4:  /* -> 0x40858 */
        /* count 1: f3B9A4(0xF596C, id) != 0 -> -100; else id pressed, the other four of 0xC4..0xC8 up (back
           buffer), f3CBBC, int16 [0xF09E0] = 1; -100 (MOVE) */
        not_ported(0x40858u);
    case 0xC5:  /* -> 0x408E9 */
        /* count 2: f3B9A4(0xF596C, id) != 0 -> -100; else id pressed, the other four of 0xC4..0xC8 up (back
           buffer), f3CBBC, int16 [0xF09E0] = 2; -100 (MOVE) */
        not_ported(0x408E9u);
    case 0xC6:  /* -> 0x4097C */
        /* count 3: f3B9A4(0xF596C, id) != 0 -> -100; else id pressed, the other four of 0xC4..0xC8 up (back
           buffer), f3CBBC, int16 [0xF09E0] = 3; -100 (MOVE) */
        not_ported(0x4097Cu);
    case 0xC7:  /* -> 0x40A10 */
        /* count 4: f3B9A4(0xF596C, id) != 0 -> -100; else id pressed, the other four of 0xC4..0xC8 up (back
           buffer), f3CBBC, int16 [0xF09E0] = 4; -100 (MOVE) */
        not_ported(0x40A10u);
    case 0xC8:  /* -> 0x40AA4 */
        /* count 5: f3B9A4(0xF596C, id) != 0 -> -100; else id pressed, the other four of 0xC4..0xC8 up (back
           buffer), f3CBBC, int16 [0xF09E0] = 5; -100 (MOVE) */
        not_ported(0x40AA4u);
    case 0x27:  /* -> 0x40B38 */
        /* SEA: f3B9A4(0xF596C, 0x27) != 0 -> -100; else 0x27 pressed, byte [0x108804] = 2, [0xF06CC] = 0, 0x26 up,
           0xC4..0xC8 disabled (f3BFBC 0) and drawn up, f3CBBC; -2 (MOVE) */
        not_ported(0x40B38u);
    case 0x28:  /* -> 0x40C4A */
        /* MOVE: [0x108810] == 1: tokens from [0x10880C] to [0x108808] (0x57CE8, area +0x24 add, 0x3F698, 0x57E28,
           f3C3D0(0x28), 0x572CC, f3D52C(0, 0)); else the selected ship (0x580CC, 0x576F8, 0x3F70C, f3C3D0(0x28),
           0x3F670, 0x3F624, 0x426D0, 0x3DA60); then f3CBBC, PBEM counter int16 [0xF09B8] - 1; -4 (MOVE) */
        not_ported(0x40C4Au);
    case 0x29:  /* -> 0x40E1D */
        code = -0x79;                   /* 0x40E1D mov [ebp-0x3C], 0xFFFFFF87 */
        break;
    case 0x4E:  /* -> 0x40E31 */
        code = -0xF;                    /* 0x40E31 mov [ebp-0x3C], 0xFFFFFFF1 */
        break;
    case 0x2F:  /* -> 0x40E45 */
        /* LOAD one token of [0xF40F8] from territory [0x109E10] onto its selected ship: 0x57CE8, 0x3E89C, nation
           +0x4D+k + 1, 0x426D0, PBEM log at [0xF09B4], f3C3D0, f3CBBC; -100 (MOVE) */
        not_ported(0x40E45u);
    case 0x2B:  /* -> 0x40F80 */
        /* as 0x2F (load one token) with 0x3E8FC for the territory; -100 (MOVE) */
        not_ported(0x40F80u);
    case 0x30:  /* -> 0x410AE */
        /* UNLOAD one token from the selected ship: 0x57E28, 0x3E89C, nation +0x4D+k - 1, 0x426D0, PBEM log,
           f3C3D0, f3CBBC; -100 (MOVE) */
        not_ported(0x410AEu);
    case 0x2C:  /* -> 0x41223 */
        /* as 0x30 (unload one) with 0x3E8FC; -100 (MOVE) */
        not_ported(0x41223u);
    case 0x31:  /* -> 0x41380 */
        /* UNLOAD ALL tokens from the selected ship (loop of 0x57E28), 0x426D0, PBEM log, f3C3D0, f3CBBC; -100
           (MOVE) */
        not_ported(0x41380u);
    case 0x2D:  /* -> 0x4152E */
        /* as 0x31 (unload all) with 0x3E8FC; -100 (MOVE) */
        not_ported(0x4152Eu);
    case 0x32:  /* -> 0x416C6 */
        /* NEXT SHIP: area byte +0x2D+n = (k + 1) % 4 (0x3E89C), PBEM log, 0x576F8, 0x426D0; -100 (MOVE) */
        not_ported(0x416C6u);
    case 0x2E:  /* -> 0x417A5 */
        /* NEXT SHIP (no alias): area +0x2D+n = (k + 1) % 4, PBEM log, [0xF06CC] = 0, 0x576F8, 0x426D0, 0x576A8,
           0x3F70C; -2 (MOVE) */
        not_ported(0x417A5u);
    case 0x33:  /* -> 0x418A7 */
        code = -8;                      /* 0x418A7 mov [ebp-0x3C], 0xFFFFFFF8 */
        break;
    case 0x34:  /* -> 0x418BB */
    case 0x36:
        code = -5;                      /* 0x418BB mov [ebp-0x3C], 0xFFFFFFFB */
        break;
    case 0x35:  /* -> 0x418CF */
    case 0x37:
        code = -6;                      /* 0x418CF mov [ebp-0x3C], 0xFFFFFFFA */
        break;
    case 0x38:  /* -> 0x418E3 */
        code = -7;                      /* 0x418E3 mov [ebp-0x3C], 0xFFFFFFF9 */
        break;
    case 0x39:  /* -> 0x418F7 */
        code = -9;                      /* 0x418F7 mov [ebp-0x3C], 0xFFFFFFF7 */
        break;
    case 0x3A:  /* -> 0x4190B */
        code = -0xA;                    /* 0x4190B mov [ebp-0x3C], 0xFFFFFFF6 */
        break;
    case 0x3B:  /* -> 0x4191F */
        code = -0xB;                    /* 0x4191F mov [ebp-0x3C], 0xFFFFFFF5 */
        break;
    case 0x3C:  /* -> 0x41933 */
        code = -0xC;                    /* 0x41933 mov [ebp-0x3C], 0xFFFFFFF4 */
        break;
    case 0x3D:  /* -> 0x41947 */
        code = -0xD;                    /* 0x41947 mov [ebp-0x3C], 0xFFFFFFF3 */
        break;
    case 0x3E:  /* -> 0x4195B */
        code = -0xE;                    /* 0x4195B mov [ebp-0x3C], 0xFFFFFFF2 */
        break;
    case 0x3F:  /* -> 0x4196F */
        code = -0x65;                   /* 0x4196F mov [ebp-0x3C], 0xFFFFFF9B */
        break;
    case 0x40:  /* -> 0x41983 */
        code = -0x66;                   /* 0x41983 mov [ebp-0x3C], 0xFFFFFF9A */
        break;
    case 0x41:  /* -> 0x41997 */
        code = -0x63;                   /* 0x41997 mov [ebp-0x3C], 0xFFFFFF9D */
        break;
    case 0x58:  /* -> 0x419AB */
        code = -0x10;                   /* 0x419AB mov [ebp-0x3C], 0xFFFFFFF0 */
        break;
    case 0xDA:  /* -> 0x419BF */
        code = -0x75;                   /* 0x419BF mov [ebp-0x3C], 0xFFFFFF8B */
        break;
    case 0x69:  /* -> 0x419D3 */
        code = -0x7B;                   /* 0x419D3 mov [ebp-0x3C], 0xFFFFFF85 */
        break;
    case 0x6A:  /* -> 0x419E7 */
        code = -0x7C;                   /* 0x419E7 mov [ebp-0x3C], 0xFFFFFF84 */
        break;
    case 0x6B:  /* -> 0x419FB */
        code = -0x7D;                   /* 0x419FB mov [ebp-0x3C], 0xFFFFFF83 */
        break;
    case 0x6C:  /* -> 0x41A0F */
        code = -0x7E;                   /* 0x41A0F mov [ebp-0x3C], 0xFFFFFF82 */
        break;
    case 0x6D:  /* -> 0x41A23 */
        code = -0x7F;                   /* 0x41A23 mov [ebp-0x3C], 0xFFFFFF81 */
        break;
    case 0x6E:  /* -> 0x41A37 */
        code = -0x80;                   /* 0x41A37 mov [ebp-0x3C], 0xFFFFFF80 */
        break;
    case 0x6F:  /* -> 0x41A4B */
        code = -0x81;                   /* 0x41A4B mov [ebp-0x3C], 0xFFFFFF7F */
        break;
    case 0x70:  /* -> 0x41A5F */
        code = -0x11;                   /* 0x41A5F mov [ebp-0x3C], 0xFFFFFFEF */
        break;
    case 0x71:  /* -> 0x41A73 */
        code = -0x12;                   /* 0x41A73 mov [ebp-0x3C], 0xFFFFFFEE */
        break;
    case 0x72:  /* -> 0x41A87 */
        code = -0x13;                   /* 0x41A87 mov [ebp-0x3C], 0xFFFFFFED */
        break;
    case 0x73:  /* -> 0x41A9B */
        code = -0x14;                   /* 0x41A9B mov [ebp-0x3C], 0xFFFFFFEC */
        break;
    case 0x74:  /* -> 0x41AAF */
        code = -0x15;                   /* 0x41AAF mov [ebp-0x3C], 0xFFFFFFEB */
        break;
    case 0x75:  /* -> 0x41AC3 */
        code = -0x1D;                   /* 0x41AC3 mov [ebp-0x3C], 0xFFFFFFE3 */
        break;
    case 0x76:  /* -> 0x41AD7 */
        code = -0x1C;                   /* 0x41AD7 mov [ebp-0x3C], 0xFFFFFFE4 */
        break;
    case 0x78:  /* -> 0x41AEB */
        code = -0x1E;                   /* 0x41AEB mov [ebp-0x3C], 0xFFFFFFE2 */
        break;
    case 0x79:  /* -> 0x41AFF */
        code = -0x20;                   /* 0x41AFF mov [ebp-0x3C], 0xFFFFFFE0 */
        break;
    case 0x7A:  /* -> 0x41B13 */
        code = -0x22;                   /* 0x41B13 mov [ebp-0x3C], 0xFFFFFFDE */
        break;
    case 0x7B:  /* -> 0x41B27 */
        code = -0x1F;                   /* 0x41B27 mov [ebp-0x3C], 0xFFFFFFE1 */
        break;
    case 0x7C:  /* -> 0x41B3B */
        code = -0x21;                   /* 0x41B3B mov [ebp-0x3C], 0xFFFFFFDF */
        break;
    case 0x7D:  /* -> 0x41B4F */
        code = -0x23;                   /* 0x41B4F mov [ebp-0x3C], 0xFFFFFFDD */
        break;
    case 0x77:  /* -> 0x41B63 */
        code = -0x24;                   /* 0x41B63 mov [ebp-0x3C], 0xFFFFFFDC */
        break;
    case 0x7E:  /* -> 0x41B77 */
        code = -0x25;                   /* 0x41B77 mov [ebp-0x3C], 0xFFFFFFDB */
        break;
    case 0x7F:  /* -> 0x41B8B */
        code = -0x7A;                   /* 0x41B8B mov [ebp-0x3C], 0xFFFFFF86 */
        break;
    case 0x85:  /* -> 0x41B9F */
        code = -0x2B;                   /* 0x41B9F mov [ebp-0x3C], 0xFFFFFFD5 */
        break;
    case 0x86:  /* -> 0x41BB3 */
        code = -0x2C;                   /* 0x41BB3 mov [ebp-0x3C], 0xFFFFFFD4 */
        break;
    case 0x80:  /* -> 0x41BC7 */
        code = -0x26;                   /* 0x41BC7 mov [ebp-0x3C], 0xFFFFFFDA */
        break;
    case 0x81:  /* -> 0x41BDB */
        code = -0x27;                   /* 0x41BDB mov [ebp-0x3C], 0xFFFFFFD9 */
        break;
    case 0x82:  /* -> 0x41BEF */
        code = -0x28;                   /* 0x41BEF mov [ebp-0x3C], 0xFFFFFFD8 */
        break;
    case 0x83:  /* -> 0x41C03 */
        code = -0x29;                   /* 0x41C03 mov [ebp-0x3C], 0xFFFFFFD7 */
        break;
    case 0x84:  /* -> 0x41C17 */
        code = -0x2A;                   /* 0x41C17 mov [ebp-0x3C], 0xFFFFFFD6 */
        break;
    case 0x87:  /* -> 0x41C2B */
        code = -0x2F;                   /* 0x41C2B mov [ebp-0x3C], 0xFFFFFFD1 */
        break;
    case 0x88:  /* -> 0x41C3F */
        code = -0x30;                   /* 0x41C3F mov [ebp-0x3C], 0xFFFFFFD0 */
        break;
    case 0x89:  /* -> 0x41C53 */
        code = -0x31;                   /* 0x41C53 mov [ebp-0x3C], 0xFFFFFFCF */
        break;
    case 0x8A:  /* -> 0x41C67 */
        code = -0x32;                   /* 0x41C67 mov [ebp-0x3C], 0xFFFFFFCE */
        break;
    case 0x8B:  /* -> 0x41C7B */
        code = -0x33;                   /* 0x41C7B mov [ebp-0x3C], 0xFFFFFFCD */
        break;
    case 0x8C:  /* -> 0x41C8F */
        code = -0x34;                   /* 0x41C8F mov [ebp-0x3C], 0xFFFFFFCC */
        break;
    case 0x8D:  /* -> 0x41CA3 */
        code = -0x35;                   /* 0x41CA3 mov [ebp-0x3C], 0xFFFFFFCB */
        break;
    case 0x8E:  /* -> 0x41CB7 */
        setD32(0xF6758u, 0u);           /* 0x41CB7 int32 [0xF6758] = 0 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0x8F:  /* -> 0x41CD4 */
        setD32(0xF6758u, 1u);           /* 0x41CD4 int32 [0xF6758] = 1 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0x90:  /* -> 0x41CF4 */
        setD32(0xF6758u, 2u);           /* 0x41CF4 int32 [0xF6758] = 2 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0x91:  /* -> 0x41D14 */
        setD32(0xF6758u, 3u);           /* 0x41D14 int32 [0xF6758] = 3 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0x92:  /* -> 0x41D34 */
        setD32(0xF6758u, 4u);           /* 0x41D34 int32 [0xF6758] = 4 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0x93:  /* -> 0x41D54 */
        setD32(0xF6758u, 5u);           /* 0x41D54 int32 [0xF6758] = 5 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0x94:  /* -> 0x41D74 */
        setD32(0xF6758u, 6u);           /* 0x41D74 int32 [0xF6758] = 6 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0x95:  /* -> 0x41D94 */
        setD32(0xF6758u, 7u);           /* 0x41D94 int32 [0xF6758] = 7 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0x96:  /* -> 0x41DB4 */
        setD32(0xF6758u, 8u);           /* 0x41DB4 int32 [0xF6758] = 8 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0x97:  /* -> 0x41DD4 */
        setD32(0xF6758u, 9u);           /* 0x41DD4 int32 [0xF6758] = 9 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0x98:  /* -> 0x41DF4 */
        setD32(0xF6758u, 10u);          /* 0x41DF4 int32 [0xF6758] = 10 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0x99:  /* -> 0x41E14 */
        setD32(0xF6758u, 11u);          /* 0x41E14 int32 [0xF6758] = 11 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0x9A:  /* -> 0x41E34 */
        setD32(0xF6758u, 12u);          /* 0x41E34 int32 [0xF6758] = 12 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0x9B:  /* -> 0x41E54 */
        setD32(0xF6758u, 13u);          /* 0x41E54 int32 [0xF6758] = 13 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0x9C:  /* -> 0x41E74 */
        setD32(0xF6758u, 14u);          /* 0x41E74 int32 [0xF6758] = 14 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0x9D:  /* -> 0x41E94 */
        setD32(0xF6758u, 15u);          /* 0x41E94 int32 [0xF6758] = 15 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0x9E:  /* -> 0x41EB4 */
        setD32(0xF6758u, 16u);          /* 0x41EB4 int32 [0xF6758] = 16 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0x9F:  /* -> 0x41ED4 */
        setD32(0xF6758u, 17u);          /* 0x41ED4 int32 [0xF6758] = 17 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0xA0:  /* -> 0x41EF4 */
        setD32(0xF6758u, 18u);          /* 0x41EF4 int32 [0xF6758] = 18 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0xA1:  /* -> 0x41F14 */
        setD32(0xF6758u, 19u);          /* 0x41F14 int32 [0xF6758] = 19 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0xA2:  /* -> 0x41F34 */
        setD32(0xF6758u, 20u);          /* 0x41F34 int32 [0xF6758] = 20 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0xA3:  /* -> 0x41F54 */
        setD32(0xF6758u, 21u);          /* 0x41F54 int32 [0xF6758] = 21 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0xA4:  /* -> 0x41F74 */
        setD32(0xF6758u, 22u);          /* 0x41F74 int32 [0xF6758] = 22 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0xA5:  /* -> 0x41F94 */
        setD32(0xF6758u, 23u);          /* 0x41F94 int32 [0xF6758] = 23 */
        code = -0x3E;                       /* [ebp-0x3C] = 0xFFFFFFC2 */
        break;
    case 0xA6:  /* -> 0x41FB4 */
        code = -0x3F;                   /* 0x41FB4 mov [ebp-0x3C], 0xFFFFFFC1 */
        break;
    case 0xA9:  /* -> 0x41FC8 */
        code = -0x42;                   /* 0x41FC8 mov [ebp-0x3C], 0xFFFFFFBE */
        break;
    case 0xA7:  /* -> 0x41FDC */
        code = -0x40;                   /* 0x41FDC mov [ebp-0x3C], 0xFFFFFFC0 */
        break;
    case 0xA8:  /* -> 0x41FF0 */
        code = -0x41;                   /* 0x41FF0 mov [ebp-0x3C], 0xFFFFFFBF */
        break;
    case 0xAA:  /* -> 0x42004 */
        code = -0x43;                   /* 0x42004 mov [ebp-0x3C], 0xFFFFFFBD */
        break;
    case 0xAB:  /* -> 0x42018 */
        code = -0x44;                   /* 0x42018 mov [ebp-0x3C], 0xFFFFFFBC */
        break;
    case 0x42:  /* -> 0x4202C */
        code = 0;                             /* 0x4202C xor ecx, ecx; mov [ebp-0x3C], ecx */
        break;
    case 0x43:  /* -> 0x4203E */
        code = 1;                       /* 0x4203E mov [ebp-0x3C], 0x00000001 */
        break;
    case 0x44:  /* -> 0x42052 */
        code = 2;                       /* 0x42052 mov [ebp-0x3C], 0x00000002 */
        break;
    case 0x45:  /* -> 0x42066 */
        code = 3;                       /* 0x42066 mov [ebp-0x3C], 0x00000003 */
        break;
    case 0x46:  /* -> 0x4207A */
        code = 4;                       /* 0x4207A mov [ebp-0x3C], 0x00000004 */
        break;
    case 0x47:  /* -> 0x4208E */
        code = 5;                       /* 0x4208E mov [ebp-0x3C], 0x00000005 */
        break;
    case 0x48:  /* -> 0x420A2 */
        code = 6;                       /* 0x420A2 mov [ebp-0x3C], 0x00000006 */
        break;
    case 0x49:  /* -> 0x420B6 */
        code = 7;                       /* 0x420B6 mov [ebp-0x3C], 0x00000007 */
        break;
    case 0x4A:  /* -> 0x420CA */
        code = 8;                       /* 0x420CA mov [ebp-0x3C], 0x00000008 */
        break;
    case 0x4B:  /* -> 0x420DE */
        code = 9;                       /* 0x420DE mov [ebp-0x3C], 0x00000009 */
        break;
    case 0x4C:  /* -> 0x420F2 */
        code = 0xA;                     /* 0x420F2 mov [ebp-0x3C], 0x0000000A */
        break;
    case 0x4D:  /* -> 0x42106 */
        code = 0xB;                     /* 0x42106 mov [ebp-0x3C], 0x0000000B */
        break;
    case 0xBA:  /* -> 0x4211A */
        code = -0x48;                   /* 0x4211A mov [ebp-0x3C], 0xFFFFFFB8 */
        break;
    case 0xBB:  /* -> 0x4212E */
        code = -0x49;                   /* 0x4212E mov [ebp-0x3C], 0xFFFFFFB7 */
        break;
    case 0xBC:  /* -> 0x42142 */
        code = -0x4A;                   /* 0x42142 mov [ebp-0x3C], 0xFFFFFFB6 */
        break;
    case 0xBD:  /* -> 0x42156 */
        code = -0x4B;                   /* 0x42156 mov [ebp-0x3C], 0xFFFFFFB5 */
        break;
    case 0xBE:  /* -> 0x4216A */
        code = -0x4C;                   /* 0x4216A mov [ebp-0x3C], 0xFFFFFFB4 */
        break;
    case 0xBF:  /* -> 0x4217E */
        code = -0x4D;                   /* 0x4217E mov [ebp-0x3C], 0xFFFFFFB3 */
        break;
    case 0xC0:  /* -> 0x42192 */
        code = -0x4E;                   /* 0x42192 mov [ebp-0x3C], 0xFFFFFFB2 */
        break;
    case 0xC1:  /* -> 0x421A6 */
        code = -0x4F;                   /* 0x421A6 mov [ebp-0x3C], 0xFFFFFFB1 */
        break;
    case 0xB2:  /* -> 0x421BA */
        code = -0x50;                   /* 0x421BA mov [ebp-0x3C], 0xFFFFFFB0 */
        break;
    case 0xB3:  /* -> 0x421CE */
        code = -0x51;                   /* 0x421CE mov [ebp-0x3C], 0xFFFFFFAF */
        break;
    case 0xB4:  /* -> 0x421E2 */
        code = -0x52;                   /* 0x421E2 mov [ebp-0x3C], 0xFFFFFFAE */
        break;
    case 0xB5:  /* -> 0x421F6 */
        code = -0x53;                   /* 0x421F6 mov [ebp-0x3C], 0xFFFFFFAD */
        break;
    case 0xB6:  /* -> 0x4220A */
        code = -0x54;                   /* 0x4220A mov [ebp-0x3C], 0xFFFFFFAC */
        break;
    case 0xB7:  /* -> 0x4221E */
        code = -0x55;                   /* 0x4221E mov [ebp-0x3C], 0xFFFFFFAB */
        break;
    case 0xB8:  /* -> 0x42232 */
        code = -0x56;                   /* 0x42232 mov [ebp-0x3C], 0xFFFFFFAA */
        break;
    case 0xB9:  /* -> 0x42246 */
        code = -0x57;                   /* 0x42246 mov [ebp-0x3C], 0xFFFFFFA9 */
        break;
    case 0xB0:  /* -> 0x4225A */
        code = -0x58;                   /* 0x4225A mov [ebp-0x3C], 0xFFFFFFA8 */
        break;
    case 0xB1:  /* -> 0x4226E */
        code = -0x59;                   /* 0x4226E mov [ebp-0x3C], 0xFFFFFFA7 */
        break;
    case 0xC2:  /* -> 0x42282 */
        code = -0x5A;                   /* 0x42282 mov [ebp-0x3C], 0xFFFFFFA6 */
        break;
    case 0xC3:  /* -> 0x42296 */
        code = -0x5B;                   /* 0x42296 mov [ebp-0x3C], 0xFFFFFFA5 */
        break;
    case 0xC9:  /* -> 0x422AA */
        code = -0x5E;                   /* 0x422AA mov [ebp-0x3C], 0xFFFFFFA2 */
        break;
    case 0xCA:  /* -> 0x422BE */
        code = -0x61;                   /* 0x422BE mov [ebp-0x3C], 0xFFFFFF9F */
        break;
    case 0xCB:  /* -> 0x422D2 */
        code = -0x62;                   /* 0x422D2 mov [ebp-0x3C], 0xFFFFFF9E */
        break;
    case 0xCC:  /* -> 0x422E6 */
        code = -0x67;                   /* 0x422E6 mov [ebp-0x3C], 0xFFFFFF99 */
        break;
    case 0xCD:  /* -> 0x422FA */
        code = -0x68;                   /* 0x422FA mov [ebp-0x3C], 0xFFFFFF98 */
        break;
    case 0xCE:  /* -> 0x4230E */
        code = -0x69;                   /* 0x4230E mov [ebp-0x3C], 0xFFFFFF97 */
        break;
    case 0xCF:  /* -> 0x42322 */
        code = -0x6A;                   /* 0x42322 mov [ebp-0x3C], 0xFFFFFF96 */
        break;
    case 0xD0:  /* -> 0x42336 */
        code = -0x6B;                   /* 0x42336 mov [ebp-0x3C], 0xFFFFFF95 */
        break;
    case 0xD1:  /* -> 0x4234A */
        code = -0x6C;                   /* 0x4234A mov [ebp-0x3C], 0xFFFFFF94 */
        break;
    case 0xD2:  /* -> 0x4235E */
        code = -0x6D;                   /* 0x4235E mov [ebp-0x3C], 0xFFFFFF93 */
        break;
    case 0xD3:  /* -> 0x42372 */
        code = -0x6E;                   /* 0x42372 mov [ebp-0x3C], 0xFFFFFF92 */
        break;
    case 0xD4:  /* -> 0x42386 */
        code = -0x6F;                   /* 0x42386 mov [ebp-0x3C], 0xFFFFFF91 */
        break;
    case 0xD5:  /* -> 0x4239A */
        code = -0x70;                   /* 0x4239A mov [ebp-0x3C], 0xFFFFFF90 */
        break;
    case 0xD6:  /* -> 0x423AE */
        code = -0x71;                   /* 0x423AE mov [ebp-0x3C], 0xFFFFFF8F */
        break;
    case 0xD7:  /* -> 0x423C2 */
        code = -0x72;                   /* 0x423C2 mov [ebp-0x3C], 0xFFFFFF8E */
        break;
    case 0xD8:  /* -> 0x423D6 */
        code = -0x73;                   /* 0x423D6 mov [ebp-0x3C], 0xFFFFFF8D */
        break;
    case 0xD9:  /* -> 0x423EA */
        code = -0x74;                   /* 0x423EA mov [ebp-0x3C], 0xFFFFFF8C */
        break;
    case 0xDB:  /* -> 0x423FE */
        code = -0x76;                   /* 0x423FE mov [ebp-0x3C], 0xFFFFFF8A */
        break;
    case 0xDC:  /* -> 0x42412 */
        code = -0x77;                   /* 0x42412 mov [ebp-0x3C], 0xFFFFFF89 */
        break;
    case 0xDD:  /* -> 0x42426 */
        code = -0x78;                   /* 0x42426 mov [ebp-0x3C], 0xFFFFFF88 */
        break;
    case 0xDE:  /* -> 0x4243A */
        code = -0x83;                   /* 0x4243A mov [ebp-0x3C], 0xFFFFFF7D */
        break;
    case 0xDF:  /* -> 0x4244E */
        code = -0x84;                   /* 0x4244E mov [ebp-0x3C], 0xFFFFFF7C */
        break;
    case 0xE0:  /* -> 0x42462 */
        code = -0x85;                   /* 0x42462 mov [ebp-0x3C], 0xFFFFFF7B */
        break;
    case 0xE1:  /* -> 0x42476 */
        code = -0x86;                   /* 0x42476 mov [ebp-0x3C], 0xFFFFFF7A */
        break;
    case 0xE3:  /* -> 0x4248A */
        code = -0x87;                   /* 0x4248A mov [ebp-0x3C], 0xFFFFFF79 */
        break;
    case 0xE4:  /* -> 0x4249E */
        code = -0x88;                   /* 0x4249E mov [ebp-0x3C], 0xFFFFFF78 */
        break;
    case 0xE5:  /* -> 0x424B2 */
        code = -0x89;                   /* 0x424B2 mov [ebp-0x3C], 0xFFFFFF77 */
        break;
    case 0xE6:  /* -> 0x424C6 */
        code = -0x8A;                   /* 0x424C6 mov [ebp-0x3C], 0xFFFFFF76 */
        break;
    case 0xE7:  /* -> 0x424DA */
        code = -0x8B;                   /* 0x424DA mov [ebp-0x3C], 0xFFFFFF75 */
        break;
    case 0xE8:  /* -> 0x424EE */
        code = -0x8C;                   /* 0x424EE mov [ebp-0x3C], 0xFFFFFF74 */
        break;
    case 0xE9:  /* -> 0x42502 */
        code = -0xAC;                   /* 0x42502 mov [ebp-0x3C], 0xFFFFFF54 */
        break;
    case 0xEA:  /* -> 0x42516 */
        code = -0x95;                   /* 0x42516 mov [ebp-0x3C], 0xFFFFFF6B */
        break;
    case 0xEC:  /* -> 0x4252A */
        code = -0x97;                   /* 0x4252A mov [ebp-0x3C], 0xFFFFFF69 */
        break;
    case 0xED:  /* -> 0x4253E */
        code = -0x98;                   /* 0x4253E mov [ebp-0x3C], 0xFFFFFF68 */
        break;
    case 0xEE:  /* -> 0x42552 */
        code = -0x99;                   /* 0x42552 mov [ebp-0x3C], 0xFFFFFF67 */
        break;
    case 0xEF:  /* -> 0x42566 */
        code = -0x9A;                   /* 0x42566 mov [ebp-0x3C], 0xFFFFFF66 */
        break;
    case 0xF0:  /* -> 0x4257A */
        code = -0x9B;                   /* 0x4257A mov [ebp-0x3C], 0xFFFFFF65 */
        break;
    case 0xF1:  /* -> 0x4258E */
        code = -0x9C;                   /* 0x4258E mov [ebp-0x3C], 0xFFFFFF64 */
        break;
    case 0xF2:  /* -> 0x425A2 */
        code = -0x9D;                   /* 0x425A2 mov [ebp-0x3C], 0xFFFFFF63 */
        break;
    case 0xF3:  /* -> 0x425B6 */
        code = -0x9E;                   /* 0x425B6 mov [ebp-0x3C], 0xFFFFFF62 */
        break;
    case 0xF4:  /* -> 0x425CA */
        code = -0x9F;                   /* 0x425CA mov [ebp-0x3C], 0xFFFFFF61 */
        break;
    case 0xF5:  /* -> 0x425DE */
        code = -0xA0;                   /* 0x425DE mov [ebp-0x3C], 0xFFFFFF60 */
        break;
    case 0xF6:  /* -> 0x425F2 */
        code = -0xA1;                   /* 0x425F2 mov [ebp-0x3C], 0xFFFFFF5F */
        break;
    case 0xF7:  /* -> 0x42606 */
        code = -0xA2;                   /* 0x42606 mov [ebp-0x3C], 0xFFFFFF5E */
        break;
    case 0xF8:  /* -> 0x4261A */
        code = -0xA3;                   /* 0x4261A mov [ebp-0x3C], 0xFFFFFF5D */
        break;
    case 0xF9:  /* -> 0x4262E */
        code = -0xA4;                   /* 0x4262E mov [ebp-0x3C], 0xFFFFFF5C */
        break;
    case 0xFA:  /* -> 0x42642 */
        code = -0xA5;                   /* 0x42642 mov [ebp-0x3C], 0xFFFFFF5B */
        break;
    case 0xFB:  /* -> 0x42656 */
        code = -0xA6;                   /* 0x42656 mov [ebp-0x3C], 0xFFFFFF5A */
        break;
    case 0xFC:  /* -> 0x4266A */
        code = -0xA7;                   /* 0x4266A mov [ebp-0x3C], 0xFFFFFF59 */
        break;
    case 0xFD:  /* -> 0x4267E */
        code = -0xA8;                   /* 0x4267E mov [ebp-0x3C], 0xFFFFFF58 */
        break;
    case 0xFE:  /* -> 0x42692 */
        code = -0xA9;                   /* 0x42692 mov [ebp-0x3C], 0xFFFFFF57 */
        break;
    case 0xFF:  /* -> 0x426A6 */
        code = -0xAA;                   /* 0x426A6 mov [ebp-0x3C], 0xFFFFFF56 */
        break;
    case 0x100: /* -> 0x426BA */
        code = -0xAB;                   /* 0x426BA mov [ebp-0x3C], 0xFFFFFF55 (falls into 0x426C1) */
        break;
    /* 0x426C1, the default: code stays -100. Ids 3..0x107 whose table entry is 0x426C1 (and every id outside 3..0x107,
       through the range check above). */
    case 0x11:
    case 0x1A:
    case 0x54:
    case 0x55:
    case 0x59:
    case 0x5A:
    case 0x5B:
    case 0x5C:
    case 0x5D:
    case 0x5E:
    case 0x5F:
    case 0x60:
    case 0x61:
    case 0x62:
    case 0x63:
    case 0x64:
    case 0x65:
    case 0x66:
    case 0x67:
    case 0x68:
    case 0xAC:
    case 0xAD:
    case 0xAE:
    case 0xAF:
    case 0xEB:
    case 0x102:
    case 0x104:
    case 0x105:
    case 0x106:
    default:
        break;
    }

    return code;                                /* 0x426C1 mov eax, [ebp-0x3C]; epilogue */
}
