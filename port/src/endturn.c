/* End Turn: literal translation of EXE func_0x09BA (disasm/EXE_RUNTIME.clean.asm) and the routines it calls.
 * Works on the DS image (state.h) with the original addresses and integer semantics (8/16-bit, signed where the
 * original uses cbw/imul/idiv/jg/jl, unsigned where it uses mul/div/ja/jb).
 *
 * Province records: 27 x 32 bytes from DS:5270. The loop pointer si = 0x5285 + 32*k (record +0x15, the status byte).
 * Legion records: 12 x 20 bytes from DS:1918 (zone at +0x0C, the loop pointer si = record + 0x0D).
 *
 * NOT YET PORTED (reports on stderr if reached):
 *   - the rest of the revolt handler func_0x0D13 after the state reset (messages + legions in the revolting province)
 *   - battles reached from func_0x4C24 (enemy.c; events.c holds 0x1E98)
 *   - messages / screen updates (0x1D55, 0x1D8A, 0x494A map redraw): UI only, no state.
 */
#include "state.h"
#include <stdio.h>
#include <stdint.h>

/* 0x04FA: min(v + step, max), signed compare */
int add_cap(int v, int step, int max) { int r = (int16_t)(v + step); return r > max ? max : r; }
/* 0x050F: max(v - step, min), signed compare */
static int sub_floor(int v, int step, int min) { int r = (int16_t)(v - step); return r < min ? min : r; }
/* idiv-style truncation of a signed 16-bit value by 2^n, as the cdq/xor/sub/sar/xor/sub idiom does */
static int sdiv_pow2(int16_t v, int n) { int a = v < 0 ? -v : v; a >>= n; return v < 0 ? -a : a; }

/* 0x7330: score (DS:594A) += n, capped to 0..256 */
void score_add(int n) {
    if (n >= 0) setW(0x594A, (uint16_t)add_cap(SW(0x594A), n, 0x100));
    else        setW(0x594A, (uint16_t)sub_floor(SW(0x594A), -n, 0));
}

/* 0x6C26: province mood 0..3 = ((+0x10) - (+0x0E) + (+0x0D)) / 25, signed */
static int mood(int k) {
    uint16_t bx = (uint16_t)(k << 5);
    int a = SB(bx + 0x527E), b = SB(bx + 0x5280), c = SB(bx + 0x527D);
    int v = (int16_t)(b - a + c) / 25;
    return v < 0 ? 0 : v > 3 ? 3 : v;
}

/* 0x0D13: revolt check for province k */
static void revolt_check(int k) {
    uint16_t bx = (uint16_t)(k << 5);
    int8_t diff = (int8_t)(B(bx + 0x527E) - B(bx + 0x527D));
    if (!(diff > SB(bx + 0x5280))) return;
    /* the province throws off the Roman yoke */
    int16_t ax = (int16_t)(5 - B(bx + 0x5285));
    ax = (int16_t)(ax * (int16_t)B(0x634E));
    ax = (int16_t)(ax * 25);
    B(0x634E) = (uint8_t)(ax / 100);
    score_add(k > 5 ? -5 : k == 2 ? -30 : -15);
    B(bx + 0x5285) = 0;
    B(bx + 0x5287) = 0xFF;
    setW(bx + 0x5270, (uint16_t)((int16_t)(20 * SB(bx + 0x527E)) + W(bx + 0x5274)));
    setW(bx + 0x5272, 0);
    fprintf(stderr, "end_turn: province %d revolts; legion handling of 0x0D98-0x0EDF NOT YET PORTED\n", k);
}

/* 0x72DE: rank (DS:5971) from score DS:594A against the thresholds at DS:594E */
static void rank_update(void) {
    int r = 0;
    while (r != 8 && SW(0x594E + 2 * r) < SW(0x594A)) r++;
    if (B(0x5971) < r) B(0x5971) = (uint8_t)r;
    if (B(0x5971) == 8) {
        for (uint16_t s = 0x5285; s < 0x55E5; s += 0x20)
            if (B(s) == 0) { B(0x5971) = 7; break; }
    }
}

void end_turn(void) {
    int tribute = 0;                                            /* [bp-0x0A] */

    /* 0x09CE */
    B(0x634E) = (uint8_t)add_cap(B(0x634E), 5, 100);
    if (W(0x0ED8))
        B(0x527E) = (uint8_t)add_cap(SB(0x527E), 15, 100);
    else if (W(0x0EDA)) {
        uint16_t bx = (uint16_t)(SB(0x0ED6) << 5);
        B(bx + 0x527E) = (uint8_t)add_cap(SB(bx + 0x527E), 15, 100);
    }

    /* 0x0A26: legions */
    for (uint16_t si = 0x1925; si < 0x1A15; si += 0x14) {
        if (B(si) == 0xFF) continue;
        B(si + 1) = 0;
        B(si - 2) = (uint8_t)add_cap(B(si - 2), 2, 99);
        if (B(si + 4) != 0xFF) continue;
        setW(si - 7, (uint16_t)add_cap(SW(si - 7), 100, SW(0x1AAC + 2 * B(si))));
        if (B(si) == 0) continue;
        setW(si - 5, (uint16_t)add_cap(SW(si - 5), 10, SW(0x1AB2 + 2 * B(si))));
    }

    /* 0x0A8A: Roman fleets (records 0-2) */
    for (uint16_t bx = 0x1116; bx < 0x113A; bx += 0x0C)
        if (B(bx) != 0xFF) B(bx) = 0;

    /* 0x0AA0: provinces */
    int k = 0;
    for (uint16_t si = 0x5285; si < 0x55E5; si += 0x20, k++) {
        B(si + 4)++;                                             /* +0x19 */
        if (B(si) == 0 || si == 0x5285) {
            setW(si - 0x15, (uint16_t)add_cap(SW(si - 0x15), 100, SW(si - 0x19)));
            setW(si - 0x13, (uint16_t)add_cap(SW(si - 0x13), 25, SW(si - 0x17)));
        }
        if (B(si) == 0) continue;                                /* 0x0AD9 */

        uint16_t c = W(si - 0x0F);                               /* [bp-0x0C] */
        uint16_t step = (uint16_t)(c / 40);
        if (step < 50) step = 50;
        setW(si - 0x11, (uint16_t)add_cap(SW(si - 0x11), step, (int16_t)c));

        int16_t prod = (int16_t)(B(si - 6) * B(si));             /* mul byte -> ax, then cdq */
        int di = sdiv_pow2(prod, 2);
        int bl = B(si - 7);                                      /* [bp-1] */
        switch (B(si - 3)) {                                     /* tribute level +0x12 */
        case 0: B(si - 4) = 0;                               bl = sub_floor((int8_t)bl, 10, 0);  break;
        case 1: B(si - 4) = (uint8_t)sdiv_pow2((int16_t)di, 2); bl = sub_floor((int8_t)bl, 5, 0); break;
        case 2: B(si - 4) = (uint8_t)sdiv_pow2((int16_t)di, 1); bl = add_cap((int8_t)bl, 5, 100); break;
        case 3: B(si - 4) = (uint8_t)sdiv_pow2((int16_t)(di * 3), 2); bl = add_cap((int8_t)bl, 10, 100); break;
        case 4: B(si - 4) = (uint8_t)di;                     bl = add_cap((int8_t)bl, 20, 100);  break;
        default: break;
        }
        /* 0x0BBA */
        if (B(si - 2) < B(si - 3)) {
            if (B(0x634E) < 15) B(0x634E) = 0; else B(0x634E) -= 15;
            B(si - 7) = (uint8_t)add_cap(SB(si - 7), 30, 100);  /* overwritten below, as in the original */
        }
        if ((int8_t)bl < 1) bl = 1;
        B(si - 7) = (uint8_t)bl;
        B(si - 0x0D) = (uint8_t)di;

        /* 0x0BF9 */
        uint16_t q = (uint16_t)((uint16_t)(50u * W(si - 0x11)) / W(si - 0x0F));
        int16_t t = (int16_t)(B(si - 4) * -50);
        int16_t bx = (int16_t)(q + t / (int16_t)(di + 1) + 50);
        int16_t ax;
        if (si == 0x5285) {                                      /* Rome: 0x0C26, long mul/div */
            int32_t p = (int32_t)((uint32_t)B(0x634E) * (uint16_t)bx);
            p = p * (int32_t)SW(0x594A);
            ax = (int16_t)(p / 0x639C);
        } else {                                                 /* 0x0C45 */
            uint32_t p = (uint32_t)(uint16_t)(B(0x634E) * B(si)) * (uint16_t)bx;
            ax = (int16_t)(p / 400u);
        }
        if (ax < 1) ax = 1;
        if (ax > 99) ax = 99;
        B(si - 5) = (uint8_t)ax;                                 /* +0x10 */

        revolt_check(k);                                         /* 0x0C69 */
        if (B(si) != 0 && B(0x5285) != 0) {
            tribute += B(si - 4);
            if ((uint8_t)(25 * B(si)) < B(0x634E) && B(si + 4) > 5 && mood(k) >= 2) {
                B(si + 4) = 0;
                B(si)++;
                if (B(si) > 4) B(si) = 4;
                else score_add(1);
            }
        }
    }

    /* 0x0CC1 */
    enemy_turn();                                                /* 0x4C24, enemy.c (returns at once before turn 5) */
    if (B(0x634E) > 75) score_add(1);
    rank_update();
    setW(0x634C, (uint16_t)(W(0x634C) + 1));                    /* turn */
    setW(0x634A, (uint16_t)(W(0x634A) + tribute));              /* treasury */
    {   /* 0x0CE9: [bp-4] = 27 after the loop, so this reads DS:55E1 */
        uint32_t acc = (uint32_t)W(0xC092) | ((uint32_t)W(0xC094) << 16);
        acc += B((27 << 5) + 0x5281);
        setW(0xC092, (uint16_t)acc); setW(0xC094, (uint16_t)(acc >> 16));
    }
    if (W(0x634C) > 4) random_events((uint16_t)tribute);        /* 0x0D07: 0x1E98, events.c */
}
