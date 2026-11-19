/* Module POP of docs/M6_PLAN.md: the POPULATION EXPANSION phase 0x673A8 and its human screen ManualPopExpansion
   0x67758. Literal translations; the address of each instruction a line comes from is in its comment. D = read in the
   code, G = inference. Prototypes: pop.h.

   Not ported yet (the call site stops with not_ported(address); the statements after it are kept as in the
   original, so the call can be replaced when the module lands):
     0x680DC  the computer nation's choice when its stock is short (module AI, M7: needs AIMOVE 0x461BC and CONFLICT
              0x5AAA4), called from 0x67661 with eax = [0xF40F8], edx = need
     0x28CFC  the choice list of module CAL1, called by 0x67758 with (n, 3, 0) at 0x67A1F, 0x67F6D and 0x6807D. Kind 3
              (D, 0x28D18..0x28D42, 0x28F6E..0x28FC8): [0xF7C9C] = 0, int32 0xF7CA0[0..156] = -1, then every
              territory t 0..156 with byte [0xE31DC + 69(t + 1) + n] (area +0x24+n, the allowance) != 0 appended in
              ascending order. Territories 157..159 are never listed.

   Play by e-mail (dropped, Erik's ruling): the bit-0x80 tests of byte [0xE1624] are kept as in loop.c; neither
   routine calls PBEM code (0x673A8 only lets the PBEM player's own nation byte [0xE1622] expand and sets byte
   [0xF40FE] = 1).

   Watcom runtime -> standard C:
     0x7DB3D sprintf -> snprintf bounded to the stack buffers: 100 bytes [ebp-0x9C] in 0x673A8, 200 bytes [ebp-0xFC]
             in 0x67758 (the locals below them start at [ebp-0x38] / [ebp-0x34])
     0x8C897 itoa(value, buf, 10) -> snprintf "%d" (as ui_kit3.c's f3A148)
     0x822B5 malloc / 0x877E6 free -> load_heap_alloc / load_heap_free for the data fields [0xEED74], [0xEED78],
             [0xEED7C] (game.h); the copies go through load_heap_ptr. Port guard (G): a block that could not be
             allocated is skipped (the original would copy through NULL)
     0x8A080 clock: 0x673A8 takes start = clock() before its loop and at the end busy-waits until clock() reaches start
             + (1000 / [0xBB3F4]) / 10 (unsigned, 0x67722..0x6774C: an inline copy of f78A20(1000) whose start is the
             phase's start). The port's clock is ui_kit2.c's, which advances only inside f78A20, so the translation
             calls f78A20(1000) there (G, port design): the same wait when no message or screen came up (the
             automatic case); after one the original's wait is shorter or none, the port's a full second
     rep movsd / movsw / movsb -> memcpy; 0x7D4AC (__CHK) dropped; 0x78B0C ("ManualPopExpansion", a no-op debug log)
             left out
   game_rand f78AE4: neither routine draws.

   Data (D, read in the code at the addresses given; meanings G unless said):
     nation n at 0xF14CC + 199n: +0x60 int8 count of the area list, +0x61 + i uint8 area record index (territory + 1),
       +0x98 uint8 tokens on the board, +0x99 int8 tokens in stock, +0x9A int8 0 out / 2 computer / 3 human
     area record a at 0xE31B8 + 69a (a = territory + 1): +0x01 + n the marker (n when n has tokens there), +0x0E + n
       int8 n's tokens, +0x24 + n uint8 n's allowance (written here; the movement phase uses the same byte for the
       tokens that may still move, map.h's f3DAD8)
     0xF06AE int16 count and 0xF06B0 int16[] the player list (f783B8, f7844C and the dispatcher's player buttons read
       it); 0xF40F8 int32 the current nation; 0xF488C uint8[10] per nation (cleared here; 0x680DC sets 1 when the
       computer is short, read by 0x6DE04, G); 0xEE168 int8 the picture now in the background buffer (3 =
       egypt5.gif, the map screen's, f3CD6C(3)); 0xF5940 int32 the first record of the player buttons in 0xF596C
     message area int16 0xBB3FA x, 0xBB3FC y, 0xBB3FE w, 0xBB400 h: (0xA9, 0x154, 0x12C, 0x23) for the phase; h 0x15
       during 0x67758 (the one-line box of f3A9F4), 0x23 again at its end
     0xF7C9C int32 count, 0xF7CA0 int32[157] the choice list (0x28CFC kind 3: n's territories with an allowance)
     0xC8954 dialog (obj2, 7 records): 0x39 / 0x3A arrows (251 / 376, 390, 13 x 19, style 2), 0xDB DISTRIBUTE ALL
       (169, 410, 100 x 19), 0xDC ADD ONE (269, 410), 0xDD ADD MAX (369, 410), 0x41 RESTART (219, 430), 0x2A
       FINISHED (319, 430). Codes through f3FB94 (game.h): -9 / -0xA, -0x76, -0x77, -0x78, -0x63, -1
     0xC768C nation names, 0xC76C4 nation adjectives ("Restart %s Population Expansion?"), 0xC77D8 territory names
     0xF097C bit 0x08: ask before RESTART (aciv.cfg's confirmations toggle)

   Heap pointers in data fields: [0xEED74] (areas, 0x2B65 bytes), [0xEED7C] (territory states, 0x1E0), [0xEED78]
   (nations, 0x954 = 12 x 199: records 1..11 at 199i) hold load_heap handles while 0x67758 runs and 0 after it (D:
   0x68096..0x680C2). TAX's 0x66D8C uses the same three fields the same way (G, its record).

   1995 bugs fixed (the original's behaviour under g_original, marked BUG(1995)):
     - 0x67758 DISTRIBUTE ALL (0x67D1A..0x67DC8) loops while the stock is not 0, one token to each listed territory
       with an allowance per pass. If the listed allowances run out first it never ends (D code). G: the list always
       holds every territory 0..156 with an allowance left (each removal is followed by a rebuild) and each token
       added takes one from the stock and one from an allowance, so the hang needs need (> stock on entry) to count
       something the listed allowances do not: an area-list entry (+0x61..) for territories 157..159 (allowed but
       never listed by 0x28CFC), an entry present twice, or an entry 0 or above 160 (no allowance is computed there).
       Tokens outside the area list and negative token bytes only add allowances. Unreachable with a well-formed
       area list of territories 0..156. The fix stops when a pass adds nothing (M6_PLAN risk 6). The player then
       still has stock and no FINISHED (it needs stock == 0) and an empty list, where ADD ONE puts a token into area
       record 0 (no territory) until the stock is 0; not changed (for Erik).

   Oddities kept (D):
     - The need loop reads the tokens with n = [0xF40F8] (dword [0xE31C3 + 69a + [0xF40F8]] sar 24), the expansion
       with the loop variable; the two are equal there.
     - The automatic expansion writes the marker only for areas with 1 or 2 tokens; tokens > 2 get + 2 without it.
     - Allowances are computed for all area records 1..160 (territories 0..159), token byte != 0: 1 for exactly one
       token, 2 otherwise (a negative count too); need counts only n's area list and positive counts.
     - The allowance loop of 0x67758 starts its counter from edx after the debug log 0x78B0C (0x67899), which leaves
       edx = 1; the RESTART copy (0x67EE1) starts at 1 explicitly.
     - ADD ONE, ADD MAX and DISTRIBUTE ALL remove the selected entry by hand (0x68038: shift down, count - 1, f79A9C
       clamps the selection) and then rebuild the list with 0x28CFC anyway: an entry whose allowance is left (ADD MAX
       stopped by the stock) comes back, and the selection may have moved to the entry before it.
     - ADD ONE neither checks the stock nor the allowance (the button is enabled only while stock > 0, and listed
       entries have an allowance).
     - The territory name and the border highlight use 0xF7CA0[sel] even when the list is empty (-1: the name is the
       dword before 0xC77D8, which points to "PIRACY" 0xB6744; the highlight byte is 0xFFB5F), and ADD ONE then adds
       to area record 0. G: an empty list is only reached on the path of the BUG(1995) guard above.
     - The YES/NO of RESTART answers -0x62 (NO) back into the loop; without the confirmations toggle RESTART acts at
       once. RESTART restores all 161 area records, territory states 0..159 and nations 1..11 (the other nations'
       records too, unchanged meanwhile).

   Easter eggs and jokes: none here. The debug log name "ManualPopExpansion" (0xB8ABC) is the M6_PLAN's item 14.
   Texts (obj2, no slips): "%s, you have no stock for expansion." 0xB8A97, "LEFT IN" 0xB8ACF, "STOCK" 0xB8AD7, "%s,
   you have insufficient stock for full expansion. Choose territories to be expanded." 0xB8ADD, "Restart %s
   Population Expansion?" 0xB8B34. */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "state.h"
#include "ui.h"
#include "game.h"
#include "fg.h"
#include "kit.h"
#include "map.h"
#include "mapdata.h"
#include "panel.h"
#include "pop.h"

#define POP_PANEL     0xF596Cu   /* the map screen's panel dialog */
#define POP_PLAYERS   0xF06AEu   /* int16 count; int16 0xF06B0[] */
#define POP_CUR       0xF40F8u   /* int32 the current nation */
#define POP_SHORT     0xF488Cu   /* uint8[10] */
#define POP_FLAGS     0xE1624u   /* bit 0x80 PBEM */
#define POP_PBEM_SELF 0xE1622u   /* int8 the PBEM player's nation */
#define POP_DLG       0xC8954u   /* the screen's buttons */
#define POP_SNAP_AREA 0xEED74u
#define POP_SNAP_NAT  0xEED78u
#define POP_SNAP_TS   0xEED7Cu
#define POP_LIST_N    0xF7C9Cu   /* int32 */
#define POP_LIST      0xF7CA0u   /* int32[157] */

/* nation record n: 0xF14CC + 199n (imul reg, n, 0xC7) */
static uint32_t nrec(int32_t n)
{
    return GAME_NATION + 199u * (uint32_t)n;
}

/* area record a: 0xE31B8 + 69a (shl 4; add; shl 2; add) */
static uint32_t area(int32_t a)
{
    return GAME_AREA + 69u * (uint32_t)a;
}

/* entry i of the choice list (int32 [4i + 0xF7CA0]) */
static int32_t list_at(int32_t i)
{
    return S32(POP_LIST + 4u * (uint32_t)i);
}

/* 0x673A8: POPULATION EXPANSION */
void f673A8(void)
{
    int32_t i;          /* [ebp-0x38] */
    int32_t n;          /* [ebp-0x1C] */
    uint32_t rec;       /* [ebp-0x24] */
    int32_t need;       /* [ebp-0x30] */
    int8_t count;       /* [ebp-0x18] (byte) */
    int32_t j;          /* [ebp-0x34] */
    uint32_t ar;        /* [ebp-0x28] */
    int32_t tok;        /* [ebp-0x2C] */
    int32_t t;
    char buf[100];      /* [ebp-0x9C] */

    setD16(0xBB3FAu, 0xA9);                                         /* 0x673D6 the message area */
    setD16(0xBB3FCu, 0x154);                                        /* 0x673DD */
    setD16(0xBB3FEu, 0x12C);                                        /* 0x673E4 */
    setD16(0xBB400u, 0x23);                                         /* 0x673EB */
    setD16(POP_PLAYERS, 0);                                         /* 0x673F2 */
    for (i = 1; i <= 9; i++) {                                      /* 0x673F9, 0x67408 cmp 9; jg */
        if (D8(nrec(i) + 0x9A) != 0) {                              /* 0x67415 test dl, dl */
            int16_t k = S16(POP_PLAYERS);                           /* 0x6741F movsx edx, ax */
            setD16(POP_PLAYERS, (uint16_t)(k + 1));                 /* 0x67429 */
            setD16(POP_PLAYERS + 2u + 2u * (uint32_t)(int32_t)k, (uint16_t)i);   /* 0x67432 [2*edx + 0xF06B0] = ax */
        }
    }
    f783B8(POP_PANEL, S32(0xF5940u));                               /* 0x67447 the player buttons' labels */
    f3CD6C(3);                                                      /* 0x67451 */
    setD8(0xEE168u, 3);                                             /* 0x6747A */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(0xF52CCu), S32(0xF52C8u));   /* 0x67480 background -> back buffer */
    f7876C();                                                       /* 0x67488 the phase name */
    f3B05C(POP_PANEL);                                              /* 0x67492 */
    f459E4(S16(POP_PLAYERS + 2u));                                  /* 0x6749F dword [0xF06AE] sar 16: the first player */
    f3CBBC();                                                       /* 0x674A4 */
    f3D9FC(S16(POP_PLAYERS + 2u));                                  /* 0x674B1 */
    /* 0x674B6: start = clock() ([ebp-0x20]); the port waits with f78A20 at the end (file comment) */

    for (n = 1; n <= 9; n++) {                                      /* 0x674BE, 0x6766C cmp 9; jg 0x67712 */
        rec = nrec(n);                                              /* 0x67684 */
        setD32(POP_CUR, (uint32_t)n);                               /* 0x6768A */
        setD8(POP_SHORT + (uint32_t)n, 0);                          /* 0x67694 */
        /* 0x676A0 test bh, 0x80; jne 0x676B1; 0x676A8 cmp byte [+0x9A], 0; jne 0x676C7; 0x676B1 test 0x80; je 0x67666;
           0x676BA dword [0xE161F] sar 24 (the PBEM player's nation); jne 0x67666 */
        if (!(((D8(POP_FLAGS) & 0x80) == 0 && D8(rec + 0x9A) != 0)
              || ((D8(POP_FLAGS) & 0x80) != 0 && S8(POP_PBEM_SELF) == n)))
            continue;

        need = 0;                                                   /* 0x676C9 */
        for (i = 0; S8(rec + 0x60) > i; i++) {                      /* 0x674FC dword [rec+0x5D] sar 24; jle 0x6753C */
            /* 0x6750A..0x6752E: tokens = dword [[0xF40F8] + 69a + 0xE31C3] sar 24 with a = byte [rec + i + 0x61];
               0x674CA reads the same byte again for the test == 1 */
            tok = S8(area(D8(rec + (uint32_t)i + 0x61)) + 0x0Eu + D32(POP_CUR));
            if (tok >= 2)                                           /* 0x67531 cmp 2; jl 0x674CA */
                need += 2;                                          /* 0x67536 */
            else if (tok == 1)                                      /* 0x674EB cmp 1; jne */
                need++;                                             /* 0x674F3 */
        }

        if (S8(rec + 0x99) >= need) {                               /* 0x6753C dword [rec+0x96] sar 24; jl 0x67615 */
            count = S8(rec + 0x60);                                 /* 0x67557 */
            for (j = 0; count > j; j++) {                           /* 0x6758D movsx; jle 0x67666 */
                ar = area(D8(rec + (uint32_t)j + 0x61));            /* 0x675B8 */
                tok = S8(ar + (uint32_t)n + 0x0E);                  /* 0x675C1 dword [ar + n + 0xB] sar 24 */
                if (tok <= 2 && tok > 0) {                          /* 0x675CD jg 0x67561; 0x675D2 jle 0x67561 */
                    setD8(rec + 0x98, (uint8_t)(D8(rec + 0x98) + (uint8_t)tok));   /* 0x675DE add byte [+0x98], al */
                    setD8(rec + 0x99, (uint8_t)(D8(rec + 0x99) - (uint8_t)tok));   /* 0x675EA sub byte [+0x99], al */
                    tok += tok;                                     /* 0x675F3 */
                    setD8(ar + (uint32_t)n + 1, (uint8_t)n);        /* 0x67601 the marker */
                    setD8(ar + (uint32_t)n + 0x0E, (uint8_t)tok);   /* 0x6760D */
                } else if (tok > 2) {                               /* 0x67564 cmp esi, 2; jle 0x67587 */
                    setD8(ar + (uint32_t)n + 0x0E, (uint8_t)(D8(ar + (uint32_t)n + 0x0E) + 2));   /* 0x6756F */
                    setD8(rec + 0x98, (uint8_t)(D8(rec + 0x98) + 2));   /* 0x67576 */
                    setD8(rec + 0x99, (uint8_t)(D8(rec + 0x99) - 2));   /* 0x67580 */
                }
            }
        } else {
            f7844C(POP_PANEL, S32(0xF5940u));                       /* 0x67620 highlight n's player button */
            if (S8(rec + 0x99) <= 0) {                              /* 0x67631 test eax, eax; jle 0x676D4 */
                snprintf(buf, sizeof buf, DSTR(0xB8A97u),           /* 0x676EB "%s, you have no stock for expansion." */
                         DSTR(D32(0xC768Cu + 4u * (uint32_t)n)));
                f79270(-500, n, -200, buf, 0);                      /* 0x67708 */
            } else {
                t = S8(rec + 0x9A);                                 /* 0x6763C dword [rec+0x97] sar 24 */
                if (t == 3) {                                       /* 0x67645 */
                    f67758(n);                                      /* 0x6764D ManualPopExpansion */
                } else if (t == 2) {                                /* 0x67654 */
                    /* 0x67661 f680DC([0xF40F8], need): the computer's choice (module AI, M7) */
                    not_ported(0x680DCu);
                }
            }
        }
    }

    if ((D8(POP_FLAGS) & 0x80) != 0)                                /* 0x67712 */
        setD8(0xF40FEu, 1);                                         /* 0x6771B */
    /* 0x67722..0x6774C: wait until clock() >= start + (1000 / [0xBB3F4]) / 10 (file comment) */
    f78A20(1000);
}

/* the five copies of 0x67758 (port guard: a block that could not be allocated is skipped) */
static void to_block(uint32_t field, uint32_t off, uint32_t src, uint32_t len)
{
    uint8_t *p = load_heap_ptr(D32(field) + off, len);

    if (p != NULL)
        memcpy(p, DP(src), len);
}

static void from_block(uint32_t field, uint32_t off, uint32_t dst, uint32_t len)
{
    uint8_t *p = load_heap_ptr(D32(field) + off, len);

    if (p != NULL)
        memcpy(DP(dst), p, len);
}

/* 0x67899..0x67915 / 0x67EE1..0x67F61: allowance byte [0xE31DD + 69a + (n - 1)] (area +0x24+n) for a = 1..160: 0
   where the token byte [0xE31C4 + 69a + (n - 1)] sar 24 (area +0x0E+n) is 0, else (tokens != 1) + 1 */
static void allowances(int32_t nm1)
{
    int32_t a;     /* [ebp-0x34] */
    int32_t tok;

    for (a = 1; a <= 0xA0; a++) {                                   /* 0x678AC cmp 0xA0; jg */
        tok = S8(area(a) + 0x0Fu + (uint32_t)nm1);                  /* 0x678C7 */
        if (tok == 0)                                               /* 0x678E2 test ecx, ecx; je 0x6789E */
            setD8(area(a) + 0x25u + (uint32_t)nm1, 0);              /* 0x678A0 */
        else
            setD8(area(a) + 0x25u + (uint32_t)nm1, (uint8_t)((tok != 1) + 1));   /* 0x678F2 setne; inc; 0x6790F */
    }
}

/* one token into territory t: tokens +1 (inc byte [0xE31C7 + 69(t+1) + n-1]), allowance -1 (dec byte [0xE31DD + ..]),
   stock -1, board +1 (0x67CCB..0x67CFF, 0x67D8E..0x67DC2, 0x67FBC..0x67FF0) */
static void add_token(int32_t t, int32_t nm1, uint32_t rec)
{
    uint32_t a = area(t + 1) + (uint32_t)nm1;

    setD8(a + 0x0F, (uint8_t)(D8(a + 0x0F) + 1));
    setD8(a + 0x25, (uint8_t)(D8(a + 0x25) - 1));
    setD8(rec + 0x99, (uint8_t)(D8(rec + 0x99) - 1));
    setD8(rec + 0x98, (uint8_t)(D8(rec + 0x98) + 1));
}

/* the allowance of territory t: byte [0xE31DD + 69(t+1) + n-1] */
static uint8_t allowance(int32_t t, int32_t nm1)
{
    return D8(area(t + 1) + 0x25u + (uint32_t)nm1);
}

/* 0x67758: ManualPopExpansion */
void f67758(int n_)
{
    int32_t n = n_;     /* [ebp-0x18] */
    uint32_t rec;       /* [ebp-0x1C] */
    int32_t nm1;        /* [ebp-0x20] n - 1 */
    int32_t i;          /* [ebp-0x34] */
    int32_t r;          /* [ebp-0x30] */
    int32_t redraw;     /* [ebp-0x2C] */
    int32_t sel;        /* [ebp-0x28] */
    int32_t remove;     /* [ebp-0x24] */
    int32_t added;      /* port: the BUG(1995) guard of DISTRIBUTE ALL */
    char buf[200];      /* [ebp-0xFC] */

    setD16(0xBB400u, 0x15);                                         /* 0x67773 */
    rec = nrec(n);                                                  /* 0x6778A */
    nm1 = n - 1;                                                    /* 0x67791 */
    setD32(POP_SNAP_AREA, load_heap_alloc(0x2B65));                 /* 0x67799 malloc; 0x6779E */
    setD32(POP_SNAP_TS, load_heap_alloc(0x1E0));                    /* 0x677A8; 0x677AD */
    setD32(POP_SNAP_NAT, load_heap_alloc(0x954));                   /* 0x677B9; 0x677BE */
    for (i = 0; i < 0xA1; i++)                                      /* 0x677CE cmp 0xA1; jge */
        to_block(POP_SNAP_AREA, 69u * (uint32_t)i, area(i), 69);    /* 0x67808 rep movsd (0x11) + movsb */
    for (i = 0; i < 0xA0; i++)                                      /* 0x6781A cmp 0xA0; jge */
        to_block(POP_SNAP_TS, 3u * (uint32_t)i, GAME_TSTATE + 3u * (uint32_t)i, 3);   /* 0x67849 movsw + movsb */
    for (i = 1; i <= 0xB; i++)                                      /* 0x6785D cmp 0xB; jg */
        to_block(POP_SNAP_NAT, 199u * (uint32_t)i, nrec(i), 199);   /* 0x67883 rep movsd (0x31) + movsw + movsb */
    /* 0x67894 f78B0C("ManualPopExpansion", 1): the debug log, a no-op; edx = 1 survives it as the counter below */
    allowances(nm1);                                                /* 0x67899..0x67915 */

    f3CD6C(3);                                                      /* 0x6791C */
    setD8(0xEE168u, 3);                                             /* 0x67946 */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(0xF52CCu), S32(0xF52C8u));   /* 0x6794C */
    fg_vbopen(S32(0xF52C8u));                                       /* 0x6795B */
    f3AF8C(0xF51ACu, 0xFB, 0x186);                                  /* 0x67972 the arrows' pictures */
    f3AF8C(0xF51B4u, 0x178, 0x186);                                 /* 0x67986 */
    fg_setcolor(5);                                                 /* 0x6798D */
    f3A3A0(0x12F, 0x176, DSTR(0xB8ACFu));                           /* 0x679A4 "LEFT IN" */
    f3A044(0x150, 0x176, DSTR(0xB8AD7u));                           /* 0x679B8 "STOCK" */
    fg_vbclose();                                                   /* 0x679BD */
    snprintf(buf, sizeof buf, DSTR(0xB8ADDu), DSTR(D32(0xC768Cu + 4u * (uint32_t)n)));   /* 0x679D9 "%s, you have insufficient ..." */
    f3A9F4(buf);                                                    /* 0x679E7 */
    f7876C();                                                       /* 0x679EC */
    f7853C(POP_DLG);                                                /* 0x679F6 the buttons onto the panel dialog */
    f3B05C(POP_PANEL);                                              /* 0x67A00 */
    f7844C(POP_PANEL, S32(0xF5940u));                               /* 0x67A10 */
    not_ported(0x28CFCu);                                           /* 0x67A1F f28CFC(n, 3, 0): the list (CAL1) */
    redraw = 1;                                                     /* 0x67A24 */
    sel = 0;                                                        /* 0x67A2D */
    remove = 0;                                                     /* 0x67A30 */

    for (;;) {
        /* 0x67A33: the selected territory's name and the stock */
        fg_vbopen(S32(0xF52C8u));                                   /* 0x67A3A */
        f3C9D0(0x109, 0x186, 0x6E, 0x13, 5);                        /* 0x67A58 */
        fg_setcolor(5);                                             /* 0x67A5F */
        f3A0B0(0x109, 0x18A, 0x6E, DSTR(D32(0xC77D8u + 4u * (uint32_t)list_at(sel))));   /* 0x67A87 */
        f3C9D0(0x136, 0x172, 0x13, 0x13, 5);                        /* 0x67A9F */
        fg_setcolor(5);                                             /* 0x67AA6 */
        snprintf(buf, sizeof buf, "%d", (int)S8(rec + 0x99));       /* 0x67ACB itoa(dword [rec+0x96] sar 24, buf, 10) */
        f3A0B0(0x136, 0x176, 0x13, buf);                            /* 0x67ADF */
        fg_vbclose();                                               /* 0x67AE4 */
        f3F670();                                                   /* 0x67AE9 */
        f3F624(list_at(sel), 0xF);                                  /* 0x67AFD */
        if (S8(rec + 0x99) > 0) {                                   /* 0x67B0E test; jle 0x67B50 */
            f3C3D0(POP_PANEL, 0x2A);                                /* 0x67B1C FINISHED off */
            f3C314(POP_PANEL, 0xDC);                                /* 0x67B2B ADD ONE */
            f3C314(POP_PANEL, 0xDD);                                /* 0x67B3A ADD MAX */
            f3C314(POP_PANEL, 0xDB);                                /* 0x67B49 DISTRIBUTE ALL */
        } else {
            f3C314(POP_PANEL, 0x2A);                                /* 0x67B5A */
            f3C3D0(POP_PANEL, 0xDC);                                /* 0x67B69 */
            f3C3D0(POP_PANEL, 0xDD);                                /* 0x67B78 */
            f3C3D0(POP_PANEL, 0xDB);                                /* 0x67B87 */
        }
        if (redraw != 0) {                                          /* 0x67B8C */
            f459E4(n);                                              /* 0x67B95 */
            if (S32(POP_LIST_N) > 0)                                /* 0x67BA0 test edi, edi; jle 0x67BB5 */
                f3DA60(list_at(sel));                               /* 0x67BAE centre on the selection */
            else
                f3D52C(0, 0);                                       /* 0x67BB9 */
            redraw = 0;                                             /* 0x67BC0 */
        }
        f3CBBC();                                                   /* 0x67BC3 */
        r = f78BA4(POP_PANEL, 1);                                   /* 0x67BD2 */
        if (r == -100)                                              /* 0x67BDA */
            continue;
        if (r >= 0) {                                               /* 0x67BE7 test; jl 0x67BFD */
            sel = r;                                                /* 0x67BEE a territory of the list */
            redraw = 1;                                             /* 0x67BF1 */
            continue;
        }
        if (r == -1)                                                /* 0x67C00 FINISHED: je 0x68087 */
            break;

        /* 0x67C09: unsigned compares on r */
        if ((uint32_t)r < (uint32_t)-0x76) {                        /* 0x67C0F jb 0x67F91 */
            if ((uint32_t)r < (uint32_t)-0x78) {                    /* 0x67F94 jb 0x67C67: nothing */
            } else if (r == -0x78) {                                /* 0x67F9A jbe 0x67C7C: ADD MAX */
                while (D8(rec + 0x99) != 0                          /* 0x67C7F cmp byte [+0x99], 0; je 0x67D0A */
                       && allowance(list_at(sel), nm1) != 0)        /* 0x67CA6 cmp byte [..+0xE31DD], 0; je */
                    add_token(list_at(sel), nm1, rec);              /* 0x67CCB..0x67CFF */
                remove = 1;                                         /* 0x67D0F */
                redraw = 1;                                         /* 0x67D12 */
            } else {                                                /* 0x67FA0: -0x77 ADD ONE */
                add_token(list_at(sel), nm1, rec);                  /* 0x67FBC..0x67FF0 */
                if (S8(area(list_at(sel) + 1) + 0x25u + (uint32_t)nm1) == 0)   /* 0x68012 dword [..+0xE31DA] sar 24 */
                    remove = 1;                                     /* 0x6801F */
                redraw = 1;                                         /* 0x68026 */
            }
        } else if (r == -0x76) {                                    /* 0x67C15 jbe 0x67D1A: DISTRIBUTE ALL */
            while (D8(rec + 0x99) != 0) {                           /* 0x67D1D cmp byte [+0x99], 0; je 0x67DCD */
                added = 0;
                for (i = 0; i < S32(POP_LIST_N); i++) {             /* 0x67D37 cmp [0xF7C9C]; jge 0x67D1A */
                    if (D8(rec + 0x99) != 0                         /* 0x67D4B test dh, dh */
                        && allowance(list_at(i), nm1) != 0) {       /* 0x67D69 */
                        add_token(list_at(i), nm1, rec);            /* 0x67D8E..0x67DC2 */
                        added = 1;
                    }
                }
                /* BUG(1995): with stock left and no listed allowance the original passes over the list forever
                   (file comment); the fix stops */
                if (!g_original && added == 0)
                    break;
            }
            remove = 1;                                             /* 0x67DD2 */
            redraw = 1;                                             /* 0x67DD5 */
        } else if ((uint32_t)r < (uint32_t)-0xA) {                  /* 0x67C1E jb 0x67F83 */
            if (r == -0x63) {                                       /* 0x67F83 je 0x67DDD: RESTART */
                snprintf(buf, sizeof buf, DSTR(0xB8B34u),           /* 0x67DF4 "Restart %s Population Expansion?" */
                         DSTR(D32(0xC76C4u + 4u * (uint32_t)n)));
                if ((D8(0xF097Cu) & 0x08) == 0                      /* 0x67E02 test dh, 8; je 0x67E1B */
                    || f795A8(buf) == -0x61) {                      /* 0x67E0D; 0x67E12 cmp -0x61; jne 0x67C67 */
                    for (i = 0; i < 0xA1; i++)                      /* 0x67E28 */
                        from_block(POP_SNAP_AREA, 69u * (uint32_t)i, area(i), 69);   /* 0x67E5E */
                    for (i = 1; i <= 0xB; i++)                      /* 0x67E72 */
                        from_block(POP_SNAP_NAT, 199u * (uint32_t)i, nrec(i), 199);  /* 0x67E99 */
                    for (i = 0; i < 0xA0; i++)                      /* 0x67EAD */
                        from_block(POP_SNAP_TS, 3u * (uint32_t)i, GAME_TSTATE + 3u * (uint32_t)i, 3);   /* 0x67EDC */
                    allowances(nm1);                                /* 0x67EE1..0x67F61 */
                    not_ported(0x28CFCu);                           /* 0x67F6D f28CFC(n, 3, 0) (CAL1) */
                    sel = 0;                                        /* 0x67F74 */
                    redraw = 1;                                     /* 0x67F77 */
                }
            }
        } else if (r == -0xA) {                                     /* 0x67C24 jbe 0x67C4B: next */
            if (S32(POP_LIST_N) - 1 > sel) {                        /* 0x67C51 cmp; jle */
                sel++;                                              /* 0x67C5D */
                redraw = 1;                                         /* 0x67C60 */
            }
        } else if (r == -9 && sel > 0) {                            /* 0x67C26, 0x67C2F: previous */
            sel--;                                                  /* 0x67C3C */
            redraw = 1;                                             /* 0x67C3F */
        }

        /* 0x67C67 */
        if (remove != 0) {
            for (i = sel; S32(POP_LIST_N) - 1 > i; i++)             /* 0x68038 cmp; jle 0x68059 */
                setD32(POP_LIST + 4u * (uint32_t)i, (uint32_t)list_at(i + 1));   /* 0x68050 */
            setD32(POP_LIST_N, (uint32_t)(S32(POP_LIST_N) - 1));    /* 0x68065 */
            f79A9C(&sel);                                           /* 0x6806B */
            remove = 0;                                             /* 0x68070 (ecx = 0) */
            not_ported(0x28CFCu);                                   /* 0x6807D f28CFC(n, 3, 0) (CAL1) */
        }
    }

    /* 0x68087 FINISHED */
    f78664(POP_DLG);                                                /* 0x68091 */
    load_heap_free(D32(POP_SNAP_AREA));                             /* 0x6809D */
    setD32(POP_SNAP_AREA, 0);                                       /* 0x680A7 */
    load_heap_free(D32(POP_SNAP_TS));                               /* 0x680AD */
    setD32(POP_SNAP_TS, 0);                                         /* 0x680B7 */
    load_heap_free(D32(POP_SNAP_NAT));                              /* 0x680BD */
    setD32(POP_SNAP_NAT, 0);                                        /* 0x680C2 */
    setD16(0xBB400u, 0x23);                                         /* 0x680C8 mov word [0xBB400], bx (0x23 since 0x6808C) */
}
