/* The land battle: OVL0 section 2 (file 0x09A0-0x3EF0), loaded at 0823:B0E0, entry 0xDF24 (EXE thunk 0xB06E).
 * Literal translation of the simulation from disasm/OVL_BATTLE.asm; map and field names in docs/BATTLE_NOTES.md.
 * Claude, 2026-11-09.
 *
 * Everything lives in the DS image like in the original: the unit table (pointer DS:C292, count C294, 18-byte records), the
 * strength snapshot (C296), the 48x48 grid (C2AE) and the tactics buffer (76EA) are "calloc"ed at fixed DS addresses below,
 * so out-of-range reads behave as in the original (they read whatever DS holds there).
 *
 * Not here yet: graphics and the orders mode (D0F2..DC72). Orders are applied through battle_orders_hook at the point where
 * the original polls for them (after each tick), so a recorded battle can be replayed. The report is printed on stderr.
 */
#include <stdio.h>
#include <string.h>
#include "state.h"
#include "assets.h"
#include "screen.h"   /* battle_auto: g_battle_ui_attached, screen_scripted() */

#define UNITS   W(0xC292)
#define NUNITS  W(0xC294)
#define SNAP    W(0xC296)
#define GRID    W(0xC2AE)
#define U(i)    ((uint16_t)(UNITS + 0x12 * (i)))
#define LEG(l)  ((uint16_t)(0x1918 + 0x14 * (l)))
#define CMDR    ((uint16_t)(0x1A1C + 6 * B(0xC28C)))   /* +0 name, +2 charisma, +3 voice, +4 status */

/* fixed DS homes for the original's callocs (where they were in the captured game) */
#define DS_GRID     0xCF48u   /* 0x900 bytes */
#define DS_TACTICS  0xF356u   /* tactics.lzw, 0x275 bytes */
#define DS_UNITS    0xF5CEu

int g_battle_quirks;                      /* original quirks the port can't reproduce exactly (logged) */
/* A 1-wide unit (the general) makes CDC7 and B5E3 read a second front cell that C948 never wrote: a stale stack byte.
   Worked out from the frames (2026-11-09): for CDC7 called from the unit loop it is the high byte of B94D's local
   6*spritecount from the previous unit's B94D (so 0 below 43 sprites); for B5E3 called from CDC7 it is the high byte of
   the unit index CB3C pushed (0). g_stale keeps the former. */
static uint8_t g_stale;
int (*battle_orders_hook)(int tick);      /* player orders, called where D0F2 polls; NULL = no orders */

static void quirk(const char *what, int i) { g_battle_quirks++; fprintf(stderr, "battle: %s (unit %d)\n", what, i); }

/* ---- grid helpers ---- */
static void border(int x, int y) {                              /* C11E */
    if (y < 0 || y >= 0x30 || x < 0 || x >= 0x30) return;
    B(GRID + 0x30 * x + y) = 0xFE;
}
static void dims(uint16_t u, int swap_on_odd, uint8_t *a, uint8_t *b) {
    uint8_t w = B(0x6AE8 + B(u + 0xA)), d = B(0x6AEC + B(u + 0xA)), f = B(u + 0xE);
    int odd = (f == 1 || f == 3);
    if (swap_on_odd ? odd : !odd) { uint8_t t = w; w = d; d = t; }
    *a = w; *b = d;
}
static void erase(int i) {                                      /* CB3C */
    uint16_t u = U(i); uint8_t a, b; dims(u, 0, &a, &b);           /* swap on facing 0/2: dx < a, dy < b */
    for (uint8_t dx = 0; dx < a; dx++)
        for (uint8_t dy = 0; dy < b; dy++) {
            uint16_t c = (uint16_t)(GRID + (uint8_t)(B(u + 2) + dx) * 0x30 + (uint8_t)(B(u + 3) + dy));
            if (B(c) == (uint8_t)i) B(c) = 0;
        }
}
static void stamp(int i) {                                      /* CA55 */
    uint16_t u = U(i);
    if (B(u + 0xD) == 4) return;
    uint8_t a, b; dims(u, 0, &a, &b);
    for (uint8_t dx = 0; dx < a; dx++)
        for (uint8_t dy = 0; dy < b; dy++) {
            uint16_t c = (uint16_t)(GRID + (uint8_t)(B(u + 2) + dx) * 0x30 + (uint8_t)(B(u + 3) + dy));
            if (B(c) == 0) B(c) = (uint8_t)i;
            if (B(c) == 0xFE) { B(U(i) + 0xD) = 4; erase(i); return; }
        }
}
static uint8_t blocked(int i, uint16_t u) {                     /* C7E7: 0xFE border, 0xFF other unit, 0 free (last cell wins) */
    uint8_t a, b, r = 0; dims(u, 1, &a, &b);                    /* swap on facing 1/3: dx < b(orig d), dy < a */
    for (uint8_t dx = 0; dx < b; dx++)
        for (uint8_t dy = 0; dy < a; dy++) {
            uint8_t c = B(GRID + (uint8_t)(B(u + 2) + dx) * 0x30 + (uint8_t)(B(u + 3) + dy));
            if (c == 0xFE) r = c;
            else if (c != 0 && c != (uint8_t)i) r = 0xFF;
        }
    return r;
}
static uint8_t blocked_by_enemy(uint16_t u) {                   /* C890 (routed units pass through friends) */
    uint8_t a, b, r = 0, side = B(u + 0xB); dims(u, 1, &a, &b);
    for (uint8_t dx = 0; dx < b; dx++)
        for (uint8_t dy = 0; dy < a; dy++) {
            uint8_t c = B(GRID + (uint8_t)(B(u + 2) + dx) * 0x30 + (uint8_t)(B(u + 3) + dy));
            if (c == 0xFE) r = c;
            else if (c != 0 && B(UNITS + 0x12 * c + 0xB) != side) r = 0xFF;
        }
    return r;
}
static void front(uint8_t *buf, uint8_t dir, int i) {          /* C948: the w cells along edge dir (w = 6AE8[type], unswapped) */
    uint16_t u = U(i);
    uint8_t w = B(0x6AE8 + B(u + 0xA)), d = B(0x6AEC + B(u + 0xA)), x = B(u + 2), y = B(u + 3);
    for (uint8_t k = 0; k < w; k++) {
        uint16_t c;
        switch (dir) {
        case 0: c = (uint16_t)((x + d) * 0x30 + y + k); break;
        case 1: c = (uint16_t)(x * 0x30 + y + d + 0x30 * k); break;
        case 2: c = (uint16_t)(x * 0x30 + y + k - 0x30); break;
        case 3: c = (uint16_t)(x * 0x30 + y + 0x30 * k - 1); break;
        default: return;
        }
        buf[k] = B(GRID + c);
    }
}

/* ---- per-unit logic ---- */
static void set_pending(int i, uint8_t act) {                   /* B925 */
    uint16_t p = (uint16_t)(U(i) + 0x11);
    if (B(p) != 4) B(p) = act;
}
static uint8_t eff_morale(int i) {                              /* B7BE */
    uint16_t u = U(i); uint8_t m = B(u + 0xC);
    if (B(CMDR + 4) != 0 || B(u + 0xB) != 0) return m;
    uint16_t g = U(B(0xC298));
    int dx = SB(g + 2) - SB(u + 2); if (dx < 0) dx = -dx;
    int dy = SB(g + 3) - SB(u + 3); if (dy < 0) dy = -dy;
    int16_t v = (int16_t)(B(CMDR + 2) - dy - dx); if (v < 0) v = 0;
    return (uint8_t)(v + m);
}
static void flee(int i) {                                       /* D08E */
    uint16_t u = U(i);
    if (B(u + 0xD) == 4) return;
    uint8_t dir, steps;
    if (B(u + 0xB) == 1) { dir = 3; steps = B(u + 3); } else { dir = 1; steps = (uint8_t)(0x2C - B(u + 3)); }
    B(u + 6) = (uint8_t)(dir << 6 | steps);
    if (B(u + 0xE) != dir) setW(u + 4, 0xFFFF);
}
static void target_nearest(int i) {                             /* B5E3 */
    uint16_t u = U(i); int best = 0; uint16_t bestd = 0xFFFF;
    for (int j = 1; j < NUNITS; j++) {
        uint16_t t = U(j);
        if (B(t + 0xB) == B(u + 0xB) || B(t + 0xD) == 4 || B(t + 0xD) == 3) continue;
        int16_t dy = (int16_t)(SB(u + 3) - SB(t + 3)), dx = (int16_t)(SB(u + 2) - SB(t + 2));
        uint16_t d = (uint16_t)((uint16_t)(dy * dy) + (uint16_t)(dx * dx));
        if (d < bestd || (d == bestd && B(t + 0xA) == 3)) { best = j; bestd = d; }
    }
    if (!best) { B(u + 6) = 0x80; return; }
    uint16_t t = U(best);
    int16_t dx = (int16_t)(SB(u + 2) - SB(t + 2)), dy = (int16_t)(SB(u + 3) - SB(t + 3));
    uint8_t sa = (uint8_t)(((dx < 0 ? -dx : dx) + 1) / 2), sb = (uint8_t)(((dy < 0 ? -dy : dy) + 1) / 2);
    uint8_t a = (uint8_t)((dx > 0 ? 2 : 0) << 6 | sa), b = (uint8_t)((dy > 0 ? 3 : 1) << 6 | sb);
    if (dy > dx) { uint8_t s = a; a = b; b = s; }
    if (a & 0x3F) {
        B(u + 6) = a;
        uint8_t buf[2] = {0, 0}; front(buf, (uint8_t)(a >> 6), i);   /* buf[1] stays 0 for a 1-wide unit, see g_stale */
        for (int k = 0; k < 2; k++)
            if (buf[k] && B(UNITS + 0x12 * buf[k] + 0xB) == B(u + 0xB)) B(u + 6) = b;
    } else B(u + 6) = b;
    setW(u + 4, 0xFFFF);
}
static int move_unit(int i) {                                   /* CDC7: nonzero = moved (no attack this tick) */
    uint16_t u = U(i);
    if (W(u) == 0 || B(u + 0xD) == 4 || B(u + 0x10) == 4 || (B(u + 0x10) & 0xF) == 2) return 0;
    uint8_t o = B(u + 6), steps = o & 0x3F;
    if (o == 0x80 || steps == 0) {
        if (B(u + 0xD) == 2) { target_nearest(i); return 0xFFFF; }
        return 0;
    }
    uint16_t save = W(u + 2), moved;
    erase(i);
    uint8_t dir = (uint8_t)((SB(u + 6) >> 6) & 3);
    switch (dir) { case 0: B(u + 2)++; break; case 1: B(u + 3)++; break; case 2: B(u + 2)--; break; default: B(u + 3)--; }
    steps--;
    set_pending(i, 1);
    if (B(u + 0xD) == 3 && B(u + 0xA) != 1) {
        if (blocked_by_enemy(u) == 0xFF) { steps++; setW(u + 2, save); set_pending(i, 0); moved = 0; }
        else moved = 0xFFFF;
    } else if (blocked(i, u) == 0xFF) {
        steps++; setW(u + 2, save);
        uint8_t buf[2] = {0, g_stale}; front(buf, B(u + 0xE), i);
        int no_enemy = 1;
        for (int k = 0; k < 2; k++)
            if (buf[k] && B(UNITS + 0x12 * buf[k] + 0xB) != B(u + 0xB)) no_enemy = 0;
        set_pending(i, 0);
        if (no_enemy && B(u + 0xD) == 2) {
            B(u + 6) = 0x80; target_nearest(i);
            dir = (uint8_t)((SB(u + 6) >> 6) & 3); steps = B(u + 6) & 0x3F;
        }
        moved = 0;
    } else moved = 0xFFFF;
    if (steps == 0) {
        setW(u + 4, 0xFFFF);
        if (B(u + 0xD) == 2) { target_nearest(i); moved = 0xFFFF; }
        else { B(u + 6) = B(u + 7); B(u + 7) = B(u + 8); B(u + 8) = 0x80; }
    } else B(u + 6) = (uint8_t)(dir << 6 | steps);
    stamp(i);
    u = U(i);
    if (B(u + 3) == 1 || B(u + 3) == 0x2C) { B(u + 0xD) = 4; erase(i); }
    setW(0xC2B0 + 2 * B(u + 0xB), moved);
    return moved;
}
static void turn_unit(int i) {                                  /* CC7C */
    uint16_t u = U(i);
    if ((B(u + 6) & 0x3F) == 0) { setW(u + 4, 0); return; }
    erase(i);
    uint8_t old = B(u + 0xE), dir = (uint8_t)((SB(u + 6) >> 6) & 3);
    if (dir == old) { setW(u + 4, 0); stamp(i); return; }
    if (B(u + 0xA) == 2) { setW(u + 4, 0); B(u + 0xE) = dir; set_pending(i, 0); stamp(i); return; }
    int diff = dir - old; if (diff < 0) diff = -diff;
    uint8_t nf = diff == 2 ? (uint8_t)((dir + old) / 2) : dir;
    B(u + 0xE) = nf;
    uint16_t save = W(u + 2);
    if (B(u + 0xA) == 1 || B(u + 0xA) == 3) {
        B(u + 2) += B(0x740A + old * 4 + nf);
        B(u + 3) += B(0x741A + old * 4 + nf);
    }
    if (blocked(i, u) == 0xFF) { setW(u + 2, save); B(u + 0xE) = old; set_pending(i, 0); }
    else { if (B(u + 0xE) == dir) setW(u + 4, 0); set_pending(i, (uint8_t)(old << 4 | 2)); }
    stamp(i);
}
static uint16_t hit(uint16_t t, uint16_t a) {                   /* B319: casualties of one attack */
    uint8_t f = B(0x6AF0 + (B(a + 0xA) * 3 + B(t + 0xA)) * 4 + ((uint8_t)(B(a + 0xE) - B(t + 0xE)) & 3));
    switch (B(0x48E)) {                                          /* land battle difficulty */
    case 0: if (B(t + 0xB) == 0) f = (uint8_t)(f * 3 / 2); break;   /* Galley Slave: Romans harder to hurt */
    case 2: if (B(t + 0xB) == 1) f = (uint8_t)(f * 5 / 4); break;   /* Senator: the enemy tougher */
    case 3: if (B(t + 0xB) == 1) f = (uint8_t)(f * 3 / 2); break;   /* Emperor */
    }
    int16_t maxd = (int16_t)(64 / f);
    uint16_t n = (uint16_t)((int16_t)W(a) / (int16_t)B(0x6B14 + B(a + 0xA)) + 1), loss = 0;
    for (uint16_t k = 0; k < n; k++) loss = (uint16_t)(loss + rng_roll((uint16_t)(maxd + 1)));
    if (B(t + 0xA) != 1 && loss < 1) loss = 1;
    return loss;
}
static void general_killed(uint8_t side) {                      /* B4E5 */
    for (int j = 1; j < NUNITS; j++) {
        uint16_t u = U(j);
        if (B(u + 0xB) != side || B(u + 0xD) == 4) continue;
        int16_t m = (int16_t)(SB(u + 0xC) - 15); if (m < 0) m = 0;
        B(u + 0xC) = (uint8_t)m;
        uint16_t r = rng_roll(100);
        if (SB(u + 0xC) < (int8_t)(uint8_t)r) { B(u + 0xD) = 3; flee(j); }
    }
}
static void melee(int i) {                                      /* B0E0 */
    uint16_t u = U(i);
    if (W(u) == 0) return;
    if (B(u + 0xA) != 1 && B(u + 0xD) == 3) return;
    if (B(u + 0xD) == 4) return;
    uint8_t w = B(0x6AE8 + B(u + 0xA)), buf[4] = {0, 0, 0, 0};
    front(buf, B(u + 0xE), i);
    for (uint8_t k = 0; k < w; k++) {
        uint8_t ti = buf[k];
        if (ti == 0 || ti == 0xFF || ti == 0xFE) continue;
        uint16_t t = (uint16_t)(UNITS + 0x12 * ti);
        if (!(B(t + 0xB) != B(u + 0xB) || B(u + 0xA) == 1)) continue;
        set_pending(i, 3);
        if (B(u + 0xA) == 1 && W(0xC28E + 2 * B(u + 0xB))) {    /* elephant terror (unreachable, see notes) */
            int16_t m = (int16_t)(eff_morale(ti) - 25); if (m < 0) m = 0;
            if (rng_roll(100) > (uint8_t)m) { B(t + 0xD) = 3; flee(ti); }
        }
        if (B(t + 0x10) == 0 && B(u + 0xD) < 3) {
            uint8_t nf = (uint8_t)((B(u + 0xE) + 2) & 3);
            if (nf != B(t + 0xE)) { setW(t + 4, 0xFFFF); B(t + 6) = (uint8_t)(nf << 6 | 1); }
        }
        int16_t v = (int16_t)W(SNAP + 2 * ti);
        if (B(t + 0xA) == 3) {
            setW(t, (uint16_t)(W(t) - 1));
            if (W(t) == 0) {
                set_pending(ti, 4);
                general_killed(B(t + 0xB));
                if (B(t + 0xB) == 0) B(CMDR + 4) = 2;          /* the Roman commander dies */
            }
        } else {
            uint16_t dmg = B(u + 0xA) == 3 ? 0 : hit(t, u);
            v = (int16_t)(v - dmg); if (v < 0) v = 0;
        }
        setW(SNAP + 2 * ti, (uint16_t)v);
    }
}
static void end_of_tick(void) {                                 /* B405 */
    for (int i = 1; i < NUNITS; i++) {
        uint16_t u = U(i);
        if (W(u) == 0 || B(u + 0xA) == 3) continue;
        uint16_t loss = (uint16_t)(W(u) - W(SNAP + 2 * i));
        int rout = 0;
        if ((int16_t)(uint16_t)(loss << 4) > (int16_t)W(u)) {
            int8_t m = (int8_t)(B(u + 0xC) - (uint8_t)((int16_t)loss / 2));
            if (m < 0) m = 0;
            B(u + 0xC) = (uint8_t)m;
            if (SB(u + 0xC) < 50) { uint16_t em = eff_morale(i); if (rng_roll(100) > 2 * em) rout = 1; }
        } else if (B(u + 0xA) == 1 && loss != 0) {
            if (rng_roll(100) > 0x4B) rout = 1;
        }
        if (rout) { B(u + 0xD) = 3; flee(i); }
        setW(u, W(SNAP + 2 * i));
        if (W(u) == 0) set_pending(i, 4);
    }
}
static void enemy_ai(int tick, int i) {                         /* B54C */
    if (tick <= 15) return;
    uint16_t n = 0;
    for (int j = 1; j < NUNITS; j++) if (B(U(j) + 0xD) != 4) n++;
    if (n > 13) n = 13;
    uint16_t tbl = (uint16_t)(0x6B18 + ((n - 1) & 0xFFFC));
    uint16_t u = U(i);
    if (B(u + 0xD) != 1) return;
    uint16_t r = rng_roll(100);
    uint8_t thr = B(tbl + (int8_t)(SB(u + 0xC) / 25));
    if ((uint16_t)(B(0xC2A3) - thr) < r) B(u + 0xD) = 2;
}
static void elephant_amok(int i) {                              /* D012 */
    uint16_t u = U(i);
    if (W(u) == 0) return;
    uint8_t old = B(u + 0xE), d;
    erase(i);
    d = rng_roll(3) == 0 ? (uint8_t)rng_roll(4) : old;
    if (d != old) setW(u + 4, 0xFFFF);
    B(u + 7) = (uint8_t)(d << 6 | 2);
    stamp(i);
}
static void next_action(int i) {                                /* B848 */
    uint16_t u = U(i);
    if (B(u + 0x10) == 4) { B(u + 0xD) = 4; erase(i); return; }
    uint8_t a = B(u + 0x11) & 0xF, anim;
    if (a == 2) {
        anim = B(0x6C24 + (a * 4 + B(u + 0xA)) * 4 + ((B(u + 0x11) & 0x30) >> 4));
        if (B(u + 0xE) == 2 || B(u + 0xE) == 1) anim++;
    } else anim = B(0x6C24 + (a * 4 + B(u + 0xA)) * 4 + B(u + 0xE));
    B(u + 9) = anim;
    B(u + 0x10) = B(u + 0x11);
    B(u + 0x11) = B(0x724C + (B(u + 0x11) & 0xF));
}
static void sprite_step(int adv, int i) {                       /* B94D: sprite list entry + animation frame */
    uint16_t u = U(i);
    if (B(u + 0xD) == 4) return;
    uint16_t lst = W(0x6DA6 + 2 * B(u + 9));
    uint8_t f = B(lst) == 0xFF ? B(u + 0xF) : B(lst + B(u + 0xF));
    uint16_t spr = (uint16_t)(f + W(0x6B28 + 2 * B(u + 9)));
    uint16_t s = (uint16_t)(0xA92A + 6 * W(0xA928));
    g_stale = (uint8_t)((6 * W(0xA928)) >> 8);
    setW(s, (uint16_t)(B(u + 3) << 8 | (uint16_t)(int16_t)SB(u + 2)));
    setW(s + 2, (uint16_t)(spr + (B(u + 0xB) == 1 ? 0xBF : 0)));
    B(s + 4) = B(u + 0xF); B(s + 5) = B(u + 9);
    setW(0xA928, (uint16_t)(W(0xA928) + 1));
    if (W(0xA928) > 36) quirk("sprite list overflow (> 36 units on the field)", i);
    if (!adv) return;
    B(u + 0xF)++;
    if (B(0x6BD0 + B(u + 9)) <= B(u + 0xF)) B(u + 0xF) = 0;
}
static int battle_over(void) {                                  /* E5B8 */
    int rom = 0, ene = 0;
    for (int i = 1; i < NUNITS; i++) {
        uint16_t u = U(i);
        if (W(u) == 0 || B(u + 0xA) == 3 || B(u + 0xD) == 4 || B(u + 0xD) == 3) continue;
        if (B(u + 0xB) == 0) rom = 1;
        if (B(u + 0xB) == 1) ene = 1;
    }
    if (!ene || !rom) { B(0xAA4C)++; if (B(0xAA4C) > 0x1E) return 1; }
    return 0;
}

/* ---- setup ---- */
static void make_units(uint8_t morale, uint8_t side, uint8_t facing, uint8_t type, int16_t size, int16_t total) {   /* C5EC */
    while (total > 0) {
        uint16_t u = U(W(0xAA4A));
        B(u + 0xB) = side; B(u + 0xA) = type; B(u + 0xE) = facing; setW(u + 4, 0xFFFF);
        setW(u, (uint16_t)(size <= total ? size : total));
        total = (int16_t)(total - size);
        B(u + 0xC) = morale; B(u + 0xD) = 0; B(u + 0xF) = 0;
        B(u + 9) = B(0x7244 + type * 2 + side);
        B(u + 0x10) = 0; B(u + 0x11) = 0;
        for (int k = 0; k < 3; k++) B(u + 6 + k) = 0x80;
        setW(0xAA4A, (uint16_t)(W(0xAA4A) + 1));
    }
}
static int build_units(int L) {                                 /* C153 */
    uint16_t leg = LEG(L);
    uint16_t si = (uint16_t)((W(leg + 6) + 11) / 12), floor = W(0x7402 + 2 * B(0x48E));
    if (si < floor) si = floor;
    if (si > 500) si = 500;
    uint16_t ne = (uint16_t)((int16_t)(B(0xC2A2) + 0x13) / 0x14 + 1);
    ne = (uint16_t)(ne + (uint16_t)(W(0xC29C) + 0x95) / 0x96);
    ne = (uint16_t)(ne + (uint16_t)(W(0xC29E) + 0x1F3) / 0x1F4);
    setW(0x73FE, ne);
    uint16_t nr = (uint16_t)((uint16_t)(W(leg + 8) + 0x95) / 0x96 + 1);
    nr = (uint16_t)(nr + (uint16_t)(W(leg + 6) + si - 1) / si);
    setW(0x7400, nr);
    setW(0xC294, (uint16_t)(nr + ne + 1));
    if (DS_UNITS + 0x14u * NUNITS + 4 > 0x10000u) { fprintf(stderr, "battle: %u units do not fit\n", NUNITS); return 0; }
    setW(0xC292, DS_UNITS);
    setW(0xC296, (uint16_t)((DS_UNITS + 0x12 * NUNITS + 3) & ~1u));   /* next heap block: 2-byte header, even */
    memset(&g_ds[DS_UNITS], 0, 0x14u * NUNITS + 4);
    setW(0xAA4A, 1);
    uint8_t m = B(leg + 0xB);
    make_units(m, 0, 3, 0, 0x96, (int16_t)W(leg + 8));
    make_units(m, 0, 3, 2, (int16_t)si, (int16_t)W(leg + 6));
    B(0xC298) = B(0xAA4A);
    make_units(m, 0, 3, 3, 5, 5);
    m = B(0xC2A4);
    make_units(m, 1, 1, 1, 0x14, B(0xC2A2));
    make_units(m, 1, 1, 0, 0x96, (int16_t)W(0xC29C));
    make_units(m, 1, 1, 2, 0x1F4, (int16_t)W(0xC29E));
    make_units(m, 1, 1, 3, 5, 5);
    return 1;
}
static void place(uint8_t f, uint8_t side, uint8_t mode) {     /* C2F2: mode 2 = base + offset (Romans), else base - offset */
    uint16_t lay = W(0x76EC + 2 * f), offs = (uint16_t)(lay + 2);
    uint16_t base = W(lay + 0x24 + 2 * side);
    #define SETPOS(i, s) do { uint16_t _u = U(i); int8_t _dx = SB(offs + 2 * (s)), _dy = SB(offs + 2 * (s) + 1); \
        B(_u + 2) = (uint8_t)(mode == 2 ? (uint8_t)base + _dx : (uint8_t)base - _dx); \
        B(_u + 3) = (uint8_t)(mode == 2 ? (uint8_t)(base >> 8) + _dy : (uint8_t)(base >> 8) - _dy); } while (0)
    if (side == 1) {
        uint16_t k = 0;
        for (int i = 1; i < NUNITS; i++) {
            uint16_t u = U(i);
            if (B(u + 0xA) != 1) continue;
            B(u + 2) = B(0x73DA + 6 * f + k); B(u + 3) = B(0x73F8 + f);
            stamp(i); B(U(i) + 6) = 0xFE; k++;
        }
    }
    int i = 1;
    for (int c = 0; c < 4; c++) {
        for (; i < NUNITS; i++) {
            uint16_t u = U(i);
            if (B(u + 0xB) == side && B(u + 0xA) == 0 && B(u + 2) == 0 && B(u + 3) == 0) break;
        }
        if (i < NUNITS) {
            uint8_t s = B(lay + 0x28 + c);                      /* the formation's 4 cavalry slots */
            SETPOS(i, s);
            if (side == 1) { B(U(i) + 2)--; B(U(i) + 3)--; }
            B(U(i) + 6) = s;
            stamp(i);
            i++;
        }
    }
    uint16_t k = 0;
    for (i = 1; i < NUNITS; i++) {
        uint16_t u = U(i);
        if (B(u + 0xB) != side || B(u + 6) != 0x80) continue;
        int done = 0; uint8_t s = 0;
        while (!done) {
            uint8_t type = B(U(i) + 0xA);
            if (k > 0x11) break;
            int last = 0;
            if (type == 3) { s = 0x10; last = 1; } else s = (uint8_t)k++;
            SETPOS(i, s);
            if (side == 1) { B(U(i) + 2) += (uint8_t)(1 - B(0x6AE8 + type)); if (type != 2) B(U(i) + 3)--; }
            if (blocked(i, U(i)) == 0) done = 1; else if (last) done = 1;
        }
        if (done) { B(U(i) + 6) = s; stamp(i); }
    }
    #undef SETPOS
}
static void initial_orders(uint16_t tac, uint8_t side) {       /* C525 */
    for (int i = 1; i < NUNITS; i++) {
        uint16_t u = U(i);
        if (B(u + 0xB) != side) continue;
        uint8_t slot = B(u + 6);
        B(u + 6) = 0x80;
        if (slot == 0xFE) B(u + 6) = 0x4B;
        else for (int q = 0; q < 3; q++) {
            uint16_t off = W(tac + 2 + 2 * q);
            if (off == 0xFFFF) { B(u + 6 + q) = 0x80; continue; }
            uint8_t b = B(W(0x76EA) + off + slot), dir = (uint8_t)(b >> 6), st = b & 0x3F;
            if (B(u + 0xB) == 1) dir = (uint8_t)((dir + 2) & 3);
            B(u + 6 + q) = (uint8_t)(dir << 6 | st);
        }
        B(u + 0xD) = 1;
    }
}

/* ---- the battle ---- */
static uint8_t enemy_style(void) { return B((W(0xC29A) << 5) + 0x5286); }   /* prov[enemy home]+0x16 */

static void roll_enemy_formation(void) {                       /* BE3C 0xBF40: before the formation menu */
    uint8_t style = enemy_style(), size = (uint8_t)((uint16_t)(W(0x73FE) - 2) >> 2);
    uint16_t r = rng_roll(100), sum = 0, row = (uint16_t)((style * 4 + size) * 5);
    B(0xC2A9) = 0xFF;
    do { B(0xC2A9)++; sum = (uint16_t)(sum + B(0x77F0 + row + B(0xC2A9))); } while (sum <= r);
}
/* Interactive hooks (battle_ui.c); all NULL in headless runs. */
int  (*battle_menu_hook)(int which);         /* 0 = Choose Formation, 1 = Choose Tactic: index, or -1 = Esc (cancel) */
void (*battle_setup_hook)(int Z);            /* BB97: load the field and sprites (DS:C29A = enemy home is set) */
void (*battle_message_hook)(const char *s);  /* 59D4 message box ("Release The Legion!" ...) */
void (*battle_report_hook)(int won, const char *title, const char *text, unsigned rcas, unsigned ecas, unsigned rsurv, unsigned esurv);
static void message(const char *s) { if (battle_message_hook) battle_message_hook(s); else fprintf(stderr, "message: %s\n", s); }

/* BB4D (+ BA2C): the sprite list of every unit, frames not advanced (for screens drawn outside a tick) */
void battle_sprites_build(void) {
    setW(0xA928, 0);
    for (int i = 1; i < NUNITS; i++) sprite_step(0, i);
}

static int setup_after_formation(uint8_t pf, uint8_t pt) {    /* BE3C 0xBFA2..0xC10C; 0 = cancelled */
    B(0xC2AA) = pf;
    place(pf, 0, 2);
    place(B(0xC2A9), 1, 0);
    uint16_t p = W(0x7986 + 2 * (enemy_style() * 5 + B(0xC2A9)));
    if (p) {
        uint16_t r = rng_roll(100), sum = 0;
        B(0xC2AB) = 0xFF;
        do { B(0xC2AB)++; sum = (uint16_t)(sum + B(p + B(0xC2AA) * 5 + B(0xC2AB))); } while (sum <= r);
    } else B(0xC2AB) = (uint8_t)rng_roll(4);
    B(0xC2AD) = B(0x76FE + (B(0xC2A9) * 5 + B(0xC2AB)) * 10);
    if (battle_menu_hook) {
        int r = battle_menu_hook(1);
        if (r < 0) return 0;
        pt = (uint8_t)r;
    }
    B(0xC2AC) = pt;
    initial_orders((uint16_t)(0x76F6 + (pf * 5 + pt) * 10), 0);
    initial_orders((uint16_t)(0x76F6 + (B(0xC2A9) * 5 + B(0xC2AB)) * 10), 1);
    return 1;
}
static void fight(void) {                                       /* DF24 main loop 0xE07D..0xE19C */
    for (int tick = 0; !battle_over(); tick++) {
        setW(0xA928, 0);                                        /* BA2C: empty sprite list */
        for (int i = 1; i < NUNITS; i++) setW(SNAP + 2 * i, W(U(i)));
        for (int i = 1; i < NUNITS; i++) {
            uint16_t u = U(i);
            if (B(u + 0xD) == 4) continue;
            if (B(u + 0xF) == 0) {
                if (W(u + 4) == 0) {
                    if (!move_unit(i)) melee(i);
                    u = U(i);
                    if (B(u + 0xD) == 3 && B(u + 0xA) == 1) elephant_amok(i);
                } else turn_unit(i);
                if (B(U(i) + 0xB) == 1) enemy_ai(tick, i);
                next_action(i);
            }
            sprite_step(1, i);
        }
        end_of_tick();
        if (battle_orders_hook) battle_orders_hook(tick);      /* D0F2: the player may open the orders mode */
        for (int s = 0; s < 2; s++) {
            if (W(0xC2B0 + 2 * s) == 0) B(0xAA4E + s)++; else B(0xAA4E + s) = 0;
            setW(0xC2B0 + 2 * s, 0);
        }
        if (B(0xC2AD) == 1 && B(0xAA4F) >= 10)                  /* the enemy's tactic: charge after 10 idle ticks */
            for (int i = 1; i < NUNITS; i++) {
                uint16_t u = U(i);
                if (B(u + 0xB) == 1 && B(u + 0xD) == 1) B(u + 0xD) = 2;
            }
    }
}
static const char *dstr(uint16_t a) { return (const char *)&g_ds[a]; }

/* EXE 0x6738: number -> Roman numeral from the DS tables (hundreds 4F82, tens 4F6E, units 4F5A); > 999 = "OOPS" (4F54) */
void roman_numeral(unsigned n, char *out) {
    if (n > 999) { strcpy(out, dstr(0x4F54)); return; }
    strcpy(out, dstr(W(0x4F82 + 2 * (n / 100))));
    strcat(out, dstr(W(0x4F6E + 2 * (n / 10 % 10))));
    strcat(out, dstr(W(0x4F5A + 2 * (n % 10))));
}

/* E3ED..E4AA: the report's text, title first (7ADC + the pieces 7ADE..7B04; numeral = 6738 of legion +0x0A) */
void battle_report_text(int L, int won, char *out, size_t n) {
    uint16_t leg = LEG(L), zn = W(0x5222 + 2 * W(0xC29A));
    uint16_t et = (uint16_t)(0x76F6 + (B(0xC2A9) * 5 + B(0xC2AB)) * 10), rt = (uint16_t)(0x76F6 + (B(0xC2AA) * 5 + B(0xC2AC)) * 10);
    char num[16]; roman_numeral(B(leg + 0xA), num);
    const char *p[] = { won ? dstr(0x7A02) : dstr(0x7A16), dstr(0x7A2C), num, dstr(leg), dstr(0x7A43), dstr(zn), dstr(0x7A5A),
                        dstr(W(0x5F1C + 2 * W(0xC29A))), dstr(0x7A75), dstr(W(et)), dstr(0x7A87), dstr(W(rt)), dstr(0x7A9C),
                        dstr(W(CMDR)), dstr(0x7AA1), won ? dstr(0x79E3) : dstr(0x79C2), num, dstr(leg), dstr(0x7AB7), dstr(zn),
                        dstr(0x7ABE) };
    out[0] = 0;
    for (size_t k = 0; k < sizeof(p) / sizeof(p[0]); k++) strncat(out, p[k], n - strlen(out) - 1);
}

static uint16_t result(int L, int Z) {                          /* E21A */
    uint16_t rc = 0, ri = 0, ec = 0, ee = 0, ei = 0, won = 0;
    for (int i = 1; i < NUNITS; i++) {
        uint16_t u = U(i);
        if (W(u) == 0) continue;
        if (B(u + 0xB) == 0) {
            if (B(u + 0xD) != 3 && B(u + 0xD) != 4 && B(u + 0xA) != 3) won = 0xFFFF;
            if (B(u + 0xA) == 0) rc = (uint16_t)(rc + W(u));
            if (B(u + 0xA) == 2) ri = (uint16_t)(ri + W(u));
        }
        if (B(u + 0xB) == 1) {
            if (B(u + 0xA) == 0) ec = (uint16_t)(ec + W(u));
            if (B(u + 0xA) == 1) ee = (uint16_t)(ee + W(u));
            if (B(u + 0xA) == 2) ei = (uint16_t)(ei + W(u));
        }
    }
    uint16_t leg = LEG(L);
    uint16_t rcas = (uint16_t)(W(leg + 6) + W(leg + 8) - ri - rc), rsurv = (uint16_t)(ri + rc);
    setW(leg + 8, rc); setW(leg + 6, ri);
    uint16_t ecas = (uint16_t)(B(0xC2A2) - ei - ee - ec + W(0xC29E) + W(0xC29C)), esurv = (uint16_t)(ei + ec);
    setW(0xC29C, ec); B(0xC2A2) = (uint8_t)ee; setW(0xC29E, ei);
    if (won) {
        setW(0xC2A0, 0);
        B(leg + 0xB) = B(leg + 0xB) > 0x5E ? 0x63 : (uint8_t)(B(leg + 0xB) + 5);
        if (Z == W(0xC29A)) {
            uint16_t p = (uint16_t)(Z << 5);
            setW(p + 0x5274, esurv);
            if (W(p + 0x5276) < esurv) setW(p + 0x5274, W(p + 0x5276));
        }
        for (int k = 0; k < 8; k++) {
            uint8_t n = B(0x50EC + Z * 8 + k);
            if (n >= 0x1B) continue;
            int v = SB((n << 5) + 0x527D) + 25;
            B((n << 5) + 0x527D) = (uint8_t)(v > 100 ? 100 : v);
        }
    } else if (W(leg + 8) == 0 && W(leg + 6) == 0) legion_lost(L);
    else { int v = B(leg + 0xB) - 5; B(leg + 0xB) = (uint8_t)(v < 0 ? 0 : v); }
    /* report (0xE3ED..0xE58B) */
    uint16_t et = (uint16_t)(0x76F6 + (B(0xC2A9) * 5 + B(0xC2AB)) * 10), rt = (uint16_t)(0x76F6 + (B(0xC2AA) * 5 + B(0xC2AC)) * 10);
    uint16_t zn = W(0x5222 + 2 * W(0xC29A));
    {
        static char text[600];
        battle_report_text(L, won != 0, text, sizeof(text));
        if (battle_report_hook) battle_report_hook(won != 0, NULL, text, rcas, ecas, rsurv, esurv);
        fprintf(stderr, "battle: %s Legion %s met the army of %s. %s used %s against %s of %s.\n"
                        "battle: casualties Rome %u, %s %u; survivors %u, %u\n",
                won ? dstr(0x7A02) : dstr(0x7A16), dstr(leg), dstr(zn), dstr(W(0x5F1C + 2 * W(0xC29A))),
                dstr(W(et)), dstr(W(rt)), dstr(W(CMDR)), rcas, dstr(zn), ecas, rsurv, esurv);
    }
    if (won) setW(0xC0FA, (uint16_t)(W(0xC0FA) + 1));
    uint32_t t = ((uint32_t)W(0xC088) << 16 | W(0xC086)) + rcas; setW(0xC086, (uint16_t)t); setW(0xC088, (uint16_t)(t >> 16));
    t = ((uint32_t)W(0xC084) << 16 | W(0xC082)) + ecas; setW(0xC082, (uint16_t)t); setW(0xC084, (uint16_t)(t >> 16));
    if (B(CMDR + 4) == 2 && B(leg + 0xD) != 0xFF) {
        g_enemy_unported = 1;
        fprintf(stderr, "battle: the commander fell; a new one must be assigned (EXE 0x4733, not ported)\n");
    }
    return won;
}
static uint16_t finish(int A, int L, int Z) {                  /* DF24 0xE000..0xE205 around the main loop */
    uint16_t leg = LEG(L);
    setW(0xC28E, B(leg + 0xF) == 0);
    if (B(0xC2A2)) { B(leg + 0xF) = 0xFF; setW(0xC290, 0); }
    B(0xAA4E) = B(0xAA4F) = 0; setW(0xC2B0, 0); setW(0xC2B2, 0);
    fight();
    uint16_t ret = result(L, Z);
    if (W(0xC29A) != (uint16_t)Z) memcpy(&g_ds[(uint16_t)(0xC114 + 14 * A)], &g_ds[0xC29A], 12);
    else if (!ret) {
        uint16_t p = (uint16_t)(Z << 5);
        setW(p + 0x5272, W(0xC29C)); B(p + 0x527C) = B(0xC2A2); setW(p + 0x5270, W(0xC29E));
    }
    return ret;
}

/* Resume a battle captured at the Choose Formation menu (units built, enemy formation already rolled). */
uint16_t battle_from_formation(int A, int L, int Z, uint8_t pf, uint8_t pt) {
    if (!setup_after_formation(pf, pt)) return 0;    /* Esc in the tactic menu: nothing written back */
    return finish(A, L, Z);
}

/* The whole battle, EXE thunk 0xB06E -> DF24(A = army or 0xFF, L = legion, Z = zone); pf/pt = the player's menu choices.
   tactics = TACTICS.LZW from BATL.DAT, or NULL to use what DS:[76EA] already points at. Returns 0xFFFF = Rome won. */
uint16_t battle(int A, int L, int Z, uint8_t pf, uint8_t pt, const uint8_t *tactics, size_t tactics_len) {
    B(0xC28C) = B(LEG(L) + 0x10);
    setW(0xC0F8, (uint16_t)(W(0xC0F8) + 1));
    if (A == 0xFF) {
        uint16_t p = (uint16_t)(Z << 5);
        setW(0xC29E, W(p + 0x5270)); setW(0xC29C, W(p + 0x5272));
        B(0xC2A2) = B(p + 0x527C); B(0xC2A3) = B(p + 0x527B); B(0xC2A4) = B(p + 0x527A);
        setW(0xC29A, (uint16_t)Z); setW(0xC2A0, 0xFFFF);
    } else memcpy(&g_ds[0xC29A], &g_ds[(uint16_t)(0xC114 + 14 * A)], 12);
    if (W(0xC2A0) == 0) return 0xFFFF;
    setW(0xC2AE, DS_GRID); memset(&g_ds[DS_GRID], 0, 0x900);
    B(0xAA4C) = 0;
    /* BE3C: the diamond-shaped field */
    for (int s = 0; s < 0x13; s++) border(0x12 - s, s);
    for (int s = 0; s < 0x18; s++) { border(s, s + 0x17); border(0x2B - s, 0x17 - s); }
    for (int s = 0; s < 0x14; s++) border(s + 0x18, 0x2F - s);
    for (int s = 0x13; s <= 0x15; s++) border(0, s);
    border(0x13, 0);
    {   /* BB97 0xBC9B: barbarian enemies (home 11-14, 23-26) get the 'b' general/infantry sprites and music 9/10 */
        uint16_t h = W(0xC29A);
        int barb = (h >= 0xB && h <= 0xE) || (h >= 0x17 && h <= 0x1A);
        B(0xC2A6) = barb ? 9 : 0xB; B(0xC2A7) = barb ? 0xA : 0xC;
    }
    if (tactics) {
        if (tactics_len > 0x400) tactics_len = 0x400;
        memcpy(&g_ds[DS_TACTICS], tactics, tactics_len); setW(0x76EA, DS_TACTICS);
    }
    if (battle_setup_hook) battle_setup_hook(Z);
    if (!build_units(L)) return 0;
    roll_enemy_formation();
    if (battle_menu_hook) {
        int r = battle_menu_hook(0);
        if (r < 0) return 0;                           /* Esc in the formation menu: nothing written back */
        pf = (uint8_t)r;
    }
    return battle_from_formation(A, L, Z, pf, pt);
}

/* ---- the player's orders (orders mode D0F2/D198/D33A; the screen and input are the caller's) ----
   The battle is frozen while the player gives orders: call these from battle_orders_hook. */

/* D21E "Melee": "Release The Legion!" (message 74B6 + 74B8). Units already attacking are re-aimed, everyone holding
   charges the nearest enemy, the general halts. Leaves the orders mode. */
void battle_order_legion_melee(void) {
    { char m[64]; snprintf(m, sizeof(m), "%s%s", dstr(W(0x74B6)), dstr(W(0x74B8))); message(m); }
    for (int i = 1; i < NUNITS; i++) {
        uint16_t u = U(i);
        if (B(u + 0xB) != 0) continue;
        if (B(u + 0xD) == 2) { B(u + 6) &= 0xC0; B(u + 0xD) = 1; }
        if (B(u + 0xD) != 1) continue;
        if (B(u + 0xA) != 3) { B(u + 0xD) = 2; target_nearest(i); }
        else B(u + 6) &= 0xC0;
    }
}
/* D299 "Retreat": "Run Away!\nRun Away!" (74D4). Every Roman unit marches back to its edge (state 1, units that walk off
   keep their men). Leaves the orders mode. */
void battle_order_legion_retreat(void) {
    message(dstr(0x74D4));
    for (int i = 1; i < NUNITS; i++) {
        uint16_t u = U(i);
        if (B(u + 0xB) != 0 || B(u + 0xD) == 4) continue;
        if (B(u + 0xD) == 2) B(u + 0xD) = 1;
        flee(i);
    }
}
/* D8AD: is unit i within the general's voice? (commander alive and on duty, dx^2+dy^2 <= voice^2) */
int battle_in_voice(int i) {
    if (B(CMDR + 4) != 0) return 0;
    uint16_t u = U(i), g = U(B(0xC298));
    int16_t dx = (int16_t)(SB(g + 2) - SB(u + 2)), dy = (int16_t)(SB(g + 3) - SB(u + 3));
    uint16_t d = (uint16_t)((uint16_t)(dx * dx) + (uint16_t)(dy * dy));
    return (uint16_t)(B(CMDR + 3) * B(CMDR + 3)) >= d;
}
/* CBEE: one order leg from unit i toward cell (x, y): the dominant axis, Chebyshev length */
static uint8_t leg_to(int i, uint8_t x, uint8_t y) {
    uint16_t u = U(i);
    int16_t dx = (int16_t)(SB(u + 2) - x), dy = (int16_t)(SB(u + 3) - y);
    int16_t ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy;
    uint8_t steps = (uint8_t)(ay >= ax ? ay : ax), dir = 0;
    if (steps) {
        if (ay > ax) dir = ay == dy ? 3 : 1;
        else dir = ax == dx ? 2 : 0;
    }
    return (uint8_t)(dir << 6 | steps);
}
/* D33A: editing one unit's orders. battle_cmd_begin(i) after the player picked unit i on the field (cell under the
   cursor or a neighbour, D2DD) - returns 0 if it can't take orders (enemy, routed, out of the general's voice);
   then any of target/hold/melee/retreat; then battle_cmd_end(). melee and retreat end the editing themselves.
   from_x/from_y = the cell where the drag starts (mouse: the clicked cell; keyboard: the unit's position). */
static struct { int i; uint8_t leg, px, py; int changed; } g_cmd;
int battle_cmd_begin(int i, uint8_t from_x, uint8_t from_y) {
    uint16_t u = U(i);
    g_cmd.i = 0;                                                 /* refused: the editing calls below do nothing */
    if (i < 1 || i >= NUNITS || B(u + 0xB) != 0 || B(u + 0xD) == 3 || !battle_in_voice(i)) return 0;
    uint8_t o = B(u + 6);
    g_cmd.i = i; g_cmd.leg = (uint8_t)(((o >> 6) & 3) << 6 | ((o & 0x3F) ? 1 : 0));
    if (from_x == 0xFF) { from_x = B(u + 2); from_y = B(u + 3); }   /* keyboard: the cursor starts on the unit */
    g_cmd.px = from_x; g_cmd.py = from_y; g_cmd.changed = 0;
    return 1;
}
void battle_cmd_target(uint8_t x, uint8_t y) {                  /* drag / cursor moved to cell (x, y) */
    if (!g_cmd.i) return;
    if (x == g_cmd.px && y == g_cmd.py) return;
    uint16_t u = U(g_cmd.i);
    g_cmd.changed = 1;
    B(u + 6) = leg_to(g_cmd.i, x, y); B(u + 7) = 0x80; B(u + 8) = 0x80;
    g_cmd.px = x; g_cmd.py = y;
}
void battle_cmd_hold(void) {                                    /* Space: no orders (the state is left as it is) */
    if (!g_cmd.i) return;
    uint16_t u = U(g_cmd.i);
    g_cmd.px = B(u + 2); g_cmd.py = B(u + 3);
    B(u + 6) = B(u + 7) = B(u + 8) = 0x80;
}
void battle_cmd_end(void) {
    if (!g_cmd.i) return;
    if (!g_cmd.changed) return;
    uint16_t u = U(g_cmd.i);
    setW(u + 4, 0xFFFF); B(u + 0xD) = 1;
}
void battle_cmd_melee(void) {                                   /* D7E8 "Release Cohort <n>" */
    if (!g_cmd.i) return;
    int i = g_cmd.i; uint16_t u = U(i);
    if (B(u + 0xA) != 3) {
        uint8_t steps = B(u + 6) & 0x3F;
        { char m[64], n[16]; roman_numeral((unsigned)i, n); snprintf(m, sizeof(m), "%s%s%s%s", dstr(W(0x74B6)), dstr(W(0x74BA)), dstr(0x74F2), n); message(m); }
        if (B(u + 0xD) == 2) { B(u + 6) &= 0xC0; B(u + 0xD) = 1; }
        if (B(u + 0xD) == 1) {
            B(u + 0xD) = 2; target_nearest(i);
            if (steps) { B(u + 7) = B(u + 6); B(u + 6) = g_cmd.leg; }
        }
    }
    battle_cmd_end();
}
void battle_cmd_retreat(void) {                                 /* D77C "Run Away!" */
    if (!g_cmd.i) return;
    int i = g_cmd.i; uint16_t u = U(i);
    uint8_t steps = B(u + 6) & 0x3F;
    message(dstr(0x74E8));
    if (B(u + 0xD) != 4) {
        if (B(u + 0xD) == 2) B(u + 0xD) = 1;
        flee(i);
        if (steps) { B(u + 7) = B(u + 6); B(u + 6) = g_cmd.leg; }
    }
    battle_cmd_end();
}

/* EXE 0x4733: assign a free commander (status 1) to legion L at random; if none is free, every commander not on duty
   (status 1 or 2: the dead come back) is made free first. The roll and the record writes only; the message "<name> is
   assigned\nto command <numeral><legion>" is built and shown by the callers (commander_msg_4733 in enemy.c / legions.c). */
void assign_commander(int L) {
    uint16_t leg = LEG(L);
    if (B(leg + 0xD) == 0xFF) return;
    uint8_t list[24]; uint16_t n = 0;
    for (uint8_t c = 0; c < 0x18; c++) if (B(0x1A20 + 6 * c) == 1) list[n++] = c;
    if (n == 0)
        for (uint8_t c = 0; c < 0x18; c++)
            if (B(0x1A20 + 6 * c) != 0) { B(0x1A20 + 6 * c) = 1; list[n++] = c; }
    uint8_t c = list[rng_roll(n)];   /* n == 0 (all 24 on duty) would read a stale local in the original */
    B(leg + 0x10) = c;
    B(0x1A20 + 6 * c) = 0;
    /* the message (47C6..480B) is shown by the callers: enemy.c / legions.c commander_msg_4733 */
}

/* Battles started by End Turn. battle_choices supplies the player's formation and tactic (headless runs: from the command
   line); NULL = not available, the caller stops as before. Tactics come from BATL.DAT in the current folder.
   With battle_ui_attach(1) (screen.c, the game): the battle is fought on screen (battle_ui.c battle_play); in a scripted
   headless run (screen_scripted()) its menus take battle_choices and the orders come from battle_orders_hook, both set
   by main.c from the script's "battle" / "orders" lines. */
int (*battle_choices)(uint8_t *formation, uint8_t *tactic);
uint16_t battle_ui_script(int A, int L, int Z, int pf, int pt, int (*orders)(int tick));   /* battle_ui.c */
int battle_auto(int A, int L, int Z, uint16_t *won) {
    static uint8_t *tac; static size_t tac_n;
    uint8_t pf, pt;
    if (g_battle_ui_attached) {
        if (!screen_scripted()) { *won = battle_play(A, L, Z); return 1; }
        if (!battle_choices || !battle_choices(&pf, &pt)) return 0;
        *won = battle_ui_script(A, L, Z, pf, pt, battle_orders_hook);
        return 1;
    }
    if (!battle_choices || !battle_choices(&pf, &pt)) return 0;
    if (!tac) {
        int k;
        if (assets_load("BATL.DIR", "BATL.DAT") == 0 && (k = assets_find("TACTICS.LZW")) >= 0) tac = assets_decode(k, &tac_n);
        if (!tac) { fprintf(stderr, "battle: TACTICS.LZW not found (BATL.DAT in the current folder)\n"); return 0; }
    }
    *won = battle(A, L, Z, pf, pt, tac, tac_n);
    return 1;
}
