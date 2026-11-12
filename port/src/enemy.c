/* Enemy armies and fleets at the end of a turn: literal translation of EXE func_0x4C24 and the routines it calls
 * (0x4DB5, 0x4E2D, 0x4EC1, 0x4F27, 0x541F, 0x4816) plus the dice 0x076B. Works on the DS image (state.h).
 *
 * Army record i (0..10) at A = DS:C112 + 14*i (save block C112, 196 bytes = 14 records; 11-13 hold the enemy fleets' strength):
 *   A+0 mode (1 = wanders at random, 0 = only into weaker zones), A+1 current zone, A+2 home zone (word),
 *   A+4 / A+6 strengths (grow toward the home province's +0x02 / +0x00), A+8 active (0xFFFF), A+0x0A..0x0C from home +0x10/+0x0F/+0x0E.
 * Fleet record k (0..5) at F = DS:110E + 12*k: F+0..5 three ship counts (by type), F+6 zone, F+7 home province, F+8 0xFF = none
 *   (Roman fleets: 0 = may move), F+9 legion aboard / the marines' army record, F+0x0A men per ship. Fleets 3-5 are the
 *   enemy's (docs/FLEET_NOTES.md section 1).
 *
 * The naval path of 0x4F27 (0x51C5..0x523C) calls fleet.c (0x531F, 0x5244) and naval.c (0xB0C2 -> section 5 B12A: fought
 * on screen; with no screen, --end-turn, it returns 0 = cancelled and sets g_enemy_unported). NOT YET PORTED (reported on
 * stderr, and g_enemy_unported is set): a land battle
 * (0xB06E) when no battle choices are available; and the two places where the original reads an uninitialised local
 * (noted below). Messages (0x4BAC, 0x59D4, the tail of 0x4733) go through game_message() (endturn.c):
 * a message box when a screen is open, else stderr. Disk swaps (0x1E21) are not needed.
 */
#include "state.h"
#include "fleet.h"
#include "naval.h"
#include <stdio.h>
#include <string.h>

void game_message(const char *s);   /* endturn.c */
void game_not_ported(const char *what);   /* endturn.c: "<what>\nis not ported yet." box when a screen is open */

static char *dsp(uint16_t a) { return (char *)&g_ds[a]; }

/* 0x4BAC(name, numeral, zone, army) "An army from <home>\ninvades <zone>\n<numeral><name> meets the enemy on\nthe <field of
   battle. | rolling waves of destiny.>": the word table DS:3D7A..3D8A gets its variable slots ([3D7C] = home name 5222[army
   +2], [3D80] = zone name, [3D86] = name, [3D8A] = 3D8C on land / 3D8E at sea; [3D84] is C072, filled by 6738(numeral)),
   then AC60 = [3D7A] + the words 3D7C..3D8A; 59D4(AC60). */
void army_meets_4bac(uint16_t name, unsigned numeral, unsigned zone, unsigned army) {   /* also fleet.c 4ADA */
    setW(0x3D7C, W(0x5222 + 2 * W(0xC114 + 14 * army)));
    setW(0x3D80, W(0x5222 + 2 * zone));
    {
        char num[32];
        roman_numeral(numeral, num);                                 /* 6738 -> C072 */
        strcpy(dsp(0xC072), num);
    }
    setW(0x3D86, name);
    setW(0x3D8A, zone >= 0x1B ? W(0x3D8E) : W(0x3D8C));
    strcpy(dsp(0xAC60), dsp(W(0x3D7A)));                             /* 9E32 */
    for (uint16_t si = 0x3D7C; si < 0x3D8C; si += 2) strcat(dsp(0xAC60), dsp(W(si)));   /* 9DF2 */
    game_message(dsp(0xAC60));
}

/* 0x4733's message (47C6..480B), after battle.c's assign_commander() made the choice: AC60 = commander name [1A1C + 6c] +
   1F89 " is assigned\nto command " + 6738(legion +0x0A) (C072) + legion name; 59D4(AC60). Nothing when the legion's
   +0x0D is FFh (474C). As legions.c's commander_msg_4733. */
static void commander_msg_4733(unsigned L) {
    uint16_t rec = (uint16_t)(20 * L);
    if (B(rec + 0x1925) == 0xFF) return;
    uint8_t c = B(rec + 0x1928);
    char num[32];
    roman_numeral(B(rec + 0x1922), num);
    strcpy(dsp(0xC072), num);
    strcpy(dsp(0xAC60), dsp(W(0x1A1C + 6 * c)));
    strcat(dsp(0xAC60), dsp(0x1F89));
    strcat(dsp(0xAC60), dsp(0xC072));
    strcat(dsp(0xAC60), dsp((uint16_t)(rec + 0x1918)));
    game_message(dsp(0xAC60));
}

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
void fleets_province_lost(unsigned p) {   /* also called by endturn.c's revolt handler */
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

    if (t >= 0x1B) {   /* 0x51C5: out to sea (the army is there whatever follows); Roman fleets there must fight */
        B(a + 1) = (uint8_t)t;
        for (int k = 0; k < 3; k++) {                                        /* [bp-1] */
            uint16_t f = FLEET(k);
            if (B(f + 8) == 0xFF || B(f + 6) != t) continue;
            army_meets_4bac(W(0x1100 + 2 * k), 0, t, (unsigned)i);   /* 0x520E: fleet name, numeral 0 */
            /* 1E21 */
            fleet_army_to_record6_531f((uint16_t)i);                 /* 0x5217: a new die for every Roman fleet */
            uint16_t r = naval_battle_b12a(6, (uint8_t)k);           /* 0x5221 thunk B0C2: push k, 6 */
            if (r & 2) return;                                       /* 0x5226: the army is beaten (2) or both gone (3) */
            if (r & 4) fleet_retreat_5244((uint8_t)k);               /* 0x522C `test al, 4`: both afloat, Rome retreats */
        }                                                            /* 0 (cancelled, FLEET_NOTES 9.4) or 1 (lost): next fleet */
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
        army_meets_4bac((uint16_t)(s - 0x0D), B(s - 3), t, (unsigned)i);   /* 0x5049: legion name, its +0x0A */
        uint16_t won;
        if (!battle_auto(i, j, (int)t, &won)) { unported("land battle 0xB06E", i, j); return; }
        if (won) { setW(a + 8, 0); break; }   /* 0x5063 */
    }
    if (t == 0 && W(a + 8) != 0 && B(0x5285) != 0) {   /* 0x508D */
        {   /* 0x508D: a local buffer = 3D9D "The Marauding Army of " + home name + 3DB4 "\nMeets the Roman Garrison"; 59D4 */
            char m[160];
            snprintf(m, sizeof(m), "%s%s%s", dsp(0x3D9D), dsp(W(0x5222 + 2 * W(a + 2))), dsp(0x3DB4));
            game_message(m);
        }
        uint8_t c = B(0x1A18);
        if (c == 0xFF || B(0x1A20 + 6 * c) != 0) { assign_commander(12); commander_msg_4733(12); }   /* 0x50E0: 4733(12) */
        setW(0x1A0E, W(0x5270)); setW(0x1A10, W(0x5272));   /* 0x50E3: Rome's troops into the garrison */
        uint16_t won;
        if (!battle_auto(i, 12, 0, &won)) { unported("land battle 0xB06E (garrison)", i, 12); return; }
        if (won) { setW(0x5270, W(0x1A0E)); setW(0x5272, W(0x1A10)); setW(a + 8, 0); }
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
