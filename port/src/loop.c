/* Module LOOP of docs/M6_PLAN.md: the game loop 0x54824 (all 21 phase cases, the next-phase writes, the empty hook
   0x5E7B4, the autosave 0x5E8EC, the elimination check 0x22B00, the round end), the restored game's set-up 0x14AA4,
   the end-of-round save 0x55258, the end-of-game mark 0x55304, the phase music 0x553C0, the trade-session check
   0x55500, the elimination check 0x22B00 with its end screens 0x22DCC / 0x22F3C, the AI's score matrix 0x7C960, and
   main 0x1188C..0x118C9 as game_loop_restored / game_loop_new. Literal translations; the address of each instruction a
   line comes from is in its comment. D = read in the code, G = inference. Prototypes: loop.h.

   The jump table 0x547D0 (21 dwords before the function, obj1.bin offset 0x447D0, each entry a fixup; D, checked
   against fixups.txt): 0 0x5491E, 1 0x5494B, 2 0x54976, 3 0x549A1, 4 0x549D0, 5 0x549FF, 6 0x54AFF, 7 0x54B33,
   8 0x54B7A, 9 0x54BD1, 10 0x54C01, 11 0x54C64, 12 0x54CB5, 13 0x54DB2, 14 0x54E07, 15 0x54FAA, 16 0x55096,
   17 0x550BC, 18 0x550FE, 19 0x55178, 20 0x551DB. A phase above 20 (cmp al, 0x14; ja, unsigned) goes to 0x55222.

   The cases (D). P = the phase byte [0xF4102] written, S = f5E7B4, A(p) = the autosave f5E8EC(p), E = f22B00; every
   path ends in E exactly once and loops. The next phase is written BEFORE the autosave, so the autosave after phase
   p is a save at the start of the next phase.
      0 TAXATION            f553C0, f7BCD4 (300 game_rand), 0x66050; P 1, S, A(0)
      1 POPULATION EXP.     f553C0, 0x673A8; P 2, S, A(1)
      2 CENSUS              f553C0, 0x13D44; P 3, S, A(2)
      3 SHIP MAINTENANCE    f553C0, 0x60AE0, 0x60BD8; P 4, S, A(3)
      4 SHIP CONSTRUCTION   f553C0, 0x60AE0, 0x61940; P 5, S, A(4)
      5 MOVEMENT            f553C0, copies area 0xE31B8 -> 0xE8888 (0x2B65) and territory state 0xE2C18 -> 0xE2FD8
                            (0x1E0); 0x55574; copies area -> 0xE5D20, territory state -> 0xE2DF8, nations 0xF14CC
                            -> 0xF1FB0 (0xAE2); int32 [0xEDF58] = 1, [0xEDF5C] = int16 round; P 7 (6 with PBEM), S,
                            A(5); PBEM: 0x6E53C, 0x5055C
      6 VIEW MOVEMENT       (PBEM only) f553C0, byte [0xF40FE] = 0, S, 0x59298, [0xE1624] &= ~1, S, P 7; no autosave
      7 CONFLICT            f553C0, byte [0xF40FE] = 0, [0xE1624] &= ~1, 0x5AAA4(1); P 8, S, A(7)
      8 CITY CONSTRUCTION   f553C0, int16 [0xF408E] = 0, 0x32214; P 9, S, A(8); PBEM and [0xF408E] == 1: 0x5055C
      9 REMOVE SURPLUS      f553C0, 0x5C238(1); P 10, S, A(9)
     10 CITY SUPPORT        f553C0, int16 [0x109E20] = 0, 0x5C62C; P 11, S, A(10), then int16 [0xF5476] = 1; PBEM
                            and [0x109E20] == 1: 0x5055C
     11 ACQUIRE GOODS       (no f553C0: 0x6BD98 calls it) int16 [0xF5478] = 0, 0x6BD98; P 12, S, A(11); PBEM and
                            [0xF5478] == 1: 0x5055C; byte [0x110F74] = 0
     12 DECLARE OFFERS      byte [0xF408C] != 0 (PBEM only): f763FC, 0x50D08. f55500(): int32 [0xF4230] = 1,
                            f553C0, (PBEM and [0xF5476] > 1: 0x762E0, f763FC), 0x6F270; P 13, S, A(12), bytes
                            0xF09D4[0..9] = 0; PBEM: 0x5055C. Else: [0xF4230] != 0: "Lack of nations ... (%d min)
                            cancels this round of trading." (f7983C); P 15; no autosave
     13 COUNTER OFFERS      [0xF408C] != 0: 0x50D08. f553C0, 0x723DC; P 14, S, A(13); PBEM and [0x112560]: 0x5055C
     14 ACCEPT TRADES       [0xF408C] != 0: 0x50D08. f553C0, byte [0x110F74] = 0, int16 [0xF4090] = 0, int16
                            [0xF4100] = 0, 0x734E4, S; unless int16 [0xF4090] != 0 (then nothing more: phase 14 again):
                            trades made ([0x110F74]) or PBEM: f763FC (not PBEM) and count = nations n with byte
                            0xF09D4[n] == 1 holding >= min goods (min = [0xE1623] >> 4; computers max(min, 6));
                            another session (P 12, int16 [0xF5476] + 1, PBEM message) if count >= (bit 0x10 of
                            [0xE1625] ? 2 : ([0xF40EC] + 1) / 2) and [0xF5476] < [0xE1623] & 0x0F, else P 15; A(14).
                            PBEM: int16 [0xF4100] = 1, 0x5055C
     15 CALAMITIES          (PBEM first: P 14, f553C0, P 15, f763FC, 0x762E0) int16 [0xF5476] = 1, byte [0xF408C]
                            = 0, int16 [0xF4094] = 0, int16 [0xF7AFC] = [0xF7AFE] = 0, 0x1E0A0, again both 0; P 16, S,
                            A(15); PBEM: nine 25-byte records 0xF07B4 -> 0xF0898, their counts 0
     16 MONOTHEISM          (no f553C0: 0x21914 calls it) 0x21914; P 17, S, A(16)
     17 CITY SUPPORT        f553C0, int16 [0x109E20] = 0, 0x5C62C, [0xF7AFC] = [0xF7AFE] = 0; P 18, S, A(17)
     18 ACQUIRE CIV.        f553C0, int16 [0xF4092] = 0, 0x162D0, int32 [0xF09E4] = any n 1..9 (in play or not) with
                            byte +0x28 != 0; 0x1A3A0 (not PBEM); P 19, S, A(18)
     19 DISCARD EXCESS      f553C0, int16 [0xF4096] = 0, 0x6E770; P 20, S, A(19); PBEM and one of int16 [0x109E20],
                            [0xF4092], [0xF4096] != 0: 0x5055C
     20 SHOW AST            f553C0, system("copy savgamea.dat savgameb.dat >NUL:"), f55258, (PBEM: 0x1A3A0), done =
                            f136B0(3); not done: P 0, A(20) (no S); done: no autosave, the loop ends
   Phases 6, 12 (no session) and 14 ([0xF4090] set) skip the autosave, and so does the winning path of 20 (D).

   Every phase routine is ported (19 Nov). The module that holds each (docs/M6_PLAN.md):
      0  0x66050 [TAX]                     1  0x673A8 [POP]                  2  0x13D44 [CENSUS]
      3  0x60AE0, 0x60BD8 [SHIP]           4  0x60AE0, 0x61940 [SHIP]        5  0x55574 [MOVE]
      7  0x5AAA4 [CONFLICT]                8  0x32214 [CITY]                 9  0x5C238 [CITY]
     10  0x5C62C [CITY]                   11  0x6BD98 [CARDS]               12  0x6F270 [TRADE]
     13  0x723DC [TRADE]                  14  0x734E4 [TRADE]               15  0x1E0A0 [CAL1]
     16  0x21914 [MONO]                   17  0x5C62C [CITY]                18  0x162D0, 0x1A3A0 [ADV]
     19  0x6E770 [CARDS]                  20  f136B0, f55258 (here)
     game end: 0x14294 [END] (after the loop, and in f22F3C)
   The computers' parts (M7) and the animation player (ANIM 0x7A0A0) still stop at not_ported inside them.
     elimination: 0x7784C (the player-button rows): newgame.c's, exported in game.h
     loop top, phases 15 / 16 with digital sound and a WAV in sample slot 0: AIL's 0x7D2E8 / 0x7CB9D, menu2.c's
       (exported 19 Nov). Slot 0 is loaded only by CAL5 0x1C6AF and ANIM 0x79B2E, 0x79DA2, 0x7A2BC, 0x7A47F (D: every
       call of 0x7B504 with eax = 0).
   Play by e-mail is dropped (Erik's ruling, OVERVIEW §7.5). The bit-0x80 tests of byte [0xE1624] (and the byte
   [0xF408C], which only PBEM code sets: 0x51F0A, 0x53260..0x53558) are kept; each PBEM branch calls
   not_ported(its first PBEM routine) and its remaining statements are kept as in the original after that call. Left
   out entirely: f22B00's PBEM player-record removal 0x22BB8..0x22C24 (its slot index is uninitialised when n has no
   record) and the message of a further PBEM trade session (0x54F42..0x54F72).

   Watcom runtime -> standard C:
     0x7DB3D sprintf -> snprintf bounded to the destination: 0x109E7C (8 bytes: the next variable is 0x109E84), 0xF5500
             (500 bytes, as map.c and load.c), the 100-byte stack buffers of 0x22B00 ([ebp-0x8C]), 0x22DCC ([ebp-0x80])
             and 0x22F3C ([ebp-0x84]), the 51-byte descriptions of 0x55258 / 0x55304
     0x7D8C8 fopen, 0x7D95E fclose, 0x912D5 fwrite(buf eax, size edx, n ebx, fp ecx), 0x91C47 rewind, 0x91CA7 remove
             -> the same, relative to the current folder (the port runs in the game folder). Port guard: the original
             goes on with a NULL FILE when fopen fails; the port skips the file work then
     0x7D4EC system("copy savgamea.dat savgameb.dat >NUL:") -> a C file copy (as save.c does: COPY of one file is
             binary; its result is not read). 0x22B00's system("del *.pl? >NUL") is PBEM (dropped)
     0x877AD stricmp -> a local case-insensitive compare (only == 0 is tested)
     0x7D5D0 memset, rep movsd / movsb -> memset / memcpy inside g_ds (the ranges do not overlap)
     0x87879 longjmp(0xF58D4, 1) -> port_longjmp(0xF58D4) (stack.h; main.c's setjmp)
     0x7D4AC (__CHK) dropped. The Miles AIL calls are port no-ops (static stubs below, as menu2.c's file comment
     describes them: AIL_active_sample_count -> 0); their arguments are still read from the data object.
   game_rand f78AE4: the loop draws none of its own; f7BCD4 draws 300 in phase 0, before 0x66050 (D).

   Data (D, read in the code at the addresses given; G for meanings not stated by the code):
     0xF4102 uint8 the phase (0..20; 0x15 during the starting-territory screen); 0xF40FC int16 the round
     0xE1620 int8 humans, 0xE1621 int8 computers, 0xE1623 uint8 trading: min goods in bits 4..7, sessions in bits
       0..3; 0xE1624 uint8 flags: 0x80 PBEM, 0x04 new game (cleared by a restore), 0x01 (G: a PBEM move log is
       pending); 0xE1625 bit 0x10: two nations suffice for another trade session
     0xF40EC int32 the players in play; nation n at 0xF14CC + 199n: +0x01..+0x12 commodity cards, +0x13 int8 calamity
       count, +0x14 + j calamity ids, +0x1C + j their givers, +0x28 (Roadbuilding, G), +0x51 int8 cities, +0x5C int8
       AST square, +0x98 int8 tokens on the board, +0x9A 0 out / 2 computer / 3 human
     0xF09D4 int8[10] per nation 1 = wants another trade session (written by the trade screens, G); 0x110F74 int8 trades
       made this session; 0xF4230 int32 a session was ever held (0 for a new game, main 0x11814; in the save, 0
       when an old save lacks it, 0x5F9ED)
     0xF5476 int16 the trade session number; 0xF5478, 0xF4090, 0xF4092, 0xF4094, 0xF4096, 0xF4100, 0xF408E, 0x109E20
       int16 per-phase flags the phase routines set (0xF4090: written only here, always 0, D: so phase 14 always
       autosaves); 0xF408C uint8 (PBEM only); 0xF7AFC / 0xF7AFE int16 cleared around the calamities
     0xEDF58 int32 1 = a post-movement snapshot exists, 0xEDF5C int32 its round (REPLAY, stack.c's dcase_101)
     0xE8888 / 0xE2FD8 the pre-movement copies, 0xE5D20 / 0xE2DF8 / 0xF1FB0 the post-movement copies
     0xF09E4 int32 some nation holds advance +0x28 (G: Roadbuilding, for the movement)
     0xCAB78 9-byte music records per phase: +0 name (link address), +4 WAV (0 none), +8 int8 sequence. obj2: phases
       0..11 and 17..21 "gmap__1c.xmi", 12..14 "gtrade2c.xmi" with "marketpl.wav", 15..16 "gcalam3c.xmi"; seq 0
     0xCAB1C the title music's name: obj2 "gcivil2_.xmi " (with a space), overwritten at start-up by the driver's
       "gcivil2.xmi" / "fcivil2.xmi" / "scivil2.xmi" (main 0x10C13..); 0xF61D8 the music now loaded; 0xF6284 its AIL
       sequence; 0xF097C bits 0x01 effects, 0x02 music; 0xF097E effects volume in bits 7..13; 0xF625C digital sound;
       sample slots s 0..2: name 0xF6118 + 64s, block [0xF6260 + 4s] (a port handle), AIL sample [0xF6278 + 4s];
       [0xF6274] the AIL digital driver
     0xF52E0 int32[10][10] the score differences, 0xF52D4 int32 the leader (written only when a score beats the
       running best, so with no nation in play it keeps its old value); 0xBCCE0 uint8[9][17][5] per nation and AST
       square (byte 0 rises 0..4 along the track, G: the AI's stage of the game)
     0xBB800 dialog with CONTINUE (id 0x2A); 0xC768C nation names; 0xC65C0 / 0xC65F4 51 zero bytes (the description
       templates of 0x55258 / 0x55304)

   Heap pointers in data fields: [0xF6260] (slot 0's WAV block, a port handle) is only passed to not_ported's
   placeholder; 0x55304 reloads the save with f5F05C, which frees and re-reads the 11 heap lists [0xF0984 + 4i] (port
   handles, saveload.c). Nothing else here.

   1995 bugs: none fixed here (no g_original branches). Kept as the original, for Erik (D unless marked):
     - 0x22B00 discards an eliminated nation's calamity cards to their piles (0x771F8) but leaves its count +0x13 and
       its list +0x14.. as they were (the commodity counts are decremented to 0). The cards then exist twice; harmless
       while every reader skips nations with +0x9A == 0 (G, not checked across the unported modules).
     - 0x22F3C's nation is stack garbage if no nation is in play (G: unreachable, it runs when one player is left);
       the port starts it at 0 (name "").
     - 0x55304's "r+b" leaves the old CRC in place and appends a second one (0): the file grows by 4 bytes and still
       checks as intact. Kept for byte-exact files.
   Oddities (D): phase 20 copies savgamea.dat over savgameb.dat and 0x55258 rewrites savgameb.dat at once; 0x55304
   reloads the last autosave into the game just before quitting; 0x7C960 calls f147C4 1 + players times per nation and
   keeps a maximum it never uses; main's code after the call of 0x54824 (0x118CE..0x118F4) is unreachable. */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "fg.h"
#include "game.h"
#include "info.h"
#include "kit.h"
#include "loop.h"
#include "tax.h"
#include "pop.h"
#include "census.h"
#include "ship.h"
#include "movephase.h"
#include "conflict.h"
#include "end.h"
#include "city.h"
#include "cards.h"
#include "cal1.h"
#include "mono.h"
#include "adv.h"
#include "menu2.h"
#include "rules.h"
#include "saveload.h"
#include "savewrite.h"
#include "stack.h"
#include "trade.h"
#include "start.h"
#include "state.h"
#include "ui.h"

/* A call of a phase routine (or helper) that is not translated yet. With LOOP_TEST (port/test/loop_test.c) it goes
   to the test's recorder, which returns, so the transition table can be checked with every phase routine stubbed;
   otherwise it is not_ported(addr): the port stops there with the address. */
#ifdef LOOP_TEST
void loop_test_unported(uint32_t addr);
#define UNPORTED(addr) loop_test_unported(addr)
#define PORTED(call, addr) loop_test_unported(addr)   /* the test stubs every phase routine, ported or not */
#else
#define UNPORTED(addr) not_ported(addr)
#define PORTED(call, addr) call                        /* a ported phase routine */
#endif
/* A play-by-e-mail routine (dropped by Erik's ruling): not_ported in every build */
#define PBEM_DROPPED(addr) not_ported(addr)

#define PHASE     0xF4102u
#define ROUND     0xF40FCu
#define FLAGS     0xE1624u
#define PBEM_BIT  0x80u
#define NATION(n) (0xF14CCu + 199u * (uint32_t)(n))
#define NAME(n)   DSTR(D32(0xC768Cu + 4u * (uint32_t)(n)))

/* ---- Miles AIL (port no-ops, see the file comment) --------------------------------------------------------------- */

static int ail_active_sample_count(uint32_t dig)
{
    (void)dig;
    return 0;
}

static void ail_end_sample(uint32_t s)
{
    (void)s;
}

static void ail_set_sample_loop_count(uint32_t s, int n)
{
    (void)s;
    (void)n;
}

static void ail_set_sample_volume(uint32_t s, int v)
{
    (void)s;
    (void)v;
}

static void ail_start_sample(uint32_t s)
{
    (void)s;
}

static void ail_end_sequence(uint32_t seq)
{
    (void)seq;
}

/* ---- Runtime stand-ins ----------------------------------------------------------------------------------------- */

/* 0x877AD stricmp: 0 when equal ignoring case (Watcom lowercases both; only the zero test is used) */
static int ds_stricmp(const char *a, const char *b)
{
    for (;; a++, b++) {
        int ca = tolower((unsigned char)*a), cb = tolower((unsigned char)*b);
        if (ca != cb)
            return ca - cb;
        if (ca == 0)
            return 0;
    }
}

/* system("copy SRC DST >NUL:") (0x7D4EC) done in C: the whole file, binary. The result is not read (0x551E5) */
static void copy_file(const char *src, const char *dst)
{
    FILE *in, *out;
    char buf[4096];
    size_t got;

    in = fopen(src, "rb");
    if (in == NULL)
        return;
    out = fopen(dst, "wb");
    if (out == NULL) {
        fclose(in);
        return;
    }
    while ((got = fread(buf, 1, sizeof buf, in)) > 0)
        if (fwrite(buf, 1, got, out) != got)
            break;
    fclose(out);
    fclose(in);
}

/* the 4-byte CRC appended to a save file with fopen "ab" and fwrite(&crc, 4, 1, fp), little-endian as on the 386 */
static void append_crc(const char *name, uint32_t crc)
{
    FILE *fp;
    uint8_t b[4];

    fp = fopen(name, "ab");
    if (fp == NULL)                                   /* port guard (no NULL check in the original) */
        return;
    b[0] = (uint8_t)crc;
    b[1] = (uint8_t)(crc >> 8);
    b[2] = (uint8_t)(crc >> 16);
    b[3] = (uint8_t)(crc >> 24);
    (void)fwrite(b, 4, 1, fp);
    fclose(fp);
}

/* ---- 0x5E7B4, 0x14AA4 ------------------------------------------------------------------------------------------ */

void f5E7B4(void)
{
    /* 0x5E7B4: __CHK, push ebp, mov ebp, esp, pop ebp, ret (G: a trace or autosave hook emptied for release) */
}

void f14AA4(void)
{
    int32_t n;                                        /* [ebp-0x14] */

    for (n = 1; n <= 9; n++) {                        /* 0x14AB8; 0x14AC7 cmp 9; jg 0x14B0E */
        if (D8(NATION(n) + 0x9Au) == 0)               /* 0x14AD4 byte [199n + 0xF1566] */
            continue;
        if (D32(0xF60E8u + 4u * (uint32_t)n) == 0)    /* 0x14AE1 the override flag (always 0, main 0x10172) */
            continue;
        f15FE0(n, DSTR(0xF6064u + 13u * (uint32_t)n));   /* 0x14B07 */
    }
    memcpy(DP(0xEF3C4u), DP(0xEED84u), 0x640);         /* 0x14B0E..0x14B2A rep movsd 0x190, movsb 0 */
}

/* ---- 0x55258, 0x55304: the end-of-round and end-of-game saves ----------------------------------------------------- */

void f55258(void)
{
    char desc[0x33];                                  /* [ebp-0x50], 51 bytes */
    FILE *fp;                                         /* [ebp-0x18] */
    uint32_t crc;                                     /* [ebp-0x1C] */

    port_checkpoint(0x55258u);                        /* port (M7): the end-of-round save, a comparison point */
    memcpy(desc, DP(0xC65C0u), sizeof desc);          /* 0x5526D..0x5527E rep movsd 0xC, movsw, movsb: zeros */
    fp = fopen("savgameb.dat", "wb");                 /* 0x55289 (0xB5A30, mode 0xB5A2D) */
    snprintf(desc, sizeof desc, "AutoSaved at end of Round %d", (int)S16(ROUND));   /* 0x552A3 (0xB5A3D) */
    if (fp == NULL)                                   /* port guard: the original writes through a NULL FILE */
        return;
    f5E9B0(desc, fp);                                 /* 0x552B1 */
    fclose(fp);                                       /* 0x552B9 */
    crc = f7BBF0("savgameb.dat");                     /* 0x552C3 (0xB5A5A) */
    append_crc("savgameb.dat", crc);                  /* 0x552D5 fopen "ab" (0xB5A6A, 0xB5A67), 0x552ED fwrite, 0x552F5 */
}

void f55304(void)
{
    char desc[0x33];                                  /* [ebp-0x54], 51 bytes */
    int16_t version;                                  /* [ebp-0x18] */
    FILE *fp;                                         /* [ebp-0x1C] */
    uint32_t crc;                                     /* [ebp-0x20] */

    memcpy(desc, DP(0xC65F4u), sizeof desc);          /* 0x55319..0x5532A: zeros */
    fp = fopen("savgamea.dat", "r+b");                /* 0x55335 (0xB5A7B, mode 0xB5A77) */
    if (fp == NULL)                                   /* port guard */
        return;
    f5F05C(&version, desc, fp);                       /* 0x55346: the last autosave back into the game globals */
    snprintf(desc, sizeof desc, "AutoSaved at End of Game");   /* 0x55354 (0xB5A88) */
    rewind(fp);                                       /* 0x5535F */
    f5E9B0(desc, fp);                                 /* 0x5536A: the old CRC stays after the rewritten body */
    fclose(fp);                                       /* 0x55372 */
    crc = f7BBF0("savgamea.dat");                     /* 0x5537C (0xB5AA1): 0 over body + old CRC */
    append_crc("savgamea.dat", crc);                  /* 0x5538E "ab" (0xB5AB1, 0xB5AAE), 0x553A6 fwrite, 0x553AE */
}

/* ---- 0x553C0: phase music and sample ---------------------------------------------------------------------------- */

void f553C0(void)
{
    int32_t p;
    uint32_t wav;                                     /* ebx */
    int32_t v;                                        /* [ebp-0x14] */

    if (ds_stricmp(DSTR(0xF61D8u), DSTR(0xCAB1Cu)) == 0)   /* 0x553DE stricmp(0xF61D8, 0xCAB1C); jne 0x553F6 */
        ail_end_sequence(D32(0xF6284u));              /* 0x553EE AIL_end_sequence([0xF6284]) */
    if ((D8(0xF097Cu) & 0x02) != 0) {                 /* 0x553F6 music */
        p = S8(PHASE);                                /* 0x553FF dword [0xF40FF] sar 24 */
        f7B854(DSTR(D32(0xCAB78u + 9u * (uint32_t)p)),   /* 0x5541A name +0 */
               S8(0xCAB80u + 9u * (uint32_t)p));      /* 0x55411 dword [9p + 0xCAB7D] sar 24: seq +8 */
    }
    if ((D8(0xF097Cu) & 0x01) == 0)                   /* 0x55425 effects; je 0x554F5 */
        return;
    p = S8(PHASE);                                    /* 0x55432 */
    wav = D32(0xCAB7Cu + 9u * (uint32_t)p);           /* 0x55444 +4 */
    if (wav == 0) {                                   /* 0x5544A; je 0x554E6 */
        ail_end_sample(D32(0xF6280u));                /* 0x554ED AIL_end_sample([0xF6280]): slot 2 */
        return;
    }
    f7B504(2, DSTR(wav));                             /* 0x55459 */
    if (D8(0xF625Cu) == 0)                            /* 0x5545E; je 0x554F5 */
        return;
    v = (int32_t)(uint16_t)((uint16_t)(D16(0xF097Eu) << 2) >> 9);   /* 0x5546C..0x5547B: bits 7..13 */
    v = v / 2;                                        /* 0x5547E..0x55485 signed /2 */
    v = v * 127 / 9;                                  /* 0x55487..0x55498 (shl 7, sub; idiv 9) */
    if (v == 0)                                       /* 0x554A0 */
        v = 1;                                        /* 0x554A4 */
    ail_set_sample_volume(D32(0xF6280u), v);          /* 0x554B6 */
    ail_start_sample(D32(0xF6280u));                  /* 0x554C4 */
    ail_set_sample_loop_count(D32(0xF6280u), 0);      /* 0x554D5: forever */
}

/* ---- 0x55500: two nations with the minimum goods ---------------------------------------------------------------- */

int f55500(void)
{
    int32_t count = 0;                                /* [ebp-0x8] */
    int32_t n;                                        /* [ebp-0x10] */
    int32_t held;                                     /* [ebp-0xC] */

    for (n = 1; n <= 9; n++) {                        /* 0x55516; 0x55525 cmp 9; jg 0x55561 */
        if (D8(NATION(n) + 0x9Au) == 0)               /* 0x55532 */
            continue;
        held = f76F14(n);                             /* 0x5553F */
        if ((int32_t)(D8(0xE1623u) >> 4) > held)      /* 0x55547..0x55557 cmp min, held; jg skip */
            continue;
        count++;                                      /* 0x5555C */
    }
    return count >= 2;                                /* 0x55561 setge */
}

/* ---- 0x7C960: the AI's score matrix ------------------------------------------------------------------------------ */

void f7C960(void)
{
    int32_t best = -9999;                             /* [ebp-0x18] = 0xFFFFD8F1 */
    int32_t stage = -1;                               /* [ebp-0x14]: never read after the loop (dead, D) */
    int32_t i, j;                                     /* [ebp-0x20], [ebp-0x24] */
    int32_t s;                                        /* [ebp-0x1C] */
    int32_t v;

    memset(DP(0xF52E0u), 0, 0x190);                   /* 0x7C993 memset(0xF52E0, 0, 0x190) */
    for (i = 1; i <= 9; i++) {                        /* 0x7C998; 0x7C9A3 cmp 9; jg 0x7CAAB */
        if (D8(NATION(i) + 0x9Au) == 0)               /* 0x7C9B4 */
            continue;
        v = S8(0xBCCE0u + 5u * (uint32_t)(int32_t)S8(NATION(i) + 0x5Cu) + 85u * (uint32_t)(i - 1));   /* 0x7C9EB */
        if (v > stage)                                /* 0x7C9F8 cmp eax, ecx; jle */
            stage = v;                                /* 0x7CA29..0x7CA33 (the same byte read again) */
        s = f147C4(i);                                /* 0x7CA39 */
        if (s > best) {                               /* 0x7CA44; jle 0x7CA57 */
            best = s;                                 /* 0x7CA4C */
            setD32(0xF52D4u, (uint32_t)i);            /* 0x7CA52 the leader */
        }
        for (j = 1; j <= 9; j++) {                    /* 0x7CA57; 0x7CA66 cmp 9; jg 0x7C99D */
            if (D8(NATION(j) + 0x9Au) == 0)           /* 0x7CA77 */
                continue;
            setD32(0xF52E0u + 40u * (uint32_t)i + 4u * (uint32_t)j,   /* 0x7CAA2 [40i + 4j + 0xF52E0] */
                   (uint32_t)(f147C4(j) - s));        /* 0x7CA84; 0x7CA8E sub ebx, esi */
        }
    }
    (void)stage;
}

/* ---- 0x22B00: eliminations, and the two end screens -------------------------------------------------------------- */

void f22B00(void)
{
    int32_t n;                                        /* [ebp-0x20] */
    uint32_t rec;                                     /* [ebp-0x1C] */
    int32_t i;                                        /* [ebp-0x18] */
    int32_t m, j;                                     /* [ebp-0x18], [ebp-0x28] */
    char buf[100];                                    /* [ebp-0x8C] */

    for (n = 1; n <= 9; n++) {                        /* 0x22B15; 0x22D22 cmp 9; jg 0x22DC0 */
        rec = NATION(n);                              /* 0x22D3A */
        if (D8(rec + 0x9Au) == 0)                     /* 0x22D40 not in play */
            continue;
        if (S8(rec + 0x98u) != 0)                     /* 0x22D4D dword [rec + 0x95] sar 24: tokens on the board */
            continue;
        if (S8(rec + 0x51u) != 0)                     /* 0x22D5A dword [rec + 0x4E] sar 24: cities */
            continue;
        if (S8(rec + 0x9Au) == 2) {                   /* 0x22D67 dword [rec + 0x97] sar 24; cmp 2; jne 0x22B21 */
            setD8(0xE1621u, (uint8_t)(D8(0xE1621u) - 1));   /* 0x22D7C dec byte [0xE1621]: computers */
            snprintf(buf, sizeof buf, "%s has been eliminated from the game.", NAME(n));   /* 0x22D99 (0xB1F24) */
            f79270(-500, n, -200, buf, 0);            /* 0x22DB6 */
        } else if ((D8(FLAGS) & PBEM_BIT) == 0 && S8(0xE1620u) > 1) {   /* 0x22B21..0x22B37 */
            setD8(0xE1620u, (uint8_t)(D8(0xE1620u) - 1));   /* 0x22B39 dec byte [0xE1620]: humans */
            snprintf(buf, sizeof buf, "%s has been eliminated from the game.", NAME(n));   /* 0x22B56 (0xB1F4A) */
            f79270(-500, n, -200, buf, 0);            /* 0x22B73 */
        } else if ((D8(FLAGS) & PBEM_BIT) != 0) {     /* 0x22B7A */
            /* 0x22B85 system("del *.pl? >NUL"), 0x22B92 f230B8(n): PBEM */
            PBEM_DROPPED(0x230B8u);
        } else
            f22DCC(n);                                /* 0x22B9C: the last human; does not return */
        setD8(rec + 0x9Au, 0);                        /* 0x22BA4 */
        /* 0x22BAB..0x22C24: with PBEM, n's 32-byte player record 0xF4104 + 32p taken out (left out: PBEM) */
        setD32(0xF40ECu, D32(0xF40ECu) - 1u);         /* 0x22C26..0x22C2D players - 1 */
        if (S32(0xF40ECu) == 1)                       /* 0x22C33 */
            f22F3C();                                 /* 0x22C38: one player left; does not return */
        for (i = 1; i <= 0x12; i++)                   /* 0x22C3D; 0x22C4C cmp 0x12; jg 0x22C6F */
            while (D8(rec + (uint32_t)i) != 0) {      /* 0x22C58 cmp byte [rec + i], 0 */
                f771F8(i);                            /* 0x22C60 the commodity card back to its pile */
                setD8(rec + (uint32_t)i, (uint8_t)(D8(rec + (uint32_t)i) - 1));   /* 0x22C6B */
            }
        for (i = 0; S8(rec + 0x13u) > i; i++)         /* 0x22C6F; 0x22C7F dword [rec + 0x10] sar 24 */
            f771F8(S8(rec + 0x14u + (uint32_t)i));    /* 0x22C90 dword [rec + i + 0x11] sar 24; 0x22C96. The count
                                                         +0x13 and the list stay (file comment) */
        for (m = 1; m <= 9; m++) {                    /* 0x22C9D; 0x22CAC cmp 9; jg 0x22D12 */
            if (D8(NATION(m) + 0x9Au) == 0)           /* 0x22CB9 */
                continue;
            for (j = 0; S8(NATION(m) + 0x13u) > j; j++)   /* 0x22CC3; 0x22CD7 dword [199m + 0xF14DC] sar 24 */
                if (S8(NATION(m) + 0x1Cu + (uint32_t)j) == n)   /* 0x22CEF dword [199m + j + 0xF14E5] sar 24 */
                    setD8(NATION(m) + 0x1Cu + (uint32_t)j, 0xFF);   /* 0x22D09 the giver gone */
        }
        f7784C(S32(0xF40ECu));                        /* 0x22D12 eax = [0xF40EC]; 0x22D17 the player-button rows for
                                                         the new count (newgame.c) */
    }
}

/* the common screen of 0x22DCC / 0x22F3C: background 21 into the back buffer, font 2, colour 13 (0x22DE4..0x22E37,
   0x22F7D..0x22FCF) */
static void end_screen_open(void)
{
    f3CD6C(0x15);                                     /* 0x22DE9 */
    fg_vbcopy(0, 0x27F, 0, 0x1DF, 0, 0x1DF, S32(0xF52CCu), S32(0xF52C8u));   /* 0x22E11 background -> back buffer */
    fg_vbopen(S32(0xF52C8u));                         /* 0x22E20 */
    fg_setcolor(0x0D);                                /* 0x22E2A */
    f39F90(2);                                        /* 0x22E37 */
}

/* CONTINUE moved to (0x109, 0x190), the dialog drawn, the back buffer shown, then the wait until -1
   (0x22EB7..0x22F10, 0x23029..0x23082) */
static void end_screen_wait(void)
{
    int r;                                            /* [ebp-0x18] */

    fg_vbclose();                                     /* 0x22EB7 */
    f3BB48(0xBB800u, 0x2A, 0x109);                    /* 0x22ECB */
    f3BC64(0xBB800u, 0x2A, 0x190);                    /* 0x22EDF */
    f3B05C(0xBB800u);                                 /* 0x22EE9 */
    f3CC14();                                         /* 0x22EEE */
    do
        r = f78BA4(0xBB800u, 0);                      /* 0x22EFA */
    while (r == -100 || r != -1);                     /* 0x22F05 cmp -0x64; je; 0x22F0D cmp -1; jne */
}

_Noreturn void f22DCC(int n)
{
    char buf[100];                                    /* [ebp-0x80] */

    end_screen_open();
    snprintf(buf, sizeof buf, "%s, none of your cities or units", NAME(n));   /* 0x22E58 (0xB1F7F) */
    f3A0B0(0, 0x32, 0x280, buf);                      /* 0x22E67 */
    f3A0B0(0, 0x50, 0x280, DSTR(0xB1FA0u));           /* 0x22E7D "remain on the board." */
    f3A0B0(0, 0x6E, 0x280, DSTR(0xB1FB5u));           /* 0x22E93 "You have been eliminated from the game." */
    if ((D8(FLAGS) & PBEM_BIT) != 0)                  /* 0x22E98 */
        f3A0B0(0, 0x96, 0x280, DSTR(0xB1FDDu));       /* 0x22EB2 "Do not send anything to the host." */
    end_screen_wait();
    if ((D8(FLAGS) & PBEM_BIT) != 0) {                /* 0x22F12 */
        setD8(0xF5500u, 0);                           /* 0x22F1F */
        f44CAC();                                     /* 0x22F25 quit */
    }
    port_longjmp(0xF58D4u);                           /* 0x22F2C..0x22F36 longjmp(0xF58D4, 1): the title menu */
}

_Noreturn void f22F3C(void)
{
    int32_t n;                                        /* [ebp-0x20] */
    int32_t w = 0;                                    /* [ebp-0x1C]: stack garbage in the original if no nation is
                                                         in play (G: unreachable; file comment) */
    char buf[100];                                    /* [ebp-0x84] */

    for (n = 1; n <= 9; n++)                          /* 0x22F51; 0x22F60 cmp 9; jg 0x22F7D */
        if (D8(NATION(n) + 0x9Au) != 0) {             /* 0x22F6D */
            w = n;                                    /* 0x22F7A */
            break;
        }
    end_screen_open();
    snprintf(buf, sizeof buf, "%s, you have conquered the known world!", NAME(w));   /* 0x22FF6 (0xB1FFF) */
    f3A0B0(0, 0x50, 0x280, buf);                      /* 0x23005 */
    if ((D8(FLAGS) & PBEM_BIT) != 0)                  /* 0x2300A */
        f3A0B0(0, 0x96, 0x280, DSTR(0xB2027u));       /* 0x23024 "Do not send anything to the host." */
    end_screen_wait();
    if ((D8(FLAGS) & PBEM_BIT) != 0) {                /* 0x23084 */
        setD8(0xF5500u, 0);                           /* 0x23091 */
        f44CAC();                                     /* 0x23097 */
    }
    PORTED(f14294(), 0x14294u);                               /* 0x2309E Final Civilization Standings [END] */
    f788AC();                                         /* 0x230A3 */
    port_longjmp(0xF58D4u);                           /* 0x230A8..0x230B2 */
}

/* ---- 0x54824: the game loop -------------------------------------------------------------------------------------- */

/* 0x5483E..0x548CA: in CALAMITIES and MONOTHEISM with digital sound, the sample of slot 0 stopped and its WAV freed */
static void loop_top_sound(void)
{
    uint8_t ah = D8(PHASE);                           /* 0x54848 */

    if (ah != 0x0F && ah != 0x10)                     /* 0x5484E, 0x54853 */
        return;
    if (D8(0xF625Cu) == 0)                            /* 0x5485C */
        return;
    if (ail_active_sample_count(D32(0xF6274u)) > 0)   /* 0x5486D; jle 0x54888 */
        ail_end_sample(D32(0xF6278u));                /* 0x54880 */
    ail_set_sample_loop_count(D32(0xF6278u), 1);      /* 0x54891 */
    if (strlen(DSTR(0xF6118u)) != 0) {                /* 0x548A3 repne scasb; je 0x548D0 */
        PORTED(f7CB9D(D32(0xF6260u), f7D2E8(DSTR(0xF6118u))), 0x7CB9Du);   /* 0x548B1 file size, 0x548C0 free */
        setD8(0xF6118u, 0);                           /* 0x548CA */
    }
}

/* 0x549FF..0x54A20 / 0x54A51..0x54AA6: the movement snapshots (rep movsd + movsb) */
static void snap(uint32_t dst, uint32_t src, uint32_t len)
{
    memcpy(DP(dst), DP(src), len);
}

_Noreturn void f54824(void)
{
    int32_t done = 0;                                 /* [ebp-0x24] */
    int32_t n;                                        /* [ebp-0x1C] */
    int32_t count;                                    /* [ebp-0x20] */
    int32_t need;
    int8_t min;                                       /* [ebp-0x18] */

    while (done == 0) {                               /* 0x5483E cmp [ebp-0x24], 0; jne 0x5522C */
        loop_top_sound();
        snprintf((char *)DP(0x109E7Cu), 8, "%d", (int)S16(ROUND));   /* 0x548E8 sprintf(0x109E7C, "%d" 0xB5964) */
        f3C284(0xF596Cu, 0x101, 0x109E7Cu);           /* 0x548FA the round on the replay button */
        f7C960();                                     /* 0x548FF */
        switch (D8(PHASE)) {                          /* 0x54904 cmp al, 0x14; ja 0x55222; jmp [4*eax + 0x547D0] */
        case 0:                                       /* 0x5491E TAXATION */
            f553C0();
            f7BCD4();                                 /* 0x54923 the AI's random table: 300 game_rand */
            PORTED(f66050(), 0x66050u);                       /* 0x5492A [TAX] */
            setD8(PHASE, 1);                          /* 0x5492F */
            f5E7B4();
            f5E8EC(0);                                /* 0x5493C */
            break;
        case 1:                                       /* 0x5494B POPULATION EXPANSION */
            f553C0();
            PORTED(f673A8(), 0x673A8u);                       /* 0x54952 [POP] */
            setD8(PHASE, 2);                          /* 0x54957 */
            f5E7B4();
            f5E8EC(1);                                /* 0x54967 */
            break;
        case 2:                                       /* 0x54976 CENSUS */
            f553C0();
            PORTED(f13D44(), 0x13D44u);                       /* 0x5497D [CENSUS] */
            setD8(PHASE, 3);                          /* 0x54982 */
            f5E7B4();
            f5E8EC(2);                                /* 0x54992 */
            break;
        case 3:                                       /* 0x549A1 SHIP MAINTENANCE */
            f553C0();
            PORTED(f60AE0(), 0x60AE0u);                       /* 0x549A6 [SHIP] */
            PORTED(f60BD8(), 0x60BD8u);                       /* 0x549AB [SHIP] */
            setD8(PHASE, 4);                          /* 0x549B0 */
            f5E7B4();
            f5E8EC(3);                                /* 0x549C1 */
            break;
        case 4:                                       /* 0x549D0 SHIP CONSTRUCTION */
            f553C0();
            PORTED(f60AE0(), 0x60AE0u);                       /* 0x549D5 [SHIP] */
            PORTED(f61940(), 0x61940u);                       /* 0x549DA DoShipContruction [SHIP] */
            setD8(PHASE, 5);                          /* 0x549DF */
            f5E7B4();
            f5E8EC(4);                                /* 0x549F0 */
            break;
        case 5:                                       /* 0x549FF MOVEMENT */
            f553C0();                                 /* 0x54A0E (the copy's registers are set before the call) */
            snap(0xE8888u, 0xE31B8u, 0x2B65);         /* 0x54A13..0x54A20 areas before the movement */
            snap(0xE2FD8u, 0xE2C18u, 0x1E0);          /* 0x54A32..0x54A3F territory state */
            PORTED(f55574(), 0x55574u);                       /* 0x54A51 the movement phase [MOVE] */
            snap(0xE5D20u, 0xE31B8u, 0x2B65);         /* 0x54A56..0x54A63 areas after it (REPLAY) */
            snap(0xE2DF8u, 0xE2C18u, 0x1E0);          /* 0x54A75..0x54A82 */
            snap(0xF1FB0u, 0xF14CCu, 0xAE2);          /* 0x54A99..0x54AA6 nations */
            setD32(0xEDF58u, 1);                      /* 0x54AAE */
            setD32(0xEDF5Cu, (uint32_t)(int32_t)S16(ROUND));   /* 0x54AA9 dword [0xF40FA] sar 16; 0x54ABD */
            setD8(PHASE, (uint8_t)(((D8(FLAGS) & PBEM_BIT) == 0) + 6));   /* 0x54AC2 sete al; add al, 6 */
            f5E7B4();
            f5E8EC(5);                                /* 0x54AD9 */
            if ((D8(FLAGS) & PBEM_BIT) != 0) {        /* 0x54ADE */
                PBEM_DROPPED(0x6E53Cu);               /* 0x54AEB */
                PBEM_DROPPED(0x5055Cu);               /* 0x54AF0 */
            }
            break;
        case 6:                                       /* 0x54AFF VIEW MOVEMENT (PBEM only) */
            f553C0();
            setD8(0xF40FEu, 0);                       /* 0x54B06 */
            f5E7B4();
            PBEM_DROPPED(0x59298u);                   /* 0x54B11 */
            setD8(FLAGS, (uint8_t)(D8(FLAGS) & ~1u)); /* 0x54B16 */
            f5E7B4();
            setD8(PHASE, 7);                          /* 0x54B22; no autosave */
            break;
        case 7:                                       /* 0x54B33 CONFLICT */
            f553C0();
            setD8(0xF40FEu, 0);                       /* 0x54B40 */
            setD8(FLAGS, (uint8_t)(D8(FLAGS) & ~1u)); /* 0x54B3A..0x54B4E */
            PORTED(f5AAA4(1), 0x5AAA4u);                       /* 0x54B56 f5AAA4(1) [CONFLICT] */
            setD8(PHASE, 8);                          /* 0x54B5B */
            f5E7B4();
            f5E8EC(7);                                /* 0x54B6B */
            break;
        case 8:                                       /* 0x54B7A CITY CONSTRUCTION */
            f553C0();
            setD16(0xF408Eu, 0);                      /* 0x54B83 */
            PORTED(f32214(), 0x32214u);                       /* 0x54B8A [CITY] */
            setD8(PHASE, 9);                          /* 0x54B8F */
            f5E7B4();
            f5E8EC(8);                                /* 0x54B9F */
            if ((D8(FLAGS) & PBEM_BIT) != 0 && S16(0xF408Eu) == 1)   /* 0x54BA4; 0x54BB1 dword [0xF408C] sar 16 */
                PBEM_DROPPED(0x5055Cu);               /* 0x54BC2 */
            break;
        case 9:                                       /* 0x54BD1 REMOVE SURPLUS */
            f553C0();
            PORTED(f5C238(1), 0x5C238u);                       /* 0x54BDD f5C238(1) [CITY] */
            setD8(PHASE, 10);                         /* 0x54BE2 */
            f5E7B4();
            f5E8EC(9);                                /* 0x54BF2 */
            break;
        case 10:                                      /* 0x54C01 CITY SUPPORT */
            f553C0();
            setD16(0x109E20u, 0);                     /* 0x54C0A */
            PORTED(f5C62C(), 0x5C62Cu);                       /* 0x54C11 [CITY] */
            setD8(PHASE, 11);                         /* 0x54C16 */
            f5E7B4();
            f5E8EC(10);                               /* 0x54C2B */
            setD16(0xF5476u, 1);                      /* 0x54C35 after the autosave */
            if ((D8(FLAGS) & PBEM_BIT) != 0 && S16(0x109E20u) == 1)   /* 0x54C3C; 0x54C44 dword [0x109E1E] sar 16 */
                PBEM_DROPPED(0x5055Cu);               /* 0x54C55 */
            break;
        case 11:                                      /* 0x54C64 ACQUIRE GOODS (0x6BD98 plays the music itself) */
            setD16(0xF5478u, 0);                      /* 0x54C66 */
            PORTED(f6BD98(), 0x6BD98u);                       /* 0x54C6D DrawTradingCards [CARDS] */
            setD8(PHASE, 12);                         /* 0x54C72 */
            f5E7B4();
            f5E8EC(11);                               /* 0x54C83 */
            if ((D8(FLAGS) & PBEM_BIT) != 0 && S16(0xF5478u) == 1)   /* 0x54C88; 0x54C91 dword [0xF5476] sar 16 */
                PBEM_DROPPED(0x5055Cu);               /* 0x54C9E */
            setD8(0x110F74u, 0);                      /* 0x54CA5 */
            break;
        case 12:                                      /* 0x54CB5 DECLARE OFFERS */
            if (D8(0xF408Cu) != 0) {                  /* 0x54CB5 (PBEM only) */
                f763FC();                             /* 0x54D9E */
                PBEM_DROPPED(0x50D08u);               /* 0x54DA3 */
                break;
            }
            if (f55500() != 0) {                      /* 0x54CC2 */
                setD32(0xF4230u, 1);                  /* 0x54CCF */
                f553C0();                             /* 0x54CD9 */
                if ((D8(FLAGS) & PBEM_BIT) != 0 && S16(0xF5476u) > 1) {   /* 0x54CE4; 0x54CE9 dword [0xF5474] sar 16 */
                    PBEM_DROPPED(0x762E0u);           /* 0x54CF6 */
                    f763FC();                         /* 0x54CFB */
                }
                PORTED(f6F270(), 0x6F270u);                   /* 0x54D02 [TRADE] */
                setD8(PHASE, 13);                     /* 0x54D07 */
                f5E7B4();
                f5E8EC(12);                           /* 0x54D19 */
                for (n = 0; n <= 9; n++)              /* 0x54D1E; 0x54D29 cmp 9; jg 0x54D3C */
                    setD8(0xF09D4u + (uint32_t)n, 0); /* 0x54D34 */
                if ((D8(FLAGS) & PBEM_BIT) != 0)      /* 0x54D3C */
                    PBEM_DROPPED(0x5055Cu);           /* 0x54D49 */
                break;
            }
            if (D32(0xF4230u) != 0) {                 /* 0x54D58 */
                snprintf((char *)DP(0xF5500u), 0x1F4, "Lack of nations with sufficient trade goods (%d min) cancels"
                         " this round of trading.", (int)(D8(0xE1623u) >> 4));   /* 0x54D7B (0xB5967) */
                f7983C(DSTR(0xF5500u));               /* 0x54D88 */
            }
            setD8(PHASE, 15);                         /* 0x54D8D; no autosave */
            break;
        case 13:                                      /* 0x54DB2 COUNTER OFFERS */
            if (D8(0xF408Cu) != 0) {                  /* 0x54DB2; jne 0x54DA3 (PBEM only) */
                PBEM_DROPPED(0x50D08u);               /* 0x54DA3 */
                break;
            }
            f553C0();
            PORTED(f723DC(), 0x723DCu);                       /* 0x54DC2 [TRADE] */
            setD8(PHASE, 14);                         /* 0x54DC7 */
            f5E7B4();
            f5E8EC(13);                               /* 0x54DD7 */
            if ((D8(FLAGS) & PBEM_BIT) != 0 && D32(0x112560u) != 0)   /* 0x54DE2; 0x54DEB */
                PBEM_DROPPED(0x5055Cu);               /* 0x54DF8 */
            break;
        case 14:                                      /* 0x54E07 ACCEPT TRADES */
            if (D8(0xF408Cu) != 0) {                  /* 0x54E07 bh = [0xF408C]; jne 0x54DA3 (PBEM only) */
                PBEM_DROPPED(0x50D08u);
                break;
            }
            f553C0();
            setD8(0x110F74u, 0);                      /* 0x54E16 (bh = 0) */
            setD16(0xF4090u, 0);                      /* 0x54E20 */
            setD16(0xF4100u, 0);                      /* 0x54E27 */
            PORTED(f734E4(), 0x734E4u);                       /* 0x54E2D [TRADE] */
            f5E7B4();
            if (S16(0xF4090u) == 0) {                 /* 0x54E37 dword [0xF408E] sar 16; jne 0x54F85 */
                count = 0;                            /* 0x54E47 */
                if (S8(0x110F74u) != 0 || (D8(FLAGS) & PBEM_BIT) != 0) {   /* 0x54E4A dword [0x110F71] sar 24; 0x54E56 */
                    if ((D8(FLAGS) & PBEM_BIT) == 0)  /* 0x54E63 */
                        f763FC();                     /* 0x54E6C Trading Wrapup */
                    for (n = 1; n <= 9; n++) {        /* 0x54E71; 0x54EB0 cmp 9; jg 0x54EEB */
                        if (S8(0xF09D4u + (uint32_t)n) != 1)   /* 0x54EB9 dword [n + 0xF09D1] sar 24 */
                            continue;
                        if (S8(NATION(n) + 0x9Au) == 3)   /* 0x54ECE dword [199n + 0xF1563] sar 24: human */
                            min = (int8_t)(D8(0xE1623u) >> 4);   /* 0x54EDC */
                        else {
                            int32_t v = D8(0xE1623u) >> 4;   /* 0x54E7A */
                            if (v < 6)                /* 0x54E87 cmp 6; jge */
                                v = 6;                /* 0x54E8C a computer wants 6 at least */
                            min = (int8_t)v;
                        }
                        if (f76F14(n) >= min)         /* 0x54E91 byte [ebp-0x18]; 0x54E97; 0x54E9C movsx; jl */
                            count++;                  /* 0x54EA7 */
                    }
                }
                if ((D8(0xE1625u) & 0x10) != 0)       /* 0x54EEB */
                    need = 2;                         /* 0x54EF4 */
                else
                    need = (S32(0xF40ECu) + 1) / 2;   /* 0x54EFB..0x54F09 signed /2 */
                if (need > count || S16(0xF5476u) >= (int32_t)(D8(0xE1623u) & 0x0F))   /* 0x54F0B jg; 0x54F25 jge */
                    setD8(PHASE, 15);                 /* 0x54F74 */
                else {
                    setD8(PHASE, 12);                 /* 0x54F29 another session */
                    setD16(0xF5476u, (uint16_t)(D16(0xF5476u) + 1));   /* 0x54F30 inc word */
                    if ((D8(FLAGS) & PBEM_BIT) != 0)  /* 0x54F37: sprintf(0xF5500, "%s nations have indicated
                                                         their desire\nto have Trade Session %d", 0xC08AC[count],
                                                         [0xF5476]) and f7983C (0x54F42..0x54F6D) */
                        PBEM_DROPPED(0x54F42u);
                }
                f5E8EC(14);                           /* 0x54F80 */
            }
            if ((D8(FLAGS) & PBEM_BIT) != 0) {        /* 0x54F85 */
                setD16(0xF4100u, 1);                  /* 0x54F92 */
                PBEM_DROPPED(0x5055Cu);               /* 0x54F9B */
            }
            break;
        case 15:                                      /* 0x54FAA CALAMITIES (0x1E0A0 plays the music itself) */
            if ((D8(FLAGS) & PBEM_BIT) != 0) {        /* 0x54FAA */
                setD8(PHASE, 14);                     /* 0x54FB3 */
                f553C0();                             /* 0x54FBA */
                setD8(PHASE, 15);                     /* 0x54FBF */
                f763FC();                             /* 0x54FC6 */
                PBEM_DROPPED(0x762E0u);               /* 0x54FCB */
            }
            setD16(0xF5476u, 1);                      /* 0x54FDD */
            setD8(0xF408Cu, 0);                       /* 0x54FE4 */
            setD16(0xF4094u, 0);                      /* 0x54FEA */
            setD16(0xF7AFCu, 0);                      /* 0x54FF3 */
            setD16(0xF7AFEu, 0);                      /* 0x54FFA */
            PORTED(f1E0A0(), 0x1E0A0u);                       /* 0x55000 [CAL1] */
            setD16(0xF7AFCu, 0);                      /* 0x55005 */
            setD16(0xF7AFEu, 0);                      /* 0x5500C */
            setD8(PHASE, 16);                         /* 0x55013 */
            f5E7B4();
            f5E8EC(15);                               /* 0x55023 */
            if ((D8(FLAGS) & PBEM_BIT) != 0)          /* 0x55028 */
                for (n = 0; n < 9; n++) {             /* 0x55035; 0x55042 cmp 9; jge 0x55222 */
                    memcpy(DP(0xF0898u + 25u * (uint32_t)n), DP(0xF07B4u + 25u * (uint32_t)n), 25);   /* 0x5507B */
                    setD16(0xF07B4u + 25u * (uint32_t)n, 0);   /* 0x5508C */
                }
            break;
        case 16:                                      /* 0x55096 MONOTHEISM (0x21914 plays the music itself) */
            PORTED(f21914(), 0x21914u);                       /* 0x55098 [MONO] */
            setD8(PHASE, 17);                         /* 0x5509D */
            f5E7B4();
            f5E8EC(16);                               /* 0x550AD */
            break;
        case 17:                                      /* 0x550BC CITY SUPPORT (again) */
            f553C0();
            setD16(0x109E20u, 0);                     /* 0x550C5 */
            PORTED(f5C62C(), 0x5C62Cu);                       /* 0x550CC [CITY] */
            setD16(0xF7AFCu, 0);                      /* 0x550D1 */
            setD16(0xF7AFEu, 0);                      /* 0x550D8 */
            setD8(PHASE, 18);                         /* 0x550DF */
            f5E7B4();
            f5E8EC(17);                               /* 0x550EF */
            break;
        case 18:                                      /* 0x550FE ACQUIRE CIVILIZATION ADVANCES */
            f553C0();
            setD16(0xF4092u, 0);                      /* 0x5510C */
            PORTED(f162D0(), 0x162D0u);                       /* 0x55113 Acquire Civ [ADV] */
            setD32(0xF09E4u, 0);                      /* 0x55118 */
            for (n = 1; n <= 9; n++)                  /* 0x5511E; 0x55129 cmp 9; jg 0x5514A */
                if (D8(NATION(n) + 0x28u) != 0) {     /* 0x55136 byte [199n + 0xF14F4] (in play or not) */
                    setD32(0xF09E4u, 1);              /* 0x55140 */
                    break;
                }
            if ((D8(FLAGS) & PBEM_BIT) == 0)          /* 0x5514A */
                PORTED(f1A3A0(), 0x1A3A0u);                   /* 0x55153 Tool Acquisition Wrapup [ADV] */
            setD8(PHASE, 19);                         /* 0x55158 */
            f5E7B4();
            f5E8EC(18);                               /* 0x55169 */
            break;
        case 19:                                      /* 0x55178 DISCARD EXCESS */
            f553C0();
            setD16(0xF4096u, 0);                      /* 0x5517F */
            PORTED(f6E770(), 0x6E770u);                       /* 0x55186 [CARDS] */
            setD8(PHASE, 20);                         /* 0x5518B */
            f5E7B4();
            f5E8EC(19);                               /* 0x5519C */
            if ((D8(FLAGS) & PBEM_BIT) != 0 &&        /* 0x551A1 */
                (D16(0x109E20u) != 0 || D16(0xF4092u) != 0 || D16(0xF4096u) != 0))   /* 0x551AE..0x551CA */
                PBEM_DROPPED(0x5055Cu);               /* 0x551CC */
            break;
        case 20:                                      /* 0x551DB SHOW AST, the end of the round */
            f553C0();
            copy_file("savgamea.dat", "savgameb.dat");   /* 0x551E5 system("copy savgamea.dat savgameb.dat >NUL:") */
            f55258();                                 /* 0x551EA */
            if ((D8(FLAGS) & PBEM_BIT) != 0)          /* 0x551EF */
                PORTED(f1A3A0(), 0x1A3A0u);                   /* 0x551F8 [ADV] */
            done = f136B0(3);                         /* 0x55202 */
            if (done == 0) {                          /* 0x5520A; jne 0x55222 */
                setD8(PHASE, 0);                      /* 0x55217 */
                f5E8EC(20);                           /* 0x5521D (no f5E7B4 here) */
            }
            break;
        default:                                      /* above 20: only 0x22B00 */
            break;
        }
        f22B00();                                     /* 0x55222 (and each case's own call) */
    }
    PORTED(f14294(), 0x14294u);                               /* 0x5522C Final Civilization Standings [END] */
    f55304();                                         /* 0x55231 */
    (void)remove("savgameb.dat");                     /* 0x5523D (0xB5A20) */
    setD8(0xF5500u, 0);                               /* 0x55242 */
    f44CAC();                                         /* 0x55248 quit to DOS: main's 0x118CE is never reached */
}

/* ---- main 0x1188C..0x118C9 --------------------------------------------------------------------------------------- */

static _Noreturn void main_1188C(void)
{
    uint8_t bh = D8(FLAGS);                           /* 0x1188C */
    uint8_t ch;

    if ((bh & 0x04) == 0 && (bh & PBEM_BIT) == 0)     /* 0x11892 jne 0x118A1; 0x11897 jne 0x118A1 */
        f14AA4();                                     /* 0x1189C */
    ch = D8(FLAGS);                                   /* 0x118A1 */
    if ((ch & 0x04) != 0 && (ch & PBEM_BIT) != 0)     /* 0x118A7 je 0x118C9; 0x118AC je 0x118C9 */
        /* 0x118B1 f14A00, 0x118B6 f4FE30, 0x118BB int16 [0xF408A] = -0x5C, 0x118C4 jmp 0x11168: a new PBEM game */
        PBEM_DROPPED(0x14A00u);
    f54824();                                         /* 0x118C9 */
}

_Noreturn void game_loop_restored(void)
{
    main_1188C();
}

_Noreturn void game_loop_new(void)
{
    main_1188C();
}
