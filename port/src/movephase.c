/* Module MOVE of docs/M6_PLAN.md, the phase: MOVEMENT 0x55574 (7,148 bytes) and its helpers 0x57160, 0x57254, 0x574B0,
   0x577B0, 0x57810, 0x5790C, 0x57940, 0x5799C, 0x57A90, 0x57AFC, 0x57BD8, 0x582CC, 0x58330; the computers' movement
   playback 0x4D518 with 0x4D82C and 0x4DACC; the animated moves 0x5A084, 0x5A2C4, 0x5A4BC, 0x5A750. Literal
   translations; the address of each instruction a line comes from is in its comment. D = read in the code, G =
   inference. Prototypes: movephase.h. The primitives and the dispatcher's movement buttons are move.c's.

   Where it waits (D): a human's turn waits in the phase's own loop, not in f78BA4: f775F0 (until no button is down),
   then fg_mousepos into 0xE1628 / 0xE162C / 0xE1630 until [0xE1630] != 0 (0x55FFD; the second copy 0x56F93 once
   nothing is left to move), then f3D434(x, y, 0xF596C): a panel button goes through f3C5D4 and the dispatcher f3FB94
   (its code), a map click gives the territory (y <= 0x145; -100 outside). The codes the loop handles (dispatcher cases
   in move.c / dispatch.c): -1 FINISHED (0x2A), -2 MOVE SHIPS / NEXT SHIP, -3 MOVE TOKENS, -4 UNDO MOVE, -0x63 RESTART
   (0x41), -0x79 FIND UNMOVED; a territory >= 0. A computer's turn ends in f78BA4(0xF596C, 0) on the dialog 0xC59EC
   (REPLAY 0xC9 -> -0x5E, CONTINUE 0x2A -> -1), or f78A20(250 / 1000) when unattended (f7BA64).
   The structure keeps the original's jumps as gotos with its labels (L55934 a nation's turn, L55C6E the next nation,
   L55F8D the click loop, L56037 a click, L560D5 a territory click, L56EDF nothing left to move).

   Not ported here (each call site stops the port with not_ported(address); the static stubs below carry the names,
   as tax.c's do; with -DMOVEPHASE_TEST they go to the test's recorders, which return):
     0x353C4  the computer's trial planning (AICITY), called at 0x55A52 between a World State push and pop
     0x461BC  the AI movement planner (AIMOVE), called at 0x55A5F; it is the only caller of f4D518 (0x468D6)
   So a MOVEMENT with a computer in play stops at its first computer; an all-human game runs to the end. f4D518,
   f4DACC and the four animated moves are translated here but reached only through 0x461BC (and the dropped PBEM viewer
   0x59298) until AIMOVE lands (M7).

   Play by e-mail (dropped by Erik's ruling): no PBEM routine is called from these functions. The bit-0x80 tests of
   byte [0xE1624] are kept, and so are their branches (they only keep the move log and the local player's records):
   the 10-byte move log [0xF09B4] (malloc 0x55E5A, realloc 0x56090 / 0x57045 into 0xF0984[n], counts int16 0xF09B8 /
   0xF09BA + 2n), written through load_heap_ptr with move.c's guard (a handle that does not resolve is skipped); the
   "NOTE: Tax assessments..." message; the local player's records put back at the end (0x57093). The port never sets
   bit 0x80, so none of this runs (G).

   Watcom runtime -> standard C:
     0x7DB3D sprintf -> snprintf bounded to the destination: 0xF5500 (500 bytes, as loop.c), the 100-byte stack
             buffers of 0x55574 ([ebp-0xAC]..[ebp-0x48]), 0x57160 ([ebp-0x80]..[ebp-0x1C]), 0x57254 ([ebp-0x6C]..
             [ebp-0x8]) and 0x4D518 ([ebp-0xA4]..[ebp-0x40])
     0x822B5 malloc(size, tag, "movement") -> load_heap_alloc (port handles in [0xEED74] / [0xEED78], as ship.c and
             stack.c's dcase_50 / 51 use them); 0x877E6 free -> load_heap_free; 0x91CAC realloc -> mp_realloc below
             (PBEM only)
     rep movsd / movsb record copies -> memcpy between g_ds and the heap blocks
     0x7D4AC (__CHK) dropped; 0x78B24 (the allocation trace) and 0x78B0C (the debug trace, "movement - MSStore") are
     no-ops (game.h) and dropped.
   game_rand f78AE4: none drawn here (D). The computer's turn saves the seed [0xCAC5C] before planning (0x55963) and
   puts it back for REPLAY (0x55B28), so the replay draws the same numbers (in 0x353C4 / 0x461BC, not ported).

   Data (D, read in the code at the addresses given; G for meanings not stated by the code):
     0xBB3FA..0xBB400 int16 message area x, y, w, h = 0xA9, 0x14E, 0x12C, 0x22 (0x5558C); 0x4D518 sets y / h to 0x14C /
       0x7B for its message and restores them
     0xE2C18 + 3t territory state: +0 owner of the city, +1 its round, +2 bit 0x01 city, bit 0x02 (cleared for all
       157 here, set by the AI playback's marker records, from 0xC8; G: a target of the AI)
     0xF069A int16[] / int16 [0xF0698] the census order (census.c); 0xF06B0 int16[] / int16 [0xF06AE] the movement order
     nation n (0xF14CC + 199n): +0x25 Clothmaking (ships 5), +0x28 Roadbuilding (2 land steps), +0x2B Astronomy (open
       sea), +0x35 Military (moves last), +0x40.. ships (move.h), +0x9A 0 out / 2 computer / 3 human
     int32 [0xF40F8] the moving nation; int32 [0xF09E4] some nation 1..9 holds +0x28 (0x557F1); int32 [0xF5908] 1 while
       a computer plans (G: read by the AI code)
     int8 [0x108804] 1 tokens / 2 ships; int16 [0xF09E0] the count; int32 [0x109E10] the territory of the panel; int32
       [0xF06CC] 1 = a source is chosen; int32 [0x109E18] / [0x109E1C] FIND UNMOVED's cursors; 0x108808..0x10881C the
       last move (move.c's file comment)
     int32 0xC4990[n] nation n's start territory for its turn (-1 in obj2): the first source it chooses in a turn
       (int32 0x1087DC[n] = -1 at the start of the turn marks "not chosen yet")
     [0xEED74] TempTerr (0x2B65 bytes: area records 0..160), [0xEED78] Tempnation (199 bytes: one nation record)
     int32 0xC65B0 = 500, 0xC65B8 = 100, 0xC65BC = 300 (ms) the AI playback's waits; bit 0x40 of [0xF097C] Show AI
       Movement, bit 0 of [0xF097D] (word [0xF097C] << 7 >> 15) Speedup AI Movement; bit 0x08 Restart Confirmations,
       bit 0x10 Movement Finished Confirmation
     land move list (AI): 4-byte records at 0x107694, int32 [0x1079B4] count: +0 from (0xFF skip, 0xC8 marker of the
       territory +1, 0xC9 skip), +1 to, +2 int8 count (> 50: +50 flag a, > 75: +75 flags a and b, see f4D518)
     ship move list (AI): 6-byte records at 0x1079B8, int32 [0x107D3C] count: +0 from, +1 to, +2 int8 count, +3 ship
       index, +4 type (3..10)
     dialogs: 0xF596C the map screen's panel; 0xC4A3C the movement buttons (MOVE TOKENS 0x26, 1..5 0xC4..0xC8, MOVE
       SHIPS 0x27, FIND UNMOVED 0x29, UNDO MOVE 0x28, FINISHED 0x2A, RESTART 0x41); 0xC4E28 the ship buttons
       0x2B..0x32 (EMBARK, DISEMBARK, DISEMBARK ALL, NEXT SHIP; the short second-coast twins); 0xC59EC REPLAY 0xC9 /
       CONTINUE 0x2A; [0xF8E78] the dialog f3B05C drew last; [0xF5940] the first player button's record
     names 0xC768C[n], adjectives 0xC76C4[n] (uint32 link addresses)

   1995 bugs, fixed unless g_original (each marked BUG(1995) in the code; grades D code / G effect):
     1. 0x55F33 / 0x55F7B: a human nation in play with no movable token on the board (f577B0 returns -1, e.g. a nation
        left with cities and ships, or cities only) gets f3F624(-1, 15), which writes 15 into byte 0xFFB5F, the top byte
        of the heap pointer [0xFFB5C] (seaadj.dat's block, freed by 0x3F558 at START NEW GAME: G a crash then), and
        f3DA60(-1), which centres the map on territory 0xC803C[-1] = 0 and names it "PIRACY" (0xC77D8[-1]) in the
        panel. Fixed: no highlight, and the nation's home view (f3D9FC) instead of the centring. D (code), G (reach:
        a nation with no tokens on the board but a city keeps playing).
     2. 0x57810 FIND UNMOVED never returns when nothing qualifies: it rescans for ever. Reachable in sea mode: the
        button is enabled when f5799C is true (0x55FC7), which also counts a ship with tokens aboard and no points
        left, while the scan wants points > 0; e.g. the last ship with points sails its last point with tokens aboard,
        then FIND UNMOVED hangs the game (D code, G reach). Fixed: after one full empty scan it returns -1, which the
        caller already treats as "nothing" (0x56245 jl 0x560D5).
     3. Text: "...ship maint./const.for the other players..." (0xB5AE4, PBEM only) -> "const. for" (missing space;
        Erik's ruling of 19 Nov).
   Kept as the original, for Erik (D code unless marked):
     - f4D82C merges runs of equal from / to by adding run - 1 to the first record's count: right only if every merged
       record moves one token (G: the AI's records). It runs only when show != 0, and AIMOVE passes show = ([0xF5908]
       != 0) (0x468C4..0x468D6), which MOVEMENT sets to 1 around its call (0x5598C / 0x55A6B); the Show AI Movement
       toggle (bit 0x40) is not read before the sort. So with a record of more tokens the board could differ between
       AIMOVE's run in MOVEMENT and its runs from the other callers of 0x461BC (0x60F6A.., 0x682A0) with [0xF5908] == 0,
       not with the toggle; the merge can also run from the < 50 records into the >= 50 ones (it stops only at the
       0xFF sentinel). To settle with AIMOVE (M7) in DOSBox-X. Its local copy [ebp-0x334]..[ebp-0x15] holds 200
       records with no bound check (a 201st overwrites its loop counter [ebp-0x14]); the list has room for 200, and
       with exactly 200 the sentinel lands in the count's low byte (200 -> 255); the port's copy is bounded (G
       unreachable).
     - f4D518 passes its colour locals uninitialised to f5A084 when show == 0 (unused there); the port passes 0.
     - f5A750's display looks up the ship selection in the record of the ship's territory d, not of t2 = f3E8FC(i)
       where the counts were changed: for a ship on a second coast it can box another ship's territory (display only).
     - f5A084 / f5A2C4 / f5A4BC / f5A750 leave the phase byte 0xF4102 at 5, whatever it was (their callers are in
       MOVEMENT or the PBEM viewer).
     - The ship points (+0x41 + k) are set for all four slots at the start of a human's turn, whatever +0x40 says;
       f57BD8 and the source colour after a ship move (0x56BA1) also look at all four slots (0x78D98 zeroes a freed
       slot's word, so no stale slot matches).
     - Once nothing is left to move, FINISHED ends the turn without the confirmation (0x56F84..0x5701F); UNDO there
       re-enables MOVE TOKENS / MOVE SHIPS.
     - "Ship #%d has no movement points remaining" has no full stop (left: not in the ruling's list).
   Easter eggs / oddities: the trace tags "movement", "movement - MSStore", "TempTerr", "Tempnation" (M6_PLAN egg 14);
   the computer's REPLAY of its own turn (seed saved and restored around a World State push / pop, so the replay is
   the same turn again); the "PIRACY" panel label of bug 1. */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "fg.h"
#include "game.h"
#include "kit.h"
#include "map.h"
#include "mapdata.h"
#include "move.h"
#include "movephase.h"
#include "mstack.h"
#include "panel.h"
#include "shippanel.h"
#include "stack.h"
#include "start.h"
#include "state.h"
#include "ui.h"

/* ---- calls into code that is not ported yet (see the file comment) ----------------------------------------------- */

#ifdef MOVEPHASE_TEST
void movephase_test_f353C4(int n);
void movephase_test_f461BC(int n);
#define f353C4(n) movephase_test_f353C4(n)
#define f461BC(n) movephase_test_f461BC(n)
#else
#include "aicity.h"                             /* f353C4 the computer's trial planning (M7 batch 1) */
#include "aimove.h"                             /* f461BC the movement planner (M7 batch 1) */
#endif

#define FLAGS        0xE1624u
#define PBEM_BIT     0x80u
#define PBEM_LOCAL   0xE1622u   /* int8: dword [0xE161F] sar 24 */
#define TOGGLES      0xF097Cu
#define CUR_NATION   0xF40F8u
#define PANEL_DLG    0xF596Cu
#define MOVE_DLG     0xC4A3Cu
#define SHIP_DLG     0xC4E28u
#define AIEND_DLG    0xC59ECu
#define CUR_DLG      0xF8E78u
#define PLAYER_BASE  0xF5940u
#define BACKBUF      0xF52C8u
#define BGBUF        0xF52CCu
#define BG_NUM       0xEE168u   /* int8 (dword [0xEE165] sar 24) */
#define MODE         0x108804u  /* int8 (dword [0x108801] sar 24) */
#define COUNT        0xF09E0u   /* int16 (dword [0xF09DE] sar 16) */
#define PANEL_T      0x109E10u
#define CHOSEN       0xF06CCu
#define FIND_T       0x109E18u
#define FIND_S       0x109E1Cu
#define UNDO_KIND    0x108810u
#define UNDO_FROM    0x108808u
#define UNDO_TO      0x10880Cu
#define UNDO_FIRST   0x108814u
#define UNDO_COUNT   0x108818u
#define STEPS        0x10881Cu
#define START_T      0xC4990u   /* int32[10] */
#define START_SET    0x1087DCu  /* int32[10] */
#define TEMP_TERR    0xEED74u
#define TEMP_NATION  0xEED78u
#define SEED         0xCAC5Cu
#define MOUSE_X      0xE1628u
#define MOUSE_Y      0xE162Cu
#define MOUSE_B      0xE1630u
#define MSG          0xF5500u
#define LOG          0xF09B4u   /* uint32 the PBEM move log (a port handle) */
#define LOG_N        0xF09B8u   /* int16 (dword [0xF09B6] sar 16) */
#define LOG_NN       0xF09BAu   /* int16[12] */
#define LOG_LIST     0xF0984u   /* uint32[12] */
#define LIST1        0x107694u  /* 4-byte land move records */
#define LIST1_N      0x1079B4u  /* int32 */
#define LIST2        0x1079B8u  /* 6-byte ship move records */
#define LIST2_N      0x107D3Cu  /* int32 */
#define AREA_SIZE    0x2B65u
#define REC_SIZE     69u
#define NAT_SIZE     199u

#define NAME(n)      DSTR(D32(0xC768Cu + 4u * (uint32_t)(n)))
#define ADJ(n)       DSTR(D32(0xC76C4u + 4u * (uint32_t)(n)))

/* area record of territory t (record t + 1), field off + n: 69(t + 1) + n + 0xE31B8 + off (32-bit wrap as the code) */
static uint32_t ar(int32_t t, int32_t n, uint32_t off)
{
    return GAME_AREA + REC_SIZE * (uint32_t)(t + 1) + (uint32_t)n + off;
}

/* nation record n: 0xF14CC + 199n (imul reg, n, 0xC7) */
static uint32_t nrec(int32_t n)
{
    return GAME_NATION + NAT_SIZE * (uint32_t)n;
}

/* n's tokens in t that may still move: dword [69(t+1) + (n-1) + 0xE31DA] sar 24 = int8 +0x24 + n */
static int32_t mvbl(int32_t t, int32_t n)
{
    return (int32_t)S8(ar(t, n, 0x24u));
}

/* the colour of a token box: 5 when nothing may move from t any more, else 0x0F (the recurring "test eax; jne") */
static int box_color(int32_t t, int32_t n)
{
    return mvbl(t, n) == 0 ? 5 : 0x0F;
}

/* low byte of ship k's word (mov ax, word [rec + 2k + 0x45]; xor ah, ah; cwde / movsx: 0..255) */
static int32_t ship_lo(int32_t n, int32_t k)
{
    return (int32_t)D8(nrec(n) + 2u * (uint32_t)k + 0x45u);
}

/* Speedup AI Movement: word [0xF097C] shl 7 (32-bit), shr ax, 15: bit 8 of the word = bit 0 of byte 0xF097D */
static int speedup(void)
{
    return (D16(TOGGLES) >> 8) & 1;
}

/* the AI playback's last wait: [0xC65BC] (300) with Speedup, else [0xC65B0] (500) */
static int last_wait(void)
{
    return speedup() != 0 ? S32(0xC65BCu) : S32(0xC65B0u);
}

/* shown: PBEM or Show AI Movement (test byte [0xE1624], 0x80; jne; test byte [0xF097C], 0x40; je) */
static int ai_shown(void)
{
    return (D8(FLAGS) & PBEM_BIT) != 0 || (D8(TOGGLES) & 0x40) != 0;
}

/* a 16-bit list word at the handle h (the adjacency lists live in heap blocks, game.h): movsx word [eax] */
static int32_t hword(uint32_t h)
{
    const uint8_t *p = load_heap_ptr(h, 2);
    return p != NULL ? (int32_t)(int16_t)(p[0] | p[1] << 8) : 0;   /* port guard: an unresolved handle reads 0 */
}

/* the full-screen copy of the background into the back buffer (0x5580C..0x55834 and its two copies) */
static void bg_copy(void)
{
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(BGBUF), S32(BACKBUF));
}

/* ---- the PBEM move log (see the file comment; never reached in the port) ------------------------------------------ */

static uint32_t log_at(uint32_t off)            /* [0xF09B4] + 10 * int16 [0xF09B8] + off (lea [4*edx]; add; add) */
{
    return D32(LOG) + 10u * (uint32_t)(int32_t)S16(LOG_N) + off;
}

static void log_put(uint32_t off, uint8_t v)
{
    uint8_t *p = load_heap_ptr(log_at(off), 1);
    if (p != NULL)
        *p = v;
}

static int32_t log_s8(uint32_t off)
{
    const uint8_t *p = load_heap_ptr(log_at(off), 1);
    return p != NULL ? (int32_t)(int8_t)*p : 0;
}

/* 0x91CAC realloc(eax, edx) as the port's handles allow: a new block with the old block's first size bytes (copied
   only when the old block holds them all; both callers shrink the 10000-byte log), the old one freed; size 0 frees
   and gives 0 (G: Watcom's realloc) */
static uint32_t mp_realloc(uint32_t h, uint32_t size)
{
    uint32_t nh;
    const uint8_t *src;
    uint8_t *dst;

    if (size == 0) {
        if (h != 0)
            load_heap_free(h);
        return 0;
    }
    nh = load_heap_alloc(size);
    if (nh == 0)
        return 0;
    if (h != 0) {
        src = load_heap_ptr(h, size);
        dst = load_heap_ptr(nh, size);
        if (src != NULL && dst != NULL)
            memcpy(dst, src, size);
        load_heap_free(h);
    }
    return nh;
}

/* the FINISHED bookkeeping of PBEM (0x56077..0x560BF and 0x5702C..0x5706B): the log cut to its records and kept
   per nation, bit 0x01 of [0xE1624] set */
static void log_keep(int32_t n)
{
    uint32_t h = mp_realloc(D32(LOG), 10u * (uint32_t)(int32_t)S16(LOG_N));   /* 0x56090 realloc(log, 10 * count) */
    setD32(LOG, h);                                                 /* 0x56095 */
    setD32(LOG_LIST + 4u * (uint32_t)n, h);                         /* 0x5609D [0xF0984 + 4n] */
    setD16(LOG_NN + 2u * (uint32_t)n, D16(LOG_N));                  /* 0x560B7 word [0xF09BA + 2n] */
    setD8(FLAGS, (uint8_t)(D8(FLAGS) | 1u));                        /* 0x560BF or byte [0xE1624], 1 */
}

/* ---- the snapshots for RESTART ------------------------------------------------------------------------------------- */

/* area records 0..160 to TempTerr (0x558B8 / 0x5567D: rep movsd ecx 0x11, movsb per record) */
static void terr_save(void)
{
    int32_t r;
    uint8_t *p;

    for (r = 0; r < 0xA1; r++) {                                    /* 0x558B8 cmp [ebp-0x3C], 0xA1; jge */
        p = load_heap_ptr(D32(TEMP_TERR) + REC_SIZE * (uint32_t)r, REC_SIZE);
        if (p != NULL)                                              /* port guard: no block, no copy */
            memcpy(p, DP(GAME_AREA + REC_SIZE * (uint32_t)r), REC_SIZE);
    }
}

/* area records 0..160 back from TempTerr (0x571BD / 0x570A9) */
static void terr_restore(void)
{
    int32_t r;
    const uint8_t *p;

    for (r = 0; r < 0xA1; r++) {
        p = load_heap_ptr(D32(TEMP_TERR) + REC_SIZE * (uint32_t)r, REC_SIZE);
        if (p != NULL)
            memcpy(DP(GAME_AREA + REC_SIZE * (uint32_t)r), p, REC_SIZE);
    }
}

/* nation n's record to Tempnation (rep movsd ecx 0x31, movsw, movsb: 199 bytes) */
static void nation_save(int32_t n)
{
    uint8_t *p = load_heap_ptr(D32(TEMP_NATION), NAT_SIZE);
    if (p != NULL)
        memcpy(p, DP(nrec(n)), NAT_SIZE);
}

static void nation_restore(int32_t n)
{
    const uint8_t *p = load_heap_ptr(D32(TEMP_NATION), NAT_SIZE);
    if (p != NULL)
        memcpy(DP(nrec(n)), p, NAT_SIZE);
}

/* the panel set up for tokens (0x55846..0x558A5, again at 0x55B37 and 0x55C0B): the two dialogs appended, the ship
   panel of nation 0, land mode, MOVE TOKENS and count 1 pressed, the panel drawn */
static void panel_setup(void)
{
    f7853C(MOVE_DLG);                                               /* 0x5584B */
    f7853C(SHIP_DLG);                                               /* 0x55857 (edx = 0 is no argument) */
    f5D14C(0, 0);                                                   /* 0x55863 eax = 0, edx = 0 (0x55855) */
    setD8(MODE, 1);                                                 /* 0x5586F mov [0x108804], ah (1) */
    f3B914(PANEL_DLG, 0x26, 1);                                     /* 0x5587A ebx = 1 from 0x5585E */
    setD16(COUNT, 1);                                               /* 0x5588E mov word [0xF09E0], ax (1) */
    f3B914(PANEL_DLG, 0xC4, 1);                                     /* 0x55899 */
    f3B05C(PANEL_DLG);                                              /* 0x558A5 */
}

/* the wait for a click in the phase's own loop (0x55FF8..0x5601D / 0x56F8E..0x56FB3), then f3D434 on the panel */
static int32_t wait_click(void)
{
    int mx, my, mb;

    f775F0();                                                       /* 0x55FF8 */
    while (S32(MOUSE_B) == 0) {                                     /* 0x55FFD cmp dword [0xE1630], 0; jne */
        fg_mousepos(&mx, &my, &mb);                                 /* 0x56015 fg_mousepos(0xE1628, 0xE162C, 0xE1630) */
        setD32(MOUSE_X, (uint32_t)mx);
        setD32(MOUSE_Y, (uint32_t)my);
        setD32(MOUSE_B, (uint32_t)mb);
    }
    return f3D434(S32(MOUSE_X), S32(MOUSE_Y), PANEL_DLG);           /* 0x5602F (ebx = 0xF596C) */
}

/* ==== 0x55574: MOVEMENT =========================================================================================== */

void f55574(void)
{
    int32_t p;                 /* [ebp-0x48] index into the movement order */
    int32_t n = 0;             /* [ebp-0x44] the nation (port: 0 before the first) */
    uint32_t rec = 0;          /* [ebp-0x1C] its record */
    int32_t i;                 /* [ebp-0x3C] */
    int32_t code = 0;          /* [ebp-0x38] the click's code, or a territory */
    int32_t src = 0;           /* [ebp-0x18] the source territory */
    int32_t steps_max = 0;     /* [ebp-0x34] 1 + Roadbuilding */
    int32_t ship_pts;          /* [ebp-0x30] */
    int32_t color = 0;         /* [ebp-0x2C] */
    int32_t r;                 /* [ebp-0x28] */
    uint32_t seed = 0;         /* [ebp-0x20] */
    int32_t k;
    uint32_t a;
    char buf[100];             /* [ebp-0xAC]..[ebp-0x48] */

    setD16(0xBB3FAu, 0xA9);                                         /* 0x555A0 the message area */
    setD16(0xBB3FCu, 0x14E);                                        /* 0x555A7 */
    setD16(0xBB3FEu, 0x12C);                                        /* 0x555AE */
    setD16(0xBB400u, 0x22);                                         /* 0x555B5 */
    for (i = 0; i < 0x9D; i++) {                                    /* 0x555C9 cmp [ebp-0x3C], 0x9D; jge */
        a = GAME_TSTATE + 3u * (uint32_t)i + 2u;
        setD8(a, (uint8_t)(D8(a) & 0xFDu));                         /* 0x555DE and byte [3i + 0xE2C1A], -3 */
        if ((D8(a) & 1u) != 0)                                      /* 0x555F1..0x555FE a city: next */
            continue;
        setD8(GAME_TSTATE + 3u * (uint32_t)i, 0);                   /* 0x5560E +0 = 0 */
        setD8(GAME_TSTATE + 3u * (uint32_t)i + 1u, 0);              /* 0x55622 +1 = 0 */
    }
    setD32(TEMP_TERR, load_heap_alloc(AREA_SIZE));                  /* 0x55639 malloc(0x2B65) "TempTerr"; 0x5563E */
    setD32(TEMP_NATION, load_heap_alloc(NAT_SIZE));                 /* 0x5565C malloc(0xC7) "Tempnation"; 0x55661 */
    terr_save();                                                    /* 0x5567D..0x556BA */
    nation_save((int32_t)S8(PBEM_LOCAL));                           /* 0x556BC dword [0xE161F] sar 24; imul 0xC7 */

    /* the movement order: the census order, nations without Military (+0x35) first, then those with it */
    setD16(0xF06AEu, 0);                                            /* 0x556E4 */
    for (i = 0; (int32_t)S16(0xF0698u) > i; i++) {                  /* 0x556F6 dword [0xF0696] sar 16; jle */
        if (D8(nrec((int32_t)S16(0xF069Au + 2u * (uint32_t)i)) + 0x35u) != 0)   /* 0x55706..0x5571E */
            continue;
        k = (int32_t)S16(0xF06AEu);                                 /* 0x55723 mov ax, word [0xF06AE]; movsx */
        setD16(0xF06AEu, (uint16_t)(k + 1));                        /* 0x55733 inc eax; 0x5573C */
        setD16(0xF06B0u + 2u * (uint32_t)k, D16(0xF069Au + 2u * (uint32_t)i));   /* 0x55742 */
    }
    for (i = 0; (int32_t)S16(0xF0698u) > i; i++) {                  /* 0x55758 */
        if (D8(nrec((int32_t)S16(0xF069Au + 2u * (uint32_t)i)) + 0x35u) == 0)   /* 0x55768..0x55780 */
            continue;
        k = (int32_t)S16(0xF06AEu);
        setD16(0xF06AEu, (uint16_t)(k + 1));                        /* 0x5579E */
        setD16(0xF06B0u + 2u * (uint32_t)k, D16(0xF069Au + 2u * (uint32_t)i));   /* 0x557A4 */
    }
    f783B8(PANEL_DLG, S32(PLAYER_BASE));                            /* 0x557BD (ecx = 1 is no argument) */
    f7A8D0();                                                       /* 0x557C4 */
    setD32(0xF09E4u, 0);                                            /* 0x557C9 ebx = 0, kept across the call */
    for (i = 1; i <= 9; i++) {                                      /* 0x557CF ecx = 1 (0x557B8, kept); cmp 9; jg */
        if (D8(nrec(i) + 0x28u) == 0)                               /* 0x557E7 mov ch, [199i + 0xF14F4]; test */
            continue;
        setD32(0xF09E4u, 1);                                        /* 0x557F1 */
        break;
    }
    setD8(BG_NUM, 3);                                               /* 0x557FB */
    f3CD6C(3);                                                      /* 0x55807 */
    bg_copy();                                                      /* 0x5582F */
    f7876C();                                                       /* 0x55837 the phase name */
    fg_setcolor(5);                                                 /* 0x5583E */
    panel_setup();                                                  /* 0x55846..0x558A5 */
    p = 0;                                                          /* 0x558AA */
    goto L55C74;

L55C6E:
    p++;                                                            /* 0x55C71 inc dword [ebp-0x48] */
L55C74:
    if ((int32_t)S16(0xF06AEu) <= p)                                /* 0x55C74 dword [0xF06AC] sar 16; jle 0x57077 */
        goto L57077;
    n = (int32_t)S16(0xF06B0u + 2u * (uint32_t)p);                  /* 0x55C88 dword [2p + 0xF06AE] sar 16 */
    setD32(CUR_NATION, (uint32_t)n);                                /* 0x55C9F */
    rec = nrec(n);                                                  /* 0x55CA4..0x55CB2 */
    f3F6E4();                                                       /* 0x55CB5 token boxes cleared */
    f3F758();                                                       /* 0x55CBA ship boxes cleared */
    f3A924();                                                       /* 0x55CBF */
    f3C314(PANEL_DLG, 0x26);                                        /* 0x55CCE MOVE TOKENS enabled */
    f3C314(PANEL_DLG, 0x27);                                        /* 0x55CDD MOVE SHIPS enabled */
    if ((D8(FLAGS) & PBEM_BIT) == 0) {                              /* 0x55CE8 test dl, -0x80; jne 0x55CFD */
        if (D8(rec + 0x9Au) == 0)                                   /* 0x55CF4 cmp byte [199n + 0xF1566], 0; jne */
            goto L55C6E;                                            /* 0x55CFD..0x55D04 (no PBEM): next */
    } else if ((int32_t)S8(PBEM_LOCAL) != n) {                      /* 0x55D0A dword [0xE161F] sar 24; cmp; jne */
        goto L55C6E;
    }
    /* 0x55D1B -> 0x558B8: the snapshot for RESTART */
    terr_save();                                                    /* 0x558B8..0x558F5 */
    nation_save(n);                                                 /* 0x558F7..0x55913 */
    if ((D8(FLAGS) & PBEM_BIT) != 0 && (int32_t)S8(PBEM_LOCAL) == n)   /* 0x55914; 0x55925 */
        f3A9F4(g_original ? DSTR(0xB5AE4u)                          /* 0x5592F "NOTE: Tax assessments, ..." */
                          : "NOTE: Tax assessments, pop. expansions and ship maint./const. for the other players "
                            "are not shown.");                      /* BUG(1995) 3: the missing space after "const." */

L55934:
    setD32(FIND_T, 0);                                              /* 0x5593B */
    setD32(FIND_S, 0xFFFFFFFFu);                                    /* 0x55941 */
    if ((int32_t)S8(rec + 0x9Au) != 2)                              /* 0x5594E dword [199n + 0xF1563] sar 24; cmp 2 */
        goto L55D25;

    /* ---- a computer's turn (0x55960..0x55C69) ---- */
    seed = D32(SEED);                                               /* 0x55963 [ebp-0x20] = [0xCAC5C] */
    f7A940();                                                       /* 0x5596B World State push (for REPLAY) */
    f3F670();                                                       /* 0x55970 */
    f3A924();                                                       /* 0x55975 */
    f78884();                                                       /* 0x5597A */
    setD32(0xF5908u, 1);                                            /* 0x5598C */
    f3CD6C(S8(BG_NUM));                                             /* 0x55992 dword [0xEE165] sar 24 */
    bg_copy();                                                      /* 0x559BA */
    f7876C();                                                       /* 0x559C2 */
    f78664(MOVE_DLG);                                               /* 0x559CC */
    f78664(SHIP_DLG);                                               /* 0x559D6 */
    f3B05C(PANEL_DLG);                                              /* 0x559E0 */
    f3D9FC(n);                                                      /* 0x559E8 */
    f459E4(n);                                                      /* 0x559F0 */
    f3CBBC();                                                       /* 0x559F5 */
    f7844C(PANEL_DLG, S32(PLAYER_BASE));                            /* 0x55A05 */
    snprintf((char *)DP(MSG), 500, DSTR(0xB5B46u), NAME(n));        /* 0x55A1F "%s considers movement." */
    f79270(-550, n, -200, DSTR(MSG), 0);                            /* 0x55A3B */
    f3C828(D32(CUR_DLG));                                           /* 0x55A45 */
    f7A940();                                                       /* 0x55A4A push */
    f353C4(n);                                                      /* 0x55A52 [AICITY, not ported] */
    f7AC60();                                                       /* 0x55A57 pop: the trial undone */
    f461BC(n);                                                      /* 0x55A5F [AIMOVE, not ported]: the real moves */
    port_checkpoint(0x55A64u);                                      /* port (M7): the planner's return, a comparison point */
    setD32(0xF5908u, 0);                                            /* 0x55A6B */
    f3C7A0(D32(CUR_DLG));                                           /* 0x55A71 */
    f788AC();                                                       /* 0x55A76 */
    snprintf(buf, sizeof buf, DSTR(0xB5B5Du), NAME(n));             /* 0x55A92 "Movement for %s complete." */
    f79270(-550, n, -200, buf, 0);                                  /* 0x55AAE (push ebx = 0) */
    f7853C(AIEND_DLG);                                              /* 0x55AB8 REPLAY / CONTINUE */
    f3B05C(PANEL_DLG);                                              /* 0x55AC2 */
    f3CC14();                                                       /* 0x55AC7 */
    for (;;) {
        if (f7BA64() == 0) {                                        /* 0x55ACC; test eax; jne 0x55AE6 */
            r = f78BA4(PANEL_DLG, 0);                               /* 0x55ADC; 0x55AE1 [ebp-0x28] */
        } else {
            f78A20(S16(0xF40BAu) != 0 ? 1000 : 250);                /* 0x55AE6 cmp word [0xF40BA], 0; 0x55B01 */
            r = -1;                                                 /* 0x55AFC mov esi, -1; 0x55B06 */
        }
        if (r == -100)                                              /* 0x55B09 cmp [ebp-0x28], -0x64; je 0x55ACC */
            continue;
        if (r == -0x5E) {                                           /* 0x55B12 REPLAY */
            f78664(AIEND_DLG);                                      /* 0x55B20 */
            setD32(SEED, seed);                                     /* 0x55B28 the seed of the turn's start */
            f7AC60();                                               /* 0x55B2D pop: the board as before the turn */
            f3A990();                                               /* 0x55B32 */
            panel_setup();                                          /* 0x55B3E..0x55B95 */
            goto L55934;                                            /* 0x55B9A */
        }
        if (r == -1)                                                /* 0x55BA2 cmp edx, -1; jne 0x55ACC */
            break;
    }
    f78664(AIEND_DLG);                                              /* 0x55BB0 CONTINUE */
    f7AD24();                                                       /* 0x55BB5 World State drop */
    f3A990();                                                       /* 0x55BBA */
    f3CBBC();                                                       /* 0x55BBF */
    f3CD6C(S8(BG_NUM));                                             /* 0x55BCC */
    bg_copy();                                                      /* 0x55BF4 */
    f7876C();                                                       /* 0x55BFC */
    fg_setcolor(5);                                                 /* 0x55C03 */
    panel_setup();                                                  /* 0x55C0B..0x55C69 */
    goto L55C6E;

    /* ---- a human's turn (0x55D25..0x55F87) ---- */
L55D25:
    for (i = 1; i <= 0xA0; i++) {                                   /* 0x55D25 [ebp-0x3C] = 1; cmp 0xA0; jg 0x55DA5 */
        a = GAME_AREA + REC_SIZE * (uint32_t)i + (uint32_t)n;       /* 69i + (n - 1) + 1 */
        if (D8(a + 0x01u) == 0) {                                   /* 0x55D6B mov al, [ecx + 0xE31BA]; test al, al */
            setD8(a + 0x24u, 0);                                    /* 0x55D2E mov [ebx + 0xE31DD], al (0) */
            continue;
        }
        setD8(a + 0x24u, D8(a + 0x0Eu));                            /* 0x55D89 [ebx + 0xE31C7] -> [edx + 0xE31DD] */
        f3F698(i - 1, 0x0F);                                        /* 0x55D9E */
    }
    f7844C(PANEL_DLG, S32(PLAYER_BASE));                            /* 0x55DB0 */
    if (D8(rec + 0x40u) == 0) {                                     /* 0x55DC3 byte [199n + 0xF150C]; jne 0x55E1B */
        f3C3D0(PANEL_DLG, 0x27);                                    /* 0x55DDA MOVE SHIPS disabled */
        fg_vbopen(S32(BACKBUF));                                    /* 0x55DE6 */
        f3B7CC(PANEL_DLG, 0x26);                                    /* 0x55DF8 MOVE TOKENS pressed */
        setD8(MODE, 1);                                             /* 0x55E04 */
        f3B870(PANEL_DLG, 0x27);                                    /* 0x55E0F */
        fg_vbclose();                                               /* 0x55E14 */
    } else {
        f3C314(PANEL_DLG, 0x27);                                    /* 0x55E25 */
    }
    if ((D8(FLAGS) & PBEM_BIT) != 0) {                              /* 0x55E2A */
        for (i = 0; i < 0x0C; i++)                                  /* 0x55E40 */
            setD16(LOG_NN + 2u * (uint32_t)i, 0);                   /* 0x55E4B */
        setD32(LOG, load_heap_alloc(0x2710));                       /* 0x55E5A malloc(10000); 0x55E61 */
        setD16(LOG_N, 0);                                           /* 0x55E6B */
    }
    f459E4(n);                                                      /* 0x55E7A */
    setD32(START_SET + 4u * (uint32_t)n, 0xFFFFFFFFu);              /* 0x55E82 */
    ship_pts = (D8(rec + 0x25u) != 0) + 4;                          /* 0x55E90 cmp byte [rec + 0x25], 0; setne; add 4 */
    for (i = 0; i < 4; i++) {                                       /* 0x55EAF */
        setD8(rec + (uint32_t)i + 0x41u, (uint8_t)ship_pts);        /* 0x55EBE */
        if (ship_lo(n, i) == 0)                                     /* 0x55EC9 word; xor ah, ah; cwde; test */
            continue;
        f3F70C(ship_lo(n, i) - 1, 0x0F);                            /* 0x55EE9 */
    }
    f3F670();                                                       /* 0x55EF0 */
    code = S32(START_T + 4u * (uint32_t)n);                         /* 0x55EF8 [ebp-0x38] = 0xC4990[n] */
    if (code == -1 || (int32_t)S8(ar(code, n, 0x0Eu)) == 0)         /* 0x55F02; 0x55F1B dword [.. + 0xE31C4] sar 24 */
        code = f577B0(n);                                           /* 0x55F2B */
    if (g_original || code >= 0)
        f3F624(code, 0x0F);                                         /* 0x55F3B */
    /* BUG(1995) 1: with code == -1 (no movable token on the board) this writes 15 into byte 0xFFB5F, the top byte of
       the seaadj block pointer [0xFFB5C]; fixed: no highlight then */
    src = code;                                                     /* 0x55F43 */
    setD32(PANEL_T, (uint32_t)src);                                 /* 0x55F49 */
    f576F8(code);                                                   /* 0x55F51 */
    f5D14C(n, code);                                                /* 0x55F5C */
    f3C3D0(PANEL_DLG, 0x28);                                        /* 0x55F6B UNDO MOVE disabled */
    f572CC(code);                                                   /* 0x55F73 */
    if (g_original || code >= 0)
        f3DA60(code);                                               /* 0x55F7B */
    else
        f3D9FC(n);   /* BUG(1995) 1: f3DA60(-1) centres on territory 0 and labels it "PIRACY"; the home view instead */
    f3CBBC();                                                       /* 0x55F82 */
    setD32(CHOSEN, 0);                                              /* 0x55F87 */

    /* ---- the click loop ---- */
L55F8D:
    if (f5790C(n) == 0)                                             /* 0x55F90; je 0x56EDF */
        goto L56EDF;
    if ((int32_t)S8(MODE) == 1) {                                   /* 0x55F9D dword [0x108801] sar 24; cmp 1 */
        if (f57940(n) != 0)                                         /* 0x55FAD */
            f3C314(PANEL_DLG, 0x29);                                /* 0x55FC0 FIND UNMOVED enabled */
        else
            f3C3D0(PANEL_DLG, 0x29);                                /* 0x55FEE */
    } else {
        if (f5799C(n) != 0)                                         /* 0x55FCA */
            f3C314(PANEL_DLG, 0x29);                                /* 0x55FDD */
        else
            f3C3D0(PANEL_DLG, 0x29);                                /* 0x55FEE */
    }
    f3CBBC();                                                       /* 0x55FF3 */
    code = wait_click();                                            /* 0x55FF8..0x56034 */

L56037:
    f3CD6C(3);                                                      /* 0x5603E */
    setD8(BG_NUM, 3);                                               /* 0x56043 mov [0xEE168], bh (3) */
    if (code == -1) {                                               /* 0x56049 FINISHED */
        if (f57AFC(n) != 0) {                                       /* 0x56056 tokens left on a ship */
            f3A924();                                               /* 0x5605F */
            f3A9F4(DSTR(0xC7AA8u));                                 /* 0x56064 -> 0x56ED0 "There are still tokens left..." */
            f3CBBC();                                               /* 0x56ED5 */
            goto L55F8D;                                            /* 0x56EDA */
        }
        if ((D8(FLAGS) & PBEM_BIT) != 0)                            /* 0x5606E */
            log_keep(n);                                            /* 0x56077..0x560BF */
        if (f57254(n) != 0)                                         /* 0x560C8; jne 0x56EDF */
            goto L56EDF;
        goto L560D5;
    }
    if (code == -2) {                                               /* 0x5613D MOVE SHIPS / NEXT SHIP */
        f3F698(src, box_color(src, n));                             /* 0x56142..0x56171 */
        f3F70C(src, f576A8(src, n, 0x0F));                          /* 0x56181; 0x5618D */
        f3D52C(0, 0);                                               /* 0x56196 */
        goto L560D5;
    }
    if (code == -3) {                                               /* 0x561A3 MOVE TOKENS */
        f3F70C(src, f576A8(src, n, 0x0F));                          /* 0x561B3; 0x561BF */
        color = box_color(src, n);                                  /* 0x561C4..0x561F0 [ebp-0x2C] */
        f3F698(src, color);                                         /* 0x561F9 */
        f3D52C(0, 0);                                               /* 0x56202 */
        goto L560D5;
    }
    if (code == -0x63) {                                            /* 0x5620F RESTART */
        if (f57160(n) != 0)                                         /* 0x56217; jne 0x55934 */
            goto L55934;
        goto L560D5;
    }
    if (code == -0x79) {                                            /* 0x5622C FIND UNMOVED */
        code = f57810(n);                                           /* 0x56238; 0x5623D */
        if (code < 0)                                               /* 0x56243 test ebx, ebx; jl 0x560D5 */
            goto L560D5;
        setD32(CHOSEN, 0);                                          /* 0x5624D */
        for (i = 0; i < 0x9D; i++) {                                /* 0x56287 cmp 0x9D; jge 0x560D5 */
            f3F698(i, box_color(i, n));                             /* 0x56294..0x56260 */
            f3F70C(i, f576A8(i, n, 0x0F));                          /* 0x56270; 0x5627C */
        }
        goto L560D5;                                                /* the territory found, as a click on it */
    }
    if (code == -4) {                                               /* 0x562BE UNDO MOVE */
        src = S32(UNDO_FROM);                                       /* 0x562C7 [ebp-0x18] = [0x108808] */
        goto L55F8D;
    }

L560D5:
    if (code < 0)                                                   /* 0x560D5 cmp [ebp-0x38], 0; jl 0x55F8D */
        goto L55F8D;
    f3A924();                                                       /* 0x560DF */
    f3CBBC();                                                       /* 0x560E4 */
    if (S32(CHOSEN) == 0) {                                         /* 0x560E9 cmp [0xF06CC], 0; jne 0x56346 */
        f3A924();                                                   /* 0x560F6 */
        if (f57BD8(code, n) == 0) {                                 /* 0x56101; jne 0x562D4 */
            if (f3B9A4(PANEL_DLG, 0x26) != 1)                       /* 0x56118; 0x5611D cmp eax, 1; jne 0x56332 */
                goto L56332;
            f3A9F4(DSTR(0xC7B0Cu));                                 /* 0x5612B "You have no tokens in that territory." */
            f3CBBC();                                               /* 0x56130 */
            goto L55F8D;
        }
        if ((int32_t)S8(MODE) == 1 && mvbl(code, n) == 0) {         /* 0x562D4..0x56300 */
            f3A9F4(DSTR(0xC7B34u));                                 /* 0x56307 "There are no tokens with movement points left..." */
            f3CBBC();                                               /* 0x5630C */
            goto L55F8D;
        }
        if ((int32_t)S8(MODE) == 2 && f57A90(code, n) == 0)         /* 0x56316..0x56330 */
            goto L56332;
    }
    /* L56346 */
    k = (int32_t)S8(MODE);                                          /* 0x56346 dword [0x108801] sar 24 */
    if (k == 1)
        goto L56357;
    if (k != 2)                                                     /* 0x56968 cmp eax, 2; jne 0x55F8D */
        goto L55F8D;
    goto L56971;

L56332:
    f3A9F4(DSTR(0xC7B78u));                                         /* 0x56337 "There are no ships in that territory." */
    f3CBBC();                                                       /* 0x5633C */
    goto L55F8D;

    /* ---- tokens (0x56357..0x56963) ---- */
L56357:
    if (S32(CHOSEN) == 0) {                                         /* 0x5635C test eax; jne 0x56438: choose the source */
        f3F670();                                                   /* 0x56364 */
        f3F624(code, 0x0F);                                         /* 0x56371 */
        setD32(PANEL_T, (uint32_t)code);                            /* 0x56379 */
        src = code;                                                 /* 0x56381 */
        if (S32(START_SET + 4u * (uint32_t)n) == -1) {              /* 0x5638E cmp edx, -1; jne 0x563AC */
            setD32(START_SET + 4u * (uint32_t)n, 0);                /* 0x56398 */
            setD32(START_T + 4u * (uint32_t)n, (uint32_t)code);     /* 0x563A5 next turn's start territory */
        }
        setD32(STEPS, 0);                                           /* 0x563AE */
        f576F8(code);                                               /* 0x563B7 */
        f5D14C(n, code);                                            /* 0x563C2 */
        f3C3D0(PANEL_DLG, 0x28);                                    /* 0x563D1 */
        f572CC(src);                                                /* 0x563D9 */
        f3F70C(code, f576A8(code, n, 0x0F));                        /* 0x563E9; 0x563F5 */
        f3F698(code, 0x0E);                                         /* 0x56407 */
        setD32(CHOSEN, 1);                                          /* 0x5640C */
        steps_max = (D8(rec + 0x28u) != 0) + 1;                     /* 0x56415 cmp byte [rec + 0x28], 0; setne; add esi */
        f3DA60(code);                                               /* 0x56429 */
        f3CBBC();                                                   /* 0x5642E */
        goto L55F8D;
    }
    if (S32(CHOSEN) != 1)                                           /* 0x56438 cmp edi, 1; jne 0x55F8D */
        goto L55F8D;
    if (code == src) {                                              /* 0x5644D cmp eax, edx; jne 0x564B0: deselect */
        setD32(CHOSEN, 0);                                          /* 0x56453 */
        f3F698(code, box_color(code, n));                           /* 0x56459..0x56489 */
        f3D52C(0, 0);                                               /* 0x56492 */
        f3C3D0(PANEL_DLG, 0x28);                                    /* 0x564A1 */
        f3CBBC();                                                   /* 0x564A6 */
        goto L55F8D;
    }
    if (f582CC(src, code) <= 0) {                                   /* 0x564B6; test eax; jle 0x5694F */
        f3A924();                                                   /* 0x5694F */
        f3A9F4(DSTR(0xC7BA0u));                                     /* 0x56959 "You may not move your token into this territory." */
        f3CBBC();                                                   /* 0x5695E */
        goto L55F8D;
    }
    setD32(STEPS, D32(STEPS) + 1u);                                 /* 0x564C9 mov esi, [0x10881C]; inc esi; 0x564D5 */
    f57CE8(src, n, S32(STEPS) == 1, (int)S16(COUNT));               /* 0x564DB sete; 0x564C3 dword [0xF09DE] sar 16; 0x564E9 */
    f57E28(code, n, (int)S16(COUNT));                               /* 0x564EE..0x564FD */
    setD32(UNDO_KIND, 1);                                           /* 0x56502 */
    setD32(UNDO_FROM, (uint32_t)src);                               /* 0x5650F */
    setD32(UNDO_TO, (uint32_t)code);                                /* 0x5651D */
    setD32(UNDO_FIRST, S32(STEPS) == 1 ? 1u : 0u);                  /* 0x56522 cmp edx, 1; sete; 0x5652D */
    setD32(UNDO_COUNT, (uint32_t)(int32_t)S16(COUNT));              /* 0x56532..0x5653A */
    color = box_color(src, n);                                      /* 0x5653F..0x5656B [ebp-0x2C] */
    f3F698(src, color);                                             /* 0x56574 */
    f3F698(code, 0x0E);                                             /* 0x56581 */
    if ((D8(FLAGS) & PBEM_BIT) != 0) {                              /* 0x56586: the log record of a land step */
        log_put(0, D8(PANEL_T));                                    /* 0x565B5 +0 = byte [0x109E10] */
        log_put(1, D8(STEPS));                                      /* 0x565D7 +1 = byte [0x10881C] */
        log_put(2, (uint8_t)src);                                   /* 0x565F8 +2 = src */
        log_put(3, (uint8_t)color);                                 /* 0x5661A +3 = colour */
        log_put(4, D8(ar(src, n, 0x24u)));                          /* 0x56654 +4 = movable at src */
        log_put(5, (uint8_t)code);                                  /* 0x56676 +5 = dest */
        log_put(8, S32(STEPS) == 1 ? 1 : 6);                        /* 0x5667F cmp ecx, 1: 0x5669C 1, 0x566BB 6 */
        log_put(9, D8(COUNT));                                      /* 0x566E1 +9 = byte [0xF09E0] */
    }
    f3D52C(0, 0);                                                   /* 0x566E8 */
    if (S32(STEPS) < steps_max                                      /* 0x566F2 cmp eax, [ebp-0x34]; jge 0x56781 */
        && (int32_t)S8(ar(code, 0, 0x01u)) == 1                     /* 0x56706 dword [69(t+1) + 0xE31B6] sar 24: count */
        && ((D8(GAME_TSTATE + 3u * (uint32_t)code + 2u) & 1u) == 0  /* 0x5672E no city: go on */
            || (int32_t)D8(GAME_TSTATE + 3u * (uint32_t)code) == n)) {   /* 0x56765 its own city */
        /* 0x5676A: the stack may go on (Roadbuilding): the destination is the new source */
        color = 0x0E;                                               /* 0x5676A */
        src = code;                                                 /* 0x56774 */
        f574B0();                                                   /* 0x56777 */
    } else {
        if ((int32_t)S8(ar(code, 0, 0x01u)) != 1                    /* 0x56796 cmp eax, 1; jne 0x567D5 */
            || ((D8(GAME_TSTATE + 3u * (uint32_t)code + 2u) & 1u) == 1   /* 0x567B4; jne 0x567DD */
                && (int32_t)D8(GAME_TSTATE + 3u * (uint32_t)code) != n))  /* 0x567D0; je 0x567DD */
            setD32(CHOSEN, 0);                                      /* 0x567D7 */
        color = box_color(code, n);                                 /* 0x567DD..0x56809 [ebp-0x2C] */
        f78A20(0x190);                                              /* 0x56811 400 ms */
        f572CC(src);                                                /* 0x56819 */
    }
    /* L5681E */
    if ((D8(FLAGS) & PBEM_BIT) != 0) {                              /* 0x5681E */
        log_put(7, D8(ar(code, n, 0x24u)));                         /* 0x56862 +7 = movable at dest */
        log_put(6, (uint8_t)color);                                 /* 0x56884 +6 = colour */
        if (log_s8(8) == 6)                                         /* 0x5689F dword [.. + 5] sar 24: +8; cmp 6 */
            log_put(6, (uint8_t)box_color(code, n));                /* 0x568F2 */
        setD16(LOG_N, (uint16_t)(D16(LOG_N) + 1u));                 /* 0x568F6 inc word [0xF09B8] */
    }
    f3F698(code, color);                                            /* 0x56903 */
    f3D52C(0, 0);                                                   /* 0x5690C */
    f5D14C(n, S32(PANEL_T));                                        /* 0x5691A */
    f3C314(PANEL_DLG, 0x28);                                        /* 0x56929 UNDO MOVE enabled */
    f3CBBC();                                                       /* 0x5692E */
    if (S32(STEPS) == steps_max)                                    /* 0x5693B cmp eax, edi; jne 0x55F8D */
        setD32(CHOSEN, 0);                                          /* 0x56943 xor eax, edi (0); 0x56945 */
    goto L55F8D;

    /* ---- ships (0x56971..0x56EDA) ---- */
L56971:
    if (S32(CHOSEN) == 0) {                                         /* 0x56977 test edx; jne 0x56AC2: choose the ship */
        f3F670();                                                   /* 0x5697F */
        f3F624(code, 0x0F);                                         /* 0x5698C */
        setD32(PANEL_T, (uint32_t)code);                            /* 0x56994 */
        src = code;                                                 /* 0x5699C */
        f576F8(code);                                               /* 0x569A2 */
        f5D14C(n, code);                                            /* 0x569AD */
        f3C3D0(PANEL_DLG, 0x28);                                    /* 0x569BC */
        f3F70C(code, f576A8(code, n, 0x0E));                        /* 0x569CC; 0x569D8 */
        color = box_color(code, n);                                 /* 0x569DD..0x56A09 */
        f3F698(code, color);                                        /* 0x56A12 */
        k = (int32_t)D8(ar(src, n, 0x2Du));                         /* 0x56A2A the selected ship */
        if ((int32_t)D8(rec + (uint32_t)k + 0x41u) > 0) {           /* 0x56A38 byte [rec + k + 0x41]; jle 0x56A50 */
            setD32(CHOSEN, 1);                                      /* 0x56A44 */
        } else {
            k = (int32_t)D8(ar(src, n, 0x2Du));                     /* 0x56A63 */
            snprintf(buf, sizeof buf, DSTR(0xB5B8Au), k + 1);       /* 0x56A7C "Ship #%d has no movement points remaining" */
            f3F70C(code, f576A8(code, n, 0x0F));                    /* 0x56A8F; 0x56A9B */
            f3A924();                                               /* 0x56AA0 */
            f3A9F4(buf);                                            /* 0x56AAB */
        }
        f3DA60(code);                                               /* 0x56AB3 */
        f3CBBC();                                                   /* 0x56AB8 */
        goto L55F8D;
    }
    if (S32(CHOSEN) != 1)                                           /* 0x56AC2 cmp [0xF06CC], 1; jne 0x55F8D */
        goto L55F8D;
    if (code == src) {                                              /* 0x56AD5 cmp eax, ecx; jne 0x56B1F: deselect */
        setD32(CHOSEN, 0);                                          /* 0x56ADB */
        f3F70C(code, f576A8(code, n, 0x0F));                        /* 0x56AEC; 0x56AF8 */
        f3D52C(0, 0);                                               /* 0x56B01 */
        f3C3D0(PANEL_DLG, 0x28);                                    /* 0x56B10 */
        f3CBBC();                                                   /* 0x56B15 */
        goto L55F8D;
    }
    r = f58330(src, code, n);                                       /* 0x56B28; 0x56B2D [ebp-0x28] */
    if (r <= 0) {                                                   /* 0x56B33 test edi; jle 0x56EA5 */
        f3A924();                                                   /* 0x56EAD / 0x56EC6 */
        if (r == -2)                                                /* 0x56EA8 cmp ecx, -2; jne 0x56EC6 */
            f3A9F4(DSTR(0xC7BD4u));                                 /* 0x56EB7 "You may not move your ship into this territory." */
        else
            f3A9F4(DSTR(0xC7C04u));                                 /* 0x56ED0 "...into the open sea as its last move." */
        f3CBBC();                                                   /* 0x56EBC / 0x56ED5 */
        goto L55F8D;
    }
    f580CC(src, code, n);                                           /* 0x56B44 */
    setD32(UNDO_KIND, 2);                                           /* 0x56B49 */
    setD32(UNDO_FROM, (uint32_t)src);                               /* 0x56B56 */
    setD32(UNDO_TO, (uint32_t)code);                                /* 0x56B5E */
    k = (int32_t)D8(ar(code, n, 0x2Du));                            /* 0x56B76 the ship, now selected at the destination */
    a = rec + (uint32_t)k + 0x41u;
    setD8(a, (uint8_t)(D8(a) - 1u));                                /* 0x56B84 mov bh, [eax + 0x41]; dec bh; 0x56B8E */
    color = 5;                                                      /* 0x56B91 [ebp-0x2C] = 5 */
    for (i = 0; i < 4; i++) {                                       /* 0x56BA1 cmp 4; jge 0x56BD9 */
        if (ship_lo(n, i) != src + 1)                               /* 0x56BAF word; xor ah, ah; movsx; 0x56BBC cmp */
            continue;
        if ((int32_t)D8(rec + (uint32_t)i + 0x41u) <= 0)            /* 0x56BC6; jle 0x56B9B */
            continue;
        color = 0x0F;                                               /* 0x56BD2 another ship of n at src can still sail */
        break;
    }
    f576F8(src);                                                    /* 0x56BDC */
    f3F70C(src, color);                                             /* 0x56BE7 */
    f3F70C(code, 0x0E);                                             /* 0x56BF4 */
    if ((D8(FLAGS) & PBEM_BIT) != 0) {                              /* 0x56BF9: the log record of a ship move */
        log_put(0, D8(PANEL_T));                                    /* 0x56C27 +0 = byte [0x109E10] */
        log_put(2, (uint8_t)src);                                   /* 0x56C47 */
        log_put(3, (uint8_t)color);                                 /* 0x56C69 */
        log_put(4, D8(rec + (uint32_t)D8(ar(src, n, 0x2Du)) + 0x41u));   /* 0x56CAB +4 = points of src's selection */
        log_put(5, (uint8_t)code);                                  /* 0x56CCC */
        log_put(8, 2);                                              /* 0x56CE7 */
        log_put(9, D8(ar(src, n, 0x2Du)));                          /* 0x56D23 +9 = src's selection */
    }
    f3DA60(code);                                                   /* 0x56D29 */
    src = code;                                                     /* 0x56D31 */
    if ((int32_t)D8(rec + (uint32_t)D8(ar(code, n, 0x2Du)) + 0x41u) > 0) {   /* 0x56D55; jle 0x56D84 */
        color = 0x0E;                                               /* 0x56D61 */
        f3F670();                                                   /* 0x56D68 */
        f3F624(code, 0x0F);                                         /* 0x56D75 */
        setD32(PANEL_T, (uint32_t)code);                            /* 0x56D7D */
    } else {
        f3F670();                                                   /* 0x56D84 */
        f3F624(code, 0x0F);                                         /* 0x56D91 */
        f3DA60(code);                                               /* 0x56D99 */
        setD32(PANEL_T, (uint32_t)code);                            /* 0x56DA1 */
        color = f576A8(code, n, 0x0F);                              /* 0x56DB1; 0x56DB6 */
        f78A20(0x190);                                              /* 0x56DBE 400 ms */
    }
    if ((D8(FLAGS) & PBEM_BIT) != 0) {                              /* 0x56DC3 */
        log_put(6, (uint8_t)color);                                 /* 0x56DEB */
        log_put(7, D8(rec + (uint32_t)D8(ar(code, n, 0x2Du)) + 0x41u));   /* 0x56E30 +7 = points left */
        setD16(LOG_N, (uint16_t)(D16(LOG_N) + 1u));                 /* 0x56E33 */
    }
    f5D14C(n, src);                                                 /* 0x56E40 */
    f3F70C(code, color);                                            /* 0x56E4B */
    f3DA60(code);                                                   /* 0x56E53 */
    f3C314(PANEL_DLG, 0x28);                                        /* 0x56E62 UNDO MOVE enabled */
    f3CBBC();                                                       /* 0x56E67 */
    if (D8(rec + (uint32_t)D8(ar(code, n, 0x2Du)) + 0x41u) == 0)    /* 0x56E8D; and eax, 0xFF; jne 0x55F8D */
        setD32(CHOSEN, 0);                                          /* 0x56E9B mov [0xF06CC], eax (0) */
    goto L55F8D;

    /* ---- nothing left to move, or FINISHED confirmed (0x56EDF..0x57072) ---- */
L56EDF:
    if (code == -1)                                                 /* 0x56EDF cmp [ebp-0x38], -1; je 0x55C6E */
        goto L55C6E;
    f3A924();                                                       /* 0x56EE9 */
    f3A9F4(DSTR(0xC7C44u));                                         /* 0x56EF8 "You have no more tokens (or ships) to move.  Press FINISHED." */
    f3C3D0(PANEL_DLG, 0x26);                                        /* 0x56F04 (edx = 0x26 kept across f3A9F4) */
    f3BFBC(PANEL_DLG, 0xC4, 0);                                     /* 0x56F13 (ebx = 0) */
    f3BFBC(PANEL_DLG, 0xC5, 0);                                     /* 0x56F24 */
    f3BFBC(PANEL_DLG, 0xC6, 0);                                     /* 0x56F35 */
    f3BFBC(PANEL_DLG, 0xC7, 0);                                     /* 0x56F46 */
    f3BFBC(PANEL_DLG, 0xC8, 0);                                     /* 0x56F57 */
    f3C3D0(PANEL_DLG, 0x27);                                        /* 0x56F66 */
    f3C3D0(PANEL_DLG, 0x29);                                        /* 0x56F77 */
    f3CBBC();                                                       /* 0x56F7C */
    code = 0;                                                       /* 0x56F81 */
    while (code != -1) {                                            /* 0x56F84 cmp [ebp-0x38], -1; je 0x5701F */
        code = wait_click();                                        /* 0x56F8E..0x56FCA (poll 0x56F93) */
        if (code == -4) {                                           /* 0x56FCD UNDO MOVE */
            f3A924();                                               /* 0x56FD3 */
            f3C314(PANEL_DLG, 0x26);                                /* 0x56FE2 */
            f3C314(PANEL_DLG, 0x27);                                /* 0x56FF1 */
            f3CBBC();                                               /* 0x56FF6 */
            goto L56037;                                            /* 0x56FFB */
        }
        if (code == -0x63 && f57160(n) != 0)                        /* 0x57000; 0x5700D; jne 0x55934 */
            goto L55934;
    }
    if ((D8(FLAGS) & PBEM_BIT) != 0)                                /* 0x5701F */
        log_keep(n);                                                /* 0x5702C..0x5706B */
    goto L55C6E;

    /* ---- the end of the phase (0x57077..0x5714D) ---- */
L57077:
    setD8(MODE, 0);                                                 /* 0x5707E mov [0x108804], dl (0) */
    f78664(MOVE_DLG);                                               /* 0x57084 */
    f78664(SHIP_DLG);                                               /* 0x5708E */
    if ((D8(FLAGS) & PBEM_BIT) != 0) {                              /* 0x57093: the local player's records back */
        terr_restore();                                             /* 0x570A9..0x570E2 */
        nation_restore((int32_t)S8(PBEM_LOCAL));                    /* 0x570E4..0x57107 */
    }
    f7ABD0();                                                       /* 0x57108 the World State Stack freed */
    load_heap_free(D32(TEMP_TERR));                                 /* 0x5711C free */
    setD32(TEMP_TERR, 0);                                           /* 0x57134 */
    load_heap_free(D32(TEMP_NATION));                               /* 0x5713A */
    setD32(TEMP_NATION, 0);                                         /* 0x5714D */
}

/* ==== the helpers ================================================================================================ */

/* 0x57160: RESTART for nation n (eax) */
int f57160(int n)
{
    char buf[100];                                                  /* [ebp-0x80]..[ebp-0x1C] */

    snprintf(buf, sizeof buf, DSTR(0xB5BDAu), ADJ(n));              /* 0x5718C "Restart %s Movement?" */
    if ((D8(TOGGLES) & 0x08u) != 0 && f795A8(buf) != -0x61)         /* 0x5719A test ah, 8; 0x571A2; cmp -0x61; jne */
        return 0;                                                   /* 0x57246 */
    terr_restore();                                                 /* 0x571B0..0x571F6 records 0..160 */
    nation_restore(n);                                              /* 0x571F8..0x57214 */
    f3F6E4();                                                       /* 0x57215 */
    f3F758();                                                       /* 0x5721A */
    f3A924();                                                       /* 0x5721F */
    if ((D8(FLAGS) & PBEM_BIT) != 0)                                /* 0x57224 */
        load_heap_free(D32(LOG));                                   /* 0x57232 free([0xF09B4]); the field is not cleared */
    return 1;                                                       /* 0x57237 */
}

/* 0x57254: the FINISHED confirmation for nation n (eax) */
int f57254(int n)
{
    char buf[100];                                                  /* [ebp-0x6C]..[ebp-0x8] */

    snprintf(buf, sizeof buf, DSTR(0xB5BEFu), NAME(n));             /* 0x5727C "Are you sure that you are Finished movement for %s?" */
    if ((D8(FLAGS) & PBEM_BIT) != 0)                                /* 0x5728A test ah, -0x80; je 0x572A3 */
        return f795A8(buf) == -0x61 ? 1 : 0;                        /* 0x57292; 0x5729A */
    if ((D8(TOGGLES) & 0x10u) != 0 && f795A8(buf) != -0x61)         /* 0x572A9 test dl, 0x10; 0x572B1; jne 0x572C2 */
        return 0;
    return 1;                                                       /* 0x572BB */
}

/* 0x574B0: the count buttons for the stack's next step: enabled up to int16 [0xF09E0], that one pressed */
void f574B0(void)
{
    int id;

    for (id = 0xC4; id <= 0xC8; id++)                               /* 0x574C0..0x575A6, five unrolled tests */
        f3BFBC(PANEL_DLG, id, (int32_t)S16(COUNT) >= id - 0xC3 ? 1 : 0);   /* dword [0xF09DE] sar 16; cmp i; jl */
    fg_vbopen(S32(BACKBUF));                                        /* 0x575B2 */
    for (id = 0xC4; id <= 0xC8; id++) {                             /* 0x575B7..0x5769B */
        if ((int32_t)S16(COUNT) == id - 0xC3)                       /* cmp eax, i; jne */
            f3B7CC(PANEL_DLG, id);                                  /* pressed */
        else
            f3B870(PANEL_DLG, id);                                  /* up */
    }
    fg_vbclose();                                                   /* 0x5769B */
}

/* 0x577B0: nation n's first territory with movable tokens */
int f577B0(int n)
{
    int32_t r;                                                      /* [ebp-0x4] */

    for (r = 1; r < 0x9E; r++)                                      /* 0x577C4; 0x577D3 cmp 0x9E; jge 0x57804 */
        if ((int32_t)S8(GAME_AREA + REC_SIZE * (uint32_t)r + (uint32_t)n + 0x24u) != 0)   /* 0x577EE [.. + 0xE31D9] sar 24 */
            return r - 1;                                           /* 0x577FE */
    return -1;                                                      /* 0x57804 */
}

/* 0x57810: FIND UNMOVED for nation n */
int f57810(int n)
{
    int32_t r;                                                      /* [ebp-0x4] */
    int32_t k;
    int wrapped = 0;                                                /* port: the BUG(1995) 2 guard */

    if ((int32_t)S8(MODE) == 1) {                                   /* 0x57825 dword [0x108801] sar 24; cmp 1 */
        for (;;) {
            for (r = S32(FIND_T) + 1; r < 0x9E; r++) {              /* 0x57832..0x5784A cmp 0x9E; jge 0x57879 */
                if (D8(GAME_AREA + REC_SIZE * (uint32_t)r + (uint32_t)n + 0x24u) == 0)   /* 0x5785E [.. + 0xE31DC] */
                    continue;
                setD32(FIND_T, (uint32_t)r);                        /* 0x5786B */
                return r - 1;                                       /* 0x57873 */
            }
            setD32(FIND_T, 0);                                      /* 0x5787B and scan again */
            if (!g_original && wrapped++ != 0)
                return -1;  /* BUG(1995) 2: the original rescans for ever when no territory qualifies */
        }
    }
    for (;;) {
        for (k = S32(FIND_S) + 1; k < 4; k++) {                     /* 0x57883..0x57898 cmp 4; jge 0x578F0 */
            if (ship_lo(n, k) == 0)                                 /* 0x578A4 word [199n + 2k + 0xF1511]; xor ah, ah */
                continue;
            if ((int32_t)D8(nrec(n) + (uint32_t)k + 0x41u) <= 0)    /* 0x578BD byte [.. + 0xF150D]; jle */
                continue;
            setD32(FIND_S, (uint32_t)k);                            /* 0x578CF */
            return ship_lo(n, k) - 1;                               /* 0x578DE..0x578E9 */
        }
        setD32(FIND_S, 0xFFFFFFFFu);                                /* 0x578F0 and scan again from k = 0 */
        if (!g_original && wrapped++ != 0)
            return -1;      /* BUG(1995) 2: the original rescans for ever when no ship qualifies (tokens aboard, no
                               points: f5799C keeps the button enabled) */
    }
}

/* 0x5790C: anything left to move for nation n */
int f5790C(int n)
{
    if (f57940(n) != 0)                                             /* 0x57922; jne 0x57937 */
        return 1;
    return f5799C(n) != 0 ? 1 : 0;                                  /* 0x5792E; je 0x5793C (eax 0) */
}

/* 0x57940: 1 if nation n has movable tokens in a record 1..157 */
int f57940(int n)
{
    int32_t r;

    for (r = 1; r < 0x9E; r++)                                      /* 0x57954; 0x57963 cmp 0x9E; jge 0x57992 */
        if (D8(GAME_AREA + REC_SIZE * (uint32_t)r + (uint32_t)n + 0x24u) != 0)   /* 0x5797E [.. + 0xE31DC]; test dl */
            return 1;
    return 0;
}

/* 0x5799C: 1 if a ship of nation n has points left or tokens aboard */
int f5799C(int n)
{
    uint32_t rec = nrec(n);                                         /* [ebp-0x4] */
    int32_t k;                                                      /* [ebp-0x8] */

    for (k = 0; (int32_t)D8(rec + 0x40u) > k; k++) {                /* 0x579D2 byte & 0xFF; cmp; jle 0x57A08 */
        if ((int32_t)D8(rec + (uint32_t)k + 0x41u) > 0)             /* 0x579E5; test; jg 0x57A01 */
            return 1;
        if ((int32_t)S8(rec + (uint32_t)k + 0x4Du) != 0)            /* 0x579F7 dword [rec + k + 0x4A] sar 24 */
            return 1;
    }
    return 0;
}

/* 0x57A90: 1 if one of nation n's ships stands at t (the whole int16 word) */
int f57A90(int t, int n)
{
    uint32_t rec = nrec(n);
    int32_t k;

    for (k = 0; (int32_t)D8(rec + 0x40u) > k; k++)                  /* 0x57AC8; jle 0x57AF5 */
        if ((int32_t)S16(rec + 2u * (uint32_t)k + 0x45u) == t + 1)  /* 0x57ADD dword [rec + 2k + 0x43] sar 16; cmp */
            return 1;
    return 0;
}

/* 0x57AFC: 1 if one of nation n's ships has tokens aboard */
int f57AFC(int n)
{
    uint32_t rec = nrec(n);
    int32_t k;

    for (k = 0; (int32_t)D8(rec + 0x40u) > k; k++)                  /* 0x57B32; jle 0x57B53 */
        if (D8(rec + (uint32_t)k + 0x4Du) != 0)                     /* 0x57B45 mov dl, [eax + 0x4D]; test dl */
            return 1;
    return 0;
}

/* 0x57BD8: a first click on territory t valid for nation n in the current mode */
int f57BD8(int t, int n)
{
    int32_t i;
    int32_t mode = (int32_t)S8(MODE);                               /* 0x57BF0 dword [0x108801] sar 24 */

    if (mode == 1) {
        if (D8(ar(t, 0, 0x01u)) == 0)                               /* 0x57C09 byte [69(t+1) + 0xE31B9]; test bl */
            return 0;
        for (i = 0; i < 9; i++)                                     /* 0x57C25 cmp 9; jge 0x57C9B */
            if ((int32_t)S8(ar(t, i, 0x02u)) == n)                  /* 0x57C42 dword [.. + i + 0xE31B7] sar 24 */
                return 1;
        return 0;
    }
    if (mode != 2)                                                  /* 0x57C59 cmp eax, 2; jne 0x57C9B */
        return 0;
    for (i = 0; i < 4; i++)                                         /* 0x57C6B cmp 4: all four slots */
        if (ship_lo(n, i) == t + 1)                                 /* 0x57C7B word; xor ah, ah; movsx; cmp */
            return 1;
    return 0;
}

/* 0x582CC: 1 if u is in territory t's land list, else -2 */
int f582CC(int t, int u)
{
    int32_t r = -2;                                                 /* [ebp-0x4] */
    uint32_t list = D32(GAME_LANDADJ_PTR + 4u * (uint32_t)t);       /* 0x582EC [ebp-0x8] */
    int32_t i;

    for (i = 1; hword(list) >= i; i++) {                            /* 0x58305 movsx eax, word [eax]; cmp; jl */
        if (hword(list + 2u * (uint32_t)i) == u) {                  /* 0x58318; 0x5831E cmp; jne */
            r = 1;                                                  /* 0x58322 */
            break;
        }
    }
    return r;
}

/* 0x58330: can nation n's selected ship at t sail to u (eax t, edx u, ebx n) */
int f58330(int t, int u, int n)
{
    int32_t r = -2;                                                 /* [ebp-0x4] */
    uint32_t coast = D32(GAME_COASTADJ_PTR + 4u * (uint32_t)t);     /* 0x58355 [ebp-0x8] */
    uint32_t list;                                                  /* [ebp-0xC] */
    int32_t i;                                                      /* [ebp-0x10] */
    int32_t k;

    if (D8(nrec(n) + 0x2Bu) != 0)                                   /* 0x58369 Astronomy */
        list = D32(GAME_SEAADJ_PTR + 4u * (uint32_t)t);             /* 0x58376 */
    else
        list = D32(GAME_COASTADJ_PTR + 4u * (uint32_t)t);           /* 0x5837E */
    k = (int32_t)D8(ar(t, n, 0x2Du));                               /* 0x5839C the selected ship */
    if ((int32_t)D8(nrec(n) + (uint32_t)k + 0x41u) > 1) {           /* 0x583A9; cmp eax, 1; jle 0x583E8 */
        for (i = 1; hword(list) >= i; i++)                          /* 0x583C9; jl 0x58433 */
            if (hword(list + 2u * (uint32_t)i) == u)                /* 0x583DC; jne */
                return 1;                                           /* 0x583E6 -> 0x5842C */
        return r;
    }
    if (D8(nrec(n) + 0x2Bu) != 0)                                   /* 0x583EF mov bh, [.. + 0xF14F7]; test */
        r = -3;                                                     /* 0x583F9 */
    for (i = 1; hword(coast) >= i; i++)                             /* 0x5840F; jl 0x58433 */
        if (hword(coast + 2u * (uint32_t)i) == u)                   /* 0x58422; jne */
            return 1;                                               /* 0x5842C */
    return r;
}

/* ==== the computers' movement playback ============================================================================= */

/* 0x4D518: plays back nation n's planned moves (eax n, edx show) */
void f4D518(int n, int show)
{
    char buf[100];                                                  /* [ebp-0xA4]..[ebp-0x40] */
    int32_t h, y;                                                   /* [ebp-0x38], [ebp-0x30] */
    int32_t i;                                                      /* [ebp-0x3C] */
    int32_t from, to, c;                                            /* [ebp-0x2C], [ebp-0x28], [ebp-0x24] */
    int32_t fa, fb;                                                 /* [ebp-0x20], [ebp-0x1C] */
    int32_t cfrom = 0, cto = 0;   /* [ebp-0x18], [ebp-0x14]: uninitialised in the original when show == 0 (unused then) */

    if (show != 0 && (D8(TOGGLES) & 0x40u) != 0) {                  /* 0x4D535; 0x4D53F */
        snprintf(buf, sizeof buf, DSTR(0xB4734u), NAME(n));         /* 0x4D55F "%s is moving." */
        f3A990();                                                   /* 0x4D567 */
        h = (int32_t)S16(0xBB400u);                                 /* 0x4D56C dword [0xBB3FE] sar 16 */
        y = (int32_t)S16(0xBB3FCu);                                 /* 0x4D577 dword [0xBB3FA] sar 16 */
        setD16(0xBB3FCu, 0x14C);                                    /* 0x4D592 */
        setD16(0xBB400u, 0x7B);                                     /* 0x4D599 */
        f3A9F4(buf);                                                /* 0x4D5A0 */
        setD16(0xBB400u, (uint16_t)h);                              /* 0x4D5A8 */
        setD16(0xBB3FCu, (uint16_t)y);                              /* 0x4D5B1 */
        f3CBBC();                                                   /* 0x4D5B7 */
    }
    if (S32(LIST1_N) > 0) {                                         /* 0x4D5BC cmp [0x1079B4], 0; jle 0x4D77D */
        setD8(MODE, 1);                                             /* 0x4D5C9 */
        if (show != 0)                                              /* 0x4D5D3 */
            f4D82C();                                               /* 0x4D5D7 */
        for (i = 0; i < S32(LIST1_N); i++) {                        /* 0x4D719 cmp eax, [0x1079B4]; jge 0x4D7B0 */
            from = (int32_t)D8(LIST1 + 4u * (uint32_t)i);           /* 0x4D72D */
            if (from == 0xFF)                                       /* 0x4D73A */
                continue;
            to = (int32_t)D8(LIST1 + 4u * (uint32_t)i + 1u);        /* 0x4D747 */
            fa = 0;                                                 /* 0x4D753 */
            fb = 0;                                                 /* 0x4D756 */
            if (from == 0xC8) {                                     /* 0x4D75C a marker record */
                setD8(GAME_TSTATE + 3u * (uint32_t)to + 2u,
                      (uint8_t)(D8(GAME_TSTATE + 3u * (uint32_t)to + 2u) | 2u));   /* 0x4D774 or byte [3to + 0xE2C1A], 2 */
                continue;
            }
            if (from == 0xC9)                                       /* 0x4D5E9 cmp eax, 0xC9; je 0x4D713 */
                continue;
            c = (int32_t)S8(LIST1 + 4u * (uint32_t)i + 2u);         /* 0x4D5F7 dword [4i + 0x107693] sar 24 */
            if (c > 0x4B) {                                         /* 0x4D607 cmp edx, 0x4B; jle 0x4D61D */
                fa = 1;                                             /* 0x4D611 */
                fb = 1;                                             /* 0x4D614 */
                c -= 0x4B;                                          /* 0x4D617 */
            } else if (c > 0x32) {                                  /* 0x4D620 cmp edi, 0x32; jle 0x4D630 */
                fa = 1;                                             /* 0x4D625 */
                c -= 0x32;                                          /* 0x4D62C */
            }
            if (show != 0) {                                        /* 0x4D630; je 0x4D6E6 */
                if ((D8(TOGGLES) & 0x40u) != 0) {                   /* 0x4D640 test bl, 0x40; je 0x4D66C */
                    f3F670();                                       /* 0x4D645 */
                    f3F624(from, 0x0F);                             /* 0x4D652 */
                    f3DA60(from);                                   /* 0x4D65A */
                    f459E4(n);                                      /* 0x4D662 */
                    f3CBBC();                                       /* 0x4D667 */
                }
                cfrom = mvbl(from, n) - c == 0 ? 5 : 0x0F;          /* 0x4D67F sar 24; sub eax, [ebp-0x24]; jne */
                if (fa != 0 && fb == 0)                             /* 0x4D69C..0x4D6AC */
                    cto = 0x0E;                                     /* 0x4D6DF */
                else
                    cto = box_color(to, n);                         /* 0x4D6AE..0x4D6DA */
            }
            f5A084(5, n, from, 1, from, cfrom, to, cto, c, show);   /* 0x4D6E6..0x4D70E */
        }
    } else if (S32(LIST2_N) == 0 && show != 0 && (D8(TOGGLES) & 0x40u) != 0) {   /* 0x4D77D..0x4D797 */
        f3D9FC(n);                                                  /* 0x4D79C nothing to move: the home view */
        f3CBBC();                                                   /* 0x4D7A1 */
        f78A20(0x1F4);                                              /* 0x4D7AB 500 ms */
    }
    if (show != 0 && (D8(TOGGLES) & 0x40u) != 0 && speedup() == 0)  /* 0x4D7B0..0x4D7D3 */
        f78A20(0x12C);                                              /* 0x4D7DA 300 ms */
    if (S32(LIST2_N) > 0)                                           /* 0x4D7DF cmp [0x107D3C], 0; jle */
        f4DACC(n, show);                                            /* 0x4D7EE */
    if (show != 0 && (D8(TOGGLES) & 0x40u) != 0 && speedup() == 0)  /* 0x4D7F3..0x4D816 */
        f78A20(0x12C);                                              /* 0x4D81D 300 ms */
}

/* 0x4D82C: the land move list reordered for display */
void f4D82C(void)
{
    uint32_t local[0x400];       /* [ebp-0x334]: 200 dwords in the original, unbounded (see the file comment) */
    int32_t cnt;                 /* [ebp-0x8] */
    int32_t m;                   /* [ebp-0x4] */
    int32_t i;                   /* [ebp-0x14] */
    int32_t j;                   /* [ebp-0x10] */
    uint32_t tmp;                /* [ebp-0xC] */
    uint32_t a;

    setD8(LIST1 + 4u * D32(LIST1_N), 0xFF);                         /* 0x4D848 the sentinel: from of record [count] */
    for (i = 0; i < S32(LIST1_N); i++)                              /* 0x4D85D mov ecx, [0x1079B4]; cmp; jge */
        if (i < (int32_t)(sizeof local / sizeof local[0]))          /* port guard (G unreachable) */
            local[i] = D32(LIST1 + 4u * (uint32_t)i);               /* 0x4D877 */
    cnt = S32(LIST1_N);                                             /* 0x4D880 [ebp-0x8] = ecx */
    if (cnt > (int32_t)(sizeof local / sizeof local[0]))
        cnt = (int32_t)(sizeof local / sizeof local[0]);            /* port guard */
    setD32(LIST1_N, 0);                                             /* 0x4D885 */
    for (i = 0; i < cnt; i++) {                                     /* 0x4D896: counts < 50 first */
        if ((int32_t)(int8_t)(uint8_t)(local[i] >> 16) >= 0x32)     /* 0x4D8A1 dword [.. - 0x335] sar 24; cmp 0x32; jge */
            continue;
        a = LIST1 + 4u * D32(LIST1_N);                              /* 0x4D8B8 lea edx, [4*eax] */
        setD32(LIST1_N, D32(LIST1_N) + 1u);                         /* 0x4D8C0 */
        setD32(a, local[i]);                                        /* 0x4D8CC */
    }
    m = S32(LIST1_N);                                               /* 0x4D8D9 [ebp-0x4] */
    for (i = 0; i < cnt; i++) {                                     /* 0x4D8E9: then the others */
        if ((int32_t)(int8_t)(uint8_t)(local[i] >> 16) < 0x32)      /* 0x4D8F4; jl */
            continue;
        a = LIST1 + 4u * D32(LIST1_N);
        setD32(LIST1_N, D32(LIST1_N) + 1u);                         /* 0x4D913 */
        setD32(a, local[i]);                                        /* 0x4D91F */
    }
    for (i = 1; i < m; i++)                                         /* 0x4D927..0x4D93C: bubble by to (+1) */
        for (j = m - 1; j >= i; j--)                                /* 0x4D93E; 0x4D94D cmp; jl */
            if (D8(LIST1 + 4u * (uint32_t)j - 3u) > D8(LIST1 + 4u * (uint32_t)j + 1u)) {   /* 0x4D95B [4j + 0x107691] vs [4j + 0x107695]; jbe */
                tmp = D32(LIST1 + 4u * (uint32_t)j - 4u);           /* 0x4D970 */
                setD32(LIST1 + 4u * (uint32_t)j - 4u, D32(LIST1 + 4u * (uint32_t)j));   /* 0x4D987 */
                setD32(LIST1 + 4u * (uint32_t)j, tmp);              /* 0x4D994 */
            }
    for (i = 1; i < m; i++)                                         /* 0x4D99D..0x4D9B2: bubble by from (+0) */
        for (j = m - 1; j >= i; j--)
            if (D8(LIST1 + 4u * (uint32_t)j - 4u) > D8(LIST1 + 4u * (uint32_t)j)) {   /* 0x4D9D1 / 0x4D9D8; jbe */
                tmp = D32(LIST1 + 4u * (uint32_t)j - 4u);           /* 0x4D9E6 */
                setD32(LIST1 + 4u * (uint32_t)j - 4u, D32(LIST1 + 4u * (uint32_t)j));   /* 0x4D9FD */
                setD32(LIST1 + 4u * (uint32_t)j, tmp);              /* 0x4DA0A */
            }
    for (i = 0; m - 1 > i; i++) {                                   /* 0x4DA43 mov eax, [ebp-0x4]; dec; cmp; jle */
        if ((int32_t)D8(LIST1 + 4u * (uint32_t)i) > 0xA0)           /* 0x4DA5B and 0xFF; cmp 0xA0; jg */
            continue;
        j = i;                                                      /* 0x4DA65 */
        while (D8(LIST1 + 4u * (uint32_t)i) == D8(LIST1 + 4u * (uint32_t)j + 4u)              /* 0x4DA75 [4j + 0x107698] */
               && D8(LIST1 + 4u * (uint32_t)i + 1u) == D8(LIST1 + 4u * (uint32_t)j + 5u)) {   /* 0x4DA8B [4j + 0x107699] */
            j++;                                                    /* 0x4DA94 */
            setD8(LIST1 + 4u * (uint32_t)j, 0xFF);                  /* 0x4DA9A the merged record skipped */
        }
        if (j != i)                                                 /* 0x4DA1D cmp; je 0x4DA3D */
            setD8(LIST1 + 4u * (uint32_t)i + 2u,                    /* 0x4DA36 add bl, dl (dl = j - i, bytes) */
                  (uint8_t)(D8(LIST1 + 4u * (uint32_t)i + 2u) + (uint8_t)((uint8_t)j - (uint8_t)i)));
    }
}

/* 0x4DACC: plays back nation n's ship move list (eax n, edx show) */
void f4DACC(int n, int show)
{
    uint32_t rec = nrec(n);                                         /* [ebp-0xC] */
    int32_t i;                                                      /* [ebp-0x24] */
    int32_t from, to;                                               /* [ebp-0x20], [ebp-0x1C] */
    int32_t k, cnt;
    uint32_t r6;
    int type;

    setD8(MODE, 2);                                                 /* 0x4DB00 mov [0x108804], ah (2) */
    for (i = 0; i < S32(LIST2_N); i++) {                            /* 0x4DB6A cmp eax, [0x107D3C]; jge 0x4DD33 */
        r6 = LIST2 + 6u * (uint32_t)i;                              /* lea 3i, *2 */
        to = (int32_t)D8(r6 + 1u);                                  /* 0x4DB8A [6i + 0x1079B9] */
        from = (int32_t)D8(r6);                                     /* 0x4DBA4 [6i + 0x1079B8] */
        type = (int)(uint8_t)(D8(r6 + 4u) - 3u);                    /* 0x4DBB9 mov al, [6i + 0x1079BC]; sub al, 3 */
        if (type > 7)                                               /* 0x4DBC2 cmp al, 7; ja (unsigned) */
            continue;
        switch (type) {                                             /* 0x4DBF4 jmp [4 * type + 0x4DAAC] */
        case 0:                                                     /* 0x4DB0B type 3 EMBARK */
            setD8(ar(from, n, 0x2Du), D8(r6 + 3u));                 /* 0x4DB34 [69(from+1) + n - 1 + 0xE31E6] = +3 */
            f5A4BC(5, n, from, (int32_t)S8(r6 + 2u), show);         /* 0x4DB4A dword [6i + 0x1079B7] sar 24; 0x4DB5F */
            break;
        case 1:                                                     /* 0x4DBFB type 4 DISEMBARK */
            setD8(ar(to, n, 0x2Du), D8(r6 + 3u));                   /* 0x4DC01 */
            f5A750(5, n, to, (int32_t)S8(r6 + 2u), show);           /* 0x4DC2C */
            break;
        case 2:                                                     /* 0x4DC36 type 5 DISEMBARK ALL */
            setD8(ar(to, n, 0x2Du), D8(r6 + 3u));                   /* 0x4DC3C */
            k = (int32_t)D8(ar(f3E8FC(to), n, 0x2Du));              /* 0x4DC49; 0x4DC60 */
            cnt = (int32_t)S8(rec + (uint32_t)k + 0x4Du);           /* 0x4DC6E dword [.. + 0x4A] sar 24: all aboard */
            f5A750(5, n, to, cnt, show);                            /* 0x4DC7F */
            break;
        case 7:                                                     /* 0x4DC89 type 10 a ship move */
            setD8(ar(from, n, 0x2Du), D8(r6 + 3u));                 /* 0x4DCA4 */
            cnt = 0;                                                /* 0x4DCAC [ebp-0x14] */
            for (k = 0; (int32_t)D8(rec + 0x40u) > k; k++)          /* 0x4DCBA; jle 0x4DCFA */
                if ((int32_t)S16(rec + 2u * (uint32_t)k + 0x45u) == from + 1   /* 0x4DCD2 dword [rec + 2k + 0x43] sar 16 */
                    && (int32_t)D8(rec + (uint32_t)k + 0x41u) > 0)  /* 0x4DCE6; jle */
                    cnt++;                                          /* 0x4DCF5 */
            f5A2C4(5, n, from, from, cnt > 0 ? 0x0F : 5, to, show); /* 0x4DCFE..0x4DD29 */
            break;
        default:                                                    /* 0x4DB64 types 6..9: nothing */
            break;
        }
    }
    setD8(MODE, 1);                                                 /* 0x4DD33 */
}

/* ==== the animated moves ============================================================================================ */

/* the source blinks: 0x5A106..0x5A1CB (land, f3F698) / 0x5A346..0x5A40B (ships, f3F70C) */
static void blink(void (*box)(int, int), int t)
{
    if (speedup() == 0) {                                           /* 0x5A118 jne 0x5A158: two extra blinks */
        box(t, 0x0E);                                               /* 0x5A122 */
        f3DA60(t);                                                  /* 0x5A12A */
        f78A20(S32(0xC65B8u));                                      /* 0x5A134 100 ms */
        box(t, 5);                                                  /* 0x5A141 */
        f3DA60(t);                                                  /* 0x5A149 */
        f78A20(S32(0xC65B8u));                                      /* 0x5A153 */
    }
    box(t, 0x0E);                                                   /* 0x5A160 */
    f3DA60(t);                                                      /* 0x5A168 */
    f78A20(S32(0xC65B8u));                                          /* 0x5A172 */
    box(t, 5);                                                      /* 0x5A17F */
    f3DA60(t);                                                      /* 0x5A187 */
    f78A20(S32(0xC65B8u));                                          /* 0x5A191 */
    box(t, 0x0E);                                                   /* 0x5A19E */
    f3DA60(t);                                                      /* 0x5A1A6 */
    f78A20(last_wait());                                            /* 0x5A1AB..0x5A1CB */
}

/* the move's start when shown: 0x5A0C0..0x5A101 / 0x5A300..0x5A341 */
static void show_start(int phase, int n, int hl, int t)
{
    f3F670();                                                       /* 0x5A0C0 */
    f3F624(hl, 0x0F);                                               /* 0x5A0CD */
    setD8(0xF4102u, (uint8_t)phase);                                /* 0x5A0D5 */
    if ((int32_t)S8(nrec(n) + 0x9Au) == 3)                          /* 0x5A0E1 dword [.. + 0xF1563] sar 24; cmp 3 */
        f5D14C(n, t);                                               /* 0x5A0F5 */
    setD8(0xF4102u, 5);                                             /* 0x5A0FA MOVEMENT, not the saved phase */
    f3CBBC();                                                       /* 0x5A101 */
}

/* 0x5A084: one land move (al phase, edx n, ebx hl, ecx flag; stack from, cfrom, to, cto, k, show; ret 0x18) */
void f5A084(int phase, int n, int hl, int flag, int from, int cfrom, int to, int cto, int k, int show)
{
    if (show != 0 && ai_shown()) {                                  /* 0x5A0A0; 0x5A0AA; 0x5A0B3 */
        show_start(phase, n, hl, from);                             /* 0x5A0C0..0x5A101 */
        blink(f3F698, from);                                        /* 0x5A106..0x5A1CB */
    }
    f57CE8(from, n, flag == 1, k);                                  /* 0x5A1D3 cmp [ebp-0x8], 1; sete; 0x5A1E4 */
    f57E28(to, n, k);                                               /* 0x5A1F2 */
    if (show != 0 && ai_shown()) {                                  /* 0x5A1F7..0x5A211 */
        f3F698(from, cfrom);                                        /* 0x5A21D */
        f3F698(to, 0x0E);                                           /* 0x5A22A */
        f3D52C(0, 0);                                               /* 0x5A233 */
        f78A20(S32(0xC65B8u));                                      /* 0x5A23D */
        f3F698(to, 5);                                              /* 0x5A24A */
        f3D52C(0, 0);                                               /* 0x5A253 */
        f78A20(S32(0xC65B8u));                                      /* 0x5A25D */
        f3F698(to, 0x0E);                                           /* 0x5A26A */
        f3D52C(0, 0);                                               /* 0x5A273 */
        f78A20(S32(0xC65B8u));                                      /* 0x5A27D */
        f3F698(to, cto);                                            /* 0x5A288 */
        f3D52C(0, 0);                                               /* 0x5A291 */
        f78A20(last_wait());                                        /* 0x5A296..0x5A2B6 */
    }
}

/* 0x5A2C4: one ship move (al phase, edx n, ebx hl, ecx from; stack color, to, show; ret 0xC) */
void f5A2C4(int phase, int n, int hl, int from, int color, int to, int show)
{
    uint32_t a;

    if (show != 0 && ai_shown()) {                                  /* 0x5A2E0..0x5A2FA */
        show_start(phase, n, hl, from);                             /* 0x5A300..0x5A341 */
        blink(f3F70C, from);                                        /* 0x5A346..0x5A40B */
    }
    f580CC(from, to, n);                                            /* 0x5A419 */
    a = nrec(n) + (uint32_t)D8(ar(to, n, 0x2Du)) + 0x41u;           /* 0x5A431 byte [.. + 0xE31E5]; 0x5A43C */
    setD8(a, (uint8_t)(D8(a) - 1u));                                /* 0x5A443 dec byte [edx + eax + 0xF150D] */
    f576F8(from);                                                   /* 0x5A44D (nation [0xF40F8]) */
    if (show != 0 && ai_shown()) {                                  /* 0x5A452..0x5A468 */
        f3F70C(from, color);                                        /* 0x5A470 */
        f3F70C(to, 0x0E);                                           /* 0x5A47D */
        f3DA60(to);                                                 /* 0x5A485 */
        f3CBBC();                                                   /* 0x5A48A */
        f78A20(last_wait());                                        /* 0x5A48F..0x5A4AF */
    }
}

/* 0x5A4BC: EMBARK k tokens (al phase, edx n, ebx i, ecx k; stack show; ret 4) */
void f5A4BC(int phase, int n, int i, int k, int show)
{
    int32_t t2 = f3E8FC(i);                                         /* 0x5A4DB; [ebp-0x8] */
    int32_t j;                                                      /* [ebp-0xC] */
    uint32_t a;

    for (j = 0; j < k; j++) {                                       /* 0x5A4F0 cmp eax, [ebp-0x10]; jge */
        f57CE8((int)D8(0xC803Cu + (uint32_t)i), n, 1, 1);           /* 0x5A4FE..0x5A510 (ebx = ecx = 1) */
        a = nrec(n) + (uint32_t)D8(ar(t2, n, 0x2Du)) + 0x4Du;       /* 0x5A528; 0x5A533 */
        setD8(a, (uint8_t)(D8(a) + 1u));                            /* 0x5A53A inc byte [edx + eax + 0xF1519] */
    }
    if (show != 0 && ai_shown()) {                                  /* 0x5A543..0x5A55D */
        setD8(0xF4102u, (uint8_t)phase);                            /* 0x5A566 */
        f3F698(t2, box_color(t2, n));                               /* 0x5A57E..0x5A59A */
        if ((int32_t)S8(nrec(n) + 0x9Au) == 3)                      /* 0x5A5A6; cmp 3 */
            f5D14C(S32(CUR_NATION), t2);                            /* 0x5A5BC */
        setD8(0xF4102u, 5);                                         /* 0x5A5C1 */
        f3DA60(t2);                                                 /* 0x5A5CB */
        f3CBBC();                                                   /* 0x5A5D0 */
        f78A20(last_wait());                                        /* 0x5A5D5..0x5A5F5 */
    }
}

/* 0x5A750: DISEMBARK k tokens (al phase, edx n, ebx i, ecx k; stack show; ret 4) */
void f5A750(int phase, int n, int i, int k, int show)
{
    int32_t t2 = f3E8FC(i);                                         /* 0x5A76F; [ebp-0xC] */
    int32_t d;                                                      /* [ebp-0x8] */
    int32_t j;                                                      /* [ebp-0x14] */
    int32_t w;
    uint32_t a;

    w = (int32_t)S16(nrec(n) + 2u * (uint32_t)D8(ar(t2, n, 0x2Du)) + 0x45u);   /* 0x5A79C dword [.. + 0xF150F] sar 16 */
    d = (int32_t)D8((uint32_t)((int32_t)0xC803Bu + w));             /* 0x5A7A8 mov al, [edx + 0xC803B] */
    for (j = 0; j < k; j++) {                                       /* 0x5A7BE cmp eax, [ebp-0x18]; jge */
        f57E28(d, n, 1);                                            /* 0x5A7D1 (ebx = 1) */
        a = nrec(n) + (uint32_t)D8(ar(t2, n, 0x2Du)) + 0x4Du;       /* 0x5A7E9; 0x5A7F4 */
        setD8(a, (uint8_t)(D8(a) - 1u));                            /* 0x5A7FB dec byte [edx + eax + 0xF1519] */
    }
    if (show != 0 && ai_shown()) {                                  /* 0x5A804..0x5A81E */
        setD8(0xF4102u, (uint8_t)phase);                            /* 0x5A827 */
        /* 0x5A83F..0x5A85F: the selection looked up in d's record (see the file comment), its word w */
        w = (int32_t)S16(nrec(n) + 2u * (uint32_t)D8(ar(d, n, 0x2Du)) + 0x45u);
        f3F698(w - 1, mvbl(w - 1, n) == 0 ? 5 : 0x0F);              /* 0x5A862 lea eax, [edx - 1]; 0x5A877 record w */
        if ((int32_t)S8(nrec(n) + 0x9Au) == 3)                      /* 0x5A8A5; cmp 3 */
            f5D14C(S32(CUR_NATION), w - 1);                         /* 0x5A8B5 */
        setD8(0xF4102u, 5);                                         /* 0x5A8BA */
        f3DA60(d);                                                  /* 0x5A8C4 */
        f3CBBC();                                                   /* 0x5A8C9 */
        f78A20(last_wait());                                        /* 0x5A8CE..0x5A8EE */
    }
}
