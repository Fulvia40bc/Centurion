/* Module ADV of docs/M6_PLAN.md, the human side: ACQUIRE CIVILIZATION 0x162D0 (5,100 bytes) with the affordability
   pass 0x17EDC (688), the purchase screen 0x1818C (2,400), the still-needed sum 0x18AEC (380), the hand strip 0x18C68
   (368), the number boxes 0x18DD8 (920), the goods strip 0x19170 (572) and Tool Acquisition Wrapup 0x1A3A0 (1,256).
   Literal translations; the address of each instruction a line comes from is in its comment. D = read in the code,
   G = inference. Prototypes and globals: adv.h.

     0x162D0  f162D0  ACQUIRE CIVILIZATION (phase 18 of the game loop, call at 0x55113); the human's click loop and
                      the computer's purchase are parts of it (static human_turn / computer_turn here, split at the
                      original's own branch 0x16490 / 0x16D83; RESTART returns to 0x1640F as the original jumps there)
     0x17EDC  f17EDC  the affordability pass (0x16548, 0x16E47, 0x16F0F, 0x17008, 0x17603; CARDS's 0x6E084, 0x6E097)
     0x1818C  f1818C  the purchase screen (0x1655D, 0x1684B, 0x16B41, 0x17388)
     0x18AEC  f18AEC  still needed (0x187D7, 0x18EE1)
     0x18C68  f18C68  the hand strip (0x16D6C)
     0x18DD8  f18DD8  the number boxes (0x16787, 0x167CA, 0x16831, 0x1686A, 0x16D79)
     0x19170  f19170  the goods strip (0x16D71, 0x1738D)
     0x1A3A0  f1A3A0  Tool Acquisition Wrapup (the loop's 0x55153 and 0x551F8)
   0x176BC (the credit a purchase slot uses) and 0x193AC (the goods value with the Mining bonus) are formulas.c's.

   Not ported here (each call site stops the port with not_ported(address); the static stubs below carry the names, so
   a later translation that exports one of these names collides with its stub at compile time instead of being
   shadowed by it):
     M7 (ADVAI, computers only, reached only from computer_turn):
       0x1A888  the desirability of each advance (0xF62EC[24]); 0x16E57, 0x16F1F, 0x17014
       0x177EC  the advance to buy next or -1 (one game_rand); 0x16E5F, 0x16F27, 0x1701C
       0x1B59C  which cards pay `need` points (record 0, [0xF6750]); 1 found, 0 not; 0x1718B
       0x19700  the "%s has acquired ..." box; 0x175DA
     CARDS (being translated in parallel; cards.h exports both with these prototypes):
       0x6FBDC  REVEAL GOODS (n, 1); 0x1656A, every human with an affordable advance
       0x6D0C4  the hand strip face up (7, 0x196, 0x23A, n); 0x16581
   So until CARDS lands, the port stops at 0x6FBDC at the first human who can buy something; a round in which nobody
   can buy (round 1) runs the phase to its end. Built with -DADV_TEST (port/test/adv_test.c) the six go to the test's
   recorders instead, which return.

   Play by e-mail (dropped by Erik's ruling): no PBEM routine is called from these functions. The bit-0x80 tests of
   byte [0xE1624] and the local-player byte [0xE1622] (int8, read as dword [0xE161F] sar 24) are kept literally: they
   only choose between the human screen, the computer's purchase and the bare pass f17EDC(n, -1, 0) (0x175E1).

   Watcom runtime -> standard C:
     0x8C897 itoa(v, buf, 10) -> snprintf "%d" into the caller's stack buffer (52 bytes: [ebp-0x84] of 0x162D0,
             [ebp-0x64] of 0x1818C, [ebp-0x4C] of 0x18C68 / 0x18DD8; 16 bytes: [ebp-0x2C] of 0x18AEC)
     0x7DB3D sprintf(buf, "Tool Acquisition Wrapup") (no argument) -> snprintf "%s" into 100 bytes ([ebp-0x98])
     0x822B5 malloc(199) / 0x877E6 free -> load_heap_alloc / load_heap_free for the field [0xEED78] (a port handle; the
             record copies go through load_heap_ptr, skipped when the handle is 0 as ship.c does)
     0x7D5D0 memset -> memset in g_ds; the rep movs copies -> memcpy in g_ds
   __CHK dropped. The debug stub 0x78B0C ("Acquire Civ", 0x16365) is a no-op. No game_rand f78AE4 call in these
   functions (D; 0x177EC draws one per call, M7).

   Data (D, read in the code at the addresses given; more in adv.h):
     nation n = 0xF14CC + 199n: +0x01..+0x12 int8 commodity cards (+0x08 Grain), +0x24 + i uint8 advance i held,
       +0x3E int8 treasury, +0x5E int16 advance points, +0x99 int8 stock, +0x9A 0 out / 2 computer / 3 human; record 0
       (0xF14CC) is scratch: the hand minus the goods toward purchase (0x1818C copies record n there), the cards a
       computer pays (0x1B59C), the cards spent in the wrap-up
     the discard piles 0xF07B4 + 25s, s 0..8 (rules.h); the cards spent go there through f771F8
     cost int16 0xBD480[i] read as dword [2i + 0xBD47E] sar 16; prerequisite int16 0xBD4B0[i] ([2i + 0xBD4AE] sar
       16; 0 none, else the advance index: Roadbuilding and Mining need Engineering 6, Democracy and Philosophy Law 15,
       Monotheism and Theology Enlightenment 21); credit int8 0xBD240 + 24j + i (dword [24j + i + 0xBD23D] sar 24: the
       row stride is 24, not the 25 of the inventory record) for advance i from advance j held; frame colour int16
       0xBD4E0[i] (a high byte != 0: two colours, left half the high byte, right half the low byte)
     the states int8 0xF67C4[i] read as dword [i + 0xF67C1] sar 24; the goods int8 0xF67E4[i] as [i + 0xF67E1] sar 24
     the dialog 0xF634C: int16 count (read as dword [0xF634A] sar 16), records r = 0xF634E + 20i: +0 x, +2 y, +4 w,
       +6 h (int16), +0x0E id (dword [r + 0x0C] sar 16), +0x10 enabled (dword [r + 0x0D] sar 24)
     sprites: 0xF49EC + 8n the nation's token, 0xF4ACC + 8n the large token (w / h words at +4 / +6), 0xF4C1C + 8c the
       tiny card c, 0xF4C5C the Grain marker, 0xF51AC / 0xF51B4 the left / right arrows
     the background number int8 [0xEE168] (read as dword [0xEE165] sar 24): 0x12 (buyciv) here, 0x19 the wrap-up;
       [0xF52C8] back buffer, [0xF52CC] background buffer; the hand strip rectangle int16 0x11158C..0x111592 =
       (7, 0x196, 0x23A, 0x2E); the hand spacing [0x11254C] / [0x112550] (written by f71F2C / f71FB0, info.c)
     strings: 0xB14E0 "Acquire Civ", 0xB14EC / 0xB155D / 0xB15DF "POINTS" (three copies), 0xB14F3 / 0xB1564 "Goods
       toward purchase" (two), 0xB1509 "Applied Mining to", 0xB151B "Credits Used", 0xB1528 "Goods Used", 0xB1533
       "Treasury Used", 0xB1541 "", 0xB1542 "Acquire Civilization Tools", 0xB157A "Carry Forward", 0xB1588
       "Credits", 0xB1590 "Goods", 0xB1596 "From Treas.", 0xB15A2 "Apply Mining to", 0xB15B2 "Already used to
       reduce Famine losses.", 0xBDD2C "Still needed", 0xB15D8 "Excess", 0xB169B "Tool Acquisition Wrapup", 0xB16B3
       "Acquired", 0xB16BC "Using"; no slip among them (Erik's text ruling of 19 Nov: nothing to fix here; the advance
       descriptions' "Allow" for "Allows" stay, as info.c's pop-up shows them)

   1995 bugs, fixed unless g_original (each marked BUG(1995) in the code; grades D code / G effect):
     1. 0x162D0: the treasury used [0xF67E0] is cleared only by ACQUIRE (0x1697F) and by a computer's turn (0x16DAB),
        never when a human's turn starts (0x16433 clears the carry and the Mining flags only). A human who adds
        treasury with the arrow and then leaves with FINISHED (or RESTART after a purchase does not touch it either)
        hands that amount to the next human: "From Treas." shows it, it lowers the next nation's still-needed sum,
        and ACQUIRE subtracts it from a treasury that may not hold it (int8 +0x3E goes negative) and adds it to the
        stock. Fixed: [0xF67E0] = 0 with the other per-nation resets (so also on RESTART, which restores the treasury
        anyway). D (code), G (reach: hot seat, the default 3-human game).
     2. 0x18AEC: when treasury is in use and there is an excess, the treasury is reduced and still-needed recomputed
        with the raw credit 0xF6764[s] (0x18B9F) where the first computation caps it at the cost (0x18B20). Credits
        can exceed the cost (Pottery and Clothmaking cost 45, up to 60 credit from the crafts: D, the tables), so
        one click on the treasury arrow turns credit - cost into extra "Excess", which ACQUIRE then keeps as carry
        forward (0x16938) for the next purchase. Fixed: the capped credit in the recomputation. D (code, tables),
        G (reach).
     3. 0x162D0's last store (0x17674) clears int16 [0xF409A + 2n] with n == 10 after the loop: 0xF40AE, one past the
        Grain array, is the low-memory flag main sets at 0x100E7 when there is too little XMS ("we will have to
        disable the animations and sound"; menu2.c tests it before letting AUDIO/VISUALS turn effects or animations
        on, title.c at start-up). So every ACQUIRE CIVILIZATION phase re-enables those toggles on a low-memory
        machine. Fixed: no store (the Grain values 0xF409A[1..9] are left as the original leaves them; 0x1E0A0 clears
        all ten at the next calamity phase, and only 0x1818C / 0x162D0 read them before that, D). D.
     4. 0x162D0, a computer's third purchase (0x17008..0x1702F): the advance is chosen into 0xF6E16 + 10n and its
        cost (minus its credit) is added to the points 0x1B59C pays (0x170D0), but the count int16 0xF6E76 + 2n is
        incremented only after the first two (0x16EFC, 0x16FF5). So the computer pays for three advances and gets
        two; and 0x19700 never sees a count above 2 (which is why its ", " / " and " slip with three or more names,
        M6_PLAN text slips, never shows in the original). Fixed: the count incremented when the third is chosen.
        With the fix 0x19700 (ADVAI, M7) receives 3: its join must then be fixed too (count - 2 for the ", " test).
        D (code), G (effect: M7 compares with --original).
   Kept as the original (D unless marked; for Erik where marked):
     - After ACQUIRE (0x169EB) the states are recomputed with f6DD98 (the hand without the Mining bonus) and the
       credits of 0xF6764 as 0x17EDC left them at the start: an advance bought this turn adds no credit to the
       others until the next turn, and one that is affordable only with the unused Mining bonus shows red for the
       rest of the turn (G: possibly deliberate; for Erik).
     - 0x1818C sets ACQUIRE / FINISHED by still-needed >= 0, 0x18DD8 by > 0 (with exactly 0 they differ; every
       0x1818C with an affordable selection is followed by 0x18DD8, so the screen ends as 0x18DD8 says).
     - The Mining arrows' paths (-0x43 / -0x44) both end in 0x18DD8 whether or not the choice moved.
     - The hand / goods hit tests (0x16B57, 0x16C69) look at x only and accept x0 <= x <= x0 + min(32, step); a
       card wider than the step lets the later card win only when the earlier did not hit.
     - 0x19170 reads the calamity count / ids at 0xF67F7.. (+0x13 / +0x14 of the 19-byte goods buffer, as in a nation
       record); nothing writes those bytes (D: the only references are here), so none are drawn.
     - The computer's need (0x17037) is computed from slot 0 even when slot 0 is -1 (cost index -1, int16 0xBD47E);
       the count is 0 then and nothing uses it.
     - 0x1A3A0 counts the nations that bought only up to the first one (enough for its test); the title goes through
       sprintf with no argument.
     - RESTART (0x41) is disabled once per phase (0x16393) and enabled by the first ACQUIRE (0x1691E); nothing
       disables it again, so every later human of the phase has it on before buying. Harmless: it restores that
       nation's unchanged start-of-turn record and piles and shows REVEAL GOODS again (D; noted in review).
   Easter eggs / oddities: the toggle "Alternate Tool Acquisition Colors" (bit 0 of [0xF0981]) paints affordable
   advances 0x90 instead of 0x0A (M6_PLAN egg 23); "Already used to reduce Famine losses." with one Grain marker per
   card set aside (egg 18); the wrap-up's eight invisible row buttons 0xB2..0xB9 (egg 16) are the dialog 0xBD940 of
   Civilization Tools (info.c f19C10), where a row click opens the advance pop-up: here their codes -0x50..-0x57 are
   just ignored (D: the wait 0x1A794; G: the dialog was reused, not a planned view); the debug tag "Acquire Civ"
   (egg 14); "POINTS" stored three times and "Goods toward purchase" twice (egg 28). */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "adv.h"
#include "fg.h"
#include "formulas.h"
#include "game.h"
#include "info.h"
#include "kit.h"
#include "panel.h"
#include "rules.h"
#include "state.h"
#include "ui.h"

#define NATION(n)     (0xF14CCu + 199u * (uint32_t)(n))
#define REC0          0xF14CCu
#define NAT_SIZE      199u
#define NAME(n)       DSTR(D32(0xC768Cu + 4u * (uint32_t)(n)))
#define ADV_NAME(i)   D32(0xC7DC4u + 4u * (uint32_t)(i))   /* link address of the name */
#define ADV_UNAME(i)  D32(0xC7E24u + 4u * (uint32_t)(i))   /* upper case */
#define ADV_DESC(i)   D32(0xC7E9Cu + 4u * (uint32_t)(i))
#define METAL(k)      DSTR(D32(0xBDD44u + 4u * (uint32_t)(k)))
#define COST(i)       ((int32_t)S16(0xBD480u + 2u * (uint32_t)(i)))
#define PREREQ(i)     ((int32_t)S16(0xBD4B0u + 2u * (uint32_t)(i)))
#define FRAMECOL      0xBD4E0u
#define CREDIT_TAB    0xBD240u
#define DLG           0xF634Cu
#define DLG_WRAPUP    0xBD940u
#define ID(i)         ((int)(int16_t)(0x8E + (i)))           /* add eax, 0x8E; movsx edx, ax */
#define SEL           0xF6758u   /* int32 */
#define PREV          0xF675Cu   /* int32 */
#define HANDVAL       0xF6760u   /* int32 */
#define CREDITS       0xF6764u   /* int32[24] */
#define STATES        0xF67C4u   /* int8[24] */
#define NEED          0xF67DCu   /* int32 */
#define TREAS_USED    0xF67E0u   /* int32 */
#define GOODS         0xF67E4u   /* int8[19] */
#define GOODS_STEP    0xF6738u   /* int32 */
#define GOODS_OVER    0xF673Cu   /* int32 */
#define MINING_BOUGHT 0xF6740u   /* int32 */
#define MINING_USED   0xF6744u   /* int32 */
#define AI_TREAS      0xF6750u   /* int32 */
#define MINING_FLAG   0xF6754u   /* int8 */
#define LABEL_W       0xF68ACu   /* int32 */
#define MINING_CHOICE 0xF68B0u   /* int32 */
#define MINING_COUNT  0xF68B4u   /* int32 */
#define MINING_METAL  0xF68C0u   /* int8[]: dword [choice + 0xF68BD] sar 24 */
#define CARRY         0xF68C8u   /* int32 */
#define SPENT(n)      (0xF6D54u + 19u * (uint32_t)(n))
#define BUYS(n)       (0xF6E12u + 10u * (uint32_t)(n))   /* int16[5] */
#define BUYCNT(n)     (0xF6E76u + 2u * (uint32_t)(n))    /* int16 */
#define GRAIN(n)      (0xF409Au + 2u * (uint32_t)(n))    /* int16 */
#define ANY_AFFORD    0xF4092u   /* int16 */
#define TEMP_NATION   0xEED78u   /* heap handle: one nation record */
#define BG            0xEE168u   /* int8 */
#define BACKBUF       0xF52C8u
#define BGBUF         0xF52CCu
#define CUR_NATION    0xF40F8u
#define ORDER_COUNT   0xF06AEu   /* int16 */
#define ORDER         0xF06B0u   /* int16[] */
#define PILES         0xF07B4u   /* 9 x 25 bytes */
#define HAND_STEP     0x11254Cu  /* int32 */
#define HAND_OVER     0x112550u  /* int32 */
#define MOUSE_X       0xE1628u
#define MOUSE_Y       0xE162Cu
#define MOUSE_B       0xE1630u
#define FLAGS         0xE1624u
#define PBEM_BIT      0x80u
#define LOCAL_PBEM    0xE1622u   /* int8: dword [0xE161F] sar 24 */
#define TOGGLES2      0xF0981u   /* bit 0: Alternate Tool Acquisition Colors */

/* ---- not ported here (see the file comment) ------------------------------------------------------------------- */

#ifdef ADV_TEST
void adv_test_f6FBDC(int n, int disable);
void adv_test_f6D0C4(int x, int y, int w, int n);
void adv_test_f1A888(int n);
int  adv_test_f177EC(int n);
int  adv_test_f1B59C(int n, int need);
void adv_test_f19700(int n, int count, uint32_t list);
#define f6FBDC(n, d)          adv_test_f6FBDC(n, d)
#define f6D0C4(x, y, w, n)    adv_test_f6D0C4(x, y, w, n)
#define f1A888(n)             adv_test_f1A888(n)
#define f177EC(n)             adv_test_f177EC(n)
#define f1B59C(n, need)       adv_test_f1B59C(n, need)
#define f19700(n, c, l)       adv_test_f19700(n, c, l)
#else
#include "cards.h"                              /* f6FBDC REVEAL GOODS, f6D0C4 the hand strip (cards.c) */

static void f1A888(int n)                         /* M7 (ADVAI): the desirability of each advance for computer n */
{
    (void)n;
    not_ported(0x1A888u);
}

static int f177EC(int n)                          /* M7 (ADVAI): the advance computer n buys next, or -1 */
{
    (void)n;
    not_ported(0x177ECu);
}

static int f1B59C(int n, int need)                /* M7 (ADVAI): the cards computer n pays need points with */
{
    (void)n; (void)need;
    not_ported(0x1B59Cu);
}

static void f19700(int n, int count, uint32_t list)   /* M7 (ADVAI): "%s has acquired ..." (list: int16 link address) */
{
    (void)n; (void)count; (void)list;
    not_ported(0x19700u);
}
#endif

/* ---- small helpers -------------------------------------------------------------------------------------------- */

static void itoa10(int32_t v, char *buf, size_t size)   /* 0x8C897 itoa(v, buf, 10) */
{
    snprintf(buf, size, "%ld", (long)v);
}

static int32_t abs32(int32_t v)   /* cdq; xor eax, edx; sub eax, edx (INT32_MIN stays) */
{
    return v < 0 ? (int32_t)(0u - (uint32_t)v) : v;
}

static uint8_t *temp_nation(void)   /* the 199 bytes behind the handle [0xEED78], NULL if none */
{
    return D32(TEMP_NATION) != 0 ? load_heap_ptr(D32(TEMP_NATION), NAT_SIZE) : NULL;
}

static void add8(uint32_t a, int v) { setD8(a, (uint8_t)(D8(a) + (uint8_t)v)); }   /* byte add / sub / inc / dec */

/* ==== 0x17EDC: the affordability pass ============================================================================ */

int f17EDC(int n, int budget, int draw)
{
    int32_t any = 0;                                                    /* [ebp-0x18] 0x17EFA */
    uint32_t rec = NATION(n);                                           /* [ebp-0x10] 0x17F0B */
    int32_t total;                                                      /* [ebp-0x14] */
    int32_t i, j;                                                       /* [ebp-0x20], [ebp-0x1C] */
    int32_t p, v;

    if (budget == -1) {                                                 /* 0x17F0E */
        setD32(HANDVAL, (uint32_t)f195D8(n));                           /* 0x17F1B; 0x17F20 */
        for (i = 0; i <= 0x17; i++) {                                   /* 0x17F32 */
            setD32(CREDITS + 4u * (uint32_t)i, 0);                      /* 0x17F3D */
            for (j = 0; j <= 0x17; j++) {                               /* 0x17F4F */
                if (D8(rec + 0x24u + (uint32_t)j) == 0)                 /* 0x17F5B test dl, dl */
                    continue;
                setD32(CREDITS + 4u * (uint32_t)i,                      /* 0x17F77 [24j + i + 0xBD23D] sar 24; 0x17F89 */
                       (uint32_t)(S32(CREDITS + 4u * (uint32_t)i) + S8(CREDIT_TAB + 24u * (uint32_t)j + (uint32_t)i)));
            }
        }
        total = S32(CARRY) + S32(HANDVAL) + S8(rec + 0x3Eu);            /* 0x17F92..0x17FAA */
    } else {
        total = budget;                                                 /* 0x17FB7 */
    }

    for (i = 0; i <= 0x17; i++) {                                       /* 0x17FC7 */
        f3C284(DLG, ID(i), ADV_NAME(i));                                /* 0x17FEB */
        if (D8(rec + 0x24u + (uint32_t)i) != 0) {                       /* 0x17FF6 mov dh, [eax+0x24]; test dh, dh */
            setD8(STATES + (uint32_t)i, 1);                             /* 0x18000 */
            f3C284(DLG, ID(i), ADV_UNAME(i));                           /* 0x18021 */
            if (draw == 1)                                              /* 0x18029 */
                f3BEA0(DLG, ID(i), 0x0E);                               /* 0x1815E..0x18166 */
            continue;
        }
        v = total + (budget == -1 ? S32(CREDITS + 4u * (uint32_t)i) : 0);   /* 0x18043..0x18059 */
        if (v < COST(i)) {                                              /* 0x18068 cmp; jge */
            setD8(STATES + (uint32_t)i, 0);                             /* 0x18071 mov [eax+0xF67C4], bh (0) */
            goto red;                                                   /* 0x18077 -> 0x18145 */
        }
        p = PREREQ(i);                                                  /* 0x1807F */
        if (p == 0) {                                                   /* 0x1808B */
            any = 1;                                                    /* 0x1808D */
            setD8(STATES + (uint32_t)i, 2);                             /* 0x18097 */
            if (draw == 1)                                              /* 0x180A1 */
                f3BEA0(DLG, ID(i), (D8(TOGGLES2) & 1) ? 0x90 : 0x0A);   /* 0x180AA..0x180CF */
            continue;
        }
        if (D8(rec + 0x24u + (uint32_t)p) != 0) {                       /* 0x180E9 mov dl, [eax+0x24]; test dl, dl */
            any = 1;                                                    /* 0x180F5 */
            setD8(STATES + (uint32_t)i, 2);                             /* 0x180FB */
            if (draw == 1)                                              /* 0x18105 cmp esi, ecx (1) */
                f3BEA0(DLG, ID(i), (D8(TOGGLES2) & 1) ? 0x90 : 0x0A);   /* 0x1810D..0x18132 */
            continue;
        }
        setD8(STATES + (uint32_t)i, 0);                                 /* 0x1813F mov [eax+0xF67C4], dl (0) */
red:
        if (draw == 1)                                                  /* 0x18148 */
            f3BEA0(DLG, ID(i), 0x0C);                                   /* 0x18159..0x18166 */
    }
    if (any == 1)                                                       /* 0x18170 */
        setD16(ANY_AFFORD, 1);                                          /* 0x18176 */
    return any;                                                         /* 0x1817F */
}

/* ==== 0x18AEC: still needed ======================================================================================= */

int f18AEC(void)
{
    int32_t sel = S32(SEL);
    int32_t credit = S32(CREDITS + 4u * (uint32_t)sel);                 /* [ebp-0x18] 0x18B06 */
    int32_t need;                                                       /* [ebp-0x1C] */
    int32_t t;
    char buf[16];                                                       /* [ebp-0x2C] */

    if (COST(sel) < credit)                                             /* 0x18B20 cmp edx, credit; jge */
        credit = COST(sel);                                             /* 0x18B2E */
    need = COST(sel) - credit;                                          /* 0x18B41 */
    need -= f193AC();                                                   /* 0x18B44; 0x18B49 sub edx, eax */
    t = S32(TREAS_USED);                                                /* 0x18B4B */
    need = need - t - S32(CARRY);                                       /* 0x18B59, 0x18B5B */
    if (t > 0 && need < 0) {                                            /* 0x18B60 jle; 0x18B68 jge */
        setD32(TREAS_USED, (uint32_t)(t + need));                       /* 0x18B75; 0x18B78 */
        if (S32(TREAS_USED) < 0)                                        /* 0x18B7E */
            setD32(TREAS_USED, 0);                                      /* 0x18B84 */
        /* BUG(1995): the recomputation takes the raw credit 0xF6764[s] (0x18B9F), not the credit capped at the cost
           (0x18B20): with credit > cost one treasury click adds credit - cost to the excess and so to the carry
           forward. Fixed: the capped credit. */
        need = COST(sel) - (g_original ? S32(CREDITS + 4u * (uint32_t)sel) : credit);   /* 0x18B8A..0x18BA8 */
        need -= f193AC();                                               /* 0x18BAA; 0x18BAF */
        need = need - S32(TREAS_USED) - S32(CARRY);                     /* 0x18BB1..0x18BBE */
        f3C9D0(0x239, 0x10C, 0x1A, 0x13, 5);                            /* 0x18BD9 */
        itoa10(S32(TREAS_USED), buf, sizeof buf);                       /* 0x18BEB */
        fg_setcolor(5);                                                 /* 0x18BF2 */
        f3A0B0(0x239, 0x110, 0x1A, buf);                                /* 0x18C0C */
    }
    f3A3A0(0x222, 0x134, need >= 0 ? DSTR(0xBDD2Cu) : DSTR(0xB15D8u));  /* 0x18C11..0x18C2D "Still needed" / "Excess" */
    if (need > 0)                                                       /* 0x18C32 */
        f3C48C(DLG, 0xA8);                                              /* 0x18C42 */
    else
        f3C530(DLG, 0xA8);                                              /* 0x18C53 */
    return need;                                                        /* 0x18C58 */
}

/* ==== 0x1818C: the purchase screen ================================================================================ */

void f1818C(int n, int mode)
{
    int32_t i;                                                          /* [ebp-0x2C] */
    int32_t x, y, w, h;                                                 /* [ebp-0x14], [ebp-0x20], [ebp-0x1C], [ebp-0x18] */
    int32_t colw, sel;
    int32_t maxw;                                                       /* [ebp-0x28] */
    char buf[52];                                                       /* [ebp-0x64] */
    const char *nm;

    f3CD6C(S8(BG));                                                     /* 0x181AE */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BGBUF), S32(BACKBUF));  /* 0x181DB */
    setD16(0x111590u, 0x23A);                                           /* 0x181F2 */
    setD16(0x111592u, 0x2E);                                            /* 0x181FF */
    setD16(0x11158Cu, 7);                                               /* 0x18207 */
    setD16(0x11158Eu, 0x196);                                           /* 0x1820E */
    fg_vbopen(S32(BACKBUF));                                            /* 0x18215 */
    f3A18C(0, 9, 0x280, 0xB0, DSTR(0xB1542u));                          /* 0x18233 "Acquire Civilization Tools" */
    f3A18C(0, 0x21, 0x280, 0x23, NAME(n));                              /* 0x18254 */
    if (mode == 0)                                                      /* 0x18259 */
        f71F2C(7, 0x196, 0x23A, n);                                     /* 0x18271 */
    else
        f71FB0(7, 0x196, 0x23A, n);                                     /* 0x1828A */
    if (mode == 1) {                                                    /* 0x1828F */
        fg_setcolor(0x0E);                                              /* 0x18297 */
        f3A0B0(0x249, 0x19F, 0x32, DSTR(0xB155Du));                     /* 0x182B3 "POINTS" */
        itoa10(f195D8(n), buf, sizeof buf);                             /* 0x182BB; 0x182CB */
        f3A0B0(0x249, 0x1B3, 0x32, buf);                                /* 0x182DF */
        fg_setcolor(5);                                                 /* 0x182E6 */
    }
    f3AF8C(0xF49ECu + 8u * (uint32_t)n, 0x23F, 0x16E);                  /* 0x18303 */
    itoa10(S8(NATION(n) + 0x3Eu), buf, sizeof buf);                     /* 0x18314 [199n + 0xF1507] sar 24; 0x18320 */
    fg_setcolor(5);                                                     /* 0x18327 */
    f3A044(0x259, 0x171, buf);                                          /* 0x1833C */

    for (i = 0; i <= 0x17; i++) {                                       /* 0x183EF */
        x = (int16_t)f3BBD8(DLG, ID(i));                                /* 0x18409 (movsx word at each use) */
        y = (int16_t)f3BCF4(DLG, ID(i));                                /* 0x18421 */
        w = (int16_t)f3BA30(DLG, ID(i));                                /* 0x18439 */
        h = (int16_t)f3BABC(DLG, ID(i));                                /* 0x18451 */
        colw = S16(FRAMECOL + 2u * (uint32_t)i);                        /* 0x1845C [2i + 0xBD4DE] sar 16 */
        if (((uint32_t)colw >> 8 & 0xFFu) != 0) {                       /* 0x1846B test dh, 0xFF */
            fg_setcolor(S8(FRAMECOL + 2u * (uint32_t)i + 1u));          /* 0x1834B [2i + 0xBD4DE] sar 24 */
            fg_rect(x - 2, w / 2 + x, y - 2, y + h + 1);                /* 0x1838D (w / 2: sar with the sign fix) */
            fg_setcolor(D8(FRAMECOL + 2u * (uint32_t)i));               /* 0x183A4 xor ah, ah; cwde */
            fg_rect(w / 2 + x + 1, x + w + 1, y - 2, y + h + 1);        /* 0x183E1 */
        } else {
            fg_setcolor(colw);                                          /* 0x1847E */
            fg_rect(x - 2, x + w + 1, y - 2, y + h + 1);                /* 0x183E1 (from 0x184AD) */
        }
    }

    setD32(NEED, 0);                                                    /* 0x184B9 */
    sel = S32(SEL);                                                     /* 0x184B4 */
    if (sel >= 0) {                                                     /* 0x184BF */
        if (S32(PREV) != -1)                                            /* 0x184CD */
            f3B914(DLG, (int)(int16_t)(D16(PREV) + 0x8Eu), 0);          /* 0x184E7 */
        f3B914(DLG, (int)(int16_t)(D16(SEL) + 0x8Eu), 1);               /* 0x18504 */
        setD32(PREV, (uint32_t)S32(SEL));                               /* 0x18510 */
        fg_setcolor(0x0E);                                              /* 0x18515 */
        f3A044(0x115, 0x9C, DSTR(ADV_NAME(S32(SEL))));                  /* 0x18534 */
        fg_move(0x113, 0xA6);                                           /* 0x18543 */
        nm = DSTR(ADV_NAME(S32(SEL)));                                  /* 0x18556 */
        fg_draw(fgf_width(nm, (int)strlen(nm)) + 0x115, 0xA6);          /* 0x18575 fgf_width; 0x18583 fg_draw */
        f3A42C(0x104, 0xAB, 0xD1, 0, DSTR(ADV_DESC(S32(SEL))), &maxw, 1);   /* 0x185AF */
        if (S8(STATES + (uint32_t)S32(SEL)) == 2) {                     /* 0x185B4..0x185C5 */
            memcpy(DP(REC0), DP(NATION(n)), NAT_SIZE);                  /* 0x185CB..0x185F2 rep movsd + movsw + movsb */
            memset(DP(GOODS), 0, 0x13);                                 /* 0x185F3 memset(0xF67E4, 0, 0x13) */
            fg_setcolor(5);                                             /* 0x185FA */
            fg_setcolor(5);                                             /* 0x18604 */
            f3A044(0x119, 0x51, DSTR(0xB1564u));                        /* 0x1861B "Goods toward purchase" */
            f3AF8C(0xF51ACu, 0x229, 0x10C);                             /* 0x1862F */
            f3AF8C(0xF51B4u, 0x256, 0x10C);                             /* 0x18643 */
            f3A3A0(0x222, 0xCC, DSTR(0xB157Au));                        /* 0x1865C "Carry Forward" */
            f3C9D0(0x239, 0xC7, 0x1A, 0x13, 5);                         /* 0x18672 */
            itoa10(S32(CARRY), buf, sizeof buf);                        /* 0x18684 */
            fg_setcolor(5);                                             /* 0x1868B */
            f3A0B0(0x239, 0xCB, 0x1A, buf);                             /* 0x186A5 */
            f3A3A0(0x222, 0xE3, DSTR(0xB1588u));                        /* 0x186BE "Credits" */
            f3C9D0(0x239, 0xDE, 0x1A, 0x13, 5);                         /* 0x186D4 */
            itoa10(S32(CREDITS + 4u * (uint32_t)S32(SEL)), buf, sizeof buf);   /* 0x186ED */
            fg_setcolor(5);                                             /* 0x186F4 */
            f3A0B0(0x239, 0xE2, 0x1A, buf);                             /* 0x1870E */
            f3A3A0(0x222, 0xFA, DSTR(0xB1590u));                        /* 0x18727 "Goods" */
            f3C9D0(0x239, 0xF5, 0x1A, 0x13, 5);                         /* 0x1873D */
            itoa10(f193AC(), buf, sizeof buf);                          /* 0x1874A; 0x1874F */
            fg_setcolor(5);                                             /* 0x18756 */
            f3A0B0(0x239, 0xF9, 0x1A, buf);                             /* 0x18770 */
            f3A3A0(0x222, 0x112, DSTR(0xB1596u));                       /* 0x18789 "From Treas." */
            f3C9D0(0x239, 0x10C, 0x1A, 0x13, 5);                        /* 0x1879F */
            itoa10(S32(TREAS_USED), buf, sizeof buf);                   /* 0x187B1 */
            fg_setcolor(5);                                             /* 0x187B8 */
            f3A0B0(0x239, 0x110, 0x1A, buf);                            /* 0x187D2 */
            setD32(NEED, (uint32_t)f18AEC());                           /* 0x187D7; 0x187ED */
            f3C9D0(0x239, 0x12D, 0x1A, 0x13, 5);                        /* 0x187F7 */
            itoa10(abs32(S32(NEED)), buf, sizeof buf);                  /* 0x18806..0x1880E */
            fg_setcolor(5);                                             /* 0x18815 */
            f3A0B0(0x239, 0x131, 0x1A, buf);                            /* 0x1882F */
            if (D8(MINING_FLAG) != 0) {                                 /* 0x18834 */
                f3AF8C(0xF51ACu, 0x1DE, 0xA9);                          /* 0x18850 */
                f3AF8C(0xF51B4u, 0x22B, 0xA9);                          /* 0x18869 */
                f3A0B0(0x1DE, 0x99, 0x5A, DSTR(0xB15A2u));              /* 0x1887D "Apply Mining to" */
                f3C9D0(0x1ED, 0xA9, 0x3C, 0x13, 5);                     /* 0x18898 */
                fg_setcolor(5);                                         /* 0x1889F */
                f3A0B0(0x1ED, 0xAD, 0x3C, METAL(S8(MINING_METAL + D32(MINING_CHOICE))));   /* 0x188A4..0x188CB */
            }
        }
    }

    if (S16(GRAIN(n)) != 0) {                                           /* 0x188D3 */
        fg_setcolor(5);                                                 /* 0x188E0 */
        f3A42C(0x1CC, 0x1E, 0x55, 0, DSTR(0xB15B2u), &maxw, 1);         /* 0x18904 "Already used to reduce Famine losses." */
        for (i = 0; S16(GRAIN(n)) > i; i++)                             /* 0x18916 [2n + 0xF4098] sar 16 */
            f3AF8C(0xF4C5Cu, 10 * i + 0x224, 0x18);                     /* 0x18946 */
    } else {
        fg_vbcopy(0x1CC, 0x223, 0x1E, 0x45, 0x1CC, 0x45, S32(BGBUF), S32(BACKBUF));   /* 0x18970 */
        fg_vbcopy(0x224, 0x27E, 0x18, 0x45, 0x224, 0x45, S32(BGBUF), S32(BACKBUF));   /* 0x1899A */
    }
    fg_vbclose();                                                       /* 0x189A2 */
    f3C3D0(DLG, 0xA9);                                                  /* 0x189B1 */
    f3C3D0(DLG, 0x75);                                                  /* 0x189C0 */
    f3C3D0(DLG, 0xA7);                                                  /* 0x189CF */
    f3C3D0(DLG, 0xA8);                                                  /* 0x189DE */
    f3C3D0(DLG, 0xA6);                                                  /* 0x189ED */
    f3C3D0(DLG, 0xAA);                                                  /* 0x189FC */
    f3C3D0(DLG, 0xAB);                                                  /* 0x18A0B */
    sel = S32(SEL);                                                     /* 0x18A10 */
    if (sel >= 0 && S8(STATES + (uint32_t)sel) == 2) {                  /* 0x18A18, 0x18A1A..0x18A26 */
        f3C314(DLG, 0xA9);                                              /* 0x18A32 */
        f3C314(DLG, 0x75);                                              /* 0x18A41 */
        f3C314(DLG, 0xA7);                                              /* 0x18A50 */
        f3C314(DLG, 0xA8);                                              /* 0x18A5F */
        if (D8(MINING_FLAG) != 0) {                                     /* 0x18A64 */
            f3C314(DLG, 0xAA);                                          /* 0x18A78 */
            f3C314(DLG, 0xAB);                                          /* 0x18A87 */
        }
    }
    f3B05C(DLG);                                                        /* 0x18A91 */
    f3CC14();                                                           /* 0x18A96 */
    if (S32(NEED) >= 0) {                                               /* 0x18A9B jl */
        f3C530(DLG, 0xA6);                                              /* 0x18AAE */
        f3C48C(DLG, 0x2A);                                              /* 0x18ABD */
    } else {
        f3C48C(DLG, 0xA6);                                              /* 0x18ACE */
        f3C530(DLG, 0x2A);                                              /* 0x18ADD */
    }
}

/* ==== 0x18C68: the hand strip face up ============================================================================= */

void f18C68(int n)
{
    char buf[52];                                                       /* [ebp-0x4C] */

    f3CD6C(S8(BG));                                                     /* 0x18C88 */
    fg_vbcopy(7, 0x240, 0x196, 0x1C3, 7, 0x1C3, S32(BGBUF), S32(BACKBUF));          /* 0x18CB3 */
    fg_vbcopy(0x249, 0x27A, 0x19F, 0x1BC, 0x249, 0x1BC, S32(BGBUF), S32(BACKBUF));  /* 0x18CE7 */
    fg_vbopen(S32(BACKBUF));                                            /* 0x18CF6 */
    f71FB0(7, 0x196, 0x23A, n);                                         /* 0x18D10 */
    fg_setcolor(5);                                                     /* 0x18D17 */
    fg_setcolor(0x0E);                                                  /* 0x18D21 */
    f39F90(1);                                                          /* 0x18D3D */
    f3A0B0(0x249, 0x19F, 0x32, DSTR(0xB15DFu));                         /* 0x18D47 "POINTS" */
    itoa10(f195D8(n), buf, sizeof buf);                                 /* 0x18D4F; 0x18D5F */
    f3A0B0(0x249, 0x1B3, 0x32, buf);                                    /* 0x18D73 */
    fg_vbclose();                                                       /* 0x18D78 */
    f78884();                                                           /* 0x18D7D */
    fg_vbpaste(0x249, 0x27A, 0x19F, 0x1BC, 0x249, 0x1BC);               /* 0x18DA0 */
    fg_vbpaste(7, 0x240, 0x196, 0x1C3, 7, 0x1C3);                       /* 0x18DC0 */
    f788AC();                                                           /* 0x18DC8 */
}

/* ==== 0x18DD8: the number boxes =================================================================================== */

void f18DD8(int n)
{
    char buf[52];                                                       /* [ebp-0x4C] */
    int32_t x;

    f3CD6C(S8(BG));                                                     /* 0x18DF8 */
    fg_vbopen(S32(BACKBUF));                                            /* 0x18E04 */
    f3C9D0(0x239, 0xF5, 0x1A, 0x13, 5);                                 /* 0x18E22 */
    itoa10(f193AC(), buf, sizeof buf);                                  /* 0x18E2F; 0x18E34 */
    fg_setcolor(5);                                                     /* 0x18E3B */
    f3A0B0(0x239, 0xF9, 0x1A, buf);                                     /* 0x18E55 */
    f3C9D0(0x239, 0x10C, 0x1A, 0x13, 5);                                /* 0x18E70 */
    itoa10(S32(TREAS_USED), buf, sizeof buf);                           /* 0x18E82 */
    fg_setcolor(5);                                                     /* 0x18E89 */
    f3A0B0(0x239, 0x110, 0x1A, buf);                                    /* 0x18EA3 */
    x = 0x222 - S32(LABEL_W);                                           /* 0x18EB6..0x18EC6 */
    fg_vbcopy(x, S32(LABEL_W) + x - 1, 0x134, 0x13F, x, 0x13F, S32(BGBUF), S32(BACKBUF));   /* 0x18ED9 */
    setD32(NEED, (uint32_t)f18AEC());                                   /* 0x18EE1; 0x18EF7 */
    f3C9D0(0x239, 0x12D, 0x1A, 0x13, 5);                                /* 0x18F01 */
    itoa10(abs32(S32(NEED)), buf, sizeof buf);                          /* 0x18F06..0x18F18 */
    fg_setcolor(5);                                                     /* 0x18F1F */
    f3A0B0(0x239, 0x131, 0x1A, buf);                                    /* 0x18F39 */
    if (D8(MINING_FLAG) != 0) {                                         /* 0x18F3E */
        f3C9D0(0x1ED, 0xA9, 0x3C, 0x13, 5);                             /* 0x18F5D */
        fg_setcolor(5);                                                 /* 0x18F64 */
        f3A0B0(0x1ED, 0xAD, 0x3C, METAL(S8(MINING_METAL + D32(MINING_CHOICE))));   /* 0x18F69..0x18F90 */
    }
    fg_vbclose();                                                       /* 0x18F95 */
    fg_vbcopy(0x259, 0x276, 0x171, 0x17C, 0x259, 0x17C, S32(BGBUF), S32(BACKBUF));  /* 0x18FC5 */
    fg_vbopen(S32(BACKBUF));                                            /* 0x18FD4 */
    itoa10(S8(NATION(n) + 0x3Eu) - S32(TREAS_USED), buf, sizeof buf);  /* 0x18FDC..0x18FFC */
    fg_setcolor(5);                                                     /* 0x19003 */
    f3A044(0x259, 0x171, buf);                                          /* 0x19018 */
    fg_vbclose();                                                       /* 0x1901D */
    f78884();                                                           /* 0x19022 */
    x = 0x222 - S32(LABEL_W);                                           /* 0x19027..0x19037 */
    fg_vbpaste(x, 0x222, 0x134, 0x140, x, 0x140);                       /* 0x1904A */
    fg_vbpaste(0x239, 0x252, 0xF5, 0x107, 0x239, 0x107);                /* 0x19070 */
    fg_vbpaste(0x239, 0x252, 0x10C, 0x11E, 0x239, 0x11E);               /* 0x19096 */
    fg_vbpaste(0x239, 0x252, 0x12D, 0x13F, 0x239, 0x13F);               /* 0x190BC */
    fg_vbpaste(0x259, 0x276, 0x171, 0x17C, 0x259, 0x17C);               /* 0x190E2 */
    if (D8(MINING_FLAG) != 0)                                           /* 0x190EA */
        fg_vbpaste(0x1ED, 0x228, 0xA9, 0xBB, 0x1ED, 0xBB);              /* 0x19111 */
    f788AC();                                                           /* 0x19119 */
    if (S32(NEED) > 0) {                                                /* 0x1911E jle */
        f3C530(DLG, 0xA6);                                              /* 0x19131 */
        f3C48C(DLG, 0x2A);                                              /* 0x19140 */
    } else {
        f3C48C(DLG, 0xA6);                                              /* 0x19151 */
        f3C530(DLG, 0x2A);                                              /* 0x19160 */
    }
}

/* ==== 0x19170: the goods toward purchase ========================================================================== */

void f19170(void)
{
    int32_t width;                                                      /* [ebp-0x1C] */
    int32_t i, j;                                                       /* [ebp-0x24], [ebp-0x20] */
    int32_t x;                                                          /* [ebp-0x18] */
    int32_t c;

    f3CD6C(S8(BG));                                                     /* 0x1918D */
    fg_vbcopy(0x10A, 0x259, 0x62, 0x8F, 0x10A, 0x8F, S32(BGBUF), S32(BACKBUF));    /* 0x191C0 */
    setD32(GOODS_OVER, 10);                                             /* 0x191CD */
    setD32(GOODS_STEP, 0x28);                                           /* 0x191D3 */
    do {
        width = 0;                                                      /* 0x191DB */
        for (i = 1; i <= 0x12; i++) {                                   /* 0x19200 */
            if (D8(GOODS + (uint32_t)i) == 0)                           /* 0x1920F test dl, dl */
                continue;
            c = S8(GOODS + (uint32_t)i);                                /* 0x19216 [i + 0xF67E1] sar 24 */
            if (c == 1)                                                 /* 0x1921F */
                width += S32(GOODS_STEP);                               /* 0x19224; 0x191F7 */
            else
                width += (c - 1) * S32(GOODS_OVER) + S32(GOODS_STEP);   /* 0x191E7..0x191F7 */
        }
        width += S8(GOODS + 0x13u) * S32(GOODS_STEP);                   /* 0x1922B [0xF67F4] sar 24 (byte 0xF67F7); 0x1923C */
        if (width > 0x150) {                                            /* 0x1923F */
            setD32(GOODS_STEP, (uint32_t)(S32(GOODS_STEP) - 2));        /* 0x19248; 0x19250 */
            if (S32(GOODS_OVER) > 3)                                    /* 0x19256 */
                setD32(GOODS_OVER, (uint32_t)(S32(GOODS_OVER) - 1));    /* 0x1925E */
        }
    } while (width > 0x150);                                            /* 0x19267 */

    fg_vbopen(S32(BACKBUF));                                            /* 0x1927F */
    x = 0x10A;                                                          /* 0x19287 */
    for (i = 1; i <= 0x12; i++) {                                       /* 0x192EB */
        c = S8(GOODS + (uint32_t)i);                                    /* 0x192F4 */
        if (c == 1) {                                                   /* 0x192FD */
            f3AF8C(0xF4C1Cu + 8u * (uint32_t)i, x, 0x62);               /* 0x19315 */
            x += S32(GOODS_STEP);                                       /* 0x1931A; 0x192E2 */
        } else if (c != 0) {                                            /* 0x19293 test eax, eax */
            for (j = 0; S8(GOODS + (uint32_t)i) > j; j++) {             /* 0x192A4 */
                f3AF8C(0xF4C1Cu + 8u * (uint32_t)i, x, 0x62);           /* 0x192C8 */
                x += S32(GOODS_OVER);                                   /* 0x192D2 */
            }
            x += S32(GOODS_STEP) - S32(GOODS_OVER);                     /* 0x192D7..0x192E2 */
        }
    }
    if (D8(GOODS + 0x13u) != 0) {                                       /* 0x19321 cmp byte [0xF67F7], 0 */
        for (j = 0; S8(GOODS + 0x13u) > j; j++) {                       /* 0x19337 */
            f3AF8C(0xF4C1Cu + 8u * (uint32_t)S8(GOODS + 0x14u + (uint32_t)j), x, 0x62);   /* 0x1934A [j + 0xF67F5] sar 24; 0x19360 */
            x += S32(GOODS_STEP);                                       /* 0x19365 */
        }
    }
    fg_vbclose();                                                       /* 0x1936F */
    f78884();                                                           /* 0x19374 */
    fg_vbpaste(0x10A, 0x259, 0x62, 0x8F, 0x10A, 0x8F);                  /* 0x19394 */
    f788AC();                                                           /* 0x1939C */
}

/* ==== 0x162D0: ACQUIRE CIVILIZATION ================================================================================ */

/* The x hit test of the hand strip (0x16B57..0x16C3C, counts in record 0 from x 7, steps [0x11254C] / [0x112550])
   and of the goods strip (0x16C69..0x16D4E, counts 0xF67E4 from x 0x10A, steps [0xF6738] / [0xF673C]): the commodity
   under the mouse, or -100 */
static int32_t strip_hit(uint32_t counts, int32_t x0, uint32_t step, uint32_t over)
{
    int32_t k = -100;                                                   /* [ebp-0x40] */
    int32_t w;                                                          /* [ebp-0x3C] */
    int32_t i, j;                                                       /* [ebp-0x4C], [ebp-0x50] */

    w = S32(step) < 0x20 ? S32(step) : 0x20;                            /* 0x16B6B / 0x16C80 cmp, 0x20; jl */
    for (i = 1; i <= 0x12; i++) {                                       /* 0x16B8B / 0x16C9D */
        if (S8(counts + (uint32_t)i) == 1) {                            /* 0x16B9B / 0x16CAD movsx; cmp eax, 1 */
            if (S32(MOUSE_X) >= x0 && x0 + w >= S32(MOUSE_X))           /* 0x16BAB jl, 0x16BBF jl */
                k = i;                                                  /* 0x16BC4 */
            x0 += S32(step);                                            /* 0x16BC7; 0x16C2F */
        } else if (S8(counts + (uint32_t)i) != 0) {                     /* 0x16BD7 test eax, eax */
            for (j = 0; S8(counts + (uint32_t)i) > j; j++) {            /* 0x16BE8 / 0x16CFA */
                if (S32(MOUSE_X) >= x0 && x0 + w >= S32(MOUSE_X))       /* 0x16C00, 0x16C12 */
                    k = i;                                              /* 0x16C17 */
                x0 += S32(over);                                        /* 0x16C1A */
            }
            x0 += S32(step) - S32(over);                                /* 0x16C24..0x16C2F */
        }
        if (k != -100)                                                  /* 0x16C32 cmp, -0x64; je (continue) */
            break;
    }
    return k;
}

/* The human's screen and click loop (0x164C8..0x16D7E). Returns 1 for RESTART (the original jumps to 0x1640F with
   [ebp-0x4C] = 1), 0 when the nation is done (0x17608). */
static int human_turn(int n, uint32_t rec, const uint8_t *piles, int32_t *nothing)
{
    int32_t i, j, k;                                                    /* [ebp-0x4C], [ebp-0x50], [ebp-0x40] */
    int32_t code;                                                       /* [ebp-0x48] */
    int32_t c, v, p, color;
    uint32_t r;                                                         /* [ebp-0x38] */
    uint8_t *backup;
    char buf[52];                                                       /* [ebp-0x84] */
    int mx, my, mb;

    setD32(SEL, 0xFFFFFFFFu);                                           /* 0x164CD */
    setD32(PREV, 0xFFFFFFFFu);                                          /* 0x164D5 */
    setD16(BUYCNT(n), 0);                                               /* 0x164DE xor ecx, ebx (-1 ^ -1); 0x164E2 */
    for (i = 0; i <= 0x17; i++) {                                       /* 0x164F5 */
        f3B914(DLG, ID(i), 0);                                          /* 0x1650D */
        f3C284(DLG, ID(i), ADV_NAME(i));                                /* 0x1652C */
    }
    f194E4(n);                                                          /* 0x16536 */
    if (f17EDC(n, -1, 1) == 0)                                          /* 0x16548; 0x1654F */
        return 0;
    *nothing = 0;                                                       /* 0x16557 */
    f1818C(n, 0);                                                       /* 0x1655D */
    f6FBDC(n, 1);                                                       /* 0x1656A */
    f6D0C4(7, 0x196, 0x23A, n);                                         /* 0x16581 */
    fg_setcolor(0x0E);                                                  /* 0x16588 */
    f3A0B0(0x249, 0x19F, 0x32, DSTR(0xB14ECu));                         /* 0x165A4 "POINTS" */
    itoa10(f195D8(n), buf, sizeof buf);                                 /* 0x165AC; 0x165C2 */
    f3A0B0(0x249, 0x1B3, 0x32, buf);                                    /* 0x165D6 */
    fg_setcolor(5);                                                     /* 0x165DD */
    f3CD6C(S8(BG));                                                     /* 0x165E2..0x165ED */

L_165F2:
    f775F0();                                                           /* 0x165F2 */
L_165F7:
    while (S32(MOUSE_B) == 0) {                                         /* 0x165F7 cmp dword [0xE1630], 0; jne */
        fg_mousepos(&mx, &my, &mb);                                     /* 0x1660F fg_mousepos(0xE1628, 0xE162C, 0xE1630) */
        setD32(MOUSE_X, (uint32_t)mx);
        setD32(MOUSE_Y, (uint32_t)my);
        setD32(MOUSE_B, (uint32_t)mb);
    }
    code = -100;                                                        /* 0x16619 */
    for (i = 0; i < S16(DLG); i++) {                                    /* 0x1662D [0xF634A] sar 16 */
        r = DLG + 2u + 20u * (uint32_t)i;                               /* 0x16654 */
        if (S8(r + 0x10u) != 1)                                         /* 0x1665A [r + 0x0D] sar 24 */
            continue;
        if (S16(r) > S32(MOUSE_X) || S16(r) + S16(r + 4u) <= S32(MOUSE_X))     /* 0x16671 jg, 0x16680 jle */
            continue;
        if (S16(r + 2u) > S32(MOUSE_Y) || S16(r + 2u) + S16(r + 6u) <= S32(MOUSE_Y))   /* 0x16692 jg, 0x166A1 jle */
            continue;
        f3C5D4(DLG, S16(r + 0x0Eu));                                    /* 0x166B3 */
        code = f3FB94(S16(r + 0x0Eu));                                  /* 0x166C1; 0x166C6 */
        break;                                                          /* falls into 0x166C9 */
    }

    if (code == -100)                                                   /* 0x166C9 */
        goto L_165F2;
    if (code == -1)                                                     /* 0x166D6 FINISHED */
        return 0;
    if (code == -0x63) {                                                /* 0x166E2 RESTART */
        backup = temp_nation();
        if (backup != NULL)
            memcpy(DP(NATION(n)), backup, NAT_SIZE);                    /* 0x166EB..0x16707 rep movsd + movsw + movsb */
        for (i = 0; i < 9; i++)                                         /* 0x16715 */
            memcpy(DP(PILES + 25u * (uint32_t)i), piles + 25 * i, 25);  /* 0x1674B rep movsd (6) + movsb */
        setD16(BUYCNT(n), 0);                                           /* 0x16755 */
        return 1;                                                       /* 0x1675D [ebp-0x4C] = 1; jmp 0x1640F */
    }
    if (code == -0x40) {                                                /* 0x1676C treasury - 1 */
        if (S32(TREAS_USED) > 0)                                        /* 0x16777 */
            setD32(TREAS_USED, (uint32_t)(S32(TREAS_USED) - 1));        /* 0x1677E */
        f18DD8(n);                                                      /* 0x16787 */
        f78A20(0x96);                                                   /* 0x16793 */
        setD32(MOUSE_B, 0);                                             /* 0x16798 */
        goto L_165F7;
    }
    if (code == -0x41) {                                                /* 0x167A6 treasury + 1 */
        if (S8(rec + 0x3Eu) > S32(TREAS_USED))                          /* 0x167AE [rec + 0x3B] sar 24; 0x167BA jle */
            setD32(TREAS_USED, (uint32_t)(S32(TREAS_USED) + 1));        /* 0x167C1 */
        f18DD8(n);                                                      /* 0x167CA */
        f78A20(0x96);                                                   /* 0x167D6 */
        setD32(MOUSE_B, 0);                                             /* 0x167DB */
        goto L_165F7;
    }
    if (code == -0x43) {                                                /* 0x167E9 Mining choice - 1 */
        if (S32(MINING_CHOICE) > 0)                                     /* 0x167F4 */
            setD32(MINING_CHOICE, (uint32_t)(S32(MINING_CHOICE) - 1));  /* 0x167FF */
        goto L_16D76;
    }
    if (code == -0x44) {                                                /* 0x1680C Mining choice + 1 */
        if (S32(MINING_COUNT) - 1 > S32(MINING_CHOICE)) {               /* 0x1681D jle */
            setD32(MINING_CHOICE, (uint32_t)(S32(MINING_CHOICE) + 1));  /* 0x16828 */
            f18DD8(n);                                                  /* 0x16831 */
            goto L_165F2;
        }
        goto L_16D76;
    }
    if (code == -0x3E) {                                                /* 0x1683E an advance */
        f1818C(n, 1);                                                   /* 0x1684B */
        if (S8(STATES + (uint32_t)S32(SEL)) == 2)                       /* 0x16850..0x1685E */
            f18DD8(n);                                                  /* 0x1686A */
        goto L_165F2;
    }
    if (code == -0x3F) {                                                /* 0x16877 ACQUIRE */
        setD8(rec + 0x24u + (uint32_t)S32(SEL), 1);                     /* 0x16889 */
        if (S32(SEL) == 5)                                              /* 0x16893 Mining */
            setD32(MINING_BOUGHT, 1);                                   /* 0x16898 */
        c = S16(BUYCNT(n));                                             /* 0x168A5 */
        setD16(BUYCNT(n), (uint16_t)(c + 1));                           /* 0x168B3 */
        setD16(BUYS(n) + 2u * (uint32_t)c, D16(SEL));                   /* 0x168CB mov ax, word [0xF6758]; 0x168D1 */
        f3BEA0(DLG, (int)(int16_t)(D16(SEL) + 0x8Eu), 0x0F);            /* 0x168D8..0x168EA */
        f3C284(DLG, (int)(int16_t)(D16(SEL) + 0x8Eu), ADV_UNAME(S32(SEL)));   /* 0x168EF..0x1690F */
        f3C314(DLG, 0x41);                                              /* 0x1691E RESTART enabled */
        f3C48C(DLG, 0x2A);                                              /* 0x1692D FINISHED enabled */
        if (S32(NEED) < 0)                                              /* 0x16938 jge */
            setD32(CARRY, 0u - D32(NEED));                              /* 0x1693E (the excess), 0x16946 = -excess */
        else
            setD32(CARRY, 0);                                           /* 0x16950 */
        add8(rec + 0x3Eu, -(int)D8(TREAS_USED));                        /* 0x16955..0x1695E sub byte [rec + 0x3E], dl */
        add8(rec + 0x99u, (int)D8(TREAS_USED));                         /* 0x16961..0x16974 */
        setD32(TREAS_USED, 0);                                          /* 0x1697F */
        for (i = 1; i <= 0x12; i++)                                     /* 0x16988 */
            for (j = 0; S8(GOODS + (uint32_t)i) > j; j++) {             /* 0x169AE [i + 0xF67E1] sar 24 */
                add8(SPENT(n) + (uint32_t)i, 1);                        /* 0x169D3 */
                add8(rec + (uint32_t)i, -1);                            /* 0x169DF */
                f771F8(i);                                              /* 0x169E4 */
            }
        setD32(HANDVAL, (uint32_t)f6DD98(n));                           /* 0x169EE; 0x169F3 */
        for (k = 0; k <= 0x17; k++) {                                   /* 0x16AE8 */
            if (D8(rec + 0x24u + (uint32_t)k) != 0) {                   /* 0x16AF4 mov ch, [eax+0x24] */
                setD8(STATES + (uint32_t)k, 1);                         /* 0x16B02 */
                continue;
            }
            v = S32(HANDVAL) + (S32(CARRY) + S32(CREDITS + 4u * (uint32_t)k)) + S8(rec + 0x3Eu);   /* 0x16A02..0x16A24 */
            if (v < COST(k)) {                                          /* 0x16A33 jge */
                setD8(STATES + (uint32_t)k, 0);                         /* 0x16A3A mov [eax+0xF67C4], ch (0) */
                color = 0x0C;                                           /* 0x16AD0 */
            } else {
                p = PREREQ(k);                                          /* 0x16A48 */
                if (p == 0) {                                           /* 0x16A54 */
                    setD8(STATES + (uint32_t)k, 2);                     /* 0x16A59 */
                    color = (D8(TOGGLES2) & 1) ? 0x90 : 0x0A;           /* 0x16A60..0x16A70 */
                } else if (D8(rec + 0x24u + (uint32_t)p) != 0) {        /* 0x16A8F mov cl, [eax+0x24]; test cl, cl */
                    setD8(STATES + (uint32_t)k, 2);                     /* 0x16A99 */
                    color = (D8(TOGGLES2) & 1) ? 0x90 : 0x0A;           /* 0x16AA0..0x16AB0 */
                } else {
                    setD8(STATES + (uint32_t)k, 0);                     /* 0x16AC2 mov [eax+0xF67C4], cl (0) */
                    color = 0x0C;                                       /* 0x16AD0 */
                }
            }
            f3BEA0(DLG, ID(k), color);                                  /* 0x16AD5..0x16ADD */
        }
        if (S32(MINING_CHOICE) != 0) {                                  /* 0x16B0B (the bonus was applied) */
            f194E4(n);                                                  /* 0x16B17 */
            setD32(MINING_USED, 1);                                     /* 0x16B23 */
            setD8(MINING_FLAG, 0);                                      /* 0x16B29 */
        } else {
            f194E4(n);                                                  /* 0x16B34 */
        }
        f1818C(n, 1);                                                   /* 0x16B41 */
        goto L_165F2;
    }
    if (code == -0x1D) {                                                /* 0x16B4E a card of the hand */
        k = strip_hit(REC0, 7, HAND_STEP, HAND_OVER);                   /* 0x16B57..0x16C3C */
        if (k == -100)                                                  /* 0x16C3C */
            goto L_165F2;
        add8(REC0 + (uint32_t)k, -1);                                   /* 0x16C49 dec byte [k + 0xF14CC] */
        add8(GOODS + (uint32_t)k, 1);                                   /* 0x16C52 inc byte [k + 0xF67E4] */
        goto L_16D6A;
    }
    if (code == -0x42) {                                                /* 0x16C60 a card of the goods */
        k = strip_hit(GOODS, 0x10A, GOODS_STEP, GOODS_OVER);            /* 0x16C69..0x16D4E */
        if (k == -100)                                                  /* 0x16D4E */
            goto L_165F2;
        add8(REC0 + (uint32_t)k, 1);                                    /* 0x16D5B */
        add8(GOODS + (uint32_t)k, -1);                                  /* 0x16D64 */
        goto L_16D6A;
    }
    goto L_165F2;                                                       /* 0x16C63 jne 0x165F2 */

L_16D6A:
    f18C68(0);                                                          /* 0x16D6C */
    f19170();                                                           /* 0x16D71 */
L_16D76:
    f18DD8(n);                                                          /* 0x16D79 */
    goto L_165F2;                                                       /* 0x16D7E */
}

/* A computer's purchase (0x16D98..0x175DF): up to three advances chosen by M7's routines, paid with record 0's
   cards (0x1B59C) and treasury [0xF6750], and the summary over the purchase screen. */
static void computer_turn(int n, uint32_t rec, int32_t *nothing)
{
    int32_t i, j;                                                       /* [ebp-0x4C], [ebp-0x50] */
    int32_t budget;                                                     /* [ebp-0x24] */
    int32_t need;                                                       /* [ebp-0x20] */
    int32_t used = 0;                                                   /* [ebp-0x1C] the credits used */
    int32_t a, t;
    char buf[52];                                                       /* [ebp-0x84] */

    setD32(SEL, 0xFFFFFFFFu);                                           /* 0x16D9F */
    setD32(PREV, 0xFFFFFFFFu);                                          /* 0x16DA5 */
    setD32(TREAS_USED, 0);                                              /* 0x16DAB */
    setD32(AI_TREAS, 0);                                                /* 0x16DB1 */
    for (i = 0; i <= 0x17; i++)                                         /* 0x16DC5 */
        f3B914(DLG, ID(i), 0);                                          /* 0x16DDD */
    setD16(BUYS(n), 0xFFFFu);                                           /* 0x16DF5 */
    setD16(BUYS(n) + 2u, 0xFFFFu);                                      /* 0x16E09 */
    setD16(BUYS(n) + 4u, 0xFFFFu);                                      /* 0x16E1D */
    f194E4(n);                                                          /* 0x16E28 */
    setD16(BUYCNT(n), 0);                                               /* 0x16E32 */
    if (f17EDC(n, -1, 1) == 0)                                          /* 0x16E47; 0x16E4E */
        goto L_171E2;

    f1A888(n);                                                          /* 0x16E57 */
    setD16(BUYS(n), (uint16_t)f177EC(n));                               /* 0x16E5F; 0x16E72 */
    if (S16(BUYS(n)) == -1)                                             /* 0x16E86 [10n + 0xF6E10] sar 16 */
        goto L_17037;
    setD8(rec + 0x24u + (uint32_t)S16(BUYS(n)), 1);                     /* 0x16EB2 */
    a = f176BC(n, 0);                                                   /* 0x16EBB */
    a = f195D8(n) + a;                                                  /* 0x16EC5; 0x16ECA */
    budget = S8(rec + 0x3Eu) + a - COST(S16(BUYS(n)));                  /* 0x16ECF..0x16EF7 */
    setD16(BUYCNT(n), (uint16_t)(S16(BUYCNT(n)) + 1));                  /* 0x16EFF */
    if (f17EDC(n, budget, 0) == 0)                                      /* 0x16F0F; 0x16F16 */
        goto L_17037;

    f1A888(n);                                                          /* 0x16F1F */
    setD16(BUYS(n) + 2u, (uint16_t)f177EC(n));                          /* 0x16F27; 0x16F3A */
    if (S16(BUYS(n) + 2u) == -1)                                        /* 0x16F4E */
        goto L_17037;
    setD8(rec + 0x24u + (uint32_t)S16(BUYS(n) + 2u), 1);                /* 0x16F7A */
    a = f176BC(n, 0);                                                   /* 0x16F83 */
    a += f176BC(n, 1);                                                  /* 0x16F92; 0x16F97 */
    a += f195D8(n);                                                     /* 0x16F9C; 0x16FA1 */
    a += S8(rec + 0x3Eu);                                               /* 0x16FAC */
    a -= COST(S16(BUYS(n)));                                            /* 0x16FCE */
    a -= COST(S16(BUYS(n) + 2u));                                       /* 0x16FF0 */
    budget = a;                                                         /* 0x16FF2 */
    setD16(BUYCNT(n), (uint16_t)(S16(BUYCNT(n)) + 1));                  /* 0x16FF8 */
    if (f17EDC(n, budget, 0) == 0)                                      /* 0x17008; 0x1700F */
        goto L_17037;

    f1A888(n);                                                          /* 0x17014 */
    setD16(BUYS(n) + 4u, (uint16_t)f177EC(n));                          /* 0x1701C; 0x1702F */
    /* BUG(1995): the third advance is chosen and its cost is paid below (0x170D0), but the count is incremented only
       after the first two (0x16EFC, 0x16FF5): the computer pays for three advances and gets two. Fixed: counted. */
    if (!g_original && S16(BUYS(n) + 4u) != -1)
        setD16(BUYCNT(n), (uint16_t)(S16(BUYCNT(n)) + 1));

L_17037:
    need = COST(S16(BUYS(n))) - f176BC(n, 0);                           /* 0x17043..0x17063 */
    used += f176BC(n, 0);                                               /* 0x1706B; 0x17070 */
    if (S16(BUYS(n) + 2u) != -1) {                                      /* 0x1707F..0x1708C */
        t = COST(S16(BUYS(n) + 2u));                                    /* 0x170A4 */
        need += t - f176BC(n, 1);                                       /* 0x170B6; 0x170BB, 0x170BD */
        used += f176BC(n, 1);                                           /* 0x170C8; 0x170CD */
    }
    if (S16(BUYS(n) + 4u) != -1) {                                      /* 0x170DC..0x170E9 */
        t = COST(S16(BUYS(n) + 4u));                                    /* 0x17101 */
        need += t - f176BC(n, 2);                                       /* 0x17113; 0x17118, 0x1711A */
        used += f176BC(n, 2);                                           /* 0x17125; 0x1712A */
    }
    if (S16(BUYCNT(n)) > 0) {                                           /* 0x1712D [2n + 0xF6E74] sar 16; jle */
        for (i = 0; S16(BUYCNT(n)) > i; i++)                            /* 0x1714F */
            setD8(rec + 0x24u + (uint32_t)S16(BUYS(n) + 2u * (uint32_t)i), 0);   /* 0x1717F */
        if (f1B59C(n, need) == 0) {                                     /* 0x1718B; 0x17192 */
            for (i = 0; S16(BUYCNT(n)) > i; i++)                        /* 0x1719F */
                setD8(rec + 0x24u + (uint32_t)S16(BUYS(n) + 2u * (uint32_t)i), 0);   /* 0x171CF */
            setD16(BUYCNT(n), 0);                                       /* 0x171DA */
        }
    }

L_171E2:
    if (S16(BUYCNT(n)) <= 0)                                            /* 0x171E2 */
        return;                                                         /* -> 0x17608 */
    *nothing = 0;                                                       /* 0x171F9 */
    for (i = 1; i <= 0x12; i++)                                         /* 0x171FC */
        for (j = 0; S8(REC0 + (uint32_t)i) > j; j++) {                  /* 0x17229 [i + 0xF14C9] sar 24 */
            add8(SPENT(n) + (uint32_t)i, 1);                            /* 0x1724E */
            add8(rec + (uint32_t)i, -1);                                /* 0x1725A */
            f771F8(i);                                                  /* 0x1725F */
        }
    add8(rec + 0x3Eu, -(int)D8(AI_TREAS));                              /* 0x17266..0x1726F */
    add8(rec + 0x99u, (int)D8(AI_TREAS));                               /* 0x17272..0x17285 */
    for (i = 0; S16(BUYCNT(n)) > i; i++) {                              /* 0x17296 */
        a = S16(BUYS(n) + 2u * (uint32_t)i);
        setD8(rec + 0x24u + (uint32_t)a, 1);                            /* 0x172CA */
        f3BEA0(DLG, (int)(int16_t)(D16(BUYS(n) + 2u * (uint32_t)i) + 0x8Eu), 0x0F);   /* 0x172DF..0x172F9 */
        f3C284(DLG, (int)(int16_t)(D16(BUYS(n) + 2u * (uint32_t)i) + 0x8Eu), ADV_UNAME(a));   /* 0x17319..0x17346 */
    }
    for (i = 1; i <= 0x12; i++)                                         /* 0x1735F */
        setD8(GOODS + (uint32_t)i, D8(REC0 + (uint32_t)i));             /* 0x17368..0x17371 */
    setD32(SEL, 0xFFFFFFFFu);                                           /* 0x17379 */
    f1818C(n, 0);                                                       /* 0x17388 */
    f19170();                                                           /* 0x1738D */
    fg_setcolor(5);                                                     /* 0x17394 */
    f3A044(0x119, 0x51, DSTR(0xB14F3u));                                /* 0x173AB "Goods toward purchase" */
    if (D8(MINING_FLAG) != 0) {                                         /* 0x173B0 */
        f3A0B0(0x1DE, 0x99, 0x5A, DSTR(0xB1509u));                      /* 0x173CD "Applied Mining to" */
        f3C9D0(0x1ED, 0xA9, 0x3C, 0x13, 5);                             /* 0x173E8 */
        fg_setcolor(5);                                                 /* 0x173EF */
        f3A0B0(0x1ED, 0xAD, 0x3C, METAL(S8(MINING_METAL + D32(MINING_CHOICE))));   /* 0x173F4..0x1741B */
    }
    if (used != 0) {                                                    /* 0x17420 */
        f3A3A0(0x222, 0xE3, DSTR(0xB151Bu));                            /* 0x1743A "Credits Used" */
        f3C9D0(0x239, 0xDE, 0x1A, 0x13, 5);                             /* 0x17450 */
        itoa10(used, buf, sizeof buf);                                  /* 0x17463 */
        fg_setcolor(5);                                                 /* 0x1746A */
        f3A0B0(0x239, 0xE2, 0x1A, buf);                                 /* 0x17487 */
    }
    f3A3A0(0x222, 0xFA, DSTR(0xB1528u));                                /* 0x174A0 "Goods Used" */
    f3C9D0(0x239, 0xF5, 0x1A, 0x13, 5);                                 /* 0x174B6 */
    itoa10(f193AC(), buf, sizeof buf);                                  /* 0x174C6; 0x174CB */
    fg_setcolor(5);                                                     /* 0x174D2 */
    f3A0B0(0x239, 0xF9, 0x1A, buf);                                     /* 0x174EF */
    f3A3A0(0x222, 0x112, DSTR(0xB1533u));                               /* 0x17508 "Treasury Used" */
    f3C9D0(0x239, 0x10C, 0x1A, 0x13, 5);                                /* 0x1751E */
    itoa10(S32(AI_TREAS), buf, sizeof buf);                             /* 0x17533 */
    fg_setcolor(5);                                                     /* 0x1753A */
    f3A0B0(0x239, 0x110, 0x1A, buf);                                    /* 0x17557 */
    f3AF8C(0xF49ECu + 8u * (uint32_t)n, 0x23F, 0x16E);                  /* 0x17571 */
    itoa10(S8(rec + 0x3Eu), buf, sizeof buf);                           /* 0x17583 [199n + 0xF1507] sar 24; 0x17591 */
    fg_setcolor(5);                                                     /* 0x17598 */
    f3A044(0x259, 0x171, buf);                                          /* 0x175B0 */
    f19700(n, S16(BUYCNT(n)), BUYS(n));                                 /* 0x175DA */
}

void f162D0(void)
{
    uint8_t piles[9 * 25];                                              /* [ebp-0x168] the discard piles at the start */
    int32_t i;                                                          /* [ebp-0x4C] */
    int32_t n;                                                          /* [ebp-0x28] */
    int32_t c;
    uint32_t rec;                                                       /* [ebp-0x34] */
    int32_t nothing = 1;                                                /* [ebp-0x18] 0x162E8: nobody could buy */
    uint8_t *backup;
    const char *still = DSTR(0xBDD2Cu);                                 /* "Still needed" */

    setD16(ORDER_COUNT, 0);                                             /* 0x162F6 */
    for (i = 1; i <= 9; i++) {                                          /* 0x16308 */
        if (D8(NATION(i) + 0x9Au) == 0)                                 /* 0x1631B */
            continue;
        c = S16(ORDER_COUNT);                                           /* 0x16325 */
        setD16(ORDER_COUNT, (uint16_t)(c + 1));                         /* 0x16329 */
        setD16(ORDER + 2u * (uint32_t)c, (uint16_t)i);                  /* 0x16332 */
    }
    f783B8(DLG, S32(0xF5968u));                                         /* 0x16347 */
    setD32(TEMP_NATION, load_heap_alloc(NAT_SIZE));                     /* 0x16351 malloc(199); 0x16356 */
    /* 0x16365: the debug stub 0x78B0C("Acquire Civ") is a no-op */
    setD32(LABEL_W, (uint32_t)fgf_width(still, (int)strlen(still)));    /* 0x1637A; 0x16387 */
    f3BFBC(DLG, 0x41, 0);                                               /* 0x16393 RESTART disabled */
    setD8(BG, 0x12);                                                    /* 0x1639F */

    for (n = 1; n <= 9; n++) {                                          /* 0x17642 */
        backup = temp_nation();
        if (backup != NULL)
            memcpy(backup, DP(NATION(n)), NAT_SIZE);                    /* 0x17648..0x17666 */
        for (i = 0; i < 9; i++)                                         /* 0x163C1 */
            memcpy(piles + 25 * i, DP(PILES + 25u * (uint32_t)i), 25);  /* 0x163FB rep movsd (6) + movsb */
        i = 1;                                                          /* 0x16400 */
L_1640F:
        for (; i <= 0x12; i++)                                          /* 0x1640F */
            setD8(SPENT(n) + (uint32_t)i, 0);                           /* 0x1642B */
        rec = NATION(n);                                                /* 0x16441 */
        setD32(CUR_NATION, (uint32_t)n);                                /* 0x16449 */
        setD32(CARRY, 0);                                               /* 0x1644E */
        setD32(MINING_BOUGHT, 0);                                       /* 0x16454 */
        setD32(MINING_USED, 0);                                         /* 0x1645A */
        /* BUG(1995): the treasury used [0xF67E0] is not cleared when a human's turn starts: what one human put in
           and left (FINISHED without ACQUIRE) is spent from the next human's treasury. Fixed: cleared here. */
        if (!g_original)
            setD32(TREAS_USED, 0);
        if (D8(rec + 0x9Au) == 0)                                       /* 0x16463 */
            goto L_17626;
        if (S16(GRAIN(n)) != 0)                                         /* 0x16473 */
            add8(rec + 0x08u, -(int)D8(GRAIN(n)));                      /* 0x16483 mov dl, byte [2n + 0xF409A]; 0x1648D */

        if ((S8(rec + 0x9Au) == 3 && (D8(FLAGS) & PBEM_BIT) == 0)       /* 0x16490..0x164A8 [rec + 0x97] sar 24 */
            || ((D8(FLAGS) & PBEM_BIT) != 0 && S8(LOCAL_PBEM) == n)) {  /* 0x164AA..0x164C2 */
            if (human_turn(n, rec, piles, &nothing) != 0) {             /* 0x164C8..0x16D7E */
                i = 1;                                                  /* 0x1675D */
                goto L_1640F;                                           /* 0x16764 */
            }
        } else if (S8(rec + 0x9Au) == 2) {                              /* 0x16D83..0x16D92 */
            computer_turn(n, rec, &nothing);                            /* 0x16D98..0x175DF */
        } else if ((D8(FLAGS) & PBEM_BIT) != 0 && S8(LOCAL_PBEM) != n) {   /* 0x175E1..0x175F7 */
            f17EDC(n, -1, 0);                                           /* 0x17603 */
        }

        if (S16(GRAIN(n)) != 0)                                         /* 0x1760B (0x17608) */
            add8(rec + 0x08u, (int)D8(GRAIN(n)));                       /* 0x17619..0x17623 */
L_17626:
        setD16(NATION(n) + 0x5Eu, (uint16_t)f17760(n));                 /* 0x17629; 0x17635 */
    }

    /* BUG(1995): with n == 10 after the loop, int16 [0xF409A + 2n] = 0 (0x17674) clears 0xF40AE, the low-XMS flag
       that keeps sound effects and animations disabled. Fixed: not written. */
    if (g_original)
        setD16(GRAIN(n), 0);                                            /* 0x1766F..0x17674 */
    load_heap_free(D32(TEMP_NATION));                                   /* 0x17683 free */
    setD32(TEMP_NATION, 0);                                             /* 0x17688 */
    if (nothing != 0)                                                   /* 0x1768E */
        f79270(100, S16(ORDER), -200, DSTR(0xB1541u), 0);               /* 0x16694..0x176AD ("", [0xF06AE] sar 16) */
}

/* ==== 0x1A3A0: Tool Acquisition Wrapup ============================================================================ */

void f1A3A0(void)
{
    int32_t any = 0;                                                    /* [ebp-0x28] */
    int32_t n;                                                          /* [ebp-0x2C] */
    int32_t row = 0;                                                    /* [ebp-0x34] */
    int32_t j;                                                          /* [ebp-0x18] */
    int32_t step = 0, y0 = 0;                                           /* [ebp-0x20], [ebp-0x24] */
    int32_t code;                                                       /* [ebp-0x30] */
    char buf[100];                                                      /* [ebp-0x98] */

    for (n = 1; n <= 9; n++) {                                          /* 0x1A3CC */
        if (D8(NATION(n) + 0x9Au) == 0)                                 /* 0x1A3DF */
            continue;
        if (S16(BUYCNT(n)) <= 0)                                        /* 0x1A3E6 [2n + 0xF6E74] sar 16; jle */
            continue;
        any++;                                                          /* 0x1A3F7 */
        break;
    }
    if (any == 0)                                                       /* 0x1A3FA */
        return;

    memset(DP(REC0), 0, NAT_SIZE);                                      /* 0x1A410 memset(0xF14CC, 0, 0xC7) */
    f3CD6C(0x19);                                                       /* 0x1A41A */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BGBUF), S32(BACKBUF));  /* 0x1A442 */
    fg_vbopen(S32(BACKBUF));                                            /* 0x1A451 */
    f39F90(2);                                                          /* 0x1A45E */
    snprintf(buf, sizeof buf, "%s", DSTR(0xB169Bu));                    /* 0x1A479 sprintf(buf, "Tool Acquisition Wrapup") */
    f3A18C(0, 6, 0x280, 0xB0, buf);                                     /* 0x1A491 */
    fg_setcolor(0x0D);                                                  /* 0x1A49B */
    f3A0B0(0x3D, 0x1A, 0x5A, DSTR(0xB16B3u));                           /* 0x1A4B7 "Acquired" */
    f3A044(0xBE, 0x1A, DSTR(0xB16BCu));                                 /* 0x1A4CB "Using" */

    for (n = 1; n <= 9; n++) {                                          /* 0x1A5FF */
        if (D8(NATION(n) + 0x9Au) == 0)                                 /* 0x1A616 test dh, dh */
            continue;
        f3AF8C(0xF4ACCu + 8u * (uint32_t)n,                             /* 0x1A65B */
               (0x33 - (int32_t)D16(0xF4AD0u + 8u * (uint32_t)n)) / 2 + 5,   /* 0x1A635..0x1A64D */
               51 * row + 0x2D);                                        /* 0x1A62D */
        fg_setcolor(0x0F);                                              /* 0x1A662 */
        f3A0B0(5, 51 * row + 0x2D + (int32_t)D16(0xF4AD2u + 8u * (uint32_t)n) + 2, 0x33, NAME(n));   /* 0x1A66A..0x1A6A6 */
        if (S16(BUYCNT(n)) > 0) {                                       /* 0x1A6AB..0x1A6BA */
            step = 0x33 / (S16(BUYCNT(n)) + 1);                         /* 0x1A6C3..0x1A6D8 idiv */
            y0 = 51 * row + 0x26 + step - 5;                            /* 0x1A6DD..0x1A6F6 */
            for (j = 0; S16(BUYCNT(n)) > j; j++) {                      /* 0x1A4E2 */
                fg_setcolor(0x0D);                                      /* 0x1A4F6 */
                f3A0B0(0x3D, step * j + y0, 0x5A,                       /* 0x1A536 */
                       DSTR(ADV_UNAME(S16(BUYS(n) + 2u * (uint32_t)j))));   /* 0x1A50F [2j + 10n + 0xF6E10] sar 16 */
            }
            for (j = 1; j <= 0x12; j++)                                 /* 0x1A54C */
                setD8(REC0 + (uint32_t)j, D8(SPENT(n) + (uint32_t)j));  /* 0x1A566..0x1A56F */
            f71FB0(0xA0, 51 * row + 0x26, 0x1CC, 0);                    /* 0x1A577..0x1A599 */
        }
        if (row != 0) {                                                 /* 0x1A59E */
            fg_move(0x3D, 51 * row + 0x23);                             /* 0x1A5BD */
            fg_setcolor(5);                                             /* 0x1A5C7 */
            fg_draw(0x259, 51 * row + 0x23);                            /* 0x1A5EB */
        }
        row++;                                                          /* 0x1A5F6 */
    }
    for (j = 0; S16(DLG_WRAPUP) > j; j++)                               /* 0x1A713 [0xBD93E] sar 16 */
        f3B4E0(DLG_WRAPUP + 2u + 20u * (uint32_t)j);                    /* 0x1A720..0x1A73C */
    fg_vbclose();                                                       /* 0x1A743 */
    setD32(0xF8E78u, DLG_WRAPUP);                                       /* 0x1A748 */
    f3CC14();                                                           /* 0x1A752 */
    do {
        if (f7BA64() == 0) {                                            /* 0x1A757; 0x1A75E */
            code = f78BA4(DLG_WRAPUP, 0);                               /* 0x1A767; 0x1A76C */
        } else {
            f78A20(S16(0xF40BAu) == 0 ? 0xFA : 0x3E8);                  /* 0x1A771..0x1A78C */
            code = -1;                                                  /* 0x1A787 mov ecx, -1; 0x1A791 */
        }
        /* 0x1A794..0x1A7AE: -100 polls again (je 0x1A757), the row buttons' -0x50..-0x57 are ignored (jge 0x1A757),
           and so is anything else but -1 (jne 0x1A757) */
    } while (code != -1);
}
