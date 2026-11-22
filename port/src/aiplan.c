/* Module AIPLAN of M7 batch 1 (docs/M6_PLAN.md, module AIMOVE's passes): the movement passes of the computer players'
   planner 0x461BC. Literal translations; the address of each instruction a line comes from is in its comment. D =
   read in the code, G = inference. Prototypes, records and globals: aiplan.h.

     0x46BB8  f46BB8  spread tokens (passes of f46CBC)                                  260 bytes
     0x46CBC  f46CBC  spread from one territory                                       1,296
     0x471CC  f471CC  reorder the sources of the first spread pass                       724
     0x474A0  f474A0  attack weak neighbours (the roll counter [0x108198])             1,080
     0x478D8  f478D8  plan more cities                                                   364
     0x47A44  f47A44  plan more cities (crowded)                                         348
     0x49A1C  f49A1C  would t be refilled (trial)                                        620
     0x49C88  f49C88  shed surplus                                                       380
     0x49E04  f49E04  fill marked empty lands                                            644
     0x4A088  f4A088  excess tokens                                                      600
     0x4A2E0  f4A2E0  relocate tokens (recursive)                                      2,416
     0x4BBDC  f4BBDC  retreat                                                          1,176
     0x4C074  f4C074  attack decision                                                  1,648
     0x4C6E4  f4C6E4  reinforcement                                                      724
     0x4C9B8  f4C9B8  expansion targets                                                  676

   Not ported here (each call site stops the port with not_ported(address); the static stubs below carry the names, so
   a later translation that exports one of them collides with its stub at compile time instead of being shadowed).
   All belong to the other modules of M7 batch 1:
     AIQ:    0x48028 f48028(int32 *cnt, int32 list[], t, n, filter)   the land neighbours of t passing filter (sets *cnt)
             0x4876C f4876C(int32 *cnt, int32 list[], t, n, filter)   appends lone weak neighbours (filter 0x16)
             0x48D44 f48D44(int32 *cnt, int32 list[], n, t, flags)    sorts / filters a list in place
             0x495C0 f495C0(t, n, mode, int32 *enemy) -> int           the largest foreign stack next to t
             0x49790 f49790(r, t, n, enemy) -> int                     the tokens to keep in t
             0x47BA0 f47BA0(int32 *cnt, int32 list[], n, mode, flags)  n's territories by mode (sets *cnt)
             0x4CC5C f4CC5C(t, n, mode) -> int                         the tokens n needs to take t (-1 none)
             0x4CEE4 f4CEE4(t, n, int32 *out) -> int                   the tokens n needs to take the city at t
             0x7C838 f7C838(t, n, k, int32 *left) -> int               the what-if conflict (n's losses)
             0x160EC f160EC(n)                                         refresh n's territory scores
     AICITY: 0x33964 f33964(goal, int32 *count, n, mode)               plan cities (eax unused here)
             0x3580C f3580C(need, t, n, mode, s) -> int                move tokens toward t (ret 4; 0 = done)
     AISEA:  0x4AC50 f4AC50(t, n, int32 *k, mode) -> int               ship tokens away from t
   With -DAIPLAN_TEST (port/test/aiplan_test.c) they go to the test's recorders instead.

   Ported callees: f46A4C, f46B34 (formulas.c), f78D08, f7C760, f7C7C4, f57CA4 (rules.c), f5AAA4 (conflict.c, silent
   with 0), f5C238 (city.c, silent with 0), the World State Stack f7A940 / f7AC60 (stack.c) / f7AD24 and the Move List
   Stack f7AA1C / f7AD7C / f7AE14 (mstack.c), each in the original's order.

   No game_rand (f78AE4), no clock(), no screen and no text in these functions (D: their calls, listed above, and
   __CHK 0x7D4AC (dropped) and memset 0x7D5D0 (-> memset on the data object)). The only randomness is the pre-rolled
   table 0x112A94, read as the original does; 0x474A0's row counter [0x108198] lives in the data object and nothing
   resets it, so it is a process-lifetime static, as in the original (a restore does not reset it; risk 3).

   State the stacks do not restore (D; for the DOSBox-X check): [0xC28B4] (0x49A1C sets it 0, then 1), [0x1082DC],
   [0x108198], [0x108194], [0x107D40], 0x10819C / 0x10823C / 0x108164 + 4n / the mark array 0x107EE4, 0xF7C9C /
   0xF7CA0, 0xF0044 (0x49A1C restores its byte), nation +0x5D (written outside the pushes), the land move records past
   a restored count (0x49A1C, 0x4C9B8), and the globals of the silent 0x5AAA4(0) / 0x5C238(0) (risk 3).

   1995 bugs (graded code / effect). In the planner an odd heuristic is not a bug: only garbage reads are fixed.
     Stack words read before they are written (no fix needed):
     1./2. Stale locals of sibling frames, kept as the original (D frame layout; Claude, 21 Nov, after the DOSBox-X
        compare of the R4 movement showed the original's mark at 0x47247 for Babylon, territory 67). 0x46BB8 calls
        0x47BA0 (one stack argument: its ebp is 4 lower), 0x471CC and 0x46CBC (register arguments only) one after
        another at the same depth, so their frames overlap word for word; __CHK (0x7D4AC) and memset stay above
        [ebp-0x10]. Two words are read before they are written:
        - 0x471CC's spare [ebp-0x34] (written only for a source with targets, 0x472DF) = 0x47BA0's [ebp-0x30] = ebx = n
          (0x47BBB): 0x46BB8 always calls 0x47BA0 right before 0x471CC. So a target-less source before the first one
          with targets tests spare = n > 0 and is marked when F3580 bit 0x20 is set and 0x10819C[s] <= 0.
        - 0x46CBC's avail [ebp-0x38] (written only when the target list is not empty, 0x46D68) = the previous sibling's
          word there: 0x471CC's [ebp-0x38] = n - 1 (0x471EF), 0x47BA0's [ebp-0x34] = edx, the list pointer (0x47BB8:
          a stack address, about 0x31xxxx under DOS/4GW), or the previous 0x46CBC's final avail. 0x474A0 attacks from
          a target-less source when that is positive. The port keeps the word in s_slot38 (below); for the list pointer
          it uses SLOT38_LIST_PTR: avail is only tested against 0 and reduced by attack sizes (D: 0x474D1, 0x47708,
          0x478AA, 0x46E51, 0x471A5), so any value larger than a nation's 55 tokens behaves alike (G).
          0x46CBC's other caller, 0x49A1C (0x49B74), calls 0x48028 first (0x49A87, one stack argument: its ebp is 4
          lower), whose [ebp-0x34] = eax = &c (0x4803A, a stack address) is the same word, and nothing is called in
          between (0x49A8C..0x49B74): its first 0x46CBC starts from that pointer too (SLOT38_LIST_PTR), the later ones
          from the previous one's avail. Those trials are undone, but their 0x474A0 rolls advance [0x108198] (Claude,
          22 Nov: the DOSBox-X compare of round 11 of /SRD8 seed 0x5917060E, 110 more rolls in the original).
        Both are the original's behaviour in both modes (no g_original split): they only steer the computers' choices.
     Kept as the original (AI choices, deterministic):
     - 0x46BB8 clears 4 bytes of the int32[160] mark array 0x107EE4 (memset(*(int **)0xC2860, 0, 4): entry 0 only), so
       0x471CC reorders by marks left from earlier calls and from the ship routines (G: sizeof-pointer slip).
     - 0x49E04 reads its target u and u's limit once (list[0]); the loop index moves on but only u is filled.
     - 0x4C074 builds each attack's support list in the global 0xF7CA0 / [0xF7C9C] that its own outer loop walks: after
       the first attack candidate the outer loop goes on over the support territories w (territory numbers) as if they
       were area records (territory w - 1). w = 0 reads area record 0, the territory state bytes 0xE2C15..0xE2C17 and
       territory record -1 (0xE1615..), all inside the data object (risk 8: read through the data object, byte-exact);
       the land list of territory -1 ([0xF8E78], the current dialog's link address) is read through the data object
       too (land_word below), though the area record 0 count stops it first (G: record 0 is never written).
     - 0x49A1C empties t by writing 0 to the nations count +1 (not count - 1) and keeps the trial's move records past
       the restored count [0x1079B4]; the nested 0x474A0 rolls advance [0x108198] inside the trial.
     - 0x4BBDC calls f7C7C4(t, n) and ignores the result.
     - The land move list has no bound check (D): record 200 would overlap the count [0x1079B4] and later ones the
       ship list. The records are written through the data object in each site's own byte order (+0, +1, +3, then
       the count and +2 in the site's order), so even an overflow behaves as the original's while it stays inside the
       data object (G: unreachable, a nation has at most 55 tokens); a port guard skips writes outside it.
   Guards (port, no change in play): the relay's list of 0x4A2E0 (candidates + visited, 160 entries as the original's
   [ebp-0x2DC]) stops appending at 160 (G: at most about 50 at depth 4); land lists read 0 for a handle that is
   neither a heap handle nor a data-object address.
   Easter eggs: none in these functions (no strings, D). */

#include <stdint.h>
#include <string.h>

#include "aiplan.h"
#include "city.h"
#include "conflict.h"
#include "formulas.h"
#include "game.h"
#include "mstack.h"
#include "rules.h"
#include "stack.h"
#include "state.h"

#define NAT(n)       (0xF14CCu + 199u * (uint32_t)(n))
#define AREA(r)      (0xE31B8u + 69u * (uint32_t)(r))            /* area record r = territory r - 1 */
#define TS(t)        (0xE2C18u + 3u * (uint32_t)(t))              /* territory state t */
#define TERR(t)      (0xE1638u + 35u * (uint32_t)(t))             /* territory record t */
#define POPLIMIT(t)  ((int32_t)((((unsigned)D8(TERR(t) + 1u) << 2) & 0xFFu) >> 5))   /* shl al, 2; shr al, 5 */
#define BPM(n, t)    (0xF0044u + 160u * (uint32_t)(n) + (uint32_t)(t))
#define SCORE(n, t)  (0xEED84u + 160u * (uint32_t)(n) + (uint32_t)(t))
#define F3580(n, t)  (0xF3580u + 314u * (uint32_t)((n) - 1) + 2u * (uint32_t)(t))   /* +1: the hi byte 0xF3581 */
#define AITAB(row, n) (0x112A94u + 20u * (uint32_t)(row) + 2u * (uint32_t)((n) - 1))
#define LANDADJ      0xF8E7Cu    /* uint32[160]: handles to int16 count, int16 territories (game.h) */
#define MOVES        0x107694u   /* the land move list, 4-byte records */
#define MOVECNT      0x1079B4u   /* int32 */
#define CHOICE_CNT   0xF7C9Cu    /* int32 */
#define CHOICE       0xF7CA0u    /* int32 [157] */
#define DONE1        0x10819Cu   /* uint8 [160] */
#define DONE2        0x10823Cu   /* uint8 [160] */
#define MARKS_PTR    0xC2860u    /* int32 -> 0x107EE4 */
#define LOSS_TOK     0xF547Cu    /* uint8 [14] */
#define ORDER        0xF06C0u    /* int8 [14] */
#define LISTN        160         /* the stack lists' 0x280 bytes */

/* area record fields (movsx byte, as the original's dword ... sar 0x18 reads) */
static int32_t cnt_at(int32_t r)              { return S8(AREA(r) + 1u); }
static int32_t marker(int32_t r, int32_t n)   { return S8(AREA(r) + 1u + (uint32_t)n); }
static int32_t tokens(int32_t r, int32_t n)   { return S8(AREA(r) + 0x0Eu + (uint32_t)n); }
static int32_t movable(int32_t r, int32_t n)  { return S8(AREA(r) + 0x24u + (uint32_t)n); }

/* the AI level: mov ax, word [0xE1624]; shl eax, 4; shr ax, 0xE */
static int32_t ai_level(void)
{
    return (int32_t)((uint16_t)(D16(0xE1624u) << 4) >> 14);
}

/* one int16 of a land list (movsx word [h]): the lists live in a heap block (port handles, game.h); a value that is a
   data-object address (the out-of-range territory -1, see the file comment) is read there, as the original does */
static int32_t land_word(uint32_t h)
{
    const uint8_t *p = load_heap_ptr(h, 2);
    if (p != NULL)
        return (int16_t)(p[0] | p[1] << 8);
    if (ds_valid(h, 2))
        return S16(h);
    return 0;   /* port guard */
}

static void put8(uint32_t a, uint8_t v)
{
    if (ds_valid(a, 1))   /* port guard: the list overflowing out of the data object (file comment) */
        setD8(a, v);
}

/* one land move record at the count: +0 from, +1 to, +3 type, then the count + 1 and +2 k (late != 0) or +2 k and then
   the count (late == 0), the two orders of the original's sites */
static void move_rec(int32_t from, int32_t to, int32_t k, int type, int late)
{
    int32_t i = S32(MOVECNT);
    uint32_t a = MOVES + 4u * (uint32_t)i;
    put8(a, (uint8_t)from);
    put8(a + 1u, (uint8_t)to);
    put8(a + 3u, (uint8_t)type);
    if (late) {
        setD32(MOVECNT, (uint32_t)(i + 1));
        put8(a + 2u, (uint8_t)k);
    } else {
        put8(a + 2u, (uint8_t)k);
        setD32(MOVECNT, (uint32_t)(i + 1));
    }
}

/* ---- not ported here (see the file comment) ------------------------------------------------------------------ */

#ifdef AIPLAN_TEST
void aiplan_test_f48028(int32_t *cnt, int32_t *list, int t, int n, int filter);
void aiplan_test_f4876C(int32_t *cnt, int32_t *list, int t, int n, int filter);
void aiplan_test_f48D44(int32_t *cnt, int32_t *list, int n, int t, int flags);
int  aiplan_test_f495C0(int t, int n, int mode, int32_t *enemy);
int  aiplan_test_f49790(int r, int t, int n, int enemy);
void aiplan_test_f47BA0(int32_t *cnt, int32_t *list, int n, int mode, int flags);
int  aiplan_test_f4CC5C(int t, int n, int mode);
int  aiplan_test_f4CEE4(int t, int n, int32_t *out);
int  aiplan_test_f7C838(int t, int n, int k, int32_t *left);
void aiplan_test_f160EC(int n);
void aiplan_test_f33964(int goal, int32_t *count, int n, int mode);
int  aiplan_test_f3580C(int need, int t, int n, int mode, int s);
int  aiplan_test_f4AC50(int t, int n, int32_t *k, int mode);
#define f48028 aiplan_test_f48028
#define f4876C aiplan_test_f4876C
#define f48D44 aiplan_test_f48D44
#define f495C0 aiplan_test_f495C0
#define f49790 aiplan_test_f49790
#define f47BA0 aiplan_test_f47BA0
#define f4CC5C aiplan_test_f4CC5C
#define f4CEE4 aiplan_test_f4CEE4
#define f7C838 aiplan_test_f7C838
#define f160EC aiplan_test_f160EC
#define f33964 aiplan_test_f33964
#define f3580C aiplan_test_f3580C
#define f4AC50 aiplan_test_f4AC50
#else
/* M7 batch 1 linked (Claude, 21 Nov): the other planner modules' real routines (they were not_ported stubs
   while the five modules were translated in parallel) */
#include "aiq.h"
#include "aicity.h"
#include "aisea.h"
#include "aimove.h"
#endif

/* ---- 0x46BB8 spread tokens ----------------------------------------------------------------------------------- */

/* The word at [ebp-0x38] of 0x46BB8's children (0x47BA0's [ebp-0x34]): file comment, items 1./2. */
#define SLOT38_LIST_PTR 0x00310000
static int32_t s_slot38;
#ifdef AIPLAN_TEST
int32_t *aiplan_test_slot38(void) { return &s_slot38; }
#endif

void f46BB8(int n)
{
    int32_t cnt;              /* [ebp-0x18] */
    int32_t list[LISTN];      /* [ebp-0x2A8] */
    int32_t first;            /* [ebp-0x20] */
    int32_t changed;          /* [ebp-0x24] */
    int32_t i;                /* [ebp-0x1C] */
    uint32_t marks;

    if (f46B34(n) != 0)                                                  /* 0x46BD6 */
        return;                                                          /* (0x160EC skipped too) */
    memset(DP(DONE1), 0, 0xA0);                                          /* 0x46BEF */
    marks = D32(MARKS_PTR);                                              /* 0x46BF9 [0xC2860] */
    if (ds_valid(marks, 4))
        memset(DP(marks), 0, 4);                                         /* 0x46C00 4 bytes: entry 0 only (kept) */
    memset(DP(DONE2), 0, 0xA0);                                          /* 0x46C11 */
    first = 1;                                                           /* 0x46C1B */
    changed = 1;
    while (changed != 0) {                                               /* 0x46C21 */
        changed = 0;                                                     /* 0x46C2D */
        f47BA0(&cnt, list, n, 3, 1);                                     /* 0x46C43 */
        s_slot38 = SLOT38_LIST_PTR;                                      /* its [ebp-0x34] = the list pointer */
        if (first != 0 && D8(0xC28ACu) != 0) {                           /* 0x46C4B, 0x46C4F */
            f471CC(&cnt, list, n);                                       /* 0x46C64 */
            s_slot38 = n - 1;                                            /* its [ebp-0x38] (0x471EF) */
            first = 0;                                                   /* 0x46C69 */
        }
        for (i = 0; i < cnt; i++)                                        /* 0x46C79 */
            if (f46CBC(list[i], n) == 1) {                               /* 0x46C8E */
                changed = 1;                                             /* 0x46C9A */
                setD32(0x108164u + 4u * (uint32_t)n, 1);                 /* 0x46CA0 */
            }
    }
    f160EC(n);                                                           /* 0x46CAC */
}

/* ---- 0x46CBC spread from one territory ---------------------------------------------------------------------- */

int f46CBC(int s, int n)
{
    int32_t ret = 0;          /* [ebp-0x44] */
    int32_t r = s + 1;        /* [ebp-0x18] */
    int32_t cnt;              /* [ebp-0x14] */
    int32_t list[LISTN];      /* [ebp-0x2D0] */
    int32_t avail;            /* [ebp-0x38] */
    int32_t moved;            /* [ebp-0x34] tokens already moved out of s */
    int32_t keep;             /* [ebp-0x30] */
    int32_t k;                /* [ebp-0x1C] / [ebp-0x20] */
    int32_t enemy;            /* [ebp-0x3C] */
    int32_t avail0;           /* [ebp-0x2C] */
    int32_t i;                /* [ebp-0x4C] */
    int32_t u, ru;            /* [ebp-0x28], [ebp-0x24] */
    int32_t lim;

    avail = s_slot38;   /* the previous sibling's word when the list is empty (file comment, items 1./2.) */

    f48028(&cnt, list, s, n, 0);                                         /* 0x46CFC */
    f4876C(&cnt, list, s, n, 0x16);                                      /* 0x46D12 */
    f48D44(&cnt, list, n, s, D8(0xF488Cu + (uint32_t)n) != 0 ? 0x436 : 0x36);   /* 0x46D1A, 0x46D3E */
    if (cnt != 0) {                                                      /* 0x46D43 */
        avail = movable(r, n);                                           /* 0x46D5F */
        moved = tokens(r, n) - avail;                                    /* 0x46D7D */
        keep = 0;                                                        /* 0x46D90 */
        k = f495C0(s, n, 0, &enemy);                                     /* 0x46D9E */
        k = f49790(k, s, n, enemy);                                      /* 0x46DB2 */
        if ((D8(BPM(n, s)) & 0x01) != 0)                                 /* 0x46DCC */
            keep = k;                                                    /* 0x46DF4 */
        else if (k != 0 && f49A1C(s, n, k, -1) == 0)                     /* 0x46DD7, 0x46DEB */
            keep = k;
        if (keep != 0 && moved == 0)                                     /* 0x46DFA */
            avail -= k;                                                  /* 0x46E09 */
        if (ret == 0 && avail > 0)                                       /* 0x46E0C */
            ret = 1;
    }
    if (D8(0xC28ACu) != 0)                                               /* 0x46E1F */
        f474A0(s, n, &avail, &ret, cnt, list);                           /* 0x46E3F */
    if (cnt == 0) {                                                      /* 0x46E44 */
        s_slot38 = avail;
        return ret;
    }
    avail0 = avail;                                                      /* 0x46E51 */
    for (i = 0; i < cnt && avail > 0; i++) {                             /* 0x46E59 */
        u = list[i];                                                     /* 0x46E79 */
        ru = u + 1;
        if (S8(SCORE(n, u)) > S8(SCORE(n, s)) && D8(DONE2 + (uint32_t)s) == 0 &&   /* 0x46EB7, 0x46EBE */
            ((D8(F3580(n, u)) & 0x80) != 0 || (D8(F3580(n, u)) & 0x04) != 0)) {    /* 0x46ED1, 0x46EE5 */
            avail -= f49C88(s, n);                                       /* 0x46EF5 */
            setD8(DONE2 + (uint32_t)s, 1);                               /* 0x46F00 */
            continue;
        }
        k = 1;                                                           /* 0x46F11 */
        lim = POPLIMIT(u);
        if ((D8(BPM(n, u)) & 0x02) != 0 && cnt_at(ru) == 1) {            /* 0x46F2C, 0x46F45 */
            k = avail;                                                   /* 0x46F4C */
            if (lim < k)                                                 /* 0x46F74 */
                k = lim;
        } else if ((D8(BPM(n, u)) & 0x02) != 0 && cnt_at(ru) > 1) {      /* 0x46FB9, 0x46FD6 */
            k = tokens(ru, f7C760(u));                                   /* 0x46FED, 0x46FF2 the first nation there */
            if (k > avail)                                               /* 0x47007 */
                k = (avail + k <= lim) ? avail : 0;                      /* 0x47035 */
        } else if ((D8(F3580(n, u) + 1u) & 0x04) != 0 || (D8(F3580(n, u)) & 0x20) != 0 ||   /* 0x47059, 0x47068 */
                   (D8(F3580(n, u)) & 0x40) != 0) {                      /* 0x4707C */
            k = f495C0(u, n, 0, &enemy);                                 /* 0x47095 */
            if (k == 0)                                                  /* 0x470A0 */
                k = 1;
            else if (lim < k)                                            /* 0x470CF */
                k = lim;
            if (k > avail)                                               /* 0x470FC */
                k = avail;
        }
        if (k != 0) {                                                    /* 0x47107 */
            f57CA4(s, u, n, 1, k);                                       /* 0x47123 */
            if (cnt_at(ru) == 1)                                         /* 0x4713C */
                setD8(AREA(ru) + 0x3Bu + (uint32_t)n, 0);                /* 0x47155 */
            move_rec(s, u, k, 5, 1);                                     /* 0x4715B..0x4718B */
            avail -= k;                                                  /* 0x47194 */
        }
    }
    if (avail0 != avail && S32(0xC28B4u) != 0)                           /* 0x471A5, 0x471AA */
        setD8(DONE1 + (uint32_t)s, 1);                                   /* 0x471B6 */
    s_slot38 = avail;
    return ret;                                                          /* 0x471BD */
}

/* ---- 0x471CC reorder the sources ---------------------------------------------------------------------------- */

void f471CC(int32_t *cnt, int32_t *list, int n)
{
    int32_t i;                /* [ebp-0x48] */
    int32_t s;                /* [ebp-0x40] */
    int32_t r;                /* [ebp-0x18] */
    int32_t c;                /* [ebp-0x1C] */
    int32_t l2[LISTN];        /* [ebp-0x54C] */
    int32_t spare;            /* [ebp-0x34]: lives across the loop (file comment, items 1./2.) */
    int32_t moved;            /* [ebp-0x30] */
    int32_t keep;             /* [ebp-0x2C] */
    int32_t k;                /* [ebp-0x28] */
    int32_t enemy;            /* [ebp-0x24] */
    int32_t flag;             /* [ebp-0x10] */
    int32_t j;                /* [ebp-0x20] */
    int32_t copy[LISTN];      /* [ebp-0x2CC] */
    int32_t total;            /* [ebp-0x14] */
    uint32_t marks = D32(MARKS_PTR);

    spare = n;   /* until a source with targets sets it: 0x47BA0's [ebp-0x30] = n (file comment, items 1./2.) */
    for (i = 0; i < *cnt; i++) {                                         /* 0x47254 */
        s = list[i];                                                     /* 0x4726D */
        r = s + 1;
        f48028(&c, l2, s, n, 0);                                         /* 0x47288 */
        f4876C(&c, l2, s, n, 0x16);                                      /* 0x4729E */
        f48D44(&c, l2, n, s, 0x36);                                      /* 0x472B4 */
        if (c != 0) {                                                    /* 0x472BC */
            spare = movable(r, n);                                       /* 0x472DF */
            moved = tokens(r, n) - spare;                                /* 0x47302 */
            keep = 0;                                                    /* 0x47307 */
            k = f495C0(s, n, 0, &enemy);                                 /* 0x47315 */
            k = f49790(k, s, n, enemy);                                  /* 0x47329 */
            if ((D8(BPM(n, s)) & 0x01) != 0)                             /* 0x47349 */
                keep = k;
            else if (k != 0 && f49A1C(s, n, k, -1) == 0)                 /* 0x4734E, 0x47362 */
                keep = k;
            if (keep != 0 && moved == 0)                                 /* 0x47371 */
                spare -= k;                                              /* 0x47380 */
        }
        if (spare > 0 && (D8(F3580(n, s)) & 0x20) != 0 && S8(DONE1 + (uint32_t)s) <= 0) {   /* 0x47383..0x473B3 */
            flag = 0;                                                    /* 0x473BB */
            for (j = 0; j < c; j++)                                      /* 0x471FF */
                if ((D8(BPM(n, l2[j])) & 0x02) != 0) {                   /* 0x47226 */
                    flag = 1;                                            /* 0x4722B */
                    break;
                }
            if (flag != 0 || c <= 1)                                     /* 0x47232, 0x47238 */
                if (ds_valid(marks + 4u * (uint32_t)s, 4))
                    setD32(marks + 4u * (uint32_t)s, 1);                 /* 0x47247 */
        }
    }
    /* 0x473C6: rep movs of the list, then marked entries first, then the rest */
    memcpy(copy, list, 4u * (uint32_t)(*cnt > 0 ? (*cnt < LISTN ? *cnt : LISTN) : 0));
    total = *cnt;                                                        /* 0x473EC */
    *cnt = 0;                                                            /* 0x473F2 */
    for (i = 0; i < total; i++)                                          /* 0x47405 */
        if (ds_valid(marks + 4u * (uint32_t)copy[i], 4) && S32(marks + 4u * (uint32_t)copy[i]) != 0)   /* 0x4741C */
            list[(*cnt)++] = copy[i];                                    /* 0x47436, 0x47442 */
    for (i = 0; i < total; i++)                                          /* 0x47453 */
        if (!(ds_valid(marks + 4u * (uint32_t)copy[i], 4) && S32(marks + 4u * (uint32_t)copy[i]) != 0))   /* 0x4746B */
            list[(*cnt)++] = copy[i];                                    /* 0x47485, 0x47491 */
}

/* ---- 0x474A0 attack weak neighbours ------------------------------------------------------------------------- */

void f474A0(int s, int n, int32_t *avail, int32_t *ret, int cnt, const int32_t *list)
{
    int32_t r = s + 1;        /* [ebp-0x1C] */
    int32_t flag;             /* [ebp-0x14] */
    int32_t j;                /* [ebp-0x24] */
    int32_t c;                /* [ebp-0x8] */
    int32_t l[LISTN];         /* [ebp-0x2B4] */
    int32_t u;                /* [ebp-0xC] */
    int32_t k;                /* [ebp-0x10] */
    int32_t x;                /* [ebp-0x20] */
    int32_t ctr, v;
    int pass;

    if (*avail <= 0)                                                     /* 0x474D1 */
        return;
    if ((D8(F3580(n, s)) & 0x20) == 0)                                   /* 0x474E4 */
        return;
    if (S8(DONE1 + (uint32_t)s) > 0)                                     /* 0x474F5 */
        return;
    flag = 0;                                                            /* 0x47508 */
    for (j = 0; j < cnt; j++)                                            /* 0x47516 */
        if ((D8(BPM(n, list[j])) & 0x02) != 0) {                         /* 0x4753C */
            flag = 1;                                                    /* 0x47547 */
            break;
        }
    if (flag == 0 && cnt > 1)                                            /* 0x4754E, 0x47554 */
        return;
    f48028(&c, l, s, n, 0x0B);                                           /* 0x4756F */
    f48D44(&c, l, n, s, 1);                                              /* 0x47585 */
    for (pass = 1; pass <= 2; pass++)                                    /* 0x47597 pass 1, 0x47738 pass 2 */
        for (j = 0; j < c; j++) {
            u = l[j];                                                    /* 0x475AD / 0x4774E */
            if (pass == 1 ? (D8(F3580(n, u) + 1u) & 0x02) == 0           /* 0x475BA hi bit 0x02 */
                          : (D8(F3580(n, u)) & 0x10) == 0)               /* 0x4775B lo bit 0x10 */
                continue;
            k = f4CC5C(u, n, 3);                                         /* 0x475D1 / 0x47772 */
            if (k <= 0)
                continue;
            if (movable(r, n) < k)                                       /* 0x475F2 / 0x47793 */
                continue;
            if (f78D08(u, n) < k)                                        /* 0x47606 / 0x477A7 */
                continue;
            if (f7C838(u, n, k, &x) != 0)                                /* 0x4761C / 0x477BD */
                continue;
            ctr = S32(0x108198u);                                        /* 0x47629 / 0x477CA */
            setD32(0x108198u, (uint32_t)(ctr + 1));                      /* 0x47649 / 0x477EA */
            v = S16(AITAB(ctr % 10 + 10, n)) / 300;                      /* 0x4763B idiv 10, 0x47655, 0x47669 idiv 300 */
            if (v >= 90 - 20 * ai_level())                               /* 0x4766D..0x47694 */
                continue;
            f57CA4(s, u, n, 1, k);                                       /* 0x476AC / 0x4784D */
            setD8(AREA(u + 1) + 0x3Bu + (uint32_t)n, 0);                 /* 0x476CC / 0x4786D */
            move_rec(s, u, k, pass == 1 ? 0x13 : 0x15, 1);               /* 0x476D2..0x476FC / 0x47873..0x4789E */
            *avail -= k;                                                 /* 0x47708 / 0x478AA */
            if (S32(0xC28B4u) != 0)                                      /* 0x4770A / 0x478AC */
                setD8(DONE1 + (uint32_t)s, 1);
            *ret = 1;                                                    /* 0x47720 / 0x478C2 */
        }
}

/* ---- 0x478D8 / 0x47A44 plan more cities --------------------------------------------------------------------- */

/* the spare measured after a silent conflict and surplus removal, undone by the World State Stack:
   tokens (+0x98) - 2 - (2(cities + *cnt) + 2) (0x47915..0x4795A, 0x4798D..0x479D2, 0x47AA7..0x47AEC, 0x47B20..0x47B67) */
static int32_t measure_spare(int n, const int32_t *cnt)
{
    int32_t spare;
    f7A940();
    f5AAA4(0);
    f5C238(0);
    spare = S8(NAT(n) + 0x98u) - 2 - ((S8(NAT(n) + 0x51u) + *cnt) * 2 + 2);
    f7AC60();
    return spare;
}

void f478D8(int c, int32_t *cnt, int n, int32_t *flag)
{
    int32_t spare;            /* [ebp-0x4] */
    int32_t c2;               /* [ebp-0xC] */
    int32_t l[LISTN];         /* [ebp-0x29C] */
    int32_t old;              /* [ebp-0x8] */

    if (S8(NAT(n) + 0x51u) + *cnt >= 9)                                  /* 0x4790C */
        return;
    spare = measure_spare(n, cnt);                                       /* 0x47915..0x4795A */
    if (spare <= 0 && *flag == 0) {                                      /* 0x47962, 0x4796B */
        *flag = 1;                                                       /* 0x47972 */
        f4A088(n, 0, 1);                                                 /* 0x47980 (edx = *flag, 0) */
        f4C074(n);                                                       /* 0x47988 */
        spare = measure_spare(n, cnt);                                   /* 0x4798D..0x479D2 */
    }
    if (spare <= 0)                                                      /* 0x479D7 */
        return;
    setD8(NAT(n) + 0x5Du, (uint8_t)spare);                               /* 0x479E7 */
    f47BA0(&c2, l, n, 0x0E, 0);                                          /* 0x47A00 */
    if (c2 == 0)                                                         /* 0x47A0A */
        return;
    old = *cnt;                                                          /* 0x47A11 */
    f33964(c + 1, cnt, n, 1);                                            /* 0x47A23 */
    if (old != *cnt)                                                     /* 0x47A30 */
        setD32(0x108194u, 1);                                            /* 0x47A34 */
}

void f47A44(int c, int32_t *cnt, int n, int32_t *flag)
{
    int32_t spare;            /* [ebp-0x4] */

    if (S8(NAT(n) + 0x98u) < 20)                                         /* 0x47A70 */
        return;
    if (f46A4C(n) == 0)                                                  /* 0x47A7C */
        return;
    if (S8(NAT(n) + 0x51u) + *cnt >= 9)                                  /* 0x47A9E */
        return;
    spare = measure_spare(n, cnt);                                       /* 0x47AA7..0x47AEC */
    if (spare <= 0 && *flag == 0) {                                      /* 0x47AF4, 0x47AFB */
        *flag = 1;                                                       /* 0x47B03 */
        f4A088(n, 0, 1);                                                 /* 0x47B13 */
        f4C074(n);                                                       /* 0x47B1B */
        spare = measure_spare(n, cnt);                                   /* 0x47B20..0x47B67 */
    }
    if (spare <= 0)                                                      /* 0x47B6C */
        return;
    setD8(NAT(n) + 0x5Du, (uint8_t)spare);                               /* 0x47B7C */
    f33964(c + S32(0x108194u) + 1, cnt, n, 0);                           /* 0x47B8B, 0x47B94 */
}

/* ---- 0x49A1C would t be refilled ---------------------------------------------------------------------------- */

int f49A1C(int t, int n, int k, int exclude)
{
    int32_t res = 0;          /* [ebp-0x10] */
    int32_t found = 0;        /* [ebp-0xC] */
    int32_t saved;            /* [ebp-0x1C] */
    int32_t r;                /* [ebp-0x18] */
    int32_t c;                /* [ebp-0x20] */
    int32_t l[LISTN];         /* [ebp-0x2B4] */
    int32_t i;                /* [ebp-0x28] */
    uint8_t fl;               /* [ebp-0x8] */
    int32_t v;

    if (S8(0x1082DCu) != 0)                                              /* 0x49A4B dword [0x1082D9] sar 24 */
        return 0;
    setD8(0x1082DCu, 1);                                                 /* 0x49A5B */
    f7A940();                                                            /* 0x49A62 */
    saved = S32(MOVECNT);                                                /* 0x49A67 */
    r = t + 1;
    f48028(&c, l, t, n, 0x0A);                                           /* 0x49A87 */
    s_slot38 = SLOT38_LIST_PTR;          /* its [ebp-0x34] = &c (0x4803A): the first f46CBC's avail (file comment) */
    for (i = 0; i < c; i++)                                              /* 0x49A99 */
        if (l[i] == exclude) {                                           /* 0x49AAE */
            l[i] = -1;                                                   /* 0x49AB5 */
            break;
        }
    setD8(AREA(r) + 1u, 0);                                              /* 0x49ACC the nations count */
    setD8(AREA(r) + 1u + (uint32_t)n, 0);                                /* 0x49AE5 the marker */
    setD8(AREA(r) + 0x0Eu + (uint32_t)n, 0);                             /* 0x49AFD the tokens */
    fl = D8(BPM(n, t));                                                  /* 0x49B15 */
    if ((fl & 0x02) == 0)                                                /* 0x49B1E */
        setD8(BPM(n, t), (uint8_t)(D8(BPM(n, t)) | 0x02));               /* 0x49B36 */
    setD32(0xC28B4u, 0);                                                 /* 0x49B3F */
    for (i = 0; i < c; i++) {                                            /* 0x49B50 */
        if (l[i] == -1)                                                  /* 0x49B62 */
            continue;
        if (f46CBC(l[i], n) == 1 && tokens(r, n) != 0) {                 /* 0x49B74, 0x49B99 */
            found = 1;                                                   /* 0x49B9D */
            break;
        }
    }
    setD32(0xC28B4u, 1);                                                 /* 0x49BA9 */
    if (found != 0) {                                                    /* 0x49BAF */
        v = S16(AITAB(t % 8 + 12, n)) / 300;                             /* 0x49BC7 idiv 8, 0x49BDA, 0x49BEE idiv 300 */
        if (v < 90 - 15 * ai_level()) {                                  /* 0x49BF2..0x49C26 */
            if (tokens(r, n) >= k)                                       /* 0x49C34 */
                res = 1;
        } else if (tokens(r, n) != 0)                                    /* 0x49C43 */
            res = 1;                                                     /* 0x49C47 */
    }
    setD8(BPM(n, t), fl);                                                /* 0x49C61 */
    setD32(MOVECNT, (uint32_t)saved);                                    /* 0x49C6A */
    f7AC60();                                                            /* 0x49C71 */
    setD8(0x1082DCu, 0);                                                 /* 0x49C76 */
    return res;
}

/* ---- 0x49C88 shed surplus ------------------------------------------------------------------------------------- */

int f49C88(int s, int n)
{
    int32_t moved = 0;        /* [ebp-0x10] */
    int32_t surplus;          /* [ebp-0x14] */
    uint32_t adj;             /* [ebp-0xC] */
    int32_t i;                /* [ebp-0x24] */
    int32_t u;                /* [ebp-0x1C] */

    surplus = tokens(s + 1, n) - f78D08(s, n);                           /* 0x49CBF, 0x49CCE, 0x49CD3 */
    if (surplus <= 0)                                                    /* 0x49CD8 */
        return moved;
    adj = D32(LANDADJ + 4u * (uint32_t)s);                               /* 0x49CE5 */
    for (i = 1; land_word(adj) >= i && surplus > 0; i++) {               /* 0x49D01, 0x49D0D */
        u = land_word(adj + 2u * (uint32_t)i);                           /* 0x49D1F */
        if ((D8(F3580(n, u)) & 0x20) == 0)                               /* 0x49D3D */
            continue;
        if ((D8(F3580(n, u) + 1u) & 0x02) != 0)                          /* 0x49D4C */
            continue;
        if (POPLIMIT(u) - tokens(u + 1, n) != 1)                         /* 0x49D93, 0x49D95 */
            continue;
        f57CA4(s, u, n, 1, 1);                                           /* 0x49DAD (pushed eax = 1) */
        move_rec(s, u, 1, 0x17, 1);                                      /* 0x49DB2..0x49DDF */
        surplus--;                                                       /* 0x49DE9 */
        moved++;                                                         /* 0x49DEF */
    }
    return moved;
}

/* ---- 0x49E04 fill marked empty lands ------------------------------------------------------------------------ */

void f49E04(int n)
{
    uint32_t rec = NAT(n);    /* [ebp-0x18] */
    int32_t i;                /* [ebp-0x48] */
    int32_t r, t;             /* [ebp-0x38], [ebp-0x3C] */
    int32_t m;                /* [ebp-0x2C] min(limit, 2) */
    int32_t spare;            /* [ebp-0x34] */
    int32_t c;                /* [ebp-0x1C] */
    int32_t l[LISTN];         /* [ebp-0x2C8] */
    int32_t j;                /* [ebp-0x44] */
    int32_t u;                /* [ebp-0x30] */
    int32_t ulim;             /* [ebp-0x24] */

    for (i = 0; S8(rec + 0x60u) > i; i++)                                /* 0x49E44 */
        setD32(CHOICE + 4u * (uint32_t)i, D8(rec + 0x61u + (uint32_t)i));   /* 0x49E63 */
    setD32(CHOICE_CNT, (uint32_t)S8(rec + 0x60u));                       /* 0x49E6C */
    for (i = 0; i < S32(CHOICE_CNT); i++) {                              /* 0x49E7E */
        r = S32(CHOICE + 4u * (uint32_t)i);                              /* 0x49E90 */
        t = r - 1;
        if (cnt_at(r) != 1)                                              /* 0x49EB5 */
            continue;
        if (marker(r, n) != n)                                           /* 0x49ED5 */
            continue;
        if (movable(r, n) <= 0)                                          /* 0x49EF5 */
            continue;
        if ((D8(TS(t) + 2u) & 0x02) != 0)                                /* 0x49F09 */
            continue;
        m = f78D08(t, n);                                                /* 0x49F1C */
        if (m >= 3)                                                      /* 0x49F27 */
            m = 2;
        spare = tokens(r, n) - m;                                        /* 0x49F57 */
        if (spare <= 0)                                                  /* 0x49F5C */
            continue;
        f48028(&c, l, t, n, 0x13);                                       /* 0x49F77 */
        f48D44(&c, l, n, t, 0x12);                                       /* 0x49F8D */
        if (c <= 0)                                                      /* 0x49F95 */
            continue;
        j = 0;                                                           /* 0x49F9F */
        u = l[j];                                                        /* 0x49FAC the first target only (kept) */
        ulim = f78D08(u, n);                                             /* 0x49FB5 */
        for (; j < c && spare > 0; j++)                                  /* 0x49FBD, 0x49FC9 */
            while (spare > 0 && movable(r, n) > 0 && tokens(u + 1, n) < ulim) {   /* 0x49FD3, 0x49FF8, 0x4A01B */
                f57CA4(t, u, n, 1, 1);                                   /* 0x4A02D */
                move_rec(t, u, 1, 0x18, 1);                              /* 0x4A032..0x4A05F */
                spare--;                                                 /* 0x4A069 */
            }
    }
}

/* ---- 0x4A088 excess tokens -------------------------------------------------------------------------------------- */

void f4A088(int n, int mode, int conquer)
{
    uint32_t rec = NAT(n);    /* [ebp-0x10] */
    int32_t l[LISTN];         /* [ebp-0x2CC] */
    int32_t i;                /* [ebp-0x2C] */
    int32_t count;            /* [ebp-0x20] */
    int32_t r, t;             /* [ebp-0x24], [ebp-0x44] */
    int32_t lim;              /* [ebp-0x30] */
    int32_t x = 0;            /* [ebp-0x38] (always written before it is read, D) */
    int32_t left;             /* [ebp-0x28] */
    int32_t excess;           /* [ebp-0x34] */
    int32_t vis[1];           /* [ebp-0x1C] */
    int32_t nvis;             /* [ebp-0x18] */
    int32_t tn;

    for (i = 0; S8(rec + 0x60u) > i; i++)                                /* 0x4A0CF */
        l[i] = D8(rec + 0x61u + (uint32_t)i);                            /* 0x4A0EE */
    count = S8(rec + 0x60u);                                             /* 0x4A100 */
    for (i = 0; i < count; i++) {                                        /* 0x4A15E */
        r = l[i];                                                        /* 0x4A174 */
        t = r - 1;
        lim = f78D08(t, n);                                              /* 0x4A184 */
        if ((D8(TS(t) + 2u) & 0x02) != 0 && (int32_t)D8(TS(t)) == n)     /* 0x4A19E, 0x4A1BA */
            continue;
        if (marker(r, n) != n)                                           /* 0x4A1DA */
            continue;
        if (!(cnt_at(r) == 1 && tokens(r, n) > lim)) {                   /* 0x4A1F7, 0x4A217 */
            if (cnt_at(r) < 1)                                           /* 0x4A230 */
                continue;
            tn = tokens(r, n);                                           /* 0x4A24B */
            x = f7C838(t, n, 0, &left);                                  /* 0x4A25F */
            if (tn != x)                                                 /* 0x4A267 */
                continue;
        }
        if (movable(r, n) <= 0)                                          /* 0x4A28B */
            continue;
        if (cnt_at(r) == 1)                                              /* 0x4A2B9 */
            excess = tokens(r, n) - lim;                                 /* 0x4A2CE */
        else
            excess = movable(r, n) == x ? x : 0;                         /* 0x4A116 */
        if (excess <= 0)                                                 /* 0x4A124 */
            continue;
        vis[0] = t;                                                      /* 0x4A12D */
        nvis = 1;                                                        /* 0x4A130 */
        f4A2E0(excess, t, n, 0, vis, nvis, mode, conquer);               /* 0x4A153 (ecx = [ebp-0x14] = 0) */
    }
}

/* ---- 0x4A2E0 relocate tokens (recursive) ------------------------------------------------------------------------ */

static int visited_in(int32_t u, const int32_t *visited, int nvisited)   /* the scans 0x4A347 / 0x4A5D9 / 0x4A85A */
{
    int32_t ok = 1;           /* [ebp-0x38] */
    int32_t j;                /* [ebp-0x3C] */
    for (j = 0; j < nvisited; j++)
        if (u == visited[j])
            ok = 0;           /* no break */
    return ok;
}

int f4A2E0(int k, int s, int n, int depth, const int32_t *visited, int nvisited, int mode, int conquer)
{
    int32_t moved;            /* [ebp-0x2C] */
    uint32_t adj;             /* [ebp-0xC] */
    int32_t cnt;              /* [ebp-0x24] */
    int32_t list[LISTN];      /* [ebp-0x2DC] */
    int32_t i;                /* [ebp-0x40] */
    int32_t u, ru;            /* [ebp-0x18] / [ebp-0x34] / [ebp-0x20], [ebp-0x14] / [ebp-0x1C] */
    int32_t room;             /* [ebp-0x30] */
    int32_t lim;              /* [ebp-0x48] */
    int32_t o;                /* [ebp-0x44] */
    int32_t nc;               /* [ebp-0x10] */
    int32_t x;                /* [ebp-0x28] */
    int32_t w;

    if (depth >= S32(0xC28B0u))                                          /* 0x4A304 */
        return 0;
    depth++;                                                             /* 0x4A316 */
    moved = 0;                                                           /* 0x4A322 */

    /* pass 1: empty lands (no city) and n's own below the limit */
    adj = D32(LANDADJ + 4u * (uint32_t)s);                               /* 0x4A328 */
    cnt = 0;                                                             /* 0x4A332 */
    for (i = 1; land_word(adj) >= i; i++) {                              /* 0x4A426 */
        u = land_word(adj + 2u * (uint32_t)i);                           /* 0x4A439 */
        if (!visited_in(u, visited, nvisited))                           /* 0x4A368 */
            continue;
        ru = u + 1;
        if (!(cnt_at(ru) == 1 && marker(ru, n) == n && tokens(ru, n) < f78D08(u, n)) &&   /* 0x4A390, 0x4A3B0, 0x4A3DA */
            (cnt_at(ru) != 0 || (D8(TS(u) + 2u) & 0x01) != 0))           /* 0x4A3F2, 0x4A400 */
            continue;
        if (cnt < LISTN)                                                 /* port guard */
            list[cnt++] = u;                                             /* 0x4A419 */
    }
    f48D44(&cnt, list, n, s, 6);                                         /* 0x4A463 */
    for (i = 0; i < cnt && k > 0; i++) {                                 /* 0x4A46B, 0x4A477 */
        u = list[i];                                                     /* 0x4A48B */
        room = k;                                                        /* 0x4A491 */
        lim = f78D08(u, n);                                              /* 0x4A49A */
        if (tokens(u + 1, n) + room > lim)                               /* 0x4A4C6 */
            room = lim - tokens(u + 1, n);                               /* 0x4A4E9 */
        while (k > 0 && room > 0) {                                      /* 0x4A4F2, 0x4A4F8 */
            f57CA4(s, u, n, 1, 1);                                       /* 0x4A50E */
            move_rec(s, u, 1, depth + 6, 1);                             /* 0x4A513..0x4A543 type depth + 6 (7..10) */
            k--;                                                         /* 0x4A54D */
            room--;
            moved++;
        }
    }

    /* by ship */
    if (mode != 1 && k > 0 && (D8(TERR(s)) & 0x02) != 0)                 /* 0x4A569, 0x4A56F, 0x4A586 */
        moved += f4AC50(s, n, &k, depth);                                /* 0x4A59B */

    /* conquest: neighbours held by one weaker nation below the limit */
    if (k > 0 && conquer == 1) {                                         /* 0x4A5A3, 0x4A5B0 */
        adj = D32(LANDADJ + 4u * (uint32_t)s);                           /* 0x4A5BC */
        cnt = 0;                                                         /* 0x4A5C8 */
        for (i = 1; land_word(adj) >= i; i++) {                          /* 0x4A6AE */
            u = land_word(adj + 2u * (uint32_t)i);                       /* 0x4A6C1 */
            if (!visited_in(u, visited, nvisited))                       /* 0x4A5FC */
                continue;
            ru = u + 1;
            if (cnt_at(ru) != 1)                                         /* 0x4A624 */
                continue;
            if (marker(ru, n) == n)                                      /* 0x4A648 */
                continue;
            if (tokens(ru, f7C760(u)) >= POPLIMIT(u))                    /* 0x4A65C, 0x4A68F */
                continue;
            if (cnt < LISTN)                                             /* port guard */
                list[cnt++] = u;                                         /* 0x4A6A1 */
        }
        f48D44(&cnt, list, n, s, 6);                                     /* 0x4A6E9 */
        for (i = 0; i < cnt && k > 0; i++) {                             /* 0x4A6F3, 0x4A6FF */
            u = list[i];                                                 /* 0x4A713 */
            o = f7C760(u);                                               /* 0x4A719 */
            room = k;                                                    /* 0x4A724 */
            if (room + tokens(u + 1, o) > POPLIMIT(u))                   /* 0x4A76A */
                room = POPLIMIT(u) - tokens(u + 1, o);                   /* 0x4A7AF */
            while (k > 0 && room > 0) {                                  /* 0x4A7B8, 0x4A7BE */
                f57CA4(s, u, n, 1, 1);                                   /* 0x4A7D4 */
                move_rec(s, u, 1, 0x22, 0);                              /* 0x4A7D9..0x4A808 (+2 before the count) */
                k--;                                                     /* 0x4A810 */
                room--;
                moved++;
            }
        }
    }

    /* relay: n's neighbours with spare movable tokens move on themselves, then s fills the room they leave */
    if (k > 0) {                                                         /* 0x4A82C */
        adj = D32(LANDADJ + 4u * (uint32_t)s);                           /* 0x4A839 */
        cnt = 0;                                                         /* 0x4A845 */
        for (i = 1; land_word(adj) >= i; i++) {                          /* 0x4A9A6 */
            u = land_word(adj + 2u * (uint32_t)i);                       /* 0x4A9B9 */
            if (!visited_in(u, visited, nvisited))                       /* 0x4A87D */
                continue;
            ru = u + 1;
            if (!(cnt_at(ru) == 1 && marker(ru, n) == n && movable(ru, n) > 0 &&   /* 0x4A8A2, 0x4A8C6, 0x4A8E8 */
                  movable(ru, n) > tokens(ru, n) - f78D08(u, n)) &&      /* 0x4A90B, 0x4A92F */
                (cnt_at(ru) <= 1 || marker(ru, n) != n || movable(ru, n) <= 0))   /* 0x4A948, 0x4A968, 0x4A987 */
                continue;
            if (cnt < LISTN)                                             /* port guard */
                list[cnt++] = u;                                         /* 0x4A999 */
        }
        f48D44(&cnt, list, n, s, 6);                                     /* 0x4A9E1 */
        nc = cnt;                                                        /* 0x4A9E9 */
        for (i = 0; i < nvisited; i++)                                   /* 0x4A9F9 the visited list appended */
            if (cnt < LISTN)                                             /* port guard */
                list[cnt++] = visited[i];                                /* 0x4AA12 */
        for (i = 0; i < nc && k > 0; i++) {                              /* 0x4AA28, 0x4AA34 */
            u = list[i];                                                 /* 0x4AA48 */
            ru = u + 1;
            lim = f78D08(u, n);                                          /* 0x4AA58 */
            x = movable(ru, n) > k ? k : movable(ru, n);                 /* 0x4AA80 */
            if (x > lim)                                                 /* 0x4AAAB */
                x = lim;
            room = f4A2E0(x, u, n, depth, list, cnt, mode, conquer);     /* 0x4AAD2 */
            if (room + tokens(ru, n) > POPLIMIT(u)) {                    /* 0x4AB1E */
                w = S16(AREA(ru) + 0x37u);                               /* 0x4AB2E int16 +0x37 */
                if (w != 0)                                              /* 0x4AB47 */
                    room = w - tokens(ru, n);                            /* 0x4AB98..0x4ABBA */
                else if (cnt_at(ru) <= 1)                                /* 0x4AB57 */
                    room = POPLIMIT(u) - tokens(ru, n);                  /* 0x4AB59..0x4ABBA */
            }
            while (k > 0 && room > 0) {                                  /* 0x4ABC7, 0x4ABD1 */
                f57CA4(s, u, n, 1, 1);                                   /* 0x4ABEB */
                move_rec(s, u, 1, depth + 6, 0);                         /* 0x4ABF0..0x4AC22 (+2 before the count) */
                k--;                                                     /* 0x4AC2A */
                room--;
                moved++;
            }
        }
    }
    return moved;                                                        /* 0x4AC41 */
}

/* ---- 0x4BBDC retreat ------------------------------------------------------------------------------------------ */

void f4BBDC(int n)
{
    uint32_t rec = NAT(n);    /* [ebp-0x18] */
    int32_t cand = 0;         /* [ebp-0x2C] */
    int32_t list[0x140];      /* [ebp-0x7CC] */
    int32_t l[LISTN];         /* [ebp-0x2CC] */
    int32_t c;                /* [ebp-0x24] */
    int32_t i;                /* [ebp-0x48] */
    int32_t j;                /* [ebp-0x28] */
    int32_t r, t;             /* [ebp-0x20], [ebp-0x4C] */
    int32_t x;                /* [ebp-0x34] */
    int32_t mv;               /* [ebp-0x40] */
    int32_t m;                /* [ebp-0x3C] */
    int32_t tn;

    for (i = 0; S8(rec + 0x60u) > i; i++) {                              /* 0x4BC1F */
        r = D8(rec + 0x61u + (uint32_t)i);                               /* 0x4BC39 */
        t = r - 1;
        (void)f7C7C4(t, n);                                              /* 0x4BC4C, result [ebp-0x30] unused (kept) */
        if (cnt_at(r) <= 1)                                              /* 0x4BC68 */
            continue;
        if (marker(r, n) != n)                                           /* 0x4BC88 */
            continue;
        tn = tokens(r, n);                                               /* 0x4BC9F */
        if (f7C838(t, n, 0, &x) != tn)                                   /* 0x4BCB3, 0x4BCB8 */
            continue;
        if (movable(r, n) <= 0)                                          /* 0x4BCDB */
            continue;
        if (S8(AREA(r) + 0x3Bu) != 0)                                    /* 0x4BCF7 byte +0x3B */
            continue;
        if (cand < 0x140)                                                /* port guard (n's list has <= 127) */
            list[cand++] = t;                                            /* 0x4BD08 */
    }

    /* pass 1: everything to a refuge */
    for (i = 0; i < cand; i++) {                                         /* 0x4BD2B */
        f48028(&c, l, list[i], n, 0x17);                                 /* 0x4BD4F */
        mv = movable(list[i] + 1, n);                                    /* 0x4BD79 */
        if (c <= 0 || mv <= 0)                                           /* 0x4BD81, 0x4BD87 */
            continue;
        move_rec(list[i], l[0], mv, 0x29, 1);                            /* 0x4BD8C..0x4BDC3 */
        f57CA4(list[i], l[0], n, 1, mv);                                 /* 0x4BDE5 */
        list[i] = -1;                                                    /* 0x4BDED */
    }

    /* pass 2: into neighbours with two nations, as many as each needs */
    for (i = 0; i < cand; i++) {                                         /* 0x4BE14 */
        if (list[i] == -1)                                               /* 0x4BE2A */
            continue;
        t = list[i];                                                     /* 0x4BE39 */
        r = t + 1;
        f48028(&c, l, t, n, 0x0C);                                       /* 0x4BE54 */
        mv = movable(r, n);                                              /* 0x4BE74 */
        if (c <= 0 || mv <= 0)                                           /* 0x4BE7C, 0x4BE82 */
            continue;
        for (j = 0; j < c; j++) {                                        /* 0x4BE9F */
            if (cnt_at(l[j] + 1) != 2)                                   /* 0x4BEC9 */
                continue;
            m = f4CC5C(l[j], n, 0);                                      /* 0x4BEDD */
            while (mv > 0 && m > 0) {                                    /* 0x4BEE5, 0x4BEEB */
                move_rec(t, l[j], 1, 0x29, 1);                           /* 0x4BEF1..0x4BF25 */
                f57CA4(t, l[j], n, 1, 1);                                /* 0x4BF43 */
                mv--;                                                    /* 0x4BF4B */
                m--;
            }
            list[i] = -1;                                                /* 0x4BE8E */
        }
    }

    /* pass 3: the rest to the first neighbour */
    for (i = 0; i < cand; i++) {                                         /* 0x4BF6D */
        if (list[i] == -1)                                               /* 0x4BF83 */
            continue;
        t = list[i];                                                     /* 0x4BF92 */
        r = t + 1;
        f48028(&c, l, t, n, 0x18);                                       /* 0x4BFAD */
        mv = movable(r, n);                                              /* 0x4BFCD */
        if (c <= 0 || mv <= 0)                                           /* 0x4BFD5, 0x4BFDB */
            continue;
        for (j = 0; j < c; j++) {                                        /* 0x4BFF8 */
            while (mv > 0) {                                             /* 0x4C004 */
                move_rec(t, l[j], 1, 0x29, 1);                           /* 0x4C00A..0x4C03E */
                f57CA4(t, l[j], n, 1, 1);                                /* 0x4C05C */
                mv--;                                                    /* 0x4C064 */
            }
            list[i] = -1;                                                /* 0x4BFE7 */
        }
    }
}

/* ---- 0x4C074 attack decision ---------------------------------------------------------------------------------- */

void f4C074(int n)
{
    uint32_t rec = NAT(n);    /* [ebp-0x18] */
    int32_t metal;            /* [ebp-0x2C] */
    int32_t i;                /* [ebp-0x24] */
    int32_t r, t;             /* [ebp-0x5C], [ebp-0x40] */
    int32_t lim;              /* [ebp-0x44] */
    int32_t surplus;          /* [ebp-0x64] */
    uint32_t adj;             /* [ebp-0x1C] */
    int32_t c;                /* [ebp-0x3C] */
    int32_t list[LISTN];      /* [ebp-0x2EC] */
    int32_t k;                /* [ebp-0x48] */
    int32_t j;                /* [ebp-0x38] */
    int32_t u, w;             /* [ebp-0x54], [ebp-0x34] */
    int32_t m;                /* [ebp-0x50] */
    int32_t diff;             /* [ebp-0x28] */
    int32_t e;                /* [ebp-0x60] */
    int32_t sum;              /* [ebp-0x4C] */
    int32_t x;                /* [ebp-0x58] */
    int32_t need;             /* [ebp-0x20] */
    int32_t idx, v, thr;

    metal = S8(rec + 0x26u);                                             /* 0x4C0AA dword [rec + 0x23] sar 24 */
    for (i = 0; S8(rec + 0x60u) > i; i++)                                /* 0x4C0C0 */
        setD32(CHOICE + 4u * (uint32_t)i, D8(rec + 0x61u + (uint32_t)i));   /* 0x4C0DF */
    setD32(CHOICE_CNT, (uint32_t)S8(rec + 0x60u));                       /* 0x4C0E8 */
    /* the outer loop walks the shared list, which the support lists below overwrite (kept, file comment) */
    for (i = 0; i < S32(CHOICE_CNT); i++) {                              /* 0x4C0FA */
        r = S32(CHOICE + 4u * (uint32_t)i);                              /* 0x4C10C */
        t = r - 1;
        lim = f78D08(t, n);                                              /* 0x4C123 */
        if ((D8(TS(t) + 2u) & 0x02) != 0)                                /* 0x4C13D */
            continue;
        if ((D8(TS(t) + 2u) & 0x01) != 0)                                /* 0x4C14E */
            continue;
        if (cnt_at(r) != 1)                                              /* 0x4C16B */
            continue;
        if (marker(r, n) != n)                                           /* 0x4C18B */
            continue;
        if (movable(r, n) <= 0)                                          /* 0x4C1AF */
            continue;
        if (tokens(r, n) <= lim)                                         /* 0x4C1D2 */
            continue;
        surplus = tokens(r, n) - lim;                                    /* 0x4C1FB */
        adj = D32(LANDADJ + 4u * (uint32_t)t);                           /* 0x4C201 */
        c = 0;                                                           /* 0x4C20D */
        for (k = 1; land_word(adj) >= k; k++) {                          /* 0x4C21F */
            u = land_word(adj + 2u * (uint32_t)k);                       /* 0x4C232 */
            if (cnt_at(u + 1) != 1)                                      /* 0x4C253 */
                continue;
            if (marker(u + 1, n) != 0)                                   /* 0x4C273 */
                continue;
            if (c < LISTN)                                               /* port guard */
                list[c++] = u;                                           /* 0x4C287 */
        }
        if (c == 0)                                                      /* 0x4C290 */
            continue;
        for (j = 0; j < c && surplus > 0; j++) {                         /* 0x4C2A7, 0x4C2B3 */
            u = list[j];                                                 /* 0x4C2C7 */
            m = f7C760(u);                                               /* 0x4C2CD */
            diff = S8(NAT(m) + 0x26u) - metal;                           /* 0x4C2DC, 0x4C2E8 */
            e = tokens(u + 1, m);                                        /* 0x4C300 */
            adj = D32(LANDADJ + 4u * (uint32_t)u);                       /* 0x4C30F */
            setD32(CHOICE_CNT, 0);                                       /* 0x4C320 */
            for (k = 1; land_word(adj) >= k; k++) {                      /* 0x4C331 */
                w = land_word(adj + 2u * (uint32_t)k);                   /* 0x4C348 */
                if (cnt_at(w + 1) != 1)                                  /* 0x4C369 */
                    continue;
                if ((D8(TS(w) + 2u) & 0x02) != 0)                        /* 0x4C37A */
                    continue;
                if (marker(w + 1, n) != n)                               /* 0x4C39E */
                    continue;
                if (tokens(w + 1, n) <= f78D08(w, n))                    /* 0x4C3C4, 0x4C3C9 */
                    continue;
                idx = S32(CHOICE_CNT);                                   /* 0x4C3D1 */
                setD32(CHOICE_CNT, (uint32_t)(idx + 1));                 /* 0x4C3DE */
                setD32(CHOICE + 4u * (uint32_t)idx, (uint32_t)w);        /* 0x4C3E6 */
            }
            if (S32(CHOICE_CNT) < 1)                                     /* 0x4C3F1 */
                continue;
            sum = 0;                                                     /* 0x4C400 */
            for (k = 0; k < S32(CHOICE_CNT); k++) {                      /* 0x4C40E */
                w = S32(CHOICE + 4u * (uint32_t)k);
                sum += tokens(w + 1, n) - f78D08(w, n);                  /* 0x4C435, 0x4C44B, 0x4C452 */
            }
            (void)f7C838(u, n, sum, &x);                                 /* 0x4C463 (eax unused) */
            if (!((S8(LOSS_TOK + (uint32_t)m) + S8(LOSS_TOK + (uint32_t)n) > 0 &&   /* 0x4C480 */
                   S8(LOSS_TOK + (uint32_t)m) >= S8(LOSS_TOK + (uint32_t)n)) ||     /* 0x4C492 signed bytes */
                  POPLIMIT(u) == 1))                                     /* 0x4C4BC */
                continue;
            thr = 100 - 10 * ai_level() + 10 * S8(rec + 0x9Bu);          /* 0x4C4C5..0x4C504 */
            v = S16(AITAB(u % 4 + 3, n)) / 300;                          /* 0x4C515 idiv 4, 0x4C528, 0x4C53C idiv 300 */
            if (v >= thr)                                                /* 0x4C53E */
                continue;
            need = -999;                                                 /* 0x4C546 */
            if (POPLIMIT(u) == 1 &&                                      /* 0x4C56F */
                !(S8(LOSS_TOK + (uint32_t)m) + S8(LOSS_TOK + (uint32_t)n) > 0 &&    /* 0x4C58C */
                  S8(LOSS_TOK + (uint32_t)m) >= S8(LOSS_TOK + (uint32_t)n))) {      /* 0x4C59E */
                if (S8(ORDER + (uint32_t)n) > S8(ORDER + (uint32_t)m))   /* 0x4C5B8 signed bytes */
                    continue;
                need = 1;                                                /* 0x4C5C0 */
            }
            if (need == -999)                                            /* 0x4C5C7 */
                need = e + diff + (e + diff < sum ? 1 : 0);              /* 0x4C5D9 setl */
            for (k = 0; k < S32(CHOICE_CNT); k++)                        /* 0x4C5FC */
                while (need > 0) {                                       /* 0x4C60B */
                    w = S32(CHOICE + 4u * (uint32_t)k);
                    if (movable(w + 1, n) <= 0)                          /* 0x4C634 */
                        break;
                    if (tokens(w + 1, n) <= f78D08(w, n))                /* 0x4C66F */
                        break;
                    f57CA4(w, u, n, 1, 1);                               /* 0x4C68A */
                    need--;                                              /* 0x4C692 */
                    surplus--;                                           /* 0x4C698 */
                    move_rec(w, u, 1, 0x11, 1);                          /* 0x4C69E..0x4C6CE */
                }
        }
    }
}

/* ---- 0x4C6E4 reinforcement -------------------------------------------------------------------------------------- */

void f4C6E4(int n)
{
    int32_t t;                /* [ebp-0x40] */
    int32_t safe;             /* [ebp-0x2C] */
    int32_t x;                /* [ebp-0x20] */
    int32_t l0, l1;           /* [ebp-0x28], [ebp-0x24] */
    int32_t c;                /* [ebp-0x18] */
    int32_t l[LISTN];         /* [ebp-0x2C4] */
    int32_t j;                /* [ebp-0x3C] */
    int32_t u;                /* [ebp-0x1C] */
    int32_t done;             /* [ebp-0x34] */

    for (t = 0; t < 0x9D; t++) {                                         /* 0x4C883 */
        if ((D8(F3580(n, t) + 1u) & 0x02) == 0)                          /* 0x4C89A */
            continue;
        safe = 1;                                                        /* 0x4C8AB */
        l0 = f7C838(t, n, 0, &x);                                        /* 0x4C8B9 */
        if (!(tokens(t + 1, n) > l0))                                    /* 0x4C8E3 */
            safe = 0;
        if (safe != 0)                                                   /* 0x4C8EA */
            continue;
        l1 = f7C838(t, n, 1, &x);                                        /* 0x4C8FE */
        if (l1 > l0)                                                     /* 0x4C90C */
            continue;
        if (tokens(t + 1, n) + 1 < l1)                                   /* 0x4C931 */
            continue;
        if ((D8(TERR(t) + 1u) & 0x40) == 0 && (D8(BPM(n, t)) & 0x02) == 0)   /* 0x4C94B, 0x4C966 */
            continue;
        f48028(&c, l, t, n, 0x0A);                                       /* 0x4C984 */
        f48D44(&c, l, n, t, 1);                                          /* 0x4C99C */
        done = 0;                                                        /* 0x4C9A1 */
        for (j = 0; j < c; j++) {                                        /* 0x4C716 */
            u = l[j];                                                    /* 0x4C72C */
            if (tokens(u + 1, n) <= f78D08(u, n))                        /* 0x4C735, 0x4C75E */
                continue;
            f57CA4(u, t, n, 1, 1);                                       /* 0x4C770 */
            move_rec(u, t, 1, 0x1A, 1);                                  /* 0x4C775..0x4C7A7 */
            done = 1;                                                    /* 0x4C7AE */
            break;
        }
        if (done != 0)                                                   /* 0x4C7B6 */
            continue;
        f48028(&c, l, t, n, 0x12);                                       /* 0x4C7CD */
        f48D44(&c, l, n, t, 1);                                          /* 0x4C7E3 */
        for (j = 0; j < c; j++) {                                        /* 0x4C7F3 */
            u = l[j];                                                    /* 0x4C809 */
            if (movable(u + 1, n) <= 0)                                  /* 0x4C82A */
                continue;
            f57CA4(u, t, n, 1, 1);                                       /* 0x4C83C */
            move_rec(u, t, 1, 0x1A, 1);                                  /* 0x4C841..0x4C873 */
            done = 1;                                                    /* 0x4C87A */
            break;
        }
    }
}

/* ---- 0x4C9B8 expansion targets ---------------------------------------------------------------------------------- */

void f4C9B8(int n, int x)
{
    int32_t zero = 0;         /* [ebp-0x20] */
    int32_t cnt;              /* [ebp-0x18] */
    int32_t list[LISTN];      /* [ebp-0x2AC] */
    int32_t i;                /* [ebp-0x1C] */
    int32_t need;             /* [ebp-0x14] */
    int32_t tmp;              /* [ebp-0x10] */
    int32_t t, r;

    if (S8(NAT(n) + 0x98u) < 14)                                         /* 0x4C9F0 */
        return;
    cnt = 0;                                                             /* 0x4C9F9 */
    for (i = 1; i <= 0x9D; i++)                                          /* 0x4CA0B */
        if (S8(AREA(i) + 0x3Bu + (uint32_t)n) == x)                      /* 0x4CA26 */
            list[cnt++] = i - 1;                                         /* 0x4CA47 */
    if (cnt == 0)                                                        /* 0x4CA50 */
        return;
    f48D44(&cnt, list, n, zero, 5);                                      /* 0x4CA6D */
    for (i = 0; i < cnt; i++) {                                          /* 0x4CBAD */
        t = list[i];
        r = t + 1;
        need = -1;                                                       /* 0x4CBB9 */
        if (S32(0x107D40u) == 0 && (D8(TS(t) + 2u) & 0x01) != 0 && cnt_at(r) == 0) {   /* 0x4CBC6, 0x4CBE1, 0x4CC0C */
            need = f4CEE4(t, n, &tmp);                                   /* 0x4CC24 */
            setD32(0x107D40u, 1);                                        /* 0x4CC2C */
        } else if ((D8(TS(t) + 2u) & 0x01) == 0) {                       /* 0x4CA93 */
            if (cnt_at(r) <= 1 || marker(r, n) == n)                     /* 0x4CABD, 0x4CB07 */
                need = f4CC5C(t, n, 4);                                  /* 0x4CB1E */
        }
        if (need == -1)                                                  /* 0x4CB26 */
            continue;
        f7A940();                                                        /* 0x4CB2C */
        f7AA1C();                                                        /* 0x4CB31 */
        if (f3580C(need, t, n, 1, need) != 0) {                          /* 0x4CB4F, 0x4CB54 */
            f7AC60();                                                    /* 0x4CC3B */
            f7AE14();                                                    /* 0x4CC42 */
            setD32(0x107D40u, 0);                                        /* 0x4CC47 (ecx = 0 kept across the call) */
        } else {
            f7AD24();                                                    /* 0x4CB5C */
            f7AD7C();                                                    /* 0x4CB61 */
            setD8(AREA(r) + 0x3Bu, D8(AREA(r) + 0x0Eu + (uint32_t)n));   /* 0x4CB9B, 0x4CBA1 */
        }
    }
}
