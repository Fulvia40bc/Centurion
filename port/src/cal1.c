/* Module CAL1 of docs/M6_PLAN.md, the human side: the calamity phase and the city-loss calamities. Literal
   translations; the address of each instruction a line comes from is in its comment. D = read in the code, G =
   inference. Prototypes and globals: cal1.h.

     0x1E0A0  f1E0A0  CALAMITIES (phase 15 of the game loop, call at 0x55000)                 672 bytes
     0x1E340  f1E340  any calamity held?                                                        72
     0x1E388  f1E388  Zeus (more than two calamities)                                          560
     0x1E5B8  f1E5B8  the calamity list in resolution order                                    228
     0x1E69C  f1E69C  a resolved calamity card out of the hand, to its pile                    260
     0x20564  f20564  Treachery                                                                592
     0x207B4  f207B4  the human's territory chooser (Treachery, Flood, Barbarians)           1,212
     0x20FC4  f20FC4  Treachery's result                                                     1,020
     0x23230  f23230  Superstition                                                             836
     0x23574  f23574  the computer's city for Superstition (wraps 0x213C0)                    112
     0x235E4  f235E4  Slave Revolt                                                             452
     0x237A8  f237A8  Civil Disorder                                                           804
     0x23ACC  f23ACC  Iconoclasm & Heresy                                                    1,804
     0x241D8  f241D8  Piracy                                                                 1,204
     0x2468C  f2468C  ChooseCitiesToLoseToPirates (Piracy modes 0 / 1, Tax Revolt mode 2)    2,092
     0x28CFC  f28CFC  the choice list 0xF7CA0 / [0xF7C9C] by kind                            1,236

   Not ported here (each call site stops the port with not_ported(address); the static stubs below carry the names,
   so the call sites read as in the original and a later translation that exports one of these names collides with
   its stub at compile time instead of being shadowed by it). Built with -DCAL1_TEST (port/test/cal1_test.c) they go
   to the test's recorders instead, which return.
     M7 (AI):  0x213C0  a computer's own city to reduce (t + 1): 0x23574 (Superstition), 0x237A8 (0x23A3C), 0x23ACC
                        (0x23DA8, 0x2414A)
               0x20C70  a computer's city taken by Treachery (t + 1): 0x206E6
               0x24EB8  a computer takes the victim's coastal cities for the Pirates: 0x243FC
               0x252BC  a computer picks the secondary victims' coastal cities: 0x2462B
               0x27518  a computer assigns Iconoclasm's secondary losses: 0x24074
               (0x25548 and 0x2E1D0 are reached only through 0x252BC / 0x24EB8: no call site here)
     The animations (anim.c), CAL2 (cal2.c: 0x25A38 Famine, 0x2D4F0 Flood, 0x2E59C Epidemic, 0x291D0, 0x29550), CAL3
     (cal3.c: 0x1E7A0 Volcano / Earthquake), CAL4 (cal4.c: 0x29E34 Civil War) and CAL5 (cal5.c: 0x217A0 Barbarian
     Hordes) are called directly since 19 Nov; only the computers' routines above still stop the port.

   tax.c (0x6685C's 0x2468C mode 2, 0x66D8C's 0x28CFC kind 2) and pop.c (0x67758's 0x28CFC kind 3) call the real
   f2468C / f28CFC (cal1.h).

   game_rand (f78AE4): one call per card Zeus discards (0x1E47E), nothing else in this module (D: no other call of
   0x78AE4 in these 16 functions). The computers' routines (0x20C70, 0x213C0, ...) and the animation draw on their
   own (M7, ANIM).

   Play by e-mail (dropped by Erik's ruling): no PBEM routine is called here. The bit-0x80 tests of byte [0xE1624]
   are kept literally: they only send a PBEM human down the computer's path.

   Watcom runtime -> standard C:
     0x7DB3D sprintf -> snprintf bounded to the stack buffer (sizes at each function); the inline strcat copies
             (repne scasb + the byte loop) -> cat(), bounded likewise; the formats are written out with their
             addresses in the comments
     0x8C897 itoa(v, buf, 10) -> snprintf "%d"
     0x7D5D0 memset -> memset on the data object
     0x822B5 malloc -> load_heap_alloc (port handles in the 32-bit fields, game.h); 0x877E6 free -> load_heap_free
     rep movsd / movsw / movsb record copies -> memcpy between g_ds and the block (load_heap_ptr)
   __CHK dropped; 0x78B0C (the debug trace, "ChooseCitiesToLoseToPirates" 0xB27DF) is a no-op and dropped.

   Data (D, read in the code at the addresses given):
     nation record n: 0xF14CC + 199n (rules.h): +0x13 int8 calamities, +0x14 + j their ids, +0x1C + j their givers;
       the advances +0x24.. (names 0xB6CA4..: +0x27 Agriculture, +0x28 Roadbuilding, +0x29 Mining, +0x2A
       Engineering, +0x2F Drama & Poetry, +0x30 Music, +0x33 Law, +0x34 Democracy, +0x35 Military, +0x36 Philosophy,
       +0x37 Mysticism, +0x38 Deism, +0x39 Enlightenment, +0x3A Monotheism, +0x3B Theology); +0x3D uint8 n itself;
       +0x51 int8 cities, +0x52 + j city j's territory + 1; +0x98 int8 tokens on the board; +0x99 int8 stock; +0x9A
       0 out / 2 computer / 3 human. Nation 10 = the Pirates (+0x51 at 0xF1CE3, read as dword [0xF1CE0] sar 24).
     territory record t: 0xE1638 + 35t: +0 bit 0x02 coastal (G: from its use by Piracy and Flood), +1 bits 3..5 the
       population limit, bit 0x80 on a flood plain (G)
     territory state t: 0xE2C18 + 3t (+0 owner, +1 the round built, +2 bit 0x01 city); area record r: 0xE31B8 + 69r
       (+0x0E + n tokens, +0x24 + n POP's allowance, +0x3B + n the "lost territory" mark: 1 lost to a nation, 2 to the
       Pirates)
     names 0xC768C[n], adjectives 0xC76C4[n], card names 0xC775C[c], territory names 0xC77D8[t] (entry -1 is "PIRACY")
     the river table 0xBEE08 + 22s: int16 +2 the count, int16 +8 + 2j the territories (0x28CFC kind 0x19)
     dialogs: 0xF596C the map screen's panel; 0xBDE30 (button 0x58 -> -16, relabelled by 0x207B4); 0xBEE78 (0x2468C:
       RESTART 0x41 -> -0x63, REMOVE 0x3F -> -0x65, FINISHED 0x2A -> -1); arrows 0x39 / 0x3A -> -9 / -10
     sprites 0xF51AC / 0xF51B4 (the two arrows); [0xF52C8] the back buffer, [0xF52CC] the background buffer, int8
       [0xEE168] the background number; byte [0xF0980] bit 0x04 (computer messages wait for CONTINUE, G: an aciv.cfg
       toggle), byte [0xF097C] bit 0x08 (restart confirmations); byte [0xF4102] the phase

   1995 bugs, fixed unless g_original (each marked BUG(1995) in the code; grades code / effect):
     1. 0x1E388 Zeus (D / D): the discarded card's slot is shifted out of the id list +0x14.. but not out of the givers
        +0x1C.., and the freed giver slot is not reset to 0xFF (0x1E69C does both). Every card after the discarded one
        then carries the giver of the card before it: a Treachery card read later by 0x20564 names the wrong
        beneficiary (or a drawn card a trader, or a traded one nobody), and 0x1E5B8 copies the wrong giver into the
        calamity list, which Piracy (the chooser) and Iconoclasm (the excluded trader) use. Fixed: both lists shift,
        the freed giver slot is 0xFF. No game_rand change.
     2. 0x1E69C and 0x20FC4 (D / G): the index of a card (0x1E69C) or a city (0x20FC4, own Treachery) that is not in
        the list is never written ([ebp-0x14] / [ebp-0x18]) and the shift starts from stack garbage, the count drops
        anyway. A port cannot reproduce the garbage: --original starts at index 0. Fixed: the lists stay (and 0x1E69C
        discards nothing). G: not reachable while the hands and city lists are consistent (M6_PLAN risk 7).
     3. 0x20564 Treachery (D / G): the same uninitialised index ([ebp-0x1C]) when the victim holds no Treachery card
        selects the giver byte from garbage. --original: index 0. Fixed: the giver of the calamity record (0xF77E5 +
        3k, which 0x1E5B8 copied from the same hand). G: not reachable while the hands are consistent.
     4. Text, 0x2110B (Erik's ruling of 19 Nov, wrong article; TEXT_SWEEP.md): "replaced by %s %s city." takes "a" when
        0x789EC says "An" and "an" when it says "A": "replaced by a African city", "an Thracian city". Fixed: "an" /
        "a" as 0x21021 does.
     5. Text (misspelling / spacing, same ruling): "Enlightenment nullifys the effect." (0xB222C) -> "nullifies";
        "%s reduces a city in %s ." (0xB253A) -> "%s.". "...for the Pirates to takeover." (0xB25EA, 0xB2705) -> "take
        over" (the verb, a missing space).
     6. 0x207B4 / 0x2468C with an empty choice list (D / G): the box prints territory -1's name, which is "PIRACY"
        (0xC77D4), and f3F624(-1, 15) writes 15 into byte 0xFFB5F, the top byte of the heap pointer [0xFFB5C] (in the
        port a handle, which that write breaks); 0x207B4 also centres the map on territory -1. Fixed as tax.c's 0x66D8C:
        no name, no highlight, and 0x207B4 redraws the map as it is (f3D52C(0, 0), what 0x2468C does with an empty
        list). G: 0x207B4's Treachery list is never empty (the victim has cities); Flood and Barbarians are CAL2/CAL5's.
     7. 0x2468C mode 1, the secondary victims (D / G, reachable): the number to take is min(room, 2, the number of
        secondary coastal CITIES) (0x241D8), but each nation gives one city: REMOVE unmarks the owner and its other
        cities leave the list. When one nation holds all the candidates (any 3-player game with a traded Piracy: one
        secondary nation), the list is empty while one is left to take: FINISHED stays disabled, and REMOVE (still
        enabled) acts on entry 0 = -1: the owner byte read before the territory states (0xE2C15), the "city" of
        territory -1 removed and given to the Pirates (territory 0 into their city list), and the marks of area
        record 0 written. Fixed: when the list is empty, nothing is left to take (left = 0, FINISHED enabled), as the
        board game's "one city from each of two other players" says.
     8. 0x2468C RESTART in mode 1 (D / D): the records are restored but the nation marks 0xF7AC4 are not, so a
        secondary victim whose city was taken before the RESTART can no longer be chosen. Fixed: the marks are set
        again as at the start.
   Kept as the original (D code, G effect unless marked):
     - 0x20FC4's own Treachery adds Agriculture (+0x27) to the limit unconditionally where 0x78FEC adds it only when
       the territory has no other nation's tokens; after a city there are none (G), so the two agree.
     - Superstition's and Civil Disorder's computer path subtracts 1 from 0x213C0 / 0x23574 without testing for -1
       (t = -2 would index before the tables): M7's to check with 0x213C0.
     - Piracy: the trader chooses the primary victim's cities, the victim the secondary ones (the board game's rule).
     - 0x207B4 with a kind other than 0x14 / 0x19 / 0x1A shows an uninitialised stack text (port: an empty text) and
       leaves the button label; no caller passes one (D).
     - 0x1E5B8 has no bound on the list: 0xF7808 sits 36 bytes after 0xF77E4, so a 13th calamity record would
       overwrite the count (its victim byte lands in the count's third byte) and the next records would be written
       far past the data object. G: unreachable, 12 calamity cards exist (ids 0x13..0x1E, one each), so at most 12
       records. Port guard in both modes (not a game rule): records past the 12th are not added.
     - The empty "no calamity" message 0xB19CC (a 100 ms home view of the first player, M6_PLAN egg 15).
   Easter eggs / oddities: "The management of Skull & Crossbones, Inc. regret to inform you that they are unable to
   add new cities to their portfolio." (0xB2554, OVERVIEW §6.3); Zeus's own animation code -298 (zeus.flc, which is
   byte-identical to iconclsm.flc, OVERVIEW §6.13); the debug trace name "ChooseCitiesToLoseToPirates" (0xB27DF); the
   Tax Revolt borrowing the Pirates' chooser (mode 2); "%s is a victim of his own Treachery." (the drawn Treachery
   card); with animations on, 0x207B4 calls the calamity animation on every redraw of its map (G: 0x7A4A0's memo
   [0xCAC60] may make it play once); the "thru" in "%s, select cities to be received thru Tax Revolt." kept (style,
   Erik's ruling). */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "anim.h"
#include "cal1.h"
#include "cal2.h"
#include "cal3.h"
#include "cal4.h"
#include "cal5.h"
#include "city.h"
#include "fg.h"
#include "game.h"
#include "kit.h"
#include "loop.h"
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
#define ADJ(n)       DSTR(D32(0xC76C4u + 4u * (uint32_t)(n)))
#define CARDNAME(c)  DSTR(D32(0xC775Cu + 4u * (uint32_t)(c)))
#define TERR_NAME(t) DSTR(D32(0xC77D8u + 4u * (uint32_t)(t)))
#define TERR(t)      (0xE1638u + 35u * (uint32_t)(t))
#define TSTATE(t)    (0xE2C18u + 3u * (uint32_t)(t))
#define AREA(r)      (0xE31B8u + 69u * (uint32_t)(r))
#define CAL_ID(k)    (0xF77E4u + 3u * (uint32_t)(k))   /* uint8 */
#define CAL_GIVER(k) (0xF77E5u + 3u * (uint32_t)(k))   /* int8 */
#define CAL_VICTIM(k) (0xF77E6u + 3u * (uint32_t)(k))  /* int8 (dword [0xF77E3 + 3k] sar 24) */
#define CAL_COUNT    0xF7808u
#define FLAGS        0xE1624u
#define PBEM_BIT     0x80u
#define CUR_NATION   0xF40F8u
#define PANEL_DLG    0xF596Cu
#define CHOOSE_DLG   0xBDE30u
#define PIRATE_DLG   0xBEE78u
#define BACKBUF      0xF52C8u
#define BACKGROUND   0xF52CCu
#define BGNUM        0xEE168u
#define CHOICE       0xF7CA0u   /* int32[157] */
#define CHOICE_N     0xF7C9Cu   /* int32 */
#define MARKS        0xF7AC4u   /* uint8[12] */
#define SEC_CNT      0xF796Cu   /* int16[10] */
#define SEC_LOSS     0xF547Cu   /* uint8[14] */
#define SNAP_AREA    0xEED74u
#define SNAP_NATION  0xEED78u
#define SNAP_TSTATE  0xEED7Cu
#define AI_PAUSE     0xF0980u   /* byte, bit 0x04 */

/* ---- calls into code that is not ported yet (see the file comment) ----------------------------------------------- */

#ifdef CAL1_TEST
int  cal1_test_f213C0(int n);
int  cal1_test_f20C70(int p, int v);
void cal1_test_f24EB8(int c, int v, int k);
void cal1_test_f252BC(int c, int v, int k);
void cal1_test_f27518(int n, int trader, int total, int kind);
void cal1_test_f7A4A0(void);
void cal1_test_f7A4C4(int anim);
void cal1_test_f291D0(int a, int b, const char *text);
void cal1_test_f29550(int a, int b, int total, const char *text);
void cal1_test_handler(uint32_t addr, int k);
#define f213C0(n)                  cal1_test_f213C0(n)
#define f20C70(p, v)               cal1_test_f20C70(p, v)
#define f24EB8(c, v, k)            cal1_test_f24EB8(c, v, k)
#define f252BC(c, v, k)            cal1_test_f252BC(c, v, k)
#define f27518(n, tr, total, kind) cal1_test_f27518(n, tr, total, kind)
#define f7A4A0()                   cal1_test_f7A4A0()
#define f7A4C4(anim)               cal1_test_f7A4C4(anim)
#define f291D0(a, b, text)         cal1_test_f291D0(a, b, text)
#define f29550(a, b, total, text)  cal1_test_f29550(a, b, total, text)
#define f1E7A0(k)                  cal1_test_handler(0x1E7A0u, k)
#define f25A38(k)                  cal1_test_handler(0x25A38u, k)
#define f29E34(k)                  cal1_test_handler(0x29E34u, k)
#define f2D4F0(k)                  cal1_test_handler(0x2D4F0u, k)
#define f217A0(k)                  cal1_test_handler(0x217A0u, k)
#define f2E59C(k)                  cal1_test_handler(0x2E59Cu, k)
#else
static int f213C0(int n)                        /* M7 (AI): computer n's own city to reduce, territory + 1 */
{
    (void)n;
    not_ported(0x213C0u);
}

static int f20C70(int p, int v)                 /* M7 (AI): the city of v that computer p takes by Treachery, t + 1 */
{
    (void)p; (void)v;
    not_ported(0x20C70u);
}

static void f24EB8(int c, int v, int k)         /* M7 (AI): computer c gives k of v's coastal cities to the Pirates */
{
    (void)c; (void)v; (void)k;
    not_ported(0x24EB8u);
}

static void f252BC(int c, int v, int k)         /* M7 (AI): computer v picks k secondary coastal cities (trader c) */
{
    (void)c; (void)v; (void)k;
    not_ported(0x252BCu);
}

static void f27518(int n, int trader, int total, int kind)   /* M7 (AI): n assigns the secondary losses */
{
    (void)n; (void)trader; (void)total; (void)kind;
    not_ported(0x27518u);
}

#endif

/* ---- small shared pieces ------------------------------------------------------------------------------------------ */

/* the inline strcat copies (repne scasb, then the byte-pair loop), bounded to the caller's stack buffer */
static void cat(char *buf, size_t size, const char *s)
{
    size_t n = strlen(buf);
    if (n + 1 < size)
        snprintf(buf + n, size - n, "%s", s);
}

/* the background 3 into the back buffer: f3CD6C(3), byte [0xEE168] = 3 (stored after the call), fg_vbcopy of the
   whole screen from [0xF52CC] to [0xF52C8] (0x23284..0x232B9, 0x237F9..0x2382E, 0x23B6D..0x23B9D, 0x20A4F..0x20A83,
   0x2483F..0x24874) */
static void background3(void)
{
    f3CD6C(3);
    setD8(BGNUM, 3);
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BACKGROUND), S32(BACKBUF));
}

/* 0xF7AC4[12] cleared, then the nations whose cities the Pirates may take (0x247BA..0x24829): modes 0 / 2 the victim,
   mode 1 every nation 1..9 in play other than v and c */
static void pirate_marks(int c, int v, int mode)
{
    int32_t i;                                                      /* [ebp-0x2C] */

    for (i = 0; i < 0xC; i++)                                       /* 0x247BA */
        setD8(MARKS + (uint32_t)i, 0);                              /* 0x247C5 */
    if (mode == 0 || mode == 2) {                                   /* 0x247CD / 0x247D3 */
        setD8(MARKS + (uint32_t)v, 1);                              /* 0x247DC */
    } else if (mode == 1) {                                         /* 0x247EB */
        for (i = mode; i <= 9; i++) {                               /* 0x247ED (i = eax = 1), 0x247F8 */
            if (D8(NATION(i) + 0x9Au) == 0)                         /* 0x24805 */
                continue;
            if (i == v)                                             /* 0x24812 */
                continue;
            if (i == c)                                             /* 0x2481A */
                continue;
            setD8(MARKS + (uint32_t)i, 1);                          /* 0x24822 */
        }
    }
}

/* the snapshot of 0x2468C: area records 0..160 (69 bytes), territory states 0..159 (3) and nations 1..11 (199, at
   199n) to the heap blocks [0xEED74] / [0xEED7C] / [0xEED78] (0x246E7..0x247A1) or back (0x24D38..0x24DEE: areas,
   nations, then territory states). Port guard: a block that is not there (out of memory) is skipped; the original
   would write through a NULL pointer */
static void pirate_snapshot(int to_block)
{
    uint8_t *area = load_heap_ptr(D32(SNAP_AREA), 0x2B65u);
    uint8_t *ts = load_heap_ptr(D32(SNAP_TSTATE), 0x1E0u);
    uint8_t *nat = load_heap_ptr(D32(SNAP_NATION), 0x954u);
    uint32_t i;

    if (area != NULL) {
        for (i = 0; i < 0xA1u; i++) {                               /* 0x246E7 / 0x24D38 */
            if (to_block)
                memcpy(area + 69u * i, DP(AREA(i)), 69);            /* 0x24721 rep movsd (0x11) + movsb */
            else
                memcpy(DP(AREA(i)), area + 69u * i, 69);            /* 0x24D6E */
        }
    }
    if (to_block) {
        if (ts != NULL)
            for (i = 0; i < 0xA0u; i++)                             /* 0x24733 */
                memcpy(ts + 3u * i, DP(TSTATE(i)), 3);              /* 0x24762 movsw + movsb */
        if (nat != NULL)
            for (i = 1; i <= 0xBu; i++)                             /* 0x24776 */
                memcpy(nat + 199u * i, DP(NATION(i)), 199);         /* 0x2479C rep movsd (0x31) + movsw + movsb */
    } else {
        if (nat != NULL)
            for (i = 1; i <= 0xBu; i++)                             /* 0x24D82 */
                memcpy(DP(NATION(i)), nat + 199u * i, 199);         /* 0x24DA8 */
        if (ts != NULL)
            for (i = 0; i < 0xA0u; i++)                             /* 0x24DBC */
                memcpy(DP(TSTATE(i)), ts + 3u * i, 3);              /* 0x24DEB */
    }
}

/* ==== 0x1E0A0 CALAMITIES ========================================================================================== */

void f1E0A0(void)
{
    int32_t k;           /* [ebp-0x18] */
    int32_t v;
    int32_t c;

    memset(DP(0xF409Au), 0, 0x14);                                  /* 0x1E0C1 memset(0xF409A, 0, 0x14) */
    if (f1E340() == 0) {                                            /* 0x1E0C6, 0x1E0CD je 0x1E318 */
        f79270(100, S16(0xF06B0u), -200, DSTR(0xB19CCu), 0);        /* 0x1E331 (edx = dword [0xF06AE] sar 16; push
                                                                       eax = 0); "" */
        return;
    }
    f553C0();                                                       /* 0x1E0D3 */
    setD16(0xBB3FAu, 0xA9);                                         /* 0x1E0F2 */
    setD16(0xBB3FCu, 0x154);                                        /* 0x1E0F9 */
    setD16(0xBB3FEu, 0x12C);                                        /* 0x1E100 */
    setD16(0xBB400u, 0x23);                                         /* 0x1E107 */
    if ((D8(FLAGS) & 0x10) == 0)                                    /* 0x1E10E test ah, 0x10; jne 0x1E118 */
        f1E388();                                                   /* 0x1E113 Zeus */
    f1E5B8();                                                       /* 0x1E118 */
    setD16(0xF06AEu, 0);                                            /* 0x1E124 */
    for (k = 1; k <= 9; k++) {                                      /* 0x1E12B, 0x1E136 jg 0x1E16A */
        if (D8(NATION(k) + 0x9Au) == 0)                             /* 0x1E143 */
            continue;
        c = S16(0xF06AEu);                                          /* 0x1E14D movsx */
        setD16(0xF06AEu, (uint16_t)(c + 1));                        /* 0x1E157 */
        setD16(0xF06B0u + 2u * (uint32_t)c, (uint16_t)k);           /* 0x1E160 */
    }
    f783B8(PANEL_DLG, S32(0xF5940u));                               /* 0x1E175 */
    f7A4A0();                                                       /* 0x1E17C [ANIM] */
    for (k = 0; k < S32(CAL_COUNT); k++) {                          /* 0x1E181, 0x1E1A4 cmp eax, [0xF7808]; jge */
        v = S8(CAL_VICTIM(k));                                      /* 0x1E1BC dword [0xF77E3 + 3k] sar 24 */
        if (D8(NATION(v) + 0x9Au) != 0) {                           /* 0x1E1CB, 0x1E1D3 je 0x1E191 */
            /* 0x1E1F0..0x1E23A: board * 100 / [0xF4898] > 75 (idiv; setg) */
            setD32(0xF4204u + 4u * (uint32_t)v,
                   (uint32_t)((int32_t)S8(NATION(v) + 0x98u) * 100 / S32(0xF4898u) > 0x4B));
            f7C960();                                               /* 0x1E241 */
            setD32(0xF783Cu, (uint32_t)(int32_t)S8(CAL_ID(k)));     /* 0x1E25B dword [0xF77E1 + 3k] sar 24 */
            switch ((uint8_t)(D8(CAL_ID(k)) - 0x13u)) {             /* 0x1E272 sub al, 0x13; cmp al, 0xB; ja */
            case 0x0: f1E7A0(k); break;                             /* 0x1E18C [CAL3] Volcano / Earthquake */
            case 0x1: f20564(k); break;                             /* 0x1E28C Treachery */
            case 0x2: f25A38(k); break;                             /* 0x1E299 [CAL2] Famine */
            case 0x3: f23230(k); break;                             /* 0x1E2A6 Superstition */
            case 0x4: f29E34(k); break;                             /* 0x1E2B3 [CAL4] Civil War */
            case 0x5: f235E4(k); break;                             /* 0x1E2C0 Slave Revolt */
            case 0x6: f2D4F0(k); break;                             /* 0x1E2CD [CAL2] Flood */
            case 0x7: f217A0(k); break;                             /* 0x1E2DA [CAL5] Barbarian Hordes */
            case 0x8: f2E59C(k); break;                             /* 0x1E2E7 [CAL2] Epidemic */
            case 0x9: f237A8(k); break;                             /* 0x1E2F4 Civil Disorder */
            case 0xA: f23ACC(k); break;                             /* 0x1E301 Iconoclasm & Heresy */
            case 0xB: f241D8(k); break;                             /* 0x1E30E Piracy */
            default: break;                                         /* 0x1E276 ja 0x1E191 */
            }
        }
        f22B00();                                                   /* 0x1E191 */
        f5E7B4();                                                   /* 0x1E196 */
    }
}

/* ==== 0x1E340: any calamity held? ================================================================================= */

int f1E340(void)
{
    int32_t n;           /* [ebp-0x4] */

    for (n = 1; n <= 9; n++)                                        /* 0x1E351, 0x1E360 jg 0x1E381 */
        if (D8(NATION(n) + 0x13u) != 0)                             /* 0x1E36D, 0x1E375 */
            return 1;                                               /* 0x1E377 */
    return 0;                                                       /* 0x1E381 */
}

/* ==== 0x1E388 Zeus ================================================================================================ */

void f1E388(void)
{
    char part[100];      /* [ebp-0x94]..[ebp-0x31] */
    char text[300];      /* [ebp-0x1C0]..[ebp-0x95] */
    int32_t n;           /* [ebp-0x24] */
    uint32_t rec;        /* [ebp-0x20] */
    int8_t idx;          /* [ebp-0x1C] */
    int8_t card;         /* [ebp-0x18] */
    int32_t j;           /* [ebp-0x28] */
    int32_t disc;        /* [ebp-0x2C] */
    int32_t m;           /* [ebp-0x30] */
    int32_t cnt;

    for (n = 1; n <= 9; n++) {                                      /* 0x1E3A0, 0x1E513 jg 0x1E5AE */
        rec = NATION(n);                                            /* 0x1E52B */
        text[0] = 0;                                                /* 0x1E530 */
        if (D8(rec + 0x9Au) == 0)                                   /* 0x1E539 */
            continue;
        if (S8(rec + 0x13u) <= 2)                                   /* 0x1E546 dword [rec + 0x10] sar 24; jle */
            continue;
        disc = S8(rec + 0x13u) - 2;                                 /* 0x1E551 */
        snprintf(part, sizeof part, "Zeus takes pity upon %s and discards ", NAME(n));   /* 0x1E56E (0xB19CD) */
        cat(text, sizeof text, part);                               /* 0x1E576..0x1E5A3 */
        for (j = 0; j < disc; j++) {                                /* 0x1E5A6, 0x1E46D jge 0x1E4C3 */
            cnt = S8(rec + 0x13u);                                  /* 0x1E478 sar 24 */
            idx = (int8_t)((uint32_t)(f78AE4() * cnt) >> 15);       /* 0x1E47E game_rand, imul, shr 0xF */
            card = S8(rec + 0x14u + (uint32_t)(int32_t)idx);        /* 0x1E493 */
            f771F8(card);                                           /* 0x1E49D */
            if (j == 0)                                             /* 0x1E4A5 jne 0x1E3AC */
                snprintf(part, sizeof part, "%s", CARDNAME(card));  /* 0x1E4B9 (0xB19F3) */
            else if (disc - 1 != j)                                 /* 0x1E3BA */
                snprintf(part, sizeof part, ", %s", CARDNAME(card));    /* 0x1E3CA (0xB19F6) */
            else
                snprintf(part, sizeof part, " and %s", CARDNAME(card)); /* 0x1E3DD (0xB19FB) */
            for (m = idx; S8(rec + 0x13u) - 1 > m; m++) {           /* 0x1E3F5, 0x1E400..0x1E40D */
                setD8(rec + (uint32_t)m + 0x14u, D8(rec + (uint32_t)m + 0x15u));   /* 0x1E41B / 0x1E41E */
                /* BUG(1995): the givers +0x1C.. are not shifted with the cards (0x1E69C shifts both) */
                if (!g_original)
                    setD8(rec + (uint32_t)m + 0x1Cu, D8(rec + (uint32_t)m + 0x1Du));
            }
            setD8(rec + 0x13u, (uint8_t)(D8(rec + 0x13u) - 1u));    /* 0x1E426 dec byte [rec + 0x13] */
            setD8(rec + (uint32_t)(int32_t)S8(rec + 0x13u) + 0x14u, 0);   /* 0x1E441 */
            if (!g_original)   /* BUG(1995): the freed giver slot is not reset (0x1E69C writes 0xFF) */
                setD8(rec + (uint32_t)(int32_t)S8(rec + 0x13u) + 0x1Cu, 0xFFu);
            cat(text, sizeof text, part);                           /* 0x1E435..0x1E466 */
        }
        cat(text, sizeof text, ".");                                /* 0x1E4C3..0x1E4EF (0xB1A03) */
        f79270(-500, n, -200, text, -298);                          /* 0x1E508 (push 0xFFFFFED6 = -298: Zeus) */
    }
}

/* ==== 0x1E5B8: the calamity list ================================================================================== */

void f1E5B8(void)
{
    int32_t id;          /* [ebp-0x4] */
    int32_t n;           /* [ebp-0x10] */
    uint32_t rec;        /* [ebp-0x8] */
    int32_t j;           /* [ebp-0xC] */
    int32_t c;

    setD32(CAL_COUNT, 0);                                           /* 0x1E5D2 */
    for (id = 0x13; id <= 0x1E; id++) {                             /* 0x1E5D8, 0x1E5E3 jg 0x1E693 */
        for (n = 1; n <= 9; n++) {                                  /* 0x1E5ED, 0x1E5FC jg */
            rec = NATION(n);                                        /* 0x1E610 */
            if (D8(rec + 0x13u) == 0)                               /* 0x1E616 */
                continue;
            for (j = 0; j < 8; j++) {                               /* 0x1E61F, 0x1E62A jge */
                if ((int32_t)S8(rec + (uint32_t)j + 0x14u) != id)   /* 0x1E636 dword [rec + j + 0x11] sar 24 */
                    continue;
                c = S32(CAL_COUNT);                                 /* 0x1E649 */
                /* port guard (both modes): a 13th record would overwrite the count itself and the next ones would be
                   written past the data object (see the file comment) */
                if (c >= 12)
                    continue;
                setD8(CAL_ID(c), D8(rec + (uint32_t)j + 0x14u));    /* 0x1E65D */
                setD8(CAL_GIVER(c), D8(rec + (uint32_t)j + 0x1Cu)); /* 0x1E66C */
                setD32(CAL_COUNT, (uint32_t)(c + 1));               /* 0x1E682 */
                setD8(CAL_VICTIM(c), (uint8_t)n);                   /* 0x1E68B */
            }
        }
    }
}

/* ==== 0x1E69C: calamity record k resolved ========================================================================= */

void f1E69C(int k)
{
    int8_t v;            /* [ebp-0x8] */
    uint8_t id;          /* [ebp-0xC] */
    uint32_t rec;        /* [ebp-0x10] */
    int32_t at;          /* [ebp-0x14] */
    int32_t j;           /* [ebp-0x18] */
    int found = 0;       /* port: whether [ebp-0x14] was written */

    setD16(0xF4094u, 1);                                            /* 0x1E6B0 */
    v = S8(CAL_VICTIM(k));                                          /* 0x1E6C5 */
    id = D8(CAL_ID(k));                                             /* 0x1E6DA */
    rec = NATION((int32_t)v);                                       /* 0x1E6E3 movsx; imul */
    at = 0;                                                         /* port: uninitialised in the original */
    for (j = 0; S8(rec + 0x13u) > j; j++) {                         /* 0x1E6F9, 0x1E704 sar 24; cmp; jle 0x1E728 */
        if (D8(rec + (uint32_t)j + 0x14u) == id) {                  /* 0x1E71E cmp al, ah */
            at = j;                                                 /* 0x1E725 */
            found = 1;
            break;
        }
    }
    /* BUG(1995): a card not in the hand leaves [ebp-0x14] unwritten; the original shifts from stack garbage, drops the
       count and discards the card anyway. Fixed: the hand stays as it is and nothing is discarded. */
    if (!found && !g_original)
        return;
    for (j = at; S8(rec + 0x13u) - 1 > j; j++) {                    /* 0x1E72B, 0x1E736..0x1E743 */
        setD8(rec + (uint32_t)j + 0x14u, D8(rec + (uint32_t)j + 0x15u));   /* 0x1E751 / 0x1E754 */
        setD8(rec + (uint32_t)j + 0x1Cu, D8(rec + (uint32_t)j + 0x1Du));   /* 0x1E763 / 0x1E766 */
    }
    setD8(rec + 0x13u, (uint8_t)(D8(rec + 0x13u) - 1u));            /* 0x1E76E */
    setD8(rec + (uint32_t)(int32_t)S8(rec + 0x13u) + 0x14u, 0);     /* 0x1E77D */
    setD8(rec + (uint32_t)(int32_t)S8(rec + 0x13u) + 0x1Cu, 0xFFu); /* 0x1E78D */
    f771F8((int8_t)id);                                             /* 0x1E795 movsx */
}

/* ==== 0x20564 Treachery =========================================================================================== */

void f20564(int k)
{
    char buf[100];       /* [ebp-0x8C]..[ebp-0x29] */
    uint32_t vrec;       /* [ebp-0x28] */
    uint32_t prec;       /* [ebp-0x24] */
    int32_t idx;         /* [ebp-0x1C] */
    int32_t j;           /* [ebp-0x20] */
    int32_t t;           /* [ebp-0x18] */
    int32_t giver;
    int found = 0;       /* port: whether [ebp-0x1C] was written */

    vrec = NATION(S8(CAL_VICTIM(k)));                               /* 0x2059D */
    if (D8(vrec + 0x51u) == 0) {                                    /* 0x205A3 cmp byte [eax + 0x51], 0; je 0x2075B */
        snprintf(buf, sizeof buf, "%s has no cities and cannot be affected by Treachery.",
                 NAME(S8(vrec + 0x3Du)));                           /* 0x20778 (0xB1C8A) */
        f79270(-500, S8(vrec + 0x3Du), -200, buf, 1);               /* 0x2079B */
        f1E69C(k);                                                  /* 0x207A3 */
        return;
    }
    idx = 0;                                                        /* port: uninitialised in the original */
    for (j = 0; j < 8; j++) {                                       /* 0x205AF, 0x205BA jge 0x205D7 */
        if (S8(vrec + (uint32_t)j + 0x14u) == 0x14) {               /* 0x205C6 dword [.. + 0x11] sar 24; cmp 0x14 */
            idx = j;                                                /* 0x205D4 */
            found = 1;
            break;
        }
    }
    /* BUG(1995): no Treachery card in the hand leaves [ebp-0x1C] unwritten (the giver comes from stack garbage).
       Fixed: the giver of the calamity record (copied from the same hand by 0x1E5B8). */
    if (!found && !g_original)
        giver = S8(CAL_GIVER(k));
    else
        giver = S8(vrec + (uint32_t)idx + 0x1Cu);                   /* 0x205DD dword [.. + 0x19] sar 24 */
    if (giver != -1)                                                /* 0x205E3 cmp eax, -1; je 0x20606 */
        prec = NATION(giver);                                       /* 0x20601 */
    else
        prec = vrec;                                                /* 0x20609 drawn: the victim's own Treachery */
    if ((D8(FLAGS) & PBEM_BIT) == 0 && S8(prec + 0x9Au) == 3) {     /* 0x2060C, 0x20618 dword [+0x97] sar 24; cmp 3 */
        t = f207B4(S8(prec + 0x3Du), S8(vrec + 0x3Du), 0x14);       /* 0x2063D (eax p, edx v, ebx 0x14) */
    } else {
        if (D8(prec + 0x3Du) != D8(vrec + 0x3Du))                   /* 0x20653 cmp dl, bl */
            snprintf(buf, sizeof buf, "%s is a victim of Treachery by %s.",
                     NAME(S8(vrec + 0x3Du)), NAME(S8(prec + 0x3Du)));   /* 0x20685 (0xB1C42) */
        else
            snprintf(buf, sizeof buf, "%s is a victim of his own Treachery.", NAME(S8(vrec + 0x3Du)));   /* 0x206AC */
        f79270(-500, S8(vrec + 0x3Du), -200, buf, 1);               /* 0x206CF */
        t = f20C70(S8(prec + 0x3Du), S8(vrec + 0x3Du)) - 1;         /* 0x206E6 [AI], 0x206EB dec eax */
    }
    if (prec != vrec && S8(prec + 0x51u) < 9)                       /* 0x206F2 je; 0x20700 cmp eax, 9; jge 0x20726 */
        setD8(AREA(t + 1) + 0x3Bu + (uint32_t)(int32_t)S8(vrec + 0x3Du), 1);   /* 0x2071E [edx + eax + 0xE31F3] */
    f3A990();                                                       /* 0x20726 */
    f3CBBC();                                                       /* 0x2072B */
    f20FC4(t, S8(prec + 0x3Du), S8(vrec + 0x3Du));                  /* 0x20745 (eax t, edx p, ebx v) */
    f1E69C(k);                                                      /* 0x2074D */
}

/* ==== 0x207B4: the human's territory chooser ====================================================================== */

int f207B4(int p, int v, int kind)
{
    char buf[100];       /* [ebp-0x98]..[ebp-0x35] */
    uint32_t prec;       /* [ebp-0x14] */
    int32_t i;           /* [ebp-0x10] */
    int32_t t;           /* [ebp-0x1C] */
    int32_t sel;         /* [ebp-0x24] */
    int32_t redraw;      /* [ebp-0x20] */
    int32_t loop;        /* [ebp-0x18] */
    int32_t r;           /* [ebp-0x28] */
    uint32_t label = 0;
    int32_t c;

    buf[0] = 0;   /* port: the original leaves the stack text uninitialised for a kind other than 0x14 / 0x19 / 0x1A */
    prec = NATION(p);                                               /* 0x207E1 */
    if (kind == 0x14) {                                             /* 0x207E4 */
        if (p != v) {                                               /* 0x207F4 */
            snprintf(buf, sizeof buf, "%s is a victim of Treachery by %s.  %s select the city to be %s.",
                     NAME(v), NAME(p), NAME(p),
                     9 - S8(prec + 0x51u) != 0 ? "replaced" : "eliminated");   /* 0x2080A, 0x20848 (0xB1CD4) */
            label = 9 - S8(prec + 0x51u) != 0 ? 0xB1D15u : 0xB1D22u;    /* 0x2085E "REPLACE CITY" / "ELIMINATE CITY" */
        } else {
            snprintf(buf, sizeof buf, "%s is a victim of his own Treachery. Select the city to be reduced.",
                     NAME(v));                                      /* 0x20894 (0xB1D31) */
            label = 0xB1D75u;                                       /* 0x2089C "REDUCE CITY" */
        }
        f3C284(CHOOSE_DLG, 0x58, label);                            /* 0x2093E */
    } else if (kind == 0x1A) {                                      /* 0x208A9 */
        snprintf(buf, sizeof buf, "%s, choose territory for Barbarians to invade.", NAME(p));   /* 0x208CA (0xB1D81) */
        f3C284(CHOOSE_DLG, 0x58, 0xB1DB0u);                         /* 0x2093E "INVADE HERE" */
    } else if (kind == 0x19) {                                      /* 0x208D7 */
        snprintf(buf, sizeof buf, "%s select your coastal city to be %s due to Flooding.", NAME(v),
                 D8(NATION(v) + 0x2Au) != 0 ? "reduced" : "eliminated");   /* 0x208E3, 0x20910 (0xB1DCF) */
        f3C284(CHOOSE_DLG, 0x58, D8(NATION(v) + 0x2Au) != 0 ? 0xB1E05u : 0xB1E11u);   /* 0x2091F, 0x2093E */
    }
    if (kind != 0x1A) {                                             /* 0x20943 je 0x20A45 */
        setD32(CHOICE_N, 0);                                        /* 0x2094F */
        for (i = 0; i < 0x9D; i++)                                  /* 0x20955, 0x20960 jge */
            setD32(CHOICE + 4u * (uint32_t)i, 0xFFFFFFFFu);         /* 0x2096C */
        for (t = 0; t < 0x9D; t++) {                                /* 0x2097B, 0x209F4 jge 0x20A45 */
            if (kind == 0x14) {                                     /* 0x20A0C jne 0x20983 */
                if (D8(TSTATE(t)) != v)                             /* 0x20A20 */
                    continue;
                if ((D8(TSTATE(t) + 2u) & 1) != 1)                  /* 0x20A3E */
                    continue;
            } else {
                if (kind != 0x19)                                   /* 0x20986 */
                    continue;
                if (D8(TSTATE(t)) != v)                             /* 0x20996 */
                    continue;
                if ((D8(TSTATE(t) + 2u) & 1) != 1)                  /* 0x209B4 */
                    continue;
                if ((D8(TERR(t)) & 0x02) == 0)                      /* 0x209CA test byte [eax + 0xE1638], 2 */
                    continue;
            }
            c = S32(CHOICE_N);                                      /* 0x209D3 */
            setD32(CHOICE_N, (uint32_t)(c + 1));                    /* 0x209E0 */
            setD32(CHOICE + 4u * (uint32_t)c, (uint32_t)t);         /* 0x209E8 */
        }
    }
    f7853C(CHOOSE_DLG);                                             /* 0x20A4A */
    background3();                                                  /* 0x20A54..0x20A83 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x20A92 */
    f3AF8C(0xF51ACu, 0xFB, 0x195);                                  /* 0x20AA9 */
    f3AF8C(0xF51B4u, 0x178, 0x195);                                 /* 0x20ABD */
    f3A9F4(buf);                                                    /* 0x20AC8 */
    fg_setcolor(5);                                                 /* 0x20ACF */
    fg_vbclose();                                                   /* 0x20AD7 */
    f7876C();                                                       /* 0x20ADC */
    f3B05C(PANEL_DLG);                                              /* 0x20AE8 */
    sel = 0;                                                        /* 0x20AED */
    redraw = 1;                                                     /* 0x20AF5 */
    loop = 1;                                                       /* 0x20AF8 */
    r = 0;
    setD32(CUR_NATION, (uint32_t)v);                                /* 0x20AFE */
    while (loop != 0) {                                             /* 0x20B03 */
        fg_vbopen(S32(BACKBUF));                                    /* 0x20B13 */
        f3C9D0(0x109, 0x195, 0x6E, 0x13, 5);                        /* 0x20B31 */
        fg_setcolor(5);                                             /* 0x20B38 */
        if (g_original || S32(CHOICE_N) > 0)   /* BUG(1995): an empty list shows entry -1, "PIRACY" (0xC77D4) */
            f3A0B0(0x109, 0x199, 0x6E, TERR_NAME(S32(CHOICE + 4u * (uint32_t)sel)));   /* 0x20B60 */
        fg_vbclose();                                               /* 0x20B65 */
        f459E4(p);                                                  /* 0x20B6D */
        f3F670();                                                   /* 0x20B72 */
        if (g_original || S32(CHOICE_N) > 0)   /* BUG(1995): an empty list writes 15 into byte 0xFFB5F (f3F624(-1)) */
            f3F624(S32(CHOICE + 4u * (uint32_t)sel), 15);           /* 0x20B86 */
        f3CBBC();                                                   /* 0x20B8B */
        if (redraw != 0) {                                          /* 0x20B93 */
            if (g_original || S32(CHOICE_N) > 0)   /* BUG(1995): an empty list centres on territory -1 */
                f3DA60(S32(CHOICE + 4u * (uint32_t)sel));           /* 0x20BA1 */
            else
                f3D52C(0, 0);
            f7A4C4(-600);                                           /* 0x20BAD (eax 0xFFFFFDA8) [ANIM] */
            redraw = 0;                                             /* 0x20BB2 */
        }
        r = f78BA4(PANEL_DLG, 1);                                   /* 0x20BBF */
        if (r == -100)                                              /* 0x20BC7 */
            continue;
        if (r >= 0) {                                               /* 0x20BD6 jl 0x20BEA */
            sel = r;                                                /* 0x20BDB a map click on a listed territory */
            redraw = 1;                                             /* 0x20BDE */
            continue;
        }
        if ((uint32_t)r < (uint32_t)-0xA) {                         /* 0x20BF0 jb 0x20C51 */
            if (r == -0x10) {                                       /* 0x20C51 cmp eax, -0x10; je 0x20C3A */
                r = S32(CHOICE + 4u * (uint32_t)sel);               /* 0x20C44 */
                loop = 0;                                           /* 0x20C49 */
            }
        } else if (r == -0xA) {                                     /* 0x20BF2 jbe 0x20C19 */
            if (S32(CHOICE_N) - 1 > sel) {                          /* 0x20C1F cmp; jle */
                sel++;                                              /* 0x20C2B */
                redraw = 1;                                         /* 0x20C2E */
            }
        } else if (r == -9) {                                       /* 0x20BF4 */
            if (sel > 0) {                                          /* 0x20C01 jle */
                sel--;                                              /* 0x20C0A */
                redraw = 1;                                         /* 0x20C0D */
            }
        }
    }
    f78664(CHOOSE_DLG);                                             /* 0x20C60 */
    return r;                                                       /* 0x20C65 */
}

/* ==== 0x20FC4: Treachery's result in t (p beneficiary, v victim) ================================================== */

void f20FC4(int t, int p, int v)
{
    char text[100];      /* [ebp-0xF8]..[ebp-0x95] */
    char part[100];      /* [ebp-0x94]..[ebp-0x31] */
    uint32_t prec;       /* [ebp-0x2C] */
    uint32_t vrec;       /* [ebp-0x28] */
    int32_t j;           /* [ebp-0x24] */
    int32_t at;          /* [ebp-0x18] */
    int32_t lim;         /* [ebp-0x20] */
    int32_t cnt;         /* [ebp-0x1C] */
    int found = 0;       /* port: whether [ebp-0x18] was written */
    int an;

    prec = NATION(p);                                               /* 0x20FF1 */
    vrec = NATION(v);                                               /* 0x21002 */
    snprintf(text, sizeof text, "%s %s city in %s was ", f789EC(S8(vrec + 0x3Du)) != 0 ? "An" : "A",
             ADJ(S8(vrec + 0x3Du)), TERR_NAME(t));                  /* 0x21021 f789EC, 0x21043 (0xB1E25, 0xB1E20 /
                                                                       0xB1E23) */
    f3F670();                                                       /* 0x2104B */
    f3F624(t, 15);                                                  /* 0x21058 */
    f3DA60(t);                                                      /* 0x21060 */
    f78A20(500);                                                    /* 0x2106A */
    if (p != v) {                                                   /* 0x21072 je 0x211F7 */
        if (9 - S8(prec + 0x51u) != 0) {                            /* 0x21089 sub edx, eax; je 0x21165 */
            f78F04(t, v);                                           /* 0x21097 */
            f78E74(t, p);                                           /* 0x210A2 */
            setD8(TSTATE(t), D8(prec + 0x3Du));                     /* 0x210BB (again, after f78E74) */
            setD8(TSTATE(t) + 1u, D8(0xF40FCu));                    /* 0x210D4 */
            if ((D8(FLAGS) & PBEM_BIT) != 0 || S8(prec + 0x9Au) == 2) {   /* 0x210E0, 0x210F1 */
                an = f789EC(S8(prec + 0x3Du));                      /* 0x2110B */
                /* BUG(1995) (text): the original maps 1 to "a" and 0 to "an" ("replaced by a African city") */
                if (g_original)
                    snprintf(part, sizeof part, "replaced by %s %s city.", an != 0 ? "a" : "an",
                             ADJ(S8(prec + 0x3Du)));                /* 0x2112D (0xB1E40, 0xB1E3B / 0xB1E3D) */
                else
                    snprintf(part, sizeof part, "replaced by %s %s city.", an != 0 ? "an" : "a",
                             ADJ(S8(prec + 0x3Du)));
                cat(text, sizeof text, part);                       /* 0x21135..0x21162 */
            }
        } else {
            cat(text, sizeof text, "eliminated.");                  /* 0x21165..0x21191 (0xB1E58) */
            f78F04(t, S8(vrec + 0x3Du));                            /* 0x2119E */
        }
        if ((D8(FLAGS) & PBEM_BIT) != 0 || S8(prec + 0x9Au) == 2) { /* 0x211A3, 0x211B8 */
            f79270(2000, v, t, text, 1);                            /* 0x211D0 */
            return;
        }
        f3DA60(t);                                                  /* 0x211DD */
        f459E4(p);                                                  /* 0x211E5 */
        f3CBBC();                                                   /* 0x211EA */
        return;
    }
    /* 0x211F7: the victim's own Treachery: the city removed inline, then tokens placed */
    setD8(TSTATE(t) + 2u, (uint8_t)(D8(TSTATE(t) + 2u) & 0xFEu));   /* 0x21203 and byte [..], -2 */
    setD8(TSTATE(t), 0);                                            /* 0x21218 */
    setD8(TSTATE(t) + 1u, 0xFFu);                                   /* 0x2122A */
    at = 0;                                                         /* port: uninitialised in the original */
    for (j = 0; S8(vrec + 0x51u) > j; j++) {                        /* 0x21233, 0x2123E sar 24; cmp; jle 0x21268 */
        if ((int32_t)D8(vrec + (uint32_t)j + 0x52u) == t + 1) {     /* 0x21252 and 0xFF; 0x2125D inc edx; cmp */
            at = j;                                                 /* 0x21265 */
            found = 1;
            break;
        }
    }
    /* BUG(1995): a city not in the list leaves [ebp-0x18] unwritten; the original shifts from stack garbage and drops
       the count anyway (as rules.c's f78F04). Fixed: the list stays as it is (the state writes above still happen). */
    if (found || g_original) {
        for (j = at; S8(vrec + 0x51u) - 1 > j; j++)                 /* 0x2126B, 0x21276..0x21283 */
            setD8(vrec + (uint32_t)j + 0x52u, D8(vrec + (uint32_t)j + 0x53u));   /* 0x21291 / 0x21294 */
        setD8(vrec + 0x51u, (uint8_t)(D8(vrec + 0x51u) - 1u));      /* 0x2129C dec byte [vrec + 0x51] */
        setD8(vrec + (uint32_t)(int32_t)S8(vrec + 0x51u) + 0x52u, 0);   /* 0x212AB */
    }
    lim = (int32_t)(uint8_t)((uint8_t)(D8(TERR(t) + 1u) << 2) >> 5)      /* 0x212C0 shl al, 2; shr al, 5 */
          + (int32_t)S8(NATION(v) + 0x27u);                         /* 0x212D7 dword [0xF14F0 + 199v] sar 24 */
    cnt = S8(vrec + 0x99u) < lim ? S8(vrec + 0x99u) : lim;          /* 0x212E8 dword [+0x96] sar 24; cmp; jl */
    for (j = 0; j < cnt; j++)                                       /* 0x212FE, 0x21309 jge 0x21323 */
        f57E28(t, v, 1);                                            /* 0x2131C */
    setD8(vrec + 0x99u, (uint8_t)(D8(vrec + 0x99u) - (uint8_t)cnt));    /* 0x21329 sub byte [+0x99], al */
    setD8(vrec + 0x98u, (uint8_t)(D8(vrec + 0x98u) + (uint8_t)cnt));    /* 0x21335 add byte [+0x98], al */
    if ((D8(FLAGS) & PBEM_BIT) != 0 || S8(vrec + 0x9Au) == 2) {     /* 0x2133B, 0x21350 */
        cat(text, sizeof text, "reduced.");                         /* 0x21355..0x21381 (0xB1E64) */
        f79270(2000, v, t, text, 1);                                /* 0x21395 */
        return;
    }
    f3DA60(t);                                                      /* 0x213A5 */
    f459E4(v);                                                      /* 0x213AD */
    f3CBBC();                                                       /* 0x213B2 */
}

/* ==== 0x23230 Superstition ======================================================================================== */

void f23230(int k)
{
    char buf[200];       /* [ebp-0xF4]..[ebp-0x2D] */
    uint32_t rec;        /* [ebp-0x18] */
    int32_t n;           /* [ebp-0x20] (its address goes to f5C880) */
    int32_t cnt;         /* [ebp-0x2C] */
    int32_t i;           /* [ebp-0x28] */
    int32_t t;           /* [ebp-0x24] */

    rec = NATION(S8(CAL_VICTIM(k)));                                /* 0x2326D */
    n = S8(rec + 0x3Du);                                            /* 0x23279 dword [rec + 0x3A] sar 24 */
    setD32(CUR_NATION, (uint32_t)n);                                /* 0x2327F */
    background3();                                                  /* 0x23289..0x232B9 */
    if (D8(rec + 0x51u) == 0) {                                     /* 0x232C4 */
        snprintf(buf, sizeof buf, "%s has no cities to be affected by Superstition.", NAME(n));   /* 0x232E1 (0xB2174) */
        f79270(-500, n, -200, buf, 1);                              /* 0x232FE */
        f1E69C(k);                                                  /* 0x23565 */
        return;
    }
    if (D8(rec + 0x39u) != 0)                                       /* 0x2330B Enlightenment */
        cnt = 0;                                                    /* 0x23313 */
    else if (D8(rec + 0x38u) != 0)                                  /* 0x2331E Deism */
        cnt = 1;                                                    /* 0x23322 */
    else
        cnt = (D8(rec + 0x37u) == 0) + 2;                           /* 0x23333 sete (Mysticism); add eax, 2 */
    snprintf(buf, sizeof buf, "%s has been affected by Superstition. ", NAME(n));   /* 0x23358 (0xB21A5) */
    if (S8(rec + 0x51u) < cnt) {                                    /* 0x23366 sar 24; cmp; jge 0x233BF */
        cnt = 0;                                                    /* 0x23370 */
        cat(buf, sizeof buf, "All your cities have been reduced. ");    /* 0x23373..0x2339F (0xB21CC) */
        while (D8(rec + 0x51u) != 0)                                /* 0x233A3 */
            f78FEC((int32_t)D8(rec + 0x52u) - 1, n);                /* 0x233B8 */
    } else if (cnt == 2) {                                          /* 0x233C2 */
        cat(buf, sizeof buf, "Mysticism acts as a restraint. ");    /* 0x233C7 (0xB21F0) */
    } else if (cnt == 1) {                                          /* 0x233D1 */
        cat(buf, sizeof buf, "Deism acts as a restraint. ");        /* 0x233D6 (0xB2210) */
    } else if (cnt == 0) {                                          /* 0x233E0 */
        /* BUG(1995) (text): "nullifys" (0xB222C) */
        cat(buf, sizeof buf, g_original ? "Enlightenment nullifys the effect." : "Enlightenment nullifies the effect.");
    }
    f79270(-500, n, -200, buf, 1);                                  /* 0x23426 */
    f3A990();                                                       /* 0x2342B */
    f3CBBC();                                                       /* 0x23430 */
    if (cnt != 0) {                                                 /* 0x23435 je 0x23562 */
        for (i = 0; i < cnt; i++) {                                 /* 0x23441, 0x23487 jge 0x23562 */
            if ((D8(FLAGS) & PBEM_BIT) == 0 && S8(rec + 0x9Au) == 3) {   /* 0x23499, 0x234AA */
                snprintf(buf, sizeof buf, "You must reduce %d cit%s.", cnt - i,
                         cnt - i == 1 ? "y" : "ies");               /* 0x234B5, 0x23466 (0xB2255, 0xB224F / 0xB2251) */
                f5C880(&n, 1, buf, 0);                              /* 0x23477 (eax &n, edx 1, ebx buf, ecx 0) */
                f3A924();                                           /* 0x2347C */
            } else {
                t = f23574((int8_t)n) - 1;                          /* 0x234C5 movsx; 0x234CA dec eax */
                f3F670();                                           /* 0x234CE */
                f3F624(t, 15);                                      /* 0x234DB */
                f3DA60(t);                                          /* 0x234E3 */
                f78A20(500);                                        /* 0x234ED */
                f78FEC(t, n);                                       /* 0x234F8 */
                snprintf(buf, sizeof buf, "%s reduces a city in %s.", NAME(n), TERR_NAME(t));   /* 0x2351F (0xB226F) */
                if ((D8(AI_PAUSE) & 0x04) == 0)                     /* 0x2352C test al, 4; jne 0x23545 */
                    f79270(2000, n, t, buf, 1);                     /* 0x23558 (eax 0x7D0) */
                else
                    f79270(-500, n, t, buf, 1);                     /* 0x23558 (eax 0xFFFFFE0C) */
            }
        }
    }
    f1E69C(k);                                                      /* 0x23565 */
}

/* ==== 0x23574: the computer's city for Superstition ================================================================ */

int f23574(int n)
{
    int8_t nb = (int8_t)n;   /* [ebp-0xC] */
    int32_t t1;          /* [ebp-0x14] */
    uint32_t rec;        /* [ebp-0x10] */
    int8_t j;            /* [ebp-0x8] */

    t1 = f213C0(nb);                                                /* 0x2358C [AI] */
    rec = NATION((int32_t)nb);                                      /* 0x235A5 */
    for (j = 0; (int32_t)j < 9; j++)                                /* 0x235AA, 0x235B9 cmp eax, 9; jge 0x235D7 */
        if ((int32_t)D8(rec + (uint32_t)(int32_t)j + 0x52u) == t1)  /* 0x235C8 and 0xFF; cmp */
            return t1;                                              /* 0x235D2 */
    return -1;                                                      /* 0x235D7 */
}

/* ==== 0x235E4 Slave Revolt ======================================================================================== */

void f235E4(int k)
{
    char buf[100];       /* [ebp-0x84]..[ebp-0x21] */
    uint32_t rec;        /* [ebp-0x18] */
    int32_t n;           /* [ebp-0x1C] */

    rec = NATION(S8(CAL_VICTIM(k)));                                /* 0x2361E */
    n = S8(rec + 0x3Du);                                            /* 0x23627 */
    setD16(0xF7AFEu, (uint16_t)n);                                  /* 0x23630 */
    setD32(CUR_NATION, (uint32_t)n);                                /* 0x2363E */
    setD16(0xF7AFCu, 0xF);                                          /* 0x23643 */
    if (D8(rec + 0x29u) != 0)                                       /* 0x2364D Mining */
        setD16(0xF7AFCu, 0x14);                                     /* 0x23653 */
    if (D8(rec + 0x39u) != 0)                                       /* 0x2365F Enlightenment */
        setD16(0xF7AFCu, (uint16_t)(D16(0xF7AFCu) - 5u));           /* 0x23665 sub word [0xF7AFC], 5 */
    if (S16(0xF7AFCu) > S8(rec + 0x98u))                            /* 0x23676 dword [0xF7AFA] sar 16; cmp; jle */
        setD16(0xF7AFCu, (uint16_t)(int16_t)S8(rec + 0x98u));       /* 0x23689 movsx ax, byte [+0x98] */
    snprintf(buf, sizeof buf, "%s suffers a Slave Revolt", NAME(n));    /* 0x236AE (0xB2288) */
    if (2 * S8(rec + 0x51u) <= S8(rec + 0x98u) - S16(0xF7AFCu)) {   /* 0x236CA..0x236D9 jg 0x23724 */
        cat(buf, sizeof buf, " and emerges without any losses.");   /* 0x236DB (0xB22A2) */
        f79270(-500, n, -200, buf, 1);                              /* 0x2371D */
    } else {
        cat(buf, sizeof buf, ".");                                  /* 0x23724 (0xB22C3) */
        f79270(-500, n, -200, buf, 1);                              /* 0x23766 */
        f3A990();                                                   /* 0x2376B */
        f3CBBC();                                                   /* 0x23772 */
        buf[0] = 0;                                                 /* 0x23777 */
        f5C750(n, S16(0xF7AFCu), buf);                              /* 0x2378F (eax n, edx loss, ebx buf) */
    }
    f1E69C(k);                                                      /* 0x23797 */
}

/* ==== 0x237A8 Civil Disorder ====================================================================================== */

void f237A8(int k)
{
    char buf[100];       /* [ebp-0x90]..[ebp-0x2D] */
    uint32_t rec;        /* [ebp-0x18] */
    int32_t n;           /* [ebp-0x20] (its address goes to f5C880) */
    int32_t cnt;         /* [ebp-0x2C] */
    int32_t i;           /* [ebp-0x28] */
    int32_t t;           /* [ebp-0x24] */

    rec = NATION(S8(CAL_VICTIM(k)));                                /* 0x237E2 */
    n = S8(rec + 0x3Du);                                            /* 0x237EE */
    setD32(CUR_NATION, (uint32_t)n);                                /* 0x237F4 */
    background3();                                                  /* 0x237FE..0x2382E */
    if (D8(rec + 0x51u) == 0) {                                     /* 0x23839 */
        snprintf(buf, sizeof buf, "%s has no cities to be affected by Civil Disorder.", NAME(n));   /* 0x23856 */
        f79270(-500, n, -200, buf, 1);                              /* 0x23873 */
        f1E69C(k);                                                  /* 0x23ABA */
        return;
    }
    cnt = S8(rec + 0x51u) - 3;                                      /* 0x23886 */
    if (D8(rec + 0x30u) != 0) cnt--;                                /* 0x2388F Music */
    if (D8(rec + 0x2Fu) != 0) cnt--;                                /* 0x2389E Drama & Poetry */
    if (D8(rec + 0x33u) != 0) cnt--;                                /* 0x238AD Law */
    if (D8(rec + 0x34u) != 0) cnt--;                                /* 0x238BC Democracy */
    if (D8(rec + 0x28u) != 0) cnt++;                                /* 0x238CB Roadbuilding */
    if (D8(rec + 0x35u) != 0) cnt++;                                /* 0x238DA Military */
    if (cnt < 0)                                                    /* 0x238E6 jge */
        cnt = 0;                                                    /* 0x238EE */
    snprintf(buf, sizeof buf, "%s has been struck by Civil Disorder", NAME(n));   /* 0x23908 (0xB22F8) */
    if (cnt == 0) {                                                 /* 0x23910 jne 0x2396F */
        cat(buf, sizeof buf, " and is unaffected.");                /* 0x23916 (0xB231D) */
        f79270(-500, n, -200, buf, 1);                              /* 0x23958 */
        f1E69C(k);                                                  /* 0x23960 */
        return;
    }
    cat(buf, sizeof buf, ". ");                                     /* 0x2396F (0xB2331) */
    f79270(-500, n, -200, buf, 1);                                  /* 0x239B1 */
    for (i = 0; i < cnt; i++) {                                     /* 0x239B8, 0x239FE jge 0x23AB7 */
        if ((D8(FLAGS) & PBEM_BIT) == 0 && S8(rec + 0x9Au) == 3) {  /* 0x23A10, 0x23A21 */
            snprintf(buf, sizeof buf, "You must reduce %d cit%s.", cnt - i,
                     cnt - i == 1 ? "y" : "ies");                   /* 0x239DD (0xB233A, 0xB2334 / 0xB2336) */
            f5C880(&n, 1, buf, 0);                                  /* 0x239EE */
            f3A924();                                               /* 0x239F3 */
        } else {
            t = f213C0((int8_t)n) - 1;                              /* 0x23A3C [AI], 0x23A41 dec eax */
            f78FEC(t, n);                                           /* 0x23A4B */
            snprintf(buf, sizeof buf, "%s reduces a city in %s.", NAME(n), TERR_NAME(t));   /* 0x23A72 (0xB2354) */
            if ((D8(AI_PAUSE) & 0x04) == 0)                         /* 0x23A80 */
                f79270(2000, n, t, buf, 1);                         /* 0x23AAD */
            else
                f79270(-500, n, t, buf, 1);                         /* 0x23AAD */
        }
    }
    f1E69C(k);                                                      /* 0x23ABA */
}

/* ==== 0x23ACC Iconoclasm & Heresy ================================================================================= */

void f23ACC(int k)
{
    char buf[100];       /* [ebp-0x9C]..[ebp-0x39] */
    int32_t trader;      /* [ebp-0x28] */
    uint32_t rec;        /* [ebp-0x18] */
    int32_t n;           /* [ebp-0x1C] (its address goes to f5C880) */
    int32_t cnt;         /* [ebp-0x38] */
    int32_t i;           /* [ebp-0x34] */
    int32_t t;           /* [ebp-0x30] */
    int32_t m;           /* [ebp-0x24] (its address goes to f5C880) */
    int32_t total;       /* [ebp-0x20] (also the cities of m in the first loop) */
    int32_t left;

    trader = S8(CAL_GIVER(k));                                      /* 0x23AF9 dword [0xF77E2 + 3k] sar 24 */
    rec = NATION(S8(CAL_VICTIM(k)));                                /* 0x23B21 */
    n = S8(rec + 0x3Du);                                            /* 0x23B2A */
    setD32(CUR_NATION, (uint32_t)n);                                /* 0x23B47 */
    setD16(0xBB3FAu, 0xA9);                                         /* 0x23B51 */
    setD16(0xBB3FCu, 0x154);                                        /* 0x23B58 */
    setD16(0xBB3FEu, 0x12C);                                        /* 0x23B5F */
    setD16(0xBB400u, 0x23);                                         /* 0x23B66 */
    background3();                                                  /* 0x23B6D..0x23B9D */
    if (D8(rec + 0x51u) == 0) {                                     /* 0x23BA8 */
        snprintf(buf, sizeof buf, "%s has no cities to be affected by Iconoclasm.", NAME(n));   /* 0x23BC5 */
        f79270(-500, n, -200, buf, 1);                              /* 0x23BE2 */
        f1E69C(k);                                                  /* 0x241C8 */
        return;
    }
    cnt = 4;                                                        /* 0x23BEC */
    if (D8(rec + 0x33u) != 0) cnt--;                                /* 0x23BF6 Law */
    if (D8(rec + 0x36u) != 0) cnt--;                                /* 0x23C05 Philosophy */
    if (D8(rec + 0x3Bu) != 0) cnt -= 3;                             /* 0x23C14 Theology */
    if (D8(rec + 0x3Au) != 0) cnt++;                                /* 0x23C21 Monotheism */
    if (D8(rec + 0x28u) != 0) cnt++;                                /* 0x23C30 Roadbuilding */
    if (cnt < 0)                                                    /* 0x23C3C jge */
        cnt = 0;                                                    /* 0x23C44 */
    if (S8(rec + 0x51u) < cnt)                                      /* 0x23C50 cmp eax, cnt; jge */
        cnt = S8(rec + 0x51u);                                      /* 0x23C5E */
    snprintf(buf, sizeof buf, "%s has been struck by Iconoclasm & Heresy", NAME(n));   /* 0x23C78 (0xB239C) */
    if (cnt == 0) {                                                 /* 0x23C80 jne 0x23CD2 */
        cat(buf, sizeof buf, " and is unaffected.");                /* 0x23C86 (0xB23C6) */
        f79270(-500, n, -200, buf, 1);                              /* 0x23CC8, then jmp 0x23E23 */
    } else {
        cat(buf, sizeof buf, ".");                                  /* 0x23CD2 (0xB23DA) */
        f79270(-500, n, -200, buf, 1);                              /* 0x23D16 */
        for (i = 0; i < cnt; i++) {                                 /* 0x23D1B, 0x23D6C jge 0x23E23 */
            if ((D8(FLAGS) & PBEM_BIT) == 0 && S8(rec + 0x9Au) == 3) {   /* 0x23D7D, 0x23D8D */
                snprintf(buf, sizeof buf, "%s, as Iconoclasm's primary victim you must reduce %d cit%s.", NAME(n),
                         cnt - i, cnt - i == 1 ? "y" : "ies");      /* 0x23D4F (0xB23E2, 0xB23DC / 0xB23DE) */
                f5C880(&n, 1, buf, 0);                              /* 0x23D5C */
                f3A924();                                           /* 0x23D61 */
            } else {
                t = f213C0((int8_t)n) - 1;                          /* 0x23DA8 [AI], 0x23DAD dec eax */
                f78FEC(t, n);                                       /* 0x23DB7 */
                snprintf(buf, sizeof buf, "%s reduces a city in %s due to Iconoclasm.", NAME(n),
                         TERR_NAME(t));                             /* 0x23DDE (0xB241F) */
                if ((D8(AI_PAUSE) & 0x04) == 0)                     /* 0x23DEC */
                    f79270(2000, n, t, buf, 1);                     /* 0x23E19 */
                else
                    f79270(-500, n, t, buf, 1);                     /* 0x23E19 */
            }
        }
    }
    /* 0x23E23: the secondary victims */
    setD16(0xBB400u, 0x15);                                         /* 0x23E2D */
    for (m = 1; m <= 9; m++) {                                      /* 0x23E34, 0x23E7C jg 0x23EE9 */
        setD16(SEC_CNT + 2u * (uint32_t)m, 0);                      /* 0x23E87 */
        if (D8(NATION(m) + 0x9Au) == 0)                             /* 0x23E96 */
            continue;
        if (m == n)                                                 /* 0x23EA3 */
            continue;
        if (m == trader)                                            /* 0x23EAB */
            continue;
        if (D8(NATION(m) + 0x3Bu) != 0)                             /* 0x23EB7 Theology */
            continue;
        total = S8(NATION(m) + 0x51u);                              /* 0x23EC7 dword [0xF151A + 199m] sar 24 */
        setD16(SEC_CNT + 2u * (uint32_t)m, (uint16_t)(total <= 2 ? total : 2));   /* 0x23ED6, 0x23E3F */
        if (D8(NATION(m) + 0x36u) != 0 && S16(SEC_CNT + 2u * (uint32_t)m) == 2)   /* 0x23E4E Philosophy, 0x23E64 */
            setD16(SEC_CNT + 2u * (uint32_t)m, 1);                  /* 0x23E6C */
    }
    total = 0;                                                      /* 0x23EEB */
    for (m = 1; m <= 9; m++)                                        /* 0x23EEE, 0x23EFD jg 0x23F15 */
        total += S16(SEC_CNT + 2u * (uint32_t)m);                   /* 0x23F06 dword [2m + 0xF796A] sar 16 */
    if (total == 0) {                                               /* 0x23F15 jne 0x23F46 */
        f79270(-500, n, -200, DSTR(0xB244Au), 1);                   /* 0x23F2F "There are no eligible secondary losses" */
        f1E69C(k);                                                  /* 0x23F37 */
        return;
    }
    if ((D8(FLAGS) & PBEM_BIT) == 0 && S8(rec + 0x9Au) == 3) {      /* 0x23F4C, 0x23F61 */
        f3A924();                                                   /* 0x23F6A */
        if (total <= 2) {                                           /* 0x23F72 jg 0x23FB8 */
            for (m = 1; m <= 9; m++)                                /* 0x23F77, 0x23F86 */
                setD8(SEC_LOSS + (uint32_t)m, D8(SEC_CNT + 2u * (uint32_t)m));   /* 0x23F99 (the low byte) */
            f291D0(n, -1, DSTR(0xB2471u));                          /* 0x23FAE [CAL2] "Assignments for Iconoclasm
                                                                       secondary losses" */
        } else {
            snprintf(buf, sizeof buf, "%s, assign secondary losses for Iconoclasm", NAME(n));   /* 0x23FCF (0xB249D) */
            f29550(n, -1, 2, buf);                                  /* 0x23FEA [CAL2] */
        }
    } else {
        for (m = 1; m <= 9; m++)                                    /* 0x2400A, 0x24019 */
            setD8(SEC_LOSS + (uint32_t)m, D8(SEC_CNT + 2u * (uint32_t)m));   /* 0x2402C */
        if (total <= 2)                                             /* 0x24034 jg 0x24064 */
            f291D0(n, -1, DSTR(0xB24C8u));                          /* 0x24047 [CAL2] (the second copy of the text) */
        else
            f27518(n, trader, 2, 0x1D);                             /* 0x24074 [AI] */
    }
    /* 0x24079: each secondary victim reduces its cities */
    for (m = 1; m <= 9; m++) {                                      /* 0x24079 / 0x2404C / 0x23FEF, 0x24091 */
        for (i = 0; S8(SEC_LOSS + (uint32_t)m) > i; i++) {          /* 0x2408A, 0x240AE dword [m + 0xF5479] sar 24 */
            left = S8(SEC_LOSS + (uint32_t)m);
            if ((D8(FLAGS) & PBEM_BIT) == 0 && S8(NATION(m) + 0x9Au) == 3) {   /* 0x240C5, 0x240DE */
                snprintf(buf, sizeof buf, "%s, as an Iconoclasm secondary victim you must reduce %d cit%s.",
                         NAME(m), left - i, left - i == 1 ? "y" : "ies");   /* 0x2412A (0xB24FA, 0xB24F4 / 0xB24F6) */
                f5C880(&m, 1, buf, 0);                              /* 0x24137 (eax &m) */
                f3A924();                                           /* 0x2413C */
            } else {
                t = f213C0((int8_t)m) - 1;                          /* 0x2414A [AI], 0x2414F dec eax */
                f78FEC(t, m);                                       /* 0x24159 */
                /* BUG(1995) (text): "%s reduces a city in %s ." (0xB253A), a space before the period */
                snprintf(buf, sizeof buf, g_original ? "%s reduces a city in %s ." : "%s reduces a city in %s.",
                         NAME(m), TERR_NAME(t));                    /* 0x24180 */
                if ((D8(AI_PAUSE) & 0x04) == 0)                     /* 0x2418E */
                    f79270(2500, m, t, buf, 1);                     /* 0x241BB (eax 0x9C4) */
                else
                    f79270(-500, m, t, buf, 1);                     /* 0x241BB */
            }
        }
    }
    f1E69C(k);                                                      /* 0x241C8 */
}

/* ==== 0x241D8 Piracy ============================================================================================== */

void f241D8(int k)
{
    char buf[300];       /* [ebp-0x160]..[ebp-0x35] */
    int32_t v;           /* [ebp-0x2C] */
    int32_t c;           /* [ebp-0x28] */
    uint32_t vrec;       /* [ebp-0x34] */
    uint32_t crec;       /* [ebp-0x30] */
    int32_t room;        /* [ebp-0x1C] */
    int32_t j;           /* [ebp-0x24] */
    int32_t m;           /* [ebp-0x20] */
    int32_t pc;
    int32_t take;
    int anim;

    v = S8(CAL_VICTIM(k));                                          /* 0x24205 */
    c = S8(CAL_GIVER(k));                                           /* 0x2421D dword [0xF77E2 + 3k] sar 24 */
    vrec = NATION(v);                                               /* 0x24231 */
    if (c != -1) {                                                  /* 0x24234 */
        crec = NATION(c);                                           /* 0x24248 */
    } else {
        crec = NATION(v);                                           /* 0x2425B */
        c = v;                                                      /* 0x24261 */
    }
    setD32(CUR_NATION, (uint32_t)v);                                /* 0x24267 */
    pc = S8(NATION(10) + 0x51u);                                    /* 0x2426C dword [0xF1CE0] sar 24: Pirate cities */
    if (pc == 9) {                                                  /* 0x24274 jne 0x242A2 */
        snprintf(buf, sizeof buf, "The management of Skull & Crossbones, Inc. regret to inform you that they are "
                 "unable to add new cities to their portfolio.  %s is spared from Piracy.", NAME(v));   /* 0x24290 */
        anim = 1;                                                   /* 0x24298 push 1 */
        goto message;                                               /* 0x2429D jmp 0x24662 */
    }
    room = 9 - pc;                                                  /* 0x242A9 */
    if (room > 4)                                                   /* 0x242AC jle */
        room = 4;                                                   /* 0x242B2 */
    setD32(CHOICE_N, 0);                                            /* 0x242BB */
    for (j = 0; S8(vrec + 0x51u) > j; j++)                          /* 0x242C1, 0x242CC sar 24; cmp; jle 0x24309 */
        if ((D8(TERR((int32_t)D8(vrec + (uint32_t)j + 0x52u) - 1)) & 0x02) != 0)   /* 0x242F8 */
            setD32(CHOICE_N, (uint32_t)(S32(CHOICE_N) + 1));        /* 0x24301 inc dword [0xF7C9C] */
    if (S32(CHOICE_N) == 0) {                                       /* 0x24315 jne 0x24353 */
        /* BUG(1995) (text): "takeover" for the verb "take over" (0xB25EA) */
        snprintf(buf, sizeof buf, g_original
                 ? "The primary victim %s has no coastal cities for the Pirates to takeover."
                 : "The primary victim %s has no coastal cities for the Pirates to take over.", NAME(v));   /* 0x2432C */
        f79270(-500, v, -200, buf, 1);                              /* 0x24349 */
    } else {
        snprintf(buf, sizeof buf, "%s falls victim to Pirates.", NAME(v));   /* 0x24366 (0xB2633) */
        f79270(-500, v, -200, buf, 1);                              /* 0x24383 */
        if ((D8(FLAGS) & PBEM_BIT) == 0 && S8(crec + 0x9Au) == 3) { /* 0x2438E, 0x2439F: the chooser c */
            if (room < 2)                                           /* 0x243A6 jl 0x243C1 */
                take = room;                                        /* 0x243C1 */
            else
                take = S32(CHOICE_N) <= 2 ? S32(CHOICE_N) : 2;      /* 0x243B1 */
            f2468C(c, v, take, 0);                                  /* 0x243CA (ecx 0) */
        } else {
            f3C720(PANEL_DLG);                                      /* 0x243D6 */
            if (room < 2)                                           /* 0x243DB */
                take = room;
            else
                take = S32(CHOICE_N) <= 2 ? S32(CHOICE_N) : 2;      /* 0x243E1 */
            f24EB8(c, v, take);                                     /* 0x243FC [AI] */
            f3C6A0(PANEL_DLG);                                      /* 0x24406 */
        }
    }
    /* 0x2440B */
    if (room == 1 && room < S32(CHOICE_N)) {                        /* 0x2440E jne; 0x24413 cmp ecx, [0xF7C9C]; jge */
        snprintf(buf, sizeof buf, "The Pirates are unable to add more cities.  The remaining primary and all "
                 "secondary victims are spared.");                  /* 0x24427 (0xB264F) */
        anim = room;                                                /* 0x24432 push ecx (= 1) */
        goto message;
    }
    pc = S8(NATION(10) + 0x51u);                                    /* 0x24438 */
    if (pc == 9) {                                                  /* 0x24440 jne 0x24463 */
        snprintf(buf, sizeof buf, "The Pirates are unable to add more cities.  The secondary victims are spared.");
        anim = 1;                                                   /* 0x24459 */
        goto message;                                               /* 0x2445E */
    }
    room = 9 - pc;                                                  /* 0x24468 */
    if (room > 2)                                                   /* 0x2446D jle */
        room = 2;                                                   /* 0x24473 */
    setD32(CHOICE_N, 0);                                            /* 0x24481 */
    for (m = 1; m <= 9; m++) {                                      /* 0x24487, 0x244E2 jg 0x24510 */
        if (D8(NATION(m) + 0x9Au) == 0)                             /* 0x244EF */
            continue;
        if (m == v)                                                 /* 0x244FC */
            continue;
        if (m == c)                                                 /* 0x24504 */
            continue;
        for (j = 0; S8(NATION(m) + 0x51u) > j; j++)                 /* 0x2450B, 0x24492 */
            if ((D8(TERR((int32_t)D8(NATION(m) + (uint32_t)j + 0x52u) - 1)) & 0x02) != 0)   /* 0x244CD */
                setD32(CHOICE_N, (uint32_t)(S32(CHOICE_N) + 1));    /* 0x244D6 */
    }
    if (S32(CHOICE_N) == 0) {                                       /* 0x24516 jne 0x2454D */
        /* BUG(1995) (text): "takeover" (0xB2705) */
        snprintf(buf, sizeof buf, g_original
                 ? "Secondary victims have no coastal cities for the Pirates to takeover."
                 : "Secondary victims have no coastal cities for the Pirates to take over.");   /* 0x24526 */
        f79270(-500, v, -200, buf, 1);                              /* 0x24543 */
    } else if ((D8(FLAGS) & PBEM_BIT) == 0 && S8(vrec + 0x9Au) == 3) {   /* 0x24553, 0x24564: the victim v */
        if (room < 2)                                               /* 0x2456E jl 0x24584 */
            take = room;
        else
            take = S32(CHOICE_N) <= 2 ? S32(CHOICE_N) : 2;          /* 0x24574 (edi = the count) */
        f2468C(c, v, take, 1);                                      /* 0x2458D (ecx 1) */
    } else {
        f3A924();                                                   /* 0x24597 */
        snprintf(buf, sizeof buf, "%s selects secondary coastal cities to be taken by Pirates.", NAME(v));   /* 0x245B3 */
        if ((D8(AI_PAUSE) & 0x04) == 0)                             /* 0x245C1 */
            f79270(2500, v, -200, buf, 1);                          /* 0x245F2 (eax 0x9C4) */
        else
            f79270(-500, v, -200, buf, 1);                          /* 0x245F2 */
        f3C720(PANEL_DLG);                                          /* 0x245FC */
        f3CBBC();                                                   /* 0x24601 */
        if (room < 2)                                               /* 0x24606 */
            take = room;
        else
            take = S32(CHOICE_N) <= 2 ? S32(CHOICE_N) : 2;          /* 0x2460C */
        f252BC(c, v, take);                                         /* 0x2462B [AI] */
        f3C6A0(PANEL_DLG);                                          /* 0x24635 */
    }
    /* 0x2463A */
    if (room == 1 && room < S32(CHOICE_N)) {                        /* 0x2463D jne; 0x24642 cmp edi, [0xF7C9C]; jge */
        snprintf(buf, sizeof buf, "The Pirates are unable to add more cities.  The remaining secondary victims are "
                 "spared.");                                        /* 0x24656 (0xB2787) */
        anim = room;                                                /* 0x24661 push edi (= 1) */
        goto message;
    }
    f1E69C(k);                                                      /* 0x2467A */
    return;

message:                                                            /* 0x24662 */
    f79270(-500, v, -200, buf, anim);                               /* 0x24672 */
    f1E69C(k);                                                      /* 0x2467A */
}

/* ==== 0x2468C ChooseCitiesToLoseToPirates ========================================================================= */

void f2468C(int c, int v, int k, int mode)
{
    char buf[100];       /* [ebp-0x9C]..[ebp-0x39] */
    int32_t kind;        /* [ebp-0x10] */
    int32_t redraw;      /* [ebp-0xC] */
    int32_t sel;         /* [ebp-0x20] (its address goes to f79A9C) */
    int32_t removed;     /* [ebp-0x1C] */
    int32_t left;        /* [ebp-0x18] */
    int32_t owner;       /* [ebp-0x14] */
    int32_t r;           /* [ebp-0x28] */
    int32_t i;           /* [ebp-0x2C] */
    int32_t t;

    setD32(SNAP_AREA, load_heap_alloc(0x2B65u));                    /* 0x246B2 malloc, 0x246B7 */
    setD32(SNAP_TSTATE, load_heap_alloc(0x1E0u));                   /* 0x246C1, 0x246C6 */
    setD32(SNAP_NATION, load_heap_alloc(0x954u));                   /* 0x246D2, 0x246D7 */
    pirate_snapshot(1);                                             /* 0x246DC..0x247A1 */
    /* 0x247AA 0x78B0C("ChooseCitiesToLoseToPirates"): the no-op debug trace, dropped */
    pirate_marks(c, v, mode);                                       /* 0x247AF..0x24829 */
    kind = mode == 2 ? mode : 0x1E;                                 /* 0x2482E..0x2483C */
    background3();                                                  /* 0x2483F..0x24874 */
    fg_vbopen(S32(BACKBUF));                                        /* 0x24883 */
    f3AF8C(0xF51ACu, 0xFB, 0x195);                                  /* 0x2489A */
    f3AF8C(0xF51B4u, 0x178, 0x195);                                 /* 0x248AE */
    fg_setcolor(5);                                                 /* 0x248B5 */
    f3A3A0(0x12F, 0x185, DSTR(0xB27FBu));                           /* 0x248CC "LEFT TO" */
    f3A044(0x150, 0x185, DSTR(0xB2803u));                           /* 0x248E0 "REMOVE" */
    fg_vbclose();                                                   /* 0x248E5 */
    if (mode == 2)                                                  /* 0x248EA */
        snprintf(buf, sizeof buf, "%s, select cities to be received thru Tax Revolt.", NAME(c));   /* 0x2494A (0xB280A) */
    else if (mode == 0)                                             /* 0x2490B */
        snprintf(buf, sizeof buf, "%s, select %s coastal cities to be taken by Pirates.", NAME(c), ADJ(v));   /* 0x2492D */
    else
        snprintf(buf, sizeof buf, "%s, select secondary coastal cities to be taken by Pirates.", NAME(v));    /* 0x2494A */
    f3AB7C(buf);                                                    /* 0x24958 */
    f7876C();                                                       /* 0x2495D */
    f7853C(PIRATE_DLG);                                             /* 0x24967 */
    f3B05C(PANEL_DLG);                                              /* 0x24971 */
    f28CFC(v, kind, 0);                                             /* 0x2497E */
    redraw = 1;                                                     /* 0x24983 */
    sel = 0;                                                        /* 0x2498C */
    removed = 0;                                                    /* 0x2498F */
    left = k;                                                       /* 0x24995 */
    owner = 0;
    /* BUG(1995): an empty list with cities left to take (mode 1, see the file comment): nothing is left */
    if (!g_original && S32(CHOICE_N) == 0 && left > 0)
        left = 0;
    for (;;) {
        /* 0x24998 */
        fg_vbopen(S32(BACKBUF));                                    /* 0x2499F */
        f3C9D0(0x109, 0x195, 0x6E, 0x13, 5);                        /* 0x249BD */
        fg_setcolor(5);                                             /* 0x249C4 */
        if (g_original || S32(CHOICE_N) > 0)   /* BUG(1995): an empty list shows entry -1, "PIRACY" (0xC77D4) */
            f3A0B0(0x109, 0x199, 0x6E, TERR_NAME(S32(CHOICE + 4u * (uint32_t)sel)));   /* 0x249EC */
        f3C9D0(0x136, 0x181, 0x13, 0x13, 5);                        /* 0x24A04 */
        fg_setcolor(5);                                             /* 0x24A0B */
        snprintf(buf, sizeof buf, "%d", left);                      /* 0x24A27 itoa(left, buf, 10) */
        f3A0B0(0x136, 0x185, 0x13, buf);                            /* 0x24A3B */
        fg_vbclose();                                               /* 0x24A40 */
        f3F670();                                                   /* 0x24A45 */
        if (g_original || S32(CHOICE_N) > 0)   /* BUG(1995): an empty list writes 15 into byte 0xFFB5F (f3F624(-1)) */
            f3F624(S32(CHOICE + 4u * (uint32_t)sel), 15);           /* 0x24A59 */
        if (left > 0) {                                             /* 0x24A61 jle 0x24A85 */
            f3C3D0(PANEL_DLG, 0x2A);                                /* 0x24A6F FINISHED off */
            f3C314(PANEL_DLG, 0x3F);                                /* 0x24A7E REMOVE on */
        } else {
            f3C314(PANEL_DLG, 0x2A);                                /* 0x24A8F FINISHED on */
            f3C3D0(PANEL_DLG, 0x3F);                                /* 0x24A9E REMOVE off */
        }
        if (redraw != 0) {                                          /* 0x24AA3 */
            f459E4(v);                                              /* 0x24AAC */
            if (S32(CHOICE_N) > 0)                                  /* 0x24AB8 jle */
                f3DA60(S32(CHOICE + 4u * (uint32_t)sel));           /* 0x24AC4 */
            else
                f3D52C(0, 0);                                       /* 0x24ACF */
        }
        f3CBBC();                                                   /* 0x24AD4 */
        if (redraw != 0) {                                          /* 0x24AD9 */
            if (S8(0xF4102u) == 0xF)                                /* 0x24AE5 cmp ch, 0xF: phase 15 */
                f7A4C4(-600);                                       /* 0x24AEF [ANIM] */
            redraw = 0;                                             /* 0x24AF6 */
        }
        r = f78BA4(PANEL_DLG, 1);                                   /* 0x24B03 */
        if (r == -100)                                              /* 0x24B0B */
            continue;
        if (r >= 0) {                                               /* 0x24B1A jl 0x24B2E */
            sel = r;                                                /* 0x24B1F a map click on a listed territory */
            redraw = 1;                                             /* 0x24B22 */
            continue;
        }
        if (r == -1)                                                /* 0x24B31 je 0x24E75 */
            break;
        if ((uint32_t)r < (uint32_t)-0x63) {                        /* 0x24B40 jb 0x24E14 */
            if (r == -0x65) {                                       /* 0x24E14 REMOVE -> 0x24BAD */
                t = S32(CHOICE + 4u * (uint32_t)sel);
                if (mode == 1)                                      /* 0x24BAD */
                    setD8(MARKS + (uint32_t)D8(TSTATE(t)), 0);      /* 0x24BD3: one city per secondary victim */
                owner = D8(TSTATE(t));                              /* 0x24BF6 */
                f78F04(t, owner);                                   /* 0x24C06 */
                if (mode != 2) {                                    /* 0x24C0B je 0x24CAF */
                    f78E74(t, 10);                                  /* 0x24C24 a Pirate city */
                    setD8(AREA(t + 1) + 0x3Bu + (uint32_t)owner, 2);    /* 0x24C47 */
                    for (i = 0; i < 9; i++)                         /* 0x24C4D, 0x24C58 jge 0x24CE2 */
                        if (S8(AREA(t + 1) + 0x3Cu + (uint32_t)i) == 1)   /* 0x24C7E dword [.. + 0xE31F1] sar 24 */
                            setD8(AREA(t + 1) + 0x3Cu + (uint32_t)i, 2);  /* 0x24CA6 */
                } else {
                    f78E74(t, c);                                   /* 0x24CBC the city to the absorber */
                    setD8(AREA(t + 1) + 0x3Bu + (uint32_t)owner, 1);    /* 0x24CDB */
                }
                left--;                                             /* 0x24CE5 */
                removed = 1;                                        /* 0x24CED */
                redraw = 1;                                         /* 0x24CF0 */
            }
        } else if (r == -0x63) {                                    /* 0x24B46 jbe 0x24CF8 RESTART */
            snprintf(buf, sizeof buf, "Are you sure you want to Restart?");   /* 0x24D04 (0xB28AD) */
            if ((D8(0xF097Cu) & 0x08) == 0 || f795A8(buf) == -0x61) {   /* 0x24D12, 0x24D1D, 0x24D22 */
                pirate_snapshot(0);                                 /* 0x24D2B..0x24DEE */
                /* BUG(1995): the nation marks of mode 1 are not restored with the records */
                if (!g_original)
                    pirate_marks(c, v, mode);
                left = k;                                           /* 0x24DF3 */
                f28CFC(v, kind, 0);                                 /* 0x24DFE */
                sel = 0;                                            /* 0x24E05 */
                redraw = 1;                                         /* 0x24E08 */
                if (!g_original && S32(CHOICE_N) == 0 && left > 0)  /* BUG(1995): as above */
                    left = 0;
            }
        } else if ((uint32_t)r < (uint32_t)-0xA) {                  /* 0x24B4F jb 0x24B98 */
            /* nothing */
        } else if (r == -0xA) {                                     /* 0x24B55 jbe 0x24B7C */
            if (S32(CHOICE_N) - 1 > sel) {                          /* 0x24B82 cmp; jle */
                sel++;                                              /* 0x24B8E */
                redraw = 1;                                         /* 0x24B91 */
            }
        } else if (r == -9) {                                       /* 0x24B5A */
            if (sel > 0) {                                          /* 0x24B64 jle */
                sel--;                                              /* 0x24B6D */
                redraw = 1;                                         /* 0x24B70 */
            }
        }
        /* 0x24B98 */
        if (removed != 0) {                                         /* 0x24B9C je 0x24998 */
            for (i = sel; S32(CHOICE_N) - 1 > i; i++)               /* 0x24BA5, 0x24E28..0x24E31 */
                setD32(CHOICE + 4u * (uint32_t)i, D32(CHOICE + 4u * (uint32_t)(i + 1)));   /* 0x24E40 */
            setD32(CHOICE_N, (uint32_t)(S32(CHOICE_N) - 1));        /* 0x24E53 */
            f79A9C(&sel);                                           /* 0x24E59 */
            removed = 0;                                            /* 0x24E60 */
            f28CFC(v, kind, 0);                                     /* 0x24E6B */
            if (!g_original && S32(CHOICE_N) == 0 && left > 0)      /* BUG(1995): as above */
                left = 0;
        }
    }
    f78664(PIRATE_DLG);                                             /* 0x24E7A */
    load_heap_free(D32(SNAP_AREA));                                 /* 0x24E86 */
    setD32(SNAP_AREA, 0);                                           /* 0x24E90 */
    load_heap_free(D32(SNAP_TSTATE));                               /* 0x24E96 */
    setD32(SNAP_TSTATE, 0);                                         /* 0x24EA0 */
    load_heap_free(D32(SNAP_NATION));                               /* 0x24EA6 */
    setD32(SNAP_NATION, 0);                                         /* 0x24EAB */
}

/* ==== 0x28CFC: the choice list ==================================================================================== */

/* the shared append (0x28DC3, 0x28E5A, ...): entry [count] = t, count + 1 */
static void choice_add(int32_t t)
{
    int32_t c = S32(CHOICE_N);
    setD32(CHOICE_N, (uint32_t)(c + 1));
    setD32(CHOICE + 4u * (uint32_t)c, (uint32_t)t);
}

/* territory state t holds a city of nation n (state +2 bit 0x01, owner +0 == n) */
static int city_of(int32_t t, int32_t n)
{
    return (D8(TSTATE(t) + 2u) & 1) != 0 && (int32_t)D8(TSTATE(t)) == n;
}

void f28CFC(int n, int kind, int s)
{
    int32_t t;           /* [ebp-0x4] */
    int32_t i;           /* [ebp-0x8] */
    uint32_t a;

    setD32(CHOICE_N, 0);                                            /* 0x28D1A */
    for (i = 0; i < 0x9D; i++)                                      /* 0x28D20, 0x28D2B jge */
        setD32(CHOICE + 4u * (uint32_t)i, 0xFFFFFFFFu);             /* 0x28D37 */
    if (kind == 0x15 || kind == 4 || kind == 9) {                   /* 0x28D44..0x28D54 */
        for (t = 0; t < 0x9D; t++)                                  /* 0x28D5C, 0x28D67 jge 0x291C6 */
            if (city_of(t, n) || D8(AREA(t + 1) + 0x0Eu + (uint32_t)n) != 0)   /* 0x28D86 / 0x28DA2, 0x28DBA */
                choice_add(t);                                      /* 0x28DC3 */
    } else if (kind == 0x1B) {                                      /* 0x28DE3 */
        for (t = 0; t < 0x9D; t++)                                  /* 0x28DF9 */
            if (city_of(t, n) || S8(AREA(t + 1) + 0x0Eu + (uint32_t)n) > 1)   /* 0x28E18 / 0x28E34, 0x28E4C sar 24 */
                choice_add(t);                                      /* 0x28E5A */
    } else if (kind == 7) {                                         /* 0x28E7D */
        for (t = 0; t < 0x9D; t++)                                  /* 0x28E8F */
            if (D8(0xF7984u + (uint32_t)t) != 0)                    /* 0x28E9F */
                choice_add(t);                                      /* 0x28EA9 */
    } else if (kind == 8) {                                         /* 0x28EC9 */
        for (t = 0; t < 0x9D; t++)                                  /* 0x28EDB */
            if (D8(0xF7A24u + (uint32_t)t) != 0)                    /* 0x28EEB */
                choice_add(t);                                      /* 0x28EF5 */
    } else if (kind == 6) {                                         /* 0x28F15 */
        for (t = 0; t < 0x9D; t++)                                  /* 0x28F27 */
            if (D8(AREA(t + 1) + 0x0Eu + (uint32_t)n) != 0)         /* 0x28F47 */
                choice_add(t);                                      /* 0x28F51 */
    } else if (kind == 3) {                                         /* 0x28F71 */
        for (t = 0; t < 0x9D; t++)                                  /* 0x28F83 */
            if (D8(AREA(t + 1) + 0x24u + (uint32_t)n) != 0)         /* 0x28FA3 [.. + 0xE31DC] */
                choice_add(t);                                      /* 0x28FAD */
    } else if (kind == 2 || kind == 5) {                            /* 0x28FCD / 0x28FD2 */
        for (t = 0; t < 0x9D; t++)                                  /* 0x28FE5 */
            if (city_of(t, n))                                      /* 0x29004, 0x29020 */
                choice_add(t);                                      /* 0x29025 */
    } else if (kind == 0x1E) {                                      /* 0x29045 */
        for (t = 0; t < 0x9D; t++) {                                /* 0x2905B */
            if ((D8(TSTATE(t) + 2u) & 1) == 0)                      /* 0x2907A */
                continue;
            if (D8(MARKS + (uint32_t)D8(TSTATE(t))) == 0)           /* 0x29096 cmp byte [eax + 0xF7AC4], 0 */
                continue;
            if ((D8(TERR(t)) & 0x02) == 0)                          /* 0x290B0 */
                continue;
            choice_add(t);                                          /* 0x290B9 */
        }
    } else if (kind == 0x19) {                                      /* 0x290DC */
        a = 0xBEE08u + 22u * (uint32_t)s;                           /* 11 * s words (0x290F5..0x29101) */
        for (i = 0; S16(a + 2u) > i; i++) {                         /* 0x290E7, 0x29103 dword [2 * 11s + 0xBEE08] sar 16 */
            t = S16(a + 8u + 2u * (uint32_t)i);                     /* 0x2912C dword [2i + 22s + 0xBEE0E] sar 16 */
            if ((city_of(t, n) && (D8(TERR(t) + 1u) & 0x80) != 0)  /* 0x2914B, 0x29167, 0x2917D test [+0xE1639], 0x80 */
                || D8(AREA(t + 1) + 0x0Eu + (uint32_t)n) != 0)      /* 0x29199 */
                choice_add(t);                                      /* 0x291A6 */
        }
    }
}
