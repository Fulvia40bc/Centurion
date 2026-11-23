/* Module CAL5 of docs/M6_PLAN.md: the Barbarian Hordes. Literal translations; the address of each instruction a line
   comes from is in its comment. D = read in the code, G = inference. Prototypes and globals: cal5.h.

     0x217A0  f217A0  Barbarian Hordes (calamity card 0x1A; the director, Crete)                372 bytes
     0x1C3D0  f1C3D0  the invasion: messages, landing, moves, the BRPC                       1,580
     0x1C9FC  f1C9FC  the landing list                                                       2,104
     0x1D234  f1D234  one move (settle / invade, the losses box)                             1,572
     0x1D858  f1D858  the recursive route search                                             1,620
     0x1DEAC  f1DEAC  the route entries' sort                                                  336
     0x1DFFC  f1DFFC  the losses of everybody                                                  164
     0x7A4F0  f7A4F0  snapshot of areas / nations / territory states into three heap blocks   168
     0x7AB24  f7AB24  restore and free it                                                      172

   Calls into code that is not ported yet: none directly. CONFLICT's 0x5B224 (f5B224), 0x5B588 (f5B588) and 0x5CDE8
   (f5CDE8, always with eax = 0: no animation) are conflict.c's; cal1.c's f207B4 (kind 0x1A) and f1E69C are called as
   they are. The animations are reached only through kit.c's f79270, which stops at them itself (not_ported): the
   calamity animation 0x7A0A0 for every message here (anim 2) when animations are on, and the BRPC picture 0x79C70
   (anim -299) always (anim.c since 19 Nov). So no -DCAL5_TEST recorder
   is needed; port/test/cal5_test.c links stubs of the callees instead. No computer-player (M7) code is called: the
   barbarian routing runs for every victim (M6_PLAN), and a computer director's choice is inline (game_rand).

   Play by e-mail (dropped by Erik's ruling): no PBEM routine is called here. 0x1D234's test of bit 0x80 of byte
   [0xE1624] (0x1D6A4) is kept literally: it only sends a PBEM human director down the computer's (game_rand) path.

   game_rand (f78AE4): 0x1D234 only, at most one draw per move: 0x1D45D (the settle path with several destinations)
   or 0x1D6D0 (the invasion with several destinations and a director that is a computer, a PBEM human or out of
   play). Nothing else here draws (D: no other call of 0x78AE4 in these nine functions); conflict.c's f5B224 / f5B588
   draw nothing either (their header).

   Watcom runtime -> standard C:
     0x7DB3D sprintf -> snprintf bounded to the stack buffers (0x217A0 [ebp-0x84] 100 bytes, 0x1C3D0 [ebp-0xA8] 120,
             0x1D234 [ebp-0x88] 100) and to 0xF5500 (500 bytes, as loop.c)
     0x7D5D0 memset -> memset on the data object
     0x822B5 malloc -> load_heap_alloc (port handles: in [0xEED74] / [0xEED78] / [0xEED7C], and in 0x1D858's locals);
             0x877E6 free -> load_heap_free; repne movsd / movsb block copies -> memcpy (with load_heap_ptr)
     0x8A080 clock: 0x1C3D0 reads start = clock() before a message and busy-waits until clock() >= start +
             (1500 / [0xBB3F4]) / 10 hundredths (0x1C46B, 0x1C59A) or (4000 / [0xBB3F4]) / 10 (0x1C7EB). Port (G, as
             ship.c): ui_kit2.c's clock advances only in waits, so each wait is f78A20(1500) / f78A20(4000) at the
             original's wait, which equals the original when the work in between spends no clock time.
     Miles AIL: AIL_set_sample_volume 0x8427E, AIL_start_sample 0x84054, AIL_set_sample_loop_count 0x8436A,
             AIL_end_sample 0x8419B are port no-ops (static stubs below, as loop.c's); their arguments are still read
             from the data object (the sample [0xF6278], the volume from word [0xF097E]).
   __CHK dropped; no 0x78B0C / 0x78B24 call in these functions.

   Data (D, read in the code at the addresses given):
     nation n: 0xF14CC + 199n: +0x51 int8 cities, +0x98 int8 tokens on the board, +0x99 int8 stock, +0x9A 0 out / 2
       computer / 3 human; nation 11 = the Barbarians (their tokens are area +0x19 = +0x0E + 11)
     area record r = t + 1: 0xE31B8 + 69r: +0x01 int8 nations there, +0x0E + n tokens, +0x19 barbarians, +0x3B + n the
       "lost territory" mark (2 = to the barbarians)
     territory record t: 0xE1638 + 35t: +0 high nibble the nation whose landing zone t is (terrflag.dat: Africa 4,
       Iberia 6, Illyria 2, Thrace 1, Crete 2, Asia 2, Assyria 2, Babylon 3, Egypt 6 zones), +1 bits 3..5 the
       population limit
     territory state t: 0xE2C18 + 3t (+0 owner, +2 bit 0x01 a city)
     the land / coast lists [0xF8E7C + 4t] / [0xF90FC + 4t]: handles to int16 count, int16 territories (game.h)
     the calamity record k: 0xF77E4 + 3k {id, int8 giver, int8 victim} (cal1.h)
     names 0xC768C[n], adjectives 0xC76C4[n], territory names 0xC77D8[t]; the spinner 0xBB404[4] = "|", "/", "-",
       "\"; "brpc.wav" at 0xCAB6C; dialog 0xC5214 (FINISHED, appended to the panel 0xF596C for the losses box)
     message area int16 0xBB3FA x, 0xBB3FC y, 0xBB3FE w, 0xBB400 h; byte [0xEE168] the background number; [0xF52C8] /
       [0xF52CC] the back / background buffers; byte [0xF097C] bit 0x01 sound effects; byte [0xF625C] digital sound

   1995 bugs, fixed unless g_original (each marked BUG(1995) in the code; grades code / effect):
     1. 0x1DFFC (D / D): the second loop has no jump back after its add (0x1E061 falls into the return 0x1E064), so only
        the FIRST non-zero city loss of 0xF548C[1..10] is added (M6_PLAN risk 11 confirmed). Effect: none on the game.
        Both callers only test the sum against 0 (0x1D60C, 0x1D7C9: the losses box), and 0x1D858 stores its low byte in
        route entry +1, which only 0x1D234 reads, again against 0 (0x1D263); all terms are non-negative, so zero /
        non-zero is the same either way. Only the bytes +1 of the route buffer 0xF6E94 (data object) differ when a
        search simulates cities lost by two or more nations. Fixed: both tables summed.
     2. 0x217A0 (D / G): the director [ebp-0x1C] is never written when no nation 1..9 in play other than the victim
        scores 5 * (9 - cities) + stock above 0 (all others with 9 cities and no stock), and 0x1C3D0 / 0x1D234 then use
        stack garbage as a nation index (+0x9A test, f207B4). A port cannot reproduce the garbage: --original takes
        nation 0 (whose +0x9A is 0, so the computer's game_rand path). Fixed: the best-scoring nation in play other than
        the victim (the first of equals), even with a score of 0 or less; the victim itself if nobody else is in play.
     3. The runs over equal entries have no bound (D / G):
        a. 0x1C3D0 (0x1C5D6, 0x1C795) and 0x1D234 (0x1D319) take route entries while they equal entry 0 (+2 and the
           low / high nibble). The route holds 10 entries; when all 10 are equal the run goes on into the visited
           bytes 0xF6EBC (cleared, so entries {0,0,0,0}) and lists territory 0 (SUDETEN) up to 39 more times, until
           visited[157..159] (entry 49) stops it. G: reachable only from a territory with 10 unvisited neighbours (only
           SYRIA has 10 with the shipped lists) whose 10 entries are all equal. --original reads the data object as the
           original does (port guard: at most 157 entries, the list's size). Fixed: the run stops after entry 9.
        b. 0x1C9FC's two runs over its local table (6 entries at [ebp-0x50]) stop only at an unequal entry: past the
           count they read uninitialised entries (stack garbage; a port cannot reproduce it), and when all 6 entries
           are equal (Iberia or Egypt, the nations with six landing zones) they read the landing list int16 [ebp-0x38]
           as entries 6..8 and n as entry 9, each taken (+3 = 0: territory 0 again) while it still matches entry 0
           (the low nibble of land[2j], and in the trials' run also +2 = the low byte of land[2j + 1]). Reachable with
           the shipped data: Iberia (land[0] = 7) with 7 tokens in each of its six zones, or Egypt (land[0] = 108,
           nibble 12) with 12, each gives one extra territory-0 entry (a city in each zone, counting 6, gives none).
           --original: entries 6..9 as the frame holds them (land[], n), entries past the count below 6 and from 10
           on as the port's stand-in (the run stops). Fixed: the runs stop at the count.
   Kept as the original (D code, G effect unless marked):
     - The list entries past the count in the route buffer (0xF6E94 + 4i, i = count..9 at depth 0) are copies of
       0x1D858's uninitialised local entries; the port's stand-in is {0x0F, 0xFF, 0xFF, 0xFF} (it never equals a
       real entry in the fields the runs compare: +2 = 0xFF entries all have a low nibble 0, D). The high nibble of
       every entry that is not a depth-0 hit is likewise stack garbage in the original (0 in the port): only the
       depth-0 hits' high nibble is ever used (0x1D234's settle path, D). By design in a data-object compare.
     - 0x1D234's settle path ("The Barbarians finally settle in %s.") moves only the high nibble of route entry 0 (the
       barbarians left after a simulated first move with nobody hurt) and resolves no conflict; the loop of 0x1C3D0
       goes on if they still exceed the limit there.
     - The BRPC search is the normal search with 20 barbarians instead of the excess (not a depth of 20, as the
       inventory reads it); [0xF6F60] stays 1 for the rest of the calamity, so every later search shows the spinner.
     - [0xF6F5C] (the spinner's call counter) is never reset.
     - The search's three blocks are 157 areas / 157 territory states / 11 nations (0x2A51 / 0x1D7 / 0x88D), the
       snapshot's 160 / 160 / 11: areas 158..160 and states 157..159 are outside the search (no barbarian reaches
       them: 157..159 are preset visited and no list holds a sea).
     - 0x1C9FC's empty-zone trial (no candidate) searches with 15 - limit barbarians without placing them; the
       all-zones trial places 15 and resolves the battle first.
     - 0x1C9FC writes its local tables without a bound: landing zones per nation at most 6 (D: terrflag.dat), route
       candidates at most 10 per territory (D: landadj.dat + coastadj.dat, SYRIA), so the 6 and 10 entries fit; port
       guards keep the arrays in bounds for other data (not reached with the shipped files).
   Text: Erik's ruling of 19 Nov (obvious errors only). "Crete can not be the Victim of Barbarian Hordes." (0xB1E6D,
   TEXT_SWEEP: "cannot", "victim") is left as it is: "can not" and the capital are style and wording under that
   ruling (for Erik). No other slip in these strings.
   Easter eggs / oddities: the Barbarian Ravaging and Plundering Committee (OVERVIEW §6.4; M6_PLAN egg 2): "The next
   destination has been referred to the BRPC (Barbarian Ravaging and Plundering Committee). Please wait. This could
   take several minutes." with the BRPC picture (anim -299, even with animations off), brpc.wav looping, an ASCII
   spinner | / - \ appended to that text every 250 search calls, at least 4 s; "The BRPC is deadlocked. They decide to
   settle in %s."; "%s has been wiped out by the Barbarians. Satisfied with a job well done they settle in %s.";
   Crete cannot be the victim (M6_PLAN egg 7); the duplicate strings "%s is invaded." and "BARBARIAN CONFLICT - UNITS
   REMOVED" (0xB1960 / 0xB1992, 0xB196F / 0xB19A1: one copy per path, the compiler did not merge them; new). */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cal1.h"
#include "cal5.h"
#include "conflict.h"
#include "fg.h"
#include "game.h"
#include "kit.h"
#include "map.h"
#include "menu2.h"
#include "move.h"
#include "panel.h"
#include "snd.h"
#include "state.h"
#include "ui.h"

#define NATION(n)    (0xF14CCu + 199u * (uint32_t)(n))
#define NAME(n)      DSTR(D32(0xC768Cu + 4u * (uint32_t)(n)))
#define ADJ(n)       DSTR(D32(0xC76C4u + 4u * (uint32_t)(n)))
#define TERR_NAME(t) DSTR(D32(0xC77D8u + 4u * (uint32_t)(t)))
#define TERR(t)      (0xE1638u + 35u * (uint32_t)(t))
#define TSTATE(t)    (0xE2C18u + 3u * (uint32_t)(t))
#define AREA(r)      (0xE31B8u + 69u * (uint32_t)(r))
#define CAL_GIVER(k) (0xF77E5u + 3u * (uint32_t)(k))   /* int8 (dword [0xF77E2 + 3k] sar 24) */
#define CAL_VICTIM(k) (0xF77E6u + 3u * (uint32_t)(k))  /* int8 (dword [0xF77E3 + 3k] sar 24) */
#define FLAGS        0xE1624u
#define PBEM_BIT     0x80u
#define ROUTE        0xF6E94u   /* 4-byte entries, 10 written at depth 0 */
#define VISITED      0xF6EBCu   /* uint8[160] */
#define DEPTH        0xF6E8Cu   /* int32 */
#define SWAP         0xF6E90u   /* int32 */
#define BRPC_CALLS   0xF6F5Cu   /* int32 */
#define BRPC_MODE    0xF6F60u   /* int32 */
#define CHOICE       0xF7CA0u   /* int32[157] */
#define CHOICE_N     0xF7C9Cu   /* int32 */
#define LOST_TOK     0xF547Cu   /* uint8[14] */
#define LOST_CITY    0xF548Cu   /* uint8[14] */
#define SNAP_AREA    0xEED74u
#define SNAP_NATION  0xEED78u
#define SNAP_TSTATE  0xEED7Cu
#define LANDADJ      0xF8E7Cu
#define COASTADJ     0xF90FCu
#define PANEL_DLG    0xF596Cu
#define LOSSES_DLG   0xC5214u
#define BACKBUF      0xF52C8u
#define BACKGROUND   0xF52CCu
#define BGNUM        0xEE168u
#define MSG_Y        0xBB3FCu
#define MSG_H        0xBB400u
#define BARB         11

/* ---- Miles AIL (snd.c since 22 Nov; without a backend the old no-ops) -------------------------------------------- */

static void ail_set_sample_volume(uint32_t s, int v) { snd_set_sample_volume(s, v); }
static void ail_start_sample(uint32_t s) { snd_start_sample(s); }
static void ail_set_sample_loop_count(uint32_t s, int n) { snd_set_sample_loop_count(s, n); }
static void ail_end_sample(uint32_t s) { snd_end_sample(s); }

/* ---- small shared pieces ------------------------------------------------------------------------------------------ */

/* the barbarians of territory t minus its population limit: dword [69(t+1) + 0xE31CE] sar 24 (area +0x19) minus
   ((byte [35t + 0xE1639] << 2) >> 5) & 0xFF (0x1C4AF..0x1C4E6 and its five copies) */
static int32_t barb(int32_t t) { return S8(AREA(t + 1) + 0x19u); }
static int32_t limit(int32_t t) { return (int32_t)((uint8_t)(D8(TERR(t) + 1u) << 2) >> 5); }

/* a city of n in territory t: byte [3t + 0xE2C1A] & 1 and byte [3t + 0xE2C18] == n */
static int own_city(int32_t t, int n)
{
    return (D8(TSTATE(t) + 2u) & 1) != 0 && (int32_t)D8(TSTATE(t)) == n;
}

/* the int16 at handle h + off (the land / coast lists: port handles, game.h). Port guard: 0 for a bad handle */
static int16_t hword(uint32_t h, uint32_t off)
{
    const uint8_t *p = load_heap_ptr(h + off, 2);
    return p != NULL ? (int16_t)(p[0] | p[1] << 8) : 0;
}

static uint32_t rd32(const uint8_t *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static void wr32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

/* the choice list: list[++count] = v (0x1C60F..0x1C61C and its copies: [0xF7C9C] + 1, then the store at 4 * that +
   0xF7C9C, i.e. 0xF7CA0[count]). Port guard: the list holds 157 entries */
static void append(uint32_t v)
{
    int32_t c = S32(CHOICE_N);

    if (c >= 0 && c < 0x9D) {
        setD32(CHOICE + 4u * (uint32_t)c, v);
        setD32(CHOICE_N, (uint32_t)(c + 1));
    }
}

/* the visited bytes cleared, 157..159 set (0x1C531..0x1C550, 0x1C735..0x1C752, 0x1CE57..0x1CE7E, 0x1D13B..0x1D15A) */
static void clear_visited(void)
{
    memset(DP(VISITED), 0, 0xA0);
    setD8(0xF6F59u, 1);
    setD8(0xF6F5Au, 1);
    setD8(0xF6F5Bu, 1);
}

/* the equal-best route entries into the list (0x1C5C6..0x1C621 and 0x1C786..0x1C7E4): [0xF7C9C] = 0, then while
   entry i has entry 0's +2 and low nibble, list += its +3 */
static void route_run_lo(void)
{
    int32_t i;   /* [ebp-0x2C] */

    setD32(CHOICE_N, 0);                                            /* 0x1C5CD / 0x1C78C */
    for (i = 0;; i++) {
        /* BUG(1995) 3a: no bound; past entry 9 the run reads the visited bytes (file comment) */
        if (!g_original && i >= 10)
            break;
        if (S32(CHOICE_N) >= 0x9D)                                  /* port guard: the list's size */
            break;
        if (D8(ROUTE + 2u) != D8(ROUTE + 4u * (uint32_t)i + 2u))     /* 0x1C5DF / 0x1C79E */
            break;
        if ((D8(ROUTE + 4u * (uint32_t)i) & 0xF) != (D8(ROUTE) & 0xF))   /* 0x1C5FC / 0x1C7BB */
            break;
        append(D8(ROUTE + 4u * (uint32_t)i + 3u));                  /* 0x1C608 / 0x1C7C5 */
    }
}

/* ==== 0x217A0 Barbarian Hordes ===================================================================================== */

void f217A0(int k)
{
    char buf[100];       /* [ebp-0x84] */
    int32_t v;           /* [ebp-0x18] */
    int32_t d = 0;       /* [ebp-0x1C] (uninitialised in the original: BUG(1995) 2) */
    int16_t best = 0;    /* [ebp-0x14] */
    int16_t i;           /* [ebp-0x10] */
    int found = 0;       /* port: d written */

    v = S8(CAL_VICTIM(k));                                          /* 0x217C7 dword [0xF77E3 + 3k] sar 24 */
    if (S8(CAL_GIVER(k)) != -1) {                                   /* 0x217DF..0x217EB */
        d = S8(CAL_GIVER(k));                                       /* 0x21802 */
        found = 1;
    } else {
        for (i = 1; i <= 9; i++) {                                  /* 0x2180A, 0x21819 movsx; cmp 9; jg */
            int32_t score;
            if (D8(NATION(i) + 0x9Au) == 0)                         /* 0x21830 */
                continue;
            /* 0x2183A..0x21870: 5 * (9 - (dword [0xF151A + 199i] sar 24)) + (dword [0xF1562 + 199i] sar 24) */
            score = (9 - S8(NATION(i) + 0x51u)) * 5 + S8(NATION(i) + 0x99u);
            if (score <= best)                                      /* 0x21876 cmp edx, movsx best; jle */
                continue;
            if (i == v)                                             /* 0x2187E */
                continue;
            /* 0x21883..0x218B3: the same sum through movsx ax (16 bits kept) */
            best = (int16_t)((9 - S8(NATION(i) + 0x51u)) * 5 + S8(NATION(i) + 0x99u));
            d = i;                                                  /* 0x218BA */
            found = 1;
        }
        if (!found) {
            if (g_original) {
                /* BUG(1995) 2: [ebp-0x1C] is stack garbage here; the port's stand-in is nation 0 */
                d = 0;
            } else {
                /* fixed: the best-scoring nation in play other than v, the first of equals; v if nobody else */
                int32_t b = INT32_MIN;
                d = v;
                for (i = 1; i <= 9; i++) {
                    int32_t score;
                    if (D8(NATION(i) + 0x9Au) == 0 || i == v)
                        continue;
                    score = (9 - S8(NATION(i) + 0x51u)) * 5 + S8(NATION(i) + 0x99u);
                    if (score > b) {
                        b = score;
                        d = i;
                    }
                }
            }
        }
    }
    if (v != 5) {                                                   /* 0x218C2 cmp 5; je */
        f1C3D0(v, d);                                               /* 0x218CE */
    } else {
        snprintf(buf, sizeof buf, "%s", DSTR(0xB1E6Du));            /* 0x218E1 "Crete can not be the Victim of
                                                                       Barbarian Hordes." (sprintf with no argument) */
        f79270(-500, v, -200, buf, 1);                              /* 0x218FE (push 1) */
    }
    f1E69C(k);                                                      /* 0x21906 */
}

/* ==== 0x1C3D0 the invasion ========================================================================================= */

void f1C3D0(int n, int d)
{
    char buf[120];       /* [ebp-0xA8] */
    int32_t t = -1;      /* [ebp-0x28] */
    int32_t excess;      /* [ebp-0x24] */
    int32_t vol;         /* [ebp-0x2C] (also the list index in the original) */
    int32_t save_h;      /* [ebp-0x18] */
    int32_t save_y;      /* [ebp-0x14] */

    setD32(BRPC_MODE, 0);                                           /* 0x1C3F8 */
    setD8(BGNUM, 3);                                                /* 0x1C3FE */
    snprintf(buf, sizeof buf, DSTR(0xB182Cu), NAME(n));             /* 0x1C41B "%s falls victim to the Barbarian
                                                                       Hordes." */
    f79270(-500, n, -200, buf, 2);                                  /* 0x1C438 */
    /* 0x1C43D start = clock(): see the wait below */
    f79270(-550, n, -200, DSTR(0xBDD5Cu), 2);                       /* 0x1C459 "The Barbarians consider their
                                                                       destination." */
    f78884();                                                       /* 0x1C45E */
    f1C9FC(n);                                                      /* 0x1C466 */
    f78A20(1500);       /* 0x1C46B..0x1C495: until clock() >= start + (1500 / [0xBB3F4]) / 10 (file comment) */
    f788AC();                                                       /* 0x1C497 */
    f1D234(n, d, 0xF, &t);                                          /* 0x1C4AA */

    for (;;) {
        if (barb(t) - limit(t) <= 0)                                /* 0x1C4AF..0x1C4EA jle 0x1C9F3 */
            return;
        f7A4F0();                                                   /* 0x1C4F0 */
        excess = barb(t) - limit(t);                                /* 0x1C4F5..0x1C52E */
        clear_visited();                                            /* 0x1C53D..0x1C550 */
        /* 0x1C556 start = clock() */
        f79270(-550, n, t, DSTR(0xBDD5Cu), 2);                      /* 0x1C570 */
        f78884();                                                   /* 0x1C577 */
        setD32(DEPTH, 0);                                           /* 0x1C57C */
        f1D858(n, t, excess, DP(ROUTE), DP(VISITED));               /* 0x1C595 */
        f78A20(1500);                                               /* 0x1C59A..0x1C5C4 (file comment) */
        f788AC();                                                   /* 0x1C5C8 */
        route_run_lo();                                             /* 0x1C5CD..0x1C621 */
        f7AB24();                                                   /* 0x1C623 */
        if (D8(ROUTE + 2u) == 0xFF) {                               /* 0x1C628..0x1C634 */
            /* 0x1C63A start = clock() */
            setD32(BRPC_MODE, 1);                                   /* 0x1C642 */
            save_h = S16(MSG_H);                                    /* 0x1C64C dword [0xBB3FE] sar 16 */
            save_y = S16(MSG_Y);                                    /* 0x1C657 dword [0xBB3FA] sar 16 */
            setD16(MSG_Y, 0x14C);                                   /* 0x1C66C */
            setD16(MSG_H, 0x7B);                                    /* 0x1C673 */
            f79270(-550, n, t, DSTR(0xBDD88u), -299);               /* 0x1C68F the BRPC text, picture -299 */
            f78884();                                               /* 0x1C694 */
            if ((D8(0xF097Cu) & 1) != 0) {                          /* 0x1C69F sound effects on */
                f7B504(0, DSTR(0xCAB6Cu));                          /* 0x1C6AF "brpc.wav" into slot 0 */
                if (D8(0xF625Cu) != 0) {                            /* 0x1C6B4 digital sound */
                    /* 0x1C6BE..0x1C6EC: v = bits 7..13 of word [0xF097E] ((w << 2) & 0xFFFF) >> 9; (v / 2) * 127 / 9 */
                    int32_t v = (int32_t)((uint16_t)(D16(0xF097Eu) << 2) >> 9);
                    vol = v / 2 * 127 / 9;
                    if (vol == 0)                                   /* 0x1C6F2 */
                        vol = 1;
                    ail_set_sample_volume(D32(0xF6278u), vol);      /* 0x1C708 */
                    ail_start_sample(D32(0xF6278u));                /* 0x1C717 */
                    ail_set_sample_loop_count(D32(0xF6278u), 0);    /* 0x1C728 for ever */
                }
            }
            f7A4F0();                                               /* 0x1C730 */
            clear_visited();                                        /* 0x1C741..0x1C752 */
            setD32(DEPTH, 0);                                       /* 0x1C759 */
            f1D858(n, t, 0x14, DP(ROUTE), DP(VISITED));             /* 0x1C773: 20 barbarians */
            setD16(MSG_H, (uint16_t)save_h);                        /* 0x1C77B */
            setD16(MSG_Y, (uint16_t)save_y);                        /* 0x1C786 */
            route_run_lo();                                         /* 0x1C78C..0x1C7E4 */
            f7AB24();                                               /* 0x1C7E6 */
            f78A20(4000);   /* 0x1C7EB..0x1C815: until clock() >= start + (4000 / [0xBB3F4]) / 10 (file comment) */
            f788AC();                                               /* 0x1C817 */
            if (D8(0xF625Cu) != 0)                                  /* 0x1C81C */
                ail_end_sample(D32(0xF6278u));                      /* 0x1C82C */
            if (D8(ROUTE + 2u) == 0xFF) {                           /* 0x1C834..0x1C840 */
                if (barb(t) > limit(t))                             /* 0x1C846..0x1C87F jle */
                    f57CE8(t, BARB, 0, barb(t) - limit(t));         /* 0x1C8C4 */
                snprintf(buf, sizeof buf, DSTR(0xB1855u), TERR_NAME(t));   /* 0x1C8E0 "The BRPC is deadlocked.
                                                                       They decide to settle in %s." */
                goto final;                                         /* 0x1C8E8 */
            }
        }
        f1D234(n, d, excess, &t);                                   /* 0x1C8F9 */
        if (S8(NATION(n) + 0x98u) != 0)                             /* 0x1C8FE dword [0xF1561 + 199n] sar 24 */
            continue;
        if (S8(NATION(n) + 0x51u) != 0)                             /* 0x1C916 dword [0xF151A + 199n] sar 24 */
            continue;
        if (barb(t) > limit(t))                                     /* 0x1C92E..0x1C967 jle */
            f57CE8(t, BARB, 0, barb(t) - limit(t));                 /* 0x1C9AC */
        snprintf(buf, sizeof buf, DSTR(0xB188Au), NAME(n), TERR_NAME(t));   /* 0x1C9D3 "%s has been wiped out by the
                                                                       Barbarians. Satisfied with a job well done they
                                                                       settle in %s." */
        goto final;
    }
final:
    f79270(-500, n, t, buf, 2);                                     /* 0x1C9EE */
}

/* ==== 0x1C9FC the landing list ===================================================================================== */

/* entry i of 0x1C9FC's local table [ebp-0x50] as a run reads it (BUG(1995) 3b): 0 when the run stops. The frame:
   [ebp-0x50] 6 entries, [ebp-0x38] int16 land[6], [ebp-0x2C] n */
static int c9_entry(const uint8_t *e, const int16_t *land, int n, int32_t count, int32_t i, uint8_t out[4])
{
    if (i < count && i < 6) {
        memcpy(out, e + 4 * i, 4);
        return 1;
    }
    if (!g_original)
        return 0;        /* fixed: the run stops at the count */
    if (i < 6 || i > 9)
        return 0;        /* stack garbage (or [ebp-0x28] and above): the port's stand-in stops the run */
    if (i < 9) {         /* land[2(i - 6)], land[2(i - 6) + 1], little-endian */
        out[0] = (uint8_t)land[2 * (i - 6)];
        out[1] = (uint8_t)((uint16_t)land[2 * (i - 6)] >> 8);
        out[2] = (uint8_t)land[2 * (i - 6) + 1];
        out[3] = (uint8_t)((uint16_t)land[2 * (i - 6) + 1] >> 8);
        return 1;
    }
    wr32(out, (uint32_t)n);   /* i == 9: n */
    return 1;
}

/* the search state restored from the snapshot blocks before each trial (0x1CDA5..0x1CE55 / 0x1CFD8..0x1D08A): areas
   j + 1 <- [0xEED74] + 69j and territory states j <- [0xEED7C] + 3j for j 0..156, nations j <- [0xEED78] + 199(j - 1)
   for j 1..11. Port guard: a block that is not there is skipped (the original would read through a NULL pointer) */
static void c9_restore(void)
{
    const uint8_t *a = load_heap_ptr(D32(SNAP_AREA), 0x2B20u);
    const uint8_t *s = load_heap_ptr(D32(SNAP_TSTATE), 0x1E0u);
    const uint8_t *m = load_heap_ptr(D32(SNAP_NATION), 0x88Du);
    int16_t j;           /* [ebp-0x18] */

    for (j = 0; j < 0x9D; j++) {
        if (a != NULL)
            memcpy(DP(AREA(j + 1)), a + 69 * j, 69);                /* 0x1CDE0 rep movsd (0x11) + movsb */
        if (s != NULL)
            memcpy(DP(TSTATE(j)), s + 3 * j, 3);                    /* 0x1CE0B movsw + movsb */
    }
    for (j = 1; j <= 0xB; j++)
        if (m != NULL)
            memcpy(DP(NATION(j)), m + 199 * (j - 1), 199);          /* 0x1CE50 rep movsd (0x31) + movsw + movsb */
}

/* the trials' run: list += entry i's +3 while it has entry 0's +2 and low nibble (0x1CF22..0x1CF77 /
   0x1D1D9..0x1D222) */
static void c9_run_trial(const uint8_t *e, const int16_t *land, int n, int32_t count)
{
    uint8_t x[4];
    int16_t i;           /* [ebp-0x20] */

    setD32(CHOICE_N, 0);                                            /* 0x1CF26 / 0x1D1D9 */
    for (i = 0; c9_entry(e, land, n, count, i, x); i++) {
        if (x[2] != e[2])                                           /* 0x1CF37 / 0x1D1EA */
            break;
        if ((x[0] & 0xF) != (e[0] & 0xF))                           /* 0x1CF50 / 0x1D1FF */
            break;
        append(x[3]);                                               /* 0x1CF61 / 0x1D20C */
    }
}

void f1C9FC(int n)
{
    int16_t land[6];     /* [ebp-0x38] */
    uint8_t e[24];       /* [ebp-0x50]: 6 entries */
    uint8_t x[4];
    int16_t nland;       /* [ebp-0x1C] */
    int16_t i;           /* [ebp-0x20] */
    int16_t j;           /* [ebp-0x18] */
    int16_t k;           /* [ebp-0x24] */
    uint32_t tmp;        /* [ebp-0x28] */
    int32_t t;

    memset(e, 0, sizeof e);   /* port: the uninitialised entries' stand-in */
    for (i = 0; i < 6; i++)                                         /* 0x1CA19..0x1CA35 */
        land[i] = 0;
    nland = 0;                                                      /* 0x1CA39 */
    for (i = 0; i < 0x9D; i++) {                                    /* 0x1CA3C, 0x1CA4B cmp 0x9D; jge */
        if ((int32_t)(D8(TERR(i)) >> 4) != n)                       /* 0x1CA64..0x1CA75 */
            continue;
        if (nland < 6)   /* port guard: at most 6 landing zones per nation in terrflag.dat (D) */
            land[nland] = i;                                        /* 0x1CA88 */
        nland++;                                                    /* 0x1CA7A */
    }
    if (nland > 6)
        nland = 6;       /* port guard (as above) */

    /* the zones where n has tokens or a city */
    setD32(CHOICE_N, 0);                                            /* 0x1CA91 */
    for (i = 0; i < nland; i++) {                                   /* 0x1CA99, 0x1CAA7 cmp ax, word; jge */
        t = land[i];
        if (D8(AREA(t + 1) + 0x0Eu + (uint32_t)n) == 0 && !own_city(t, n))   /* 0x1CACE, 0x1CAEC, 0x1CB09 */
            continue;
        append((uint32_t)t);                                        /* 0x1CB19..0x1CB31 */
    }
    if (S32(CHOICE_N) > 0) {                                        /* 0x1CB41 jle 0x1CCFD */
        if (S32(CHOICE_N) <= 1)                                     /* 0x1CB49 jle 0x1D229 */
            return;
        for (i = 0; i < S32(CHOICE_N); i++) {                       /* 0x1CB52, 0x1CBBB */
            t = S32(CHOICE + 4u * (uint32_t)i);
            if (own_city(t, n))                                     /* 0x1CBDB..0x1CC0C */
                e[4 * i] = (uint8_t)((e[4 * i] & 0xF0) | 6);        /* 0x1CC12..0x1CC26 */
            else
                e[4 * i] = (uint8_t)((e[4 * i] & 0xF0) | (D8(AREA(t + 1) + 0x0Eu + (uint32_t)n) & 0xF));   /* 0x1CB59 */
            e[4 * i + 3] = D8(CHOICE + 4u * (uint32_t)i);           /* 0x1CB95..0x1CBB1 the low byte */
        }
        /* 0x1CC2F: i = word [0xF7C9C] - 2, down to 0; j = 0..i: the larger low nibble first */
        for (i = (int16_t)(S32(CHOICE_N) - 2); i >= 0; i--)
            for (j = 0; j <= i; j++) {
                if ((e[4 * j + 4] & 0xF) <= (e[4 * j] & 0xF))      /* 0x1CC76 jbe */
                    continue;
                tmp = rd32(e + 4 * j + 4);                          /* 0x1CC82 */
                wr32(e + 4 * j + 4, rd32(e + 4 * j));               /* 0x1CC9F */
                wr32(e + 4 * j, tmp);                               /* 0x1CCB1 */
            }
        {
            int32_t count = S32(CHOICE_N);
            setD32(CHOICE_N, 0);                                    /* 0x1CCBB */
            for (i = 0; c9_entry(e, land, n, count, i, x); i++) {   /* 0x1CCC4 (no bound: BUG(1995) 3b) */
                if ((x[0] & 0xF) != (e[0] & 0xF))                   /* 0x1CCD4 */
                    break;
                append(x[3]);                                       /* 0x1CCE5 */
            }
        }
        return;
    }

    /* none: the empty zones (no nation, no city) */
    setD32(CHOICE_N, 0);                                            /* 0x1CD01 */
    for (i = 0; i < nland; i++) {                                   /* 0x1CD07, 0x1CD15 */
        t = land[i];
        if (S8(AREA(t + 1) + 1u) != 0)                              /* 0x1CD30 dword [69(t+1) + 0xE31B6] sar 24 */
            continue;
        if ((D8(TSTATE(t) + 2u) & 1) != 0)                          /* 0x1CD52 */
            continue;
        append((uint32_t)t);                                        /* 0x1CD5B..0x1CD73 */
    }
    if (S32(CHOICE_N) > 0) {                                        /* 0x1CD7F jle 0x1CF79 */
        int32_t count;
        if (S32(CHOICE_N) <= 1)                                     /* 0x1CD87 jle 0x1D229 */
            return;
        f7A4F0();                                                   /* 0x1CD92 */
        for (i = 0; i < S32(CHOICE_N); i++) {                       /* 0x1CD97, 0x1CF00 */
            c9_restore();                                           /* 0x1CDA5..0x1CE55 */
            memset(DP(VISITED), 0, 0xA0);                           /* 0x1CE63 */
            setD32(DEPTH, 0);                                       /* 0x1CE6C */
            setD8(0xF6F59u, 1);                                     /* 0x1CE72 */
            setD8(0xF6F5Au, 1);
            setD8(0xF6F5Bu, 1);
            t = S32(CHOICE + 4u * (uint32_t)i);
            f1D858(n, t, 0xF - limit(t), DP(ROUTE), DP(VISITED));   /* 0x1CECE: 15 - limit barbarians, not placed */
            memcpy(e + 4 * i, DP(ROUTE), 4);                        /* 0x1CEDD */
            e[4 * i + 3] = D8(CHOICE + 4u * (uint32_t)i);           /* 0x1CEF6 */
        }
        count = S32(CHOICE_N);
        f1DEAC(count, e);                                           /* 0x1CF1D */
        c9_run_trial(e, land, n, count);                            /* 0x1CF22..0x1CF77 */
        f7AB24();                                                   /* 0x1D224 */
        return;
    }

    /* none empty: every zone, each with 15 barbarians placed and the battle resolved first */
    setD32(CHOICE_N, 0);                                            /* 0x1CF7D */
    for (i = 0; i < nland; i++)                                     /* 0x1CF83, 0x1CF91 */
        append((uint32_t)(int32_t)land[i]);                         /* 0x1CF9B..0x1CFAF */
    if (S32(CHOICE_N) <= 1)                                         /* 0x1CFB6 jle 0x1D229 */
        return;
    f7A4F0();                                                       /* 0x1CFC5 */
    for (i = 0; i < S32(CHOICE_N); i++) {                           /* 0x1CFCA, 0x1D1B3 */
        c9_restore();                                               /* 0x1CFD8..0x1D08A */
        t = S32(CHOICE + 4u * (uint32_t)i);
        f57E28(t, BARB, 0xF);                                       /* 0x1D0A1 */
        memset(DP(LOST_TOK), 0, 0xE);                               /* 0x1D0B2 */
        memset(DP(LOST_CITY), 0, 0xE);                              /* 0x1D0C3 */
        if ((D8(TSTATE(t) + 2u) & 1) != 0)                          /* 0x1D0DC */
            f5B224(AREA(t + 1));                                    /* 0x1D102 */
        else
            f5B588(t + 1);                                          /* 0x1D115 */
        k = (int16_t)S8(AREA(t + 1) + 0x19u);                       /* 0x1D12F movsx ax, byte: the barbarians left */
        memset(DP(VISITED), 0, 0xA0);                               /* 0x1D147 */
        setD8(0xF6F59u, 1);                                         /* 0x1D150 */
        setD8(0xF6F5Au, 1);
        setD8(0xF6F5Bu, 1);
        setD32(DEPTH, 0);                                           /* 0x1D15F */
        f1D858(n, t, k, DP(ROUTE), DP(VISITED));                    /* 0x1D181 */
        memcpy(e + 4 * i, DP(ROUTE), 4);                            /* 0x1D196 */
        e[4 * i + 3] = D8(CHOICE + 4u * (uint32_t)i);               /* 0x1D1A9 */
    }
    {
        int32_t count = S32(CHOICE_N);
        f1DEAC(count, e);                                           /* 0x1D1D4 */
        c9_run_trial(e, land, n, count);                            /* 0x1D1D9..0x1D222 */
    }
    f7AB24();                                                       /* 0x1D224 */
}

/* ==== 0x1D234 one move ============================================================================================= */

/* the losses box after a barbarian invasion (0x1D610..0x1D685 / 0x1D7CB..0x1D83F); title is the path's own copy of
   "BARBARIAN CONFLICT - UNITS REMOVED" (edx survives f3B05C: it saves edx, 0x3B06B) */
static void losses_box(uint32_t title)
{
    setD16(MSG_H, 0x14);                                            /* 0x1D61A / 0x1D7D5 */
    f3CD6C(3);                                                      /* 0x1D621 / 0x1D7DC */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BACKGROUND), S32(BACKBUF));   /* 0x1D649 / 0x1D803 */
    f7876C();                                                       /* 0x1D651 / 0x1D80B */
    f7853C(LOSSES_DLG);                                             /* 0x1D65B / 0x1D815 */
    f3B05C(PANEL_DLG);                                              /* 0x1D66A / 0x1D824 */
    (void)f5CDE8(0, DSTR(title));                                   /* 0x1D671 / 0x1D82B */
    f78664(LOSSES_DLG);                                             /* 0x1D680 / 0x1D83A */
    setD16(MSG_H, 0x23);                                            /* 0x1D685 / 0x1D83F */
}

/* the invasion of territory dest by k barbarians from *pt (0x1D51C..0x1D69D / 0x1D6E9..0x1D84C) */
static void invade(int n, int k, int32_t *pt, int32_t dest, uint32_t fmt, uint32_t title)
{
    char buf[100];       /* [ebp-0x88] */

    if (*pt != -1)                                                  /* 0x1D521 / 0x1D6EC */
        f57CE8(*pt, BARB, 0, k);                                    /* 0x1D535 / 0x1D700 */
    f57E28(dest, BARB, k);                                          /* 0x1D547 / 0x1D710 */
    snprintf(buf, sizeof buf, DSTR(fmt), TERR_NAME(dest));          /* 0x1D565 / 0x1D72C "%s is invaded." */
    f79270(-500, n, dest, buf, 2);                                  /* 0x1D583 / 0x1D747 */
    memset(DP(LOST_TOK), 0, 0xE);                                   /* 0x1D594 / 0x1D758 */
    memset(DP(LOST_CITY), 0, 0xE);                                  /* 0x1D5A5 / 0x1D769 */
    if ((D8(TSTATE(dest) + 2u) & 1) != 0)                           /* 0x1D5C7 / 0x1D786 */
        f5B224(AREA(dest + 1));                                     /* 0x1D5DD / 0x1D79C */
    else
        f5B588(dest + 1);                                           /* 0x1D5E4 / 0x1D7A3 */
    setD8(AREA(dest + 1) + 0x3Bu + (uint32_t)n, 2);                 /* 0x1D600 / 0x1D7BB */
    if (f1DFFC() != 0)                                              /* 0x1D607 / 0x1D7C2 */
        losses_box(title);
    *pt = dest;                                                     /* 0x1D695 / 0x1D84C */
}

void f1D234(int n, int d, int k, int32_t *pt)
{
    char buf[100];       /* [ebp-0x88] */
    int32_t i;           /* [ebp-0x18] */
    int32_t j;           /* [ebp-0x14] */
    int32_t dest;        /* [ebp-0x10] */

    if (*pt != -1 && D8(ROUTE + 1u) == 0 && D8(ROUTE + 2u) == 0) {  /* 0x1D258, 0x1D263, 0x1D270 */
        /* the route's equal-best entries (the list's count) by their high nibble, the larger first (0x1D27D) */
        for (i = S32(CHOICE_N) - 2; i >= 0; i--)                    /* 0x1D285, 0x1D29C */
            for (j = 0; j <= i; j++) {                              /* 0x1D294, 0x1D2B5 jg */
                if ((D8(ROUTE + 4u * (uint32_t)j + 4u) >> 4) <= (D8(ROUTE + 4u * (uint32_t)j) >> 4))   /* 0x1D2D8 jbe */
                    continue;
                setD32(SWAP, D32(ROUTE + 4u * (uint32_t)j + 4u));   /* 0x1D2E4 */
                setD32(ROUTE + 4u * (uint32_t)j + 4u, D32(ROUTE + 4u * (uint32_t)j));   /* 0x1D2F6 */
                setD32(ROUTE + 4u * (uint32_t)j, D32(SWAP));        /* 0x1D305 */
            }
        setD32(CHOICE_N, 0);                                        /* 0x1D310 */
        for (i = 0;; i++) {                                         /* 0x1D316 */
            /* BUG(1995) 3a: no bound (file comment) */
            if (!g_original && i >= 10)
                break;
            if (S32(CHOICE_N) >= 0x9D)                              /* port guard: the list's size */
                break;
            if (D8(ROUTE + 4u * (uint32_t)i + 2u) != D8(ROUTE + 2u))   /* 0x1D323 */
                break;
            if ((D8(ROUTE + 4u * (uint32_t)i) >> 4) != (D8(ROUTE) >> 4))   /* 0x1D340 */
                break;
            append(D8(ROUTE + 4u * (uint32_t)i + 3u));              /* 0x1D34C */
        }
        if ((D8(ROUTE) >> 4) == 0) {                                /* 0x1D367..0x1D374 jne 0x1D43E */
            k = barb(*pt) - limit(*pt);                             /* 0x1D37A..0x1D3B4 [ebp-0x20] */
            f57CE8(*pt, BARB, 0, k);                                /* 0x1D3C6 */
            snprintf(buf, sizeof buf, DSTR(0xB18E5u), ADJ(n));      /* 0x1D3E2 "The Barbarian Hordes can no longer do
                                                                       any damage to %s units in adjacent
                                                                       territories." */
            f79270(-500, n, *pt, buf, 2);                           /* 0x1D3FF */
            if (S8(AREA(*pt + 1) + 0x0Eu + (uint32_t)n) > 0)        /* 0x1D41B dword [n + 69(t+1) + 0xE31C3] sar 24;
                                                                       jle */
                setD8(AREA(*pt + 1) + 0x3Bu + (uint32_t)n, 2);      /* 0x1D430 */
            return;
        }
        f57CE8(*pt, BARB, 0, k);                                    /* 0x1D44D */
        if (S32(CHOICE_N) > 1)                                      /* 0x1D458 jle */
            dest = S32(CHOICE + 4u * (((uint32_t)f78AE4() * D32(CHOICE_N)) >> 15));   /* 0x1D45D game_rand; imul;
                                                                       shr 15 */
        else
            dest = S32(CHOICE);                                     /* 0x1D475 */
        *pt = dest;                                                 /* 0x1D47D */
        f57E28(*pt, BARB, D8(ROUTE) >> 4);                          /* 0x1D495: the high nibble of entry 0 */
        snprintf(buf, sizeof buf, DSTR(0xB193Bu), TERR_NAME(*pt));  /* 0x1D4B3 "The Barbarians finally settle in
                                                                       %s." */
        f79270(-500, n, *pt, buf, 2);                               /* 0x1D4D0 */
        if (S8(AREA(*pt + 1) + 0x0Eu + (uint32_t)n) > 0)            /* 0x1D4EC; jle */
            setD8(AREA(*pt + 1) + 0x3Bu + (uint32_t)n, 2);          /* 0x1D501 */
        return;
    }

    if (S32(CHOICE_N) == 1) {                                       /* 0x1D50F cmp 1; jne 0x1D69E */
        invade(n, k, pt, S32(CHOICE), 0xB1960u, 0xB196Fu);
        return;
    }
    if ((D8(FLAGS) & PBEM_BIT) == 0 && S8(NATION(d) + 0x9Au) == 3)  /* 0x1D6A4, 0x1D6B0 dword [0xF1563 + 199d] sar
                                                                       24 == 3 */
        dest = f207B4(d, n, 0x1A);                                  /* 0x1D6C9: the director picks */
    else
        dest = S32(CHOICE + 4u * (((uint32_t)f78AE4() * D32(CHOICE_N)) >> 15));   /* 0x1D6D0 game_rand; imul;
                                                                       shr 15 */
    invade(n, k, pt, dest, 0xB1992u, 0xB19A1u);
}

/* ==== 0x1D858 the route search ===================================================================================== */

void f1D858(int n, int t, int k, uint8_t *out, const uint8_t *vis_in)
{
    uint8_t vis[160];    /* [ebp-0x128] */
    uint8_t e[40];       /* [ebp-0x88]: 10 entries */
    uint8_t cand[20];    /* [ebp-0x60] */
    int8_t cnt;          /* [ebp-0x18] */
    int8_t any;          /* [ebp-0x14] */
    int8_t here;         /* [ebp-0x10] */
    int8_t dup;          /* [ebp-0x0C] */
    uint32_t list;       /* [ebp-0x40] a handle */
    int32_t i;           /* [ebp-0x3C] */
    int32_t j;           /* [ebp-0x38] */
    int32_t x;           /* [ebp-0x30] */
    int32_t c;           /* [ebp-0x2C] */
    int32_t excess;      /* [ebp-0x34] */
    uint32_t ha, hn, ht; /* [ebp-0x24] areas, [ebp-0x20] nations, [ebp-0x1C] territory states */
    uint8_t *pa, *pn, *pt;

    /* port: the uninitialised entries' stand-in {0x0F, 0xFF, 0xFF, 0xFF} (file comment) */
    for (i = 0; i < 10; i++)
        wr32(e + 4 * i, 0xFFFFFF0Fu);

    if (S32(BRPC_MODE) != 0) {                                      /* 0x1D879 */
        setD32(BRPC_CALLS, D32(BRPC_CALLS) + 1u);                   /* 0x1D886 */
        if (S32(BRPC_CALLS) % 0xFA == 0) {                          /* 0x1D89B idiv 250; edx == 0 */
            int32_t s = S32(BRPC_CALLS) / 0xFA % 4;                 /* 0x1D8AB / 0x1D8B7 */
            snprintf((char *)DP(0xF5500u), 0x1F4, DSTR(0xB19C4u),   /* 0x1D8D0 "%s %s": the BRPC text, the spinner */
                     DSTR(0xBDD88u), DSTR(D32(0xBB404u + 4u * (uint32_t)s)));
            fg_vbopen(S32(BACKBUF));                                /* 0x1D8DF */
            f3A990();                                               /* 0x1D8E7 */
            f3A9F4(DSTR(0xF5500u));                                 /* 0x1D8F1 */
            fg_vbclose();                                           /* 0x1D8F6 */
            f3CBBC();                                               /* 0x1D8FB */
        }
    }
    memcpy(vis, vis_in, 0xA0);                                      /* 0x1D914 repne movsd + movsb (0xA0 bytes) */

    /* the unvisited land neighbours, then the coast neighbours not already listed */
    list = D32(LANDADJ + 4u * (uint32_t)t);                         /* 0x1D921 */
    cnt = 0;                                                        /* 0x1D92D */
    for (i = 1; hword(list, 0) >= i; i++) {                         /* 0x1D930, 0x1D942 movsx; jl */
        c = hword(list, 2u * (uint32_t)i);                          /* 0x1D952 */
        if (vis[c] != 0)                                            /* 0x1D95B */
            continue;
        if (cnt < 10)    /* port guard: at most 10 candidates per territory in the shipped lists (D) */
            cand[cnt++] = (uint8_t)c;                               /* 0x1D966..0x1D972 */
    }
    list = D32(COASTADJ + 4u * (uint32_t)t);                        /* 0x1D97B */
    for (i = 1; hword(list, 0) >= i; i++) {                         /* 0x1D985, 0x1D9DF */
        c = hword(list, 2u * (uint32_t)i);                          /* 0x1D9EF */
        dup = 0;                                                    /* 0x1D9F7 */
        for (j = 0; cnt > j; j++)                                   /* 0x1D9FC, 0x1D994 movsx byte; jle */
            if ((int32_t)cand[j] == c) {                            /* 0x1D9A9 */
                dup = 1;                                            /* 0x1D9AE */
                break;
            }
        if (dup != 0)                                               /* 0x1D9B2 */
            continue;
        if (vis[c] != 0)                                            /* 0x1D9BB */
            continue;
        if (cnt < 10)    /* port guard (as above) */
            cand[cnt++] = (uint8_t)c;                               /* 0x1D9C6..0x1D9D2 */
    }
    vis[t] = 1;                                                     /* 0x1DA04 */
    any = 0;                                                        /* 0x1DA0E */
    if (cnt == 0) {                                                 /* 0x1DA11 jne 0x1DA3A */
        out[0] &= 0xF0;                                             /* 0x1DA1C */
        out[1] = 0;                                                 /* 0x1DA22 */
        out[2] = 0xFF;                                              /* 0x1DA28 */
        out[3] = (uint8_t)t;                                        /* 0x1DA32 */
        return;
    }
    /* any: a neighbour holds n's tokens or city (then no move goes further) */
    for (i = 0; cnt > i; i++) {                                     /* 0x1DA3A, 0x1DA47 */
        c = cand[i];                                                /* 0x1DA55 */
        if (D8(AREA(c + 1) + 0x0Eu + (uint32_t)n) != 0 || own_city(c, n)) {   /* 0x1DA75, 0x1DA85, 0x1DAA5 */
            any = 1;                                                /* 0x1DAAA */
            break;
        }
    }

    ha = load_heap_alloc(0x2A51u);                                  /* 0x1DAB3 areas 1..157 */
    hn = load_heap_alloc(0x88Du);                                   /* 0x1DAC0 nations 1..11 */
    ht = load_heap_alloc(0x1D7u);                                   /* 0x1DACD territory states 0..156 */
    pa = load_heap_ptr(ha, 0x2A51u);
    pn = load_heap_ptr(hn, 0x88Du);
    pt = load_heap_ptr(ht, 0x1D7u);
    /* port guard: a block that is not there (out of memory) is skipped */
    if (pa != NULL)
        memcpy(pa, DP(0xE31FDu), 0x2A51);                           /* 0x1DAE8 */
    if (pt != NULL)
        memcpy(pt, DP(0xE2C18u), 0x1D7);                            /* 0x1DB05 */
    if (pn != NULL)
        memcpy(pn, DP(0xF1593u), 0x88D);                            /* 0x1DB22 */

    for (x = 0; cnt > x; x++) {                                     /* 0x1DB2E, 0x1DBED movsx byte; jle */
        c = cand[x];                                                /* 0x1DBFF */
        if (pa != NULL)
            memcpy(DP(0xE31FDu), pa, 0x2A51);                       /* 0x1DC19 */
        if (pt != NULL)
            memcpy(DP(0xE2C18u), pt, 0x1D7);                        /* 0x1DC36 */
        if (pn != NULL)
            memcpy(DP(0xF1593u), pn, 0x88D);                        /* 0x1DC53 */
        here = 0;                                                   /* 0x1DC5F */
        if (D8(AREA(c + 1) + 0x0Eu + (uint32_t)n) != 0 || own_city(c, n))   /* 0x1DC7B, 0x1DC8B, 0x1DCAB */
            here = 1;                                               /* 0x1DCB0 */
        f57E28(c, BARB, k);                                         /* 0x1DCBF */
        memset(DP(LOST_TOK), 0, 0xE);                               /* 0x1DCD0 */
        memset(DP(LOST_CITY), 0, 0xE);                              /* 0x1DCE1 */
        if ((D8(TSTATE(c) + 2u) & 1) != 0)                          /* 0x1DCF7 */
            f5B224(AREA(c + 1));                                    /* 0x1DD18 */
        else
            f5B588(c + 1);                                          /* 0x1DB36 */
        if (here != 0) {                                            /* 0x1DB3B */
            /* 0x1DB45..0x1DB75: low nibble = (3 * cities lost + tokens lost of n) & 0xF (byte arithmetic) */
            e[4 * x] = (uint8_t)((e[4 * x] & 0xF0) |
                                 ((uint8_t)(D8(LOST_CITY + (uint32_t)n) * 3u + D8(LOST_TOK + (uint32_t)n)) & 0xF));
            e[4 * x + 1] = (uint8_t)f1DFFC();                       /* 0x1DB7C, 0x1DB84 */
            e[4 * x + 2] = D8(DEPTH);                               /* 0x1DB93 */
            e[4 * x + 3] = (uint8_t)c;                              /* 0x1DBA0 */
            if (S32(DEPTH) == 0)                                    /* 0x1DBA7 jne */
                /* 0x1DBB1..0x1DBE0: high nibble = the barbarians left there & 0xF */
                e[4 * x] = (uint8_t)((e[4 * x] & 0x0F) | (uint8_t)((D8(AREA(c + 1) + 0x19u) & 0xF) << 4));
            continue;
        }
        if (any == 0) {                                             /* 0x1DD22 jne 0x1DDB1 */
            excess = barb(c) - limit(c);                            /* 0x1DD2E..0x1DD67 */
            if (excess > 0) {                                       /* 0x1DD6A jle 0x1DDB1 */
                setD32(DEPTH, D32(DEPTH) + 1u);                     /* 0x1DD77 */
                f1D858(n, c, excess, e + 4 * x, vis);               /* 0x1DD94 */
                e[4 * x + 3] = (uint8_t)c;                          /* 0x1DD9F */
                setD32(DEPTH, D32(DEPTH) - 1u);                     /* 0x1DDA6 */
                continue;
            }
        }
        e[4 * x] &= 0xF0;                                           /* 0x1DDB4 */
        e[4 * x + 1] = (uint8_t)f1DFFC();                           /* 0x1DDBC, 0x1DDC4 */
        e[4 * x + 2] = 0xFF;                                        /* 0x1DDCE */
        e[4 * x + 3] = (uint8_t)c;                                  /* 0x1DDDC */
    }
    if (pa != NULL)
        memcpy(DP(0xE31FDu), pa, 0x2A51);                           /* 0x1DDFB */
    if (pt != NULL)
        memcpy(DP(0xE2C18u), pt, 0x1D7);                            /* 0x1DE18 */
    if (pn != NULL)
        memcpy(DP(0xF1593u), pn, 0x88D);                            /* 0x1DE35 */
    load_heap_free(ha);                                             /* 0x1DE42 */
    load_heap_free(hn);                                             /* 0x1DE4A */
    load_heap_free(ht);                                             /* 0x1DE52 */
    f1DEAC(cnt, e);                                                 /* 0x1DE61 */
    memcpy(out, e, 4);                                              /* 0x1DE66..0x1DE6F */
    if (S32(DEPTH) == 0)                                            /* 0x1DE71 jne */
        for (i = 0; i < 0xA; i++)                                   /* 0x1DE7B, 0x1DE86 cmp 0xA; jge */
            memcpy(out + 4 * i, e + 4 * i, 4);                      /* 0x1DE98..0x1DE9F */
}

/* ==== 0x1DEAC the route entries' sort ============================================================================== */

void f1DEAC(int count, uint8_t *e)
{
    int32_t i;           /* [ebp-0x8] */
    int32_t j;           /* [ebp-0x4] */

    for (i = count - 2; i >= 0; i--)                                /* 0x1DEC7, 0x1DEE1 */
        for (j = 0; j <= i; j++) {                                  /* 0x1DED9, 0x1DEFE jg */
            if ((e[4 * j + 4] & 0xF) <= (e[4 * j] & 0xF))           /* 0x1DF21 jbe */
                continue;
            setD32(SWAP, rd32(e + 4 * j + 4));                      /* 0x1DF2F */
            wr32(e + 4 * j + 4, rd32(e + 4 * j));                   /* 0x1DF4C */
            wr32(e + 4 * j, D32(SWAP));                             /* 0x1DF5F */
        }
    for (i = 1; i < count; i++)                                     /* 0x1DF63, 0x1DF78 jge */
        for (j = count - 1; j >= i; j--) {                          /* 0x1DF82, 0x1DF93 jl */
            if (e[4 * j - 2] <= e[4 * j + 2])                       /* 0x1DFB3 jbe */
                continue;
            setD32(SWAP, rd32(e + 4 * j - 4));                      /* 0x1DFC1 */
            wr32(e + 4 * j - 4, rd32(e + 4 * j));                   /* 0x1DFDE */
            wr32(e + 4 * j, D32(SWAP));                             /* 0x1DFF1 */
        }
}

/* ==== 0x1DFFC the losses of everybody ============================================================================== */

int f1DFFC(void)
{
    int32_t sum = 0;     /* [ebp-0x4] */
    int32_t i;           /* [ebp-0x8] */

    for (i = 1; i < 0xB; i++)                                       /* 0x1E012, 0x1E021 */
        if (S8(LOST_TOK + (uint32_t)i) != 0)                        /* 0x1E02A dword [0xF5479 + i] sar 24 */
            sum += S8(LOST_TOK + (uint32_t)i);                      /* 0x1E037 */
    for (i = 1; i < 0xB; i++)                                       /* 0x1E03C, 0x1E04B */
        if (S8(LOST_CITY + (uint32_t)i) != 0) {                     /* 0x1E054 dword [0xF5489 + i] sar 24 */
            sum += S8(LOST_CITY + (uint32_t)i);                     /* 0x1E061 */
            if (g_original)
                break;   /* BUG(1995) 1: the original falls into the return after its first add (0x1E064) */
        }
    return sum;                                                     /* 0x1E064 */
}

/* ==== 0x7A4F0 / 0x7AB24 the snapshot =============================================================================== */

void f7A4F0(void)
{
    uint8_t *p;

    setD32(SNAP_AREA, load_heap_alloc(0x2B20u));                    /* 0x7A505, 0x7A50A */
    setD32(SNAP_NATION, load_heap_alloc(0x88Du));                   /* 0x7A519, 0x7A51E */
    setD32(SNAP_TSTATE, load_heap_alloc(0x1E0u));                   /* 0x7A52D, 0x7A538 */
    /* port guard: a block that is not there (out of memory) is skipped */
    if ((p = load_heap_ptr(D32(SNAP_AREA), 0x2B20u)) != NULL)
        memcpy(p, DP(0xE31FDu), 0x2B20);                            /* 0x7A543 areas 1..160 */
    if ((p = load_heap_ptr(D32(SNAP_TSTATE), 0x1E0u)) != NULL)
        memcpy(p, DP(0xE2C18u), 0x1E0);                             /* 0x7A563 territory states 0..159 */
    if ((p = load_heap_ptr(D32(SNAP_NATION), 0x88Du)) != NULL)
        memcpy(p, DP(0xF1593u), 0x88D);                             /* 0x7A583 nations 1..11 */
}

void f7AB24(void)
{
    const uint8_t *p;

    /* port guard: a field without a block is skipped (the original copies from address 0) */
    if ((p = load_heap_ptr(D32(SNAP_AREA), 0x2B20u)) != NULL)
        memcpy(DP(0xE31FDu), p, 0x2B20);                            /* 0x7AB4B */
    if ((p = load_heap_ptr(D32(SNAP_TSTATE), 0x1E0u)) != NULL)
        memcpy(DP(0xE2C18u), p, 0x1E0);                             /* 0x7AB6B */
    if ((p = load_heap_ptr(D32(SNAP_NATION), 0x88Du)) != NULL)
        memcpy(DP(0xF1593u), p, 0x88D);                             /* 0x7AB8B */
    load_heap_free(D32(SNAP_AREA));                                 /* 0x7AB9C */
    setD32(SNAP_AREA, 0);                                           /* 0x7ABA6 */
    load_heap_free(D32(SNAP_NATION));                               /* 0x7ABAC */
    setD32(SNAP_NATION, 0);                                         /* 0x7ABB6 */
    load_heap_free(D32(SNAP_TSTATE));                               /* 0x7ABBC */
    setD32(SNAP_TSTATE, 0);                                         /* 0x7ABC1 */
}
