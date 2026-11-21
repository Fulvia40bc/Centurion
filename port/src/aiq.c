/* Module AIQ of docs/M6_PLAN.md: the computer player's queries, filters and scores (milestone M7, batch 1; 14,988
   bytes). Literal translations; the address of each instruction a line comes from is in its comment. D = read in the
   code, G = inference. Prototypes: aiq.h. The module's three small formulas 0x7CAB4, 0x46A4C and 0x46B34 are
   formulas.c's (ported earlier).

     0x48028  f48028  neighbour filter, 25-entry jump table at 0x47FC4 (22 call sites)
     0x4876C  f4876C  weak neighbours, filter 0x16 (f46CBC, f471CC)
     0x4892C  f4892C  second ring of a territory list (0x348B0, 0x3580C, f4D110, f7BD5C (2))
     0x48D44  f48D44  sort / filter a territory list (20 call sites)
     0x47BA0  f47BA0  n's territories by mode (f46BB8, f478D8)
     0x495C0  f495C0  the strongest enemy next to t (0x3580C (4), f374E0, f46CBC (2), f471CC, 0x65010)
     0x49790  f49790  defenders to leave (0x3580C (4), f374E0, f46CBC, f471CC, 0x65010)
     0x49934  f49934  can n reach t (0x33ECC, f39068)
     0x4CC5C  f4CC5C  tokens needed to take t (f38E54, f474A0 (2), f4BBDC, f4C9B8)
     0x4CEE4  f4CEE4  tokens needed to take the city at t (f39068 (2), f4C9B8)
     0x4D110  f4D110  land reinforcements of m near t (f387C8, f4CC5C, f4CEE4)
     0x7BD5C  f7BD5C  the map analysis: n's row of the flags 0xF3580 (f353C4, f461BC)
     0x7C838  f7C838  conflict trial (f3580C, f474A0 (2), f4A088, f4AC50 (2), f4BBDC, f4C074, f4C6E4 (2))
     0x7B044  f7B044  position score (f61254, f62840 (2), f65010)
     0x468EC  f468EC  aggression trigger (f461BC)
     0x46998  f46998  mark targets (f461BC)
     0x39C08  f39C08  city sites held (f38E54)
     0x160EC  f160EC  opening-map priorities (f353C4, f461BC, f46BB8)
     0x7701C  f7701C  table shuffle (f33ECC)
   No routine of this module is called from code ported before M7 (D: no not_ported(0x...) of these addresses in
   port/src); every caller is in AICITY, AISEA, AIMOVE, AIPLAN or SHIPAI.

   Not ported here (static stub carrying the name, so a later translation that exports it collides at compile time
   instead of being shadowed):
     0x4D29C  AISEA (aisea.c, this batch): int f4D29C(int t, int n), the tokens n can bring to t by sea (pushes and
              pops both stacks, f58888, qsort). Called by f4CEE4 at 0x4CFC9. Built with -DAIQ_TEST
              (port/test/aiq_test.c) it goes to the test's recorder, which returns.

   Calls into ported code: f78D08, f7C760, f7C7C4 (rules.c), f7CAB4 (formulas.c), f5843C (route.c), f5B588
   (conflict.c), f7A778 / f7A7B0 (mstack.c, the nations snapshot 0xF1593 -> 0x116FFC). No World State or Move List
   Stack push, pop, peek or drop in this module (D); the trials of f495C0, f39C08 and f7C838 use the fixed nations
   snapshot and a 69-byte copy of the area record on their own stack. No game_rand f78AE4, no clock 0x8A080, no wait,
   no screen, no trace string (D). The pre-rolled table 0x112A94 (f7BCD4, saved) is read by f4876C, f49790 and f7701C.

   Watcom runtime -> standard C: 0x7D5D0 memset(eax, edx, ebx) -> memset in g_ds (f7BD5C keeps ecx across it: the
   original's [ebp-0x64] = 1 relies on that, D 0x7BDAE); rep movsd / movsb copies -> memcpy (69-byte area records,
   lists into locals). The Watcom convention keeps edx / ecx across a call: the byte [0xBB3F8] = dl / cl / ch after
   f5B588 is the 1 loaded before the call (D 0x49767, 0x39D82, 0x7C905). __CHK dropped.

   Data (D, read in the code at the addresses given; records as in aiq.h):
     adjacency: [0xF8E7C + 4t] a heap handle (load.c) to int16 count, int16 territories (game.h); read through
       load_heap_ptr as cal4.c does
     0xEE16C uint8[160] in play; 0xC803C uint8[160] the canonical territory (f160EC); 0xF62A8 uint8[10] / int32
       [0xF62B4] / 0xF62B8 uint8[9][5] the flagged list of f160EC; 0xEF3C4 int8[10][160] the base scores (loop's
       0x14AA4 copies 0xEED84 there after a restore)
     [0xF09E4] int32 1 lets f4892C run: some nation 1..9 holds +0x28 (G: Roadbuilding; set by the movement phase
       0x557C9..0x557F1, the loop 0x55118 and the restore, saveload.c 0x5FEED / 0x5FF15)
     [0x108190] uint8 set by f48D44's flag 0x20 when it drops an entry (never cleared here)
     [0xBB3F8] uint8 1 real resolution / 0 trial; 0xF547C uint8[10] the conflict losses (f7C838 clears and reads them)
     [0x1079B4] int32 / 0x107694 4-byte entries: the move list 1; byte 0 == 0xC9 an attack on nation byte 1 (f7B044)
     0xBCCE0 int8[9][17][5] the attack index rows (nation m - 1, AST square of n); 0xC2864 int32[][3] the weights by
       AI level; 0xF488C uint8[10] and int32 0x108164[10] the "weak" switches of f7B044; [0xF5470] int32 (G: the round)
     0xEFA04 int8[10][160] the values f7B044 adds for marked sites ([0xEFA03 + 160n + a], a = territory + 1)
   AI flags 0xF3580 of n (f7BD5C; meanings G except where the code says):
     lo 0x01 an empty territory whose land neighbours are all empty or held only by foreigners
     lo 0x02 an empty territory with no second ring (n lacks +0x28, G: Roadbuilding as loop.c reads it, or [0xF09E4]
             is 0) or with a second-ring entry not held by n alone
     lo 0x04 an empty territory next to n alone, or next to the Pirates / Barbarians and n (see below)
     lo 0x08 an empty territory whose second ring is all n alone
     lo 0x10 one foreign nation alone there, or several nations without n's marker
     lo 0x20 n alone with a neighbour held by others; lo 0x40 the same in the second ring
     lo 0x80 n alone with no such neighbour (also when one neighbour is n's or a city: see the code)
     hi 0x01 n alone, second ring with nobody else; hi 0x02 n contested; hi 0x04 an empty territory where the last
     foreign neighbour scanned outnumbers limit + Metalworking; hi 0x08 a lone enemy pocket (no neighbour of the
     same foreign nation with flag lo 0x10 / hi 0x02)

   1995 bugs, fixed unless g_original (each marked BUG(1995) in the code; grades D code / G effect):
     1. 0x7701C reads stack garbage (Risk 7): when the first row's pick is in range, the routine takes a second row
        (0x770CF) and does not clamp that pick; for a remaining count c with 30000 % c != 0 the pick can equal c
        (e.g. c = 7, table value 29995..29999). On the first pick c is the caller's count, so the byte read is the
        uninitialised local just past the copy ([ebp-0xBC + count]), which goes into the caller's list while one real
        entry is lost. Later picks read a stale but defined byte (the last entry the gap-closing left behind):
        deterministic, kept. Fixed: a pick >= the caller's count is clamped to c - 1 (only possible on the first
        pick). With g_original the port reads its zeroed local there (a port cannot reproduce the garbage). D code,
        G reach (about 1 in 6,000 calls per row pair for c = 7; the table is fixed for a round, so a round can hit it
        repeatedly).
   Kept as the original (D unless marked; AI choices, not crashes):
     - f46998's loop runs to t = 157 (Risk 8): it reads flag entry 157, i.e. the high byte of the next row's entry 0
       (for n = 9 the byte 0xF408B after the table), the score 0xEED84[n][157] and area record 158, and may write
       area record 158's mark. Read through the data object, byte-exact.
     - f39C08 restores the area record before it tests the trial's result, so a contested site never counts; the
       trial f5B588 still runs (its losses 0xF547C stay).
     - f4876C does not reset *cnt (it appends); f48D44's flag 0x10 only shortens the count (the list is sorted
       descending, so it keeps the first entries with scores >= 0); its score(t) is read in four steps that use it
       in one (0x02).
     - f7BD5C: the compare of the city owner with n at 0x7C71E has no branch (its result is thrown away); the
       strongest foreign stack (hi 0x04) is the one of the LAST foreign neighbour scanned (best and nation are reset
       per neighbour, 0x7BE5F); lo 0x80 is set whenever no neighbour is held by others, whatever a58 says; the lone-
       pocket test compares the second neighbour list's single entry with the first neighbour itself (0x7C6A3), which
       a land list never holds, so hi 0x08 comes only from an empty first list; neighbours u > t are tested against
       flags not yet computed (still 0 from the memset); territories 157..159 read the next row's entry 0.
     - f7701C: when the first row's pick is in range it is discarded and the next row decides (two rows per pick).
     - f160EC adds its +5 to the base 0xEF3C4, and the loop's restore 0x14AA4 copies the saved 0xEED84 (which holds
       the last +5s) over that base (loop.c, 0x14B0E): each restore of a save adds another +5 to the territories
       flagged then (D, both pieces). This settles M6_PLAN's LOOP question: cumulative across restores, not within a
       run. AI choices only: kept.
     - f160EC: the list 0xF62B8 + 5(n-1) has no bound: a sixth flagged territory would go into the next nation's list
       (D code). Unreachable with the game's data (D, review 21 Nov): of the 65 opening maps *.BPM in ACIV.CT3 (+0xCB
       -> 0xF0044 + 160n) only the ILLY maps set bit 0x04, on one territory each (the others none), and the save
       carries the same 0xF0044. The cumulative +5 above therefore touches Illyria's one flagged territory only; being
       an int8, it wraps after enough restores (AI choice only, kept).
     - f7B044: the room-to-grow sum [ebp-0x38] is computed and never used; the AI level 3 would index the next row
       of 0xC2864; a non-zero site bit with a value of -1 adds 0 instead of -1/4 = 0 (same result).
     - f49790's max(limit - 1, 1) turns only 0 into 1 (limit 0 gives -1).
     - f4CC5C / f4CEE4 / f7B044 index nations 10..13 into nine- and ten-nation tables when the owner or the attacked
       nation is one of them (0xF06C0, 0xF52E0, 0xBCCE0): deterministic data-object reads, kept.
     - f48D44 copies the list into a 157-int local ([ebp-0x2A8]..[ebp-0x35]); a list of more than 157 entries would
       overwrite its own locals. No caller passes one (G: territory lists of < 157 entries), so the port's local is
       160 ints and no guard is added.
   Easter eggs / oddities: none of these routines prints anything; f7BD5C's dead compare, the self-neighbour test
   and the one-row-too-far loop of f46998 are developer slips, not jokes. */

#include <stdint.h>
#include <string.h>

#include "aiq.h"
#include "conflict.h"
#include "formulas.h"
#include "game.h"
#include "mstack.h"
#include "route.h"
#include "rules.h"
#include "state.h"

#define NATION(n)     (0xF14CCu + 199u * (uint32_t)(n))
#define AREA(r)       (0xE31B8u + 69u * (uint32_t)(r))           /* area record r = territory r - 1 */
#define CNT(r)        S8(AREA(r) + 1u)                            /* dword [69r + 0xE31B6] sar 24 */
#define MARK(r, n)    S8(AREA(r) + 1u + (uint32_t)(n))            /* dword [69r + n - 1 + 0xE31B7] sar 24 */
#define TOK(r, n)     S8(AREA(r) + 0x0Eu + (uint32_t)(n))         /* dword [69r + n - 1 + 0xE31C4] sar 24 */
#define MOV(r, n)     S8(AREA(r) + 0x24u + (uint32_t)(n))         /* dword [69r + n - 1 + 0xE31DA] sar 24 */
#define TSTATE(t)     (0xE2C18u + 3u * (uint32_t)(t))
#define TERR(t)       (0xE1638u + 35u * (uint32_t)(t))
#define POP_LIMIT(t)  ((int32_t)((D8(TERR(t) + 1u) >> 3) & 7u))   /* shl al, 2; shr al, 5 */
#define SITE(t)       ((D8(TERR(t) + 1u) & 0x40u) != 0)
#define CITY(t)       ((D8(TSTATE(t) + 2u) & 0x01u) != 0)
#define MARKED(t)     ((D8(TSTATE(t) + 2u) & 0x02u) != 0)
#define AIF(n, t)     (0xF3580u + 314u * (uint32_t)((n) - 1) + 2u * (uint32_t)(t))   /* low byte; +1 the high */
#define SCORE(n, t)   S8(0xEED84u + 160u * (uint32_t)(n) + (uint32_t)(t))
#define BPM(n, t)     D8(0xF0044u + 160u * (uint32_t)(n) + (uint32_t)(t))
#define ORDER(n)      S8(0xF06C0u + (uint32_t)(n))
#define ATT(n, m)     S32(0xF52E0u + 40u * (uint32_t)(n) + 4u * (uint32_t)(m))
#define MW(n)         S8(NATION(n) + 0x26u)                       /* Metalworking: dword [199n + 0xF14EF] sar 24 */
#define ENG(n)        S8(NATION(n) + 0x2Au)                       /* Engineering: dword [199n + 0xF14F3] sar 24 */
#define HUMAN(n)      (S8(NATION(n) + 0x9Au) == 3)                /* dword [199n + 0xF1563] sar 24 */
#define LANDADJ       0xF8E7Cu
#define REAL_RES      0xBB3F8u
#define LOSS_TOK      0xF547Cu
#define F09E4         0xF09E4u
#define DROPPED       0x108190u

/* the AI level, bits 10..11 of word [0xE1624] (mov ax, [0xE1624]; shl eax, 4; shr ax, 0xE) */
static int32_t level(void)
{
    return (int32_t)((D16(0xE1624u) >> 10) & 3u);
}

/* a 16-bit word of a land list at the handle h (the lists live in heap blocks, game.h): movsx word [eax] */
static int32_t hword(uint32_t h)
{
    const uint8_t *p = load_heap_ptr(h, 2);
    return p != NULL ? (int32_t)(int16_t)(p[0] | p[1] << 8) : 0;   /* port guard: an unresolved handle reads 0 */
}

static int32_t adj_count(uint32_t list) { return hword(list); }
static int32_t adj_at(uint32_t list, int32_t i) { return hword(list + 2u * (uint32_t)i); }

/* the 69-byte area record r into / out of a local (rep movsd x 0x11, movsb) */
static void area_save(int32_t r, uint8_t *copy) { memcpy(copy, DP(AREA(r)), 69); }
static void area_restore(int32_t r, const uint8_t *copy) { memcpy(DP(AREA(r)), copy, 69); }

/* ---- calls into code that is not ported yet (see the file comment) ----------------------------------------------- */

#ifdef AIQ_TEST
int aiq_test_f4D29C(int t, int n);
#define f4D29C(t, n) aiq_test_f4D29C(t, n)
#else
/* M7 batch 1 linked (Claude, 21 Nov): the other planner modules' real routines (they were not_ported stubs
   while the five modules were translated in parallel) */
#include "aicity.h"
#include "aisea.h"
#include "aimove.h"
#include "aiplan.h"
#endif

/* ==== 0x48028 the neighbour filter ================================================================================ */

void f48028(int32_t *cnt, int32_t *list, int t, int n, int filter)
{
    uint32_t rec = NATION(n);                                       /* 0x48054 [ebp-0x14] */
    int32_t  nm1 = n - 1;                                           /* 0x4805B [ebp-0xC] */
    uint32_t adj = D32(LANDADJ + 4u * (uint32_t)t);                 /* 0x48068 [ebp-0x18] */
    int32_t  i, u, r, lim;                                          /* [ebp-0x24], [ebp-0x20], [ebp-0x1C], [ebp-0x10] */
    int      ok;

    *cnt = 0;                                                       /* 0x48073 */
    for (i = 1; adj_count(adj) >= i; i++) {                         /* 0x4809F movsx; cmp; jl 0x48762 */
        u = adj_at(adj, i);                                         /* 0x480B9 */
        r = u + 1;                                                  /* 0x480C0 */
        if ((uint32_t)filter > 0x18u)                               /* 0x480C6 cmp eax, 0x18; ja */
            continue;
        ok = 0;
        switch (filter) {                                           /* 0x48105 jmp [4*filter + 0x47FC4] */
        case 0x00:                                                  /* 0x4821F empty, no city */
            ok = CNT(r) == 0 && !CITY(u);
            break;
        case 0x01:                                                  /* 0x485A1 n alone under its limit */
            lim = POP_LIMIT(u);
            if (D8(rec + 0x27u) != 0)                               /* 0x485B8 cmp byte [eax + 0x27], 0 */
                lim++;
            ok = CNT(r) == 1 && MARK(r, n) == n && TOK(r, n) < lim; /* 0x485D8, 0x485FC, 0x48620 jge */
            break;
        case 0x02:                                                  /* 0x486EF movable tokens */
            ok = MOV(r, n) > 0;                                     /* [ebx + 0xE31DA] sar 24; jle */
            break;
        case 0x07:                                                  /* 0x482BC n alone, no city site */
            ok = CNT(r) == 1 && MARK(r, n) == n && !SITE(u);
            break;
        case 0x08:                                                  /* 0x48455 n alone */
            ok = CNT(r) == 1 && MARK(r, n) == n;
            break;
        case 0x09:                                                  /* 0x484A9 n alone, not marked */
            ok = CNT(r) == 1 && !MARKED(u) && MARK(r, n) == n;
            break;
        case 0x0A:                                                  /* 0x4871E movable, not marked */
            ok = MOV(r, n) > 0 && !MARKED(u);
            break;
        case 0x0B:                                                  /* 0x4807E all */
            ok = 1;
            break;
        case 0x0C:                                                  /* 0x4810C n has tokens */
            ok = TOK(r, n) > 0;
            break;
        case 0x0F:                                                  /* 0x48516 */
            ok = CNT(r) == 1 && (D8(TERR(u)) & 0x02u) == 0 && !MARKED(u) && MARK(r, n) == n;   /* 0x48539 [0xE1638] */
            break;
        case 0x11:                                                  /* 0x48648 n alone over its limit */
            lim = POP_LIMIT(u);
            if (D8(rec + 0x27u) != 0)                               /* 0x4865F */
                lim++;
            ok = CNT(r) == 1 && MARK(r, n) == n && TOK(r, n) > lim; /* 0x486C7 jle */
            break;
        case 0x12:                                                  /* 0x4818B flag lo 0x80 */
            ok = (D8(AIF(n, u)) & 0x80u) != 0;                      /* test byte [ecx + 0xF3580], -0x80 */
            break;
        case 0x13:                                                  /* 0x481B7 */
            ok = CNT(r) == 0 && !CITY(u)
                 && D8(0xF4234u + 160u * (uint32_t)nm1 + (uint32_t)u) != 0;   /* 0x481F3 */
            break;
        case 0x14:                                                  /* 0x4832E */
            if (SITE(u) && ((D8(AIF(n, u)) & 0x10u) != 0 || (D8(AIF(n, u) + 1u) & 0x02u) != 0))
                ok = ORDER(n) > ORDER(f7C7C4(u, n));                /* 0x48362; 0x48370 cmp dl, byte; jle */
            break;
        case 0x15:                                                  /* 0x48267 contested with n */
            ok = CNT(r) > 1 && MARK(r, n) == n;
            break;
        case 0x17:                                                  /* 0x4839B */
            if (SITE(u) && TOK(r, n) == 0 && CNT(r) == 1
                && ((D8(AIF(n, u)) & 0x10u) != 0 || (D8(AIF(n, u) + 1u) & 0x02u) != 0))
                ok = ORDER(n) > ORDER(f7C7C4(u, n));                /* 0x4841C; 0x4842A jle */
            break;
        case 0x18:                                                  /* 0x48137 one nation, n absent */
            ok = CNT(r) == 1 && TOK(r, n) == 0;
            break;
        default:                                                    /* 3..6, 0x0D, 0x0E, 0x10, 0x16 -> 0x48099 */
            break;
        }
        if (ok)
            list[(*cnt)++] = u;                                     /* list[*cnt] = u; *cnt + 1 */
    }
}

/* ==== 0x4876C the weak neighbours (filter 0x16) =================================================================== */

void f4876C(int32_t *cnt, int32_t *list, int t, int n, int filter)
{
    int32_t  nm1 = n - 1;                                           /* 0x4878B [ebp-0x8] */
    uint32_t adj = D32(LANDADJ + 4u * (uint32_t)t);                 /* 0x48799 [ebp-0xC] */
    int32_t  i, u, r, o, div, x, y;

    for (i = 1; adj_count(adj) >= i; i++) {                         /* 0x488F5 */
        u = adj_at(adj, i);                                         /* 0x4890B [ebp-0x14] */
        r = u + 1;                                                  /* 0x48912 [ebp-0x10] */
        if (filter != 0x16)                                         /* 0x48918 cmp eax, 0x16; je */
            continue;
        if (CNT(r) != 1)                                            /* 0x487BC */
            continue;
        if (MARK(r, n) == n)                                        /* 0x487E0 */
            continue;
        if ((BPM(n, u) & 0x02u) == 0)                               /* 0x487FB test byte [0xF0044 + 160n + u], 2 */
            continue;
        o = f7C7C4(u, n);                                           /* 0x4880E [ebp-0x4] */
        if (ATT(n, o) < 0) {                                        /* 0x48828 [40n + 4o + 0xF52E0]; jge 0x488D5 */
            div = (HUMAN(o) ? 1 : 0) + 1;                           /* 0x48847 sete; lea ebx, [eax + 1] */
            div *= 5 - level();                                     /* 0x4886E imul ebx, edx */
            x = ATT(o, n) - 200 / div;                              /* 0x4887B idiv; 0x4889D sub ecx, edx */
            y = (int32_t)S16(0x112A94u + 20u * (uint32_t)(25 + u % 3) + 2u * (uint32_t)nm1) / 300;   /* 0x488BB */
            if (y <= x)                                             /* 0x488D1 cmp eax, ecx; jle 0x488EF */
                continue;
        }
        list[(*cnt)++] = u;                                         /* 0x488D5 */
    }
}

/* ==== 0x4892C the second ring ===================================================================================== */

void f4892C(int start, int32_t *cnt, int32_t *out, int count, const int32_t *list, int n, int filter)
{
    int32_t  i, j, k, t, u, found;                                  /* [ebp-0x28] i, [ebp-0x2C] j, [ebp-0x34] t, [ebp-0x1C] u */
    uint32_t adj;                                                   /* [ebp-0xC] */

    *cnt = 0;                                                       /* 0x48954 */
    if (S32(F09E4) == 0)                                            /* 0x4895A cmp [0xF09E4], 0; je */
        return;
    for (i = 0; i < count; i++) {                                   /* 0x48974 */
        t = list[i];                                                /* 0x4898B */
        if (!(CITY(t) && (int32_t)D8(TSTATE(t)) == n)) {            /* 0x489A7 test 1; 0x489C3 owner == n -> 0x48A36 */
            if (!(CNT(t + 1) == 0 && !CITY(t))) {                   /* 0x489DE; 0x489F3 je 0x48A36 */
                if (CNT(t + 1) != 1 || MARK(t + 1, n) != n)         /* 0x48A0C, 0x48A30 jne 0x4896E */
                    continue;
            }
        }
        adj = D32(LANDADJ + 4u * (uint32_t)t);                      /* 0x48A39 */
        for (j = 1; adj_count(adj) >= j; j++) {                     /* 0x48B09 */
            u = adj_at(adj, j);                                     /* 0x48B23 */
            if (u == start) {                                       /* 0x48B2E je 0x48A71 */
                found = 1;
            } else {
                found = 0;                                          /* 0x48B36 */
                for (k = 0; k < count; k++) {                       /* 0x48A55 [ebp-0x20] */
                    if (u == list[k]) {                             /* 0x48A6D */
                        found = 1;                                  /* 0x48A71 */
                        break;
                    }
                }
            }
            if (found == 0) {                                       /* 0x48A7D */
                for (k = 0; k < *cnt; k++) {                        /* 0x48A8A [ebp-0x24] */
                    if (out[k] == u) {                              /* 0x48AA2 */
                        found = 1;                                  /* 0x48AA6 */
                        break;
                    }
                }
            }
            if (found != 0)                                         /* 0x48AAD */
                continue;
            switch (filter) {                                       /* 0x48CEB jb / jbe chain on the unsigned filter */
            case 0x00:                                              /* 0x48ABB */
                if (CNT(u + 1) == 0 && !CITY(u))
                    out[(*cnt)++] = u;
                break;
            case 0x09:                                              /* 0x48C6B */
                if (CNT(u + 1) == 1 && !MARKED(u) && MARK(u + 1, n) == n)
                    out[(*cnt)++] = u;
                break;
            case 0x0B:                                              /* 0x48CCC */
                out[(*cnt)++] = u;
                break;
            case 0x14:                                              /* 0x48B41 */
                if (SITE(u) && ((D8(AIF(n, u)) & 0x10u) != 0 || (D8(AIF(n, u) + 1u) & 0x02u) != 0)
                    && ORDER(n) > ORDER(f7C7C4(u, n)))              /* 0x48B78; 0x48B86 jle */
                    out[(*cnt)++] = u;
                break;
            case 0x17:                                              /* 0x48BB1 */
                if (SITE(u) && TOK(u + 1, n) == 0 && CNT(u + 1) == 1
                    && ((D8(AIF(n, u)) & 0x10u) != 0 || (D8(AIF(n, u) + 1u) & 0x02u) != 0)
                    && ORDER(n) > ORDER(f7C7C4(u, n)))              /* 0x48C32; 0x48C40 jle */
                    out[(*cnt)++] = u;
                break;
            default:
                break;
            }
        }
    }
}

/* ==== 0x48D44 sort / filter a territory list ====================================================================== */

/* one stable partition: the entries of the copy that pass first, then (when both) the others; *cnt rebuilt */
static void d44_split(int32_t *cnt, int32_t *list, const int32_t *copy, int32_t c, const uint8_t *pass, int both)
{
    int32_t i;
    *cnt = 0;
    for (i = 0; i < c; i++)
        if (pass[i])
            list[(*cnt)++] = copy[i];
    if (both)
        for (i = 0; i < c; i++)
            if (!pass[i])
                list[(*cnt)++] = copy[i];
}

void f48D44(int32_t *cnt, int32_t *list, int n, int t, int flags)
{
    int32_t copy[160];      /* [ebp-0x2A8]: 157 ints in the original (see the file comment) */
    uint8_t pass[160];
    int32_t c, i, j, tmp, st;

    if (*cnt <= 0)                                                  /* 0x48D68 jle 0x495B4 */
        return;
    if ((flags & 0x80) != 0) {                                      /* 0x48D7B test bl, -0x80 city sites first */
        c = *cnt;                                                   /* 0x48DAA [ebp-0x10] */
        memcpy(copy, list, 4u * (uint32_t)c);                       /* 0x48D9B repne movsd */
        for (i = 0; i < c; i++)
            pass[i] = SITE(copy[i]);                                /* 0x48DE9 test dl, 0x40 */
        d44_split(cnt, list, copy, c, pass, 1);                     /* 0x48DC3, 0x48E1E */
    }
    /* 0x48E6C the bubble sort, descending by the signed score */
    for (i = *cnt - 2; i >= 0; i--) {                               /* [ebp-0x28] */
        for (j = 0; j <= i; j++) {                                  /* [ebp-0x24] */
            if (SCORE(n, list[j + 1]) > SCORE(n, list[j])) {        /* 0x48EF6 cmp al, bl; jle */
                tmp = list[j + 1];                                  /* 0x48F06 */
                list[j + 1] = list[j];
                list[j] = tmp;
            }
        }
    }
    if ((flags & 0x10) != 0) {                                      /* 0x48F3C */
        c = *cnt;                                                   /* 0x48F45 [ebp-0x1C] */
        *cnt = 0;
        for (i = 0; i < c; i++)
            if (SCORE(n, list[i]) >= 0)                             /* 0x48F8D jl */
                (*cnt)++;                                           /* 0x48F94 inc dword [eax]: the count only */
    }
    if ((flags & 0x20) != 0) {                                      /* 0x48F9B */
        st = SCORE(n, t);                                           /* 0x48FBE [ebp-0x18] (unused here) */
        (void)st;
        c = *cnt;
        memcpy(copy, list, 4u * (uint32_t)c);                       /* 0x48FD8 */
        *cnt = 0;
        for (i = 0; i < c; i++) {                                   /* 0x49007 */
            if (D8(0xF4234u + 160u * (uint32_t)(n - 1) + (uint32_t)copy[i]) != 0)   /* 0x49028 */
                setD8(DROPPED, 1);                                  /* 0x48FFA */
            else
                list[(*cnt)++] = copy[i];                           /* 0x49032 */
        }
    }
    if ((flags & 0x200) != 0) {                                     /* 0x49058 test dh, 2: drop the cities */
        st = SCORE(n, t);                                           /* 0x4907C */
        (void)st;
        c = *cnt;
        memcpy(copy, list, 4u * (uint32_t)c);
        for (i = 0; i < c; i++)
            pass[i] = !CITY(copy[i]);                               /* 0x490DF test ch, 1; jne */
        d44_split(cnt, list, copy, c, pass, 0);
    }
    if ((flags & 0x02) != 0) {                                      /* 0x4910B scores <= score(t) first */
        st = SCORE(n, t);                                           /* 0x4912E */
        c = *cnt;
        memcpy(copy, list, 4u * (uint32_t)c);
        for (i = 0; i < c; i++)
            pass[i] = SCORE(n, copy[i]) <= st;                      /* 0x4919D cmp; jg; 0x491FE jle */
        d44_split(cnt, list, copy, c, pass, 1);
    }
    if ((flags & 0x04) != 0) {                                      /* 0x49228 test di, 4: .BPM 0x02 first */
        c = *cnt;
        memcpy(copy, list, 4u * (uint32_t)c);
        for (i = 0; i < c; i++)
            pass[i] = (BPM(n, copy[i]) & 0x02u) != 0;               /* 0x49299, 0x492F5 */
        d44_split(cnt, list, copy, c, pass, 1);
    }
    if ((flags & 0x400) != 0) {                                     /* 0x49321 keep scores > 105 */
        st = SCORE(n, t);                                           /* 0x49347 (unused) */
        (void)st;
        c = *cnt;
        memcpy(copy, list, 4u * (uint32_t)c);
        for (i = 0; i < c; i++)
            pass[i] = SCORE(n, copy[i]) > 0x69;                     /* 0x493B3 cmp eax, 0x69; jle */
        d44_split(cnt, list, copy, c, pass, 0);
    }
    if ((flags & 0x100) != 0) {                                     /* 0x493DE test dh, 1: cities first */
        c = *cnt;
        memcpy(copy, list, 4u * (uint32_t)c);
        for (i = 0; i < c; i++)
            pass[i] = CITY(copy[i]);                                /* 0x49447, 0x4949D */
        d44_split(cnt, list, copy, c, pass, 1);
    }
    if ((flags & 0x40) != 0) {                                      /* 0x494C8 flag hi 0x04 first */
        c = *cnt;
        memcpy(copy, list, 4u * (uint32_t)c);
        for (i = 0; i < c; i++)
            pass[i] = (D8(AIF(n, copy[i]) + 1u) & 0x04u) != 0;      /* 0x4952B [0xF3581 + 314(n-1) + 2u] */
        d44_split(cnt, list, copy, c, pass, 1);
    }
}

/* ==== 0x47BA0 n's territories by mode ============================================================================= */

void f47BA0(int32_t *cnt, int32_t *list, int n, int mode, int flags)
{
    int32_t copy[160];      /* [ebp-0x2A8] */
    int32_t t, r, i, j, c, tmp;

    *cnt = 0;                                                       /* 0x47BC4 */
    for (t = 0; t < 0x9D; t++) {                                    /* 0x47C80 cmp 0x9D; jge */
        if (D8(0xEE16Cu + (uint32_t)t) == 0)                        /* 0x47C96 */
            continue;
        r = t + 1;                                                  /* 0x47C9E [ebp-0x1C] */
        if ((uint32_t)mode == 3u) {                                 /* 0x47CA4 jb / jbe 0x47BDB */
            if (MARKED(t))                                          /* 0x47BE7 */
                continue;
            if (MOV(r, n) <= 0)                                     /* 0x47C0F jle */
                continue;
            if (CNT(r) != 1) {                                      /* 0x47C2A je 0x47C60 */
                if (CNT(r) <= 1)                                    /* 0x47C43 jle */
                    continue;
                if ((BPM(n, t) & 0x02u) == 0)                       /* 0x47C57 */
                    continue;
            }
            list[(*cnt)++] = t;                                     /* 0x47C60 */
        } else if ((uint32_t)mode == 0x0Eu) {                       /* 0x47CAF cmp eax, 0xE; jne */
            if (MARKED(t) || !SITE(t))                              /* 0x47CC0, 0x47CDA */
                continue;
            if ((D8(TERR(t) + 2u) & 0x01u) == 0)                    /* 0x47CF4 test byte [0xE163A], 1 */
                continue;
            if (CNT(r) != 1 || MARK(r, n) != n)                     /* 0x47D15, 0x47D39 */
                continue;
            if (TOK(r, n) <= POP_LIMIT(t) + S8(NATION(n) + 0x27u))  /* 0x47D6A [0xF14F0] sar 24; 0x47D91 jle */
                continue;
            list[(*cnt)++] = t;                                     /* 0x47D99 */
        }
    }
    if (flags == 0 || *cnt <= 0)                                    /* 0x47DB8, 0x47DC5 */
        return;
    if ((flags & 0x01) != 0) {                                      /* 0x47DD1 bubble sort, ascending */
        for (i = 1; i < *cnt; i++) {                                /* [ebp-0x28] */
            for (j = *cnt - 1; j >= i; j--) {                       /* [ebp-0x24] */
                if (SCORE(n, list[j - 1]) > SCORE(n, list[j])) {    /* 0x47E57 cmp al, ah; jle */
                    tmp = list[j - 1];                              /* 0x47E64 */
                    list[j - 1] = list[j];
                    list[j] = tmp;
                }
            }
        }
    }
    if ((flags & 0x08) != 0) {                                      /* 0x47E99 over the limit first */
        c = *cnt;                                                   /* 0x47EC7 [ebp-0x10] */
        memcpy(copy, list, 4u * (uint32_t)c);                       /* 0x47EB8 */
        *cnt = 0;
        for (i = 0; i < c; i++)                                     /* 0x47EE0 */
            if (TOK(copy[i] + 1, n) > f78D08(copy[i], n))           /* 0x47F18; 0x47F1D cmp ecx, eax; jle */
                list[(*cnt)++] = copy[i];
        for (i = 0; i < c; i++)                                     /* 0x47F51 */
            if (!(TOK(copy[i] + 1, n) > f78D08(copy[i], n)))        /* 0x47F8B; jg */
                list[(*cnt)++] = copy[i];
    }
}

/* ==== 0x495C0 the strongest enemy next to t ======================================================================= */

int f495C0(int t, int n, int mode, int32_t *enemy)
{
    uint8_t  copy[69];                                              /* [ebp-0x80] */
    int32_t  my = ORDER(n);                                         /* 0x495F3 [ebp-0x14] */
    int32_t  nm1 = n - 1;                                           /* 0x495FA [ebp-0x20] */
    uint32_t adj = D32(LANDADJ + 4u * (uint32_t)t);                 /* 0x49607 [ebp-0x10] */
    int32_t  best = 0;                                              /* 0x4960C [ebp-0x18] */
    int32_t  i, r, j, saved, e;

    *enemy = 0;                                                     /* 0x495E1 */
    for (i = 1; adj_count(adj) >= i; i++) {                         /* 0x496E0 [ebp-0x2C] */
        r = adj_at(adj, i) + 1;                                     /* 0x496FB [ebp-0x24] */
        if (CNT(r) == 0)                                            /* 0x49714 */
            continue;
        saved = 0;                                                  /* 0x49718 [ebp-0x1C] */
        if (CNT(r) > 1) {                                           /* 0x49732 jle 0x49779 */
            saved = 1;
            area_save(r, copy);                                     /* 0x49754 */
            f7A778();                                               /* 0x49757 */
            setD8(REAL_RES, 0);                                     /* 0x4975E */
            f5B588(r);                                              /* 0x49769 */
            setD8(REAL_RES, 1);                                     /* 0x4976E mov [0xBB3F8], dl (dl = 1) */
            f7A7B0();                                               /* 0x49774 */
        }
        for (j = 0; j < 9; j++) {                                   /* 0x49675 cmp [ebp-0x28], 9; jge */
            if (j == nm1)                                           /* 0x49621 / 0x49689 */
                continue;
            if (mode == 1 && ORDER(j + 1) <= my)                    /* 0x4967E; 0x49690 [j + 0xF06BE] sar 24; jle */
                continue;
            e = TOK(r, j + 1);                                      /* 0x49628 / 0x496B0 [69r + j + 0xE31C4] */
            if (e > best) {                                         /* jle 0x49660 */
                best = TOK(r, j + 1);                               /* 0x49654 */
                *enemy = j + 1;                                     /* 0x4965E */
            }
        }
        if (saved != 0)                                             /* 0x496C3 */
            area_restore(r, copy);                                  /* 0x496D7 */
    }
    return best;                                                    /* 0x49783 */
}

/* ==== 0x49790 defenders to leave ================================================================================== */

int f49790(int r, int t, int n, int enemy)
{
    int32_t res = 0;                                                /* 0x497B5 [ebp-0xC] */
    int32_t lim, thr, x, tok;

    if (r == 0)                                                     /* 0x497B8 */
        return res;
    lim = POP_LIMIT(t) - 1;                                         /* 0x497E4 [ebp-0x8] */
    if (lim == 0)                                                   /* 0x497EC */
        lim = 1;
    thr = lim + 1 + (MW(n) - MW(enemy));                            /* 0x4981D [ebp-0x4] */
    if (r < thr)                                                    /* 0x49826 jl 0x49926 */
        return 1;
    x = (int32_t)S16(0x112A94u + 20u * (uint32_t)(12 + t % 8) + 2u * (uint32_t)(n - 1)) / 300;   /* 0x4984E, 0x49862 */
    if (x < 0x5F - 10 * level()) {                                  /* 0x4988A cmp edx, ebx; jge 0x4991E */
        tok = TOK(t + 1, n);                                        /* 0x498A5 */
        res = tok <= POP_LIMIT(t) ? TOK(t + 1, n) : POP_LIMIT(t);   /* 0x498D2 jg 0x498F2 */
        return res;                                                 /* 0x49917 */
    }
    if (res == 1)                                                   /* 0x4991E (res is 0 here) */
        return res;
    return 1;                                                       /* 0x49926 */
}

/* ==== 0x49934 can n reach t ======================================================================================= */

int f49934(int t, int n, int mode)
{
    uint32_t adj = D32(LANDADJ + 4u * (uint32_t)t);                 /* 0x4995F [ebp-0xC] */
    uint32_t rec;
    int32_t  i, k;

    for (i = 1; adj_count(adj) >= i; i++)                           /* 0x49971 [ebp-0x14] */
        if (TOK(adj_at(adj, i) + 1, n) != 0)                        /* 0x49999 [69(u+1) + n - 1 + 0xE31C4] sar 24 */
            return 1;
    if (mode == 1)                                                  /* 0x499AD je 0x49A11 */
        return 0;
    rec = NATION(n);                                                /* 0x499C1 [ebp-0x8] */
    for (k = 0; (int32_t)D8(rec + 0x40u) > k; k++) {                /* 0x499D1 ships; jle 0x49A11 */
        if (f5843C((int32_t)S16(rec + 0x45u + 2u * (uint32_t)k) - 1, t, n, D8(rec + 0x41u + (uint32_t)k)) == 1)   /* 0x49A05 */
            return 1;
    }
    return 0;                                                       /* 0x49A11 */
}

/* ==== 0x4CC5C tokens n needs to take t ============================================================================ */

/* need = m's tokens + extra + base - n's tokens + Metalworking(m) - Metalworking(n) (0x4CD94 / 0x4CE60) */
int f4CC5C(int t, int n, int mode)
{
    int32_t list[160];                                              /* [ebp-0x2C0] */
    int32_t cnt;                                                    /* [ebp-0x10] */
    int32_t r = t + 1;                                              /* 0x4CC85 [ebp-0x30] */
    int32_t my = ORDER(n);                                          /* 0x4CC9B [ebp-0x18] */
    int32_t e = 0;                                                  /* 0x4CCA0 [ebp-0x2C] */
    int32_t m = 0, big = 0, base, s, k;                             /* [ebp-0x24], [ebp-0x28] (set when e > 0) */

    for (k = 0; k <= 0x0B; k++) {                                   /* 0x4CCAE cmp 0xB; jg */
        if (n - 1 == k)                                             /* 0x4CCBB */
            continue;
        if (TOK(r, k + 1) > e) {                                    /* 0x4CCD1 [69r + k + 0xE31C4]; jle */
            e = TOK(r, k + 1);                                      /* 0x4CCFA */
            m = k + 1;                                              /* 0x4CD01 */
            big = (m > 9) + 1;                                      /* 0x4CD0A setg; inc */
        }
    }
    if (e == 0)                                                     /* 0x4CD1C */
        return CNT(r) == 0 ? 1 : -1;                                /* 0x4CD34: 0x4CD36 / 0x4CD40 */
    base = POP_LIMIT(t) - 1;                                        /* 0x4CD6D [ebp-0x20] */
    if (base == 0)
        base = 1;
    if (ORDER(m) < my || big == 2)                                  /* 0x4CD89 jl 0x4CD94; 0x4CD8E */
        goto formula;
    if (mode == 3)                                                  /* 0x4CE00 je 0x4CED8 */
        return -1;
    if (f7CAB4(n) == 0)                                             /* 0x4CE0C; je 0x4CED1 */
        return -1;
    f48028(&cnt, list, t, m, 0x0C);                                 /* 0x4CE2A */
    if (cnt == 0)                                                   /* 0x4CE34 je 0x4CD94 */
        goto formula;
    s = f4D110(t, &cnt, list, m, 0);                                /* 0x4CE4B */
    if (s > 2)                                                      /* 0x4CE53 */
        s = 2;
    return TOK(r, m) + s + base - TOK(r, n) + (MW(m) - MW(n));      /* 0x4CE60..0x4CEC7 */
formula:
    return base + TOK(r, m) - TOK(r, n) + (MW(m) - MW(n));          /* 0x4CD94..0x4CDF5 [69r + m + 0xE31C3] */
}

/* ==== 0x4CEE4 tokens n needs to take the city at t ================================================================ */

int f4CEE4(int t, int n, int32_t *out)
{
    int32_t list[160];                                              /* [ebp-0x2B8] */
    int32_t cnt;                                                    /* [ebp-0x10] */
    int32_t my = ORDER(n);                                          /* 0x4CF15 [ebp-0x18] */
    int32_t m = D8(TSTATE(t));                                      /* 0x4CF28 [ebp-0x2C] the city's owner */
    int32_t big = (m > 9) + 1;                                      /* 0x4CF3E [ebp-0x14] */
    int32_t need, sea, s, extra, bonus;

    if (ORDER(m) < my || big == 2) {                                /* 0x4CF4D jl; 0x4CF52 */
        need = TOK(t + 1, m) + 7 + (ENG(m) - ENG(n));               /* 0x4CF58..0x4CF9D */
        *out = need;                                                /* 0x4CFA6 */
        return need;
    }
    f48028(&cnt, list, t, m, 0x0C);                                 /* 0x4CFBE */
    sea = f4D29C(t, m);                                             /* 0x4CFC9 [ebp-0x1C] */
    if (cnt + sea == 0) {                                           /* 0x4CFDE jne 0x4D035 */
        need = TOK(t + 1, m) + 7 + (ENG(m) - ENG(n));               /* 0x4CFE2..0x4D020 */
        *out = need;                                                /* 0x4D029 */
        return need;
    }
    *out = (ENG(m) - ENG(n)) + 7;                                   /* 0x4D035..0x4D058 */
    s = f4D110(t, &cnt, list, m, 0);                                /* 0x4D06B [ebp-0x20] */
    if (s + sea == 0) {                                             /* 0x4D07B */
        extra = 0;                                                  /* 0x4D07D [ebp-0x24] */
    } else {
        bonus = (D8(NATION(m) + 0x26u) != 0 && D8(NATION(n) + 0x26u) == 0) ? 1 : 0;   /* 0x4D089, 0x4D099 */
        extra = bonus + (s + sea - 1);                              /* 0x4D0B8 */
    }
    return TOK(t + 1, m) + 7 + extra + (ENG(m) - ENG(n));           /* 0x4D0BD..0x4D103 */
}

/* ==== 0x4D110 land reinforcements of m near t ===================================================================== */

int f4D110(int t, int32_t *cnt, const int32_t *list, int m, int k)
{
    int32_t list2[160];                                             /* [ebp-0x2AC] */
    int32_t cnt2;                                                   /* [ebp-0xC] */
    int32_t sum = 0;                                                /* 0x4D133 [ebp-0x18] */
    int32_t i, r, s;

    f4892C(t, &cnt2, list2, *cnt, list, m, 0x0B);                   /* 0x4D151 */
    for (i = 0; i < *cnt; i++) {                                    /* 0x4D163 cmp eax, [edx] */
        r = list[i] + 1;                                            /* 0x4D17D [ebp-0x1C] */
        if (CNT(r) == 0)                                            /* 0x4D194 */
            continue;
        s = TOK(r, m) - k;                                          /* 0x4D1B6 [ebp-0x14] */
        if (s <= 0)                                                 /* 0x4D1C0 jle */
            continue;
        if (SITE(r - 1) && s > 4)                                   /* 0x4D1DA; 0x4D1E3 jg */
            continue;
        sum += s;                                                   /* 0x4D1EC */
    }
    for (i = 0; i < cnt2; i++) {                                    /* 0x4D201 */
        r = list2[i] + 1;                                           /* 0x4D218 */
        if (CNT(r) == 0)                                            /* 0x4D231 */
            continue;
        s = TOK(r, m) - k;                                          /* 0x4D253 */
        if (s <= 0)                                                 /* 0x4D25B */
            continue;
        if (SITE(r - 1) && s > 4)                                   /* 0x4D275; 0x4D27E */
            continue;
        sum += s;                                                   /* 0x4D287 */
    }
    return sum;                                                     /* 0x4D28F */
}

/* ==== 0x7BD5C the map analysis ==================================================================================== */

/* flag bytes of n for territory t (0xF3580 + 314(n-1) + 2t, the high byte at +1) */
static void flag_lo(int n, int t, uint8_t bit) { setD8(AIF(n, t), (uint8_t)(D8(AIF(n, t)) | bit)); }
static void flag_hi(int n, int t, uint8_t bit) { setD8(AIF(n, t) + 1u, (uint8_t)(D8(AIF(n, t) + 1u) | bit)); }
static int  enemy_flag(int n, int u) { return (D8(AIF(n, u)) & 0x10u) != 0 || (D8(AIF(n, u) + 1u) & 0x02u) != 0; }

void f7BD5C(int n)
{
    int32_t  near1[160];                                            /* [ebp-0x304] the land neighbours */
    int32_t  ring[160];                                             /* [ebp-0x584] f4892C's second ring */
    int32_t  nm1 = n - 1;                                           /* 0x7BD7B [ebp-0x6C] */
    int32_t  road = S8(NATION(n) + 0x28u);                          /* 0x7BD8E [ebp-0x70] dword [199n + 0xF14F1] sar 24 */
    int32_t  r, t, i, u, u1, k, cnt2, nn;                           /* [ebp-0x64] r, [ebp-0x38] t, [ebp-0x84] i */
    int32_t  occ, rings, own1, foreign1, own2, foreign2;            /* [ebp-0x78], [ebp-0x74], [ebp-0x48], [ebp-0x44], [ebp-0x40], [ebp-0x3C] */
    int32_t  best = 0, bestm = 0, lim, thr;                         /* [ebp-0x24], [ebp-0x1C], [ebp-0x20], [ebp-0x28] */
    int32_t  a58, a54, a50, a4c;                                    /* [ebp-0x58], [ebp-0x54], [ebp-0x50], [ebp-0x4C] */
    int32_t  o, s;                                                  /* [ebp-0x60], [ebp-0x5C] */
    uint32_t adj;                                                   /* [ebp-0x18] */

    memset(DP(0xF3580u + 314u * (uint32_t)nm1), 0, 314);            /* 0x7BDA9 */
    for (r = 1; r <= 0x9D; r++) {                                   /* 0x7BDAE [ebp-0x64] = ecx = 1; 0x7C6C0 jg */
        t = r - 1;                                                  /* 0x7C6D1 [ebp-0x38] */
        if (CNT(r) != 0)                                            /* 0x7C6EA jne 0x7C0C8 */
            goto occupied;

        /* ---- an empty territory (0x7C6F0) ---- */
        if (CITY(t))                                                /* 0x7C702 */
            (void)(D8(TSTATE(t)) == (uint32_t)n);                   /* 0x7C71E cmp eax, [ebp-0x30]: no branch (D) */
        occ = 0;                                                    /* 0x7C723 */
        rings = 0;                                                  /* 0x7C726 */
        adj = D32(LANDADJ + 4u * (uint32_t)t);                      /* 0x7C733 */
        own1 = foreign1 = own2 = foreign2 = 0;                      /* 0x7C736..0x7C73F */
        nn = 0;                                                     /* 0x7C742 [ebp-0x34] */
        for (i = 1; adj_count(adj) >= i; i++) {                     /* 0x7BDC2 */
            u1 = adj_at(adj, i) + 1;                                /* 0x7BDE3 [ebp-0x7C] */
            near1[nn++] = u1 - 1;                                   /* 0x7BDF7 */
            if (CNT(u1) == 0)                                       /* 0x7BE14 */
                continue;
            occ++;                                                  /* 0x7BE19 */
            if (MARK(u1, n) == n && CNT(u1) == 1) {                 /* 0x7BE3C, 0x7BE55 */
                own1 = 1;                                           /* 0x7BE57 [ebp-0x48] = eax = 1 */
                continue;
            }
            foreign1 = 1;                                           /* 0x7BE5F */
            best = 0;                                               /* 0x7BE68 */
            bestm = 11;                                             /* 0x7BE6B */
            for (k = 0; k < 9; k++) {                               /* 0x7BE7D cmp 9; jge */
                if (k == nm1)
                    continue;
                if (TOK(u1, k + 1) > best) {                        /* 0x7BEAC jle */
                    best = TOK(u1, k + 1);                          /* 0x7BECC */
                    bestm = k + 1;                                  /* 0x7BED3 */
                }
            }
        }
        if (road != 0) {                                            /* 0x7BED8 */
            f4892C(t, &cnt2, ring, nn, near1, n, 0x0B);             /* 0x7BEFE */
            for (i = 0; i < cnt2; i++) {                            /* 0x7BF20 */
                u1 = ring[i] + 1;                                   /* 0x7BF39 */
                rings++;                                            /* 0x7BF3F */
                if (MARK(u1, n) == n && CNT(u1) == 1)               /* 0x7BF62, 0x7BF7B */
                    own2 = 1;                                       /* 0x7BF7D */
                else
                    foreign2 = 1;                                   /* 0x7BF0D */
            }
        }
        if (occ == 0 || (foreign1 == 1 && own1 == 0)) {            /* 0x7BF82..0x7BF92 */
            flag_lo(n, t, 0x01);                                    /* 0x7BF9E */
        } else if (foreign1 == 0 && own1 == 1) {                    /* 0x7BFAB */
            flag_lo(n, t, 0x04);                                    /* 0x7BFC2 */
        } else if (foreign1 == 1 && own1 == foreign1) {             /* 0x7BFCF, 0x7BFDB */
            if (bestm > 9) {                                        /* 0x7BFEA jg 0x7C067 */
                flag_lo(n, t, 0x04);                                /* 0x7C071 */
            } else {
                lim = POP_LIMIT(t) - 1;                             /* 0x7C012 sub eax, ecx (ecx = 1) */
                if (lim == 0)
                    lim = 1;                                        /* 0x7C01D mov [ebp-0x20], ecx */
                thr = lim + 1 + (MW(n) - MW(bestm));                /* 0x7C048 [ebp-0x28] */
                if (thr <= best)                                    /* 0x7C04E jg 0x7C079 */
                    flag_hi(n, t, 0x04);                            /* 0x7C05D */
            }
        }
        if (rings == 0 || foreign2 == 1)                            /* 0x7C079, 0x7C07F */
            flag_lo(n, t, 0x02);                                    /* 0x7C08F */
        else if (foreign2 == 0 && own2 == 1)                        /* 0x7C09C, 0x7C0A7 */
            flag_lo(n, t, 0x08);                                    /* 0x7C0BB */
        goto common;

    occupied:                                                       /* 0x7C0C8 */
        if (CNT(r) != 1) {                                          /* 0x7C0DF jne 0x7C4C6 */
            if (MARK(r, n) == n)                                    /* 0x7C4F2 */
                flag_hi(n, t, 0x02);                                /* 0x7C4F6 */
            else
                flag_lo(n, t, 0x10);                                /* 0x7C4FF */
            goto common;
        }
        if (f7C760(t) != n) {                                       /* 0x7C0E8 [ebp-0x60]; 0x7C0F8 */
            flag_lo(n, t, 0x10);                                    /* 0x7C104 */
            goto common;
        }
        occ = 0;                                                    /* 0x7C113 */
        rings = 0;                                                  /* 0x7C118 xor eax, ecx (both n) */
        adj = D32(LANDADJ + 4u * (uint32_t)t);                      /* 0x7C125 */
        a58 = a54 = a50 = a4c = 0;                                  /* 0x7C128..0x7C131 */
        nn = 0;                                                     /* 0x7C134 */
        if (adj_count(adj) == 0) {                                  /* 0x7C13D */
            a58 = 1;                                                /* 0x7C141 */
        } else {
            for (i = 1; adj_count(adj) >= i; i++) {                 /* 0x7C16C */
                u = adj_at(adj, i);                                 /* 0x7C18C [ebp-0x80] */
                u1 = u + 1;                                         /* 0x7C193 [ebp-0x7C] */
                near1[nn++] = u;                                    /* 0x7C19F */
                if (CNT(u1) == 0 && !CITY(u))                       /* 0x7C1BC, 0x7C1D1 je 0x7C160 */
                    continue;
                occ++;                                              /* 0x7C1D3 */
                if (CNT(u1) == 0 && !CITY(u)) {                     /* 0x7C1EF, 0x7C204 (never true here) */
                    a58 = 1;
                    continue;
                }
                if (CITY(u)) {                                      /* 0x7C21D jne 0x7C2B5 */
                    a58 = 1;
                    continue;
                }
                if (CNT(u1) == 2 && MARK(u1, n) == n && TOK(u1, 11) > 0) {   /* 0x7C23A, 0x7C25A, 0x7C270 [0xE31CE] */
                    a58 = 1;
                    continue;
                }
                if (CNT(u1) == 1 && MARK(u1, n) == n)               /* 0x7C28B, 0x7C2AF */
                    a58 = 1;                                        /* 0x7C2B5 */
                else
                    a54 = 1;                                        /* 0x7C159 */
            }
        }
        if (road != 0) {                                            /* 0x7C2C1 */
            f4892C(t, &cnt2, ring, nn, near1, n, 0x0B);             /* 0x7C2E7 */
            for (i = 0; i < cnt2; i++) {                            /* 0x7C309 */
                u = ring[i];                                        /* 0x7C325 */
                u1 = u + 1;
                rings++;                                            /* 0x7C332 */
                if ((CNT(u1) == 0 && !CITY(u)) || CITY(u)           /* 0x7C34B..0x7C379 */
                    || (CNT(u1) == 2 && MARK(u1, n) == n && TOK(u1, 11) > 0)   /* 0x7C396..0x7C3CE */
                    || (CNT(u1) == 1 && MARK(u1, n) == n))          /* 0x7C3E7, 0x7C40B */
                    a50 = 1;                                        /* 0x7C411 */
                else
                    a4c = 1;                                        /* 0x7C2F6 */
            }
        }
        if (a54 == 1)                                               /* 0x7C42B */
            flag_lo(n, t, 0x20);                                    /* 0x7C431 */
        else if (a54 == 0 && a58 == 1)                              /* 0x7C43A..0x7C445 */
            flag_lo(n, t, 0x80);                                    /* 0x7C447 */
        else if (a54 == 0 && a58 == 0)                              /* 0x7C450..0x7C45B */
            flag_lo(n, t, 0x80);                                    /* 0x7C467 */
        if (a4c == 1)                                               /* 0x7C47D */
            flag_lo(n, t, 0x40);                                    /* 0x7C483 */
        else if (a4c == 0 && a50 == 1)                              /* 0x7C48F..0x7C49A */
            flag_hi(n, t, 0x01);                                    /* 0x7C49C */
        else if (a4c == 0 && a50 == 0)                              /* 0x7C4A5..0x7C4B0 */
            flag_hi(n, t, 0x01);                                    /* 0x7C4BC */

    common:                                                         /* 0x7C506 */
        if (!enemy_flag(n, t))                                      /* 0x7C510, 0x7C524 je 0x7C6BA */
            continue;
        o = f7C7C4(t, n);                                           /* 0x7C538 [ebp-0x60] */
        adj = D32(LANDADJ + 4u * (uint32_t)t);                      /* 0x7C543 */
        nn = 0;                                                     /* 0x7C54F */
        for (i = 1; adj_count(adj) >= i; i++) {                     /* 0x7C56A */
            u = adj_at(adj, i);                                     /* 0x7C586 [ebp-0x80] */
            if (enemy_flag(n, u) && f7C7C4(u, n) == o)              /* 0x7C5A1, 0x7C5B0; 0x7C5C0 */
                near1[nn++] = u;                                    /* 0x7C5D5 */
        }
        if (nn == 0) {                                              /* 0x7C5DE je 0x7C6A8 */
            flag_hi(n, t, 0x08);                                    /* 0x7C6B2 */
            continue;
        }
        if (nn != 1)                                                /* 0x7C5EE jne 0x7C6BA */
            continue;
        s = near1[0];                                               /* 0x7C5FA [ebp-0x5C] */
        adj = D32(LANDADJ + 4u * (uint32_t)s);                      /* 0x7C600 */
        nn = 0;                                                     /* 0x7C60C */
        for (i = 1; adj_count(adj) >= i; i++) {                     /* 0x7C60F (edi = 1); 0x7C623 */
            u = adj_at(adj, i);                                     /* 0x7C63F */
            if (enemy_flag(n, u) && f7C7C4(u, n) == o)              /* 0x7C65A, 0x7C669; 0x7C679 */
                near1[nn++] = u;                                    /* 0x7C68E */
        }
        if (nn == 1 && near1[0] == s)                               /* 0x7C697, 0x7C6A3 (see the file comment) */
            flag_hi(n, t, 0x08);                                    /* 0x7C6B2 */
    }
}

/* ==== 0x7C838 the conflict trial ================================================================================== */

int f7C838(int t, int n, int k, int32_t *left)
{
    uint8_t copy[69];                                               /* [ebp-0x64] */
    int32_t r = t + 1;                                              /* 0x7C85A [ebp-0xC] */

    area_save(r, copy);                                             /* 0x7C876 */
    f7A778();                                                       /* 0x7C879 */
    setD8(AREA(r) + 0x0Eu + (uint32_t)n, (uint8_t)(D8(AREA(r) + 0x0Eu + (uint32_t)n) + (uint8_t)k));   /* 0x7C895 add byte */
    if (MARK(r, n) != n) {                                          /* 0x7C8B6 */
        setD8(AREA(r) + 1u, (uint8_t)(D8(AREA(r) + 1u) + 1u));      /* 0x7C8C5 inc byte [0xE31B9 + 69r] */
        setD8(AREA(r) + 1u + (uint32_t)n, (uint8_t)n);              /* 0x7C8E3 */
    }
    memset(DP(LOSS_TOK), 0, 10);                                    /* 0x7C8F5 */
    setD8(REAL_RES, 0);                                             /* 0x7C8FC */
    f5B588(r);                                                      /* 0x7C907 */
    setD8(REAL_RES, 1);                                             /* 0x7C90C mov [0xBB3F8], cl (cl = 1) */
    *left = CNT(r);                                                 /* 0x7C929 */
    f7A7B0();                                                       /* 0x7C92B */
    area_restore(r, copy);                                          /* 0x7C949 */
    return S8(LOSS_TOK + (uint32_t)n);                              /* 0x7C94F dword [n + 0xF5479] sar 24 */
}

/* ==== 0x7B044 the position score ================================================================================== */

int f7B044(int n)
{
    uint32_t rec = NATION(n);                                       /* 0x7B068 [ebp-0x4] */
    int32_t  count = S8(rec + 0x60u);                               /* 0x7B07B [ebp-0x3C] dword [rec + 0x5D] sar 24 */
    int32_t  room = 0;                                              /* [ebp-0x38] (dead: never read, D) */
    int32_t  over = 0;                                              /* [ebp-0x34] */
    int32_t  flag1 = 0;                                             /* [ebp-0x28] */
    int32_t  marked = 0;                                            /* [ebp-0x24] */
    int32_t  mval = 0;                                              /* [ebp-0x20] */
    int32_t  val = 0;                                               /* [ebp-0x1C] */
    int32_t  att = 0;                                               /* [ebp-0x10] */
    int32_t  i, a, lim, v, tok, m, idx, base, res;
    uint32_t c;                                                     /* [ebp-0xC] (32-bit products) */

    for (i = 0; i < count; i++) {                                   /* 0x7B0EB [ebp-0x48] */
        a = D8(rec + 0x61u + (uint32_t)i);                          /* 0x7B0FF [ebp-0x44] territory + 1 */
        lim = POP_LIMIT(a - 1);                                     /* 0x7B128 [ebp-0x40] */
        v = S8(0xEED83u + 160u * (uint32_t)n + (uint32_t)a);        /* 0x7B13D dword [160n + a + 0xEED80] sar 24 */
        if (v + (SITE(a - 1) ? 1 : 0) == 0)                         /* 0x7B15E add al, al; shr al, 7; 0x7B168 add; je */
            val += 0;                                               /* 0x7B194 (eax = 0) */
        else
            val += v / 4;                                           /* 0x7B18C sbb idiom: toward zero */
        if (CNT(a) == 1 && MARK(a, n) == n && D8(rec + 0x27u) != 0) /* 0x7B1AB, 0x7B1CB, 0x7B1D3 */
            lim++;                                                  /* 0x7B1DC */
        if ((D8(TERR(a - 1) + 2u) & 0x01u) != 0)                    /* 0x7B1F1 [0xE163A] */
            flag1++;
        if (MARKED(a - 1)) {                                        /* 0x7B20D je 0x7B097 */
            marked++;                                               /* 0x7B21D */
            mval += S8(0xEFA03u + 160u * (uint32_t)n + (uint32_t)a);   /* 0x7B232 dword [0xEFA00 + 160n + a] sar 24 */
            continue;
        }
        tok = TOK(a, n);                                            /* 0x7B0B2 [ebp-0x2C] */
        if (CNT(a) != 1)                                            /* 0x7B0CC */
            continue;
        if (lim > tok)                                              /* 0x7B0D6 jle 0x7B243 */
            room += lim - tok;                                      /* 0x7B0E2 */
        else if (lim < tok)                                         /* 0x7B24B jge */
            over += tok - lim;                                      /* 0x7B257 */
    }
    (void)room;
    for (i = 0; i < S32(0x1079B4u); i++) {                          /* 0x7B2EB cmp eax, [0x1079B4]; jge */
        if (D8(0x107694u + 4u * (uint32_t)i) != 0xC9u)              /* 0x7B309 */
            continue;
        m = D8(0x107695u + 4u * (uint32_t)i);                       /* 0x7B31C [ebp-0x18] */
        idx = S8(0xBCCE0u + 85u * (uint32_t)(m - 1) + 5u * (uint32_t)S8(rec + 0x5Cu));   /* 0x7B345 [ebp-0x8] */
        if (HUMAN(m))                                               /* 0x7B362 */
            idx++;
        base = HUMAN(m) ? 200 : 50;                                 /* 0x7B380: 0x7B386 0xC8 / 0x7B26F 0x32 */
        c = (uint32_t)(ATT(n, m) + base);                           /* 0x7B286, 0x7B28F */
        c *= (uint32_t)(S32(0xC2864u + 12u * (uint32_t)idx + 4u * (uint32_t)level()) / 100);   /* 0x7B2B9, 0x7B2C9 idiv, 0x7B2CE imul */
        if ((int32_t)c < 0)                                         /* 0x7B2D4 jge */
            c = 0;
        att = (int32_t)((uint32_t)att + c);                         /* 0x7B2E2 */
    }
    /* 32-bit sums as the original's add / sub (no overflow trap) */
    c = 500u * (uint32_t)S8(rec + 0x51u) + 500u * (uint32_t)marked + 100u * (uint32_t)mval;   /* 0x7B3BC..0x7B3F1 */
    if (D8(0xF488Cu + (uint32_t)n) == 0 && S32(0x108164u + 4u * (uint32_t)n) == 0) {   /* 0x7B393, 0x7B3A3 */
        c += 300u * (uint32_t)count + (uint32_t)val - 200u * (uint32_t)over - 200u * (uint32_t)flag1;   /* 0x7B409..0x7B438 */
    } else {
        c += (uint32_t)((300 * count + val) / 4) - 300u * (uint32_t)over - 200u * (uint32_t)flag1;   /* 0x7B4A2..0x7B4DF sbb idiom /4 */
    }
    res = (int32_t)(c + (uint32_t)att);                             /* 0x7B43D / 0x7B4E4 */
    if (res > 0x7FFF)                                               /* 0x7B4E9 */
        res = 0x7FFF;
    return res;
}

/* ==== 0x468EC the aggression trigger ============================================================================== */

int f468EC(void)
{
    int32_t  r = 0;                                                 /* 0x468FF [ebp-0x8] */
    int32_t  m;
    uint32_t rec;

    if (level() != 0)                                               /* 0x46914 jne 0x4698F */
        return r;
    for (m = 1; m <= 9; m++) {                                      /* 0x46929 */
        rec = NATION(m);                                            /* 0x4693D [ebp-0x4] */
        if (S8(rec + 0x9Au) != 3)                                   /* 0x4694C */
            continue;
        if (S8(rec + 0x99u) >= 4)                                   /* 0x4695D jge */
            continue;
        if (S8(rec + 0x51u) <= 0)                                   /* 0x4696D jle */
            continue;
        if (S32(0xF5470u) < 3)                                      /* 0x4696F jl */
            continue;
        if (D8(rec + 0x40u) >= 4)                                   /* 0x46983 jge */
            continue;
        r = 1;                                                      /* 0x46988 */
        break;
    }
    return r;
}

/* ==== 0x46998 mark targets ======================================================================================== */

void f46998(int n)
{
    int32_t t;

    for (t = 0; t <= 0x9D; t++) {                                   /* 0x469C1 jg: 157 included (Risk 8, kept) */
        if ((D8(AIF(n, t) + 1u) & 0x08u) == 0)                      /* 0x469D8 [314(n-1) + 2t + 0xF3581] */
            continue;
        if (SCORE(n, t) <= 0)                                       /* 0x469F6 dword [160n + t + 0xEED81] sar 24 */
            continue;
        if (S8(AREA(t + 1) + 0x3Bu + (uint32_t)n) == 1)             /* 0x46A16 [69(t+1) + n - 1 + 0xE31F1] sar 24 */
            continue;
        setD8(AREA(t + 1) + 0x3Bu + (uint32_t)n, 1);                /* 0x46A37 [0xE31F4 + ...] */
    }
}

/* ==== 0x39C08 the city sites n holds ============================================================================== */

int f39C08(int n)
{
    uint8_t copy[69];                                               /* [ebp-0x70] */
    int32_t count = 0;                                              /* 0x39C22 [ebp-0x24] */
    int32_t t, r;

    for (t = 0; t < 0x9D; t++) {                                    /* 0x39C37 jge */
        if (D8(0xEE16Cu + (uint32_t)t) == 0)                        /* 0x39C4D */
            continue;
        r = t + 1;                                                  /* 0x39C55 [ebp-0x20] */
        if (CITY(t) && (int32_t)D8(TSTATE(t)) == n) {               /* 0x39C6A, 0x39C89 je 0x39DF1 */
            count++;
            continue;
        }
        if (!SITE(t))                                               /* 0x39CA6 */
            continue;
        if ((D8(AIF(n, t)) & 0x04u) != 0) {                         /* 0x39CBC test bh, 4 (low byte) */
            count++;
            continue;
        }
        if (CNT(r) == 1 && MARK(r, n) == n) {                       /* 0x39CDC, 0x39CFC */
            count++;
            continue;
        }
        if (CNT(r) <= 1)                                            /* 0x39D19 jle */
            continue;
        if (TOK(r, n) == 0)                                         /* 0x39D3C */
            continue;
        if (f7C7C4(t, n) > 9) {                                     /* 0x39D48; 0x39D50 jg 0x39DF1 */
            count++;
            continue;
        }
        area_save(r, copy);                                         /* 0x39D6F */
        f7A778();                                                   /* 0x39D74 */
        setD8(REAL_RES, 0);                                         /* 0x39D79 (cl = 0) */
        f5B588(r);                                                  /* 0x39D84 */
        setD8(REAL_RES, 1);                                         /* 0x39D89 (ch = 1) */
        f7A7B0();                                                   /* 0x39D8F */
        area_restore(r, copy);                                      /* 0x39DAD */
        if (CNT(r) == 1 && MARK(r, n) == n)                         /* 0x39DC7, 0x39DEB on the restored record */
            count++;
    }
    return count;                                                   /* 0x39DFC */
}

/* ==== 0x160EC the opening-map priorities ========================================================================== */

void f160EC(int n)
{
    uint32_t score = 0xEED84u + 160u * (uint32_t)n;                 /* 0x16129 [ebp-0x8] */
    uint32_t base = 0xEF3C4u + 160u * (uint32_t)n;                  /* 0x16142 [ebp-0x4] */
    int32_t  t, idx, c;
    uint8_t  v;

    setD8(0xF62A8u + (uint32_t)n, 0);                               /* 0x1610D */
    setD32(0xF62B4u, 0);                                            /* 0x16147 */
    for (t = 0; t < 0xA0; t++) {                                    /* 0x16163 jge */
        if ((BPM(n, t) & 0x04u) == 0)                               /* 0x16188 test dh, 4 */
            continue;
        setD8(0xF62A8u + (uint32_t)n, 1);                           /* 0x16190 */
        idx = S32(0xF62B4u);                                        /* 0x161A4 */
        setD32(0xF62B4u, (uint32_t)(idx + 1));                      /* 0x161AC */
        setD8(0xF62B8u + 5u * (uint32_t)(n - 1) + (uint32_t)idx, (uint8_t)t);   /* 0x161B7 (no bound, kept) */
        c = D8(0xC803Cu + (uint32_t)t);                             /* 0x161C2 */
        if (TOK(c + 1, n) > 0)                                      /* 0x161D8 [n - 1 + 69(c+1) + 0xE31C4] sar 24; jle */
            v = (uint8_t)(S8(base + (uint32_t)t) + 5);              /* 0x161F3 add eax, 5 */
        else
            v = D8(base + (uint32_t)t);                             /* 0x16152 */
        setD8(score + (uint32_t)t, v);                              /* 0x1615B */
    }
}

/* ==== 0x7701C the table shuffle =================================================================================== */

void f7701C(int count, uint8_t *buf, int k)
{
    uint8_t local[192];                                             /* [ebp-0xBC]: 188 bytes; 4 more for a pick of 188 / 189 */
    int32_t n = count;                                              /* [ebp-0x1C] the entries left */
    int32_t out = 0;                                                /* [ebp-0x8] */
    int32_t row = 0;                                                /* [ebp-0x4] */
    int32_t i, x, div, idx;                                         /* [ebp-0x10] i, [ebp-0xC] idx */

    memset(local, 0, sizeof local);                                 /* port: defined bytes past the copy (BUG 1) */
    for (i = 0; i < n; i++)                                         /* 0x7704D */
        local[i] = buf[i];                                          /* 0x77060 */
    while (n != 0) {                                                /* 0x77069 */
        x = S16(0x112A92u + 20u * (uint32_t)(row % 30) + 2u * (uint32_t)k);   /* 0x77094 dword [... + 0x112A90] sar 16 */
        row++;                                                      /* 0x77076 */
        div = 30000 / (n > 30000 ? 30000 : n);                      /* 0x770A1..0x770BD */
        if (x / div >= n) {                                         /* 0x770C8 idiv; 0x770CD jge 0x77126 */
            idx = n - 1;                                            /* 0x77126 */
        } else {
            x = S16(0x112A92u + 20u * (uint32_t)(row % 30) + 2u * (uint32_t)k);   /* 0x770F0: the next row */
            row++;                                                  /* 0x770D2 */
            div = 30000 / (n > 30000 ? 30000 : n);                  /* 0x770FA..0x77117 */
            idx = x / div;                                          /* 0x77122 (not clamped) */
            /* BUG(1995): idx == n on the first pick reads the uninitialised byte past the caller's entries */
            if (!g_original && idx >= count)
                idx = n - 1;
        }
        buf[out++] = local[idx];                                    /* 0x7713B, 0x77142 */
        for (i = idx; n - 1 > i; i++)                               /* 0x77152 jle */
            local[i] = local[i + 1];                                /* 0x7715E */
        n--;                                                        /* 0x77174 */
    }
}
