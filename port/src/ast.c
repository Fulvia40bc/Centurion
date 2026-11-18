/* The AST chart of AC_MAIN.EXE and the score helpers (module N2 of docs/MAP_PLAN.md): 0x136B0 (the chart, astblan2.gif),
   0x147C4 (a nation's score), 0x1486C (advance groups), 0x17760 (advance points), 0x6DD98 (value of the trade cards)
   and 0x7BA64 (the self-running demo's unattended check). Literal translations; the address of each instruction a
   line comes from is in its comment. D = read in the code, G = inference. Prototypes and the data layouts: game.h
   (section "AST chart, button kit, click wait, dispatcher, score helpers") and ui.h.

   Callers: f136B0(1) from the nation choice's AST button (0x3012F, choose.c), f136B0(2) from the dispatcher (id 3,
   0x3FBDA), f136B0(3) from the AST phase (0x55202, the only caller that reads eax).

   Watcom runtime: none called here. __CHK dropped. Library calls through fg.h in the original argument order; the
   mouse and key reads write the data fields the original passes (0xE1628/0xE162C/0xE1630, 0xE1634/0xE1635).

   Heap pointers in 32-bit data fields: these routines keep none of their own. They read the sprite records' +0 handles
   (0xF4ACC + 8n, 0xF521C, 0xF5224) through f3AF8C and [0xF522C] through f3CD6C.

   Not ported yet (calls not_ported(addr)):
     0x3CC68 (module S1 of MAP_PLAN.md: show the background buffer [0xF52CC], fg_vbpaste(0, 0x27F, 0, 0x1DF, 0, 0x1DF)
             with the mouse hidden), called at 0x13B9E (modes 1 and 2) and 0x13CB2 (mode 3), right after the chart is
             drawn. Until it is translated the chart stops there.

   1995 bugs: none found (no g_original branches). Noted, not changed:
     - a nation on the last AST square (16) reads entry 17 of the table, which is the next nation's entry 0 (or, for
       n = 9, the 4 bytes after the table: (0, 0xA00) in obj2), so next = 0 and mode 3 would step it to 17. Not
       reachable: the only step onto 16 is next == 5 (entry 16 is the only era 5 in every row), which sets won; the AST
       phase then leaves its turn loop (D: 0x5520A jne 0x55222, 0x55227 jmp 0x5483E, 0x5483E cmp [ebp-0x24],0; jne
       0x5522C) and ends the game (0x5522C: removes savgameb.dat, f44CAC), skipping the save 0x5E8EC.
     - f1486C sets flag 0xF62A1 instead of 0xF62A2 when advance 19 gives the third group; nothing reads 0xF62A0..
       0xF62A4 (game.h), so no effect. Advance 19 counting for the third group as well as the fifth is not a bug: it is
       Mysticism (names at 0xB6CA4.., D), which the board game puts in Arts and Religion (G: the board-game rule). */

#include <stdint.h>
#include "game.h"
#include "fg.h"

/* Defined in ui_screen.c, not in ui.h (title.c and ui_screen.c declare it the same way): 0x78508 empties the keyboard
   buffer (fg_intkey until key 0). */
void f78508(void);

/* ---- 0x136B0: the AST chart -------------------------------------------------------------------------------------- */

/* The AST table 0xBCFDA: int16 {era, need} pairs, 17 per nation, nation n at 0xBCFDA + 68(n-1), entry k at +4k (the
   code: edx = (n-1) << 4 + (n-1), then [edx*4 + 4*k + 0xBCFDA] / +0xBCFDE / +0xBCFE0) */
static uint32_t ast_entry(int32_t n, int32_t k) { return (uint32_t)(0xBCFDA + 4 * ((n - 1) * 17) + 4 * k); }

/* 0x136B0: the AST chart. mode (eax) 1 = from the nation choice (names of the choosable nations only), 2 = view from
   the dispatcher (steps evaluated, not applied), 3 = the AST phase (steps applied, score history, turn + 1). Returns 0
   in modes 1 and 2; in mode 3, 1 if a nation stepped onto the last square, else 0. */
int f136B0(int mode)
{
    int32_t n;                  /* [ebp-0x58] */
    uint32_t rec;               /* [ebp-0x5c] nation record 0xF14CC + 199n */
    int32_t shown;              /* [ebp-0x40] 1 = draw the name */
    int32_t i;                  /* [ebp-0x50] */
    int32_t step;               /* [ebp-0x44] -1, 0 or 1 */
    int32_t won;                /* [ebp-0x38] */
    int32_t r;                  /* [ebp-0x48] */
    int32_t t;                  /* [ebp-0x54] the score-history column */
    int32_t groups_n;           /* [ebp-0x3c] f1486C's count */
    int16_t cur;                /* [ebp-0x30] entry[pos].era (word stores into a dword local, read movsx) */
    int16_t next;               /* [ebp-0x2c] entry[pos+1].era */
    int16_t need;               /* [ebp-0x28] entry[pos+1].need */
    int16_t city;               /* [ebp-0x24] */
    int16_t adv;                /* [ebp-0x20] */
    int16_t groups;             /* [ebp-0x1c] */
    int16_t count;              /* [ebp-0x18] advances owned (inc dword, read movsx word) */
    int32_t c, pos, k;

    won = 0;                                                    /* 0x136CA */
    f3CD6C(4);                                                  /* 0x136D2 astblan2.gif into [0xF52CC] */
    fg_vbopen(S32(0xF52CCu));                                   /* 0x136DE draw into the background buffer */
    f39F90(1);                                                  /* 0x136F0 font 1 (ecx = 1 kept across the call) */
    for (n = 1; n <= 9; n++) {                                  /* 0x136F5 [ebp-0x58] = ecx; 0x13AC0 cmp 9; jg */
        step = 0;                                               /* 0x13AC8 */
        rec = (uint32_t)(n * 0xC7 + 0xF14CC);                   /* 0x13ACB imul 0xC7; 0x13AD9 */
        shown = 1;                                              /* 0x13AE1 */
        if (mode != 1 && D8(rec + 0x9A) == 0)                   /* 0x13AE7 cmp ecx,ebx; je; 0x13AEE byte +0x9A; jne */
            continue;                                           /* 0x13AF7 cmp mode,1; jne 0x13ABA */
        if (mode == 1) {                                        /* 0x13B00 cmp edi,1; jne 0x13726 */
            shown = 0;                                          /* 0x13B0B */
            for (i = 0; i < S32(0xF7C9Cu); i++)                 /* 0x13B0E, 0x13706 cmp; jge 0x13726 */
                if (S32((uint32_t)(0xF7CA0 + 4 * i)) == n) {    /* 0x13711, 0x1371B cmp; jne 0x136FD */
                    shown = 1;                                  /* 0x1371F */
                    break;
                }
        }

        city = 0;                                               /* 0x13728 [ebp-0x24] */
        groups = 0;                                             /* 0x1372B [ebp-0x1c] */
        adv = 0;                                                /* 0x1372E [ebp-0x20] */
        need = 0;                                               /* 0x13731 [ebp-0x28] */
        count = 0;                                              /* 0x13736 [ebp-0x18] */
        fg_setcolor(0x13);                                      /* 0x13739 */
        if (shown != 0) {                                       /* 0x13741 cmp 0; je 0x137FA */
            fg_setcolor(5);                                     /* 0x1374D */
            if (n == 3)                                         /* 0x13758 cmp edx,3; jne */
                f3A0B0(0x48, 25 * (n - 1) + 0x86, 0x35,         /* 0x13789 one more colour-5 copy (ILLYRIA) */
                       DSTR(D32((uint32_t)(0xC768C + 4 * n))));
            f3A0B0(0x48, 25 * (n - 1) + 0x87, 0x35,             /* 0x137BA the name's shadow */
                   DSTR(D32((uint32_t)(0xC768C + 4 * n))));
            fg_setcolor(0xF);                                   /* 0x137C1 */
            f3A0B0(0x47, 25 * (n - 1) + 0x86, 0x35,             /* 0x137F5 the name */
                   DSTR(D32((uint32_t)(0xC768C + 4 * n))));
        }
        if (mode == 1)                                          /* 0x137FA cmp 1; je 0x13ABA */
            continue;

        /* The step (0x13804..0x13A49) */
        pos = S8(rec + 0x5C);                                   /* 0x13815 dword [rec+0x59] sar 24 */
        cur = S16(ast_entry(n, pos));                           /* 0x1381B word [edx + 4*pos + 0xBCFDA] */
        pos = S8(rec + 0x5C);                                   /* 0x13829 */
        next = S16(ast_entry(n, pos) + 4);                      /* 0x13841 word [4*pos + 4*eax + 0xBCFDE] */
        c = S8(rec + 0x51);                                     /* 0x1384F dword [rec+0x4e] sar 24: cities */
        if (c == 0)                                             /* 0x13855 test; jne */
            city = 0;                                           /* 0x1385B */
        else if (c == 1)                                        /* 0x13863 */
            city = 1;                                           /* 0x13868 */
        else if (c >= 2 && next == 1)                           /* 0x13874 cmp 2; jl; 0x1387D movsx; cmp 1 */
            city = 2;                                           /* 0x13882 */
        else if (S8(rec + 0x51) >= 3 && next == 2)              /* 0x13894, 0x1389D */
            city = 3;                                           /* 0x138A2 */
        else if (S8(rec + 0x51) >= 4 && next == 3)              /* 0x138B4, 0x138BD */
            city = 4;                                           /* 0x138C2 */
        else if (S8(rec + 0x51) >= 5)                           /* 0x138D4 cmp 5; jl */
            city = 5;                                           /* 0x138D9 */

        groups_n = f1486C(n);                                   /* 0x138E3 */
        if (groups_n == 5 && next >= 3)                         /* 0x138EB, 0x138F5 cmp 3; jl */
            groups = 4;                                         /* 0x138FA */
        else if (groups_n >= 3)                                 /* 0x13906 cmp esi,3; jl */
            groups = 3;                                         /* 0x1390B */

        for (k = 0; k < 0x18; k++)                              /* 0x13914, 0x1391F cmp 0x18; jge */
            if (D8(rec + (uint32_t)k + 0x24) != 0)              /* 0x1392B mov dh,[eax+0x24]; test; je */
                count++;                                        /* 0x13935 inc dword [ebp-0x18] */
        if (count >= 9)                                         /* 0x1393A movsx word; cmp 9; jl */
            adv = 4;                                            /* 0x13943 */

        setD16(rec + 0x5E, (uint16_t)f17760(n));                /* 0x1394D, 0x13957 mov word [edx+0x5e], ax */
        step = 0;                                               /* 0x1395B ecx = 0 */
        pos = S8(rec + 0x5C);                                   /* 0x13961 */
        need = S16(ast_entry(n, pos) + 6);                      /* 0x13979 word [ebx + 4*eax + 0xBCFE0] */

        if (S8(rec + 0x51) < 1 && cur != 0) {                   /* 0x1398D cmp 1; jge; 0x13992 movsx; test; je */
            step = -1;                                          /* 0x1399A */
        } else if (next == 5 && city >= 5) {                    /* 0x139AA cmp 5; jne 0x139C8; 0x139B3 cmp 5; jl 0x139C8
                                                                   (next == 5 with city < 5 fails every test below) */
            step = 1;                                           /* 0x139BD */
            won = 1;                                            /* 0x139C0 */
        } else if (next == 4) {                                 /* 0x139CF cmp 4; jne 0x139F3 */
            if ((int16_t)D16(rec + 0x5E) >= need                /* 0x139D4 mov ax,[eax+0x5e]; 0x139DB cmp ax,si; jl */
                && city >= 5)                                   /* 0x139E8 cmp 5; jl */
                step = 1;                                       /* 0x13A49 */
        } else if (next == 3) {                                 /* 0x139FA cmp 3; jne 0x13A13 */
            if ((int32_t)city + adv + groups >= 0xC)            /* 0x139FC..0x13A0C movsx, add; cmp 0xC; jl */
                step = 1;
        } else if (next == 2) {                                 /* 0x13A1A cmp 2; jne 0x13A2D */
            if ((int32_t)city + groups >= 6)                    /* 0x13A20..0x13A26 cmp 6; jl */
                step = 1;
        } else if (next == 1) {                                 /* 0x13A34 cmp 1; jne 0x13A41 */
            if (city >= 2)                                      /* 0x13A3A cmp 2; jl */
                step = 1;
        } else if (next == 0) {                                 /* 0x13A45 test; jne 0x13A50 */
            step = 1;                                           /* 0x13A49 */
        }

        if (mode == 3) {                                        /* 0x13A50 cmp 3; jne 0x13A73 */
            if (step > 0)                                       /* 0x13A59 test; jle */
                setD8(rec + 0x5C, (uint8_t)(D8(rec + 0x5C) + 1));   /* 0x13A60 inc byte [eax+0x5c] */
            else if (step == -1)                                /* 0x13A68 cmp -1; jne */
                setD8(rec + 0x5C, (uint8_t)(D8(rec + 0x5C) - 1));   /* 0x13A70 dec byte [eax+0x5c] */
        }
        pos = S8(rec + 0x5C);                                   /* 0x13A8E dword [eax+0x59] sar 24 */
        f3AF8C((uint32_t)(n * 8 + 0xF4ACC),                     /* 0x13AB5 the nation's token (tokens.gif, 24x23) */
               26 * pos + 0x83,                                 /* 0x13A94..0x13AA4 */
               25 * (n - 1) + 0x80);                            /* 0x13A76..0x13A85 */
    }

    if (mode != 3) {                                            /* 0x13B16 cmp 3; je 0x13BC7 */
        f3BB48(0xBB800u, 0x2A, 0x109);                          /* 0x13B2F CONTINUE x 265 */
        f3BC64(0xBB800u, 0x2A, 0x192);                          /* 0x13B43 CONTINUE y 402 */
        f3AF8C(0xF521Cu, 0x106, 0x18F);                         /* 0x13B59 */
        for (i = 0; S16(0xBB800u) > i; i++)                     /* 0x13B69 dword [0xBB7FE] sar 16; cmp; jle */
            f3B4E0((uint32_t)(0xBB802 + 20 * i));               /* 0x13B76..0x13B92 */
        fg_vbclose();                                           /* 0x13B99 */
        f3CC68();                                               /* 0x13B9E: show the background buffer */
        do                                                      /* 0x13BA3 */
            r = f78BA4(0xBB800u, 0);                            /* 0x13BAA */
        while (r == -100 || r != -1);                           /* 0x13BB5 cmp -0x64; je; 0x13BBD cmp -1; jne */
    } else {
        t = S16(0xF40FCu);                                      /* 0x13BC7 dword [0xF40FA] sar 16 */
        if (t > 0x2C)                                           /* 0x13BCF cmp 0x2C; jle */
            t = 0x2C;                                           /* 0x13BD4 */
        for (i = 1; i <= 9; i++)                                /* 0x13BDC, 0x13BEB cmp 9; jg */
            if (D8((uint32_t)(i * 0xC7 + 0xF1566)) != 0)        /* 0x13BF1 imul; 0x13BF8 byte +0x9A; test; je */
                setD16((uint32_t)(0xEE9F0 + 90 * i + 2 * t),    /* 0x13C24 mov word [eax + 2*edx + 0xEE9F0], bx */
                       (uint16_t)f147C4(i));                    /* 0x13C05 */
        setD16(0xF40FCu, (uint16_t)(D16(0xF40FCu) + 1));        /* 0x13C33 mov di,[0xF40FC]; inc edi; 0x13C45 */
        f3AF8C(0xF521Cu, 0x106, 0x180);                         /* 0x13C4C */
        f3AF8C(0xF5224u, 0xC2, 0x19D);                          /* 0x13C60 */
        for (i = 0; S16(0xBC8F0u) > i; i++)                     /* 0x13C72 dword [0xBC8EE] sar 16; cmp; jle */
            f3B4E0((uint32_t)(0xBC8F2 + 20 * i));               /* 0x13C7F..0x13C9B */
        fg_vbclose();                                           /* 0x13CA7 */
        setD32(UI_CUR_DIALOG, 0xBC8F0u);                        /* 0x13CAC mov [0xF8E78], edi */
        f3CC68();                                               /* 0x13CB2: show the background buffer */
        {
            int mx, my, mb;
            fg_mousepos(&mx, &my, &mb);                         /* 0x13CC6 fg_mousepos(0xE1628, 0xE162C, 0xE1630) */
            setD32(0xE1628u, (uint32_t)mx);
            setD32(0xE162Cu, (uint32_t)my);
            setD32(0xE1630u, (uint32_t)mb);
        }
        do {                                                    /* 0x13CCE */
            if (f7BA64() == 0) {                                /* 0x13CCE call; test; jne 0x13CE8 */
                r = f78BA4(0xBC8F0u, 0);                        /* 0x13CDE */
            } else {
                f78A20(D16(0xF40BAu) == 0 ? 0xFA : 0x7D0);      /* 0x13CE8 cmp word [0xF40BA],0; 0x13CFE */
                r = -1;                                         /* 0x13D03 */
            }
        } while (r == -100 || r != -1);                         /* 0x13D0A cmp -0x64; je; 0x13D13 cmp -1; jne */
    }

    if (mode == 3)                                              /* 0x13D18 cmp 3; jne 0x13D31 */
        return won == 1;                                        /* 0x13D21 cmp esi,1; sete al; and eax,0xFF */
    return 0;                                                   /* 0x13D31 */
}

/* ---- 0x147C4: a nation's score ------------------------------------------------------------------------------------ */

/* 0x147C4: nation n's score (int32 sum at [ebp-0x8]). 5 callers, all read eax. */
int f147C4(int n)
{
    int32_t score = 0;                                          /* 0x147DA [ebp-0x8] */

    score += S16((uint32_t)(n * 0xC7 + 0xF152A));               /* 0x147E4 dword [+0xF1528] sar 16: int16 +0x5E */
    score += f6DD98(S8((uint32_t)(n * 0xC7 + 0xF1509)));        /* 0x147F7 dword [+0xF1506] sar 24: int8 +0x3D; 0x14800 */
    score += S8((uint32_t)(n * 0xC7 + 0xF150A));                /* 0x1480F dword [+0xF1507] sar 24: int8 +0x3E */
    score += 100 * S8((uint32_t)(n * 0xC7 + 0xF1528));          /* 0x14822 dword [+0xF1525] sar 24: int8 +0x5C; *25*4 */
    score += 50 * S8((uint32_t)(n * 0xC7 + 0xF151D));           /* 0x14846 dword [+0xF151A] sar 24: int8 +0x51; *25*2 */
    return score;                                               /* 0x14862 */
}

/* ---- 0x1486C: advance groups -------------------------------------------------------------------------------------- */

/* 0x1486C: in how many of the five advance groups nation n owns an advance (int8 +0x24+i > 0, signed), with the flags
   0xF62A0..0xF62A4 (cleared first; each group's loop ends at its first owned advance). Groups: advances 0..6, 6..10,
   10..14 or advance 19, 14..18, 19..23. One caller (0x138E3). */
int f1486C(int n)
{
    int32_t count = 0;                                          /* 0x14883 [ebp-0x8] */
    int8_t i;                                                   /* [ebp-0x4] byte counter, read movsx */

    for (i = 0; i < 5; i++)                                     /* 0x14888, 0x14897 cmp 5; jge */
        setD8((uint32_t)(0xF62A0 + i), 0);                      /* 0x148A2 */

    for (i = 0; i <= 6; i++)                                    /* 0x148AC, 0x148BB cmp 6; jg 0x148E3 */
        if (S8((uint32_t)(n * 0xC7 + 0xF14F0 + i)) > 0) {       /* 0x148CB dword [edx + eax + 0xF14ED] sar 24; jle */
            count++;                                            /* 0x148D9 */
            setD8(0xF62A0u, 1);                                 /* 0x148DC */
            break;
        }

    for (i = 6; i <= 0xA; i++)                                  /* 0x148E3, 0x148F3 cmp 0xA; jg 0x1491B */
        if (S8((uint32_t)(n * 0xC7 + 0xF14F0 + i)) > 0) {       /* 0x14903 */
            count++;                                            /* 0x14911 */
            setD8(0xF62A1u, 1);                                 /* 0x14914 */
            break;
        }

    if (S8((uint32_t)(n * 0xC7 + 0xF1503)) > 0) {               /* 0x14922 dword [+0xF1500] sar 24: advance 19; jle */
        count++;                                                /* 0x1492F */
        setD8(0xF62A1u, 1);                                     /* 0x14932 (0xF62A1, not 0xF62A2: no reader, see the top) */
    } else {
        for (i = 0xA; i <= 0xE; i++)                            /* 0x1493B, 0x1494B cmp 0xE; jg 0x14987 */
            if (S8((uint32_t)(n * 0xC7 + 0xF14F0 + i)) > 0      /* 0x1495B; jg 0x1497D */
                || S8((uint32_t)(n * 0xC7 + 0xF1503)) > 0) {    /* 0x14970 advance 19 again; jle 0x14941 */
                count++;                                        /* 0x1497D */
                setD8(0xF62A2u, 1);                             /* 0x14980 */
                break;
            }
    }

    for (i = 0xE; i <= 0x12; i++)                               /* 0x14987, 0x14997 cmp 0x12; jg 0x149BF */
        if (S8((uint32_t)(n * 0xC7 + 0xF14F0 + i)) > 0) {       /* 0x149A7 */
            count++;                                            /* 0x149B5 */
            setD8(0xF62A3u, 1);                                 /* 0x149B8 */
            break;
        }

    for (i = 0x13; i <= 0x17; i++)                              /* 0x149BF, 0x149CF cmp 0x17; jg 0x149F7 */
        if (S8((uint32_t)(n * 0xC7 + 0xF14F0 + i)) > 0) {       /* 0x149DF */
            count++;                                            /* 0x149ED */
            setD8(0xF62A4u, 1);                                 /* 0x149F0 */
            break;
        }

    return count;                                               /* 0x149F7 */
}

/* ---- 0x17760: advance points -------------------------------------------------------------------------------------- */

/* 0x17760: nation n's advance points: the sum of the costs int16 0xBD480[i] of the advances i (0..23) with int8
   +0x24+i == 1. 3 callers read eax (0x13957 and 0x17635 store ax into +0x5E). */
int f17760(int n)
{
    int32_t sum = 0;                                            /* 0x17777 [ebp-0x8] */
    uint32_t rec = (uint32_t)(n * 0xC7 + 0xF14CC);              /* 0x1777A imul; 0x17788 [ebp-0x4] */
    int32_t i;                                                  /* [ebp-0xc] */

    for (i = 0; i <= 0x17; i++)                                 /* 0x1778D, 0x17798 cmp 0x17; jg */
        if (S8(rec + (uint32_t)i + 0x24) == 1)                  /* 0x177A4 dword [eax+0x21] sar 24; cmp 1; jne */
            sum += S16((uint32_t)(0xBD480 + 2 * i));            /* 0x177B2 dword [2*eax + 0xBD47E] sar 16; 0x177BC */
    return sum;                                                 /* 0x177C1 */
}

/* ---- 0x6DD98: value of the trade cards ---------------------------------------------------------------------------- */

/* 0x6DD98: the value of nation n's trade cards: sum over card ids i 1..18 of c * c * ((i + 1) >> 1), c = int8 +i (the
   number of cards of id i held). 6 call sites, all read eax; two pass n = 0 (record 0). */
int f6DD98(int n)
{
    int32_t sum = 0;                                            /* 0x6DDAE [ebp-0x8] */
    uint32_t rec = (uint32_t)(n * 0xC7 + 0xF14CC);              /* 0x6DDB1 imul; 0x6DDBF [ebp-0x4] */
    int32_t i;                                                  /* [ebp-0xc] */

    for (i = 1; i <= 0x12; i++)                                 /* 0x6DDC2, 0x6DDD1 cmp 0x12; jg */
        sum += S8(rec + (uint32_t)i) * S8(rec + (uint32_t)i)    /* 0x6DDDD, 0x6DDE6 movsx byte; 0x6DDE9 imul */
               * ((i + 1) >> 1);                                /* 0x6DDEF inc; sar; 0x6DDF2 imul; 0x6DDF5 add */
    return sum;                                                 /* 0x6DDFA */
}

/* ---- 0x7BA64: the self-running demo's unattended check ------------------------------------------------------------ */

/* 0x7BA64: 0 when int16 [0xF40B8] and [0xF40BA] (the self-running demo's player count, /SRD) are both 0. Else a key:
   Esc quits the game when [0xF40BA] != 0, otherwise clears [0xF40B8] and returns 0; no key or any other key empties the
   keyboard buffer (f78508). Then 1 if no mouse button is down, else 0. 20 callers read eax. */
int f7BA64(void)
{
    unsigned char key, aux;

    if (S16(0xF40B8u) == 0 && S16(0xF40BAu) == 0)              /* 0x7BA74 dword [0xF40B6] sar 16; 0x7BA80 [0xF40B8] sar 16 */
        return 0;                                               /* 0x7BA8A je 0x7BB09 (eax = 0 from the sar) */

    fg_intkey(&key, &aux);                                      /* 0x7BA9A fg_intkey(0xE1634, 0xE1635) */
    setD8(0xE1634u, key);
    setD8(0xE1635u, aux);
    if (D8(0xE1634u) == 0x1B) {                                 /* 0x7BAA4 mov al,[0xE1634]; cmp 0x1B; jne 0x7BAD4 */
        if (D16(0xF40BAu) != 0) {                               /* 0x7BAAE mov dx,[0xF40BA]; test; je 0x7BAC9 */
            setD8(0xF5500u, 0);                                 /* 0x7BABC the quit message "" */
            f44CAC();                                           /* 0x7BAC2 quit (the jmp 0x7BAD9 after it is never reached) */
        }
        setD16(0xF40B8u, 0);                                    /* 0x7BAC9 mov [0xF40B8], dx (dx = 0) */
        return 0;                                               /* 0x7BAD0 */
    }
    f78508();                                                   /* 0x7BAD4 empty the keyboard buffer */

    {
        int mx, my, mb;
        fg_mousepos(&mx, &my, &mb);                             /* 0x7BAE8 fg_mousepos(0xE1628, 0xE162C, 0xE1630) */
        setD32(0xE1628u, (uint32_t)mx);
        setD32(0xE162Cu, (uint32_t)my);
        setD32(0xE1630u, (uint32_t)mb);
    }
    if (D32(0xE1630u) != 0)                                     /* 0x7BAED mov edx,[0xE1630]; test; je 0x7BB04 */
        return 0;                                               /* 0x7BAFA */
    return 1;                                                   /* 0x7BB04 */
}
