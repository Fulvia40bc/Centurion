/* Enemy armies and fleets at the end of a turn: literal translation of EXE func_0x4C24 and the routines it calls
 * (0x4DB5, 0x4E2D, 0x4EC1, 0x4F27, 0x541F, 0x4816) plus the dice 0x076B. Works on the DS image (state.h).
 *
 * Army record i (0..10) at A = DS:C112 + 14*i (save block C112, 196 bytes = 14 records; 11-13 hold the enemy fleets' strength):
 *   A+0 mode (1 = wanders at random, 0 = only into weaker zones), A+1 current zone, A+2 home zone (word),
 *   A+4 / A+6 strengths (grow toward the home province's +0x02 / +0x00), A+8 active (0xFFFF), A+0x0A..0x0C from home +0x10/+0x0F/+0x0E.
 * Fleet record k (0..5) at F = DS:110E + 12*k: F+0..5 three ship counts (by type), F+6 zone, F+7 home province, F+8 0xFF = none
 *   (Roman fleets: 0 = may move), F+0x0A ships shown. Fleets 3-5 are the enemy's.
 *
 * NOT YET PORTED (reported on stderr, and g_enemy_unported is set): the land battle (overlay call 0xB06E), the naval battle
 * (0xB0C2), garrison setup 0x4733, 0x531F, fleet retreat 0x5244; and the two places where the original reads an
 * uninitialised local (noted below). Messages (0x4BAC, 0x59D4) and disk swaps (0x1E21) are UI only; they are logged.
 */
#include "state.h"
#include <stdio.h>

int g_enemy_unported;
int g_original_bugs;

/* 0x075A: seed from the BIOS tick count (int 1Ah: CX = high word, DX = low word) */
void rng_seed(uint32_t ticks) { setW(0xC8, (uint16_t)(ticks >> 16)); setW(0xCA, (uint16_t)ticks); }

/* 0x076B: roll(n) = high word of ((s1 ^ s2) * n), 0..n-1 */
uint16_t rng_roll(uint16_t n) {
    uint16_t s1 = (uint16_t)(W(0xC8) * 0x7ABDu + 0x1B0Fu);
    uint16_t s2 = (uint16_t)(W(0xCA) * 0x1B0Fu + 0x7ABDu);
    setW(0xC8, s1);
    setW(0xCA, s2);
    return (uint16_t)(((uint32_t)(uint16_t)(s1 ^ s2) * n) >> 16);
}

#define ARMY(i)  ((uint16_t)(0xC112 + 14 * (i)))
#define FLEET(k) ((uint16_t)(0x110E + 12 * (k)))

static void unported(const char *what, int a, int b) {
    g_enemy_unported = 1;
    fprintf(stderr, "enemy: %s (%d, %d) NOT YET PORTED\n", what, a, b);
}

/* zone strength: province record +0x1A, or DS:3D55[zone] for the seas */
static uint8_t zone_strength(unsigned z) { return z < 0x1B ? B((z << 5) + 0x528A) : B(0x3D55 + z); }

/* 0x4816: legion j is lost */
void legion_lost(int j) {
    uint16_t b = (uint16_t)(20 * j);
    B(b + 0x1925) = 0xFF;
    uint16_t n = (uint16_t)(0x1A20 + 6 * B(b + 0x1928));
    if (B(n) != 2) B(n) = 1;
    B(b + 0x1924) = 0xFF;
}

/* 0x541F: province p left Roman hands; an enemy fleet based there is restored */
static void fleets_province_lost(unsigned p) {
    for (int k = 0; k < 3; k++) {
        uint16_t f = FLEET(3 + k);
        if (B(f + 7) != p) continue;
        uint16_t sum = 0;
        for (int t = 0; t < 3; t++) { sum = (uint16_t)(sum + W(f + 2 * t)); setW(f + 2 * t, 0); }
        uint16_t a = (uint16_t)(f + 2 * W(0x3D90 + 2 * k));
        setW(a, W(0x3D96 + 2 * k));
        if (sum == 0) setW(a, (uint16_t)(W(a) / 5));
        B(f + 8) = 0;
    }
}

/* 0x4EC1: army i appears in zone (only if that zone is not Roman) */
static void army_spawn(int i, unsigned zone, uint16_t home) {
    uint16_t p = (uint16_t)(zone << 5), a = ARMY(i);
    if (B(p + 0x5285) != 0) return;
    B(a + 1) = (uint8_t)zone;
    setW(a + 2, home);
    setW(a + 6, W(p + 0x526C));
    setW(a + 4, W(p + 0x526E));
    B(a + 0x0A) = B(p + 0x527C);
    B(a + 0x0B) = B(p + 0x527B);
    B(a + 0x0C) = B(p + 0x527A);
    setW(a + 8, 0xFFFF);
}

/* 0x4E2D: a rebel army (slots 7-10) rises in a random non-Roman province */
static void rebel_spawn(void) {
    int di = -1;
    for (int k = 7; k < 11; k++) if (W(ARMY(k) + 8) == 0) { di = k; break; }
    if (di < 0) {   /* the original uses an uninitialised local here; it skips when that is >= 11 */
        fprintf(stderr, "enemy: no free rebel slot (original reads an uninitialised local); skipped\n");
        return;
    }
    int any = 0;
    for (uint16_t b = 0x52A5; b < 0x55E5; b += 0x20) if (B(b) == 0) any = 1;
    if (!any) return;
    unsigned si;
    do si = (unsigned)rng_roll(0x1A) + 1; while (B((si << 5) + 0x5285) != 0);
    B(ARMY(di)) = rng_roll(10) > 4 ? 0 : 1;
    army_spawn(di, si, (uint16_t)si);
}

/* 0x4DB5: a foreign power declares war; which one depends on the era (table DS:3D2E, 7 per era) */
static void war_spawn(void) {
    unsigned era = (unsigned)(uint16_t)(W(0x634C) - 5) >> 3;
    if (era > 4) era = 4;
    uint16_t r = rng_roll(100);
    unsigned si = 0, acc = 0;
    while (si < 7) {
        acc += B(0x3D2E + 7 * era + si);
        if (acc > r) break;
        si++;
    }
    if (W(ARMY(si) + 8) != 0) { rebel_spawn(); return; }
    B(ARMY(si)) = 0;
    army_spawn((int)si, B(0x3D58 + si), B(0x3D60 + si));
}

/* 0x4F27: army i moves */
static void army_move(int i) {
    uint16_t a = ARMY(i);
    unsigned z = B(a + 1);
    uint8_t list[8];
    unsigned n = 0;
    if (B(a) == 1) {
        for (unsigned s = 0; s < 8; s++) {
            uint8_t nb = B(0x50EC + z * 8 + s);
            if (nb != 0xFF) list[n++] = nb;
        }
    } else {
        if (z == 0) {   /* an army standing in Rome: deactivated, then the original reads an uninitialised local */
            setW(a + 8, 0);
            fprintf(stderr, "enemy: army %d in Rome (original reads an uninitialised local); stopped\n", i);
            return;
        }
        uint8_t mine = zone_strength(z);
        for (unsigned s = 0; s < 8; s++) {
            uint8_t nb = B(0x50EC + z * 8 + s);
            if (nb != 0xFF && mine > zone_strength(nb)) list[n++] = nb;
        }
    }
    uint16_t pick = rng_roll((uint16_t)n);
    if (n == 0) {   /* the original takes an uninitialised list entry here */
        fprintf(stderr, "enemy: army %d has nowhere to go (original reads an uninitialised local); stays\n", i);
        return;
    }
    unsigned t = list[pick];

    if (t >= 0x1B) {   /* 0x51C5: out to sea; Roman fleets there must fight */
        B(a + 1) = (uint8_t)t;
        for (int k = 0; k < 3; k++) {
            uint16_t f = FLEET(k);
            if (B(f + 8) == 0xFF || B(f + 6) != t) continue;
            fprintf(stderr, "message 0x4BAC: army %d meets Roman fleet %d in zone %u\n", i, k, t);
            unported("0x531F + naval battle 0xB0C2 (+ retreat 0x5244)", i, k);
            return;    /* outcome unknown without the battle; stop here */
        }
        return;
    }

    if (t == 0) {   /* 0x4FFD: Rome's garrison as pseudo-legion 12 (DS:1A08, not saved) */
        B(0x1A13) = B(0x527A);
        B(0x1A15) = 2;
        B(0x1A19) = 0xFF;
        B(0x1A12) = 0; B(0x1A14) = 0; B(0x1A16) = 0; B(0x1A17) = 0; B(0x1A1A) = 0;
    }
    for (int j = 0; j < 12; j++) {   /* 0x501E: every legion in the zone fights */
        uint16_t s = (uint16_t)(0x1925 + 20 * j);
        if (B(s) == 0xFF || B(s - 1) != t) continue;
        fprintf(stderr, "message 0x4BAC: army %d meets legion %d in zone %u\n", i, j, t);
        unported("land battle 0xB06E", i, j);
        return;
    }
    if (t == 0 && W(a + 8) != 0 && B(0x5285) != 0) {   /* 0x508D */
        fprintf(stderr, "message: %s%s%s\n", (const char *)&g_ds[0x3D9D], (const char *)&g_ds[W(0x5222 + 2 * W(a + 2))],
                (const char *)&g_ds[0x3DB4]);
        unported("garrison 0x4733 + land battle 0xB06E", i, 12);
        return;
    }
    if (W(a + 8) == 0) return;
    for (int j = 0; j < 12; j++)   /* 0x5125 */
        if (B(0x1924 + 20 * j) == t) legion_lost(j);
    B(a + 1) = (uint8_t)t;
    uint16_t p = (uint16_t)(t << 5);
    if (B(p + 0x5285) == 0) return;
    /* 0x5175: an undefended Roman province falls */
    B(p + 0x5287) = 0xFF;
    B(p + 0x5285) = 0;
    fleets_province_lost(t);
    {   /* reads the status after zeroing it, so this is 634E * 125 / 100, as in the original */
        int16_t ax = (int16_t)((5 - B(p + 0x5285)) * (int16_t)B(0x634E));
        ax = (int16_t)(ax * 25);
        B(0x634E) = (uint8_t)(ax / 100);
    }
    score_add(t > 5 ? -5 : t == 2 ? -30 : -15);
    fprintf(stderr, "enemy: army %d takes province %u\n", i, t);
}

/* 0x4C24 */
void enemy_turn(void) {
    if (W(0x634C) < 5) return;
    for (int i = 0; i < 11; i++)
        if (W(ARMY(i) + 8) != 0) army_move(i);

    B(0x3D9C) = (uint8_t)add_cap(B(0x3D9C), 10, 50);
    if ((uint8_t)rng_roll(100) < B(0x3D9C)) {
        war_spawn();
        B(0x3D9C) = 10;
    } else {
        unsigned n = 0;
        for (uint16_t b = 0x5285; b < 0x55E5; b += 0x20) if (B(b) != 0) n++;
        uint8_t cl = (uint8_t)rng_roll(100);
        if (B(0x3D52 + n / 5) > cl) rebel_spawn();
    }

    for (int i = 0; i < 11; i++) {   /* 0x4CAE: armies regain strength toward their home's levels */
        uint16_t a = ARMY(i);
        if (W(a + 8) == 0) continue;
        uint16_t p = (uint16_t)(W(a + 2) << 5);
        uint16_t v = (uint16_t)(W(a + 6) + 100), c = W(p + 0x526C);
        setW(a + 6, c > v ? v : c);
        v = (uint16_t)(W(a + 4) + 20); c = W(p + 0x526E);
        setW(a + 4, c > v ? v : c);
    }

    for (int k = 0; k < 3; k++) {    /* 0x4CF2: enemy fleets 3-5 rebuild */
        uint16_t f = FLEET(3 + k);
        if (B(f + 8) == 0xFF) continue;
        unsigned type = W(0x3D90 + 2 * k);
        uint16_t sw = (uint16_t)(0xC1B2 + 14 * k);
        uint16_t cap = (uint16_t)(W(sw) + 100);
        if (cap > 8000) cap = 8000;
        uint16_t c = (uint16_t)(f + 2 * type);
        if (W(c) < W(0x3D96 + 2 * k)) setW(c, (uint16_t)(W(c) + 1));
        uint16_t cnt = W(c);
        uint16_t s = (uint16_t)(cnt * B(0x8EDE + type));
        if (s > cap) s = cap;
        setW(sw, s);
        if (cnt == 0) { fprintf(stderr, "enemy: fleet %d has no ships (the original divides by zero here)\n", 3 + k); continue; }
        B(f + 10) = (uint8_t)(s / cnt);
    }
}
