/* The opening maps of the nations (module START of docs/MAP_PLAN.md, the computers' starting territories): 0x14C6C,
   which picks the .BPM file of nation n for the player count and the nations in play. Literal translation; the address
   of each instruction a line comes from is in its comment. D = read in the code or the files, G = inference.
   Prototype: aistart.h.

   Callers (D): 0x454D0 at 0x4593D ("%s place your token in a starting territory", start.c) and 0x14A00 at 0x14A60
   (called by main at 0x118B1; 0x14AA4, its twin without the name choice, at 0x1189C). Both loop n = 1..9 over the
   nations in play (byte +0x9A != 0), call f14C6C(buffer, n), then 0x15FE0(n, name) with the buffer, or with
   0xF6064 + 13n when int32 [0xF60E8 + 4n] != 0. Humans' nations get a map too (both branches of the +0x9A test).

   Watcom runtime: none. Every copy is an inline strcpy (two bytes a step, the NUL copied), translated as strcpy from
   the literal in the data object (DSTR), as choose.c does; __CHK dropped. No game_rand. No callees.

   Data (D):
     [0xF40F0]          int32 players (humans + computers, START GAME 0x4E281)
     0xF14CC + 0xC7n    nation n's record; byte +0x9A: 0 not in play, 2 computer (0x30191), 3 human (0x2F7B6). The
                        human test reads dword [rec + 0x97] sar 24 == 3 (signed byte); "in play" tests are
                        cmp byte [..], 0 on 0xF1566 + 0xC7m: m 2 IBERIA 0xF16F4, 3 ILLYRIA 0xF17BB, 4 THRACE 0xF1882,
                        5 CRETE 0xF1949, 8 BABYLON 0xF1B9E (the only five this function tests)
     0xF60E8            int32[10] per nation: use the override name. Main clears all 40 bytes at 0x10172
                        (0x7D5D0(0xF60E8, 0, 0x28) = memset) and no code writes them (the other references all read:
                        0x14A68, 0x14AE1, 0x14C99, 0x45945, and Iberia's flag [0xF60F0] = 0xF60E8[2] at 0x1290F and
                        0x129BD in 0x128F0, newgame.c). G: a developers' switch from before release, whose
                        command-line parser was removed; the names it would use are char[13] at 0xF6064 + 13n.
                        D: 0x128F0 (newgame.c), when Iberia's flag is set, writes bytes 1..3 of Iberia's name
                        (0xF607F..0xF6081) as "BER" (board bit 0x02 of 0xE1624) or "TAL": the override name followed
                        the Iberia/Italy switch (IBERnn / ITALnn), more evidence of a working developers' feature
     Literals: the 112 names "AFRC30.bpm" .. "EGYP80.bpm" at 0xB1010..0xB14D5, one copy per use (each branch has its
     own literal even when the name repeats).
   Locals: [ebp-0x18] dst, [ebp-0x14] n, [ebp-0x10] rec = 0xF14CC + 0xC7n (imul, 0x14C85).

   The choice (D; "(H)" = only when n is human; an untested condition means any):
     1 AFRICA   3: AFRC30. 4: (H) Illyria, Thrace and Crete all in play -> AFRC40, else AFRC41 (computer always
                AFRC41). 5..8: AFRCp0.
     2 IBERIA   3: Thrace in play -> ITAL32; else (H) Illyria in play -> ITAL30; else ITAL33. 4: IBER40, 5: IBER50,
                6: IBER61, 7: IBER71, 8: IBER81 (IBER60/70/80, "Base map by Jim", are never read).
     3 ILLYRIA  3: Thrace -> ILLY33; else (H) Iberia -> ILLY32; else ILLY31. 4: (H) Illyria, Thrace and Crete all in
                play -> ILLY40 (BUG(1995) below: fixed, Africa in place of Illyria); else Thrace -> ILLY41,
                else ILLY42. 5, 6, 7: Thrace -> ILLY50, ILLY61, ILLY70; else ILLY51, ILLY62, ILLY71. 8: ILLY80.
     4 THRACE   3: THRC31. 4..7: Illyria in play -> THRC41, THRC51, THRC60, THRC70; else THRC40, THRC50, THRC61,
                THRC71. 8: THRC80.
     5 CRETE    CRETp0 for p = 3..8.
     6 ASIA     ASIAp0 for p = 5..8 (lea eax,[edi-5]); fewer players: nothing.
     7 ASSYRIA  6: ASSY60. 7, 8: Babylon in play -> ASSY70 / ASSY80, else ASSY71 / ASSY81. Fewer: nothing.
     8 BABYLON  7: BABL70, 8: BABL80. Fewer: nothing.
     9 EGYPT    EGYPp0 for p = 6..8. Fewer: nothing.
   Human and computer branches are separate code with the same results except the three (H) tests. Players outside
   3..8 (or below the nation's first count) leave dst untouched; the setups the game makes never get there. D: the
   nations a game can have are exactly the nations with maps for its player count: the humans' candidate lists
   (0xC0794 record v, v from [0xF40F0] as in game.h, obj2) are 1..5 for 3 and 4 players, 1..6 for 5, 1..7 and 9 for
   6, 1..9 for 7 and 8, and the computer's fixed orders (nations.c) stay inside them, also with its BUG(1995) A kept,
   which only adds nations from the same lists.
   Check (V, scratch harness): f14C6C against an interpreter of the machine code of 0x14C6C (the listing's
   instructions and jump tables, tables checked against obj1.bin), for n 0..10, [0xF40F0] 0..9 and 0xFFFFFFFF, every
   in-play pattern of nations 1..9, human or computer: 123,904 cases, identical; the override path for n 1..9 too.
   Review (V): a second interpreter, written apart (jump tables from obj1.bin checked against fixups.txt; callee-saved
   registers and the return address checked at every ret; the 32-byte dst compared whole, so untouched and
   past-the-NUL bytes count), n 0..10, players 0..9, 0xFFFFFFFF and 0x80000003, 512 patterns, human or not, and the
   override: 135,212 cases identical with g_original set; without it exactly the 64 cases of the fix differ
   (n 3 human, 4 players, Thrace and Crete in play, Africa not: ILLY41 for ILLY40).

   The .BPM files (D, read from ACIV.CT3 with tools/ct_list.py's directory reader; the code that reads them is 0x15FE0,
   start.c): 66 files of 523 bytes, kind 0, all in ACIV.CT3, the catalogue at [0xF40DC] (title.c 0x108C9). 0x15FE0
   takes cat_size(name, [0xF40DC]); if it is -2 (not in the catalogue) it loads a loose file of that name with
   0x7D334(name, [0xF522C]) (G: a developers' path for edited maps), else cat_load(name, [0xF522C], [0xF40DC]). Then:
     +0x000        u8       1 in every file (not read by the game)
     +0x001..0x029 char[41] a label, NUL-terminated, the rest of the field holding older, longer labels' leftovers
                            (an editor that wrote the whole buffer; not read by the game)
     +0x02A        u8       the player count, = the file name's digit in all 66 (not read by the game)
     +0x02B        int8[160] -> 0xEED84 + 160n (0x16041), per territory t: the starting-territory score; values 66..120,
                            or 0xFF (-1) = never. The computer's pick 0x45F88 (start.h) takes, from 0x7F down, the
                            territories t < 157 whose score + 5 * (bit 2 of the +0x0CB byte) is the highest, and draws
                            one with game_rand
     +0x0CB        u8[160]  -> 0xF0044 + 160n (0x16075); values 0, 1, 2, 4 (zero in most files); bit 2 = +5 above
     +0x16B        u8[160]  -> 0xEFA04 + 160n (0x160AC); values 0..12 (G: used later in the game, AI modules M7)
   (rep movsd/movsb of 0xA0 bytes each; 0x16B + 0xA0 = 0x20B = 523.) Main then copies 0xEED84[10][160] to
   0xEF3C4 (0x11884, and 0x14A7B / 0x14B0E).
   Read by the code: 58 of the 66 (the names above). Never read: ITAL31, ILLY30, THRC30, IBER60, IBER70, IBER80,
   THRC72, THRC73.

   1995 bugs:
     - BUG(1995), fixed (review), --original keeps it: 0x15254 (Illyria, human, 4 players) tests Illyria's own
       byte (D: always set for a nation in play) together with Thrace and Crete: the same three addresses as
       Africa's test at 0x14D4D, so G a copy of it. For ILLY40 ("non-iberia", the label AFRC40 has too) the test
       that fits is Africa, Thrace and Crete; as written, a 4-player game of Iberia, Illyria, Thrace and Crete (no
       Africa, possible only with 4 humans) gives the human Illyria ILLY40 instead of ILLY41.
     Oddity kept (G, for Erik's ruling):
     - The labels do not always match the choice: ILLY32 ("illy vs italy vs thrc") is read only when Thrace is NOT
       in play; ITAL32 ("Italy vs Africa vs Thrace") also serves Italy + Illyria + Thrace, which ITAL31 ("Italy vs
       Illyria vs Thrace") describes and which is never read. The labels are the editor's, not the game's, so this
       may be renaming during development rather than a bug. */

#include <stdint.h>
#include <string.h>
#include "state.h"
#include "aistart.h"

/* The inline strcpy loop (e.g. 0x14D2B..0x14D42): from the literal at link address src in the data object. */
static void bpm_copy(char *dst, uint32_t src)
{
    strcpy(dst, DSTR(src));
}

/* 0x14C6C */
void f14C6C(char *dst, int n)
{
    uint32_t rec;   /* [ebp-0x10] */
    uint32_t p;     /* esi in the Assyria, Babylon and Egypt branches (unsigned compares) */

    rec = 0xF14CCu + 0xC7u * (uint32_t)n;                       /* 0x14C85 imul n, 0xC7; 0x14C8C add 0xF14CC */
    if (D32(0xF60E8u + 4u * (uint32_t)n) != 0) {                /* 0x14C99 cmp dword [4n + 0xF60E8], 0; je 0x14CDD */
        bpm_copy(dst, 0xF6064u + 13u * (uint32_t)n);            /* 0x14CA3..0x14CD5 the override name */
        return;                                                 /* 0x14CD8 jmp 0x15FD7 */
    }
    switch ((uint32_t)(n - 1)) {                                /* 0x14CDD lea eax,[edi-1]; cmp eax,8; ja 0x15FD7;
                                                                   table 0x14C48 */
    case 0:                                                     /* 1 AFRICA: 0x14CF4 */
        if (S8(rec + 0x9A) == 3) {                              /* 0x14CF4 dword [rec+0x97] sar 24; cmp 3; jne 0x14E44 */
            switch ((uint32_t)(S32(0xF40F0u) - 3)) {            /* 0x14D09 lea eax,[edi-3]; cmp eax,5; ja; table 0x14B38 */
            case 0: bpm_copy(dst, 0xB1010u); return;            /* 0x14D23 "AFRC30.bpm" */
            case 1:                                             /* 0x14D4D */
                if (D8(0xF17BBu) != 0 && D8(0xF1882u) != 0 && D8(0xF1949u) != 0)  /* 0x14D4D, 0x14D56, 0x14D5F je 0x14D72 */
                    bpm_copy(dst, 0xB101Bu);                    /* 0x14D6B "AFRC40.bpm" */
                else
                    bpm_copy(dst, 0xB1026u);                    /* 0x14D75 "AFRC41.bpm" */
                return;
            case 2: bpm_copy(dst, 0xB1031u); return;            /* 0x14D9C "AFRC50.bpm" */
            case 3: bpm_copy(dst, 0xB103Cu); return;            /* 0x14DC6 "AFRC60.bpm" */
            case 4: bpm_copy(dst, 0xB1047u); return;            /* 0x14DF0 "AFRC70.bpm" */
            case 5: bpm_copy(dst, 0xB1052u); return;            /* 0x14E1A "AFRC80.bpm" */
            default: return;                                    /* 0x15FD7 */
            }
        } else {
            switch ((uint32_t)(S32(0xF40F0u) - 3)) {            /* 0x14E44 table 0x14B50 */
            case 0: bpm_copy(dst, 0xB105Du); return;            /* 0x14E5E "AFRC30.bpm" */
            case 1: bpm_copy(dst, 0xB1068u); return;            /* 0x14E88 "AFRC41.bpm" */
            case 2: bpm_copy(dst, 0xB1073u); return;            /* 0x14EB2 "AFRC50.bpm" */
            case 3: bpm_copy(dst, 0xB107Eu); return;            /* 0x14EDC "AFRC60.bpm" */
            case 4: bpm_copy(dst, 0xB1089u); return;            /* 0x14F06 "AFRC70.bpm" */
            case 5: bpm_copy(dst, 0xB1094u); return;            /* 0x14F30 "AFRC80.bpm" */
            default: return;                                    /* 0x15FD7 */
            }
        }
    case 1:                                                     /* 2 IBERIA: 0x14F5A */
        if (S8(rec + 0x9A) == 3) {                              /* 0x14F5A jne 0x150AC */
            switch ((uint32_t)(S32(0xF40F0u) - 3)) {            /* 0x14F6F table 0x14B68 */
            case 0:                                             /* 0x14F89 */
                if (D8(0xF1882u) != 0)                          /* 0x14F89 Thrace; je 0x14F9C */
                    bpm_copy(dst, 0xB109Fu);                    /* 0x14F95 "ITAL32.bpm" */
                else if (D8(0xF17BBu) != 0)                     /* 0x14F9C mov bh,[Illyria]; test bh,bh; je 0x14FB0 */
                    bpm_copy(dst, 0xB10AAu);                    /* 0x14FA9 "ITAL30.bpm" */
                else
                    bpm_copy(dst, 0xB10B5u);                    /* 0x14FB3 "ITAL33.bpm" */
                return;
            case 1: bpm_copy(dst, 0xB10C0u); return;            /* 0x14FDA "IBER40.bpm" */
            case 2: bpm_copy(dst, 0xB10CBu); return;            /* 0x15004 "IBER50.bpm" */
            case 3: bpm_copy(dst, 0xB10D6u); return;            /* 0x1502E "IBER61.bpm" */
            case 4: bpm_copy(dst, 0xB10E1u); return;            /* 0x15058 "IBER71.bpm" */
            case 5: bpm_copy(dst, 0xB10ECu); return;            /* 0x15082 "IBER81.bpm" */
            default: return;                                    /* 0x15FD7 */
            }
        } else {
            switch ((uint32_t)(S32(0xF40F0u) - 3)) {            /* 0x150AC table 0x14B80 */
            case 0:                                             /* 0x150C6 */
                if (D8(0xF1882u) != 0)                          /* 0x150C6 Thrace; je 0x150D9 */
                    bpm_copy(dst, 0xB10F7u);                    /* 0x150D2 "ITAL32.bpm" */
                else
                    bpm_copy(dst, 0xB1102u);                    /* 0x150DC "ITAL33.bpm" */
                return;
            case 1: bpm_copy(dst, 0xB110Du); return;            /* 0x15103 "IBER40.bpm" */
            case 2: bpm_copy(dst, 0xB1118u); return;            /* 0x1512D "IBER50.bpm" */
            case 3: bpm_copy(dst, 0xB1123u); return;            /* 0x15157 "IBER61.bpm" */
            case 4: bpm_copy(dst, 0xB112Eu); return;            /* 0x15181 "IBER71.bpm" */
            case 5: bpm_copy(dst, 0xB1139u); return;            /* 0x151AB "IBER81.bpm" */
            default: return;                                    /* 0x15FD7 */
            }
        }
    case 2:                                                     /* 3 ILLYRIA: 0x151D5 */
        if (S8(rec + 0x9A) == 3) {                              /* 0x151D5 jne 0x15398 */
            switch ((uint32_t)(S32(0xF40F0u) - 3)) {            /* 0x151EA table 0x14B98 */
            case 0:                                             /* 0x15204 */
                if (D8(0xF1882u) != 0)                          /* 0x15204 Thrace; je 0x15217 */
                    bpm_copy(dst, 0xB1144u);                    /* 0x15210 "ILLY33.bpm" */
                else if (D8(0xF16F4u) != 0)                     /* 0x15217 mov al,[Iberia]; test al,al; je 0x1522A */
                    bpm_copy(dst, 0xB114Fu);                    /* 0x15223 "ILLY32.bpm" */
                else
                    bpm_copy(dst, 0xB115Au);                    /* 0x1522D "ILLY31.bpm" */
                return;
            case 1:                                             /* 0x15254 */
                /* BUG(1995): 0x15254 tests Illyria's own byte 0xF17BB (D: always set, she is in play and human here)
                   with Thrace and Crete, the three addresses of Africa's test at 0x14D4D (G: copied from it without
                   swapping Illyria for Africa). ILLY40 is "non-iberia" like AFRC40, so the test that fits is Africa
                   (0xF1566 + 0xC7 = 0xF162D), Thrace, Crete. Fixed: Africa; --original: Illyria (a 4-human game of
                   Iberia, Illyria, Thrace, Crete then gives the human Illyria ILLY40 instead of ILLY41). */
                if ((g_original ? D8(0xF17BBu) : D8(0xF162Du)) != 0     /* 0x15254 */
                    && D8(0xF1882u) != 0 && D8(0xF1949u) != 0)  /* 0x1525D, 0x15266 je 0x15279 */
                    bpm_copy(dst, 0xB1165u);                    /* 0x15272 "ILLY40.bpm" */
                else if (D8(0xF1882u) != 0)                     /* 0x15279 mov bl,[Thrace]; test bl,bl; je 0x1528D */
                    bpm_copy(dst, 0xB1170u);                    /* 0x15286 "ILLY41.bpm" */
                else
                    bpm_copy(dst, 0xB117Bu);                    /* 0x15290 "ILLY42.bpm" */
                return;
            case 2:                                             /* 0x152B7 */
                if (D8(0xF1882u) != 0)                          /* 0x152B7 Thrace; je 0x152CA */
                    bpm_copy(dst, 0xB1186u);                    /* 0x152C3 "ILLY50.bpm" */
                else
                    bpm_copy(dst, 0xB1191u);                    /* 0x152CD "ILLY51.bpm" */
                return;
            case 3:                                             /* 0x152F4 */
                if (D8(0xF1882u) != 0)                          /* 0x152F4 Thrace; je 0x15307 */
                    bpm_copy(dst, 0xB119Cu);                    /* 0x15300 "ILLY61.bpm" */
                else
                    bpm_copy(dst, 0xB11A7u);                    /* 0x1530A "ILLY62.bpm" */
                return;
            case 4:                                             /* 0x15331 */
                if (D8(0xF1882u) != 0)                          /* 0x15331 Thrace; je 0x15344 */
                    bpm_copy(dst, 0xB11B2u);                    /* 0x1533D "ILLY70.bpm" */
                else
                    bpm_copy(dst, 0xB11BDu);                    /* 0x15347 "ILLY71.bpm" */
                return;
            case 5: bpm_copy(dst, 0xB11C8u); return;            /* 0x1536E "ILLY80.bpm" */
            default: return;                                    /* 0x15FD7 */
            }
        } else {
            switch ((uint32_t)(S32(0xF40F0u) - 3)) {            /* 0x15398 table 0x14BB0 */
            case 0:                                             /* 0x153B2 */
                if (D8(0xF1882u) != 0)                          /* 0x153B2 Thrace; je 0x153C5 */
                    bpm_copy(dst, 0xB11D3u);                    /* 0x153BE "ILLY33.bpm" */
                else
                    bpm_copy(dst, 0xB11DEu);                    /* 0x153C8 "ILLY31.bpm" */
                return;
            case 1:                                             /* 0x153EF */
                if (D8(0xF1882u) != 0)                          /* 0x153EF Thrace; je 0x15402 */
                    bpm_copy(dst, 0xB11E9u);                    /* 0x153FB "ILLY41.bpm" */
                else
                    bpm_copy(dst, 0xB11F4u);                    /* 0x15405 "ILLY42.bpm" */
                return;
            case 2:                                             /* 0x1542C */
                if (D8(0xF1882u) != 0)                          /* 0x1542C Thrace; je 0x1543F */
                    bpm_copy(dst, 0xB11FFu);                    /* 0x15438 "ILLY50.bpm" */
                else
                    bpm_copy(dst, 0xB120Au);                    /* 0x15442 "ILLY51.bpm" */
                return;
            case 3:                                             /* 0x15469 */
                if (D8(0xF1882u) != 0)                          /* 0x15469 Thrace; je 0x1547C */
                    bpm_copy(dst, 0xB1215u);                    /* 0x15475 "ILLY61.bpm" */
                else
                    bpm_copy(dst, 0xB1220u);                    /* 0x1547F "ILLY62.bpm" */
                return;
            case 4:                                             /* 0x154A6 */
                if (D8(0xF1882u) != 0)                          /* 0x154A6 Thrace; je 0x154B9 */
                    bpm_copy(dst, 0xB122Bu);                    /* 0x154B2 "ILLY70.bpm" */
                else
                    bpm_copy(dst, 0xB1236u);                    /* 0x154BC "ILLY71.bpm" */
                return;
            case 5: bpm_copy(dst, 0xB1241u); return;            /* 0x154E3 "ILLY80.bpm" */
            default: return;                                    /* 0x15FD7 */
            }
        }
    case 3:                                                     /* 4 THRACE: 0x1550D */
        if (S8(rec + 0x9A) == 3) {                              /* 0x1550D jne 0x15684 */
            switch ((uint32_t)(S32(0xF40F0u) - 3)) {            /* 0x15522 table 0x14BC8 */
            case 0: bpm_copy(dst, 0xB124Cu); return;            /* 0x1553C "THRC31.bpm" */
            case 1:                                             /* 0x15566 */
                if (D8(0xF17BBu) != 0)                          /* 0x15566 Illyria; je 0x15579 */
                    bpm_copy(dst, 0xB1257u);                    /* 0x15572 "THRC41.bpm" */
                else
                    bpm_copy(dst, 0xB1262u);                    /* 0x1557C "THRC40.bpm" */
                return;
            case 2:                                             /* 0x155A3 */
                if (D8(0xF17BBu) != 0)                          /* 0x155A3 Illyria; je 0x155B6 */
                    bpm_copy(dst, 0xB126Du);                    /* 0x155AF "THRC51.bpm" */
                else
                    bpm_copy(dst, 0xB1278u);                    /* 0x155B9 "THRC50.bpm" */
                return;
            case 3:                                             /* 0x155E0 */
                if (D8(0xF17BBu) != 0)                          /* 0x155E0 Illyria; je 0x155F3 */
                    bpm_copy(dst, 0xB1283u);                    /* 0x155EC "THRC60.bpm" */
                else
                    bpm_copy(dst, 0xB128Eu);                    /* 0x155F6 "THRC61.bpm" */
                return;
            case 4:                                             /* 0x1561D */
                if (D8(0xF17BBu) != 0)                          /* 0x1561D Illyria; je 0x15630 */
                    bpm_copy(dst, 0xB1299u);                    /* 0x15629 "THRC70.bpm" */
                else
                    bpm_copy(dst, 0xB12A4u);                    /* 0x15633 "THRC71.bpm" */
                return;
            case 5: bpm_copy(dst, 0xB12AFu); return;            /* 0x1565A "THRC80.bpm" */
            default: return;                                    /* 0x15FD7 */
            }
        } else {
            switch ((uint32_t)(S32(0xF40F0u) - 3)) {            /* 0x15684 table 0x14BE0 */
            case 0: bpm_copy(dst, 0xB12BAu); return;            /* 0x1569E "THRC31.bpm" */
            case 1:                                             /* 0x156C8 */
                if (D8(0xF17BBu) != 0)                          /* 0x156C8 Illyria; je 0x156DB */
                    bpm_copy(dst, 0xB12C5u);                    /* 0x156D4 "THRC41.bpm" */
                else
                    bpm_copy(dst, 0xB12D0u);                    /* 0x156DE "THRC40.bpm" */
                return;
            case 2:                                             /* 0x15705 */
                if (D8(0xF17BBu) != 0)                          /* 0x15705 Illyria; je 0x15718 */
                    bpm_copy(dst, 0xB12DBu);                    /* 0x15711 "THRC51.bpm" */
                else
                    bpm_copy(dst, 0xB12E6u);                    /* 0x1571B "THRC50.bpm" */
                return;
            case 3:                                             /* 0x15742 */
                if (D8(0xF17BBu) != 0)                          /* 0x15742 Illyria; je 0x15755 */
                    bpm_copy(dst, 0xB12F1u);                    /* 0x1574E "THRC60.bpm" */
                else
                    bpm_copy(dst, 0xB12FCu);                    /* 0x15758 "THRC61.bpm" */
                return;
            case 4:                                             /* 0x1577F */
                if (D8(0xF17BBu) != 0)                          /* 0x1577F Illyria; je 0x15792 */
                    bpm_copy(dst, 0xB1307u);                    /* 0x1578B "THRC70.bpm" */
                else
                    bpm_copy(dst, 0xB1312u);                    /* 0x15795 "THRC71.bpm" */
                return;
            case 5: bpm_copy(dst, 0xB131Du); return;            /* 0x157BC "THRC80.bpm" */
            default: return;                                    /* 0x15FD7 */
            }
        }
    case 4:                                                     /* 5 CRETE: 0x157E6 */
        if (S8(rec + 0x9A) == 3) {                              /* 0x157E6 jne 0x15911 */
            switch ((uint32_t)(S32(0xF40F0u) - 3)) {            /* 0x157FB table 0x14BF8 */
            case 0: bpm_copy(dst, 0xB1328u); return;            /* 0x15815 "CRET30.bpm" */
            case 1: bpm_copy(dst, 0xB1333u); return;            /* 0x1583F "CRET40.bpm" */
            case 2: bpm_copy(dst, 0xB133Eu); return;            /* 0x15869 "CRET50.bpm" */
            case 3: bpm_copy(dst, 0xB1349u); return;            /* 0x15893 "CRET60.bpm" */
            case 4: bpm_copy(dst, 0xB1354u); return;            /* 0x158BD "CRET70.bpm" */
            case 5: bpm_copy(dst, 0xB135Fu); return;            /* 0x158E7 "CRET80.bpm" */
            default: return;                                    /* 0x15FD7 */
            }
        } else {
            switch ((uint32_t)(S32(0xF40F0u) - 3)) {            /* 0x15911 table 0x14C10 */
            case 0: bpm_copy(dst, 0xB136Au); return;            /* 0x1592B "CRET30.bpm" */
            case 1: bpm_copy(dst, 0xB1375u); return;            /* 0x15955 "CRET40.bpm" */
            case 2: bpm_copy(dst, 0xB1380u); return;            /* 0x1597F "CRET50.bpm" */
            case 3: bpm_copy(dst, 0xB138Bu); return;            /* 0x159A9 "CRET60.bpm" */
            case 4: bpm_copy(dst, 0xB1396u); return;            /* 0x159D3 "CRET70.bpm" */
            case 5: bpm_copy(dst, 0xB13A1u); return;            /* 0x159FD "CRET80.bpm" */
            default: return;                                    /* 0x15FD7 */
            }
        }
    case 5:                                                     /* 6 ASIA: 0x15A27 */
        if (S8(rec + 0x9A) == 3) {                              /* 0x15A27 jne 0x15AFE */
            switch ((uint32_t)(S32(0xF40F0u) - 5)) {            /* 0x15A3C lea eax,[edi-5]; cmp eax,3; ja; table 0x14C28 */
            case 0: bpm_copy(dst, 0xB13ACu); return;            /* 0x15A56 "ASIA50.bpm" */
            case 1: bpm_copy(dst, 0xB13B7u); return;            /* 0x15A80 "ASIA60.bpm" */
            case 2: bpm_copy(dst, 0xB13C2u); return;            /* 0x15AAA "ASIA70.bpm" */
            case 3: bpm_copy(dst, 0xB13CDu); return;            /* 0x15AD4 "ASIA80.bpm" */
            default: return;                                    /* 0x15FD7 */
            }
        } else {
            switch ((uint32_t)(S32(0xF40F0u) - 5)) {            /* 0x15AFE table 0x14C38 */
            case 0: bpm_copy(dst, 0xB13D8u); return;            /* 0x15B18 "ASIA50.bpm" */
            case 1: bpm_copy(dst, 0xB13E3u); return;            /* 0x15B42 "ASIA60.bpm" */
            case 2: bpm_copy(dst, 0xB13EEu); return;            /* 0x15B6C "ASIA70.bpm" */
            case 3: bpm_copy(dst, 0xB13F9u); return;            /* 0x15B96 "ASIA80.bpm" */
            default: return;                                    /* 0x15FD7 */
            }
        }
    case 6:                                                     /* 7 ASSYRIA: 0x15BC0 */
        if (S8(rec + 0x9A) == 3) {                              /* 0x15BC0 jne 0x15CA5 */
            p = D32(0xF40F0u);                                  /* 0x15BD5 mov esi,[0xF40F0] */
            if (p < 7) {                                        /* 0x15BDB cmp esi,7; jb 0x15C94 */
                if (p == 6)                                     /* 0x15C94 cmp esi,6; je 0x15BF0 */
                    bpm_copy(dst, 0xB1404u);                    /* 0x15BF3 "ASSY60.bpm" */
                return;                                         /* 0x15C9D */
            } else if (p == 7) {                                /* 0x15BE4 jbe 0x15C1A */
                if (D8(0xF1B9Eu) != 0)                          /* 0x15C1A Babylon; je 0x15C2D */
                    bpm_copy(dst, 0xB140Fu);                    /* 0x15C26 "ASSY70.bpm" */
                else
                    bpm_copy(dst, 0xB141Au);                    /* 0x15C30 "ASSY71.bpm" */
                return;
            } else if (p == 8) {                                /* 0x15BE6 cmp esi,8; je 0x15C57 */
                if (D8(0xF1B9Eu) != 0)                          /* 0x15C57 Babylon; je 0x15C6A */
                    bpm_copy(dst, 0xB1425u);                    /* 0x15C63 "ASSY80.bpm" */
                else
                    bpm_copy(dst, 0xB1430u);                    /* 0x15C6D "ASSY81.bpm" */
                return;
            }
            return;                                             /* 0x15BEB jmp 0x15C8C */
        } else {
            p = D32(0xF40F0u);                                  /* 0x15CA5 */
            if (p < 7) {                                        /* 0x15CAE jb 0x15D64 */
                if (p == 6)                                     /* 0x15D64 cmp esi,6; je 0x15CC0 */
                    bpm_copy(dst, 0xB143Bu);                    /* 0x15CC3 "ASSY60.bpm" */
                return;                                         /* 0x15D6D */
            } else if (p == 7) {                                /* 0x15CB4 jbe 0x15CEA */
                if (D8(0xF1B9Eu) != 0)                          /* 0x15CEA Babylon; je 0x15CFD */
                    bpm_copy(dst, 0xB1446u);                    /* 0x15CF6 "ASSY70.bpm" */
                else
                    bpm_copy(dst, 0xB1451u);                    /* 0x15D00 "ASSY71.bpm" */
                return;
            } else if (p == 8) {                                /* 0x15CB9 je 0x15D27 */
                if (D8(0xF1B9Eu) != 0)                          /* 0x15D27 Babylon; je 0x15D3A */
                    bpm_copy(dst, 0xB145Cu);                    /* 0x15D33 "ASSY80.bpm" */
                else
                    bpm_copy(dst, 0xB1467u);                    /* 0x15D3D "ASSY81.bpm" */
                return;
            }
            return;                                             /* 0x15CBB jmp 0x15D5C */
        }
    case 7:                                                     /* 8 BABYLON: 0x15D75 */
        if (S8(rec + 0x9A) == 3) {                              /* 0x15D75 jne 0x15DFC */
            p = D32(0xF40F0u);                                  /* 0x15D8A */
            if (p < 7)                                          /* 0x15D93 jb 0x15FD7 */
                return;
            else if (p == 7)                                    /* 0x15D99 jbe 0x15DA8 */
                bpm_copy(dst, 0xB1472u);                        /* 0x15DAB "BABL70.bpm" */
            else if (p == 8)                                    /* 0x15D9E je 0x15DD2 */
                bpm_copy(dst, 0xB147Du);                        /* 0x15DD5 "BABL80.bpm" */
            return;                                             /* 0x15DA0 */
        } else {
            p = D32(0xF40F0u);                                  /* 0x15DFC */
            if (p < 7)                                          /* 0x15E05 jb 0x15FD7 */
                return;
            else if (p == 7)                                    /* 0x15E0B jbe 0x15E1A */
                bpm_copy(dst, 0xB1488u);                        /* 0x15E1D "BABL70.bpm" */
            else if (p == 8)                                    /* 0x15E10 je 0x15E44 */
                bpm_copy(dst, 0xB1493u);                        /* 0x15E47 "BABL80.bpm" */
            return;                                             /* 0x15E12 */
        }
    case 8:                                                     /* 9 EGYPT: 0x15E6E */
        if (S8(rec + 0x9A) == 3) {                              /* 0x15E6E jne 0x15F30 */
            p = D32(0xF40F0u);                                  /* 0x15E83 */
            if (p < 7) {                                        /* 0x15E8C jb 0x15F1F */
                if (p == 6)                                     /* 0x15F1F cmp esi,6; je 0x15EA1 */
                    bpm_copy(dst, 0xB149Eu);                    /* 0x15EA4 "EGYP60.bpm" */
                return;                                         /* 0x15F28 */
            } else if (p == 7) {                                /* 0x15E92 jbe 0x15ECB */
                bpm_copy(dst, 0xB14A9u);                        /* 0x15ECE "EGYP70.bpm" */
            } else if (p == 8) {                                /* 0x15E97 je 0x15EF5 */
                bpm_copy(dst, 0xB14B4u);                        /* 0x15EF8 "EGYP80.bpm" */
            }
            return;                                             /* 0x15E99 */
        } else {
            p = D32(0xF40F0u);                                  /* 0x15F30 */
            if (p < 7) {                                        /* 0x15F39 jb 0x15FC6 */
                if (p == 6)                                     /* 0x15FC6 cmp esi,6; je 0x15F48 */
                    bpm_copy(dst, 0xB14BFu);                    /* 0x15F4B "EGYP60.bpm" */
                return;                                         /* 0x15FCF */
            } else if (p == 7) {                                /* 0x15F3F jbe 0x15F72 */
                bpm_copy(dst, 0xB14CAu);                        /* 0x15F75 "EGYP70.bpm" */
            } else if (p == 8) {                                /* 0x15F44 je 0x15F9C */
                bpm_copy(dst, 0xB14D5u);                        /* 0x15F9F "EGYP80.bpm" */
            }
            return;                                             /* 0x15F46 jmp 0x15FBE */
        }
    default:
        return;                                                 /* 0x15FD7 */
    }
}
