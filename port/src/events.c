/* Random events at the end of a turn: literal translation of EXE func_0x1E98 (called by End Turn when turn > 4) and the
 * routines it calls: 0x21A4 (the two crises: Rome demands games, a governor demands games) and 0x3A32 (a legion aboard a
 * fleet that lost ships). Works on the DS image (state.h).
 *
 * Event table DS:0F02, 19 x {kind, score change, talents change}: kind 0 = once per game (DS:0EDC marks used), 1 = repeatable.
 * Talents change: -128 = half this turn's tribute, 127 = all of it, else a fixed amount. Texts: DS:0F48[i] (or built, 0/1/3).
 * Messages (0x59D4) are UI; here they are logged on stderr.
 */
#include "state.h"
#include <stdio.h>
#include <string.h>

static char g_msg[640];
static void msg_cat(uint16_t a) { strncat(g_msg, (const char *)&g_ds[a], sizeof(g_msg) - strlen(g_msg) - 1); }
static void msg_show(void) { fprintf(stderr, "message: %s\n", g_msg); g_msg[0] = 0; }

/* 0x21A4: the crises. DS:0ED8 = Rome is bored, DS:0EDA = a governor (province DS:0ED6) wants games; chances DS:0ED4/0ED5 */
static void crises(void) {
    if (W(0x0EDA) == 0 && (int8_t)rng_roll(100) < SB(0x0ED4)) setW(0x0ED8, 0xFFFF);
    if (W(0x0ED8) == 0 && (int8_t)rng_roll(100) < SB(0x0ED5)) setW(0x0EDA, 0xFFFF);
    int v = SB(0x0ED4) + 5; if (v > 100) v = 100; B(0x0ED4) = (uint8_t)v;
    v = SB(0x0ED5) + 5;     if (v > 100) v = 100; B(0x0ED5) = (uint8_t)v;
    if (W(0x0ED8)) {
        if (B(0x5285) == 4) { msg_cat(0x0F6E); msg_show(); }
        return;
    }
    if (W(0x0EDA) == 0) return;
    if (B(0x0ED6) == 0) {   /* pick one of the Roman provinces 1-26 */
        int n = 0;
        for (uint16_t b = 0x52A5; b < 0x55E5; b += 0x20) if (B(b) != 0) n++;
        if (n > 0) {
            int dx = (int16_t)rng_roll((uint16_t)n);
            B(0x0ED6) = 1;
            int k = 1;
            for (uint16_t b = 0x52A5; b < 0x55E5; b += 0x20, k++) {
                if (B(b) == 0) continue;
                if (dx == 0) { B(0x0ED6) = (uint8_t)k; break; }
                dx--;
            }
        }
    }
    if (B(0x0ED6) != 0) {
        msg_cat(0x0FB2); msg_cat(W(0x5222 + 2 * SB(0x0ED6))); msg_cat(0x0FC3); msg_show();
        return;
    }
    setW(0x0EDA, 0);
}

/* 0x3A32: legion L aboard fleet k: the fleet's capacity (ships x DS:8EDE, max 10000) limits the troops
 * (legion +8 counts 4 each, +6 counts 1 each); returns the men lost (unused by the caller) */
static uint16_t cargo_fit(uint16_t *pa, uint16_t *pb, unsigned L, unsigned k) {
    uint32_t cap = 0;
    for (unsigned t = 0; t < 3; t++) cap += (uint32_t)W(0x110E + 12 * k + 2 * t) * B(0x8EDE + t);
    uint16_t bx = cap >= 10000 ? 10000 : (uint16_t)cap;
    *pb = W(0x1920 + 20 * L);
    *pa = W(0x191E + 20 * L);
    uint16_t dx;
    if ((uint16_t)(*pb << 2) >= bx) {
        uint16_t ax = (uint16_t)(bx >> 2);
        dx = (uint16_t)(*pb - ax);
        *pb = ax;
        bx = 0;
    } else {
        bx = (uint16_t)(bx - (uint16_t)(*pb << 2));
        dx = 0;
    }
    if (*pa >= bx) { dx = (uint16_t)(dx + (*pa - bx)); *pa = bx; }
    return dx;
}

/* 0x1E98 */
void random_events(uint16_t tribute) {
    crises();
    if (W(0x0ED8) || W(0x0EDA)) return;
    if ((int16_t)rng_roll(100) >= 20) return;
    unsigned idx;
    for (;;) {
        idx = rng_roll(19);
        uint8_t kind = B(0x0F02 + 3 * idx);
        if ((kind == 0 && W(0x0EDC + 2 * idx) == 0) || kind == 1) break;
    }
    setW(0x0EDC + 2 * idx, 0xFFFF);
    g_msg[0] = 0;

    switch (idx) {
    case 0:     /* storm, the navy is caught at sea (10 + roll(10) per cent of the ships) */
    case 1: {   /* storm, the navy rides at anchor (roll(10) per cent) */
        uint16_t pct = (uint16_t)((idx == 0 ? 10 : 0) + rng_roll(10));
        uint16_t lost = 0;
        unsigned k = 0;
        for (uint16_t si = 0x1117; si < 0x113B; si += 12, k++) {   /* Roman fleets 0-2, si = record +9 */
            if (B(si - 1) == 0xFF || B(si - 3) >= 0x22) continue;      /* none, or not in the Mediterranean */
            uint16_t left = 0;
            for (unsigned t = 0; t < 3; t++) {
                uint16_t a = (uint16_t)(si - 9 + 2 * t), c = W(a);
                if (c == 0) continue;
                uint16_t l = (uint16_t)((uint16_t)(c * pct) / 100);
                if (l == 1) l = 2;
                lost = (uint16_t)(lost + l);
                setW(a, (uint16_t)(c - l));
                left = (uint16_t)(left + W(a));
            }
            unsigned L = B(si);                                          /* legion aboard */
            if (L == 0xFF) continue;
            uint16_t pa, pb;
            cargo_fit(&pa, &pb, L, k);
            setW(0x1920 + 20 * L, pb);
            setW(0x191E + 20 * L, pa);
            if (left == 0) { fprintf(stderr, "events: fleet %u sank with a legion aboard (the original divides by zero here)\n", k); continue; }
            B(si + 1) = (uint8_t)((uint16_t)((uint16_t)(pb << 2) + pa) / left);
        }
        if (lost == 0) return;
        char num[8];
        snprintf(num, sizeof(num), "%u", lost);
        msg_cat(W(0x0F3C)); msg_cat(idx == 0 ? W(0x0F40) : W(0x0F42));
        strncat(g_msg, num, sizeof(g_msg) - strlen(g_msg) - 1);
        msg_cat(W(0x0F3E));
        break;
    }
    case 2:     /* text + rank name + "." */
        msg_cat(W(0x0F48 + 2 * idx)); msg_cat(W(0x5974 + 2 * B(0x5971))); msg_cat(0x0ED1);
        break;
    case 3: {   /* disease: every legion in the zone of legion 0 loses a tenth of its +6 troops */
        int any = 0;
        for (uint16_t b = 0x1925; b < 0x1A15; b += 0x14) if (B(b) != 0xFF) any = 1;
        if (!any) return;
        rng_roll(0);   /* the original rolls with n = 0, so it always picks legion record 0 */
        uint8_t zone = B(0x1924);
        for (uint16_t si = 0x191E; si < 0x1A0E; si += 0x14)
            if (B(si + 6) == zone) setW(si, (uint16_t)(W(si) - W(si) / 10));
        msg_cat(W(0x0F44)); msg_cat(W(0x5222 + 2 * zone)); msg_cat(W(0x0F46));
        break;
    }
    default:
        msg_cat(W(0x0F48 + 2 * idx));
        break;
    }

    /* 0x1FB0: effects */
    score_add(SB(0x0F03 + 3 * idx));
    int8_t r = SB(0x0F04 + 3 * idx);
    if (r == -128)      setW(0x634A, (uint16_t)(W(0x634A) - (tribute >> 1)));
    else if (r == 127)  setW(0x634A, (uint16_t)(W(0x634A) - tribute));
    else if (r < 0)     setW(0x634A, (uint16_t)(-r) < W(0x634A) ? (uint16_t)(W(0x634A) + r) : 0);
    else                setW(0x634A, (uint16_t)(W(0x634A) + r));
    msg_show();
}
