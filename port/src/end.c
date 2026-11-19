/* Module END of docs/M6_PLAN.md: the end of the game and the credits. Literal translations; the address of each
   instruction a line comes from is in its comment. D = read in the code, G = inference. Prototypes and globals: end.h.

     0x14294  f14294  Final Civilization Standings (and the /C continuous test's log)        1,328 bytes
     0x1330C  f1330C  the scrolling credits                                                     932
     0x3CDF0  f3CDF0  palette to black (the target saved)                                        84
     0x3CE44  f3CE44  palette from a GIF's table at once                                        116
     0x39F50  f39F50  font release                                                               64
   0x7762C (thousands commas, the /C log) is formulas.c's f7762C.

   Nothing of another module of this batch, of the computers (M7) or of ANIM is called here, so there are no
   not_ported stubs. Play by e-mail: no PBEM routine is called here (0x39F50's PBEM callers 0x5055C / 0x50D08 are
   dropped by Erik's ruling, not this module's).

   Library calls:
     0x8E0F8 fgf_release(font) (Fastgraph/Fonts; D: frees the font's DOS block with int 21h AH=49h and clears its slot,
             -1 all slots, the current font number [0xE0A74] cleared when it is released) -> fgf_release_8E0F8 below, a
             no-op: fg.c has no release, and the port's fonts stay defined for the life of the process (fgfont.c keeps
             pointers into the catalog data). G: main's only call is after the game loop, which never returns
             (loop.h), so nothing draws text after a release in either program.
     0x85BBF AIL_set_sequence_loop_count -> port no-op, its argument [0xF6284] read (menu2.c's convention).
     0x7D8C8 fopen / 0x878D6 fprintf / 0x7D95E fclose -> stdio, "ctest.txt" relative to the working directory as in the
             original. The FILE * the original keeps in [0xF52D8] stays in the port (a host pointer does not fit the
             32-bit field; title.c does the same for aciv.cfg): [0xF52D8] is not written (by design). Port guard: a
             failed fopen writes nothing (the original would write through a NULL FILE *).
     0x8A080 clock: the credits busy-wait until clock() > start + (40 / [0xBB3F4]) / 10 hundredths (0x135AE, 0x13628),
             with start taken just before each paste. The port's clock (ui_kit2.c's clock_8A080, static) advances only
             in f78A20's waits, and drawing takes no clock time, so each wait is f78A20(40): the same formula and the
             same number of DOS clock steps between pastes (file comment of ui_kit2.c). The first paste of a window
             waits from the previous window's last paste (which has no wait after it), as in the original.
     0x7D5D0 memset, rep movsd/movsb -> memset / memcpy on the data object; 0x7DB3D is not used here.
   __CHK dropped. game_rand: none (D).

   Data (D, read in the code at the addresses given):
     nation record n: 0xF14CC + 199n, +0x9A 0 out / 2 computer / 3 human (byte [199n + 0xF1566]); scores f147C4
     token sprites 0xF4A5C + 8n (35 x 35); names 0xC768C[n]
     the sort's array {int16 score, int16 nation} at [ebp-0x50] holds 8 entries in the original (the next local,
       [ebp-0x30], is the sort's inner index): a 9th nation in play would overwrite it. Unreachable: at most 8 players
       (cards.c, setup); the port's array has 9 entries.

   1995 bugs, fixed unless g_original (each marked BUG(1995) in the code; grades code / effect):
     1. 0x14294 (D / G, reachable): the shared CONTINUE button 0x2A of dialog 0xBB800 is relabelled "EXIT" and never
        given its label back (no other code relabels 0xBB800, D: the only f3C284 call on it is 0x14588). After a
        conquest (f22F3C: Final Standings, credits, then longjmp to the title menu) a new game in the same session
        shows "EXIT" on every CONTINUE of 0xBB800 (f79270's -500 messages, the elimination screens, the AST chart's
        modes 1 / 2). Fixed: the label is put back after the EXIT wait. The position is not restored: every user of
        0xBB800 sets x and y itself (kit.c, loop.c, ast.c). (M6_PLAN egg 17.)
   Kept as the original (D code, G effect unless marked):
     - 0x14294 draws a row for each r < [0xF40EC] while the array holds the nations in play (+0x9A != 0): more rows
       than nations would read uninitialised stack. G: unreachable, the elimination check lowers [0xF40EC] as it
       clears +0x9A (loop.c 0x22C26). Port guard (both modes): the rows stop at the nations in play.
     - The /C log's "Max Memory used including .map analysis" adds 0x100934 to [0xF6060] (D: a constant, G: the size
       of the map analysis tables). The memory figures and the stack marks are the port's (by design).
     - The credits draw the 11 lines of a window but scroll only 30 of their 330 rows into the 300-row window, so each
       window shows the next line rising from the bottom; the last window starts at record 52 (k < 63 - 10).
   Text: none changed (Erik's ruling of 19 Nov). "Final Civilization Standings", "EXIT" and the /C log lines are as
   the original has them.

   Easter eggs / oddities (OVERVIEW §6.9, M6_PLAN egg 5; new ones marked NEW):
     - The credits (63 records, 0xC7EFC): Producer Don Greenwood; Executive Producer Bill Levay; Program and AI Design
       Jim Synoski; "AI Design Nemesis" Don Greenwood; Game Programming Jim Synoski; Ancillary Programming Norv Brooks;
       Computer Graphics Joe Amoral; Animations Joe Amoral, Trish Wilson, Joseph B. Pape; Music Eric Heberling; Sound
       Effects Bill Levay; Testing Coordination Ken Robinson; Manual Don Greenwood; "Original Boardgame Design (Thank
       You!)" Francis Tresham, Bruce Harper.
     - NEW: the executive producer made the sound effects himself (Bill Levay credited twice), and Don Greenwood is
       credited three times: as producer, as the AI's nemesis and for the manual.
     - The title row "A D V A N C E D   C I V I L I Z A T I O N ~" ends in a tilde, drawn in the title style (f3A18C,
       colour 0x45, font 2) (egg 5). NEW (D: COGNAC19.FGF in ACIV.CT1, glyph '~' 10 x 8): font 2's tilde slot holds
       a circled R, so the row reads "ADVANCED CIVILIZATION" with the registered-trademark sign; BLOCK05 and COGNAC10
       have a plain tilde.
     - NEW: the roll begins with ten empty records and ends with ten, so the title rises from an empty window and the
       last names scroll up into an empty one; a key ends it at any line.
     - The /C continuous test (OVERVIEW §6.1, egg 13): "End of game %d.", the developers' memory statistics, "Max
       WorldStack / Max MoveStack / Max RouteList", "RoutingMax", then a new game by itself (START NEW GAME 0x51).
     - The EXIT label that sticks (egg 17), fixed above. */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "end.h"
#include "fg.h"
#include "formulas.h"
#include "game.h"
#include "state.h"
#include "ui.h"

#define NATION(n)    (0xF14CCu + 199u * (uint32_t)(n))
#define NAME(n)      DSTR(D32(0xC768Cu + 4u * (uint32_t)(n)))
#define BACKBUF      0xF52C8u
#define BACKGROUND   0xF52CCu
#define BGNOW        0xF52D0u   /* int8: the background in [0xF52CC] */
#define CONT_DLG     0xBB800u   /* CONTINUE, id 0x2A */
#define PLAYERS      0xF40ECu   /* int32 */
#define CTEST        0xF628Cu   /* int32: the /C continuous test */
#define NUMTEXT      0xF5500u   /* the number text of f7762C */
#define CREDITS      0xC7EFCu   /* 5-byte records {int8 type, char *text} */
#define CREDITS_N    0xC8038u   /* int32 */
#define PAL_TARGET   0xF8B40u   /* uint8[768] */
#define PAL_WORK     0xF5D58u   /* uint8[768] */
#define KEY          0xE1634u
#define KEY_AUX      0xE1635u

/* ---- library calls with no port counterpart (file comment) --------------------------------------------------------- */

#ifdef END_TEST
void end_test_fgf_release(int font);
void end_test_ail_loop_count(int32_t seq, int count);
#define fgf_release_8E0F8(font)               end_test_fgf_release(font)
#define AIL_set_sequence_loop_count(s, c)     end_test_ail_loop_count(s, c)
#else
static void fgf_release_8E0F8(int font)        /* 0x8E0F8 Fastgraph/Fonts release: the port's fonts stay */
{
    (void)font;
}

static void AIL_set_sequence_loop_count(int32_t seq, int count)   /* 0x85BBF AIL (port no-op) */
{
    (void)seq; (void)count;
}
#endif

/* ==== 0x14294 Final Civilization Standings ============================================================================ */

void f14294(void)
{
    struct {
        int16_t score;   /* [ebp-0x50 + 4r] */
        int16_t nation;  /* [ebp-0x4E + 4r] */
    } e[9], tmp;         /* 8 entries in the original (file comment) */
    int32_t cnt;         /* [ebp-0x1C] */
    int32_t n;           /* [ebp-0x2C] (the nations; the sort's i; the rows' r) */
    int32_t j;           /* [ebp-0x30] */
    int32_t row;         /* [ebp-0x28] */
    int32_t w;           /* [ebp-0x20] */
    int32_t r;           /* [ebp-0x24] */
    int32_t x;
    int32_t y;
    uint32_t label;
    FILE *fp;            /* [0xF52D8] in the original (file comment) */

    cnt = 0;                                                        /* 0x142AB */
    for (n = 1; n <= 9; n++) {                                      /* 0x142AE, 0x142BD cmp 9; jg */
        if (D8(NATION(n) + 0x9Au) == 0)                             /* 0x142CA byte [199n + 0xF1566]; je */
            continue;
        e[cnt].score = (int16_t)f147C4(n);                          /* 0x142D7, 0x142DF mov word, ax */
        e[cnt].nation = (int16_t)n;                                 /* 0x142E7 cnt + 1, 0x142ED mov word, ax */
        cnt++;
    }
    /* 0x142F4..0x14362: bubble sort, descending; a strictly greater score moves up (16-bit signed, cmp dx, bx; jle) */
    for (n = cnt - 2; n >= 0; n--) {                                /* 0x142F7, 0x142FD jl / 0x1430D dec */
        for (j = 0; j <= n; j++) {                                  /* 0x14305, 0x14326 cmp; jg */
            if (e[j + 1].score > e[j].score) {                      /* 0x14331 [ebp + 4j - 0x4C], 0x14336 [.. - 0x50] */
                tmp = e[j + 1];                                     /* 0x14343 the whole dword */
                e[j + 1] = e[j];                                    /* 0x14354 */
                e[j] = tmp;                                         /* 0x1435E */
            }
        }
    }

    if (S32(CTEST) != 0)                                            /* 0x14364 cmp dword [0xF628C], 0; jne 0x1465C */
        goto ctest;

    f3CD6C(0x17);                                                   /* 0x14376 */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BACKGROUND), S32(BACKBUF));   /* 0x1439E */
    fg_vbopen(S32(BACKBUF));                                        /* 0x143AD */
    f3A18C(2, 6, 0x280, 0xB0, DSTR(0xB0EC1u));                      /* 0x143CE "Final Civilization Standings" */
    fg_setcolor(5);                                                 /* 0x143D5 */
    f39F90(2);                                                      /* 0x143E4 */
    row = 0;                                                        /* 0x143E9 */
    for (n = 0; n < S32(PLAYERS); n++) {                            /* 0x143EC, 0x143FA cmp [0xF40EC]; jge */
        if (n >= cnt)   /* port guard (both modes): the original reads uninitialised entries (file comment) */
            break;
        y = 49 * row;                                               /* 0x14409..0x14415 (3 * 16 + 1) */
        f3AF8C(0xF4A5Cu + 8u * (uint32_t)(int32_t)e[n].nation, 0xDB, y + 0x2A);    /* 0x14431 */
        f3AF8C(0xF4A5Cu + 8u * (uint32_t)(int32_t)e[n].nation, 0x183, y + 0x2A);   /* 0x14461 */
        f3A0B0(0, y + 0x28, 0x280, NAME(e[n].nation));              /* 0x14492 */
        w = fgf_width(NAME(e[n].nation), (int)strlen(NAME(e[n].nation)));          /* 0x144A8 repne scasb, 0x144C5 */
        x = (0x280 - w) / 2 - 2;                                    /* 0x144E9..0x144F6 signed /2 (sar 31, sub, sar) */
        fg_move(x, y + 0x3C);                                       /* 0x144FA */
        fg_draw(x + w + 2, y + 0x3C);                               /* 0x14532 */
        f3A148(0, y + 0x3F, 0x280, e[n].score);                     /* 0x1455F dword [ebp + 4n - 0x52] sar 16 */
        row++;                                                      /* 0x14567 */
    }
    f39F90(1);                                                      /* 0x1457E */
    label = D32(CONT_DLG + DLG_REC(0) + BTN_LABEL);   /* port: kept for the fix below (0xB0EAB "CONTINUE") */
    f3C284(CONT_DLG, 0x2A, 0xB0EDEu);                               /* 0x14588 "EXIT" (edx 0x2A, ebx kept across
                                                                       f39F90) */
    f3BB48(CONT_DLG, 0x2A, 0x109);                                  /* 0x1459C */
    y = 49 * row + 0x28;                                            /* 0x145A1..0x145B7 */
    f3BC64(CONT_DLG, 0x2A, y + (0x1DC - y - 0x13) / 2);             /* 0x145BA..0x145EA signed /2 */
    fg_vbclose();                                                   /* 0x145EF */
    f3B05C(CONT_DLG);                                               /* 0x145F9 */
    f3CC14();                                                       /* 0x145FE */
    f7B854(DSTR(0xCAB1Cu), 0);                                      /* 0x1460A music "gcivil2_.xmi " (the set's name) */
    if (D8(0xF625Du) != 0)                                          /* 0x1460F */
        AIL_set_sequence_loop_count(S32(0xF6284u), 1);              /* 0x14621 [AIL, port no-op] */
    do {
        r = f78BA4(CONT_DLG, 0);                                    /* 0x14630 */
    } while (r != -1);                                              /* 0x1463B je (-100), 0x14646 jne: back to 0x14629 */
    /* BUG(1995): the shared CONTINUE keeps the label "EXIT" for the rest of the session (file comment, 1) */
    if (!g_original)
        f3C284(CONT_DLG, 0x2A, label);
    f78884();                                                       /* 0x14648 */
    f1330C();                                                       /* 0x1464D the credits */
    return;

ctest:
    /* the /C continuous test: the statistics appended to ctest.txt, then a new game */
    fp = fopen(DSTR(0xB0EE6u), DSTR(0xB0EE3u));                     /* 0x14666 fopen("ctest.txt", "at") */
    if (fp != NULL) {   /* port guard (both modes): the original writes through a NULL FILE * */
        fprintf(fp, "End of game %d.\n", S32(0xBC8DCu));            /* 0x1467D (0xB0EF0) */
        f7762C(S32(0xF605Cu), (char *)DP(NUMTEXT));                 /* 0x1468F */
        fprintf(fp, "Memory available after game is loaded: %s\n", DSTR(NUMTEXT));   /* 0x146A5 (0xB0F01) */
        f7762C(S32(0xF6060u), (char *)DP(NUMTEXT));                 /* 0x146B7 */
        fprintf(fp, "Max memory dynamically allocated by arrays: %s\n", DSTR(NUMTEXT));   /* 0x146CC (0xB0F2C) */
        f7762C((int)(D32(0xF6060u) + 0x100934u), (char *)DP(NUMTEXT));   /* 0x146D9..0x146E3 add eax, 0x100934 */
        fprintf(fp, "Max Memory used including .map analysis: %s\n", DSTR(NUMTEXT));   /* 0x146F9 (0xB0F5C) */
        f7762C((int)(D32(0xF605Cu) - D32(0xF6060u)), (char *)DP(NUMTEXT));   /* 0x14711 sub eax, ebx; 0x14713 */
        fprintf(fp, "Memory Headroom: %s\n", DSTR(NUMTEXT));        /* 0x14729 (0xB0F89) */
        fprintf(fp, "Max WorldStack: %d   Max MoveStack: %d  Max RouteList: %d\n",
                S32(0x112A8Cu), S32(0x112A90u), S32(0x109E04u));    /* 0x14751 (0xB0F9E) */
        fprintf(fp, "RoutingMax: %d\n", S32(0x10818Cu));            /* 0x1476C (0xB0FD9) */
        fclose(fp);                                                 /* 0x14779 */
    }
    setD32(0xBC8DCu, D32(0xBC8DCu) + 1u);                           /* 0x1478F inc esi; 0x14791 */
    if (S32(0xBC8E0u) - 1 > S32(0xF6290u))                          /* 0x14790 dec eax; 0x14797 cmp eax, edi; jle */
        setD32(0xF6290u, (uint32_t)(S32(0xF6290u) + 1));            /* 0x1479B lea eax, [edi + 1] */
    else
        setD32(0xF6290u, 0);                                        /* 0x147A7 */
    (void)f3FB94(0x51);                                             /* 0x147B2 START NEW GAME (a longjmp; a tail
                                                                       position in the original) */
}

/* ==== 0x1330C the credits ============================================================================================= */

void f1330C(void)
{
    int32_t limit;       /* [ebp-0x20] */
    int32_t k;           /* [ebp-0x24] */
    int32_t first;       /* [ebp-0x2C]: 0 until the window's first paste */
    int32_t i;           /* [ebp-0x30] (the lines; then the pixel steps) */
    int32_t r;           /* [ebp-0x1C] */
    int32_t type;
    int32_t w;           /* [ebp-0x28] */
    int32_t x;
    const char *text;

    f3CDF0();                                                       /* 0x13321 */
    fg_vbopen(S32(BACKGROUND));                                     /* 0x1332D */
    fg_move(0, 0);                                                  /* 0x13339 */
    cat_show(DSTR(D32(0xC19D0u)), 3, S32(0xF40D8u));                /* 0x13351 "message.gif" from ACIV.CT2 */
    fg_vbclose();                                                   /* 0x13359 */
    setD8(BGNOW, 0x15);                                             /* 0x13383 (ah = 0x15, stored before the copy) */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BACKGROUND), S32(BACKBUF));   /* 0x13389 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x13397 */
    fg_vbpaste(0, 0x27F, 0, 0x1DF, 0, 0x1DF);                       /* 0x133B4 */
    fg_vbclose();                                                   /* 0x133BC */
    f3CE44(DSTR(0xB0E64u));                                         /* 0x133C6 "pal.pal" */
    limit = S32(CREDITS_N) - 10;                                    /* 0x133CB..0x133D3 */
    f39F90(2);                                                      /* 0x133DB */
    fg_setcolor(0xD);                                               /* 0x133E2 */
    /* 0x133EA start = clock(): the port's waits below (file comment) */
    for (k = 0; k < limit; k++) {                                   /* 0x133F4, 0x13402 cmp; jge 0x13697 */
        first = 0;                                                  /* 0x1340D */
        fg_vbcopy(0x50, 0x22F, 0x5A, 0x1A3, 0x50, 0x1A3, S32(BACKGROUND), S32(BACKBUF));   /* 0x13432 the strip */
        fg_vbclose();                                               /* 0x1343A */
        fg_vbopen(S32(BACKBUF));                                    /* 0x13446 */
        for (i = 0; i < 0xB; i++) {                                 /* 0x1344E, 0x13527 cmp 0xB; jge */
            r = i + k;                                              /* 0x13535 */
            type = S8(CREDITS + 5u * (uint32_t)r);                  /* 0x1353D dword [5r + 0xC7EF9] sar 24 */
            text = DSTR(D32(CREDITS + 1u + 5u * (uint32_t)r));      /* [5r + 0xC7EFD] */
            if (type == 2) {                                        /* 0x13553 jne 0x13456 */
                f3A18C(0x50, 30 * i + 0x5A, 0x1E0, 0x45, text);     /* 0x13581 the title */
                f39F90(2);                                          /* 0x1358B */
                fg_setcolor(0xD);                                   /* 0x13592 */
            } else {
                f3A0B0(0x50, 30 * i + 0x5A, 0x1E0, text);           /* 0x13475 */
            }
            if (S8(CREDITS + 5u * (uint32_t)r) == 1) {              /* 0x1347F dword [5r + 0xC7EF9] sar 24; 0x13489 */
                /* a heading: underlined 0x15 below its top, 2 pixels wider than the text on each side */
                w = fgf_width(text, (int)strlen(text));             /* 0x134A3 repne scasb, 0x134B6 */
                x = (0x1E0 - w) / 2 + 0x4E;                         /* 0x134D5..0x134E2 signed /2 */
                fg_move(x, 30 * i + 0x6F);                          /* 0x134E6 */
                fg_draw(x + w + 2, 30 * i + 0x6F);                  /* 0x13519 */
            }
        }
        fg_vbclose();                                               /* 0x1359F */
        for (i = 0; i < 0x1E; i++) {                                /* 0x135A6, 0x1365A cmp 0x1E; jge 0x133F9 */
            fg_intkey(DP(KEY), DP(KEY_AUX));                        /* 0x1366E */
            if (D8(KEY) != 0)                                       /* 0x1367C test dl, dl; jne 0x13697 */
                return;
            if (first == 0) {                                       /* 0x13680 je 0x135AE */
                f78A20(0x28);                                       /* 0x135AE..0x135D8 the wait from the last start */
                /* 0x135DA start = clock() */
                first = 1;                                          /* 0x135E2 */
            }
            /* else 0x1368A start = clock() */
            fg_vbopen(S32(BACKBUF));                                /* 0x135EF */
            fg_vbpaste(0x50, 0x22F, 0x5A + i, 0x185 + i, 0x50, 0x185);   /* 0x13615 one pixel further */
            fg_vbclose();                                           /* 0x1361D */
            if (i != 0x1D)                                          /* 0x13622 cmp 0x1D; je 0x13654 */
                f78A20(0x28);                                       /* 0x13628..0x13652 */
        }
    }
}

/* ==== 0x3CDF0 palette to black ======================================================================================== */

void f3CDF0(void)
{
    fg_getdacs(0, 0x100, DP(PAL_TARGET));                           /* 0x3CE0C */
    memset(DP(PAL_WORK), 0, 0x300);                                 /* 0x3CE20 */
    fg_setdacs(0, 0x100, DP(PAL_WORK));                             /* 0x3CE31 */
}

/* ==== 0x3CE44 palette from a GIF at once ============================================================================== */

void f3CE44(const char *name)
{
    if (strlen(name) != 0)                                          /* 0x3CE5C repne scasb; 0x3CE69 test ecx, ecx; je */
        fg_gifpal(name, DP(PAL_TARGET));                            /* 0x3CE76 (the result is not checked, D) */
    memcpy(DP(PAL_WORK), DP(PAL_TARGET), 0x300);                    /* 0x3CE93 rep movsd + movsb */
    fg_setdacs(0, 0x100, DP(PAL_WORK));                             /* 0x3CEA5 */
}

/* ==== 0x39F50 font release ============================================================================================ */

void f39F50(int i)
{
    if (i != -1)                                                    /* 0x39F69 cmp edx, -1; je */
        fgf_release_8E0F8(S16(UI_FONT_NUM + 2u * (uint32_t)i));     /* 0x39F71 dword [2i + 0xF8B2A] sar 16; 0x39F7F */
    else
        fgf_release_8E0F8(i);                                       /* 0x39F7E push edx (-1): every font */
}
