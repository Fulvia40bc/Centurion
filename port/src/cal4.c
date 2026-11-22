/* Module CAL4 of docs/M6_PLAN.md, the human side: Civil War. Literal translations; the address of each instruction a
   line comes from is in its comment. D = read in the code, G = inference. Prototypes and globals: cal4.h.

     0x29E34  f29E34  Civil War (calamity card 0x17)                                          3,680 bytes
     0x2AD50  f2AD50  the Military losses of the two factions                                 2,460
     0x2B6EC  f2B6EC  ChooseFirstFaction: the human's unit chooser (factions, annexing)       2,724
     0x2D3E0  f2D3E0  the human victim's choice of side                                         272
     0x2EE6C  f2EE6C  one automatic step shown (name, count, 1.5 s)                             308
   Ported elsewhere and called here: 0x2AC94 (formulas.c), 0x2C778 (helpers2.c), 0x1E69C and 0x28CFC (cal1.c), 0x5CDE8
   (conflict.c), 0x79270 / 0x795A8 (kit.c), 0x78F04 / 0x78E74 (rules.c), 0x57CE8 / 0x57E28 (move.c).

   Not ported here (each call site stops the port with not_ported(address); the static stubs below carry the names,
   so the call sites read as in the original and a later translation that exports one of these names collides with
   its stub at compile time instead of being shadowed by it). Built with -DCAL4_TEST (port/test/cal4_test.c) they go
   to the test's recorders instead, which return. Real signatures (Watcom: eax, edx, ebx, ecx, then the stack):
     M7 (AI):  0x2C190  void f2C190(int x, int count, const char *text, int src, int target, int mode, int flag)
                        (stack target, mode, flag; ret 0xC): the computer chooses a faction's units (screen, then
                        0x2C298, which calls f2EE6C): 0x2A141 (flag 0), 0x2A247 (flag 1)
               0x2C894  void f2C894(int n, int count, char *text, int ov, int flag, int mode) (stack flag, mode;
                        ret 8): the computer takes a faction's Military losses down to ov; it itoa's into the caller's
                        buffer: 0x2B428 (n 12), 0x2B6DC (n 13)
               0x2D10C  void f2D10C(int v, int k, int o, int b): computer b annexes k cities of faction o: 0x2A90F
               0x2D1C0  void f2D1C0(int v, int k, int o, int b): computer b annexes k tokens of faction o: 0x2AAB9
               (0x2C298 is reached only through 0x2C190: no call site here)
     CAL2:     0x27FCC  void f27FCC(int n, int count, const char *text, int kind, int sub, int target) (stack sub,
                        target; ret 8): NationLosesUnits, the human takes a faction's Military losses: 0x2B3E8 (n 12),
                        0x2B690 (n 13), kind 7 / 8 / 9, sub 0, target = 0x2AC94's remainder
   So today a Civil War runs for an all-human game up to the Military losses: with Military and a remainder to take
   (the usual case) it stops at 0x27FCC until CAL2 lands; without Military it runs to its end. A computer victim or
   beneficiary stops at its 0x2C190 / 0x2D10C / 0x2D1C0, a computer victim's Military at 0x2C894.

   game_rand (f78AE4): no call in these five functions (D), nor in the routines they call that are ported.

   Play by e-mail (dropped by Erik's ruling): no PBEM routine is called here (D), so nothing is listed as
   PBEM_DROPPED. The bit-0x80 tests of byte [0xE1624] are kept literally, as cal1.c does: they only send a PBEM human
   down the computer's path.

   Watcom runtime -> standard C:
     0x7DB3D sprintf -> snprintf bounded to the stack buffer (0x98 bytes in 0x29E34, 0x64 in the other four); the
             formats are written out with their addresses in the comments
     0x8C897 itoa(v, buf, 10) -> snprintf "%d"
     0x7D5D0 memset -> memset on the data object
     0x822B5 malloc -> load_heap_alloc (port handles in the 32-bit fields, game.h); 0x877E6 free -> load_heap_free
     rep movsd / movsw / movsb record copies -> memcpy between g_ds and the block (load_heap_ptr)
   __CHK dropped; 0x78B0C (the debug trace, "ChooseFirstFaction" 0xB32D8) is a no-op and dropped.

   1995 bugs, fixed unless g_original (each marked BUG(1995) in the code; grades code / effect):
     1. 0x2B6EC ALL on tokens (D / D): each token moved decrements the target's stock (0x2BF48) and increments it again
        (0x2BF55), so the target's stock is not reduced, unlike ONE (0x2BE05). With mode 6 (a human beneficiary
        annexing tokens) the beneficiary gains tokens on the board out of nothing (board + k, stock unchanged); with mode
        4 the faction 12's stock counter drifts by k for later Civil Wars. Fixed: the second increment dropped.
     2. Text (misspellings, Erik's ruling of 19 Nov): "beneficary" (0xB2FB3, 0xB2FF2, 0xB308C, 0xB30CB) ->
        "beneficiary", as 0xB2DBD / 0xB2E5A spell it.
     3. Text (a wrong word, same ruling; M6_PLAN "Text slips"): the computer's annexing messages 0xB2FF2 / 0xB30CB say
        "cannot fully absorb the Second Faction cities / tokens" also when the faction being annexed is the first (the
        victim chose to play the second). Fixed: "First" or "Second" by the faction annexed.
     4. 0x2B6EC with an empty choice list (D / G): the box prints entry [0xF7CA0 + 4 * sel] and highlights it although
        nothing is listed. A list built empty by f28CFC would show entry -1 (cal1.c's bug 6: the name "PIRACY" 0xC77D4,
        and f3F624(-1, 15) writing 15 into byte 0xFFB5F, the top byte of the heap pointer [0xFFB5C]); no caller builds
        one (every count is below what the source holds). The list does empty in mode 4 after an overshoot (reviewer,
        G from the code): when the last unit of src is a city taken with left < 5 (e.g. 21 units = 3 cities and 6
        tokens chosen for 20: the tokens, then the cities, left -1). The removal loop does not clear the old entry and
        f79A9C clamps sel to 0, so the original then shows the name of the territory just taken and highlights it
        while only RESTART is enabled. Fixed: no name, no highlight while the list is empty.
   Kept as the original (D code, G effect unless marked):
     - 0x29E34 sorts [0xF40EC] entries of a 9-entry local array that holds only the nations in play. The loop 0x22B00
       decrements [0xF40EC] whenever it clears a +0x9A, so the two counts agree (G); the port zeroes the array, and a
       zero entry (key 0) never moves ahead of a real one (keys >= 0), so even a mismatch would sort as the count. The
       bound is clamped to the array (port guard).
     - 0x29E34 marks the victim's "lost territory" byte +0x3B + v inconsistently: the whole annexation marks city areas
       and token areas where v has no tokens; the partial one marks only through 0x2B6EC modes 5 / 6, where ONE on a
       token marks nothing and ALL marks the area; the "annexes ... cities / tokens" paths mark nothing.
     - 0x29E34's "%s annexes %s faction tokens." shows the victim's panel and home view (edx = v), the other annex
       messages the beneficiary's.
     - 0x29E34's "%s, you are the victim of Civil War but %s will choose units for the first faction." (0xB2E06) is the
       message of the human beneficiary's screen when the victim has Philosophy: it addresses the victim.
     - "The Second Faction no longer exists due to Military losses.  %s will take control of the First Faction" (0xB2EF4)
       ends without a full stop; "%s, Choose side to play." (0xB33BB) has a capital after the comma (style, ruling).
     - 0x2B6EC in mode 4 can take a city (5 points) with fewer than 5 left: left < 0 disables ONE, ALL and FINISHED and
       only RESTART helps (D). An exact choice always exists (G: v holds more than f1 + f2), so this is no lock.
     - 0x2B6EC mode 5 hides the ALL button (style 2) but its loop enables it with ONE: an invisible ALL is clickable,
       with ONE's effect on a city. The labels, x and style of dialog 0xBFE28 stay in the data object for the next call.
     - 0x2B6EC takes "a city" when territory t has any city (the owner is not tested) and its "TARGET" box (local
       [ebp-0x10], always 0) is dead code, translated as such.
     - 0x2AD50 writes 0xF7984 / 0xF7A24 at every land / coast neighbour, also 157..159 (the land aliases) past the 157
       bytes, into the bytes up to 0xF7AC3 (padding before 0xF7AC4).
     - 0x2D3E0 returns 0x5CDE8's code: 0 if 0x5CDE8 ends otherwise than by a faction button (unattended, f7BA64 != 0, or
       another code -1 of the panel); 0x29E34 would then return nation 0's "units" to the victim and give faction 12 to
       the beneficiary, leaving faction 13 on the board. G: not reachable, the unattended runs (/SRD) have no human.
   Easter eggs / oddities: the hidden nations FIRST FACTION / SECOND FACTION (12 / 13) are real nation records that the
   units pass through (M6_PLAN egg 8); the debug trace "ChooseFirstFaction" names the chooser also when it annexes
   (egg 14); new here: the human's choice of side is the CONFLICT phase's losses box 0x5CDE8, showing the two factions'
   tokens and cities as if they were losses, with two extra buttons; the dead "TARGET" box of 0x2B6EC (a TARGET count
   next to LEFT TO, never shown); the invisible but clickable ALL button of the city annexing screen. */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cal1.h"
#include "cal2.h"
#include "cal4.h"
#include "conflict.h"
#include "fg.h"
#include "formulas.h"
#include "game.h"
#include "helpers2.h"
#include "kit.h"
#include "map.h"
#include "mapdata.h"
#include "move.h"
#include "panel.h"
#include "rules.h"
#include "state.h"
#include "ui.h"

#define NATION(n)    (0xF14CCu + 199u * (uint32_t)(n))
#define NAME(n)      DSTR(D32(0xC768Cu + 4u * (uint32_t)(n)))
#define TERR_NAME(t) DSTR(D32(0xC77D8u + 4u * (uint32_t)(t)))
#define TSTATE(t)    (0xE2C18u + 3u * (uint32_t)(t))
#define AREA(r)      (0xE31B8u + 69u * (uint32_t)(r))
#define CAL_VICTIM(k) (0xF77E6u + 3u * (uint32_t)(k))  /* int8 (dword [0xF77E3 + 3k] sar 24) */
#define FLAGS        0xE1624u
#define PBEM_BIT     0x80u
#define CUR_NATION   0xF40F8u
#define PLAYERS      0xF40ECu
#define CUR_DLG      0xF8E78u
#define PANEL_DLG    0xF596Cu
#define FACTION_DLG  0xBFE28u
#define SIDE_DLG     0xC0214u
#define BACKBUF      0xF52C8u
#define BACKGROUND   0xF52CCu
#define BGNUM        0xEE168u
#define CHOICE       0xF7CA0u   /* int32[157] */
#define CHOICE_N     0xF7C9Cu   /* int32 */
#define NEAR12       0xF7984u   /* uint8[157] */
#define NEAR13       0xF7A24u   /* uint8[157] */
#define LANDADJ      0xF8E7Cu   /* uint32[160]: handles (game.h GAME_LANDADJ_PTR) */
#define COASTADJ     0xF90FCu   /* uint32[160]: handles (game.h GAME_COASTADJ_PTR) */
#define LOSS_TOK     0xF547Cu   /* uint8[14] */
#define LOSS_CITY    0xF548Cu   /* uint8[14] */
#define SNAP_AREA    0xEED74u
#define SNAP_NATION  0xEED78u
#define SNAP_TSTATE  0xEED7Cu
#define AI_PAUSE     0xF0980u   /* byte, bit 0x04 */
#define F12          NATION(12) /* 0xF1E20 */
#define F13          NATION(13) /* 0xF1EE7 */

/* ---- calls into code that is not ported yet (see the file comment) ----------------------------------------------- */

#ifdef CAL4_TEST
void cal4_test_f2C190(int x, int count, const char *text, int src, int target, int mode, int flag);
void cal4_test_f2C894(int n, int count, char *text, int ov, int flag, int mode);
void cal4_test_f2D10C(int v, int k, int o, int b);
void cal4_test_f2D1C0(int v, int k, int o, int b);
void cal4_test_f27FCC(int n, int count, const char *text, int kind, int sub, int target);
#define f2C190(x, count, text, src, target, mode, flag) cal4_test_f2C190(x, count, text, src, target, mode, flag)
#define f2C894(n, count, text, ov, flag, mode)          cal4_test_f2C894(n, count, text, ov, flag, mode)
#define f2D10C(v, k, o, b)                              cal4_test_f2D10C(v, k, o, b)
#define f2D1C0(v, k, o, b)                              cal4_test_f2D1C0(v, k, o, b)
#define f27FCC(n, count, text, kind, sub, target)       cal4_test_f27FCC(n, count, text, kind, sub, target)
#else
#include "aical2.h"                             /* f2C190, f2C894, f2D10C, f2D1C0: the computers' Civil War (M7) */

#endif

/* ---- small shared pieces ------------------------------------------------------------------------------------------ */

/* a human player's record: bit 0x80 of [0xE1624] (PBEM) clear and +0x9A == 3 (dword [rec + 0x97] sar 24) */
static int human(uint32_t rec)
{
    return (D8(FLAGS) & PBEM_BIT) == 0 && S8(rec + 0x9Au) == 3;
}

/* territory state t holds a city of nation n (state +2 bit 0x01, owner +0 == n; and eax, 0xFF; cmp) */
static int city_of(int32_t t, int32_t n)
{
    return (D8(TSTATE(t) + 2u) & 1) != 0 && (int32_t)D8(TSTATE(t)) == n;
}

/* the tokens of n in territory t: byte [69(t+1) + n + 0xE31C6] */
static uint32_t tok(int32_t t, int32_t n)
{
    return AREA(t + 1) + 0x0Eu + (uint32_t)n;
}

static void inc8(uint32_t a) { setD8(a, (uint8_t)(D8(a) + 1u)); }
static void dec8(uint32_t a) { setD8(a, (uint8_t)(D8(a) - 1u)); }

/* one token of n in t moves to m (f57CE8(t, n, 0, 1), n's board - 1, stock + 1, f57E28(t, m, 1), m's board + 1,
   stock - 1): the token loops of 0x29E34 (0x2A2DC.., 0x2A4E6.., 0x2A6A9.., 0x2AB08..) */
static void move_token(int32_t t, int32_t n, int32_t m)
{
    f57CE8(t, n, 0, 1);
    dec8(NATION(n) + 0x98u);
    inc8(NATION(n) + 0x99u);
    f57E28(t, m, 1);
    inc8(NATION(m) + 0x98u);
    dec8(NATION(m) + 0x99u);
}

/* the background 3 into the back buffer: f3CD6C(3), byte [0xEE168] = 3, fg_vbcopy of the whole screen from [0xF52CC]
   to [0xF52C8] (0x2B19F..0x2B1D2, 0x2B441..0x2B475, 0x2B81E..0x2B850, 0x2D449..0x2D47E) */
static void background3(void)
{
    f3CD6C(3);
    setD8(BGNUM, 3);
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BACKGROUND), S32(BACKBUF));
}

/* a 16-bit list word at the handle h (the land / coast lists live in heap blocks, game.h): movsx word [eax] */
static int32_t hword(uint32_t h)
{
    const uint8_t *p = load_heap_ptr(h, 2);
    return p != NULL ? (int32_t)(int16_t)(p[0] | p[1] << 8) : 0;   /* port guard: an unresolved handle reads 0 */
}

/* ==== 0x29E34 Civil War ============================================================================================ */

void f29E34(int k)
{
    char buf[0x98];          /* [ebp-0x108]..[ebp-0x71] */
    struct {
        int16_t n;           /* +0 the nation */
        int16_t key;         /* +2 its units in stock */
    } rk[9], tmp;            /* [ebp-0x70] + 4i (9 entries up to [ebp-0x4C]); tmp [ebp-0x24] */
    int32_t cnt;             /* [ebp-0x40] */
    int32_t n;               /* [ebp-0x44] the victim */
    uint32_t rec;            /* [ebp-0x2C] */
    int32_t i;               /* [ebp-0x28] */
    int32_t j;               /* [ebp-0x50] */
    int32_t f1;              /* [ebp-0x1C] (read as a word) */
    int32_t f2;              /* [ebp-0x18] (read as a word) */
    int32_t b;               /* [ebp-0x34] the beneficiary, then the annexing nation */
    int32_t chosen;          /* [ebp-0x3C] */
    int32_t annex;           /* [ebp-0x30] */
    int32_t o;               /* [ebp-0x38] the faction annexed */
    int32_t t;               /* [ebp-0x48] */
    int32_t idx;             /* [ebp-0x20] */
    uint32_t on;

    memset(rk, 0, sizeof rk);   /* port: the original leaves the entries past cnt uninitialised (file comment) */
    n = S8(CAL_VICTIM(k));                                          /* 0x29E5B dword [3k + 0xF77E3] sar 24 */
    rec = NATION(n);                                                /* 0x29E75 */
    setD32(CUR_NATION, (uint32_t)n);                                /* 0x29E7B */
    cnt = 0;                                                        /* 0x29E82 */
    for (i = 1; i <= 9; i++) {                                      /* 0x29E85, 0x29E94 */
        if (D8(NATION(i) + 0x9Au) == 0)                             /* 0x29EA1 byte [199i + 0xF1566] */
            continue;
        rk[cnt].n = (int16_t)i;                                     /* 0x29EB1 */
        rk[cnt].key = (int16_t)((9 - S8(NATION(i) + 0x51u)) * 5     /* 0x29EC2 movsx ax, cities; 0x29ECC imul 5 */
                                + S8(NATION(i) + 0x99u));           /* 0x29ED6 movsx ax, stock; 0x29EE6 */
        cnt++;                                                      /* 0x29EE3 */
    }
    i = S32(PLAYERS) - 2;                                           /* 0x29EED..0x29EF5 */
    if (i > 7)
        i = 7;   /* port guard: rk holds 9 entries ([0xF40EC] <= 9 in every game) */
    for (; i >= 0; i--)                                             /* 0x29EF8, 0x29F08 */
        for (j = 0; j <= i; j++)                                    /* 0x29F00, 0x29F1E..0x29F24 jg */
            if (rk[j + 1].key > rk[j].key) {                        /* 0x29F2C..0x29F39 signed word compare */
                tmp = rk[j + 1];                                    /* 0x29F3E */
                rk[j + 1] = rk[j];                                  /* 0x29F4F */
                rk[j] = tmp;                                        /* 0x29F59 */
            }
    if (rk[0].n == n) {                                             /* 0x29F5F..0x29F66 */
        snprintf(buf, sizeof buf, "%s has the most units in stock so there is no Civil War.", NAME(n));   /* 0x29F7F (0xB2C80) */
        f79270(-500, n, -200, buf, 1);                              /* 0x29F9C */
        goto end;                                                   /* 0x29FA1 jmp 0x2AC7F */
    }
    if (D8(rec + 0x36u) != 0) {                                     /* 0x29FA9 Philosophy */
        f1 = 0;                                                     /* 0x29FB1 */
        f2 = 0xF;                                                   /* 0x29FB4 */
    } else {
        f1 = 0xF;                                                   /* 0x29FBD */
        f2 = 0x14;                                                  /* 0x29FC4 */
        if (D8(rec + 0x30u) != 0)                                   /* 0x29FD1 Music */
            f1 += 5;                                                /* 0x29FD5 */
        if (D8(rec + 0x2Fu) != 0)                                   /* 0x29FDC Drama & Poetry */
            f1 += 5;                                                /* 0x29FE2 */
        if (D8(rec + 0x34u) != 0)                                   /* 0x29FE9 Democracy */
            f1 += 0xA;                                              /* 0x29FEF */
    }
    if (f1 + f2 >= S8(rec + 0x51u) * 5 + S8(rec + 0x98u)) {         /* 0x29FF6..0x2A025 jl */
        snprintf(buf, sizeof buf,
                 "There are no units available for a second faction, so %s quells Civil War with no losses.",
                 NAME(n));                                          /* 0x2A03A (0xB2CB9) */
        f79270(-500, n, -200, buf, 1);                              /* 0x2A057 */
        f1E69C(k);                                                  /* 0x2A05F */
        return;                                                     /* 0x2A06D */
    }
    snprintf(buf, sizeof buf, "Civil War breaks out in %s.", NAME(n));   /* 0x2A081 (0xB2D13) */
    f79270(-500, n, -200, buf, 1);                                  /* 0x2A09E */
    b = rk[0].n;                                                    /* 0x2A0A3 movsx eax, word [ebp-0x70] */

    /* the first faction: f1 units chosen by the victim */
    if (f1 > 0) {                                                   /* 0x2A0AA..0x2A0B0 */
        if (human(rec)) {                                           /* 0x2A0B6..0x2A0D0 */
            snprintf(buf, sizeof buf,
                     "%s, you are the victim of Civil War. Choose units for the first faction.", NAME(n));   /* 0x2A0E9 (0xB2D2F) */
            f2B6EC(n, f1, buf, n, 0xC, 4);                          /* 0x2A105 */
        } else {
            snprintf(buf, sizeof buf,
                     "%s is the victim of Civil War. Choosing units for the first faction.", NAME(n));   /* 0x2A123 (0xB2D78) */
            f2C190(n, f1, buf, n, 0xC, 4, 0);                       /* 0x2A141 [M7] */
        }
    }
    /* the rest of the first faction: f2 units chosen by the beneficiary, still into nation 12 */
    if (f2 > 0) {                                                   /* 0x2A146..0x2A14C */
        if (human(NATION(b))) {                                     /* 0x2A152..0x2A172 */
            if (f1 > 0)                                             /* 0x2A17E */
                snprintf(buf, sizeof buf,
                         "%s, as beneficiary of the Civil War, choose units for the first faction.", NAME(b));   /* 0x2A195 (0xB2DBD) */
            else
                snprintf(buf, sizeof buf,
                         "%s, you are the victim of Civil War but %s will choose units for the first faction.",
                         NAME(n), NAME(b));                         /* 0x2A1BD (0xB2E06) */
            f2B6EC(n, f2, buf, n, 0xC, 4);                          /* 0x2A1D9 */
        } else {
            if (f1 > 0)                                             /* 0x2A1EA */
                snprintf(buf, sizeof buf,
                         "%s is a beneficiary of the Civil War. Choosing units for the first faction.", NAME(b));   /* 0x2A201 (0xB2E5A) */
            else
                snprintf(buf, sizeof buf,
                         "%s is the victim of Civil War but %s is choosing units for the first faction.",
                         NAME(n), NAME(b));                         /* 0x2A229 (0xB2EA6) */
            f2C190(n, f2, buf, n, 0xC, 4, 1);                       /* 0x2A247 [M7] */
        }
    }
    /* everything the victim has left becomes the second faction's */
    for (t = 0; t < 0x9D; t++) {                                    /* 0x2A24C, 0x2A259 jge 0x2A31B */
        if (city_of(t, n)) {                                        /* 0x2A272, 0x2A294 */
            f78F04(t, n);                                           /* 0x2A29F */
            f78E74(t, 0xD);                                         /* 0x2A2AC */
            continue;                                               /* 0x2A2B1 jmp 0x2A253 */
        }
        while (D8(tok(t, n)) != 0)                                  /* 0x2A2B3..0x2A2CD */
            move_token(t, n, 0xD);                                  /* 0x2A2DC..0x2A313 (0xF1F7F, 0xF1F80) */
    }
    if (D8(rec + 0x35u) != 0)                                       /* 0x2A322 byte [199n + 0xF1501] Military */
        f2AD50(n, b);                                               /* 0x2A331 */

    /* the side the victim plays */
    if (S8(F13 + 0x98u) == 0 && S8(F13 + 0x51u) == 0) {             /* 0x2A336 dword [0xF1F7C] sar 24, 0x2A342 [0xF1F35] */
        snprintf(buf, sizeof buf,
                 "The Second Faction no longer exists due to Military losses.  %s will take control of the First Faction",
                 NAME(n));                                          /* 0x2A365 (0xB2EF4) */
        f79270(-500, n, -200, buf, 1);                              /* 0x2A382 */
        chosen = 0xC;                                               /* 0x2A387 */
        annex = 0;                                                  /* 0x2A390 */
    } else {
        if (human(rec)) {                                           /* 0x2A398..0x2A3B2 */
            chosen = f2D3E0(n);                                     /* 0x2A3B7 */
        } else {
            chosen = f2C778(n);                                     /* 0x2A3C7 */
            snprintf(buf, sizeof buf, "%s chooses to play the %s Faction.", NAME(n),
                     chosen == 0xC ? DSTR(0xB2F5Bu) : DSTR(0xB2F61u));   /* 0x2A3FB (0xB2F68) "First" / "Second" */
            f79270((D8(AI_PAUSE) & 4) == 0 ? 0x9C4 : -500, n, -200, buf, 1);   /* 0x2A409, 0x2A43A */
        }
        annex = 1;                                                  /* 0x2A43F */
    }
    o = (chosen == 0xC) + 0xC;                                      /* 0x2A446..0x2A455 sete */
    on = NATION(o);
    for (t = 0; t < 0x9D; t++) {                                    /* 0x2A458, 0x2A465 jge 0x2A531 */
        if (city_of(t, chosen)) {                                   /* 0x2A47E, 0x2A4A0 */
            f78F04(t, chosen);                                      /* 0x2A4AB */
            f78E74(t, n);                                           /* 0x2A4B6 */
            continue;
        }
        while (D8(tok(t, chosen)) != 0)                             /* 0x2A4BD..0x2A4D7 */
            move_token(t, chosen, n);                               /* 0x2A4E6..0x2A52F */
    }
    if (annex == 0)                                                 /* 0x2A531 */
        goto end;
    setD8(rec + 0x9Bu, 1);                                          /* 0x2A542 byte [199n + 0xF1567] */

    /* the other faction o to the beneficiary: all of it if it fits */
    if ((9 - S8(NATION(b) + 0x51u)) * 5 + S8(NATION(b) + 0x99u)     /* 0x2A549..0x2A579 */
            >= S8(on + 0x51u) * 5 + S8(on + 0x98u)                  /* 0x2A57B..0x2A5A8 jl */
        && 9 - S8(NATION(b) + 0x51u) >= S8(on + 0x51u)              /* 0x2A5AE..0x2A5D7 jl */
        && S8(NATION(b) + 0x99u) >= S8(on + 0x98u)) {               /* 0x2A5DD..0x2A5F7 cmp dl, byte; jl */
        for (t = 0; t < 0x9D; t++) {                                /* 0x2A5FD, 0x2A60A jge 0x2A735 */
            if (city_of(t, o)) {                                    /* 0x2A62C, 0x2A648 */
                f78F04(t, o);                                       /* 0x2A650 */
                f78E74(t, b);                                       /* 0x2A65B */
                setD8(AREA(t + 1) + 0x3Bu + (uint32_t)n, 1);        /* 0x2A673 [69(t+1) + n + 0xE31F3] */
                continue;                                           /* 0x2A67A */
            }
            while (D8(tok(t, o)) != 0) {                            /* 0x2A67C..0x2A696 */
                move_token(t, o, b);                                /* 0x2A6A9..0x2A6EC */
                if (S8(tok(t, n)) == 0)                             /* 0x2A705 dword [69(t+1) + n + 0xE31C3] sar 24 */
                    setD8(AREA(t + 1) + 0x3Bu + (uint32_t)n, 1);    /* 0x2A729 */
            }
        }
        snprintf(buf, sizeof buf, "%s annexes the %s faction.", NAME(b),
                 o == 0xD ? DSTR(0xB2F8Bu) : DSTR(0xB2F92u));       /* 0x2A75F (0xB2F98) "second" / "first" */
        f79270(-500, b, -200, buf, 1);                              /* 0x2A77C */
        f1E69C(k);                                                  /* 0x2A784 */
        return;                                                     /* 0x2A792 */
    }

    /* else the nations in rank order take what they can */
    idx = 0;                                                        /* 0x2A795 */
    while (S8(on + 0x51u) * 5 + S8(on + 0x98u) > 0                  /* 0x2A798..0x2A7C5 jle 0x2ABAA */
           && idx < cnt) {                                          /* 0x2A7CB..0x2A7D1 jge 0x2ABAA */
        b = rk[idx].n;                                              /* 0x2A7DA dword [ebp + 4idx - 0x72] sar 16 */
        if (b == n)                                                 /* 0x2A7F1 je 0x2AB9F */
            goto next;
        if (S8(on + 0x51u) > 0) {                                   /* 0x2A800..0x2A80B jle 0x2A9C6 */
            if (9 - S8(NATION(b) + 0x51u) >= S8(on + 0x51u)) {      /* 0x2A811..0x2A83A jge 0x2A919 */
                for (t = 0; t < 0x9D; t++)                          /* 0x2A919, 0x2A926 jge 0x2A97A */
                    if (city_of(t, o)) {                            /* 0x2A93B, 0x2A960 */
                        f78F04(t, o);                               /* 0x2A968 */
                        f78E74(t, b);                               /* 0x2A973 */
                    }
                snprintf(buf, sizeof buf, "%s annexes %s faction cities.", NAME(b),
                         o == 0xD ? DSTR(0xB3061u) : DSTR(0xB3068u));   /* 0x2A9A4 (0xB306E) */
                f79270(-500, b, -200, buf, 1);                      /* 0x2A9C1 */
            } else if (human(NATION(b))) {                          /* 0x2A840..0x2A85E */
                if (g_original)   /* BUG(1995) 2: "beneficary" (0xB2FB3; text) */
                    snprintf(buf, sizeof buf,
                             "%s, you are a beneficary of Civil War. Choose cities to annex.", NAME(b));   /* 0x2A877 */
                else
                    snprintf(buf, sizeof buf,
                             "%s, you are a beneficiary of Civil War. Choose cities to annex.", NAME(b));
                f2B6EC(n, 9 - S8(NATION(b) + 0x51u), buf, o, b, 5); /* 0x2A8AA */
            } else {
                if (g_original)   /* BUG(1995) 2 and 3: "beneficary", "Second" for either faction (0xB2FF2; text) */
                    snprintf(buf, sizeof buf,
                             "%s is a beneficary of Civil War, but cannot fully absorb the Second Faction cities.  "
                             "Choosing cities to annex.", NAME(b)); /* 0x2A8CB */
                else
                    snprintf(buf, sizeof buf,
                             "%s is a beneficiary of Civil War, but cannot fully absorb the %s Faction cities.  "
                             "Choosing cities to annex.", NAME(b), o == 0xD ? "Second" : "First");
                f79270(0x9C4, b, -200, buf, 1);                     /* 0x2A8E8 */
                f2D10C(n, 9 - S8(NATION(b) + 0x51u), o, b);         /* 0x2A90F [M7] */
            }
        }
        /* 0x2A9C6 */
        if (D8(on + 0x98u) == 0)                                    /* 0x2A9CD je 0x2AB9F */
            goto next;
        if (S8(on + 0x98u) > S8(NATION(b) + 0x99u)) {               /* 0x2A9E8..0x2A9F6 cmp al, cl; jle 0x2AAC9 */
            if (human(NATION(b))) {                                 /* 0x2A9FC..0x2AA1A */
                if (g_original)   /* BUG(1995) 2: "beneficary" (0xB308C; text) */
                    snprintf(buf, sizeof buf,
                             "%s, you are a beneficary of Civil War. Choose tokens to annex.", NAME(b));   /* 0x2AA33 */
                else
                    snprintf(buf, sizeof buf,
                             "%s, you are a beneficiary of Civil War. Choose tokens to annex.", NAME(b));
                f2B6EC(n, S8(NATION(b) + 0x99u), buf, o, b, 6);     /* 0x2AA5D */
            } else {
                if (g_original)   /* BUG(1995) 2 and 3: "beneficary", "Second" for either faction (0xB30CB; text) */
                    snprintf(buf, sizeof buf,
                             "%s is a beneficary of Civil War, but cannot fully absorb the Second Faction tokens. "
                             "Choosing tokens to annex.", NAME(b)); /* 0x2AA7E */
                else
                    snprintf(buf, sizeof buf,
                             "%s is a beneficiary of Civil War, but cannot fully absorb the %s Faction tokens. "
                             "Choosing tokens to annex.", NAME(b), o == 0xD ? "Second" : "First");
                f79270(0x9C4, b, -200, buf, 1);                     /* 0x2AA9B */
                f2D1C0(n, S8(NATION(b) + 0x99u), o, b);             /* 0x2AAB9 [M7] */
            }
        } else {
            for (t = 0; t < 0x9D; t++)                              /* 0x2AAC9, 0x2AAD6 jge 0x2AB53 */
                while (D8(tok(t, o)) != 0)                          /* 0x2AADF..0x2AAF9 */
                    move_token(t, o, b);                            /* 0x2AB08..0x2AB4B */
            snprintf(buf, sizeof buf, "%s annexes %s faction tokens.", NAME(b),
                     o == 0xD ? DSTR(0xB3139u) : DSTR(0xB3140u));   /* 0x2AB7D (0xB3146) */
            f79270(-500, n, -200, buf, 1);                          /* 0x2AB9A (edx = the victim) */
        }
    next:
        idx++;                                                      /* 0x2AB9F / 0x2AABE */
    }
    /* 0x2ABAA: whatever o still holds is destroyed */
    if (S8(on + 0x51u) * 5 + S8(on + 0x98u) > 0) {                  /* 0x2ABAA..0x2ABD7 jle 0x2AC7F */
        for (t = 0; t < 0x9D; t++) {                                /* 0x2ABDD, 0x2ABEA jge 0x2AC7F */
            if (city_of(t, o))                                      /* 0x2AC03, 0x2AC28 */
                f78F04(t, o);                                       /* 0x2AC30 */
            while (D8(tok(t, o)) != 0) {                            /* 0x2AC35..0x2AC4F */
                f57CE8(t, o, 0, 1);                                 /* 0x2AC5E */
                dec8(on + 0x98u);                                   /* 0x2AC6A */
                inc8(on + 0x99u);                                   /* 0x2AC77 */
            }
        }
    }
end:
    f1E69C(k);                                                      /* 0x2AC82 */
}

/* ==== 0x2AD50 Military losses ====================================================================================== */

/* arr[t] = 1, then 1 at every land and coast neighbour of t (0x2ADF1..0x2AE73, 0x2AF5D..0x2AFDF) */
static void mark_near(uint32_t arr, int32_t t)
{
    uint32_t p;          /* [ebp-0x1C] */
    int32_t j;           /* [ebp-0x38] */

    setD8(arr + (uint32_t)t, 1);
    p = D32(LANDADJ + 4u * (uint32_t)t);                            /* 0x2ADFE / 0x2AF6A */
    for (j = 1; hword(p) >= j; j++)                                 /* 0x2AE17..0x2AE20 jl */
        setD8(arr + (uint32_t)hword(p + 2u * (uint32_t)j), 1);      /* 0x2AE2D / 0x2AF99 */
    p = D32(COASTADJ + 4u * (uint32_t)t);                           /* 0x2AE39 / 0x2AFA5 */
    for (j = 1; hword(p) >= j; j++)                                 /* 0x2AE52..0x2AE5B jl */
        setD8(arr + (uint32_t)hword(p + 2u * (uint32_t)j), 1);      /* 0x2AE6C / 0x2AFD8 */
}

/* f in t: a city of faction f (5), or its tokens (+0x0E + f) */
static int faction_there(int32_t t, int32_t f)
{
    return city_of(t, f) || D8(tok(t, f)) != 0;
}

/* the units of faction f in the marked areas: a city 5, else its tokens (0x2B067..0x2B0F2, 0x2B0F4..0x2B17F) */
static int32_t near_units(uint32_t arr, int32_t f)
{
    int32_t s = 0;       /* [ebp-0x24] / [ebp-0x20] */
    int32_t t;           /* [ebp-0x3C] */

    for (t = 0; t < 0x9D; t++) {
        if (D8(arr + (uint32_t)t) == 0)                             /* 0x2B0B9 / 0x2B146 */
            continue;
        if (city_of(t, f))                                          /* 0x2B0D0, 0x2B0EC / 0x2B15D, 0x2B179 */
            s += 5;                                                 /* 0x2B0EE / 0x2B17B */
        else if (D8(tok(t, f)) != 0)                                /* 0x2B083 / 0x2B110 */
            s += S8(tok(t, f));                                     /* 0x2B099 dword [.. + 0xE31CF] sar 24 / 0xE31D0 */
    }
    return s;
}

/* every unit of faction f in the marked areas removed, one area at a time, each shown for 1.5 s (0x2B19F..0x2B34C,
   0x2B441..0x2B5EF); returns the points left of 5 */
static int32_t auto_losses(uint32_t arr, int32_t f, int32_t left, uint32_t text, uint32_t s_left, uint32_t s_remove)
{
    int32_t t;           /* [ebp-0x3C] */
    int32_t x;           /* [ebp-0x18] */

    background3();                                                  /* 0x2B19F..0x2B1D2 / 0x2B441..0x2B475 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x2B1E1 */
    fg_setcolor(5);                                                 /* 0x2B1EB */
    f3A3A0(0x12F, 0x176, DSTR(s_left));                             /* 0x2B202 "LEFT TO" */
    f3A044(0x150, 0x176, DSTR(s_remove));                           /* 0x2B216 "REMOVE" */
    fg_vbclose();                                                   /* 0x2B21B */
    f3AB7C(DSTR(text));                                             /* 0x2B225 */
    f7876C();                                                       /* 0x2B22A */
    f3B05C(PANEL_DLG);                                              /* 0x2B234 */
    f3C940(D32(CUR_DLG));                                           /* 0x2B23E */
    f2EE6C(f, -1, left);                                            /* 0x2B252 */
    for (t = 0; t < 0x9D; t++) {                                    /* 0x2B257, 0x2B2D5 jge 0x2B347 */
        if (D8(arr + (uint32_t)t) == 0)                             /* 0x2B2E9 */
            continue;
        if (city_of(t, f)) {                                        /* 0x2B300, 0x2B320 */
            f78F04(t, f);                                           /* 0x2B32E */
            left -= 5;                                              /* 0x2B333 */
            setD8(arr + (uint32_t)t, 0);                            /* 0x2B33C */
        } else {
            if (D8(tok(t, f)) == 0)                                 /* 0x2B270 je 0x2B2CF */
                continue;
            x = S8(tok(t, f));                                      /* 0x2B284 sar 24 */
            f57CE8(t, f, 0, x);                                     /* 0x2B297 */
            setD8(NATION(f) + 0x98u, (uint8_t)(D8(NATION(f) + 0x98u) - (uint8_t)x));   /* 0x2B29F sub byte, al */
            setD8(NATION(f) + 0x99u, (uint8_t)(D8(NATION(f) + 0x99u) + (uint8_t)x));   /* 0x2B2A8 add byte, al */
            left -= x;                                              /* 0x2B2B1 */
            setD8(arr + (uint32_t)t, 0);                            /* 0x2B2B9 */
        }
        f2EE6C(f, t, left);                                         /* 0x2B2CA */
    }
    f3C8C4(D32(CUR_DLG));                                           /* 0x2B34C */
    return left;
}

void f2AD50(int v, int b)
{
    char buf[0x64];      /* [ebp-0xA8]..[ebp-0x45] */
    int32_t t;           /* [ebp-0x3C] */
    int32_t a1;          /* [ebp-0x24] the first faction's units next to the second */
    int32_t a2;          /* [ebp-0x20] the second faction's units next to the first */
    int32_t r1;          /* [ebp-0x30] */
    int32_t r2;          /* [ebp-0x2C] */
    int32_t mode;        /* [ebp-0x34] */
    int32_t ov;          /* [ebp-0x28] */
    int8_t flag;         /* [ebp-0x14] */

    memset(DP(NEAR12), 0, 0x9D);                                    /* 0x2AD79 */
    memset(DP(NEAR13), 0, 0x9D);                                    /* 0x2AD8A */
    for (t = 0; t < 0x9D; t++)                                      /* 0x2AD8F, 0x2AD9C jge 0x2AE75 */
        if (faction_there(t, 0xC))                                  /* 0x2ADBE, 0x2ADDA, 0x2ADE7 [.. + 0xE31D2] */
            mark_near(NEAR13, t);                                   /* 0x2ADF4.. 12's areas and their neighbours */
    for (t = 0; t < 0x9D; t++) {                                    /* 0x2AE75, 0x2AE82 jge 0x2AEFB */
        if (D8(NEAR13 + (uint32_t)t) == 0)                          /* 0x2AE9A */
            continue;
        setD8(NEAR13 + (uint32_t)t, 0);                             /* 0x2AEA1 */
        if (faction_there(t, 0xD))                                  /* 0x2AEBC, 0x2AED8, 0x2AEE5 [.. + 0xE31D3] */
            setD8(NEAR13 + (uint32_t)t, 1);                         /* 0x2AEF2: 13's areas next to 12 */
    }
    for (t = 0; t < 0x9D; t++)                                      /* 0x2AEFB, 0x2AF08 jge 0x2AFE1 */
        if (faction_there(t, 0xD))                                  /* 0x2AF2A, 0x2AF46, 0x2AF53 */
            mark_near(NEAR12, t);                                   /* 0x2AF60.. */
    for (t = 0; t < 0x9D; t++) {                                    /* 0x2AFE1, 0x2AFEE jge 0x2B067 */
        if (D8(NEAR12 + (uint32_t)t) == 0)                          /* 0x2B006 */
            continue;
        setD8(NEAR12 + (uint32_t)t, 0);                             /* 0x2B00D */
        if (faction_there(t, 0xC))                                  /* 0x2B028, 0x2B044, 0x2B051 */
            setD8(NEAR12 + (uint32_t)t, 1);                         /* 0x2B05E: 12's areas next to 13 */
    }
    a1 = near_units(NEAR12, 0xC);                                   /* 0x2B067..0x2B0F2 */
    a2 = near_units(NEAR13, 0xD);                                   /* 0x2B0F4..0x2B17F */
    r1 = 5;                                                         /* 0x2B186 */
    r2 = 5;                                                         /* 0x2B189 */

    /* the first faction */
    if (a1 > 0 && 5 >= a1)                                          /* 0x2B18C jle, 0x2B196 jl */
        r1 = auto_losses(NEAR12, 0xC, r1, 0xB3173u, 0xB3164u, 0xB316Cu);   /* 0x2B19F..0x2B34C */
    mode = a1 <= 5 ? 9 : 7;                                         /* 0x2B351..0x2B360 */
    ov = f2AC94(0xC, r1);                                           /* 0x2B36F */
    flag = (int8_t)(f2C778(v) != 0xC);                              /* 0x2B37A, 0x2B382 setne */
    if (r1 > 0) {                                                   /* 0x2B388 jle 0x2B42D */
        if (human(NATION(v))) {                                     /* 0x2B392..0x2B3B0 */
            snprintf(buf, sizeof buf, "%s, choose MILITARY losses for First Faction.", NAME(v));   /* 0x2B3C9 (0xB31C0) */
            f27FCC(0xC, r1, buf, mode, 0, ov);                      /* 0x2B3E8 [CAL2] */
        } else {
            snprintf(buf, sizeof buf, "%s choosing MILITARY losses for First Faction.", NAME(v));   /* 0x2B406 (0xB31EE) */
            f2C894(0xC, r1, buf, ov, flag, mode);                   /* 0x2B428 [M7] */
        }
    }

    /* the second faction */
    if (a2 > 0 && a2 <= 5)                                          /* 0x2B42D jle, 0x2B437 jg */
        r2 = auto_losses(NEAR13, 0xD, r2, 0xB322Cu, 0xB321Du, 0xB3225u);   /* 0x2B441..0x2B5EF */
    if (S8(F13 + 0x98u) != 0 || S8(F13 + 0x51u) != 0) {             /* 0x2B5F4 [0xF1F7C] sar 24, 0x2B600 [0xF1F35] */
        mode = (a2 <= 5) + 8;                                       /* 0x2B610..0x2B61C setle */
        ov = f2AC94(0xD, r2);                                       /* 0x2B62A */
        if (r2 > 0) {                                               /* 0x2B632 jle 0x2B6E1 */
            if (human(NATION(b))) {                                 /* 0x2B63C..0x2B658 */
                snprintf(buf, sizeof buf, "%s, choose MILITARY losses for Second Faction.", NAME(b));   /* 0x2B671 (0xB3279) */
                f27FCC(0xD, r2, buf, mode, 0, ov);                  /* 0x2B690 [CAL2] */
            } else {
                snprintf(buf, sizeof buf, "%s choosing MILITARY losses for Second Faction.", NAME(b));   /* 0x2B6AE (0xB32A8) */
                flag = (int8_t)(flag == 0);                         /* 0x2B6B6..0x2B6BF sete */
                f2C894(0xD, r2, buf, ov, flag, mode);               /* 0x2B6DC [M7] */
            }
        }
    }
}

/* ==== 0x2B6EC ChooseFirstFaction ================================================================================== */

/* the snapshot of 0x2B6EC: area records 0..160 (69 bytes), territory states 0..159 (3) and nations 1..13 (199, at
   199n) to the heap blocks [0xEED74] / [0xEED7C] / [0xEED78] (0x2B748..0x2B80D) or back (0x2BFF8..0x2C0BB: areas,
   nations, then territory states). Port guard: a block that is not there (out of memory) is skipped; the original
   would write through a NULL pointer */
static void faction_snapshot(int to_block)
{
    uint8_t *area = load_heap_ptr(D32(SNAP_AREA), 0x2B65u);
    uint8_t *ts = load_heap_ptr(D32(SNAP_TSTATE), 0x1E0u);
    uint8_t *nat = load_heap_ptr(D32(SNAP_NATION), 0xAE2u);
    uint32_t i;

    if (area != NULL) {
        for (i = 0; i < 0xA1u; i++) {                               /* 0x2B753 / 0x2C005 */
            if (to_block)
                memcpy(area + 69u * i, DP(AREA(i)), 69);            /* 0x2B78D rep movsd (0x11) + movsb */
            else
                memcpy(DP(AREA(i)), area + 69u * i, 69);            /* 0x2C03B */
        }
    }
    if (to_block) {
        if (ts != NULL)
            for (i = 0; i < 0xA0u; i++)                             /* 0x2B79F */
                memcpy(ts + 3u * i, DP(TSTATE(i)), 3);              /* 0x2B7CE movsw + movsb */
        if (nat != NULL)
            for (i = 1; i < 0xEu; i++)                              /* 0x2B7E2 */
                memcpy(nat + 199u * i, DP(NATION(i)), 199);         /* 0x2B808 rep movsd (0x31) + movsw + movsb */
    } else {
        if (nat != NULL)
            for (i = 1; i < 0xEu; i++)                              /* 0x2C04F */
                memcpy(DP(NATION(i)), nat + 199u * i, 199);         /* 0x2C075 */
        if (ts != NULL)
            for (i = 0; i < 0xA0u; i++)                             /* 0x2C089 */
                memcpy(DP(TSTATE(i)), ts + 3u * i, 3);              /* 0x2C0B8 */
    }
}

void f2B6EC(int x, int count, const char *text, int src, int target, int mode)
{
    char buf[0x64];      /* [ebp-0x9C]..[ebp-0x39] */
    int32_t tgt;         /* [ebp-0x10] the "TARGET" count: always 0 */
    int32_t citycost;    /* [ebp-0x14] what a city counts */
    int32_t left;        /* [ebp-0x18] */
    int32_t removed;     /* [ebp-0x1C] */
    int32_t sel;         /* [ebp-0x20] (its address goes to f79A9C) */
    int32_t redraw;      /* [ebp-0x24] */
    int32_t r;           /* [ebp-0x28] */
    int32_t i;           /* [ebp-0x2C] */
    int32_t t;

    setD16(0xBB400u, 0x15);                                         /* 0x2B717 */
    setD32(SNAP_AREA, load_heap_alloc(0x2B65u));                    /* 0x2B71E malloc, 0x2B723 */
    setD32(SNAP_TSTATE, load_heap_alloc(0x1E0u));                   /* 0x2B72D, 0x2B732 */
    setD32(SNAP_NATION, load_heap_alloc(0xAE2u));                   /* 0x2B73E, 0x2B743 */
    faction_snapshot(1);                                            /* 0x2B748..0x2B80D */
    tgt = 0;                                                        /* 0x2B811 */
    /* 0x2B819 0x78B0C("ChooseFirstFaction"): the no-op debug trace, dropped */
    citycost = 0;   /* port: [ebp-0x14] is left uninitialised for a mode other than 4, 5, 6 (no caller passes one) */
    background3();                                                  /* 0x2B81E..0x2B850 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x2B85F */
    f3AF8C(0xF51ACu, 0xFB, 0x186);                                  /* 0x2B876 */
    f3AF8C(0xF51B4u, 0x178, 0x186);                                 /* 0x2B88A */
    if (tgt != 0) {                                                 /* 0x2B88F: dead code, kept */
        fg_setcolor(5);                                             /* 0x2B897 */
        f3A3A0(0x1B7, 0x176, DSTR(0xB32EBu));                       /* 0x2B8B3 "TARGET" */
        f3C9D0(0x1BC, 0x172, 0x13, 0x13, 5);                        /* 0x2B8C6 */
        fg_setcolor(5);                                             /* 0x2B8CD */
        snprintf(buf, sizeof buf, "%d", tgt);                       /* 0x2B8E9 itoa(tgt, buf, 10) */
        f3A0B0(0x1BC, 0x176, 0x13, buf);                            /* 0x2B8FD */
    }
    fg_setcolor(5);                                                 /* 0x2B904 */
    f3A3A0(0x12F, 0x176, DSTR(0xB32F2u));                           /* 0x2B91B "LEFT TO" */
    if (mode == 4) {                                                /* 0x2B920 */
        f3A044(0x150, 0x176, DSTR(0xB32FAu));                       /* 0x2B939 "SELECT" */
        citycost = 5;                                               /* 0x2B93E */
        f3C284(FACTION_DLG, 0x3F, 0xB3301u);                        /* 0x2B954 "SELECT ONE" */
        f3C284(FACTION_DLG, 0x40, 0xB330Cu);                        /* 0x2B968 "SELECT ALL" */
        f3BB48(FACTION_DLG, 0x3F, 0xDB);                            /* 0x2B97C */
        f3BFBC(FACTION_DLG, 0x40, 1);                               /* 0x2B990 */
        f3C0D8(FACTION_DLG, 0x40, 1);                               /* 0x2B9A4 */
    } else if (mode == 5) {                                         /* 0x2B9B1 */
        f3A044(0x150, 0x176, DSTR(0xB3317u));                       /* 0x2B9CA "ANNEX" */
        citycost = 1;                                               /* 0x2B9CF */
        f3C284(FACTION_DLG, 0x3F, 0xB331Du);                        /* 0x2B9E1 "ANNEX" */
        f3BB48(FACTION_DLG, 0x3F, 0x10E);                           /* 0x2B9F5 */
        f3BFBC(FACTION_DLG, 0x40, 0);                               /* 0x2BA06 */
        f3C0D8(FACTION_DLG, 0x40, 2);                               /* 0x2BA1A (no label: invisible) */
    } else if (mode == 6) {                                         /* 0x2BA27 */
        f3A044(0x150, 0x176, DSTR(0xB3323u));                       /* 0x2BA40 "ANNEX" */
        citycost = 1;                                               /* 0x2BA45 */
        f3BB48(FACTION_DLG, 0x3F, 0xDB);                            /* 0x2BA57 */
        f3BFBC(FACTION_DLG, 0x40, 1);                               /* 0x2BA68 */
        f3C0D8(FACTION_DLG, 0x40, 1);                               /* 0x2BA79 */
        f3C284(FACTION_DLG, 0x3F, 0xB3329u);                        /* 0x2BA8D "ANNEX ONE" */
        f3C284(FACTION_DLG, 0x40, 0xB3333u);                        /* 0x2BAA1 "ANNEX ALL" */
    }
    fg_vbclose();                                                   /* 0x2BAA6 */
    f3AB7C(text);                                                   /* 0x2BAAE */
    f7876C();                                                       /* 0x2BAB3 */
    f7853C(FACTION_DLG);                                            /* 0x2BABD */
    f3B05C(PANEL_DLG);                                              /* 0x2BAC7 */
    left = count;                                                   /* 0x2BACF */
    f28CFC(src, mode, 0);                                           /* 0x2BADA */
    redraw = 1;                                                     /* 0x2BADF */
    sel = 0;                                                        /* 0x2BAE8 */
    removed = 0;                                                    /* 0x2BAEB */
    for (;;) {
        /* 0x2BAEE */
        fg_vbopen(S32(BACKBUF));                                    /* 0x2BAF5 */
        f3C9D0(0x109, 0x186, 0x6E, 0x13, 5);                        /* 0x2BB13 */
        fg_setcolor(5);                                             /* 0x2BB1A */
        if (g_original || S32(CHOICE_N) > 0)   /* BUG(1995) 4: an empty list shows a stale entry (file comment) */
            f3A0B0(0x109, 0x18A, 0x6E, TERR_NAME(S32(CHOICE + 4u * (uint32_t)sel)));   /* 0x2BB42 */
        f3C9D0(0x136, 0x172, 0x13, 0x13, 5);                        /* 0x2BB5A */
        fg_setcolor(5);                                             /* 0x2BB61 */
        snprintf(buf, sizeof buf, "%d", left);                      /* 0x2BB7D itoa(left, buf, 10) */
        f3A0B0(0x136, 0x176, 0x13, buf);                            /* 0x2BB91 */
        fg_vbclose();                                               /* 0x2BB96 */
        f459E4(x);                                                  /* 0x2BB9E */
        f3F670();                                                   /* 0x2BBA3 */
        if (g_original || S32(CHOICE_N) > 0)   /* BUG(1995) 4: an empty list highlights a stale entry */
            f3F624(S32(CHOICE + 4u * (uint32_t)sel), 15);           /* 0x2BBB7 */
        if (left != tgt) {                                          /* 0x2BBC2 je 0x2BBF5 */
            f3C3D0(PANEL_DLG, 0x2A);                                /* 0x2BBD0 FINISHED off */
            f3C314(PANEL_DLG, 0x3F);                                /* 0x2BBDF ONE on */
            f3C314(PANEL_DLG, 0x40);                                /* 0x2BBEE ALL on */
        } else {
            f3C314(PANEL_DLG, 0x2A);                                /* 0x2BBFF FINISHED on */
            f3C3D0(PANEL_DLG, 0x3F);                                /* 0x2BC0E ONE off */
            f3C3D0(PANEL_DLG, 0x40);                                /* 0x2BC1D ALL off */
        }
        if (left < tgt) {                                           /* 0x2BC25 jge 0x2BC57 */
            f3C3D0(PANEL_DLG, 0x3F);                                /* 0x2BC34 */
            f3C3D0(PANEL_DLG, 0x40);                                /* 0x2BC43 */
            f3C3D0(PANEL_DLG, 0x2A);                                /* 0x2BC52 */
        }
        if (redraw != 0) {                                          /* 0x2BC57 */
            if (S32(CHOICE_N) > 0)                                  /* 0x2BC63 jle */
                f3DA60(S32(CHOICE + 4u * (uint32_t)sel));           /* 0x2BC71 */
            else
                f3D52C(0, 0);                                       /* 0x2BC7C */
            redraw = 0;                                             /* 0x2BC83 */
        }
        f3CBBC();                                                   /* 0x2BC86 */
        r = f78BA4(PANEL_DLG, 1);                                   /* 0x2BC95 */
        if (r == -100)                                              /* 0x2BC9D je 0x2BAEE */
            continue;
        if (r >= 0) {                                               /* 0x2BCAA jl 0x2BCC0 */
            sel = r;                                                /* 0x2BCB1 a map click on a listed territory */
            redraw = 1;                                             /* 0x2BCB4 */
            continue;                                               /* 0x2BCBB jmp 0x2BAEE */
        }
        if (r == -1)                                                /* 0x2BCC3 je 0x2C149 */
            break;
        if ((uint32_t)r < (uint32_t)-0x63) {                        /* 0x2BCCF jb 0x2C0DE */
            if ((uint32_t)r >= (uint32_t)-0x66) {                   /* 0x2C0DE jb 0x2C0FB */
                t = S32(CHOICE + 4u * (uint32_t)sel);               /* 0x2BD2F / 0x2BE45 */
                if (r == -0x66) {                                   /* 0x2C0EC jbe 0x2BE45: ALL */
                    if ((D8(TSTATE(t) + 2u) & 1) != 0) {            /* 0x2BE54 */
                        if (mode != 4)                              /* 0x2BE63 */
                            setD8(AREA(t + 1) + 0x3Bu + (uint32_t)x, 1);   /* 0x2BE81 */
                        f78F04(t, src);                             /* 0x2BE95 */
                        f78E74(t, target);                          /* 0x2BEA7 */
                        left -= citycost;                           /* 0x2BEAF */
                        removed = 1;                                /* 0x2BFB2 */
                    } else {
                        while (left != 0 && D8(tok(t, src)) != 0) { /* 0x2BEB7, 0x2BEDB */
                            left--;                                 /* 0x2BEE7 */
                            f57CE8(t, src, 0, 1);                   /* 0x2BEFE */
                            dec8(NATION(src) + 0x98u);              /* 0x2BF0A */
                            inc8(NATION(src) + 0x99u);              /* 0x2BF17 */
                            f57E28(t, target, 1);                   /* 0x2BF2F */
                            inc8(NATION(target) + 0x98u);           /* 0x2BF3B */
                            dec8(NATION(target) + 0x99u);           /* 0x2BF48 */
                            if (g_original)   /* BUG(1995) 1: the target's stock given back (the ALL path) */
                                inc8(NATION(target) + 0x99u);       /* 0x2BF55 */
                        }
                        if (mode != 4)                              /* 0x2BF60 */
                            setD8(AREA(t + 1) + 0x3Bu + (uint32_t)x, 1);   /* 0x2BF82 */
                        if (S8(tok(t, src)) == 0)                   /* 0x2BFA5 dword [.. + 0xE31C3] sar 24 */
                            removed = 1;                            /* 0x2BFB2 */
                    }
                    redraw = 1;                                     /* 0x2BFB9 */
                } else if (r == -0x65) {                            /* 0x2C0F2 je 0x2BD2F: ONE */
                    if ((D8(TSTATE(t) + 2u) & 1) != 0) {            /* 0x2BD3E */
                        if (mode != 4)                              /* 0x2BD4D */
                            setD8(AREA(t + 1) + 0x3Bu + (uint32_t)x, 1);   /* 0x2BD6B */
                        f78F04(t, src);                             /* 0x2BD7F */
                        f78E74(t, target);                          /* 0x2BD91 */
                        left -= citycost;                           /* 0x2BD99 */
                        removed = 1;                                /* 0x2BE32 */
                    } else {
                        left--;                                     /* 0x2BDA4 */
                        f57CE8(t, src, 0, 1);                       /* 0x2BDBB */
                        dec8(NATION(src) + 0x98u);                  /* 0x2BDC7 */
                        inc8(NATION(src) + 0x99u);                  /* 0x2BDD4 */
                        f57E28(t, target, 1);                       /* 0x2BDEC */
                        inc8(NATION(target) + 0x98u);               /* 0x2BDF8 */
                        dec8(NATION(target) + 0x99u);               /* 0x2BE05 */
                        if (S8(tok(t, src)) == 0)                   /* 0x2BE25 sar 24 */
                            removed = 1;                            /* 0x2BE32 */
                    }
                    redraw = 1;                                     /* 0x2BE39 */
                }
            }
        } else if (r == -0x63) {                                    /* 0x2BCD8 jbe 0x2BFC5 RESTART */
            snprintf(buf, sizeof buf, "Are you sure you want to Restart?");   /* 0x2BFD1 (0xB333D) */
            if ((D8(0xF097Cu) & 0x08) == 0 || f795A8(buf) == -0x61) {   /* 0x2BFDF, 0x2BFEA, 0x2BFEF */
                faction_snapshot(0);                                /* 0x2BFF8..0x2C0BB */
                left = count;                                       /* 0x2C0C0 */
                f28CFC(src, mode, 0);                               /* 0x2C0CB */
                sel = 0;                                            /* 0x2C0D2 */
                redraw = 1;                                         /* 0x2C0D5 */
            }
        } else if ((uint32_t)r < (uint32_t)-0xA) {                  /* 0x2BCE1 jb 0x2C0FB */
            /* nothing */
        } else if (r == -0xA) {                                     /* 0x2BCE7 jbe 0x2BD0E */
            if (S32(CHOICE_N) - 1 > sel) {                          /* 0x2BD14 cmp; jle */
                sel++;                                              /* 0x2BD20 */
                redraw = 1;                                         /* 0x2BD23 */
            }
        } else if (r == -9) {                                       /* 0x2BCEC */
            if (sel > 0) {                                          /* 0x2BCF6 jle */
                sel--;                                              /* 0x2BCFF */
                redraw = 1;                                         /* 0x2BD02 */
            }
        }
        /* 0x2C0FB */
        if (removed != 0) {                                         /* 0x2C0FF je 0x2BAEE */
            for (i = sel; S32(CHOICE_N) - 1 > i; i++)               /* 0x2C105, 0x2C113..0x2C11C */
                setD32(CHOICE + 4u * (uint32_t)i, D32(CHOICE + 4u * (uint32_t)(i + 1)));   /* 0x2C12B */
            setD32(CHOICE_N, (uint32_t)(S32(CHOICE_N) - 1));        /* 0x2C134 */
            f79A9C(&sel);                                           /* 0x2C13F */
            removed = 0;                                            /* 0x2BAEB (edx = 0, kept by f79A9C) */
        }
    }
    f78664(FACTION_DLG);                                            /* 0x2C14E */
    load_heap_free(D32(SNAP_AREA));                                 /* 0x2C15A */
    setD32(SNAP_AREA, 0);                                           /* 0x2C164 */
    load_heap_free(D32(SNAP_TSTATE));                               /* 0x2C16A */
    setD32(SNAP_TSTATE, 0);                                         /* 0x2C174 */
    load_heap_free(D32(SNAP_NATION));                               /* 0x2C17A */
    setD32(SNAP_NATION, 0);                                         /* 0x2C17F */
}

/* ==== 0x2D3E0 the human's side ===================================================================================== */

int f2D3E0(int n)
{
    char buf[0x64];      /* [ebp-0x84]..[ebp-0x21] */
    int32_t i;           /* [ebp-0x18] */
    int32_t r;           /* [ebp-0x1C] */

    for (i = 0; i < 0xE; i++) {                                     /* 0x2D3FA, 0x2D405 jge 0x2D421 */
        setD8(LOSS_TOK + (uint32_t)i, 0);                           /* 0x2D410 */
        setD8(LOSS_CITY + (uint32_t)i, 0);                          /* 0x2D419 */
    }
    setD8(LOSS_TOK + 0xCu, D8(F12 + 0x98u));                        /* 0x2D426 [0xF5488] = [0xF1EB8] */
    setD8(LOSS_CITY + 0xCu, D8(F12 + 0x51u));                       /* 0x2D430 [0xF5498] = [0xF1E71] */
    setD8(LOSS_TOK + 0xDu, D8(F13 + 0x98u));                        /* 0x2D43A [0xF5489] = [0xF1F7F] */
    setD8(LOSS_CITY + 0xDu, D8(F13 + 0x51u));                       /* 0x2D444 [0xF5499] = [0xF1F38] */
    background3();                                                  /* 0x2D449..0x2D47E */
    f7853C(SIDE_DLG);                                               /* 0x2D48B */
    f3B05C(PANEL_DLG);                                              /* 0x2D495 */
    f3D9FC(n);                                                      /* 0x2D49D */
    f3CBBC();                                                       /* 0x2D4A2 */
    snprintf(buf, sizeof buf, "%s, Choose side to play.", NAME(n)); /* 0x2D4BE (0xB33BB; the capital kept, style) */
    r = f5CDE8(0, buf);                                             /* 0x2D4CE */
    f78664(SIDE_DLG);                                               /* 0x2D4DB */
    return r;                                                       /* 0x2D4E0 */
}

/* ==== 0x2EE6C one automatic step =================================================================================== */

void f2EE6C(int n, int t, int left)
{
    char buf[0x64];      /* [ebp-0x78]..[ebp-0x15] */

    fg_vbopen(S32(BACKBUF));                                        /* 0x2EE8E */
    f3C9D0(0x109, 0x186, 0x6E, 0x13, 5);                            /* 0x2EEAC */
    if (t >= 0) {                                                   /* 0x2EEB1 jl */
        fg_setcolor(5);                                             /* 0x2EEB9 */
        f3A0B0(0x109, 0x18A, 0x6E, TERR_NAME(t));                   /* 0x2EEDA */
    }
    f3C9D0(0x136, 0x172, 0x13, 0x13, 5);                            /* 0x2EEF2 */
    fg_setcolor(5);                                                 /* 0x2EEF9 */
    snprintf(buf, sizeof buf, "%d", left);                          /* 0x2EF0F itoa(left, buf, 10) */
    f3A0B0(0x136, 0x176, 0x14, buf);                                /* 0x2EF23 */
    fg_vbclose();                                                   /* 0x2EF28 */
    f3B05C(PANEL_DLG);                                              /* 0x2EF32 */
    f459E4(n);                                                      /* 0x2EF3A */
    f3F670();                                                       /* 0x2EF3F */
    if (t >= 0)                                                     /* 0x2EF48 jl */
        f3F624(t, 15);                                              /* 0x2EF52 */
    if (t >= 0)                                                     /* 0x2EF5B jl */
        f3DA60(t);                                                  /* 0x2EF60 */
    else if (n <= 9)                                                /* 0x2EF6D jg */
        f3D9FC(n);                                                  /* 0x2EF72 */
    else
        f3D52C(0, 0);                                               /* 0x2EF7D */
    f3CBBC();                                                       /* 0x2EF82 */
    f78A20(0x5DC);                                                  /* 0x2EF8C */
}
