/* Module CAL2 of docs/M6_PLAN.md, the human side: the unit-loss calamities, their loss screens and the secondary
   losses. Literal translations; the address of each instruction a line comes from is in its comment. D = read in the
   code, G = inference. Prototypes and globals: cal2.h.

     0x25A38  f25A38  Famine (card 0x15)                                                     1,676 bytes
     0x27CEC  f27CEC  the human's Grain reduction of Famine losses                             736
     0x27FCC  f27FCC  NationLosesUnits: the human removes units                              3,376
     0x291D0  f291D0  the secondary losses shown                                               896
     0x29550  f29550  the human victim assigns the secondary losses                          1,860
     0x29C94  f29C94  0x29550's numbers                                                        416
     0x2D4F0  f2D4F0  Flood (card 0x19)                                                      3,296
     0x2E59C  f2E59C  Epidemic (card 0x1B)                                                   1,692
   0x2EC38 (the Epidemic target when only cities can go) is helpers2.c's f2EC38.

   Not ported here (each call site stops the port with not_ported(address); the static stubs below carry the names,
   so the call sites read as in the original and a later translation that exports one of these names collides with
   its stub at compile time instead of being shadowed by it). Built with -DCAL2_TEST (port/test/cal2_test.c) they go
   to the test's recorders instead, which return. The real signatures, for linking them later:
     M7 (AI):  void f27850(int n)                          the computer's Grain against Famine: 0x25DFE, 0x25FF7
               void f260C4(int n, int L, int kind, int s)  a computer loses L unit points (kind 0x15 / 0x19 / 0x1B, s
                                                           the flood plain or -1): 0x25E2E, 0x26098, 0x2DC4D, 0x2DF5D,
                                                           0x2E199, 0x2E1B9, 0x2E8CC, 0x2EBC5. Flood also calls it for
                                                           a HUMAN secondary victim that loses everything on the plain
                                                           (0x2DF5D) and for the Pirates (0x2E1B9), so a human Flood
                                                           with secondary units stops here too.
               void f27518(int n, int trader, int total, int kind)   a computer victim assigns the secondary losses:
                                                           0x25F7D, 0x2E0EF, 0x2E9FD
               int  f2E45C(int n)                          a computer's choice among tied flood plains: 0x2DB77
               int  f2E1D0(int n, int c)                   a computer's coastal city to lose to Flood (CAL1's row,
                                                           AI): 0x2D8C7
     ANIM:     void f7A4C4(int anim)                       the calamity animation f7A0A0([0xF783C]) when bit 0x04 of
                                                           [0xF097C] is set (anim stored, unused): 0x2859C, every first
                                                           redraw of 0x27FCC's map (unconditional, unlike 0x2468C's
                                                           phase test). So every human loss screen stops here until
                                                           ANIM lands, as cal1.c's 0x207B4 does.
     CAL3:     int  f1EAEC(int n, int kind)                the human's choice among the candidates of 0xF7CA0 (kind
                                                           0x19: the tied flood plains, returns the plain): 0x2DB0C
   (0x7A0A0 / 0x79ADC / 0x79C70 are reached through kit.c's f79270, which stops at them itself: every calamity
   message here passes anim 1, so with animations on the first message stops there.)
   So today, with animations off: Famine and Epidemic with human victims run up to the first NationLosesUnits screen
   (0x7A4C4); without a loss screen (Grain covers all, Medicine) they run through; the secondary screens 0x291D0 /
   0x29550 run. Flood stops at 0x7A4C4 or 0x260C4 as above, at 0x1EAEC on a human tie, at f207B4's 0x7A4C4 for the
   coastal city.

   game_rand (f78AE4): not called by these eight routines (D: no call of 0x78AE4 in them).

   Play by e-mail (dropped by Erik's ruling): no PBEM routine is called here. The bit-0x80 tests of byte [0xE1624]
   are kept literally (as in cal1.c): they only send a PBEM human down the computer's path.

   Watcom runtime -> standard C:
     0x7DB3D sprintf -> snprintf bounded to the stack buffer (100 bytes in each routine: the locals' layout); the
             formats are written out with their addresses in the comments
     0x8C897 itoa(v, buf, 10) -> snprintf "%d" (32 bytes in 0x291D0 / 0x29C94, 100 elsewhere)
     0x7D5D0 memset -> memset on the data object
     0x822B5 malloc -> load_heap_alloc (port handles in the 32-bit fields, game.h); 0x877E6 free -> load_heap_free
     rep movsd / movsw / movsb record copies -> memcpy between g_ds and the block (load_heap_ptr), or within g_ds
     fg_mousepos(0xE1628, 0xE162C, 0xE1630) -> fg_mousepos into locals, stored to the three dwords
   __CHK dropped; 0x78B0C (the debug trace, "NationLosesUnits" 0xB2C28) is a no-op and dropped.

   Data (D, read in the code at the addresses given):
     nation record n: 0xF14CC + 199n (rules.h): +0x08 int8 Grain cards; the advances +0x24 Pottery, +0x27
       Agriculture (int8, added to the city's limit), +0x28 Roadbuilding, +0x2A Engineering, +0x2D Medicine; +0x51
       int8 cities; +0x60 int8 areas with tokens; +0x98 int8 tokens on the board; +0x99 int8 stock; +0x9A 0 out / 2
       computer / 3 human (nation 10, the Pirates: 3, game.h)
     territory record t: 0xE1638 + 35t: +0 bit 0x02 coastal, +1 bits 3..5 the population limit, +1 bit 0x80 a city
       there is on the flood plain (G: what the bit means; the code counts such a city as 5 flooded units)
     territory state t: 0xE2C18 + 3t (+0 owner, +2 bit 0x01 city); area record r = t + 1: 0xE31B8 + 69r, +0x0E + n
       int8 tokens of n
     names 0xC768C[n] (long), 0xC7664[n] (short, 10 entries: read for n 1..9 only), territory names 0xC77D8[t]
     token sprites 0xF489C + 8n (18 x 18, w at +4), arrows 0xF51AC / 0xF51B4 (0x27CEC, 0x27FCC), small arrows 0xF51BC /
       0xF51C4 (0x29550)
     dialogs: 0xF596C the map screen's panel (count int16, base count int32 [0xF5944], records from 0xF596E);
       0xBF264 (0x27CEC: arrows 0x39 / 0x3A, FINISHED 0x2A); 0xBF650 (0x27FCC, rewritten per call: the two REMOVE
       buttons' x, REDUCE CITY's style and enabled byte); 0xBB800 (CONTINUE, moved by 0x291D0); 0xBFA3C (0x29550's
       CONTINUE at 319, 430, never removed: cut off with the count); the +/- templates 0xC0614 (id 0xD3, w 7) and
       0xC0600 (id 0xCC)
     [0xF52C8] the back buffer, [0xF52CC] the background buffer, int8 [0xEE168] its picture; byte [0xF0980] bit 0x04
       (computer messages wait for CONTINUE), byte [0xF097C] bit 0x08 (restart confirmations); int16 [0xF40BA] the
       self-running demo

   1995 bugs, fixed unless g_original (each marked BUG(1995) in the code; grades code / effect):
     1. 0x2D4F0 Flood, more than 10 secondary units (D / D): each nation m with assigned losses gets the human's
        NationLosesUnits or the computer's 0x260C4 by the VICTIM's +0x9A ([ebp-0x1C] + 0x9A, 0x2E12B) where m's is
        meant (the <= 10 branch, 0x2DFB7, tests m). In a mixed game a human victim would choose a computer's losses on
        its screen, and a computer victim's human neighbours would lose units chosen by the AI routine. Fixed: m's
        flag. No difference in an all-human or all-computer game.
     2. 0x27FCC with an empty choice list (D / G): the box prints entry 0 of 0xF7CA0, which is -1 ("PIRACY", 0xC77D4)
        when f28CFC found nothing (an Epidemic victim with no units: L = 0), and f3F624(-1, 15) writes 15 into byte
        0xFFB5F, the top byte of the heap pointer [0xFFB5C] (in the port a handle, which that write breaks). Fixed as
        cal1.c's 0x207B4 / 0x2468C (its bug 6): no name and no highlight while the list is empty.
     3. 0x27FCC after the last listed territory was taken out (D / G): the list shift leaves the old entry in slot 0
        with the count 0, and REMOVE ONE / REMOVE ALL / REDUCE CITY stay enabled while left > 0 (they test left and
        FINISHED, not the count). They then act on that stale territory: REMOVE ONE on an empty one takes a token
        that is not there (n's count there -1 = 0xFF, board - 1, stock + 1). G: reachable when a REDUCE CITY placed
        fewer tokens than the limit it was credited for (stock short, 0x78FEC places min(stock, limit)), so the units
        run out before left does. Fixed: the three buttons are disabled while the list is empty (RESTART is left).
   Kept as the original (D code, G effect unless marked):
     - Flood's tie branch for a computer leaves the plain uninitialised when fewer than two plains are listed
       (0x2DB72); with ties > 0 at least two are (D). Port: it starts as the last best plain.
     - Flood clears the message area's height to 0x15 at its start and never puts back 0x23 (Famine and Epidemic do,
       0x260B1, 0x2EC25); the next calamity's messages inherit it (kit.c's f79270 sets y / h during each call).
     - Flood counts the trader (the record's giver byte) among the secondary units and then exempts it in the <= 10
       branch only; G: Flood cannot be traded (a non-tradable calamity, game.h's stacks), so the giver is always -1.
     - 0x27FCC's REDUCE (and Epidemic's city removal) credits 5 - (limit + Agriculture +0x27) whatever 0x78FEC placed
       (it adds Agriculture only when n is alone there and places at most the stock); with limit + Agriculture over 5
       left grows. The "left != 0" token loops then remove every token of the territory (the inventory's risk, G).
     - Epidemic: a secondary victim with Medicine and Roadbuilding goes -5 (floor 0) then +5 (0x2EA54, 0x2EA89), so
       an assignment of 1..4 becomes 5; the "saved by MEDICINE" message shows the PRIMARY victim's panel and home view
       (edx = [ebp-0x34], 0x2EBFB). For Erik (game rules / display, not obvious slips).
     - Famine: a victim with no units at all (loss 0) gets "Your usage of Grain has prevented Famine in %s." even
       without Pottery or Grain (0x25BD0, 0x25E35).
     - 0x29550's buttons have ids 0xCC + j / 0xD3 + j, so an 8th column's plus (0xD3) would act as column 0's minus;
       G: unreachable, at most 8 players, so at most 7 columns besides the victim.
     - 0x29550's columns divide 300 by the count: no nation would divide by 0 (D); every caller has one (G: the
       callers call it only when the total exceeds the amount assigned, so some 0xF796C[m] != 0).
     - 0x291D0 and 0x29550 write the CONTINUE dialogs' positions into the data object (they persist).
   Text (Erik's ruling of 19 Nov, obvious errors only, TEXT_SWEEP.md): the river name "TIGRUS-EUPHRATES" (0xB36D4)
   printed by Flood's two messages, fixed to "TIGRIS-EUPHRATES" unless g_original (CAL3's 0x1EAEC and M7's 0x2E45C
   print the same table entry: they need the same fix). Kept as style: "The %s river Floods." (capital), "Assignments
   for Epidemic secondary losses." on the human path and without the period on the computer's (0xB35EF / 0xB3644),
   "...by MEDICINE.".
   Easter eggs / oddities: the trace name "NationLosesUnits" (known); duplicated strings the compiler did not merge
   ("Famine ravages %s.", "Your usage of Grain...", "Assignments for Famine secondary losses", "Flood also ravages
   %s...", "LEFT TO", "GRAIN/USED/LOSSES/REDUCED TO" (also in 0x27850)); the flood-plain table names the Rhone, Po,
   Danube, Tigris-Euphrates and Nile, and a city "on the flood plain" counts as 5 flooded units. */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "anim.h"
#include "cal1.h"
#include "cal2.h"
#include "cal3.h"
#include "fg.h"
#include "game.h"
#include "helpers2.h"
#include "kit.h"
#include "map.h"
#include "mapdata.h"
#include "move.h"
#include "panel.h"
#include "rules.h"
#include "start.h"
#include "state.h"
#include "ui.h"

#define NATION(n)    (0xF14CCu + 199u * (uint32_t)(n))
#define NAME(n)      DSTR(D32(0xC768Cu + 4u * (uint32_t)(n)))
#define SHORTNAME(n) DSTR(D32(0xC7664u + 4u * (uint32_t)(n)))
#define TERR_NAME(t) DSTR(D32(0xC77D8u + 4u * (uint32_t)(t)))
#define TERR(t)      (0xE1638u + 35u * (uint32_t)(t))
#define TSTATE(t)    (0xE2C18u + 3u * (uint32_t)(t))
#define AREA(r)      (0xE31B8u + 69u * (uint32_t)(r))
#define CAL_GIVER(k) (0xF77E5u + 3u * (uint32_t)(k))   /* int8 (dword [0xF77E2 + 3k] sar 24) */
#define CAL_VICTIM(k) (0xF77E6u + 3u * (uint32_t)(k))  /* int8 (dword [0xF77E3 + 3k] sar 24) */
#define PLAIN(s)     (0xBEE08u + 22u * (uint32_t)(s))   /* +0 int16 units, +2 int16 count, +4 char *, +8 int16[7] */
#define FLAGS        0xE1624u
#define PBEM_BIT     0x80u
#define CUR_NATION   0xF40F8u
#define PANEL_DLG    0xF596Cu
#define PANEL_BASE   0xF5944u   /* int32 */
#define GRAIN_DLG    0xBF264u
#define LOSS_DLG     0xBF650u
#define CONT_DLG     0xBB800u
#define ASSIGN_DLG   0xBFA3Cu
#define BACKBUF      0xF52C8u
#define BACKGROUND   0xF52CCu
#define BGNUM        0xEE168u
#define CHOICE       0xF7CA0u   /* int32[157] */
#define CHOICE_N     0xF7C9Cu   /* int32 */
#define SEC_CNT      0xF796Cu   /* int16[11] */
#define SEC_LOSS     0xF547Cu   /* uint8[14] */
#define GRAIN_USED   0xF409Au   /* int16[10] */
#define SNAP_AREA    0xEED74u
#define SNAP_NATION  0xEED78u
#define SNAP_TSTATE  0xEED7Cu
#define MOUSE_X      0xE1628u
#define MOUSE_Y      0xE162Cu
#define MOUSE_B      0xE1630u

/* ---- calls into code that is not ported yet (see the file comment) ----------------------------------------------- */

#ifdef CAL2_TEST
void cal2_test_f27850(int n);
void cal2_test_f260C4(int n, int L, int kind, int s);
void cal2_test_f27518(int n, int trader, int total, int kind);
int  cal2_test_f2E45C(int n);
int  cal2_test_f2E1D0(int n, int c);
void cal2_test_f7A4C4(int anim);
int  cal2_test_f1EAEC(int n, int kind);
#define f27850(n)                  cal2_test_f27850(n)
#define f260C4(n, L, kind, s)      cal2_test_f260C4(n, L, kind, s)
#define f27518(n, tr, total, kind) cal2_test_f27518(n, tr, total, kind)
#define f2E45C(n)                  cal2_test_f2E45C(n)
#define f2E1D0(n, c)               cal2_test_f2E1D0(n, c)
#define f7A4C4(anim)               cal2_test_f7A4C4(anim)
#define f1EAEC(n, kind)            cal2_test_f1EAEC(n, kind)
#else
#include "aical2.h"                             /* f27850, f260C4, f27518, f2E45C (M7) */
#include "aical1.h"                             /* f2E1D0 (M7) */

#endif

/* ---- small shared pieces ------------------------------------------------------------------------------------------ */

/* the background 3 into the back buffer: f3CD6C(3), byte [0xEE168] = 3 (stored after the call), fg_vbcopy of the
   whole screen from [0xF52CC] to [0xF52C8] (0x27D0E..0x27D43, 0x280ED..0x28122, 0x291EC..0x29221, 0x2956E..0x295A3) */
static void background3(void)
{
    f3CD6C(3);
    setD8(BGNUM, 3);
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BACKGROUND), S32(BACKBUF));
}

/* the human test the three calamities share: byte [0xE1624] bit 0x80 clear and +0x9A == 3 (dword [0xF1563 + 199n]
   sar 24) */
static int human(int n)
{
    return (D8(FLAGS) & PBEM_BIT) == 0 && S8(NATION(n) + 0x9Au) == 3;
}

/* a flood plain's river name as printed (port): the text fix unless g_original, "TIGRUS" (0xB36D4) -> "TIGRIS";
   shared with CAL3's 0x1EAEC (cal2.h) */
const char *cal2_river_text(const char *r)
{
    if (!g_original && strcmp(r, "TIGRUS-EUPHRATES") == 0)   /* BUG(1995) (text): "TIGRUS" (0xB36D4) */
        return "TIGRIS-EUPHRATES";
    return r;
}

/* the river name of plain s (the pointer at +4, dword [0xBEE0C + 22s]) */
static const char *river(int s)
{
    return cal2_river_text(DSTR(D32(PLAIN(s) + 4u)));
}

/* the snapshot of 0x27FCC: area records 0..160 (69 bytes), territory states 0..159 (3) and nations 1..13 (199, at
   199n) to the heap blocks [0xEED74] / [0xEED7C] / [0xEED78] (0x2801C..0x280E1) or back (0x28B4F..0x28C13: areas,
   nations, then territory states). Port guard: a block that is not there (out of memory) is skipped; the original
   would write through a NULL pointer */
static void loss_snapshot(int to_block)
{
    uint8_t *area = load_heap_ptr(D32(SNAP_AREA), 0x2B65u);
    uint8_t *ts = load_heap_ptr(D32(SNAP_TSTATE), 0x1E0u);
    uint8_t *nat = load_heap_ptr(D32(SNAP_NATION), 0xAE2u);
    uint32_t i;

    if (area != NULL) {
        for (i = 0; i < 0xA1u; i++) {                               /* 0x28027 / 0x28B5C */
            if (to_block)
                memcpy(area + 69u * i, DP(AREA(i)), 69);            /* 0x28061 rep movsd (0x11) + movsb */
            else
                memcpy(DP(AREA(i)), area + 69u * i, 69);            /* 0x28B92 */
        }
    }
    if (to_block) {
        if (ts != NULL)
            for (i = 0; i < 0xA0u; i++)                             /* 0x28073 */
                memcpy(ts + 3u * i, DP(TSTATE(i)), 3);              /* 0x280A2 movsw + movsb */
        if (nat != NULL)
            for (i = 1; i < 0xEu; i++)                              /* 0x280B6 */
                memcpy(nat + 199u * i, DP(NATION(i)), 199);         /* 0x280DC rep movsd (0x31) + movsw + movsb */
    } else {
        if (nat != NULL)
            for (i = 1; i < 0xEu; i++)                              /* 0x28BA6 */
                memcpy(DP(NATION(i)), nat + 199u * i, 199);         /* 0x28BCD */
        if (ts != NULL)
            for (i = 0; i < 0xA0u; i++)                             /* 0x28BE1 */
                memcpy(DP(TSTATE(i)), ts + 3u * i, 3);              /* 0x28C10 */
    }
}

/* one token of n off territory t (0x28715..0x28742, and its three twins): f57CE8(t, n, 0, 1), board - 1, stock + 1
   (byte dec / inc) */
static void lose_token(int32_t t, int n)
{
    f57CE8(t, n, 0, 1);                                             /* 0x28729 */
    setD8(NATION(n) + 0x98u, (uint8_t)(D8(NATION(n) + 0x98u) - 1u));   /* 0x28735 dec byte [0xF1564 + 199n] */
    setD8(NATION(n) + 0x99u, (uint8_t)(D8(NATION(n) + 0x99u) + 1u));   /* 0x28742 inc byte [0xF1565 + 199n] */
}

/* n's tokens in territory t (area record t + 1, dword [69(t+1) + n + 0xE31C3] sar 24) */
static int32_t tokens_in(int32_t t, int n)
{
    return S8(AREA(t + 1) + 0x0Eu + (uint32_t)n);
}

/* 5 - (the population limit of t + Agriculture of n): what a city reduction takes off (0x286A3..0x286DC) */
static int32_t reduce_credit(int32_t t, int n)
{
    int32_t limit = (int32_t)((D8(TERR(t) + 1u) >> 3) & 7u);        /* 0x286B1 shl al, 2; shr al, 5 */
    return 5 - (S8(NATION(n) + 0x27u) + limit);                     /* 0x286C8 dword [0xF14F0 + 199n] sar 24 */
}

/* ==== 0x25A38 Famine ============================================================================================== */

void f25A38(int k)
{
    char buf[100];       /* [ebp-0x90]..[ebp-0x2D] */
    int32_t n;           /* [ebp-0x24] */
    int32_t v;           /* [ebp-0x20] */
    int32_t loss;        /* [ebp-0x1C] */
    int32_t m;           /* [ebp-0x28] */
    int32_t j;           /* [ebp-0x18] */
    uint32_t rec;

    setD16(0xBB3FAu, 0xA9);                                         /* 0x25A64 */
    setD16(0xBB3FCu, 0x154);                                        /* 0x25A6B */
    setD16(0xBB3FEu, 0x12C);                                        /* 0x25A72 */
    setD16(0xBB400u, 0x15);                                         /* 0x25A79 */
    n = S8(CAL_VICTIM(k));                                          /* 0x25A8C dword [0xF77E3 + 3k] sar 24 */
    setD32(CUR_NATION, (uint32_t)n);                                /* 0x25A9B */
    rec = NATION(n);
    if (human(n)) {                                                 /* 0x25AA0, 0x25AC0 */
        snprintf(buf, sizeof buf, "Famine ravages %s.", NAME(n));   /* 0x25ADD (0xB2953) */
        f79270(-500, n, -200, buf, 1);                              /* 0x25AFA */
        snprintf(buf, sizeof buf, "%s, choose sites of Famine losses.", NAME(n));   /* 0x25B16 (0xB2966) */
        v = 5 * S8(rec + 0x51u) + S8(rec + 0x98u);                  /* 0x25B25..0x25B47 */
        loss = v > 10 ? 10 : v;                                     /* 0x25B4F jle */
        setD8(SEC_LOSS + (uint32_t)n, (uint8_t)loss);               /* 0x25B67 */
        if (D8(rec + 0x24u) != 0 && S8(rec + 0x08u) != 0)           /* 0x25B74 Pottery, 0x25B8D Grain cards */
            f27CEC(n);                                              /* 0x25B94 */
        loss = S8(SEC_LOSS + (uint32_t)n);                          /* 0x25B9C dword [0xF5479 + n] sar 24 */
        if (loss < 0)                                               /* 0x25BA7 jge */
            loss = 0;
        if (loss > 0)                                               /* 0x25BB2 jle */
            f27FCC(n, loss, buf, 0x15, 0, 0);                       /* 0x25BC9 */
        else {
            snprintf(buf, sizeof buf, "Your usage of Grain has prevented Famine in %s.", NAME(n));   /* 0x25BE7 (0xB2989) */
            f79270(-500, n, -200, buf, 1);                          /* 0x25C04 */
        }
        for (m = 1; m <= 9; m++) {                                  /* 0x25C09, 0x25C28 jg */
            setD16(SEC_CNT + 2u * (uint32_t)m, 0);                  /* 0x25C33 */
            if (D8(NATION(m) + 0x9Au) == 0)                         /* 0x25C42 */
                continue;
            if (m == n)                                             /* 0x25C4F */
                continue;
            v = 5 * S8(NATION(m) + 0x51u) + S8(NATION(m) + 0x98u);  /* 0x25C5B..0x25C7F */
            setD16(SEC_CNT + 2u * (uint32_t)m, (uint16_t)(v > 8 ? 8 : v));   /* 0x25C86 jle, 0x25C1A */
        }
        v = 0;                                                      /* 0x25C91 */
        for (m = 1; m <= 9; m++)                                    /* 0x25C94 */
            v += S16(SEC_CNT + 2u * (uint32_t)m);                   /* 0x25CAC dword [0xF796A + 2m] sar 16 */
        if (v <= 0x14) {                                            /* 0x25CBB jg 0x25D02 */
            for (m = 1; m <= 9; m++)                                /* 0x25CC1 */
                setD8(SEC_LOSS + (uint32_t)m, D8(SEC_CNT + 2u * (uint32_t)m));   /* 0x25CD9 (the low byte) */
            f291D0(n, -1, DSTR(0xB29B9u));                          /* 0x25CF8 "Assignments for Famine secondary losses" */
        } else {
            snprintf(buf, sizeof buf, "%s, assign secondary losses for Famine.", NAME(n));   /* 0x25D19 (0xB29E1) */
            f29550(n, -1, 0x14, buf);                               /* 0x25D34 */
        }
    } else {
        snprintf(buf, sizeof buf, "Famine ravages %s.", NAME(n));   /* 0x25D5C (0xB2A09) */
        f79270(-500, n, -200, buf, 1);                              /* 0x25D79 */
        f3A990();                                                   /* 0x25D7E */
        f3CBBC();                                                   /* 0x25D83 */
        v = 5 * S8(rec + 0x51u) + S8(rec + 0x98u);                  /* 0x25D88..0x25DB1 */
        loss = v > 10 ? 10 : v;                                     /* 0x25DBC jle */
        setD8(SEC_LOSS + (uint32_t)n, (uint8_t)loss);               /* 0x25DD1 */
        if (D8(rec + 0x24u) != 0 && S8(rec + 0x08u) != 0)           /* 0x25DDE, 0x25DF7 */
            f27850(n);                                              /* 0x25DFE [M7] */
        loss = S8(SEC_LOSS + (uint32_t)n);                          /* 0x25E06 */
        if (loss < 0)                                               /* 0x25E11 */
            loss = 0;
        if (loss > 0)                                               /* 0x25E1C */
            f260C4(n, loss, 0x15, -1);                              /* 0x25E2E [M7] */
        else {
            snprintf(buf, sizeof buf, "Your usage of Grain has prevented Famine in %s.", NAME(n));   /* 0x25E4C (0xB2A1C) */
            f79270(-500, n, -200, buf, 1);                          /* 0x25E69 */
        }
        for (m = 1; m <= 9; m++) {                                  /* 0x25E6E, 0x25E8D jg */
            setD16(SEC_CNT + 2u * (uint32_t)m, 0);                  /* 0x25E98 */
            if (D8(NATION(m) + 0x9Au) == 0)                         /* 0x25EA7 */
                continue;
            if (m == n)                                             /* 0x25EB4 */
                continue;
            v = 5 * S8(NATION(m) + 0x51u) + S8(NATION(m) + 0x98u);  /* 0x25EB9..0x25EE4 */
            setD16(SEC_CNT + 2u * (uint32_t)m, (uint16_t)(v > 8 ? 8 : v));   /* 0x25EEB, 0x25E7F */
        }
        v = 0;                                                      /* 0x25EF6 */
        for (m = 1; m <= 9; m++)                                    /* 0x25EF9 */
            v += S16(SEC_CNT + 2u * (uint32_t)m);                   /* 0x25F11 */
        if (v <= 0x14) {                                            /* 0x25F20 jg 0x25F6B */
            for (m = 1; m <= 9; m++)                                /* 0x25F26 */
                setD8(SEC_LOSS + (uint32_t)m, D8(SEC_CNT + 2u * (uint32_t)m));   /* 0x25F48 */
            f291D0(n, -1, DSTR(0xB2A4Cu));                          /* 0x25F5D "Assignments for Famine secondary losses" */
        } else
            f27518(n, -1, 0x14, 0x15);                              /* 0x25F7D [M7] */
    }
    for (j = 1; j <= 9; j++) {                                      /* 0x25F82 / 0x25D39 / 0x25F62, 0x25F91 jg */
        if (D8(SEC_LOSS + (uint32_t)j) != 0                         /* 0x25FA4 */
            && D8(NATION(j) + 0x24u) != 0                           /* 0x25FAF Pottery */
            && S8(NATION(j) + 0x08u) != 0) {                        /* 0x25FC8 Grain cards */
            if (human(j))                                           /* 0x25FD1, 0x25FE8 */
                f27CEC(j);                                          /* 0x25FED */
            else {
                f27850(j);                                          /* 0x25FF7 [M7] */
                f78A20(0x7D0);                                      /* 0x26001 */
            }
        }
        if (S8(SEC_LOSS + (uint32_t)j) <= 0)                        /* 0x26014 jle */
            continue;
        if (human(j)) {                                             /* 0x26023, 0x26038 */
            snprintf(buf, sizeof buf, "Famine also ravages %s.\nChoose sites of losses.", NAME(j));   /* 0x26051 (0xB2A74) */
            f27FCC(j, S8(SEC_LOSS + (uint32_t)j), buf, 0x15, 0, 0); /* 0x26077 */
        } else
            f260C4(j, S8(SEC_LOSS + (uint32_t)j), 0x15, -1);        /* 0x2609A [M7] */
    }
    f1E69C(k);                                                      /* 0x260AC */
    setD16(0xBB400u, 0x23);                                         /* 0x260B1 */
}

/* ==== 0x27CEC: the human's Grain against Famine =================================================================== */

void f27CEC(int n)
{
    char buf[100];       /* [ebp-0x80]..[ebp-0x1D] */
    int32_t r;           /* [ebp-0x18] */
    int32_t loss;

    f7853C(GRAIN_DLG);                                              /* 0x27D09 */
    background3();                                                  /* 0x27D0E..0x27D43 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x27D52 */
    f3AF8C(0xF51ACu, 0x128, 0x181);                                 /* 0x27D69 */
    f3AF8C(0xF51B4u, 0x14A, 0x181);                                 /* 0x27D7D */
    fg_setcolor(5);                                                 /* 0x27D84 */
    f3A3A0(0x125, 0x185, DSTR(0xB2BDDu));                           /* 0x27D9B "GRAIN" */
    f3A044(0x15A, 0x185, DSTR(0xB2BE3u));                           /* 0x27DAF "USED" */
    f3A3A0(0x12F, 0x199, DSTR(0xB2BE8u));                           /* 0x27DC3 "LOSSES" */
    f3A044(0x150, 0x199, DSTR(0xB2BEFu));                           /* 0x27DD7 "REDUCED TO" */
    fg_vbclose();                                                   /* 0x27DDC */
    f7876C();                                                       /* 0x27DE1 */
    snprintf(buf, sizeof buf, "%s - choose Grain reduction of Famine losses.", NAME(n));   /* 0x27DFA (0xB2BFA) */
    f3A9F4(buf);                                                    /* 0x27E05 */
    f3B05C(PANEL_DLG);                                              /* 0x27E0F */
    setD16(GRAIN_USED + 2u * (uint32_t)n, 0);                       /* 0x27E19 */
    f459E4(n);                                                      /* 0x27E24 */
    f3F670();                                                       /* 0x27E29 */
    f3D9FC(n);                                                      /* 0x27E31 */
    for (;;) {
        fg_vbopen(S32(BACKBUF));                                    /* 0x27E3D */
        f3C9D0(0x136, 0x181, 0x13, 0x13, 5);                        /* 0x27E58 */
        fg_setcolor(5);                                             /* 0x27E5F */
        snprintf(buf, sizeof buf, "%d", (int)S16(GRAIN_USED + 2u * (uint32_t)n));   /* 0x27E7F itoa, dword [0xF4098 + 2n] sar 16 */
        f3A0B0(0x136, 0x185, 0x13, buf);                            /* 0x27E93 */
        f3C9D0(0x136, 0x195, 0x13, 0x13, 5);                        /* 0x27EAB */
        fg_setcolor(5);                                             /* 0x27EB2 */
        loss = S8(SEC_LOSS + (uint32_t)n);                          /* 0x27EC5 */
        if (loss < 0)                                               /* 0x27ED0 jge */
            loss = 0;
        snprintf(buf, sizeof buf, "%d", loss);                      /* 0x27ED4 itoa */
        f3A0B0(0x136, 0x199, 0x13, buf);                            /* 0x27EEB */
        fg_vbclose();                                               /* 0x27EF0 */
        f3CBBC();                                                   /* 0x27EF5 */
        r = f78BA4(PANEL_DLG, 0);                                   /* 0x27F01 */
        if (r == -100)                                              /* 0x27F09 */
            continue;
        if (r == -1)                                                /* 0x27F16 je 0x27FB8 */
            break;
        if ((uint32_t)r < (uint32_t)-0xA)                           /* 0x27F25 jb */
            continue;
        if (r == -0xA) {                                            /* 0x27F2B jbe 0x27F65: one card more */
            if (S8(SEC_LOSS + (uint32_t)n) > 0                      /* 0x27F71 jle */
                && S16(GRAIN_USED + 2u * (uint32_t)n) < S8(NATION(n) + 0x08u)) {   /* 0x27F96 jge (dword [0xF14D1 + 199n] sar 24) */
                setD16(GRAIN_USED + 2u * (uint32_t)n, (uint16_t)(D16(GRAIN_USED + 2u * (uint32_t)n) + 1u));   /* 0x27FA1 */
                setD8(SEC_LOSS + (uint32_t)n, (uint8_t)(D8(SEC_LOSS + (uint32_t)n) - 4u));   /* 0x27FAC sub byte, 4 */
            }
        } else if (r == -9) {                                       /* 0x27F2D: one card less */
            if (S16(GRAIN_USED + 2u * (uint32_t)n) > 0) {           /* 0x27F45 jle */
                setD16(GRAIN_USED + 2u * (uint32_t)n, (uint16_t)(D16(GRAIN_USED + 2u * (uint32_t)n) - 1u));   /* 0x27F4E */
                setD8(SEC_LOSS + (uint32_t)n, (uint8_t)(D8(SEC_LOSS + (uint32_t)n) + 4u));   /* 0x27F59 add byte, 4 */
            }
        }
    }
    f78664(GRAIN_DLG);                                              /* 0x27FBD */
}

/* ==== 0x27FCC NationLosesUnits ==================================================================================== */

void f27FCC(int n, int L, const char *text, int kind, int sub, int target)
{
    char buf[100];       /* [ebp-0x98]..[ebp-0x35] */
    int32_t left;        /* [ebp-0x10] */
    int32_t sel;         /* [ebp-0x18] (its address goes to f79A9C) */
    int32_t redraw;      /* [ebp-0x1C] */
    int32_t removed;     /* [ebp-0x14] */
    int32_t fin;         /* [ebp-0xC] */
    int32_t r;           /* [ebp-0x20] */
    int32_t i;           /* [ebp-0x24] */
    int32_t t;
    int empty;           /* port: the list is empty (BUG(1995) 2 and 3) */

    setD32(SNAP_AREA, load_heap_alloc(0x2B65u));                    /* 0x27FF2 malloc, 0x27FF7 */
    setD32(SNAP_TSTATE, load_heap_alloc(0x1E0u));                   /* 0x28001, 0x28006 */
    setD32(SNAP_NATION, load_heap_alloc(0xAE2u));                   /* 0x28012, 0x28017 */
    loss_snapshot(1);                                               /* 0x2801C..0x280E1 */
    /* 0x280E8 0x78B0C("NationLosesUnits"): the no-op debug trace, dropped */
    background3();                                                  /* 0x280ED..0x28122 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x28131 */
    f3AF8C(0xF51ACu, 0xFB, 0x186);                                  /* 0x28148 */
    f3AF8C(0xF51B4u, 0x178, 0x186);                                 /* 0x2815C */
    fg_setcolor(5);                                                 /* 0x28163 */
    f3A3A0(0x12F, 0x176, DSTR(0xB2C39u));                           /* 0x2817A "LEFT TO" */
    f3A044(0x150, 0x176, DSTR(0xB2C41u));                           /* 0x2818E "REMOVE" */
    fg_vbclose();                                                   /* 0x28193 */
    f3AB7C(text);                                                   /* 0x2819B */
    f7876C();                                                       /* 0x281A0 */
    if (kind == 7 || kind == 8 || kind == 9) {                      /* 0x281A5..0x281B5 (Civil War) */
        f3BB48(LOSS_DLG, 0x3F, 0xDB);                               /* 0x28213 */
        f3BB48(LOSS_DLG, 0x40, 0x13F);                              /* 0x28227 */
        f3C0D8(LOSS_DLG, 0x3E, 2);                                  /* 0x2823B */
        f3BFBC(LOSS_DLG, 0x3E, 0);                                  /* 0x2824C */
    } else {
        f3BB48(LOSS_DLG, 0x3F, 0xA9);                               /* 0x281C6 */
        f3BB48(LOSS_DLG, 0x40, 0x10D);                              /* 0x281DA */
        f3C0D8(LOSS_DLG, 0x3E, 1);                                  /* 0x281EE */
        f3BFBC(LOSS_DLG, 0x3E, 1);                                  /* 0x2824C */
    }
    f7853C(LOSS_DLG);                                               /* 0x28256 */
    f3B05C(PANEL_DLG);                                              /* 0x28260 */
    left = L;                                                       /* 0x28268 */
    f28CFC(n, kind, sub);                                           /* 0x28274 */
    if (target != 0) {                                              /* 0x28279 je 0x28304 */
        fg_vbopen(S32(BACKBUF));                                    /* 0x2828A */
        fg_setcolor(5);                                             /* 0x28294 */
        f3A3A0(0x1B7, 0x176, DSTR(0xB2C48u));                       /* 0x282B0 "TARGET" */
        f3C9D0(0x1BC, 0x172, 0x13, 0x13, 5);                        /* 0x282C3 */
        fg_setcolor(5);                                             /* 0x282CA */
        snprintf(buf, sizeof buf, "%d", target);                    /* 0x282E6 itoa */
        f3A0B0(0x1BC, 0x176, 0x13, buf);                            /* 0x282FA */
        fg_vbclose();                                               /* 0x282FF */
    }
    redraw = 1;                                                     /* 0x28304 */
    sel = 0;                                                        /* 0x2830D */
    removed = 0;                                                    /* 0x28310 */
    for (;;) {
        /* 0x28313 */
        empty = !g_original && S32(CHOICE_N) <= 0;
        fg_vbopen(S32(BACKBUF));                                    /* 0x28319 */
        f3C9D0(0x109, 0x186, 0x6E, 0x13, 5);                        /* 0x28337 */
        fg_setcolor(5);                                             /* 0x2833E */
        if (!empty)   /* BUG(1995) 2: an empty list shows entry -1, "PIRACY" (0xC77D4) */
            f3A0B0(0x109, 0x18A, 0x6E, TERR_NAME(S32(CHOICE + 4u * (uint32_t)sel)));   /* 0x28366 */
        f3C9D0(0x136, 0x172, 0x13, 0x13, 5);                        /* 0x2837E */
        fg_setcolor(5);                                             /* 0x28385 */
        snprintf(buf, sizeof buf, "%d", left);                      /* 0x283A1 itoa */
        f3A0B0(0x136, 0x176, 0x13, buf);                            /* 0x283B5 */
        fg_vbclose();                                               /* 0x283BA */
        f3F670();                                                   /* 0x283BF */
        if (!empty)   /* BUG(1995) 2: an empty list writes 15 into byte 0xFFB5F (f3F624(-1)) */
            f3F624(S32(CHOICE + 4u * (uint32_t)sel), 15);           /* 0x283D3 */
        if (left != target)                                         /* 0x283E0 je 0x283F3 */
            f3C3D0(PANEL_DLG, 0x2A);                                /* 0x283EC FINISHED off */
        else
            f3C314(PANEL_DLG, 0x2A);                                /* 0x283FD FINISHED on */
        fin = f3BF30(PANEL_DLG, 0x2A);                              /* 0x2840C, 0x28411 */
        t = S32(CHOICE + 4u * (uint32_t)sel);
        if (fin == 0 && left > 0                                    /* 0x28418, 0x28422 jle 0x2849B */
            && (kind != 0x1B                                        /* 0x2842E jne 0x2847B */
                || tokens_in(t, n) > 1                              /* 0x2845D jg 0x2847B */
                || (D8(TSTATE(t) + 2u) & 1) != 0)                   /* 0x28472 test byte [.. + 0xE2C1A], 1 */
            && !empty) {      /* BUG(1995) 3: REMOVE acted on the stale slot 0 of an emptied list */
            f3C314(PANEL_DLG, 0x3F);                                /* 0x28485 REMOVE ONE on */
            f3C314(PANEL_DLG, 0x40);                                /* 0x28494 REMOVE ALL on */
        } else {
            f3C3D0(PANEL_DLG, 0x3F);                                /* 0x284A5 */
            f3C3D0(PANEL_DLG, 0x40);                                /* 0x284B4 */
        }
        if (kind != 7 && kind != 8) {                               /* 0x284B9, 0x284BF je 0x28514 */
            if (f3BF30(PANEL_DLG, 0x2A) == 0                        /* 0x284CF, 0x284D6 jne 0x28505 */
                && (D8(TSTATE(t) + 2u) & 1) != 0                    /* 0x284EB */
                && !empty)    /* BUG(1995) 3 */
                f3C314(PANEL_DLG, 0x3E);                            /* 0x284FE REDUCE CITY on */
            else
                f3C3D0(PANEL_DLG, 0x3E);                            /* 0x2850F */
        }
        if (redraw != 0) {                                          /* 0x28514 je 0x28546 */
            f459E4(n);                                              /* 0x2851D */
            if (S32(CHOICE_N) > 0)                                  /* 0x2852A jle */
                f3DA60(S32(CHOICE + 4u * (uint32_t)sel));           /* 0x28536 */
            else
                f3D52C(0, 0);                                       /* 0x28541 */
        }
        if (left < target) {                                        /* 0x2854C jge 0x2858A: too many removed */
            f3C3D0(PANEL_DLG, 0x3F);                                /* 0x28558 */
            f3C3D0(PANEL_DLG, 0x40);                                /* 0x28567 */
            f3C3D0(PANEL_DLG, 0x3E);                                /* 0x28576 */
            f3C3D0(PANEL_DLG, 0x2A);                                /* 0x28585 */
        }
        f3CBBC();                                                   /* 0x2858A */
        if (redraw != 0) {                                          /* 0x2858F */
            f7A4C4(-600);                                           /* 0x2859C (eax 0xFFFFFDA8) [ANIM] */
            redraw = 0;                                             /* 0x285A1 */
        }
        r = f78BA4(PANEL_DLG, 1);                                   /* 0x285AE */
        if (r == -100)                                              /* 0x285B6 */
            continue;
        if (r >= 0) {                                               /* 0x285C5 jl 0x285D9 */
            sel = r;                                                /* 0x285CA a map click on a listed territory */
            redraw = 1;                                             /* 0x285CD */
            continue;
        }
        if (r == -1)                                                /* 0x285DC je 0x28CB4 */
            break;
        t = S32(CHOICE + 4u * (uint32_t)sel);
        if ((uint32_t)r < (uint32_t)-0x63) {                        /* 0x285EB jb 0x28C42 */
            if ((uint32_t)r < (uint32_t)-0x66) {                    /* 0x28C45 jb */
                /* nothing */
            } else if (r == -0x66) {                                /* 0x28C50 jbe 0x2880D: REMOVE ALL */
                if ((D8(TSTATE(t) + 2u) & 1) != 0) {                /* 0x2881C */
                    if (kind != 0x1B) {                             /* 0x2882F je 0x2890A */
                        f78FEC(t, n);                               /* 0x28842 */
                        left -= reduce_credit(t, n);                /* 0x28888 */
                        while (tokens_in(t, n) > 0 && left != 0) {  /* 0x288B0 jle, 0x288B6 je */
                            left--;                                 /* 0x288BB */
                            lose_token(t, n);                       /* 0x288D2..0x288EB */
                        }
                        redraw = 1;                                 /* 0x288F8 */
                        removed = 1;                                /* 0x288FB */
                        redraw = 1;                                 /* 0x288FE */
                    } else {
                        f78FEC(t, n);                               /* 0x28917 */
                        left -= reduce_credit(t, n);                /* 0x28960 */
                        while (tokens_in(t, n) > 1 && left != 0) {  /* 0x28989 jle, 0x2898F je */
                            left--;                                 /* 0x28994 */
                            lose_token(t, n);                       /* 0x289AB..0x289C4 */
                        }
                        redraw = 1;                                 /* 0x289CC, 0x289D3 */
                    }
                } else {
                    /* 0x289DF: tokens until left is 0 (Epidemic keeps one) */
                    while (left != 0) {                             /* 0x289E3 je 0x28A80 */
                        if (kind != 0x1B) {                         /* 0x289ED je 0x28A14 */
                            if (D8(AREA(t + 1) + 0x0Eu + (uint32_t)n) == 0)   /* 0x28A0B cmp byte [.. + 0xE31C6], 0 */
                                break;                              /* -> 0x28A14 -> 0x28A80 */
                        } else if (tokens_in(t, n) <= 1)            /* 0x28A3D jle 0x28A80 */
                            break;
                        left--;                                     /* 0x28A45 */
                        lose_token(t, n);                           /* 0x28A5C..0x28A75 */
                    }
                    if (tokens_in(t, n) == 0)                       /* 0x28AA3 jne */
                        removed = 1;                                /* 0x28AA7 */
                    redraw = 1;                                     /* 0x28AAE */
                }
            } else if (r == -0x65) {                                /* 0x28C59 je 0x28648: REMOVE ONE */
                if ((D8(TSTATE(t) + 2u) & 1) != 0) {                /* 0x28657 */
                    if (kind != 0x1B) {                             /* 0x2866A je 0x28687 */
                        f78F04(t, n);                               /* 0x28679 the city eliminated */
                        left -= 5;                                  /* 0x2867E */
                        removed = 1;                                /* 0x287FA */
                        redraw = 1;                                 /* 0x28801 */
                    } else {
                        f78FEC(t, n);                               /* 0x28694 Epidemic: reduced */
                        left -= reduce_credit(t, n);                /* 0x286DC */
                        while (tokens_in(t, n) > 1 && left != 0) {  /* 0x28707 jle, 0x2870D je */
                            left--;                                 /* 0x28712 */
                            lose_token(t, n);                       /* 0x28729..0x28742 */
                        }
                        redraw = 1;                                 /* 0x2874A, 0x28751 */
                    }
                } else if (kind != 0x1B || tokens_in(t, n) > 1) {   /* 0x28763 jne 0x28798; 0x28796 jle 0x28801 */
                    left--;                                         /* 0x2879B */
                    lose_token(t, n);                               /* 0x287B2..0x287CB */
                    if (tokens_in(t, n) == 0)                       /* 0x287F8 jne 0x28801 */
                        removed = 1;                                /* 0x287FA */
                    redraw = 1;                                     /* 0x28801 */
                } else
                    redraw = 1;                                     /* 0x28801 */
            }
        } else if (r == -0x63) {                                    /* 0x285F1 jbe 0x28B1C: RESTART */
            snprintf(buf, sizeof buf, "Are you sure you want to Restart?");   /* 0x28B28 (0xB2C4F) */
            if ((D8(0xF097Cu) & 0x08) == 0 || f795A8(buf) == -0x61) {   /* 0x28B36, 0x28B41, 0x28B46 */
                loss_snapshot(0);                                   /* 0x28B4F..0x28C13 */
                left = L;                                           /* 0x28C18 */
                f28CFC(n, kind, sub);                               /* 0x28C26 */
                sel = 0;                                            /* 0x28C2B */
                redraw = 1;                                         /* 0x28C2E */
            }
        } else if ((uint32_t)r < (uint32_t)-0xA) {                  /* 0x285FA jb 0x28C37 */
            if (r == -0xE) {                                        /* 0x28C3A je 0x28ABA: REDUCE CITY */
                f78FEC(t, n);                                       /* 0x28AC7 */
                left -= reduce_credit(t, n);                        /* 0x28B0D */
                redraw = 1;                                         /* 0x28B10 */
            }
        } else if (r == -0xA) {                                     /* 0x28600 jbe 0x28627 */
            if (S32(CHOICE_N) - 1 > sel) {                          /* 0x2862D cmp; jle */
                sel++;                                              /* 0x28639 */
                redraw = 1;                                         /* 0x2863C */
            }
        } else if (r == -9) {                                       /* 0x28602 jne 0x28C5F */
            if (sel > 0) {                                          /* 0x2860F jle */
                sel--;                                              /* 0x28618 */
                redraw = 1;                                         /* 0x2861B */
            }
        }
        /* 0x28C5F */
        if (removed != 0) {                                         /* 0x28C63 je 0x28313 */
            for (i = sel; S32(CHOICE_N) - 1 > i; i++)               /* 0x28C69, 0x28C7D cmp; jle */
                setD32(CHOICE + 4u * (uint32_t)i, D32(CHOICE + 4u * (uint32_t)(i + 1)));   /* 0x28C8F */
            setD32(CHOICE_N, (uint32_t)(S32(CHOICE_N) - 1));        /* 0x28CA4 */
            f79A9C(&sel);                                           /* 0x28CAA */
            removed = 0;                                            /* 0x28310 (esi = 0 at 0x28CA2) */
        }
    }
    f78664(LOSS_DLG);                                               /* 0x28CB9 */
    load_heap_free(D32(SNAP_AREA));                                 /* 0x28CC5 */
    setD32(SNAP_AREA, 0);                                           /* 0x28CCF */
    load_heap_free(D32(SNAP_TSTATE));                               /* 0x28CD5 */
    setD32(SNAP_TSTATE, 0);                                         /* 0x28CDF */
    load_heap_free(D32(SNAP_NATION));                               /* 0x28CE5 */
    setD32(SNAP_NATION, 0);                                         /* 0x28CEA */
}

/* ==== 0x291D0 / 0x29550 / 0x29C94: the secondary losses ========================================================== */

/* the nations of the columns: m 1..9 in play, m != a, m != b, 0xF796C[m] != 0 (0x2923D..0x29278 and its twins) */
static int column_nation(int m, int a, int b)
{
    if (D8(NATION(m) + 0x9Au) == 0)                                 /* 0x2924A test dl, dl */
        return 0;
    if (m == a)                                                     /* 0x29257 */
        return 0;
    if (m == b)                                                     /* 0x2925F */
        return 0;
    return S16(SEC_CNT + 2u * (uint32_t)m) != 0;                    /* 0x29267 cmp word [2m + 0xF796C], 0 */
}

/* a column's head: its name centred over w (the short name 0xC7664[m] when fgf_width(long) >= w), its token sprite
   centred at y 0x17C (0x29441..0x29386 and 0x29984..0x29718) */
static void column_head(int m, int x, int w)
{
    const char *name;

    fg_setcolor(5);                                                 /* 0x2944B / 0x29990 */
    name = NAME(m);
    if (fgf_width(name, (int)strlen(name)) >= w)                    /* 0x29473, 0x29486 jge */
        name = SHORTNAME(m);                                        /* 0x29327 */
    f3A0B0(x, 0x16D, w, name);                                      /* 0x29341 */
    f3AF8C(0xF489Cu + 8u * (uint32_t)m,                             /* 0x29381 */
           x + (w - (int)D16(0xF48A0u + 8u * (uint32_t)m)) / 2, 0x17C);   /* 0x29349 word [8m + 0xF48A0], signed /2 */
}

void f291D0(int a, int b, const char *text)
{
    char buf[32];        /* [ebp-0x6C]..[ebp-0x4D] */
    int32_t list[9];     /* [ebp-0x4C]..[ebp-0x2D] */
    int32_t cnt;         /* [ebp-0x1C] */
    int32_t m;           /* [ebp-0x14] */
    int32_t w;           /* [ebp-0x10] */
    int32_t x0;          /* [ebp-0x18] */
    int32_t j;           /* [ebp-0x20] */
    int32_t r;           /* [ebp-0x24] */

    background3();                                                  /* 0x291EC..0x29221 */
    cnt = 0;                                                        /* 0x2922B */
    for (m = 1; m <= 9; m++)                                        /* 0x2922E, 0x2923D jg */
        if (column_nation(m, a, b))
            cnt++;                                                  /* 0x29275 */
    if (cnt == 0)                                                   /* 0x2927E je 0x29545 */
        return;
    w = (cnt == 2 ? 0xC8 : 0x12C) / cnt;                            /* 0x29284..0x2929E idiv */
    x0 = cnt == 2 ? 0xDC : 0xAA;                                    /* 0x292A3..0x292B5 */
    f3A924();                                                       /* 0x292B8 */
    f3A9F4(text);                                                   /* 0x292C0 */
    f7876C();                                                       /* 0x292C5 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x292D1 */
    fg_vbcopy(0xAA, 0x1D5, 0x17C, 0x1A2, 0xAA, 0x1A2, S32(BACKGROUND), S32(BACKBUF));   /* 0x29304 */
    fg_setcolor(5);                                                 /* 0x2930E */
    j = 0;                                                          /* 0x29318 */
    for (m = 1; m <= 9; m++) {                                      /* 0x2931B, 0x29406 jg 0x294A9 */
        if (!column_nation(m, a, b))
            continue;
        if (j < 9)            /* port guard: the original's list has 9 slots (G: at most 8 columns, 8 players) */
            list[j] = m;                                            /* 0x29447 */
        column_head(m, x0 + w * j, w);                              /* 0x29445..0x29386 */
        f3C9D0(x0 + w * j + (w - 0x10) / 2, 0x198, 0x10, 0xE, 5);   /* 0x293B2 */
        fg_setcolor(5);                                             /* 0x293B9 */
        snprintf(buf, sizeof buf, "%d", (int)S8(SEC_LOSS + (uint32_t)m));   /* 0x293D9 itoa(dword [0xF5479 + list[j]] sar 24), list[j] = m */
        f3A0B0(x0 + w * j, 0x19A, w, buf);                          /* 0x293F5 */
        j++;                                                        /* 0x293FD */
    }
    fg_vbclose();                                                   /* 0x294A9 */
    f3BB48(CONT_DLG, 0x2A, 0x109);                                  /* 0x294BD */
    f3BC64(CONT_DLG, 0x2A, 0x1AE);                                  /* 0x294D1 */
    f7853C(CONT_DLG);                                               /* 0x294DB */
    f3B05C(PANEL_DLG);                                              /* 0x294E7 */
    f3D52C(0, 0);                                                   /* 0x294EE */
    f3CBBC();                                                       /* 0x294F3 */
    do {
        if (f7BA64() == 0)                                          /* 0x294F8, 0x294FF jne */
            r = f78BA4(PANEL_DLG, 0);                               /* 0x29508 */
        else {
            f78A20(S16(0xF40BAu) == 0 ? 0xFA : 0x3E8);              /* 0x29512..0x2952D */
            r = -1;                                                 /* 0x29532 */
        }
    } while (r != -1);                                              /* 0x29535 */
    f78664(CONT_DLG);                                               /* 0x29540 */
}

void f29550(int a, int b, int total, const char *text)
{
    int32_t list[9] = {0};   /* [ebp-0x5C]..[ebp-0x3D] */
    int32_t cnt;         /* [ebp-0x18] */
    int32_t m;           /* [ebp-0x34] */
    int32_t w;           /* [ebp-0x10] */
    int32_t x0;          /* [ebp-0x14] */
    int32_t j;           /* [ebp-0x28] */
    int32_t sum;         /* [ebp-0x24] */
    int32_t left;        /* [ebp-0x20] */
    int32_t r;           /* [ebp-0x2C] */
    int32_t col;         /* [ebp-0x1C] */
    int32_t i, x, idx;
    uint32_t rec;
    int mx, my, mb;

    background3();                                                  /* 0x2956E..0x295A3 */
    cnt = 0;                                                        /* 0x295AD */
    for (m = 1; m <= 9; m++)                                        /* 0x295B0, 0x295BF jg */
        if (column_nation(m, a, b))
            cnt++;                                                  /* 0x295F7 */
    w = (cnt == 2 ? 0xC8 : 0x12C) / cnt;                            /* 0x295FC..0x29616 idiv (no test for 0: see the file comment) */
    x0 = cnt == 2 ? 0xDC : 0xAA;                                    /* 0x2961B..0x2962D */
    memset(DP(SEC_LOSS), 0, 0xE);                                   /* 0x2963E */
    j = 0;                                                          /* 0x29643 */
    f3A924();                                                       /* 0x29646 */
    f3A9F4(text);                                                   /* 0x2964E */
    f7876C();                                                       /* 0x29653 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x2965F */
    fg_vbcopy(0xAA, 0x1D5, 0x17C, 0x1A2, 0xAA, 0x1A2, S32(BACKGROUND), S32(BACKBUF));   /* 0x29692 */
    fg_setcolor(5);                                                 /* 0x2969C */
    setD16(PANEL_DLG, D16(PANEL_BASE));                             /* 0x296AF the appended records dropped */
    for (m = 1; m <= 9; m++) {                                      /* 0x296B5, 0x2994B jg 0x299EE */
        if (!column_nation(m, a, b))
            continue;
        if (j < 9)            /* port guard, as in 0x291D0 */
            list[j] = m;                                            /* 0x2998C */
        column_head(m, x0 + w * j, w);                              /* 0x2998A..0x29718 */
        x = x0 + w * j + (w - 0x10) / 2;                            /* 0x29718..0x29733 */
        f3AF8C(0xF51BCu, x - 7, 0x19A);                             /* 0x29742 the minus */
        f3AF8C(0xF51C4u, x + 0x12, 0x19A);                          /* 0x2976F the plus */
        /* record base + 2j: the minus, a copy of 0xC0614 (id 0xD3) */
        idx = S32(PANEL_BASE) + 2 * j;                              /* 0x29777..0x2978A dword [0xF5944] */
        rec = PANEL_DLG + 2u + 20u * (uint32_t)idx;
        memcpy(DP(rec), DP(0xC0614u), 20);                          /* 0x29796 movsd x 5 */
        setD16(rec + 0x00u, (uint16_t)(x - 8));                     /* 0x297D1 */
        setD16(rec + 0x02u, 0x19A);                                 /* 0x297EF */
        setD16(rec + 0x0Eu, (uint16_t)(D16(rec + 0x0Eu) + (uint32_t)j));   /* 0x29830 id + j */
        setD8(rec + 0x10u, 1);                                      /* 0x29848 enabled */
        setD16(PANEL_DLG, (uint16_t)(D16(PANEL_DLG) + 1u));         /* 0x29850 */
        /* record base + 2j + 1: the plus, a copy of 0xC0600 (id 0xCC) */
        rec = PANEL_DLG + 2u + 20u * (uint32_t)(idx + 1);
        memcpy(DP(rec), DP(0xC0600u), 20);                          /* 0x2987C movsd x 5 */
        setD16(rec + 0x00u, (uint16_t)(x + 0x12));                  /* 0x298BA */
        setD16(rec + 0x02u, 0x19A);                                 /* 0x298DC */
        setD16(rec + 0x0Eu, (uint16_t)(D16(rec + 0x0Eu) + (uint32_t)j));   /* 0x29915 */
        setD8(rec + 0x10u, 1);                                      /* 0x29930 */
        setD16(PANEL_DLG, (uint16_t)(D16(PANEL_DLG) + 1u));         /* 0x29938 */
        j++;                                                        /* 0x29942 */
    }
    sum = 0;                                                        /* 0x299F0 */
    for (m = 1; m <= 9; m++)                                        /* 0x299F3 */
        sum += S16(SEC_CNT + 2u * (uint32_t)m);                     /* 0x29A15 */
    left = sum > total ? total : sum;                               /* 0x29A1D jle */
    f3A3A0(0xE8, 0x1B2, DSTR(0xB2C71u));                            /* 0x29A3C "LEFT TO" */
    f3A044(0x107, 0x1B2, DSTR(0xB2C79u));                           /* 0x29A50 "ASSIGN" */
    fg_vbclose();                                                   /* 0x29A55 */
    f7853C(ASSIGN_DLG);                                             /* 0x29A5F */
    f3B05C(PANEL_DLG);                                              /* 0x29A6B */
    f3D52C(0, 0);                                                   /* 0x29A72 */
    for (;;) {
        /* 0x29A77 */
        f29C94(a, b, list, w, left, x0);                            /* 0x29A8B */
        f775F0();                                                   /* 0x29A90 */
    poll:
        while (S32(MOUSE_B) == 0) {                                 /* 0x29A95 cmp dword [0xE1630], 0 */
            fg_mousepos(&mx, &my, &mb);                             /* 0x29AAD fg_mousepos(0xE1628, 0xE162C, 0xE1630) */
            setD32(MOUSE_X, (uint32_t)mx);
            setD32(MOUSE_Y, (uint32_t)my);
            setD32(MOUSE_B, (uint32_t)mb);
        }
        r = -100;                                                   /* 0x29AB7 */
        for (i = 0; i < S16(PANEL_DLG); i++) {                      /* 0x29AC0, 0x29ACB dword [0xF596A] sar 16 */
            rec = PANEL_DLG + 2u + 20u * (uint32_t)i;               /* 0x29AF2 */
            if (S8(rec + 0x10u) != 1)                               /* 0x29AF8 dword [rec + 0xD] sar 24 */
                continue;
            if (S16(rec) > S32(MOUSE_X) || S16(rec) + S16(rec + 4u) <= S32(MOUSE_X))   /* 0x29B11 jg, 0x29B20 jle */
                continue;
            if (S16(rec + 2u) > S32(MOUSE_Y) || S16(rec + 2u) + S16(rec + 6u) <= S32(MOUSE_Y))   /* 0x29B32, 0x29B41 */
                continue;
            f3C5D4(PANEL_DLG, S16(rec + 0x0Eu));                    /* 0x29B51 */
            r = f3FB94(S16(rec + 0x0Eu));                           /* 0x29B5F, 0x29B64 */
            break;
        }
        if (r == -1)                                                /* 0x29B67 je 0x29C81: CONTINUE */
            break;
        if (r <= -0x67 && r >= -0x6D) {                             /* 0x29B74 jg, 0x29B7D jl: a plus */
            if (left <= 0)                                          /* 0x29B8A jle 0x29A77 */
                continue;
            col = -0x67 - r;                                        /* 0x29B92..0x29B9C */
            if (S8(SEC_LOSS + (uint32_t)list[col]) < S16(SEC_CNT + 2u * (uint32_t)list[col])) {   /* 0x29BC0 jge */
                setD8(SEC_LOSS + (uint32_t)list[col], (uint8_t)(D8(SEC_LOSS + (uint32_t)list[col]) + 1u));   /* 0x29BCB */
                left--;                                             /* 0x29BD4 */
            }
            f29C94(a, b, list, w, left, x0);                        /* 0x29BEB */
            f78A20(0x96);                                           /* 0x29BF7 */
            setD32(MOUSE_B, 0);                                     /* 0x29BFC the button counts as up: auto-repeat */
            goto poll;                                              /* 0x29C02 */
        }
        if (r <= -0x6E && r >= -0x74) {                             /* 0x29C0D jg, 0x29C17 jl: a minus */
            col = -0x6E - r;                                        /* 0x29C1D..0x29C27 */
            if (S8(SEC_LOSS + (uint32_t)list[col]) > 0) {           /* 0x29C3C jle */
                setD8(SEC_LOSS + (uint32_t)list[col], (uint8_t)(D8(SEC_LOSS + (uint32_t)list[col]) - 1u));   /* 0x29C45 */
                left++;                                             /* 0x29C4E */
            }
            f29C94(a, b, list, w, left, x0);                        /* 0x29C65 */
            f78A20(0x96);                                           /* 0x29C71 */
            setD32(MOUSE_B, 0);                                     /* 0x29C76 */
            goto poll;                                              /* 0x29C7C */
        }
        /* any other code: redraw and wait for the release (0x29A77) */
    }
    setD16(PANEL_DLG, D16(PANEL_BASE));                             /* 0x29C87 (CONTINUE 0xBFA3C cut off, not removed) */
}

void f29C94(int a, int b, const int32_t *list, int w, int left, int x0)
{
    char buf[32];        /* [ebp-0x3C]..[ebp-0x1D] */
    int32_t j;           /* [ebp-0x8] */
    int32_t m;           /* [ebp-0xC] */

    fg_vbopen(S32(BACKBUF));                                        /* 0x29CB8 */
    f3C9D0(0xEF, 0x1AE, 0x12, 0x11, 5);                             /* 0x29CD6 */
    snprintf(buf, sizeof buf, "%d", left);                          /* 0x29CE6 itoa */
    fg_setcolor(5);                                                 /* 0x29CED */
    f3A0B0(0xEF, 0x1B1, 0x12, buf);                                 /* 0x29D07 */
    j = 0;                                                          /* 0x29D0E */
    for (m = 1; m <= 9; m++) {                                      /* 0x29D11, 0x29D20 jg 0x29DDC */
        if (!column_nation(m, a, b))
            continue;
        f3C9D0(x0 + w * j + (w - 0x10) / 2, 0x198, 0x10, 0xE, 5);   /* 0x29D85 */
        fg_setcolor(5);                                             /* 0x29D8C */
        snprintf(buf, sizeof buf, "%d", (int)S8(SEC_LOSS + (uint32_t)list[j < 9 ? j : 8]));   /* 0x29DB0 itoa */
        f3A0B0(x0 + w * j, 0x19A, w, buf);                          /* 0x29DCC */
        j++;                                                        /* 0x29DD4 */
    }
    fg_vbclose();                                                   /* 0x29DDC */
    if ((D8(FLAGS) & PBEM_BIT) == 0 && S8(NATION(a) + 0x9Au) == 3) {   /* 0x29DE1, 0x29DFA */
        if (left > 0)                                               /* 0x29E04 jle */
            f3C3D0(PANEL_DLG, 0x2A);                                /* 0x29E10 CONTINUE off */
        else
            f3C314(PANEL_DLG, 0x2A);                                /* 0x29E21 CONTINUE on */
        f3CBBC();                                                   /* 0x29E26 */
    }
}

/* ==== 0x2D4F0 Flood =============================================================================================== */

/* the units of nation n on plain s: a flood-plain city of n counts 5, else n's tokens (0x2D5C1..0x2D757 for the
   victim, 0x2DC5E..0x2DDC3 for the others); *any + 1 for each territory that counted */
static int32_t plain_units(int s, int n, int32_t *any)
{
    int32_t j, t, u = 0;

    for (j = 0; S16(PLAIN(s) + 2u) > j; j++) {                      /* 0x2D667 / 0x2DCE4 dword [0xBEE08 + 22s] sar 16 */
        t = S16(PLAIN(s) + 8u + 2u * (uint32_t)j);                  /* dword [2j + 22s + 0xBEE0E] sar 16 */
        if ((D8(TSTATE(t) + 2u) & 1) != 0                           /* 0x2D6BA / 0x2DD37 */
            && D8(TSTATE(t)) == (uint32_t)n                         /* 0x2D6F8 / 0x2DD74 */
            && (D8(TERR(t) + 1u) & 0x80) != 0) {                    /* 0x2D730 / 0x2DDAB test byte [.. + 0xE1639], 0x80 */
            u += 5;                                                 /* 0x2D74E / 0x2DDBB */
            if (any != NULL)
                (*any)++;                                           /* 0x2D65E */
        } else if (D8(AREA(t + 1) + 0x0Eu + (uint32_t)n) != 0) {    /* 0x2D5F3 / 0x2DC91 */
            u += S8(AREA(t + 1) + 0x0Eu + (uint32_t)n);             /* 0x2D64A movsx / 0x2DCD7 */
            if (any != NULL)
                (*any)++;
        }
    }
    return u;
}

void f2D4F0(int k)
{
    char buf[100];       /* [ebp-0xF0]..[ebp-0x8D] */
    int32_t u[11];       /* [ebp-0x8C + 4m], m 1..10 (u[10] = [ebp-0x64], the Pirates) */
    int32_t n;           /* [ebp-0x54] */
    int32_t e;           /* [ebp-0x50] */
    uint32_t rec;        /* [ebp-0x1C] */
    int32_t any;         /* [ebp-0x4C] (later the count of nations with secondary units) */
    int32_t s;           /* [ebp-0x44] */
    int32_t found;       /* [ebp-0x18] */
    int32_t t;           /* [ebp-0x24] */
    int32_t best;        /* [ebp-0x30] */
    int32_t ties;        /* [ebp-0x2C] */
    int32_t last;        /* [ebp-0x28] */
    int32_t plain;       /* [ebp-0x20] */
    int32_t units;       /* [ebp-0x3C] */
    int32_t cap;         /* [ebp-0x38] */
    int32_t L;           /* [ebp-0x34] */
    int32_t m;           /* [ebp-0x58] */
    int32_t total;       /* [ebp-0x40] */
    int32_t c, v, wait;

    setD16(0xBB400u, 0x15);                                         /* 0x2D51A */
    memset(DP(SEC_LOSS), 0, 0xE);                                   /* 0x2D523 */
    memset(DP(0xF548Cu), 0, 0xE);                                   /* 0x2D534 */
    n = S8(CAL_VICTIM(k));                                          /* 0x2D545 dword [0xF77E3 + 3k] sar 24 */
    e = S8(CAL_GIVER(k));                                           /* 0x2D55D dword [0xF77E2 + 3k] sar 24 */
    rec = NATION(n);                                                /* 0x2D577 */
    setD32(CUR_NATION, (uint32_t)n);                                /* 0x2D57D */
    any = 0;                                                        /* 0x2D584 */
    for (s = 0; s < 5; s++) {                                       /* 0x2D587, 0x2D592 jge */
        setD16(PLAIN(s), 0);                                        /* 0x2D5AF */
        for (c = 0; S16(PLAIN(s) + 2u) > c; c++) {                  /* 0x2D5B9 */
            t = S16(PLAIN(s) + 8u + 2u * (uint32_t)c);
            if ((D8(TSTATE(t) + 2u) & 1) != 0 && D8(TSTATE(t)) == (uint32_t)n && (D8(TERR(t) + 1u) & 0x80) != 0) {
                setD16(PLAIN(s), (uint16_t)(D16(PLAIN(s)) + 5u));   /* 0x2D74E add word, 5 */
                any++;                                              /* 0x2D65E */
            } else if (D8(AREA(t + 1) + 0x0Eu + (uint32_t)n) != 0) {    /* 0x2D5FB */
                setD16(PLAIN(s), (uint16_t)(D16(PLAIN(s)) + (uint16_t)S8(AREA(t + 1) + 0x0Eu + (uint32_t)n)));   /* 0x2D652 add si, ax (16 bits) */
                any++;
            }
        }
    }
    if (any == 0) {                                                 /* 0x2D761 jne 0x2D9F0 */
        /* no units on any plain: a coastal city */
        found = 0;                                                  /* 0x2D767 */
        setD32(CHOICE_N, 0);                                        /* 0x2D76A */
        for (t = 0; t < 0x9D; t++) {                                /* 0x2D770, 0x2D77B jge */
            if ((D8(TSTATE(t) + 2u) & 1) == 0)                      /* 0x2D79A */
                continue;
            if (D8(TSTATE(t)) != (uint32_t)n)                       /* 0x2D7B6 */
                continue;
            if ((D8(TERR(t)) & 0x02) == 0)                          /* 0x2D7CC test byte [.. + 0xE1638], 2 */
                continue;
            found = 1;                                              /* 0x2D7DA */
            c = S32(CHOICE_N);                                      /* 0x2D7DD */
            setD32(CHOICE_N, (uint32_t)(c + 1));                    /* 0x2D7EB */
            setD32(CHOICE + 4u * (uint32_t)c, (uint32_t)t);         /* 0x2D7F3 */
        }
        if (found == 0) {                                           /* 0x2D802 jne 0x2D838 */
            snprintf(buf, sizeof buf,
                     "%s has no units on any flood plain or coastal cities, so is unaffected by Flood.",
                     NAME(n));                                      /* 0x2D81B (0xB33D4) */
            f79270(-500, n, -200, buf, 1);                          /* 0x2D9D9 (eax 0x2D9D4) */
            f1E69C(k);                                              /* 0x2D9E1 */
            return;
        }
        snprintf(buf, sizeof buf, "%s has no units on any flood plain so a coastal city must be %s.", NAME(n),
                 D8(rec + 0x2Au) != 0 ? DSTR(0xB3425u) : DSTR(0xB342Du));   /* 0x2D83F Engineering, 0x2D86D (0xB3438) */
        f79270(-500, n, -200, buf, 1);                              /* 0x2D88A */
        if (human(n))                                               /* 0x2D88F, 0x2D8AB */
            t = f207B4(n, n, 0x19);                                 /* 0x2D8B8 */
        else
            t = f2E1D0(n, 0x19);                                    /* 0x2D8C7 [M7] */
        if (D8(NATION(n) + 0x2Au) != 0)                             /* 0x2D8D6 */
            f78FEC(t, n);                                           /* 0x2D8E5 reduced */
        else
            f78F04(t, n);                                           /* 0x2D8F2 eliminated */
        if (human(n)) {                                             /* 0x2D8F7, 0x2D913 */
            f3D52C(0, 0);                                           /* 0x2D919 */
            f78A20(0x7D0);                                          /* 0x2D923 */
            f1E69C(k);                                              /* 0x2E1C1 (0x2D928 jmp 0x2E1BE) */
            return;
        }
        snprintf(buf, sizeof buf, "%s %s a coastal city in %s.", NAME(n),
                 D8(NATION(n) + 0x2Au) != 0 ? DSTR(0xB3479u) : DSTR(0xB3481u), TERR_NAME(t));   /* 0x2D96D (0xB348C) */
        if ((D8(0xF0980u) & 0x04) != 0)                             /* 0x2D97B jne 0x2D9C6 */
            wait = -500;                                            /* 0x2D9D4 */
        else if (f7BA64() == 0)                                     /* 0x2D980, 0x2D987 jne */
            wait = 0x7D0;                                           /* 0x2D997 */
        else
            wait = S16(0xF40BAu) == 0 ? 0x1F4 : 0x5DC;              /* 0x2D9AA..0x2D9BF */
        f79270(wait, n, t, buf, 1);                                 /* 0x2D9D9 */
        f1E69C(k);                                                  /* 0x2D9E1 */
        return;
    }

    /* 0x2D9F0: the plain with the most units (the last of equals); ties */
    best = 0;                                                       /* 0x2D9F2 */
    ties = 0;                                                       /* 0x2D9F5 */
    last = 0;   /* port: [ebp-0x28] is always written below (some plain has units, D) */
    for (s = 0; s < 5; s++) {                                       /* 0x2D9F8, 0x2DA2C jge */
        v = S16(PLAIN(s));                                          /* 0x2DA43 dword [0xBEE06 + 22s] sar 16 */
        if (v < best)                                               /* 0x2DA52 jl */
            continue;
        if (v != best)                                              /* 0x2DA74 jne 0x2D9FD */
            ties = 0;                                               /* 0x2D9FF */
        else
            ties++;                                                 /* 0x2DA79 */
        best = S16(PLAIN(s));                                       /* 0x2DA1D */
        last = s;                                                   /* 0x2DA23 */
    }
    plain = last;   /* port: the computer's tie branch with fewer than two listed plains leaves [ebp-0x20] as it was
                       (stack garbage); unreachable (file comment) */
    if (ties == 0) {                                                /* 0x2DA82 jne 0x2DA8C */
        plain = last;                                               /* 0x2DA84, 0x2DB7C */
    } else if (human(n)) {                                          /* 0x2DA92, 0x2DAAA (rec + 0x9A, dword [rec + 0x97] sar 24) */
        setD32(CHOICE_N, 0);                                        /* 0x2DAAE */
        for (s = 0; s < 5; s++) {                                   /* 0x2DAB4, 0x2DABF jge */
            if (S16(PLAIN(s)) != best)                              /* 0x2DAE5 */
                continue;
            c = S32(CHOICE_N);                                      /* 0x2DAE7 */
            setD32(CHOICE_N, (uint32_t)(c + 1));                    /* 0x2DAF4 */
            setD32(CHOICE + 4u * (uint32_t)c, (uint32_t)s);         /* 0x2DAFC */
        }
        plain = f1EAEC(n, 0x19);                                    /* 0x2DB0C [CAL3], 0x2DB7C */
    } else {
        setD32(CHOICE_N, 0);                                        /* 0x2DB15 */
        for (s = 0; s < 5; s++) {                                   /* 0x2DB1B, 0x2DB26 jge */
            if (S16(PLAIN(s)) != best)                              /* 0x2DB4C */
                continue;
            c = S32(CHOICE_N);                                      /* 0x2DB4E */
            setD32(CHOICE_N, (uint32_t)(c + 1));                    /* 0x2DB5B */
            setD32(CHOICE + 4u * (uint32_t)c, (uint32_t)s);         /* 0x2DB63 */
        }
        if (S32(CHOICE_N) > 1)                                      /* 0x2DB72 jle 0x2DB7F */
            plain = f2E45C(n);                                      /* 0x2DB77 [M7], 0x2DB7C */
    }
    units = S16(PLAIN(plain));                                      /* 0x2DB90 dword [0xBEE06 + 22p] sar 16 */
    cap = D8(NATION(n) + 0x2Au) != 0 ? 7 : 0x11;                    /* 0x2DBA4 Engineering */
    L = units > cap ? cap : units;                                  /* 0x2DBC2 jle */
    if (human(n)) {                                                 /* 0x2DBCF, 0x2DBE7 */
        snprintf(buf, sizeof buf, "The %s river Floods.\n%s, choose sites of losses.", river(plain), NAME(n));   /* 0x2DC19 (0xB34A8) */
        f27FCC(n, L, buf, 0x19, plain, 0);                          /* 0x2DC38 */
    } else
        f260C4(n, L, 0x19, plain);                                  /* 0x2DC4D [M7] */

    /* the secondary units on the plain, m 1..10 (10 = the Pirates) */
    for (m = 1; m <= 10; m++) {                                     /* 0x2DC52, 0x2DE32 jg */
        u[m] = 0;                                                   /* 0x2DE3D */
        setD16(SEC_CNT + 2u * (uint32_t)m, 0);                      /* 0x2DE49 (m = 10: 0xF7980) */
        if (m == n)                                                 /* 0x2DE57 */
            continue;
        if (D8(NATION(m) + 0x9Au) == 0)                             /* 0x2DE62 */
            continue;
        u[m] = plain_units(plain, m, NULL);                         /* 0x2DE6B..0x2DDC3 */
        setD16(SEC_CNT + 2u * (uint32_t)m, (uint16_t)u[m]);         /* 0x2DDD6 */
        if (S16(SEC_CNT + 2u * (uint32_t)m) > 10)                   /* 0x2DDEB jle */
            setD16(SEC_CNT + 2u * (uint32_t)m, 10);                 /* 0x2DDF3 */
        if (D8(NATION(m) + 0x2Au) != 0 && S16(SEC_CNT + 2u * (uint32_t)m) > 7)   /* 0x2DE04, 0x2DE1A */
            setD16(SEC_CNT + 2u * (uint32_t)m, 7);                  /* 0x2DE22 */
    }
    total = 0;                                                      /* 0x2DE75 */
    any = 0;                                                        /* 0x2DE78 */
    for (s = 1; s <= 9; s++) {                                      /* 0x2DE7B */
        total += S16(SEC_CNT + 2u * (uint32_t)s);                   /* 0x2DE9D */
        if (S16(SEC_CNT + 2u * (uint32_t)s) != 0)                   /* 0x2DEAB */
            any++;                                                  /* 0x2DEB3 */
    }
    if (total != 0) {                                               /* 0x2DEBC je 0x2E1A3 */
        if (total <= 10) {                                          /* 0x2DEC8 jg 0x2E01A: all of them */
            snprintf(buf, sizeof buf, "The %s river flooding eliminates all secondary units on the flood plain.",
                     river(plain));                                 /* 0x2DEF3 (0xB34D9) */
            f79270(-500, n, S16(PLAIN(plain) + 8u), buf, 1);        /* 0x2DF26 (ebx = the plain's first territory) */
            for (m = 1; m <= 9; m++) {                              /* 0x2DF2B, 0x2DF68 jg */
                if (D8(NATION(m) + 0x9Au) == 0)                     /* 0x2DF7F */
                    continue;
                if (m == n)                                         /* 0x2DF89 */
                    continue;
                if (m == e)                                         /* 0x2DF91 */
                    continue;
                if (S16(SEC_CNT + 2u * (uint32_t)m) < u[m] && human(m)) {   /* 0x2DFAC jge, 0x2DFB5, 0x2DFCA */
                    snprintf(buf, sizeof buf, "Flood also ravages %s.\nChoose sites of losses.", NAME(m));   /* 0x2DFE7 (0xB3522) */
                    f27FCC(m, S16(SEC_CNT + 2u * (uint32_t)m), buf, 0x19, plain, 0);   /* 0x2E010 */
                } else if (S16(SEC_CNT + 2u * (uint32_t)m) < u[m]   /* a computer (or PBEM) losing part: 0x2DF45 */
                           || S16(SEC_CNT + 2u * (uint32_t)m) > 0)  /* everything: 0x2DF34 jle */
                    f260C4(m, S16(SEC_CNT + 2u * (uint32_t)m), 0x19, plain);   /* 0x2DF5D [M7] */
            }
        } else {
            if (human(n)) {                                         /* 0x2E01A, 0x2E038 */
                if (any > 1) {                                      /* 0x2E044 jle 0x2E080 */
                    snprintf(buf, sizeof buf, "%s, assign secondary losses for Flood.", NAME(n));   /* 0x2E05D (0xB3551) */
                    f29550(n, e, 10, buf);                          /* 0x2E076 */
                } else {
                    for (s = 1; s <= 9; s++) {                      /* 0x2E080 (dead: one nation has at most 10) */
                        if (S16(SEC_CNT + 2u * (uint32_t)s) == 0)   /* 0x2E0A3 */
                            continue;
                        v = S16(SEC_CNT + 2u * (uint32_t)s);        /* 0x2E0AF */
                        setD8(SEC_LOSS + (uint32_t)s, (uint8_t)(v > 10 ? 10 : v));   /* 0x2E0CE */
                        break;                                      /* 0x2E0D4 jmp 0x2E103: the first one only */
                    }
                }
            } else
                f27518(n, -1, 10, 0x19);                            /* 0x2E0EF [M7] */
            for (m = 1; m <= 9; m++) {                              /* 0x2E0F4, 0x2E103 jg */
                if (S8(SEC_LOSS + (uint32_t)m) <= 0)                /* 0x2E11B jle */
                    continue;
                /* BUG(1995) 1: the screen was chosen by the victim's flag ([ebp-0x1C] + 0x9A), not m's */
                if (human(g_original ? n : m)) {                    /* 0x2E11D, 0x2E137 */
                    snprintf(buf, sizeof buf, "Flood also ravages %s.\nChoose sites of losses.", NAME(m));   /* 0x2E150 (0xB3578) */
                    f27FCC(m, S8(SEC_LOSS + (uint32_t)m), buf, 0x19, plain, 0);   /* 0x2E178 */
                } else
                    f260C4(m, S8(SEC_LOSS + (uint32_t)m), 0x19, plain);   /* 0x2E199 [M7] */
            }
        }
    }
    if (u[10] != 0)                                                 /* 0x2E1A3 [ebp-0x64]: the Pirates */
        f260C4(10, u[10], 0x19, plain);                             /* 0x2E1B9 [M7] (uncapped) */
    f1E69C(k);                                                      /* 0x2E1C1 */
}

/* ==== 0x2E59C Epidemic ============================================================================================ */

void f2E59C(int k)
{
    char buf[100];       /* [ebp-0xA0]..[ebp-0x3D] */
    int32_t n;           /* [ebp-0x34] */
    int32_t e;           /* [ebp-0x30] */
    uint32_t rec;        /* [ebp-0x28] */
    int32_t losses;      /* [ebp-0x20] */
    int32_t v;           /* [ebp-0x24] */
    int32_t x;           /* [ebp-0x1C] */
    int32_t m;           /* [ebp-0x2C] */
    int32_t j;           /* [ebp-0x38] */
    int32_t pos;         /* [ebp-0x18] */

    setD16(0xBB3FAu, 0xA9);                                         /* 0x2E5CB */
    setD16(0xBB3FCu, 0x154);                                        /* 0x2E5D2 */
    setD16(0xBB3FEu, 0x12C);                                        /* 0x2E5D9 */
    setD16(0xBB400u, 0x15);                                         /* 0x2E5E0 */
    n = S8(CAL_VICTIM(k));                                          /* 0x2E5F3 */
    e = S8(CAL_GIVER(k));                                           /* 0x2E60B */
    rec = NATION(S8(CAL_VICTIM(k)));                                /* 0x2E623..0x2E639 */
    setD32(CUR_NATION, (uint32_t)n);                                /* 0x2E644 */
    losses = 0x10;                                                  /* 0x2E649 */
    if (D8(rec + 0x2Du) != 0)                                       /* 0x2E64F Medicine */
        losses -= 8;                                                /* 0x2E655 */
    if (D8(rec + 0x28u) != 0)                                       /* 0x2E65C Roadbuilding */
        losses += 5;                                                /* 0x2E662 */
    if (human(n)) {                                                 /* 0x2E666, 0x2E686 */
        snprintf(buf, sizeof buf, "Epidemic ravages %s.\nChoose sites of losses.", NAME(n));   /* 0x2E6A3 (0xB35C2) */
        v = S8(rec + 0x98u) - S8(rec + 0x60u) + 4 * S8(rec + 0x51u);    /* 0x2E6AB..0x2E6CE */
        if (v < losses)                                             /* 0x2E6DB jge */
            losses = v;                                             /* 0x2E6E0 */
        x = 0;                                                      /* 0x2E6E5 */
        if (S8(rec + 0x98u) - S8(rec + 0x60u) == 0)                 /* 0x2E6FD jne: only cities can go */
            x = f2EC38(n, losses);                                  /* 0x2E707 */
        f27FCC(n, losses, buf, 0x1B, 0, x);                         /* 0x2E726 */
        for (m = 1; m <= 9; m++) {                                  /* 0x2E72B, 0x2E74A jg */
            setD16(SEC_CNT + 2u * (uint32_t)m, 0);                  /* 0x2E759 */
            if (D8(NATION(m) + 0x9Au) == 0)                         /* 0x2E768 */
                continue;
            if (m == n || m == e)                                   /* 0x2E775, 0x2E77D */
                continue;
            v = S8(NATION(m) + 0x98u) - S8(NATION(m) + 0x60u) + 4 * S8(NATION(m) + 0x51u);   /* 0x2E782..0x2E7B9 */
            setD16(SEC_CNT + 2u * (uint32_t)m, (uint16_t)(v > 10 ? 10 : v));   /* 0x2E7C0 jle, 0x2E73C */
        }
        v = 0;                                                      /* 0x2E7D2 */
        for (m = 1; m <= 9; m++)                                    /* 0x2E7D5 */
            v += S16(SEC_CNT + 2u * (uint32_t)m);                   /* 0x2E7F7 */
        if (v <= 0x19) {                                            /* 0x2E7FC jg 0x2E843 */
            for (m = 1; m <= 9; m++)                                /* 0x2E802 */
                setD8(SEC_LOSS + (uint32_t)m, D8(SEC_CNT + 2u * (uint32_t)m));   /* 0x2E824 */
            f291D0(n, -1, DSTR(0xB35EFu));                          /* 0x2E839 "Assignments for Epidemic secondary losses." */
        } else {
            snprintf(buf, sizeof buf, "%s, assign secondary losses for Epidemic.", NAME(n));   /* 0x2E85A (0xB361A) */
            f29550(n, e, 0x19, buf);                                /* 0x2E873 */
        }
    } else {
        v = S8(rec + 0x98u) - S8(rec + 0x60u) + 4 * S8(rec + 0x51u);    /* 0x2E884..0x2E8A9 */
        if (v < losses)                                             /* 0x2E8B4 jge */
            losses = v;                                             /* 0x2E8B9 */
        f260C4(n, losses, 0x1B, -1);                                /* 0x2E8CC [M7] */
        for (m = 1; m <= 9; m++) {                                  /* 0x2E8D1, 0x2E8F0 jg */
            setD16(SEC_CNT + 2u * (uint32_t)m, 0);                  /* 0x2E8FF */
            if (D8(NATION(m) + 0x9Au) == 0)                         /* 0x2E90E */
                continue;
            if (m == n || m == e)                                   /* 0x2E91B, 0x2E923 */
                continue;
            v = S8(NATION(m) + 0x98u) - S8(NATION(m) + 0x60u) + 4 * S8(NATION(m) + 0x51u);   /* 0x2E928..0x2E95F */
            setD16(SEC_CNT + 2u * (uint32_t)m, (uint16_t)(v > 10 ? 10 : v));   /* 0x2E966, 0x2E8E2 */
        }
        v = 0;                                                      /* 0x2E978 */
        for (m = 1; m <= 9; m++)                                    /* 0x2E97B */
            v += S16(SEC_CNT + 2u * (uint32_t)m);                   /* 0x2E99D */
        if (v <= 0x19) {                                            /* 0x2E9A2 jg 0x2E9ED */
            for (m = 1; m <= 9; m++)                                /* 0x2E9A8 */
                setD8(SEC_LOSS + (uint32_t)m, D8(SEC_CNT + 2u * (uint32_t)m));   /* 0x2E9CA */
            f291D0(n, -1, DSTR(0xB3644u));                          /* 0x2E9DF "Assignments for Epidemic secondary losses" */
        } else
            f27518(n, e, 0x19, 0x1B);                               /* 0x2E9FD [M7] */
    }
    for (j = 1; j <= 9; j++) {                                      /* 0x2EA02 / 0x2E878 / 0x2E9E4, 0x2EA11 jg */
        if (D8(SEC_LOSS + (uint32_t)j) == 0)                        /* 0x2EA24 */
            continue;
        pos = S8(SEC_LOSS + (uint32_t)j) > 0;                       /* 0x2EA36 setg */
        if (D8(NATION(j) + 0x2Du) != 0) {                           /* 0x2EA48 Medicine */
            setD8(SEC_LOSS + (uint32_t)j, (uint8_t)(D8(SEC_LOSS + (uint32_t)j) - 5u));   /* 0x2EA54 sub byte, 5 */
            if (S8(SEC_LOSS + (uint32_t)j) < 0)                     /* 0x2EA69 jge */
                setD8(SEC_LOSS + (uint32_t)j, 0);                   /* 0x2EA70 */
        }
        if (D8(NATION(j) + 0x28u) != 0)                             /* 0x2EA7D Roadbuilding */
            setD8(SEC_LOSS + (uint32_t)j, (uint8_t)(D8(SEC_LOSS + (uint32_t)j) + 5u));   /* 0x2EA89 add byte, 5 */
        v = S8(NATION(j) + 0x98u) - S8(NATION(j) + 0x60u) + 4 * S8(NATION(j) + 0x51u);   /* 0x2EA90..0x2EAC7 */
        if (S8(SEC_LOSS + (uint32_t)j) > v)                         /* 0x2EAD6 jle */
            setD8(SEC_LOSS + (uint32_t)j, (uint8_t)v);              /* 0x2EAE1 */
        if (S8(SEC_LOSS + (uint32_t)j) > 0) {                       /* 0x2EAF5 jle 0x2EBCF */
            if (human(j)) {                                         /* 0x2EAFB, 0x2EB1D */
                snprintf(buf, sizeof buf, "Epidemic also ravages %s.\nChoose sites of losses.", NAME(j));   /* 0x2EB3C (0xB366E) */
                x = 0;                                              /* 0x2EB44 */
                if (S8(NATION(j) + 0x98u) - S8(NATION(j) + 0x60u) == 0)   /* 0x2EB69 jne */
                    x = f2EC38(j, S8(SEC_LOSS + (uint32_t)j));      /* 0x2EB7A */
                f27FCC(j, S8(SEC_LOSS + (uint32_t)j), buf, 0x1B, 0, x);   /* 0x2EBA2 */
            } else
                f260C4(j, S8(SEC_LOSS + (uint32_t)j), 0x1B, -1);    /* 0x2EBC5 [M7] */
        } else if (pos != 0) {                                      /* 0x2EBD4 je */
            snprintf(buf, sizeof buf, "%s saved from Epidemic by MEDICINE.", NAME(j));   /* 0x2EBF1 (0xB36A0) */
            f79270(-500, n, -200, buf, 1);                          /* 0x2EC0E (edx = the victim n, kept) */
        }
    }
    f1E69C(k);                                                      /* 0x2EC20 */
    setD16(0xBB400u, 0x23);                                         /* 0x2EC25 */
}
