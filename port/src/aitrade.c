/* Module AITRADE of M7 batch 2 (docs/M6_PLAN.md, module TRADEAI): the computer players' side of the trade steps.
   Literal translations; the address of each instruction a line comes from is in its comment. D = read in the code,
   G = inference. Prototypes: aitrade.h.

     0x68730  f68730  DECLARE OFFERS: one computer declares (trade.c 0x6F32B)              88 bytes
     0x68788  f68788  the ranked list of held commodities                                 732
     0x68A64  f68A64  which stacks the other nations hold                                 224
     0x68B44  f68B44  how many and which cards to ask for                                 332
     0x68C90  f68C90  the declared offer (game_rand x1 or x2, f6937C, f76F70)             844
     0x68FDC  f68FDC  the most harmful calamity held                                      208 (+ 0x690AC's jump table)
     0x6937C  f6937C  the stated bluff card(s) (game_rand per card, recursive)          1,320
     0x698A4  f698A4  COUNTER OFFERS: one computer answers (trade.c 0x724E8)            3,416
     0x6A5FC  f6A5FC  one counter-offer row (game_rand x1, f6937C)                      1,908
     0x6AD70  f6AD70  impossible cards out of the copied offers                           452
     0x6AF34  f6AF34  the same for the response list (clears in the wrong table, kept)    440
     0x6B0EC  f6B0EC  ACCEPT TRADES: one computer accepts (trade.c 0x73887)             2,108

   Ported callees (never re-translated here): f690AC, f6B928, f7CAB4 (formulas.c), f6DD98 (ast.c), f194E4, f195D8
   (info.c), f76F70 and game_rand f78AE4 (newgame.c), f75074 (trade.c). No call into another batch-2 module, so no
   stubs and no -DAITRADE_TEST recorders. PBEM_DROPPED: none (no PBEM test or call in these twelve functions, D).
   No clock(), no screen, no text, no trace string (D: the calls listed and __CHK 0x7D4AC (dropped), memset 0x7D5D0
   (-> memset on the data object); the record copies rep movsd/movsw/movsb -> memmove / memcpy).

   game_rand (risk 2), every draw in the original's order: f68C90 one (0x68CC9 / 0x68CE8), a second in the last
   session without a bad calamity (0x68D41), then f6937C's, then f76F70's (one per wanted card when there are 2+);
   f6937C one per card it states (0x6971C), plus its recursive call's; f6A5FC one (0x6A716 / 0x6A735), then f6937C's.
   All rolls are game_rand()*100 >> 15 (lea/shl: 100r, shr 15 unsigned).

   Data (D, read in the code):
     nation record n: 0xF14CC + 199n: +1..+0x12 commodities, +0x13 calamity count, +0x14.. ids, +0x29 Mining,
       +0x51 cities, +0x9A 0 out / 2 computer / 3 human, +0x9C..+0xA8 the declared offer (slots 0, 1 truly stated,
       2 the stated bluff, 5..9 wanted, 10 the card really given for the bluff), +0xA8 + m, +0xB1 + m (trust),
       +0xBB, +0xBC (honesty: 1 = mostly truthful), +0xBD + m. Record 0 is the scratch hand (rewritten many times).
     slot offsets: the bytes 0xCA9A0.. (trade.c's tables), read here as int8 at the byte the original's dword..sar 24
       lands on: 0xCA9A1 = 2, 0xCA9A2 = 5, 0xCA9A3 = 10, 0xCA9AC = 0, 0xCA9AD = 2, 0xCA9AE = 5, 0xCA9AF = 10,
       0xCA9B0 = 5 (the data object's values; read through it, never hard-coded).
     ranked list 0x1101AC + 16e (18 entries, count [0x1103EC]): +0 card id, +4 value, +8 count held, +0xC count used
       for the value; [0x1103F0] = 0 (written, never read here); [0x11059C] the first entry offered (= the want count).
     0x1105A0[9] stacks held by other nations (f68A64); [0x1105AC + 4n] want count (saved; can become -1, see below);
     [0x1105D4 + 4n] the highest stack worth asking for (saved, session 1 only); [0x1105FC] the last f6B928 result;
     [0x110600] / [0x110604] the worst calamity's score / id; [0x1106B0 + 4n] "will be generous" flag (saved).
     0x110608 + 21i: the copy of the public offers 0x110ECC (count [0x112560]); +0x12 the nation.
     candidates 0x1103F4 + 28k (count [0x1104B8]): +0 matches, +4 3, +8 gain, +0xC partner, +0x10 offer index,
       +0x14 never written by f698A4 (D: only 0x6970B / 0x6982E read 0x110408..; 0 in the data object for entries
       0..6, so there a = 1 and third = -1 in f6937C's counter path; but entry 7's +0x14 is 0x1104CC = the accept
       table's [0]+0x10, written by f6B0EC at 0x6B721, and the sorts move entry 7 down: read through the data object),
       +0x18 value. Entry 7's +0 is the count [0x1104B8] itself, entries 7.. run into the accept table: kept through
       the data object (only with 8 offers from others, a 9-player game, G).
     accept table 0x1104BC + 28i (n's responses): +0 gain, +4 value, +8 response index, +0xC partner, +0x10 margin,
       +0x14 3, +0x18 matches.
     counter-offer rows 0x111118 + 16r (count int16 [0x112548], saved, 56 rows): +0..+1 truly stated, +2..+4 bluffs,
       +5..+7 replacements, +0xD the offering nation, +0xE the responder. Response list 0x111498 + 20i (count int16
       [0x112568]): +0..+0xF a copy of a row. Rows past 56 land on the response list (data object, as the original).
     0xC8DA0 int32[19] want counts by list size; 0xC8D40 / 0xC8D4C int32 by AI level (thresholds); 0xC8D58 int32[19]
       the cards per stack; 0xBDD3C bytes {0, 4, 12, 11, 16, 18} (the Mining goods); int16 0x112C9A + 2n (dword
       [0x112C98 + 2n] sar 16: the inventory's "0x112C98" is the word two bytes on; no writer, D); int32 0xF590C[n];
       0xF52E0 + 40n + 4m int32; [0xF52D4] the leader; byte [0xF4102] the phase (0x0C DECLARE OFFERS).
   Not saved (state outside the save, risk 9): 0x1101AC.., [0x1103EC], [0x1103F0], 0x1103F4.., [0x1104B8], 0x1104BC..,
   [0x11059C], 0x1105A0, [0x1105FC], [0x110600], [0x110604], 0x110608... Saved: the nation records (record 0 too),
   0x111118 rows and [0x112548], 0x111498 (7 records) and [0x112568], 0x1105D4, 0x1105AC, 0x1106B0.

   Stack words read before they are written (batch 1's rule applied first), fixed in both modes; word 2 is not a
   bug (noted after the list):
     1. f6937C's card [ebp-0x3C] (0x697EA) when no candidate was ever tried (count <= 0, or top < 1: 0x1105A0[0] == 0).
        Not a sibling's word: game_rand (0x78AE4) and __CHK, called just before at the same depth, never reach
        [ebp-0x3C] (game_rand: 3 words). The word is left by older, deeper frames (f6AD70's table, f195D8, the previous
        f6937C, for DECLARE OFFERS the screens of 0x6F270), which a port cannot follow. G, reachability: in DECLARE
        OFFERS top < 1 means an empty ranked list, so slot 10 was empty and flag = 1 returns before the word is used;
        in COUNTER OFFERS count is always 1 ([0x1103F8 + 28k] = 3) and 0x1105A0[0] is 1 because n or the offering
        nation holds a countable card. BUG(1995) (D code / G unreachable): the port reads 0 (no recursion) in both modes.
     3. 0x6AD70 / 0x6AF34's [10][9] tables ([ebp-0x1FC] / [ebp-0x1D4]): rows of nations not in play are never written,
        and a card c >= 19 reads row m + 1 ((c - 1) / 2 >= 9): stack garbage when m + 1 is not in play, the next
        local array (held[], memset) for m = 9, and past it the locals q, i, c, n and the record pointer (D frame).
        The locals are modelled; unwritten rows read 0 in both modes (BUG(1995), D code / G: offers and responses hold
        only commodities, except a human's card 19 / 20 from trade.c's chart slip under --original).
   Noted, not bugs (nothing to fix; word 2 keeps its number for the code's comments):
     (2) f698A4's [ebp-0x34] (out of f6A5FC) is read at 0x6A3C1 even if f6A5FC never ran, but only together with
        added > 3, which needs four f6A5FC calls: no effect (D). The port starts it at 0.
     f6B0EC's [0x1104C8 + 28i] read before it is written (0x6B496, M6_PLAN risk 7) is not stack garbage: it is the
     data-object word the previous f6B0EC (or f698A4's candidate 7 + i) left, 0 after a fresh start. Kept literally.

   1995 bugs (graded code / effect). The AI's odd weights and slips stay literal (they only steer its choices):
     4. f698A4's stack copy of the rows (0x6A537): 16 x [0x112548] bytes into a 56-row buffer [ebp-0x5EC]; rows 56..
        overwrite the pair list [ebp-0x26C] that the next loop reads, rows 91.. the counts and the return address
        (D / G: 9 players answering nearly every offer give up to 72 rows). --original models the frame for up to 90
        rows (byte-exact inside [ebp-0x5EC, ebp-0x3C): the overwritten pairs are read as the original reads them; a
        row index read from an overwritten pair that points past that range, a write the original makes into the
        counts, the caller's frames or beyond, is skipped); fixed (and --original past 90 rows, a crash in the
        original): a separate buffer. BUG(1995).
   Kept as the original (D code, G effect unless marked):
     - f68B44 can set the want count to -1 (a one-card list worth < 3: 0xC8DA0[1] = 0); f68730 then still builds an
       offer. f698A4 computes the same count but writes only [0x11059C] (0 then), never the want count (D).
     - f68788's Mining test reads 0xBDD3C[0..5] (entry 0 is 0, never a card).
     - f6937C, f6A5FC and f68C90 write the stated card from fixed slot bytes; f6AD70 / f6AF34 index past a 9-entry row.
     - f6AF34 reads the response list 0x111498 (20-byte records) but clears 0x110608 + 21i + q (M6_PLAN item 11): the
       filter never reaches the responses; 0x110608 is rebuilt by the next f698A4 (D: 0x69A28).
     - f6B0EC's trust is n's trust in itself (+0xB1 + the response's +0xD, the offerer = n, D 0x6B427).
     - f698A4 / f6B0EC leave the value field (+0x18 / +4) unwritten when the expected gain equals the gain (data
       object, kept). f6B0EC decrements its stack copy at the three offered card ids without checks (id 0 hits byte 0).
   Guards (port, no change in play): the pair list stops at 59 pairs (the original would run into its loop index; G at
   most 8); a frame-model or data-object index outside its array reads 0 / is not written.
   Easter eggs: none (no strings in these functions, D). New oddity (D): f6B0EC weighs a response by n's trust in
   itself, f698A4 by its trust in the partner. */

#include <stdint.h>
#include <string.h>

#include "aitrade.h"
#include "formulas.h"
#include "game.h"
#include "info.h"
#include "state.h"
#include "trade.h"

#define NAT(n)       (0xF14CCu + 199u * (uint32_t)(n))
#define ENT(e)       (0x1101ACu + 16u * (uint32_t)(e))
#define CAND(k)      (0x1103F4u + 28u * (uint32_t)(k))
#define ACC(i)       (0x1104BCu + 28u * (uint32_t)(i))
#define OCOPY(i)     (0x110608u + 21u * (uint32_t)(i))
#define ROW(r)       (0x111118u + 16u * (uint32_t)(r))
#define RESP(i)      (0x111498u + 20u * (uint32_t)(i))
#define LIST_CNT     0x1103ECu   /* int32 */
#define LIST_FIRST   0x11059Cu   /* int32 */
#define HELD_BY      0x1105A0u   /* uint8[9] */
#define WANTS        0x1105ACu   /* int32[n] */
#define TOPSTACK     0x1105D4u   /* int32[n] */
#define GAP          0x1105FCu   /* int32 */
#define CALVAL       0x110600u   /* int32 */
#define CALID        0x110604u   /* int32 */
#define GENEROUS     0x1106B0u   /* int32[n] */
#define CAND_CNT     0x1104B8u   /* int32 */
#define NOFFERS      0x112560u   /* int32 */
#define CROW         0x112548u   /* int16: dword [0x112546] sar 16 */
#define NRESP        0x112568u   /* int16: dword [0x112566] sar 16 */
#define SESSION      0xF5476u    /* int16: dword [0xF5474] sar 16 */
#define PHASE        0xF4102u    /* uint8 */
#define BOUGHT       0xF590Cu    /* int32[n] */
#define LEADER       0xF52D4u    /* int32 */
#define OFF(a)       ((int32_t)S8(a))   /* a slot offset byte of 0xCA9A0.. (dword [a - 3] sar 24) */

/* the AI level: mov ax, word [0xE1624]; shl eax, 4; shr ax, 0xE */
static int32_t ai_level(void)
{
    return (int32_t)((uint16_t)(D16(0xE1624u) << 4) >> 14);
}

/* game_rand() * 100 >> 15 (mov edx, eax; shl 2; sub; shl 3; add; shl 2; shr 0xF) */
static int32_t roll100(void)
{
    uint32_t r = (uint32_t)f78AE4();
    return (int32_t)((r * 100u) >> 15);
}

/* 32-bit imul (wraps as the 386 does) */
static int32_t mul(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a * (uint32_t)b);
}

static void inc8(uint32_t a) { setD8(a, (uint8_t)(D8(a) + 1u)); }
static void dec8(uint32_t a) { setD8(a, (uint8_t)(D8(a) - 1u)); }

/* rep movsd 0x31 + movsw + movsb: a nation record */
static void copy_nation(uint32_t dst, uint32_t src)
{
    memmove(DP(dst), DP(src), 199);
}

static void swap_bytes(uint32_t a, uint32_t b, size_t n)   /* the stack-temp swaps (movsd x n / rep movsd 7) */
{
    uint8_t tmp[28];
    memcpy(tmp, DP(b), n);
    memmove(DP(b), DP(a), n);
    memcpy(DP(a), tmp, n);
}

/* ================================================================================================================ */

/* 0x68A64: 0x1105A0[s] = 1 when another nation in play holds a card of stack >= s (the >= 7 rule for buyers) */
void f68A64(int n, int unused)
{
    int32_t m, i, b;

    (void)unused;                                                   /* 0x68A79 [ebp-0x14] = edx, never read */
    memset(DP(HELD_BY), 0, 9);                                      /* 0x68A88 */
    for (m = 1; m <= 9; m++) {                                      /* 0x68A8D, 0x68A9C jg */
        if (D8(NAT(m) + 0x9Au) == 0)                                /* 0x68AAD mov dl; test */
            continue;
        if (m == n)                                                 /* 0x68ABA */
            continue;
        for (i = 18; i >= 1; i--) {                                 /* 0x68ABF, 0x68ACE jl */
            if (S8(NAT(m) + (uint32_t)i) == 0)                      /* 0x68ADE dword [.. + 0xF14C9] sar 24 */
                continue;
            b = S32(BOUGHT + 4u * (uint32_t)m);                     /* 0x68AEE */
            if (b != -1 && S32(BOUGHT + 4u * (uint32_t)m) >= 7 && (i - 1) / 2 >= 7)   /* 0x68AF5, 0x68B04, 0x68B16 */
                continue;
            memset(DP(HELD_BY), 1, (size_t)((i - 1) / 2 + 1));      /* 0x68B28 lea ebx,[eax+1]; 0x68B35 */
        }
    }
}

/* 0x68788: n's ranked list of held commodities */
void f68788(int n)
{
    uint32_t rec = NAT(n);                                          /* [ebp-0x1C] */
    int32_t i, s, e, c, j, k, mining, v, used;

    f68A64(n, S8(rec + 0xBBu));                                     /* 0x687B4 dword [rec + 0xB8] sar 24; 0x687C0 */
    if (S16(SESSION) == 1) {                                        /* 0x687C5..0x687D0 */
        for (i = 18; i >= 1; i--) {                                 /* 0x687D6, 0x687E5 jl */
            if (S8(rec + (uint32_t)i) == 0)                         /* 0x687F1 movsx */
                continue;
            if (S32(BOUGHT + 4u * (uint32_t)n) >= 7 && (i - 1) / 2 >= 7)   /* 0x687FB, 0x68814 */
                continue;
            s = (i - 1) / 2;                                        /* 0x68826 */
            if (s - 1 > S8(rec + 0x51u))                            /* 0x68830 dword [+0xF151A] sar 24; 0x6883F jg */
                continue;
            setD32(TOPSTACK + 4u * (uint32_t)n, (uint32_t)s);       /* 0x68847 */
            break;
        }
    }
    memset(DP(ENT(0)), 0, 0x120);                                   /* 0x68861 */
    setD32(LIST_CNT, 0);                                            /* 0x68866 */
    setD32(0x1103F0u, 0);                                           /* 0x6886C */
    for (i = 18; i >= 1; i--) {                                     /* 0x68872, 0x68960 jl */
        if (D8(rec + (uint32_t)i) == 0)                             /* 0x6896C mov dl; test */
            continue;
        if (D8(HELD_BY + (uint32_t)((i - 1) / 2)) == 0)             /* 0x6897F mov dh; test */
            continue;
        e = S32(LIST_CNT);                                          /* 0x68989 */
        setD32(ENT(e), (uint32_t)i);                                /* 0x68995 +0 */
        c = S8(rec + (uint32_t)i);
        setD32(ENT(e) + 8u, (uint32_t)c);                           /* 0x689A6 +8 */
        mining = 0;                                                 /* 0x689AC [ebp-0x24] */
        for (j = 0; j < 6; j++) {                                   /* 0x689AF, 0x68880 */
            if (S8(0xBDD3Cu + (uint32_t)j) == i) {                  /* 0x68889 dword [j + 0xBDD39] sar 24 */
                mining = 1;                                         /* 0x68899 */
                break;
            }
        }
        if (i == 1 || i == 2) {                                     /* 0x688A0, 0x688A6 */
            v = mul(mul(c, c), (i + 1) >> 1);                       /* 0x688BE, 0x688C5, 0x68934 */
            used = c;                                               /* 0x6894B */
        } else if (D8(rec + 0x29u) != 0 && mining != 0) {           /* 0x688CC, 0x688D3 */
            v = mul(mul(c + 2, c + 2), (i + 1) >> 1);               /* 0x688F1, 0x688FA */
            used = c + 1;                                           /* 0x68914 */
        } else {
            v = mul(mul(c + 1, c + 1), (i + 1) >> 1);               /* 0x6892B, 0x68934 */
            used = c;
        }
        setD32(ENT(S32(LIST_CNT)) + 4u, (uint32_t)v);               /* 0x68905 / 0x6893F */
        setD32(ENT(S32(LIST_CNT)) + 0xCu, (uint32_t)used);          /* 0x6894E */
        setD32(LIST_CNT, (uint32_t)(S32(LIST_CNT) + 1));            /* 0x68954 */
    }
    for (k = S32(LIST_CNT) - 2; k >= 0; k--)                        /* 0x689B7..0x689C6, 0x689D3 */
        for (j = 0; j <= k; j++)                                    /* 0x689F0 jg */
            if (S32(ENT(j + 1) + 4u) > S32(ENT(j) + 4u))            /* 0x68A04 [+0x1101C0], 0x68A0A [+0x1101B0] */
                swap_bytes(ENT(j), ENT(j + 1), 16);                 /* 0x68A23..0x68A55 */
}

/* 0x68B44: how many cards n asks for, and which */
void f68B44(int n)
{
    uint32_t rec = NAT(n);
    int32_t k, off, sum, j;

    k = S32(0xC8DA0u + 4u * (uint32_t)S32(LIST_CNT));               /* 0x68B5F [ebp-0x14] */
    off = OFF(0xCA9A2u);                                            /* 0x68B69 dword [0xCA99F] sar 24 */
    memset(DP(rec + 0x9Cu + (uint32_t)off), 0, 5);                  /* 0x68B8F */
    if (S32(LIST_CNT) < 8) {                                        /* 0x68B94 jge */
        sum = 0;                                                    /* 0x68B9F */
        for (j = k; j < S32(LIST_CNT); j++)                         /* 0x68BA5, 0x68BB0 */
            sum += S32(ENT(j) + 8u);                                /* 0x68BC1 */
        if (S32(CALVAL) < 0)                                        /* 0x68BCC */
            sum++;
        if (sum < 3)                                                /* 0x68BDB */
            k--;                                                    /* 0x68BE4 */
    }
    if (S32(CALVAL) < -1 || f7CAB4(n) != 0) {                       /* 0x68BE7, 0x68BF3 */
        if (k > 3)                                                  /* 0x68BFF, 0x68C28 jle */
            k = 3;
    } else if (S16(0x112C9Au + 2u * (uint32_t)n) / 300 < 25) {      /* 0x68C07 dword [2n + 0x112C98] sar 16; idiv */
        if (k > 3)
            k = 3;                                                  /* 0x68C2A */
    }
    setD32(WANTS + 4u * (uint32_t)n, (uint32_t)k);                  /* 0x68C37 */
    if (k != 0) {                                                   /* 0x68C3E */
        for (j = 0; j < k; j++)                                     /* 0x68C46, 0x68C49 */
            setD8(rec + (uint32_t)(off + j) + 0x9Cu, D8(ENT(j)));  /* 0x68C57, 0x68C72 */
        setD32(LIST_FIRST, (uint32_t)j);                            /* 0x68C83 (0 when k < 0) */
    }
}

/* 0x68FDC: the most harmful calamity n holds */
void f68FDC(int n)
{
    uint32_t rec = NAT(n);
    int32_t i, v;

    setD32(CALVAL, 0);                                              /* 0x68FF4 */
    setD32(CALID, 0);                                               /* 0x68FFA */
    if (D8(rec + 0x13u) == 0)                                       /* 0x69014 cmp byte, 0 */
        return;
    for (i = 0; S8(rec + 0x13u) > i; i++) {                         /* 0x6901A, 0x6902A dword [rec + 0x10] sar 24 */
        v = f690AC(S8(rec + 0x14u + (uint32_t)i), n);               /* 0x6903E dword [rec + i + 0x11] sar 24; 0x69044 */
        if (v < S32(CALVAL)) {                                      /* 0x69055 jge */
            setD32(CALID, (uint32_t)(int32_t)S8(rec + 0x14u + (uint32_t)i));   /* 0x69065 */
            setD32(CALVAL, (uint32_t)v);                            /* 0x6906D */
        }
    }
}

/* 0x6937C: the stated card(s). truth points to the caller's flag; count cards; k the candidate (counter phase);
   flag (stack argument, ret 4) 1 = the actual third card is a calamity or missing */
void f6937C(int n, int32_t *truth, int count, int k, int flag)
{
    uint32_t rec = NAT(n);                                          /* [ebp-0xC] */
    int32_t one = 1;                                                /* [ebp-0x10] */
    int32_t top, j, i, q, w, m, v, third;
    int32_t c = 0;                                                  /* [ebp-0x3C]: BUG(1995) item 1, both modes */
    int32_t a, r, b, found = 0, e, f;
    int counter;

    if (*truth != 0) {                                              /* 0x693B5, 0x693BA */
        if (D8(PHASE) == 0x0C) {                                    /* 0x693C4 */
            setD8(rec + (uint32_t)OFF(0xCA9ADu) + 0x9Cu, D8(rec + (uint32_t)OFF(0xCA9AFu) + 0x9Cu));   /* 0x693E8, 0x693EE */
            return;
        }
        v = S8(ROW(S16(CROW)) + (uint32_t)OFF(0xCA9B0u));           /* 0x693F9..0x69414 */
        if (v == 1 || v == 2) {                                     /* 0x6941D, 0x69422 */
            *truth = 0;                                             /* 0x6942B */
        } else {
            setD8(ROW(S16(CROW)) + (uint32_t)OFF(0xCA9ADu), D8(ROW(S16(CROW)) + (uint32_t)OFF(0xCA9B0u)));   /* 0x69455, 0x6945B */
            return;
        }
    }
    top = 8;                                                        /* 0x6946A [ebp-0x38] */
    for (j = 0; j < 9; j++) {                                       /* 0x69471, 0x6947E */
        if (S8(HELD_BY + (uint32_t)j) == 0) {                       /* 0x69487 dword [j + 0x11059D] sar 24 */
            top = j - 1;                                            /* 0x69498 */
            break;
        }
    }
    if (top > S32(TOPSTACK + 4u * (uint32_t)n))                     /* 0x694A1 jle */
        top = S32(TOPSTACK + 4u * (uint32_t)n);                     /* 0x694B4 */
    top = top + top + 2;                                            /* 0x694BA..0x694BF */
    for (i = 0; i < count; i++) {                                   /* 0x694C4, 0x696E3 jge */
        a = 0;                                                      /* 0x696F1 [ebp-0x34] */
        if (D8(PHASE) != 0x0C && S32(0x110408u + 28u * (uint32_t)k) >= 0)   /* 0x696FA, 0x6970B */
            a = 1;
        r = roll100() >= 66 ? 1 : 0;                                /* 0x6971C game_rand; 0x69733 setge [ebp-0x28] */
        b = D8(PHASE) != 0x0C ? 1 : 0;                              /* 0x69741 setne [ebp-0x20] */
        q = top;                                                    /* 0x69756 [ebp-0x4C] */
        while (q >= 1) {                                            /* 0x697C1 jl 0x696DD */
            c = q;                                                  /* 0x697CE */
            counter = D8(PHASE) != 0x0C;
            if (!counter) {                                         /* 0x697D1 jne 0x69504 */
                for (w = 0; w < S32(WANTS + 4u * (uint32_t)n); w++) {   /* 0x697E0, 0x694D2 */
                    if (S8(rec + (uint32_t)(OFF(0xCA9AEu) + w) + 0x9Cu) == c) {   /* 0x694EF dword [.. + 0x99] sar 24 */
                        c = 0;                                      /* 0x694FF xor eax, edi */
                        break;
                    }
                }
            }
            if (c != 0) {                                           /* 0x69504 je 0x6975E */
                if (D8(PHASE) != 0x0C && a != 0) {                  /* 0x6950E, 0x69519 */
                    found = 0;                                      /* 0x69521 [ebp-0x2C] */
                    m = S32(0x110400u + 28u * (uint32_t)k);         /* 0x69530 */
                    for (w = 0; w < S32(WANTS + 4u * (uint32_t)m); w++) {   /* 0x6953A, 0x69545 */
                        if (S8(NAT(m) + (uint32_t)(OFF(0xCA9AEu) + w) + 0x9Cu) == c) {   /* 0x69566 [+0xF1565] sar 24 */
                            found = 1;                              /* 0x69577 */
                            break;
                        }
                    }
                }
                if (D8(PHASE) != 0x0C && a != 0 && found == 0)      /* 0x6957E..0x69592 */
                    c = 0;                                          /* 0x69594 */
                e = 0;                                              /* 0x69599 [ebp-0x24] */
                if (r != 0 && c != 0) {                             /* 0x6959C, 0x695A6 */
                    for (w = 0; w < 2; w++) {                       /* 0x695AC, 0x695B7 */
                        if (D8(PHASE) == 0x0C)                      /* 0x695BD */
                            v = S8(NAT(n) + (uint32_t)(OFF(0xCA9ACu) + w) + 0x9Cu);   /* 0x695DA */
                        else
                            v = S8(ROW(S16(CROW)) + (uint32_t)w);  /* 0x695F9 dword [16r + w + 0x111115] sar 24 */
                        if (v == c) {                               /* 0x69607 */
                            e = 1;                                  /* 0x69609 */
                            break;
                        }
                    }
                }
                if (e != 0)                                         /* 0x69610 */
                    c = 0;
                f = 0;                                              /* 0x6961D [ebp-0x1C] */
                if (b != 0 && c != 0) {                             /* 0x69620, 0x69626 */
                    m = S32(0x110400u + 28u * (uint32_t)k);         /* 0x69638 */
                    for (w = 0; w < 3; w++) {                       /* 0x69642, 0x6964D */
                        if (S8(NAT(m) + (uint32_t)(OFF(0xCA9ACu) + w) + 0x9Cu) == c) {   /* 0x69665 */
                            f = 1;                                  /* 0x69676 */
                            break;
                        }
                    }
                }
                if (f != 0)                                         /* 0x6967D */
                    c = 0;
                if (c != 0) {                                       /* 0x69688 */
                    if (D8(PHASE) == 0x0C)                          /* 0x69692 */
                        setD8(rec + (uint32_t)OFF(0xCA9ADu) + 0x9Cu, (uint8_t)c);   /* 0x696AB */
                    else
                        setD8(ROW(S16(CROW)) + (uint32_t)(i + OFF(0xCA9ADu)), (uint8_t)c);   /* 0x696D0 */
                    top = c - 1;                                    /* 0x696DA */
                    break;                                          /* 0x696DD: the next card */
                }
            }
            /* 0x6975E: rejected; the fallbacks restart the scan without one restriction each */
            if (r != 0 && q == 1 && c == 0) {                       /* 0x6975E..0x69771 */
                q = top + q;                                        /* 0x69776 */
                r = 0;                                              /* 0x6977B */
            }
            if (b != 0 && q == 1 && c == 0) {                       /* 0x6977E..0x69791 */
                q = top + q;                                        /* 0x69796 */
                b = 0;
            }
            if (a != 0 && q == 1 && c == 0) {                       /* 0x6979E..0x697AF */
                q = top + 1;                                        /* 0x697B4 */
                a = 0;
            }
            q--;                                                    /* 0x697BE */
        }
    }
    if (c == 0)                                                     /* 0x697EA */
        return;
    if (flag != 0)                                                  /* 0x697F4 [ebp+8] */
        return;
    if (D8(PHASE) == 0x0C)                                          /* 0x697FE */
        third = S8(rec + (uint32_t)OFF(0xCA9AFu) + 0x9Cu);          /* 0x69814 */
    else if (S32(0x110408u + 28u * (uint32_t)k) > 0)                /* 0x6982E jle */
        third = S8(ROW(S16(CROW)) + (uint32_t)OFF(0xCA9B0u));      /* 0x6984D */
    else
        third = -1;                                                 /* 0x6985C */
    if ((c - 1) / 2 >= (third - 1) / 2)                             /* 0x69863..0x6987F jge */
        return;
    f6937C(n, &one, 1, k, 0);                                       /* 0x69883..0x69893 */
}

/* 0x68C90: n's declared offer */
void f68C90(int n)
{
    uint32_t rec = NAT(n);                                          /* [ebp-0x14] */
    int32_t truth, m3, j, e, cc, flag, k, t, x[3];

    if (S8(rec + 0xBCu) == 1)                                       /* 0x68CBB dword [rec + 0xB9] sar 24 */
        truth = roll100() >= 66 ? 1 : 0;                            /* 0x68CC9, 0x68CE3 setge */
    else
        truth = roll100() < 66 ? 1 : 0;                             /* 0x68CE8, 0x68D02 setl */
    setD32(GENEROUS + 4u * (uint32_t)n, 0);                         /* 0x68D18 */
    if (S32(CALVAL) < 0) {                                          /* 0x68D1F jl 0x68D93 */
        m3 = 2;                                                     /* 0x68D93 */
        truth = 0;                                                  /* 0x68D9A */
    } else {
        m3 = 3;                                                     /* 0x68D23 */
        if (S16(SESSION) == (int32_t)(D8(0xE1623u) & 0x0Fu)) {      /* 0x68D2A..0x68D3F: the last session */
            int32_t r2 = roll100();                                 /* 0x68D41 game_rand */
            if (r2 < 80 - 20 * ai_level())                          /* 0x68D54..0x68D81 */
                setD32(GENEROUS + 4u * (uint32_t)n, 1);             /* 0x68D86 */
        }
    }
    j = 0;                                                          /* 0x68D9F [ebp-0x30] */
    for (e = S32(LIST_FIRST); e < S32(LIST_CNT) && m3 > 0; e++) {   /* 0x68DA2, 0x68DB2, 0x68DC1 */
        cc = S32(ENT(e) + 8u);                                      /* 0x68DD1 [ebp-0x20] */
        while (cc > 0 && m3 > 0) {                                  /* 0x68DE9, 0x68DEF */
            setD8(rec + (uint32_t)j + 0x9Cu, D8(ENT(e)));           /* 0x68E04, 0x68E0A */
            j++;                                                    /* 0x68DFE */
            cc--;                                                   /* 0x68E13 */
            if (j == 2) {                                           /* 0x68E19 */
                j = OFF(0xCA9AFu);                                  /* 0x68E1E dword [0xCA9AC] sar 24 */
                if (truth == 0 && S32(CALVAL) >= 0) {               /* 0x68E2E, 0x68E30 */
                    setD8(rec + (uint32_t)j + 0x9Cu, D8(0x11019Cu + 16u * (uint32_t)S32(LIST_CNT)));   /* 0x68E47, 0x68E4D */
                    m3--;                                           /* 0x68E56 */
                }
            }
            m3--;                                                   /* 0x68E5C */
        }
    }
    flag = 0;                                                       /* 0x68E63 [ebp-0x24] */
    if (S8(rec + (uint32_t)OFF(0xCA9A3u) + 0x9Cu) == 0) {           /* 0x68E66..0x68E7D */
        flag = 1;                                                   /* 0x68E7F */
        setD8(rec + (uint32_t)OFF(0xCA9A3u) + 0x9Cu, D8(CALID));   /* 0x68E89, 0x68E8F */
    }
    f6937C(n, &truth, 1, 0, flag);                                  /* 0x68E95..0x68EA6 */
    if (S32(WANTS + 4u * (uint32_t)n) > 1)                          /* 0x68EAE */
        f76F70(S32(WANTS + 4u * (uint32_t)n), DP(rec + 0x9Cu + (uint32_t)OFF(0xCA9AEu)));   /* 0x68EB8..0x68ED5 */
    if (flag != 0 || truth == 0)                                    /* 0x68EDA, 0x68EE4 */
        return;
    x[0] = S8(rec + 0x9Cu);                                         /* 0x68EF1 [ebp-0x48] */
    x[1] = S8(rec + 0x9Du);                                         /* 0x68F00 [ebp-0x44] */
    x[2] = S8(rec + (uint32_t)OFF(0xCA9A3u) + 0x9Cu);               /* 0x68F17 [ebp-0x40] */
    for (k = 1; k >= 0; k--)                                        /* 0x68F23, 0x68F37 */
        for (j = 0; j <= k; j++)                                    /* 0x68F50 jg */
            if (x[j + 1] > x[j]) {                                  /* 0x68F5E, 0x68F66 jle */
                t = x[j + 1];
                x[j + 1] = x[j];
                x[j] = t;
            }
    setD8(rec + 0x9Cu, (uint8_t)x[0]);                              /* 0x68F94 */
    setD8(rec + 0x9Du, (uint8_t)x[1]);                              /* 0x68FA0 */
    setD8(rec + (uint32_t)OFF(0xCA9A3u) + 0x9Cu, (uint8_t)x[2]);    /* 0x68FB5 */
    setD8(rec + (uint32_t)OFF(0xCA9A1u) + 0x9Cu, (uint8_t)x[2]);    /* 0x68FCB the stated third = the real one */
}

/* 0x68730: n declares its offer */
void f68730(int n)
{
    f68FDC(n);                                                      /* 0x68747 */
    f68788(n);                                                      /* 0x6874F */
    f68B44(n);                                                      /* 0x68757 */
    if (S32(WANTS + 4u * (uint32_t)n) != 0)                         /* 0x6875F */
        f68C90(n);                                                  /* 0x6876C */
    else
        setD8(NAT(n) + 0x9Cu, 0);                                   /* 0x6877C */
}

/* ---- the [10][9] tables of 0x6AD70 / 0x6AF34 ---------------------------------------------------------------------- */

/* The frame from the table to the locals: int32 words, index 9m + s = table[m][s] (rows 0 and nations not in play
   unwritten: 0, file comment item 3), then held[] from word 90, then the locals q, i, c, n, the record pointer. */
#define TW_TABLE 90

static int32_t tw_read(const int32_t *w, int nheld, int32_t idx, int32_t q, int32_t i, int32_t c, int32_t n)
{
    int32_t past = idx - (TW_TABLE + nheld);
    if (idx < 0)
        return 0;                                                   /* port guard (negative cards: unreachable) */
    if (past < 0)
        return w[idx];
    switch (past) {                                                 /* [ebp-0x20], -0x1C, -0x18, -0x14, -0x10 */
    case 0: return q;
    case 1: return i;
    case 2: return c;
    case 3: return n;
    case 4: return (int32_t)NAT(n);
    default: return 0;                                              /* port guard: saved registers (unreachable) */
    }
}

/* 0x6AD70: impossible cards out of the copied offers */
void f6AD70(int n)
{
    uint32_t rec = NAT(n);                                          /* [ebp-0x10] */
    int32_t w[TW_TABLE + 29];                                       /* [ebp-0x1FC] table, [ebp-0x94] held[29] */
    int32_t m, s, c, i, q;

    memset(w, 0, sizeof w);                                         /* BUG(1995) item 3: unwritten rows read 0 */
    for (m = 1; m <= 9; m++) {                                      /* 0x6AD9A, 0x6ADA9 jg */
        if (D8(NAT(m) + 0x9Au) == 0)                                /* 0x6ADB6 */
            continue;
        f68A64(m, S8(rec + 0xBBu));                                 /* 0x6ADC7 dword [rec + 0xB8] sar 24; 0x6ADD3 */
        for (s = 0; s < 9; s++)                                     /* 0x6ADDA, 0x6ADE5 */
            w[9 * m + s] = S8(HELD_BY + (uint32_t)s);              /* 0x6ADEE dword [s + 0x11059D] sar 24; 0x6AE0E */
    }
    memset(w + TW_TABLE, 0, 29 * sizeof(int32_t));                  /* 0x6AE29 memset([ebp-0x94], 0, 0x74) */
    for (c = 1; c <= 18; c++)                                       /* 0x6AE2E, 0x6AE39 jg */
        if (S8(rec + (uint32_t)c) == S32(0xC8D58u + 4u * (uint32_t)c))   /* 0x6AE45, 0x6AE4B */
            w[TW_TABLE + c] = 1;                                    /* 0x6AE59 */
    for (i = 0; i < S32(NOFFERS); i++) {                            /* 0x6AE68, 0x6AE76 jge */
        for (q = 0; q < 5; q++) {                                   /* 0x6AE84, 0x6AE8F */
            c = S8(OCOPY(i) + (uint32_t)q);                         /* 0x6AEA9 dword [21i + q + 0x110605] sar 24 */
            if (c == 0)
                continue;
            m = S8(OCOPY(i) + 0x12u);                               /* 0x6AEC8 dword [21i + 0x110617] sar 24 */
            if (tw_read(w, 29, 9 * m + (c - 1) / 2, q, i, c, n) != 0 &&   /* 0x6AEF0 [36m + 4s + ebp-0x1FC] */
                tw_read(w, 29, TW_TABLE + c, q, i, c, n) == 0)     /* 0x6AEFE [4c + ebp-0x94] */
                continue;
            setD8(OCOPY(i) + (uint32_t)q, 0);                       /* 0x6AF1E */
        }
    }
}

/* 0x6AF34: the same test on n's response list 0x111498, clearing in 0x110608 (kept, file comment) */
void f6AF34(int n)
{
    uint32_t rec = NAT(n);                                          /* [ebp-0x10] */
    int32_t w[TW_TABLE + 19];                                       /* [ebp-0x1D4] table, [ebp-0x6C] held[19] */
    int32_t m, s, c, i, q;

    memset(w, 0, sizeof w);                                         /* BUG(1995) item 3: unwritten rows read 0 */
    for (m = 1; m <= 9; m++) {                                      /* 0x6AF5E, 0x6AF6D jg */
        if (D8(NAT(m) + 0x9Au) == 0)                                /* 0x6AF7A */
            continue;
        f68A64(m, S8(rec + 0xBBu));                                 /* 0x6AF8B; 0x6AF97 */
        for (s = 0; s < 9; s++)                                     /* 0x6AF9E, 0x6AFA9 */
            w[9 * m + s] = S8(HELD_BY + (uint32_t)s);              /* 0x6AFD2 [36m + 4s + ebp-0x1D4] */
    }
    memset(w + TW_TABLE, 0, 19 * sizeof(int32_t));                  /* 0x6AFEA memset([ebp-0x6C], 0, 0x4C) */
    for (c = 1; c <= 18; c++)                                       /* 0x6AFEF, 0x6AFFA jg */
        if (S8(rec + (uint32_t)c) == S32(0xC8D58u + 4u * (uint32_t)c))   /* 0x6B006, 0x6B00C */
            w[TW_TABLE + c] = 1;                                    /* 0x6B01A */
    for (i = 0; i < S16(NRESP); i++) {                              /* 0x6B026, 0x6B031..0x6B03C jle */
        for (q = 0; q < 5; q++) {                                   /* 0x6B044, 0x6B04F */
            c = S8(RESP(i) + (uint32_t)q);                          /* 0x6B067 dword [20i + q + 0x111495] sar 24 */
            if (c == 0)
                continue;
            m = S8(RESP(i) + 0xEu);                                 /* 0x6B086 dword [20i + 0x1114A3] sar 24 */
            if (tw_read(w, 19, 9 * m + (c - 1) / 2, q, i, c, n) != 0 &&   /* 0x6B0AE */
                tw_read(w, 19, TW_TABLE + c, q, i, c, n) == 0)     /* 0x6B0BC [4c + ebp-0x6C] */
                continue;
            setD8(OCOPY(i) + (uint32_t)q, 0);                       /* 0x6B0D9 [21i + q + 0x110608]: the other table */
        }
    }
}

/* ---- 0x6A5FC ------------------------------------------------------------------------------------------------------ */

/* the packed 5-byte records [ebp-0x70] {u8 card; int32 score} */
static int32_t pk_score(const uint8_t *p, int q)
{
    const uint8_t *s = p + 5 * q + 1;
    return (int32_t)((uint32_t)s[0] | (uint32_t)s[1] << 8 | (uint32_t)s[2] << 16 | (uint32_t)s[3] << 24);
}

static void pk_set(uint8_t *p, int q, int32_t v)
{
    uint8_t *s = p + 5 * q + 1;
    s[0] = (uint8_t)v; s[1] = (uint8_t)(v >> 8); s[2] = (uint8_t)(v >> 16); s[3] = (uint8_t)(v >> 24);
}

/* one placement of pass 1 / pass 2 (0x6A846..0x6A8D2 and 0x6A9DD..0x6AA69 are the same code) */
static void place(int32_t id, int32_t e, int32_t *j, int32_t *low, int32_t *cc, int32_t truth, int32_t *m3)
{
    uint32_t row = ROW(S16(CROW));
    setD8(row + (uint32_t)*j, (uint8_t)id);                         /* 0x6A85D */
    (*j)++;                                                         /* 0x6A855 */
    if (S32(LIST_CNT) - 1 == e)                                     /* 0x6A863..0x6A86E */
        (*low)--;                                                   /* 0x6A873 */
    (*cc)--;                                                        /* 0x6A879 */
    if (*j == 2) {                                                  /* 0x6A87C */
        *j = OFF(0xCA9B0u);                                         /* 0x6A882 dword [0xCA9AD] sar 24 */
        if (truth == 0 && S32(CALVAL) >= 0 && *low != 0) {          /* 0x6A890, 0x6A894, 0x6A8A0 */
            setD8(ROW(S16(CROW)) + (uint32_t)*j, D8(0x11019Cu + 16u * (uint32_t)S32(LIST_CNT)));   /* 0x6A8BD, 0x6A8C3 */
            (*m3)--;                                                /* 0x6A8CC */
        }
    }
    (*m3)--;                                                        /* 0x6A8D2 */
}

static int wanted_by(int32_t m, int32_t id)   /* id among m's five wanted cards (0xCA9A2 / 0xCA9AE = 5) */
{
    int32_t w;
    for (w = 0; w < 5; w++)                                         /* 0x6A7DB, 0x6A7E6 */
        if (S8(NAT(m) + (uint32_t)(OFF(0xCA9A2u) + w) + 0x9Cu) == id)   /* 0x6A7FE [199m + .. + 0xF1565] sar 24 */
            return 1;
    return 0;
}

/* 0x6A5FC: one counter-offer row by n for candidate k; *out = 1 when the calamity goes in */
void f6A5FC(int n, int k, int32_t *out, int c)
{
    uint32_t recn = NAT(n);                                         /* [ebp-0x10] */
    int32_t m = S32(0x110400u + 28u * (uint32_t)k);                 /* 0x6A642 [ebp-0x1C] */
    uint32_t recm = NAT(m);                                         /* [ebp-0xC] */
    int32_t m3, matches, e, q, truth, nt, j, any, low, id, cc, cnt, v, w, o, kk;
    uint8_t pk[15], tmp[5];

    memset(DP(ROW(S16(CROW))), 0, 8);                               /* 0x6A61A..0x6A631 */
    m3 = 3;                                                         /* 0x6A66E [ebp-0x48] */
    *out = 0;                                                       /* 0x6A678 */
    if (S32(CALVAL) < -1 || c != 0) {                               /* 0x6A67E, 0x6A687 */
        m3--;                                                       /* 0x6A690 */
        *out = 1;                                                   /* 0x6A696 */
    }
    matches = 0;                                                    /* 0x6A69E [ebp-0x44] */
    for (e = S32(LIST_CNT) - 1; e >= 0; e--)                        /* 0x6A6A1..0x6A6AE, 0x6A6B7 */
        for (q = 0; q < 5; q++)                                     /* 0x6A6B2, 0x6A6D0 */
            if (S8(recm + (uint32_t)(OFF(0xCA9AEu) + q) + 0x9Cu) == S32(ENT(e)))   /* 0x6A6E4, 0x6A6F3 */
                matches++;                                          /* 0x6A700 */
    if (S8(recn + 0xBCu) == 1)                                      /* 0x6A708 */
        truth = roll100() >= 75 ? 1 : 0;                            /* 0x6A716, 0x6A730 setge */
    else
        truth = roll100() < 75 ? 1 : 0;                             /* 0x6A735, 0x6A74F setl */
    if (*out != 0)                                                  /* 0x6A75D */
        truth = 0;
    nt = truth == 0 ? 1 : 0;                                        /* 0x6A767..0x6A772 [ebp-0x18] */
    if (matches + nt == 0)                                          /* 0x6A779, 0x6A77F je 0x6AD68 */
        return;
    j = 0;                                                          /* 0x6A787 [ebp-0x24] */
    any = 0;                                                        /* 0x6A78A [ebp-0x38] */
    low = S32(0x1101A4u + 16u * (uint32_t)S32(LIST_CNT));          /* 0x6A795 the last entry's count [ebp-0x20] */
    for (e = S32(LIST_FIRST); e < S32(LIST_CNT) && m3 > 0; e++) {   /* 0x6A79E, 0x6A7AE, 0x6A7BD: pass 1 */
        id = S32(ENT(e));                                           /* 0x6A7CD [ebp-0x14] */
        if (!wanted_by(m, id))                                      /* 0x6A816 */
            continue;
        cc = S32(ENT(e) + 8u);                                      /* 0x6A822 [ebp-0x2C] */
        any = 1;                                                    /* 0x6A82B */
        while (cc > 0 && m3 > 0)                                    /* 0x6A832, 0x6A83C */
            place(id, e, &j, &low, &cc, truth, &m3);
    }
    if (m3 > 0) {                                                   /* 0x6A8DA */
        if (any == 0)                                               /* 0x6A8E7 */
            truth = 0;                                              /* 0x6A8EB */
        for (e = S32(LIST_FIRST); e < S32(LIST_CNT) && m3 > 0; e++) {   /* 0x6A8EE, 0x6A8FE, 0x6A90D: pass 2 */
            id = S32(ENT(e));                                       /* 0x6A91D */
            w = wanted_by(m, id);                                   /* 0x6A926..0x6A95F [ebp-0x34] */
            o = 0;                                                  /* 0x6A968 [ebp-0x30] */
            for (q = 0; q < 3; q++)                                 /* 0x6A96B, 0x6A976 */
                if (S8(recm + (uint32_t)(OFF(0xCA9A0u) + q) + 0x9Cu) == id) {   /* 0x6A98E dword [0xCA99D] sar 24 */
                    o = 1;                                          /* 0x6A99F */
                    break;
                }
            if (w != 0 || o != 0)                                   /* 0x6A9A6, 0x6A9B0 */
                continue;
            cc = S32(ENT(e) + 8u);                                  /* 0x6A9C0 */
            while (cc > 0 && m3 > 0)                                /* 0x6A9C9, 0x6A9D3 */
                place(id, e, &j, &low, &cc, truth, &m3);
        }
    }
    if (m3 > 0) {                                                   /* 0x6AA71 */
        setD8(ROW(S16(CROW)), 0);                                   /* 0x6AA84 */
        return;
    }
    if (*out != 0) {                                                /* 0x6AA92 */
        setD8(ROW(S16(CROW)) + (uint32_t)j, D8(CALID));             /* 0x6AAAB, 0x6AAB0 */
        j++;                                                        /* 0x6AAA6 */
        if (j == 2)                                                 /* 0x6AAB9 */
            j = OFF(0xCA9B0u);                                      /* 0x6AABE (not read again) */
    }
    f6937C(n, &truth, S32(0x1103F8u + 28u * (uint32_t)k) - 2, k, *out);   /* 0x6AAC9..0x6AAF0 */
    cnt = 0;                                                        /* 0x6AAF5 [ebp-0x44] */
    for (e = 0; e < 5; e++) {                                       /* 0x6AAF8, 0x6AB03 */
        v = S8(ROW(S16(CROW)) + (uint32_t)e);                       /* 0x6AB17 */
        if (v == 0)
            continue;
        for (q = 0; q < 5; q++)                                     /* 0x6AB2C, 0x6AB37 */
            if (S8(recm + (uint32_t)(OFF(0xCA9AEu) + q) + 0x9Cu) == v)   /* 0x6AB4B, 0x6AB57 */
                cnt++;                                              /* 0x6AB5E */
    }
    if (cnt == 0) {                                                 /* 0x6AB63 */
        setD8(ROW(S16(CROW)), 0);                                   /* 0x6AB76 */
        return;
    }
    if (*out != 0 || truth == 0)                                    /* 0x6AB86, 0x6AB8F */
        return;
    if (S8(ROW(S16(CROW))) == 0)                                    /* 0x6ABA5 */
        return;
    pk_set(pk, 0, S8(ROW(S16(CROW))));                              /* 0x6ABBF [ebp-0x6F] */
    pk[0] = (uint8_t)pk_score(pk, 0);                               /* 0x6ABC5 [ebp-0x70] */
    pk_set(pk, 1, S8(ROW(S16(CROW)) + 1u));                         /* 0x6ABD1 [ebp-0x6A] */
    pk[5] = (uint8_t)pk_score(pk, 1);                               /* 0x6ABD7 [ebp-0x6B] */
    pk_set(pk, 2, S8(ROW(S16(CROW)) + (uint32_t)OFF(0xCA9B0u)));    /* 0x6ABEC [ebp-0x65] */
    pk[10] = (uint8_t)pk_score(pk, 2);                              /* 0x6ABF2 [ebp-0x66] */
    for (q = 0; q < 3; q++) {                                       /* 0x6ABF7, 0x6AC02 */
        id = (int8_t)pk[5 * q];                                     /* 0x6AC14 dword [5q + ebp-0x73] sar 24 */
        if (id == 0)
            continue;
        for (w = 0; w < 5; w++)                                     /* 0x6AC27, 0x6AC32 */
            if (S8(recm + (uint32_t)(OFF(0xCA9AEu) + w) + 0x9Cu) == id)   /* 0x6AC46, 0x6AC52 */
                pk_set(pk, q, mul(pk_score(pk, q), 2));             /* 0x6AC62..0x6AC68 add ebx, ebx */
    }
    for (kk = 1; kk >= 0; kk--)                                     /* 0x6AC6E, 0x6AC86 */
        for (q = 0; q <= kk; q++)                                   /* 0x6ACA3 jg */
            if (pk_score(pk, q + 1) > pk_score(pk, q)) {            /* 0x6ACC4, 0x6ACC8, 0x6ACCE jle */
                memcpy(tmp, pk + 5 * (q + 1), 5);                   /* 0x6ACE4 */
                memcpy(pk + 5 * (q + 1), pk + 5 * q, 5);            /* 0x6AD07 */
                memcpy(pk + 5 * q, tmp, 5);                         /* 0x6AD1C */
            }
    setD8(ROW(S16(CROW)), pk[0]);                                   /* 0x6AD32 */
    setD8(ROW(S16(CROW)) + 1u, pk[5]);                              /* 0x6AD3B */
    setD8(ROW(S16(CROW)) + (uint32_t)OFF(0xCA9B0u), pk[10]);        /* 0x6AD4F */
    setD8(ROW(S16(CROW)) + (uint32_t)OFF(0xCA9ADu), pk[10]);        /* 0x6AD62 */
}

/* ---- 0x698A4 ------------------------------------------------------------------------------------------------------ */

/* f698A4's frame from [ebp-0x5EC] (the row buffer, 56 rows) through the pair list [ebp-0x26C] and the swap temp
   [ebp-0x90] to [ebp-0x3D]: 90 rows fit before the counts [ebp-0x3C] / [ebp-0x38] (item 4) */
#define FR_SIZE    0x5B0
#define FR_LIST    0x380   /* [ebp-0x26C] */
#define FR_PAIRS   59      /* the pairs before the temp [ebp-0x90] (port guard) */
#define ROWS_MAX   ((DS_BASE + DS_SIZE - 0x111118u) / 16u)

static uint8_t s_frame[FR_SIZE];
static uint8_t s_rows[ROWS_MAX * 16u];

static int32_t fr32(const uint8_t *p)
{
    return (int32_t)((uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24);
}

static void fr32set(uint8_t *p, int32_t v)
{
    p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24);
}

/* 0x698A4: n's counter offers */
void f698A4(int n)
{
    uint32_t rec = NAT(n);                                          /* [ebp-0x28] */
    int32_t h = 0;                                                  /* 0x698C1 [ebp-0x68] */
    int32_t added = 0;                                              /* 0x698C4 [ebp-0x30] */
    int32_t out = 0;                                                /* [ebp-0x34]: stale word 2 (noted, no effect) */
    int32_t kw, sum, j, i, q, w, c, m, matches, k, ex, diff, trust, total, avg, thr, x, v, low, s, g, a, b, idx;
    int32_t cnt, rows, r, t;
    uint8_t *buf, tmp[8];
    uint32_t sz;

    if (S32(WANTS + 4u * (uint32_t)n) == 0)                         /* 0x698DB */
        return;
    f68FDC(n);                                                      /* 0x698EC */
    f68788(n);                                                      /* 0x698F4 */
    kw = S32(0xC8DA0u + 4u * (uint32_t)S32(LIST_CNT));              /* 0x69906 [ebp-0x20] */
    if (S32(LIST_CNT) < 8) {                                        /* 0x69909 */
        sum = 0;                                                    /* 0x69910 [ebp-0x18] */
        for (j = kw; j < S32(LIST_CNT); j++)                        /* 0x69916, 0x6992A */
            sum += S32(ENT(j) + 8u);                                /* 0x69941 */
        if (S32(CALVAL) < 0)                                        /* 0x6994C */
            sum++;
        if (sum < 3)                                                /* 0x6995B */
            kw--;
    }
    if (S32(CALVAL) < -1 || f7CAB4(n) != 0) {                       /* 0x69967, 0x69973 */
        if (kw > 3)
            kw = 3;
    } else if (S16(0x112C9Au + 2u * (uint32_t)n) / 300 < 25) {      /* 0x69987..0x699A0 */
        if (kw > 3)
            kw = 3;                                                 /* 0x699AA */
    }
    if (kw != 0) {                                                  /* 0x699B1 */
        j = 0;                                                      /* 0x699B9 */
        while (j < kw)                                              /* 0x699BC */
            j++;
        setD32(LIST_FIRST, (uint32_t)j);                            /* 0x699CF */
    }
    for (i = 0; i < S32(NOFFERS); i++)                              /* 0x699D6, 0x699EA */
        memmove(DP(OCOPY(i)), DP(0x110ECCu + 21u * (uint32_t)i), 21);   /* 0x69A28..0x69A2D */
    f6AD70(n);                                                      /* 0x69A35 */
    setD32(CAND_CNT, 0);                                            /* 0x69A3A */
    total = 0;                                                      /* 0x69A40 [ebp-0x6C] */
    for (i = 0; i < S32(NOFFERS); i++) {                            /* 0x69A43, 0x69DB1 */
        m = S8(OCOPY(i) + 0x12u);                                   /* 0x69DCE dword [21i + 0x110617] sar 24 */
        if (m == n)                                                 /* 0x69DDB */
            continue;
        matches = 0;                                                /* 0x69DE1 [ebp-0x64] */
        for (q = 0; q < 3; q++) {                                   /* 0x69DE4, 0x69A99 */
            c = S8(OCOPY(i) + (uint32_t)q);                         /* 0x69AB6 [ebp-0x60] */
            if (c == 0)
                continue;
            for (w = 0; w < S32(WANTS + 4u * (uint32_t)n); w++)     /* 0x69ACB, 0x69A5A */
                if (S8(rec + (uint32_t)(OFF(0xCA9AEu) + w) + 0x9Cu) == c) {   /* 0x69A7D, 0x69A89 */
                    matches++;                                      /* 0x69A90 */
                    break;
                }
        }
        k = S32(CAND_CNT);                                          /* 0x69AD3 */
        setD32(CAND(k), (uint32_t)matches);                         /* 0x69AF1 */
        setD32(CAND(k) + 4u, 3);                                    /* 0x69AFA [ebp-0x5C] = 3 */
        copy_nation(NAT(0), rec);                                   /* 0x69B19 */
        for (q = 0; q < 3; q++)                                     /* 0x69B1E, 0x69B29 */
            inc8(NAT(0) + (uint32_t)(int32_t)S8(OCOPY(i) + (uint32_t)q));   /* 0x69B4F */
        a = f6DD98(0);                                              /* 0x69B59 */
        b = f6DD98(n);                                              /* 0x69B63 */
        setD32(CAND(S32(CAND_CNT)) + 8u, (uint32_t)(a - b));        /* 0x69B6A..0x69B7B */
        ex = 0;                                                     /* 0x69B84 [ebp-0x7C] */
        if ((D8(0xE1625u) & 0x20u) == 0) {                          /* 0x69B87 */
            for (w = 0; w < 3; w++) {                               /* 0x69B94, 0x69BFF */
                copy_nation(NAT(0), rec);                           /* 0x69C21 */
                for (q = 0; q < 3; q++)                             /* 0x69C26, 0x69BA2 */
                    if (q != w)                                     /* 0x69BB3 */
                        inc8(NAT(0) + (uint32_t)(int32_t)S8(OCOPY(i) + (uint32_t)q));   /* 0x69BD5 */
                a = f195D8(0);                                      /* 0x69BDF */
                b = f195D8(n);                                      /* 0x69BE9 */
                ex += a - b;                                        /* 0x69BF0 */
            }
            ex = ex / 3;                                            /* 0x69C3A idiv [ebp-0x5C] */
        } else {
            w = 2;                                                  /* 0x69C44 */
            copy_nation(NAT(0), rec);                               /* 0x69C65 */
            for (q = 0; q < 3; q++)                                 /* 0x69C6A, 0x69C75 */
                if (q != w)
                    inc8(NAT(0) + (uint32_t)(int32_t)S8(OCOPY(i) + (uint32_t)q));   /* 0x69CA8 */
            a = f195D8(0);                                          /* 0x69CB2 */
            b = f195D8(n);                                          /* 0x69CBC */
            ex = a - b;                                             /* 0x69CC3 */
        }
        diff = S32(CAND(S32(CAND_CNT)) + 8u) - ex;                  /* 0x69CC6..0x69CE1 [ebp-0x78] */
        if (diff != 0) {                                            /* 0x69CE4 */
            trust = S8(rec + 0xB1u + (uint32_t)(int32_t)S8(OCOPY(i) + 0x12u));   /* 0x69CF9..0x69D0C */
            if (trust < 0)
                trust = 0;                                          /* 0x69D19 */
            if (trust > 100)
                trust = 100;                                        /* 0x69D24 */
            setD32(CAND(S32(CAND_CNT)) + 0x18u, (uint32_t)(ex + mul(diff, trust) / 100));   /* 0x69D2B..0x69D52 */
        }
        k = S32(CAND_CNT);                                          /* 0x69D59 */
        total += S32(CAND(k) + 0x18u);                              /* 0x69D71 */
        setD32(CAND(k) + 0xCu, (uint32_t)(int32_t)S8(OCOPY(i) + 0x12u));   /* 0x69D8D */
        setD32(CAND(k) + 0x10u, (uint32_t)i);                       /* 0x69D99 */
        setD32(CAND_CNT, (uint32_t)(S32(CAND_CNT) + 1));            /* 0x69D9F */
    }
    for (t = S32(CAND_CNT) - 2; t >= 0; t--)                        /* 0x69DEC, 0x69DFC */
        for (j = 0; j <= t; j++)                                    /* 0x69E22 jg */
            if (S32(CAND(j + 1) + 8u) > S32(CAND(j) + 8u))          /* 0x69E49, 0x69E4F */
                swap_bytes(CAND(j), CAND(j + 1), 28);               /* 0x69E79, 0x69EA9, 0x69EC9 */
    for (t = S32(CAND_CNT) - 2; t >= 0; t--)                        /* 0x69ED0, 0x69EE0 */
        for (j = 0; j <= t; j++)                                    /* 0x69F06 jg */
            if (S32(CAND(j + 1)) > S32(CAND(j)))                    /* 0x69F2D, 0x69F33 */
                swap_bytes(CAND(j), CAND(j + 1), 28);               /* 0x69F5D, 0x69F8D, 0x69FAD */

    for (k = 0; k < S32(CAND_CNT); k++) {                           /* 0x69FB6, 0x6A2F0 */
        m = S32(0x110400u + 28u * (uint32_t)k);                     /* 0x6A30B [ebp-0x54] */
        setD32(GAP, (uint32_t)f6B928(n, m));                        /* 0x6A31B, 0x6A320 */
        avg = total / S32(CAND_CNT);                                /* 0x6A325..0x6A334 [ebp-0x70] */
        if (avg < S32(0xC8D40u + 4u * (uint32_t)ai_level()))        /* 0x6A339..0x6A359 */
            avg = S32(0xC8D40u + 4u * (uint32_t)ai_level());        /* 0x6A35B */
        thr = avg;                                                  /* 0x6A361 [ebp-0x4C] */
        thr += mul(S32(CALVAL), avg) / 20;                          /* 0x6A364..0x6A37A */
        if (S32(CAND(k)) != 0 || S32(CALVAL) < -1)                  /* 0x6A389, 0x6A397 */
            x = 0;                                                  /* 0x69FBE */
        else
            x = mul(avg, 5) / 20;                                   /* 0x6A3A4..0x6A3BA */
        thr += x;                                                   /* 0x69FC0 */
        v = S32(GAP);                                               /* 0x69FC3 */
        if (v > 0 && S32(CALVAL) < -1)                              /* 0x69FC9, 0x69FCD */
            x = mul(v, avg) / 20;                                   /* 0x69FD6..0x69FE6 */
        else
            x = 0;
        thr -= x;                                                   /* 0x69FEC */
        thr += mul(S8(rec + 0xA8u + (uint32_t)m), avg) / 20;        /* 0x69FF5 dword [rec + m + 0xA5] sar 24; 0x6A00E */
        thr += mul(avg, mul(S8(rec + 0xBDu + (uint32_t)m), 30)) / 20;   /* 0x6A017..0x6A036 */
        if (S32(CALVAL) != 0) {                                     /* 0x6A039 */
            h = f690AC(S8(rec + (uint32_t)OFF(0xCA9A3u) + 0x9Cu), m);   /* 0x6A046..0x6A062 */
            if (h < -1) {                                           /* 0x6A06B jge */
                h = h / 2 < -5 ? h / 2 : -5;                        /* 0x6A071..0x6A097 */
                if (m == S32(LEADER))                               /* 0x6A09D */
                    thr -= mul(avg, 3) / 20;                        /* 0x6A0A5..0x6A0BD */
                if (S8(NAT(m) + 0x9Au) == 3)                        /* 0x6A0C7 dword [199m + 0xF1563] sar 24 */
                    thr += mul(h, avg) / 20;                        /* 0x6A0D5..0x6A0E8 */
            }
        }
        low = 10;                                                   /* 0x6A0EB [ebp-0x48] */
        for (j = 0; j < 5; j++) {                                   /* 0x6A0F2, 0x6A108 */
            c = S8(NAT(m) + (uint32_t)(OFF(0xCA9AEu) + j) + 0x9Cu);    /* 0x6A126 [ebp-0x80] */
            if (c == 0)
                continue;
            s = (c - 1) / 2;                                        /* 0x6A13A..0x6A147 */
            if (s < low)
                low = s;                                            /* 0x6A157 */
        }
        thr += S32(TOPSTACK + 4u * (uint32_t)n) < low ? 500 : 0;    /* 0x6A15C..0x6A174 */
        if (S32(0x11040Cu + 28u * (uint32_t)k) <= thr)              /* 0x6A183, 0x6A18D jle */
            continue;
        f6A5FC(n, k, &out, h < -1 ? 1 : 0);                         /* 0x6A193..0x6A1A9 */
        if (S8(ROW(S16(CROW))) == 0)                                /* 0x6A1AE..0x6A1C4 */
            continue;
        copy_nation(NAT(0), rec);                                   /* 0x6A1E3 */
        for (q = 0; q < 3; q++) {                                   /* 0x6A1E8, 0x6A1F3 */
            idx = S32(0x110404u + 28u * (uint32_t)k);               /* 0x6A205 [ebp-0x74] */
            inc8(NAT(0) + (uint32_t)(int32_t)S8(OCOPY(idx) + (uint32_t)q));   /* 0x6A22C */
        }
        for (q = 0; q < 3; q++) {                                   /* 0x6A236, 0x6A276 */
            idx = q < 2 ? q : q + 3;                                /* 0x6A27F..0x6A287 */
            v = S8(ROW(S16(CROW)) + (uint32_t)idx);                 /* 0x6A24F [ebp-0x80] */
            if (v >= 1 && v <= 18)                                  /* 0x6A25B, 0x6A261 */
                dec8(NAT(0) + (uint32_t)v);                         /* 0x6A26A */
        }
        a = f6DD98(0);                                              /* 0x6A28E */
        b = f6DD98(n);                                              /* 0x6A298 */
        g = a - b;                                                  /* 0x6A29D [ebp-0x50] */
        if (thr - mul(avg, 2) / 20 > g)                             /* 0x6A2A2..0x6A2BD jg */
            continue;
        setD8(ROW(S16(CROW)) + 0xDu, (uint8_t)m);                   /* 0x6A2CE */
        setD8(ROW(S16(CROW)) + 0xEu, (uint8_t)n);                   /* 0x6A2D7 */
        added++;                                                    /* 0x6A2E0 */
        setD16(CROW, (uint16_t)(D16(CROW) + 1u));                   /* 0x6A2E3 inc word */
    }

    if (out == 0 || added <= 3)                                     /* 0x6A3C1, 0x6A3CB */
        return;
    /* keep the three best rows by 0xF52E0[n][m]: the pairs {row, m} at [ebp-0x26C] (item 4 for the frame) */
    memset(s_frame, 0, sizeof s_frame);
    cnt = 0;                                                        /* 0x6A3D7 [ebp-0x38] */
    for (r = 0; r < S16(CROW); r++) {                               /* 0x6A3DA, 0x6A3EE */
        if (S8(ROW(r) + 0xEu) != n)                                 /* 0x6A407 dword [16r + 0x111123] sar 24 */
            continue;
        if (cnt >= FR_PAIRS)                                        /* port guard (file comment) */
            continue;
        fr32set(s_frame + FR_LIST + 8 * cnt, r);                    /* 0x6A420 */
        fr32set(s_frame + FR_LIST + 8 * cnt + 4, S8(ROW(r) + 0xDu));   /* 0x6A430, 0x6A43F */
        cnt++;
    }
    for (t = cnt - 2; t >= 0; t--)                                  /* 0x6A448, 0x6A456 */
        for (j = 0; j <= t; j++) {                                  /* 0x6A47C jg */
            int32_t m1 = fr32(s_frame + FR_LIST + 8 * (j + 1) + 4), m0 = fr32(s_frame + FR_LIST + 8 * j + 4);
            if (S32(0xF52E0u + 40u * (uint32_t)n + 4u * (uint32_t)m1) >   /* 0x6A48A..0x6A4C5 */
                S32(0xF52E0u + 40u * (uint32_t)n + 4u * (uint32_t)m0)) {
                memcpy(tmp, s_frame + FR_LIST + 8 * (j + 1), 8);    /* 0x6A4E0 */
                memcpy(s_frame + FR_LIST + 8 * (j + 1), s_frame + FR_LIST + 8 * j, 8);   /* 0x6A4F6 */
                memcpy(s_frame + FR_LIST + 8 * j, tmp, 8);          /* 0x6A508 */
            }
        }
    rows = S16(CROW);                                               /* 0x6A50F [ebp-0x3C] */
    sz = 16u * (uint32_t)rows;                                      /* 0x6A51A..0x6A53E */
    if (rows > 0 && (uint32_t)rows > ROWS_MAX)
        sz = 16u * ROWS_MAX;                                        /* port guard: the data object ends */
    if (g_original && rows <= 90) {
        /* BUG(1995) item 4: the copy into [ebp-0x5EC]; past 56 rows it runs over the pair list (as the original) */
        buf = s_frame;
    } else {
        /* fixed: a buffer of its own, the pair list intact */
        buf = s_rows;
    }
    if (rows > 0)
        memcpy(buf, DP(ROW(0)), sz);                                /* 0x6A537 repne movsd, 0x6A53E repne movsb */
    for (j = 3; j < cnt; j++) {                                     /* 0x6A541, 0x6A559 */
        r = fr32(s_frame + FR_LIST + 8 * j);                        /* 0x6A56A [8j + ebp-0x26C] */
        if (r >= 0 && 16u * (uint32_t)r + 0xEu < (buf == s_frame ? (uint32_t)FR_SIZE : sizeof s_rows))
            buf[16 * r + 0xE] = 0;                                  /* 0x6A576 [16r + ebp-0x5DE] (port guard on the index) */
    }
    setD16(CROW, 0);                                                /* 0x6A581 */
    for (r = 0; r < rows && 16u * (uint32_t)r < sz; r++) {          /* 0x6A58A, 0x6A59E */
        if ((int8_t)buf[16 * r + 0xE] == 0)                         /* 0x6A5B2 dword [16r + ebp-0x5E1] sar 24 */
            continue;
        t = S16(CROW);                                              /* 0x6A5C6 movsx edi, ax */
        setD16(CROW, (uint16_t)(t + 1));                            /* 0x6A5D6 */
        memcpy(DP(ROW(t)), buf + 16 * r, 16);                       /* 0x6A5E9..0x6A5EC */
    }
}

/* ---- 0x6B0EC ------------------------------------------------------------------------------------------------------ */

/* 0x6B0EC: n accepts at most one response to its offer */
void f6B0EC(int n)
{
    uint32_t rec = NAT(n);                                          /* [ebp-0x20] */
    uint8_t cp[199];                                                /* [ebp-0x13C] */
    int32_t hascal, sum = 0, i, q, w, c, matches, ex, diff, trust, avg, thr, x, v, m, h, a, b, k, j, id;
    int32_t cnt;

    hascal = S8(rec + (uint32_t)OFF(0xCA9A3u) + 0x9Cu) >= 0x13 ? 1 : 0;   /* 0x6B118..0x6B132 [ebp-0x24] */
    f68FDC(n);                                                      /* 0x6B13D */
    f6AF34(n);                                                      /* 0x6B145 */
    if (S16(NRESP) > 0) {                                           /* 0x6B14A..0x6B154 jle 0x6B54D */
        memcpy(cp, DP(rec), 199);                                   /* 0x6B172..0x6B176 */
        id = S8(rec + 0x9Cu);                                       /* 0x6B17A */
        if (id >= 0 && id < 199)                                    /* port guard (ids 0..30) */
            cp[id]--;                                               /* 0x6B183 */
        id = S8(rec + 0x9Du);                                       /* 0x6B18D */
        if (id >= 0 && id < 199)
            cp[id]--;                                               /* 0x6B196 */
        id = S8(rec + 0xA6u);                                       /* 0x6B1A0 */
        if (id >= 0 && id < 199)
            cp[id]--;                                               /* 0x6B1A9 */
        f194E4(n);                                                  /* 0x6B1B5 */
        sum = 0;                                                    /* 0x6B1BA [ebp-0x38] */
        for (i = 0; i < S16(NRESP); i++) {                          /* 0x6B1BD, 0x6B533 */
            matches = 0;                                            /* 0x6B542 [ebp-0x34] */
            for (q = 0; q < 3; q++) {                               /* 0x6B545, 0x6B204 */
                c = S8(RESP(i) + (uint32_t)q);                      /* 0x6B21C [ebp-0x30] */
                if (c == 0)
                    continue;
                for (w = 0; w < S32(WANTS + 4u * (uint32_t)n); w++)   /* 0x6B231, 0x6B1CB */
                    if (S8(rec + (uint32_t)(OFF(0xCA9A2u) + w) + 0x9Cu) == c) {   /* 0x6B1E8, 0x6B1F4 */
                        matches++;                                  /* 0x6B1FB */
                        break;
                    }
            }
            memcpy(DP(NAT(0)), cp, 199);                            /* 0x6B25C..0x6B260 */
            setD32(ACC(i) + 0x18u, (uint32_t)matches);              /* 0x6B261 */
            for (q = 0; q < 3; q++)                                 /* 0x6B269, 0x6B274 */
                inc8(NAT(0) + (uint32_t)(int32_t)S8(RESP(i) + (uint32_t)q));   /* 0x6B295 */
            a = f195D8(0);                                          /* 0x6B29F */
            b = f195D8(n);                                          /* 0x6B2AB */
            setD32(ACC(i), (uint32_t)(a - b));                      /* 0x6B2C3 */
            setD32(ACC(i) + 0x14u, 3);                              /* 0x6B2DB [ebp-0x50] = 3 */
            ex = 0;                                                 /* 0x6B2E2 [ebp-0x48] */
            if ((D8(0xE1625u) & 0x20u) == 0) {                      /* 0x6B2E5 */
                for (w = 0; w < 3; w++) {                           /* 0x6B2F2, 0x6B34C */
                    memcpy(DP(NAT(0)), cp, 199);                    /* 0x6B362 */
                    for (q = 0; q < 3; q++)                         /* 0x6B367, 0x6B2FD */
                        if (q != w)
                            inc8(NAT(0) + (uint32_t)(int32_t)S8(RESP(i) + (uint32_t)q));   /* 0x6B328 */
                    a = f195D8(0);                                  /* 0x6B332 */
                    b = f195D8(n);                                  /* 0x6B33C */
                    ex += a - b;                                    /* 0x6B343 */
                }
                ex = ex / 3;                                        /* 0x6B37A idiv [ebp-0x50] */
            } else {
                w = 2;                                              /* 0x6B384 */
                memcpy(DP(NAT(0)), cp, 199);                        /* 0x6B39B */
                for (q = 0; q < 3; q++)                             /* 0x6B3A2, 0x6B3AD */
                    if (q != w)
                        inc8(NAT(0) + (uint32_t)(int32_t)S8(RESP(i) + (uint32_t)q));   /* 0x6B3D8 */
                a = f195D8(0);                                      /* 0x6B3E2 */
                b = f195D8(n);                                      /* 0x6B3EC */
                ex = a - b;                                         /* 0x6B3F3 */
            }
            diff = S32(ACC(i)) - ex;                                /* 0x6B3F6..0x6B40E [ebp-0x44] */
            if (diff != 0) {                                        /* 0x6B411 */
                trust = S8(rec + 0xB1u + (uint32_t)(int32_t)S8(RESP(i) + 0xDu));   /* 0x6B427..0x6B43D: n itself */
                if (trust < 0)
                    trust = 0;                                      /* 0x6B449 */
                if (trust > 100)
                    trust = 100;                                    /* 0x6B452 */
                setD32(ACC(i) + 4u, (uint32_t)(ex + mul(diff, trust) / 100));   /* 0x6B459..0x6B47D */
                if (hascal != 0) {                                  /* 0x6B484 */
                    v = f6B928(n, S32(ACC(i) + 0xCu));              /* 0x6B496: read before this call writes it */
                    setD32(GAP, (uint32_t)v);                       /* 0x6B4A7 */
                    setD32(ACC(i) + 4u, (uint32_t)(S32(ACC(i) + 4u) + (v > 0 ? v / 2 : 0)));   /* 0x6B4BF..0x6B4D0 */
                }
            }
            sum += S32(ACC(i) + 4u);                                /* 0x6B4DE, 0x6B4E5 */
            setD32(ACC(i) + 8u, (uint32_t)i);                       /* 0x6B4FE */
            setD32(ACC(i) + 0xCu, (uint32_t)(int32_t)S8(RESP(i) + 0xEu));   /* 0x6B510, 0x6B526 */
        }
    }
    for (i = 0; i < S16(NRESP); i++) {                              /* 0x6B54D, 0x6B72E */
        setD32(GAP, (uint32_t)f6B928(n, S32(ACC(i) + 0xCu)));       /* 0x6B74B..0x6B760 */
        avg = sum / S16(NRESP);                                     /* 0x6B75A, 0x6B768..0x6B771 [ebp-0x18] */
        if (avg < S32(0xC8D4Cu + 4u * (uint32_t)ai_level()))        /* 0x6B776..0x6B796 */
            avg = S32(0xC8D4Cu + 4u * (uint32_t)ai_level());        /* 0x6B798 */
        thr = avg;                                                  /* 0x6B79E [ebp-0x28] */
        thr += mul(S32(CALVAL), avg) / 20;                          /* 0x6B7A1..0x6B7B7 */
        if (S32(ACC(i) + 0x18u) != 0 || S32(CALVAL) < -1)           /* 0x6B7C6, 0x6B7D4 */
            x = 0;                                                  /* 0x6B557 */
        else
            x = mul(avg, 5) / 20;                                   /* 0x6B7E1..0x6B7F2 */
        thr += x;                                                   /* 0x6B559 */
        thr += mul(avg, mul(S32(GENEROUS + 4u * (uint32_t)n), 40)) / 20;   /* 0x6B55C..0x6B584 */
        v = S32(GAP);                                               /* 0x6B587 */
        if (v > 0 && S32(CALVAL) < -1)                              /* 0x6B58D, 0x6B591 */
            x = mul(v, avg) / 20;                                   /* 0x6B59A..0x6B5AA */
        else
            x = 0;
        thr -= x;                                                   /* 0x6B5B0 */
        m = S32(ACC(i) + 0xCu);
        thr += mul(S8(rec + 0xA8u + (uint32_t)m), avg) / 20;        /* 0x6B5BF..0x6B5E2 */
        thr += mul(avg, mul(S8(rec + 0xBDu + (uint32_t)m), 30)) / 20;   /* 0x6B5F1..0x6B61A */
        if (S32(CALVAL) != 0) {                                     /* 0x6B61D */
            h = f690AC(S8(rec + (uint32_t)OFF(0xCA9A3u) + 0x9Cu), m);   /* 0x6B636..0x6B656 [ebp-0x1C] */
            if (h < -1) {                                           /* 0x6B65F jge */
                h = h / 2 < -5 ? h / 2 : -5;                        /* 0x6B665..0x6B68B */
                if (S32(LEADER) == m)                               /* 0x6B69A..0x6B6A7 */
                    thr -= mul(avg, 3) / 20;                        /* 0x6B6A9..0x6B6C1 */
                if (S8(NAT(m) + 0x9Au) == 3)                        /* 0x6B6D0..0x6B6E7 */
                    thr += mul(h, avg) / 20;                        /* 0x6B6E9..0x6B6FC */
            }
        }
        setD32(ACC(i) + 0x10u, (uint32_t)(S32(ACC(i) + 4u) - thr));   /* 0x6B70B..0x6B721 */
    }
    cnt = S16(NRESP);                                               /* 0x6B733 eax at 0x6B7F9 */
    for (k = cnt - 2; k >= 0; k--)                                  /* 0x6B7F9, 0x6B810 */
        for (j = 0; j <= k; j++)                                    /* 0x6B82D jg */
            if (S32(ACC(j + 1) + 0x10u) > S32(ACC(j) + 0x10u))      /* 0x6B851, 0x6B857 */
                swap_bytes(ACC(j), ACC(j + 1), 28);                 /* 0x6B87E, 0x6B8AE, 0x6B8CB */
    for (i = 0; i < S16(NRESP); i++) {                              /* 0x6B8D2, 0x6B8DF */
        if (S32(ACC(i) + 0x10u) < 0)                                /* 0x6B8F8, 0x6B901 jl */
            continue;
        f75074(S32(ACC(i) + 8u));                                   /* 0x6B90F, 0x6B916 */
        return;
    }
}
