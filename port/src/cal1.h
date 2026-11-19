#ifndef CAL1_H
#define CAL1_H

/* Module CAL1 of docs/M6_PLAN.md (cal1.c), the human side: the calamity phase 0x1E0A0 (phase 15 of the game loop
   0x54824, call at 0x55000) with Zeus 0x1E388, its helpers 0x1E340 / 0x1E5B8 / 0x1E69C, and the city-loss calamities
   Treachery 0x20564 (with the human's territory chooser 0x207B4 and the result 0x20FC4), Superstition 0x23230 (and
   0x23574), Slave Revolt 0x235E4, Civil Disorder 0x237A8, Iconoclasm & Heresy 0x23ACC, Piracy 0x241D8, the city
   chooser ChooseCitiesToLoseToPirates 0x2468C (also the Tax Revolt's, mode 2) and the choice-list builder 0x28CFC.
   The computers' parts (0x213C0, 0x20C70, 0x24EB8, 0x252BC, 0x25548, 0x2E1D0: M7), the animations (ANIM) and the
   other calamities (CAL2..CAL5) are not ported yet: their call sites stop the port through not_ported(address); the
   list is in cal1.c's file comment.

   tax.c and pop.c still carry static not_ported stubs named f2468C / f28CFC (and pop.c calls not_ported(0x28CFC)
   directly): this header exports the real ones, so those stubs are to be replaced by an #include "cal1.h" (Claude,
   when these files are merged; this module does not touch tax.c or pop.c).

   Conventions: those of ui.h, game.h, kit.h and rules.h (fXXXXX = the original at 0xXXXXX, Watcom register arguments
   eax, edx, ebx, ecx, then the stack; void unless a caller reads eax). D = read in the code, G = inference.

   Globals (D):
     the calamity list 0xF77E4 + 3k {uint8 card id, int8 giver (the nation that traded it, -1 = drawn), uint8 victim}
       with its count int32 [0xF7808] (built by 0x1E5B8; 12 records fit before the count, see cal1.c)
     int32 [0xF783C] the calamity being resolved (the animation's id); int32 0xF4204[n] 1 = more than 75% of n's tokens
       are on the board (written by 0x1E0A0, read by the AI); int16 0xF409A[10] Grain spent against Famine (cleared)
     int16 [0xF4094] 1 = a calamity card was resolved this phase (0x1E69C; the loop clears it)
     int16 [0xF7AFE] / [0xF7AFC] the Slave Revolt victim and its token loss (city.c's 0x5C62C / 0x5C750 read them)
     int16 0xF796C[10] Iconoclasm's secondary losses per nation; uint8 0xF547C[14] the assigned losses (conflict.h)
     uint8 0xF7AC4[12] the nations whose coastal cities the Pirates may take (0x2468C, read by 0x28CFC kind 0x1E)
     the choice list int32 [0xF7C9C] / 0xF7CA0[157] (city.h); int32 [0xF40F8] the current nation
     records: rules.h (nation, territory state), conflict.h (area record +0x3B + n "lost territory" marks) */

#include <stdint.h>

/* ---- the phase ------------------------------------------------------------------------------------------------- */

void f1E0A0(void);         /* D: CALAMITIES: memset(0xF409A, 0, 20); no calamity held (f1E340 == 0): f79270(100, int16
                              [0xF06B0], -200, "", 0) and return (the player list is not rebuilt on that path). Else
                              f553C0, the message area (0xA9, 0x154, 0x12C, 0x23), Zeus f1E388 unless bit 0x10 of
                              [0xE1624], f1E5B8, the player list (nations 1..9 with +0x9A != 0), f783B8(0xF596C,
                              [0xF5940]), 0x7A4A0 (ANIM); for each record k < [0xF7808] whose victim is in play:
                              0xF4204[v] = board * 100 / [0xF4898] > 75, f7C960, [0xF783C] = id, the handler by id
                              (jump table 0x1E070: 0x13 0x1E7A0, 0x14 f20564, 0x15 0x25A38, 0x16 f23230, 0x17 0x29E34,
                              0x18 f235E4, 0x19 0x2D4F0, 0x1A 0x217A0, 0x1B 0x2E59C, 0x1C f237A8, 0x1D f23ACC, 0x1E
                              f241D8); after every record (also a skipped one) f22B00 and f5E7B4. One caller (the game
                              loop, 0x55000); eax unused */

/* ---- the helpers --------------------------------------------------------------------------------------------- */

int  f1E340(void);         /* D: 1 if a nation 1..9 holds a calamity (+0x13 != 0), else 0 (no +0x9A test) */
void f1E388(void);         /* D: Zeus: each nation 1..9 in play with more than two calamities discards count - 2 of
                              them at random (game_rand once per discard: idx = rand * count >> 15), each to its pile
                              (f771F8); "Zeus takes pity upon %s and discards A, B and C." through f79270(-500, n, -200,
                              text, -298). BUG(1995) fixed unless g_original: the givers +0x1C.. were not shifted with
                              the cards (cal1.c) */
void f1E5B8(void);         /* D: the calamity list in resolution order: for id 0x13..0x1E, nations 1..9 with +0x13 != 0,
                              slots j 0..7 with +0x14 + j == id: {id, +0x1C + j, n}; [0xF7808] = the count */
void f1E69C(int k);        /* D: calamity record k resolved: int16 [0xF4094] = 1; the card taken out of its victim's
                              hand (both lists shifted down, count - 1, the old last slots 0 / 0xFF) and discarded
                              (f771F8). BUG(1995) fixed unless g_original: a card not in the hand (cal1.c) */

/* ---- the calamities (eax = the record index k of 0xF77E4) ---------------------------------------------------- */

void f20564(int k);        /* D: Treachery: no cities -> message; else the giver p of the victim's Treachery card
                              (+0x1C + j; -1: p = v); a human p: t = f207B4(p, v, 0x14); else the message and t =
                              0x20C70(p, v) - 1 (AI); p != v with < 9 cities: area t+1 +0x3B + v = 1; f3A990, f3CBBC,
                              f20FC4(t, p, v), f1E69C(k) */
int  f207B4(int p, int v, int kind);   /* D: human p picks a territory of victim v: kind 0x14 Treachery (v's cities;
                              "REPLACE CITY" / "ELIMINATE CITY" / "REDUCE CITY" on button 0x58 of 0xBDE30), 0x19 Flood
                              (v's coastal cities), 0x1A Barbarians (the caller's list). Arrows -9 / -10, map clicks, the
                              action button -16 returns the territory. Callers read eax (0x20564, 0x1D234, 0x2D4F0) */
void f20FC4(int t, int p, int v);      /* D: Treachery's result in territory t: p != v: v's city to p (p < 9 cities) or
                              eliminated; p == v: the city reduced to min(limit + Agriculture, stock) tokens. Message
                              (2000 ms) for a computer p (or PBEM). Text fixed unless g_original: "replaced by a African
                              city" (the article the wrong way round) */
void f23230(int k);        /* D: Superstition: 3 cities (2 Mysticism, 1 Deism, 0 Enlightenment), all of them when fewer;
                              the human reduces through f5C880, the computer through 0x213C0 (AI). Text "nullifys" fixed
                              unless g_original */
int  f23574(int n);        /* D: 0x213C0(n) (AI) if it is one of n's cities (t + 1), else -1 */
void f235E4(int k);        /* D: Slave Revolt: int16 [0xF7AFE] = v, [0xF7AFC] = 15 (20 with Mining, -5 with
                              Enlightenment, at most the tokens on the board); city support f5C750 when 2 * cities >
                              board - loss */
void f237A8(int k);        /* D: Civil Disorder: cities - 3, -1 per Music, Drama & Poetry, Law, Democracy, +1 per
                              Roadbuilding, Military; the human through f5C880, the computer through 0x213C0 (AI) */
void f23ACC(int k);        /* D: Iconoclasm & Heresy: 4, -1 Law, -1 Philosophy, -3 Theology, +1 Monotheism, +1
                              Roadbuilding (0..cities); then up to 2 secondary losses over the other nations without
                              Theology (min(cities, 2), 1 with Philosophy; not the trader): 0x291D0 / 0x29550 (CAL2),
                              0x27518 (AI); each reduced through f5C880 or 0x213C0. Text "%s reduces a city in %s ."
                              fixed unless g_original */
void f241D8(int k);        /* D: Piracy: the Pirates (nation 10) take up to 2 coastal cities of the victim (the trader
                              chooses: f2468C mode 0, or 0x24EB8 AI) and up to 2 of the others (the victim chooses: mode
                              1, or 0x252BC AI), within 9 Pirate cities ("Skull & Crossbones, Inc." when full). Text
                              "takeover" fixed unless g_original */

/* ---- the choosers -------------------------------------------------------------------------------------------- */

/* 0x2468C ChooseCitiesToLoseToPirates: human c takes k cities (mode 0: v's coastal cities for the Pirates; 1: the
   secondary victims' coastal cities, one per nation, chosen by v; 2: Tax Revolt, v's cities to c). Snapshots of the
   areas / territory states / nations in [0xEED74] / [0xEED7C] / [0xEED78] (port handles) for RESTART, dialog 0xBEE78
   (arrows -9 / -10, RESTART -0x63, REMOVE -0x65, FINISHED -1), map clicks select. Callers: 0x241D8 (Piracy), tax.c's
   0x6685C (Tax Revolt, mode 2). BUG(1995) fixes unless g_original: see cal1.c */
void f2468C(int c, int v, int k, int mode);

/* 0x28CFC: the choice list 0xF7CA0 / [0xF7C9C] (entries 0..156 first -1) for nation n by kind: 0x15, 4, 9 n's cities
   or tokens; 0x1B n's cities or more than one token; 7 byte 0xF7984[t]; 8 byte 0xF7A24[t]; 6 n's tokens; 3 an
   allowance area +0x24 + n; 2, 5 n's cities; 0x1E the cities of the nations marked in 0xF7AC4 on coastal territories;
   0x19 the territories of river s (table 0xBEE08 + 22s) with a flood-plain city of n (territory +1 bit 0x80) or n's
   tokens; other kinds an empty list. Callers: 0x2468C, 0x24EB8, 0x260C4, 0x27FCC, 0x2B6EC, 0x2C190, 0x2C894, tax.c's
   0x66D8C, pop.c's 0x67758 */
void f28CFC(int n, int kind, int s);

#endif
